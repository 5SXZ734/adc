#include "main.h"
#include "prefix.h"
#include <fstream>

#include "qx/MyDir.h"
#include "qx/MyFileMgr.h"
#include "shared/defs.h"
#include "mem.h"
#include "command.h"
#include "script.h"
#include "ui_main.h"
#include "interface/IADCGui.h"
#include "field.h"
#include "type_proc.h"
#include "proj.h"
#include "type_code.h"
#include "anlzbin.h"
#include "clean.h"
#include "front_impl.h"
#include "save.h"
#include "save_impl.h"


#ifdef _DEMO
int ERRMODE = 1;//don't mess with it
#else
int ERRMODE = 0;
#endif

using namespace adc;

Main_t * gpMain = nullptr;

void EmergencyClose()
{
	static bool bDone = false;
	if (bDone)
		return;
	//?	uint32_t u = G DC.m_flags;
	//?	G DC.m_flags |= DC_DC_;

#ifndef _DEMO
	//	SCRIPT.CloseScripts();
#endif
	//?	GD C.m_flags = u;
	bDone = true;
}

void __assert(void *file, unsigned line)
{
#ifdef _DEBUG
	if (ERRMODE)
#endif
	{
		fprintf(STDERR, "Fatal Error: Assertion failed in %s, line %d\n", (char *)file, line);
	}
#ifdef _DEBUG
	else
	{
		EmergencyClose();
		__assert((char *)file, line);//__FILE__, __LINE__);
	}
#endif
}

//static I_UI sUI;


//////////////////////////////////////////////////////////

//id2fe_t	* RegisterFrontend_t::m = nullptr;

static IGui_t guiProxy;

////////////////////////////////////////////////////////////////

Main_t::Main_t(int argc, char** argv, const char* appName, const char *appCodeName, const char* appVersion, const char* comanyName, const Options_t& opt)
	: My::EventLoop(argc, argv),
	CMDServer_t(mCmdMap),
	mAppName(appName),
	mAppCodeName(appCodeName),
	mAppVersion(appVersion),
	mCompanyName(comanyName),
	mpGui(&guiProxy),
	m_bDebugMode(false),
	m_dwDumpFlags(0),
	mpProject(nullptr),
	mbUISignalsBlocked(false),
	mxProject(2),//recursive?, only 2 readers (core & gui)
	mpMemoryMgr(nullptr),
	//mFrontImpl(*this),
	mOptions(opt),
	mpScript(nullptr),
	mWriteLock(0),
	mReadLock(0)
{
	m_dwDumpFlags |= adcui::DUMP_FUNCEX;
	m_dwDumpFlags |= adcui::DUMP_BLOCKS;

	mExePath = MyPath(argv[0]);

	gpMain = this;
}

Main_t::~Main_t()
{
	for (FrontMapIt i(mFrontends.begin()); i != mFrontends.end(); i++)
		delete i->second;

	delete mpScript;
	gpMain = nullptr;
}

void Main_t::setGui(IGui_t* p)
{
	if (!p)
		p = &guiProxy;
	mpGui = p;
}

const char* Main_t::colorTag(adcui::LogColorEnum e) const
{
	//if (startupInfo().bConsole)
	if (ADCRedirect::instance() == 0)
		return "";
	return gui().GuiColorTag(e);
}

std::ostream& Main_t::printWarning() const
{
	return std::cout << colorTag(adcui::COLORTAG_ORANGE) << "Warning: " << colorTag(adcui::COLORTAG_OFF);
}

std::ostream& Main_t::printError() const
{
	return std::cout << colorTag(adcui::COLORTAG_RED) << "Error: " << colorTag(adcui::COLORTAG_OFF);
}

std::ostream& Main_t::printInfo() const
{
	return std::cout;
}

FullName_t Main_t::hyperLinked(const FullName_t& a) const
{
	FullName_t b;
	for (FullName_t::const_iterator i(a.begin()); i != a.end(); ++i)
	{
		std::ostringstream ss;
		ss << colorTag(adcui::COLORTAG_HYPERLINK) << *i << colorTag(adcui::COLORTAG_HYPERLINK_OFF);
		b.append(ss.str());
	}
	return std::move(b);
}

My::IUnk *Main_t::createGuiBroker(My::IGui *pIGui, bool bEnableOutputCapture)
{
	Gui_t *pGui(::CreateADBGuiBroker(pIGui, *this, bEnableOutputCapture));
	setGui(pGui);
	return pGui->core();
}

void Main_t::onCreate(const SturtupInfo_t& si)//the very first event
{
	BuildDynamicTypesList();//ask frontends

	if (!si.sTarget.empty())
	{
		// start a new project
		MyPath a(si.sTarget);
		MyString s(si.sTarget);
		if (s.find(' ') > 0)
			s = "'" + s + "'";
		if (a.Ext() == PRIMARY_EXT && !si.bForceNew)
			postEvent(new adc::CEventCommand(MyStringf("open %s", s.c_str())));
		else
			postEvent(new adc::CEventCommand(MyStringf("new %s", s.c_str())));
	}
	if (!si.sScript.empty())
	{
		postEvent(new adc::CEventScript(si.sScript, si.bScriptAbortIf));
	}
}

typedef I_Front * createfe_t(I_Module *);

void Main_t::askFrontend(const MyPath &fPath)
{
	FrontInfo_t *p(new FrontInfo_t(fPath));
	if (!p->loadInitial(*this))
	{
		printError() << "Could not load frontend: " << fPath.Path() << std::endl;
		delete p;
		return;
	}

	mFrontends.insert(std::make_pair(p->name(), p));

	/*	MyString frontName(MyString(pIFront->name()));
		if (!frontName.startsWith(FENAME_PFX))
		{
		PrintError() << "Frontend's name (" << frontName << ") must start with " << FENAME_PFX << std::endl;
		delete p;
		return;
		}

		frontName.remove(0, (unsigned)MyString(FENAME_PFX).length());
		mFrontends.insert(std::make_pair(frontName, p));
		p->setName(frontName);*/

	//run initial setup
	//FrontImpl_t tmp(*this, *p);
	//pIFront->RegisterTypes(tmp);

#ifdef WIN32
	//?	mFmtFactory.clear();
	p->unload(*this, true);
#endif
}

void Main_t::releaseFrontend(const char *symbolName)
{
	MyString frontDll;
	MyString sym(FrontInfo_t::fromFrontKey(symbolName, frontDll));
	FrontMapIt i(mFrontends.find(frontDll));
	if (i == mFrontends.end())
		return;
	FrontInfo_t &r(*i->second);
	r.unload(*this);
}

I_FrontMain *Main_t::findFrontByName(const char *frontName)
{
	FrontMapIt i(mFrontends.find(frontName));
	if (i == mFrontends.end())
		return nullptr;
	return i->second->load(*this);
}

MyPath Main_t::frontPath(const char *frontName) const
{
	FrontMapCIt i(mFrontends.find(frontName));
	if (i == mFrontends.end())
		return MyPath();
	return i->second->path();
}

int Main_t::writeExternalTypes(MyStreamBase &ss) const
{
	int count(0);
	MyStreamUtil ssh(ss);
	for (FrontMapCIt i(mFrontends.begin()); i != mFrontends.end(); i++)
	{
		const FrontInfo_t &r(*i->second);
		for (CreateTypeMapCIt j(r.mFmtFactory.begin()); j != r.mFmtFactory.end(); j++)
		{
			const std::string &s(j->first);
			ssh.WriteString(s);
		}
	}
	return count;
}

void Main_t::clearExternalTypes()
{
	for (FrontMapIt i(mFrontends.begin()); i != mFrontends.end(); i++)
	{
		FrontInfo_t &r(*i->second);
		for (CreateTypeMapIt j(r.mFmtFactory.begin()); j != r.mFmtFactory.end(); j++)
		{
			if (j->second)
			{
				r.unload(*this);
				j->second = nullptr;
			}
		}
	}
}


class FrontIter_t : public MyDirIterator
{
	Main_t &m_main;
public:
	FrontIter_t(const MyPath &f, Main_t &a)
		: MyDirIterator(f),
		m_main(a)
	{
	}
	~FrontIter_t()
	{
	}
protected:
	virtual void OnEntry(const MyPath &f, int)
	{
		std::string a(Name());
		std::string b(f.Name());
		if (MatchWildcard(a.c_str(), b.c_str()))
		{
			//fprintf(stdout, "%s\n", b.c_str()); fflush(stdout);
			m_main.askFrontend(f);
		}
	}
};

void Main_t::BuildDynamicTypesList()
{
	//?	mExtTypeRefs.clear();

	FrontIter_t frontIt(exePath(), *this);
	frontIt.SetName(LIB_PFX "front*." DLL_EXT);

	frontIt.readDirEntries(MyDirIterator::Files | MyDirIterator::NoSymLinks);
}




/*I_Front * Main_t::loadFrontend( FEID_t feid )
{
if (feid == FE_NULL)
{
if (ghFRONTENDDLL)
{
FreeLibrary( ghFRONTENDDLL );
ghFRONTENDDLL = nullptr;
}
return nullptr;
}

if (mpIFront)
{
if (mpIFront->GetID() == feid)
return mpIFront;

mpIFront->Release();
mpIFront = nullptr;
FreeLibrary( ghFRONTENDDLL );
ghFRONTENDDLL = nullptr;
}

LPCTSTR lpszFileName = nullptr;
switch ( feid )
{
default:
case FE_X86_32:
lpszFileName = "front.x86.32.dll";
break;
case FE_X86_16:
lpszFileName = "front.x86.16.dll";
break;
case FE_MPL:
lpszFileName = "front.mpl.dll";
break;
case FE_JAVA:
lpszFileName = "front.java.dll";
}

ghFRONTENDDLL = LoadLibrary( lpszFileName );
if ( !ghFRONTENDDLL )
{
std::string path( mrMain.GetFrontDir() );
path += "/";
path += lpszFileName;

ghFRONTENDDLL = LoadLibrary(path.c_str());
if ( !ghFRONTENDDLL )
return nullptr;
}

createfe_t * pf = (createfe_t *)GetProcAddress( ghFRONTENDDLL, "CreateFrontend" );
if ( !pf )
return nullptr;

mpIFront = (*pf)( this );
return mpIFront;
}*/

/*Main_t::FrontInfoMapIt Main_t::loadFrontendIt(const MyPath &path)
{
FrontInfoMapIt ifIt = mFrontends.find(path.Path());
if (ifIt != mFrontends.end())
{
FrontInfo_t &fi = ifIt->second;
if (fi.pIFront)
{
//fi.pIFront->AddRef();
fi.nRefs++;
return ifIt;
}
}

FrontInfo_t fi;
//Type_t * pType = nullptr;
fi.hModule = ::LoadLibrary(path.Path().c_str() );
if (!fi.hModule)
return mFrontends.end();

createfe_t * pf = (createfe_t *)::GetProcAddress( fi.hModule, "CreateFrontend" );
if (pf)
{
fi.pIFront = (*pf)( gpMain );
if (fi.pIFront)
{
if (ifIt == mFrontends.end())
{
std::pair<FrontInfoMapIt, bool> it;
it = mFrontends.insert(pair<string, FrontInfo_t>(path.Path(), fi));
return it.first;
}

ifIt->second.hModule = fi.hModule;
ifIt->second.pIFront = fi.pIFront;
return ifIt;
}
}

::FreeLibrary( fi.hModule );
return mFrontends.end();
}*/


MyPath Main_t::frontEnd2Path(I_FrontMain *pIFront)
{
	assert(0);/*
	for (size_t i(0); i < mFrontends.size(); i++)
	if (mFrontends[i].front() == pIFront)
	return mFrontends[i].path();*/
	return MyPath();
}



void Main_t::resumeAnalysis(StopFlag eStop)
{
	if (mpProject && project().analyzer())
	{
		project().analyzer()->setStopFlag(eStop);
		postEvent(new SxCustomEvent((SxEventEnum)adc::EVENT_ANLZ_RESUME, nullptr));
	}
}

bool Main_t::writeToDoList(MyStreamBase &ss)
{
	if (!mpProject || !project().analyzer())
		return 0;
	return project().analyzer()->writeToDoList(ss);
}

/*I_Front * Main_t::front()
{
if (!mpIFront)
loadFrontend(FE_X86_32);//default
return mpIFront;
}*/

void Main_t::setFlags(uint32_t f)
{
	m_dwDumpFlags |= f;
}

void Main_t::clearFlags(uint32_t f)
{
	m_dwDumpFlags &= ~f;
}

void Main_t::SaveToStream(std::ostream &os, MyString path, unsigned)
{
	MemmoryAccessorG_t<GlobalSerializer_t> SAVELOAD(SR_Saving, *mpMemoryMgr, *mpProject, path);

	SAVELOAD.Save(os);
}

void Main_t::LoadFromStream(std::istream &is, MyString path)
{
	assert(!mpProject);
	mpProject = newProject(*mpMemoryMgr);
	mpProject->setSelfObj(mpMemoryMgr->NewTypeRef(mpProject));

	MemmoryAccessorG_t<GlobalSerializer_t> SAVELOAD(SR_Loading, *mpMemoryMgr, *mpProject, path);

	SAVELOAD.Load(is);

	GlobalRecoverer_t rec(*mpProject);
	rec.recover();

}

bool Main_t::OpenScript(const MyString &path, bool bAbortIf)
{
	assert(!mpScript);
	mpScript = new ScriptMgr_t(path, bAbortIf);
	return true;
}

void Main_t::CloseScript()
{
	delete mpScript;
	mpScript = nullptr;
}

ScriptMgr_t* Main_t::OpenScript()
{
	if (!mpScript)
		mpScript = new ScriptMgr_t;
	return mpScript;
}

void Main_t::NewGlobalMemMgr()
{
	assert(!mpMemoryMgr);
	mpMemoryMgr = new MemoryMgr_t();
#ifdef _DEBUG
	mpMemoryMgr->mName = "$GLOBAL";
#endif
}

void Main_t::DeleteGlobalMemMgr()
{
	delete mpMemoryMgr;
	mpMemoryMgr = nullptr;
}

bool Main_t::SaveToFile(MyString path, unsigned flags)
{
	MyPath fPath(path);
	if (fPath.Ext().empty())
		fPath.SetExt(PRIMARY_EXT);
	path = fPath.Path();

	std::ofstream ofs(path, std::ios::out | std::ios::binary);
	if (!ofs.is_open())
	{
		printError() << "Can't open file " << path << std::endl;
		path.clear();
		return false;
	}

#if(!DEBUG_SAVE)
	try 
#endif
	{
		SaveToStream(ofs, path, flags);
		fprintf(stdout, "Project saved: %s\n", path.c_str());
		project().setPath(path);
	}
#if(!DEBUG_SAVE)
	catch (...)
	{
		printError() << "Failed to save project: " << path << std::endl;
		return false;
	}
#endif

	return true;
}

bool Main_t::LoadFromFile(MyString sFileName)
{
	MyPath fPath(sFileName);

	std::ifstream ifs;
	ifs.open(fPath.Path(), std::ios::binary);
	if (ifs.fail())
	{
		if (fPath.Ext().empty())
		{
			ifs.clear();
			fPath.SetExt(PRIMARY_EXT);
		}
		ifs.open(fPath.Path(), std::ios::binary);
	}

	if (!ifs.is_open())
	{
		printError() << "Can't open file " << sFileName << std::endl;
		return false;
	}

	NewGlobalMemMgr();
	assert(!hasProject());
	//MemmoryAccessor_t memAcc(*mpMemoryMgr, SR_Loading);
	//GlobalSerializerEx_t SAVELOAD(memAcc);
#if(!DEBUG_SAVE)
	try
#endif
	{
		LoadFromStream(ifs, fPath.Path());
	}
#if(!DEBUG_SAVE)
	catch (const std::string &sErr)
	{
		closeProject();
		//DeleteGlobalMemMgr();
		printError() << "Failed to load project " << sFileName << " (" << sErr << ")" << std::endl;
	}
	catch (...)
	{
		closeProject();
		//DeleteGlobalMemMgr();
		printError() << "Failed to load project " << sFileName << " (unknown reason)" << std::endl;
	}
#endif

	//ifs.close();

	if (hasProject())
	{
		project().setPath(fPath.Path());
		fprintf(stdout, "Project loaded: %s\n", fPath.Path().c_str());
	}

	return true;
}

static void SubstituteEnv(MyString &iString)
{
	MyString oString(iString);

	size_t iLeftChevronPos = oString.find("%");
	if (iLeftChevronPos == MyString::npos)
		return;

	size_t iRightChevronPos = oString.find("%", (int)iLeftChevronPos + 1);

	/* Make sure the string has chevrons, that the left chevron is on
	the left and that they are
	not next to each other */
	if (iLeftChevronPos > iRightChevronPos || (iRightChevronPos - iLeftChevronPos) == 1)
		return;

	MyString oEnv = oString.substr(iLeftChevronPos + 1, (iRightChevronPos - (iLeftChevronPos + 1)));
	MyString sValue;

	if (oEnv.lower() == "cd")
		sValue = MyPath("").Dir(true);
	else
	{
		char* pcEnv = getenv(oEnv.c_str());
		if (!pcEnv)
			return;
		sValue = pcEnv;
	}

	oString.replace((unsigned)iLeftChevronPos, unsigned(iRightChevronPos - iLeftChevronPos + 1), sValue);

	iString = oString;
	// Recurse in case of multiple env statements
	MyString temp;
	while (temp != iString)
	{
		temp = iString;
		SubstituteEnv(iString);
	}
}

bool Main_t::CheckEvents()
{
	if (hasProject())
	{
		if (project().checkAnalyzerStatus())//analizer can be terminated here
			return true;//a new event has been posted or entered a paused mode
	//	checkUiEvents(true);//force UI update
		STOP
	}
	if (isProcessingScript())
	{
		if (!script().empty())
		{
			MyString sCmd(script().front());
			SubstituteEnv(sCmd);

			//fprintf(stdout, "-->%s\n", sCmd.c_str());
			script().pop_front();
			CEventCommand *e(new CEventCommand(sCmd));
			e->setRefPath(script().path());
			I_Context *pICtx(script().context());
		//	assert(pICtx->RefsNum() == 1);
			//pICtx->setHint(pICtx->hint() + 1);
			e->setContextZ(pICtx);
			postEvent(e);
			return true;
		}
		CloseScript();
	}
	return false;
}

void Main_t::OnIdle()
{
	if (CheckEvents())
	{
		if (checkStopRequest() > 0)//step mode?
		{
			gui().GuiOnAnalizerPaused();
			checkUiEvents(true);//force UI update
		}
		return;
	}

#if(0)
	static int xx = 0;
	fprintf(stdout, "IDLE(%d)\n", ++xx);
	fflush(stdout);
#endif

	//assert(gui().self());//No idling in batch mode!
	if (!gui().igui())//batch mode? nothing else to proceess? exit!
	{
		My::EventLoop::exit();
		return;
	}

	gui().GuiOnAnalizerStopped();

	checkUiEvents(true);//force UI update

	if (options().bTimeStats)
	{
		double dtime(mStatTimer.elapsed() / 1000.0);
		mStatTimer.restart();
		if (dtime > 1.0)
		{
			fprintf(stdout, "*** operation time: %g sec\n", dtime);
			fflush(stdout);
		}
	}

	if (!debugMode())
		gui().GuiOnShowProgressInfo("");
}

void Main_t::OnBusy()
{
#if(0)
	static int xx = 0;
	fprintf(stdout, "BUSY(%d)\n", ++xx);
	fflush(stdout);
#endif
	mStatTimer.start();
#if(0)
	Project_t *pProj(project());
	if (pProj && pProj->analyzer())
	{
		pProj->checkAnalyzerStatus();
		if (!pProj->analyzer())
			return;
		gui().GuiOnAnalizerStarted();
	}
#else
	mUiNotifier.start();
	gui().GuiOnAnalizerStarted();
#endif
}

void Main_t::OnTimeout()
{
#if(0)
	static int z = 0;
	fprintf(stdout, "TIMEOUT(%d)\n", ++z);
	fflush(0);
#endif
	assert(gui().igui());//no timeouts in batch mode!
}

bool Main_t::canQuit()
{
	//Sleep(1000);
	CloseProject();// nullptr);
	return My::EventLoop::canQuit();
}

void Main_t::DestroyProject()
{
	BinaryCleaner_t<> PC(project());
	PC.destroyProject();
}

void Main_t::closeProject()
{
	while (project().analyzer())
		project().popAnalizer();//a few may chained

	//close all opened files in GUI - release view models
	for (FilesMgr0_t::FolderIterator i(mpProject->files()); i; i++)
	{
		CFolderRef rFolder(*i);
		if (!rFolder.fileFolder())
			gui().GuiOnFilePriorRemoved(mpProject->files().relPath(&rFolder));
	}

	DestroyProject();

	//?mpMemoryMgr->Delete(miProject);
	delete mpProject;
	mpProject = nullptr;

	clearExternalTypes();//release front end(s)

	DeleteGlobalMemMgr();
}

int Main_t::CloseProject()//I_Context *pICtx)
{
	if (!hasProject())
		return 0;

	project().setClosing();
	gui().GuiOnProjectAboutToClose();

	{//RAII-block
		WriteLocker lock;

		//clear context
//		if (pICtx)
//			pICtx->clear_all();

		closeProject();

#ifdef MEMTRACE_ENABLED
		//MemTrace_t::print_summary(std::cout);
		MemTrace_t::print_status(std::cout, "MEMTRACE OBJECT LEAKAGE INFO");
		MemTrace_t::clear();
#endif
	}

	gui().GuiOnProjectClosed();
	return 1;
}


static int CheckPortableExecutable(const char *ptr, unsigned size)
{
	if (size < 0x3C)
		return 0;
	//#define IMAGE_DOS_SIGNATURE				0x5A4D      // MZ
	//#define IMAGE_OS2_SIGNATURE				0x454E      // NE
	//#define IMAGE_OS2_SIGNATURE_LE			0x454C      // LE
	//#define IMAGE_VXD_SIGNATURE				0x454C      // LE
	//#define IMAGE_NT_SIGNATURE				0x00004550  // PE00
	//#define IMAGE_PESIG_OFFSET				0x3C
	//#define IMAGE_FILE_HEADER_SIZE			0x14
	uint16_t* pMagic1((uint16_t*)ptr);
	if (*pMagic1 == 0x5A4D)//IMAGE_DOS_SIGNATURE
	{
		unsigned pe_offset(*(unsigned*)(ptr + 0x3C));
		uint32_t* pSig((uint32_t*)(ptr + pe_offset));
		if (*pSig == 0x00004550)//IMAGE_NT_SIGNATURE
		{
			uint8_t* pFileHeader((uint8_t*)(pSig + 1));
			uint16_t* pMagic2((uint16_t*)(pFileHeader + 0x14));//IMAGE_FILE_HEADER_SIZE
			if (*pMagic2 == 0x10b)
				return 1;//PE32
			if (*pMagic2 == 0x20b)
				return 2;//PE64
		}
	}
	return 0;
}

I_DynamicType *Main_t::getDynamicType(const char *typekey, FrontInfo_t **pfi)
{
	MyString frontDll;
	MyString sTypeName(FrontInfo_t::fromFrontKey(typekey, frontDll));
	FrontMapIt i(mFrontends.find(frontDll));
	if (i == mFrontends.end())
		return nullptr;
	*pfi = i->second;
	FrontInfo_t &r(**pfi);
	CreateTypeMapIt j(r.mFmtFactory.find(typekey));
	if (j == r.mFmtFactory.end())
		return nullptr;
	if (j->second)
		return j->second;
	HDYNFUNC pf(r.loadSymbol(*this, sTypeName));
	if (pf)
	{
		r.storeDynamicTypeRef(typekey, (I_DynamicType *)pf);
		return (I_DynamicType *)pf;
	}
	return nullptr;
}

I_DynamicType *Main_t::getContextDependentType(const char *typekey)
{
	MyString frontDll;
	MyString sTypeName(FrontInfo_t::fromFrontKey(typekey, frontDll));
	FrontMapIt i(mFrontends.find(frontDll));
	if (i == mFrontends.end())
		return nullptr;
	FrontInfo_t &r(*i->second);
	HDYNFUNC pf(r.loadSymbol(*this, sTypeName));
	if (pf)
		return (I_DynamicType *)pf;
	return nullptr;
}

I_Code *Main_t::getCodeType(const char *typekey)
{
	MyString frontDll;
	MyString sTypeName(FrontInfo_t::fromFrontKey(typekey, frontDll));
	FrontMapIt i(mFrontends.find(frontDll));
	if (i == mFrontends.end())
		return nullptr;
	FrontInfo_t &r(*i->second);
	HDYNFUNC pf(r.loadSymbol(*this, sTypeName));
	if (pf)
		return (I_Code *)pf;
	return nullptr;
}

I_Front *Main_t::getFrontend(const char *frontkey, const I_DataSourceBase *aRaw)
{
	MyString frontDll;
	MyString frontName(FrontInfo_t::fromFrontKey(frontkey, frontDll));
	FrontMapIt i(mFrontends.find(frontDll));
	if (i == mFrontends.end())
		return nullptr;
	FrontInfo_t &r(*i->second);
	I_FrontMain *pFrontMain(r.load(*this));
	if (!pFrontMain)
		return nullptr;
	//HDYNFUNC pf(r.loadSymbol(*this, frontName));
	//if (pf)
		//return (I_Front *)pf;
	return pFrontMain->CreateFrontend(frontkey, aRaw, this);
}

FieldPtr Main_t::createExternalType(MyString typeKey, Locus_t &addrx, MyString options)
{
	TypePtr iModule(addrx.module());
	Module_t &aModule(*iModule->typeModule());

	ModuleInfo_t MI(project(), *iModule);

	OFF_t blksz(aModule.rawBlock().m_size);
	if (addrx.back().field())
		blksz = MI.rangeFrom(addrx.back().cont(), addrx.back().field()->_key());
	if (blksz == 0)
		return nullptr;

	if (typeKey.empty())
		return nullptr;

	FrontInfo_t *pfi(nullptr);
	I_DynamicType *pf(getDynamicType(typeKey, &pfi));
	if (!pf)
	{
		printError() << "Failed to locate a context-dependent type: " << typeKey << std::endl;
		return nullptr;
	}

	MI.AssureNamespace(iModule);
	MI.AssureTypeMgr();

	FrontImplEx_t fimp(project(), addrx, *pfi, iModule);
	FrontIfaceEx_t iface(fimp);
	fimp.setOptions(options);
	fimp.blockSignals(false);//allow exceptions
	TRY
	{
		pf->createz(iface, (unsigned long)blksz);
	}
	CATCH(int)
	{
		MyString s(fimp.getLastError());
		if (s.empty())
			s = "<no code>";
		printError() << "exception caught: " << s << std::endl;
	}
#if(!NO_TRY)
	catch (...)
	{
		fprintf(stderr, "Error: unknown\n");
	}
#endif

	if (fimp.geomChanged())
	{
		fimp.UpdateViewGeometry2();
		fimp.DumpSegments(fimp.ModulePtr(), std::cout, 0);
	}

	FieldPtr pRoot(MI.Field(addrx.struc(), addrx.addr()));
	project().markDirty(DIRTY_GLOBALS);
	return pRoot;
}

Project_t &Main_t::project() const
{
	assert(mpProject);
	return *mpProject;
}

int Main_t::NewProject(const MyPath &path, I_Context *pICtx, MyString opts)
{
	CloseProject();// nullptr);

	int ret(1);
	Folder_t *pFolder(nullptr);
	{//RAII-block
		WriteLocker lock;
#if(!NO_FILE_ID)
		Folder_t::resetUniqueId();
		TypeCode_t::resetUniqueId();
#endif

#if(NO_OBJ_ID)
		Complex_t::resetUniqueId();
		Typedef_t::resetUniqueId();
#else
		Obj_t::resetUniqueId();
#endif

		pFolder = newProjectFromPath(path);
	}

	if (pFolder)
	{
		assert(mpProject);
		gui().GuiOnProjectNew();

		ProjectInfo_t PJ(project());

		TypePtr iModule(PJ.ModuleOf(pFolder));
		Module_t &aBin(*iModule->typeModule());

		Probe_t *loc(dynamic_cast<Probe_t *>(pICtx));
		if (!loc)
			loc = mpProject->NewLocus(Locus_t());
		else
		{
			//loc->setFolder(pFolder);
			loc->AddRef();
		}
		loc->clear();
		loc->locus().push_back(Frame_t(aBin.rawBlock(), iModule, 0, nullptr));
		//loc->setFolder(pFolder);

		MyString s("preformat");
		if (!opts.isEmpty())
			s.append(" " + opts);
		s.append(" -n " + ProjectInfo_t::ModuleTitle(iModule));

		postContextCommand(s.c_str(), loc);
		loc->Release();
	}
	else
	{
		assert(!mpProject);
	}

	//lockProjectWrite(false);
	return ret;
}

void Main_t::postContextCommand(const MyString &s, I_Context *loc, bool bEcho)
{
	CEventCommand *cmd(new CEventCommand(s, bEcho));
	cmd->setContextZ(loc);
	//loc->Release();
	postEvent(cmd);
}


int Main_t::LoadProject(const char * fname/*, I_Context *pICtx*/)
{
	CloseProject();// pICtx);

	lockProjectWrite(true);
	LoadFromFile(fname);
	lockProjectWrite(false);

	gui().GuiOnProjectOpened();
	return 1;
}

Project_t *Main_t::newProject(MemoryMgr_t &rMemMgr)
{
	return new Project_t(*this, rMemMgr);
}

const char* Main_t::executablePath() const
{
	return 0;
}

const char* Main_t::frontPathFromName(const char* frontName) const
{
	MyPath path(frontPath(frontName));
	static MyString s;
	s = path.Path();
	return s.c_str();
}

const char* Main_t::protoPath(const char* fileName) const
{
	return 0;
}

Folder_t *Main_t::newProjectFromPath(const MyPath &path)
{
	assert(!path.IsNull());
	NewGlobalMemMgr();

	Project_t *pProject(newProject(*mpMemoryMgr));
	TypePtr iProject(mpMemoryMgr->NewTypeRef(pProject));
	pProject->setSelfObj(iProject);

	ProjectInfo_t PI(*pProject);

	//	fprintf(stdout, "New project from binary: %s\n", path.Path().c_str());

	Folder_t *pFolder(PI.LoadBinary(path));
	if (!pFolder)
	{
		printError() << "Could not load module: " << path.Path() << std::endl;

		//BinaryCleaner_t PC(projInfo, nullptr);
		delete pProject;
		DeleteGlobalMemMgr();
		return nullptr;
	}

#if(SHARED_STOCK_TYPES)
	PI.NewTypesMgr(iProject);
	PI.RegisterTypesMap(iProject, true);
	StockTracer_t ST(*pProject, *pProject->typeMgr());
	ST.AddStockTypes();
#endif

	mpProject = pProject;
	//PI.UpdateViewGeometry2();
	return pFolder;
}

void Main_t::OnAnlzReady()
{
	postEvent(new SxCustomEvent((SxEventEnum)adc::EVENT_ANLZ_PROCESS, nullptr));
}

int Main_t::checkStopRequest() const
{
	if (!mpProject || !mpProject->analyzer())
		return 0;
	if (mpProject->analyzer()->stopFlag() == StopFlag::RESET && !debugMode())
		return 0;
	if (mpProject->analyzer()->stopFlag() == StopFlag::ABORT)
		return -1;//abort
	if (!gui().igui())
		return -1;//abort in batch mode
	return 1;
}

int Main_t::processEvent2(const SxCustomEvent *e0)
{
	switch ((adc::EventId)e0->m_id)
	{
	case EVENT_CREATE:
	{
		CEventCreate* e((CEventCreate*)e0);
		onCreate(e->m_si);
		break;
	}
	case EVENT_SCRIPT:
	{
		CEventScript *e((CEventScript *)e0);
		OpenScript(e->m_path, e->m_bAbortIf);
		/*if (hasPendingEvents())
		{
		postEvent(e);//re-post
		}
		else
		{
		if (!e->mScript.empty())
		{
		MyString sCmd(e->mScript.front());
		//fprintf(stdout, "-->%s\n", sCmd.c_str());
		e->mScript.pop_front();
		CEventCommand *e2(new CEventCommand(sCmd));
		e2->setRefPath(e->mScript.path());
		postEvent(e2);
		postEvent(e);//re-post
		}
		else if (e->mScript.empty())
		{
		e->m_autodelete = true;
		}
		}*/
		break;
	}
	case EVENT_COMMAND:
	{
		CEventCommand *e((CEventCommand *)e0);
		MyString sCmd(e->m_command);
		//if (mpProject)
			//sCmd = mpProject->expandCommand(sCmd, e->getCtx<Probe_t>());
		if (e->m_bEcho)
		{
			fprintf(stdout, "-->%s\n", sCmd.c_str());
			fflush(stdout);
			gui().GuiProcessEvents();
		}
		Cmd_t cmd(sCmd.c_str());
		cmd.mpResponce = e->m_result;
		cmd.mRefPath = e->mRefPath;
		cmd.setContext(e->contextZ());
		/*if (!cmd.context())
		{
		cmd.setContext(new Probe_t());
		cmd.context()->Release();
		}*/

		if (hasProject())
		{
			int ret;
			if (project().ExecuteCommand(cmd, &ret))
				return ret;
		}

		if (CMDServer_t::ExecuteCommand(cmd))
			return mRet;

		if (e->m_bEcho)
			printError() << "Unrecognized command: " << e->m_command << std::endl;
	}
	break;
	case EVENT_ANLZ_PROCESS:
		if (hasProject())
		{
			project().execAnalyzer();
			//project()->checkAnalyzerStatus();
		}
		break;
	case EVENT_ANLZ_RESUME:
	{
		OnAnlzReady();
		if (project().analyzer())
			gui().GuiOnAnalizerResumed();
	}
	break;

	case EVENT_DEBUG_TERMINATED:
	{
		ProjectInfo_t projInfo(project());
		projInfo.destroyDebugger();
		break;
	}

	/*case EVENT_COMMAND_REQUEST_STUB:
	{
	CEventRequestStub &e(*(CEventRequestStub *)e0);
	if (project()->analyzer())
	{
	project()->analyzer()->setContextFile(e.m_ctx.mFileName.c_str());
	//gui().GuiShowFile(e.m_ctx.mFileName.c_str());
	}
	break;
	}*/

	//	case ADCEVENT_EXIT:
	//		this->exit(0);
	//		break;
	default:
		break;
	}

	return 0;
}

void Project_t::dispatchDirty() const
{
	if (checkDirty(DIRTY_GLOBALS) || checkDirty(DIRTY_TYPES))
		gui().GuiOnProjectModified();
	if (checkDirty(DIRTY_TYPES))
		gui().GuiOnTypesMapChanged();
	if (checkDirty(DIRTY_LOCALITY))
		gui().GuiOnLocalityChanged();
	if (checkDirty(DIRTY_LOCUS))
		gui().GuiOnLocusChanged();
	if (checkDirty(DIRTY_NAMES))
		gui().GuiOnNameChanged();
	if (checkDirty(DIRTY_TASKLIST))
		gui().GuiOnToDoListChanged();
	if (checkDirty(DIRTY_CURVA))
		if (analyzer())
			gui().GuiOnShowAnalizerInfo(MyStringf("VA: <font color=darkGreen>%08X</font>", analyzer()->currentVA()));
	if (checkDirty(DIRTY_LOCUS_ADJUSTED))
		gui().GuiOnLocusAdjusted();
	if (checkDirty(DIRTY_FILES))
		gui().GuiOnFileListChanged();
}

bool Main_t::checkUiEvents(bool bFlush)
{
	if (!hasProject())
		return false;

	Project_t &proj(project());
	if (!proj.isDirty())
		return false;

#if(1)
	int iElapsed(mUiNotifier.elapsed());
	//fprintf(stdout, "ELAPSED: %d\n", iElapsed);
	if (iElapsed < 100)
		if (!bFlush)
			return false;
#endif

#if(0)
	static int z = 0;
	fprintf(stdout, "GUI FLUSH (%d)\n", z++);
#endif

	proj.dispatchDirty();
	proj.clearDirty();
//	Sleep(0);
	gui().GuiProcessEvents();//give GUI a chance to respond a user's action
	mUiNotifier.restart();
	return true;
}

int Main_t::processEvent(const SxCustomEvent *e0)
{
	static int here = 0;
	here++;
	//?	assert(here == 1);
	//lockProjectWrite(true);
	int ret = processEvent2(e0);
	//cout.flush();
	//cerr.flush();
	fflush(nullptr);
	//lockProjectWrite(false);
	checkUiEvents(false);

	here--;
	return ret;
}

/*CMDQueue_t * Main_t::newCMDQueue()
{
return new CMDQueue_t();
}


CMDQueue_t * Main_t::createCMDQueue()
{
mpCMDQueue = newCMDQueue();
mpCMDQueue->PushCMDServer(this);
return mpCMDQueue;
}

int Main_t::PostCommand(const char * cmdstr)
{
mpCMDQueue->PushCommand(cmdstr);
return 1;
}

int Main_t::CallCommand(const char * cmd, I_String ** ppIResp)
{
CMDServer_t * pCMDServer = mpCMDQueue->CMDServer();
if (pCMDServer)
return pCMDServer->ExecuteCommand(cmd, ppIResp);
return -1;
}*/

///////////////////////////////////////

/*TypesMgr_t * Main_t::typeMgr() const
{
return project().typeMgr();
}*/

int Main_t::loadProject(const MyPath &)
{
	return 0;
}

//////////////////////////////////////////

#define MYCOMMAND_IMPL(name) \
	int Main_t::COMMAND_##name(CMDServer_t *pSelf, Cmd_t &args){ return static_cast<Main_t*>(pSelf)->OnCommand_##name(args); } \
	int Main_t::OnCommand_##name(Cmd_t &args)


// Synopsis: new [[-a] path]
//		-a: add module
MYCOMMAND_IMPL(new)
{
	bool bAddModule(args.RemoveOpt("-a"));
	if (args.size() <= 1)
		return 0;//error in command

	MyPath path(args.back(), args.mRefPath);
	args.pop_back();

	MyString opts(args.AsString(1));
	//MyArgs opts(&*sOpts.begin());

	if (bAddModule)
	{
		if (hasProject())
		{
			ProjectInfo_t PI(project());
			// if this is tottaly unrelated binary - start a new project
			//if (PI.FindModuleFolder(path.Name()))
			{
				DECLARE_CONTEXT(ctx);
				Probe_t& aLoc(ctx);
				int iCase(1);//no clue what arch is
				if (PI.AddModule(path, iCase == 1, aLoc, opts))
					return 1;
			}
		}
	}
	NewProject(path, args.context(), opts);//skip cmd
	if (mpProject)
	{
		gui().GuiProcessEvents();
		return 1;
	}

	return 0;
}

MYCOMMAND_IMPL(script)
{
	if (args.size() > 1)
	{
		MyFile file(MyPath(args[1].c_str()));
		file.ChDir();//not sure if this is OK
		postEvent(new adc::CEventScript(file.Path(), false));
		return 1;
	}
	return 0;
}

bool Main_t::validateProjectPath(MyString &s)
{
	if (!mpProject)
		return false;
	if (s.empty())
	{
		if (mpProject->path().empty())
		{
			gui().GuiOnSaveRequest();
			return false;//1
		}
		s = mpProject->path();
	}
	return true;
}

//Synopsis: save <path>
//	Save project to file <path>
MYCOMMAND_IMPL(save)
{
	MyString path((args.size() > 1) ? args.back() : MyString());

	MainReadLocker lock(this);

	if (!validateProjectPath(path))
		return 0;

	if (!SaveToFile(path, 0))
		return 0;

	gui().GuiOnProjectSaved();
	return 1;
}

MYCOMMAND_IMPL(open)
{
	return LoadProject((args.size() > 1) ? args[1].c_str() : nullptr);// , args.context());
}


Main_t::MainCmdMap_t::MainCmdMap_t()
{
#define MYCOMMAND_REGISTER(arg)	RegisterCommand(#arg, (CMDServerHandlerPtr)Main_t::COMMAND_##arg);
	MYCOMMAND_REGISTER(cmdlist);
	MYCOMMAND_REGISTER(new);
	MYCOMMAND_REGISTER(script);
	MYCOMMAND_REGISTER(save);
	MYCOMMAND_REGISTER(open);
#undef MYCOMMAND_REGISTER
}








