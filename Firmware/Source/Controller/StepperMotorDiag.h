// ----------------------------------------
// SM driver module diag
// ----------------------------------------

#ifndef __STEPPER_MOTOR_DIAG_H
#define __STEPPER_MOTOR_DIAG_H

// Include
#include "stdinc.h"

void SMD_ConnectHandler();
Boolean SMD_GoToDistanceMm(Int16U PositionMm);
void SMD_RequstStop();

#endif // __STEPPER_MOTOR_DIAG_H
