#ifndef AUTOTRACKER_TYPES_H
#define AUTOTRACKER_TYPES_H

#include <QImage>
#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QObject>

#define RGB888_CS 0
#define GRAY8B_CS 1

enum RFREMOTE { RF_JOY_01_X = 0, RF_JOY_01_Y, RF_JOY_02_X, RF_JOY_02_Y, RF_SLIDER, RF_TOGGLE_01, RF_TOGGLE_02, RF_PUSH_BT_01, RF_PUSH_BT_02,
                RF_JOY_SW, RF_KEY_BT, RF_GPS_LATI, RF_GPS_LONGI, RF_GPS_HEIGHT, AZI_ERR_VOLT, ELEV_ERR_VOLT, INP_VOLT, INP_CURR,
                BAT_VOLT, BAT_CURR, RF_VALID, rf_count };

enum USERPANEL { USP_JOY_X = 0, USP_JOY_Y, USP_JOY_SW, USP_SLIDER, USP_PUSH_01, USP_TOUCH_01, USP_TOUCH_02, USP_TOUCH_03, up_count };

enum EXT_DATA { GUNNER_OP = 0, READY_SIG, AMMO_TYPE, HORI_LEAD, VERT_LEAD, DISTANCE, SHOW_OSD, AUTOLOADER_READY_SIG, ed_count };

//enum IMUDATA {YAW = 0, PITCH, ROLL, GX, GY, GZ, AVG_GX, AVG_GY, AVG_GZ, LIN_AX, LIN_AY, LIN_AZ, _imuDataCount};
enum IMUDATA {AVG_GX = 0, AVG_GY, AVG_GZ, _imuDataCount};

enum GC_PANEL { GC_AUTO_MAN_LOCK = 0, GC_FAR_NEAR_LOCK, GC_LOCK, GC_UNLOCK, GC_UP, GC_DOWN, GC_LEFT, GC_RIGHT, _gc_panel_count };

enum GM_PANEL { GM_AUTO_SEMI = 0, GM_AUTORANGE, GM_BORESIGHT, GM_HN_CRHN, GM_HP_CRHP, GM_IR_AUTO_MAN, GM_IR_W_N_FOV, GM_LN_CRVN,
                GM_LP_CRVP, GM_TRACKER_RESET, GM_TV_TI, _gm_panel_count };

struct TrackerFrame{
    QImage frame;
    QImage lutFrame;
    QImage fgMask;
    QImage f_lutFrame;
    QImage scaledFrame;

    QImage teleImage;
    QRect teleViewRect;
    QRect teleWindow;
    float teleViewZoom = 3.0f;
    bool showTeleView = false;
    float client_server_scale = 1.0f;

    QImage stabImage;
    QRectF stab_win;
    QPointF win_jump;
    bool stab_win_valid = true;
    bool useIS = true;

    bool useLutFrame = false;
    bool useFrameSampling = false;
    double dt = 0.050f;
    qint64 frame_time = 0;
    int devType = 0;

    QVector2D worldDisplacement;
};

struct CapDeviceConfig
{
    QByteArrayList device_paths;
    QVector<bool> colorSpace;
    QVector<QSize> frameSize;
};

struct YoloResult
{
    QRectF bbox;
//    QString className;
    int label;
    float confidence = 0.0;
    int labelID;
};

struct ObjectState
{
    QVector3D vel;
    QVector3D accel;
    QVector3D pos;
};

struct ObjectMotion
{
    ObjectState measured;
    ObjectState estimated;
    ObjectState current;

    qint64 frame_time = 0;
};

struct OPResult
{
    QVector<QVector<QVector2D>> velocityGrid;
    QVector2D sceneVel;
    double dt = 0.05;
    int downsample = 2;
    int gridBlockSize = 20;
};

struct MotionLimits
{
    float xVelMin = 0.0f;
    float xVelMax = 0.0f;
    float xAccelMin = 0.0f;
    float xAccelMax = 0.0f;

    float yVelMin = 0.0f;
    float yVelMax = 0.0f;
    float yAccelMin = 0.0f;
    float yAccelMax = 0.0f;
};

struct CpuTimes{
    unsigned long long user = 0;
    unsigned long long nice = 0;
    unsigned long long system = 0;
    unsigned long long idle = 0;
    unsigned long long iowait = 0;
    unsigned long long irq = 0;
    unsigned long long softirq = 0;
    unsigned long long steal = 0;
    unsigned long long guest = 0;
    unsigned long long guest_nice = 0;
};

struct TransformParam
{
    TransformParam() {}
    TransformParam(double _dx, double _dy, double _da) {
        dx = _dx;
        dy = _dy;
        da = _da;
    }

    double dx;
    double dy;
    double da; // angle
};

struct Trajectory
{
    Trajectory() {}
    Trajectory(double _x, double _y, double _a) {
        x = _x;
        y = _y;
        a = _a;
    }
    // "+"
    friend Trajectory operator+(const Trajectory &c1,const Trajectory  &c2){
        return Trajectory(c1.x+c2.x,c1.y+c2.y,c1.a+c2.a);
    }
    //"-"
    friend Trajectory operator-(const Trajectory &c1,const Trajectory  &c2){
        return Trajectory(c1.x-c2.x,c1.y-c2.y,c1.a-c2.a);
    }
    //"*"
    friend Trajectory operator*(const Trajectory &c1,const Trajectory  &c2){
        return Trajectory(c1.x*c2.x,c1.y*c2.y,c1.a*c2.a);
    }
    //"/"
    friend Trajectory operator/(const Trajectory &c1,const Trajectory  &c2){
        return Trajectory(c1.x/c2.x,c1.y/c2.y,c1.a/c2.a);
    }
    //"="
    Trajectory operator =(const Trajectory &rx){
        x = rx.x;
        y = rx.y;
        a = rx.a;
        return Trajectory(x,y,a);
    }

    double x;
    double y;
    double a; // angle
};

struct OPConfig
{
    int numLevels = 5;
    double pyrScale = 0.75;
    bool fastPyramids = false;
    int winSize = 17;
    int numIters = 8;
    int polyN = 7;
    double polySigma = 2.0;
    int flags = 0;
};

Q_DECLARE_METATYPE(TrackerFrame)
Q_DECLARE_METATYPE(ObjectState)
Q_DECLARE_METATYPE(ObjectMotion)
Q_DECLARE_METATYPE(OPResult)
Q_DECLARE_METATYPE(QVector<QRect>)
Q_DECLARE_METATYPE(QVector<QRectF>)
Q_DECLARE_METATYPE(QVector<QPointF>)
Q_DECLARE_METATYPE(QVector<float>)
Q_DECLARE_METATYPE(QVector <YoloResult>)

#endif // AUTOTRACKER_TYPES_H
