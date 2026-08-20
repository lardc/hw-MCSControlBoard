#ifndef __TIMER3_CH4PWM_H
#define __TIMER3_CH4PWM_H

// Include
#include "stdinc.h"

// Defines
#define T3CH4PWM_MAX_OUTPUT	0.95f

// Functions
void T3Ch4PWM_Init(uint32_t SystemClock, uint32_t Period);
// Cycles — полный период STEP в тиках таймера (ARR + 1), скважность 50%
void T3Ch4PWM_SetPeriodTicks(uint32_t Cycles);
Boolean T3Ch4PWM_Start();
Boolean T3Ch4PWM_Stop();
uint32_t T3Ch4PWM_GetMaxCycles();

#endif // __TIMER3_CH4PWM_H
