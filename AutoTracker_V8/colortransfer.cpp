#include "colortransfer.h"
#include "autotrackercontroller.h"

using namespace std;

ColorTransfer::ColorTransfer(QObject *parent) : QObject{parent}
{
    //    refImagePath = QDir::homePath() + "/Qt/AutoTracker/AutoTracker_V6/images/lutRef_07.png";

//    if (captureDevice->deviceConfig.colorSpace == RGB888_CS) { refImagePath = "/home/comswd/Pictures/s2.jpg"; }
//    else { refImagePath = "/home/comswd/Pictures/lutRef_07.png"; }

//    refImagePath = "/home/comswd/Pictures/s2.jpg";
        refImagePath = "/home/jetson/Documents/AutoTracker/images/lutRef_07.png";
}

void ColorTransfer::init()
{
    cout << "Color Transfer - INIT: " << QThread::currentThreadId() << endl;
    if(!refImage.load(refImagePath))
    {
        qDebug() << "Ref Image load failed: " << refImagePath;
        initialized = false;
        return;
    }
    int channelCount;

    if (refImage.format() == QImage::Format_Grayscale8) { channelCount = 1; }

    else if (refImage.format() == QImage::Format_RGB888 || refImage.format() == QImage::Format_RGB32) { refImage.convertTo(QImage::Format_RGB888); channelCount = 3; }
    else {  qDebug() << "ref image format not correct"; }

    refImage = refImage.scaledToWidth(1280, Qt::SmoothTransformation);
    refImageChannelValues = QVector<QVector<int>>(channelCount, QVector<int>(256,0));
    refImageChannelCFD = QVector<QVector<int>>(channelCount, QVector<int>(256,0));

    //    qDebug() << "refChannelcount" << channelCount;
    //    qDebug() << "refImFormat" << refImage.format();

    for(int i = 0; i < channelCount; i++)
    {
        createImageHistogram(refImage, refImageChannelValues[i], i);
        createCFD(refImageChannelValues[i], refImageChannelCFD[i]);
    }

    initialized = true;
    last_time.start();
}

void ColorTransfer::calcLUT(QImage subImage)
{
    if(subImage.isNull() || !initialized || (last_time.elapsed() < 500) || busy) return;

    eTimer.start();
    busy = true;
    last_time.start();
    int channelCount;

    if(subImage.format() == QImage::Format_RGB888) { channelCount = 3; }
    else { channelCount = 1; }

    QVector<QVector<int>> subImageChannelValues(channelCount, QVector<int>(256,0));
    QVector<QVector<int>> subImageChannelCFD(channelCount, QVector<int>(256,0));
    mappedValues = QVector<QVector<int>>(channelCount, QVector<int>(256,0));

    for(int i = 0; i < channelCount; i++)
    {
        createImageHistogram(subImage, subImageChannelValues[i], i);
        createCFD(subImageChannelValues[i], subImageChannelCFD[i]);
        createHistogramMapping(subImageChannelCFD[i], refImageChannelCFD[i], mappedValues[i]);
        smooth(mappedValues[i]);
    }

    if (channelCount == 3) { emit rgblutReady(mappedValues); }
    else { emit lutReady(mappedValues[0]); }

    avg_processing_time = (avg_processing_time*0.8) + ((eTimer.nsecsElapsed() / 1000000.0f))*0.2f;

    busy = false;
}

void ColorTransfer::createImageHistogram(QImage& img, QVector<int>& histArray, int channel)
{
    if(img.isNull()) return;

    if (img.format() == (QImage::Format_RGB888)) {

        if(channel == 0)
        {
            for(int y = 0; y < img.height(); y++)
            {
                for(int x = 0; x < img.width(); x++)
                {
                    histArray[img.pixelColor(x,y).red()]++;
                }
            }
        }

        if(channel == 1)
        {
            for(int y = 0; y < img.height(); y++)
            {
                for(int x = 0; x < img.width(); x++)
                {
                    histArray[img.pixelColor(x,y).green()]++;
                }
            }
        }

        if(channel == 2)
        {
            for(int y = 0; y < img.height(); y++)
            {
                for(int x = 0; x < img.width(); x++)
                {
                    histArray[img.pixelColor(x,y).blue()]++;
                }
            }
        }
    }

    else {
        for(int y = 0; y < img.height(); y++)
            for(int x = 0; x < img.width(); x++)
                histArray[img.pixelColor(x,y).lightness()]++;
    }
}

void ColorTransfer::createCFD(QVector<int>& histArray, QVector<int>& cfdArray)
{
    if(histArray.size() < 256 || cfdArray.size() < 256)
        return;

    QVector<int> cfValues(256, 0);
    cfValues[0] = histArray[0];
    for(int i = 1; i < histArray.size(); i++){
        cfValues[i] = histArray[i] + cfValues[i-1];
    }

    double max = cfValues.last();
    for(int i = 0; i < cfValues.size(); i++){
        cfdArray[i] = ceil((static_cast<double>(cfValues[i]) / max) * 255.0);
    }
}

void ColorTransfer::createHistogramMapping(QVector<int>& subHist, QVector<int>& targetHist, QVector<int> &mappedHist)
{
    for(int s = 0; s < subHist.size(); s++)
    {
        for(int t = 0; t < targetHist.size(); t++)
        {
            if((targetHist[t] - subHist[s]) >= 0)
            {
                int num = targetHist[t];
                bool lastUpdated = false;
                int i = t;
                for(; i < targetHist.size(); i++)
                {
                    if(targetHist[i] > num)
                    {
                        mappedHist[s] = --i;
                        lastUpdated = true;
                        break;
                    }
                }
                if(i == targetHist.size())
                    mappedHist[s] = 255;

                else if(!lastUpdated)
                    mappedHist[s] = t;
                break;
            }
        }
    }
}

void ColorTransfer::smooth(QVector<int> &values)
{
    if(values.isEmpty() || sWin == 1 || sWin > 65) return;
    int mWin = sWin/2;

    for(int i = 0; i < mWin; i++)
    {
        values.prepend(values[0]);
        values.append(values.last());
    }

    for(int i = mWin; i < (values.size() - mWin); i++)
    {
        float sum = 0;
        for(int w = 0; w < sWin; w++)
        {
            sum += (float)values[i+(w - mWin)];
        }
        values[i] = sum / (float)sWin;
    }

    values = values.mid(mWin, 256);
}
