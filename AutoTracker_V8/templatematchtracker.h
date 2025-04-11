#ifndef TEMPLATEMATCHTRACKER_H
#define TEMPLATEMATCHTRACKER_H

#include "app_includes.h"

struct TrackHistroy
{
    QRectF pos;
    double time;
    QVector2D worldDisplacement;
};

class TemplateMatchTracker : public QObject
{
    Q_OBJECT
public:
    explicit TemplateMatchTracker(QObject *parent = nullptr);

    QRectF trackingRect;
    QRectF last_trackingRect;
    QVector2D trackingRectDisp;
    ObjectState objectState;
    QImage curr_frame;
    QImage ori_template;
    QImage curr_template;
    cv::Mat ori_template_mat;
    cv::Mat curr_template_mat;
    QElapsedTimer dtTimer;

    bool trackerCreated = false;
    bool initialized = false;
    bool roiSet = false;
    bool enabled = false;
    bool trackerSuccessful = false;
    bool busy = false;

    QElapsedTimer pTimer;
    QElapsedTimer dTimer;
    double frame_dt = 0.04f;
    double processingTime = 0;
    double diffTime = 0;

    QVector<TrackHistroy> tejectory;
    QVector<QImage> prev_matches;
    int max_pre_matches = 5;
    int tejectoryKeyFrame = 15;
    int maxTejectoryHistory = 50;
    int frameCount = 0;
    int last_frameCount = 0;
    float confidence = 0.0f, maxConfidence = 0.0f;

    double world_x = 0.0f;
    double world_y = 0.0f;

    QVector2D worldFrameDisp;
    QVector2D worldFrameVel;

    // Template-Matcher

    //Adaptive_Threshold
    QRectF roi;
    QImage templateMaskImage;
    cv::Mat templateMask;
    QRectF tmRect;
    QRectF curr_tmRect;
    float tm_HWin = 36;
    float tm_VWin = 24;
    int x_stride = 2;
    int y_stride = 2;
    QImage templateSeg;
    cv::Mat templateEdges_Inverted;
    cv::Mat templateEdges;
    QPointF roiCenter;
    QRectF center_region;
    QImage center_rect;
    cv::Mat center_image;
    cv::Scalar meanVal;
    double c_mean;
    float lb;
    int r = 4;

    float err_threadhold = 0.2f;
    QVector2D tm_vel;
    QVector2D tm_accel;
    QVector2D tm_curr_vel;
    QVector2D tm_curr_accel;
    QVector2D displacement;
    QVector3D localMatchedLoc;

    int failed = 0;
    float velocityThreshPixel = 10.0f;
    float accelThreshPixel = 3.0f;
    float velocityThresh = velocityThreshPixel / 0.04f;
    float accelThresh = accelThreshPixel / 0.04f;
    //float match_thres = 0.85f;
    double match_thres;
    float tmConfidence = 0.0f;
    int tm_update_kf = 10;
    int tm_update_count = 0;

// OpticalFlow

#ifdef USE_GPU
    cv::Ptr<cv::cuda::DenseOpticalFlow> opticalFlow;
    cv::cuda::GpuMat d_flow;
    cv::cuda::GpuMat planes[2];
#else
    cv::Ptr<cv::DenseOpticalFlow> opticalFlow;
    cv::Mat d_flow;
    cv::Mat planes[2];

#endif
    OPConfig op_config;
    float op_HWin = 48;
    float op_VWin = 12;
    cv::Mat curr_OF_frame, prev_OF_frame;
    cv::Mat flowx, flowy;
    QVector<QVector<QVector2D>> velocityGrid;
    QVector<QVector<QVector2D>> f_velocityGrid;
    QVector2D op_vel;
    QVector2D op_accel;
    QVector2D op_curr_vel;
    QVector2D op_curr_accel;
    QVector2D opDisplacement;
    QImage flowImage;
    QRectF opRect;

    double uMin = 0, uMax = 0;
    double vMin = 0, vMax = 0;
    float gridBlockSize = 8.0f;
    float dMax = 0;
    int blurRadius = 0;
    int downsample = 1;
    int user_downsample = 1;
    float vf_gain = 0.75f;
    bool opSuccess = false;
    bool opRectValid = false;

    cv::Scalar tempVal;
    cv::Scalar WindowVal;
    cv::Point minLoc, maxLoc;
    cv::Mat result;
    double temp_avg;
    double window_avg;
    double rel_cont;
    double curr_at;
    double at;

    // Contours
    float ct_HWin = 36;
    float ct_VWin = 24;
    double canny_thresh1 = 40;
    double canny_thresh2 = 160;
    QImage contourImage;
    QRectF contourRect;
    bool validContour = false;

public slots:
    void init();
    void setROI(QRect rect);
    void updatePos(QRectF objectTrackedRect);
    void track(TrackerFrame trackerFrame, double dt, QVector2D _worldDisplacement);
    void setParam(QStringList params);

signals:
    void tracked(QRectF trackedRect, ObjectState objectState, bool success, int tracker_type);

private:
    bool isRoiInImageArea(QRect &rect);
    float updateTM(QImage &frame, QRectF &bbox);
    bool updateOP(QImage &_frame, QRectF &bbox);

    inline float templateMatching(cv::Mat &inFrame, cv::Mat &tmpl, cv::Point &matchLoc);
    inline void updateOCVMotion(QImage &frame, QRectF &bbox, cv::Point &matchLoc, bool updatePrev);
    inline void filterVelocityGrid();
    inline void calcBlobPos(QImage &_frame, QImage &flowImage, QRectF &bbox);
    inline void findTargetContours(QImage &frame, QRectF &bbox);
    inline void applyImageMask(QImage &image, QImage &mask);

    void calVelocityGrid(const cv::Mat &u, const cv::Mat &v);
    void drawVelocityGrid(QImage &img, int _width, int _height);

    inline cv::Point qPointToCVPoint(QPointF qpoint);
    inline cv::Mat qImageToCV(QImage &frame);
    inline QImage cvToQImage(cv::Mat &cvImage, QImage::Format format = QImage::Format_Grayscale8);

    inline double getPolygonArea(QPolygonF &polygon)
    {
        QPointF p1, p2;
        double area = 0.0f, d = 0.0f;
        for (int i = 0; i < polygon.size(); i++)
        {
            p1 = polygon[i];
            p2 = polygon[(i + 1) % polygon.size()];
            d = p1.x() * p2.y() - p2.x() * p1.y();
            area += d;
        }

        return abs(area) / 2;
    }
};

#endif // TEMPLATEMATCHTRACKER_H
