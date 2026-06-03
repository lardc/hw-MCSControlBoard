// ----------------------------------------
// Global definitions
// ----------------------------------------

// Header
#include "Constraints.h"
#include "Logic.h"

// Constants
//
const TableItemConstraint Constraint[DATA_TABLE_WP_START] = {
		{0, 0, 0},																	// 0
		{0, 0, 0},																	// 1
		{0, 0, 0},																	// 2
		{0, 0, 0},																	// 3
		{0, 0, 0},																	// 4
		{0, 0, 0},																	// 5
		{PRESSURE_OFFSET_MIN, PRESSURE_OFFSET_MAX, PRESSURE_OFFSET_DEF},			// 6
		{PRESSURE_K_MIN, PRESSURE_K_MAX, PRESSURE_K_DEF},							// 7
		{PRESSURE_P2_MIN, PRESSURE_P2_MAX, PRESSURE_P2_DEF},						// 8
		{PRESSURE_P1_MIN, PRESSURE_P1_MAX, PRESSURE_P1_DEF},						// 9
		{PRESSURE_P0_MIN, PRESSURE_P0_MAX, PRESSURE_P0_DEF},						// 10
		{SET_PRESSURE_VALUE_MIN, SET_PRESSURE_VALUE_MAX, SET_PRESSURE_VALUE_DEF},	// 11
		{0, PRESS_COUNTER_MAX, PRESS_COUNTER_DEF},									// 12
		{AVG_SAMPLES_MIN, AVG_SAMPLES_MAX, AVG_SAMPLES_DEF},						// 13
		{LABEL_ABS_ERR_MIN, LABEL_ABS_ERR_MAX, LABEL_ABS_ERR_DEF},					// 14
		{LABEL_REL_ERR_MIN, LABEL_REL_ERR_MAX, LABEL_REL_ERR_DEF},					// 15
		{0, 0, 0},																	// 16
		{0, 0, 0},																	// 17
		{0, 0, 0},																	// 18
		{0, 0, 0},																	// 19
		{0, 0, 0},																	// 20
		{0, 0, 0},																	// 21
		{0, 0, 0},																	// 22
		{0, 0, 0},																	// 23
		{0, 0, 0},																	// 24
		{0, 0, 0},																	// 25
		{0, 0, 0},																	// 26
		{0, 0, 0},																	// 27
		{0, 0, 0},																	// 28
		{0, 0, 0},																	// 29
		{0, 0, 0},																	// 30
		{0, 0, 0},																	// 31
		{0, 0, 0},																	// 32
		{0, 0, 0},																	// 33
		{0, 0, 0},																	// 34
		{0, 0, 0},																	// 35
		{0, 0, 0},																	// 36
		{0, 0, 0},																	// 37
		{0, 0, 0},																	// 38
		{0, 0, 0},																	// 39
		{ADPTR_REF_MIN, ADPTR_REF_MAX, ADPTR_MIAA_REF_DEF},							// 40
		{ADPTR_REF_MIN, ADPTR_REF_MAX, ADPTR_MIDA_REF_DEF},							// 41
		{ADPTR_REF_MIN, ADPTR_REF_MAX, ADPTR_MIFA_REF_DEF},							// 42
		{ADPTR_REF_MIN, ADPTR_REF_MAX, ADPTR_MIHA_REF_DEF},							// 43
		{ADPTR_REF_MIN, ADPTR_REF_MAX, ADPTR_MIHM_REF_DEF},							// 44
		{ADPTR_REF_MIN, ADPTR_REF_MAX, ADPTR_MIHV_REF_DEF},							// 45
		{ADPTR_REF_MIN, ADPTR_REF_MAX, ADPTR_MISM_REF_DEF},							// 46
		{0, 0, 0},																	// 47
		{0, 0, 0},																	// 48
		{ADPTR_REF_MIN, ADPTR_REF_MAX, ADPTR_MISV_REF_DEF},							// 49
		{ADPTR_REF_MIN, ADPTR_REF_MAX, ADPTR_MIXM_REF_DEF},							// 50
		{ADPTR_REF_MIN, ADPTR_REF_MAX, ADPTR_MIXV_REF_DEF},							// 51
		{0, 0, 0},																	// 52
		{0, 0, 0},																	// 53
		{ADPTR_REF_MIN, ADPTR_REF_MAX, ADPTR_MISM2_REF_DEF},						// 54
		{ADPTR_REF_MIN, ADPTR_REF_MAX, ADPTR_MCDA_REF_DEF},							// 55
		{0, 0, 0},																	// 56
		{0, 0, 0},																	// 57
		{0, 0, 0},																	// 58
		{0, 0, 0},																	// 59
		{0, 0, 0},																	// 60
		{0, 0, 0},																	// 61
		{0, 0, 0},																	// 62
		{0, 0, 0},																	// 63
		{0, 0, 0},																	// 64
		{0, 0, 0},																	// 65
		{0, 0, 0},																	// 66
		{0, 0, 0},																	// 67
		{0, 0, 0},																	// 68
		{0, 0, 0},																	// 69
		{0, INT16U_MAX, 0},															// 70
		{DT_None, DT_MISM2, DT_None},												// 71
		{0, INT16U_MAX, 0},															// 72
		{0, 0, 0},																	// 73
		{0, 0, 0},																	// 74
		{0, 0, 0},																	// 75
		{0, 0, 0},																	// 76
		{0, 0, 0},																	// 77
		{0, 0, 0},																	// 78
		{0, 0, 0},																	// 79
		{0, 0, 0},																	// 80
		{0, 0, 0},																	// 81
		{0, 0, 0},																	// 82
		{0, 0, 0},																	// 83
		{0, 0, 0},																	// 84
		{0, 0, 0},																	// 85
		{0, 0, 0},																	// 86
		{0, 0, 0},																	// 87
		{0, 0, 0},																	// 88
		{0, 0, 0},																	// 89
		{0, 0, 0},																	// 90
		{0, 0, 0},																	// 91
		{0, 0, 0},																	// 92
		{0, 0, 0},																	// 93
		{0, 0, 0},																	// 94
		{0, 0, 0},																	// 95
};
