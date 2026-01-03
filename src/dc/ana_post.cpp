#include "ana_post.h"
#include "db/debug.h"
#include "db/front_impl.h"
#include "db/symbol_map.h"
#include "main_ex.h"
#include "proj_ex.h"
#include "info_dc.h"
#include "type_funcdef.h"
#include "ui_main_ex.h"
#include "sym_parse.h"
#include "probe_ex.h"
#include "make_type.h"
#include "clean_ex.h"
#include "info_func.h"
#include "info_class.h"
#include "c_pp.h"

////////////////////////////////////////////////CxxSymbMap

CxxSymbMap::~CxxSymbMap()
{
	for (CxxSymbMapIt i(begin()); i != end(); i++)
		delete_node(i->second);
}

CxxSymbMapIt CxxSymbMap::addSymbolIt(const char *mangled, const char *demangled)
{
	CxxSymbMapIt i(find(mangled));
	if (i != end())
		return i;

	//CHECK(MyString(mangled) == "?GetElements@CDynamicMission@@AAEAAV?$AIVec@PAVCElement@@$0GE@@@XZ")
	//STOP
	node_t *node(SymParse(demangled));
	if (!node)
		return end();

	if (node->type == NODE_TYPE && node->isTerminal())//probably a specification of data without type
	{
		//convert to data (prefer data over types for automatic exported/imported/debug symbols)
		node->type = NODE_NULL;
	}
	return insert(std::make_pair(mangled, node)).first;
}

node_t* CxxSymbMap::addSymbol(const char* mangled, const char* demangled)
{
	CxxSymbMapIt it(addSymbolIt(mangled, demangled));
	if (it != end())
		return it->second;
	return nullptr;
}

node_t *CxxSymbMap::findSymbol(const char *mangled) const
{
	const_iterator i(find(mangled));
	if (i == end())
		return nullptr;
	return i->second;
}


////////////////////////////////////////////////StageExports_t

StageExports_t::StageExports_t(PostAnalysis_t &r)
	: StageBase_t(r, STAGE_EXPORTS),
	iFrontSeg(r.dc().primeSeg()),
	pExpFolder(nullptr),
	symb(DcInfo_t(mr.dc()).GetDataSource()->pvt(), DcInfo_t(mr.dc()).IFrontOf(iFrontSeg)),
	PM("Exports"),
	symIt(symb)
{
	DcInfo_t DI(mr.dc());
	pExpFolder = DI.AddFileEx(FPATH_FROM_EXPORTED),
	//assert(!FolderOfKind(FPATH_FROM_EXPORTED));

	assert(DI.IFrontOf(iFrontSeg));

	//const int count_limit = 600;
	//int count(0);
}

StageExports_t::~StageExports_t()
{
	DcInfo_t DI(mr.dc());
	if (pExpFolder->fileDef()->IsEmpty())//not counting the includes
	{
		FileInfo_t expFile(mr.dc(), *pExpFolder->fileDef(), DI.memMgrGlob());
		expFile.ClearFile(false);
		BinaryCleaner_t<> BC(DI);
		BC.DestroyFolder(pExpFolder);//expFile will contain invalid ref after this
		DI.DeleteFolder(pExpFolder);
	}
	PM.reset();
	DI.SendProgress(PM);
}

void StageExports_t::init()
{
	assert(!symIt);
	symb.dumpExported();
	PM.reset(1, symb.size());
	m_progress = 0;
}

bool StageExports_t::run()
{
	if (m_progress < 0)
	{
		init();
		return true;
	}
	if (!symIt)
		return false;
	if (PM.update(symIt.index() + 1))
	{
		DcInfo_t DI(mr.dc());
		DI.SendProgress(PM);
	}
	run0();
	symIt++;
	m_progress++;
	return true;
}

void StageExports_t::run0()
{
	DcInfo_t DI(mr.dc());
	MyStringEx aName(symIt.symbol());
	MyString sModule(aName[2]);
	aName.set(2, "");//in PE, a file name may not match the embedded one

	if (sModule.isEmpty())
		sModule = DI.ModuleName();

#if(0)
	int iCase(DI.IsArchCaseSensitive());
	assert(DI.IsArchCaseSensitive() == 1 || sModule.lower() == DI.ModuleName().lower());
	assert(DI.IsArchCaseSensitive() == 0 || sModule == DI.ModuleName());
#endif

	aName = DI.Adjusted(aName);
	MyString sMangled(aName[0]);

	ADDR va(symIt.va());
CHECK(va == 0x001a00c0)
STOP

//static int z(0);
//fprintf(stdout, "--> %d: %s\n", z++, sMangled.c_str());
	MyString sDemangled;
	I_Front::SymbolKind eKind(symIt.demangled(sDemangled));
	if (1 && eKind > I_Front::SYMK_SPECIAL)
	{
		FrontImplSymDump_t fimp(mr.dc(), pExpFolder);
		TypePtr pSeg(DI.FindSegAt(DI.PrimeSeg(), va, DI.PrimeSeg()->typeSeg()->affinity()));
		if (pSeg)
		{
			fimp.PushScope(pSeg, va);
			IFront_Src<I_Module> iface(fimp);
			I_Front* pIFront(DI.IFrontOf(DI.PrimeSeg()));
			if (pIFront->processSpecial(iface, eKind, sDemangled))
				return;
		}
	}


	node_t *pn(nullptr);
	bool bIsFunc(symIt.isFunc());

CHECK(va == 0x5f490730)
STOP
	if (!sDemangled.isEmpty() && sDemangled != sMangled)
		pn = DI.projx().cxxSymbols().addSymbol(sMangled, sDemangled);//add one to ProjectEx_t::cxxSymbols()
	if (!pn)//try stubs
	{
		const StubSet& aSet(mr.loadSignatures(sModule));
		if (!aName[0].isEmpty())
			pn = aSet.findProto(aName);
//		else//try ordinal (sometimes the ordinals of named import/export entries do not match, so try ordinals as a last resort)
	//		pn = aSet.findProtoEx(aName);
	}

//CHECK(count == count_limit)
//CHECK(symIt.va() == 0x20f6aa50)
CHECK(aName[0] == "_ZTCN3Gtk9TreeModelE0_N4Glib9InterfaceE")
STOP
//if (count > count_limit) break;

	FileInfo_t expFile(mr.dc(), *pExpFolder->fileDef(), DI.memMgrGlob());
	TypeFomNode_t conv(expFile, mr.stubsPool(), true);
	try
	{
		conv.ProcessExportedEntry(va, pn, eKind, aName, bIsFunc);
	}
	catch (int err)
	{
		DI.PrintError() << "Failed to process exported symbol in module '" << DI.ModuleName() << "' (code=" << err << "):"
			<< std::endl << "\t" << aName[0] << std::endl;
	}
}




///////////////////////////////////////////////////////////// debug info


StageDebugInfo_t::StageDebugInfo_t(PostAnalysis_t &r, MyString hint)
	: StageBase_t(r, STAGE_DBGINFO),
	PM("Debug Symbols"),
	mHint(hint),
	mpDbgInfoDumper(nullptr),
	mrSP(mr.stubsPool()),
	mfi(r.dc(), nullptr)
{
	PM.reset(0, 100);
	m_progress = 0;
	try
	{
		mpDbgInfoDumper = mr.dc().frontEnd()->createDebugInfoDumper(mHint.c_str());
	}
	catch (const DataAccessFault_t& e)
	{
		mfi.PrintError() << e.what() << std::endl;
	}
}

StageDebugInfo_t::~StageDebugInfo_t()
{
	if (mpDbgInfoDumper)
		mpDbgInfoDumper->Release();
	PM.reset();
	mfi.SendProgress(PM);
}

void StageDebugInfo_t::resetProgress(const char *desc, unsigned procent)
{
	if (PM.update(procent))
	{
		PM.setDesc(desc);
		mfi.SendProgress(PM);
	}
}


bool StageDebugInfo_t::run()
{
	//if (0)
	if (mpDbgInfoDumper)
	{
		unsigned u(0);
		IFront_SymDump iface(*this);
		if (mpDbgInfoDumper->processNext(iface, u))
		{
			//if (PM.update(u))
//				SendProgress(PM);
			m_progress++;
			return true;
		}
	}
	return false;
}

/*HTYPE StageTypes_t::NewScope(const char *pTypeName, SCOPE_enum eScope, const char *pFieldName, AttrIdEnum attr)
{
	//		fprintf(stdout, "%s\n", pTypeName);
	DcInfo_t DI(mrDC);

	FolderPtr pFolder(mpFolder);
	if (!pFolder)
		pFolder = DI.AddFileEx(FPATH_FROM_DBG);

	TypePtr iType;
	FileInfo_t FI(mrDC, *pFolder->fileDef(), memMgrGlob());
	StubPool_t stubs;
	TypeFomNode_t TN(FI, stubs, false);
//	if (mpFolder)
//		iType = FI.ProcessClass(pTypeName, 0, false);//forced in this module
//	else
		iType = TN.Process Compound(pTypeName, 0, false);//may be external?
	if (iType)
	{
//CHECKID(iType, 0x17529)
//STOP
		if (eScope == SCOPE_ENUM)
			iType->flags() |= TYP_ENUM;
		m_cont.push_scope(iType, 0, false);
		if (mpFolder)
		{
			FolderPtr pFolder2(USERFOLDER(iType));
			if (pFolder2 != mpFolder)
			{
				if (ModuleOf(iType) != ModulePtr())
				{
					TypePtr iTop0(TypeTop(iType));
					//assert(iTop0 == iClass0);//later
					if (FI.CanRelocateType(iTop0))
						FI.RelocateType(iTop0, pTypeName);
					else
					{
						fprintf(STDERR, "Warning: Failed to accommodate %s at %s\n", pTypeName, mpFolder->name().c_str());
					}
				}
				else
					DI.AddTypeToFile(iType, mpFolder->fileDef());
			}
		}
	}
	return iType;
	//return DumpDebugInfoCallback::NewScope(pTypeName, eScope, pFieldName, attr);
}*/

///////////////////////////////////////////////////////////// imports

StageImports_t::StageImports_t(PostAnalysis_t &r)
	: StageBase_t(r, STAGE_IMPORTS),
	mrFileImported(r.addFileEx()),
	mpExportingModule(nullptr),
	symb(r.dataSource()->pvt(), r.dc().frontEnd()),
	PM("Imports"),
	symIt(symb)
{
	//const int count_limit = 300;
	//int count(0);
}

StageImports_t::~StageImports_t()
{
	ProjectInfo_t PI(mr.project());
	const FoldersMap &m(mr.dc().project().rootFolder().children());
	for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
	{
		CFolderRef rFolder(*i);
		if (ProjectInfo_s::IsPhantomFolder(rFolder))
		{
			ModuleInfo_t MI(PI, *ProjectInfo_s::ModuleOf(&rFolder));
			MI.UpdateViewGeometry2();
		}
	}

	PM.reset();
	PI.SendProgress(PM);
}

void StageImports_t::init()
{
	assert(!symIt);
	symb.dumpImported();
	PM.reset(1, symb.size());
	m_progress = 0;
}

bool StageImports_t::run()
{
	if (m_progress < 0)
	{
		init();
		return true;
	}
	if (!symIt)
		return false;//stop the stage
	if (PM.update(symIt.index() + 1))
	{
		ProjectInfo_t PI(mr.project());
		PI.SendProgress(PM);
	}
	run0();
	symIt++;
	m_progress++;
	return true;
}

//static int zi(0);
void StageImports_t::run0()
{
	DcInfo_t DI(mr.dc());//self
	MyString moduleName(DI.ModuleName());
	
	ProjectInfo_t PI(DI.Project());
	FileInfo_t impFile(DI, mrFileImported, mr.memMgrGlob());//work in context of this file
	MyString sm(mpExportingModule ? ProjectInfo_s::ModuleTitle(mpExportingModule) : MyString());//old
	
	MyStringEx aName(symIt.symbol());
	MyString sMangled(aName[0]);
	MyString sTag(aName[1]);
	MyString sModuleExp(aName[2]);

//CHECK(sModuleExp.startsWith("MFC"))
//STOP

	aName = DI.Adjusted(aName);//fix module

	if (sModuleExp.isEmpty())
	{
		sModuleExp = "__unresolved_externals";
		if (sModuleExp != sm)
		{
			mpExportingModule = DI.projx().unresolvedExternalsModule();
			if (!mpExportingModule)
			{
				FolderPtr pFolder(PI.CreateModuleFolder(sModuleExp));
				mpExportingModule = pFolder->fileModule()->module();
				mr.AssureDC(mpExportingModule);
				DI.projx().setUnresolvedExternalsModule(mpExportingModule);
			}
		}
	}
	else if (sModuleExp != sm)
	{
		int iCase(DI.IsArchCaseSensitive());
		mpExportingModule = PI.FindModule(sModuleExp, iCase == 1);
		assert(mpExportingModule);
	}
	assert(mpExportingModule);

CHECK(sMangled == "_ZTVSt11logic_error")
STOP
	
//fprintf(stdout, "--> %d: %s\n", zi++, sMangled.c_str());

	MyString sDemangled;
	I_Front::SymbolKind eKind(symIt.demangled(sDemangled));
	ADDR va(symIt.va());
	bool bIsFunc(symIt.isFunc());

CHECK(va == 0x206AD50c)
STOP

	// [x] Instantiate symbol (global or function) in exporting module 
	//		... and create a reference (ptr to void) field in current
	// [x] If there are any strucs created during expression parseout, add their definitions to the exporting module
	//		... and proxies to the current

	FieldPtr pImpField(nullptr);
	if (va != 0)
	{
		Locus_t aLoc;
		pImpField = DI.FindFieldInSubsegs(DI.PrimeSeg(), va, aLoc);
		if (pImpField)
		{
CHECKID(pImpField, 0xa6a5)
STOP
			unsigned off;
			if ((pImpField = aLoc.stripToSeg(off)) != nullptr && off != 0)
				pImpField = nullptr;
		}
	}

	node_t *pn(nullptr);
	if (!sDemangled.isEmpty() && sDemangled != sMangled)
		pn = DI.projx().cxxSymbols().addSymbol(sMangled, sDemangled);
	if (!pn)//try signatures/export map
	{
		const StubSet& aSet(mr.loadSignatures(sModuleExp));
		if (!aName[0].isEmpty())
			pn = aSet.findProto(aName);
//		else//try ordinal (sometimes the ordianls of named import/export entries do not match, so try ordinals as a last resort)
	//		pn = aSet.findProtoEx(aName);
	}

	if (eKind >= 0x10)
	{
		STOP
	}

	Dc_t* pDC2(DCREF(mpExportingModule));
	if (!pDC2)
		pDC2 = mr.AssureDC(mpExportingModule);
	DcInfo_t DC2(*pDC2);
	FolderPtr pExpFolder(DC2.AddFileEx(FPATH_FROM_EXPORTED));//?
	FileDef_t& rFileDef(*pExpFolder->fileDef());
	FileInfo_t expFile(DC2, rFileDef, mr.memMgrGlob());
	FieldPtr pExpField(expFile.GetExportedField(aName));
	if (!pExpField)
		pExpField = expFile.CreateExportedField(aName, sDemangled, bIsFunc);

	//try
	//{
		if (pExpField)
		{
			GlobPtr pExpGlob(DcInfo_s::GlobObj(pExpField));
			if (pExpGlob)
			{
				if (!pExpField->type())//was it already added? (this is a phantom binary, right?)
				{
					TypeFomNode_t conv(expFile, mr.stubsPool(), true);

					conv.ProcessExportedEntry(pExpGlob, pn, eKind, aName, bIsFunc);//process a symbol in exporting (other) module

					conv.ProcessNewTypes(impFile);//establish proxies to other module's newly created types
				}

				if (!DcInfo_s::OwnerScope(pExpGlob) && pExpField->hasUglyName())//top-levels only
				{
					if (!pn && !aName[0].isEmpty())
						expFile.ApplyPrettyName(pExpGlob, aName[0], nullptr);//pretty name for exporting symbol (other module)
				}
			}
		}
	//}
#if(0)
	catch (int err)
	{
		PrintError() << "Failed to process imported symbol in module '" << ModuleTitle(ModulePtr()) << "' (code=" << err << "):"
			<< std::endl << "\t" << sDemangled << std::endl;
	}
	catch (...)
	{
		PrintError() << "<unspecified>" << std::endl;
	}
#endif

	//CHECK(count == count_limit)
	//CHECK(count == 0x2b7)
	//STOP
	//if (count > count_limit) return;
	//fprintf(stdout, "Importing object %s: %s\n", sVA.c_str(), sMangled.c_str());
	if (pImpField)
	{
//CHECKID(pImpField, 1860)
//STOP

		if (!DcInfo_s::IsTypeImp(pImpField))
			return;

		// [x] Add import field reference to the file

		impFile.AddImpField(pImpField);
	}
}








///////////////////////////////////////////////////// StageStubs_t
StageStubs_t::StageStubs_t(PostAnalysis_t& r)
	: StageBase_t(r, -1)
{
}

bool StageStubs_t::run()
{
	DcInfo_t DI(mr.dc());
	StubInfo_t SI(DI);
	SI.LoadStubs();
	return false;//stop the stage
}



///////////////////////////////////////////////////// StageResources_t
StageResources_t::StageResources_t(PostAnalysis_t& r)
	: StageBase_t(r, -1)
{
}

bool StageResources_t::run()
{
	ModuleInfo_t MI(mr.dc().project(), *mr.dc().module());
	MI.CheckResources();
	return false;//stop the stage
}



///////////////////////////////////////////////////// StageResources_t
StageSignatures_t::StageSignatures_t(PostAnalysis_t& r)
	: StageBase_t(r, -1)
{
}

bool StageSignatures_t::run()
{
	mr.loadSignatures();
	return false;//stop the stage
}




///////////////////////////////////////////////////

std::vector<double> PostAnalysis_t::gStats(STAGES_TOTAL, 0.0);

PostAnalysis_t::PostAnalysis_t(ProjectEx_t &rProjRef, unsigned segAffinity, Dc_t &rDC)
: //BinaryAnalysis_t(rProjRef, rDC.module(), segAffinity),
mrDC(rDC),
mProgress(STAGES_TOTAL, 0)
{
	//if (rProjRef.exp TypesMap().empty())
		//rProjRef.rebuildTypesMap();
	RebuildTypesMap();
}

PostAnalysis_t::~PostAnalysis_t()
{
#if(0)
	for (size_t i(0); i < gStats.size(); i++)
		fprintf(stdout, "[%d]=%g ", (int)i, gStats[i] / 1000);
	fprintf(stdout, "\n");
#endif

	while (!mStages.empty())
		popStage();

	int done(0);
	for (int i(0); i < STAGES_TOTAL; i++)
	{
		if (mProgress[i] == 0)
			continue;
		if (done == 0)
			fprintf(stdout, "%s: ", mrDC.moduleRef().title().c_str());
		else
			fprintf(stdout, ", ");
		if (i == STAGE_EXPORTS)
			fprintf(stdout, "%d symbol(s) exported", mProgress[i]);
		else if (i == STAGE_IMPORTS)
			fprintf(stdout, "%d symbol(s) imported", mProgress[i]);
		else if (i == STAGE_DBGINFO)
			fprintf(stdout, "%d debug info symbol(s) examined", mProgress[i]);
		//else if (i == STAGE_MAPINFO)
			//fprintf(stdout, "%d map info symbol(s) examined", mProgress[i]);
		done++;
	}
	if (done > 0)
		fprintf(stdout, "\n");

#if(0)
	const FoldersMap &m(mrProject.rootFolder().children());
	for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
	{
		FolderPtr pFolder(i.pvt());
		//const Module_t &aBin(*pFolder->miBinary->typeModule());
		Dc_t *pDC(DCREF(pFolder->miBinary));
		if (pDC)
			pDC->mTypeCache.clear();
	}
#endif
}

void PostAnalysis_t::addStage(StageBase_t* pStage)
{
	mStages.push_back(pStage);
}

void PostAnalysis_t::RebuildTypesMap()
{
	const FoldersMap &m(mrDC.project().rootFolder().children());
	for (FoldersMap::const_iterator it(m.begin()); it != m.end(); it++)
	{
		CFolderRef rFolder(*it);
		TypePtr iModule(ProjectInfo_s::ModuleOf(&rFolder));
		Dc_t *pDC(DCREF(iModule));
		if (pDC && pDC->primeSeg())
		{
			DcInfo_t DI(*pDC);
			//RebuildTypesMap(DI);
			RebuildTypesMap2(DI);
		}
	}
}

/*void PostAnalysis_t::RebuildTypesMap(const DcInfo_t &DI)
{
	std::list<TypesMgr_t::OrderIterator> l;//stack
	l.push_back(TypesMgr_t::OrderIterator(*DI.PrimeSeg()->typeSeg()->typeMgr()));

	while (!l.empty())
	{
		TypesMgr_t::OrderIterator &i(l.back());
		if (!i)
		{
			l.pop_back();
			continue;
		}

		TypePtr iType(*i);
		if (iType->isExporting())
		{
			MyString s(DI.TypeNameFull(iType));
			exp TypesMap().addType(s, iType);
			if (iType->typeStruc())
			{
				TypesMgr_t *ptm(iType->typeStruc()->typeMgr());
				if (ptm)
				{
					++i;
					l.push_back(TypesMgr_t::OrderIterator(*ptm));
					continue;
				}
			}
		}

		++i;
	}
}*/

void PostAnalysis_t::RebuildTypesMap2(const DcInfo_t &DI)
{
	FolderPtr pFolderTypes(DI.FolderOfKind(FTYP_TYPES));
	if (!pFolderTypes)
		return;
	FileTypes_t *rFile(pFolderTypes->fileTypes());
	const std::set<TypePtr> &m(rFile->typeMaps());
	for (std::set<TypePtr>::const_iterator i(m.begin()); i != m.end(); i++)
	{
		TypesMgr_t *ptm((*i)->typeMgr());
		for (TypesMapCIt it2(ptm->aliases().begin()); it2 != ptm->aliases().end(); it2++)
		{
			TypePtr pType(it2->pSelf);
			if (!pType->hasUserData())
				continue;
			if (pType->typeProxy())
				continue;
			FolderPtr iFolder(USERFOLDER(pType));
			if (iFolder)
			{
				MyString s(DI.TypeNameFull(pType).join());
				mrDC.project().addExpTypesMap(s, pType);
			}
		}
		
	}
}

//given a file's name, return a path in proto dir
MyPath PostAnalysis_t::protoPath(MyString aName) const
{
	const I_FrontDC &frontDC(*mrDC.frontDC());
	//MyPath path1(mrMain.exePath());
	MyString frontName(frontDC.Name());
	MyPath path(MAIN.frontPath(frontName));
	MyString protoDir(path.Dir() + "proto");
	path.SetDir(protoDir);
	if (!aName.isEmpty())
		path.SetName(aName);
	return path;
}

StubSet& PostAnalysis_t::loadSignatures()
{
	const I_FrontDC &frontDC(*mrDC.frontDC());

	//load stubs associated with a front end

	MyPath path(protoPath());
	if (path.Name().startsWith("lib"))
		path.SetName(path.Name().mid(3));
	path.SetExt("stubs.hh");

	C_PP_t ccp(path);
#ifdef _DEBUG
	if (!ccp)
	{
		MAIN.printError() << "Could not open " << path.Path() << std::endl;
		return mStubPool;
	}
#endif

	for (const char **pp = frontDC.Symbols(); pp && *pp; pp++)
		ccp.addSymbol(*pp);

	mStubPool.setName(path.Name());
	for (int n(0); ccp; ++ccp, n++)
	{
		node_t *node(SymParse(ccp.mStatement));
		if (node && !mStubPool.add(node))
			delete_node(node);
		//std::cout << ccp.mStatement << std::endl;
	}

	//mStubPool.dump(std::cout);
	return mStubPool;
}

StubSet& PostAnalysis_t::loadSignatures(MyString module)
{
	//const I_FrontDC &frontDC(*mrDC.frontDC());
	if (mStubPool.hasModule(module))
		return mStubPool.findModule(module);//already loaded

	assert(!module.isEmpty());
	StubSet& aSet(mStubPool.assureModule(module));

	MyPath pathHdr(protoPath(module + ".h"));

	C_PP_t ccp;
	ccp.skipInclude(mStubPool.name());//the include to skip - already has been processed
	ccp.parse(pathHdr);
	for (int n(0); ccp; ++ccp, n++)
	{
		node_t *node(SymParse(ccp.mStatement));
		if (node && !aSet.add(node))
			delete_node(node);
		//std::cout << ccp.mStatement << std::endl;
	}
	return aSet;
}

bool PostAnalysis_t::finished()
{
	if (!mStages.empty())
		return false;
	return true;
	//return BinaryAnalysis_t::finished();
}

int PostAnalysis_t::process()
{
	if (!mStages.empty())
	{
		QxTime _timer;
		_timer.start();
		StageBase_t *pStage(mStages.front());
		bool bRet(pStage->run());
		if (pStage->id() < STAGES_TOTAL)
			gStats[pStage->id()] += _timer.elapsed();
		if (!bRet)
			popStage();
		return 1;
	}
	return 0;// BinaryAnalysis_t::process();
}

bool PostAnalysis_t::popStage()
{
	if (!mStages.empty())
	{
		StageBase_t *pStage(mStages.front());
		if (pStage->id() < STAGES_TOTAL)
			mProgress[pStage->id()] = pStage->progress();
		mStages.pop_front();
		delete pStage;
	}
	return !mStages.empty();
}

//add an exporting module to a top-level module folder list
Dc_t* PostAnalysis_t::AssureDC(TypePtr pModule)
{
	ProjectInfo_t PI(mrDC.project());

	//check if exporting binary has no dc yet
	Dc_t *pDC(DCREF(pModule));
	if (!pDC)
	{
		//add a dummy front seg (without seg range)
		Seg_t *pSeg(new Seg_t);
		TypePtr iSeg(mrDC.memMgr().NewTypeRef(pSeg));
		pSeg->setTitle(ProjectInfo_s::ModuleTitle(pModule));//?
		pSeg->setPhantom(true);
		PI.RegisterSubseg(pModule, 0, iSeg);
		pSeg->setFrontIndex(PI.NewFrontEnd(mrDC.front().name(), 0));//clone the front
		ModuleInfo_t MI(mrDC.project(), *pModule);
		MI.AssureNamespace(iSeg);
		MI.AssureTypeMgr(iSeg);

		pDC = new Dc_t(mrDC.project(), iSeg, pModule);
		pDC->setFrondDC(mrDC.frontDC());
	}

	DcInfo_t DI(*pDC);
	FolderPtr pFolder(DI.AddFileEx(FPATH_FROM_EXPORTED));
	DI.AssureFileDef(pFolder, false);
	return pDC;
}




