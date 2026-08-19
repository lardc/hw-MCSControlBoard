#ifndef __STORAGE_DESCRIPTION_H
#define __STORAGE_DESCRIPTION_H

#include "stdinc.h"
#include "SaveToFlash.h"

extern RecordDescription StorageDescription[];
extern Int32U TablePointers[];
extern const Int16U StorageSize;

extern const CounterDescription CounterStorageDescription[];
extern CounterData CounterTablePointers[];
extern const Int16U CounterStorageSize;

#endif // __STORAGE_DESCRIPTION_H
