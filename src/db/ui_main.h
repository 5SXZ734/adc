#pragma once

#include "interface/IADCGui.h"
#include "qx/MyString.h"
#include "qx/MyRedirect2.h"
#include "mem.h"
#include "files.h"
#include "main.h"
#include "probe.h"

class MyStreamBase;
class ADCOutput;
class SSSRemoteCore;
class BinViewModel_t;
class Main_t;
class Project_t;
class DumpTarget_t;
class Gui_t;
class DumpInvariant_t;

class MyLineEdit : public MyLineEditBase
{
	const Probe_t& mLocus;
	bool bArray;
public:
	MyLineEdit(Core_t& rCore, MyString s, const Probe_t&);
	~MyLineEdit();
protected:
	virtual int apply();
	virtual int startPos() const;
};



///////////////////////////////////////////////////ADCOutputSink
class ADCOutputSink
{
public:
	virtual void GuiOutputReady(MyStreamBase &) = 0;
	virtual void GuiOutputReady() = 0;
};



//////////////////////////////////////////////////ADCRedirect
class ADCRedirect
{
	friend class ADCOutput;
	static ADCRedirect *spThis;
protected:
	ADCOutput	*mpStdout;
	ADCOutput	*mpStderr;
	My::XRedirect mOut;
	My::XRedirect mErr;
public:
	ADCRedirect();
	virtual ~ADCRedirect();
	static ADCRedirect *instance(){ return spThis; }
	int flushOutput(MyStreamBase &, unsigned);
	int flushOutput(MyStreamBase &);
	virtual int StopOutErrCapture();
	virtual int redirectOutputLock(bool, int);
	void redirect2files(int, const MyString &, bool);
	int redirect2pipes(ADCOutputSink &, int);
	void restoreRedirect();
	My::XRedirect &out(){ return mOut; }
	My::XRedirect &err(){ return mErr; }
};



//////////////////////////////////////////////////////////ADCPostData
class ADCPostData : public My::IGuiMsg<MyStream>
{
public:
	ADCPostData(){}
	virtual void *data(){ return static_cast<MyStreamBase *>(&m_t); }
};


//////////////////////////////////////////////////////////ReadLocker
class ReadLocker
{
	Main_t& mrMain;
public:
	ReadLocker(Main_t& rMain);
	~ReadLocker();
};


class IGui_t
{
public:
	virtual My::IGui *igui() const { return nullptr; }
	virtual void GuiProcessEvents() const{}
	virtual bool GuiNotify(int) const { return false; }
	virtual bool GuiNotify(int, MyStreamBase &) const { return false; }

	virtual void GuiOnProjectNew() const {}
	virtual void GuiOnProjectSaved() const {}
	virtual void GuiOnProjectOpened() const {}
	virtual void GuiOnProjectClosed() const {}
	virtual void GuiOnToDoListChanged() const {}
	virtual void GuiOnProjectModified() const {}
	virtual void GuiOnAnalizerStarted() const {}
	virtual void GuiOnAnalizerPaused() const {}
	virtual void GuiOnAnalizerResumed() const {}
	virtual void GuiOnAnalizerStopped() const {}
	virtual void GuiOnPreanalized() const {}
	virtual void GuiOnShowAnalizerInfo(const MyString &) const {}
	virtual void GuiOnShowProgressInfo(const MyString &) const {}
	virtual void GuiOnFileListChanged() const {}
	virtual void GuiOnFileModified(const MyString &relPath) const {}
	virtual void GuiOnFileRenamed(const MyString &relPathOld, const MyString &relPathNew) const {}
	virtual void GuiOnFilePriorRemoved(const char *fileName) const {}
	virtual bool GuiOnSaveRequest() const { return false; }
	virtual void GuiOnProjectAboutToClose() const {}
	virtual void GuiShowFile(const char *fileName, const char *extra, bool bAsk) const {}
	virtual void GuiNewFolderRecoil(const MyString &relPath) const {}

	virtual void GuiOnNameChanged() const {}
	virtual void GuiOnCurOpChanged() const {}
	virtual void GuiOnCurFuncModified() const {}
	virtual void GuiOnCurStrucModified() const {}
	virtual void GuiOnCurFolderChanged() const {}
	virtual void GuiOnCurFuncChanged() const {}
	virtual void GuiOnSelObjChanged() const {}
	virtual void GuiOnLocusChanged() const {}
	virtual void GuiOnLocusAdjusted() const {}
	virtual void GuiOnTypesMapChanged() const {}
	virtual void GuiOnLocalityChanged() const {}

	virtual void GuiOnNoOutputDirectory(const char *cmd) const {}
	virtual void GuiDebuggerStarted() const {}
	virtual void GuiDebuggerStopped() const {}
	virtual void GuiDebuggerBreak() const {}
	virtual void GuiDebuggerResumed() const {}

	virtual const char* GuiColorTag(adcui::LogColorEnum) const { return ""; }//for output log
};


class Core_t : public virtual adcui::IADBCore
{
protected:
	ADCOutputSink &mrGui;
	Main_t	&mrMain;
	ADCRedirect	*mpRedirect;
	bool	mbAllowCaptureOutput;
	typedef std::map<TypePtr, DumpInvariant_t *>	BinDumpInvMap;
	BinDumpInvMap	mBinDumps;
	MyLineEditBase* mpEd;//weak
	RefPtr_t<Probe_t>	mctx;
	friend class Gui_t;
public:
	Core_t(Main_t&, ADCOutputSink &, bool bEnableOutputCapture);
	virtual ~Core_t();
	Main_t &main() const { return mrMain; }
	Project_t &project() const;
	bool hasProject() const;
	adcui::IADCTextEdit* startEd(const std::string&);
//protected:
	DumpInvariant_t *dumpInvariant(CTypePtr iModule) const;
	DumpTarget_t *aquireDumpTarget(CTypePtr);
	bool releaseDumpTarget(CTypePtr);
	bool resetBinaryDump(CTypePtr);
	void refreshBinaryDump() { return RefreshBinaryDump(-1, false); }
	void refreshBinaryDump(size_t, bool);
	//void dumpTypes(MyStreamBase &ss, MyString);
	MyLineEditBase* ed() const { return mpEd; }
	void setEd(MyLineEditBase* p) {
		mpEd = p; }
	bool isEditing() const { return mpEd != nullptr; }
	MyString editName() const { return (mpEd ? mpEd->editName() : MyString()); }

	virtual Probe_t* setLocus(Probe_t *);
	Probe_t *setContext(Probe_t *pctx){ 
		return mctx.set(pctx); }
	Probe_t *getContext() const {
		return mctx.get();
	}
	void releaseContext(I_Context *pCtx) const {
		assert(pCtx);
		pCtx->Release();
	}
	virtual int checkSelection(CTypePtr, const DA_t &) { return 0; }//binary

public:
	//adcui::IADBCore
	virtual const char *GetCompanyName() const;
	virtual const char *GetProductName() const;
	virtual const char *GetProductCodeName() const;
	virtual const char *GetProductVersion() const;

	virtual long EnableOutputCapture();
	virtual long FlushOutput(MyStreamBase &ss);
	virtual long Shutdown();
	virtual void ClearLocus();
	virtual long PostCommand(const char *, bool);
	virtual long CallCommand(const char *, MyStreamBase &, bool);
	virtual long SetDebugMode(bool);
	virtual long Start();
	virtual long Pause();
	virtual long Stop();
	virtual bool GetToDoList(MyStreamBase &);
	virtual long GetProjectInfo(MyStreamBase &);
	//virtual long GetProjectFiles(MyStreamBase &);
	virtual long GetLocusInfo(unsigned, MyStreamBase &);
	virtual void DumpResources(MyStreamBase &);
	virtual void RefreshBinaryDump(size_t, bool);
	//virtual void DumpTypes(MyStreamBase &);
	virtual unsigned ContextId() const;
	virtual adcui::IBinViewModel *NewBinViewModel(const char*);
	virtual adcui::ITypesViewModel *NewTypesViewModel(const char*);
	virtual adcui::INamesViewModel *NewNamesViewModel(const char*);
	virtual adcui::IExportsViewModel *NewExportsViewModel(const char *);
	virtual adcui::IImportsViewModel *NewImportsViewModel(const char *);
	virtual adcui::IFilesViewModel *NewFilesViewModel();
	virtual adcui::IResViewModel *NewResViewModel(const char*);
	virtual adcui::IBinViewModel *NewScopeViewModel(const char *);
	virtual adcui::ITasksViewModel *NewTasksViewModel();
	virtual adcui::IDataSource *GetResourceData(ResType_e &);
	virtual bool SetCP(const char *);
	virtual bool ApplySettings(MyStreamBase&);
protected:
	virtual TypePtr OnObjectPicked(ObjPtr, TypePtr module);
	virtual adcui::IExportsViewModel *newExportsViewModel(FolderPtr);
	adcui::ITypesViewModel *newTypesViewModel(TypePtr);//binary
	adcui::INamesViewModel *newNamesViewModel(FolderPtr);//binary
	virtual BinViewModel_t *NewBinViewModel(CObjPtr iScope, bool bTypeEdit);
};

////////////////////////////////////////////////////////////Gui_t
class Gui_t : protected virtual IGui_t,
	public ADCOutputSink
{
protected:
	//Main_t	&mrMain;
	My::IGui *mpIGui;
	Core_t *mpCore;

public:
	friend class Main_t;
	friend class TypesViewModel_t;
	friend class FilesViewModel_t;

public:
	Gui_t(My::IGui *pIGui, Main_t &rMain);
	virtual ~Gui_t();

	void setCore(Core_t *p){ mpCore = p; }
	//Main_t &main() const { return mrMain; }
	Core_t *core() const { return mpCore; }

protected:
	//IGui_t
	virtual My::IGui *igui() const { return mpIGui; }
	virtual void GuiProcessEvents() const;
	virtual bool GuiNotify(int) const;
	virtual bool GuiNotify(int, MyStreamBase &) const;
	virtual void GuiOnProjectNew() const;
	virtual void GuiOnProjectSaved() const;
	virtual void GuiOnProjectOpened() const;
	virtual void GuiOnProjectClosed() const;
	/*virtual void GuiOnToDoListPushedBack(const char *s);
	virtual void GuiOnToDoListPushedFront(const char *s);
	virtual void GuiOnToDoListPopedFront(){ GuiNotify(adcui::UIMSG_TODOLIST_POPPED_FRONT); }*/
	virtual void GuiOnToDoListChanged() const { GuiNotify(adcui::UIMSG_TODOLIST_CHANGED); }
	//virtual void GuiOnProjectModified() const;
	virtual void GuiOnAnalizerStarted() const { GuiNotify(adcui::UIMSG_ANALIZER_STARTED); }
	virtual void GuiOnAnalizerPaused() const { GuiNotify(adcui::UIMSG_ANALIZER_PAUSED); }
	virtual void GuiOnAnalizerResumed() const { GuiNotify(adcui::UIMSG_ANALIZER_RESUMED); }
	virtual void GuiOnAnalizerStopped() const { GuiNotify(adcui::UIMSG_ANALIZER_STOPPED); }
	virtual void GuiOnPreanalized() const { GuiNotify(adcui::UIMSG_PREANALIZED); }
	virtual void GuiOnShowAnalizerInfo(const MyString &s) const;
	virtual void GuiOnShowProgressInfo(const MyString &s) const;
	virtual void GuiOnFileListChanged() const;
	virtual void GuiOnFileModified(const MyString &relPath) const;
	virtual void GuiOnFileRenamed(const MyString &relPathOld, const MyString &relPathNew) const;
	virtual void GuiOnFilePriorRemoved(const char *fileName) const;
	virtual bool GuiOnSaveRequest() const { return GuiNotify(adcui::UIMSG_SAVE_RECOIL); }
	virtual void GuiOnProjectAboutToClose() const;
	virtual void GuiNewFolderRecoil(const MyString &relPath) const;
	virtual void GuiOnNameChanged() const { GuiNotify(adcui::UIMSG_NAME_CHANGED); }
	virtual void GuiOnCurOpChanged() const { GuiNotify(adcui::UIMSG_CUROP_CHANGED); }
	virtual void GuiOnCurFuncModified() const { GuiNotify(adcui::UIMSG_CURFUNC_MODIFIED); }
	virtual void GuiOnCurStrucModified() const { GuiNotify(adcui::UIMSG_CURSTRUC_MODIFIED); }
	virtual void GuiOnCurFolderChanged() const { GuiNotify(adcui::UIMSG_CURFOLDER_CHANGED); }
	virtual void GuiOnCurFuncChanged() const { GuiNotify(adcui::UIMSG_CURFUNC_CHANGED); }
	virtual void GuiOnSelObjChanged() const { GuiNotify(adcui::UIMSG_SELOBJ_CHANGED); }
	virtual void GuiOnLocusChanged() const { GuiNotify(adcui::UIMSG_LOCUS_CHANGED); }
	virtual void GuiOnLocusAdjusted() const { GuiNotify(adcui::UIMSG_LOCUS_ADJUSTED); }
	virtual void GuiOnTypesMapChanged() const { GuiNotify(adcui::UIMSG_TYPESMAP_CHANGED); }
	virtual void GuiOnLocalityChanged() const { GuiNotify(adcui::UIMSG_LOCALITY_CHANGED); }
	virtual void GuiOnNoOutputDirectory(const char *cmd) const;
	virtual void GuiDebuggerStarted() const { GuiNotify(adcui::UIMSG_DEBUGGER_STARTED); }
	virtual void GuiDebuggerStopped() const { GuiNotify(adcui::UIMSG_DEBUGGER_STOPPED); }
	virtual void GuiDebuggerBreak() const { GuiNotify(adcui::UIMSG_DEBUGGER_BREAK); }
	virtual void GuiDebuggerResumed() const { GuiNotify(adcui::UIMSG_DEBUGGER_RESUMED); }
	//virtual void GuiOnSyncPanes(int line) const;
	virtual void GuiOnProjectModified() const;
	virtual void GuiShowFile(const char *fileName, const char *extra, bool bAsk) const;

	virtual const char* GuiColorTag(adcui::LogColorEnum c) const//for output log
	{
		struct s_t { adcui::LogColorEnum in; const char* out; } s = { c, "" };//see GuiColorTag
		mpIGui->CallEvent(adcui::MSGID_OUTPUT_COLOR, &s);
		return s.out;
	}

protected:
	//ADCOutputSink
	virtual void GuiOutputReady(MyStreamBase &);
	virtual void GuiOutputReady();

public:
	//Project_t &project() const;
	//bool hasProject() const;
};

template <typename T = Probe_t>
class ContextSafe_t
{
	Core_t &rCore;
	T *pCtx;
public:
	ContextSafe_t(Core_t &r)
		: rCore(r),
		pCtx(r.getContext())
	{
	}
	~ContextSafe_t()
	{
		rCore.releaseContext(pCtx);
	}
	operator bool() const { return pCtx != nullptr; }
	bool empty() const { return !pCtx && pCtx->locus().empty(); }
	T *get() const { return pCtx; }
};


Gui_t *CreateADBGuiBroker(My::IGui *pIGui, Main_t &rMain, bool bEnableOutputCapture);



