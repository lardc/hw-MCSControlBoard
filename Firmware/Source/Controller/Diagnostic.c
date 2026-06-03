// Header
#include "Diagnostic.h"

// Includes
#include "DataTable.h"
#include "LowLevel.h"
#include "Controller.h"
#include "DebugActions.h"

// Functions
bool DIAG_HandleDiagnosticAction(uint16_t ActionID, uint16_t *pUserError)
{
	switch(ActionID)
	{
		case ACT_DBG_IND_CSM:
			DBACT_ToggleIndCSM();
			break;
			
		case ACT_DBG_IND_ADPTR:
			DBACT_ToggleIndADPTR();
			break;
			
		case ACT_DBG_PNEUM_TOP:
			DBACT_TogglePneumTOP();
			break;
			
		case ACT_DBG_PNEUM_BOT:
			DBACT_TogglePneumBOT();
			break;
			
		case ACT_DBG_PNEUM_DUT:
			DBACT_TogglePneumDUT();
			break;
			
		case ACT_DBG_SF_OUT:
			DBACT_ToggleSFOutput();
			break;
			
		case ACT_DBG_MEASURE_ID_TOP:
			DBACT_MeasureIDDividerTop();
			break;
			
		case ACT_DBG_MEASURE_ID_BOT:
			DBACT_MeasureIDDividerBot();
			break;
			
		case ACT_DBG_MEASURE_PRESSURE:
			DBACT_MeasurePressure();
			break;
			
		default:
			return false;
	}
	
	return true;
}
//-------------------------------------
