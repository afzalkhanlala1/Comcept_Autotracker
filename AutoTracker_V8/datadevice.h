#ifndef DATADEVICE_H
#define DATADEVICE_H

#include "app_includes.h"

enum CONNECTION_STATUS { DEV_UNCONNECTED = 0, DEV_LOOKING_UP, DEV_CONNECTED };

class DataDevice : public QObject
{
    Q_OBJECT
public:
    explicit DataDevice(QObject *parent = nullptr);
    ~DataDevice();
    QTcpServer *tcpServer = nullptr;
    QTcpSocket *tcpSocket = nullptr;
    QSerialPort *serialPort = nullptr;

    QTimer *reconnectTimer = nullptr;
    QTimer *commTimer = nullptr;

    QVector<QByteArray> pendingData;
    QByteArray incomingData;

    QString serialPortName;
    QString ipAddress;
    quint16 tcpPort;
    int baudrate = 460800;
    float maxSerialSpeed = baudrate / 10.0f;
    bool clientMode = false;
    bool useNetwork = false;

    int commDuration = 20;

    QElapsedTimer dataTimer;
    QElapsedTimer packetTimer;
    float data_dt = 40.0f;
    float avgDataSpeed = 1.0f;
    float avgDataSize = 0.0f;

    CONNECTION_STATUS connStatus = CONNECTION_STATUS::DEV_UNCONNECTED;

    QTimer *heartBeatTimer = nullptr;
    bool enableHeartBeat = false;
    QString heartBeatMessage = "HRBT";
    int heartBeatDelay = 400;

    bool forceUnlockDevice = false;
    bool initialized = false;

    QElapsedTimer last_send_time;
    QByteArray curr_packet;

    qint64 os_socketBufferSize = 50*1000000;  //50 MB
    int serial_err_count = 0;

public slots:
    void init(QString _serialPortName, int _baudrate = 115200, QString _ip = "", quint16 _tcpPort = 0, bool _clientMode = false);
//    void sendFrame(TrackerFrame trackerFrame);
    void reInitSerial(QString _serialPortName, int _baudrate);
    void checkConnection();
    void incomingConnection();
    void readIncomingData();
    void beginSerialPort();
    void addData(QByteArray header, QByteArray data);
    void addRawData(QByteArray data);
    void sendPendingData();
    void sendCommand(QString header, float val);
    void socketStateChanged(QTcpSocket::SocketState socketState);
    void serialErrorOccurred(QSerialPort::SerialPortError error);

    void doHeartBeat();
    void closeConnection();

signals:
    void incomingDataPacket(PacketData packetData, QByteArray packet);
    void connectionStatus(int connStatus);

private:
    int getExistingPacketIndex(QString header);
    int getExistingRawDataIndex(QByteArray data);
    void parseData();
    QByteArray popPacket(QByteArray &data);

    bool searchedForNL = false;
};

#endif // DATADEVICE_H
