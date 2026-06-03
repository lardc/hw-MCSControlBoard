#ifndef __CONSTRAINTS_H
#define __CONSTRAINTS_H

// Include
#include "stdinc.h"
#include "DataTable.h"
#include "Global.h"

//Definitions
//
#define ADPTR_REF_MIN				1		// в мВ
#define ADPTR_REF_MAX				3000	// в мВ

#define ADPTR_MCDA_REF_DEF			2400	// в мВ
#define ADPTR_MIAA_REF_DEF			2035	// в мВ
#define ADPTR_MIDA_REF_DEF			2750	// в мВ
#define ADPTR_MIFA_REF_DEF			29		// в мВ
#define ADPTR_MIHA_REF_DEF			1140	// в мВ
#define ADPTR_MIHM_REF_DEF			70		// в мВ
#define ADPTR_MIHV_REF_DEF			137		// в мВ
#define ADPTR_MISM_REF_DEF			286		// в мВ
#define ADPTR_MISM2_REF_DEF			567		// в мВ
#define ADPTR_MISV_REF_DEF			440		// в мВ
#define ADPTR_MIXM_REF_DEF			810		// в мВ
#define ADPTR_MIXV_REF_DEF			1374	// в мВ

#define PRESSURE_OFFSET_MIN			0
#define PRESSURE_OFFSET_MAX			4095
#define PRESSURE_OFFSET_DEF			0

#define PRESSURE_K_MIN				0
#define PRESSURE_K_MAX				10
#define PRESSURE_K_DEF				1

#define PRESSURE_P2_MIN				0
#define PRESSURE_P2_MAX				10
#define PRESSURE_P2_DEF				0

#define PRESSURE_P1_MIN				0
#define PRESSURE_P1_MAX				10
#define PRESSURE_P1_DEF				1

#define PRESSURE_P0_MIN				0
#define PRESSURE_P0_MAX				10
#define PRESSURE_P0_DEF				0

#define SET_PRESSURE_VALUE_MIN		1		// в Бар
#define SET_PRESSURE_VALUE_MAX		8		// в Бар
#define SET_PRESSURE_VALUE_DEF		5		// в Бар

#define PRESS_COUNTER_MAX			10
#define	PRESS_COUNTER_DEF			3
//
#define NO							0
#define YES							1
//
#define TOP_ADAPTER					0
#define BOT_ADAPTER					1

#define AVG_SAMPLES_MIN				1
#define AVG_SAMPLES_MAX				1000
#define AVG_SAMPLES_DEF				100

#define LABEL_ABS_ERR_MIN			1
#define LABEL_ABS_ERR_MAX			1000
#define LABEL_ABS_ERR_DEF			50

#define LABEL_REL_ERR_MIN			0.001f
#define LABEL_REL_ERR_MAX			1.0f
#define LABEL_REL_ERR_DEF			0.15f

// Types
typedef struct __TableItemConstraint
{
	float Min;
	float Max;
	float Default;
} TableItemConstraint;

// Variables
extern const TableItemConstraint Constraint[];

#endif // __CONSTRAINTS_H
