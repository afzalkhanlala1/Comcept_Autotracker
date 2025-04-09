#include "datadevice.h"

using namespace std;

DataDevice::DataDevice(QObject *parent) : QObject{parent}
{

}

DataDevice::~DataDevice()
{
    closeConnection();
}

void DataDevice::init(QString _serialPortName, int _baudrate, QString _ip, quint16 _tcpPort, bool _clientMode)
{
    qDebug() << "Data Device - INIT |" << QThread::currentThreadId() << _serialPortName << "|" << _baudrate << "|" << _ip << "|" << _tcpPort << "|" << _clientMode << endl;
    initialized = false;
    //system("nmcli connection up Lenovo-Legion-5");

    serialPortName = _serialPortName;
    baudrate = _baudrate;

    ipAddress = _ip;
    tcpPort = _tcpPort;
    clientMode = _clientMode;

    tcpServer = new QTcpServer;
    serialPort = new QSerialPort;
    reconnectTimer = new QTimer;
    commTimer = new QTimer;
    heartBeatTimer = new QTimer;

    connect(tcpServer, &QTcpServer::newConnection, this, &DataDevice::incomingConnection);
    connect(serialPort, &QSerialPort::readyRead, this, &DataDevice::readIncomingData);
    connect(serialPort, &QSerialPort::errorOccurred, this, &DataDevice::serialErrorOccurred);
    //    connect(commTimer, &QTimer::timeout, this, &DataDevice::sendFrame);
    connect(commTimer, &QTimer::timeout, this, &DataDevice::sendPendingData);
    connect(reconnectTimer, &QTimer::timeout, this, &DataDevice::checkConnection);
    connect(heartBeatTimer, &QTimer::timeout, this, &DataDevice::doHeartBeat);

    if(!serialPortName.isEmpty())
    {
        if(forceUnlockDevice)
        {
            QString serial_lock_file = "/run/lock/LCK." + serialPortName.split('/').last();
            if(QFile().exists(serial_lock_file))
            {
                QString cmd = "sudo rm " + serial_lock_file;
                system(cmd.toStdString().c_str());
                //QFile().remove(serial_lock_file);
                qDebug() << "WARNING - Serial lock file found and removed:" << serial_lock_file;
            }
        }

        useNetwork = false;
        beginSerialPort();
    }

    //    else if(tcpPort > 0)
    //    {
    //        useNetwork = true;
    //        if(clientMode)
    //        {
    //            tcpSocket = new QTcpSocket;
    //            connect(tcpSocket, &QTcpSocket::readyRead, this, &DataDevice::readIncomingData);
    //            connect(tcpSocket, &QTcpSocket::stateChanged, this, &DataDevice::socketStateChanged);
    //            tcpSocket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, os_socketBufferSize);
    //            tcpSocket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, os_socketBufferSize);
    //            tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    //            tcpSocket->connectToHost(ipAddress, tcpPort);
    //        }

    //        else if(tcpServer->listen(QHostAddress::Any, tcpPort))
    //        {
    //            qDebug() << "Data Server Listening: " << tcpPort;
    //        }
    //    }

    initialized = true;
    serial_err_count = 0;

    commTimer->start(commDuration);
    reconnectTimer->start(2000);
    if(enableHeartBeat) { heartBeatTimer->start(heartBeatDelay); }

    last_send_time.start();
}


void DataDevice::reInitSerial(QString _serialPortName, int _baudrate)
{
    if(serialPort == nullptr) { return; }

    reconnectTimer->stop();
    commTimer->stop();
    pendingData.clear();

    if(serialPort->isOpen())
    {
        serialPort->flush();
        serialPort->clear();
        serialPort->close();
    }

    serialPortName = _serialPortName;
    baudrate = _baudrate;

    beginSerialPort();

    commTimer->start(commDuration);
    reconnectTimer->start(1000);
}

void DataDevice::checkConnection()
{
    if(!useNetwork) { beginSerialPort(); }

    else if(clientMode)
    {
        if(tcpSocket != nullptr)
        {
            if(tcpSocket->state() == QTcpSocket::SocketState::UnconnectedState)
            {
                tcpSocket->connectToHost(ipAddress, tcpPort);
            }
        }
    }

    else if(!tcpServer->isListening()) { tcpServer->listen(QHostAddress::Any, tcpPort); }
}

void DataDevice::beginSerialPort()
{
    if(serialPort == nullptr) return;
    if(serialPort->isOpen())
    {
        if(enableHeartBeat && !heartBeatTimer->isActive()) { heartBeatTimer->start(heartBeatDelay); }
        serial_err_count = 0;
        return;
    }

    serialPort->setPortName(serialPortName);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);
    serialPort->setBaudRate(baudrate);
    serialPort->clearError();

    if(serialPort->open(QIODevice::ReadWrite))
    {
        //qDebug() << "SERIAL PORT OPENED" << endl;
        packetTimer.start();
    }

    else if(serial_err_count < 2) { qDebug() << "Serial port open - FAILED:" << serialPortName; serial_err_count++; }
}

void DataDevice::socketStateChanged(QTcpSocket::SocketState socketState)
{
    if(socketState == QTcpSocket::ConnectedState) { connStatus = CONNECTION_STATUS::DEV_CONNECTED; }
    else if(socketState == QTcpSocket::HostLookupState) { connStatus = CONNECTION_STATUS::DEV_LOOKING_UP; }
    else { connStatus = CONNECTION_STATUS::DEV_UNCONNECTED; }

    emit connectionStatus(connStatus);

    //qDebug() << "Socket-State:" << socketState;
}

void DataDevice::serialErrorOccurred(QSerialPort::SerialPortError error)
{
    if(error == QSerialPort::NoError && serialPort->isOpen()) { connStatus = CONNECTION_STATUS::DEV_CONNECTED; }
    else { connStatus = CONNECTION_STATUS::DEV_UNCONNECTED; }

    emit connectionStatus(connStatus);

    //qDebug() << "Serial-State:" << error;
    serialPort->clearError();
}

void DataDevice::doHeartBeat()
{
    if(!enableHeartBeat) { heartBeatTimer->stop(); }
    addData(heartBeatMessage.toStdString().c_str(), "0");
}

void DataDevice::closeConnection()
{
    if(useNetwork)
    {
        if(tcpSocket != nullptr)
        {
            if(tcpSocket->state() == QTcpSocket::ConnectedState) { tcpSocket->disconnectFromHost(); }
        }
    }

    else if(serialPort != nullptr)
    {
        if(!serialPort->isOpen())
        {
            serialPort->flush();
            serialPort->clear();
            serialPort->close();
        }
    }
}

void DataDevice::addData(QByteArray header, QByteArray data)
{
    if(!initialized) { return; }

    if(useNetwork)
    {
        if(tcpSocket == nullptr) return;
        if(tcpSocket->state() != QTcpSocket::ConnectedState) return;
    }

    else if(serialPort == nullptr) { return; }
    else if(!serialPort->isOpen()) { return; }

    commTimer->stop();
    QByteArray toSend = "#" + header + data + "!";

    int h_index = getExistingPacketIndex(header);

    if(h_index >= 0){ pendingData[h_index] = toSend; }
    else { pendingData.append(toSend); }

    if(pendingData.size() == 1 && (last_send_time.elapsed() >= commDuration)) { sendPendingData(); }

    commTimer->start(commDuration);
}

void DataDevice::addRawData(QByteArray data)
{
    if(!initialized) { return; }

    if(useNetwork)
    {
        if(tcpSocket == nullptr) return;
        if(tcpSocket->state() != QTcpSocket::ConnectedState) return;
    }

    else
    {
        if(serialPort == nullptr) { return; }
        else if(!serialPort->isOpen()) { return; }
    }

    commTimer->stop();
    QByteArray toSend = data;

    int h_index = getExistingRawDataIndex(toSend);
    h_index = -1;

    if(h_index >= 0){ pendingData[h_index] = toSend; }
    else { pendingData.append(toSend); }

    if(pendingData.size() == 1 && (last_send_time.elapsed() >= commDuration)) { sendPendingData(); }

    if(!commTimer->isActive()) { commTimer->start(commDuration); }
}

void DataDevice::sendPendingData()
{
    qDebug() << "send";
    if(pendingData.isEmpty()) return;

    QByteArray toSend;
    if(useNetwork && tcpSocket != nullptr)
    {
        if(tcpSocket->state() == QTcpSocket::ConnectedState && !pendingData.isEmpty())
        {
            toSend = pendingData.takeFirst();
            tcpSocket->write(toSend);
        }
    }

    else if(serialPort != nullptr)
    {
        if(serialPort->isOpen() && !pendingData.isEmpty())
        {
            toSend = pendingData.takeFirst();
            serialPort->write(toSend);
        }
    }

    //    if(!toSend.isEmpty())
    //        qDebug() << "TO: " << toSend;

    last_send_time.start();
}

void DataDevice::sendCommand(QString header, float val)
{
    addData(header.toStdString().c_str(), QByteArray::number(val));
}

void DataDevice::incomingConnection()
{
    bool accept = false;

    if(tcpSocket == nullptr) { accept = true; }
    else if(tcpSocket->state() == QTcpSocket::UnconnectedState) { tcpSocket->disconnect(); accept = true; }

    if(accept)
    {
        tcpSocket = tcpServer->nextPendingConnection();
        connect(tcpSocket, &QTcpSocket::readyRead, this, &DataDevice::readIncomingData);
        connect(tcpSocket, &QTcpSocket::stateChanged, this, &DataDevice::socketStateChanged);
        tcpSocket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, os_socketBufferSize);
        tcpSocket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, os_socketBufferSize);
        tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
        qDebug() << "socket connected" << endl;
    }
}

int DataDevice::getExistingPacketIndex(QString header)
{
    int h_index = -1;
    header.prepend("#");

    for(int i = 0; i < pendingData.size(); i++)
    {
        if(header == pendingData[i].mid(0, 5))
        {
            h_index = i;
            break;
        }
    }

    return h_index;
}

int DataDevice::getExistingRawDataIndex(QByteArray data)
{
    int h_index = -1;

    for(int i = 0; i < pendingData.size(); i++)
    {
        if(data.mid(0, 3) == pendingData[i].mid(0, 3))
        {
            h_index = i;
            break;
        }
    }

    return h_index;
}

void DataDevice::readIncomingData()
{
    QIODevice *iodev = (QIODevice*)QObject::sender();
    if(iodev == nullptr) return;

    double dataSize = (double)(iodev->bytesAvailable());
    avgDataSpeed = avgDataSpeed*0.995f + ((dataSize / (dataTimer.nsecsElapsed()/1000000000.0f))*0.005f);
    dataTimer.start();

    incomingData.append(iodev->readAll());
    parseData();

    if(incomingData.size() > 512) { incomingData.clear(); }

    //qDebug() << avgDataSpeed/1000.0f << "KB/s of" << maxSerialSpeed << " = " << ((avgDataSpeed/1000.0f) / maxSerialSpeed) * 100.0f;
}

void DataDevice::parseData()
{
    bool isValidPacket = false;
    while(incomingData.size() > 4)
    {
        isValidPacket = false;
        QByteArray packet = popPacket(incomingData);
        if(packet.size() < 4) { break; }

        avgDataSize = (avgDataSize*0.99f) + ((float)packet.size()*0.01f);
        PacketData packetData;
        if(searchedForNL)
        {
            QByteArrayList packetValues = packet.split(' ');
            if(packetValues.size() > 1)
            {
                packetData.header = "CTLD";
                //                packetData.tag = packetValues.takeFirst();

                for(int i = 0; i < packetValues.size(); i++){
                    packetData.values.append(packetValues[i].toDouble());
                }

                isValidPacket = true;
            }
        }

        else
        {
            packetData.header = packet.mid(0, 4);
            QByteArrayList packetValues = packet.mid(4).split(',');

            for(int i = 0; i < packetValues.size(); i++){
                packetData.values.append(packetValues[i].toDouble());
            }
            isValidPacket = true;
        }

        if(isValidPacket)
        {
            data_dt = data_dt*0.95f + ((packetTimer.nsecsElapsed() / 1000000.0f)*0.05f);
            packetTimer.start();

            packetData.setModule();
            emit incomingDataPacket(packetData, packet);
        }
    }
}

QByteArray DataDevice::popPacket(QByteArray &data)
{
    curr_packet.clear();
    searchedForNL = false;
    int start = data.indexOf('#');
    int end = data.indexOf('!', start+1);

    if(start < 0) { start = data.indexOf('$'); }

    if(end < 0)
    {
        end = data.indexOf('\n', start+1);
        searchedForNL = true;
    }

    if(start < 0 || end < 0 || end <= start+1)
    {
        // qDebug() << "Packet - Incomplete: " << start << "  | " << end << endl;
        return curr_packet;
    }

    curr_packet = data.mid(start + 1, end - start - 1);
    //qDebug() << "Rec: " << packet;
    data.remove(0, end + 1);
    return curr_packet;
}
