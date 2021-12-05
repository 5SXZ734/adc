#pragma once

#include <math.h>//ceil
#include <vector>
#include <sstream>
#include "interface/IADCMain.h"
#include "shared/data_source.h"
#include "wintypes.h"
#include <stdint.h>

namespace pdb {

	typedef adcwin::GUID	GUID;

#define GUID_DEFINED
#define	_VC_VER_INC	0
	//#define	CV_ZEROLEN 1

#include "cv/cvinfo.h"

}//namespace pdb

namespace PDB20 {

	static const char PDB_SIGNATURE[] = "Microsoft C/C++ program database 2.00\r\n\x1AJG\0";
	struct PDB_STREAM
	{
		uint32_t StreamSize;  // in bytes, -1 = free stream
		uint32_t StreamPagesPtr; // array of page numbers
	};
	struct SuperBlock
	{
		uint8_t Signature[sizeof(PDB_SIGNATURE)];
		uint32_t     PageSize;   // 0x0400, 0x0800, 0x1000
		uint16_t     StartPage;   // 0x0009, 0x0005, 0x0002
		uint16_t     TotalPages;   // file size / PageSize
		PDB_STREAM  RootStream;   // stream directory
		uint16_t     RootPages[1]; // pages containing PDB_ROOT
	};

	struct NameIndexHeader
	{
		unsigned long Version;
		unsigned long TimeDateStamp;
		unsigned long Age;
		unsigned long NamesOffset;
	};
}

namespace PDB70 {

	static const char PDB_SIGNATURE[] = "Microsoft C/C++ MSF 7.00\r\n\x1A\x44\x53";

	typedef unsigned short	ulittle16_t;
	typedef unsigned int	ulittle32_t;
	//typedef int	int32_t;
	//typedef unsigned int	uint32_t;
	//typedef unsigned short	uint16_t;
	//typedef unsigned char	uint8_t;

	struct SuperBlock {
		char FileMagic[sizeof(PDB_SIGNATURE) + 2];
		ulittle32_t BlockSize;
		ulittle32_t FreeBlockMapBlock;
		ulittle32_t NumBlocks;
		ulittle32_t NumDirectoryBytes;
		ulittle32_t Unknown;
		ulittle32_t BlockMapAddr;
	};

	typedef char	Guid[16];

	struct PdbStreamHeader
	{
		ulittle32_t Version;
		ulittle32_t Signature;
		ulittle32_t Age;
		Guid UniqueId;
	};

	struct NameIndexHeader
	{
		unsigned long Version;
		unsigned long TimeDateStamp;
		unsigned long Age;
		pdb::GUID	 Guid;
		unsigned long NamesOffset;
	};

}//namespace PDB70

namespace pdb {

	enum PdbFixedStreamsEnum
	{
		PDB_STREAM_OLD_DIR,
		PDB_STREAM_PDB,
		PDB_STREAM_TPI,
		PDB_STREAM_DBI,
		PDB_STREAM_IPI
	};

	enum PdbStreamVersion {
		VC2 = 19941610,
		VC4 = 19950623,
		VC41 = 19950814,
		VC50 = 19960307,
		VC98 = 19970604,
		VC70Dep = 19990604,
		VC70 = 20000404,
		VC80 = 20030901,
		VC110 = 20091201,
		VC140 = 20140508,
	};

	struct DbiStreamHeader {
		int32_t VersionSignature;
		uint32_t VersionHeader;
		uint32_t Age;
		uint16_t GlobalStreamIndex;
		uint16_t BuildNumber;
		uint16_t PublicStreamIndex;
		uint16_t PdbDllVersion;
		uint16_t SymRecordStream;
		uint16_t PdbDllRbld;
		int32_t ModInfoSize;
		int32_t SectionContributionSize;
		int32_t SectionMapSize;
		int32_t SourceInfoSize;
		int32_t TypeServerSize;
		uint32_t MFCTypeServerIndex;
		int32_t OptionalDbgHeaderSize;
		int32_t ECSubstreamSize;
		uint16_t Flags;
		uint16_t Machine;
		uint32_t Padding;
	};

	struct SectionContribEntry {
		uint16_t Section;
		char Padding1[2];
		int32_t Offset;
		int32_t Size;
		uint32_t Characteristics;
		uint16_t ModuleIndex;
		char Padding2[2];
		uint32_t DataCrc;
		uint32_t RelocCrc;
	};

	struct SectionContribEntry2 {
		SectionContribEntry SC;
		uint32_t ISectCoff;
	};

	struct ModInfo {
		uint32_t Unused1;
		SectionContribEntry SectionContr;
		uint16_t Flags;
		uint16_t ModuleSymStream;
		uint32_t SymByteSize;
		uint32_t C11ByteSize;
		uint32_t C13ByteSize;
		uint16_t SourceFileCount;
		char Padding[2];
		uint32_t Unused2;
		uint32_t SourceFileNameIndex;
		uint32_t PdbFilePathNameIndex;
		//char ModuleName[1];
		//char ObjFileName[];
	};

	enum SectionContrSubstreamVersion {
		Ver60 = 0xeffe0000 + 19970605,
		V2 = 0xeffe0000 + 20140516
	};

	struct SectionMapHeader {
		uint16_t Count;    // Number of segment descriptors
		uint16_t LogCount; // Number of logical segment descriptors
	};

	struct SectionMapEntry {
		uint16_t Flags;         // See the SectionMapEntryFlags enum below.
		uint16_t Ovl;           // Logical overlay number
		uint16_t Group;         // Group index into descriptor array.
		uint16_t Frame;
		uint16_t SectionName;   // Byte index of segment / group name in string table, or 0xFFFF.
		uint16_t ClassName;     // Byte index of class in string table, or 0xFFFF.
		uint32_t Offset;        // Byte offset of the logical segment within physical segment.  If group is set in flags, this is the offset of the group.
		uint32_t SectionLength; // Byte count of the segment or group.
	};

	enum class SectionMapEntryFlags : uint16_t {
		Read = 1 << 0,              // Segment is readable.
		Write = 1 << 1,             // Segment is writable.
		Execute = 1 << 2,           // Segment is executable.
		AddressIs32Bit = 1 << 3,    // Descriptor describes a 32-bit linear address.
		IsSelector = 1 << 8,        // Frame represents a selector.
		IsAbsoluteAddress = 1 << 9, // Frame represents an absolute address.
		IsGroup = 1 << 10           // If set, descriptor represents a group.
	};

	struct DBIDebugHeader
	{
		unsigned short FPO;
		unsigned short exception;
		unsigned short fixup;
		unsigned short omapToSource;
		unsigned short omapFromSource;
		unsigned short sectionHdr;
		unsigned short tokenRidMap;
		unsigned short XData;
		unsigned short PData;
		unsigned short newFPO;
		unsigned short sectionHdrOriginal;
	};

	enum DBGTYPE {
		dbgtypeFPO,
		dbgtypeException,   // deprecated
		dbgtypeFixup,
		dbgtypeOmapToSrc,
		dbgtypeOmapFromSrc,
		dbgtypeSectionHdr,
		dbgtypeTokenRidMap,
		dbgtypeXdata,
		dbgtypePdata,
		dbgtypeNewFPO,
		dbgtypeSectionHdrOrig,
		dbgtypeMax          // must be last!
	};

	struct OffCb
	{
		int	off;
		int cb;
	};

	struct TypeInfoHeader
	{
		unsigned	version;
		int		headerSize;
		unsigned	min;
		unsigned	max;
		unsigned	followSize;

		unsigned short	sn;
		unsigned short	padding;
		int		hashKey;
		int		buckets;
		OffCb		hashVals;
		OffCb		tiOff;
		OffCb		hashAdjust;
	};

	struct TypeRecord
	{
		uint16_t	length;
		uint16_t	leaf;
	};

	struct CV_SubsectionHeader
	{
		int32_t sig;
		int32_t size;
	};

	struct CV_LineSection
	{
		uint32_t		off;
		uint16_t		sec;
		uint16_t		flags;
		uint32_t		cod;
	};

	struct CV_Fileblock
	{
		uint32_t fileid;
		uint32_t nLines;
		uint32_t cbFileBlock;
	};

	enum PdbStreams
	{
		PdbStream_OldDir,
		PdbStream_Pdb,
		PdbStream_Tpi,
		PdbStream_Dbi,
		PdbStream_Ipi
	};

	struct NameStreamHeader
	{
		uint32_t sig;
		int32_t	 version;
		int32_t  offset;
	};

	struct CV_FileChecksum
	{
		uint32_t	name;
		uint8_t		len;
		uint8_t		type;
		//uint8_t	data[len];
	};

	struct CV_LineInfoHeader
	{
		uint32_t	offCon;
		uint16_t	segCon;
		uint16_t	flags;
		uint32_t	cbCon;
		CV_Fileblock fileBlock;
	};

	struct HRFile
	{
		int32_t off;
		int32_t cRef;
	};

	struct GSIHashHdr
	{
		uint32_t		verSignature;
		uint32_t		verHdr;
		uint32_t		cbHr;
		uint32_t		cbBuckets;
	};

	struct PSGSIHDR
	{
		uint32_t cbSymHash;
		uint32_t cbAddrMap;
		uint32_t nThunks;
		uint32_t cbSizeOfThunk;
		uint16_t isectThunkTable;
		uint32_t offThunkTable;
		uint32_t nSects;
	};

	/*#pragma pack(push,1)
		struct lfMyEasy
		{
		unsigned short length;
		};
		template <typename T>
		struct lfMyLeaf : public lfMyEasy, public T
		{
		};
		#pragma pack(pop)*/

	/*struct lfMyEnumerate {
		unsigned short  leaf;
		CV_fldattr_t    attr;
		unsigned short   value;
		unsigned char   Name[1];
		};*/

	struct lfMyBClass
	{
		unsigned short  leaf;
		CV_fldattr_t    attr;
		CV_typ_t        index;
		unsigned short  offset;//?
	};

	/////////////////////////////////////// MyPdbReader
	class MyPdbReader
	{
		struct StreamInfo_t
		{
			struct AddrRange_t
			{
				uint16_t	seg;
				uint32_t	off;
				uint32_t	span;
				uint32_t	fileid;
				AddrRange_t(uint16_t _seg, uint32_t _off, uint32_t _span, uint32_t _fileid)
					: seg(_seg),
					off(_off),
					span(_span),
					fileid(_fileid)
				{
				}
				bool operator< (const AddrRange_t& o) const
				{
					if (seg < o.seg)
						return true;
					if (seg > o.seg)
						return false;
					return (off < o.off);
				}
				bool operator==(const AddrRange_t& o) const
				{
					return (seg == o.seg && off == o.off);
				}
				bool isInside(uint16_t _seg, uint32_t _off) const
				{
					return _seg == seg && off <= _off && _off < off + span;
				}
			};

			typedef std::set<AddrRange_t>	AddrRangeMap;
			typedef AddrRangeMap::const_iterator	AddrRangeMapCIt;

			I_DataSourceLeech* pData;
			OFF_t	oModInfo;
			OFF_t	oFileChksms;
			AddrRangeMap* pRanges;
			StreamInfo_t()
				: pData(nullptr),
				oModInfo(0),
				oFileChksms(0),
				pRanges(nullptr)
			{
			}
			~StreamInfo_t()
			{
				delete pRanges;
			}
			void addRange(AddrRange_t addr)
			{
				if (!pRanges)
					pRanges = new std::set<AddrRange_t>();
				std::pair<AddrRangeMap::iterator, bool> ret(pRanges->insert(addr));
				if (!ret.second)
					fprintf(stderr, "Error: range exists\n");
				else if (ret.first != pRanges->begin())
					if ((--ret.first)->isInside(addr.seg, addr.off))
						fprintf(stderr, "Warning: overlapping range\n");
			}
			uint32_t fileidInRangeOf(uint16_t seg, uint32_t off) const//returns beginning of a range arg falls into
			{
				if (pRanges)//there may be no lines
				{
					AddrRangeMapCIt i(pRanges->upper_bound(AddrRange_t(seg, off, 0, 0)));
					if (i != pRanges->begin())
						if ((--i)->isInside(seg, off))
							return i->fileid;
				}
				return 0;
			}
		};

		const I_DataSourceBase& mData;
		
		int m_eVersion;
		PDB20::SuperBlock* m_pPdb20Header;
		PDB70::SuperBlock* m_pSuperBlock;

		I_DataSourceLeech* m_pDirectory;
		std::vector<StreamInfo_t>* m_pStreams;
		TypeInfoHeader* m_pTpiHeader;
		TypeInfoHeader* m_pIpiHeader;
		DbiStreamHeader* m_pDbiHeader;
		std::vector<OFF_t>* m_pTypesIdx;
		std::vector<OFF_t>* m_pITypesIdx;
		std::vector<uint16_t> m_ModIdx2StrIdx;
		PDB70::NameIndexHeader* m_pPdbHeader;
		size_t m_iNamesStreamIndex;
		size_t m_iGlobalStreamIndex;
		NameStreamHeader* m_pNamesHeader;

	public:
		MyPdbReader(const I_DataSourceBase& r, int eVersion)
			: mData(r),
			m_eVersion(eVersion),
			m_pSuperBlock(nullptr),
			m_pPdb20Header(nullptr),
			m_pDirectory(nullptr),
			m_pStreams(nullptr),
			m_pTpiHeader(nullptr),
			m_pIpiHeader(nullptr),
			m_pDbiHeader(nullptr),
			m_pTypesIdx(nullptr),
			m_pITypesIdx(nullptr),
			m_pPdbHeader(nullptr),
			m_iNamesStreamIndex((size_t)-1),
			m_iGlobalStreamIndex((size_t)-1),
			m_pNamesHeader(nullptr)
		{
		}
		~MyPdbReader()
		{
			delete m_pTypesIdx;
			delete m_pTpiHeader;
			delete m_pIpiHeader;
			if (m_pStreams)
			{
				for (size_t i(0); i < m_pStreams->size(); i++)
					delete m_pStreams->at(i).pData;
				delete m_pStreams;
			}
			delete m_pDirectory;
			delete m_pSuperBlock;
			delete m_pPdb20Header;
		}
		const I_DataSourceBase& dataSource() const { return mData; }
		const PDB20::SuperBlock& PDB20_SuperBlock()
		{
			if (!m_pPdb20Header)
				const_cast<MyPdbReader*>(this)->CreatePdb20Header();
			return *m_pPdb20Header;
		}
		const PDB70::SuperBlock& PDB70_SuperBlock()
		{
			if (!m_pSuperBlock)
				const_cast<MyPdbReader*>(this)->CreateSuperBlock();
			return *m_pSuperBlock;
		}
		OFF_t DirectoryOffset()
		{
			if (m_eVersion == 2)
				return OFF_t(offsetof(PDB20::SuperBlock, RootPages));
			return OFF_t(PDB70_SuperBlock().BlockMapAddr * BlockSize());
		}
		unsigned DirectorySize()
		{
			if (m_eVersion == 2)
				return PDB20_SuperBlock().RootStream.StreamSize;
			return PDB70_SuperBlock().NumDirectoryBytes;
		}
		PDB70::ulittle32_t BlockSize()
		{
			if (m_eVersion == 2)
				return PDB20_SuperBlock().PageSize;
			return PDB70_SuperBlock().BlockSize;
		}

		template <typename T>
		size_t skipName(DataStream_t &ds) const
		{
			if (m_eVersion == 2)
				return ds.skipLPfxString<T>();
			return ds.skipStringZ<T>();
		}

		template <typename T>
		size_t fetchName(DataStream_t& ds, std::basic_ostream<T>& s) const
		{
			if (m_eVersion == 2)
				return ds.fetchLPfxString(s);
			return ds.fetchString(s);
		}

//PDB2.0->
		bool checkSignature(const adcwin::CV_INFO_PDB20& aHdr) const
		{
			const I_DataSourceLeech& aPdbStrm(Stream(PDB_STREAM_PDB));
			DataFetch2_t<PDB20::NameIndexHeader> aHdr2(aPdbStrm, 0);
			if (aHdr2.TimeDateStamp == aHdr.Signature)
				if (aHdr2.Age == aHdr.Age)
					return true;
			return false;
		}
//<--PDB2.0
		bool checkSignature(const adcwin::CV_INFO_PDB70& aHdr) const
		{
			const I_DataSourceLeech& aPdbStrm(Stream(PDB_STREAM_PDB));
			DataFetch2_t<PDB70::NameIndexHeader> aHdr2(aPdbStrm, 0);
			if (memcmp(&aHdr2.Guid, &aHdr.guidSig, sizeof(GUID)) == 0)
				if (aHdr2.Age == aHdr.age)
					return true;
			return false;
		}
		unsigned long getIDinfo(GUID &guid) const
		{
			const I_DataSourceLeech& aPdbStrm(Stream(PDB_STREAM_PDB));
			DataFetch2_t<PDB70::NameIndexHeader> aHdr(aPdbStrm, 0);
			guid = aHdr.Guid;
			return aHdr.Age;
		}
		unsigned int getIDinfo(unsigned int &/*sig*/) const
		{
			//const I_DataSourceLeech& aPdbStrm(Stream(PDB_STREAM_PDB));
			//DataFetch2_t<PDB70::NameIndexHeader> aHdr(aPdbStrm, 0);
			//sig = aHdr.Guid;
			return 0;// aHdr.Age;
		}
		const I_DataSourceLeech& Directory() const
		{
			if (!m_pDirectory)
				const_cast<MyPdbReader*>(this)->CreateDirectory();
			return *m_pDirectory;
		}
		const StreamInfo_t& StreamInfo(size_t i) const
		{
			if (!m_pStreams)
			{
				if (m_eVersion == 2)
					const_cast<MyPdbReader*>(this)->CreateStreams20();
				else//if (m_eVersion == 7)
					const_cast<MyPdbReader*>(this)->CreateStreams70();
			}
			if (!(i < m_pStreams->size()))
				throw(-1);
			return m_pStreams->at(i);
		}

		const I_DataSourceLeech& Stream(size_t i) const
		{
			return *StreamInfo(i).pData;
		}
		const I_DataSourceLeech* StreamPtr(size_t i) const
		{
			return StreamInfo(i).pData;
		}
		const I_DataSourceLeech* StreamTpiPtr() const
		{
			return StreamInfo(PDB_STREAM_TPI).pData;
		}
		const I_DataSourceLeech* StreamIpiPtr() const
		{
			if (m_eVersion == 2)
				return nullptr;
			return StreamInfo(PDB_STREAM_IPI).pData;
		}
		const I_DataSourceLeech& GlobalsSream() const
		{
			const DbiStreamHeader& aDbiHdr(DebugInfoHeader());
			return Stream(aDbiHdr.SymRecordStream);
		}
		const TypeInfoHeader& TPIHeader() const
		{
			if (!m_pTpiHeader)
				const_cast<MyPdbReader*>(this)->CreateTypeInfoHeader();
			return *m_pTpiHeader;
		}
		const TypeInfoHeader& IPIHeader() const
		{
			if (!m_pIpiHeader)
				const_cast<MyPdbReader*>(this)->CreateITypeInfoHeader();
			return *m_pIpiHeader;
		}
		const DbiStreamHeader& DebugInfoHeader() const
		{
			if (!m_pDbiHeader)
				const_cast<MyPdbReader*>(this)->CreateDebugInfoHeader();
			return *m_pDbiHeader;
		}
		OFF_t GetModInfoOffsetOfStream(size_t istrm)
		{
			assert(m_pStreams);
			assert(istrm < m_pStreams->size());
			return m_pStreams->at(istrm).oModInfo;
		}
		OFF_t FileChecksumOffset(size_t istrm) const//stream
		{
			const StreamInfo_t &a(StreamInfo(istrm));
			if (a.oFileChksms == 0)//never scanned
				const_cast<MyPdbReader*>(this)->CalculateFileChecksumOffset(istrm);
			if (a.oFileChksms == (OFF_t)-1)
				return 0;//has no chksms
			return a.oFileChksms;
		}
		OFF_t TypeOffsetFromIndex(unsigned i) const
		{
			if (i < TPIHeader().min || i >= TPIHeader().max)
				return 0;
			if (!m_pTypesIdx)
				const_cast<MyPdbReader*>(this)->CreateTypeInfoIndexer();
			return m_pTypesIdx->at(i - TPIHeader().min);
		}
		OFF_t ITypeOffsetFromIndex(unsigned i) const
		{
			if (i < IPIHeader().min || i >= IPIHeader().max)
				return 0;
			if (!m_pITypesIdx)
				const_cast<MyPdbReader*>(this)->CreateITypeInfoIndexer();
			return m_pITypesIdx->at(i - IPIHeader().min);
		}
		const PDB70::NameIndexHeader& PdbHeader() const
		{
			if (!m_pPdbHeader)
				const_cast<MyPdbReader*>(this)->CreatePdbHeader();
			return *m_pPdbHeader;
		}
		size_t NamesStreamIndex() const
		{
			if (m_iNamesStreamIndex == (size_t)-1)
				const_cast<MyPdbReader*>(this)->InitializeNamesStreamIndex();
			return m_iNamesStreamIndex;
		}
		const I_DataSourceBase& NamesStream() const
		{
			return Stream(NamesStreamIndex());
		}
		const NameStreamHeader& NamesHeader() const
		{
			if (!m_pNamesHeader)
				const_cast<MyPdbReader*>(this)->CreateNamesHeader();
			return *m_pNamesHeader;
		}
		OFF_t NameOffsetFromIndex(OFF_t i) const
		{
			return sizeof(NameStreamHeader) + i;
		}
		size_t GlobalStreamIndex() const
		{
			if (m_iGlobalStreamIndex == (size_t)-1)
				return DebugInfoHeader().GlobalStreamIndex;
			return m_iGlobalStreamIndex;
		}

		size_t NameFromAddressInModule(size_t iStrm, uint16_t seg, uint32_t off, std::ostream &os) const
		{
			const StreamInfo_t &a(StreamInfo(iStrm));
			OFF_t oFileChksms(FileChecksumOffset(iStrm));
			if (oFileChksms != 0)
			{
				uint32_t fileid(a.fileidInRangeOf(seg, off));
				if (fileid != 0)
				{
					DataStream_t aData(Stream(iStrm), oFileChksms + fileid);
					uint32_t oName;
					if (aData.read(oName))
					{
						OFF_t oName2(NameOffsetFromIndex(oName));
						DataStream_t p(NamesStream(), oName2);
						return p.fetchString(os);
					}
				}
			}
			return 0;
		}

		void CreateModIndexToStreamMap()
		{
			assert(m_ModIdx2StrIdx.empty());
			m_ModIdx2StrIdx.push_back(0);//1-biased
			for (ModInfoIterator it(*this); it; ++it)
			{
				uint16_t a(it.ModuleSymStream == 0xFFFF ? 0 : it.ModuleSymStream);
				m_ModIdx2StrIdx.push_back(a);
			}
		}

		uint16_t ModuleIndexToStream(uint16_t iMod) const
		{
			if (m_ModIdx2StrIdx.empty())
				const_cast<MyPdbReader*>(this)->CreateModIndexToStreamMap();
			if (iMod > 0 && iMod < m_ModIdx2StrIdx.size())
				return m_ModIdx2StrIdx[iMod];
			return 0;
		}

		//////////////////////////////////////// (TPI only)
		class TypeInfoIterator 
		{
		protected:
			const MyPdbReader& mPdbReader;
			const I_DataSourceBase& mTpiStrm;
		public:
			TypeInfoIterator(const MyPdbReader& r)
				: mPdbReader(r),
				mTpiStrm(r.Stream(PDB_STREAM_TPI))
			{
			}
			const MyPdbReader& reader() const { return mPdbReader; }
		};


		////////////////////////////// ITypeInfoIterator (IPI -> TPI)
		class ITypeInfoIterator : public TypeInfoIterator
		{
			const I_DataSourceBase& mIpiStrm;
			const TypeInfoHeader& mIpiHdr;
			unsigned miCur;
		public:
			ITypeInfoIterator(const MyPdbReader& r)
				: TypeInfoIterator(r),
				mIpiStrm(r.Stream(PDB_STREAM_IPI)),
				mIpiHdr(r.IPIHeader()),
				miCur(mIpiHdr.min)
			{
			}
			operator bool() const {
				return miCur < mIpiHdr.max;
			}
			unsigned range() const {
				return mIpiHdr.max - mIpiHdr.min;
			}
			unsigned index() const {
				return miCur - mIpiHdr.min;
			}
			unsigned progress() const {
				return TProgress(index(), range());
			}
			ITypeInfoIterator& operator++() {
				miCur++;
				return *this;
			}
			OFF_t ITypeOffsetFromIndex() const {//offset in IPI stream
				return mPdbReader.ITypeOffsetFromIndex(miCur);
			}
			LEAF_ENUM_e leafType() const {
				DataFetchPtr_t<uint16_t> p(mIpiStrm, ITypeOffsetFromIndex());
				if (p[0] < sizeof(uint16_t))
					return (LEAF_ENUM_e)0;
				return (LEAF_ENUM_e)p[1];
			}
			int typeIndex() const//TPI ref
			{
				DataFetch_t<int> n(mIpiStrm, ITypeOffsetFromIndex() + sizeof(TypeRecord));//skip header
				return n;
			}
			size_t source(std::ostream& os) {
				LEAF_ENUM_e e(leafType());
				if (e != pdb::LF_UDT_MOD_SRC_LINE)
					return 0;
				DataFetch2_t<lfUdtModSrcLine> a(mIpiStrm, ITypeOffsetFromIndex() + sizeof(uint16_t));//skip reclen
				DataStream_t p(mPdbReader.NamesStream(), mPdbReader.NameOffsetFromIndex(a.src));
				return p.fetchString(os);
			}
		};

		//////////////
		template <typename T>
		class TypeDesc : public DataFetchPtr2_t<T>
		{
			typedef DataFetchPtr2_t<T> BASE;
		public:
			TypeDesc(const I_DataSourceBase& r, OFF_t o)
				: DataFetchPtr2_t<T>(r, o)
			{
			}
		};


		////////////////////////////// ModInfoIterator
		class ModInfoIterator : public ModInfo
		{
		protected:
			const MyPdbReader& mPdbReader;
			const I_DataSourceBase& mDbiStrm;
			OFF_t moCur, moEnd;
		public:
			ModInfoIterator(const MyPdbReader& r, OFF_t from = 0)
				: mPdbReader(r),
				mDbiStrm(mPdbReader.Stream(PDB_STREAM_DBI)),
				moCur(from == 0 ? lower() : from),
				moEnd(moCur + upper())
			{
				fetch();
			}
			operator bool() const
			{
				return (moCur < moEnd);
			}
			ModInfoIterator& operator++()//iterate through ModInfo's
			{
				moCur += sizeof(ModInfo);
				DataStream_t ds(mDbiStrm, moCur);
				moCur += ds.skipStringZ<char>();//mPdbReader.skipName<char>(ds);//ModuleName
				moCur += ds.skipStringZ<char>();//mPdbReader.skipName<char>(ds);//ObjFileName
				moCur = ALIGNED(moCur, sizeof(uint32_t));
				fetch();
				return *this;
			}
			//ModInfoIterator& operator ++(int) { return operator++(); }//postfix
			size_t moduleName(std::ostream& os)
			{
				DataStream_t p(mDbiStrm, moCur + sizeof(ModInfo));
				return p.fetchString(os);
			}
			const I_DataSourceBase& module() const
			{
				return mPdbReader.Stream(ModuleSymStream);
			}
			bool isValid() const
			{
				if (ModuleSymStream != 0xFFFF)
					if (mPdbReader.StreamPtr(ModuleSymStream) != nullptr)
						return true;
				return false;
			}
			const MyPdbReader& reader() const { return mPdbReader; }
			const I_DataSourceBase &data() const
			{
				assert(isValid());
				return mPdbReader.Stream(ModuleSymStream);
			}
			OFF_t lower() const {
				return sizeof(DbiStreamHeader);
			}
			OFF_t upper() const {
				return mPdbReader.DebugInfoHeader().ModInfoSize;
			}
			unsigned progress() const {
				return TProgress(moCur - lower(), upper() - lower());
			}
			OFF_t current() const { return moCur; }
			unsigned streamId() const { return ModuleSymStream; }
			void finalize(){ moCur = moEnd; }
		private:
			void fetch()
			{
				if (operator bool())
					mDbiStrm.dataAt(moCur, sizeof(ModInfo), (PDATA)this);
			}
		};

		////////////////////////////// C13LineInfoIterator
		class C13LineInfoIterator : public CV_SubsectionHeader
		{
			const MyPdbReader& mr;
			const I_DataSourceBase& mrSelf;
			OFF_t moCur, moEnd;
		public:
			explicit C13LineInfoIterator(const ModInfoIterator& r)
				: mr(r.reader()),
				mrSelf(r.module()),
				moCur(OFF_t(r.SymByteSize + r.C11ByteSize)),//skip signature and
				moEnd(moCur + r.C13ByteSize)
			{
				fetch();
			}
			operator bool() const
			{
				return (moCur < moEnd);
			}
			C13LineInfoIterator& operator++()
			{
				moCur += sizeof(CV_SubsectionHeader);
				moCur += size;
				moCur = ALIGNED(moCur, sizeof(uint32_t));
				fetch();
				return *this;
			}
			//C13LineInfoIterator& operator ++(int) { return operator++(); }
			const I_DataSourceBase& self() const { return mrSelf; }
			const MyPdbReader& reader() const { return mr; }
			OFF_t cur() const { return moCur; }


			///////////////////////////////// LineInfoIterator
			class LineInfoArray : public CV_LineInfoHeader
			{
				const C13LineInfoIterator& mr;
				const I_DataSourceBase& mrSelf;
				OFF_t moCur;
			public:
				LineInfoArray(const C13LineInfoIterator& r)
					: mr(r),
					mrSelf(r.self()),
					moCur(mr.cur())
				{
					assert(r.sig == DEBUG_S_LINES);
					OFF_t o(moCur + sizeof(CV_SubsectionHeader));
					mrSelf.dataAt(o, sizeof(CV_LineInfoHeader), (PDATA)this);
				}
				StreamInfo_t::AddrRange_t range() const {
					return StreamInfo_t::AddrRange_t(segCon, offCon, cbCon, fileBlock.fileid);
				}
				uint32_t linesTotal() const {
					return fileBlock.nLines;
				}
				enum {
					AlwaysStepIntoLineNumber = 0xfeefee,//ASI
					NeverStepIntoLineNumber = 0xf00f00//NSI
				};
				ADDR targetAt(uint32_t i) const
				{
					uint32_t u;
					OFF_t o(moCur + sizeof(CV_SubsectionHeader) + sizeof(CV_LineInfoHeader) + i * sizeof(CV_Line_t));
					return (ADDR)mrSelf.dataAt(o, sizeof(uint32_t), (PDATA)&u);
				}

			private:
				void fetch()
				{
					//if (moCur < moEnd)
						//mrSelf.dataAt(moCur, sizeof(CV_LineInfoHeader), (PDATA)this);
				}
			};


			///////////////////////////////// FileChecksumIterator
			class FileChecksumIterator : public CV_FileChecksum
			{
				const C13LineInfoIterator& mr;
				const I_DataSourceBase& mrSelf;
				const I_DataSourceBase& mrNames;
				OFF_t moCur, moEnd;
			public:
				FileChecksumIterator(const C13LineInfoIterator& r)
					: mr(r),
					mrSelf(r.self()),
					mrNames(r.reader().NamesStream()),
					moCur(mr.cur() + sizeof(CV_SubsectionHeader)),
					moEnd(moCur + r.size)
				{
					assert(r.sig == DEBUG_S_FILECHKSMS);
					fetch();
				}
				operator bool() const
				{
					return (moCur < moEnd);
				}
				FileChecksumIterator& operator++()
				{
					moCur += sizeof(CV_FileChecksum);//skip length
					moCur += len;
					moCur = ALIGNED(moCur, sizeof(uint32_t));
					fetch();
					return *this;
				}
				//const I_DataSourceBase &self() const { return mrSelf; }
				size_t fetchName(std::ostream& os)
				{
					DataStream_t p(mrNames, OFF_t(sizeof(NameStreamHeader)) + OFF_t(name));
					return p.fetchString(os);
				}
			private:
				void fetch()
				{
					if (moCur < moEnd)
						mrSelf.dataAt(moCur, sizeof(CV_FileChecksum), (PDATA)this);
				}
			};


		private:
			void fetch()
			{
				if (moCur < moEnd)
					mrSelf.dataAt(moCur, sizeof(CV_SubsectionHeader), (PDATA)this);
			}
		};


	private:
		void CreateSuperBlock()
		{
			DataFetch2_t<PDB70::SuperBlock> a(mData, 0);
			m_pSuperBlock = new PDB70::SuperBlock;
			*m_pSuperBlock = a;
		}
		void CreatePdb20Header()
		{
			DataFetch2_t<PDB20::SuperBlock> a(mData, 0);
			m_pPdb20Header = new PDB20::SuperBlock;
			*m_pPdb20Header = a;
		}
		void CreateDirectory()
		{
			int uStreamDirIndicesNum((int)ceil((double)DirectorySize() / BlockSize()));
			PatchList_t aPL;
			if (m_eVersion == 2)
			{
				DataFetchPtr_t<PDB70::ulittle16_t> pSDBIndex(mData, DirectoryOffset());
				for (int i(0); i < uStreamDirIndicesNum; i++, ++pSDBIndex)
					aPL.append(*pSDBIndex * BlockSize(), BlockSize());
			}
			else
			{
				DataFetchPtr_t<PDB70::ulittle32_t> pSDBIndex(mData, DirectoryOffset());
				for (int i(0); i < uStreamDirIndicesNum; i++, ++pSDBIndex)
					aPL.append(*pSDBIndex * BlockSize(), BlockSize());
			}
			m_pDirectory = new I_DataSourceLeech(aPL, mData);
		}
		void CreateStreams20()
		{
			OFF_t o(0);
			DataFetch_t<uint16_t> uNumStreams(Directory(), o);
			o += uNumStreams.size();
			o += sizeof(uint16_t);//skip
			//an array of stream sizes
			DataFetchPtr2_t<PDB20::PDB_STREAM> pStreamSizes(Directory(), o);
			o += uNumStreams * pStreamSizes.size();
			//stream blocks array goes right after stream sizes array
			DataFetchPtr_t<uint16_t> pStreamBlocks(Directory(), o);
			m_pStreams = new std::vector<StreamInfo_t>(uNumStreams);
			for (size_t i(0); i < uNumStreams; i++, ++pStreamSizes)
			{
				const PDB20::PDB_STREAM &uStreamDesc(*pStreamSizes);
				unsigned uStreamSize(uStreamDesc.StreamSize);
				if ((int)uStreamSize <= 0)
					continue;
				size_t n(uStreamSize / BlockSize());
				PatchList_t aPL;
				for (size_t j(0); j < n; j++, ++pStreamBlocks)
					aPL.append(*pStreamBlocks * BlockSize(), BlockSize());
				size_t r(uStreamSize % BlockSize());//reminder
				if (r > 0)
				{
					aPL.append(*pStreamBlocks * BlockSize(), r);
					++pStreamBlocks;
				}
				(*m_pStreams)[i].pData = new I_DataSourceLeech(aPL, mData);
			}
			int j(0);
			for (ModInfoIterator i(*this); i; ++i, ++j)
			{
				if (i.ModuleSymStream != 0xFFFF)
					(*m_pStreams)[i.ModuleSymStream].oModInfo = i.current();
			}
		}
		void CreateStreams70()
		{
			OFF_t o(0);
			DataFetch_t<PDB70::ulittle32_t> uNumStreams(Directory(), o);
			o += uNumStreams.size();
			DataFetchPtr_t<PDB70::ulittle32_t> pStreamSizes(Directory(), o);
			o += uNumStreams * pStreamSizes.size();
			DataFetchPtr_t<PDB70::ulittle32_t> pStreamBlocks(Directory(), o);
			m_pStreams = new std::vector<StreamInfo_t>(uNumStreams);
			for (size_t i(0); i < uNumStreams; i++, ++pStreamSizes)
			{
				uint32_t uStreamSize(*pStreamSizes);
				if ((int)uStreamSize <= 0)
					continue;
				size_t n(uStreamSize / BlockSize());
				PatchList_t aPL;
				for (size_t j(0); j < n; j++, ++pStreamBlocks)
					aPL.append(*pStreamBlocks * BlockSize(), BlockSize());
				size_t r(uStreamSize % BlockSize());//reminder
				if (r > 0)
				{
					aPL.append(*pStreamBlocks * BlockSize(), r);
					++pStreamBlocks;
				}
				(*m_pStreams)[i].pData = new I_DataSourceLeech(aPL, mData);
			}
			int j(0);
			for (ModInfoIterator i(*this); i; ++i, ++j)
			{
				if (i.ModuleSymStream != 0xFFFF)
					(*m_pStreams)[i.ModuleSymStream].oModInfo = i.current();
			}
		}
		void CreateTypeInfoHeader()
		{
			const I_DataSourceLeech& aData2(Stream(PDB_STREAM_TPI));
			DataFetch2_t<pdb::TypeInfoHeader> a(aData2, 0);
			m_pTpiHeader = new pdb::TypeInfoHeader;
			*m_pTpiHeader = a;
		}
		void CreateITypeInfoHeader()
		{
			const I_DataSourceLeech& aData2(Stream(PDB_STREAM_IPI));
			DataFetch2_t<pdb::TypeInfoHeader> a(aData2, 0);
			m_pIpiHeader = new pdb::TypeInfoHeader;
			*m_pIpiHeader = a;
		}
		void CreateDebugInfoHeader()
		{
			const I_DataSourceLeech& aData2(Stream(PDB_STREAM_DBI));
			DataFetch2_t<pdb::DbiStreamHeader> a(aData2, 0);
			m_pDbiHeader = new pdb::DbiStreamHeader;
			*m_pDbiHeader = a;
		}
		void CreateTypeInfoIndexer()
		{
			const pdb::TypeInfoHeader& tih(TPIHeader());
			m_pTypesIdx = new std::vector<OFF_t>(tih.max - tih.min);
			const I_DataSourceLeech& aData(Stream(PDB_STREAM_TPI));
			OFF_t o(tih.headerSize);
			for (uint32_t i(tih.min); i < tih.max; ++i)
			{
				(*m_pTypesIdx)[i - tih.min] = o;
				DataFetch2_t<TypeRecord> a(aData, o);
				o = ALIGNED(o, sizeof(uint32_t)) + sizeof(TypeRecord);
				if (a.leaf != 0)
					o += a.length - sizeof(uint16_t);//starting from leafType
			}
		}
		void CreateITypeInfoIndexer()
		{
			const pdb::TypeInfoHeader& tih(IPIHeader());
			m_pITypesIdx = new std::vector<OFF_t>(tih.max - tih.min);
			const I_DataSourceLeech& aData(Stream(PDB_STREAM_IPI));
			OFF_t o(tih.headerSize);
			for (uint32_t i(tih.min); i < tih.max; ++i)
			{
				(*m_pITypesIdx)[i - tih.min] = o;
				DataFetch2_t<TypeRecord> a(aData, o);
				//o = ALIGNED(o, sizeof(uint32_t)) + sizeof(TypeRecord);
				if (a.leaf == 0)
					o += sizeof(TypeRecord);
				else
					o += sizeof(uint16_t) + a.length;//skip size of TypeRecord.length
			}
		}
		void CreatePdbHeader()
		{
			const I_DataSourceLeech& aData2(Stream(PDB_STREAM_PDB));
			DataFetch2_t<PDB70::NameIndexHeader> a(aData2, 0);
			m_pPdbHeader = new PDB70::NameIndexHeader;
			*m_pPdbHeader = a;
		}
		void CreateNamesHeader()
		{
			const I_DataSourceBase& aData2(NamesStream());
			DataFetch2_t<pdb::NameStreamHeader> a(aData2, 0);
			m_pNamesHeader = new pdb::NameStreamHeader;
			*m_pNamesHeader = a;
		}
		void InitializeNamesStreamIndex()
		{
			const I_DataSourceLeech& aPdb(Stream(PDB_STREAM_PDB));
			const PDB70::NameIndexHeader& aHeader(PdbHeader());
			OFF_t oNameStart(sizeof(PDB70::NameIndexHeader));
			OFF_t o(oNameStart + aHeader.NamesOffset);
			DataFetch_t<uint32_t> numOk(aPdb, o);
			o += sizeof(uint32_t);
			DataFetch_t<uint32_t> count(aPdb, o);
			o += sizeof(uint32_t);
			DataFetch_t<uint32_t> skip(aPdb, o);
			o += sizeof(uint32_t);
			if ((count / 32) > skip)
				throw - 1;//Invalid name index
			DataFetchPtr_t<uint32_t> okBits(aPdb, o);
			int n(EXTENDED(skip.get(), sizeof(uint32_t)));
			o += sizeof(uint32_t) * n;
			//OFF_t oDeletedOffset(o);
			DataFetch_t<uint32_t> deletedskip(aPdb, o);
			o += sizeof(uint32_t);
			if (deletedskip != 0)
			{
				n = EXTENDED(deletedskip.get(), sizeof(uint32_t));
				o += sizeof(uint32_t) * n;
			}
			struct StringVal
			{
				uint32_t id;
				uint32_t stream;
			};
			DataFetchPtr2_t<StringVal> val(aPdb, o);
			for (uint32_t i = 0; i < count; ++i)
			{
				if (!(okBits[i / 32] & (1 << (i % 32))))
					continue;
				DataStream_t pStr(aPdb, oNameStart + val.id);
				if (pStr.strCmp("/names") == 0)
				{
					m_iNamesStreamIndex = val.stream;
					return;
				}
				val++;
			}
		}
		void CalculateFileChecksumOffset(size_t istrm)//for 1 stream
		{
			assert(m_pStreams);
			StreamInfo_t &a((*m_pStreams)[istrm]);
			assert(a.oFileChksms == 0);//never scanned
			ModInfoIterator imod(*this, GetModInfoOffsetOfStream(istrm));
			//if (istrm > 116)
			for (C13LineInfoIterator i(imod); i; ++i)
			{
				if (i.sig == DEBUG_S_LINES)
				{
					C13LineInfoIterator::LineInfoArray b(i);
					a.addRange(b.range());
				}
				else if (i.sig == DEBUG_S_FILECHKSMS)
				{
					assert(!a.oFileChksms);//?
					a.oFileChksms = i.cur() + 8;//skip header
					//return;//hope there is no ather such symbol
				}
			}
			if (a.oFileChksms == 0)
				a.oFileChksms = (OFF_t)-1;
		}

	};

}//namespace pdb

inline int CheckPdbSignature(const I_DataSourceBase& aRaw, unsigned /*size*/)
{
	DataStream_t pc(aRaw, 0);
	std::ostringstream ss;
	pc.fetchString(ss);
	std::string s(ss.str());
	if (s == PDB20::PDB_SIGNATURE)
		return 2;
	if (s == PDB70::PDB_SIGNATURE)
		return 7;
	return 0;
}

