#include "front_impl.h"
#include "clean.h"
#include "main.h"
#include "proj.h"
#include "type_alias.h"
#include "type_seg.h"
#include "type_code.h"
#include "info_types.h"
#include "type_strucvar.h"
#include "shared/data_source.h"
#include "interface/IADCMain.h"
#include "anlzbin.h"

//#if(F_DEBUG)
//ADDR _FCP = ADDR(-1);
//#endif

//////////////////////////////////////////////////////IFront_Tmp

class IFront_Tmp : public I_ModuleEx//avoid being derived from project info!
{
	Main_t &mrMain;
	FrontInfo_t &mrFront;
public:
	IFront_Tmp(Main_t &rMain, FrontInfo_t &rFront)
		: mrMain(rMain),
		mrFront(rFront)
	{
	}

protected:
	virtual bool DeclareContextDependentType(const char *){ assert(0); return false; }
	virtual bool RegisterFormatterType(const char *sTypeName){
		return mrFront.storeDynamicTypeRef0(sTypeName, nullptr);
	}
	virtual bool DeclareCodeType(const char *){ assert(0); return false; }

//protected:
	//virtual size_t dataAt(OFF_t rawOffs, OFF_t size, PDATA pDest) const { assert(0); return 0; }
	//virtual OFF_t size() const { assert(0); return 0; }
	//virtual const I_AuxData *aux() const { assert(0); return 0; }
};




//////////////////////////////////////////////////////FrontInfo_t


MyString FrontInfo_t::toFrontKey(const char *pc, const char *frontDll)
{
	MyString s(FENAME_PFX);
	s.append(frontDll);
	s.append(pc);
	return s;
}

MyString FrontInfo_t::fromFrontKey(const MyString &s, MyString &frontDll)
{
	if (!s.startsWith(FENAME_PFX))
		return s;
	int n(s.find(EXSYM_PFX));
	if (n < 0)
		return s;
	frontDll = s.left(n);
	return s;
}

I_FrontMain *FrontInfo_t::loadMain(Main_t &rMain)
{
	assert(!hModule);
	hModule = rMain.dynLibMgr().Load(fPath.Path().c_str());
	if (!hModule)
		return nullptr;

	//perform setup
	HDYNFUNC pf(rMain.dynLibMgr().GetFunction(hModule, "imp_FrontMain"));
	if (!pf)
	{
		rMain.dynLibMgr().Unload(hModule);
		unload(rMain);
		return nullptr;
	}

	I_FrontMain *pIFront((I_FrontMain *)pf);
	miRefs++;
	return pIFront;
}

bool FrontInfo_t::loadInitial(Main_t &rMain)
{
	I_FrontMain *pFrontMain(loadMain(rMain));
	if (!pFrontMain)
		return false;

	IFront_Tmp tmp(rMain, *this);

	if (name().empty())//never have been loaded
	{
		//set name
		MyString frontName(pFrontMain->name());
		if (!frontName.startsWith(FENAME_PFX))
		{
			rMain.printError() << "Frontend's name (" << frontName << ") must start with " << FENAME_PFX << std::endl;
			unload(rMain);
			return false;
		}
		//frontName.remove(0, (unsigned)MyString(FENAME_PFX).length());
		setName(frontName);

		//register external types
		pFrontMain->RegisterTypes(tmp);
	}
	else
	{
		//pIFront->Setup(tmp);
	}

	return true;
}

HDYNFUNC FrontInfo_t::loadSymbol(Main_t &rMain, const char *symName)
{
	bool bJustLoaded(false);
	if (!hModule)
		bJustLoaded = loadInitial(rMain);
	HDYNFUNC pf(nullptr);
	MyString s(symName);
	int n(s.find(EXSYM_PFX));
	if (!(n < 0))
	{
		s.remove(0, n + 1);
		s.prepend("imp_");
		pf = rMain.dynLibMgr().GetFunction(hModule, s.c_str());
	}
//	if (bJustLoaded)
	//	unload(rMain);
	return pf;
}

I_FrontMain *FrontInfo_t::load(Main_t &rMain)
{
	HDYNFUNC pf(loadSymbol(rMain, EXSYM_PFX "FrontMain"));
	return (I_FrontMain *)pf;
}

bool FrontInfo_t::unload(Main_t &rMain, bool bQuite)
{
	if (!hModule)
		return false;
	//assert(miRefs > 0);
	if (--miRefs != 0)
		return false;
	if (rMain.dynLibMgr().Unload(hModule))
	{
		hModule = nullptr;
		if (!bQuite)
			fprintf(stdout, "Front end unloaded %s\n", fPath.Path().c_str());
	}
	return true;
}

bool FrontInfo_t::storeDynamicTypeRef0(const char *sTypeName, I_DynamicType *pf)
{
	if (!MyString(sTypeName).startsWith(FENAME_PFX))
		return false;
	assert(MyString(sTypeName).startsWith(name()));
	return storeDynamicTypeRef(sTypeName, nullptr);
}

bool FrontInfo_t::storeDynamicTypeRef(const char *pc, I_DynamicType *pf)
{
	std::pair<CreateTypeMapIt, bool> ret;
	ret = mFmtFactory.insert(std::make_pair(std::string(pc), pf));
	if (!ret.second)
		ret.first->second = pf;//refreash
	return ret.second;
}



////////////////////////////////////////////////////////////// FrontImplBase_t

FrontImplBase_t::FrontImplBase_t(Project_t &rProj, TypePtr iModule)
	: ModuleInfo_t(rProj, *iModule),
	m_cont(*this),
	//mbInstMode(false),
	mbSignalsEnabled(false)
{
}

FrontImplBase_t::~FrontImplBase_t()
{//RAII
	BinaryCleaner_t<> BC2(ProjectInfo_t(*this, mMemMgrNS));
	BC2.destroyUserTypesNS(mTypesMgrNS);
	BC2.destroy(mTypesMgrNS);
}

TypePtr FrontImplBase_t::NewTypedefImpl(const char *name, TypePtr type)
{
	assert(0);
	return nullptr;
}

TypePtr	FrontImplBase_t::NewBitsetImpl(Locus_t& aLoc)
{
	TypePtr pCont(aLoc.struc());

	Strucvar_t* pStrucvar(pCont->typeStrucvar());
	assert(!pStrucvar);

	ADDR va(scope().m_addr);
	//aLoc.add(pCont, va, nullptr);
	FieldPtr pField0(aLoc.field0());
	assert(pField0);
	aLoc.setField(nullptr);

	const FieldMap& m(pCont->typeStruc()->fields());
	for (FieldMapCIt it(pField0); it != m.end() && KEY(it) == va; ++it)
	{
		CFieldPtr pField(VALUE(it));
		if (!pField->type() || pField->isTypeBitset())
		{
			aLoc.setField(pField);
			break;
		}
	}
	if (!aLoc.field0())
		AppendUField(aLoc);

	if (!aLoc.field0()->type())
		SetTypeEx(aLoc.field0(), memMgr().NewTypeRef(new Bitset_t()));

	TypePtr iType(aLoc.field0()->type());
	assert(iType->typeBitset());

	return iType;
}

/*TypePtr FrontImplBase_t::NewProcImpl(Locus_t& aLoc)
{
	ProjModifier_t PM(*this);
	if (PM.MakeCode(aLoc, true) == RESULT_FAILED)
		return nullptr;

	RESULT_e eRet(PM.MakeProcedure(aLoc, false));
	if (eRet == RESULT_FAILED)
		return nullptr;

	if (eRet != RESULT_ALREADY)
		PM.SweepProc(aLoc.field0(), mrMain.options().nCallDepth);
	
	FieldPtr pField(aLoc.field0());
	TypePtr iProc(pField->type());
	assert(IsProc(iProc));

	SetTypeEx(aLoc.field0(), iProc);
	pushScope(iProc, iProc->base(), false);

	return iProc;
}*/


TypePtr FrontImplBase_t::NewScopeImpl0(SCOPE_enum eScope, Locus_t& aLoc, const MyString& sTypeName)
{
	TypePtr iType(nullptr);
	TypePtr pCont(aLoc.struc());


	switch (eScope)
	{
	case SCOPE_ENUM:
	case SCOPE_STRUC:
	case SCOPE_VFTABLE:
	{
		Struc_t *pStruc(new Struc_t());
		iType = memMgr().NewTypeRef(pStruc);
		if (eScope == SCOPE_ENUM)
			iType->flags() |= TYP_ENUM;
		break;
	}
	case SCOPE_NAMESPACE:
	{
		Struc_t *pStruc(mrProject.OnNewNamespace());
		iType = memMgr().NewTypeRef(pStruc);
		break;
	}

	default:
		assert(0);
		break;
	}

	assert(iType);

	bool bShared(!sTypeName.empty());
	if (bShared)
	{
		TypesMgr_t* pTypesMgr(nullptr);
		for (std::list<frame2_t>::reverse_iterator i(m_cont.rbegin()); i != m_cont.rend(); i++)
		{
			if ((pTypesMgr = i->cont()->typeStruc()->typeMgr()) != nullptr)
				break;
		}
		if (!pTypesMgr)
		{
			//signalError("non-existant types map");
			AssureNamesFile();
			pTypesMgr = AssureTypeMgr(ModulePtr());
		}
		TypesTracer_t TT(*this, *pTypesMgr);
		TT.addTypeNew0(iType);
		TT.addNamedObj(pTypesMgr->owner(), iType, sTypeName);
		pushScope(iType, iType->base(), false);
	}
	else
	{
		SetTypeEx(aLoc.field0(), iType);
		pushScope(iType, iType->base(), true);
	}
	return iType;
}

TypePtr	FrontImplBase_t::NewScopeImpl(FieldPtr pField, SCOPE_enum eScope)
{
	if (!pField || pField->type())
		return nullptr;

	TypePtr pCont(!m_cont.empty() ? scope().cont() : FindFrontSeg());
	assert(pCont);

	frame2_t &aTop(scope());

	assert(eScope != SCOPEX_BITSET);

	int uBytes(pField->size());
	if (uBytes == 0)
		uBytes = OPSZ_BYTE;
	if (uBytes > 0)
		aTop.m_addr -= uBytes;//rollback a top frame
		

	if (pCont->typeStrucvar())
	{
		pushScope(pCont, pCont->base(), true);
		installNamespaceImpl();
		return pCont;
	}

	Locus_t aLoc;
	aLoc.add(pCont, scope().m_addr, pField);

	return NewScopeImpl0(eScope, aLoc);
}

TypePtr FrontImplBase_t::NewStrucvarImpl(TypePtr pType)
{
	pushScope(pType, pType->base(), true);
	if (/*mbInstMode ||*/ pType->typeStrucvar())
		installNamespaceImpl();
	return pType;
}

TypePtr FrontImplBase_t::NewBitsetImpl()
{
	TypePtr pType(nullptr);
	TypePtr pCont(!m_cont.empty() ? scope().cont() : FindFrontSeg());
	assert(pCont);

	Locus_t aLoc;
	aLoc.add(pCont, scope().m_addr, nullptr);

	if (!pCont->typeStrucvar())
	{
		if (!aLoc.field0() && !InsertField(aLoc))
			return nullptr;
		pType = NewBitsetImpl(aLoc);
		if (!pType)
			return nullptr;
		SetTypeEx(aLoc.field0(), pType);
	}
	else
	{
		pType = pCont->typeStrucvar()->bitset(pCont);
	}

	pushScope(pType, pType->base(), true);
	if (pType->typeStrucvar())
		installNamespaceImpl();

	return pType;
}

TypePtr FrontImplBase_t::NewScopeImpl(const char* pTypeName, SCOPE_enum eScope)
{
	MyString sTypeName(pTypeName);
//CHECK(sTypeName.startsWith("tagCY"))
//STOP

	TypePtr pCont(!m_cont.empty() ? scope().cont() : FindFrontSeg());
	assert(pCont);

	if (!sTypeName.empty())//shared
	{
		if (!pCont->typeStrucvar())
		{
			NamesMgr_t *pNS(OwnerNamesMgr(pCont, nullptr));
			if (pNS && pNS->isInitialized())//no name fiddling here, otherwise the refs from FE would fail
			{
				ObjPtr pObj(pNS->findObj(sTypeName));
				if (pObj)
				{
					TypePtr pType(pObj->objType());
					if (pType && pType->typeStruc() && !pType->typeStruc()->hasFields())//not to process twice
					{
						if (eScope == SCOPE_ENUM)
							if (pType->ObjType() == OBJID_TYPE_STRUC)
								if (!pType->isEnum())
								{
									//pType->flags() |= TYP_ENUM;//ca't do this - the fields can already by typed with it!
									PrintError() << "Attempt to override existing scope with ENUM" << std::endl;
									return nullptr;
								}
						pushScope(pType, pType->base(), true);
						return pType;//if empty
					}
					return nullptr;//no error reporting as well - this could be intent (for attic types)
				}
			}
		}
	}

	Locus_t aLoc;
	aLoc.add(pCont, scope().m_addr, nullptr);

	assert(eScope != SCOPEX_BITSET);

	if (sTypeName.isEmpty())//a field if both field and type names not given
	{
		assert(!pCont->typeStrucvar());

		if (!aLoc.field0() && !InsertField(aLoc))
			return nullptr;

		return NewScopeImpl0(eScope, aLoc);
	}

	return NewScopeImpl0(eScope, aLoc, sTypeName);
}

void FrontImplBase_t::LeaveImpl()
{
	if (m_cont.empty())
		throw(-8);
	closeBitset();
	popScope();
}

void FrontImplBase_t::selectFileImpl(const char *path, const char* folder)
{
	if (folder)
		std::cout << folder << "/";
	std::cout << path << std::endl;
}

void FrontImplBase_t::installNamespaceImpl()
{
	assert(!m_cont.empty());
	AssureNamespace(scope().cont());
}

void FrontImplBase_t::installTypesMgrImpl()
{
	TypePtr iScope(scope().cont());
	if (iScope->typeMgr())
		return;
	TypePtr iModule(ModuleOf(iScope));
	if (iModule)
	{
		AssureTypeMgr(iScope);
		return;
	}
	NewTypesMgr(iScope);
	RegisterTypesMap(iScope, true);
}

static bool check_ordinal(FieldPtr pField, unsigned uOrdinal)
{
/*?	if (!pField->nameless())
		if (pField->ordinal() != uOrdinal)
		{
			if (pField->ordinal() != -1)
			{
				fprintf(STDERR, "Warning: Export ordinal %d does not match one in importing module (%d) for entry: %s\n",
					uOrdinal, (unsigned)pField->ordinal(), pField->name()->c_str());
			}
			return false;
		}*/
	return true;
	/*if (!sName.empty())
	{
		NamesMgr_t *pNS(OwnerNamesMgr(scp.cont(), nullptr));
		Obj_t *pObj(pNS->find Obj(sName));
		if (pObj && pObj->objField())
		{
			//check if ordinal in exporting module and that one in the importing do match
			unsigned uOrdinal2(pObj->objField()->_key() - ORDINAL_BIAS);
			if (uOrdinal2 != uOrdinal)
			{
				if (uOrdinal == -1)
					uOrdinal = uOrdinal2;
				else
					fprintf(STDERR, "Warning: Import/export ordinals do not match for entry: %s\n", sName.c_str());
			}
		}
	}
	else
	{
		//reclaim phony fields by ordinal
		STOP
	}*/
}

FieldPtr FrontImplBase_t::declUnionField(const MyStringEx& name, TypePtr iType, AttrIdEnum attr)
{
	if (scope().cont()->typeBitset())
		popScope(true);//do not advance a parent frame!

	TypePtr iScope(scope().cont());
	assert(iScope->typeStruc()->maybeUnion());

	//first, lets find if such field already exists
	ADDR addr(scope().addr());

	FieldMap& m(iScope->typeStruc()->fields());
	for (FieldMapIt it(m.lower_bound(addr)); KEY(it) == addr && it != m.end(); ++it)
	{
		FieldPtr pField(VALUE(it));
		//assert(pField->offset() == 0 || pField->isTypeBitset());
		if (pField->type() == iType)
		{
			if (!pField->name())
				return pField;
			const char* pc(name.c_str());
			if (!pc)
				return pField;//adopt existing one
			if (strcmp(pField->name()->c_str(), pc) == 0)//the same?
				return pField;
		}
	}

	Locus_t aLoc;
	aLoc.add(iScope, addr, nullptr);

	FieldPtr pField;
	if (iType && !iType->isShared() && iScope == iType)
	{
		assert(0);
		//handle is a scopefield request
		pField = iType->parentField();
	}
	else
	{
		pField = AppendUField(aLoc);
		SetTypeEx(aLoc.field0(), iType);
	}

	if (pField)
	{
		if (!pField->name())
		{
			NamesMgr_t* pNs(nullptr);
			for (std::list<frame2_t>::reverse_iterator i(m_cont.rbegin()); i != m_cont.rend(); i++)
			{
				pNs = i->cont()->typeComplex()->namesMgr();
				if (pNs)
				{
					//NamespaceInitialized(i->cont());
					break;
				}
			}

			if (name.c_str())
			{
				if (!pNs)
				{
					pNs = iScope->typeComplex()->namesMgr();
					if (!pNs)
					{
						TypePtr pNsOwner;
						pNs = OwnerNamesMgr(iScope, &pNsOwner);//try parent
						//if (pNs)
							//NamespaceInitialized(pNsOwner);
						if (!pNs)
						{
							pNs = AssureNamespace(iScope);//establish own
							//NamespaceInitialized(iScope);
						}
					}
				}
				if (pNs)
					AddName(*pNs, name.c_str(), pField);
				else
					fprintf(STDERR, "Could not assign a name to the union's field: %s\n", name.c_str());
			}

		}

		pField->setAttributeFromId(attr);
	}
	return pField;
}

MyStringEx ModuleInfo_t::Adjusted(const MyStringEx& aName0) const
{
	MyStringEx aName(aName0);
	if (aName.size() > 2)//convert a module name to its uniuqe
	{
		//int uq(ToModuleUnique(aName[2]));//a module may be created here
		int uq;
		const MyString& s(aName[2]);
		if (!s.isEmpty())
		{
			int iCase(IsArchCaseSensitive());
			CTypePtr pModule(FindModule(s, iCase == 1));
			if (!pModule)
			{
				FolderPtr pFolder(CreateModuleFolder(s));
				pModule = pFolder->fileModule()->module();
			}
			uq = pModule->typeModule()->unique();//MODULE
		}
		else
			uq = mrModule.unique();//this module
		aName.set(2, NumberToString(uq));
	}
	return aName;
}

FieldPtr FrontImplBase_t::declUnifieldImpl(const MyStringEx& aName, TypePtr iType, AttrIdEnum attr)//see: declFieldImpl
{
	frame_t &scp(scope());
	TypePtr iScope(scope().cont());

	assert(!iScope->typeStrucvar() && (!iScope->typeBitset() || !iScope->parentField()->owner()->typeStrucvar()));
	assert(!iScope->typeUnion());
	assert(iScope->typeStruc());

	FieldPtr pField;
	if (attr & ATTR_EXPORTED)
	{
		pField = mrProject.OnMakeExported(scp, iType, aName, ModulePtr());
		if (!pField)
			return nullptr;
		assert(!IsTypeImp(pField));
		if (SetFieldName2(pField, aName, true))
			pField->setHardNamed(true);
		if (!pField->isExported())
		{
			pField->setExported(true);
			AssureExportsFile();
		}
	}
	else
	{
		if (iType && !iType->isShared() && scope().cont() == iType)
		{
			assert(0);
			//handle is a scopefield request
			pField = iType->parentField();
		}
		else
		{
			Locus_t aLoc;
			aLoc.add0(iScope, scope().m_addr, nullptr);
			AppendUField(aLoc);
			pField = aLoc.field0();
			assert(!pField->type());
			if (!SetTypeEx(aLoc.field0(), iType))
				return nullptr;//had a type already
		}
		if (!pField->name())
			SetFieldName2(pField, aName, true);
	}

	pField->setAttributeFromId(attr);
	mrProject.OnDeclField(pField, attr, aName);
	return pField;
}

FieldPtr FrontImplBase_t::declFieldImpl(const MyStringEx& aName, TypePtr iType, AttrIdEnum attr)
{
//CHECKID(iType, 0x360)
//STOP

	frame_t &scp(scope());
	TypePtr iScope(scope().cont());

//CHECK(scp.m_addr == 0xb4a90)
//STOP

	assert(!iScope->typeStrucvar() && (!iScope->typeBitset() || !iScope->parentField()->owner()->typeStrucvar()));
	assert(!iScope->typeUnion());
	assert(iScope->typeStruc());

//CHECK(aName[0].startsWith("_ZNK8QDomNode6isNullEv"))
//CHECK(aName[1] == "283")
//CHECK(scp.o == 0x207113c5)
CHECK(aName[1] == "956")
STOP

	FieldPtr pField;
	bool bExisted(false);
	if (attr & ATTR_EXPORTED)
	{
		pField = mrProject.OnMakeExported(scp, iType, aName, ModulePtr());
		if (!pField)
			return nullptr;
CHECKID(pField, 0xb92)
STOP
		assert(!IsTypeImp(pField));
		if (SetFieldName2(pField, aName, true))
			pField->setHardNamed(true);
		if (!pField->isExported())
		{
			pField->setExported(true);
			AssureExportsFile();
		}
	}
	else
	{
		if (iType && !iType->isShared() && scope().cont() == iType)
		{
//?			assert(0);
			//handle is a scopefield request
			pField = iType->parentField();
		}
		else
		{
			Locus_t aLoc;
			aLoc.add(scope().cont(), scope().m_addr, nullptr);
			if (!InsertField(aLoc))
			{
				if (!aLoc.field0())
					return nullptr;//neither existed and failed to insert
				bExisted = true;
//				return nullptr;
			}
			pField = aLoc.field0();
			if (TypeObj_t::canConform(iType, pField->type()))
				if (!SetTypeEx(aLoc.field0(), iType))
					return nullptr;//had a type already
		}
		
		//assert(!pField->isExported());
		if (!pField->name() && aName.size() > 0)
		{
			if (SetFieldName2(pField, aName, true))
			{
				if (IsTypeImp(pField))
				{
					assert(!pField->nameless());
					pField->setHardNamed(true);//cannot be renamed by user
					AssureImportsFile();
				}
			}
		}
	}

	if (!bExisted)
	{
		pField->setAttributeFromId(attr);
		mrProject.OnDeclField(pField, attr, aName);
	}
	return pField;
}

void FrontImplBase_t::updateFrameSize(TypePtr type)//, FieldPtr pField)
{
	frame2_t &scp(scope());
	assert(!scp.cont()->typeArray());
	if (!type)
		scp.m_addr += OPSZ_BYTE;
	else if (type->typeStrucvar())
	{
		//?assert(0);
	}
	else
	{
		Seg_t *pSeg(type->typeSeg());
		if (pSeg)
			scp.m_addr += (ADDR)SegTraceSize(type);
		else
		{
			if (scp.cont()->typeBitset())//if (IsTypeBit(type))
			{
				//ADDR ofs(pField->offset());
				//if ((ofs + 1) % CHAR_BIT == 0)
				scp.m_addr += BitSize(type);
			}
			else
			{
				unsigned uSz(type->sizeBytes());
				if (uSz != -1)
					scp.m_addr += type->sizeBytes();
			}
		}
	}
}

FieldPtr FrontImplBase_t::declBitfieldImpl(HNAME name, TypePtr iType, AttrIdEnum attr, ADDR at)
{
	TypePtr iType2(iType);
	unsigned uBitsNum(1);
	unsigned uBaseSize(0);
	if (!iType->typeSimple())
	{
		Array_t *pArray(iType->typeArray());
		if (!pArray)
			ASSERT0;
		iType2 = pArray->baseType();
		if (!iType2->typeSimple())
			ASSERT0;
		uBaseSize = iType2->size();
		uBitsNum = (unsigned)pArray->total();
		if (uBitsNum == 0 || uBitsNum > uBaseSize * CHAR_BIT)
		{
			if (mbSignalsEnabled)
				throw (-5);
			return nullptr;
		}
	}
	else
		uBaseSize = iType2->size();

	if (iType2->typePtr())//and enum!
		ASSERT0;

	//check if it's time to leave the current bitset
	if (scope().cont()->typeBitset())
	{
		unsigned uScopeOffs(scope().m_addr);
		if (at != -1)
			uScopeOffs = at;
		//unsigned uTypeSize(iType->size());
		//unsigned uBitsNum(uTypeSize / uBaseSize);
		if (uScopeOffs + uBitsNum > uBaseSize * CHAR_BIT)
			popScope();
	}

	if (!scope().cont()->typeBitset())
	{
		if (!NewBitsetImpl())
		{
			printError(signalError("b-field declared not in scope of bitset"));
			return nullptr;
		}
		frame2_t &fr(scope());
		fr.uExtent = iType2->size();//the very first field establishes a bitset's size
	}

	Strucvar_t *pStrucvar(scope().cont()->parentField()->owner()->typeStrucvar());

	//FieldPtr pField(AddStrucvarField(scope().cont(), name, iType, attr));
	//if (pField)
	if (pStrucvar)
	{
		updateFrameSize(iType);
		return nullptr;// pField;
	}

	if (at != -1)
		scope().m_addr = at;

	Locus_t aLoc;
	aLoc.add(scope().cont(), scope().m_addr, nullptr);

	if (!InsertField(aLoc) || !SetTypeEx(aLoc.field0(), iType))
		return nullptr;

	aLoc.field0()->setAttributeFromId(attr);

	if (name)
	{
		assert(!aLoc.field0()->isExported());
		SetFieldName2(aLoc.field0(), name, false);
	}

	updateFrameSize(iType);// , pField);
	return aLoc.field0();
}

FieldPtr FrontImplBase_t::declEnumfieldImpl(HNAME name, /*AttrIdEnum attr, */I_Module&)
{
	frame2_t &scp(scope());
	TypePtr cpx(scope().cont());
	if (!(cpx->flags() & TYP_ENUM))
	{
		printError(signalError("e-field declared not in scope of enum"));
		return nullptr;
	}
	//if (val != -1 || attr == ATTRE_CONFN)
		//setcpImpl(val);//set at specified offset

	MyStringEx aName(name);
	assert(aName.size() == 1);

	closeBitset();

	FieldPtr pField(declFieldImpl(aName, nullptr, /*attr*/ATTR_NULL));
	if (pField)
		updateFrameSize(nullptr);
	return pField;
}

FieldPtr FrontImplBase_t::declEnumfieldImpl(HNAME name, ADDR val, /*AttrIdEnum attr, */I_Module&)
{
	frame2_t &scp(scope());
	TypePtr cpx(scope().cont());
	if (!(cpx->flags() & TYP_ENUM))
	{
		printError(signalError("e-field declared not in scope of enum"));
		return nullptr;
	}
	//if (val != -1 || attr == ATTRE_CONFN)
		setcpImpl(val);//set at specified offset

	MyStringEx aName(name);
	assert(aName.size() == 1);
	closeBitset();
	FieldPtr pField(declFieldImpl(aName, nullptr, /*attr*/ATTR_NULL));
	if (pField)
		updateFrameSize(nullptr);
	return pField;
}

HFIELD FrontImplBase_t::declCodeFieldImpl(HNAME name, I_Module::CODE_TYPE_e eType, AttrIdEnum attr, I_Module&)
{
	TypePtr pSeg(scope().cont());
	if (!pSeg->typeSeg())
		return nullptr;

	TypesMgr_t* pTypeMgr(findTypeMgr(pSeg));
	if (!pTypeMgr)
		return nullptr;

	MyStringEx aName(Adjusted(name));
	closeBitset();
	FieldPtr pField(declFieldImpl(aName, nullptr, attr));
	if (!pField)
		return nullptr;

CHECKID(pField, 0x7c6)
STOP

	//TypePtr pSeg2(rSeg.ownerRangeSet(pSeg));

	ADDR va(scope().addr());

	FieldPtr pField0(CloneLead(pField));//could be a clone

	Locus_t aLoc;
	LocusFromVA(pField0, aLoc);

	//FieldMapIt it(pField0);
	//if (SetTypeEx(FieldMapItEx(aLoc.back().cont(), it), pTypeMgr->getTypeCode()))
	{
		RESULT_e eRes(RESULT_FAILED);
		ProjModifier_t MI(*this);
		MyString sOpt;
		if (eType == I_Module::CODE_TYPE_THUNK)
		{
			if ((eRes = MI.MakeThunk(aLoc, false, true)) == RESULT_FAILED)
				eType = I_Module::CODE_TYPE_PROC;//if not a thunk - fall back to a proc
			//else if (eRes == RESULT_OK && !aName[0].isEmpty())
				//sOpt = "-nppg";//propagate a name to the thunk's target - not to a thunk itself! (maybe this should be optional?)
		}
		if (eType == I_Module::CODE_TYPE_PROC)
		{
			eRes = MI.MakeCode(aLoc, false);
			if (eRes != RESULT_FAILED)//could be a thunk
				if (MI.MakeThunk(aLoc, false, true) == RESULT_FAILED)//via a thunk
					MI.MakeProcedure(aLoc, false);
		}
		else if (eType == I_Module::CODE_TYPE_PROC_FORCE)
		{
			eRes = MI.MakeCode(aLoc, false);
			MI.MakeProcedure(aLoc, false);
		}
		else if (eType == I_Module::CODE_TYPE_CODE)
		{
			eRes = MI.MakeCode(aLoc, false);
		}
		if (eRes == RESULT_FAILED)
			return nullptr;

		if (eRes == RESULT_OK)
			MI.TriggerCodeSweep(pSeg, va, sOpt);
	}

	return aLoc.field0();
}

TypePtr FrontImplBase_t::typeImpl(OpType_t typeID)
{
	if (mrProject.typeMgr())
		return mrProject.getStockType(typeID);
	TypesMgr_t *pTypesMgr;
	TypePtr iType(GetStockType(typeID, &pTypesMgr));
	if (iType)
	{
		assert(pTypesMgr == iType->ownerTypeMgr());
		return iType;
	}
	return nullptr;
}

TypePtr FrontImplBase_t::typeImpl(HNAME name)
{
	//MyString s(name);
	TypesMgr_t *pTypesMgr;
	TypePtr p(nullptr);
	if (!m_cont.empty())//?
		p = findType(scope().cont(), name, &pTypesMgr);
	if (!p)
		signalError("type can't be found: %s", name);
	return p;
}


TypePtr FrontImplBase_t::arrayOfImpl(TypePtr t, unsigned num, bool bytes)
{
	TypePtr iType((TypePtr)t);
	if (!iType)
		return nullptr;
		//signalError("invalid base type for array");
	assert(num >= 0);
	if (bytes)
	{
		if (iType->typeSimple())//try to exclude simple types
		{
			int sz(iType->size());
			assert(num % sz == 0);
			num /= sz;
		}
		else
			num |= ARRAY_BYTES_MASK;
	}
	if (m_cont.isTopStrucvar())
	{
		ModuleInfo_t MI2(*this, mMemMgrNS);
		TypesTracer_t TT(MI2, mTypesMgrNS);
		iType = TT.arrayOf(iType, num);//don't register refs for cached types (produced by strucvars)
		iType->flags() |= TYP_NO_REF;
		return iType;
	}
	TypesMgr_t *pTypesMgr(iType->ownerTypeMgr());
	TypesTracer_t TT(*this, *pTypesMgr);
	return TT.arrayOf(iType, num);
}

TypePtr FrontImplBase_t::arrayOfIndexImpl(TypePtr iType, TypePtr iTypeIndex)
{
	TypesMgr_t *pTypesMgr(iType->ownerTypeMgr());
	TypesTracer_t tt(*this, *pTypesMgr);
	return tt.arrayOfIndex(iType, iTypeIndex);
}

TypePtr	FrontImplBase_t::constOfImpl(TypePtr iType)
{
	TypesMgr_t *pTypesMgr(iType->ownerTypeMgr());
	TypesTracer_t tt(*this, *pTypesMgr);
	return tt.constOf(iType);
}

TypePtr FrontImplBase_t::enumOfImpl(TypePtr t, int typ)
{
	TypePtr iType((TypePtr)t);
	if (!iType)
		errorImpl("undefined enum type");
	if (!typ || !(typ & OPSZ_MASK))
		typ = OPTYP_UINT32;//?
	else if (!(typ & OPTYP_MASK))
		typ = MAKETYP_UINT(typ);
	else if (!OPTYP_IS_INT(typ))
		ASSERT0;
	if (m_cont.isTopStrucvar())
	{
		ModuleInfo_t MI2(*this, mMemMgrNS);
		TypesTracer_t TT(MI2, mTypesMgrNS);
		iType = TT.enumOf(iType, (OpType_t)typ);//don't register refs for cached types (produced by strucvars)
		iType->flags() |= TYP_NO_REF;
		return iType;
	}
	TypesMgr_t *pTypesMgr(iType->ownerTypeMgr());
	TypesTracer_t TT(*this, *pTypesMgr);
	return TT.enumOf(iType, (OpType_t)typ);
}

OpType_t from_PTR_TYPE(I_Module::PTR_TYPE_t e)
{
	switch (e)
	{
	case I_Module::PTR_16BIT: return OPSZ_WORD;
	case I_Module::PTR_32BIT: return OPSZ_DWORD;
	case I_Module::PTR_64BIT: return OPSZ_QWORD;
	case I_Module::PTR_AUTO:
	default: break;
	}
	return OPSZ_NULL;
}

TypePtr FrontImplBase_t::ptrOfImpl(TypePtr _iType, I_Module::PTR_TYPE_t eType, I_Module::PTR_MODE_t eMode)
{
	TypePtr iType((TypePtr)_iType);
	TypesMgr_t *pTypesMgr;
	if (!iType)
	{
		iType = GetStockType(OPTYP_NULL, &pTypesMgr);
		assert(iType && pTypesMgr == iType->ownerTypeMgr());
		iType = nullptr;//?
	}
	else
		pTypesMgr = iType->ownerTypeMgr();

	OpType_t ptrSize(from_PTR_TYPE(eType));
	if (ptrSize == OPTYP_NULL)
	{
		TypePtr iSeg(OwnerSeg(scope().cont()));
		ptrSize = iSeg->typeSeg()->ptrSize();
	}
	//assert(ptrSize == OPSZ_QWORD || ptrSize == OPSZ_DWORD);

	TypesTracer_t TT(*this, *pTypesMgr);

	if (eMode == I_Module::PTR_MODE_REF)
		return TT.refOf(iType, ptrSize);
	if (eMode == I_Module::PTR_MODE_RVREF)
		return TT.rvalRefOf(iType, ptrSize);
	return TT.ptrOf(iType, ptrSize);
}

TypePtr FrontImplBase_t::impOfImpl(TypePtr base)//I_Module::PTR_TYPE_t eType)
{
	//find typesmgr where 'void*' resides
//	TypePtr iType(GetStockType(TYPEID_IPTR));
	//if (iType)
		//return iType;
	TypesMgr_t *pTypesMgr;
	TypePtr iType(scope().cont());
	TypePtr iSeg(OwnerSeg(iType));
	iType = GetStockType(OPTYP_NULL, &pTypesMgr);
	assert(iType && pTypesMgr == iType->ownerTypeMgr());
	//bool bLarge(iSeg ? iSeg->typeSeg()->ptrSize() : false);
	TypesTracer_t tt(*this, *pTypesMgr);
	return tt.impOf(base);// PtrSizeOf(iSeg));
}

TypePtr FrontImplBase_t::expOfImpl(TypePtr base)
{
	TypesMgr_t *pTypesMgr;
	TypePtr iType(scope().cont());
	TypePtr iSeg(OwnerSeg(iType));
	iType = GetStockType(OPTYP_NULL, &pTypesMgr);
	assert(iType && pTypesMgr == iType->ownerTypeMgr());
	TypesTracer_t tt(*this, *pTypesMgr);
	return tt.expOf(base);
}

TypePtr	FrontImplBase_t::pairOfImpl(TypePtr iLeft, TypePtr iRight)
{
	if (!iLeft)
		iLeft = GetStockType(OPTYP_NULL);
	if (!iRight)
		iRight = GetStockType(OPTYP_NULL);
	TypesMgr_t &tm(*mrProject.typeMgr());
	TypesTracer_t TT(*this, tm);
	return TT.pairOf(iLeft, iRight);
}

TypePtr	FrontImplBase_t::funcOfImpl(TypePtr iRetVal, TypePtr iArgs, unsigned flags)
{
	TypesMgr_t &tm(*mrProject.typeMgr());
	TypesTracer_t TT(*this, tm);
	return TT.funcTypeOf(iRetVal, iArgs, flags);
}

POSITION FrontImplBase_t::fp(TypePtr p) const
{
	POSITION pos(p->base());
	while (p && !p->typeSeg())
	{
		assert(!p->typeStrucvar());
		pos -= p->base();
		FieldPtr f(p->parentField());
		if (!f)
			break;
		pos += f->_key();
		p = f->owner();
	}
	return pos;
}

POSITION FrontImplBase_t::setcpImpl(POSITION offs)
{
	frame_t& aTop(m_cont.back());
	POSITION bias(fp(aTop.cont()));
	if (aTop.cont()->typeBitset())
	{
		ScopeStack_t::reverse_iterator rit(m_cont.rbegin());
		assert(rit != m_cont.rend());
		frame_t& aTop2(*(++rit));
		POSITION bias2(fp(aTop2.cont()));
		if (offs >= bias2)
			aTop2.m_addr = (offs - bias2) + aTop2.cont()->base();
		//update a position in bitset as well..
		if (offs >= bias)
		{
			size_t bytePos((offs - bias) + aTop.cont()->base());
			if (bytePos < aTop.cont()->sizeBytes())
				aTop.m_addr = ADDR(bytePos * CHAR_BIT);
		}
	}
	else
	{
		if (offs >= bias)
			aTop.m_addr = (offs - bias) + aTop.cont()->base();
	}
	return cpImpl();
}

POSITION FrontImplBase_t::cpImpl() const
{
	POSITION pos(0);
	ScopeStack_t::const_reverse_iterator cri(m_cont.rbegin());
	if (cri == m_cont.rend())
		return -1;
	
	for (; cri != m_cont.rend(); ++cri)
	{
		const frame2_t &f(*cri);
		if (!f.cont()->typeStrucvar())
		{
			if (!f.cont()->typeBitset())
				break;
			if (f.bAdvance)
				pos += f.cont()->sizeBytes();
			++cri;//discard a bitset
			break;
		}
		pos += f.m_addr;
	}

	const frame_t &scp(*cri);
	pos += fp(scp.cont());
	pos += (scp.m_addr - scp.cont()->base());
	return pos;
}

OFF_t FrontImplBase_t::cprImpl() const
{
	const frame_t &f(scope());
	if (f.empty())
		return 0;
	ADDR o(f.m_addr - f.cont()->base());
	if (f.cont()->typeBitset())
		o /= CHAR_BIT;
	return f.m_offs + o;
}

void FrontImplBase_t::errorImpl(const char *message)
{
	POSITION pos(cpImpl());
	PrintError() << "at position " << VA2STR(pos) << ": " << message << std::endl;
	if (mbSignalsEnabled)
		throw(-666);
	//throw message;
}

unsigned FrontImplBase_t::skipImpl(int bytes) 
{
	scope().m_addr += bytes;
	return bytes;
}

unsigned FrontImplBase_t::skipBitsImpl(int bits)
{
	if (!scope().cont()->typeBitset())
		NewBitsetImpl();
	scope().m_addr += bits;
	return bits;
}

POSITION FrontImplBase_t::alignImpl(unsigned powerOf2)
{
	closeBitset();

	frame_t &r(scope());

	POSITION o(cpImpl());
	unsigned u(((unsigned)-1) << powerOf2);
	u = ~u;
	if (o & u)
	{
		POSITION o0(o);
		o >>= powerOf2;
		o++;
		o <<= powerOf2;
		r.m_addr += (o - o0);
	}
	return o;
}

#ifndef WIN32
int _vscprintf (const char * format, va_list pargs) { 
      int retval; 
      va_list argcopy; 
      va_copy(argcopy, pargs); 
      retval = vsnprintf(nullptr, 0, format, argcopy); 
      va_end(argcopy); 
      return retval; 
   }
#endif

const MyString &FrontImplBase_t::signalError(const char *fmt, ...)
{
	if (fmt)
	{
#if(1)
		va_list args;
		va_start(args, fmt);
		int len = _vscprintf(fmt, args) + 1;// _vscprintf doesn't count terminating '\0'
		char *buf = (char *)malloc(len * sizeof(char));
#ifdef WIN32
		vsprintf_s(buf, len, fmt, args);
#else
		vsprintf(buf, fmt, args);
#endif
		va_end(args);
		mLastError = buf;
		free(buf);
#else
		char buf[1024];
		va_list va;
		va_start(va, fmt);
#ifdef WIN32
		vsprintf_s(buf, sizeof(buf) - 1, fmt, va);
#else
		vsprintf(buf, fmt, va); 
#endif
		va_end(va);
		mLastError = buf;
#endif
	}
	else
		mLastError.clear();
	if (mbSignalsEnabled)
		throw (-1);
	return mLastError;
}

void FrontImplBase_t::printError(const MyString &s)
{
	if (!s.isEmpty())
		MAIN.printError() << s << std::endl;
}

std::ostream& FrontImplBase_t::printError() const
{
	return MAIN.printError();
}

std::ostream& FrontImplBase_t::printWarning() const
{
	return MAIN.printWarning();
}

FullName_t FrontImplBase_t::hyperLinked(const FullName_t& a) const
{
	return MAIN.hyperLinked(a);
}

bool FrontImplEx_t::RegisterFormatterType(const char *pTypeName)
{
	assert(isScopeEmpty());
	return mrFront.storeDynamicTypeRef0(pTypeName, nullptr);
}

bool FrontImplBase_t::EnterScopeImpl(ADDR addr)
{
	assert(!m_cont.empty());
	TypePtr pScope(scope().cont());
	FieldPtr pField(Field(pScope, addr, nullptr, FieldIt_Exact));
	if (pField)
	{
		TypePtr pSubScope(pField->isTypeStruc());
		if (!pSubScope)
			return false;
		if (!pSubScope->isShared())
		{
			pScope = pSubScope;
			addr = pField->_key();
		}
	}
	m_cont.push_scope(pScope, addr, false);
	return true;
}

bool FrontImplBase_t::EnterScopeImpl(TypePtr iScope, ADDR addr)
{
	assert(!m_cont.empty());
	if (!iScope)
		return false;
	assert(iScope->typeStruc());
	m_cont.jump_scope(iScope, addr, false);
	return true;
}

/*bool FrontImplBase_t::EnterScope(const char *varname)
{
	TypePtr iType(getType(varname));
	assert(iType);
	m_cont.push_scope(iType, 0);
	return true;
}*/

bool FrontImplBase_t::EnterSegmentImpl(TypePtr pRangeSeg, ADDR addr)
{
	assert(!m_cont.empty());
	if (!pRangeSeg)
	{
		//try to find one
		TypePtr iSeg(OwnerSeg(scope().cont()));
		if (!iSeg)
			return false;
		pRangeSeg = iSeg->typeSeg()->ownerRangeSet(iSeg);
		if (!pRangeSeg)
			return false;
		assert(pRangeSeg->typeSeg()->isRangeSet());
	}
	else if (!pRangeSeg->typeSeg())// a seg trace? file pointer?
	{
		TypePtr pStruc(pRangeSeg);
		if (pStruc->typeStruc() && !pStruc->isShared())
		{
			ADDR base(pStruc->base());
			if (base <= addr && addr < base + pStruc->size())
			{
				//translate an offset in trace to VA in some seg (possible nested one)
				Seg_t* pSegPvt(pStruc->parentField()->owner()->typeSeg());
				if (pSegPvt)
				//if(0)
				{
					for (SubSegMapCIt i(pSegPvt->subsegs().begin()); i != pSegPvt->subsegs().end(); ++i)
					{
						TypePtr pSubSeg(IVALUE(i));
						if (pSubSeg->typeSeg()->traceLink() == pStruc)
						{
							ROWID ra(0);
							TypePtr pSeg5(R2D2(pSubSeg, addr, ra));
							if (!pSeg5)
								return false;
							/*addr = pSeg5->base() + (ADDR)ra;*/
							addr += pSeg5->base();//?
							m_cont.push_scope(pSeg5, addr, false);
							return true;
						}
					}
				}

				m_cont.push_scope(pStruc, addr, false);
				return true;
			}
		}
		return false;
	}

	const Seg_t &rSegRange(*pRangeSeg->typeSeg());
	//assert(rSeg);

#if(1)
	for (FieldMapCRIt I = rSegRange.fields().rbegin(), E = rSegRange.fields().rend(); I != E;  ++I)//IMPORTANT: look from the back to enter a superseg last!
	{
		CFieldPtr pField(VALUE(I));
		TypePtr iSeg2(pField->type());
		if (!iSeg2)//skip a seg end
			continue;
		assert(iSeg2->typeSeg());
		ADDR base(iSeg2->base());
		if (base <= addr && addr < base + iSeg2->size())
		{
			m_cont.push_scope(iSeg2, addr, false);
			return true;
		}
	}
#else
	for (SubSegMapCIt i(rSegRange.subsegs().begin()); i != rSegRange.subsegs().end(); i++)
	{
		//TypePtr iSeg0(VALUE(i));
		TypePtr iSeg2(IVALUE(i));
		//if (iSeg2->typeSeg()->traceLink())
			//iSeg0 = iSeg2->typeSeg()->traceLink();

		ADDR base(iSeg2->base());
		if (base <= addr && addr < base + iSeg2->size())
		{
			m_cont.push_scope(iSeg2, addr, false);
			return true;
		}
	}
	ADDR base(pRangeSeg->base());
	if (base <= addr && addr < base + pRangeSeg->size())
	{
		m_cont.push_scope(pRangeSeg, addr, false);
		return true;
	}
#endif
	return false;
}

TypePtr	FrontImplBase_t::TraceOf(TypePtr pSeg) const
{
	return pSeg->typeSeg()->traceLink();
}













//////////////////////////////////////////////////////////////// (FrontImpl_t)


/*FrontImpl_t::FrontImpl_t(Main_t &rMain, FrontInfo_t &rFront, TypePtr iModule)
	: ProjectInfo_t(rMain.project()),
	mrMain(rMain),
	mrFront(rFront),
	mbInstMode(false),
	m_cont(*this),
	miBinary(iModule),
	mbGeomChanged(false)
{
}
*/

FrontImpl_t::FrontImpl_t(Project_t &rProj, const Locus_t &start, TypePtr iModule)
	: FrontImplBase_t(rProj, iModule),
	mrLocusz(start),
	mbGeomChanged(false)
{
	for (Locus_t::const_iterator i(start.begin()); i != start.end(); i++)
	{
		const Frame_t &f(*i);
		pushScope(f.cont(), f.addrx(), false);
	}
}

FrontImpl_t::~FrontImpl_t()
{
	for (size_t i(0); i < mPatchies.size(); i++)
	{
		delete mPatchies[i];
		mPatchies[i] = nullptr;
	}

	mrProject.OnFinalizeFrontEnd(ModulePtr());
}

void FrontImpl_t::setEndianness(bool lsb)
{
	TypePtr iSeg(scope().cont());
	Seg_t *pSeg(iSeg->typeSeg());
	if (!pSeg)
		throw(-15);//not a seg
	uint32_t u(pSeg->uflags());
	u &= ~I_SuperModule::ISEG_MSB;
	if (!lsb)
		u |= I_SuperModule::ISEG_MSB;
	pSeg->setUFlags(u);
}

void FrontImpl_t::declStrucvar(TypePtr pType, I_Module& iface)
{
	assert(pType->typeStrucvar());
	I_DynamicType *device(pType->typeStrucvar()->device());
	if (NewStrucvarImpl(pType))
	{
		SAFE_SCOPE_HERE(iface);
		device->createz(iface, -1);//!
	}
}

FieldPtr FrontImpl_t::instField(HNAME name, TypePtr iType, AttrIdEnum attr, I_Module& iface)
{
	if (iType)
	{
		Strucvar_t *pStrucvar(iType->typeStrucvar());
		if (pStrucvar)
		{
			//mbInstMode = true;
			I_DynamicType *device(pStrucvar->device());
			TypePtr iStruc(NewScopeImpl((HNAME)nullptr, SCOPE_STRUC));
			//FieldPtr pField(iStruc->parentField());
			if (!iStruc)
				return nullptr;
			FieldPtr pField(declFieldImpl(name, iStruc, ATTR_NULL));
			installNamespaceImpl();
			TRY
			{
				device->createz(iface, -1);
			}
			CATCH (const char *msg)
			{
#if(!NO_TRY)
				if (!msg)
					msg = "Unknown";
				fprintf(STDERR, "%s\n", msg);
#endif
				if (!iStruc->typeStruc()->hasFields())
				{
					LeaveImpl();//?TODO: scope chain must be consistent!

					Locus_t aLoc;
					aLoc.add(pField->owner(), pField->_key(), pField);//fix later
					DeleteField(aLoc);
					return nullptr;
				}
			}
			LeaveImpl();
			//mbInstMode = false;
			return pField;
		}
	}
	MyStringEx aName(name);
	assert(aName.size() == 1);
	if (scope().cont()->typeBitset())
		popScope();
	return declFieldImpl(aName, iType, attr);
}

bool FrontImpl_t::InstantiateType(FieldPtr pField, bool bUnnamed, I_Module& rIface)
{
	if (!pField)
		return false;
	TypePtr pType(pField->type());
	if (!pType)
		return false;
	Strucvar_t* pStrucvar(pType->typeStrucvar());
	if (!pStrucvar)
		return false;

	TakeTypeOf(pField);

	I_DynamicType* device(pStrucvar->device());
	//TypePtr iStruc((TypePtr)fimp.NewScopeImpl(nullptr, SCOPE_STRUC, nullptr, ATTR_NULL));

	Struc_t* pStruc(new Struc_t());
	TypePtr pNewType(memMgrGlob().NewTypeRef(pStruc));

	SetType(pField, pNewType);

	pushScope(pNewType, pNewType->base(), false);//enter scope

	if (!bUnnamed)
		installNamespaceImpl();
	try
	{
		device->createz(rIface, -1);
	}
	catch (...)
	{
		STOP
	}
	LeaveImpl();

	BinaryCleaner_t<> BC(*this);
	BC.ReleaseTypeRef(pType);
	return true;
}


/*void FrontImpl_t::setFlags(unsigned flags)
{
	Seg_t *pSeg(scope().cont()->typeSeg());
	assert(pSeg);
	pSeg->setUFlags(flags);
}*/

bool FrontImpl_t::DeclareContextDependentType(const char *pTypeName)
{
	assert(!isScopeEmpty());
	
	TypePtr iScope(scope().cont());

	assert(!iScope->typeStrucvar());
	{
		NamesMgr_t *pNS(OwnerNamesMgr(iScope, nullptr));
		if (pNS && pNS->findObj(pTypeName))//no name fiddling here, otherwise the refs from FE would fail
			return false;//no error reporting as well - this could be intent (for attic types)
	}

	Struc_t *pSeg(iScope->typeStruc());
	TypesMgr_t *pTypesMgr(pSeg->typeMgr());
	if (!pTypesMgr)
	{
		signalError("no type manager");
		return false;
	}
	TypesTracer_t TT(*this, *pTypesMgr);
	Strucvar_t *pStrucvar(new Strucvar_t);
	TypePtr iType(memMgr().NewTypeRef(pStrucvar));
	if (!TT.addTypeNew0(iType))
	{
		memMgr().Delete(iType);
		return false;
	}
	if (pTypeName)
		TT.addNamedObj(iScope, iType, pTypeName);
	return true;
}

bool FrontImpl_t::DeclareCodeType(const char *codeName)
{
	if (!MyString(codeName).startsWith(FENAME_PFX))
		return false;
	TypePtr iSeg(scope().cont());
	TypesMgr_t &rTypesMgr(*AssureTypeMgr(iSeg));
	TypesTracer_t TT(*this, rTypesMgr);
	TypePtr iType(memMgr().NewTypeRef(new TypeCode_t));
	if (!TT.addTypeNew0(iType))
	{
		memMgr().Delete(iType);
		return false;
	}
	if (codeName)
		TT.addNamedObj(rTypesMgr.owner(), iType, codeName);
	//TT.setTypeCode(iType);
	return true;
}

FieldPtr FrontImpl_t::declUField(HNAME name, TypePtr iType, AttrIdEnum attr)
{
	MyStringEx aName(Adjusted(MyStringEx(name)));

	closeBitset();

	frame_t &scp(scope());
	TypePtr iScope(scope().cont());
	if (iScope->typeStrucvar())
		signalError("U-field cannot be be a strucvar: %s", aName.c_str("<noname>"));


	if (iScope->typeBitset())//check bitvar
		signalError("U-field are not allowed in bitset: %s", aName.c_str("<noname>"));

	if (iType && !iType->isShared() && iScope == iType)//a scopefield?
		ASSERT0;// return declUnifieldImpl(aName, iType, attr);

	FieldPtr pField(declUnifieldImpl(aName, iType, attr));
	if (!pField)
		signalError("field not created: %s", aName.c_str("<noname>"));

	return pField;
}

FieldPtr FrontImpl_t::declUField(HNAME name, SCOPE_enum eScope, AttrIdEnum attr)
{
	assert(0);
	return 0;
}

FieldPtr FrontImpl_t::declField(HNAME name, TypePtr iType, AttrIdEnum attr, ADDR at, I_Module& iface)
{
	MyStringEx aName(Adjusted(MyStringEx(name)));

	//HFIELD h(declFieldImpl2(aName, (TypePtr)type.pvt, attr, at, iface));

	closeBitset();

	frame_t &scp(scope());
	TypePtr iScope(scope().cont());
	Strucvar_t *pStrucvar(iScope->typeStrucvar());
	//FieldTmp_t tmpField(pStrucvar ? iType : nullptr, pStrucvar ? name : nullptr);

	if (!pStrucvar)
	{
		if (iScope->typeBitset())//check bitvar
			pStrucvar = iScope->parentField()->owner()->typeStrucvar();
	}

	if (pStrucvar)
	{
		//tmpField.setOwnerComplex(iScope);
		//tmpField.set AttributeFromId(attr);
		//updateFrameSize(iType);
		if (iType && iType->typeStrucvar())
		{
			if (iType != iScope)//check if not a scopefield request
				declStrucvar(iType, iface);
			else if (!iType->isShared())
				ASSERT0;
		}
		else
			updateFrameSize(iType);
		//dsize = type->typeStrucvar()->querySize((PDATA)scope().rawdata());
		//strucvar advances caller frame's offset on leave
		static Field_t dummy;
		return &dummy;
	}

	if (iScope->typeUnion())
		return declUnionField(aName, iType, attr);

	if (iType && !iType->isShared() && scope().cont() == iType)//a scopefield?
		ASSERT0;// return declFieldImpl(aName, iType, attr);

	if (at != -1)
		scope().m_addr = at;

	FieldPtr pField(declFieldImpl(aName, iType, attr));
	if (!pField)
		signalError("field not created: %s", aName.c_str("<noname>"));

//	if (at == ADDR(-1))
	{
		TypePtr iType(pField->type());
		if (iType && iType->typeStrucvar())
			declStrucvar(iType, iface);//promote cp
		else
			updateFrameSize(iType);
	}
//	else
//		updateFrameSize(nullptr);//do not promote CP if address was specified (speedup)

	F_CHECK2(cprImpl());
	return pField;
}

bool FrontImpl_t::setDefaultCodeType(TypePtr iType)
{
	if (!iType || !iType->typeCode())
		return false;

	assert(!isScopeEmpty());
	Struc_t *pSeg(scope().cont()->typeStruc());

	TypesMgr_t *pTypesMgr(pSeg->typeMgr());
	TypesTracer_t TT(*this, *pTypesMgr);

	TT.setTypeCode(iType);
	return true;
}

FieldPtr FrontImpl_t::field() const
{
	return Field(scope().cont(), scope().m_addr);
}

bool FrontImpl_t::EnterAttic()
{
	if (isScopeEmpty())
	{
		printError(signalError("invalid context"));
		return false;
	}
	pushScope(mrProject.self(), 0, false);//-1
	return true;
}

/*void FrontImpl_t::setFr ontEnd(PSEG iSeg, I_Front * pIFront)
{
	Seg_t *pSeg(iSeg->typeSeg());
	pSeg->setF rontEnd(pIFront);
}*/


void FrontImpl_t::installFrontend(const char *frontName, int id)
{
	assert(!isScopeEmpty());
	TypePtr iSeg(scope().cont());
	Seg_t *pSeg(iSeg->typeSeg());
	if (!frontName || !pSeg)
	{
		printError(signalError("frontend '%s' not installed", frontName));
		return;
	}

	if (!MyString(frontName).startsWith(FENAME_PFX))
		return;
	pSeg->setFrontIndex(NewFrontEnd(frontName, id));
	FRONT_t *pF(FrontEnd(iSeg));
	pF->AddRef();
	if (pSeg != &mrModule)
		pSeg->setTitle(ModuleTitle(ModulePtr()));
	mrProject.OnInstallFrontEnd(iSeg, mOptions);
}

I_Front* FrontImpl_t::frontendImpl() const
{
	assert(!isScopeEmpty());
	CTypePtr pFromSeg(OwnerSeg(scope().cont()));
	CTypePtr pFrontSeg(FindFrontSegUp(pFromSeg));
	if (pFrontSeg)
	{
		FRONT_t *pF(FrontEnd(pFrontSeg));
		return pF->device(GetDataSource());
	}
	return nullptr;
}

void FrontImpl_t::reuseFrontend(int id)
{
	TypePtr iSeg(scope().cont());
	Seg_t *pSeg(iSeg->typeSeg());
	if (!pSeg)
	{
		printError(signalError("invalid context for frontend"));
		return;
	}

	const FoldersMap &m(mrProject.rootFolder().children());
	for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
	{
		CFolderRef rFolder(*i);
		TypePtr iModule(rFolder.fileModule()->module());
		if (iModule == ModulePtr())
			continue;
		if (iModule->typeModule()->title() != mrModule.title())//check if of the same module group
			continue;
		TypePtr iSeg2(FindFrontSegById(iModule, id));
		if (iSeg2)
		{
			pSeg->setFrontIndex(iSeg2->typeSeg()->frontIndex());
			FRONT_t *pF(FrontEnd(iSeg));
			pF->AddRef();
			mrProject.OnInstallFrontEnd(iSeg, mOptions);
			return;
		}
	}
}

TypePtr FrontImpl_t::NewRangeSet(ADDR64 base64, const char *name)
{
	return ModuleInfo_t::NewRangeSet(base64, name);
}

/*HRANGE FrontImpl_t::NewRange(TypePtr hRangeSet, ADDR base, ADDR_RANGE size)
{
	Seg_t *pSeg(scope().cont()->typeSeg());
	if (!pSeg)
		return 0;
	return pSeg->newRange(hRangeSet, base, size);
}*/

/*bool FrontImpl_t::checkRangeSet(TypePtr iRangeSet)
{
	Seg_t *pRangeSeg(iRangeSet->typeSeg());

	TypePtr iOwnerTrace(pRangeSeg->traceLink());
	if (!iOwnerTrace)
		return true;

	//convert a regular terminal seg into a range seg
	if (!pRangeSeg->subsegs().empty())
		return false;//must not have subsegs
	if (pRangeSeg->hasFields())
		return false;//some other fields have been added already
#if(0)//all fields in the trace will be deleted (including eos)
	if (iOwnerTrace->typeStruc()->hasFields())
		return false;
#endif
	//eliminate a trace link
	assert(pRangeSeg->rawBlock().m_size == iOwnerTrace->size());//in case of recovery
	TypePtr iSuperSeg(pRangeSeg->superLink());
	FieldPtr pTraceField(iOwnerTrace->parentField());
	pRangeSeg->setTraceLink(nullptr);
//	pRangeSeg->setSuperLink(nullptr);
	assert(pTraceField);
	BinaryCleaner_t BC(*this);
	BC.ClearType(pTraceField);
	iSuperSeg->typeSeg()->takeField(pTraceField);
	memMgr().Delete(pTraceField);//iOwnerTrace should be deleted here
	//range set must be registered in superseg
	mrModule.addSubRange(iRangeSet);
	//but must be not treated like a subseg
	iSuperSeg->typeSeg()->removeSubseg(iRangeSet);
	pRangeSeg->setRangeSet(true);

	return true;
}*/

TypePtr FrontImpl_t::AddSubRange(TypePtr iRangeSeg, ADDR addrV, ADDR_RANGE sizeV, TypePtr iSeg)
{
	if (!iSeg)
	{
		PrintError() << "Invalid parameter to I_SuperModule::AddSubRange()" << std::endl;
		return nullptr;
	}
	ModuleInfo_t::AddSubRange(iRangeSeg, addrV, sizeV, iSeg);
	//adjust cp
	scope().m_addr += iSeg->base();
	return iSeg;
}

TypePtr FrontImpl_t::NewSegment(unsigned rawSize, const char *name, unsigned uFlags)
{
	ADDR rawOffs(0);
	TypePtr iSegOwner(ModulePtr());
	if (!isScopeEmpty())
	{
		iSegOwner = scope().cont();
		rawOffs = scope().m_addr;
	}

	/*TypePtr pRangeSet((TypePtr)hRangeSet);
	if (iSegOwner == pRangeSet)
	{
		if (!checkRangeSet(pRangeSet))
			return nullptr;
	}*/

	//TypePtr pRange((TypePtr)hRange);
//	ADDR addrBaseV(pRange->base);
//	ADDR_RANGE segSizeV(pRange->iSeg->size());
//	uint64_t imageBase(pRange->parent->imageBase);
	/*if (rawSize == 0)//?
	{
		PrintError() << "failed to create portal '" << name << "' (0 raw size)" << std::endl;
		return nullptr;
	}*/

	TypePtr iSeg(MakeSegment(iSegOwner, rawOffs, rawSize, name, uFlags));
	if (!iSeg)
		return nullptr;//signalError(nullptr);
	
	mbGeomChanged = true;//geom updated will be required
	
	pushScope(iSeg, iSeg->base(), true);//addrBaseV);
	return iSeg;
}

size_t FrontImpl_t::dataAt(OFF_t rawOffs, OFF_t size, PDATA pDest) const
{
	const Block_t &r(mrModule.rawBlock());
	if (r.empty() || size == 0)
		return 0;

	if (!(rawOffs < r.m_size))
		printError(const_cast<FrontImpl_t *>(this)->signalError("no data at ~%08X", rawOffs));

	return GetDataSource()->pvt().dataAt(rawOffs, size, pDest);
}

I_DataSourceBase *FrontImpl_t::openFile(const char *path, bool bLocal) const
{
	return GetDataSource()->pvt().openFile(path, bLocal);
}

DataPtr FrontImpl_t::dataHost() const
{
	return GetDataSource()->dataHost();
}

const I_DataSourceBase *FrontImpl_t::host() const
{
	return &GetDataSource()->dataHost()->pvt();
}

/*void FrontImpl_t::setFieldAttribute(HFIELD iField, unsigned iFlags)
{
	FieldPtr pField((FieldPtr )iField);
	if (iFlags & 1)//imported
		pField->m_nFlags |= FLD_IMPORTED;
}*/


/*HFIELD FrontImpl_t::registerField(unsigned id, HNAME n, HTYPE t, unsigned attr)
{
	PSTRUC cpx(scope().cont());

	Strucvar_t *pStrucvar(cpx->typeStrucvar());
	assert(pStrucvar);

	StrucModifier_t an(*mrMain.project());
	return an.AppendFieldOfType(pStrucvar, n, t);
}*/

/*HFIELD FrontImpl_t::insertField( PSTRUC iStruc, int offs, HNAME name, HTYPE iType, int iFlags)
{ 
	if (!iStruc)
		iStruc = GetProject();
	assert(iStruc->typeStruc());
	Seg_t *pSeg(iStruc->typeSeg());
	assert(0);
	//?if (pSeg && pSeg->isSegList())
		//?iStruc = pSeg->findSubseg(offs);
	StrucModifier_t an(*mrMain.project());
	FieldPtr pField(an.InsertFieldOfType(frame_t(iStruc, offs), name, iType));
	if (pField)
	{
		if (iFlags & 1)//imported
			pField->m_nFlags |= FLD_IMPORTED;
	}
	return pField;
}*/

/*PSEG FrontImpl_t::findSubseg(PSEG p, ADDR a)
{
	Seg_t * pdC = p->typeSeg();
	return pdC->findSubseg(a);
}*/

#if(0)
void FrontImpl_t::addFileView(const char *name)
{
	MyString s(name);
	//assert(s.endsWith(RESOURCE_EXT));

/*	FTYP_Enum e(FTYP_FOLDER);
	int n(s.findRev('.'));
	if (n >= 0)
	{
		MyString sExt(s.mid(n));
		if (sExt == RESOURCE_EXT)
			e = FTYP_RESOURCES;
		else
		{
			assert(0);//later
		}
		s.truncate(n);
	}

	Folder_t *pFolder(mrMain.project()->files().AddFile(s.c_str(), e));
	(void)pFolder;*/
}
#endif

/*bool FrontImpl_t::setFieldType(HFIELD iField, HTYPE iType)
{
	FieldPtr pField((FieldPtr )iField);
	StrucModifier_t an(*mrMain.project());
	if (!an.setT ype(pField, iType))
		return false;
	return true;
}*/

/*void FrontImpl_t::addType(PSTRUC tseg, HTYPE t, HNAME name)
{
	Struc_t *pOwner(nullptr);
	if (tseg)
		pOwner = tseg->typeSeg();
	else
	{

	}

	TypePtr pType(t);
	if (pType->typeStrucvar())
	{
		STOP
	}
	//else
	{
		TypesTracer_t tt(*pOwner->assureTypeMgr());
		tt.addTypeNew(pType, name);//?
	}
}*/






TypePtr FrontImpl_t::code()
{
	Struc_t *pSeg(scope().cont()->typeStruc());
	if (!pSeg->typeSeg())
		return nullptr;
	TypesMgr_t *pTypesMgr(pSeg->typeMgr());
	return pTypesMgr->getTypeCode();
}

/*bool FrontImpl_t::enqueEntryPoint(ADDR va)
{
	CTypePtr iFrontSeg(FindFrontSeg());
	if (!iFrontSeg)
		return false;
	//TypePtr iSeg(scope().cont());
	CTypePtr iSeg(FindSegAt(iFrontSeg, va, iFrontSeg->typeSeg()->affinity()));
	if (iSeg)
	{
		//const Seg_t &rSeg(iSeg->typeSeg());
		//!pSeg->setEntryPoint(va);

		fprintf(stdout, "Enqueuing address %s for code analysis...\n", VA2STR(iSeg, va, -1).c_str());
		//MyString s(MyStringf("makefunc -@ %X", va));
		MyString s(MyStringf("makefunc"));
		if (mrMain.mrSturtupInfo.bFastDisasm)
			s.append(MyStringf(" -d 1"));//do not deeply analyze funcs except this one (how many functions are going to be analized)

		Locus_t aLoc;
		LocusFromVA_1(iSeg, va, aLoc);
			
		//LocusFromDA(rSeg2.viewOffs(iSeg2, va), loc);

		Probe_t *pLoc(new Probe_t(aLoc));
		mrMain.postContextCommand(s, pLoc);
		pLoc->Release();

		return true;
	}
	return false;
}*/

/*void FrontImpl_t::AddCxxSymbol(ADDR va, const char *mangled, const char *demangled)
{
	mrProject.OnAddCxxSymbol(va, mangled, demangled);
}*/

void FrontImpl_t::setAuxData(PDATA ptr, size_t sz)
{
	GetDataSource()->pvt().setAuxData(ptr, sz);
}

/*int FrontImpl_t::FieldOffset(HFIELD f)
{
	FieldPtr  pField = (FieldPtr )f;
	assert(pField->objField());
	return pField->_key();
}

int FrontImpl_t::FieldSize(HFIELD f)
{
	FieldPtr  pField = (FieldPtr )f;
	assert(pField->objField());
	return pField->size();
}

int FrontImpl_t::FieldEnd(HFIELD f)
{
	FieldPtr  pField((FieldPtr )f);
	assert(pField->objField());
	return pField->_key() + pField->size();
}*/

bool FrontImpl_t::setSize(unsigned size)
{
	frame_t &r(scope());
	//Struc_t *pStruc(r.cont()->typeStruc());
	return SetStrucSize(r.cont(), size);
}

HPATCHMAP	FrontImpl_t::newPatchMap()
{
	return (HPATCHMAP)mPatchies.newSlot(new PatchList_t);
}

bool FrontImpl_t::addPatch(HPATCHMAP index, OFF_t offs, unsigned size)
{
	PatchList_t *p(mPatchies.getSlot(index));
	if (!p)
		return false;
	if (p->append(offs, size))
		return true;
	//mPatchies.deleteSlot(index);//can't do this: the new map can be created in it's stead!
	delete mPatchies[index];
	mPatchies[index] = nullptr;
	PrintError() << "Invalid range passed to the patch map " << index << ". The map is invalidated." << std::endl;
	return false;
}

I_DataSourceBase *FrontImpl_t::stitchDerivativeModule(HPATCHMAP patchMapIndex, const char *moduleSfx, const char *type, bool bDelayed, int moduleTag)
{
	I_DataSourceBase *pRet(nullptr);
	PatchList_t *pm(mPatchies.getSlot(patchMapIndex));
	if (pm && !pm->empty())
	{
		DataPtr pData(GetDataHost());
		const char *pcTitle(pData->name());
		MyString sTitle(pcTitle ? MyString(pcTitle) : ModuleTitle(ModulePtr()));

		assert(moduleSfx);
		MyString sSubTitle(Module_t::makeSubtitle(sTitle, moduleSfx));
		FolderPtr pFolder(AddFile(sSubTitle + MODULE_SEP));
		if (!pFolder->hasPvt())
		{
			TypePtr iModule(CreateModule(NextModuleUnique()));
			pFolder->SetPvt(new FileModule_t(iModule));
			iModule->typeModule()->setFolderPtr(pFolder);
			ModuleInfo_t MI2(*this, *iModule);
			if (!bDelayed)
			{
				MI2.AssureNamesFile();
				MI2.AssureTypeMgr();
			}

			Module_t &aBin(*iModule->typeModule());
			aBin.setTitle(sTitle);
			aBin.setSubTitle(sSubTitle);//no path yet
			aBin.setModuleTag(moduleTag);

			DataLeech_t *pDataLeech(new DataLeech_t(*pm, pData));
			DataPtr pData(memMgr().NewDataObj());
			pData->setPvt(pDataLeech);
			MI2.SetDataSource(pData);//not needed
			//pDataLeech->Release();

			MI2.UpdateViewGeometry2();
			if (type != nullptr)
			{
				aBin.setDelayedFormat(type);
				if (!bDelayed)
					MI2.FireDelayedFormat();
			}

			pRet = pDataLeech;
		}
	}

	delete mPatchies[patchMapIndex];
	mPatchies.deleteSlot(patchMapIndex);
	return pRet;
}

void ModuleInfo_t::FireDelayedFormat()
{
	MyString sType(mrModule.delayedFormat());
	if (!sType.empty())
	{
		mrModule.setDelayedFormat("");
		Locus_t aLoc;
		aLoc.push_back(Frame_t(mrModule.rawBlock(), ModulePtr(), 0, nullptr));
		//aLoc.setFolder(pFolder);
		Probe_t *pCtx(mrProject.NewLocus(aLoc));
		MyStringf sCmd("preformat -n %s -t %s", ModuleTitle(ModulePtr()).c_str(), sType.c_str());
		mrMain.postContextCommand(sCmd, pCtx, true);//raw offs
	}
}

TypePtr ModuleInfo_t::FindFrontSeg() const
{
	for (SubSegMapCIt i(mrModule.subsegs().begin()); i != mrModule.subsegs().end(); i++)
	{
		TypePtr pSeg(ProjectInfo_t::FindFrontSegIn(IVALUE(i)));//look only in real segments (not seg traces)
		if (pSeg)
			return pSeg;
	}
	if (mrModule.frontIndex())
		return ModulePtr();
	return nullptr;
}

//check if arch is case-sensitive(1) or case-insensitve(0), (-1:n/a)
int ModuleInfo_t::IsArchCaseSensitive() const
{
	TypePtr pSeg(FindFrontSeg());
	if (!pSeg)
		return -1;
	return pSeg->typeSeg()->uflags() & I_SuperModule::ISEG_NCASE ? 0 : 1;
}

/*I_DataSource *FrontImpl_t::createDataLeech(HPATCHMAP patchMapIndex, bool bDropPM)
{
	DataLeech_t *pDataSource(nullptr);
	PatchList_t *pm(mPatchies.getSlot(patchMapIndex));
	if (pm && !pm->empty())
		pDataSource = new DataLeech_t(*pm, mrModule.dataSource());
	if (bDropPM)
	{
		delete mPatchies[patchMapIndex];
		mPatchies.deleteSlot(patchMapIndex);
	}
	return pDataSource;
}*/

bool FrontImpl_t::preformatModule(int moduleTag, const char *typeStr, bool bDelayed)
{
	//const char *moduleSfx;
	DataPtr pData(GetDataHost());
	const char *pcTitle(pData->name());
	MyString sTitle(pcTitle ? MyString(pcTitle) : ModuleTitle(ModulePtr()));

	//MyString sModuleTo(Module_t::makeSubtitle(sTitle, moduleSfx));
	//FolderPtr pFolderTo(ModuleFromPath(sModuleTo));
	FolderPtr pFolderTo(FindModuleFolderByTag(moduleTag));
	if (!pFolderTo)
		return false;

	TypePtr iModule(ModuleOf(pFolderTo));
	Module_t &aBin(*iModule->typeModule());
	ModuleInfo_t MI2(*this, *iModule);

	if (typeStr)
	{
		aBin.setDelayedFormat(typeStr);
		if (!bDelayed)
			MI2.FireDelayedFormat();
	}
/*	{
		Locus_t aLoc;
		aLoc.push_back(Frame_t(aBin.rawBlock(), iModule, 0, nullptr));

		char  cmd[128];
		sprintf(cmd, "preformat %s", typeStr);
		bool echo(true);

		Probe_t *pCtx(mrProject.NewLocus(aLoc));
		adc::CEventCommand *pCmd(new adc::CEventCommand(cmd, echo));
		pCmd->setContextZ(pCtx);
		pCtx->Release();

		mrMain.postEvent(pCmd);
	}*/
	return true;
}

I_DataSourceBase *FrontImpl_t::module(const char *moduleSfx)
{
	DataPtr pData(GetDataHost());
	const char *pcTitle(pData->name());
	MyString sTitle(pcTitle ? MyString(pcTitle) : ModuleTitle(ModulePtr()));
	MyString sModuleTo(Module_t::makeSubtitle(sTitle, moduleSfx));
	FolderPtr pFolderTo(ModuleFromPath(sModuleTo));
	if (!pFolderTo)
		return nullptr;

	TypePtr iModule(ModuleOf(pFolderTo));
	ModuleInfo_t MI2(mrProject, *iModule);
	return &MI2.GetDataSource()->pvt();
}

I_DataSourceBase *FrontImpl_t::module(int moduleTag)
{
	//DataPtr pData(GetDataHost());
	//const char *pcTitle(pData->name());
	//MyString sTitle(pcTitle ? pcTitle : ModuleTitle(binary()));
	//MyString sModuleTo(Module_t::makeSubtitle(sTitle, moduleSfx));
	//FolderPtr pFolderTo(ModuleFromPath(sModuleTo));
	FolderPtr pFolderTo(FindModuleFolderByTag(moduleTag));
	if (!pFolderTo)
		return nullptr;

	TypePtr iModule(ModuleOf(pFolderTo));
	ModuleInfo_t MI2(mrProject, *iModule);
	return &MI2.GetDataSource()->pvt();
}


