#ifndef __LOWLEVEL_H
#define __LOWLEVEL_H

#include "Board.h"
#include "stdinc.h"

void LL_InitGPIO();

Boolean LL_FilterSafetyCircuit(Boolean NewState);
void LL_CSMux(Int16U SPIDevice);

Boolean LL_IsSafetySensorOk();
Boolean LL_HomeSensorActuate();
Boolean LL_IsBusToolingSensorOk();
Boolean LL_IsAdapterToolingSensorOk();

void LL_SwitchPowerConnection(Boolean State);
Boolean LL_IsPowerConnected();
void LL_SwitchControlConnection(Boolean State);
Boolean LL_IsControlConnected();

void LL_SwitchStep(Boolean State);
void LL_ToggleStep();
void LL_SwitchUpDir(Boolean State);
Boolean LL_IsDirUp();
void LL_SwitchEnable(Boolean State);
void LL_SwitchFan(Boolean State);

void LL_ToggleBoardLED();
float LL_MeasurePressure();

#endif // __LOWLEVEL_H
