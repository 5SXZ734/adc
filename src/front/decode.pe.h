#pragma once

#include "PE.h"
#include "format.dwarf.h"

struct PE32_types_t : public ADCPE::IA32_types_t
{
	typedef class IA32FrontDC_t	FRONTDC;
};

struct PE64_types_t : public ADCPE::IA64_types_t
{
	typedef class  IA64FrontDC_t	FRONTDC;
};

enum PE_AttrIdEnum
{
	ATTR_RELOC = ATTR___DWARF_END,							//pointer to relocation
	ATTR_RES_OFFS,						//offset from resource segment (with high bit masked)
	ATTR_NET_METADATA_OFFS,				//offset from metadata (.NET)
	ATTR_NET_STRING_INDEX,
	ATTR_NET_BLOB_INDEX,
	ATTR_RIP,
	ATTR_COFF_STRING_TABLE_REF,
	ATTR_COFF_SYMBOL_VALUE_REF,
	ATTR_MANGLED_NAME_REF
};

enum {
	PE_META_EXPORT,//0: //map export symbol's ordinal to name's index
	PE_META_DWARF,
	PE__META_TOTAL
	//..
};

template <typename T_PE>
class PE_Dwarf_t : public dwarf::DebugInfo<T_PE>
{
	typedef typename T_PE::MyShdr	MyShdr;
public:
	PE_Dwarf_t(const T_PE& r)
		: dwarf::DebugInfo<T_PE>(r,
			fromSection(r, ".debug_info"),
			fromSection(r, ".debug_abbrev"),
			fromSection(r, ".debug_line"),
			fromSection(r, ".debug_str")
			)
	{
	}
	static DataSubSource_t fromSection(const T_PE& h, const char* name)
	{
		const MyShdr* psec(h.FindSection(name));
		if (!psec)
			throw dwarf::exception("no DWARF debug info in PE");
		return DataSubSource_t(h.dataSource(), psec->PointerToRawData, psec->SizeOfRawData);
	}
};

template <typename T>
class PE2_t : public ADCPE::PE_t<T>
{
	typedef ADCPE::PE_t<T> BASE;
 public:
	 typedef typename BASE::XWORD MyAddr;
	 typedef adcwin::IMAGE_SECTION_HEADER MyShdr;
	 typedef PE_Dwarf_t<PE2_t<T>>	MyDebugInfo;

	 PE2_t(const I_DataSourceBase& aRaw)
		 : BASE(aRaw)
	 {
	 }
 };



