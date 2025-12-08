
#include "ADCMainWin.h"

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

#include "sx/SxDocument.h"
#include "sx/SxSignalMultiplexer.h"
#include "sx/SxDockBar.h"
#include "sx/SxViewMgrTabs.h"
#include "sx/SxOutputWin.h"
#include "ADCApp.h"
#include "ADCBlocksWin.h"
#include "ADCCodeWin.h"
#include "ADCExprWin.h"
#include "ADCCutListWin.h"
#include "ADCStubsWin.h"
#include "ADCSourceWin.h"
#include "ADCUserPrefs.h"
#include "ADCTypesWin.h"
#include "ADCNamesWin.h"
#include "ADCTemplWin.h"
#include "ADCUtils.h"
#include "ADCProtoDlg.h"


static DocumentObject* createSourceWinDoc(SxMainWindow* pMainWin0, QWidget*, QString& path, QString& extra)
{
	ADCMainWin* pMainWin(static_cast<ADCMainWin*>(pMainWin0));
	return pMainWin->CreateSourceWin(path, extra);
}

static DocumentObject* createStubsWinDoc(SxMainWindow* pMainWin0, QWidget*, QString& path, QString& extra)
{
	ADCMainWin* pMainWin(static_cast<ADCMainWin*>(pMainWin0));
	return pMainWin->CreateStubsView(path, extra);
}

static DocumentCreator StubsAreaDocCreator("ADC_STUBSWIN", createStubsWinDoc);
static DocumentCreator SourceAreaDocCreator("ADC_SOURCEWIN", createSourceWinDoc);

//==============================================================
ADCMainWin::ADCMainWin(ADBResManager& rm, adcui::IADCCore& rADC)
	: ADBMainWin(rm, rADC),
	mrADC(rADC),
	mpProtoDlg(nullptr)
{
	CreateTemplatesWin();
	CreateStubsWin();
	CreateBlocksWin();
	CreateExprWin();
	CreateProtoEditDlg();
	CreateInterWin();
	CreateCutListWin();

	mpGoBackAction = new QAction(SxIcon(":go_back_{16|24}.png"), tr("Navigate Backward"), this);
	mpGoBackAction->setShortcut(Qt::CTRL | Qt::Key_Minus);
	connect(mpGoBackAction, SIGNAL(triggered()), SLOT(slotJumpSourceBack()));

	mpGoForwardAction = new QAction(SxIcon(":go_forward_{16|24}.png"), tr("Navigate Forward"), this);
	mpGoForwardAction->setShortcut(Qt::CTRL | Qt::Key_Equal);//Qt::KeyPlus duesn't work!
	connect(mpGoForwardAction, SIGNAL(triggered()), SLOT(slotJumpSourceForward()));

	mpSyncModeAction = new QAction(SxIcon(":sync_mode_{16|24}.png"), tr("Synchronize Mode"), this);//sync_mode2//arrow_left_right//sync_mode3
	mpSyncModeAction->setCheckable(true);
	mpSyncModeAction->setStatusTip(tr("Toggle Synchronize mode"));
	//connect(mpSyncModeAction, SIGNAL(toggled(bool)), SLOT(slotSyncModeToggled(bool)));
	connect(mpSyncModeAction, SIGNAL(toggled(bool)), SIGNAL(signalSyncModeToggled(bool)));

	mpSplitPaneLeftAction = new QAction(SxIcon(":left_pane_{16|24}.png"), tr("Toggle Left Pane"), this);
	connect(mpSplitPaneLeftAction, SIGNAL(triggered()), SLOT(slotToggleLeftPane()));

	mpSplitPaneRightAction = new QAction(SxIcon(":right_pane_{16|24}.png"), tr("Toggle Right Pane"), this);
	connect(mpSplitPaneRightAction, SIGNAL(triggered()), SLOT(slotToggleRightPane()));

	mpCompileAction = new QAction(QIcon(":compile.png"), tr("Compile"), this);
	mpCompileAction->setShortcut(tr("Ctrl+F7"));
	connect(mpCompileAction, SIGNAL(triggered()), SLOT(slotCompile()));

	mpBuildMenu = new QMenu(tr("&Build"), this);
	mpBuildMenu->addAction(mpCompileAction);
	
	menuBar()->insertMenu(mpAnalMenu->menuAction(), mpBuildMenu);

	mpShowCutsAction = new QAction(SxIcon(":cut_list_{16|24}.png"), tr("Cut List"), this);
	connect(mpShowCutsAction, SIGNAL(triggered()), SLOT(slotShowCutsView()));

	mpShowInterCodeViewAction = new QAction(SxIcon(":inter_code_{16|24}.png"), tr("Inter Code"), this);//QIcon(":source_low.png")
	connect(mpShowInterCodeViewAction, SIGNAL(triggered()), SLOT(slotShowInterView()));

	mpShowStubsAction = new QAction(QIcon(":document_stubs.png"), tr("Prototypes"), this);
	connect(mpShowStubsAction, SIGNAL(triggered()), SLOT(slotShowStubsView()));

	mpShowBlocksAction = new QAction(SxIcon(":blocks_{16|24}.png"), tr("Code Blocks"), this);//QIcon(":blocks.png")
	connect(mpShowBlocksAction, SIGNAL(triggered()), SLOT(slotShowBlocksView()));

	mpShowExprAction = new QAction(SxIcon(":expressions_{16|24}.png"), tr("Expressions"), this);//QIcon(":expressions.png")
	connect(mpShowExprAction, SIGNAL(triggered()), SLOT(slotShowExprView()));

	//mpViewMenu->addAction(mpShowStubsAction);
	//mpViewMenu->addAction(mpShowProtoDlgAction);
	//mpViewMenu->addAction(mpShowDecompyleAction);

	QAction* pFirst(mpViewMenu->actions().first());
	mpViewMenu->insertAction(pFirst, mpGoBackAction);
	mpViewMenu->insertAction(pFirst, mpGoForwardAction);
	mpViewMenu->insertSeparator(pFirst);
	
	mpViewMenu->insertAction(mpViewMSepa, mpShowInterCodeViewAction);
	mpViewMenu->insertAction(mpViewMSepa, mpShowCutsAction);
	mpViewMenu->insertAction(mpViewMSepa, mpShowBlocksAction);
	mpViewMenu->insertAction(mpViewMSepa, mpShowExprAction);
	mpViewMenu->insertSeparator(mpViewMSepa);
	mpViewMenu->insertAction(mpViewMSepa, mpSplitPaneLeftAction);
	mpViewMenu->insertAction(mpViewMSepa, mpSplitPaneRightAction);
	mpViewMenu->insertAction(mpViewMSepa, mpSyncModeAction);
	mpViewMenu->insertAction(mpViewMSepa, mpTabbedWindowsAction);

	pFirst = mpViewTBar->actions().first();
	mpViewTBar->insertAction(pFirst, mpGoBackAction);
	mpViewTBar->insertAction(pFirst, mpGoForwardAction);
	mpViewTBar->insertSeparator(pFirst);

	mpViewTBar->insertAction(mpViewTSepa, mpShowInterCodeViewAction);
	mpViewTBar->insertAction(mpViewTSepa, mpShowCutsAction);
	mpViewTBar->insertAction(mpViewTSepa, mpShowBlocksAction);
	mpViewTBar->insertAction(mpViewTSepa, mpShowExprAction);
	mpViewTBar->insertSeparator(mpViewTSepa);
	mpViewTBar->insertAction(mpViewTSepa, mpSplitPaneLeftAction);
	mpViewTBar->insertAction(mpViewTSepa, mpSplitPaneRightAction);
	mpViewTBar->insertAction(mpViewTSepa, mpSyncModeAction);

	updateSplitPanesUI();
}

void ADCMainWin::CreateTemplatesWin()
{
	if (!page(PAGEX_TEMPLATES))
	{
		DocumentWin<ADCTemplWin>* pWin = new DocumentWin<ADCTemplWin>(this, "ADCTEMPLATESWIN");
		pWin->setPermanent(true);
		pWin->mID = PAGEX_TEMPLATES;
		pWin->setStrID(QString(), tr("Aliases"));
		pWin->mstrIcon = ":type_template.png";

		connect(this, SIGNAL(signalOpenTemplatesView(QString)), pWin, SLOT(slotOpenView(QString)));

		connect(this, SIGNAL(signalProjectAboutToClose()), pWin, SLOT(depopulate()));

		connect(pWin, SIGNAL(signalRequestModel(ADCTemplView&, QString, bool)), SLOT(slotRequestModel(ADCTemplView&, QString, bool)));
		//connect(pWin, SIGNAL(signalItemDoubleClicked(QString)), SLOT(slotTypeItemDoubleClicked(QString)));

		RegisterPage(PAGEX_TEMPLATES, pWin, nullptr, Qt::BottomDockWidgetArea);

		//pWin->init();
	}
}

void ADCMainWin::CreateStubsWin()
{
	if (!page(PAGEX_PROTOTYPES))
	{
		DocumentWin<ADCStubsWin>* pWin = new DocumentWin<ADCStubsWin>(this, "ADCSTUBSWIN");
		//?pWin->setModel(ADB().GetStubsViewModel());
		pWin->setPermanent(true);
		pWin->mID = PAGEX_PROTOTYPES;
		pWin->setStrID(QString(), tr("Prototypes"));
		pWin->mstrIcon = ":document_stubs.png";

		connect(this, SIGNAL(signalOpenStubsView(QString)), pWin, SLOT(slotOpenView(QString)));
		connect(this, SIGNAL(signalProjectAboutToClose()), pWin, SLOT(depopulate()));

		//connect(this, SIGNAL(signalProjectClosed()), pWin, SLOT(slotProjectClosed()));
/*		connect(this, SIGNAL(signalProjectNew()), pWin, SLOT(slotProjectNew()));
		connect(this, SIGNAL(signalDcNew()), pWin, SLOT(slotDcNew()));
		connect(this, SIGNAL(signalProjectChanged()), pWin, SLOT(slotReset()));*/
		connect(this, SIGNAL(signalStubsModified()), pWin, SLOT(slotUpdate()));
		/*connect(this, SIGNAL(signalEditStub()), pWin, SLOT(slotEditStub()));
		connect(this, SIGNAL(signalGoToTaskTop()), pWin, SLOT(slotGoToTaskTop()));
		//connect(this, SIGNAL(signalHighlightLocationAt(QString, QString, QString, int)), pWin, SLOT(slotHighlightLocationAt(QString, QString, QString, int)));
		connect(this, SIGNAL(signalAnalysisStarted()), pWin, SLOT(slotAnalysisStarted()));
		connect(this, SIGNAL(signalSelObjChanged()), pWin, SLOT(slotSelFuncChanged()));
		connect(this, SIGNAL(signalNameChanged()), pWin, SLOT(slotUpdate()));
		connect(this, SIGNAL(signalAnalysisFinished()), pWin, SLOT(slotUpdate()));
		connect(pWin, SIGNAL(signalGoToLocation(QString)), SLOT(slotShowDisassemblyAt(QString)));*/


		connect(pWin, SIGNAL(signalRequestModel(ADCStubsView&, QString)), SLOT(slotRequestModel(ADCStubsView&, QString)));
		//connect(pWin, SIGNAL(signalRequestModel(ADCStubsWin &)), SLOT(slotRequestModel(ADCStubsWin &)));

		RegisterPage(PAGEX_PROTOTYPES, pWin, nullptr, Qt::BottomDockWidgetArea);
	}
}

void ADCMainWin::CreateBlocksWin()
{
	if (!page(PAGEX_BLOCKS))
	{
		DocumentWin<ADCBlocksWin>* pWin = new DocumentWin<ADCBlocksWin>(this, "ADCBLOCKSWIN");
		pWin->setPermanent(true);
		pWin->mID = PAGEX_BLOCKS;
		pWin->setStrID(QString(), tr("Code Blocks"));
		pWin->mstrIcon = ":blocks_24.png";

		connect(this, SIGNAL(signalProjectChanged()), pWin, SLOT(slotReset()));
		connect(this, SIGNAL(signalCurInfoChanged(const ADCCurInfo &)), pWin, SLOT(slotCurInfoChanged(const ADCCurInfo &)));
		connect(this, SIGNAL(signalSyncPanesResponce(int, bool)), pWin, SLOT(slotSyncPanesResponce2(int, bool)));
		connect(pWin, SIGNAL(signalDumpBlocks(QString, ADCStream&)), SLOT(slotDumpBlocks(QString, ADCStream&)));

		RegisterPage(PAGEX_BLOCKS, pWin, nullptr, Qt::RightDockWidgetArea);
	}
}

void ADCMainWin::CreateInterWin()
{
	if (!page(PAGEX_INTER))
	{
		DocumentWin<ADCInterCodeWin>* pWin = new DocumentWin<ADCInterCodeWin>(this, "ADCIRWIN");
		pWin->setPermanent(true);
		pWin->mID = PAGEX_INTER;
		pWin->setStrID(QString(), tr("Inter Code"));
		pWin->mstrIcon = ":inter_code_24.png";//source_low

		connect(this, SIGNAL(signalProjectAboutToClose()), pWin, SLOT(slotReset()));
		//connect(this, SIGNAL(signalProjectChanged()), pWin, SLOT(slotReset()));
		connect(this, SIGNAL(signalCurInfoChanged(const ADCCurInfo &)), pWin, SLOT(slotCurInfoChanged(const ADCCurInfo &)));
		connect(this, SIGNAL(signalSyncPanesResponce(int, bool)), pWin, SLOT(slotSyncPanesResponce2(int, bool)));
		connect(this, SIGNAL(signalContextIdChanged(unsigned)), pWin, SIGNAL(signalContextIdChanged(unsigned)));
		connect(this, SIGNAL(signalSrcDumpInvalidated(QString)), pWin, SLOT(slotSrcDumpInvalidated(QString)));
		connect(pWin, SIGNAL(signalCaretPosChanged(int, int, const ADCCurInfo &)), SLOT(slotCaretPosChanged(int, int, const ADCCurInfo &)));
		connect(pWin, SIGNAL(signalRequestModel(ADCInterCodeWin &, QString)), SLOT(slotRequestModel(ADCInterCodeWin &, QString)));
		connect(pWin, SIGNAL(signalContextIdInq(uint &)), SLOT(slotContextIdInq(uint &)));
		connect(pWin, SIGNAL(signalPostCommand(const QString&, bool)), SLOT(slotPostCommand(const QString&, bool)));
		connect(pWin, SIGNAL(signalSaveListing3(QString, QString)), SLOT(slotDumpSourceFile(QString, QString)));
		connect(pWin, SIGNAL(signalSyncPanesRequest3(int, bool)), SLOT(slotSyncSourcePanesRequest(int, bool)));
		connect(pWin, SIGNAL(signalQuickPrototype()), SLOT(slotShowProtoDlg()));

		RegisterPage(PAGEX_INTER, pWin, nullptr, Qt::RightDockWidgetArea);
	}
}

void ADCMainWin::CreateExprWin()
{
	if (!page(PAGEX_EXPR))
	{
		DocumentWin<ADCExprWin>* pWin = new DocumentWin<ADCExprWin>(this, "ADCEXPRWIN");
		pWin->setPermanent(true);
		pWin->mID = PAGEX_EXPR;
		pWin->setStrID(QString(), tr("Expressions"));
		pWin->mstrIcon = ":expressions_24.png";

		connect(this, SIGNAL(signalProjectNew()), pWin, SLOT(slotProjectNew()));
		connect(this, SIGNAL(signalProjectAboutToClose()), pWin, SLOT(slotAboutToClose()));
		//connect(this, SIGNAL(signalProjectChanged()), pWin, SLOT(slotReset()));
		connect(this, SIGNAL(signalCurInfoChanged(const ADCCurInfo &)), pWin, SLOT(slotCurInfoChanged(const ADCCurInfo &)));
		connect(this, SIGNAL(signalCurOpChanged()), pWin, SLOT(slotCurExprChanged()));
		//connect(this, SIGNAL(signalSync PanesResponce(int, bool)), pWin, SLOT(slotSyncResponce(int, bool)));
//		connect(this, SIGNAL(signalBinaryDumpPicked()), pWin, SLOT(slotSyncResponce()));
		connect(this, SIGNAL(signalSourceDumpPicked(bool)), pWin, SLOT(slotSyncResponce(bool)));

		//connect(pWin, SIGNAL(signalDumpExpr(ADCStream&, bool)), SLOT(slotDumpExpr(ADCStream&, bool)));
		//connect(pWin, SIGNAL(signalDumpPtrExprList(ADCStream&)), SLOT(slotDumpPtrExprList(ADCStream&)));
		connect(pWin, SIGNAL(signalRequestModel(ADCExprWin&)), SLOT(slotRequestModel(ADCExprWin&)));
		//connect(pWin, SIGNAL(signalSyncModeInquiry(bool&)), SLOT(slotSyncModeInquiry(bool&)));


		RegisterPage(PAGEX_EXPR, pWin, nullptr, Qt::TopDockWidgetArea);
	}
}

void ADCMainWin::CreateProtoEditDlg()
{
	if (!mpProtoDlg)
	{
		mpProtoDlg = new ADCProtoEditDlg(this);
		connect(mpProtoDlg, SIGNAL(signalSetProtoData(const ADCProtoEditDlg::Data &, bool)), SLOT(slotSetProtoData(const ADCProtoEditDlg::Data&, bool)));
		connect(this, SIGNAL(signalSetProtoData(const ADCProtoEditDlg::Data&)), mpProtoDlg, SLOT(slotSetProtoData(const ADCProtoEditDlg::Data&)));
	}
}

void ADCMainWin::CreateCutListWin()
{
	if (!page(PAGEX_CUTLIST))
	{
		DocumentWin<ADCCutListWin>* pWin = new DocumentWin<ADCCutListWin>(this, "ADCCUTLISTWIN");
		pWin->setPermanent(true);
		pWin->mID = PAGEX_CUTLIST;
		pWin->setStrID(QString(), tr("Cut List"));
		pWin->mstrIcon = ":cut_list_24.png";

		connect(this, SIGNAL(signalCutListUpdated()), pWin, SLOT(slotUpdate()));
		connect(this, SIGNAL(signalProjectAboutToClose()), pWin, SLOT(slotClean()));
		connect(pWin, SIGNAL(signalDumpCutList(ADCStream&)), SLOT(slotDumpCutList(ADCStream&)));
		connect(pWin, SIGNAL(signalUncut(int)), SLOT(slotUncutItem(int)));

		RegisterPage(PAGEX_CUTLIST, pWin, nullptr, Qt::RightDockWidgetArea);
	}
}

DocumentObject* ADCMainWin::CreateStubsView(QString path, QString extra)
{
	QString sStem(path);
	sStem.truncate(sStem.lastIndexOf('.'));

	adcui::IStubsViewModel* pIModel(ADC().NewStubsViewModel(sStem.toUtf8()));
	if (!pIModel)
		return nullptr;

	DocumentWin<ADCStubsWin>* pWin(new DocumentWin<ADCStubsWin>(this, "ADCSTUBSWIN"));
	pWin->setStrID(path, extra);
	pWin->mstrIcon = toIconStr(path);

	connect(this, SIGNAL(signalStubsModified()), pWin, SLOT(slotUpdate()));
	connect(this, SIGNAL(signalEditStub()), pWin, SLOT(slotEditStub()));
	//connect(this, SIGNAL(signalGoToTaskTop()), pWin, SLOT(slotGoToTaskTop()));
	connect(this, SIGNAL(signalAnalysisStarted()), pWin, SLOT(slotAnalysisStarted()));
	connect(this, SIGNAL(signalSelObjChanged()), pWin, SLOT(slotSelFuncChanged()));
	connect(this, SIGNAL(signalNameChanged()), pWin, SLOT(slotUpdate()));
	connect(this, SIGNAL(signalAnalysisFinished()), pWin, SLOT(slotUpdate()));
	//connect(pWin, SIGNAL(signalGoToLocation(QString)), SLOT(slotShowDisassemblyAt(QString)));
	//connect(pWin, SIGNAL(signalRequestModel(ADCStubsWin &)), SLOT(slotRequestModel(ADCStubsWin &)));

	pWin->setModel(mModels, pIModel);
	pIModel->Release();
	//DocumentObject *pDoc(dynamic_cast<DocumentObject *>(pWin));
	//pDoc->updateOutputFont(UPREFS.getOutputFont());

	return static_cast<DocumentObject*>(pWin);
}

DocumentObject* ADCMainWin::CreateSourceWin(QString path, QString extra)
{
	//QString file(path);
	DocumentWin<ADCSourceWin0>* pWin0(new DocumentWin<ADCSourceWin0>(this, "ADCSOURCEWIN"));
	pWin0->setStrID(fixPath(path), extra);
	pWin0->mstrIcon = toIconStr(path);
#if(!NEW_SPLITPANE)
	int fileKind(toFileKind(path));
	QString fileName(pWin0->path().simplified());

	ADCSourceWin* pWin(CreateSourceWin(pWin0));
	pWin0->setWin(pWin);
	pWin->setFileName(fileName);
	pWin->init(ADC().GetFileIdByName(pWin->fileName().toUtf8()), fileKind);
#endif

	dynamic_cast<DocumentObject*>(pWin0)->updateOutputFont(mrResMgr.getOutputFont());
	return static_cast<DocumentObject*>(pWin0);
}


ADCSourceWin* ADCMainWin::CreateSourceWin(ADCSourceWin0* parent)
{
	ADCSourceWin* pWin(new ADCSourceWin(parent, nullptr));//, parent->panes()));
	connect(pWin, SIGNAL(signalRequestModel(ADCSourceWin&, QString, int, int)), SLOT(slotRequestModel(ADCSourceWin&, QString, int, int)));
	connect(pWin, SIGNAL(signalCaretPosChanged(int, int, const ADCCurInfo &)), SLOT(slotCaretPosChanged(int, int, const ADCCurInfo &)));
	connect(pWin, SIGNAL(signalLocusInfo(QString, int)), SLOT(slotLocusInfoChanged(QString, int)));
	connect(pWin, SIGNAL(signalPostCommand(const QString&, bool)), SLOT(slotPostCommand(const QString&, bool)));
	connect(pWin, SIGNAL(signalRefreshBinaryDump(int)), SLOT(slotRefreshBinaryDump(int)));
	connect(pWin, SIGNAL(signalSyncPanesRequest2(int, bool)), SLOT(slotSyncSourcePanesRequest(int, bool)));
	//connect(pWin, SIGNAL(signalSyncModeInquiry(bool&)), SLOT(slotSyncModeInquiry(bool&)));
	connect(pWin, SIGNAL(signalSaveListing2(QString, QString)), SLOT(slotDumpSourceFile(QString, QString)));
	connect(pWin, SIGNAL(signalReleaseModel(QString)), SLOT(slotReleaseModel(QString)));
	connect(pWin, SIGNAL(signalContextIdInq(uint &)), SLOT(slotContextIdInq(uint &)));
	connect(pWin, SIGNAL(signalQuickPrototype()), SLOT(slotShowProtoDlg()));
	connect(pWin, SIGNAL(signalJumpSourceBack(bool)), SLOT(slotJumpSourceBack(bool)));

	connect(this, SIGNAL(signalFileInvalidated(int)), pWin, SIGNAL(signalFileInvalidated(int)));
	connect(this, SIGNAL(signalExprInvalidated()), pWin, SIGNAL(signalUpdateContents()));
	connect(this, SIGNAL(signalCurOpChanged()), pWin, SLOT(slotCurOpChanged()));
	//connect(this, SIGNAL(signalFocusTaskTop(int)), pWin, SLOT(slotFocusTaskTop(int)));
	connect(this, SIGNAL(signalProjectChanged()), pWin, SIGNAL(signalGlobalsModified()));
	connect(this, SIGNAL(signalProjectModified()), pWin, SIGNAL(signalGlobalsModified()));
	connect(this, SIGNAL(signalAnalysisStarted()), pWin, SIGNAL(signalAnalysisStarted()));
	connect(this, SIGNAL(signalDebuggerBreak()), pWin, SLOT(slotDebuggerBreak()));
	connect(this, SIGNAL(signalNameChanged()), pWin, SIGNAL(signalUpdateContents()));
	connect(this, SIGNAL(signalSyncPanesResponce(int, bool)), pWin, SLOT(slotSyncPanesResponce2(int, bool)));
	connect(this, SIGNAL(signalContextIdChanged(unsigned)), pWin, SIGNAL(signalContextIdChanged(unsigned)));
	connect(this, SIGNAL(signalLocusAdjusted()), pWin, SIGNAL(signalLocusAdjusted()));
	//connect(this, SIGNAL(signalSyncModeToggled(bool)), pWin, SLOT(slotSyncModeChanged(bool)));

	pWin->setFont(parent->font());
	//pWin->setSyncMode(mpSyncModeAction->isChecked());
	return pWin;
}

void ADCMainWin::slotRequestModel(ADCSourceWin& rWin, QString path, int index, int type)//-1:src_unfold, 0:src, 1:bin
{
	using namespace adcui;
	//path is unfixed!
	ADCModelData* pModelData(findModel(path, type));
	if (pModelData)
	{
		rWin.setModelDataAt(index, pModelData);
		return;
	}
	switch (type)//SRC
	{
	case -1://unfold
	{
		ISrcViewModel* pIModel(ADC().NewSrcViewModel(path.toUtf8(), DUMP_UNFOLD | DUMP_DEADCODE | DUMP_DEADLABELS | DUMP_COLORS | DUMP_FONTS, nullptr));
		Q_ASSERT(pIModel);
		pModelData = rWin.setModel(mModels, pIModel, index);
		pIModel->Release();//1 left
		registerModelData(path + "\tu", pModelData);
		pModelData->Release();//1 ref remains
		break;
	}
	case 0://SRC
	{
		ISrcViewModel* pIModel(ADC().NewSrcViewModel(path.toUtf8(), DUMP_BLOCKS | DUMP_FUNCEX | DUMP_COLORS | DUMP_FONTS, nullptr));
		Q_ASSERT(pIModel);
		pModelData = rWin.setModel(mModels, pIModel, index);
		pIModel->Release();//1 left
		registerModelData(path, pModelData);
		pModelData->Release();//1 ref remains
		break;
	}
	case 1://BIN
	{
		IBinViewModel* pIModel(ADB().NewBinViewModel(path.toUtf8()));
		if (pIModel)
		{
			pIModel->setColFlags(IBinViewModel::CLMN_OFFS, adcui::CLMNFLAGS_DISPSEL | adcui::CLMNFLAGS_HEADER);
			pIModel->setColFlags(IBinViewModel::CLMN_FILE, adcui::CLMNFLAGS_DISPSEL | adcui::CLMNFLAGS_HEADER);
			//pIModel->setColumnWidth(IBinViewModel::CLMN_TYPES, 16);
			pIModel->setColumnWidth(IBinViewModel::CLMN_NAMES, 16);
			pModelData = rWin.setModel(mModels, pIModel, index);
			pIModel->Release();//1 left
			registerModelData(path, pModelData);
			pModelData->Release();//1 ref remains
		}
		break;
	}
	default:
		Q_ASSERT(0);
		break;
	}
}

void ADCMainWin::slotCaretPosChanged(int nX, int nY, const ADCCurInfo &si)
{
	bool bShown(nX != 0 && nY != 0);
	if (bShown)
	{
		mpCursorDial->show();
		QString s(tr("Line: <font color=darkRed>%1</font>").arg(nY));
		if (!si.scope.isEmpty())
			s.append(QString(" {<font color=darkGreen>%1</font>}").arg(si.scope));
		s.append(" ");
		s.append(tr("Col: <font color=darkRed>%1</font>").arg(nX));
		setStatusSlotText(mpCursorDial, tr("<nobr> %1 </nobr>").arg(s));
	}
	else
		mpCursorDial->hide();

	emit signalCurInfoChanged(si);
}

void ADCMainWin::OnDebuggerStopped()
{
	emit signalExprInvalidated();//redraw a view
}

void ADCMainWin::OnProjectStatus(bool bActive)
{
	ADBMainWin::OnProjectStatus(bActive);
	mpCompileAction->setEnabled(bActive);
}

ADCSourceWin* ADCMainWin::getCurrentSourceWin()
{
	DocumentObject* pDoc(getActiveDoc());
	ADCSourceWin0* pWin(dynamic_cast<ADCSourceWin0*>(pDoc));
	if (!pWin)
		return nullptr;
	return pWin->win();
}

int ADCMainWin::closePage(QWidget* pWin0)
{
	ADCSplitWin* pWin(dynamic_cast<ADCSplitWin*>(pWin0));
	if (pWin)
		pWin->aboutToClose();
	return ADBMainWin::closePage(pWin0);
}

void ADCMainWin::slotRequestModel(ADCExprWin& rWin)
{
	adcui::IADCExprViewModel* pIModel(ADC().NewExprViewModel());
	rWin.setModel(pIModel);
	if (pIModel)
		pIModel->Release();
}

void ADCMainWin::slotRequestModel(ADCInterCodeWin &rWin, QString subjFullName)
{
	using namespace adcui;
	ISrcViewModel* pIModel(ADC().NewSrcViewModel(nullptr, DUMP_UNFOLD | DUMP_DEADCODE | DUMP_DEADLABELS | DUMP_COLORS | DUMP_FONTS, subjFullName.toUtf8().constData()));//from context
//	if (pIModel && !subjFullName.isEmpty())
//		pIModel->setSubject(subjFullName.toUtf8().constData());
	rWin.setModel(pIModel);
	if (pIModel)
		pIModel->Release();
}

void ADCMainWin::slotRequestModel(ADCStubsWin& rWin)
{
	adcui::IStubsViewModel* pIModel(ADC().NewStubsViewModel());
	if (pIModel)
	{
		rWin.setModel(mModels, pIModel);
		pIModel->Release();
	}
}

void ADCMainWin::slotRequestModel(ADCStubsView& rView, QString path)
{
	QString sStem(path);
	sStem.truncate(sStem.lastIndexOf('.'));

	adcui::IStubsViewModel* pIModel(ADC().NewStubsViewModel(sStem.toUtf8()));
	if (pIModel)
	{
		rView.setModel(mModels, pIModel);
		pIModel->Release();
	}
}

void ADCMainWin::slotRequestModel(ADCTemplWin& rWin, QString path, bool bFields)
{
	QString sStem(path);
	sStem.truncate(sStem.lastIndexOf('.'));

	adcui::ITemplViewModel* pIModel(ADC().NewTemplViewModel(sStem.toUtf8(), bFields));
	if (pIModel)
	{
		rWin.setModel(pIModel);
		pIModel->Release();
	}
}

void ADCMainWin::slotRequestModel(ADCTemplView& rView, QString path, bool bFields)
{
	QString sStem(path);
	if (!sStem.isEmpty())
		sStem.truncate(sStem.lastIndexOf('.'));

	adcui::ITemplViewModel* pIModel(ADC().NewTemplViewModel(sStem.toUtf8(), bFields));
	if (pIModel)
	{
		rView.setModel(pIModel);
		pIModel->Release();
	}
}


void ADCMainWin::slotDumpCutList(ADCStream& ss)
{
	ADC().DumpCutList(ss);
}

void ADCMainWin::slotUncutItem(int id)
{
	slotPostCommand(QString("uncut %1").arg(id), false);
}

void ADCMainWin::slotSetProtoData(const ADCProtoEditDlg::Data&aData, bool)// bContinue)
{
	ADCStream ss;
	aData.write(ss);
	ADC().SetProtoInfo(ss);

	//if (bContinue)
		//slotStart();
}

void ADCMainWin::slotDumpBlocks(QString s, ADCStream& ss)
{
	ADC().DumpBlocks(s.toLatin1(), ss);
}

void ADCMainWin::slotShowTemplatesView()
{
	showPageEx(page(PAGEX_TEMPLATES));
}

void ADCMainWin::slotShowStubsView()
{
	showPageEx(page(PAGEX_PROTOTYPES));
}

void ADCMainWin::slotShowBlocksView()
{
	showPageEx(page(PAGEX_BLOCKS));
}

void ADCMainWin::slotShowExprView()
{
	showPageEx(page(PAGEX_EXPR));
}

void ADCMainWin::slotShowCutsView()
{
	showPageEx(page(PAGEX_CUTLIST));
}

void ADCMainWin::slotShowInterView()
{
	showPageEx(page(PAGEX_INTER));
}

void ADCMainWin::showProtoDlg(ADCStream &ss)
{
	QString fileName(ss.ReadString());
	showSourceWin(fileName, "$task", false);

	static ADCProtoEditDlg::DataParser aData;
	aData.parse(ss);
	emit signalSetProtoData(aData);
	mpProtoDlg->show();
}

void ADCMainWin::slotShowProtoDlg()
{
	ADCStream ss;
	if (ADC().GetProtoInfo(ss))
		showProtoDlg(ss);
	//else
		//mpProtoDlg->show();
}

void ADCMainWin::slotActivateFile(QString path)
{
	//slotPostCommand(QString("show %1").arg(path), false);//click
	slotShowSourceWin(path, QString());
}

void ADCMainWin::slotTabChanged(QWidget* pWin0)
{
	DocumentObject* pDoc(dynamic_cast<DocumentObject*>(pWin0));
	if (!pDoc)
		return;

	QString fileName(unfixPath(pDoc->path()));

#if(NEW_SPLITPANE)
	int fileKind(toFileKind(fileName));
	ADCSourceWin0* pWin(dynamic_cast<ADCSourceWin0*>(pWin0));
	if (pWin)
	{
		if (!pWin->win())
		{
			if (fileKind != 0)
				for (QList<DocumentObject*>::Iterator i(DocMgr().begin()); i != DocMgr().end(); i++)
				{
					DocumentObject* pDoc(*i);
					ADCSourceWin0* pWin2(dynamic_cast<ADCSourceWin0*>(pDoc));
					if (pWin2 && pWin2 != pWin)
					{
						//take contents from win2 and give it to win0
						if (pWin2->moveWinTo(pWin))
						{
							pWin->win()->setFileName(fileName);
							int fileId(ADC().GetFileIdByName(fileName.toUtf8()));
							pWin->win()->init(fileId, fileKind);
							pWin->show();
							break;
						}
					}
				}

			if (!pWin->win())
			{
				ADCSourceWin* pWin3(CreateSourceWin(pWin));
				pWin->setWin(pWin3);
				pWin3->setFileName(fileName);
				int fileId(ADC().GetFileIdByName(fileName.toUtf8()));
				if (fileId < 0)
					return;//?
				pWin3->init(fileId, fileKind);
				//pDoc->updateOutputFont(UPREFS.getOutputFont());
			}
		}
	}
/*	else
	{
		ADCBinWin* pWin2(dynamic_cast<ADCBinWin*>(pWin0));
		if (pWin2)
		{
			//pWin->activated();
		}
	}*/
#endif
	emit signalCurFileChanged(fileName);

	updateSplitPanesUI();
}

void ADCMainWin::slotTabChanged(DocumentObject*)
{
}

void ADCMainWin::slotJumpSourceBack(bool bFlipFwd)
{
	if (!ADC().JumpSourceBack(bFlipFwd ? "jumpback" : "jump"))
		QApplication::beep();
}

void ADCMainWin::slotJumpSourceBack()
{
	if (!ADC().JumpSourceBack("jumpback"))
		QApplication::beep();
}

void ADCMainWin::slotJumpSourceForward()
{
	if (!ADC().JumpSourceForward("jumpfwd"))
		QApplication::beep();
}

void ADCMainWin::showSourceWin(QString fileName, QString atHint, bool bAskToOpen)
{
	switch (CheckSpecialFile(fileName))
	{
	default:
		break;
	case adcui::FOLDERTYPE_FILE_TT:
		slotShowTemplatesView();
		emit signalOpenTemplatesView(fileName);
		return;
		//cl_name = "ADC_TYPESWIN";
		//break;
	case adcui::FOLDERTYPE_FILE_STUB:
		slotShowStubsView();
		emit signalOpenStubsView(fileName);
		return;
	}
	ADBMainWin::showSourceWin(fileName, atHint, bAskToOpen);
}

void ADCMainWin::OnDecompileFunction(QString sFile, QString sFunc)
{
	DocumentObject* pDoc(DocMgr().findDoc(sFile, QString()));
	if (pDoc)
	{
		ADCSourceWin0* pWin(dynamic_cast<ADCSourceWin0*>(pDoc));
		if (pWin)
			pWin->assureSourcePane(sFunc);
	}
}
void ADCMainWin::OnSrcDumpInvalidated(QString fileName)
{
    //fileName = QDir::toNativeSeparators(fileName);
    //CHECK_QSTRING(fileName, z);

    QString fileName2(fixPath(fileName));

#if QT_VERSION_MAJOR >= 6
    // Qt 6: QRegExp is gone, use QRegularExpression + wildcard conversion
    QRegularExpression rx(
        QRegularExpression::wildcardToRegularExpression(fileName2)
    );
#else
    // Qt 5
    QRegExp rx(fileName2);
    rx.setPatternSyntax(QRegExp::WildcardUnix);
#endif

    for (int i(0); i < getDocsCount(); i++)
    {
        DocumentObject* pDoc(getDoc(i));
        if (pDoc)
        {
            ADCSourceWin0* pWin(dynamic_cast<ADCSourceWin0*>(pDoc));
            if (pWin)
            {
                QString s(KEY2PATH(pDoc->mstrID));
                //CHECK_QSTRING(s, z2);

#if QT_VERSION_MAJOR >= 6
                QRegularExpressionMatch m = rx.match(s);
                if (m.hasMatch())
                {
                    pWin->redump();
                }
                else
                {
                    pWin->updateContents();
                }
                int l = m.capturedLength();
                (void)l;
#else
                if (rx.exactMatch(s))
                    pWin->redump();
                else
                    pWin->updateContents();
                int l = rx.matchedLength();
                (void)l;
#endif
            }
        }
    }

    // let docked/floating window(s) know about change
    emit signalSrcDumpInvalidated(fileName);
}


void ADCMainWin::slotNoSourceContextRecoil(QString s)
{
    ADCSourceWin* pWin(getCurrentSourceWin());
    if (!pWin)
    {
        QMessageBox::critical(
            this,
            tr("Error"),
            tr("The requested operation requires a source file context:\n%1").arg(s),
            QMessageBox::Ok
        );
        return;
    }

    s.append(QString(" -src %1").arg(pWin->fileName()));
    slotPostCommand(s); // re-post
}


void ADCMainWin::slotSyncSourcePanesRequest(int nLine, bool bForce)
{
	if (!mpSyncModeAction->isChecked() && !bForce)
		return;

	//from here a request is turned into a responce!

	/*	if (nLine == 666)//sync request from other kind of window (e.g: resources)
		{
			emit signalSync PanesResponce(0, true);//position at line 1
			return;
		}*/

	emit signalSyncPanesResponce(nLine, bForce);
	emit signalSourceDumpPicked(bForce);
}

void ADCMainWin::slotCompileFile(QString s)
{
#if(1)
	slotPostCommand(QString("compile %1").arg(s));
#else
	QProcess proc;
	QString program("C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\bin\\cl.exe");
	QStringList args;
	proc.start(program, args);
#endif
}

void ADCMainWin::slotCompile()
{
	DocumentObject* pDoc(getActiveDoc());
	if (pDoc)
	{
		ADCSourceWin0* pWin(dynamic_cast<ADCSourceWin0*>(pDoc));
		if (pWin)
			slotCompileFile(unfixPath(pDoc->path()).trimmed());
	}
}

void ADCMainWin::slotSyncModeInquiry(bool& b)
{
	b = mpSyncModeAction->isChecked();
}

void ADCMainWin::slotSyncModeToggled(bool)
{
}

void ADCMainWin::updateSplitPanesUI()
{
	ADCSplitWin* pWin1(getCurrentSourceWin());
	mpSplitPaneLeftAction->setEnabled(pWin1 != nullptr && (!pWin1->isLeftPaneOn() || pWin1->isRightPaneOn()));
	mpSplitPaneRightAction->setEnabled(pWin1 != nullptr && (!pWin1->isRightPaneOn() || pWin1->isLeftPaneOn()));
}

void ADCMainWin::slotToggleLeftPane()
{
	ADCSplitWin* pWin1(getCurrentSourceWin());
	if (pWin1)//?
		pWin1->toggleLeftPane();
	updateSplitPanesUI();
}

void ADCMainWin::slotToggleRightPane()
{
	ADCSplitWin* pWin1(getCurrentSourceWin());
	if (pWin1)//?
		pWin1->toggleRightPane();
	updateSplitPanesUI();
}

void ADCMainWin::slotBinaryPanePicked(int nLine, bool bForce)
{
	(void)nLine;
	if (!mpSyncModeAction->isChecked() && !bForce)
		return;

	//emit signalBinaryDumpPicked();
	emit signalSyncPanesResponce(nLine, bForce);
}

static QString truncExt(QString& s)
{
	for (int n(s.length() - 1); n >= 0; n--)
	{
		if (s[n] == '\\' || s[n] == '/')
			break;
		if (s[n] == '.')
		{
			QString ext(s.mid(n));
			s.truncate(n);
			return ext;
		}
	}
	return "";
}

void ADCMainWin::OnFileRenamed(QString sOld, QString sNew)
{
	emit signalFolderRenamed(sNew);//make it current in files view

	QString extOld(truncExt(sOld));
	QString extNew(truncExt(sNew));//new name may come with new extention

	QList<DocumentObject*>::ConstIterator i;
	for (i = DocMgr().constBegin(); i != DocMgr().constEnd(); i++)
	{
		DocumentObject* pDoc(*i);
		if (pDoc)
		{
			QString path(KEY2PATH(pDoc->mstrID));
			QString title(KEY2TITLE(pDoc->mstrID));

			//suppress doc's extention
			QString ext(truncExt(path));

			if (path == sOld)
			{
				ADCSourceWin0* pWin(dynamic_cast<ADCSourceWin0*>(pDoc));
				if (pWin)
				{
					if (ext.isEmpty() && !extNew.isEmpty())
						ext = extNew;//incase if a binary was transformed into a source

					if (renameDoc(pDoc, sNew + ext, title))
					{
						updateIcon(pWin, toIconStr(ext));
						pWin->updateActions(toFileKind(ext));
					}
				}
			}
		}
	}
}


void ADCMainWin::OnShowDocumentAtHint(DocumentObject* pDoc, QString atHint)
{
	/*if (atHint == "$task")//open a binary pane
	{
		ADCSourceWin0* pWin(dynamic_cast<ADCSourceWin0*>(pDoc));
		if (pWin)
			pWin->locatePosition(atHint);
	}*/
	if (atHint == "locus")
	{
		ADCSourceWin0* pWin(dynamic_cast<ADCSourceWin0*>(pDoc));
		if (pWin)
		{
			if (pDoc->path().endsWith(HEADER_EXT))
				pWin->goToDeclarationRequest(false);
			else if (pDoc->path().endsWith(SOURCE_EXT))
				pWin->goToDeclarationRequest(true);
		}
	}
	else if (atHint.startsWith("jump"))
	{
		int linesFromTop(-1);
		int n(atHint.indexOf('('));
		if (n > 0)
		{
			int m(atHint.indexOf(')'));
			if (m > n)
				linesFromTop = atHint.mid(n + 1, m - n - 1).toInt();
		}
		ADCSourceWin0* pWin(dynamic_cast<ADCSourceWin0*>(pDoc));
		if (pWin)
		{
			bool bFwd(atHint.startsWith("jumpfwd"));
			bool bFlip(bFwd || atHint.startsWith("jumpback"));
			pWin->goToDeclarationRecoil(linesFromTop, bFwd, bFlip);
		}
	}
	else
		ADBMainWin::OnShowDocumentAtHint(pDoc, atHint);
}

int ADCMainWin::HandleEvent(int msgID, void* pData)
{
	using namespace adcui;
	switch (msgID)
	{
	default:
		return ADBMainWin::HandleEvent(msgID, pData);

	case UIMSG_SHOW_VIEW:
	{
		SpiceStream2 ss(*(MyStreamBase*)pData);
		QString fileName(ss.ReadString());
		QString atHint(ss.ReadString());
		bool bAskToOpen(false);
		ss.ReadBool(&bAskToOpen);
		if (fileName == "?STUBS")
		{
			slotShowStubsView();
			if (atHint == "$task")
				emit signalGoToTaskTop();
		}
		else
			showSourceWin(fileName, atHint, bAskToOpen);
		break;
	}

	case UIMSG_DC_NEW:
		emit signalDcNew();
		//emit signalStubsModified();
		break;

	case UIMSG_STUBS_LIST_MODIFIED:
		emit signalStubsModified();
		break;

	case UIMSG_EDIT_STUB:
	{
		//SpiceStream2 ss(*(MyStreamBase *)pData);
		//QString s;// (ss.ReadString());
		emit signalEditStub();
		break;
	}

	case UIMSG_UNDEFINED_PROTO:
	{
		SpiceStream2 ss(*(MyStreamBase*)pData);
		showProtoDlg(ss);
		break;
	}

	case UIMSG_CUTLIST_UPDATED:
		slotShowCutsView();
		emit signalCutListUpdated();
		break;

	case UIMSG_DISPLAY_OPENED:
	{
		assert(0);
		/*SpiceStream2 ss(*(MyStreamBase *)pData);
		int fileId;
		ss.ReadInt(&fileId);
		QString fileName;
		ss.ReadUString(fileName);
		//emit signalFileInvalidated(fileId);
		DocumentObject *pDoc(SxMainWindow::openDoc("ADC_SOURCEWIN", QString(), fileName));
		ADCSourceWin *pWin(dynamic_cast<ADCSourceWin *>(pDoc));
		pWin->setModels(ADB().GetSrcViewModel(fileId), ADB().GetSrcViewModel(fileId));*/

	}
	break;

	case UIMSG_FILE_RENAMED:
	{
		SpiceStream2 ss(*(MyStreamBase*)pData);
		QString sOld(ss.ReadString());
		QString sNew(ss.ReadString());
		OnFileRenamed(sOld, sNew);
		//emit signalFileInvalidated(fileId);
	}
	break;

	case UIMSG_FILE_PRIOR_REMOVED:
	{
		//int fileId(*(int *)pData);
		SpiceStream2 ss(*(MyStreamBase*)pData);
		QString fileName;
		ss.ReadUString(fileName);
		for (int i(0); i < getDocsCount(); i++)
		{
			DocumentObject* pDoc(getDoc(i));
			ADCSourceWin* pWin(dynamic_cast<ADCSourceWin*>(pDoc));
			if (pWin)
			{
				QString s(unfixPath(pDoc->path()));
				//z = s.toStdString();
				int n(s.lastIndexOf(QChar('.')));
				if (n > 0)
					s.truncate(n);
				//z = s.toStdString();
				if (s == fileName)
					closePage(pWin);
			}
		}

		break;
	}

	//case UIMSG_FILE_MODIFIED:
	case UIMSG_SRC_DUMP_INVALIDATED:
	{
		SpiceStream2 ss(*(MyStreamBase*)pData);
		QString s(ss.ReadString());
		OnSrcDumpInvalidated(s);
		//emit signalFileInvalidated(fileId);
	}
	break;

	case UIMSG_EXPR_MODIFIED:
		emit signalExprInvalidated();
		break;

	case UIMSG_DECOMPILE_FUNCTION:
	{
		SpiceStream2 ss(*(MyStreamBase*)pData);
		QString sFile(ss.ReadString());
		QString sFunc(ss.ReadString());
		OnDecompileFunction(sFile, sFunc);
	}
	break;

	case UIMSG_CUROP_CHANGED:
		emit signalCurOpChanged();
		break;

	case UIMSG_DECOMP_RECOIL:
	{
		SpiceStream2 ss(*(MyStreamBase*)pData);
		slotNoSourceContextRecoil(ss.ReadString());
		break;
	}
	break;

	}

	return 1;
}
