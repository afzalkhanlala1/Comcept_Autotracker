#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H

#include "app_includes.h"


/*

    Q_INVOKABLE int setTrackTarget(int tx, int ty, int width, int height);
    Q_INVOKABLE void setUserEnableTracking(bool val);
    Q_INVOKABLE void setActiveTarget(int index);
    Q_INVOKABLE void cancelTargetRect(int index);
    Q_INVOKABLE void setTracking(bool val);
    Q_INVOKABLE void set(QString header, float val, bool send = true, bool save = true, bool cpCommand = false);
    Q_INVOKABLE void setTrackToCenter(bool val, int pointX, int pointY);

    Q_INVOKABLE void setShowOri(bool val);
    Q_INVOKABLE void setApplyLut(bool val);
    Q_INVOKABLE void setLutAdaption(double val);
    Q_INVOKABLE void setLutSmoothingWindow(int winSize);
    Q_INVOKABLE void setLutStrength(float val);
    Q_INVOKABLE void setPreProcessingEffectStrength(double _val);
    Q_INVOKABLE void enableCLAHE(bool val);
    Q_INVOKABLE void setLutClaheFactor(double val);
    Q_INVOKABLE void setClaheParams(int rows, int cols, double clipLimit);
    Q_INVOKABLE void setEnableFrameBlending(bool val);
    Q_INVOKABLE void setNoiseH(float _noiseH);
    Q_INVOKABLE void setNoiseSearchWindow(int _searchWin);
    Q_INVOKABLE void setNoiseBlockSize(int _noiseBlock);
    Q_INVOKABLE void setPreNR(bool val);
    Q_INVOKABLE void setPostNR(bool val);
    Q_INVOKABLE void setEnableSharpen(bool val);

    Q_INVOKABLE void setOpticalFlowView(int val);
    Q_INVOKABLE void setOpticalFlowWinSize(int _winSize);
    Q_INVOKABLE void setOpticalFlowPreBlur(int blur);
    Q_INVOKABLE void setOpticalFlowDownsample(int _downsample);
    Q_INVOKABLE void setOpticalFlowFilter(float gain);
    Q_INVOKABLE void setOpticalFlowGridSize(int _gridSize);


    Q_INVOKABLE void enableIS(bool val);
    Q_INVOKABLE void setISViewMode(int val);
    Q_INVOKABLE void setISWindowMargin(float win_h, float win_v);

    Q_INVOKABLE void setVideoDevice(QString path, bool isFile);


  */

enum FUNC { SetTrackigTarget = 0, SetUserEnableTracking, SetActiveTarget, CancelTargetRect, SetTracking, Set, SetShowOri, SetApplyLut, SetLutAdaption,
            SetLutSmoothingWindow, SetLutStrength, SetEffectStrength, EnableCLAHE, SetLutClaheFactor, SetClaheParams, SetTrackToCenter,
            SetOpticalFlowView, SetOpticalFlowWinSize, SetOpticalFlowPreBlur, SetOpticalFlowDownsample, SetOpticalFlowFilter,
            SetOpticalFlowGridSize, SetEnableFrameBlending, SetNoiseH, SetNoiseSearchWindow, SetNoiseBlockSize, SetPreNR,
            SetPostNR, SetEnableSharpen, EnableIS, SetISViewMode, SetISWindowMargin, SetVideoDevice, SetConsole, SetGuiJoystick, _func_count };


class RemoteControl : public QObject
{
    Q_OBJECT
public:
    explicit RemoteControl(QObject *parent = nullptr);

    QTcpServer *imageServer = nullptr;
    QTcpServer *dataServer = nullptr;
    QVector<QTcpSocket*> imageSockets;
    QVector<QTcpSocket*> dataSockets;
    QImage toSendFrame;

    QVector<QByteArray> pendingData;
    QByteArray incomingData;

    QTimer *statusTimer = nullptr;
    QTimer *commTimer = nullptr;

    int compressionQuality = 100;
    float curr_target_range = 2000.0f;

    QString appPath, settingsPath;

public slots:
    void init(QString _appPath);
    void newImageConnection();
    void newDataConnection();
    void readImageIncoming();
    void readDataIncoming();
    void sendFrame(TrackerFrame trackerFrame);
    void sendPendingData();
    void addData(QByteArray data);
    void addData(QByteArray header, float value);
    void addTeleData(QByteArray data);
    void checkConnectionStatus();
    void sendSettings(QTcpSocket *socket, QString dataFilePath);

signals:
    void incomingDataPacket(PacketData packetData);
    void rcStatus(int status);

    void setTrackTarget(int tx, int ty, int width, int height, bool scaleDown);
    void setUserEnableTracking(bool val);
    void setActiveTarget(int index);
    void cancelTargetRect(int index);
    void setTracking(bool val);
    void set(QString header, float val, bool send = true, bool save = true, bool cpCommand = false);
    void setConsole(QString header, float val);

    void setShowOri(bool val);
    void setApplyLut(bool val);
    void setLutAdaption(double val);
    void setLutSmoothingWindow(int winSize);
    void setLutStrength(float val);
    void setPreProcessingEffectStrength(double _val);
    void enableCLAHE(bool val);
    void setLutClaheFactor(double val);
    void setClaheParams(int rows, int cols, double clipLimit);
    void setTrackToCenter(bool val, int pointX, int pointY);

    void setOpticalFlowView(int val);
    void setOpticalFlowWinSize(int _winSize);
    void setOpticalFlowPreBlur(int blur);
    void setOpticalFlowDownsample(int _downsample);
    void setOpticalFlowFilter(float gain);
    void setOpticalFlowGridSize(int _gridSize);
    void setEnableFrameBlending(bool val);
    void setNoiseH(float _noiseH);
    void setNoiseSearchWindow(int _searchWin);
    void setNoiseBlockSize(int _noiseBlock);
    void setPreNR(bool val);
    void setPostNR(bool val);
    void setEnableSharpen(bool val);

    void enableIS(bool val);
    void setISViewMode(int val);
    void setISWindowMargin(float win_h, float win_v);

    void setVideoDevice(QString path, bool isFile);

    void setRemoteGuiJoy(float xVal, float yVal);

private:
    void parseData();
    QByteArray popPacket(QByteArray &data);
    inline void invokeUserCmd(QByteArrayList &packetValues);
    inline int getAvailableSocket(QVector<QTcpSocket*> &sockets);
};

#endif // REMOTECONTROL_H
