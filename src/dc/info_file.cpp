#include "info_file.h"
#include "db/front_impl.h"
#include "db/ui_main.h"
#include "db/type_proxy.h"
#include "db/info_types.h"
#include "dump_file.h"
#include "clean_ex.h"
#include "save_ex.h"
#include "savex_move.h"
#include "expr_simpl.h"
#include "info_class.h"

#ifdef _DEBUG
#define STRUC_CATCH_ID	168206
static void __catch_struc_id(TypePtr p)
{
CHECK(p->ID() == STRUC_CATCH_ID)
STOP
}
#endif

///////////////////////////////////////////////////////
// FileInfo_t

FileInfo_t::FileInfo_t(const DcInfo_t &rDcRef, FileDef_t &rFileDef)
: DcInfo_t(rDcRef, rFileDef.ownsMemory() ? rFileDef.memMgr() : rDcRef.memMgr()),
mrFileDef(rFileDef)
{
	//assert(rFileDef.ownsMemory());
}

FileInfo_t::FileInfo_t(const DcInfo_t &rDc, FileDef_t &rFileDef, MemoryMgr_t &rMemMgr)
: DcInfo_t(rDc, rMemMgr),
mrFileDef(rFileDef)
{
}

FileInfo_t::FileInfo_t(const FileInfo_t &rFI, MemoryMgr_t &rMM)
	: DcInfo_t(rFI, rMM),
	mrFileDef(rFI.FileDef())
{
}

/*FileInfo_t::FileInfo_t(const DcInfo_t &rDcInfo, FileDef_t &rFileDef)
: DcInfo_t(rDcInfo),
mrFileDef(rFileDef)
{
}*/

TypePtr FileInfo_t::TakeTypeFromFile(TypePtr iType) const
{
	iType = mrFileDef.takeType(iType);

CHECKID(iType, 0x17af)
STOP

	//assert(!iType->type Proc());
	if (iType->typeFuncDef())
	{
		assert(0);
		/*assert(!HasMethods(iType));//?
		FieldPtr pFuncField(DockField((CGlobPtr)iType));
		if (pFuncField)
			DetachFuncdef(pFuncField);*/
	}
	else
	{
		if (!iType->typeProxy())
		if (iType->typeClass())
		{
			ClassInfo_t classInfo(*this, iType, memMgrGlob());
			classInfo.ClearMemberList();
		}
	}

	SET_USERFOLDER(iType, nullptr);
	return iType;
}

/*bool FileInfo_t::UndoStatic(FieldPtr pField, TypePtr pClass)
{
	if (!IsStaticMemberFunction(pField))
		return false;
	if (!Remove ClassMember(pClass, pField))
		ASSERT0;
	return true;
}

bool FileInfo_t::UndoFuncdef(FieldPtr pField, TypePtr pClass)
{
	TypePtr ifDef(GlobFuncObj(pField));
	if (ifDef)
		DeleteFuncDef(ifDef, pField);
	else
		Remove ClassMember(pClass, pField);
	return true;
}*/

bool FileInfo_t::TakeType(TypePtr iType) const
{
	assert(iType->objTypeGlob());
	return TakeTypeFromFile(iType) != nullptr;
}

FieldPtr FileInfo_t::RecallGlob(GlobPtr pGlob) const//returns progenitor field
{
	assert(pGlob);
	GlobMapIt itGlob(mrFileDef.findGlobIt(pGlob));
	if (itGlob == mrFileDef.globs().end())
		return nullptr;

	return RecallGlobIt(itGlob);


/*	if (pGlob->func())
	{
		DeleteFuncDef(pGlob);
		//memMgrGlob().Delete(ifDef);
	}
	else if (i->owner())
	{
		ClassInfo_t classInfo(*this, i->owner());
		if (!classInfo.RemoveClassMember(pGlob))
			if (!classInfo.RemoveClassVirtualMember(pGlob))
				if (!classInfo.RemoveVirtualTable(pGlob))//scope ptr?
					ASSERT0;
		pGlob->setOwnerScope(nullptr);//?
	}

	GlobPtr iGlob2(mrFileDef.takeGlobIt(i));
	assert(iGlob2 == pGlob);
//?	DestroyField(pField);

	memMgrGlob2().Delete1(pGlob);*/
}

MyString FileInfo_t::DumpFieldDeclaration(CFieldPtr pField) const
{
	DisplayField_t disp2(true);
	disp2.setFlags0(0);
//?	disp2.setSyncOpLine(HOP());
//	disp2.setCurLine(pDisp->curLine());//?

	FuncDumpFlags_t flags(FUNCDUMP_DECL_FLAGS);
	flags = FuncDumpFlags_t(flags | FUNCDUMP_UGLY_NAME);

	//bool bDone(false);
	if (pField->isTypeImp())
	{
		FileDumper_t fileDumper(*this, disp2, nullptr, nullptr, true);

		CFieldPtr pExpField(ToExportedField(pField));
		if (pExpField)
		{
			TypePtr pModule2(ModuleOf(pExpField));
			TypePtr pScope(DCREF(pModule2)->primeSeg());

			//CGlobPtr ifDef(IsCFuncOrStub(pExpField));
			CGlobPtr pGlob(GlobObj(pField));
			CGlobPtr pExpGlob(GlobObj(pExpField));
			if (pExpGlob && pExpGlob->func())
			{
				fileDumper.DumpFunctionDeclaration0(pExpGlob, flags, pGlob, pScope);
			}
			else
			{
				fileDumper.drawGlobDefinition(pExpGlob, pGlob, pScope, !fileDumper.IsHeader());
			}
			return disp2.finalizeAsString();
		}
		
		//if (fileDumper.drawImpFieldDecl(pField, PrimeSeg()))
			//bDone = true;
	}
	//if (!bDone)
	{
		FileDumper_t fileDumper(*this, disp2, nullptr, nullptr, false);
		assert(!fileDumper.draftMode());
		if (!IsGlobal(pField))
		{
			fileDumper.drawFieldDefinition(pField, nullptr, nullptr, true);
		}
		else
		{
			CGlobPtr pGlob(GlobObj(pField));
			if (pGlob)
			{
				if (pGlob->func())
				{
					fileDumper.DumpFunctionDeclaration0(pGlob, flags, nullptr, nullptr);// PrimeSeg());
				}
				else
				{
					fileDumper.drawGlobDefinition(pGlob, nullptr, nullptr, true);
					if (pField->isTypeSimple())
						fileDumper.DumpGlobInitialization(pGlob);
				}
			}
			else//labels?
			{
				fileDumper.drawFieldName(pField, nullptr, false);
			}
		}

	}

	return disp2.finalizeAsString();
}

TypePtr FileInfo_t::AddTypedefEx(TypePtr baseType, const char *name, TypePtr owner)
{
	assert(owner);
	//const NamesMgr_t &rNS(NamespaceInitialized(owner));//PrimeSeg()));
	const NamesMgr_t& rNS(*owner->typeComplex()->namesMgr());
	ObjPtr pObj(rNS.findObj(name));
	if (pObj)
	{
		TypePtr iType(pObj->objTypeGlob());
		if (iType)
		{
			if (iType == baseType)
				return iType;//something like this: typedef struct name {} name;
			if (iType->isEnum() && baseType->typeEnum()->enumRef() == iType)
				return baseType;//something like this: typedef enum name {} name;
			if (iType->typeTypedef() && iType->typeTypedef()->baseType() == baseType)
				return iType;//already
		}
	}
	TypePtr iType(AddTypedef(baseType, name, owner));
	if (iType && (!iType->isNested() || iType->owner()->typeNamespace()))
		AddTypeToFile(iType, OwnerFolder());
	return iType;
}

void FileInfo_t::AddTypeObj(TypePtr p, unsigned at) const
{
CHECKID(p, 9662)
STOP
	mrFileDef.addTypeObj(p, at);
}

/*void FileInfo_t::AddIntrObj(GlobPtr p)
{
	mrFileDef.addIntrObj(p);
}*/

bool FileInfo_t::OfThisModule(TypePtr p) const
{
	return OfTheSameModule(USERFOLDER(p), FileDef().ownerFolder());
}

bool FileInfo_t::AddImpField(FieldPtr pField) const
{
	assert(pField->isTypeImp());
	TypePtr iType(pField->type());
CHECK(pField->_key() == 0x6f1)
STOP

	/*if (iType->typeFuncDef())//imported func defs are pointers - must be added to the type map
	{
	TypesTracer_t tt(mrProject, *PrimeSeg()->typeMgr());
	tt.addTypeNew(iType);
	iPtr = tt.ptrOf(iType);
	}
	else
	{
	TypesTracer_t tt(mrProject, *iType->ownerTypeMgr());//void*?
	iPtr = tt.ptrOf(iType);
	}*/

	//BinaryCleaner_t PC(*this, mrDC.module());
	//PC.ClearType(pField);
	//SetType(pField, iPtr);

	FieldExPtr pFieldx(AddGlobToFile2(pField, OwnerFolder()));
	//check if the name has to be shared
	if (iType->typeFuncDef() /*&& IsTypeImp(pFieldx)*/ && (GlobToTypePtr)IsPtrToCFunc(pFieldx) == iType)
		iType->setName0(pFieldx->name());

	//CHECKID(pFieldx, 2282)
	//STOP
	//#ifdef _DEBUG
	//					Name(pFieldx);
	//#endif
	return true;
}

#if(!NO_TYPE_PROXIES)
// Converts the exported type from another module into a proxy, transferring the instance into a current
TypePtr FileInfo_t::RelocateExTypeInfo(const char *pcNameFull, TypePtr iSelf, TypePtr iProxy0)
{
	FolderPtr pFolder(USERFOLDER(iSelf));
	Dc_t *pOtherDC(DcFromFolder(*pFolder));
	DcInfo_t dcInfo0(*pOtherDC);

#ifdef _DEBUG
	MyString _thisModule(mrDC.binaryFolderPtr()->name());
	MyString _otherModule(pOtherDC->binaryFolderPtr()->name());
#endif

//CHECKID(iProxy0, 0xa878)
//STOP

CHECKID(iSelf, 0xe75e)
STOP

//CHECKID(iSelf, 14933)
//STOP

	assert(!iSelf->typeProxy());
	assert(!OfTheSameModule(pFolder, FileDef().ownerFolder()));

	//TypePtr iSelf0(SkipModifier(iSelf));
	//Struc_t *pSelf0(iSelf0->typeStruc());
	//assert(pSelf0);
	TypePtr iOwnerClass0(iSelf->owner());

	//first, re-located all nested types

	TypesMgr_t *pTypesMgr(iSelf->typeMgr());
	if (pTypesMgr)
	{
		//remove dereived types (like ptrs, arrays and such, but not typedefs to uglies) from orginal tmap. They are to stay with a proxy.
		std::list<TypePtr> l;
		pTypesMgr->takeDerivedTypes(l);

		const TypesMap_t &m(pTypesMgr->aliases());

		//t-map is modified below, so construct a list to safely work with
		std::list<TypePtr> l0;
		for (TypesMapCIt i(m.begin()); i != m.end(); i++)
			l0.push_back(i.pvt()->pSelf);

		//The (original) types in this t-map are to be turned into proxies below and replaced with newly created incumbents. The derivative times are to be...

		while (!l0.empty())
		{
			TypePtr iType(l0.front());
			l0.pop_front();

			TypePtr iType0(iType->absBaseType());
			//TypePtr iUglyLocum(IsUglyLocum(iType) ? iType : nullptr);//the target could have been re-located already

CHECKID(iType, 17708)
STOP

			MyString sn(dcInfo0.TypeNameFull(iType));
			/*if (!iType->typeStruc() && !iUglyLocum)
			{
				assert(!iType->isExp orting());
				TypePtr iProxy0(iType->stripToProxy());
				assert(iProxy0);
				assert(iType0->ownerTypeMgr() == pTypesMgr);
				if (!pTypesMgr->removeNameAlias(iType))
				ASSERT0;
				iProxy0->ownerTypeMgr()->addNameAlias(iType);
				continue;
			}*/


			bool bExp(iType->isExp orting());
			if (bExp)
			{
				/*if (iUglyLocum)
				{
				assert(!iType0->isExp orting());
				sn = TypeNameFull(iType0);
				}
				else*/
				if (!RemoveExportedTypeInfo(iType, sn))
					ASSERT0;
			}

			TypePtr iType1(RelocateExTypeInfo(sn, iType, nullptr));
			TypePtr iProxy2(iType);
			assert(!iProxy2->isExpo rting());
			assert(iType1);//incumbent, iType become a proxy

			if (bExp)
				AddExportedTypeInfo(iType1, sn, false);

			/*if (IsUglyLocum(iType1) && iType1->baseType()->typeProxy())
			{
			//we've got a typedef(this) => proxy(other) => uglic(this). Must get rid of dangling proxy.
			TypePtr iProxy2(iType1->baseType());
			DcInfo_t DI(*DcFromType(iProxy2));
			DcCleaner_t DC(DI);
			iType0->addRef();
			DC.ReleaseTypeRef(iProxy2);
			iType1->typeTypedef()->setBaseType0(nullptr);
			SetType(*iType1->typeTypedef(), iType0);
			iType0->releaseRef();
			}*/
		}

		//now t-map contains only proxies... Re-unite them with dependent types.

		while (!l.empty())
		{
			TypePtr p(l.front());
			l.pop_front();
			TypePtr px(p->stripToProxy());
			assert(px->typeProxy());
			TypesMgr_t *ptm(px->ownerTypeMgr());
			ptm->addNameAlias(p);
			assert(ptm->contains(p));
		}
	}

	const FilesMgr0_t &files(Project().files());
	fprintf(stdout, "Re-locating class declaration (%s) from %s to %s\n",
		dcInfo0.TypePrettyNameFull(iSelf, CHOP_SYMB).c_str(),
		files.relPath(pFolder).c_str(),
		files.relPath(FileDef().ownerFolder()).c_str());

	//found in another module, not a proxy;
	//should not contain members; must not contain fields for now;
	//must transfer it to the current module, by creating a proxy in current module
	TypePtr iClass(MakeProxyOf(pcNameFull, iSelf, iProxy0));
	//REMEMBER: typerefs are staying with its modules

	//CHECKID(iProxy1, -416)
	//STOP

	//////////////////////
	TypePtr iProxy(iSelf);
	assert(iProxy->typeProxy() && !iClass->typeProxy());
	assert(OfThisModule(iClass));

	//OBJ FLAGS???

	//if found entity was residing in a exporting file, move it to an importing one
	if (pOtherDC->isFolderOfKind(*pFolder, FPATH_FROM_EXPORTED))
	{
		Folder_t *pImpFolder(pOtherDC->folderPtr(FPATH_FROM_IMPORTED));
		if (!pImpFolder)//create import file if it is not already
		{
			pImpFolder = dcInfo0.AddFileEx(FPATH_FROM_IMPORTED);
			assert(pImpFolder);
		}
		AddTypeToFile(iProxy, FILEDEF(pImpFolder));
		//if (iProxy->isEx porting())
		iProxy->setEx porting(false);
	}

	/*if (iClass->isUgly())
	{
	TakeTypeFromFile2(iClass);
	AddTemplatedType(iClass);
	}
	else*/
	{
		FolderPtr pClassFolder(USERFOLDER(iClass));
		if (mrDC.isFolderOfKind(*pClassFolder, FPATH_FROM_IMPORTED))
		{
			FolderPtr pExpFolder(mrDC.folderPtr(FPATH_FROM_EXPORTED));
			assert(pExpFolder == FileDef().ownerFolder());
			AddTypeToFile(iClass, FILEDEF(pExpFolder));
		}
	}

	/*	if (iClass->typeTypedef())
		{//the typedef's target is also sitting in other module's 'templated' folder. Move it from.
		RegainTemplatedType(iClass);
		}*/


	//now, when owner class has been relocated, fix the nested types
	if (pTypesMgr)
	{
		TypePtr iOwnerOld(pTypesMgr->owner());
		assert(iOwnerOld->typeProxy()->incumbent() == iClass);
		pTypesMgr->setOwner(iClass);
		const TypesMap_t &m(pTypesMgr->aliases());
		for (TypesMapCIt i(m.begin()); i != m.end(); i++)
		{
			TypePtr p(i.pvt()->pSelf);
			assert(p->owner()->typeProxy()->incumbent() == iClass);
			p->setOwner(iClass);
		}

		//finally, move the t-map to the current module
		assert(ModuleOf(iOwnerOld));
		assert(ModuleOf(iClass));
		dcInfo0.RegisterTypesMapEx(iOwnerOld, false);//deregister
		RegisterTypesMapEx(iClass, true);
	}

	//make sure the class is exporting
	//	iClass->setExpo rting(true); //sis gonna be set when added to the exp-map
	return iClass;
}
#endif

// Relocates a type object (pType) from another module into current
TypePtr FileInfo_t::RelocateType(TypePtr pType, const FullName_t& sNameFull, bool bCreateProxy) const
{
	FolderPtr pFolder(USERFOLDER(pType));//source folder
	Dc_t* pOtherDC(DcFromFolder(*pFolder));
	DcInfo_t DIfrom(*pOtherDC);//source DC

#ifdef _DEBUG
	MyString _thisModule(mrDC.moduleFolder()->name());
	MyString _otherModule(pOtherDC->moduleFolder()->name());
#endif

	//CHECKID(pType, 0x17c6a)
	//STOP

	assert(!pType->typeProxy());
	assert(!OfTheSameModule(pFolder, FileDef().ownerFolder()));

	TypePtr iOwnerClass0(pType->owner());

	const FilesMgr0_t& files(Files());
	std::cout << "Re-locating class declaration ("
		<< DIfrom.TypePrettyNameFull(pType, CHOP_SYMB) << ") from "
		<< files.relPath(pFolder) << " to "
		<< files.relPath(FileDef().ownerFolder())
		<< std::endl;

	NamesMgr_t* pnm0(OwnerNamesMgr(pType->owner(), nullptr));//old n-map
	NamesMgr_t* pnm(OwnerNamesMgr(PrimeSeg(), nullptr));//new

	if (!pType->isNested())
	{
		//remove a name in another module
		MyString sName;
		if (!pType->nameless())
		{
			sName = pType->name()->c_str();
			if (!RemoveNameRef(*pnm0, pType->name()))
				ASSERT0;
			pType->setName(nullptr);
		}

		TypesMgr_t* ptm0(pType->ownerTypeMgr());//old
		TypesMgr_t* ptm(PrimeSeg()->typeMgr());//new

		//remove a proxy in current module (if any)
		TypePtr iProxy(ptm->removeNameAlias(pType));
		if (iProxy)
		{
			//CHECKID(iProxy, 0x87cdd)
			//STOP
			assert(iProxy->typeProxy());
			DcCleaner_t<> BC(*this);// memMgrGlob());
			BC.setClosing(true);//avoid unlinking of the incumbent
			BC.destroyTypeProxy(iProxy);
			FolderPtr pFolderOld(USERFOLDER(iProxy));
			if (pFolderOld->fileDef()->takeType(iProxy) != iProxy)
				ASSERT0;
			SET_USERFOLDER(iProxy, nullptr);
			iProxy->setOwner(nullptr);
			memMgrGlob().Delete(iProxy);
		}

		//remove type ref in another's module t-map
		if (!ptm0->removeNameAlias(pType))//this also will decrease a refs count
			ASSERT0;

		// add to current module's t-map
		if (!ptm->addNameAlias(pType))
			ASSERT0;

		if (!sName.empty())
			if (!ForceName(*pnm, sName, pType))
				ASSERT0;
	}

	FolderPtr pFolderOld(USERFOLDER(pType));
	if (pFolderOld->fileDef()->takeType(pType) != pType)
		ASSERT0;
	SET_USERFOLDER(pType, nullptr);

	//	FolderPtr pExpFolder(mrDC.folderPtr(FPATH_FROM_EXPORTED));
	//	assert(pExpFolder == FileDef().ownerFolder());

		//add the new type to the file
	if (!pType->typeStruc() || IsEmptyStruc(pType))
		AddTypeObj(pType, FileDef_t::AT_HEAD);
	else
		AddTypeObj(pType);

	//now, when owner class has been relocated, relocate the nested types
	TypesMgr_t* pTypesMgr(pType->typeMgr());
	if (pTypesMgr)
	{
		//remove dereived types (like ptrs, arrays and such, but not typedefs to uglies) from orginal tmap. They are to stay with a proxy.
		//std::list<TypePtr> l;
		//pTypesMgr->takeDerivedTypes(l);
		//assert(l.empty());

		const TypesMap_t& m(pTypesMgr->aliases());

		//t-map is modified below, so construct a list to safely work with
		for (TypesMapCIt i(m.begin()); i != m.end(); i++)
		{
			TypePtr iType(i->pSelf);
			//CHECKID(iType, 17708)
			//STOP
			FullName_t sn(DIfrom.TypeNameFull(iType));
			RelocateType(iType, sn);
		}

		//now t-map contains only proxies... Re-unite them with dependent types.

		/*while (!l.empty())
		{
			TypePtr p(l.front());
			l.pop_front();
			TypePtr px(p->stripToProxy());
			assert(px->typeProxy());
			TypesMgr_t *ptm(px->ownerTypeMgr());
			ptm->addNameAlias(p);
			assert(ptm->contains(p));
		}*/

		//finally, move the t-map to the current module
		DIfrom.RegisterTypesMapEx(pType, false);//deregister
		RegisterTypesMapEx(pType, true);
	}

	//should be here, TypeNameFull() call above relies on this
	if (pType->hasUglyName())//before flags are reset
	{
		//relocate a pretty name to the new module
		PNameRef pnPretty(DIfrom.TakePrettyName(pType));
		if (pnPretty)//the pretty name may be missing from PrettyTypesMap
		{
			MyString s;
			ChopName(pnPretty->c_str(), s);//get rid of the suffix
			assert(!s.empty());
			if (!pType->isNested())//must be put into the new N-map as well
			{
				RemoveNameRef(*pnm0, pnPretty);
				pnPretty = ForcePrettyName(*pnm, s, pType, 1);//re-establish in new N-map
			}
			if (!RegisterPrettyTypeName(pnPretty, pType))
				ASSERT0;
		}
	}

	//create a proxy in old module
	FolderPtr pImpFolder(DIfrom.AddFileEx(FPATH_FROM_IMPORTED));
	FileDef_t *pImpFile(DIfrom.AssureFileDef(pImpFolder, false));
	FileInfo_t FI3(*pOtherDC, *pImpFile, memMgrGlob());
	FI3.CreateProxyTo(pType, sNameFull);

	return pType;
}

//	+ creates a proxy in old module
TypePtr FileInfo_t::CreateProxyTo(TypePtr pType, const FullName_t& sNameFull) const
{
	assert(pType->isNested() || CheckOwnership(pType));

	assert(!FindProxyOf(pType));
	TypePtr pProxy(MakeProxyTypeTo(pType));
	if (pProxy)
	{
		RenameProxyType(pProxy, sNameFull.join());
		AcquireProxyType(pProxy);
	}

	return pProxy;
}

bool FileInfo_t::CanRelocateType(TypePtr iSelf) const
{
	if (!iSelf->typeStruc())
		return true;

	assert(CheckOwnership(iSelf));

	assert(!iSelf->typeProxy());
	assert(ModuleOf(iSelf) != ModulePtr());//not of the same module

	TypePtr iSelf0(SkipModifier(iSelf));
	Struc_t *pSelf0(iSelf0->typeStruc());
	assert(pSelf0);
#if(NESTED_SCOPES)
#if(0)
	if (pSelf0->hasFields())
		throw(-4);//later!
#endif

	if (HasMethods(iSelf0))//used by a module where it belongs to. Can't relocate. Let the caller handle the situation.
		return false;
#else
	if (!IsEmptyStruc(iSelf0))
		throw(-4);
#endif

	TypesMgr_t *pTypesMgr(pSelf0->typeMgr());
	if (pTypesMgr)
	{
		const TypesMap_t &m(pTypesMgr->aliases());
		for (TypesMapCIt i(m.begin()); i != m.end(); i++)
		{
			TypePtr iType(i->pSelf);
			if (!iType->typeStruc())
			{
				TypePtr iBaseType(iType->baseType());
				assert(iBaseType && iBaseType->owner() == iType->owner());
				continue;
			}

			assert(ModuleOf(iType) == ModuleOf(iSelf));//of the same module as its parent

			if (!CanRelocateType(iType))
				return false;
		}
	}

	return true;
}

void FileInfo_t::DeleteFuncDef(GlobPtr pGlob) const
{
CHECKID(pGlob, 0x1174)
STOP

	//FuncInfo_t FI0(DcRef(), *pGlob);// , FileDef_t());//?mrFileDef);
	//assert(pFuncField == FI0.DockField());
	UnmakeMemberMethod(pGlob);
	//assert(!FI0.IsThisCallType());//must have been disconnected by parent ??? memeber??

	ProtoInfo_t DI(*this, pGlob);
	DcCleaner_t<> DC(DI);

	if (!ProtoInfo_t::IsStub(pGlob))
	{
		FuncInfo_t FI(DcRef(), *pGlob);// , mrFileDef);
		FuncCleaner_t FC(FI);
		FC.DestroyBody();
		FC.Cleanup();
		//?FC.CleanupFinal();

		//all unrefed types are owned by global memmgr - must be undone by dc cleaner
		while (FC.hasUnrefedTypes())
			DC.DestroyTypeRef(FC.popUnrefedType());
		//FC.destroyUnrefedTypes();//?DC);
	}

	if (pGlob->hasPrettyName())
	{
		PNameRef pn(TakePrettyName(pGlob));//may have a pretty name
		if (pn)
		{
			DC.DestroyPrettyName(pGlob, pn);
			/*if (!pNS->removen(pn))
				ASSERT0;
			memMgrGlob().Delete(pn);*/
		}
	}

	DC.destroyGlob(pGlob, true);

	//if (pFuncField)
		//DetachFuncdef(pFuncField);
	
	DC.destroyUnrefedTypes();//must be here to properly destroy owner class!

	//if (a.OwnsMemMgr())
	//delete pMemMgr;
}

bool FileInfo_t::DeleteFunc(TypePtr iFunc) const
{
	assert(IsProc(iFunc));
	//Struc_t &rFunc(*iFunc->typeStruc());
	//assert(!rFunc.namesMgr());

	FieldPtr pFuncField(iFunc->parentField());
	GlobPtr pGlob(GlobFuncObj(pFuncField));
	if (pGlob)
	{
		DeleteFuncDef(pGlob);
		memMgrGlob2().Delete1(pGlob);
		return true;
	}
	return false;
}

FieldMap& DcInfo_s::OwnerMap(CFieldPtr p)
{
	assert(IsGlobal(p));
/*	if (p->isClone())
		return p->owner()->typeSeg()->conflictFields();*/
	return p->owner()->typeStruc()->fields();
}

void FileInfo_t::ClearFile(bool bClosing) const
{
	//assert(rSelf.includes().empty());
	//remove itself from other files
	//dc().files().ExcludeFile(rSelf);

	mrFileDef.includes().clear();

	mrFileDef.releaseDeferredTypes();

	while (!mrFileDef.types().empty())
	{
		TakeTypeFromFile(nullptr);//front
	}

	while (!mrFileDef.globs().empty())
	{

		RecallGlobIt(mrFileDef.firstGlobIt());

		//?DestroyField(pField);//why?
	}
}

FieldPtr FileInfo_t::RecallGlobIt(GlobMapIt itGlob) const
{
	GlobPtr iGlob(&(*itGlob));
	//FieldPtr pField(iGlob->dockField());//null for intrinsics
	FieldExPtr pFieldx(FieldEx_t::dockField(iGlob));
	//assert(pField == pFieldx);
//CHECKID(pField, 0x3464)
//STOP

	if (iGlob->func())
		DeleteFuncDef(iGlob);//already detached
	else if (iGlob->owner())
	{
		ClassInfo_t classInfo(*this, iGlob->owner());
		if (!classInfo.RemoveClassMember(iGlob))
			if (!classInfo.RemoveClassVirtualMember(iGlob))
				if (!classInfo.RemoveVirtualTable(iGlob))
					ASSERT0;
	}

	mrFileDef.takeGlobIt(itGlob);

	//if (!bClosing)//revert globs?
	//not an intrinsic - revert back to old field object

	bool bExported(false);
	if (pFieldx->isExported())
		bExported = projx().exportPool().remove(pFieldx);
	FieldMap& m(OwnerMap(pFieldx));//before moved
	FieldPtr pField2(memMgrGlob().NewField(std::move(*pFieldx)));
	//pField is now in prestine state
	if (pField2->name())
	{
		assert(pField2->name()->obj() == pFieldx);
		pField2->name()->setObj(pField2);
	}
	TypePtr pType(pField2->type());
	if (pType && !pType->isShared())
	{
		assert(pType->parentField() == pFieldx);
		pType->setParentField(pField2);
	}
	if (bExported)
		projx().exportPool().add(pField2);

	m.rebind(pFieldx, pField2);

	memMgrGlob2().Delete(pFieldx);
	return pField2;
}

void FileInfo_t::UnloadFuncdefs() const
{
	for (GlobMapIt i(mrFileDef.globs().begin()); i != mrFileDef.globs().end(); ++i)
	{
		GlobPtr pGlob(&(*i));
		if (pGlob->func())
		{
			FuncInfo_t FI(DcRef(), *pGlob);// , mrFileDef);
			FuncCleaner_t FD(FI);
			FD.DetachArgsRefs();//is it needed?
			FD.DestroyBody();
			FD.Cleanup();
			assert(!FD.hasUnrefedTypes());//all types must have been deferred
		}
	}
}

void FileInfo_t::LoadFuncdefs() const
{
	MyString sFile(mrFileDef.dispersedName());
	assert(!sFile.isEmpty());
	MyString sPath(mrProject.path());
	assert(!sPath.isEmpty());

#if(FUNC_SAVE_ENABLED)
	MemmoryAccessorG_t<GlobalSerializerEx_t> SAVELOAD(SR_Loading, memMgrGlob(), mrProject, sPath);
	if (SAVELOAD.LoadDispersed(sFile, *OwnerFolder(), mrDC))
	{
#ifdef _DEBUG
		MyPath fPath(sFile, MyPath(sPath));
		fprintf(stdout, "Deferred file loaded: %s\n", fPath.Path().c_str());
		fflush(stdout);
#endif
	}
#endif
}

size_t FileInfo_t::findIntrinsic(const char *name) const
{
		for (size_t i(0); i < mrDC.mIntrinsics.size(); i++)
	{
		GlobPtr iGlob(mrDC.mIntrinsics[i]);
		if (TypeName((GlobToTypePtr)iGlob) == name)
			return i;
	}
	return (size_t)-1;
}


/*void FileInfo_t::SetDcOwnerSegPtr(TypePtr iDC, TypePtr iSeg)
{
Dc_t &rdC(*iDC->typedC());
if (iSeg)
iSeg->typeSeg()->SetUserData<TypeObj_t>(iDC);
else if (rdC.primeSeg())
rdC.primeSeg()->typeSeg()->SetUserData<Dc_t>(nullptr);
rdC.miFrontSeg = iSeg;
}*/

FieldPtr FileInfo_t::CreateExportedField(const MyStringEx& aName, MyString demangled, bool bIsFunc) const
{
	 //mangled, unsigned short uOrdinal

	TypePtr iFrontSeg(PrimeSeg());
	//NamesMgr_t &rNS(NamespaceInitialized(iFrontSeg));
	NamesMgr_t& rNS(*iFrontSeg->typeComplex()->namesMgr());

	MyString sOrd(aName[1]);
	ADDR uOrd(sOrd.isEmpty() ? 0 : atoi(sOrd.c_str()));//it may be not a number, atoi returns 0 in this case

	ADDR uOffs(ORDINAL_BIAS + uOrd);//address 0 is not allowed!

	assert(IsPhantomModule(ModulePtr()));

	FieldPtr pField(memMgrGlob().NewField(uOffs));
#if(0)
	if (mangled.empty())
		fprintf(stdout, "Unnamed exported entry created with ordinal: %d\n", (int)uOrdinal);
#endif

	Seg_t &rSeg(*iFrontSeg->typeSeg());
	FieldMap &m(rSeg.fields());
CHECK(pField->_key() == 0x7e)
STOP
	FieldMapIt it(InsertUniqueFieldIt(iFrontSeg, uOffs, pField));
	if (it == m.end())
	{
		//SOME IMPORTING MODULE HAS THIS ORDINAL ASSIGNED TO OTHER SYMBOL (IMPORT BY NAME MUST BE ENFORCED)

		assert(IsPhantomModule(ModulePtr()));
		//assert(!mangled.isEmpty());
		pField = AddSecondaryField(iFrontSeg, uOffs, pField);
		//SetHard Name2(pField, mangled, demangled);
		//pField->set AttributeFromId(ATTR_EXPORTED);
		//AddGlobToFile(pField, OwnerFolder());
		//return pField;
	}
	else
	{
		pField->setOwnerComplex(iFrontSeg);
		assert(pField->_key() == uOffs);//pField->setOffset(uOffs);
	}

	FieldExPtr pFieldx(AddGlobToFile2(pField, OwnerFolder()));//don't forget to assign it to the export file
	if (bIsFunc)
		AssureFuncDef(GlobObj(pFieldx));

	pField = pFieldx;

	MyString sName(FromCompoundName(aName));
//CHECKID(pField, 0xa8d)
//STOP
	SetFieldName2(pField, sName, true);
	pField->setHardNamed(true);

	pField->setExported(true);
	if (!projx().exportPool().add(pField))
		ASSERT0;
	//if (!projx().exportPool().checkValid())
		//ASSERT0;
	//FieldPtr pField2 = projx().exportPool().find(pField->name()->c_str());
	return pField;
}

static bool	g_bProgressInfo;

void FileInfo_t::ProgressInfo(const char *s, OpPtr pOp) const
{
	if (!g_bProgressInfo)
		return;
#if(0)
	char buf[80];
	CON.ClearLine();
	sprintf(buf, "%s (%d)", s, pOp->No());
	CON.Write(buf); 
#endif
}

static bool isCName(const char *pc)
{
	if (*pc == '_')
		pc++;
	else
	{
		if (!isalpha(*pc))
			return false;
		pc++;
	}
	for (; *pc; pc++)
	if (!iscsym(*pc))
		return false;
	return true;
}

static bool isTemplatedTypeName(const char *pc)
{
	//	if (*pc == '~')//destructor is OK
	//	pc++;
	for (; *pc; pc++)
	{
		if (isalnum(*pc))
			continue;
		if (*pc != '_')
			return true;
		if (*pc == DUB_SEPARATOR)
			break;//the rest expected to be OK
	}
	return false;
}

static bool isNamespaceAppropriateName(const char *pc)
{
	assert(*pc);
	if (strcmp(pc, "(anonymous namespace)") == 0)
		return true;//special case
	return !isTemplatedTypeName(pc);
}

TypePtr FileInfo_t::MakeStruct(MyString sName, TypePtr pOwnerScope, int iTryNS, int iForceName, E_KIND eClass) const
{
CHECK(sName == "nc")
STOP
	DcInfo_t dcInfo(mrDC, memMgrGlob());

	if (pOwnerScope)
	{
		pOwnerScope = SkipModifier(pOwnerScope);
		assert(ModuleOf(pOwnerScope) == ModulePtr());
		if (!pOwnerScope->typeNamespace())
			iTryNS = 0;//only namespaces may contain other namespaces
	}
	else
		pOwnerScope = PrimeSeg();

	TypesMgr_t *pTypesMgr(dcInfo.AssureTypeMgr(pOwnerScope));//in global memory pool(!)
	TypesTracer_t TT(*this, memMgrGlob(), *pTypesMgr);

	Struc_t *pStruc(nullptr);
#if(RECOVER_NAMESPACES)
	if (iTryNS > 0)
	if (isNamespaceAppropriateName(sName))
		pStruc = new TypeNamespace_t(
#ifdef _DEBUG
		Complex_t::nextNegativeId()
#endif
		);
#endif

	if (!pStruc)
#ifdef _DEBUG
		if (eClass == E_KIND_STRUC || eClass == E_KIND_ENUM)
			pStruc = new Struc_t(Complex_t::nextNegativeId());
		else if (eClass == E_KIND_CLASS)
			pStruc = new TypeClass_t(Complex_t::nextNegativeId());
		/*else if (eClass == E_KIND_UNION)
			pStruc = new TypeUnion_t(Complex_t::nextNegativeId());*/
		else
			ASSERT0;
#else
		if (eClass == E_KIND_STRUC || eClass == E_KIND_ENUM)
			pStruc = new Struc_t;
		else if (eClass == E_KIND_CLASS)
			pStruc = new TypeClass_t;
		/*else if (eClass == E_KIND_UNION)
			pStruc = new TypeUnion_t;*/
		else
			ASSERT0;
#endif

	TypePtr iStruc(TT.memMgr().NewTypeRef(pStruc));//, OBJ_ID_ZERO));//prevent objno to increment
	ON_OBJ_CREATED(iStruc);
	//iStruc->m_nFlags |= TYPEOBJ_ID_OVERIDE;
	if (eClass == E_KIND_ENUM)
		iStruc->flags() |= TYP_ENUM;

#ifdef _DEBUG
	__catch_struc_id(iStruc);
#endif

	TypePtr iType(TT.addTypeNew0(iStruc));
	assert(iType);

	if (!sName.empty())
	{
		NamesMgr_t *pNS(nullptr);
		TT.addNamedObj(pOwnerScope, iType, sName, &pNS, iForceName);

#if(0)
		static int z = 0;
CHECK(z == 75)
STOP
		MyString s;
		ChopName(sName, s, '|');
		fprintf(stdout, "[%d]: %s\n", z++, s.c_str());
#endif

		ApplyPrettyName(iType, sName, pOwnerScope);

	}

	//CHECK(pStruc->CompName("STRUC_dword_6A5E68"))
	//STOP

	return iStruc;
}

bool FileInfo_t::AcquireProxyType(TypePtr pTypePxy) const
{
	assert(pTypePxy->typeProxy());
	TypePtr pIncumbent(pTypePxy->typeProxy()->incumbent());
	if (pIncumbent->typeTypedef() || IsEmptyStruc(pIncumbent))
		AddTypeObj(pTypePxy, FileDef_t::AT_HEAD);
	else
		AddTypeObj(pTypePxy);
	return true;
}

static char escape_chars[] = "\a\b\f\n\r\t\v";//\'\"\?\\";

int FileInfo_t::CheckString(CFieldPtr pSelf, bool bUser) const
{
	OFF_t oData;
	if (!GetRawOffset(pSelf, oData))
		return 0;

	TypePtr iTypeEl = pSelf->type();
	if (!iTypeEl)
		return 0;

	if (!iTypeEl->typeArray())
		return 0;

	iTypeEl = iTypeEl->typeArray()->baseType();
	if (iTypeEl->typeArray())
		return 0;

	if (iTypeEl->typeComplex())
		return 0;

	if (iTypeEl->size() != OPSZ_BYTE)
		return 0;

	const I_DataSource &aRaw(GetDataSource()->pvt());

	int i(pSelf->size());
	//char *p = (char *)pIData;
	if (bUser)//dont check printability of chars
	{//only last character must be zero!
		char c;
		if (!aRaw.dataAt(oData + i - 1, OPSZ_BYTE, (PDATA)&c) || (c != 0))
			return 0;
	}
	else
	{
		while (i--)
		{
			char c;
			if (!aRaw.dataAt(oData, OPSZ_BYTE, (PDATA)&c))
				return 0;
			if (!isprint(c))
			{
				if (strchr(escape_chars, c) == 0)
				if (c != 0 || i)
					return 0;
			}
			oData++;
		}
	}

	return 1;
}

bool FileInfo_t::Overlap(FieldPtr pSelf, FieldPtr  pData) const
{
	SSID_t ssid1 = FuncInfo_s::SSIDx(pSelf);
	SSID_t ssid2 = FuncInfo_s::SSIDx(pData);
	if (ssid1 != ssid2)
		return false;

	if (ssid1 == SSID_LOCAL)
	{
		if (pSelf->ownerProc() != pData->ownerProc())
			return false;
		int offs1 = FuncInfo_s::address(pSelf);
		int offs2 = FuncInfo_s::address(pData);
		int d = offs2 - offs1;
		if (d > 0)
		{
			if (pSelf->size() <= d)
				return false;
		}
		else if (d < 0)
		{
			if (pData->size() <= -d)
				return false;
		}
		return true;
	}
	
	if (ssid1 == SSID_GLOBAL)
	{
		if (OwnerSeg(pSelf->owner()) != OwnerSeg(pData->owner()))
			return false;
	}

	ADDR offs1 = pSelf->_key();
	ADDR offs2 = pData->_key();
	if (offs2 > offs1)
	{
		assert(pSelf->size() >= 0);
		if ((ADDR)pSelf->size() <= offs2 - offs1)
			return false;
	}
	else if (offs2 < offs1)
	{
		assert(pSelf->size() >= 0);
		if ((ADDR)pData->size() <= offs1 - offs2)
			return false;
	}

	return true;
}

void FileInfo_t::setSyncOp(ProbeExIn_t &ctx) const
{
	if (!ctx.locus().empty() && ctx.folder())
	{
		const Frame_t &f(ctx.locus().back());
		//TypePtr pFunc(f.cont());
		//if (pFunc->type Proc())
		{
			//TypePtr ifDef(GlobFuncObj(pFunc->parentField()));
			GlobPtr ifDef(ctx.scopeFunc());
			if (ifDef && !ProtoInfo_t::IsStub(ifDef))
			{
				FileDef_t *pFileDef(ctx.fileDef());
				if (pFileDef)
				{
					FuncInfo_t FI(mrDC, *ifDef);
					HOP pOp(ctx.opLine());
					if (!pOp)
					{
						pOp = FI.FindOpByVA(f.addr());
						if (pOp)//adjustment for tmp ops
						{
							for (;;)
							{
								OpPtr pOpNx(FI.NextPrime(pOp));
								if (!pOpNx || FuncInfo_t::OpVA(pOpNx) != FuncInfo_t::OpVA(pOp))
									break;
								pOp = pOpNx;
							}
						}
					}
					ctx.pickOpLine(pOp);
					ctx.pickScope((GlobToTypePtr)ifDef);//pFunc);
					//context()->SetContext(ProbeEx_t(&mrDcRef, aLoc.folder(), pObj, pOp, f.pCont));//must have a func context
					if (pOp)//crash if clicked on 'endp'
						FI.VAListFromOp(ctx.curVAs(), pOp);
				}

				return;
			}
		}
	}
	//context()->SetContext(ProbeEx_t(&mrDcRef, aLoc.folder(), pObj, pOp, pFunc));
}

GlobPtr FileInfo_s::ContextFuncDef(CFieldPtr pSelf)
{
#if(NEW_LOCAL_VARS)
	return GetLocalOwner(pSelf);
#else
	CFieldPtr pField(pSelf);
	while (pField)
	{
		TypePtr pCplx(pField->OwnerComplex());
		if (pCplx->typeFuncDef())
			return pCplx;
		if (pCplx->typeUnion())
			return IsLocalsTop(pCplx);
		pField = pCplx->parentField();
	}
	return nullptr;
#endif
}

MemoryMgr_t *FileInfo_t::memMgrOf(CFieldPtr pField) const
{
	if (FuncInfo_s::IsLocalVar(pField))//not an arg
	{
		TypePtr pCplx(pField->OwnerComplex());
		assert(pCplx);
		//			if (!pCplx)
		//			return &memMgrGlob();//ret field or pseudo label?
		//			if (FuncInfo_t::Lab elPath(pField))
		//			return &memMgrGlob();//ret field or pseudo label?
		assert(!pCplx->typeFuncDef());
		return &memMgr();
	}
	return &memMgrGlob();
}

MemoryMgr_t *FileInfo_t::memMgrOf(CTypePtr pType) const
{
	return &memMgrGlob();
}

MemoryMgr_t *FileInfo_s::memMgrOfBody(CGlobPtr iGlob)
{
	FolderPtr pFolder(FolderOf(iGlob));
	return &FILEDEF(pFolder)->memMgr();
}

/*NamesMgr_t *FileInfo_t::namespaceOf(FieldPtr pField, MemoryMgr_t **ppMemMgr) const
{
NamesMgr_t *pNS(dc().namesMgr());
MemoryMgr_t *pMemMgr2(memMgrOf(pField));

if (pMemMgr2 != *ppMemMgr)
{
TypePtr ifDef(ContextFuncDef(pField));
pNS = ifDef->typeComplex()->assureN amespace();
pNS->NamespaceInitialized(ifDef);
*ppMemMgr = pMemMgr2;
}
else
{
if (IsStackLocal(pField) || IsLocalReg(pField))
{
TypePtr ifDef(pField->GetLocalOwner());
pNS = ifDef->typeComplex()->assureN amespace();
}
TypePtr pCplx;
pNS = OwnerNamesMgr(pField, &pCplx);
pNS->NamespaceInitialized(pCplx);
}
return pNS;
}*/

void FileInfo_t::DeleteRootInfo(HOP hSelf) const
{
	if (hSelf->ins().mpPrimeOp == hSelf)
	//if (IsPrimeOp(hSelf))
	{
#if(NEW_OP)
		m_pRI->args().Unlink(lin ked());
#endif
		memMgrEx().Delete(hSelf->m_pRI);
	}
	hSelf->setInsPtr(nullptr);
#ifdef _DEBUG
	hSelf->__isPrime = false;
#endif
}

HPATH FileInfo_t::NewPath() const
{
	HPATH h(memMgrEx().NewPath());
//CHECK(h.memRef() == 0x50)
//STOP
	return h;
}

HOP FileInfo_t::NewOp() const
{
#if(!NO_OBJ_ID)
	return memMgrEx().NewIdOp();
#else
	return memMgrEx().NewOp();
#endif
}

HOP FileInfo_t::NewTmpOp() const
{
	return memMgrEx().NewOp();
}

HOP FileInfo_t::NewPrimeOp() const
{
	HOP pOp(NewOp());
	pOp->setInsPtr(NewRootInfo());
	pOp->ins().mpPrimeOp = pOp;
#if(NEW_OP)
	pOp->m_pRI->args().empty();
	pOp->m_pRI->args().push_back(pOp);
#endif
	//pOp->setInsPtr(pOp);
CHECKID(pOp, 0x1aaf)
STOP

#ifdef _DEBUG
	pOp->__isPrime = true;
#endif
	return pOp;
}

HOP FileInfo_t::NewPrimeTmpOp() const
{
	HOP pOp(NewTmpOp());
	pOp->setInsPtr(NewRootInfo());
	pOp->ins().mpPrimeOp = pOp;
#if(NEW_OP)
	pOp->m_pRI->args().empty();
	pOp->m_pRI->args().push_back(pOp);
#endif
CHECKID(pOp, 0x1aaf)
STOP
#ifdef _DEBUG
	pOp->__isPrime = true;
#endif
	return pOp;
}


Ins_t* FileInfo_t::NewRootInfo() const
{
	return memMgrEx().NewRootInfo();
}

XOpLink_t *FileInfo_t::NewXOpLink(HOP pOp) const
{
	XOpLink_t *p(memMgrEx().NewXOpLink());
	p->setData(pOp);
	return p;
}

XOpLink_t *FileInfo_t::NewXOpLink2(HOP pOp) const
{
	XOpLink_t *p(memMgrEx().NewXOpLink2());
	p->setData(pOp);
	return p;
}

void FileInfo_t::Delete(HPATH pPath) const
{
	return memMgrEx().Delete(pPath);
}

void FileInfo_t::Delete(HOP pOp) const
{
	return memMgrEx().Delete(pOp);
}

bool FileInfo_t::DumpToStream(std::ostream &os, adcui::UDispFlags dispflags) const
{
	bool bHeader(dispflags.testL(adcui::DUMP_HEADER));
	ExprCache_t aCache(mrDC.primeSeg()->typeSeg()->ptrSize());
	DisplayStd_t disp(bHeader, false, os, aCache);
	disp.setFlags0(dispflags);

	FileDumper_t FD(*this, disp, nullptr, nullptr, false);

//?	disp.Clear();
//?	bool bCont(disp.openScope(nullptr));//root
//?	assert(bCont);
	FD.DumpFile();
//?	disp.closeScope(false);

//?	FD.write(os);
	return true;
}


//checks if field's type is an instantiation of some compound type, declared in another file (returned)
FolderPtr FileInfo_t::CheckTypeInstantiation(const Field_t &rField) const
{
	TypePtr pType2(rField.type());
	if (pType2 && pType2->typeStruc())
	{
		FolderPtr pFolder2(USERFOLDER(pType2));
		if (pFolder2 && pFolder2 != OwnerFolder())//LATER
		{
			if (ModuleOf(pFolder2) == mrDC.module())
				return pFolder2;
STOP//proxy?
		}
	}
	return nullptr;
}

bool FileInfo_t::DeleteFieldGlobal(Locus_t &aLoc, TypePtr pContext) const
{
	Frame_t &aTop(aLoc.back());
	FieldPtr pField(aTop.field());

	if (!IsGlobal(pField))//? || FuncInfo_s::isLocalArg(pField))
		return false;

	//if (FuncInfo_s::IsAnyLocalVar(pField))
		//return false;//?

	if (!IsMemMgrGlob())
	{
		FileInfo_t FI(*this, memMgrGlob());
		return FI.DeleteFieldGlobal(aLoc, pContext);
	}

	GlobPtr pGlob(GlobObj(pField));

	TypePtr pScope(OwnerScope(pGlob));
	if (pScope)
	{
		ClassInfo_t classInfo(*this, pScope);
		if (!classInfo.RemoveClassMember(pGlob))
			return false;
		if (pContext == pScope)//just undo a class member
		{
			return true;
		}
	}

	//remove from src only
	if (!RecallGlob(pGlob))
		return false;

	//mrProject.markDirty(DIRTY_GLOBALS);
	aTop.setField(nullptr);
	mrProject.markDirty(DIRTY_LOCUS_ADJUSTED);
	return true;
}

void FileInfo_t::CheckInclusion() const
{
	for (FilesMgr0_t::FolderIterator i(mrProject.files()); i; i++)
	{
		CFolderRef pFolder(*i);
		if (!pFolder.fileDef())
			pFolder.fileDef()->CheckInclusion();
	}
}


FuncDef_t* FileInfo_t::typeFuncDefIntrinsic(TypePtr iSelf) const
{
	FuncDef_t *pfDef(iSelf->typeFuncDef());
	if (pfDef)
		if (iSelf->flags() & FDEF_INTRINSIC)
			return pfDef;
	return nullptr;
}

GlobPtr FileInfo_s::IsLocalsTop(TypePtr iUnion)
{
#if(NEW_LOCAL_VARS)
	assert(0);
	return nullptr;
#else
	if (!iUnion->typeUnionLoc())
		return nullptr;
	assert(!iUnion->parentField());
	//if (iUnion->parentField()) return nullptr;
	//regular shared union?
	assert(!iUnion->ownerTypeMgr());
	//if (iUnion->ownerTypeMgr()) return nullptr;

	TypeUnion_t &rSelf(*iUnion->typeUnion());
	if (!rSelf.hasFields())
		return nullptr;
	FieldMapCIt it(rSelf.fields().begin());
	TypePtr ifDef(it->type());
	if (!ifDef->typeFuncDef())
		return nullptr;
	return ifDef;//first union's field is always a funcdef (not a pointer to funcdef)
#endif
}

bool FileInfo_t::DumpToFile(SrcDumpFile &ofs, adcui::UDispFlags dispflags) const
{
	CFolderPtr pFolder(OwnerFolder());
	//check if the folder is dump-eligible (under dispflags)
	bool bHeader(dispflags.testL(adcui::DUMP_HEADER));
	bool bDumpableAsHeader(IsDeclarationFile(pFolder));//does the file has a definition contents?
	bool bDumpableAsSource(IsDefinitionFile(pFolder));
	if (dispflags.testL(adcui::DUMP_UNFOLD))
	{
		if (bHeader)//no headers under UNFOLD option
			return false;
		if (!mrFileDef.hasNonStubs())
			return false;
	}
	else
	{
		if (bHeader)
		{
			if (!bDumpableAsHeader)
				return false;
		}
		else
		{
			if (!bDumpableAsSource)
				return false;
		}
	}

	if (!ofs.isOpen() && fileExists(ofs.path()))
		PrintWarning() << "Overwriting existing file: " << ofs.path() << std::endl;

	if (!ofs.open())
	{
		PrintError() << "Can't open file " << ofs.path() << std::endl;
		return false;
	}

	return DumpToStream(ofs.os(), dispflags);
}

/*void FileInfo_t::createStubsMgr()
{
assert(!mrDC.mpStubMgr);
mrDC.mpStubMgr = new StubMgr_t(mrDC);
}*/

TypePtr FileInfo_t::MakeNonStaticMember(GlobPtr g, FieldPtr pArg) const
{
	//TypePtr ifDef(mrFuncDef.type obj());
//CHECKID(g, 0x1c8c)
//STOP

	assert(pArg);
	assert(g->typeFuncDef()->IsMineArg(pArg));
	TypePtr iClass(pArg->isConstPtrToStruc());

	if (iClass)
	{
		if (ProtoInfo_t::ThisPtrArg(g) == pArg)
		{
			assert(g->typeFuncDef());
			assert(ProtoInfo_s::IsThisCallType(g));
			return iClass;
		}
	}

	//assert(pfDef);
	if (!iClass)
	{
		//FileInfo_t GI(*this, *g->folder()->fileDef());
		iClass = MakeStruct(MyString(), nullptr, 0);//add to file before last
		AddTypeObj(iClass, FileDef_t::AT_BEFORE_LAST);
	}

	SetConstPtrToStruc(pArg, iClass);
	MakeMemberThisCallFromArg(pArg);
//CHECKID(pArg, 7623)
//STOP

	return iClass;
}

#include "save_ex.h"

void FileInfo_t::RelocateFuncInnards(GlobPtr pGlob, FileDef_t& rFileTo)
{
	DcInfo_s::AssureMemMgr(rFileTo.ownerFolder());

	MoveAccessor_t<FuncSerializer_t> SAVE(pGlob, OwnerFolder(), rFileTo.memMgrEx());

	std::stringstream ss;

	{//cleanup a body here
		FuncInfo_t FI(*this, pGlob);
		SAVE.save(ss);
		SAVE.saveFunction(ss, pGlob);

		SAVE.blockTypes(true);//none of the types must go un-refed due to the cleanup
		FuncCleaner_t FD(FI);
		FD.DetachArgsRefs();//prevent args to be deleted
		FD.DestroyBody();
		FD.Cleanup();
		FD.CleanupFinal();
		SAVE.blockTypes(false);
	}

	FileInfo_t FI2(*this, rFileTo);//recovery must be performed in conext of dest file
	{
		FuncInfo_t FI(FI2, pGlob);

		SAVE.load(ss);
		TypePtr pLocals(SAVE.loadFunction(ss, *pGlob->typeFuncDef()));
		std::vector<TypePtr> v;
		GlobalRecovererEx_t aRec(mrProject);
		aRec.recoverFunc(v, pGlob, pLocals, FieldEx_t::dockField(pGlob)->_key());//no need for args recovery
	}
}


































