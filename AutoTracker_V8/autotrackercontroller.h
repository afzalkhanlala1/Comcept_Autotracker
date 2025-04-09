#ifndef AUTOTRACKERCONTROLLER_H
#define AUTOTRACKERCONTROLLER_H

#include "app_includes.h"
#include "imageenhancer.h"
#include "opencvvideocapture.h"
#include "imagestabilization.h"
#include "remotecontrol.h"
#include "objecttracker.h"
#include "pid.h"
#include "systemstatsreport.h"
#include "coprocessor.h"
#include "frametransmitter.h"
#include "datadevice.h"

class AutoTrackerController : public QQuickPaintedItem
{
    Q_OBJECT
public:
    AutoTrackerController(QQuickItem *parent = nullptr);
    ~AutoTrackerController();
    void paint(QPainter *painter);

    //    QTcpServer *tcpServer = nullptr;
    //    QTcpSocket *tcpSocket = nullptr;

    OpenCVVideoCapture *captureDevice = nullptr;
    ImageEnhancer *imageEnhancer = nullptr;
    ImageStabilization *imageStabilization = nullptr;
    RemoteControl *remoteControl = nullptr;
    QVector<ObjectTracker*> objectTrackers;
    SystemStatsReport *systemStatsReport = nullptr;
    CoProcessor *coProcessor = nullptr;
    FrameTransmitter *frameTransmitter = nullptr;

    DataDevice *telemetryDevice = nullptr;
    QThread telemetryDeviceThread;

    QThread captureDeviceThread;
    QThread imageEnhancerThread;
    QThread isThread;
    QThread remoteControlThread;
    QVector<QThread*> trackerThreads;
    QThread systemStatsReportThread;
    QThread coProcessorThread;
    QThread frameTransmitterThread;

    PacketData rfRemotePacket;
    QStringList localDataFields;
    QPointF rch_move;
    QPointF rch_speed;

    PID trackingPIDx;
    PID trackingPIDy;
    double trackingXErr = 0;
    double trackingYErr = 0;
    double trackingPIDxVal = 0;
    double trackingPIDyVal = 0;

    QString homePath;
    QImage frameImg;
    QImage frameImgf;
    TrackerFrame trackerFrame;
    QVector<QRectF> tragetTrackingRects;
    int activeTarget = 0;

    int rcStatus = -1;
    QTimer *rcDataTimer = nullptr;
    QVector<double> localValues;

    double imgScalingFactor = 1.0f;
    float x_win_offset = 0.0f;
    float y_win_offset = 0.0f;
    QRectF stab_win;
    bool tracking = false;
    bool localDataReady = false;
    bool trackToImageCenter = true;
    QPoint trackToPoint;
    bool userEnableIS = false;

    bool graphUpdating = false;

    bool rfRemoteConnected = false;
    bool switchCamera_BtPressed = false;

    QVector<float> gcValues;
    QVector<float> gmValues;

    //     bool settingsReady = false;

    int selectedCamera = CAP_DEV::DAY_SIGHT;
    //        int selectedCamera = CAP_DEV::THERMAL_SIGHT;
    bool userPanelConnected = false;

    QElapsedTimer renderTimer;
    double renderTime = 0.0f;
    int ISViewMode = 1;
    bool showOriginalImage = false;
    bool showTragetRectWindow = true, showTragetRect = true;
    bool showTragetHistory = true, showTargetMask = false, showTargetFlow = false, showTargetTejectory = false;

    QVector<QVector<QRectF>> targetRects;

    QVector<float> extDataValues;
    QStringList ammoTypes;

    bool movingCrossHair = false;
    bool userEnableTracking = false;
    //    bool userEnableTracking = false;
    QVector<float> userPanelValues;

    QString systemStatsStr;
    QString systemStatsInfoStr;
    QByteArray systemStatsTele;

    float guiJoyXVal = 0.0f, guiJoyYVal = 0.0f;
    float remoteGuiJoyXVal = 0.0f, remoteGuiJoyYVal = 0.0f;

    QTimer *controlDataTimer = nullptr;

    QElapsedTimer telemetryTimer;
    QByteArray telemetryData;
    double totalTime = 0.0f;


public slots:
    //    void incomingConnection();

    void init();
    void captureDeviceInitCompleted();
    void frameReady(TrackerFrame _trackerFrame);
    void setActiveCamera(int _activeCamera);
    void cameraType();
    void sendTrackedRectsInfo();

    void rcIncomingData(PacketData packetData);
    void setRcStatus(int status);
    void sendRCData();
    void updateControlData();
    void updateTelemetryData();

    double getRFRemoteValue(int index);
    bool getGM_TV_TI();
    float getGMValue(int index);

    int getLocalFieldCount();
    QString getLocalFieldName(int i);
    double getLocalData(int index);
    bool getLocalDataReady();

    void setGraphUpdating(bool val);

    int getInputCameraCount();
    QString getInputCameraName(int index);
    void setVideoDevice(QString path, bool isFile);

    void setVideoPos(double pos);
    void nextVideoTimestamp();
    void previousVideoTimestamp();
    void getframeImg(QImage FrameImg);
    void getframeImgEnh(TrackerFrame trackerFrame);

    void setClaheParams(int rows, int cols, double clipLimit);
    bool isHeadlessMode();
    bool isDeployMode();
    //    int setTrackTarget(int tx, int ty, int width, int height, bool scaleDown = true);

    int getNextAvailableTracker();
    void setParam(QStringList params, int tracker_type);
    void imageTracked(QVector<QRectF> trackingRects, int index);
    void updateTrackerView();
    void cpIncomingData(PacketData packetData);

    void switchToNextTarget();

    double getUserPanelValue(int index);
    void handleYoloResults(const QVector<YoloResult> &results);
    bool getTracking();
    void setActiveTarget(int index);
    void cancelAllTargetRect();
    int getNextActiveTracker();
    void setTrackToCenter(bool val, int pointX, int pointY);
    int setTrackTarget(int tx, int ty, int width, int height, bool scaleDown = true);
    void cancelTargetRect(int index);
    int getTotalAtiveTracks();
    void setUserEnableTracking(bool val);
    void setTracking(bool val);
    void set(QString header, float val, bool send = true, bool save = true, bool cpCommand = false);

    void setPIDxMax(double val);
    void setPIDxMaxI(double val);
    void setPIDxKP(double val);
    void setPIDxKI(double val);
    void setPIDxKD(double val);
    void setPIDyMax(double val);
    void setPIDyMaxI(double val);
    void setPIDyKP(double val);
    void setPIDyKI(double val);
    void setPIDyKD(double val);

    double getImgScalingFactor();
    //    float getOPVelX();
    //    float getOPVelY();
    float getDaSiamRPNVelX();
    float getDaSiamRPNVelY();

    float getObjectVelX();
    float getObjectVelY();
    float getObjectAccelX();
    float getObjectAccelY();

    double getExtData(int index);
    bool getShowOSD();
    int getGunnerMode();
    bool getExtReady();
    QString getAmmo();
    double getHoriLead();
    double getVertLead();
    double getExtDistance();

    int isCrossHairOnTarget(int x, int y, bool useScaling = true);

    //      bool getSettingsReady();
    float getCorssHairX();
    float getCorssHairY();
    bool isCorssHairMoving();

    QString getInfo();
    QString getTrackInfo();

    QString getSystemStatsStr();
    QString getSystemStatsInfoStr();

    void statsReady(QString stats);

    void setShowOri(bool val);
    void setApplyLut(bool val);
    void enableCLAHE(bool val);
    void setLutClaheFactor(double val);
    void setEnableFrameBlending(bool val);
    void setEnableSharpen(bool val);
    void setPreNR(bool val);
    void setPostNR(bool val);
    void setFrameSamplingRate(float val);
    void setLutAdaption(double val);
    void setLutSmoothingWindow(int winSize);
    void setLutStrength(float val);
    void setNoiseH(float _noiseH);
    void setNoiseSearchWindow(int _searchWin);
    void setNoiseBlockSize(int _noiseBlock);
    void setPreProcessingEffectStrength(double _val);
    void setConsole(QString header, float val);
    double getOutputValues(int i);
    //    void setClaheParams(int rows, int cols, double clipLimit);
    void setGuiJoystick(float xVal, float yVal);
    void setRemoteGuiJoy(float xVal, float yVal);

signals:
    void initCaptureDevice();
    void initImageEnhancer();
    void initIS();
    void initRemoteControl(QString _appPath);
    void initTracker();
    void initSystemReport();
    void initCoProcessor(QString serialPortName);
    void initFrameTransmitter();

    void trackedRect(const QRectF &rect);

    void sendCommand(QString header, float val);
    void sendCPCommand(QString header, float val);
    void saveSetting(QString header, float val);

    //    void setCaptureDevice(QByteArray _deviceName);
    void setScreenSize(QSize _screenSize);
    void setActiveDevice(int _activeDev);
    void cameraDev(int selectedCamera);
    //    void setCaptureDevices(QByteArrayList _devices);
    void setCaptureDevices(QByteArrayList _deviceList, QSize targetFrameSize);
    void addRCTeleData(QByteArray data);
    void setVideoFile(QString fileName);
    void setCapturePreviewSizes(QRectF imgRect);
    void setVideoPlaybackPos(double pos);
    void setNextVideoTimestamp();
    void setPreVideoTimestamp();

    void updateClaheParams(int rows, int cols, double clipLimit);
    void setROI(QRect rect, int index);
    void _setParam(QStringList param, int tracker_type);

    void imuIncomingData(QVector<float> _imuData);
    void guiJoystickData(float _joy_x, float _joy_y, bool trackerEnabled);
    void cpAddData(QByteArray newData);

    void sendTelemetryData(QByteArray data);
    void initTelemetryDevice(QString _serialPortName, int _baudrate = 115200, QString _ip = "", quint16 _tcpPort = 0, bool _clientMode = false);


private:
    //    int maxMultiTargets = 1;
    int maxMultiTargets = 2;
    void drawDelpoyView(QPainter *painter);
    void drawDevelopmentView(QPainter *painter);
    void updateLocalValues();
    QVector<YoloResult> yoloResults;
    void processRFRemote();

    QTime remoteJoystickTime = QTime::currentTime();

};

#endif // AUTOTRACKERCONTROLLER_H
