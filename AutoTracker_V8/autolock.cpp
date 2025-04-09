#include "autolock.h"

using namespace std;

AutoLock::AutoLock(QObject *parent) : QObject{parent}
{

}

void AutoLock::init()
{
    qDebug() << "Auto Lock - INIT" << endl;
}

void AutoLock::lockTarget(QImage frame, QRectF roi)
{

}
