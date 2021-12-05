segment {

	struct //IMAGE_DOS_HEADER
	{
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
	} ImageDosHeader;

	assert(ImageDosHeader.e_magic == 'MZ\0\0');

	DWORD ImagePESignature @ImageDosHeader.e_lfanew;

	struct //IMAGE_FILE_HEADER
	{
		WORD    Machine;
		WORD    NumberOfSections;
		DWORD   TimeDateStamp;
		DWORD   PointerToSymbolTable;
		DWORD   NumberOfSymbols;
		WORD    SizeOfOptionalHeader;
		WORD    Characteristics;
	} ImageFileHeader;

	assert(ImageFileHeader.Magic == 0x10B);

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

	struct IMAGE_DATA_DIRECTORY {
		DWORD   VirtualAddress;
		DWORD   Size;
	};

	struct //IMAGE_OPTIONAL_HEADER
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
		IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
	} ImageOptionalHeader;

#define IMAGE_SIZEOF_SHORT_NAME	8

	struct //IMAGE_SECTION_HEADER
	{
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
	} ImageSectionHeader[ImageFileHeader.NumberOfSections];

#define IMAGE_SCN_CNT_CODE		0x00000020  // Section contains code.
#define IMAGE_SCN_MEM_WRITE		0x80000000	 // The section can be written to.

	for (ImageFileHeader.NumberOfSections)
	{
		const o=ImageSectionHeader[$];
		segment <
			code=o.Characteristics & IMAGE_SCN_CNT_CODE,
			const=o.Characteristics & IMAGE_SCN_MEM_WRITE,
			size=o.SizeOfRawData, 
			vbase=ImageOptionalHeader.ImageBase + o.VirtualAddress,
			vsize=o.Misc.VirtualSize
			>
		{
			//...
		} *o.Name @o.PointerToRawData;
	};

// Directory Entries
#define IMAGE_DIRECTORY_ENTRY_EXPORT          0   // Export Directory
#define IMAGE_DIRECTORY_ENTRY_IMPORT          1   // Import Directory
#define IMAGE_DIRECTORY_ENTRY_RESOURCE        2   // Resource Directory
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION       3   // Exception Directory
#define IMAGE_DIRECTORY_ENTRY_SECURITY        4   // Security Directory
#define IMAGE_DIRECTORY_ENTRY_BASERELOC       5   // Base Relocation Table
#define IMAGE_DIRECTORY_ENTRY_DEBUG           6   // Debug Directory
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE    7   // Architecture Specific Data
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR       8   // RVA of GP
#define IMAGE_DIRECTORY_ENTRY_TLS             9   // TLS Directory
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10   // Load Configuration Directory
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   11   // Bound Import Directory in headers
#define IMAGE_DIRECTORY_ENTRY_IAT            12   // Import Address Table
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13   // Delay Load Import Descriptors
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14   // COM Runtime descriptor

	ref ImageBase = ImageOptionalHeader.ImageBase;

	///////////////////////////////////////////////////////
	// IMPORT

	ref rImageDirectoryEntryImport = ImageOptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (rImageDirectoryEntryImport.Size)
	{
		portal (ImageBase + rImageDirectoryEntryImport.VirtualAddress)
		{
			alias aImportTableSize = rImageDirectoryEntryImport.Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);
			struct IMAGE_IMPORT_DESCRIPTOR
			{
				union {
					DWORD   Characteristics;
					DWORD   OriginalFirstThunk;
				};
				DWORD   TimeDateStamp;
				DWORD   ForwarderChain;
				DWORD   Name;
				DWORD   FirstThunk;
			} ImageImportDesc[aImportTableSize];

			struct IMAGE_IMPORT_BY_NAME
			{
				WORD    Hint;
				BYTE    Name[strlen(@) + 1];
			};

			for (aImportTableSize - 1)
			{
				ref ImportDesc = ImageImportDesc[$];
				portal (ImageBase + ImportDesc.OriginalFirstThunk)
				{
					DWORD OriginalThunk[];	//to be determined
					while (OriginalThunk[$] != 0)
					{
						if (!(OriginalThunk[$] & 0x80000000)) //bByOrdinal?
						{
							portal (ImageBase + OriginalThunk[$])
							{
								IMAGE_IMPORT_BY_NAME impbyname;
								portal (ImageBase + ImportDesc.FirstThunk)
								{
									DWORD impbyname.Name;
								}
							}
						}
						else
						{
							assert(0);
						}
					}
				}
				portal (ImageBase + ImportDesc.Name)
				{
					BYTE ImageImportName[strlen(@) + 1];
				}
			}
		}
	}

	///////////////////////////////////////////////////////
	// RESOURCE

	struct IMAGE_RESOURCE_DIRECTORY 
	{
		DWORD   Characteristics;
		DWORD   TimeDateStamp;
		WORD    MajorVersion;
		WORD    MinorVersion;
		WORD    NumberOfNamedEntries;
		WORD    NumberOfIdEntries;
	} ImageResourceDirectory;

	struct IMAGE_RESOURCE_DIRECTORY_ENTRY
	{
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

	struct IMAGE_RESOURCE_DATA_ENTRY
	{
		DWORD   OffsetToData;
		DWORD   Size;
		DWORD   CodePage;
		DWORD   Reserved;
	};

	struct IMAGE_RESOURCE_DIR_STRING_U
	{
		USHORT  Length;
		WCHAR   NameString[Length];
	};

	var ResourceBase = ImageBase + ImageOptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;

	function FUNCTION_RESOURCE_ENTRY(level, isNamedEntry)
	{
		IMAGE_RESOURCE_DIRECTORY_ENTRY irde;
			
		if (isNamedEntry)
		{
			scope (ResourceBase + irde.Id)
			{
				IMAGE_RESOURCE_DIR_STRING_U irdsu;
			}
		}
			
		if (irde.DataIsDirectory)
		{
			scope (ResourceBase + irde.OffsetToDirectory)
			{
				call FUNCTION_RESOURCE_DIRECTORY(level);
			}
		}
		else
		{
			scope (ResourceBase + irde.OffsetToData)
			{
				IMAGE_RESOURCE_DATA_ENTRY irde;
				portal (ImageBase + irde.OffsetToData)
				{
					BYTE data[irdata.Size];
				}
			}
		}
	}

	function FUNCTION_RESOURCE_DIRECTORY(level)
	{
		level++;
		IMAGE_RESOURCE_DIRECTORY ImageResourceDirectory;

		for (ImageResourceDirectory.NumberOfNamedEntries)
		{
			call FUNCTION_RESOURCE_ENTRY(level, true);
		}
		for (ImageResourceDirectory.NumberOfIdEntries)
		{
			call FUNCTION_RESOURCE_ENTRY(level, false);
		}
	};
	
	portal (ResourceBase)
	{
		call FUNCTION_RESOURCE_DIRECTORY(0);
	}

	ref ideb = ImageOptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT];
	if (ideb.Size)
	{
		scope (ideb.VirtualAddress)
		{
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
			for (ideb.Size / sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR) - 1)
			{
				IMAGE_BOUND_IMPORT_DESCRIPTOR ibid;
				IMAGE_BOUND_FORWARDER_REF[ibid.NumberOfModuleForwarderRefs];
				scope (ibid.OffsetModuleName)
				{
					BYTE moduleName[strlen(@) + 1];
				}
			}
		}
	}


	portal (ImageBase + ImageOptionalHeader.AddressOfEntryPoint)
	{
		CODE main;
	}

} MSEXE;