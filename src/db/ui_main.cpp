#include "ui_main.h"
#include "prefix.h"

#include "qx/MyPath.h"
#include "qx/MyStringList.h"
#include <mutex>

#include "interface/IADCGui.h"
#include "shared/data_source.h"
#include "main.h"
#include "proj.h"
#include "dump_bin.h"
#include "field.h"
#include "info_types.h"
#include "ui_bin_view.h"
#include "ui_res_view.h"
#include "ui_files.h"
#include "ui_types.h"
#include "ui_names.h"
#include "ui_exports.h"

using namespace adc;

#define	COMPACT_VIEW	0

/////////////////////////////////////
// MyLineEditBase

MyLineEditBase::MyLineEditBase(Core_t &r)
	: mrCore(r)
{
}

void MyLineEditBase::readData(MyStreamBase &ss)
{
	MyStreamUtil ssh(ss);
	ssh.WriteString(sEditName);
}

void MyLineEditBase::writeData(MyStreamBase &ss)
{
	MyStreamUtil ssh(ss);
	ssh.ReadString(sEditName);
}

int MyLineEditBase::apply()
{
	MyString s("setnam");
	if (!sEditName.empty())
		s += " " + sEditName;
	return post(s);
}

int MyLineEditBase::post(const std::string& s) const
{
	adc::CEventCommand* p(new adc::CEventCommand(s));
	p->setContextZ(mrCore.getContext())->Release();
	mrCore.main().postEvent(p);
	return 1;
}

/////////////////////////////////////////////////////////////// (MyLineEdit)

MyLineEdit::MyLineEdit(Core_t& rCore, MyString s, const Probe_t& aLoc)
	: MyLineEditBase(rCore),
	//mrProbe(r),
	mLocus(aLoc)
{
	mrCore.setEd(this);
	setEditName(s);
}

MyLineEdit::~MyLineEdit()
{
	mrCore.setEd(nullptr);
}

int MyLineEdit::apply()
{
	if (mLocus.entityId() == adcui::COLOR_ARRAY)
	{
		MyString s("editarr");
		s += " " + sEditName;
		return post(s);
	}
	return MyLineEditBase::apply();
}

int MyLineEdit::startPos() const
{
	return mLocus.rangeObj().begin();
}




/////////////////////////////////////
// Gui_t

void Gui_t::GuiProcessEvents() const
{
	igui()->SendEvent(adcui::UIMSG_PROCESS_EVENTS, nullptr);
}

bool Gui_t::GuiNotify(int eId) const
{
	ADCPostData *pData(new ADCPostData());
	MyStream &ss(pData->obj());
	//ss.WriteStream(iss);
	igui()->PostEvent(eId, pData);
	return true;
}

bool Gui_t::GuiNotify(int eId, MyStreamBase &iss) const
{
	ADCPostData *pData(new ADCPostData());
	MyStream &ss(pData->obj());
	ss.WriteStream(iss);
	igui()->PostEvent(eId, pData);
	return true;
}

void Gui_t::GuiOnFileListChanged() const
{
#if(0)
	typedef std::list<Fil e_t *>		listFile_t;
	typedef listFile_t::iterator	listFile_it;

	MyStream ss;
	listFile_t &files = mrMain.files();
	for (listFile_it it(files.begin()); it != files.end(); it++)
	{
		Fi le_t *pFile(*it);
		ss.WriteString(pFile->nam exx());
	}
	pIGui->SendEvent(adcui::UIMSG_FILE_LIST_CHANGED, &ss);
#else
#if(0)
	ADCPostData *pData(new ADCPostData());
	MyStream &ss(pData->obj());

	MyString sProj;
	if (mrMain.project())
	{
		sProj = mrMain.project()->nam exx();
		ss.WriteStringf("%s", sProj.c_str());
		mrMain.project()->files().dump(ss, 1);

		/*for (Folder_t::iterator it(files.begin()); it != files.end(); it++)
		{
		Fil e_t &file(**it);
		const char *s(file.nam exx());
		MyString tabs;
		ss.WriteStringf("%s%s", tabs, s);
		}*/
	}
#endif
	igui()->PostEvent(adcui::UIMSG_FILE_LIST_CHANGED, nullptr);//pData);
#endif
}

void Gui_t::GuiOnShowAnalizerInfo(const MyString &s) const
{
	ADCPostData *pData(new ADCPostData());
	MyStream &ss(pData->obj());
	ss.WriteString(s);
	igui()->PostEvent(adcui::UIMSG_ANALIZER_INFO, pData);
}

void Gui_t::GuiOnShowProgressInfo(const MyString &s) const
{
	ADCPostData *pData(new ADCPostData());
	MyStream &ss(pData->obj());
	ss.WriteString(s);
	igui()->PostEvent(adcui::UIMSG_PROGRESS_INFO, pData);
}

void Gui_t::GuiOnFileModified(const MyString &relPath) const
{
	My::IGui *pIGui(igui());
	if (pIGui)
	{
		ADCPostData *pData(new ADCPostData());
		MyStream &ss(pData->obj());
		ss.WriteString(relPath);
		pIGui->PostEvent(adcui::UIMSG_SRC_DUMP_INVALIDATED, pData);
	}
}

void Gui_t::GuiOnFileRenamed(const MyString &relPathOld, const MyString &relPathNew) const
{
	ADCPostData *pData(new ADCPostData());
	MyStream &ss(pData->obj());
	ss.WriteString(relPathOld);
	ss.WriteString(relPathNew);
	igui()->PostEvent(adcui::UIMSG_FILE_RENAMED, pData);
}

void Gui_t::GuiNewFolderRecoil(const MyString &relPath) const
{
	ADCPostData *pData(new ADCPostData());
	MyStream &ss(pData->obj());
	ss.WriteString(relPath);
	igui()->PostEvent(adcui::UIMSG_NEW_FOLDER_RECOIL, pData);
}

void Gui_t::GuiOnProjectNew() const
{
	GuiNotify(adcui::UIMSG_PROJECT_NEW);
}

void Gui_t::GuiOnProjectSaved() const
{
	GuiNotify(adcui::UIMSG_PROJECT_SAVED);
}

void Gui_t::GuiOnProjectOpened() const
{
	GuiNotify(adcui::UIMSG_PROJECT_OPENED);
}

void Gui_t::GuiOnProjectClosed() const
{
	GuiNotify(adcui::UIMSG_PROJECT_CLOSED);
}


/*void Gui_t::GuiOnToDoListPushedBack(const char *s)
{
	My::IGui *pIGui(gui());
	if (pIGui)
	{
		ADCPostData *pData(new ADCPostData());
		MyStream &ss(pData->obj());
		ss.WriteString(s);
		pIGui->PostEvent(adcui::UIMSG_TODOLIST_PUSHED_BACK, pData);
	}
}

void Gui_t::GuiOnToDoListPushedFront(const char *s)
{
	My::IGui *pIGui(gui());
	if (pIGui)
	{
		ADCPostData *pData(new ADCPostData());
		MyStream &ss(pData->obj());
		ss.WriteString(s);
		pIGui->PostEvent(adcui::UIMSG_TODOLIST_PUSHED_FRONT, pData);
	}
}*/

///////////////////////////////////////////////////////////////
// ADCOutput

class ADCOutput : public My::XCaptureStream
{
public:
	ADCOutput(ADCOutputSink &r, const char *header, int kB)
		: My::XCaptureStream(header, kB, 1024),//chunk size
		mr(r),
		mpf(nullptr)
	{
		//mpf = FileOpen("bzzz.log", "w");
	}

	/*ADCOutput(ADCOutputSink &r, const char *name, int kB, int winId)
		: My::XCaptureStream(winId, kB, name, 1024),//chunk size
		mr(r),
		mpf(nullptr)
	{
		//mpf = FileOpen("bzzz.log", "w");
	}*/

	~ADCOutput()
	{
		if (mpf)
			fclose(mpf);
	}

	bool flush(MyStreamBase &ss, unsigned bytes)
	{
		return Flush(ss, mpf, bytes, -1);
	}

	//int winid(){ return WinId(); }

protected:
	void OutputReady(bool bSend)
	{
		if (bSend)
		{
			//MyPostData *pData = new MyPostData;
			//MyStream &ss(pData->obj());
			MyStream ss;
			Flush(ss, mpf, (unsigned)-1, -1);
			//mrCore.GuiPostMessage(GUIMSG_OUTPUT_READY, pData);
			mr.GuiOutputReady(ss);
		}
		else
		{
			mr.GuiOutputReady();
			//mrCore.GuiPostMessage(GUIMSG_OUTPUT_READY, nullptr);
		}
	}

private:
	ADCOutputSink	&mr;
	FILE	*mpf;
};



//////////////////////////////////////
// ADCRedirect

ADCRedirect *ADCRedirect::spThis(nullptr);

ADCRedirect::ADCRedirect()
	: mpStdout(nullptr),
	mpStderr(nullptr),
	mOut(stdout),
	mErr(stderr)
{
	assert(!spThis);
	spThis = this;
}

ADCRedirect::~ADCRedirect()
{
	restoreRedirect();
	assert(spThis == this);
	spThis = nullptr;
}

void ADCRedirect::restoreRedirect()
{
	mOut.Restore(-1);
	mErr.Restore(-1);
	delete mpStdout;
	mpStdout = nullptr;
	delete mpStderr;
	mpStderr = nullptr;
}

int ADCRedirect::flushOutput(MyStreamBase &ss, unsigned bytes)
{
	if (mpStdout && mpStdout->flush(ss, bytes))
		return 1;
	if (mpStderr && mpStderr->flush(ss, bytes))
		return 1;
	return 0;
}

int ADCRedirect::flushOutput(MyStreamBase &ss)
{
	unsigned bytes;
	MyStreamUtil ssh(ss);
	if (!ssh.ReadInt((int *)&bytes))
		bytes = (unsigned)-1;
	return flushOutput(ss, bytes);
}

void ADCRedirect::redirect2files(int anRedirectOutput, const MyString &asName, bool bAppend)
{
	switch (anRedirectOutput)
	{
	case 0:
		break;
	default:
	case 1://stdout+stderr combined
		{
			MyString sOutputPath = MyPath(asName+".out").Path();
			//std::map<MyString, SSSPathInfo>::iterator it(alFiles.find(sOutputPath));
			//bool bAppend(it != alFiles.end());
			if (mOut.toFile(sOutputPath.c_str(), bAppend))
			{
			//	if (bAppend)
			//		alFiles.erase(it);
			}
			mErr.toBuddy(mOut);
		}
		break;
	case 2://stdout only
		{
			MyString sOutputPath = MyPath(asName+".out").Path();
			//std::map<MyString, SSSPathInfo>::iterator it(alFiles.find(sOutputPath));
			//bool bAppend(it != alFiles.end());
			if (mOut.toFile(sOutputPath.c_str(), bAppend))
			{
			//	if (bAppend)
			//		alFiles.erase(it);
			}
		}
		break;
	case 3://stderr only
		{
			MyString sErrorPath = MyPath(asName+".err").Path();
			//map<string, SSSPathInfo>::iterator it(alFiles.find(sErrorPath));
			//bool bAppend(it != alFiles.end());
			if (mErr.toFile(sErrorPath.c_str(), bAppend))
			{
			//	if (bAppend)
			//		alFiles.erase(it);
			}
		}
		break;
	case 4://stdout, stderr separated
		{
			MyString sOutputPath = MyPath(asName+".out").Path();
			//map<string, SSSPathInfo>::iterator it(alFiles.find(sOutputPath));
			//bool bAppend(it != alFiles.end());
			if (mOut.toFile(sOutputPath.c_str(), bAppend))
			{
			//	if (bAppend)
			//		alFiles.erase(it);
			}
		}
		{
			MyString sErrorPath = MyPath(asName+".err").Path();
			//map<string, SSSPathInfo>::iterator it(alFiles.find(sErrorPath));
			//bool bAppend(it != alFiles.end());
			if (mErr.toFile(sErrorPath.c_str(), bAppend))
			{
			//	if (bAppend)
			//		alFiles.erase(it);
			}
		}
		break;
	}
}

int ADCRedirect::redirect2pipes(ADCOutputSink &rSink, int f)
{
	assert(!mpStdout && !mpStderr);
	switch (f)
	{
	case 0:
		break;
	default:
	case 1://stdout+stderr combined
		mpStdout = new ADCOutput(rSink, "[STDOUT] 0", 64);
		mOut.toPipe(mpStdout->pipeReader());
		mErr.toBuddy(mOut);
		break;
	case 2://stdout only
		mpStdout = new ADCOutput(rSink, "[STDOUT] 0", 48);
		mOut.toPipe(mpStdout->pipeReader());
		break;
	case 3://stderr only
		mpStderr = new ADCOutput(rSink, "[STDERR] 0", 16);
		mErr.toPipe(mpStderr->pipeReader());
		break;
	case 4://stdout, stderr separated
		mpStdout = new ADCOutput(rSink, "[STDOUT] 0", 48);
		mOut.toPipe(mpStdout->pipeReader());
		mpStderr = new ADCOutput(rSink, "[STDERR] 0", 16);
		mErr.toPipe(mpStderr->pipeReader());
		break;
	}

	return 1;
}

int ADCRedirect::StopOutErrCapture()
{
	restoreRedirect();
	return 1;
}

int ADCRedirect::redirectOutputLock(bool bLock, int winId)
{
	static std::mutex x;
	int winId0(winId);
	if (bLock)
	{
		x.lock();
	}
	else
	{
		winId = 0;
	}

	if (mpStdout)
	{
		//assert(bLock || mpStdout->winid() == winId0);
		mOut.Restore(1);
		char buf[32];
		sprintf(buf, "[STDOUT] %d", winId);
		mpStdout->Redirect(buf);
		mOut.toPipe(mpStdout->pipeReader());
	}

	if (mpStderr)
	{
		//assert(bLock || mpStderr->winid() == winId0);
		mErr.Restore(1);
		char buf[32];
		sprintf(buf, "[STDERR] %d", winId);
		mpStderr->Redirect(buf);
		mErr.toPipe(mpStdout->pipeReader());
	}

	if (!bLock)
	{
		x.unlock();
	}

	(void)winId0;
	return winId;
}



/////////////Gui_t


Gui_t::Gui_t(My::IGui *pIGui, Main_t &rMain)// bool bEnableOutputCapture)
: //Gui_t(pIGui),
//mrMain(rMain),
mpIGui(pIGui),
mpCore(nullptr)
{
}

Gui_t::~Gui_t()
{
	delete mpCore;
}


ReadLocker::ReadLocker(Main_t& rMain) : mrMain(rMain){ mrMain.lockProjectRead(true); }
ReadLocker::~ReadLocker(){ mrMain.lockProjectRead(false); }

/*Project_t &Gui_t::project() const
{
	return mrMain.project();
}

bool Gui_t::hasProject() const
{
	return mrMain.hasProject();
}*/

/*TypePtr Gui_t::binaryRef() const
{
	return mrMain.project().binaryRef();
}*/




/////////////////////////////////////////////////////

Core_t::Core_t(Main_t& rMain, ADCOutputSink &rGui, bool bEnableOutputCapture)
	: mrGui(rGui),
	mrMain(rMain),
	mpRedirect(nullptr),
	mbAllowCaptureOutput(bEnableOutputCapture),
	//mpProbe(nullptr),
	mpEd(nullptr)
{
}

Core_t::~Core_t()
{
	//assert(!mpProbe);//must have been handled by a model
	delete mpRedirect;
}

Project_t &Core_t::project() const
{
	return mrMain.project();
}

bool Core_t::hasProject() const
{
	return mrMain.hasProject();
}

adcui::IADCTextEdit* Core_t::startEd(const std::string& s)
{
	Probe_t* pLoc(getContext());
	if (!pLoc)
		return nullptr;
	adcui::IADCTextEdit* pIEd(new MyLineEdit(*this, s, *pLoc));
	pLoc->Release();
	return pIEd;
}

const char *Core_t::GetCompanyName() const
{
	return mrMain.companyName().c_str();
}

const char *Core_t::GetProductName() const
{
	return mrMain.appName().c_str();
}

const char* Core_t::GetProductCodeName() const
{
	return mrMain.appCodeName().c_str();
}

const char* Core_t::GetProductVersion() const
{
	return mrMain.appVersion().c_str();
}

Probe_t* Core_t::setLocus(Probe_t *loc)
{
	Probe_t* pOld(mctx.ptr());

	//bool bFolderChanged(pOld ? pOld->folder() != loc->folder() : true);
	//bool bTypesMapChanged(pOld ? pOld->typesMap() != loc->typesMap() : true);
	bool bLocalityChanged(pOld ? pOld->locality().u != loc->locality().u : true);

	mctx.set(loc);

	if (bLocalityChanged)
	{
#if(0)
		fprintf(stdout, "locality = %X\n", pOld->locality().u);
		fflush(stdout);
#endif
		mrMain.project().markDirty(DIRTY_LOCALITY);
	}
	
	mrMain.project().markDirty(DIRTY_LOCUS);
	return mctx.ptr();
}

void Gui_t::GuiOnProjectModified() const
{
	mpCore->RefreshBinaryDump(-1, false);
	GuiNotify(adcui::UIMSG_GLOBALS_MODIFIED);
}

void Gui_t::GuiShowFile(const char *fileName, const char *atLocation, bool bAsk) const//if bAsk=true, ask if the file is not open
{
	ADCPostData *pData(new ADCPostData());
	MyStream &ss(pData->obj());
	ss.WriteString(fileName);
	ss.WriteString(atLocation);
	ss.WriteBool(bAsk);
	igui()->PostEvent(adcui::UIMSG_SHOW_VIEW, pData);
}

/*void Gui_t::GuiAnalisysAction(adcui::MsgIdEnum e)
{
	ADCPostData *pData(new ADCPostData());
	MyStream &ss(pData->obj());
	//ss.WriteStream(iss);
	igui()->PostEvent(e, pData);
}*/

/*void Gui_t::GuiOnSyncPanes(int line) const
{
	ADCPostData *pData(new ADCPostData());
	MyStream &ss(pData->obj());
	ss.WriteInt(line);
	igui()->PostEvent(adcui::UIMSG_PING_BACK, pData);
}*/

void Gui_t::GuiOnNoOutputDirectory(const char *cmd) const
{
	ADCPostData *pData(new ADCPostData());
	MyStream &ss(pData->obj());
	ss.WriteString(cmd);
	igui()->PostEvent(adcui::UIMSG_COMMAND_RECOIL, pData);
}

void Gui_t::GuiOnFilePriorRemoved(const char *fileName) const
{
	//ADCPostData *pData(new ADCPostData());
	MyStream ss;// (pData->obj());
	//ss.WriteInt(fileId);
	ss.WriteString(fileName);
	igui()->SendEvent(adcui::UIMSG_FILE_PRIOR_REMOVED, &ss);
}

void Gui_t::GuiOnProjectAboutToClose() const
{
	igui()->SendEvent(adcui::UIMSG_PROJECT_ABOUT_TO_CLOSE, 0);
	//?mpBinViewModel->clear();
	//mpStubsViewModel->clear();
}

void Gui_t::GuiOutputReady(MyStreamBase &iss)
{
	ADCPostData *pData(new ADCPostData());
	MyStream &ss(pData->obj());
	ss.WriteStream(iss);
	igui()->PostEvent(adcui::MSGID_OUTPUT_READY, pData);
}

void Gui_t::GuiOutputReady()
{
	My::IGui *pIGui(igui());
	if (pIGui)
	{
		pIGui->PostEvent(adcui::MSGID_OUTPUT_READY, nullptr);
	}
}

long Core_t::EnableOutputCapture()
{
	if (!mbAllowCaptureOutput)
		return 0;
	mpRedirect = new ADCRedirect();
	return mpRedirect->redirect2pipes(mrGui, 1);
}

long Core_t::FlushOutput(MyStreamBase &ss)
{
	if (!mpRedirect)
		return 0;
	return mpRedirect->flushOutput(ss);
}

long Core_t::Shutdown()
{
	mrMain.postEvent(new SxCustomEvent(SXEVENT_QUIT, nullptr));
	return 1;
}

void Core_t::ClearLocus()
{
	Probe_t *pCtx(getContext());
	if (pCtx)
	{
		pCtx->clear_all();
		releaseContext(pCtx);
	}
}

long Core_t::PostCommand(const char *cmd, bool bEcho)
{
	CEventCommand* p(new CEventCommand(cmd, bEcho));
	p->setContextZ(getContext());
	mrMain.postEvent(p);
	return 1;
}

long Core_t::CallCommand(const char *cmd, MyStreamBase &iss, bool bEcho)
{
	CEventCommand *e(new CEventCommand(cmd, bEcho));
	e->setResult(&iss);
	return mrMain.callEvent(e);
}

long Core_t::SetDebugMode(bool b)
{
	mrMain.setDebugMode(b);
	return 1;
}

long Core_t::Start()
{
	mrMain.resumeAnalysis(StopFlag::RESET);
	return 1;
}

long Core_t::Pause()
{
	mrMain.resumeAnalysis(StopFlag::PAUSE);
	return 1;
}

long Core_t::Stop()
{
	mrMain.resumeAnalysis(StopFlag::ABORT);
	return 1;
}

adcui::IBinViewModel *Core_t::NewBinViewModel(const char* moduleName)
{
	if (!mrMain.hasProject())
		return 0;
	Folder_t *pFolder(nullptr);
	assert(0);/*?
	bool bHeader(false);
	if (fileName)
		pFolder = findFolder(fileName, bHeader);
	if (!pFolder)
		return nullptr;*/
/*?	BinViewModel_t *pModel(new BinViewModel_t(*this, mrMain, *pFolder));
	//pModel->reset(mrMain.project());
	return pModel;*/return nullptr;
	//return static_cast<adcui::IBinViewModel *>(mpBinViewModel);
}

TypePtr Core_t::OnObjectPicked(ObjPtr pObj, TypePtr module)
{
	if (pObj)
	{
		TypePtr pType(pObj->objType());
		if (!pType)
		{
			CFieldPtr pField(pObj->objField());
			if (pField)
				pType = pField->type();
		}
		if (pType)
		{
			if (pType->typeProxy())
				pType = pType->typeProxy()->incumbent();
			if (pType->typeComplex())
				return pType;
		}
	}
	return nullptr;
}

adcui::IBinViewModel *Core_t::NewScopeViewModel(const char *pTypeName)
{
	if (!mrMain.hasProject())
		return nullptr;

	ObjPtr pObj(nullptr);

	ZPath_t zPath(pTypeName);
	if (zPath.empty())//from locus?
	{
		Probe_t *pCtx(getContext());
		if (pCtx)
		{
			pObj = OnObjectPicked(pCtx->obj(), pCtx->locus().module2());
			releaseContext(pCtx);
		}
	}
	else
	{
		if (zPath.front() == ATTIC_NAME)
		{
			zPath.pop_front();
			if (zPath.empty())
				return nullptr;
			MyString sType(zPath.toString());
			ProjectInfo_t PI(project());
			pObj = PI.FindAtticTypeByName(sType);
		}
		else if (!zPath.isAbsolute())
		{
			Probe_t *pCtx(getContext());
			if (pCtx)
			{
				const Probe_t &l(*pCtx);
				TypePtr iModule(l.moduleFromLocus());
				if (iModule)
				{
					ModuleInfo_t MI(project(), *iModule);
					MyString sType(zPath.toString());
					pObj = project().findObject(sType, iModule);
					/*TypesMgr_t *ptm(l.typesMap());
					for (; ptm; ptm = MI.superTypesMgr(ptm))
					{
						TypesTracer_t TT(MI, *ptm);
						if ((pObj = TT.findTypeByName(pTypeName)) != nullptr)
							break;
					}*/
				}
				releaseContext(pCtx);
			}
		}
		else
		{
			ProjectInfo_t PI(project());
			TypePtr iModule(PI.FindModuleEx(zPath.front(), true));
			if (!iModule)
				return nullptr;

			zPath.pop_front();
#if(0)
			if (zPath.empty() || !zPath.front().startsWith(TYPES_EXT))
				return nullptr;
			zPath.pop_front();
#endif
			if (zPath.empty())
				return nullptr;

			MyString sType(zPath.toString());//"::"));//scope separator
			pObj = project().findObject(sType, iModule);
		}
	}

	return NewBinViewModel(pObj, true);
}

BinViewModel_t *Core_t::NewBinViewModel(CObjPtr pObj, bool bTypeEdit)
{
	if (pObj)
	{
		CTypePtr pScope(pObj->objType());
		if (pScope)
		{
			if (!(pScope->typeStrucvar() || pScope->typeCode()))
			{
				resetBinaryDump(nullptr);//binary dump at index 1 is reserved for type editor
				return new BinViewModel_t(*this, mrMain, ProjectInfo_t::ModuleOf(pScope), pScope, bTypeEdit);
			}
		}
	}
	return nullptr;
}

bool Core_t::GetToDoList(MyStreamBase &ss)
{
	return mrMain.writeToDoList(ss);
}

long Core_t::GetProjectInfo(MyStreamBase &ss)
{
	if (!mrMain.hasProject())
		return 0;
	MyStreamUtil ssh(ss);
	ProjectInfo_t PI(project());
	ssh.WriteString(PI.TypeName(project().self()));
	ssh.WriteString(project().path());
	return 1;
}

/*adcui::IStubsViewModel *Gui_t::GetStubsViewModel()
{
	return mpStubsViewModel;
}*/

long Core_t::GetLocusInfo(unsigned, MyStreamBase &ss)
{
	ReadLocker lock(mrMain);
	if (!mrMain.hasProject())
		return 0;
	Probe_t *pCtx(getContext());
	if (pCtx)
	{
		MyStreamUtil ssu(ss);
		ssu.WriteString(pCtx->locus().toString());
		releaseContext(pCtx);
	}

	return 1;
}

void Core_t::DumpResources(MyStreamBase &ss)
{
	ReadLocker lock(mrMain);
	if (hasProject())
	{
		ProjectInfo_t projInfo(project());
		try
		{
			const FoldersMap &m(project().rootFolder().children());
			for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
			{
				CFolderRef rFolder(*i);
				if (!projInfo.IsPhantomFolder(rFolder))
				{
					TypePtr iModule(projInfo.ModuleOf(&rFolder));
					ModuleInfo_t MI(project(), *iModule);
					//TypePtr iFrontSeg(MI.FindFrontSeg());
					MI.DumpSegResources(ss);
				}
			}
		}
		catch (...)
		{
		}
	}
}

#if(0)
void Core_t::dumpTypes(MyStreamBase &ss, MyString sType)
{
	if (!hasProject())
		return;

	ProjectInfo_t PI(project());

	Probe_t *pCtx(getContext());
	if (pCtx)
	{
		MyStreamUtil ssh(ss);

		//std::set<std::string> aCheck;

		TypesMgr_t *pTypeMgr(pCtx->typesMap());
		while (pTypeMgr)
		{
			TypePtr pOwner(pTypeMgr->owner());

			MyString s("[" + (pOwner->typeModule() ? MyString("BINARY") : PI.TypeName(pOwner)) + "]");
			ssh.WriteString(s);

			for (TypesMapCIt it2(pTypeMgr->aliases().begin()); it2 != pTypeMgr->aliases().end(); it2++)
			{
				TypePtr pType(it2->pSelf);
				if (!pType->typeArray() && !pType->typePtr())
				{
					s = PI.TypeName(pType);
					//if (aCheck.insert(s).second)
					{
						if (s == sType)
							s.prepend("->");//current

						unsigned flags(0);
						if (pType->typeStrucvar())
							flags = 2;
						if (pType->typeStruc())
						{
							flags = 1;
							//if (pType->typeStruc()->use)
						}
						ssh.WriteString(s);
					}
				}
			}

			if (pOwner->typeSeg())
				pTypeMgr = PI.findTypeMgr(pOwner->typeSeg()->superLink());
			else if (pOwner->parentField())
				pTypeMgr = PI.findTypeMgr(pOwner->parentField()->owner());
			else
				pTypeMgr = nullptr;
		}

		ssh.WriteString("[EXTERNAL]");
		mrMain.writeExternalTypes(ss);

		releaseContext(pCtx);
	}

/*
	TypesMgr_t *pTypeMgr(project().context().typesMap());
	if (pTypeMgr)
	{
		MyString s;
		MyStreamUtil ssh(ss);
		//for (TypesMapIt it2(pTypeMgr->aliases().begin()); it2 != pTypeMgr->aliases().end(); it2++)
		for (TypesMgr_t::setTYPE_cit it2(pTypeMgr->types().begin()); it2 != pTypeMgr->types().end(); it2++)
		{
			TypePtr pType(*it2);
			if (!pType->typeArray() && !pType->typePtr())
			{
				s = PI.TypeName(pType);
				ssh.WriteString(s);
			}
		}
	}*/
}
#endif

/*void Core_t::DumpTypes(MyStreamBase &ss)
{
	ReadLocker lock(mrMain);
	dumpTypes(ss, MyString());
}*/

unsigned Core_t::ContextId() const
{
	//ReadLocker lock(mrMain); 
	unsigned eRet(0);
	if (hasProject())
	{
		Probe_t *pCtx(getContext());
		if (pCtx)
		{
			eRet = pCtx->locality().u;
			releaseContext(pCtx);
		}
	}
	return eRet;
}

adcui::ITypesViewModel *Core_t::newTypesViewModel(TypePtr iModule)
{
	if (!iModule && hasProject())
	{
		Probe_t *pCtx(getContext());
		if (pCtx)
		{
			iModule = pCtx->moduleFromLocus();
			releaseContext(pCtx);
		}
	}

	if (!iModule)
		return nullptr;

	//return new TypesViewModel2_t(*this);
	return new TypesViewModel3_t(*this, iModule);
}


adcui::ITypesViewModel *Core_t::NewTypesViewModel(const char*)
{
	//return new TypesViewModel2_t(*this);
	return newTypesViewModel(nullptr);
}

adcui::INamesViewModel *Core_t::newNamesViewModel(FolderPtr pNamesFolder)
{
	//return new NamesViewModel_t(*this, pNamesFolder);
	return new NamesViewModelBiased_t(*this, pNamesFolder);
}

adcui::INamesViewModel *Core_t::NewNamesViewModel(const char*s)
{
	return newNamesViewModel(nullptr);
}

adcui::IFilesViewModel *Core_t::NewFilesViewModel()
{
	return new FilesViewModel_t(*this);
}

adcui::IResViewModel *Core_t::NewResViewModel(const char*)
{
	return new ResViewModel_t(*this, nullptr);
}


class ADCTasksViewModel : public adcui::ITasksViewModel
{
protected:
	Core_t &mrCore;
public:
	ADCTasksViewModel(Core_t &rCore)
		: mrCore(rCore)
	{
	}
	virtual void reset()
	{
		
	}
	virtual size_t count() const 
	{
		if (!mrCore.main().hasProject())
			return 0;
		IAnalyzer *pAnalizer(mrCore.main().project().analyzer());
		if (!pAnalizer)
			return 0;
		return mrCore.main().project().analyzer()->size();
	}
	virtual void data(size_t index, MyStreamBase &ss) const
	{
		IAnalyzer *pAnalizer(mrCore.main().project().analyzer());
		if (pAnalizer)
			pAnalizer->writeTask(index, ss);
	}
	virtual void module(size_t index, MyStreamBase &ss) const 
	{
		IAnalyzer *pAnalizer(mrCore.main().project().analyzer());
		if (pAnalizer)
			pAnalizer->writeModule(index, ss);
	}
	virtual void viewPos(size_t index, MyStreamBase &ss) const
	{
		IAnalyzer *pAnalizer(mrCore.main().project().analyzer());
		if (pAnalizer)
			pAnalizer->writeDA(index, ss);
	}
	virtual void lockRead(bool bLock)
	{
		mrCore.main().lockProjectRead(bLock);
	}
};


adcui::ITasksViewModel *Core_t::NewTasksViewModel()
{
	return new ADCTasksViewModel(*this);
}

struct __getResTypeUserData
{
	const ADDR va;
	adcui::IADBCore::ResType_e resType;
	unsigned dataSize;
	__getResTypeUserData(ADDR _va) : va(_va), resType(adcui::IADBCore::UNKNOWN), dataSize(0){}
};

static bool __getResTypeCallback(size_t level, const char *name, ADDR va, unsigned dataSize, void *pvUser)
{
	__getResTypeUserData *p((__getResTypeUserData *)pvUser);
	if (level == 1)
	{
		if (strcmp(name, "Cursors") == 0)
			p->resType = adcui::IADBCore::CUR;
		else if (strcmp(name, "Bitmaps") == 0)
			p->resType = adcui::IADBCore::BMP;
		else if (strcmp(name, "Icons") == 0)
			p->resType = adcui::IADBCore::ICO;
		else
			p->resType = adcui::IADBCore::UNKNOWN;
	}
	else if (level == 2)
	{
		if (va == p->va)
		{
			p->dataSize = dataSize;
			return false;//stop
		}
	}
	return true;//continue enumaration
}

class ResDataSource_t : public adcui::IDataSource
{
	PDATA mpData;
	size_t mSize;
public:
	ResDataSource_t(const I_DataSource &aRaw, size_t aSize) {
		mSize = aSize;
		mpData = new char[mSize];
	}
	virtual ~ResDataSource_t() {
		delete mpData;
	}
	virtual void *data() const {
		return (void *)mpData;
	}
	virtual size_t size() const {
		return mSize;
	}
};

adcui::IDataSource *Core_t::GetResourceData(ResType_e &eData)
{
	Probe_t *pCtx(getContext());
	if (!pCtx)
		return nullptr;

	const Probe_t &l(*pCtx);
	if (l.locus().empty())
		return nullptr;

	TypePtr iModule(l.moduleFromLocus());
	ModuleInfo_t MI(project(), *iModule);
	//Module_t &aBin(*iModule->typeModule());

	eData = adcui::IADBCore::UNKNOWN;

	const Block_t &f(l.locus().back());
	size_t aSize(size_t(f.m_size));

	//find out a resource type
	TypePtr iSeg(MI.FindFrontSeg());
	if (iSeg)
	{
		Seg_t &rSeg(*iSeg->typeSeg());
		__getResTypeUserData cb(l.locus().va());
		MI.FrontEnd(iSeg)->device(MI.GetDataSource())->dumpResources2(__getResTypeCallback, &cb, nullptr);
		if (cb.dataSize)
		{
			eData = cb.resType;
			aSize = cb.dataSize;
		}
	}

	releaseContext(pCtx);

	return new ResDataSource_t(MI.GetDataSource()->pvt(), aSize);
}

//path is in format <Module>:\\@<VA>
bool Core_t::SetCP(const char *command)
{
//	bool bLock(mrMain.writeLock() == 0);
	//if (bLock)
		//mrMain.lockProjectWrite(true);
	//MyStream ss;
	//bool bRet(CallCommand(command, ss, false) != 0);


	MyArgs2 a(command);
	if (a.size() > 1)
	{
		TypePtr iModule(nullptr);

		ZPath_t l(ProjectInfo_s::fixFileName(a[1], iModule));
		if (l.empty() || l.front() == l.back())
			return 0;

		ProjectInfo_t PI(mrMain.project());

		if (!iModule)
		{
			Folder_t* pFolder(PI.Files().FindFileByStem(l.root()));
			if (!pFolder)
				return 0;
			iModule = ProjectInfo_s::ModuleOf(pFolder);
		}
		MyString sAt(l.back());
		if (sAt.startsWith("@"))
			sAt = sAt.mid(1);

		ModuleInfo_t MI(PI, *iModule);

		DA_t da;
		Locus_t aLoc;
		if (!sAt.empty() && !(MI.Str2DA(sAt, &da.row) && PI.LocusFromDA(da.row, aLoc)))
			return 0;

		//ADDR va(ctx.addr());
		//ADDR va(strtoul(sVA.c_str(), nullptr, 16));
		//TypePtr iSeg(FindSegAt(iModule, va));
		//if (iSeg)
		{
			//DA_t da(iSeg->typeSeg()->viewOffs(iSeg, va), 0, 0);
			//Locus_t aLoc;
			//terminalFieldAtSeg(iModule, da, aLoc, iModule->typeModule()->rawBlock());
			setLocus(mrMain.project().NewLocus(aLoc));
			mrMain.project().markDirty(DIRTY_LOCUS_ADJUSTED);
		}
		return true;
	}

	//if (bLock)
		//mrMain.lockProjectWrite(false);
	return false;

	/*
	ProjectInfo_t PJ(project());

	TypePtr iModule(nullptr);

	ZPath_t l(PJ.fixFile Name(path, iModule));
	if (!l.empty() && l.front() != l.back())
	{
		if (!iModule)
		{
			Folder_t *pFolder(project().files().FindFileByStem(l.root()));
			if (pFolder)
				iModule = PJ.ModuleOf(pFolder);
		}

		if (iModule)
		{
			MyString sAt(l.back());
			if (sAt.startsWith("@"))
				sAt = sAt.mid(1);

			if (!sAt.empty())
			{
				Probe_t *pCtx(project().getContext());
				if (pCtx)
				{
					DA_t da;
				if (PJ.Str2DA(sAt, &da.row) && PJ.LocusFromDA(da.row, ctx))
				{
					
				if (pCtx)
				{
					proj().setLocus(cx);
					proj().releaseContext(pCtx);
				}
				}
			}
		}
	}

	mrMain.lockProjectRead(false);
	return false;*/
}

bool Core_t::ApplySettings(MyStreamBase& ss)
{
	struct DataParser : public Options_t
	{
		MY_SCRIPT(DataParser);
		//MY_STRING(String);
		MY_INT(CallDepth);
		//MY_BOOL(Bool);

		DataParser()
		{
			//MY_IMPL(String);
			MY_IMPL(CallDepth);
			//MY_IMPL(Bool);
		}
	};

	MyStreamUtil ssu(ss);

	DataParser aData;
	aData.parse(ssu);

//	mrMain.applyOptions(aData);
	return false;
}

////////////////////////////////////////////////////////////

DumpInvariant_t *Core_t::dumpInvariant(CTypePtr iModule) const 
{
	BinDumpInvMap::const_iterator i(mBinDumps.find((TypePtr)iModule));
	if (i != mBinDumps.end())
		return i->second;
	return nullptr;
}

DumpTarget_t *Core_t::aquireDumpTarget(CTypePtr iModule)
{
	DumpInvariant_t *pInv(dumpInvariant(iModule));
	if (!pInv)
	{
		bool bStrucDump(iModule == nullptr);
		pInv = new DumpInvariant_t(true, bStrucDump ? true : COMPACT_VIEW);	//persistent info stay preserved
		pInv->initColumns(iModule);
		if (!mBinDumps.insert(std::make_pair((TypePtr)iModule, pInv)).second)
			ASSERT0;
	}

	if (!pInv->mpTarget)
	{
		pInv->mpTarget = new DumpTarget_t(*pInv);
		//a.mpTarget->setRowIdLen(project().binary().AddressWidth(binaryRef()));
		//a.mpTarget->setRowIdLen(8);
	}
	else
		pInv->mpTarget->AddRef();
	return pInv->mpTarget;
}

bool Core_t::releaseDumpTarget(CTypePtr iModule)
{
	DumpInvariant_t *pInv(dumpInvariant(iModule));
	if (!pInv)
		return false;
	if (!pInv->mpTarget)
		return false;
	if (pInv->mpTarget->Release() == 0)
		pInv->mpTarget = nullptr;
	return true;//there are still refs
}

bool Core_t::resetBinaryDump(CTypePtr iModule)
{
	DumpInvariant_t *pInv(dumpInvariant(iModule));
	if (!pInv)
		return false;

	int iRefs(0);
	while (releaseDumpTarget(iModule))
		iRefs++;

	bool bRet(iRefs > 0);
	if (iRefs > 0)
	{
		while (iRefs-- > 0)
			aquireDumpTarget(iModule);//have to restore reference num
	}
	return bRet;
}

void Core_t::refreshBinaryDump(size_t dumpId, bool bVerbose)
{
	bool bOk(false);
	size_t n(0);
	for (BinDumpInvMap::const_iterator i(mBinDumps.begin()); i != mBinDumps.end(); ++i, n++)
	{
		if (dumpId < mBinDumps.size())
			if (dumpId != n)
				continue;
		if (resetBinaryDump(i->first))
			bOk = true;
	}

	if (bVerbose && bOk)
	{
		mrMain.printInfo() << "Binary dump(" << (int)dumpId << ")" << " : refreshed." << std::endl;
		mrMain.printInfo().flush();
	}

}

void Core_t::RefreshBinaryDump(size_t dumpId, bool bVerbose)
{
	bool bLock(mrMain.writeLock() == 0);
	if (bLock)
		mrMain.lockProjectWrite(true);

	refreshBinaryDump(dumpId, bVerbose);

	if (bLock)
		mrMain.lockProjectWrite(false);
}

Gui_t *CreateADBGuiBroker(My::IGui *pIGui, Main_t &rMain,/*const char *appName, const char *appVersion,*/ bool bAllowOutputCapture)
{
	Gui_t *pGui(new Gui_t(pIGui, rMain));
	pGui->setCore(new Core_t(rMain, *pGui, bAllowOutputCapture));
	return pGui;
}

adcui::IExportsViewModel* Core_t::NewExportsViewModel(const char* pcModuleName)
{
	MyString moduleName;
	if (pcModuleName)
		moduleName = pcModuleName;
	if (moduleName.empty())
		return nullptr;// Core_t::NewExportsViewModel();

	ReadLocker lock(mrMain);

	assert(hasProject());

	ProjectInfo_t PI(project());
	CTypePtr iModule(PI.FindModuleEx(moduleName, false));
	if (iModule)
	{
		ModuleInfo_t MI(project(), *iModule);
		FolderPtr pFolder(MI.FolderOfKind(FTYP_EXPORTS));
		if (pFolder)
			return newExportsViewModel(pFolder);
	}
	return nullptr;
}

adcui::IImportsViewModel* Core_t::NewImportsViewModel(const char* pcModuleName)
{
	MyString moduleName;
	if (pcModuleName)
		moduleName = pcModuleName;
	if (moduleName.empty())
		return nullptr;
	ReadLocker lock(mrMain);
	assert(hasProject());
	ProjectInfo_t PI(project());
	CTypePtr pModule(PI.FindModuleEx(moduleName, false));
	if (!pModule)
		return nullptr;
	ModuleInfo_t MI(project(), *pModule);
	FolderPtr pFolder(MI.FolderOfKind(FTYP_IMPORTS));
	if (!pFolder)
		return nullptr;
	return new ImportsViewModel_t(*this, pFolder);
}

adcui::IExportsViewModel* Core_t::newExportsViewModel(FolderPtr pFolder)
{
	return new ExportsViewModel_t(*this, pFolder);
}


