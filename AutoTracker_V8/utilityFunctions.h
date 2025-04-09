#ifndef UTILITYFUNCTIONS_H
#define UTILITYFUNCTIONS_H

#include <stdlib.h>
#include <cmath>

template <typename T>
inline T constrain(T val, T low, T high)
{
    if(val < low) return low;
    if(val > high) return high;
    return val;
}

inline float deadband(float value, float threshold)
{
    if (fabs(value) < threshold) value = 0;
    else if (value > 0) value -= threshold;
    else if (value < 0) value += threshold;
    return value;
}

inline float softDeadband(float value, float threshold, float radius)
{
    if (fabs(value) < threshold) value = 0;
    else if (value > 0) value -= threshold * constrain(value / (threshold + radius), 0.0f, 1.0f);
    else if (value < 0) value += threshold * constrain(abs(value) / (threshold + radius), 0.0f, 1.0f);
    return value;
}

#endif // UTILITYFUNCTIONS_H
