#include "proj_ex.h"
#include "db/debug.h"
#include "db/anlzbin.h"
#include "db/front_impl.h"
#include "db/symbol_map.h"
#include "main_ex.h"
#include "info_dc.h"
#include "type_funcdef.h"
#include "ui_main_ex.h"
#include "probe_ex.h"
#include "make_type.h"
#include "clean_ex.h"
#include "ana_post.h"
#include "ana_main.h"
#include "sym_parse.h"
#include "info_class.h"
#include "file_cmd.h"
#include "db/type_proxy.h"


ProjectEx_t::ProjectEx_t(Main_t &rMain, MemoryMgr_t &rMemMgr)
	: Project_t(rMain, rMemMgr),
	mDisperseMode(false),
	mpUnExtModule(nullptr)
{
//?	setContext(new ProbeEx_t)->Release();

	//expand auto-prefix map
	mAutoPfxMap.add("func_", 4);//hex suffixes for globs (2 is reserved for fields in Project_t)
	mAutoPfxMap.add("thunk_", 4);
}

ProjectEx_t::~ProjectEx_t()
{
//?	setContext(nullptr);
}

/*void ProjectEx_t::rebuildTypesMap()
{
	const FoldersMap &m(rootFolder().children());
	for (FoldersMap::const_iterator it(m.begin()); it != m.end(); it++)
	{
		const Folder_t &rFolder(*it.pvt());
		TypePtr iModule(ProjectInfo_t::ModuleOf(rFolder));
		Dc_t *pDC(DCREF(iModule));
		if (pDC->primeSeg())
		{
			DcInfo_t DI(*pDC);
			DI.RebuildTypesMap(exp TypesMap());
		}
	}
}*/

/*bool ProjectInfoEx_t::UpdateExpFieldInfo(FieldPtr pField) const
{
	assert(!pField->nameless());
	ExpFieldsMap &m(projx().mExpFieldsMap);
	ExpFieldsMapIt i(m.find(pField->name()));
	if (i == m.end())
		return false;
	ExpFieldInfo_t &exp(i->second);
	exp.pField = pField;
	return true;
}*/

/*ExpTypesMapIt ProjectEx_t::getExportedTypeInfoIt(const MyString &fullName, TypePtr iModule)//search in other modules
{
	ExpTypesMap &m(exp TypesMap());

	ExpTypesMapIt i(m.lower_bound(fullName));
	for (; i != m.end() && i->first == fullName; i++)
	{
		TypePtr iModule2(ModuleInfo_t::ModuleOf(i->second));
		if (iModule2 != iModule)
			return i;
	}

	return m.end();

#if(0)

	ExpTypesMapIt i(m.find(name));
	if (i != m.end())
	{
/ *#ifdef _DEBUG
		ExpTypesMapIt j(i);
		j++;
		if (j != m.end())
			if (i->first == j->first)
			//if (strcmp(i->first->c_str(), j->first->c_str()) == 0)
			{
				STOP
			}
#endif* /
	}
	return i;
#endif
}*/

#if(!NO_TYPE_PROXIES)
ExpTypesMapIt ProjectEx_t::getExportedTypeInfoIt2(const MyString &name)
{
	ExpTypesMap &m(ex pTypesMap());
	ExpTypesMapIt i(m.lower_bound(name));// &nameRef));
	if (i != m.end())
	{
#ifdef _DEBUG
		ExpTypesMapIt j(i);
		j++;
		if (j != m.end())
			if (i->first == j->first)
			//if (strcmp(i->first->c_str(), j->first->c_str()) == 0)
			{
				STOP
			}
#endif
	}
	return i;
}

ExpTypesMapIt ProjectEx_t::getExportedTypeInfoIt(TypePtr pType)
{
	MyString sName(TypeNameFull(pType));

	const ExpTypesMap &m(expTy pesMap());
	for (ExpTypesMapIt i(m.lower_bound(sName)); i != m.end(); i++)
	{
		if (i->first != sName)
			break;
		if (i->second == pType)
			return i;
	}
	return m.end();
}
#endif

/*TypePtr DcInfo_t::FindCachedType(const MyString &fullName, bool bOwn)
{
	TypePtr iModule(ModulePtr());
	ExpTypesMap &m(projx().exp TypesMap());
	ExpTypesMapIt i(m.lower_bound(fullName));
	for (; i != m.end() && i->first == fullName; i++)
	{
		TypePtr iModule2(ModuleInfo_t::ModuleOf(i->second));
		bool bEqual(iModule2 == iModule);
		if (bOwn == bEqual)
			return i->second;
	}
	return nullptr;
}*/

/*bool ProjectEx_t::removeExportedTypeInfo(const MyString &sName, TypePtr pType)
{
#ifndef _DEBUG
	if (!pType->isExp orting())
		return false;
#endif

	bool bRemoved(false);
	ExpTypesMap &m(exp TypesMap());
	for (ExpTypesMapIt i(m.lower_bound(sName)); i != m.end(); i++)
	{
		if (i->first != sName)
			break;
		if (i->second == pType)
		{
			assert(!bRemoved);
			assert(pType->isEx porting());
			m.erase(i);
#if(!DEBUG_NS)
			pType->releaseRef();
#endif
			pType->setExp orting(false);
			bRemoved = true;
			break;
			//check if there is no more entries with such value?
		}
	}
	return bRemoved;
}*/

void ExpTypesMap::addType(const MyString &fullName, TypePtr p)
{
	assert(!fullName.isEmpty() && p);
	assert(!p->typeProxy());
	const MyString &s(fullName);

CHECKID(p, 38350)
STOP

#ifdef _DEBUG
	//make sure this type is not already in a map
	ExpTypesMapIt i(lower_bound(s));
	for (; i != end() && i->first == s; i++)
	{
		if (i->second == p)
			return;//already
		TypePtr iModule1(ModuleInfo_t::ModuleOf(p));
		TypePtr iModule2(ModuleInfo_t::ModuleOf(i->second));
		assert(iModule1 != iModule2);
	}
#endif
	insert(std::make_pair(fullName, p));
	//p->setExporting(true);

#if(0)
	std::pair<std::map<MyString, TypePtr>::iterator, bool> iret(mTypeCache.insert(std::make_pair(fullName, p)));
	if (!iret.second)
	{
		if (iret.first->second->typeProxy())
		{
			assert(!p->typeProxy());
			iret.first->second = p;//from now on, all refs are to the non-proxy
		}
		else
		{
			assert(p->typeProxy());
		}
	}
#endif
}

TypePtr ExpTypesMap::findType(const MyString &fullName, TypePtr iModule, bool bOwn)
{
	size_t m(MyString::npos);
	ExpTypesMapIt i;
	if (fullName.back() == DUB_TERMINATOR)
		m = fullName.rfind(DUB_SEPARATOR);

	if (m != MyString::npos)
		i = lower_bound(fullName.substr(0, m));
	else
		i = lower_bound(fullName);
	
	for (; i != end(); i++)
	{
		const MyString &s(i->first);
		if (s != fullName)
		{
#if(1)
			break;
#else
			if (s.back() == DUB_TERMINATOR)
			{
				size_t n(s.rfind(DUB_SEPARATOR));
				if (n != MyString::npos)
				{
					if (m != MyString::npos)//different ids!
					{
						if (s.substr(0, n) != fullName.substr(0, m))
							break;
					}
					else if (s.substr(0, n) != fullName)
						break;
				}
			}
			else
			{
				if (m != MyString::npos)//different ids!
				{
					if (s != fullName.substr(0, m))
						break;
				}
				else
					break;
			}
#endif
		}
		TypePtr iModule2(ModuleInfo_t::ModuleOf(i->second));
		bool bEqual(iModule2 == iModule);
		if (bOwn == bEqual)
			return i->second;
	}

//	ExpTypesMapIt i(projx().getExportedTypeInfoIt(fullName, ModulePtr()));
//	if (i != projx().exp TypesMap().end())
//		return i->second;
	return nullptr;
}

/*TypePtr Dc_t::removeCachedType(const char *fullName, TypePtr p)
{
	assert(p);
	MyString s(fullName);
	ExpTypesMap &m(project().exp TypesMap());
	ExpTypesMapIt i(m.lower_bound(s));
	for (; i != m.end() && i->first == s; i++)
	{
		if (i->second == p)
		{
			m.erase(i);
			p->setExp orting(false);
			return p;
		}
	}
	return nullptr;

#if(0)
	std::map<MyString, TypePtr>::const_iterator i(mTypeCache.find(fullName));
	if (i == mTypeCache.end())
		return nullptr;
	TypePtr p(i->second);
	mTypeCache.erase(i);
#endif
	return p;
}*/

/*TypePtr Dc_t::findCachedType(const MyString &fullName, TypePtr iModule)
{
	assert(iModule->typeModule());
	ExpTypesMap &m(project().exp TypesMap());
	ExpTypesMapIt i(m.lower_bound(fullName));
	for (; i != m.end() && i->first == fullName; i++)
	{
		assert(ModuleInfo_t::CheckOwnership(i->second));
		TypePtr iModule2(ModuleInfo_t::ModuleOf(i->second));
		if (iModule2 == iModule)
			return i->second;
	}

#if(0)
	std::map<MyString, TypePtr>::const_iterator i(mTypeCache.find(fullName));
	if (i != mTypeCache.end())
		return i->second;
#endif
	return nullptr;
}*/

bool ProjectEx_t::OnInstallFrontEnd(TypePtr iPrimeSeg, MyString sOpts)
{
	TypePtr iModule(ProjectInfo_t::ModuleOf(iPrimeSeg));
	ModuleInfo_t MI(*this, *iModule);

	const Seg_t &rSeg(*iPrimeSeg->typeSeg());
	FRONT_t *pFE(MI.FrontEnd(iPrimeSeg));// rSeg.frontEnd());
	assert(pFE);

	bool bRet(false);

	MI.AssureNamespace(iPrimeSeg);
	//?installTypesMgr();
	//only binary and prime segs are alive at this point

	Dc_t *pDC(DcInfo_t::DcFromModule(iModule));
	if (pDC)
	{
		pFE->releaseDevice();//refresh
		pDC->setFrontEnd(nullptr);
		pDC->setFrondDC(nullptr);
		pDC->initFrontend();
	}
	else
	{
		FolderPtr pModuleFolder(MI.FolderOfModule(iModule));
		//create DC if front end provides support for it
		I_Front *pFront(pFE->device(MI.GetDataSource()));
		I_FrontDC *pFrontDC(pFront->createFrontDC());
		if (!pFrontDC)
			return false;
		pFrontDC->Release();
		pDC = new Dc_t(*this, iPrimeSeg, iModule);
	}

	DcInfo_t DI(*pDC);
	DI.InitDc();

	//request post-formatting
	Probe_t* pLoc(new Probe_t);
	pLoc->locus().add(iModule, 0, nullptr);
	MyString sCmd("postformat");
	if (!sOpts.isEmpty())
		sCmd.append(" " + sOpts);
	mrMain.postContextCommand(sCmd, pLoc);
	pLoc->Release();

	return true;
}

void ProjectEx_t::OnBogusFields(TypePtr pSeg)
{

}

TypePtr ProjectEx_t::OnNewPortal(TypePtr iSegOwner)//, FrontImpl_t &frontImpl)
{
	TypePtr iSeg(nullptr);
	Seg_t *pSegMain(iSegOwner->typeSeg());
	if (pSegMain->typeModule())
	if (pSegMain->subsegs().size() == 1)
	{
		TypePtr iSeg2(pSegMain->subsegs().front());
		//if (!iSeg2->typeSeg()->traceLink())
		if (iSeg2->typeSeg()->isPhantom())
		{
			//got a phantom seg
			//convert a phantom seg into a normal one
			iSeg = iSeg2;
			Seg_t &rSeg(*iSeg->typeSeg());

			assert(!rSeg.traceLink());
			
			//now have to relocate the phony (export) fields

			TypePtr iFrontSeg(iSeg);
			ProjectInfo_t PI(*this);
			assert(PI.FrontOf(iFrontSeg));

			TypePtr iModule(ProjectInfo_t::ModuleOf(iSegOwner));
			Dc_t *pDC(DCREF(iModule));

			//clear a statics map
//			pDC->m_stat.clear();//will be recovered later

			FieldMap &m(rSeg.fields());
			while (!m.empty())
			{
				FieldMapIt i(m.begin());
				FieldPtr pField(rSeg.takeField0(i));
				/*if (pField->isCloneMaster())
				{
					assert(!pField->type());//it's a phantom module!
					assert(!pField->hasUserData() && pField->nameless());
					memMgr().Delete(pField);
				}
				else*/
				{
					assert(pField->isExported());
					//?assert(pField->address() != 0);
					pField->setOwnerComplex(nullptr);//avoid assertion
					pDC->addBogusField(pField->_key(), pField);
					/*if (pField->isStaticMember())
					{
						Dc_t::StaticMapIt i(dcInfo.FindStaticIt(pField));
						assert(i != pDC->m_stat.end());
						pDC->m_stat.erase(i);//do not disrupt an established member list (recover a stats map later)
					}*/
				}
			}
/*			ConflictFieldMap &m2(rSeg.conflictFields());
			while (!m2.empty())
			{
				ClonedFieldMapIt i(m2.begin());
				FieldPtr pField(VALUE(i));
				m2.take(i);
				assert(!pField->nameless());
	//?			pField->setOrdinal(-1);
				pField->flags() &= ~FLD_CLONED;
				pField->setOwnerComplex(nullptr);//avoid assertion
				pDC->addBogusField(-1, pField);//va, which is ordinal cannot be trusted for cloned fields (in phantom modules)
			}*/

			rSeg.setPhantom(false);//no longer a phantom
		}
	}
	return iSeg;
}

static unsigned __toOrdinal(const MyString& s)
{
	return s.empty() ? ORDINAL_NULL : atoi(s.c_str());
}

FieldPtr ProjectEx_t::OnMakeExported(const frame_t& scp, TypePtr iType, const MyStringEx& aName, TypePtr pModule)
{
	ModuleInfo_t MI(*this, *pModule);
//	exportPool().checkValid();

	//check if there is already one among phony fields
	Dc_t* pDC(DCREF(pModule));
	if (pDC)
	{
		DcInfo_t DI(*pDC);
		FieldPtr pField(DI.TakeBogusField(aName));
		if (pField)
		{
			if (!MI.InsertFieldAt(scp.cont(), pField, scp.m_addr))
			{
				//the field at this address already exists (it is normal for exported stuff to share code)
				pField = MI.AddSecondaryField(scp.cont(), scp.m_addr, pField);
				//assert(pField && pField->isClone());
			}
			return pField;
		}
	}

	//check the unresolved externals module to see if the field is already there;
	//if found, use that field (having it stripped from glob,name and type) - those will be re-created in post-anal
	TypePtr pUModule(unresolvedExternalsModule());
	if (pUModule)
	{
		DcInfo_t DI2(*DCREF(pUModule));
		FieldPtr pField(DI2.ExtraditeUnresolvedExternal(aName, pModule));
		if (pField)
		{
			if (!MI.InsertFieldAt(scp.cont(), pField, scp.m_addr))
			{
				pField = MI.AddSecondaryField(scp.cont(), scp.m_addr, pField);
				//assert(pField && pField->isClone());
			}
			return pField;
		}
	}

	return Project_t::OnMakeExported(scp, iType, aName, pModule);
}

void ProjectEx_t::OnUnmakeExported(FieldPtr pField)
{
	exportPool().remove(pField);
	TypePtr pModule(ProjectInfo_t::ModuleOf(pField));
	DcInfo_t DI(*DCREF(pModule));
	MyStringEx aName(ToCompoundName(pField->name()));
	DI.ClearFieldName(pField);
	DI.SetFieldName(pField, aName[0], true);
	Project_t::OnUnmakeExported(pField);
}

void ProjectEx_t::OnDeclField(FieldPtr pField, AttrIdEnum attr, const MyStringEx& aName)
{
	if (pField->isExported())
	{
		exportPool().add(pField);

		/*unsigned uOrdinal(__toOrdinal(aName[1]));
		assert(pField->isExported());
		//if (pField->nameless())
		if (uOrdinal != ORDINAL_NULL)
		{
			TypePtr iModule(ProjectInfo_t::ModuleOf(pField));
			ModuleInfo_t MI(*this, *iModule);
			FolderPtr pFolder(MI.AssureExportsFile());
			//if (!pDC->exportMap().add(uOrdinal, pField))
			if (!pFolder->fileExports()->expMap().add(uOrdinal, pField))
				ASSERT0;
		}*/
		if (pField->hasUserData())
		{
			GlobPtr iGlob(DcInfo_s::GlobObj(pField));
//CHECKID(iGlob, 22871)
//STOP
			assert(iGlob->folder() == iGlob->folder());
			FileDef_t* pFileDef(iGlob->folder()->fileDef());
			GlobMapIt i(pFileDef->findGlobSlowIt(iGlob));//realy?? when adding phantom modules?
			assert(i != pFileDef->globs().end());
			if (i->_key() != pField->_key())
			{
				//re-insert field at right address
				//userdataglobal_t ud(pField);
				if (pFileDef->takeGlobIt(i) != iGlob)
					ASSERT0;
				pFileDef->insertGlob(iGlob);
				//GlobMapIt i(pFileDef->findGlobIt(ud.pSelf));
				//assert(i != pFileDef->fields().end());
			}
		}
	}
	else if (CHECK_ATTR_TYPE(attr, __ATTR_CLASS))
	{
		if (DcInfo_s::IsMember(pField))
		{
			TypePtr pModule(ProjectInfo_t::ModuleOf(pField));
			Dc_t* pDC(DCREF(pModule));
			if (pDC)
			{
				DcInfo_t DI(*pDC);
				TypePtr pScope(pField->owner());
				if (attr == ATTRC_HEIR)
				{
					if (!DI.AddInheritance(pScope, pField, 1))
						mrMain.printError() << "Failed to add inheritance for " << DI.TypePrettyNameFull(pScope, CHOP_SYMB)
						<< " at offset " << pField->offset() << " with " << (pField->type() ? DI.TypePrettyNameFull(pField->type(), CHOP_SYMB) : FullName_t("<null>")) << std::endl;
				}
				else if (attr == ATTRC_VPTR)
				{
					if (!DI.MakeTypeVPtr(pField))
						mrMain.printError() << "Failed to make v-ptr for " << DI.TypePrettyNameFull(pScope, CHOP_SYMB) << pField->offset() << std::endl;
				}
			}
			else
				mrMain.printError() << "Invalid field attribute: " << attr << std::endl;
		}
		else
			mrMain.printError() << "Wrong attribute for storage class: " << attr << std::endl;
	}

	//exportPool.checkValid();
}

TypePtr ProjectEx_t::OnModuleRef(TypePtr iSelf) const
{
	if (iSelf->typeFuncDef())
		return ProjectInfo_t::ModuleOf(DcInfo_t::DockField((CGlobPtr)iSelf)->owner());
	return nullptr;
}

Struc_t *ProjectEx_t::OnNewNamespace()
{
	return new TypeNamespace_t();
}

void ProjectEx_t::OnMakeFunction(CFieldPtr pField0)
{
	//the function has been just created
	assert(pField0->isTypeProc());
	GlobPtr iGlob(DcInfo_t::GlobObj(pField0));//already in src?
	if (!iGlob)
		return;
	if (!iGlob->func())
	{
		FolderPtr pFolder(iGlob->folder());
		TypePtr iModule(ModuleInfo_t::ModuleOf(pFolder));
		DcInfo_t DI(*DCREF(iModule));
		DI.AssureFuncDef(iGlob);
		//redump(ctx, REDUMP_ALL);
		guix().GuiInvalidateDisplays(pFolder, nullptr, REDUMP_ALL);
	}
}

void ProjectEx_t::OnFinalizeFrontEnd(TypePtr pModule)
{
	Dc_t* pDC(DCREF(pModule));
	if (pDC)
	{
		DcInfo_t DI(*pDC);
		DI.CleanupUnclaimedExports();
	}
	Project_t::OnFinalizeFrontEnd(pModule);
}

void ProjectEx_t::OnNewDebugger()
{
	debugger()->setContext(new ProbeEx_t);
}

/*class CProxyFolder : public Folder_t
{
	FileDef_t fileDef;
public:
	CProxyFolder()
		: Folder_t(MyString("@TEMP"))
	{
		SetPvt0(&fileDef);
	}
	~CProxyFolder()
	{
		SetPvt0(nullptr);
	}
} g_proxyFolder;*/

void ProjectEx_t::OnDebuggerBreak(Debugger_t &dbg)
{
	mrMain.lockProjectRead(true);
	
	ProbeEx_t &cx(dynamic_cast<ProbeEx_t &>(dbg.context()));
	assert(cx.locus().empty());

	ModuleInfo_t MI(*this, *dbg.ModuleCur());
	
	//Locus_t loc;
	FieldPtr pFieldRef(MI.FindFieldInSubsegs(dbg.segOwner(), dbg.current(), cx.locus()));
	assert(0);
//?	cx.pickScope(cx.locus().funcOwner());
	//if (cx.setDcRef())
	if (cx.scopeFunc())
	{
		FileInfo_t dcInfo(*cx.dcRef(), *cx.fileDef());
		//?cx.pickFolder(dcInfo.FolderOf(cx.scopeFunc()));
		FuncInfo_t funcInfo(*cx.dcRef(), *cx.scopeFunc());// , * cx.fileDef());
		//?cx.pickOpLine(funcInfo.FindOpByVA(dbg.current()));
	}

	mrMain.lockProjectRead(false);
}

/*#ifdef _DEBUG
static bool __assert_node_type(NodeTypeEnum n)
{
	switch (n)
	{
	case NODE_FUNC:
	case NODE_THUNK:
	case NODE_VTHUNK:
	case NODE_NVTHUNK:
	case NODE_NULL:
	case NODE_TYPE:
	case NODE_RTTID_TYPE:
		return true;
	default:
		break;
	}
	return false;
}
#endif

const std::string &ProjectEx_t::demangledName(const std::string &s) const
{
	CxxSymbMapCIt i(mCxxImports.find(s));
	if (i != mCxxImports.end())
	{
		node_t *node(i->second);
		assert(__assert_node_type(node->type));//func or data
		return node->name;
	}
	static const std::string s0;
	return s0;
}*/


/*MyString ProjectEx_t::scopeFromMangled(PNameRef pName) const
{
	MyString s;
	if (pName)
	{
		s = demangledName(pName->c_str());
		size_t n(DcInfo_t::ScopePos(s));
		if (n != MyString::npos)
			return s.substr(n);
	}
	return s;
}*/

/*MyString ProjectEx_t::nameFromMangled(PNameRef pName, bool bTrimScope, unsigned &n) const
{
	MyString s;
	if (pName)
	{
		n = ProjectInfo_t::ChopName(pName->c_str(), s);

		s = demangledName(s);
		if (bTrimScope)
		{
			size_t n(DcInfo_t::ScopePos(s));
			if (n != MyString::npos)
				s.erase(0, n + 2);
		}
	}
	return s;
}*/

/*PNameRef ProjectEx_t::displayName(FieldPtr pSelf, TypePtr iModule) const
{
	if (pSelf->hasUgly Name())
	{
		if (!iModule)
		{
			ProjectInfo_t PI(*(Project_t *)this);
			iModule = PI.ModuleOf(pSelf->owner());
		}
		DcInfo_t DI(*DCREF(iModule));
		PNameRef pn(DI.FindPrettyName(pSelf));
		assert(pn);
		return pn;
	}
/ *?	if (!pSelf->nameless() && ProjectInfo_t::IsTypeImp(pSelf))
	{
		const std::string &s(demangledName(pSelf->name()->c_str()));
		if (!s.empty())
			return s;
	}* /
	return Project_t::displayName(pSelf, iModule);
}*/

MyString ProjectEx_t::typeName(CTypeBasePtr pSelf) const
{
	ProjectInfo_t PI(*this);
	if (pSelf->objGlob())
	{
		CFieldPtr pField(DcInfo_t::DockField(pSelf->objGlob()));
		if (pField)//can be intrinsic
			return PI.FieldName(pField);
	}
	if (pSelf->hasUglyName() && pSelf->typeStruc())
	{
		if (pSelf->typeStrucvar())
		{
			MyString s;
			pSelf->namex(s);
			return s;//struc vars have ugly names (~)
		}
		TypePtr iModule(nullptr);
		if (pSelf->typeFuncDef())
			iModule = ProjectInfo_t::ModuleOf(DcInfo_t::DockField((CGlobPtr)pSelf));
		if (!iModule)
		{
			TypePtr iScope(pSelf->owner());
			if (!iScope)
				iScope = DcInfo_t::OwnerScope(pSelf->objType());
			assert(iScope);
			iModule = PI.ModuleOf(iScope);
		}
		if (iModule && DCREF(iModule))
		{
			DcInfo_t DI(*DCREF(iModule));
			PNameRef pn(DI.FindPrettyName(pSelf->objType()));
			if (pn)
				return pn->c_str();
		}
		return PI.StrucNameless(pSelf);
	}
	return Project_t::typeName(pSelf);
}

void ProjectEx_t::typeNameScoped(CTypeBasePtr pSelf, char chopSymb, FullName_t &v) const
{
	if (pSelf && !pSelf->typeSeg())
	{
		//assert(pSelf->typeStruc() && (!pSelf->nameless() || pSelf->has UglyName()));
		//if (pSelf->objGlob())
			//pSelf = pSelf->owner();
		typeNameScoped(pSelf->owner(), chopSymb, v);
		ProjectInfo_t PI(*this);
		v.append(PI.TypeName0((TypePtr)pSelf, chopSymb));//no pretty names!
		//s.append("::");
	}
}

MyString ProjectEx_t::fieldName(CFieldPtr pSelf) const
{
	if (DcInfo_t::IsGlobal(pSelf))
	{
		CGlobPtr iGlob(DcInfo_t::GlobObj(pSelf));
		if (iGlob && iGlob->hasPrettyName())
		{
			TypePtr iModule(ProjectInfo_t::ModuleOf(pSelf));
			if (iModule && DCREF(iModule))
			{
				DcInfo_t DI(*DCREF(iModule));
				return DI.GlobPrettyName(iGlob);
			}
		}
	}

	return Project_t::fieldName(pSelf);
}

MyString DcInfo_t::GlobPrettyName(CGlobPtr iGlob)
{
	PNameRef pn(FindPrettyName(iGlob));
	if (pn)
		return pn->c_str();
	return "";
}


MyString ProjectEx_t::fieldDisplayName(CFieldPtr pSelf) const
{
	if (!pSelf->nameless())
	{
		int n;
		const char* pc(NameRef_t::skipLenPfx(pSelf->name()->c_str(), n));
		if (n < 0)
			return pc;
		if (n > 0)
			return MyString(pc, n);
		STOP
	}

	if (FuncInfo_s::IsLocal(pSelf))
	{
		GlobPtr pGlob(DcInfo_t::GetLocalOwner(pSelf));
		if (pGlob)
		{
			TypePtr iModule(ProjectInfo_t::ModuleOf(pGlob->folder()));
			assert(iModule);
			{
				Dc_t *pDC(DCREF(iModule));
				assert(pDC);
				if (ProtoInfo_t::IsStub(pGlob))
				{
					ProtoInfo_t FI(*pDC, pGlob);
					return FI.LocalName(pSelf);
				}
				else
				{
					FuncInfo_t FI(*pDC, *pGlob);
					return FI.LocalName(pSelf);
				}
			}
		}
	}
	else if (pSelf->isGlobal())
	{
		TypePtr iModule(ProjectInfo_t::ModuleOf(pSelf->owner()));
		if (iModule)
		{
			Dc_t* pDC(DCREF(iModule));
			if (pDC)
			{
				DcInfo_t DI(*pDC);
				if (pSelf->isTypeImp())//global
				{
					CFieldPtr pExpField(DI.ToExportedField(pSelf));
					if (pExpField)
						return DI.AutoName(pExpField, pSelf);
				}
				else
				{
					CGlobPtr pGlob(DcInfo_t::GlobObj(pSelf));
					if (pGlob && pGlob->hasPrettyName())
					{
						PNameRef pn(DI.FindPrettyName(pGlob));
						return pn->c_str();
					}
				}
			}
		}
	}

	ProjectInfo_t PI(*(Project_t *)this);

	MyString s;
	if (PI.LabelName(pSelf, s))
		return s;//chop?

	return PI.AutoName(pSelf, nullptr);//needs not to be chopped
	//return Project_t::fieldDisplayName(pSelf);
}

MyString ProjectEx_t::typeDisplayName(CTypePtr pType) const
{
	if (pType->typeFuncDef())
	{
		CGlobPtr pGlob((CGlobPtr)pType);
		if (pGlob->hasPrettyName())
		{
			TypePtr iModule(ProjectInfo_t::ModuleOf(pGlob->folder()));
			DcInfo_t DI(*DCREF(iModule));
			PNameRef pn(DI.FindPrettyName(pGlob));
			if (pn)
				return pn->c_str();
		}
		FieldPtr pDockField(DcInfo_t::DockField(pGlob));
		assert(pDockField);
		return fieldDisplayName(pDockField);
	}
	return Project_t::typeDisplayName(pType);
}

bool ProjectEx_t::getCellStr(int col, CTypePtr pScope, ADDR _key, unsigned sz, MyString& s) const
{
	if (!pScope->typeFuncDef())
		return false;

	TypePtr iModule(DcInfo_t::ModuleOfEx(pScope));
	if (!iModule)
		return false;

	SSID_t ssid(FuncInfo_s::SSIDx(_key));
	if (col == adcui::IBinViewModel::CLMN_OFFS)
	{
		ADDR va(FuncInfo_s::address(_key));
		if (ssid != SSID_NULL)
		{
			if (ssid != SSID_LOCAL)//IsLocalReg?
			{
				Dc_t* pDC(DCREF(iModule));
				if (pDC)
				{

					DcInfo_t DI(*pDC);
					s = DI.RegToString(ssid, REG_t(va, sz));
				}
			}
			else// isLocalArg?
			{
				s = "+";
				s.append(Int2Str(va, I2S_HEXA));
			}
			s += '\n';//finalize cell (no more to be appended)
			return true;
		}
	}
	else if (col == adcui::IBinViewModel::CLMN_DATA)
	{
		if (ssid != SSID_NULL)
		{
			if (ssid != SSID_LOCAL)//IsLocalReg?
			{
				s += '\n';
				return true;//finalize
			}
		}
	}

	return false;
}

TypePtr ProjectEx_t::objectDisplayScope(CObjPtr pObj) const
{
	FieldPtr pField(pObj->objField());
	if (pField && !FuncInfo_s::IsLocal(pField))
	{
		bool bUglyName(pField->hasUglyName());
		if (bUglyName && pField->isExported() && !pField->name()->name())
			bUglyName = false;

		if (!bUglyName)
		{
			CTypePtr pScope(nullptr);
			if (DcInfo_s::IsGlobal(pField))
			{
				CGlobPtr pGlob(DcInfo_s::GlobObj(pField));
				if (pGlob)
					pScope = DcInfo_s::OwnerScope(pGlob);
			}
			else
				pScope = pField->owner();
			if (pScope)
				return (TypePtr)pScope;
		}
	}
	return Project_t::objectDisplayScope(pObj);
}

MyString ProjectEx_t::tipName(CFieldPtr pSelf, CTypePtr iModule) const
{
	if (!iModule)
	{
		//ProjectInfo_t PI(*(Project_t *)this);
		iModule = ProjectInfo_t::ModuleOf(pSelf->owner());
		if (!iModule)
			return "";
	}
	const Dc_t *pDC(DCREF(iModule));
	if (pDC)
	{
		DcInfo_t DI(*pDC);

		if (DcInfo_s::IsGlobal(pSelf))
		{
			CGlobPtr pGlob(DcInfo_s::GlobObj(pSelf));
			if (pGlob)
			{
				FileInfo_t FI(DI, *pGlob->folder()->fileDef());
				MyString s(FI.DumpFieldDeclaration(pSelf));
				if (!s.empty())
					return s;
			}
			//return TypeName(pfSelf);
		}

		//if (pSelf->hasUgly Name())
	{
		//PNameRef pn(DI.FindPrettyName(pSelf));
		//if (pn)
		//return pn->c_str();

		if (!pSelf->nameless())//the pretty name may not exist
		{
			TypePtr iSeg(DI.PrimeSeg());
			FRONT_t *pFRONT(DI.FrontOf(iSeg));
			if (pFRONT)//try to demangle
			{
				MyStream ss;
				pFRONT->device(DI.GetDataSource())->demangleName(pSelf->name()->c_str(), ss);
				MyString s;
				if (ss.ReadString(s))
					return s;
			}
		}
	}
	}

	return Project_t::tipName(pSelf, iModule);
}

MyString ProjectEx_t::tipName(CTypePtr pSelf, CTypePtr iModule) const
{
	return Project_t::tipName(pSelf, iModule);
}

FieldPtr ProjectEx_t::findGlobal(const char *name, TypePtr iModule) const
{
	Dc_t* pDC(DCREF(iModule));
	if (pDC)
	{
		DcInfo_t DI(*pDC);
		return DI.FindTemplatedField(name);
	}
	ProjectInfo_t PI(*this);
	TypePtr pSeg(ProjectInfo_t::FindFrontSegIn(iModule));
	return PI.FindFieldByName(name, pSeg);
}

ObjPtr ProjectEx_t::findObject(const char* pName, TypePtr pModule)
{
	ObjPtr pObj(Project_t::findObject(pName, pModule));
	if (!pObj)
	{
		const Dc_t* pDC(DCREF(pModule));
		if (pDC)
		{
			DcInfo_t DI(*pDC);
			return DI.FindObjByScopedNameEx(pName, DI.PrimeSeg());
			//pObj = DI.CheckGlob(pObj);
		}
	}
	return pObj;
}

/*Dc_t *ProjectInfoEx_t::FindDcFromSeg(TypePtr iSeg) const
{
	Seg_t &rSeg(*iSeg->typeSeg());
	for (SubSegMapCIt i(rSeg.subsegs().begin()); i != rSeg.subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));//look only in real segments (not seg traces)
		Dc_t *pDC(FindDcFromSeg(iSeg));
		if (pDC)
			return pDC;
	}
	TypePtr iDC(rSeg.UserData<TypeObj_t>());
	if (!iDC)
		return nullptr;

	assert(rSeg.frontEnd());
	return iDC;
}*/


FieldPtr ProjectEx_t::toExportedField(CFieldPtr pImpField) const
{
	assert((pImpField->isTypeImp() || pImpField->ID() == 0) || pImpField->isTypeExp());
//CHECKID(pImpField, 2915)
//STOP

	FieldPtr pField;
	const char* pc(pImpField->name()->c_str());
	if (pImpField->isTypeExp())
	{
		MyStringEx aName(pc);
		MyString sName(FromCompoundName0(aName, 1));//no tag
		pField = exportPool().findPrefix(sName);
	}
	else if (!pImpField->name()->name())//imported by ordinal only?
	{
		pField = exportPool().findSuffix(pc);
	}
	else
	{
		pField = exportPool().find(pc, false);
		if (!pField)//no exact match; maybe ordinals are inconsistent; try by pure name
		{
			const char* pc2(NameRef_t::skipToTag(pc));
			if (pc2)
			{
				MyString s(pc, pc2 - pc);
				pField = exportPool().findPrefix(s);
			}
		}
	}
	return pField;
}

bool ProjectEx_t::addCutList(CObjPtr pObj)
{
	for (size_t i(0); i < mCutList.size(); i++)
		if (mCutList[i] == pObj)
			return false;
	mCutList.push_back((ObjPtr)pObj);
	return true;
}

ObjPtr ProjectEx_t::takeCutListItem(size_t i)
{
	if (i < mCutList.size())
	{
		auto it(mCutList.begin());
		std::advance(it, i);
		assert(it != mCutList.end());
		ObjPtr pObj(*it);
		mCutList.erase(it);
		return pObj;
	}
	return nullptr;
}

const IGuiEx_t &ProjectEx_t::guix() const
{
	return mainx().guix();
}

void ProjectEx_t::dispatchDirty() const
{
	Project_t::dispatchDirty();

	if (checkDirty(DIRTY_CUR_FOLDER))
		gui().GuiOnCurFolderChanged();
	if (checkDirty(DIRTY_CUR_FUNC))
		gui().GuiOnCurFuncChanged();
	if (checkDirty(DIRTY_CUR_OP))
		gui().GuiOnCurOpChanged();
	if (checkDirty(DIRTY_SEL_OBJ))
		gui().GuiOnSelObjChanged();
}

/*ProbeEx_t *ProjectEx_t::getContextEx()
{
	ProbeEx_t *pCtx2(nullptr);
	Probe_t *pCtx(getContext());
	if (pCtx)
	{
		pCtx2 = dynamic_cast<ProbeEx_t *>(pCtx);
		if (!pCtx2)
			releaseContext(pCtx);
	}
	return pCtx2;
}*/

Probe_t *ProjectEx_t::NewLocus(const Locus_t &aLoc)
{
	ProbeEx_t *pCtx(new ProbeEx_t);
	pCtx->locus().assign(aLoc);
	//pCtx->setObj(pObj);
	return pCtx;
}

/*TypePtr ProjectEx_t::currrentVA(ADDR &va) const
{
	return Project_t::currrentVA(va);
}*/

bool ProjectEx_t::OnReleaseTypeRef(TypePtr pType)
{
	assert(pType->refsNum() == 0);

CHECKID(pType, -4)
STOP
	if (USERFOLDER(pType))
	{
		FolderPtr pFolder(USERFOLDER(pType));

		FileDef_t *pFileDef(DcInfo_t::FILEDEF(pFolder));
		Dc_t *pDcRef2(DcInfo_t::DcFromFolder(*pFolder));//!
		//Dc_t *pDcRef(DCREF(iModule));
		//assert(pDcRef == pDcRef2);
		FileInfo_t fileInfo(*pDcRef2, *pFileDef);
		fileInfo.TakeType(pType);
	}

	/*MemoryMgr_t &rMemMgr(pDcRef->memMgr());
	DcInfo_t DH(*pDcRef, rMemMgr);
	DcCleaner_t DC(DH);
	DC.destroyTypeRef(*pType, true);
	rMemMgr.Delete(pType);*/
	return true;
}

TypePtr ProjectEx_t::GetNameOwnerOf(TypePtr pCplx) const
{
	while (pCplx)
	{
		if (pCplx->typeComplex()->namesMgr())
			return pCplx;
		Seg_t *pSeg(pCplx->typeSeg());
		if (pSeg)
		{
			/*if (pSeg->traceLink())
			{
				pCplx = pSeg->traceLink();
				continue;
			}
			else*/ if (pSeg->superLink())
			{
				pCplx = pSeg->superLink();
				continue;
			}
			else
				ASSERT0;
		}
		if (pCplx->typeFuncDef())
			return pCplx;
#if(NEW_LOCAL_VARS)
		if (pCplx->typeStrucLoc())
			return pCplx->owner();
#else
		if (pCplx->typeUnion())
		{
			TypePtr pCplx2(FileInfo_t::IsLocalsTop(pCplx));
			if (pCplx2)
				return pCplx2;
		}
#endif
		FieldPtr pField = pCplx->parentField();
		if (pField)
			pCplx = pField->OwnerComplex();
	}
	return nullptr;
}

TypePtr ProjectEx_t::GetNameOwnerOf(FieldPtr pField) const
{
/*	if (pField->userDataType() > FUDT_ LOCAL)
	//if (pField->owner()->typeSeg())//global
	{
		TypePtr iScope(DcInfo_t::OwnerScope(pField));
		if (iScope)
			return GetNameOwnerOf(iScope);
	}*/
	return Project_t::GetNameOwnerOf(pField);
}

void ProjectEx_t::OnShowTaskTop()
{
	MyString fileName;
	IAnalyzer *pIAnlz(analyzer());
	if (pIAnlz)
	{
		Folder_t *pFolder(pIAnlz->currentFile());
		if (pFolder)
			fileName = files().relPath(pFolder) + SOURCE_EXT;
	}
	gui().GuiShowFile(fileName.c_str(), "$task", false);
//?	gui().GuiShowFile("?STUBS", "$task", false);//open stubs view at task's top
}

static bool IsDigit(char c)
{
	return ('0' <= c && c <= '9');
}

static size_t TailingDecimalAt(const MyString &s)//no leading 0's, but a single '0' is allowed as a number
{
	size_t i(s.length());
	if (i < 2)
		return 0;
	for (; i > 0; i--)
		if (!IsDigit(s[i - 1]))
			break;
	if (i == s.length())
		return 0;
	if (s[i] == '0' && i != s.length() - 1)//must be the last if 0
		return 0;//invalid
	return i;
}

static size_t Tailing0HEXhAt(const MyString &s)//uppercase, a leading single '0' iff next is a alpha, a tailing 'h'
{
	size_t i(s.length());
	if (i < 3)
		return 0;
	if (s[i - 1] != 'h')
		return 0;
	i--;
	for (; i > 0; i--)
	{
		char c(s[i - 1]);
		if (IsDigit(c))
			continue;
		if ('A' <= c && c <= 'F')
			continue;
		break;
	}
	if (i == s.length() - 1)
		return 0;
	if (!IsDigit(s[i]))//must start with a number
		return 0;
	if (s[i] == '0' && IsDigit(s[i + 1]))//must be followed by an alpha if starts with a zero
		return 0;
	return i;
}

static bool IsAlphaLow(char c)
{
	return ('a' <= c && c <= 'z');
}

static bool IsLocalAutoPrefix(const std::string &s)
{
	size_t l(s.length());
	assert(l > 0);
	if (IsAlphaLow(s[0]))
	{
		if (s[0] == 'f' && l == 1)
			return true;//fpu arg
		if (s[0] == 'g' && (l > 1) && IsAlphaLow(s[1]))
			return true;//fpu var
	}
	return false;
}

static bool IsFpuRegAutoname(const std::string &s)
{
	assert(!s.empty());
	if (s.length() == 2)
	{
		if (*s.begin() == 'f')
			if ('0' <= *s.end() && s.back() < '8')
				return true;//arg
	}
	else if (s.length() == 3)
	{
		if (*s.begin() == 'g')
			if (IsAlphaLow(s[1]))
				if ('1' <= s[2] && s.back() <= '8')
					return true;
	}
	return false;
}

static bool IsAuxRegAutoname(const std::string &s)
{
	size_t l(s.length());
	assert(l > 0);
	if (IsAlphaLow(s[0]))
	{
		if (s[0] == 't' && (l > 1) && IsAlphaLow(s[1]))
			return true;//fpu var
	}
	return false;
}

bool ProjectEx_t::isAutoname(const MyString &s, TypePtr iModule) const
{
	if (Project_t::isAutoname(s, iModule))
		return true;//do not permit user's identifiers with global prefixes

	if (IsFpuRegAutoname(s))
		return true;

	size_t n(TailingDecimalAt(s));
	if (n > 0)
	{
		MyString s2(s.substr(0, n));
		Dc_t *pDC(DCREF(iModule));
		if (!pDC)
			return false;
		assert(pDC->frontDC());
		return pDC->hasReg(s2);
	}

	n = Tailing0HEXhAt(s);
	if (n > 0)
		return IsLocalAutoPrefix(s.substr(0, n));

	if (IsAuxRegAutoname(s))
		return true;

	return false;
}

uint8_t ProjectEx_t::dumpLineStatus(CTypePtr iStruc) const
{
	if (iStruc->typeProc())
	{
		GlobPtr iGlob(DcInfo_s::GlobFuncObj(iStruc->parentField()));
		if (iGlob)
		{
			if (ProtoInfo_t::IsFuncStatusAborted(iGlob))
				return adcui::ITEM_FUNC_DECOMPILING;
			if (ProtoInfo_t::IsFuncStatusFinished(iGlob))
				return adcui::ITEM_FUNC_DECOMPILED;
		}
	}
	return Project_t::dumpLineStatus(iStruc);
}

void ProjectEx_t::OnDataEdited(TypePtr iModule)
{
	Project_t::OnDataEdited(iModule);
	Dc_t *pDC(DCREF(iModule));
	if (pDC)
	{
		//ProbeEx_t ctx;
		DcInfo_t dcInfo(*pDC);
		dcInfo.redump();// ctx, REDUMP_ALL);//TODO: NEED REDUMP EXPRESSIONS ONLY? (but data declarations also change)
		//gui().GuiOnExprModified();
	}
}

bool ProjectEx_t::ExecuteCommand(Cmd_t &cmd, int *pret)
{
#ifdef _DEBUG
	MyString sCommand(cmd.AsString());
#endif
	static ProjExCmdMap_t cmdMap;
	ProjExCmdServer_t cmdSvr(cmdMap, ProjectInfo_t(*this));
	if (!cmdSvr.ExecuteCommand(cmd))
		return false;
	*pret = cmdSvr.mRet;
	return true;
}


bool ProjectEx_t::deleteField(Locus_t& aLoc)
{
	Frame_t &aTop(aLoc.back());
	FieldPtr pField(aTop.field());
	if (DcInfo_t::IsGlobal(pField))
	{
		GlobPtr pGlob(DcInfo_t::GlobObj(pField));
		if (pGlob)
		{
			TypePtr pModule(DcInfo_t::ModuleOf(pField));
			DcInfo_t DI(*DCREF(pModule));
			FolderPtr pFolder(pGlob->folder());
			FileInfo_t GI(DI, *pFolder->fileDef());
			FieldPtr pField(GI.RecallGlob(pGlob));
			if (!pField)
				return false;
			aLoc.setField(pField);//update locus
			DI.redump(pFolder);//!!!
		}
	}
	return Project_t::deleteField(aLoc);
}


bool ProjectEx_t::deleteBogusField(FieldPtr pField, NamesMgr_t *pNS)
{
	assert(DcInfo_t::IsGlobal(pField));
	GlobPtr pGlob(DcInfo_t::GlobObj(pField));
	if (pGlob)
	{
		//remove from SRC file
		TypePtr pModule(ProjectInfo_t::ModuleOf(pField));
		Dc_t* pDC(DCREF(pModule));
		FileInfo_t GI(*pDC, *pGlob->folder()->fileDef());
		GI.RecallGlob(pGlob);
	}
	return Project_t::deleteBogusField(pField, pNS);
}

//Commands


bool ProjExCmdServer_t::ExecuteCommand(Cmd_t &cmd)
{
	if (cmd.context())
	{
		/*if (!cmd.context())
		{
			Probe_t *pCtx(mrProject.getContext());
			assert(pCtx);
			cmd.setContext(pCtx);
			mrProject.releaseContext(pCtx);
		}*/

		FileDef_t* pFileDef(nullptr);
		ProbeEx_t* pCtx(dynamic_cast<ProbeEx_t*>(cmd.context()));
		if (pCtx && pCtx->folder())
			pFileDef = pCtx->folder()->fileDef();

		if (pFileDef)
		{
			TypePtr pModule(ModuleOf(pCtx->folder()));
			Dc_t* pDC(DCREF(pModule));
			if (!pDC)
				return false;
			static FileInfoCmdServer_t::CommandMap_t cmdMap;
			FileInfo_t fileInfo(*pDC, *pFileDef);
			FileInfoCmdServer_t cmdSvr(cmdMap, fileInfo);
			int iRet(cmdSvr.ExecuteCommand(cmd));
			if (iRet)
			{
				mRet = cmdSvr.mRet;//borrow the return code
				if (mRet >= 0)//failed or succeded
					return true;
			}
		}
	}

	return ProjCmdServer_t::ExecuteCommand(cmd);
}

template <typename T>
std::ostream& printid(std::ostream &os, T p)
{
#if(!NO_OBJ_ID)
	os << "(id=" << std::dec << p->ID() << ")";
#endif
	return os;
}

void ProjectEx_t::PrintObjInfo(std::ostream &os, ObjPtr pObj, TypePtr iModule0)//objinfo
{
	ProjectInfo_t PI(*this);
	CFieldPtr pField(pObj->objField());
	if (pField)
	{
		CTypePtr pModule(DcInfo_s::ModuleOfEx(pField->owner()));
		CGlobPtr iGlob(DcInfo_t::IsGlobal(pField) ? DcInfo_t::GlobObj(pField) : nullptr);
		if (iGlob)
		{
			os << "glob";
			printid(os, iGlob) << " => ";
		}

		os << "field";
		printid(os, pField);

		if (DcInfo_t::IsGlobal(pField))
		{
			/*if (pField->isClone())
			{
				os << " => master";
				CFieldPtr pMaster(ProjectInfo_t::CloneLead(pField));
				printid(os, pMaster);
			}*/

			if (pField->isTypeProc())
			{
				CFieldPtr pEntry(ProjectInfo_t::EntryField(pField->type()));
				if (pEntry)
				{
					os << " => label";
					printid(os, pEntry);
				}
			}

			os << ":\n\tVA: " << std::hex << pField->_key();

			os << "\n\tName: " << PI.FieldName0(pField, CHOP_SYMB);

			if (iGlob)
			{
				if (iGlob->hasPrettyName())
				{
					MyString s;
					DcInfo_t DI(*DCREF(pModule));
					ProjectInfo_t::ChopName(DI.GlobPrettyName(iGlob), s, CHOP_SYMB);
					os << "\n\tPrettyName: " << s;
				}
				FolderPtr pFolder(DcInfo_t::FolderOf(iGlob));
				assert(pFolder);
				os << "\n\tFile: " << files().relPath(pFolder);
			}

			if (pField->type())
			{
				os << "\n\tType: " << PI.TypeName(pField->type());
				printid(os, pField->type());
				os << ", size = " << std::hex << pField->size();
			}
			if (!iGlob && pModule)
				os << "\n\tModule: " << ProjectInfo_s::ModuleTitle(pModule);
		}
		else
		{
			os << ":\n\tOffset: ";
			int off(pField->_key());
			if (pField->owner()->typeBitset())
			{
				off = pField->owner()->parentField()->_key();
				os << std::hex << off << ":" << pField->_key();
			}
			else
			{
				if (FuncInfo_s::IsLocal(pField))
					off = FuncInfo_s::address(pField);
				if (off < 0)
				{
					os << "-";
					off = -off;
				}
				os << std::hex << off << " (key=" << pField->_key() << ")";
			}
			os << "\n\tName: " << PI.FieldName(pField, CHOP_SYMB);
			os << "\n\tSize: " << std::hex << pField->size();

			TypePtr iModule(DcInfo_t::ModuleOfEx(pField->owner()));
			if (!iModule && FuncInfo_t::IsLocal(pField))
			{
				GlobPtr iCFunc(DcInfo_t::GetLocalOwner(pField));
				if (iCFunc)
				{
					DcInfo_t DI(*DCREF(pModule));
					os << "\n\tLocal owner: " << DI.GlobNameFull(iCFunc, DcInfo_t::E_PRETTY);
					printid(os, iCFunc);
				}
			}
			if (iModule)
				os << "\n\tModule: " << PI.TypeName(iModule);
		}
	}
	else
	{
		TypePtr pType(pObj->objTypeGlob());
		assert(pType);
		TypePtr pProxy(pType->typeProxy() ? pType : nullptr);
		if (!pProxy)
		{
			TypePtr iModule2(DcInfo_t::ModuleOfEx(pType));
			if (iModule0 && iModule2 != iModule0)//maybe a proxy?
			{
				Dc_t* pDC(DCREF(iModule0));
				if (pDC)
				{
					DcInfo_t DC(*pDC);
					pProxy = DC.FindProxyOf(pType);
					if (pProxy)
						pType = pProxy;
				}
			}
		}

		assert(pType);
		if (pType->typeProxy())//ObjType() == OBJID_TYPE_PROXY)
		{
			os << "proxy";
			printid(os, pType);
			os << " => ";
			pType = pType->typeProxy()->incumbent();
		}
		CTypePtr pTypedef(nullptr);
		if (pType->typeTypedef())
		{
			pTypedef = pType;
			os << "typedef";
			printid(os, pType);
			os << " => ";
			pType = pType->typeTypedef()->baseType();
		}
		if (pType)//could be a typedef to VOID?
		{
			os << PI.TypeName0(pType, CHOP_SYMB);
			if (pProxy)
				os << "\n\tProxy Name: " << PI.TypeName0(pProxy, CHOP_SYMB);
			if (pTypedef)
				os << "\n\tTypedef Name: " << PI.TypeName0(pTypedef, CHOP_SYMB);
			printid(os, pType);
			os << "\n\tSize: " << std::hex << pType->size();
			if (!pType->owner()->typeSeg())
				os << "\n\tOwner: " << PI.TypeName(pType->owner());
			FolderPtr pFolder(USERFOLDER(pType));
			if (pFolder)
				os << "n\tFile: " << files().relPath(pFolder);
			else if (pType->hasUserData())
				os << "\n(userdata)";
		}
		else
			os << "?";
	}
	os << std::endl;
}

const char *ProjectEx_t::autoPrefix(CFieldPtr pSelf, CTypePtr pType) const
{
	if (DcInfo_s::IsGlobal(pSelf))
	{
		CGlobPtr iGlob(DcInfo_t::GlobObj(pSelf));
		if (iGlob)
		{
			if (iGlob->func())
				return "func";
			if (DcInfo_s::DockField(iGlob)->isTypeThunk())
				return "thunk";
		}
	}
	return Project_t::autoPrefix(pSelf, pType);
}

void ProjectEx_t::printMemInfo(std::ostream& os) const
{
	Project_t::printMemInfo(os);
	print_obj_stats();
	print_ops_stats();
}

Decompiler_t* ProjectEx_t::decompiler() const
{
	return dynamic_cast<Decompiler_t*>(analyzer());
}


#define MYCOMMAND_IMPL(name) \
	int ProjExCmdServer_t::COMMAND_##name(ProjExCmdServer_t *pSelf, Cmd_t &args){ return pSelf->OnCommand_##name(args); } \
	int ProjExCmdServer_t::OnCommand_##name(Cmd_t &args)


void ProjectEx_t::CollectSymbols()
{
	const FoldersMap &m(rootFolder().children());
	for (FoldersMap::const_iterator it(m.begin()); it != m.end(); it++)
	{
		CFolderRef &rFolder(*it);
		if (!ProjectInfo_t::IsPhantomFolder(rFolder))
		{
			TypePtr iModule(ProjectInfo_t::ModuleOf(&rFolder));
			if (DCREF(iModule))
			{
				DcInfo_t DI(*DCREF(iModule));
				//re-aquire cxx symbols here
				DI.CollectExportedSymbols();
				DI.CollectDebugSymbols();
				DI.CollectImportedSymbols();
			}
		}
	}
}

//////////////////////////////////////////////
// Synopsis: $dumpfunc <object name>
MYCOMMAND_IMPL(dumpfunc)
{
	DECLARE_DC_CONTEXT(ctx);
	if (args.size() > 1)
	{
		FieldPtr pField(FindFieldByNameInSegs3(ctx.locus().module(), args[1].c_str()));
		if (pField && pField->isTypeProc())
		{
			GlobPtr ifDef(DcInfo_s::GlobFuncObj(pField));
			if (ifDef)
			{
				assert(0);/*
				   FileInfo_t dcInfo(*projx().DcRef());
				   FuncInfo_t FI(*dcInfo.dcRef(), *ifDef, dcInfo.FindFileDefOf(*ifDef->typeFuncDef()));
				   FI.dump(*ifDef->typeFuncDef(), std::cout);
				   return 1;*/
			}
		}
	}
	return 0;
}

/*ProbeEx_t *ProjectInfoEx_t::context() const
{
	if (!mrProject.hasContext())
		return nullptr;
	return dynamic_cast<ProbeEx_t *>(&mrProject.context()); 
}*/

bool FuncInfo_t::LocusFromOp(CHOP pOp0, Locus_t &aLoc) const
{
	if (!pOp0)
		return false;
	HOP pOp(pOp0);
	/*for (;;)
	{
		HOP pOpNx(NextPrime(pOp));
		if (!pOpNx || !IsAuxOp(pOpNx))
			break;
		pOp = pOpNx;
	}*/
	ADDR va(OpVA(pOp));
	if (va == -1)
		return 0;
	return LocusFromVA_2(OwnerProc(), va, aLoc);
}


//////////////////////////////////////////////
// Synopsis: $decompile [-src <src file path>]
MYCOMMAND_IMPL(decompile)
{
	DECLARE_DC_CONTEXT(ctx);
	Cmd_t args2(args);

	if (!ctx.dcRef())
		return 0;

	assert(ctx.folder());

	if (!ctx.folder()->fileDef())
	{
		MyString s;
		if (args2.RemoveOptEx("-src", s))
		{
			ZPath_t zPath(s);
			AssurePathAbsolute(zPath, ctx.moduleFromLocus());
			if (!zPath.empty())
			{
				adcui::FolderTypeEnum iKind;
				FolderPtr pFolder(mrProject.files().FindFileByPath(zPath.toString(), iKind));
				if (pFolder)
					ctx.setFolder(pFolder);
			}
		}
		else if (projx().guix().GuiOnNoSourceContext(args2.AsString()))//try a recoil
			return 0;//ok

		if (!ctx.folder()->fileDef())
		{
			PrintError() << "Operation must be carried out in context of a source file" << std::endl;
			return 0;
		}
	}

	DcInfo_t dcInfo(*ctx.dcRef());

/*	if (mrProject.files().IsModule(*ctx.folder()))
	{
		Folder_t *pFolder(AddFile(fixFile Name("src/?", ctx.moduleFromLocus())));
		assert(pFolder);
		if (!pFolder->hasPvt())
			pFolder->SetPvt(new File_t);
		//?		context().setFolder(pFolder);
		ctx.setFolder(pFolder);

		MyString s(Project().files().relPath(pFolder));
		fprintf(stdout, "A new source file created: %s ...\n", s.c_str());
	}

	if (!ctx.folder()->fileDef())//the binary has been converted into the source?
	{
		dcInfo.AssureFileDef(ctx.folder());
		MyString s(Project().files().relPath(ctx.folder()));
		gui().GuiOnFileRenamed(s, s + SOURCE_EXT);//file name changed
		gui().GuiOnFileListChanged();
	}*/

	FileInfo_t fileInfo(dcInfo, *ctx.fileDef());
	CMDServerCommandMap cmdMap;
	FileInfoCmdServer_t cmdSvr(cmdMap, fileInfo);
	
	return FileInfoCmdServer_t::COMMAND_decompile(&cmdSvr, args2);
}

// Synopsis: newfile [path]
MYCOMMAND_IMPL(newfile)
{
	DECLARE_DC_CONTEXT(ctx);
	//MyArgs2 a(args);

	int i;
	bool bRecoil(false);
	if ((i = args.Find("-r")) >= 0)
	{
		args.RemoveAt(i);
		bRecoil = true;
	}

	//ProjectInfoEx_t proInfo(mrProject);
	Folder_t *pFolder(nullptr);
	if (args.size() > 1)
		pFolder = AddFile(fixFileName(args[1], ctx.moduleFromLocus()));
	else
	{
		//if (a.size() > 2 && a[2] == "-h")
		//	mFiles.AddFile("src/?", FTYP_HEADER);
		//else
		pFolder = AddFile(fixFileName("?", ctx.moduleFromLocus()));
	}
	if (!pFolder)
		return 0;

	if (!pFolder->hasPvt())
		pFolder->SetPvt(new File_t);

	//?	context().setFolder(pFolder);
	ctx.setFolder(pFolder);

	if (bRecoil)
	{
		MyString s(Project().files().relPath(pFolder));
		gui().GuiNewFolderRecoil(s);
	}

	return 1;
}

// Command: newsrc
// Desc: Create new virtual file in current module.
//			If `v-path` not provided, a name is auto-generated.
// Synopsis: newsrc [-r] [v-path]
//		-r : recoil request
//		v-path: virtual path to create a file at
MYCOMMAND_IMPL(newsrc)
{
	DECLARE_DC_CONTEXT(ctx);
	//MyArgs2 a(args);

	int i;
	bool bRecoil(false);
	if ((i = args.Find("-r")) >= 0)
	{
		args.RemoveAt(i);
		bRecoil = true;
	}

	FolderPtr pFolder(nullptr);
	if (args.size() > 1)
		pFolder = AddFile(fixFileName(args[1], ctx.moduleFromLocus()));
	else
		pFolder = AddFile(fixFileName("?", ctx.moduleFromLocus()));

	if (!pFolder)
		return 0;

	MyString sFile(FilesMgr0_t::relPath(pFolder));
	if (!pFolder->hasPvt())
	{
		DcInfo_t::AssureFileDef(pFolder);
		PrintInfo() << "A new source file created: " << sFile << std::endl;;
	}
	else
		PrintWarning() <<  "A source file exists: " << sFile << std::endl;

	ctx.setFolder(pFolder);

	if (bRecoil)
	{
		gui().GuiNewFolderRecoil(sFile);
	}

	return 1;
}

// Coomand: select_source_file
// Alias: selsrc
// Desc: Select (switch to) an existing virtual file at path `v-path`
// Synopsis: selsrc <v-path>
MYCOMMAND_IMPL(select_source_file)
{
	DECLARE_DC_CONTEXT(ctx);

	MyArgs2 a(args);
	if (a.size() < 2)
	{
		PrintError() << "Missing argument for command `" << args[0] << "`" << std::endl;
		return 0;
	}

	DcInfo_t DI(*DCREF(ctx.locus().module()));

	FolderPtr pVFile(DI.FindFileByStem(a[1]));
	if (!pVFile || !pVFile->fileDef())
	{
		PrintError() << "Virtual path does not exist: `" << a[1] << "`" << std::endl;
		return 0;
	}

	ctx.setFolder(pVFile);
	return 1;
}

// Synopsis: delfile <path>
MYCOMMAND_IMPL(delfile)
{
	DECLARE_DC_CONTEXT(ctx);

	MyArgs2 a(args);
	if (a.size() > 1)
	{
		MyString filePath(a[1]);

		MyString s(ZPath_t::moduleOf(filePath));

	//ProjectInfo_t PI(*(Project_t *)this);
		TypePtr iFound(FindModuleEx(s, true));
		if (!iFound)
			return -1;
		Dc_t *pDC(DCREF(iFound));
		if (!pDC)
			return -1;

		DcInfo_t dcInfo(*pDC);//*ctx.dcRef());//, *ctx.fileDef());
		//ProjectInfoEx_t proInfo(mrProject);
		dcInfo.DeleteFileByName(a[1]);
	}
	return 1;
}

MYCOMMAND_IMPL(symdump)
{
	const CxxSymbMap& m(projx().cxxSymbols());
	if (m.empty())
	{
		fprintf(stdout, "Collecting symbols...\n");
		projx().CollectSymbols();
	}

	for (CxxSymbMapCIt i(m.begin()); i != m.end(); i++)
	{
		SymParsePrintFlat(i->second, std::cout);
		std::cout << std::endl;
	}
	return 1;
}

//dump types map
MYCOMMAND_IMPL(exptypdump)
{
/*?	if (projx().exp TypesMap().empty())
	{
		fprintf(stdout, "Rebuilding types map...\n");
		projx().rebuildTypesMap();
	}

	for (ExpTypesMapIt i(projx().exp TypesMap().begin()); i != projx().exp TypesMap().end(); i++)
	{
		std::cout << i->first << std::endl;
	}*/
	return 1;
}

static MyPath makeDumpPath(MyString s, const MyDirPath &dir, adcui::UDispFlags dispflags)
{
	MyPath path(s, dir);
	if (dispflags.testL(adcui::DUMP_UNFOLD))
	{
		path.SetExt(UNFOLD_EXT);
		dispflags.clearL(adcui::DUMP_HEADER);
	}
	else if (dispflags.testL(adcui::DUMP_HEADER))
		path.SetExt(HEADER_EXT);
	else
		path.SetExt(SOURCE_EXT);
	return path;
}

// Synopsis: $gensrc [options] <[outdir]>
//		Options:
//		-u - dump unfold
//		-l - line numbers
//		-b - blocks
//		-sloc - struclocs
//		-path []
//		-h [header_file] - dump ALL headers to a single header_file
//		-a [header_source] - dump ALL to a single header_source pair
//		... (more follows)

MYCOMMAND_IMPL(gensrc)
{
	DECLARE_CONTEXT(ctx);

	MyArgs2 args2(args);
	
	adcui::UDispFlags flags(adcui::DUMP_TABS | adcui::DUMP_NOREAL80);

	std::map<FolderPtr, adcui::FolderTypeEnum> lFolders;//value - is a requested file type
	MyString s;
	while (args2.RemoveOptEx("-path", s))
	{
		ZPath_t z(s);
		AssurePathAbsolute(z, ctx.moduleFromLocus());
		if (!z.empty())
		{
			adcui::FolderTypeEnum iKind;
			FolderPtr pFolder(mrProject.files().FindFileByPath(z.toString(), iKind));
			if (pFolder)
				lFolders.insert(std::make_pair(pFolder, iKind));
		}
	}

	unsigned iSingleMode(0);//1:all headers, 2:all sources, 3:all headers & sources
	MyString sH, sCpp;
	if (args2.RemoveOptEx("-h", sH))
	{
		iSingleMode = 1;
		if (sH.empty())
			sH = "untitled";
	}
	if (args2.RemoveOptEx("-a", sCpp))
	{
		iSingleMode = 3;
		if (sCpp.empty())
			sCpp = "untitled";
		if (sH.empty())
			sH = sCpp;//dump only headers, or sources & headers, never just only sources
	}

		//flags.l |= adcui::DUMP_HEADER;
	if (args2.RemoveOpt("-u"))
		flags.l |= adcui::DUMP_UNFOLD;
	if (args2.RemoveOpt("-l"))
		flags.l |= adcui::DUMP_LNUMS;
	if (args2.RemoveOpt("-b"))
		flags.l |= adcui::DUMP_BLOCKS;
	if (args2.RemoveOpt("-d"))
		flags.l |= adcui::DUMP_DEADCODE | adcui::DUMP_DEADLABELS;
	if (args2.RemoveOpt("-s"))
		flags.l |= adcui::DUMP_STACKTOP;
	if (args2.RemoveOpt("-f"))
		flags.l |= adcui::DUMP_FPUTOP;
	if (args2.RemoveOpt("-sloc"))
		flags.l |= adcui::DUMP_STRUCLOCS;

	MyDirPath fDir;
	if (args2.size() > 1)
		fDir = MyDirPath(args2[1]);
	if (fDir.empty())
		fDir = MyDirPath(Project().outputDir());

	if (fDir.empty() && iSingleMode == 0)
		return RecoilNoOutDir(args2.AsString());

	MyString sTmp(mrProject.outputDir());

	if (!fDir.empty())
		SetRootDirectory(fDir);

	int iCount(0);
	MyString sPath;//for messaging
	SrcDumpFile ofs_h, ofs_cpp;//dump whole bunch of files into a single header (or a header/source pair)
	if (!sH.empty())
	{
		MyPath f(sH, fDir);
		if (f.Ext() != HEADER_EXT)
			f.SetExt(HEADER_EXT);
		ofs_h.setPath(f);
		if (fileExists(ofs_h.path()))
			PrintWarning() << "Overwriting existing file: " << ofs_h.path() << std::endl;
		if (!ofs_h.open())
			PrintWarning() << "Could not open file: " << sH << std::endl;
		else
			sPath = ofs_h.path(), iCount++;
	}
	if (!sCpp.empty())
	{
		MyPath f(sCpp, fDir);
		if (f.Ext() != SOURCE_EXT)
			f.SetExt(SOURCE_EXT);
		ofs_cpp.setPath(f);
		if (fileExists(ofs_cpp.path()))
			PrintWarning() << "Overwriting existing file: " << ofs_cpp.path() << std::endl;
		if (!ofs_cpp.open())
			PrintWarning() << "Could not open file: " << sCpp << std::endl;
		else
			sPath = ofs_cpp.path(), iCount++;
	}

	if (!lFolders.empty())
	{
		for (std::map<FolderPtr, adcui::FolderTypeEnum>::const_iterator i(lFolders.begin()); i != lFolders.end(); ++i)
		{
			FolderPtr pFolder(i->first);
			Dc_t *pDC(DcInfo_t::DcFromFolder(*pFolder));
			if (pDC)
			{
				adcui::FolderTypeEnum iKind(i->second);
				if (pFolder->fileDef())
				{
					FileDef_t *pFileDef(pFolder->fileDef());
					if (pFileDef)
					{
						FileInfo_t FI(*pDC, *pFileDef);
						if (iKind == adcui::FOLDERTYPE_FILE_H || iKind == adcui::FOLDERTYPE_UNK)//header?
						{
							adcui::UDispFlags flags2(flags);
							if (!flags2.testL(adcui::DUMP_UNFOLD))
								flags2.l |= adcui::DUMP_HEADER;
							if (ofs_h.isSet())
								FI.DumpToFile(ofs_h, flags2);
							else if (!ofs_h.isSet())
							{
								SrcDumpFile ofs(makeDumpPath(pFolder->name(), fDir, flags2));//stripped path
								if (FI.DumpToFile(ofs, flags2))
									sPath = ofs.path(), iCount++;
							}
						}
						else
						{
							if (ofs_h.isSet())
								FI.DumpToFile(ofs_h, flags);
							else
							{
								SrcDumpFile ofs(makeDumpPath(pFolder->name(), fDir, flags));//stripped path
								if (FI.DumpToFile(ofs, flags))
									sPath = ofs.path(), iCount++;
							}
						}
					}
				}
				else//a folder selected
				{
					for (FilesMgr0_t::FolderIterator i(pFolder); i; ++i)//only subtree
					{
						CFolderRef pFolder(*i);
						FileDef_t *pFileDef(pFolder.fileDef());
						if (pFileDef)
						{
							FileInfo_t FI(*pDC, *pFileDef);
							//dump source/header pair
							if (ofs_cpp.isSet())
								FI.DumpToFile(ofs_cpp, flags);
							else if (!ofs_h.isSet())
							{
								SrcDumpFile ofs(makeDumpPath(FilesMgr0_t::realPath(&pFolder), fDir, flags));
								if (FI.DumpToFile(ofs, flags))//source
									sPath = ofs.path(), iCount++;
							}
							adcui::UDispFlags flags2(flags);
							flags2.setL(adcui::DUMP_HEADER);
							if (ofs_h.isSet())
								FI.DumpToFile(ofs_h, flags2);
							else
							{
								SrcDumpFile ofs(makeDumpPath(FilesMgr0_t::realPath(&pFolder), fDir, flags2));
								if (FI.DumpToFile(ofs, flags2))//header
									sPath = ofs.path(), iCount++;
							}
						}
					}
				}
			}//pDC
		}
	}
	else//otherwise, dump all modules
	{
		const FoldersMap &m(mrProject.rootFolder().children());
		for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
		{
			CFolderRef pModuleFolder(*i);
			Dc_t *pDC(DcInfo_t::DcFromFolder(pModuleFolder));
			int iH(0), iCpp(0);//count in module

			for (FilesMgr0_t::FolderIterator i(&pModuleFolder); i; ++i)//only subtree
			{
				CFolderRef pFolder(*i);
				FileDef_t *pFileDef(pFolder.fileDef());
				if (pFileDef)
				{
					FileInfo_t FI(*pDC, *pFileDef);
					//dump source/header pair
					if (ofs_cpp.isSet())
					{
						//if (iCpp++ == 0)
							//ofs_cpp.os() << "\n\n/*** MODULE: " << ModuleTitle(pModuleFolder) << " */\n\n";
						ofs_cpp.os() << "\n\n/* FILE: " << mrProject.files().relPath(&pFolder) << SOURCE_EXT << " */\n\n";
						FI.DumpToFile(ofs_cpp, flags);
					}
					else if (!ofs_h.isSet())
					{
						SrcDumpFile ofs(makeDumpPath(FilesMgr0_t::realPath(&pFolder, true), fDir, flags));
						if (FI.DumpToFile(ofs, flags))//source
							sPath = ofs.path(), iCount++;
					}

					adcui::UDispFlags flags2(flags);
					flags2.setL(adcui::DUMP_HEADER);
					if (ofs_h.isSet())
					{
						//if (iH++ == 0)
							//ofs_h.os() << "\n\n/*** MODULE: " << ModuleTitle(pModuleFolder) << " */\n\n";
						ofs_h.os() << "\n\n/* FILE: " << mrProject.files().relPath(&pFolder) << HEADER_EXT << " */\n\n";
						FI.DumpToFile(ofs_h, flags2);
					}
					else
					{
						SrcDumpFile ofs(makeDumpPath(FilesMgr0_t::realPath(&pFolder, true), fDir, flags2));
						if (FI.DumpToFile(ofs, flags2))//header
							sPath = ofs.path(), iCount++;
					}
				}
			}
		}
	}

	if (iCount == 1)
		PrintInfo() << "1 file generated: " << sPath << std::endl;
	else
		PrintInfo() << iCount << " file(s) generated in folder " << mrProject.outputDir() << std::endl;

	mrProject.setOutputDir(sTmp);//restore root dir
	return 1;
}

/*** MODULE: hdedede.dll ***/

// Synopsis: setoutdir <outdir>
MYCOMMAND_IMPL(setoutdir)
{
	if (args.size() > 1)
	{
		MyDirPath fDir(args[1]);// , MyPath(args.mRefPath));

		//ProjectInfo_t pjx(mrProject);
		if (SetRootDirectory(fDir))
			fprintf(stdout, "Output directory set: %s\n", mrProject.outputDir().c_str());
	}
	return 1;
}

//Synopsis: calcdp [file]
//	Calculate header dependencies on a file (or whole module)
MYCOMMAND_IMPL(calcdp)
{
	DECLARE_DC_CONTEXT(ctx);
	FolderPtr pFolder(nullptr);
	if (args.size() > 1)
	{
		ZPath_t zPath(args[1]);
		AssurePathAbsolute(zPath, ctx.moduleFromLocus());
		if (!zPath.empty())
		{
			adcui::FolderTypeEnum iKind;
			pFolder = mrProject.files().FindFileByPath(zPath.toString(), iKind);
		}
	}

	TypePtr iModule(ctx.moduleFromLocus());
	if (pFolder)
		iModule = ModuleInfo_t::ModuleOf(pFolder);

	if (iModule && DCREF(iModule))
	{
		WriteLocker lock;
		DcInfo_t DI(*DCREF(iModule));
		//FileInfo_t FI(DI, *pFolder->fileDef());
		DI.CalculateDependencies(pFolder);
		DI.redump(ctx, REDUMP_ALL);
		return 1;
	}
	
	return 0;
}

//find object by name in SRC context by fully qualified name in specific module;
//if iModule is not specified, expect fullName prefixed with a module name;
//if bPretty is true, take into consideration a map of pretty aliases;
//note, the fullName can consist of auto names;
ObjPtr ProjectEx_t::findObject(MyString fullName, bool bPretty, CTypePtr iModule) const
{
	CTypePtr iScope;
	if (!iModule)
	{
		MyString sModule(ZPath_t::moduleOf(fullName));
		ProjectInfo_t PI(*this);
		CFolderPtr pFolder(PI.FindModuleFolder(sModule, true));
		if (!pFolder)
			return nullptr;
		iModule = ProjectInfo_t::ModuleOf(pFolder);
		if (!iModule)
			return nullptr;
		fullName.remove(0, unsigned(sModule.length() + 1));
	}
	const Dc_t *pDC(DCREF(iModule));
	if (!pDC)
		return nullptr;
	iScope = pDC->primeSeg();
	if (!iScope)
		return nullptr;

	DcInfo_t DI(*pDC);

	ObjPtr pObj;
	if ((pObj = DI.FindObjByAutoNameEx(fullName, iScope)) == nullptr)
		pObj = DI.FindObjByName(fullName, iScope);
	return pObj;
}

void ProjectEx_t::setUnresolvedExternalsModule(TypePtr p)
{
	if (p == mpUnExtModule)
		return;
	if (mpUnExtModule)
		mpUnExtModule->releaseRef();
	mpUnExtModule = p;
	if (mpUnExtModule)
		mpUnExtModule->addRef();
}

// Set up a stub
// Synopsis: mkstub [-module <module>] [-apply] @{addr} {stub_string}
//		-apply	: redefine an associated function
//		-module	: target module
MYCOMMAND_IMPL(mkstub)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.dcRef())
		return 0;
	if (args.size() < 2)
		return 0;
	MyArgs2 args2(args);

	CTypePtr iModule(ctx.locus().module());
	MyString sModule;
	if (args2.RemoveOptEx("-module", sModule))
		iModule = FindModuleEx(sModule, true);

	//bool bRedecomp(args2.RemoveOpt("-redecomp"));
	bool bApply(args2.RemoveOpt("-apply"));

	args2.RemoveAt(0);//args name

	StubInfo_t SI(*DCREF(iModule));

	ADDR atAddr(0);
	REFMODE_t refMode(StubParser_t::parseAtAddr(args2.at(0).c_str(), atAddr, SI.ImageBase()));//refMode: don't care

	//assert(refMode != REFMODE__BAD);
	args2.RemoveAt(0);//addr
	MyString sValue(args2.AsString());

	const Stub_t* pStub(SI.TouchStub(atAddr, sValue));
	if (!pStub)
		pStub = SI.MakeStub(atAddr, refMode, sValue);
	SI.SaveOne(*pStub);
	SI.RedumpStubs();

	/*if (bRedecomp)
	{
		if (!ctx.scopeFunc())
			return 0;
		FuncInfo_t FI(SI, *ctx.scopeFunc(), *ctx.folder()->fileDef());
		FI.InitiateRedecompile();
	}*/

	if (bApply)
	{
		FieldPtr pField(SI.FindFieldRef(atAddr, refMode, iModule));
		if (pField && pField->isGlobal())
		{
			DcInfo_t DI(*DCREF(iModule));

			if (refMode == REFMODE_DIRECT)
			{
				GlobPtr iGlob(DcInfo_t::GlobObj(pField));
				if (!iGlob)
					iGlob = DI.AcquireFunction(pField, nullptr, FTYP_STUBS);
				if (!iGlob->func())
					return 0;
				FuncInfo_t FI(DI, *iGlob);// , * iGlob->folder()->fileDef());
				if (ProtoInfo_t::IsStub(iGlob))
				{
					FuncProfile_t fp;
					if (!DI.ToFuncProfile(pStub->value(), fp))
						return 0;
					ProtoInfo_t TI(FI);
					TI.FromFuncProfile(fp);
				}
				else if (iGlob == ctx.scopeFunc())
				{
					if (!FI.Redecompile(true))
						return false;
					DI.redump(ctx, REDUMP_SRC);
				}
			}
			else if (refMode == REFMODE_GLOBAL_PTR)
			{
				if (!pField->isTypeImp())
					return 0;
				FieldPtr pExpField(DI.ToExportedField(pField));
				if (!pExpField)
					return 0;

				TypePtr iModule2(ModuleInfo_t::ModuleOf(pExpField));
				DcInfo_t DI2(*DCREF(iModule2));

				GlobPtr iGlob(DcInfo_t::GlobObj(pExpField));
				if (!iGlob)
				{
					FolderPtr pFolder(DI2.AddFileEx(FPATH_FROM_EXPORTED));
					DI2.AcquireFunction(pExpField, pFolder);
				}
				if (!iGlob->func())
					return 0;
				//FuncInfo_t FI2(DI2, *iGlob);
				FuncProfile_t fp;
				if (!DI2.ToFuncProfile(pStub->value(), fp))
					return 0;
				if (ProtoInfo_t::IsStub(iGlob))
				{
					ProtoInfo_t TI2(DI2, iGlob);
					TI2.FromFuncProfile(fp);
				}
			}
			if (ctx.scopeFunc())
			{
				//COMMAND_decompile(this, args);
				/*FileInfo_t fileInfo(DI, *ctx.fileDef());
				FileInfoCmdServer_t cmdSvr(CMDServerCommandMap(), fileInfo);
				return FileInfoCmdServer_t::COMMAND_decompile(&cmdSvr, args2);*/
			}
		}
	}

	return 1;
}


MYCOMMAND_IMPL(load_stubs)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.dcRef())
		return 0;

	StubInfo_t SI(*ctx.dcRef());

	StubMgr_t& m(ctx.dcRef()->stubs());
	m.clear();

	MyPath myPath(SI.ModulePath());
	myPath.SetExt("proto");
	if (!SI.LoadStubs(myPath.Path()))
		return 0;

	fprintf(stdout, "Stubs loaded: %s\n", myPath.Path().c_str());
	projx().guix().GuiOnStubsModified();
	return 1;
}

// Postformat executable module (create SRC declarations from export/import sections, process debug information)
// Synopsis: postformat [-module <module>] [options]
// Options:
//		-ne	: skip processing of EXPORT symbols
//		-ni	: skip processing of IMPORT symbols
//		-di <hint>	: initiate processing of debug information, where hint maybe like: {coff|dwarf|map|map@<path>|pdb|pdb@<path>}
MYCOMMAND_IMPL(postformat)
{
	DECLARE_CONTEXT(ctx);

	TypePtr pModule(ctx.locus().module());
	Dc_t* pDC(DCREF(pModule));
	if (!pDC)
		return 0;

	if (args.size() == 1)
	{

	}

	bool bSkipExports(args.RemoveOpt("-ne"));
	bool bSkipImports(args.RemoveOpt("-ni"));
	MyString dbgHint;
	bool bDbgInfo(args.RemoveOptEx("-di", dbgHint));

	DcInfo_t DI(*pDC);

	unsigned segAffinity(DI.PrimeSeg()->typeSeg()->affinity());

	assert(!projx().decompiler());
	PostAnalysis_t* pAnal(new PostAnalysis_t(projx(), segAffinity, *pDC));

	pAnal->addStage(new StageResources_t(*pAnal));
	
#if(1)
	//pAnal->addStage(new StageTypes_t(*pAnal));

	if (!mrMain.options().bNoDbg)
		pAnal->addStage(new StageDebugInfo_t(*pAnal, dbgHint));

	if (!bSkipExports)
		pAnal->addStage(new StageExports_t(*pAnal));

	//if (!mrMain.startupInfo().bNoMap)
		//pAnal->addStage(new StageMapInfo_t(*pAnal));
#endif
#if(1)
	pAnal->addStage(new StageSignatures_t(*pAnal));

	if (!bSkipImports)
		pAnal->addStage(new StageImports_t(*pAnal));

	if (mrMain.options().nProtoMode > 0)
		pAnal->addStage(new StageStubs_t(*pAnal));

#endif

	projx().pushAnalyzer(pAnal);
	//pAnal->Release();
	return 1;

}


