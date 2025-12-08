#include "decode.pe.h"

#include <assert.h>
#include <iostream>
#include <fstream>

#include "qx/MyPath.h"
#include "qx/MyStream.h"
#include "qx/MyString.h"
#include "qx/MyStringList.h"
#include "qx/QxTime.h"

#include "shared.h"
#include "shared/front.h"
#include "shared/data_source.h"
#include "front_IA.h"
#include "decode.PDB.h"
#include "decode.COFF.h"
#include "decode.DWARF.h"
#include "decode.RTTI.h"
#include "PE.h"


using namespace adcwin;

///////////////////////////////////////////////MyPE
template <typename T_PE>
class PE_Decoder_t : public T_PE
{
public:
	PE_Decoder_t(const I_DataSourceBase& aRaw)
		: T_PE(aRaw)

	{
	}
	~PE_Decoder_t()
	{
	}

	class I_FrontCB
	{
	public:
		virtual MyString fromOrdinal(unsigned, const char*) const = 0;
	};

		/////////////////////////////////////// MyMapDumper
	//template <typename T_PE>
	class MyMapDumper : public I_DebugInfoDumper
	{
		struct SymbolInfo_t
		{
			ADDR va;
			MyString	sSymbolName;
			unsigned short uOrdinal;
		};
		typedef std::vector<SymbolInfo_t>	SymbolMap;

		const PE_Decoder_t& mrpe;
		std::string mPath;
		SymbolMap symb;
		typename SymbolMap::iterator symbIt;

	public:
		MyMapDumper(PE_Decoder_t& r, const char* path)
			: mrpe(r),
			mPath(path ? path : ""),
			symbIt(symb.begin())
		{
			assert(symb.empty());
			std::string myPath(mrpe.dataSource().modulePath());
			MyPath f(myPath);
			f.SetExt("map");
			try
			{
				dumpMapInfo(f.Path().c_str(), I_ModuleCB::DUMP_D_FUNCTIONS_ONLY);
			}
			catch (...)
			{
				//MAIN.printError() << "unknown" << std::endl;
			}
		}
		~MyMapDumper()
		{
		}
	protected:
		virtual bool processNext(I_ModuleCB& rICb, unsigned& progress)//global symbols
		{
			if (symbIt == symb.end())
				return false;
			rICb.selectFile("from_map");
			size_t iCur(std::distance(symb.begin(), symbIt));
			rICb.resetProgress("Map Symbols", TProgress(iCur, symb.size()));
			const SymbolInfo_t& a(*symbIt);
			rICb.dump(a.va, a.sSymbolName.c_str(), 0);
			++symbIt;
			return true;
		}
	private:
		bool dumpMapInfo(const char* path, I_ModuleCB::Dump_D_Flags)
		{
			std::ifstream is(path);
			if (!is.is_open())
				return false;

			MyPath f(path);

			MyString s;
			get_next_line(is, s);
#ifdef WIN32
			s = s.lower();
#endif
			if (s.stripWhiteSpace() != f.Basename())
				return false;

			get_next_line(is, s);
			if (!s.stripWhiteSpace().startsWith("Timestamp"))
				return false;

			get_next_line(is, s);
			if (!s.stripWhiteSpace().startsWith("Preferred load address"))
				return false;

			MyStringList l(MyStringList::split(" ", s));
			value_t vBase;
			StrHex2Int(l.back(), vBase);

			get_next_line(is, s);
			if (!check_line(s, "Start Length Name Class"))
				return false;

			for (;;)//skip sections
			{
				std::getline(is, s);
				if (s.isEmpty())
					break;
				l = MyStringList::split(" ", s);
			}

			get_next_line(is, s);
			if (!check_line(s, "Address Publics by Value Rva+Base Lib:Object"))
				return false;

			//symbols
			get_next_line(is, s);//skip empty lines (take first entry)
			do {
				l = MyStringList::split(" ", s);
				l.pop_front();//Address
				MyString sName(l.front());//Publics by Value
				l.pop_front();
				value_t v;
				StrHex2Int(l.front(), v);
				l.pop_front();
				if (v.ui64 != 0)
				{
					ADDR va(mrpe.is64bit() ? ADDR(v.ui64 - vBase.ui64) : v.ui32);
					const char* pcName(sName.c_str());
					MangleSchemeEnum eMangle(__checkMangleScheme(&pcName));
					(void)eMangle;
					if (pcName)
					{
						SymbolInfo_t a;
						a.va = va;
						a.sSymbolName = pcName;
						symb.push_back(a);
					}
				}

				std::getline(is, s);
			} while (!s.isEmpty());

			symbIt = symb.begin();
			return true;
		}
	};


	/////////////////////////////////////// MyPdbDumper
	//template <typename T_PE>
	class MyPdbDumper : public I_DebugInfoDumper
	{
		const PE_Decoder_t& mrpe;
		PDB::CSymbolProcessor<T_PE>* mpPdbProcessor;
		std::string mPath;
	public:
		MyPdbDumper(PE_Decoder_t& r, const char* path, unsigned flags = 0)//flags: depend on dumper impl
			: mrpe(r),
			mpPdbProcessor(nullptr),
			mPath(path ? path : "")
		{
			mpPdbProcessor = new PDB::CSymbolProcessor<T_PE>(r);
			int ret(mpPdbProcessor->createA(mPath));
			if (ret == 0)
			{
				delete mpPdbProcessor;
				mpPdbProcessor = nullptr;
			}
			else if (ret > 0)
			{
				bool bNoTypeInfo((flags & 1) != 0);
				bool bNoPubSyms((flags & 2) != 0);
				mpPdbProcessor->createProcessors(bNoTypeInfo, bNoPubSyms);
			}
		}
		~MyPdbDumper()
		{
			delete mpPdbProcessor;
		}
	protected:
		//virtual bool processNextType(I_ModuleCB &, unsigned &progress){ return false; }
		virtual bool processNext(I_ModuleCB& rICb, unsigned& progress)//global symbols
		{
			if (mpPdbProcessor)
			{
				if (mpPdbProcessor->processNext(rICb, progress))
					return true;
				delete mpPdbProcessor;
				mpPdbProcessor = nullptr;
			}
			return false;
		}
	};

	/////////////////////////////////////// MyCoffDumper
	//template <typename T_PE>
	class MyCoffDumper : public I_DebugInfoDumper
	{
		const PE_Decoder_t& mrpe;
		COFFSymbolProcessor<T_PE>* mpCoffSymbProcessor;
	public:
		MyCoffDumper(PE_Decoder_t& r)
			: mrpe(r),
			mpCoffSymbProcessor(nullptr)
		{
			DWORD fp0(mrpe.ImageFileHeader().PointerToSymbolTable);
			if (fp0 != 0)
			{
				fprintf(stdout, "COFF symbols table present at offset %08X\n", fp0);
				try {
					mpCoffSymbProcessor = new COFFSymbolProcessor<T_PE>(mrpe);
				}
				catch (int)//data access fault?
				{
					STOP
				}
			}
		}
		~MyCoffDumper()
		{
			delete mpCoffSymbProcessor;
		}
	protected:
		//virtual bool processNextType(I_ModuleCB &, unsigned &progress){ return false; }
		virtual bool processNext(I_ModuleCB& rICb, unsigned& progress)//global symbols
		{
			if (mpCoffSymbProcessor)
			{
				if (mpCoffSymbProcessor->processNext(rICb, progress))
					return true;
				delete mpCoffSymbProcessor;
				mpCoffSymbProcessor = nullptr;
			}
			return false;
		}
	};

	/////////////////////////////////////// MyDwarfDumper0
	//template <typename T_PE>
	class MyDwarfDumper0 : public I_DebugInfoDumper
	{
		const PE_Decoder_t& mrpe;
	public:
		MyDwarfDumper0(PE_Decoder_t& r)
			: mrpe(r)
		{
		}
		~MyDwarfDumper0()
		{
		}
	protected:
		//virtual bool processNextType(I_ModuleCB &, unsigned &progress){ return false; }
		virtual bool processNext(I_ModuleCB& rICb, unsigned& progress)//global symbols
		{

			return false;
		}
	};

	/////////////////////////////////////// MyOldPdbDumper
	//template <typename T_PE>
	class MyOldPdbDumper : public I_DebugInfoDumper
	{
		const PE_Decoder_t& mrpe;
		PDB::CSymbolProcessorOld<T_PE>* mpPdbProcessor;
	public:
		MyOldPdbDumper(PE_Decoder_t& r)
			: mrpe(r),
			mpPdbProcessor(nullptr)
		{
			mpPdbProcessor = new PDB::CSymbolProcessorOld<T_PE>(r);
			int ret(mpPdbProcessor->createA(""));
			if (ret == 0)
			{
				delete mpPdbProcessor;
				mpPdbProcessor = nullptr;
			}
			else
				mpPdbProcessor->initPdb0();
		}
		~MyOldPdbDumper()
		{
			delete mpPdbProcessor;
		}
	protected:
		/*virtual bool processNextType(I_ModuleCB& rICb, unsigned& progress)
		{
			if (!mpPdbProcessor)
				return false;
			return mpPdbProcessor->processNextType(rICb, progress);
		}*/

		virtual bool processNext(I_ModuleCB& rICb, unsigned& progress)//global symbols
		{
			if (mpPdbProcessor)
			{
				if (mpPdbProcessor->processNext(rICb))//, progress))
					return true;
				delete mpPdbProcessor;
				mpPdbProcessor = nullptr;
			}
#if(0)
			else
				test(rICb);
#endif
			return false;
		}
	private:////////////////////////////////////////////////
		void test(I_Module& mr)
		{
			mr.selectFile("test.h");
			if (mr.NewScope("ThisIsATest"))
			{
				mr.declField("a", mr.arrayOf(mr.arrayOf(mr.type(TYPEID_INT), 3), 2));//int a[2][3]
				mr.declField("b", mr.ptrOf(mr.arrayOf(mr.ptrOf(mr.arrayOf(mr.type(TYPEID_INT), 3)), 2)));	//int (*(*b)[2])[3]
				mr.declField("c", mr.ptrOf(mr.arrayOf(mr.arrayOf(mr.type(TYPEID_INT), 3), 2)));//int (*a)[2][3]
				mr.declField("d", mr.arrayOf(mr.arrayOf(mr.ptrOf(mr.type(TYPEID_INT)), 3), 2));//int *a[2][3]
				mr.Leave();
			}
		}
	};

	I_DebugInfoDumper *createDebugInfoDumper(const char* hint)
	{
		int flags(0);
		if (hint)//supported modes
		{
			if (strcmp(hint, "pdb0") == 0)
				return new MyOldPdbDumper(*this);//OLD
			if (strcmp(hint, "map") == 0)
				return new MyMapDumper(*this, nullptr);
			if (strncmp(hint, "map", 2) == 0 && hint[3] == '@')
				return new MyMapDumper(*this, hint + 4);
			if (strcmp(hint, "dwarf") == 0)
				return new MyDwarfDumper<T_PE>(*this);//<T_PE>(*this);
			if (strcmp(hint, "coff") == 0)
				return new MyCoffDumper(*this);
			if (strncmp(hint, "pdb", 3) == 0)
			{
				//format: pdb#<flags>@<path>
				hint += 3;
				if (*hint)
				{
					if (*hint == '#')
						flags = atoi(++hint);
					hint = strchr(hint, '@');
					if (hint)//a path
						return new MyPdbDumper(*this, ++hint, flags);
				}
			}
		}
		return new MyPdbDumper(*this, nullptr, flags);
	}

	static bool check_line(const MyString &s, const char *ref)
	{
		MyStringList l(MyStringList::split(" ", s));
		MyStringList l2(MyStringList::split(" ", ref));
		return l == l2;
	}

	static void get_next_line(std::istream &is, MyString &s)//skip empty
	{
		do { std::getline(is, s); } while (s.isEmpty());
	}

	using typename T_PE::ImportModuleIterator;
	using typename T_PE::ImportEntryIterator;

	void dumpImports(I_Front::I_DumpSymbol* pf, I_Front::Dump_IE_Flags dumpFlags, const I_FrontCB* pcb)
	{
		if (T_PE::CheckDataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT) != 1)
			return;

		if (dumpFlags == I_Front::DUMP_IE_MODULES)
		{
			for (ImportModuleIterator i(*this); i; i++)
			{
				std::ostringstream ss;
				i.moduleName(ss);
				pf->setName(ss.str().c_str());
				pf->flush();
			}
			return;
		}
		for (ImportEntryIterator i(*this); i; i++)
		{
			std::ostringstream ss;
			if (i.functionName(ss) == 0 && (dumpFlags == I_Front::DUMP_IE_NAMED_ONLY))
				continue;
			if (ss.str().empty())
			{
				std::ostringstream ss2;
				i.moduleName(ss2);
				ss << pcb->fromOrdinal(i.ordinal(), ss2.str().c_str());
			}
			DWORD va(RVA2VA(i.functionRVA()));
			pf->setVA(va);
			ss << '\n' << i.ordinal() << '\n';
			i.moduleName(ss);
			//pf->setModule(i.moduleName());
			pf->setName(ss.str().c_str());
			pf->flush();
		}
	}

	using T_PE::RVA2VA;
	using T_PE::dataSource;
	using typename T_PE::ExportEntryIterator;

	void dumpExports(I_Front::I_DumpSymbol* pf, I_Front::Dump_IE_Flags bNamedOnly, const I_FrontCB* pcb)
	{
		//if (ImageDataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT).Size == 0)//is there an export table?
		if (T_PE::CheckDataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT) != 1)
			return;

		AuxData_t<PE__META_TOTAL> meta(dataSource().aux());
		PWORD pmap(meta.data<WORD>(PE_META_EXPORT));

		for (ExportEntryIterator i(*this); i; ++i)
		{
			DWORD rva(i.funcRVA());//some export entries may be 0
			if (rva != 0)
			{
				std::ostringstream ss;
				pf->setVA(RVA2VA(rva));
				i.symbolName(pmap, ss);
				if (ss.str().empty())
				{
					MyPath aPath(dataSource().modulePath());
					ss << pcb->fromOrdinal(i.ordinal(), aPath.Name());
				}
				ss << '\n' << i.ordinal() << '\n';
CHECK(i.moduleName() == 0xcdd5c)
STOP
				i.moduleName(ss);
				pf->setName(ss.str().c_str());
				pf->flush();
			}
		}
	}

	using typename T_PE::ResourceIterator;

	void dumpResources(MyStreamBase &ss, const char *rootStr)
	{
		MyStreamUtil ssu(ss);

		std::string s;
		if (rootStr)
			s = rootStr;
		if (s.empty())
			s = "Resources";

		for (ResourceIterator i(*this); i; i++)
		{
			if (i.isDirectory())
			{
				ssu.WriteString(s);
				s.clear();
				for (size_t j(0); j < i.depth(); j++)
					s.append("\t");
				s.append(i.namez());
				s.append("/");
			}
			else
			{
				s.resize(s.length() - 1);//remove '/'
				s.append(MyStringf("\t%08X", RVA2VA(i.dataRVA())));
			}

			//std::cout << s << std::endl;
		}
		ssu.WriteString(s);
	}

	void dumpResources2(I_Front::PFDumpResCallback pf, void *pvUser, const char *rootStr)
	{
		std::string s;
		if (rootStr)
			s = rootStr;
		if (s.empty())
			s = "Resources";

		size_t level(0);
		DataStream_t ds(dataSource(), OFF_NULL);
		IMAGE_RESOURCE_DATA_ENTRY irde;
		for (ResourceIterator i(*this); i; i++)
		{
			if (i.isDirectory())
			{
				ADDR va(0);
				DWORD sz(0);
				if (!ds.isAtEnd())
				{
					va = RVA2VA(irde.OffsetToData);
					sz = irde.Size;
				}
				if (!(*pf)(level, s.c_str(), va, sz, pvUser))
					return;
				s = i.namez();
				level = i.depth();
				ds.seek(OFF_NULL);
			}
			else
			{
				//level++;
				ds.seek(RvaToFO(i.dataEntryRVA()));
				ds.read(irde);
			}

			//std::cout << s << std::endl;
		}
		ADDR va(0);
		DWORD sz(0);
		if (!ds.isAtEnd())
		{
			va = RVA2VA(irde.OffsetToData);
			sz = irde.Size;
		}
		(*pf)(level, s.c_str(), va, sz, pvUser);
	}

	using typename T_PE::XWORD;
	using T_PE::is64bit;
	using T_PE::ImageDataDirectory;
	using T_PE::RvaToFO;
	using T_PE::__COR_MetaDataInfo;
	using T_PE::StringTableEntryAtOffset2;
	using T_PE::SymbolTableEntryAtFP2;
	using T_PE::CheckDataDirectory;
	using T_PE::ImageFileHeader;
	using T_PE::ImageSectionHeader;

	I_Front::AKindEnum translateAddress(const I_DataStreamBase &mr, int moduleId, ADDR &addr, AttrIdEnum attr)
	{
		ADDR vaAt(mr.cp());
		//const IMAGE_OPTIONAL_HEADER &ioh(GetOptionalHeader());
		switch (attr)
		{
		case ATTR_FILEPTR:
			return I_Front::AKIND_RAW;//this is a raw address
		case ATTR_OFFS:
			return I_Front::AKIND_VA;
		case ATTR_RVA_OR_ORDINAL:
			if (addr & IMAGE_ORDINAL_FLAG32)
				return I_Front::AKIND_NULL;
		case ATTR_RVA:
		{
			addr &= ~IMAGE_ORDINAL_FLAG32;//the msb can be used for something else, never a part of RVA
			XWORD x(addr);
			if (!is64bit())//?
				x += (XWORD)T_PE::ImageOptionalHeader().ImageBase;
			addr = (ADDR)x;
			return I_Front::AKIND_VA;
		}
		case ATTR_RES_OFFS://offset from beggining of resurce directory root (may have a high bit set, denoting a directory)
		{
			const IMAGE_DATA_DIRECTORY &idd2(ImageDataDirectory(IMAGE_DIRECTORY_ENTRY_RESOURCE));
			if (idd2.VirtualAddress != 0)
			{
				addr &= ~0x80000000;//the msb can be used for something else, never a part of RVA
				addr = RVA2VA(idd2.VirtualAddress) + addr;
				return I_Front::AKIND_VA;
			}
			/*			DWORD rvaAt(DWORD(vaAt - imageBase()));
						const IMAGE_SECTION_HEADER *psec(RvaToSection(rvaAt));//find a section
						if (psec)
						{
						addr &= ~0x80000000;//the msb can be used for something else, never a part of RVA
						addr = RVA2VA(psec->VirtualAddress) + addr;
						return I_Front::AKIND_VA;
						}*/
			break;
		}
		case ATTR_RELOC:
		{
			//find relocatin block
			const IMAGE_DATA_DIRECTORY &idd5(ImageDataDirectory(IMAGE_DIRECTORY_ENTRY_BASERELOC));
			if (idd5.Size)
			{
				DWORD rva(idd5.VirtualAddress);
				DWORD va(RVA2VA(rva));
				DWORD va2(va + idd5.Size);
				while (va < va2)
				{
					DataFetch2_t<FixupBlock> aBlock(dataSource(), RvaToFO(rva));
					if (va <= vaAt && vaAt < va + aBlock.BlockSize)
					{
						addr = RVA2VA(aBlock.PageRVA + (addr & 0xFFF));
						return I_Front::AKIND_VA;
					}
					rva += aBlock.BlockSize;
					va += aBlock.BlockSize;
				}
			}
			break;
		}
		case ATTR_NET_METADATA_OFFS:
		{
			const IMAGE_DATA_DIRECTORY &idd14(ImageDataDirectory(IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR));
			if (idd14.Size)
			{
				OFF_t o(RvaToFO(idd14.VirtualAddress));
				DataFetch2_t<IMAGE_COR20_HEADER> a(dataSource(), o);
				DWORD va(RVA2VA(a.MetaData.VirtualAddress));
				addr += va;
				return I_Front::AKIND_VA;
			}
			break;
		}
		case ATTR_NET_STRING_INDEX:
		{
			addr += RVA2VA( __COR_MetaDataInfo().rvaStrings);
			return I_Front::AKIND_VA;
		}
		case ATTR_COFF_STRING_TABLE_REF:
		{
			addr = (ADDR)StringTableEntryAtOffset2(addr);
			//addr = ADDR(StringTableEntryAtOffset(addr) - (PDATA)GetData(0));
			return I_Front::AKIND_RAW;
		}
		case ATTR_COFF_SYMBOL_VALUE_REF:
		{
			OFF_t o(SymbolTableEntryAtFP2(vaAt));//vaAt?
			if (o != OFF_NULL)
			{
				DataFetchPtr2_t<IMAGE_SYMBOL_ENTRY> aSymb(dataSource(), o);
				if (aSymb.SectionNumber > 0)
				{
					if (aSymb.SectionNumber - 1 < ImageFileHeader().NumberOfSections)
					{
						const IMAGE_SECTION_HEADER &ish(ImageSectionHeader(aSymb.SectionNumber - 1));
						addr = ish.VirtualAddress + aSymb.Value;
						return I_Front::AKIND_VA;
					}
				}
			}
			break;
		}
		case ATTR_DWARF_ABBREV_CODE:
		{
			AuxDataBase_t meta(mr.aux());
			dwarf::die2abbrev_t* pmap(meta.data<dwarf::die2abbrev_t>("DIE2ABBREV"));
			if (pmap)//not available during formatting
			{
				POSITION oBeg((POSITION)mr.cpr());
				const dwarf::die2abbrev_t::elt_t* e = pmap->find(oBeg);
				if (e)
				{
					addr = e->abbrev;
					return I_Front::AKIND_RAW;
				}
			}
			break;
		}
		default:
			break;
		}//switch
		return I_Front::AKIND_NULL;
	}

	ADDR getImportPtrByName(const char *name, OFF_t *ppModuleName) const
	{
		if (CheckDataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT) == 1)
		{
			for (ImportEntryIterator i(*this); i; i++)
			{
				std::ostringstream ss;
				if (i.functionName(ss) == 0)
					continue;
				if (ss.str() == name)
				{
					if (ppModuleName)
						*ppModuleName = i.moduleName();
					return RVA2VA(i.functionRVA());
				}
			}
		}
		return 0;
	}

	bool getImportInfo(ADDR va, I_Front::I_DumpSymbol* pf) const
	{
		if (CheckDataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT) == 1)
		{
			for (ImportEntryIterator i(*this); i; i++)
			{
				//OFF_t oFunc(i.functionName());
				if (RVA2VA(i.functionRVA()) == va)
				{
					pf->setVA(va);
					std::stringstream ss;
					i.functionName(ss);
					ss << '\n' << i.ordinal() << '\n';
					i.moduleName(ss);
					pf->setName(ss.str().c_str());
					//pf->setModule(i.moduleName());
					pf->flush();
					return true;
				}
			}
		}
		return false;
	}

	bool getExportInfo(const char *name, unsigned ordinal, I_Front::I_DumpSymbol* pf) const
	{
		ExportEntryIterator i(*this);
		i.lookup(name, ordinal);
		if (i)
		{
			AuxData_t<PE__META_TOTAL> meta(dataSource().aux());
			PWORD pmap(meta.data<WORD>(PE_META_EXPORT));
			pf->setVA(i.funcVA());
			std::ostringstream ss;
			i.symbolName(pmap, ss);
			ss << '\n' << i.ordinal() << '\n';
			pf->setName(ss.str().c_str());
			//pf->setModule(i.moduleName());
			pf->flush();
			return true;
		}
		return false;
	}

	bool dumpClassHierachyImpl(DWORD rva, I_Front::IDumpClassHierarchy *pI, I_Front::RTTI_Method eMethod, DWORD rvaVTable)
	{
		try {
			OFF_t fo(this->__RVA2FO(rva));
			//DWORD  va2(rvaVTable != 0 ? __RVA2FO(rvaVTable) : 0);
			switch (eMethod)
			{
				case I_Front::RTTI_MSVC:
				{
					RTTI_Decoder_MSVC_t<T_PE> aRtti(*this);
					if (is64bit())
						return aRtti.dumpClassHierachyMsvc64(fo, pI);
					return aRtti.dumpClassHierachyMsvc(fo, pI);
				}

				case I_Front::RTTI_GCC_SI:
				case I_Front::RTTI_GCC_VMI:
				{
					RTTI_Decoder_GCC_t<T_PE> aRtti(*this, rva, rvaVTable, pI);
					if (eMethod == I_Front::RTTI_GCC_SI)
					{
						if (is64bit())
							return aRtti.dump_SI64(rva);
						return aRtti.dump_SI(rva);
					}
					if (is64bit())
						return aRtti.dump_VMI64(rva);
					return aRtti.dump_VMI(rva);
				}

				default:
					break;
			}
		}
		catch (int){
		}
		return false;
	}

	static std::string _chk(const char* pc)
	{
		std::string s(pc ? pc : "");
		if (!s.empty())
		{
			if (s.rfind("const ", 0) == 0)//starts with
				s.erase(0, 6);
			if (s.substr(0, 2) != "::")
				s.insert(0, "::");
		}
		return s;
	}
	bool processSpecial(I_Module& mr, I_Front::SymbolKind eKind, const char* name)
	{
		switch (eKind)
		{
		case I_Front::SYMK_VFTABLE:
break;
			if (!mr.NewScope(mr.declField(_chk(name).c_str()), SCOPE_VFTABLE))//?
				break;
			mr.Leave();
			return true;

		case I_Front::SYMK_TYPEINFO:
			return mr.declField(_chk(name).c_str(), mr.type("__class_type_info"));
		case I_Front::SYMK_RTTI_TD:
			return mr.declField(_chk(name).c_str(), mr.type(_PFX("_TypeDescriptor")));
		case I_Front::SYMK_RTTI_BCD:
			return mr.declField(_chk(name).c_str(), mr.type("__RTTIBaseClassDescriptor"));
		case I_Front::SYMK_RTTI_CHD:
			return mr.declField(_chk(name).c_str(), mr.type("__RTTIClassHierarchyDescriptor"));
		case I_Front::SYMK_RTTI_COL:
			return mr.declField(_chk(name).c_str(), mr.type("__RTTICompleteObjectLocator"));
		default:
			break;
		}
		return false;
	}

};






class TxtReader_t : private std::ifstream
{
	const MyPath	m_path;
	int m_line;
	const char m_chComment;
public:
	TxtReader_t(MyPath path, char comm = 0)
		: m_path(path),
		m_line(0),
		m_chComment(comm)
	{
		open(m_path.Path());
	}
	~TxtReader_t()
	{
		if (is_open())
			close();
	}
	bool isOpen() const { return is_open(); }
	bool getLine(MyString &s)
	{
		while (!eof())
		{
			++m_line;
			std::getline(*this, s);
			if (m_chComment != 0)
			{
				size_t n(s.find(m_chComment));
				if (n != MyString::npos)
					s.resize(n);//cut comment off
			}
			s = s.stripWhiteSpace();
			if (!s.isEmpty())
				return true;
		}
		return false;
	}
};

class ExportFileParser_t
{
	MyString m_library;
	MyString m_section;
	MyString m_symbol;
	unsigned m_ordinal;
public:
	ExportFileParser_t() : m_ordinal(-1){}
	bool parse(MyString &s)
	{
		assert(!s.isEmpty());
		MyStringList l(MyStringList::split(" ", s));
		if (m_library.isEmpty())
		{
			if (l.size() != 2 || l.front().upper() != "LIBRARY")
				return false;
			m_library = l.back();
		}
		else if (m_section.isEmpty())
		{
			if (l.size() != 1 || l.front().upper() != "EXPORTS")
				return false;
			m_section = s;
		}
		else
		{
			m_ordinal = -1;
			MyStringList l2(MyStringList::split("=", l.front()));
			m_symbol = l2.front();
			l.pop_front();
			if (!l.empty() && l.front() == "@")
			{
				l.pop_front();
				if (!l.empty())
				{
					m_ordinal = atoi(l.front().c_str());
CHECK(m_ordinal==6412)
STOP
				}
			}
		}
		return true;
	}
	MyString symbol() const { return m_symbol; }
	unsigned ordinal() const { return m_ordinal; }
};


typedef std::map<unsigned, MyString>	O2NMap;

class OrdinalMap : std::vector<O2NMap>
{
	std::map<MyString, size_t> mOrd;//module => (ord=>mangled)
public:
	OrdinalMap()
	{
	}
	size_t isLoaded(MyString key) const
	{
		std::map<MyString, size_t>::const_iterator i(mOrd.find(key));
		if (i == mOrd.end())
			return -1;
		return i->second;
	}
	size_t load(MyString key, MyString pathExport)
	{
		assert(isLoaded(key) == -1);
		resize(size() + 1);
		size_t index(size() - 1);
		mOrd.insert(std::make_pair(key, index));
		O2NMap& m(back());

		ExportFileParser_t aParser;
		TxtReader_t aReader(pathExport, ';');
		if (aReader.isOpen())
		{
			MyString s;
			while (aReader.getLine(s))
			{
				if (!aParser.parse(s))
					break;
				MyString mangled(aParser.symbol());
				if (!mangled.isEmpty())
					m.insert(std::make_pair(aParser.ordinal(), mangled));
			}
		}

		return index;
	}

	MyString find(unsigned ord, size_t index) const
	{
		assert(index < size());
		const O2NMap& m(at(index));
		O2NMap::const_iterator i(m.find(ord));
		if (i != m.end())
			return i->second;
		return "";
	}
};


/////////////////////////////////////////////////////// IA32Front_t

template <typename T>
class T_IAFront_t : public I_Front,
	public PE_Decoder_t<PE2_t<T>>::I_FrontCB
{
	typedef typename PE_Decoder_t<PE2_t<T>>::I_FrontCB I_FrontCB;
	typedef PE_Decoder_t<PE2_t<T>>	CPE_t;
	CPE_t *mpe;
	const I_Main* mpIMain;
	OrdinalMap	mOrd;
	typedef typename T::FRONTDC	FRONTDC;
public:
	T_IAFront_t(const I_DataSourceBase *aRaw, const I_Main* pIMain)
		: mpe(aRaw ? new CPE_t(*aRaw) : nullptr),
		mpIMain(pIMain)
	{
	}

	virtual ~T_IAFront_t()
	{
		delete mpe;
	}

	virtual void release()
	{
		delete this;
	}

	virtual I_FrontDC *createFrontDC() const {
		return new FRONTDC;
	}

	virtual SymbolKind demangleName(const char *mangled, MyStreamBase &ss){
		return __demangleName(mangled, ss);
	}
	virtual I_DebugInfoDumper *createDebugInfoDumper(const char* hint){
		if (mpe)
			return mpe->createDebugInfoDumper(hint);
		return nullptr;
	}
	virtual void dumpDebugInfo(I_ModuleCB::Dump_D_Flags uFlags, I_ModuleCB &rICb){
		//unsigned progress;
		//if (mpe) mpe->process(rICb, uFlags, progress);
	}
	virtual void dumpImports(I_DumpSymbol* pf, Dump_IE_Flags bNamedOnly){
		if (mpe) mpe->dumpImports(pf, bNamedOnly, this);
	}
	virtual void dumpExports(I_DumpSymbol* pf, Dump_IE_Flags bNamedOnly){
		if (mpe) mpe->dumpExports(pf, bNamedOnly, this);
	}
	virtual void dumpResources(MyStreamBase &ss, const char *rootStr){
		if (mpe) mpe->dumpResources(ss, rootStr);
	}
	virtual void dumpResources2(PFDumpResCallback pf, void *pvUser, const char *rootStr){
		if (mpe) mpe->dumpResources2(pf, pvUser, rootStr);
	}
	virtual I_Front::AKindEnum translateAddress(const I_DataStreamBase &r, int moduleId, ADDR &addr, AttrIdEnum attr){
		return mpe ? mpe->translateAddress(r, moduleId, addr, attr) : AKIND_NULL;
	}
	virtual ADDR getImportPtrByName(const char *name, OFF_t *ppModuleName) const {
		return mpe ? mpe->getImportPtrByName(name, ppModuleName) : 0;
	}
	virtual bool getImportInfo(ADDR va, I_DumpSymbol* pf) const {
		return mpe ? mpe->getImportInfo(va, pf) : false;
	}
	virtual bool getExportInfo(const char *name, unsigned ordinal, I_DumpSymbol* pf) const {
		return mpe ? mpe->getExportInfo(name, ordinal, pf) : false;
	}
	virtual bool dumpClassHierachy(IDumpClassHierarchy *pcb, ADDR vaTypeInfo, RTTI_Method eMethod, ADDR vaVTable){
		if (mpe)
		{
			DWORD rvaTypeInfo(vaTypeInfo);
			DWORD rvaVTable(vaVTable);
			if (!mpe->is64bit())
			{
				rvaTypeInfo = DWORD(vaTypeInfo - mpe->imageBase());
				if (rvaVTable)
					rvaVTable = DWORD(vaVTable - mpe->imageBase());
			}
			return mpe->dumpClassHierachyImpl(rvaTypeInfo, pcb, eMethod, rvaVTable);
		}
		return false;
	}
	virtual bool getEntryVA(ADDR &va){
		if (!mpe)
			return false;
		va = mpe->RVA2VA(mpe->GetEntryPointRVA());
		return true;
	}
	virtual bool processSpecial(I_Module &mr, I_Front::SymbolKind eKind, const char* name) override
	{
		return mpe->processSpecial(mr, eKind, name);
	}
protected:
	virtual MyString fromOrdinal(unsigned ord, const char* moduleName) const 
	{
		size_t index(mOrd.isLoaded(moduleName));
		if (index == -1)
		{
			const char* frontPath(mpIMain->frontPathFromName(FRONT_NAME_PFX));
			MyPath aPath(frontPath);
			MyString protoDir(aPath.Dir() + "proto");
			aPath.SetDir(protoDir);
			MyString s(moduleName);
			s += ".export";
			aPath.SetName(s);
			index = const_cast<OrdinalMap *>(&mOrd)->load(moduleName, aPath.Path());
		}
		return mOrd.find(ord, index);
	}
};


/////////////////////////////////////////////////////

I_Front *CreateIA32Front(const I_DataSourceBase *aRaw, const I_Main* pIMain)
{
	return new T_IAFront_t<PE32_types_t>(aRaw, pIMain);
}

I_Front *CreateIA64Front(const I_DataSourceBase *aRaw, const I_Main* pIMain)
{
	return new T_IAFront_t<PE64_types_t>(aRaw, pIMain);
}
