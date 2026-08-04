// Header
#include "InitConfig.h"
// Includes
#include "Board.h"
#include "SysConfig.h"
#include "BCCIxParams.h"
#include "LowLevel.h"
#include "ZwSPI.h"
#include "ZwDMA.h"
#include "ZwUSART.h"
#include "DS18B20.h"
#include "DS2431.h"
#include "OneWire.h"
#include "Modbus.h"
#include "Controller.h"
#include "Timer3_Ch4PWM.h"

// Variables
Int16U INITCFG_PressureAdcBuffer[ADC_PRESSURE_BUF_SIZE];

// Functions
Boolean INITCFG_ConfigSystemClock()
{
	return RCC_PLL_HSE_Config(QUARTZ_FREQUENCY, PREDIV_4, PLL_14);
}
//------------------------------------------------

void INITCFG_ConfigGPIO()
{
	RCC_GPIO_Clk_EN(PORTA);
	RCC_GPIO_Clk_EN(PORTB);
	RCC_GPIO_Clk_EN(PORTC);

	GPIO_InitAnalog(GPIO_MEASURE_PRESS);

	GPIO_InitAltFunction(GPIO_ALT_UART2_RX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_UART2_TX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_UART3_RX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_UART3_TX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_CAN1_RX, AltFn_9);
	GPIO_InitAltFunction(GPIO_ALT_CAN1_TX, AltFn_9);
	GPIO_InitAltFunction(GPIO_ALT_SPI3_SCK, AltFn_6);
	GPIO_InitAltFunction(GPIO_ALT_SPI3_MISO, AltFn_6);
	GPIO_InitAltFunction(GPIO_ALT_SPI3_MOSI, AltFn_6);
	GPIO_InitAltFunction(GPIO_STPM_STEP, AltFn_2);

	GPIO_InitPushPullOutput(GPIO_LED);
	GPIO_InitPushPullOutput(GPIO_RS485_CTRL);
	GPIO_InitPushPullOutput(GPIO_STPM_DIR);
	GPIO_InitPushPullOutput(GPIO_STPM_EN);
	GPIO_InitPushPullOutput(GPIO_SPI_SS);
	GPIO_InitPushPullOutput(GPIO_TEST);

	GPIO_InitOpenDrainOutput(GPIO_SPI_LD, NoPull);
	GPIO_InitOpenDrainOutput(GPIO_SPI_OE, NoPull);

	GPIO_InitInput(GPIO_SEN_S1, NoPull);
	GPIO_InitInput(GPIO_SEN_S3, NoPull);
	GPIO_InitInput(GPIO_SEN_S2, NoPull);
	GPIO_InitInput(GPIO_SEN_S4, NoPull);
	GPIO_InitInput(GPIO_SEN_S5, NoPull);
	GPIO_InitInput(GPIO_HOMING, NoPull);

	GPIO_SetState(GPIO_LED, false);
	GPIO_SetState(GPIO_RS485_CTRL, false);
	GPIO_SetState(GPIO_STPM_DIR, false);
	GPIO_SetState(GPIO_STPM_EN, false);
	GPIO_SetState(GPIO_SPI_SS, false);
	GPIO_SetState(GPIO_TEST, false);
	GPIO_SetState(GPIO_SPI_LD, false);
	GPIO_SetState(GPIO_SPI_OE, true);
}
//------------------------------------------------

void INITCFG_ConfigOneWire()
{
	OneWireBus Config;
	Config.writePin =		GPIO_DQ_CTRL;
	Config.readPin =		GPIO_DQ_IN;
	Config.powerPin =		GPIO_DQ_PWR;
	Config.useSinglePin =	false;
	Config.hasPowerPin =	true;
	Config.invertWrite =	true;
	Config.invertPower =	true;
	OneWire_Init(Config);

	// Инициализация устройств
	DS18B20_Init(Config.hasPowerPin);
	DS2431_Init();
}
//------------------------------------------------

void INITCFG_ConfigModbus()
{
	// Общая инициализация шины
	Int16U TimeoutTicks = 100;

	Modbus_Init((ModbusFunc_SendByte)USART2_SendChar, (ModbusFunc_GetBytesToReceive)USART2_GetBytesToReceive,
			(ModbusFunc_ReceiveByte)USART2_ReceiveChar, LL_RS485_SetTxMode, USART_BAUDRATE, &CONTROL_TimeCounter, TimeoutTicks);
}
//------------------------------------------------

void INITCFG_ConfigUART()
{
	USARTx_Init(USART3, SYSCLK, USART_BAUDRATE);
	USARTx_RecieveInterrupt(USART3, true);
}
//------------------------------------------------

void INITCFG_ConfigTRMUART()
{
	USARTx_Init(USART2, SYSCLK, USART_BAUDRATE);
	USARTx_RecieveInterrupt(USART2, true);
}
//------------------------------------------------

void INITCFG_ConfigCAN()
{
	RCC_CAN_Clk_EN(CAN_1_ClkEN);
	NCAN_Init(SYSCLK, CAN_BAUDRATE, FALSE);
	NCAN_FIFOInterrupt(TRUE);
	NCAN_FilterInit(0, CAN_SLAVE_FILTER_ID, CAN_SLAVE_NID_MASK);
}
//------------------------------------------------

void INITCFG_ConfigSPI()
{
	SPI_Init8b(SPI3, SPI3_BAUDRATE_BITS, SPI_MSB_FIRST);
}
//------------------------------------------------

void INITCFG_ConfigRS485()
{
	GPIO_SetState(GPIO_RS485_CTRL, false);
}
//------------------------------------------------

void INITCFG_ConfigTimer7()
{
	TIM_Clock_En(TIM_7);
	TIM_Config(TIM7, SYSCLK, TIMER7_uS);
	TIM_Interupt(TIM7, 2, true);
	TIM_Start(TIM7);
}
//------------------------------------------------

void INITCFG_ConfigTimer1()
{
	TIM_Clock_En(TIM_1);
	TIM_Config(TIM1, SYSCLK, TIMER1_uS);
	TIM_Interupt(TIM1, 1, true);
	TIM_Start(TIM1);
}
//------------------------------------------------

void INITCFG_PWM()
{
	T3Ch4PWM_Init(SYSCLK, TIMER3_PWM_uS);
}
//------------------------------------------------

void INITCFG_ConfigWatchDog()
{
	IWDG_Config();
	IWDG_ConfigureSlowUpdate();
}
//------------------------------------------------

void INITCFG_ConfigTimer15()
{
	TIM_Clock_En(TIM_15);
	TIM_Config(TIM15, SYSCLK, TIMER15_uS);
	TIM_MasterMode(TIM15, MMS_UPDATE);
	TIM_Start(TIM15);
}
//------------------------------------------------

void INITCFG_ConfigADC()
{
	RCC_ADC_Clk_EN(ADC_12_ClkEN);

	ADC_Calibration(ADC2);
	ADC_Enable(ADC2);
	ADC_TrigConfig(ADC2, ADC12_TIM15_TRGO, RISE);

	ADC_ChannelSeqReset(ADC2);
	ADC_ChannelSet_Sequence(ADC2, ADC_PRESSURE_CHANNEL, 1);
	ADC_ChannelSeqLen(ADC2, ADC_PRESSURE_SEQ_LENGTH);

	ADC_ChannelSet_SampleTime(ADC2, ADC_PRESSURE_CHANNEL, ADC_SMPL_TIME_4_5);
	ADC_DMAConfigWithAutDLY(ADC2);
	ADC_SamplingStart(ADC2);
}
//------------------------------------------------

void INITCFG_ConfigDMA()
{
	DMA_Clk_Enable(DMA2_ClkEN);

	DMA_Reset(DMA2_Channel1);
	DMAChannelX_DataConfig(DMA2_Channel1, (uint32_t)INITCFG_PressureAdcBuffer, (uint32_t)(&ADC2->DR),
			ADC_PRESSURE_BUF_SIZE);
	DMAChannelX_Config(DMA2_Channel1, DMA_MEM2MEM_DIS, DMA_LvlPriority_LOW, DMA_MSIZE_16BIT, DMA_PSIZE_16BIT,
			DMA_MINC_EN, DMA_PINC_DIS, DMA_CIRCMODE_EN, DMA_READ_FROM_PERIPH);
	DMA_ChannelEnable(DMA2_Channel1, true);
}
//------------------------------------------------
