#include "dasiamrpntracker.h"

using namespace std;

DaSiamRPNTracker::DaSiamRPNTracker(QObject *parent) : QObject{parent}
{

}

void DaSiamRPNTracker::init()
{
    QString modelsPath = QDir::homePath() + "/Documents/AutoTracker/TrackerModels/";
    daSiamRPNParams.model = QString(modelsPath + "dasiamrpn_model.onnx").toStdString().c_str();
    daSiamRPNParams.kernel_r1 = QString(modelsPath + "dasiamrpn_kernel_r1.onnx").toStdString().c_str();
    daSiamRPNParams.kernel_cls1 = QString(modelsPath + "dasiamrpn_kernel_cls1.onnx").toStdString().c_str();
#ifdef USE_GPU
    daSiamRPNParams.backend = 5;
    daSiamRPNParams.target = 6;
#endif
    tracker = cv::TrackerDaSiamRPN::create(daSiamRPNParams);
    trackerCreated = initialized = true;
}

void DaSiamRPNTracker::setROI(QRect rect)
{
//     qDebug() << "update";
    if(!trackerCreated) { return; }
    if(rect.width() == -1 || rect.height() == -1) { cout << "Disabling Tracker" << endl; return; }
    if(currMatFrame.empty() && !isRoiInImageArea(rect)) { cout << "das" << "Roi is not valid" << endl;  return;}
    trackingRect = rect;
    cv::Rect rectSel(trackingRect.x(), trackingRect.y(), trackingRect.width(), trackingRect.height());

    roiSet = false;
    tracker->init(currMatFrame, rectSel);
    roiSet = true;
    //cout << "============================ " << "roiRect: " << trackingRect.x() << ", " << trackingRect.y() << " | " << trackingRect.width() << " | " << trackingRect.height() << endl;
}

void DaSiamRPNTracker::track(QImage frame, double dt, QVector2D )
{
    if(busy) { return; }

    busy = true;
    pTimer.start();
    frame.convertTo(QImage::QImage::Format_RGB888);
    currMatFrame = cv::Mat(frame.height(), frame.width(), CV_8UC3, (void*)frame.constBits(), frame.bytesPerLine());

    if(!trackerCreated || !enabled || currMatFrame.empty() || !initialized || !roiSet) { busy = false; return;}
    //    qDebug() << "t" << trackedRect.x << trackedRect.y << trackedRect.width << trackedRect.height ;
    tracker->update(currMatFrame, trackedRect);

    DasRPN_score = tracker->getTrackingScore() * 100;
    trackerSuccessful = (DasRPN_score > 80.0f);
    //    qDebug() << "DasiamScore" << DasRPN_score;

    if(trackerSuccessful)
    {
        float curr_mid_x = (float)trackedRect.x + ((float)trackedRect.width*0.5f);
        float curr_mid_y = (float)trackedRect.y + ((float)trackedRect.height*0.5f);
        float last_mid_x = (float)trackingRect.x() + ((float)trackingRect.width()*0.5f);
        float last_mid_y = (float)trackingRect.y() + ((float)trackingRect.height()*0.5f);

        float vel_x = (curr_mid_x - last_mid_x)/dt;
        float vel_y = (curr_mid_y - last_mid_y)/dt;

        float accel_x = (vel_x - objectState.vel.x())/dt;
        float accel_y = (vel_x - objectState.vel.y())/dt;

        objectState.accel.setX(accel_x);
        objectState.accel.setY(accel_y);

        //        objectState.vel.setX((objectState.vel.x()*lp_gain) + (vel_x*(1.0f-lp_gain)));
        //        objectState.vel.setY((objectState.vel.x()*lp_gain) + (vel_y*(1.0f-lp_gain)));

        objectState.vel.setX(vel_x);
        objectState.vel.setY(vel_y);

        int tx = constrain(trackedRect.x, 0, (currMatFrame.cols-trackedRect.width));
        int ty = constrain(trackedRect.y, 0, (currMatFrame.rows-trackedRect.height));
        trackingRect.setRect(tx, ty, trackedRect.width, trackedRect.height);

        objectState.pos.setX(trackingRect.center().x());
        objectState.pos.setY(trackingRect.center().y());
    }

    processingTime = (processingTime * 0.9f) + ((double)pTimer.elapsed()*0.1f);


    emit tracked(trackingRect, objectState, trackerSuccessful, TRACKER_TYPE::DaSiamRPN);
//    qDebug() << "t" << trackingRect.x() << trackingRect.y() << trackingRect.width() << trackingRect.height() ;
    busy = false;
    //qDebug() << "DaSiamRPN:" << processingTime << "ms";
}

void DaSiamRPNTracker::setParam(QStringList){}

bool DaSiamRPNTracker::isRoiInImageArea(QRect &rect)
{
    if(currMatFrame.empty()){ return false; }

    return (rect.x() >= 0) && (rect.x()+rect.width() <= currMatFrame.cols) &&
           (rect.y() >= 0) && (rect.y()+rect.height() <= currMatFrame.rows) &&
           (rect.width() >= 0) && (rect.height() >= 0);
}

