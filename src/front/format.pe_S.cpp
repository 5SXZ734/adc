#include "shared.h"
#include "format.pe.h"
#include "decode.pe.h"



void PE_Strucs_t::createStructures()
{
#if(1)
	if (mr.NewScope("__imp_ptr"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("pvt", mr.type(m64bit ? TYPEID_QWORD : TYPEID_DWORD));
	}
	if (mr.NewScope("__class_type_info"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("vptr", ptrOf(mr.type(TYPEID_VOID)));
		mr.declField("type_name", ptrOf(mr.type(TYPEID_VOID)));
	}

	if (mr.NewScope("__si_class_type_info"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("vptr", ptrOf(mr.type(TYPEID_VOID)));
		mr.declField("type_name", ptrOf(mr.type(TYPEID_VOID)));
		mr.declField("base_type", ptrOf(type(TYPEID_VOID)));
	}

	if (mr.NewScope("__pointer_type_info"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("vptr", ptrOf(mr.type(TYPEID_VOID)));
		mr.declField("type_name", ptrOf(mr.type(TYPEID_VOID)));
		mr.declBField("const_mask", type(TYPEID_DWORD));//0x1
		mr.declBField("volatile_mask", type(TYPEID_DWORD));//0x2
		mr.declBField("restrict_mask", type(TYPEID_DWORD));//0x4
		mr.declBField("incomplete_mask", type(TYPEID_DWORD));//0x8
		mr.declBField("incomplete_class_mask", type(TYPEID_DWORD));//0x10
	}

	if (mr.NewScope("__pointer_to_member_type_info"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("vptr", ptrOf(mr.type(TYPEID_VOID)));
		mr.declField("type_name", ptrOf(mr.type(TYPEID_VOID)));
		mr.declBField("const_mask", type(TYPEID_DWORD));//0x1
		mr.declBField("volatile_mask", type(TYPEID_DWORD));//0x2
		mr.declBField("restrict_mask", type(TYPEID_DWORD));//0x4
		mr.declBField("incomplete_mask", type(TYPEID_DWORD));//0x8
		mr.declBField("incomplete_class_mask", type(TYPEID_DWORD));//0x10
		mr.declField("context", ptrOf(mr.type(TYPEID_VOID)));
	}

	if (mr.NewScope("__base_class_type_info"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("base_type", ptrOf(type(TYPEID_VOID)));
		//mr.declField("offset_flags", mr.type(TYPEID_LONG));
		//mr.declBField("flags", arrayOf(enumOf(type("UNWIND_OP_CODES"), TYPEID_DWORD), 8));
		mr.declBField("virtual_mask", type(TYPEID_DWORD));
		mr.declBField("public_mask", type(TYPEID_DWORD));
		mr.skipBits(6);
		mr.declBField("offset", arrayOf(type(TYPEID_INT32), 24));
		mr.declField("unknown", mr.type(TYPEID_LONG));
	}

#endif

	if (mr.NewScope("IMAGE_DOS_HEADER"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("e_magic", type(TYPEID_WORD));
		mr.declField("e_cblp", type(TYPEID_WORD));
		mr.declField("e_cp", type(TYPEID_WORD));
		mr.declField("e_crlc", type(TYPEID_WORD));
		mr.declField("e_cparhdr", type(TYPEID_WORD));
		mr.declField("e_minalloc", type(TYPEID_WORD));
		mr.declField("e_maxalloc", type(TYPEID_WORD));
		mr.declField("e_ss", type(TYPEID_WORD));
		mr.declField("e_sp", type(TYPEID_WORD));
		mr.declField("e_csum", type(TYPEID_WORD));
		mr.declField("e_ip", type(TYPEID_WORD));
		mr.declField("e_cs", type(TYPEID_WORD));
		mr.declField("e_lfarlc", type(TYPEID_WORD));
		mr.declField("e_ovno", type(TYPEID_WORD));
		mr.declField("e_res", arrayOf(type(TYPEID_WORD), 4));
		mr.declField("e_oemid", type(TYPEID_WORD));
		mr.declField("e_oeminfo", type(TYPEID_WORD));
		mr.declField("e_res2", arrayOf(type(TYPEID_WORD), 10));
		mr.declField("e_lfanew", type(TYPEID_DWORD), ATTR_RVA);//LONG
	}

	if (mr.NewScope("IMAGE_FILE_MACHINE", SCOPE_ENUM))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("I386", 0x014c);
		mr.declEField("R3000", 0x0162);
		mr.declEField("R4000", 0x0166);
		mr.declEField("R10000", 0x0168);
		mr.declEField("WCEMIPSV2", 0x0169);
		mr.declEField("ALPHA", 0x0184);
		mr.declEField("SH3", 0x01a2);
		mr.declEField("SH3DSP", 0x01a3);
		mr.declEField("SH3E", 0x01a4);
		mr.declEField("SH4", 0x01a6);
		mr.declEField("SH5", 0x01a8);
		mr.declEField("ARM", 0x01c0);
		mr.declEField("THUMB", 0x01c2);
		mr.declEField("ARM2", 0x01c4);
		mr.declEField("AM33", 0x01d3);
		mr.declEField("POWERPC", 0x01F0);
		mr.declEField("POWERPCFP", 0x01f1);
		mr.declEField("IA64", 0x0200);
		mr.declEField("MIPS16", 0x0266);
		mr.declEField("ALPHA64", 0x0284);
		mr.declEField("MIPSFPU", 0x0366);
		mr.declEField("MIPSFPU16", 0x0466);
		mr.declEField("AXP64", 0x0284);
		mr.declEField("TRICORE", 0x0520);
		mr.declEField("CEF", 0x0CEF);
		mr.declEField("EBC", 0x0EBC);
		mr.declEField("AMD64", 0x8664);
		mr.declEField("M32R", 0x9041);
		mr.declEField("CEE", 0xC0EE);
	}

	if (mr.NewScope("IMAGE_SIGNATURE", SCOPE_ENUM))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("DOS", 0x5A4D);
		mr.declEField("OS2", 0x454E);
		mr.declEField("VXD", 0x454C);
		mr.declEField("NT", 0x00004550);
	}

	if (mr.NewScope("IMAGE_FILE_HEADER"))
	{
		SAFE_SCOPE_HERE(mr);
		//mr.installTypesMgr();
		mr.declField("Machine", enumOf(type("IMAGE_FILE_MACHINE"), TYPEID_WORD));
		mr.declField("NumberOfSections", type(TYPEID_WORD));
		mr.declField("TimeDateStamp", type(TYPEID_DWORD));
		mr.declField("PointerToSymbolTable", type(TYPEID_DWORD), ATTR_FILEPTR);
		mr.declField("NumberOfSymbols", type(TYPEID_DWORD));
		mr.declField("SizeOfOptionalHeader", type(TYPEID_WORD));
		//			mr.declField("Characteristics", type("CharacteristicsType"), ATTR_COLLAPSED);
		if (mr.NewScope(mr.declField("Characteristics", ATTR_COLLAPSED)))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declBField("RELOCS_STRIPPED", type(TYPEID_WORD));//0
			mr.declBField("EXECUTABLE_IMAGE", type(TYPEID_WORD));//1
			mr.declBField("LINE_NUMS_STRIPPED", type(TYPEID_WORD));//2
			mr.declBField("LOCAL_SYMS_STRIPPED", type(TYPEID_WORD));//3
			mr.declBField("AGGRESIVE_WS_TRIM", type(TYPEID_WORD));//4
			mr.declBField("LARGE_ADDRESS_AWARE", type(TYPEID_WORD));//5
			mr.skipBits(1);//6
			mr.declBField("BYTES_REVERSED_LO", type(TYPEID_WORD));//7
			mr.declBField("IS_32BIT_MACHINE", type(TYPEID_WORD));//8
			mr.declBField("DEBUG_STRIPPED", type(TYPEID_WORD));//9
			mr.declBField("REMOVABLE_RUN_FROM_SWAP", type(TYPEID_WORD));//10
			mr.declBField("NET_RUN_FROM_SWAP", type(TYPEID_WORD));//11
			mr.declBField("SYSTEM", type(TYPEID_WORD));//12
			mr.declBField("DLL", type(TYPEID_WORD));//13
			mr.declBField("UP_SYSTEM_ONLY", type(TYPEID_WORD));//14
			mr.declBField("BYTES_REVERSED_HI", type(TYPEID_WORD));//15
		}
	}

	if (mr.NewScope("IMAGE_DATA_DIRECTORY"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("VirtualAddress", type(TYPEID_DWORD), ATTR_RVA);
		mr.declField("Size", type(TYPEID_DWORD));
	}

	if (mr.NewScope("IMAGE_DIRECTORY_ENTRY", SCOPE_ENUM))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("EXPORT");
		mr.declEField("IMPORT");
		mr.declEField("RESOURCE");
		mr.declEField("EXCEPTION");
		mr.declEField("SECURITY");
		mr.declEField("BASERELOC");
		mr.declEField("DEBUG");
		mr.declEField("ARCHITECTURE");
		mr.declEField("GLOBALPTR");
		mr.declEField("TLS");
		mr.declEField("LOAD_CONFIG");
		mr.declEField("BOUND_IMPORT");
		mr.declEField("IAT");
		mr.declEField("DELAY_IMPORT");
		mr.declEField("COM_DESCRIPTOR");
	}

	if (mr.NewScope("IMAGE_OPTIONAL_HEADER"))
	{
		SAFE_SCOPE_HERE(mr);
		// Standard fields.
		mr.declField("Magic", type(TYPEID_WORD));
		mr.declField("MajorLinkerVersion", type(TYPEID_BYTE));
		mr.declField("MinorLinkerVersion", type(TYPEID_BYTE));
		mr.declField("SizeOfCode", type(TYPEID_DWORD));
		mr.declField("SizeOfInitializedData", type(TYPEID_DWORD));
		mr.declField("SizeOfUninitializedData", type(TYPEID_DWORD));
		mr.declField("AddressOfEntryPoint", type(TYPEID_DWORD), ATTR_RVA);
		mr.declField("BaseOfCode", type(TYPEID_DWORD), ATTR_RVA);
		if (!m64bit)
			mr.declField("BaseOfData", type(TYPEID_DWORD), ATTR_RVA);//missing in PE32+
		// NT additional fields.
		mr.declField("ImageBase", pointerType());
		mr.declField("SectionAlignment", type(TYPEID_DWORD));
		mr.declField("FileAlignment", type(TYPEID_DWORD));
		mr.declField("MajorOperatingSystemVersion", type(TYPEID_WORD));
		mr.declField("MinorOperatingSystemVersion", type(TYPEID_WORD));
		mr.declField("MajorImageVersion", type(TYPEID_WORD));
		mr.declField("MinorImageVersion", type(TYPEID_WORD));
		mr.declField("MajorSubsystemVersion", type(TYPEID_WORD));
		mr.declField("MinorSubsystemVersion", type(TYPEID_WORD));
		mr.declField("Win32VersionValue", type(TYPEID_DWORD));
		mr.declField("SizeOfImage", type(TYPEID_DWORD));
		mr.declField("SizeOfHeaders", type(TYPEID_DWORD));
		mr.declField("CheckSum", type(TYPEID_DWORD));
		mr.declField("Subsystem", type(TYPEID_WORD));
		mr.declField("DllCharacteristics", type(TYPEID_WORD));
		mr.declField("SizeOfStackReserve", pointerType());
		mr.declField("SizeOfStackCommit", pointerType());
		mr.declField("SizeOfHeapReserve", pointerType());
		mr.declField("SizeOfHeapCommit", pointerType());
		mr.declField("LoaderFlags", type(TYPEID_DWORD));
		mr.declField("NumberOfRvaAndSizes", type(TYPEID_DWORD));
		//mr.declField("DataDirectory", arrayOf(type("IMAGE_DATA_DIRECTORY"), 16));//IMAGE_NUMBEROF_DIRECTORY_ENTRIES
	}

	if (mr.NewScope("IMAGE_SECTION_HEADER"))
	{
		mr.installTypesMgr();
		if (mr.NewScope("IMAGE_SCN"))
		{
			mr.skipBits(5);//<0-4>
			mr.declBField("CNT_CODE", type(TYPEID_DWORD));//<5>
			mr.declBField("CNT_INITIALIZED_DATA", type(TYPEID_DWORD));//<6>
			mr.declBField("CNT_UNINITIALIZED_DATA", type(TYPEID_DWORD));//<7>
			mr.skipBits(1);//<8>
			mr.declBField("LNK_INFO", type(TYPEID_DWORD));//<9>
			mr.skipBits(1);//<10>
			mr.declBField("LNK_REMOVE", type(TYPEID_DWORD));//<11>
			mr.declBField("LNK_COMDAT", type(TYPEID_DWORD));//<12>
			mr.skipBits(2);//<13,14>
			mr.declBField("MEM_FARDATA", type(TYPEID_DWORD));//<15>
			mr.skipBits(1);//<16>
			mr.declBField("MEM_PURGEABLE", type(TYPEID_DWORD));//<17>
			mr.declBField("MEM_LOCKED", type(TYPEID_DWORD));//<18>
			mr.declBField("MEM_PRELOAD", type(TYPEID_DWORD));//<19>
			mr.declBField("ALIGN", arrayOf(type(TYPEID_DWORD), 4));//<20-23>
			mr.declBField("LNK_NRELOC_OVFL", type(TYPEID_DWORD));//<24>
			mr.declBField("MEM_DISCARDABLE", type(TYPEID_DWORD));//<25>
			mr.declBField("MEM_NOT_CACHED", type(TYPEID_DWORD));//<26>
			mr.declBField("MEM_NOT_PAGED", type(TYPEID_DWORD));//<27>
			mr.declBField("MEM_SHARED", type(TYPEID_DWORD));//<28>
			mr.declBField("MEM_EXECUTE", type(TYPEID_DWORD));//<29>
			mr.declBField("MEM_READ", type(TYPEID_DWORD));//<30>
			mr.declBField("MEM_WRITE", type(TYPEID_DWORD));//<31>
			mr.Leave();
		}
		mr.declField("Name", arrayOf(type(TYPEID_CHAR), 8));//IMAGE_SIZEOF_SHORT_NAME
		if (mr.NewScope(mr.declField("Misc")))//, SCOPE_ UNION)))
		{
			mr.installNamespace();
			mr.declUField("PhysicalAddress", type(TYPEID_DWORD));
			mr.declUField("VirtualSize", type(TYPEID_DWORD));
			mr.Leave();
		}
		//mr.declField("Misc", pUnion);
		mr.declField("VirtualAddress", type(TYPEID_DWORD), ATTR_RVA);
		mr.declField("SizeOfRawData", type(TYPEID_DWORD));
		mr.declField("PointerToRawData", type(TYPEID_DWORD), ATTR_FILEPTR);
		mr.declField("PointerToRelocations", type(TYPEID_DWORD));
		mr.declField("PointerToLinenumbers", type(TYPEID_DWORD));
		mr.declField("NumberOfRelocations", type(TYPEID_WORD));
		mr.declField("NumberOfLinenumbers", type(TYPEID_WORD));
		mr.declField("Characteristics", type("IMAGE_SCN"), ATTR_COLLAPSED);//TYPEID_DWORD
		mr.Leave();
	}

	if (mr.NewScope("IMAGE_EXPORT_DIRECTORY"))
	{
		mr.declField("Characteristics", type(TYPEID_DWORD));
		mr.declField("TimeDateStamp", type(TYPEID_DWORD));
		mr.declField("MajorVersion", type(TYPEID_WORD));
		mr.declField("MinorVersion", type(TYPEID_WORD));
		mr.declField("Name", type(TYPEID_DWORD), ATTR_RVA);
		mr.declField("Base", type(TYPEID_DWORD));
		mr.declField("NumberOfFunctions", type(TYPEID_DWORD));
		mr.declField("NumberOfNames", type(TYPEID_DWORD));
		mr.declField("AddressOfFunctions", type(TYPEID_DWORD), ATTR_RVA);
		mr.declField("AddressOfNames", type(TYPEID_DWORD), ATTR_RVA);
		mr.declField("AddressOfNameOrdinals", type(TYPEID_DWORD), ATTR_RVA);
		mr.Leave();
	}

	if (mr.NewScope("IMAGE_IMPORT_DESCRIPTOR"))
	{
		if (mr.NewScope(mr.declField()))//, SCOPE_ UNION))
		{//unnamed union
			mr.declUField("Characteristics", type(TYPEID_DWORD));
			mr.declUField("OriginalFirstThunk", type(TYPEID_DWORD), ATTR_RVA);
			mr.Leave();
		}
		//mr.declField(nullptr, pUnion);//unnamed field
		mr.declField("TimeDateStamp", type(TYPEID_DWORD));
		mr.declField("ForwarderChain", type(TYPEID_DWORD));
		mr.declField("Name", type(TYPEID_DWORD), ATTR_RVA);
		mr.declField("FirstThunk", type(TYPEID_DWORD), ATTR_RVA);
		mr.Leave();
	}

	/////////////////////////
	if (mr.NewScope("IMAGE_RESOURCE_DIRECTORY"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("Characteristics", type(TYPEID_DWORD));
		mr.declField("TimeDateStamp", type(TYPEID_DWORD));
		mr.declField("MajorVersion", type(TYPEID_WORD));
		mr.declField("MinorVersion", type(TYPEID_WORD));
		mr.declField("NumberOfNameEntries", type(TYPEID_WORD));
		mr.declField("NumberOfIDEntries", type(TYPEID_WORD));
	}

#if(1)
	if (mr.NewScope("IMAGE_RESOURCE_DIRECTORY_ENTRY"))
	{
		SAFE_SCOPE_HERE(mr);
		if (mr.NewScope(mr.declField()))//, SCOPE_UNION))
		{
			SAFE_SCOPE_HERE(mr);
			if (mr.NewScope(mr.declUField()))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declBField("NameOffset", arrayOf(type(TYPEID_DWORD), 31), (AttrIdEnum)ATTR_RES_OFFS);//<0>
				mr.declBField("NameIsString", arrayOf(type(TYPEID_DWORD), 1));//<31>
			}
			mr.declUField("Name", type(TYPEID_DWORD), (AttrIdEnum)ATTR_RES_OFFS);
			mr.declUField("Id", type(TYPEID_WORD));
		}
		if (mr.NewScope(mr.declField()))//, SCOPE_UNION))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declUField("OffsetToData", type(TYPEID_DWORD), (AttrIdEnum)ATTR_RES_OFFS);
			if (mr.NewScope(mr.declUField()))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declBField("OffsetToDirectory", arrayOf(type(TYPEID_DWORD), 31), (AttrIdEnum)ATTR_RES_OFFS);//<0>
				mr.declBField("DataIsDirectory", arrayOf(type(TYPEID_DWORD), 1));//<31>
			}
		}
	}

#else
	if (mr.NewScope("IMAGE_RESOURCE_DIRECTORY_ENTRY"))
	{
		SAFE_SCOPE_HERE(mr);
		if (mr.NewScope(mr.declField(), SCOPE_UNION))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declUField("NameRVA", type(TYPEID_DWORD), ATTR_RES_OFFS);
			mr.declUField("IntegerID", type(TYPEID_DWORD));
		}
		if (mr.NewScope(mr.declField(), SCOPE_UNION))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declUField("DataEntryRVA", type(TYPEID_DWORD), ATTR_RES_OFFS);
			mr.declUField("SubdirectoryRVA", type(TYPEID_DWORD), ATTR_RES_OFFS);
		}
	}
#endif

	if (mr.NewScope("IMAGE_RESOURCE_TYPE", SCOPE_ENUM))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("CURSOR", 1);
		mr.declEField("BITMAP", 2);
		mr.declEField("ICON", 3);
		mr.declEField("MENU", 4);
		mr.declEField("DIALOG", 5);
		mr.declEField("STRING", 6);
		mr.declEField("FONTDIR", 7);
		mr.declEField("FONT", 8);
		mr.declEField("ACCELERATOR", 9);
		mr.declEField("RCDATA", 10);
		mr.declEField("MESSAGETABLE", 11);
		mr.declEField("GROUP_CURSOR", 12);
		mr.declEField("GROUP_ICON", 14);
		mr.declEField("VERSION", 16);
		mr.declEField("DLGINCLUDE", 17);
		mr.declEField("PLUGPLAY", 19);
		mr.declEField("VXD", 20);
		mr.declEField("ANICURSOR", 21);
		mr.declEField("ANIICON", 22);
		mr.declEField("HTML", 23);
		mr.declEField("MANIFEST", 24);
	}

	if (mr.NewScope("IMAGE_RESOURCE_DIRECTORY_ROOT"))
	{
		SAFE_SCOPE_HERE(mr);
		if (mr.NewScope(mr.declField()))//nullptr, SCOPE_ UNION))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declUField("NameOffset", type(TYPEID_DWORD), (AttrIdEnum)ATTR_RES_OFFS);
			mr.declUField("ResourceType", enumOf(type("IMAGE_RESOURCE_TYPE"), TYPEID_DWORD));
		}
		//mr.declBField("TypeID", arrayOf(enumOf(type("IMAGE_RESOURCE_TYPE"), TYPEID_DWORD), 31));//<0>
		//mr.declBField("IsName", arrayOf(type(TYPEID_DWORD), 1));//<31>
		mr.declBField("SubdirectoryOffset", arrayOf(type(TYPEID_DWORD), 31), (AttrIdEnum)ATTR_RES_OFFS);//<32>
		mr.declBField("IsDirectory", arrayOf(type(TYPEID_DWORD), 1));//<63>
	}

	if (mr.NewScope("IMAGE_RESOURCE_DATA_ENTRY"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("OffsetToData", type(TYPEID_DWORD), ATTR_RVA);
		mr.declField("Size", type(TYPEID_DWORD));
		mr.declField("CodePage", type(TYPEID_DWORD));
		mr.declField("Reserved", type(TYPEID_DWORD));
	}

	if (mr.NewScope("ACCELTABLEENTRY"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("fFlags", type(TYPEID_WORD));
		mr.declField("wAnsi", type(TYPEID_WORD));
		mr.declField("wId", type(TYPEID_WORD));
		mr.declField("padding", type(TYPEID_WORD));
	}

	if (mr.NewScope("DLGTEMPLATE"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("style", type(TYPEID_DWORD));
		mr.declField("dwExtendedStyle", type(TYPEID_DWORD));
		mr.declField("cdit", type(TYPEID_WORD));
		mr.declField("x", type(TYPEID_SHORT));
		mr.declField("y", type(TYPEID_SHORT));
		mr.declField("cx", type(TYPEID_SHORT));
		mr.declField("cy", type(TYPEID_SHORT));
	}

	if (mr.NewScope("DLGITEMTEMPLATE"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("style", type(TYPEID_DWORD));
		mr.declField("dwExtendedStyle", type(TYPEID_DWORD));
		mr.declField("x", type(TYPEID_SHORT));
		mr.declField("y", type(TYPEID_SHORT));
		mr.declField("cx", type(TYPEID_SHORT));
		mr.declField("cy", type(TYPEID_SHORT));
		mr.declField("id", type(TYPEID_WORD));
	}

	if (mr.NewScope("MENUHEADER"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("wVersion", type(TYPEID_WORD));
		mr.declField("cbHeaderSize", type(TYPEID_WORD));
	}

	if (mr.NewScope("VS_FIXEDFILEINFO"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("dwSignature", type(TYPEID_DWORD));
		mr.declField("dwStrucVersion", type(TYPEID_DWORD));
		mr.declField("dwFileVersionMS", type(TYPEID_DWORD));
		mr.declField("dwFileVersionLS", type(TYPEID_DWORD));
		mr.declField("dwProductVersionMS", type(TYPEID_DWORD));
		mr.declField("dwProductVersionLS", type(TYPEID_DWORD));
		mr.declField("dwFileFlagsMask", type(TYPEID_DWORD));
		mr.declField("dwFileFlags", type(TYPEID_DWORD));
		mr.declField("dwFileOS", type(TYPEID_DWORD));
		mr.declField("dwFileType", type(TYPEID_DWORD));
		mr.declField("dwFileSubtype", type(TYPEID_DWORD));
		mr.declField("dwFileDateMS", type(TYPEID_DWORD));
		mr.declField("dwFileDateLS", type(TYPEID_DWORD));
	}

	if (mr.NewScope("IMAGE_BOUND_IMPORT_DESCRIPTOR"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("TimeDateStamp", type(TYPEID_DWORD));
		mr.declField("OffsetModuleName", type(TYPEID_WORD));
		mr.declField("NumberOfModuleForwarderRefs", type(TYPEID_WORD));
	}

	if (mr.NewScope("IMAGE_BOUND_FORWARDER_REF"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("TimeDateStamp", type(TYPEID_DWORD));
		mr.declField("OffsetModuleName", type(TYPEID_WORD));
		mr.declField("Reserved", type(TYPEID_WORD));
	}

	if (mr.NewScope("BITMAPINFOHEADER"))
	{
		SAFE_SCOPE_HERE(mr);
		//mr.addType(mpico, s, "BITMAPINFOHEADER");
		mr.declField("Size", type(TYPEID_DWORD));
		mr.declField("Width", type(TYPEID_DWORD));
		mr.declField("Height", type(TYPEID_DWORD));
		mr.declField("Planes", type(TYPEID_WORD));
		mr.declField("BitCount", type(TYPEID_WORD));
		mr.declField("Compression", type(TYPEID_DWORD));
		mr.declField("ImageSize", type(TYPEID_DWORD));
		mr.declField("XpixelsPerM", type(TYPEID_DWORD));
		mr.declField("YpixelsPerM", type(TYPEID_DWORD));
		mr.declField("ColorsUsed", type(TYPEID_DWORD));
		mr.declField("ColorsImportant", type(TYPEID_DWORD));
	}

	if (mr.NewScope("RGBQUAD"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("Blue", type(TYPEID_BYTE));
		mr.declField("Green", type(TYPEID_BYTE));
		mr.declField("Red", type(TYPEID_BYTE));
		mr.declField("Reserved", type(TYPEID_BYTE));
	}

	if (mr.NewScope("RGBTRIPLE"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("Blue", type(TYPEID_BYTE));
		mr.declField("Green", type(TYPEID_BYTE));
		mr.declField("Red", type(TYPEID_BYTE));
	}

	if (mr.NewScope("IMAGE_LOAD_CONFIG_DIRECTORY32"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("Size", type(TYPEID_DWORD));
		mr.declField("TimeDateStamp", type(TYPEID_DWORD));
		mr.declField("MajorVersion", type(TYPEID_WORD));
		mr.declField("MinorVersion", type(TYPEID_WORD));
		mr.declField("GlobalFlagsClear", type(TYPEID_DWORD));
		mr.declField("GlobalFlagsSet", type(TYPEID_DWORD));
		mr.declField("CriticalSectionDefaultTimeout", type(TYPEID_DWORD));
		mr.declField("DeCommitFreeBlockThreshold", type(TYPEID_DWORD));
		mr.declField("DeCommitTotalFreeThreshold", type(TYPEID_DWORD));
		mr.declField("LockPrefixTable", type(TYPEID_DWORD), ATTR_VA);
		mr.declField("MaximumAllocationSize", type(TYPEID_DWORD));
		mr.declField("VirtualMemoryThreshold", type(TYPEID_DWORD));
		mr.declField("ProcessHeapFlags", type(TYPEID_DWORD));
		mr.declField("ProcessAffinityMask", type(TYPEID_DWORD));
		mr.declField("CSDVersion", type(TYPEID_WORD));
		mr.declField("Reserved1", type(TYPEID_WORD));
		mr.declField("EditList", type(TYPEID_DWORD), ATTR_VA);
		mr.declField("SecurityCookie", type(TYPEID_DWORD), ATTR_VA);
		mr.declField("SEHandlerTable", type(TYPEID_DWORD), ATTR_VA);
		mr.declField("SEHandlerCount", type(TYPEID_DWORD));
		mr.declField("GuardCFCheckFunctionPointer", type(TYPEID_DWORD), ATTR_VA);
		mr.declField("Reserved2", type(TYPEID_DWORD));
		mr.declField("GuardCFFunctionTable", type(TYPEID_DWORD), ATTR_VA);
		mr.declField("GuardCFFunctionCount", type(TYPEID_DWORD));
		mr.declField("GuardFlags", type(TYPEID_DWORD));
	};

	if (mr.NewScope("IMAGE_LOAD_CONFIG_DIRECTORY64"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("Size", type(TYPEID_DWORD));
		mr.declField("TimeDateStamp", type(TYPEID_DWORD));
		mr.declField("MajorVersion", type(TYPEID_WORD));
		mr.declField("MinorVersion", type(TYPEID_WORD));
		mr.declField("GlobalFlagsClear", type(TYPEID_DWORD));
		mr.declField("GlobalFlagsSet", type(TYPEID_DWORD));
		mr.declField("CriticalSectionDefaultTimeout", type(TYPEID_DWORD));
		mr.declField("DeCommitFreeBlockThreshold", type(TYPEID_ULONGLONG));
		mr.declField("DeCommitTotalFreeThreshold", type(TYPEID_ULONGLONG));
		mr.declField("LockPrefixTable", type(TYPEID_ULONGLONG), ATTR_VA);
		mr.declField("MaximumAllocationSize", type(TYPEID_ULONGLONG));
		mr.declField("VirtualMemoryThreshold", type(TYPEID_ULONGLONG));
		mr.declField("ProcessAffinityMask", type(TYPEID_ULONGLONG));
		mr.declField("ProcessHeapFlags", type(TYPEID_DWORD));
		mr.declField("CSDVersion", type(TYPEID_WORD));
		mr.declField("Reserved1", type(TYPEID_WORD));
		mr.declField("EditList", type(TYPEID_ULONGLONG), ATTR_VA);                    // VA
		mr.declField("SecurityCookie", type(TYPEID_ULONGLONG), ATTR_VA);              // VA
		mr.declField("SEHandlerTable", type(TYPEID_ULONGLONG), ATTR_VA);              // VA
		mr.declField("SEHandlerCount", type(TYPEID_ULONGLONG), ATTR_OFFS);
		mr.declField("GuardCFCheckFunctionPointer", type(TYPEID_ULONGLONG), ATTR_VA); // VA
		mr.declField("Reserved2", type(TYPEID_ULONGLONG));
		mr.declField("GuardCFFunctionTable", type(TYPEID_ULONGLONG), ATTR_VA);        // VA
		mr.declField("GuardCFFunctionCount", type(TYPEID_ULONGLONG));
		mr.declField("GuardFlags", type(TYPEID_DWORD));
	};

	if (mr.NewScope("IMAGE_DEBUG_TYPE", SCOPE_ENUM))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("IMAGE_DEBUG_TYPE_UNKNOWN");// 0
		mr.declEField("IMAGE_DEBUG_TYPE_COFF");// 1
		mr.declEField("IMAGE_DEBUG_TYPE_CODEVIEW");// 2
		mr.declEField("IMAGE_DEBUG_TYPE_FPO");// 3
		mr.declEField("IMAGE_DEBUG_TYPE_MISC");// 4
		mr.declEField("IMAGE_DEBUG_TYPE_EXCEPTION");// 5
		mr.declEField("IMAGE_DEBUG_TYPE_FIXUP");// 6
		mr.declEField("IMAGE_DEBUG_TYPE_OMAP_TO_SRC");// 7
		mr.declEField("IMAGE_DEBUG_TYPE_OMAP_FROM_SRC");// 8
		mr.declEField("IMAGE_DEBUG_TYPE_BORLAND");// 9
		mr.declEField("IMAGE_DEBUG_TYPE_RESERVED10");// 10
		mr.declEField("IMAGE_DEBUG_TYPE_CLSID");// 11
	}

	if (mr.NewScope("IMAGE_DEBUG_DIRECTORY"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("Characteristics", type(TYPEID_ULONG));
		mr.declField("TimeDateStamp", type(TYPEID_ULONG));
		mr.declField("MajorVersion", type(TYPEID_USHORT));
		mr.declField("MinorVersion", type(TYPEID_USHORT));
		mr.declField("Type", enumOf(type("IMAGE_DEBUG_TYPE"), TYPEID_ULONG));
		mr.declField("SizeOfData", type(TYPEID_ULONG));
		mr.declField("AddressOfRawData", type(TYPEID_ULONG), ATTR_RVA);
		mr.declField("PointerToRawData", type(TYPEID_ULONG), ATTR_FILEPTR);
	}

	if (mr.NewScope("IMAGE_TLS_DIRECTORY32"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("StartAddressOfRawData", type(TYPEID_DWORD), ATTR_VA);
		mr.declField("EndAddressOfRawData", type(TYPEID_DWORD), ATTR_VA);
		mr.declField("AddressOfIndex", type(TYPEID_DWORD), ATTR_VA);
		mr.declField("AddressOfCallBacks", type(TYPEID_DWORD), ATTR_VA);
		mr.declField("SizeOfZeroFill", type(TYPEID_DWORD));
		mr.declField("Characteristics", type(TYPEID_DWORD));
	};

	if (mr.NewScope("IMAGE_TLS_DIRECTORY64"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("StartAddressOfRawData", type(TYPEID_ULONGLONG), ATTR_VA);
		mr.declField("EndAddressOfRawData", type(TYPEID_ULONGLONG), ATTR_VA);
		mr.declField("AddressOfIndex", type(TYPEID_ULONGLONG), ATTR_VA);
		mr.declField("AddressOfCallBacks", type(TYPEID_ULONGLONG), ATTR_VA);
		mr.declField("SizeOfZeroFill", type(TYPEID_DWORD));
		mr.declField("Characteristics", type(TYPEID_DWORD));
	};

	if (mr.NewScope("WIN_CERTIFICATE_TYPE", SCOPE_ENUM))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("X509", 1);
		mr.declEField("PKCS_SIGNED_DATA", 2);
		mr.declEField("RESERVED_1", 3);
		mr.declEField("PKCS1_SIGN", 9);
	};

	if (mr.NewScope("RUNTIME_FUNCTION"))
	{
		mr.declField("BeginAddress", type(TYPEID_DWORD), ATTR_RVA);
		mr.declField("EndAddress", type(TYPEID_DWORD), ATTR_RVA);
		mr.declField("UnwindData", type(TYPEID_DWORD), ATTR_RVA);
		mr.Leave();
	}
	if (mr.NewScope("UNWIND_OP_CODES", SCOPE_ENUM))
	{
		mr.declEField("UWOP_PUSH_NONVOL");//0
		mr.declEField("UWOP_ALLOC_LARGE");//1
		mr.declEField("UWOP_ALLOC_SMALL");//2
		mr.declEField("UWOP_SET_FPREG");//3
		mr.declEField("UWOP_SAVE_NONVOL");//4 
		mr.declEField("UWOP_SAVE_NONVOL_FAR");//5
		mr.declEField("UWOP_SAVE_XMM128");//6
		mr.declEField("UWOP_SAVE_XMM128_FAR");//7
		mr.declEField("UWOP_PUSH_MACHFRAME");//8
		mr.Leave();
	}
	if (mr.NewScope("UNW_FLAG", SCOPE_ENUM))
	{
		mr.declEField("UNW_FLAG_EHANDLER", 1);
		mr.declEField("UNW_FLAG_UHANDLER", 2);
		mr.declEField("UNW_FLAG_CHAININFO", 4);
		mr.Leave();
	}
	if (mr.NewScope("UNWIND_CODE"))
	{
		mr.declField("OffsetInProlog", type(TYPEID_BYTE));
		mr.declBField("OpCode", arrayOf(enumOf(type("UNWIND_OP_CODES"), TYPEID_BYTE), 4));
		mr.declBField("OpInfo", arrayOf(type(TYPEID_BYTE), 4));
		mr.Leave();
	}

	if (mr.NewScope("IMAGE_SYMBOL_TYPE", SCOPE_ENUM))
	{
		mr.declEField("NULL");// 0 No type information or unknown base type. Microsoft tools use this setting.
		mr.declEField("VOID");// 1 No valid type; used with void pointers and functions.
		mr.declEField("CHAR");// 2 Character (signed byte).
		mr.declEField("SHORT");// 3 Two-byte signed integer.
		mr.declEField("INT");// 4 Natural integer type (normally four bytes in Windows NT).
		mr.declEField("LONG");// 5 Four-byte signed integer.
		mr.declEField("FLOAT");// 6 Four-byte floating-point number.
		mr.declEField("DOUBLE");// 7 Eight-byte floating-point number.
		mr.declEField("STRUCT");// 8 Structure.
		mr.declEField("UNION");// 9 Union.
		mr.declEField("ENUM");// 10 Enumerated type.
		mr.declEField("MOE");// 11 Member of enumeration (a specific value).
		mr.declEField("BYTE");// 12 Byte; unsigned one-byte integer.
		mr.declEField("WORD");// 13 Word; unsigned two-byte integer.
		mr.declEField("UINT");// 14 Unsigned integer of natural size (normally, four bytes).
		mr.declEField("DWORD");// 15 Unsigned four-byte integer.
		mr.declEField("FUNCTION", 0x20);//M$-specific
		mr.Leave();
	}

	if (mr.NewScope("IMAGE_SYMBOL_TYPE_HI", SCOPE_ENUM))
	{
		mr.declEField("DTYPE_NULL");
		mr.declEField("DTYPE_POINTER");		//1
		mr.declEField("DTYPE_FUNCTION");		//2
		mr.declEField("DTYPE_ARRAY");		//3
		mr.Leave();
	};

	if (mr.NewScope("IMAGE_SYMBOL_SECTION", SCOPE_ENUM))
	{
		mr.declEField("UNDEFINED");
		mr.declEField("ABSOLUTE", 0xFFFF);//-1
		mr.declEField("DEBUG", 0xFFFE);//-2
		mr.Leave();
	};

	if (mr.NewScope("IMAGE_SYMBOL_CLASS", SCOPE_ENUM))
	{
		mr.declEField("END_OF_FUNCTION", -1);
		mr.declEField("NULL", 0);
		mr.declEField("AUTOMATIC", 0x01);
		mr.declEField("EXTERNAL", 0x02);
		mr.declEField("STATIC", 0x03);
		mr.declEField("REGISTER", 0x04);
		mr.declEField("EXTERNAL_DEF", 0x05);
		mr.declEField("LABEL", 0x06);
		mr.declEField("UNDEFINED_LABEL", 0x07);
		mr.declEField("MEMBER_OF_STRUCT", 0x08);
		mr.declEField("ARGUMENT", 0x09);
		mr.declEField("STRUCT_TAG", 0x0A);
		mr.declEField("MEMBER_OF_UNION", 0x0B);
		mr.declEField("UNION_TAG", 0x0C);
		mr.declEField("TYPE_DEFINITION", 0x0D);
		mr.declEField("UNDEFINED_STATIC", 0x0E);
		mr.declEField("ENUM_TAG", 0x0F);
		mr.declEField("MEMBER_OF_ENUM", 0x10);
		mr.declEField("REGISTER_PARAM", 0x11);
		mr.declEField("BIT_FIELD", 0x12);
		mr.declEField("FAR_EXTERNAL", 0x44);
		mr.declEField("BLOCK", 0x64);
		mr.declEField("FUNCTION", 0x65);
		mr.declEField("END_OF_STRUCT", 0x66);
		mr.declEField("FILE", 0x67);
		mr.declEField("SECTION", 0x68);
		mr.declEField("WEAK_EXTERNAL", 0x69);
		mr.declEField("CLR_TOKEN", 0x6B);
		mr.Leave();
	};

	if (mr.NewScope("IMAGE_SYMBOL_ENTRY"))
	{
		if (mr.NewScope(mr.declField("Union")))//nullptr, SCOPE_ UNION)))
		{
			mr.declUField("Name", arrayOf(type(TYPEID_CHAR), 8));
			if (mr.NewScope(mr.declUField()))
			{
				mr.declField("Zeroes", type(TYPEID_DWORD));
				mr.declField("Offset", type(TYPEID_DWORD), (AttrIdEnum)ATTR_COFF_STRING_TABLE_REF);
				mr.Leave();
			}
			mr.Leave();
		}
		mr.declField("Value", type(TYPEID_DWORD));
		mr.declField("SectionNumber", type(TYPEID_SHORT));
		mr.declField("TypeLo", enumOf(type("IMAGE_SYMBOL_TYPE"), TYPEID_BYTE));
		mr.declField("TypeHi", type(TYPEID_BYTE));
		mr.declField("StorageClass", enumOf(type("IMAGE_SYMBOL_CLASS"), TYPEID_BYTE));
		mr.declField("NumberOfAuxSymbols", type(TYPEID_BYTE));
		mr.Leave();
	}

	if (mr.NewScope("COFF_SymbolEntryAux1"))
	{
		mr.declField("TagIndex", type(TYPEID_DWORD));
		mr.declField("TotalSize", type(TYPEID_DWORD));
		mr.declField("PointerToLinenumber", type(TYPEID_DWORD));
		mr.declField("PointerToNextFunction", type(TYPEID_DWORD));
		mr.declField("Unused", type(TYPEID_USHORT));
		mr.Leave();
	}

	if (mr.NewScope("COFF_SymbolEntryAux4"))
	{
		mr.declField("FileName", mr.arrayOf(mr.type(TYPEID_CHAR), 18));
		mr.Leave();
	}

	if (mr.NewScope("COFF_SymbolEntryAux5"))
	{
		mr.declField("Length", type(TYPEID_DWORD));
		mr.declField("NumberOfRelocations", type(TYPEID_USHORT));
		mr.declField("NumberOfLinenumbers", type(TYPEID_USHORT));
		mr.declField("CheckSum", type(TYPEID_DWORD));
		mr.declField("Number", type(TYPEID_USHORT));
		mr.declField("Selection", type(TYPEID_BYTE));
		mr.declField("Unused", mr.arrayOf(mr.type(TYPEID_BYTE), 3));//padding
		mr.Leave();
	}

	////////////////////// RTTI (VC)

	if (mr.NewScope("_PMD"))
	{
		mr.declField("mdisp", type(TYPEID_INT));
		mr.declField("pdisp", type(TYPEID_INT));
		mr.declField("vdisp", type(TYPEID_INT));
		mr.Leave();
	};

	/*if (mr.NewScope("_TypeDescriptor"))
	{
	mr.declField("pVFTable", pointerType());//const
	mr.declField("spare", pointerType());
	mr.declField("name", arrayOf(type(TYPEID_CHAR), 0));
	mr.Leave();
	}*/

	mr.DeclareContextDependentType(_PFX("__vmi_class_type_info"));
	mr.DeclareContextDependentType(_PFX("_TypeDescriptor"));

	if (mr.NewScope("__RTTIBaseClassDescriptor"))
	{
		if (!m64bit)
			mr.declField("pTypeDescriptor", ptrOf(type(_PFX("_TypeDescriptor"))));
		else
			mr.declField("rvaTypeDescriptor", type(TYPEID_DWORD), ATTR_RVA);
		mr.declField("numContainedBases", type(TYPEID_ULONG));
		mr.declField("where", type("_PMD"));
		mr.declField("attributes", type(TYPEID_ULONG));
#if(1)//not found in older versions
		if (!m64bit)
			mr.declField("pClassHierarchyDescriptor", ptrOf(type(TYPEID_VOID)));// ptrOf(type("__RTTIClassHierarchyDescriptor")));
		else
			mr.declField("rvaClassHierarchyDescriptor", type(TYPEID_DWORD), ATTR_RVA);
#endif
		mr.Leave();
	}
	if (mr.NewScope("__RTTIBaseClassArray"))
	{
		if (!m64bit)
			mr.declField("arrayOfBaseClassDescriptors", arrayOf(ptrOf(type("__RTTIBaseClassDescriptor")), 0));
		else
			mr.declField("rvaArrayOfBaseClassDescriptors", arrayOf(type(TYPEID_DWORD), 0), ATTR_RVA);
		//__RTTIBaseClassDescriptor *arrayOfBaseClassDescriptors [];
		mr.Leave();
	}
	if (mr.NewScope("__RTTIClassHierarchyDescriptor"))
	{
		mr.declField("signature", type(TYPEID_ULONG));
		mr.declField("attributes", type(TYPEID_ULONG));
		mr.declField("numBaseClasses", type(TYPEID_ULONG));
		//mr.declField("pBaseClassArray", ptrOf(arrayOf(ptrOf(type("__RTTIBaseClassDescriptor")), -1)));
		if (!m64bit)
			mr.declField("pBaseClassArray", ptrOf(type("__RTTIBaseClassArray")));
		else
			mr.declField("rvaBaseClassArray", type(TYPEID_DWORD), ATTR_RVA);
		mr.Leave();
	}
	if (mr.NewScope("__RTTICompleteObjectLocator"))
	{
		mr.declField("signature", type(TYPEID_ULONG));
		mr.declField("offset", type(TYPEID_ULONG));
		mr.declField("cdOffset", type(TYPEID_ULONG));
		if (!m64bit)
		{
			mr.declField("pTypeDescriptor", ptrOf(type(_PFX("_TypeDescriptor"))));
			mr.declField("pClassDescriptor", ptrOf(type("__RTTIClassHierarchyDescriptor")));
		}
		else
		{
			mr.declField("rvaTypeDescriptor", type(TYPEID_DWORD), ATTR_RVA);
			mr.declField("rvaClassDescriptor", type(TYPEID_DWORD), ATTR_RVA);
			mr.declField("rvaSelf", type(TYPEID_DWORD), ATTR_RVA);
		}
		mr.Leave();
	}

	if (mr.NewScope("CV_HEADER"))
	{
		mr.declField("Signature", type(TYPEID_DWORD));
		mr.declField("Offset", type(TYPEID_DWORD));
		mr.Leave();
	};

	if (mr.NewScope("MSVC_MANGLE_AC", SCOPE_ENUM))//Member objects access codes
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("private", '0');
		mr.declEField("protected", '1');
		mr.declEField("public", '2');
	}

	if (mr.NewScope("MSVC_MANGLE_MFMC", SCOPE_ENUM))//Member function modifier codes
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("private", 'A');
		mr.declEField("private|far", 'B');
		mr.declEField("private|static", 'C');
		mr.declEField("private|static far", 'D');
		mr.declEField("private|virtual", 'E');
		mr.declEField("private|virtual far", 'F');

		mr.declEField("protected", 'I');
		mr.declEField("protected|far", 'J');
		mr.declEField("protected|static", 'K');
		mr.declEField("protected|static far", 'L');
		mr.declEField("protected|virtual", 'M');
		mr.declEField("protected|virtual far", 'N');

		mr.declEField("public", 'Q');
		mr.declEField("public|far", 'R');
		mr.declEField("public|static", 'S');
		mr.declEField("public|static far", 'T');
		mr.declEField("public|virtual", 'U');
		mr.declEField("public|virtual far", 'V');
	}

	if (mr.NewScope("MSVC_MANGLE_CV", SCOPE_ENUM))// Member function access codes (storage for `this' target)
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("default", 'A');
		mr.declEField("const", 'B');
		mr.declEField("volatile", 'C');
		mr.declEField("const volatile");
	}

	if (mr.NewScope("MSVC_MANGLE_SCC", SCOPE_ENUM))// Storage class codes
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("near", 'A');
		mr.declEField("const", 'B');
		mr.declEField("volatile", 'C');
		mr.declEField("const volatile", 'D');
		mr.declEField("far", 'E');
		mr.declEField("const far", 'F');
		mr.declEField("volatile far", 'G');
		mr.declEField("const volatile far", 'H');
		mr.declEField("huge", 'I');
		//case 'F': //__unaligned F
		//case 'I': //__restrict I
	}

	if (mr.NewScope("MSVC_MANGLE_CC", SCOPE_ENUM))//Function calling convention codes
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("__cdecl", 'A');
		mr.declEField("__pascal", 'C');
		//mr.declEField("__fortran", 'C');
		mr.declEField("__thiscall", 'E');
		mr.declEField("__stdcall", 'G');
		mr.declEField("__fastcall", 'I');
		//mr.declEField("__msfastcall", '');
		//mr.declEField("__regcall", 'E');
		mr.declEField("__vectorcall", 'Q');
		//mr.declEField("interrupt", 'A');
	}

	if (mr.NewScope("MSVC_MANGLE_TC", SCOPE_ENUM))//Type codes
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("reference(&)", 'A');
		mr.declEField("signed char", 'C');
		mr.declEField("char", 'D');
		mr.declEField("unsigned char", 'E');
		mr.declEField("short int", 'F');
		mr.declEField("unsigned short int", 'G');
		//mr.declEField("wchar_t", 'G');
		mr.declEField("int", 'H');
		mr.declEField("unsigned int", 'I');
		mr.declEField("long int", 'J');
		mr.declEField("unsigned long int", 'K');
		mr.declEField("float", 'M');
		mr.declEField("double", 'N');
		mr.declEField("long double", 'O');//_T, _Z
		mr.declEField("pointer(*)", 'P');
		mr.declEField("const pointer", 'Q');
		mr.declEField("union", 'T');
		mr.declEField("struct", 'U');
		mr.declEField("class", 'V');
		mr.declEField("enum", 'W');
		mr.declEField("void", 'X');
	}

	if (mr.NewScope("MSVC_MANGLE_TC_", SCOPE_ENUM))//Type codes (underscored)
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("bool", 'N');
		mr.declEField("__int64", 'J');//long long
		mr.declEField("unsigned __int64", 'K');//unsigned long long
		mr.declEField("wchar_t", 'W');
		mr.declEField("long double", 'T');
		mr.declEField("long double", 'Z');
	}
#if(0)
//__complex__ float 
//__complex__ double
__m64 T__m64@@
__m128 T__m128@@
__m128d U__m128d@@
__m128i T__m128i@@
__m256 T__m256@@
__m256d U__m256d@@
__m256i T__m256i@@
__m512 T__m512@@
__m512d U__m512d@@
__m512i T__m512i@@
varargs ... Z
const X X
X * PEAX
const X * PEBX
volatile X * PECX
const volatile X * PEDX
X * const QEAX
X * volatile REAX
X * const volatile SEAX
const X * const QEBX
X * __restrict PEIAX
X & AEAX
//X && 
const X & AEBX
volatile X & AECX
const volatile X & AEDX
X[ ] (as global object) PAX
X[][8] (as global object) PAY07X
X[][16][5] (as glob obj) PAY1BA@4X
X[ ] (as function parameter) QEAX
const X[] (as function param.) QEBX
X[][8] (as func-tion parameter) QEAY07X


X[][16][5] (as function param.) QEAY1BA@4X
X near * PAX
X far * PEX
X huge * PIX
//X _seg * 
X near & AAX
X far & AEX
X huge & AIX
union X TX@@
struct X UX@@
class X VX@@
enum X W4X@@
enum Y::X W4X@Y@@
X (*Y)(W)10 P6AXW@Z
X Y::*V PEQY@@X
X (Y::*V)(W) P8Y@@EAEXW@Z
#endif

	if (mr.NewScope("MSVC_MANGLE_ONC", SCOPE_ENUM))//Operator name codes
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("constructor X", '0');
		mr.declEField("destructor ~X", '1');
		mr.declEField("operator []", 'A');
		mr.declEField("operator ()", 'R');
		mr.declEField("operator ->", 'C');
		mr.declEField("operator ++X, X++", 'E');
		mr.declEField("operator --X, X--", 'F');
		mr.declEField("operator new", '2');
		mr.declEField("operator delete", '3');
		mr.declEField("operator *X", 'D');
		mr.declEField("operator &X", 'I');
		mr.declEField("operator +X", 'H');
		mr.declEField("operator -X", 'G');
		mr.declEField("operator !", '7');
		mr.declEField("operator ~", 'S');
		mr.declEField("operator ->*", 'J');
		mr.declEField("operator X * Y", 'D');
		mr.declEField("operator /", 'K');
		mr.declEField("operator %", 'L');
		mr.declEField("operator X + Y", 'H');
		mr.declEField("operator X - Y", 'G');
		mr.declEField("operator <<", '6');
		mr.declEField("operator >>", '5');
		mr.declEField("operator <", 'M');
		mr.declEField("operator >", 'O');
		mr.declEField("operator <=", 'N');
		mr.declEField("operator >=", 'P');
		mr.declEField("operator ==", '8');
		mr.declEField("operator !=", '9');
		mr.declEField("operator X & Y", 'I');
		mr.declEField("operator |", 'U');
		mr.declEField("operator ^", 'T');
		mr.declEField("operator &&", 'V');
		mr.declEField("operator ||", 'W');
		mr.declEField("operator =", '4');
		mr.declEField("operator *=", 'X');
		mr.declEField("operator +=", 'Y');
		mr.declEField("operator -=", 'Z');
		mr.declEField("operator ,", 'Q');
		mr.declEField("operator TYPE()", 'B');
	}

	if (mr.NewScope("MSVC_MANGLE_ONC_", SCOPE_ENUM))//Operator name codes (underscore-prefixed)
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("operator new[]", 'U');
		mr.declEField("operator delete[]", 'V');
		mr.declEField("operator /=", '0');
		mr.declEField("operator %=", '1');
		mr.declEField("operator <<=", '3');
		mr.declEField("operator >>=", '2');
		mr.declEField("operator &=", '4');
		mr.declEField("operator |=", '5');
		mr.declEField("operator ^=", '6');
		mr.declEField("virtual table", '7');
	}

	/*if (mr.NewScope("__flags_masks", SCOPE_ENUM))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("__non_diamond_repeat_mask", 0x1);
		mr.declEField("__diamond_shaped_mask", 0x2);
	};*/

}

