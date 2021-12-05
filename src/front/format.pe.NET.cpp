#include "shared.h"
#include "format.pe.h"
#include "decode.pe.h"

using namespace adcwin;


////////////////////////////////////////////////////////.NET
void PE_Strucs_t::createStructures_dotNET()
{
	if (mr.NewScope("IMAGE_COR20_HEADER"))
	{
		mr.declField("cb", type(TYPEID_DWORD));
		mr.declField("MajorRuntimeVersion", type(TYPEID_WORD));
		mr.declField("MinorRuntimeVersion", type(TYPEID_WORD));
		mr.declField("MetaData", type("IMAGE_DATA_DIRECTORY"));
		mr.declField("Flags", type(TYPEID_DWORD));
		if (mr.NewScope(mr.declField()))//nullptr, SCOPE_ UNION))
		{
			mr.declUField("EntryPointToken", type(TYPEID_DWORD));
			mr.declUField("EntryPointRVA", type(TYPEID_DWORD), ATTR_RVA);
			mr.Leave();
		}
		mr.declField("Resources", type("IMAGE_DATA_DIRECTORY"));
		mr.declField("StrongNameSignature", type("IMAGE_DATA_DIRECTORY"));
		mr.declField("CodeManagerTable", type("IMAGE_DATA_DIRECTORY"));
		mr.declField("VTableFixups", type("IMAGE_DATA_DIRECTORY"));
		mr.declField("ExportAddressTableJumps", type("IMAGE_DATA_DIRECTORY"));
		mr.declField("ManagedNativeHeader", type("IMAGE_DATA_DIRECTORY"));
		/*mr.declField("EEInfoTable", type("IMAGE_DATA_DIRECTORY"));
		mr.declField("HelperTable", type("IMAGE_DATA_DIRECTORY"));
		mr.declField("DynamicInfo", type("IMAGE_DATA_DIRECTORY"));
		mr.declField("DelayLoadInfo", type("IMAGE_DATA_DIRECTORY"));
		mr.declField("ModuleImage", type("IMAGE_DATA_DIRECTORY"));
		mr.declField("ExternalFixups", type("IMAGE_DATA_DIRECTORY"));
		mr.declField("RidMap", type("IMAGE_DATA_DIRECTORY"));
		mr.declField("DebugMap", type("IMAGE_DATA_DIRECTORY"));
		mr.declField("IPMap", type("IMAGE_DATA_DIRECTORY"));*/
		mr.Leave();
	}

	/////////////////////////////////////////////////////

	if (mr.NewScope("CORTable_Module"))
	{
		mr.declField("Generation", type(TYPEID_WORD));
		mr.declField("Name", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_STRING_INDEX);
		mr.declField("Mvid", type(TYPEID_WORD));
		mr.declField("EncId", type(TYPEID_WORD));
		mr.declField("EncBaseId", type(TYPEID_WORD));
		mr.Leave();
	}

	if (mr.NewScope("CORTable_TypeRef"))
	{
		mr.declField("ResolutionScope", type(TYPEID_WORD));
		mr.declField("TypeName", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_STRING_INDEX);
		mr.declField("TypeNamespace", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_STRING_INDEX);
		mr.Leave();
	}

	if (mr.NewScope("CORTable_TypeDef"))
	{
		mr.installTypesMgr();
		if (mr.NewScope("VisibilityEnum", SCOPE_ENUM))
		{
			mr.declEField("NotPublic");//0
			mr.declEField("Public");//1
			mr.declEField("NestedPublic");//2
			mr.declEField("NestedPrivate");//3
			mr.declEField("NestedFamily");//4
			mr.declEField("NestedAssembly");//5
			mr.declEField("NestedFamANDAssem");//6
			mr.declEField("NestedFamORAssem");//7
			mr.Leave();
		}

		if (mr.NewScope("LayoutEnum", SCOPE_ENUM))
		{
			mr.declEField("AutoLayout");//0
			mr.declEField("SequentialLayout");//1
			mr.declEField("ExplicitLayout");//2
			mr.Leave();
		}

		if (mr.NewScope("ClassSemanticsEnum", SCOPE_ENUM))
		{
			mr.declEField("Class");//0
			mr.declEField("Interface");//1
			mr.Leave();
		}

		if (mr.NewScope("StringFormatEnum", SCOPE_ENUM))
		{
			mr.declEField("AnsiClass");//0
			mr.declEField("UnicodeClass");//1
			mr.declEField("AutoClass");//2
			mr.declEField("CustomFormatClass");//3
			mr.Leave();
		}

		if (mr.NewScope(mr.declField("Flags")))//, ATTR_COLLAPSED))
		{
			mr.declBField("Visibility", arrayOf(enumOf(type("VisibilityEnum"), TYPEID_DWORD), 3));//<0>
			//NotPublic:0,Public:1,NestedPublic:2,NestedPrivate:3,NestedFamily:4,NestedAssembly:5,NestedFamANDAssem:6,NestedFamORAssem:7
			mr.declBField("Layout", arrayOf(enumOf(type("LayoutEnum"), TYPEID_DWORD), 2));//<3>
			//AutoLayout:0,SequentialLayout:1,ExplicitLayout:2
			mr.declBField("ClassSemantics", enumOf(type("ClassSemanticsEnum"), TYPEID_DWORD));//<5>
			//Class:0,Interface:1
			mr.skip(1);
			mr.declBField("Abstract", type(TYPEID_DWORD));//<7>
			mr.declBField("Sealed", type(TYPEID_DWORD));//<8>
			mr.skip(1);
			mr.declBField("SpecialName", type(TYPEID_DWORD));//<10>
			mr.declBField("RTSpecialName", type(TYPEID_DWORD));//<11>
			mr.declBField("Import", type(TYPEID_DWORD));//<12>
			mr.declBField("Serializable", type(TYPEID_DWORD));//<13>
			mr.skip(2);
			mr.declBField("StringFormat", arrayOf(enumOf(type("StringFormatEnum"), TYPEID_DWORD), 2));//<16>
			//AnsiClass:0,UnicodeClass:1,AutoClass:2,CustomFormatClass:3
			mr.skip(1);
			mr.declBField("HasSecurity", type(TYPEID_DWORD));//<19>
			mr.declBField("BeforeFieldInit", type(TYPEID_DWORD));//<20>
			mr.skip(1);
			mr.declBField("IsTypeForwarder", type(TYPEID_DWORD));//<22>
			mr.declBField("CustomStringFormat", arrayOf(type(TYPEID_DWORD), 3));//<22>
			//mr.skip(7);
			mr.Leave();
		}
		mr.declField("TypeName", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_STRING_INDEX);
		mr.declField("TypeNamespace", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_STRING_INDEX);
		mr.declField("Extends", type(TYPEID_WORD));
		mr.declField("FieldList", type(TYPEID_WORD));
		mr.declField("MethodList", type(TYPEID_WORD));
		mr.Leave();
	}

	//4
	if (mr.NewScope("CORTable_Field"))
	{
		mr.installTypesMgr();
		if (mr.NewScope("FieldAccessEnum", SCOPE_ENUM))
		{
			mr.declEField("CompilerControlled");//0
			mr.declEField("Private");//1
			mr.declEField("FamANDAssem");//2
			mr.declEField("Assembly");//3
			mr.declEField("Family");//4
			mr.declEField("FamORAssem");//5
			mr.declEField("Public");//6
			mr.Leave();
		}

		//mr.declField("Flags", type(TYPEID_WORD), ATTR_BINARY);
		if (mr.NewScope(mr.declField("Flags")))//, ATTR_COLLAPSED))
		{
			mr.declBField("FieldAccess", arrayOf(enumOf(type("FieldAccessEnum"), TYPEID_WORD), 3));//<0>
			//CompilerControlled:0,Private:1,FamANDAssem:2,Assembly:3,Family:4,FamORAssem:5,Public:6
			mr.skip(1);
			mr.declBField("Static", type(TYPEID_WORD));//<4>
			mr.declBField("InitOnly", type(TYPEID_WORD));//<5>
			mr.declBField("Literal", type(TYPEID_WORD));//<6>
			mr.declBField("NotSerialized", type(TYPEID_WORD));//<7>
			mr.declBField("HasFieldRVA", type(TYPEID_WORD));//<8>
			mr.declBField("SpecialName", type(TYPEID_WORD));//<9>
			mr.declBField("RTSpecialName", type(TYPEID_WORD));//<10>
			mr.skipBits(1);
			mr.declBField("HasFieldMarshal", type(TYPEID_WORD));//<12>
			mr.declBField("PInvokeImpl", type(TYPEID_WORD));//<13>
			mr.skipBits(1);
			mr.declBField("HasDefault", type(TYPEID_WORD));//<15>
			mr.Leave();
		}
		mr.declField("Name", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_STRING_INDEX);
		mr.declField("Signature", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_BLOB_INDEX);
		mr.Leave();
	}

	//6
	if (mr.NewScope("CORTable_MethodDef"))
	{
		mr.installTypesMgr();
		if (mr.NewScope("CodeTypeEnum", SCOPE_ENUM))
		{
			mr.declEField("IL");//0
			mr.declEField("Native");//1
			mr.declEField("OPTIL");//2
			mr.declEField("Runtime");//3
			mr.Leave();
		}

		if (mr.NewScope("ManagedMaskEnum", SCOPE_ENUM))
		{
			mr.declEField("Managed");//0
			mr.declEField("Unmanaged");//1
			mr.Leave();
		}

		mr.declField("RVA", type(TYPEID_DWORD), ATTR_RVA);
		if (mr.NewScope(mr.declField("ImplFlags")))//, ATTR_COLLAPSED))
		{
			mr.declBField("CodeType", arrayOf(enumOf(type("CodeTypeEnum"), TYPEID_WORD), 2));//<0>
			//IL:0,Native:1,OPTIL:2,Runtime:3
			mr.declBField("ManagedMask", enumOf(type("ManagedMaskEnum"), TYPEID_WORD));//<2>
			//Managed:0,Unmanaged:1
			mr.declBField("NoInlining", type(TYPEID_WORD));//<3>
			mr.declBField("ForwardRef", type(TYPEID_WORD));//<4>
			mr.declBField("Synchronized", type(TYPEID_WORD));//<5>
			mr.declBField("NoOptimization", type(TYPEID_WORD));//<6>
			mr.declBField("PreserveSig", type(TYPEID_WORD));//<7>
			mr.skipBits(4);
			mr.declBField("InternalCall", type(TYPEID_WORD));//<12>
			mr.skipBits(3);
			mr.Leave();
		}

		if (mr.NewScope("MemberAccessEnum", SCOPE_ENUM))
		{
			mr.declEField("CompilerControlled");//0
			mr.declEField("Private");//1
			mr.declEField("FamANDAssem");//2
			mr.declEField("Assem");//3
			mr.declEField("Family");//4
			mr.declEField("FamORAssem");//5
			mr.declEField("Public");//6
			mr.Leave();
		}

		if (mr.NewScope("VtableLayoutEnum", SCOPE_ENUM))
		{
			mr.declEField("ReuseSlot");//0
			mr.declEField("NewSlot");//1
			mr.Leave();
		}

		if (mr.NewScope(mr.declField("Flags")))//, ATTR_COLLAPSED))
		{
			mr.declBField("MemberAccess", arrayOf(enumOf(type("MemberAccessEnum"), TYPEID_WORD), 3));//<0>
			//CompilerControlled:0,Private:1,FamANDAssem:2,Assem:3,Family:4,FamORAssem:5,Public:6
			mr.declBField("UnmanagedExport", type(TYPEID_WORD));//<3>
			mr.declBField("Static", type(TYPEID_WORD));//<4>
			mr.declBField("Final", type(TYPEID_WORD));//<5>
			mr.declBField("Virtual", type(TYPEID_WORD));//<6>
			mr.declBField("HideBySig", type(TYPEID_WORD));//<7>
			mr.declBField("VtableLayout", enumOf(type("VtableLayoutEnum"), TYPEID_WORD));//<8>
			//ReuseSlot:0,NewSlot:1
			mr.declBField("Strict", type(TYPEID_WORD));//<9>
			mr.declBField("Abstract", type(TYPEID_WORD));//<10>
			mr.declBField("SpecialName", type(TYPEID_WORD));//<11>
			mr.declBField("RTSpecialName", type(TYPEID_WORD));//<12>
			mr.declBField("PInvokeImpl", type(TYPEID_WORD));//<13>
			mr.declBField("HasSecurity", type(TYPEID_WORD));//<14>
			mr.declBField("RequireSecObject", type(TYPEID_WORD));//<15>
			mr.Leave();
		}
		mr.declField("Name", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_STRING_INDEX);
		mr.declField("Signature", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_BLOB_INDEX);
		mr.declField("ParamList", type(TYPEID_WORD));
		mr.Leave();
	}

	//8
	if (mr.NewScope("CORTable_Param"))
	{
		if (mr.NewScope(mr.declField("Flags")))//, ATTR_COLLAPSED))
		{
			mr.declBField("In", type(TYPEID_WORD));//<0>
			mr.declBField("Out", type(TYPEID_WORD));//<1>
			mr.skipBits(2);
			mr.declBField("Optional", type(TYPEID_WORD));//<4>
			mr.skipBits(7);
			mr.declBField("HasDefault", type(TYPEID_WORD));//<12>
			mr.declBField("HasFieldMarshal", type(TYPEID_WORD));//<13>
			mr.skipBits(2);
			mr.Leave();
		}
		mr.declField("Sequence", type(TYPEID_WORD));
		mr.declField("Name", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_STRING_INDEX);
		mr.Leave();
	}

	//9
	if (mr.NewScope("CORTable_InterfaceImpl"))
	{
		mr.declField("Class", type(TYPEID_WORD));
		mr.declField("Interface", type(TYPEID_WORD));
		mr.Leave();
	}

	//0x0A(10)
	if (mr.NewScope("CORTable_FieldLayout"))
	{
		mr.declField("Offset", type(TYPEID_DWORD));
		mr.declField("Field", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.Leave();
	}

	//0x0B(11)
	if (mr.NewScope("CORTable_Constant"))
	{
		mr.declField("Type", type(TYPEID_BYTE));
		mr.skip(1);
		mr.declField("Parent", type(TYPEID_WORD));
		mr.declField("Value", type(TYPEID_WORD));
		mr.Leave();
	}

	//0x0C(12)
	if (mr.NewScope("CORTable_CustomAttribute"))
	{
		mr.declField("Parent", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.declField("Type", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.declField("Value", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_BLOB_INDEX);
		mr.Leave();
	}
	
	//0x0D(13)
	if (mr.NewScope("CORTable_FieldMarshal"))
	{
		mr.declField("Parent", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.declField("NativeType", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_BLOB_INDEX);
		mr.Leave();
	}
	//15
	if (mr.NewScope("CORTable_ClassLayout"))
	{
		mr.declField("PackingSize", type(TYPEID_WORD));
		mr.declField("ClassSize", type(TYPEID_DWORD));
		mr.declField("Parent", type(TYPEID_WORD));
		mr.Leave();
	}
	//16
	if (mr.NewScope("CORTable_FieldLayout"))
	{
		mr.declField("Offset", type(TYPEID_DWORD));
		mr.declField("Field", type(TYPEID_WORD));
		mr.Leave();
	}
	//0x11
	if (mr.NewScope("CORTable_StandAloneSig"))
	{
		mr.declField("Signature", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_BLOB_INDEX);
		mr.Leave();
	}

	//0x15
	if (mr.NewScope("CORTable_PropertyMap"))
	{
		mr.declField("Parent", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.declField("PropertyList", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.Leave();
	}

	//0x17
	if (mr.NewScope("CORTable_Property"))
	{
		if (mr.NewScope(mr.declField("Flags")))//, ATTR_COLLAPSED))
		{
			mr.skipBits(9);
			mr.declBField("SpecialName", type(TYPEID_WORD));//<9>
			mr.declBField("RTSpecialName", type(TYPEID_WORD));//<10>
			mr.skipBits(1);
			mr.declBField("HasDefault", type(TYPEID_WORD));//<11>
			mr.skipBits(4);
			mr.Leave();
		}
		mr.declField("Name", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_STRING_INDEX);
		mr.declField("Type", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_BLOB_INDEX);
		mr.Leave();
	}

	//0x18 (24)
	if (mr.NewScope("CORTable_MethodSemantics"))
	{
		//mr.declField("Flags", type(TYPEID_WORD), ATTR_BINARY);
		if (mr.NewScope(mr.declField("Flags")))//, ATTR_COLLAPSED))
		{
			mr.declBField("Setter", type(TYPEID_WORD));//<0>
			mr.declBField("Getter", type(TYPEID_WORD));//<1>
			mr.declBField("Other", type(TYPEID_WORD));//<2>
			mr.declBField("AddOn", type(TYPEID_WORD));//<3>
			mr.declBField("RemoveOn", type(TYPEID_WORD));//<4>
			mr.declBField("Fire", type(TYPEID_WORD));//<5>
			mr.setSize(CHAR_BIT * sizeof(WORD));//?
			mr.Leave();
		}
		mr.declField("Method", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.declField("Association", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.Leave();
	}

	//0x19 (25)
	if (mr.NewScope("CORTable_MethodImpl"))
	{
		mr.declField("Class", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.declField("MethodBody", type(TYPEID_WORD));
		mr.declField("MethodDeclaration", type(TYPEID_WORD));
		mr.Leave();
	}

	//0x1A (26)
	if (mr.NewScope("CORTable_MethodDeclaration"))
	{
		mr.declField("Name", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_BLOB_INDEX);
		mr.Leave();
	}

	//0x1B
	if (mr.NewScope("CORTable_TypeSpec"))
	{
		mr.declField("Signature", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_BLOB_INDEX);
		mr.Leave();
	}

	//0x1C
	if (mr.NewScope("CORTable_ImplMap"))
	{
		mr.declField("MappingFlags", type(TYPEID_WORD));
		mr.declField("MemberForwarded", type(TYPEID_WORD));
		mr.declField("ImportName", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_BLOB_INDEX);
		mr.declField("ImportScope", type(TYPEID_WORD));
		mr.Leave();
	}

	//0x1D
	if (mr.NewScope("CORTable_FieldRVA"))
	{
		mr.declField("RVA", type(TYPEID_DWORD), ATTR_RVA);
		mr.declField("Field", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.Leave();
	}

	//0x20
	if (mr.NewScope("CORTable_Assembly"))
	{
		mr.declField("HashAlgId", type(TYPEID_DWORD));
		mr.declField("MajorVersion,", type(TYPEID_WORD));
		mr.declField("MinorVersion", type(TYPEID_WORD));
		mr.declField("BuildNumber", type(TYPEID_WORD));
		mr.declField("RevisionNumber", type(TYPEID_WORD));
		mr.declField("Flags", type(TYPEID_DWORD), ATTR_BINARY);
		mr.declField("PublicKey", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_BLOB_INDEX);
		mr.declField("Name", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_STRING_INDEX);
		mr.declField("Culture", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_STRING_INDEX);
		mr.Leave();
	}

	//0x23
	if (mr.NewScope("CORTable_AssemblyRef"))
	{
		mr.declField("MajorVersion", type(TYPEID_WORD));
		mr.declField("MinorVersion", type(TYPEID_WORD));
		mr.declField("BuildNumber", type(TYPEID_WORD));
		mr.declField("RevisionNumber", type(TYPEID_WORD));
		mr.declField("Flags", type(TYPEID_DWORD), ATTR_BINARY);
		mr.declField("PublicKeyOrToken", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_BLOB_INDEX);
		mr.declField("Name", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_STRING_INDEX);
		mr.declField("Culture", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_STRING_INDEX);
		mr.declField("HashValue", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_BLOB_INDEX);
		mr.Leave();
	}

	//0x28 (40)
	if (mr.NewScope("CORTable_ManifestResource"))
	{
		mr.declField("Offset", type(TYPEID_DWORD));
		mr.declField("Flags", type(TYPEID_DWORD), ATTR_BINARY);
		mr.declField("Name", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_STRING_INDEX);
		mr.declField("Implementation", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.Leave();
	}

	//0x29 (41)
	if (mr.NewScope("CORTable_NestedClass"))
	{
		mr.declField("NestedClass", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.declField("EnclosingClass", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.Leave();
	}

	//0x2A (42)
	if (mr.NewScope("CORTable_GenericParam"))
	{
		mr.declField("Number", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.declField("Flags", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.declField("Owner", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.declField("Name", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_STRING_INDEX);
		mr.Leave();
	}

	//0x2B (43)
	if (mr.NewScope("CORTable_MethodSpec"))
	{
		mr.declField("Method", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.declField("Instantiation", type(TYPEID_WORD), (AttrIdEnum)ATTR_NET_BLOB_INDEX);
		mr.Leave();
	}

	//0x2C (44)
	if (mr.NewScope("CORTable_GenericParamConstraint"))
	{
		mr.declField("Owner", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.declField("Constraint", type(TYPEID_WORD), ATTR_DECIMAL);
		mr.Leave();
	}
}

/*
00 - Module                  01 - TypeRef                02 - TypeDef
04 - Field                   06 - MethodDef              08 - Param
09 - InterfaceImpl           10 - MemberRef              11 - Constant
12 - CustomAttribute         13 - FieldMarshal           14 - DeclSecurity
15 - ClassLayout             16 - FieldLayout            17 - StandAloneSig
18 - EventMap                20 - Event                  21 - PropertyMap
23 - Property                24 - MethodSemantics        25 - MethodImpl
26 - ModuleRef               27 - TypeSpec               28 - ImplMap
29 - FieldRVA                32 - Assembly               33 - AssemblyProcessor
34 - AssemblyOS              35 - AssemblyRef            36 - AssemblyRefProcessor
37 - AssemblyRefOS           38 - File                   39 - ExportedType
40 - ManifestResource        41 - NestedClass            42 - GenericParam
44 - GenericParamConstraint
*/

void CORTable_declField(I_Module &mr, unsigned i, int rows)
{
	switch (i)
	{
	case 0://Module
		mr.declField("#ModuleTable", mr.arrayOf(mr.type("CORTable_Module"), rows), ATTR_COLLAPSED);
		break;
	case 1://TypeRef
		mr.declField("#TypeRefTable", mr.arrayOf(mr.type("CORTable_TypeRef"), rows), ATTR_COLLAPSED);
		break;
	case 2://TypeDef
		mr.declField("#TypeDefTable", mr.arrayOf(mr.type("CORTable_TypeDef"), rows), ATTR_COLLAPSED);
		break;
	case 4://Field
		mr.declField("#FieldTable", mr.arrayOf(mr.type("CORTable_Field"), rows), ATTR_COLLAPSED);
		break;
	case 6://MethodDef
		mr.declField("#MethodDefTable", mr.arrayOf(mr.type("CORTable_MethodDef"), rows), ATTR_COLLAPSED);
		break;
	case 8://Param
		mr.declField("#ParamTable", mr.arrayOf(mr.type("CORTable_Param"), rows), ATTR_COLLAPSED);
		break;
	case 9://InterfaceImpl
		mr.declField("#InterfaceImplTable", mr.arrayOf(mr.type("CORTable_InterfaceImpl"), rows), ATTR_COLLAPSED);
		break;
	case 0x0A://FieldLayout
		mr.declField("#FieldLayoutTable", mr.arrayOf(mr.type("CORTable_FieldLayout"), rows), ATTR_COLLAPSED);
		break;
	case 0x0B://Constant
		mr.declField("#ConstantTable", mr.arrayOf(mr.type("CORTable_Constant"), rows), ATTR_COLLAPSED);
		break;
	case 0x0C://CustomAttribute
		mr.declField("#CustomAttributeTable", mr.arrayOf(mr.type("CORTable_CustomAttribute"), rows), ATTR_COLLAPSED);
		break;
	case 0x0D://FieldMarshal
		mr.declField("#FieldMarshalTable", mr.arrayOf(mr.type("CORTable_FieldMarshal"), rows), ATTR_COLLAPSED);
		break;
	case 0x0F://ClassLayout (15)
		mr.declField("#ClassLayoutTable", mr.arrayOf(mr.type("CORTable_ClassLayout"), rows), ATTR_COLLAPSED);
		break;
	case 0x10://FieldLayout (16)
		mr.declField("#FieldLayoutTable", mr.arrayOf(mr.type("CORTable_FieldLayout"), rows), ATTR_COLLAPSED);
		break;
	case 0x11://StandAloneSig
		mr.declField("#StandAloneSigTable", mr.arrayOf(mr.type("CORTable_StandAloneSig"), rows), ATTR_COLLAPSED);
		break;
	case 0x15://PropertyMap
		mr.declField("#PropertyMapTable", mr.arrayOf(mr.type("CORTable_PropertyMap"), rows), ATTR_COLLAPSED);
		break;
	case 0x17://Property
		mr.declField("#PropertyTable", mr.arrayOf(mr.type("CORTable_Property"), rows), ATTR_COLLAPSED);
		break;
	case 0x18://MethodSemantics (24)
		mr.declField("#MethodSemanticsTable", mr.arrayOf(mr.type("CORTable_MethodSemantics"), rows), ATTR_COLLAPSED);
		break;
	case 0x19://MethodImpl (25)
		mr.declField("#MethodImplTable", mr.arrayOf(mr.type("CORTable_MethodImpl"), rows), ATTR_COLLAPSED);
		break;
	case 0x1A://MethodDeclaration (26)
		mr.declField("#MethodDeclarationTable", mr.arrayOf(mr.type("CORTable_MethodDeclaration"), rows), ATTR_COLLAPSED);
		break;
	case 0x1B://TypeSpec (27)
		mr.declField("#TypeSpecTable", mr.arrayOf(mr.type("CORTable_TypeSpec"), rows), ATTR_COLLAPSED);
		break;
	case 0x1C://ImplMap (28)
		mr.declField("#ImplMapTable", mr.arrayOf(mr.type("CORTable_ImplMap"), rows), ATTR_COLLAPSED);
		break;
	case 0x1D://FieldRVA
		mr.declField("#FieldRVATable", mr.arrayOf(mr.type("CORTable_FieldRVA"), rows), ATTR_COLLAPSED);
		break;
	case 0x20://Assembly
		mr.declField("#AssemblyTable", mr.arrayOf(mr.type("CORTable_Assembly"), rows), ATTR_COLLAPSED);
		break;
	case 0x23://AssemblyRef
		mr.declField("#AssemblyRefTable", mr.arrayOf(mr.type("CORTable_AssemblyRef"), rows), ATTR_COLLAPSED);
		break;
	case 0x28://ManifestResource (40)
		mr.declField("#ManifestResourceTable", mr.arrayOf(mr.type("CORTable_ManifestResource"), rows), ATTR_COLLAPSED);
		break;
	case 0x29://NestedClass (41)
		mr.declField("#NestedClassTable", mr.arrayOf(mr.type("CORTable_NestedClass"), rows), ATTR_COLLAPSED);
		break;
	case 0x2A://GenericParam (42)
		mr.declField("#GenericParamTable", mr.arrayOf(mr.type("CORTable_GenericParam"), rows), ATTR_COLLAPSED);
		break;
	case 0x2B://MethodSpec (43)
		mr.declField("#MethodSpecTable", mr.arrayOf(mr.type("CORTable_MethodSpec"), rows), ATTR_COLLAPSED);
		break;
	case 0x2C://GenericParamConstraint (44)
		mr.declField("#GenericParamConstraintTable", mr.arrayOf(mr.type("CORTable_GenericParamConstraint"), rows), ATTR_COLLAPSED);
		break;
	default:
		mr.error("unimplemented .NET table");
		break;
	}
}


void PE_Strucs_t::declareDynamicTypes_dotNET()
{
	mr.DeclareContextDependentType(_PFX("COR_MetaDataRoot"));
	mr.DeclareContextDependentType(_PFX("COR_StreamHeader"));
	mr.DeclareContextDependentType(_PFX("COR_TablesRoot"));
	mr.DeclareContextDependentType(_PFX("COR_Blob"));
	mr.DeclareContextDependentType(_PFX("COR_UserString"));
}






