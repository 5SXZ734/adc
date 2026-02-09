#include "dc.h"
#include "prefix.h"
#include <fstream>
#include "qx/MyFileMgr.h"
#include "shared/defs.h"
#include "shared/link.h"
#include "shared/misc.h"
#include "shared/front.h"
#include "db/mem.h"
#include "db/main.h"
#include "db/field.h"
#include "db/type_struc.h"
#include "db/names.h"
#include "db/script.h"
#include "db/command.h"
#include "db/type_seg.h"
#include "db/anlzbin.h"
#include "db/proj.h"
#include "db/front_impl.h"
#include "db/symbol_map.h"
#include "files_ex.h"
#include "ana_pcode.h"
#include "ana_expr.h"
#include "ana_switch.h"
#include "ana_branch.h"
#include "ana_local.h"
#include "ana_main.h"
#include "expr_term.h"
#include "path.h"
#include "dump.h"
#include "proj_ex.h"
#include "main_ex.h"
#include "clean_ex.h"
#include "ui_main_ex.h"
#include "sym_parse.h"
#include "dump_file.h"
#include "arglist.h"
#include "expr_simpl.h"
#include "info_class.h"
#include "expr_dump.h"


Dc_t::Dc_t(ProjectEx_t &rProject, TypePtr iPrimeSeg, TypePtr pModule)
	: mrProject(rProject),
	mpModuleFolder(pModule->typeModule()->folderPtr()),
	miFrontSeg(iPrimeSeg),
	mpIFront(nullptr)
{
	initFrontend();

	module()->SetUserData<Dc_t>(this);

	ModuleInfo_t MI(rProject, *module());
	FolderPtr pNFolder(MI.FolderOfKind(FTYP_NAMES));
	if (pNFolder)
		pNFolder->fileNames()->setNamesMap(iPrimeSeg->typeComplex()->namesMgr());
	FolderPtr pEFolder(MI.FolderOfKind(FTYP_EXPORTS));
	if (pEFolder)
		pEFolder->fileExports()->setNamesMap(iPrimeSeg->typeComplex()->namesMgr());

	FolderPtr pIFolder(MI.FolderOfKind(FTYP_IMPORTS));
	if (pIFolder)
		pIFolder->fileImports()->setNamesMap(iPrimeSeg->typeComplex()->namesMgr());

//	mIntrinsics.push_back(nullptr);//first element is NUL
}

Dc_t::~Dc_t()
{
	PrintPoolRequstsInfo();
}

FRONT_t &Dc_t::front()
{
	return *mrProject.frontFromIndex(miFrontSeg->typeSeg()->frontIndex());
}

const FRONT_t &Dc_t::front() const
{
	return *mrProject.frontFromIndex(miFrontSeg->typeSeg()->frontIndex());
}

/*const ExportMap_t& Dc_t::exportMap() const
{
	ModuleInfo_t MI(project(), *module());
	FolderPtr pFolder(MI.FolderOfKind(FTYP_EXPORTS));
	if (pFolder)
		return pFolder->fileExports()->expMap();
	static ExportMap_t fake;
	return fake;
}

void Dc_t::cleanExportMap()
{
	ModuleInfo_t MI(project(), *module());
	FolderPtr pFolder(MI.FolderOfKind(FTYP_EXPORTS));
	if (pFolder)
		pFolder->fileExports()->expMap().clear();
}*/

void Dc_t::initFrontend()
{
	assert(!mpIFront && !mFDC.get());
	FRONT_t &rFront(front());
	ModuleInfo_t MI(mrProject, *module());
	mpIFront = rFront.device(MI.GetDataSource());
	assert(mpIFront);
	mFDC.set(mpIFront->createFrontDC());
}

GlobPtr Dc_t::getIntrinsic(size_t i) const
{
	assert(i > 0);
	--i;//1-biased
	if (i < mIntrinsics.size())
		return mIntrinsics[i];
	return nullptr;
}

int	Dc_t::PtrSize() const
{
	return fe().ptr_near;
}

int Dc_t::PtrSizeEx() const
{
	return fe().ptr_far;
}

int	Dc_t::stackAddrSize() const
{
	return fe().stackaddrsz;
}

#if(0)
FieldPtr  Dc_t::globalRef(OpPtr  pOp) const
{
	mapOp2Fld_itc it = mOp2Fld.find(pOp);
	if (it != mOp2Fld.end())
		return it->second;
	return nullptr;
}

void Dc_t::setGlobalRef(OpPtr  pOp, FieldPtr  pField)
{
	assert(pOp);
	if (pField)
	{
		pair<mapOp2Fld_it,bool> it;
		it = mOp2Fld.insert( pair<OpPtr, FieldPtr >(pOp, pField) );
	}
	else
	{
		mOp2Fld.erase(pOp);
	}
}
#endif

const STORAGE_t &Dc_t::SS(SSID_t ssid) const {
	return fe().storage(ssid);
}

/*
int Dc_t::MoveToFile(Obj_t *pObj, Fi le_t *pFile)
{
assert(pObj);
assert(pFile);

if (!IS_ PTR(pObj))
return 0;

Fil e_t *pOwnerFile = pObj->Get OwnerFile();
if (pOwnerFile == pFile)
return 0;

switch (pObj->Obj Type())
{
case OBJID_OP:
return 0;

case OBJID_TYPE_STRUC://struc
case OBJID_TYPE_FUNCDEF:
pFile->LinkObj(pObj);
break;

case OBJ_ DATA://data
if (!((FieldPtr )pObj)->Is Global())
return 0;
case OBJID_TYPE_PROC://func
if (pFile->IsHeader())
return 0;
pFile->LinkObj(pObj);
break;
}
return 1;
}*/

StubMgr_t *Dc_t::stubsPtr() const
{
	FolderPtr pFolder(moduleRef().special(FTYP_PROTOTYPES));
	if (pFolder)
		return &pFolder->fileStubs()->stubs();
	return nullptr;
}

const StubMgr_t &Dc_t::stubs() const
{
	FolderPtr pFolder(moduleRef().special(FTYP_PROTOTYPES));
	return pFolder->fileStubs()->stubs();
}

StubMgr_t &Dc_t::stubs()
{
	FolderPtr pFolder(moduleRef().special(FTYP_PROTOTYPES));
	return pFolder->fileStubs()->stubs();
}

void Dc_t::saveStubs() const
{
	StubInfo_t SI(*this);
	SI.SaveStubs();//before c-funcs gone
}

//#define DEFSTRUCPFX	"struc"

/*const char *Dc_t::GenerateName(const char *pfx)
{
static char buf2[20];
sprintf(buf2, "%d", mStrucId);
const char *pBuf = MakeName(pfx, 0, buf2);
mStrucId++;
return pBuf;
}*/

void Dc_t::NewPtrDump(ADDR funcAddr, int line, MyStreamBase &ss)
{
	mPtrDumps.add(funcAddr, line, ss);
}

bool Dc_t::ReadPtrDump(ADDR funcAddr, int line, MyStreamBase &ss)
{
	if (mPtrDumps.empty())
	{
		fprintf(STDERR, "Warning: PTR dump is empty! Recompile with `TRACK_PTRS2VIEW=1`\n");
		return false;
	}
	return mPtrDumps.read(funcAddr, line, ss);
}

/*ProbeEx_t *Dc_t::getContext() const
{
if (!project().hasContext())
return false;
return dynamic_cast<ProbeEx_t *>(&project().context());
}*/

/*void Dc_t::setContext(ProbeEx_t *pctx)
{
project().setContext(pctx);
}

const ProbeEx_t &Dc_t::context() const
{
return dynamic_cast<const ProbeEx_t &>(project().context());
}

ProbeEx_t &Dc_t::context()
{
return dynamic_cast<ProbeEx_t &>(project().context());
}*/

/*void Dc_t::RegisterExternalSymbol(FieldPtr pExtField, FieldPtr pMyField)//pExtField:exported in another module, pMyField:import entry in my module
{
std::pair<std::map<FieldPtr , FieldPtr >::iterator, bool> ret;
ret = mExtSymbolMap.insert(std::make_pair(pExtField, pMyField));
assert(ret.second);//the imported entries may be duplicated!
}*/

//look forward
/*ADDR Struc_t::CheckOverlap(FieldMapCIt it0) const
{
	FieldPtr pField0(VALUE(it0));
	assert(pField0->owner()->typeStruc() == this);

	ADDR upper(size0());

	FieldMapCIt it(it0);

	if (it != mFields.end())
	{
		++it;
		FieldPtr pField(VALUE(it));
		upper = pField->key;
	}
	else if (size0() < 0)
		return 0;//no overlap for non-bounding strucs

	int sz(pField0->size());
	assert(sz >= 0);
	if (sz == 0)
		sz = OPSZ_BYTE;
	ADDR target(pField0->key + sz);
	if (target > upper)
		return target - upper;

	return 0;
}*/

void TypeClass_t::moveFrom(TypeClass_t &o)
{
	Struc_t::moveFrom(o);
	ClassMemberList_t &l(o.methods());
	while (!l.empty())
	{
		ClassMemberListIt i(l.begin());
		//XFieldLink_t *pXRef(l.take_front());
		//mMethods.push_back(pXRef);
		addMember(*i);
		l.erase(i);
	}
}

bool TypeClass_t::addMember(GlobPtr p)
{
//CHECK(p->address() == 0x10000000)
CHECKID(p, 0x1c2c)
STOP
	mMethods.push_back(p);
	return true;
}

ClassVTable_t& TypeClass_t::addVTable(GlobPtr p, int voffs)
{
	//assert(p->isGlobal());
	ClassVTables_t::iterator i(mVTables.insert(std::make_pair(voffs, ClassVTable_t(p))));
	return i->second;
}

bool TypeClass_t::addVirtMember(GlobPtr p, int off, ClassVTable_t &table)
{
	//assert(p->isGlobal());
	table.entries.insert(std::make_pair(off, p));
	p->flags() |= FDEF_VIRTUAL;
	return true;
}













