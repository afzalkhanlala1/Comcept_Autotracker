#ifndef COPROCESSOR_H
#define COPROCESSOR_H

#include "app_includes.h"

class CoProcessor : public QObject
{
    Q_OBJECT
public:
    explicit CoProcessor(QObject *parent = nullptr);

    QVector<PacketData> pendingOutgoing;
    QVector<QByteArray> pendingData;

    QString serialPortName;
    QSerialPort *serial = nullptr;

    double trackingPIDxVal = 0;
    double trackingPIDyVal = 0;
    float joyXVal = 0;
    float joyYVal = 0;
    bool at_gate = false;
    bool trackerEnabled = false;

public slots:
    void init(QString _serialPortName);
    void connectSerial();
    void serialPortError(QSerialPort::SerialPortError error);
    void readSerialData();
    void parseData();
    QByteArray popPacket(QByteArray &data);
    void addPacket(PacketData packetData);
    void addData(QByteArray newData);
    void sendCommand(QString header, float val);
    void sendPendingPacket();
    void sendPendingData();
    void sendTrackerData();
    void setGuiJoystickData(float _joy_x, float _joy_y, bool _trackerEnabled);

signals:
    void incomingDataPacket(PacketData packetData);
    void connectionStateChanged(int socketState);

private:

    QTimer *reconnectTimer = nullptr;
    QTimer *commTimer = nullptr;
    bool sendindData = false;
    double dt = 0;
    double packetTime = 0;
    QByteArray serialData;

    void mapExtDataValues(QVector<float> &values);

};

#endif // COPROCESSOR_H
