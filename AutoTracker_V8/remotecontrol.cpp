#include "remotecontrol.h"

using namespace std;

RemoteControl::RemoteControl(QObject *parent) : QObject{parent}
{

}

void RemoteControl::init(QString _appPath)
{
    qDebug() << "RemoteControl - INIT: " << QThread::currentThreadId();
    appPath = _appPath;
    settingsPath = appPath + "Settings/2DGimbal-Params";

    imageServer = new QTcpServer;
    dataServer = new QTcpServer;
    imageSockets = QVector<QTcpSocket*>(RC_MAX_CONN, nullptr);
    dataSockets = QVector<QTcpSocket*>(RC_MAX_CONN, nullptr);
    statusTimer = new QTimer;
    commTimer = new QTimer;

    connect(imageServer, &QTcpServer::newConnection, this, &RemoteControl::newImageConnection);
    connect(dataServer, &QTcpServer::newConnection, this, &RemoteControl::newDataConnection);
    connect(statusTimer, &QTimer::timeout, this, &RemoteControl::checkConnectionStatus);
    connect(commTimer, &QTimer::timeout, this, &RemoteControl::sendPendingData);

    if(imageServer->listen(QHostAddress::Any, RC_IMAGE_PORT)) { qDebug() << "RC Image Server Listening: " << RC_IMAGE_PORT; }
    else { qDebug() << "RC Image Server Listen FAILE: " << RC_IMAGE_PORT; }

    if(dataServer->listen(QHostAddress::Any, RC_DATA_PORT)) { qDebug() << "RC Data Server Listening: " << RC_DATA_PORT; }
    else { qDebug() << "RC Data Server Listen FAILE: " << RC_DATA_PORT; }

    statusTimer->start(2500);
    commTimer->start(40);
}

void RemoteControl::checkConnectionStatus()
{
    int img_status = -1;
    int data_status = -1;

    for(int i = 0; i < imageSockets.size(); i++)
    {
        if(imageSockets[i] != nullptr){
            if(imageSockets[i]->state() == QTcpSocket::ConnectedState){
                img_status = 1;
            }
        }

        if(dataSockets[i] != nullptr){
            if(dataSockets[i]->state() == QTcpSocket::ConnectedState){
                data_status = 1;
            }
        }
    }

    emit rcStatus(img_status + data_status);
}

void RemoteControl::sendSettings(QTcpSocket *socket, QString dataFilePath)
{
    if(!QFile().exists(dataFilePath)) { qDebug() << "RC: Settings File not found"; return; }

    QFile file(dataFilePath);
    QByteArray data;
    if(file.open(QIODevice::ReadOnly))
    {
        data = file.readAll().split('\n').join("|");
        file.close();
    }

    if(!data.isEmpty())
    {
        data.prepend("#RCST");
        data.append("!");
        socket->write(data);
    }
}

void RemoteControl::newImageConnection()
{
    int i = getAvailableSocket(imageSockets);
    if(i == -1) { qDebug() << "RC: Image Socket Limit reached"; imageServer->nextPendingConnection(); }
    else
    {
        if(imageSockets[i] != nullptr) { imageSockets[i]->disconnect(); }
        imageSockets[i] = imageServer->nextPendingConnection();

        connect(imageSockets[i], &QTcpSocket::readyRead, this, &RemoteControl::readImageIncoming);
    }
}

void RemoteControl::newDataConnection()
{
    int i = getAvailableSocket(dataSockets);
    if(i == -1) { qDebug() << "RC: Data Socket Limit reached"; dataServer->nextPendingConnection(); }
    else
    {
        if(dataSockets[i] != nullptr) { dataSockets[i]->disconnect(); }
        dataSockets[i] = dataServer->nextPendingConnection();
        sendSettings(dataSockets[i], settingsPath);

        connect(dataSockets[i], &QTcpSocket::readyRead, this, &RemoteControl::readDataIncoming);

        qDebug() << "RC Data Socket Connection: " << i;
    }
}

void RemoteControl::readDataIncoming()
{
    QIODevice *iodev = (QIODevice*)QObject::sender();
    if(iodev == nullptr) return;

    incomingData.append(iodev->readAll());
    parseData();

    if(incomingData.size() > 8192){ incomingData.clear(); }
}

void RemoteControl::parseData()
{
    while(incomingData.size() > 4)
    {
        QByteArray packet = popPacket(incomingData);
        if(packet.size() < 4)
            break;

        PacketData packetData;
        packetData.header = packet.mid(0, 4);
        QByteArrayList packetValues = packet.mid(5).split(',');

        packetData.setModule();
        if(packetData.module == Module::GUI_REMOTE) { invokeUserCmd(packetValues); }

        else
        {
            for(int i = 0; i < packetValues.size(); i++)
                packetData.values.append(packetValues[i].toDouble());

            emit incomingDataPacket(packetData);
        }
    }
}

QByteArray RemoteControl::popPacket(QByteArray &data)
{
    QByteArray packet;
    int start = data.indexOf('#');
    int end = data.indexOf('!', start+1);

    if(start < 0 || end < 0 || end <= start+1)
    {
        // cout << "Packet - Incomplete: " << start << "  | " << end << endl;
        return packet;
    }

    packet = data.mid(start + 1, end - start - 1);
    //qDebug() << "Rec: " << packet;
    data.remove(0, end + 1);
    return packet;
}

void RemoteControl::addData(QByteArray data){
    pendingData.append(data);
}

void RemoteControl::addData(QByteArray header, float value)
{
    QByteArray data = "#";
    data += header;
    data += QByteArray::number(value);
    data += "!";
    addData(data);
}

void RemoteControl::addTeleData(QByteArray data){
    pendingData.append(data);
}

void RemoteControl::readImageIncoming()
{

}

void RemoteControl::sendFrame(TrackerFrame trackerFrame)
{
    toSendFrame = trackerFrame.stabImage;

    if(toSendFrame.isNull()) return;

    bool send = false;
    for(int i = 0; i < imageSockets.size(); i++)
    {
        if(imageSockets[i] == nullptr) { continue; }
        else if(imageSockets[i]->state() == QTcpSocket::ConnectedState) { send = true; break; }
    }

    if(!send) return;

    QByteArray imgArr;
    QBuffer buffer (&imgArr);
    buffer.open(QIODevice::WriteOnly);
    //toSendFrame.save(&buffer, "JPG", compressionQuality);
    toSendFrame.save(&buffer, "JPG", 50);


    QByteArray frame_data = "~!##,";
    frame_data.append(QByteArray::number(imgArr.size())); frame_data.append(",");
    frame_data.append(QByteArray::number(toSendFrame.width())); frame_data.append(",");
    frame_data.append(QByteArray::number(toSendFrame.height())); frame_data.append(",");
    frame_data.append(QByteArray::number(curr_target_range, 'f', 2)); frame_data.append(",");
    frame_data.append(QByteArray::number(trackerFrame.dt, 'f', 6)); frame_data.append(",");

    frame_data.append("|#DAT#");

    //cout << "Sending Image: " << frame_data.toStdString() << "...ABC123...!DAT!,#!";

    frame_data.append(imgArr);
    frame_data.append("!DAT!:,##!~");

    for(int i = 0; i < imageSockets.size(); i++)
    {
        if(imageSockets[i] == nullptr) { continue; }
        else if(imageSockets[i]->state() == QTcpSocket::ConnectedState) { imageSockets[i]->write(frame_data); }
    }
}

void RemoteControl::sendPendingData()
{
    if(pendingData.isEmpty()) { return; }

    bool send = false;
    for(int i = 0; i < dataSockets.size(); i++)
    {
        if(dataSockets[i] == nullptr) { continue; }
        else if(dataSockets[i]->state() == QTcpSocket::ConnectedState) { send = true; break; }
    }

    if(!send) return;
    QByteArray data = pendingData.takeFirst();

    for(int i = 0; i < dataSockets.size(); i++)
    {
        if(dataSockets[i] == nullptr) { continue; }
        else if(dataSockets[i]->state() == QTcpSocket::ConnectedState) { dataSockets[i]->write(data); }
    }
}

int RemoteControl::getAvailableSocket(QVector<QTcpSocket *> &sockets)
{
    for(int i = 0; i < sockets.size(); i++)
    {
        if(sockets[i] == nullptr) { return i; }
        else if(sockets[i]->state() == QTcpSocket::UnconnectedState) { return i; }
    }

    return -1;
}

void RemoteControl::invokeUserCmd(QByteArrayList &packetValues)
{
    //qDebug() << "invokeUserCmd" << packetValues;
    if(packetValues.isEmpty()) { return; }

    switch (packetValues[0].toInt()) {
    case SetGuiJoystick:
        if(packetValues.size() > 2) { emit setRemoteGuiJoy(packetValues[1].toFloat(), packetValues[2].toFloat()); }
        break;
    case SetTrackigTarget:
        //        if(packetValues.size() > 2) { emit setTrackTarget(packetValues[1].toInt(), packetValues[2].toInt(), 56, 28, false); }
        if(packetValues.size() > 2) { emit setTrackTarget(packetValues[1].toInt(), packetValues[2].toInt(), 66, 40, false); }
        break;
    case SetUserEnableTracking:
        if(packetValues.size() > 1) { emit setUserEnableTracking(packetValues[1].toInt()); }
        break;
    case SetActiveTarget:
        if(packetValues.size() > 1) { emit setActiveTarget(packetValues[1].toInt()); }
        break;
    case CancelTargetRect:
        if(packetValues.size() > 1) { emit cancelTargetRect(packetValues[1].toInt()); }
        break;
    case SetTracking:

        break;
    case Set:
        if(packetValues.size() > 2) { emit set(packetValues[1], packetValues[2].toFloat(), true, true, true); }
        break;
    case SetShowOri:
        if(packetValues.size() > 1) { emit setShowOri(packetValues[1].toInt()); }
        break;
    case SetApplyLut:
        if(packetValues.size() > 1) { emit setApplyLut(packetValues[1].toInt()); }
        break;
    case SetLutAdaption:
        if(packetValues.size() > 1) { emit setLutAdaption(packetValues[1].toDouble()); }
        break;
    case SetLutSmoothingWindow:
        if(packetValues.size() > 1) { emit setLutSmoothingWindow(packetValues[1].toDouble()); }
        break;
    case SetLutStrength:
        if(packetValues.size() > 1) { emit setLutStrength(packetValues[1].toDouble()); }
        break;
    case SetEffectStrength:
        if(packetValues.size() > 1) { emit setPreProcessingEffectStrength(packetValues[1].toDouble()); }
        break;
    case EnableCLAHE:
        if(packetValues.size() > 1) { emit enableCLAHE(packetValues[1].toInt()); }
        break;
    case SetLutClaheFactor:
        if(packetValues.size() > 1) { emit setLutClaheFactor(packetValues[1].toDouble()); }
        break;
    case SetClaheParams:
        if(packetValues.size() > 2) { emit setClaheParams(packetValues[1].toInt(), packetValues[1].toInt(), packetValues[2].toInt()); }
        break;
    case SetTrackToCenter:

        break;
    case SetOpticalFlowView:
        if(packetValues.size() > 1) { emit setOpticalFlowView(packetValues[1].toInt()); }
        break;
    case SetOpticalFlowWinSize:
        if(packetValues.size() > 1) { emit setOpticalFlowWinSize(packetValues[1].toInt()); }
        break;
    case SetOpticalFlowPreBlur:
        if(packetValues.size() > 1) { emit setOpticalFlowPreBlur(packetValues[1].toInt()); }
        break;
    case SetOpticalFlowDownsample:
        if(packetValues.size() > 1) { emit setOpticalFlowDownsample(packetValues[1].toInt()); }
        break;
    case SetOpticalFlowFilter:
        if(packetValues.size() > 1) { emit setOpticalFlowFilter(packetValues[1].toFloat()); }
        break;
    case SetOpticalFlowGridSize:
        if(packetValues.size() > 1) { emit setOpticalFlowGridSize(packetValues[1].toInt()); }
        break;
    case SetEnableFrameBlending:
        if(packetValues.size() > 1) { emit setEnableFrameBlending(packetValues[1].toInt()); }
        break;
    case SetNoiseH:
        if(packetValues.size() > 1) { emit setNoiseH(packetValues[1].toFloat()); }
        break;
    case SetNoiseSearchWindow:
        if(packetValues.size() > 1) { emit setNoiseSearchWindow(packetValues[1].toInt()); }
        break;
    case SetNoiseBlockSize:
        if(packetValues.size() > 1) { emit setNoiseBlockSize(packetValues[1].toInt()); }
        break;
    case SetPreNR:
        if(packetValues.size() > 1) { emit setPreNR(packetValues[1].toInt()); }
        break;
    case SetPostNR:
        if(packetValues.size() > 1) { emit setPostNR(packetValues[1].toInt()); }
        break;
    case SetEnableSharpen:
        if(packetValues.size() > 1) { emit setEnableSharpen(packetValues[1].toInt()); }
        break;
    case EnableIS:
        if(packetValues.size() > 1) { emit enableIS(packetValues[1].toInt()); }
        break;
    case SetISViewMode:

        break;
    case SetISWindowMargin:
        if(packetValues.size() > 2) { emit setISWindowMargin(packetValues[1].toFloat(), packetValues[2].toFloat()); }
        break;
    case SetVideoDevice:

        break;

    case SetConsole:
        if(packetValues.size() > 2) { emit setConsole(packetValues[1], packetValues[2].toFloat()); }
        break;
    default:
        break;
    }
}
