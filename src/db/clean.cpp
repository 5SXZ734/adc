#include "clean.h"
#include "shared/data_source.h"
#include "type_proc.h"
#include "type_strucvar.h"
#include "type_code.h"
#include "types_mgr.h"
#include "main.h"
#include "info_proj.h"
#include "info_module.h"

#define TEMPL	template <typename T>

/*TEMPL BinaryCleaner_t<T>::BinaryCleaner_t(Project_t &r)
: ProjectInfo_t(r),
mbClosing(false)
{
}*/

/*TEMPL BinaryCleaner_t<T>::BinaryCleaner_t(const T &r, MemoryMgr_t &mm)
: T(r, mm),
mbClosing(false)
{
}*/

/*TEMPL BinaryCleaner_t<T>::BinaryCleaner_t(const ModuleInfo_t &r)
: ModuleInfo_t(r),
mbClosing(false)
{
}*/

/*TEMPL BinaryCleaner_t<T>::BinaryCleaner_t(ModuleInfo_t &MI)
: ModuleInfo_t(MI),
mbClosing(false)
{
}

TEMPL BinaryCleaner_t<T>::BinaryCleaner_t(ProjectInfo_t &PI, TypePtr iModule)
: ModuleInfo_t(PI, *iModule),
mbClosing(false)
{
}*/

TEMPL BinaryCleaner_t<T>::~BinaryCleaner_t()
{
	/*while (!mDanglingTypes.empty())
	{
		std::set<TypePtr>::iterator i(mDanglingTypes.begin());
		TypePtr p(*i);
		mDanglingTypes.erase(i);

	}*/
	destroyUnrefedTypes();//?*this);
}


TEMPL void BinaryCleaner_t<T>::destroyBinaryData(Module_t &rSelf)
{
	DataPtr pData(rSelf.dataSourcePtr0());
	if (pData)
	{
		if (pData->releaseRef() == 0)
		{
			destroyDataObj(*pData);
			memMgr().Delete(pData);
		}
		rSelf.setDataSource(nullptr);
	}
}

TEMPL void BinaryCleaner_t<T>::destroyDataObj(DataObj_t &rSelf)
{
	switch (rSelf.pvt().dataId())
	{
	case DATAID_SOURCE:
		destroyDataSource(*rSelf.pvt().dataSource());
		break;
	case DATAID_LEECH:
		destroyDataLeech(*rSelf.pvt().dataLeech());
		break;
	default:
		assert(0);
	}
}

TEMPL void BinaryCleaner_t<T>::destroyDataSource(DataSource_t &)
{
}

TEMPL void BinaryCleaner_t<T>::destroyDataLeech(DataLeech_t &rSelf)
{
	DataPtr pData(rSelf.dataHost());
	assert(pData);
	if (pData->releaseRef() == 0)
	{
		destroyDataObj(*pData);
		memMgr().Delete(pData);
	}
	rSelf.setDataHost(nullptr);
}

TEMPL void BinaryCleaner_t<T>::destroyTypeRef(TypeObj_t &rSelf, bool bClearPvt, NamesMgr_t& rNS0)
{
CHECKID((&rSelf), 0xa5fe)
STOP

	Type_t *pType(rSelf.type());
	assert(pType);

	switch (pType->ObjType())
	{
	case OBJID_TYPE_PROJECT:
		assert(0);
		break;
	case OBJID_TYPE_MODULE:
		destroyModule(&rSelf);
		break;
	case OBJID_TYPE_STRUC:
	case OBJID_TYPE_STRUCLOC:
		destroyTypeStruc(&rSelf, rNS0);
		break;
/*	case OBJID_TYPE_UNION:
#if(!NEW_LOCAL_VARS)
	case OBJID_TYPE_UNIONLOC:
#endif
		destroyUnion(&rSelf, rNS0);
		break;*/
	case OBJID_TYPE_PTR:
	case OBJID_TYPE_REF:
	case OBJID_TYPE_RREF:
	case OBJID_TYPE_THISPTR:
		destroyTypePtr(&rSelf);
		break;
	case OBJID_TYPE_IMP:
		destroyTypeImp(&rSelf);
		break;
	case OBJID_TYPE_EXP:
		destroyTypeExp(&rSelf);
		break;
	case OBJID_TYPE_VPTR:
		destroy(*pType->typePtr());
		break;
	case OBJID_TYPE_ARRAY:
		destroyArray(&rSelf);
		break;
	case OBJID_TYPE_TYPEDEF:
		destroyTypedef(&rSelf);
		break;
	case OBJID_TYPE_SIMPLE:
		destroySimple(*pType->typeSimple());
		break;
	case OBJID_TYPE_SEG:
		destroySeg(&rSelf, rNS0);
		break;
	case OBJID_TYPE_PROC:
		destroyProc(&rSelf, rNS0);
		break;
	case OBJID_TYPE_CODE:
		destroyCode(&rSelf, rNS0);
		break;
	case OBJID_TYPE_STRUCVAR:
		destroyStrucvar(&rSelf, rNS0);
		break;
	case OBJID_TYPE_ENUM:
		destroyEnum(&rSelf);
		break;
	case OBJID_TYPE_ARRAY_INDEX:
		destroyArrayIndex(&rSelf);
		break;
	case OBJID_TYPE_CONST:
		destroyTypeConst(&rSelf);
		break;
	case OBJID_TYPE_THUNK:
		destroyTypeThunk(&rSelf);
		break;
	case OBJID_TYPE_PAIR:
		destroyTypePair(&rSelf);
		break;
	case OBJID_TYPE_FUNC:
		destroyTypeFunc(&rSelf);
		break;
	/*case OBJID_TYPE_BIT:
		destroyBit(&rSelf);
		break;*/
	case OBJID_TYPE_BITSET:
		destroyBitset(&rSelf, rNS0);
		break;
/*	case OBJID_TYPE_VARARRAY:
		destroy(*pType->typeVarArray());
		break;*/
	default:
		return;
	}
	if (bClearPvt)
		rSelf.ClearPvt();
}

TEMPL void BinaryCleaner_t<T>::destroyFileRef(Folder_t &rSelf, bool bClearPvt)
{
	File_t *pFile(rSelf.file());
	assert(pFile);

	switch (pFile->fileId())
	{
	default:
		return;
	//case FILEID_NULL:
	case FILEID_MODULE:
		break;
	case FILEID_FOLDER:
		break;
	case FILEID_RESOURCES:
		break;
	case FILEID_TYPES:
		break;
	case FILEID_NAMES:
		break;
	case FILEID_EXPORTS:
		break;
	}

	if (bClearPvt)
		rSelf.ClearPvt();
}

TEMPL void BinaryCleaner_t<T>::destroy(FieldRef rSelf)
{
	ClearType(&rSelf);
	//mrPI.SetType(&rSelf, nullptr);
}

TEMPL void BinaryCleaner_t<T>::destroyModule(TypePtr iSelf)
{
	class Module_t &rSelf(*iSelf->typeModule());

	//destroy seg ranges
	//this will not destroy subsegs because there are still refs in subsegs list (some subsegs may not be in the seg range)
	RangeSetMap	&ranges(rSelf.rangeMgr());
	while (!ranges.empty())
	{
		TypePtr iSegRange(*ranges.begin());
		ranges.erase(ranges.begin());
		if (iSegRange)
		{
			destroySeg(iSegRange, *rSelf.namesMgr());
			assert(iSegRange->refsNum() == 0);
			memMgr().Delete(iSegRange);
		}
	}

	static NamesMgr_t rNS0;//no higher authority..
	destroySeg(iSelf, rNS0);
	ReleaseDataRef(iSelf);

	memMgr().Delete(iSelf);
}

TEMPL void BinaryCleaner_t<T>::DestroyFolder(FolderPtr p)
{
	FileFolder_t *pFolder(p->fileFolder());
	if (pFolder)
	{
		FoldersMap &m(pFolder->children());
		while (!m.empty())
		{
			FoldersMap::iterator i(m.begin());
			FolderPtr p(m.take(i));
			DestroyFolder(p);
			memMgr().Delete(p);
		}
	}
	p->ClearPvt();
}

TEMPL bool BinaryCleaner_t<T>::DestroyFolders()
{
	FolderPtr p(mrProject.files().rootFolder());
	DestroyFolder(p);
	memMgr().Delete(p);
	mrProject.files().setRootFolder(nullptr);
	return true;
}

TEMPL void BinaryCleaner_t<T>::destroyProject()
{
	//clear context
	//?	if (pICtx)
	//?	pICtx->clear_all();

	//Folder_t *tmp1 = PI.ModuleFromPath("fsres.dll");
	//Seg_t *tmp2 = PI.ModuleOf(tmp1)->typeSeg();
	//Seg_t *tm3 = tmp2->subsegs().front()->typeSeg();
	setClosing(true);

	//destroyTypesNS();//clear NS types cache
	mrProject.OnDestroyProject(isClosing());

	if (mrProject.typeMgr())
	{
		destroyStockTypes();
	}

	T::RegisterTypesMap(mrProject.self(), false);
	//mrProject.deleteMemMgrNS();

	DestroyFolders();
}

/*TEMPL void BinaryCleaner_t<T>::destroyTypesNS()
{
	TypesMgr_t *pTypesMgr(mrProject.typesMgrNS());
	if (pTypesMgr)
	{
		destroyUserTypesNS();// , nullptr);// pNS);
		destroy(*pTypesMgr);
		mrProject.deleteTypesMgrNS();
	}
}*/

/*TEMPL void BinaryCleaner_t<T>::destroyExtraFields(TypePtr iSelf, NamesMgr_t& rNS0)
{
	Seg_t &rSelf(*iSelf->typeSeg());
	NamesMgr_t *pNS(rSelf.namesMgr());
	if (!pNS)//the seg has no n-map - its fields must use parent field's namespace
		pNS = &rNS0;

	ConflictFieldMap &m(rSelf.conflictFields());
	while (!m.empty())
	{
		FieldPtr pField(VALUE(m.begin()));
		FieldPtr pMasterField(T::CloneLead(pField));

		m.take(m.begin());
		destroyConflictField(pField, pNS);
	}
}*/

TEMPL bool BinaryCleaner_t<T>::UnregisterSubseg(TypePtr iSeg)
{
	Seg_t &rSelf(*iSeg->typeSeg());
	TypePtr iSuperSeg(rSelf.superLink());

	if (!iSuperSeg)
		return false;

CHECKID(iSeg, 0x355)
STOP

/*	SubSegMap &m(iSuperSeg->typeSeg()->subsegs());
	for (SubSegMapCIt i(m.begin()); i != m.end(); i++)
	{
//		assert(i != mSubSegs.end());
		if (IVALUE(i) == iSeg)
		{
			m.erase(i);
			break;
		}
	}*/

	Seg_t &rSeg(*iSeg->typeSeg());
	rSeg.setSuperLink(nullptr);

	//remove a range in super seg
	TypePtr pRange(rSeg.range(iSeg));
	if (pRange && DeleteRangeSeg(nullptr, pRange))
	{
		// rSeg.setRange(nullptr);
	}
	return true;
}

TEMPL bool BinaryCleaner_t<T>::DeleteRangeSeg(TypePtr iSuperSeg, TypePtr iSeg)
{
	TypePtr iRangeSet(iSeg->typeSeg()->ownerRangeSet(iSeg));
	if (!iRangeSet)
		return false;

	FieldMap &m(iRangeSet->typeSeg()->fields());

	//TypeObj_t *pRangeSet(mRangeMgr.getSlot(hRangeSet));
	//if (!pRangeSet)
		//return false;
	RangeIt it(m.find(iSeg->parentField()->_key()));
	if (it == m.end())
		return false;

	Field_t &r(*it);
	FieldPtr pField(&r);
	assert(pField->nameless());

	assert(iSeg->refsNum() > 1);//the range seg must not be deleted

	ClearType(pField);

	RangeIt itnx(it);//the upper bound
	itnx++;
	m.take(it);

	memMgr().Delete(pField);

	if (itnx != m.end() && !itnx->type())
	{
		pField = VALUE(itnx);
		m.take(itnx);
		memMgr().Delete(pField);
	}
	return true;
}

TEMPL void BinaryCleaner_t<T>::ReleaseDataRef(TypePtr iSelf)
{
	destroyBinaryData(*iSelf->typeModule());
}

TEMPL void BinaryCleaner_t<T>::destroySeg(TypePtr iSelf, NamesMgr_t& rNS0)
{
CHECKID(iSelf, 0x32ba)
STOP
	Seg_t &rSelf(*iSelf->typeSeg());

	NamesMgr_t *pNS(rSelf.namesMgr());
	if (!pNS)//if has no n-map - its contents must use (some) parent seg's one
		pNS = &rNS0;

	//destroy subsegs list
	SubSegMap &m(rSelf.subsegs());
	while (!m.empty())
	{
		SubSegMapCIt i(m.begin());
		TypePtr iSeg(IVALUE(i));
		destroySeg(iSeg, *pNS);
		m.erase(i);
		if (iSeg->releaseRef() != 0)
			ASSERT0;
		memMgr().Delete(iSeg);
	}

	//extra filds must be destroyed first because destroyTypeStruc() cleans up a namespace
	//destroyExtraFields(iSelf, rNS0);

#if(NEW_VARSIZE_MAP)
	rSelf.clearVarObjMap();
#endif

	//destroy this before super seg, as this may require a link to it
	destroyTypeStruc(iSelf, *pNS);//all children must be dead before types mgr is killed

	//destroyUnrefedTypes(*this);

	//unregister itself in seg owner
	UnregisterSubseg(iSelf);

	TypePtr iTrace(rSelf.traceLink());
	if (iTrace)
	{
//		iTrace->typeSeg()->setInferLink(nullptr);
		rSelf.setTraceLink(nullptr);
		//me responsible for destroying super seg as well
		FieldPtr pField(iTrace->parentField());
		destroyField(pField->owner(), pField);
	}

	T::ReleaseFrontEnd(rSelf.frontIndex());
	rSelf.setFrontIndex(0);
}

TEMPL void BinaryCleaner_t<T>::destroyProc(TypePtr iSelf, NamesMgr_t& rNS0)
{
	assert(!iSelf->hasUserData());
	destroyTypeStruc(iSelf, rNS0);
}

TEMPL void BinaryCleaner_t<T>::destroyCode(TypePtr iSelf, NamesMgr_t& rNS0)
{
	destroyCompound(iSelf, rNS0);
}

TEMPL void BinaryCleaner_t<T>::destroyCompound(TypePtr iSelf, NamesMgr_t& rNS0)
{
CHECKID(iSelf, 5534)
STOP
	Complex_t &rSelf(*iSelf->typeComplex());

	if (!iSelf->nameless())
	{
		assert(iSelf->isShared());
		T::ClearTypeName(rNS0, iSelf);
	}

	assert(rSelf.nameless());
	T::DeleteNamespace(iSelf);
}

TEMPL void BinaryCleaner_t<T>::destroyTypeStruc(TypePtr iSelf, NamesMgr_t& rNS0)
{
CHECKID(iSelf, 5534)
//CHECK(!iSelf->nameless() && MyString(iSelf->name()->c_str()) == "std")
STOP
	Struc_t &rSelf(*iSelf->typeStruc());
	NamesMgr_t *pNS(rSelf.namesMgr());
	if (!pNS)//the struc is unnamed - its fields must use parent field's namespace
		pNS = &rNS0;

//?	assert(!iSelf->isExpo rting());//must be dealed beforehand
		//RemoveExportedTypeInfo(iSelf);

	//if (pNS) pNS->check();
	clearFields(rSelf.fields(), pNS);
	//if (pNS) pNS->check();

	if (rSelf.typeMgr())
	{
		//if (rSelf.mpTypesMgr)
		destroyUserTypes(*rSelf.typeMgr(), *pNS);

		destroy(*rSelf.typeMgr());

		TypePtr iModule(T::ModuleOf(iSelf));
		if (iModule)
		{
			ModuleInfo_t MI(*this, *iModule);
			MI.RegisterTypesMapEx(iSelf, false);//deregister
		}
		else
			T::RegisterTypesMap(iSelf, false);
		T::DeleteTypesMgr(iSelf);
	}
	destroyCompound(iSelf, rNS0);//names
}

template <typename T>
class FieldMapCleaner_t
{
	template <typename, typename> friend class sbtree::deallocator_t;
	BinaryCleaner_t<T>& bc;
	FieldMap& m;
	NamesMgr_t* pns;
public:
	FieldMapCleaner_t(BinaryCleaner_t<T>& _bc, FieldMap& _m, NamesMgr_t* _pns)
		: bc(_bc),
		m(_m),
		pns(_pns)
	{
	}
	void clean()
	{
		sbtree::deallocator_t<FieldMap, FieldMapCleaner_t<T>> dealloc(m, *this);
		dealloc.clear();
	}
private:
	void destroy_node(FieldPtr node)
	{
		bc.destroyField(node, pns);
	}
};

TEMPL void BinaryCleaner_t<T>::clearFields(FieldMap &m, NamesMgr_t *pNS)
{

#if(0)//SLOW!
	FieldMapIt i(m.begin());//death march
	while (i != m.end())
	{
		FieldPtr pField(m.take(i));//i++
		destroyField(pField, pNS);
	}
#else//FAST

	FieldMapCleaner_t<T> cleaner(*this, m, pNS);
	cleaner.clean();
	assert(m.empty());
#endif
}

TEMPL void BinaryCleaner_t<T>::destroyField(TypePtr iStruc, FieldPtr pField)
{
	Struc_t &rStruc(*iStruc->typeStruc());
	assert(pField->nameless());
	assert(rStruc.IsMineField(pField));
	ADDR addr(pField->_key());
	if (rStruc.takeField0(addr) != pField)
		ASSERT0;
	destroyField(pField, nullptr);
}

TEMPL void BinaryCleaner_t<T>::destroyStrucvar(TypePtr iSelf, NamesMgr_t& rNS0)
{
CHECKID(iSelf, 0x337)
STOP
	Strucvar_t &rSelf(*iSelf->typeStrucvar());
	assert(rSelf.fields().empty());
	/*NamesMgr_t *pNS(rSelf.namesMgr());
	if (!pNS)
		return;
	if (&mrProject.memMgrNS() != &memMgr())
	{
		BinaryCleaner_t BC2(*this, mrProject.memMgrNS());
		BC2.clearFields(rSelf.fields(), pNS);
	}
	else
		clearFields(rSelf.fields(), pNS);*/
	assert(!rSelf.typeMgr());
	assert(!rSelf.namesMgr() || rSelf.namesMgr()->empty());
	destroyCompound(iSelf, rNS0);
}

TEMPL void BinaryCleaner_t<T>::destroyBitset(TypePtr iSelf, NamesMgr_t& rNS0)
{
	destroyTypeStruc(iSelf, rNS0);
}

/*TEMPL void BinaryCleaner_t<T>::destroyConflictField(FieldPtr pField, NamesMgr_t *pNS)
{
	destroyField(pField, pNS);
}*/

TEMPL void BinaryCleaner_t<T>::destroyField(FieldPtr pField, NamesMgr_t *pNS)
{
//CHECK(pField->type() && pField->type()->typeSegRef())
CHECKID(pField, 0xa23c)
STOP
	ClearType(pField);
	//mrPI.SetType(pField, nullptr);
	if (pNS)
		T::ClearFieldName(pField, pNS);
	destroy(*pField);
	memMgr().Delete(pField);
}

TEMPL void BinaryCleaner_t<T>::destroyUnion(TypePtr iSelf, NamesMgr_t& rNS0)
{
	destroyTypeStruc(iSelf, rNS0);
}

TEMPL void BinaryCleaner_t<T>::destroyTypePtr(TypePtr iSelf)
{
	TypePtr_t &rSelf(*iSelf->typePtr());
	TypePtr iType(rSelf.pointee());
	if (iType)
	{
		if (ReleaseTypeRef0(iType, !mbClosing))// && !mbClosing)
			pushUnrefedType(iType);
		rSelf.setPointee0(nullptr);
	}
}

TEMPL void BinaryCleaner_t<T>::destroyTypeImpPtr(TypePtr iSelf)
{
	TypeImp_t &rSelf(*iSelf->typeImp());
	TypePtr iType(rSelf.baseType());
	if (ReleaseTypeRef0(iType, !mbClosing))// && !mbClosing)
		pushUnrefedType(iType);
	rSelf.setBaseType(nullptr);
}

TEMPL void BinaryCleaner_t<T>::destroyTypeImp(TypePtr iSelf)
{
	TypeImp_t &rSelf(*iSelf->typeImp());
	TypePtr iType(rSelf.baseType());
	if (ReleaseTypeRef0(iType, !mbClosing))// && !mbClosing)
		pushUnrefedType(iType);
	rSelf.setBaseType(nullptr);
}

TEMPL void BinaryCleaner_t<T>::destroyTypeExp(TypePtr iSelf)
{
	TypeExp_t &rSelf(*iSelf->typeExp());
	TypePtr iType(rSelf.baseType());
	if (ReleaseTypeRef0(iType, !mbClosing))// && !mbClosing)
		pushUnrefedType(iType);
	rSelf.setBaseType(nullptr);
}

TEMPL void BinaryCleaner_t<T>::destroyTypeConst(TypePtr iSelf)
{
	TypeConst_t &rSelf(*iSelf->typeConst());
	TypePtr iType(rSelf.baseType());
	if (ReleaseTypeRef0(iType, !mbClosing))// && !mbClosing)
		pushUnrefedType(iType);
	rSelf.setBaseType0(nullptr);
}

TEMPL void BinaryCleaner_t<T>::destroyTypeThunk(TypePtr iSelf)
{
	TypeThunk_t &rSelf(*iSelf->typeThunk());
	TypePtr iType(rSelf.baseType());
	if (ReleaseTypeRef0(iType, !mbClosing))// && !mbClosing)
		pushUnrefedType(iType);
	rSelf.setBaseType(nullptr);
}

TEMPL void BinaryCleaner_t<T>::destroyTypePair(TypePtr iSelf)
{
	TypePair_t &rSelf(*iSelf->typePair());
	TypePtr iLeft(rSelf.left());
	if (iLeft)
	{
		if (ReleaseTypeRef0(iLeft, !mbClosing))// && !mbClosing)
			pushUnrefedType(iLeft);
		rSelf.setLeft(nullptr);
	}
	TypePtr iRight(rSelf.right());
	if (iRight)
	{
		if (ReleaseTypeRef0(iRight, !mbClosing))// && !mbClosing)
			pushUnrefedType(iRight);
		rSelf.setRight(nullptr);
	}
}

TEMPL void BinaryCleaner_t<T>::destroyTypeFunc(TypePtr iSelf)
{
	TypeFunc_t &rSelf(*iSelf->typeFunc());
	TypePtr iRetVal(rSelf.retVal());
	if (iRetVal)
	{
		if (ReleaseTypeRef0(iRetVal, !mbClosing))// && !mbClosing)
			pushUnrefedType(iRetVal);
		rSelf.setRetVal(nullptr);
	}
	TypePtr iArgs(rSelf.args());
	if (iArgs)
	{
		if (ReleaseTypeRef0(iArgs, !mbClosing))// && !mbClosing)
			pushUnrefedType(iArgs);
		rSelf.setArgs(nullptr);
	}
}

TEMPL void BinaryCleaner_t<T>::destroyArray(TypePtr iSelf)
{
CHECKID(iSelf, 0xbc0)
STOP
	Array_t &rSelf(*iSelf->typeArray());
	if (!(iSelf->flags() & TYP_NO_REF))
	{
		TypePtr iType(rSelf.baseType());
		if (ReleaseTypeRef0(iType, !mbClosing))// && !mbClosing)
			pushUnrefedType(iType);
	}
	rSelf.setBaseType(nullptr);
}

TEMPL void BinaryCleaner_t<T>::destroyEnum(TypePtr iSelf)
{
	TypeEnum_t &rSelf(*iSelf->typeEnum());
	//if (iType)
	if (!(iSelf->flags() & TYP_NO_REF))
	{
		TypePtr iType(rSelf.enumRef());
		if (ReleaseTypeRef0(iType, !mbClosing))// && !mbClosing)
			pushUnrefedType(iType);
	}
	rSelf.setEnumRef(nullptr);
}

TEMPL void BinaryCleaner_t<T>::destroyArrayIndex(TypePtr iSelf)
{
	TypeArrayIndex_t &rSelf(*iSelf->typeArrayIndex());
	if (!(iSelf->flags() & TYP_NO_REF))
	{
		TypePtr iBasicType(rSelf.arrayRef());
		if (ReleaseTypeRef0(iBasicType, !mbClosing))// && !mbClosing)
			pushUnrefedType(iBasicType);
	}
	rSelf.setArrayRef(nullptr);
	if (!(iSelf->flags() & TYP_NO_REF))
	{
		TypePtr pIndexRef(rSelf.indexRef());
		if (ReleaseTypeRef0(pIndexRef, !mbClosing))// && !mbClosing)
			pushUnrefedType(pIndexRef);
	}
	rSelf.setIndexRef(nullptr);
}

/*void ProjectInfo_t::destroy(VarArray_t &rSelf)
{
	set ype(rSelf, nullptr);
}*/

TEMPL void BinaryCleaner_t<T>::destroyTypedef(TypePtr iSelf)
{
	Typedef_t &rSelf(*iSelf->typeTypedef());
	TypePtr iType(rSelf.mpBaseType);
	if (iType)
	{
		assert(!iType->parentField());
		if (!iType->nameless())
		{
			//redirect name
			/*?TypesMgr_t *pTypeMgr(iType->ownerTypeMgr());
			assert(pTypeMgr);
			NamesMgr_t *pNS(T::Owner NamesMgr(pTypeMgr->owner(), nullptr));
			NamesMapIt it(pNS->findn(iType->name()->c_str()));
			if (it->obj() == iSelf)
			{
				assert(0);
				it->setObj(iType);
			}*/

			//RemoveNameRef(rNS, pSelf->name());
			//pSelf->setName(nullptr);
		}

		if (ReleaseTypeRef0(iType, !mbClosing))// && !mbClosing)
			pushUnrefedType(iType);
		rSelf.mpBaseType = nullptr;
	}

	if (!iSelf->nameless())
	{
		TypesMgr_t *pTypeMgr2(iSelf->ownerTypeMgr());
		assert(pTypeMgr2);
		//NamesMgr_t *pNS2(T::Owner NamesMgr(pTypeMgr2->owner(), nullptr));
		NamesMgr_t *pNS2(pTypeMgr2->owner()->typeComplex()->namesMgr());
		assert(pNS2);
		T::ClearTypeName(*pNS2, iSelf);
	}
}

TEMPL void BinaryCleaner_t<T>::destroyUserTypes(TypesMgr_t &rSelf, NamesMgr_t& rNS0)
{
	//there must be no type refs outside the scope of type map's owner.
	//can't do it in single pass - succeeding types may ref the preceeding ones.

	const TypesMap_t &m(rSelf.aliases());

	//first pass - release type map's refs
	for (TypesMapCIt i(m.begin()); i != m.end(); i++)
	{
		TypePtr iType(i->pSelf);
CHECKID(iType, 0x337)
STOP

//?		if (iType->refsNum() > 0)//there can be unref'd types
	//?		iType->releaseRef();
		if (iType->typeProxy())//proxies cannot be destroyed now as it's aka's may be used in this loop
		{
			destroyTypeRef(*iType, false, rNS0);
		}
		else if (iType->typeComplex())
		{
			assert(!iType->typeSeg());
			iType->releaseDevice();
			destroyTypeRef(*iType, false, rNS0);
		}
	}

	//second pass - clean the map up and possible destroy unref'd types
	while (!rSelf.empty())
	{
//?		TypesMgr_t *pTypeMgr2(m.begin()->second.pSelf->ownerTypeMgr());
	//?	assert(pTypeMgr2 && pTypeMgr2 == &rSelf);

		TypePtr iType(rSelf.takeFront());
CHECKID(iType, 0xa7ba)
STOP

		//if (iType->refsNum() > 0)//there can be unref'd types
		if (iType->refsNum() == 0)
		{
			//if (iType->typeProxy())//destroy proxies here, must not be referenced(?)
			{
				//assert(iType->refsNum() == 0);
				//?destroyTypeRef(*iType, false);
			}

			destroyTypeRef(*iType, true, rNS0);
			iType->setOwner(nullptr);
			memMgr().Delete(iType);
		}
		else
		{
#if(1)//crash!
			T::ClearTypeName(rNS0, iType);
#endif

			//destroyTypeRef(*iType, true);//do not delete it - the type can be referenced from another module(!)
			iType->setOwner(nullptr);

			//add it to attic?
#if(1)
			mrProject.typeMgr()->addNameAlias(iType);
#endif
		}
	}

	//clear the aka map
//	rSelf.mAliases.clear();
}

TEMPL void BinaryCleaner_t<T>::destroyUserTypesNS(TypesMgr_t &rSelf)
{
	static NamesMgr_t rNS0;
	const TypesMap_t &m(rSelf.aliases());
	while (!rSelf.empty())
	{
		TypePtr iType(rSelf.takeFront());
		assert(!iType->typeProxy());
		assert(!iType->typeComplex());
		assert(iType->refsNum() == 0);
		destroyTypeRef(*iType, true, rNS0);
		memMgr().Delete(iType);
	}
}

TEMPL void BinaryCleaner_t<T>::destroy(TypesMgr_t &rSelf)
{
#if(0)
	NamesMgr_t * ns = rSelf.ownerCplx()->typeComplex()->namesMgr();

	while (!rSelf.mTypes.empty())
	{
		TypesMgr_t::setTYPE_it i(rSelf.mTypes.begin());
		while (i != rSelf.mTypes.end())
		{
			TypePtr pType(*i);
			TypesMgr_t::setTYPE_it j(i);
			j++;
CHECKID(pType, -102)
STOP
			if (pType->refsNum() == 1)
			{
				pType->releas eRef();
				rSelf.mTypes.erase(i);
				if (ns && !pType->nameless())
				{
					mrPI.RemoveNameRef(*ns, pType->name());
//					if (!ns->remo ven(pType, memMgr()))//the namespace has not been initialized yet?
						pType->setName(nullptr);
				}
				rSelf.enableRemovals(false);
				destroyTypeRef(*pType, true);
				rSelf.enableRemovals(true);
				mrPI.memMgr().Delete(pType);
			}
			else
			{
				rSelf.enableRemovals(false);
				destroyTypeRef(*pType, false);
				rSelf.enableRemovals(true);
			}
			i = j;
		}
	}
#endif

/*#if(1)
	while (!rSelf.mStockTypes.empty())
	{
		TypesMgr_t::StockTypesIt_t i(rSelf.mStockTypes.begin());
		TypePtr iType(i->second);
		if (iType->releaseRef() != 0)
		{
			fprintf(STDERR, "Warning: Stock type '%s' being destroyed is still referenced\n", mrPI.TypeName(iType).c_str());
			while (iType->releaseRef() != 0);
		}
		rSelf.mStockTypes.erase(i);
		mrPI.memMgr().Delete(iType);
	}
#else
	rSelf.mStockTypes.clear();
#endif*/

	//rSelf.mAliases.clear();
	assert(rSelf.aliases().empty());
}

TEMPL
void BinaryCleaner_t<T>::destroyUnrefedTypes()//?BinaryCleaner_t &BC)
{
	while (hasUnrefedTypes())
	{
		DestroyTypeRef(popUnrefedType());
	}
}

TEMPL void BinaryCleaner_t<T>::ClearType(FieldPtr pSelf)
{
	TypePtr iType(pSelf->mpType);
	if (iType)
	{
		if (ReleaseTypeRef0(iType, !mbClosing))// && !mbClosing)
			pushUnrefedType(iType);
		pSelf->setType0(nullptr);
	}
}

static bool shouldUnlinkType(TypePtr iType)
{
	if (iType->refsNum() > 2)
		return false;
	if (iType->typePtr() || iType->typeArray() || iType->typeConst() || iType->typeEnum() || iType->typeProxy())
		return true;
	Struc_t *pStruc(iType->typeStruc());
	if (pStruc && pStruc->nameless() && !pStruc->typeMgr())
		return true;
	return false;
}

TEMPL bool BinaryCleaner_t<T>::ReleaseTypeRef0(TypePtr iType, bool bUnlink)//unlink in types map if unref'd (IF NOT CLOSING!)
{
	if (!iType)
		return false;

CHECKID(iType, 0x903d)
STOP
CHECKID(iType, 0x904f)
STOP
		//OutputDebugString("-\n");

		//if (mpBaseType->typeComplex())
		//mpBaseType->typeComplex()->ReleaseObjRef(type obj(), G_Mem Mgr);

	TypesMgr_t *pTypeMgr;
	if ((pTypeMgr = iType->ownerTypeMgr()) != nullptr)
	{
CHECK(iType->typeProxy())
STOP

		//bool bRemoveFromTypesMap(!mbClosing);//if unrefed

		bool bUnlink2(bUnlink && shouldUnlinkType(iType));

		if (pTypeMgr->releaseTypeRef(iType, bUnlink2) > 0)
			return false;

		iType->releaseDevice();

		assert(!iType->parentField());
		NamesMgr_t *pNS(T::OwnerNamesMgr(pTypeMgr->owner(), nullptr));
		if (pNS)
			T::ClearTypeName(*pNS, iType);

		mrProject.OnReleaseTypeRef(iType);//give dc a chance to remove it from file
		mrProject.markDirty(DIRTY_TYPES);
		return true;
	}

	if (iType->releaseRef() > 0)
	{
		iType->setParentField(nullptr);//prevent it from referencing a dead field
	}
	else
	{
		assert(iType->typeProxy() || iType->nameless());
		DestroyTypeRef(iType);
	}
	return false;
}

TEMPL void BinaryCleaner_t<T>::DestroyTypeRef(TypePtr iType)
{
CHECKID(iType, 0x1178)
STOP

	NamesMgr_t& rNS0(*T::OwnerNamesMgr(iType, nullptr));

	//Project_t &rProj(Project());
	if (!iType->parentField())//shared only
	{
		assert(this->IsMemMgrGlob());
//?		mrProject.OnReleaseTypeRef(iType);//give dc a chance to remove it from file
		destroyTypeRef(*iType, true, rNS0);
		iType->setOwner(nullptr);
//?		mrProject.markDirty(DIRTY_TYPES);
	}
	else
	{
		//BinaryCleaner_t PC(*this);
		destroyTypeRef(*iType, true, rNS0);
		iType->setParentField(nullptr);
	}
	memMgr().Delete(iType);
}

TEMPL bool BinaryCleaner_t<T>::ReleaseTypeRef(TypePtr iType)
{
	if (!ReleaseTypeRef0(iType, !mbClosing))// && !mbClosing)
		return false;
	DestroyTypeRef(iType);
	return true;
}


TEMPL void BinaryCleaner_t<T>::destroyStockTypes()
{
	destroyUnrefedTypes();//?*this);//if any

	NamesMgr_t *pNS(mrProject.namesMgr());

	TypesMgr_t *ptm(mrProject.typeMgr());
	const TypesMap_t &m(ptm->aliases());

	//first pass - release type map's refs
	for (TypesMapCIt i(m.begin()); i != m.end(); i++)
	{
		TypePtr iType(i->pSelf);
		assert(!iType->typeProxy());
		assert(!iType->typeSeg());
		iType->releaseDevice();
		destroyTypeRef(*iType, false, *pNS);
		assert(iType->nameless());
		//if (iType->typeComplex())
			//T::ClearTypeName(*pNS, iType);
		assert(!iType->baseType());
	}

	//none of the types now references other types!
	while (!ptm->empty())
	{
		TypePtr iType(ptm->takeFront());

		//only simple times!!!
		assert(!iType->typeProxy());
		//assert(!iType->typeComplex());

		while (iType->refsNum() > 0)
			iType->releaseRef();

		//iType->setBaseType(nullptr);

		destroyTypeRef(*iType, true, *pNS);//delete a pvt
		iType->setOwner(nullptr);
		memMgr().Delete(iType);
	}

	T::DeleteNamespace(mrProject.self());//names
	T::DeleteTypesMgr(mrProject.self());
}


/////////////////////////////////////////////////////////

TEMPL
ModuleCleaner_t<T>::ModuleCleaner_t(const ModuleInfo_t &r)
	: BinaryCleaner_t<T>(r)
{
}

TEMPL
void ModuleCleaner_t<T>::destroyModule()
{
}


//explicit template instantiation
template class BinaryCleaner_t<ProjectInfo_t>;
//template class BinaryCleaner_t<ModuleInfo_t>;


