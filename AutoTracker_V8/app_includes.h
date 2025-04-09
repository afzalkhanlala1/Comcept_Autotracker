#ifndef APP_INCLUDES_H
#define APP_INCLUDES_H

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <sstream>
#include <cmath>
#include <algorithm>

#include <QObject>
#include <QtQuick>
#include <QQuickItem>
#include <QQuickPaintedItem>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QPainter>
#include <QPainterPath>
#include <QBrush>
#include <QImage>
#include <QColor>
#include <QString>
#include <QByteArray>
#include <QByteArrayList>
#include <QVector>
#include <QPoint>
#include <QRandomGenerator>
#include <QFontDatabase>
#include <QTime>
#include <QElapsedTimer>
#include <QTimer>
#include <QRect>
#include <QRectF>
#include <QPoint>
#include <QSize>
#include <QVector2D>
#include <QVector3D>
#include <QLine>
#include <QLineF>
#include <QMatrix>
#include <QFont>

#include <QMediaPlayer>
#include <QMediaContent>
#include <QAudio>
#include <QAudioProbe>
#include <QBuffer>
#include <QMediaPlaylist>
#include <QAbstractVideoSurface>
#include <QVideoFrame>
#include <QVideoProbe>

#include <QCamera>
#include <QCameraExposure>
#include <QCameraControl>
#include <QCameraInfo>
#include <QCameraImageCapture>
#include <QVideoFrame>
#include <QCameraViewfinder>
#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include <QCameraViewfinderSettings>

#include <QSerialPort>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>

#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/dnn_superres.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>
#include <opencv2/bgsegm.hpp>

#ifdef USE_GPU
#include <nvml.h>
#include <opencv2/cudaoptflow.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/photo/cuda.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include <opencv2/cudabgsegm.hpp>
#endif

#include "config.h"

#include "PacketData.h"
#include "dataFields.h"
#include "autotracker_types.h"
#include "utilityFunctions.h"

#endif // APP_INCLUDES_H
