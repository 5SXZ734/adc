#include "format.pdb.h"
#include "shared.h"
#include "PDB.h"
#include "cv/cvinfo.h"

using namespace pdb;


/////////////////////////////////////////////////////////////
BEGIN_DYNAMIC_TYPE(ModInfo)
{
	mr.declField("Unused1", mr.type(TYPEID_UINT32));
	mr.declField("SectionContr", mr.type("SectionContribEntry"));
	mr.declField("Flags", mr.type(TYPEID_UINT16));
	mr.declField("ModuleSymStream", mr.type(TYPEID_UINT16), (AttrIdEnum)ATTR_STREAM_INDEX);
	mr.declField("SymByteSize", mr.type(TYPEID_UINT32));
	mr.declField("C11ByteSize", mr.type(TYPEID_UINT32));
	mr.declField("C13ByteSize", mr.type(TYPEID_UINT32));
	mr.declField("SourceFileCount", mr.type(TYPEID_UINT16));
	mr.align(ALIGN_DWORD);//char Padding[2];
	mr.declField("Unused2", mr.type(TYPEID_UINT32));
	mr.declField("SourceFileNameIndex", mr.type(TYPEID_UINT32));
	mr.declField("PdbFilePathNameIndex", mr.type(TYPEID_UINT32));
	mr.declField("ModuleName", toAscii(mr));
	mr.declField("ObjFileName", toAscii(mr));
	mr.align(ALIGN_DWORD);
}
END_DYNAMIC_TYPE(ModInfo);



///////////////////////////////////////////////////////////// CodeView_Base

class CodeView_Base : public I_DynamicType
{
protected:
	void declName(I_Module& mr, bool bLengthPfxd)
	{
		HTYPE hType(bLengthPfxd ? toAsciiPre(mr) : toAscii(mr));
		mr.declField("name", hType);
	}
	void declName(I_Module& mr, bool bLengthPfxd, POSITION end)
	{
		declName(mr, bLengthPfxd);
		if (mr.cp() < end)
		{
			DECLDATA(uint8_t, pad);
			if (pad < LF_PAD0)
			{
				HTYPE hType(bLengthPfxd ? toAsciiPre(mr) : toAscii(mr));
				mr.declField("mangled?", hType);
			}
		}
	}
	bool declNumeric0(I_Module& mr, uint16_t leaf, const char* name)
	{
		OpType_t t;
		char buf[32] = { 0 };
		if (name)
			strcpy(buf, name);
		switch (leaf)
		{
		case LF_CHAR: strcat(buf, "_char"); t = TYPEID_CHAR; break;
		case LF_SHORT: strcat(buf, "_short"); t = TYPEID_SHORT; break;
		case LF_USHORT: strcat(buf, "_ushort"); t = TYPEID_USHORT; break;
		case LF_LONG: strcat(buf, "_long"); t = TYPEID_LONG; break;
		case LF_ULONG: strcat(buf, "_ulong"); t = TYPEID_ULONG; break;
		case LF_QUADWORD: strcat(buf, "_quadword"); t = TYPEID_LONGLONG; break;
		case LF_UQUADWORD: strcat(buf, "_uquadword"); t = TYPEID_ULONGLONG; break;
		default: return false;
		}
		mr.declField(buf, mr.type(t));
		return true;
	}
	bool declNumeric(I_Module& mr, const char* name)
	{
		DECLDATA(uint16_t, leaf);
		if (leaf < LF_NUMERIC)
		{
			mr.declField(name ? name : "value", mr.type(TYPEID_UINT16));
			return true;
		}
		mr.declField("leaf", mr.enumOf(mr.type("LEAF_ENUM_e"), TYPEID_UINT16));
		return declNumeric0(mr, leaf, name);
	}
	bool declSize(I_Module& mr, const char* name)
	{
		DECLDATA(uint16_t, sz);
		if (sz < LF_NUMERIC)
		{
			mr.declField(name ? name : "size", mr.type(TYPEID_UINT16));
			return true;
		}
		std::string s(name ? name : "size");
		s.append("_def");
		mr.declField(s.c_str(), mr.type(TYPEID_UINT16));
		return declNumeric0(mr, sz, name);
	}
	void declType(I_Module &mr, const char *name, unsigned uArray = 0)//leaf
	{
		HTYPE p(mr.enumOf(mr.type("TYPE_ENUM_e"), TYPEID_CV_TYP));
		if (uArray > 0)
			p = mr.arrayOf(p, uArray);
		mr.declField(name, p, (AttrIdEnum)ATTR_PDB_TYPEOFF);
	}
	void declFldAttr(I_Module& mr)
	{
		mr.declField("attr", mr.type("CV_fldattr_t"), ATTR_COLLAPSED);
	}
	static bool isLengthPfxd(LEAF_ENUM_e leaf)
	{
		switch (leaf)
		{
		case LF_ENUMERATE_ST:
		case LF_ENUM_ST:
		case LF_STRUCTURE_ST:
		case LF_MEMBER_ST:
		case LF_UNION_ST:
		case LF_NESTTYPE_ST:
		case LF_ONEMETHOD_ST:
		case LF_METHOD_ST:
		case LF_STMEMBER_ST:
		case LF_ARRAY_ST:
		case LF_CLASS_ST:
			//case LF_STRING_ID_ST:
			//case LF_FUNC_ID_ST:
			//case LF_MFUNC_ID_ST:
			return true;
		default:
			break;
		}
		return false;
	}
	static bool isLengthPfxd(SYM_ENUM_e rectyp)
	{
		switch (rectyp)
		{
		case S_COMPILE:
		case S_COMPILE2_ST:
		case S_OBJNAME_ST:
		case S_GPROC32_ST:
		case S_LPROC32_ST:
		case S_REGISTER_ST:
		case S_LDATA32_ST:
		case S_GDATA32_ST:
		case S_LMANDATA_ST:
		case S_GMANDATA_ST:
		case S_LABEL32_ST:
		case S_PROCREF_ST:
		case S_DATAREF_ST:
		case S_LPROCREF_ST:
		case S_PUB32_ST:
		case S_BPREL32_ST:
		case S_UDT_ST:
		case S_THUNK32_ST:
		case S_CONSTANT_ST:
			return true;
		}
		return false;
	}
};



///////////////////////////////////////////////////////////// CodeView_SymbolBase

class CodeView_SymbolBase : public CodeView_Base
{
protected:
	void ON_EXPORTSYM(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("ordinal", mr.type(TYPEID_USHORT));
		mr.declBField("fConstant", mr.type(TYPEID_USHORT));      // CONSTANT
		mr.declBField("fData", mr.type(TYPEID_USHORT));          // DATA
		mr.declBField("fPrivate", mr.type(TYPEID_USHORT));       // PRIVATE
		mr.declBField("fNoName", mr.type(TYPEID_USHORT));        // NONAME
		mr.declBField("fOrdinal", mr.type(TYPEID_USHORT));       // Ordinal was explicitly assigned
		mr.declBField("fForwarder", mr.type(TYPEID_USHORT));     // This is a forwarder
		mr.declBField("reserved", mr.arrayOf(mr.type(TYPEID_USHORT), 10));	// Reserved. Must be zero.
		declName(mr, isLengthPfxd(rectyp));
	}
	void ON_OBJNAME(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("signature", mr.type(TYPEID_UINT32));
		declName(mr, isLengthPfxd(rectyp));
	}
	void ON_COMPILE(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		if (rectyp == S_COMPILE)
		{
			mr.declField("machine", mr.type(TYPEID_BYTE));    // target processor
			if (mr.NewScope(mr.declField("flags", ATTR_COLLAPSED)))//ATTR_COLLAPSED doesn't work!
			{
				SAFE_SCOPE_HERE(mr);
				mr.declBField("language", mr.arrayOf(mr.type(TYPEID_UINT8), 8));// : 8; // language index
				mr.declBField("pcode", mr.type(TYPEID_UINT8));// : 1; // true if pcode present
				mr.declBField("floatprec", mr.arrayOf(mr.type(TYPEID_UINT8), 2));// : 2; // floating precision
				mr.declBField("floatpkg", mr.arrayOf(mr.type(TYPEID_UINT8), 2));// : 2; // float package
				mr.declBField("ambdata", mr.arrayOf(mr.type(TYPEID_UINT8), 3));// : 3; // ambient data model
				mr.declBField("ambcode", mr.arrayOf(mr.type(TYPEID_UINT8), 3));// : 3; // ambient code model
				mr.declBField("mode32", mr.type(TYPEID_UINT8));// : 1; // true if compiled 32 bit mode
				mr.declBField("pad", mr.arrayOf(mr.type(TYPEID_UINT8), 4));// : 4; // reserved
			}
		}
		else
		{
			if (mr.NewScope(mr.declField("flags", ATTR_COLLAPSED)))//ATTR_COLLAPSED doesn't work!
			{
				//mr.installNamespace();
				mr.declBField("iLanguage", mr.arrayOf(mr.type(TYPEID_ULONG), 8));   // language index
				mr.declBField("fEC", mr.type(TYPEID_ULONG));				// compiled for E/C
				mr.declBField("fNoDbgInfo", mr.type(TYPEID_ULONG));		// not compiled with debug info
				mr.declBField("fLTCG", mr.type(TYPEID_ULONG));			// compiled with LTCG
				mr.declBField("fNoDataAlign", mr.type(TYPEID_ULONG));		// compiled with -Bzalign
				mr.declBField("fManagedPresent", mr.type(TYPEID_ULONG));	// managed code/data present
				mr.declBField("fSecurityChecks", mr.type(TYPEID_ULONG));	// compiled with /GS
				mr.declBField("fHotPatch", mr.type(TYPEID_ULONG));		// compiled with /hotpatch
				mr.declBField("fCVTCIL", mr.type(TYPEID_ULONG));			// converted with CVTCIL
				mr.declBField("fMSILModule", mr.type(TYPEID_ULONG));		// MSIL netmodule
				if (rectyp == S_COMPILE3)
				{
					mr.declBField("fSdl", mr.type(TYPEID_ULONG));		// compiled with /sdl
					mr.declBField("fPGO", mr.type(TYPEID_ULONG));		// compiled with /ltcg:pgo or pgu
					mr.declBField("fExp", mr.type(TYPEID_ULONG));		// .exp module
				}
				mr.declBField("pad", mr.arrayOf(mr.type(TYPEID_ULONG), 12));   // reserved, must be 0
				mr.Leave();
			}
			mr.declField("machine", mr.type(TYPEID_USHORT));    // target processor
			mr.declField("verFEMajor", mr.type(TYPEID_USHORT)); // front end major version #
			mr.declField("verFEMinor", mr.type(TYPEID_USHORT)); // front end minor version #
			mr.declField("verFEBuild", mr.type(TYPEID_USHORT)); // front end build version #
			if (rectyp == S_COMPILE3)
				mr.declField("verFEQFE", mr.type(TYPEID_USHORT));   // front end QFE version #
			mr.declField("verMajor", mr.type(TYPEID_USHORT));   // back end major version #
			mr.declField("verMinor", mr.type(TYPEID_USHORT));   // back end minor version #
			mr.declField("verBuild", mr.type(TYPEID_USHORT));   // back end build version #
			if (rectyp == S_COMPILE3)
				mr.declField("verQFE", mr.type(TYPEID_USHORT));	//* back end QFE version #
		}
		declName(mr, isLengthPfxd(rectyp));   // compiler version string
	}
	void ON_PROCSYM32(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("pParent", mr.type(TYPEID_ULONG));    // pointer to the parent
		mr.declField("pEnd", mr.type(TYPEID_ULONG), ATTR_OFFS);       // pointer to this blocks end
		mr.declField("pNext", mr.type(TYPEID_ULONG));      // pointer to next symbol
		mr.declField("len", mr.type(TYPEID_ULONG));        // Proc length
		mr.declField("DbgStart", mr.type(TYPEID_ULONG));   // Debug start offset
		mr.declField("DbgEnd", mr.type(TYPEID_ULONG));     // Debug end offset
		declType(mr, "typind");     // Type index or ID (CV_typ_t)
		mr.declField("off", mr.type(TYPEID_ULONG));		   // CV_uoff32_t
		mr.declField("seg", mr.type(TYPEID_USHORT));
		mr.declField("flags", mr.type("CV_PROCFLAGS"));      // Proc flags (CV_PROCFLAGS)
		declName(mr, isLengthPfxd(rectyp));
	}
	void ON_DATASYM32(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		declType(mr, "typind");     // Type index, or Metadata token if a managed symbol
		mr.declField("off", mr.type(TYPEID_CV_UOFF32));
		mr.declField("seg", mr.type(TYPEID_USHORT));
		declName(mr, isLengthPfxd(rectyp));    // name 
	}
	void ON_FRAMEPROCSYM(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("cbFrame", mr.type(TYPEID_ULONG));    // count of bytes of total frame of procedure
		mr.declField("cbPad", mr.type(TYPEID_ULONG));      // count of bytes of padding in the frame
		mr.declField("offPad", mr.type(TYPEID_CV_UOFF32));     // offset (relative to frame poniter) to where padding starts
		mr.declField("cbSaveRegs", mr.type(TYPEID_ULONG)); // count of bytes of callee save registers
		mr.declField("offExHdlr", mr.type(TYPEID_CV_UOFF32));  // offset of exception handler
		mr.declField("sectExHdlr", mr.type(TYPEID_USHORT)); // section id of exception handler

		//mr.declField("flags", mr.type(TYPEID_ULONG));
		if (mr.NewScope(mr.declField("flags", ATTR_COLLAPSED)))//ATTR_COLLAPSED doesn't work!
		{
			//mr.installNamespace();
			mr.declBField("fHasAlloca", mr.type(TYPEID_ULONG));	// function uses _alloca()
			mr.declBField("fHasSetJmp", mr.type(TYPEID_ULONG));	// function uses setjmp()
			mr.declBField("fHasLongJmp", mr.type(TYPEID_ULONG));	// function uses longjmp()
			mr.declBField("fHasInlAsm", mr.type(TYPEID_ULONG));	// function uses inline asm
			mr.declBField("fHasEH", mr.type(TYPEID_ULONG));		// function has EH states
			mr.declBField("fInlSpec", mr.type(TYPEID_ULONG));		// function was speced as inline
			mr.declBField("fHasSEH", mr.type(TYPEID_ULONG));		// function has SEH
			mr.declBField("fNaked", mr.type(TYPEID_ULONG));		// function is __declspec(naked)
			mr.declBField("fSecurityChecks", mr.type(TYPEID_ULONG));	// function has buffer security check introduced by /GS.
			mr.declBField("fAsyncEH", mr.type(TYPEID_ULONG));			// function compiled with /EHa
			mr.declBField("fGSNoStackOrdering", mr.type(TYPEID_ULONG));	// function has /GS buffer checks, but stack ordering couldn't be done
			mr.declBField("fWasInlined", mr.type(TYPEID_ULONG));			// function was inlined within another function
			mr.declBField("fGSCheck", mr.type(TYPEID_ULONG));				// function is __declspec(strict_gs_check)
			mr.declBField("fSafeBuffers", mr.type(TYPEID_ULONG));			// function is __declspec(safebuffers)
			mr.declBField("encodedLocalBasePointer", mr.arrayOf(mr.type(TYPEID_ULONG), 2));	// record function's local pointer explicitly.
			mr.declBField("encodedParamBasePointer", mr.arrayOf(mr.type(TYPEID_ULONG), 2));  // record function's parameter pointer explicitly.
			mr.declBField("fPogoOn", mr.type(TYPEID_ULONG));			// function was compiled with PGO/PGU
			mr.declBField("fValidCounts", mr.type(TYPEID_ULONG));		// Do we have valid Pogo counts?
			mr.declBField("fOptSpeed", mr.type(TYPEID_ULONG));		// Did we optimize for speed?
			mr.declBField("fGuardCF", mr.type(TYPEID_ULONG));			// function contains CFG checks (and no write checks)
			mr.declBField("fGuardCFW", mr.type(TYPEID_ULONG));		// function contains CFW checks and/or instrumentation
			mr.declBField("pad", mr.arrayOf(mr.type(TYPEID_ULONG), 9));   // reserved, must be 0
			mr.Leave();
		}
	}
	void ON_GMANPROC(I_Module& mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("pParent", mr.type(TYPEID_ULONG));		// pointer to the parent
		mr.declField("pEnd", mr.type(TYPEID_ULONG));		// pointer to this blocks end
		mr.declField("pNext", mr.type(TYPEID_ULONG));		// pointer to next symbol
		mr.declField("len", mr.type(TYPEID_ULONG));			// Proc length
		mr.declField("DbgStart", mr.type(TYPEID_ULONG));	// Debug start offset
		mr.declField("DbgEnd", mr.type(TYPEID_ULONG));		// Debug end offset
		mr.declField("token", mr.type(TYPEID_CV_TKN));      // COM+ metadata token for method
		mr.declField("off", mr.type(TYPEID_CV_UOFF32));
		mr.declField("seg", mr.type(TYPEID_USHORT));
		mr.declField("flags", mr.type("CV_PROCFLAGS"));		// Proc flags
		mr.declField("retReg", mr.type(TYPEID_USHORT));     // Register return value is in (may not be used for all archs)
		declName(mr, isLengthPfxd(rectyp));    // optional name field	
	}
	void ON_MANSLOT(I_Module& mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("iSlot", mr.type(TYPEID_ULONG));		// slot index
		declType(mr, "typind");								// Type index or Metadata token
		mr.declField("attr", mr.type("CV_lvar_attr"));       // local var attributes
		declName(mr, isLengthPfxd(rectyp));
	}
	void ON_GTHREAD32(I_Module& mr, SYM_ENUM_e rectyp, POSITION end)
	{
		declType(mr, "typind");								// type index
		mr.declField("off", mr.type(TYPEID_CV_UOFF32));		// offset into thread storage
		mr.declField("seg", mr.type(TYPEID_USHORT));		// segment of thread storage
		declName(mr, isLengthPfxd(rectyp));					// length prefixed name
	}
	void ON_HEAPALLOCSITE(I_Module& mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("off", mr.type(TYPEID_CV_UOFF32));		// offset of call site
		mr.declField("sect", mr.type(TYPEID_USHORT));		// section index of call site
		mr.declField("cbInstr", mr.type(TYPEID_USHORT));	// length of heap allocation call instruction
		declType(mr, "typind");								// type index describing function signature
	}
	void ON_TRAMPOLINE(I_Module& mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("trampType", mr.type(TYPEID_USHORT));		// trampoline sym subtype
		mr.declField("cbThunk", mr.type(TYPEID_USHORT));		// size of the thunk
		mr.declField("offThunk", mr.type(TYPEID_CV_UOFF32));	// offset of the thunk
		mr.declField("offThunk", mr.type(TYPEID_CV_UOFF32));	// offset of the target of the thunk
		mr.declField("sectThunk", mr.type(TYPEID_USHORT));		// section index of the thunk
		mr.declField("sectTarget", mr.type(TYPEID_USHORT));		// section index of the target of the thunk
	}
	void ON_SECTION(I_Module& mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("isec", mr.type(TYPEID_USHORT));			// Section number
		mr.declField("align", mr.type(TYPEID_UINT8));			// Alignment of this section (power of 2)
		mr.declField("bReserved", mr.type(TYPEID_UINT8));		// Reserved.  Must be zero.
		mr.declField("rva", mr.type(TYPEID_ULONG));
		mr.declField("cb", mr.type(TYPEID_ULONG));
		mr.declField("characteristics", mr.type(TYPEID_ULONG));
		declName(mr, isLengthPfxd(rectyp));						// name
	}
	void ON_COFFGROUP(I_Module& mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("cb", mr.type(TYPEID_ULONG));
		mr.declField("characteristics", mr.type(TYPEID_ULONG));
		mr.declField("off", mr.type(TYPEID_CV_UOFF32));			// Symbol offset
		mr.declField("seg", mr.type(TYPEID_USHORT));			// Symbol segment
		declName(mr, isLengthPfxd(rectyp));						// name
	}
	void ON_ENVBLOCK(I_Module& mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("flags", mr.type(TYPEID_UINT8));
		for (;;)
		{
			DECLDATA(unsigned char, uChar);
			mr.declField("rgsz", toAscii(mr));
			if (uChar == 0)
				break;
		}
	}
	void ON_REGREL32(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("off", mr.type(TYPEID_CV_UOFF32));        // offset of symbol
		declType(mr, "typind");     // Type index or metadata token
		mr.declField("reg", mr.enumOf(mr.type("CV_HREG_e"), TYPEID_USHORT));        // register index for symbol
		declName(mr, isLengthPfxd(rectyp));		// name
	}
	void ON_CALLSITEINFO(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("off", mr.type(TYPEID_CV_UOFF32));                // offset of call site
		mr.declField("sect", mr.type(TYPEID_USHORT));               // section index of call site
		mr.skip(sizeof(unsigned short)); // alignment padding field, must be zero
		declType(mr, "typind");             // type index describing function signature
	}
	void ON_BLOCKSYM32(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("pParent", mr.type(TYPEID_ULONG));    // pointer to the parent
		mr.declField("pEnd", mr.type(TYPEID_ULONG));       // pointer to this blocks end
		mr.declField("len", mr.type(TYPEID_ULONG));        // Block length
		mr.declField("off", mr.type(TYPEID_CV_UOFF32));		// Offset in code segment
		mr.declField("seg", mr.type(TYPEID_USHORT));      // segment of label
		declName(mr, isLengthPfxd(rectyp));    // name
	}
	void ON_THUNKSYM32(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("pParent", mr.type(TYPEID_ULONG));    // pointer to the parent
		mr.declField("pEnd", mr.type(TYPEID_ULONG));       // pointer to this blocks end
		mr.declField("pNext", mr.type(TYPEID_ULONG));      // pointer to next symbol
		mr.declField("off", mr.type(TYPEID_CV_UOFF32));
		mr.declField("seg", mr.type(TYPEID_USHORT));
		mr.declField("len", mr.type(TYPEID_USHORT));        // length of thunk
		mr.declField("ord", mr.type(TYPEID_UINT8));        // THUNK_ORDINAL specifying type of thunk
		declName(mr, isLengthPfxd(rectyp));    // name
		//mr.declField("variant", mr.arrayOf(mr.type(TYPEID_UINT8), CV_ZEROLEN); // variant portion of thunk 
	}
	void ON_LABELSYM32(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("off", mr.type(TYPEID_CV_UOFF32));
		mr.declField("seg", mr.type(TYPEID_USHORT));
		mr.declField("flags", mr.type(TYPEID_BYTE));		//CV_PROCFLAGS      // flags
		declName(mr, isLengthPfxd(rectyp));		// name
	}
	void ON_BPRELSYM32(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("off", mr.type(TYPEID_CV_UOFF32));        // BP-relative offset
		declType(mr, "typind");     // Type index or Metadata token
		declName(mr, isLengthPfxd(rectyp));    // name
	}
	void ON_REGSYM(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		declType(mr, "typind");     // Type index or Metadata token
		mr.declField("reg", mr.enumOf(mr.type("CV_HREG_e"), TYPEID_USHORT));        // register enumerate
		declName(mr, isLengthPfxd(rectyp));
	}
	void ON_REFSYM(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("sumName", mr.type(TYPEID_ULONG));    // SUC of the name
		mr.declField("ibSym", mr.type(TYPEID_ULONG));      // Offset of actual symbol in $$Symbols
		mr.declField("imod", mr.type(TYPEID_USHORT), AttrIdEnum(ATTR_MODULE_INDEX));       // Module containing the actual symbol
		//mr.declField("usFill", mr.type(TYPEID_USHORT));     // align this record
		mr.align(ALIGN_DWORD);
		declName(mr, isLengthPfxd(rectyp));
	}
	void ON_REFSYM2(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("sumName", mr.type(TYPEID_ULONG));	// SUC of the name
		mr.declField("ibSym", mr.type(TYPEID_ULONG));	// Offset of actual symbol in $$Symbols
		mr.declField("imod", mr.type(TYPEID_USHORT), AttrIdEnum(ATTR_MODULE_INDEX));	// Module containing the actual symbol
		declName(mr, isLengthPfxd(rectyp));	// hidden name made a first class member
	}
	void ON_PUBSYM32(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("pubsymflags", mr.type("CV_pubsymflag_t"));
		mr.declField("off", mr.type(TYPEID_UINT32));
		mr.declField("seg", mr.type(TYPEID_UINT16));
		declName(mr, isLengthPfxd(rectyp));
	}
	void ON_BUILDINFOSYM(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("id", mr.type(TYPEID_CV_ITEMID)); // CV_ItemId of Build Info.
	}
	void ON_UDTSYM(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		declType(mr, "typind");     // Type index
		declName(mr, isLengthPfxd(rectyp));    // name
	}
	void ON_FRAMECOOKIE(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("off", mr.type(TYPEID_CV_UOFF32));	// Frame relative offset
		mr.declField("reg", mr.enumOf(mr.type("CV_HREG_e"), TYPEID_USHORT));	// Register index
		mr.declField("cookietype", mr.enumOf(mr.type("CV_cookietype_e"), OPTYP_UINT8));	// Type of the cookie
		mr.declField("flags", mr.type(TYPEID_UINT8));			// Flags describing this cookie
	}
	void ON_LOCALSYM(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		declType(mr, "typind");     // type index   
		mr.declField("flags", mr.type("CV_LVARFLAGS"));      // local var flags
		declName(mr, isLengthPfxd(rectyp));   // Name of this symbol, a null terminated array of UTF8 characters.
	}
	void ON_DEFRANGESYMREGISTER(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)// A live range of en-registed variable
	{
		mr.declField("reg", mr.enumOf(mr.type("CV_HREG_e"), TYPEID_USHORT));	// Register to hold the value of the symbol
		mr.declField("attr", mr.type("CV_RANGEATTR"));	// Attribute of the register range.
		mr.declField("range", mr.type("CV_LVAR_ADDR_RANGE"));	// Range of addresses where this program is valid
		//?mr.declField("gaps", mr.arrayOf(mr.type("CV_LVAR_ADDR_GAP"), 1));  // The value is not available in following gaps. 
	}
	void ON_SEPCODESYM(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("pParent", mr.type(TYPEID_ULONG));    // pointer to the parent
		mr.declField("pEnd", mr.type(TYPEID_ULONG));       // pointer to this block's end
		mr.declField("length", mr.type(TYPEID_ULONG));     // count of bytes of this block
		mr.declField("scf", mr.type("CV_SEPCODEFLAGS"));        // flags
		mr.declField("off", mr.type(TYPEID_CV_UOFF32));        // sect:off of the separated code
		mr.declField("offParent", mr.type(TYPEID_CV_UOFF32));  // sectParent:offParent of the enclosing scope
		mr.declField("sect", mr.type(TYPEID_USHORT));       //  (proc, block, or sepcode)
		mr.declField("sectParent", mr.type(TYPEID_USHORT));
	}
	void ON_ANNOTATION(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		mr.declField("off", mr.type(TYPEID_CV_UOFF32));
		mr.declField("seg", mr.type(TYPEID_USHORT));
		mr.declField("csz", mr.type(TYPEID_USHORT));        // Count of zero terminated annotation strings
		while (mr.cp() < end)// Sequence of zero terminated annotation strings
		{
			DECLDATA(unsigned char, uChar);
			mr.declField("rgsz", toAscii(mr));
			if (uChar == 0)
				break;
		}
	}
	void ON_CONSTANT(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		declType(mr, "typind");		// Type index (containing enum if enumerate) or metadata token
		//mr.declField("value", mr.type(TYPEID_USHORT));	// numeric leaf containing value
		declNumeric(mr, "value");
		declName(mr, isLengthPfxd(rectyp));				// name
	}
	void ON_UNAMESPACE(I_Module &mr, SYM_ENUM_e rectyp, POSITION end)
	{
		declName(mr, isLengthPfxd(rectyp));				// name
	}
	bool dispatchSymbol(I_Module &mr, SYM_ENUM_e symb, POSITION end)
	{
		switch (symb)
		{
		case S_EXPORT:
			ON_EXPORTSYM(mr, symb, end);
			break;
		case S_OBJNAME_ST:
		case S_OBJNAME:
			ON_OBJNAME(mr, symb, end);
			break;
		case S_COMPILE:
		case S_COMPILE2:
		case S_COMPILE2_ST:
		case S_COMPILE3:
			ON_COMPILE(mr, symb, end);
			break;
		case S_GPROC32:
		case S_LPROC32:
		case S_LPROC32_ST:
		case S_GPROC32_ST:
			ON_PROCSYM32(mr, symb, end);
			break;
		case S_LDATA32:
		case S_GDATA32:
		case S_LMANDATA:
		case S_GMANDATA:
		case S_LDATA32_ST:
		case S_GDATA32_ST:
		case S_LMANDATA_ST:
		case S_GMANDATA_ST:
			ON_DATASYM32(mr, symb, end);
			break;
		case S_FRAMEPROC:
			ON_FRAMEPROCSYM(mr, symb, end);
			break;
		case S_ENVBLOCK:
			ON_ENVBLOCK(mr, symb, end);
			break;
		case S_REGREL32:
			ON_REGREL32(mr, symb, end);
			break;
		case S_CALLSITEINFO:
			ON_CALLSITEINFO(mr, symb, end);
			break;
		case S_BLOCK32:
			ON_BLOCKSYM32(mr, symb, end);
			break;
		case S_THUNK32:
		case S_THUNK32_ST:
			ON_THUNKSYM32(mr, symb, end);
			break;
		case S_LABEL32:
		case S_LABEL32_ST:
			ON_LABELSYM32(mr, symb, end);
			break;
		case S_BPREL32:
		case S_BPREL32_ST:
			ON_BPRELSYM32(mr, symb, end);
			break;
		case S_REGISTER:
		case S_REGISTER_ST:
			ON_REGSYM(mr, symb, end);
			break;
		case S_PROCREF_ST:
		case S_DATAREF_ST:
		case S_LPROCREF_ST:
			ON_REFSYM(mr, symb, end);
			break;
		//?case S_ANNOTATIONREF:
		case S_PROCREF:
		case S_DATAREF:
		case S_LPROCREF:
		case S_TOKENREF:
			ON_REFSYM2(mr, symb, end);
			break;
		case S_PUB32:
		case S_PUB32_ST:
			ON_PUBSYM32(mr, symb, end);
			break;
		case S_BUILDINFO:
			ON_BUILDINFOSYM(mr, symb, end);
			break;
		case S_UDT:
		case S_UDT_ST:
		case S_COBOLUDT:
			ON_UDTSYM(mr, symb, end);
			break;
		case S_FRAMECOOKIE:
			ON_FRAMECOOKIE(mr, symb, end);
			break;
		case S_LOCAL:
			ON_LOCALSYM(mr, symb, end);
			break;
		case S_DEFRANGE_REGISTER:
			ON_DEFRANGESYMREGISTER(mr, symb, end);
			break;
		case S_SEPCODE:
			ON_SEPCODESYM(mr, symb, end);
			break;
		case S_ANNOTATION:
			ON_ANNOTATION(mr, symb, end);
			break;
		case S_CONSTANT:
		case S_CONSTANT_ST:
		case S_MANCONSTANT:
			ON_CONSTANT(mr, symb, end);
			break;
		case S_UNAMESPACE:
			ON_UNAMESPACE(mr, symb, end);
			break;
		case S_END: break;
		case S_GMANPROC:
			ON_GMANPROC(mr, symb, end);
			break;
		case S_MANSLOT:
			ON_MANSLOT(mr, symb, end);
			break;
		case S_LTHREAD32:
		case S_GTHREAD32:
			ON_GTHREAD32(mr, symb, end);
			break;
		case S_HEAPALLOCSITE:
			ON_HEAPALLOCSITE(mr, symb, end);
			break;
		case S_TRAMPOLINE:
			ON_TRAMPOLINE(mr, symb, end);
			break;
		case S_SECTION:
			ON_SECTION(mr, symb, end);
			break;
		case S_COFFGROUP:
			ON_COFFGROUP(mr, symb, end);
			break;
		case S_DEFRANGE_REGISTER_REL:
		case S_DEFRANGE_FRAMEPOINTER_REL_FULL_SCOPE:
		case S_DEFRANGE_FRAMEPOINTER_REL:
		case S_DEFRANGE_SUBFIELD_REGISTER:
		case S_FASTLINK:
		case S_INLINEES:
		case S_INLINESITE:
		case S_INLINESITE_END:
		case S_CALLEES:
			break;//LATER!
		default:
			fprintf(stdout, "Unrecognized record id at %08X\n", (unsigned)mr.cpr());
			return false;
		}
		return true;
	}
};

#define BEGIN_CV_SYMBOL(arg)	BEGIN_DYNAMIC_TYPE_0(arg, CodeView_SymbolBase) 
#define END_CV_SYMBOL(arg)	END_DYNAMIC_TYPE(arg)

BEGIN_CV_SYMBOL(CodeView_SYMBOL)
{
CHECK(mr.cp() >= 0x128b4)
STOP
	DECLDATA(uint16_t, reclen);
	mr.declField("reclen", mr.type(TYPEID_UINT16));
	if (reclen < 2)
		mr.error("invalid symbol");
	POSITION beg(mr.cp());
	POSITION end(beg + reclen);
	DECLDATA(uint16_t, rectyp);
	mr.declField("rectyp", mr.enumOf(mr.type("SYM_ENUM_e"), TYPEID_UINT16));
	if (!dispatchSymbol(mr, (SYM_ENUM_e)rectyp.get(), end))//dispatcher
	{
		mr.skip(end - mr.cp());
	}
	//unsigned iSkip(reclen - sizeof(uint16_t));
	//if (iSkip > 0)
	//mr.skip(iSkip);
	/*POSITION pos(mr.cp());
	if (rectyp == pdb::S_PROCREF_ST
		|| rectyp == pdb::S_LPROCREF_ST)
	{
		DECLDATA(uint8_t, length);
		if (length != 0)
		{
			declName(mr, isLengthPfxd((pdb::SYM_ENUM_e)rectyp.get()));
		}
	}*/

}
END_CV_SYMBOL(CodeView_SYMBOL);





/////////////////////////////////////////// CodeViewSubsection ***
/////////////////////////////////////////// FileChecksums

BEGIN_DYNAMIC_TYPE(CVFileChecksum)
{
	mr.declField("name", mr.type(TYPEID_UINT32), (AttrIdEnum)ATTR_PDB_NAME);           // Index of name in name table.
	DECLDATA(uint8_t, len);
	mr.declField("len", mr.type(TYPEID_UINT8));            // Hash length
	mr.declField("type", mr.type(TYPEID_UINT8));           // Hash type
	if (len > 0)
	{
		int n(len / 8);
		if (len % 16)
			mr.declField("data", mr.arrayOf(mr.type(TYPEID_BYTE), len), ATTR_COLLAPSED);
		else if (n > 1)
			mr.declField("data", mr.arrayOf(mr.type(TYPEID_QWORD), n));
		else
			mr.declField("data", mr.type(TYPEID_QWORD));
	}
}
END_DYNAMIC_TYPE(CVFileChecksum);

BEGIN_DYNAMIC_TYPE(CodeView_D_FILECHKSMS)
{
	mr.declField("sig", mr.enumOf(mr.type("DEBUG_S_SUBSECTION_TYPE"), TYPEID_INT32));
	DECLDATA(int32_t, size);
	mr.declField("size", mr.type(TYPEID_INT32));
	POSITION end(mr.cp() + size);
	while (mr.cp() < end)
	{
		mr.declField("checksum", mr.type(_PFX("CVFileChecksum")));
		mr.align(ALIGN_DWORD);
	}
}
END_DYNAMIC_TYPE(CodeView_D_FILECHKSMS);

BEGIN_DYNAMIC_TYPE(CodeView_D_LINES)
{
	mr.declField("sig", mr.enumOf(mr.type("DEBUG_S_SUBSECTION_TYPE"), TYPEID_INT32));
	mr.declField("size", mr.type(TYPEID_INT32));
	if (mr.NewScope(mr.declField("header")))//CV_LineSection
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("offCon", mr.type(TYPEID_DWORD));
        mr.declField("segCon", mr.type(TYPEID_WORD));
		DECLDATA(uint16_t, flags);
        mr.declField("flags", mr.type(TYPEID_WORD));
        mr.declField("cbCon", mr.type(TYPEID_DWORD));
		if (flags & CV_LINES_HAVE_COLUMNS)
			return;
    }
	if (mr.NewScope(mr.declField("fileBlock")))//CV_Fileblock
	{
		SAFE_SCOPE_HERE(mr);
		DECLDATAEX(CV_Fileblock, fileBlock);
		mr.declField("fileid", mr.type(TYPEID_DWORD), (AttrIdEnum)ATTR_MOD_FILECHKSMS);
		mr.declField("nLines", mr.type(TYPEID_DWORD));
		mr.declField("cbFileBlock", mr.type(TYPEID_DWORD));
		if (fileBlock.nLines > 0)
			mr.declField("lines", mr.arrayOf(mr.type("CV_Line_t"), fileBlock.nLines));
	}
}
END_DYNAMIC_TYPE(CodeView_D_LINES);

/////////////////////////////////////////// CodeViewSubsection_?
BEGIN_DYNAMIC_TYPE(CodeViewSubsection__TBD)
{
	mr.declField("sig", mr.enumOf(mr.type("DEBUG_S_SUBSECTION_TYPE"), TYPEID_INT32));
	mr.declField("size", mr.type(TYPEID_INT32));
}
END_DYNAMIC_TYPE(CodeViewSubsection__TBD);






///////////////////////////////////////////////////////////// CodeView_LeafBase

class CodeView_LeafBase : public CodeView_Base
{
protected:
	void ON_LF_FIELDLIST(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		while (mr.cp() < end)
		{
			if (mr.NewScope(mr.declField("FIELD")))
			{
				SAFE_SCOPE_HERE(mr);
				DECLDATA(uint16_t, leaf);
				mr.declField("leaf", mr.enumOf(mr.type("LEAF_ENUM_e"), TYPEID_UINT16));
				if (!dispatchLeaf(mr, leaf, end))
					break;
				mr.align(ALIGN_DWORD);
			}
		}
	}
	void ON_LF_METHODLIST(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		if (mr.NewScope(mr.declField("list")))
		{
			while (mr.cp() < end)
			{
				mr.declField("property", mr.type("CV_prop_t"), ATTR_COLLAPSED);
				mr.skip(2);
				declType(mr, "index");
			}
			mr.Leave();
		}
	}
	void ON_LF_ENUMERATE(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		declFldAttr(mr);
		if (!declNumeric(mr, nullptr))
			return;
		declName(mr, isLengthPfxd(leaf));
	}
	void ON_LF_ENUM(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		mr.declField("count", mr.type(TYPEID_UINT16));		// count of number of elements in class 
		mr.declField("property", mr.type("CV_prop_t"), ATTR_COLLAPSED);	// property attribute field
		declType(mr, "utype");		// underlying type of the enum
		declType(mr, "field");		// type index of LF_FIELD descriptor list
		declName(mr, isLengthPfxd(leaf), end);	// length prefixed name of enum 
	}
	void ON_LF_MODIFIER(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		declType(mr, "type");
		mr.declField("modifier", mr.type("CV_modifier_t"), ATTR_COLLAPSED);							
	}
	void ON_LF_POINTER(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		declType(mr, "utype");
		DECLDATAEX(lfPointerBody::lfPointerAttr, attr);
		mr.declField("attr", mr.type("CV_PointerAttr_t"), ATTR_COLLAPSED);
		if (attr.ptrmode == CV_PTR_MODE_PMFUNC)
		{
			declType(mr, "pmclass");
			mr.declField("pmenum", mr.enumOf(mr.type("CV_pmtype_e"), TYPEID_UINT16));
		}
	}
	void ON_LF_STRUCTURE(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)// LF_CLASS, LF_STRUCT, LF_INTERFACE
	{
		bool bWTF(leaf == LF_STRUCTURE_WTF || leaf == LF_CLASS_WTF);
		if (!bWTF)
			mr.declField("count", mr.type(TYPEID_UINT16));		// count of number of elements in class
		mr.declField("property", mr.type("CV_prop_t"), ATTR_COLLAPSED);       // property attribute field (prop_t)
		if (bWTF)
			mr.skip(2);//?
		declType(mr, "field");		// type index of LF_FIELD descriptor list
		declType(mr, "derived");	// type index of derived from list if not zero
		declType(mr, "vshape");		// type index of vshape table for this class
		if (!bWTF)
		{
			if (!declSize(mr, "size"))		// length of structure in bytes
				return;
		}
		else
		{
			mr.declField("xz1", mr.type(OPTYP_UINT16));//?
			DECLDATA(unsigned short, xz2);
			mr.declField("xz2", mr.type(OPTYP_UINT16));//?
			if (xz2 & 0x8000)
				mr.declField("xz3", mr.type(OPTYP_UINT16));//?
		}
		declName(mr, isLengthPfxd(leaf), end);
	}
	void ON_LF_MEMBER(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		declFldAttr(mr);	// attribute mask
		declType(mr, "index");		// index of type record for field
		//mr.declField("offset", mr.type(TYPEID_UINT16));
		if (!declSize(mr, "offset"))// variable length offset of field
			return;
		declName(mr, isLengthPfxd(leaf));		// followed by name of field
	}
	void ON_LF_UNION(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		mr.declField("count", mr.type(TYPEID_UINT16));          // count of number of elements in class
		mr.declField("property", mr.type("CV_prop_t"), ATTR_COLLAPSED);       // property attribute field
		declType(mr, "field");          // type index of LF_FIELD descriptor list
		if (!declSize(mr, "size"))			// length of structure in bytes
			return;
		declName(mr, isLengthPfxd(leaf), end);
	}
	void ON_LF_MFUNCTION(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		declType(mr, "rvtype");         // type index of return value
		declType(mr, "classtype");      // type index of containing class
		declType(mr, "thistype");       // type index of this pointer (model specific)
		mr.declField("calltype", mr.enumOf(mr.type("CV_call_e"), TYPEID_UINT8));       // calling convention (call_t)
		mr.declField("funcattr", mr.type("CV_funcattr_t"), ATTR_COLLAPSED);       // attributes
		mr.declField("parmcount", mr.type(TYPEID_UINT16));			// number of parameters
		declType(mr, "arglist");		// type index of argument list
		mr.declField("thisadjust", mr.type(TYPEID_LONG));	// this adjuster (long because pad required anyway) 
	}
	void ON_LF_NESTTYPE(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		mr.skip(2);//pad0;		// internal padding, must be 0
		declType(mr, "index");      // index of nested type definition
		declName(mr, isLengthPfxd(leaf));    // length prefixed type name 
	}
	void ON_LF_ONEMETHOD(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		DECLDATAEX(CV_fldattr_t, attr);
		declFldAttr(mr); // method attribute
		declType(mr, "index");           // index to type record for procedure
		if (attr.mprop == CV_MTintro || attr.mprop == CV_MTpureintro)
			mr.declField("vbaseoff", mr.type(TYPEID_ULONG));// offset in vfunctable if intro virtual
		declName(mr, isLengthPfxd(leaf));								// length prefixed name of method 
	}
	void ON_LF_METHOD(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		mr.declField("count", mr.type(TYPEID_UINT16));	// number of occurrences of function 
		declType(mr, "mList");		// index to LF_METHODLIST record 
		declName(mr, isLengthPfxd(leaf));
	}
	void ON_LF_PROCEDURE(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		declType(mr, "rvtype");	// type index of return value
		mr.declField("calltype", mr.enumOf(mr.type("CV_call_e"), TYPEID_UINT8));	// calling convention (CV_call_t)
		mr.declField("funcattr", mr.type("CV_funcattr_t"), ATTR_COLLAPSED);		// attributes
		mr.declField("parmcount", mr.type(TYPEID_UINT16));			// number of parameters
		declType(mr, "arglist");			// type index of argument list 
	}
	void ON_LF_ARGLIST(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		DECLDATA(unsigned long, count);
		mr.declField("count", mr.type(TYPEID_ULONG));
		if (count > 0)
			declType(mr, "args", count);
	}
	void ON_LF_VFUNCTAB(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		mr.skip(2);		//pad0				// internal padding, must be 0. 
		declType(mr, "type");		// type index of pointer 
	}
	void ON_LF_STMEMBER(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		declFldAttr(mr);
		declType(mr, "index");
		declName(mr, isLengthPfxd(leaf));
	}
	void ON_LF_ARRAY(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		declType(mr, "elemtype");		// type index of element type
		declType(mr, "idxtype");		// type index of indexing type
		if (!declSize(mr, "length"))	// variable length data specifying size in bytes and name 
			return;
		declName(mr, isLengthPfxd(leaf));
	}
	void ON_LF_VBCLASS(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		declFldAttr(mr);
		declType(mr, "index");	// type index of direct virtual base class 
		declType(mr, "vbptr");	// type index of virtual base pointer 
		declNumeric(mr, "offVbp");		// virtual base pointer offset from address point
		declNumeric(mr, "offVbte");		// followed by virtual base offset from vbtable 
	}
	void ON_LF_BCLASS(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		declFldAttr(mr);
		declType(mr, "index");	// type index of base class
		mr.declField("offset", mr.type(TYPEID_UINT16));			// variable length offset of base within class // CAN BE 32 bit or more!!! 
	}
	void ON_LF_UDT_MOD_SRC_LINE(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		declType(mr, "type");	// UDT's type index
		mr.declField("src", mr.type(TYPEID_CV_ITEMID), (AttrIdEnum)ATTR_PDB_NAME);	// index into string table where source file name is saved
		mr.declField("line", mr.type(TYPEID_ULONG), ATTR_DECIMAL);	// line number
		mr.declField("imod", mr.type(TYPEID_USHORT), (AttrIdEnum)ATTR_MODULE_INDEX);	// module that contributes this UDT definition 
	}
	void ON_LF_STRING_ID(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		mr.declField("id", mr.type(TYPEID_CV_ITEMID));	// ID to list of sub string IDs
		declName(mr, isLengthPfxd(leaf));
	}
	void ON_LF_BUILDINFO(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		DECLDATA(unsigned short, count);
		mr.declField("count", mr.type(TYPEID_USHORT));	// number of arguments
		if (count > 0)
			mr.declField("arg", mr.arrayOf(mr.type(TYPEID_CV_ITEMID), count));// arguments as CodeItemId
	}
	void ON_LF_FUNC_ID(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		mr.declField("scopeId", mr.type(TYPEID_CV_ITEMID));		// parent scope of the ID, 0 if global
		declType(mr, "type");	// function type
		declName(mr, isLengthPfxd(leaf));
	}
	void ON_LF_MFUNC_ID(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		declType(mr, "parentType");	// type index of parent
		declType(mr, "type");	// function type
		declName(mr, isLengthPfxd(leaf));
	}
	void ON_LF_VTSHAPE(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		DECLDATA(unsigned short, count);
		mr.declField("count", mr.type(TYPEID_USHORT));	// number of entries in vfunctable 
		for (int i(0); i < count; i++)
			mr.declBField("desc", mr.arrayOf(mr.enumOf(mr.type("CV_VTS_desc_e"), TYPEID_UINT8), 4));
	}
	void ON_LF_INDEX(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		mr.skip(2);				//pad0				// internal padding, must be 0. 
		declType(mr, "index");	// type index of referenced leaf
	}
	void ON_LF_BITFIELD(I_Module &mr, LEAF_ENUM_e leaf, POSITION end)
	{
		declType(mr, "type");		// type of bitfield
		mr.declField("length", mr.type(TYPEID_UINT8));
		mr.declField("position", mr.type(TYPEID_UINT8));
	}
	bool dispatchLeaf(I_Module &mr, uint16_t leaf, POSITION end)
	{
		LEAF_ENUM_e eLeaf((LEAF_ENUM_e)leaf);
		switch (eLeaf)
		{
		case LF_FIELDLIST:
			ON_LF_FIELDLIST(mr, eLeaf, end);
			break;
		case LF_METHODLIST:
			ON_LF_METHODLIST(mr, eLeaf, end);
			break;
		case LF_ENUM:
		case LF_ENUM_ST:
			ON_LF_ENUM(mr, eLeaf, end);
			break;
		case LF_ENUMERATE:
		case LF_ENUMERATE_ST:
			ON_LF_ENUMERATE(mr, eLeaf, end);
			break;
		case LF_MODIFIER:
			ON_LF_MODIFIER(mr, eLeaf, end);
			break;
		case LF_POINTER:
			ON_LF_POINTER(mr, eLeaf, end);
			break;
		case LF_STRUCTURE_WTF:
		case LF_STRUCTURE_ST:
		case LF_STRUCTURE:
		case LF_CLASS_WTF:
		case LF_CLASS_ST:
		case LF_CLASS:
			ON_LF_STRUCTURE(mr, eLeaf, end);
			break;
		case LF_MEMBER:
		case LF_MEMBER_ST:
			ON_LF_MEMBER(mr, eLeaf, end);
			break;
		case LF_UNION:
		case LF_UNION_ST:
			ON_LF_UNION(mr, eLeaf, end);
			break;
		case LF_BCLASS:
			ON_LF_BCLASS(mr, eLeaf, end);
			break;
		case LF_MFUNCTION:
			ON_LF_MFUNCTION(mr, eLeaf, end);
			break;
		case LF_NESTTYPE:
		case LF_NESTTYPE_ST:
			ON_LF_NESTTYPE(mr, eLeaf, end);
			break;
		case LF_ONEMETHOD:
		case LF_ONEMETHOD_ST:
			ON_LF_ONEMETHOD(mr, eLeaf, end);
			break;
		case LF_METHOD:
		case LF_METHOD_ST:
			ON_LF_METHOD(mr, eLeaf, end);
			break;
		case LF_PROCEDURE:
			ON_LF_PROCEDURE(mr, eLeaf, end);
			break;
		case LF_ARGLIST:
			ON_LF_ARGLIST(mr, eLeaf, end);
			break;
		case LF_SUBSTR_LIST:
			ON_LF_ARGLIST(mr, eLeaf, end);
			break;
		case LF_VFUNCTAB:
			ON_LF_VFUNCTAB(mr, eLeaf, end);
			break;
		case LF_STMEMBER:
		case LF_STMEMBER_ST:
			ON_LF_STMEMBER(mr, eLeaf, end);
			break;
		case LF_ARRAY:
		case LF_ARRAY_ST:
			ON_LF_ARRAY(mr, eLeaf, end);
			break;
		case LF_VBCLASS:
		case LF_IVBCLASS:
			ON_LF_VBCLASS(mr, eLeaf, end);
			break;
		case LF_UDT_MOD_SRC_LINE:
			ON_LF_UDT_MOD_SRC_LINE(mr, eLeaf, end);
			break;
		case LF_STRING_ID:
			ON_LF_STRING_ID(mr, eLeaf, end);
			break;
		case LF_BUILDINFO:
			ON_LF_BUILDINFO(mr, eLeaf, end);
			break;
		case LF_FUNC_ID:
			ON_LF_FUNC_ID(mr, eLeaf, end);
			break;
		case LF_MFUNC_ID:
			ON_LF_MFUNC_ID(mr, eLeaf, end);
			break;
		case LF_VTSHAPE:
			ON_LF_VTSHAPE(mr, eLeaf, end);
			break;
		case LF_INDEX:
			ON_LF_INDEX(mr, eLeaf, end);
			break;
		case LF_BITFIELD:
			ON_LF_BITFIELD(mr, eLeaf, end);
			break;
		default:
#ifdef _DEBUG
			fprintf(stdout, "unknown leaf at %08X\n", (unsigned)mr.cpr());
#endif
			return false;
		}
		return true;
	}
};

#define BEGIN_CV_LEAF(arg)	BEGIN_DYNAMIC_TYPE_0(arg, CodeView_LeafBase) 
#define END_CV_LEAF(arg)	END_DYNAMIC_TYPE(arg)


BEGIN_CV_LEAF(CodeView_LEAF)
{
	DECLDATA(uint16_t, length);
	mr.declField("reclen", mr.type(TYPEID_UINT16));
	POSITION beg(mr.cp());
	POSITION end(beg + length);
	DECLDATA(uint16_t, leaf);
	mr.declField("leaf", mr.enumOf(mr.type("LEAF_ENUM_e"), TYPEID_UINT16));
	dispatchLeaf(mr, leaf, end);//dispatcher
}
END_CV_LEAF(CodeView_LEAF);


void PDB20_DeclareDynamicTypes(I_SuperModule &mr)
{
	if (mr.EnterAttic())
	{
		mr.DeclareContextDependentType(_PFX("ModInfo"));
		mr.DeclareContextDependentType(_PFX("CodeView_SYMBOL"));
		mr.DeclareContextDependentType(_PFX("CVFileChecksum"));
		mr.DeclareContextDependentType(_PFX("CodeView_D_FILECHKSMS"));
		mr.DeclareContextDependentType(_PFX("CodeView_D_LINES"));
		mr.DeclareContextDependentType(_PFX("CodeViewSubsection__TBD"));
		mr.DeclareContextDependentType(_PFX("CodeView_LEAF"));
		mr.Leave();
	}
}

void PDB70_DeclareDynamicTypes(I_SuperModule &mr)
{
	PDB20_DeclareDynamicTypes(mr);
}
