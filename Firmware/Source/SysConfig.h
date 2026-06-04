#ifndef __SYSCONFIG_H
#define __SYSCONFIG_H

#include "BCCIxParams.h"

// Flash loader options
#define BOOT_LOADER_VARIABLE			(*((volatile uint32_t *)0x20000000))
#define BOOT_LOADER_REQUEST				0x12345678
#define BOOT_LOADER_MAIN_PR_ADDR		0x08008000
//-----------------------------------------------

// System clock
#define SYSCLK							70000000	// Тактовая частота системной шины процессора
#define QUARTZ_FREQUENCY				20000000	// Частота кварца
// ----------------------------------------------

// USART
#define USART_BAUDRATE					115200		// Скорость USART
#define USART_FIFOlen					32			// Длина FIFO USART
// ----------------------------------------------

// Timers
#define TIMER7_uS						1000		// в мкс
// ----------------------------------------------

// CAN
#define CAN_BAUDRATE					1000000		// Битрейт CAN
// ----------------------------------------------

// ADC
#define ADC_PRESSURE_CHANNEL			1
// ----------------------------------------------

// TRM
#define TRM_CH1_ADDR					0
#define TRM_TEMP_THR					500
#define TRM_TIMEOUT_TICKS				100
#define FAN_TIMEOUT						30000
// ----------------------------------------------

// SPI mux (stub)
#define SPIMUX_EPROM					4
// ----------------------------------------------

#endif // __SYSCONFIG_H
