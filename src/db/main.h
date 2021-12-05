#pragma once

#include <list>
#include "qx/MyPath.h"
#include "qx/QxReadWriteMutex.h"
#include "qx/DynLib.h"
#include "qx/MyApp.h"

#include "shared/misc.h"
#include "script.h"
#include "type_struc.h"
#include "startup.h"


class Folder_t;
class Obj_t;
class Struc_t;
class TypesMgr_t;
class I_Front;
struct fileinfo_t;
class Project_t;
class BinaryAnalyzer_t;
class ScriptMgr_t;
class Main_t;
class FrontInfo_t;
class I_FrontMain;
class I_DataSource;
class IGui_t;
class FullName_t;
enum class StopFlag;

namespace adc
{
	enum EventId
	{
		EVENT_NULL = SXEVENT_USER,
		EVENT_EXIT,
		EVENT_CREATE,
		EVENT_COMMAND,
		EVENT_SCRIPT,
		EVENT_ANLZ_PROCESS,
		EVENT_ANLZ_RESUME,
		//EVENT_OPEN_DISPLAY,
		//EVENT_CLOSE_DISPLAY,
		//EVENT_COMMAND_DC,
		EVENT_COMMAND_REQUEST_STUB,
		//EVENT_SET_CUR_POS,
		EVENT_DEBUG_TERMINATED,
		//..
		EVENT_LAST
	};



	class CEventCommand : public SxCustomEvent
	{
	public:
		CEventCommand(const std::string &s, bool bEcho = true)
			: SxCustomEvent((SxEventEnum)EVENT_COMMAND, nullptr),
			m_command(s),
			m_result(nullptr),
			m_bEcho(bEcho)
		{
		}
		CEventCommand(I_Context *pIContext, const std::string& s, bool bEcho = true)
			: SxCustomEvent((SxEventEnum)EVENT_COMMAND, nullptr),
			m_command(s),
			m_result(nullptr),
			m_bEcho(bEcho)
		{
			setContextZ(pIContext);
		}
		virtual ~CEventCommand()
		{
		}
		void setRefPath(const MyPath &refPath){ mRefPath = refPath; }
		void setResult(MyStreamBase *pss){ m_result = pss; }
		I_Context *setContextZ(I_Context *p){ mcx.set(p); return mcx.ptr(); }
		I_Context *contextZ(){ return mcx.ptr(); }
		template <typename T>
		T *getCtx(){ return dynamic_cast<T*>(mcx.get()); }
	public:
		MyString		m_command;
		MyStreamBase	*m_result;
		MyPath			mRefPath;
		bool	m_bEcho;
	private:
		RefPtr_t<I_Context> mcx;
	};

	/*struct CEventSetCurPos : public SxCustomEvent
	{
		CEventSetCurPos(const Locus_t &a, Obj_t *pObj)
			: SxCustomEvent((SxEventEnum)EVENT_SET_CUR_POS, nullptr),
			m_location(a),
			mpObj(pObj)
		{
		}
		Locus_t	m_location;
		Obj_t *mpObj;
	};*/

	/*struct CEventRequestStub : public SxCustomEvent
	{
		CEventRequestStub(const AnalyzerContext_t &ctx)
			: SxCustomEvent((SxEventEnum)EVENT_COMMAND_REQUEST_STUB, nullptr),
			m_ctx(ctx)
		{
		}
		AnalyzerContext_t m_ctx;
	};*/

	class CEventCreate : public SxCustomEvent
	{
	public:
		const SturtupInfo_t &m_si;
	public:
		CEventCreate(const SturtupInfo_t &si)
			: SxCustomEvent((SxEventEnum)EVENT_CREATE, nullptr),
			m_si(si)
		{
		}
	};

	struct CEventScript : public SxCustomEvent
	{
		CEventScript(const MyString &path, bool bAbortIf)
			: SxCustomEvent((SxEventEnum)EVENT_SCRIPT, nullptr),
			m_path(path),
			m_bAbortIf(bAbortIf)
			//mScript(path, bAbortIf)
		{
			//m_autodelete = false;
		}
		//ScriptMgr_t mScript;
		MyString m_path;
		bool m_bAbortIf;
	};

	struct CEventStream : public SxCustomEvent
	{
		CEventStream(EventId eId)
			: SxCustomEvent((SxEventEnum)eId, nullptr){}
		MyStream	mss;
	};

}//namespace adc

typedef std::map<MyString, FrontInfo_t *>	FrontMap;
typedef FrontMap::iterator		FrontMapIt;
typedef FrontMap::const_iterator		FrontMapCIt;


//********************************************************* (Main_t)
class Main_t : public My::EventLoop,
	public CMDServer_t,
	public I_Main
{
	//typedef std::map<std::string, int>	mapExtTyp_t;
	//typedef mapExtTyp_t::iterator		mapExtTyp_it;
	//typedef mapExtTyp_t::const_iterator		mapExtTyp_itc;

	MyString	mAppName;
	MyString	mAppCodeName;
	MyString	mAppVersion;
	MyString	mCompanyName;

	IGui_t		*mpGui;
	uint32_t	m_dwDumpFlags;
	MyPath		mOutPath;		//default path
	MyPath		mExePath;

protected:

	FrontMap	mFrontends;
	//mapExtTyp_t		mExtTypeRefs;
	//FormatterMap	mFmtFactory;

	Project_t*		mpProject;

	bool			mbUISignalsBlocked;

	QxReadWriteMutex<std::recursive_mutex>	mxProject;

	DynLibMgr	mDynLibMgr;

	MemoryMgr_t *mpMemoryMgr;

	ScriptMgr_t	*mpScript;
	volatile int	mWriteLock;
	volatile int	mReadLock;

public:
	bool m_bDebugMode;

	//const StartupInfo_t &mrSturtupInfo;
	Options_t	mOptions;

	QxTime	mUiNotifier;
	QxTime	mStatTimer;

public:
	Main_t(int argc, char ** argv, const char *appName, const char *appCodeName, const char *appVersion, const char* comanyName, const Options_t &);
	virtual ~Main_t();
	FieldPtr createExternalType(MyString typeKey, Locus_t &, MyString options);
	FrontMap& frontends() { return mFrontends; }
	const FrontMap& frontends() const { return mFrontends; }
	virtual My::IUnk *createGuiBroker(My::IGui *pIGui, bool bEnableOutputCapture);
	const IGui_t &gui() const { return *mpGui; }
	void setGui(IGui_t* p);
	//const StartupInfo_t &startupInfo() const { return mrSturtupInfo; }
	const char* colorTag(adcui::LogColorEnum) const;
	std::ostream& printWarning() const;
	std::ostream& printError() const;
	std::ostream& printInfo() const;
	FullName_t hyperLinked(const FullName_t&) const;

	void applyOptions(const Options_t& a) { mOptions = a; }
	const Options_t options() const { return mOptions; }

	void releaseFrontend(const char *);
	I_DynamicType *getDynamicType(const char *, FrontInfo_t **);
	I_DynamicType *getContextDependentType(const char *);
	I_Code *getCodeType(const char *);
	I_Front *getFrontend(const char *, const I_DataSourceBase *);

	virtual void onCreate(const SturtupInfo_t&);
	virtual bool OpenScript(const MyString &, bool);
	ScriptMgr_t *OpenScript();
	void CloseScript();
	ScriptMgr_t &script() { return *mpScript; }
	bool isProcessingScript() const { return mpScript != nullptr; }
	int writeLock() const { return mWriteLock; }
	int readLock() const { return mReadLock; }

	virtual void NewGlobalMemMgr();
	void DeleteGlobalMemMgr();
	MemoryMgr_t &memMgr(){ return *mpMemoryMgr; }

	DynLibMgr &dynLibMgr(){ return mDynLibMgr; }

	void askFrontend(const MyPath &);
	I_FrontMain *findFrontByName(const char *);
	MyPath frontPath(const char *) const;
	MyPath frontEnd2Path(I_FrontMain *);
	int writeExternalTypes(MyStreamBase &) const;
	void clearExternalTypes();
	bool checkUiEvents(bool);

	const MyString& appName() const { return mAppName; }
	const MyString& appCodeName() const { return mAppCodeName; }
	const MyString& appVersion() const { return mAppVersion; }
	const MyString& companyName() const { return mCompanyName; }

	int processEvent2(const SxCustomEvent *);
protected:
	// SxApp
	virtual int processEvent(const SxCustomEvent *);
	virtual bool canQuit();
	virtual void OnBusy();
	virtual void OnIdle();
	virtual void OnTimeout();

	bool CheckEvents();
	void BuildDynamicTypesList();

	//FrontInfoMapIt loadFrontendIt(const MyPath &);
	virtual Project_t *newProject(MemoryMgr_t &);

	//I_Main
	virtual const char* executablePath() const;
	virtual const char* frontPathFromName(const char* frontName) const;
	virtual const char* protoPath(const char* fileName) const;

public:
	void OnAnlzReady();
	void setDebugMode(bool b){
		m_bDebugMode = b; }
	bool debugMode() const { return m_bDebugMode; }
	//void setStopFlag(StopFlag i){
		//m_eStopFlag = i; }
	//StopFlag stopFlag() const { return m_eStopFlag; }
	void resumeAnalysis(StopFlag);
	bool writeToDoList(MyStreamBase &);
	int checkStopRequest() const;

	const MyPath &exePath(){ return mExePath; }

	Folder_t *newProjectFromPath(const MyPath &);

	uint32_t	flags(){ return m_dwDumpFlags; }
	void	setFlags(uint32_t f);
	void	clearFlags(uint32_t f);


	bool SaveToFile(MyString, unsigned flags);//flags: not used in adb
	bool LoadFromFile(MyString);

	virtual void SaveToStream(std::ostream &, MyString path, unsigned flags);
	virtual void LoadFromStream(std::istream &, MyString path);

	//int		OpenFile(const char *pFileName, int nFileType = -1);
	void closeProject();
	int	CloseProject();// I_Context*);
	int	NewProject(const MyPath &, I_Context *, MyString);
	virtual int	LoadProject(const char* fname);// , I_Context*);
	virtual void DestroyProject();

	//TypePtr	projectRef() const { return miProject; }
	bool	hasProject() const { return mpProject != nullptr; }
	Project_t &project() const;
	void		postContextCommand(const MyString &, I_Context *, bool bEcho = false);

	int loadProject(const MyPath &);

	//TypesMgr_t	*	typeMgr() const;

	int lockProjectRead(bool bLock)
	{
		if (bLock)
		{
			mxProject.lockRead();
			mReadLock++;
			return 1;
		}
		mReadLock--;
		mxProject.unlockRead();
		return -1;
	}
	int lockProjectWrite(bool bLock)
	{
		if (bLock)
		{
			mxProject.lockWrite();
			mWriteLock++;
			return 1;
		}
		mWriteLock--;
		mxProject.unlockWrite();
		return -1;
	}

protected:
	bool validateProjectPath(MyString &);

	// *** commands

#define MYCOMMAND(name) \
	private: static int COMMAND_##name(CMDServer_t *, Cmd_t &); \
	protected: virtual int OnCommand_##name(Cmd_t &);

	//commands
	MYCOMMAND(new);
	MYCOMMAND(script);
	MYCOMMAND(open);
	MYCOMMAND(save);
#undef MYCOMMAND

private:
	class MainCmdMap_t : public CMDServerCommandMap
	{
	public:
		MainCmdMap_t();
	};

	MainCmdMap_t	mCmdMap;
};


extern Main_t *	gpMain;	
#undef MAIN
#define	MAIN	(*gpMain)

#ifdef WIN32
	#define DLL_EXT	"dll"
	#define LIB_PFX	""
#else
	#define DLL_EXT	"so"
	#define LIB_PFX	"lib"
#endif

class WriteLocker
{
public:
	WriteLocker(){ MAIN.lockProjectWrite(true); }
	~WriteLocker(){ MAIN.lockProjectWrite(false); }
};

class MainReadLocker
{
	Main_t &mr;
public:
	MainReadLocker(Main_t *p) : mr(*p) { mr.lockProjectWrite(true); }
	~MainReadLocker(){ mr.lockProjectWrite(false); }
};



