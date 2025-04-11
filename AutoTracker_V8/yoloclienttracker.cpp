#include "yoloclienttracker.h"

using namespace std;

#define PN_01       1128889
#define PN_02       2269733
#define PN_03       3042161
#define PN_04       4535189
#define PN_SZ       7

YoloClientTracker::YoloClientTracker(QObject *parent): QObject{parent}
{

}

//yolov8_track.py in YOLO_V8(VScode server side)
//deepsort_socketf.py in YOLO_V8

void YoloClientTracker::init()
{
//    system("cd /home/comswd/VisualStudio_code/YOLO_V8/venv && source ~/venv/bin/activate && python3 /home/comswd/VisualStudio_code/YOLO_V8/yolov8_track.py &");
//    system("source ~/home/comswd/VisualStudio_code/YOLO_V8/venv/bin/activate && python3 /home/comswd/VisualStudio_code/YOLO_V8/yolov8_track.py &");

    socket = new QTcpSocket;
    socket->connectToHost(QHostAddress::LocalHost, 30555);
    connect(socket, &QTcpSocket::readyRead, this, &YoloClientTracker::readIncomingData);

    sof = "~!SOF" + QByteArray::number(PN_01) + ",";
    soi = "|~IMS" + QByteArray::number(PN_02);
    eof = "#EMI!" + QByteArray::number(PN_03);

    reconnectTimer = new QTimer;
    connect(reconnectTimer, &QTimer::timeout, this, &YoloClientTracker::checkConnection);
    reconnectTimer->start(3000);

    trackerCreated = initialized = true;
//    initialized = true;
}

void YoloClientTracker::setROI(QRect rect)
{
    roiSet = false;
    if(!trackerCreated) { return; }
    //    if(!isRoiInImageArea(rect)) { cout << "yolo" << "Roi is not valid - Disabling Tracker" << endl; return; }

    //    if(rect.width() == -1 || rect.height() == -1) { cout << "Disabling yolo Tracker" << endl; return; }
    //    if(currMatFrame.empty() && !isRoiInImageArea(rect)) { cout << "yolo" << "Roi is not valid" << endl;  return;}

    trackingRect = rect;
    cv::Rect rectSel(trackingRect.x(), trackingRect.y(), trackingRect.width(), trackingRect.height());

    roiSet = true;
}

void YoloClientTracker::track(TrackerFrame trackerFrame, double dt, QVector2D worldDisplacement)
{
    QImage originalFrame = trackerFrame.frame;
    if (originalFrame.isNull()) {
         qDebug() << "YOLO Tracker received null original frame.";
         return;
    }

    // Increment the frame counter
    frameCounter++;

    // Send only every third frame
    if (frameCounter % 3 != 0) {
        return;
    }

    // Optional: Reset counter to prevent overflow
    if (frameCounter >= 30000) {
        frameCounter = 0;
    }

    // Prepare and send the frame
    pTimer.start();
    originalFrame.convertTo(QImage::Format_RGB888);

    QByteArray imgArr;
    QBuffer buffer(&imgArr);
    buffer.open(QIODevice::WriteOnly);
    originalFrame.save(&buffer, "JPG", compressionQuality);

    QByteArray frame_data = sof;

    frame_data.append(QByteArray::number(imgArr.size())); frame_data.append(",");
    frame_data.append(QByteArray::number(originalFrame.width())); frame_data.append(",");
    frame_data.append(QByteArray::number(originalFrame.height())); frame_data.append(",");

    // Append ROI coordinates
    if (roiSet) {
        frame_data.append(QByteArray::number(trackingRect.x())); frame_data.append(",");
        frame_data.append(QByteArray::number(trackingRect.y())); frame_data.append(",");
        frame_data.append(QByteArray::number(trackingRect.width())); frame_data.append(",");
        frame_data.append(QByteArray::number(trackingRect.height())); frame_data.append(",");
    } else {
        frame_data.append("-1,-1,-1,-1,"); // Indicate no ROI
    }

    frame_data.append(soi);
    frame_data.append(imgArr);
    frame_data.append(eof);

    if (socket != nullptr && socket->state() == QTcpSocket::ConnectedState) {
        socket->write(frame_data);
    }

    processingTime = (processingTime * 0.9f) + ((double)pTimer.elapsed() * 0.1f);
}

void YoloClientTracker::setParam(QStringList params)
{

}

void YoloClientTracker::readIncomingData()
{
    data += socket->readAll();

    QVector<YoloResult> results;

    QByteArrayList frame_labels = data.split('\n');
    for (int i = 0; i < frame_labels.size(); i++) {
        QByteArrayList label_output = frame_labels[i].split(',');
        if (label_output.size() >= 7) { // Ensure we have all expected fields
            YoloResult r1;
            r1.label = label_output[0].toInt();
            r1.confidence = label_output[1].toFloat();
            float x = label_output[2].toFloat();
            float y = label_output[3].toFloat();
            float x2 = label_output[4].toFloat();
            float y2 = label_output[5].toFloat();
            r1.labelID = label_output[6].toInt();
            r1.bbox = QRectF(QPointF(x, y), QPointF(x2, y2));
            results.append(r1);
        }
    }
    qDebug() << "Received" << results.size() << "bounding boxes in clienttracker";
    emit imagelabeled(results);
    data.clear();

}

void YoloClientTracker::checkConnection()
{
    if (socket == nullptr) {return;}
    if (socket->state() == QTcpSocket::UnconnectedState) { socket->connectToHost(QHostAddress::LocalHost, 30555); }
}

bool YoloClientTracker::isRoiInImageArea(QRect &rect)
{
    if(currMatFrame.empty()){ return false; }
    return (rect.x() >= 0) && (rect.x()+rect.width() <= currMatFrame.cols) &&
           (rect.y() >= 0) && (rect.y()+rect.height() <= currMatFrame.rows) &&
           (rect.width() >= 0) && (rect.height() >= 0);
}
