// ----------------------------------------
// Device data table
// ----------------------------------------

// Header
#include "DataTable.h"
//
#include "SysConfig.h"
#include "FirmwareInfo.h"

// Constants
//
#define DT_EPROM_ADDRESS	0x0801F800ul

// Variables
//
static EPROMServiceConfig EPROMServiceCfg;
volatile float DataTable[DATA_TABLE_SIZE];

// Functions
//
void DT_Init(EPROMServiceConfig EPROMService, Boolean NoRestore)
{
	Int16U i;
	
	EPROMServiceCfg = EPROMService;
	
	for(i = 0; i < DATA_TABLE_SIZE; ++i)
		DataTable[i] = 0;
	
	if(!NoRestore)
		DT_RestoreNVPartFromEPROM();
}
// ----------------------------------------

void DT_RestoreNVPartFromEPROM()
{
#ifdef USE_FLOAT_DT
	const Int16U TableSize = DATA_TABLE_NV_SIZE * 2;
#else
	const Int16U TableSize = DATA_TABLE_NV_SIZE;
#endif
	if(EPROMServiceCfg.ReadService)
		EPROMServiceCfg.ReadService(DT_EPROM_ADDRESS, (pInt16U)&DataTable[DATA_TABLE_NV_START], TableSize);
}
// ----------------------------------------

void DT_SaveNVPartToEPROM()
{
#ifdef USE_FLOAT_DT
	const Int16U TableSize = DATA_TABLE_NV_SIZE * 2;
#else
	const Int16U TableSize = DATA_TABLE_NV_SIZE;
#endif
	if(EPROMServiceCfg.WriteService)
		EPROMServiceCfg.WriteService(DT_EPROM_ADDRESS, (pInt16U)&DataTable[DATA_TABLE_NV_START], TableSize);
}
// ----------------------------------------

void DT_ResetNVPart(FUNC_SetDefaultValues SetFunc)
{
	Int16U i;
	
	for(i = DATA_TABLE_NV_START; i < (DATA_TABLE_NV_SIZE + DATA_TABLE_NV_START); ++i)
		DataTable[i] = 0;
	
	if(SetFunc)
		SetFunc();
}
// ----------------------------------------

void DT_ResetWRPart(FUNC_SetDefaultValues SetFunc)
{
	Int16U i;
	
	for(i = DATA_TABLE_WR_START; i < DATA_TABLE_WP_START; ++i)
		DataTable[i] = 0;
	
	if(SetFunc)
		SetFunc();
}
//-------------------------------------------

void DT_SaveFirmwareInfo(Int16U SlaveNID, Int16U MasterNID)
{
	if(DATA_TABLE_SIZE > REG_FWINFO_STR_BEGIN)
	{
		DataTable[REG_FWINFO_SLAVE_NID] = SlaveNID;
		DataTable[REG_FWINFO_MASTER_NID] = MasterNID;

		DataTable[REG_FWINFO_STR_LEN] = FWINF_Compose((pInt16U)(&DataTable[REG_FWINFO_STR_BEGIN]),
				(DATA_TABLE_SIZE - REG_FWINFO_STR_BEGIN) * 2);
	}
}
//------------------------------------------
