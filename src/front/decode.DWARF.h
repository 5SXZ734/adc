#pragma once

#include "shared/front.h"
#include "DWARF.h"

#define PATH_FROM_DWARF	"from_DWARF"

/////////////////////////////////////// MyDwarfDumper
template <typename T_HOST>
class MyDwarfDumper : public I_DebugInfoDumper
{
	typedef typename T_HOST::MyDebugInfo	MyDebugInfo;
	typedef	typename T_HOST::MyAddr	MyAddr;

	MyDebugInfo m_DI;
	typedef typename MyDebugInfo::CU_Iterator CU_Iterator;
	typedef typename CU_Iterator::DIE_Iterator	DIE_Iterator;
	typedef typename CU_Iterator::DIE_ChildIterator	DIE_ChildIterator;
	typedef typename CU_Iterator::DIE_ChildIteratorRef	DIE_ChildIteratorRef;
	typedef typename CU_Iterator::DIE_AttributeReader DIE_AttributeReader;
	CU_Iterator		m_itCU;
	DIE_Iterator& m_itDIE;//reference!
	unsigned m_cus_num;//for progress tracking
	unsigned m_cu_no;//cuurent
	//std::ofstream m_ofs;

	struct fileline_t
	{
		size_t file;//in unit
		size_t line;
		fileline_t() : file(0), line(0){}
		bool operator < (const fileline_t& o) const
		{
			if (file < o.file)
				return true;
			if (file == o.file)
				return line < o.line;
			return false;
        }
	};

	typedef std::map<fileline_t, HTYPE>	MyTypesCache;
	typedef typename MyTypesCache::iterator MyTypesCacheIt;
	typedef typename MyTypesCache::const_iterator MyTypesCacheCIt;
	MyTypesCache mTypesCache;

public:
	MyDwarfDumper(const T_HOST& elf)
		: //m_ELF(elf),
		m_DI(elf),
		m_itCU(m_DI),
		m_itDIE(m_itCU.dieIterator()),
		m_cus_num(0),
		m_cu_no(0)
		//,m_ofs("test.log")
	{
		for (int n(0); m_itCU; ++m_itCU, n++)
			m_cus_num++;
		m_itCU.reset();
		if (fetchFileTable())
		{
		}//++m_itDIE;//advance to the next die in CU
	}
	~MyDwarfDumper()
	{
	}
protected:

#define NO_TRY 0
#if(NO_TRY)
#define	TRY
#define CATCH(a) if (0)
#else
#define	TRY	try
#define CATCH catch
#endif

	void print(int level, OFF_t off, uint32_t code, uint16_t tag, std::ostream& os)
	{
		os << "<" << std::dec << level << "><" << std::hex << off << ">:  Abbrev Number: " << std::dec << code << " (DW_TAG_" << DW_TAG_to_string(tag) << ")" << std::endl;
	}

	void print1(std::ostream& os)//sequential iteration example
	{
		for (CU_Iterator cu(m_DI); cu; ++cu)
		{
			for (DIE_Iterator& i(cu.dieIterator()); i; ++i)
			{
				print(i.level(), i.dieLower(), i.code(), i.tag(), os);
			}
		}
	}

	bool print2r(DIE_Iterator& i, int level, std::ostream& os)//returns true if the parent has been already advanced (by a child)
	{
		print(level, i.dieLower(), i.code(), i.tag(), os);
		if (i.hasChildren())
		{
#if(1)
			if (i.tag() == DW_TAG_subprogram)
			{
				i.toNextSibling();//skip children example
			}
			else
#endif
			{
				for (DIE_ChildIterator j(i, true); j;)
					if (!print2r(j, level + 1, os))
						++j;
			}
			return true;
		}
		return false;
	}

	void print2(std::ostream& os)//recursive iteration example
	{
		for (CU_Iterator cu(m_DI); cu; ++cu)
			print2r(cu.dieIterator(), 0, os);
	}


	virtual bool processNext(I_ModuleCB& rICb, unsigned& progress)//global symbols
	{
		//print2(m_ofs);
		//return false;
#if(0)
		if (!processNextImpl(rICb, progress))
			return false;
#else
		TRY
		{
			if (!processNextImpl(rICb, progress))
				return false;
		}
		CATCH(...)
		{
			rICb.error("failed to process a DIE");
			return false;
		}
#endif
		//			return false;//just one CU
		return true;
	}

private:
	bool processNextImpl(I_ModuleCB& rICb, unsigned& progress)
	{
		if (!m_itDIE)
			return false;
CHECK(m_itDIE.dieLower() == 0x4abee0)
STOP

		if (m_itDIE.tag() != DW_TAG_compile_unit)
			return false;

		DIE_AttributeReader dar(m_itDIE);
		std::ostringstream ss;
		if (dar.fetchStringAttr(ss, DW_AT_name))
		{
			std::string s(ss.str());
			ss.str("");//clear
			if (dar.fetchStringAttr(ss, DW_AT_comp_dir))
				if (s.rfind(ss.str(), 0) != 0)//if name not prefixed by dir
					s.insert(0, ss.str() + "/");
			rICb.selectFile(s.c_str(), PATH_FROM_DWARF);
		}

		for (DIE_ChildIterator j(m_itDIE); j;)//no need to update parent
		{
			if (!process_DIE(j, rICb, false))
				++j;
		}
		//progress = TProgress(++m_cu_no, m_cus_num);
		rICb.resetProgress("DWARF Symbols", TProgress(++m_cu_no, m_cus_num));
		if (++m_itCU)
			fetchFileTable();
		return true;
	}

	bool fetchFileTable()
	{
		if (m_itDIE.tag() != DW_TAG_compile_unit)
			return false;
		DIE_AttributeReader DR(m_itDIE);
		OFF_t oStmtList;
		if (DR.seekAttr(DW_AT_stmt_list) && DR.readConstant(oStmtList))
			if (!m_itCU.fetchFileTable(oStmtList))
				return false;
		m_itCU.fetchLevelsTable();
		return true;
	}
	bool process_DIE(DIE_Iterator& i, I_ModuleCB& rICb, bool bUnion)//returns true if the parent iterator has been advanced (by the child)
	{
CHECK(i.dieLower() == 0x38b)
STOP
		//print(level, i.dieLower(), i.code(), i.tag(), os);
		if (i.tag() == DW_TAG_member || i.tag() == DW_TAG_inheritance)
			fetchMember(i, rICb, bUnion);//should be in a scope!
		else if (i.tag() == DW_TAG_enumerator)
			fetchEnumerator(i, rICb);
		else if (i.tag() == DW_TAG_typedef)
			declTypedef(i, rICb);
		else if (i.tag() == DW_TAG_formal_parameter)
			declFormalParameter(i, rICb);
		else if (i.tag() == DW_TAG_structure_type)
		{
			STOP
		}

		if (i.hasChildren())
		{
			if (!fetchSubprogram(i, rICb))
			{
//CHECK(i.dieLower() == 0xc720)
//STOP
				bool bUnion(false);
				if (newScope(i, rICb, bUnion))
				{
//CHECK(i.dieLower() == 0x129cf)
//STOP
					for (DIE_ChildIteratorRef j(i); j;)//should update parent
						if (!process_DIE(j, rICb, bUnion))
							++j;
					rICb.Leave();
				}
				else
					i.toNextSibling();
			}

			return true;
		}
		return false;
	}
	struct subprogram_info_t
	{
		MyAddr low_pc, high_pc;
		uint32_t frame_base;
		uint8_t	virtuality;
		subprogram_info_t()
			: low_pc(0), high_pc(0), frame_base(0), virtuality(false)
		{
		}
	};

	void selectFile(const fileline_t& info, I_ModuleCB& rICb)
	{
		const char* path(info.file != 0 ? m_itCU.fileAt(info.file) : nullptr);
		if (path)
			rICb.selectFile(path, PATH_FROM_DWARF);
	}

	fileline_t fetchFileInfo(const DIE_Iterator& i)
	{
		DIE_AttributeReader dar(i);

		fileline_t info;
		if (dar.seekAttr(DW_AT_decl_file) && dar.readConstant(info.file))
		{
			if (!(dar.seekAttr(DW_AT_decl_line) && dar.readConstant(info.line)))
			{
				STOP//?Unknown AT value: 2116: 1
			}
			return info;
		}

		if (i.tag() == DW_TAG_subprogram)
		{
			if (dar.seekAttr(DW_AT_abstract_origin) || dar.seekAttr(DW_AT_specification))
			{
				uint32_t uRef;
				if (dar.readReference(uRef))
				{
					DIE_Iterator j(m_itCU, uRef);
					return fetchFileInfo(j);
				}
			}
		}
		return info;
	}

	bool fetchSubprogram(DIE_Iterator& i, I_ModuleCB& rICb)
	{//return 0;
CHECK(i.dieLower() == 0x0000000000292fbf)
STOP
		if (i.tag() != DW_TAG_subprogram)
			return false;

		subprogram_info_t info;
		bool bSkip(true);
		DIE_AttributeReader dar(i);
		if (dar.seekAttr(DW_AT_low_pc))
		{
			info.low_pc = dar.readAddress();
			ADDR va(m_DI.VA2RVA(info.low_pc));
			if (rICb.EnterSegment(nullptr, va))//default rangeseg
			{
				SAFE_SCOPE_HERE(rICb);

				if (dar.seekAttr(DW_AT_abstract_origin) || dar.seekAttr(DW_AT_specification))
				{
					uint32_t uRef;
					if (dar.readReference(uRef))
					{
						DIE_Iterator j(m_itCU, uRef);
						DIE_AttributeReader darj(j);
						if (darj.seekAttr(DW_AT_virtuality))
							darj.readConstant(info.virtuality);
					}
				}

				std::ostringstream ss;
				dar.fetchLinkageName(ss);

				selectFile(fetchFileInfo(i), rICb);//put in a source file

				rICb.declPField(ss.str().c_str());//create a procedure here first

				//HTYPE hScope;
				//if ((hScope = rICb.NewScope(rICb.declField(ss.str().c_str()), SCOPE_PROC)) || rICb.EnterScope(va))
				{
					//SAFE_SCOPE_HERE(rICb);

					//if (hScope)
						//rICb.declField(ss.str().c_str(), hScope);

					if (dar.seekAttr(DW_AT_frame_base))
						dar.readConstant(info.frame_base);

					if (dar.seekAttr(DW_AT_high_pc))
					{
						info.high_pc = dar.readAddress();
						if (info.high_pc > info.low_pc)
						{
							//?rICb.skip(int(info.high_pc - info.low_pc));
							//?rICb.declField(nullptr, nullptr, ATTR_VA);//end of function
						}
					}
					std::ostringstream ss2;
					fetchName(i, ss2);//fully qualified

					AttrScopeEnum attr2(info.virtuality ? AttrScopeEnum::ATTRP_VIRTUAL : AttrScopeEnum::null);
					if (rICb.NewScope(ss2.str().c_str(), SCOPE_FUNC, attr2))
					{
						SAFE_SCOPE_HERE(rICb);
						bSkip = false;//no longer skip required
						//..create funcdef
						for (DIE_ChildIteratorRef j(i); j;)//with parent update
							if (!process_DIE(j, rICb, false))
								++j;
					}
				}
			}
		}
		if (bSkip)
			i.toNextSibling();
		return true;
	}
	HFIELD declFormalParameter(const DIE_Iterator& i, I_ModuleCB& rICb)
	{//return nullptr;
CHECK(i.dieLower() == 0x140271)
STOP
		assert(i.tag() == DW_TAG_formal_parameter);
		DIE_AttributeReader dar(i);
		if (dar.seekAttr(DW_AT_location))
		{
			uint32_t uRef;
			int32_t iOffset(-1);//invalid
			if (dar.skipBlockSize())//was a block
			{
				DW_OP op((DW_OP)dar.template read<uint8_t>());
				if (op == DW_OP_fbreg)
					iOffset = dar.template read_SLEB128<int32_t>();//LATER: can't get OPC/OPOFF from formal parameters(!)
			}
			else if (dar.readConstant(uRef))
			{
				STOP
			}

			if (dar.seekAttr(DW_AT_abstract_origin))
			{
				if (dar.readReference(uRef))
				{
					DIE_Iterator j(m_itCU, uRef);
					return declFormalParameter0(j, rICb);
				}
			}
			return declFormalParameter0(i, rICb);
		}
		return nullptr;
	}
	HFIELD declFormalParameter0(const DIE_Iterator& i, I_ModuleCB& rICb)
	{
CHECK(i.dieLower() == 0xbc311)
STOP
		HTYPE hType(nullptr);
		DIE_AttributeReader dar(i);
		if (dar.seekAttr(DW_AT_type))
		{
			uint32_t uRef;
			if (dar.readReference(uRef))
			{
				DIE_Iterator k(m_itCU, uRef);
				hType = fetchType(k, rICb);
			}
		}

		AttrIdEnum attr(ATTR_NULL);
		std::ostringstream ss;
		dar.fetchAttrName(ss);
		if (ss.str() == "this" && dar.seekAttr(DW_AT_artificial))
			attr = ATTRF_THISPTR;

		return rICb.declField(ss.str().c_str(), hType, attr);//hType should be 'T * const this'
	}
	bool fetchLocation(const DIE_Iterator& i, I_ModuleCB& rICb)
	{
		DIE_AttributeReader dar(i);
		if (dar.seekAttr(DW_AT_low_pc))
		{
			MyAddr low_pc(dar.readAddress());
			return rICb.EnterSegment(nullptr, m_DI.VA2RVA(low_pc));
			/*if (dar.seekAttr(DW_AT_high_pc))
			{
				MyAddr highPc(dar.readAddress());
				STOP
			}*/
			return true;
		}
		return false;
	}
	bool fetchName(const DIE_Iterator& i, std::ostringstream& ss)//if there is a specification, fetch a full qualified name
	{
		DIE_AttributeReader dar(i);
		if (dar.fetchStringAttr(ss, DW_AT_name))
			return true;
		ss << "::";
		if (dar.fetchAttrFullName(ss))
			return true;
		if (ss.str() == "::")
			ss.str("");//clear
		return false;
	}

	HTYPE findTypeByFileinfo(const fileline_t& info) const
	{
		MyTypesCacheCIt it(mTypesCache.find(info));
		if (it != mTypesCache.end())
			return it->second;
		return nullptr;
	}

	HTYPE newScope(DIE_Iterator& i, I_ModuleCB& rICb, bool& bUnion)
	{
		//if (newFunction(i, rICb))
			//return true;

		SCOPE_enum eScope;
		if (i.tag() == DW_TAG_namespace)
			eScope = SCOPE_NAMESPACE;
		else if (i.tag() == DW_TAG_structure_type)
			eScope = SCOPE_STRUC;
		else if (i.tag() == DW_TAG_class_type)
			eScope = SCOPE_CLASS;
		else if (i.tag() == DW_TAG_union_type)
		{
			eScope = SCOPE_STRUC;//SCOPE_UNION;
			bUnion = true;
		}
		else if (i.tag() == DW_TAG_enumeration_type)
			eScope = SCOPE_ENUM;
		else
		{
			assert(i.tag() != DW_TAG_compile_unit);
			return nullptr;
		}

//CHECK(i.dieLower() == 0x6ba)
//STOP
CHECK(i.dieLower() == 0x1626c)
STOP

		std::ostringstream ss;

		fileline_t info(fetchFileInfo(i));

#if(1)
		if (fetchName(i, ss))
		{
			selectFile(info, rICb);

			return rICb.NewScope(ss.str().c_str(), eScope);
		}
#endif

		if (i.tag() != DW_TAG_namespace)
		{
			if (info.file != 0)//namespaces can be scattered
			{
				fileline_t info_g(info);//convert a file index from unit-based to module-based
				info_g.file = m_itCU.toUFileIndex(info.file);
				HTYPE hType(findTypeByFileinfo(info_g));
				if (hType)
				{
					//if (rICb.EnterScope(hType, 0))
						//return hType;
					return nullptr;//failed - exists
				}

				selectFile(info, rICb);

				hType = rICb.NewScope(ss.str().c_str(), eScope);
				if (hType)
				{
					//CHECK(info_g.file == 0x22c && info_g.line == 0x9a)
						//STOP
					mTypesCache.insert(std::make_pair(info_g, hType));
				}

				return hType;
			}
		}
		else
		{
		}

		return nullptr;

		/*			DIE_ChildIterator j(i, true);

					switch (j.tag())
					{
					case DW_TAG_union_type:
						//new Scope(j, rICb, SCOPE_UNION);
						break;
					case DW_TAG_class_type:
					case DW_TAG_structure_type:
					case DW_TAG_namespace://in namespace
						//							new Scope(j, rICb, SCOPE_STRUC);
						break;
					case DW_TAG_typedef:
						fetchTypedef(j, rICb);
						break;
					case DW_TAG_member:
						fetchMember(j, rICb);
						break;
					case DW_TAG_inheritance:
					case DW_TAG_template_type_parameter:
					case DW_TAG_template_value_parameter:
					case DW_TAG_subprogram:
					case DW_TAG_enumeration_type:
						break;
						//for namespaces
					case DW_TAG_imported_declaration:
					case DW_TAG_variable:
					case DW_TAG_imported_module:
						break;
					default:
						STOP
					}
					*/
	}

	union MyValue { uint64_t u64; struct { uint32_t u32, u32h; }; };
	HFIELD fetchEnumerator(const DIE_Iterator& i, I_ModuleCB& rICb)
	{
CHECK(i.dieLower() == 0x8c0f1)
STOP
		assert(i.tag() == DW_TAG_enumerator);
		DIE_AttributeReader dar(i);
		MyValue value;
		if (dar.seekAttr(DW_AT_const_value) && dar.readConstant(value.u64))
		{
			if (value.u32h != 0)//uValue & 0xFFFFFFFF00000000)
				return nullptr;//LATER!
			std::ostringstream ss;
			dar.fetchAttrName(ss);
			return rICb.declEField(ss.str().c_str(), (ADDR)value.u32);
		}
		return nullptr;
	}
	HFIELD fetchMember(const DIE_Iterator& i, I_ModuleCB& rICb, bool bUnion)
	{
CHECK(i.dieLower() == 0x38b)
STOP
		assert(i.tag() == DW_TAG_member || i.tag() == DW_TAG_inheritance);
		DIE_AttributeReader dar(i);
		unsigned uOffset(0);
		if (!bUnion)
		{
			if (!dar.seekAttr(DW_AT_data_member_location))
				return nullptr;
			if (dar.skipBlockSize())
			{
				DW_OP op((DW_OP)dar.template read<uint8_t>());
				if (op != DW_OP_plus_uconst)
					return nullptr;
				uOffset = dar.template read_ULEB128<unsigned>();
			}
			else if (!dar.readConstant(uOffset))
				return nullptr;
		}
		AttrIdEnum eAttr(i.tag() == DW_TAG_inheritance ? ATTRC_HEIR : ATTR_NULL);
		std::ostringstream ss;
		dar.fetchAttrName(ss);
		std::string s(ss.str());
		if (eAttr == ATTR_NULL && dar.seekAttr(DW_AT_artificial))
			if (s.rfind("_vptr.", 0) == 0)//startsWith
				eAttr = ATTRC_VPTR;
		HTYPE hType(nullptr);
		if (eAttr != ATTRC_VPTR)
		if (dar.seekAttr(DW_AT_type))
		{
			uint32_t uType;
			if (dar.readReference(uType))
			{
				DIE_Iterator k(m_itCU, uType);
				hType = fetchType(k, rICb);
			}
		}
		if (!hType)
			hType = rICb.type(OPTYP_VOID);
		if (bUnion)
			return rICb.declUField(s.c_str(), hType, eAttr);
		return rICb.declField(s.c_str(), hType, eAttr, uOffset);
	}
	HTYPE fetchType(const DIE_Iterator& i, I_ModuleCB& rICb)//THE TYPES ARE NOT DECLARED IN CURENT SCOPE!
	{
//CHECK(i.dieLower() == 0x123)
//STOP
		switch (i.tag())
		{
		case DW_TAG_base_type:
			return fetchBaseType(i, rICb);
		case DW_TAG_pointer_type:
		case DW_TAG_reference_type:
		case DW_TAG_rvalue_reference_type:
			return fetchPointerType(i, rICb);
		case DW_TAG_structure_type:
		case DW_TAG_class_type:
			return fetchCompoundType(i, rICb);
		case DW_TAG_enumeration_type:
			return fetchEnumerationType(i, rICb);
		case DW_TAG_subroutine_type:
			return nullptr;//LATER!
		case DW_TAG_union_type:
			return fetchCompoundType(i, rICb);
		case DW_TAG_typedef:
			return fetchTypedef(i, rICb);
		case DW_TAG_const_type:
			return fetchConstType(i, rICb);
		case DW_TAG_volatile_type:
			return fetchVolatileType(i, rICb);
		case DW_TAG_array_type:
			return fetchArrayType(i, rICb);
		default:
			break;
		}
		return nullptr;
	}
	HTYPE declTypedef(const DIE_Iterator& i, I_ModuleCB& rICb)//in scope!
	{
//CHECK(i.dieLower() == 0x475665)
//STOP
		assert(i.tag() == DW_TAG_typedef);
		DIE_AttributeReader dar(i);

		std::ostringstream ss;
		if (dar.fetchAttrName(ss))//not a full name, not supposed to contain '::'
		{
			uint32_t uType;
			if (dar.seekAttr(DW_AT_type) && dar.readReference(uType))
			{
				DIE_Iterator j(i.cu(), uType);
				HTYPE hBase(fetchType(j, rICb));
				if (hBase)
				{
					selectFile(fetchFileInfo(i), rICb);
					return rICb.declTypedef(ss.str().c_str(), hBase);
				}
			}
		}
		return nullptr;
	}
	HTYPE fetchTypedef(const DIE_Iterator& i, I_ModuleCB& rICb)//global
	{
		assert(i.tag() == DW_TAG_typedef);
		DIE_AttributeReader dar(i);
		std::ostringstream ss;
		ss << "::";
		HTYPE hType(nullptr);
		if (dar.fetchAttrFullName(ss))
		{
			//hType = rICb.type(ss.str().c_str());//should be able to locate a scoped object!
			//if (!hType)
			{
				//no such type yet - try to create one
				uint32_t uType;
				if (dar.seekAttr(DW_AT_type) && dar.readReference(uType))
				{
					DIE_Iterator j(i, uType);
					HTYPE hBase(fetchType(j, rICb));
					if (hBase)
					{
						selectFile(fetchFileInfo(i), rICb);
						hType = rICb.declTypedef(ss.str().c_str(), hBase);
					}
				}
			}
		}
		return hType;
	}
	HTYPE fetchConstType(const DIE_Iterator& i, I_ModuleCB& rICb)
	{
//CHECK(i.dieLower() == 0x1245e)
//STOP
		assert(i.tag() == DW_TAG_const_type);
		DIE_AttributeReader dar(i);
		uint32_t uType;
		if (dar.seekAttr(DW_AT_type) && dar.readReference(uType))
		{
			DIE_Iterator j(i, uType);
			HTYPE hType(fetchType(j, rICb));//LATER:a real const type
			if (hType)
				return rICb.constOf(hType);
		}
		return nullptr;
	}
	HTYPE fetchVolatileType(const DIE_Iterator& i, I_ModuleCB& rICb)
	{
//CHECK(i.dieLower() == 0x1245e)
//STOP
		assert(i.tag() == DW_TAG_volatile_type);
		DIE_AttributeReader dar(i);
		uint32_t uType;
		if (dar.seekAttr(DW_AT_type) && dar.readReference(uType))
		{
			DIE_Iterator j(i, uType);
			return fetchType(j, rICb);//LATER:a real volatile type
		}
		return nullptr;
	}
	HTYPE fetchCompoundType(const DIE_Iterator& i, I_ModuleCB& rICb)
	{
		SCOPE_enum eScope;
		if (i.tag() == DW_TAG_structure_type || i.tag() == DW_TAG_class_type)
			eScope = SCOPE_STRUC;
		else if (i.tag() == DW_TAG_union_type)
			eScope = SCOPE_STRUC;//SCOPE_UNION;
		else
			return nullptr;
CHECK(i.dieLower() == 0x4aeb60)
STOP

		HTYPE hType(nullptr);
		std::ostringstream ss;
		ss << "::";//a type in global scope
		DIE_AttributeReader dar(i);
		if (dar.fetchAttrFullName(ss))
		{
			hType = rICb.type(ss.str().c_str());//shuld be able to locate a scoped object!
			if (!hType)
			{
				//no such type yet - try to create one
				hType = rICb.NewScope(ss.str().c_str(), eScope);
				if (hType)
					rICb.Leave();
			}
		}
		return hType;
	}
	HTYPE fetchEnumerationType(const DIE_Iterator& i, I_ModuleCB& rICb)
	{
		assert(i.tag() == DW_TAG_enumeration_type);
		std::ostringstream ss;
		ss << "::";//a type in global scope
		DIE_AttributeReader dar(i);
		if (dar.fetchAttrFullName(ss))
		{
			HTYPE hType(rICb.type(ss.str().c_str()));//should be able to locate a scoped object!
			if (!hType)
			{
				//no such type yet - try to create one
				hType = rICb.NewScope(ss.str().c_str(), SCOPE_ENUM);
				if (hType)
					rICb.Leave();
			}
			if (hType)
			{
				OpType_t optype(TYPEID_INT);
				uint8_t uSize;
				if (dar.seekAttr(DW_AT_byte_size) && dar.readConstant(uSize))
				{
					switch (uSize)//signed?
					{
					case 1: optype = TYPEID_UINT8; break;
					case 2: optype = TYPEID_USHORT; break;
					case 4: optype = TYPEID_ULONG; break;
					default: assert(uSize == 4);
					}
				}
				return rICb.enumOf(hType, optype);
			}
		}
		return nullptr;
	}
	HTYPE fetchArrayType(const DIE_Iterator& i, I_ModuleCB& rICb)
	{
CHECK(i.dieLower() == 0x16a69c)
STOP
		assert(i.tag() == DW_TAG_array_type);
		DIE_AttributeReader dar(i);
		uint32_t uType;
		HTYPE hType(nullptr);
		if (dar.seekAttr(DW_AT_type) && dar.readReference(uType))
		{
			DIE_Iterator j(i, uType);
			hType = fetchType(j, rICb);
			if (hType)
			{
				for (DIE_ChildIterator j(i); j; ++j)//shouldn't it be reversed?
				{
					if (j.tag() == DW_TAG_subrange_type)
					{
						DIE_AttributeReader darj(j);
						uint32_t uLower(0), uUpper(0);
						if (darj.seekAttr(DW_AT_lower_bound))
							darj.readConstant(uLower);
						if (darj.seekAttr(DW_AT_upper_bound))
							darj.readConstant(uUpper);
						uint32_t uRange(uUpper - uLower + 1);
						if ((hType = rICb.arrayOf(hType, uRange)) == HTYPE())
							break;//?
					}
				}
			}
		}
		return hType;
	}
	HTYPE fetchBaseType(const DIE_Iterator& k, I_ModuleCB& rICb)
	{
		assert(k.tag() == DW_TAG_base_type);
		DIE_AttributeReader dar(k);
		uint8_t uSize, uEncoding;
		if (dar.seekAttr(DW_AT_byte_size) && dar.readConstant(uSize))
			if (dar.seekAttr(DW_AT_encoding) && dar.readConstant(uEncoding))
			{
				switch ((DW_ATE)uEncoding)
				{
				case DW_ATE_address:
					break;
				case DW_ATE_complex_float:
					break;
				case DW_ATE_float:
					switch (uSize) {
					case 4: return rICb.type(TYPEID_FLOAT);
					case 8: return rICb.type(TYPEID_DOUBLE);
					}
					break;
				case DW_ATE_signed:
				case DW_ATE_signed_char:
					switch (uSize) {
					case 1: return rICb.type(TYPEID_CHAR);
					case 2: return rICb.type(TYPEID_SHORT);
					case 4: return rICb.type(TYPEID_LONG);
					case 8: return rICb.type(TYPEID_LONGLONG);
					}
					break;
				case DW_ATE_boolean:
				case DW_ATE_unsigned:
				case DW_ATE_unsigned_char:
					switch (uSize) {
					case 1: return rICb.type(TYPEID_UINT8);
					case 2: return rICb.type(TYPEID_USHORT);
					case 4: return rICb.type(TYPEID_ULONG);
					case 8: return rICb.type(TYPEID_ULONGLONG);
					}
					break;
				}
			}
		return nullptr;
	}
	HTYPE fetchPointerType(const DIE_Iterator& i, I_ModuleCB& rICb)
	{
		assert(i.tag() == DW_TAG_pointer_type || i.tag() == DW_TAG_reference_type || i.tag() == DW_TAG_rvalue_reference_type);
		DIE_AttributeReader dar(i);
		uint8_t uSize;
		if (dar.seekAttr(DW_AT_byte_size) && dar.readConstant(uSize))
		{
			I_Module::PTR_TYPE_t eType(I_Module::PTR_AUTO);
			I_Module::PTR_MODE_t eMode(I_Module::PTR_MODE_PTR);
			if (uSize == 4)
				eType = I_Module::PTR_32BIT;
			else if (uSize == 8)
				eType = I_Module::PTR_64BIT;
			if (eType != I_Module::PTR_AUTO)
			{
				HTYPE pPointee(nullptr);
				if (dar.seekAttr(DW_AT_type))
				{
					uint32_t uType;
					if (dar.readReference(uType))
					{
						DIE_Iterator j(i, uType);
						pPointee = fetchType(j, rICb);
						if (pPointee)
						{
							if (i.tag() == DW_TAG_reference_type)
								eMode = I_Module::PTR_MODE_REF;
							else if (i.tag() == DW_TAG_rvalue_reference_type)
								eMode = I_Module::PTR_MODE_RVREF;
							//else if (attr.get().ptrmode == CV_PTR_MODE_RVREF)
							//eMode = I_Module::PTR_MODE_RVREF;
						}
						else
						{
							STOP
						}
					}
				}
				return rICb.ptrOf(pPointee, eType, eMode);
			}
		}
		return nullptr;
	}
};



