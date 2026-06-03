// Header
//
#include "OWENProtocol.h"
#include "SysConfig.h"
#include "ZwUtils.h"

// Functions
//
Int16U OWENProtocol_CRC16(pInt16U Buffer, Int16U BufferSize)
{
	Int16U i, j, b, crc = 0;

	for (i = 0; i < BufferSize; ++i)
	{
		b = Buffer[i];
		for (j = 0; j < 8; ++j, b = (b << 1) & 0xFF)
		{
			if ((b ^ (crc >> 8)) & 0x80)
			{
				crc <<= 1;
				crc ^= 0x8F57;
			}
			else
				crc <<= 1;
		}
	}

	return crc;
}
// ----------------------------------------

Int16U OWENProtocol_FrameToASCII(pInt16U Buffer, Int16U BufferSize, pInt16U BufferASCII)
{
	Int16U i, j;

	if (BufferSize * 2 + 2 > OWNP_MAX_ASCII_FRAME_SIZE) return 0;

	BufferASCII[0] = '#';
	for (i = 0, j = 1; i < BufferSize; ++i, j += 2)
	{
		BufferASCII[j] = 'G' + ((Buffer[i] >> 4) & 0x0f);
		BufferASCII[j + 1] = 'G' + (Buffer[i] & 0x0f);
	}
	BufferASCII[BufferSize * 2 + 1] = 0x0d;

	return BufferSize * 2 + 2;
}
// ----------------------------------------

Int16U OWENProtocol_ASCIIToFrame(pInt16U BufferASCII, Int16U BufferASCIISize, pInt16U Buffer)
{
	Int16U i, j;

	if (BufferASCIISize < 2) return 0;
	if ((BufferASCIISize - 2)/2 > OWNP_MAX_FRAME_SIZE) return 0;

	for (i = 1, j = 0; i < BufferASCIISize - 2;  i += 2, ++j)
		Buffer[j] = (BufferASCII[i] - 'G') << 4 | (BufferASCII[i + 1] - 'G');

	return (BufferASCIISize - 2)/2;
}
// ----------------------------------------

Int16U OWENProtocol_FramePack(pOWENProtocol_Frame Frame, pInt16U Buffer)
{
	Int16U crc;

	if (Frame->AddressLength == 8)
	{
		Buffer[0] = Frame->Address & 0xff;
		Buffer[1] = 0;
	}
	else
	{
		Buffer[0] = (Frame->Address >> 3)  & 0xff;
		Buffer[1] = (Frame->Address & 0x07) << 5;
	}

	if (Frame->Request)
		Buffer[1] |= 0x10;

	Buffer[1] |= Frame->DataSize;
	Buffer[2] = (Frame->Hash >> 8) & 0xff;
	Buffer[3] = Frame->Hash & 0xff;

	if (Frame->DataSize)
		MemCopy16(Frame->Data, Buffer + 4, Frame->DataSize);

	crc = OWENProtocol_CRC16(Buffer, Frame->DataSize + 4);
	Frame->CRC = crc;
	Frame->CRC_OK = TRUE;

	Buffer[Frame->DataSize + 4] = (crc >> 8) & 0xff;
	Buffer[Frame->DataSize + 5] = crc & 0xff;

	return Frame->DataSize + 6;
}
// ----------------------------------------

void OWENProtocol_FrameUnPack(pInt16U Buffer, Int16U BufferSize, pOWENProtocol_Frame Frame)
{
	Int16U DataSize;

	// ATTENTION! Unable to differ 11bit addresses from 8bit if they are divisible by 8
	if (Buffer[1] & 0xe0)
	{
		Frame->Address = (Buffer[0] << 3) | (Buffer[1] >> 5);
		Frame->AddressLength = 11;
	}
	else
	{
		Frame->Address = Buffer[0];
		Frame->AddressLength = 8;
	}

	Frame->Request = (Buffer[1] & 0x10) != 0;
	Frame->Hash = (Buffer[2] << 8) | Buffer[3];
	DataSize = Buffer[1] & 0x0F;

	if (DataSize)
	{
		Frame->DataSize = DataSize;
		MemCopy16(Buffer + 4, Frame->Data, DataSize);
	}
	else
	{
		Frame->DataSize = 0;
	}
	Frame->CRC = (Buffer[BufferSize - 2] << 8) | Buffer[BufferSize - 1];
	Frame->CRC_OK = (Frame->CRC == OWENProtocol_CRC16(Buffer, BufferSize - 2));
}
// ----------------------------------------

void OWENProtocol_NameToID(char *Name, Int16U NameLength, Int16U Id[4])
{
	Int16U i, j, b, symbol;

	for (i = 0, j = 0; i < NameLength && j <= 4; ++i)
	{
		symbol = Name[i];

		if ('0' <= symbol && symbol <= '9')
		{
			b = symbol - '0';
		}
		else if ('a' <= symbol && symbol <= 'z')
		{
			b = 10 + symbol - 'a';
		}
		else if ('A' <= symbol && symbol <= 'Z')
		{
			b = 10 + symbol - 'A';
		}
		else if ('-' == symbol)
		{
			b = 10 + 26 + 0;
		}
		else if ('_' == symbol)
		{
			b = 10 + 26 + 1;
		}
		else if ('/' == symbol)
		{
			b = 10 + 26 + 2;
		}
		else if ('.' == symbol)
		{
			++Id[j - 1];
			continue;
		}
		else if (' ' == symbol)
		{
			break;
		}

		Id[j++] = b * 2;
	}

	if (j != 4)
	{
		for (; i < NameLength; ++i)
			Id[j++] = 78;

		for (; j < 4; ++j)
			Id[j] = 78;
	}
}
// ----------------------------------------

Int16U OWENProtocol_IDToHash(Int16U Id[4])
{
	Int16U i, j, b, hash;

	hash = 0;
	for (i = 0; i < 4; ++i)
	{
		b = Id[i];
		b <<= 1;
		for (j = 0; j < 7; ++j, b <<= 1)
		{
			if ((b ^ (hash >> 8)) & 0x80)
			{
				hash <<= 1;
				hash ^= 0x8F57;
			}
			else
				hash <<= 1;
		}
	}
	return hash;
}
// ----------------------------------------

// No more
