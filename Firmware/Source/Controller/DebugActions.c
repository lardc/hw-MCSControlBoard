// Header
#include "DebugActions.h"

// Includes
#include "DataTable.h"
#include "TRM10.h"
#include "StepperMotorDiag.h"
#include "DS18B20.h"
#include "DS2431.h"
#include "OneWire.h"
#include "Controller.h"
#include "MemLabel.h"
#include "Measurement.h"
#include "LowLevel.h"
#include "Delay.h"
#include "Timer3_Ch4PWM.h"
#include "Logic.h"
#include "StepperMotor.h"

// Variables
static Int8U DS2431DeviceIndex = 0;
MemLabelEntry Labels[MEM_LABEL_MAX_LABELS];

// Functions
bool DEBUG_HandleDiagnosticAction(uint16_t ActionID, uint16_t *UserError)
{
	switch(ActionID)
	{
		case ACT_DBG_MEAS_PRESSURE:
			DataTable[REG_DBG] = MEAS_GetRawVoltage();
			break;
		case ACT_DBG_SET_OUTPUT:
			{
				Int16U Exit = DataTable[REG_DBG];
				if(DataTable[REG_DBG] > 7)
					break;
				LL_SPI_SetOutBit(Exit, true);
				LL_SPI_FlushOut();
			}
			break;
		case ACT_DBG_MEAS_INPUT:
			DataTable[REG_DBG] = LL_SPI_ReadInRaw();
			break;
		case ACT_DBG_STPM:
			T3Ch4PWM_Stop();
			GPIO_InitPushPullOutput(GPIO_STPM_STEP);

			GPIO_SetState(GPIO_STPM_DIR, true);
			GPIO_SetState(GPIO_STPM_EN, true);
			GPIO_SetState(GPIO_STPM_STEP, true);
			DELAY_MS(100);
			GPIO_SetState(GPIO_STPM_DIR, false);
			GPIO_SetState(GPIO_STPM_EN, false);
			GPIO_SetState(GPIO_STPM_STEP, false);

			GPIO_InitAltFunction(GPIO_STPM_STEP, AltFn_2);
			break;
		case ACT_DBG_STPM_EN:
			GPIO_SetState(GPIO_STPM_EN, DataTable[REG_DBG]);
			break;
		case ACT_DBG_DQ_PWR:
			{
				Boolean prev = GPIO_GetState(GPIO_DQ_PWR);
				GPIO_SetState(GPIO_DQ_PWR, !prev);
				DELAY_MS(100);
				GPIO_SetState(GPIO_DQ_PWR, prev);
			}
			break;
		case ACT_DBG_DQ_CTRL:
			{
				Boolean prev = GPIO_GetState(GPIO_DQ_CTRL);
				GPIO_SetState(GPIO_DQ_CTRL, true);
				DELAY_MS(100);
				GPIO_SetState(GPIO_DQ_CTRL, prev);
			}
			break;
		case ACT_DBG_DQ_IN:
			DataTable[REG_DBG] = GPIO_GetState(GPIO_DQ_IN);
			break;
		case ACT_DBG_HOMING:
			DataTable[REG_DBG] = LL_HomeSensorActuate();
			break;
		case ACT_DBG_SFT:
			DataTable[REG_DBG] = LL_IsSafetyS3Ok();
			break;
		case ACT_DBG_OPTICAL:
			DataTable[REG_DBG] = LL_IsSafetyS5Ok();
			break;
		case ACT_DBG_TRM_READ:
			{
				TRMError error;
				DataTable[REG_TRM_DATA] = TRM10_ReadReg((Int8U)DataTable[REG_DBG_TRM_ADDRESS], (Int16U)DataTable[REG_DBG], &error);
				DataTable[REG_TRM_ERROR] = error;
			}
			break;
		case ACT_DBG_TRM_WRITE:
			{
				TRMError error;
				TRM10_WriteReg((Int8U)DataTable[REG_DBG_TRM_ADDRESS], (Int16U)DataTable[REG_DBG], (float)DataTable[REG_DBG2], &error);
				DataTable[REG_TRM_ERROR] = error;
			}
			break;

		case ACT_DBG_READ_EXT_TEMP:
		case ACT_DBG_READ_TRM_TEMP:
			{
				if(DataTable[REG_USE_HEATING])
				{
					TRMError error;

					DataTable[REG_TRM_DATA] = TRM10_ReadTemp(
							(Int8U)DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;
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

					DataTable[REG_TRM_DATA] = TRM10_ReadPower(
							(Int8U)DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;
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

					TRM10_Start((Int8U)DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;
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

					TRM10_Stop((Int8U)DataTable[REG_DBG_TRM_ADDRESS], &error);
					DataTable[REG_TRM_ERROR] = error;
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
			SM_RequestStop();
			break;

		case ACT_DBG_DS18_READ:
			{
				Int16U value;

				if(DS18B20_ReadReg(&value))
					DataTable[REG_DBG] = value;
				else
					CONTROL_FinishedWithProblem(PROBLEM_OW_DS18);
			}
			break;

		case ACT_DBG_DS18_WRITE:
			{
				Int16U value = (Int16U)DataTable[REG_DBG];

				if(!DS18B20_WriteReg(&value))
					CONTROL_FinishedWithProblem(PROBLEM_OW_DS18);
			}
			break;

		case ACT_DBG_ONEWIRE_SEARCH:
			{
				Int16U ds18Count = 0;
				Int16U ds2431Count = 0;

				DataTable[REG_DBG] = 0;

				if(!OneWire_SearchFamily(DS18B20_FAMILY_CODE, &ds18Count, NULL)
						|| !OneWire_SearchFamily(DS2431_FAMILY_CODE, &ds2431Count, NULL))
				{
					CONTROL_FinishedWithProblem(PROBLEM_OW_ERROR_LINE);
					break;
				}

				DataTable[REG_DBG] = ds18Count + ds2431Count*1000;

				if(DataTable[REG_DBG] == 0)
					CONTROL_FinishedWithProblem(PROBLEM_OW_NO_DEVICE);
			}
			break;

		case ACT_DBG_DS18_READ_TEMP:
			{
				float temp;

				if(DS18B20_ReadTemperature(&temp))
					DataTable[REG_DBG] = temp;
				else
					CONTROL_FinishedWithProblem(PROBLEM_OW_DS18);
			}
			break;

		case ACT_DBG_DS2431_ERASE:
			{
				DS2431DeviceIndex = (Int8U)DataTable[REG_DBG];

				if(!DS2431_EraseAll(DS2431DeviceIndex, true))
					CONTROL_FinishedWithProblem(CONTROL_ProblemFromDs2431());
			}
			break;

		case ACT_DBG_DS2431_READ:
			{
				Int8U buf[2];

				DS2431DeviceIndex = (Int8U)DataTable[REG_DBG];

				if(!DS2431_ReadArray(DS2431DeviceIndex, buf, sizeof(buf)))
					CONTROL_FinishedWithProblem(CONTROL_ProblemFromDs2431());
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
					CONTROL_FinishedWithProblem(CONTROL_ProblemFromDs2431());
			}
			break;

		case ACT_DBG_LABEL_FIND:
			DataTable[REG_DBG] = (MemLabel_Read(0, Labels, MEM_LABEL_MAX_LABELS) > 0) ? 1 : 0;
			if(DataTable[REG_DBG] == 0 && DS2431_GetLastError() != DS2431_OK)
				CONTROL_FinishedWithProblem(CONTROL_ProblemFromDs2431());
			break;

		case ACT_DBG_LABEL_SHOW_AMOUNT:
			DataTable[REG_DBG] = MemLabel_Read(0, Labels, MEM_LABEL_MAX_LABELS);
			if(DataTable[REG_DBG] == 0 && DS2431_GetLastError() != DS2431_OK)
				CONTROL_FinishedWithProblem(CONTROL_ProblemFromDs2431());
			break;

		case ACT_DBG_LABEL_ERASE:
			if(!MemLabel_EraseAll(0))
				CONTROL_FinishedWithProblem(CONTROL_ProblemFromDs2431());
			break;

		case ACT_DBG_LABEL_WRITE:
			{
				MemLabelEntry DataEntry;

				if(DataTable[REG_DBG] >= 0xFF)
					break;

				DataEntry.Type = (Int8U)DataTable[REG_DBG];
				DataEntry.Value = DataTable[REG_DBG2];
				if(!MemLabel_AddOne(0, DataEntry))
				{
					if(DS2431_GetLastError() != DS2431_OK)
						CONTROL_FinishedWithProblem(CONTROL_ProblemFromDs2431());
					else
						CONTROL_FinishedWithProblem(PROBLEM_OW_PARAM);
				}
			}
			break;

		case ACT_DBG_LABEL_READ_DATA:
			{
				Int8U Index = DataTable[REG_DBG];
				if(Index >= MEM_LABEL_MAX_LABELS)
					break;
				MemLabel_Read(0, Labels, MEM_LABEL_MAX_LABELS);
				DataTable[REG_DBG] = Labels[Index].Type;
				DataTable[REG_DBG2] = Labels[Index].Value;
			}
			break;

		case ACT_DBG_ADAPTER_WRITE_ID:
			LOGIC_AdapterIdInit();
			{
				AdapterIdentifier Id;
				Id.Code = DataTable[REG_DEV_CASE];
				Id.ClampHeightMm = DataTable[REG_DBG_ADAPTER_CLAMP_HEIGHT];
				Id.MaxCurrent = DataTable[REG_TEST_CURRENT];
				Id.MaxVoltage = DataTable[REG_TEST_VOLTAGE];
				Id.Serial = DataTable[REG_DBG_ADAPTER_SERIAL];
				Id.Version = DataTable[REG_DBG_ADAPTER_VERSION];
				Id.Device = DataTable[REG_DBG_ADAPTER_DEVICE];
				if(LOGIC_AdapterIdWrite(&Id))
					DataTable[REG_OP_RESULT] = OPRESULT_OK;
			}
			break;

		default:
			return false;
	}

	return true;
}
//-------------------------------------
