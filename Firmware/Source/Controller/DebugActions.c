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

// Forward functions
static Boolean DEBUG_SearchOneWireFamily(Int8U familyCode, Int16U *foundCount, Int8U firstRom[8]);
static Boolean DEBUG_SelectDS2431ByIndex(Int8U deviceIndex);

// Variables
static Int8U DbgDS2431DeviceIndex = 0;

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

				if(!DEBUG_SearchOneWireFamily(DS18B20_FAMILY_CODE, &ds18Count, NULL)
						|| !DEBUG_SearchOneWireFamily(DS2431_ONE_WIRE_FAMILY_CODE, &ds2431Count, NULL))
				{
					CONTROL_FinishedWithProblem(PROBLEM_ONEWIRE);
					break;
				}

				DataTable[REG_DBG] = ds18Count + ds2431Count;

				if(DataTable[REG_DBG] == 0)
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

		case ACT_DBG_DS2431_ERASE:
			{
				DbgDS2431DeviceIndex = (Int8U)DataTable[REG_DBG];

				if(!DEBUG_SelectDS2431ByIndex(DbgDS2431DeviceIndex)
						|| !DS2431_EraseAll(false))
					CONTROL_FinishedWithProblem(PROBLEM_ONEWIRE);
			}
			break;

		case ACT_DBG_DS2431_READ:
			{
				Int8U buf[2];

				DbgDS2431DeviceIndex = (Int8U)DataTable[REG_DBG];

				if(!DEBUG_SelectDS2431ByIndex(DbgDS2431DeviceIndex)
						|| !DS2431_ReadData(buf, sizeof(buf)))
					CONTROL_FinishedWithProblem(PROBLEM_ONEWIRE);
				else
					DataTable[REG_DBG] = ((Int16U)buf[0] << 8) | buf[1];
			}
			break;

		case ACT_DBG_DS2431_WRITE:
			{
				Int16U value = DataTable[REG_DBG];
				Int8U buf[2];

				buf[0] = (Int8U)((value >> 8) & 0xFF);
				buf[1] = (Int8U)(value & 0xFF);

				if(!DEBUG_SelectDS2431ByIndex(DbgDS2431DeviceIndex)
						|| !DS2431_WriteData(buf, sizeof(buf)))
					CONTROL_FinishedWithProblem(PROBLEM_ONEWIRE);
			}
			break;

		default:
			return false;
	}

	return true;
}
//-------------------------------------

static Boolean DEBUG_SearchOneWireFamily(Int8U familyCode, Int16U *foundCount, Int8U firstRom[8])
{
	Int8U addr[8];

	*foundCount = 0;

	OneWire_ResetSearch();
	OneWire_TargetSearch(familyCode);

	while(OneWire_Search(addr, true))
	{
		if(!OneWire_CheckCrc8(addr, 7, addr[7]))
			return false;

		if(*foundCount == 0 && firstRom != NULL)
		{
			for (Int8U i = 0; i < 8; i++)
				firstRom[i] = addr[i];
		}

		(*foundCount)++;
	}

	return true;
}
//-------------------------------------

static Boolean DEBUG_SelectDS2431ByIndex(Int8U deviceIndex)
{
	Int8U addr[8];
	Int16U count = 0;

	OneWire_ResetSearch();
	OneWire_TargetSearch(DS2431_ONE_WIRE_FAMILY_CODE);

	while(OneWire_Search(addr, true))
	{
		if(!OneWire_CheckCrc8(addr, 7, addr[7]))
			return false;

		if(count == deviceIndex)
		{
			DS2431_Begin(addr);
			return true;
		}

		count++;
	}

	return false;
}
//-------------------------------------
