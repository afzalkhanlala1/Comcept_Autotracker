#ifndef IMUSTAB_H
#define IMUSTAB_H

#include "app_includes.h"

class IMUStab : public QObject
{
    Q_OBJECT
public:
    explicit IMUStab(QObject *parent = nullptr);

    float win_h = 64.0f;
    float win_v = 48.0f;
    QSizeF imageSize = QSizeF(640, 480);
    QRectF stab_win = QRectF(0, 0, 640, 480);
    QVector2D worldDisplacment;

    QVector3D win_offset, win_anchor;

    float da = 0.0f, dx = 0.0f, dy = 0.0f;
    float fov_h = 43.3781f;
    float fov_v = 32.5336f;
    float fov_d = sqrt((fov_h*fov_h) + (fov_v*fov_v));
    float h_deg_to_pixel = 640.0f/fov_h;
    float v_deg_to_pixel = 480.0f/fov_v;
    float d_deg_to_pixel = sqrt((h_deg_to_pixel*h_deg_to_pixel) + (v_deg_to_pixel*v_deg_to_pixel));

public slots:
    void init();
    void stabilize(QVector<float> imuData, double dt);
    void setMaxAmplitude(float _win_h, float _win_v);

signals:

private:


};

#endif // IMUSTAB_H
