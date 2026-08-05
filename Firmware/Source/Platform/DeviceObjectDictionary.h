// ----------------------------------------
// Device object dictionary
// ----------------------------------------

#ifndef __DEV_OBJ_DIC_H
#define __DEV_OBJ_DIC_H


// Команды
//
#define ACT_CLR_FAULT						3	// Clear fault
#define ACT_CLR_WARNING						4	// Clear warning
#define ACT_CLR_HALT						5	// Clear halt state

#define ACT_ADAPTER_WRITE_ID				10	//	Запись в индификатор
#define ACT_ADAPTER_READ_ID					11	//	Чтение с индификатора

#define ACT_DBG_MEAS_PRESSURE				20	// Измерение и запись в REG_DBG значения напряжения на входе АЦП датчика давления
#define ACT_DBG_SET_OUTPUT					21	// Установить напряжение 24 В на универсальном выходе (разъемы P2-P9) равным в регистре REG_DBG (1-8)
#define ACT_DBG_MEAS_INPUT					22	// Считывание состояния входов (разъемы P2-P9) и запись состояния в регистр REG_DBG (0-255)
#define ACT_DBG_STPM						23	// На пинах STPM_DIR, STPM_STEP и STPM_EN выставить высокий уровень на 100 мс
#define ACT_DBG_DQ_PWR						24	// На пине DQ_PWR отключить подтяжку на 100 мс
#define ACT_DBG_DQ_CTRL						25	// На пине DQ_CTRL выставить высокий уро-вень на 100 мс
#define ACT_DBG_DQ_IN						26	// Считывание состояния пина DQ_IN и запись состояния в регистр REG_DBG
#define ACT_DBG_HOMING 						27	// Считывание состояния пина HOMING и за-пись в регистр REG_DBG (0 – нет напряжения, 1 – 3,3 В на пине)
#define ACT_DBG_SFT							28	// Считывание состояния пина S3 и запись в регистр REG_DBG (0 – нет напря-жения, 1 – 4,3 В на пине)
#define ACT_DBG_OPTICAL						29	// Считывание состояния пина S5 и запись в регистр REG_DBG (0 – нет напря-жения, 1 – 4,3 В на пине)
#define ACT_DBG_TRM_READ					30	// Чтение FLOAT32 из holding-регистра TRM10; REG_DBG_TRM_ADDRESS — slave, REG_DBG — адрес регистра
#define ACT_DBG_TRM_WRITE					31	// Запись FLOAT32 в holding-регистр TRM10; REG_DBG_TRM_ADDRESS — slave, REG_DBG — адрес, REG_DBG2 — значение

#define ACT_HOMING							100	// Start homing
// 101
#define ACT_START_CLAMPING					102 // Star clamping
// 103
#define ACT_RELEASE_CLAMPING				104 // Perform unclamp
#define ACT_HALT							105 // Abort operation
#define ACT_UPDATE_ADAPTER_MATCH			106 // Обновить регистр сооствествия адаптера
// 107
#define ACT_SET_TEMPERATURE					108	// Set temperature
#define ACT_RELEASE_ADAPTER					109	// Release adapter for changing
#define ACT_HOLD_ADAPTER					110	// Hold adapter

#define ACT_DBG_READ_EXT_TEMP				114 // Read actual temperature value from Ext TRM
#define ACT_DBG_READ_TRM_TEMP				115	// Read temperature value from TRM
#define ACT_DBG_READ_TRM_POWER				116	// Read TRM output power
#define ACT_DBG_TRM_START					117	// Start TRM operation
#define ACT_DBG_TRM_STOP					118	// Stop TRM operation
// 119-120
#define ACT_DBG_MOTOR_START					121	// Запуск отладочного вращения моторов
#define ACT_DBG_MOTOR_STOP					122	// Остановка отладочного вращения моторов

#define ACT_DBG_DS18_READ					123 // Считать 2 байта DS18 в REG_DBG (десят.)
#define ACT_DBG_DS18_WRITE					124	// Записать 2 байта DS18 из REG_DBG (десят.)
#define ACT_DBG_ONEWIRE_SEARCH				125 // Поиск DS18B20 и DS2431 → REG_DBG: ds18 + ds2431×1000 (напр. 2 и 3 → 3002)
#define ACT_DBG_DS18_READ_TEMP				126 // Температура DS18 в REG_DBG, 0.1°C (десят.)
#define ACT_DBG_DS2431_ERASE				127 // Стирание области данных DS2431 (0x00..0x7F); REG_DBG — индекс устройства (0, 1, …)
#define ACT_DBG_DS2431_READ					128 // Чтение 2 байт с адреса 0; REG_DBG: вход — номер, выход — данные (десят.)
#define ACT_DBG_DS2431_WRITE				129 // Запись 2 байт с адреса 0; REG_DBG — данные (десят.), устройство — из 127/128

#define ACT_DBG_LABEL_FIND					130 // Поиск меток на чипах памяти. В отладочный регистр сохраняется 0 если меток нет, 1 если есть
#define ACT_DBG_LABEL_SHOW_AMOUNT			131 // Считывание количества меток из чипов памяти в отладочный регистр
#define ACT_DBG_LABEL_ERASE					132 // Удаление меток из чипов памяти
#define ACT_DBG_LABEL_WRITE					133 // Запись меток на чип памяти. DBG - тип данных, DBG2 - сами данные.
#define ACT_DBG_LABEL_READ_DATA				134 // Считывание меток в отладочные регистры.При считывании DBG - индекс метки,
												// после считывания -  DBG - тип данных, DBG2 - сами данные.

#define ACT_SAVE_TO_ROM						200	// Save parameters to EEPROM module
#define ACT_RESTORE_FROM_ROM				201	// Restore parameters from EEPROM module
#define ACT_RESET_TO_DEFAULT				202	// Reset parameters to default values (only in controller memory)

#define ACT_BOOT_LOADER_REQUEST				320	// Request reboot to bootloader

#define ACT_FLASH_DIAG_INIT_READ			331	// Инициализировать начало считывания отладочной информации
#define ACT_FLASH_DIAG_SAVE					332	// Сохранение блока отладочной информации во флэш
#define ACT_FLASH_DIAG_ERASE				333	// Стирание области отладочной информации

#define ACT_FLASH_DIAG_TO_EP				340	// Выполнить чтение массива из памяти отладочной информации в EP

// Регистры
// Сохраняемые регистры
// 0-9
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
// 21-29
#define REG_PRESSURE_OFFSET 				30	// Смещение давления
#define REG_PRESSURE_K						31  // Линейный коэффициент давления x1000
#define REG_PRESSURE_OK						32	// Корректное давление системы бар x1000
// 33-63

// Несохраняемые регистры чтения-записи
//64-70
#define REG_DEV_CASE						71	// Код корпуса прибора (задание Master для сверки)
#define REG_TEMP_SETPOINT					72	// Уставка температуры (С х10)
//
#define REG_ADAPTER_ID						73	// Код адаптера в идентификаторе 1-Wire
#define REG_ADAPTER_SERIAL					74	// Серийный номер адаптера
#define REG_ADAPTER_CLAMP_HEIGHT			75	// Высота зажатия из идентификатора (мм)
#define REG_ADAPTER_MAX_CURRENT				76	// Макс. ток из идентификатора
#define REG_ADAPTER_MAX_VOLTAGE				77	// Макс. напряжение из идентификатора
// 78-79
#define REG_TEST_CURRENT					80	// Заданный Master предел тока для сверки
#define REG_TEST_VOLTAGE					81	// Заданный Master предел напряжения для сверки
// 82-83
#define REG_DBG_TRM_ADDRESS					84	// Адрес ТРМ по RS485 для отладки
// 85-89
#define REG_DBG_STEP_DIV					90	// Коэффициент деления шагов в отладочном режиме
#define REG_DBG_STEPS_MAX					91	// Количество шагов для поворота в отладочном режиме
//
#define REG_DBG								92	// Отладочный регистр
#define REG_DBG2							93	// Отладочный регистр
// 94-95

// Регистры только чтение
#define REG_DEV_STATE						96	// Device state
#define REG_FAULT_REASON					97	// Fault reason in the case DeviceState -> FAULT
#define REG_DISABLE_REASON					98	// Fault reason in the case DeviceState -> DISABLED
#define REG_WARNING							99	// Warning if present
#define REG_PROBLEM							100	// Problem if present
#define REG_OP_RESULT						101	// Регистр результата операции
//
#define REG_TEMP_CH1						102	// Sampled temperature on channel 1
#define REG_TRM_DATA						103	// Data read from TRM
#define REG_TRM_ERROR						104	// TRM error value
#define REG_PRESSURE						105	// Давление в пневмомагистрали
#define REG_SENSOR_S2						106	// Датчик столика S2 (PA10)
#define REG_HOMING_SENSOR					107	// Состояние датчика хоуминга
#define REG_BUS_TOOLING_SENSOR				108	// SPI: датчик подключения силовых шин
#define REG_ADAPTER_TOOLING_SENSOR			109	// SPI: датчик подключения адаптера
//
#define REG_DEV_SUBSTATE					110	// Device substate
#define REG_SELFTEST_RESULT					111	// Маска ошибок самодиагностики ОШ (бит0=ОШ1, бит1=ОШ2)
//
#define REG_ADAPTER_MATCH					112	// Результат сверки идентификатора
#define REG_ADAPTER_MISMATCH				113 // Показатель того, что разошлось при сверке (1 - код, 2 - ток, 3 - напряжение, 4 -высота)
//
#define REG_SPI_IN_STATE					114	// Сырой байт регистра входа 2SPI
#define REG_SENSOR_S3						115	// Датчик безопасности S3 (PA9)
#define REG_SENSOR_S5						116	// Датчик безопасности S5 (PA12)
// 117-119
#define REG_CANA_BUSOFF_COUNTER				120 // Counter of bus-off states
#define REG_CANA_STATUS_REG					121	// CAN status register (32 bit)
#define REG_CANA_STATUS_REG_32				122
#define REG_CANA_DIAG_TEC					123	// CAN TEC
#define REG_CANA_DIAG_REC					124	// CAN REC
// 125-158
#define REG_SP__3							159
// 160-255
//
// ----------------------------------------
#define REG_FWINFO_SLAVE_NID				256	// Device CAN slave node ID
#define REG_FWINFO_MASTER_NID				257	// Device CAN master node ID (if presented)
// 258-259
#define REG_FWINFO_STR_LEN					260	// Length of the information string record
#define REG_FWINFO_STR_BEGIN				261	// Begining of the information string record

// Operation results
#define OPRESULT_NONE						0	// No information or not finished
#define OPRESULT_OK							1	// Operation was successful
#define OPRESULT_FAIL						2	// Operation failed

//  Fault codes
#define DF_NONE								0	// No fault
// 1-3
#define DF_TRM								4	// TRM communication fault
#define DF_PRESSURE							5	// Давление ниже нормы
// 6-9
#define DF_SELFTEST							10	// Ошибка самодиагностики оптронов
#define DF_SPI_TIMEOUT						11	// Таймаут ожидания SPI-входа

// Problem
#define PROBLEM_NONE						0	// No problem
#define PROBLEM_MISSING_LABEL				1	// Недостаточно данных в метке
#define PROBLEM_ADAPTER_MISMATCH			2	// Несовпадение идентификатора адаптера
#define PROBLEM_NO_HOLD_OR_MISMATCH			3	// Не выполнено зажатие или было несовпадение адаптера
#define PROBLEM_OW_ERROR_LINE				20	// Проблема подключения на линии OW
#define PROBLEM_OW_NO_DEVICE				21	// Устройство не найдено / неверный индекс
#define PROBLEM_OW_VERIFY					22	// Ошибка verify / scratchpad / CRC / copy
#define PROBLEM_OW_PARAM					23	// Неверные параметры запроса
#define PROBLEM_OW_DS18						24	// Проблема OneWire на DS18

// Disable
#define DISABLE_NONE						0	// No fault

//  Warning
#define WARNING_NONE						0	// No warning

//  User Errors
#define ERR_NONE							0	// No error
#define ERR_CONFIGURATION_LOCKED			1	// Device is locked for writing
#define ERR_OPERATION_BLOCKED				2	// Operation can't be done due to current device state
#define ERR_DEVICE_NOT_READY				3	// Device isn't ready to switch state
#define ERR_TRM_COMM_ERR					7	// Communication with TRM failed

// ENDPOINTS
//
// TODO

#endif // __DEV_OBJ_DIC_H
