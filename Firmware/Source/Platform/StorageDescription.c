#include "StorageDescription.h"
#include "Global.h"

RecordDescription StorageDescription[] =
{
	{"REG_DEV_STATE",				DT_Float, 1},
	{"REG_FAULT_REASON",			DT_Float, 1},
	{"REG_DISABLE_REASON",			DT_Float, 1},
	{"REG_WARNING",					DT_Float, 1},
	{"REG_PROBLEM",					DT_Float, 1},
	{"REG_OP_RESULT",				DT_Float, 1},

	{"REG_TEMP_CH1",				DT_Float, 1},
	{"REG_TRM_DATA",				DT_Float, 1},
	{"REG_TRM_ERROR",				DT_Float, 1},
	{"REG_PRESSURE",				DT_Float, 1},
	{"REG_SENSOR_S2",				DT_Float, 1},
	{"REG_HOMING_SENSOR",			DT_Float, 1},
	{"REG_BUS_TOOLING_SENSOR",		DT_Float, 1},
	{"REG_ADAPTER_TOOLING_SENSOR",	DT_Float, 1},

	{"REG_DEV_SUBSTATE",			DT_Float, 1},
	{"REG_SELFTEST_RESULT",			DT_Float, 1},

	{"REG_ADAPTER_MATCH",			DT_Float, 1},
	{"REG_ADAPTER_MISMATCH_CODE",	DT_Float, 1},

	{"REG_SPI_IN_STATE",			DT_Float, 1},
	{"REG_SENSOR_S3",				DT_Float, 1},
	{"REG_SENSOR_S5",				DT_Float, 1},
	{"REG_DEBUG_SCALING_COEF",		DT_Float, 1},

	{"EP_MotorMovement",			DT_Float, VALUES_x_SIZE},
	{"EP_MotorSpeed",				DT_Float, VALUES_x_SIZE},
};

Int32U TablePointers[sizeof(StorageDescription) / sizeof(StorageDescription[0])] = {0};
const Int16U StorageSize = sizeof(StorageDescription) / sizeof(StorageDescription[0]);

const CounterDescription CounterStorageDescription[] =
{
	{"0. Pneumatic cylinders"},
	{"1. Bus"},
	{"2. Clampings"},
};
CounterData CounterTablePointers[sizeof(CounterStorageDescription) / sizeof(CounterStorageDescription[0])] = {0};
const Int16U CounterStorageSize = sizeof(CounterStorageDescription) / sizeof(CounterStorageDescription[0]);
