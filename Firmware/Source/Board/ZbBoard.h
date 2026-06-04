// -----------------------------------------
// Board layer for MCS (STM32) — replaces TMS ZbBoard / ZwDSP
// -----------------------------------------

#ifndef __ZB_BOARD_H
#define __ZB_BOARD_H

#include "stdinc.h"
#include "LowLevel.h"
#include "ZwTIM.h"
#include "Delay.h"

void MemCopy16(pInt16U Src, pInt16U Dst, Int16U Size);

#endif // __ZB_BOARD_H
