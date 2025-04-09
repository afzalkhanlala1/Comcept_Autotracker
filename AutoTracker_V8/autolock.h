#ifndef AUTOLOCK_H
#define AUTOLOCK_H

#include "app_includes.h"

class AutoLock : public QObject
{
    Q_OBJECT
public:
    explicit AutoLock(QObject *parent = nullptr);

public slots:
    void init();
    void lockTarget(QImage frame, QRectF roi);

signals:
    void possibleTarget(QRectF target, float confidence);

private:


};

#endif // AUTOLOCK_H
