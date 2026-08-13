// -----------------------------------------
// Global definitions
// ----------------------------------------

#ifndef __CONSTRAINTS_H
#define __CONSTRAINTS_H

// Include
#include "stdinc.h"
//
#include "DataTable.h"
#include "Global.h"
#include "LowLevel.h"

// Types
//
typedef struct __TableItemConstraint
{
	Int16U Min;
	Int16U Max;
	Int16U Default;
} TableItemConstraint;

// Параметры позиционирования (мм)
#define POS_MAX						180

// Расстояние замедления (мм)
#define SLOW_DOWN_MIN				0
#define SLOW_DOWN_MAX				50
#define SLOW_DOWN_DEF				10

// Оффсет хоуминга (мм)
#define HOMING_OFFS_MIN				1
#define HOMING_OFFS_MAX				20
#define HOMING_OFFS_DEF				5

// Параметры скорости (мм/сек)
#define SPEED_MIN					1
#define SPEED_MAX					100
#define SPEED_DEF					5

// Температура (С х10)
#define TRM_TEMP_MIN				0		// in C x10
#define TRM_TEMP_MAX				2000	// in C x10
#define TRM_TEMP_DEF				0		// in C x10

// Давление (мбар)
#define PRESSURE_OK_DEF				5000

// Идентификатор адаптера
#define ADAPTER_CLAMP_HEIGHT_MIN	1
#define ADAPTER_CLAMP_HEIGHT_MAX	POS_MAX
#define ADAPTER_CLAMP_HEIGHT_DEF	50

#define ADAPTER_MISMATCH_NONE		0
#define ADAPTER_MISMATCH_CODE		1
#define ADAPTER_MISMATCH_CURRENT	2
#define ADAPTER_MISMATCH_VOLTAGE	3
#define ADAPTER_MISMATCH_HEIGHT		4

// Variables
//
extern const TableItemConstraint Constraint[];

#endif // __CONSTRAINTS_H
