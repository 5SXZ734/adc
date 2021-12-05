#include "shared.h"
#include "format.pdb.h"
#include "PDB.h"

using namespace pdb;

static void PDB_CreateStructures_0(I_Module& mr)//common for PDB2.0 and PDB7.0
{
	if (mr.NewScope("CV_SIGNATURE", SCOPE_ENUM))
	{
		mr.declEField("CV_SIGNATURE_C6");		//0L
		mr.declEField("CV_SIGNATURE_C7");		//1L
		mr.declEField("CV_SIGNATURE_C11");		//2L
		mr.declEField("CV_SIGNATURE_C13");		//4L
		mr.declEField("CV_SIGNATURE_RESERVED");	//5L
		mr.Leave();
	}

	if (mr.NewScope("PdbStreamVersion", SCOPE_ENUM))
	{
		mr.declEField("VC2", 19941610);
		mr.declEField("VC4", 19950623);
		mr.declEField("VC41", 19950814);
		mr.declEField("VC50", 19960307);
		mr.declEField("VC98", 19970604);
		mr.declEField("VC70Dep", 19990604);
		mr.declEField("VC70", 20000404);
		mr.declEField("VC80", 20030901);
		mr.declEField("VC110", 20091201);
		mr.declEField("VC140", 20140508);
		mr.Leave();
	}

	if (mr.NewScope("DbiStreamHeader"))
	{
		mr.declField("VersionSignature", mr.type(TYPEID_INT32), ATTR_DECIMAL);
		mr.declField("VersionHeader", mr.type(TYPEID_UINT32), ATTR_DECIMAL);
		mr.declField("Age", mr.type(TYPEID_UINT32));
		mr.declField("GlobalStreamIndex", mr.type(TYPEID_UINT16), (AttrIdEnum)ATTR_STREAM_INDEX);
		mr.declField("BuildNumber", mr.type(TYPEID_UINT16));
		mr.declField("PublicStreamIndex", mr.type(TYPEID_UINT16), (AttrIdEnum)ATTR_STREAM_INDEX);
		mr.declField("PdbDllVersion", mr.type(TYPEID_UINT16));
		mr.declField("SymRecordStream", mr.type(TYPEID_UINT16), (AttrIdEnum)ATTR_STREAM_INDEX);
		mr.declField("PdbDllRbld", mr.type(TYPEID_UINT16));
		mr.declField("ModInfoSize", mr.type(TYPEID_INT32));
		mr.declField("SectionContributionSize", mr.type(TYPEID_INT32));
		mr.declField("SectionMapSize", mr.type(TYPEID_INT32));
		mr.declField("SourceInfoSize", mr.type(TYPEID_INT32));
		mr.declField("TypeServerSize", mr.type(TYPEID_INT32));
		mr.declField("MFCTypeServerIndex", mr.type(TYPEID_UINT32));
		mr.declField("OptionalDbgHeaderSize", mr.type(TYPEID_INT32));
		mr.declField("ECSubstreamSize", mr.type(TYPEID_INT32));
		mr.declField("Flags", mr.type(TYPEID_UINT16));
		mr.declField("Machine", mr.type(TYPEID_UINT16));
		mr.declField("Padding", mr.type(TYPEID_UINT32));
		mr.Leave();
	};

	if (mr.NewScope("SectionContribEntry"))
	{
		mr.declField("Section", mr.type(TYPEID_UINT16));
		mr.align(ALIGN_DWORD);//mr.declField("Padding1", mr.type(TYPEID_UINT16));
		mr.declField("Offset", mr.type(TYPEID_INT32));
		mr.declField("Size", mr.type(TYPEID_INT32));
		mr.declField("Characteristics", mr.type(TYPEID_UINT32));
		mr.declField("ModuleIndex", mr.type(TYPEID_UINT16));//, (AttrIdEnum)ATTR_MODULE_INDEX);
		mr.align(ALIGN_DWORD);//mr.declField("Padding2", mr.type(TYPEID_UINT16));
		mr.declField("DataCrc", mr.type(TYPEID_UINT32));
		mr.declField("RelocCrc", mr.type(TYPEID_UINT32));
		mr.Leave();
	};

	if (mr.NewScope("ModInfo"))//not used
	{
		mr.declField("Unused1", mr.type(TYPEID_UINT32));
		mr.declField("SectionContr", mr.type("SectionContribEntry"));
		mr.declField("Flags", mr.type(TYPEID_UINT16));
		mr.declField("ModuleSymStream", mr.type(TYPEID_UINT16));
		mr.declField("SymByteSize", mr.type(TYPEID_UINT32));
		mr.declField("C11ByteSize", mr.type(TYPEID_UINT32));
		mr.declField("C13ByteSize", mr.type(TYPEID_UINT32));
		mr.declField("SourceFileCount", mr.type(TYPEID_UINT16));
		mr.align(ALIGN_DWORD);//char Padding[2];
		mr.declField("Unused2", mr.type(TYPEID_UINT32));
		mr.declField("SourceFileNameIndex", mr.type(TYPEID_UINT32));
		mr.declField("PdbFilePathNameIndex", mr.type(TYPEID_UINT32));
		mr.declField("ModuleName", mr.arrayOf(mr.type(TYPEID_CHAR), 1));//[];
		//char ObjFileName[];
		mr.Leave();
	};

	if (mr.NewScope("SectionContrSubstreamVersion", SCOPE_ENUM))
	{
		mr.declEField("Ver60", 0xeffe0000 + 19970605);
		mr.declEField("V2", 0xeffe0000 + 20140516);
		mr.Leave();
	};

	if (mr.NewScope("SectionContribEntry"))
	{
		mr.declField("Section", mr.type(TYPEID_UINT16));
		mr.align(ALIGN_DWORD);//mr.declField("Padding1", mr.type(TYPEID_UINT16));
		mr.declField("Offset", mr.type(TYPEID_INT32));
		mr.declField("Size", mr.type(TYPEID_INT32));
		mr.declField("Characteristics", mr.type(TYPEID_UINT32));
		mr.declField("ModuleIndex", mr.type(TYPEID_UINT16));//, (AttrIdEnum)ATTR_MODULE_INDEX);
		mr.align(ALIGN_DWORD);//mr.declField("Padding2", mr.type(TYPEID_UINT16));
		mr.declField("DataCrc", mr.type(TYPEID_UINT32));
		mr.declField("RelocCrc", mr.type(TYPEID_UINT32));
		mr.Leave();
	};

	if (mr.NewScope("SectionMapHeader"))
	{
		mr.declField("Count", mr.type(TYPEID_UINT16));    // Number of segment descriptors
		mr.declField("LogCount", mr.type(TYPEID_UINT16)); // Number of logical segment descriptors
		mr.Leave();
	};

	if (mr.NewScope("SectionMapEntryFlags", SCOPE_ENUM))
	{
		mr.declBField("Read", mr.type(TYPEID_UINT16));//0					// Segment is readable.
		mr.declBField("Write", mr.type(TYPEID_UINT16));//1				// Segment is writable.
		mr.declBField("Execute", mr.type(TYPEID_UINT16));//2				// Segment is executable.
		mr.declBField("AddressIs32Bit", mr.type(TYPEID_UINT16));//3		// Descriptor describes a 32-bit linear address.
		mr.skipBits(4);//4:4
		mr.declBField("IsSelector", mr.type(TYPEID_UINT16));//8			// Frame represents a selector.
		mr.declBField("IsAbsoluteAddress", mr.type(TYPEID_UINT16));//9	// Frame represents an absolute address.
		mr.declBField("IsGroup", mr.type(TYPEID_UINT16));//10				// If set, descriptor represents a group.
		mr.Leave();
	};

	if (mr.NewScope("SectionMapEntry"))
	{
		mr.declField("Flags", mr.type(TYPEID_UINT16));         // See the SectionMapEntryFlags enum below.
		mr.declField("Ovl", mr.type(TYPEID_UINT16));           // Logical overlay number
		mr.declField("Group", mr.type(TYPEID_UINT16));         // Group index into descriptor array.
		mr.declField("Frame", mr.type(TYPEID_UINT16));
		mr.declField("SectionName", mr.type(TYPEID_UINT16));   // Byte index of segment / group name in string table, or 0xFFFF.
		mr.declField("ClassName", mr.type(TYPEID_UINT16));     // Byte index of class in string table, or 0xFFFF.
		mr.declField("Offset", mr.type(TYPEID_UINT32));        // Byte offset of the logical segment within physical segment.  If group is set in flags, this is the offset of the group.
		mr.declField("SectionLength", mr.type(TYPEID_UINT32)); // Byte count of the segment or group.
		mr.Leave();
	};

	if (mr.NewScope("DBIDebugHeader"))
	{
		mr.declField("FPO", mr.type(TYPEID_UINT16));
		mr.declField("exception", mr.type(TYPEID_UINT16));
		mr.declField("fixup", mr.type(TYPEID_UINT16));
		mr.declField("omapToSource", mr.type(TYPEID_UINT16));
		mr.declField("omapFromSource", mr.type(TYPEID_UINT16));
		mr.declField("sectionHdr", mr.type(TYPEID_UINT16));
		mr.declField("tokenRidMap", mr.type(TYPEID_UINT16));
		mr.declField("XData", mr.type(TYPEID_UINT16));
		mr.declField("PData", mr.type(TYPEID_UINT16));
		mr.declField("newFPO", mr.type(TYPEID_UINT16));
		mr.declField("sectionHdrOriginal", mr.type(TYPEID_UINT16));
		mr.Leave();
	}

	if (mr.NewScope("DBGTYPE", SCOPE_ENUM))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("dbgtypeFPO");
		mr.declEField("dbgtypeException");
		mr.declEField("dbgtypeFixup");
		mr.declEField("dbgtypeOmapToSrc");
		mr.declEField("dbgtypeOmapFromSrc");
		mr.declEField("dbgtypeSectionHdr");
		mr.declEField("dbgtypeTokenRidMap");
		mr.declEField("dbgtypeXdata");
		mr.declEField("dbgtypePdata");
		mr.declEField("dbgtypeNewFPO");
		mr.declEField("dbgtypeSectionHdrOrig");
	}

	if (mr.NewScope("CV_pubsymflag_t"))
	{
		mr.declBField("fCode", mr.type(TYPEID_ULONG));		// set if public symbol refers to a code address
		mr.declBField("fFunction", mr.type(TYPEID_ULONG));	// set if public symbol is a function
		mr.declBField("fManaged", mr.type(TYPEID_ULONG));		// set if managed code (native or IL)
		mr.declBField("fMSIL", mr.type(TYPEID_ULONG));		// set if managed IL code
		mr.declBField("__unused", mr.arrayOf(mr.type(TYPEID_ULONG), 28));// must be zero
		mr.Leave();
	}

}

static void PDB20_CreateStructures_0(I_Module& mr)
{
	PDB_CreateStructures_0(mr);

	if (mr.NewScope("PDB_STREAM"))
	{
		mr.declField("StreamSize", mr.type(TYPEID_UINT));
		mr.declField("StreamPages", mr.type(TYPEID_UINT));
		mr.Leave();
	}
	if (mr.NewScope("PDB_HEADER"))
	{
		mr.declField("Signature", mr.arrayOf(mr.type(TYPEID_CHAR), 44));
		mr.declField("PageSize", mr.type(TYPEID_UINT));
		mr.declField("StartPage", mr.type(TYPEID_USHORT));
		mr.declField("TotalPages", mr.type(TYPEID_USHORT));
		mr.declField("RootStream", mr.type("PDB_STREAM"));
		//uint16_t     mr.declField("awRootPages[1]; // pages containing PDB_ROOT
		mr.Leave();
	};


}

static void PDB70_CreateStructures_0(I_Module &mr)
{
	PDB_CreateStructures_0(mr);

	if (mr.NewScope("SuperBlock"))//pdb70
	{
		mr.declField("FileMagic", mr.arrayOf(mr.type(TYPEID_CHAR), 0x20));
		mr.declField("BlockSize", mr.type(TYPEID_ULITTLE32));
		mr.declField("FreeBlockMapBlock", mr.type(TYPEID_ULITTLE32));
		mr.declField("NumBlocks", mr.type(TYPEID_ULITTLE32));
		mr.declField("NumDirectoryBytes", mr.type(TYPEID_ULITTLE32));
		mr.declField("Unknown", mr.type(TYPEID_ULITTLE32));
		mr.declField("BlockMapAddr", mr.type(TYPEID_ULITTLE32), (AttrIdEnum)ATTR_PDB_BLK);
		mr.Leave();
	}
	if (mr.NewScope("PdbStreamHeader"))
	{
		mr.declField("Version", mr.enumOf(mr.type("PdbStreamVersion"), TYPEID_ULITTLE32));
		mr.declField("Signature", mr.type(TYPEID_ULITTLE32));
		mr.declField("Age", mr.type(TYPEID_ULITTLE32));
		mr.declField("UniqueId", mr.arrayOf(mr.type(TYPEID_UINT8), 16));
		mr.Leave();
	};
}

static void PDB_CreateStructures_MOD(I_Module &mr)
{
	if (mr.NewScope("SYM_ENUM_e", SCOPE_ENUM))
	{
		mr.declEField("S_COMPILE", 0x0001);// Compile flags symbol
		mr.declEField("S_REGISTER_16t", 0x0002);// Register variable
		mr.declEField("S_CONSTANT_16t", 0x0003);// constant symbol
		mr.declEField("S_UDT_16t", 0x0004);// User defined type
		mr.declEField("S_SSEARCH", 0x0005);// Start Search
		mr.declEField("S_END", 0x0006);// Block, procedure, "with" or thunk end
		mr.declEField("S_SKIP", 0x0007);// Reserve symbol space in $$Symbols table
		mr.declEField("S_CVRESERVE", 0x0008);// Reserved symbol for CV internal use
		mr.declEField("S_OBJNAME_ST", 0x0009);// path to object file name
		mr.declEField("S_ENDARG", 0x000a);// end of argument/return list
		mr.declEField("S_COBOLUDT_16t", 0x000b);// special UDT for cobol that does not symbol pack
		mr.declEField("S_MANYREG_16t", 0x000c);// multiple register variable
		mr.declEField("S_RETURN", 0x000d);// return description symbol
		mr.declEField("S_ENTRYTHIS", 0x000e);// description of this pointer on entry

		mr.declEField("S_BPREL16", 0x0100);// BP-relative
		mr.declEField("S_LDATA16", 0x0101);// Module-local symbol
		mr.declEField("S_GDATA16", 0x0102);// Global data symbol
		mr.declEField("S_PUB16", 0x0103);// a public symbol
		mr.declEField("S_LPROC16", 0x0104);// Local procedure start
		mr.declEField("S_GPROC16", 0x0105);// Global procedure start
		mr.declEField("S_THUNK16", 0x0106);// Thunk Start
		mr.declEField("S_BLOCK16", 0x0107);// block start
		mr.declEField("S_WITH16", 0x0108);// with start
		mr.declEField("S_LABEL16", 0x0109);// code label
		mr.declEField("S_CEXMODEL16", 0x010a);// change execution model
		mr.declEField("S_VFTABLE16", 0x010b);// address of virtual function table
		mr.declEField("S_REGREL16", 0x010c);// register relative address

		mr.declEField("S_BPREL32_16t", 0x0200);// BP-relative
		mr.declEField("S_LDATA32_16t", 0x0201);// Module-local symbol
		mr.declEField("S_GDATA32_16t", 0x0202);// Global data symbol
		mr.declEField("S_PUB32_16t", 0x0203);// a public symbol (CV internal reserved)
		mr.declEField("S_LPROC32_16t", 0x0204);// Local procedure start
		mr.declEField("S_GPROC32_16t", 0x0205);// Global procedure start
		mr.declEField("S_THUNK32_ST", 0x0206);// Thunk Start
		mr.declEField("S_BLOCK32_ST", 0x0207);// block start
		mr.declEField("S_WITH32_ST", 0x0208);// with start
		mr.declEField("S_LABEL32_ST", 0x0209);// code label
		mr.declEField("S_CEXMODEL32", 0x020a);// change execution model
		mr.declEField("S_VFTABLE32_16t", 0x020b);// address of virtual function table
		mr.declEField("S_REGREL32_16t", 0x020c);// register relative address
		mr.declEField("S_LTHREAD32_16t", 0x020d);// local thread storage
		mr.declEField("S_GTHREAD32_16t", 0x020e);// global thread storage
		mr.declEField("S_SLINK32", 0x020f);// static link for MIPS EH implementation

		mr.declEField("S_LPROCMIPS_16t", 0x0300);// Local procedure start
		mr.declEField("S_GPROCMIPS_16t", 0x0301);// Global procedure start

		// if these ref symbols have names following then the names are in ST format
		mr.declEField("S_PROCREF_ST", 0x0400);// Reference to a procedure
		mr.declEField("S_DATAREF_ST", 0x0401);// Reference to data
		mr.declEField("S_ALIGN", 0x0402);// Used for page alignment of symbols

		mr.declEField("S_LPROCREF_ST", 0x0403);// Local Reference to a procedure
		mr.declEField("S_OEM", 0x0404);// OEM defined symbol

		// sym records with 32-bit types embedded instead of 16-bit
		// all have 0x1000 bit set for easy identification
		// only do the 32-bit target versions since we don't really
		// care about 16-bit ones anymore.
		mr.declEField("S_TI16_MAX", 0x1000);

		mr.declEField("S_REGISTER_ST", 0x1001);// Register variable
		mr.declEField("S_CONSTANT_ST", 0x1002);// constant symbol
		mr.declEField("S_UDT_ST", 0x1003);// User defined type
		mr.declEField("S_COBOLUDT_ST", 0x1004);// special UDT for cobol that does not symbol pack
		mr.declEField("S_MANYREG_ST", 0x1005);// multiple register variable
		mr.declEField("S_BPREL32_ST", 0x1006);// BP-relative
		mr.declEField("S_LDATA32_ST", 0x1007);// Module-local symbol
		mr.declEField("S_GDATA32_ST", 0x1008);// Global data symbol
		mr.declEField("S_PUB32_ST", 0x1009);// a public symbol (CV internal reserved)
		mr.declEField("S_LPROC32_ST", 0x100a);// Local procedure start
		mr.declEField("S_GPROC32_ST", 0x100b);// Global procedure start
		mr.declEField("S_VFTABLE32", 0x100c);// address of virtual function table
		mr.declEField("S_REGREL32_ST", 0x100d);// register relative address
		mr.declEField("S_LTHREAD32_ST", 0x100e);// local thread storage
		mr.declEField("S_GTHREAD32_ST", 0x100f);// global thread storage

		mr.declEField("S_LPROCMIP_ST", 0x1010);// Local procedure start
		mr.declEField("S_GPROCMIP_ST", 0x1011);// Global procedure start

		mr.declEField("S_FRAMEPROC", 0x1012);// extra frame and proc information
		mr.declEField("S_COMPILE2_ST", 0x1013);// extended compile flags and info

		// new symbols necessary for 16-bit enumerates of IA64 registers
		// and IA64 specific symbols

		mr.declEField("S_MANYREG2_ST", 0x1014);// multiple register variable
		mr.declEField("S_LPROCIA64_ST", 0x1015);// Local procedure start (IA64)
		mr.declEField("S_GPROCIA64_ST", 0x1016);// Global procedure start (IA64)

		// Local symbols for IL
		mr.declEField("S_LOCALSLOT_ST", 0x1017);// local IL sym with field for local slot index
		mr.declEField("S_PARAMSLOT_ST", 0x1018);// local IL sym with field for parameter slot index

		mr.declEField("S_ANNOTATION", 0x1019);// Annotation string literals

		// symbols to support managed code debugging
		mr.declEField("S_GMANPROC_ST", 0x101a);// Global proc
		mr.declEField("S_LMANPROC_ST", 0x101b);// Local proc
		mr.declEField("S_RESERVED1", 0x101c);// reserved
		mr.declEField("S_RESERVED2", 0x101d);// reserved
		mr.declEField("S_RESERVED3", 0x101e);// reserved
		mr.declEField("S_RESERVED4", 0x101f);// reserved
		mr.declEField("S_LMANDATA_ST", 0x1020);
		mr.declEField("S_GMANDATA_ST", 0x1021);
		mr.declEField("S_MANFRAMEREL_ST", 0x1022);
		mr.declEField("S_MANREGISTER_ST", 0x1023);
		mr.declEField("S_MANSLOT_ST", 0x1024);
		mr.declEField("S_MANMANYREG_ST", 0x1025);
		mr.declEField("S_MANREGREL_ST", 0x1026);
		mr.declEField("S_MANMANYREG2_ST", 0x1027);
		mr.declEField("S_MANTYPREF", 0x1028);// Index for type referenced by name from metadata
		mr.declEField("S_UNAMESPACE_ST", 0x1029);// Using namespace

		// Symbols w/ SZ name fields. All name fields contain utf8 encoded strings.
		mr.declEField("S_ST_MAX", 0x1100);// starting point for SZ name symbols

		mr.declEField("S_OBJNAME", 0x1101);// path to object file name
		mr.declEField("S_THUNK32", 0x1102);// Thunk Start
		mr.declEField("S_BLOCK32", 0x1103);// block start
		mr.declEField("S_WITH32", 0x1104);// with start
		mr.declEField("S_LABEL32", 0x1105);// code label
		mr.declEField("S_REGISTER", 0x1106);// Register variable
		mr.declEField("S_CONSTANT", 0x1107);// constant symbol
		mr.declEField("S_UDT", 0x1108);// User defined type
		mr.declEField("S_COBOLUDT", 0x1109);// special UDT for cobol that does not symbol pack
		mr.declEField("S_MANYREG", 0x110a);// multiple register variable
		mr.declEField("S_BPREL32", 0x110b);// BP-relative
		mr.declEField("S_LDATA32", 0x110c);// Module-local symbol
		mr.declEField("S_GDATA32", 0x110d);// Global data symbol
		mr.declEField("S_PUB32", 0x110e);// a public symbol (CV internal reserved)
		mr.declEField("S_LPROC32", 0x110f);// Local procedure start
		mr.declEField("S_GPROC32", 0x1110);// Global procedure start
		mr.declEField("S_REGREL32", 0x1111);// register relative address
		mr.declEField("S_LTHREAD32", 0x1112);// local thread storage
		mr.declEField("S_GTHREAD32", 0x1113);// global thread storage

		mr.declEField("S_LPROCMIPS", 0x1114);// Local procedure start
		mr.declEField("S_GPROCMIPS", 0x1115);// Global procedure start
		mr.declEField("S_COMPILE2", 0x1116);// extended compile flags and info
		mr.declEField("S_MANYREG2", 0x1117);// multiple register variable
		mr.declEField("S_LPROCIA64", 0x1118);// Local procedure start (IA64)
		mr.declEField("S_GPROCIA64", 0x1119);// Global procedure start (IA64)
		mr.declEField("S_LOCALSLOT", 0x111a);// local IL sym with field for local slot index
		//mr.declEField("S_SLOT", S_LOCALSLOT);// alias for LOCALSLOT
		mr.declEField("S_PARAMSLOT", 0x111b);// local IL sym with field for parameter slot index

		// symbols to support managed code debugging
		mr.declEField("S_LMANDATA", 0x111c);
		mr.declEField("S_GMANDATA", 0x111d);
		mr.declEField("S_MANFRAMEREL", 0x111e);
		mr.declEField("S_MANREGISTER", 0x111f);
		mr.declEField("S_MANSLOT", 0x1120);
		mr.declEField("S_MANMANYREG", 0x1121);
		mr.declEField("S_MANREGREL", 0x1122);
		mr.declEField("S_MANMANYREG2", 0x1123);
		mr.declEField("S_UNAMESPACE", 0x1124);// Using namespace

		// ref symbols with name fields
		mr.declEField("S_PROCREF", 0x1125);// Reference to a procedure
		mr.declEField("S_DATAREF", 0x1126);// Reference to data
		mr.declEField("S_LPROCREF", 0x1127);// Local Reference to a procedure
		mr.declEField("S_ANNOTATIONREF", 0x1128);// Reference to an S_ANNOTATION symbol
		mr.declEField("S_TOKENREF", 0x1129);// Reference to one of the many MANPROCSYM's

		// continuation of managed symbols
		mr.declEField("S_GMANPROC", 0x112a);// Global proc
		mr.declEField("S_LMANPROC", 0x112b);// Local proc

		// short, light-weight thunks
		mr.declEField("S_TRAMPOLINE", 0x112c);// trampoline thunks
		mr.declEField("S_MANCONSTANT", 0x112d);// constants with metadata type info

		// native attributed local/parms
		mr.declEField("S_ATTR_FRAMEREL", 0x112e);// relative to virtual frame ptr
		mr.declEField("S_ATTR_REGISTER", 0x112f);// stored in a register
		mr.declEField("S_ATTR_REGREL", 0x1130);// relative to register (alternate frame ptr)
		mr.declEField("S_ATTR_MANYREG", 0x1131);// stored in >1 register

		// Separated code (from the compiler) support
		mr.declEField("S_SEPCODE", 0x1132);

		mr.declEField("S_LOCAL_2005", 0x1133);// defines a local symbol in optimized code
		mr.declEField("S_DEFRANGE_2005", 0x1134);// defines a single range of addresses in which symbol can be evaluated
		mr.declEField("S_DEFRANGE2_2005", 0x1135);// defines ranges of addresses in which symbol can be evaluated

		mr.declEField("S_SECTION", 0x1136);// A COFF section in a PE executable
		mr.declEField("S_COFFGROUP", 0x1137);// A COFF group
		mr.declEField("S_EXPORT", 0x1138);// A export

		mr.declEField("S_CALLSITEINFO", 0x1139);// Indirect call site information
		mr.declEField("S_FRAMECOOKIE", 0x113a);// Security cookie information

		mr.declEField("S_DISCARDED", 0x113b);// Discarded by LINK /OPT:REF (experimental, see richards)

		mr.declEField("S_COMPILE3", 0x113c);// Replacement for S_COMPILE2
		mr.declEField("S_ENVBLOCK", 0x113d);// Environment block split off from S_COMPILE2

		mr.declEField("S_LOCAL", 0x113e);// defines a local symbol in optimized code
		mr.declEField("S_DEFRANGE", 0x113f);// defines a single range of addresses in which symbol can be evaluated
		mr.declEField("S_DEFRANGE_SUBFIELD", 0x1140);         // ranges for a subfield

		mr.declEField("S_DEFRANGE_REGISTER", 0x1141);         // ranges for en-registered symbol
		mr.declEField("S_DEFRANGE_FRAMEPOINTER_REL", 0x1142); // range for stack symbol.
		mr.declEField("S_DEFRANGE_SUBFIELD_REGISTER", 0x1143);// ranges for en-registered field of symbol
		mr.declEField("S_DEFRANGE_FRAMEPOINTER_REL_FULL_SCOPE", 0x1144);// range for stack symbol span valid full scope of function body, gap might apply.
		mr.declEField("S_DEFRANGE_REGISTER_REL", 0x1145);// range for symbol address as register + offset.

		// S_PROC symbols that reference ID instead of type
		mr.declEField("S_LPROC32_ID", 0x1146);
		mr.declEField("S_GPROC32_ID", 0x1147);
		mr.declEField("S_LPROCMIPS_ID", 0x1148);
		mr.declEField("S_GPROCMIPS_ID", 0x1149);
		mr.declEField("S_LPROCIA64_ID", 0x114a);
		mr.declEField("S_GPROCIA64_ID", 0x114b);

		mr.declEField("S_BUILDINFO", 0x114c);// build information.
		mr.declEField("S_INLINESITE", 0x114d);// inlined function callsite.
		mr.declEField("S_INLINESITE_END", 0x114e);
		mr.declEField("S_PROC_ID_END", 0x114f);

		mr.declEField("S_DEFRANGE_HLSL", 0x1150);
		mr.declEField("S_GDATA_HLSL", 0x1151);
		mr.declEField("S_LDATA_HLSL", 0x1152);

		mr.declEField("S_FILESTATIC", 0x1153);

		//#if defined(CC_DP_CXX) && CC_DP_CXX

		mr.declEField("S_LOCAL_DPC_GROUPSHARED", 0x1154);// DPC groupshared variable
		mr.declEField("S_LPROC32_DPC", 0x1155);// DPC local procedure start
		mr.declEField("S_LPROC32_DPC_ID", 0x1156);
		mr.declEField("S_DEFRANGE_DPC_PTR_TAG", 0x1157);// DPC pointer tag definition range
		mr.declEField("S_DPC_SYM_TAG_MAP", 0x1158);// DPC pointer tag value to symbol record map

		//#endif // CC_DP_CXX

		mr.declEField("S_ARMSWITCHTABLE", 0x1159);
		mr.declEField("S_CALLEES", 0x115a);
		mr.declEField("S_CALLERS", 0x115b);
		mr.declEField("S_POGODATA", 0x115c);
		mr.declEField("S_INLINESITE2", 0x115d);    // extended inline site information

		mr.declEField("S_HEAPALLOCSITE", 0x115e);  // heap allocation site

		mr.declEField("S_MOD_TYPEREF", 0x115f);    // only generated at link time

		mr.declEField("S_REF_MINIPDB", 0x1160);    // only generated at link time for mini PDB
		mr.declEField("S_PDBMAP", 0x1161);    // only generated at link time for mini PDB

		mr.declEField("S_GDATA_HLSL32", 0x1162);
		mr.declEField("S_LDATA_HLSL32", 0x1163);

		mr.declEField("S_GDATA_HLSL32_EX", 0x1164);
		mr.declEField("S_LDATA_HLSL32_EX", 0x1165);

		mr.declEField("S_FASTLINK", 0x1167); // Undocumented
		mr.declEField("S_INLINEES", 0x1168); // Undocumented

		//mr.declEField("S_RECTYPE_MAX);             // one greater than last
		//mr.declEField("S_RECTYPE_LAST", S_RECTYPE_MAX - 1,
		//mr.declEField("S_RECTYPE_PAD", S_RECTYPE_MAX + 0x100 // Used *only* to verify symbol record types so that current PDB code can potentially read
		// future PDBs (assuming no format change, etc).

		mr.Leave();
	}

	if (mr.NewScope("CV_PROCFLAGS"))
	{
		mr.declBField("CV_PFLAG_NOFPO", mr.type(TYPEID_UINT8)); // frame pointer present
		mr.declBField("CV_PFLAG_INT", mr.type(TYPEID_UINT8)); // interrupt return
		mr.declBField("CV_PFLAG_FAR", mr.type(TYPEID_UINT8)); // far return
		mr.declBField("CV_PFLAG_NEVER", mr.type(TYPEID_UINT8)); // function does not return
		mr.declBField("CV_PFLAG_NOTREACHED", mr.type(TYPEID_UINT8)); // label isn't fallen into
		mr.declBField("CV_PFLAG_CUST_CALL", mr.type(TYPEID_UINT8)); // custom calling convention
		mr.declBField("CV_PFLAG_NOINLINE", mr.type(TYPEID_UINT8)); // function marked as noinline
		mr.declBField("CV_PFLAG_OPTDBGINFO", mr.type(TYPEID_UINT8)); // function has debug information for optimized code
		mr.Leave();
	};

	if (mr.NewScope("DEBUG_S_SUBSECTION_TYPE", SCOPE_ENUM))
	{
		mr.declEField("DEBUG_S_SYMBOLS", 0xf1);
		mr.declEField("DEBUG_S_LINES");
		mr.declEField("DEBUG_S_STRINGTABLE");
		mr.declEField("DEBUG_S_FILECHKSMS");
		mr.declEField("DEBUG_S_FRAMEDATA");
		mr.declEField("DEBUG_S_INLINEELINES");
		mr.declEField("DEBUG_S_CROSSSCOPEIMPORTS");
		mr.declEField("DEBUG_S_CROSSSCOPEEXPORTS");
		mr.declEField("DEBUG_S_IL_LINES");
		mr.declEField("DEBUG_S_FUNC_MDTOKEN_MAP");
		mr.declEField("DEBUG_S_TYPE_MDTOKEN_MAP");
		mr.declEField("DEBUG_S_MERGED_ASSEMBLYINPUT");
		mr.declEField("DEBUG_S_COFF_SYMBOL_RVA");
		//mr.declEField("DEBUG_S_IGNORE" 0x80000000");    // if this bit is set in a subsection type then ignore the subsection contents
		mr.Leave();
	};

	if (mr.NewScope("CV_Line_t"))
	{
		mr.declField("offset", mr.type(TYPEID_ULONG));             // Offset to start of code bytes for line number
		mr.declBField("linenumStart", mr.arrayOf(mr.type(TYPEID_ULONG), 24), ATTR_DECIMAL);    // line where statement/expression starts
		mr.declBField("deltaLineEnd", mr.arrayOf(mr.type(TYPEID_ULONG), 7));     // delta to line where statement ends (optional)
		mr.declBField("fStatement", mr.type(TYPEID_ULONG));       // true if a statement linenumber, else an expression line num
		mr.Leave();
	};

	/*		if (mr.NewScope("SubsectionHeader"))
			{
			mr.declField("sig", mr.enumOf(mr.type("DEBUG_S_SUBSECTION_TYPE"), TYPEID_INT32));
			mr.declField("size", mr.type(TYPEID_INT32));
			mr.Leave();
			}*/

	/*		if (mr.NewScope("CVFileChecksum"))
			{
			mr.declField("name", mr.type(TYPEID_UINT32));           // Index of name in name table.
			mr.declField("len", mr.type(TYPEID_UINT8));            // Hash length
			mr.declField("type", mr.type(TYPEID_UINT8));           // Hash type
			mr.Leave();
			};*/

	if (mr.NewScope("CV_cookietype_e", SCOPE_ENUM))
	{
		mr.declEField("CV_COOKIETYPE_COPY");
		mr.declEField("CV_COOKIETYPE_XOR_SP");
		mr.declEField("CV_COOKIETYPE_XOR_BP");
		mr.declEField("CV_COOKIETYPE_XOR_R13");
		mr.Leave();
	};

	if (mr.NewScope("CV_LVARFLAGS"))// local variable flags
	{
		mr.declBField("fIsParam", mr.type(TYPEID_USHORT)); // variable is a parameter
		mr.declBField("fAddrTaken", mr.type(TYPEID_USHORT)); // address is taken
		mr.declBField("fCompGenx", mr.type(TYPEID_USHORT)); // variable is compiler generated
		mr.declBField("fIsAggregate", mr.type(TYPEID_USHORT)); // the symbol is splitted in temporaries, which are treated by compiler as independent entities
		mr.declBField("fIsAggregated", mr.type(TYPEID_USHORT)); // Counterpart of fIsAggregate - tells that it is a part of a fIsAggregate symbol
		mr.declBField("fIsAliased", mr.type(TYPEID_USHORT)); // variable has multiple simultaneous lifetimes
		mr.declBField("fIsAlias", mr.type(TYPEID_USHORT)); // represents one of the multiple simultaneous lifetimes
		mr.declBField("fIsRetValue", mr.type(TYPEID_USHORT)); // represents a function return value
		mr.declBField("fIsOptimizedOut", mr.type(TYPEID_USHORT)); // variable has no lifetimes
		mr.declBField("fIsEnregGlob", mr.type(TYPEID_USHORT)); // variable is an enregistered global
		mr.declBField("fIsEnregStat", mr.type(TYPEID_USHORT)); // variable is an enregistered static
		mr.declBField("unused", mr.arrayOf(mr.type(TYPEID_USHORT), 5)); // must be zero
		mr.Leave();
	};

	if (mr.NewScope("CV_RANGEATTR"))
	{
		mr.declBField("maybe", mr.type(TYPEID_USHORT));	// May have no user name on one of control flow path.
		mr.declBField("padding", mr.arrayOf(mr.type(TYPEID_USHORT), 15));	// Padding for future use.
		mr.Leave();
	}

	if (mr.NewScope("CV_LVAR_ADDR_RANGE"))// defines a range of addresses
	{
		mr.declField("offStart", mr.type(TYPEID_CV_UOFF32));
		mr.declField("isectStart", mr.type(TYPEID_USHORT));
		mr.declField("cbRange", mr.type(TYPEID_USHORT));
		mr.Leave();
	};

	if (mr.NewScope("CV_LVAR_ADDR_GAP"))
	{
		mr.declField("gapStartOffset", mr.type(TYPEID_USHORT));	// relative offset from the beginning of the live range.
		mr.declField("cbRange", mr.type(TYPEID_USHORT));		// length of this gap.
		mr.Leave();
	};
}

static void PDB_CreateStructures_TPI(I_Module &mr)
{
	if (mr.NewScope("OffCb"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("off", mr.type(TYPEID_INT32));
		mr.declField("cb", mr.type(TYPEID_INT32));
	};

	if (mr.NewScope("TypeInfoHeader"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("version", mr.type(TYPEID_UINT32), ATTR_DECIMAL);
		mr.declField("headerSize", mr.type(TYPEID_INT32));
		mr.declField("min", mr.type(TYPEID_UINT32), (AttrIdEnum)ATTR_PDB_TYPEOFF);
		mr.declField("max", mr.type(TYPEID_UINT32), (AttrIdEnum)ATTR_PDB_TYPEOFF);
		mr.declField("followSize", mr.type(TYPEID_UINT32));

		mr.declField("sn", mr.type(TYPEID_INT16));
		mr.declField("padding", mr.type(TYPEID_INT16));
		mr.declField("hashKey", mr.type(TYPEID_INT32));
		mr.declField("buckets", mr.type(TYPEID_INT32));
		mr.declField("hashVals", mr.type("OffCb"));
		mr.declField("tiOff", mr.type("OffCb"));
		mr.declField("hashAdjust", mr.type("OffCb"));
	}
	if (mr.NewScope("CV_lvar_attr"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("off", mr.type(TYPEID_CV_UOFF32));	// first code address where var is live
		mr.declField("seg", mr.type(TYPEID_USHORT));
		mr.declField("flags", mr.type("CV_LVARFLAGS"));      // local var flags
	}
	if (mr.NewScope("TYPE_ENUM_e", SCOPE_ENUM))// Special Types
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("T_NOTYPE", 0x0000);   // uncharacterized type (no type)
		mr.declEField("T_ABS", 0x0001);   // absolute symbol
		mr.declEField("T_SEGMENT", 0x0002);   // segment type
		mr.declEField("T_VOID", 0x0003);   // void
		mr.declEField("T_HRESULT", 0x0008);   // OLE/COM HRESULT
		mr.declEField("T_32PHRESULT", 0x0408);   // OLE/COM HRESULT __ptr32 *
		mr.declEField("T_64PHRESULT", 0x0608);   // OLE/COM HRESULT __ptr64 *
		//
		mr.declEField("T_PVOID", 0x0103);   // near pointer to void
		mr.declEField("T_PFVOID", 0x0203);   // far pointer to void
		mr.declEField("T_PHVOID", 0x0303);   // huge pointer to void
		mr.declEField("T_32PVOID", 0x0403);   // 32 bit pointer to void
		mr.declEField("T_32PFVOID", 0x0503);   // 16:32 pointer to void
		mr.declEField("T_64PVOID", 0x0603);   // 64 bit pointer to void
		mr.declEField("T_CURRENCY", 0x0004);   // BASIC 8 byte currency value
		mr.declEField("T_NBASICSTR", 0x0005);   // Near BASIC string
		mr.declEField("T_FBASICSTR", 0x0006);   // Far BASIC string
		mr.declEField("T_NOTTRANS", 0x0007);   // type not translated by cvpack
		mr.declEField("T_BIT", 0x0060);   // bit
		mr.declEField("T_PASCHAR", 0x0061);   // Pascal CHAR
		mr.declEField("T_BOOL32FF", 0x0062);   // 32-bit BOOL where true is 0xffffffff
		//      Character types
		mr.declEField("T_CHAR", 0x0010);   // 8 bit signed
		mr.declEField("T_PCHAR", 0x0110);   // 16 bit pointer to 8 bit signed
		mr.declEField("T_PFCHAR", 0x0210);   // 16:16 far pointer to 8 bit signed
		mr.declEField("T_PHCHAR", 0x0310);   // 16:16 huge pointer to 8 bit signed
		mr.declEField("T_32PCHAR", 0x0410);   // 32 bit pointer to 8 bit signed
		mr.declEField("T_32PFCHAR", 0x0510);   // 16:32 pointer to 8 bit signed
		mr.declEField("T_64PCHAR", 0x0610);   // 64 bit pointer to 8 bit signed
		//
		mr.declEField("T_UCHAR", 0x0020);   // 8 bit unsigned
		mr.declEField("T_PUCHAR", 0x0120);   // 16 bit pointer to 8 bit unsigned
		mr.declEField("T_PFUCHAR", 0x0220);   // 16:16 far pointer to 8 bit unsigned
		mr.declEField("T_PHUCHAR", 0x0320);   // 16:16 huge pointer to 8 bit unsigned
		mr.declEField("T_32PUCHAR", 0x0420);   // 32 bit pointer to 8 bit unsigned
		mr.declEField("T_32PFUCHAR", 0x0520);   // 16:32 pointer to 8 bit unsigned
		mr.declEField("T_64PUCHAR", 0x0620);   // 64 bit pointer to 8 bit unsigned
		//      really a character types
		mr.declEField("T_RCHAR", 0x0070);   // really a char
		mr.declEField("T_PRCHAR", 0x0170);   // 16 bit pointer to a real char
		mr.declEField("T_PFRCHAR", 0x0270);   // 16:16 far pointer to a real char
		mr.declEField("T_PHRCHAR", 0x0370);   // 16:16 huge pointer to a real char
		mr.declEField("T_32PRCHAR", 0x0470);   // 32 bit pointer to a real char
		mr.declEField("T_32PFRCHAR", 0x0570);   // 16:32 pointer to a real char
		mr.declEField("T_64PRCHAR", 0x0670);   // 64 bit pointer to a real char
		//      really a wide character types
		mr.declEField("T_WCHAR", 0x0071);   // wide char
		mr.declEField("T_PWCHAR", 0x0171);   // 16 bit pointer to a wide char
		mr.declEField("T_PFWCHAR", 0x0271);   // 16:16 far pointer to a wide char
		mr.declEField("T_PHWCHAR", 0x0371);   // 16:16 huge pointer to a wide char
		mr.declEField("T_32PWCHAR", 0x0471);   // 32 bit pointer to a wide char
		mr.declEField("T_32PFWCHAR", 0x0571);   // 16:32 pointer to a wide char
		mr.declEField("T_64PWCHAR", 0x0671);   // 64 bit pointer to a wide char
		//      really a 16-bit unicode char
		mr.declEField("T_CHAR16", 0x007a);   // 16-bit unicode char
		mr.declEField("T_PCHAR16", 0x017a);   // 16 bit pointer to a 16-bit unicode char
		mr.declEField("T_PFCHAR16", 0x027a);   // 16:16 far pointer to a 16-bit unicode char
		mr.declEField("T_PHCHAR16", 0x037a);   // 16:16 huge pointer to a 16-bit unicode char
		mr.declEField("T_32PCHAR16", 0x047a);   // 32 bit pointer to a 16-bit unicode char
		mr.declEField("T_32PFCHAR16", 0x057a);   // 16:32 pointer to a 16-bit unicode char
		mr.declEField("T_64PCHAR16", 0x067a);   // 64 bit pointer to a 16-bit unicode char
		//      really a 32-bit unicode char
		mr.declEField("T_CHAR32", 0x007b);   // 32-bit unicode char
		mr.declEField("T_PCHAR32", 0x017b);   // 16 bit pointer to a 32-bit unicode char
		mr.declEField("T_PFCHAR32", 0x027b);   // 16:16 far pointer to a 32-bit unicode char
		mr.declEField("T_PHCHAR32", 0x037b);   // 16:16 huge pointer to a 32-bit unicode char
		mr.declEField("T_32PCHAR32", 0x047b);   // 32 bit pointer to a 32-bit unicode char
		mr.declEField("T_32PFCHAR32", 0x057b);   // 16:32 pointer to a 32-bit unicode char
		mr.declEField("T_64PCHAR32", 0x067b);   // 64 bit pointer to a 32-bit unicode char
		//      8 bit int types
		mr.declEField("T_INT1", 0x0068);   // 8 bit signed int
		mr.declEField("T_PINT1", 0x0168);   // 16 bit pointer to 8 bit signed int
		mr.declEField("T_PFINT1", 0x0268);   // 16:16 far pointer to 8 bit signed int
		mr.declEField("T_PHINT1", 0x0368);   // 16:16 huge pointer to 8 bit signed int
		mr.declEField("T_32PINT1", 0x0468);   // 32 bit pointer to 8 bit signed int
		mr.declEField("T_32PFINT1", 0x0568);   // 16:32 pointer to 8 bit signed int
		mr.declEField("T_64PINT1", 0x0668);   // 64 bit pointer to 8 bit signed int
		//
		mr.declEField("T_UINT1", 0x0069);   // 8 bit unsigned int
		mr.declEField("T_PUINT1", 0x0169);   // 16 bit pointer to 8 bit unsigned int
		mr.declEField("T_PFUINT1", 0x0269);   // 16:16 far pointer to 8 bit unsigned int
		mr.declEField("T_PHUINT1", 0x0369);   // 16:16 huge pointer to 8 bit unsigned int
		mr.declEField("T_32PUINT1", 0x0469);   // 32 bit pointer to 8 bit unsigned int
		mr.declEField("T_32PFUINT1", 0x0569);   // 16:32 pointer to 8 bit unsigned int
		mr.declEField("T_64PUINT1", 0x0669);   // 64 bit pointer to 8 bit unsigned int
		//      16 bit short types
		mr.declEField("T_SHORT", 0x0011);   // 16 bit signed
		mr.declEField("T_PSHORT", 0x0111);   // 16 bit pointer to 16 bit signed
		mr.declEField("T_PFSHORT", 0x0211);   // 16:16 far pointer to 16 bit signed
		mr.declEField("T_PHSHORT", 0x0311);   // 16:16 huge pointer to 16 bit signed
		mr.declEField("T_32PSHORT", 0x0411);   // 32 bit pointer to 16 bit signed
		mr.declEField("T_32PFSHORT", 0x0511);   // 16:32 pointer to 16 bit signed
		mr.declEField("T_64PSHORT", 0x0611);   // 64 bit pointer to 16 bit signed
		//
		mr.declEField("T_USHORT", 0x0021);   // 16 bit unsigned
		mr.declEField("T_PUSHORT", 0x0121);   // 16 bit pointer to 16 bit unsigned
		mr.declEField("T_PFUSHORT", 0x0221);   // 16:16 far pointer to 16 bit unsigned
		mr.declEField("T_PHUSHORT", 0x0321);   // 16:16 huge pointer to 16 bit unsigned
		mr.declEField("T_32PUSHORT", 0x0421);   // 32 bit pointer to 16 bit unsigned
		mr.declEField("T_32PFUSHORT", 0x0521);   // 16:32 pointer to 16 bit unsigned
		mr.declEField("T_64PUSHORT", 0x0621);   // 64 bit pointer to 16 bit unsigned
		//      16 bit int types
		mr.declEField("T_INT2", 0x0072);   // 16 bit signed int
		mr.declEField("T_PINT2", 0x0172);   // 16 bit pointer to 16 bit signed int
		mr.declEField("T_PFINT2", 0x0272);   // 16:16 far pointer to 16 bit signed int
		mr.declEField("T_PHINT2", 0x0372);   // 16:16 huge pointer to 16 bit signed int
		mr.declEField("T_32PINT2", 0x0472);   // 32 bit pointer to 16 bit signed int
		mr.declEField("T_32PFINT2", 0x0572);   // 16:32 pointer to 16 bit signed int
		mr.declEField("T_64PINT2", 0x0672);   // 64 bit pointer to 16 bit signed int
		//
		mr.declEField("T_UINT2", 0x0073);   // 16 bit unsigned int
		mr.declEField("T_PUINT2", 0x0173);   // 16 bit pointer to 16 bit unsigned int
		mr.declEField("T_PFUINT2", 0x0273);   // 16:16 far pointer to 16 bit unsigned int
		mr.declEField("T_PHUINT2", 0x0373);   // 16:16 huge pointer to 16 bit unsigned int
		mr.declEField("T_32PUINT2", 0x0473);   // 32 bit pointer to 16 bit unsigned int
		mr.declEField("T_32PFUINT2", 0x0573);   // 16:32 pointer to 16 bit unsigned int
		mr.declEField("T_64PUINT2", 0x0673);   // 64 bit pointer to 16 bit unsigned int
		//      32 bit long types
		mr.declEField("T_LONG", 0x0012);   // 32 bit signed
		mr.declEField("T_ULONG", 0x0022);   // 32 bit unsigned
		mr.declEField("T_PLONG", 0x0112);   // 16 bit pointer to 32 bit signed
		mr.declEField("T_PULONG", 0x0122);   // 16 bit pointer to 32 bit unsigned
		mr.declEField("T_PFLONG", 0x0212);   // 16:16 far pointer to 32 bit signed
		mr.declEField("T_PFULONG", 0x0222);   // 16:16 far pointer to 32 bit unsigned
		mr.declEField("T_PHLONG", 0x0312);   // 16:16 huge pointer to 32 bit signed
		mr.declEField("T_PHULONG", 0x0322);   // 16:16 huge pointer to 32 bit unsigned
		//
		mr.declEField("T_32PLONG", 0x0412);   // 32 bit pointer to 32 bit signed
		mr.declEField("T_32PULONG", 0x0422);   // 32 bit pointer to 32 bit unsigned
		mr.declEField("T_32PFLONG", 0x0512);   // 16:32 pointer to 32 bit signed
		mr.declEField("T_32PFULONG", 0x0522);   // 16:32 pointer to 32 bit unsigned
		mr.declEField("T_64PLONG", 0x0612);   // 64 bit pointer to 32 bit signed
		mr.declEField("T_64PULONG", 0x0622);   // 64 bit pointer to 32 bit unsigned
		//      32 bit int types
		mr.declEField("T_INT4", 0x0074);   // 32 bit signed int
		mr.declEField("T_PINT4", 0x0174);   // 16 bit pointer to 32 bit signed int
		mr.declEField("T_PFINT4", 0x0274);   // 16:16 far pointer to 32 bit signed int
		mr.declEField("T_PHINT4", 0x0374);   // 16:16 huge pointer to 32 bit signed int
		mr.declEField("T_32PINT4", 0x0474);   // 32 bit pointer to 32 bit signed int
		mr.declEField("T_32PFINT4", 0x0574);   // 16:32 pointer to 32 bit signed int
		mr.declEField("T_64PINT4", 0x0674);   // 64 bit pointer to 32 bit signed int
		//
		mr.declEField("T_UINT4", 0x0075);   // 32 bit unsigned int
		mr.declEField("T_PUINT4", 0x0175);   // 16 bit pointer to 32 bit unsigned int
		mr.declEField("T_PFUINT4", 0x0275);   // 16:16 far pointer to 32 bit unsigned int
		mr.declEField("T_PHUINT4", 0x0375);   // 16:16 huge pointer to 32 bit unsigned int
		mr.declEField("T_32PUINT4", 0x0475);   // 32 bit pointer to 32 bit unsigned int
		mr.declEField("T_32PFUINT4", 0x0575);   // 16:32 pointer to 32 bit unsigned int
		mr.declEField("T_64PUINT4", 0x0675);   // 64 bit pointer to 32 bit unsigned int
		//      64 bit quad types
		mr.declEField("T_QUAD", 0x0013);   // 64 bit signed
		mr.declEField("T_PQUAD", 0x0113);   // 16 bit pointer to 64 bit signed
		mr.declEField("T_PFQUAD", 0x0213);   // 16:16 far pointer to 64 bit signed
		mr.declEField("T_PHQUAD", 0x0313);   // 16:16 huge pointer to 64 bit signed
		mr.declEField("T_32PQUAD", 0x0413);   // 32 bit pointer to 64 bit signed
		mr.declEField("T_32PFQUAD", 0x0513);   // 16:32 pointer to 64 bit signed
		mr.declEField("T_64PQUAD", 0x0613);   // 64 bit pointer to 64 bit signed
		//
		mr.declEField("T_UQUAD", 0x0023);   // 64 bit unsigned
		mr.declEField("T_PUQUAD", 0x0123);   // 16 bit pointer to 64 bit unsigned
		mr.declEField("T_PFUQUAD", 0x0223);   // 16:16 far pointer to 64 bit unsigned
		mr.declEField("T_PHUQUAD", 0x0323);   // 16:16 huge pointer to 64 bit unsigned
		mr.declEField("T_32PUQUAD", 0x0423);   // 32 bit pointer to 64 bit unsigned
		mr.declEField("T_32PFUQUAD", 0x0523);   // 16:32 pointer to 64 bit unsigned
		mr.declEField("T_64PUQUAD", 0x0623);   // 64 bit pointer to 64 bit unsigned
		//      64 bit int types
		mr.declEField("T_INT8", 0x0076);   // 64 bit signed int
		mr.declEField("T_PINT8", 0x0176);   // 16 bit pointer to 64 bit signed int
		mr.declEField("T_PFINT8", 0x0276);   // 16:16 far pointer to 64 bit signed int
		mr.declEField("T_PHINT8", 0x0376);   // 16:16 huge pointer to 64 bit signed int
		mr.declEField("T_32PINT8", 0x0476);   // 32 bit pointer to 64 bit signed int
		mr.declEField("T_32PFINT8", 0x0576);   // 16:32 pointer to 64 bit signed int
		mr.declEField("T_64PINT8", 0x0676);   // 64 bit pointer to 64 bit signed int
		//
		mr.declEField("T_UINT8", 0x0077);   // 64 bit unsigned int
		mr.declEField("T_PUINT8", 0x0177);   // 16 bit pointer to 64 bit unsigned int
		mr.declEField("T_PFUINT8", 0x0277);   // 16:16 far pointer to 64 bit unsigned int
		mr.declEField("T_PHUINT8", 0x0377);   // 16:16 huge pointer to 64 bit unsigned int
		mr.declEField("T_32PUINT8", 0x0477);   // 32 bit pointer to 64 bit unsigned int
		mr.declEField("T_32PFUINT8", 0x0577);   // 16:32 pointer to 64 bit unsigned int
		mr.declEField("T_64PUINT8", 0x0677);   // 64 bit pointer to 64 bit unsigned int
		//      128 bit octet types
		mr.declEField("T_OCT", 0x0014);   // 128 bit signed
		mr.declEField("T_POCT", 0x0114);   // 16 bit pointer to 128 bit signed
		mr.declEField("T_PFOCT", 0x0214);   // 16:16 far pointer to 128 bit signed
		mr.declEField("T_PHOCT", 0x0314);   // 16:16 huge pointer to 128 bit signed
		mr.declEField("T_32POCT", 0x0414);   // 32 bit pointer to 128 bit signed
		mr.declEField("T_32PFOCT", 0x0514);   // 16:32 pointer to 128 bit signed
		mr.declEField("T_64POCT", 0x0614);   // 64 bit pointer to 128 bit signed

		mr.declEField("T_UOCT", 0x0024);   // 128 bit unsigned
		mr.declEField("T_PUOCT", 0x0124);   // 16 bit pointer to 128 bit unsigned
		mr.declEField("T_PFUOCT", 0x0224);   // 16:16 far pointer to 128 bit unsigned
		mr.declEField("T_PHUOCT", 0x0324);   // 16:16 huge pointer to 128 bit unsigned
		mr.declEField("T_32PUOCT", 0x0424);   // 32 bit pointer to 128 bit unsigned
		mr.declEField("T_32PFUOCT", 0x0524);   // 16:32 pointer to 128 bit unsigned
		mr.declEField("T_64PUOCT", 0x0624);   // 64 bit pointer to 128 bit unsigned
		//      128 bit int types
		mr.declEField("T_INT16", 0x0078);   // 128 bit signed int
		mr.declEField("T_PINT16", 0x0178);   // 16 bit pointer to 128 bit signed int
		mr.declEField("T_PFINT16", 0x0278);   // 16:16 far pointer to 128 bit signed int
		mr.declEField("T_PHINT16", 0x0378);   // 16:16 huge pointer to 128 bit signed int
		mr.declEField("T_32PINT16", 0x0478);   // 32 bit pointer to 128 bit signed int
		mr.declEField("T_32PFINT16", 0x0578);   // 16:32 pointer to 128 bit signed int
		mr.declEField("T_64PINT16", 0x0678);   // 64 bit pointer to 128 bit signed int
		//
		mr.declEField("T_UINT16", 0x0079);   // 128 bit unsigned int
		mr.declEField("T_PUINT16", 0x0179);   // 16 bit pointer to 128 bit unsigned int
		mr.declEField("T_PFUINT16", 0x0279);   // 16:16 far pointer to 128 bit unsigned int
		mr.declEField("T_PHUINT16", 0x0379);   // 16:16 huge pointer to 128 bit unsigned int
		mr.declEField("T_32PUINT16", 0x0479);   // 32 bit pointer to 128 bit unsigned int
		mr.declEField("T_32PFUINT16", 0x0579);   // 16:32 pointer to 128 bit unsigned int
		mr.declEField("T_64PUINT16", 0x0679);   // 64 bit pointer to 128 bit unsigned int
		//      16 bit real types
		mr.declEField("T_REAL16", 0x0046);   // 16 bit real
		mr.declEField("T_PREAL16", 0x0146);   // 16 bit pointer to 16 bit real
		mr.declEField("T_PFREAL16", 0x0246);   // 16:16 far pointer to 16 bit real
		mr.declEField("T_PHREAL16", 0x0346);   // 16:16 huge pointer to 16 bit real
		mr.declEField("T_32PREAL16", 0x0446);   // 32 bit pointer to 16 bit real
		mr.declEField("T_32PFREAL16", 0x0546);   // 16:32 pointer to 16 bit real
		mr.declEField("T_64PREAL16", 0x0646);   // 64 bit pointer to 16 bit real
		//      32 bit real types
		mr.declEField("T_REAL32", 0x0040);   // 32 bit real
		mr.declEField("T_PREAL32", 0x0140);   // 16 bit pointer to 32 bit real
		mr.declEField("T_PFREAL32", 0x0240);   // 16:16 far pointer to 32 bit real
		mr.declEField("T_PHREAL32", 0x0340);   // 16:16 huge pointer to 32 bit real
		mr.declEField("T_32PREAL32", 0x0440);   // 32 bit pointer to 32 bit real
		mr.declEField("T_32PFREAL32", 0x0540);   // 16:32 pointer to 32 bit real
		mr.declEField("T_64PREAL32", 0x0640);   // 64 bit pointer to 32 bit real
		//      32 bit partial-precision real types
		mr.declEField("T_REAL32PP", 0x0045);   // 32 bit PP real
		mr.declEField("T_PREAL32PP", 0x0145);   // 16 bit pointer to 32 bit PP real
		mr.declEField("T_PFREAL32PP", 0x0245);   // 16:16 far pointer to 32 bit PP real
		mr.declEField("T_PHREAL32PP", 0x0345);   // 16:16 huge pointer to 32 bit PP real
		mr.declEField("T_32PREAL32PP", 0x0445);   // 32 bit pointer to 32 bit PP real
		mr.declEField("T_32PFREAL32PP", 0x0545);   // 16:32 pointer to 32 bit PP real
		mr.declEField("T_64PREAL32PP", 0x0645);   // 64 bit pointer to 32 bit PP real
		//      48 bit real types
		mr.declEField("T_REAL48", 0x0044);   // 48 bit real
		mr.declEField("T_PREAL48", 0x0144);   // 16 bit pointer to 48 bit real
		mr.declEField("T_PFREAL48", 0x0244);   // 16:16 far pointer to 48 bit real
		mr.declEField("T_PHREAL48", 0x0344);   // 16:16 huge pointer to 48 bit real
		mr.declEField("T_32PREAL48", 0x0444);   // 32 bit pointer to 48 bit real
		mr.declEField("T_32PFREAL48", 0x0544);   // 16:32 pointer to 48 bit real
		mr.declEField("T_64PREAL48", 0x0644);   // 64 bit pointer to 48 bit real
		//      64 bit real types
		mr.declEField("T_REAL64", 0x0041);   // 64 bit real
		mr.declEField("T_PREAL64", 0x0141);   // 16 bit pointer to 64 bit real
		mr.declEField("T_PFREAL64", 0x0241);   // 16:16 far pointer to 64 bit real
		mr.declEField("T_PHREAL64", 0x0341);   // 16:16 huge pointer to 64 bit real
		mr.declEField("T_32PREAL64", 0x0441);   // 32 bit pointer to 64 bit real
		mr.declEField("T_32PFREAL64", 0x0541);   // 16:32 pointer to 64 bit real
		mr.declEField("T_64PREAL64", 0x0641);   // 64 bit pointer to 64 bit real
		//      80 bit real types
		mr.declEField("T_REAL80", 0x0042);   // 80 bit real
		mr.declEField("T_PREAL80", 0x0142);   // 16 bit pointer to 80 bit real
		mr.declEField("T_PFREAL80", 0x0242);   // 16:16 far pointer to 80 bit real
		mr.declEField("T_PHREAL80", 0x0342);   // 16:16 huge pointer to 80 bit real
		mr.declEField("T_32PREAL80", 0x0442);   // 32 bit pointer to 80 bit real
		mr.declEField("T_32PFREAL80", 0x0542);   // 16:32 pointer to 80 bit real
		mr.declEField("T_64PREAL80", 0x0642);   // 64 bit pointer to 80 bit real
		//      128 bit real types
		mr.declEField("T_REAL128", 0x0043);   // 128 bit real
		mr.declEField("T_PREAL128", 0x0143);   // 16 bit pointer to 128 bit real
		mr.declEField("T_PFREAL128", 0x0243);   // 16:16 far pointer to 128 bit real
		mr.declEField("T_PHREAL128", 0x0343);   // 16:16 huge pointer to 128 bit real
		mr.declEField("T_32PREAL128", 0x0443);   // 32 bit pointer to 128 bit real
		mr.declEField("T_32PFREAL128", 0x0543);   // 16:32 pointer to 128 bit real
		mr.declEField("T_64PREAL128", 0x0643);   // 64 bit pointer to 128 bit real
		//      32 bit complex types
		mr.declEField("T_CPLX32", 0x0050);   // 32 bit complex
		mr.declEField("T_PCPLX32", 0x0150);   // 16 bit pointer to 32 bit complex
		mr.declEField("T_PFCPLX32", 0x0250);   // 16:16 far pointer to 32 bit complex
		mr.declEField("T_PHCPLX32", 0x0350);   // 16:16 huge pointer to 32 bit complex
		mr.declEField("T_32PCPLX32", 0x0450);   // 32 bit pointer to 32 bit complex
		mr.declEField("T_32PFCPLX32", 0x0550);   // 16:32 pointer to 32 bit complex
		mr.declEField("T_64PCPLX32", 0x0650);   // 64 bit pointer to 32 bit complex
		//      64 bit complex types
		mr.declEField("T_CPLX64", 0x0051);   // 64 bit complex
		mr.declEField("T_PCPLX64", 0x0151);   // 16 bit pointer to 64 bit complex
		mr.declEField("T_PFCPLX64", 0x0251);   // 16:16 far pointer to 64 bit complex
		mr.declEField("T_PHCPLX64", 0x0351);   // 16:16 huge pointer to 64 bit complex
		mr.declEField("T_32PCPLX64", 0x0451);   // 32 bit pointer to 64 bit complex
		mr.declEField("T_32PFCPLX64", 0x0551);   // 16:32 pointer to 64 bit complex
		mr.declEField("T_64PCPLX64", 0x0651);   // 64 bit pointer to 64 bit complex
		//      80 bit complex types
		mr.declEField("T_CPLX80", 0x0052);   // 80 bit complex
		mr.declEField("T_PCPLX80", 0x0152);   // 16 bit pointer to 80 bit complex
		mr.declEField("T_PFCPLX80", 0x0252);   // 16:16 far pointer to 80 bit complex
		mr.declEField("T_PHCPLX80", 0x0352);   // 16:16 huge pointer to 80 bit complex
		mr.declEField("T_32PCPLX80", 0x0452);   // 32 bit pointer to 80 bit complex
		mr.declEField("T_32PFCPLX80", 0x0552);   // 16:32 pointer to 80 bit complex
		mr.declEField("T_64PCPLX80", 0x0652);   // 64 bit pointer to 80 bit complex
		//      128 bit complex types
		mr.declEField("T_CPLX128", 0x0053);   // 128 bit complex
		mr.declEField("T_PCPLX128", 0x0153);   // 16 bit pointer to 128 bit complex
		mr.declEField("T_PFCPLX128", 0x0253);   // 16:16 far pointer to 128 bit complex
		mr.declEField("T_PHCPLX128", 0x0353);   // 16:16 huge pointer to 128 bit real
		mr.declEField("T_32PCPLX128", 0x0453);   // 32 bit pointer to 128 bit complex
		mr.declEField("T_32PFCPLX128", 0x0553);   // 16:32 pointer to 128 bit complex
		mr.declEField("T_64PCPLX128", 0x0653);   // 64 bit pointer to 128 bit complex
		//      boolean types
		mr.declEField("T_BOOL08", 0x0030);   // 8 bit boolean
		mr.declEField("T_PBOOL08", 0x0130);   // 16 bit pointer to  8 bit boolean
		mr.declEField("T_PFBOOL08", 0x0230);   // 16:16 far pointer to  8 bit boolean
		mr.declEField("T_PHBOOL08", 0x0330);   // 16:16 huge pointer to  8 bit boolean
		mr.declEField("T_32PBOOL08", 0x0430);   // 32 bit pointer to 8 bit boolean
		mr.declEField("T_32PFBOOL08", 0x0530);   // 16:32 pointer to 8 bit boolean
		mr.declEField("T_64PBOOL08", 0x0630);   // 64 bit pointer to 8 bit boolean
		//
		mr.declEField("T_BOOL16", 0x0031);   // 16 bit boolean
		mr.declEField("T_PBOOL16", 0x0131);   // 16 bit pointer to 16 bit boolean
		mr.declEField("T_PFBOOL16", 0x0231);   // 16:16 far pointer to 16 bit boolean
		mr.declEField("T_PHBOOL16", 0x0331);   // 16:16 huge pointer to 16 bit boolean
		mr.declEField("T_32PBOOL16", 0x0431);   // 32 bit pointer to 18 bit boolean
		mr.declEField("T_32PFBOOL16", 0x0531);   // 16:32 pointer to 16 bit boolean
		mr.declEField("T_64PBOOL16", 0x0631);   // 64 bit pointer to 18 bit boolean
		//
		mr.declEField("T_BOOL32", 0x0032);   // 32 bit boolean
		mr.declEField("T_PBOOL32", 0x0132);   // 16 bit pointer to 32 bit boolean
		mr.declEField("T_PFBOOL32", 0x0232);   // 16:16 far pointer to 32 bit boolean
		mr.declEField("T_PHBOOL32", 0x0332);   // 16:16 huge pointer to 32 bit boolean
		mr.declEField("T_32PBOOL32", 0x0432);   // 32 bit pointer to 32 bit boolean
		mr.declEField("T_32PFBOOL32", 0x0532);   // 16:32 pointer to 32 bit boolean
		mr.declEField("T_64PBOOL32", 0x0632);   // 64 bit pointer to 32 bit boolean
		//
		mr.declEField("T_BOOL64", 0x0033);   // 64 bit boolean
		mr.declEField("T_PBOOL64", 0x0133);   // 16 bit pointer to 64 bit boolean
		mr.declEField("T_PFBOOL64", 0x0233);   // 16:16 far pointer to 64 bit boolean
		mr.declEField("T_PHBOOL64", 0x0333);   // 16:16 huge pointer to 64 bit boolean
		mr.declEField("T_32PBOOL64", 0x0433);   // 32 bit pointer to 64 bit boolean
		mr.declEField("T_32PFBOOL64", 0x0533);   // 16:32 pointer to 64 bit boolean
		mr.declEField("T_64PBOOL64", 0x0633);   // 64 bit pointer to 64 bit boolean
		//      ???
		mr.declEField("T_NCVPTR", 0x01f0);   // CV Internal type for created near pointers
		mr.declEField("T_FCVPTR", 0x02f0);   // CV Internal type for created far pointers
		mr.declEField("T_HCVPTR", 0x03f0);   // CV Internal type for created huge pointers
		mr.declEField("T_32NCVPTR", 0x04f0);   // CV Internal type for created near 32-bit pointers
		mr.declEField("T_32FCVPTR", 0x05f0);   // CV Internal type for created far 32-bit pointers
		mr.declEField("T_64NCVPTR", 0x06f0);   // CV Internal type for created near 64-bit pointers
	}
	if (mr.NewScope("LEAF_ENUM_e", SCOPE_ENUM))
	{
		SAFE_SCOPE_HERE(mr);
		// leaf indices starting records but referenced from symbol records
		mr.declEField("LF_MODIFIER_16t", 0x0001);
		mr.declEField("LF_POINTER_16t", 0x0002);
		mr.declEField("LF_ARRAY_16t", 0x0003);
		mr.declEField("LF_CLASS_16t", 0x0004);
		mr.declEField("LF_STRUCTURE_16t", 0x0005);
		mr.declEField("LF_UNION_16t", 0x0006);
		mr.declEField("LF_ENUM_16t", 0x0007);
		mr.declEField("LF_PROCEDURE_16t", 0x0008);
		mr.declEField("LF_MFUNCTION_16t", 0x0009);
		mr.declEField("LF_VTSHAPE", 0x000a);
		mr.declEField("LF_COBOL0_16t", 0x000b);
		mr.declEField("LF_COBOL1", 0x000c);
		mr.declEField("LF_BARRAY_16t", 0x000d);
		mr.declEField("LF_LABEL", 0x000e);
		mr.declEField("LF_NULL", 0x000f);
		mr.declEField("LF_NOTTRAN", 0x0010);
		mr.declEField("LF_DIMARRAY_16t", 0x0011);
		mr.declEField("LF_VFTPATH_16t", 0x0012);
		mr.declEField("LF_PRECOMP_16t", 0x0013);       // not referenced from symbol
		mr.declEField("LF_ENDPRECOMP", 0x0014);       // not referenced from symbol
		mr.declEField("LF_OEM_16t", 0x0015);       // oem definable type string
		mr.declEField("LF_TYPESERVER_ST", 0x0016);       // not referenced from symbol
		// leaf indices starting records but referenced only from type records
		mr.declEField("LF_SKIP_16t", 0x0200);
		mr.declEField("LF_ARGLIST_16t", 0x0201);
		mr.declEField("LF_DEFARG_16t", 0x0202);
		mr.declEField("LF_LIST", 0x0203);
		mr.declEField("LF_FIELDLIST_16t", 0x0204);
		mr.declEField("LF_DERIVED_16t", 0x0205);
		mr.declEField("LF_BITFIELD_16t", 0x0206);
		mr.declEField("LF_METHODLIST_16t", 0x0207);
		mr.declEField("LF_DIMCONU_16t", 0x0208);
		mr.declEField("LF_DIMCONLU_16t", 0x0209);
		mr.declEField("LF_DIMVARU_16t", 0x020a);
		mr.declEField("LF_DIMVARLU_16t", 0x020b);
		mr.declEField("LF_REFSYM", 0x020c);
		mr.declEField("LF_BCLASS_16t", 0x0400);
		mr.declEField("LF_VBCLASS_16t", 0x0401);
		mr.declEField("LF_IVBCLASS_16t", 0x0402);
		mr.declEField("LF_ENUMERATE_ST", 0x0403);
		mr.declEField("LF_FRIENDFCN_16t", 0x0404);
		mr.declEField("LF_INDEX_16t", 0x0405);
		mr.declEField("LF_MEMBER_16t", 0x0406);
		mr.declEField("LF_STMEMBER_16t", 0x0407);
		mr.declEField("LF_METHOD_16t", 0x0408);
		mr.declEField("LF_NESTTYPE_16t", 0x0409);
		mr.declEField("LF_VFUNCTAB_16t", 0x040a);
		mr.declEField("LF_FRIENDCLS_16t", 0x040b);
		mr.declEField("LF_ONEMETHOD_16t", 0x040c);
		mr.declEField("LF_VFUNCOFF_16t", 0x040d);
		// 32-bit type index versions of leaves, all have the 0x1000 bit set
		mr.declEField("LF_TI16_MAX", 0x1000);
		mr.declEField("LF_MODIFIER", 0x1001);
		mr.declEField("LF_POINTER", 0x1002);
		mr.declEField("LF_ARRAY_ST", 0x1003);
		mr.declEField("LF_CLASS_ST", 0x1004);
		mr.declEField("LF_STRUCTURE_ST", 0x1005);
		mr.declEField("LF_UNION_ST", 0x1006);
		mr.declEField("LF_ENUM_ST", 0x1007);
		mr.declEField("LF_PROCEDURE", 0x1008);
		mr.declEField("LF_MFUNCTION", 0x1009);
		mr.declEField("LF_COBOL0", 0x100a);
		mr.declEField("LF_BARRAY", 0x100b);
		mr.declEField("LF_DIMARRAY_ST", 0x100c);
		mr.declEField("LF_VFTPATH", 0x100d);
		mr.declEField("LF_PRECOMP_ST", 0x100e);       // not referenced from symbol
		mr.declEField("LF_OEM", 0x100f);       // oem definable type string
		mr.declEField("LF_ALIAS_ST", 0x1010);       // alias (typedef) type
		mr.declEField("LF_OEM2", 0x1011);       // oem definable type string
		// leaf indices starting records but referenced only from type records
		mr.declEField("LF_SKIP", 0x1200);
		mr.declEField("LF_ARGLIST", 0x1201);
		mr.declEField("LF_DEFARG_ST", 0x1202);
		mr.declEField("LF_FIELDLIST", 0x1203);
		mr.declEField("LF_DERIVED", 0x1204);
		mr.declEField("LF_BITFIELD", 0x1205);
		mr.declEField("LF_METHODLIST", 0x1206);
		mr.declEField("LF_DIMCONU", 0x1207);
		mr.declEField("LF_DIMCONLU", 0x1208);
		mr.declEField("LF_DIMVARU", 0x1209);
		mr.declEField("LF_DIMVARLU", 0x120a);
		mr.declEField("LF_BCLASS", 0x1400);
		mr.declEField("LF_VBCLASS", 0x1401);
		mr.declEField("LF_IVBCLASS", 0x1402);
		mr.declEField("LF_FRIENDFCN_ST", 0x1403);
		mr.declEField("LF_INDEX", 0x1404);
		mr.declEField("LF_MEMBER_ST", 0x1405);
		mr.declEField("LF_STMEMBER_ST", 0x1406);
		mr.declEField("LF_METHOD_ST", 0x1407);
		mr.declEField("LF_NESTTYPE_ST", 0x1408);
		mr.declEField("LF_VFUNCTAB", 0x1409);
		mr.declEField("LF_FRIENDCLS", 0x140a);
		mr.declEField("LF_ONEMETHOD_ST", 0x140b);
		mr.declEField("LF_VFUNCOFF", 0x140c);
		mr.declEField("LF_NESTTYPEEX_ST", 0x140d);
		mr.declEField("LF_MEMBERMODIFY_ST", 0x140e);
		mr.declEField("LF_MANAGED_ST", 0x140f);
		// Types w/ SZ names
		mr.declEField("LF_ST_MAX", 0x1500);
		mr.declEField("LF_TYPESERVER", 0x1501);       // not referenced from symbol
		mr.declEField("LF_ENUMERATE", 0x1502);
		mr.declEField("LF_ARRAY", 0x1503);
		mr.declEField("LF_CLASS", 0x1504);
		mr.declEField("LF_STRUCTURE", 0x1505);
		mr.declEField("LF_UNION", 0x1506);
		mr.declEField("LF_ENUM", 0x1507);
		mr.declEField("LF_DIMARRAY", 0x1508);
		mr.declEField("LF_PRECOMP", 0x1509);       // not referenced from symbol
		mr.declEField("LF_ALIAS", 0x150a);       // alias (typedef) type
		mr.declEField("LF_DEFARG", 0x150b);
		mr.declEField("LF_FRIENDFCN", 0x150c);
		mr.declEField("LF_MEMBER", 0x150d);
		mr.declEField("LF_STMEMBER", 0x150e);
		mr.declEField("LF_METHOD", 0x150f);
		mr.declEField("LF_NESTTYPE", 0x1510);
		mr.declEField("LF_ONEMETHOD", 0x1511);
		mr.declEField("LF_NESTTYPEEX", 0x1512);
		mr.declEField("LF_MEMBERMODIFY", 0x1513);
		mr.declEField("LF_MANAGED", 0x1514);
		mr.declEField("LF_TYPESERVER2", 0x1515);
		mr.declEField("LF_STRIDED_ARRAY", 0x1516);    // same as mr.declEField("LF_ARRAY, but with stride between adjacent elements
		mr.declEField("LF_HLSL", 0x1517);
		mr.declEField("LF_MODIFIER_EX", 0x1518);
		mr.declEField("LF_INTERFACE", 0x1519);
		mr.declEField("LF_BINTERFACE", 0x151a);
		mr.declEField("LF_VECTOR", 0x151b);
		mr.declEField("LF_MATRIX", 0x151c);
		mr.declEField("LF_VFTABLE", 0x151d);      // a virtual function table
		//LF_ENDOFLEAFRECORD", LF_VFTABLE,
		//LF_TYPE_LAST,                    // one greater than the last type record
		//LF_TYPE_MAX"", LF_TYPE_LAST - 1,
		mr.declEField("LF_FUNC_ID", 0x1601);    // global func ID
		mr.declEField("LF_MFUNC_ID", 0x1602);    // member func ID
		mr.declEField("LF_BUILDINFO", 0x1603);    // build info: tool, version, command line, src/pdb file
		mr.declEField("LF_SUBSTR_LIST", 0x1604);    // similar to mr.declEField("LF_ARGLIST, for list of sub strings
		mr.declEField("LF_STRING_ID", 0x1605);    // string ID
		mr.declEField("LF_UDT_SRC_LINE", 0x1606);    // source and line on where an UDT is defined
		// only generated by compiler
		mr.declEField("LF_UDT_MOD_SRC_LINE", 0x1607);    // module, source and line on where an UDT is defined
		// only generated by linker
		//LF_ID_LAST,                      // one greater than the last ID record
		//LF_ID_MAX", LF_ID_LAST - 1,
		mr.declEField("LF_NUMERIC", 0x8000);
		mr.declEField("LF_CHAR", 0x8000);
		mr.declEField("LF_SHORT", 0x8001);
		mr.declEField("LF_USHORT", 0x8002);
		mr.declEField("LF_LONG", 0x8003);
		mr.declEField("LF_ULONG", 0x8004);
		mr.declEField("LF_REAL32", 0x8005);
		mr.declEField("LF_REAL64", 0x8006);
		mr.declEField("LF_REAL80", 0x8007);
		mr.declEField("LF_REAL128", 0x8008);
		mr.declEField("LF_QUADWORD", 0x8009);
		mr.declEField("LF_UQUADWORD", 0x800a);
		mr.declEField("LF_REAL48", 0x800b);
		mr.declEField("LF_COMPLEX32", 0x800c);
		mr.declEField("LF_COMPLEX64", 0x800d);
		mr.declEField("LF_COMPLEX80", 0x800e);
		mr.declEField("LF_COMPLEX128", 0x800f);
		mr.declEField("LF_VARSTRING", 0x8010);
		mr.declEField("LF_OCTWORD", 0x8017);
		mr.declEField("LF_UOCTWORD", 0x8018);
		mr.declEField("LF_DECIMAL", 0x8019);
		mr.declEField("LF_DATE", 0x801a);
		mr.declEField("LF_UTF8STRING", 0x801b);
		mr.declEField("LF_REAL16", 0x801c);
		mr.declEField("LF_PAD0", 0xf0);
		mr.declEField("LF_PAD1", 0xf1);
		mr.declEField("LF_PAD2", 0xf2);
		mr.declEField("LF_PAD3", 0xf3);
		mr.declEField("LF_PAD4", 0xf4);
		mr.declEField("LF_PAD5", 0xf5);
		mr.declEField("LF_PAD6", 0xf6);
		mr.declEField("LF_PAD7", 0xf7);
		mr.declEField("LF_PAD8", 0xf8);
		mr.declEField("LF_PAD9", 0xf9);
		mr.declEField("LF_PAD10", 0xfa);
		mr.declEField("LF_PAD11", 0xfb);
		mr.declEField("LF_PAD12", 0xfc);
		mr.declEField("LF_PAD13", 0xfd);
		mr.declEField("LF_PAD14", 0xfe);
		mr.declEField("LF_PAD15", 0xff);
	}
	if (mr.NewScope("TypeRecord"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("length", mr.type(TYPEID_INT16));
		mr.declField("leafType", mr.enumOf(mr.type("LEAF_ENUM_e"), TYPEID_INT16));
	}
	if (mr.NewScope("CV_prop_t"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declBField("packed", mr.type(TYPEID_USHORT));     // true if structure is packed
		mr.declBField("ctor", mr.type(TYPEID_USHORT));     // true if constructors or destructors present
		mr.declBField("ovlops", mr.type(TYPEID_USHORT));     // true if overloaded operators present
		mr.declBField("isnested", mr.type(TYPEID_USHORT));     // true if this is a nested class
		mr.declBField("cnested", mr.type(TYPEID_USHORT));     // true if this class contains nested types
		mr.declBField("opassign", mr.type(TYPEID_USHORT));     // true if overloaded assignment (=)
		mr.declBField("opcast", mr.type(TYPEID_USHORT));     // true if casting methods
		mr.declBField("fwdref", mr.type(TYPEID_USHORT));     // true if forward reference (incomplete defn)
		mr.declBField("scoped", mr.type(TYPEID_USHORT));     // scoped definition
		mr.declBField("hasuniquename", mr.type(TYPEID_USHORT));   // true if there is a decorated name following the regular name
		mr.declBField("sealed", mr.type(TYPEID_USHORT));     // true if class cannot be used as a base class
		mr.declBField("hfa", mr.arrayOf(mr.type(TYPEID_USHORT), 2));     // CV_HFA_e
		mr.declBField("intrinsic", mr.type(TYPEID_USHORT));     // true if class is an intrinsic type (e.g. __m128d)
		mr.declBField("mocom", mr.arrayOf(mr.type(TYPEID_USHORT), 2));     // CV_MOCOM_UDT_e
	}
	if (mr.NewScope("CV_access_t", SCOPE_ENUM))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("none");
		mr.declEField("private", 1);
		mr.declEField("protected", 2);
		mr.declEField("public", 3);
	}
	if (mr.NewScope("CV_methodprop_e", SCOPE_ENUM))//CV_MT
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("vanilla", 0x00);
		mr.declEField("virtual", 0x01);
		mr.declEField("static", 0x02);
		mr.declEField("friend", 0x03);
		mr.declEField("intro", 0x04);
		mr.declEField("purevirt", 0x05);
		mr.declEField("pureintro", 0x06);
	}
	if (mr.NewScope("CV_fldattr_t"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declBField("access", mr.arrayOf(mr.enumOf(mr.type("CV_access_t"), TYPEID_USHORT), 2));     // access protection CV_access_t
		mr.declBField("mprop", mr.arrayOf(mr.enumOf(mr.type("CV_methodprop_e"), TYPEID_USHORT), 3));     // method properties CV_methodprop_t
		mr.declBField("pseudo", mr.type(TYPEID_USHORT));     // compiler generated fcn and does not exist
		mr.declBField("noinherit", mr.type(TYPEID_USHORT));     // true if class cannot be inherited
		mr.declBField("noconstruct", mr.type(TYPEID_USHORT));     // true if class cannot be constructed
		mr.declBField("compgenx", mr.type(TYPEID_USHORT));     // compiler generated fcn and does exist
		mr.declBField("sealed", mr.type(TYPEID_USHORT));     // true if method cannot be overridden
		//mr.declBField("unused", mr.arrayOf(mr.type(TYPEID_USHORT), 6));     // unused
	}
	if (mr.NewScope("CV_modifier_t"))//MOD_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declBField("const", mr.type(TYPEID_USHORT));
		mr.declBField("volatile", mr.type(TYPEID_USHORT));
		mr.declBField("unaligned", mr.type(TYPEID_USHORT));
		//mr.declBField("unused", mr.arrayOf(mr.type(TYPEID_USHORT), 13));
	}
	if (mr.NewScope("CV_ptrtype_e", SCOPE_ENUM))//CV_PTR_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("NEAR", 0x00);	// 16 bit pointer
		mr.declEField("FAR", 0x01);	// 16:16 far pointer
		mr.declEField("HUGE", 0x02);	// 16:16 huge pointer
		mr.declEField("BASE_SEG", 0x03);	// based on segment
		mr.declEField("BASE_VAL", 0x04);	// based on value of base
		mr.declEField("BASE_SEGVAL", 0x05);	// based on segment value of base
		mr.declEField("BASE_ADDR", 0x06);	// based on address of base
		mr.declEField("BASE_SEGADDR", 0x07);	// based on segment address of base
		mr.declEField("BASE_TYPE", 0x08);	// based on type
		mr.declEField("BASE_SELF", 0x09);	// based on self
		mr.declEField("NEAR32", 0x0a);	// 32 bit pointer
		mr.declEField("FAR32", 0x0b);	// 16:32 pointer
		mr.declEField("_64", 0x0c);	// 64 bit pointer
		//mr.declEField("UNUSEDPTR", 0x0d);	// first unused pointer type
	}
	if (mr.NewScope("CV_ptrmode_e", SCOPE_ENUM))//CV_PTR_MODE_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("PTR", 0x00); // "normal" pointer
		mr.declEField("REF", 0x01); // "old" reference
		mr.declEField("LVREF", 0x01); // l-value reference
		mr.declEField("PMEM", 0x02); // pointer to data member
		mr.declEField("PMFUNC", 0x03); // pointer to member function
		mr.declEField("RVREF", 0x04); // r-value reference
		//.mr.declEField("RESERVED", 0x05);  // first unused pointer mode
	}
	if (mr.NewScope("CV_PointerAttr_t"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declBField("ptrtype", mr.arrayOf(mr.enumOf(mr.type("CV_ptrtype_e"), TYPEID_ULONG), 5));	// ordinal specifying pointer type (CV_ptrtype_e)
		mr.declBField("ptrmode", mr.arrayOf(mr.enumOf(mr.type("CV_ptrmode_e"), TYPEID_ULONG), 3));	// ordinal specifying pointer mode (CV_ptrmode_e)
		mr.declBField("isflat32", mr.type(TYPEID_ULONG));	// true if 0:32 pointer
		mr.declBField("isvolatile", mr.type(TYPEID_ULONG));	// TRUE if volatile pointer
		mr.declBField("isconst", mr.type(TYPEID_ULONG));		// TRUE if const pointer
		mr.declBField("isunaligned", mr.type(TYPEID_ULONG));		// TRUE if unaligned pointer
		mr.declBField("isrestrict", mr.type(TYPEID_ULONG));	// TRUE if restricted pointer (allow agressive opts)
		mr.declBField("size", mr.arrayOf(mr.type(TYPEID_ULONG), 6));		// size of pointer (in bytes)
		mr.declBField("ismocom", mr.type(TYPEID_ULONG));		// TRUE if it is a MoCOM pointer (^ or %)
		mr.declBField("islref", mr.type(TYPEID_ULONG));	// TRUE if it is this pointer of member function with & ref-qualifier
		mr.declBField("isrref", mr.type(TYPEID_ULONG));	// TRUE if it is this pointer of member function with && ref-qualifier
		//mr.declBField("unused", mr.arrayOf(mr.type(TYPEID_ULONG), 10));	// pad out to 32-bits for following cv_typ_t's
	}
	if (mr.NewScope("CV_pmtype_e", SCOPE_ENUM))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("Undef", 0x00); // not specified (pre VC8)
		mr.declEField("D_Single", 0x01); // member data, single inheritance
		mr.declEField("D_Multiple", 0x02); // member data, multiple inheritance
		mr.declEField("D_Virtual", 0x03); // member data, virtual inheritance
		mr.declEField("D_General", 0x04); // member data, most general
		mr.declEField("F_Single", 0x05); // member function, single inheritance
		mr.declEField("F_Multiple", 0x06); // member function, multiple inheritance
		mr.declEField("F_Virtual", 0x07); // member function, virtual inheritance
		mr.declEField("F_General", 0x08); // member function, most general
	}
	if (mr.NewScope("CV_funcattr_t"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declBField("cxxreturnudt", mr.type(TYPEID_UINT8));		// true if C++ style ReturnUDT
		mr.declBField("ctor", mr.type(TYPEID_UINT8));				// true if func is an instance constructor
		mr.declBField("ctorvbase", mr.type(TYPEID_UINT8));		// true if func is an instance constructor of a class with virtual bases
		//mr.declBField("unused", mr.arrayOf(mr.type(TYPEID_UINT8), 5));	// unused
	}
	if (mr.NewScope("CV_call_e", SCOPE_ENUM))//CV_CALL_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("NEAR_C", 0x00); // near right to left push, caller pops stack
		mr.declEField("FAR_C", 0x01); // far right to left push, caller pops stack
		mr.declEField("NEAR_PASCAL", 0x02); // near left to right push, callee pops stack
		mr.declEField("FAR_PASCAL", 0x03); // far left to right push, callee pops stack
		mr.declEField("NEAR_FAST", 0x04); // near left to right push with regs, callee pops stack
		mr.declEField("FAR_FAST", 0x05); // far left to right push with regs, callee pops stack
		mr.declEField("SKIPPED", 0x06); // skipped (unused) call index
		mr.declEField("NEAR_STD", 0x07); // near standard call
		mr.declEField("FAR_STD", 0x08); // far standard call
		mr.declEField("NEAR_SYS", 0x09); // near sys call
		mr.declEField("FAR_SYS", 0x0a); // far sys call
		mr.declEField("THISCALL", 0x0b); // this call (this passed in register)
		mr.declEField("MIPSCALL", 0x0c); // Mips call
		mr.declEField("GENERIC", 0x0d); // Generic call sequence
		mr.declEField("ALPHACALL", 0x0e); // Alpha call
		mr.declEField("PPCCALL", 0x0f); // PPC call
		mr.declEField("SHCALL", 0x10); // Hitachi SuperH call
		mr.declEField("ARMCALL", 0x11); // ARM call
		mr.declEField("AM33CALL", 0x12); // AM33 call
		mr.declEField("TRICALL", 0x13); // TriCore Call
		mr.declEField("SH5CALL", 0x14); // Hitachi SuperH-5 call
		mr.declEField("M32RCALL", 0x15); // M32R Call
		mr.declEField("CLRCALL", 0x16); // clr call
		mr.declEField("INLINE", 0x17); // Marker for routines always inlined and thus lacking a convention
		mr.declEField("NEAR_VECTOR", 0x18); // near left to right push with regs, callee pops stack
		mr.declEField("RESERVED", 0x19);  // first unused call enumeration
	}
	if (mr.NewScope("CV_SEPCODEFLAGS"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declBField("fIsLexicalScope", mr.type(TYPEID_ULONG));		// S_SEPCODE doubles as lexical scope
		mr.declBField("fReturnsToParent", mr.type(TYPEID_ULONG));		// code frag returns to parent
		mr.declBField("pad", mr.arrayOf(mr.type(TYPEID_ULONG), 30));	// must be zero
	}
	if (mr.NewScope("CV_VTS_desc_e", SCOPE_ENUM))//CV_VTS_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("near", 0x00);
		mr.declEField("far", 0x01);
		mr.declEField("thin", 0x02);
		mr.declEField("outer", 0x03);
		mr.declEField("meta", 0x04);
		mr.declEField("near32", 0x05);
		mr.declEField("far32", 0x06);
		mr.declEField("unused", 0x07);
	}

	if (mr.NewScope("GSIHashHdr"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("verSignature", mr.type(TYPEID_ULONG));
		mr.declField("verHdr", mr.type(TYPEID_ULONG));
		mr.declField("cbHr", mr.type(TYPEID_ULONG), ATTR_DECIMAL);
		mr.declField("cbBuckets", mr.type(TYPEID_ULONG), ATTR_DECIMAL);
	}

	if (mr.NewScope("HRFile"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("off", mr.type(TYPEID_INT), (AttrIdEnum)ATTR_SYM_OFF_P1);
		mr.declField("cRef", mr.type(TYPEID_INT));
	}

	if (mr.NewScope("PSGSIHDR"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("cbSymHash", mr.type(TYPEID_ULONG));//CB
		mr.declField("cbAddrMap", mr.type(TYPEID_ULONG), ATTR_DECIMAL);//CB
		mr.declField("nThunks", mr.type(TYPEID_UINT), ATTR_DECIMAL);
		mr.declField("cbSizeOfThunk", mr.type(TYPEID_ULONG));//CB
		mr.declField("isectThunkTable", mr.type(TYPEID_USHORT));//SECT
		mr.align(ALIGN_DWORD);
		mr.declField("offThunkTable", mr.type(TYPEID_ULONG));//OFF
		mr.declField("nSects", mr.type(TYPEID_UINT));
	}

	if (mr.NewScope("CV_HREG_e", SCOPE_ENUM))//CV_ALLREG_
	{
		SAFE_SCOPE_HERE(mr);
		//  Register set for the Intel 80x86 and ix86 processor series
		//  (plus PCODE registers)

		//mr.declEField("NONE", 0);
		mr.declEField("AL", 1);
		mr.declEField("CL", 2);
		mr.declEField("DL", 3);
		mr.declEField("BL", 4);
		mr.declEField("AH", 5);
		mr.declEField("CH", 6);
		mr.declEField("DH", 7);
		mr.declEField("BH", 8);
		mr.declEField("AX", 9);
		mr.declEField("CX", 10);
		mr.declEField("DX", 11);
		mr.declEField("BX", 12);
		mr.declEField("SP", 13);
		mr.declEField("BP", 14);
		mr.declEField("SI", 15);
		mr.declEField("DI", 16);
		mr.declEField("EAX", 17);
		mr.declEField("ECX", 18);
		mr.declEField("EDX", 19);
		mr.declEField("EBX", 20);
		mr.declEField("ESP", 21);
		mr.declEField("EBP", 22);
		mr.declEField("ESI", 23);
		mr.declEField("EDI", 24);
		mr.declEField("ES", 25);
		mr.declEField("CS", 26);
		mr.declEField("SS", 27);
		mr.declEField("DS", 28);
		mr.declEField("FS", 29);
		mr.declEField("GS", 30);
		mr.declEField("IP", 31);
		mr.declEField("FLAGS", 32);
		mr.declEField("EIP", 33);
		mr.declEField("EFLAGS", 34);
		mr.declEField("TEMP", 40);          // PCODE Temp
		mr.declEField("TEMPH", 41);          // PCODE TempH
		mr.declEField("QUOTE", 42);          // PCODE Quote
		mr.declEField("PCDR3", 43);          // PCODE reserved
		mr.declEField("PCDR4", 44);          // PCODE reserved
		mr.declEField("PCDR5", 45);          // PCODE reserved
		mr.declEField("PCDR6", 46);          // PCODE reserved
		mr.declEField("PCDR7", 47);          // PCODE reserved
		mr.declEField("CR0", 80);          // CR0 -- control registers
		mr.declEField("CR1", 81);
		mr.declEField("CR2", 82);
		mr.declEField("CR3", 83);
		mr.declEField("CR4", 84);          // Pentium
		mr.declEField("DR0", 90);          // Debug register
		mr.declEField("DR1", 91);
		mr.declEField("DR2", 92);
		mr.declEField("DR3", 93);
		mr.declEField("DR4", 94);
		mr.declEField("DR5", 95);
		mr.declEField("DR6", 96);
		mr.declEField("DR7", 97);
		mr.declEField("GDTR", 110);
		mr.declEField("GDTL", 111);
		mr.declEField("IDTR", 112);
		mr.declEField("IDTL", 113);
		mr.declEField("LDTR", 114);
		mr.declEField("TR", 115);

		mr.declEField("PSEUDO1", 116);
		mr.declEField("PSEUDO2", 117);
		mr.declEField("PSEUDO3", 118);
		mr.declEField("PSEUDO4", 119);
		mr.declEField("PSEUDO5", 120);
		mr.declEField("PSEUDO6", 121);
		mr.declEField("PSEUDO7", 122);
		mr.declEField("PSEUDO8", 123);
		mr.declEField("PSEUDO9", 124);

		mr.declEField("ST0", 128);
		mr.declEField("ST1", 129);
		mr.declEField("ST2", 130);
		mr.declEField("ST3", 131);
		mr.declEField("ST4", 132);
		mr.declEField("ST5", 133);
		mr.declEField("ST6", 134);
		mr.declEField("ST7", 135);
		mr.declEField("CTRL", 136);
		mr.declEField("STAT", 137);
		mr.declEField("TAG", 138);
		mr.declEField("FPIP", 139);
		mr.declEField("FPCS", 140);
		mr.declEField("FPDO", 141);
		mr.declEField("FPDS", 142);
		mr.declEField("ISEM", 143);
		mr.declEField("FPEIP", 144);
		mr.declEField("FPEDO", 145);

		mr.declEField("MM0", 146);
		mr.declEField("MM1", 147);
		mr.declEField("MM2", 148);
		mr.declEField("MM3", 149);
		mr.declEField("MM4", 150);
		mr.declEField("MM5", 151);
		mr.declEField("MM6", 152);
		mr.declEField("MM7", 153);

		mr.declEField("XMM0", 154); // KATMAI registers
		mr.declEField("XMM1", 155);
		mr.declEField("XMM2", 156);
		mr.declEField("XMM3", 157);
		mr.declEField("XMM4", 158);
		mr.declEField("XMM5", 159);
		mr.declEField("XMM6", 160);
		mr.declEField("XMM7", 161);

		/*mr.declEField("XMM00", 162); // KATMAI sub-registers
		mr.declEField("XMM01", 163);
		mr.declEField("XMM02", 164);
		mr.declEField("XMM03", 165);
		mr.declEField("XMM10", 166);
		mr.declEField("XMM11", 167);
		mr.declEField("XMM12", 168);
		mr.declEField("XMM13", 169);
		mr.declEField("XMM20", 170);
		mr.declEField("XMM21", 171);
		mr.declEField("XMM22", 172);
		mr.declEField("XMM23", 173);
		mr.declEField("XMM30", 174);
		mr.declEField("XMM31", 175);
		mr.declEField("XMM32", 176);
		mr.declEField("XMM33", 177);
		mr.declEField("XMM40", 178);
		mr.declEField("XMM41", 179);
		mr.declEField("XMM42", 180);
		mr.declEField("XMM43", 181);
		mr.declEField("XMM50", 182);
		mr.declEField("XMM51", 183);
		mr.declEField("XMM52", 184);
		mr.declEField("XMM53", 185);
		mr.declEField("XMM60", 186);
		mr.declEField("XMM61", 187);
		mr.declEField("XMM62", 188);
		mr.declEField("XMM63", 189);
		mr.declEField("XMM70", 190);
		mr.declEField("XMM71", 191);
		mr.declEField("XMM72", 192);
		mr.declEField("XMM73", 193);

		mr.declEField("XMM0L", 194);
		mr.declEField("XMM1L", 195);
		mr.declEField("XMM2L", 196);
		mr.declEField("XMM3L", 197);
		mr.declEField("XMM4L", 198);
		mr.declEField("XMM5L", 199);
		mr.declEField("XMM6L", 200);
		mr.declEField("XMM7L", 201);

		mr.declEField("XMM0H", 202);
		mr.declEField("XMM1H", 203);
		mr.declEField("XMM2H", 204);
		mr.declEField("XMM3H", 205);
		mr.declEField("XMM4H", 206);
		mr.declEField("XMM5H", 207);
		mr.declEField("XMM6H", 208);
		mr.declEField("XMM7H", 209);*/

		mr.declEField("MXCSR", 211); // XMM status register

		mr.declEField("EDXEAX", 212); // EDX:EAX pair

		mr.declEField("EMM0L", 220); // XMM sub-registers (WNI integer)
		mr.declEField("EMM1L", 221);
		mr.declEField("EMM2L", 222);
		mr.declEField("EMM3L", 223);
		mr.declEField("EMM4L", 224);
		mr.declEField("EMM5L", 225);
		mr.declEField("EMM6L", 226);
		mr.declEField("EMM7L", 227);

		mr.declEField("EMM0H", 228);
		mr.declEField("EMM1H", 229);
		mr.declEField("EMM2H", 230);
		mr.declEField("EMM3H", 231);
		mr.declEField("EMM4H", 232);
		mr.declEField("EMM5H", 233);
		mr.declEField("EMM6H", 234);
		mr.declEField("EMM7H", 235);

		// do not change the order of these regs); first one must be even too
		mr.declEField("MM00", 236);
		mr.declEField("MM01", 237);
		mr.declEField("MM10", 238);
		mr.declEField("MM11", 239);
		mr.declEField("MM20", 240);
		mr.declEField("MM21", 241);
		mr.declEField("MM30", 242);
		mr.declEField("MM31", 243);
		mr.declEField("MM40", 244);
		mr.declEField("MM41", 245);
		mr.declEField("MM50", 246);
		mr.declEField("MM51", 247);
		mr.declEField("MM60", 248);
		mr.declEField("MM61", 249);
		mr.declEField("MM70", 250);
		mr.declEField("MM71", 251);

		/////////////////////////////////////////////////////////////////////
		//taken from LLVM Code/View/SymbolRecordsMapping.cpp
		mr.declEField("XMM8", 252);
		mr.declEField("XMM9", 253);
		mr.declEField("XMM10", 254);
		mr.declEField("XMM11", 255);
		mr.declEField("XMM12", 256);
		mr.declEField("XMM13", 257);
		mr.declEField("XMM14", 258);
		mr.declEField("XMM15", 259);

		mr.declEField("SIL", 324);
		mr.declEField("DIL", 325);
		mr.declEField("BPL", 326);
		mr.declEField("SPL", 327);

		mr.declEField("RAX", 328);
		mr.declEField("RBX", 329);
		mr.declEField("RCX", 330);
		mr.declEField("RDX", 331);
		mr.declEField("RSI", 332);
		mr.declEField("RDI", 333);
		mr.declEField("RBP", 334);
		mr.declEField("RSP", 335);

		mr.declEField("R8", 336);
		mr.declEField("R9", 337);
		mr.declEField("R10", 338);
		mr.declEField("R11", 339);
		mr.declEField("R12", 340);
		mr.declEField("R13", 341);
		mr.declEField("R14", 342);
		mr.declEField("R15", 343);

		mr.declEField("R8B", 344);
		mr.declEField("R9B", 345);
		mr.declEField("R10B", 346);
		mr.declEField("R11B", 347);
		mr.declEField("R12B", 348);
		mr.declEField("R13B", 349);
		mr.declEField("R14B", 350);
		mr.declEField("R15B", 351);

		mr.declEField("R8W", 352);
		mr.declEField("R9W", 353);
		mr.declEField("R10W", 354);
		mr.declEField("R11W", 355);
		mr.declEField("R12W", 356);
		mr.declEField("R13W", 357);
		mr.declEField("R14W", 358);
		mr.declEField("R15W", 359);

		mr.declEField("R8D", 360);
		mr.declEField("R9D", 361);
		mr.declEField("R10D", 362);
		mr.declEField("R11D", 363);
		mr.declEField("R12D", 364);
		mr.declEField("R13D", 365);
		mr.declEField("R14D", 366);
		mr.declEField("R15D", 367);

/*		mr.declEField("YMM0", 252); // AVX registers
		mr.declEField("YMM1", 253);
		mr.declEField("YMM2", 254);
		mr.declEField("YMM3", 255);
		mr.declEField("YMM4", 256);
		mr.declEField("YMM5", 257);
		mr.declEField("YMM6", 258);
		mr.declEField("YMM7", 259);

		mr.declEField("YMM0H", 260);
		mr.declEField("YMM1H", 261);
		mr.declEField("YMM2H", 262);
		mr.declEField("YMM3H", 263);
		mr.declEField("YMM4H", 264);
		mr.declEField("YMM5H", 265);
		mr.declEField("YMM6H", 266);
		mr.declEField("YMM7H", 267);

		mr.declEField("YMM0I0", 268);    // AVX integer registers
		mr.declEField("YMM0I1", 269);
		mr.declEField("YMM0I2", 270);
		mr.declEField("YMM0I3", 271);
		mr.declEField("YMM1I0", 272);
		mr.declEField("YMM1I1", 273);
		mr.declEField("YMM1I2", 274);
		mr.declEField("YMM1I3", 275);
		mr.declEField("YMM2I0", 276);
		mr.declEField("YMM2I1", 277);
		mr.declEField("YMM2I2", 278);
		mr.declEField("YMM2I3", 279);
		mr.declEField("YMM3I0", 280);
		mr.declEField("YMM3I1", 281);
		mr.declEField("YMM3I2", 282);
		mr.declEField("YMM3I3", 283);
		mr.declEField("YMM4I0", 284);
		mr.declEField("YMM4I1", 285);
		mr.declEField("YMM4I2", 286);
		mr.declEField("YMM4I3", 287);
		mr.declEField("YMM5I0", 288);
		mr.declEField("YMM5I1", 289);
		mr.declEField("YMM5I2", 290);
		mr.declEField("YMM5I3", 291);
		mr.declEField("YMM6I0", 292);
		mr.declEField("YMM6I1", 293);
		mr.declEField("YMM6I2", 294);
		mr.declEField("YMM6I3", 295);
		mr.declEField("YMM7I0", 296);
		mr.declEField("YMM7I1", 297);
		mr.declEField("YMM7I2", 298);
		mr.declEField("YMM7I3", 299);

		mr.declEField("YMM0F0", 300);     // AVX floating-point single precise registers
		mr.declEField("YMM0F1", 301);
		mr.declEField("YMM0F2", 302);
		mr.declEField("YMM0F3", 303);
		mr.declEField("YMM0F4", 304);
		mr.declEField("YMM0F5", 305);
		mr.declEField("YMM0F6", 306);
		mr.declEField("YMM0F7", 307);
		mr.declEField("YMM1F0", 308);
		mr.declEField("YMM1F1", 309);
		mr.declEField("YMM1F2", 310);
		mr.declEField("YMM1F3", 311);
		mr.declEField("YMM1F4", 312);
		mr.declEField("YMM1F5", 313);
		mr.declEField("YMM1F6", 314);
		mr.declEField("YMM1F7", 315);
		mr.declEField("YMM2F0", 316);
		mr.declEField("YMM2F1", 317);
		mr.declEField("YMM2F2", 318);
		mr.declEField("YMM2F3", 319);
		mr.declEField("YMM2F4", 320);
		mr.declEField("YMM2F5", 321);
		mr.declEField("YMM2F6", 322);
		mr.declEField("YMM2F7", 323);
		mr.declEField("YMM3F0", 324);
		mr.declEField("YMM3F1", 325);
		mr.declEField("YMM3F2", 326);
		mr.declEField("YMM3F3", 327);
		mr.declEField("YMM3F4", 328);
		mr.declEField("YMM3F5", 329);
		mr.declEField("YMM3F6", 330);
		mr.declEField("YMM3F7", 331);
		mr.declEField("YMM4F0", 332);
		mr.declEField("YMM4F1", 333);
		mr.declEField("YMM4F2", 334);
		mr.declEField("YMM4F3", 335);
		mr.declEField("YMM4F4", 336);
		mr.declEField("YMM4F5", 337);
		mr.declEField("YMM4F6", 338);
		mr.declEField("YMM4F7", 339);
		mr.declEField("YMM5F0", 340);
		mr.declEField("YMM5F1", 341);
		mr.declEField("YMM5F2", 342);
		mr.declEField("YMM5F3", 343);
		mr.declEField("YMM5F4", 344);
		mr.declEField("YMM5F5", 345);
		mr.declEField("YMM5F6", 346);
		mr.declEField("YMM5F7", 347);
		mr.declEField("YMM6F0", 348);
		mr.declEField("YMM6F1", 349);
		mr.declEField("YMM6F2", 350);
		mr.declEField("YMM6F3", 351);
		mr.declEField("YMM6F4", 352);
		mr.declEField("YMM6F5", 353);
		mr.declEField("YMM6F6", 354);
		mr.declEField("YMM6F7", 355);
		mr.declEField("YMM7F0", 356);
		mr.declEField("YMM7F1", 357);
		mr.declEField("YMM7F2", 358);
		mr.declEField("YMM7F3", 359);
		mr.declEField("YMM7F4", 360);
		mr.declEField("YMM7F5", 361);
		mr.declEField("YMM7F6", 362);
		mr.declEField("YMM7F7", 363);

		mr.declEField("YMM0D0", 364);    // AVX floating-point double precise registers
		mr.declEField("YMM0D1", 365);
		mr.declEField("YMM0D2", 366);
		mr.declEField("YMM0D3", 367);
		mr.declEField("YMM1D0", 368);
		mr.declEField("YMM1D1", 369);
		mr.declEField("YMM1D2", 370);
		mr.declEField("YMM1D3", 371);
		mr.declEField("YMM2D0", 372);
		mr.declEField("YMM2D1", 373);
		mr.declEField("YMM2D2", 374);
		mr.declEField("YMM2D3", 375);
		mr.declEField("YMM3D0", 376);
		mr.declEField("YMM3D1", 377);
		mr.declEField("YMM3D2", 378);
		mr.declEField("YMM3D3", 379);
		mr.declEField("YMM4D0", 380);
		mr.declEField("YMM4D1", 381);
		mr.declEField("YMM4D2", 382);
		mr.declEField("YMM4D3", 383);
		mr.declEField("YMM5D0", 384);
		mr.declEField("YMM5D1", 385);
		mr.declEField("YMM5D2", 386);
		mr.declEField("YMM5D3", 387);
		mr.declEField("YMM6D0", 388);
		mr.declEField("YMM6D1", 389);
		mr.declEField("YMM6D2", 390);
		mr.declEField("YMM6D3", 391);
		mr.declEField("YMM7D0", 392);
		mr.declEField("YMM7D1", 393);
		mr.declEField("YMM7D2", 394);
		mr.declEField("YMM7D3", 395);

		mr.declEField("BND0", 396);
		mr.declEField("BND1", 397);
		mr.declEField("BND2", 398);
		mr.declEField("BND3", 399);*/
	}
}

void PDB20_CreateStructures(I_SuperModule &mr)
{
	if (mr.EnterAttic())
	{
		SAFE_SCOPE_HERE(mr);
		PDB20_CreateStructures_0(mr);
		PDB_CreateStructures_MOD(mr);
		PDB_CreateStructures_TPI(mr);
	}
}

void PDB70_CreateStructures(I_SuperModule &mr)
{
	if (mr.EnterAttic())
	{
		SAFE_SCOPE_HERE(mr);
		PDB70_CreateStructures_0(mr);
		PDB_CreateStructures_MOD(mr);
		PDB_CreateStructures_TPI(mr);
		//PDB70_DeclareDynamicTypes(mr);
	}
}

