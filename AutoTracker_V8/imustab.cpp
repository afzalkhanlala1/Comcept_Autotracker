#include "imustab.h"

using namespace std;

// distance = 264mm | fov H = 211mm
//                  | AOV H = 43.3781 V = 32.5336

#define ang180(tmp) ((tmp>=180) ? (tmp-=360) : (tmp<-180) ? (tmp+=360) : tmp )

IMUStab::IMUStab(QObject *parent) : QObject{parent}
{

}

void IMUStab::init()
{
    setMaxAmplitude(win_h, win_v);
}

void IMUStab::stabilize(QVector<float> imuData, double dt)
{
    dx = constrain((float)(imuData[AVG_GZ] * dt * h_deg_to_pixel), -win_h*2.0f, win_h*2.0f);
    dy = constrain((float)(imuData[AVG_GY] * dt * v_deg_to_pixel), -win_v*2.0f, win_v*2.0f);
    da = constrain((float)(imuData[AVG_GX] * dt), -25.0f, 25.0f);

//    dx = imuData[AVG_GZ] * dt * h_deg_to_pixel;
//    dy = imuData[AVG_GY] * dt * v_deg_to_pixel;
//    da = imuData[AVG_GX] * dt;

    win_offset += QVector3D(dx, -dy, da);
    win_offset = (win_offset*0.95f) + (win_anchor*0.05f);
    stab_win.moveTo(win_offset.toVector2D().toPointF());
}

void IMUStab::setMaxAmplitude(float _win_h, float _win_v)
{
    win_h = _win_h;
    win_v = _win_v;
    float w = imageSize.width() - (2*win_h);
    float h = imageSize.height() - (2*win_v);

    win_offset = win_anchor = QVector3D((imageSize.width() - w)*0.5f, (imageSize.height() - h)*0.5f, 0.0f);
    stab_win = QRectF(win_anchor.x(), win_anchor.y(), w, h);
}
