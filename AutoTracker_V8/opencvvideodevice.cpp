#include "opencvvideodevice.h"
#include <QDebug>

using namespace std;
using namespace cv;

OpenCVVideoDevice::OpenCVVideoDevice(QObject *parent): QObject(parent), capRunning(false), formatContext(nullptr), codecContext(nullptr), frame(nullptr), packet(nullptr), swsContext(nullptr)
{

}

OpenCVVideoDevice::~OpenCVVideoDevice()
{
    capRunning = false;

    if (useRTSP) { closeRTSP(); }
    else if(cap != nullptr)
    {
        if(cap->isOpened()) { cap->release(); }
        delete cap;
    }

    if(pingDeviceThread.isRunning())
    {
        pingDeviceThread.quit();
        pingDeviceThread.wait();
    }
}

void OpenCVVideoDevice::init(int _devType, QByteArray path, QByteArray name, int _apiID)
{
    if ((devType != _devType) && (_devType != -1)) { return; }
    capRunning = false;

    devicePath = path.toStdString();
    deviceName = name.toStdString();
    apiID = _apiID;

    targetSize = QSize(frameWidth, frameHeight);
    target_dt = (1.0f / (double)targetFPS)*0.8f;
    //    qDebug() << "target" << target_dt;

    pingDevice = new PingDevice;
    pingDevice->moveToThread(&pingDeviceThread);

    useRTSP = path.startsWith("rtsp://");
    captureTimer = new QTimer;

    if (useRTSP)
    {
        connect(this, &OpenCVVideoDevice::initPingDevice, pingDevice, &PingDevice::init);
        connect(pingDevice, &PingDevice::pingState, this, &OpenCVVideoDevice::pingDeviceState);
        connect(captureTimer, &QTimer::timeout, this, &OpenCVVideoDevice::captureRTSPFrame);
        pingDeviceThread.start();

        emit initPingDevice("192.168.1.168");
    }

    else
    {
        if(cap != nullptr)
        {
            if(cap->isOpened()) { cap->release(); }
            delete cap;
        }

        cap = new VideoCapture;
        connect(captureTimer, &QTimer::timeout, this, &OpenCVVideoDevice::captureFrame);
    }

    initialized = true;
    emit initCompleted(devType);
}

void OpenCVVideoDevice::pingDeviceState(int ping_state)
{
    //qDebug() << "Ping:" << ping_state;

    if(ping_state) { initRTSP(); }
    else if (rtsp_initialized) { closeRTSP(); }
}

void OpenCVVideoDevice::startCap(int _devType)
{
    if (((devType != _devType) && (_devType != -1)) || !initialized) { return; }

    if(!useRTSP)
    {
        if(cap->isOpened()) { cap->release(); }
        cap->open(devicePath, apiID);
        if(!cap->isOpened()) { qDebug() << "OpenCV Capture failed: " << devicePath.c_str() << deviceName.c_str() << apiID; return; }
        cap->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G') );
        cap->set(cv::CAP_PROP_BUFFERSIZE, 2);
        cap->set(cv::CAP_PROP_FRAME_WIDTH, frameWidth);
        cap->set(cv::CAP_PROP_FRAME_HEIGHT, frameHeight);
        cap->set(cv::CAP_PROP_FPS, targetFPS);
    }

    dtTimer.start();
    enable = capRunning = true;
    captureTimer->start(5);
}

void OpenCVVideoDevice::stopCap(int _devType)
{
    if (((devType != _devType) && (_devType != -1)) || !initialized) { return; }

    capRunning = false;

    if (useRTSP){ closeRTSP(); }
    else if (cap->isOpened()) { cap->release(); }

    captureTimer->stop();
}

void OpenCVVideoDevice::captureFrame()
{
    if(!capRunning || !enable) { return; }
    if(!cap->read(cvframe)) { return; }
    //if(cvframe.empty()) { return; }

    dt = dtTimer.nsecsElapsed() / 1000000000.0f;
    dtTimer.start();
    pTimer.start();
    if(image_post_rotation != NONE_ROTATION) { rotate(cvframe, cvframe, image_post_rotation); }

    cvtColor(cvframe, cvframe, cv::COLOR_BGR2RGB);
    qImageFrame = QImage(cvframe.data, cvframe.cols, cvframe.rows, static_cast<int>(cvframe.step), QImage::Format_RGB888);
    avgCovTime = (avgCovTime*0.9f)+((pTimer.nsecsElapsed() / 1000000.0f)*0.1f);

//    emit frameReady(qImageFrame.copy(), dt, devType, avgCovTime);

    //IF PLAYING VIDEO USE THESE OTHERWISE DONT
    double fps = cap->get(cv::CAP_PROP_FPS);
    int frame_delay = 1000 / fps;
    {
        emit frameReady(qImageFrame.copy(), dt, devType, avgCovTime);
    }
    QThread::msleep(frame_delay);
}

void OpenCVVideoDevice::captureRTSPFrame()
{
    if(!rtsp_initialized) { return; }

    captureTimer->stop();

    if(av_read_frame(formatContext, packet) < 0) { av_packet_unref(packet); captureTimer->start(10); return; }
    if(packet->stream_index != videoStreamIndex) { av_packet_unref(packet); captureTimer->start(10); return; }

    avcodec_send_packet(codecContext, packet);
    if(avcodec_receive_frame(codecContext, frame) == 0)
    {
        dt = dtTimer.nsecsElapsed() / 1000000000.0f;
        dtTimer.start();
        pTimer.start();

        AVFrame* rgbFrame = av_frame_alloc();
        int numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGR24, codecContext->width, codecContext->height, 1);
        uint8_t* buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
        av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, AV_PIX_FMT_BGR24, codecContext->width, codecContext->height, 1);
        sws_scale(swsContext, frame->data, frame->linesize, 0, codecContext->height, rgbFrame->data, rgbFrame->linesize);

        qImageFrame = QImage(rgbFrame->data[0], codecContext->width, codecContext->height, QImage::Format_BGR888);
        qImageFrame.convertTo(QImage::Format_RGB888);
        avgCovTime = (avgCovTime * 0.9f) + ((pTimer.nsecsElapsed() / 1000000.0f) * 0.1f);

        av_free(buffer);
        av_frame_free(&rgbFrame);

        //if(dt >= target_dt)
        {
            emit frameReady(qImageFrame.copy(), dt, devType, avgCovTime);
        }
    }

    av_packet_unref(packet);
    captureTimer->start(10);
}

void OpenCVVideoDevice::initRTSP()
{
    if(rtsp_initialized) { return; }

    qDebug() << "init-RTSP";

    //av_register_all();
    avformat_network_init();

    AVDictionary *dict = NULL; // "create" an empty dictionary
    av_dict_set(&dict, "stimeout", "5000000", 0);

    if (avformat_open_input(&formatContext, devicePath.c_str(), nullptr, &dict) != 0) {
        qDebug() << "Unable to open input";
        return;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        qDebug() << "Unable to find stream info";
        return;
    }

    videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        qDebug() << "Unable to find video stream";
        return;
    }

    codecContext = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(codecContext, formatContext->streams[videoStreamIndex]->codecpar);
    codec = avcodec_find_decoder(codecContext->codec_id);

    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        qDebug() << "Unable to open codec";
        return;
    }

    frame = av_frame_alloc();
    packet = av_packet_alloc();

    swsContext = sws_getContext(
        codecContext->width, codecContext->height, codecContext->pix_fmt,
        codecContext->width, codecContext->height, AV_PIX_FMT_BGR24,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    rtsp_initialized = initialized = true;
}

void OpenCVVideoDevice::closeRTSP()
{
    if (formatContext) { avformat_close_input(&formatContext); }
    if (codecContext) { avcodec_free_context(&codecContext); }
    if (frame) { av_frame_free(&frame); }
    if (packet) { av_packet_free(&packet); }
    if (swsContext) { sws_freeContext(swsContext); }

    rtsp_initialized = false;
}

