#include <sstream>
#include <vector>
#include <algorithm>
#include "qx/MyStream.h"
#include "shared.h"
#include "format.elf.h"
#include "format.dwarf.h"
#include "decode.DWARF.h"
//#include "libiberty/demangle.h"
#include "demanglers/demanglers.h"
#include "interface/IADCFront.h"


typedef const char *	PBYTE;



#if(0)
#define DBG_BREAK	break
#else
#define DBG_BREAK	STOP
#endif


enum {
	ELF_META_DIE2ABBREV,//0: mapping of DWARF DIE (offset) to ABBREV (offset)
	__ELF_META_END
	//..
};

// simple types
/*class DataFetchLEB128_t : public DataFetchPtr_t<char>
{
public:
	DataFetchLEB128_t(const I_DataSourceBase &r, size_t _cpr)
		: DataFetchPtr_t(r, _cpr)
	{
	}
	void skip()
	{
		skip_LEB128();
		//mRawOffs += dwarf::sizeof_LEB128(mr, mRawOffs);
		if (isValid())
			if (fetch(mRawOffs) != sizeof(char))
				mRawOffs = OFF_NULL;//make it invalid - do not throw(!)//throw(-346);
	}
	template <typename T>
	T read() const
	{
		DataFetch_t<T> a(mr, mRawOffs);
		return a.get();
	}
};*/

template <typename T_ELF>
class ELF_DebugInfo_t : public dwarf::DebugInfo<T_ELF>
{
	typedef typename T_ELF::MyShdr	MyShdr;
public:
	ELF_DebugInfo_t(const T_ELF& r)
		: dwarf::DebugInfo<T_ELF>(r,
			fromSection(r, ".debug_info"),
			fromSection(r, ".debug_abbrev"),
			fromSection(r, ".debug_line"),
			fromSection(r, ".debug_str")
			)
	{
	}
private:
	static DataSubSource_t fromSection(const T_ELF&h, const char* name)
	{
		const MyShdr* psec(h.FindSectionHeader(name));
		if (!psec)
			throw dwarf::exception("no debug info in ELF");
		return DataSubSource_t(h.data(), psec->sh_offset, psec->sh_size);
	}
};

template <typename T>
class ELF2_t : public ELF_t<T>
{
 public:
	 typedef ELF_DebugInfo_t<ELF_t<T>>	MyDebugInfo;
	 ELF2_t(const I_DataSourceBase& aRaw)
		 : ELF_t<T>(aRaw)
	 {
	 }
 };


template <class T_ELF>
class ELF_Formatter_t : public T_ELF,
	public ELF_Strucs_t
{
	I_DebugInfoDumper *mpDbgInfoDumper;//shared between 2 stages
	HTYPE mhRangeSet;

public:
	//typedef typename T_ELF	MyElf;
	using typename T_ELF::MyDyn;
	using typename T_ELF::MyRel;
	using typename T_ELF::MyRela;
	using typename T_ELF::MySym;
	using typename T_ELF::MyOff;
	using typename T_ELF::MyPhdr;
	using typename T_ELF::MyHalf;
	using typename T_ELF::MyEhdr;
	using typename T_ELF::MyAddr;
	using typename T_ELF::MyShdr;
	using typename T_ELF::Verneed;
	using typename T_ELF::Vernaux;
	using typename T_ELF::DynsymIterator;
	using typename T_ELF::RelIterator;
	typedef ELF_DebugInfo_t<T_ELF>	MyDebugInfo;
	//using typename MyDebugInfo::CU_Iterator;

	using T_ELF::IsInside;
	using T_ELF::GetSectionHeader;
	using T_ELF::ProgramHeader;
	using T_ELF::FindSectionHeader;
	using T_ELF::IsMappedIntoSection;
	using T_ELF::IBASE;
	using T_ELF::Is64bit;
	using T_ELF::mb64bit;
	using T_ELF::VA2RVA;
	using T_ELF::FP2ADDR;
	using T_ELF::mbSwapBytes;
	//using T_ELF::template _uform;
	template <typename U> U _X(U t) const { return T_ELF::_X(t); }

	//template <typename V, typename U> V _uform(U u) const

public:
	ELF_Formatter_t(const I_DataSourceBase &aRaw)//, I_Module &rMain)
		: T_ELF(aRaw),
		ELF_Strucs_t(Is64bit()),
		mpDbgInfoDumper(nullptr),
		mhRangeSet(nullptr)
	{
	}
	~ELF_Formatter_t()
	{
		if (mpDbgInfoDumper)
			mpDbgInfoDumper->Release();
	}

	//HTYPE top() const { return m_top; }
	const char *name() const { return "ELF32"; }

	//convenience overrides (unused)
	//HTYPE type(OpType_t n){ return mr.type(n); }
	//HTYPE type(HNAME h){ return mr.type(h); }
	//HTYPE arrayOf(HTYPE h, unsigned n){ return mr.arrayOf(h, n); }
	//HTYPE enumOf(HTYPE h, OpType_t i){ return mr.enumOf(h, i); }
	//HFIELD	declField(HNAME n, HTYPE t, AttrIdEnum a){ return mr.declField(n, t, a); }
	//HFIELD	declBField(HNAME n, HTYPE t, AttrIdEnum a, ADDR at){ return mr.declBField(n, t, a, at); }
	//HFIELD	declEField(HNAME n, ADDR u, AttrIdEnum a){ return mr.declEField(n, u, a); }


	void declareDynamicTypes(I_SuperModule &mr)
	{
		DeclareUDis86Types(mr);
		DeclareCapstoneTypes(mr);
	}

	void preformat(I_SuperModule &mr, unsigned long dataSize)
	{
		//PE_t &pe(*this);
		//int flavor;
		//if (!(flavor = pe.CheckWindowsPortableExecutable()))
		//return;

		unsigned flags0(mb64bit ? I_SuperModule::ISEG_64BIT : I_SuperModule::ISEG_32BIT);
		if (mbSwapBytes)
			flags0 |= I_SuperModule::ISEG_MSB;

		mhRangeSet = mr.NewRangeSet(IBASE(), "primary");

		ADDR rvaImage(0);// (ADDR)(RVA2VA(0)));//a trick to acquire RVA of an image

		const MyEhdr &PH(ProgramHeader());

		HTYPE m_top;
		if (m_top = mr.AddSubRange(mhRangeSet, rvaImage, 0, mr.NewSegment(dataSize, name(), flags0)))
		//if (m_top = mr.NewSegment(dataSize, name(), flags0))
		{
			SAFE_SCOPE_HERE(mr);
			__SafeAuxData<__ELF_META_END> meta(mr);

			//IMAGE_DOS_HEADER &idh(pe.GetImageDosHeader());
			if (!mb64bit)
			{
				mr.installFrontend(_PFX("ELF32Front"));
			}
			else
			{
				mr.installFrontend(_PFX("ELF64Front"));
			}

			mr.installNamespace();
			mr.installTypesMgr();

			if (mr.EnterAttic())
			{
				createElfStructures(mr);
				DWARF_CreateStructures(mr);
				mr.Leave();
			}

			declareDynamicTypes(mr);

			switch (_X(PH.e_machine))
			{
			case EM_386:
				if (!mb64bit)
					mr.setDefaultCodeType(mr.type(_PFX("UDIS86_32")));
				break;
			case EM_X86_64:
				if (mb64bit)
					mr.setDefaultCodeType(mr.type(_PFX("UDIS86_64")));
				break;
			case EM_ARM:
				if (!mb64bit)
					mr.setDefaultCodeType(mr.type(_PFX("CAPSTONE_ARM")));
				break;
			case EM_AARCH64:
				if (mb64bit)
					mr.setDefaultCodeType(mr.type(_PFX("CAPSTONE_AARCH64")));
				break;
			default:
				break;
			}

			mr.declField("ELF_Header", mr.type(mb64bit ? "Elf64_Ehdr" : "Elf32_Ehdr"));

			HTYPE hPhdr(mr.type(mb64bit ? "Elf64_Phdr" : "Elf32_Phdr"));
			
			mr.setcp((POSITION)FP2ADDR(_X(PH.e_phoff)));//(POSITION)RVA2VA(_uform<ADDR>(PH.e_phoff)));

#if(0)//program segments
			auto e_entry(_X(PH.e_entry));
			auto e_phnum(_X(PH.e_phnum));
			for (int i(0); i < e_phnum; i++)//fake section at index 0
			{
				MyPhdr *phdr(GetProgramSectionHeader(i));
				assert(phdr);
				if (mr.EnterScope(_uform<ADDR>(phdr->p_offset)))
				{
					try 
					{
						unsigned rawSize(_uform<unsigned>(phdr->p_filesz));
						ADDR addrV(_uform<ADDR>(phdr->p_vaddr));
						ADDR sizeV(_uform<ADDR_RANGE>(phdr->p_memsz));
						if (mr.AddSubRange(mhRangeSet, addrV, sizeV, mr.NewSegment(rawSize, nullptr, flags0)))
						{
							if (e_entry != 0)
								if (addrV <= e_entry && e_entry < addrV + sizeV)
									mr.setEntryPoint(_uform<ADDR>(e_entry));
							mr.Leave();
						}
					}
					catch(...){}

					mr.Leave();
				}
			}
#endif

			//////////////////////////////////////////////
			// create program headers table

			if (PH.e_phoff != 0)
			{
				if (mr.EnterScope(FP2ADDR(_X(PH.e_phoff))))// RVA2VA(_uform<ADDR>(e_phoff))))
				{
					//program header padding
					MyHalf e_phentsize(_X(PH.e_phentsize));
					MyHalf e_phnum(_X(PH.e_phnum));
					if (e_phentsize == sizeof(MyPhdr))
					{
						MyHalf p_padding(e_phentsize - sizeof(MyPhdr));
					//if (p_padding == 0)
						mr.declField("ELF_Phdrs", mr.arrayOf(hPhdr, e_phnum));
					}
					else
					{
						for (int i(0); i < e_phnum; i++)
						{
							char buf[32];
							sprintf(buf, "ELF_Phdr#%d", i);
							mr.declField(buf, hPhdr);
						}
					}
					mr.Leave();
				}
			}

			//////////////////////////////////////////////////////
			// create sections headers table

			mr.setcp((POSITION)_X(PH.e_shoff));// (POSITION)RVA2VA(_uform<ADDR>(PH.e_shoff)));

			HTYPE hShdr(mr.type(mb64bit ? "Elf64_Shdr" : "Elf32_Shdr"));

			//section header padding
			int sh_padding(_X(PH.e_shentsize) - sizeof(MyShdr));
			if (sh_padding == 0)
				mr.declField("ELF_Shdrs", mr.arrayOf(hShdr, _X(PH.e_shnum)));
			else
				for (int i(0); i < _X(PH.e_shnum); i++)
				{
					char buf[32];
					sprintf(buf, "ELF_Shdr#%d", i);
					mr.declField(buf, hShdr);
				}

			/////////////////////////////////////////////////
			createSections(mr, flags0, meta);
			//processImportExports(mr);
			//formatGOTSections(mr);
			formatExports(mr);
			formatImports(mr);
		}
	}

protected:

	void formatImports(I_SuperModule& mr)
	{
		// The only link between location of the import entry and its name (in this module) can be retrived through a relocation table;
		// A relocation table entry contains 2 pieces of usefull information:
		//	1) address of importing symbol;
		//	2) index of the symbol in .dynsym table (referencing a name);
		//.rel.plt
		//		\_ offset => .got (ptr to symbol to be resolved) 
		//		 \_ sym => .dynsym                           \
		//                      \_ name => .dynstr            \
		//                       \_ value => __________________\_ => .plt {jmp GOT[x]} => .got {push y; push GOT[1]; jmp GOT[2]}

		const MyShdr *pshGot(FindSectionHeader(".got.plt"));
		if (!pshGot)
			pshGot = FindSectionHeader(".got");
		if (!pshGot)
			return;
		const MyShdr *pshPlt(FindSectionHeader(".plt"));
		DynsymIterator dynIt(*this);
		for (RelIterator relIt(*this, *pshGot); relIt; ++relIt)
		{
			MyAddr va(relIt.r_offset());//in GOT section
//CHECK(va == 0x80B9430)
//STOP
			int iDynsym((int)relIt.r_sym());//index of .dynsym element
			if (!dynIt.locate(iDynsym))
				continue;

			std::ostringstream ss;
			dynIt.namex(ss);
			MyString s(ss.str());
			ss << '\n';
			dynIt.tagx(ss);
			ss << '\n';
			dynIt.modulex(ss);
			ADDR rva(VA2RVA(va));
			if (mr.EnterSegment(mhRangeSet, rva))
			{
				SAFE_SCOPE_HERE(mr);

				//get address of initial resolution thunk (this cell is about to be overwritten)
				DECLDATA(MyOff, va2);

				MyAddr vad(dynIt.st_value());
				bool bImported(vad == 0 || (pshPlt && IsInside(*pshPlt, vad)));
				if (bImported)
					mr.declField(ss.str().c_str(), mr.impOf(mr.ptrOf(nullptr)));
				else
					mr.declField(s.c_str(), mr.expOf(mr.ptrOf(nullptr)));//name is taken by exporting entity

				if (va2 != 0 && mr.EnterSegment(mhRangeSet, VA2RVA(va2)))//this likely falls into a .plt section
				{
					SAFE_SCOPE_HERE(mr);
					mr.declCField(nullptr, I_Module::CODE_TYPE_THUNK);
				}
			}
			else
				fprintf(stdout, "formatImports: WTF@%d: %s\n", iDynsym, ss.str().c_str());
		}
	}

	void formatExports(I_SuperModule& mr)// +format .plt section (with thunks)
	{
		const MyShdr *pshPlt(FindSectionHeader(".plt"));

		size_t i(0);
		for (DynsymIterator dynIt(*this); dynIt; ++dynIt, ++i)
		{
			bool bFunc(dynIt.st_type() == STT_FUNC);
			if (bFunc || dynIt.st_type() == STT_OBJECT)//st_type
			{
				MyAddr va(dynIt.st_value());
				if (va == 0)
					continue;

				ADDR rva(VA2RVA(va));
				if (pshPlt && IsInside(*pshPlt, va))//imported?
				{
					if (mr.EnterSegment(mhRangeSet, VA2RVA(va)))//this will probably fall into .plt section
					{
						SAFE_SCOPE_HERE(mr);
						mr.declCField(nullptr, I_Module::CODE_TYPE_THUNK);//no name for a thunk
					}
				}
				else
				{
					std::ostringstream ss;
					dynIt.namex(ss);
					ss << '\n';
					dynIt.tagx(ss);
					ss << '\n';
					if (mr.EnterSegment(mhRangeSet, rva))
					{
						SAFE_SCOPE_HERE(mr);
						if (bFunc)
							mr.declCField(ss.str().c_str(), I_Module::CODE_TYPE_THUNK, ATTR_EXPORTED);//may become a proc
						else
							mr.declField(ss.str().c_str(), nullptr, ATTR_EXPORTED);
					}
					else
						fprintf(stdout, "formatExports: WTF@%d: %s\n", int(i), ss.str().c_str());
				}
			}
		}
	}

	typedef AuxData_t<__ELF_META_END> MyAux;

	void createSections(I_SuperModule &mr, unsigned flags0, MyAux &meta)
	{
		const MyEhdr &PH(ProgramHeader());
		//get string table section
		const MyShdr* shdr_str(GetSectionHeader(_X(PH.e_shstrndx)));

		for (int i(1); i < _X(PH.e_shnum); i++)//fake section at index 0
		{
			//if (i > 38)	break;
//CHECK(i==16)
//STOP

			const MyShdr& shdr(*GetSectionHeader(i));

			//check overlapped sections - pick a larger one, discard a smaller
			if (IsMappedIntoSection(i) != 0)
				continue;

			HTYPE hRangeSet2(mhRangeSet);

			std::ostringstream name;
			this->GetSectionName(shdr, name);
			/*if (mr.EnterScope(FP2ADDR(_X(shdr_str->sh_offset) + _X(shdr.sh_name))))//rva2va?
			{
				DataStream_t pName(mr, mr.cpr());
				pName.fetchString(name);
				mr.Leave();
			}*/

			ENTER_SCOPE(mr, (ADDR)_X(shdr.sh_offset))//rva2va?
			{
				// check if the section falls into any segment
				if (!this->IsMappedIntoSegment(shdr))
				{
					//fprintf(stderr, "Warning: Section #%d (%s) orphaned\n", i, name ? name : "?");
					hRangeSet2 = nullptr;
					//mr.Leave();
					//continue;
				}

				//				char buf[32];
				//			sprintf(buf, "seg_%X", shdr.sh_addr);
				//		name = buf;

				//CHECK(shdr.sh_addr == 0x080D2940)
				//STOP

				unsigned rawSize(_X(shdr.sh_type) == SHT_NOBITS ? 0 : this->template _uform<unsigned>(shdr.sh_size));
				/*if (!_X(shdr.sh_addr))//no virtual base
				{
					if (mr.NewScope(mr.declField(name)))
					{
						mr.setSize(_uform<unsigned>(shdr.sh_size));
						if (_X(shdr.sh_type) == SHT_STRTAB)
						{
							format_string_table(mr, _uform<unsigned>(shdr.sh_size));
						}
						mr.Leave();
					}
				}
				else*/
				{
					try {
						if (shdr.sh_addr != 0)
						{
							if (mr.AddSubRange(hRangeSet2, VA2RVA(_X(shdr.sh_addr)), (ADDR_RANGE)_X(shdr.sh_size), mr.NewSegment(rawSize, name.str().c_str(), flags0)))
							{
								SAFE_SCOPE_HERE(mr);
								if (IsInside(shdr, PH.e_entry))
								{
									//mr.enqueEntryPoint(VA2RVA(_X(PH.e_entry)));
									if (mr.EnterScope(VA2RVA(_X(PH.e_entry))))
									{
										SAFE_SCOPE_HERE(mr);
										mr.declCField(nullptr);
									}
								}

								createSection(mr, shdr, name.str(), meta);
							}
						}
						else
						{
							if (mr.NewScope(mr.declField(name.str().c_str())))
							{
								SAFE_SCOPE_HERE(mr);
								createSection(mr, shdr, name.str(), meta);
							}
						}
					}
					catch (int)
					{
						STOP
							//mr.error("processing section");
					}
				}
			}
			EXIT_SCOPE(mr)//leave
		}
	}

	void createSection(I_SuperModule &mr, const MyShdr& shdr, const std::string& name, MyAux &meta)
	{
		auto sh_type(_X(shdr.sh_type));
		if (sh_type == SHT_STRTAB)
		{
			format_string_table(mr, this->template _uform<unsigned>(shdr.sh_size));
		}
		else if (sh_type == SHT_SYMTAB)
		{
			unsigned n(this->template _uform<unsigned>(shdr.sh_size) / sizeof(MySym));
			if (n > 0)
				mr.declField("Sym_Table", mr.arrayOf(mr.type(mb64bit ? "Elf64_Sym" : "Elf32_Sym"), n));
		}
		else if (sh_type == SHT_DYNAMIC)
		{
			unsigned n((unsigned)this->DynamicTableSize());
			if (n > 0)
				mr.declField("Dyn_Table", mr.arrayOf(mr.type(mb64bit ? "Elf64_Dyn" : "Elf32_Dyn"), n));
		}
		else if (sh_type == SHT_DYNSYM)
		{
			unsigned n(this->template _uform<unsigned>(shdr.sh_size) / sizeof(MySym));
			if (n > 0)
				mr.declField("DynSym_Table", mr.arrayOf(mr.type(mb64bit ? "Elf64_Sym" : "Elf32_Sym"), n));
		}
		else if (sh_type == SHT_REL)
		{
			unsigned n(this->template _uform<unsigned>(shdr.sh_size) / sizeof(MyRel));
			if (n > 0)
			{
				if (mb64bit)
					mr.declField("Rel_Table", mr.arrayOf(mr.type("Elf64_Rel"), n));
				else
					mr.declField("Rel_Table", mr.arrayOf(mr.type("Elf32_Rel"), n));
			}
		}
		else if (sh_type == SHT_RELA)
		{
			unsigned n(this->template _uform<unsigned>(shdr.sh_size) / sizeof(MyRela));
			if (n > 0)
				mr.declField("Rela_Table", mr.arrayOf(mr.type(mb64bit ? "Elf64_Rela" : "Elf32_Rela"), n));
		}
		else if (name == ".got")
		{
			//all sections must be created before formatting GOT sections
			/*unsigned n(this->template _uform<unsigned>(shdr.sh_size) / sizeof(MyOff));
			if (n > 0)
				mr.declField("Got_Table", mr.arrayOf(mr.ptrOf(nullptr), n));*/
			//format_sec_got_plt(mr, shdr);
		}
		else if (name == ".got.plt")
		{
			//format_sec_got_plt(mr, shdr);
		}
		else if (name == ".gnu.version")
		{
			unsigned n(this->template _uform<unsigned>(shdr.sh_size) / sizeof(MyHalf));
			if (n > 0)
				mr.declField("SymVer_Table", mr.arrayOf(mr.type(_Half), n), ATTR_DECIMAL);
		}
		else if (name == ".gnu.version_d")
		{
			mr.declField("Verdef", mr.type("Elf_Verdef"));
		}
		else if (name == ".gnu.version_r")
		{
			for (size_t i(0); i < this->DynamicTableSize(); i++)
			{
				const MyDyn& a(this->DynamicTableEntryAt(i));
				if (a.d_tag == DT_VERNEEDNUM)
				{
					for (size_t j(0); j < a.d_un.d_val; j++)
					{
						DECLDATAEX(Verneed, aVerneed);
						mr.declField("Verneed", mr.type("Elf_Verneed"));
						if (aVerneed.vn_cnt > 0)
							mr.declField("Vernaux", mr.arrayOf(mr.type("Elf_Vernaux"), aVerneed.vn_cnt));
					}
					break;
				}
			}
		}
		else if (name == ".debug_info")
		{
			format_sec_debug_info(mr, shdr, meta);
		}
		else if (name == ".debug_abbrev")
		{
			format_sec_debug_abbrev(mr);
		}
		else if (name == ".debug_str")
		{
			format_sec_debug_str(mr, shdr);
		}
		else if (name == ".debug_pubnames" || name == ".debug_pubtypes")
		{
			format_sec_debug_pubxx(mr, shdr);
		}
		else if (name == ".debug_line")
		{
			format_sec_debug_line(mr, shdr);
		}
		else if (name == ".debug_loc")
		{
			format_sec_debug_loc(mr, shdr);
		}
	}

	/*void processImportExports(I_SuperModule& mr)
	{
		const MyShdr *psh(FindSectionHeader(SHT_DYNSYM));
		if (!psh)
			return;

		std::vector<MyAddr> vImp;
		BuildDynSimImpMap(vImp);

		const MyShdr *pshPlt(FindSectionHeader(".plt"));

		int i(0);
		for (DynsymIterator dynIt(*this); dynIt; ++dynIt, ++i)
		{
			bool bFunc(dynIt.st_type() == STT_FUNC);
			if (bFunc || dynIt.st_type() == STT_OBJECT)//st_type
			{
				std::string sSymbolName(dynIt.name(true));
CHECK(sSymbolName == "_ZN12QMapIteratorI7QStringiE3decEv")
STOP

				MyAddr va(dynIt.st_value());
				bool bImported(false);
				if (vImp[i] != 0)
				{
					//create IMPORTED entry
					va = vImp[i];
					bImported = true;
				}
				else if (va == 0)
				{
					fprintf(stdout, "(!) WTF@%d: %s\n", i, sSymbolName.c_str());
					continue;
				}

				// create EXPORTED entry
				ADDR rva(VA2RVA(va));
				if (mr.EnterSegment(mhRangeSet, rva))
				{
					SAFE_SCOPE_HERE(mr);
					if (bImported)
					{
						mr.declField(sSymbolName.c_str(), mr.impOf(mr.ptrOf(nullptr)));
					}
					else
					{
						if (bFunc)
							mr.declCField(sSymbolName.c_str(), I_Module::CODE_TYPE_THUNK, ATTR_EXPORTED);//may come up a proc
						else
							mr.declField(sSymbolName.c_str(), nullptr, ATTR_EXPORTED);
					}
				}
				else
					fprintf(stdout, "(!!) WTF@%d: %s\n", i, sSymbolName.c_str());
			}
		}
	}
	
	void formatGOTSections(I_SuperModule& mr)
	{
		const MyEhdr& PH(ProgramHeader());
		for (int i(1); i < _X(PH.e_shnum); i++)//fake section at index 0
		{
			const MyShdr& shdr(*GetSectionHeader(i));
			std::ostringstream ss;
			if (GetSectionName(shdr, ss))
			{
				std::string s(ss.str());
				if (s == ".got" || s == ".got.plt")
				{
					if (mr.EnterSegment(mhRangeSet, (ADDR)_X(shdr.sh_addr)))
					{
						SAFE_SCOPE_HERE(mr);
						format_sec_got_plt(mr, shdr);
					}
				}
			}
		}
	}

	void BuildDynSimImpMap(std::vector<MyAddr>& v)
	{
		DynsymIterator dynIt(*this);

		//std::vector<MyAddr> v(dynIt.size());//imported addresses map
		v.resize(dynIt.size());

		const MyShdr *pshPlt(FindSectionHeader(".plt"));

		const MyEhdr& PH(ProgramHeader());
		for (int i(1); i < _X(PH.e_shnum); i++)//fake section at index 0
		{
			const MyShdr& shdr(*GetSectionHeader(i));
			std::ostringstream ss;
			if (GetSectionName(shdr, ss))
			{
				std::string s(ss.str());
				if (s == ".got" || s == ".got.plt")
				{
					for (RelIterator relIt(*this, shdr); relIt; ++relIt)
					{
						MyAddr va(relIt.r_offset());
						int iDynSym((int)relIt.r_sym());//index of .dynsym element
						if (dynIt.locate(iDynSym))
						{
CHECK(iDynSym == 1)
STOP
							MyAddr vad(dynIt.st_value());
							if (vad == 0 || (!pshPlt || IsInside(*pshPlt, vad)))
							{
								if (v[iDynSym] != 0)
									fprintf(stdout, "(!!!) WTF@%d\n", iDynSym);
								v[iDynSym] = va;
							}
						}
					}
				}
			}
		}
	}

	void format_sec_got_plt(I_Module& mr, const MyShdr& shGot)
	{
		const MyShdr *pshPlt(FindSectionHeader(".plt"));

		DynsymIterator dynIt(*this);

		//std::vector<MyAddr> v(dynIt.size());//imported addresses map

		for (RelIterator relIt(*this, shGot); relIt; ++relIt)
		{
			MyAddr va(relIt.r_offset());
			int iDynSym((int)relIt.r_sym());//index of .dynsym element
CHECK(va == 0x80b9430)
STOP
			if (dynIt.locate(iDynSym))
			{
				//v[iDynSym] = va;


				MyAddr vad(dynIt.st_value());
				bool bImported(vad == 0 || (pshPlt && IsInside(*pshPlt, vad)));
				bool bExported(vad != 0 && (!pshPlt || !IsInside(*pshPlt, vad)));
CHECK(bExported)
STOP
				std::string sSymbolName(dynIt.name(true));

				//position at import entry's location
				if (mr.EnterScope(VA2RVA(va)))//all references fall into the current segment
				{
					SAFE_SCOPE_HERE(mr);

					//get address of initial resolution thunk (this cell is about to be overwritten)
					DECLDATA(MyOff, va2);

					if (bExported)
					{
						//create EXPORTED entry
						bool bFunc(dynIt.st_type() == STT_FUNC);
						if (bFunc || dynIt.st_type() == STT_OBJECT)
						{
							ADDR rvad(VA2RVA(vad));
							if (mr.EnterSegment(mhRangeSet, rvad))
							{
								SAFE_SCOPE_HERE(mr);
								if (bFunc)
									mr.declCField(sSymbolName.c_str(), I_Module::CODE_TYPE_THUNK, ATTR_EXPORTED);//if not a thunk - will turn to a proc
								else
									mr.declField(sSymbolName.c_str(), nullptr, ATTR_EXPORTED);
							}
						}
					}

CHECK(va2!=0)
STOP
					if (bImported)
					{
						//create IMPORTED entry
						mr.declField(sSymbolName.c_str(), mr.impOf(mr.ptrOf(nullptr)));
					}
					else
						mr.declField(nullptr, mr.ptrOf(nullptr));//the name is supposed to be taken by exported entry

					if (va2 != 0 && mr.EnterSegment(mhRangeSet, VA2RVA(va2)))//this will probably fall into .plt section - MUST exist!
					{
						SAFE_SCOPE_HERE(mr);
						mr.declCField(nullptr, I_Module::CODE_TYPE_THUNK);
					}
				}
			}
		}
	}*/

	void format_sec_debug_info(I_Module &mr, const MyShdr &shdr, MyAux &meta)
	{
		const MyShdr* psh_abbr(FindSectionHeader(".debug_abbrev"));

		//meda data
		std::vector<dwarf::die2abbrev_t::elt_t>	m;
		m.reserve(1024);

		int cus(0), dies(0);
		MyDebugInfo di(*this);
		try
		{
			for (typename MyDebugInfo::CU_Iterator i(di); i; ++i, cus++)
			{
				assert(i.dieIterator().tag() == DW_TAG_compile_unit);
				//										if (cus > 1)
				//											break;
				mr.setcp((POSITION)i.lowerBound());

				//										if (mr.NewScope(mr.declField(, ATTR_COLLAPSED)))
				{
					mr.declField(nullptr/*"DWARF_CUH_#"*/, mr.type(_PFX("DWARF_CompilationUnit")), ATTR_COLLAPSED);
					//mr.declField("DWARF_CUH_#", mr.type(_PFX("DWARF_CompilationUnitHeader")));
					for (typename MyDebugInfo::CU_Iterator::DIE_Iterator& j(i.dieIterator()); j; ++j, ++dies)
					{
						POSITION a(POSITION(i.dieIterator().dieLower()));
//CHECK(a == 0x2ebe)
//STOP
						POSITION a2(POSITION(shdr.sh_offset + a));
						POSITION b(POSITION(psh_abbr->sh_offset + i.dieIterator().abbrevLower()));
						assert(m.empty() || a2 > m.back().die);
						m.push_back(dwarf::die2abbrev_t::elt_t(a2, b));
						//mr.setcp(a);
						//if (j.level() == 0)
							//mr.declField(nullptr, mr.type(_PFX("DWARF_DebugInformationEntry")));//, ATTR_NULL, a);//cp is no promoted
					}
					STOP
				}
				//DBG_BREAK;
			}
		}
		catch (...)
		{
			STOP
				//mr.error("error caught");
		}
		fprintf(stdout, "[DWARF] elements processed: CU(s): %d, DIE(s): %d\n", cus, dies);

		unsigned aux_sz(unsigned(offsetof(dwarf::die2abbrev_t, a) + m.size() * sizeof(dwarf::die2abbrev_t::elt_t)));
		meta.reserve("DIE2ABBREV", aux_sz, ELF_META_DIE2ABBREV);
		dwarf::die2abbrev_t* pmap(meta.data<dwarf::die2abbrev_t>(ELF_META_DIE2ABBREV));
		pmap->n = unsigned(m.size());
		std::copy(m.begin(), m.begin() + m.size(), &pmap->a[0]);
		//memcpy(pmap, &(m)[0], m.size());
	}

	void format_sec_debug_abbrev(I_Module &mr)
	{
		const MyShdr* psh_inf(FindSectionHeader(".debug_info"));
#if(1)
		MyDebugInfo di(*this);
		for (typename MyDebugInfo::CU_Iterator i(di); i; ++i)
		{
			//WARNING: THIS IS NOT AN ABBREV ITERATOR!
			for (typename MyDebugInfo::CU_Iterator::DIE_Iterator& j(i.dieIterator()); j; ++j)
			{
				POSITION oPos((POSITION)j.abbrevLower());
				mr.setcp(oPos);
CHECK(oPos == 0x189)
STOP
				//IT MAY HAPPEN THE FIELD AT THIS LOCATION ALREADY EXISTS
				mr.declField(nullptr, mr.type(_PFX("DWARF_Abbreviation")), ATTR_COLLAPSED);
			}
			//DBG_BREAK;
		}
#endif
STOP
		/*DataFetch2_t<dwarf::DebugInfoUnitHeader_t> h(mr, psh_inf->sh_offset);

		POSITION beg((POSITION)mr.cpr());
		while (mr.cpr() - beg < h.unit_length)
		{
			DataFetchLEB128_t p(mr, mr.cpr());
			if (!p.read<char>() == 0)
			{
				p.skip();//skip code
				p.skip();//skip tag
				//char hasChildren = p.read<char>();
				mr.declField(nullptr, mr.type(_PFX("DWARF_Abbreviation")), ATTR_COLLAPSED);
			}
			else
			{
				mr.skip(1);
			}
		}*/
	}

	void format_sec_debug_str(I_Module &mr, const MyShdr &shdr)
	{
		format_string_table(mr, this->template _uform<unsigned>(shdr.sh_size));
	}

	void format_sec_debug_pubxx(I_Module &mr, const MyShdr &shdr)
	{
		POSITION end(mr.cp() + this->template _uform<unsigned>(shdr.sh_size));
		while (mr.cp() < end)
		{
			DECLDATA(uint32_t, len);
			POSITION beg2(mr.cp() + sizeof(uint32_t));//skip length
			mr.declField(nullptr, mr.type(_PFX("DWARF_PubNamesHeader")));
//DBG_BREAK;
			while (mr.cp() - beg2 < len)
			{
				DECLDATA(uint32_t, off);
				if (off == 0)
				{
					mr.skip(OPSZ_DWORD);
					break;
				}
				mr.declField(nullptr, mr.type(_PFX("DWARF_PubNamesTuple")));
//DBG_BREAK;
			}
//DBG_BREAK;
		}
	}

	void format_sec_debug_line(I_Module &mr, const MyShdr &shdr)
	{
		//const MyShdr *psh_inf(FindSectionHeader(".debug_info"));
		//POSITION end(mr.cp() + this->template _uform<unsigned>(shdr.sh_size));
		//while (mr.cp() < end)
		{
			MyDebugInfo di(*this);
			for (typename MyDebugInfo::CU_Iterator cui(di); cui; ++cui)
			{
				if (cui.dieIterator().tag() == DW_TAG_compile_unit)
				{
					typename MyDebugInfo::CU_Iterator::DIE_AttributeReader DR(cui.dieIterator());

					POSITION pos;
					if (DR.seekAttr(DW_AT_stmt_list) && DR.readConstant(pos))
					{
						//POSITION pos(POSITION(i.dieIterator().die().compile_unit.stmt_list));
						mr.setcp(pos);
						//mr.declField(nullptr, mr.type(_PFX("DWARF_ProgramStatement")));
						DWARF_Declare_ProgramStatement(mr);
					}
				}
				//											DBG_BREAK;
			}
		}
	}

	void format_sec_debug_loc(I_Module &mr, const MyShdr &shdr)
	{
		POSITION end(mr.cp() + this->template _uform<unsigned>(shdr.sh_size));
		while (mr.cp() < end)
		{
			mr.declField(nullptr, mr.type(_PFX("DWARF_LocationList64")), ATTR_COLLAPSED);
			//										DBG_BREAK;//LATER:crash at the end of section(!)
		}
	}

	void format_string_table(I_Module &mr, unsigned sh_size)
	{
		POSITION cp_beg(mr.cp());
		while (mr.cp() - cp_beg < sh_size)
		{
			//DECLDATAPTR(const char, pName);
			HTYPE h(toAscii(mr, true));
			if (h)
				mr.declField(nullptr, h);
			else
				mr.skip(1);
		}
	}

public:

	void printImp(OFF_t oSymbolName, OFF_t offSym, uint32_t i)
	{
		std::string s(this->offToString(oSymbolName));
		if (offSym != 0)
			fprintf(stdout, "imported entry @ %08X (%d): %s\n", (ADDR)offSym, (int)i, s.c_str());
		else
			fprintf(stdout, "imported entry (%d): %s\n", (int)i, s.c_str());

	}

	void dumpImports(I_Front::I_DumpSymbol* pf, I_Front::Dump_IE_Flags dumpFlags)
	{
		const MyShdr *pshGot(FindSectionHeader(".got.plt"));
		if (!pshGot)
			pshGot = FindSectionHeader(".got");
		if (!pshGot)
			return;
		const MyShdr *pshPlt(FindSectionHeader(".plt"));
		DynsymIterator dynIt(*this);
		for (RelIterator relIt(*this, *pshGot); relIt; ++relIt)
		{
			int iDynsym((int)relIt.r_sym());//index of .dynsym element
			if (!dynIt.locate(iDynsym))
				continue;

			MyAddr vad(dynIt.st_value());
			bool bImported(vad == 0 || (pshPlt && IsInside(*pshPlt, vad)));
			if (!bImported)
				continue;
						
			MyAddr va(relIt.r_offset());
#if(0)
			std::string s(dynIt.name(true));
CHECK(s == "stdin")
STOP
#endif
			pf->setVA(VA2RVA(va));
			std::ostringstream ss;
			dynIt.namex(ss);
			ss << '\n';
			dynIt.tagx(ss);
			ss << '\n';
			dynIt.modulex(ss);
			pf->setName(ss.str().c_str());
			if (dynIt.isFunc())
				pf->setKind(I_Front::SYMK_FUNC);
			pf->flush();
		}
	}

	void dumpExports(I_Front::I_DumpSymbol* pf, I_Front::Dump_IE_Flags dumpFlags)
	{
		const MyShdr *pshPlt(FindSectionHeader(".plt"));

		size_t i(0);
		for (DynsymIterator dynIt(*this); dynIt; ++dynIt, ++i)
		{
			bool bIsFunc(dynIt.st_type() == STT_FUNC);
			if (bIsFunc || dynIt.st_type() == STT_OBJECT)//st_type
			{
				MyAddr va(dynIt.st_value());
				bool bImported(va == 0 || (pshPlt && IsInside(*pshPlt, va)));
				if (bImported)
					continue;

				ADDR rva(VA2RVA(va));
//CHECK(rva == 0xFFC00000)
//STOP
				pf->setVA(rva);
				if (bIsFunc)
					pf->setKind(I_Front::SYMK_FUNC);

				std::ostringstream ss;
				dynIt.namex(ss);
				ss << '\n';
				dynIt.tagx(ss);
				ss << '\n';
				//dynIt.modulex(ss);
				//OFF_t oTag;
				//OFF_t oModuleName(dynIt.moduleOff(oTag));
				pf->setName(ss.str().c_str());
				//pf->setModule(oModuleName);
				pf->flush();
			}
		}
	}

	//template <typename T_ELF>
	class SymbolDumper : public I_DebugInfoDumper
	{
		//typedef typename T_ELF	MyElf;
		typedef typename T_ELF::MyShdr	MyShdr;
		typedef typename T_ELF::MySym	MySym;

		const T_ELF& mrElf;
		DataStream_t mData;
		unsigned mTotal;
		unsigned mCur;
		const MyShdr* mpShdr;
		const MyShdr* mpSShdr;//strings
	public:
		SymbolDumper(const T_ELF& elf, Elf_SHT eSht)
			: mData(elf.data(), OFF_NULL),//invalid at start
			mrElf(elf),
			mTotal(0),
			mCur(0)
		{
			mpShdr = mrElf.FindSectionHeader(eSht);
			if (mpShdr)
			{
				mpSShdr = mrElf.GetSectionHeader(mpShdr->sh_link);
				mTotal = mrElf.template _uform<unsigned>(mpShdr->sh_size) / sizeof(MySym);
				OFF_t oStart((OFF_t)mrElf._X(mpShdr->sh_offset));
				mData.seek(oStart);
			}
		}
		bool processNext(I_ModuleCB& rICb, unsigned& progress)//I_ModuleCB::Dump_D_Flags
		{
			if (mCur >= mTotal)
				return false;

			assert(!mData.isAtEnd());
			MySym r;
			OFF_t oLower(mData.tell());
			mData.read(r);//advance a stream's pointer

			int eType(0);
			if (r.st_type == STT_FUNC)
				eType = 1;
			else if (r.st_type == STT_OBJECT)
				eType = 2;
			if (eType != 0)
			{
				if (r.st_shndx < mrElf.GetSectionsNum())
				{
					rICb.selectFile("from_SYMTAB", nullptr);

					MyAddr va(mrElf._X(r.st_value));
					if (va != 0)
					{
						ADDR rva(mrElf.VA2RVA(va));
//CHECK(rva == 0xFFC00000)
//STOP
						OFF_t oSymbolName(mpSShdr->sh_offset + r.st_name);
						rICb.dump(rva, oSymbolName, 0, eType);
					}

				}
			}
			mCur++;
			progress = TProgress(mCur, mTotal);
			return true;
		}
	};


	I_DebugInfoDumper *createDebugInfoDumper(const char *hint)
	{
		if (!mpDbgInfoDumper)
		{
			try {
				if (hint && hint[0])
				{
					if (strcmp(hint, "dwarf") == 0)
						mpDbgInfoDumper = new MyDwarfDumper<T_ELF>(*this);
					else if (strcmp(hint, "symtab") == 0)
						mpDbgInfoDumper = new SymbolDumper(*this, SHT_SYMTAB);
					else if (strcmp(hint, "dynsym") == 0)
						mpDbgInfoDumper = new SymbolDumper(*this, SHT_DYNSYM);
					else
						fprintf(stderr, "Warning: Unrecognized hint for debug info dumping: %s\n", hint);
				}
			}
			catch (dwarf::exception&)
			{
				return nullptr;//no debug info
			}
		}
		if (mpDbgInfoDumper)
			mpDbgInfoDumper->AddRef();
		return mpDbgInfoDumper;
	}

	I_Front::AKindEnum translateAddress(const I_DataStreamBase &mr, int moduleId, ADDR &addr, AttrIdEnum attr0)
	{
		ELF_AttrIdEnum attr = (ELF_AttrIdEnum)attr0;
		DWARF_AttrIdEnum attr2 = (DWARF_AttrIdEnum)attr0;
		bool b64bit(Is64bit());
//?		assert(!b64bit || attr != ATTR_VA);

		if (attr0 == ATTR_FILEPTR)
			return I_Front::AKIND_RAW;
		if (attr0 == ATTR_VA)
			return I_Front::AKIND_VA;
		if (attr0 == ATTR_RVA)
		{
			//addr = RVA2VA(addr);
			return I_Front::AKIND_VA;
		}
		if (attr == ATTR_ELF_STRING_INDEX0)
		{
			OFF_t fp;
			if (this->GetSHStringTableFP(fp))
			{
				addr += (ADDR)fp;
				return I_Front::AKIND_RAW;
			}
		}
		else if (attr == ATTR_ELF_OFF_LNK_SEC_VIA_RVA)
		{
			ADDR rvaAt(mr.cp());
			const MyShdr *psh(this->GetSectionHeaderFromRVA(rvaAt));
			if (psh)
			{
				const MyShdr *psh2(this->GetSectionHeader(_X(psh->sh_link)));
				if (psh2)
				{
					addr += this->template SafeCast<ADDR>(_X(psh2->sh_offset));
					return I_Front::AKIND_RAW;
				}
			}
		}
		else if (attr == ATTR_ELF_OFF_LNK_SEC_VIA_FP)
		{
			OFF_t offAt(mr.cpr());
			const MyShdr *psh(this->GetSectionHeaderFromFP((MyOff)offAt));
			if (psh)
			{
				const MyShdr *psh2(this->GetSectionHeader(_X(psh->sh_link)));
				if (psh2)
				{
					addr += this->template SafeCast<ADDR>(_X(psh2->sh_offset));
					return I_Front::AKIND_RAW;
				}
			}
		}
		else if (attr == ATTR_ELF_STRING_INDEX_DYNSYM)
		{
			ADDR rvaAt(mr.cp());
			const MyShdr *psh(this->GetSectionHeaderFromRVA(rvaAt));
			if (psh)
			{
				STOP
			}
			MyOff sh_offset;
			if (this->GetSectionLinkFP(SHT_DYNSYM, sh_offset))
			{
				addr += this->template SafeCast<ADDR>(sh_offset);
				return I_Front::AKIND_RAW;
			}
		}
		else if (attr == ATTR_ELF_DYN_TAG_PTR)
		{
			ADDR rvaAt(mr.cp());
			const MyDyn *pdyn(this->GetDynamicEntryFromVA(this->RVA2VA(rvaAt)));
			if (pdyn)
			{
				if (pdyn->d_tag == DT_NEEDED)
				{
					MyOff sh_offset;
					if (this->GetSectionLinkFP(SHT_DYNAMIC, sh_offset))
					{
						addr += this->template SafeCast<ADDR>(sh_offset);
						return I_Front::AKIND_RAW;
					}
				}
				else
				{
					return I_Front::AKIND_VA;
				}
			}
		}
		else if (attr == ATTR_ELF_DYNSYM_INDEX)
		{
			const MyShdr* psh(this->FindSectionHeader(SHT_DYNSYM));
			if (psh)
			{
				addr = ADDR(_X(psh->sh_offset) + sizeof(MySym) * addr);
				return I_Front::AKIND_RAW;
			}
		}
		else if (attr == ATTR_ELF_SECTION_HEADER)
		{
			unsigned fp;
			if (this->GetSectionHeaderFP(fp, addr))
			{
				addr = fp;
				return I_Front::AKIND_RAW;
			}
		}
		else if (attr2 == ATTR_DWARF_ABBREV_OFFS)
		{
			const MyShdr *psh(this->FindSectionHeader(".debug_abbrev"));
			if (psh)
			{
				addr += (ADDR)_X(psh->sh_offset);
				return I_Front::AKIND_RAW;
			}
		}
		else if (attr2 == ATTR_DWARF_INFO_OFFS)
		{
			const MyShdr *psh(this->FindSectionHeader(".debug_info"));
			if (psh)
			{
				addr += (ADDR)_X(psh->sh_offset);
				return I_Front::AKIND_RAW;
			}
		}
		else if (attr2 == ATTR_DWARF_ABBREV_CODE)
		{
			AuxDataBase_t meta(mr.aux());
			dwarf::die2abbrev_t *pmap(meta.data<dwarf::die2abbrev_t>("DIE2ABBREV"));
			if (pmap)//not available during formatting
			{
				POSITION oBeg((POSITION)mr.cpr());
				//elf::die2abbrev_t::elt_t *e = std::lower_bound(pmap->a, pmap->a + pmap->n, oBeg);
				//if (e != &pmap->a[pmap->n] && e->die == oBeg)
				const dwarf::die2abbrev_t::elt_t *e = pmap->find(oBeg);
				if (e)
				{
					addr = e->abbrev;
					return I_Front::AKIND_RAW;
				}
			}
			/*typedef typename dwarf::DebugInfo<T>	dwarf::DebugInfo<T>;
			MyDebugInfo di(*this);
			for (typename MyDebugInfo::CU_Iterator i(di); i; ++i)
			{
			}*/
		}
		else if (attr2 == ATTR_DWARF_STR_OFFS)
		{
			const MyShdr *psh(this->FindSectionHeader(".debug_str"));
			if (psh)
			{
				addr += (ADDR)_X(psh->sh_offset);
				return I_Front::AKIND_RAW;
			}
		}
		else if (attr2 == ATTR_DWARF_LINE_OFFS)
		{
			const MyShdr *psh(this->FindSectionHeader(".debug_line"));
			if (psh)
			{
				addr += (ADDR)_X(psh->sh_offset);
				return I_Front::AKIND_RAW;
			}
		}
		return I_Front::AKIND_NULL;
	}
};

//////////////////////////////////////////////////////////////////

I_Front::SymbolKind __demangleName_gcc(const char *mangled, MyStreamBase &ss)
{
	char *buf(iberty_demangle(mangled));
	if (!buf)
		return I_Front::SYMK_NULL;

	MyStreamUtil ssu(ss);
	static const MyString z1("vtable for ");
	static const MyString z2("VTT for ");
	static const MyString z3("non-virtual thunk to ");
	static const MyString z4("virtual thunk to ");
	static const MyString z5("typeinfo for ");
	static const MyString z6("typeinfo name for ");
	static const MyString z7("guard variable for ");
	static const MyString z8("construction vtable for ");
	static const MyString z9("-in-");
	MyString s(buf);
	free_demangled(buf);
	I_Front::SymbolKind eKind(I_Front::SYMK_OK);
	if (s.startsWith(z1))
	{
		eKind = I_Front::SYMK_VFTABLE;// _ADDRP;
		s.remove(0, (unsigned)z1.length());
		s.append("::`vtable'");
	}
	else if (s.startsWith(z2))
	{
		s.remove(0, (unsigned)z2.length());
		s.append("::`VTT'");
	}
	else if (s.startsWith(z3) || s.startsWith(z4))
	{
		s.prepend("`");
		s.append("\'");
		//s.remove(0, (unsigned)c.length());
		//s.append("::`non-virtual thunk'");
	}
	else if (s.startsWith(z5))
	{
		eKind = I_Front::SYMK_TYPEINFO;// _ADDRP;
		s.remove(0, (unsigned)z5.length());
		s.append("::`type info'");
	}
	else if (s.startsWith(z6))
	{
		s.remove(0, (unsigned)z6.length());
		s.append("::`type info name'");
	}
	else if (s.startsWith(z7))
	{
		s.remove(0, (unsigned)z7.length());
		s.append("::`guard variable'");
	}
	else if (s.startsWith(z8))
	{
		eKind = I_Front::SYMK_CVFTABLE;
		s.remove(0, (unsigned)z8.length());
		s.append("::`construction vftable'");
		size_t n(s.find(z9));
		if (n != MyString::npos)
		{
			MyString s2(s.substr(0, n));
			s.erase(0, n + z9.length());
			s.append("{for `" + s2 + "\'}");
		}
	}
	ssu.WriteString(s);
	return eKind;
}

//////////////////////////////////////////////////////////////////
#include "front_IA.h"

struct ELF32_t : public Elf32_t
{
	typedef IA32FrontDC_t	FRONTDC;
};

struct ELF64_t : public Elf64_t
{
	typedef IA64FrontDC_t	FRONTDC;
};

////////////////////////////////////////////////////////////////// ELF32Front_t

template <typename T>
class ELF_Front_t : public I_Front
{
	typedef ELF_Formatter_t<ELF2_t<T>> CELF_t;
	const I_DataSourceBase *aRaw;
	CELF_t *mpe;
	typedef typename T::FRONTDC	FRONTDC;
public:
	ELF_Front_t(const I_DataSourceBase *p)
		: aRaw(p),
		mpe(p ? new CELF_t(*p) : nullptr)
	{
	}
	virtual ~ELF_Front_t()
	{
		delete mpe;
	}

	virtual void release() override { delete this; }

	virtual I_FrontDC *createFrontDC() const override {
		return new FRONTDC;
	}

	virtual I_DebugInfoDumper *createDebugInfoDumper(const char* hint) override {
		if (mpe)
			return mpe->createDebugInfoDumper(hint);
		return nullptr;
	}

	virtual void dumpResources2(PFDumpResCallback, void *pvUser, const char *) override {}
	virtual void dumpResources(MyStreamBase &ss, const char *rootStr) override {
		//if (mpe) mpe->dumpResources(ss, rootStr); 
	}
	virtual void dumpImports(I_DumpSymbol* pf, Dump_IE_Flags bNamedOnly) override {
		if (mpe) mpe->dumpImports(pf, bNamedOnly);
	}
	virtual void dumpExports(I_DumpSymbol* pf, Dump_IE_Flags bNamedOnly) override {
		if (mpe) mpe->dumpExports(pf, bNamedOnly);
	}
	virtual void dumpDebugInfo(I_ModuleCB::Dump_D_Flags, I_ModuleCB &) override {
	}
	virtual AKindEnum translateAddress(const I_DataStreamBase &r, int moduleId, ADDR &addr, AttrIdEnum attr) override {
		return mpe ? mpe->translateAddress(r, moduleId, addr, attr) : AKIND_NULL;
	}
	virtual ADDR getImportPtrByName(const char *name, OFF_t *) const override {
		//return mpe ? mpe->getImportPtrByName(name) : 0;
		return 0;
	}
	virtual bool getImportInfo(ADDR va, I_DumpSymbol*) const override {
		//return mpe ? getImportInfo(va, out) : false;
		return false;
	}
	virtual bool getExportInfo(const char *, unsigned, I_DumpSymbol*) const override { return false; }
	virtual SymbolKind demangleName(const char *mangled, MyStreamBase &ss) override {
		return __demangleName_gcc(mangled, ss);
	}
	virtual bool dumpClassHierachy(IDumpClassHierarchy *pcb, ADDR, I_Front::RTTI_Method, ADDR) override {
		return false;
	}
	virtual bool getEntryVA(ADDR &va) override {
		if (!mpe)
			return false;
		va = mpe->VA2RVA(mpe->_X(mpe->ProgramHeader().e_entry));
		return true;
	}
	virtual bool processSpecial(I_Module &, I_Front::SymbolKind, const char*) override
	{
		return false;
	}
};


I_Front *CreateELF32Front(const I_DataSourceBase *aRaw)
{
	return new ELF_Front_t<ELF32_t>(aRaw);
}

I_Front *CreateELF64Front(const I_DataSourceBase *aRaw)
{
	return new ELF_Front_t<ELF64_t>(aRaw);
}

///////////////////////////////////////////////////////// ELF_DynamicType

class ELF_DynamicType : public I_FormatterType
{
	bool mb64bit;
public:
	ELF_DynamicType(bool b64bit) : mb64bit(b64bit){}
	virtual const char *name() const { return mb64bit ? "PE64" : "PE32"; }
	virtual void createz(I_SuperModule &r, unsigned long nSize)
	{
		I_SuperModule &mr(r);
		//PDATA pData((PDATA)mr.data());
		if (!mb64bit)
		{
			ELF_Formatter_t<ELF2_t<ELF32_t>> ELF(mr);
			if (!CheckElfSignature(mr, nSize))
				return;
			ELF.preformat(mr, nSize);
		}
		else
		{
			ELF_Formatter_t<ELF2_t<ELF64_t>> ELF(mr);
			if (!CheckElfSignature(mr, nSize))
				return;
			ELF.preformat(mr, nSize);
		}
	}
};

DECLARE_FORMATTER1(ELF_DynamicType, ELF32, false);
DECLARE_FORMATTER1(ELF_DynamicType, ELF64, true);

void ELF_RegisterFormatters(I_ModuleEx &rMain)
{
	rMain.RegisterFormatterType(_PFX("ELF32"));//decoder.elf.cpp
	rMain.RegisterFormatterType(_PFX("ELF64"));//decoder.elf.cpp
}

