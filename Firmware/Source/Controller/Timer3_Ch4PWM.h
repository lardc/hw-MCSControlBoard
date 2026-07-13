#ifndef __TIMER3_CH4PWM_H
#define __TIMER3_CH4PWM_H

// Include
#include "stdinc.h"

// Functions
void T3Ch4PWM_Init(uint32_t SystemClock, uint32_t Period);
void T3Ch4PWM_SetDutyCycle(float Value);
void T3Ch4PWM_Start();
void T3Ch4PWM_Stop();
uint32_t T3Ch4PWM_GetPWMBase();

#endif // __TIMER3_CH4PWM_H
