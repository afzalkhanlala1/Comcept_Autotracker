#include "coprocessor.h"

using namespace std;

#define RECONNECT_DELAY 3000
#define PACKET_DELAY 20

CoProcessor::CoProcessor(QObject *parent) : QObject{parent}
{

}

void CoProcessor::init(QString _serialPortName)
{
    qDebug() <<"SerialClient - Init: " << QThread::currentThreadId() << endl;

    serialPortName = _serialPortName;
    serial = new QSerialPort(this);
    reconnectTimer = new QTimer(this);
    commTimer = new QTimer(this);
    connect(reconnectTimer, &QTimer::timeout, this, &CoProcessor::connectSerial);
    connect(commTimer, &QTimer::timeout, this, &CoProcessor::sendPendingPacket);
    connect(serial, &QSerialPort::errorOccurred, this, &CoProcessor::serialPortError);
    connect(serial, &QSerialPort::readyRead, this, &CoProcessor::readSerialData, Qt::DirectConnection);

    connectSerial();

    reconnectTimer->start(RECONNECT_DELAY);
    commTimer->start(PACKET_DELAY);
}

void CoProcessor::connectSerial()
{
    if(serial == nullptr) return;
    if(serial->isOpen() && serial->error() == QSerialPort::NoError) return;

    serial->clearError();
    serial->setPortName(serialPortName);
    serial->setBaudRate(460800);

    if(serial->open(QIODevice::ReadWrite))
        qDebug() <<"SERIAL PORT OPENED" << endl;
}

void CoProcessor::serialPortError(QSerialPort::SerialPortError error)
{
    if(error == QSerialPort::NoError) emit connectionStateChanged(3);
    else emit connectionStateChanged(0);
}

void CoProcessor::readSerialData()
{
    serialData.append(serial->readAll());
    //qDebug() <<"Serial-IN: " << serialData.toStdString() << endl;
    parseData();

    if(serialData.size() > 2048)
    {
        qDebug() <<"Serial Buffer Full - Emptying..." << endl;
        serialData.clear();
    }
}

void CoProcessor::parseData()
{
    while(serialData.size() > 4)
    {
        QByteArray packet = popPacket(serialData);
        if(packet.size() < 3)
            break;

        PacketData packetData;
        packetData.header = packet.mid(0, 4);
        packetData.setModule();

        if(packetData.module == Module::DEBUG)
        {
            QByteArray debugStr = packet.mid(4, packet.size()-1);
            qDebug() <<"DEBUG: " << debugStr << flush;
        }

        else
        {
            packetData.time = QTime::currentTime();
            QByteArrayList packetValues = packet.mid(4).split(',');

            for(int i = 0; i < packetValues.size(); i++)
                packetData.values.append(packetValues[i].toFloat());

            if(packetData.module == Module::EXT_DATA)
            {
                mapExtDataValues(packetData.values);
            }

            emit incomingDataPacket(packetData);
        }
    }
}

QByteArray CoProcessor::popPacket(QByteArray &data)
{
    QByteArray packet;
    int start = data.indexOf('#');
    int end = data.indexOf('!', start+1);

    if(start < 0 || end < 0 || end <= start+1)
    {
        // qDebug() <<"Packet - Incomplete: " << start << "  | " << end << endl;
        return packet;
    }
    packet = data.mid(start + 1, end - start - 1);
    //qDebug() <<"Rec: " << packet;
    data.remove(0, end + 1);
    return packet;
}


//=========================================================


void CoProcessor::addPacket(PacketData packetData){
    pendingOutgoing.append(packetData);
}

void CoProcessor::addData(QByteArray newData)
{
    if(serial == nullptr) return;
    if(!serial->isOpen()) return;

    //if(sendindData) return;

    int index = -1;
    for(int i = 0; i < pendingData.size(); i++)
    {
        if(pendingData[i].mid(0, 5) == newData.mid(0, 5))
        {
            index = i;
            break;
        }
    }

    if(index != -1) { pendingData[index] = newData; }
    else { pendingData.append(newData); }
}

void CoProcessor::sendCommand(QString header, float val)
{
   QByteArray data = "#";
   data.append(header);
   data.append(QByteArray::number(val, 'f', 3));
   data.append("!");
   addData(data);
}

void CoProcessor::sendPendingPacket()
{
    if(serial == nullptr) return;
    if(!serial->isOpen()) return;

    sendTrackerData();

    if(pendingOutgoing.isEmpty()) { sendPendingData(); return; }

    PacketData packet = pendingOutgoing.first();

    QByteArray data = "#";
    data.append(packet.header.toStdString().c_str());
    for(int i = 0; i < packet.values.size(); i++)
    {
        data += QByteArray::number(packet.values[i]);
        data += ",";
    }

    if(packet.values.isEmpty()) data += "!";
    else data[data.size() - 1] = '!';

    serial->write(data);
    //qDebug() <<pendingOutgoing.size() - 1 << " ." << "TO: " << data << endl;
    pendingOutgoing.removeFirst();
}

void CoProcessor::sendPendingData()
{
    if(pendingData.isEmpty() || serial == nullptr) return;
    if(!serial->isOpen()) return;

    sendindData = true;

    QByteArray data = pendingData.first();
    serial->write(data);
    pendingData.removeFirst();

    sendindData = false;
}

void CoProcessor::sendTrackerData()
{
    QByteArray data = "#DATA";
    data.append(QByteArray::number(trackingPIDxVal, 'f', 4)); data.append(",");
    data.append(QByteArray::number(trackingPIDyVal, 'f', 4)); data.append(",");
    data.append(QByteArray::number(at_gate)); data.append(",");
    data.append(QByteArray::number(joyXVal, 'f', 0)); data.append(",");
    data.append(QByteArray::number(joyYVal, 'f', 0)); data.append(",");
    data.append(QByteArray::number(trackerEnabled));
    data.append("!");

    //qDebug() << "CP:" << data;

    serial->write(data);
}

void CoProcessor::setGuiJoystickData(float _joy_x, float _joy_y, bool _trackerEnabled)
{
    joyXVal = _joy_x;
    joyYVal = _joy_y;
    trackerEnabled = _trackerEnabled;
}

void CoProcessor::mapExtDataValues(QVector<float> &values)
{
    if(values.size() != EXT_DATA::ed_count) { return; }

    values[EXT_DATA::AMMO_TYPE] = (int)(values[EXT_DATA::AMMO_TYPE] / 0.26f);
    values[EXT_DATA::HORI_LEAD] = -(values[EXT_DATA::HORI_LEAD] - 0.5f)*20.0f;
    values[EXT_DATA::VERT_LEAD] = (values[EXT_DATA::VERT_LEAD] - 0.5f)*20.0f;
    values[EXT_DATA::DISTANCE] = 500.0f + (values[EXT_DATA::DISTANCE]*5000.0f);
}
