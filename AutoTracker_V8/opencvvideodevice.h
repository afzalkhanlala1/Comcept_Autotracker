#ifndef OPENCVVIDEODEVICE_H
#define OPENCVVIDEODEVICE_H

#include "app_includes.h"
#include "pingdevice.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
}

enum IMAGE_POST_ROTATION { NONE_ROTATION = -1, IMAGE_ROTATE_90_CLOCKWISE, IMAGE_ROTATE_180, IMAGE_ROTATE_90_COUNTERCLOCKWISE };

class OpenCVVideoDevice : public QObject
{
    Q_OBJECT
public:
    explicit OpenCVVideoDevice(QObject *parent = nullptr);
    ~OpenCVVideoDevice();

    cv::VideoCapture *cap = nullptr;
    cv::Mat cvframe;
    cv::Mat scaledFrame;
    QImage qImageFrame;
    QElapsedTimer dtTimer;
    QElapsedTimer pTimer;
    QTimer *captureTimer = nullptr;

    PingDevice *pingDevice = nullptr;
    QThread pingDeviceThread;

    bool useRTSP = true;
    bool initialized = false;
    bool rtsp_initialized = false;
    bool enable = false;
    bool capRunning = false;

    int frameWidth = 1920;
    int frameHeight = 1080;
    int targetFPS = 30;
    double target_dt = 0.030;
    QSize targetSize = QSize(frameWidth, frameHeight);
    bool greyScale = false;

    std::string devicePath = "";
    std::string deviceName = "";
    int apiID = cv::CAP_ANY;
    int devType = 0;
    double dt = 0.0;
    double avgCovTime = 0.0;

    IMAGE_POST_ROTATION image_post_rotation = IMAGE_POST_ROTATION::NONE_ROTATION;

public slots:
    void init(int _devType, QByteArray path = "", QByteArray name = "", int _apiID = cv::CAP_ANY);
    void pingDeviceState(int ping_state);
    void startCap(int _devType);
    void stopCap(int _devType);
    void captureFrame();
    void captureRTSPFrame();

signals:
    void frameReady(QImage frameImg, double _frame_dt, int devType, double avgCovTime);
    void initPingDevice(QString ip_addr);
    void initCompleted(int my_id);

private:

    void initRTSP();
    void closeRTSP();

    AVFormatContext *formatContext = nullptr;
    AVCodecContext *codecContext = nullptr;
    AVFrame *frame = nullptr;
    AVPacket *packet = nullptr;
    SwsContext *swsContext = nullptr;
    int videoStreamIndex = -1;
    const AVCodec *codec = nullptr;
};

#endif // OPENCVVIDEODEVICE_H
