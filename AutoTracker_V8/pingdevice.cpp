#include "pingdevice.h"

PingDevice::PingDevice(QObject *parent) : QObject{parent}
{

}

void PingDevice::init(QString _ping_address)
{
    ping_address = _ping_address;
    processName = "ping";
    args << "-W2" << "-c2" << "-t100" << ping_address;

    p = new QProcess;
    p->setReadChannel(QProcess::StandardOutput);
    p->setProcessChannelMode(QProcess::MergedChannels);
    connect(p, &QProcess::stateChanged, this, &PingDevice::processStateChanged);
    connect(p, &QProcess::readyRead, this, &PingDevice::readOutput);

    pingTimer = new QTimer;
    connect(pingTimer, &QTimer::timeout, this, &PingDevice::pingDev);

    initialized = true;
    if(autoStart) { start(); }
}

void PingDevice::pingDev()
{
    if(!initialized) { return; }
    if(p->state() != QProcess::NotRunning) { return; }

    pingOutput.clear();
    p->start(processName, args);
}

void PingDevice::processStateChanged(QProcess::ProcessState newState)
{
    if(newState == QProcess::Starting) {  }
    else if(newState == QProcess::Running) {  }
    else if(newState == QProcess::NotRunning)
    {
        ping_success = pingOutput.contains("2 packets transmitted, 2 received, 0% packet loss,");
        emit pingState(ping_success);
    }
}

void PingDevice::start()
{
    if(!initialized) { return; }
    pingTimer->start(ping_interval);
}

void PingDevice::stop()
{
    if(!initialized) { return; }
    pingTimer->stop();
}

void PingDevice::readOutput()
{
    pingOutput.append(p->readAll());
}
