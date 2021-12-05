#pragma once

#include "shared/defs.h"
#include "interface/IADCMain.h"

enum DW_TAG
{
	DW_TAG_array_type = 0x01,
	DW_TAG_class_type = 0x02,
	DW_TAG_entry_point = 0x03,
	DW_TAG_enumeration_type = 0x04,
	DW_TAG_formal_parameter = 0x05,
	DW_TAG_imported_declaration = 0x08,
	DW_TAG_label = 0x0a,
	DW_TAG_lexical_block = 0x0b,
	DW_TAG_member = 0x0d,
	DW_TAG_pointer_type = 0x0f,
	DW_TAG_reference_type = 0x10,
	DW_TAG_compile_unit = 0x11,
	DW_TAG_string_type = 0x12,
	DW_TAG_structure_type = 0x13,
	DW_TAG_subroutine_type = 0x15,
	DW_TAG_typedef = 0x16,

	DW_TAG_union_type = 0x17,
	DW_TAG_unspecified_parameters = 0x18,
	DW_TAG_variant = 0x19,
	DW_TAG_common_block = 0x1a,
	DW_TAG_common_inclusion = 0x1b,
	DW_TAG_inheritance = 0x1c,
	DW_TAG_inlined_subroutine = 0x1d,
	DW_TAG_module = 0x1e,
	DW_TAG_ptr_to_member_type = 0x1f,
	DW_TAG_set_type = 0x20,
	DW_TAG_subrange_type = 0x21,
	DW_TAG_with_stmt = 0x22,
	DW_TAG_access_declaration = 0x23,
	DW_TAG_base_type = 0x24,
	DW_TAG_catch_block = 0x25,
	DW_TAG_const_type = 0x26,
	DW_TAG_constant = 0x27,
	DW_TAG_enumerator = 0x28,
	DW_TAG_file_type = 0x29,
	DW_TAG_friend_ = 0x2a,

	DW_TAG_namelist = 0x2b,
	DW_TAG_namelist_item = 0x2c,
	DW_TAG_packed_type = 0x2d,
	DW_TAG_subprogram = 0x2e,
	DW_TAG_template_type_parameter = 0x2f,
	DW_TAG_template_value_parameter = 0x30,
	DW_TAG_thrown_type = 0x31,
	DW_TAG_try_block = 0x32,
	DW_TAG_variant_part = 0x33,
	DW_TAG_variable = 0x34,
	DW_TAG_volatile_type = 0x35,
	DW_TAG_dwarf_procedure = 0x36,
	DW_TAG_restrict_type = 0x37,
	DW_TAG_interface_type = 0x38,
	DW_TAG_namespace = 0x39,
	DW_TAG_imported_module = 0x3a,
	DW_TAG_unspecified_type = 0x3b,
	DW_TAG_partial_unit = 0x3c,
	DW_TAG_imported_unit = 0x3d,
	DW_TAG_condition = 0x3f,

	DW_TAG_shared_type = 0x40,
	DW_TAG_type_unit = 0x41,
	DW_TAG_rvalue_reference_type = 0x42,
	DW_TAG_template_alias = 0x43,
	DW_TAG_lo_user = 0x4080,
	DW_TAG_hi_user = 0xffff,
};

enum DW_AT
{
	DW_AT_nil,
	DW_AT_sibling = 0x01, // reference
	DW_AT_location = 0x02, // exprloc, loclistptr
	DW_AT_name = 0x03, // string
	DW_AT_ordering = 0x09, // constant
	DW_AT_byte_size = 0x0b, // constant, exprloc, reference
	DW_AT_bit_offset = 0x0c, // constant, exprloc, reference
	DW_AT_bit_size = 0x0d, // constant, exprloc, reference
	DW_AT_stmt_list = 0x10, // lineptr
	DW_AT_low_pc = 0x11, // address
	DW_AT_high_pc = 0x12, // address, constant
	DW_AT_language = 0x13, // constant
	DW_AT_discr = 0x15, // reference
	DW_AT_discr_value = 0x16, // constant
	DW_AT_visibility = 0x17, // constant
	DW_AT_import = 0x18, // reference
	DW_AT_string_length = 0x19, // exprloc, loclistptr
	DW_AT_common_reference = 0x1a, // reference
	DW_AT_comp_dir = 0x1b, // string
	DW_AT_const_value = 0x1c, // block, constant, string

	DW_AT_containing_type = 0x1d, // reference
	DW_AT_default_value = 0x1e, // reference
	DW_AT_inline_ = 0x20, // constant
	DW_AT_is_optional = 0x21, // flag
	DW_AT_lower_bound = 0x22, // constant, exprloc, reference
	DW_AT_producer = 0x25, // string
	DW_AT_prototyped = 0x27, // flag
	DW_AT_return_addr = 0x2a, // exprloc, loclistptr
	DW_AT_start_scope = 0x2c, // constant, rangelistptr
	DW_AT_bit_stride = 0x2e, // constant, exprloc, reference
	DW_AT_upper_bound = 0x2f, // constant, exprloc, reference
	DW_AT_abstract_origin = 0x31, // reference
	DW_AT_accessibility = 0x32, // constant
	DW_AT_address_class = 0x33, // constant
	DW_AT_artificial = 0x34, // flag
	DW_AT_base_types = 0x35, // reference
	DW_AT_calling_convention = 0x36, // constant
	DW_AT_count = 0x37, // constant, exprloc, reference
	DW_AT_data_member_location = 0x38, // constant, exprloc, loclistptr
	DW_AT_decl_column = 0x39, // constant

	DW_AT_decl_file = 0x3a, // constant
	DW_AT_decl_line = 0x3b, // constant
	DW_AT_declaration = 0x3c, // flag
	DW_AT_discr_list = 0x3d, // block
	DW_AT_encoding = 0x3e, // constant
	DW_AT_external = 0x3f, // flag
	DW_AT_frame_base = 0x40, // exprloc, loclistptr
	DW_AT_friend_ = 0x41, // reference
	DW_AT_identifier_case = 0x42, // constant
	DW_AT_macro_info = 0x43, // macptr
	DW_AT_namelist_item = 0x44, // reference
	DW_AT_priority = 0x45, // reference
	DW_AT_segment = 0x46, // exprloc, loclistptr
	DW_AT_specification = 0x47, // reference
	DW_AT_static_link = 0x48, // exprloc, loclistptr
	DW_AT_type = 0x49, // reference
	DW_AT_use_location = 0x4a, // exprloc, loclistptr
	DW_AT_variable_parameter = 0x4b, // flag
	DW_AT_virtuality = 0x4c, // constant
	DW_AT_vtable_elem_location = 0x4d, // exprloc, loclistptr

	// DWARF 3
	DW_AT_allocated = 0x4e, // constant, exprloc, reference
	DW_AT_associated = 0x4f, // constant, exprloc, reference
	DW_AT_data_location = 0x50, // exprloc
	DW_AT_byte_stride = 0x51, // constant, exprloc, reference
	DW_AT_entry_pc = 0x52, // address
	DW_AT_use_UTF8 = 0x53, // flag
	DW_AT_extension = 0x54, // reference
	DW_AT_ranges = 0x55, // rangelistptr
	DW_AT_trampoline = 0x56, // address, flag, reference, string
	DW_AT_call_column = 0x57, // constant
	DW_AT_call_file = 0x58, // constant
	DW_AT_call_line = 0x59, // constant
	DW_AT_description = 0x5a, // string
	DW_AT_binary_scale = 0x5b, // constant
	DW_AT_decimal_scale = 0x5c, // constant
	DW_AT_small = 0x5d, // reference
	DW_AT_decimal_sign = 0x5e, // constant
	DW_AT_digit_count = 0x5f, // constant
	DW_AT_picture_string = 0x60, // string
	DW_AT_mutable_ = 0x61, // flag

	DW_AT_threads_scaled = 0x62, // flag
	DW_AT_explicit_ = 0x63, // flag
	DW_AT_object_pointer = 0x64, // reference
	DW_AT_endianity = 0x65, // constant
	DW_AT_elemental = 0x66, // flag
	DW_AT_pure = 0x67, // flag
	DW_AT_recursive = 0x68, // flag

	// DWARF 4
	DW_AT_signature = 0x69, // reference
	DW_AT_main_subprogram = 0x6a, // flag
	DW_AT_data_bit_offset = 0x6b, // constant
	DW_AT_const_expr = 0x6c, // flag
	DW_AT_enum_class = 0x6d, // flag
	DW_AT_linkage_name = 0x6e, // string

	DW_AT_lo_user = 0x2000,
	//MIPS extentions
	DW_AT_MIPS_loop_begin = 0x2002,
	DW_AT_MIPS_tail_loop_begin = 0x2003,
	DW_AT_MIPS_epilog_begin = 0x2004,
	DW_AT_MIPS_loop_unroll_factor = 0x2005,
	DW_AT_MIPS_software_pipeline_depth = 0x2006,
	DW_AT_MIPS_linkage_name = 0x2007,
	DW_AT_MIPS_stride = 0x2008,
	DW_AT_MIPS_abstract_name = 0x2009,
	DW_AT_MIPS_clone_origin = 0x200a,
	DW_AT_MIPS_has_inlines = 0x200b,
	DW_AT_MIPS_stride_byte = 0x200c,
	DW_AT_MIPS_stride_elem = 0x200d,
	DW_AT_MIPS_ptr_dopetype = 0x200e,
	DW_AT_MIPS_allocatable_dopetype = 0x200f,
	DW_AT_MIPS_assumed_shape_dopetype = 0x2010,

	DW_AT_hi_user = 0x3fff,
};

enum DW_FORM
{
	DW_FORM_nil,
	DW_FORM_addr = 0x01,    // address
	DW_FORM_block2 = 0x03,    // block
	DW_FORM_block4 = 0x04,    // block
	DW_FORM_data2 = 0x05,    // constant
	DW_FORM_data4 = 0x06,    // constant
	DW_FORM_data8 = 0x07,    // constant
	DW_FORM_string = 0x08,    // string
	DW_FORM_block = 0x09,    // block
	DW_FORM_block1 = 0x0a,    // block
	DW_FORM_data1 = 0x0b,    // constant
	DW_FORM_flag = 0x0c,    // flag
	DW_FORM_sdata = 0x0d,    // constant
	DW_FORM_strp = 0x0e,    // string
	DW_FORM_udata = 0x0f,    // constant
	DW_FORM_ref_addr = 0x10,    // reference
	DW_FORM_ref1 = 0x11,    // reference
	DW_FORM_ref2 = 0x12,    // reference
	DW_FORM_ref4 = 0x13,    // reference
	DW_FORM_ref8 = 0x14,    // reference

	DW_FORM_ref_udata = 0x15,    // reference
	DW_FORM_indirect = 0x16,    // (Section 7.5.3)

	// DWARF 4
	DW_FORM_sec_offset = 0x17,    // lineptr, loclistptr, macptr, rangelistptr
	DW_FORM_exprloc = 0x18,    // exprloc
	DW_FORM_flag_present = 0x19,    // flag
	DW_FORM_ref_sig8 = 0x20,    // reference
};

enum DW_LANG
{
	DW_LANG_C89 = 0x0001,
	DW_LANG_C = 0x0002,
	DW_LANG_Ada83 = 0x0003,
	DW_LANG_C_plus_plus = 0x0004,
	DW_LANG_Cobol74 = 0x0005,
	DW_LANG_Cobol85 = 0x0006,
	DW_LANG_Fortran77 = 0x0007,
	DW_LANG_Fortran90 = 0x0008,
	DW_LANG_Pascal83 = 0x0009,
	DW_LANG_Modula2 = 0x000a,
	DW_LANG_Java = 0x000b,
	DW_LANG_C99 = 0x000c,
	DW_LANG_Ada95 = 0x000d,
	DW_LANG_Fortran95 = 0x000e,
	DW_LANG_PLI = 0x000f,
	DW_LANG_ObjC = 0x0010,
	DW_LANG_ObjC_plus_plus = 0x0011,
	DW_LANG_UPC = 0x0012,
	DW_LANG_D = 0x0013,
	DW_LANG_Python = 0x0014,
	DW_LANG_OpenCL = 0x0015,
	DW_LANG_Go = 0x0016,
	DW_LANG_Modula3 = 0x0017,
	DW_LANG_Haskell = 0x0018,
	DW_LANG_C_plus_plus_03 = 0x0019,
	DW_LANG_C_plus_plus_11 = 0x001a,
	DW_LANG_OCaml = 0x001b,
	DW_LANG_Rust = 0x001c,
	DW_LANG_C11 = 0x001d,
	DW_LANG_Swift = 0x001e,
	DW_LANG_Julia = 0x001f,
	DW_LANG_Dylan = 0x0020,
	DW_LANG_C_plus_plus_14 = 0x0021,
	DW_LANG_Fortran03 = 0x0022,
	DW_LANG_Fortran08 = 0x0023,
	DW_LANG_RenderScript = 0x0024,
	DW_LANG_BLISS = 0x0025,
	DW_LANG_lo_user = 0x8000,
	DW_LANG_hi_user = 0xffff
};

enum DW_LNS
{
	DW_LNS_copy = 1,
	DW_LNS_advance_pc = 2,
	DW_LNS_advance_line = 3,
	DW_LNS_set_file = 4,
	DW_LNS_set_column = 5,
	DW_LNS_negate_stmt = 6,
	DW_LNS_set_basic_block = 7,
	DW_LNS_const_add_pc = 8,
	DW_LNS_fixed_advance_pc = 9,
	DW_LNS__special
};

enum DW_LNE
{
	DW_LNE_end_sequence = 1,
	DW_LNE_set_address = 2,
	DW_LNE_define_file = 3
};

enum DW_OP
{
	DW_OP_addr = 0x03,
	DW_OP_deref = 0x06,
	DW_OP_const1u = 0x08,
	DW_OP_const1s = 0x09,
	DW_OP_const2u = 0x0a,
	DW_OP_const2s = 0x0b,
	DW_OP_const4u = 0x0c,
	DW_OP_const4s = 0x0d,
	DW_OP_const8u = 0x0e,
	DW_OP_const8s = 0x0f,
	DW_OP_constu = 0x10,
	DW_OP_consts = 0x11,
	DW_OP_dup = 0x12,
	DW_OP_drop = 0x13,
	DW_OP_over = 0x14,
	DW_OP_pick = 0x15,
	DW_OP_swap = 0x16,
	DW_OP_rot = 0x17,
	DW_OP_xderef = 0x18,
	DW_OP_abs = 0x19,
	DW_OP_and = 0x1a,
	DW_OP_div = 0x1b,
	DW_OP_minus = 0x1c,
	DW_OP_mod = 0x1d,
	DW_OP_mul = 0x1e,
	DW_OP_neg = 0x1f,
	DW_OP_not = 0x20,
	DW_OP_or = 0x21,
	DW_OP_plus = 0x22,
	DW_OP_plus_uconst = 0x23,
	DW_OP_shl = 0x24,
	DW_OP_shr = 0x25,
	DW_OP_shra = 0x26,
	DW_OP_xor = 0x27,
	DW_OP_skip = 0x2f,
	DW_OP_bra = 0x28,
	DW_OP_eq = 0x29,
	DW_OP_ge = 0x2a,
	DW_OP_gt = 0x2b,
	DW_OP_le = 0x2c,
	DW_OP_lt = 0x2d,
	DW_OP_ne = 0x2e,
	DW_OP_lit0 = 0x30,
	DW_OP_lit1 = 0x31,
	//...
	DW_OP_lit31 = 0x4f,
	DW_OP_reg0 = 0x50,
	DW_OP_reg1 = 0x51,
	//...
	DW_OP_reg31 = 0x6f,
	DW_OP_breg0 = 0x70,
	DW_OP_breg1 = 0x71,
	//...
	DW_OP_breg31 = 0x8f,
	DW_OP_regx = 0x90,
	DW_OP_fbreg = 0x91,
	DW_OP_bregx = 0x92,
	DW_OP_piece = 0x93,
	DW_OP_deref_size = 0x94,
	DW_OP_xderef_size = 0x95,
	DW_OP_nop = 0x96,
	DW_OP_lo_user = 0xe0,
	DW_OP_hi_user = 0xff
};

enum DW_ATE//base rtype encoding
{
	DW_ATE_address = 0x1,
	DW_ATE_boolean = 0x2,
	DW_ATE_complex_float = 0x3,
	DW_ATE_float = 0x4,
	DW_ATE_signed = 0x5,
	DW_ATE_signed_char = 0x6,
	DW_ATE_unsigned = 0x7,
	DW_ATE_unsigned_char = 0x8,
	DW_ATE_lo_user = 0x80,
	DW_ATE_hi_user = 0xff
};


namespace dwarf
{
	int sizeof_LEB128(const I_DataSourceBase&, OFF_t);
	OpType_t typeId_LEB128(I_Module&);
	HTYPE type_LEB128(I_Module&);

#pragma pack(push, 1)
	struct DebugInfoUnitHeader_t
	{
		uint32_t	unit_length;
		uint16_t	version;
		uint32_t	debug_abbrev_offset;
		uint8_t	address_size;
	};
	struct DebugInfoUnitHeader64_t
	{
		uint32_t	unit_length32;
		uint64_t	unit_length;
		uint16_t	version;
		uint64_t	debug_abbrev_offset;
		uint8_t	address_size;
	};
	struct DebugInfoLineHeader_t
	{
		uint32_t	total_length;
		uint16_t	version;
		uint32_t	prologue_length;
		uint8_t		minimum_instruction_length;
		uint8_t		default_is_stmt;
		int8_t		line_base;
		uint8_t		line_range;
		uint8_t		opcode_base;
		uint8_t		opcode_length[1];
	};
#pragma pack(pop)

	class exception : public std::exception
	{
		std::string m_what;
	public:
		exception(const std::string& what) : m_what(what){}
	protected:
		virtual const char* what() const throw () { return m_what.c_str();}
	};

	template <typename T_HOST>
	class DebugInfo
	{
		typedef	T_HOST	MyHost;//ELF_t<T>
		//typedef	typename	MyHost::MyShdr	MyShdr;
		typedef	typename	MyHost::MyAddr	MyAddr;
		//typedef	typename	MyHost::MyOff	MyOff;

		const MyHost& m_host;

		DataSubSource_t	m_debugInfo;
		DataSubSource_t	m_debugAbbr;
		DataSubSource_t	m_debugLine;
		DataSubSource_t	m_debugStr;

		typedef std::map<std::string, size_t>	MyUFiles;
		typedef MyUFiles::iterator MyUFilesIt;

		MyUFiles	mUFiles;	//unique list of files gathered across the compilation units, used to resolve (unnamed) types

	public:
		DebugInfo(const MyHost& elf, DataSubSource_t debugInfo, DataSubSource_t	debugAbbr, DataSubSource_t debugLine, DataSubSource_t m_debugStr)
			: m_host(elf),
			m_debugInfo(debugInfo),
			m_debugAbbr(debugAbbr),
			m_debugLine(debugLine),
			m_debugStr(m_debugStr)
		{
		}

		const MyHost& host() const { return m_host; }
		const I_DataSourceBase& data() const { return m_host.data(); }
		const I_DataSourceBase& debugStrData() const { return m_debugStr; }
		ADDR VA2RVA(MyAddr va) const { return m_host.VA2RVA(va); }

		MyUFilesIt addUFile(std::string s){
			std::pair<MyUFilesIt, bool> res(mUFiles.insert(std::make_pair(s, 0)));
			if (res.second)
				res.first->second = mUFiles.size();//unique id
			return res.first;
		}

		//////////////////////////////////////////////////
		class DebugInfoStream_t : public DataStream_t
		{
		public:
			DebugInfoStream_t(const I_DataSourceBase& r)
				: DataStream_t(r)
			{
			}
			void skip_form(DW_FORM form, bool b64bit, bool bAddr64)
			{
				switch (form)
				{
				case DW_FORM_addr: bAddr64 ? skip<uint64_t>() : skip<uint32_t>(); break;
				case DW_FORM_block2: forward(read<uint16_t>()); break;
				case DW_FORM_block4: forward(read<uint32_t>()); break;
				case DW_FORM_data2: skip<uint16_t>(); break;
				case DW_FORM_data4: skip<uint32_t>(); break;
				case DW_FORM_data8: skip<uint64_t>(); break;
				case DW_FORM_string://0-terminated string embeded
					while (read<uint8_t>()) {}
					break;
				case DW_FORM_block: forward(read_ULEB128<unsigned>()); break;
				case DW_FORM_block1: forward(read<uint8_t>()); break;
				case DW_FORM_data1: skip<uint8_t>(); break;
				case DW_FORM_flag: skip<uint8_t>(); break;
				case DW_FORM_sdata: ::skip_LEB128(*this); break;
				case DW_FORM_strp: b64bit ? skip<uint64_t>() : skip<uint32_t>(); break;
				case DW_FORM_udata: ::skip_LEB128(*this); break;
				case DW_FORM_ref_addr: b64bit ? skip<uint64_t>() : skip<uint32_t>(); break;
				case DW_FORM_ref1: skip<uint8_t>(); break;
				case DW_FORM_ref2: skip<uint16_t>(); break;
				case DW_FORM_ref4: skip<uint32_t>(); break;
				case DW_FORM_ref8: skip<uint64_t>(); break;
				//DWARF4
				case DW_FORM_sec_offset: b64bit ? skip<uint64_t>() : skip<uint32_t>();	break;
				case DW_FORM_flag_present: /*skip<uint8_t>();*/ break;
				case DW_FORM_exprloc: forward(read_ULEB128<unsigned>()); break;
				case DW_FORM_ref_sig8:
					assert(0);
					break;
				default:
					assert(0);
				}
			}
		};

		template <typename T = size_t>
		class MyIntervalTree : private std::vector<std::map<T, T> >//layers of intervals
		{
			typedef typename std::map<T, T>	Layer;
			typedef typename std::vector<Layer>	Base;
		public:
			MyIntervalTree()
			{
			}
			void clear() { Base::clear(); }
			void openInterval(size_t level, T lower)
			{
				assert(level <= this->size());
				if (level == this->size())
					this->push_back(Layer());
				assert(canAppend(level, lower));
				this->at(level).insert(std::make_pair(lower, 0));
			}
			void closeInterval(size_t level, T upper)
			{
				assert(level < this->size() && !this->at(level).empty());
				assert(upper > this->at(level).rbegin()->first);
				assert(this->at(level).rbegin()->second == 0);//wasn't closed yet
				this->at(level).rbegin()->second = upper;
			}
			size_t depth() const
			{
				return this->size();
			}
			T findInterval(size_t level, T off) const
			{
				assert(level < this->size());
				typename Layer::const_iterator i(this->at(level).upper_bound(off));//always greater
				if (i != this->at(level).begin())
				{
					--i;
					if (i->first <= off && off < i->second)
						return i->first;
				}
				return 0;
			}
		private:
			bool canAppend(size_t level, OFF_t off)//if properly closed
			{
				if (this->at(level).empty())
					return true;
				T first(this->at(level).rbegin()->first);
				T second(this->at(level).rbegin()->second);
				if (second == 0)
					return false;
				return off >= second;
			}
		};

		////////////////////////////////////////////////////////////// CU_Iterator
		class CU_Iterator//compilation unit iterator
		{
			const DebugInfo& md;
			//DebugInfoStream_t	m_dsInfo;
			//DataStream_t	m_dsAbbr;
			DataStream_t	m_dsLine;
			bool		m_b64bit;
			uint16_t	m_version;
			uint8_t		m_address_size;
			OFF_t		m_offLower;
			OFF_t		m_offUpper;
			std::vector<OFF_t>			m_abbrevs;//abbreviations offsets
			std::vector<MyUFilesIt>	m_files;
//std::vector<std::string>	m_files;
			//typedef std::vector<std::pair<size_t,size_t>> MyLevelsVec;
			//MyLevelsVec	m_levels;//to track die's parentage (pairs of <CU offset,span>)
			MyIntervalTree<OFF_t>	m_levels;


		public:

			///////////////////////////////////////////////////////// DIE_Iterator (hierarchy)
			class DIE_Iterator
			{
				friend class CU_Iterator;
				const DebugInfo& md;
				const CU_Iterator& mcu;
				DebugInfoStream_t	m_dsInfo;
				DataStream_t		m_dsAbbr;
				int	m_level;
				OFF_t		m_dieLower;//no init
				uint32_t	m_code;//no init
				uint32_t	m_tag;//no init
				uint8_t		m_children;

			public:
				DIE_Iterator(const CU_Iterator& r)
					: md(r.parent()),
					mcu(r),
					m_dsInfo(md.m_debugInfo),
					m_dsAbbr(md.m_debugAbbr),
					m_level(0),
					m_children(0)
				{
				}
				DIE_Iterator(const DIE_Iterator& o)
					: md(o.md),
					mcu(o.mcu),
					m_dsInfo(o.m_dsInfo),
					m_dsAbbr(o.m_dsAbbr),
					m_level(o.m_level),
					m_dieLower(o.m_dieLower),
					m_code(o.m_code),
					m_tag(o.m_tag),
					m_children(o.m_children)
				{
				}
				DIE_Iterator(const DIE_Iterator& o, uint32_t off)
					: md(o.md),
					mcu(o.mcu),
					m_dsInfo(o.m_dsInfo),
					m_dsAbbr(o.m_dsAbbr),
					m_level(o.m_level)
					//m_dieLower(o.m_dieLower),
					//m_code(o.m_code),
					//m_tag(o.m_tag),
					//m_children(o.m_children)
				{
					seek_CU(off);
				}
				void seek(OFF_t off)//debug_info relative
				{
					m_dsInfo.seek(off);
					fetch_();
					//if (m_code != 0)
					//sweep_attributes();
				}
				void seek_CU(OFF_t off)//CU relative
				{
					m_dsInfo.seek(mcu.lowerBound() + off);
					fetch_();
					//if (m_code != 0)
					//sweep_attributes();
				}

				const CU_Iterator& cu() const { return mcu; }
				uint32_t tag() const { return m_tag; }
				uint32_t code() const { return m_code; }
				void setCode(uint32_t code) { m_code = code; }
				bool hasChildren() const { return m_children != 0; }
				int level() const { return m_level; }
				OFF_t dieLower() const { return m_dieLower; }
				OFF_t abbrevLower() const { return mcu.abbrevs().at(m_code - 1); }
				operator bool() const {
					return (m_dsInfo.tell() < mcu.upperBound() && m_code != 0);
				}
				DIE_Iterator& operator++()//pre
				{
					if (m_code != 0)
						sweep_attributes();
					if (*this)
						do {
							if (m_children)
								++m_level;
							fetch_();
						} while (m_code == 0 && m_level > 0);
						return *this;
				}
				DIE_Iterator& toNextSibling()
				{
					if (!m_children)
						return this->operator++();
					DIE_AttributeReader dar(*this);
					uint32_t uSibling;
					if (dar.seekAttr(DW_AT_sibling) && dar.readReference(uSibling))
					{
						seek_CU(uSibling);
						//m_level--;//was incremented in fetch()
					}
					else
					{
						//this->operator++();
						for (DIE_ChildIteratorRef j(*this); j; j.toNextSibling())//advance parent
						{
							STOP
						}
					}
					return *this;
				}
				/*template <typename T>
				bool fetchAttr(DW_AT at, T &t)
				{
				DataStreamEx_t ds(m_dsAbbr);
				DataStreamEx_t ds2(m_dsInfo);
				while (ds.peek<uint16_t>() != 0)
				{
				uint16_t attr, form;
				ds.decode_ULEB128(attr);
				ds.decode_ULEB128(form);
				if (attr == at)
				{
				t = read_form<T>((DW_FORM)form, ds2);
				return true;
				}
				read_form<uint64_t>((DW_FORM)form, ds2);//just skip
				}
				return false;
				}*/
			protected:
				void fetch_()
				{
					m_dieLower = m_dsInfo.tell();
					//CHECK(m_dieLower == 0x5d)
					//STOP
					::decode_ULEB128(m_code, m_dsInfo);
					if (m_code == 0)
					{
						--m_level;
						return;
					}
					if (!*this)
						return;

					if (!seekAbbr(m_code))//read code
					{
						std::ostringstream ss;
						ss << "abbriviation not exists for code=" << m_code;
						throw exception(ss.str());
					}
					
					::decode_ULEB128(m_tag, m_dsAbbr);
					m_dsAbbr.read(m_children);
				}
				void sweep_attributes()
				{
					for (;;)
					{
						uint16_t attr;
						uint8_t form;
						::decode_ULEB128(attr, m_dsAbbr);
						::decode_ULEB128(form, m_dsAbbr);
						if (attr == 0 && form == 0)
							break;
						m_dsInfo.skip_form((DW_FORM)form, mcu.m_b64bit, mcu.m_address_size == 8);
					}
				}

				bool seekAbbr(uint32_t code)
				{
					if (!(code - 1 < mcu.abbrevs().size()))
						return false;
					m_dsAbbr.seek(mcu.abbrevs().at(code - 1));
					uint32_t code2;
					::decode_ULEB128(code2, m_dsAbbr);
					if (code2 != code)
						return false;
					return true;
				}
			};

			///////////////////////////////////////////////////////////// DIE_ChildIterator
			class DIE_ChildIterator : public DIE_Iterator
			{
				const DIE_Iterator& m_r;
			public:
				DIE_ChildIterator(const DIE_Iterator& r)
					: DIE_Iterator(r),
					m_r(r)
				{
					this->m_level = 0;
					this->operator++();
				}
			};

			class DIE_ChildIteratorRef : public DIE_Iterator//updates parent
			{
				DIE_Iterator& m_r;
			public:
				DIE_ChildIteratorRef(DIE_Iterator& r)
					: DIE_Iterator(r),
					m_r(r)
				{
					this->m_level = 0;
					this->operator++();
				}
				~DIE_ChildIteratorRef()
				{
					m_r.seek(this->m_dsInfo.tell());
				}
			};


			////////////////////////////////////////////////////////////////// DIE_Reader
			class DIE_AttributeReader : public DebugInfoStream_t//dsInfo
			{
				const DIE_Iterator& m_die;
				DW_FORM m_form;
			public:
				using DataStream_t::read;
			public:
				DIE_AttributeReader(const DIE_Iterator& die)
					: DebugInfoStream_t(die.m_dsInfo),
					m_die(die),
					m_form(DW_FORM_nil)
				{
				}
				bool is64bit() const { return m_die.cu().m_b64bit; }
				bool isAddr64() const { return m_die.cu().m_address_size == 8; }
				const CU_Iterator& cu() const { return m_die.cu(); }
				DW_FORM form() const { return m_form; }
				bool seekAttr(DW_AT at)
				{
					this->seek(m_die.m_dsInfo.tell());//reset
					DataStream_t dsAbbr(m_die.m_dsAbbr);
					for (;;)
					{
						uint16_t attr;
						uint8_t form;
						::decode_ULEB128(attr, dsAbbr);
						::decode_ULEB128(form, dsAbbr);
						if (attr == 0 && form == 0)
							break;
						if (attr == at)
						{
							m_form = (DW_FORM)form;
							return true;
						}
						this->skip_form((DW_FORM)form, is64bit(), isAddr64());
					}
					return false;
				}
				template <typename T, typename U> U read_() { return static_cast<U>(this->template read<T>()); }
				template <typename T> T read_ULEB128_() { return this->template read_ULEB128<T>(); }

				template <typename T>
				bool readConstant(T& t)
				{
					if (std::is_signed<T>::value)
					{
						switch (m_form) {
						case DW_FORM_data1: t = (T)this->template read<int8_t>(); break;
						case DW_FORM_data2: t = read_<int16_t, T>(); break;
						case DW_FORM_data4: t = read_<int32_t, T>(); break;
						case DW_FORM_data8: t = read_<int64_t, T>(); break;
						case DW_FORM_sdata: ::decode_SLEB128(t, *this); break;
						default: return false;
						}
					}
					else
					{
						switch (m_form) {
						case DW_FORM_data1: t = read_<uint8_t, T>(); break;
						case DW_FORM_data2: t = read_<uint16_t, T>(); break;
						case DW_FORM_data4: t = read_<uint32_t, T>(); break;
						case DW_FORM_data8: t = read_<uint64_t, T>(); break;
						case DW_FORM_sdata: ::decode_ULEB128(t, *this); break;
						case DW_FORM_sec_offset:
							t = is64bit() ? read_<uint64_t, T>() : read_<uint32_t, T>(); break;
						default: return false;
						}
					}
					return true;
				}
				template <typename T>
				bool readReference(T& t)
				{
					switch (m_form) {
					case DW_FORM_ref1: t = read_<int8_t, T>(); break;
					case DW_FORM_ref2: t = read_<int16_t, T>(); break;
					case DW_FORM_ref4: t = read_<int32_t, T>(); break;
					case DW_FORM_ref8: t = read_<int64_t, T>(); break;
					case DW_FORM_ref_udata: t = read_ULEB128_<T>(); break;
						//case DW_FORM_ref_addr: assert(0); break;
					default: return false;
					}
					return true;
				}
				bool readBlockSize(size_t& s)
				{
					switch (m_form) {
					case DW_FORM_block1: s = this->template read<uint8_t>(); break;
					case DW_FORM_block2: s = this->template read<uint16_t>(); break;
					case DW_FORM_block4: s = this->template read<uint32_t>(); break;
					case DW_FORM_block: s = read_ULEB128_<size_t>(); break;
					case DW_FORM_exprloc: s = read_ULEB128_<size_t>(); break;
					default: return false;
					}
					return true;
				}
				bool skipBlockSize()
				{
					size_t s;//don't care
					return readBlockSize(s);
				}
				MyAddr readAddress()
				{
					return this->template read<MyAddr>();
				}
				bool fetchStringAttr(std::ostringstream& ss, DW_AT at)
				{
					if (!seekAttr(at))
						return false;
					if (m_form == DW_FORM_string)
						return this->fetchString(ss) != 0;
					if (m_form != DW_FORM_strp)
						return false;
					OFF_t off(readStringOff());
					DataStream_t p(cu().parent().debugStrData(), off);
					return p.fetchString(ss) != 0;
				}
				bool fetchStringAttrEx(std::ostringstream& ss, DW_AT at = DW_AT_name)//check specification
				{
					if (!fetchStringAttr(ss, at))
					{
						uint32_t uRef;
						if (!seekAttr(DW_AT_specification) || !readReference(uRef))
							return false;
						DIE_Iterator j(m_die, uRef);
						DIE_AttributeReader dar(j);
						return dar.fetchAttrFullName(ss);
						//return dar.fetchStringAttr(ss, at);
					}
					return true;
				}
				bool fetchAttrName(std::ostringstream& ss)
				{
					ss.clear();
					ss.str("");//clear
					return fetchStringAttr(ss, DW_AT_name);
				}
				bool fetchAttrFullName(std::ostringstream& ss)//, bool bClean = false)
				{
					bool bRet(true);
					/*if (bClean)
					{
						ss.clear();
						ss.str("");//clear
					}*/
					OFF_t oTarget(m_die.dieLower());
					DIE_Iterator j(m_die);
					if (!seekAttr(DW_AT_name))
					{
						uint32_t uRef;
#if(0)
						if (!(seekAttr(DW_AT_abstract_origin) || seekAttr(DW_AT_specification)))
							return false;
						if (!readReference(uRef))
							return false;
#else
						if ((seekAttr(DW_AT_abstract_origin) || seekAttr(DW_AT_specification)) && readReference(uRef))
#endif
						{
							j.seek_CU(uRef);
							oTarget = j.dieLower();
						}
						else
							bRet = false;//unnamed (autonamed?)
					}
					DIE_AttributeReader dar(j);
					for (size_t n(1); n < m_die.cu().levelsDepth(); ++n)
					{
						OFF_t off(m_die.cu().findInterval(n, oTarget));
						if (off == 0)
							off = oTarget;//just itself
						assert(off <= oTarget);//nested at some level?
						j.seek(off);
						if (!dar.fetchStringAttrEx(ss))//may follow a specification
						{//unnamed namespace?
CHECK(off==6407834)
STOP
CHECK(off==6157955)
STOP
							ss << "__unnamed_scope_" << std::hex << off << std::dec;
							//break;
						}
						if (off == 0 || off == oTarget)
						{
							return bRet;
						}
						if (j.tag() != DW_TAG_structure_type
							&& j.tag() != DW_TAG_class_type
							&& j.tag() != DW_TAG_union_type
							&& j.tag() != DW_TAG_namespace)
							break;//must be a valid container
						ss << "::";
					}
					return false;
				}
				bool fetchLinkageName(std::ostringstream& ss)
				{
					assert(m_die.tag() == DW_TAG_subprogram);
					if (fetchStringAttr(ss, DW_AT_linkage_name))
						return true;
					if (fetchStringAttr(ss, DW_AT_MIPS_linkage_name))
						return true;
					if (seekAttr(DW_AT_abstract_origin) || seekAttr(DW_AT_specification))
					{
						uint32_t uRef;
						if (readReference(uRef))
						{
							DIE_Iterator i(m_die, uRef);
							DIE_AttributeReader dar(i);
							return dar.fetchLinkageName(ss);
						}
					}
					return false;
				}
				OFF_t readStringOff()
				{
					if (m_form == DW_FORM_string)
						return this->tell();
					if (m_form == DW_FORM_strp)
						return cu().m_b64bit ? read_<uint64_t, OFF_t>() : read_<uint32_t, OFF_t>();
					std::ostringstream ss;
					ss << "unrecognized string form for DIE attribute at " << std::hex << m_die.dieLower();
					throw exception(ss.str());
					return 0;
				}
			};


			///////////////////////////////////////////////////////////// DIE_SiblingIterator
			class DIE_SiblingIterator : public DIE_Iterator
			{
				DIE_AttributeReader m_dar;
			public:
				DIE_SiblingIterator(CU_Iterator& r)
					: DIE_Iterator(r),
					m_dar(*this)
				{
				}
				operator bool() const {
					return false;//!mcu.isOverflow() && m_code != 0;
				}
				DIE_SiblingIterator& operator++()//pre
				{
					/*sweep_attributes();
					if (!mcu.isOverflow())
						fetch_();*/
					return *this;
				}
			};




		private:

			DIE_Iterator	m_DIEiter;

		public:
			CU_Iterator(const DebugInfo& r)
				: md(r),
				m_dsLine(md.m_debugLine),
				m_offLower(0),
				//m_offUpper(-1),
				m_DIEiter(*this),
				m_files(1)//1-based
			{
				if (!fetch())
					invalidate();
			}

			void reset()
			{
				dsInfo().seek(0);
				fetch();
			}

			DIE_Iterator& dieIterator() { return m_DIEiter; }
			OFF_t lowerBound() const { return m_offLower; }
			OFF_t upperBound() const { return m_offUpper; }
			const std::vector<OFF_t>& abbrevs() const { return m_abbrevs; }
			//std::vector<OFF_t> &abbrevs(){ return m_abbrevs; }
			DebugInfoStream_t& dsInfo() { return m_DIEiter.m_dsInfo; }
			const DebugInfoStream_t& dsInfo() const { return m_DIEiter.m_dsInfo; }
			DataStream_t& dsAbbr() { return m_DIEiter.m_dsAbbr; }
			const DataStream_t& dsAbbr() const { return m_DIEiter.m_dsAbbr; }
			//const MyIntervalTree &levels() const { return m_levels; }
			size_t levelsDepth() const { return m_levels.depth(); }
			OFF_t findInterval(size_t level, OFF_t off) const { return m_levels.findInterval(level, off); }
			void invalidate() { dsInfo().seek(OFF_NULL); }

			operator bool() const {
				return !dsInfo().isAtEnd();
			}
			bool isOverflow() const {
				return !(dsInfo().tell() < m_offUpper);
			}
			CU_Iterator& operator++()//pre
			{
				if (dsInfo().seek(m_offUpper))
					if (!fetch())
						invalidate();
				return *this;
			}

		private:

			bool fetch()
			{
				DebugInfoStream_t& dis(dsInfo());
				if (!dis)
					return false;
				m_files.resize(1);
				m_offLower = dis.tell();
				uint32_t unit_length;
				dis.read(unit_length);
				if (unit_length < 0xfffffff0)
				{
					if (unit_length == 0)
						return false;//padding?
					m_b64bit = false;
					m_offUpper = dis.tell() + unit_length;
				}
				else
				{
					m_b64bit = true;
					uint64_t unit_length64;
					dis.read(unit_length64);
					m_offUpper = dis.tell() + unit_length64;
				}
				dis.read(m_version);
				if (!(m_version == 2 || m_version == 4))
					return false;
				OFF_t debug_abbrev_offset;
				if (!m_b64bit)
					debug_abbrev_offset = OFF_t(dis.template read<uint32_t>());
				else
					debug_abbrev_offset = OFF_t(dis.template read<uint64_t>());

				m_abbrevs.clear();
				fetchAbbrevTable(debug_abbrev_offset);
				//m_abbrevs.push_back(debug_abbrev_offset);//starting offset
				//m_dsAbbr.seek(debug_abbrev_offset);

				//fetchLevelsTable();

				dis.read(m_address_size);
				m_DIEiter.fetch_();
				return true;
			}

		public:
			const DebugInfo& parent() const { return md; }
			bool fetchAbbrevTable(OFF_t o)
			{
				DataStream_t& dsa(dsAbbr());
				dsa.seek(o);
				for (;;)
				{
					uint32_t code;
					::decode_ULEB128(code, dsa);
					if (code == 0)
						break;
					if (code - 1 != m_abbrevs.size())
					{
						std::ostringstream ss;
						ss << "abbreviation code mismatch for index=" << code;
						throw exception(ss.str());
					}
					m_abbrevs.push_back(o);
					::skip_LEB128(dsa);//tag
					dsa.template skip<uint8_t>();//children
					for (;;)
					{
						uint16_t attr;
						uint8_t form;
						::decode_ULEB128(attr, dsa);
						::decode_ULEB128(form, dsa);
						if (attr == 0 && form == 0)
							break;
					}
					o = dsa.tell();
				}
				return true;
			}

			bool fetchFileTable(OFF_t o)
			{
				m_dsLine.seek(o);
				DebugInfoLineHeader_t h;
				m_dsLine.read(h);
				if (!(h.version == 2 || h.version == 4))
					return false;
				m_dsLine.forward(h.opcode_base - 1);
				std::vector<std::string> dirTable(1);//for nullptr element
				std::ostringstream ss;
				while (m_dsLine.fetchString(ss))
				{
					dirTable.push_back(ss.str());
					ss.str("");
				}
				while (m_dsLine.fetchString(ss))
				{
					uint32_t dir_index;
					::decode_ULEB128(dir_index, m_dsLine);//directory_index
					if (dir_index >= dirTable.size())
						return false;//error
					std::string s(dirTable[dir_index] + "/" + ss.str());
					m_files.push_back(const_cast<DebugInfo&>(md).addUFile(s));
					//m_files.push_back(dirTable[dir_index] + "/" + ss.str());
#if(0)
					std::cout << " ---  file[" << m_files.size() - 1 << "] = " << m_files.back() << std::endl;
#endif
					::skip_LEB128(m_dsLine);//modification_time
					::skip_LEB128(m_dsLine);//length
					ss.str("");
				}
				return true;
			}
			bool fetchLevelsTable()
			{
				int level(0);
				m_levels.clear();
				assert(dieIterator().tag() == DW_TAG_compile_unit);
				DIE_Iterator i(dieIterator());
				for (; i; ++i)
				{
					//CHECK(i.dieLower() == 0x359)
					//STOP
					while (i.level() < level)
						m_levels.closeInterval(--level, i.dieLower());

					if (i.hasChildren())
						m_levels.openInterval(level++, i.dieLower());//next die is a child
				}
				while (i.level() < level)
					m_levels.closeInterval(--level, i.dieLower());
				return false;
			}

			size_t toUFileIndex(size_t i) const
			{
				if (!(i < m_files.size()))
					return 0;
				return m_files.at(i)->second;
			}
			const char* fileAt(size_t i) const
			{
				if (!(i < m_files.size()))
					return nullptr;
				return m_files.at(i)->first.c_str();
				//return m_files.at(i).c_str();
			}
		};//CU_Iterator
	};//DebugInfo

	inline int sizeof_LEB128(const I_DataSourceBase& r, OFF_t o)
	{
		int sz(1);
		DataStream_t p(r, o);
		for (; sz < 16; ++sz)
			if (!(p.read<unsigned char>() & 0x80))
				break;
		return sz;
	}

	inline OpType_t typeId_LEB128(I_Module& r)
	{
		return MAKETYP_UINT(dwarf::sizeof_LEB128(r, r.cpr()));
	}

	inline HTYPE type_LEB128(I_Module& r)
	{
		HTYPE h(r.type(dwarf::typeId_LEB128(r)));
		if (!h)
			h = r.arrayOf(r.type(OPSZ_BYTE), dwarf::sizeof_LEB128(r, r.cpr()));
		return h;
	}


}//dwarf

const char* DW_TAG_to_string(unsigned);
const char* DW_AT_to_string(unsigned);



