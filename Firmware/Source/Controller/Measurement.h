#ifndef __MEASUREMENT_H
#define __MEASUREMENT_H

#include "stdinc.h"

float MEAS_GetPressureBar();
Int32U MEAS_GetPressureMilliBar();
Boolean MEAS_IsPressureOk();
float MEAS_GetRawVoltage();

#endif // __MEASUREMENT_H
