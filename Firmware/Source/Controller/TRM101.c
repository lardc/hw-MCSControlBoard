// Header
#include "TRM101.h"
//
// Include
#include "OWENProtocol.h"
#include "SysConfig.h"
#include "ZwUtils.h"
#include "ZbBoard.h"
#include "IQmathUtils.h"

// Functions
//
void TRM_DataExchange(Int16U Address, Int16U Hash, Boolean Request, pInt16U DataIn, Int16U DataInSize, pInt16U DataOut, pInt16U DataOutSize, pTRMError error)
{
	Int16S ret_val;
	Int16U DataCounter, i;
	Int16U RawBuffer[OWNP_MAX_FRAME_SIZE], ASCIIBuffer[OWNP_MAX_ASCII_FRAME_SIZE];
	OWENProtocol_Frame frameOut, frameIn;

	// Compose frame
	frameOut.Address = Address;
	frameOut.AddressLength = 8;
	frameOut.Hash = Hash;
	frameOut.Request = Request ? 1 : 0;
	frameOut.DataSize = Request ? 0 : DataInSize;
	if (frameOut.DataSize)
		MemCopy16(DataIn, frameOut.Data, frameOut.DataSize);

	// Pack frame
	DataCounter = OWENProtocol_FramePack(&frameOut, RawBuffer);

	// Convert to ASCII and send
	DataCounter = OWENProtocol_FrameToASCII(RawBuffer, DataCounter, ASCIIBuffer);
	ZbSU_SendData(ASCIIBuffer, DataCounter);

	// Recieve data
	ret_val = ZbSU_ReadData(ASCIIBuffer, OWNP_MAX_ASCII_FRAME_SIZE);
	if (ret_val == -1)
	{
		*error = TRME_ResponseTimeout;
		return;
	}
	else if (ret_val == 0)
	{
		*error = TRME_InputBufferOverrun;
		return;
	}
	else
		DataCounter = ret_val;

	// Convert ASCII data to frame
	DataCounter = OWENProtocol_ASCIIToFrame(ASCIIBuffer, DataCounter, RawBuffer);
	OWENProtocol_FrameUnPack(RawBuffer, DataCounter, &frameIn);

	// Validate input data
	if (frameIn.CRC_OK == 0)
	{
		*error = TRME_CheckSumError;
		return;
	}
	else if (frameOut.Request == 0 && frameOut.CRC != frameIn.CRC)
	{
		*error = TRME_CheckSumError;
		return;
	}
	else if (frameOut.Request)
	{
		if (frameOut.Address != frameIn.Address ||
			frameOut.AddressLength != frameIn.AddressLength ||
			frameOut.Hash != frameIn.Hash ||
			frameIn.Request ||
			frameIn.DataSize == 0 || frameIn.DataSize > OWPNP_MAX_DATA_BYTES)
		{
			*error = TRME_WrongResponse;
			return;
		}
	}

	// Write output data
	if (frameOut.Request)
	{
		for (i = 0; i < frameIn.DataSize; i++)
			DataOut[i] = frameIn.Data[i];

		*DataOutSize = frameIn.DataSize;
	}
	*error = TRME_None;
}
// ----------------------------------------

_iq TRM_UnPackFloat24(pInt16U Buffer)
{
	Int16U i;
	Int32U tmp, Float = 0;

	for (i = 0; i < 3; i++)
	{
		tmp = Buffer[i] & 0xFF;
		tmp <<= (3 - i) * 8;
		Float |= tmp;
	}

	return FloatToIQ(Float);
}
// ----------------------------------------

Int16U TRM_ReadF24(Int16U Address, Int16U Hash, Int16U Multiplier, pTRMError error)
{
	Int16U Data[OWPNP_MAX_DATA_BYTES], DataCounter;

	TRM_DataExchange(Address, Hash, TRUE, NULL, 0, Data, &DataCounter, error);
	if (DataCounter == 3)
		return _IQmpyI32int(TRM_UnPackFloat24(Data), Multiplier);
	else
		return 0;
}
// ----------------------------------------

void TRM_WriteF24(Int16U Address, Int16U Hash, Int16U Value, Int16U Divisor, pTRMError error)
{
	Int16U Data[OWPNP_MAX_DATA_BYTES], DataCounter, DataOut[3];
	Int32U tmp;

	tmp = IQToFloat(_FPtoIQ2(Value, Divisor));
	DataOut[0] = (tmp >> 24);
	DataOut[1] = (tmp >> 16) & 0xFF;
	DataOut[2] = (tmp >>  8) & 0xFF;

	TRM_DataExchange(Address, Hash, FALSE, DataOut, 3, Data, &DataCounter, error);
}
// ----------------------------------------

void TRM_Command(Int16U Address, Int16U Hash, Boolean Start, pTRMError error)
{
	Int16U Data[OWPNP_MAX_DATA_BYTES], DataCounter;
	Int16U CMD = Start ? 1 : 0;

	TRM_DataExchange(Address, Hash, FALSE, &CMD, 1, Data, &DataCounter, error);
}
// ----------------------------------------

Int16U TRM_ReadTemp(Int16U Address, pTRMError error)
{
	return TRM_ReadF24(Address, 0xB8DF, 10, error);
}
// ----------------------------------------

Int16U TRM_ReadPower(Int16U Address, pTRMError error)
{
	return TRM_ReadF24(Address, 0x35E8, 10, error);
}
// ----------------------------------------

void TRM_SetTemp(Int16U Address, Int16U Temperature, pTRMError error)
{
	TRM_WriteF24(Address, 0x9107, Temperature, 10, error);
}
// ----------------------------------------

void TRM_Start(Int16U Address, pTRMError error)
{
	TRM_Command(Address, 0xAF90, TRUE, error);
}
// ----------------------------------------

void TRM_Stop(Int16U Address, pTRMError error)
{
	TRM_Command(Address, 0xAF90, FALSE, error);
}
// ----------------------------------------

// No more.
