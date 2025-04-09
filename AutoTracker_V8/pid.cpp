#include "pid.h"

PID::PID()
{
    I = 0.0; kp=0.0; ki=0.0; kd=0.0;
    pError = 0.0; maxI = 0; maxErr = 0;
}

void PID::setParameters(double mKp, double mKi, double mKd, double mI, double mE)
{

    kp=mKp;  ki=mKi;  kd=mKd;
    maxI = mI; maxErr = mE;
    P=I=D=0;
}

double PID::compute(double error, double dt)
{
    double ret;

    if(dt < 0.000001) return(0);

    P = kp * error;
    D = (kd * (error-pError) / dt); pError = error;
    I = (I + (ki * error * dt));   if(I>maxI) I=maxI; if(I<-maxI) I=-maxI;

    ret = P + I + D;   if(ret>maxErr) ret=maxErr; if(ret<-maxErr) ret=-maxErr;

    return(ret);
}

void PID::resetI()
{
    I = 0.0f;
}
