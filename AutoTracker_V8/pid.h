#ifndef PID_H
#define PID_H

#include <stdlib.h>
#include <math.h>

class PID
{
public:
    PID();
    void setParameters(double mKp, double mKi, double mKd, double mI, double mE);
    double compute(double error, double dt);
    void resetI();

    double kp;
    double ki;
    double kd;
    double pError;
    double P, I, D;
    double maxI, maxErr;
};

#endif // PID_H
