#pragma once

#include <algorithm>
#include "format.dwarf.h"
#include "ELF.h"

void ELF_RegisterFormatters(I_ModuleEx &);
I_Front *CreateELF32Front(const I_DataSourceBase *);
I_Front *CreateELF64Front(const I_DataSourceBase *);


enum ELF_AttrIdEnum
{
	ATTR_ELF_STRING_INDEX0 = ATTR___DWARF_END,							//pointer to relocation
	ATTR_ELF_OFF_LNK_SEC_VIA_RVA,		//offset in linked section (via current position's RVA)
	ATTR_ELF_OFF_LNK_SEC_VIA_FP,		//offset in linked section (via current position's position in file)
	ATTR_ELF_STRING_INDEX_DYNSYM,		//position in a section => an offset in linked section (via SH)
	ATTR_ELF_DYN_TAG_PTR,
	ATTR_ELF_DYNSYM_INDEX,
	ATTR_ELF_SECTION_HEADER
};

class ELF_Strucs_t
{
//protected:
	//I_SuperModule& mr;
	bool	m64bit;
protected:
	
	const OpType_t _Addr = OPSZ_DWORD;
	const OpType_t _Addr64 = OPSZ_QWORD;
	const OpType_t _Off = OPSZ_DWORD;
	const OpType_t _Off64 = OPSZ_QWORD;
	const OpType_t _Half = OPTYP_UINT16;
	const OpType_t _SWord = OPTYP_INT32;
	const OpType_t _Word = OPTYP_UINT32;
	const OpType_t _XWord = OPTYP_UINT64;
	const OpType_t _SXWord = OPTYP_INT64;
	const OpType_t _uchar = OPSZ_BYTE;
	const OpType_t _char = OPTYP_CHAR8;

protected:
	ELF_Strucs_t(bool b64bit)//I_SuperModule& r)
		: //mr(r),
		m64bit(b64bit)
	{
	}
	//void createStructures();
	void createElfStructures(I_Module&);
	void create32bitStructures(I_Module&);
	void create64bitStructures(I_Module&);
};









