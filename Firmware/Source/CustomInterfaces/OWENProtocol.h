#ifndef __OWENPROTOCOL_H
#define __OWENPROTOCOL_H

// Include
#include "OWENProtocolTypes.h"
#include "stdinc.h"

// Definitions
//
#define OWNP_MAX_FRAME_SIZE				21
#define OWNP_MAX_ASCII_FRAME_SIZE		45
#define OWPNP_MAX_DATA_BYTES			15

// Functions
//
void OWENProtocol_NameToID(char *Name, Int16U NameLength, Int16U Id[4]);
Int16U OWENProtocol_IDToHash(Int16U Id[4]);
Int16U OWENProtocol_FramePack(pOWENProtocol_Frame Frame, pInt16U Buffer);
Int16U OWENProtocol_FrameToASCII(pInt16U Buffer, Int16U BufferSize, pInt16U BufferASCII);
Int16U OWENProtocol_ASCIIToFrame(pInt16U BufferASCII, Int16U BufferASCIISize, pInt16U Buffer);
void OWENProtocol_FrameUnPack(pInt16U Buffer, Int16U BufferSize, pOWENProtocol_Frame Frame);

#endif // __OWENPROTOCOL_H
