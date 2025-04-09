#ifndef IMAGESTABILIZATION_H
#define IMAGESTABILIZATION_H

#include "app_includes.h"
#include "kalmanopticalflowstab.h"
#include "imustab.h"

class ImageStabilization : public QObject
{
    Q_OBJECT
public:
    explicit ImageStabilization(QObject *parent = nullptr);

    bool use2Passes = false;
    KalmanOpticalFlowStab *kalman_opStab = nullptr;
    KalmanOpticalFlowStab *kalman_opStab_2Pass = nullptr;
    IMUStab *imuStab = nullptr;

    QImage stabImage;
    QImage inputFrame;
    float win_h = 64.0f;
    float win_v = 48.0f;
    QSizeF imageSize = QSizeF(640, 480);
    bool initialized = false;
    bool enabled = true;
    bool firstFrame = true;
    bool useIMU = true;
    bool useOpticalFlow = true;

    cv::Mat cur_grey, cur_grey_2;
    cv::Mat prev_grey, prev_grey_2;
    float x_offset = 0.0f, y_offset = 0.0f;
    float win_center_x = 0.0f, win_center_y = 0.0f;

    QElapsedTimer *pTimer;
    double pTime = 0;
    QVector<QRectF> targetRects;
    int activeTarget = 0;

    bool useMask = true;
    float maskVMargin = 0.09f;
    float maskHMargin = 0.0f;
    cv::Mat marginMask;
    cv::Mat mask;

    float op_corr_angle = 0.0f, corr_angle = 0.0f, imu_corr_angle = 0.0f;
    QPointF cent_err;
    QRectF anchorRect, stabWin;
    QRectF currStabWin, imuCurrStabWin, opCurrStabWin, lastOPCurrStabWin;
    QVector3D opMotion;
    QVector3D imuMotion;
    QVector3D imageMotion;
    QImage imuImage;

    bool useFrameSampling = false;
    QImage f_stabImage;
    QVector<float> imuData;
    QVector<float> sumImuData;
    QVector<float> avgImuData;
    float imuDataCount = 0.0f;
    float frameSamplingRate = 0.4f;

#ifdef USE_GPU
    cv::Ptr<cv::cuda::BackgroundSubtractorMOG2> backSub;
#else
    cv::Ptr<cv::BackgroundSubtractorMOG2> backSub;
#endif

public slots:
    void init();
    void imuIncomingData(QVector<float> _imuData);
    void stabilizeFrame(TrackerFrame trackerFrame);
    void setMaxAmplitude(float _win_h, float _win_v);
    void centerWindow();
    void setTargetRegions(QVector<QRectF> _targetRects, int _activeTarget);
    void setEnabled(bool val);

signals:
    void initCompleted();
    void frameStabilized(TrackerFrame trackerFrame);
    void worldPosition(double x, double y);

private:
    inline void createMask();
    inline void subtractBackgound(TrackerFrame &trackerFrame);
    inline void updateFrameSampling(TrackerFrame &trackerFrame, QImage &image);
};

#endif // IMAGESTABILIZATION_H
