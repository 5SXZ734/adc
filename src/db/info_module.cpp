#include "type_module.h"
#include "qx/MyStringList.h"
#include "clean.h"
#include "names.h"
#include "symbol_map.h"
#include "type_proxy.h"
#include "info_types.h"
#include "main.h"
#include "anlzbin.h"


////////////////////////////////////////////// I_DataSource

I_DataSource::I_DataSource()
	: mpAuxData(nullptr)
{
}
I_DataSource::~I_DataSource()
{
	clearAuxData();
}

void I_DataSource::setAuxData(PDATA p, size_t size)
{
	if (mpAuxData)
	{
		mpAuxData->Release();
		mpAuxData = nullptr;
	}
	if (p)
	{
		mpAuxData = new AuxDataSource_t(p, size, true);//data must be copied - it may come from another module
		return;
	}
	assert(size == 0);
}

void I_DataSource::clearAuxData()
{
	if (mpAuxData)
	{
		mpAuxData->Release();
		mpAuxData = nullptr;
	}
}

I_DataSourceBase *I_DataSource::openFile(const char *path, bool bLocalFirst) const
{
	if (path)
	{
		MyPath aPath(path);
		DataSource_t *pDataSource(new DataSource_t);
		if (bLocalFirst)
		{
			MyPath aRefPath(modulePath());
			if (!aRefPath.IsNull())
			{
				MyPath aPath2(aPath.Name(), aRefPath);
				if (pDataSource->load(aPath2.Path().c_str()))//try current location
					return pDataSource;
			}
		}
		if (pDataSource->load(aPath.Path()))
			return pDataSource;
		delete pDataSource;
	}
	return nullptr;
}

MyString ModuleInfo_t::ModulePath() const
{
	if (mrModule.dataSourcePtr0())
		return mrModule.dataSourcePtr0()->dataSource()->path();
	return "";
}

FolderPtr ModuleInfo_t::FolderOfKind(FTYP_Enum e) const
{
	return mrModule.special(e);
}

TypePtr ModuleInfo_t::FindTypeByName(const MyString &s0) const
{
	//check in a front seg
	TypePtr iScope(FindFrontSeg());
	if (iScope)
		return FindTypeByName(s0, iScope);
	return nullptr;
}

ObjPtr ModuleInfo_t::FindObjByAutoName(const MyString &s, CTypePtr iScope) const
{
	value_t v;
	int kind(mrProject.checkAutoPrefix(s, &v));
	if (kind == 0)
		return nullptr;
	//autoname detected!
#ifdef _DEBUG
	if (kind == 1 || kind == -1)//a type
#else
	if (kind == 1)//a type
#endif
	{
		int compId(v.i32);
#ifdef _DEBUG
		if (kind < 0)
			compId = -compId;
#endif
		const TypesMgr_t *ptm(iScope->typeStruc()->typeMgr());
		if (ptm)
			return ptm->findTypeByCompId(compId);
	}
	if (kind == 2)//field
	{
		//?assert(0);//later
		return FindFieldInSubsegs(FindFrontSeg(), v.ui32);
	}
	return nullptr;
}

ObjPtr ModuleInfo_t::FindObjByName(const MyString &s, CTypePtr iScope) const
{
	//try a namespace
	NamesMgr_t *pNs(iScope->typeComplex()->namesMgr());
	if (!pNs)
		return nullptr;
	//NamespaceInitialized((TypePtr)iScope);
	return pNs->findObj(s.c_str());
}

TypePtr ModuleInfo_t::FindTypeByName(const MyString &s0, CTypePtr iScope) const
{
	MyStringList l(MyStringList::split("::", s0));
	assert(!l.empty());
	for (;;)
	{
		MyString s(l.front());
		l.pop_front();
		ObjPtr pObj;
		if ((pObj = FindObjByAutoName(s, iScope)) == nullptr)
			pObj = FindObjByName(s, iScope);
		if (!pObj)
			break;
		CTypePtr iType(pObj->objTypeGlob());
		if (!iType)
			break;
		if (l.empty())
			return (TypePtr)iType;
		iScope = iType;
	}
	return nullptr;
}

FolderPtr ModuleInfo_t::AssureNamesFile() const
{
	return AssureFolderOfKind(FTYP_NAMES);
	//FileNames_t *rFile(pFolder->m.fileNames());
}

FolderPtr ModuleInfo_t::AssureExportsFile() const
{
	return AssureFolderOfKind(FTYP_EXPORTS);
}

FolderPtr ModuleInfo_t::AssureImportsFile() const
{
	return AssureFolderOfKind(FTYP_IMPORTS);
}

/*class DataValueIterator : public ModuleInfo_t
{
	//container's base
	class scope_t : public std::list<std::pair<int,int>>//dim:cur stack (for arrays of strucs)
	{
		TypePtr mpSelf;//struct
		const FieldMap& m;
		OFF_t moBeg;
		FieldMapCIt mit;
	public:
		scope_t(DataValueIterator &owner, TypePtr p)
			: mpSelf(p),
			m(p->typeStruc()->fields()),
			moBeg(owner.dataStream().current()),
			mit(m.begin())
		{
		}
		void addDim(int n) { push_back(std::make_pair(n, 0)); }
		TypePtr cont() const { return mpSelf; }
		virtual TypePtr valueType() const
		{
			CFieldPtr pField(VALUE(mit));
			return pField->type();
		}
		virtual bool isValid() const
		{
			return mit != m.end();
		}
		virtual void advance()
		{
			++mit;
		}
		unsigned offset() const
		{
			CFieldPtr pField(VALUE(mit));
			return pField->offset();
		}
	};
private:
	TypePtr mpSeg;
	DataSubSource_t mData;
	DataStream_t mRaw;
	std::list<scope_t>	mStack;
	TypePtr mpTop;
public:
	DataValueIterator(ProjectInfo_t& r, TypePtr pModule, CFieldPtr pField)
		: ModuleInfo_t(r, *pModule),
		mpSeg(OwnerSeg(pField->owner())),
		mData(GetDataSource()->pvt(), mpSeg->typeSeg()->rawBlock()),
		mRaw(mData, pField->offset()),
		mpTop(pField->type())
	{
	}
	DataStream_t& dataStream() { return mRaw; }
	operator bool() const
	{
		return false;
	}
	void operator++()
	{
	}
private:
	void advance(TypePtr pType)
	{
		if (!pType)
		{
			mRaw.skip(OPSZ_BYTE);
			return;
		}
		Array_t* pArray_p(pType->typeArray());
		if (pArray_p)
		{
			pushScope(pType);
			if (!top())
				scope_t& a(push(pArray_p->total()));
			TypePtr pBaseType(pType->typeArray()->absBaseType());
			if ()
		}
	}
	scope_t &top() const { return *this; }
	void pushScope(TypePtr p)
	{
		if (mStack.empty() || mStack.back().cont())//empty or is in struc
			mStack.push_back(scope_t(*this, p->typeArray() ? nullptr : p));//do not push an array
		if (p->typeArray())
			mStack.back().addDim(p->typeArray()->total());
	}
};*/


bool ModuleInfo_t::SetFieldName(FieldPtr pField, const char *pName, bool bForce) const
{

	//NamesMgr_t *pNS(ModuleOf()->typeModule()->namesMgr());
	MemoryMgr_t *pMemMgr(&mrMemMgr);

	if (pField->owner()->typeStrucvar())
	{
		PrintError() << "Cannot rename fields of context dependent object (" << TypeName(pField->owner()) << ")" << std::endl;
		return false;
	}

	if (pName && mrProject.isAutoname(pName, ModulePtr()))
	{
		PrintError() << "Command 'setnam' failed. '" << pName << "' may collide with autoname" << std::endl;
		return false;
	}

CHECKID(pField, 58064)
STOP
	TypePtr iNSOwner(mrProject.GetNameOwnerOf(pField->owner()));
	NamesMgr_t *pNS(AssureNamespace(iNSOwner));
	//NamespaceInitialized(iNSOwner);

	//pNS = namespaceOf(pField);
	//pNS = mrProject.GetNamespace(pField);
	//ProjectInfo_t PI2(mrProjRef, *pMemMgr);

	//if (!pName || !bForce)
	{
		SetFieldName(*pNS, pField, pName, bForce);
		return true;
	}

/*	int iTry(1);
	MyString s0(pName);
	//for (;;)
	{
		//MyString s(MyStringf("%s%d", s0.c_str(), ++iTry));
		if (SetObjName(*pNS, pField, s0.c_str()))
			break;
	}*/
	return true;
}

bool ModuleInfo_t::SetTypeName(TypePtr pType, const char *pName, bool bForce) const
{
	FieldPtr pField(pType->parentField());
	
	MemoryMgr_t *pMemMgr(&mrMemMgr);
	if (pField)
		return SetFieldName(pField, pName, bForce);

	if (pType->typeEnum())
		pType = pType->baseType();

	//shared types!
	TypesMgr_t *pTypeMgr(pType->ownerTypeMgr());
	assert(pTypeMgr);//?

	NamesMgr_t *pNS(OwnerNamesMgr(pTypeMgr->owner(), nullptr));
	//NamesMgr_t *pNS(OwnerNamesMgr(pType, nullptr));
	//NamesMgr_t *pNS(mrProject.namesMgr());
	if (!pName || !bForce)
	{
		SetTypeName(*pNS, pType, pName);//, *pMemMgr);
		return true;
	}

	int iTry(1);
	MyString s0(pName);
	for (;;)
	{
		MyString s(MyStringf("%s%d", s0.c_str(), ++iTry));
		if (SetTypeName(*pNS, pType, s.c_str()))
			break;
	}
	return true;
}


FieldPtr ModuleInfo_t::AddSecondaryField(TypePtr iSeg, ADDR addr, FieldPtr pField5) const
{
CHECK(addr == 0x5f4918e0)
STOP

	FieldPtr pField(pField5);
	
	Seg_t &rSeg(*iSeg->typeSeg());
	FieldMap &m(rSeg.fields());
	
	FieldMapIt i(m.find(addr));
	
	if (i == m.end())//vacant
	{
		assert(!pField);
		pField = VALUE(InsertUniqueFieldIt(iSeg, addr, mrMemMgr.NewField(addr)));//regular field added, nothing to do extra
		assert(pField->_key() == addr);
	}
	else
	{
		//Location is already occupied
		
		//Check if it's already a shared field,
		FieldPtr pField0(VALUE(i));
//CHECKID(pField0, 0x9522)
//STOP
		if (pField0->isTypeImp())
			return nullptr;

		if (!pField)
			pField = mrMemMgr.NewField(addr);

		if (m.insert(pField) == m.end())
			ASSERT0;
	}
	
	pField->setOwnerComplex(iSeg);
	return pField;
}

NamesMgr_t *ModuleInfo_t::AssureNamespace(TypePtr p) const
{
	if (!p)
		p = ModulePtr();
	if (p != ModulePtr() && !p->typeProject() && !p->typeFuncDef())
		if (!p->typeComplex()->namesMgr())
			AssureNamesFile();
	return AssureNamespace0(p);
}

TypesMgr_t *ModuleInfo_t::AssureTypeMgr(TypePtr iSelf)
{
//CHECKID(iSelf, -36)
//STOP
	if (!iSelf)
		iSelf = ModulePtr();
	assert(iSelf->typeStruc());
	Struc_t &rSelf(*iSelf->typeStruc());
	if (!rSelf.typeMgr())
	{
		//CreateTypesMgr(binary());
		if (iSelf != ModulePtr())
			AssureTypeMgr(ModulePtr());
		TypesMgr_t *pTypesMgr(NewTypesMgr(iSelf));
		if (iSelf == ModulePtr())
		{
#if(!SHARED_STOCK_TYPES)
			StockTracer_t ST(mrProject, *pTypesMgr);
			ST.AddStockTypes();
#endif
		}
		RegisterTypesMapEx(iSelf, true);
	}
	return rSelf.typeMgr();
}

void ModuleInfo_t::RegisterTypesMapEx(TypePtr iOwner, bool bRegister) const
{
CHECKID(iOwner, 0xe75e)
STOP
	assert(iOwner->typeStruc()->typeMgr());
	if (bRegister)
	{
		FolderPtr pFolder(AssureFolderOfKind(FTYP_TYPES));		
		FileTypes_t *rFile(pFolder->fileTypes());
		std::set<TypePtr> &m(rFile->typeMaps());
		if (!m.insert(iOwner).second)
			ASSERT0;
	}
	else
	{
		FolderPtr pFolder(FolderOfKind(FTYP_TYPES));
		assert(pFolder);
		FileTypes_t *rFile(pFolder->fileTypes());
		std::set<TypePtr> &m(rFile->typeMaps());
		if (m.erase(iOwner) != 1)
			ASSERT0;
		//TODO: delete file if empty
	}
}

FolderPtr ModuleInfo_t::AssureFolderOfKind(FTYP_Enum e) const
{
	FolderPtr pFolder(AssureSpecialFile(e));
	assert(pFolder);
	TypePtr pSeg(FindFrontSeg());
	if (!pFolder->hasPvt())
	switch (e)
	{
	case FTYP_TYPES:
		pFolder->SetPvt(new FileTypes_t);
		break;
	case FTYP_NAMES:
	{
		NamesMgr_t *pns(AssureNamespace(pSeg));
		pFolder->SetPvt(new FileNames_t(pns));
		break;
	}
	case FTYP_EXPORTS:
	{
		NamesMgr_t* pns(AssureNamespace(pSeg));
		pFolder->SetPvt(new FileExports_t(pns));
		break;
	}
	case FTYP_IMPORTS:
	{
		NamesMgr_t* pns(AssureNamespace(pSeg));
		pFolder->SetPvt(new FileImports_t(pns));
		break;
	}
	default:
		break;
	}
	return pFolder;
}

FolderPtr ModuleInfo_t::AssureSpecialFile(FTYP_Enum ftyp) const//, bool bFileDef)
{
	//Folder_t &rTopFolder
	//TypePtr iModule(ModuleOf(rTopFolder));
	//Module_t &aBin(*iModule->typeModule());
	Folder_t *pFolder(mrModule.special(ftyp));
	if (!pFolder)
	{
		//Folder_t &rTopFolder(rDC.binaryFolder());
		MyString s(FTYP2name((FTYP_Enum)ftyp));//ADC_DIR);
		if (s.empty())
			s = FolderType2Ext((FTYP_Enum)ftyp);
		s.prepend(ADC_DIR);
		//s.prepend(MODULE_SEP);
		ZPath_t z(mrModule.folderPtr()->name() + s);

		pFolder = AddFile(z.toString());
		/*if (bFileDef)
		{
			pFolder->m.SetUserData(new FileDef_t(pFolder));
			FileDef_t *pFileDef(FILEDEF(pFolder));
		}*/
		mrModule.setSpecial(ftyp, pFolder);
	}
	return pFolder;
}

PNameRef ModuleInfo_t::AddName(NamesMgr_t &rNS, const char *pcName, ObjPtr pObj, bool bShare) const
{
CHECKID(pObj, 5535)
//CHECK(sName == "_CxxThrowException")
STOP

	//assert(rNS.isInitialized());
	//if (!rNS.isInitialized())
	rNS.initialize();
	FieldPtr pField(nullptr);
	TypePtr pType(nullptr);

	if (pObj)
	{
		pField = pObj->objField();
		if (!pField)
			pType = pObj->objTypeGlob();
	}

	PNameRef pnOld(nullptr);// rNS.end());
	if (pField)
	{
		if (!pcName || !pcName[0])//empty
		{
			if (!pField->nameless())
			{
				RemoveNameRef(rNS, pField->name());
				pField->setName(nullptr);
			}
			//pField->flags() &= ~FLD_SHAREDNAME;
			return nullptr;
		}

		//alternative name can reference an already named object
		if (!pField->nameless())//can be nullptr when loading
		{
			//NameRef_t sObj(((char *)pField->name()->c_str());
			pnOld = pField->name();// rNS.findn(&sObj);
			//sObj.clearPvt();
		}
	}
	else
	{
		if (!pcName || !pcName[0])//empty
		{
			RemoveNameRef(rNS, pField->name());
			pType->setName(nullptr);
			return nullptr;
		}

		if (!pType->nameless())//can be nullptr when loading
		{
			pnOld = pType->name();
		}
	}

	assert(!pnOld || rNS.findn(pnOld->c_str()) == pnOld);

	PNameRef pNewName(mrMemMgr.NewNameRef(pcName));
	pNewName->setObj(pObj);
#if(SHARED_NAMES)
	pNewName->addRef();
#endif

	NamesMapIt itp(rNS.insertIfNotExist(pNewName));
	if (itp != rNS.end())
	{
#ifdef _DEBUG
		CFieldPtr fld(itp->obj()->objField());
		CTypePtr typ(itp->obj()->objTypeGlob());
#endif
		//there was no such name in this namespace
		if (pnOld)
		{
			NamesMapIt itOld(pnOld);
			PNameRef pn(rNS.removeIt(itOld));
#if(SHARED_NAMES)
			pOldName->releaseRef();
#endif
			mrMemMgr.Delete(pn);
		}
	}
	else
	{

//#ifdef _DEBUG
//		FieldPtr p2(itp.first->second.pObj->objField());
//		TypePtr p3(itp.first->second.pObj->objTypeGlob());
//#endif

#if(SHARED_NAMES)
		pNewName->releaseRef();
#endif
		mrMemMgr.Delete(pNewName);
		if (!bShare)
			return nullptr;
		PNameRef pNameRef0(&(*itp));
		assert(pField);
		pField->setName(pNameRef0);
		//pField->flags() |= FLD_SHAREDNAME;
		return pNameRef0;
	}

	PNameRef ps(&(*itp));
//CHECK(!strcmp(ps->c_str(), "UNK"))
//STOP
	if (pField)
		pField->setName(ps);
	else //if (!pType->typeProxy())
		pType->setName(ps);
	return ps;
}

PNameRef ModuleInfo_t::AddPrettyName(NamesMgr_t &rNS, const char *pcName, TypeBasePtr pObj) const
{
CHECKID(pObj,0x2c5)
//CHECK(sName == "_CxxThrowException")
STOP

	rNS.initialize();
/*	FieldPtr pField(nullptr);
	TypePtr pType(nullptr);

	if (pObj)
	{
		pField = pObj->objField();
		if (!pField)
			pType = pObj->objTypeGlob();
	}*/

	assert(pcName && pcName[0]);

	NamesMapIt itOld(rNS.end());
	/*if (pField)
	{
		if (!pcName || !pcName[0])//empty
		{
			RemoveNameRef(rNS, pField->name());
			pField->setName(nullptr);
			return nullptr;
		}
	}
	else
	{
		if (!pcName || !pcName[0])//empty
		{
			RemoveNameRef(rNS, pField->name());
			pType->setName(nullptr);
			return nullptr;
		}
	}*/

	PNameRef pNewName(mrMemMgr.NewNameRef(pcName));
	pNewName->setObj(pObj);

	NamesMapIt itp(rNS.insertIfNotExist(pNewName));
	if (itp != rNS.end())
	{
		//there was no such name in this namespace
		if (itOld != rNS.end())//?
		{
			PNameRef pOldName(rNS.removeIt(itOld));
			mrMemMgr.Delete(pOldName);
		}
	}
	else
	{
		mrMemMgr.Delete(pNewName);
		return nullptr;
	}

	PNameRef ps(&(*itp));
	return ps;
}

//forceMode:
//	-1: try add the name as is, if failed - return;
//	0: try add the name as is, if failed - start aappending suffix until success
//	1: do not try to add name as is, start appending suffix until success
PNameRef ModuleInfo_t::ForceName(NamesMgr_t &rNS, const char *pcName, ObjPtr pObj, int forceMode) const
{
	assert(pcName);
//CHECK(MyString(pcName) == "ADCGuiThread")
//CHECK(strcmp(pcName, "CloseHandle") == 0)
//STOP
	MyString s(pcName);
	assert(!s.isEmpty());
	PNameRef pn(forceMode < 1 ? AddName(rNS, s, pObj, false) : nullptr);
	if (!pn)
	{
		/*if (forceMode == 1)
		{
			for (int i(1);; i++)
			{
				s.append("~");
				pn = AddName(rNS, s, pObj);
				if (pn)
					break;
			}
		}
		else if (forceMode == 2)*/
		if (forceMode >= 0)
		{
			for (int i(1);; i++)
			{
				std::ostringstream ss;
				ss << s << DUB_SEPARATOR << i << DUB_TERMINATOR;//duplicate #
				pn = AddName(rNS, ss.str().c_str(), pObj, false);
				if (pn)
					break;
			}
		}

	}
	return pn;
}

PNameRef ModuleInfo_t::ForcePrettyName(NamesMgr_t &rNS, const char *pcName, TypeBasePtr pObj, int forceMode) const
{
	assert(pcName);
	MyString s(pcName);
	assert(!s.isEmpty());
	PNameRef pn(forceMode < 1 ? AddPrettyName(rNS, s, pObj) : nullptr);
	if (!pn)
	{
		if (forceMode >= 0)
		{
			for (int i(1);; i++)
			{
				std::ostringstream ss;
				ss << s << DUB_SEPARATOR << i << DUB_TERMINATOR;//duplicate #
				pn = AddPrettyName(rNS, ss.str().c_str(), pObj);
				if (pn)
					break;
			}
		}

	}
	return pn;
}

size_t ModuleInfo_t::FetchStringAt(OFF_t off, MyString &s) const
{
	DataStream_t a(GetDataSource()->pvt(), off);
	std::ostringstream os;
	size_t n(a.fetchString(os));
	s = os.str();
	return n;
}

bool ModuleInfo_t::IsNull(OFF_t off) const
{
	return !(off < GetDataSource()->pvt().size());
}

/*FieldPtr ModuleInfo_t::AddStrucvarField(TypePtr iScope, const char *name, TypePtr iType, AttrIdEnum attr)
{
	Strucvar_t *pStrucvar(iScope->typeStrucvar());
	Bitset_t *pBitset(iScope->typeBitset());
	if (pBitset)//check bitvar
		pStrucvar = iScope->parentField()->owner()->typeStrucvar();
	
	if (!pStrucvar)
		return nullptr;

	assert(name);
	assert(iType);

	ModuleInfo_t MI2(*this, memMgrGlobNS());
	
	FieldPtr pField(pStrucvar->findFieldByName(name));
	if (!pField)
	{
		if (pBitset)//it's a bitvar!
			pField = MI2.AppendBField(iScope);
		else
			pField = MI2.appendStrucvarField(iScope);

		if (name)
			MI2.SetFiel dName2(pField, name);

		pField->set AttributeFromId(attr);
	}
#if(!STRUCVARS_LITE)
	else
	{
		//assert(pField->offset() == ADDR_STRUCVAR);
		//if (pField->_key() != scp.o)
		//	pField->setOffset(-1);//not fixed
		if (pField->type() != iType)
		{
			Array_t *pArray1(pField->type()->typeArray());
			Array_t *pArray2(iType->typeArray());

			if (!pArray1 || !pArray2
				|| pArray1->baseType() != pArray2->baseType())
			{
				PrintError() << "Attempt to redeclare a context dependent field '" << name << "' with different type" << std::endl;
				throw (-1);
			}

			TypePtr iBaseType(pArray2->baseType());
			TypesTracer_t TT(MI2, *iType->ownerTypeMgr());
			TypePtr iArrayX(TT.arrayOf(iBaseType, -1));
			if (iArrayX != pField->type())
			{
				BinaryCleaner_t BC(*this);
				BC.ClearType(pField);
				SetType(pField, iArrayX);
			}
		}
	}
#endif
	return pField;
}*/

ROWID ModuleInfo_t::updateViewGeometry2(TypePtr pSelf, ROWID daBase, OFF_t raBase, size_t offBase) const
{
CHECKID(pSelf, 0x519)
STOP
	Seg_t& rSelf(*pSelf->typeSeg());

	ROWID traceSize(SegTraceSize(pSelf));
	ROWID baseSize(rSelf.subsegs().empty() ? rSelf.size(pSelf) : 0);
	ROWID lowerSize(std::min(traceSize, baseSize));//all subsegs reside withing this portion
	ROWID upperSize(std::max(traceSize, baseSize));//no subsegs  here

	//run through all subsegs to accumulate the oversize
	ROWID daCur(daBase);
	OFF_t raCur(raBase);
	ADDR vaCur(pSelf->base());
	size_t offCur(offBase);
	for (SubSegMapCIt i(rSelf.subsegs().begin()); i != rSelf.subsegs().end(); i++)
	{
		TypePtr pSub(IVALUE(i));
		const Seg_t& rSub(*pSub->typeSeg());
		size_t subTraceSize(SegTraceSize(pSub));
		ADDR subBase(rSub.addressP());
		ADDR subOff(subBase - vaCur);
		if (subOff > 0)
			mrModule.sections().add(daCur, subOff, raCur, (ADDR)offCur, pSelf);
		lowerSize += subOff;//grow up a lower portion (originally 0)
		daCur += subOff;
		raCur += subOff;
		offCur += subOff;
		daCur = updateViewGeometry2(pSub, daCur, raCur, 0);
		lowerSize += subTraceSize;
		raCur += subTraceSize;
		offCur += subTraceSize;
		vaCur = subBase + (ADDR)subTraceSize;
	}

	if (rSelf.hasSubsegs())
	{
		size_t l(size_t(traceSize - lowerSize));
		if (l > 0)
		{
			mrModule.sections().add(daCur, l, raCur, (ADDR)offCur, pSelf);
			daCur += l;
			raCur += l;
			offCur += l;
			lowerSize += l;
		}
	}
	else
	{
		if (lowerSize > 0)
		{
			mrModule.sections().add(daCur, (size_t)lowerSize, raCur, (ADDR)offCur, pSelf);
			daCur += lowerSize;
			raCur += lowerSize;
			offCur += (size_t)lowerSize;
		}
	}
	size_t diffSize(size_t(upperSize - lowerSize));
	if (diffSize > 0)
	{
		OFF_t raDelta(traceSize == upperSize ? diffSize : 0);
		mrModule.sections().add(daCur, diffSize, raDelta > 0 ? raCur : OFF_NULL, (ADDR)offCur, pSelf);
		daCur += diffSize;
	}

	rSelf.setView(daBase, daCur - daBase);
	return rSelf.viewOffs() + rSelf.viewSize0();
}

void ModuleInfo_t::UpdateViewGeometry2() const
{
	mrModule.sections().reset();
	updateViewGeometry2(ModulePtr(), 0, mrModule.rawBlock().m_offs, 0);
}

bool ModuleInfo_t::DumpSegResources(MyStreamBase &ss) const
{
	TypePtr iSeg(FindFrontSeg());
	if (!iSeg)
		return false;

	Seg_t &rSeg(*iSeg->typeSeg());
	assert(FrontEnd(iSeg));

	//if (!checkDataAtVA(iSeg, iSeg->base()))
		//return false;

	//get the binary index
	MyString sBin;
	sBin = ModuleTitle(ModulePtr()) + MODULE_SEP;
	FrontEnd(iSeg)->device(GetDataSource())->dumpResources(ss, sBin);
	return true;
}

void ModuleInfo_t::SetModuleTitle(const MyString &s)
{
//#ifdef WIN32//FolderMap is case-insensitive on Windows
	//mrModule.setTitle(s.lower());
//#else
	mrModule.setTitle(s);
//#endif
}

void ModuleInfo_t::SetDataSource(DataPtr pData) const
{
	assert(!mrModule.dataSourcePtr0());
	mrModule.setDataSource(pData);
	pData->addRef();
	mrModule.rawBlock().m_offs = 0;
	mrModule.rawBlock().m_size = pData->pvt().size();
}

PDATA ModuleInfo_t::GetAuxData(OFF_t &size) const
{
	if (mrModule.dataSourcePtr0())
	{
		const I_AuxData *paux(GetDataSource()->pvt().aux());
		if (paux)
		{
			size = paux->size();
			return paux->data();
		}
	}
	return nullptr;
}

DataPtr ModuleInfo_t::GetDataHost() const
{
	DataPtr p(mrModule.dataSourcePtr0());
	if (!p)
		return nullptr;
	DataPtr pHost(p->dataHost());
	return pHost ? pHost : p;
}

DataPtr ModuleInfo_t::GetDataSource() const
{
	return mrModule.dataSourcePtr0();
}

const I_DataSourceBase &ModuleInfo_t::GetDataSourceRef() const
{
	return mrModule.dataSourceRef();
}

/*FieldPtr ModuleInfo_t::insertStrucvarField(ADDR va, TypePtr iSelf, Name_t *pName, TypePtr pFieldType)
{
	Strucvar_t &rSelf(*iSelf->typeStrucvar());

	FieldMap &m(rSelf.fields());
	ADDR offs(va);

	FieldPtr pField(mrMemMgr.NewField(offs));
	SetType(pField, pFieldType);

	rSelf.insertField(pField);
	pField->setOwnerComplex(iSelf);

	if (pName)
	{
		AssureNamespace(iSelf);
		NamesMgr_t &rNS(NamespaceInitialized(iSelf));
		AddName(rNS, pName, pField);// , mrMemMgr);
	}

	return pField;
}*/


bool ModuleInfo_t::VA2FP(TypePtr pCplx, ADDR a, OFF_t &oRaw) const
{
	for (;;)
	{
		Module_t *pModule(pCplx->typeModule());
		if (pModule)
		{
			assert(pModule == &mrModule);
			if (!pModule->dataSourcePtr0())
				break;;
			if (a < GetDataSource()->size())
			{
				oRaw = a;
				return true;
			}
			//PDATA pData(pModule->rawBlock().data());
			//if (!pData)
				//break;
			//if (a < (ADDR)pModule->rawBlock().rawsize())
				//return &pData[a];
			break;
		}

		Seg_t *pSeg(pCplx->typeSeg());
		if (pSeg)
		{
			//segments must be resolute about raw data
			if (a < pCplx->base())
				break;
			ADDR_RANGE a2(a - pCplx->base());
			if (a2 > (ADDR_RANGE)SegTraceSize(pCplx))
				break;

			TypePtr iTrace(pSeg->traceLink());
			if (iTrace)
			{
				if (ModuleOf(iTrace) != ModulePtr())
					return false;//???
				OFF_t o;
				if (!VA2FP(iTrace, a2, o))
					break;
				oRaw = o;
				return true;
			}
			if (a2 >= (ADDR_RANGE)pCplx->size())
				break;
			FieldPtr pField(pCplx->parentField());
			if (!pField)
				return  false;//.BSS?
			pCplx = pField->OwnerComplex();
			a = a2 + pField->_key();
			continue;
		}

		assert(pCplx->typeStruc());
		FieldPtr pField2(pCplx->parentField());
		if (!pField2)
			break;

		pCplx = pField2->OwnerComplex();
		a += pField2->_key();
	}

	return false;
}

bool ModuleInfo_t::GetRawOffset(CFieldPtr pField, OFF_t &o) const
{
	assert(pField->isGlobal());

	//if (!VA2FP(pField->OwnerComplex(), pField->offset(), o))
	if (!VA2FP(OwnerSeg(pField->owner()), pField->_key(), o))
		return false;
	return true;
}

bool ModuleInfo_t::SetTypeName(NamesMgr_t &rNS, TypePtr pSelf, const char *pcName) const//, MemoryMgr_t &rMemMgr)
{
	if (pcName && !pcName[0])
		pcName = nullptr;

	if (!pcName && !pSelf->name())
		return true;

	if (pcName)
	{
		MyString s(ObjName(pSelf));
		if (s == pcName)
			return true;//nothing to change - already a default or user's name
	}

	/*if (!pNSp)
	{
	assert(!pName);
	return true;
	}*/

	NamesMgr_t *pNSp(&rNS);

	if (!pcName)
	{
		RemoveNameRef(rNS, pSelf->name());
		//if (!pNSp->remo ven(pSelf, mrMemMgr))//the namespace has not been initialized yet?
			pSelf->setName(nullptr);
		return true;
	}

	if (!isalpha(pcName[0]) && pcName[0] != '_')
	{
		PrintWarning() << "Could not rename object " << ObjName(pSelf) << " (illegal identifer's name)." << std::endl;
		return false;
	}

	if (!AddName(*pNSp, pcName, pSelf))//, mrMemMgr))
	{
		PrintWarning() << "Could not rename object " << ObjName(pSelf) << ". Name exists in current namespace." << std::endl;
		return false;
	}

	return true;
}

bool ModuleInfo_t::SetFieldName(NamesMgr_t &rNS, FieldPtr pSelf, const char *pName, bool bForce) const
{
	if (pName && !pName[0])
		pName = nullptr;

	if (!pName && !pSelf->name())
		return true;

	if (pName)
	{
		MyString s(AutoName(pSelf, nullptr));
		if (s == pName)
			return true;//nothing to change - already a default or user's name
	}

	/*if (!pNSp)
	{
	assert(!pName);
	return true;
	}*/

	if (!pName)
	{
		RemoveNameRef(rNS, pSelf->name());
//		if (!pNSp->remo ven(pSelf, mrMemMgr))//the namespace has not been initialized yet?
			pSelf->setName(nullptr);
		return true;
	}

	if (bForce)
	{
		if (ForceName(rNS, pName, pSelf))
			return true;
	}
	else
	{
		if (!IsLegalCName(pName))
		{
			PrintWarning() << "Could not rename object " << FieldName(pSelf) << " (illegal identifer's name)." << std::endl;
			return false;
		}

		if (!AddName(rNS, pName, pSelf, pSelf->isTypeImp()))//, mrMemMgr))
		{
			PrintWarning() << "Could not rename object " << FieldName(pSelf) << ". Name exists in current context." << std::endl;
			return false;
		}
	}

	return true;
}

MyString ModuleInfo_t::FieldNameDemagled(CFieldPtr pSelf) const
{
	if (pSelf->name())
	{
		MyStringEx aNamex(ToCompoundName(pSelf->name()));
		if (!aNamex.empty(0))
		{
			TypePtr iSeg(OwnerSeg(pSelf->owner()));
			if (iSeg)
			{
				if ((iSeg = FrontSeg(iSeg)) != nullptr)
				{
					FRONT_t* pFRONT(FrontOf(iSeg));
					if (pFRONT)//try to demangle
					{
						MyString s;
						ProjectInfo_t::ChopName(aNamex[0].c_str(), s);
						MyStream ss;
						pFRONT->device(GetDataSource())->demangleName(s, ss);
						if (ss.ReadString(s))
							return s;
					}
				}
			}
		}
	}
	return MyString();
}

TypePtr ModuleInfo_t::NewRangeSet(ADDR64 base64, const char *name)
{
	TypePtr iRangeSeg(memMgr().NewTypeRef(new Seg_t));
	Seg_t &rRangeSeg(*iRangeSeg->typeSeg());
	rRangeSeg.setBase64(base64);
	rRangeSeg.setTitle(name ? name : "$RANGESET");
	mrModule.addSubRange(iRangeSeg);
	rRangeSeg.setRangeSet(true);
	return iRangeSeg;
}

template <typename T>
MyString hexToString(T n, unsigned w)
{
	char buf[32];
	PrintHex((unsigned)n, w, buf);
	return buf;
}

void ModuleInfo_t::DumpSegments(CTypePtr iSelf, std::ostream &os, int level)
{
	Seg_t &rSelf(*iSelf->typeSeg());

	char buf[32];

	unsigned w(HexWidthMax(mrModule.viewSize0()));
	unsigned wr(HexWidthMax(mrModule.size(ModulePtr())));

	MyString s;
	for (int i(0); i < level; i++)
		s.append("\t");
	s.append(MyStringf("VA=%s(%08X)", rSelf.printVA(iSelf, rSelf.base(iSelf), buf, 8), iSelf->size()));
	if (rSelf.traceLink())
		s.append(MyStringf(", RAW=.%s(%s)", hexToString(rSelf.rawBlock().lower(), wr).c_str(), hexToString(rSelf.rawBlock().size(), wr).c_str()));
//#ifndef _DEBUG
	s.append(MyStringf(", DA=~%s(%s)", hexToString(rSelf.viewOffs(), w).c_str(), hexToString(rSelf.viewSize0(), w).c_str()));
//#endif
	if (!s.empty())
		s.append(", ");
	s.append(rSelf.title().empty() ? MyString("<noname>"): rSelf.title());
	if (iSelf->parentField())
		s.append(MyStringf(" (%s)", mrModule.rangeSetName(iSelf->parentField()->owner()).c_str()));
	os << s << std::endl;

	for (SubSegMapIt i(rSelf.subsegs().begin()); i != rSelf.subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));
		DumpSegments(iSeg, os, level + 1);
	}
}

void ModuleInfo_t::DumpSections(std::ostream& os)
{
	os << " *** Sections Map (" << ModuleName() << ")" << std::endl;
	unsigned w(HexWidthMax(mrModule.viewSize0()));
	for (size_t i(0); i < mrModule.sections().size(); i++)
	{
		const Section_t& aSect(mrModule.sections().get(i));
		const Seg_t& rSeg(*aSect.seg->typeSeg());
		MyString s;
		s = MyStringf("DA=~%s(%s)", hexToString(aSect.da, w).c_str(), hexToString(aSect.size, w).c_str());
		//if (aSect.off > 0)
			//s.append(MyStringf("+%X", (unsigned)aSect.off));
		if (aSect.ra != OFF_NULL)
			s.append(MyStringf(", RAW=.%X", (unsigned)aSect.ra));
		s.append(MyStringf(" (%s)", rSeg.title().c_str()));
		os << "\t" << "[" << i << "] " << s << std::endl;
	}
}

FieldPtr ModuleInfo_t::FindFieldInSeg(CTypePtr iSeg, ADDR va, Locus_t &aLoc) const
{
	if (iSeg)
	{
		assert(!IsRangeSeg(iSeg));
		const Seg_t &rSeg(*iSeg->typeSeg());
		aLoc.push_back(Frame_t(rSeg.rawBlock(), (TypePtr)iSeg, va, nullptr));
		terminalFieldAt(aLoc);
		if (!aLoc.empty())
			return aLoc.back().field();
	}
	return nullptr;
}

//given a starting seg (or segrange) and va, get the seg va falls into
TypePtr ModuleInfo_t::VA2Seg(CTypePtr iSegRange, ADDR va) const
{
	CTypePtr iSeg(nullptr);
	if (!IsRangeSeg(iSegRange) && iSegRange->parentField())
		iSegRange = iSegRange->parentField()->owner();
	else
		iSeg = iSegRange;

	if (!iSeg)
	{
		const Seg_t &rSegRange(*iSegRange->typeSeg());
		for (FieldMapCRIt I = rSegRange.fields().rbegin(), E = rSegRange.fields().rend(); I != E; ++I)//look from the back to bypass a superseg
		{
			CFieldPtr pField(VALUE(I));
			if (pField->type())//maybe a seg end
			{
				CTypePtr iSeg2(pField->type());
				assert(iSeg2->typeSeg());
				if (IsInside(iSeg2, va))
					return (TypePtr)iSeg2;
			}
		}
	}
	return (TypePtr)iSeg;
}

FieldPtr ModuleInfo_s::FindFieldInSubsegs(CTypePtr iSelf, ADDR va, Locus_t &aLoc)
{
	Seg_t &rSeg0(*iSelf->typeSeg());
	if (rSeg0.frontIndex() && rSeg0.superLink()->typeModule() && IsPhantomModule(rSeg0.superLink()))
		return Field(iSelf, va);//phantom binary?

	TypePtr iSeg(rSeg0.findSubseg(va, rSeg0.affinity()));
	if (iSeg)
	{
		Seg_t &rSeg(*iSeg->typeSeg());
		aLoc.push_back(Frame_t(rSeg.rawBlock(), iSeg, va, nullptr));
		terminalFieldAt(aLoc);

		aLoc.wrapUp();
		if (!aLoc.empty())
			return aLoc.back().field();
	}
	return nullptr;
}

FieldPtr ModuleInfo_s::FindFieldInSubsegs(CTypePtr iSeg0, ADDR addr, unsigned *pshift)
{
	CTypePtr iSegTop(OwnerSegList(iSeg0));
	if (!iSegTop)
		iSegTop = iSeg0;

	CTypePtr iSeg(iSegTop->typeSeg()->findSubseg(addr, iSegTop->typeSeg()->affinity()));
	if (iSeg)
	{
		const Seg_t &rSeg(*iSeg->typeSeg());
		return __findFieldV(iSeg, addr, FieldIt_Overlap, false);
	}
	return nullptr;

	//ieldPtr  pLoc(ProjectInfo_t::__findFieldV(pType, addr0, ProjectInfo_t::FieldIt_Exact, true));

	Locus_t loc;
	FieldPtr f(FindFieldInSubsegs(iSegTop, addr, loc));
	if (!f)
		return nullptr;
	loc.stripToSeg();
	if (pshift)
		*pshift = loc.offs();
	return f;
}

FieldPtr ModuleInfo_s::GetFieldFromValue(const VALUE_t& v, CTypePtr pSeg)
{
	ADDR va;
	if (GetVAfromValue(v, pSeg, va))
		return FindFieldInSubsegs(pSeg, va);
	return nullptr;
}

bool ModuleInfo_s::GetVAfromValue(const VALUE_t& v, CTypePtr pSeg, ADDR& va)
{
	if (v.opsiz == OPSZ_DWORD)
	{
		va = v.ui32;
		return true;
	}

	if (v.opsiz == OPSZ_QWORD)
	{
		ADDR64 imageBase(pSeg->typeSeg()->imageBase(pSeg));
		if (v.ui64 >= imageBase)//can be a wrong pointer
		{
			ADDR64 d64(v.ui64 - imageBase);
			if (d64 < 0x100000000)
			{
				va = ADDR(d64);
				return true;
			}
		}
	}
	return false;
}

bool ModuleInfo_t::Str2DA(MyString s0, ROWID *pr, CTypePtr pStartSeg)
{
	MyString s(s0.simplifyWhiteSpace());

	if (s.empty())
		return false;

	if (s.startsWith("~"))
	{
		s.remove(0, 1);
		ROWID r(strtoul(s.c_str(), nullptr, 16));
		if (pr)
			*pr = r;
		return true;
	}

	if (s.left(1) == ".")//raw offset?
	{
		s.remove(0, 1);
		ADDR addr(strtoul(s.c_str(), nullptr, 16));
		ROWID r;//(addr);
		if (!R2D(ModulePtr(), addr, r))
			return false;
		if (pr)
			*pr = r;
	}
	else
	{
		//virtual address
		value_t v;
		if (mrProject.checkAutoPrefix(s.c_str(), &v) == 0)
			v.ui64 = strtoull(s.c_str(), nullptr, 16);
		if (!pStartSeg)
			pStartSeg = ModulePtr();
		const Seg_t& rSeg(*pStartSeg->typeSeg());
		v.ui64 -= rSeg.imageBase(pStartSeg);
		assert(v.ui64h == 0);
		TypePtr pSeg2(FindSegAt(pStartSeg, v.ui32, rSeg.affinity()));
		if (!pSeg2)
			return false;
		if (pr)
			*pr = pSeg2->typeSeg()->viewOffsAt(pSeg2, v.ui32);
	}
	return true;
}

TypePtr ModuleInfo_t::FindSegAt(CTypePtr iSelf, ADDR va, unsigned affinity)
{
	const Seg_t &rSelf(*iSelf->typeSeg());
	if (rSelf.affinity() != affinity)
		return nullptr;

	for (SubSegMapCIt i(rSelf.subsegs().begin()); i != rSelf.subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));
		TypePtr pSeg2(FindSegAt(iSeg, va, affinity));
		if (pSeg2)
			return pSeg2;
	}
	ADDR aBase(iSelf->base());
	if (aBase <= va && va < aBase + iSelf->size())
		return (TypePtr)iSelf;
	return nullptr;
}

bool ModuleInfo_t::R2D(CTypePtr iSelf, ADDR addr, ROWID &rowID)
{
	Seg_t &rSelf(*iSelf->typeSeg());

	for (SubSegMapCIt i(rSelf.subsegs().begin()); i != rSelf.subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));
		if (R2D(iSeg, addr, rowID))
			return true;
	}

	ADDR lower((ADDR)rawOffs(iSelf));
	if (lower <= addr && addr < lower + rawSize(iSelf))
	{
		rowID = rSelf.viewOffs() + (addr - lower);
		return true;
	}

	ROWID over(SegOverSize0(iSelf));
	if (over > 0)//add extra only if raw size > seg size
		rowID += over;
	return false;
}

TypePtr ModuleInfo_t::R2D2(CTypePtr iSelf, ADDR addr, ROWID &da0)//this one is better(!)
{
	const Seg_t &rSelf(*iSelf->typeSeg());

	ADDR a0(iSelf->base());

	for (SubSegMapCIt i(rSelf.subsegs().begin()); i != rSelf.subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));
		const Seg_t &rSeg(*iSeg->typeSeg());
		ADDR a1(rSeg.addressP());
		assert(a1 >= a0);
		unsigned segOffs(a1 - a0);
		if (segOffs > addr)
		{
			da0 += addr;
			return iSeg;
		}

		da0 += segOffs;
		iSeg = R2D2(iSeg, addr - segOffs, da0);
		if (iSeg)
			return iSeg;
		da0 -= segOffs;
	}

	if (addr < rawSize(iSelf))
	{
		da0 += addr;
		return (TypePtr)iSelf;
	}

	ROWID over(SegOverSize0(iSelf));
	if (over > 0)//add extra only if seg size > raw size
		da0 += over;
	return nullptr;
}

TypePtr ModuleInfo_t::SegTraceOffsetToVA(CTypePtr iSelf, unsigned offs, ADDR& va)
{
	return 0;
}

TypePtr ModuleInfo_t::FindSegFromRA(OFF_t ra, CTypePtr iSelf)
{
	assert(iSelf);
	Seg_t &rSelf(*iSelf->typeSeg());

	for (SubSegMapCIt i(rSelf.subsegs().begin()); i != rSelf.subsegs().end(); ++i)
	{
		TypePtr iSeg2(FindSegFromRA(ra, IVALUE(i)));
		if (iSeg2)
			return iSeg2;
	}

	OFF_t lower(rawOffs(iSelf));
	if (lower <= ra && ra < lower + rawSize(iSelf))
		return (TypePtr)iSelf;

	return nullptr;
}


TypePtr ModuleInfo_t::GetStockType(OpType_t optyp, TypesMgr_t **pptm) const
{
	TypesMgr_t *ptm(mrProject.typeMgr());
	if (!ptm)
		ptm = ModulePtr()->typeMgr();
	if (pptm)
		*pptm = ptm;
	return ptm->getStockType(optyp);
}


TypePtr ModuleInfo_t::FindEntryPoint(ADDR &va) const
{
	CTypePtr iFrontSeg(FindFrontSeg());
	if (!iFrontSeg)
		return nullptr;
	FRONT_t *pFE(FrontEnd(iFrontSeg));
	if (!pFE->device(GetDataSource())->getEntryVA(va))
		return nullptr;
	return FindSegAt(iFrontSeg, va, iFrontSeg->typeSeg()->affinity());
}

////////////////////////////////////////////////

bool ModuleInfo_t::SetFieldName2(FieldPtr pField, const MyStringEx &aName, bool bForce) const
{
	assert(aName);
	MyString sName(FromCompoundName(aName));
	if (sName.empty())
		return false;
	
	TypePtr iSelf(pField->owner());
	Struc_t &rSelf(*iSelf->typeStruc());
	if (iSelf->typeStrucvar())// || iSelf->typeBitset())
	{assert(0);//?
		AssureNamespace(iSelf);
		//NamesMgr_t &rNS(NamespaceInitialized(iSelf));
		NamesMgr_t &rNS(*iSelf->typeComplex()->namesMgr());
		if (!AddName(rNS, sName, pField))
			return false;
		return true;
	}

	NamesMgr_t *pNs(rSelf.namesMgr());
	if (!pNs)
	{
		TypePtr pNsOwner;
		pNs = OwnerNamesMgr(iSelf, &pNsOwner);
		if (!pNs)
		{
			pNs = AssureNamespace(iSelf);
			pNsOwner = iSelf;
		}
		//pNs = &NamespaceInitialized(pNsOwner);
		pNs = pNsOwner->typeComplex()->namesMgr();
	}
	//else
		//NamespaceInitialized(iSelf);

	if (!ForceName(*pNs, sName, pField, bForce ? 0 : -1))
	{
		PrintWarning() << "A name already exists in current namespace: " << sName << std::endl;
		return false;
	}
	
	/*static int z(0);
	if (z)
		pNs->check("?DoBuild@CImgDialog@@MAEXHPAVCImage@@@Z");*/
	return true;
}

bool ModuleInfo_t::CheckUglyName(FieldPtr pField, MyString demangled) const
{
	MyString mangled(pField->name()->c_str());
	if (!demangled.isEmpty() && demangled != mangled)
	{
		pField->setUglyName();
		return true;
	}
	return false;
}

bool ModuleInfo_t::SetHardName2(FieldPtr pField, MyString mangled) const
{
	if (mangled.isEmpty())
		return false;
//CHECKID(pField, 0xa8d)
//STOP
	if (!SetFieldName2(pField, mangled, true))
		return false;

	pField->setHardNamed(true);
	return true;
}

MyString ModuleInfo_t::FormatterAtLocus(Probe_t &addrx)
{
	MyString typeKey;

	for (FrontMapCIt i(mrMain.frontends().begin()); i != mrMain.frontends().end(); i++)
	{
		FrontInfo_t& fi(*i->second);
		I_FrontMain* pIFrontMain(fi.load(mrMain));
		if (pIFrontMain)
		{
			FrontImplEx_t fimp(mrProject, addrx.locus(), fi, ModulePtr());//overkill!
			FrontIfaceEx_t iface(fimp);
			MyString sExt;
			const MyString& sTitle(ModuleTitle(ModulePtr()));
			size_t n(sTitle.rfind('.'));
			if (n != std::string::npos)
				sExt = sTitle.right(unsigned(sTitle.length() - (n + 1)));
			const char* pc(pIFrontMain->RecognizeFormat(iface, sExt.c_str()));//?????
			if (pc)
				typeKey = pc;
			fi.unload(mrMain, true);//if the object was created, dll may not be really unloaded
			if (!typeKey.empty())
				break;
		}
	}
	return typeKey;
}

MyString ModuleInfo_t::FieldDisplayName(FieldPtr pField) const
{
	return FieldName(pField);
}

FieldPtr ModuleInfo_t::FindGlobalByName(const char *name) const
{
	return mrProject.findGlobal(name, ModulePtr());
}

bool ModuleInfo_t::IsReadOnlyVA(CTypePtr pRefSeg, ADDR va) const
{
	CTypePtr iSeg(VA2Seg(pRefSeg, va));
	if (iSeg && iSeg->typeSeg()->isReadOnly())
		return true;//referencing an import table?
	return false;
}

MyString ModuleInfo_t::FolderPath(CFolderPtr pFolder) const
{
	return FolderPathRel(pFolder, mrModule.folderPtr());
}

///////////////////////////////////////////

/*TypePtr ModuleInfo_t::findType(TypePtr iStart, OpType_t typeID, TypesMgr_t **ppTypesMgr) const
{
	if (mrProject.typeMgr())
	{
		TypePtr iType(mrProject.getStockType(typeID));
		if (iType)
		{
			if (ppTypesMgr)
				*ppTypesMgr = mrProject.typeMgr();
		}
		return iType;
	}

	const TypeObj_t *pTypeRef(iStart);
	while (pTypeRef)
	{
		TypesMgr_t *pTypesMgr(pTypeRef->typeMgr());
		if (pTypesMgr)
		{
			TypePtr iType(pTypesMgr->getStockType(typeID));
			if (iType)
			{
				if (ppTypesMgr)
					*ppTypesMgr = pTypesMgr;
				return iType;
			}
		}
		if (pTypeRef->owner())//shared?
			pTypeRef = pTypeRef->owner();
		else if (pTypeRef->typeSeg())
			pTypeRef = pTypeRef->typeSeg()->superLink();
		else if (pTypeRef->mpParentField)
			pTypeRef = pTypeRef->mpParentField->owner();
		else
			break;
	}
	return nullptr;
}*/

TypePtr ModuleInfo_t::findType(CTypePtr iStart, const char* name, TypesMgr_t **ppTypesMgr) const
{
	CTypePtr pTypeRef(iStart);
	while (pTypeRef)
	{
		TypesMgr_t *pTypesMgr(pTypeRef->typeMgr());
		if (pTypesMgr)
		{
			TypesTracer_t TT(*this, *pTypesMgr);
			TypePtr iType(TT.findTypeByName(name));
			if (iType)
			{
				if (iType->typeProxy())
					return iType->typeProxy()->incumbent();
				return iType;
			}
		}
		if (pTypeRef->owner())//shared?
			pTypeRef = pTypeRef->owner();
		else if (pTypeRef->typeModule())
			pTypeRef = mrProject.self();
		else if (pTypeRef->typeSeg())
			pTypeRef = pTypeRef->typeSeg()->traceLink();
		else if (pTypeRef->parentField())
			pTypeRef = pTypeRef->parentField()->owner();
		else
			break;
	}

	return nullptr;
}


static bool __dumpResCallback(size_t level, const char *name, ADDR va, unsigned, void *pvUser)
{
	FileRes_t::tree_builder_t *tb((FileRes_t::tree_builder_t *)pvUser);
	FileRes_t::tree_node_t *node(new FileRes_t::tree_node_t);
	node->name = name;
	node->va = va;
	MyString &s(node->name);
	if (s.endsWith(MODULE_SEP))
	{
		if (s.endsWith(".exe" MODULE_SEP))
			node->eType = adcui::FOLDERTYPE_BINARY_EXE;
		else
			node->eType = adcui::FOLDERTYPE_BINARY_DLL;
		s.chop(1);
	}
	else if (va == 0)//s.endsWith("/"))
	{
		node->eType = adcui::FOLDERTYPE_FOLDER;
	}
	else
		node->eType = adcui::FOLDERTYPE_FILE_RC;
	tb->add(level, node);
	return true;//continue;
}

bool ModuleInfo_t::CheckResources()
{
	TypePtr iSeg(FindFrontSeg());
	if (!iSeg)
		return false;

	FileRes_t *pFileRes(new FileRes_t);

	FileRes_t::tree_builder_t tb(pFileRes->tree());

//	for (FileTree_t::ChildrenIterator it(Project().files().rootFolder()); it; it++)
//	{
//		const Folder_t &rFolder(*it.data());
//		if (IsPhantomFolder(rFolder))
//			continue;

	Seg_t &rSeg(*iSeg->typeSeg());
	assert(FrontEnd(iSeg));

	//get the binary index
	MyString sBin(ModuleTitle(ModulePtr()) + MODULE_SEP);
	FrontEnd(iSeg)->device(GetDataSource())->dumpResources2(__dumpResCallback, &tb, sBin);

	if (pFileRes->tree().empty())
	{
		delete pFileRes;
		return false;
	}

	//disguise the only root item
	pFileRes->tree().kill_root();

	FolderPtr pFolder(AssureFolderOfKind(FTYP_RESOURCES));
	if (pFolder->hasPvt())
		pFolder->ClearPvt();

	pFolder->SetPvt(pFileRes);
	return true;

	//m_tree.print(std::cout, m_tree.root(), -1);
}

bool ModuleInfo_t::DeleteField(TypePtr iSelf, ADDR offs) const
{
	Struc_t &rSelf(*iSelf->typeStruc());

	FieldMapIt it = FieldIt(iSelf, offs /*- pSelf->base()*/, nullptr, FieldIt_Overlap);
	if (it == rSelf.fields().end())
		return false;

	FieldPtr pField(VALUE(it));

	ClearFieldName(pField, rSelf.namesMgr());

	if (SetType(pField, nullptr) == nullptr)
	{
		rSelf.takeField0(it);
		mrMemMgr.Delete(pField);
	}

	return true;
}

PNameRef ModuleInfo_t::addNamedObj(TypePtr pSelf, Obj_t *pObj, const char *pName, NamesMgr_t **ppNS, int iForceName) const
{
	NamesMgr_t &rNS(OwnerNamespaceEx(pSelf));
	PNameRef pNameRef(ForceName(rNS, pName, pObj, iForceName));
	if (!pNameRef)
		fprintf(STDERR, "Warning: Could not use name: '%s'\n", pName);
	if (ppNS)
		*ppNS = &rNS;
	return pNameRef;
}

FieldPtr ProjectInfo_s::UFieldPrev(CFieldPtr p)
{
	const Struc_t& rOwner(*p->owner()->typeStruc());
	FieldMapCIt it(p);
	if (it != rOwner.fields().begin())
	{
		--it;
		if (KEY(it) == p->_key())
			return (FieldPtr)VALUE(it);
	}
	return nullptr;
}

FieldPtr ProjectInfo_s::UFieldNext(CFieldPtr p)
{
	const Struc_t& rOwner(*p->owner()->typeStruc());
	FieldMapCIt it(p);
	if (it != rOwner.fields().end())
	{
		++it;
		if (KEY(it) == p->_key())
			return (FieldPtr)VALUE(it);
	}
	return nullptr;
}

bool ModuleInfo_t::DeleteField(Locus_t &aLoc) const
{
	Frame_t &aTop(aLoc.back());
	FieldPtr pField(aTop.field());
	
	bool bWasFunc(pField->isTypeProc() != nullptr);
	FieldPtr pNext(bWasFunc ? UFieldNext(pField): nullptr);//locus (and func type) successor (do not consider codes and thunks)
	assert(!pNext || !pNext->type());

	TypePtr pType(nullptr);
	if (pNext)
	{
		pType = pField->type();//this type will be re-set
		if (pType)
			pType->addRef();//avoid type be deleted
	}
	else
		pNext = UFieldPrev(pField);

	TakeField0(aTop.cont(), pField);
	memMgr().Delete(pField);

	if (pType)//re-set it on sub field
	{
		SetType(pNext, pType);
		pType->releaseRef();//back
	}

	aTop.setField(pNext);//whatever

	Bitset_t *pBitset(aTop.cont()->typeBitset());
	if (pBitset && !pBitset->hasFields())
	{
		aLoc.pop_back();
		return DeleteField(aLoc);
	}

	mrProject.markDirty(DIRTY_GLOBALS);
	if (bWasFunc || pNext)
		mrProject.markDirty(DIRTY_LOCUS_ADJUSTED);
	return true;
}

/*bool ModuleInfo_t::DeleteGlobal(TypePtr iSelf, FieldPtr pField)
{
	if (!DeleteField(iSelf, pField))
		return false;
	mrProject.markDirty(DIRTY_GLOBALS);
	return true;
}*/


static bool isSegTrace(CTypePtr p)
{
	if (!p->isShared())//a trace?
	{
		if (p->objType())//no a glob
		{
			CFieldPtr pOwnerField(p->parentField());
			CTypePtr pOwnerSeg(pOwnerField->owner());
			if (pOwnerSeg && pOwnerSeg->typeSeg())
			{
				const Seg_t* pSegPvt(pOwnerSeg->typeSeg());
				for (SubSegMapCIt i(pSegPvt->subsegs().begin()); i != pSegPvt->subsegs().end(); ++i)
				{
					TypePtr pSubSeg(IVALUE(i));
					if (pSubSeg->typeSeg()->traceLink() == p)
						return true;
				}
			}
		}
	}
	return false;
}


bool ModuleInfo_t::AddSubRange0(TypePtr iRangeSet, ADDR base, unsigned size, TypePtr pSeg)
{
	//assert this is a range seg!
CHECKID(pSeg, 0x125b)
STOP
CHECKID(pSeg, 0x1261)
STOP

	Seg_t *pRangeSeg(iRangeSet->typeSeg());
	assert(pRangeSeg);

	FieldMap &m(pRangeSeg->fields());

//	TypePtr pRangeSet(mRangeMgr.getSlot(hRangeSet));
	//if (!pRangeSet)
		//return nullptr;


	//RangeElt_t elt(hRangeSet, base);

	RangeIt it(m.upper_bound(base));
	if (it != m.begin())
	{
		it = m.Prior(it);
		/*FieldPtr pField(VALUE(it));
		if (pField->type())
		{
			int sz(pField->type()->size());
			if (sz != -1)//not limitless
				return nullptr;//can't wedge into an active range
		}*/
		if (KEY(it) != base)
			it = m.end();
	}
	else
		it = m.end();

	//lower boundary
//	it = m.find(base);
	//if (it != m.end())
		//if (VALUE(it)->type() != nullptr)//the range is taken?
			//return false;

	FieldPtr pField;
	if (it == m.end())
	{
		pField = memMgr().NewField(base);
		it = m.insert_unique(pField).first;
		assert(it != m.end());
		pField->setOwnerComplex(iRangeSet);
	}
	else
	{
		pField = VALUE(it);
		if (pField->type())
			return false;
	}

	assert(!pSeg->parentField());
	SetType(pField, pSeg);
	assert(pSeg->parentField());

	if (size != -1)
	{
		ADDR upper(base + size);
		RangeIt it2(m.find(upper));
		if (it2 == m.end())
		{
			it2 = it;
			if (++it2 == m.end() || upper < KEY(it2))//appending a subrange or inserting without overlap
			{
				FieldPtr pField2(memMgr().NewField(upper));
				it2 = m.insert_unique(pField2).first;
				assert(it2 != m.end());
				pField2->setOwnerComplex(iRangeSet);
				//pRangeSet->insert(RangeElt_t(hRangeSet, base + size));//upper boundary - this may fail - it's OK
			}
			else
			{
				STOP
			}
		}
	}

#if(0)
	//check if the previous range overlaps the new one
	if (ret.first != begin())
	{
		iterator ip(ret.first);
		ip--;
		if (ip->base + ip->iSeg->size() > ret.first->base)
		{
			erase(ret.first);
			return false;
		}
	}
	//check if this one overlaps the next
	iterator nx(ret.first);
	nx++;
	if (nx != end())
	{
		if (ret.first->base + ret.first->iSeg->size() > nx->base)
		{
			erase(ret.first);
			return false;
		}
	}
#endif
	//const RangeElt_t &r(*ret.first);
	//return (TypePtr)&r;
	return true;
}

TypePtr ModuleInfo_t::AddSubRange(TypePtr iRangeSet, ADDR addrV, ADDR_RANGE sizeV, TypePtr iSeg)
{
	if (iRangeSet && iSeg)
	{
		if (!AddSubRange0(iRangeSet, addrV, sizeV, iSeg))
		{
			fprintf(STDERR, "Warning: Could not join segment range '%s' with %s\n", mrModule.rangeSetName(iRangeSet).c_str(), iSeg->typeSeg()->title().c_str());
			TypePtr iRangeSet2(NewRangeSet(iRangeSet->typeSeg()->imageBase(iRangeSet), nullptr));
			AddSubRange(iRangeSet2, addrV, sizeV, iSeg);
//?			memMgr().Delete(iSeg);//a tace???
		}
			
//?		if (!pSeg->range())//from phantom seg?
	//?		pSeg->setRange(pRange);
		//?pRange->setSeg(iSeg);
	}
	return iSeg;
}

TypePtr ModuleInfo_t::MakeSegment(TypePtr iSuperSeg, ADDR addrP, unsigned sizeP, const char *name, unsigned uFlags)
{
	Seg_t *pSegMain(iSuperSeg->typeSeg());
	if (!pSegMain)
	{
		PrintError() << "A new segment can only be created in the context of another segment" << std::endl;
		return nullptr;
	}

	ADDR segOffs(addrP - iSuperSeg->base());//oofset in owner seg
	ADDR fileOffs((ADDR)pSegMain->rawBlock().m_offs + segOffs);
	if (!(segOffs < pSegMain->rawBlock().m_size) && sizeP > 0)
	{
		PrintError() << "Invalid segment raw offset: " << VA2STR(addrP) << std::endl;
		return nullptr;
	}

	if (sizeP != -1 && segOffs + sizeP > pSegMain->rawBlock().m_size)
	{
		unsigned rawSize0(sizeP);
		sizeP = (ADDR)pSegMain->rawBlock().m_size - segOffs;
		PrintWarning() << "Ivalid raw size (" << VA2STR(rawSize0) << ") for segment " << name << ", trimmed to " << VA2STR(sizeP) << std::endl;
		//return nullptr;
	}

	TypePtr iSeg(nullptr);
	TypePtr iTrace(nullptr);

	//First check if a trace can be harbored
	if (sizeP > 0)
	{
		//add a trace in a super seg
		iTrace = memMgr().NewTypeRef(new Struc_t);

		Locus_t aLoc;
		aLoc.push_back(Frame_t(iSuperSeg, addrP, nullptr));
		if (!InsertField(aLoc, iTrace->size()) || !SetTypeEx(aLoc.field0(), iTrace))
		{
			BinaryCleaner_t<> BC(*this);
			BC.DestroyTypeRef(iTrace);

			fprintf(STDERR, "Warning: Failed to harbour a segment trace: %s\n", name);
			return nullptr;
		}

		Struc_t *pTrace(iTrace->typeStruc());
		if (sizeP != -1)
			SetStrucSize(iTrace, sizeP);
	}

	//check if there is already a phantom segment
	iSeg = mrProject.OnNewPortal(iSuperSeg);//, *this);
	if (!iSeg)
		iSeg = memMgr().NewTypeRef(new Seg_t);

	Seg_t *pSeg(iSeg->typeSeg());

	if (iTrace)
		pSeg->setTraceLink(iTrace);

	if (name)
		pSeg->setTitle(MyStringf("%s", name));

//	pSeg->setImageBase(imageBase);
//	pSeg->setBase(addrBaseV);
//	if (segSizeV != -1)
//		SetStrucSize(iSeg, segSizeV);

	pSeg->rawBlock().set(fileOffs, sizeP);//raw offset may not be 0 when raw size is
	//if (sizeP > 0)
		//pSeg->rawBlock().setData(pSegMain->rawBlock().ptr + segOffs);//mrModule.rawBlock().data()

	if (!RegisterSubseg(iSuperSeg, addrP/*iSeg->base()*/, iSeg))
		ASSERT0;

	pSeg->setUFlags(uFlags);

//	SegJustCreated(iSeg);

	mrProject.markDirty(DIRTY_GLOBALS);
	return iSeg;
}

////////////////////////////////////////////////////// ScopeStack_t

frame_t &ScopeStack_t::push_scope(TypePtr p, ADDR o, bool bAdvOnLeave)
{
	TypePtr pProj(nullptr);
	frame_t *fp(nullptr);
	if (empty())
	{
		pProj = p;// ->typeProject();
		//?assert(pProj);
	}
	else
		fp = &back();
	push_back(frame2_t(p, o, bAdvOnLeave));
	frame_t &f(back());

	if (pProj && pProj->typeModule())
	{
		//f.r.ptr = pProj->typeModule()->rawBlock().data();
		f.m_offs = 0;
		f.m_size = mrPJ.rawSize(pProj);
	}
	else
	{
		if (isTopSeg())
		{
			TypePtr iSeg(back().cont());
			OFF_t oRaw;
			if (mrPJ.VA2FP(back().cont(), iSeg->base(), oRaw))
			{
				//f.r.ptr = mrPJ.dataSource()->data() + oRaw;
				//if (f.r.ptr)
				{
					//ADDR d(o - pSeg->base());
					f.m_offs = mrPJ.rawOffs(iSeg);// + d;
					f.m_size = mrPJ.rawSize(iSeg);// - d;
				}
			}
		}
		else if (fp)
		{
			if (isSegTrace(p))
			{
				TypePtr pOwnerSeg(p->parentField()->owner());
				f.m_offs = mrPJ.rawOffs(pOwnerSeg);
				f.m_size = mrPJ.rawSize(pOwnerSeg);
				return f;
			}

			if (p->typeStrucvar() || fp->cont() != f.cont())//?
			{
				ADDR o(fp->m_addr - fp->cont()->base());
				if ((int)o < fp->m_size)
				{
					//f.r.ptr = fp->r.ptr + o;
					f.m_offs = fp->m_offs + o;
					f.m_size = fp->m_size - o;
				}
			}
			else
			{
				f.m_offs = fp->m_offs;
				f.m_size = fp->m_size;
			}
		}
	}
	//assert(f.r.ptr);
	return f;
}

frame_t &ScopeStack_t::jump_scope(TypePtr p, ADDR o, bool bAdvOnLeave)
{
	TypePtr pProj(nullptr);
	frame_t *fp(nullptr);
	if (empty())
		pProj = p;
	else
		fp = &back();
	push_back(frame2_t(p, o, bAdvOnLeave));
	frame_t &f(back());

	OFF_t oRaw;
	if (mrPJ.VA2FP(f.cont(), f.cont()->base(), oRaw))
	{
		f.m_offs = oRaw;
		f.m_size = f.cont()->size();
	}
	return f;
}

bool ScopeStack_t::isTopShared() const
{
	std::list<frame2_t>::const_reverse_iterator ri(rbegin());
	if (!ri->cont()->isShared())
		return false;
	//still may be non-shared for nested strucvars
	TypePtr p(ri->cont());
	if (p->typeStrucvar())
	{
		assert(ri != rend());
		ri++;
		TypePtr p2(ri->cont());
		if (p2->typeStrucvar())
		{
			if (p == p2)
				return false;
		}
	}
	return true;
}

bool ScopeStack_t::isTopStrucvar() const
{
	TypePtr p(back().cont());
	if (p->typeStrucvar())
		return true;
	if (p->typeBitset())
	{
		p = p->parentField()->owner();
		if (p->typeStrucvar())
			return true;
	}
	return false;
}

ADDR ScopeStack_t::pop_scope(bool bForceNoAdvance)
{
	assert(!empty());
	ADDR d(0);
	const frame2_t &f(back());
	TypePtr p(f.cont());
	if (p->typeSeg())
	{
		if (f.bAdvance)
			d += (ADDR)ProjectInfo_t::SegTraceSize(p);
	}
	//else if (!p->isShared())
	else if (!isTopShared())
	{
		if (!bForceNoAdvance && f.bAdvance)
		{
			if (p->typeBitset())
			{
				if (!p->parentField()->owner()->typeStrucvar())
					d = p->sizeBytes();// f.o - p->base());
				else
					d = f.uExtent;
			}
			else
				d = p->sizeBytes();
			if (d == -1)
				d = f.offs();
		}
	}
	else if (p->typeStrucvar())
	{
		assert(f.bAdvance);
		d = f.offs();
	}
	assert(d != -1);
	pop_back();
	if (!empty())
		back().m_addr += d;//advance caller frame
	return d;
}










////////////////////////////////////////////////// StockTracer_t

TypePtr StockTracer_t::addStockType(OpType_t t)
{
	TypePtr pType;
	if (OPTYP_IS_PTR(t))
	{
		assert(0);
		//?pType = mrMemMgr.NewTypeRef(new TypePtr_t());
	}
/*	else if (t == OPTYP_BIT)
		pType = mrMemMgr.NewTypeRef(new Bit_t());*/
	else
		pType = mrMemMgr.NewTypeRef(new Simple_t(t));
	mrTypesMgr.addNameAlias(pType);
	//pType->setOwner(mrTypesMgr.owner());

	//?pType->addRef();
	
/*	if (pType->addRef() != 1)//keep a ref for itself
	{
		assert(0);
	}*/

	if (!mrTypesMgr.owner()->typeProject())
	{
		//add to the namespace - only take a slot!
		TypePtr iNSOwner(mrTypesMgr.owner());
		NamesMgr_t& rNS(*iNSOwner->typeComplex()->namesMgr());// NamespaceInitialized(iNSOwner));
		const char *pstr(OpTyp2Str(t, 1));//do not fix names(~)
		if (pstr[0] != '~')
			rNS.insertName(new NameRef_t(pstr));
	}

	return pType;
}


void StockTracer_t::AddStockTypes()
{
	//CreateTypesMgr(mrTypeRef);

	addStockType(OPTYP_NULL);
	addStockType(OPSZ_BYTE);
	addStockType(OPSZ_WORD);
	addStockType(OPSZ_DWORD);
//?	addStockType(OPSZ_FWORD);
	addStockType(OPSZ_QWORD);
//?	addStockType(OPSZ_TWORD);

	//addStockType(OPTYP_BIT);
	addStockType(OPTYP_REAL0);
	addStockType(OPTYP_REAL32);
	addStockType(OPTYP_REAL64);
	addStockType(OPTYP_REAL80);
//?	addStockType(OPTYP_BCD0);
//?	addStockType(OPTYP_BCD80);

	addStockType(OPTYP_INTEGER);
	addStockType(OPTYP_BYTE);
	addStockType(OPTYP_WORD);
	addStockType(OPTYP_DWORD);
	addStockType(OPTYP_QWORD);

	addStockType(OPTYP_BOOL);
	addStockType(OPTYP_CHAR0);
	addStockType(OPTYP_CHAR8);
	addStockType(OPTYP_CHAR16);
	addStockType(OPTYP_CHAR32);

	addStockType(OPTYP_UINT0);
	addStockType(OPTYP_UINT8);
	addStockType(OPTYP_UINT16);
	addStockType(OPTYP_UINT32);
	addStockType(OPTYP_UINT64);

	addStockType(OPTYP_INT0);
	addStockType(OPTYP_INT8);
	addStockType(OPTYP_INT16);
	addStockType(OPTYP_INT32);
	addStockType(OPTYP_INT64);

	//addStockType(OPTYP _PTR);
	//addStockType( OPTYP_PTR16 );
	//addStockType( OPTYP_PTR16_FAR );
//	addStockType(OPTYP_PTR32);
	//addStockType( OPTYP_PTR32_FAR );

	//setTypeCode( new TypeCode_t() );

	//mrTypesMgr.rebuildAliases();

	mrProject.markDirty(DIRTY_TYPES);
}


