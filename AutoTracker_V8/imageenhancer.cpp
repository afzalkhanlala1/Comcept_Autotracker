#include "imageenhancer.h"

using namespace std;
using namespace cv;

ImageEnhancer::ImageEnhancer(QObject *parent) : QObject{parent}
{

}

ImageEnhancer::~ImageEnhancer()
{
    colorTransferThread.quit();
    colorTransferThread.wait();
}

void ImageEnhancer::init()
{
    qDebug() << "Image Enhancer - INIT:" << QThread::currentThreadId();

    colorTransfer = new ColorTransfer;
    colorTransfer->moveToThread(&colorTransferThread);
    colorTransferThread.start(QThread::HighPriority);

    connect(this, &ImageEnhancer::initColorTransfer, colorTransfer, &ColorTransfer::init);
    connect(this, &ImageEnhancer::processLut, colorTransfer, &ColorTransfer::calcLUT);

    connect(colorTransfer, &ColorTransfer::rgblutReady, this, &ImageEnhancer::rgblutUpdated);
    connect(colorTransfer, &ColorTransfer::lutReady, this, &ImageEnhancer::lutUpdated);

#ifdef USE_GPU
    convolver = cuda::createConvolution(Size(3, 3));
    clahe = cuda::createCLAHE(24, cv::Size(24, 24));
    //clahe->setClipLimit(16);
#else
    clahe = createCLAHE(24, Size(24, 24));
#endif

    //sharpen_kernel = (Mat_<float>(3,3) << 0, -0.5, 0, -0.5, 3, -0.5, 0, -0.5, 0);
    sharpen_kernel = (Mat_<float>(3,3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
    //sharpen_kernel = (Mat_<float>(3,3) << 0, -2, 0, -2, 8.8, -2, 0, -2, 0);
    edge_kernel = (Mat_<float>(3,3) << -1, -1, -1, -1, 8, -1, -1, -1, -1);
    emboss_kernel = (Mat_<float>(3,3) << -1, -0.5, 0, -0.5, 0.5, 0.5, 0, 0.5, 1.6);
    blur_kernel = (Mat_<float>(3,3) << 0.0625, 0.125, 0.0625, 0.125, 0.25, 0.125, 0.0625, 0.125, 0.0625);

    emit initColorTransfer();
    initLUT();
    initrgbLUT();

    initialized = true;
}

void ImageEnhancer::processImage(TrackerFrame trackerFrame, double frame_dt)
{
    QImage frameImg = trackerFrame.frame;
    if(frameImg.isNull()) { qDebug() << "IE - Null frame"; return; }
    pTimer.start();

//    resizeAndConvImage(frameImg);

    emit processLut(frameImg);

    if(frameImg.format() == QImage::Format_RGB888)  {
        inFrame = cv::Mat(frameImg.height(), frameImg.width(), CV_8UC3, (void*)frameImg.constBits(), frameImg.bytesPerLine());
        lutFrame = cv::Mat(frameImg.height(), frameImg.width(), CV_8UC3);
    }
    else {
        inFrame = cv::Mat(frameImg.height(), frameImg.width(), CV_8UC1, (void*)frameImg.constBits(), frameImg.bytesPerLine());
        lutFrame = cv::Mat(frameImg.height(), frameImg.width(), CV_8UC1);
    }

    if(applyPreNR) { removeNoiseGPU(inFrame, noiseH, noiseSearchWindow, noiseBlockSize); }

    if(frameImg.format() == QImage::Format_RGB888)  { imagergbApplyLut(inFrame, lutFrame); }
    else { imageApplyLut(inFrame, lutFrame); }

    if(frameImg.format() == QImage::Format_Grayscale8){
        imageApplyCLAHE(lutFrame, lutFrame);
        applyConvolutionFilters(lutFrame);
        applyAdaptiveThreshold(lutFrame);
    }

    if(applyPostNR) { removeNoiseGPU(lutFrame, noiseH, noiseSearchWindow, noiseBlockSize); }

    if(frameImg.format() == QImage::Format_RGB888)  {
        trackerFrame.lutFrame = QImage(lutFrame.data, lutFrame.cols, lutFrame.rows, static_cast<int>(lutFrame.step), QImage::Format_RGB888).copy();
    }
    else {
        QImage outImage = QImage(lutFrame.data, lutFrame.cols, lutFrame.rows, static_cast<int>(lutFrame.step), QImage::Format_Grayscale8);
        outImage.scaled(640, 480, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        trackerFrame.lutFrame = outImage.copy();
    }
    blendWithOriginalImage();

    trackerFrame.useLutFrame = applyLuts || applyCLAHE;
    trackerFrame.dt = frame_dt;
    pTime = ((pTime*0.9f) + ((pTimer.nsecsElapsed() / 1000000.0))*0.1f);

//    qDebug() << trackerFrame.lutFrame.width() <<  trackerFrame.lutFrame.height() ;
    emit frameReady(trackerFrame);
}

void ImageEnhancer::resizeAndConvImage(QImage &frameImg)
{
        if( ((float)frameImg.height() / (float)frameImg.width()) != (3.0f/4.0f) )
        {
            if((frameImg.height() != 480)){ frameImg = frameImg.scaledToHeight(480, Qt::SmoothTransformation); }

            float imgw = 640.0f;
            int x_offset = (frameImg.width() - imgw) * 0.5f;
            frameImg = frameImg.copy(x_offset, 0, imgw, frameImg.height());
        }

        else { frameImg = frameImg.scaled(640, 480, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).copy(); }

        frameImg.convertTo(QImage::Format_Grayscale8);
}

void ImageEnhancer::imagergbApplyLut(cv::Mat &inFrame, cv::Mat &lutFrame)
{
    if(!applyLuts)
    {
        inFrame.copyTo(lutFrame);
        return;
    }

    float n_lut_lpf = 1.0f - lut_lpf;
    float n_lut = 1.0f - lutStrength;

    for(int i = 0; i < lut_channelCount; i++)
    {
        for(int c = 0; c < 256; c++)
        {
            float lv = (rgbcurrLutArr_f[i][c]*lut_lpf) + (rgblutArr_f[i][c]*n_lut_lpf);
            rgbcurrLutArr_f[i][c] = (lv*lutStrength) + (rgboriLutArr[i][c]*n_lut);
            rgbcurrLutArr.at<uchar>(i, c) = rgbcurrLutArr_f[i][c];
        }
    }

#ifdef USE_GPU
    lut = cuda::createLookUpTable(rgbcurrLutArr);
    if(!lut.empty())
    {
        lut->transform(inFrame, lutFrame);
    }
#else
    if(!rgbcurrLutArr.empty())
        //        qDebug() << "F";
        cv::LUT(inFrame, rgbcurrLutArr, lutFrame);
#endif
}


void ImageEnhancer::imageApplyLut(cv::Mat &inFrame, cv::Mat &lutFrame)
{
    if(!applyLuts)
    {
        inFrame.copyTo(lutFrame);
        return;
    }

    float n_lut_lpf = 1.0f - lut_lpf;
    float n_lut = 1.0f - lutStrength;
    for(int i = 0; i < 256; i++)
    {
        float lv = (currLutArr_f[i]*lut_lpf) + (lutArr_f[i]*n_lut_lpf);
        currLutArr_f[i] = (lv*lutStrength) + (oriLutArr[i]*n_lut);
        currLutArr.at<uchar>(0, i) = currLutArr_f[i];
    }

#ifdef USE_GPU
    lut = cuda::createLookUpTable(currLutArr);
    if(!lut.empty())
        lut->transform(inFrame, lutFrame);
#else
    if(!currLutArr.empty())
        cv::LUT(inFrame, currLutArr, lutFrame);
#endif
}

void ImageEnhancer::imageApplyCLAHE(cv::Mat &inFrame, cv::Mat &lutFrame)
{
    if(!applyCLAHE || updatingCLAHE)
    {
        return;
    }

    Mat claheFrame(inFrame.rows, inFrame.cols, inFrame.type());
#ifdef USE_GPU
    cuda::GpuMat imageGPU;
    cuda::GpuMat resultGPU;

    imageGPU.upload(inFrame);
    clahe->apply(imageGPU, resultGPU);
    resultGPU.download(claheFrame);
#else
    clahe->apply(inFrame, claheFrame);
#endif
    addWeighted(lutFrame, lut_clahe_factor, claheFrame, 1.0-lut_clahe_factor, 0.0, lutFrame);
}

void ImageEnhancer::applyAdaptiveThreshold(cv::Mat &lutFrame)
{
    if(!enableAdaptiveThresh) { return; }
    cv::adaptiveThreshold(lutFrame, lutFrame, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::ThresholdTypes::THRESH_BINARY, 21, 0);
}

void ImageEnhancer::removeNoiseGPU(cv::Mat &inFrame, float _noiseH, int _noiseSearchWindow, int _noiseBlockSize)
{
#ifdef USE_GPU
    cuda::GpuMat imageGPU;
    cuda::GpuMat resultGPU;

    imageGPU.upload(inFrame);
    cuda::fastNlMeansDenoising(imageGPU, resultGPU, _noiseH, _noiseSearchWindow, _noiseBlockSize);
    resultGPU.download(inFrame);
#else
    cv::fastNlMeansDenoising(inFrame, inFrame, _noiseH, _noiseSearchWindow, _noiseBlockSize);
#endif
}

void ImageEnhancer::applyConvolutionFilters(cv::Mat &inFrame)
{
    if(!enableEdge && !enableEmboss && !enableSharpen) { return; }
    inFrame.convertTo(inFrame, CV_32FC1);
//    inFrame.convertTo(inFrame, CV_32FC3);

#ifdef USE_GPU
    if(enableEdge) { convolver->convolve(inFrame, edge_kernel, inFrame, true); }
    if(enableEmboss) { convolver->convolve(inFrame, emboss_kernel, inFrame, true); }
    if(enableSharpen) { convolver->convolve(inFrame, sharpen_kernel, inFrame, true); }
#else
    //filter2D(image, result, -1, kernel, Point(-1, -1), 0, BORDER_DEFAULT);
    if(enableEdge) { filter2D(inFrame, inFrame, -1, edge_kernel, Point(-1, -1), 0, BORDER_DEFAULT); }
    if(enableEmboss) { filter2D(inFrame, inFrame, -1, emboss_kernel, Point(-1, -1), 0, BORDER_DEFAULT); }
    if(enableSharpen) { filter2D(inFrame, inFrame, -1, sharpen_kernel, Point(-1, -1), 0, BORDER_DEFAULT); }
#endif
    //    inFrame.convertTo(inFrame, CV_8UC1);
    inFrame.convertTo(inFrame, QImage::Format_RGB888);
    //    onvertTo(QImage::Format_Grayscale8)
}

void ImageEnhancer::blendWithOriginalImage()
{
    if(effectStrength > 0.99) return;

    QPainter painter(&trackerFrame.lutFrame);
    painter.setOpacity(1.0 - effectStrength);
    painter.drawImage(0, 0, trackerFrame.frame);
}

//--------------------------

void ImageEnhancer::initLUT()
{
    lutArr_f = QVector<float>(256, 0);
    currLutArr_f = QVector<float>(256, 0);
    oriLutArr = QVector<float>(256, 0);
    for(int i = 0; i < 256; i++)
    {
        currLutArr.at<uchar>(0, i) = lutArr.at<uchar>(0, i) = i;
        oriLutArr[i] = currLutArr_f[i] = lutArr_f[i] = i;
    }
}

void ImageEnhancer::initrgbLUT()
{
    rgblutArr_f = QVector<QVector<float>>(lut_channelCount, QVector<float>(256,0.0f));
    rgbcurrLutArr_f = QVector<QVector<float>>(lut_channelCount, QVector<float>(256,0.0f));
    rgboriLutArr = QVector<QVector<float>>(lut_channelCount, QVector<float>(256,0.0f));

    for(int i = 0; i < lut_channelCount; i++)
    {
        for(int c = 0; c < 256; c++)
        {
            rgbcurrLutArr.at<uchar>(i, c) = rgblutArr.at<uchar>(i, c) = c;
            rgboriLutArr[i][c] = rgbcurrLutArr_f[i][c] = rgblutArr_f[i][c] = c;
        }
    }
}


void ImageEnhancer::rgblutUpdated(QVector<QVector<int>> rgbnewLut)
{
    for(int i = 0; i < lut_channelCount; i++)
    {
        for(int c = 0; c < 256; c++)
        {
            rgblutArr_f[i][c] = rgblutArr.at<uchar>(i, c) = rgbnewLut[i][c];
        }
    }
}

void ImageEnhancer::lutUpdated(QVector<int> newLut)
{
    for(int i = 0; i < newLut.size(); i++)
    {
        lutArr_f[i] = lutArr.at<uchar>(0, i) = newLut[i];
    }
}

void ImageEnhancer::updateCLAHE(int gridRows, int gridCols, double clipLimit)
{
    updatingCLAHE = true;
#ifdef USE_GPU
    clahe = cuda::createCLAHE(clipLimit, cv::Size(gridRows, gridCols));
#else
    clahe = createCLAHE(clipLimit, cv::Size(gridRows, gridCols));
#endif
    updatingCLAHE = false;
}
