#include "make_type.h"
#include "shared/defs.h"
#include "shared/front.h"
#include "front/shared.h"
#include "shared/front.h"
#include "db/anlzbin.h"
#include "db/type_proxy.h"
#include "db/info_types.h"
#include "db/front_impl.h"
#include "sym_parse.h"
#include "type_funcdef.h"
#include "ana_local.h"
#include "cc.h"
#include "info_class.h"
#include "clean_ex.h"
#include "stubs.h"


TypePtr TypeFomNode0_t::OnType(const char *name)
{
	CObjPtr pObj(mrProject.findObject(name, ModulePtr()));
	if (pObj)
		return pObj->objType();
	return findType(m_ctx.cont()->objTypeGlob(), name);
//	return nullptr;
}

TypePtr TypeFomNode0_t::process(node_t *node)
{
	if (!node)
		return nullptr;

	//assert(mrFileDef.ownerFolder());

	TypePtr iOwnerSeg(PrimeSeg());
	//NamesMgr_t &rNS(NamespaceInitialized(iOwnerSeg));

	switch (node->type)
	{
		case NODE_FUNC:
		case NODE_NULL://data?
			assert(0);


		case NODE_TYPE:
		{
			if (node->isCompound())
				ASSERT0;
			if (node->isEnum())
				return GetStockType(OPTYP_UINT32);
			MyString s(node->name);//type_name
			if (node->isUnsigned())
				s.insert(0, "unsigned ");
			OpType_t optyp;
			if (Str2OpTyp(s.c_str(), optyp))
			{
				if (optyp == OPTYP_NULL)
					return nullptr;
				return GetStockType(optyp);
			}
			TypePtr iType(OnType(s));
			if (!iType)
			{
				if (node->isTerminal() && node->hasName())//some compound type?
					if (node->parent)//top-level nodes without a parent normally are data names
						ASSERT0;
			}
			else if (iType->isEnum())
			{
				STOP
			}
			return iType;
		}

		case NODE_PTR:
		{
			TypePtr iType(process(node->left));
			//CHECKID(iType, -130)
			//STOP
			return PtrOf(iType);
		}

		case NODE_REF://convert into const ptr? returned const ptrs seem do work like no const
		{
			TypePtr iType(process(node->left));
			return RefOf(iType);
		}

		case NODE_RVAL_REF:
		{
			TypePtr iType(process(node->left));
			return RvalRefOf(iType);
		}

		case NODE_PTR_TO_MEMBER:
		{
			TypePtr iType(process(node->left));//???FIXME(later)
			return PtrOf(iType);
		}

		case NODE_ARRAY:
		{
			TypePtr iType(process(node->left));
			assert(node->right->type == NODE_NUMBER);
			unsigned num(atoi(node->right->name.c_str()));
			return ArrayOf(iType, num);
		}

		case NODE_VALIST:
			break;

#if(NO_BS)
		case NODE_THUNK:
		case NODE_VTHUNK:
		case NODE_NVTHUNK:
		{
			break;
		}

		case NODE_STRING:
		{
			TypePtr iType(GetStockType(OPTYP_CHAR8));
			TypesTracer_t TT(*this, *findTypeMgr(iType));
			return TT.stringOf(iType, m_ctx.rawoff(), -1);//range?
		}
#endif

		default:
		assert(0);
		break;
	}
	
	return nullptr;
}


////////////////////////////////////////////////////////////

/*#define USE_CACHE	1
TypePtr TypeFomNode_t::FindStrucInFiles(const MyString &fullName)//search in files of current binary ONLY!
{
#if(USE_CACHE)
	return mTypeLookup.findType(fullName, ModulePtr());
#endif

#if(0)
	for (FilesMgr0_t::FolderIterator i(mrDC.binaryFolderPtr()); i; i++)
	{
		FolderPtr pFolder(*i);
		FileDef_t *pfDef(pFolder->fileDef());
		if (pfDef)
		{
			TypePtr p(FindStruc(*pfDef, fullName));
			if (p)
			{
#if(USE_CACHE)
				mrDC.addCac hedType(fullName, p);
#endif
				return p;
			}
		}
	}
#endif
	return nullptr;
}*/

TypePtr TypeFomNode_t::ProcessClass(FullName_t aClass, int iNesting, bool bExporting, bool bIsStruc)//from function scopes: must be declared in current module for certain (if not - force a duplicate)
{
	MyString sClass(aClass.join());
CHECK(sClass == "nc::gui::(anonymous namespace)")
STOP

	TypePtr iClass0(nullptr);//owner class

	//ExpTypesMap	&expTypesMap(mrDC.project().expTypesMap());//mTypeLookup

	if (aClass.size() > 1)//scoped
	{
		FullName_t aScope(aClass.scope());
		iClass0 = ProcessClass(aScope, iNesting + 1, bExporting, false);
		assert(iClass0);
		if (ModuleOf(iClass0) != ModulePtr())
		{
			//the containing class is in another module, try to relocate it to the current
			TypePtr iTop0(TypeTop(iClass0));
			assert(iTop0 == iClass0);//later
			if (CanRelocateType(iTop0))
			{
				RelocateType(iClass0, aScope);
			}
			else//failed to relocate - create a duplicate in the current module
			{
				iClass0 = MakeStruct(aScope.back(), nullptr, 1/*iNesting*/);//should be added to the EXPORT file
				AddTypeObj(iClass0);//in file
				projx().addExpTypesMap(aScope.join(), iClass0);
			}
		}
	}

	//FileInfo_t FI(*this, mrFileDef);
	//FileInfo_t &FI(*this);

	//First, look for the name in current module (can be a proxy)
	TypePtr iClass(projx().findExpTypeMap(sClass, ModulePtr(), true));
//!	assert(FindStrucFast(sClass) == iClass);

	if (!iClass)
	{
		//Not found in the current module. Check if it is already declared in some other module

		//ExpTypesMapIt i(projx().getExportedTypeInfoIt(sClass, ModulePtr()));
		TypePtr iExtType(projx().findExpTypeMap(sClass, ModulePtr(), false));//external
//CHECK(sClass == "QIODevice")
//STOP
		//if (i == projx().exp TypesMap().end())
		if (!iExtType)
		{
			//Nowhere seen. Create a new struc in current module.

			TypePtr pClass(MakeStruct(aClass.back(), iClass0, bIsStruc ? 0 : 1/*iNesting*/));//should be added to the EXPORT file
			AddTypeObj(pClass);//in file
			projx().addExpTypesMap(sClass, pClass);
			//if (bExporting)
				//exp.set(projx().addExportedTypeInfo(sClass, pClass));
			m_newTypes.push_back(pClass);
			iClass = pClass;
		}
		else
		{
			//Found in another module but used in the current - try to relocate it to the current one, but make a proxy in another module.
			TypePtr iTop(TypeTop(iExtType));
//CHECK(iTop != i->second)
//STOP
			if (CanRelocateType(iTop))
			{
				assert(iTop == iExtType);//later
				iClass = RelocateType(iTop, aClass);
			}

			if (!iClass)
			{
				//insert a dupicated entry (symbol declared in several modules)
				TypePtr pClass(MakeStruct(aClass.back(), iClass0, 1/*iNesting*/));//should be added to the EXPORT file
				AddTypeObj(pClass);
				projx().addExpTypesMap(sClass, pClass);
				//if (bExporting)
					//exp.set(projx().addExportedTypeInfo(sClass, exp.typeObj(), bExporting));
				m_newTypes.push_back(pClass);
				iClass = pClass;
			}
#if(0)
			else
			{
				//export map entry must be adjusted after relocation
				// if (iClass != i->second)
				if (i->second->typeProxy() && !i->second->isExp orting())
					i->second = iClass;
				else
				{
					assert(i->second == iClass);
				}
				assert(iClass->isExp orting());
				//iClass->setEx porting(true);
			}
#endif
		}
	}
	else
	{
		if (iClass->typeProxy())
		{
CHECKID(iClass, 0x2cfeb)
STOP
#if(NO_TYPE_PROXIES)
			TypePtr pIncumb(iClass->typeProxy()->incumbent());

			TypePtr iTop(TypeTop(pIncumb));
			TypePtr iClass2(nullptr);
			if (CanRelocateType(iTop))
			{
				assert(iTop == pIncumb);//later
				iClass2 = RelocateType(pIncumb, aClass);
			}

			if (!iClass2)
			{
				//insert a dupicated entry (symbol declared in several modules)
				TypePtr pClass(MakeStruct(aClass.back(), iClass0, 1/*iNesting*/, 1));//force the name - there can be a proxy already
				AddTypeObj(pClass);
				projx().addExpTypesMap(sClass, pClass);
				//if (bExporting)
					//exp.set(projx().addExportedTypeInfo(sClass, pClass, bExporting));
				iClass2 = pClass;
			}
			//assert(iClass2->isExp orting());
			return iClass2;
#else

			//check if there is one non-proxy of this module
			for (ExpTypesMapIt i(GetExportedTypeInfoIt2(sClass)); i != projx().ex pTypesMap().end(); i++)
			{
				MyString s;
				ChopName(i->first, s);
				if (s != sClass)
					break;
				if (ModuleOf(i->second) == ModulePtr())
				{
					assert(!i->second->typeProxy());
					iClass = i->second;
					return iClass;
				}
			}

			TypePtr pIncumb(iClass->typeProxy()->incumbent());
			TypePtr iModule2(ModuleOf(pIncumb));
			DcInfo_t DI2(*DCREF(iModule2));
			assert(pIncumb->isExp orting());
			pIncumb = SkipModifier(pIncumb);//typedef
			assert(!pIncumb->nameless());
			MyString s(DI2.TypeNameFull0(pIncumb));
			ExpTypesMapIt i(GetExportedTypeInfoIt(s));
			assert(i != projx().ex pTypesMap().end());

			TypePtr iTop(TypeTop(i->second));
//CHECK(iTop != i->second)
//STOP
			TypePtr iClass2(nullptr);
			if (CanRelocateType(iTop))
			{
				assert(iTop == i->second);//later
				iClass2 = RelocateExTypeInfo(sClass, i->second, iClass);
			}

			if (iClass2)
			{
			//export map entry must be adjusted after relocation
//?			if (iClass != i->second)
				i->second = iClass2;
				assert(iClass2->isEx porting());
			}
			else
			{
				//insert a dupicated entry (symbol declared in several modules)
				TypePtr pClass(MakeStruct(aClass.back(), iClass0, iNesting, 1));//force the name - there can be a proxy already
				AddTypeObj(pClass);//file
				AddCachedType(sClass, pClass);
				//if (bExporting)
				//	exp.set(projx().addExportedTypeInfo(sClass, pClass, bExporting));
				//types().push_back(pClass);
				iClass2 = pClass;
			}

			iClass = iClass2;
#endif
		}
		/*if (m_fileInfo.IsEmptyStruc(iClass))
		{
			//got an empty struc; about to make it non-empty; move it down below out of the forward declaration section;
			//m_fileInfo.CheckForwardDeclaration(iClass);
		}*/
	}
	return iClass;
}

TypePtr TypeFomNode_t::ProcessTypedef(FullName_t aName, TypePtr iBaseType)//from arguments types : maybe declared in another module
{
	TypePtr pOwnerClass(PrimeSeg());//owner class

	if (aName.size() > 1)
	{
		pOwnerClass = ProcessCompound(aName.scope(), 1, false);
		if (!pOwnerClass)
			return nullptr;//?
	}

	return AddTypedefEx(iBaseType, aName.back(), pOwnerClass);
}

FullName_t DcInfo_s::SplitScopedName(const MyString& s)//sClass is a full name
{
	FullName_t aName;
	if (!s.empty())
	{
		size_t n(ScopePos(s));
		if (n != MyString::npos)
		{
			MyString sClass0(s.substr(0, n));
			aName = std::move(SplitScopedName(std::move(sClass0)));
			aName.append(s.substr(n + 2));
		}
		else
			aName.append(s);
	}
	return std::move(aName);
}

TypePtr TypeFomNode_t::ProcessCompound(FullName_t aClass, int iNesting, bool bExporting, E_KIND eClass)//from arguments types : maybe declared in another module
{
	MyString sClass(aClass.join());
CHECK(sClass == "Qt::DockWidgetArea")
STOP
//CHECK(sClass == "std::basic_istream<unsigned short,std::char_traits<unsigned short> >::_Sentry_base\t42500\r")
//STOP

	//ExpTypesMap	&expTypesMap(mrDC.project().expTypesMap());//mTypeLookup

	//first try to find it in current module by name (can be sClass proxy)
	TypePtr iClass(projx().findExpTypeMap(sClass, ModulePtr(), true));
//!	assert(FindStrucFast(sClass) == iClass);
	if (iClass)
	{
#if(NO_TYPE_PROXIES)
		if (iClass->typeProxy())
			return iClass->typeProxy()->incumbent();
#endif
		if (iNesting == 0 && iClass->typeNamespace())
			AssureTypeClass(iClass);//convert from namespace
		return iClass;
	}

	TypePtr iClass0(nullptr);//owner class

	if (aClass.size() > 1)//has a scope
		iClass0 = ProcessCompound(aClass.scope(), iNesting + 1, bExporting);

	TypePtr pExpType(projx().findExpTypeMap(sClass, ModulePtr(), false));//this map doesn't contain refs to proxies
	if (!pExpType)
	{
		if (iClass0 && ModuleOf(iClass0) != ModulePtr())//nested. the containing class is not of this module
		{
			//owner class found in another module => the nested one must be there as well
			TypePtr iClass2(iClass0);
			FolderPtr iFolder2(USERFOLDER(iClass2));

			TypePtr iModule1(ModuleOf(iClass2));
			TypePtr iModule2(ModuleOf(iFolder2));

			Dc_t *pDc2(DcFromFolder(*iFolder2));
			FileInfo_t FI2(*pDc2, *FILEDEF(iFolder2), memMgrGlob());

			/*if (iModule1 != iModule2)
			{
				TypePtr iClass(FI2.FindCachedType(sClass, false));//is there a nested type in that module?
				STOP

				//assert(0);//declared in other module than resides in?
				/ *if (CanRelocateType(iClass2))
				{
					//try to relocated a c ontaing class
					assert(TypeTop(iClass2) == iClass2);//later
					iClass2 = RelocateType(iClass2, sClass);
				}* /
			}*/

			pExpType = FI2.MakeStruct(aClass.back(), iClass2, iNesting, 0, eClass);//should be added to the EXPORT file (move it to the forward declaration section - file's top)
			FI2.AddTypeObj(pExpType, FileDef_t::AT_HEAD);
			assert(pExpType);
			/*FI2.dc().*/projx().addExpTypesMap(sClass, pExpType);
			//if (bExporting)
				//exp.set(projx().addExportedTypeInfo(sClass, exp.typeObj(), bExporting));
			//a proxy to be created next...
		}
		else
		{
			node_t *pn(mrStubsPool.findProtoEx(sClass, mrDC.moduleFolder()->name()));//first in module
			if (pn && pn->type != NODE_TYPEDEF && pn->type != NODE_STRUCT)
				pn = nullptr;
			if (pn)
				pExpType = process(pn, I_Front::SYMK_NULL);
			if (!pExpType)
			{
				pExpType = MakeStruct(aClass.back(), iClass0, iNesting, 0, eClass);//should be added to the EXPORT file (move it to the forward declaration section - file's top)
				AddTypeObj(pExpType, FileDef_t::AT_HEAD);
				projx().addExpTypesMap(sClass, pExpType);
			}
			assert(pExpType);
			m_newTypes.push_back(pExpType);
			return pExpType;
		}
	}

	assert(!OfTheSameModule(USERFOLDER(pExpType), mrFileDef.ownerFolder()));//current module already has been searched

	//if (!pExpType->isNested())
	{
		//found in another module, maybe a reference or declaration - doesn't matter;
		//create a proxy type in current module (in import file)
		//ALL PROXIES MUST? RESIDE IN IMPORT FILES
		FolderPtr pImpFolder(AddFileEx(FPATH_FROM_IMPORTED));
		FileDef_t *pImpFile(AssureFileDef(pImpFolder, false));
		FileInfo_t FI3(mrDC, *pImpFile, memMgrGlob());

//CHECKID(pExpType, 0xe75e)
//STOP
		if (!FI3.FindProxyOf(pExpType))
		{
			TypePtr pProxy(FI3.MakeProxyTypeTo(pExpType));
			if (pProxy)
			{
				assert(pProxy->owner() == PrimeSeg());
				FI3.RenameProxyType(pProxy, "");
				FI3.AcquireProxyType(pProxy);
			}
		}
	}

	return pExpType;
//#endif
}

////////////////////////////////////////////// TypeFomNode_t

/*TypeFomNode_t::TypeFomNode_t(FileInfo_t &r, const Frame_t &f, const StubPool_t &rStubsPool)
	: FileInfo_t(r),
	m_ctx(f),
	mrStubsPool(rStubsPool),
	//mrFileDef(r.FileDef()),
	mpThisPtr(nullptr),
	//mrField(rField),
	m_va(0),
	mbExporting(false)
{
}*/

TypeFomNode_t::TypeFomNode_t(FileInfo_t &r, /*FieldPtr pField, */const StubPool_t &rStubsPool, bool bExporting)//, ExpTypesMap &typeLookup)
	: FileInfo_t(r),
	//mpField(pField),
	mrStubsPool(rStubsPool),
	mpThisPtr(nullptr),
	//m_va(va),
	mbExporting(bExporting),
	mRawOff(0)//,
	//mTypeLookup(typeLookup)
{
	//assert(m_ctx.pField->address() == va);//phantom modules?
}

TypePtr TypeFomNode_t::OnVTable(I_Front::SymbolKind eKind, GlobPtr pGlob)
{
	TypePtr pType(nullptr);
	switch (eKind)
	{
	case I_Front::SYMK_VFTABLE:
		pType = VFTableOf();
		break;
	case I_Front::SYMK_VBTABLE:
		pType = VBTableOf();
		break;
	case I_Front::SYMK_LVFTABLE:
		pType = LVFTableOf();
		break;
	case I_Front::SYMK_CVFTABLE:
		pType = CVFTableOf();
		break;
	default:
		break;
	}

	if (!pType)
		return nullptr;

	//looks like there has to be a base class.
	//so instantiate it, even though a vtable can't keep a ref to it.
	/*std::string sfx;
	if (eKind == I_Front::SYMK_CVFTABLE)
		sfx = "`construction vftable'{for `";
	else if (eKind == I_Front::SYMK_VBTABLE)
		sfx = "`vbtable'{for `";
	else
		sfx = "`vftable'{for `";

	if (!sfx.empty())
	{
		MyString sName;//? (node->name);
		size_t n(sName.find(sfx));
		if (n != std::string::npos)
		{
			size_t n2(sName.rfind('\''));
			if (n2 != std::string::npos)
			{
				n += sfx.length();
				MyString sBase(sName.substr(n, n2 - n));
				FullName_t aBase(SplitScopedName(sBase));
				TypePtr pTypeType(ProcessCompound(aBase, 0, mbExporting, E_KIND_STRUC));
				AssureTypeClass(pTypeType);
				STOP
			}
		}
	}*/

	TypePtr pClass(pGlob->ownerScope());
	if (AssureTypeClass(pClass))//enforce - must be not a namespace
	{
		if (eKind == I_Front::SYMK_VFTABLE)
		{
			ClassInfo_t classInfo(*this, pClass);
			classInfo.MakeMemberVTable(pGlob);
		}
	}

	return pType;
}

TypePtr TypeFomNode_t::process(I_Front::SymbolKind eKind, GlobPtr pGlob)
{
	if (eKind < I_Front::SYMK_SPECIAL)
		return nullptr;
	return OnVTable(eKind, pGlob);
}

TypePtr TypeFomNode_t::process(node_t *node, I_Front::SymbolKind eKind, GlobPtr pGlob)
{
	if (!node)
		return process(eKind, pGlob);

	//assert(mrFileDef.ownerFolder());
//CHECKID(mpField, 1155)
//STOP

	TypePtr iOwnerSeg(PrimeSeg());
	//NamesMgr_t &rNS(NamespaceInitialized(iOwnerSeg));

	switch (node->type)
	{
	case NODE_STRUCT:
	{
		FullName_t aName(SplitScopedName(node->name));
		TypePtr iStruc(ProcessClass(aName, 0, false, true));
		if (iStruc)
		{
			ADDR off(0);
			while (node->right)
			{
				node = node->right;
				FieldPtr pField(memMgrGlob().NewField(0));
				if (!InsertFieldAt(iStruc, pField, off))
					ASSERT0;
				TypePtr iType(process(node->left, I_Front::SYMK_NULL));
				SetType(pField, iType);
				SetFieldName(pField, node->name.c_str());
				off += iType->sizeBytes();
			}
		}
		return iStruc;
	}
	case NODE_TYPEDEF:
	{
		TypePtr iType(process(node->left, I_Front::SYMK_NULL));
		FullName_t aName(SplitScopedName(node->name));
		iType = ProcessTypedef(aName, iType);
		if (mbExporting)//this means - an import is being processed
			mrDC.project().addExpTypesMap(node->name, iType);
		return iType;
	}

	case NODE_FUNC:
		if (pGlob)
			return (GlobToTypePtr)OnFunc(node, pGlob);
		return OnFuncType(node);

	case NODE_NULL://data?
		return OnData(node, eKind, pGlob);


	case NODE_TYPE:
	{
		if (node->isCompound())
			return OnStruc(node);
//		if (node->isEnum())
//			return GetStockType(OPTYP_UINT32);
		MyString s(node->name);//type_name
		if (node->isUnsigned())
			s.insert(0, "unsigned ");
		OpType_t optyp;
		if (Str2OpTyp(s.c_str(), optyp))
		{
			if (optyp == OPTYP_NULL)
				return nullptr;
			return GetStockType(optyp);
		}
		TypePtr iType(OnType(s));
		if (!iType)
			if (node->isTerminal() && node->hasName())//some compound type?
				if (node->parent)//top-level nodes without a parent normally are data names
					iType = OnStruc(node);
		if (iType && iType->isEnum())
			iType = EnumOf(iType, OPTYP_UINT32);
		return iType;
	}

	case NODE_PTR:
	{
		TypePtr iType(process(node->left, I_Front::SYMK_NULL));
		//CHECKID(iType, -130)
		//STOP
		return PtrOf(iType);
	}

	case NODE_REF://convert into const ptr? returned const ptrs seem do work like no const
	{
		TypePtr iType(process(node->left, I_Front::SYMK_NULL));
		return RefOf(iType);
	}

	case NODE_RVAL_REF:
	{
		TypePtr iType(process(node->left, I_Front::SYMK_NULL));
		return RvalRefOf(iType);
	}

	case NODE_PTR_TO_MEMBER:
	{
		TypePtr iType(process(node->left, I_Front::SYMK_NULL));//???FIXME(later)
		return PtrOf(iType);
	}

	case NODE_ARRAY:
	{
		TypePtr iType(process(node->left, I_Front::SYMK_NULL));
		assert(node->right->type == NODE_NUMBER);
		unsigned num(atoi(node->right->name.c_str()));
		return ArrayOf(iType, num);
	}

	case NODE_VALIST:
		break;

#if(NO_BS)
	case NODE_THUNK:
	case NODE_VTHUNK:
	case NODE_NVTHUNK:
	{
		break;
	}

	case NODE_STRING:
	{
		TypePtr iType(GetStockType(OPTYP_CHAR8));
		TypesTracer_t TT(*this, *findTypeMgr(iType));
		return TT.stringOf(iType, mRawOff, -1);//range?
	}
#endif

	default:
		//assert(0);
		break;
	}

	return nullptr;
}

GlobPtr TypeFomNode_t::OnFunc(node_t *node, GlobPtr iGlob)
{
	assert(node->type == NODE_FUNC);
CHECK(node->name == "localtime")
STOP

//	GlobPtr ifDef0(nullptr);
	assert(!node->parent);
/*	{
		FieldPtr pField0(CloneLead(pField));
		if (pField0->type Proc())
			ifDef0 = GlobFuncObj(pField);
				//return false;
	}*/

//CHECKID(pField, 0x3252)
//STOP

	//GlobPtr iGlob(GlobObj(pField));

	//assert(!iGlob || iGlob->typeFuncDef());

	if (!iGlob->func())
		AssureFuncDef(iGlob);//func defs must not be added to the types map!

	bool bVirtual(false);
	bool bStatic((node->attr & NodeAttr__SV_MASK) == NodeAttr_STATIC);
	bVirtual = (node->attr & NodeAttr__SV_MASK) == NodeAttr_VIRTUAL;
//CHECK(bVirtual)
//STOP
	bool bThisCall(node->isThisCall());
	if (!bThisCall)
		bThisCall = bVirtual;//virtuals are always thiscall
	bool bConst(node->isConst());
	assert(!bStatic || !(bThisCall || bVirtual || bConst));
#if(IMPLICIT_STATICS)
	if (!bStatic && !(bThisCall || bVirtual || bConst))
		bStatic = true;
#endif

	TypePtr iClass(nullptr);
	if (ProtoInfo_t::FuncStatus(iGlob) == 0)
	{
		ProtoInfo_t FI(dc(), iGlob);

		//CHECK(node->name.find("qInstallMsgHandler") != MyString::npos)
		//STOP

		//check class membership
	//CHECK(n == MyString::npos)
	//STOP

		FullName_t aName(SplitScopedName(node->name));
		if (aName.size() > 1)//scoped
		{
			MyString sFunc(aName.back());

			iClass = ProcessClass(aName.scope(), 0, mbExporting, false);
			if (!iClass)
				return nullptr;//?


			//CHECKID(iClass, 0x472f)
			//STOP
			assert(!iClass->typeProxy());
			if (!bStatic)
			{
				if (bConst)
					iGlob->flags() |= FDEF_CONST;
				if (bThisCall)
					iGlob->flags() |= FDEF_THISCALL;
				//if (bVirtual)
					//iGlob->flags() |= FDEF_VIRTUAL;
				/*else*/ if (bStatic)
					iGlob->flags() |= FDEF_STATIC;

				bool bMaybeClass((iGlob->flags() & unsigned(FDEF_THISCALL | FDEF_STATIC)) != 0 || bVirtual || bConst);//? statics in namespace?
				if (!bMaybeClass && iClass->name())
				{
					bool bDestructor(false);
					if (sFunc.startsWith("~"))
					{
						sFunc.erase(0, 1);
						bDestructor = true;
					}
					if (strcmp(iClass->name()->c_str(), sFunc.c_str()) == 0)//maybe constructor
						bMaybeClass = true;
				}

				if (!iClass->typeNamespace() || bMaybeClass)
				{
					AssureTypeClass(iClass);
					FieldPtr pThisArg(AssureThisPtrTo(iGlob, iClass, bConst));
					//!assert(!mpThisPtr);
					if (!mpThisPtr)
					{
						//CHECKID(pThisArg, 59963)
						//STOP
						mpThisPtr = pThisArg;
					}
					else
						PrintError() << "Pointer to memeber unsupported: '" << node->name << "'" << std::endl;
					assert(!iClass->typeProxy());
				}
			}
			else
			{
				STOP
			}
		}

		//CHECKID(pField, 0x100bb3)
		//STOP

		//	if (ifDef0)//only statics map needs to be re-created
			//	return nullptr;

		if (node->isStdCall() || (node->isThisCall() && !node->isCDecl()))
			ProtoInfo_t::SetFuncCleanArged(iGlob, true);

		//create args
		//int iOffs(dc().PtrSize());//skip a return address
		node_t* arg(node->right);
		while (arg && arg->type == NODE_COMMA)
		{
			node_t* pa(arg->left);
			TypePtr iArgType(process(pa, I_Front::SYMK_NULL));
			if (!iArgType)
			{
				PrintError() << "Bad type for function argument" << std::endl;
				arg = nullptr;//stop processing args
				break;//return nullptr;
			}
			//assert(iArgType);
			ProtoInfo_t::AppendFuncArgFlags flags(pa->isRegister() ? ProtoInfo_t::AFA_REGISTER : ProtoInfo_t::AFA_NULL);
			FI.AppendFuncArg(iArgType, flags);
			arg = arg->right;
		}
		if (arg)
		{
			//the last one
			TypePtr iArgType(process(arg, I_Front::SYMK_NULL));
			if (iArgType)
			{
				ProtoInfo_t::AppendFuncArgFlags flags(arg->isRegister() ? ProtoInfo_t::AFA_REGISTER : ProtoInfo_t::AFA_NULL);
				FI.AppendFuncArg(iArgType, flags);
			}
			else if (arg->type == NODE_VALIST)
			{
				ProtoInfo_t::SetFuncVarArged(iGlob, true);
			}
		}

		//add ret val
		if (node->left)
		{
			TypePtr iRetType(process(node->left, I_Front::SYMK_NULL));
			CFieldPtr pRet(FI.AppendFuncRet(iRetType));

			int fpuIn(0);
			int fpuDiff(0);

			if (pRet && pRet->isTypeSimple(OPTYP_REAL64))
				fpuDiff = 1 * FTOP_STEP;//(OPTYP_REAL64 & OPSZ_MASK);

			CallingConv_t CC(FI, iGlob);
			FuncCCArgsCIt<> itArgs(FI.FuncDef(), CC);
			for (; itArgs; ++itArgs);
			fpuIn = itArgs.fpuInTotal();

			FI.FuncDef().setFStackPurge(fpuIn - fpuDiff);
		}

		ProtoInfo_t::SetFuncStatus(iGlob, FDEF_DEFINED);
	}
	else
	{
		iClass = OwnerScope(iGlob);
	}


	//must be here after args are assigned types (as namespaces may be converted to classes affecting this func's args list)
	if (iClass)
	{
		AssureTypeClass(iClass, !bVirtual);//thru ns first
		ClassInfo_t classInfo(*this, iClass);
		//?AssureTypeClass(iClass);
		if (bVirtual)
			classInfo.AddClassVirtualMember(iGlob);
		else
			classInfo.AddClassMember(iGlob);
	}
	
	return iGlob;
}

TypePtr TypeFomNode_t::OnFuncType(node_t *node)
{
	assert(node->type == NODE_FUNC);
//CHECK(node->name == "Gdk::AtomStringTraits::to_cpp_type[abi:cxx11]")
//CHECK(node->name.find("qInstallMsgHandler") != MyString::npos)
//STOP

	//TypePtr ifDef0(nullptr);

	//TypePtr ifDef(NewFuncDef(true);
	
	TypesTracer_t TT(*this, memMgrGlob(), *PrimeSeg()->typeMgr());

	unsigned flags(0);

	//FuncInfo_t funcInfo(dc(), *ifDef);

	//check class membership
	/*size_t n(ScopePos(node->name));
	if (n != MyString::npos)
	{
		MyString sFunc(node->name.substr(n + 2));
		MyString sClass(node->name.substr(0, n));
		if (!sClass.empty())
		{
			TypeFomNode_t &FI(*this);//?	dc(), mrFileDef, memMgrGlob());
			TypePtr iClass(FI.ProcessClass(sClass, 0, mbExporting));
			if (!iClass)
				return nullptr;

			bool bStatic((node->attr & NodeAttr__SV_MASK) == NodeAttr_STATIC);
			bool bVirtual((node->attr & NodeAttr__SV_MASK) == NodeAttr_VIRTUAL);
			bool bThisCall(node->isThisCall());
			if (!bThisCall)
				bThisCall = bVirtual;//virtuals are always thiscall
			bool bConst(node->isConst());
			assert(!bStatic || !(bThisCall || bVirtual || bConst));
#if(IMPLICIT_STATICS)
			if (!bStatic && !(bThisCall || bVirtual || bConst))
				bStatic = true;
#endif

			assert(!iClass->typeProxy());
			if (bStatic)
			{
				//TypePtr iClass2(SkipModifier(iClass));
				if (AddClassMember(iClass, pField))//AddStaticMember
				{}//if (node->hasName())
						//ApplyPrettyName(m_ctx.field(), node->name, iClass2);
			}
			else
			{
				if (bConst)
					ifDef->flags() |= TYP_FDEF_ CONST;
				if (bThisCall)
					ifDef->flags() |= FDEF_THISCALL;
				if (bVirtual)
					ifDef->flags() |= FDEF_VIRTUAL;
				 if (bStatic)
					ifDef->flags() |= FDEF_STATIC;

				bool bMaybeClass((ifDef->flags() & unsigned(TYP_FDEF_ CONST | FDEF_VIRTUAL | FDEF_THISCALL | FDEF_STATIC)) != 0);//? statics in namespace?
				if (!bMaybeClass && iClass->name())
				{
					bool bDestructor(false);
					if (sFunc.startsWith("~"))
					{
						sFunc.erase(0, 1);
						bDestructor = true;
					}
					if (strcmp(iClass->name()->c_str(), sFunc.c_str()) == 0)//maybe constructor
						bMaybeClass = true;
				}

				if (!ifDef0)
				{
					if (!iClass->typeNamespace() || bMaybeClass)
					{
						FieldPtr pArg(funcInfo.AssureThisPtrTo(iClass));
						//!assert(!mpThisPtr);
						if (!mpThisPtr)
							mpThisPtr = pArg;
						else
							PrintError() << "Pointer to memeber unhandled: '" << node->name << "'" << std::endl;
						assert(!iClass->typeProxy());
					}
					else
					{
						AddClassMember(iClass, pField);//AddStaticMember
					}
				}
			}
		}
	}

	if (ifDef0)
		return nullptr;*/

	if (node->isStdCall())
	{
		flags |= TypeFunc_t::E_CLEANARG;
		//funcInfo.SetFuncCleanArged(ifDef, true);
	}

	//create args
	TypePtr iArgs(OnFuncTypeArgs(TT, node->right, flags));

	//add ret val
	TypePtr iRetVal(nullptr);
	if (node->left)
		iRetVal = process(node->left, I_Front::SYMK_NULL);

	return TT.funcTypeOf(iRetVal, iArgs, flags);
}

TypePtr TypeFomNode_t::OnFuncTypeArgs(TypesTracer_t&TT, node_t *arg, unsigned &flags)
{
	if (arg && arg->type == NODE_COMMA)
	{
		TypePtr iLeft(process(arg->left, I_Front::SYMK_NULL));
		if (!iLeft)
		{
			PrintError() << "Bad type for function argument" << std::endl;
			return nullptr;
		}
		TypePtr iRight(OnFuncTypeArgs(TT, arg->right, flags));
		if (!iRight)
			return iLeft;
		//funcInfo.AppendFuncArg(iArgType, false);
		return TT.pairOf(iLeft, iRight);
	}
	if (arg)
	{
		//the last one
		TypePtr iLast(process(arg, I_Front::SYMK_NULL));
		if (iLast)
			return iLast;

		if (arg->type == NODE_VALIST)
			flags |= TypeFunc_t::E_VARIARDIC;//nullptr will be returned but it's ok
	}
	return nullptr;
}

TypePtr TypeFomNode_t::OnStruc(node_t *node)
{
	TypeFomNode_t &FI(*this);//dc(), mrFileDef, memMgrGlob());
	FullName_t aName(SplitScopedName(node->name));
	return FI.ProcessCompound(aName, 0, mbExporting, node->isEnum() ? E_KIND_ENUM : E_KIND_STRUC);
}

TypePtr TypeFomNode_t::OnData(node_t* node, I_Front::SymbolKind eKind, GlobPtr pGlob)
{
	assert(node->type == NODE_NULL);

	MyString sFullName(node->name);
	bool bGlobalScope(sFullName.startsWith("::"));//in global scope?
	if (bGlobalScope)
		sFullName.erase(0, 2);

	FullName_t aFullName(SplitScopedName(sFullName));

CHECK(sFullName == "CAction::`vftable'")
STOP

	if (!node->parent)
	{
		FieldPtr pField(DockField(pGlob));
		assert(pGlob);

		//check class membership
		if (aFullName.size() > 1)//has a scope
		{
			TypeFomNode_t& FI(*this);//, mrFileDef);
			TypePtr iClass(FI.ProcessClass(aFullName.scope(), 0, mbExporting, false));
			if (!iClass)
				return nullptr;

			TypePtr pClass(SkipModifier(iClass));

			if (pClass->typeClass()//namespaces allowed - it s a static member
				|| AssureTypeNamespace(iClass, false)
				|| AssureTypeClass(iClass))
				//if (AssureTypeClass(pClass, true))
			{
				ClassInfo_t classInfo(*this, SkipModifier(iClass));
				if (classInfo.AddClassMember(pGlob))//AddStaticMember
					if (node->hasName())
						ApplyPrettyName(pGlob, sFullName, classInfo.ClassPtr(), 0);
			}
		}
		else if (!pField->nameless() && node->hasName() && sFullName != pField->name()->c_str())
		{
			assert(!sFullName.empty());
//?9			DelocateFieldName(m_ctx.field());//retronym!
			ApplyPrettyName(pGlob, sFullName, nullptr, 2);
		}

		bool bConst(node->isConst());
		if (bConst)
			pField->flags() |= FLD_CONST;
	}

	if (!node->left && !aFullName.empty())//some compound type?
	{
		if (node->parent)//top-level nodes without a parent normally are data names
		{
			TypeFomNode_t &FI(*this);//dc(), mrFileDef);
			return FI.ProcessCompound(aFullName, 0, mbExporting);
		}
	}
	TypePtr iType(process(node->left, eKind, pGlob));
	if (iType && iType->isVTable())
	{
		//looks like there has to be a base class.
		//so instantiate it, even though a vtable can't keep a ref to it.
		std::string sfx;
		if (iType->tag() == I_Front::SYMK_CVFTABLE)
			sfx = "`construction vftable'{for `";
		else if (iType->tag() == I_Front::SYMK_VBTABLE)
			sfx = "`vbtable'{for `";
		else
			sfx = "`vftable'{for `";

		if (!sfx.empty())
		{
			size_t n(node->name.find(sfx));
			if (n != std::string::npos)
			{
				size_t n2(node->name.rfind('\''));
				if (n2 != std::string::npos)
				{
					n += sfx.length();
					MyString sBase(node->name.substr(n, n2 - n));
					FullName_t aBase(SplitScopedName(sBase));
					TypePtr pTypeType(ProcessCompound(aBase, 0, mbExporting, E_KIND_STRUC));
					AssureTypeClass(pTypeType);
					STOP
				}
			}
		}
	}

	return iType;
}

//returns a folder (export) where symbol resides in (not a top binary folder)
//All types, created during the operation, proxied to the context of current (import) file
void TypeFomNode_t::ProcessExportedEntry(GlobPtr pExpGlob, node_t *pn, I_Front::SymbolKind eKind, const MyStringEx &aName, bool bIsFunc)//in context of exporting module
{
	FieldPtr pExpField(DockField(pExpGlob));
	ProcessEntry(pExpGlob, pn, eKind);
	if (thisPtr())
	{
		assert(pn->type == NODE_FUNC);
		GlobPtr pCFunc((GlobPtr)thisPtr()->owner());
		DcInfo_t DI(*this);
		DI.MakeMemberThisCallFromArg(thisPtr());
		FieldPtr pField(DockField(pCFunc));
		assert(!pField || pField == pExpField);//WHY !pField?
		if (pn->hasName())
		{
			assert(pExpGlob->hasPrettyName());
			//ApplyPrettyName(pExpGlob, pn->name, pExpGlob->ownerScope());
		}
	}
	
	RenameField(pExpField, FromCompoundName(aName));
	CheckUglyName(pExpField, pn ? pn->name : "");
}

////////////////////////////////////////////////////EXPORTS

bool TypeFomNode_t::ProcessExportedEntry(ADDR va, node_t *pn, I_Front::SymbolKind eKind, const MyStringEx &aName, bool bIsFunc)
{
CHECK(va == 0x6bc5a7a6)
//CHECK(pn && pn->name == "CFSAppPath::~CFSAppPath")
STOP
	//unsigned short ordinal,
	const Seg_t &rSeg0(*PrimeSeg()->typeSeg());
	TypePtr iSeg2(rSeg0.findSubseg(va, rSeg0.affinity()));
	if (!iSeg2)
	{
		PrintError() << "Exported entry at " << VA2STR2(va) << " does not map into any segment" << std::endl;
		return false;
	}

	Locus_t loc;
	LocusFromVA(iSeg2, va, loc);
	if (loc.empty())
	{
		PrintError() << "Exported entry at " << VA2STR2(va) << " does not map into any segment (2)" << std::endl;
		return false;
	}

	/*FolderPtr pUFolder(FindModuleFolder("__unresolved_externals"));
	if (pUFolder)
	{
		TypePtr pUModule(ModuleOf(pUFolder));
		FieldPtr pUField(ToExportedField(pUModule, 0xFFFF, aName));
		if (pUField)
		{
			GlobPtr pUGlob(GlobObj(pUField));
			STOP
		}
	}*/

	FieldPtr pField0(loc.field0());
CHECKID(pField0, 40695)
STOP
	if (!pField0)
	{
#if(1)
		DcInfo_t DI(mrDC);
		pField0 = DI.AssureFieldAt(loc, aName, pn);//always primary
		if (!pField0)
		{
			//fprintf(STDERR, "Warning: symbol @%08X not processed: %s\n", va, aName.c_str());
			return false;
		}
#else
		PrintError() << "Missing field for exported entry at " << VA2STR(va) << std::endl;
		return false;
#endif
		pField0->setAttributeFromId(ATTR_EXPORTED);//?
	}

//CHECKID(pField0, 0x3d5)
//CHECK(pField0->isStaticMember())
//STOP
	if (!pField0->type())
	{
		ProjModifier_t PJ(*this);
		if (pn)
		{
			if (pn->type == NODE_FUNC)
				PJ.makeFunc2(loc, false, 0);//make function first
#if(NO_BS)
			else if (pn->type == NODE_THUNK)
				PJ.makeFunc2(loc, false, 0);//PJ.makeThunk(loc, false);
			else if (pn->type == NODE_VTHUNK)
				PJ.makeFunc2(loc, false, 0);//PJ.makeVThunk(loc, false);
			else if (pn->type == NODE_NVTHUNK)
				PJ.makeFunc2(loc, false, 0);//PJ.makeNVThunk(loc, false);
#endif
		}
		else if (bIsFunc)
			PJ.makeFunc2(loc, false, 0);//make function first
	}

	if (IsEntryLabel(pField0))
		pField0 = pField0->owner()->parentField();//this is a func

	FieldPtr pField(pField0);
	//assert(!pField0->isClone());
//?	if (pField0->isCloneMaster())
	{
		//find appropriate collision field to proceed with
//?		assert(pField0->nameless());
		TypePtr iSeg(OwnerSeg(pField0->owner()));
		//ConflictField Map &m(iSeg->typeSeg()->conflictFields());
		//assert(!m.empty());
		FieldPtr pField2(nullptr);
		if (aName)
		{
			int iModuleTag(mrModule.unique());
			ExportPool_t& mexp(projx().exportPool());
			pField2 = mexp.find(FromCompoundName(aName), false);
			if (!pField2)
			{
				pField2 = mexp.findPrefix(FromCompoundName(aName, 1), iModuleTag);//no tag
				if (!pField2)
					pField2 = mexp.findSuffix(FromCompoundName(aName, -1));//no name
			}
		}

//CHECKID(pField2, 0x3b9)
//STOP

		if (!pField2)
		{
			//export-by-ordinal symbols
			//if (!pField2)
			{
				if (!aName.empty(0))
					PrintWarning() << "Failed to process named export entry: " << aName[0] << std::endl;
				else
				{
					assert(!aName.empty(1));
					PrintWarning() << "Failed to process unnamed export entry with ordinal: " << aName[1] << std::endl;
				}
				return false;
			}
		}
		//assert(!pField2->isCloneMaster());
//?		assert(pField2 && pField2->isClone() && pField2->owner() == iSeg);
		pField = pField2;
		assert(ModuleOf(pField) == ModulePtr());
		//loc.setField(pField);
	}

	//bool bNoGlob(false);// pn&& pn->type == NODE_NULL && pn->left && pn->left->type >= NODE__EXTRA);

	FieldExPtr pFieldx(AddGlobToFile2(pField, OwnerFolder()));
	GlobPtr pGlob(GlobObj(pFieldx));

	RenameField(pFieldx, FromCompoundName(aName));
	CheckUglyName(pFieldx, pn ? pn->name : "");

	if (pn)
	{
		if (ProcessEntry(pFieldx->globPtr(), pn, eKind))
		{
			/*if (pFieldx->isVTable())
			{
				STOP
			}*/
			return true;
		}
		//PrintWarning() << "Failed to process symbol: " << aName[0] << std::endl;
		//return false;
	}

	if (pFieldx->isTypeProc())
		AssureFuncDef(pGlob);

	return true;
}

bool TypeFomNode_t::ProcessMapEntry(ADDR va, node_t *pn, I_Front::SymbolKind eKind, const MyString &sMangled)
{
	TypePtr iFrontSeg(PrimeSeg());

	const Seg_t &rSeg0(*iFrontSeg->typeSeg());
	TypePtr iSeg2(rSeg0.findSubseg(va, rSeg0.affinity()));
	if (!iSeg2)
		return false;

	Locus_t loc;
	LocusFromVA(iSeg2, va, loc);

	FieldPtr pField0(AssureFieldAt(loc, sMangled, pn));//always primary
	if (!pField0)
	{
		//fprintf(STDERR, "Warning: symbol @%08X not processed: %s\n", va, sMangled.c_str());
		return false;
	}

	if (!pField0->type() && pn)
	{
		ProjModifier_t PJ(*this);
		if (pn->type == NODE_FUNC)
		{
			//make function first
			PJ.makeFunc2(loc, false, 0);
		}
#if(NO_BS)
		else if (pn->type == NODE_STRING)
		{
	//		PJ.makeData(loc, nullptr, ATTR_ ASCII, true);
			/*pType = MakeArrayOfTypeAttr(loc.struc(), it, pType, loc.back().rawoff());
			if (pType->typeArray())
				pField0->setAttribute(ATTR_ ASCII);*/

		}
#endif
	}

	FieldPtr pField(loc.field0());//may be secondary
	//assert(!pField->isCloneMaster());
	FieldExPtr pFieldx(AddGlobToFile2(pField, OwnerFolder()));

	if (pn)
		ProcessEntry(pFieldx->globPtr(), pn, eKind);

	return true;
}

bool TypeFomNode_t::ProcessDebugSymbolEntry(ADDR va, node_t *pn, I_Front::SymbolKind eKind, const MyString &sMangled, bool bIsFunc)
{
	TypePtr iFrontSeg(PrimeSeg());

	const Seg_t &rSeg0(*iFrontSeg->typeSeg());
	TypePtr iSeg2(rSeg0.findSubseg(va, rSeg0.affinity()));
	if (!iSeg2)
	{
		PrintWarning() << "out of segment address @" << VA2STR(va) << " for " << sMangled << std::endl;
		return false;
	}

	Locus_t loc;
	LocusFromVA(iSeg2, va, loc);
	if (va != loc.va() && loc.field0() && loc.field0()->isTypeProc())
	{
		loc.clear();
		LocusFromVA(iSeg2, va, loc, false);
		ProjModifier_t MI(mrProject, *ModulePtr());
		MI.SplitFunction(loc, false);
		loc.clear();
		LocusFromVA(iSeg2, va, loc);
	}

//CHECK(loc.addr() == 0x5f401100)
CHECK(va == 0x592e0)
STOP

	FieldPtr pField0(AssureFieldAt(loc, sMangled, pn));//always primary
	if (!pField0)
	{
		//fprintf(STDERR, "Warning: symbol @%08X not processed: %s\n", va, sMangled.c_str());
		return false;
	}

CHECKID(pField0, 0x187fa)
STOP
	//assert(!pField0->isClone());
	
	if (!pField0->type())
	{
		if ((pn && pn->type == NODE_FUNC) || bIsFunc)
		{
			//make function first
			ProjModifier_t PJ(*this);
			PJ.makeFunc2(loc, false, 0);
		}
	}

	//if (IsEntryLabel(pField0))
		//pField0 = pField0->owner()->parentField();//this is a func

	FieldPtr pField(loc.field0());//may be secondary
	//assert(!pField->isCloneMaster());

	FieldExPtr pFieldx(AsFieldEx(pField));
	if (!pFieldx)
		pFieldx = AddGlobToFile2(pField, OwnerFolder());

	if (pn)
	{
		if (pFieldx->name())
			pFieldx->setUglyName();

		ProcessEntry(pFieldx->globPtr(), pn, eKind);
	}
	else if (pFieldx->hasUglyName() && !OwnerScope(pFieldx->globPtr()))
	{
		ApplyPrettyName(pFieldx->globPtr(), pFieldx->name()->c_str(), nullptr, 1);
	}

	return true;
}

bool TypeFomNode_t::ProcessEntry(GlobPtr pGlob, node_t *pn, I_Front::SymbolKind eKind)
{
	FieldPtr pField(DockField(pGlob));
//CHECKID(pField, 0x2dbc)
//STOP
//CHECK(pn->name == "std::basic_streambuf<char, std::char_traits<char> >::~basic_streambuf")
//STOP

//CHECK(eKind >= 0x10)
//STOP

	assert(pField);
	assert(!IsEntryLabel(pField));

	//fprintf(stdout, "Exporting object %s: %s\n", l.front().c_str(), l.back().c_str());

	TypePtr iType(process(pn, eKind, pGlob));
	if (!iType)
	{
		if (eKind >= I_Front::SYMK_SPECIAL)
		{
#if(0)
			//Locus_t aLoc;
			//LocusFromVA(pField, aLoc);
			FrontImplSymDump_t fimp(mrDC, pField);
			IFront_Src<I_Module> iface(fimp);

			I_Front* pIFront(IFrontOf(PrimeSeg()));
			pIFront->createz(iface, eKind);
#endif
		}
		return false;//?
	}

//CHECKID(iType, 0x9fbf)
//CHECKID(pField, 0xeaa5)
//STOP

	FuncDef_t *pfDef(iType->typeFuncDef());
	if (pfDef)
	{
		if (thisPtr())
		{
			assert(pn->type == NODE_FUNC);
			DcInfo_t DI(*this);
			DI.MakeMemberThisCallFromArg(thisPtr());
		}
		if (pn->hasName())
			ApplyPrettyName(pGlob, pn->name, pGlob->ownerScope());
		return true;
	}
/*	else
	{
		// Should I compare if 2 definitions identical? //iFuncDef == iType
		//fprintf(stdout, "\tForked exported location: %08X (%s)\n", pField->address(), iFunc->parentField()->name()->c_str());
		resetThisPtr();
		return true;
	}*/

	assert(!iType->typeFuncDef());

	if (TypeObj_t::canConform(iType, pField->type()))
	{
		ClearType(pField);
		SetType(pField, iType);
#if(NO_BS)
		if (pn->type == NODE_STRING)
		{
			/*8				if (pField->isHardNamed())
								DelocateFieldName(pField);*/
			//pField->setAttribute(ATTR_ ASCII);
		}
#endif
		if (pField->type() && pField->type()->isVTable())
		{
			TypePtr iScope(OwnerScope(pGlob));
			if (iScope)
				AssureTypeClass(iScope);
		}
		return true;
	}

	if (iType->refsNum() == 0)
	{
//CHECKID(iType, 0x59ce)
//STOP
		iType->addRef();
		DcCleaner_t<> DC(*this);
		DC.ReleaseTypeRef(iType);

		DcInfo_t dcInfo2(mrDC, memMgrGlob());
		DcCleaner_t<> DC2(dcInfo2);
		DC.destroyUnrefedTypes();//?DC2);
		DC2.destroyUnrefedTypes();//?DC2);
	}

	return true;
}

void TypeFomNode_t::ProcessNewTypes(FileInfo_t &imp)
{
	//make proxies of created types
	if (!newTypes().empty())
	{
		//Folder_t *pImpFolder(mrDC.folderPtr(FPATH_FROM_IMPORTED));
		//some types were created in exporting file; must be proxied in the current (import)
		for (std::list<TypePtr>::const_iterator i(newTypes().begin()); i != newTypes().end(); i++)
		{
			TypePtr iExtType(*i);
CHECKID(iExtType, 0x18c84)
//CHECK(iExtType->typeTypedef() != nullptr)
STOP
			if (!imp.FindProxyOf(iExtType))
			{
				//MyString sName(TypeNameFull(iExtType));
				TypePtr pProxy(imp.MakeProxyTypeTo(iExtType));
				if (pProxy)
				{
					imp.RenameProxyType(pProxy, "");
					imp.AcquireProxyType(pProxy);
				}
			}
		}
	}
}



