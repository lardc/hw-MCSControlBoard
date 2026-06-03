// ----------------------------------------
// Global definitions
// ----------------------------------------

// Header
#include "Constraints.h"

// Include
#include "Controller.h"
#include "DeviceObjectDictionary.h"

#define NO		0	// equal to FALSE
#define YES		1	// equal to TRUE

// Constants
//
const TableItemConstraint NVConstraint[DATA_TABLE_NV_SIZE] =
                                      {
											   {CASE_MIN, CASE_MAX, CASE_A2_DEF},										// 0
											   {CASE_MIN, CASE_MAX, CASE_B0_DEF},										// 1
											   {CASE_MIN, CASE_MAX, CASE_C1_DEF},										// 2
											   {CASE_MIN, CASE_MAX, CASE_D_DEF},										// 3
											   {CASE_MIN, CASE_MAX, CASE_E_DEF},										// 4
											   {CASE_MIN, CASE_MAX, CASE_F_DEF},										// 5
											   {CASE_MIN, CASE_MAX, CASE_ADAP_DEF},										// 6
											   {0, 0, 0},																// 7
											   {0, 0, 0},																// 8
											   {NO, YES, YES},															// 9
											   {SPEED_MIN, SPEED_MAX, SPEED_DEF},										// 10
											   {SPEED_MIN, SPEED_MAX, SPEED_DEF},										// 11
											   {SPEED_MIN, SPEED_MAX, SPEED_DEF},										// 12
											   {SLOW_DOWN_MIN, SLOW_DOWN_MAX, SLOW_DOWN_DEF},							// 13
											   {SPEED_MIN, SPEED_MAX, SPEED_DEF},										// 14
											   {HOMING_OFFS_MIN, HOMING_OFFS_MAX, HOMING_OFFS_DEF},						// 15
											   {SPEED_MIN, SPEED_MAX, SPEED_DEF},										// 16
											   {SPEED_MIN, SPEED_MAX, SPEED_DEF},										// 17
											   {SPEED_MIN, SPEED_MAX, SPEED_DEF},										// 18
											   {1, 100, 50},															// 19
											   {NO, YES, YES},															// 20
											   {NO, YES, YES},															// 21
											   {NO, YES, YES},															// 22
											   {NO, YES, YES},															// 23
											   {0, 0, 0},																// 24
											   {0, 0, 0},																// 25
											   {0, 0, 0},																// 26
											   {0, 0, 0},																// 27
											   {0, 0, 0},																// 28
											   {0, 0, 0},																// 29
											   {0, INT16U_MAX, 0},														// 30
											   {0, INT16U_MAX, 0},														// 31
											   {0, INT16U_MAX, 0},														// 32
											   {0, 0, 0},																// 33
											   {0, 0, 0},																// 34
											   {0, 0, 0},																// 35
											   {0, 0, 0},																// 36
											   {0, 0, 0},																// 37
											   {0, 0, 0},																// 38
											   {0, 0, 0},																// 39
											   {CASE_MIN, CASE_MAX, CASE_MIAA_DEF},										// 40
											   {CASE_MIN, CASE_MAX, CASE_MIDA_DEF},										// 41
											   {CASE_MIN, CASE_MAX, CASE_MIFA_DEF},										// 42
											   {CASE_MIN, CASE_MAX, CASE_MIHA_DEF},										// 43
											   {CASE_MIN, CASE_MAX, CASE_MIHM_DEF},										// 44
											   {CASE_MIN, CASE_MAX, CASE_MIHV_DEF},										// 45
											   {CASE_MIN, CASE_MAX, CASE_MISM_DEF},										// 46
											   {CASE_MIN, CASE_MAX, CASE_MISM2_CH_DEF},									// 47
											   {CASE_MIN, CASE_MAX, CASE_MISM2_SS_SD_DEF},								// 48
											   {CASE_MIN, CASE_MAX, CASE_MISV_DEF},										// 49
											   {CASE_MIN, CASE_MAX, CASE_MIXM_DEF},										// 50
											   {CASE_MIN, CASE_MAX, CASE_MIXV_DEF},										// 51
											   {CASE_MIN, CASE_MAX, CASE_MADAP_DEF},									// 52
											   {SC_Type_A2, SC_Type_MIADAP, SC_Type_MIADAP},							// 53
											   {0, 0, 0},																// 54
											   {0, 0, 0},																// 55
											   {0, 0, 0},																// 56
											   {0, 0, 0},																// 57
											   {0, 0, 0},																// 58
											   {0, 0, 0},																// 59
											   {0, 0, 0},																// 60
											   {0, 0, 0},																// 61
											   {0, 0, 0},																// 62
                                    		   {INT16U_MAX, 0, 0}														// 63
                                      };

const TableItemConstraint VConstraint[DATA_TABLE_WP_START - DATA_TABLE_WR_START] =
                                      {
											   {0, POS_MAX, 0},															// 64
											   {0, 0, 0},																// 65
											   {0, 0, 0},																// 66
											   {0, 0, 0},																// 67
											   {0, 0, 0},																// 68
											   {0, 0, 0},																// 69
											   {0, INT16U_MAX, 0},														// 70
											   {SC_Type_A2, SC_Type_MIADAP, SC_Type_MIHA},								// 71
											   {TRM_TEMP_MIN, TRM_TEMP_MAX, TRM_TEMP_DEF},								// 72
											   {0, INT16U_MAX, 0},														// 73
											   {0, 0, 0},																// 74
											   {0, 0, 0},																// 75
											   {0, 0, 0},																// 76
											   {0, 0, 0},																// 77
											   {0, 0, 0},																// 78
											   {0, 0, 0},																// 79
											   {0, 0, 0},																// 80
											   {0, 0, 0},																// 81
											   {0, 0, 0},																// 82
											   {0, 0, 0},																// 83
											   {0, 255, 0},																// 84
											   {0, 0, 0},																// 85
											   {0, 0, 0},																// 86
											   {0, 0, 0},																// 87
											   {0, 0, 0},																// 88
											   {0, 0, 0},																// 89
											   {0, INT16U_MAX, 0},														// 90
											   {0, INT16U_MAX, 0},														// 91
											   {0, 0, 0},																// 92
											   {0, 0, 0},																// 93
											   {0, 0, 0},																// 94
                                    		   {INT16U_MAX, 0, 0}														// 95
                                      };

// No more
