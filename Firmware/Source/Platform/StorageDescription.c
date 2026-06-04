#include "StorageDescription.h"

RecordDescription StorageDescription[] =
{
	{"Homing duration",		DT_Int32U, 1},
	{"Clamping duration",	DT_Int32U, 1},
	{"Release duration",	DT_Int32U, 1},
};

Int32U TablePointers[sizeof(StorageDescription) / sizeof(StorageDescription[0])] = {0};
const Int16U StorageSize = sizeof(StorageDescription) / sizeof(StorageDescription[0]);
