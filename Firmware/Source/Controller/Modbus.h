#ifndef CONTROLLER_MODBUS_H_
#define CONTROLLER_MODBUS_H_

// Include
#include "stdinc.h"

// Types
//
typedef enum __ModbusError
{
	MODBUS_OK					= 0,
	MODBUS_ERR_TIMEOUT			= 1,
	MODBUS_ERR_FRAME_BREAK		= 2,
	MODBUS_ERR_BAD_CRC			= 3,
	MODBUS_ERR_SLAVE_EXCEPTION	= 4,
	MODBUS_ERR_WRONG_SLAVE		= 5,
	MODBUS_ERR_WRONG_FUNCTION	= 6,
	MODBUS_ERR_BUFFER_OVERFLOW	= 7
} ModbusError, *pModbusError;

typedef void (*ModbusFunc_SendByte)(Int16U Value);
typedef Int16U (*ModbusFunc_GetBytesToReceive)(void);
typedef Int16U (*ModbusFunc_ReceiveByte)(void);
typedef void (*ModbusFunc_SetTxMode)(Boolean State);

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

// Functions
//
void Modbus_Init(pModbusInterface Interface,
		ModbusFunc_SendByte SendByte, ModbusFunc_GetBytesToReceive GetBytesToReceive,
		ModbusFunc_ReceiveByte ReceiveByte, ModbusFunc_SetTxMode SetTxMode,
		Int32U BaudRate, volatile Int64U *pTimeCounter, Int16U ResponseTimeoutTicks);

Int16U Modbus_CRC16(pInt8U Data, Int16U Length);
Boolean Modbus_CheckCRC(pInt8U Buffer, Int16U Length);

Int16U Modbus_BuildReadHoldingRegs(Int8U Slave, Int16U Address, Int16U Count, pInt8U Buffer);
Int16U Modbus_BuildWriteSingleReg(Int8U Slave, Int16U Address, Int16U Value, pInt8U Buffer);
Int16U Modbus_BuildWriteMultipleRegs(Int8U Slave, Int16U Address, Int16U Count, pInt16U Values, pInt8U Buffer);

ModbusError Modbus_ValidateResponse(pInt8U Buffer, Int16U Length, Int8U ExpectedSlave, Int8U ExpectedFunction,
		pModbusInterface Interface);

ModbusError Modbus_SendReceive(pModbusInterface Interface, pInt8U TxBuffer, Int16U TxLength,
		pInt8U RxBuffer, Int16U RxBufferSize, pInt16U RxLength);

#endif /* CONTROLLER_MODBUS_H_ */
