// ----------------------------------------
// Device object dictionary
// ----------------------------------------

#ifndef __DEV_OBJ_DIC_H
#define __DEV_OBJ_DIC_H


// ACTIONS
//
#define ACT_CLR_FAULT						3	// Clear fault
#define ACT_CLR_WARNING						4	// Clear warning
#define ACT_CLR_HALT						5	// Clear halt state

#define ACT_ADAPTER_WRITE_ID				10	//	Запись в индификатор
#define ACT_ADAPTER_READ_ID					11	//	Чтение с индификатора

#define ACT_HOMING							100	// Start homing
#define ACT_GOTO_POSITION					101 // Go to manually configured position
#define ACT_START_CLAMPING					102 // Star clamping
// 103
#define ACT_RELEASE_CLAMPING				104 // Perform unclamp
#define ACT_HALT							105 // Abort operation
// 106 - 107
#define ACT_SET_TEMPERATURE					108	// Set temperature
#define ACT_RELEASE_ADAPTER					109	// Release adapter for changing
#define ACT_HOLD_ADAPTER					110	// Hold adapter

#define ACT_DBG_READ_EXT_TEMP				114 // Read actual temperature value from Ext TRM
#define ACT_DBG_READ_TRM_TEMP				115	// Read temperature value from TRM
#define ACT_DBG_READ_TRM_POWER				116	// Read TRM output power
#define ACT_DBG_TRM_START					117	// Start TRM operation
#define ACT_DBG_TRM_STOP					118	// Stop TRM operation
#define	ACT_DBG_CONNECT_CONTROL				119	// Connect control circuit to device
#define	ACT_DBG_DISCONNECT_CONTROL			120	// Disconnect control circuit from device
#define ACT_DBG_MOTOR_START					121	// Запуск отладочного вращения моторов
#define ACT_DBG_MOTOR_STOP					122	// Остановка отладочного вращения моторов

#define ACT_SAVE_TO_ROM						200	// Save parameters to EEPROM module
#define ACT_RESTORE_FROM_ROM				201	// Restore parameters from EEPROM module
#define ACT_RESET_TO_DEFAULT				202	// Reset parameters to default values (only in controller memory)
#define ACT_LOCK_NV_AREA					203	// Lock modifications of parameters area
#define ACT_UNLOCK_NV_AREA					204	// Unlock modifications of parameters area (password-protected)

#define ACT_BOOT_LOADER_REQUEST				320	// Request reboot to bootloader

#define ACT_FLASH_DIAG_INIT_READ			331	// Инициализировать начало считывания отладочной информации
#define ACT_FLASH_DIAG_SAVE					332	// Сохранение блока отладочной информации во флэш
#define ACT_FLASH_DIAG_ERASE				333	// Стирание области отладочной информации

#define ACT_FLASH_DIAG_TO_EP				340	// Выполнить чтение массива из памяти отладочной информации в EP

// REGISTERS
//
#define REG_CLAMP_HEIGHT_CASE_A2			0	// Высота подъёма столика для корпуса А2 (мм)
#define REG_CLAMP_HEIGHT_CASE_B0			1	// Высота подъёма столика для корпуса B0 (мм)
#define REG_CLAMP_HEIGHT_CASE_C1			2	// Высота подъёма столика для корпуса C1 (мм)
#define REG_CLAMP_HEIGHT_CASE_D0			3	// Высота подъёма столика для корпуса D0 (мм)
#define REG_CLAMP_HEIGHT_CASE_E0			4	// Высота подъёма столика для корпуса E0 (мм)
#define REG_CLAMP_HEIGHT_CASE_F1			5	// Высота подъёма столика для корпуса F1 (мм)
#define REG_CLAMP_HEIGHT_CASE_ADAP  		6   // Высота подъёма столика для калибровочного адаптера (мм)
#define REG_CLAMP_HEIGHT_CASE_E2M			7 	// Высота подъёма столика для корпуса E2M (мм)

#define REG_CASE_THYRISTOR					9	// Зажимное используется для тиристоров
#define REG_POS_SPEED_MIN					10	// Минимальная скорость перемещения при позиционировании (мм/сек)
#define REG_POS_SPEED_LOW					11	// Пониженная скорость перемещения при позиционировании (мм/сек)
#define REG_POS_SPEED_MAX					12	// Максимальная скорость перемещения при позиционировании (мм/сек)
#define REG_SLOW_DOWN_DIST					13	// Расстояние от таргетной точки для начала замедления (мм)
#define REG_HOMING_SPEED					14	// Скорость хоуминга (мм/сек)
#define REG_HOMING_OFFSET					15	// Оффсет хоуминга (мм)
#define REG_CLAMP_SPEED_MIN					16	// Минимальная скорость перемещения при позиционировании зажатия (мм/сек)
#define REG_CLAMP_SPEED_LOW					17	// Пониженная скорость перемещения при позиционировании зажатия (мм/сек)
#define REG_CLAMP_SPEED_MAX					18	// Максимальная скорость перемещения при позиционировании зажатия (мм/сек)
#define REG_SM_TOGGLE_ACCELERATION			19	// Наклон ускорения

#define REG_USE_HEATING						20	// Включение/выключение обработки команд системы нагрева
#define REG_USE_SAFETY_SENSOR				21	// Включение/выключение обработки датчика безопасности
#define REG_USE_TOOLING_SENSOR				22	// Включение/выключение обработки датчика оснастки
//
#define REG_PRESSURE_OFFSET 				30	// Смещение давления
#define REG_PRESSURE_K						31  // Линейный коэффициент давления x1000
#define REG_PRESSURE_OK						32	// Корректное давление системы бар x1000
//
#define REG_CLAMP_HEIGHT_CASE_MIAA			40	// Высота подъёма столика для корпуса MIAA (мм)
#define REG_CLAMP_HEIGHT_CASE_MIDA			41	// Высота подъёма столика для корпуса MIDA (мм)
#define REG_CLAMP_HEIGHT_CASE_MIFA			42	// Высота подъёма столика для корпуса MIFA (мм)
#define REG_CLAMP_HEIGHT_CASE_MIHA			43	// Высота подъёма столика для корпуса MIHA (мм)
#define REG_CLAMP_HEIGHT_CASE_MIHM			44	// Высота подъёма столика для корпуса MIHM (мм)
#define REG_CLAMP_HEIGHT_CASE_MIHV			45	// Высота подъёма столика для корпуса MIHV (мм)
#define REG_CLAMP_HEIGHT_CASE_MISM			46	// Высота подъёма столика для корпуса MISM (мм)
#define REG_CLAMP_HEIGHT_CASE_MISM2_CH		47	// Высота подъёма столика для корпуса MISM2-CH (мм)
#define REG_CLAMP_HEIGHT_CASE_MISM2_SS_SD	48	// Высота подъёма столика для корпуса MISM2-SS-SD (мм)
#define REG_CLAMP_HEIGHT_CASE_MISV			49	// Высота подъёма столика для корпуса MISV (мм)
#define REG_CLAMP_HEIGHT_CASE_MIXM			50	// Высота подъёма столика для корпуса MIXM (мм)
#define REG_CLAMP_HEIGHT_CASE_MIXV			51	// Высота подъёма столика для корпуса MIXV (мм)
#define REG_CLAMP_HEIGHT_CASE_MADAP			52	// Высота подъёма столика для калибровочного адаптера IGBT (мм)
//
#define REG_SERT_UPPER_ADAP_ID				53	// ID верхнего адаптера аттестации (взять из DevType в Controller.h)
// ----------------------------------------
//
#define REG_CUSTOM_POS						64	// Mannually configured position (in mm)

#define REG_DEV_CASE						71	// Код корпуса прибора/индификатора
#define REG_TEMP_SETPOINT					72	// Уставка температуры (С х10)
//
#define REG_ADAPTER_ID						73	// CS adapter ID
//
#define REG_DBG_TRM_ADDRESS					84	// Адрес ТРМ по RS485 для отладки
//
#define REG_DBG_STEP_DIV					90	// Коэффициент деления шагов в отладочном режиме
#define REG_DBG_STEPS_MAX					91	// Количество шагов для поворота в отладочном режиме
//
#define REG_PWD_1							91	// Unlock password location 1
#define REG_PWD_2							92	// Unlock password location 2
#define REG_PWD_3							93	// Unlock password location 3
#define REG_PWD_4							94	// Unlock password location 4
//
#define REG_SP__2							95
//
// ----------------------------------------
//
#define REG_DEV_STATE						96	// Device state
#define REG_FAULT_REASON					97	// Fault reason in the case DeviceState -> FAULT
#define REG_DISABLE_REASON					98	// Fault reason in the case DeviceState -> DISABLED
#define REG_WARNING							99	// Warning if present
#define REG_PROBLEM							100	// Problem if present
#define REG_TEMP_CH1						101	// Sampled temperature on channel 1

#define REG_TRM_DATA						103	// Data read from TRM
#define REG_TRM_ERROR						104	// TRM error value
#define REG_PRESSURE						105	// Давление в пневмомагистрали
#define REG_SAFETY_SENSOR					106	// Состояние датчика безопасности
#define REG_HOMING_SENSOR					107	// Состояние датчика хоуминга
#define REG_BUS_TOOLING_SENSOR				108	// Состояние датчика фиксации шины оснастки
#define REG_ADAPTER_TOOLING_SENSOR			109	// Состояние датчика фиксации адаптера оснастки

#define REG_DEV_SUBSTATE					110	// Device substate

#define REG_CANA_BUSOFF_COUNTER				120 // Counter of bus-off states
#define REG_CANA_STATUS_REG					121	// CAN status register (32 bit)
#define REG_CANA_STATUS_REG_32				122
#define REG_CANA_DIAG_TEC					123	// CAN TEC
#define REG_CANA_DIAG_REC					124	// CAN REC
//
#define REG_SP__3							159
//
#define REG_DBG								170
// ----------------------------------------
//
#define REG_FWINFO_SLAVE_NID				256	// Device CAN slave node ID
#define REG_FWINFO_MASTER_NID				257	// Device CAN master node ID (if presented)
// 258 - 259
#define REG_FWINFO_STR_LEN					260	// Length of the information string record
#define REG_FWINFO_STR_BEGIN				261	// Begining of the information string record

// FAULT CODES
//
#define FAULT_NONE							0	// No fault
//
#define FAULT_TRM							4	// TRM communication fault
//
#define FAULT_PRESSURE						5	// Давление ниже нормы
#define FAULT_BUS_SEN						7	// Ошибка сигнала с датчика поджатия шин
#define FAULT_ADAPTER_SEN					8	// Ошибка сигнала с датчика поджатия адаптера
#define FAULT_IGBT_ADAPTER_CONN				9	// No connection to IGBT adapter


// PROBLEM CODES
//
#define PROBLEM_NONE						0	// No problem
#define PROBLEM_TOP_ADAPTER_MISMATCHED		3	// Установленный верхний адаптер не совпадает с заданым адаптером для проведения измерений

// DISABLE CODES
//
#define DISABLE_NONE						0	// No fault
#define DISABLE_BAD_CLOCK					1001	// Problem with main oscillator

// WARNING CODES
//
#define WARNING_NONE						0	// No warning
#define WARNING_WATCHDOG_RESET				1001	// System has been reseted by WD

// USER ERROR CODES
//
#define ERR_NONE							0	// No error
#define ERR_CONFIGURATION_LOCKED			1	// Device is locked for writing
#define ERR_OPERATION_BLOCKED				2	// Operation can't be done due to current device state
#define ERR_DEVICE_NOT_READY				3	// Device isn't ready to switch state
#define ERR_WRONG_PWD						4	// Wrong password - unlock failed
//
#define ERR_TRM_COMM_ERR					7	// Communication with TRM failed

// ENDPOINTS
//
#define EP32_ExtInfoData					20	// External information from flash

#endif // __DEV_OBJ_DIC_H
