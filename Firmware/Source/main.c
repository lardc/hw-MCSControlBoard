#include "Controller.h"
#include "InitConfig.h"
#include "SysConfig.h"

int main()
{
	__disable_irq();
	SCB->VTOR = (uint32_t)BOOT_LOADER_MAIN_PR_ADDR;
	__enable_irq();
	
	// Настройка системной частоты тактирования
	INITCFG_ConfigSystemClock();
	
	// Настройка портов
	INITCFG_ConfigGPIO();

	// Настройка OneWire
	INITCFG_ConfigOneWire();

	// USART3 — RS232 SCCI , USART2 — TRM
	INITCFG_ConfigUART();
	INITCFG_ConfigTRMUART();

	// Настройка CAN и SPI
	INITCFG_ConfigCAN();
	INITCFG_ConfigSPI();
	INITCFG_ConfigRS485();
	
	// Настройка системного счетчика
	INITCFG_ConfigTimer7();
	INITCFG_ConfigTimer1();

	// Настройка сторожевого таймера
	INITCFG_ConfigWatchDog();

	// Настройка таймера и фонового АЦП давления
	INITCFG_ConfigTimer15();
	INITCFG_ConfigADC();
	INITCFG_ConfigDMA();

	// Инициализация логики контроллера
	CONTROL_Init();

	// Фоновый цикл
	while(TRUE)
		CONTROL_Idle();
	
	return 0;
}
