#pragma once

#include <assert.h>
#include <list>
#include <string>
#include <algorithm>
#include <exception>
#include "qx/ConvertUTF.h"
#include "wintypes.h"
#include "interface/IADCMain.h"

namespace ADCPE {

	using namespace adcwin;

	class FormatFault : public std::exception
	{
		int mId;
	public:
		FormatFault(int id) : mId(id){}
		int code() const { return mId; }
	protected:
		virtual const char* what() const throw () {
			return "PE Format Fault";
		}
	};

	struct IA32_types_t
	{
		typedef DWORD	XWORD;
		typedef IMAGE_OPTIONAL_HEADER32	IMAGE_OPTIONAL_HEADER;
		//T_FRONTDC
	};

	struct IA64_types_t
	{
		typedef QWORD	XWORD;
		typedef IMAGE_OPTIONAL_HEADER64	IMAGE_OPTIONAL_HEADER;
	};

	struct COR_StreamHeader
	{
		DWORD Offset,
		Size;
		char Name[1];
	};

	class COR_MetaDataInfo
	{
	public:
		DWORD	rvaStrings;
	public:
		COR_MetaDataInfo() : rvaStrings(0){}
	};

	struct PDB20_t : public CV_INFO_PDB20
	{
		std::string		path;
	};

	struct PDB70_t : public CV_INFO_PDB70
	{
		std::string		path;
	};

	///////////////////////////////////////////////PE_t
	template <typename T>
	class PE_t
	{
	protected:
		const I_DataSourceBase &mData;

	public:
		typedef typename T::XWORD XWORD;
		typedef typename T::IMAGE_OPTIONAL_HEADER	IMAGE_OPTIONAL_HEADER;

		//#ifdef TARGET_WIN64
		//typedef QWORD XWORD;
		//typedef IMAGE_OPTIONAL_HEADER64	IMAGE_OPTIONAL_HEADER;
		//#else 
		//typedef DWORD XWORD;
		//typedef IMAGE_OPTIONAL_HEADER32	IMAGE_OPTIONAL_HEADER;
		//#endif
		const I_DataSourceBase &dataSource() const { return mData; }

	protected:
		IMAGE_DOS_HEADER *m_pidh;
		IMAGE_FILE_HEADER *m_pifh;
		IMAGE_OPTIONAL_HEADER *m_pioh;
		IMAGE_DATA_DIRECTORY *m_pidd;//array
		IMAGE_SECTION_HEADER *m_pish;//array
		IMAGE_DEBUG_DIRECTORY *m_pDebugDir;
		COR_MetaDataInfo	*m_pCORmeta;
	public:

		PE_t(const I_DataSourceBase &aRaw)
			: mData(aRaw),
			m_pidh(nullptr),
			m_pifh(nullptr),
			m_pioh(nullptr),
			m_pidd(nullptr),
			m_pish(nullptr),
			m_pDebugDir(nullptr),
			m_pCORmeta(nullptr)
		{
		}

		~PE_t()
		{
			delete m_pidh;
			delete m_pifh;
			delete m_pioh;
			delete [] m_pidd;
			delete [] m_pish;
			delete m_pDebugDir;
			delete m_pCORmeta;
		}

		const IMAGE_DOS_HEADER &ImageDOSHeader() const
		{
			if (!m_pidh)
			{
				DataFetch2_t<IMAGE_DOS_HEADER> idh(mData, 0);
				const_cast<PE_t *>(this)->m_pidh = new IMAGE_DOS_HEADER;
				*m_pidh = idh;
			}
			return *m_pidh;
		}

		const IMAGE_FILE_HEADER &ImageFileHeader() const
		{
			if (!m_pifh)
			{
				size_t o_ifh(ImageDOSHeader().e_lfanew + 4);
				DataFetch2_t<IMAGE_FILE_HEADER> idh(mData, o_ifh);
				const_cast<PE_t *>(this)->m_pifh = new IMAGE_FILE_HEADER;
				*m_pifh = idh;
			}
			return *m_pifh;
		}

		const IMAGE_OPTIONAL_HEADER &ImageOptionalHeader() const
		{
			if (!m_pioh)
			{
				size_t o_ifh(ImageDOSHeader().e_lfanew + 4);
				size_t o_ioh(o_ifh + sizeof(IMAGE_FILE_HEADER));
				DataFetch2_t<IMAGE_OPTIONAL_HEADER> ioh(mData, o_ioh);
				const_cast<PE_t *>(this)->m_pioh = new IMAGE_OPTIONAL_HEADER;
				*m_pioh = ioh;
			}
			return *m_pioh;
		}

		DWORD NumberOfDirectoryEntries() const
		{
			return IMAGE_NUMBEROF_DIRECTORY_ENTRIES;// ImageOptionalHeader().NumberOfRvaAndSizes;
		}

		const IMAGE_DATA_DIRECTORY &ImageDataDirectory(int index) const
		{
			if (!m_pidd)
			{
				size_t o_ifh(ImageDOSHeader().e_lfanew + 4);
				size_t o_ioh(o_ifh + sizeof(IMAGE_FILE_HEADER));
				size_t o_idd(o_ioh + sizeof(IMAGE_OPTIONAL_HEADER));
				int n(NumberOfDirectoryEntries());
				const_cast<PE_t *>(this)->m_pidd = new IMAGE_DATA_DIRECTORY[n];
				DataStream_t idd(mData, o_idd);
				for (int i(0); i < n; i++)
					m_pidd[i] = idd.read<IMAGE_DATA_DIRECTORY>();
			}
			assert(DWORD(index) < NumberOfDirectoryEntries());
			return m_pidd[index];
		}

		const IMAGE_DATA_DIRECTORY* ImageDataDirectoryPtr(int index) const
		{
			if (DWORD(index) < NumberOfDirectoryEntries())
				return &ImageDataDirectory(index);
			return nullptr;
		}

		const IMAGE_SECTION_HEADER &ImageSectionHeader(int index) const
		{
			if (!m_pish)
			{
				size_t o_ifh(ImageDOSHeader().e_lfanew + 4);
				size_t o_ioh(o_ifh + sizeof(IMAGE_FILE_HEADER));
				size_t o_idd(o_ioh + sizeof(IMAGE_OPTIONAL_HEADER));
				size_t o_ish(o_idd + NumberOfDirectoryEntries() * sizeof(IMAGE_DATA_DIRECTORY));
				int n(ImageFileHeader().NumberOfSections);
				const_cast<PE_t *>(this)->m_pish = new IMAGE_SECTION_HEADER[n];
				DataStream_t ish(mData, o_ish);
				for (int i(0); i < n; i++)
					m_pish[i] = ish.read<IMAGE_SECTION_HEADER>();
			}
			assert(index < ImageFileHeader().NumberOfSections);
			return m_pish[index];
		}

		const IMAGE_DEBUG_DIRECTORY &ImageDebugDirectory() const
		{
			if (!m_pDebugDir)
			{
				const IMAGE_DATA_DIRECTORY &idd(ImageDataDirectory(IMAGE_DIRECTORY_ENTRY_DEBUG));
				OFF_t off(RvaToFO(idd.VirtualAddress));
				if (off == OFF_NULL)
					throw FormatFault(-8);
				const_cast<PE_t *>(this)->m_pDebugDir = new IMAGE_DEBUG_DIRECTORY;
				DataStream_t ds(mData, off);
				*m_pDebugDir = ds.read<IMAGE_DEBUG_DIRECTORY>();
			}
			return *m_pDebugDir;
		}

		bool CheckProgramDatabase20(PDB20_t &out, const std::string& bdbPath) const
		{
			const IMAGE_DATA_DIRECTORY* pidd(ImageDataDirectoryPtr(IMAGE_DIRECTORY_ENTRY_DEBUG));
			if (!pidd || pidd->Size == 0)
				return false;
			const IMAGE_DEBUG_DIRECTORY &aDir(ImageDebugDirectory());
			if (aDir.Type != IMAGE_DEBUG_TYPE_CODEVIEW)
				return false;
			OFF_t oHdr(aDir.PointerToRawData);
			DataFetch_t<DWORD> dwSig(mData, oHdr);
			if (strncmp((const char *)&dwSig, "NB10", 4) != 0)
				return false;
			DataFetch2_t<CV_INFO_PDB20> aHdr(mData, oHdr);
			DataStream_t szPdb(mData, oHdr + sizeof(CV_INFO_PDB20));
			if (bdbPath.empty())
			{
				std::ostringstream os;
				if (!szPdb.fetchString(os))
					return false;
				out.path = os.str();
			}
			out.CvHeader = aHdr.CvHeader;
			out.Signature = aHdr.Signature;
			out.Age = aHdr.Age;
			return true;
		}

		bool CheckProgramDatabase70(PDB70_t &out, const std::string& bdbPath) const
		{
			const IMAGE_DATA_DIRECTORY* pidd(ImageDataDirectoryPtr(IMAGE_DIRECTORY_ENTRY_DEBUG));
			if (!pidd || pidd->Size == 0)
				return false;
			const IMAGE_DEBUG_DIRECTORY &aDir(ImageDebugDirectory());
			if (aDir.Type != IMAGE_DEBUG_TYPE_CODEVIEW)
				return false;
			OFF_t oHdr(aDir.PointerToRawData);
			DataFetch_t<DWORD> dwSig(mData, oHdr);
			if (strncmp((const char *)&dwSig, "RSDS", 4) != 0)
				return false;
			DataFetch2_t<CV_INFO_PDB70> aHdr(mData, oHdr);
			DataStream_t szPdb(mData, oHdr + sizeof(CV_INFO_PDB70));
			if (bdbPath.empty())
			{
				std::ostringstream os;
				if (!szPdb.fetchString(os))
					return false;
				out.path = os.str();
			}
			out.dwSig = aHdr.dwSig;
			out.guidSig = aHdr.guidSig;
			out.age = aHdr.age;
			return true;
		}

		const COR_MetaDataInfo &__COR_MetaDataInfo() const
		{
			if (!m_pCORmeta)
			{
				const IMAGE_DATA_DIRECTORY &idd14(ImageDataDirectory(IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR));
				if (idd14.Size == 0)
					throw FormatFault(-1);
				DataFetch2_t<IMAGE_COR20_HEADER> aHeader(mData, RvaToFO(idd14.VirtualAddress));
				DWORD rvaMetaData(aHeader.MetaData.VirtualAddress);
				OFF_t oMeta(RvaToFO(rvaMetaData));
				//ADCPE::COR_MetaDataInfo w(p, a.MetaData.VirtualAddress);
				//addr += RVA2VA(w.rvaStrings);
				//return I_Front::AKIND_VA;
				struct s_t
				{
					DWORD	Signature;
					WORD	MajorVersion;
					WORD	MinorVersion;
					DWORD	Reserved;
					DWORD	Length;
					// variable part...
					//char	Version[1];
					//WORD	Flags;
					//WORD	Streams;
				};

				DataFetch2_t<s_t> aMetaHdr(mData, oMeta);
				//s_t &r(*(s_t *)pData);
				size_t o(0);
				o += sizeof(s_t);
				o += aMetaHdr.Length;//skip version string length
				o += PADDING<DWORD>(rvaMetaData + o);
				DataFetch_t<WORD> uFlags(mData, oMeta + o);
				o += sizeof(WORD);//Flags
				DataFetch_t<WORD> uStreams(mData, oMeta + o);
				o += sizeof(WORD);//Streams
				const_cast<PE_t *>(this)->m_pCORmeta = new COR_MetaDataInfo;
				for (size_t j(0); j < uStreams; j++)
				{
					DataFetch2_t<COR_StreamHeader> aStreamHdr(mData, oMeta + o);
					o += sizeof(DWORD);//Offset
					o += sizeof(DWORD);//Size
					DataStream_t aName(mData, oMeta + o);
					std::ostringstream s;
					aName.fetchString(s);
					if (s.str() == "#Strings")
						m_pCORmeta->rvaStrings = rvaMetaData + aStreamHdr.Offset;
					o += s.str().length() + 1;
					o += PADDING<DWORD>(rvaMetaData + o);
				}
			}

			return *m_pCORmeta;
		}

		int CheckWindowsPortableExecutable() const
		{
			if (ImageDOSHeader().e_magic == IMAGE_DOS_SIGNATURE
#ifdef IMAGE_SAFE_DOS_SIGNATURE
				|| ImageDOSHeader().e_magic == IMAGE_SAFE_DOS_SIGNATURE
#endif
				)
			{
				DataFetch_t<DWORD> aSig(mData, ImageDOSHeader().e_lfanew);
				if (aSig == IMAGE_NT_SIGNATURE)
				{
					if (ImageOptionalHeader().Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
						return 1;
					if (ImageOptionalHeader().Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
						return 2;
				}
			}
			return 0;
		}

		bool is64bit() const {
			return ImageOptionalHeader().Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC;
		}
		DWORD GetEntryPointRVA() const {
			return ImageOptionalHeader().AddressOfEntryPoint;
		}
		XWORD imageBase() const {
			return (XWORD)ImageOptionalHeader().ImageBase;
		}

		bool IsRvaValid(DWORD rva) const
		{
			if (rva == 0)
				return true;
			return RvaToFO(rva) != OFF_NULL;
		}

		int CheckDataDirectory(unsigned index) const
		{
			const IMAGE_DATA_DIRECTORY &idd(ImageDataDirectory(index));
			if (idd.Size == 0)
				return 0;//missing
			if (index == IMAGE_DIRECTORY_ENTRY_EXPORT)
			{
				OFF_t p(RvaToFO(idd.VirtualAddress));
				if (p == OFF_NULL)
					return -1;//invalid
				DataFetch2_t<IMAGE_EXPORT_DIRECTORY> ied(mData, p);
				if (!IsRvaValid(ied.Name) || !IsRvaValid(ied.AddressOfFunctions) || !IsRvaValid(ied.AddressOfNames) || !IsRvaValid(ied.AddressOfNameOrdinals))
					return -1;
			}
			return 1;
		}

		BOOL RvaToFileOffset(DWORD& rva) const
		{
			for (WORD i(0); i < ImageFileHeader().NumberOfSections; i++)
			{
				if (!(i < ImageFileHeader().NumberOfSections))
					break;
				const IMAGE_SECTION_HEADER &ish(ImageSectionHeader(i));
				if (rva >= ish.VirtualAddress && rva < ish.VirtualAddress + ish.SizeOfRawData)
				{
					rva = rva - ish.VirtualAddress + ish.PointerToRawData;
					return 1;
				}
			}
			return 0;
		}

		OFF_t RvaToFO(DWORD rva) const
		{
			DWORD dw(rva);
			if (RvaToFileOffset(dw))
				if (dw < mData.size())
					return dw;
			return OFF_NULL;
		}

		OFF_t __RVA2FO(DWORD rva) const
		{
			OFF_t p(RvaToFO(rva));
			if (!(p < mData.size()))
				throw FormatFault(-6);
			return p;
		}

		OFF_t VA2FO(XWORD va) const
		{
			return RvaToFO((DWORD)(va - imageBase()));
		}

		DWORD VA2RVA(XWORD va) const
		{
			if (va < imageBase())//upper bound of DWORD?
				throw FormatFault(-7);
			return (DWORD)(va - imageBase());
		}

		OFF_t __VA2FO(XWORD va) const
		{
			OFF_t p(VA2FO(va));
			if (p == OFF_NULL)
				throw FormatFault(-7);
			return p;
		}

		std::string GetSectionName(const IMAGE_SECTION_HEADER& ish) const
		{
			std::string buf(9, 0);//?
			buf.assign((const char*)ish.Name, 8);
			buf[8] = 0;
			if (buf[0] == '/')
			{
				int offs(atoi(&buf[1]));
				if (offs >= sizeof(unsigned))//skip table size
				{
					OFF_t o(StringTableEntryAtOffset2(offs));
					if (o != OFF_NULL)
					{
						DataStream_t aString(mData, o);
						std::ostringstream os;
						aString.fetchString(os);
						buf = os.str();
					}
				}
			}
			return buf;
		}

		const  IMAGE_SECTION_HEADER *FindSection(const char *name) const
		{
			for (WORD i(0); i < ImageFileHeader().NumberOfSections; i++)
			{
				const IMAGE_SECTION_HEADER &ish(ImageSectionHeader(i));
				std::string s(GetSectionName(ish));
				if (s == name)
					return &ish;
			}
			return nullptr;
		}

		const  IMAGE_SECTION_HEADER *RvaToSection(DWORD dwRVA) const
		{
			for (WORD i(0); i < ImageFileHeader().NumberOfSections; i++)
			{
				const IMAGE_SECTION_HEADER &ish(ImageSectionHeader(i));
				if (dwRVA >= ish.VirtualAddress && dwRVA < ish.VirtualAddress + ish.Misc.VirtualSize)
					return &ish;
			}
			return nullptr;
		}

		const IMAGE_SECTION_HEADER *FilePointerToSection(DWORD fp) const
		{
			for (WORD i(0); i < ImageFileHeader().NumberOfSections; i++)
			{
				const IMAGE_SECTION_HEADER &ish(ImageSectionHeader(i));
				if (fp >= ish.PointerToRawData && fp < ish.PointerToRawData + ish.SizeOfRawData)
					return &ish;
			}
			return nullptr;
		}

		bool FP2RVA(DWORD fp, DWORD &rva)
		{
			rva = fp;
			for (WORD i(0); i < ImageFileHeader().NumberOfSections; i++)
			{
				const IMAGE_SECTION_HEADER &ish(ImageSectionHeader(i));
				if (fp >= ish.PointerToRawData && fp < ish.PointerToRawData + ish.SizeOfRawData)
				{
					rva = ish.VirtualAddress + (fp - ish.PointerToRawData);
					break;
				}
			}
			return true;
		}

		//temporary
		DWORD RVA2VA(DWORD rva) const
		{
			if (is64bit())
				return rva;
			return DWORD(imageBase()) + rva;
		}

		QWORD IBASE() const
		{
			if (is64bit())
				return imageBase();
			return 0;
		}

		OFF_t StringTableEntryAtOffset2(unsigned offs) const//returns raw offs
		{
			if (ImageFileHeader().PointerToSymbolTable != 0 && ImageFileHeader().NumberOfSymbols > 0)
			{
				size_t o(ImageFileHeader().PointerToSymbolTable);
				o += ImageFileHeader().NumberOfSymbols * sizeof(IMAGE_SYMBOL_ENTRY);//skip a symbol table
				DataFetch_t<DWORD> aSize(mData, o);//size of string table
				if (offs < aSize)
				{
					o += offs;
					return o;
				}
			}
			return OFF_NULL;
		}

		OFF_t SymbolTableEntryAtFP2(DWORD fp) const//returns raw offs
		{
			DWORD fp0(ImageFileHeader().PointerToSymbolTable);
			if (fp0 != 0)
			{
				if (fp >= fp0)
				{
					DWORD index((fp - fp0) / sizeof(IMAGE_SYMBOL_ENTRY));
					if (index < ImageFileHeader().NumberOfSymbols)
						return OFF_t(fp0 + index * sizeof(IMAGE_SYMBOL_ENTRY));
				}
			}
			return OFF_NULL;
		}

		//////////////////////////////////////////////ImportModuleIterator
		class ImportModuleIterator
		{
			const PE_t &mpe;
			const IMAGE_DATA_DIRECTORY &rDirectory;
			DataFetchPtr2_t<IMAGE_IMPORT_DESCRIPTOR> pDescriptor;
		public:
			ImportModuleIterator(const PE_t &pe)
				: mpe(pe),
				rDirectory(pe.ImageDataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT)),
				pDescriptor(pe.dataSource(), pe.RvaToFO(rDirectory.VirtualAddress))
			{
			}
			OFF_t moduleName() const
			{
				return mpe.RvaToFO(pDescriptor->Name);
			}
			size_t moduleName(std::ostream& ss) const
			{
				OFF_t off(moduleName());
				if (off == OFF_NULL)
					return 0;
				DataStream_t ds(mpe.dataSource(), off);
				return ds.fetchString(ss);
			}
			operator bool() const { 
				return (pDescriptor->Name < mpe.dataSource().size());
				//return (pDescriptor->Name != 0);
			}
			ImportModuleIterator &operator++()
			{
				pDescriptor++;
				return *this;
			}
			ImportModuleIterator& operator ++(int) { return operator++(); }
		};


		//////////////////////////////////////////////ImportEntryIterator
		class ImportEntryIterator
		{
			//typedef PE_t::XWORD	XWORD;
			const PE_t &mpe;
			const IMAGE_DATA_DIRECTORY &rDirectory;
			DataFetchPtr2_t<IMAGE_IMPORT_DESCRIPTOR> pDescriptor;
			DataFetchPtr_t<XWORD> pFirstThunksRVA;
			DataFetchPtr_t<XWORD> pOriginalThunksRVA;
			DWORD firstThunksRVA;
		public:
			ImportEntryIterator(const PE_t &pe)
				: mpe(pe),
				rDirectory(pe.ImageDataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT)),
				pDescriptor(pe.dataSource(), pe.RvaToFO(rDirectory.VirtualAddress)),
				pFirstThunksRVA(pe.dataSource(), firstThunkFO()),
				pOriginalThunksRVA(pe.dataSource(), originalThunkFO()),
				firstThunksRVA(pDescriptor->FirstThunk)
			{
			}
			OFF_t moduleName() const
			{
				return mpe.RvaToFO(pDescriptor->Name);
			}
			size_t moduleName(std::ostream &ss) const
			{
				OFF_t off(moduleName());
				if (off == OFF_NULL)
					return 0;
				DataStream_t ds(mpe.dataSource(), off);
				return ds.fetchString(ss);
			}
			XWORD ordinalBitMask() const { return ~(((XWORD)-1) >> 1); }
			OFF_t functionName() const
			{
				if (!(*pOriginalThunksRVA & ordinalBitMask()))
				{
					OFF_t o(mpe.RvaToFO(DWORD(*pOriginalThunksRVA)));
					if (o != OFF_NULL)
						return o + offsetof(IMAGE_IMPORT_BY_NAME, Name);
				}
				return OFF_NULL;
			}
			size_t functionName(std::ostream &ss) const
			{
				OFF_t off(functionName());
				if (off == OFF_NULL)
					return 0;
				DataStream_t ds(mpe.dataSource(), off);
				return ds.fetchString(ss);
			}
			WORD ordinal() const {
				if (!(*pOriginalThunksRVA & ordinalBitMask()))
				{
					OFF_t o(mpe.RvaToFO(DWORD(*pOriginalThunksRVA)));
					if (o != OFF_NULL)
					{
						DataFetch2_t<IMAGE_IMPORT_BY_NAME> iibn(mpe.dataSource(), o);
						return iibn.Hint;
					}
				}
				return (WORD)*pOriginalThunksRVA;
			}
			OFF_t IIBN() const
			{
				return mpe.RvaToFO(DWORD(*pOriginalThunksRVA));
			}
			DWORD functionRVA() const { return firstThunksRVA; }
			DWORD functionNameDescRVA() const {
				return (DWORD)(*pOriginalThunksRVA & ~ordinalBitMask()); }
			bool firstThunkIsRVA() const
			{
				if (!pDescriptor->OriginalFirstThunk)
					if (!(*pFirstThunksRVA & ordinalBitMask()))
						return true;
				return false;
			}
			operator bool() const { return (pDescriptor->Name != 0); }
			ImportEntryIterator &operator++()
			{
				pFirstThunksRVA++;
				pOriginalThunksRVA++;
				firstThunksRVA += sizeof(XWORD);
				if (*pFirstThunksRVA == 0 || *pOriginalThunksRVA == 0)
				{
					pDescriptor++;
					pFirstThunksRVA = firstThunkFO();
					pOriginalThunksRVA = originalThunkFO();
					firstThunksRVA = pDescriptor->FirstThunk;
				}
				//else
					//assert(*pOriginalThunksRVA);
				return *this;
			}
			ImportEntryIterator& operator ++(int) { return operator++(); }
			unsigned thunkIndex() const
			{
				return (firstThunksRVA - pDescriptor->FirstThunk) / sizeof(XWORD);
			}
			unsigned thunksCount() const {
				OFF_t fth0(originalThunkFO());
				if (fth0 == OFF_NULL)
					throw FormatFault(-1);
				int i(0);
				//scan for zero dword
				for (DataFetchPtr_t<XWORD> p(mpe.dataSource(), fth0); *p; p++, i++);
				return i;
			}
			const IMAGE_IMPORT_DESCRIPTOR &IID() const { return *pDescriptor; }
		private:
			OFF_t originalThunkFO() const
			{
				if (!pDescriptor->OriginalFirstThunk)
					return firstThunkFO();
				return mpe.RvaToFO(pDescriptor->OriginalFirstThunk & ~ordinalBitMask());
			}
			OFF_t firstThunkFO() const
			{
				if (!pDescriptor->FirstThunk)
					return OFF_NULL;
				return mpe.RvaToFO(pDescriptor->FirstThunk & ~ordinalBitMask());
			}
		};

		/////////////////////////////////////////////////////ExportIterator
		class ExportEntryIterator
		{
			const PE_t &mpe;
			const IMAGE_DATA_DIRECTORY &idd0;
			DataFetch2_t<IMAGE_EXPORT_DIRECTORY> ied;
			DataFetchPtr_t<DWORD> pdw_0;//addresses (rva)
			DataFetchPtr_t<DWORD> pdw_1;//names (rva)
			DataFetchPtr_t<WORD> pdw_2;//ordinals
			DWORD index;//names table first, then - func addresses
		public:
			ExportEntryIterator(const PE_t &pe)
				: mpe(pe),
				idd0(pe.ImageDataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT)),
				ied(pe.dataSource(), mpe.RvaToFO(idd0.VirtualAddress)),
				pdw_0(pe.dataSource(), mpe.RvaToFO(ied.AddressOfFunctions)),
				pdw_1(pe.dataSource(), mpe.RvaToFO(ied.AddressOfNames)),
				pdw_2(pe.dataSource(), mpe.RvaToFO(ied.AddressOfNameOrdinals)),
				index(0)
			{
			}
			operator bool() const
			{
				return (index < ied.NumberOfFunctions);
			}
			ExportEntryIterator &operator++()
			{
				index++;
				return *this;
			}
			ExportEntryIterator& operator ++(int) { return operator++(); }
			//bool isNamed(PWORD meta) const { 
				//return index < ied.NumberOfNames; }
			DWORD nameRVA(PWORD meta) const {
				if (!meta)
				{
					if (index < ied.NumberOfNames)
						return pdw_1[index];
					return 0;
				}
				DWORD index2(meta[index]);
				if (index2 == 0)//?
					return 0;
				index2 -= ied.Base;
				assert(index2 < ied.NumberOfNames);
				return pdw_1[index2];
			}
			OFF_t symbolName(PWORD meta) const {
				DWORD rva(nameRVA(meta));
				if (rva != 0)
					return mpe.RvaToFO(rva);
				return OFF_NULL;
			}
			size_t symbolName(PWORD meta, std::ostream& os) const {
				OFF_t off(symbolName(meta));
				if (off == OFF_NULL)
					return 0;
				DataStream_t ds(mpe.dataSource(), off);
				return ds.fetchString(os);
			}
			DWORD funcRVA() const {
				assert(index < ied.NumberOfFunctions);
#if(0)
				if (index < ied.NumberOfNames)
					return pdw_0[pdw_2[index]/*? - ied.Base*/];
#endif
				return pdw_0[index];
			}
			DWORD funcVA() const {
				return mpe.RVA2VA(funcRVA());
			}
			OFF_t moduleName() const {
				return mpe.RvaToFO(ied.Name);
			}
			size_t moduleName(std::ostream& os) const {
				OFF_t off(moduleName());
				if (off == OFF_NULL)
					return 0;
				DataStream_t ds(mpe.dataSource(), off);
				return ds.fetchString(os);
			}
			WORD funcIdFromNameId(WORD i) const
			{
				assert(i < ied.NumberOfNames);
				return pdw_2[i];
			}
			WORD ordinal() const
			{
#if(0)
				if (index < ied.NumberOfNames)
					return pdw_2[index];
				assert(0);
#endif
				return index + ied.Base;
			}

			// A iterative binary search function. It returns location of x in
			// given array arr[l..r] if present, otherwise -1
			int binarySearch(OFF_t oArr, int l, int r, const char *x)
			{
				while (l <= r)
				{
					int m = l + (r - l) / 2;

					// Check if x is present at mid
					DataFetch_t<DWORD> aRva(mpe.dataSource(), oArr + m * sizeof(DWORD));
					
					DataStream_t aStr(mpe.dataSource(), mpe.RvaToFO(aRva));
					int res = aStr.strCmp(x);

					if (res == 0)
						return m;

					// If x greater, ignore left half  
					if (res < 0)
						l = m + 1;

					// If x is smaller, ignore right half 
					else
						r = m - 1;
				}

				// if we reach here, then element was not present
				return -1;
			}

			bool lookup(const char *name, unsigned short ordinal)
			{
				if (name && name[0])
				{
					OFF_t oArr(mpe.RvaToFO(ied.AddressOfNames));
					int i(binarySearch(oArr, 0, ied.NumberOfNames - 1, name));
					if (i >= 0)
					{
						index = pdw_2[i];//translate index in name ordinals table to index in func table
						return true;
					}
				}
				//if (ordinal >= ied.Base)
				{
					WORD i(ordinal);/// -ied.Base);
					if (i < ied.NumberOfFunctions)
					{
						index = i;
						return true;
					}
				}
				return false;
			}
		};


		/////////////////////////////////////////////////////ResourceIterator
		class ResourceIterator
		{
			const PE_t& m_pe;
			const IMAGE_DATA_DIRECTORY* m_pidd2;
			DWORD resourceBaseRVA;
			unsigned backRefs;

			struct Level : public IMAGE_RESOURCE_DIRECTORY
			{
				DWORD rvaDir;
				//DataFetchPtr2_t<IMAGE_RESOURCE_DIRECTORY> pDir;
				OFF_t oDir;
				DWORD rva;
				DataFetchPtr2_t<IMAGE_RESOURCE_DIRECTORY_ENTRY> pEntry;
				WORD i, j;
				Level(const ResourceIterator& r, DWORD offs)
					: rvaDir(r.baseRVA() + offs),
					//pDir(r.pe().dataSource(), r.pe().RvaToFO(rvaDir)),
					oDir(r.pe().RvaToFO(rvaDir)),
					rva(rvaDir + sizeof(IMAGE_RESOURCE_DIRECTORY)),
					pEntry(r.pe().dataSource(), oDir + sizeof(IMAGE_RESOURCE_DIRECTORY)),//pDir.upperBound()),
					i(0), j(0)
				{
					DataStream_t ds(r.pe().dataSource(), r.pe().RvaToFO(rvaDir));
					ds.read<IMAGE_RESOURCE_DIRECTORY>(*this);
				}
				void forward(WORD& index)
				{
					index++;
					rva += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);
					pEntry++;
				}
				bool isFirst() const { return i == 0 && j == 0; }
			};
			std::list<Level>	levels;

		public:
			ResourceIterator(const PE_t& pe)
				: m_pe(pe),
				m_pidd2(pe.ImageDataDirectoryPtr(IMAGE_DIRECTORY_ENTRY_RESOURCE)),
				resourceBaseRVA(m_pidd2 ? m_pidd2->VirtualAddress : 0),
				backRefs(0)
			{
				//if (m_pidd2->Size > 0)
				if (m_pidd2 && m_pidd2->VirtualAddress > 0)//the size can be 0
					if (m_pe.RvaToFO(m_pidd2->VirtualAddress) != OFF_NULL)//check validity of rva
						levels.push_back(Level(*this, 0));
			}
			DWORD baseRVA() const { return resourceBaseRVA; }
			const PE_t& pe() const { return m_pe; }
			ResourceIterator& operator++()
			{
				for (;;)
				{
					Level& l(level());
					if (l.i < l.NumberOfNamedEntries)
					{
						if (isDirectory())
						{
							DWORD dirOffs(l.pEntry->OffsetToDirectory);
							l.forward(l.i);
							levels.push_back(Level(*this, dirOffs));// l.forward(l.i)->OffsetToDirectory));
							Level& l2(level());
							if (l2.rvaDir > l.rvaDir)//check for resorce loops (ignore backward references)
								return *this;
							levels.pop_back();
							backRefs++;
						}
						l.forward(l.i);
						if (l.i < l.NumberOfNamedEntries)
							return *this;
					}
					if (l.j < l.NumberOfIdEntries)
					{
						if (isDirectory())
						{
							DWORD dirOffs(l.pEntry->OffsetToDirectory);
							l.forward(l.j);
							levels.push_back(Level(*this, dirOffs));// l.forward(l.j)->OffsetToDirectory));
							Level& l2(level());
							if (l2.rvaDir > l.rvaDir)//check for resorce loops (ignore backward references)
								return *this;
							levels.pop_back();
							backRefs++;
						}
						l.forward(l.j);
						if (l.j < l.NumberOfIdEntries)
							return *this;
					}
					levels.pop_back();
					if (!levels.empty())
					{
						Level& l(level());
						if (l.i < l.NumberOfNamedEntries || l.j < l.NumberOfIdEntries)
							break;
					}
					else
						break;
				}
				return *this;
			}
			bool hasBackRefs() const { return backRefs > 0; }
			ResourceIterator& operator ++(int) { return operator++(); }
			operator bool() const { return !levels.empty(); }
			bool isDirectory() const {
				return level().pEntry->DataIsDirectory;
			}
			bool isNamedEntry() const {
				return level().i < level().NumberOfNamedEntries;
			}
			const Level& level() const { return levels.back(); }
			Level& level() { return levels.back(); }
			size_t depth() const { return levels.size(); }
			DWORD rvaDir() const { return level().rvaDir; }
			DWORD rva() const { return level().rva; }
			const IMAGE_RESOURCE_DIRECTORY& IRD() const { return *level(); }
			const IMAGE_RESOURCE_DIRECTORY_ENTRY& IRDE() const { return *level().pEntry; }
			bool isFirst() const { return level().isFirst(); }
			unsigned resourceType() const {//top level entry specifies res type (already advanced)
				return levels.front().pEntry[-1].Id;

			}
			std::string namez() const
			{
				const Level& l(level());
				if (isNamedEntry())
				{
					OFF_t o(m_pe.RvaToFO(nameRVA()));
					DataFetch_t<USHORT> Length(m_pe.dataSource(), o);//IMAGE_RESOURCE_DIR_STRING_U
					//?if (!irdsu.isValid())
						//?throw FormatFault(-1);
					USHORT len(Length);
					if (len == 0)
						return "?";
					if (len > 256)
						len = 256;//set a limit due to possible errors
					std::string s;
					o += sizeof(USHORT);//skip Length

					DataStream_t NameString(m_pe.dataSource(), o);
					std::wostringstream ws;
					NameString.fetchString<WCHAR>(ws, len);

					std::string s0;
					s0.resize(len * 2);//twice as much
					UTF16toUTF8(ws.str().c_str(), &(*s0.begin()), len);
					return s0.c_str();//must do this to catch eos
				}
				return resourceIdTypeToString(l.pEntry->Id, (int)depth());
			}
			DWORD nameRVA() const
			{
				assert(isNamedEntry());
				return resourceBaseRVA + level().pEntry->DUMMYSTRUCTNAME.NameOffset;
			}
			DWORD dataEntryRVA() const
			{
				assert(!isDirectory());
				return resourceBaseRVA + level().pEntry->OffsetToData;
			}
			DWORD dataRVA() const {
				DataFetch2_t<IMAGE_RESOURCE_DATA_ENTRY> irde(m_pe.dataSource(), m_pe.RvaToFO(dataEntryRVA()));
				return irde.OffsetToData;
			}
		public:
			static std::string resourceIdTypeToString(WORD Id, int level)
			{
				if (level == 1)
					switch (Id)
					{
					case RT_CURSOR: return "Cursors";
					case RT_BITMAP: return "Bitmaps";
					case RT_ICON: return "Icons";
					case RT_MENU: return "Menus";
					case RT_DIALOG: return "Dialogs";
					case RT_STRING: return "String Table";
					case RT_FONTDIR: return "Fonts Directory";
					case RT_FONT: return "Fonts";
					case RT_ACCELERATOR: return "Accelerators";
					case RT_RCDATA: return "RC Data";
					case RT_MESSAGETABLE: return "Message Table";
					case RT_GROUP_CURSOR: return "Cursor Group";
					case RT_GROUP_ICON: return "Icon Group";
					case RT_VERSION: return "Version";
					case RT_MANIFEST: return "Manifest";
					default: break;
					}
				std::ostringstream ss;
				ss << (int)Id;
				return ss.str();
			}
		};
	};

	////////////////////////////////////////
	/*template <typename T_PE>
	class PE2_t : public PE_t
	{
	public:
	PE2_t(PBYTE pData, DWORD dwSize) : PE_t(pData, dwSize){}
	DWORD VaToRVA(XWORD va) const { return DWORD(va - m_ioh.ImageBase); }
	XWORD RvaToVa(DWORD rva) const { return rva + m_ioh.ImageBase; }
	};*/


}//namespace ADCPE





