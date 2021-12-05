#pragma once

#include "shared/front.h"

I_Front *CreateFE_PDB(const I_DataSourceBase *);
void PDB_RegisterFormatters(I_ModuleEx &);
void PDB20_RegisterFormatters(I_ModuleEx &);


#define TYPEID_ULITTLE32	OPSZ_DWORD
#define TYPEID_CV_UOFF32	OPTYP_UINT32
#define TYPEID_CV_TYP		OPTYP_UINT32
#define TYPEID_CV_PROP		OPTYP_UINT16
#define TYPEID_CV_ITEMID	TYPEID_CV_TYP
#define TYPEID_CV_TKN		OPTYP_UINT32	//CV_tkn_t

enum MyAttrIdEnum
{
	ATTR_PDB_BLOCK_ID = ATTR___MISC_BEGIN,
	ATTR_PDB_NAME,
	ATTR_PDB_FILEPTR,
	ATTR_PDB_TYPEOFF,//reference to the type (in TPI stream)
	ATTR_PDB_ITYPEOFF,
	ATTR_PDB_BLK,//reference to the block (in PDB file)
	ATTR_OFFS_NAMES_HDR,//offset from the end of the NAMES stream's header
	ATTR_STREAM_INDEX,
	ATTR_MODULE_INDEX,	//as described by ModInfo substream in DBI (1-based)
	ATTR_SYM_OFF_P1,//offset into SYMREC stream plus 1
	ATTR_GSI_OFF_MH,//offset into GSI stream minus GSI header
	ATTR_MOD_FILECHKSMS,//offset from beginning of file checksums description array in module stream

	ATTR_PDB20_BLK,
	__ATTR2_LAST
};

void PDB20_CreateStructures(I_SuperModule &);
void PDB20_DeclareDynamicTypes(I_SuperModule &);

void PDB70_CreateStructures(I_SuperModule &);
//void PDB_CreateStructures_TPI(I_Module &);
//void PDB_CreateStructures_MOD(I_Module &);
void PDB70_DeclareDynamicTypes(I_SuperModule &);


#define STREAM_INDEX_TO_MODULE_ID(i)	int(i + 0x100)
#define MODULE_ID_TO_STREAM_INDEX(i)	int(i - 0x100)

#define OTHER_MODULE(i)	(AKindEnum)(I_Front::AKIND_OTHER | STREAM_INDEX_TO_MODULE_ID(i));

///////////////////////////////////////////// T_PDB_base
class T_PDB_base
{
protected:
	I_SuperModule &mr;
public:
	T_PDB_base(I_SuperModule &r) : mr(r){}
};


