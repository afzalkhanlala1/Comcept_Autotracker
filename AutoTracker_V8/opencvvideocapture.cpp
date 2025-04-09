#include "opencvvideocapture.h"

using namespace std;
using namespace cv;

OpenCVVideoCapture::OpenCVVideoCapture(QObject *parent) : QObject{parent}
{

}

OpenCVVideoCapture::~OpenCVVideoCapture()
{
    stopCapture(-1);

    for(int i = 0; i < _CAMERA_COUNT; i++)
    {
        if(capDevThreads[i] != nullptr)
        {
            if(capDevThreads[i]->isRunning())
            {
                capDevThreads[i]->quit();
                capDevThreads[i]->wait(4000);
            }
        }
    }
}

void OpenCVVideoCapture::init()
{
    qDebug() << "Opencv Video Capture - INIT: " << QThread::currentThreadId();

    for(int i = 0; i < _CAMERA_COUNT; i++)
    {
        capDev[i] = new OpenCVVideoDevice;
        capDevThreads[i] = new QThread;
        devImage[i] = new QImage;

        capDev[i]->devType = i;
        capDev[i]->moveToThread(capDevThreads[i]);
        capDevThreads[i]->start(QThread::HighPriority);

        connect(this, &OpenCVVideoCapture::initDevice, capDev[i], &OpenCVVideoDevice::init);
        connect(this, &OpenCVVideoCapture::startDev, capDev[i], &OpenCVVideoDevice::startCap);
        connect(this, &OpenCVVideoCapture::stopDev, capDev[i], &OpenCVVideoDevice::stopCap);
        connect(capDev[i], &OpenCVVideoDevice::frameReady, this, &OpenCVVideoCapture::devFrameReady);
        connect(capDev[i], &OpenCVVideoDevice::initCompleted, this, &OpenCVVideoCapture::deviceInitCompleted);
    }

    //uncomment this for thermal sight
//    capDev[THERMAL_SIGHT]->image_post_rotation = IMAGE_POST_ROTATION::IMAGE_ROTATE_90_COUNTERCLOCKWISE;

    devTimer = new QTimer;
    connect(devTimer, &QTimer::timeout, this, &OpenCVVideoCapture::checkDevices);
    initialized = true;

    emit initCompleted();
}

void OpenCVVideoCapture::deviceInitCompleted(int _id)
{
    if(++dev_init_count >= deviceList.size())
    {
        startCapture(-1);
        setActiveDevice(selectedDev);
    }
}

void OpenCVVideoCapture::setCaptureDevices(QByteArrayList _deviceList, QSize targetFrameSize)
{
    dev_init_count = 0;
    if(_deviceList.isEmpty()) { if(deviceList.isEmpty()) { return; } }
    else { deviceList = _deviceList; }

    for(int i = 0; (i < _CAMERA_COUNT) && (i < deviceList.size()); i++)
    {
        if(!targetFrameSize.isEmpty()) { capDev[i]->frameWidth = targetFrameSize.width(); capDev[i]->frameHeight = targetFrameSize.height(); }
        emit initDevice(i, deviceList[i], "", cv::CAP_ANY);
    }
    //    qDebug() << "tw" << targetFrameSize.width()  << "th" << targetFrameSize.height(); ;
}

void OpenCVVideoCapture::setCaptureDevicesByName(QByteArrayList _deviceList)
{
    deviceList = _deviceList;

    if(deviceList.isEmpty())
    {
        devTimer->stop();
        terminateCapture();
        return;
    }

    terminateCapture();

    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    for(int i = 0; (i < _CAMERA_COUNT) && (i < deviceList.size()); i++)
    {
        for(int c = 0; c < cameras.size(); c++)
        {
            if(cameras[c].description().contains(deviceList[i], Qt::CaseInsensitive))
            {
                if(deviceList[i].contains("AFN_Cap"))
                {
                    capDev[i]->frameWidth = 1024;
                    capDev[i]->frameWidth = 768;
                }

                capDev[i]->init(i, cameras[c].deviceName().toStdString().c_str(), cameras[c].description().toStdString().c_str(), cv::CAP_ANY);
            }
        }
    }

    startCapture(-1);
    devTimer->start(500);
}

void OpenCVVideoCapture::checkDevices()
{
    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    for(int i = 0; (i < _CAMERA_COUNT) && (i < deviceList.size()); i++)
    {
        int c_i = -1;
        for(int c = 0; c < cameras.size(); c++)
        {
            if(cameras[c].description().contains(capDev[i]->deviceName.c_str(), Qt::CaseInsensitive))
            {
                c_i = c;
                break;
            }
        }

        if(c_i == -1)
        {
            capDev[i]->stopCap(-1);
        }

        else
        {
            if(capDev[i]->enable && !capDev[i]->capRunning)
            {
                capDev[i]->init(i, cameras[c_i].deviceName().toStdString().c_str(), cameras[c_i].description().toStdString().c_str(), cv::CAP_V4L2);
                emit startDev(i);
            }
        }
    }
}

void OpenCVVideoCapture::setActiveDevice(int _activeDev)
{
    if(_activeDev == selectedDev) { startCapture(_activeDev); return; }

    stopCapture(selectedDev);
    startCapture(_activeDev);
    selectedDev = (CAP_DEV)_activeDev;
}

void OpenCVVideoCapture::setCapturePreviewSizes(QRectF imgRect)
{
    qDebug() << "ImagePreviewRect: " << imgRect.width() << ", " << imgRect.height() << endl;
    imagePreviewRect = imgRect;
    screenSize = imgRect.toRect().size();
}

void OpenCVVideoCapture::startCapture(int deviceID)
{
    emit startDev(deviceID);
}

void OpenCVVideoCapture::stopCapture(int deviceID)
{
    emit stopDev(deviceID);
}

void OpenCVVideoCapture::terminateCapture()
{
    emit stopDev(-1);
}

void OpenCVVideoCapture::setScreenSize(QSize _screenSize)
{
    screenSize = _screenSize;
}

void OpenCVVideoCapture::setClientDisplaySize(QSize _clientScreenSize)
{
    clientScreenSize = _clientScreenSize;
}

void OpenCVVideoCapture::devFrameReady(QImage _frameImg, double _frame_dt, int devType, double processTime)
{
    //qDebug() << devType << " | " << _frame_dt << " | " << processTime;

    if(_frameImg.isNull() || screenSize.isNull() || devType != (int)selectedDev) { return; }

    captureTimer.start();
    fps = (fps*0.9f) + ((1.0f/_frame_dt)*0.1f);
    //    fps = 1;
    //        qDebug() << "fps" << fps;
    convertTime = (convertTime*0.95f) + (processTime*0.05f);

    // if(devType == (int)CAP_DEV::THERMAL_SIGHT)
    // {
    //     _frameImg = _frameImg.copy(_frameImg.width()*0.05, _frameImg.height()*0.05, _frameImg.width()*0.9, _frameImg.height()*0.9);
    // }

    screenRatio = (float)screenSize.width() / (float)screenSize.height();
    clientScreenRatio = (float)clientScreenSize.width() / (float)clientScreenSize.height();
    imgRatio = (float)_frameImg.width() / (float)_frameImg.height();

    //    qDebug() << _frameImg.width() <<  _frameImg.height() ;
    //    qDebug() << "sr" << screenRatio << "imgRA" << imgRatio;

    if(clientScreenRatio < imgRatio)
    {
        int img_w = _frameImg.height()*clientScreenRatio;
        int x_offset = (float)(_frameImg.width() - img_w)*0.5f;
        x_offset += x_center_offset;
        currFrame = _frameImg.copy(x_offset, y_center_offset, img_w, _frameImg.height());
    }

    else if(clientScreenRatio > imgRatio)
    {
        int img_h = _frameImg.width()/clientScreenRatio;
        int y_offset = (float)(_frameImg.height() - img_h)*0.5f;
        y_offset += y_center_offset;
        currFrame = _frameImg.copy(x_center_offset, y_offset, _frameImg.width(), img_h);
    }

    else { currFrame = _frameImg; }

    cv::Size sz(clientScreenSize.width(), clientScreenSize.height());
    if(sz.empty()) { return; }

    TrackerFrame tf;

    if (deviceConfig.colorSpace[devType] == GRAY8B_CS) { currFrame.convertTo(QImage::Format_Grayscale8);  ocvFrame = cv::Mat(currFrame.height(), currFrame.width(), CV_8UC1, (void*)currFrame.constBits(), currFrame.bytesPerLine());}
    else  {currFrame.convertTo(QImage::Format_RGB888); ocvFrame = cv::Mat(currFrame.height(), currFrame.width(), CV_8UC3, (void*)currFrame.constBits(), currFrame.bytesPerLine());}

    //    qDebug() << "form" << currFrame.format();
    //    ocvFrame = cv::Mat(currFrame.height(), currFrame.width(), CV_8UC3, (void*)currFrame.constBits(), currFrame.bytesPerLine());

    if(currFrame.size() != clientScreenSize)
    {
        cv::resize(ocvFrame, clientScreenSizedFrame, cv::Size(clientScreenSize.width(), clientScreenSize.height()));

        if (currFrame.format() == QImage::Format_RGB888) {
            tf.scaledFrame = QImage(clientScreenSizedFrame.data, clientScreenSizedFrame.cols, clientScreenSizedFrame.rows, static_cast<int>(clientScreenSizedFrame.step), QImage::Format_RGB888).copy();
        }
        else {
            tf.scaledFrame = QImage(clientScreenSizedFrame.data, clientScreenSizedFrame.cols, clientScreenSizedFrame.rows, static_cast<int>(clientScreenSizedFrame.step), QImage::Format_Grayscale8).copy();
        }
    }
    else { tf.scaledFrame = currFrame.copy(); }

    cv::resize(ocvFrame, screenSizedFrame, cv::Size(screenSize.width(), screenSize.height()));

    if (deviceConfig.colorSpace[devType] == GRAY8B_CS)
    { tf.frame = QImage(screenSizedFrame.data, screenSizedFrame.cols, screenSizedFrame.rows, static_cast<int>(screenSizedFrame.step), QImage::Format_Grayscale8).copy(); }
    else {
        tf.frame = QImage(screenSizedFrame.data, screenSizedFrame.cols, screenSizedFrame.rows, static_cast<int>(screenSizedFrame.step), QImage::Format_RGB888).copy(); }

    //    tf.frame = QImage(screenSizedFrame.data, screenSizedFrame.cols, screenSizedFrame.rows, static_cast<int>(screenSizedFrame.step), QImage::Format_Grayscale16).copy();
    //tf.frame = tf.scaledFrame.scaled(screenSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    tf.dt = _frame_dt;
    tf.devType = devType;
    tf.useLutFrame = false;

    tf.teleViewZoom = teleViewZoom;
    tf.showTeleView = showTeleView;
    tf.client_server_scale = (float)screenSize.height() / (float)clientScreenSize.height();

    if(showTeleView)
    {
        int w = (float)tf.frame.width()*0.35f;
        int x = tf.frame.width()-w;
        int y = 0;
        tf.teleViewRect = QRect(x, y, w, w);

        int sw = (float)tf.teleViewRect.width() / teleViewZoom;
        int sh = (float)tf.teleViewRect.height() / teleViewZoom;
        int winX = ((tf.frame.width() - sw) * 0.5f);
        int winY = ((tf.frame.height() - sh) * 0.5f);

        tf.teleWindow = QRect(winX, winY, sw, sh);
        tf.teleImage = tf.frame.copy(tf.teleWindow).scaled(tf.teleViewRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        //painter->drawImage(teleViewRect.x(), teleViewRect.y(), teleImage);
    }

    //    qDebug() << tf.frame.width() <<  tf.frame.height() ;

    emit trackerFrameReady(tf, tf.dt);
    emit processRetical(tf.scaledFrame, tf.scaledFrame.width(), tf.scaledFrame.height());

    captureTime = captureTime*0.9f + (((double)captureTimer.nsecsElapsed()/1000000.0f)*0.1f);

    //qDebug() << frameImg.size() << " | " << 1.0f/_frame_dt << " | " << convertTime;
}


