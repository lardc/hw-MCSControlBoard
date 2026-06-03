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
#include "StepperMotor.h"

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

#define CASE_MIN					1
#define CASE_MAX					POS_MAX
#define CASE_A2_DEF					122
#define CASE_B0_DEF					122
#define CASE_C1_DEF					122
#define CASE_D_DEF					93
#define CASE_E_DEF					82
#define CASE_F_DEF					144
#define CASE_ADAP_DEF				50
//
#define CASE_MIAA_DEF				180
#define CASE_MIDA_DEF				180
#define CASE_MIFA_DEF				180
#define CASE_MIHA_DEF				180
#define CASE_MIHM_DEF				180
#define CASE_MIHV_DEF				180
#define CASE_MISM_DEF				180
#define CASE_MISM2_CH_DEF			180
#define CASE_MISM2_SS_SD_DEF		180
#define CASE_MISV_DEF				180
#define CASE_MIXM_DEF				180
#define CASE_MIXV_DEF				180
#define CASE_MADAP_DEF				180

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

// Variables
//
extern const TableItemConstraint NVConstraint[DATA_TABLE_NV_SIZE];
extern const TableItemConstraint VConstraint[DATA_TABLE_WP_START - DATA_TABLE_WR_START];

#endif // __CONSTRAINTS_H
