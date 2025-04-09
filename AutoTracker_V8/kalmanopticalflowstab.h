#ifndef KALMANOPTICALFLOWSTAB_H
#define KALMANOPTICALFLOWSTAB_H

#include "app_includes.h"

class KalmanOpticalFlowStab : public QObject
{
    Q_OBJECT
public:
    explicit KalmanOpticalFlowStab(QObject *parent = nullptr);

    int id = 0;

#ifdef USE_GPU
    cv::Ptr<cv::cuda::SparsePyrLKOpticalFlow> opFlow;
    cv::Ptr<cv::cuda::CornersDetector> detector;
#else
    cv::Ptr<cv::SparsePyrLKOpticalFlow> opFlow;
#endif

    float win_h = 64.0f;
    float win_v = 48.0f;
    QSizeF imageSize = QSizeF(640, 480);
    QRectF stab_win = QRectF(0, 0, 640, 480);
    QVector2D worldDisplacment;
    QRectF centerRect;

    float x_offset = 0.0f, y_offset = 0.0f, a_offset = 0.0f;
    float last_x_offset = 0.0f, last_y_offset = 0.0f, last_a_offset = 0.0f;
    float win_center_x = 0.0f, win_center_y = 0.0f, win_center_a = 0.0f;

    float _x_move = 0.0f, _y_move = 0.0f, _a_move = 0.0f;
    float last_x_move = 0.0f, last_y_move = 0.0f, last_a_move = 0.0f;
    float x_move = 0.0f, y_move = 0.0f, a_move;

    bool enabled = false;
    bool initialized = false;

    double a = 0;
    double x = 0;
    double y = 0;
    int k = 1;

    float corr_x = 0.0f, corr_y = 0.0f, corr_a = 0.0f;

    Trajectory X;   //posteriori state estimate
    Trajectory X_;  //priori estimate
    Trajectory P;   // posteriori estimate error covariance
    Trajectory P_;  // priori estimate error covariance
    Trajectory K;   //gain
    Trajectory z;   //actual measurement
    double pstd = 0.005f;//can be changed
    double cstd = 0.25;//can be changed
    Trajectory Q = Trajectory(pstd,pstd,pstd);// process noise covariance
    Trajectory R = Trajectory(cstd,cstd,cstd);// measurement noise covariance

    cv::Mat T = cv::Mat(2,3,CV_64F);
    cv::Mat last_T;
    cv::Mat cur_grey;
    cv::Mat prev_grey;

    double op_win_h = 25.0f;
    double op_win_v = 25.0f;
    double angle_limit = 10.0f;

public slots:
    void init(int _id = 0);
    void stabilize(cv::Mat &_prev_grey, cv::Mat &_curr_grey, bool useMask, cv::Mat& mask);
    void setMaxAmplitude(float _win_h, float _win_v);

signals:

private:
    int errCount = 0;
    inline void createMask();

};

#endif // KALMANOPTICALFLOWSTAB_H
