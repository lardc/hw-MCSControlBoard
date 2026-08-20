#ifndef __INITCONFIG_H
#define __INITCONFIG_H

#include "stdinc.h"
#include "SysConfig.h"

// Variables
extern Int16U INITCFG_PressureAdcBuffer[ADC_PRESSURE_BUF_SIZE];

// Functions
Boolean INITCFG_ConfigSystemClock();
void INITCFG_ConfigGPIO();
void INITCFG_ConfigOneWire();
void INITCFG_ConfigModbus();
void INITCFG_ConfigUART();
void INITCFG_ConfigTRMUART();
void INITCFG_ConfigCAN();
void INITCFG_ConfigSPI();
void INITCFG_ConfigRS485();
void INITCFG_ConfigTimer7();
void INITCFG_PWM();
void INITCFG_ConfigTimer15();
void INITCFG_ConfigWatchDog();
void INITCFG_ConfigADC();
void INITCFG_ConfigDMA();

#endif // __INITCONFIG_H
