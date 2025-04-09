#ifndef YOLOCLIENTTRACKER_H
#define YOLOCLIENTTRACKER_H

#include "app_includes.h"

class YoloClientTracker : public QObject
{
    Q_OBJECT
public:
    explicit YoloClientTracker(QObject *parent = nullptr);

    bool trackerCreated = false;
    bool initialized = false;
    bool roiSet = false;
    bool enabled = false;
    bool trackerSuccessful = false;
    bool busy = false;

    float compressionQuality = 70.0f;

    QElapsedTimer pTimer;
    double processingTime = 0;
    float lp_gain = 0.8f;

    QRectF trackingRect;
    ObjectState objectState;

    //    QVector <YoloResult> results;

    QByteArray sof, soi, eof;

    QTcpSocket *socket = nullptr;
    QByteArray data;
    QTimer *reconnectTimer = nullptr;

public slots:
    void init();
    void setROI(QRect rect);
    void track(QImage frame, double dt, QVector2D worldDisplacement);
    void setParam(QStringList params);
    void readIncomingData();
    void checkConnection();

signals:
    void tracked(QRectF trackedRect, ObjectState objectState, bool success, int tracker_type);
    //    void imagelabeled(QVector <YoloResult> results);
    void imagelabeled(const QVector<YoloResult>& results);


private:
    bool isRoiInImageArea(QRect &rect);
    cv::Mat currMatFrame;
    cv::Rect trackedRect;
    int frameCounter = 0;
};

#endif // YOLOCLIENTTRACKER_H
