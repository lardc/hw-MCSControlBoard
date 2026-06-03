// ----------------------------------------
// SM driver module diag
// ----------------------------------------

// Header
//
#include "StepperMotorDiag.h"
#include "StepperMotor.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"

// Variables
//
static Boolean RequestStop = FALSE;
static Int16U StepDivisorLimit = 0, TicksMaxCounter = 0;

// Funcions
//
// Main logic handler
void SMD_LogicHandler()
{
	static Boolean TickHigh = FALSE;
	static Int16U StepDivisorTicks = 0, TicksCounter = 0;

	if(++StepDivisorTicks >= StepDivisorLimit)
	{
		StepDivisorTicks = 0;
		ZbGPIO_SwitchStep(TickHigh = !TickHigh);
	}

	if(!TickHigh)
	{
		++TicksCounter;

		if(RequestStop || (TicksMaxCounter != 0 && TicksCounter >= TicksMaxCounter))
		{
			TicksCounter = 0;
			RequestStop = FALSE;
			SM_ConnectAlterHandler(NULL);
		}
	}
}
// ----------------------------------------

// Main logic handler
void SMD_RequstStop()
{
	RequestStop = TRUE;
}
// ----------------------------------------

void SMD_ConnectHandler()
{
	StepDivisorLimit = DataTable[REG_DBG_STEP_DIV];
	TicksMaxCounter = DataTable[REG_DBG_STEPS_MAX];

	SM_ConnectAlterHandler(&SMD_LogicHandler);
}
// ----------------------------------------
