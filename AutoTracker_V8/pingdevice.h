#ifndef PINGDEVICE_H
#define PINGDEVICE_H

#include "app_includes.h"

class PingDevice : public QObject
{
    Q_OBJECT
public:
    explicit PingDevice(QObject *parent = nullptr);

    QString ping_address;
    QProcess *p = nullptr;
    QString processName;
    QStringList args;
    QByteArray pingOutput;

    QTimer *pingTimer = nullptr;
    int ping_interval = 2500;

    bool autoStart = true;
    bool initialized = false;
    bool ping_success = false;

public slots:
    void init(QString _ping_address);
    void pingDev();
    void processStateChanged(QProcess::ProcessState newState);

    void start();
    void stop();
    void readOutput();

signals:
    void pingState(int state);

private:


};

#endif // PINGDEVICE_H
