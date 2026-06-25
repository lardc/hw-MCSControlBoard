// Header
#include "DebugActions.h"

// Includes
#include "DataTable.h"
#include "TRM101.h"
#include "StepperMotorDiag.h"
#include "DS18B20.h"
#include "DS2431.h"
#include "OneWire.h"
#include "Controller.h"

// Variables
static Int8U DS2431DeviceIndex = 0;

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
				Int16U ds18Count = 0;
				Int16U ds2431Count = 0;

				DataTable[REG_DBG] = 0;

				if(!OneWire_SearchFamily(DS18B20_FAMILY_CODE, &ds18Count, NULL)
						|| !OneWire_SearchFamily(DS2431_ONE_WIRE_FAMILY_CODE, &ds2431Count, NULL))
				{
					CONTROL_FinishedWithProblem(PROBLEM_ONEWIRE);
					break;
				}

				DataTable[REG_DBG] = ds18Count + ds2431Count*1000;

				if(DataTable[REG_DBG] == 0)
					CONTROL_FinishedWithProblem(PROBLEM_ONEWIRE);
			}
			break;

		case ACT_DBG_DS18_READ_TEMP:
			{
				float temp;

				if(DS18B20_ReadTemperature(&temp))
					DataTable[REG_DBG] = (Int16U)(temp * 10);
				else
					CONTROL_FinishedWithProblem(PROBLEM_ONEWIRE);
			}
			break;

		case ACT_DBG_DS2431_ERASE:
			{
				DS2431DeviceIndex = (Int8U)DataTable[REG_DBG];

				if(!DS2431_EraseAll(DS2431DeviceIndex, true))
					CONTROL_FinishedWithProblem(PROBLEM_ONEWIRE);
			}
			break;

		case ACT_DBG_DS2431_READ:
			{
				Int8U buf[2];

				DS2431DeviceIndex = (Int8U)DataTable[REG_DBG];

				if(!DS2431_ReadArray(DS2431DeviceIndex, buf, sizeof(buf)))
					CONTROL_FinishedWithProblem(PROBLEM_ONEWIRE);
				else
					DataTable[REG_DBG] = ((Int16U)buf[0] << 8) | buf[1];
			}
			break;

		case ACT_DBG_DS2431_WRITE:
			{
				// REG_DBG — данные (десят.); индекс устройства — из предшествующего READ (128) или ERASE (127)
				Int16U value = DataTable[REG_DBG];
				Int8U buf[2];

				buf[0] = (Int8U)((value >> 8) & 0xFF);
				buf[1] = (Int8U)(value & 0xFF);

				if(!DS2431_WriteArray(DS2431DeviceIndex, buf, sizeof(buf)))
					CONTROL_FinishedWithProblem(PROBLEM_ONEWIRE);
			}
			break;

		default:
			return false;
	}

	return true;
}
//-------------------------------------
