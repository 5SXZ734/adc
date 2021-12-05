#include "proj.h"
#include "prefix.h"
#include <fstream>

#include "qx/MyDir.h"
#include "qx/MyArgs.h"
#include "qx/MyFileMgr.h"
#include "shared/defs.h"
#include "shared/link.h"
#include "shared/misc.h"
#include "shared/data_source.h"
#include "mem.h"
#include "main.h"
#include "obj.h"
#include "field.h"
#include "type_struc.h"
#include "names.h"
#include "script.h"
#include "command.h"
#include "type_seg.h"
#include "type_proxy.h"
#include "anlzbin.h"
#include "debug.h"
#include "clean.h"
#include "ui_main.h"
#include "print.h"
#include "proj_cmd.h"
#include "types_mgr.h"

//using namespace std;


reserved_pfx_map::reserved_pfx_map()
{
	add("unk_");//hex suffix
	add("byte_");
	add("word_");
	add("dword_");
	add("qword_");
	add("flt_");
	add("dbl_");
	add("data_");
	add("sub_");
	add("loc_");
	add("ofs_");
	add("asc_");
	add("unz_");
	add("unc_");
	add("rva_");
	//////////////
	add("struct_", 1);//decimal suffix
	add("class_", 1);
	add("enum_", 1);
	add("union_", 1);
}

int reserved_pfx_map::is_autoname(const char *ps, value_t *pvalue) const//1:type, -1:type with negative id (debug), 2: field
{
	assert(ps && ps[0]);//!empty
	const char *pn(strchr(ps, '_'));
	if (!pn)
		return 0;
	size_t n(pn - ps + 1);
	const_iterator i(find(std::string(ps, n)));
	if (i == end())
		return 0;

	int ret;
	if (i->second == 0)
	{
		ret = 2;//fields
	}
	else if (i->second == 1)
	{
		ret = 1;//types
#ifdef _DEBUG
		//check for double underscore
		if (ps[n] == '_')
		{
			++n;
			ret = -1;//types with negative id(?)
		}
#endif
	}
	else
	{
		ret = i->second;//other values are possible for override
		assert(ret != 2);
	}
	
	if (i->second & 1)
	{
		if (pvalue && !Str2Int(ps + n, *pvalue))//! suffix may fail
			ret = 0;//reset
	}
	else
	{
		if (pvalue && !StrHex2Int(ps + n, *pvalue))//! suffix may fail
			ret = 0;//reset
	}
	
	return ret;
}


//////////////////////////////////////////////////////
// Project_t

Project_t *Project_t::spSelf(nullptr);

Project_t::Project_t(Main_t &rMain, MemoryMgr_t &rMemMgr)
	: mrMain(rMain),
	mrMemMgr(rMemMgr),
	mrGui(mrMain.gui()),
	//mpMemMgrNS(nullptr),
	//mpTypesMgrNS(nullptr),
	muDirty(0),
	//mProjectName("unnamed"),
	mpStartUpBinary(nullptr),
	mpDebugger(nullptr),
	mbIsClosing(false),
	mpSelf(nullptr)
{
//?	setContext(new Probe_t)->Release();
	spSelf = this;
}

Project_t::~Project_t()
{
	delete mpDebugger;
	assert(!mFiles.mpRootFolder);
	assert(mTypesMapOwners.empty());
	//assert(!mpMemMgrNS);
	//assert(!mpTypesMgrNS);
	spSelf = nullptr;
}

/*MemoryMgr_t &Project_t::memMgrNS()
{
	if (!mpMemMgrNS)
	{
		mpMemMgrNS = new MemoryMgr_t();
#ifdef _DEBUG
		mpMemMgrNS->mName = "$GLOBAL_NS";
#endif
	}
	return *mpMemMgrNS;
}

TypesMgr_t *Project_t::typesMgrNS()
{
	if (!mpTypesMgrNS)
		mpTypesMgrNS = new TypesMgr_t;
	return mpTypesMgrNS;
}*/

bool Project_t::ExecuteCommand(Cmd_t &cmd, int *pret)
{
	static ProjCmdMap_t cmdMap;
	ProjCmdServer_t cmdSvr(cmdMap, ProjectInfo_t(*this));
	if (!cmdSvr.ExecuteCommand(cmd))
		return false;
	*pret = cmdSvr.mRet;
	return true;
}

void Project_t::OnDestroyModule(TypePtr iModule, bool bClosing)
{
	BinaryCleaner_t<> BC(*this);
	BC.setClosing(bClosing);
	BC.destroyModule(iModule);
}

/*int Project_t::PrintListing(unsigned u, FieldPtr pObj)
{
	*//*std::ofstream ofs;
	ofs.open(filePath);
	if (ofs.fail())
	{
	std::cerr << "Error: Can't open file " << filePath << std::endl;
	return 0;
	}*/

	//PrintListing(u);

	/*std::cout << "Listing written to file: " << filePath << std::endl;
	ofs.close();*//*

	return 1;
}*/

/*void Project_t::PrintTypesList(Struc_t * pStruc, ADDR a)
{
	int n = 0;
	TypesMgr_t * pTypeMgr = pStruc->findTypeMgr();
	if (pTypeMgr)
		n = pTypeMgr->PrintTypesList(mos, 0);

	if (pTypeMgr == typeMgr())
	{
		for (mapExtTyp_it it = mExtTypeRefs.begin(); it != mExtTypeRefs.end(); it++)
		{
			if (n > 0)
				mos << ",\n";
			const string &s = it->first;
			mos << s;
			n++;
		}
	}
}*/

TypePtr Project_t::OnNewPortal(TypePtr iSegMain)//, FrontImpl_t &)
{
	return nullptr;
}

/*FieldPtr Project_t::OnFieldInsertFailed2(TypePtr, ADDR, FieldPtr pField)
{
	//just delete the field
	mrMemMgr.Delete(pField);
	return nullptr;
}*/

void Project_t::setPath(const MyString &s)
{
	msPath = s;
	if (!msPath.empty())
	{
		MyPath a(msPath);
		setTitle(a.Name());
	}
}

/*size_t Project_t::addBinaryInfo(const BinaryInfo_t &aBin)
{
	mBinInfo.push_back(aBin);
	if (!(mStartUpBinaryIndex < mBinInfo.size()))
		if (mBinInfo.back().mName.endsWith(".exe"))//is executable?
			mStartUpBinaryIndex = mBinInfo.size() - 1;
	return mBinInfo.size() - 1;
}

size_t Project_t::findBynaryIndex(MyString s) const
{
	int n(s.find(':'));
	if (n >= 0)
	{
		for (size_t i(0); i < mBinInfo.size(); i++)
			if (mBinInfo.at(i).mName == s.left(n))
				return i;
	}
	return -1;
}*/


bool Project_t::isAutoname(const MyString &s, TypePtr iModule) const
{
	if (checkAutoPrefix(s) != 0)
		return true;//do not permit user's identifiers with global prefixes
	return false;
}

bool Project_t::Autoname(CFieldPtr pSelf, MyString &s) const
{
	return false;
}

void Project_t::typeNameScoped(CTypeBasePtr pSelf, char chopSymb, FullName_t &v) const
{
	if (pSelf && !pSelf->typeSeg())
	{
		assert(pSelf->typeStruc() && (!pSelf->nameless() || pSelf->hasUglyName()));
		typeNameScoped(pSelf->owner(), chopSymb, v);
		ProjectInfo_t PI(*this);
		v.append(PI.TypeName0((TypePtr)pSelf, chopSymb));//no pretty names!
		//s.append("::");
	}
}

MyString Project_t::typeName(CTypeBasePtr pSelf0) const//if chopSymb==0, suffix is discarded
{
	CTypePtr pSelf((CTypePtr)pSelf0);
	if (pSelf->typeSeg())
	{
		MyString s;
		pSelf->namex(s);
		if (s.isEmpty())
			s = "<noname>";
		return s;
	}

	ProjectInfo_t PI(*this);

	if (!pSelf->nameless())
		return pSelf->name()->c_str();
	
	if (pSelf->typeTypedef())
	{
		Typedef_t *pTypedef(pSelf->typeTypedef());
		return PI.StrucNameless(pTypedef->baseType());
	}
	
	if (pSelf->typeProxy())
	{
		TypePtr pIncumb(pSelf->typeProxy()->incumbent());
		assert(pIncumb->typeStruc() || pIncumb->typeTypedef());
		return PI.StrucNameless(pIncumb);//?
	}
	
	if (pSelf->typeStruc())
	{
		if (!ProjectInfo_t::IsProc(pSelf))
			return PI.StrucNameless(pSelf);
		return PI.FieldName(pSelf->parentField());
	}

	MyString s2;
	pSelf->namex(s2);//simples
	return s2;
}

TypePtr Project_t::objectDisplayScope(CObjPtr pObj) const
{
	TypePtr pType(pObj->objTypeGlob());
	if (pType && pType->owner() && pType->owner()->isShared())
		return pType->owner();
	return nullptr;
}

/*PNameRef Project_t::displayName(FieldPtr pSelf, TypePtr) const
{
	//assert(!pSelf->nameless());
	return pSelf->name();
}*/

MyString Project_t::tipName(CFieldPtr pSelf, CTypePtr iModule) const
{
	ProjectInfo_t PI(*(Project_t *)this);
	return PI.FieldName(pSelf);
}

MyString Project_t::tipName(CTypePtr pSelf, CTypePtr iModule) const
{
	ProjectInfo_t PI(*(Project_t *)this);
	return PI.TypeName(pSelf);
}

ObjPtr Project_t::findObject(const char* pName, TypePtr pModule)
{
	ModuleInfo_t MI(*this, *pModule);
	return MI.FindTypeByName(pName);

}

BinaryAnalyzer_t* Project_t::binAnalizer() const
{
	return dynamic_cast<BinaryAnalyzer_t*>(analyzer());
}

BinaryAnalyzer_t* Project_t::findBinAnalizer() const
{
	BinaryAnalyzer_t* pAnal(nullptr);
	for (std::list<IAnalyzer*>::const_iterator it(mAnlz.begin()); it != mAnlz.end(); ++it)
	{
		pAnal = dynamic_cast<BinaryAnalyzer_t*>(*it);
		if (pAnal)
			break;
	}
	return pAnal;
}

int Project_t::execAnalyzer()
{
	IAnalyzer *pIAnlz(analyzer());
	if (!pIAnlz)
		return 0;

	int iModified(0);
	mrMain.lockProjectWrite(true);//project can be modified down here - prohibit any read attempts
	if (pIAnlz->stopFlag() == StopFlag::RESET)
		iModified = pIAnlz->process();
	//checkAnalyzerStatus();
	mrMain.lockProjectWrite(false);
	if (iModified)
		//gui().GuiOnProjectModified();
		markDirty(DIRTY_GLOBALS);
	return iModified;
}

bool Project_t::checkAnalyzerStatus()
{
	if (!analyzer())
		return false;

	if (analyzer()->finished())//finished - can be deleted
	{
		//IAnalyzer *pIAnalzNext(pIAnlz->spawnNext());
		mrMain.lockProjectWrite(true);
		popAnalizer();//just 1
		mrMain.lockProjectWrite(false);
		mrMain.OnAnlzReady();
		return analyzer() != nullptr;
	}

	int iStop(mrMain.checkStopRequest());
	if (iStop < 0)//abort request
	{
		//assert(0);
		//pIAnlz->cleanup();
		mrMain.lockProjectWrite(true);
		while (analyzer())//all
			popAnalizer();
		mrMain.CloseScript();
		mrMain.lockProjectWrite(false);
		//analyzer()->setStopFlag(StopFlag::RESET);
		return false;
	}
	if (iStop > 0)//pause request
	{
		//gui().GuiOnAnalizerPaused();
		//mrMain.checkUiEvents(true);//force UI update
		return true;
	}
	
	//not finished yet - proceed
	mrMain.OnAnlzReady();//re-post
	return true;
}

uint8_t Project_t::dumpLineStatus(CTypePtr iStruc) const
{
	if (ProjectInfo_t::IsBitvar(iStruc))
		iStruc = iStruc->parentField()->owner();
	if (iStruc->typeStrucvar())
		return adcui::ITEM_CONT_STRUCVAR;
	if (iStruc->isShared())
		return adcui::ITEM_CONT_SHARED;
	return 0;
}

/*MyString Project_t::expandCommand(MyString s, I_Context* pICtx)
{
	int n(s.find('%'));
	if (n >= 0)
	{
		MyString key;
		int m(s.find('%', n + 1));
		if (m > n)
		{
			key = s.substr(n + 1, m - n - 1);
		}
		else
		{
			m = -1;
			key = s.substr(n + 1);
		}
		Probe_t *pCtx(getContext());
		if (pCtx)
		{
			MyString s2;
			if (key == "FUNCVA")
			{
				TypePtr iFunc(pCtx->funcOwner());
				if (iFunc)
					s2 = MyStringf("%08X", iFunc->parentField()->_key());
			}
			else if (key == "VA")
				s2 = MyStringf("%08X", pCtx->va());
			if (!s2.empty())
				s.replace(n, m - n + 1, s2);
			releaseContext(pCtx);
		}
	}
	return s;
}*/

void Project_t::OnDestroyProject(bool bIsClosing)
{
	const FoldersMap &m(rootFolder().children());
	if (!m.empty())
	{
		for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
		{
			const Folder_t &rFolder(*i);
			OnDestroyModule(ModuleInfo_t::ModuleOf(&rFolder), bIsClosing);
		}
	}
}

TypePtr Project_t::getStockType(OpType_t t) const
{
	return mpTypesMgr->getStockType(t);
}

TypePtr Project_t::GetNameOwnerOf(TypePtr pCplx) const
{
	while (pCplx)
	{
		if (pCplx->typeComplex()->namesMgr())
			return pCplx;
		Seg_t *pSeg(pCplx->typeSeg());
		if (pSeg)
		{
			if (pSeg->traceLink())
			{
				pCplx = pSeg->traceLink();
				continue;
			}
			else if (pSeg->superLink())
			{
				pCplx = pSeg->superLink();
				continue;
			}
		}
		/*if (pCplx->typeFuncDef())
			return pCplx;
		if (pCplx->typeUnion())
		{
			pCplx = FileInfo_t::IsLocalsTop(pCplx);
			if (pCplx)
				return pCplx;
		}*/
		FieldPtr pField(pCplx->parentField());
		if (pField)
			pCplx = pField->OwnerComplex();
	}
	return nullptr;
}

TypePtr Project_t::GetNameOwnerOf(FieldPtr pField) const
{
	return GetNameOwnerOf(pField->owner());
}

 MyString Project_t::fieldDisplayName(CFieldPtr pSelf) const
{
	ProjectInfo_t PI(*(Project_t *)this);
	if (!pSelf->nameless())
		return pSelf->name()->c_str();
	MyString s;
	if (PI.LabelName(pSelf, s))
		return s;//chop?
	return PI.AutoName(pSelf, nullptr);//needs not to be chopped
}

MyString Project_t::typeDisplayName(CTypePtr pType) const
{
	/*if (!pType->isShared())
	{
		if (pType->typeSeg())
			return "; segment";
		if (pType->typeUnion())
			return "; union";
		if (pType->type Proc())
			return "; function";
	}*/
	return typeName(pType);
}

MyString Project_t::fieldName(CFieldPtr pSelf) const
{
	return fieldDisplayName(pSelf);
}

/*Obj_t *Project_t::GetContextObj()
{
	Obj_t *pObj(nullptr);
	Probe_t *pCtx(getContext());
	if (pCtx)
	{
		pObj = pCtx->obj();
		releaseContext(pCtx);
	}
	return pObj;
}*/

const char *Project_t::autoPrefix(CFieldPtr pSelf, CTypePtr pType) const
{
	if (!pType)
		return "unk";
	if (pType->typeCode())
		return "loc";
	if (!pType->isShared())
	{
		if (pType->typeProc())
			return "sub";
		if (pType->isVTable())
			return "vtable";
		//const char *pfx(pType->printType());
		//if(pfx)
			//return pfx;
	}

	switch (pSelf->attrib())
	{
	//case ATTR_ ASCII: return "asc";
	//case ATTR_ UNICODE: return "unz";
	//case ATTR_ NUNICODE: return "unc";
	case ATTR_RVA:
	case ATTR_RVA_OR_ORDINAL: return "rva";
//?	case ATTR_RES_OFFS:	return "ofs";
	case ATTR_OFFS: return "ofs";
	default: break;
	}

	while (pType->typeArray())
		pType = pType->typeArray()->baseType();

	if (pType->typePtr())
		return "ofs";
	if (pType->typeSimple())
	{
		switch (pType->typeSimple()->typeID())
		{
		case OPSZ_BYTE:	return "byte";
		case OPSZ_WORD:	return "word";
		case OPSZ_DWORD:	return "dword";
		case OPSZ_QWORD:	return "qword";
		case OPTYP_CHAR8:	return "asc";
		case OPTYP_CHAR16:
			if (pSelf->attrib() == ATTR_NUNICODE)
				return "unc";
			return "unz";
		case OPTYP_REAL32:	return "flt";
		case OPTYP_REAL64:	return "dbl";
		default:			break;
		}
	}
	if (pType->typeThunk())
		return "thunk";
	return "data";
}

int Project_t::checkAutoPrefix(const char *ps, value_t *pvalue) const
{
	return mAutoPfxMap.is_autoname(ps, pvalue);
}

bool Project_t::deleteField(Locus_t& aLoc)
{
	ModuleInfo_t MI(*this, *aLoc.module());
	return MI.DeleteField(aLoc);
}

bool Project_t::deleteBogusField(FieldPtr pField, NamesMgr_t *pNS)
{
	BinaryCleaner_t<ProjectInfo_t> BC(*this);
	BC.destroyField(pField, pNS);
	return true;
}

FieldPtr Project_t::OnMakeExported(const frame_t& scp, TypePtr iType, const MyStringEx& aName, TypePtr pModule)
{
	ModuleInfo_t MI(*this, *pModule);

CHECK(scp.m_addr == 0x5f4918e0)
STOP

	Locus_t aLoc;
	aLoc.add(scp.cont(), scp.m_addr, nullptr);

	if (!MI.InsertField(aLoc) || !MI.SetTypeEx(aLoc.field0(), iType))
	{
		//check if some other exported entity is already at this address (shared code)
		FieldPtr pField(MI.AddSecondaryField(scp.cont(), scp.m_addr, nullptr));
//		assert(pField && pField->isClone());
		return pField;
	}
	return aLoc.field0();
}





