#include "SelfTest.h"

#include "Board.h"
#include "Delay.h"
#include "LowLevel.h"

#define SELFTEST_STIMULUS_PAUSE_MS	25

static Boolean SELFTEST_CheckOpto(GPIO_PortPinSetting Sensor)
{
	Boolean Sample0, Sample1, Sample2;

	Sample0 = GPIO_GetState(Sensor);

	LL_SetTestLine(true);
	DELAY_MS(SELFTEST_STIMULUS_PAUSE_MS);
	Sample1 = GPIO_GetState(Sensor);

	DELAY_MS(SELFTEST_STIMULUS_PAUSE_MS);
	LL_SetTestLine(false);
	DELAY_MS(SELFTEST_STIMULUS_PAUSE_MS);
	Sample2 = GPIO_GetState(Sensor);

	return (Sample0 != Sample1) && (Sample1 != Sample2);
}
//-----------------------------

Int16U SELFTEST_Run()
{
	Int16U Result = 0;

	if(!SELFTEST_CheckOpto(GPIO_SEN_S4))
		Result |= SELFTEST_FAIL_OSH1;

	if(!SELFTEST_CheckOpto(GPIO_SEN_S1))
		Result |= SELFTEST_FAIL_OSH2;

	LL_SetTestLine(false);

	return Result;
}
//-----------------------------
