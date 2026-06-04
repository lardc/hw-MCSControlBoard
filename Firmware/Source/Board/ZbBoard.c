#include "ZbBoard.h"

void MemCopy16(pInt16U Src, pInt16U Dst, Int16U Size)
{
	Int16U i;

	for(i = 0; i < Size; i++)
		Dst[i] = Src[i];
}
