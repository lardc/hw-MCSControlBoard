// Header
#include "DebugActions.h"

// Includes
#include "DataTable.h"
#include "TRM101.h"
#include "StepperMotorDiag.h"
#include "DS18B20.h"
#include "OneWire.h"
#include "Controller.h"

// Functions
bool DEBUG_HandleDiagnosticAction(uint16_t ActionID, uint16_t *UserError)
{
	switch(ActionID)
	{
		case ACT_DBG_READ_EXT_TEMP:
		case ACT_DBG_READ_TRM_TEMP:
			{
				if(DataTable[REG_USE_HEATING])
				{
					TRMError error;
					DataTable[REG_TRM_DATA] = TRM_ReadTemp(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;

					if(error != TRME_None)
						*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_READ_TRM_POWER:
			{
				if(DataTable[REG_USE_HEATING])
				{
					TRMError error;
					DataTable[REG_TRM_DATA] = TRM_ReadPower(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;

					if(error != TRME_None)
						*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_TRM_START:
			{
				if(DataTable[REG_USE_HEATING])
				{
					TRMError error;
					TRM_Start(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;

					if(error != TRME_None)
						*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_TRM_STOP:
			{
				if(DataTable[REG_USE_HEATING])
				{
					TRMError error;
					TRM_Stop(DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;

					if(error != TRME_None)
						*UserError = ERR_TRM_COMM_ERR;
				}
				else
					*UserError = ERR_OPERATION_BLOCKED;
			}
			break;

		case ACT_DBG_MOTOR_START:
			SMD_ConnectHandler();
			break;

		case ACT_DBG_MOTOR_STOP:
			SMD_RequstStop();
			break;

		case ACT_DBG_DS18_READ:
			{
				Int16U value;

				if(DS18B20_ReadReg(&value))
					DataTable[REG_DBG] = value;
				else
					CONTROL_FinishedWithProblem(PROBLEM_ONEWIRE);
			}
			break;

		case ACT_DBG_DS18_WRITE:
			{
				Int16U value = (Int16U)DataTable[REG_DBG];

				if(!DS18B20_WriteReg(&value))
					CONTROL_FinishedWithProblem(PROBLEM_ONEWIRE);
			}
			break;

		case ACT_DBG_ONEWIRE_SEARCH:
			{
				Int8U addr[8];
				Int16U count = 0;
				Boolean crcOk = true;

				DataTable[REG_DBG] = 0;

				OneWire_ResetSearch();
				OneWire_TargetSearch(DS18B20_FAMILY_CODE);

				while(OneWire_Search(addr, true))
				{
					if(!OneWire_CheckCrc8(addr, 7, addr[7]))
					{
						crcOk = false;
						break;
					}

					count++;
				}

				DataTable[REG_DBG] = count;

				if(!crcOk || count == 0)
					CONTROL_FinishedWithProblem(PROBLEM_ONEWIRE);
			}
			break;

		case ACT_DBG_DS18_READ_TEMP:
			{
				Int16S temp;

				if(DS18B20_ReadTemperatureC10(&temp))
					DataTable[REG_DBG] = temp;
				else
					CONTROL_FinishedWithProblem(PROBLEM_ONEWIRE);
			}
			break;

		default:
			return false;
	}

	return true;
}
//-------------------------------------
