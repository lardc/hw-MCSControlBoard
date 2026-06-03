// Header
#include "DebugActions.h"

// Include
//
#include "LowLevel.h"
#include "Board.h"
#include "Delay.h"
#include "Controller.h"
#include "DataTable.h"

// Functions
// Send pulse to "CSM Block" indicator
void DBACT_ToggleIndCSM()
{
	LL_IndicateBlockCSM(true);
	DELAY_MS(1000);
	LL_IndicateBlockCSM(false);
}
//-----------------------

// Send pulse to "Adapter Block" indicator
void DBACT_ToggleIndADPTR()
{
	LL_IndicateBlockAdapter(true);
	DELAY_MS(1000);
	LL_IndicateBlockAdapter(false);
}
//-----------------------

// Send pulse to Top adapter blocking pneumatic cylinder
void DBACT_TogglePneumTOP()
{
	LL_HoldTopAdapter(true);
	DELAY_MS(1000);
	LL_HoldTopAdapter(false);
}
//-----------------------

// Send pulse to Bottom adapter blocking pneumatic cylinder
void DBACT_TogglePneumBOT()
{
	LL_HoldBotAdapter(true);
	DELAY_MS(1000);
	LL_HoldBotAdapter(false);
}
//-----------------------

// Send pulse to DUT blocking pneumatic cylinder
void DBACT_TogglePneumDUT()
{
	LL_ClampDUT(true);
	DELAY_MS(1000);
	LL_ClampDUT(false);
}
//-----------------------

// Send pulse to Safety Subsystem Output
void DBACT_ToggleSFOutput()
{
	LL_SetSafetyOutput(true);
	DELAY_MS(1000);
	LL_SetSafetyOutput(false);
}
//-----------------------

// Measure voltage at the resistor divider on the top side of the adapter
void DBACT_MeasureIDDividerTop()
{
	DataTable[REG_RSLT] = LL_MeasureIDTop();
}
//-----------------------

// Measure voltage at the resistor divider on the bottom side of the adapter
void DBACT_MeasureIDDividerBot()
{
	DataTable[REG_RSLT] = LL_MeasureIDBot();
}
//-----------------------

// Measure voltage from the pressure sensor
void DBACT_MeasurePressure()
{
	DataTable[REG_RSLT] = LL_MeasurePressure();
}
//-----------------------
