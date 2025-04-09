#ifndef APP_CONNECTOR_H
#define APP_CONNECTOR_H

#include "autotrackercontroller.h"

/*

    void setTrackTarget(int tx, int ty, int width, int height);
    void setActiveTarget(int index);
    void cancelTargetRect(int index);
    void getNextAvailableTracker();
    void setTracking(bool val);

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
    void setOpticalFlowSettings(int _numLevels, double _pyrScale, bool _fastPyramids, int _winSize, int _numIters, int _polyN, double _polySigma, int _flags = 0);
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

    void enableIS(bool val);
    void setISViewMode(int val);
    void setISWindowMargin(float win_h, float win_v);
    void setVideoDevice(QString path, bool isFile);

*/

class AppConnector : public QObject
{
    Q_OBJECT
public:
    AppConnector();

    static void connectAppRemote(AutoTrackerController *atc, RemoteControl *rc)
    {
        connect(rc, &RemoteControl::setTrackTarget, atc, &AutoTrackerController::setTrackTarget);
        connect(rc, &RemoteControl::setUserEnableTracking, atc, &AutoTrackerController::setUserEnableTracking);
        connect(rc, &RemoteControl::setActiveTarget, atc, &AutoTrackerController::setActiveTarget);
        connect(rc, &RemoteControl::cancelTargetRect, atc, &AutoTrackerController::cancelTargetRect);
        connect(rc, &RemoteControl::setTracking, atc, &AutoTrackerController::setTracking);
        connect(rc, &RemoteControl::set, atc, &AutoTrackerController::set);
        connect(rc, &RemoteControl::setConsole, atc, &AutoTrackerController::setConsole);

        connect(rc, &RemoteControl::setShowOri, atc, &AutoTrackerController::setShowOri);
        connect(rc, &RemoteControl::setApplyLut, atc, &AutoTrackerController::setApplyLut);
        connect(rc, &RemoteControl::setLutAdaption, atc, &AutoTrackerController::setLutAdaption);
        connect(rc, &RemoteControl::setLutSmoothingWindow, atc, &AutoTrackerController::setLutSmoothingWindow);
        connect(rc, &RemoteControl::setLutStrength, atc, &AutoTrackerController::setLutStrength);
        connect(rc, &RemoteControl::setPreProcessingEffectStrength, atc, &AutoTrackerController::setPreProcessingEffectStrength);
        connect(rc, &RemoteControl::enableCLAHE, atc, &AutoTrackerController::enableCLAHE);
        connect(rc, &RemoteControl::setLutClaheFactor, atc, &AutoTrackerController::setLutClaheFactor);
        connect(rc, &RemoteControl::setClaheParams, atc, &AutoTrackerController::setClaheParams);
        connect(rc, &RemoteControl::setTrackToCenter, atc, &AutoTrackerController::setTrackToCenter);

        //        connect(rc, &RemoteControl::setOpticalFlowView, atc, &AutoTrackerController::setOpticalFlowView);
        //        connect(rc, &RemoteControl::setOpticalFlowWinSize, atc, &AutoTrackerController::setOpticalFlowWinSize);
        //        connect(rc, &RemoteControl::setOpticalFlowPreBlur, atc, &AutoTrackerController::setOpticalFlowPreBlur);
        //        connect(rc, &RemoteControl::setOpticalFlowDownsample, atc, &AutoTrackerController::setOpticalFlowDownsample);
        //        connect(rc, &RemoteControl::setOpticalFlowFilter, atc, &AutoTrackerController::setOpticalFlowFilter);
        //        connect(rc, &RemoteControl::setOpticalFlowGridSize, atc, &AutoTrackerController::setOpticalFlowGridSize);
        connect(rc, &RemoteControl::setEnableFrameBlending, atc, &AutoTrackerController::setEnableFrameBlending);
        connect(rc, &RemoteControl::setNoiseH, atc, &AutoTrackerController::setNoiseH);
        connect(rc, &RemoteControl::setNoiseSearchWindow, atc, &AutoTrackerController::setNoiseSearchWindow);
        connect(rc, &RemoteControl::setNoiseBlockSize, atc, &AutoTrackerController::setNoiseBlockSize);
        connect(rc, &RemoteControl::setPreNR, atc, &AutoTrackerController::setPreNR);
        connect(rc, &RemoteControl::setPostNR, atc, &AutoTrackerController::setPostNR);
        connect(rc, &RemoteControl::setEnableSharpen, atc, &AutoTrackerController::setEnableSharpen);

        //        connect(rc, &RemoteControl::enableIS, atc, &AutoTrackerController::enableIS);
        //        connect(rc, &RemoteControl::setISViewMode, atc, &AutoTrackerController::setISViewMode);
        //        connect(rc, &RemoteControl::setISWindowMargin, atc, &AutoTrackerController::setISWindowMargin);
        connect(rc, &RemoteControl::setVideoDevice, atc, &AutoTrackerController::setVideoDevice);

        connect(rc, &RemoteControl::setRemoteGuiJoy, atc, &AutoTrackerController::setRemoteGuiJoy);
    }

};

#endif // APP_CONNECTOR_H
