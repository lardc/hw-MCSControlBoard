// Header
#include "Modbus.h"

// Definitions
//
#define MODBUS_MAX_FRAME_SIZE			256
#define MODBUS_RESPONSE_TIMEOUT_TICKS	100
#define MODBUS_FC_READ_HOLDING_REGS		3
#define MODBUS_FC_WRITE_SINGLE_REG		6
#define MODBUS_FC_WRITE_MULTIPLE_REGS	16
#define MODBUS_MIN_FRAME_SIZE			5
#define MODBUS_CHAR_BITS				11
#define MODBUS_FRAME_GAP_MIN_TICKS		2
#define MODBUS_TIME_TICK_US				1000

// Variables
//
typedef struct __ModbusInterface
{
	ModbusFunc_SendByte			IO_SendByte;
	ModbusFunc_GetBytesToReceive	IO_GetBytesToReceive;
	ModbusFunc_ReceiveByte		IO_ReceiveByte;
	ModbusFunc_SetTxMode		IO_SetTxMode;
	Int32U						BaudRate;
	Int16U						FrameGapTicks;
	Int16U						ResponseTimeoutTicks;
	volatile Int64U				*pTimeCounter;
	Int8U						LastExceptionCode;
} ModbusInterface, *pModbusInterface;

static ModbusInterface Interface;

// Forward functions
//
static Int16U Modbus_CalcFrameGapTicks(Int32U BaudRate);
static void Modbus_SendBuffer(pModbusInterface Interface, pInt8U Buffer, Int16U Length);
static void Modbus_WaitFrameGap(pModbusInterface Interface);
static ModbusError Modbus_ReceiveFrame(pModbusInterface Interface, pInt8U Buffer, Int16U BufferSize, pInt16U Length);

// Functions
//
// Инициализация интерфейса Modbus и сохранение указателей на функции обмена
void Modbus_Init(ModbusFunc_SendByte SendByte, ModbusFunc_GetBytesToReceive GetBytesToReceive,
		ModbusFunc_ReceiveByte ReceiveByte, ModbusFunc_SetTxMode SetTxMode,
		Int32U BaudRate, volatile Int64U *pTimeCounter, Int16U ResponseTimeoutTicks)
{
	Interface.IO_SendByte = SendByte;
	Interface.IO_GetBytesToReceive = GetBytesToReceive;
	Interface.IO_ReceiveByte = ReceiveByte;
	Interface.IO_SetTxMode = SetTxMode;
	Interface.BaudRate = BaudRate;
	Interface.pTimeCounter = pTimeCounter;
	Interface.ResponseTimeoutTicks = ResponseTimeoutTicks;
	Interface.LastExceptionCode = 0;
	Interface.FrameGapTicks = Modbus_CalcFrameGapTicks(BaudRate);
}
// ----------------------------------------

// Расчёт межкадровой паузы Modbus RTU в тиках внутреннего таймера
static Int16U Modbus_CalcFrameGapTicks(Int32U BaudRate)
{
	Int32U charTimeUs, t35Us;
	Int16U ticks;

	if(BaudRate == 0)
		return MODBUS_FRAME_GAP_MIN_TICKS;

	charTimeUs = (MODBUS_CHAR_BITS * 1000000UL) / BaudRate;
	t35Us = (charTimeUs * 35UL) / 10UL;
	ticks = (Int16U)((t35Us + MODBUS_TIME_TICK_US - 1) / MODBUS_TIME_TICK_US);

	if(ticks < MODBUS_FRAME_GAP_MIN_TICKS)
		ticks = MODBUS_FRAME_GAP_MIN_TICKS;

	return ticks;
}
// ----------------------------------------

// Расчёт контрольной суммы CRC16 Modbus для буфера данных
Int16U Modbus_CRC16(pInt8U Data, Int16U Length)
{
	Int16U crc = 0xFFFF;
	Int16U i, j;

	for(i = 0; i < Length; ++i)
	{
		crc ^= Data[i];
		for(j = 0; j < 8; ++j)
		{
			if(crc & 0x0001)
				crc = (crc >> 1) ^ 0xA001;
			else
				crc >>= 1;
		}
	}

	return crc;
}
// ----------------------------------------

// Проверка CRC у принятого Modbus кадра
Boolean Modbus_CheckCRC(pInt8U Buffer, Int16U Length)
{
	Int16U crcReceived;

	if(Length < MODBUS_MIN_FRAME_SIZE)
		return FALSE;

	crcReceived = Buffer[Length - 2] | ((Int16U)Buffer[Length - 1] << 8);
	return Modbus_CRC16(Buffer, Length - 2) == crcReceived;
}
// ----------------------------------------

// Упаковка запроса чтения holding-регистров с добавлением CRC
Int16U Modbus_BuildReadHoldingRegs(Int8U Slave, Int16U Address, Int16U Count, pInt8U Buffer)
{
	Int16U length = 6;
	Int16U crc;

	if(Buffer == NULL || Count == 0 || Count > 125)
		return 0;

	Buffer[0] = Slave;
	Buffer[1] = MODBUS_FC_READ_HOLDING_REGS;
	Buffer[2] = (Int8U)(Address >> 8);
	Buffer[3] = (Int8U)(Address & 0xFF);
	Buffer[4] = (Int8U)(Count >> 8);
	Buffer[5] = (Int8U)(Count & 0xFF);

	crc = Modbus_CRC16(Buffer, length);
	Buffer[length++] = (Int8U)(crc & 0xFF);
	Buffer[length++] = (Int8U)(crc >> 8);

	return length;
}
// ----------------------------------------

// Упаковка запроса записи одного регистра с добавлением CRC
Int16U Modbus_BuildWriteSingleReg(Int8U Slave, Int16U Address, Int16U Value, pInt8U Buffer)
{
	Int16U length = 6;
	Int16U crc;

	if(Buffer == NULL)
		return 0;

	Buffer[0] = Slave;
	Buffer[1] = MODBUS_FC_WRITE_SINGLE_REG;
	Buffer[2] = (Int8U)(Address >> 8);
	Buffer[3] = (Int8U)(Address & 0xFF);
	Buffer[4] = (Int8U)(Value >> 8);
	Buffer[5] = (Int8U)(Value & 0xFF);

	crc = Modbus_CRC16(Buffer, length);
	Buffer[length++] = (Int8U)(crc & 0xFF);
	Buffer[length++] = (Int8U)(crc >> 8);

	return length;
}
// ----------------------------------------

// Упаковка запроса записи нескольких регистров с добавлением CRC
Int16U Modbus_BuildWriteMultipleRegs(Int8U Slave, Int16U Address, Int16U Count, pInt16U Values, pInt8U Buffer)
{
	Int16U i, length, byteCount, crc;

	if(Buffer == NULL || Values == NULL || Count == 0 || Count > 123)
		return 0;

	byteCount = Count * 2;
	length = 7 + byteCount;

	if(length + 2 > MODBUS_MAX_FRAME_SIZE)
		return 0;

	Buffer[0] = Slave;
	Buffer[1] = MODBUS_FC_WRITE_MULTIPLE_REGS;
	Buffer[2] = (Int8U)(Address >> 8);
	Buffer[3] = (Int8U)(Address & 0xFF);
	Buffer[4] = (Int8U)(Count >> 8);
	Buffer[5] = (Int8U)(Count & 0xFF);
	Buffer[6] = (Int8U)byteCount;

	for(i = 0; i < Count; ++i)
	{
		Buffer[7 + i * 2] = (Int8U)(Values[i] >> 8);
		Buffer[8 + i * 2] = (Int8U)(Values[i] & 0xFF);
	}

	crc = Modbus_CRC16(Buffer, length);
	Buffer[length++] = (Int8U)(crc & 0xFF);
	Buffer[length++] = (Int8U)(crc >> 8);

	return length;
}
// ----------------------------------------

// Проверка адреса, кода функции и CRC у ответа slave-устройства
ModbusError Modbus_ValidateResponse(pInt8U Buffer, Int16U Length, Int8U ExpectedSlave, Int8U ExpectedFunction)
{
	if(Length < MODBUS_MIN_FRAME_SIZE)
		return MODBUS_ERR_FRAME_BREAK;

	if(!Modbus_CheckCRC(Buffer, Length))
		return MODBUS_ERR_BAD_CRC;

	if(Buffer[0] != ExpectedSlave)
		return MODBUS_ERR_WRONG_SLAVE;

	if(Buffer[1] == (Int8U)(ExpectedFunction | 0x80))
	{
		Interface.LastExceptionCode = Buffer[2];

		return MODBUS_ERR_SLAVE_EXCEPTION;
	}

	if(Buffer[1] != ExpectedFunction)
		return MODBUS_ERR_WRONG_FUNCTION;

	return MODBUS_OK;
}
// ----------------------------------------

// Отправка Modbus запроса и приём ответа с базовой обработкой ошибок
ModbusError Modbus_SendReceive(pInt8U TxBuffer, Int16U TxLength, pInt8U RxBuffer, Int16U RxBufferSize, pInt16U RxLength)
{
	ModbusError error;

	if(Interface.IO_SendByte == NULL || Interface.IO_GetBytesToReceive == NULL
			|| Interface.IO_ReceiveByte == NULL || Interface.pTimeCounter == NULL || TxBuffer == NULL || RxBuffer == NULL
			|| RxLength == NULL || TxLength < MODBUS_MIN_FRAME_SIZE)
		return MODBUS_ERR_FRAME_BREAK;

	Interface.LastExceptionCode = 0;

	while(Interface.IO_GetBytesToReceive())
		Interface.IO_ReceiveByte();

	Modbus_WaitFrameGap(&Interface);
	Modbus_SendBuffer(&Interface, TxBuffer, TxLength);
	Modbus_WaitFrameGap(&Interface);

	error = Modbus_ReceiveFrame(&Interface, RxBuffer, RxBufferSize, RxLength);
	if(error != MODBUS_OK)
		return error;

	return Modbus_ValidateResponse(RxBuffer, *RxLength, TxBuffer[0], TxBuffer[1]);
}
// ----------------------------------------

// Получение кода последнего exception-ответа slave
Int8U Modbus_GetLastExceptionCode()
{
	return Interface.LastExceptionCode;
}
// ----------------------------------------

// Передача буфера в линию с переключением RS485 в режим передачи
static void Modbus_SendBuffer(pModbusInterface Interface, pInt8U Buffer, Int16U Length)
{
	Int16U i;

	if(Interface->IO_SetTxMode != NULL)
		Interface->IO_SetTxMode(TRUE);

	for(i = 0; i < Length; ++i)
		Interface->IO_SendByte(Buffer[i]);

	if(Interface->IO_SetTxMode != NULL)
		Interface->IO_SetTxMode(FALSE);
}
// ----------------------------------------

// Выдержка межкадровой паузы по внутреннему таймеру
static void Modbus_WaitFrameGap(pModbusInterface Interface)
{
	Int64U startTime = *Interface->pTimeCounter;

	while(*Interface->pTimeCounter - startTime < Interface->FrameGapTicks);
}
// ----------------------------------------

// Приём Modbus кадра до паузы на линии или до таймаута ожидания
static ModbusError Modbus_ReceiveFrame(pModbusInterface Interface, pInt8U Buffer, Int16U BufferSize, pInt16U Length)
{
	Int64U startTime = *Interface->pTimeCounter;
	Int64U lastByteTime = 0;
	Int16U received = 0;
	Boolean frameStarted = FALSE;

	*Length = 0;

	while(*Interface->pTimeCounter - startTime <= Interface->ResponseTimeoutTicks)
	{
		while(Interface->IO_GetBytesToReceive())
		{
			if(received >= BufferSize)
				return MODBUS_ERR_BUFFER_OVERFLOW;

			Buffer[received++] = (Int8U)Interface->IO_ReceiveByte();
			lastByteTime = *Interface->pTimeCounter;
			frameStarted = TRUE;
		}

		if(frameStarted && (*Interface->pTimeCounter - lastByteTime) >= Interface->FrameGapTicks)
			break;
	}

	*Length = received;

	if(received == 0)
		return MODBUS_ERR_TIMEOUT;

	if(received < MODBUS_MIN_FRAME_SIZE)
		return MODBUS_ERR_FRAME_BREAK;

	if(!frameStarted || (*Interface->pTimeCounter - lastByteTime) < Interface->FrameGapTicks)
		return MODBUS_ERR_FRAME_BREAK;

	return MODBUS_OK;
}
// ----------------------------------------
