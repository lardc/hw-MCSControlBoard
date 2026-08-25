// Header
#include "SelfTest.h"
// Includes
#include "Board.h"
#include "LowLevel.h"
#include "Controller.h"

// Defines
#define SELFTEST_STIMULUS_PAUSE_MS	25

// Types
typedef enum __SelfTestState
{
	STS_None = 0,
	STS_FirstCheck = 1,
	STS_PauseAfter1st = 2,
	STS_PauseAfter2nd = 3,
	STS_PauseAfter3rd = 4
} SelfTestState;

// Variables
static SelfTestState STState = STS_None;
static Int64U Timeout = 0;
static Boolean Sample0, Sample1, Sample2;
static Boolean TestingOSH2 = FALSE;
static Int16U ResultMask = 0;

// Functions
// Возвращает TRUE, когда проверка одного оптрона закончена; *Passed — результат
static Boolean SELFTEST_CheckOpto(GPIO_PortPinSetting Sensor, Boolean *Passed)
{
	switch(STState)
	{
		case STS_FirstCheck:
			Sample0 = Sample1 = Sample2 = 0;
			Sample0 = GPIO_GetState(Sensor);
			LL_SetTestLine(true);
			Timeout = CONTROL_TimeCounter + SELFTEST_STIMULUS_PAUSE_MS;
			STState = STS_PauseAfter1st;
			break;

		case STS_PauseAfter1st:
			if(CONTROL_TimeCounter > Timeout)
			{
				Sample1 = GPIO_GetState(Sensor);
				Timeout = CONTROL_TimeCounter + SELFTEST_STIMULUS_PAUSE_MS;
				STState = STS_PauseAfter2nd;
			}
			break;

		case STS_PauseAfter2nd:
			if(CONTROL_TimeCounter > Timeout)
			{
				LL_SetTestLine(false);
				Timeout = CONTROL_TimeCounter + SELFTEST_STIMULUS_PAUSE_MS;
				STState = STS_PauseAfter3rd;
			}
			break;

		case STS_PauseAfter3rd:
			if(CONTROL_TimeCounter > Timeout)
			{
				Sample2 = GPIO_GetState(Sensor);
				STState = STS_None;
				*Passed = (Sample0 != Sample1) && (Sample1 != Sample2);
				return TRUE;
			}
			break;

		default:
			STState = STS_FirstCheck;
			break;
	}

	return FALSE;
}
//-----------------------------

Int16U SELFTEST_Run()
{
	Boolean Passed = FALSE;

	if(STState == STS_None)
		STState = STS_FirstCheck;

	if(!TestingOSH2)
	{
		if(!SELFTEST_CheckOpto(GPIO_SEN_S4, &Passed))
			return SELFTEST_IN_PROGRESS;

		if(!Passed)
			ResultMask |= SELFTEST_FAIL_OSH1;

		TestingOSH2 = TRUE;
		return SELFTEST_IN_PROGRESS;
	}

	if(!SELFTEST_CheckOpto(GPIO_SEN_S1, &Passed))
		return SELFTEST_IN_PROGRESS;

	if(!Passed)
		ResultMask |= SELFTEST_FAIL_OSH2;

	LL_SetTestLine(false);
	Int16U Result = ResultMask;
	TestingOSH2 = FALSE;
	ResultMask = 0;
	STState = STS_None;
	return Result;
}
//-----------------------------
