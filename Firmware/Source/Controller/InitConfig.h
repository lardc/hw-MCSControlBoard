#ifndef __INITCONFIG_H
#define __INITCONFIG_H

#include "stdinc.h"
#include "SysConfig.h"

Boolean INITCFG_ConfigSystemClock();
void INITCFG_ConfigIO();
void INITCFG_ConfigUART();
void INITCFG_ConfigUSART3();
void INITCFG_ConfigCAN();
void INITCFG_ConfigSPI();
void INITCFG_ConfigRS485();
void INITCFG_ConfigTimer7();
void INITCFG_ConfigTimer1();
void INITCFG_ConfigTimer15();
void INITCFG_ConfigWatchDog();
void INITCFG_ConfigADC();
void INITCFG_ConfigDMA();

extern Int16U INITCFG_PressureAdcBuffer[ADC_PRESSURE_BUF_SIZE];

#endif // __INITCONFIG_H
