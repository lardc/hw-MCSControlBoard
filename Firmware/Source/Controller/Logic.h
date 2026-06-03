#ifndef LOGIC_H_
#define LOGIC_H_

// Types
typedef enum __DUTType
{
	DT_None = 0,
	DT_MIAA = 2001,
	DT_MIDA = 2002,
	DT_MIFA = 2003,
	DT_MIHA = 2004,
	DT_MIHM = 2005,
	DT_MIHV = 2006,
	DT_MISM = 2007,
	DT_MISV = 2008,
	DT_MIXM = 2009,
	DT_MIXV = 2010,
	DT_MCDA = 2011,
	DT_MISM2 = 2013,






} DUTType;

// Functions
//
DUTType LOGIC_AdapterIDMatch(float IDVoltage);
void LOGIC_UpdateSensors();

#endif /* LOGIC_H_ */
