#ifndef FRAMETRANSMITTER_H
#define FRAMETRANSMITTER_H

#include <QObject>
#include <QQuickItem>

#include "app_includes.h"

class FrameTransmitter : public QObject
{
    Q_OBJECT

public:
    explicit FrameTransmitter(QObject *parent = nullptr);
    ~FrameTransmitter();

    QTcpServer *tcpServer = nullptr;
    QTcpSocket *tcpSocket = nullptr;
    QImage frame;
    QImage toSendFrame;
    //    QVector<QTcpSocket*> imageSockets;
    //      QTcpSocket *socket = nullptr;
    float curr_target_range = 2000.0f;
    double dt;
    int cameraType;

    QTimer *reconnectTimer = nullptr;
    QTimer *commTimer = nullptr;
    QTimer *dataTimer = nullptr;
    bool mgVEnabled = false;

    float rx1;
    float ry1;
    float rx2;
    float ry2;

    bool initialized = false;
    bool useNetwork = false;
    QSerialPort *serialPort = nullptr;
    QVector<QByteArray> pendingData;
    QByteArray toSend;

    QElapsedTimer last_send_time;
    int commDuration = 20;

public slots:

    void init();
    void getFrame(const TrackerFrame &_trackerFrame);
    void getTrackedRectInfo(QRectF rect);
    QByteArray getTrackedRectInfoAsByteArray() const;
//    int getExistingPacketIndex(QString header);
    int getExistingRawDataIndex(QByteArray data);

    void getCameraType(int selectedCamera);
    void sendFrame();
    void addRawData(QByteArray data);
    void sendPendingData();
    void checkConnection();
    void incomingConnection();
    void readincomingData();

signals:



};

#endif // FRAMETRANSMITTER_H
