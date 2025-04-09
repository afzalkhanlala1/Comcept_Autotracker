#ifndef OBJECTTRACKER_H
#define OBJECTTRACKER_H

#include "app_includes.h"
#include "autotracker_types.h"
#include "dasiamrpntracker.h"
#include "autolock.h"
#include "templatematchtracker.h"
#include <QGraphicsItem>
#include <QRectF>
//#include <QtQuick>
//#include "pid.h"
#include "yoloclienttracker.h"

enum TARGETS {TANK, target_count};

struct TrackedResult
{
    QRectF rect;
    ObjectState objectState;
    bool success = false;
};

struct TargetRefSizes
{
    QVector3D tank = QVector3D(10.0f, 3.0f, 4.0f);
};

class ObjectTracker : public QObject
{
    Q_OBJECT
public:
    explicit ObjectTracker(QObject *parent = nullptr);
    ~ObjectTracker();

    bool enableDaSiamRPN = true;
    bool enableTM = true;
    bool enableYOLOClient = true;
    bool validRange = false;

    DaSiamRPNTracker *daSiamRPNTracker = nullptr;
    QThread daSiamRPNThread;

    TemplateMatchTracker *templateTracker = nullptr;
    QThread templateMatchThread;

    YoloClientTracker *yoloClientTracker = nullptr;
    QThread yoloClientThread;

    AutoLock *autoLock = nullptr;

    QTimer* trackingStatusTimer;
    int falseStatusCount;
    int falsecount;
    const int requiredFalseCount = 3;

    QVector<YoloResult> scaledYoloResults;
    //    QVector<QRect> scaledYoloResults;
    QVector<QRectF> trackedRects;
    QVector<TrackedResult> trackedResults;
    QRectF objectTrackedRect;
    QRectF previousRect;
    QRectF lastDasRect;
    QRectF roi;
    QVector<QVector3D> targetSizeLimits;
    ObjectMotion objectMotion;
    MotionLimits motionLimits;

    QVector<QRectF> dasRectsHist;
    int dasHistCount = 10;
    float thresh_val = 0.2;
    QPointF avgPos;
    QPointF topLeft;
    QSizeF avgSize;
    QRectF avgRect;

    float weight1;
    float weight2;

    bool yoloRoi = false;
    int prevTrackID = -1;
    int classID = -1;
    int trackID = -1;
    int newtrackID;
    bool trackIDfound;
    bool trackrectfound = false;
    bool yoloSuccess = false;
    bool runyolo = true;
    //    QRect RECT;
    QRect outRect2;
    bool outrectreceived = false;

    float width;
    float height;

    QVector2D _worldDisp; 
    QVector2D worldFrameDisp;
    QVector2D worldFrameVel;
    QVector2D ct_vel;
    QVector2D displacement;
    QVector2D new_displacement;
    QVector2D final_displacement;
    int frameThresh = 10;

    QVector2D ot_curr_vel;
    QVector2D ot_vel;
    QVector2D ot_accel;
    QVector2D ot_curr_accel;
    double _dt = 0.0f;

    float velocityThreshPixel = 10.0f;
    float accelThreshPixel = 3.0f;
    float velocityThresh = velocityThreshPixel / (0.04f);
    float accelThresh = accelThreshPixel / (0.04f);
    int tm_update_kf = 10;
    int tm_update_count = 0;
    QRectF curr_rect;
    QVector<TrackHistroy> tejectory;
    int maxTejectoryHistory = 20;
    int frameCount = 0;
    int last_frameCount = 0;
    double world_x = 0.0f;
    double world_y = 0.0f;
    QElapsedTimer dtTimer;
    QRectF lastobjectTrackedRect;
    QRectF prevobjectTrackedRect;
    QRectF SE_TrackedRectF;
    QRectF SE_TrackedRectF3;
    QRect SE_TrackedRect;
    QRectF rect1;
    QRectF rect2;
    QRectF SE_TrackedRectF2;
    QRectF objectTracking_Rect;

    QImage  frame;
    QImage frameImg;

    bool Das_Success;
    bool DasiamSuccess;
    bool TMsuccess;
    bool objectRectSuccess;

    int frameNum = 0;
    int lastframeNum = 0;
    QRect outRect;
    cv::Rect boundingBox;
    QRect boundingBox2;

    QPointF err;
    float gain = 0.01;

    QRectF expandedRoi;
    QVector<YoloResult> yoloResults;
    QRectF roiRect;
    QVector<YoloResult> filteredResults;
    QRectF overlapYoloRect;
    QSize  displayScreenSize;

    int maxIntersectionArea = 0;

    int myIndex = 0;
    bool initialized = false;
    bool enabled = false;
    bool roiSet = false;
    int trackerCompleted = 0;
    int trackerCount = 1;
    double frame_dt = 0.04f;
    bool isMotionRectValid = false;
    float maxTargetSpeed_kmh = 60.0f;
    float minTargetSpeed_kmh = 1.0f;
    float target_range = 2000.0f;
    float velocityGain = 1.0f;
    bool useOPFlowRegion = false;

    QPointF frameCenter = QPointF(640*0.5, 480*0.5);
    TargetRefSizes targetRefSize;
    QSize imageSize = QSize(640, 480);

    QElapsedTimer *pTimer = nullptr;
    double processingTime = 0.0f;

public slots:
    void init();
    void trackImageFeatures(TrackerFrame trackerFrame);
    void setROI(QRect rect, int index);
    void performRegionGrowing(QRect rect);
    void possibleTarget(QRectF target, float confidence);
    void setParam(QStringList params, int tracker_type);
    void setPreviewRectSize(QRectF imgRect);
    void setscale(float w, float h);
    void trackerFinished(QRectF trackedRect, ObjectState objectState, bool success, int tracker_type);
    void opticalFlowVelocties(OPResult _opResult);
    void opticalFlowImage(QImage image, double _dt);
    void setTargetRange(float val);

    void cancelRect(int rectindex);
    void cancelTracking();
    void receiveResults(const QVector<YoloResult>& scaledResults);
    //    void labelImage(QVector <YoloResult> results);
    void labelImage(const QVector<YoloResult>& results);
    void outRectReceived(QRect outRect);

signals:
    void initTrackers();
    void initAutoLock();
    void initCompleted();

    void yoloRoiSet(QRect rect);

    void yolofilteredRect(QRectF overlapYoloRect);
    void yolofiltered(QVector<YoloResult> filteredResults);

    void setTrackersROI(QRect rect);
    void markedRoiRect(QRect rect);
    void autoLockTarget(QImage frame, QRectF roi);
    void trackFeatures(QImage frame, double dt, QVector2D worldDisplacement = QVector2D());
    void imageTracked(QVector<QRectF> trackingRects, int index);
    void processOFVel(OPResult _opResult);
    void processOFImage(QImage image, double _dt);
    //    void template_setROI(QRect SE_TrackedRect);
    void template_setROI(QRect rect);
    void Dasiam_setROI(QRect SE_TrackedRect);
    void templateUpdatePos(QRectF objectTrackedRect);

    //    void yoloResultsReady(QVector<YoloResult> results);
    void cancelbb();
    void yoloResultsReady(const QVector<YoloResult>& results);
    void cancelyolorect(int rectindex);

    void contourRect(QRect boundingBox);


private:
    QMutex mutex;
    void updateVelAccelLimits();
    void updateSizeLimits();
    void enableTrackers();
    void checkTrackingStatus();
    void combineTrackers(int tracker_type);
    void dasiumTracking();
    void update_motion(QRectF lastobjectTrackedRect);
    void state_estimator_output(QRectF lastobjectTrackedRect);

    void getYoloRects();
    QRectF selectTrackedRect();
    QRectF constrainTrackedRect(QRectF rect, int tracker_type);

    inline float kmh_to_pixelsPerFrame(float kmh, float range, float fps = 24.0f, float hRes = 640.0f){
        return ( ((kmh*(1000.0f/3600.0f)) / fps) / ((range*DISTANCE_TO_FOV)/hRes) );
    }

    inline QVector2D rangeToFieldOfView(float range_m){
        return QVector2D(0.085713333f*range_m, 0.064286667f*range_m);
    }
};

#endif // OBJECTTRACKER_H

