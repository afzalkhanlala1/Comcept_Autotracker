#ifndef OPENCVVIDEOCAPTURE_H
#define OPENCVVIDEOCAPTURE_H

#include "app_includes.h"
#include "opencvvideodevice.h"

enum CAP_DEV { DAY_SIGHT = 0, THERMAL_SIGHT, WIDE_DAY_SIGHT, _CAMERA_COUNT };

class OpenCVVideoCapture : public QObject
{
    Q_OBJECT
public:
    explicit OpenCVVideoCapture(QObject *parent = nullptr);
    ~OpenCVVideoCapture();

    QVector<OpenCVVideoDevice*> capDev = QVector<OpenCVVideoDevice*>(_CAMERA_COUNT, nullptr);
    QVector<QImage*> devImage = QVector<QImage*>(_CAMERA_COUNT, nullptr);
    QVector<QThread*> capDevThreads = QVector<QThread*>(_CAMERA_COUNT, nullptr);

    CapDeviceConfig deviceConfig;

    QByteArrayList deviceList;
    bool initialized = false;
    double fps = 24.0f;
    double convertTime = -1.0f;

    //        QSize screenSize = QSize(16, 9);
    QSize screenSize = QSize(1354, 1016);
    //    QSize screenSize = QSize(1920, 1080); //its just a ratio..we can write 16:9 here as well ..it is same thing...it is making it rect if img is square wide or etc
    QSize clientScreenSize = QSize(960, 540); //suppose in REMOTE control..it is screen ratio of that

    QElapsedTimer captureTimer;
    double captureTime = 0.0f;

    QImage currFrame;
    cv::Mat ocvFrame;
    cv::Mat scaledFrame;
    cv::Mat screenSizedFrame;
    cv::Mat clientScreenSizedFrame;

    QRectF imagePreviewRect;

    QTimer *devTimer = nullptr;

    bool showTeleView = false;
    float teleViewZoom = 3.0f;

    int widthTraget = 640, heightTraget = 480;

//    QVector<CAP_DEV> selectedDev = { CAP_DEV::DAY_SIGHT, CAP_DEV::THERMAL_SIGHT };

    CAP_DEV selectedDev = CAP_DEV::DAY_SIGHT;
//    CAP_DEV selectedDev = CAP_DEV::THERMAL_SIGHT;
    float clientScreenRatio = 16.0f / 9.0f;
    float screenRatio = 16.0f / 9.0f;
    float imgRatio = 4.0f / 3.0f;

    double h_pixels_2_deg = _H_AOV / 1920.0f, v_pixels_2_deg = _V_AOV / 1080.0f;
    double pd_x_gain = 1.0252f, pd_y_gain = 1.0452f;
    double ph_err_dir = 1.0f, pv_err_dir = 1.0f;
    double azi_corr_deg = 0.0f, elev_corr_deg = 0.0f;
    double azi_corr_px = 0.0f, elev_corr_px = 0.0f;
    double azi_corr_px_ani = 0.0f, elev_corr_px_ani = 0.0f;
    bool azi_elev_corr_act = false;

    int y_center_offset = 0;
    int x_center_offset = 0;

    int dev_init_count = 0;

public slots:
    void init();
    void deviceInitCompleted(int _id);
    void setCaptureDevices(QByteArrayList _deviceList = QByteArrayList(), QSize targetFrameSize = QSize());
    void setCaptureDevicesByName(QByteArrayList _deviceList);
    void devFrameReady(QImage _frameImg, double _frame_dt, int devType = 0, double processTime = -1.0f);
    void startCapture(int deviceID = -1);
    void stopCapture(int deviceID = -1);
    void terminateCapture();
    void setScreenSize(QSize _screenSize);
    void setClientDisplaySize(QSize _clientScreenSize);
    void checkDevices();

    void setActiveDevice(int _activeDev);
    void setCapturePreviewSizes(QRectF imgRect);

signals:
    void initCompleted();
    void initDevice(int _devType, QByteArray path = "", QByteArray name = "", int _apiID = cv::CAP_ANY);
    //    void trackerFrameReady(TrackerFrame trackerFrame);
    void trackerFrameReady(TrackerFrame trackerFrame, double frame_dt);
    void frameReady(QImage _frameImg, double _frame_dt, int devType = 0);
    void processRetical(QImage frameImg, int _tw, int _th);
    void startDev(int deviceID);
    void stopDev(int deviceID);

private:

    inline double getHoriDegToPixels(double _dx) { return (_dx / (h_pixels_2_deg * pd_x_gain)) * ph_err_dir; }
    inline double getVertDegToPixels(double _dy) { return (_dy / (v_pixels_2_deg * pd_y_gain)) * pv_err_dir; }
};

#endif // OPENCVVIDEOCAPTURE_H
