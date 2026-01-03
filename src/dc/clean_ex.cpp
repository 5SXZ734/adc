#include "clean_ex.h"
#include "db/clean.cpp"
#include "stubs.h"
#include "op.h"
#include "type_funcdef.h"
#include "db/type_proxy.h"
#include "main_ex.h"
#include "ana_main.h"
#include "cc.h"
#include "info_class.h"

//explicit template instantiations
template class DcCleaner_t<DcInfo_t>;
template class BinaryCleaner_t<DcInfo_t>;

#define	TEMPL	template <typename T>

////////////////////////////////

TEMPL void DcCleaner_t<T>::destroyIntrinsics()
{
	Dc_t &rSelf(dc());
	ModuleInfo_t PI2(*this, rSelf.memMgr());//?
	NamesMgr_t& rNS(*rSelf.primeSeg()->typeComplex()->namesMgr());

//	rSelf.mTypeCache.clear();

/*	while (!rSelf.mTypedefs.empty())
	{
		TypePtr p(rSelf.mTypedefs.front());
		PI2.ClearTypeName(*pNS, p);
		ReleaseTypeRef(p);
		rSelf.mTypedefs.pop_front();
		//BinaryCleaner_t::destroyTypeRef(*p, true);
		//mrDI.memMgr().Delete(p);
	}*/

	for (std::vector<GlobPtr>::iterator i(rSelf.mIntrinsics.begin()); i != rSelf.mIntrinsics.end(); i++)
	{
		GlobPtr p(*i);
		if (p)
		{
			PI2.ClearTypeName(rNS, (TypePtr)p);
			//if (ReleaseTypeRef0(p, !mbClosing))// && !mbClosing)
			//assert(p->refsNum() == 0);
			DestroyGlobObj(p);
		}
	}
	rSelf.mIntrinsics.clear();
}

TEMPL void DcCleaner_t<T>::disconnectLocal(FieldPtr pSelf, MemoryMgrEx_t &rMemMgr)
{
	while (!FuncInfo_t::LocalRefs(pSelf).empty())
	{
		XOpLink_t *pXRef(FuncInfo_t::TakeLocalRefFront(pSelf));
		OpPtr pOp(pXRef->data());
		assert(pOp);
		pOp->setLocalRef0(nullptr);//local
		rMemMgr.Delete(pXRef);
	}
}

/*void DcCleaner_t<T>::disconnectLabel(HPATH pSelf, MemoryMgrEx_t& rMemMgr)
{
	//FieldPtr pSelf
	while (!FuncInfo_t::PathOpRefs(pSelf).empty())
	{
		XOpLink_t* pXRef(pSelf->m.takeRefFront());
		OpPtr pOp(pXRef->data());
		assert(pOp);
		FuncInfo_t::SetLocalRef(pOp, nullptr);//label
		pOp->setLocalRef0(nullptr);//local
		rMemMgr.Delete(pXRef);
	}
}*/

TEMPL void DcCleaner_t<T>::destoyTypeClass(TypePtr iSelf)
{
	ClassInfo_t classInfo(*this, iSelf);
	classInfo.ClearMemberList();
	static NamesMgr_t rNS0;
	BASE::destroyTypeStruc(iSelf, rNS0);
}

#if(0)
TEMPL void DcCleaner_t<T>::destoyTypeVFTable(TypePtr iSelf)
{
	BASE::destroyTypeStruc(iSelf);
/*	TypeVFTable_t &rSelf(*iSelf->typeVFTable());
	TypePtr iBaseClass(rSelf.baseType());
	if (iBaseClass)
	{
		if (BASE::ReleaseTypeRef0(iBaseClass, !isClosing()))
			BASE::pushUnrefedType(iBaseClass);
		rSelf.setBaseType(nullptr);
	}*/
	/*TypePtr iOwnerClass(rSelf.ownerClass());
	if (iOwnerClass)
	{
		if (ReleaseTypeRef0(iOwnerClass, !isClosing()))
			addUnrefedType(iOwnerClass);
		rSelf.setOwnerClass(nullptr);
	}*/
}
#endif

TEMPL void DcCleaner_t<T>::destroyProc(TypePtr iSelf, NamesMgr_t& rNS0)
{
	BASE::destroyTypeStruc(iSelf, rNS0);
}

void ProjectEx_t::OnDestroyProject(bool bIsClosing)
{
	mExportPool.clear();

#if(0)
	if (mrMain.mrSturtupInfo.nProtoMode > 1)
	{
		const FoldersMap &m(rootFolder().children());
		for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
		{
			if (ProjectInfo_t::IsPhantomFolder(*i))
				continue;
			CTypePtr iModule(ModuleInfo_t::ModuleOf(i));
			const Dc_t *pDC(DCREF(iModule));
			if (pDC)
				pDC->saveStubs();//before c-funcs gone
		}
	}
#endif
	Project_t::OnDestroyProject(bIsClosing);
}

void ProjectEx_t::OnDestroyModule(TypePtr iModule, bool bClosing)
{
	Module_t &aBin(*iModule->typeModule());

	Dc_t *pDC(DCREF(iModule));
	if (!pDC)
	{
		//ProjectInfo_t PI(*this);
		BinaryCleaner_t<> BC(*this);
		BC.setClosing(bClosing);
		BC.destroyModule(iModule);
		return;
	}

	//DcInfo_t DI(*pDC);
	BinaryCleaner_t<DcInfo_t> BC(*pDC);
	BC.setClosing(bClosing);
	DcCleaner_t<> DC(BC);
	DC.setClosing(bClosing);
	DC.destroyModule(iModule);

	delete pDC;
}

TEMPL void DcCleaner_t<T>::destroyFolders()
{
	//some (special) folders must be undone beforehand
	FolderPtr pTemplFolder(BASE::TemplatesFolder());//the scope info is used during destroy, which is implemented by fileDefs. So the pretty names must be destroyed before any fileDefs.
	if (pTemplFolder)
		destroyFileRef(*pTemplFolder, false);

	for (FilesMgr0_t::FolderIterator i(dc().moduleFolder()); i; )//subtree only
	{
		CFolderRef rFolder(*i);
		++i;
		destroyFileRef((FolderRef)rFolder, false);
	}
}

TEMPL void DcCleaner_t<T>::destroyModule(TypePtr iSelf)
{
	//setClosing(true);
CHECKID(iSelf, 0x32b9)
STOP

	destroyFolders();

	destroyIntrinsics();

	ProjectEx_t& proj(BASE::projx());
//?	proj.setContext(nullptr);
	iSelf->SetUserData<Dc_t>(nullptr);

//	destroyExportedTypesInfo();
//	mrDC.cleanExportMap();

	if (proj.unresolvedExternalsModule() == iSelf)
		proj.setUnresolvedExternalsModule(nullptr);

	BASE::destroyModule(iSelf);

	//aBin.mpDC = nullptr;
}

TEMPL void DcCleaner_t<T>::destroyFuncDef(GlobPtr iSelf)
{
CHECKID(iSelf, 0)
STOP
	FuncDef_t &rSelf(*iSelf->typeFuncDef());
	assert(!rSelf.typeMgr());
	//FuncInfo_t FI(mrDC, *iSelf);
	static NamesMgr_t rNS0;

	BASE::UnmakeMemberMethod(iSelf);
//?	clearFields(rSelf.mRetFields, nullptr);
	//clearFields(rSelf.fields(), rSelf.namesMgr());//do it here before killing n-map

	FieldPtr pFunField(DcInfo_s::DockField(iSelf));
	if (pFunField)
	{
		if (!rSelf.nameless())
		{
			NamesMgr_t *pNS0(PrimeSeg()->typeComplex()->namesMgr());
			BASE::ClearTypeName(*pNS0, (GlobToTypePtr)iSelf);
		}
	}
	BASE::destroyTypeStruc((GlobToTypePtr)iSelf, rNS0);
	//rSelf.typ eobj()->rele aseRef();
}

TEMPL void DcCleaner_t<T>::destroyThunk(GlobPtr)
{
}

TEMPL void DcCleaner_t<T>::destroyTypeProxy(TypePtr iSelf)
{
CHECKID(iSelf, -167)
STOP
	//if (iSelf->refsNum() == 0)//incumbent must always reside in another module(?)
	{
		TypeProxy_t &rSelf(*iSelf->typeProxy());

		if (!iSelf->nameless())
		{
			//NamesMgr_t *pNS2(BASE::OwnerN amesMgr(iSelf->owner(), nullptr));
			NamesMgr_t *pNS2(iSelf->owner()->typeComplex()->namesMgr());
			assert(pNS2);
			BASE::ClearTypeName(*pNS2, iSelf);
		}

		BASE::ReleaseTypeRef(rSelf.incumbent());
		rSelf.setIncumbent(nullptr);//no more refs remains, so things like 'aka' not going to be called on this type
	}
}

TEMPL void DcCleaner_t<T>::DestroyGlobObj(GlobPtr p)
{
	//assert(!p->parentField());
	destroyGlob(p, true);
	assert(!p->owner());
	this->memMgrGlob2().Delete(FieldEx_t::dockField(p));
}

TEMPL void DcCleaner_t<T>::destroyGlob(GlobPtr pSelf, bool bClearPvt)
{
	Type_t *pType(pSelf->type());
	assert(pType);

	switch (pType->ObjType())
	{
	case OBJID_TYPE_FUNCDEF:
	{
		destroyFuncDef(pSelf);
		break;
	}
	case OBJID_TYPE_THUNK:
		destroyThunk(pSelf);
		break;
	default:
		assert(0);
	}
	if (bClearPvt)
		pSelf->ClearPvt();
}

TEMPL void DcCleaner_t<T>::destroyTypeRef(TypeObj_t &rSelf, bool bClearPvt, NamesMgr_t& rNS0)
{
	Type_t *pType(rSelf.type());
	assert(pType);

	switch (pType->ObjType())
	{
	/*case OBJID_TYPE_RTTI_TD:
	case OBJID_TYPE_RTTI_BCD:
	case OBJID_TYPE_RTTI_CHD:
	case OBJID_TYPE_RTTI_BCA:
		BASE::destroyTypeStruc(&rSelf);
		break;*/
	case OBJID_TYPE_CLASS:
	case OBJID_TYPE_NAMESPACE:
		destoyTypeClass(&rSelf);
		break;
	case OBJID_TYPE_FUNCDEF:
		assert(0);//destroyFuncDef((GlobPtr)&rSelf);
		break;
	case OBJID_TYPE_THUNK:
		assert(0);//destroyThunk((GlobPtr)&rSelf);
		break;
/*	case OBJID_TYPE_DC:
		assert(&mrDI.DcRef() == &rSelf);
		destroy();//*pType->typedC());
		break;*/
	case OBJID_TYPE_PROXY:
		destroyTypeProxy(&rSelf);
		break;
	/*case OBJID_TYPE_VFTABLE:
	case OBJID_TYPE_VBTABLE:
	case OBJID_TYPE_LVFTABLE:
	case OBJID_TYPE_CVFTABLE:
		destoyTypeVFTable(&rSelf);
		break;*/
	default:
		BASE::destroyTypeRef(rSelf, bClearPvt, rNS0);
		return;
	}
	if (bClearPvt)
		rSelf.ClearPvt();
}

TEMPL void DcCleaner_t<T>::DestroyPrettyName(GlobPtr pGlob, PNameRef pn)
{
//CHECKID(pField, 0x9d90)
//STOP
	TypePtr pScope(pGlob->ownerScope1());
	if (!pScope)
		pScope = PrimeSeg();
	assert(pScope);
//CHECKID(pScope, 0x10fe)
//STOP
	NamesMgr_t *pNS(pScope->typeComplex()->namesMgr());
	//pNS->check();
	pNS->removen(pn);
	//pNS->check();
	memMgr().Delete(pn);//pn must be removed from a namespace!
//?	if (!pField->name())
		pGlob->clearPrettyName();
}

TEMPL void DcCleaner_t<T>::DestroyPrettyName(TypePtr p, PNameRef pn)
{
CHECKID(p, 0x9d90)
STOP
	assert(p->hasUglyName());
	TypePtr iScope;
	if (!p->typeFuncDef())
	{
		iScope = p->owner();
		if (!iScope)
			iScope = PrimeSeg();
	}
	else
	{
		FieldPtr pField(DcInfo_t::DockField((CGlobPtr)p));
		assert(pField);
		iScope = DcInfo_t::OwnerScope(pField);
		if (!iScope)
			iScope = PrimeSeg();
	}
	assert(iScope);
//CHECKID(iScope, 0x10fe)
//STOP
	NamesMgr_t *pNS(iScope->typeComplex()->namesMgr());
	pNS->removen(pn);
	memMgr().Delete(pn);//pn must be removed from a namespace!
	p->flags() &= ~TYP_PRETTY_NAME;
}

TEMPL void DcCleaner_t<T>::destroyFileRef(Folder_t &rSelf, bool bClearPvt)
{
	File_t *pFile(rSelf.file());
	assert(pFile);

	switch (pFile->fileId())
	{
	case FILEID_FILE:	//FileDef_t
	{
		FileInfo_t FL(dc(), *pFile->fileDef());
		FL.ClearFile(isClosing());
		break;
	}
	case FILEID_STUBS:
	{
		assert(dc().moduleRef().special(FTYP_PROTOTYPES) == &rSelf);
		FolderPtr pFolderStubs(&rSelf);
		//		mpStubMgr->save();//before c-funcs gone
		pFolderStubs->fileStubs()->stubs().clear();
		break;
	}
	case FILEID_TEMPL:
	{
		assert(dc().moduleRef().special(FTYP_TEMPLATES) == &rSelf);
		FolderPtr pFolderTemp(&rSelf);
		//if (pFolderTemp)
		{
			PrettyObjMap &m(pFolderTemp->fileTempl()->map());
			if (!m.empty())
			{
				for (PrettyObjMap::const_iterator i(m.begin()); i != m.end(); ++i)
				{
//CHECKID(i->first, 0x1e781)
//STOP
					ObjPtr pObj(i->first);
					GlobPtr pGlob(pObj->objGlob());
					if (pGlob)
					{
						assert(pGlob->hasPrettyName());
						DestroyPrettyName(pGlob, i->second);
					}
					else
					{
						TypePtr pType(pObj->objType());
						assert(pType && pType->hasPrettyName());
						DestroyPrettyName(pType, i->second);
					}
				}
				m.clear();
			}
		}
		break;
	}
	default:
		BASE::destroyFileRef(rSelf, bClearPvt);
		return;
	}

	if (bClearPvt)
		rSelf.ClearPvt();
}

TEMPL void DcCleaner_t<T>::destroy(FieldRef rSelf)
{
	assert(rSelf.nameless());//name must be cleared at this point by some parent

	assert(!rSelf.hasUserData());
	//assert(!FuncInfo_t::LabelP ath(&rSelf));//must get rid of path in the first place

//?	disconnect(&rSelf, reinterpret_cast<MemoryMgrEx_t &>(mrDI.memMgr()));

	BASE::destroy(rSelf);

	/*PathPtr pPath(FuncInfo_t::Lab elPath(&rSelf));
	if (pPath)
	{
		assert(0);
		/ *?SetPat hLabel(pPath, 0);
		if (PathType(pPath) == BLK_DATA)
		{
		//?ClearPath(pPath);
		mrMemMgr.Delete(pPath);
		}* /
	}*/
}

/*TEMPL void DcCleaner_t<T>::destroyConflictField(FieldPtr pField, NamesMgr_t *pNS)
{
CHECKID(pField, 0x509)
STOP
	BASE::destroyField(pField, pNS);
}*/

/*void DcCleaner_t<T>::destroyExportedTypesInfo()
{
	//assert(iType->isEx porting() && !iType->nameless());
	ExpTypesMap &m(mrDI.projx().exp TypesMap());
	while (!m.empty())
	{
		TypePtr p(m.begin()->second);
		assert(p->isExp orting());
		m.erase(m.begin());
#if(!DEBUG_NS)
		ReleaseTypeRef0(p, false);
		//if (p->releaseRef() <= 0)
			//ASSERT0;
#endif
		p->setEx porting(false);
	}
}*/

FuncCleaner_t::FuncCleaner_t(const FuncInfo_t &r)
	: DcCleaner_t(r)//,
	//funcInfo()(r)
{
}

FuncCleaner_t::~FuncCleaner_t()
{
	Cleanup();
}

void FuncCleaner_t::ClearPathRefs(HPATH pPath) const
{
	while (!PathOpRefs(pPath).empty())
	{
		XOpLink_t *pXRef(pPath->takeRefFront());
		HOP pOp(pXRef->data());
		pOp->setPathRef(HPATH());
		memMgrEx().Delete(pXRef);
	}
}

void FuncCleaner_t::Cleanup()
{
	while (!mlOrphanEntryOps.empty())
	{
		HOP pOp(mlOrphanEntryOps.front());
		mlOrphanEntryOps.pop_front();
		assert(mrFuncDef.entryOps().contains(INSPTR(pOp)));
		HPATH pPath(PathHead());
		pPath->takeOp(pOp);
		DeleteRootInfo(pOp);
		Delete(pOp);			//kill re
	}

	while (!mlOrphanCallOuts.empty())
	{
		HOP pOp(mlOrphanCallOuts.front());
		mlOrphanCallOuts.pop_front();
		assert(mrFuncDef.mCallRets.contains(pOp));
		assert(!pOp->XOut());
		mrFuncDef.mCallRets.Unlink(pOp);
		DeleteRootInfo(pOp);//reset only
		Delete(pOp);
	}
	cleanupOrphanFields();
	cleanupOrphanArgs();
	cleanupOrphanVars();

	destroyUnrefedArgTypes();//from global context
}

void FuncCleaner_t::CleanupFinal()
{
	DcInfo_t DI(mrDC, memMgrGlob());
	DcCleaner_t<> DC(DI);
	//all unreffed types are owned by global memmgr - must be disposed with dc cleaner
	while (hasUnrefedTypes())
	{
		//assert(0);
		DC.DestroyTypeRef(popUnrefedType());
	}
	DC.destroyUnrefedTypes();
}

void FuncCleaner_t::destroyUnrefedArgTypes()
{
	if (hasUnrefedArgTypes())
	{
		DcInfo_t PI(*this, memMgrGlob());//global!
		DcCleaner_t<> DC(PI);
		do {
			DC.DestroyTypeRef(popUnrefedArgType());
		} while (hasUnrefedArgTypes());
		DC.destroyUnrefedTypes();//not in destructor of a base class!!
	}
}

void FuncCleaner_t::cleanupOrphanArgs()
{
	//assert(&memMgr() == &memMgrGlob());
	while (!mlOrphanArgs.empty())
	{
		FieldPtr pField(mlOrphanArgs.front());
		mlOrphanArgs.pop_front();
		DeleteUnrefedLocalArg(pField);
	}
}

void FuncCleaner_t::cleanupOrphanVars()
{
	//assert(&memMgr() != &memMgrGlob());
	while (!mlOrphanVars.empty())
	{
		FieldPtr pField(mlOrphanVars.front());
		mlOrphanVars.pop_front();
		DeleteUnrefedLocalVar(pField);
	}
}

void FuncCleaner_t::cleanupOrphanFields()
{
	while (!mlOrphanVarOps.empty())
	{
		HOP pOp(mlOrphanVarOps.front());
		mlOrphanVarOps.pop_front();
		assert(funcInfo().IsUnrefed(pOp));
		/*if (pField->IsDa ta())//not label!
		{
			HPATH pPath(Lab elPath(pField));
			if (pPath && pPath->Type() == BLK_DATA)
			{
				ClearPath(pPath);
				if (!pPath->IsLinked())//index table?
				{
					mrMemMgr.Delete(pPath);
					pField->setLabelPath(0);
				}
			}*/
			RemoveUnrefedVarOp(pOp);
		//}
	}
}

void FuncCleaner_t::DisconnectCallOutOp(CHOP pSelf)
{
	assert(IsCallOutOp(pSelf));
	while (pSelf->XIn())//should be only 1
	{
		HOP pOp(pSelf->XIn()->data());
		UnmakeUDLink(pSelf, pOp);
	}
	while (pSelf->XOut())
	{
		XOpLink_t* pOut(pSelf->XOut());
		HOP pOp(pOut->data());
		UnmakeUDLink(pOp, pSelf);
	}
	mrFuncDef.mCallRets.Unlink(pSelf);
	pSelf->setInsPtr(nullptr);
	//ReleaseLocalRef(pLocal, pSelf);
}

void FuncCleaner_t::DisconnectOp(CHOP pSelf, bool bPreserveCodeFlow)
{
//CHECKID(pSelf, 20)
//CHECK(No(pSelf)==20)
//STOP
	if (IsPrimeOp(pSelf))
	{
		for (OpList_t::Iterator i(pSelf->argsIt()); i; i++)
		{
			HOP pOp(i.data());
			DisconnectOp(pOp, false);
		}
	}

	while (pSelf->XIn())
	{
		HOP pOp(pSelf->XIn()->data());
		UnmakeUDLink(pSelf, pOp);
		if (pOp->m_xout.empty())		//last out xdep killed?
		{
			if (IsCallOutOp(pOp))
			{
				DisconnectOp(pOp, false);
				mlOrphanCallOuts.push_back(pOp);
			}
			else if (IsArgOp(pOp))
			{
				DisconnectOp(pOp, false);
				mlOrphanEntryOps.push_back(pOp);
			}
			else
			{
				mlOrphanOps.push_back(pOp);//these are not supposed to be deleted!
			}
		}
	}

	while (pSelf->XOut())
	{
		XOpLink_t* pOut(pSelf->m_xout.head());
		HOP pOp = pOut->data();
		
		assert(pOp->isLinked());		//not linked somewhere?
		if (IsCallOutOp(pOp))
			mlOrphanCallOuts.push_back(pOp);

		UnmakeUDLink(pOp, pSelf);
	}

	
	FieldPtr pFieldRef(pSelf->localRef());
	if (!pFieldRef)
		return;


	if (bPreserveCodeFlow)
	{
		if (IsAddr(pSelf))
			if (IsGoto(pSelf) || IsCondJump(pSelf))
				return;
		if (IsLocalArg(pFieldRef) || mrFuncDef.IsMineRet(pFieldRef))
			return;
	}

	DetachLocalRef(pSelf);
	DetachLabelRef(pSelf);
}

void FuncCleaner_t::DetachLocalRef(HOP pSelf)
{
	FieldPtr pLocal(pSelf->localRef());
	if (!pLocal)
		return;

CHECKID(pLocal, 0x5747)
STOP

	bool bIsArg(ProtoInfo_t::IsLocalArgAny(pLocal));
	ReleaseLocalRef(pLocal, pSelf);
	if (!IsVarOp(pSelf) && !IsArgOp(pSelf))//what is it then?
	{
//?!			assert(IsPrimeOp(pSelf) || IsCallOutOp(pSelf));
	//	assert(!IsLocalArg(pLocal));
			
		if (IsStackLocal(pLocal) || IsLocalReg(pLocal))
		{
			HOP pVarOp(GetVarOp(pLocal));
			if (pVarOp && IsUnrefed(pVarOp))//could've been removed already?
				mlOrphanVarOps.push_back(pVarOp);
		}
	}

	if (LocalRefs(pLocal).empty())
	{
		if (!bIsArg)
		{
			if (!TakeLocal(pLocal))
				ASSERT0;
			mlOrphanVars.push_back(pLocal);
			CheckStrucLocsEmpty();
		}
		//mlOrphanArgs.push_back(pLocal);
	}
}

void FuncCleaner_t::DetachLabelRef(HOP pSelf)
{
	HPATH pPath(FuncInfo_t::PathRef(pSelf));
	if (pPath)
	{
//CHECKID(pLabel, 0x9a9)
//STOP
		funcInfo().ReleasePathRef(pPath, pSelf);
	}
}

void FuncCleaner_t::DestroyPrimeOp(HOP pSelf)
{
//CHECK(OpNo(pSelf) == 323)
//STOP
	assert(IsPrimeOp(pSelf));
	DestroyRhs(pSelf);
	PathOf(pSelf)->takeOp(pSelf);
	DestroyOp(pSelf);
}

void FuncCleaner_t::AdjournTransientOp(HOP hSelf, RedumpCache_t & redumpCache)
{
	FieldPtr pLocal(LocalRef(hSelf));
	if (pLocal)
	{
		//redumpCache.trans.insert(hSelf);

		assert(!hSelf->XIn() && !hSelf->hasArgs());
		if (IsArgOp(hSelf))
		{
			//destroy all outbound links
			while (hSelf->XOut())
			{
				XOpLink_t* pOut(hSelf->XOut());
				HOP pOp(pOut->data());
				assert(/*!IsCallArg(pOp) &&*/ !IsCallOutOp(pOp) && pOp != hSelf);
				UnmakeUDLink(pOp, hSelf);
				if (!redumpCache.perm.insert(std::make_pair(pOp, pLocal)).second)
					ASSERT0;//?
			}
		}
		else
		{
			assert(IsVarOp(hSelf));
		}

		assert(!hSelf->XOut());
		/*while (hSelf->XOut())
		{
			XOpLink_t* pOut(hSelf->m_xout.head());
			HOP pOp = pOut->data();

			assert(pOp->isLinked());		//not linked somewhere?
			assert(!IsCallOutOp(pOp));

			UnmakeUDLink(pOp, hSelf);
		}*/

/*?		XOpList_t& l(LocalRefs(pLocal));
		for (XOpList_t::Iterator i(l); i; )
		{
			HOP pOp(i.data());
			assert(!IsCallArg(pOp));//?
			assert(LocalRef(pOp) == pLocal);
			if (pOp == hSelf)
			{
				++i;
				continue;
			}
			XOpList_t::Iterator j(i++);

			ReleaseLocalRef(pLocal, pOp);
			//SetLocalRef(pOp, nullptr);

			if (IsCallOutOp(pOp))
			{
CHECKID(pOp, 18627)
STOP
				HOP hCallOp(PrimeOp(pOp));
				assert(IsCall(hCallOp));
				DisconnectCallOutOp(pOp);
				redumpCache.insertCallOut(pOp, hCallOp, pLocal);
				STOP
			}
			else
			{
				if (!redumpCache.insertPermOp(pOp, pLocal))
					ASSERT0;
			}
			//memMgrEx().Delete(l.take(j));
		}*/

		ReleaseLocalRef(pLocal, hSelf);
	}

	TakeOp(PathOf(hSelf), hSelf);
	DeleteRootInfo(hSelf);
	Delete(hSelf);
}

void FuncCleaner_t::DestroyOp(HOP pSelf)//, OpList_t &l)
{
	//?assert(pOp->isLinked());		//not linked somewhere?
CHECKID(pSelf, 0x16d8)
STOP
CHECKID(pSelf, 18627)
STOP
	assert(!IsPrimeOp(pSelf) || !pSelf->hasArgs());

	DetachLocalRef(pSelf);
	DetachLabelRef(pSelf);

	while (pSelf->XIn())
	{
		HOP pOp(pSelf->XIn()->data());
		//ReleaseXDepOut(pOp, pSelf);
		UnmakeUDLink(pSelf, pOp);
		if (pOp->m_xout.empty())		//last out xdep killed?
		{
			if (IsCallOutOp(pOp))
			{
				mrFuncDef.mCallRets.Unlink(pOp);
				DestroyOp(pOp);
			}
			else if (IsArgOp(pOp))
			{
				assert(!pOp->hasArgs());
				DestroyPrimeOp(pOp);
			}
		}
		//memMgrEx().Delete(pSelf->m_xin.take(pSelf->XIn()));//IsCallOutOp depends on this
	}

	while (pSelf->XOut())
	{
		HOP pOp(pSelf->XOut()->data());
		bool bCallout(IsCallOutOp(pOp));
		UnmakeUDLink(pOp, pSelf);
		//ReleaseXDepIn(pOp, pSelf);
//		if (!pOp->XIn())				//last out xdep killed?
			if (bCallout)
			{
				assert(!pOp->XIn());
				mrFuncDef.mCallRets.Unlink(pOp);
				DestroyOp(pOp);
			}
		//memMgrEx().Delete(pSelf->m_xout.take(pSelf->XOut()));//IsCallOutOp depends on this
	}

	//if (IsCallOutOp(pSelf))
		//mrFuncDef.mCallRets.Unlink(pSelf);

/*#if(NEW_OP)
	if (!IsPrimeOp(pSelf))
		pSelf->setInsPtr(nullptr);
#endif*/
	DeleteRootInfo(pSelf);
	Delete(pSelf);
}

void FuncCleaner_t::DestroyRhs(HOP pSelf)
{
	//?assert(pOp->isLinked());		//not linked somewhere?
//CHECKID(pSelf, 4030)
//STOP

	assert(IsPrimeOp(PRIME(pSelf)));

	OpList_t& l(pSelf->args());
	OpList_t::Iterator i(PRIME(pSelf)->argsIt());
	while (i)
	{
		HOP pOp(i.data());
		OpList_t::Iterator j(i++);
		l.erase(j);
		DestroyOp(pOp);
	}
}

bool FuncCleaner_t::RemoveUnrefedVarOp(HOP pOp)
{
	if (!pOp)
		return false;
	assert(pOp->m_xin.empty());
	assert(pOp->m_xout.empty());
	assert(!pOp->hasArgs());//DestroyRhs(INSPTR(pOp));
	FuncInfo_t::PathOf(pOp)->takeOp(pOp);
	DestroyOp(PRIME(pOp));
	//CHECKID(pData, 1321)
	//STOP
	return true;//field will be delete in destructor
}

void FuncCleaner_t::ClearArgType(FieldPtr pSelf)
{
	TypePtr iType(pSelf->mpType);
	if (iType)
	{
		if (BASE::ReleaseTypeRef0(iType, !isClosing()))
			pushUnrefedArgType(iType);
		pSelf->setType0(nullptr);
	}
}

bool FuncCleaner_t::DeleteUnrefedLocalArg(FieldPtr pLocal)
{
	//assert(IsMemMgrGlob());
	TypeBasePtr iNSOwner(FuncDefPtr());
	if (pLocal->name())
	{
		NamesMgr_t *pNS(iNSOwner->typeComplex()->namesMgr());
		assert(pNS);
		ModuleInfo_t PI2(*this, memMgrGlob());
		PI2.ClearFieldName(pLocal, pNS);
	}

	//funcInfo().SetType(pLocal, nullptr);
	ClearArgType(pLocal);

	memMgrGlob().Delete(pLocal);
	return true;
}

bool FuncCleaner_t::DeleteUnrefedLocalVar(FieldPtr pLocal)//, MemoryMgr_t *pMemMgr)
{
	assert(!pLocal->hasUserData());
	//assert(!IsMemMgrGlob());
	TypeBasePtr iNSOwner(FuncDefPtr());
	NamesMgr_t *pNS(iNSOwner->typeComplex()->namesMgr());
	if (pNS)
	{
		ModuleInfo_t PI2(funcInfo(), memMgr());//*pMemMgr);
		PI2.ClearFieldName(pLocal, pNS);
	}

	//funcInfo().SetType(pLocal, nullptr);
	ClearType(pLocal);

	memMgr().Delete(pLocal);
	return true;
}

HOP FuncCleaner_t::TakeOrphanOp()
{
	if (!mlOrphanOps.empty())
	{
		HOP pOp(mlOrphanOps.front());
		mlOrphanOps.pop_front();
		return pOp;
	}
	return HOP();
}


FieldPtr FuncCleaner_t::TakeOrphanArg()
{
	if (!mlOrphanArgs.empty())
	{
		FieldPtr pField(mlOrphanArgs.front());
		mlOrphanArgs.pop_front();
		return pField;
	}
	return nullptr;
}

FieldPtr FuncCleaner_t::TakeOrphanVar()
{
	if (!mlOrphanVars.empty())
	{
		FieldPtr pField(mlOrphanVars.front());
		mlOrphanVars.pop_front();
		return pField;
	}
	return nullptr;
}

FieldPtr FuncCleaner_t::TakeLocal(FieldPtr pField)
{
	assert(pField->OwnerComplex());

	//CHECK(Type() == BLK_EXIT)
	//STOP

	//HPATH pPath(pField->GetOwnerPath());
	//assert(pPath);

	TypePtr iStrucLoc(pField->OwnerComplex());
	//?assert(pPath->m.mpLocals2 == iStrucLoc);
	Struc_t *pStrucLoc(iStrucLoc->typeStruc());

	if (!pStrucLoc->takeField(pField))
		return nullptr;

#if(0)
	if (pStrucLoc->fields().empty())
	{
		if (!pStrucLoc->typeFuncDef())
		{
#if(NEW_LOCAL_VARS)
			TypePtr iFuncDef(iStrucLoc->owner());
			
			TypePtr iLocals(iFuncDef->typeFuncDef()->locals());
			if (iLocals)
			{
				//DestroyTypeRef(iLocals);
				destroyTypeRef(*iLocals, true);
				iLocals->setOwner(nullptr);
				memMgr().Delete(iLocals);
				//if (ReleaseTypeRef0(iType, !isClosing()))
					//addUnrefedType(iType);
				iFuncDef->typeFuncDef()->setLocals(nullptr);
			}

#else
			FieldPtr pField2(iStrucLoc->parentField());
			ClearType(pField2);
			//funcInfo().SetType(pField2, nullptr);
			TypePtr iUnionTop(funcInfo().GetLocalsTop());
			assert(iUnionTop);
			if (!iUnionTop->typeStruc()->takeField(pField2))//take field
			{
				assert(0);
			}
			funcInfo().memMgr().Delete(pField2);
#endif
		}
		//pPath->m.mpLocals2 = nullptr;
	}
#endif
	return pField;
}

void FuncCleaner_t::DetachArgsRefs()
{
	for (FuncArgsIt i(funcInfo().FuncDef()); i; ++i)
		disconnectLocal(VALUE(i), memMgrEx());
}

void FuncCleaner_t::ClearOpList(OpList_t &l)
{
	while (!l.empty())
	{
		OpPtr pOp(l.front());
		l.pop_front();
		DestroyOp(pOp);
	}
}

/*void FuncCleaner_t::ClearLocals(HPATH pSelf)
{
	
	if (!pSelf->m.mpLocals2)
		return;

	if (PathType(pSelf) == BLK_ENTER)
	{
		pSelf->m.mpLocals2->typeStrucLoc()->SetOwnerPath(pSelf);
	}
	else
	{
		NamesMgr_t *pNs(mrFuncDef.namesMgr());
		if (pNs)
		{
			//unname fields first
			//ProjectInfo_t an(mrMemMgr);
			StrucLoc_t *pStruc(pSelf->m.mpLocals2->typeStrucLoc());
			for (FieldMapIt i(pStruc->fields().begin()); i != pStruc->fields().end(); i++)
			{
				FieldPtr pField(VALUE(i));
				ClearObjName(*pNs, pField);
			}
		}
		//mrMemMgr.Delete(pSelf->m.mpLocals2);
		FieldPtr pField(pSelf->m.mpLocals2->parentField());
		TypePtr iUnion(GetLocalsTop());
		pField->setTy pe(nullptr);//remove all vars
		if (!iUnion->typeStruc()->takeField(pField))//take field
		{
			assert(0);
		}
		mrMemMgr.Delete(pField);
	}
	pSelf->m.mpLocals2 = nullptr;
}*/

void FuncCleaner_t::DestroyPath(HPATH p)
{
	ClearPath(p);//not required for non-terminal paths?
	if (p->Parent())
		p->Parent()->UnlinkChild(p);
	else
		mrFuncDef.pathTree().m.Unlink(p);
	memMgrEx().Delete(p);
}

void FuncCleaner_t::ClearPath(HPATH pSelf)
{
	//ClearLocals(pSelf);

	//pSelf->m.mOps.clear(mrMemMgr);

	while (pSelf->hasOps())
		DestroyPrimeOp(pSelf->headOp());

	ClearPathRefs(pSelf);
	
	//MT_release();
}

/*void FuncCleaner_t::UntypeFields(FieldMap &l)
{
	for (FieldMapIt i(l.begin()); i != l.end(); i++)
	{
		FieldPtr pField(VALUE(i));
		setT ype(pField, nullptr);
	}
}*/

void FuncCleaner_t::DestroyBody()
{
	//?UnmakeMemberMethod();

//	ClearOpList(mrFuncDef.mCallRets);

	//first, clear all label's xrefs
	for (PathTree_t::LeafIterator i(mrFuncDef.pathTree().m); i; i++)
	{
		HPATH pPath(i.data());
		ClearPath(pPath);
	}

	assert(mrFuncDef.mCallRets.empty());

	cleanupOrphanFields();
	cleanupOrphanArgs();//while NS is alive
	cleanupOrphanVars();
	//DeleteNamespace();

	while (!mrFuncDef.pathTree().m.IsEmpty())//level by level starting from bottom
	{
		for (PathTree_t::LeafIterator i(mrFuncDef.pathTree().m); i;)
		{
			HPATH p(i.data());
			++i;
			DestroyPath(p);
		}
	}

#if(NEW_LOCAL_VARS)
	DestroyStrucLoc(*mrFuncDef.namesMgr());
#else
	TypePtr iUnionTop(GetLocalsTop());
	if (iUnionTop)
	{
		destroyUnion(iUnionTop);
		memMgr().Delete(iUnionTop);

		/*FieldPtr pField(mrFuncDef.type obj()->parentField());
		setT ype(pField, nullptr);
		if (!iUnionTop->typeStruc()->takeField(pField))//take field
		{
			assert(0);
		}
		//TypePtr iThis(pField->takeType());
		assert(0);//? mrFuncDef.memMgr().Delete(iUnionTop);*/
	}
#endif
}

void FuncCleaner_t::DestroyStrucLoc(NamesMgr_t& rNS0)
{
	TypePtr iType(mrFuncDef.locals());
	if (iType)
	{
		destroyTypeStruc(iType, rNS0);
		iType->setOwner(nullptr);
		memMgr().Delete(iType);
		mrFuncDef.setLocals(nullptr);
	}
}

void FuncCleaner_t::AdjournCallOuts(HOP pSelf, RedumpCache_t &redumpCache)
{
	//?assert(pOp->isLinked());		//not linked somewhere?
//CHECKID(pSelf, 4030)
//STOP

	assert(IsCall(pSelf));

	XOpList_t::Iterator i(pSelf->m_xout);
	while (i)
	{
		HOP pOp(i.data());//user
		++i;
		assert(IsCallOutOp(pOp));
		//assert(!LocalRef(pOp));
		//DisconnectOp(pOp, false);
		AdjournCallOutOp(pOp, redumpCache);
		//mrFuncDef.mCallRets.Unlink(pOp);
		//pOp->setInsPtr(nullptr);//reset only
		//Delete(pOp);
	}
}

void FuncCleaner_t::AdjournPermanentOp(CHOP pSelf, RedumpCache_t &redumpCache)
{
//CHECKID(pSelf, 20)
CHECK(OpNo(pSelf)==604)
STOP
	if (IsPrimeOp(pSelf))
	{
		for (OpList_t::Iterator i(pSelf->argsIt()); i; i++)
		{
			HOP pOp(i.data());
			AdjournPermanentOp(pOp, redumpCache);
		}
	}

	while (pSelf->XIn())
	{
		HOP pOp(pSelf->XIn()->data());
		UnmakeUDLink(pSelf, pOp);
		if (pOp->m_xout.empty())		//last out xdep killed?
		{
			if (IsCallOutOp(pOp))
			{
				AdjournCallOutOp(pOp, redumpCache);
			}
			else if (IsArgOp(pOp))
			{
				AdjournTransientOp(pOp, redumpCache);
			}
		}
	}

	while (pSelf->XOut())
	{
		XOpLink_t* pOut(pSelf->m_xout.head());
		HOP pOp = pOut->data();
		
		assert(pOp->isLinked());		//not linked somewhere?
		if (IsCallOutOp(pOp))
			AdjournCallOutOp(pOp, redumpCache);

		UnmakeUDLink(pOp, pSelf);
	}

	
	FieldPtr pLocal(LocalRef(pSelf));
	if (!pLocal)
		return;

CHECKID(pLocal, 18297)
STOP

	/*HOP hVarOp(GetVarOp(pLocal));
	if (hVarOp)//?assert
	{
		AdjournTransientOp(hVarOp, redumpCache);

		if (IsAddr(pSelf))
			if (IsGoto(pSelf) || IsCondJump(pSelf))
				return;
		if (IsLocalArg(pLocal) || mrFuncDef.IsMineRet(pLocal))
			return;
	}*/

	redumpCache.insertPermOp(pSelf, pLocal);

	ReleaseLocalRef(pLocal, pSelf);
	//DetachLocalRef(pSelf);
	//DetachLabelRef(pSelf);
	assert(!PathRef(pSelf));
}

void FuncCleaner_t::AdjournCallOutOp(HOP pSelf, RedumpCache_t &redumpCache)
{
	assert(IsCallOutOp(pSelf));
	HOP hCallOp(PrimeOp(pSelf));

CHECKID(pSelf, 19812)
//CHECK(OpNo(hCallOp) == 59)
STOP
	//assert(!pSelf->XOut());
	FieldPtr pLocal(LocalRef(pSelf));
	if (pLocal)
	{
		/*HOP hVarOp(GetVarOp(LocalRef(pSelf)));
		if (hVarOp)
			AdjournTransientOp(hVarOp, redumpCache);*/

		DisconnectCallOutOp(pSelf);
		ReleaseLocalRef(pLocal, pSelf);
		redumpCache.insertCallOut(pSelf, hCallOp, pLocal);
	}
	else//just destroy
	{
		//mrFuncDef.mCallRets.Unlink(pSelf);
		DisconnectCallOutOp(pSelf);
		Delete(pSelf);
	}
}

void FuncCleaner_t::AdjournRhsOp(HOP pSelf, RedumpCache_t &redumpCache)
{
	//assert(IsRhsOp(pSelf));//should be unlinked
	assert(!pSelf->isLinked());

	assert(!LocalRef(pSelf));
	assert(!IsPrimeOp(pSelf) || !PathRef(pSelf));

	while (pSelf->XIn())
	{
		HOP pOp(pSelf->XIn()->data());
		assert(!IsArgOp(pOp));//should be adjourned already
		UnmakeUDLink(pSelf, pOp);
		if (pOp->m_xout.empty())		//last out xdep killed?
		{
			if (IsCallOutOp(pOp))
			{
				AdjournCallOutOp(pOp, redumpCache);
			}
		}
	}

	assert(!pSelf->XOut());

	pSelf->setInsPtr(nullptr);
	Delete(pSelf);
}

void FuncCleaner_t::AdjournRhs(HOP pSelf, RedumpCache_t &redumpCache)
{
	//?assert(pOp->isLinked());		//not linked somewhere?
//CHECKID(pSelf, 4030)
//STOP

	assert(IsPrimeOp(PRIME(pSelf)));

	OpList_t& l(pSelf->args());
	OpList_t::Iterator i(PRIME(pSelf)->argsIt());
	while (i)
	{
		HOP pOp(i.data());
		OpList_t::Iterator j(i++);
		l.erase(j);
		AdjournRhsOp(pOp, redumpCache);
		//DestroyOp(pOp);
	}
}

void FuncCleaner_t::Purge(RedumpCache_t &redumpCache)
{
	assert(!IsStub());
	FuncCleaner_t &FC(*this);

	for (PathTree_t::LeafIterator i(mrPathTree.tree()); i; ++i)
	{
		HPATH pPath(i.data());
		for (PathOpList_t::Iterator i(pPath->ops()); i; )
		{
			CHOP hOp(i.data());
//CHECKID(hOp, 0x13e7)
CHECK(OpNo(hOp) == 59)
STOP
			++i;
			if (IsVarOp(hOp) || IsArgOp(hOp))
			{
				int no = PathNo(pPath);
				//all references to the attached local are handled here as this:
				//1) all perm op's refs are removed
				//2) the one from itself is preserved
				//the hOp is disconnected and put into redumpCache.trans map
				FC.AdjournTransientOp(hOp, redumpCache);
				//FC.DestroyPrimeOp(hOp);//remove
			}
			else
			{
				if (IsCall(hOp))
				{
					FC.AdjournCallOuts(hOp, redumpCache);
					FC.AdjournRhs(hOp, redumpCache);
				}
				else if (IsRetOp(hOp))
				{
					FC.AdjournRhs(hOp, redumpCache);
				}
				FC.AdjournPermanentOp(hOp, redumpCache);
				RevertLocals(hOp);
				RevertFPUREG(hOp);
				hOp->ins().reset(0);// -RetAddrSize());
				hOp->reset();
				for (OpList_t::Iterator i(hOp->argsIt()); i; ++i)
					i.data()->reset();
			}
			//pPath->reset();
		}
		pPath->reset();
	}
#if(0)
	PurgeCallOuts(redumpCache);
#endif

#if(0)
	//all locals are preserved (along with disconnected rag/var ops)
	redumpCache.clear();
	PurgeDanglingVars();
	PurgeDanglingArgs();
#endif

	FC.Cleanup();
	FC.CleanupFinal();

	BlocksTracer_t BT(*this);
	BT.UndoPathTree();

//	PathOpTracer_t pathOpTracer;
//	BT.RedoPathTree(pathOpTracer);
#if(1)
	HPATH hBody(mrFuncDef.Body());//this check is required only for the head path?
	for (PathTree_t::ChildrenIterator i(hBody); i; )
	{
		HPATH hPath(i.data());
		if (!hPath->hasOps())
		{
			if (PathOpRefs(hPath).empty())
			{
				++i;
				hBody->UnlinkChild(hPath);
				memMgrEx().Delete(hPath);
				continue;
			}
		}
		++i;
	}
#endif

	//mrFuncDef.setPStackPurge(0);
	//mrFuncDef.setFStackPurge(0);
	SetChangeInfo(FUI_ALL);
	SetFuncStatus(0);//!
	SetFuncStatus(FDEF_DC_PHASE0);

//	assert(!mrFuncDef.locals());
}

void FuncCleaner_t::PurgeFinal(RedumpCache_t& redumpCache)
{
	PurgeCallOuts(redumpCache);
	redumpCache.clear();//everything else
	PurgeDanglingVars();
	PurgeDanglingArgs();
	CleanupFinal();
	DestroyBody();
}

void FuncCleaner_t::PurgeCallOuts(RedumpCache_t &redumpCache)
{
	while (!redumpCache.callout.empty())
	{
		auto i(redumpCache.callout.begin());
		HOP hOp(i->first);//callout
		redumpCache.callout.erase(i);
		assert(!LocalRef(hOp) && !hOp->isLinked() && !hOp->insPtr());
		Delete(hOp);
	}
}

void FuncCleaner_t::PurgeDanglingVars()
{
	if (HasLocalVars())
	{
		FieldMap& m(LocalVarsMap());
		for (FieldMapIt i(m.begin()); i != m.end(); )
		{
			FieldPtr pLocal(VALUE(i++));
//CHECKID(pLocal, 0x2721)
//STOP
			if (LocalRefs(pLocal).empty())
			{
				TakeLocal(pLocal);//slow!
				DeleteUnrefedLocalVar(pLocal);
			}
			else
			{
				HOP hOp(FindDanglingOp(pLocal));
				//if (iRefStatus < 0)
				if (hOp)
				{
					//HOP hOp(LocalRefs(pLocal).head()->data());
					ReleaseLocalRef(pLocal, hOp);
					Delete(hOp);
					assert(!FindDanglingOp(pLocal));//no more
					if (LocalRefs(pLocal).empty())
					{
						TakeLocal(pLocal);//slow!
						DeleteUnrefedLocalVar(pLocal);
					}
				}
			}
		}
		CheckStrucLocsEmpty();
	}
}

void FuncCleaner_t::CheckStrucLocsEmpty()
{
	TypePtr pStrucLoc(mrFuncDef.locals());
	if (!pStrucLoc || !pStrucLoc->typeStruc()->fields().empty())
		return;
	/*destroyTypeRef(*iLocals, true);
	iLocals->setOwner(nullptr);
	memMgr().Delete(iLocals);
	mrFuncDef->setLocals(nullptr);*/

	DestroyStrucLoc(*mrFuncDef.namesMgr());
}

void FuncCleaner_t::PurgeDanglingArgs()
{
	for (FuncArgsIt i(mrFuncDef); i; )
	{
		FieldPtr pLocal(VALUE(i++));
//CHECKID(pLocal, 0x2721)
//STOP
		HOP hOp(FindDanglingOp(pLocal));
		if (hOp)
		{
			HOP hOp(LocalRefs(pLocal).head()->data());
			ReleaseLocalRef(pLocal, hOp);
			Delete(hOp);
		}
		//DeleteUnrefedLocalArg(pLocal);
	}
}








