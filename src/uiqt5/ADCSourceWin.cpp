#pragma warning(disable: 4100)//C4100 (unreferenced formal parameter)
#include <assert.h>

#include <QtCore/QSignalMapper>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QCursor>
#include <QtGui/QWheelEvent>
#include <QScrollBar>
#include <QLayout>
#include <QStyle>
#include <QToolButton>
#include <QAction>
#include <QMenu>
#include <QLabel>
#include <QStyleOption>
#include <QSplitter>
#include <QLineEdit>
#include <QToolTip>
#include <QApplication>

//#include "sx/SxDocument.h"
//#include "sx/SxSignalMultiplexer.h"
#include "sx/SxFindTextDlg.h"
#include "ADCSourceWin.h"
#include "ADCBinWin.h"
#include "ADCMainWin.h"
#include "ADCProtoDlg.h"
#include "interface/IADCGui.h"
#include "colors.h"

using namespace adcui;

ADCSourceView::FindTextObject::FindTextObject(ADCSourceView* self)
	: SxFindTextDlg::ITarget(self),
	mpSelf(self)
{
}

bool ADCSourceView::FindTextObject::findIt(const QString& what, bool cs, bool wo, bool rev, bool reset) const
{
	return mpSelf->findText(ISrcViewModel::CLMN_CODE, what, cs, wo, rev, reset);
}

ADCSourceView::ADCSourceView(QWidget* parent, const char* name)
	: ADCSourceViewBase(parent, name),//, WNoAutoErase|WState_BlockUpdates),
	mFileId(-1),
	mpModelData(nullptr),
	muContextId(0)
{
	clearActions();
	setAutoFillBackground(false);
	
//	setBackgroundRole(QPalette::Base);
  //  setAutoFillBackground(true);

	connect(this, SIGNAL(signalRefresh()), SLOT(slotRefresh0()));
		//connect(mpHorzScrollBar, SIGNAL(sliderMoved(int)), SLOT(slotHorzSliderMoved(int)));
	//connect(mpHorzScrollBar, SIGNAL(valueChanged(int)), SLOT(slotHSliderValueChanged(int)));

		//connect(mpVertScrollBar, SIGNAL(sliderMoved(int)), SLOT(slotVertSliderMoved(int)));
	//connect(mpVertScrollBar, SIGNAL(valueChanged(int)), SLOT(slotVSliderValueChanged(int)));

//	QToolButton * pButton = new QToolButton(this);
//	pButton->setAutoRaise( true );

//?	setBackgroundMode( Qt::NoBackground );

	createActions();

	//viewport()->setMouseTracking(true);//set in base class
	
	mbHighlightCaret = true;
	
	mMargin = QSize(-1, 0);
	mMarginExtra = QSize(2, 0);

	mPixmaps.push_back(QPixmap(":dbg_next.png"));
	mPixmaps.push_back(QPixmap(":dbg_bp.png"));
	mPixmaps.push_back(QPixmap(":dbg_bp_next.png"));
	mPixmaps.push_back(QPixmap(":dbg_next_hazy.png"));
}

void ADCSourceView::createContents()
{
	deleteContents();
	ADCSourceViewBase::createContents();
	QMargins margins(viewport()->contentsMargins());
	margins.setTop(0);
	viewport()->ensurePolished();
	viewport()->setContentsMargins(margins);
}

void ADCSourceView::closeEvent(QCloseEvent *e)
{
	e->ignore();
}

adcui::ISrcViewModel *ADCSourceView::reset1(QString subjFullName, unsigned)
{
	if (subjFullName.isEmpty())
	{
		setModelData(nullptr);
	}
	else
	{
		if (hasModel())
		{
			//do not re-create model if the subjects is contained in associated file
			if (srcModel()->setSubject(subjFullName.toLatin1().constData()))
			{
				redumpModel(true);
				return srcModel();
			}
			setModelData(nullptr);
		}
		emit signalRequestModel(*this, subjFullName);
		updateContents();
		if (hasModel())
			return srcModel();
	}
	return nullptr;
}

void ADCSourceView::setModelData(ADCModelData *pModelData)//adcui::ISrcViewModel *pIModel)
{
	//Q_ASSERT(!mpModelData);//weak
	mpModelData = pModelData;
	if (mpModelData)
	{
		createContents();
		syncViewportMargins();
		updateActions();
		OnContentsChanged();
	}
	else
	{
		deleteContents();
		//setBackgroundRole(QPalette::Base);
		//setAutoFillBackground(true);
	}
}

ADCSourceView::~ADCSourceView()
{
	//spFindTextDlg = nullptr;
}

/*void ADCSourceView::enableDisassemblyView()
{
	if (!mpViewDisassemblyAction)
	{
		mpViewDisassemblyAction = new QAction(tr("View Disassembly"), this);
		connect(mpViewDisassemblyAction, SIGNAL(triggered()), SLOT(slotViewDisassembly()));
	}
}*/

void ADCSourceView::clearActions()
{
	mpNameAction = nullptr;

	mpMakeIntAction = nullptr;
	mpMakeRealAction = nullptr;
	mpMakePointerAction = nullptr;
	mpMakeConstAction = nullptr;
	mpUntypeAction = nullptr;
	mpMakeArrayAction = nullptr;
	mpApplyTypeAction = nullptr;
	mpMakeGapAction = nullptr;

	mpDecompileAction = nullptr;
	mpAnalyzeAction = nullptr;
	mpQuickProtoAction = nullptr;
	mpRootAction = nullptr;
	mpIfAction = nullptr;
	mpElseAction = nullptr;
	mpWhileAction = nullptr;
	mpLogicAction = nullptr;
	mpLogicUndoAction = nullptr;
	mpExpandAction = nullptr;
	mpCollapseAction = nullptr;
	mpFlipAction = nullptr;
	mpSwitchAction = nullptr;
	mpBindAction = nullptr;
	mpUnbindAction = nullptr;
	mpAcquireAction = nullptr;
	//mpAcquireConstAction = nullptr;
	mpCutAction = nullptr;
	mpPasteAction = nullptr;
	mpDeleteAction = nullptr;
	//mpCancelAction = nullptr;
	mpSaveListingAction = nullptr;
	mpForAction = nullptr;
	//mpViewDisassemblyAction = nullptr;
	//mpViewToggleProfileAction = nullptr;
	mpFlatViewAction = nullptr;
	mpShowXRefsAction = nullptr;
	mpToggleInflowAction = nullptr;
	mpToggleOutflowAction = nullptr;
	mpToggleLostCodeAction = nullptr;
	mpToggleArgsAction = nullptr;
	mpToggleLostLabelsAction = nullptr;
	mpToggleLinesAction = nullptr;
	mpToggleStackAction = nullptr;
	mpToggleFpuAction = nullptr;
	mpTogglePathsAction = nullptr;
	mpRefreshAction = nullptr;

#ifdef ENABLE_DEBUG_CONTROLS
	mpDumpExprAction = nullptr;
	mpShowStrucLocsAction = nullptr;
	mpPrintObjInfoAction = nullptr;
	mpPrintDumpInfoAction = nullptr;
	mpToggleDraftModeAction = nullptr;
	mpToggleVarOpsAction = nullptr;
	mpTestAction = nullptr;
#endif
}

class MyAction : public QAction
{
#ifdef _DEBUG
	QString mName;
#endif
	ADCSourceView::ScopingFlags m_scoping;
public:
	MyAction(QString name, const char *slot, const QKeySequence &key, const QIcon &icon, QWidget *parent, bool bToggle)
		: QAction(icon, name, parent),
#ifdef _DEBUG
		mName(name),
#endif
		m_scoping(ADCSourceView::SCOPING_NULL)
	{
		setShortcut(key);
#if QT_VERSION >= 0x050A00//05.10.00
		setShortcutVisibleInContextMenu(true);
#endif
		setShortcutContext(Qt::WidgetWithChildrenShortcut);
		if (bToggle)
		{
			setCheckable(true);
			parent->connect(this, SIGNAL(toggled(bool)), parent, slot);
		}
		else
			parent->connect(this, SIGNAL(triggered()), parent, slot);
	}
	void setScoping(ADCSourceView::ScopingFlags e){ m_scoping = e; }
	ADCSourceView::ScopingFlags scoping() const { return m_scoping; }
};

QAction *ADCSourceView::newAction(QString name, const char *slot, const QKeySequence &key, const QIcon &icon, QWidget *parent)
{
	mActions.append(new MyAction(name, slot, key, icon, parent ? parent : this, false));
	return mActions.back();
}

QAction *ADCSourceView::newActionCheck(QString name, const char *slot, const QKeySequence &key, const QIcon &icon, QWidget *parent)
{
	mActions.append(new MyAction(name, slot, key, icon, parent ? parent : this, true));
	return mActions.back();
}

QAction *ADCSourceView::newAction_F(QString name, const char *slot, const QKeySequence &key, const QIcon &icon, ScopingFlags scoping)
{
	MyAction *p(new MyAction(name, slot, key, icon, this, false));
	mActions.append(p);
	p->setScoping(scoping);
	return p;
}

/*QAction *ADCSourceView::newAction_S(QString name, const char *slot, const QKeySequence &key, const QIcon &icon, QWidget *parent)
{
	MyAction *p(new MyAction(name, slot, key, icon, parent ? parent : this, false));
	mActions.append(p);
	p->setScoping(SCOPING_STRUC_BODY);
	return p;
}

QAction *ADCSourceView::newAction_SH(QString name, const char *slot, const QKeySequence &key, const QIcon &icon, QWidget *parent)
{
	MyAction *p(new MyAction(name, slot, key, icon, parent ? parent : this, false));
	mActions.append(p);
	p->setScoping(SCOPING_STRUC_HEADER);
	return p;
}*/

void ADCSourceView::createActions()
{
	//mpViewToggleProfileAction = newAction(tr("Toggle Profile"), SLOT(slotToggleProfile()), tr("Shift+S"));
	mpFlatViewAction = newActionCheck(tr("Flat View (No Code Structuring)"),  SLOT(slotViewBlocked(bool)), tr("Shift+F"), QIcon(":flat_list.png"));//shift_down
	mpShowXRefsAction = newActionCheck(tr("Show Data References"), SLOT(slotShowXRefs(bool)), QKeySequence("Shift+J"), QIcon(":downflow_16.png"));
	mpToggleInflowAction = newActionCheck(tr("Show Data Inflow"), SLOT(slotShowXDepsIn(bool)), tr("Shift+I"), QIcon(":inflow_16.png"));
	mpToggleOutflowAction = newActionCheck(tr("Show Data Outflow"), SLOT(slotShowXDepsOut(bool)), tr("Shift+O"), QIcon(":outflow_16.png"));
	mpToggleLostCodeAction = newActionCheck(tr("Show 'Lost' Code"), SLOT(slotShowDeadCode(bool)), tr("Shift+L"), QIcon(":lost_16.png"));
	mpToggleArgsAction = newActionCheck(tr("Show All Arguments"), SLOT(slotToggleArguments(bool)), tr("Shift+A"), QIcon(":toggle_args.png"));
	mpToggleLostLabelsAction = newActionCheck(tr("Including Degenerate Labels"), SLOT(slotShowDeadLabels(bool)));//, tr(""), QIcon(":.png"));

	mpNameAction = newAction(tr("Edit Name"), SLOT(slotEditName()), Qt::Key_N, QIcon(":document_names.png"));
	mpGotoDefinitionAction = newAction(tr("Go To Definition"), SLOT(slotGoToDefinition()));
	mpGotoDeclarationAction = newAction(tr("Go To Declaration"), SLOT(slotGoToDeclaration()));
	mpGotoLineAction = newAction(tr("Go To Line..."), SLOT(slotGoToLine()), tr("Ctrl+G"), QIcon(":go_page.png"));
	mpFindAction = newAction(tr("Find Text..."), SLOT(slotFind()), tr("Ctrl+F"), QIcon(":find_24.png"));
	mpGoBackAction = newAction(tr("Go To Back"), SLOT(slotGoBack()), Qt::Key_Escape);

	//order preserved in popup menu
	mpMakeIntAction = newAction_F(tr("Make Integer"), SLOT(slotMakeInt()), Qt::Key_D, QIcon(), SCOPING_STRUC_DATA);
	mpMakeRealAction = newAction_F(tr("Make Real"), SLOT(slotMakeReal()), Qt::Key_G, QIcon(), SCOPING_STRUC_DATA);
	mpMakePointerAction = newAction_F(tr("Make Pointer"), SLOT(slotMakePointer()), Qt::Key_P, QIcon(), SCOPING_STRUC_DATA);
	mpMakeConstAction = newAction_F(tr("Make Const"), SLOT(slotMakeConst()), Qt::Key_C, QIcon(), SCOPING_STRUC_DATA);
	mpUntypeAction = newAction_F(tr("Untype"), SLOT(slotUntype()), Qt::Key_X, QIcon(), SCOPING_STRUC_DATA);
	mpMakeArrayAction = newAction_F(tr("Make Array"), SLOT(slotMakeArray()), Qt::Key_A, QIcon(), SCOPING_STRUC_DATA);
	mpApplyTypeAction = newAction_F(tr("Apply Type"), SLOT(slotApplyType()), Qt::Key_T, QIcon(), SCOPING_STRUC_DATA);
	mpMakeGapAction = newAction_F(tr("Make Gap"), SLOT(slotMakeGap()), Qt::Key_M, QIcon(), SCOPING_STRUC_DATA);

	mpHandleRuntimeTypeInfoAction = newAction_F(tr("Reconstruct Class Hierarchy"), SLOT(slotReconstructClassHierarchy()), QKeySequence(), QIcon(), SCOPING_STRUC_HEADER);

	mpAddPublicAction = newAction_F(tr("Add Inheritance"), SLOT(slotInheritPublic()), Qt::Key_X, QIcon(), SCOPING_STRUC_HEADER);
	//mpAddProtectedAction = newAction_F(tr("Add Protected Inheritance"), SLOT(slotInheritProtected()), Qt::Key_, QIcon(), SCOPING_STRUC_HEADER);
	//mpAddPrivateAction = newAction_F(tr("Add Private Inheritance"), SLOT(slotInheritPrivate()), Qt::Key_, QIcon(), SCOPING_STRUC_HEADER);
	mpLowerInherLevelAction = newAction_F(tr("Remove Inheritance"), SLOT(slotDisinherit()), Qt::Key_U, QIcon(), SCOPING_STRUC_HEADER);
	mpToggleVFTablePtrAction = newAction_F(tr("Toggle V-Table Pointer"), SLOT(slotToggleVFTablePtr()), Qt::Key_V, QIcon(), SCOPING_STRUC_HEADER);

	mpMakeNamspaceAction = newAction(tr("Namespace"), SLOT(slotConvertNamespace()));
	mpMakeClassAction = newAction(tr("Class"), SLOT(slotConvertClass()));
	mpMakeStructAction = newAction(tr("Structure"), SLOT(slotConvertStruct()));
	mpMakeEnumAction = newAction(tr("Enumeration"), SLOT(slotConvertEnum()));
	mpMakeUnionAction = newAction(tr("Union"), SLOT(slotConvertUnion()));

	//order preserved in popup menu
	mpDecompileAction = newAction_F(tr("Decompile"), SLOT(slotDecompile()), Qt::Key_F4, QIcon(":wrench.16.png"), SCOPING_FUNC__MASK);
	mpAnalyzeAction = newAction_F(tr("Analyze"), SLOT(slotAnalyze()), Qt::Key_F4, QIcon(":wrench.16.png"), SCOPING_STRUC__MASK);
	mpQuickProtoAction = newAction_F(tr("Quick Prototype"), SIGNAL(signalQuickPrototype()), Qt::Key_Q, QIcon(":type_edit4.png"), SCOPING_FUNC__MASK);
	mpRootAction = newAction_F(tr("Propagate/Retract"), SLOT(slotToggleRoot()), Qt::Key_R, QIcon(), SCOPING_FUNC_OP);//Uproot/Replant
	mpIfAction = newAction_F(tr("Toggle If Block"), SLOT(slotToggleIf()), Qt::Key_B, QIcon(), SCOPING_FUNC_OP);
	mpElseAction = newAction_F(tr("Toggle Else Block"), SLOT(slotToggleElse()), Qt::Key_E, QIcon(), SCOPING_FUNC_OP);
	mpSwitchAction = newAction_F(tr("Toggle Switch"), SLOT(slotToggleSwitch()), Qt::Key_S, QIcon(), SCOPING_FUNC_OP);
	mpWhileAction = newAction_F(tr("Toggle While Loop"), SLOT(slotToggleWhile()), Qt::Key_W, QIcon(), SCOPING_FUNC_OP);
	mpForAction = newAction_F(tr("Toggle For Loop"), SLOT(slotForLoop()), Qt::Key_F, QIcon(), SCOPING_FUNC_OP);
	mpLogicAction = newAction_F(tr("Couple Logics"), SLOT(slotCombineLogic()), Qt::Key_L, QIcon(), SCOPING_FUNC_OP);
	mpLogicUndoAction = newAction_F(tr("Uncouple Logics"), SLOT(slotKillLogic()), Qt::Key_K, QIcon(), SCOPING_FUNC_OP);
	mpFlipAction = newAction_F(tr("Flip Condition"), SLOT(slotFlip()), Qt::Key_I, QIcon(), SCOPING_FUNC_OP);
	mpCollapseAction = newAction_F(tr("Collapse"), SLOT(slotCollapse()), Qt::Key_Y, QIcon(), SCOPING_FUNC_OP);
	mpExpandAction = newAction_F(tr("Expand"), SLOT(slotExpand()), Qt::Key_X, QIcon(), SCOPING_FUNC_OP);
	mpBindAction = newAction_F(tr("Bind"), SLOT(slotBind()), Qt::Key_D, QIcon(), ScopingFlags(SCOPING_FUNC_OP | SCOPING_FUNC_LOCAL));
	mpUnbindAction = newAction_F(tr("Unbind"), SLOT(slotUnbind()), Qt::Key_U, QIcon(), ScopingFlags(SCOPING_FUNC_OP | SCOPING_FUNC_LOCAL));
	mpAcquireAction = newAction_F(tr("Acquire"), SLOT(slotAcquire()), Qt::Key_G, QIcon(), SCOPING_FUNC_OP);
	//mpAcquireConstAction = newAction_F(tr("Acquire Constant"), SLOT(slotAcquireConstant()), Qt::Key_C, QIcon(), SCOPING_FUNC_OP);
	//mpRedecompileAction = newAction_F(tr("Re-decompile"), SLOT(slotRedecompile()), tr("Ctrl+F4"), QIcon(), SCOPING_FUNC__MASK);
	mpToggleThisPtrAction = newAction_F(tr("Toggle This Pointer"), SLOT(slotToggleThisPtr()), Qt::Key_J, QIcon(), SCOPING_STRUC_METHOD);
	mpToggleVirtualAction = newAction_F(tr("Toggle Method Virtual"), SLOT(slotToggleMethodVirtual()), Qt::Key_Z, QIcon(), SCOPING_STRUC_METHOD);

	mpCutAction = newAction(tr("Cut"), SLOT(slotCut()), tr("Ctrl+X"), QIcon(":cut_16.png"));
	mpPasteAction = newAction(tr("Paste"), SLOT(slotPaste()), tr("Ctrl+V"), QIcon(":paste_16.png"));
	mpDeleteAction = newAction(tr("Delete"), SLOT(slotDelete()), Qt::Key_Delete, QIcon(":delete_16.png"));
//	mpCancelAction = newAction(tr("Cancel"), SLOT(slotCancel()), Qt::Key_Escape);
	mpSaveListingAction = newAction(tr("Save Listing..."), SLOT(slotSaveListing()), QKeySequence(), QIcon(":diskette.png"));//save_listing
	mpSyncAction = newAction(tr("Synchronize"), SLOT(slotForceSynchronize()), Qt::Key_Space);

	//mpDecompileAction = newAction(tr("&Decompile Function"), SLOT(slotDecompile()), Qt::Key_F5, QIcon(":wrench.16.png"));
	//mpSyncAction = newAction(tr("Synchronize Auxiliary Pane"), SLOT(slotSynchronize()), Qt::Key_Space, QIcon(":clock.png"));

#ifdef ENABLE_DEBUG_CONTROLS
	mpDumpExprAction = newAction_F(tr("[DEBUG] Dump Expression"), SLOT(slotDumpExpression()), QKeySequence(), QIcon(), SCOPING_FUNC_OP);
	mpShowStrucLocsAction = newActionCheck(tr("[DEBUG] Show Locals Layout"), SLOT(slotShowStrucLocs(bool))/*, tr("Shift+X")*/);
	mpPrintObjInfoAction = newAction(tr("[DEBUG] Print Object Info"), SLOT(slotPrintObjInfo()));
	mpPrintDumpInfoAction = newAction(tr("[DEBUG] Print Dump Info"), SLOT(slotPrintDumpInfo()));
	mpToggleDraftModeAction = newAction(tr("[DEBUG] Toggle Draft Mode"), SLOT(slotToggleDraftMode()));
	mpToggleVarOpsAction = newAction(tr("[DEBUG] Toggle VarOps"), SLOT(slotToggleVarOps()));
	mpTestAction = newAction(tr("[DEBUG] Test"), SLOT(slotTest()), tr("Ctrl+Alt+T"));
#endif

	enableContextActions(true);

	///////////////////////////////////////////////////// toggle columns
	QSignalMapper *signalMapper = new QSignalMapper(this);
	connect(signalMapper, SIGNAL(mapped(int)), SLOT(slotToggleColumn(int)));

	mpToggleLinesAction = new QAction(QIcon(":toggle_lines.png"), tr("Toggle Lines"), this);
	mpToggleLinesAction->setCheckable(true);
	connect(mpToggleLinesAction, SIGNAL(triggered()), signalMapper, SLOT(map()));
	signalMapper->setMapping(mpToggleLinesAction, 0);

	mpToggleStackAction = new QAction(QIcon(":toggle_stack.png"), tr("Toggle Stack"), this);
	mpToggleStackAction->setCheckable(true);
	connect(mpToggleStackAction, SIGNAL(triggered()), signalMapper, SLOT(map()));
	signalMapper->setMapping(mpToggleStackAction, 1);

	mpToggleFpuAction = new QAction(QIcon(":toggle_fpu.png"), tr("Toggle FPU Stack"), this);
	mpToggleFpuAction->setCheckable(true);
	connect(mpToggleFpuAction, SIGNAL(triggered()), signalMapper, SLOT(map()));
	signalMapper->setMapping(mpToggleFpuAction, 2);

	mpTogglePathsAction = new QAction(QIcon(":toggle_paths.png"), tr("Toggle Paths"), this);
	mpTogglePathsAction->setCheckable(true);
	connect(mpTogglePathsAction, SIGNAL(triggered()), signalMapper, SLOT(map()));
	signalMapper->setMapping(mpTogglePathsAction, 3);

	mpRefreshAction = new QAction(QIcon(":refresh.png"), tr("Refresh"), this);
	connect(mpRefreshAction, SIGNAL(triggered()), SLOT(slotRefresh()));
}

void ADCSourceView::emitSynchronize(bool bForce)
{
	//emit signalEnableSynchronize();

	int iPos(0);
	for (QWidget *p(this); p != nullptr; p = p->parentWidget())
		iPos += p->pos().y();
	iPos += (curLine() + topMargin()) * lineHeight();

	emit signalSynchronize(iPos, bForce);//no forcing: sync only of 'sync mode' is ON
}

void ADCSourceView::slotForceSynchronize()
{
	emitSynchronize(true);
}

ADCSourceView::ScopingFlags ADCSourceView::ToScopingFlags(unsigned u0)
{
	ScopingFlags e(SCOPING_NULL);
	uint u(u0 >> 4);//locality
	u &= 0xFF;//object
	if (u != 0)
	{
		e = ScopingFlags(1 << (u - 1));
	}
	return e;
}

void ADCSourceView::blockContextActions()
{
#if(0)
	fprintf(stdout, "*** contextid = %d\n", muContextId);
	fflush(stdout);
#endif
	ScopingFlags eScoping0(ToScopingFlags(muContextId));
	for (QList<MyAction*>::Iterator i(mActions.begin()); i != mActions.end(); i++)
	{
		MyAction& action(*(*i));
		bool bNoContextAction(action.scoping() == SCOPING_NULL);//always added if action is contextless
		ScopingFlags eScoping(action.scoping());
		bool bActive(bNoContextAction || (eScoping & eScoping0));
		//action.blockSignals(!bActive);
		if (bActive)
			addAction(&action);
		else
			removeAction(&action);
#if(0)
		fprintf(stdout, "action %s: %s\n", bAdd ? "added" : "removed", (*i)->text().toLatin1().data());
		fflush(stdout);
#endif
	}
}

void ADCSourceView::enableContextActions(bool bEnable)
{
	ADCSourceViewBase::enableContextActions(bEnable);
	//emit signalContextIdInq(muContextId);
	//ScopingFlags eScoping0(ToScopingFlags(muContextId));
	for (QList<MyAction *>::Iterator i(mActions.begin()); i != mActions.end(); i++)
	{
		QAction *pa(*i);
		/*const MyAction &a(*pa);
		bool bNoContextAction(a.scoping() == SCOPING_NULL);//always added if action is contextless
		ScopingFlags eScoping(a.scoping());
		bool bAdd(bNoContextAction || (eScoping & eScoping0));*/
		if (bEnable)
			addAction(pa);
		else
			removeAction(pa);
	}
}

void ADCSourceView::checkAction(QAction *a, uint f, bool b)
{
	if (/*?srcModel() &&*/ a)
	{
		a->blockSignals(true);
		if (b)
			a->setChecked(srcModel()->mode().testL(f));
		else
			a->setChecked(!srcModel()->mode().testL(f));
		a->blockSignals(false);
	}
}

void ADCSourceView::checkAction2(QAction *a, int col)
{
	if (/*?srcModel() &&*/ a)
	{
		a->setChecked(srcModel()->columnWidth(col) > 0);
	}
}

void ADCSourceView::updateActions()
{
#if(0)
	bool bEnabled(/*?srcModel() && */!srcModel()->IsHeader());

	mpToggleLinesAction->setEnabled(bEnabled);
	mpToggleStackAction->setEnabled(bEnabled);
	mpToggleFpuAction->setEnabled(bEnabled);
	mpTogglePathsAction->setEnabled(bEnabled);

	for (QList<QAction *>::Iterator i(mActions.begin()); i != mActions.end(); i++)
		(*i)->setEnabled(bEnabled);
#ifdef _DEBUG
	mpPrintObjInfoAction->setEnabled(true);
#endif
#endif
	
	//checkAction(mpViewLowProfileAction, DUMP_UNFOLD);
	checkAction(mpFlatViewAction, DUMP_BLOCKS, false);
	checkAction(mpShowXRefsAction, DUMP_XREFS);
	checkAction(mpToggleInflowAction, DUMP_INDATA);
	checkAction(mpToggleOutflowAction, DUMP_OUTDATA);
	checkAction(mpToggleLostCodeAction, DUMP_DEADCODE);
	checkAction(mpToggleLostLabelsAction, DUMP_DEADLABELS);
#ifdef ENABLE_DEBUG_CONTROLS
	checkAction(mpShowStrucLocsAction, DUMP_STRUCLOCS);// , false);
#endif
	checkAction(mpToggleArgsAction, DUMP_ALL_ARGS);

	checkAction2(mpToggleLinesAction, 0);
	checkAction2(mpToggleStackAction, 1);
	checkAction2(mpToggleFpuAction, 2);
	checkAction2(mpTogglePathsAction, 3);

//	updateProfileAction();
}

/*void ADCSourceView::slotViewDisassembly()
{
	emit signalViewDisassembly(this);
}*/

void ADCSourceView::slotShowXDepsIn(bool bOn)
{
	//?if (srcModel())
	srcModel()->setMode(adcui::DUMP_INDATA, bOn);
	updateContents();
}

void ADCSourceView::slotShowXDepsOut(bool bOn)
{
	//?if (srcModel())
	srcModel()->setMode(adcui::DUMP_OUTDATA, bOn);
	updateContents();
}

void ADCSourceView::slotShowXRefs(bool bOn)
{
	srcModel()->setMode(adcui::DUMP_XREFS, bOn);
	updateContents();
}

void ADCSourceView::slotShowDeadCode(bool bOn)
{
	if (srcModel()->setMode(adcui::DUMP_DEADCODE, bOn))
		redumpModel(false);
}

void ADCSourceView::slotShowDeadLabels(bool bOn)
{
	if (srcModel()->setMode(adcui::DUMP_DEADLABELS, bOn))
		if (srcModel()->mode().testL(adcui::DUMP_DEADCODE))
			redumpModel(false);
}

void ADCSourceView::slotToggleArguments(bool bOn)
{
	srcModel()->setMode(adcui::DUMP_ALL_ARGS, bOn);
	updateContents();
}

void ADCSourceView::updateCurPosInDoc()
{
	if (!hasFocus() || !hasModel())
		return;

	ADCSourceViewBase::updateCurPosInDoc();
	//if (srcModel() && !mpEdit)
		//srcModel()->setCurPosIt(curIt().line(), m_ptCaret.x() + m_nOffsetChar);
	//int y(textModel()->lineFromIt(curIt().line()));

	ADCStream ss;
	int y(srcModel()->lineFromItEx(curIt().line(), ss));
	ADCCurInfo si;
	si.scope = ss.ReadString();//some line extra info
	si.scopePath = ss.ReadString();
	si.obj = ss.ReadString();
	si.objPath = ss.ReadString();

	emit signalCaretPosChanged(curIt().x() + 1, y + 1, si);

	uint u;
	emit signalContextIdInq(u);
	if (u != muContextId)
	{
		muContextId = u;
		blockContextActions();
	}
}

/*int ADCSourceView::GetContentsHeight()
{
	return  srcModel()->linesTotal();
}*/

void ADCSourceView::slotMakeInt()
{
	emit signalPostCommand("makeint -f", false);
}

void ADCSourceView::slotMakeReal()
{
	emit signalPostCommand("makereal -f", false);
}

void ADCSourceView::slotMakeBit()
{
	emit signalPostCommand("makebit -f", false);
}

void ADCSourceView::slotMakePointer()
{
	emit signalPostCommand("mkptr", false);
}

void ADCSourceView::slotMakeConst()
{
	emit signalPostCommand("mkconst", false);
}

void ADCSourceView::slotUntype()
{
	emit signalPostCommand("makeunk", false);
}

void ADCSourceView::slotMakeArray()
{
	emit signalPostCommand("makearr", false);
}

void ADCSourceView::slotMakeGap()
{
	emit signalPostCommand("mkgap -f", false);
}

void ADCSourceView::slotApplyType()
{
	emit signalPostCommand("applytype", false);
}

void ADCSourceView::slotReconstructClassHierarchy()
{
	emit signalPostCommand("reconclsh", true);
}

//inheritance

void ADCSourceView::slotInheritPublic()
{
	emit signalPostCommand("addheir -x", false);
}

void ADCSourceView::slotInheritProtected()
{
	emit signalPostCommand("addheir -y", false);
}

void ADCSourceView::slotInheritPrivate()
{
	emit signalPostCommand("addheir -z", false);
}

void ADCSourceView::slotDisinherit()
{
	emit signalPostCommand("rmheir", false);
}

void ADCSourceView::slotToggleVFTablePtr()
{
	emit signalPostCommand("togvptr", false);
}

void ADCSourceView::slotToggleThisPtr()
{
	emit signalPostCommand("togthisptr", false);
}

void ADCSourceView::slotToggleMethodVirtual()
{
	emit signalPostCommand("toggle_virtual", false);
}

//convertion

void ADCSourceView::slotConvertNamespace()
{
	emit signalPostCommand("convns", false);
}

void ADCSourceView::slotConvertClass()
{
	emit signalPostCommand("convcls", false);
}

void ADCSourceView::slotConvertStruct()
{
	emit signalPostCommand("convstr", false);
}

void ADCSourceView::slotConvertEnum()
{
	emit signalPostCommand("convenu", false);
}

void ADCSourceView::slotConvertUnion()
{
	emit signalPostCommand("convuni", false);
}

void ADCSourceView::slotDecompile()
{
	emit signalPostCommand("decompile", false);
}

void ADCSourceView::slotAnalyze()
{
	emit signalPostCommand("analyze", false);
}

/*void ADCSourceView::slotRedecompile()
{
	emit signalPostCommand("decompile -f", false);
}*/

void ADCSourceView::slotToggleRoot()
{
	emit signalPostCommand("toggle_root", false);
}

void ADCSourceView::slotToggleIf()
{
	emit signalPostCommand("toggle_if", false);
}

void ADCSourceView::slotToggleElse()
{
	emit signalPostCommand("toggle_else", false);
}

void ADCSourceView::slotToggleSwitch()
{
	emit signalPostCommand("toggle_switch -v", false);
}

void ADCSourceView::slotToggleWhile()
{
	emit signalPostCommand("toggle_while", false);
}

void ADCSourceView::slotForLoop()
{
	emit signalPostCommand("toggle_for", false);
}

void ADCSourceView::slotCombineLogic()
{
	emit signalPostCommand("do_logic", false);
}

void ADCSourceView::slotKillLogic()
{
	emit signalPostCommand("undo_logic", false);
}

void ADCSourceView::slotExpand()
{
	//?if (mpIModel)
		if (srcModel()->mode().testL(adcui::DUMP_UNFOLD))
			emit signalPostCommand("xpnd -s", false);
		else
			emit signalPostCommand("xpnd", false);
}

void ADCSourceView::slotCollapse()
{
	emit signalPostCommand("clps", false);
}

void ADCSourceView::slotFlip()
{
	emit signalPostCommand("flip", false);
}

void ADCSourceView::slotBind()
{
	emit signalPostCommand("bind", false);
}

void ADCSourceView::slotUnbind()
{
	emit signalPostCommand("unbind", false);
}

void ADCSourceView::slotAcquire()
{
	emit signalPostCommand("acquire", false);
}

/*void ADCSourceView::slotAcquireConstant()
{
	emit signalPostCommand("acquire_constant", false);
}*/

void ADCSourceView::slotCut()
{
	emit signalPostCommand("cut", false);
}

void ADCSourceView::slotPaste()
{
	emit signalPostCommand("paste", false);
}

void ADCSourceView::slotDelete()
{
	emit signalPostCommand("del", false);
}

//adcui::IADCTextEdit		*mpIEdit;
void ADCSourceView::slotEditName()
{
	startInplaceEdit();
}

bool ADCSourceView::startInplaceEdit()
{
	if (!ADCSourceViewBase::startInplaceEdit())
		return false;
	enableContextActions(false);
	return true;
}

bool ADCSourceView::stopInplaceEdit()
{
	if (!ADCSourceViewBase::stopInplaceEdit())
		return false;
	enableContextActions(true);
	return true;
}

/*void ADCSourceView::slotCancel()
{
	stopInplaceEdit();
}*/

void ADCSourceView::setActive(bool bActive)
{
	emit signalEnableToggleRoot(bActive);
	emit signalEnableToggleIf(bActive);
	emit signalEnableToggleElse(bActive);
	emit signalEnableToggleWhile(bActive);
	emit signalEnableCombineLogic(bActive);
	emit signalEnableKillLogic(bActive);
	emit signalEnableDelete(bActive);
}

void ADCSourceView::slotClose(QString filename)
{
	emit signalReleaseModel(filename);
}

static int tab2spaces(int pos)
{
	int t = 0;
	do {
		t++;
	} while ((pos + t) & 0x3);//TABMASK
	return t;
}

#if(0)
void ADCSourceView::draw Cell(QPainter *pPainter, QRect &rc0, int col, adcui::DUMPOS drawIt, int nLineCount, const ADCCell &cell)
{
	//std::string sCell(cell.toString().toStdString());
	//const char *line(sCell.c_str());

	QFontMetrics fm(pPainter->fontMetrics());
	QRect rc(rc0);
	int w(charWidth());
	QPoint pt(rc.left(), rc.bottom() - (fm.descent() + 1));

	for (int i(0); i < cell.size(); i++)
	{
		const ADCBar &bar(cell[i]);
		ColorFromId(pPainter, bar.first);
		QString s(bar.second);
		//pPainter->drawText(rc.left(), y_base, s);
		//rc.setLeft(rc.left() + s.length() * w);
		pPainter->drawText(pt, s);
		pt.rx() += s.length() * w;
	}

	/*ColorFromId(pPainter, 0);
	MapFont mapFont(pPainter);

	bool bQuit = false;
	const char * p = line;
	int lenx = 0;
	if (p)
	while (*p && !bQuit) 
	{

		int colorID = -1;
		int fontID = -1;
		int len = 0;

		while (*p)
		{
			char c = *p;

			if (c == '\n')
			{
				p++;
				bQuit = true;
				break;
			}

			if (c == (char)SYM_TAB)
			{
				if (len > 0)
					break;

				pt.rx() += tab2spaces(lenx + len)*w;
				p++;
				line++;
				continue;
			}

			if (c == (char)SYM_COLOR)
			{
				colorID = p[1];
				p += 2;
				break;
			}

			if (c == (char)SYM_FONT)
			{
				fontID = p[1];
				p += 2;
				break;
			}

			p++;
			len++;
		}

		if (len > 0)
		{
			QString s(QString::fromAscii(line, len));
			pPainter->drawText(pt, s);
			pt.rx() += len*w;
			//pt.rx() += fm.width(s);
			lenx += len;
		}
		
		if (colorID >= 0)
		{
			ColorFromId(pPainter, colorID);
			//pPainter->setBackground(MapBrush(colorID));
			colorID = -1;
		}

		if (fontID >= 0)
		{
			mapFont.map(fontID);
			fontID = -1;
		}

		line = p;
	}*/

	/*if (srcModel()->checkEqual(mSelIt, drawIt) == 0)
	{
		pPainter->setPen(ColorFromId(COLOR_SEL));
		pPainter->drawRect(rc0.left(), rc0.top(), rc0.right(), rc0.bottom());
	}*/

}
#endif


void ADCSourceView::OnContentsChanged()
{
	//if (srcModel())
	{
		ModelLocker lock(this);
		if (!lock.model())
			return;
		
		lock.model()->Redump(curIt().line(), false);
	}
	ADCSourceViewBase::OnContentsChanged();
	updateCurPosInDoc();

	setHScrollBar();
	setVScrollBar();
}

void ADCSourceView::slotRedump()
{
	OnContentsChanged();
}

bool ADCSourceView::checkDirtyContents()
{
	if (hasModel())
	{
		//int linesFromTop(curLine());
		int ret(srcModel()->IsRedumpPending());
		if (ret)
		{
/*			//srcModel()->copyIt(mpTopIt->line(), curIt().line());
			*mpTopIt = curIt();

			modelData().scrollUp(linesFromTop);

			OnContentsChanged();
			//this will produce a blinking during a refresh
			setVScrollBar();*/
			return true;
		}
	}
	return false;
}

void ADCSourceView::paintEvent(QPaintEvent* e)
{
	if (checkDirtyContents())
		return;

	ModelLocker lock(this);
	ADCSourceViewBase::paintEvent(e);
}


void ADCSourceView::slotUpdateDisplay(const QString& fname)
{
}

/*void ADCSourceView::slotHorzSliderMoved(int nPos)
{
	m_nOffsetChar = nPos;
	updateContents();
}

void ADCSourceView::slotVertSliderMoved(int nPos)
{
	srcModel()->seekLineIt(mTopIt, nPos);
	updateContents();
}*/


void ADCSourceView::keyPressEvent(QKeyEvent * e)
{
	switch (e->key()) 
	{
	case Qt::Key_Control:
		//emit signalOverrideSyncMode(true);
		break;
/*	case Qt::Key_Home:
		setCaret(0, -1);
		break;
	case Qt::Key_End:
		setCaret(pageWidth()-1, -1);
		break;*/
	default:
		break;
	}

	ADCSourceViewBase::keyPressEvent(e);
}

void ADCSourceView::keyReleaseEvent(QKeyEvent *e)
{
	switch (e->key()) 
	{
	case Qt::Key_Control:
		//emit signalOverrideSyncMode(false);
	default:
		break;
	}

	ADCSourceViewBase::keyReleaseEvent(e);
}

/*void ADCSourceView::updateProfileAction()
{
	if (mpViewToggleProfileAction)
	{
		if (srcModel()->mode().testL(adcui::DUMP_UNFOLD))
		{
			mpViewToggleProfileAction->setText("Switch To Source");
			mpViewToggleProfileAction->setIcon(QIcon(":source_high.png"));
		}
		else
		{
			mpViewToggleProfileAction->setText("Switch To Source (Low Profile)");
			mpViewToggleProfileAction->setIcon(QIcon(":source_low.png"));
		}
	}
}*/

struct Locality_t
{
	union
	{
		unsigned short u;
		struct
		{
			unsigned short locality : 4;
			unsigned short scoping : 8;
			//unsigned short decl_ : 4;
			unsigned short obj : 4;
		};
	};
	Locality_t(unsigned short _u) { u = _u; }
};

void ADCSourceView::contextMenuEvent(QContextMenuEvent *e)
{
	if (inplaceEdit() || hasSelection())
	{
		ADCTextView::contextMenuEvent(e);
		return;
	}

	QMenu *popup(new QMenu(this));

	//QLabel * caption = new QLabel( "<font color=darkblue><u><b>Context Menu</b></u></font>", contextMenu );
	//caption->setAlignment( Qt::AlignCenter );
	//contextMenu->insertItem( caption->text() );

#if(0)
	fprintf(stdout, "-->%d", muContextId);fflush(stdout);
#endif
	//do not rely on muContextId - it may lag. Ask context state directly.
	uint uContextId(muContextId);
	emit signalContextIdInq(uContextId);

	Locality_t uLoc(uContextId);

	//uint uObj(uContextId & adcui::CXTID_OBJECT_MASK);
	//uint uDecl(uContextId & adcui::CXTID_DECL_MASK);
	ScopingFlags fScoping(ToScopingFlags(uContextId));

	if (uLoc.obj)
	{
		popup->addAction(mpNameAction);
		//popup->addSeparator();
		popup->addAction(mpGotoDefinitionAction);
		popup->addAction(mpGotoDeclarationAction);
	}
	popup->addSeparator();

	int count(0);
	if (fScoping != SCOPING_NULL)
	{
		for (QList<MyAction *>::Iterator i(mActions.begin()); i != mActions.end(); i++, count++)
		{
			MyAction *a(*i);
			if (a->scoping() & fScoping)
			{
				//if (!(fScoping & SCOPING_FUNC_BODY) || uDecl & adcui::CXTID_DECL_OP)
					popup->addAction(a);
			}
		}
	}

	if (fScoping & SCOPING_STRUC_HEADER)
	{
		QMenu* convertMenu(new QMenu(tr("Convert to"), popup));
		convertMenu->addAction(mpMakeNamspaceAction);
		convertMenu->addAction(mpMakeClassAction);
		convertMenu->addAction(mpMakeStructAction);
		convertMenu->addAction(mpMakeEnumAction);
		convertMenu->addAction(mpMakeUnionAction);
		popup->addMenu(convertMenu);
	}

	if (count > 0)
		popup->addSeparator();

	/*QMenu* advanceMenu(popup);// new QMenu(tr("&Advanced"), popup));
	{
	advanceMenu->addAction(mpHandleRuntimeTypeInfoAction);
	popup->addSeparator();
	}*/

	if (fScoping == 0)
	{
		popup->addAction(mpPasteAction);
	}
	else if ((fScoping & (SCOPING_STRUC_HEADER | SCOPING_FUNC_HEADER | SCOPING_STRUC_DATA | SCOPING_STRUC_METHOD | SCOPING_STRUC_STATIC)))//last 2 are moved from class?
	{
		popup->addAction(mpCutAction);
		popup->addSeparator();
		popup->addAction(mpDeleteAction);
		popup->addSeparator();
	}

	if (!mExtraActions.empty())
	{
		for (QList<QAction *>::iterator i(mExtraActions.begin()); i != mExtraActions.end(); i++)
			popup->addAction(*i);
	}


	//popup->addAction(mpViewToggleProfileAction);

	/*if (srcModel() && (srcModel()->mode() & adcui::DUMP_UNFOLD))
		popup->addAction(mpViewHighProfileAction);
	else
		popup->addAction(mpViewLowProfileAction);*/

	//popup->addSeparator();
	//popup->addAction(mpFlatViewAction);
	//popup->addSeparator();
	//popup->addAction(mpShowXRefsAction);
	//popup->addAction(mpToggleOutflowAction);
	//popup->addAction(mpToggleInflowAction);
	//popup->addAction(mpToggleLostCodeAction);
	//popup->addAction(mpToggleArgsAction);

	/*QMenu* subColumns = new QMenu(tr("&Columns"), popup);
	{
		subColumns->addAction(mpToggleLinesAction);
		subColumns->addAction(mpToggleStackAction);
		subColumns->addAction(mpToggleFpuAction);
		subColumns->addAction(mpTogglePathsAction);
		popup->addMenu(subColumns);
	}*/

	//popup->addSeparator();
	//popup->addAction(mpDecompileAction);
	//popup->addSeparator();
	//popup->addAction(mpSyncAction);
	//popup->addAction(mpSaveListingAction);
	//popup->addAction(mpGotoLineAction);

#ifdef ENABLE_DEBUG_CONTROLS
	popup->addSeparator();
	//QMenu* debugMenu(new QMenu(tr("[DEBUG]"), popup));
	popup->addAction(mpDumpExprAction);
	//popup->addAction(mpShowStrucLocsAction);
	popup->addAction(mpPrintObjInfoAction);
	//popup->addAction(mpPrintDumpInfoAction);
	//popup->addAction(mpToggleDraftModeAction);
	//popup->addAction(mpToggleVarOpsAction);
	popup->addAction(mpTestAction);
	//popup->addMenu(debugMenu);
#endif

	popup->exec(QCursor::pos());
	delete popup;
}

/*void ADCSourceView::slotViewNormal()
{
	srcModel()->reset(0);//normal
	update();
}*/

void ADCSourceView::redumpModel(bool bResetIterators)
{
	int linesFromTop(bResetIterators ? 0 : curLine());
	Q_UNUSED(linesFromTop);
	Q_ASSERT(srcModel()->IsRedumpPending());
	srcModel()->Redump(curIt().line(), bResetIterators);

/*	if (linesFromTop > 0)
	{
		topIt() = curIt();//set top iter to cur iter and rollback it number of lines
		modelData().scrollUp(linesFromTop);
		//this will produce a blinking during a refresh
	}*/
	ADCSourceViewBase::OnContentsChanged();//recalc doc's dimentions
	setVScrollBar();
//	mpVertScrollBar->setValue(0);
	//setVScrollBar();

	updateCurPosInDoc();
	updateContents();
}

void ADCSourceView::slotContextIdChanged(unsigned u)
{
	if (!hasFocus())
		return;
	if (u != muContextId)
	{
		muContextId = u;
		setContextId(u);
	}
}

/*void ADCSourceView::slotToggleProfile()
{
	if (!srcModel()->setMode(DUMP_UNFOLD, true, curIt().line()))
		if (!srcModel()->setMode(DUMP_UNFOLD, false, curIt().line()))
			return;
	updateProfileAction();
	redumpModel(false);
	emit signalProfileChanged();
}*/

/*void ADCSourceView::slotSwitchToLowProfile()
{
	if (srcModel()->setMode(DUMP_UNFOLD, true, curIt().line()))
	{
		redumpModel(false);
		emit signalProfileChanged();
	}
}

void ADCSourceView::slotSwitchToHighProfile()
{
	if (srcModel()->setMode(DUMP_UNFOLD, false, curIt().line()))
	{
		redumpModel(false);
		emit signalProfileChanged();
	}
}*/

void ADCSourceView::slotToggleBlocked()
{
	if (!srcModel()->setMode(DUMP_BLOCKS, false))
		if (!srcModel()->setMode(DUMP_BLOCKS, true))
			return;
	redumpModel(false);
	emit signalProfileChanged();
}

void ADCSourceView::slotViewBlocked(bool bOn)
{
	if (srcModel()->setMode(DUMP_BLOCKS, !bOn))
	{
		redumpModel(false);
		emit signalProfileChanged();
	}
}

void ADCSourceView::slotFileInvalidated(int fileId)
{
	if (fileId != mFileId)
		return;
	OnContentsChanged();
//	updateContents();
}

void ADCSourceView::slotRedraw()
{
	updateContents();
}

void ADCSourceView::slotDebuggerBreak()
{
	if (srcModel()->seekPosIt("$dbg", curIt().line()))
	{
		assureCursorVisible();
		curIt().shiftHome();
	}
	updateContents();
}

void ADCSourceView::slotToggleColumn(int i)
{
	toggleColumn(i);
}

void ADCSourceView::slotRefresh0()
{
#if(TEST_COLORS)
	void test_colors();
	test_colors();
#endif
	srcModel()->invalidate(true);
	redumpModel(false);
}

void ADCSourceView::slotRefresh()
{
	srcModel()->invalidate(false);
	redumpModel(false);
}

/*void ADCSourceView::synchronizeWith(ADCSourceView *pOther)
{
	checkDirtyContents();
	ISrcViewModel *pIModelOther(pOther ? pOther->model() : nullptr);
	int itSelOther(pOther ? pOther->selIt() : -1);
	if (model()->synchronizeIt(curIt().line(), pIModelOther, itSelOther))
	{
		model()->copyIt(mpTopIt->line(), curIt().line());
		int linesFromTop(pOther->curLine());
		if (linesFromTop >= 0)
		{
			modelData().scrollUp(linesFromTop);
			setVScrollBar();
			//setCaret(caret().x(), pOther->caret().y());
			updateCurPosInDoc();
		}
	}
	updateContents();
}*/

/*QString ADCSourceView::getAddress()
{
	QString s;
	ADCStream ss;
	if (srcModel()->getAddress(curIt().line(), ss))
		ss.ReadString(s);
	return s;
}*/

bool ADCSourceView::event(QEvent *event)
{
	if (event->type() == QEvent::ToolTip)
	{
		QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
		QPoint p(helpEvent->pos());

		QString s;
		if (hasModel())
		{
			ADCDocPos tipIt(srcModel());
			if (setIter(tipIt, p.x(), p.y()))
			{
				ModelLocker lock(this);
				ADCStream ss;
				srcModel()->tipInfoIt(tipIt.line(), tipIt.x(), ss);
				ss.ReadString(s);
			}
		}

		if (!s.isEmpty())
		{
			QToolTip::showText(helpEvent->globalPos(), s);
		}
		else
		{
			QToolTip::hideText();
			event->ignore();
		}

		return true;
	}
	return ADCSourceViewBase::event(event);
}

void ADCSourceView::showEvent(QShowEvent *e)
{
	ADCSourceViewBase::showEvent(e);
}

void ADCSourceView::focusInEvent(QFocusEvent *e)
{
	ADCSourceViewBase::focusInEvent(e);

	if (ADCBinView::checkActiveWin(this))
	{
		updateCurPosInDoc();
		//muContextId = adcui::CONTEXTID_NULL;
		//emit signalContextIdInq(muContextId);

		//enableContextAction(true);
		//emit signalGotFocus();
	}
	SxFindTextDlg *pDlg(ADCSourceWin::getFindDlg(this));
	pDlg->setTed(new FindTextObject(this));
}

void ADCSourceView::focusOutEvent(QFocusEvent *e)
{
	ADCSourceViewBase::focusOutEvent(e);
	//if (!isActiveWindow())
		//;
	//disableContextActions();
}

void ADCSourceView::mouseReleaseEvent(QMouseEvent *e)
{
	if (mpViewport)
	{
		if (columnResizing() < 0)
			emitSynchronize(false);
	}

	ADCSourceViewBase::mouseReleaseEvent(e);
}

void ADCSourceView::mouseMoveEvent(QMouseEvent *e)
{
	ModelLocker lock(this);
	ADCSourceViewBase::mouseMoveEvent(e);
}

void ADCSourceView::slotDumpExpression()
{
	emit signalPostCommand("dumpexpr", false);
}

void ADCSourceView::slotPrintObjInfo()
{
	emit signalPostCommand("objinfo", true);
}

void ADCSourceView::slotPrintDumpInfo()
{
	srcModel()->printDumpInfo();
}

void ADCSourceView::slotToggleDraftMode()
{
	emit signalPostCommand("draftest", true);
}

void ADCSourceView::slotToggleVarOps()
{
	bool bOn(srcModel()->mode().testL(DUMP_NOVAROPS));
	UDispFlags f(DUMP_NOVAROPS);
	if (srcModel()->setMode(f, !bOn))
		redumpModel(false);
}

void ADCSourceView::slotTest()
{
	ADCProtoEditDlg dlg(this);
	dlg.exec();
}

void ADCSourceView::slotShowStrucLocs(bool bOn)
{
	if (srcModel()->setMode(DUMP_STRUCLOCS, bOn))
		redumpModel(false);
}

void ADCSourceView::mouseDoubleClickEvent(QMouseEvent *e)
{
	if (!mpEdit)
		slotEditName();
	else
		ADCTextView::mouseDoubleClickEvent(e);
}

void ADCSourceView::slotCurOpChanged()
{

}

void ADCSourceView::slotSyncPanesResponce3(int iPosRef, bool bNoFocus)
{
	if (!hasModel())
		return;
	//if (bNoFocus && viewport()->hasFocus())//the other view should update postion
		//return;//?
	if (hasFocus())
		return;

	if (srcModel()->seekPosIt("$sync", topIt().line()))
	{
		curIt().copyFrom(&topIt());

		int iPos(0);//global position of the widget
		for (QWidget *p(this); p != nullptr; p = p->parentWidget())
			iPos += p->pos().y();
		if (iPos < iPosRef)
			iPos = iPosRef - iPos;
		else
			iPos -= iPosRef;
		int linesFromTop(iPos / lineHeight() - topMargin());

		if (0 < linesFromTop && linesFromTop < pageHeight())
		{
			modelData().scrollUp(linesFromTop);
		}
		setVScrollBar();
	}
	updateContents();
}

void ADCSourceView::slotGoToDeclaration()
{
	int linesFromTop(curLine());
	if (srcModel()->initiateJump(false, linesFromTop))
	{
		/*curIt().copyFrom(&topIt());
		showCursorAtLine(linesFromTop);
		//setVScrollBar();
		updateContents();
		updateCurPosInDoc();*/
		emit signalJumpSourceBack(false);
	}
}

void ADCSourceView::slotGoToDefinition()
{
	int linesFromTop(curLine());
	if (srcModel()->initiateJump(true, linesFromTop))
	{
		/*curIt().copyFrom(&topIt());
		showCursorAtLine(linesFromTop);
		//setVScrollBar();
		updateContents();
		updateCurPosInDoc();*/
		emit signalJumpSourceBack(false);
	}
}

void ADCSourceView::slotGoToDeclarationRecoil(int linesFromTop, bool bFwd, bool bFlip)
{
	if (srcModel()->jump(curIt().line(), bFwd, bFlip))
	{
		if (linesFromTop < 0)
		{
			assureCursorVisible();
		}
		else
		{
			//topIt().copyFrom(&curIt());
			showCursorAtLine(linesFromTop);
			//setVScrollBar();
			updateContents();
			updateCurPosInDoc();
		}
	}
}

void ADCSourceView::slotGoBack()
{
	emit signalJumpSourceBack(true);
}

void ADCSourceView::slotGoToLine()
{
	ADCGotoDlg dlg(this, srcModel()->linesNum());
	//dlg.setWindowIcon(QIcon(":go_page.png"));

	QRect parentRect(mapToGlobal(QPoint(0, 0)), size());
	dlg.move(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, dlg.size(), parentRect).topLeft());

	if (dlg.exec() == QDialog::Accepted)
	{
		int linesFromTop(curLine());
		srcModel()->seekLineIt(topIt().line(), dlg.lineNumber() - 1);
		curIt().copyFrom(&topIt());
		showCursorAtLine(linesFromTop);//scroll to position
		//setVScrollBar();
		//updateContents();
		updateCurPosInDoc();
	}
}

void ADCSourceView::slotFind()
{
	SxFindTextDlg *pDlg(ADCSourceWin::getFindDlg(this));
	if (pDlg->isHidden())
	{
		pDlg->setFont(font());

		const QPoint parentRect(mapToGlobal(rect().center()));
		pDlg->move(parentRect.x() - pDlg->width() / 2, parentRect.y() - pDlg->height() / 2);

		pDlg->show();
	}
	else
		pDlg->activate();
	//pDlg->setTed(new FindTextObject(this));
}

QAction *ADCSourceView::addContextAction(QAction *a)//const QIcon &pix, QString s, QString shortcut)
{
	QAction *pAction(a);// new QAction(pix, s, this));
	/*if (!shortcut.isEmpty())
	{
		pAction->setShortcut(QKeySequence(shortcut));
		pAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	}*/
	mExtraActions.push_back(pAction);
	addAction(pAction);
	return pAction;
}

void ADCSourceView::slotSetFont(const QFont &f)
{
	setFont(f);
}

/*void ADCSourceView::slotToggleSyncMode(bool bOn)
{
	if (srcModel()->setSyncMode(bOn))
	{
		if (bOn && hasFocus())// isActiveWindow())
			emitSynchronize(false);
	//		slotSync PanesResponce(-1);//?
		updateContents();
	}
}*/

void ADCSourceView::slotFocusTaskTopAtLine(QString task, int linesFromTop)
{
	if (!srcModel()->seekPosIt(task.toLatin1(), topIt().line()))
		return;

	curIt().copyFrom(&topIt());
	showCursorAtLine(linesFromTop);
	//setVScrollBar();
	/*if (linesFromTop >= 0)
	{
		//srcModel()->copyIt(mpTopIt->line(), it);
		modelData().scrollUp(linesFromTop);
		setVScrollBar();
		//setFocus();
		//fprintf(stdout, "Got focus(4)\n");fflush(stdout);
	}*/
	updateContents();
}

void ADCSourceView::slotSaveListing()
{
	QStringList l;//options
	UDispFlags mode(srcModel()->mode());
	if (mode.testL(adcui::DUMP_UNFOLD))
		l.append("-u");
	if (mode.testL(adcui::DUMP_LNUMS))
		l.append("-l");
	if (mode.testL(adcui::DUMP_BLOCKS))
		l.append("-b");
	if (mode.testL(adcui::DUMP_DEADCODE) || mode.testL(adcui::DUMP_DEADLABELS))
		l.append("-d");
	if (mode.testL(adcui::DUMP_STACKTOP))
		l.append("-s");
	if (mode.testL(adcui::DUMP_FPUTOP))
		l.append("-f");
	if (mode.testL(adcui::DUMP_STRUCLOCS))
		l.append("-sloc");
	QString filePath(srcModel()->filePath());
	emit signalSaveListing1(filePath, l.join(" "));
}

void ADCSourceView::populateToolbarMenuUnfold(QMenu *pMenu)
{
	pMenu->addAction(mpGotoLineAction);
	pMenu->addAction(mpToggleInflowAction);
	pMenu->addAction(mpToggleOutflowAction);
	pMenu->addAction(mpShowXRefsAction);
	pMenu->addAction(mpFlatViewAction);
	pMenu->addAction(mpToggleLostCodeAction);
	//pMenu->addAction(mpToggleArgsAction);//irrelevant
	pMenu->addAction(mpSaveListingAction);
		pMenu->addSeparator();
	pMenu->addAction(mpToggleLinesAction);
	pMenu->addAction(mpToggleStackAction);
	pMenu->addAction(mpToggleFpuAction);
	pMenu->addAction(mpTogglePathsAction);
#ifdef ENABLE_DEBUG_CONTROLS
		pMenu->addSeparator();
	pMenu->addAction(mpToggleDraftModeAction);
	pMenu->addAction(mpToggleVarOpsAction);
#endif
}





/////////////////////////////////////////////////////////////
// ADCSrcWinToolbar

ADCSrcWinToolbar::ADCSrcWinToolbar(ADCSourcePane *parent)
: ADCSplitToolbar(parent)
{
}

void ADCSrcWinToolbar::populate(ADCSplitWin *pMainWin, int index)
{
	//ADCSplitToolbar::populate(pMainWin, index);

	ADCSourcePane *pWin(dynamic_cast<ADCSourcePane *>(parent()));
	ADCSourceView* pView(pWin->view());

	QMenu *pPopup1 = new QMenu(this);
	pPopup1->addAction(pView->mpFlatViewAction);
	pPopup1->addAction(pView->mpToggleArgsAction);
	pPopup1->addAction(pView->mpFindAction);
	pPopup1->addAction(pView->mpGotoLineAction);
	pPopup1->addAction(pView->mpGoBackAction);
#ifdef ENABLE_DEBUG_CONTROLS
	pPopup1->addSeparator();
	pPopup1->addAction(pView->mpShowStrucLocsAction);
	pPopup1->addAction(pView->mpPrintDumpInfoAction);
	pPopup1->addAction(pView->mpToggleDraftModeAction);
	pPopup1->addAction(pView->mpToggleVarOpsAction);
#endif

	addButton(pView->mpToggleInflowAction);
	addButton(pView->mpToggleOutflowAction);
	addButton(pView->mpShowXRefsAction);
	//addButton(pView->mpFlatViewAction);
	QToolButton *pButton(addButton(pView->mpToggleLostCodeAction));
	QMenu *pPopup = new QMenu(this);
	pPopup->addAction(pView->mpToggleLostLabelsAction);
	pButton->setMenu(pPopup);
	pButton->setPopupMode(QToolButton::DelayedPopup);//MenuButtonPopup//InstantPopup

	//addButton(pView->mpToggleArgsAction);

	addSeparator();
	addButton(pView->mpToggleLinesAction);
	addButton(pView->mpToggleStackAction);
	addButton(pView->mpToggleFpuAction);
	addButton(pView->mpTogglePathsAction);


	//QAction *pAction1(pWin->mpViewToggleProfileAction);
	//QAction *pAction2(pWin->mpSwitchToBinaryAction);
	//pSrcView->addContextAction(pAction2);

	addSeparator();

	QToolButton* pTbtn(addButton(QIcon(":dc_view_16.png"), tr("View Options")));
	//pTbtn->setAutoRaise(true);//arrow_down_2_16
	pTbtn->setPopupMode(QToolButton::InstantPopup);
	pTbtn->setMenu(pPopup1);

	//pWin->addToolbarAction(pAction1);
	//pWin->addToolbarAction(pAction2);
	addStretch();
	addButton(pView->mpSaveListingAction);
}

void ADCSrcWinToolbar::cleanup()
{
	ADCSplitToolbar::cleanup();
	while (QWidget* w = findChild<QWidget*>())
		delete w;
}


bool ADCSrcWinToolbar::isSyncPanesOn(){ return mpSyncPanesTBtn->isChecked(); }
void ADCSrcWinToolbar::setSyncPanes(bool bOn){ mpSyncPanesTBtn->setChecked(bOn); }


/////////////////////////////////////////////////////////////
// ADCSourcePane

ADCSourcePane::ADCSourcePane(QWidget *parent)
	: ADCSplitPane(parent)
{
	//mpViewToggleProfileAction = nullptr;
	//mpSwitchToBinaryAction = newAction(tr("Switch To Disassembly"), SLOT(slotSwitchToBinaryView()), QKeySequence(tr("Shift+D")), QIcon(":dasm.16.png"));
}

void ADCSourcePane::createView(ADCModelData *pModelData)
{
	ADCSourceView *pView = new ADCSourceView(this);
	pView->setModelData(pModelData);
	mpView = pView;
}

void ADCSourcePane::createToolbar(ADCModelData *)
{
	ADCSrcWinToolbar *pToolbar(new ADCSrcWinToolbar(this));
	//mpToolbar->show();
	//mpToolbar->populate(static_cast<ADCSourceWin *>(parent()), 0);
	pToolbar->populate(nullptr, 0);//in.index);
	mpToolbar = pToolbar;
}

void ADCSplitToolbar::addToolbarAction(QAction *pAction)
{
	if (pAction)
	{
		if (mSwitchButtons.empty())
			addSeparator();
		mSwitchButtons.push_back(addButton(pAction));
	}
}

void ADCSourcePane::slotSwitchToBinaryView()
{
	int atLine(view()->curLine() + view()->topMargin());
	bool bUnfold(/*view()->model() && */view()->srcModel()->mode().testL(adcui::DUMP_UNFOLD));
	emit signalSwitchToBinaryView(this, atLine, bUnfold);
}

void ADCSourcePane::updateActions(int fileKind)
{
	bool bIsSource(true);//fileKind == 2);
	for (QList<QAction *>::Iterator i(mActions.begin()); i != mActions.end(); i++)
		(*i)->setEnabled(bIsSource);
	//mActions[0]->setEnabled(bIsSource);
	//mActions[1]->setEnabled(bIsSource);
}

void ADCSourcePane::updateFont(const QFont& f)
{
	setFont(f);
	view()->setFont(f);
}

/////////////////////////////////////////////////////////////
// ADCSourceWin

SxFindTextDlg* ADCSourceWin::spFindTextDlg = nullptr;

SxFindTextDlg* ADCSourceWin::getFindDlg(QWidget* pFrom)
{
	if (!spFindTextDlg)
	{
		QWidget* pSelf(pFrom);
		while (pSelf && !dynamic_cast<ADCSourceWin*>(pSelf))
			pSelf = pSelf->parentWidget();

		spFindTextDlg = new SxFindTextDlg(pSelf);
	}
	return spFindTextDlg;
}

void ADCSourceWin::destroyFindDlg()
{
	if (spFindTextDlg)
	{
		delete spFindTextDlg;
		spFindTextDlg = nullptr;
	}
}

ADCSourceWin::ADCSourceWin(QWidget *parent, const char *name)
	: ADCSourceWinBase(parent, name),
	mFileId(0),
	mFileKind(0)
{
	mDefaultPanes[0] = PANE_BINARY;
	mDefaultPanes[1] = PANE_SOURCE;
}


ADCSourceWin::~ADCSourceWin()
{
	destroyFindDlg();
}

void ADCSourceWin::init(int fileId, int fileKind)
{
	assert(fileId >= 0);
	mFileId = fileId;
	mFileKind = fileKind;
	updateActions(fileKind);
	if (mpSplitter->count() == 0)
	{
		ADCSplitPane *pWin;
		if (fileKind > 0)//not binary
		{
			pWin = openAltPane(NewPane_t(1, PANE_SOURCE, NPF_NONE));
//			mpToggleRightPaneAction->setEnabled(false);
		}
		else
		{
			pWin = openAltPane(NewPane_t(0, PANE_BINARY_0, NPF_NONE));
//			mpToggleRightPaneAction->setEnabled(false);
//			mpToggleLeftPaneAction->setEnabled(false);
		}
		pWin->updateActions(fileKind);
	}
	else
	{
		for (int i(0); i < 2; i++)
		{
			ADCSourcePane *pWin(dynamic_cast<ADCSourcePane *>(mViews[i]));
			if (pWin)
			{
				openAltPane(NewPane_t(i, PANE_SOURCE, NPF_NONE));
			}
			else
			{
				ADCBinaryPane *pWin(dynamic_cast<ADCBinaryPane *>(mViews[i]));
				if (pWin)
				{
					openAltPane(NewPane_t(i, PANE_BINARY, NPF_NONE));
				}
			}
		}
	}
}

void ADCSourceWin::OnAboutToClose(QWidget *pView)
{
	ADCSourceView *pView1(dynamic_cast<ADCSourceView *>(pView));
	if (pView1)
	{
		pView1->setModelData(nullptr);
	}
	else
	{
		ADCBinView *pView2(dynamic_cast<ADCBinView *>(pView));
		if (pView2)
			pView2->setModelData(nullptr);
	}
}

ADCModelData *ADCSourceWin::srcModelData(int iPane) const
{
	const PaneInfo &pi(pane(iPane));
	if (pi.pModelData && dynamic_cast<adcui::ISrcViewModel *>(pi.pModelData->pIModel))
		return pi.pModelData;
	/*for (int i(0); i < pi.models.size(); i++)
	{
		adcui::ISrcViewModel *p(dynamic_cast<adcui::ISrcViewModel *>(pi.models[i]->pIModel));
		if (p)
			return pi.models[i];
	}*/
	return nullptr;
}

ADCModelData *ADCSourceWin::binModelData(int iPane) const
{
	const PaneInfo &pi(pane(iPane));
	if (pi.pModelData && dynamic_cast<adcui::IBinViewModel *>(pi.pModelData->pIModel))
		return pi.pModelData;
	/*for (int i(0); i < pi.models.size(); i++)
	{
		adcui::IBinViewModel *p(dynamic_cast<adcui::IBinViewModel *>(pi.models[i]->pIModel));
		if (p)
			return pi.models[i];
	}*/
	return nullptr;
}

/*ADCModelData *PaneInfo::takeModel(adcui::IADCTextModel *p)
{
	for (QVector<ADCModelData *>::iterator i(models.begin()); i != models.end(); i++)
	{
		ADCModelData *pModelData(*i);
		if (pModelData->pIModel == p)
		{
			models.erase(i);
			return pModelData;
		}
	}
	return 0;
}*/

bool ADCSourceWin::setModelDataAt(int iPane, ADCModelData *pModelData)
{
	Q_ASSERT(pModelData);
	PaneInfo &pi(pane(iPane));
	pi.set(pModelData);
	/*if (dynamic_cast<adcui::ISrcViewModel *>(pModelData->pIModel))
	{
		Q_ASSERT(!srcModelData(iPane));
		pi.models.push_front(pModelData);
	}
	else
	{
		assert(dynamic_cast<adcui::IBinViewModel *>(pModelData->pIModel));
		Q_ASSERT(!binModelData(iPane));
		pi.models.push_front(pModelData);
	}*/
	return true;
}

ADCModelData *ADCSourceWin::setModel(ADCModelDataMap &rOwner, adcui::IADCTextModel *pIModel, int index)
{
	ADCDocPos *pCur(new ADCDocTablePos(pIModel));
	ADCModelData *pModelData(new ADCModelData(rOwner, pIModel, nullptr, pCur));
	setModelDataAt(index, pModelData);
	return pModelData;
}

/*void ADCSourceWin::slotProfileChanged()
{
	slotSyncPanesResponce2(mSyncLine, false);
}*/

ADCSourcePane *ADCSourceWin::createSourcePane(ADCSourcePane *pWin, ADCModelData *pModelData)
{
	//ADCSourcePane *pWin(new ADCSourcePane(this));
	pWin->create(pModelData);
	ADCSourceView *pView(pWin->view());
	pWin->updateFont(font());

	connect(pView, SIGNAL(signalSynchronize(int, bool)), SLOT(slotSyncPanesRequest2(int, bool)));
	//connect(pView, SIGNAL(signalViewDisassembly(QWidget *)), SLOT(slotViewDisassembly(QWidget *)));
	//connect(pView, SIGNAL(signalProfileChanged()), SLOT(slotProfileChanged()));
	//connect(pView, SIGNAL(signalGotFocus()), SLOT(slotCurrentViewChanged()));
	connect(pView, SIGNAL(signalCaretPosChanged(int, int, const ADCCurInfo &)), SIGNAL(signalCaretPosChanged(int, int, const ADCCurInfo &)));
	connect(pView, SIGNAL(signalPostCommand(const QString &, bool)), SIGNAL(signalPostCommand(const QString &, bool)));
	connect(pView, SIGNAL(signalSaveListing1(QString, QString)), SLOT(slotSaveListing(QString, QString)));
	connect(pView, SIGNAL(signalReleaseModel(QString)), SIGNAL(signalReleaseModel(QString)));
	connect(pView, SIGNAL(signalContextIdInq(uint &)), SIGNAL(signalContextIdInq(uint &)));
	connect(pView, SIGNAL(signalQuickPrototype()), SIGNAL(signalQuickPrototype()));
	connect(pView, SIGNAL(signalJumpSourceBack(bool)), SIGNAL(signalJumpSourceBack(bool)));

	connect(this, SIGNAL(signalSyncPanesResponce2(int, bool)), pView, SLOT(slotSyncPanesResponce3(int, bool)));
	connect(this, SIGNAL(signalSetFont(const QFont &)), pView, SLOT(slotSetFont(const QFont &)));
	connect(this, SIGNAL(signalClose(QString)), pView, SLOT(slotClose(QString)));
	connect(this, SIGNAL(signalFileInvalidated(int)), pView, SLOT(slotFileInvalidated(int)));
	connect(this, SIGNAL(signalRedump()), pView, SLOT(slotRedump()));
	connect(this, SIGNAL(signalUpdateContents()), pView, SLOT(slotRedraw()));
	connect(this, SIGNAL(signalDebuggerBreak()), pView, SLOT(slotDebuggerBreak()));
	connect(this, SIGNAL(signalCurOpChanged()), pView, SLOT(slotCurOpChanged()));
	connect(this, SIGNAL(signalFocusTaskTopAtLine(QString, int)), pView, SLOT(slotFocusTaskTopAtLine(QString, int)));
	connect(this, SIGNAL(signalContextIdChanged(unsigned)), pView, SLOT(slotContextIdChanged(unsigned)));
	//connect(this, SIGNAL(signalFocusTaskTopAtLine(QString, int)), pView, SLOT(slotGoToTaskTop(QString, int)));
	connect(pWin, SIGNAL(signalSwitchToBinaryView(QWidget *, int, bool)), SLOT(slotSwitchToBinaryView(QWidget *, int, bool)));
	connect(this, SIGNAL(signalGoToDeclaration()), pView, SLOT(slotGoToDeclaration()));
	connect(this, SIGNAL(signalGoToDefinition()), pView, SLOT(slotGoToDefinition()));
	connect(this, SIGNAL(signalGoToDeclarationRecoil(int, bool, bool)), pView, SLOT(slotGoToDeclarationRecoil(int, bool, bool)));
	return pWin;
}

void ADCSourceWin::slotCaretPosChanged(int x, int y, const ADCCurInfo &)
{
}

void ADCSourceWin::createBinaryPane(ADCBinaryPane *pWin, ADCModelData *pModelData)
{
	//if (!pModelData)
		//return;
	//ADCBinaryPane *pWin(new ADCBinaryPane(this));
	//connect(pWin, SIGNAL(signalSwitchToSourceView(QWidget*, int)), SLOT(slotSwitchToSourceView(QWidget*, int)));
	//connect(pWin, SIGNAL(signalSwitchToSourceViewLow(QWidget*, int)), SLOT(slotSwitchToSourceViewLow(QWidget*, int)));

//return pWin;
	pWin->create(pModelData);
	ADCBinView *pView(pWin->view());
	pWin->updateFont(font());
	pView->setModelHint(1);

	connect(pView, SIGNAL(signalSynchronize(int, bool)), SLOT(slotSyncPanesRequest2(int, bool)));
	connect(pView, SIGNAL(signalLocusInfo(QString, int)), SIGNAL(signalLocusInfo(QString, int)));
	connect(pView, SIGNAL(signalRefreshBinaryDump(int)), SIGNAL(signalRefreshBinaryDump(int)));
	connect(pView, SIGNAL(signalPostCommand(const QString &, bool)), SIGNAL(signalPostCommand(const QString &, bool)));

	connect(this, SIGNAL(signalSyncPanesResponce2(int, bool)), pView, SLOT(slotSyncPanesResponce3(int, bool)));
	connect(this, SIGNAL(signalSetFont(const QFont &)), pView, SLOT(slotSetFont(const QFont &)));
	connect(this, SIGNAL(signalGlobalsModified()), pView, SLOT(slotGlobalsModified()));
	connect(this, SIGNAL(signalLocusAdjusted()), pView, SLOT(slotLocusAdjusted()));
	connect(this, SIGNAL(signalAnalysisStarted()), pView, SLOT(slotAnalysisStarted()));
	connect(this, SIGNAL(signalFocusTaskTopAtLine(QString, int)), pView, SLOT(slotFocusTaskTopAtLine(QString, int)));
	connect(this, SIGNAL(signalUpdateContents()), pView, SLOT(slotRedraw()));
	connect(this, SIGNAL(signalDebuggerBreak()), pView, SLOT(slotDebuggerBreak()));
	connect(this, SIGNAL(signalCurOpChanged()), pView, SLOT(slotCurOpChanged()));

	
	connect(pView, SIGNAL(signalReleaseModel(QString)), SIGNAL(signalReleaseModel(QString)));
	//connect(pView, SIGNAL(signalContextIdInq(uint &)), SIGNAL(signalContextIdInq(uint &)));
	connect(this, SIGNAL(signalUpdateContents()), pView, SLOT(slotRedraw()));
	connect(this, SIGNAL(signalContextIdChanged(unsigned)), pView, SLOT(slotContextIdChanged(unsigned)));
	connect(this, SIGNAL(signalClose(QString)), pView, SLOT(slotClose(QString)));
}

void ADCSourceWin::slotDebuggerBreak()
{
	emit signalDebuggerBreak();
}

void ADCSourceWin::slotCurOpChanged()
{
	emit signalCurOpChanged();
}

void ADCSourceWin::createExtraActions(ADCSplitPane *pWin, bool bSwap)
{
	/*ADCBinView *pBinView(dynamic_cast<ADCBinView *>(pWin->view()));
	ADCSourceView *pSrcView(dynamic_cast<ADCSourceView *>(pWin->view()));
	if (pBinView)
	{
		ADCBinaryPane *pBinWin(dynamic_cast<ADCBinaryPane *>(pWin));

		QAction *pAction1(pBinWin->mpSwitchToSourceAction);
		QAction *pAction2(pBinWin->mpSwitchToLowAction);

		if (bSwap)
			qSwap(pAction1, pAction2);

		pWin->addToolbarAction(pAction1);
		pWin->addToolbarAction(pAction2);

	}
	else if (pSrcView)
	{
		ADCSourcePane *pSrcWin(dynamic_cast<ADCSourcePane *>(pWin));
		QAction *pAction1(pSrcWin->mpViewToggleProfileAction);

		QAction *pAction2(pSrcWin->mpSwitchToBinaryAction);
		pSrcView->addContextAction(pAction2);

		pWin->addToolbarAction(pAction1);
		pWin->addToolbarAction(pAction2);
	}*/
}

PaneInfo &ADCSourceWin::pane(uint i) const
{
	ADCSourceWin0 &rWin0(dynamic_cast<ADCSourceWin0 &>(*parent()));
	return rWin0.panes()[i];
}

ADCSplitPane *ADCSourceWin::openAltPane(const NewPane_t &in)//0:SRC, 1:DASM
{
	PaneKindEnum iType(in.iType);
//	Q_ASSERT(!mViews[in.index]);
	if (!(in.flags & NPF_FORCE))
#if(NEW_SPLITPANE)
		if (mViews[in.index])//has this pane been opened before?
			iType = dynamic_cast<ADCSourceView *>(mViews[in.index]->view()) ? PANE_SOURCE : PANE_BINARY;//adjust model type
#else
		if (pane(in.index).pModel)//has this pane been opened before?
			iType = dynamic_cast<adcui::ISrcViewModel *>(pane(in.index).pModel->pIModel) ? PANE_SOURCE : PANE_BINARY;//adjust model type
#endif

	ADCSplitPane *pWin0(nullptr);
	ADCSplitPane *pRet;
	if (iType == PANE_SOURCE || iType == PANE_SOURCE_LOW)
	{
		if (!srcModelData(in.index))
		{
			emit signalRequestModel(*this, mFileName, in.index, (in.flags & NPF_HINT) ? -1 : 0);
			adcui::ISrcViewModel *pISrcModel(static_cast<adcui::ISrcViewModel *>(srcModelData(in.index)->pIModel));
			pISrcModel->setMode(DUMP_UNFOLD, iType == PANE_SOURCE_LOW);
		}
		
		pWin0 = srcView(in.index);
		ADCSourcePane* pWin(pWin0 ? dynamic_cast<ADCSourcePane*>(pWin0) : nullptr);
		if (!pWin)
		{
			pWin = new ADCSourcePane(this);
			mViews[in.index] = pWin;
		}
		else
			pWin->clear();

		createSourcePane(pWin, srcModelData(in.index));

		//ADCSourceView* pView(pWin->view());
		//Q_ASSERT(pView);

		//pWin->mpViewToggleProfileAction = pView->mpViewToggleProfileAction;
		//if (!pWin0)
			//pWin->toolbar()->populate(this, 0);//in.index);

		//createExtraActions(pWin, false);
		
		//pView->setModel(srcModelData(in.index));
		if (pWin0)//if we just switched a tab, update scrollbars and focus
		{
			pWin->view()->setFocus();
//			pView->updateCurPosInDoc();
		}
		//pView->OnContentsChanged();
		pRet = pWin;
	}
	else//PANE_BINARY
	{
		if (!binModelData(in.index))
		{
			static const QByteArray cs(MODULE_SEP);
			int n(mFileName.indexOf(cs));
			QString moduleName(mFileName.left(n + cs.length()));

			emit signalRequestModel(*this, moduleName, in.index, 1);//in.hint);
		}
/*		if (!binModelData(in.index))
		{
			closeAlternativePane(in.index != 0);
			return nullptr;
		}*/

		pWin0 = binView(in.index);
		ADCBinaryPane* pWin(pWin0 ? dynamic_cast<ADCBinaryPane*>(pWin0) : nullptr);
		if (!pWin)
		{
			pWin = new ADCBinaryPane(this);
			mViews[in.index] = pWin;
		}
		else
			pWin->clear();

		createBinaryPane(pWin, binModelData(in.index));// in.bFromUnfold));

		//ADCBinView* pView(pWin->view());
		//Q_ASSERT(pView);
		//	return pWin;

		/*?		if (!pWin0)
				if (iType == PANE_BINARY_0)
				{
					pWin->clearExtraActions();
					pWin->toolbar()->populate(this, -1);
				//if (iType == PANE_BINARY)//!PANE_BINARY_0
					//createExtraActions(pWin, in.bFromUnfold);
				}
				else
					pWin->toolbar()->populate(this, (in.flags & NPF_FROM_UNFLD) != 0 ? 1 : 0);//in.index);
					*/

		//pView->setModelData(binModelData(in.index));
		pRet = pWin;
	}

	if (!pWin0)
	{
		ADCSplitPane *pView1(mViews[in.index]);//this view
		ADCSplitPane *pView2(mViews[(~in.index) & 1]);//other view
		openPane(in.index, pView1, pView2, (in.flags & NPF_SET_FOCUS) != 0);
	}
	/*else
	{
		ADCSplitPane *pView1(mViews[in.index]);//this view
		pView1->setFocus();
	}*/

	return pRet;
}

ADCSplitPane *ADCSourceWin::binView(int iPane) const
{
	if (!mViews[iPane])
		return nullptr;
	ADCBinView *pView(dynamic_cast<ADCBinView *>(mViews[iPane]->view()));
	if (!pView)
		return nullptr;
	return mViews[iPane];
}

ADCSplitPane *ADCSourceWin::srcView(int iPane) const
{
	if (!mViews[iPane])
		return nullptr;
	ADCSourceView *pView(dynamic_cast<ADCSourceView *>(mViews[iPane]->view()));
	if (!pView)
		return nullptr;
	return mViews[iPane];
}

void ADCSourceWin::slotSwitchToSourceView(QWidget *pWin, int)
{
	int i(activeViewIndex(pWin));
	if (i >= 0)
	{
		uint flags(NPF_FORCE|NPF_HINT);
		if (closeAltPane(i))
			flags |= NPF_SET_FOCUS;
		openAltPane(NewPane_t(i, PANE_SOURCE, flags));
//?		emit signalSyncPanesResponce2(mSyncLine, false);
	}
}

void ADCSourceWin::slotSwitchToSourceViewLow(QWidget *pWin, int)
{
	int i(activeViewIndex(pWin));
	if (i >= 0)
	{
		uint flags(NPF_FORCE|NPF_HINT);
		if (closeAltPane(i))
			flags |= NPF_SET_FOCUS;
		openAltPane(NewPane_t(i, PANE_SOURCE_LOW, flags));
//?		emit signalSyncPanesResponce2(mSyncLine, false);
	}
}

void ADCSourceWin::slotSwitchToBinaryView(QWidget *pWin, int iLine, bool fromUnfold)
{
	int i(activeViewIndex(pWin));
	if (i >= 0)
	{
		uint flags(NPF_FORCE|NPF_HINT);
		if (closeAltPane(i))
			flags |= NPF_SET_FOCUS;
		if (fromUnfold)
			flags |= NPF_FROM_UNFLD;
		openAltPane(NewPane_t(i, PANE_BINARY, flags));
		emit signalSyncPanesResponce2(iLine, false);
	}
}

void ADCSourceWin::slotSaveListing(QString, QString options)
{
	emit signalSaveListing2(mFileName, options);
}

void ADCSourceWin::updateActions(int fileKind)
{
#if(0)
	bool bIsSource(true);//fileKind == 2);
	mpToggleLeftPaneAction->setEnabled(bIsSource);
	mpToggleRightPaneAction->setEnabled(bIsSource);
	mpSyncPanesAction->setEnabled(bIsSource);
#endif
}

void ADCSourceWin::closeEvent(QCloseEvent *e)
{
//?	emit signalCloseDisplay(activeView()->fileId());
	ADCSourceWinBase::closeEvent(e);
}

void ADCSourceWin::slotCurrentViewChanged()
{
	//emit signalGotFocus(*this);
}

/*void ADCSourceWin::slotFileInvalidated(int fileId)
{
	for (int i(0); i < mpSplitter->count(); i++)
	{
		ADCSourceView *pView(dynamic_cast<ADCSourceView *>(mpSplitter->widget(i)));
		pView->slotFileInvalidated(fileId);
	}
}*/


void ADCSourceWin::locatePosition(QString task)
{
	//open binary view
	ADCBinaryPane *pWin(0);
	for (int i(0); i < mpSplitter->count(); i++)
		if (!pWin)
			pWin = dynamic_cast<ADCBinaryPane *>(mpSplitter->widget(i));
	if (!pWin)
		pWin = static_cast<ADCBinaryPane *>(openAltPane(NewPane_t(0, PANE_BINARY, NPF_FORCE)));
	//emit signalGoToLocation(sRefAddr, 20);
	emit signalFocusTaskTopAtLine(task, 20);
}

void ADCSourceWin::goToDeclarationRequest(bool bIsDefinintion)//DEPRECATED
{
	//open source view (if not open)
	ADCSourcePane *pWin(0);
	for (int i(0); i < mpSplitter->count(); i++)
		if (!pWin)
			pWin = dynamic_cast<ADCSourcePane *>(mpSplitter->widget(i));
	if (!pWin)
		pWin = static_cast<ADCSourcePane *>(openAltPane(NewPane_t(1, PANE_SOURCE, NPF_FORCE|NPF_SET_FOCUS)));
	if (bIsDefinintion)
		emit signalGoToDefinition();
	else
		emit signalGoToDeclaration();
}

void ADCSourceWin::goToDeclarationRecoil(int linesFromTop, bool bFwd, bool bFlip)
{
	//open source view (if not open)
	ADCSourcePane *pWin(0);
	for (int i(0); i < mpSplitter->count(); i++)
		if (!pWin)
			pWin = dynamic_cast<ADCSourcePane *>(mpSplitter->widget(i));
	if (!pWin)
		pWin = static_cast<ADCSourcePane *>(openAltPane(NewPane_t(1, PANE_SOURCE, NPF_FORCE|NPF_SET_FOCUS)));
	emit signalGoToDeclarationRecoil(linesFromTop, bFwd, bFlip);
}

void ADCSourceWin::aboutToClose()
{
	emit signalClose(mFileName);
}

/*void ADCSourceWin::slotFocusTaskTop(int fileId)
{
	//if (fileId != mFileId)
		//return;
	if (isVisible())
		locatePosition();
}*/

/*void ADCSourceWin::slotToggleSyncMode(bool bOn)
{
	emit signalSynchronizeWith(nullptr);
}*/

/*void ADCSourceWin::slotToggleColumn(int col)
{
	ADCSourceView *pView(dynamic_cast<ADCSourceView *>(activeView()));
	if (pView)
		pView->slotToggleColumn(col);
}*/

bool ADCSourceWin::isActiveWidget(QWidget *w0) const
{
	QWidget *w(focusWidget());
	while (w)
	{
		if (w == w0)
			return true;
		w = w->parentWidget();
	}
	return false;
}

int ADCSourceWin::activeViewIndex(QWidget *pWin0)
{
	for (int i(0); i < 2; i++)
	{
		QWidget *pWin(mViews[i]);
		if (pWin0)
		{
			if (pWin == pWin0)
				return i;
		}
		else
		{
			if (pWin && isActiveWidget(pWin))//pWin->hasFocus())
				return i;
		}
	}
	return -1;
}

/*QWidget *ADCSourceWin::activeView()
{
	for (int i(0); i < 2; i++)
		if (pane(i).pView && pane(i).pView->isActiveWindow())
			return pane(i).pView->view();
/ *	if (mpSplitter->count() == 0)
		return 0;
	QWidget *pView(mpSplitter->widget(0));
	if (pView->hasFocus())//isActiveWindow())
		return pView;
	if (mpSplitter->count() == 1)
		return 0;
	return mpSplitter->widget(1);* /
	return 0;
}*/

void ADCSourceWin::openAlternativePane(int iKind, bool bSide)
{
	uint flags(NPF_NONE);
	if (!bSide)
		flags |= NPF_HINT;
	openAltPane(NewPane_t(bSide, (PaneKindEnum)iKind, flags));//DASM by default
}

bool ADCSourceWin::closeAlternativePane(bool bSide)
{
	return closeAltPane(bSide);
}

void ADCSourceWin::keyPressEvent(QKeyEvent * e)
{
/*	switch (e->key()) 
	{
	case Qt::Key_Space:
		if (e->isAutoRepeat() || !mpSyncPanesAction->isEnabled())
			return;
		mpSyncPanesAction->setChecked(!mpSyncPanesAction->isChecked());
		//mpToolbar->setSyncPanes(!mpToolbar->isSyncPanesOn());
		//emit signalOverrideSyncMode(true);
	default:
		break;
	}*/

	ADCSourceWinBase::keyPressEvent(e);
}

void ADCSourceWin::keyReleaseEvent(QKeyEvent * e)
{
/*	switch (e->key()) 
	{
	case Qt::Key_Space:
		if (e->isAutoRepeat() || !mpSyncPanesAction->isEnabled())
			return;
		//emit signalOverrideSyncMode(true);
	default:
		break;
	}*/

	ADCSourceWinBase::keyReleaseEvent(e);
}

void ADCSourceWin::redump()
{
	emit signalRedump();
}

void ADCSourceWin::updateContents()
{
	emit signalUpdateContents();
}

void ADCSourceWin::assureSourcePane(QString task)
{
	//open source view
	ADCSourcePane *pWin(0);
	int iPane(0);
	for (; iPane < mpSplitter->count(); iPane++)
		if (!pWin)
			pWin = dynamic_cast<ADCSourcePane *>(mpSplitter->widget(iPane));
	if (!pWin)
	{
		assert(mpSplitter->count() == 1);
		int index(activeViewIndex(nullptr));
		ADCSplitPane *pWin1(dynamic_cast<ADCSplitPane *>(mpSplitter->widget(index)));

		pWin1->repopulateTollbar(this);

		int index2((~index) & 1);
		pWin = static_cast<ADCSourcePane *>(openAltPane(NewPane_t(index2, PANE_SOURCE, NPF_FORCE)));
		//pWin->toolbar0()->populate(this, index2);
//?		mpToggleRightPaneAction->setEnabled(true);
//?		mpToggleLeftPaneAction->setEnabled(true);
		//createExtraActions(dynamic_cast<ADCSplitPane *>(mpSplitter->widget(index)), false);
	}
	emit signalFocusTaskTopAtLine(task, 1);

	//emit signalGoToLocation(sRefAddr, 20);
	//emit signalFocusTaskTopAtLine(20);
}








//////////////////////////////////////////////////////

ADCSourceWin0::ADCSourceWin0(QWidget *parent, const char *name)
	: QWidget(parent),
	mpWin(nullptr)
{
	QVBoxLayout * vbox(new QVBoxLayout);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->setSpacing(0);
	setLayout(vbox);
}

ADCSourceWin0::~ADCSourceWin0()
{
}

void ADCSourceWin0::setWin(ADCSourceWin *pWin)
{
	if (mpWin)
		delete mpWin;
	mpWin = pWin;
	if (mpWin)
		layout()->addWidget(mpWin);
}

bool ADCSourceWin0::moveWinTo(ADCSourceWin0 *pOther)
{
	if (!mpWin || pOther == this)
		return false;
	if (!mpWin->fileKind())
		return false;
	pOther->setWin(mpWin);
	mpWin = nullptr;
	return true;
}

void ADCSourceWin0::updateOutputFont(const QFont &f)
{
	setFont(f);
	if (mpWin)
		mpWin->setFont(f);
}

void ADCSourceWin0::redump()
{
	if (mpWin)
		mpWin->redump();
}

void ADCSourceWin0::updateContents()
{
	if (mpWin)
		mpWin->updateContents();
}

void ADCSourceWin0::updateActions(int i)
{
	if (mpWin)
		mpWin->updateActions(i);
}

void ADCSourceWin0::assureSourcePane(QString s)
{
	if (mpWin)
		mpWin->assureSourcePane(s);
}

void ADCSourceWin0::locatePosition(QString s)
{
	if (mpWin)
		mpWin->locatePosition(s);
}

void ADCSourceWin0::goToDeclarationRequest(bool bIsDefinintion)
{
	if (mpWin)
		mpWin->goToDeclarationRequest(bIsDefinintion);
}

void ADCSourceWin0::goToDeclarationRecoil(int linesFromTop, bool bFwd, bool bFlip)
{
	if (mpWin)
		mpWin->goToDeclarationRecoil(linesFromTop, bFwd, bFlip);
}

void ADCSourceWin0::closeEvent(QCloseEvent *e)
{
	if (mpWin)
		mpWin->close();
		//mpWin->aboutToClose();
	QWidget::closeEvent(e);
}

