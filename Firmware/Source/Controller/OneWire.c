#include "OneWire.h"
#include "Delay.h"

typedef struct __OneWireBus
{
	GPIO_PortPinSetting writePin;
	GPIO_PortPinSetting readPin;
	GPIO_PortPinSetting powerPin;
	Boolean hasPowerPin;
	Int8U ROM_NO[8];
	Int8U LastDiscrepancy;
	Int8U LastFamilyDiscrepancy;
	Boolean LastDeviceFlag;
} OneWireBus;

static OneWireBus Bus;

// Запись уровня на выходной пин (прямой доступ к регистрам для точных таймингов)
static void directWrite(GPIO_PortPinSetting pin, Boolean high)
{
	if (high)
		pin.Port->BSRRL = (1U << pin.Pin);
	else
		pin.Port->BRR = (1U << pin.Pin);
}
//-------------------

// Чтение уровня с входного пина
static Boolean directRead(GPIO_PortPinSetting pin)
{
	return (pin.Port->IDR & (1U << pin.Pin)) != 0;
}
//-------------------

// Управление линией записи
static void setWriteLine(Boolean active)
{
	directWrite(Bus.writePin, active);
}
//-------------------

// Чтение линии данных
static Boolean readLine()
{
	return directRead(Bus.readPin);
}
//-------------------

// Управление линией parasite power (сильная подтяжка к питанию)
static void setPowerLine(Boolean active)
{
	if (!Bus.hasPowerPin)
		return;

	GPIO_SetState(Bus.powerPin, active);
}
//-------------------

// Инициализация шины: пины записи, чтения, parasite power и флаг использования питания
void OneWire_Init(GPIO_PortPinSetting writePin, GPIO_PortPinSetting readPin, GPIO_PortPinSetting powerPin,
		Boolean usePowerPin)
{
	Bus.writePin = writePin;
	Bus.readPin = readPin;
	Bus.powerPin = powerPin;
	Bus.hasPowerPin = usePowerPin;

	GPIO_InitPushPullOutput(Bus.writePin);
	GPIO_InitInput(Bus.readPin, NoPull);

	if (Bus.hasPowerPin)
	{
		GPIO_InitOpenDrainOutput(Bus.powerPin, NoPull);
		setPowerLine(false);
	}

	setWriteLine(true);

	OneWire_ResetSearch();
}
//-------------------

// Сброс шины 1-Wire. Возвращает 1 при ответе устройства (presence pulse), 0 — при отсутствии или КЗ
Int8U OneWire_Reset()
{
	Int8U result;
	Int8U retries = 125;

	__disable_irq();
	setWriteLine(true);
	__enable_irq();

	// Ожидание освобождения линии
	do
	{
		if (--retries == 0)
			return 0;
		DELAY_US(2);
	}
	while (!readLine());

	__disable_irq();
	setWriteLine(false);
	__enable_irq();
	DELAY_US(480);

	__disable_irq();
	setWriteLine(true);
	DELAY_US(70);
	result = !readLine();
	__enable_irq();
	DELAY_US(410);

	return result;
}
//-------------------

// Запись одного бита на шину
void OneWire_WriteBit(Int8U value)
{
	if (value & 1)
	{
		__disable_irq();
		setWriteLine(false);
		DELAY_US(10);
		setWriteLine(true);
		__enable_irq();
		DELAY_US(55);
	}
	else
	{
		__disable_irq();
		setWriteLine(false);
		DELAY_US(65);
		setWriteLine(true);
		__enable_irq();
		DELAY_US(5);
	}
}
//-------------------

// Чтение одного бита с шины
Int8U OneWire_ReadBit()
{
	Int8U result;

	__disable_irq();
	setWriteLine(false);
	DELAY_US(3);
	setWriteLine(true);
	DELAY_US(10);
	result = readLine();
	__enable_irq();
	DELAY_US(53);

	return result;
}
//-------------------

// Запись байта. При power != 0 удерживает parasite power до вызова OneWire_Depower()
void OneWire_Write(Int8U value, Int8U power)
{
	Int8U bitMask;

	for (bitMask = 0x01; bitMask; bitMask <<= 1)
		OneWire_WriteBit((bitMask & value) ? 1 : 0);

	if (!power)
	{
		__disable_irq();
		setWriteLine(true);
		__enable_irq();
	}
	else if (Bus.hasPowerPin)
		setPowerLine(true);
}
//-------------------

// Запись буфера байтов
void OneWire_WriteBytes(const Int8U *buf, Int16U count, Boolean power)
{
	Int16U i;

	for (i = 0; i < count; i++)
		OneWire_Write(buf[i], 0);

	if (!power)
	{
		__disable_irq();
		setWriteLine(true);
		__enable_irq();
	}
	else if (Bus.hasPowerPin)
		setPowerLine(true);
}
//-------------------

// Чтение байта
Int8U OneWire_Read()
{
	Int8U bitMask;
	Int8U result = 0;

	for (bitMask = 0x01; bitMask; bitMask <<= 1)
	{
		if (OneWire_ReadBit())
			result |= bitMask;
	}

	return result;
}
//-------------------

// Чтение буфера байтов
void OneWire_ReadBytes(Int8U *buf, Int16U count)
{
	Int16U i;

	for (i = 0; i < count; i++)
		buf[i] = OneWire_Read();
}
//-------------------

// Команда MATCH ROM (0x55) — выбор устройства по 64-битному адресу
void OneWire_Select(const Int8U *rom)
{
	Int8U i;

	OneWire_Write(0x55, 0);

	for (i = 0; i < 8; i++)
		OneWire_Write(rom[i], 0);
}
//-------------------

// Команда SKIP ROM (0xCC) — обращение ко всем устройствам на шине
void OneWire_Skip()
{
	OneWire_Write(0xCC, 0);
}
//-------------------

// Отключение parasite power и освобождение линии данных
void OneWire_Depower()
{
	if (Bus.hasPowerPin)
		setPowerLine(false);

	__disable_irq();
	setWriteLine(true);
	__enable_irq();
}
//-------------------

// Сброс состояния поиска устройств на шине
void OneWire_ResetSearch()
{
	Int8S i;

	Bus.LastDiscrepancy = 0;
	Bus.LastDeviceFlag = false;
	Bus.LastFamilyDiscrepancy = 0;

	for (i = 7; i >= 0; i--)
		Bus.ROM_NO[i] = 0;
}
//-------------------

// Настройка поиска устройств заданного семейства (первый байт ROM)
void OneWire_TargetSearch(Int8U familyCode)
{
	Int8U i;

	Bus.ROM_NO[0] = familyCode;
	for (i = 1; i < 8; i++)
		Bus.ROM_NO[i] = 0;

	Bus.LastDiscrepancy = 64;
	Bus.LastFamilyDiscrepancy = 0;
	Bus.LastDeviceFlag = false;
}
//-------------------

// Поиск следующего устройства на шине. Возвращает true и записывает ROM в newAddr при успехе
Boolean OneWire_Search(Int8U *newAddr, Boolean searchMode)
{
	Int8U idBitNumber;
	Int8U lastZero, romByteNumber;
	Boolean searchResult;
	Int8U idBit, cmpIdBit;
	Int8U romByteMask, searchDirection;
	Int8S i;

	idBitNumber = 1;
	lastZero = 0;
	romByteNumber = 0;
	romByteMask = 1;
	searchResult = false;

	if (!Bus.LastDeviceFlag)
	{
		if (!OneWire_Reset())
		{
			Bus.LastDiscrepancy = 0;
			Bus.LastDeviceFlag = false;
			Bus.LastFamilyDiscrepancy = 0;
			return false;
		}

		if (searchMode)
			OneWire_Write(0xF0, 0);
		else
			OneWire_Write(0xEC, 0);

		do
		{
			idBit = OneWire_ReadBit();
			cmpIdBit = OneWire_ReadBit();

			if ((idBit == 1) && (cmpIdBit == 1))
				break;

			if (idBit != cmpIdBit)
				searchDirection = idBit;
			else
			{
				if (idBitNumber < Bus.LastDiscrepancy)
					searchDirection = ((Bus.ROM_NO[romByteNumber] & romByteMask) > 0);
				else
					searchDirection = (idBitNumber == Bus.LastDiscrepancy);

				if (searchDirection == 0)
				{
					lastZero = idBitNumber;
					if (lastZero < 9)
						Bus.LastFamilyDiscrepancy = lastZero;
				}
			}

			if (searchDirection == 1)
				Bus.ROM_NO[romByteNumber] |= romByteMask;
			else
				Bus.ROM_NO[romByteNumber] &= ~romByteMask;

			OneWire_WriteBit(searchDirection);

			idBitNumber++;
			romByteMask <<= 1;

			if (romByteMask == 0)
			{
				romByteNumber++;
				romByteMask = 1;
			}
		}
		while (romByteNumber < 8);

		if (!(idBitNumber < 65))
		{
			Bus.LastDiscrepancy = lastZero;
			if (Bus.LastDiscrepancy == 0)
				Bus.LastDeviceFlag = true;
			searchResult = true;
		}
	}

	if (!searchResult || !Bus.ROM_NO[0])
	{
		Bus.LastDiscrepancy = 0;
		Bus.LastDeviceFlag = false;
		Bus.LastFamilyDiscrepancy = 0;
		searchResult = false;
	}
	else
	{
		for (i = 0; i < 8; i++)
			newAddr[i] = Bus.ROM_NO[i];
	}

	return searchResult;
}
//-------------------

#if ONEWIRE_CRC8_TABLE
static const Int8U dscrc2x16_table[] = {
	0x00, 0x5E, 0xBC, 0xE2, 0x61, 0x3F, 0xDD, 0x83,
	0xC2, 0x9C, 0x7E, 0x20, 0xA3, 0xFD, 0x1F, 0x41,
	0x00, 0x9D, 0x23, 0xBE, 0x46, 0xDB, 0x65, 0xF8,
	0x8C, 0x11, 0xAF, 0x32, 0xCA, 0x57, 0xE9, 0x74
};

// Расчёт 8-битного CRC Dallas (ROM, scratchpad)
Int8U OneWire_Crc8(const Int8U *addr, Int8U len)
{
	Int8U crc = 0;

	while (len--)
	{
		crc = *addr++ ^ crc;
		crc = dscrc2x16_table[crc & 0x0f] ^ dscrc2x16_table[16 + ((crc >> 4) & 0x0f)];
	}

	return crc;
}
//-------------------
#else
// Расчёт 8-битного CRC Dallas (медленный вариант без таблицы)
Int8U OneWire_Crc8(const Int8U *addr, Int8U len)
{
	Int8U crc = 0;

	while (len--)
	{
		Int8U inbyte = *addr++;
		Int8U i;

		for (i = 8; i; i--)
		{
			Int8U mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix)
				crc ^= 0x8C;
			inbyte >>= 1;
		}
	}

	return crc;
}
//-------------------
#endif

#if ONEWIRE_CRC16
// Проверка 16-битного CRC (принимает инвертированные байты из ответа устройства)
Boolean OneWire_CheckCrc16(const Int8U *input, Int16U len, const Int8U *invertedCrc, Int16U crc)
{
	crc = ~OneWire_Crc16(input, len, crc);
	return (crc & 0xFF) == invertedCrc[0] && (crc >> 8) == invertedCrc[1];
}
//-------------------

// Расчёт 16-битного CRC Dallas
Int16U OneWire_Crc16(const Int8U *input, Int16U len, Int16U crc)
{
	static const Int8U oddparity[16] =
		{ 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };
	Int16U i;

	for (i = 0; i < len; i++)
	{
		Int16U cdata = input[i];
		cdata = (cdata ^ crc) & 0xff;
		crc >>= 8;

		if (oddparity[cdata & 0x0F] ^ oddparity[cdata >> 4])
			crc ^= 0xC001;

		cdata <<= 6;
		crc ^= cdata;
		cdata <<= 1;
		crc ^= cdata;
	}

	return crc;
}
//-------------------
#endif
