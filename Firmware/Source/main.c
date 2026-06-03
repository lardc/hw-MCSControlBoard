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
	INITCFG_ConfigIO();

	// Настройка UART
	INITCFG_ConfigUART();
	
	// Настройка системного счетчика
	INITCFG_ConfigTimer7();

	// Настройка сторожевого таймера
	INITCFG_ConfigWatchDog();
	
	// Настройка АЦП
	INITCFG_ConfigADC();

	// Инициализация логики контроллера
	CONTROL_Init();

	// Фоновый цикл
	while(TRUE)
		CONTROL_Idle();
	
	return 0;
}
