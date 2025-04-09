#ifndef VIDEOFRAME_CONVERTOR_H
#define VIDEOFRAME_CONVERTOR_H

#include <QtMultimedia/qvideoframe.h>

QT_BEGIN_NAMESPACE

Q_MULTIMEDIA_EXPORT QImage qt_imageFromVideoFrame(const QVideoFrame &frame);

QT_END_NAMESPACE
#endif // VIDEOFRAME_CONVERTOR_H
