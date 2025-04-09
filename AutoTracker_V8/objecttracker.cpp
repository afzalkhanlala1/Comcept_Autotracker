#include "objecttracker.h"
using namespace std;

ObjectTracker::ObjectTracker(QObject *parent) : QObject{parent}
{
    trackingStatusTimer = new QTimer(this);
    connect(trackingStatusTimer, &QTimer::timeout, this, &ObjectTracker::checkTrackingStatus);
    falseStatusCount = 0; // Initialize count
}

ObjectTracker::~ObjectTracker()
{
    if(daSiamRPNThread.isRunning()) { daSiamRPNThread.quit(); daSiamRPNThread.wait(); }
    if(templateMatchThread.isRunning()) { templateMatchThread.quit(); templateMatchThread.wait(); }
    if(yoloClientThread.isRunning()) { yoloClientThread.quit(); yoloClientThread.wait();  }
}

void ObjectTracker::init()
{
    cout << "Object Tracker - Init: " << QThread::currentThreadId() << endl;

    roi = QRectF(0, 0, 0, 0);
    objectTrackedRect = QRectF(0, 0, 0, 0);
    targetSizeLimits = QVector<QVector3D>((int)TARGETS::target_count);
    updateVelAccelLimits();
    updateSizeLimits();

    //---------


    daSiamRPNTracker = new DaSiamRPNTracker;
    if(enableDaSiamRPN)
    {
        connect(this, &ObjectTracker::initTrackers, daSiamRPNTracker, &DaSiamRPNTracker::init);
        connect(this, &ObjectTracker::trackFeatures, daSiamRPNTracker, &DaSiamRPNTracker::track);
        connect(this, &ObjectTracker::setTrackersROI, daSiamRPNTracker, &DaSiamRPNTracker::setROI);
        connect(daSiamRPNTracker, &DaSiamRPNTracker::tracked, this, &ObjectTracker::trackerFinished);

        connect(this,&ObjectTracker::Dasiam_setROI, daSiamRPNTracker, &DaSiamRPNTracker::setROI);

        daSiamRPNTracker->moveToThread(&daSiamRPNThread);
        daSiamRPNThread.start(QThread::HighestPriority);
        trackerCount++;
    }

    //---------

    templateTracker = new TemplateMatchTracker;
    if(enableTM)
    {
        connect(this, &ObjectTracker::initTrackers, templateTracker, &TemplateMatchTracker::init);
        connect(this, &ObjectTracker::trackFeatures, templateTracker, &TemplateMatchTracker::track);
        connect(this, &ObjectTracker::setTrackersROI, templateTracker, &TemplateMatchTracker::setROI);
        connect(this, &ObjectTracker::templateUpdatePos, templateTracker, &TemplateMatchTracker::updatePos);
        connect(templateTracker, &TemplateMatchTracker::tracked, this, &ObjectTracker::trackerFinished);

        connect(this,&ObjectTracker::template_setROI, templateTracker, &TemplateMatchTracker::setROI);

        templateTracker->moveToThread(&templateMatchThread);
        templateMatchThread.start(QThread::HighestPriority);
        trackerCount++;
    }

    //    //---------

    yoloClientTracker = new YoloClientTracker;
    if(enableYOLOClient)
    {
        connect(this, &ObjectTracker::initTrackers, yoloClientTracker, &YoloClientTracker::init);
        connect(this, &ObjectTracker::trackFeatures, yoloClientTracker, &YoloClientTracker::track);
        connect(this, &ObjectTracker::setTrackersROI, yoloClientTracker, &YoloClientTracker::setROI);
        connect(yoloClientTracker, &YoloClientTracker::tracked, this, &ObjectTracker::trackerFinished);
        connect(yoloClientTracker, &YoloClientTracker::imagelabeled, this, &ObjectTracker::labelImage);

        yoloClientTracker->moveToThread(&yoloClientThread);
        yoloClientThread.start(QThread::HighestPriority);
        trackerCount++;
        qDebug() << "yolo client connected";
    }

    //---------

    objectTrackedRect = QRect(288, 224, 64, 48);
    trackedRects = QVector<QRectF>(TRACKER_TYPE::tt_count);
    trackedResults = QVector<TrackedResult>(TRACKER_TYPE::tt_count);

    autoLock = new AutoLock;
    connect(this, &ObjectTracker::initAutoLock, autoLock, &AutoLock::init);
    connect(this, &ObjectTracker::autoLockTarget, autoLock, &AutoLock::lockTarget);
    connect(autoLock, &AutoLock::possibleTarget, this, &ObjectTracker::possibleTarget);

    pTimer = new QElapsedTimer;

    emit initTrackers();
    emit initAutoLock();
    initialized = true;
}

void ObjectTracker::trackImageFeatures(TrackerFrame trackerFrame)
{
    if(!initialized /*|| !enabled || !roiSet*/) return;
    pTimer->start();
    frame_dt = trackerFrame.dt;
    frame = trackerFrame.lutFrame.copy();
    frameImg = trackerFrame.stabImage.copy();
    enableTrackers();

    if (trackerFrame.frame.format()== QImage::Format_RGB888) { enableTM = false; }

    if(trackerFrame.useIS && !trackerFrame.stabImage.isNull())
    {
        if(!trackerFrame.stabImage.isNull())
        {
            frameCenter = trackerFrame.stabImage.rect().center();
            imageSize = trackerFrame.stabImage.size();            emit trackFeatures(trackerFrame.stabImage, frame_dt);
        }
    }

    else if(trackerFrame.useLutFrame && !trackerFrame.lutFrame.isNull())
    {
        frameCenter = trackerFrame.lutFrame.rect().center();
        imageSize = trackerFrame.lutFrame.size();
        emit trackFeatures(trackerFrame.lutFrame, frame_dt);
    }

    else if(!trackerFrame.frame.isNull())
    {
        frameCenter = trackerFrame.lutFrame.rect().center();
        imageSize = trackerFrame.lutFrame.size();
        emit trackFeatures(trackerFrame.frame, frame_dt);
    }
}

void ObjectTracker::trackerFinished(QRectF trackedRect, ObjectState objectState, bool success, int tracker_type) {
    QMutexLocker locker(&mutex);
    trackedRects[tracker_type] = trackedRect;
    trackedResults[tracker_type].rect = trackedRect;
    trackedResults[tracker_type].objectState = objectState;
    trackedResults[tracker_type].success = success;

    worldFrameVel = (worldFrameVel * 0.6f) + (((_worldDisp - worldFrameDisp) / frame_dt) * 0.4f);
    worldFrameDisp = _worldDisp;
    world_x += worldFrameDisp.x();
    world_y += worldFrameDisp.y();

    frameCount++;
    frameNum++;

    combineTrackers(tracker_type);

    objectTracking_Rect = selectTrackedRect(); // Replace if/else with priority selection

    // Constrain the rectangle
    if (objectTracking_Rect.x() < 0) objectTracking_Rect.translate(-objectTracking_Rect.x(), 0);
    else if (objectTracking_Rect.right() > imageSize.width()) objectTracking_Rect.translate(imageSize.width() - objectTracking_Rect.right(), 0);
    if (objectTracking_Rect.y() < 0) objectTracking_Rect.translate(0, -objectTracking_Rect.y());
    else if (objectTracking_Rect.bottom() > imageSize.height()) objectTracking_Rect.translate(0, imageSize.height() - objectTracking_Rect.bottom());

    trackedRects[FUSION] = objectTracking_Rect;
    if (trackedRects[FUSION].isValid() && myIndex >= 0 && myIndex < trackedRects.size()) {
        emit imageTracked(trackedRects, myIndex);
    } else {
        qDebug() << "Invalid tracked rect or index";
    }

    processingTime = (processingTime * 0.9f) + ((pTimer->nsecsElapsed() / 1000000000.0f) * 0.1f);
}

QRectF ObjectTracker::selectTrackedRect() {
    if (enableDaSiamRPN && enableTM && objectRectSuccess) {
        return objectTrackedRect; // Fusion result
    }
    if (enableDaSiamRPN && trackedResults[TRACKER_TYPE::DaSiamRPN].success) {
        dasiumTracking();
        return objectTrackedRect;
    }
    if (enableTM && trackedResults[TRACKER_TYPE::TEMPLATE_MATCHER].success) {
        return trackedResults[TRACKER_TYPE::TEMPLATE_MATCHER].rect;
    }
    return SE_TrackedRectF2; // Fallback to state estimator
}

void ObjectTracker::setROI(QRect rect, int index)
{
    trackID = -1;

    if(index != myIndex) return;

    for(int i = 0; i < trackedRects.size(); i++) {
        trackedRects[i] = trackedResults[i].rect = rect;
    }

    performRegionGrowing(rect);
    boundingBox2 = rect;
    rect =  boundingBox2;

    if (rect.isValid()) {
        emit setTrackersROI(rect);
        tejectory.clear();
        SE_TrackedRectF2 = rect;
    }

    else { }

    if (rect.isValid())  { roiRect = rect; /*setRoiRect = true;*/}

    enabled = roiSet = true;
}

QVector<QPair<int, int>> getNeighbors(int y, int x) {
    return {{y-1, x}, {y+1, x}, {y, x-1}, {y, x+1}};
}

void ObjectTracker::performRegionGrowing(QRect rect)
{
    QRect outRectmarked = rect;
    QImage image = frame.copy();

    if (image.isNull()) return;

    cv::Mat matImage(image.height(), image.width(), CV_8UC1, image.bits(), image.bytesPerLine());
    if (matImage.empty()) return;

    cv::Mat segmented = cv::Mat::zeros(image.height(), image.width(), CV_8U);

    int rows = image.height();
    int cols = image.width();

    QPoint seedPoint(outRectmarked.center().x(), outRectmarked.center().y());
    int seedValue = matImage.at<uchar>(seedPoint.y(), seedPoint.x());
    int threshold = 30;

    QStack<QPoint> stack;
    stack.push(seedPoint);
    segmented.at<uchar>(seedPoint.y(), seedPoint.x()) = 255;
    while (!stack.isEmpty()) {
        QPoint p = stack.pop();

        QVector<QPair<int, int>> neighbors = getNeighbors(p.y(), p.x());
        for (const QPair<int, int> &neighbor : neighbors) {
            int ny = neighbor.first;
            int nx = neighbor.second;

            if (ny >= 0 && ny < rows && nx >= 0 && nx < cols &&
                segmented.at<uchar>(ny, nx) == 0 &&
                std::abs(matImage.at<uchar>(ny, nx) - seedValue) <= threshold) {
                segmented.at<uchar>(ny, nx) = 255;
                stack.push(QPoint(nx, ny));
            }
        }
    }

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(segmented, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    width = displayScreenSize.width();
    height = displayScreenSize.height();

    //     qDebug() << "w" << width << "he" << height;

    //    float width = 1920;
    //    float height = 1080;

    if (!contours.empty()) {
        double area;
        for (int i = 0; i < contours.size(); ++i) {
            std::vector<cv::Point> contour = contours[i];
            area = cv::contourArea(contour);
            boundingBox = cv::boundingRect(contour);
        }

        //        if (!((area > 500 && boundingBox.width >(width/20) && boundingBox.width < (width/3)) && (boundingBox.height > (height/10) && boundingBox.height < (height/4))))

            //                if ((boundingBox.width > (width/5) || boundingBox.width < (width/28)) || (boundingBox.height > (height/4) || boundingBox.height < (height/18)))

        if ((boundingBox.width > (width/6) || boundingBox.width < (width/14)) || (boundingBox.height > (height/5) || boundingBox.height < (height/25)))
        {
            //            qDebug() << "a";
            QPoint centerPoint = outRectmarked.center();
            //                                        int newWidth = std::min(boundingBox.width, 300);
            //                                        int newHeight = std::min(boundingBox.height, 150);


            //            int newWidth = std::min(boundingBox.width, 200);
            //            int newHeight = std::min(boundingBox.height, 180);
            int newWidth ;
            int newHeight;
            newWidth = width/9;
            newHeight =  height/14;

            //            if (newWidth < (width/10)) { newWidth = width/10; }
            //            if (newHeight < (height/25)) { newHeight =  height/25;}

            //            qDebug() << "newwidth" << newWidth;
            //            qDebug() << "newheight" << newHeight;

            int newX = centerPoint.x() - newWidth / 2;
            int newY = centerPoint.y() - newHeight / 2;

            boundingBox = cv::Rect(newX, newY, newWidth, newHeight);

            //            QPoint centerPoint = outRectmarked.center();
            //            int newWidth = width / 6;
            //            int newHeight = height / 6;
            //            int newX = centerPoint.x() - newWidth / 2;
            //            int newY = centerPoint.y() - newHeight / 2;
            //            boundingBox = cv::Rect(newX, newY, newWidth, newHeight);
        }
        //        else {  qDebug() << "b"; }
    }

    else {
        //        qDebug() << "c";
        QPoint centerPoint = outRectmarked.center();
        int newWidth = width / 6;
        int newHeight = height / 6;
        int newX = centerPoint.x() - newWidth / 2;
        int newY = centerPoint.y() - newHeight / 2;
        boundingBox = cv::Rect(newX, newY, newWidth, newHeight);
    }

    boundingBox2 = QRect(boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height);
}

void ObjectTracker::possibleTarget(QRectF target, float confidence)
{

}

void ObjectTracker::enableTrackers()
{
    daSiamRPNTracker->enabled = enableDaSiamRPN && enabled;
    templateTracker->enabled = enableTM && enabled;
    //    yoloClientTracker->enabled = enableYOLOClient && enabled;
}

void ObjectTracker::setParam(QStringList params, int tracker_type)
{
    //    if(tracker_type == (int)TRACKER_TYPE::CSRT) { emit setCSRTParam(params); }
    //    else if(tracker_type == (int)TRACKER_TYPE::KCF) { emit setKCFParam(params); }
}

void ObjectTracker::setPreviewRectSize(QRectF imgRect)
{
    //    qDebug() << "ImagePreviewRect: " << imgRect.width() << ", " << imgRect.height() << endl;
    displayScreenSize = imgRect.toRect().size();
}

void ObjectTracker::setscale(float w, float h)
{
    //    width = w;
    //    height = h;
}

void ObjectTracker::cancelTracking()
{
    enabled = roiSet = false;
    //    emit cancelbb();
    //    previousRect = QRectF();
    //    objectTracking_Rect = QRectF();
}

void ObjectTracker::receiveResults(const QVector<YoloResult>& scaledResults)
{
    scaledYoloResults = scaledResults;
}

void ObjectTracker::labelImage(const QVector<YoloResult>& results)
{
    qDebug() << "ReceivedYOLO detections";

    // Process the results as needed
    yoloResults = results;
    emit yoloResultsReady(results);

    // If tracking a specific ID, find it in results
    if (trackID != -1) {
        for (const YoloResult& result : results) {
            if (result.labelID == trackID) {
                // Found our tracked object
                yoloSuccess = true;
                overlapYoloRect = result.bbox;
                break;
            }
        }
    } else {
        // No specific ID tracked yet, check if any result overlaps with ROI
        if (roiRect.isValid()) {
            for (const YoloResult& result : results) {
                QRectF intersection = result.bbox.intersected(roiRect);
                float area = intersection.width() * intersection.height();
                float roiArea = roiRect.width() * roiRect.height();

                if (area > 0.3 * roiArea) {
                    // This detection significantly overlaps with ROI
                    yoloSuccess = true;
                    trackID = result.labelID; // Start tracking this ID
                    overlapYoloRect = result.bbox;
                    break;
                }
            }
        }
    }
}

void ObjectTracker::setTargetRange(float val)
{
    target_range = val;
    validRange = ((target_range > 100) && (target_range < 4000));

    if(validRange)
    {
        updateVelAccelLimits();
        updateSizeLimits();
    }
}

void ObjectTracker::cancelRect(int rectindex)
{
    emit cancelyolorect(rectindex);
}

void ObjectTracker::opticalFlowVelocties(OPResult _opResult)
{
    //    if(opTracker == nullptr || !enableOP) return;
    //    if(!opTracker->useSpareOP) { emit processOFVel(_opResult); }
}

void ObjectTracker::opticalFlowImage(QImage image, double _dt)
{
    //    if(opTracker == nullptr || !enableOP) return;
    //    if(!opTracker->useSpareOP) { emit processOFImage(image, _dt); }
}

void ObjectTracker::checkTrackingStatus() {
    if (enableTM == true && enableDaSiamRPN == false)
    {
        if (trackedResults[TEMPLATE_MATCHER].success == true) { falseStatusCount = 0; }
        else
        {
            falseStatusCount++;
            if (falseStatusCount >= 3) {
                cancelTracking();
                qDebug() << "Tracker Fail. Set Roi again.";
                trackingStatusTimer->stop();
            }
        }
    }

    else if (enableDaSiamRPN == true && enableTM == false)
    {
        if (trackedResults[DaSiamRPN].success = true) { falseStatusCount = 0; }
        else
        {
            falseStatusCount++;
            //            if (falseStatusCount >= requiredFalseCount) {
            if (falseStatusCount >= 20) {
                //                qDebug() << 'd';
                cancelTracking();
                qDebug() << "Tracker Fail. Set Roi again.";
                trackingStatusTimer->stop();
            }
        }
    }

    else if (enableDaSiamRPN == true && enableTM == true && enableYOLOClient == false)
    {
        if (objectRectSuccess) {
            falseStatusCount = 0; // Reset the count if success
        }
        else
        {
            falseStatusCount++;
            if (falseStatusCount >= 50) {
                //                trackingStatusTimer->stop();
                cancelTracking();
                qDebug() << "Tracker Fail. Set Roi again.";
                trackingStatusTimer->stop();
            }
        }
    }

    else if (enableYOLOClient == true && enableDaSiamRPN == true && enableTM == true)
    {
        if (objectRectSuccess || yoloSuccess) {
            falseStatusCount = 0; // Reset the count if success
        }

        else if (objectRectSuccess == false && yoloSuccess == false)
        {
            falseStatusCount++;
            //                        if (falseStatusCount >= 40 & falseStatusCount < 45) {
            if (falseStatusCount >= 15 & falseStatusCount < 17) {
                cancelTracking();
                falseStatusCount = 0;

                qDebug() << "Tracker Fail. Set Roi again.";
                trackingStatusTimer->stop();
            }
        }
        else { }
    }
    else { return; }
}

void ObjectTracker::combineTrackers(int tracker_type)
{
    if (enableTM == true && enableDaSiamRPN == false) { objectTrackedRect = trackedResults[TRACKER_TYPE::TEMPLATE_MATCHER].rect;
        if (TMsuccess == true) { if (trackingStatusTimer->isActive()) { trackingStatusTimer->stop(); }}
        else { if (!trackingStatusTimer->isActive()) { trackingStatusTimer->start(1000); }}
    }

    else if (enableDaSiamRPN == true && enableTM == false) {/* objectTrackedRect = trackedResults[TRACKER_TYPE::DaSiamRPN].rect;*/
        dasiumTracking();
        if (trackedResults[DaSiamRPN].success == true) { if (trackingStatusTimer->isActive()) { trackingStatusTimer->stop(); }}
        else { if (!trackingStatusTimer->isActive()) { trackingStatusTimer->start(1000); }}
    }

    else if (enableDaSiamRPN == true && enableTM == true) {

        Das_Success = trackedResults[DaSiamRPN].success;
        TMsuccess = trackedResults[TEMPLATE_MATCHER].success;

        rect1 = trackedResults[TRACKER_TYPE::DaSiamRPN].rect;
        rect2 = trackedResults[TRACKER_TYPE::TEMPLATE_MATCHER].rect;

        if (rect1.isValid() && rect2.isValid())
        {
            if (frameCount < 10)
            {
                objectTrackedRect = rect1; //Dasium
            }
            else if (frameCount >= 10)
            {
                dasRectsHist.append(rect1);
                if (frameCount % dasHistCount == 0)
                {
                    while (dasRectsHist.size() > dasHistCount) {
                        dasRectsHist.removeFirst();
                    }
                    avgSize = QSize(0,0);
                    avgPos = QPointF(0,0);
                    for (int i = 0; i < dasRectsHist.size(); ++i)
                    {
                        avgSize += dasRectsHist[i].size();
                        avgPos += dasRectsHist[i].center();
                    }
                    avgPos /= dasRectsHist.size();
                    avgSize /= dasRectsHist.size();
                    topLeft = QPointF(avgPos.x() - (avgSize.width() / 2), avgPos.y() - (avgSize.height() / 2));
                    avgRect = QRectF(topLeft, avgSize);
                    emit template_setROI(avgRect.toRect());   //updating Dasium ROI after every 10 frames
                }

                float SE_area = SE_TrackedRectF2.width() * SE_TrackedRectF2.height(); //areas and intersections
                float areaThresh = thresh_val * SE_area;
                QRectF DasRPN_SE_IntersectedRect = rect1.intersected(SE_TrackedRectF2);
                qreal DasRPN_SE_Area = DasRPN_SE_IntersectedRect.width() * DasRPN_SE_IntersectedRect.height();
                QRectF TM_SE_IntersectedRect = rect2.intersected(SE_TrackedRectF2);
                qreal TM_SE_Area = TM_SE_IntersectedRect.width() * TM_SE_IntersectedRect.height();

                if (DasRPN_SE_Area > areaThresh && Das_Success == true)  { DasiamSuccess = true; }
                else { DasiamSuccess = false; }

                weight1 = DasRPN_SE_Area / (DasRPN_SE_Area + TM_SE_Area);
                weight2 = TM_SE_Area / (DasRPN_SE_Area + TM_SE_Area);

                QPointF topLeft = (rect1.topLeft() * weight1) + (rect2.topLeft() * weight2);
                QSizeF size = (rect1.size() * weight1) + (rect2.size() * weight2);

                if (DasRPN_SE_Area == 0 && TM_SE_Area == 0)
                {
                    if (SE_TrackedRectF.isValid()) {
                        /*  objectTrackedRect = SE_TrackedRectF;*/ //OutputRect shifts to SE rect if both trackers fail(no inter with SE rect)
                        objectTrackedRect = SE_TrackedRectF2;
                    }
                    else { }
                }

                else
                {
                    if (size.width() > 0 && size.height() > 0) {
                        if (size.width() > (imageSize.width() / 6)) { size.setWidth(imageSize.width() / 6); }
                        if (size.height() > (imageSize.height() / 5)) {  size.setHeight(imageSize.height() / 5); }

                        objectTrackedRect = QRectF(topLeft, size); } //OutputRect is weighted rect of both trackers
                    //                    qDebug() << "crash2";
                }
            }

            if (DasiamSuccess == true && TMsuccess == true) {
                objectRectSuccess = true;
                falsecount = 0;
                if (trackingStatusTimer->isActive()) {
                    trackingStatusTimer->stop();
                }
            }

            else {
                objectRectSuccess = false;
                falsecount++;
                if (!trackingStatusTimer->isActive()) {
                    trackingStatusTimer->start(300);
                }
            }

            update_motion(objectTrackedRect);  //trajectory points and estimating average displacement

            if (lastobjectTrackedRect.isEmpty()) {lastobjectTrackedRect = objectTrackedRect;}

            state_estimator_output(lastobjectTrackedRect);  //State Estimator Rect from displacement

            qreal smoothedX = (objectTrackedRect.x() + lastobjectTrackedRect .x()) / 2;
            qreal smoothedY = (objectTrackedRect.y() + lastobjectTrackedRect .y()) / 2;
            qreal smoothedWidth = (objectTrackedRect.width() + lastobjectTrackedRect .width()) / 2;
            qreal smoothedHeight = (objectTrackedRect.height() + lastobjectTrackedRect .height()) / 2;

            objectTrackedRect.setRect(smoothedX, smoothedY, smoothedWidth, smoothedHeight);
            //            qDebug() << "wi" << width << "hi" << height  ;

            if (objectTrackedRect.width() < (width/15) ) { objectTrackedRect.setWidth(width/15);  }
            if (objectTrackedRect.height() < (height/26) ) { objectTrackedRect.setHeight(height/26);  }

            lastobjectTrackedRect = objectTrackedRect;
        }
        else {  }
    }
    else{  }
}


void ObjectTracker::dasiumTracking()
{
    Das_Success = trackedResults[DaSiamRPN].success;
    //    qDebug() << 'succ' << Das_Success;

    QRectF currentRect = trackedResults[TRACKER_TYPE::DaSiamRPN].rect;

    if (previousRect.isEmpty()) {
        previousRect = currentRect;
    }

    const int MAX_SIZE_CHANGE = 30;
    //    const int MAX_POSITION_CHANGE = 25;
    const int MAX_POSITION_CHANGE = 50;

    //    qDebug() << "curr width" << currentRect.width();
    //    qDebug() << "prev width" << previousRect.width();

    if (abs(currentRect.width() - previousRect.width()) > MAX_SIZE_CHANGE ||
        abs(currentRect.height() - previousRect.height()) > MAX_SIZE_CHANGE) {
        currentRect.setSize(previousRect.size());
    }

    //    if (abs(currentRect.x() - previousRect.x()) > MAX_POSITION_CHANGE ||
    //        abs(currentRect.y() - previousRect.y()) > MAX_POSITION_CHANGE) {
    //        currentRect.moveTo(previousRect.topLeft());
    //    }

    //    const qreal MAX_WIDTH = boundingBox2.width() * 1.3;
    //    const qreal MAX_HEIGHT = boundingBox2.height() * 1.3;

    //    if (currentRect.width() > MAX_WIDTH) {
    //        currentRect.setWidth(MAX_WIDTH);
    //    }

    //    if (currentRect.height() > MAX_HEIGHT) {
    //        currentRect.setHeight(MAX_HEIGHT);
    //    }

    qreal smoothedX = (currentRect.x() + previousRect.x()) / 2;
    qreal smoothedY = (currentRect.y() + previousRect.y()) / 2;
    qreal smoothedWidth = (currentRect.width() + previousRect.width()) / 2;
    qreal smoothedHeight = (currentRect.height() + previousRect.height()) / 2;

    currentRect.setRect(smoothedX, smoothedY, smoothedWidth, smoothedHeight);

    //    if (abs(currentRect.x() - previousRect.x()) > MAX_POSITION_CHANGE ||
    //        abs(currentRect.y() - previousRect.y()) > 70) {

    //        currentRect.moveTo(previousRect.topLeft());
    ////        emit setTrackersROI(currentRect.toRect());
    ///
    ///
    //    }

    const qreal MAX_WIDTH = boundingBox2.width() * 1.3;
    const qreal MAX_HEIGHT = boundingBox2.height() * 1.2;

    if (currentRect.width() > MAX_WIDTH) {
        currentRect.setWidth(MAX_WIDTH);
    }

    if (currentRect.height() > MAX_HEIGHT) {
        currentRect.setHeight(MAX_HEIGHT);
    }

    objectTrackedRect = currentRect;
    previousRect = currentRect;
}

void ObjectTracker::update_motion(QRectF objectTrackedRect)
{
    if (prevobjectTrackedRect.isEmpty()) { prevobjectTrackedRect = objectTrackedRect; }

    if (objectRectSuccess == true)
    {
        //         ot_curr_vel = QVector2D(objectTrackedRect.center() - prevobjectTrackedRect.center()) / (frameThresh * frame_dt);
        ot_curr_vel = QVector2D(objectTrackedRect.center() - prevobjectTrackedRect.center()) / (frame_dt);
        ot_curr_vel += worldFrameVel;

        if(abs(ot_curr_vel.length()) > velocityThresh) { ot_curr_vel *= velocityThresh / abs(ot_curr_vel.length()); }
        ot_curr_accel = ( ot_curr_vel - ot_vel ) / frame_dt;
        if(abs(ot_curr_accel.length()) > accelThresh) { ot_curr_accel *= accelThresh / abs(ot_curr_accel.length()); }
        ot_vel = (ot_vel*0.7f) + (ot_curr_vel*0.3f);
        ot_accel = (ot_accel*0.7f) + (ot_curr_accel*0.3f);

        if ((frameNum - lastframeNum) >= frameThresh)
        {
            TrackHistroy hist;
            hist.pos = prevobjectTrackedRect;
            hist.time = (double)dtTimer.nsecsElapsed()/1000000000.0f;
            hist.worldDisplacement = QVector2D(world_x, world_y);
            tejectory.append(hist);
            if(tejectory.size() > maxTejectoryHistory) {tejectory.takeFirst();}
            lastframeNum = frameNum;
        }
        prevobjectTrackedRect = objectTrackedRect;
    }

    else {
        ot_curr_vel = ct_vel;
        if(tejectory.size() > 2)
        {
            QVector2D sum_vel(0, 0);
            QVector2D sum_accel(0, 0);
            QVector2D c_vel, l_vel;
            double vel_count = 0.0f, accel_count = 0.0f;

            for(int i = 1; i < tejectory.size(); i++)
            {
                _dt = (tejectory[i].time - tejectory[i-1].time);
                c_vel = QVector2D((tejectory[i].pos.center() - tejectory[i-1].pos.center()) / _dt);

                sum_vel += c_vel;

                if(i >= 2) { sum_accel += QVector2D((c_vel - l_vel) / _dt); accel_count++; }

                l_vel = c_vel;
                vel_count++;
            }

            ct_vel = (sum_vel / vel_count);
            //            if(accel_count > 1.0f) { ct_accel = (sum_accel) / accel_count; }
            if(accel_count > 1.0f) { ot_curr_accel = (sum_accel) / accel_count; }
            ot_vel = (ot_vel*0.80f) + (ot_curr_vel*0.2f);
            ot_accel = (ot_accel*0.80f) + (ot_curr_accel*0.2f);
            //            ot_vel = (ot_vel*0.7f) + (ot_curr_vel*0.3f);
            //            ot_accel = (ot_accel*0.7f) + (ot_curr_accel*0.3f);
        }
    }

    displacement = (ot_vel * frame_dt) + (ot_accel * frame_dt * frame_dt * 0.5f);

    ot_accel *= 0.999;
    ot_vel *= 0.999;

    final_displacement = displacement;
}

void ObjectTracker::state_estimator_output(QRectF lastobjectTrackedRect)
{
    if (SE_TrackedRectF2.isEmpty()) {SE_TrackedRectF2 = lastobjectTrackedRect;}

    if (objectRectSuccess == true)   //correcting SE error
    {
        SE_TrackedRectF2.translate(final_displacement.toPointF());
        SE_TrackedRectF3 = SE_TrackedRectF2;
        err = objectTrackedRect.center() - SE_TrackedRectF3.center();
        QPointF err_gain = err * gain;
        SE_TrackedRectF3.translate(err_gain);
        SE_TrackedRectF = SE_TrackedRectF3;
    }

    else
    {
        if (final_displacement.x() > 2) {final_displacement.setX(2); }
        if (final_displacement.y() > 2) {final_displacement.setY(2); }

        //        qDebug() << "f_dis" << final_displacement.x() << final_displacement.y();
        //        SE_TrackedRectF2.translate((final_displacement).toPointF());

        SE_TrackedRectF2.translate(final_displacement.x()/4, final_displacement.y()/10);
        SE_TrackedRectF = SE_TrackedRectF2;
    }

    if(SE_TrackedRectF.x() < 0) { SE_TrackedRectF.translate(-SE_TrackedRectF.x(), 0); }
    else if(SE_TrackedRectF.right() >  imageSize.width()) { SE_TrackedRectF.translate(imageSize.width() - SE_TrackedRectF.right(), 0); }

    if(SE_TrackedRectF.y() < 0) {SE_TrackedRectF.translate(0, -SE_TrackedRectF.y()); }
    else if(SE_TrackedRectF.bottom() >  imageSize.height()) { SE_TrackedRectF.translate(0, imageSize.height() - SE_TrackedRectF.bottom()); }

    //    trackedRects[State_EST] = SE_TrackedRectF;
    SE_TrackedRectF2 = SE_TrackedRectF;
}

void ObjectTracker::outRectReceived(QRect outRect)
{
    outRect2 = outRect;
    outrectreceived = true;
    //    qDebug() << "cen" << outRect2.center();
    //    getObjectRect(outRect);
    //    performRegionGrowing(outRect2);
}

void ObjectTracker::getYoloRects()
{
    expandedRoi = outRect2;
    for (YoloResult &result : filteredResults) {
        if (expandedRoi.intersects(result.bbox))
        {
            classID = result.label;
            trackID = result.labelID;
            trackIDfound = true;
            overlapYoloRect = result.bbox;
        }
        else { }
    }
    newtrackID = trackID;
}


QRectF ObjectTracker::constrainTrackedRect(QRectF rect, int tracker_type)
{
    return rect;
    return QRectF(trackedRects[tracker_type].x() + constrain((float)(rect.x()-trackedRects[tracker_type].x()), -motionLimits.xAccelMax, motionLimits.xAccelMax),
                  trackedRects[tracker_type].y() + constrain((float)(rect.y()-trackedRects[tracker_type].y()), -motionLimits.yAccelMax, motionLimits.yAccelMax),
                  constrain((float)rect.width(), targetSizeLimits[TARGETS::TANK].z()*0.8f, targetSizeLimits[TARGETS::TANK].x()*1.2f),
                  constrain((float)rect.height(), targetSizeLimits[TARGETS::TANK].y()*0.8f, targetSizeLimits[TARGETS::TANK].y()*1.2f) );
}

void ObjectTracker::updateVelAccelLimits()
{
    motionLimits.xVelMin = kmh_to_pixelsPerFrame(minTargetSpeed_kmh, target_range);
    motionLimits.xVelMax = kmh_to_pixelsPerFrame(maxTargetSpeed_kmh, target_range);
    motionLimits.xAccelMin = 0.0f;
    motionLimits.xAccelMax = kmh_to_pixelsPerFrame(15.0f, target_range);

    motionLimits.yVelMin = kmh_to_pixelsPerFrame(minTargetSpeed_kmh, target_range);
    motionLimits.yVelMax = kmh_to_pixelsPerFrame(maxTargetSpeed_kmh*0.5f, target_range);
    motionLimits.yAccelMin = 0.0f;
    motionLimits.yAccelMax = kmh_to_pixelsPerFrame(5.0f, target_range);
}

void ObjectTracker::updateSizeLimits()
{
    QVector2D fov = rangeToFieldOfView(target_range);
    targetSizeLimits[TANK].setX((targetRefSize.tank.x() / fov.x()) * imageSize.width());
    targetSizeLimits[TANK].setY((targetRefSize.tank.y() / fov.y()) * imageSize.height());
    targetSizeLimits[TANK].setZ((targetRefSize.tank.z() / fov.x()) * imageSize.width());
}
