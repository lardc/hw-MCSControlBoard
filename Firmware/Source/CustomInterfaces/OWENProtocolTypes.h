#ifndef __OWENPROTOCOL_TYPES_H
#define __OWENPROTOCOL_TYPES_H

// Include
#include "stdinc.h"

// Types
//
// IO configuration
typedef struct __OWENProtocol_Frame
{
	Int16U Address;
	Int16U AddressLength;
	Int16U Request;
	Int16U Hash;
	Int16U DataSize;
	Int16U Data[15];
	Int16U CRC;
	Boolean CRC_OK;
} OWENProtocol_Frame, *pOWENProtocol_Frame;


#endif // __OWENPROTOCOL_TYPES_H
