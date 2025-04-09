#ifndef PACKETDATA_H
#define PACKETDATA_H

#include <stdlib.h>
#include <QString>
#include <QMetaType>
#include <QVector>
#include <QTime>

enum Module {TELEMETERY = 0, FCS_DATA, IMU_DATA, GUNNER_CONSOLE, GUNNER_MONITOR, USER_PANEL, EXT_DATA, RF_REMOTE, GUI_REMOTE, JUNK, DEBUG, count};
enum SensorType {CAMPOS_XZ = 0, CAMPOS_YZ, INVALID, _count};

struct PacketData
{
    QString header;
    Module module = Module::JUNK;
    SensorType sensorType = SensorType::INVALID;
    QVector<float>values;
    QTime time;

    inline void setModule()
    {
        if(this->header.contains("IMUD",Qt::CaseInsensitive)) this->module = Module::IMU_DATA;
        else if(this->header.contains("FCSD", Qt::CaseInsensitive)) this->module = Module::FCS_DATA;
        else if(this->header.contains("GUCO", Qt::CaseInsensitive)) this->module = Module::GUNNER_CONSOLE;
        else if(this->header.contains("GUMT", Qt::CaseInsensitive)) this->module = Module::GUNNER_MONITOR;
        else if(this->header.contains("_TEL",Qt::CaseInsensitive)) this->module = Module::TELEMETERY;
        else if(this->header.contains("USPD",Qt::CaseInsensitive)) this->module = Module::USER_PANEL;
        else if(this->header.contains("RFRM",Qt::CaseInsensitive)) this->module = Module::RF_REMOTE;
        else if(this->header.contains("EXTD",Qt::CaseInsensitive)) this->module = Module::EXT_DATA;
        else if(this->header.contains("GURM",Qt::CaseInsensitive)) this->module = Module::GUI_REMOTE;
        else if(this->header.contains("$DB:",Qt::CaseInsensitive)) this->module = Module::DEBUG;
        else this->module = Module::JUNK;
    }

    inline void setSensorType()
    {
        if(this->header.contains("CXZD",Qt::CaseInsensitive)) this->sensorType = SensorType::CAMPOS_XZ;
        else if(this->header.contains("CYZD",Qt::CaseInsensitive)) this->sensorType = SensorType::CAMPOS_YZ;
        else this->sensorType = SensorType::INVALID;
    }

    inline bool isValid(int max_ms = 400)
    {
        return (time.msecsTo(QTime::currentTime()) < max_ms);
    }
};

Q_DECLARE_METATYPE(PacketData)


#endif // PACKETDATA_H
