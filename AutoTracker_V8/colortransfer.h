#ifndef COLORTRANSFER_H
#define COLORTRANSFER_H

#include "app_includes.h"

class ColorTransfer : public QObject
{
    Q_OBJECT
public:
    explicit ColorTransfer(QObject *parent = nullptr);

    QElapsedTimer eTimer;
    double avg_processing_time = 5;

    QImage refImage;
    QString refImagePath;
    QVector<QVector<int>> refImageChannelValues;
    QVector<QVector<int>> refImageChannelCFD;
    QVector<QVector<int>> mappedValues;

//    int channelCount = 3;

    int sWin = 55;
    bool initialized = false;
    bool busy = false;

    QElapsedTimer last_time;

public slots:
    void init();
    void calcLUT(QImage subImage);

signals:
    void rgblutReady(QVector<QVector<int>> lut);
    void lutReady(QVector<int> lut);

private:
    void createImageHistogram(QImage& img, QVector<int>& histArray, int channel);
    void createCFD(QVector<int>& histArray, QVector<int>& cfdArray);
    void createHistogramMapping(QVector<int>& subHist, QVector<int>& targetHist, QVector<int> &mappedHist);
    void smooth(QVector<int>& values);
};

#endif // COLORTRANSFER_H
