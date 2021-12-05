#pragma once

#include "shared/defs.h"

typedef uint32_t	Elf32_Addr;// (!)
typedef uint64_t	Elf64_Addr;

typedef uint32_t	Elf32_Off;// (!)
typedef uint64_t	Elf64_Off;

typedef uint16_t	Elf32_Half;
typedef uint16_t	Elf64_Half;

typedef int32_t	Elf32_SWord;
typedef int32_t	Elf64_Sword;

typedef uint32_t	Elf_Word;
typedef uint32_t	Elf32_Word;
typedef uint32_t	Elf64_Word;

typedef int64_t	Elf64_Sxword;
//typedef uint64_t	Elf64_Lword;
typedef uint64_t	Elf64_Xword;

typedef	unsigned char	uchar;

#define EI_NIDENT	16

enum Elf_EI
{
	EI_MAG0 = 0,// File identification
	EI_MAG1 = 1,// File identification
	EI_MAG2 = 2,// File identification
	EI_MAG3 = 3,// File identification
	EI_CLASS = 4,// File class
	EI_DATA = 5,// Data encoding
	EI_VERSION = 6,// File version
	EI_PAD = 7// Start of padding bytes
};

enum Elf_SHT
{
	SHT_NULL = 0,
	SHT_PROGBITS = 1,
	SHT_SYMTAB = 2,
	SHT_STRTAB = 3,
	SHT_RELA = 4,
	SHT_HASH = 5,
	SHT_DYNAMIC = 6,
	SHT_NOTE = 7,
	SHT_NOBITS = 8,
	SHT_REL = 9,
	SHT_SHLIB = 10,
	SHT_DYNSYM = 11,
	SHT_INIT_ARRAY = 14,
	SHT_FINI_ARRAY = 15,
	SHT_PREINIT_ARRAY = 16,
	SHT_GROUP = 17,
	SHT_SYMTAB_SHNDX = 18,
	SHT_NUM = 19,
	// GNU extensions
	SHT_GNU_verdef = 0x6ffffffd,
	SHT_GNU_verneed = 0x6ffffffe,
	SHT_GNU_versym = 0x6fffffff,
	// Solaris extensions
	SHT_LOSUNW = 0x6ffffff4,
	SHT_SUNW_dof = 0x6ffffff4,
	SHT_SUNW_cap = 0x6ffffff5,
	SHT_SUNW_SIGNATURE = 0x6ffffff6,
	SHT_SUNW_ANNOTATE = 0x6ffffff7,
	SHT_SUNW_DEBUGSTR = 0x6ffffff8,
	SHT_SUNW_DEBUG = 0x6ffffff9,
	SHT_SUNW_move = 0x6ffffffa,
	SHT_SUNW_COMDAT = 0x6ffffffb,
	SHT_SUNW_syminfo = 0x6ffffffc,
	SHT_SUNW_verdef = 0x6ffffffd,
	SHT_SUNW_verneed = 0x6ffffffe,
	SHT_SUNW_versym = 0x6fffffff,
	SHT_HISUNW = 0x6fffffff,
	SHT_LOPROC = 0x70000000,
	SHT_SPARC_GOTDATA = 0x70000000,
	SHT_AMD64_UNWIND = 0x70000001,
	SHT_HIPROC = 0x7fffffff,
	SHT_LOUSER = 0x80000000,
	SHT_HIUSER = 0xffffffff
};

enum ELF_DT
{
	DT_NULL = 0,
	DT_NEEDED = 1,
	DT_PLTRELSZ = 2,
	DT_PLTGOT = 3,
	DT_HASH = 4,
	DT_STRTAB = 5,
	DT_SYMTAB = 6,
	DT_RELA = 7,
	DT_RELASZ = 8,
	DT_RELAENT = 9,
	DT_STRSZ = 10,
	DT_SYMENT = 11,
	DT_INIT = 12,
	DT_FINI = 13,
	DT_SONAME = 14,
	DT_RPATH = 15,
	DT_SYMBOLIC = 16,
	DT_REL = 17,
	DT_RELSZ = 18,
	DT_RELENT = 19,
	DT_PLTREL = 20,
	DT_DEBUG = 21,
	DT_TEXTREL = 22,
	DT_JMPREL = 23,
	DT_BIND_NOW = 24,
	DT_INIT_ARRAY = 25,
	DT_FINI_ARRAY = 26,
	DT_INIT_ARRAYSZ = 27,
	DT_FINI_ARRAYSZ = 28,
	DT_LOOS = 0x60000000,
	DT_VALRNGLO = 0x6ffffd00,
	DT_VALRNGHI = 0x6ffffdff,
	DT_ADDRRNGLO = 0x6ffffe00,
	DT_ADDRRNGHI = 0x6ffffeff,
	DT_VERSYM = 0x6ffffff0,
	DT_RELACOUNT = 0x6ffffff9,
	DT_RELCOUNT = 0x6ffffffa,
	DT_FLAGS_1 = 0x6ffffffb,
	DT_VERDEF = 0x6ffffffc,
	DT_VERDEFNUM = 0x6ffffffd,
	DT_VERNEED = 0x6ffffffe,
	DT_VERNEEDNUM = 0x6fffffff,
	DT_HIOS = 0x6FFFFFFF,
	DT_LOPROC = 0x70000000,
	DT_HIPROC = 0x7FFFFFFF
};

enum ELF_STT
{
	STT_NOTYPE = 0,
	STT_OBJECT = 1,
	STT_FUNC = 2,
	STT_SECTION = 3,
	STT_FILE = 4,
	STT_COMMON = 5,
	STT_LOOS = 10,
	STT_HIOS = 12,
	STT_LOPROC = 13,
	STT_SPARC_REGISTER = 13,
	STT_HIPROC = 15
};

enum ELF_STB
{
	STB_LOCAL = 0,
	STB_GLOBAL = 1,
	STB_WEAK = 2,
	STB_LOOS = 10,
	STB_HIOS = 12,
	STB_LOPROC = 13,
	STB_HIPROC = 15
};


/* i386 relocs.  */

enum ELF_R_386
{
	R_386_NONE = 0,
	R_386_32 = 1,
	R_386_PC32 = 2,
	R_386_GOT32 = 3,
	R_386_PLT32 = 4,
	R_386_COPY = 5,
	R_386_GLOB_DAT = 6,
	R_386_JMP_SLOT = 7,
	R_386_RELATIVE = 8,
	R_386_GOTOFF = 9,
	R_386_GOTPC = 10,
	R_386_32PLT = 11,
	R_386_TLS_TPOFF = 14,
	R_386_TLS_IE = 15,
	R_386_TLS_GOTIE = 16,
	R_386_TLS_LE = 17,
	R_386_TLS_GD = 18,
	R_386_TLS_LDM = 19,
	R_386_16 = 20,
	R_386_PC16 = 21,
	R_386_8 = 22,
	R_386_PC8 = 23,
	R_386_TLS_GD_32 = 24,
	R_386_TLS_GD_PUSH = 25,
	R_386_TLS_GD_POP = 27,
	R_386_TLS_LDM_32 = 28,
	R_386_TLS_LDM_PUSH = 29,
	R_386_TLS_LDM_CALL = 30,
	R_386_TLS_LDM_POP = 31,
	R_386_TLS_LDO_32 = 32,
	R_386_TLS_IE_32 = 33,
	R_386_TLS_LE_32 = 34,
	R_386_TLS_DTPMOD32 = 35,
	R_386_TLS_DTPOFF32 = 36,
	R_386_TLS_TPOFF32 = 37,
	R_386_SIZE32 = 38,
	R_386_TLS_GOTDESC = 39,
	R_386_TLS_DESC_CALL = 40,
	R_386_TLS_DESC = 41,
	R_386_IRELATIVE = 42,
	R_386_GOT32X = 43,
	R_386_NUM = 44
};

////////////////////////////




 // Segment flag bits.
enum Elf_PF
{
	PF_X = 1,                // Execute
	PF_W = 2,                // Write
	PF_R = 4,                // Read
	PF_MASKOS = 0x0ff00000,  // Bits for operating system-specific semantics.
	PF_MASKPROC = 0xf0000000 // Bits for processor-specific semantics.
};

struct Elf32_t
{
	typedef Elf32_Word	Xword;
	typedef Elf32_Word	Word;
	typedef Elf32_SWord Sword;
	typedef Elf32_Addr	Addr;
	typedef Elf32_Off	Off;
	typedef Elf32_Half	Half;

	typedef struct {
		uchar e_ident[EI_NIDENT];
		Half e_type;
		Half e_machine;
		Word e_version;
		Addr e_entry;
		Off e_phoff;
		Off e_shoff;
		Word e_flags;
		Half e_ehsize;
		Half e_phentsize;
		Half e_phnum;
		Half e_shentsize;
		Half e_shnum;
		Half e_shstrndx;
	} Ehdr;
	typedef struct {
		Word sh_name;
		Word sh_type;
		Word sh_flags;
		Addr sh_addr;
		Off sh_offset;
		Xword sh_size;
		Word sh_link;
		Word sh_info;
		Word sh_addralign;
		Word sh_entsize;
	} Shdr;
	typedef struct {
		Word      p_type;
		Off       p_offset;
		Addr      p_vaddr;
		Addr      p_paddr;
		Xword     p_filesz;
		Word      p_memsz;
		Word      p_flags;
		Word      p_align;
	} Phdr;
	typedef struct {
		Sword d_tag;
		union {
			Word d_val;
			Addr d_ptr;
		} d_un;
	} Dyn;
	typedef struct {
		Word    st_name;
		Addr    st_value;
		Word    st_size;
		uchar	st_type : 4;
		uchar	st_bind : 4;
		uchar	st_other;
		Half    st_shndx;
	} Sym;
	typedef struct {
		Half    vd_version;
		Half    vd_flags;
		Half    vd_ndx;
		Half    vd_cnt;
		Word    vd_hash;
		Word    vd_aux;
		Word    vd_next;
	} Verdef;
	typedef struct {
		Word    vda_name;
		Word    vda_next;
	} Verdaux;
	typedef struct {
		Half    vn_version;
		Half    vn_cnt;
		Word    vn_file;
		Word    vn_aux;
		Word    vn_next;
	} Verneed;
	typedef struct {
		Word    vna_hash;
		Half    vna_flags;
		Half    vna_other;
		Word    vna_name;
		Word    vna_next;
	} Vernaux;
	typedef struct
	{
		Addr	r_offset;
		//Word	r_info;
		Word	r_type:8;
		Word	r_sym:24;
	} Rel;
	typedef struct
	{
		Addr	r_offset;
		//Word	r_info;
		Word	r_type:8;
		Word	r_sym:24;
		Sword	r_addend;
	} Rela;
};

struct Elf64_t
{
	typedef Elf64_Word	Word;
	typedef Elf64_Sword	Sword;
	typedef Elf64_Addr	Addr;
	typedef Elf64_Off	Off;
	typedef Elf64_Half	Half;
	typedef Elf64_Xword Xword;
	typedef Elf64_Sxword	Sxword;

	typedef struct {
		unsigned char	e_ident[EI_NIDENT];
		Half	e_type;
		Half	e_machine;
		Word	e_version;
		Addr	e_entry;
		Off	e_phoff;
		Off	e_shoff;
		Word	e_flags;
		Half	e_ehsize;
		Half	e_phentsize;
		Half	e_phnum;
		Half	e_shentsize;
		Half	e_shnum;
		Half	e_shstrndx;
	} Ehdr;						// EHDR(64)
	typedef struct {
		Word	sh_name;
		Word	sh_type;
		Xword	sh_flags;
		Addr	sh_addr;
		Off		sh_offset;
		Xword	sh_size;
		Word	sh_link;
		Word	sh_info;
		Xword	sh_addralign;
		Xword	sh_entsize;
	} Shdr;						// SHDR(64)
	typedef struct {
		Word      p_type;
		Word      p_flags;
		Off       p_offset;
		Addr      p_vaddr;
		Addr      p_paddr;
		Xword     p_filesz;
		Xword     p_memsz;
		Xword     p_align;
	} Phdr;						// PHDR(64)
	typedef struct
	{
		Sxword d_tag;
		union {
			Xword d_val;
			Addr d_ptr;
		} d_un;
	} Dyn;							// DYN(64)
	typedef struct {
		Word      st_name;
		uchar			st_type : 4;
		uchar			st_bind : 4;
		uchar			st_other;
		Half      st_shndx;
		Addr      st_value;
		Xword     st_size;
	} Sym;
	typedef struct {
		Half    vd_version;
		Half    vd_flags;
		Half    vd_ndx;
		Half    vd_cnt;
		Word    vd_hash;
		Word    vd_aux;
		Word    vd_next;
	} Verdef;
	typedef struct {
		Word    vda_name;
		Word    vda_next;
	} Verdaux;
	typedef struct {
		Half    vn_version;
		Half    vn_cnt;
		Word    vn_file;
		Word    vn_aux;
		Word    vn_next;
	} Verneed;
	typedef struct {
		Word    vna_hash;
		Half    vna_flags;
		Half    vna_other;
		Word    vna_name;
		Word    vna_next;
	} Vernaux;
	typedef struct
	{
		Addr	r_offset;
		//Xword	r_info;
		Xword	r_type:32;
		Xword	r_sym:32;
	} Rel;
	typedef struct
	{
		Addr	r_offset;
		//Xword	r_info;
		Xword	r_type:32;
		Xword	r_sym:32;
		Sxword	r_addend;
	} Rela;
};

template <typename T>//, typename T_SHDR>
class ELF_t
{
public:
	typedef	typename T::Xword	MyXword;
	typedef	typename T::Word	MyWord;
	typedef	typename T::Addr	MyAddr;
	typedef	typename T::Half	MyHalf;
	typedef	typename T::Off		MyOff;

	typedef	typename T::Ehdr	MyEhdr;
	typedef	typename T::Shdr	MyShdr;
	typedef	typename T::Phdr	MyPhdr;
	typedef	typename T::Dyn		MyDyn;
	typedef	typename T::Sym		MySym;
	typedef	typename T::Verneed	Verneed;
	typedef	typename T::Vernaux	Vernaux;
	typedef	typename T::Rel		MyRel;
	typedef	typename T::Rela	MyRela;

protected:
	const I_DataSourceBase& mData;
	MyEhdr* m_pehdr;
	MyPhdr* m_pProgramHeadersTable;
	MyShdr* m_pSectionsTable;
	MyDyn* m_pDynamicTable;
	size_t	m_DynamicTableSize;
	typedef std::map<uint32_t, std::pair<OFF_t, OFF_t>> VerIdMap;//vernum=>(module/tag offsets)
	VerIdMap* m_pVerIdMap;

	union {
		ADDR64 m_base64;
		struct {
			ADDR m_base;
			ADDR m_base_hi;
		};
	};

	bool	mb64bit;
	bool	mbSwapBytes;

	//template <typename T>
	//T *data(size_t off) const { return (T *)(mData.data(off, sizeof(T))); }

	bool isValid() const { return mData.size() > 0; }

public:
	ELF_t(const I_DataSourceBase& aRaw)
		: mData(aRaw),
		m_pehdr(nullptr),
		m_pProgramHeadersTable(nullptr),
		m_pSectionsTable(nullptr),
		m_pDynamicTable(nullptr),
		m_DynamicTableSize(-1),
		m_pVerIdMap(nullptr),
		mb64bit(Is64bit()),
		mbSwapBytes(IsMSB())
	{
		if (mb64bit)
			if (!CalcImageBase(m_base64))// || m_base_hi != 0)
				m_base64 = 0;//throw (-5);
	}

	~ELF_t()
	{
		delete m_pVerIdMap;
		delete m_pehdr;
		delete m_pProgramHeadersTable;
		delete m_pSectionsTable;
		delete m_pDynamicTable;
	}

	//cache most often accessed data
	const I_DataSourceBase& data() const { return mData; }

	const MyEhdr& ProgramHeader() const
	{
		if (!m_pehdr)
		{
			DataFetch2_t<MyEhdr> idh(mData, 0);
			const_cast<ELF_t*>(this)->m_pehdr = new MyEhdr;
			*m_pehdr = idh;
		}
		return *m_pehdr;
	}

	const MyPhdr& ProgramHeadersTable() const
	{
		if (!m_pProgramHeadersTable)
		{
			DataFetchPtr2_t<MyPhdr> pph(mData, (OFF_t)_X(ProgramHeader().e_phoff));
			size_t n(_X(ProgramHeader().e_phnum));
			const_cast<ELF_t*>(this)->m_pProgramHeadersTable = new MyPhdr[n];
			for (size_t i(0); i < n; i++, pph++)
				m_pProgramHeadersTable[i] = *pph;
		}
		return *m_pProgramHeadersTable;
	}

	const MyShdr& SectionsTable() const
	{
		if (!m_pSectionsTable)
		{
			DataFetchPtr2_t<MyShdr> psh(mData, (OFF_t)_X(ProgramHeader().e_shoff));
			size_t n(_X(ProgramHeader().e_shnum));
			const_cast<ELF_t*>(this)->m_pSectionsTable = new MyShdr[n];
			for (size_t i(0); i < n; i++, psh++)
				m_pSectionsTable[i] = *psh;
		}
		return *m_pSectionsTable;
	}

	size_t DynamicTableSize() const
	{
		if (m_DynamicTableSize == -1)
		{
			const_cast<ELF_t*>(this)->m_DynamicTableSize = 0;
			MyOff off;
			if (GetDynTableFP(off))
			{
				DataFetchPtr2_t<MyDyn> p(mData, (OFF_t)off);
				size_t n(1);
				for (DataFetchPtr2_t<MyDyn> q(p);; n++, q++)
					if (q->d_tag == DT_NULL)
						break;
				const_cast<ELF_t*>(this)->m_DynamicTableSize = n;
			}
		}
		return m_DynamicTableSize;
	}

	const MyDyn& DynamicTableEntryAt(size_t at) const
	{
		if (!m_pDynamicTable)
		{
			size_t n(DynamicTableSize());
			if (n > 0)
			{
				MyAddr off;
				GetDynTableFP(off);
				DataFetchPtr2_t<MyDyn> p(mData, (OFF_t)off);
				const_cast<ELF_t*>(this)->m_pDynamicTable = new MyDyn[n];
				for (size_t i(0); i < n; i++, p++)
					m_pDynamicTable[i] = *p;
			}
		}
		assert(at < DynamicTableSize());
		return m_pDynamicTable[at];
	}

	OFF_t GetDynsymEntryAt(size_t at) const
	{
		return OFF_NULL;
	}

	bool Is64bit() const
	{
		bool b64bit(ProgramHeader().e_ident[EI_CLASS] == 2);
		assert(b64bit || ProgramHeader().e_ident[EI_CLASS] == 1);
		return b64bit;
	}

	bool IsMSB() const
	{
		bool bMSB(ProgramHeader().e_ident[EI_DATA] == 2);
		assert(bMSB || ProgramHeader().e_ident[EI_DATA] == 1);
		return bMSB;
	}

	MyAddr RVA2VA(ADDR rva) const
	{
		if (!Is64bit())
			return rva;
		return (MyAddr)(m_base64 + rva);
	}

	ADDR VA2RVA(MyAddr va) const
	{
		if (!Is64bit())
			return ADDR(va);
		return ADDR(va - m_base64);
	}

	ADDR64 IBASE() const
	{
		if (Is64bit())
			return m_base64;
		return 0;
	}

	ADDR FP2ADDR(MyAddr a) const
	{
		return (ADDR)a;//later
	}

	template <typename U>
	inline U _X(U t) const
	{
		return mbSwapBytes ? swap_endian(t) : t;
	}

	template <typename V, typename U>
	inline V _uform(U u) const
	{
		return (V)_X(u);
	}

	template <typename U>
	inline bool IsInside(const MyShdr& shdr, U a) const
	{
		U l(_X(shdr.sh_addr));
		return (l <= a && a < l + _X(shdr.sh_size));
	}

	template <typename U>
	inline bool IsInside(const MyPhdr& phdr, U a) const
	{
		U l(_X(phdr.p_vaddr));
		return (l <= a && a < l + _X(phdr.p_memsz));
	}

	const MyShdr* GetSectionsTable() const
	{
		if (!isValid() || _X(ProgramHeader().e_shoff) >= mData.size())
			return nullptr;
		return &SectionsTable();
	}

	const MyPhdr* GetProgramHeadersTable() const
	{
		if (!isValid() || _X(ProgramHeader().e_phoff) >= mData.size())
			return nullptr;
		return &ProgramHeadersTable();
	}

	bool GetSHStringTableFP(OFF_t& fp) const
	{
		const MyShdr* pt(GetSectionsTable());
		if (!pt)
			return false;
		const MyShdr& sh(pt[_X(ProgramHeader().e_shstrndx)]);
		fp = SafeCast<OFF_t>(_X(sh.sh_offset));
		return true;
	}

	size_t GetSectionName(const MyShdr& shdr, std::ostringstream& ss) const
	{
		OFF_t oStr;
		if (!GetSHStringTableFP(oStr))
			return 0;
		if (shdr.sh_name == 0)
			return 0;
		oStr += _X(shdr.sh_name);
		DataStream_t ds(mData, oStr);
		if (ds.isAtEnd())
			return 0;
		return ds.fetchString(ss);
	}

	bool GetSectionHeaderFP(unsigned& fp, unsigned index) const
	{
		const MyShdr* pt(GetSectionsTable());
		if (!pt)
			return false;
		fp = SafeCast<unsigned>(_X(ProgramHeader().e_shoff) + index * sizeof(MyShdr));
		return true;
	}

	MyHalf GetSectionsNum() const
	{
		const MyEhdr& ehdr(ProgramHeader());
		if (!isValid() || _X(ehdr.e_shoff) >= mData.size())
			return 0;
		const MyShdr* pt(GetSectionsTable());
		return _X(ehdr.e_shnum);
	}

	const MyShdr* FindSectionHeader(Elf_SHT eType) const
	{
		MyHalf shnum(GetSectionsNum());
		if (shnum > 0)
		{
			const MyEhdr& ehdr(ProgramHeader());
			const MyShdr* pt(GetSectionsTable());
			for (int i(1); i < shnum; i++)
			{
				PDATA p((PDATA)pt + _X(ehdr.e_shentsize) * i);
				const MyShdr* psh((const MyShdr*)p);
				if (_X(psh->sh_type) == eType)//SHT_DYNAMIC)
					return psh;
			}
		}
		return nullptr;
	}

	bool GetDynTableFP(MyOff& fp) const
	{
		const MyShdr* p(FindSectionHeader(SHT_DYNAMIC));
		if (!p)
			return false;
		fp = _X(p->sh_offset);
		return true;
	}

	bool GetDynTableAddr(MyAddr& fp) const
	{
		const MyShdr* p(FindSectionHeader(SHT_DYNAMIC));
		if (!p)
			return false;
		fp = _X(p->sh_addr);
		return true;
	}

	const MyShdr* GetSectionHeaderFromRVA(unsigned rva) const
	{
		const MyShdr* pt(GetSectionsTable());
		if (pt)
		{
			const MyEhdr& ehdr(ProgramHeader());
			for (int i(1); i < _X(ehdr.e_shnum); i++)
			{
				PDATA p((PDATA)pt + _X(ehdr.e_shentsize) * i);
				const MyShdr* psh((const MyShdr*)p);
				MyAddr from(_X(psh->sh_addr));
				unsigned rawSize(_X(psh->sh_type) == SHT_NOBITS ? 0 : _uform<unsigned>(psh->sh_size));
				if (rawSize > 0)
				{
					MyAddr to(from + rawSize);
					if (from <= rva && rva < to)
						return psh;
				}
			}
		}
		return nullptr;
	}

	const MyShdr* GetSectionHeaderFromFP(MyOff off) const
	{
		const MyShdr* pt(GetSectionsTable());
		if (pt)
		{
			const MyEhdr& ehdr(ProgramHeader());
			for (int i(1); i < _X(ehdr.e_shnum); i++)
			{
				PDATA p((PDATA)pt + _X(ehdr.e_shentsize) * i);
				const MyShdr* psh((const MyShdr*)p);
				MyOff from(_X(psh->sh_offset));
				unsigned rawSize(_X(psh->sh_type) == SHT_NOBITS ? 0 : _uform<unsigned>(psh->sh_size));
				if (rawSize > 0)
				{
					MyOff to(from + rawSize);
					if (from <= off && off < to)
						return psh;
				}
			}
		}
		return nullptr;
	}

	const MyShdr* FindSectionHeader(const char* pc) const
	{
		const MyEhdr& ehdr(ProgramHeader());
		const MyShdr* shdr_str(GetSectionHeader(_X(ehdr.e_shstrndx)));
		if (shdr_str)
		{
			const MyShdr* pt(GetSectionsTable());
			if (pt)
			{
				for (int i(1); i < _X(ehdr.e_shnum); i++)
				{
					PDATA p((PDATA)pt + _X(ehdr.e_shentsize) * i);
					const MyShdr* psh((const MyShdr*)p);
					MyOff offName(_X(shdr_str->sh_offset) + _X(psh->sh_name));
					DataStream_t name(mData, offName);
					if (name.strCmp(pc, strlen(pc)) == 0)
						return psh;
				}
			}
		}
		return nullptr;
	}

	const MyDyn* GetDynamicEntryFromVA(MyAddr addr2) const
	{
		MyAddr addr;
		if (!GetDynTableAddr(addr))
			return nullptr;
		if (addr2 < addr)
			return nullptr;
		size_t n(size_t(addr2 - addr) / sizeof(MyDyn));
		if (n >= DynamicTableSize())
			return nullptr;
		return &DynamicTableEntryAt(n);
	}

	const MyShdr* GetSectionHeader(unsigned i) const
	{
		const MyShdr* pt(GetSectionsTable());
		if (pt)
		{
			PDATA p((PDATA)pt + _X(ProgramHeader().e_shentsize) * i);//can't do [] ?
			return ((const MyShdr*)p);
		}
		return nullptr;
	}

	bool GetSectionLinkFP(Elf_SHT i, MyOff& off) const
	{
		const MyShdr* psh(FindSectionHeader(i));
		if (psh)
		{
			const MyShdr* psh2(GetSectionHeader(psh->sh_link));
			if (psh2 && psh2->sh_offset != 0)
			{
				off = psh2->sh_offset;
				return true;
			}
		}
		return false;
	}

	const MyPhdr* GetProgramSectionHeader(unsigned i) const
	{
		const MyPhdr* pt(GetProgramHeadersTable());
		if (pt && i < _X(ProgramHeader().e_phnum))
		{
			PDATA p((PDATA)pt + _X(ProgramHeader().e_phentsize) * i);//can't do [] ?
			return ((const MyPhdr*)p);
		}
		return nullptr;
	}

	bool CalcImageBase(ADDR64& base) const
	{
		//go thru program headers and find one with offset 0, with size != 0 and ..
		auto e_phnum(_X(ProgramHeader().e_phnum));
		for (int i(0); i < e_phnum; i++)//fake section at index 0
		{
			const MyPhdr* phdr(GetProgramSectionHeader(i));
			if (!phdr)
				break;
			if (_X(phdr->p_offset) == 0 && _X(phdr->p_filesz) > 0 && _X(phdr->p_vaddr) != 0)
			{
				base = _X(phdr->p_vaddr);
				return true;
			}
		}
		return false;
	}

	const MyPhdr* IsMappedIntoSegment(const MyShdr& shdr)
	{
		MyAddr a0(_X(shdr.sh_addr));
		auto e_phnum(_X(ProgramHeader().e_phnum));
		for (int i(0); i < e_phnum; i++)//fake section at index 0
		{
			const MyPhdr* phdr(GetProgramSectionHeader(i));
			if (!phdr)
				break;
			if (IsInside(*phdr, a0))
				return phdr;
		}
		return nullptr;
	}

	template <typename U>
	struct sect_t
	{
		U	lo;
		U	hi;
		sect_t() {}
		sect_t(U _l, MyXword sz) : lo(_l), hi(_l + sz) {}
		bool contains(const sect_t& o) const {
			return (lo <= o.lo && o.hi <= hi);
		}
	};

	int IsMappedIntoSection(int i) const
	{
		const MyShdr& sh0(*GetSectionHeader(i));
		int n(GetSectionsNum());
		if (sh0.sh_addr != 0)
		{
			sect_t<MyOff> a(_X(sh0.sh_addr), _X(sh0.sh_size));
			for (int j(1); j < n; j++)
			{
				if (j == i) continue;
				const MyShdr& sh(*GetSectionHeader(j));
				if (sh.sh_addr != 0)
				{
					sect_t<MyOff> b(_X(sh.sh_addr), _X(sh.sh_size));
					if (b.contains(a))
						return j;
				}
			}
		}
		/*else if (sh0.sh_offset != 0)
		{
			sect_t<MyOff> a(_X(sh0.sh_offset), _X(sh0.sh_size));
			for (int j(1); j < n; j++)
			{
				if (j == i) continue;
				const MyShdr& sh(*GetSectionHeader(j));
				if (sh.sh_offset != 0)
				{
					sect_t<MyOff> b(_X(sh.sh_offset), _X(sh.sh_size));
					if (b.contains(a))
						return j;
				}
			}
		}*/
		return 0;//what's about equals?
	}

	int GetVerneedNum() const
	{
		for (size_t i(0); i < DynamicTableSize(); i++)
		{
			const MyDyn& a(DynamicTableEntryAt(i));
			if (_X(a.d_tag) == DT_VERNEEDNUM)
				return (int)_X(a.d_un.d_val);
		}
		return 0;
	}

	OFF_t VerSymToModuleTag(uint16_t n, OFF_t &oTag) const
	{
		const VerIdMap& m(GetVerIdMap());
		VerIdMap::const_iterator i(m.find(n));
		if (i != m.end())
		{
			oTag = i->second.second;
			return i->second.first;
		}
		return OFF_NULL;
	}

	uint32_t DynSymTableSize() const
	{
		const MyShdr *psh(FindSectionHeader(SHT_DYNSYM));
		if (psh)
			return uint32_t(_X(psh->sh_size) / sizeof(MySym));
		return 0;
	}

	OFF_t DynSymIndexToModuleTag(uint32_t n, OFF_t& oTag) const
	{
		if (n < DynSymTableSize())
		{
			const MyShdr* pshVersym(FindSectionHeader(SHT_GNU_versym));
			if (pshVersym)
			{
				OFF_t off(OFF_t(_X(pshVersym->sh_offset) + n * sizeof(uint16_t)));
				DataFetch_t<uint16_t> a(mData, off);
				return VerSymToModuleTag(a.get(), oTag);
			}
		}
		return OFF_NULL;
	}

	template <typename U>//a helper
	U fetchElt(OFF_t oStart, size_t index) const//for arrays
	{
		DataFetch2_t<U> a(mData, oStart + sizeof(U) * index);
		return a.get();
	}

	std::string offToString(OFF_t off) const//helper
	{
		if (off == OFF_NULL)
			return std::string();
		std::string s;
		DataStream_t ds(mData, off);
		std::ostringstream ss;
		ds.fetchString(ss);
		return ss.str();
	}

	///////////////////////////////////////////// DynsymIterator
	class DynsymIterator
	{
		const ELF_t& mrElf;
		const MyShdr* mpshDynsym;
		const MyShdr* mpshStr;
		int mSymNum;
		int miSymIndex;
		MySym mSym;//cache
	public:
		DynsymIterator(const ELF_t& elf, size_t from = 0)
			: mrElf(elf),
			mpshDynsym(mrElf.FindSectionHeader(SHT_DYNSYM)),
			mpshStr(mrElf.GetSectionHeader(mpshDynsym ? _X(mpshDynsym->sh_link) : 0)),
			mSymNum(mpshDynsym ? (int)(_X(mpshDynsym->sh_size) / sizeof(MySym)) : 0),
			miSymIndex(-1)
		{
			locate((unsigned)from);
		}
		operator bool() const
		{
			return miSymIndex < mSymNum;
		}
		DynsymIterator& operator++()
		{
			if (++miSymIndex < mSymNum)
				fetch();
			return *this;
		}
		size_t size() const {
			return mSymNum;
		}
		bool locate(unsigned index)
		{
			if (index < (unsigned)mSymNum)
			{
				mSym = mrElf.fetchElt<MySym>(_X(mpshDynsym->sh_offset), index);
				miSymIndex = index;
				return true;
			}
			return false;
		}
		OFF_t nameOff() const
		{
			if (mpshStr)
				return _X(mpshStr->sh_offset) + _X(mSym.st_name);
			return OFF_NULL;
		}
		OFF_t moduleOff(OFF_t& oTag) const
		{
			return mrElf.DynSymIndexToModuleTag(miSymIndex, oTag);
		}
		size_t namex(std::ostream& os) const
		{
			OFF_t off(nameOff());
			if (off == OFF_NULL)
				return 0;
			std::string s(mrElf.offToString(off));
			os << s;
			return s.length();
		}
		size_t modulex(std::ostream& os) const
		{
			//check for symbol versioning
			OFF_t oTag(OFF_NULL);
			OFF_t oMod(moduleOff(oTag));
			if (oMod == OFF_NULL)
				return 0;
			std::string s(mrElf.offToString(oMod));
			os << s;
			return s.length();
		}
		size_t tagx(std::ostream& os) const
		{
			//check for symbol versioning
			OFF_t oTag(OFF_NULL);
			OFF_t oMod(moduleOff(oTag));
			if (oMod == OFF_NULL)
				return 0;
			std::string s(mrElf.offToString(oTag));
			os << s;
			return s.length();
		}
		bool isFunc() const { return mSym.st_type == STT_FUNC; }
		ELF_STT st_type() const { return (ELF_STT)mSym.st_type; }
		MyAddr st_value() const { return _X(mSym.st_value); }
	private:
		void fetch()
		{
			mSym = mrElf.fetchElt<MySym>(_X(mpshDynsym->sh_offset), miSymIndex);
		}
		template <typename U> inline U _X(U t) const { return mrElf._X(t); }
	};

	/////////////////////////////////////////////// ImportIterator
	class RelIterator // iterate through REL entries whose refs fall into the target section
	{
		const ELF_t& mrElf;
		const MyShdr& mshPlt;//target section (where relocs fall into)
		//const MyShdr* mpshDynsym;
		//const MyShdr* mpshStr;
		unsigned mShIdx;
		int mRelNum;
		int mRelIdx;
		Elf_SHT meType;//discriminator
		union {
			MyRel mRel;//cache
			MyRela mRela;//cache
		};
	public:
		RelIterator(const ELF_t& elf, const MyShdr& shPlt)
			: mrElf(elf),
			mshPlt(shPlt),
			//mpshDynsym(mrElf.FindSectionHeader(SHT_DYNSYM)),
			//mpshStr(mrElf.GetSectionHeader(_X(mpshDynsym->sh_link))),
			mShIdx(0),
			mRelNum(0),
			mRelIdx(-1),
			meType(SHT_NULL)
		{
			next_rel_sec();
		}
		void operator++()
		{
			if (!next_rel())
				next_rel_sec();
		}
		operator bool() const
		{
			return mShIdx > 0 && mRelIdx > -1;
		}
		MyAddr r_offset() const { return meType == SHT_REL ? _X(mRel.r_offset) : _X(mRela.r_offset); }
		MyXword r_sym() const { return meType == SHT_REL ? mRel.r_sym : mRela.r_sym; }
	private:
		bool next_rel_sec()
		{
			++mShIdx;//advance
			//go through sections table...
			MyHalf shnum(mrElf.GetSectionsNum());
			for (; mShIdx < shnum; mShIdx++)
			{
				//...looking for REL-type (relocation) sections (can be a bunch of them)
				const MyShdr& sh(*mrElf.GetSectionHeader(mShIdx));
				meType = (Elf_SHT)_X(sh.sh_type);
				if (meType == SHT_REL)
					mRelNum = (int)(_X(sh.sh_size) / sizeof(MyRel));
				else if (meType == SHT_RELA)
					mRelNum = (int)(_X(sh.sh_size) / sizeof(MyRela));
				else
					continue;
				if (next_rel())
					return true;
			}
			mShIdx = 0;//reset
			mRelIdx = -1;
			meType = SHT_NULL;
			return false;
		}
		bool next_rel()
		{
			++mRelIdx;
			const MyShdr& sh(*mrElf.GetSectionHeader(mShIdx));
			//the one is found. Iterate its contents (Rel items)...
			for (; mRelIdx < mRelNum; mRelIdx++)
			{
				//...looking for entries with zero offset and falling inside a .got.plt section - the actual import table;
				if (meType == SHT_REL)
					mRel = mrElf.fetchElt<MyRel>(_X(sh.sh_offset), mRelIdx);
				else if (meType == SHT_RELA)
					mRela = mrElf.fetchElt<MyRela>(_X(sh.sh_offset), mRelIdx);
				else
					continue;
				MyAddr va(r_offset());
				if (va != 0 && mrElf.IsInside(mshPlt, va))
					return true;
			}
			mRelIdx = -1;
			return false;
		}
		template <typename U> inline U _X(U t) const { return mrElf._X(t); }
	};

	//////////////////////////////////////////////// VerneedIterator
	class VerneedIterator
	{
		const ELF_t& mrElf;
		const MyShdr* pshVersym;
		const MyShdr* pshVerneed;
		const MyShdr* pshVerneedStr;
		int iVerneedNum;
		int iVerneedCur;
		OFF_t oCur;
		Verneed aPvt;
	public:
		VerneedIterator(const ELF_t& r)
			: mrElf(r),
			pshVersym(mrElf.FindSectionHeader(SHT_GNU_versym)),
			pshVerneed(mrElf.FindSectionHeader(SHT_GNU_verneed)),
			pshVerneedStr(pshVerneed ? mrElf.GetSectionHeader(_X(pshVerneed->sh_link)) : nullptr),
			iVerneedNum(mrElf.GetVerneedNum()),
			oCur(pshVerneed ? (OFF_t)_X(pshVerneed->sh_offset) : OFF_NULL),
			iVerneedCur(0)
		{
			if (oCur != OFF_NULL)
				fetch();
		}
		void operator++()
		{
			oCur += _X(aPvt.vn_next);
			fetch();
			++iVerneedCur;
		}
		operator bool() const
		{
			return iVerneedCur < iVerneedNum;
		}
		OFF_t moduleNameOffset() const
		{
			if (!pshVerneedStr)
				return OFF_NULL;
			return _X(pshVerneedStr->sh_offset) + _X(aPvt.vn_file);
		}
		bool fetchModuleName(std::ostringstream& ss) const
		{
			OFF_t oFileName(moduleNameOffset());
			if (oFileName != OFF_NULL)
			{
				DataStream_t ds(mrElf.data(), oFileName);
				ds.fetchString(ss);
				return true;
			}
			return false;
		}
		int vernauxNum() const { return _X(aPvt.vn_cnt); }
		OFF_t vernauxOff() const { return oCur + _X(aPvt.vn_aux); }
		const ELF_t& elf() const { return mrElf; }
		OFF_t stringTableOffset() const 
		{
			if (pshVerneedStr)
				return _X(pshVerneedStr->sh_offset);
			return OFF_NULL;
		}
	private:
		//const Verneed& pvt() const { return aPvt; }
		//OFF_t current() const { return oCur; }
		void fetch()
		{
			DataStream_t ds(mrElf.data(), oCur);
			ds.read(aPvt);
		}
		template <typename U> inline U _X(U t) const { return mrElf._X(t); }
	};

	//////////////////////////////////////////////VernauxIterator
	class VernauxIterator
	{
		VerneedIterator& mr;
		int iVernauxNum;
		int iVernauxCur;
		OFF_t oCur;
		Vernaux aPvt;
	public:
		VernauxIterator(VerneedIterator& r)
			: mr(r),
			iVernauxNum(mr.vernauxNum()),
			iVernauxCur(0),
			oCur(mr ? mr.vernauxOff() : OFF_NULL)
		{
			if (oCur != OFF_NULL)
				fetch();
		}
		void operator++()
		{
			if (++iVernauxCur < iVernauxNum)
				oCur += _X(aPvt.vna_next);
			else
			{
				iVernauxCur = 0;
				mr.operator++();
				if (!mr.operator bool())
				{
					iVernauxNum = 0;
					oCur = OFF_NULL;
					return;
				}
				iVernauxNum = mr.vernauxNum();
				oCur = mr.vernauxOff();
			}
			fetch();
		}
		operator bool() const
		{
			if (iVernauxCur < iVernauxNum)
				return true;
			return mr.operator bool();
		}
		OFF_t tagOffset() const
		{
			OFF_t oStrings(mr.stringTableOffset());
			if (oStrings == OFF_NULL)
				return false;
			return oStrings + _X(aPvt.vna_name);
		}
		bool fetchTag(std::ostringstream& ss) const
		{
			OFF_t oTag(tagOffset());
			if (oTag == OFF_NULL)
				return false;
			DataStream_t ds(mr.elf().data(), oTag);
			ds.fetchString(ss);
			return true;
		}
		uint32_t verId() const 
		{
			return _X(aPvt.vna_other);
		}
	private:
		void fetch()
		{
			DataStream_t ds(mr.elf().data(), oCur);
			ds.read(aPvt);
		}
		template <typename U> inline U _X(U t) const { return mr.elf()._X(t); }
	};

protected:
	template <typename V, typename U>
	static V SafeCast(U u)
	{
		/*if (sizeof(V) < sizeof(U))
		{
			U t(U(-1) << sizeof(V) * 8);
			if (u & t)
				throw (-1);
		}*/
		return (V)u;
	}
private:
	const VerIdMap& GetVerIdMap() const
	{
		//initialize version id lookup map
		if (!m_pVerIdMap)
		{
			const_cast<ELF_t*>(this)->m_pVerIdMap = new VerIdMap;
			VerneedIterator ivn(*this);
			for (VernauxIterator i(ivn); i; ++i)
			{
				uint32_t verId(i.verId());
				OFF_t oModuleName(ivn.moduleNameOffset());
				OFF_t oTag(i.tagOffset());
				m_pVerIdMap->insert(std::make_pair(verId, std::make_pair(oModuleName, oTag)));
				//std::ostringstream ss;
				//ivn.fetchModuleName(ss);
				//std::ostringstream ss2;
				//i.fetchTag(ss2);
			}
		}
		return *m_pVerIdMap;
	}
};

inline int CheckElfSignature(const I_DataSourceBase& aRaw, unsigned /*size*/)//1:32bit, 2:64bit
{
	DataStream_t aPtr(aRaw, 0);
	char buf[EI_NIDENT];
	if (aPtr.read(EI_NIDENT, buf) != EI_NIDENT || strncmp(buf, "\x7F" "ELF", 4) != 0)
		return 0;
	//uchar *ident((uchar *)ptr);
	//return ident[EI_CLASS];
	return buf[EI_CLASS];
}

//https://github.com/lattera/glibc/blob/master/elf/elf.h
enum ELF_EM_t
{
	EM_NONE = 0,	/* No machine */
	EM_M32 = 1,	/* AT&T WE 32100 */
	EM_SPARC = 2,	/* SUN SPARC */
	EM_386 = 3,	/* Intel 80386 */
	//..
	EM_ARM = 40,	/* ARM */
	//..
	EM_X86_64 = 62,	/* AMD x86-64 architecture */
	//..
	EM_AARCH64 = 183,	/* ARM AARCH64 */
	//..
	EM_NUM = 248
};


