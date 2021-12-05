#include <sstream>
#include "shared/obj_id.h"
#include "shared.h"

class DataStream2_t : public DataStream_t//for convenience
{
public:
	DataStream2_t(const I_Module &r) : DataStream_t(r, r.cpr()){}
};


////////////////////////////////////////////////////////////StringType
BEGIN_DYNAMIC_TYPE(StringType)
{
	DECLDATA(unsigned short, uLength);
	mr.declField("Length", mr.type(TYPEID_WORD));
	if (uLength > 0)
	{
#if(0)
		DECLDATAPTR(char, p);
		std::ostringstream os;
		p.fetchString(os, uLength);
		std::string s(os.str());
#endif
		mr.declField("String", mr.arrayOf(mr.type(TYPEID_CHAR), uLength));
	}
}
END_DYNAMIC_TYPE(StringType);

////////////////////////////////////////////////////////////SubFolderType
BEGIN_DYNAMIC_TYPE(SubFolderType)
{
	mr.declField("ChildPtr", mr.type(TYPEID_DWORD));
	mr.declField("Name", mr.type(_PFX("StringType")));
}
END_DYNAMIC_TYPE(SubFolderType);

/*struct s_t
{
	unsigned short len;
	const char data[1];
};*/

struct upair_t {
	unsigned	first;
	unsigned	second;
};

////////////////////////////////////////////////////////////FolderType
BEGIN_DYNAMIC_TYPE(FolderType)
{
	DECLDATA(unsigned char, uId);
	mr.declField("Id", mr.type(TYPEID_BYTE));
	//mr.declField("Next", mr.type(TYPEID_DWORD));
	//mr.declField("FirstChild", mr.type(TYPEID_DWORD));
	mr.declField("Flags", mr.type(TYPEID_DWORD));
	mr.declField("Description", mr.type(_PFX("StringType")));
	switch (uId)
	{
	case FILEID_MODULE:
	{
		DECLDATA(unsigned, chNum);
		mr.declField("SubFoldersNum", mr.type(TYPEID_DWORD));
		if (chNum > 0)
		{
			if (mr.NewScope(mr.declField("SubFolders")))
			{
				for (unsigned i(0); i < chNum; i++)
					mr.declField("SubFolder", mr.type(_PFX("SubFolderType")));
				mr.Leave();
			}
		}
		mr.declField("BinaryPtr", mr.type(TYPEID_DWORD));
		//FILEID_MODULE
		DECLDATA(unsigned, eFTyp);
		mr.declField("QuickFilesNum", mr.type(TYPEID_DWORD));
		mr.declField("QuickFiles", mr.arrayOf(mr.type(TYPEID_DWORD), eFTyp - unsigned(1)));
		break;
	}
	case FILEID_FOLDER:
	{
		DECLDATA(unsigned, chNum);
		mr.declField("SubFoldersNum", mr.type(TYPEID_DWORD));
		if (chNum > 0)
			if (mr.NewScope(mr.declField("SubFolders")))
			{
				for (unsigned i(0); i < chNum; i++)
					mr.declField("SubFolder", mr.type(_PFX("SubFolderType")));
				mr.Leave();
			}
		break;
	}
	case FILEID_FILE:
	{
		unsigned uCount(0);
		DataStream2_t pu(mr);
		//for (DECLDATAPTR(unsigned, pu); *pu; ++pu)
		while (pu.read<unsigned>() != 0)
			uCount++;
		mr.declField("FileRefs", mr.arrayOf(mr.type(TYPEID_DWORD), uCount + 1));

#if(0)
		uCount = 0;
		//for (DECLDATAPTR(unsigned, pu); *pu; ++pu)
		while (pu.read<unsigned>() != 0)
			uCount++;
		mr.declField("TypeRefs", mr.arrayOf(mr.type(TYPEID_DWORD), uCount + 1));
		uCount = 0;
#else
		for (;;)
		{
			DECLDATA(uint32_t, uTypeRef);
			if (!uTypeRef)
			{
				mr.declField("TypesEnd", mr.type(TYPEID_DWORD));
				break;
			}
			mr.declField(nullptr, mr.type(_PFX("TypeRefEx")));//type refs + extra stuff (for class types etc.)
		}
#endif
#if(0)
		while (pu.read<unsigned>() != 0)
			uCount++;
		mr.declField("FieldRefs", mr.arrayOf(mr.type(TYPEID_DWORD), uCount + 1));//with refs to funcdefs!
#else
		//POSITION cur = mr.cp();
		for (;;)
		{
			DECLDATA(unsigned, uDockField);
			if (uDockField == 0)
			{
				mr.declField("GlobsEnd", mr.type(TYPEID_DWORD));
				break;
			}
			mr.declField(nullptr, mr.type(_PFX("GlobRef")));
		}
#endif

		break;
	}
	case FILEID_TEMPL:
	{
		unsigned u(1);//one for eos
		struct bimap_s { unsigned name, obj; };
		for (DECLDATAPTREX(bimap_s, pu); pu->name; pu++)
			u++;
		mr.declField("PrettyNames", mr.arrayOf(mr.type("NAM2OBJ"), u));
		break;
	}
	case FILEID_STUBS:
	{
		for (;;)
		{
			DECLDATA(unsigned, uAddr);
			mr.declField(nullptr/*"Stub#"*/, mr.type(_PFX("StubType")));
			if (!uAddr)
				break;
		}
		break;
	}
	default:
		break;
	}
}
END_DYNAMIC_TYPE(FolderType);

////////////////////////////////////////////////////////////DataSourceType
BEGIN_DYNAMIC_TYPE(DataSourceType)
{
	DECLDATA(unsigned char, uId);
	mr.declField("Id", mr.type(TYPEID_BYTE));
	mr.declField("Name", mr.type(_PFX("StringType")));

	switch (uId)
	{
	case DATAID_SOURCE:
	{
		mr.declField("Path", mr.type(_PFX("StringType")));
		break;
	}
	case DATAID_LEECH:
	{
		mr.declField("DataHost", mr.type(_PFX("StringType")));
		unsigned uChunks(0);
		mr.declField("ChunksNum", mr.type(TYPEID_DWORD));
		mr.declField("Chunks", mr.arrayOf(mr.type("DATA_PATCH"), uChunks));
		break;
	}
	default:
		mr.error("invalid data source id");
		return;
	}

	if (mr.NewScope(mr.declField("AuxData")))
	{
		//mr.installNamespace();
		mr.declField("Offset", mr.type(TYPEID_QWORD));
		DECLDATA(unsigned, size);
		mr.declField("Size", mr.type(TYPEID_QWORD));
		if (size > 0)
			mr.declField("Bytes", mr.arrayOf(mr.type(TYPEID_BYTE), size));
		mr.Leave();
	}
}
END_DYNAMIC_TYPE(DataSourceType);


////////////////////////////////////////////////////////////NameSpType
BEGIN_DYNAMIC_TYPE(NameSpType)
{
}
END_DYNAMIC_TYPE(NameSpType);



////////////////////////////////////////////////////////////TypesMgrType
BEGIN_DYNAMIC_TYPE(TypesMgrType)
{
	//mr.declField("Owner", mr.type(TYPEID_DWORD));
	unsigned uTypesCount(0);
	DataStream2_t pu(mr);
	//for (DECLDATAPTR(unsigned, pu); *pu; ++pu)
	while (pu.read<unsigned>() != 0)
		uTypesCount++;
	//if (uTypesCount > 0)
		mr.declField("UserTypes", mr.arrayOf(mr.type(TYPEID_DWORD), uTypesCount + 1));//including eos
	mr.declField("CodeType", mr.type(TYPEID_DWORD));
}
END_DYNAMIC_TYPE(TypesMgrType);



////////////////////////////////////////////////////////////CplxType
BEGIN_DYNAMIC_TYPE(CplxType)
{
	//DECLDATA(unsigned char, uHasNS);
	DECLDATA2(unsigned, uFlags, -(int)(sizeof(char) + sizeof(unsigned)));//get flags at negative offset (-objtype(1byte) -objflags(4bytes))
	//mr.declField("HasNS", mr.type(TYPEID_BYTE));
	//if (uHasNS)
	if (uFlags & TYP_HAS_NMAP)
		mr.declField("Name", mr.type(TYPEID_DWORD));
}
END_DYNAMIC_TYPE(CplxType);



////////////////////////////////////////////////////////////StrucType
BEGIN_DYNAMIC_TYPE(StrucType)
{
	DECLDATA2(unsigned, uFlags, -(int)(sizeof(char) + sizeof(unsigned)));//get flags at negative offset (-objtype(1byte) -objflags(4bytes))
	mr.declField("AsCplx", mr.type(_PFX("CplxType")));
	if (uFlags & TYP_HAS_TMAP)
		mr.declField("TypesMgr", mr.type(_PFX("TypesMgrType")));
//	mr.declField("Size", mr.type(TYPEID_DWORD));
	DECLDATA(unsigned, uFieldCount);
	mr.declField("FieldCount", mr.type(TYPEID_DWORD));
	if (uFieldCount)
		mr.declField("Fields", mr.arrayOf(mr.type("FIELD_ID"), uFieldCount));// , ATTR_COLLAPSED);
}
END_DYNAMIC_TYPE(StrucType);


///////////////////////////////////////////////////////////TypeRef
BEGIN_DYNAMIC_TYPE(TypeRefEx)
{
	DECLDATA(uint32_t, uTypeRef);
	mr.declField("TypeRef", mr.type(TYPEID_DWORD));
	//mr.declField("AsStruc", mr.type(_PFX("StrucType")));
	//if (uFlags & TYPEOBJ_CLASS)
	if (uTypeRef & 0x80000000)//class!
	{
		DataStream2_t pu(mr);
		unsigned uCount(1);//for eos
		while (pu.read<unsigned>() != 0)
			uCount++;
		mr.declField("MembersPtrs", mr.arrayOf(mr.type(TYPEID_DWORD), uCount));

		while (pu.read<unsigned>() != 0)
		{
			if (mr.NewScope(mr.declField()))//, SCOPE_STRUC, "VTable#"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.installNamespace();
				mr.declField("VTablePtr", mr.type(TYPEID_DWORD));
				pu.skip<unsigned>();
				mr.declField("VOffset", mr.type(TYPEID_DWORD));
				unsigned uCount(0);//for eos
				while (pu.read<unsigned>() != 0)
				{
					uCount++;
					pu.skip<unsigned>();//off
				}
				if (uCount > 0)
					mr.declField("VMethods", mr.arrayOf(mr.type("FIELD_ID"), uCount));
				mr.declField("VMethodsEnd", mr.type(TYPEID_DWORD));
			}
		}
		mr.declField("VTablesEnd", mr.type(TYPEID_DWORD));
	}
}
END_DYNAMIC_TYPE(TypeRefEx);



BEGIN_DYNAMIC_TYPE(GlobRef)
{
	mr.declField("DockField", mr.type(TYPEID_DWORD));
	mr.declField("ObjFlags", mr.type(TYPEID_DWORD));
	DECLDATA(unsigned char, uObjId);
	mr.declField("ObjId", mr.type(TYPEID_BYTE));
	switch (uObjId)
	{
	case OBJID_TYPE_FUNCDEF:
		mr.declField(nullptr, mr.type(_PFX("FuncDefType")));
		break;
	case OBJID_TYPE_THUNK:
		mr.declField("CodePtr", mr.type(TYPEID_DWORD));
		break;
	case OBJID_NULL:
		break;//a var
	default:
		//mr.Leave();
		throw(-6);
	}
}
END_DYNAMIC_TYPE(GlobRef);



////////////////////////////////////////////////////////////UnionType
/*BEGIN_DYNAMIC_TYPE(UnionType)
{
	mr.declField("AsStruc", mr.type(_PFX("StrucType")));
}
END_DYNAMIC_TYPE(UnionType);*/



////////////////////////////////////////////////////////////BitsetType
BEGIN_DYNAMIC_TYPE(BitsetType)
{
	mr.declField("AsStruc", mr.type(_PFX("StrucType")));
}
END_DYNAMIC_TYPE(BitsetType);



////////////////////////////////////////////////////////////StrucvarType
BEGIN_DYNAMIC_TYPE(StrucvarType)
{
	mr.declField("AsCplx", mr.type(_PFX("CplxType")));
	//mr.declField("AsStruc", mr.type(_PFX("StrucType")));
	/*DECLDATA(unsigned, uFieldCount);
	mr.declField("FieldCount", mr.type(TYPEID_DWORD));
	if (uFieldCount)
		mr.declField("Fields", mr.arrayOf(mr.type("FIELD_ID"), uFieldCount));*/
}
END_DYNAMIC_TYPE(StrucvarType);



////////////////////////////////////////////////////////////FuncType
BEGIN_DYNAMIC_TYPE(FuncType)
{
	mr.declField("AsStruc", mr.type(_PFX("StrucType")));
}
END_DYNAMIC_TYPE(FuncType);


////////////////////////////////////////////////////////////VTableType
/*BEGIN_DYNAMIC_TYPE(VTableType)
{
	mr.declField("AsStruc", mr.type(_PFX("StrucType")));
	mr.declField("Assignee", mr.type(TYPEID_DWORD));
}
END_DYNAMIC_TYPE(VTableType);*/


////////////////////////////////////////////////////////////SegType
BEGIN_DYNAMIC_TYPE(SegType)
{
	mr.declField("AsStruc", mr.type(_PFX("StrucType")));
	mr.declField("Title", mr.type(_PFX("StringType")));
	mr.declField("Base64", mr.type(TYPEID_QWORD));
	mr.declField("RawOffset", mr.type(TYPEID_QWORD));
	mr.declField("RawSize", mr.type(TYPEID_QWORD));
	mr.declField("Flags", mr.type(TYPEID_DWORD));
	//mr.declField("EntryPoint", mr.type(TYPEID_DWORD));

	DataStream2_t pu(mr);

	//mr.declField("SegListPtr", mr.type(TYPEID_DWORD));
	mr.declField("TraceLink", mr.type(TYPEID_DWORD));
	mr.declField("Address", mr.type(TYPEID_DWORD));
	//mr.declField("InferLink", mr.type(TYPEID_DWORD));

	unsigned uSubSegsCount(0);
	//DECLDATAPTR(unsigned, ptr);
	pu.seek(mr.cpr());
	//for (;; ++ptr, uSubSegsCount++)
	while (pu.read<unsigned>() != 0)
		uSubSegsCount++;
		//if (!(*ptr))
			//break;
	if (uSubSegsCount > 0)
		mr.declField("SubSegList", mr.arrayOf(mr.type(TYPEID_DWORD), uSubSegsCount));
	mr.declField("Eos", mr.type(TYPEID_DWORD));//eos

	DECLDATA(unsigned, num2);
	mr.declField("CoFieldCount", mr.type(TYPEID_DWORD));
	if (num2 > 0)
		mr.declField("CoFields", mr.arrayOf(mr.type("FIELD_ID"), num2));

#if(0)
	DECLDATA(unsigned, num3);
	mr.declField("UnFieldCount", mr.type(TYPEID_DWORD));
	if (num3)
		mr.declField("UnFields", mr.arrayOf(mr.type("FIELD_ID"), num3));
#endif

	//mr.declField("FrontString", mr.type(_PFX("StringType")));
	mr.declField("FrontIndex", mr.type(TYPEID_DWORD));
}
END_DYNAMIC_TYPE(SegType);



////////////////////////////////////////////////////////////ObjRefType
BEGIN_DYNAMIC_TYPE(ObjRefType)
{
	DECLDATA(unsigned char, uObjRef);
	mr.declField("Id", mr.type(TYPEID_BYTE));
	if (uObjRef == 2)
		mr.declField("FieldPtr", mr.type(TYPEID_DWORD));
	else if (uObjRef == 3)
		mr.declField("TypePtr", mr.type(TYPEID_DWORD));
	else if (uObjRef != 0)
	{ assert(0);//error
	}
}
END_DYNAMIC_TYPE(ObjRefType);



////////////////////////////////////////////////////////////StubType
BEGIN_DYNAMIC_TYPE(StubType)
{
	DECLDATA(unsigned, uVA);
	mr.declField("VA", mr.type(TYPEID_DWORD));
	if (uVA)
	{
		DECLDATA(unsigned char, uFlags);
		mr.declField("Flags", mr.type(TYPEID_BYTE));
		if (uFlags & 0x10)
			mr.declField("FieldPtr", mr.type(TYPEID_DWORD));
		if (uFlags & 0x20)
			mr.declField("ValueStr", mr.type(_PFX("StringType")));
	}
}
END_DYNAMIC_TYPE(StubType);


//static unsigned guFileDefCount = 0;

////////////////////////////////////////////////////////////DcType
BEGIN_DYNAMIC_TYPE(DcType)
{
//	mr.declField("AsStruc", mr.type(_PFX("StrucType")));
	mr.declField("ModuleFolderPtr", mr.type(TYPEID_DWORD));
//	mr.declField("FrontString", mr.type(_PFX("StringType")));
	mr.declField("OwnerSegPtr", mr.type(TYPEID_DWORD));

#if(0)
	//special folders
	DECLDATA(unsigned, eFTyp);
	mr.declField("StubsFoldersNum", mr.type(TYPEID_DWORD));
	mr.declField("SpecialFolders", mr.arrayOf(mr.type(TYPEID_DWORD), eFTyp - 1));
#endif
	/*mr.declField("StubsFolderPtr", mr.type(TYPEID_DWORD));
	mr.declField("PrefixFolderPtr", mr.type(TYPEID_DWORD));
	mr.declField("ImportFolderPtr", mr.type(TYPEID_DWORD));
	mr.declField("ExportFolderPtr", mr.type(TYPEID_DWORD));
	mr.declField("ResourceFolderPtr", mr.type(TYPEID_DWORD));
	mr.declField("TypesFolderPtr", mr.type(TYPEID_DWORD));*/

/*	unsigned uTypedefCount(1);//one for eos
	for (DECLDATAPTR(unsigned, pu); *pu; ++pu)
		uTypedefCount++;
	mr.declField("TypedefPtrs", mr.arrayOf(mr.type(TYPEID_DWORD), uTypedefCount));*/

	DECLDATA(unsigned, uIntrinsicCount);
	mr.declField("IntrinsicCount", mr.type(TYPEID_DWORD));

	for (unsigned i(0); i < uIntrinsicCount; i++)
	{
		if (mr.NewScope(mr.declField()))
		{
			SAFE_SCOPE_HERE(mr);
			mr.installNamespace();
			mr.declField("FieldExPtr", mr.type(TYPEID_DWORD));
			mr.declField("ObjFlags", mr.type(TYPEID_DWORD));
			DECLDATA(unsigned char, uObjId);
			mr.declField("ObjId", mr.type(TYPEID_BYTE));
			switch (uObjId)
			{
			case OBJID_TYPE_FUNCDEF:
				mr.declField(nullptr, mr.type(_PFX("FuncDefType")));
				break;
			default:
				//mr.Leave();
				throw(-6);
			}
		}
		else
			throw(-7);
	}

	/*struct static_t {
		unsigned	va;
		unsigned	iClass;
	};
	unsigned uStaticsCount(1);//one for eos
	for (DECLDATAPTR(static_t, pu); pu->va; ++pu)
		uStaticsCount++;
	mr.declField("StaticsMap", mr.arrayOf(mr.type("STAT_ELT"), uStaticsCount));*/
}
END_DYNAMIC_TYPE(DcType);



////////////////////////////////////////////////////////////FuncDefType
BEGIN_DYNAMIC_TYPE(FuncDefType)
{
	mr.declField("AsStruc", mr.type(_PFX("StrucType")));
/*	DECLDATA(unsigned, uRetCount);
	mr.declField("RetFieldCount", mr.type(TYPEID_DWORD));
	if (uRetCount)
		mr.declField("RetFieldPtrs", mr.arrayOf(mr.type("FIELD_ID"), uRetCount));*/
//	mr.declField("ThisPtr", mr.type(TYPEID_DWORD));
/*#if(X64_SUPPORT)
	mr.declField("SpoiltRegs", mr.type(TYPEID_QWORD));
#else
	mr.declField("SpoiltRegs", mr.type(TYPEID_DWORD));
#endif*/
	mr.declField("StackOut", mr.type(TYPEID_WORD));
	mr.declField("CPUFSpoiled", mr.type(TYPEID_WORD));
	mr.declField("FPUOut", mr.type(TYPEID_BYTE));
	//mr.declField("OwnerPtr", mr.type(TYPEID_DWORD));
}
END_DYNAMIC_TYPE(FuncDefType);



////////////////////////////////////////////////////////////ModuleType
BEGIN_DYNAMIC_TYPE(ModuleType)
{
	mr.declField("AsSeg", mr.type(_PFX("SegType")));
	mr.declField("ModuleTag", mr.type(TYPEID_DWORD));
	mr.declField("ModuleUnique", mr.type(TYPEID_DWORD));
	mr.declField("DataPtr", mr.type(TYPEID_DWORD));
	mr.declField("FolderPtr", mr.type(TYPEID_DWORD));
	mr.declField("SubTitle", mr.type(_PFX("StringType")));
	mr.declField("DelayedFormat", mr.type(_PFX("StringType")));

	unsigned uRangesNum(0);//one for eos
	//for (DECLDATAPTR(unsigned, pu); *pu; ++pu)
	DataStream2_t pu(mr);
	while (pu.read<unsigned>() != 0)
		uRangesNum++;

	mr.declField("RangeSet", mr.arrayOf(mr.type(TYPEID_DWORD), (int)uRangesNum + 1));
	/*for (;;)
	{
		DECLDATA(unsigned, uRanges);
		mr.declField("RangesNum", mr.type(TYPEID_DWORD));
		if (uRanges == 0)
			break;
		mr.declField("Base64", mr.type(TYPEID_QWORD));
		mr.declField("Ranges", mr.arrayOf(mr.type(TYPEID_QWORD), (int)uRanges));
	}*/
}
END_DYNAMIC_TYPE(ModuleType);



////////////////////////////////////////////////////////////ProjectType
BEGIN_DYNAMIC_TYPE(ProjectType)
{
	mr.declField("AsSeg", mr.type(_PFX("SegType")));
	DECLDATA(int, n);
	mr.declField("FrontSockNum", mr.type(TYPEID_DWORD));
	for (int i(0); i < n; i++)
	{
		if (mr.NewScope(mr.declField("FRONTSOCK")))
		{
			mr.declField("FrontId", mr.type(TYPEID_DWORD));
			DataStream_t p(mr, mr.cpr());
			std::ostringstream s;
			p.fetchString(s);
			mr.declField("FrontName", mr.type(_PFX("StringType")));
			mr.Leave();
		}
	}
}
END_DYNAMIC_TYPE(ProjectType);



////////////////////////////////////////////////////////////GlobalArena
BEGIN_DYNAMIC_TYPE(GlobalArena)
{
	DECLDATA(unsigned, uSkipCount);
	mr.declField("SkipCount", mr.type(TYPEID_DWORD));
	if (uSkipCount > 0)
	{
		assert(uSkipCount == sizeof(uint32_t));//?
		mr.declField("FieldExCount", mr.type(TYPEID_DWORD));
	}

	DECLDATA(unsigned, uNameCount);
	mr.declField("NameCount", mr.type(TYPEID_DWORD));

	DECLDATA(unsigned, uTypeCount);
	mr.declField("TypeCount", mr.type(TYPEID_DWORD));

	//DECLDATA(unsigned, uTypeMapCount);
	//mr.declField("TypeMapCount", mr.type(TYPEID_DWORD));

	DECLDATA(unsigned, uFieldCount);
	mr.declField("FieldCount", mr.type(TYPEID_DWORD));

	for (unsigned i(0); i < uNameCount; i++)
	{
		const char *n(nullptr);
		mr.declField(n, mr.type(_PFX("StringType")));//!
	}

	for (unsigned i(0); i < uTypeCount; i++)
	{
		if (mr.NewScope(mr.declField()))
		{
			SAFE_SCOPE_HERE(mr);
			mr.installNamespace();
			mr.declField("ObjFlags", mr.type(TYPEID_DWORD));
			DECLDATA(unsigned char, uObjId);
			mr.declField("ObjId", mr.type(TYPEID_BYTE));

			ObjId_t objId((ObjId_t)uObjId.get());
			switch (objId)
			{
			case OBJID_TYPE_PROJECT:
				mr.declField(nullptr, mr.type(_PFX("ProjectType")));
				break;
			case OBJID_TYPE_MODULE:
				mr.declField(nullptr, mr.type(_PFX("ModuleType")));
				//mr.instField(nullptr, mr.type(_PFX("DcType")));
				break;
			case OBJID_TYPE_SIMPLE:
				mr.declField(nullptr, mr.type("TYPE_SIMPLE"));
				break;
			case OBJID_TYPE_SEG:
				mr.declField(nullptr, mr.type(_PFX("SegType")));
				break;
			case OBJID_TYPE_STRUC:
			case OBJID_TYPE_STRUCLOC:
				mr.declField(nullptr, mr.type(_PFX("StrucType")));
				break;
			case OBJID_TYPE_CLASS:
			case OBJID_TYPE_NAMESPACE:
				mr.declField(nullptr, mr.type(_PFX("StrucType")));//ClassType
				break;
/*			case OBJID_TYPE_UNION:
#if(!NEW_LOCAL_VARS)
			case OBJID_TYPE_UNIONLOC:
#endif
				mr.declField(nullptr, mr.type(_PFX("UnionType")));
				break;*/
			case OBJID_TYPE_ARRAY:
				mr.declField(nullptr, mr.type("TYPE_ARRAY"));
				break;
			case OBJID_TYPE_STRUCVAR:
				mr.declField(nullptr, mr.type(_PFX("StrucvarType")));
				break;
			case OBJID_TYPE_BITSET:
				mr.declField(nullptr, mr.type(_PFX("BitsetType")));
				break;
				//case OBJID_TYPE_BIT:
				//mr.declField(nullptr, mr.type("TYPE_BIT"));
				//break;
			case OBJID_TYPE_ENUM:
				mr.declField(nullptr, mr.type("TYPE_ENUM"));
				break;
			case OBJID_TYPE_ARRAY_INDEX:
				mr.declField(nullptr, mr.type("TYPE_ARRAY_INDEX"));
				break;
			case OBJID_TYPE_CONST:
				mr.declField(nullptr, mr.type("TYPE_CONST"));
				break;
			case OBJID_TYPE_PAIR:
				mr.declField(nullptr, mr.type("TYPE_PAIR"));
				break;
			case OBJID_TYPE_FUNC:
				mr.declField(nullptr, mr.type("TYPE_FUNC"));
				break;
			case OBJID_TYPE_CODE:
				mr.declField(nullptr, mr.type(_PFX("CplxType")));
				break;
			case OBJID_TYPE_PROC:
				mr.declField(nullptr, mr.type(_PFX("FuncType")));
				break;
				//		case OBJID_TYPE_DC:
				//		mr.instField(nullptr, mr.type(_PFX("DcType")));
				//	break;
			case OBJID_TYPE_TYPEDEF:
				mr.declField(nullptr, mr.type("TYPE_TYPEDEF"));
				break;
			case OBJID_TYPE_FUNCDEF:
				mr.declField(nullptr, mr.type(_PFX("FuncDefType")));
				break;
			case OBJID_TYPE_PTR:
			case OBJID_TYPE_REF:
			case OBJID_TYPE_RREF:
				mr.declField(nullptr, mr.type("TYPE_PTR"));
				break;
			case OBJID_TYPE_IMP:
				mr.declField(nullptr, mr.type("TYPE_IMP"));
				break;
			case OBJID_TYPE_EXP:
				mr.declField(nullptr, mr.type("TYPE_EXP"));
				break;
			case OBJID_TYPE_VPTR:
				mr.declField(nullptr, mr.type("TYPE_VPTR"));
				break;
			case OBJID_TYPE_THISPTR:
				mr.declField(nullptr, mr.type("TYPE_THISPTR"));
				break;
			case OBJID_TYPE_PROXY:
				mr.declField(nullptr, mr.type("TYPE_PROXY"));
				break;
/*			case OBJID_TYPE_VFTABLE:
			case OBJID_TYPE_VBTABLE:
			case OBJID_TYPE_LVFTABLE:
			case OBJID_TYPE_CVFTABLE:
				mr.declField(nullptr, mr.type(_PFX("VTableType")));
				break;*/
			case OBJID_TYPE_THUNK:
				mr.declField(nullptr, mr.type("TYPE_THUNK"));
				break;
			default:
				//mr.Leave();
				throw(-6);
			}
		}
		else
			throw(-7);
	}

	//for (unsigned i(0); i < uTypeMapCount; i++)
		//mr.declField(nullptr, mr.type(_PFX("TypesMgrType")));

	if (uFieldCount > 0)
		mr.declField("Fields", mr.arrayOf(mr.type("FIELD"), uFieldCount));
}
END_DYNAMIC_TYPE(GlobalArena);

////////////////////////////////////////////////////////////OpType
BEGIN_DYNAMIC_TYPE(OpType)
{
	DECLDATA(unsigned, uFlags);
	mr.declField("ObjFlags", mr.type(TYPEID_DWORD));
	mr.declField("NextPtr", mr.type(TYPEID_DWORD));
	if (uFlags & OPND_EX_RI)//RI
	{
		mr.declField("No", mr.type(TYPEID_DWORD));
		mr.declField("Action", mr.type(TYPEID_DWORD));
		mr.declField("Flags", mr.type(TYPEID_BYTE));
		mr.declField("StackIn", mr.type(TYPEID_DWORD));
		mr.declField("StackDiff", mr.type(TYPEID_DWORD));
		mr.declField("FpuIn", mr.type(TYPEID_BYTE));
		mr.declField("FpuDiff", mr.type(TYPEID_BYTE));
		mr.declField("EFlagsTested", mr.type(TYPEID_WORD));
		mr.declField("EFlagsModified", mr.type(TYPEID_WORD));
		mr.declField("FpuFlags", mr.type(TYPEID_WORD));
		mr.declField("ArgsPtr", mr.type(TYPEID_DWORD));
		//if ((uFlags >> 16) == 2)
			//mr.declField("PathPtr", mr.type(TYPEID_DWORD));
//		if (uFlags &  OPND_EX_LABELREF)
//			mr.declField("LabelRef", mr.type(TYPEID_DWORD));
	}
	mr.declField("Opc", mr.type(TYPEID_BYTE));
	mr.declField("Offs", mr.type(TYPEID_BYTE));
	mr.declField("Optyp", mr.type(TYPEID_BYTE));
	if (uFlags &  OPND_EX_DISP)
		mr.declField("Disp", mr.type(TYPEID_DWORD));
	if (uFlags &  OPND_EX_DISP0)
		mr.declField("DispOld", mr.type(TYPEID_DWORD));
	if (uFlags &  OPND_EX_XIN)
	{
		unsigned count(1);
		DataStream2_t pu(mr);
		while (pu.read<unsigned>() != 0)
			count++;
		mr.declField("XIns", mr.arrayOf(mr.type(TYPEID_DWORD), count));
	}
	if (uFlags &  OPND_EX_LOCALREF)
		mr.declField("LocalRef", mr.type(TYPEID_DWORD));
}
END_DYNAMIC_TYPE(OpType);


////////////////////////////////////////////////////////////PathType
BEGIN_DYNAMIC_TYPE(PathType)
{
	mr.declField("NextPtr", mr.type(TYPEID_DWORD));
	mr.declField("ChildrenPtr", mr.type(TYPEID_DWORD));
	DECLDATA(unsigned, u);
	mr.declField("Flags", mr.type(TYPEID_DWORD));
	/*if (u & PATH_EX_HAS_ANCHOR)
		mr.declField("Anchor", mr.type(TYPEID_DWORD));*/
	if (u & PATH_EX_HAS_OPS)
		mr.declField("OpsPtr", mr.type(TYPEID_DWORD));
	if (u & PATH_EX_HAS_REFS)
	{
		unsigned count(1);//1 for eos
		DataStream2_t pu(mr);
		while (pu.read<unsigned>() != 0)
			count++;
		mr.declField("Inflow", mr.arrayOf(mr.type(TYPEID_DWORD), count));
	}
}
END_DYNAMIC_TYPE(PathType);



////////////////////////////////////////////////////////////LocalArena
BEGIN_DYNAMIC_TYPE(LocalArena)
{
	DECLDATA(unsigned short, uFolderLength);

	mr.declField("Folder", mr.type(_PFX("StringType")));

	if (uFolderLength != 0)//dispersed mode?
	{
		DECLDATA(unsigned, uDiferredRefsNum);
		mr.declField("DiferredRefsNum", mr.type(TYPEID_DWORD));
		mr.declField("DiferredRefs", mr.arrayOf(mr.type(TYPEID_DWORD), uDiferredRefsNum));
		return;
	}
	
	mr.declField("AsGlobal", mr.type(_PFX("GlobalArena")));//instField?

	DECLDATA(unsigned, uPathCount);
	mr.declField("PathCount", mr.type(TYPEID_DWORD));
	DECLDATA(unsigned, uOpCount);
	mr.declField("OpCount", mr.type(TYPEID_DWORD));

	//if (uPathCount)
		//mr.declField("Paths", mr.arrayOf(mr.type(_PFX("PathType")), uPathCount));

	for (unsigned i(0); i < uPathCount; i++)
		mr.declField(nullptr, mr.type(_PFX("PathType")));

	for (unsigned i(0); i < uOpCount; i++)
		mr.declField(nullptr, mr.type(_PFX("OpType")));

	for (;;)
	{
		mr.NewScope(mr.declField());
		mr.installNamespace();
		DECLDATA(unsigned, uFuncDefPtr);
		mr.declField("FuncDefPtr", mr.type(TYPEID_DWORD));
		if (!uFuncDefPtr)
		{
			mr.Leave();
			break;
		}
		mr.declField("Extra", mr.type("FUNCEX"));
		mr.Leave();
	}
}
END_DYNAMIC_TYPE(LocalArena);





void ADC_RegisterFormatters(I_ModuleEx &);

/////////////////////////////////////////FORMAT_ADC
class CDynamicType_FORMAT_ADC : public I_FormatterType
{
	class T_Impl
	{
		I_SuperModule &mr;
	public:
		T_Impl(I_SuperModule &r) : mr(r)
		{
		}
		void preformat()
		{
			if (mr.NewScope(mr.declField()))
			{
				SAFE_SCOPE_HERE(mr);
				mr.installNamespace();
				mr.installTypesMgr();
				createStructures(mr);
				mr.DeclareContextDependentType(_PFX("GlobalArena"));
				mr.DeclareContextDependentType(_PFX("LocalArena"));
				mr.DeclareContextDependentType(_PFX("StringType"));
				mr.DeclareContextDependentType(_PFX("DataSourceType"));
				mr.DeclareContextDependentType(_PFX("FolderType"));
				mr.DeclareContextDependentType(_PFX("SubFolderType"));
				mr.DeclareContextDependentType(_PFX("ProjectType"));
				mr.DeclareContextDependentType(_PFX("ModuleType"));
				mr.DeclareContextDependentType(_PFX("SegType"));
				mr.DeclareContextDependentType(_PFX("StrucType"));
				//mr.DeclareContextDependentType(_PFX("UnionType"));
				mr.DeclareContextDependentType(_PFX("BitsetType"));
				mr.DeclareContextDependentType(_PFX("CplxType"));
				mr.DeclareContextDependentType(_PFX("TypesMgrType"));
				mr.DeclareContextDependentType(_PFX("StrucvarType"));
				mr.DeclareContextDependentType(_PFX("FuncType"));
				mr.DeclareContextDependentType(_PFX("DcType"));
				mr.DeclareContextDependentType(_PFX("ObjRefType"));
				mr.DeclareContextDependentType(_PFX("StubType"));
				mr.DeclareContextDependentType(_PFX("TypeRefEx"));
				mr.DeclareContextDependentType(_PFX("GlobRef"));
				mr.DeclareContextDependentType(_PFX("FuncDefType"));
				mr.DeclareContextDependentType(_PFX("OpType"));
				mr.DeclareContextDependentType(_PFX("PathType"));
				//mr.DeclareContextDependentType(_PFX("VTableType"));

				mr.declField("Signature", mr.type(TYPEID_DWORD));
				mr.declField("Version", mr.type(TYPEID_DWORD));

				DECLDATA(unsigned, uDataCount);
				mr.declField("DataSourceNum", mr.type(TYPEID_DWORD));

				DECLDATA(unsigned, uFolderCount);
				mr.declField("FolderNum", mr.type(TYPEID_DWORD));

				for (unsigned i(0); i < uDataCount; i++)
					mr.declField(nullptr, mr.type(_PFX("DataSourceType")));

//return;
				//		guFileDefCount = 0;//global!
				mr.instField("GLOBAL", mr.type(_PFX("GlobalArena")));
//return;
				for (unsigned i(0); i < uFolderCount; i++)
					mr.declField(nullptr, mr.type(_PFX("FolderType")));

				mr.declField("RootFolderPtr", mr.type(TYPEID_DWORD));
				mr.declField("StartupFolderPtr", mr.type(TYPEID_DWORD));

				////////////////////////////////////////////// DC-extras begin here
				
				mr.declField("UnExtModulePtr", mr.type(TYPEID_DWORD));

				for (;;)
				{
					DECLDATA(unsigned, uBinaryFolder);
					if (!uBinaryFolder)
					{
						mr.declField("NoMoreDCs", mr.type(TYPEID_DWORD));
						break;
					}
					if (mr.NewScope(mr.declField()))
					{
						SAFE_SCOPE_HERE(mr);
						mr.installNamespace();
						mr.instField("DC", mr.type(_PFX("DcType")));
						for (unsigned i(0);; i++)
						{
							DECLDATA(unsigned, uFolderPtr);
							if (!uFolderPtr)
							{
								mr.declField("NoMoreFiles", mr.type(TYPEID_DWORD));
								break;
							}
							std::string stgt;
							mr.declField(stringf(stgt, "FolderPtr_%d", i + 1).c_str(), mr.type(TYPEID_DWORD));
							mr.instField(stringf(stgt, "FILE_%d", i + 1).c_str(), mr.type(_PFX("LocalArena")));
						}
					}
					else
						break;
				}


				/*		for (unsigned i(0); i < guFileDefCount; i++)
						{
						DECLDATA(unsigned short, uStrLen);
						if (uStrLen == 0)
						break;
						mr.instField(string_format("FILE_%d", i + 1).c_str(), mr.type(_PFX("LocalArena")));
						}*/

			}
		}

		void createStructures(I_Module &mr)
		{
			if (mr.NewScope("FIELD_ID"))
			{
				mr.declField("Index", mr.type(TYPEID_DWORD));
				mr.declField("Offset", mr.type(TYPEID_DWORD));
				mr.Leave();
			}

			if (mr.NewScope("STOCK_TYPE"))
			{
				mr.declField("Index", mr.type(TYPEID_DWORD));
				mr.declField("OpType", mr.type(TYPEID_BYTE));
				mr.Leave();
			}

			if (mr.NewScope("TYPE_SIMPLE"))
			{
				mr.declField("Id", mr.type(TYPEID_DWORD));
				mr.Leave();
			}

			if (mr.NewScope("TYPE_ARRAY"))
			{
				mr.declField("BasePtr", mr.type(TYPEID_DWORD));
				mr.declField("Total", mr.type(TYPEID_DWORD));
				mr.Leave();
			}

			if (mr.NewScope("TYPE_TYPEDEF"))
			{
				mr.declField("NamePtr", mr.type(TYPEID_DWORD));
				mr.declField("BasePtr", mr.type(TYPEID_DWORD));
				mr.Leave();
			}

			if (mr.NewScope("TYPE_PTR"))
			{
				mr.declField(nullptr, mr.type("TYPE_SIMPLE"));
				mr.declField("BasePtr", mr.type(TYPEID_DWORD));
				mr.Leave();
			}

			if (mr.NewScope("TYPE_VPTR"))
			{
				mr.declField(nullptr, mr.type("TYPE_SIMPLE"));
				mr.Leave();
			}

			if (mr.NewScope("TYPE_THISPTR"))
			{
				mr.declField(nullptr, mr.type("TYPE_SIMPLE"));
				mr.Leave();
			}

			if (mr.NewScope("TYPE_ENUM"))
			{
				mr.declField(nullptr, mr.type("TYPE_SIMPLE"));
				mr.declField("EnumRef", mr.type(TYPEID_DWORD));
				mr.Leave();
			}

			if (mr.NewScope("TYPE_IMP"))
			{
				mr.declField("BasePtr", mr.type(TYPEID_DWORD));
				mr.Leave();
			}

			if (mr.NewScope("TYPE_EXP"))
			{
				mr.declField("BasePtr", mr.type(TYPEID_DWORD));
				mr.Leave();
			}

			if (mr.NewScope("TYPE_CONST"))
			{
				mr.declField("BasePtr", mr.type(TYPEID_DWORD));
				mr.Leave();
			}

			if (mr.NewScope("TYPE_PAIR"))
			{
				mr.declField("LeftPtr", mr.type(TYPEID_DWORD));
				mr.declField("RightPtr", mr.type(TYPEID_DWORD));
				mr.Leave();
			}

			if (mr.NewScope("TYPE_FUNC"))
			{
				mr.declField("RetValPtr", mr.type(TYPEID_DWORD));
				mr.declField("ArgsPtr", mr.type(TYPEID_DWORD));
				mr.declField("Flags", mr.type(TYPEID_DWORD));
				mr.Leave();
			}

			if (mr.NewScope("TYPE_ARRAY_INDEX"))
			{
				mr.declField("ArrayRef", mr.type(TYPEID_DWORD));
				mr.declField("EnumRef", mr.type(TYPEID_DWORD));
				mr.Leave();
			}

			if (mr.NewScope("TYPE_PROXY"))
			{
				//?mr.installNamespace();
				mr.declField("NamePtr", mr.type(TYPEID_DWORD));
				mr.declField("IncumbentPtr", mr.type(TYPEID_DWORD));
				mr.Leave();
			}

			if (mr.NewScope("TYPE_THUNK"))
			{
				mr.declField("CodePtr", mr.type(TYPEID_DWORD));
				mr.Leave();
			}

			/*if (mr.NewScope("TYPE_BIT"))
			{
			mr.installNamespace();
			mr.declField("Id", mr.type(TYPEID_DWORD));
			mr.Leave();
			}*/

			if (mr.NewScope("FIELD"))
			{
				//mr.installNamespace();
				mr.declField("Flags", mr.type(TYPEID_DWORD));
				mr.declField("NamePtr", mr.type(TYPEID_DWORD));
				mr.declField("TypePtr", mr.type(TYPEID_DWORD));
				mr.Leave();
			}

			if (mr.NewScope("FUNCEX"))
			{
				mr.declField("LocalsPtr", mr.type(TYPEID_DWORD));
				mr.declField("PathsPtr", mr.type(TYPEID_DWORD));
				mr.declField("OpsPtr", mr.type(TYPEID_DWORD));
				mr.Leave();
			}

			if (mr.NewScope("NAM2OBJ"))
			{
				mr.declField("NamePtr", mr.type(TYPEID_DWORD));
				mr.declField("ObjPtr", mr.type(TYPEID_DWORD));
				mr.Leave();
			}

			if (mr.NewScope("DATA_PATCH"))
			{
				//mr.installNamespace();
				mr.declField("UpperSelf", mr.type(TYPEID_DWORD));
				mr.declField("LowerHost", mr.type(TYPEID_DWORD));
				mr.Leave();
			}
		}
	};
public:
	virtual const char *name() const { return "FORMAT_ADC"; }
	virtual void createz(I_SuperModule &r, unsigned long nSize)
	{
		T_Impl impl(r);
		impl.preformat();
	}
};

DECLARE_FORMATTER(CDynamicType_FORMAT_ADC, FORMAT_ADC);

///////////////////////////////////////////////
void ADC_RegisterFormatters(I_ModuleEx &rMain)
{
	rMain.RegisterFormatterType(_PFX("FORMAT_ADC"));
}

