#include "autotrackercontroller.h"
#include "app_connector.h"
#include "targetrenderer.h"

using namespace std;

AutoTrackerController::AutoTrackerController(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    homePath = QDir::homePath() + "/Documents/AutoTracker";
    objectTrackers = QVector<ObjectTracker*>(maxMultiTargets, nullptr);
    trackerThreads = QVector<QThread*>(maxMultiTargets, nullptr);
    targetRects = QVector<QVector<QRectF>>(maxMultiTargets, QVector<QRectF>(TRACKER_TYPE::tt_count));
    tragetTrackingRects = QVector<QRectF>(maxMultiTargets, QRectF());

    localDataFields << "XErr" << "TrackingPIDx" << "YErr" << "TrackingPIDy";
    localValues.resize(localDataFields.size());

    ammoTypes << "AP" << "HE" << "HA" << "MG" << "SPARE";

    rfRemotePacket.time = QTime::currentTime();
    rfRemotePacket.values = QVector<float>(RFREMOTE::rf_count, 0.0f);

    rch_move = QPointF(0.0f, 0.0f);
    rch_speed = QPointF(1, 1);
}

AutoTrackerController::~AutoTrackerController()
{
    captureDeviceThread.quit();
    captureDeviceThread.wait();

    imageEnhancerThread.quit();
    imageEnhancerThread.wait();

    isThread.quit();
    isThread.wait();

    remoteControlThread.quit();
    remoteControlThread.wait();

    systemStatsReportThread.quit();
    systemStatsReportThread.wait();

    coProcessorThread.quit();
    coProcessorThread.wait();

    frameTransmitterThread.quit();
    frameTransmitterThread.wait();

    if (telemetryDeviceThread.isRunning()) {
        telemetryDeviceThread.quit();
        telemetryDeviceThread.wait(3000);
    }

    for (int i = 0; i < maxMultiTargets; i++) {
        trackerThreads[i]->quit();
        trackerThreads[i]->wait();
    }
}

// Painting Methods
void AutoTrackerController::paint(QPainter *painter)
{
    // if (isHeadlessMode()) { return; }

    if (!trackerFrame.frame.isNull()) {
        // if (isDeployMode()) { drawDelpoyView(painter); }
        // else { drawDevelopmentView(painter); }
        drawDevelopmentView(painter);
    }
}

void AutoTrackerController::drawDelpoyView(QPainter *){}

void AutoTrackerController::drawDevelopmentView(QPainter *painter)
{
    renderTimer.start();

    if (captureDevice && boundingRect().width() != captureDevice->imagePreviewRect.width()) {
        emit setCapturePreviewSizes(boundingRect());
    }

    if (trackerFrame.frame.isNull()) { return; }

    updateTrackerView();

    bool paintLutFrame = false, paintOrignal = false, paintStabImage = false, paintStabWin = false;
    bool paintfgMask = false;
    double _imgScale = (double)trackerFrame.frame.width() / (double)boundingRect().width();

    if (!trackerFrame.stabImage.isNull()) {
        if (ISViewMode == 2) { paintStabWin = true; }
        else if (ISViewMode > 0) {
            paintStabImage = true;
            _imgScale = (double)trackerFrame.stabImage.width() / (double)boundingRect().width();
        }
    }

    imgScalingFactor = _imgScale;

    if (!paintStabImage || showOriginalImage) {
        if (trackerFrame.useLutFrame && !showOriginalImage) { paintLutFrame = true; }
        else { paintOrignal = true; paintStabImage = false; }
    }

    if (!trackerFrame.fgMask.isNull()) { paintfgMask = true; }

    if (paintOrignal) {
        painter->drawImage(0, 0, trackerFrame.frame.scaledToWidth(boundingRect().width(), Qt::SmoothTransformation));
    } else if (paintLutFrame) {
        painter->drawImage(0, 0, trackerFrame.lutFrame.scaledToWidth(boundingRect().width(), Qt::SmoothTransformation));
    }

    if (paintStabWin) {
        int img_x = stab_win.x();
        int img_y = stab_win.y();
        int img_width = (trackerFrame.stabImage.width() / imgScalingFactor);
        int img_height = (trackerFrame.stabImage.height() / imgScalingFactor);
        painter->drawImage(img_x, img_y, trackerFrame.stabImage.scaled(img_width, img_height, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else if (paintStabImage) {
        painter->drawImage(0, 0, trackerFrame.stabImage.scaledToWidth(boundingRect().width(), Qt::SmoothTransformation));
    }

    if (paintfgMask) {
        trackerFrame.fgMask = trackerFrame.fgMask.scaledToWidth(boundingRect().width(), Qt::SmoothTransformation);
    }

    // Overlay YOLO bounding boxes
    painter->setPen(QPen(Qt::green, 2)); // Green pen for bounding boxes
    for (const YoloResult &result : yoloResults) {
        // Scale coordinates to match the display
        QRectF scaledBBox(
            result.bbox.x() / imgScalingFactor,
            result.bbox.y() / imgScalingFactor,
            result.bbox.width() / imgScalingFactor,
            result.bbox.height() / imgScalingFactor
        );
        painter->drawRect(scaledBBox);

        // Optionally, draw label and confidence
        QString label = QString("ID:%1 C:%2").arg(result.labelID).arg(result.confidence, 0, 'f', 2);
        painter->setPen(Qt::yellow); // Yellow text for visibility
        painter->drawText(scaledBBox.topLeft() - QPointF(0, 5), label);
    }

    updateTelemetryData();
    renderTemplateMatcher(this, painter, boundingRect());

    renderTime = (renderTime * 0.9f) + ((renderTimer.nsecsElapsed() / 1000000.0f) * 0.1f);
}

// Initialization
void AutoTrackerController::init()
{
    qDebug() << "AutoTracker - INIT: " << QThread::currentThreadId() << boundingRect();
    QThread::currentThread()->setPriority(QThread::TimeCriticalPriority);

    // Object instantiation
    captureDevice = new OpenCVVideoCapture;
    imageEnhancer = new ImageEnhancer;
    imageStabilization = new ImageStabilization;
    remoteControl = new RemoteControl;
    rcDataTimer = new QTimer;
    systemStatsReport = new SystemStatsReport;
    coProcessor = new CoProcessor;
    controlDataTimer = new QTimer;
    frameTransmitter = new FrameTransmitter;
    telemetryDevice = new DataDevice;

    // Tracker setup
    for (int i = 0; i < maxMultiTargets; i++) {
        objectTrackers[i] = new ObjectTracker;
        trackerThreads[i] = new QThread;
        objectTrackers[i]->myIndex = i;
        objectTrackers[i]->moveToThread(trackerThreads[i]);
        trackerThreads[i]->start(QThread::HighestPriority);

        connect(this, &AutoTrackerController::initTracker, objectTrackers[i], &ObjectTracker::init);
        connect(this, &AutoTrackerController::setROI, objectTrackers[i], &ObjectTracker::setROI);
        connect(this, &AutoTrackerController::_setParam, objectTrackers[i], &ObjectTracker::setParam);
        connect(objectTrackers[i], &ObjectTracker::imageTracked, this, &AutoTrackerController::imageTracked);
        connect(imageStabilization, &ImageStabilization::frameStabilized, objectTrackers[i], &ObjectTracker::trackImageFeatures);
        connect(this, &AutoTrackerController::setCapturePreviewSizes, objectTrackers[i], &ObjectTracker::setPreviewRectSize);

        // Uncomment if YOLO or contour functionality is implemented
        // connect(objectTrackers[i], &ObjectTracker::cancelbb, this, &AutoTrackerController::cancelyolorects);
        connect(objectTrackers[i], &ObjectTracker::yoloResultsReady, this, &AutoTrackerController::handleYoloResults);
        // connect(objectTrackers[i], &ObjectTracker::cancelyolorect, this, &AutoTrackerController::cancelTargetRect);
        // connect(objectTrackers[i], &ObjectTracker::contourRect, this, &AutoTrackerController::contourRect);
        // connect(this, &AutoTrackerController::screenscale, objectTrackers[i], &ObjectTracker::setscale);
        // connect(this, &AutoTrackerController::yololabeled, objectTrackers[i], &ObjectTracker::receiveResults);
        // connect(objectTrackers[i], &ObjectTracker::markedRoiRect, this, &AutoTrackerController::markedRoiRect);
        // connect(objectTrackers[i], &ObjectTracker::yolofiltered, this, &AutoTrackerController::handleYoloFiltered);
        // connect(objectTrackers[i], &ObjectTracker::yolofilteredRect, this, &AutoTrackerController::handleYoloFilteredRect);
        // connect(this, &AutoTrackerController::outRectSent, objectTrackers[i], &ObjectTracker::outRectReceived);
    }

    // Signal-slot connections
    connect(this, &AutoTrackerController::initCaptureDevice, captureDevice, &OpenCVVideoCapture::init);
    connect(this, &AutoTrackerController::setCaptureDevices, captureDevice, &OpenCVVideoCapture::setCaptureDevices);
    connect(this, &AutoTrackerController::initIS, imageStabilization, &ImageStabilization::init);
    connect(this, &AutoTrackerController::initRemoteControl, remoteControl, &RemoteControl::init);
    connect(this, &AutoTrackerController::initSystemReport, systemStatsReport, &SystemStatsReport::init);
    connect(this, &AutoTrackerController::initCoProcessor, coProcessor, &CoProcessor::init);
    connect(this, &AutoTrackerController::initFrameTransmitter, frameTransmitter, &FrameTransmitter::init);
    // connect(this, &AutoTrackerController::initTelemetryDevice, telemetryDevice, &DataDevice::init); // Uncomment if telemetry init is needed

    connect(this, &AutoTrackerController::setCapturePreviewSizes, captureDevice, &OpenCVVideoCapture::setCapturePreviewSizes);
    connect(this, &AutoTrackerController::sendCPCommand, coProcessor, &CoProcessor::sendCommand);
    connect(this, &AutoTrackerController::guiJoystickData, coProcessor, &CoProcessor::setGuiJoystickData);
    connect(this, &AutoTrackerController::cpAddData, coProcessor, &CoProcessor::addData);
    connect(this, &AutoTrackerController::setActiveDevice, captureDevice, &OpenCVVideoCapture::setActiveDevice);
    connect(captureDevice, &OpenCVVideoCapture::initCompleted, this, &AutoTrackerController::captureDeviceInitCompleted);
    connect(captureDevice, &OpenCVVideoCapture::trackerFrameReady, imageEnhancer, &ImageEnhancer::processImage);
    connect(this, &AutoTrackerController::initImageEnhancer, imageEnhancer, &ImageEnhancer::init);
    connect(this, &AutoTrackerController::addRCTeleData, remoteControl, &RemoteControl::addTeleData);
    connect(imageEnhancer, &ImageEnhancer::frameReady, imageStabilization, &ImageStabilization::stabilizeFrame);
    connect(imageStabilization, &ImageStabilization::frameStabilized, this, &AutoTrackerController::frameReady);
    connect(imageStabilization, &ImageStabilization::frameStabilized, frameTransmitter, &FrameTransmitter::getFrame);
    connect(remoteControl, &RemoteControl::incomingDataPacket, this, &AutoTrackerController::rcIncomingData);
    connect(remoteControl, &RemoteControl::rcStatus, this, &AutoTrackerController::setRcStatus);
    connect(rcDataTimer, &QTimer::timeout, this, &AutoTrackerController::sendRCData);
    connect(systemStatsReport, &SystemStatsReport::statsReady, this, &AutoTrackerController::statsReady);
    connect(coProcessor, &CoProcessor::incomingDataPacket, this, &AutoTrackerController::cpIncomingData);
    connect(controlDataTimer, &QTimer::timeout, this, &AutoTrackerController::updateControlData);
    connect(this, &AutoTrackerController::cameraDev, frameTransmitter, &FrameTransmitter::getCameraType);
    connect(this, &AutoTrackerController::trackedRect, frameTransmitter, &FrameTransmitter::getTrackedRectInfo);
    connect(this, &AutoTrackerController::sendTelemetryData, frameTransmitter, &FrameTransmitter::addRawData);
    // connect(this, &AutoTrackerController::sendTelemetryData, telemetryDevice, &DataDevice::addRawData); // Uncomment if telemetry device is used

    // Thread setup
    captureDevice->moveToThread(&captureDeviceThread);
    captureDeviceThread.start(QThread::HighestPriority);

    imageEnhancer->moveToThread(&imageEnhancerThread);
    imageEnhancerThread.start(QThread::HighestPriority);

    imageStabilization->moveToThread(&isThread);
    isThread.start(QThread::HighestPriority);

    remoteControl->moveToThread(&remoteControlThread);
    remoteControlThread.start(QThread::HighPriority);

    systemStatsReport->moveToThread(&systemStatsReportThread);
    systemStatsReportThread.start(QThread::NormalPriority);

    coProcessor->moveToThread(&coProcessorThread);
    coProcessorThread.start(QThread::HighPriority);

    frameTransmitter->moveToThread(&frameTransmitterThread);
    frameTransmitterThread.start(QThread::HighestPriority);

    telemetryDevice->moveToThread(&telemetryDeviceThread);
    telemetryDeviceThread.start(QThread::HighPriority);

    // Emit initialization signals
    emit initCaptureDevice();
    emit initImageEnhancer();
    emit initIS();
    emit initRemoteControl(QString(homePath + "/Qt/AutoTracker/AutoTracker_V6/dataFiles/"));
    emit initTracker();
    emit initSystemReport();
    emit initCoProcessor(CP_SERIALPATH);
    emit initFrameTransmitter();
    emit initTelemetryDevice("", -1, "", TELE_DATA_PORT, false);

    AppConnector::connectAppRemote(this, remoteControl);
    controlDataTimer->start(50);
    telemetryTimer.start();
}

// PID Setters
void AutoTrackerController::setPIDxMax(double val) { trackingPIDx.maxErr = val; }
void AutoTrackerController::setPIDxMaxI(double val) { trackingPIDx.maxI = val; }
void AutoTrackerController::setPIDxKP(double val) { trackingPIDx.kp = val; }
void AutoTrackerController::setPIDxKI(double val) { trackingPIDx.ki = val; }
void AutoTrackerController::setPIDxKD(double val) { trackingPIDx.kd = val; }
void AutoTrackerController::setPIDyMax(double val) { trackingPIDy.maxErr = val; }
void AutoTrackerController::setPIDyMaxI(double val) { trackingPIDy.maxI = val; }
void AutoTrackerController::setPIDyKP(double val) { trackingPIDy.kp = val; }
void AutoTrackerController::setPIDyKI(double val) { trackingPIDy.ki = val; }
void AutoTrackerController::setPIDyKD(double val) { trackingPIDy.kd = val; }

// Capture Device Handling
void AutoTrackerController::captureDeviceInitCompleted()
{
    QByteArrayList devices;
    QVector<bool> colorSpaces;
    QVector<QSize> frameSizes;

    // CAMERA INPUT EXAMPLE (uncomment to use cameras instead of video)
    // devices = { "/dev/video2", "/dev/video0" };
    // colorSpaces = QVector<bool>{ GRAY8B_CS, GRAY8B_CS };
    // frameSizes = QVector<QSize>{ QSize(1280, 720), QSize(1280, 720) };
    // captureDevice->deviceConfig.device_paths = devices;
    // captureDevice->deviceConfig.colorSpace << colorSpaces;
    // captureDevice->deviceConfig.frameSize << frameSizes;
    // emit setCaptureDevices(captureDevice->deviceConfig.device_paths, QSize(1280, 960));
    // setActiveCamera(selectedCamera);
    // emit cameraDev(selectedCamera);

    // VIDEO INPUT (currently active)
    devices.append("/home/jetson/Documents/AutoTracker/Tank-Videos/batch_1/6.mkv");
    captureDevice->deviceConfig.device_paths = devices;
    captureDevice->deviceConfig.colorSpace << GRAY8B_CS;
    captureDevice->deviceConfig.frameSize << QSize(1280, 720);
    emit setCaptureDevices(captureDevice->deviceConfig.device_paths, QSize(1280, 960));
}

void AutoTrackerController::frameReady(TrackerFrame _trackerFrame)
{
    if (_trackerFrame.frame.isNull()) return;
    if (!_trackerFrame.stab_win.isNull() && !_trackerFrame.stabImage.isNull()) {
        x_win_offset = _trackerFrame.stab_win.x();
        y_win_offset = _trackerFrame.stab_win.y();
        float x = (_trackerFrame.stab_win.x() / imgScalingFactor);
        float y = (_trackerFrame.stab_win.y() / imgScalingFactor);
        float w = (_trackerFrame.stab_win.width() / imgScalingFactor);
        float h = (_trackerFrame.stab_win.height() / imgScalingFactor);
        stab_win = QRectF(x, y, w, h);
    }
    trackerFrame = _trackerFrame;
}

void AutoTrackerController::setActiveCamera(int _activeCamera)
{
    selectedCamera = _activeCamera;
    emit setActiveDevice(selectedCamera);
}

void AutoTrackerController::cameraType()
{
    emit cameraDev(selectedCamera);
}

// Tracker Management
void AutoTrackerController::sendTrackedRectsInfo()
{
    for (int i = 0; i < objectTrackers.size(); i++) {
        float rx1 = (objectTrackers[i]->trackedRects.last().left()) / imgScalingFactor;
        float ry1 = (objectTrackers[i]->trackedRects.last().top()) / imgScalingFactor;
        float rx2 = (objectTrackers[i]->trackedRects.last().right()) / imgScalingFactor;
        float ry2 = (objectTrackers[i]->trackedRects.last().bottom()) / imgScalingFactor;
        emit trackedRect(QRectF(QPointF(rx1, ry1), QPointF(rx2, ry2)));
    }
}

void AutoTrackerController::updateTrackerView()
{
    if (rch_move.manhattanLength() == 0) {
        rch_move = QPointF(boundingRect().width() * 0.5f, boundingRect().height() * 0.5f);
    }

    movingCrossHair = false;

    if (movingCrossHair) {
        rch_speed += QPointF(0.15, 0.1125);
        if (rch_speed.x() > 20) { rch_speed.setX(20); }
        if (rch_speed.y() > 20) { rch_speed.setY(20); }
    } else {
        rch_speed = QPointF(1.0, 1.0);
    }

    if (rch_move.x() > boundingRect().width() * 0.95f) { rch_move.setX(boundingRect().width() * 0.95f); }
    else if (rch_move.x() < boundingRect().width() * 0.05f) { rch_move.setX(boundingRect().width() * 0.05f); }

    if (rch_move.y() > boundingRect().height() * 0.95f) { rch_move.setY(boundingRect().height() * 0.95f); }
    else if (rch_move.y() < boundingRect().height() * 0.05f) { rch_move.setY(boundingRect().height() * 0.05f); }
}

void AutoTrackerController::handleYoloResults(const QVector<YoloResult> &results) {
    qDebug() << "Received" << results.size() << "YOLO detections";
    for (const YoloResult &r : results) {
        qDebug() << "ID:" << r.labelID << "Conf:" << r.confidence << "BBox:" << r.bbox;
    }
    yoloResults = results;
    update();
}

// Data Processing
void AutoTrackerController::cpIncomingData(PacketData packetData)
{
    if (packetData.module == Module::IMU_DATA) {
        emit imuIncomingData(packetData.values);
    } else if (packetData.module == Module::FCS_DATA || packetData.module == Module::EXT_DATA) {
        extDataValues = packetData.values;
    } else if (packetData.module == Module::RF_REMOTE) {
        rfRemotePacket = packetData;
        processRFRemote();
        rfRemoteConnected = (getRFRemoteValue(RFREMOTE::RF_VALID) > 0.5f);
    } else if (packetData.module == Module::GUNNER_CONSOLE) {
        gcValues = packetData.values;
    } else if (packetData.module == Module::GUNNER_MONITOR) {
        gmValues = packetData.values;
        // if (captureDevice) { captureDevice->day_night_blend = getGM_TV_TI(); } // Uncomment if day/night blending is implemented
    } else if (packetData.module == Module::USER_PANEL) {
        userPanelValues = packetData.values;
        showOriginalImage = getUserPanelValue(USERPANEL::USP_TOUCH_03);
        setTracking((getUserPanelValue(USERPANEL::USP_TOUCH_01) > 0.7) || userEnableTracking);
        if (imageStabilization) {
            imageStabilization->setEnabled(getUserPanelValue(USERPANEL::USP_TOUCH_03) > 0.5 || getRFRemoteValue(RFREMOTE::RF_TOGGLE_02) > 0.5 || userEnableIS);
        }
        if (captureDevice) {
            float _dnb = getUserPanelValue(USERPANEL::USP_SLIDER);
            if (_dnb > 0.95f) { _dnb = 1.0f; } else if (_dnb < 0.05f) { _dnb = 0.0f; }
            // captureDevice->day_night_blend = _dnb; // Uncomment if day/night blending is implemented
        }
        userPanelConnected = true;
    }
}

void AutoTrackerController::processRFRemote()
{
    setTracking((getUserPanelValue(USERPANEL::USP_TOUCH_01) > 0.7) || userEnableTracking);
    if (imageStabilization) {
        imageStabilization->setEnabled(getUserPanelValue(USERPANEL::USP_TOUCH_03) > 0.5 || getRFRemoteValue(RFREMOTE::RF_TOGGLE_02) > 0.5 || userEnableIS);
    }
    if (captureDevice) {
        if (getRFRemoteValue(RFREMOTE::RF_JOY_SW) > 0.5) {
            if (!switchCamera_BtPressed) {
                // if (captureDevice->day_night_blend > 0.5f) { captureDevice->day_night_blend = 0.0f; }
                // else { captureDevice->day_night_blend = 1.0f; } // Uncomment if day/night blending is implemented
                switchCamera_BtPressed = true;
            }
        } else {
            switchCamera_BtPressed = false;
        }
    }
}

double AutoTrackerController::getRFRemoteValue(int index)
{
    if (index < 0 || index >= rfRemotePacket.values.size()) { return 0; }
    return rfRemotePacket.values[index];
}

bool AutoTrackerController::getGM_TV_TI()
{
    return (getGMValue(GM_PANEL::GM_TV_TI) > 0.5f);
}

float AutoTrackerController::getGMValue(int index)
{
    if (index < 0 || index >= gmValues.size()) { return 0.0f; }
    return gmValues[index];
}

// Telemetry and Control Data
void AutoTrackerController::sendRCData()
{
    QByteArray teleData = "#_TEL";
    for (int i = 0; i < getLocalFieldCount(); i++) {
        teleData += QByteArray::number(getLocalData(i)) + ",";
    }
    for (int i = 0; i < tragetTrackingRects.size(); i++) {
        teleData.append(QByteArray::number(tragetTrackingRects[i].x(), 'f', 0) + "," +
                        QByteArray::number(tragetTrackingRects[i].y(), 'f', 0) + "," +
                        QByteArray::number(tragetTrackingRects[i].width(), 'f', 0) + "," +
                        QByteArray::number(tragetTrackingRects[i].height(), 'f', 0) + ",");
    }
    teleData += QByteArray::number(activeTarget) + "," + QByteArray::number(tracking) + "," + systemStatsTele + ",";
    if (rfRemotePacket.values.size() >= RFREMOTE::rf_count) {
        teleData += QByteArray::number(rfRemotePacket.values[RFREMOTE::AZI_ERR_VOLT]) + "," +
                    QByteArray::number(rfRemotePacket.values[RFREMOTE::ELEV_ERR_VOLT]) + "," +
                    QByteArray::number(rfRemotePacket.values[RFREMOTE::INP_VOLT]) + "," +
                    QByteArray::number(rfRemotePacket.values[RFREMOTE::BAT_CURR]) + ",";
    } else {
        teleData += "0,0,0,0,";
    }
    teleData += QByteArray::number(rch_move.x() * imgScalingFactor) + "," + QByteArray::number(rch_move.y() * imgScalingFactor) + "!";
    emit addRCTeleData(teleData);
}

void AutoTrackerController::updateControlData()
{
    bool remoteJoyValid = (abs(remoteJoystickTime.msecsTo(QTime::currentTime())) < 400);
    if (remoteJoyValid && guiJoyXVal == 0 && guiJoyYVal == 0) {
        emit guiJoystickData(remoteGuiJoyXVal, remoteGuiJoyYVal, userEnableTracking || tracking);
    } else {
        emit guiJoystickData(guiJoyXVal, guiJoyYVal, userEnableTracking || tracking);
    }
}

void AutoTrackerController::updateTelemetryData()
{
    // if (telemetryTimer.elapsed() < 10) { return; } // Uncomment if timing constraint is needed
    telemetryData = "#TELE";
    telemetryData += QByteArray::number(boundingRect().width(), 'f', 0) + "," + QByteArray::number((int)boundingRect().height(), 'f', 0);
    telemetryData += "," + QByteArray::number(totalTime, 'f', 1) + "," + QByteArray::number(remoteControl->compressionQuality, 'f', 1);

    bool localRendering = false; // Adjust if server rendering is implemented
    if (localRendering) {
        telemetryData += ",0,-1,-1,0";
    } else {
        telemetryData += "," + QByteArray::number(objectTrackers.size()) + "," + QByteArray::number(activeTarget);
        for (int i = 0; i < objectTrackers.size(); i++) {
            QRectF tr = objectTrackers[i]->objectTrackedRect;
            telemetryData += "," + QByteArray::number(objectTrackers[i]->enabled) + "," +
                            QByteArray::number(tr.x() / imgScalingFactor, 'f', 0) + "," +
                            QByteArray::number(tr.y() / imgScalingFactor, 'f', 0) + "," +
                            QByteArray::number(tr.width() / imgScalingFactor, 'f', 0) + "," +
                            QByteArray::number(tr.height() / imgScalingFactor, 'f', 0);
        }
    }
    telemetryData += "!";
    emit sendTelemetryData(telemetryData);
    telemetryTimer.start();
}

void AutoTrackerController::rcIncomingData(PacketData){}

void AutoTrackerController::setRcStatus(int status)
{
    rcStatus = status;
    if (rcStatus > 1) { rcDataTimer->start(50); }
    else { rcDataTimer->stop(); }
}

// Local Data Accessors
int AutoTrackerController::getLocalFieldCount() { return localDataFields.size(); }
QString AutoTrackerController::getLocalFieldName(int i) { return (i < 0 || i >= localDataFields.size()) ? "" : localDataFields[i]; }
double AutoTrackerController::getLocalData(int index) { return (index < 0 || index >= localValues.size()) ? 0 : localValues[index]; }
bool AutoTrackerController::getLocalDataReady() { if (localDataReady) { localDataReady = false; return true; } return false; }
void AutoTrackerController::setGraphUpdating(bool val) { graphUpdating = val; }

// Camera Input Handling
int AutoTrackerController::getInputCameraCount() { return QCameraInfo::availableCameras().size(); }
QString AutoTrackerController::getInputCameraName(int index)
{
    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    return (index < 0 || index >= cameras.size()) ? "" : cameras[index].deviceName();
}

void AutoTrackerController::setVideoDevice(QString path, bool)
{
    QByteArrayList devices;
    if (path.startsWith("file://")) { path.remove(0, 7); }
    devices.append(path.toUtf8());
    captureDevice->deviceConfig.device_paths = devices;
    captureDevice->deviceConfig.colorSpace << GRAY8B_CS;
    captureDevice->deviceConfig.frameSize << QSize(1280, 720);
    emit setCaptureDevices(captureDevice->deviceConfig.device_paths, QSize(1280, 960));
}

void AutoTrackerController::setVideoPos(double) { /* emit setVideoPlaybackPos(pos); */ }
void AutoTrackerController::nextVideoTimestamp() { emit setNextVideoTimestamp(); }
void AutoTrackerController::previousVideoTimestamp() { emit setPreVideoTimestamp(); }

// Image Processing
void AutoTrackerController::getframeImg(QImage ) { /* frameImg = FrameImg; */ }
void AutoTrackerController::getframeImgEnh(TrackerFrame trackerFrame) { frameImgf = trackerFrame.frame; }
void AutoTrackerController::setClaheParams(int rows, int cols, double clipLimit) { emit updateClaheParams(rows, cols, clipLimit); }

// Mode Checks
bool AutoTrackerController::isHeadlessMode()
{
#if defined(HEADLESS_MODE)
    return true;
#else
    return false;
#endif
}

bool AutoTrackerController::isDeployMode()
{
#if defined(DEPLOY_MODE)
    return true;
#else
    return false;
#endif
}

// Target Tracking
int AutoTrackerController::setTrackTarget(int tx, int ty, int width, int height, bool scaleDown)
{
    int index = getNextAvailableTracker();
    if (index < 0) return -1;

    setActiveTarget(index);
    float ims = imgScalingFactor;
    if (scaleDown) {
        targetRects[index].last() = QRectF((tx * ims), (ty * ims), width * ims, height * ims);
    } else {
        targetRects[index].last() = QRectF(tx - (width * 0.5), ty - (height * 0.5), width, height);
    }
    emit setROI(targetRects[index].last().toRect(), index);
    return index;
}

void AutoTrackerController::setActiveTarget(int index) { if (index >= 0 && index < objectTrackers.size()) { activeTarget = index; } }
int AutoTrackerController::getNextAvailableTracker() { for (int i = 0; i < objectTrackers.size(); i++) { if (!objectTrackers[i]->enabled) return i; } return -1; }
void AutoTrackerController::setParam(QStringList param, int tracker_type) { emit _setParam(param, tracker_type); }

void AutoTrackerController::imageTracked(QVector<QRectF> trackingRects, int index)
{
    targetRects[index] = trackingRects;
    tragetTrackingRects[index] = trackingRects.last();

    bool remoteJoyValid = ((remoteGuiJoyXVal != 0) || (remoteGuiJoyYVal != 0)) && (abs(remoteJoystickTime.msecsTo(QTime::currentTime())) < 400);
    bool guiJoyValid = (guiJoyXVal != 0) || (guiJoyYVal != 0);

    if (guiJoyValid || remoteJoyValid || !getTracking()) {
        trackingPIDxVal = trackingPIDyVal = 0;
        trackingPIDx.resetI();
        trackingPIDy.resetI();
    } else if (index == activeTarget) {
        QPointF rectCenter = targetRects[index].last().center();
        QPointF imgCenter = (!trackToImageCenter && !trackToPoint.isNull()) ?
            trackToPoint : QPointF((float)captureDevice->widthTraget * 0.5f, (float)captureDevice->heightTraget * 0.5f);
        trackingXErr = rectCenter.x() - imgCenter.x();
        trackingYErr = rectCenter.y() - imgCenter.y();
        trackingPIDxVal = trackingPIDx.compute(trackingXErr, trackerFrame.dt);
        trackingPIDyVal = trackingPIDy.compute(trackingYErr, trackerFrame.dt);
    }

    coProcessor->trackingPIDxVal = trackingPIDxVal;
    coProcessor->trackingPIDyVal = trackingPIDyVal;
    coProcessor->at_gate = ((abs(trackingXErr) < 2) && (abs(trackingYErr) < 2));
    updateLocalValues();
    sendTrackedRectsInfo();
}

bool AutoTrackerController::getTracking() { return tracking; }

void AutoTrackerController::cancelTargetRect(int index)
{
    if (objectTrackers[index]) { objectTrackers[index]->cancelTracking(); }
    tragetTrackingRects[index] = targetRects[index].last() = QRectF();

    if (getTotalAtiveTracks() == 0) {
        setUserEnableTracking(false);
        setTracking(false);
        trackingPIDxVal = trackingPIDyVal = 0;
        trackingPIDx.resetI();
        trackingPIDy.resetI();
        activeTarget = -1;
    } else {
        int at = getNextActiveTracker();
        if (at != -1) { activeTarget = at; }
    }
}

// User Panel and Settings
double AutoTrackerController::getUserPanelValue(int index) { return (index < 0 || index >= userPanelValues.size()) ? 0 : userPanelValues[index]; }
void AutoTrackerController::setUserEnableTracking(bool val) { userEnableTracking = val; setTracking((getUserPanelValue(USERPANEL::USP_TOUCH_01) > 0.7) || userEnableTracking); }
void AutoTrackerController::setTracking(bool val) { tracking = val; }
void AutoTrackerController::set(QString header, float val, bool send, bool save, bool cpCommand)
{
    if (send) { emit sendCommand(header, val); }
    if (save) { emit saveSetting(header, val); }
    if (cpCommand) { emit sendCPCommand(header, val); }
}

// Getters for Tracking Data
double AutoTrackerController::getImgScalingFactor() { return imgScalingFactor; }
float AutoTrackerController::getDaSiamRPNVelX() { return (activeTarget >= 0 && activeTarget < objectTrackers.size() && objectTrackers[activeTarget]) ? objectTrackers[activeTarget]->trackedResults[TRACKER_TYPE::DaSiamRPN].objectState.vel.x() : 0; }
float AutoTrackerController::getDaSiamRPNVelY() { return (activeTarget >= 0 && activeTarget < objectTrackers.size() && objectTrackers[activeTarget]) ? objectTrackers[activeTarget]->trackedResults[TRACKER_TYPE::DaSiamRPN].objectState.vel.y() : 0; }
float AutoTrackerController::getObjectVelX() { return (activeTarget >= 0 && activeTarget < objectTrackers.size() && objectTrackers[activeTarget]) ? objectTrackers[activeTarget]->objectMotion.measured.vel.x() : 0; }
float AutoTrackerController::getObjectVelY() { return (activeTarget >= 0 && activeTarget < objectTrackers.size() && objectTrackers[activeTarget]) ? objectTrackers[activeTarget]->objectMotion.measured.vel.y() : 0; }
float AutoTrackerController::getObjectAccelX() { return (activeTarget >= 0 && activeTarget < objectTrackers.size() && objectTrackers[activeTarget]) ? objectTrackers[activeTarget]->objectMotion.measured.accel.x() : 0; }
float AutoTrackerController::getObjectAccelY() { return (activeTarget >= 0 && activeTarget < objectTrackers.size() && objectTrackers[activeTarget]) ? objectTrackers[activeTarget]->objectMotion.measured.accel.y() : 0; }

// Crosshair and Target Interaction
int AutoTrackerController::isCrossHairOnTarget(int x, int y, bool useScaling)
{
    QPoint cp = useScaling ? QPoint(x * imgScalingFactor, y * imgScalingFactor) : QPoint(x, y);
    for (int i = 0; i < targetRects.size(); i++) {
        if (targetRects[i].isEmpty()) continue;
        if (targetRects[i].last().contains(cp)) return i;
    }
    return -1;
}

int AutoTrackerController::getTotalAtiveTracks()
{
    int count = 0;
    for (int i = 0; i < objectTrackers.size(); i++) {
        if (objectTrackers[i]->enabled) count++;
    }
    return count;
}

void AutoTrackerController::updateLocalValues()
{
    localValues[0] = trackingXErr;
    localValues[1] = trackingPIDxVal;
    localValues[2] = trackingYErr;
    localValues[3] = trackingPIDyVal;
    if (localValues.size() < 5 || activeTarget < 0) { localDataReady = true; return; }
    localValues[4] = localValues[5] = 0.0f;
    localValues[6] = objectTrackers[activeTarget]->daSiamRPNTracker->objectState.vel.x();
    localValues[7] = objectTrackers[activeTarget]->daSiamRPNTracker->objectState.vel.y();
    localValues[8] = objectTrackers[activeTarget]->objectMotion.measured.vel.x();
    localValues[9] = objectTrackers[activeTarget]->objectMotion.measured.vel.y();
    localDataReady = true;
}

void AutoTrackerController::cancelAllTargetRect()
{
    setUserEnableTracking(false);
    setTracking(false);
    for (int i = 0; i < objectTrackers.size(); i++) {
        cancelTargetRect(i);
    }
    activeTarget = -1;
}

int AutoTrackerController::getNextActiveTracker()
{
    int _activeTarget = activeTarget;
    int curr_active = activeTarget;
    for (int c = 0; c < (objectTrackers.size() * 3); c++) {
        _activeTarget++;
        if (_activeTarget >= objectTrackers.size()) { _activeTarget = 0; }
        if (_activeTarget == curr_active || objectTrackers[_activeTarget]->enabled) break;
    }
    return _activeTarget;
}

void AutoTrackerController::setTrackToCenter(bool val, int pointX, int pointY)
{
    trackToImageCenter = val;
    if (!trackToImageCenter) {
        trackToPoint.setX(pointX * imgScalingFactor);
        trackToPoint.setY(pointY * imgScalingFactor);
    }
}

void AutoTrackerController::switchToNextTarget() { setActiveTarget(getNextActiveTracker()); }

// External Data Accessors
double AutoTrackerController::getExtData(int index) { return (index < 0 || index >= extDataValues.size()) ? 0 : extDataValues[index]; }
bool AutoTrackerController::getShowOSD() { return (getExtData(EXT_DATA::SHOW_OSD) > 0.5); }
double AutoTrackerController::getHoriLead() { return getExtData(EXT_DATA::HORI_LEAD); }
double AutoTrackerController::getVertLead() { return getExtData(EXT_DATA::VERT_LEAD); }
QString AutoTrackerController::getAmmo() { return ammoTypes[getExtData(EXT_DATA::AMMO_TYPE)]; }
bool AutoTrackerController::getExtReady() { return (getExtData(EXT_DATA::READY_SIG) > 0.5); }
double AutoTrackerController::getExtDistance() { return getExtData(EXT_DATA::DISTANCE); }
int AutoTrackerController::getGunnerMode() { return getExtData(EXT_DATA::GUNNER_OP); }

// Crosshair and System Info
float AutoTrackerController::getCorssHairX() { return rch_move.x(); }
float AutoTrackerController::getCorssHairY() { return rch_move.y(); }
bool AutoTrackerController::isCorssHairMoving() { return movingCrossHair; }
QString AutoTrackerController::getSystemStatsStr() { return systemStatsStr; }

QString AutoTrackerController::getInfo()
{
    QString str;
    if (captureDevice) {
        str += "FPS  : " + QString::number(captureDevice->fps, 'f', 1) + "\n";
    }
    if (imageEnhancer) {
        str += "ET  : " + QString::number(imageEnhancer->pTime, 'f', 2) + " ms  |  ";
        if (imageEnhancer->colorTransfer) {
            str += "CT  : " + QString::number(imageEnhancer->colorTransfer->avg_processing_time, 'f', 2) + " ms  |  ";
        }
    }
    if (imageStabilization) {
        str += "IS : " + QString::number(imageStabilization->pTime, 'f', 2) + " ms\n";
    }
    return str;
}

QString AutoTrackerController::getTrackInfo()
{
    QString str;
    for (int i = 0; i < objectTrackers.size(); i++) {
        if (objectTrackers[i]->enabled) {
            str.append(QString::number(i) + "\n");
            str.append("DaSiamRPN: " + QString::number(objectTrackers[i]->daSiamRPNTracker->trackerSuccessful) + " | " + QString::number(objectTrackers[i]->daSiamRPNTracker->processingTime, 'f', 2) + "\n");
            str.append("TMT: " + QString::number(objectTrackers[i]->templateTracker->trackerSuccessful) + " | " + QString::number(objectTrackers[i]->templateTracker->processingTime, 'f', 2) + "\n");
            str.append("TRACK: " + QString::number(objectTrackers[i]->processingTime, 'f', 2) + "\n");
        }
    }
    return str;
}

QString AutoTrackerController::getSystemStatsInfoStr() { return systemStatsInfoStr; }

void AutoTrackerController::statsReady(QString stats)
{
    systemStatsStr = stats;
    systemStatsInfoStr = "CPU - U:  " + QByteArray::number(systemStatsReport->avg_cpuUsage * 100.0f, 'f', 1) + "  |  T:  " + QByteArray::number(systemStatsReport->avg_cpuTemp, 'f', 1) + "\n" +
                         "GPU - U:  " + QByteArray::number(systemStatsReport->gpu_usage, 'f', 1) + "  |  T:  " + QByteArray::number(systemStatsReport->gpu_temp, 'f', 1) + "  |  M:  " + QByteArray::number(systemStatsReport->gpu_memory, 'f', 1);
    systemStatsTele = QByteArray::number(systemStatsReport->avg_cpuUsage * 100.0f, 'f', 1) + "," +
                      QByteArray::number(systemStatsReport->avg_cpuTemp, 'f', 1) + "," +
                      QByteArray::number(systemStatsReport->ram_usage) + "," +
                      QByteArray::number(systemStatsReport->gpu_usage, 'f', 1) + "," +
                      QByteArray::number(systemStatsReport->gpu_temp, 'f', 1) + "," +
                      QByteArray::number(systemStatsReport->gpu_memory, 'f', 1);
}

// Image Processing Setters
void AutoTrackerController::setShowOri(bool val) { showOriginalImage = val; }
void AutoTrackerController::setApplyLut(bool val) { if (imageEnhancer) imageEnhancer->applyLuts = val; }
void AutoTrackerController::enableCLAHE(bool val) { if (imageEnhancer) imageEnhancer->applyCLAHE = val; }
void AutoTrackerController::setLutClaheFactor(double val) { if (imageEnhancer) { qDebug() << "setLutClaheFactor" << val; imageEnhancer->lut_clahe_factor = val; } }
void AutoTrackerController::setEnableFrameBlending(bool val) { if (imageStabilization) imageStabilization->useFrameSampling = val; }
void AutoTrackerController::setEnableSharpen(bool val) { if (imageEnhancer) imageEnhancer->enableSharpen = val; }
void AutoTrackerController::setPreNR(bool val) { if (imageEnhancer) imageEnhancer->applyPreNR = val; }
void AutoTrackerController::setPostNR(bool val) { if (imageEnhancer) imageEnhancer->applyPostNR = val; }
void AutoTrackerController::setFrameSamplingRate(float val) { if (imageStabilization) imageStabilization->frameSamplingRate = val; }
void AutoTrackerController::setLutAdaption(double val) { if (imageEnhancer) imageEnhancer->lut_lpf = val; }
void AutoTrackerController::setLutSmoothingWindow(int winSize) { if (imageEnhancer && imageEnhancer->colorTransfer) imageEnhancer->colorTransfer->sWin = winSize; }
void AutoTrackerController::setLutStrength(float val) { if (imageEnhancer) imageEnhancer->lutStrength = val; }
void AutoTrackerController::setNoiseH(float _noiseH) { if (imageEnhancer) imageEnhancer->noiseH = _noiseH; }
void AutoTrackerController::setNoiseSearchWindow(int _searchWin) { if (imageEnhancer) imageEnhancer->noiseSearchWindow = _searchWin; }
void AutoTrackerController::setNoiseBlockSize(int _noiseBlock) { if (imageEnhancer) imageEnhancer->noiseBlockSize = _noiseBlock; }
void AutoTrackerController::setPreProcessingEffectStrength(double _val) { if (imageEnhancer) imageEnhancer->effectStrength = _val; }

// Miscellaneous
void AutoTrackerController::setConsole(QString, float) {}
double AutoTrackerController::getOutputValues(int) { return 0.0f; }
void AutoTrackerController::setGuiJoystick(float xVal, float yVal) { guiJoyXVal = xVal; guiJoyYVal = yVal; }
void AutoTrackerController::setRemoteGuiJoy(float xVal, float yVal) { remoteGuiJoyXVal = xVal; remoteGuiJoyYVal = yVal; remoteJoystickTime = QTime::currentTime(); }
