#include "shared.h"
#include "format.dwarf.h"
#include "format.elf.h"

#define	TYPEID_SBYTE		OPTYP_INT8
#define	TYPEID_UBYTE		OPTYP_UINT8
#define	TYPEID_UWORD		TYPEID_UINT
#define	TYPEID_UHALF		TYPEID_USHORT
#define	TYPEID_OFF32		TYPEID_ULONG
#define	TYPEID_OFF64		TYPEID_ULONGLONG
#define	TYPEID_ADDR32		TYPEID_ULONG
#define	TYPEID_ADDR64		TYPEID_ULONGLONG

class DataStream2_t : public DataStream_t//for convenience
{
public:
	DataStream2_t(const I_Module &r) : DataStream_t(r, r.cpr()){}
};

BEGIN_DYNAMIC_TYPE(DWARF_ProgramStatementFilename)
{
	mr.declField("file_name", toAscii(mr));
	mr.declField("directory_index", dwarf::type_LEB128(mr));
	mr.declField("modification_time", dwarf::type_LEB128(mr));
	mr.declField("length", dwarf::type_LEB128(mr));
}
END_DYNAMIC_TYPE(DWARF_ProgramStatementFilename);

BEGIN_DYNAMIC_TYPE(DWARF_StringTable)
{
	for (int i(0);; i++)
	{
		DECLDATA(char, eol);
		if (!eol)
		{
			mr.skip(1);
			break;
		}
		mr.declField(nullptr, toAscii(mr));
	}
}
END_DYNAMIC_TYPE(DWARF_StringTable);

BEGIN_DYNAMIC_TYPE(DWARF_LineNumberStatement)
{
	for (;;)
	{
		DataStream2_t ds(mr);
		uint8_t opcode;
		ds.read(opcode);
		const char* pc("opcode");
		if (opcode == 0)
			pc = "extended_opcode";
		else if (opcode >= DW_LNS__special)
			pc = "special_opcode";
		mr.declField(pc, mr.enumOf(mr.type("DW_LNS"), TYPEID_UBYTE), ATTR_DECIMAL);
		switch (opcode)
		{
		case DW_LNS_copy:
			break;
		case DW_LNS_advance_pc:
			mr.declField("operand", dwarf::type_LEB128(mr));
			break;
		case DW_LNS_advance_line:
#ifdef _DEBUG
			uint32_t u;
			decode_ULEB128(u, ds);
#endif
			mr.declField("operand", dwarf::type_LEB128(mr));
			break;
		case DW_LNS_set_file:	//4
			mr.declField("operand", dwarf::type_LEB128(mr));
			break;
		case DW_LNS_set_column:
			mr.declField("operand", dwarf::type_LEB128(mr));
			break;
		case DW_LNS_negate_stmt:
			break;
		case DW_LNS_set_basic_block:
			break;
		case DW_LNS_const_add_pc:
			break;
		case DW_LNS_fixed_advance_pc:
			mr.declField("operand", mr.type(TYPEID_UHALF));//?
			break;
		case 0://extended
		{
			uint32_t length;
			::decode_ULEB128(length, ds);
			mr.declField("length", dwarf::type_LEB128(mr));
			uint8_t opcodex;
			ds.read(opcodex);
			mr.declField("opcodex", mr.enumOf(mr.type("DW_LNE"), TYPEID_UBYTE));
			switch (opcodex)
			{
			case DW_LNE_end_sequence:
				return;
			case DW_LNE_set_address:
				mr.declField("value", mr.type(((length - 1) < sizeof(uint64_t)) ? TYPEID_ADDR32 : TYPEID_ADDR64));//?
				break;
			case DW_LNE_define_file: break;
			}
			ds.forward(length);
			break;
		}
		default:
			//special opcode:
			//	adjusted_opcode = opcode - prologue.opcoode_base
			//	address_advance = adjusted_opcode / prologue.line_range
			//	line_increment = prologue.line_base + (adjusted_opcode % prologue.line_range)
			break;
		}
	}
}
END_DYNAMIC_TYPE(DWARF_LineNumberStatement);

//BEGIN_DYNAMIC_TYPE(DWARF_ProgramStatement)
void DWARF_Declare_ProgramStatement(I_Module &mr)
{
	DECLDATA(uint32_t, total_length);
	mr.declField("total_length", mr.type(TYPEID_UWORD));
	POSITION end(mr.cp() + total_length);
	mr.declField("version", mr.type(TYPEID_UHALF));
	DECLDATA(uint32_t, prologue_length);
	if (mr.NewScope(mr.declField("prologue")))
	{
		mr.declField("prologue_length", mr.type(TYPEID_UWORD));
		mr.declField("minimum_instruction_length", mr.type(TYPEID_UBYTE), ATTR_DECIMAL);
		mr.declField("default_is_stmt", mr.type(TYPEID_UBYTE));
		mr.declField("line_base", mr.type(TYPEID_SBYTE), ATTR_DECIMAL);
		mr.declField("line_range", mr.type(TYPEID_UBYTE), ATTR_DECIMAL);
		DECLDATA(uint8_t, opcode_base);
		mr.declField("opcode_base", mr.type(TYPEID_UBYTE), ATTR_DECIMAL);
		//mr.declField("standard_opcode_lengths", mr.arrayOf(mr.type(TYPEID_UBYTE), 2));
		char buf[32];
		for (uint8_t i(1); i < opcode_base; i++)
		{
			sprintf(buf, "opcode%d_length", (int)i);
			mr.declField(buf, dwarf::type_LEB128(mr), ATTR_DECIMAL);
		}
		mr.declField("directory_table", mr.type(_PFX("DWARF_StringTable")), ATTR_COLLAPSED);
		if (mr.NewScope(mr.declField("file_name_table", ATTR_COLLAPSED)))
		{
			for (;;)
			{
				DECLDATA(char, eol);
				if (!eol)
				{
					mr.skip(1);
					break;
				}
				mr.declField(nullptr, mr.type(_PFX("DWARF_ProgramStatementFilename")));
			}
			mr.Leave();
		}
		mr.Leave();
	}
	//???mr.skip(prologue_length + sizeof(prologue_length));
	while (mr.cp() < end)
	{
		mr.declField(nullptr, mr.type(_PFX("DWARF_LineNumberStatement")), ATTR_COLLAPSED);
	}
}
//END_DYNAMIC_TYPE(DWARF_ProgramStatement);

BEGIN_DYNAMIC_TYPE(DWARF_PubNamesHeader)
{
	mr.declField("length", mr.type(TYPEID_ULONG));
	mr.declField("version", mr.type(TYPEID_UHALF));
	mr.declField("unit_offset", mr.type(TYPEID_ULONG), (AttrIdEnum)ATTR_DWARF_INFO_OFFS);
	mr.declField("unit_size", mr.type(TYPEID_ULONG));
}
END_DYNAMIC_TYPE(DWARF_PubNamesHeader);

BEGIN_DYNAMIC_TYPE(DWARF_PubNamesTuple)
{
	mr.declField("offset", mr.type(TYPEID_ULONG), (AttrIdEnum)ATTR_DWARF_INFO_OFFS);
	mr.declField("name", toAscii(mr));
}
END_DYNAMIC_TYPE(DWARF_PubNamesTuple);

/*BEGIN_DYNAMIC_TYPE(DWARF_CompilationUnitHeader)
{
	DECLDATA(uint32_t, unit_length);
	bool b64bit(!(unit_length < 0xfffffff0));
	if (b64bit)
		mr.declField("DWARF64_indicator", mr.type(TYPEID_ULONG));
	mr.declField("unit_length", mr.type(b64bit ? TYPEID_ULONGLONG : TYPEID_ULONG));
	mr.declField("version", mr.type(TYPEID_UHALF));
	mr.declField("debug_abbrev_offset", mr.type(b64bit ? TYPEID_OFF64 : TYPEID_OFF32), (AttrIdEnum)ATTR_DWARF_ABBREV_OFFS);
	mr.declField("address_size", mr.type(TYPEID_UBYTE));
}
END_DYNAMIC_TYPE(DWARF_CompilationUnitHeader);*/

BEGIN_DYNAMIC_TYPE(DWARF_CompilationUnit)
{
	DECLDATA(uint32_t, unit_length);
	bool b64bit(!(unit_length < 0xfffffff0));
	if (b64bit)
		mr.declField("DWARF64_indicator", mr.type(TYPEID_ULONG));
	mr.declField("unit_length", mr.type(b64bit ? TYPEID_ULONGLONG : TYPEID_ULONG));
	POSITION beg(mr.cp());
	POSITION end(beg + unit_length);
	mr.declField("version", mr.type(TYPEID_UHALF));
	mr.declField("debug_abbrev_offset", mr.type(b64bit ? TYPEID_OFF64 : TYPEID_OFF32), (AttrIdEnum)ATTR_DWARF_ABBREV_OFFS);
	DECLDATA(uint8_t, address_size);
	mr.declField("address_size", mr.type(TYPEID_UBYTE));
	//mr.declField("header", mr.type(_PFX("DWARF_CompilationUnitHeader")));
	HNAME pcName(address_size == 8 ? _PFX("DWARF_DebugInformationEntry64") : _PFX("DWARF_DebugInformationEntry"));
	mr.declField("dies", mr.type(pcName));
	if (end > mr.cp())//error occured?
		mr.skip(end - mr.cp());
}
END_DYNAMIC_TYPE(DWARF_CompilationUnit);

BEGIN_DYNAMIC_TYPE(DWARF_Abbreviation)
{
	mr.declField("code", dwarf::type_LEB128(mr));//, (AttrIdEnum)ATTR_ELF_DEBUG_INFO_ABBREV_REF);
	mr.declField("tag", mr.enumOf(mr.type("DW_TAG"), dwarf::typeId_LEB128(mr)));
	mr.declField("children", mr.enumOf(mr.type("DW_CHILDREN"), TYPEID_BYTE));
	for (int i(0); i < 100; i++)//avoid endless loop at bad location
	{
		DECLDATA(short, end);
		if (end == 0)//both attr and form are 0
		{
			mr.declField("end", mr.type(TYPEID_WORD));
			return;
		}
		if (mr.NewScope(mr.declField("attrib_form")))
		{
			mr.declField("attrib", mr.enumOf(mr.type("DW_AT"), dwarf::typeId_LEB128(mr)));
			mr.declField("form", mr.enumOf(mr.type("DW_FORM"), dwarf::typeId_LEB128(mr)));
			mr.Leave();
		}
	}
	throw(-1);//possible mis-application
}
END_DYNAMIC_TYPE(DWARF_Abbreviation);

BEGIN_DYNAMIC_TYPE(DWARF_LocationExpression1)
{
	DECLDATA(uint8_t, len);
	mr.declField("length", mr.type(TYPEID_BYTE));
	//DECLDATA(uint8_t, op);
	if (len > 0)
	{
		mr.declField("op", mr.enumOf(mr.type("DW_OP"), TYPEID_BYTE));
		if (len > 1)
		{
			if (len > 2)
				mr.declField("data", mr.arrayOf(mr.type(TYPEID_BYTE), len - 1));
			else// if (len == 2)
				mr.declField("data", mr.type(TYPEID_BYTE));
		}
	}
}
END_DYNAMIC_TYPE(DWARF_LocationExpression1);

BEGIN_DYNAMIC_TYPE(DWARF_LocationList64)
{
	for (;;)
	{
		DECLDATA(uint64_t, start);
		mr.declField("start", mr.type(TYPEID_QWORD), ATTR_VA);
		DECLDATA(uint64_t, end);
		mr.declField("end", mr.type(TYPEID_QWORD), ATTR_VA);
		if (start == 0 && end == 0)
			break;
		DECLDATA(uint16_t, len);//DWARF2(7.7.2)
		mr.declField("length", mr.type(TYPEID_WORD));
		if (len > 0)
		{
			mr.declField("op", mr.enumOf(mr.type("DW_OP"), TYPEID_BYTE));
			if (len > 1)
			{
				if (len > 2)
					mr.declField("data", mr.arrayOf(mr.type(TYPEID_BYTE), len - 1));
				else// if (len == 2)
					mr.declField("data", mr.type(TYPEID_BYTE));
			}
		}
	}
}
END_DYNAMIC_TYPE(DWARF_LocationList64);

class SafeLoop_t//protection against an endless loop
{
	int limit;
	int i;
public:
	SafeLoop_t(int l)
		: limit(l),
		i(0)
	{
	}
	void operator++()
	{
		if (++i == limit)
			throw(-0x777);
	}
};

const char *DW_TAG_to_string(unsigned u)
{
	static const char *arr[] =
	{
		0,
		"array_type", //0x01,
		"class_type", //0x02,
		"entry_point", //0x03,
		"enumeration_type", //0x04,
		"formal_parameter", //0x05,
		0, 0,
		"imported_declaration", //0x08,
		0,
		"label", //0x0a,
		"lexical_block", //0x0b,
		0,
		"member", //0x0d,
		0,
		"pointer_type", //0x0f,
		"reference_type", //0x10,
		"compile_unit", //0x11,
		"string_type", //0x12,
		"structure_type", //0x13,
		"subroutine_type", //0x15,
		"typedef", //0x16,
		"union_type", //0x17,
		"unspecified_parameters", //0x18,
		"variant", //0x19,
		"common_block", //0x1a,
		"common_inclusion", //0x1b,
		"inheritance", //0x1c,
		"inlined_subroutine", //0x1d,
		"module", //0x1e,
		"ptr_to_member_type", //0x1f,
		"set_type", //0x20,
		"subrange_type", //0x21,
		"with_stmt", //0x22,
		"access_declaration", //0x23,
		"base_type", //0x24,
		"catch_block", //0x25,
		"const_type", //0x26,
		"constant", //0x27,
		"enumerator", //0x28,
		"file_type", //0x29,
		"friend", //0x2a,
		"namelist", //0x2b,
		"namelist_item", //0x2c,
		"packed_type", //0x2d,
		"subprogram", //0x2e,
		"template_type_parameter", //0x2f,
		"template_value_parameter", //0x30,
		"thrown_type", //0x31,
		"try_block", //0x32,
		"variant_part", //0x33,
		"variable", //0x34,
		"volatile_type", //0x35,
		"dwarf_procedure", //0x36,
		"restrict_type", //0x37,
		"interface_type", //0x38,
		"namespace", //0x39,
		"imported_module", //0x3a,
		"unspecified_type", //0x3b,
		"partial_unit", //0x3c,
		"imported_unit", //0x3d,
		0,
		"condition", //0x3f,
		"shared_type", //0x40,
		"type_unit", //0x41,
		"rvalue_reference_type", //0x42,
		"template_alias" //0x43
	};
	return (u < sizeof(arr) / sizeof(const char *) ? arr[u] : nullptr);
}

const char *DW_AT_to_string(unsigned u)
{
	static const char *arr[] =
	{
		0,
		"sibling", //0x01
		"location", //0x02
		"name", //0x03
		0, 0, 0, 0, 0,
		"ordering", //0x09
		0,
		"byte_size", //0x0b
		"bit_offset", //0x0c
		"bit_size", //0x0d
		0, 0,
		"stmt_list", //0x10
		"low_pc", //0x11
		"high_pc", //0x12
		"language", //0x13
		0,
		"discr", //0x15
		"discr_value", //0x16
		"visibility", //0x17
		"import", //0x18
		"string_length", //0x19
		"common_reference", //0x1a
		"comp_dir", //0x1b
		"const_value", //0x1c
		"containing_type", //0x1d
		"default_value", //0x1e
		0,
		"inline_", //0x20
		"is_optional", //0x21
		"lower_bound", //0x22
		0, 0,
		"producer", //0x25
		0,
		"prototyped", //0x27
		0, 0,
		"return_addr", //0x2a
		0,
		"start_scope", //0x2c
		0,
		"bit_stride", //0x2e
		"upper_bound", //0x2f
		0,
		"abstract_origin", //0x31
		"accessibility", //0x32
		"address_class", //0x33
		"artificial", //0x34
		"base_types", //0x35
		"calling_convention", //0x36
		"count", //0x37
		"data_member_location", //0x38
		"decl_column", //0x39
		"decl_file", //0x3a
		"decl_line", //0x3b
		"declaration", //0x3c
		"discr_list", //0x3d
		"encoding", //0x3e
		"external", //0x3f
		"frame_base", //0x40
		"friend_", //0x41
		"identifier_case", //0x42
		"macro_info", //0x43
		"namelist_item", //0x44
		"priority", //0x45
		"segment", //0x46
		"specification", //0x47
		"static_link", //0x48
		"type", //0x49
		"use_location", //0x4a
		"variable_parameter", //0x4b
		"virtuality", //0x4c
		"vtable_elem_location", //0x4d
		// DWARF 3
		"allocated", //0x4e
		"associated", //0x4f
		"data_location", //0x50
		"byte_stride", //0x51
		"entry_pc", //0x52
		"use_UTF8", //0x53
		"extension", //0x54
		"ranges", //0x55
		"trampoline", //0x56
		"call_column", //0x57
		"call_file", //0x58
		"call_line", //0x59
		"description", //0x5a
		"binary_scale", //0x5b
		"decimal_scale", //0x5c
		"small", //0x5d
		"decimal_sign", //0x5e
		"digit_count", //0x5f
		"picture_string", //0x60
		"mutable_", //0x61
		"threads_scaled", //0x62
		"explicit_", //0x63
		"object_pointer", //0x64
		"endianity", //0x65
		"elemental", //0x66
		"pure", //0x67
		"recursive", //0x68
		// DWARF 4
		"signature", //0x69
		"main_subprogram", //0x6a
		"data_bit_offset", //0x6b
		"const_expr", //0x6c
		"enum_class", //0x6d
		"linkage_name" //0x6e
	};
	return (u < sizeof(arr) / sizeof(const char *) ? arr[u] : nullptr);
}

static HTYPE typeFrom_DW_AT(I_Module &mr, DW_AT at, OpType_t eType)
{
	switch (at)
	{
	case DW_AT_language:
		return mr.enumOf(mr.type("DW_LANG"), eType);
	default:
		break;
	}
	return mr.type(eType);
}

class DynamicType_DWARF_DebugInformationEntry : public I_DynamicType
{
	bool mb64bit;
public:
	DynamicType_DWARF_DebugInformationEntry(bool b64bit) : mb64bit(b64bit) {}
	virtual const char* name() const { return mb64bit ? _PFX("DWARF_DebugInformationEntry64") : _PFX("DWARF_DebugInformationEntry"); }
	virtual void createz(I_Module& mr, unsigned long)
	{
		POSITION oBeg((POSITION)mr.cpr());
		DataStream2_t dsInfo(mr);
		dsInfo.seek(oBeg);
		uint32_t code;
		::decode_ULEB128(code, dsInfo);
		if (code == 0)
		{
			//mr.declField("__code_out", mr.type(TYPEID_BYTE));
	//		mr.skip(OPSZ_BYTE);
			return;
		}
		mr.declField("code", dwarf::type_LEB128(mr), (AttrIdEnum)ATTR_DWARF_ABBREV_CODE);

		int level(0);
		AuxDataBase_t meta(mr.aux());
		//dwarf::die2abbrev_t *pmap(meta.data<dwarf::die2abbrev_t>(dwarf::META_DWARF_DIE2ABBREV));
		dwarf::die2abbrev_t* pmap(meta.data<dwarf::die2abbrev_t>("DIE2ABBREV"));// dwarf::META_DWARF_DIE2ABBREV));
		const dwarf::die2abbrev_t::elt_t* e;
		if (!pmap || !(e = pmap->find(oBeg)))//not available during formatting
			return;

		DataStream2_t dsAbbr(mr);
		dsAbbr.seek(e->abbrev);

		uint32_t code2;
		::decode_ULEB128(code2, dsAbbr);
		if (code2 == code)
		{
			uint32_t tag;
			::decode_ULEB128(tag, dsAbbr);
			uint8_t children;
			dsAbbr.read(children);
			if (children)
				level++;
			for (SafeLoop_t z(100);; ++z)
			{
				uint16_t attr;
				::decode_ULEB128(attr, dsAbbr);
				uint8_t form;
				::decode_ULEB128(form, dsAbbr);
				if (attr == 0 && form == 0)
					break;
				const char* pname(DW_AT_to_string(attr));
				declDIEField(mr, (DW_AT)attr, (DW_FORM)form, pname);
			}
			if (children)
			{
				for (;;)
				{
					DECLDATA(uint8_t, more_children);
					if (!more_children)
						break;
					mr.declField("child_#", mr.type(name()));//recursion!
				}
				mr.skip(OPSZ_BYTE);//children chain end
			}
		}
	}
private:
	void declDIEField(I_Module& mr, DW_AT at, DW_FORM form, const char* pname)
	{
		HTYPE hType(nullptr);
		AttrIdEnum attr(ATTR_NULL);
		switch (form)
		{
		case DW_FORM_addr:
			hType = mr.type(mb64bit ? TYPEID_QWORD : TYPEID_DWORD);
			break;
		case DW_FORM_block2:
		{
			DECLDATA(uint16_t, len);
			mr.declField("length", mr.type(TYPEID_WORD));
			hType = mr.arrayOf(mr.type(TYPEID_BYTE), len);
			break;
		}
		case DW_FORM_block4:
			break;
		case DW_FORM_data2:
			hType = mr.type(TYPEID_WORD);
			break;
		case DW_FORM_data4:
			hType = mr.type(TYPEID_DWORD);
			break;
		case DW_FORM_data8:
			hType = mr.type(TYPEID_QWORD);
			break;
		case DW_FORM_string://0-terminated string embeded
			hType = toAscii(mr);
			//attr = ATTR_ ASCII;
			break;
		case DW_FORM_block:
			break;
		case DW_FORM_block1:
		{
			if (at == DW_AT_data_member_location)
			{
				mr.declField(pname, mr.type(_PFX("DWARF_LocationExpression1")));
				return;
			}
			DECLDATA(uint8_t, len);
			mr.declField("length", mr.type(TYPEID_BYTE));
			hType = mr.arrayOf(mr.type(TYPEID_BYTE), len);
			break;
		}
		case DW_FORM_data1:
			hType = typeFrom_DW_AT(mr, at, TYPEID_BYTE);
			break;
		case DW_FORM_flag:
			hType = mr.type(TYPEID_BYTE);
			break;
		case DW_FORM_sdata:
			hType = dwarf::type_LEB128(mr);
			break;
		case DW_FORM_strp:
			hType = mr.type(/*mb64bit ? TYPEID_QWORD : */TYPEID_DWORD);
			attr = (AttrIdEnum)ATTR_DWARF_STR_OFFS;
			break;
		case DW_FORM_udata:
			hType = dwarf::type_LEB128(mr);
			break;
		case DW_FORM_ref_addr:
			hType = mr.type(mb64bit ? TYPEID_QWORD : TYPEID_DWORD);
			break;
		case DW_FORM_ref1:
			hType = mr.type(TYPEID_BYTE);
			break;
		case DW_FORM_ref2:
			hType = mr.type(TYPEID_WORD);
			break;
		case DW_FORM_ref4:
			hType = mr.type(TYPEID_DWORD);
			break;
		case DW_FORM_ref8:
			hType = mr.type(TYPEID_QWORD);
			break;
			//DWARF4
		case DW_FORM_sec_offset:
			hType = mr.type(/*mb64bit ? TYPEID_QWORD : */TYPEID_DWORD);
			break;
		case DW_FORM_flag_present:
			//hType = mr.type(TYPEID_BYTE);
			//break;
			return;
		case DW_FORM_exprloc:
		{
			DECLDATA(uint8_t, len);
			mr.declField("length", mr.type(TYPEID_BYTE));
			hType = mr.arrayOf(mr.type(TYPEID_BYTE), len);
			break;
		}
		default:
			break;
		}
		if (attr == ATTR_NULL)
			attr = attr_from_DW_AT(at);
		mr.declField(pname, hType, attr);
	}
	static AttrIdEnum attr_from_DW_AT(DW_AT at)
	{
		switch (at)
		{
		case DW_AT_stmt_list:
			return (AttrIdEnum)ATTR_DWARF_LINE_OFFS;
		default:
			break;
		}
		return ATTR_NULL;
	}
};

DECLARE_DYNAMIC_TYPE1(DynamicType_DWARF_DebugInformationEntry, DWARF_DebugInformationEntry, false);
DECLARE_DYNAMIC_TYPE1(DynamicType_DWARF_DebugInformationEntry, DWARF_DebugInformationEntry64, true);



void DWARF_CreateStructures(I_SuperModule &mr)
{
	if (mr.NewScope("DW_TAG", SCOPE_ENUM))
	{
		mr.declEField("array_type", 0x01);
		mr.declEField("class_type", 0x02);
		mr.declEField("entry_point", 0x03);
		mr.declEField("enumeration_type", 0x04);
		mr.declEField("formal_parameter", 0x05);
		//mr.declEField("Reserved", 0x06);
		//mr.declEField("Reserved", 0x07);
		mr.declEField("imported_declaration", 0x08);
		//mr.declEField("Reserved", 0x09);
		mr.declEField("label", 0x0a);
		mr.declEField("lexical_block", 0x0b);
		//mr.declEField("Reserved", 0x0c);
		mr.declEField("member", 0x0d);
		//mr.declEField("Reserved", 0x0e);
		mr.declEField("pointer_type", 0x0f);
		mr.declEField("reference_type", 0x10);
		mr.declEField("compile_unit", 0x11);
		mr.declEField("string_type", 0x12);
		mr.declEField("structure_type", 0x13);
		//mr.declEField("Reserved", 0x14);
		mr.declEField("subroutine_type", 0x15);
		mr.declEField("typedef", 0x16);
		mr.declEField("union_type", 0x17);
		mr.declEField("unspecified_parameters", 0x18);
		mr.declEField("variant", 0x19);
		mr.declEField("common_block", 0x1a);
		mr.declEField("common_inclusion", 0x1b);
		mr.declEField("inheritance", 0x1c);
		mr.declEField("inlined_subroutine", 0x1d);
		mr.declEField("module", 0x1e);
		mr.declEField("ptr_to_member_type", 0x1f);
		mr.declEField("set_type", 0x20);
		mr.declEField("subrange_type", 0x21);
		mr.declEField("with_stmt", 0x22);
		mr.declEField("access_declaration", 0x23);
		mr.declEField("base_type", 0x24);
		mr.declEField("catch_block", 0x25);
		mr.declEField("const_type", 0x26);
		mr.declEField("constant", 0x27);
		mr.declEField("enumerator", 0x28);
		mr.declEField("file_type", 0x29);
		mr.declEField("friend", 0x2a);
		mr.declEField("namelist", 0x2b);
		mr.declEField("namelist_item", 0x2c);
		mr.declEField("packed_type", 0x2d);
		mr.declEField("subprogram", 0x2e);
		mr.declEField("template_type_parameter", 0x2f);
		mr.declEField("template_value_parameter", 0x30);
		mr.declEField("thrown_type", 0x31);
		mr.declEField("try_block", 0x32);
		mr.declEField("variant_part", 0x33);
		mr.declEField("variable", 0x34);
		mr.declEField("volatile_type", 0x35);
		mr.declEField("dwarf_procedure", 0x36);
		mr.declEField("restrict_type", 0x37);
		mr.declEField("interface_type", 0x38);
		mr.declEField("namespace", 0x39);
		mr.declEField("imported_module", 0x3a);
		mr.declEField("unspecified_type", 0x3b);
		mr.declEField("partial_unit", 0x3c);
		mr.declEField("imported_unit", 0x3d);
		mr.declEField("condition", 0x3f);
		mr.declEField("shared_type", 0x40);
		mr.declEField("type_unit", 0x41);
		mr.declEField("rvalue_reference_type", 0x42);
		mr.declEField("template_alias", 0x43);
		mr.declEField("coarray_type", 0x44);
		mr.declEField("generic_subrange", 0x45);
		mr.declEField("dynamic_type", 0x46);
		mr.declEField("atomic_type", 0x47);
		mr.declEField("call_site", 0x48);
		mr.declEField("call_site_parameter", 0x49);
		mr.declEField("skeleton_unit", 0x4a);
		mr.declEField("immutable_type", 0x4b);
		mr.declEField("lo_user", 0x4080);
		mr.declEField("hi_user", 0xffff);
		mr.Leave();
	}

	/////////////////////////////////////////////////////////////////////DW_AT
	if (mr.NewScope("DW_AT", SCOPE_ENUM))
	{
		mr.declEField("sibling", 0x01);// reference
		mr.declEField("location", 0x02);// exprloc, loclist
		mr.declEField("name", 0x03);// string
		//Reserved 0x04 not applicable
		//Reserved 0x05 not applicable

		//Reserved 0x06 not applicable
		//Reserved 0x07 not applicable
		//Reserved 0x08 not applicable
		mr.declEField("ordering", 0x09);// constant
		//Reserved 0x0a not applicable
		mr.declEField("byte_size", 0x0b);// constant, exprloc, reference
		//Reserved 0x0c2 constant, exprloc, reference
		mr.declEField("bit_size", 0x0d);// constant, exprloc, reference
		//Reserved 0x0e not applicable
		//Reserved 0x0f not applicable
		mr.declEField("stmt_list", 0x10);// lineptr
		mr.declEField("low_pc", 0x11);// address
		mr.declEField("high_pc", 0x12);// address, constant
		mr.declEField("language", 0x13);// constant
		//Reserved 0x14 not applicable
		mr.declEField("discr", 0x15);// reference
		mr.declEField("discr_value", 0x16);// constant
		mr.declEField("visibility", 0x17);// constant
		mr.declEField("import", 0x18);// reference
		mr.declEField("string_length", 0x19);// exprloc, loclist, reference
		mr.declEField("common_reference", 0x1a);// reference
		mr.declEField("comp_dir", 0x1b);// string
		mr.declEField("const_value", 0x1c);// block, constant, string
		mr.declEField("containing_type", 0x1d);// reference
		mr.declEField("default_value", 0x1e);// constant, reference, flag
		//Reserved 0x1f not applicable
		mr.declEField("inline", 0x20);// constant
		mr.declEField("is_optional", 0x21);// flag
		mr.declEField("lower_bound", 0x22);// constant, exprloc, reference
		//Reserved 0x23 not applicable

		//Reserved 0x24 not applicable
		mr.declEField("producer", 0x25);// string
		//Reserved 0x26 not applicable
		mr.declEField("prototyped", 0x27);// flag
		//Reserved 0x28 not applicable
		//Reserved 0x29 not applicable
		mr.declEField("return_addr", 0x2a);// exprloc, loclist
		//Reserved 0x2b not applicable
		mr.declEField("start_scope", 0x2c);// constant, rnglist
		//Reserved 0x2d not applicable
		mr.declEField("bit_stride", 0x2e);// constant, exprloc, reference
		mr.declEField("upper_bound", 0x2f);// constant, exprloc, reference
		//Reserved 0x30 not applicable
		mr.declEField("abstract_origin", 0x31);// reference
		mr.declEField("accessibility", 0x32);// constant
		mr.declEField("address_class", 0x33);// constant
		mr.declEField("artificial", 0x34);// flag
		mr.declEField("base_types", 0x35);// reference
		mr.declEField("calling_convention", 0x36);// constant
		mr.declEField("count", 0x37);// constant, exprloc, reference
		mr.declEField("data_member_location", 0x38);// constant, exprloc, loclist
		mr.declEField("decl_column", 0x39);// constant
		mr.declEField("decl_file", 0x3a);// constant
		mr.declEField("decl_line", 0x3b);// constant
		mr.declEField("declaration", 0x3c);// flag
		mr.declEField("discr_list", 0x3d);// block
		mr.declEField("encoding", 0x3e);// constant
		mr.declEField("external", 0x3f);// flag
		mr.declEField("frame_base", 0x40);// exprloc, loclist
		mr.declEField("friend", 0x41);// reference
		mr.declEField("identifier_case", 0x42);// constant

		//Reserved 0x433 macptr
		mr.declEField("namelist_item", 0x44);// reference
		mr.declEField("priority", 0x45);// reference
		mr.declEField("segment", 0x46);// exprloc, loclist
		mr.declEField("specification", 0x47);// reference
		mr.declEField("static_link", 0x48);// exprloc, loclist
		mr.declEField("type", 0x49);// reference
		mr.declEField("use_location", 0x4a);// exprloc, loclist
		mr.declEField("variable_parameter", 0x4b);// flag
		mr.declEField("virtuality", 0x4c);// constant
		mr.declEField("vtable_elem_location", 0x4d);// exprloc, loclist
		mr.declEField("allocated", 0x4e);// constant, exprloc, reference
		mr.declEField("associated", 0x4f);// constant, exprloc, reference
		mr.declEField("data_location", 0x50);// exprloc
		mr.declEField("byte_stride", 0x51);// constant, exprloc, reference
		mr.declEField("entry_pc", 0x52);// address, constant
		mr.declEField("use_UTF8", 0x53);// flag
		mr.declEField("extension", 0x54);// reference
		mr.declEField("ranges", 0x55);// rnglist
		mr.declEField("trampoline", 0x56);// address, flag, reference, string
		mr.declEField("call_column", 0x57);// constant
		mr.declEField("call_file", 0x58);// constant
		mr.declEField("call_line", 0x59);// constant
		mr.declEField("description", 0x5a);// string
		mr.declEField("binary_scale", 0x5b);// constant
		mr.declEField("decimal_scale", 0x5c);// constant
		mr.declEField("small", 0x5d);// reference
		mr.declEField("decimal_sign", 0x5e);// constant
		mr.declEField("digit_count", 0x5f);// constant
		mr.declEField("picture_string", 0x60);// string

		mr.declEField("mutable", 0x61);// flag
		mr.declEField("threads_scaled", 0x62);// flag
		mr.declEField("explicit", 0x63);// flag
		mr.declEField("object_pointer", 0x64);// reference
		mr.declEField("endianity", 0x65);// constant
		mr.declEField("elemental", 0x66);// flag
		mr.declEField("pure", 0x67);// flag
		mr.declEField("recursive", 0x68);// flag
		mr.declEField("signature", 0x69);// reference
		mr.declEField("main_subprogram", 0x6a);// flag
		mr.declEField("data_bit_offset", 0x6b);// constant
		mr.declEField("const_expr", 0x6c);// flag
		mr.declEField("enum_class", 0x6d);// flag
		mr.declEField("linkage_name", 0x6e);// string
		mr.declEField("string_length_bit_size", 0x6f);// constant
		mr.declEField("string_length_byte_size", 0x70);// constant
		mr.declEField("rank", 0x71);// constant, exprloc
		mr.declEField("str_offsets_base", 0x72);// stroffsetsptr
		mr.declEField("addr_base", 0x73);// addrptr
		mr.declEField("rnglists_base", 0x74);// rnglistsptr
		//Reserved 0x75 Unused
		mr.declEField("dwo_name", 0x76);// string
		mr.declEField("reference", 0x77);// flag
		mr.declEField("rvalue_reference", 0x78);// flag
		mr.declEField("macros", 0x79);// macptr
		mr.declEField("call_all_calls", 0x7a);// flag
		mr.declEField("call_all_source_calls", 0x7b);// flag
		mr.declEField("call_all_tail_calls", 0x7c);// flag
		mr.declEField("call_return_pc", 0x7d);// address
		mr.declEField("call_value", 0x7e);// exprloc
		mr.declEField("call_origin", 0x7f);// exprloc
		mr.declEField("call_parameter", 0x80);// reference

		mr.declEField("call_pc", 0x81);// address
		mr.declEField("call_tail_call", 0x82);// flag
		mr.declEField("call_target", 0x83);// exprloc
		mr.declEField("call_target_clobbered", 0x84);// exprloc
		mr.declEField("call_data_location", 0x85);// exprloc
		mr.declEField("call_data_value", 0x86);// exprloc
		mr.declEField("noreturn", 0x87);// flag
		mr.declEField("alignment", 0x88);// constant
		mr.declEField("export_symbols", 0x89);// flag
		mr.declEField("deleted", 0x8a);// flag
		mr.declEField("defaulted", 0x8b);// constant
		mr.declEField("loclists_base", 0x8c);// loclistsptr
		//mr.declEField("lo_user", 0x2000);
		// --MIPS--
		mr.declEField("MIPS_loop_begin", 0x2002);
		mr.declEField("MIPS_tail_loop_begin", 0x2003);
		mr.declEField("MIPS_epilog_begin", 0x2004);
		mr.declEField("MIPS_loop_unroll_factor", 0x2005);
		mr.declEField("MIPS_software_pipeline_depth", 0x2006);
		mr.declEField("MIPS_linkage_name", 0x2007);
		mr.declEField("MIPS_stride", 0x2008);
		mr.declEField("MIPS_abstract_name", 0x2009);
		mr.declEField("MIPS_clone_origin", 0x200a);
		mr.declEField("MIPS_has_inlines", 0x200b);
		mr.declEField("MIPS_stride_byte", 0x200c);
		mr.declEField("MIPS_stride_elem", 0x200d);
		mr.declEField("MIPS_ptr_dopetype", 0x200e);
		mr.declEField("MIPS_allocatable_dopetype", 0x200f);
		mr.declEField("MIPS_assumed_shape_dopetype", 0x2010);
		//mr.declEField("hi_user", 0x3fff);

		mr.Leave();
	}

	///////////////////////////////////////////////////////////////////DW_FORM
	if (mr.NewScope("DW_FORM", SCOPE_ENUM))
	{
		mr.declEField("addr", 0x01);// address
		//Reserved 0x02
		mr.declEField("block2", 0x03);// block
		mr.declEField("block4", 0x04);// block
		mr.declEField("data2", 0x05);// constant
		mr.declEField("data4", 0x06);// constant
		mr.declEField("data8", 0x07);// constant
		mr.declEField("string", 0x08);// string
		mr.declEField("block", 0x09);// block
		mr.declEField("block1", 0x0a);// block
		mr.declEField("data1", 0x0b);// constant
		mr.declEField("flag", 0x0c);// flag
		mr.declEField("sdata", 0x0d);// constant
		mr.declEField("strp", 0x0e);// string
		mr.declEField("udata", 0x0f);// constant
		mr.declEField("ref_addr", 0x10);// reference
		mr.declEField("ref1", 0x11);// reference
		mr.declEField("ref2", 0x12);// reference
		mr.declEField("ref4", 0x13);// reference
		mr.declEField("ref8", 0x14);// reference
		mr.declEField("ref_udata", 0x15);// reference
		mr.declEField("indirect", 0x16);// (see Section 7.5.3 on page 203)
		mr.declEField("sec_offset", 0x17);// addrptr, lineptr, loclist, loclistsptr, macptr, rnglist, rnglistsptr, stroffsetsptr
		mr.declEField("exprloc", 0x18);// exprloc
		mr.declEField("flag_present", 0x19);// flag
		mr.declEField("strx", 0x1a);// string
		mr.declEField("addrx", 0x1b);// address
		mr.declEField("ref_sup4", 0x1c);// reference
		mr.declEField("strp_sup", 0x1d);// string
		mr.declEField("data16", 0x1e);// constant
		mr.declEField("line_strp", 0x1f);// string
		mr.declEField("ref_sig8", 0x20);// reference
		mr.declEField("implicit_const", 0x21);// constant
		mr.declEField("loclistx", 0x22);// loclist
		mr.declEField("rnglistx", 0x23);// rnglist
		mr.declEField("ref_sup8", 0x24);// reference
		mr.declEField("strx1", 0x25);// string
		mr.declEField("strx2", 0x26);// string
		mr.declEField("strx3", 0x27);// string
		mr.declEField("strx4", 0x28);// string
		mr.declEField("addrx1", 0x29);// address
		mr.declEField("addrx2", 0x2a);// address
		mr.declEField("addrx3", 0x2b);// address
		mr.declEField("addrx4", 0x2c);// address
		mr.Leave();
	}

	if (mr.NewScope("DW_CHILDREN", SCOPE_ENUM))
	{
		mr.declEField("no", 0);
		mr.declEField("yes", 1);
		mr.Leave();
	}

	if (mr.NewScope("DW2_LINES_HEADER"))
	{
		mr.declField("total_length", mr.type(TYPEID_DWORD));
		mr.declField("version", mr.type(TYPEID_WORD));
		mr.declField("prologue_length", mr.type(TYPEID_DWORD));
		mr.declField("minimum_instruction_length", mr.type(TYPEID_UINT8));
		mr.declField("default_is_stmt", mr.type(TYPEID_UINT8));
		mr.declField("line_base", mr.type(TYPEID_INT8));
		mr.declField("line_range", mr.type(TYPEID_UINT8));
		mr.declField("opcode_base", mr.type(TYPEID_UINT8));
		mr.declField("standard_opcode_lengths", mr.type(TYPEID_UINT8));

		mr.Leave();
	}

	if (mr.NewScope("DW_LANG", SCOPE_ENUM))
	{
		mr.declEField("C89", 0x0001);
		mr.declEField("C", 0x0002);
		mr.declEField("Ada83", 0x0003);
		mr.declEField("C_plus_plus", 0x0004);
		mr.declEField("Cobol74", 0x0005);
		mr.declEField("Cobol85", 0x0006);
		mr.declEField("Fortran77", 0x0007);
		mr.declEField("Fortran90", 0x0008);
		mr.declEField("Pascal83", 0x0009);
		mr.declEField("Modula2", 0x000a);
		mr.declEField("Java", 0x000b);
		mr.declEField("C99", 0x000c);
		mr.declEField("Ada95", 0x000d);
		mr.declEField("Fortran95", 0x000e);
		mr.declEField("PLI", 0x000f);
		mr.declEField("ObjC", 0x0010);
		mr.declEField("ObjC_plus_plus", 0x0011);
		mr.declEField("UPC", 0x0012);
		mr.declEField("D", 0x0013);
		mr.declEField("Python", 0x0014);
		mr.declEField("OpenCL", 0x0015);
		mr.declEField("Go", 0x0016);
		mr.declEField("Modula3", 0x0017);
		mr.declEField("Haskell", 0x0018);
		mr.declEField("C_plus_plus_03", 0x0019);
		mr.declEField("C_plus_plus_11", 0x001a);
		mr.declEField("OCaml", 0x001b);
		mr.declEField("Rust", 0x001c);
		mr.declEField("C11", 0x001d);
		mr.declEField("Swift", 0x001e);
		mr.declEField("Julia", 0x001f);
		mr.declEField("Dylan", 0x0020);
		mr.declEField("C_plus_plus_14", 0x0021);
		mr.declEField("Fortran03", 0x0022);
		mr.declEField("Fortran08", 0x0023);
		mr.declEField("RenderScript", 0x0024);
		mr.declEField("BLISS", 0x0025);
		mr.declEField("lo_user", 0x8000);
		mr.declEField("hi_user", 0xffff);
		mr.Leave();
	};

	if (mr.NewScope("DW_LNS", SCOPE_ENUM))
	{
		mr.declEField("copy", 1);
		mr.declEField("advance_pc", 2);
		mr.declEField("advance_line", 3);
		mr.declEField("set_file", 4);
		mr.declEField("set_column", 5);
		mr.declEField("negate_stmt", 6);
		mr.declEField("set_basic_block", 7);
		mr.declEField("const_add_pc", 8);
		mr.declEField("fixed_advance_pc", 9);
		mr.Leave();
	}

	if (mr.NewScope("DW_LNE", SCOPE_ENUM))
	{
		mr.declEField("end_sequence", 1);
		mr.declEField("set_address", 2);
		mr.declEField("define_file", 3);
		mr.Leave();
	}

	if (mr.NewScope("DW_OP", SCOPE_ENUM))
	{
		mr.declEField("DW_OP_addr", 0x03);
		mr.declEField("DW_OP_deref", 0x06);
		mr.declEField("DW_OP_const1u", 0x08);
		mr.declEField("DW_OP_const1s", 0x09);
		mr.declEField("DW_OP_const2u", 0x0a);
		mr.declEField("DW_OP_const2s", 0x0b);
		mr.declEField("DW_OP_const4u", 0x0c);
		mr.declEField("DW_OP_const4s", 0x0d);
		mr.declEField("DW_OP_const8u", 0x0e);
		mr.declEField("DW_OP_const8s", 0x0f);
		mr.declEField("DW_OP_constu", 0x10);
		mr.declEField("DW_OP_consts", 0x11);
		mr.declEField("DW_OP_dup", 0x12);
		mr.declEField("DW_OP_drop", 0x13);
		mr.declEField("DW_OP_over", 0x14);
		mr.declEField("DW_OP_pick", 0x15);
		mr.declEField("DW_OP_swap", 0x16);
		mr.declEField("DW_OP_rot", 0x17);
		mr.declEField("DW_OP_xderef", 0x18);
		mr.declEField("DW_OP_abs", 0x19);
		mr.declEField("DW_OP_and", 0x1a);
		mr.declEField("DW_OP_div", 0x1b);
		mr.declEField("DW_OP_minus", 0x1c);
		mr.declEField("DW_OP_mod", 0x1d);
		mr.declEField("DW_OP_mul", 0x1e);
		mr.declEField("DW_OP_neg", 0x1f);
		mr.declEField("DW_OP_not", 0x20);
		mr.declEField("DW_OP_or", 0x21);
		mr.declEField("DW_OP_plus", 0x22);
		mr.declEField("DW_OP_plus_uconst", 0x23);
		mr.declEField("DW_OP_shl", 0x24);
		mr.declEField("DW_OP_shr", 0x25);
		mr.declEField("DW_OP_shra", 0x26);
		mr.declEField("DW_OP_xor", 0x27);
		mr.declEField("DW_OP_skip", 0x2f);
		mr.declEField("DW_OP_bra", 0x28);
		mr.declEField("DW_OP_eq", 0x29);
		mr.declEField("DW_OP_ge", 0x2a);
		mr.declEField("DW_OP_gt", 0x2b);
		mr.declEField("DW_OP_le", 0x2c);
		mr.declEField("DW_OP_lt", 0x2d);
		mr.declEField("DW_OP_ne", 0x2e);
		//lit
		mr.declEField("DW_OP_lit0", 0x30);
		mr.declEField("DW_OP_lit1", 0x31);
		mr.declEField("DW_OP_lit2", 0x32);
		mr.declEField("DW_OP_lit3", 0x33);
		mr.declEField("DW_OP_lit4", 0x34);
		mr.declEField("DW_OP_lit5", 0x35);
		mr.declEField("DW_OP_lit6", 0x36);
		mr.declEField("DW_OP_lit7", 0x37);
		mr.declEField("DW_OP_lit8", 0x38);
		mr.declEField("DW_OP_lit9", 0x39);
		mr.declEField("DW_OP_lit10", 0x3a);
		mr.declEField("DW_OP_lit11", 0x3b);
		mr.declEField("DW_OP_lit12", 0x3c);
		mr.declEField("DW_OP_lit13", 0x3d);
		mr.declEField("DW_OP_lit14", 0x3e);
		mr.declEField("DW_OP_lit15", 0x3f);
		mr.declEField("DW_OP_lit16", 0x40);
		mr.declEField("DW_OP_lit17", 0x41);
		mr.declEField("DW_OP_lit18", 0x42);
		mr.declEField("DW_OP_lit19", 0x43);
		mr.declEField("DW_OP_lit20", 0x44);
		mr.declEField("DW_OP_lit21", 0x45);
		mr.declEField("DW_OP_lit22", 0x46);
		mr.declEField("DW_OP_lit23", 0x47);
		mr.declEField("DW_OP_lit24", 0x48);
		mr.declEField("DW_OP_lit25", 0x49);
		mr.declEField("DW_OP_lit26", 0x4a);
		mr.declEField("DW_OP_lit27", 0x4b);
		mr.declEField("DW_OP_lit28", 0x4c);
		mr.declEField("DW_OP_lit29", 0x4d);
		mr.declEField("DW_OP_lit30", 0x4e);
		mr.declEField("DW_OP_lit31", 0x4f);
		//reg
		mr.declEField("DW_OP_reg0", 0x50);
		mr.declEField("DW_OP_reg1", 0x51);
		mr.declEField("DW_OP_reg2", 0x52);
		mr.declEField("DW_OP_reg3", 0x53);
		mr.declEField("DW_OP_reg4", 0x54);
		mr.declEField("DW_OP_reg5", 0x55);
		mr.declEField("DW_OP_reg6", 0x56);
		mr.declEField("DW_OP_reg7", 0x57);
		mr.declEField("DW_OP_reg8", 0x58);
		mr.declEField("DW_OP_reg9", 0x59);
		mr.declEField("DW_OP_reg10", 0x5a);
		mr.declEField("DW_OP_reg11", 0x5b);
		mr.declEField("DW_OP_reg12", 0x5c);
		mr.declEField("DW_OP_reg13", 0x5d);
		mr.declEField("DW_OP_reg14", 0x5e);
		mr.declEField("DW_OP_reg15", 0x5f);
		mr.declEField("DW_OP_reg16", 0x60);
		mr.declEField("DW_OP_reg17", 0x61);
		mr.declEField("DW_OP_reg18", 0x62);
		mr.declEField("DW_OP_reg19", 0x63);
		mr.declEField("DW_OP_reg20", 0x64);
		mr.declEField("DW_OP_reg21", 0x65);
		mr.declEField("DW_OP_reg22", 0x66);
		mr.declEField("DW_OP_reg23", 0x67);
		mr.declEField("DW_OP_reg24", 0x68);
		mr.declEField("DW_OP_reg25", 0x69);
		mr.declEField("DW_OP_reg26", 0x6a);
		mr.declEField("DW_OP_reg27", 0x6b);
		mr.declEField("DW_OP_reg28", 0x6c);
		mr.declEField("DW_OP_reg29", 0x6d);
		mr.declEField("DW_OP_reg30", 0x6e);
		mr.declEField("DW_OP_reg31", 0x6f);
		//breg
		mr.declEField("DW_OP_breg0", 0x70);
		mr.declEField("DW_OP_breg1", 0x71);
		mr.declEField("DW_OP_breg2", 0x72);
		mr.declEField("DW_OP_breg3", 0x73);
		mr.declEField("DW_OP_breg4", 0x74);
		mr.declEField("DW_OP_breg5", 0x75);
		mr.declEField("DW_OP_breg6", 0x76);
		mr.declEField("DW_OP_breg7", 0x77);
		mr.declEField("DW_OP_breg8", 0x78);
		mr.declEField("DW_OP_breg9", 0x79);
		mr.declEField("DW_OP_breg10", 0x7a);
		mr.declEField("DW_OP_breg11", 0x7b);
		mr.declEField("DW_OP_breg12", 0x7c);
		mr.declEField("DW_OP_breg13", 0x7d);
		mr.declEField("DW_OP_breg14", 0x7e);
		mr.declEField("DW_OP_breg15", 0x7f);
		mr.declEField("DW_OP_breg16", 0x80);
		mr.declEField("DW_OP_breg17", 0x81);
		mr.declEField("DW_OP_breg18", 0x82);
		mr.declEField("DW_OP_breg19", 0x83);
		mr.declEField("DW_OP_breg20", 0x84);
		mr.declEField("DW_OP_breg21", 0x85);
		mr.declEField("DW_OP_breg22", 0x86);
		mr.declEField("DW_OP_breg23", 0x87);
		mr.declEField("DW_OP_breg24", 0x88);
		mr.declEField("DW_OP_breg25", 0x89);
		mr.declEField("DW_OP_breg26", 0x8a);
		mr.declEField("DW_OP_breg27", 0x8b);
		mr.declEField("DW_OP_breg28", 0x8c);
		mr.declEField("DW_OP_breg29", 0x8d);
		mr.declEField("DW_OP_breg30", 0x8e);
		mr.declEField("DW_OP_breg31", 0x8f);
		//
		mr.declEField("DW_OP_regx", 0x90);
		mr.declEField("DW_OP_fbreg", 0x91);
		mr.declEField("DW_OP_bregx", 0x92);
		mr.declEField("DW_OP_piece", 0x93);
		mr.declEField("DW_OP_deref_size", 0x94);
		mr.declEField("DW_OP_xderef_size", 0x95);
		mr.declEField("DW_OP_nop", 0x96);
		mr.declEField("DW_OP_lo_user", 0xe0);
		mr.declEField("DW_OP_hi_user", 0xff);
		mr.Leave();
	}

	if (mr.NewScope("DW_ATE", SCOPE_ENUM))
	{
		mr.declEField("address", 0x1);
		mr.declEField("boolean", 0x2);
		mr.declEField("complex_float", 0x3);
		mr.declEField("float", 0x4);
		mr.declEField("signed", 0x5);
		mr.declEField("signed_char", 0x6);
		mr.declEField("unsigned", 0x7);
		mr.declEField("unsigned_char", 0x8);
		//mr.declEField("lo_user", 0x80");
		//mr.declEField("hi_user", 0xff");
		mr.Leave();
	}

	mr.DeclareContextDependentType(_PFX("DWARF_CompilationUnit"));
	//mr.DeclareContextDependentType(_PFX("DWARF_CompilationUnitHeader"));
	mr.DeclareContextDependentType(_PFX("DWARF_Abbreviation"));
	mr.DeclareContextDependentType(_PFX("DWARF_DebugInformationEntry"));
	mr.DeclareContextDependentType(_PFX("DWARF_DebugInformationEntry64"));
	mr.DeclareContextDependentType(_PFX("DWARF_PubNamesHeader"));
	mr.DeclareContextDependentType(_PFX("DWARF_PubNamesTuple"));
	//mr.DeclareContextDependentType(_PFX("DWARF_ProgramStatement"));
	mr.DeclareContextDependentType(_PFX("DWARF_ProgramStatementFilename"));
	mr.DeclareContextDependentType(_PFX("DWARF_LineNumberStatement"));
	mr.DeclareContextDependentType(_PFX("DWARF_StringTable"));
	mr.DeclareContextDependentType(_PFX("DWARF_LocationExpression1"));
	mr.DeclareContextDependentType(_PFX("DWARF_LocationList64"));
}

