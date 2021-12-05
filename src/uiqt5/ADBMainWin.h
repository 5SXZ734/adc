#pragma once

#include <set>
#include <QDialog>
#include "sx/SxMainWindow.h"
#include "ADCTextView.h"

class QToolBar;
class QMenu;
class QLabel;
class QDragEnterEvent;
class QDropEvent;
class QPrinter;

class ADCApp;
class ADCProjectWin;
class ADC_OutputWin;
class ADCTasksWin;
class ADCStream;
class MyStreamBase;
class DocumentObject;
class ADCBinWin;
class ADCModelData;
class ADCTypesWin;
class ADCTypesView;
class ADCResWin;
class ADCInterDataWin;
class ADCResView;
class ADCNamesWin;
class ADCNamesView;
class ADCExportsWin;
class ADCExportsView;
class ADCTypeCompleter;
struct ADCCurInfo;
class ADCOpenFileDialog;
class ADCConfigDlg;
class ADCTypesDlg;
class ADBResManager;

#define	ADC_NO_PRINTER

namespace adcui { class IADCTextModel; }

typedef	SxMainWindow	ADCMainWinBase;

class ADBMainWin: public ADCMainWinBase
{
Q_OBJECT

protected:
	enum PAGE_e
	{
		PAGE_OUTPUT,
		PAGE_FILES,
		PAGE_DASM,
		PAGE_TASKS,
		PAGE_TYPES,
		PAGE_RESOURCES,
		PAGE_CANVAS,
		PAGE_STRUCED,//structure editor
		//PAGE_TYPEBLDR,
		PAGE_NAMES,
		PAGE_EXPORTS,
		PAGE_IMPORTS,
		//..
		PAGE_TOTAL
	};
public:
    ADBMainWin(ADBResManager&, adcui::IADBCore&);
    virtual ~ADBMainWin();

	adcui::IADBCore& ADB() { return mrADB; }

	static int DEFAULT_ASSBARCONFIG()
	{
		return 0;
	}
	static int DEFAULT_WINDOCKCONFIG()
	{
		return (1 << PAGE_OUTPUT) | (1 << PAGE_FILES) | (1 << PAGE_RESOURCES) | (1 << PAGE_STRUCED) | (1 << PAGE_CANVAS);
	}
	static QString DEFAULT_PAGESHOWCONFIG()
	{
		QStringList l;
		l << tr("Output");
		l << tr("Files");
		return l.join("\n");
	}

	virtual int HandleEvent(int, void*);
	//int processMessage(int msgID, void * pData);
	virtual void OnDebuggerStopped() {}

	DocumentObject* CreateBinWin(QString, QString);
	//DocumentObject* CreateResourceWin(QString, QString);
	DocumentObject* CreateTypesView(QString, QString);
	DocumentObject* CreateNamesView(QString, QString);
	DocumentObject* CreateResourceView(QString, QString);

protected:
    virtual void closeEvent( QCloseEvent * );
	virtual void showEvent(QShowEvent *);
	void dragEnterEvent(QDragEnterEvent *);
	void dropEvent(QDropEvent *);

	//void CreateBottomBar();
	void CreateProjectWin();
	void CreateOutputWin();
	//void CreateDisassemblyWin();
	void DestroyDisassemblyWin();
	void CreateTasksWin();
	void CreateTypesWin();
	void CreateNamesWin();
	void CreateExportsWin();
	void CreateImportsWin();
	void CreateResourceWin();
	void CreateCanvasWin();
	void CreateInterDataWin();
	//void CreateTypeEdWin();

	//virtual void setCurrentObject(QWidget * pWidget);
	bool IsCommandAvailable(const QString&);

	void CreateToolbars();
	void CreateActions();
	void CreateMenus();

	virtual void slotTabbedWindowsToggled(bool);
	virtual void slotPageAboutToClose(QWidget *);

	void OnOutput(MyStreamBase *);
	bool handleOutput(ADCStream &);
	//void OnFileListChanged(ADCStream &);
	virtual void OnProjectAboutToClose();
	virtual void OnProjectStatus(bool bActive);
	virtual void OnContextIdChanged();
	virtual void OnShowDocumentAtHint(DocumentObject*, QString);


protected:
	ADCApp &app();
	bool createMRUs();
	void updateRecentProjects(const QString &);
	void updateRecentScripts(const QString &);
	void enableDebuggerActions(int);
	void updateTitle();
	void registerModelData(QString, ADCModelData *);
	ADCModelData *findModel(QString, int);
	virtual void showSourceWin(QString fileName, QString atHint, bool bAskToOpen);
	ADCOpenFileDialog* openModuleDialog(QString);
	static QString fixPath(QString);
	static QString unfixPath(QString);
	static int toFileKind(QString);
	static adcui::FolderTypeEnum CheckSpecialFile(QString);
	void applyCoreSettings();

protected slots:
	void slotSaveListing(QString);
	void slotNew();
	void slotNewScript();
	void slotOpen();
	void slotAddModule();
	void slotRecentFile(const QString&);
	void slotRecentScript(const QString&);
	void slotCloseProject();
	void slotSave();
	void slotSaveAs();
	void slotPrint();

	void slotRefreshBinaryDump(int);
	void slotCommand(const QString&, const QString&);
	void slotPostCommand(const QString&, bool bEcho = true);
	void slotCallCommand(const QString&, ADCStream&);
	void slotDecompile();
	void slotStart();
	void slotPause();
	void slotStop();
	void slotGoTaskTop();
	void slotTraceModeToggled(bool);

	void slotAbout();

	void slotShowProjectView();
	void slotShowOutputView();
	void slotShowInputLine();
	void slotShowDisassemblyView();
	//void slotShowDecompyleView();
	void slotShowTasksView();
	void slotShowTypesView();
	void slotShowNamesView();
	void slotShowExportsView();
	void slotShowImportsView();
	void slotShowResourcesView();
	void slotShowCanvasWin();
	void slotShowStrucEditView();
	void slotOpenTypeDlg();

	void slotAvailCmdListChanget(const QStringList&);
	void slotShowDisassemblyAt(QString s);
	void slotDumpResources(ADCStream&);
	//void slotDumpTypes(ADCStream&);

	void slotRequestModel(ADCTypesDlg&, QString);
	void slotRequestModel(ADCTypesWin&, QString);
	void slotRequestModel(ADCTypesView&, QString);
	void slotRequestModel(ADCNamesWin&, QString);
	void slotRequestModel(ADCNamesView&, QString);
	void slotRequestModel(ADCExportsWin&, QString);
	void slotRequestModel(ADCExportsView&, QString);

	//void slotRequestBinModel(ADCTypesWin &, QString);
	void slotRequestModel(ADCProjectWin&);
	void slotRequestModel(ADCResWin&);
	void slotRequestModel(ADCResView&, QString);
	//void slotRequestModel(ADCBinWin &, QString);
	void slotRequestModel(ADCInterDataWin&, QString);
	void slotRequestModel(ADCTypeCompleter&, QString);

	virtual void slotShowSourceWin(QString, QString);
	virtual void slotActivateFile(QString);

protected slots:
	void slotAppendText(const char * str);
	void slotCommandSucceeded(int cmdID);
	//void slotCurPosChanged(ADDR pos);
	void slotProjectClosed();
	void slotProjectChanged();
	void slotProjectSaved();
	void slotProjectOpened();
	void slotProjectNew();
	void slotAnlzStatusChanged(int);
	void slotDialAnalizerInfo(QString);
	void slotDialProgressInfo(QString);
	void slotSetTargetDirectory();
	void slotGenerateSources(QString);
	void slotDumpExports(QString);
	void slotCalculateDependencies(QString);
	void slotDumpSourceFile(QString, QString);
	void slotNoOutputDirectoryRecoil(QString);
	//void slotUpdateFileList();
	//void slotPopulateFiles(ADCStream &);
	//void slotShowFontsDlg();
	void slotShowOptionsDlg();
	void slotPrefsApplied();
	void slotApplyFont();
	//void slotFontApplied(QFont);
	void slotNewFileOrFolder(QString);
	void slotDeleteFileOrFolder(QString);
	void slotDebug();
	void slotDebugStepInto();
	void slotDebugStepOver();
	void slotDebugStepOut();
	void slotDebugToggleBreakpoint();
	void slotDebugContinue();
	virtual void slotBinaryPanePicked(int, bool);
	void slotContextIdInq(unsigned &);
	void slotLocusInfoChanged(QString, int);
	//void openRecentFile();

	void slotResourceItemClicked(QString);
	void slotResourceItemActivated(QString);

	void slotTaskItemActivated(QString);
	void slotTaskItemClicked(QString);

	void slotTypeItemDoubleClicked(QString);
	void slotPopulateTasks(ADCStream &);
	void slotGetPixmapAtCP(QPixmap &);
	void slotPickModuleType(QString);
	void slotReleaseModel(QString);
	void slotReleaseModelData(ADCModelData *);
	void slotTabifiedDockWidgetActivated(QDockWidget *);
	void slotFindText();

signals:
	void signalAppendOutput(const QString &, int);
	void signalClearOutput();
	void signalFlushOutput();

	void signalProjectNew();
	void signalProjectClosed();
	void signalProjectChanged();
	void signalProjectModified();
	void signalFileListChanged();
	void signalAppendText(const char *);
	void signalPreanalized();

	void signalPostCommand(const QString &, bool = true);
	void signalCallCommand(const QString &, ADCStream &);

	void signalToDoListPushedBack(QString, QString);
	void signalToDoListPushedFront(QString, QString);
	void signalToDoListPoppedFront();
	void signalShowDisassemblyAt(QString, int);
	void signalTaskListChanged();
	void signalAnalysisStarted();
	void signalAnalysisFinished();
	void signalFileInvalidated(int);
	void signalHighlightLocationAt(QString, QString, QString, int);
	//void signalFocusTaskTop(int);
	void signalSelObjChanged();
	void signalLocusAdjusted();
	void signalTypesMapChanged();
	void signalProjectAboutToClose();
	void signalSyncPanesResponce(int, bool);
	void signalNameChanged();
	void signalCurFileChanged(QString);
	void signalShowCurrentFile();
	void signalDebuggerBreak();
	void signalGoToTaskTop();
	void signalContextIdChanged(unsigned);
	void signalNewFolderRecoil(QString);
	void signalFolderRenamed(QString);
	void signalToggleInputLine();
	void signalOpenTypesView(QString);
	void signalOpenNamesView(QString);
	void signalOpenExportsView(QString);
	void signalOpenImportsView(QString);
	void signalOpenTemplatesView(QString);
	void signalOpenResView(QString);
	void signalEditType(QString);
	void signalLocusChanged(QString, int);
	void signalUpdateContextTypes();
	void signalBinaryDumpPicked();
	void signalResourceItemPicked();
	void signalCurInfoChanged(const ADCCurInfo &);

protected:
	ADBResManager& mrResMgr;
	adcui::IADBCore& mrADB;

	//File
	QAction*	mpFileNewAction;
	QAction*	mpFileAddBinaryAction;
	QAction*	mpFileNewScriptAction;
	QAction*	mpFileOpenAction;
	QAction*	mpFileCloseAction;
	QAction*	mpFileSaveAction;
	QAction*	mpFileSaveAsAction;
	QAction*	mpFilePrintAction;
	QAction*	mpFileQuitAction;

	//View
	QAction*	mpShowProjectAction;
	QAction*	mpShowOutputAction;
	QAction*	mpShowInputAction;
	//QAction*	mpShowDisassemblyAction;
	QAction*	mpShowTasksAction;
	QAction*	mpShowInterDataViewAction;
	QAction*	mpOpenTypesDlgAction;
	QAction*	mpShowCanvasAction;

	QAction*	mpViewMSepa;
	QAction*	mpViewTSepa;

	//QAction*	mpMDIenvAction;
	//QAction*	mpShowFontsAction;
	//QAction*	mpShowProtoDlgAction;
	QAction*	mpFindAction;
	QAction*	mpShowConfigDlgAction;

	//Analysis
	//QAction	*mpDecompileAction;
	QAction*	mpContinueAction;
	QAction*	mpPauseAction;
	QAction*	mpStopAction;
	QAction*	mpGoTaskTopAction;
	QAction*	mpTraceModeAction;
	QAction*	mpDebugAction;
	QAction*	mpDebugStepIntoAction;
	QAction*	mpDebugStepOverAction;
	QAction*	mpDebugStepOutAction;
	QAction*	mpDebugToggleBreakpointAction;
	QAction*	mpDebugContinueAction;

	//Help
	QAction	*mpAboutAction;

#ifndef ADC_NO_PRINTER
    QPrinter *printer;
#endif

    QToolBar *mpFileTBar;
	QToolBar *mpAnalTBar;
	QToolBar *mpDebugTBar;
	QToolBar *mpViewTBar;

	//QMenu *mpFileMenu;
	//QMenu *mpEditMenu;
	//QMenu *mpViewMenu;
	QMenu	*mpAnalMenu;
 	QMenu	*mpDebugMenu;
   //QMenu *mpWindowMenu;

	//QLabel *		mpCurPosDial;

	SignalMultiplexer *		mpx;
	int						mWindowsMenuID;

	std::set<QString>		mAvailCmdList;

	bool	mbHasProject;
	bool	mbInitialized;
	QString mProjectName;
	QString	mProjectPath;

	QWidget*	mpCursorDial;
	QWidget*	mpAnalizerDial;
	QWidget*	mpProgressDial;

	QAction*	mpRecentFilesAction;
	QAction*	mpRecentScriptsAction;

	QString	mLastModule;
	QString	mLastScript;
	QString	mLastProject;

	ADCModelDataMap	mModels;

	ADCTypesDlg *mpTypesDlg;

	ADCOpenFileDialog* mpOpenModuleDlg;

	ADCConfigDlg* mpConfigDlg;
	//unsigned	mContextId;
};


class ADCAboutDlg : public QDialog
{
Q_OBJECT
public:
	ADCAboutDlg(QString, QString, QString, QWidget*);
protected slots:
	void slotLinkActivated(const QString &);
	void slotCopyContactAddress();
protected:
	virtual void mousePressEvent(QMouseEvent *)
	{
		close();
	}
private:
	static const QString sHyperLink;
	QString mProductName;
	QString mProductVersion;
};

