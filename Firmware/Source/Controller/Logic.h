#ifndef __LOGIC_H
#define __LOGIC_H

#include "stdinc.h"

typedef struct __AdapterIdentifier
{
	Int16U Code;
	Int16U ClampHeightMm;
	Int16U MaxCurrent;
	Int16U MaxVoltage;
	Int16U Serial;
	Boolean Cached;
} AdapterIdentifier, *pAdapterIdentifier;

extern AdapterIdentifier LOGIC_Id;

void LOGIC_Process();
Boolean LOGIC_IsCycleActive();

void LOGIC_AdapterIdInit();
Boolean LOGIC_AdapterIdRead(pAdapterIdentifier Id);
Boolean LOGIC_AdapterIdWrite(pAdapterIdentifier Id);
Boolean LOGIC_ValidateAdapter(pAdapterIdentifier Id);

#endif // __LOGIC_H
