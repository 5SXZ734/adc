#pragma once

#define IMAGE_SAFE_DOS_SIGNATURE	0xFFFF

namespace adcwin {

	//#define nullptr    ((void *)0)

	typedef unsigned char       BYTE;
	typedef unsigned char       UBYTE;
	typedef unsigned short      WORD;
	typedef unsigned int		DWORD;
	typedef uint64_t			QWORD;
	typedef int                 BOOL;

	typedef char				CHAR;
	typedef unsigned char		UCHAR;
	typedef int16_t				SHORT;
	typedef uint16_t			USHORT;
	typedef int32_t				LONG;
	typedef uint32_t			ULONG;
	typedef float               FLOAT;
	typedef double				DOUBLE;
	typedef int64_t				LONGLONG;
	typedef uint64_t			ULONGLONG;

	typedef CHAR*				PCHAR;
	typedef DWORD*				PDWORD;
	typedef QWORD*				PQWORD;
	typedef BOOL*				PBOOL;
	typedef BYTE*				PBYTE;
	typedef WORD*				PWORD;
	typedef FLOAT*				PFLOAT;

	typedef void *PVOID;
	typedef wchar_t WCHAR;


	struct IMAGE_DOS_HEADER {
		WORD   e_magic;
		WORD   e_cblp;
		WORD   e_cp;
		WORD   e_crlc;
		WORD   e_cparhdr;
		WORD   e_minalloc;
		WORD   e_maxalloc;
		WORD   e_ss;
		WORD   e_sp;
		WORD   e_csum;
		WORD   e_ip;
		WORD   e_cs;
		WORD   e_lfarlc;
		WORD   e_ovno;
		WORD   e_res[4];
		WORD   e_oemid;
		WORD   e_oeminfo;
		WORD   e_res2[10];
		LONG   e_lfanew;
	};

#define IMAGE_DOS_SIGNATURE                 0x5A4D      // MZ
#define IMAGE_OS2_SIGNATURE                 0x454E      // NE
#define IMAGE_OS2_SIGNATURE_LE              0x454C      // LE
#define IMAGE_VXD_SIGNATURE                 0x454C      // LE
#define IMAGE_NT_SIGNATURE                  0x00004550  // PE00

	struct IMAGE_FILE_HEADER {
		WORD    Machine;
		WORD    NumberOfSections;
		DWORD   TimeDateStamp;
		DWORD   PointerToSymbolTable;
		DWORD   NumberOfSymbols;
		WORD    SizeOfOptionalHeader;
		WORD    Characteristics;
	};

	struct IMAGE_DATA_DIRECTORY {
		DWORD   VirtualAddress;
		DWORD   Size;
	};

	struct IMAGE_OPTIONAL_HEADER64
	{
		// Standard fields.
		WORD        Magic;
		BYTE        MajorLinkerVersion;
		BYTE        MinorLinkerVersion;
		DWORD       SizeOfCode;
		DWORD       SizeOfInitializedData;
		DWORD       SizeOfUninitializedData;
		DWORD       AddressOfEntryPoint;
		DWORD       BaseOfCode;
		// NT additional fields.
		ULONGLONG   ImageBase;
		DWORD       SectionAlignment;
		DWORD       FileAlignment;
		WORD        MajorOperatingSystemVersion;
		WORD        MinorOperatingSystemVersion;
		WORD        MajorImageVersion;
		WORD        MinorImageVersion;
		WORD        MajorSubsystemVersion;
		WORD        MinorSubsystemVersion;
		DWORD       Win32VersionValue;
		DWORD       SizeOfImage;
		DWORD       SizeOfHeaders;
		DWORD       CheckSum;
		WORD        Subsystem;
		WORD        DllCharacteristics;
		ULONGLONG   SizeOfStackReserve;
		ULONGLONG   SizeOfStackCommit;
		ULONGLONG   SizeOfHeapReserve;
		ULONGLONG   SizeOfHeapCommit;
		DWORD       LoaderFlags;
		DWORD       NumberOfRvaAndSizes;
		//IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
	};

	struct IMAGE_OPTIONAL_HEADER32
	{
		// Standard fields.
		WORD    Magic;
		BYTE    MajorLinkerVersion;
		BYTE    MinorLinkerVersion;
		DWORD   SizeOfCode;
		DWORD   SizeOfInitializedData;
		DWORD   SizeOfUninitializedData;
		DWORD   AddressOfEntryPoint;
		DWORD   BaseOfCode;
		DWORD   BaseOfData;
		// NT additional fields.
		DWORD   ImageBase;
		DWORD   SectionAlignment;
		DWORD   FileAlignment;
		WORD    MajorOperatingSystemVersion;
		WORD    MinorOperatingSystemVersion;
		WORD    MajorImageVersion;
		WORD    MinorImageVersion;
		WORD    MajorSubsystemVersion;
		WORD    MinorSubsystemVersion;
		DWORD   Win32VersionValue;
		DWORD   SizeOfImage;
		DWORD   SizeOfHeaders;
		DWORD   CheckSum;
		WORD    Subsystem;
		WORD    DllCharacteristics;
		DWORD   SizeOfStackReserve;
		DWORD   SizeOfStackCommit;
		DWORD   SizeOfHeapReserve;
		DWORD   SizeOfHeapCommit;
		DWORD   LoaderFlags;
		DWORD   NumberOfRvaAndSizes;
		//IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
	};

#define IMAGE_NT_OPTIONAL_HDR32_MAGIC      0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC      0x20b
#define IMAGE_ROM_OPTIONAL_HDR_MAGIC       0x107

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16


	// Directory Entries
	enum IMAGE_DIRECTORY_ENTRY
	{
		IMAGE_DIRECTORY_ENTRY_EXPORT,		// 0:Export Directory
		IMAGE_DIRECTORY_ENTRY_IMPORT,		// 1:Import Directory
		IMAGE_DIRECTORY_ENTRY_RESOURCE,		// 2:Resource Directory
		IMAGE_DIRECTORY_ENTRY_EXCEPTION,	// 3:Exception Directory
		IMAGE_DIRECTORY_ENTRY_SECURITY,		// 4:Security Directory
		IMAGE_DIRECTORY_ENTRY_BASERELOC,	// 5:Base Relocation Table
		IMAGE_DIRECTORY_ENTRY_DEBUG,		// 6:Debug Directory
		IMAGE_DIRECTORY_ENTRY_ARCHITECTURE,		// 7:Architecture Specific Data
		IMAGE_DIRECTORY_ENTRY_GLOBALPTR,	// 8:RVA of GP
		IMAGE_DIRECTORY_ENTRY_TLS,			// 9:TLS Directory
		IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,  // 10:Load Configuration Directory
		IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT,	// 11:Bound Import Directory in headers
		IMAGE_DIRECTORY_ENTRY_IAT,			// 12:Import Address Table
		IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT,		// 13:Delay Load Import Descriptors
		IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR	// 14:COM Runtime descriptor
	};

#define IMAGE_SIZEOF_SHORT_NAME              8

	struct IMAGE_SECTION_HEADER {
		BYTE    Name[IMAGE_SIZEOF_SHORT_NAME];
		union {
			DWORD   PhysicalAddress;
			DWORD   VirtualSize;
		} Misc;
		DWORD   VirtualAddress;
		DWORD   SizeOfRawData;
		DWORD   PointerToRawData;
		DWORD   PointerToRelocations;
		DWORD   PointerToLinenumbers;
		WORD    NumberOfRelocations;
		WORD    NumberOfLinenumbers;
		DWORD   Characteristics;
	};

#define IMAGE_SCN_CNT_CODE                   0x00000020  // Section contains code.
#define IMAGE_SCN_MEM_WRITE                  0x80000000	 // The section can be written to.

	struct IMAGE_RESOURCE_DIRECTORY {
		DWORD   Characteristics;
		DWORD   TimeDateStamp;
		WORD    MajorVersion;
		WORD    MinorVersion;
		WORD    NumberOfNamedEntries;
		WORD    NumberOfIdEntries;
		//  IMAGE_RESOURCE_DIRECTORY_ENTRY DirectoryEntries[];
	};

	struct IMAGE_BOUND_IMPORT_DESCRIPTOR
	{
		DWORD   TimeDateStamp;
		WORD    OffsetModuleName;
		WORD    NumberOfModuleForwarderRefs;
	};
	struct IMAGE_BOUND_FORWARDER_REF
	{
		DWORD   TimeDateStamp;
		WORD    OffsetModuleName;
		WORD    Reserved;
	};

#define IMAGE_RESOURCE_NAME_IS_STRING        0x80000000
#define IMAGE_RESOURCE_DATA_IS_DIRECTORY     0x80000000

	enum IMAGE_RESOURCE_TYPE
	{
		RT_CURSOR = 1,
		RT_BITMAP = 2,
		RT_ICON = 3,
		RT_MENU = 4,
		RT_DIALOG = 5,
		RT_STRING = 6,
		RT_FONTDIR = 7,
		RT_FONT = 8,
		RT_ACCELERATOR = 9,
		RT_RCDATA = 10,
		RT_MESSAGETABLE = 11,
		RT_GROUP_CURSOR = 12,
		RT_GROUP_ICON = 14,
		RT_VERSION = 16,
		RT_DLGINCLUDE = 17,
		RT_PLUGPLAY = 19,
		RT_VXD = 20,
		RT_ANICURSOR = 21,
		RT_ANIICON = 22,
		RT_HTML = 23,
		RT_MANIFEST = 24
	};

	struct IMAGE_RESOURCE_DIRECTORY_ENTRY {
		union {
			struct {
				DWORD NameOffset : 31;
				DWORD NameIsString : 1;
			} DUMMYSTRUCTNAME;
			DWORD   Name;
			WORD    Id;
		};
		union {
			DWORD   OffsetToData;
			struct {
				DWORD   OffsetToDirectory : 31;
				DWORD   DataIsDirectory : 1;
			};
		};
	};

	struct IMAGE_RESOURCE_DATA_ENTRY {
		DWORD   OffsetToData;
		DWORD   Size;
		DWORD   CodePage;
		DWORD   Reserved;
	};

	struct IMAGE_RESOURCE_DIR_STRING_U {
		USHORT  Length;
		WCHAR   NameString[1];
	};

	struct IMAGE_EXPORT_DIRECTORY {
		DWORD   Characteristics;
		DWORD   TimeDateStamp;
		WORD    MajorVersion;
		WORD    MinorVersion;
		DWORD   Name;
		DWORD   Base;
		DWORD   NumberOfFunctions;
		DWORD   NumberOfNames;
		DWORD   AddressOfFunctions;     // RVA from base of image
		DWORD   AddressOfNames;         // RVA from base of image
		DWORD   AddressOfNameOrdinals;  // RVA from base of image
	};

	struct  DLGTEMPLATE {
		DWORD style;
		DWORD dwExtendedStyle;
		WORD cdit;
		short x;
		short y;
		short cx;
		short cy;
	};

	struct DLGTEMPLATEEX {
		WORD      dlgVer;
		WORD      signature;
		DWORD     helpID;
		DWORD     exStyle;
		DWORD     style;
		WORD      cDlgItems;
		short     x;
		short     y;
		short     cx;
		short     cy;
		//..more follows
	};

	// Dialog Styles

#define DS_FIXEDSYS         0x0008L
#define DS_SETFONT          0x0040L

	struct DLGITEMTEMPLATE {
		DWORD style;
		DWORD dwExtendedStyle;
		short x;
		short y;
		short cx;
		short cy;
		WORD  id;
	};


	// Menus

	struct MENUHEADER {
		WORD wVersion;
		WORD cbHeaderSize;
	};

#define MF_POPUP            0x10
#define MF_END				0x80L


	// Accelerators

	struct ACCELTABLEENTRY {
		WORD   fFlags;
		WORD   wAscii;
		WORD   wId;
		WORD   padding;
	};

	// Version

	struct VS_FIXEDFILEINFO {
		DWORD dwSignature;
		DWORD dwStrucVersion;
		DWORD dwFileVersionMS;
		DWORD dwFileVersionLS;
		DWORD dwProductVersionMS;
		DWORD dwProductVersionLS;
		DWORD dwFileFlagsMask;
		DWORD dwFileFlags;
		DWORD dwFileOS;
		DWORD dwFileType;
		DWORD dwFileSubtype;
		DWORD dwFileDateMS;
		DWORD dwFileDateLS;
	};

	//Import

	struct IMAGE_IMPORT_BY_NAME {
		WORD    Hint;
		BYTE    Name[1];
	};

	struct IMAGE_IMPORT_DESCRIPTOR {
		union {
			DWORD   Characteristics;
			DWORD   OriginalFirstThunk;
		};
		DWORD   TimeDateStamp;
		DWORD   ForwarderChain;
		DWORD   Name;
		DWORD   FirstThunk;
	};

	//relocations
	struct FixupBlock
	{
		DWORD	PageRVA;
		DWORD	BlockSize;
	};



	// Load Configuration Directory Entry

	struct IMAGE_LOAD_CONFIG_DIRECTORY32
	{
		DWORD   Size;
		DWORD   TimeDateStamp;
		WORD    MajorVersion;
		WORD    MinorVersion;
		DWORD   GlobalFlagsClear;
		DWORD   GlobalFlagsSet;
		DWORD   CriticalSectionDefaultTimeout;
		DWORD   DeCommitFreeBlockThreshold;
		DWORD   DeCommitTotalFreeThreshold;
		DWORD   LockPrefixTable;                // VA
		DWORD   MaximumAllocationSize;
		DWORD   VirtualMemoryThreshold;
		DWORD   ProcessHeapFlags;
		DWORD   ProcessAffinityMask;
		WORD    CSDVersion;
		WORD    Reserved1;
		DWORD   EditList;                       // VA
		DWORD   SecurityCookie;                 // VA
		DWORD   SEHandlerTable;                 // VA
		DWORD   SEHandlerCount;
		DWORD   GuardCFCheckFunctionPointer;    // VA
		DWORD   Reserved2;
		DWORD   GuardCFFunctionTable;           // VA
		DWORD   GuardCFFunctionCount;
		DWORD   GuardFlags;
	};

	struct IMAGE_LOAD_CONFIG_DIRECTORY64
	{
		DWORD      Size;
		DWORD      TimeDateStamp;
		WORD       MajorVersion;
		WORD       MinorVersion;
		DWORD      GlobalFlagsClear;
		DWORD      GlobalFlagsSet;
		DWORD      CriticalSectionDefaultTimeout;
		ULONGLONG  DeCommitFreeBlockThreshold;
		ULONGLONG  DeCommitTotalFreeThreshold;
		ULONGLONG  LockPrefixTable;             // VA
		ULONGLONG  MaximumAllocationSize;
		ULONGLONG  VirtualMemoryThreshold;
		ULONGLONG  ProcessAffinityMask;
		DWORD      ProcessHeapFlags;
		WORD       CSDVersion;
		WORD       Reserved1;
		ULONGLONG  EditList;                    // VA
		ULONGLONG  SecurityCookie;              // VA
		ULONGLONG  SEHandlerTable;              // VA
		ULONGLONG  SEHandlerCount;
		ULONGLONG  GuardCFCheckFunctionPointer; // VA
		ULONGLONG  Reserved2;
		ULONGLONG  GuardCFFunctionTable;        // VA
		ULONGLONG  GuardCFFunctionCount;
		DWORD      GuardFlags;
	};



	///////////////////////////////////////////////////////////
	//exceptions
	enum UNWIND_OP_CODES
	{
		UWOP_PUSH_NONVOL = 0, /* info == register number */
		UWOP_ALLOC_LARGE,     /* no info, alloc size in next 2 slots */
		UWOP_ALLOC_SMALL,     /* info == size of allocation / 8 - 1 */
		UWOP_SET_FPREG,       /* no info, FP = RSP + UNWIND_INFO.FPRegOffset*16 */
		UWOP_SAVE_NONVOL,     /* info == register number, offset in next slot */
		UWOP_SAVE_NONVOL_FAR, /* info == register number, offset in next 2 slots */
		UWOP_SAVE_XMM128,     /* info == XMM reg number, offset in next slot */
		UWOP_SAVE_XMM128_FAR, /* info == XMM reg number, offset in next 2 slots */
		UWOP_PUSH_MACHFRAME   /* info == 0: no error-code, 1: error-code */
	};

	union UNWIND_CODE
	{
		struct {
			UBYTE CodeOffset;
			UBYTE UnwindOp : 4;
			UBYTE OpInfo : 4;
		};
		USHORT FrameOffset;
	};

#define UNW_FLAG_EHANDLER  0x01  
#define UNW_FLAG_UHANDLER  0x02  
#define UNW_FLAG_CHAININFO 0x04  

	struct UNWIND_INFO {
		UBYTE Version : 3;
		UBYTE Flags : 5;
		UBYTE SizeOfProlog;
		UBYTE CountOfCodes;
		UBYTE FrameRegister : 4;
		UBYTE FrameOffset : 4;
		UNWIND_CODE UnwindCode[1];
		/*  UNWIND_CODE MoreUnwindCode[((CountOfCodes + 1) & ~1) - 1];
		*   union {
		*       OPTIONAL ULONG ExceptionHandler;
		*       OPTIONAL ULONG FunctionEntry;
		*   };
		*   OPTIONAL ULONG ExceptionData[]; */
	};

	struct RUNTIME_FUNCTION {
		ULONG BeginAddress;
		ULONG EndAddress;
		ULONG UnwindData;
	};

	// Icons

	struct BITMAPINFOHEADER {
		DWORD biSize;
		LONG  biWidth;
		LONG  biHeight;
		WORD  biPlanes;
		WORD  biBitCount;
		DWORD biCompression;
		DWORD biSizeImage;
		LONG  biXPelsPerMeter;
		LONG  biYPelsPerMeter;
		DWORD biClrUsed;
		DWORD biClrImportant;
	};


	//////////////////////////////////////////////////////////////
	// Debug

	struct IMAGE_DEBUG_DIRECTORY {
		ULONG Characteristics;
		ULONG TimeDateStamp;
		USHORT MajorVersion;
		USHORT MinorVersion;
		ULONG Type;
		ULONG SizeOfData;
		ULONG AddressOfRawData;
		ULONG PointerToRawData;
	};

#define IMAGE_DEBUG_TYPE_UNKNOWN 0
#define IMAGE_DEBUG_TYPE_COFF 1
#define IMAGE_DEBUG_TYPE_CODEVIEW 2
#define IMAGE_DEBUG_TYPE_FPO 3
#define IMAGE_DEBUG_TYPE_MISC 4

	// TLS
	struct IMAGE_TLS_DIRECTORY32
	{
		DWORD   StartAddressOfRawData;
		DWORD   EndAddressOfRawData;
		DWORD   AddressOfIndex;
		DWORD   AddressOfCallBacks;
		DWORD   SizeOfZeroFill;
		DWORD   Characteristics;
	};

	struct IMAGE_TLS_DIRECTORY64
	{
		ULONGLONG   StartAddressOfRawData;
		ULONGLONG   EndAddressOfRawData;
		ULONGLONG   AddressOfIndex;
		ULONGLONG   AddressOfCallBacks;
		DWORD   SizeOfZeroFill;
		DWORD   Characteristics;
	};

	//Security

	enum WIN_CERTIFICATE_TYPE
	{
		WIN_CERT_TYPE_X509 = 1,
		WIN_CERT_TYPE_PKCS_SIGNED_DATA = 2,
		WIN_CERT_TYPE_RESERVED_1 = 3,
		WIN_CERT_TYPE_PKCS1_SIGN = 9
	};

	//Symbols

#pragma pack (push, 1)
	struct IMAGE_SYMBOL_ENTRY
	{
		union
		{
			struct//prime
			{
				union {
					char	Name[8];
					struct {
						DWORD Zeroes;
						DWORD Offset;
					};
				} prime;
				DWORD Value;
				SHORT SectionNumber;
				BYTE TypeLo;//IMAGE_SYMBOL_TYPE
				BYTE TypeHi;
				BYTE StorageClass;
				BYTE NumberOfAuxSymbols;
			};
			struct//aux1
			{
				DWORD	TagIndex;//Symbol-table index of the corresponding .bf (begin function) symbol record
				DWORD	TotalSize;//Size of the executable code for the function itself. If the function is in its own section, the Size of Raw Data in the section header will be greater or equal to this field, depending on alignment considerations.
				DWORD	PointerToLinenumber;//File offset of the first COFF line-number entry for the function, or zero if none exists. See Section 5.3, “COFF Line Numbers,” for more information. 
				DWORD	PointerToNextFunction;//Symbol-table index of the record for the next function. If the function is the last in the symbol table, this field is set to zero 
				USHORT	Unused;
			} aux1;
			struct//aux4
			{
				CHAR	FileName[18];
			} aux4;
			struct//aux5
			{
				DWORD	Length;//Size of section data; same as Size of Raw Data in the section header
				USHORT	NumberOfRelocations;//Number of relocation entries for the section
				USHORT	NumberOfLinenumbers;//Number of line-number entries for the section
				DWORD	CheckSum;//Checksum for communal data. Applicable if the IMAGE_SCN_LNK_COMDAT flag is set in the section header
				USHORT	Number;//One-based index into the Section Table for the associated section; used when the COMDAT Selection setting is 5
				BYTE	Selection;//COMDAT selection number. Applicable if the section is a COMDAT section
				BYTE	Unused[3];
			} aux5;
		};
	};
#pragma pack (pop)

	enum IMAGE_SYMBOL_TYPE
	{
		IMAGE_SYMBOL_TYPE_NULL,		// 0 No type information or unknown base type. Microsoft tools use this setting.
		IMAGE_SYMBOL_TYPE_VOID,		// 1 No valid type; used with void pointers and functions.
		IMAGE_SYMBOL_TYPE_CHAR,		// 2 Character (signed byte).
		IMAGE_SYMBOL_TYPE_SHORT,	// 3 Two-byte signed integer.
		IMAGE_SYMBOL_TYPE_INT,		// 4 Natural integer type (normally four bytes in Windows NT).
		IMAGE_SYMBOL_TYPE_LONG,		// 5 Four-byte signed integer.
		IMAGE_SYMBOL_TYPE_FLOAT,	// 6 Four-byte floating-point number.
		IMAGE_SYMBOL_TYPE_DOUBLE,	// 7 Eight-byte floating-point number.
		IMAGE_SYMBOL_TYPE_STRUCT,	// 8 Structure.
		IMAGE_SYMBOL_TYPE_UNION,	// 9 Union.
		IMAGE_SYMBOL_TYPE_ENUM,		// 10 Enumerated type.
		IMAGE_SYMBOL_TYPE_MOE,		// 11 Member of enumeration (a specific value).
		IMAGE_SYMBOL_TYPE_BYTE,		// 12 Byte; unsigned one-byte integer.
		IMAGE_SYMBOL_TYPE_WORD,		// 13 Word; unsigned two-byte integer.
		IMAGE_SYMBOL_TYPE_UINT,		// 14 Unsigned integer of natural size (normally, four bytes).
		IMAGE_SYMBOL_TYPE_DWORD,		// 15 Unsigned four-byte integer.
		IMAGE_SYMBOL_TYPE_FUNCTION	= 0x20
	};

	// Type (derived) values.
	enum IMAGE_SYMBOL_TYPE_HI
	{
		DTYPE_NULL,			//0       // no derived type.
		DTYPE_POINTER,		//1
		DTYPE_FUNCTION,		//2
		DTYPE_ARRAY			//3
	};

	enum IMAGE_SYMBOL_SECTION
	{
		IMAGE_SYM_UNDEFINED,
		IMAGE_SYM_ABSOLUTE	= -1,
		IMAGE_SYM_DEBUG		= -2
	};

	enum IMAGE_SYMBOL_CLASS
	{
		IMAGE_SYM_CLASS_END_OF_FUNCTION		= -1,
		IMAGE_SYM_CLASS_NULL                = 0,
		IMAGE_SYM_CLASS_AUTOMATIC           = 0x01,
		IMAGE_SYM_CLASS_EXTERNAL            = 0x02,
		IMAGE_SYM_CLASS_STATIC              = 0x03,
		IMAGE_SYM_CLASS_REGISTER            = 0x04,
		IMAGE_SYM_CLASS_EXTERNAL_DEF        = 0x05,
		IMAGE_SYM_CLASS_LABEL               = 0x06,
		IMAGE_SYM_CLASS_UNDEFINED_LABEL     = 0x07,
		IMAGE_SYM_CLASS_MEMBER_OF_STRUCT    = 0x08,
		IMAGE_SYM_CLASS_ARGUMENT            = 0x09,
		IMAGE_SYM_CLASS_STRUCT_TAG          = 0x0A,
		IMAGE_SYM_CLASS_MEMBER_OF_UNION     = 0x0B,
		IMAGE_SYM_CLASS_UNION_TAG           = 0x0C,
		IMAGE_SYM_CLASS_TYPE_DEFINITION     = 0x0D,
		IMAGE_SYM_CLASS_UNDEFINED_STATIC    = 0x0E,
		IMAGE_SYM_CLASS_ENUM_TAG            = 0x0F,
		IMAGE_SYM_CLASS_MEMBER_OF_ENUM      = 0x10,
		IMAGE_SYM_CLASS_REGISTER_PARAM      = 0x11,
		IMAGE_SYM_CLASS_BIT_FIELD           = 0x12,
		IMAGE_SYM_CLASS_FAR_EXTERNAL        = 0x44,
		IMAGE_SYM_CLASS_BLOCK               = 0x64,
		IMAGE_SYM_CLASS_FUNCTION            = 0x65,
		IMAGE_SYM_CLASS_END_OF_STRUCT       = 0x66,
		IMAGE_SYM_CLASS_FILE                = 0x67,
		IMAGE_SYM_CLASS_SECTION             = 0x68,
		IMAGE_SYM_CLASS_WEAK_EXTERNAL       = 0x69,
		IMAGE_SYM_CLASS_CLR_TOKEN           = 0x6B
	};


	struct IMAGE_COR20_HEADER
	{
		DWORD                   cb;
		WORD                    MajorRuntimeVersion;
		WORD                    MinorRuntimeVersion;
		IMAGE_DATA_DIRECTORY    MetaData;
		DWORD                   Flags;
		union {
			DWORD               EntryPointToken;
			DWORD               EntryPointRVA;
		} DUMMYUNIONNAME;
		IMAGE_DATA_DIRECTORY    Resources;
		IMAGE_DATA_DIRECTORY    StrongNameSignature;
		IMAGE_DATA_DIRECTORY    CodeManagerTable;
		IMAGE_DATA_DIRECTORY    VTableFixups;
		IMAGE_DATA_DIRECTORY    ExportAddressTableJumps;
		IMAGE_DATA_DIRECTORY    ManagedNativeHeader;
		// Managed Native Code
		/*?IMAGE_DATA_DIRECTORY EEInfoTable;
		IMAGE_DATA_DIRECTORY HelperTable;
		IMAGE_DATA_DIRECTORY DynamicInfo;
		IMAGE_DATA_DIRECTORY DelayLoadInfo;
		IMAGE_DATA_DIRECTORY ModuleImage;
		IMAGE_DATA_DIRECTORY ExternalFixups;
		IMAGE_DATA_DIRECTORY RidMap;
		IMAGE_DATA_DIRECTORY DebugMap;
		IMAGE_DATA_DIRECTORY IPMap;*/
	};

	struct GUID	// size is 16
	{          
		unsigned long   Data1;
		unsigned short  Data2;
		unsigned short  Data3;
		unsigned char   Data4[8];
	};

	struct CV_HEADER
	{
		DWORD Signature;
		DWORD Offset;
	};

	struct CV_INFO_PDB20
	{
		CV_HEADER CvHeader;
		DWORD Signature;
		DWORD Age;
		//BYTE PdbFileName[1];
	};

	struct CV_INFO_PDB70				// RSDS debug info
	{
		DWORD   dwSig;					// RSDS
		GUID    guidSig;
		DWORD   age;
		//char    szPdb[1];
	};

	struct COR_TablesRoot
	{
		DWORD	Reserved;
		BYTE	MajorVersion;
		BYTE	MinorVersion;
		BYTE	HeapSizes;
		BYTE	Reserved2;
		QWORD	Valid;
		QWORD	Sorted;
		DWORD	Rows[1];
	};

#define		IMAGE_ORDINAL_FLAG32	0x80000000

	typedef enum _IMAGE_FILE_MACHINE {
		UNKNOWN = 0,
		I386 = 0x014c,
		R3000 = 0x0162,
		R4000 = 0x0166,
		R10000 = 0x0168,
		WCEMIPSV2 = 0x0169,
		ALPHA = 0x0184,
		SH3 = 0x01a2,
		SH3DSP = 0x01a3,
		SH3E = 0x01a4,
		SH4 = 0x01a6,
		SH5 = 0x01a8,
		ARM = 0x01c0,
		THUMB = 0x01c2,
		ARM2 = 0x01c4,
		AM33 = 0x01d3,
		POWERPC = 0x01F0,
		POWERPCFP = 0x01f1,
		IA64 = 0x0200,
		MIPS16 = 0x0266,
		ALPHA64 = 0x0284,
		MIPSFPU = 0x0366,
		MIPSFPU16 = 0x0466,
		AXP64 = 0x0284,
		TRICORE = 0x0520,
		CEF = 0x0CEF,
		EBC = 0x0EBC,
		AMD64 = 0x8664,
		M32R = 0x9041,
		CEE = 0xC0EE
	} IMAGE_FILE_MACHINE;

}//namespace adcwin




