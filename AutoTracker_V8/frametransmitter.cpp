#include "frametransmitter.h"
#include <QBuffer>
#include <QTcpSocket>
#include <QDebug>

using namespace std;

FrameTransmitter::FrameTransmitter(QObject *parent) : QObject{parent}
{

}

FrameTransmitter::~FrameTransmitter()
{
    if (tcpServer) delete tcpServer;
    if (commTimer) delete commTimer;
    if (dataTimer) delete dataTimer;
    if (reconnectTimer) delete reconnectTimer;
}

void FrameTransmitter::init()
{
    qDebug() << "Frame Transmitter - INIT:" << QThread::currentThreadId();

    tcpServer = new QTcpServer(this);
    reconnectTimer = new QTimer(this);
    commTimer = new QTimer(this);
    dataTimer = new QTimer(this);

    connect(tcpServer, &QTcpServer::newConnection, this, &FrameTransmitter::incomingConnection);
    connect(commTimer, &QTimer::timeout, this, &FrameTransmitter::sendFrame);
    connect(dataTimer, &QTimer::timeout, this, &FrameTransmitter::sendPendingData);

    connect(reconnectTimer, &QTimer::timeout, this, &FrameTransmitter::checkConnection);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &FrameTransmitter::readincomingData);

    if (tcpServer->listen(QHostAddress::Any, IMAGE_PORT)) {
        qDebug() << "Data Server Listening on port:" << IMAGE_PORT;
    }
    else{
        qDebug() << "not listening";
    }

    commTimer->start(20);
    reconnectTimer->start(2000);
    dataTimer->start(20);

    initialized = true;
    //    serial_err_count = 0;
}

void FrameTransmitter::getFrame(const TrackerFrame &_trackerFrame)
{
    this->frame = _trackerFrame.lutFrame;
    this->dt = _trackerFrame.dt;
}

void FrameTransmitter::getTrackedRectInfo(QRectF rect)
{
    rx1 = rect.left();
    ry1 = rect.top();
    rx2 = rect.right();
    ry2 = rect.bottom();

    //    qDebug() << "rrr" << rx1 << ry1 << rx2 << ry2;
}

QByteArray FrameTransmitter::getTrackedRectInfoAsByteArray() const
{
    //    QByteArray byteArray;
    //    byteArray.append(QByteArray::number(rx1, 'f', 2)).append(",");
    //    byteArray.append(QByteArray::number(ry1, 'f', 2)).append(",");
    //    byteArray.append(QByteArray::number(rx2, 'f', 2)).append(",");
    //    byteArray.append(QByteArray::number(ry2, 'f', 2));
    ////      qDebug() << "byte" << byteArray;
    //    return byteArray;
}

void FrameTransmitter::getCameraType(int selectedCamera)
{
    cameraType = selectedCamera;
}

void FrameTransmitter::sendFrame()
{
    if (frame.isNull()) {
        qDebug() << "No frame to send";
        return;
    }

    toSendFrame = frame;

    if (toSendFrame.isNull()) return;

    bool send = false;

    //    if (tcpSocket != nullptr && tcpSocket->state() == QTcpSocket::ConnectedState && mgVEnabled == true)
    if (tcpSocket != nullptr && tcpSocket->state() == QTcpSocket::ConnectedState) {
        send = true;
    }

    if (!send) {return;}

    QByteArray imgArr;
    QBuffer buffer(&imgArr);
    buffer.open(QIODevice::WriteOnly);
    toSendFrame.save(&buffer, "JPG", 50);
    //    qDebug() << "tosenddd" << toSend;

    QByteArray frame_data = "~!##,";
    frame_data.append(QByteArray::number(imgArr.size()));
    frame_data.append(",");
    frame_data.append(QByteArray::number(toSendFrame.width()));
    frame_data.append(",");
    frame_data.append(QByteArray::number(toSendFrame.height()));
    frame_data.append(",");
    frame_data.append(QByteArray::number(curr_target_range, 'f', 2));  frame_data.append(",");
    frame_data.append(QByteArray::number(dt, 'f', 6));  frame_data.append(",");
    frame_data.append(QByteArray::number(cameraType));  frame_data.append(",");
    //    frame_data.append(toSend);  frame_data.append(",");
    //    frame_data.append(getTrackedRectInfoAsByteArray()); frame_data.append(",");
    frame_data.append("|#DAT#");
    frame_data.append(imgArr);
    frame_data.append("!DAT!:,##!~");

    //            qDebug() << "frame_data" << frame_data;

    //    if (tcpSocket != nullptr && tcpSocket->state() == QTcpSocket::ConnectedState && mgVEnabled == true) {
    if (tcpSocket != nullptr && tcpSocket->state() == QTcpSocket::ConnectedState) {
        tcpSocket->write(frame_data);
    }
}


void FrameTransmitter::addRawData(QByteArray data)
{
    if(!initialized) { return; }

    //    if(useNetwork)
    //    {
    //        if(tcpSocket == nullptr) return;
    //        if(tcpSocket->state() != QTcpSocket::ConnectedState) return;
    //    }

    //    else
    //    {
    //        if(serialPort == nullptr) { return; }
    //        else if(!serialPort->isOpen()) { return; }
    //    }

    commTimer->stop();
    QByteArray toSend = data;
    //    toSend = data;
    //    qDebug() << "tosend" << toSend;

    int h_index = getExistingRawDataIndex(toSend);
    h_index = -1;

    if(h_index >= 0){ pendingData[h_index] = toSend; }
    else { pendingData.append(toSend); }
    //    sendPendingData();

    //        if( (last_send_time.elapsed() >= commDuration)) { sendPendingData(); }

    if(!commTimer->isActive()) { commTimer->start(commDuration); }
}

void FrameTransmitter::sendPendingData()
{
    //        qDebug() << "send";
    if(pendingData.isEmpty()) return;

    QByteArray toSend;
    if( tcpSocket != nullptr)
    {
        if(tcpSocket->state() == QTcpSocket::ConnectedState && !pendingData.isEmpty())
        {
            toSend = pendingData.takeFirst();
//            qDebug() << "tosend" << toSend;
            tcpSocket->write(toSend);
        }
    }
    else {
        //        qDebug() << "null";
    }

    //    else if(serialPort != nullptr)
    //    {
    //        if(serialPort->isOpen() && !pendingData.isEmpty())
    //        {
    //            toSend = pendingData.takeFirst();
    //            serialPort->write(toSend);
    //        }
    //    }

    //    if(!toSend.isEmpty())
    //        qDebug() << "TO: " << toSend;

    last_send_time.start();
}

int FrameTransmitter::getExistingRawDataIndex(QByteArray data)
{
    int h_index = -1;

    for(int i = 0; i < pendingData.size(); i++)
    {
        //        qDebug() << "data" << data.mid(0,3);
        //        qDebug() << "penddata" << pendingData[i].mid(0, 3);

        if(data.mid(0, 3) == pendingData[i].mid(0, 3))
        {
            h_index = i;
            //             qDebug() <<"h_index" << h_index;
            break;
        }
    }

    //    qDebug() <<"ind" << h_index;
    return h_index;

}

void FrameTransmitter::checkConnection()
{
    if (!tcpServer->isListening()) {
        tcpServer->listen(QHostAddress::Any, IMAGE_PORT);
    }
}

void FrameTransmitter::incomingConnection()
{
    qDebug() << "Incoming connection";
    if (tcpSocket == nullptr || tcpSocket->state() == QTcpSocket::UnconnectedState) {
        qDebug() << "Connecting...";
        if (tcpSocket != nullptr) {
            tcpSocket->disconnect();
            tcpSocket->deleteLater();
        }
        tcpSocket = tcpServer->nextPendingConnection();

        connect(tcpSocket, &QTcpSocket::readyRead, this, &FrameTransmitter::readincomingData);

        if (tcpSocket->state() == QTcpSocket::ConnectedState) {
            qDebug() << "Socket connected";
        }
    } else {
        qDebug() << "Already connected, state:" << tcpSocket->state();
    }
}

void FrameTransmitter::readincomingData()
{
    if (tcpSocket->bytesAvailable()){
        QByteArray data = tcpSocket->readAll();

        if (data.size() == 1) {
            char receivedChar = data.at(0);
            //            qDebug() << "received " << receivedChar;
            if (receivedChar == '1') {
                mgVEnabled = true;
            } else if (receivedChar == '0') {
                mgVEnabled = false;
            } else {
                qDebug() << "Unknown data received";
            }
            //            else{
            //                mgVEnabled = false;
            //            }
        } else {
            //            qDebug() << "Unexpected data size";
        }
    }
}


