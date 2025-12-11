#include "ADBMainWin.h"

#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QUrl>
#include <QObjectList>
#include <QMimeData>
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QDesktopServices>
#include <QClipboard>
#include <QCloseEvent>

#include <QApplication>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>
#include <QFontDialog>
#include <QLabel>
#include <QStatusBar>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QAction>
#if QT_VERSION_MAJOR < 6
#include <QDesktopWidget>
#include <QtCore/QRegExp>
#else
#include <QRegularExpression>
#endif

#include "qsil/xshared.h"
#include "qsil/xresmgr.h"
#include "sx/SxDocument.h"
#include "sx/SxSignalMultiplexer.h"
#include "sx/SxDockBar.h"
#include "sx/SxViewMgrTabs.h"
#include "sx/SxOutputWin.h"
#include "ADCApp.h"
#include "ADBFilesWin.h"
#include "ADCResourceWin.h"
#include "ADCTasksWin.h"
#include "ADCOutputWin.h"
#include "ADCBinWin.h"
#include "ADCUserPrefs.h"
#include "ADCTypesWin.h"
#include "ADCNamesWin.h"
#include "ADCUtils.h"
#include "ADBDataWin.h"
#include "ADCConfigDlg.h"
//#include <vld.h>

#ifndef ADC_NO_PRINTER
#include <QtPrintSupport/QPrinter>
#endif

#define NEW_VIEWTABSMGR	0

ADBMainWin::ADBMainWin(ADBResManager& rResMgr, adcui::IADBCore& rADB)
	: SxMainWindow(nullptr, "ADCMAINWIN"),
	mrResMgr(rResMgr),
	mrADB(rADB),
	mWindowsMenuID(-1),
	mbHasProject(false),
	mbInitialized(false),
	mpTypesDlg(nullptr),
	mpOpenModuleDlg(nullptr),
	mpConfigDlg(nullptr)
	//mContextId(adcui::CONTEXTID_NULL)
{
	//setWindowIcon(QIcon(":reverse32.png"));
	//setWindowIcon(QIcon(":logo64.png"));
	//setWindowIcon(QIcon(":notepad.16.png"));
	setWindowIcon(QIcon(":wrench_32.png"));
	//setWindowIcon(QIcon(":wrench.16.png"));
	//setWindowIcon(QIcon(":wrench2.png"));

	CreateActions();
	CreateMenus();
	CreateToolbars();

	//QStatusBar * pStatusBar = statusBar();

	/*mpCurPosDial = new QLabel(this);
	mpCurPosDial->setAlignment(Qt::AlignCenter);
	mpCurPosDial->setTextFormat(Qt::RichText);
	mpCurPosDial->setShown(false);
	pStatusBar->addPermanentWidget(mpCurPosDial);*/

	//create view manager
	//	mpTabbedWindowsAction->blockSignals(true);
	//	mpTabbedWindowsAction->setOn( true);//mpUPrefs->GetMxTabbedWindows() );//should be connection
	//	mpTabbedWindowsAction->blockSignals(false);
	//	slotTabbedWindowsToggled(mpTabbedWindowsAction->isOn());

//	slotTabbedWindowsToggled(UPREFS.getTabbedWindows());
	mpTabbedWindowsAction->setChecked(mrResMgr.getTabbedWindows());//no connection yet

	mpWindowsMenuAction->setVisible(false);//!mpUPrefs->GetMxTabbedWindows() );

	//CreateBottomBar();
	setAcceptDrops(true);

	CreateProjectWin();
	CreateOutputWin();
	//	CreateDisassemblyWin();
	CreateTasksWin();
	CreateTypesWin();
	CreateNamesWin();
	CreateExportsWin();
	CreateImportsWin();
	CreateResourceWin();
	CreateCanvasWin();
	CreateInterDataWin();
	//CreateTypeEdWin();
	

	/*ADBResManager * pResMgr = app().resManager();
	if ( !pResMgr->GetPageHideConfig( PAGE_OUTPUT ) )
	slotShowOutputView();
	if ( !pResMgr->GetPageHideConfig( PAGE_FILES ) )
	slotShowProjectView();
	if ( !pResMgr->GetPageHideConfig( PAGE_DASM ) )
	slotShowDisassemblyView();*/

	//?	slotDockPage(OutputWin(), Qt::DockBottom);
	//	closePage(OutputWin());

	/*
	mpExprBar = new QDockWindow( this, "EXPRBAR" );
	mpExprBar->setCaption( "Expression" );
	moveDockWindow( mpExprBar, Qt::DockRight );
	QWidget * pExprView = new QTextEdit( mpExprBar, "EXPRVIEW" );
	mpExprBar->setWidget( pExprView );
	mpExprBar->setCloseMode( QDockWindow::Always );
	mpExprBar->setResizeEnabled( true );

	mpBlockBar = new QDockWindow( this, "BLOCKBAR" );
	mpBlockBar->setCaption( "Structure" );
	moveDockWindow( mpBlockBar, Qt::DockRight );
	QWidget * pBlockView = new QTextEdit( mpBlockBar, "BLOCKVIEW" );
	mpBlockBar->setWidget( pBlockView );
	mpBlockBar->setCloseMode( QDockWindow::Always );
	mpBlockBar->setResizeEnabled( true );*/

	setStatusSlotText(mpCopyright, QString());

	mpCursorDial = addStatusSlot();
	setStatusSlotText(mpCursorDial, tr("<nobr> Line: 0000 Col: 00 </nobr>"));
	mpCursorDial->setMinimumSize(mpCursorDial->sizeHint());
	setStatusSlotText(mpCursorDial, tr("<nobr> Line: 1 Col: 1 </nobr>"));
	mpCursorDial->hide();

	mpAnalizerDial = addStatusSlot();
	setStatusSlotText(mpAnalizerDial, "VA");
	mpAnalizerDial->hide();

	mpProgressDial = addStatusSlot();
	setStatusSlotText(mpProgressDial, "PROGRESS");
	mpProgressDial->hide();

	connect(this, SIGNAL(tabifiedDockWidgetActivated(QDockWidget *)), SLOT(slotTabifiedDockWidgetActivated(QDockWidget *)));

	/*mpProgressDial = addStatusSlot();
	setStatusSlotText(mpProgressDial, "PROGRESS");
	mpProgressDial->hide();*/

	//	UPREFS.loadWindowGeometry(this);
	//	UPREFS.loadShortcuts(this);

	/*	QDockWindow * pDockWin;

	showPageEx(ProjectWin(), true);
	pDockWin = getDockWindow(ProjectWin());
	if (pDockWin)
	moveDockWindow(pDockWin, Qt::DockLeft);

	showPageEx(OutputWin(), true);
	pDockWin = getDockWindow(OutputWin());
	if (pDockWin)
	moveDockWindow(pDockWin, Qt::DockBottom);*/
	slotAnlzStatusChanged(-1);//just update actions
	enableDebuggerActions(0);
	OnProjectStatus(false);//not active

	statusBar()->showMessage(tr("Ready"), 2000);

	//UPREFS.loadWindow(this);
//	slotApplyFont();
	applyCoreSettings();
}

ADBMainWin::~ADBMainWin()
{
#ifndef ADC_NO_PRINTER
	delete printer;
#endif

/*	UPREFS.setABarConfig(getABarConfig());
	UPREFS.setWinDockConfig(getWinDockConfig());
	UPREFS.setMDIWinLayout(GetMDIWinLayout(UPREFS.getSaveEditDocs()));

	UPREFS.setValue("geometry", saveGeometry());
	UPREFS.setValue("windowState", saveState());*/
}

ADCApp& ADBMainWin::app()
{
	return *static_cast<ADCApp*>(qApp);
}

void ADBMainWin::CreateActions()
{
	// FILE

	mpFileNewAction = new QAction(SxIcon(":file_new_{16|24}.png"), tr("New..."), this);//std_new_16.png
	mpFileNewAction->setShortcut(tr("Ctrl+N"));
	connect(mpFileNewAction, SIGNAL(triggered()), SLOT(slotNew()));

	mpFileAddBinaryAction = new QAction(QIcon(":binary_add.png"), tr("Add Module..."), this);
	connect(mpFileAddBinaryAction, SIGNAL(triggered()), SLOT(slotAddModule()));

	mpFileNewScriptAction = new QAction(QIcon(":script.png"), tr("Run Script..."), this);
	//mpFileNewScriptAction->setShortcut(tr(""));
	connect(mpFileNewScriptAction, SIGNAL(triggered()), SLOT(slotNewScript()));

	mpFileOpenAction = new QAction(SxIcon(":open_file_{16|24}.png"), tr("Open..."), this);//std_open_16.png
	connect(mpFileOpenAction, SIGNAL(triggered()), SLOT(slotOpen()));

	mpFileCloseAction = new QAction(QIcon(".png"), tr("Close"), this);
	//mpFileCloseAction->setShortcut( tr("Ctrl+C") );
	connect(mpFileCloseAction, SIGNAL(triggered()), SLOT(slotCloseProject()));

	mpFileSaveAction = new QAction(SxIcon(":file_save_{16|24}.png"), tr("Save Project"), this);//std_save_16.png
	mpFileSaveAction->setShortcut(tr("Ctrl+S"));
	connect(mpFileSaveAction, SIGNAL(triggered()), SLOT(slotSave()));

	mpFileSaveAsAction = new QAction(QIcon(".png"), tr("Save Project As..."), this);
	connect(mpFileSaveAsAction, SIGNAL(triggered()), SLOT(slotSaveAs()));

	mpFilePrintAction = new QAction(QIcon(":std_print_16.png"), tr("Print..."), this);
	//mpFilePrintAction->setShortcut(tr("Ctrl+P"));
	connect(mpFilePrintAction, SIGNAL(triggered()), SLOT(slotPrint()));

	/*for (int i = 0; i < MaxRecentFiles; ++i)
	{
		recentFileActs[i] = new QAction(this);
		recentFileActs[i]->setVisible(false);
		connect(recentFileActs[i], SIGNAL(triggered()), SLOT(openRecentFile()));
	}*/

#ifndef ADC_NO_PRINTER
	printer = new QPrinter(QPrinter::HighResolution);
	//QPixmap printIcon = QPixmap::fromMimeSource( "std_print_16.png" );
	//QToolButton * filePrint	= new QToolButton( printIcon, "Print File", QString(),
	//		   this, SLOT(print()), mpFileTools, "print file" );
	//QWhatsThis::add( filePrint, filePrintText );
#endif

	mpFileQuitAction = new QAction(SxIcon(":log_out_{16|24}.png"), tr("Exit"), this);
	mpFileQuitAction->setShortcut(tr("Ctrl+Q"));
	connect(mpFileQuitAction, SIGNAL(triggered()), SLOT(close()));
	//	    file->insertItem( "&Quit", qApp, SLOT( closeAllWindows() ), CTRL+Key_Q );

	//EDIT

	//mpx = new SignalMultiplexer(this);

/*	spToggleRootAction = new QAction(tr("&Root"), this);
	spToggleRootAction->setShortcut(Qt::Key_R);
	spToggleRootAction->setEnabled(false);

	spToggleIfAction = new QAction(tr("&If/Switch"), this);
	spToggleIfAction->setShortcut(Qt::Key_I);
	spToggleIfAction->setEnabled(false);

	spToggleElseAction = new QAction(tr("&Else"), this);// , "ELSE_ACTION");
	spToggleElseAction->setShortcut(Qt::Key_E);
	spToggleElseAction->setEnabled(false);

	spToggleWhileAction = new QAction(tr("&While"), this);// , "WHILE_ACTION");
	spToggleWhileAction->setShortcut(Qt::Key_W);
	spToggleWhileAction->setEnabled(false);

	spCombineLogicAction = new QAction(tr("&Logic"), this);// , "LOGIC_ACTION");
	spCombineLogicAction->setShortcut(Qt::Key_L);
	spCombineLogicAction->setEnabled(false);

	spKillLogicAction = new QAction(tr("Logic &Kill"), this);
	spKillLogicAction->setShortcut(Qt::Key_K);
	spKillLogicAction->setEnabled(false);

	spDeleteAction = new QAction(tr("Logic &Delete"), this);
	spDeleteAction->setShortcut(Qt::Key_Delete);
	spDeleteAction->setEnabled(false);

	spBindDataAction = new QAction(tr("Bind/Unbind &Data"), this);
	spBindDataAction->setShortcut(Qt::Key_D);
	spBindDataAction->setEnabled(false);*/

	/*mpx->connect(SIGNAL(signalEnableToggleRoot(bool)), spToggleRootAction, SLOT(setEnabled(bool)));
	mpx->connect(SIGNAL(signalEnableToggleIf(bool)), spToggleIfAction, SLOT(setEnabled(bool)));
	mpx->connect(SIGNAL(signalEnableToggleElse(bool)), spToggleElseAction, SLOT(setEnabled(bool)));
	mpx->connect(SIGNAL(signalEnableToggleWhile(bool)), spToggleWhileAction, SLOT(setEnabled(bool)));
	mpx->connect(SIGNAL(signalEnableCombineLogic(bool)), spCombineLogicAction, SLOT(setEnabled(bool)));
	mpx->connect(SIGNAL(signalEnableKillLogic(bool)), spKillLogicAction, SLOT(setEnabled(bool)));
	mpx->connect(SIGNAL(signalEnableDelete(bool)), spDeleteAction, SLOT(setEnabled(bool)));

	mpx->connect(spToggleRootAction, SIGNAL(triggered()), SLOT(slotToggleRoot()));
	mpx->connect(spToggleIfAction, SIGNAL(triggered()), SLOT(slotToggleIf()));
	mpx->connect(spToggleElseAction, SIGNAL(triggered()), SLOT(slotToggleElse()));
	mpx->connect(spToggleWhileAction, SIGNAL(triggered()), SLOT(slotToggleWhile()));
	mpx->connect(spCombineLogicAction, SIGNAL(triggered()), SLOT(slotCombineLogic()));
	mpx->connect(spKillLogicAction, SIGNAL(triggered()), SLOT(slotKillLogic()));
	mpx->connect(spDeleteAction, SIGNAL(triggered()), SLOT(slotDelete()));?*/


	// VIEW
	//mpTabbedWindowsAction->setVisible(false);

	mpShowProjectAction = new QAction(SxIcon(":project_view_{16|24}.png"), tr("Files"), this);
	mpShowProjectAction->setStatusTip(tr("Display Files view"));
	connect(mpShowProjectAction, SIGNAL(triggered()), SLOT(slotShowProjectView()));

	mpShowOutputAction = new QAction(SxIcon(":output_{16|24}.png"), tr("Output"), this);
	mpShowOutputAction->setShortcut(tr("Ctrl+O"));
	mpShowOutputAction->setStatusTip(tr("Show Output View"));
	connect(mpShowOutputAction, SIGNAL(triggered()), SLOT(slotShowOutputView()));

	mpShowInputAction = new QAction(SxIcon(":input_line_{16|24}.png"), tr("Input Line"), this);//QIcon(":input.png")
	mpShowInputAction->setShortcut(tr("Ctrl+I"));
	mpShowInputAction->setStatusTip(tr("Toggle Input Line"));
	connect(mpShowInputAction, SIGNAL(triggered()), SLOT(slotShowInputLine()));

	//	mpShowDisassemblyAction = new QAction(QIcon(":dasm.16.png"), tr("Disassembly"), this);
	//	connect(mpShowDisassemblyAction, SIGNAL(triggered()), SLOT(slotShowDisassemblyView()));

	/*	mpShowDecompyleAction = new QAction(this, "ShowDecompyleAction");
		mpShowDecompyleAction->setIconSet(QIconSet(QPixmap::fromMimeSource(".16.png")));
		//mpShowDecompyleAction->addTo(mpDisplayTbar);
		mpShowDecompyleAction->setText(tr("Decompyle"));
		mpShowDecompyleAction->setMenuText(tr("&Decompyle"));
		mpShowDecompyleAction->setStatusTip(tr("Display Decompyle view"));
		connect( mpShowDecompyleAction, SIGNAL(triggered()), SLOT(slotShowDecompyleView()));*/

	mpShowTasksAction = new QAction(SxIcon(":task_list_{16|24}.png"), tr("Tasks"), this);
	connect(mpShowTasksAction, SIGNAL(triggered()), SLOT(slotShowTasksView()));

	//mpBuildTypeAction = newAction(tr("Build Type ..."), SLOT(slotBuildType()), tr("Ctrl+T"), QIcon(":types.png"));

	mpOpenTypesDlgAction = new QAction(SxIcon(":type_edit_{16|24}.png"), tr("Type Builder"), this);//QIcon(":type_edit3.png")
	mpOpenTypesDlgAction->setShortcut(tr("Ctrl+T"));
	connect(mpOpenTypesDlgAction, SIGNAL(triggered()), SLOT(slotOpenTypeDlg()));
	addAction(mpOpenTypesDlgAction);

	mpShowCanvasAction = new QAction(SxIcon(":canvas_{16|24}.png"), tr("Canvas"), this);//QIcon(":canvas.png")
	connect(mpShowCanvasAction, SIGNAL(triggered()), SLOT(slotShowCanvasWin()));
	
	//mpShowFontsAction = new QAction(SxIcon(":fonts_{16|24}"), tr("&Font ..."), this);
	//connect(mpShowFontsAction, SIGNAL(triggered()), SLOT(slotShowFontsDlg()));

	//mpShowProtoDlgAction = new QAction(QIcon(":type_edit4.png"), tr("Prototype Editor"), this);
	//mpShowProtoDlgAction->setShortcut(tr("Ctrl+P"));
	//connect(mpShowProtoDlgAction, SIGNAL(triggered()), SLOT(slotShowProtoDlg()));

	mpShowInterDataViewAction = new QAction(SxIcon(":inter_data_{16|24}.png"), tr("Inter Data"), this);//QIcon(":data_edit.png")
	connect(mpShowInterDataViewAction, SIGNAL(triggered()), SLOT(slotShowStrucEditView()));


	mpFindAction = new QAction(SxIcon(":find_{16|24}.png"), tr("&Find..."), this);
	mpFindAction->setShortcut(tr("Ctrl+F"));
	connect(mpFindAction, SIGNAL(triggered()), SLOT(slotFindText()));

	mpShowConfigDlgAction = new QAction(SxIcon(":options2_{16|24}.png"), tr("&Options..."), this);
	mpShowConfigDlgAction->setShortcut(tr("Ctrl+P"));
	connect(mpShowConfigDlgAction, SIGNAL(triggered()), SLOT(slotShowOptionsDlg()));


	// AMALYSIS

	/*mpDecompileAction = new QAction(QIcon(":wrench.16.png"), tr("&Decompile Function"), this);
	//mpDecompileAction->setShortcut(Qt::Key_F5);
	connect(mpDecompileAction, SIGNAL(triggered()), SLOT(slotDecompile()));
	//mpDecompileAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	*/
	mpContinueAction = new QAction(SxIcon(":start_{16|24}.png"), tr("Continue"), this);
	//mpContinueAction->setShortcut(Qt::Key_F5);
	connect(mpContinueAction, SIGNAL(triggered()), SLOT(slotStart()));

	mpPauseAction = new QAction(SxIcon(":pause_{16|24}.png"), tr("Pause"), this);
	connect(mpPauseAction, &QAction::triggered, this, [this]() {ADB().Pause(); });




	mpStopAction = new QAction(SxIcon(":stop_{16|24}.png"), tr("Stop"), this);
	connect(mpStopAction, SIGNAL(triggered()), SLOT(slotStop()));

	mpGoTaskTopAction = new QAction(QIcon(":show_next.png"), tr("Show Next"), this);
	connect(mpGoTaskTopAction, SIGNAL(triggered()), SLOT(slotGoTaskTop()));

	mpTraceModeAction = new QAction(QIcon(":trace.png"), tr("Trace Mode"), this);
	mpTraceModeAction->setCheckable(true);
	mpTraceModeAction->setStatusTip(tr("Toggle Trace mode"));
	connect(mpTraceModeAction, SIGNAL(toggled(bool)), SLOT(slotTraceModeToggled(bool)));

	// DEBUG

	mpDebugAction = new QAction(QIcon(":debug.png"), tr("Start Debugging"), this);
	//mpDebugAction->setShortcut(tr("Ctrl+"));
	connect(mpDebugAction, SIGNAL(triggered()), SLOT(slotDebug()));

	mpDebugStepIntoAction = new QAction(QIcon(":dbg_step_into.png"), tr("Step Into"), this);
	mpDebugStepIntoAction->setShortcut(tr("F11"));
	connect(mpDebugStepIntoAction, SIGNAL(triggered()), SLOT(slotDebugStepInto()));

	mpDebugStepOverAction = new QAction(QIcon(":dbg_step_over.png"), tr("Step Over"), this);
	mpDebugStepOverAction->setShortcut(tr("F10"));
	connect(mpDebugStepOverAction, SIGNAL(triggered()), SLOT(slotDebugStepOver()));

	mpDebugStepOutAction = new QAction(QIcon(":dbg_step_out.png"), tr("Step Out"), this);
	mpDebugStepOutAction->setShortcut(tr("Shift+F10"));
	connect(mpDebugStepOutAction, SIGNAL(triggered()), SLOT(slotDebugStepOut()));

	mpDebugToggleBreakpointAction = new QAction(QIcon(":dbg_bp.png"), tr("Toggle Breakpoint"), this);
	mpDebugToggleBreakpointAction->setShortcut(tr("F9"));
	connect(mpDebugToggleBreakpointAction, SIGNAL(triggered()), SLOT(slotDebugToggleBreakpoint()));

	mpDebugContinueAction = new QAction(QIcon(":.png"), tr("Continue"), this);
	mpDebugContinueAction->setShortcut(tr("F5"));
	connect(mpDebugContinueAction, SIGNAL(triggered()), SLOT(slotDebugContinue()));


	// HELP

	mpAboutAction = new QAction(QIcon(":about_16.png"), tr("About"), this);
	mpAboutAction->setShortcut(Qt::Key_F1);
	connect(mpAboutAction, SIGNAL(triggered()), SLOT(slotAbout()));
}

void ADBMainWin::OnProjectStatus(bool bActive)
{
	mpFileCloseAction->setEnabled(bActive);
	mpFileSaveAction->setEnabled(bActive);
	mpFileSaveAsAction->setEnabled(bActive);
	mpFilePrintAction->setEnabled(bActive);
	mpFileAddBinaryAction->setEnabled(bActive);
	mpDebugAction->setEnabled(bActive);
}

void ADBMainWin::CreateMenus()
{
	mpFileMenu->clear();
	//(tr("&File")
	//menuBar()->insertItem( "&File", mpFileMenu );
	//connect( mpFileMenu, SIGNAL(aboutToShow()), this, SLOT(slotFileMenuAboutToShow()) );
	mpFileMenu->addAction(mpFileNewAction);
	mpFileMenu->addAction(mpFileOpenAction);
	mpFileMenu->addAction(mpFileCloseAction);
	mpFileMenu->addAction(mpFileSaveAction);
	mpFileMenu->addAction(mpFileSaveAsAction);
	mpFileMenu->addSeparator();
	mpFileMenu->addAction(mpFileAddBinaryAction);
	mpFileMenu->addSeparator();
	mpFileMenu->addAction(mpFileNewScriptAction);
	mpFileMenu->addSeparator();
	mpFileMenu->addAction(mpFilePrintAction);

	mpFileMenu->addSeparator();

	createMRUs();
	/*separatorAct = mpFileMenu->addSeparator();
	for (int i = 0; i < MaxRecentFiles; ++i)
		mpFileMenu->addAction(recentFileActs[i]);*/

	mpFileMenu->addSeparator();
	mpFileMenu->addAction(mpFileQuitAction);

	mpEditMenu->clear();
	/*mpEditMenu = new QMenu( this );
	menuBar()->insertItem( "&Edit", mpEditMenu );
	connect( mpEditMenu, SIGNAL(aboutToShow()), this, SLOT(slotEditMenuAboutToShow()) );*/
/*	mpEditMenu->addAction(spToggleRootAction);
	mpEditMenu->addAction(spToggleIfAction);
	mpEditMenu->addAction(spToggleElseAction);
	mpEditMenu->addAction(spToggleWhileAction);
	mpEditMenu->addAction(spCombineLogicAction);
	mpEditMenu->addAction(spKillLogicAction);
	mpEditMenu->addAction(spBindDataAction);*/

	mpViewMenu->clear();
	//menuBar()->insertItem( "&View", mpViewMenu );
	//connect( mpViewMenu, SIGNAL(aboutToShow()), this, SLOT(slotViewMenuAboutToShow()) );
	mpViewMenu->addAction(mpShowProjectAction);
	mpViewMenu->addAction(mpShowOutputAction);
	mpViewMenu->addAction(mpShowInputAction);
	//	mpViewMenu->addAction(mpShowDisassemblyAction);
	mpViewMenu->addAction(mpShowTasksAction);
	//mpViewMenu->addAction(mpOpenTypesDlgAction);
	mpViewMenu->addAction(mpShowCanvasAction);
	mpViewMenu->addAction(mpShowInterDataViewAction);
	mpViewMSepa = mpViewMenu->addSeparator();
	//mpViewMenu->addAction(mpShowFontsAction);
	mpViewMenu->addAction(mpShowConfigDlgAction);


	mpAnalMenu = new QMenu(tr("&Analysis"), this);
	/*mpAnalMenu->addAction(mpDecompileAction);
	mpAnalMenu->addSeparator();*/
	mpAnalMenu->addAction(mpContinueAction);
	mpAnalMenu->addAction(mpPauseAction);
	mpAnalMenu->addAction(mpStopAction);
	mpAnalMenu->addSeparator();
	mpAnalMenu->addAction(mpGoTaskTopAction);
	mpAnalMenu->addSeparator();
	mpAnalMenu->addAction(mpTraceModeAction);

	mpDebugMenu = new QMenu(tr("&Debug"), this);
	mpDebugMenu->addAction(mpDebugAction);
	mpDebugMenu->addAction(mpDebugStepIntoAction);
	mpDebugMenu->addAction(mpDebugStepOverAction);
	mpDebugMenu->addAction(mpDebugStepOutAction);
	mpDebugMenu->addAction(mpDebugToggleBreakpointAction);
	mpDebugMenu->addAction(mpDebugContinueAction);

	/*mpWindowMenu = new QMenu( this );
	mpWindowMenu->setCheckable( TRUE );
	connect( mpWindowMenu, SIGNAL( aboutToShow() ),
	this, SLOT( slotWindowMenuAboutToShow() ) );
	mWindowsMenuID = menuBar()->insertItem( "&Windows", mpWindowMenu );*/

	//menuBar()->insertSeparator();
	//QMenu * help = new QMenu( this );
	//menuBar()->insertItem( "&Help", help );

	mpHelpMenu->clear();
	mpHelpMenu->addAction(mpAboutAction);
	//mpHelpMenu->insertItem( "About &Qt", this, SLOT(aboutQt()));
	//mpHelpMenu->insertSeparator();
	//mpHelpMenu->insertItem("What's &This", this, SLOT(whatsThis()), Qt::SHIFT + Qt::Key_F1);

	menuBar()->clear();
	menuBar()->addMenu(mpFileMenu);
	//mpMainMenuBar->insertItem(tr("&Edit"), mpEditMenu);
	menuBar()->addMenu(mpViewMenu);
	menuBar()->addMenu(mpAnalMenu);
	menuBar()->addMenu(mpDebugMenu);
	menuBar()->addMenu(mpWindowMenu);//, WINDOW_MENU_ID);
	menuBar()->addMenu(mpHelpMenu);

}


void ADBMainWin::CreateToolbars()
{
	mpFileTBar = addToolBar(tr("File"));//, Qt::DockTop);
	mpFileTBar->setObjectName("ToolBarFile");
	//mpFileTBar->addAction(mpFileNewAction);
	mpFileTBar->addAction(mpFileSaveAction);
	mpFileTBar->addAction(mpFileOpenAction);
	//mpFileTBar->addAction(mpFileCloseAction);
	//mpFileTBar->addAction(mpFileSaveAsAction);
	//mpFileTBar->addAction(mpFilePrintAction);
	//mpFileTBar->addAction(mpFileQuitAction);
	//(void)QWhatsThis::whatsThisButton( mpFileTBar );

	mpViewTBar = addToolBar(tr("View"));
	mpViewTBar->setObjectName("ToolBarView");
	mpViewTBar->addAction(mpShowProjectAction);
	mpViewTBar->addAction(mpShowOutputAction);
	//	mpViewTBar->addAction(mpShowDisassemblyAction);
	mpViewTBar->addAction(mpShowTasksAction);
	//mpViewTBar->addAction(mpOpenTypesDlgAction);
	mpViewTBar->addAction(mpShowCanvasAction);
	mpViewTBar->addAction(mpShowInterDataViewAction);
	mpViewTSepa = mpViewTBar->addSeparator();

	mpAnalTBar = addToolBar(tr("Analysis"));
	mpAnalTBar->setObjectName("ToolBarAnalysis");
	mpAnalTBar->addAction(mpContinueAction);
	mpAnalTBar->addAction(mpPauseAction);
	mpAnalTBar->addAction(mpStopAction);
	mpAnalTBar->addSeparator();
	mpAnalTBar->addAction(mpGoTaskTopAction);
	mpAnalTBar->addSeparator();
	mpAnalTBar->addAction(mpTraceModeAction);

	mpDebugTBar = addToolBar(tr("Debug"));
	mpDebugTBar->setObjectName("ToolBarDebug");
	mpDebugTBar->addAction(mpDebugAction);
	mpDebugTBar->addAction(mpDebugStepIntoAction);
	mpDebugTBar->addAction(mpDebugStepOverAction);
	mpDebugTBar->addAction(mpDebugStepOutAction);
	mpDebugTBar->addAction(mpDebugToggleBreakpointAction);
	mpDebugTBar->addAction(mpDebugContinueAction);
}

void ADBMainWin::dragEnterEvent(QDragEnterEvent* e)
{
	if (!mpStopAction->isEnabled())//only if idle
		e->acceptProposedAction();
}

static QChar isQuote(QChar c)
{
	if (c == QChar('\'') || c == QChar('"') || c == QChar('`'))
		return c;
	return QChar::Null;
}

static QString checkPath(QString fn0)
{
	QString fn;

	//check for double quotes
	for (int i(0); i < fn0.length(); i++)
	{
		QChar q(isQuote(fn0[i]));
		if (!q.isNull())
			fn.append(q);//double it
		fn.append(fn0[i]);
	}

	// call a function to open the files
#if QT_VERSION_MAJOR >= 6
if (QRegularExpression(QStringLiteral("\\s")).match(fn).hasMatch())
    fn = QString("'") + fn + QString("'");
#else
if (QRegExp("\\s").indexIn(fn) != -1)
    fn = QString("'") + fn + QString("'");
#endif

	return fn;
}

void ADBMainWin::dropEvent(QDropEvent* event)
{
	const QMimeData* mimeData(event->mimeData());

	// check for our needed mime type, here a file or a list of files
	if (mimeData->hasUrls())
	{
		QList<QUrl> urlList(mimeData->urls());
		// extract the local paths of the files (the very first path only)
		QString s(checkPath(urlList.front().toLocalFile()));
		QFileInfo fi(s);
		if (fi.suffix() == PRIMARY_EXT)
			s = QString("open %1").arg(fi.absoluteFilePath());
		else
			s = QString("new -a %1").arg(fi.absoluteFilePath());
		slotPostCommand(s);
	}
}

bool ADBMainWin::createMRUs()
{
//?	mpFileMenu->insertSeparator(mpFileQuitAction);

	mpRecentFilesAction = mrResMgr.createMruMenu(this,
		tr("&Recent Projects"),
		this,
		SLOT(slotRecentFile(const QString&)),
		mpFileMenu,
		mpFileQuitAction,
		10);

	mpRecentScriptsAction = mrResMgr.createMruMenu(this,
		tr("&Recent Scripts"),
		this,
		SLOT(slotRecentScript(const QString&)),
		mpFileMenu,
		mpFileQuitAction,
		10);


	return true;
}

void ADBMainWin::updateRecentProjects(const QString& s)
{
	mrResMgr.updateMru(this, tr("Recent Projects"), s);
}

void ADBMainWin::updateRecentScripts(const QString& s)
{
	mrResMgr.updateMru(this, tr("Recent Scripts"), s);
}

/*void ADBMainWin::openRecentFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)
		;// loadFile(action->data().toString());
}

void ADBMainWin::setCurrentFile(const QString &fileName)
{
	curFile = fileName;
	setWindowFilePath(curFile);

	QSettings settings;
	QStringList files = settings.value("recentFileList").toStringList();
	files.removeAll(fileName);
	files.prepend(fileName);
	while (files.size() > MaxRecentFiles)
		files.removeLast();

	settings.setValue("recentFileList", files);

	/ *foreach(QWidget *widget, QApplication::topLevelWidgets()) {
		ADBMainWin *mainWin = qobject_cast<ADBMainWin *>(widget);
		if (mainWin)
			mainWin->* /updateRecentFileActions();
	//}
}

void ADBMainWin::updateRecentFileActions()
{
	QSettings settings;
	QStringList files = settings.value("recentFileList").toStringList();

	int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

	for (int i = 0; i < numRecentFiles; ++i) {
		QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
		recentFileActs[i]->setText(text);
		recentFileActs[i]->setData(files[i]);
		recentFileActs[i]->setVisible(true);
	}
	for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
		recentFileActs[j]->setVisible(false);

	separatorAct->setVisible(numRecentFiles > 0);
}

QString ADBMainWin::strippedName(const QString &fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}*/

void ADBMainWin::slotLocusInfoChanged(QString s0, int x)
{
	bool bShown(!s0.isEmpty());
	if (bShown)
	{
		mpCursorDial->show();
		QString s;
		if (!s0.isEmpty())
			s = tr("Locus: <font color=darkRed>%1</font>").arg(s0);
		s.append(" ");
		s.append(tr("Col: <font color=darkRed>%1</font>").arg(x));
		setStatusSlotText(mpCursorDial, tr("<nobr> %1 </nobr>").arg(s));
	}
	else
		mpCursorDial->hide();

	emit signalLocusChanged(s0, x);
}

void ADBMainWin::slotTabbedWindowsToggled(bool bTabs)
{
#if(NEW_VIEWTABSMGR)
	createViewMgrTabbedEx();
#else
	ADCMainWinBase::slotTabbedWindowsToggled(bTabs);
#endif
	mrResMgr.setTabbedWindows(bTabs);
}


void ADBMainWin::showEvent(QShowEvent*)
{
	if (!mbInitialized)
	{
		mbInitialized = true;

#if(1)
		restoreGeometry(mrResMgr.value("geometry").toByteArray());
		restoreState(mrResMgr.value("windowState").toByteArray());
#else
		mrResMgr.loadWindow(this);
#endif

		uint wdc(mrResMgr.getWinDockConfig());
		uint abc(mrResMgr.getABarConfig());
		uint phc(-1);
		RestorePagesConfig(SxWinLayout(mrResMgr.getMDIWinLayout()), phc, abc, wdc);

		slotApplyFont();
		slotShowProjectView();
	}
}

void ADBMainWin::closeEvent(QCloseEvent* e)
{
#ifndef _DEBUG
	if (mProjectName.isEmpty() || QMessageBox::question(this,
		tr("Confirm exit"),
		tr("Are you sure you want to exit?"),
		tr("&Yes"), tr("&No"),
		QString(), 0, 1) == 0
		)
#endif
	{
		//mrResMgr.setABarConfig(getABarConfig());
		mrResMgr.setWinDockConfig(getWinDockConfig());
		mrResMgr.setMDIWinLayout(GetMDIWinLayout(mrResMgr.getSaveEditDocs()));

		mrResMgr.setValue("geometry", saveGeometry());
		mrResMgr.setValue("windowState", saveState());

		ADB().Shutdown();
	}

	/*	QList<QDockWidget *> dockWidgets(findChildren<QDockWidget *>());
		for (QList<QDockWidget *>::Iterator it(dockWidgets.begin()); it != dockWidgets.end(); it++)
		{
			QDockWidget *w(*it);
			QString s(w->objectName());
			settings.setValue(s+"/geometry", w->saveGeometry());
			//settings.setValue("s+/windowState", w->saveState());
		}*/

		//?	QMainWindow::closeEvent( e );
	e->ignore();//the core will take care of GUI shutting down
}

void ADBMainWin::slotShowSourceWin(QString fileName, QString atHint)
{
	showSourceWin(fileName, atHint, false);
}

void ADBMainWin::slotActivateFile(QString path)
{
}

void ADBMainWin::CreateProjectWin()
{
	if (!page(PAGE_FILES))
	{
		DocumentWin<ADCProjectWin>* pWin = new DocumentWin<ADCProjectWin>(this, "ADCPROJECTWIN");
		pWin->setPermanent(true);
		pWin->mID = PAGE_FILES;
		pWin->setStrID(QString(), tr("Files"));
		pWin->mstrIcon = ":project_view_16.png";

		connect(this, SIGNAL(signalProjectNew()), pWin, SLOT(slotProjectNew()));
		connect(this, SIGNAL(signalProjectChanged()), pWin, SLOT(slotUpdate()));
		connect(this, SIGNAL(signalFileListChanged()), pWin, SLOT(slotUpdate()));
		//connect(this, SIGNAL(signalFileListChanged(ADCStream &)), pWin, SLOT(slotFileListChanged(ADCStream &)));
		connect(this, SIGNAL(signalCurFileChanged(QString)), pWin, SLOT(slotCurFileChanged(QString)));
		connect(this, SIGNAL(signalShowCurrentFile()), pWin, SLOT(slotShowCurrentFile()));
		connect(this, SIGNAL(signalProjectAboutToClose()), pWin, SLOT(slotAboutToClose()));
		connect(this, SIGNAL(signalNewFolderRecoil(QString)), pWin, SLOT(slotNewFolderRecoil(QString)));
		connect(this, SIGNAL(signalFolderRenamed(QString)), pWin, SLOT(slotFolderRenamed(QString)));

		//connect(this, SIGNAL(signalShowDisassemblyAt(QString, int)), pWin, SLOT(slotShowBinaryAt(QString, int)));
		connect(pWin, SIGNAL(signalShowSourceWin(QString, QString)), SLOT(slotShowSourceWin(QString, QString)));
		connect(pWin, SIGNAL(signalSetTargetDirectory()), SLOT(slotSetTargetDirectory()));
		connect(pWin, SIGNAL(signalGenerateSources(QString)), SLOT(slotGenerateSources(QString)));
		connect(pWin, SIGNAL(signalDumpExports(QString)), SLOT(slotDumpExports(QString)));
		connect(pWin, SIGNAL(signalCalculateDependencies(QString)), SLOT(slotCalculateDependencies(QString)));
		connect(pWin, SIGNAL(signalNewItem(QString)), SLOT(slotNewFileOrFolder(QString)));
		connect(pWin, SIGNAL(signalDeleteItem(QString)), SLOT(slotDeleteFileOrFolder(QString)));
//?		connect(pWin, SIGNAL(signalCompileItem(QString)), SLOT(slotCompileFile(QString)));
		//connect(pWin, SIGNAL(signalFileListRequest(ADCStream &)), SLOT(slotPopulateFiles(ADCStream &)));
		connect(pWin, SIGNAL(signalRequestModel(ADCProjectWin&)), SLOT(slotRequestModel(ADCProjectWin&)));
		connect(pWin, SIGNAL(signalActivateItem(QString)), SLOT(slotActivateFile(QString)));

		RegisterPage(PAGE_FILES, pWin, nullptr, Qt::LeftDockWidgetArea);
	}
}

void ADBMainWin::CreateTasksWin()
{
	if (!page(PAGE_TASKS))
	{
		adcui::ITasksViewModel* pIModel(ADB().NewTasksViewModel());
		if (!pIModel)
			return;

		//ADCTasksWin * pWin = new ADCTasksWin( this );
		DocumentWin<ADCTasksWin>* pWin = new DocumentWin<ADCTasksWin>(this, "ADCTASKWIN");
		pWin->setPermanent(true);
		pWin->mID = PAGE_TASKS;
		pWin->setStrID(QString(), tr("Tasks"));
		pWin->mstrIcon = ":task_list_16.png";

		connect(this, SIGNAL(signalTaskListChanged()), pWin, SLOT(slotUpdate()));
		//		connect( this, SIGNAL(signalToDoListPushedBack(QString, QString)),
		//			pWin, SLOT(slotPushBack(QString, QString)) );
		//		connect( this, SIGNAL(signalToDoListPushedFront(QString, QString)),
		//			pWin, SLOT(slotPushFront(QString, QString)) );
		//		connect( this, SIGNAL(signalToDoListPoppedFront()),
		//			pWin, SLOT(slotPopFront()) );

		connect(pWin, SIGNAL(signalRequestData(ADCStream&)), SLOT(slotPopulateTasks(ADCStream&)));
		connect(pWin, SIGNAL(signalItemClicked(QString)), SLOT(slotTaskItemClicked(QString)));
		connect(pWin, SIGNAL(signalItemActivated(QString)), SLOT(slotTaskItemActivated(QString)));


		pWin->setModel(pIModel);
		pIModel->Release();
		/*
		connect( pWin, SIGNAL(signalShowSourceWin(const QString&)),
		this, SLOT(slotShowSourceWin(const QString&)) );
		*/
		//RegisterPage(pWin);
		//mWins[PAGE_TASKS] = pWin;
		RegisterPage(PAGE_TASKS, pWin, nullptr, Qt::LeftDockWidgetArea);//tabWin(PAGE_ASSBAR));
	}

	//return dynamic_cast<ADCTasksWin *>( mWins[PAGE_TASKS] );
}

void ADBMainWin::CreateOutputWin()
{
	if (!page(PAGE_OUTPUT))
	{
		//ADC_OutputWin * pWin = new ADC_OutputWin( this );
		DocumentWin<SeditexOutputWin>* pWin = new DocumentWin<SeditexOutputWin>(this, "ADCOUTPUTWIN");
		pWin->setPermanent(true);
		pWin->mID = PAGE_OUTPUT;
		pWin->setStrID(QString(), tr("Output"));
		pWin->mstrIcon = ":output_16.png";

		pWin->textEdit()->setLineWrapMode(QTextEdit::NoWrap);

		//connect( this, SIGNAL(signalAppendText(const char *)), pWin, SLOT(slotAppendText(const char *)) );

		connect(this, SIGNAL(signalToggleInputLine()), pWin, SLOT(slotToggleCommandLine()));
		connect(this, SIGNAL(signalAppendOutput(const QString&, int)), pWin, SLOT(slotAppend(const QString&, int)));
		//connect( this, SIGNAL(signalClearOutput()), pWin->textEdit(), SLOT(slotClear()) );

		connect(pWin, SIGNAL(signalCommand(const QString&, const QString&)),
			SLOT(slotCommand(const QString&, const QString&)));


		RegisterPage(PAGE_OUTPUT, pWin, nullptr, Qt::BottomDockWidgetArea);
	}

	//return dynamic_cast<ADC_OutputWin *>( mWins[PAGE_OUTPUT] );
}

void ADBMainWin::DestroyDisassemblyWin()
{
	ADCBinWin* pWin(dynamic_cast<ADCBinWin*>(page(PAGE_DASM)));
	if (pWin)
	{
		disconnect(pWin->view());
		disconnect(pWin);
		dynamic_cast<DocumentObject*>(pWin)->setPermanent(false);
		closePage(pWin);
		setPage(PAGE_DASM, 0);
	}
}

int ADBMainWin::toFileKind(QString s)//1:header, 2:source
{
	int n(0);//1:header, 2:source
	if (s.endsWith(HEADER_EXT))
		n = 1;
	else if (s.endsWith(SOURCE_EXT))
		n = 2;
	return n;
}

static const QByteArray g_cs(":\\\\");
QString ADBMainWin::fixPath(QString s)
{
	if (MODULE_SEP != g_cs)
	{
		const QByteArray cs(MODULE_SEP);
		int i(s.indexOf(cs));
		if (i > 0)
			s.replace(i, cs.length(), g_cs);
	}
	return s;
}

QString ADBMainWin::unfixPath(QString s)
{
	if (MODULE_SEP != g_cs)
	{
		int i(s.indexOf(g_cs));
		if (i > 0)
			s.replace(i, g_cs.length(), MODULE_SEP);
	}
	return s;
}



//////////////////////////////////////////

DocumentObject* ADBMainWin::CreateBinWin(QString path, QString extra)
{
	QString path2(unfixPath(path));

	adcui::IBinViewModel* pIModel(nullptr);
	ADCModelData* pModelData(findModel(path2.toUtf8(), 1));
	if (pModelData)
		pIModel = dynamic_cast<adcui::IBinViewModel*>(pModelData->pIModel);
	if (!pIModel)
	{
		pIModel = ADB().NewBinViewModel(path2.toUtf8());
		if (!pIModel)
			return nullptr;
	}
	//else
		//pIModel->AddRef();

	DocumentWin<ADCBinWin>* pWin(new DocumentWin<ADCBinWin>(this, "ADCDASMWIN"));
	pWin->setStrID(path, extra);
	//pWin->setStrID(QString(), tr("Disassembly"));
	pWin->mstrIcon = toIconStr(path2);

	connect(pWin, SIGNAL(signalSaveListing(QString)), SLOT(slotSaveListing(QString)));
	connect(pWin, SIGNAL(signalPostCommand(const QString&, bool)), SLOT(slotPostCommand(const QString&, bool)));
	connect(pWin, SIGNAL(signalLocusInfo(QString, int)), SLOT(slotLocusInfoChanged(QString, int)));
	connect(pWin, SIGNAL(signalRefreshBinaryDump(int)), SLOT(slotRefreshBinaryDump(int)));
	//connect(pWin, SIGNAL(signalContextIdInq(uint &)), SLOT(slotContextIdInq(uint &)));
	//connect(pWin, SIGNAL(signalPickModuleType(QString)), SLOT(slotPickModuleType(QString)));
	connect(pWin, SIGNAL(signalRequestModel(ADCTypeCompleter&, QString)), SLOT(slotRequestModel(ADCTypeCompleter&, QString)));
	connect(pWin, SIGNAL(signalReleaseModel(QString)), SLOT(slotReleaseModel(QString)));
	connect(pWin, SIGNAL(signalSyncRequestAtLine(int, bool)), SLOT(slotBinaryPanePicked(int, bool)));
	connect(pWin, SIGNAL(signalOpenTypeDlg()), SLOT(slotOpenTypeDlg()));

	connect(this, SIGNAL(signalProjectAboutToClose()), pWin, SLOT(slotAboutToClose()));
	connect(this, SIGNAL(signalProjectModified()), pWin, SIGNAL(signalGlobalsModified()));
	connect(this, SIGNAL(signalAnalysisStarted()), pWin, SIGNAL(signalAnalysisStarted()));
	connect(this, SIGNAL(signalSyncPanesResponce(int, bool)), pWin, SIGNAL(signalSyncPanesResponce2(int, bool)));
	connect(this, SIGNAL(signalLocusAdjusted()), pWin, SIGNAL(signalLocusAdjusted()));
	connect(this, SIGNAL(signalNameChanged()), pWin, SIGNAL(signalGlobalsModified()));
	connect(this, SIGNAL(signalResourceItemPicked()), pWin, SLOT(slotUpdateViewFromLocus()));

	//connect(pWin, SIGNAL(signalRequestModel(ADCBinWin &, QString)), SLOT(slotRequestModel(ADCBinWin &, QString)));
	//connect(this, SIGNAL(signalProjectNew()), pWin, SLOT(slotProjectNew()));
	//connect(this, SIGNAL(signalProjectChanged()), pWin, SLOT(slotReset()));
	//connect(this, SIGNAL(signalShowDisassemblyAt(QString, int)), pWin, SLOT(slotGoToLocation(QString, int)));
	//connect(this, SIGNAL(signalHighlightLocationAt(QString, QString, QString, int)), pWin, SLOT(slotHighlightLocationAt(QString, QString, QString, int)));
	//connect(pWin, SIGNAL(signalPostCommand(const QString &)), SLOT(slotPostCommand(const QString &)));
	//connect(pWin, SIGNAL(signalCallCommand(const QString &, ADCStream &)), SLOT(slotCallCommand(const QString &, ADCStream &)));

	//slotRequestModel(*pWin, path);
	//pIModel->showColumn(IBinViewModel::CLMN_DA, true);
	//?	pIModel->showColumn(IBinViewModel::CLMN_FILE, true);
	//pIModel->showColumn(IBinViewModel::CLMN_BYTES, true);

	if (pModelData)
		pWin->setModelData(pModelData);
	else
	{
		pModelData = pWin->setModel(mModels, pIModel);
		pIModel->Release();//1 left
		registerModelData(path2, pModelData);
	}
	//pWin->setModel(pIModel);
	//pIModel->Release();
	DocumentObject* pDoc(dynamic_cast<DocumentObject*>(pWin));
	pDoc->updateOutputFont(mrResMgr.getOutputFont());

	return static_cast<DocumentObject*>(pWin);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
static DocumentObject* createBinaryWinDoc(SxMainWindow* pMainWin0, QWidget*, QString& path, QString& extra)
{
	ADBMainWin* pMainWin(static_cast<ADBMainWin*>(pMainWin0));
	return pMainWin->CreateBinWin(path, extra);
}

static DocumentCreator BinaryAreaDocCreator("ADC_DASMWIN", createBinaryWinDoc);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
static DocumentObject* createTypesWinDoc(SxMainWindow* pMainWin0, QWidget*, QString& path, QString& extra)
{
	ADBMainWin* pMainWin(static_cast<ADBMainWin*>(pMainWin0));
	return pMainWin->CreateTypesView(path, extra);
}

static DocumentCreator TypesAreaDocCreator("ADC_TYPESWIN", createTypesWinDoc);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
static DocumentObject* createNamesWinDoc(SxMainWindow* pMainWin0, QWidget*, QString& path, QString& extra)
{
	ADBMainWin* pMainWin(static_cast<ADBMainWin*>(pMainWin0));
	return pMainWin->CreateNamesView(path, extra);
}

static DocumentCreator NamesAreaDocCreator("ADC_NAMESWIN", createNamesWinDoc);


//////////////////////////////////////////////////////////////////////////////////////////////////////////
static DocumentObject* createResourceWinDoc(SxMainWindow* pMainWin0, QWidget*, QString& path, QString& extra)
{
	ADBMainWin* pMainWin(static_cast<ADBMainWin*>(pMainWin0));
	return pMainWin->CreateResourceView(path, extra);
}

static DocumentCreator ResAreaDocCreator("ADC_RESWIN", createResourceWinDoc);



void ADBMainWin::CreateCanvasWin()
{
	if (!page(PAGE_CANVAS))
	{
		DocumentWin<ADCCanvasWin>* pWin = new DocumentWin<ADCCanvasWin>(this, "ADCCANVASWIN");
		pWin->setPermanent(true);
		pWin->mID = PAGE_CANVAS;
		pWin->setStrID(QString(), tr("Canvas"));
		pWin->mstrIcon = ":canvas_24.png";

		connect(this, SIGNAL(signalResourceItemPicked()), pWin, SLOT(slotUpdate()));
		connect(pWin, SIGNAL(signalUpdatePixmap(QPixmap&)), SLOT(slotGetPixmapAtCP(QPixmap&)));

		RegisterPage(PAGE_CANVAS, pWin, nullptr, Qt::LeftDockWidgetArea);
	}
}

void ADBMainWin::CreateInterDataWin()
{
	if (!page(PAGE_STRUCED))
	{
		DocumentWin<ADCInterDataWin>* pWin = new DocumentWin<ADCInterDataWin>(this, "ADCINTERDATAWIN");
		pWin->setPermanent(true);
		pWin->mID = PAGE_STRUCED;
		pWin->setStrID(QString(), tr("Inter Data"));
		pWin->mstrIcon = ":inter_data_24.png";//data_edit

		connect(this, SIGNAL(signalProjectAboutToClose()), pWin, SLOT(slotAboutToClose()));
		connect(this, SIGNAL(signalEditType(QString)), pWin, SLOT(slotReset(QString)));
		connect(this, SIGNAL(signalProjectModified()), pWin, SIGNAL(signalModified()));
		//connect(this, SIGNAL(signalBinaryDumpPicked()), pWin, SLOT(slotUpdateDataFromLocus()));
		//connect(this, SIGNAL(signalSourceDumpPicked(bool)), pWin, SLOT(slotResetFromLocus(bool)));
		connect(this, SIGNAL(signalSyncPanesResponce(int, bool)), pWin, SLOT(slotSyncPanesResponce2(int, bool)));
		connect(this, SIGNAL(signalCurInfoChanged(const ADCCurInfo &)), pWin, SLOT(slotCurInfoChanged(const ADCCurInfo &)));

		//connect(this, SIGNAL(signalResourceClicked()), pWin, SLOT(slotUpdate()));
		//connect(pWin, SIGNAL(signalUpdatePixmap(QPixmap &)), SLOT(slotGetPixmapAtCP(QPixmap &)));
		connect(pWin, SIGNAL(signalRequestModel(ADCInterDataWin&, QString)), SLOT(slotRequestModel(ADCInterDataWin&, QString)));
		connect(pWin, SIGNAL(signalRefreshBinaryDump(int)), SLOT(slotRefreshBinaryDump(int)));
		connect(pWin, SIGNAL(signalPostCommand(const QString&, bool)), SLOT(slotPostCommand(const QString&, bool)));
		connect(pWin, SIGNAL(signalLocusInfo(QString, int)), SLOT(slotLocusInfoChanged(QString, int)));

		RegisterPage(PAGE_STRUCED, pWin, nullptr, Qt::RightDockWidgetArea);
	}
}

/*void ADBMainWin::CreateTypeEdWin()
{
	if (!page(PAGE_TYPEBLDR))
	{
		DocumentWin<ADCTypesDlg>* pWin = new DocumentWin<ADCTypesDlg>(this, "ADCTYPEEDITWIN");
		pWin->setPermanent(true);
		pWin->mID = PAGE_TYPEBLDR;
		pWin->setStrID(QString(), tr("Type Builder"));
		pWin->mstrIcon = ":type_edit_24.png";

		connect(pWin, SIGNAL(signalRequestModel(ADCTypesView&, QString)), SLOT(slotRequestModel(ADCTypesView&, QString)));
		connect(this, SIGNAL(signalLocusChanged(QString, int)), pWin, SLOT(slotLocusChanged(QString, int)));
		connect(this, SIGNAL(signalProjectAboutToClose()), pWin, SLOT(depopulate()));
		connect(this, SIGNAL(signalUpdateContextTypes()), pWin, SLOT(slotUpdateContextTypes()));

		RegisterPage(PAGE_TYPEBLDR, pWin, nullptr, Qt::RightDockWidgetArea);

		SxDockWindow* pDockWin(dockWin(pWin));
		pDockWin->setFloating(true);
	}
}*/



////////////////////////////////////////// SLOTS...

void ADBMainWin::slotFindText()
{
	//SxFindTextDlg* pDlg(SxFindTextDlg::getInstance(this));
	//if (pDlg)
		//pDlg->show();
}

void ADBMainWin::slotTabifiedDockWidgetActivated(QDockWidget *)
{
}//doesn't work?!

void ADBMainWin::slotGetPixmapAtCP(QPixmap& pix)
{
	adcui::IADBCore::ResType_e eType;
	adcui::IDataSource* pIData(ADB().GetResourceData(eType));
	if (pIData)
	{
		if (eType == adcui::IADBCore::BMP)
		{
#pragma pack (push, 1)
			struct BITMAPFILEHEADER {   // bmfh
				char		bfType[2];
				uint32_t	bfSize;
				uint16_t	bfReserved1;
				uint16_t	bfReserved2;
				uint32_t	bfOffBits;
			} bmfh = { {'B', 'M'}, uint32_t(sizeof(BITMAPFILEHEADER) + pIData->size()), 0, 0, 0 };
#pragma pack (pop)
			/*typedef struct {    // bmih
				DWORD   biSize;
				LONG    biWidth;
				LONG    biHeight;
				WORD    biPlanes;
				WORD    biBitCount;
				DWORD   biCompression;
				DWORD   biSizeImage;
				LONG    biXPelsPerMeter;
				LONG    biYPelsPerMeter;
				DWORD   biClrUsed;
				DWORD   biClrImportant;
			} BITMAPINFOHEADER;*/
			QByteArray ba((const char*)& bmfh, sizeof(bmfh));
			ba.append((const char*)pIData->data(), (int)pIData->size());
			pix.loadFromData(ba);
		}
		else
			pix.loadFromData((uchar*)pIData->data(), (int)pIData->size());
		pIData->Release();
	}
}

void ADBMainWin::slotShowDisassemblyAt(QString s)
{
	s = QString("setcp %1").arg(s);
	ADB().SetCP(s.toLatin1());
	//slotPostCommand(QString("setcp %1").arg(s), false);
	slotBinaryPanePicked(666, true);
}

void ADBMainWin::slotResourceItemClicked(QString s)
{
	s = QString("setcp %1").arg(s);
	//slotPostCommand(s, false);
	if (ADB().SetCP(s.toLatin1()))
	{
		emit signalResourceItemPicked();
	}
}

void ADBMainWin::slotResourceItemActivated(QString s)
{
	const QByteArray cs(MODULE_SEP);
	int n(s.indexOf(cs));
	if (n > 0)
	{
		slotShowSourceWin(s.left(n + cs.length()), "");
		s = QString("setcp %1").arg(s);
		if (ADB().SetCP(s.toLatin1()))
		{
			emit signalResourceItemPicked();
		}
	}
}

void ADBMainWin::slotTaskItemActivated(QString s)
{
	const QByteArray cs(MODULE_SEP);
	int n(s.indexOf(cs));
	if (n > 0)
	{
		slotShowSourceWin(s.left(n + cs.length()), "");
		s = QString("setcp %1").arg(s);
		ADB().SetCP(s.toLatin1());
		//slotPostCommand(s, false);
		slotBinaryPanePicked(666, true);
	}
}

void ADBMainWin::slotTaskItemClicked(QString s)
{
	s = QString("setcp %1").arg(s);
	//slotPostCommand(s, false);
	if (ADB().SetCP(s.toLatin1()))
	{
		//emit signalSync PanesResponce(0);
		slotBinaryPanePicked(666, true);
		//emit signalResourceClicked();
	}
}

void ADBMainWin::slotTypeItemDoubleClicked(QString s)
{
	slotShowStrucEditView();
	emit signalEditType(s);
}

/////////////////////////////////////////////////TYPES
void ADBMainWin::CreateTypesWin()
{
	if (!page(PAGE_TYPES))
	{
		DocumentWin<ADCTypesWin>* pWin = new DocumentWin<ADCTypesWin>(this, "ADCTYPESWIN");
		pWin->setPermanent(true);
		pWin->mID = PAGE_TYPES;
		pWin->setStrID(QString(), tr("Types"));
		pWin->mstrIcon = ":document_types.png";

		connect(this, SIGNAL(signalOpenTypesView(QString)), pWin, SLOT(slotOpenView(QString)));

		//		connect(this, SIGNAL(signalProjectNew()), pWin, SLOT(slotProjectNew()));
		//		connect(this, SIGNAL(signalTypesMapChanged()), pWin, SLOT(slotUpdate()));
		//		connect(this, SIGNAL(signalContextIdChanged(unsigned)), pWin, SLOT(slotContextIdChanged(unsigned)));
		connect(this, SIGNAL(signalProjectAboutToClose()), pWin, SLOT(depopulate()));

		//connect(pWin, SIGNAL(signalRequestBinModel(ADCTypesWin &, QString)), SLOT(slotRequestBinModel(ADCTypesWin &, QString)));
		//connect(pWin, SIGNAL(signalRequestModel(ADCTypesWin &, QString)), SLOT(slotRequestModel(ADCTypesWin &, QString)));
		connect(pWin, SIGNAL(signalRequestModel(ADCTypesView&, QString)), SLOT(slotRequestModel(ADCTypesView&, QString)));
		//		connect(pWin, SIGNAL(signalDumpTypes(ADCStream &)), SLOT(slotDumpTypes(ADCStream &)));
		//?		connect( pWin, SIGNAL(signalLocationSelected(QString)),	SLOT(slotShowDisassemblyAt(QString)) );
		connect(pWin, SIGNAL(signalItemDoubleClicked(QString)), SLOT(slotTypeItemDoubleClicked(QString)));

		RegisterPage(PAGE_TYPES, pWin, nullptr);

		//pWin->init();
	}
}

DocumentObject* ADBMainWin::CreateTypesView(QString path, QString extra)
{
	QString sStem(path);
	sStem.truncate(sStem.lastIndexOf('.'));

	adcui::ITypesViewModel* pIModel(ADB().NewTypesViewModel(sStem.toUtf8()));
	if (!pIModel)
		return nullptr;

	DocumentWin<ADCTypesWin>* pWin(new DocumentWin<ADCTypesWin>(this, "ADCTYPESWIN"));
	pWin->setStrID(path, extra);
	pWin->mstrIcon = toIconStr(path);

	connect(this, SIGNAL(signalProjectNew()), pWin, SLOT(slotProjectNew()));
	connect(this, SIGNAL(signalTypesMapChanged()), pWin, SLOT(slotUpdate()));
	connect(this, SIGNAL(signalContextIdChanged(unsigned)), pWin, SLOT(slotContextIdChanged(unsigned)));
	connect(this, SIGNAL(signalProjectAboutToClose()), pWin, SLOT(slotAboutToClose()));

	//connect(pWin, SIGNAL(signalRequestBinModel(ADCTypesWin &, QString)), SLOT(slotRequestBinModel(ADCTypesWin &, QString)));
	connect(pWin, SIGNAL(signalRequestModel(ADCTypesWin&)), SLOT(slotRequestModel(ADCTypesWin&)));
	//connect(pWin, SIGNAL(signalDumpTypes(ADCStream&)), SLOT(slotDumpTypes(ADCStream&)));

	pWin->setModel(pIModel);
	pIModel->Release();
	//DocumentObject *pDoc(dynamic_cast<DocumentObject *>(pWin));
	//pDoc->updateOutputFont(UPREFS.getOutputFont());

	return static_cast<DocumentObject*>(pWin);
}

//////////////////////////////////////////////////////////NAMES
void ADBMainWin::CreateNamesWin()
{
	if (!page(PAGE_NAMES))
	{
		DocumentWin<ADCNamesWin>* pWin = new DocumentWin<ADCNamesWin>(this, "ADCNAMESWIN");
		pWin->setPermanent(true);
		pWin->mID = PAGE_NAMES;
		pWin->setStrID(QString(), tr("Names"));
		pWin->mstrIcon = ":document_names.png";

		connect(this, SIGNAL(signalOpenNamesView(QString)), pWin, SLOT(slotOpenView(QString)));

		connect(this, SIGNAL(signalProjectAboutToClose()), pWin, SLOT(depopulate()));

		connect(pWin, SIGNAL(signalRequestModel(ADCNamesView&, QString)), SLOT(slotRequestModel(ADCNamesView&, QString)));
		connect(pWin, SIGNAL(signalItemDoubleClicked(QString)), SLOT(slotTypeItemDoubleClicked(QString)));

		RegisterPage(PAGE_NAMES, pWin, nullptr, Qt::BottomDockWidgetArea);

		//pWin->init();
	}
}

//////////////////////////////////////////////////////////EXPORTS
void ADBMainWin::CreateExportsWin()
{
	if (!page(PAGE_EXPORTS))
	{
		DocumentWin<ADCExportsWin>* pWin = new DocumentWin<ADCExportsWin>(this, "ADCEXPORTSWIN");
		pWin->setPermanent(true);
		pWin->mID = PAGE_EXPORTS;
		pWin->setStrID(QString(), tr("Exports"));
		pWin->mstrIcon = ":doc_export_24.png";

		connect(this, SIGNAL(signalOpenExportsView(QString)), pWin, SLOT(slotOpenView(QString)));

		connect(this, SIGNAL(signalProjectAboutToClose()), pWin, SLOT(depopulate()));

		connect(pWin, SIGNAL(signalRequestModel(ADCExportsView&, QString)), SLOT(slotRequestModel(ADCExportsView&, QString)));
		connect(pWin, SIGNAL(signalItemDoubleClicked(QString)), SLOT(slotTypeItemDoubleClicked(QString)));

		RegisterPage(PAGE_EXPORTS, pWin, nullptr, Qt::BottomDockWidgetArea);

		//pWin->init();
	}
}


//////////////////////////////////////////////////////////IMPORTS
void ADBMainWin::CreateImportsWin()
{
	if (!page(PAGE_IMPORTS))
	{
		DocumentWin<ADCImportsWin>* pWin = new DocumentWin<ADCExportsWin>(this, "ADCIMPORTSWIN");
		pWin->setImportView();
		pWin->setPermanent(true);
		pWin->mID = PAGE_IMPORTS;
		pWin->setStrID(QString(), tr("Imports"));
		pWin->mstrIcon = ":doc_import_24.png";

		connect(this, SIGNAL(signalOpenImportsView(QString)), pWin, SLOT(slotOpenView(QString)));

		connect(this, SIGNAL(signalProjectAboutToClose()), pWin, SLOT(depopulate()));

		connect(pWin, SIGNAL(signalRequestModel(ADCExportsView&, QString)), SLOT(slotRequestModel(ADCExportsView&, QString)));
		connect(pWin, SIGNAL(signalItemDoubleClicked(QString)), SLOT(slotTypeItemDoubleClicked(QString)));

		RegisterPage(PAGE_IMPORTS, pWin, nullptr, Qt::BottomDockWidgetArea);

		//pWin->init();
	}
}

DocumentObject* ADBMainWin::CreateNamesView(QString path, QString extra)
{
	QString sStem(path);
	sStem.truncate(sStem.lastIndexOf('.'));

	adcui::INamesViewModel* pIModel(ADB().NewNamesViewModel(sStem.toUtf8()));
	if (!pIModel)
		return nullptr;

	DocumentWin<ADCNamesWin>* pWin(new DocumentWin<ADCNamesWin>(this, "ADCNAMESWIN"));
	pWin->setStrID(path, extra);
	pWin->mstrIcon = toIconStr(path);

	connect(this, SIGNAL(signalProjectNew()), pWin, SLOT(slotProjectNew()));
	connect(this, SIGNAL(signalTypesMapChanged()), pWin, SLOT(slotUpdate()));
	connect(this, SIGNAL(signalContextIdChanged(unsigned)), pWin, SLOT(slotContextIdChanged(unsigned)));
	connect(this, SIGNAL(signalProjectAboutToClose()), pWin, SLOT(slotAboutToClose()));

	connect(pWin, SIGNAL(signalRequestModel(ADCNamesWin&)), SLOT(slotRequestModel(ADCNamesWin&)));
	//connect(pWin, SIGNAL(signalDumpTypes(ADCStream&)), SLOT(slotDumpTypes(ADCStream&)));

	pWin->setModel(pIModel);
	pIModel->Release();

	return static_cast<DocumentObject*>(pWin);
}

void ADBMainWin::slotPickModuleType(QString sModule)
{
	/*if (!mpTypesDlg)
	{
		mpTypesDlg = new ADCTypesDlg(this);
		connect(mpTypesDlg, SIGNAL(signalRequestModel(ADCTypesView &, QString)), SLOT(slotRequestModel(ADCTypesView &, QString)));
		mpTypesDlg->setFloating(true);
	}

	mpTypesDlg->show();
	*/

	slotOpenTypeDlg();

	//mpTypesDlg->raise();
	//mpTypesDlg->activateWindow();

	//dlg.setField(sObj);
	//dlg.setContents(l);
	//dlg.setArray(iArray);
	//if (mpTypesDlg->exec() == QDialog::Accepted)
	{
		//QString tname(mpTypesDlg->getSelected());
		//int n(dlg.array());
		//if (n > 0)
			//tname = QString("%1[%2]").arg(tname).arg(n);
		//emit signalPostCommand(QString("makeobj %1").arg(tname));
	}
}

void ADBMainWin::CreateResourceWin()
{
	if (!page(PAGE_RESOURCES))
	{
		DocumentWin<ADCResWin>* pWin = new DocumentWin<ADCResWin>(this, "ADCRESWIN");
		pWin->setPermanent(true);
		pWin->mID = PAGE_RESOURCES;
		pWin->setStrID(QString(), tr("Resources"));
		pWin->mstrIcon = ":resources.png";

		connect(this, SIGNAL(signalOpenResView(QString)), pWin, SLOT(slotOpenView(QString)));
		connect(this, SIGNAL(signalProjectAboutToClose()), pWin, SLOT(depopulate()));

		connect(pWin, SIGNAL(signalRequestModel(ADCResView&, QString)), SLOT(slotRequestModel(ADCResView&, QString)));
		connect(pWin, SIGNAL(signalItemDoubleClicked(QString)), SLOT(slotResourceItemActivated(QString)));
		connect(pWin, SIGNAL(signalItemClicked(QString)), SLOT(slotResourceItemClicked(QString)));
		connect(pWin, SIGNAL(signalShowCanvas()), SLOT(slotShowCanvasWin()));

		/*		connect(pWin, SIGNAL(signalDumpResources(ADCStream &)), SLOT(slotDumpResources(ADCStream &)));
				connect(pWin, SIGNAL(signalPostCommand(const QString &, bool)), SLOT(slotPostCommand(const QString &, bool)));

				connect(this, SIGNAL(signalProjectNew()), pWin, SLOT(slotProjectNew()));
				//connect(this, SIGNAL(signalProjectChanged()), pWin, SLOT(slotReset()));
				connect(this, SIGNAL(signalPreanalized()), pWin, SLOT(slotReset()));*/

		RegisterPage(PAGE_RESOURCES, pWin, nullptr, Qt::LeftDockWidgetArea);
	}
}

DocumentObject* ADBMainWin::CreateResourceView(QString path, QString extra)//single tab mode
{
	QString sStem(path);
	sStem.truncate(sStem.lastIndexOf('.'));

	adcui::IResViewModel* pIModel(ADB().NewResViewModel(sStem.toUtf8()));
	if (!pIModel)
		return nullptr;

	DocumentWin<ADCResWin>* pWin = new DocumentWin<ADCResWin>(this, "ADC_RESWIN");
	pWin->setStrID(QString(), tr("Resources"));
	pWin->mstrIcon = ":resources.png";

	/*	connect(pWin, SIGNAL(signalDumpResources(ADCStream &)), SLOT(slotDumpResources(ADCStream &)));
		connect(pWin, SIGNAL(signalPostCommand(const QString &, bool)), SLOT(slotPostCommand(const QString &, bool)));
		connect(pWin, SIGNAL(signalItemDoubleClicked(QString)), SLOT(slotResourceItemActivated(QString)));
		connect(pWin, SIGNAL(signalItemClicked(QString)), SLOT(slotResourceItemClicked(QString)));
		connect(pWin, SIGNAL(signalRequestModel(ADCResWin &)), SLOT(slotRequestModel(ADCResWin &)));
		connect(pWin, SIGNAL(signalShowCanvas()), SLOT(slotShowCanvasWin()));

		connect(this, SIGNAL(signalProjectNew()), pWin, SLOT(slotProjectNew()));
		connect(this, SIGNAL(signalProjectAboutToClose()), pWin, SLOT(slotAboutToClose()));
		//connect(this, SIGNAL(signalProjectChanged()), pWin, SLOT(slotReset()));
		connect(this, SIGNAL(signalPreanalized()), pWin, SLOT(slotReset()));*/

	pWin->setModel(pIModel);
	pIModel->Release();

	return static_cast<DocumentObject*>(pWin);
}

void ADBMainWin::slotBinaryPanePicked(int, bool)
{
	//emit signalBinaryDumpPicked();
}

void ADBMainWin::registerModelData(QString path, ADCModelData* pModelData)
{
	//pat must be 'unfixed', e.g. as it appears in core
	Q_ASSERT(!path.isEmpty());
	mModels.insert(path, pModelData);
}

void ADBMainWin::slotReleaseModelData(ADCModelData*)
{
}

ADCModelData* ADBMainWin::findModel(QString file, int type)//-1:src_unfold, 0:src, 1:bin
{
	QMap<QString, ADCModelData*>::iterator i(mModels.find(file));
	for (; i != mModels.end() && i.key() == file; i++)
	{
		if (type == 1)//BIN
		{
			adcui::IBinViewModel* pIModel(dynamic_cast<adcui::IBinViewModel*>(i.value()->pIModel));
			if (pIModel)
				return i.value();
		}
		else
		{
			adcui::ISrcViewModel* pIModel(dynamic_cast<adcui::ISrcViewModel*>(i.value()->pIModel));
			if (pIModel)
			{
				bool bUnfold(pIModel->mode().testL(adcui::DUMP_UNFOLD));
				if (type == -1)//UNFOLD
				{
					if (bUnfold)
						return i.value();
				}
				else//SRC
				{
					if (!bUnfold)
						return i.value();
				}
			}
		}
	}
	return nullptr;
}

void ADBMainWin::slotPageAboutToClose(QWidget* pWin0)
{
	/*ADCSourceWin0 *pWin(dynamic_cast<ADCSourceWin0 *>(pWin0));
	if (pWin)
	{
		DocumentObject &rDoc(*dynamic_cast<DocumentObject *>(pWin));
		QString path(KEY2PATH(rDoc.mstrID));
		QMultiMap<QString, ADCModelData *>::iterator i(mModels.find(path));
		for (; i != mModels.end() && i.key() == path; i++)
			delete i.value();
		mModels.remove(path);
	}
	else*/
	pWin0->close();
}

void ADBMainWin::slotReleaseModel(QString path)
{
	/*	QMap<QString, ADCModelData *>::iterator i(mModels.find(path));
		if (i != mModels.end())
		{
			ADCModelData *pModelData(i.value());
			adcui::IADCTextModel *pIModel(pModelData->pIModel);
			if (pIModel->RefsNum() == 1)
			{
				pModelData->Release();
				mModels.remove(path);
			}
			else
				pIModel->Release();
		}*/
}

/*void ADBMainWin::slotRequestModel(ADCBinWin &rWin, QString fileName)
{
	IBinViewModel *pIModel(ADB().NewBinViewModel(fileName.toUtf8()));
	if (pIModel)
	{
		//pIModel->showColumn(IBinViewModel::CLMN_DA, true);
//?		pIModel->showColumn(IBinViewModel::CLMN_FILE, true);
		//pIModel->showColumn(IBinViewModel::CLMN_BYTES, true);
		rWin.setModel(pIModel);
		pIModel->Release();
		DocumentObject &rDoc(dynamic_cast<DocumentObject &>(rWin));
		rDoc.updateOutputFont(UPREFS.getOutputFont());
	}
}*/

void ADBMainWin::slotRequestModel(ADCTypesWin& rWin, QString path)
{
	QString sStem(path);
	sStem.truncate(sStem.lastIndexOf('.'));

	adcui::ITypesViewModel* pIModel(ADB().NewTypesViewModel(sStem.toUtf8()));
	if (pIModel)
	{
		rWin.setModel(pIModel);
		pIModel->Release();
	}
}

void ADBMainWin::slotRequestModel(ADCTypesView& rView, QString path)
{
	QString sStem(path);
	if (!sStem.isEmpty())
		sStem.truncate(sStem.lastIndexOf('.'));

	adcui::ITypesViewModel* pIModel(ADB().NewTypesViewModel(sStem.toUtf8()));
	if (pIModel)
	{
		rView.setModel(pIModel);
		pIModel->Release();
	}
}

void ADBMainWin::slotRequestModel(ADCTypesDlg& rDlg, QString path)
{
	QString sStem(path);
	if (!sStem.isEmpty())
		sStem.truncate(sStem.lastIndexOf('.'));

	adcui::ITypesViewModel* pIModel(ADB().NewTypesViewModel(sStem.toUtf8()));
	if (pIModel)
	{
		rDlg.setModel(pIModel);
		pIModel->Release();
	}
}

void ADBMainWin::slotRequestModel(ADCNamesWin& rWin, QString path)
{
	QString sStem(path);
	sStem.truncate(sStem.lastIndexOf('.'));

	adcui::INamesViewModel* pIModel(ADB().NewNamesViewModel(sStem.toUtf8()));
	if (pIModel)
	{
		rWin.setModel(pIModel);
		pIModel->Release();
	}
}

void ADBMainWin::slotRequestModel(ADCNamesView& rView, QString path)
{
	QString sStem(path);
	if (!sStem.isEmpty())
		sStem.truncate(sStem.lastIndexOf('.'));

	adcui::INamesViewModel* pIModel(ADB().NewNamesViewModel(sStem.toUtf8()));
	if (pIModel)
	{
		rView.setModel(pIModel);
		pIModel->Release();
	}
}

void ADBMainWin::slotRequestModel(ADCExportsWin& rWin, QString path)
{
	QString sStem(path);
	sStem.truncate(sStem.lastIndexOf('.'));
	adcui::IExportsViewModel* pIModel(ADB().NewExportsViewModel(sStem.toUtf8()));
	if (pIModel)
	{
		rWin.setModel(pIModel);
		pIModel->Release();
	}
}

void ADBMainWin::slotRequestModel(ADCExportsView& rView, QString path)
{
	QString sStem(path);
	if (!sStem.isEmpty())
		sStem.truncate(sStem.lastIndexOf('.'));
	adcui::IExportsViewModel* pIModel;
	if (dynamic_cast<ADCImportsView *>(&rView))
		pIModel = ADB().NewImportsViewModel(sStem.toUtf8());
	else
		pIModel = ADB().NewExportsViewModel(sStem.toUtf8());
	if (pIModel)
	{
		rView.setModel(pIModel);
		pIModel->Release();
	}
}

void ADBMainWin::slotRequestModel(ADCTypeCompleter& rWin, QString path)
{
	QString sStem(path);
	if (!sStem.isEmpty())
		sStem.truncate(sStem.lastIndexOf('.'));

	adcui::ITypesViewModel* pIModel(ADB().NewTypesViewModel(sStem.toUtf8()));
	if (pIModel)
	{
		rWin.setModel(pIModel);
		pIModel->Release();
	}
}

void ADBMainWin::slotRequestModel(ADCProjectWin& rWin)
{
	adcui::IFilesViewModel* pIModel(ADB().NewFilesViewModel());
	if (pIModel)
	{
		rWin.setModel(pIModel);
		pIModel->Release();
	}
}

void ADBMainWin::slotRequestModel(ADCResWin& rWin)
{
	adcui::IResViewModel* pIModel(ADB().NewResViewModel(nullptr));
	if (pIModel)
	{
		rWin.setModel(pIModel);
		pIModel->Release();
	}
}

void ADBMainWin::slotRequestModel(ADCResView& rView, QString path)
{
	QString sStem(path);
	sStem.truncate(sStem.lastIndexOf('.'));

	adcui::IResViewModel* pIModel(ADB().NewResViewModel(sStem.toUtf8()));
	if (pIModel)
	{
		rView.setModel(pIModel);
		pIModel->Release();
	}
}

void ADBMainWin::slotRequestModel(ADCInterDataWin& rWin, QString s)
{
	adcui::IBinViewModel* pIModel(ADB().NewScopeViewModel(s.toLatin1()));
	if (pIModel)//do not invalidate the view if got no model
	{
		rWin.setModel(pIModel);
		pIModel->Release();
	}
}

/*void ADBMainWin::slotRequestBinModel(ADCTypesWin &rWin, QString s)
{
	IBinViewModel *pIModel(ADB().NewScopeViewModel(s.toLatin1()));
	if (pIModel)
	{
		rWin.setBinModel(pIModel);
		pIModel->Release();
	}
}*/

void ADBMainWin::slotNewFileOrFolder(QString s)
{
	slotPostCommand(QString("newsrc %1").arg(s));
}

void ADBMainWin::slotDeleteFileOrFolder(QString s)
{
	slotPostCommand(QString("delfile %1").arg(s));
}

void ADBMainWin::slotDebug()
{
	ADB().PostCommand("debug", false);
}

void ADBMainWin::slotDebugStepInto()
{
	ADB().PostCommand("step", false);
}

void ADBMainWin::slotDebugStepOver()
{
	ADB().PostCommand("next", false);
}

void ADBMainWin::slotDebugStepOut()
{
	ADB().PostCommand("out", false);
}

void ADBMainWin::slotDebugToggleBreakpoint()
{
	ADB().PostCommand("bp", false);
}

void ADBMainWin::slotDebugContinue()
{
	ADB().PostCommand("cont", false);
}

void ADBMainWin::enableDebuggerActions(int iStatus)//0:inactive, 1:running, 2:paused
{
	bool bRunning(iStatus > 0);
	bool bPaused(iStatus > 1);
	mpDebugAction->setEnabled(!bRunning);
	mpDebugStepIntoAction->setEnabled(bPaused);
	mpDebugStepOverAction->setEnabled(bPaused);
	mpDebugStepOutAction->setEnabled(bPaused);
	mpDebugToggleBreakpointAction->setEnabled(bRunning);//?always?
	mpDebugContinueAction->setEnabled(bPaused);
}

/*void ADBMainWin::slotShowFontsDlg()
{
#if QT_VERSION >= 0x040500//4.5
	bool ok;
	QFont font = QFontDialog::getFont(&ok, UPREFS.getOutputFont(), this, QString("Choose a Font"), QFontDialog::MonospacedFonts | QFontDialog::DontUseNativeDialog);
	if (ok)
		slotFontApplied(font);
#else
#if(1)
	QFontDialog dlg(UPREFS.getOutputFont(), this);
#else//doesn't work
	QFontDialog::FontDialogOptions opt = dlg.options();
	dlg.setOptions(QFontDialog::ProportionalFonts | QFontDialog::MonospacedFonts | QFontDialog::DontUseNativeDialog | QFontDialog::NoButtons);
	opt = dlg.options();
#endif
	if (dlg.exec() == QDialog::Accepted)
		slotFontApplied(dlg.selectedFont());
#endif
}*/

void ADBMainWin::slotShowOptionsDlg()
{
	if (!mpConfigDlg)
		mpConfigDlg = new ADCConfigDlg(mrResMgr, this);

	connect(mpConfigDlg, SIGNAL(signalPostApply()), SLOT(slotPrefsApplied()));

    mpConfigDlg->show();
}

void ADBMainWin::applyCoreSettings()
{
	ADCStream ss;
	ss.WriteStringf("CallDepth=%d", mrResMgr.getCallDepth());
	//ss.WriteStringf("String=%s", sString.toLatin1().constData());
	//ss.WriteStringf("Bool=%d", bBool ? 1 : 0);
	ADB().ApplySettings(ss);
}

void ADBMainWin::slotPrefsApplied()
{
	slotApplyFont();

	applyCoreSettings();
}

/*void ADBMainWin::slotFontApplied(QFont ff)
{
	mrResMgr.setOutputFont(ff);
	slotApplyFont();
}*/

void ADBMainWin::slotApplyFont()
{
	QFont ff(mrResMgr.getOutputFont());
	QList<DocumentObject*>::Iterator it;
	for (it = DocMgr().begin(); it != DocMgr().end(); it++)
	{
		DocumentObject* pDoc(*it);
		pDoc->updateOutputFont(ff);
	}
}

/*void ADBMainWin::slotCloseDisplay(int fileId)
{
	//ADB().RequestCloseDisplay(fileId);
}*/

/*void ADBMainWin::CreateDecompyleWin()
{
	if (!page(PAGE_DC))
	{
		DocumentWin<ADCSourceView> *pWin = new DocumentWin<ADCSourceView>(this, "ADCDCWIN");
		pWin->setPermanent(true);
		pWin->mID = PAGE_DC;
		pWin->setStrID( QString(), tr("Decompyle") );
		pWin->mstrIcon = ".16.png";

		RegisterPage(PAGE_DC, pWin, nullptr);//tabWin(PAGE_BOTTOMBAR));
	}
}*/

void ADBMainWin::slotDumpResources(ADCStream& ss)
{
	ADB().DumpResources(ss);
}

/*void ADBMainWin::slotDumpTypes(ADCStream& ss)
{
	ADB().DumpTypes(ss);
}*/

/*void ADBMainWin::slotDumpExpr(ADCStream& ss, bool bPtrDump)
{
	ADB().DumpExpr(ss, bPtrDump ? (adcui::IADCExprViewModel::DUMPEXPR_PTRS) : adcui::IADCExprViewModel::DUMPEXPR_NULL);
}*/

/*void ADBMainWin::slotDumpPtrExprList(ADCStream& ss)
{
	ADB().GetPtrExprList(ss);
}*/

void ADBMainWin::slotPostCommand(const QString& cmd, bool bEcho)
{
	ADB().PostCommand(cmd.toUtf8().data(), bEcho);
}

void ADBMainWin::slotRefreshBinaryDump(int dumpId)
{
	ADB().RefreshBinaryDump(dumpId, true);
}

void ADBMainWin::slotCommand(const QString&, const QString& cmd)
{
	ADB().PostCommand(cmd.toUtf8().data(), true);
}

void ADBMainWin::slotCallCommand(const QString& cmd, ADCStream& resp)
{
	//emit signalCallCommand(cmd, resp);
	ADB().CallCommand(cmd.toUtf8().data(), resp, false);
}

void ADBMainWin::slotDecompile()
{
	//ADB().PostCommand("decompile -@%FUNCVA%", true);
	ADB().PostCommand("dc", true);
}

void ADBMainWin::slotStart()
{
	setFocus();
	ADB().Start();
}

/*void ADBMainWin::slotPause()
{
	ADB().Pause();
}*/

void ADBMainWin::slotStop()
{
	ADB().Stop();
}

void ADBMainWin::slotGoTaskTop()
{
	ADB().PostCommand("show $task", false);
	/*int fileId(ADB().GetFileIdByName("$"));
	if (fileId >= 0)
	{
		ADCStream ss;
		if (ADB().GetFileNameById(fileId, ss))
		{
			QString sFileName(ss.ReadString());
			slotShowSourceWin(sFileName);
			emit signalHighlightLocationAt(QString(), QString(), QString(), fileId);
		}
	}*/
}

void ADBMainWin::slotTraceModeToggled(bool b)
{
	ADB().SetDebugMode(b);
}


adcui::FolderTypeEnum ADBMainWin::CheckSpecialFile(QString s0)
{
	using namespace adcui;
	if (s0.endsWith(RESOURCE_EXT))
		return FOLDERTYPE_FILE_RC;
	if (s0.endsWith(TYPES_EXT))
		return FOLDERTYPE_FILE_T;
	if (s0.endsWith(NAMES_EXT))
		return FOLDERTYPE_FILE_N;
	if (s0.endsWith(EXPORTS_EXT))
		return FOLDERTYPE_FILE_E;
	if (s0.endsWith(IMPORTS_EXT))
		return FOLDERTYPE_FILE_I;
	if (s0.endsWith(TEMPLATES_EXT))
		return FOLDERTYPE_FILE_TT;
	if (s0.endsWith(STUBS_EXT))
		return FOLDERTYPE_FILE_STUB;


	/*if (s0.endsWith('>'))
	{
		int n(s0.lastIndexOf(".<"));
		Q_ASSERT(n > 1);
		QString s(s0.mid(n));
CHECK_QSTRING(s, z);
		if (s == RESOURCE_EXT)
			return FOLDERTYPE_FILE_RC;
		if (s == TYPES_EXT)
			return FOLDERTYPE_FILE_T;
		Q_ASSERT(0);
	}*/
	return FOLDERTYPE_UNK;
}

void ADBMainWin::showSourceWin(QString fileName, QString atHint, bool bAskToOpen)
{
	using namespace adcui;
	const char* cl_name(nullptr);
	switch (CheckSpecialFile(fileName))
	{
	case FOLDERTYPE_FILE_RC:
		slotShowResourcesView();
		emit signalOpenResView(fileName);
		return;
	case FOLDERTYPE_FILE_T:
		slotShowTypesView();
		emit signalOpenTypesView(fileName);
		return;
	case FOLDERTYPE_FILE_N:
		slotShowNamesView();
		emit signalOpenNamesView(fileName);
		return;
	case FOLDERTYPE_FILE_E:
		slotShowExportsView();
		emit signalOpenExportsView(fileName);
		return;
	case FOLDERTYPE_FILE_I:
		slotShowImportsView();
		emit signalOpenImportsView(fileName);
		return;
	default:
		break;
	}

	DocumentObject* pDoc(DocMgr().findDoc(fixPath(fileName), QString()));
	if (!pDoc)
	{
		const QByteArray cs(MODULE_SEP);
		if (bAskToOpen)
		{
			int n(fileName.indexOf(cs));
			if (QMessageBox::question(this, tr("Open File"), tr("Would you like to open module: %1?").arg(fileName.left(n)),
				QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes)
				return;
		}

		if (!cl_name)
		{
			if (fileName.endsWith(cs))
				cl_name = "ADC_DASMWIN";
			else
				cl_name = "ADC_SOURCEWIN";//?
		}
		pDoc = SxMainWindow::openDoc(cl_name, fixPath(fileName), QString());
		if (!pDoc)
			return;
		//pDoc->updateOutputFont(UPREFS.getOutputFont());
	}
	else
	{
		QWidget* pWin(dynamic_cast<QWidget*>(pDoc));
		showWin(pWin);
	}

	if (!atHint.isEmpty())
		OnShowDocumentAtHint(pDoc, atHint);
}


void ADBMainWin::OnShowDocumentAtHint(DocumentObject* pDoc, QString atHint)
{
	ADCBinWin* pWin(dynamic_cast<ADCBinWin*>(pDoc));
	if (pWin)
		pWin->locatePosition(atHint);
}

/*void ADBMainWin::setCurrentObject(QWidget * pWidget)
{

SxMainWindow::setCurrentObject(pWidget);
//	if ( mbClosingDown )
//		return;

*//*	DocumentObjectBase * pDoc = dynamic_cast< DocumentObjectBase * >(pWidget);
if (!pDoc)
pWidget = nullptr;

if (mpSignalMultiplexer)
if (mpSignalMultiplexer->setCurrentObject( pWidget ) == -1)
return;//no change

mpCurPos->setShown(false);

if (pDoc)
pDoc->emitAllSignals();
*/
//}

/*void ADBMainWin::OnFileListChanged(ADCStream &ss)
{
/ *	QMap<QString, int> m;
	QString s;
	while (ss.ReadString(s))
		m.insert(s, 0);

	//close removed documents
	QPtrList<DocumentObject> l2Close;
	for (int i(0); i < getDocsCount(); i++)
	{
		DocumentObject *pDoc(getDoc(i));
		ADCSourceView *pWin(dynamic_cast<ADCSourceView *>(pDoc));
		if (pWin)
		{
			QString fileName(pWin->fileName());
			if (m.contains(fileName))
			//Fil e_t *pFile = pWin->getFile();
			//if (!MAIN.IsMineFile(pFile))
				l2Close.append(pDoc);
		}
	}

	while (!l2Close.isEmpty())
	{
		DocumentObject * pDoc(l2Close.getFirst());
		l2Close.removeFirst();
		QWidget *pWin(dynamic_cast<QWidget *>(pDoc));
		closePage(pWin);//removeDoc(pDoc);
	}

	ss.Rewind(0);* /
	emit signalFileListChanged();
}*/


//////////////////////////////////////////// QFileDialog
class ADCOpenFileDialog : public QFileDialog
{
	QComboBox* mpcbOptions;
	QString msOptions;
public:
	ADCOpenFileDialog(QWidget *parent,
		const QString &caption = QString(),
		const QString &directory = QString(),
		const QString &filter = QString())
		: QFileDialog(parent, caption, directory, filter)
	{
		setOption(DontUseNativeDialog, true); //we need qt layout
		setAcceptMode(AcceptOpen);

		mpcbOptions = new QComboBox(this);
		mpcbOptions->setEditable(true);

		//fix layout
		QGridLayout *l(static_cast<QGridLayout*>(layout()));
		int row(l->rowCount());

		QLayoutItem* c = l->itemAtPosition(row - 2, 1);//file name

		QLayoutItem* a = l->takeAt(l->indexOf(l->itemAtPosition(row - 1, 0)));//label
		QLayoutItem* b = l->takeAt(l->indexOf(l->itemAtPosition(row - 1, 1)));//file types

		l->addWidget(new QLabel(tr("Options:"), this), row - 1, 0, 1, 1);
		l->addWidget(mpcbOptions, row - 1, 1, 1, 1);

		l->addWidget(a->widget(), row, 0, 1, 1);
		l->addWidget(b->widget(), row, 1, 1, 1);

		setTabOrder(c->widget(), mpcbOptions);
		setTabOrder(mpcbOptions, b->widget());
	}
	QString selectedFile() const {
		if (selectedFiles().empty())
			return QString();
		return selectedFiles().value(0);
	}
	QString options() const {
		return msOptions;
	}
protected:
	virtual void accept()
	{
		QFileDialog::accept();
		msOptions = mpcbOptions->currentText().trimmed();
		if (!msOptions.isEmpty())
		{
			mpcbOptions->insertItem(0, msOptions);
			mpcbOptions->clearEditText();
		}
	}
};

ADCOpenFileDialog* ADBMainWin::openModuleDialog(QString title)
{
	if (!mpOpenModuleDlg)
		mpOpenModuleDlg = new ADCOpenFileDialog(this,
			tr("Open"),
			QFileInfo(mLastModule).dir().path(),
#ifdef _DEBUG
			tr("Supported Files (*.exe *.dll *.ico *.class);;") +
#endif
			tr("Executable Files (*.exe *.dll);;") +
			tr("Media Files (*.ico);;") +
			tr("Java Files (*.class);;") +
			tr("All Files (*)")
		);

	mpOpenModuleDlg->setWindowTitle(title);
	return mpOpenModuleDlg;
}

void ADBMainWin::slotNew()
{
	ADCOpenFileDialog* pDlg(openModuleDialog(tr("New Project")));
	//pDlg->setDirectory(QFileInfo(mLastModule).dir().path());

	QString fn;
	if (pDlg->exec() == QDialog::Accepted)
		fn = pDlg->selectedFile();

	if (fn.isEmpty())
	{
		statusBar()->showMessage(tr("Loading aborted"), 2000);
		return;
	}

	mLastModule = fn;

	QString s("new");
	if (!pDlg->options().isEmpty())
		s.append(" " + pDlg->options());
	s.append(" " + checkPath(fn));

	slotPostCommand(s);
}

void ADBMainWin::slotAddModule()
{
	ADCOpenFileDialog* pDlg(openModuleDialog(tr("Add Module")));
	//pDlg->setDirectory(QFileInfo(mLastModule).dir().path());

	QString fn;
	if (pDlg->exec() == QDialog::Accepted)
		fn = pDlg->selectedFile();

	if (fn.isEmpty())
	{
		statusBar()->showMessage(tr("Loading aborted"), 2000);
		return;
	}

	mLastModule = fn;

	QString s("new -a");
	if (!pDlg->options().isEmpty())
		s.append(" " + pDlg->options());
	s.append(" " + checkPath(fn));

	slotPostCommand(s);
}

void ADBMainWin::slotNewScript()
{
	ADCOpenFileDialog dialog(this,
		tr("Run Script File"), 
		QFileInfo(mLastScript).dir().path(),
		tr("Scripts Files (*.s);;") +
		tr("All Files (*)")
		);

	QString fn;
	if (dialog.exec() == QDialog::Accepted)
		fn = dialog.selectedFile();

	if (fn.isEmpty())
	{
		statusBar()->showMessage(tr("Loading aborted"), 2000);
		return;
	}

	mLastScript = fn;
	updateRecentScripts(fn);

	slotPostCommand(QString("script %1").arg(fn));
}

void ADBMainWin::slotOpen()
{
	ADCOpenFileDialog dialog(this,
		tr("Open Project"), 
		QFileInfo(mLastProject).dir().path(),//dir
		tr("Projects (*." PRIMARY_EXT ");;") + tr("All Files (*)"));

	QString fn;
	if (dialog.exec() == QDialog::Accepted)
		fn = dialog.selectedFile();

	if (!fn.isEmpty())
	{
		slotPostCommand(QString("open %1").arg(checkPath(fn)));
		return;
	}

	mLastProject = fn;
	statusBar()->showMessage(tr("Loading aborted"), 2000);
}

void ADBMainWin::slotRecentFile(const QString& fn)
{
	if (!fn.isEmpty())
		slotPostCommand(QString("open %1").arg(fn));
}

void ADBMainWin::slotRecentScript(const QString& fn)
{
	if (!fn.isEmpty())
		slotPostCommand(QString("script %1").arg(fn));
}

void ADBMainWin::slotSaveListing(QString command)
{
	QString fn = QFileDialog::getSaveFileName(this,
		tr("Save Listing"),
		QString(),//dir
		QString("Projects (*.lst);;") + QString("All Files (*)"),
		nullptr
#if(0)
		, QFileDialog::DontConfirmOverwrite
#endif
	);

	if (fn.isEmpty())
	{
		statusBar()->showMessage(tr("Dumping aborted"), 2000);
		return;
	}

	QFileInfo fi(fn);
	if (fi.suffix().isEmpty())
		fi = QFileInfo(fi.absoluteFilePath() + ".lst");
#if(0)
	if (fi.exists() && QMessageBox::question(this,
		tr("Overwrite File?"),
		tr("A file '%1' already exists.\nDo you want to overwrite it?").arg(fi.absoluteFilePath()),
		tr("&Yes"), tr("&No"),
		QString(), 0, 1))
		return;
#endif

	command.append(QString(" > %1").arg(fi.absoluteFilePath()));

	slotPostCommand(command);
}

void ADBMainWin::slotCloseProject()
{
    auto reply = QMessageBox::question(
        this,
        tr("Close Project?"),
        tr("Are you sure you want to close the project?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply == QMessageBox::No)
        return;

    slotPostCommand("close");
}


void ADBMainWin::slotSave()
{
	slotPostCommand(QString("save"));
}

#include <QCheckBox>

class ADCSaveProjectDialog : public QFileDialog
{
	QCheckBox *mpDisperseModeCBx;
public:
	ADCSaveProjectDialog(QWidget *parent,
		const QString &caption = QString(),
		const QString &directory = QString(),
		const QString &filter = QString())
		: QFileDialog(parent, caption, directory, filter)
	{
		setOption(DontUseNativeDialog, true); //we need qt layout
		setAcceptMode(AcceptSave);
		
		mpDisperseModeCBx = new QCheckBox(tr("Disperse mode (separate contents of source files)"), this);
		
		QGridLayout *l(static_cast<QGridLayout*>(layout()));
		l->addWidget(mpDisperseModeCBx, l->rowCount(), 1, 1, 4);
	}
	void setDispersedMode(bool b) {
		mpDisperseModeCBx->setChecked(b);
	}
	bool dispersedMode() const {
		return mpDisperseModeCBx->isChecked();
	}
	QString selectedFile() const {
		if (selectedFiles().empty())
			return QString();
		return selectedFiles().value(0);
	}
};

void ADBMainWin::slotSaveAs()
{
	ADCSaveProjectDialog dialog(this, tr("Save Project"), 
		QString(),//working dir
		tr("Projects (*." PRIMARY_EXT ");;") + tr("All Files (*)"));

//	dialog.setDispersedMode(UPREFS.getDispersedMode());

	QString fn;
	if (dialog.exec() == QDialog::Accepted)
		fn = dialog.selectedFile();

	if (!fn.isEmpty())
	{
		QString cmd("save");
		if (dialog.dispersedMode())
			cmd.append(" -d");
		else
			cmd.append(" -s");
		cmd.append(" " + QDir::toNativeSeparators(fn));
		slotPostCommand(cmd);
//		UPREFS.setDispersedMode(dialog.dispersedMode());
		return;
	}

	statusBar()->showMessage(tr("Save aborted"), 2000);
}

void ADBMainWin::slotSetTargetDirectory()
{
	QString dir(QFileDialog::getExistingDirectory(this, tr("Select Output Directory"),
		QString(),//dir
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));

	if (!dir.isEmpty())
		slotPostCommand(QString("setoutdir %1").arg(dir));
}

void ADBMainWin::slotGenerateSources(QString aPath)
{
	QString dir(QFileDialog::getExistingDirectory(this, tr("Select Output Directory"),
		QString(),//dir
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));

	if (dir.isEmpty())
		return;

	QString s("gensrc -b");
	if (!aPath.isEmpty())
		s.append(" -path " + checkPath(aPath));
	s.append(" " + dir);

	slotPostCommand(s);
}

void ADBMainWin::slotDumpExports(QString aModule)
{
	QString fn(QFileDialog::getSaveFileName(this,
		tr("Dump Exports"),
		aModule.toUpper() + EXPORT_FILE_EXT,//dir
		QString("Export Files (*" EXPORT_FILE_EXT ");;") + QString("All Files (*)"),
		nullptr
#if(0)
		, QFileDialog::DontConfirmOverwrite
#endif
	));
	if (fn.isEmpty())
		return;

	QFileInfo fi(fn);
	QStringList l;
	l << "dump_exports";
	if (!aModule.isEmpty())
		l << "-module" << checkPath(aModule);
	//l << "-name" << fi.fileName();
	l << fi.absoluteFilePath();
	slotPostCommand(l.join(" "));
}

void ADBMainWin::slotCalculateDependencies(QString aPath)
{
	QString s("calcdp");
	if (!aPath.isEmpty())
		s.append(" " + checkPath(aPath));
	slotPostCommand(s);
}

void ADBMainWin::slotDumpSourceFile(QString sPath, QString sOptions)
{
	if (sPath.isEmpty())
		return;

	QString fn = QFileDialog::getSaveFileName(this,
		tr("Save Listing"),
		fixPath(sPath),
		QString("Source/Header Files (*.c *.cpp *.cxx *.h *.hpp *.hxx);;") + QString("All Files (*)")
	);

	if (!fn.isEmpty())
	{
		QFileInfo fi(fn);
		QStringList l;
		l << "gensrc";
		if (!sOptions.isEmpty())
			l << sOptions.split(" ");
		l << "-path" << checkPath(sPath);
		//l << "-name" << fi.fileName();
		l << fi.absoluteDir().path();//absoluteFilePath()
		slotPostCommand(l.join(" "));
		//slotPostCommand(QString("save %1").arg(QDir::toNativeSeparators(fn)));
		return;
	}

	statusBar()->showMessage(tr("Save aborted"), 2000);
}

void ADBMainWin::slotNoOutputDirectoryRecoil(QString sRecoiledCmd)
{
	QString dir(QFileDialog::getExistingDirectory(this, tr("Choose a Directory"),
		QString(),//dir
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));

	if (dir.isEmpty())
		return;

	slotPostCommand(QString("setoutdir %1").arg(dir));
	if (!sRecoiledCmd.isEmpty())//post recoiled command
		slotPostCommand(sRecoiledCmd);
}

void ADBMainWin::slotPrint()
{
#ifndef ADC_NO_PRINTER
	dCChildWin* m = (dCChildWin*)ws->activeWindow();
	if (m)
		m->print(printer);
#endif
}


/////////////////////////////////////////////////// ADCAboutDlg

const QString ADCAboutDlg::sHyperLink("andromedadecompiler@gmail.com");

ADCAboutDlg::ADCAboutDlg(QString companyName, QString productName, QString productVersion, QWidget* parent)
	: QDialog(parent),
	mProductName(productName),
	mProductVersion(productVersion)
{
	setWindowTitle(tr("About"));
	setWindowFlags(Qt::Popup);// | Qt::FramelessWindowHint);

	QImage pix(":about2.png");
	setFixedSize(pix.size());

	static const char* fontFamily("Helvetica [Adobe]");//"Andalus"

	QLabel* pProductName(new QLabel(mProductName, this));
	pProductName->setFont(QFont(fontFamily, 20));
	pProductName->setAlignment(Qt::AlignCenter);
	//pProductName->setIndent(12);

	QLabel* pVersion(new QLabel(tr("Version %1").arg(mProductVersion), this));
	pVersion->setFont(QFont(fontFamily, 12));
	pVersion->setAlignment(Qt::AlignCenter);

	QLabel* pCopyright(new QLabel(tr("Copyright %1 2015-%2 %3. All Rights Reserved").arg(QChar(0x00A9)).arg("2021").arg(companyName), this));
	pCopyright->setFont(QFont(fontFamily, 12));
	pCopyright->setAlignment(Qt::AlignCenter);

	QLabel* pContact(new QLabel(tr("<a href=\"%1\">%2</a>").arg(sHyperLink).arg(sHyperLink), this));
	pContact->setFont(QFont(fontFamily, 12));
	pContact->setAlignment(Qt::AlignCenter);
	pContact->setTextFormat(Qt::RichText);
	pContact->setTextInteractionFlags(Qt::TextBrowserInteraction);
	//pContact->setOpenExternalLinks(true);
	connect(pContact, SIGNAL(linkActivated(const QString&)), SLOT(slotLinkActivated(const QString&)));

	QVBoxLayout* v_layout = new QVBoxLayout(nullptr);
#if QT_VERSION_MAJOR >= 6
	v_layout->setContentsMargins(4, 4, 4, 4);
#else
	v_layout->setMargin(4);
#endif
	v_layout->setSpacing(0);
	v_layout->addWidget(pProductName);
	v_layout->addWidget(pVersion);
	v_layout->addWidget(pCopyright);
	v_layout->addWidget(pContact);

	QSpacerItem* spacer1 = new QSpacerItem(1, 40, QSizePolicy::Minimum, QSizePolicy::Fixed);
	QSpacerItem* spacer2 = new QSpacerItem(1, 300, QSizePolicy::Minimum, QSizePolicy::Fixed);

	QVBoxLayout* main_layout = new QVBoxLayout(this);

#if QT_VERSION_MAJOR >= 6
	main_layout->setContentsMargins(0, 0, 0, 0);
#else
	main_layout->setMargin(0);
#endif

	main_layout->setSpacing(0);
	main_layout->addItem(spacer1);
	main_layout->addLayout(v_layout);
	main_layout->addItem(spacer2);

	QPalette palette;
	palette.setBrush(this->backgroundRole(), QBrush(pix));
	this->setPalette(palette);

	int dpi_y = logicalDpiY();

	// check for if Windows has 'Large Fonts' activated.
	// Large fonts will mess up our layout unless we make this correction.

	static const int WIN32_NORMAL_POINT_SIZE = 96;
	if (dpi_y != WIN32_NORMAL_POINT_SIZE)
	{
		// Scale all fonts in labels so that they fit
		float scale_factor = float(WIN32_NORMAL_POINT_SIZE) / dpi_y;

		QFont new_font;
		new_font = pProductName->font();
		new_font.setPointSizeF(new_font.pointSizeF() * scale_factor);
		pProductName->setFont(new_font);

		new_font = pVersion->font();
		new_font.setPointSizeF(new_font.pointSizeF() * scale_factor);
		pVersion->setFont(new_font);

		new_font = pCopyright->font();
		new_font.setPointSizeF(new_font.pointSizeF() * scale_factor);
		pCopyright->setFont(new_font);

		new_font = pContact->font();
		new_font.setPointSizeF(new_font.pointSizeF() * scale_factor);
		pContact->setFont(new_font);
	}
}

void ADCAboutDlg::slotLinkActivated(const QString&)
{
	//CHECK_QSTRING(link, z);
	  //  QDesktopServices::openUrl(QUrl(link));
	QMenu* popup(new QMenu(this));

	QAction* pAction = new QAction(QIcon(":.png"), tr("Copy Address"), this);
	connect(pAction, SIGNAL(triggered()), SLOT(slotCopyContactAddress()));
	popup->addAction(pAction);

	popup->exec(QCursor::pos());
	delete popup;
}


void ADCAboutDlg::slotCopyContactAddress()
{
	QApplication::clipboard()->setText(sHyperLink);
}

void ADBMainWin::slotAbout()
{
//const QString productName("ANDROMEDA DECOMPILER");
//const QString productVersion("2.0.0");

	ADCAboutDlg dlg(ADB().GetCompanyName(), ADB().GetProductName(), ADB().GetProductVersion(), this);
	dlg.exec();
	/*	QMessageBox::about( this, "Qt Application Example",
			"This example demonstrates simple use of\n "
			"Qt's Multiple Document Interface (MDI).");*/
}


/*void ADBMainWin::showPageEx( QWidget * pWin, bool checkOnly, int bDock, int bBar )
{
ADBResManager * pResMgr = app().resManager();
int abconfig = pResMgr->getOnBarConfig( abconfig );
int wdconfig = pResMgr->getWinDockConfig( wdconfig );

DocumentObject * pDoc = dynamic_cast< DocumentObject * >( pWin );
if ( !pDoc || pDoc->mID < 0 )
return;

int pageID = pDoc->mID;

SxMainWindow::showPageEx( pWin, checkOnly,
(wdconfig&(1<<pageID))!=0,
(abconfig&(1<<pageID))!=0 );

pResMgr->SetPageHideConfig( pageID, false );
}

//place docked win on assistant bar
void ADBMainWin::slotPlacePageOnABar(QWidget * pWin)
{
DocumentObject * pDoc = dynamic_cast< DocumentObject * >( pWin );
if ( !pDoc || pDoc->mID < 0 )
return;

//undock it first
ADBResManager * pResMgr = app().resManager();
int dwconfig = pResMgr->getWinDockConfig( dwconfig );
dwconfig &= ~(1 << pDoc->mID);
pResMgr->setWinDockConfig( dwconfig );

int abconfig = pResMgr->getOnBarConfig( abconfig );
abconfig |= (1 << pDoc->mID);
pResMgr->setOnBarConfig( abconfig );

SxMainWindow::slotPlacePageOnABar( pWin );
}

//turn docked state off
void ADBMainWin::slotUndockWin(QWidget * pWin)
{
DocumentObject * pDoc = dynamic_cast< DocumentObject * >( pWin );
if ( pDoc == nullptr )
return;

if ( pDoc->mID < 0 )
return;

ADBResManager * pResMgr = app().resManager();
int dwconfig = pResMgr->getWinDockConfig( dwconfig );
dwconfig &= ~(1 << pDoc->mID);
pResMgr->setWinDockConfig( dwconfig );

SxMainWindow::slotUndockWin( pWin );
}

//move page from assistant bar to main area
void ADBMainWin::slotReplacePage(QWidget * pWin)
{
DocumentObject * pDoc = dynamic_cast< DocumentObject * >( pWin );
if ( pDoc == nullptr )
return;

if ( pDoc->mID < 0 )
return;

ADBResManager * pResMgr = app().resManager();
int abconfig = pResMgr->getOnBarConfig( abconfig );
abconfig &= ~(1 << pDoc->mID);
pResMgr->setOnBarConfig( abconfig );

SxMainWindow::slotReplacePage( pWin );
}

void ADBMainWin::slotDockPage(QWidget * pWin)
{
DocumentObject * pDoc = dynamic_cast< DocumentObject * >( pWin );
if ( pDoc == nullptr )
return;

if ( pDoc->mID < 0 )
return;

ADBResManager * pResMgr = app().resManager();
int abconfig = pResMgr->getWinDockConfig( abconfig );
abconfig |= (1 << pDoc->mID);
pResMgr->setWinDockConfig( abconfig );

SxMainWindow::slotDockPage( pWin );
}
*/



void ADBMainWin::slotShowProjectView()
{
	showPageEx(page(PAGE_FILES));
}

void ADBMainWin::slotShowOutputView()
{
	showPageEx(page(PAGE_OUTPUT)/*, -1*/);
}

void ADBMainWin::slotShowInputLine()
{
	showPageEx(page(PAGE_OUTPUT)/*, -1*/);
	emit signalToggleInputLine();
}

void ADBMainWin::slotShowDisassemblyView()
{
	showPageEx(page(PAGE_DASM));
}

/*void ADBMainWin::slotShowDecompyleView()
{
	showPageEx(page(PAGE_DC));
}*/

void ADBMainWin::slotShowTypesView()
{
	showPageEx(page(PAGE_TYPES));
}

void ADBMainWin::slotShowNamesView()
{
	showPageEx(page(PAGE_NAMES));
}

void ADBMainWin::slotShowExportsView()
{
	showPageEx(page(PAGE_EXPORTS));
}

void ADBMainWin::slotShowImportsView()
{
	showPageEx(page(PAGE_IMPORTS));
}

void ADBMainWin::slotShowResourcesView()
{
	showPageEx(page(PAGE_RESOURCES));
}

void ADBMainWin::slotShowCanvasWin()
{
	showPageEx(page(PAGE_CANVAS));
}

void ADBMainWin::slotShowStrucEditView()
{
	showPageEx(page(PAGE_STRUCED));
}

void ADBMainWin::slotOpenTypeDlg()
{
#if(0)
	showPageEx(page(PAGE_TYPEBLDR));
	emit signalUpdateContextTypes();
#else
	if (!mpTypesDlg)
	{
		mpTypesDlg = new ADCTypesDlg(this, "ADCTYPEEDITWIN");
		mpTypesDlg->updateOutputFont(mrResMgr.getOutputFont());

		connect(mpTypesDlg, SIGNAL(signalRequestModel(ADCTypesDlg&, QString)), SLOT(slotRequestModel(ADCTypesDlg&, QString)));
		connect(this, SIGNAL(signalLocusChanged(QString, int)), mpTypesDlg, SLOT(slotLocusChanged(QString, int)));
		connect(this, SIGNAL(signalProjectAboutToClose()), mpTypesDlg, SLOT(depopulate()));
		connect(this, SIGNAL(signalUpdateContextTypes()), mpTypesDlg, SLOT(slotUpdateContextTypes()));
	}

	mpTypesDlg->show();


#endif
}

void ADBMainWin::slotShowTasksView()
{
	showPageEx(page(PAGE_TASKS));
}

void ADBMainWin::slotAvailCmdListChanget(const QStringList& lst)
{
	for (int i = 0; i < (int)lst.size(); i++)
	{
		const QString& s = lst[i];
		mAvailCmdList.insert(s);
	}
}

bool ADBMainWin::IsCommandAvailable(const QString& s)
{
	std::set<QString>::iterator it = mAvailCmdList.find(s);
	return (it != mAvailCmdList.end());
}

/*void ADBMainWin::windowsMenuAboutToShow()
{
FillWindowsMenu( mpWindowMenu );
}

void ADBMainWin::windowsMenuActivated( int  )
{
QWidget* w = ws->windowList().at( id );
if ( w )
w->showNormal();
w->set Focus();
}

void ADBMainWin::tileHorizontal()
{
// primitive horizontal tiling
QWidgetList windows = ws->windowList();
if ( !windows.count() )
return;

int heightForEach = ws->height() / windows.count();
int y = 0;
for ( int i = 0; i < int(windows.count()); ++i ) {
QWidget *window = windows.at(i);
if ( window->testWState( WState_Maximized ) ) {
// prevent flicker
window->hide();
window->showNormal();
}
int preferredHeight = window->minimumHeight()+window->parentWidget()->baseSize().height();
int actHeight = QMAX(heightForEach, preferredHeight);

window->parentWidget()->setGeometry( 0, y, ws->width(), actHeight );
y += actHeight;
}
}*/

void ADBMainWin::slotAppendText(const char* str)
{
	slotShowOutputView();
	emit signalAppendText(str);
}

/*void ADBMainWin::slotCurPosChanged(adcui::ADDR pos)
{
	mpCurPos->setShown(true);
	QString s;
	s.setNum(pos, 16);
	mpCurPos->setText(QString("CP:%1").arg(s));
	*//*if (0)
	if (DisassemblyWin()->isVisible())
	{
	ADCBinView * v = DisassemblyWin()->view();
	v->go2AddrV(pos, 0, false);
	}*/
	//}

void ADBMainWin::slotAnlzStatusChanged(int status)
{
	//bool bPluggedIn = (status != 0);
	bool bRunning(status > 0);
	bool bPaused(status > 1);

	//mpDecompileAction->setEnabled(mbHasProject && !bRunning);
	mpContinueAction->setEnabled(bPaused);
	mpPauseAction->setEnabled(bRunning && !bPaused);
	mpStopAction->setEnabled(bRunning);
	mpGoTaskTopAction->setEnabled(bPaused);

	if (bPaused)
	{
		mpContinueAction->setShortcut(Qt::Key_F4);
		//mpDecompileAction->setShortcut(QKeySequence());
	}
	else
	{
		mpContinueAction->setShortcut(QKeySequence());
		//mpDecompileAction->setShortcut(Qt::Key_F4);
	}

	if (status == 1)//started
	{
		emit signalAnalysisStarted();
	}
	else if (status == 2)//paused
	{
	}
	else if (status == 0)//stopped
	{
		emit signalAnalysisFinished();
	}

	if (status == 0)//stopped
		mpAnalizerDial->hide();
}

void ADBMainWin::slotDialAnalizerInfo(QString s)
{
	//mpProgressInfoDial->show();
	setStatusSlotText(mpAnalizerDial, s);
}

void ADBMainWin::slotDialProgressInfo(QString s)
{
	//mpProgressInfoDial->show();
	setStatusSlotText(mpProgressDial, s);
}

void ADBMainWin::slotCommandSucceeded(int)// cmdID)
{
	/*	if (cmdID == CMD_LOADBIN)
		{
			slotShowDisassemblyView();
			return;
		}

		if (cmdID == CMD_DECOMP)
		{
			slotShowProjectView();
			return;
		}*/
}

void ADBMainWin::slotProjectClosed()
{
	slotProjectChanged();
	emit signalProjectClosed();
	mbHasProject = false;
	OnProjectStatus(false);
	slotLocusInfoChanged(QString(), 0);//hide locus info
}

void ADBMainWin::updateTitle()
{
	QString sCaption = app().ProductName();

	mProjectName.clear();
	mProjectPath.clear();

	ADCStream ss;
	if (ADB().GetProjectInfo(ss))
	{
		mProjectName = ss.ReadUString();
		mProjectPath = ss.ReadUString();
		QString s(mProjectName);
		if (!s.isEmpty())
		{
			if (!mProjectPath.isEmpty() && mProjectPath != s)
				s.append(QString(" (%1)").arg(mProjectPath));
		}
		if (!s.isEmpty())
		{
			s.append(" - ");
			sCaption.prepend(s);
		}
	}
	setWindowTitle(sCaption);
}

void ADBMainWin::slotProjectChanged()
{
	updateTitle();
	{//?
		//ADCStream ss;
		//OnFileListChanged(ss);
		emit signalProjectChanged();
	}

	//emit signalProjectModified();

	//slotUpdateFileList();
}

void ADBMainWin::slotProjectNew()
{
	//CreateDisassemblyWin();
	emit signalProjectNew();
	slotProjectChanged();
	mbHasProject = true;
	OnProjectStatus(mbHasProject);
}

void ADBMainWin::slotProjectOpened()
{
	slotProjectNew();
	updateRecentProjects(mProjectPath);
}

void ADBMainWin::slotProjectSaved()
{
	updateTitle();
	//slotProjectChanged();
	//setCurrentFile(mProjectPath);
	updateRecentProjects(mProjectPath);
}

bool ADBMainWin::handleOutput(ADCStream& ss)
{
	QString qs;
	if (!ss.ReadString(qs))//header
		return false;

	if (qs.section(' ', 1, 1).toInt() == 0)
	{
		assert(PAGE_OUTPUT == 0);
	}

	int iRet(0);
	static QString gs;
	QString s(gs);
	gs.clear();
	while (ss.ReadUStringAppend(s, "\n", true))//append delimiter to find if a full line was read
	{
		if (s.back() == '\n')
		{
			s.chop(1);
			emit signalAppendOutput(s, 0);//mOutputId);
			s.clear();
			iRet++;
		}
		else
			gs = s;
	}
	return (iRet > 0);
}

void ADBMainWin::OnOutput(MyStreamBase* pss)
{
	if (pss)
	{
		SpiceStream2 ss(*pss);
		handleOutput(ss);
	}
	else
	{
		static int here(0);
		if (!here)
		{
			here++;
			qApp->processEvents();
			ADCStream ss;
			ss.WriteInt(32 * 1024);
			if (ADB().FlushOutput(ss))
			{
				if (handleOutput(ss))
				{
					app().postCoreEvent(adcui::MSGID_OUTPUT_READY);
				}
			}
			here--;
		}
	}
}

/*void ADBMainWin::slotUpdateFileList()
{
	ADCStream ss;
	ADB().GetProjectFiles(ss);
	OnFileListChanged(ss);
}*/

/*void ADBMainWin::slotPopulateFiles(ADCStream &ss)
{
	//ADCStream ss;
	ADB().GetProjectFiles(ss);
	//OnFileListChanged(ss);
}*/


void ADBMainWin::OnProjectAboutToClose()
{
	ADB().ClearLocus();
	while (getDocsCount() > 0)
	{
		DocumentObject* pDoc(getDoc(0));
		QWidget* pWin0(dynamic_cast<QWidget*>(pDoc));
		closePage(pWin0);
	}
	emit signalProjectAboutToClose();
}

void ADBMainWin::OnContextIdChanged()
{
	//mContextId = ADB().ContextId();
	emit signalContextIdChanged(ADB().ContextId());
}

void ADBMainWin::slotContextIdInq(unsigned &u)
{
	u = ADB().ContextId();
}

void ADBMainWin::slotPopulateTasks(ADCStream& ss)
{
	ADB().GetToDoList(ss);
}

struct here_t
{
	static int count;
	here_t()
	{
		assert(count == 0);
		count++;
	}
	~here_t()
	{
		count--;
		assert(count == 0);
	}
};

int here_t::count = 0;

static const char* GuiColorTag(adcui::LogColorEnum c)//for output log
{
	using namespace adcui;
	switch (c)
	{
	case COLORTAG_OFF: return "</font>";
	case COLORTAG_RED: return "<font color=red>";
	case COLORTAG_ORANGE: return "<font color=#9600F6>";
	case COLORTAG_DARKRED: return "<font color=darkRed>";
	case COLORTAG_DARKBLUE: return "<font color=darkBlue>";
	case COLORTAG_DARKGREEN: return "<font color=darkGreen>";
	case COLORTAG_HYPERLINK: return "<u><font color=#0080c0>";
	case COLORTAG_HYPERLINK_OFF: return "</u></font>";
	default:
		break;
	}
	Q_ASSERT(0);
	return "";
}

int ADBMainWin::HandleEvent(int msgID, void* pData)
{
	using namespace adcui;
#ifdef _DEBUG
	//here_t here;
#endif

#if(0)
	if (msgID != MSGID_OUTPUT_READY)
	{
		static int zz = 0;
		fprintf(stdout, "%d:\tevent(%d)\n", ++zz, (int)msgID);
		fflush(stdout);
	}
#endif

	switch (msgID)
	{
	case MSGID_OUTPUT_COLOR:
	{
		struct s_t { adcui::LogColorEnum in; const char* out; } *s = (s_t *)pData;//Warning:duplicated
		s->out = GuiColorTag(s->in);
		break;
	}

	case MSGID_OUTPUT_READY:
		OnOutput((MyStreamBase*)pData);
		break;

	case UIMSG_PROCESS_EVENTS:
		qApp->processEvents();
		break;

	case UIMSG_PROJECT_NEW:
		slotProjectNew();
		break;
	case UIMSG_PROJECT_OPENED:
		slotProjectOpened();
		break;

	case UIMSG_PROJECT_ABOUT_TO_CLOSE:
		OnProjectAboutToClose();
		break;

	case UIMSG_PROJECT_CLOSED:
		//DestroyDisassemblyWin();
		slotProjectClosed();
		break;

	case UIMSG_PROJECT_SAVED:
		slotProjectSaved();
		break;

	case UIMSG_GLOBALS_MODIFIED:
		emit signalProjectModified();
		break;

	case UIMSG_SAVE_RECOIL:
	{
		//SpiceStream2 ss(*(MyStreamBase*)pData);
		//bool disperseMode(false);
		//ss.ReadBool(&disperseMode);
		slotSaveAs();
		break;
	}

	case UIMSG_FILE_LIST_CHANGED:
		//slotUpdateFileList();
		emit signalFileListChanged();
		break;

	/*	case UIMSG_TODOLIST_PUSHED_BACK:
			{
				SpiceStream2 ss(*(MyStreamBase *)pData);
				QString s(ss.ReadString());
				emit signalToDoListPushedBack(s.section('\t', 0, 0), s.section('\t', 1));
			}
			break;

		case UIMSG_TODOLIST_PUSHED_FRONT:
			{
				SpiceStream2 ss(*(MyStreamBase *)pData);
				QString s(ss.ReadString());
				emit signalToDoListPushedFront(s.section('\t', 0, 0), s.section('\t', 1));
			}
			break;

		case UIMSG_TODOLIST_POPPED_FRONT:
			emit signalToDoListPoppedFront();
			break;*/

	case UIMSG_APPEND_TEXT:
		slotAppendText((const char*)pData);
		break;

	case UIMSG_COMMAND_SUCCEEDED:
		slotCommandSucceeded(*(int*)pData);
		break;

	case UIMSG_GLOBAL_ADDED:
		//slotCurPosChanged(*(adcui::ADDR *)pData);
		break;

		//case UIMSG_ANLZ_STATUS_CHANGED:
			//slotAnlzStatusChanged(*(int *)pData);
			//break;

	case UIMSG_PREANALIZED:
		emit signalPreanalized();
		break;

	case UIMSG_ANALIZER_PAUSED:
		slotAnlzStatusChanged(2);
		break;

	case UIMSG_ANALIZER_STARTED:
	case UIMSG_ANALIZER_RESUMED:
		slotAnlzStatusChanged(1);
		break;

	case UIMSG_ANALIZER_STOPPED:
		slotAnlzStatusChanged(0);
		break;

	case UIMSG_TODOLIST_CHANGED:
	{
		emit signalTaskListChanged();
		break;
	}

	case UIMSG_ANALIZER_INFO:
	{
		SpiceStream2 ss(*(MyStreamBase*)pData);
		QString s(ss.ReadString());
		slotDialAnalizerInfo(s);
		break;
	}

	case UIMSG_PROGRESS_INFO:
	{
		SpiceStream2 ss(*(MyStreamBase*)pData);
		QString s(ss.ReadString());
		slotDialProgressInfo(s);
		break;
	}

	case UIMSG_GOTO_LOCATION:
	{
		SpiceStream2 ss(*(MyStreamBase*)pData);
		QString s(ss.ReadString());
		emit signalShowDisassemblyAt(s, 20);
		break;
	}

	case UIMSG_CURFUNC_MODIFIED:
	case UIMSG_CURSTRUC_MODIFIED:
		break;

	case UIMSG_CURFUNC_CHANGED:
	{
		/*ADCStream ss;
		ADB().GetLocusInfo(3, ss);//path up to function, skip segments
		QString sName(ss.ReadString());
		QString sPath(ss.ReadString());
		emit signalCurInfoChanged(sName, sPath);*/
		break;
	}

	case UIMSG_SELOBJ_CHANGED:
		emit signalSelObjChanged();
		break;

	case UIMSG_LOCUS_CHANGED:
		signalSelObjChanged();
		break;

	case UIMSG_LOCUS_ADJUSTED:
		signalLocusAdjusted();
		break;

	case UIMSG_TYPESMAP_CHANGED:
		signalTypesMapChanged();
		break;

	case UIMSG_LOCALITY_CHANGED:
		OnContextIdChanged();
		break;

	/*case UIMSG_PING_BACK:
	{
		SpiceStream2 ss(*(MyStreamBase*)pData);
		int n;
		if (ss.ReadInt(&n))
		{
			if (n == 666)
				emit signalSync PanesResponce(0, true);
			else
				emit signalSync PanesResponce(n, true);
		}
		break;
	}*/

	case UIMSG_COMMAND_RECOIL:
	{
		SpiceStream2 ss(*(MyStreamBase*)pData);
		slotNoOutputDirectoryRecoil(ss.ReadString());
		break;
	}

	case UIMSG_NAME_CHANGED:
		emit signalNameChanged();
		break;

		//debugger
	case UIMSG_DEBUGGER_STARTED:
	case UIMSG_DEBUGGER_RESUMED:
		enableDebuggerActions(1);
		//emit signalDebuggerBreak();
		break;

	case UIMSG_DEBUGGER_STOPPED:
		enableDebuggerActions(0);
		OnDebuggerStopped();
		break;

	case UIMSG_DEBUGGER_BREAK:
		enableDebuggerActions(2);
		emit signalDebuggerBreak();
		break;

	case UIMSG_NEW_FOLDER_RECOIL:
	{
		SpiceStream2 ss(*(MyStreamBase*)pData);
		emit signalNewFolderRecoil(ss.ReadString());
		break;
	}

	default:
		return 0;
	}

	return 1;
}

