#pragma once

#include <iostream>

#include <QtGui/QFont>
#include <QSplitter>
#include <QComboBox>
#include <QLayout>

#include "Sx/SxFindTextDlg.h"
#include "ADCTableView.h"
#include "ADCSplitWin.h"

class QScrollBar;
class QColor;
class QAction;
class QMenu;
class QSplitter;
class QToolButton;
class ADCBinView;
class IADCTableModel;
class ADCSourceWin;
class ADCBinaryPane;
class ADCSourcePane;
class MyAction;
struct ADCCurInfo;
class ADCBinaryPane;
class SxFindTextDlg;

#define NEW_SPLITPANE	1

namespace adcui { class ISrcViewModel; }

typedef	ADCTableView	ADCSourceViewBase;
//typedef	ADCTextView	ADCSourceViewBase;

#ifdef _DEBUG
#define	ENABLE_DEBUG_CONTROLS
#endif

class ADCSourceView : public ADCSourceViewBase
{
Q_OBJECT
private:
	class ModelLocker
	{
		adcui::ISrcViewModel *mpModel;
	public:
		ModelLocker(ADCSourceView *pView)
			: mpModel(pView->hasModel() ? pView->srcModel() : nullptr)
		{
			if (mpModel)
			{
				mpModel->AddRef();
				mpModel->lockRead(true);
			}
		}
		~ModelLocker()
		{
			if (mpModel)
			{
				mpModel->lockRead(false);
				mpModel->Release();
			}
		}
		adcui::ISrcViewModel* model() const { return mpModel; }
	};
	adcui::ISrcViewModel *srcModel() { return static_cast<adcui::ISrcViewModel *>(modelData().pIModel); }
	class FindTextObject : public SxFindTextDlg::ITarget
	{
		ADCSourceView* mpSelf;
	public:
		FindTextObject(ADCSourceView*);
		virtual bool findIt(const QString&, bool cs, bool wo, bool rev, bool reset) const override;
	};
public:
	friend class ADCSrcWinToolbar;
	friend class ADCSourcePane;
	friend class ADCSourceWin;
	ADCSourceView(QWidget * parent, const char *name = 0);//, const QString& fname);
	virtual ~ADCSourceView();
	adcui::ISrcViewModel *reset1(QString subjFullName, unsigned flags = 0);//subjFullName: module!subjScopedName
	void setModelData(ADCModelData *);//adcui::ISrcViewModel *pIModel);
	int fileId(){ return mFileId; }
//?	const QString &fileName(){ return mFileName; }
//?	void setFileName(const QString &s){ mFileName = s; }
//?	QString getAddress();
	QAction *addContextAction(QAction *);// const QIcon &, QString, QString);
	void createActions();
	void setContextId(unsigned u){
		muContextId = u;
	}
	void populateToolbarMenuUnfold(QMenu *);

protected:
	virtual void	createContents();
	//virtual adcui::IADCTextModel *textModel(){ return static_cast<adcui::IADCTextModel *>(mpIModel); }
	//virtual adcui::IADCTableModel *tableModel(){ return static_cast<adcui::IADCTableModel *>(mpIModel); }
	virtual ADCModelData &modelData() const { return *mpModelData; }
	virtual bool hasModel() const { return mpModelData != nullptr; }

protected:
	void clearActions();
	virtual void OnContentsChanged();
	virtual void setActive(bool);

	virtual bool event(QEvent *);
	virtual void showEvent(QShowEvent *);
	virtual void focusInEvent(QFocusEvent *);
	virtual void focusOutEvent(QFocusEvent *);
	virtual void contextMenuEvent(QContextMenuEvent *);
	//virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void keyPressEvent(QKeyEvent *);
	virtual void keyReleaseEvent(QKeyEvent *);
	virtual void paintEvent(QPaintEvent *);
	virtual void mouseDoubleClickEvent(QMouseEvent *);
	virtual void closeEvent(QCloseEvent * e);

	virtual bool startInplaceEdit();
	virtual bool stopInplaceEdit();

private:
	virtual void updateCurPosInDoc();
	bool checkDirtyContents();
	void updateActions();
	virtual void enableContextActions(bool);
	void blockContextActions();
	void checkAction(QAction *, uint, bool = true);
	void checkAction2(QAction *a, int col);
	void redumpModel(bool bAdjustCurLine);
	//void updateProfileAction();
	void emitSynchronize(bool bForce);
	
public slots:
	void slotUpdateDisplay(const QString& fname);
	//void slotToggleProfile();
	void slotToggleBlocked();
	//void slotSwitchToLowProfile();
	//void slotSwitchToHighProfile();
	void slotViewBlocked(bool);
	void slotFileInvalidated(int);
	void slotRedraw();
	void slotToggleColumn(int);
	void slotRefresh();
	void slotRefresh0();//with echo

	void slotShowXDepsIn(bool);
	void slotShowXDepsOut(bool);
	void slotShowXRefs(bool);
	void slotShowDeadCode(bool);
	void slotToggleArguments(bool);
	void slotShowDeadLabels(bool);
	void slotSyncPanesResponce3(int, bool);
	void slotCurOpChanged();
	void slotSetFont(const QFont &);
	//void slotToggleSyncMode(bool);
	void slotFocusTaskTopAtLine(QString, int);
	void slotRedump();
	void slotDebuggerBreak();
	void slotContextIdChanged(unsigned);
	void slotClose(QString);

protected slots:
	//void slotHorzSliderMoved(int nPos);
	//void slotVertSliderMoved(int nPos);
	//void slotHSliderValueChanged(int nPos);
	//void slotVSliderValueChanged(int nPos);

	void slotMakeInt();
	void slotMakeReal();
	void slotMakeBit();
	void slotMakePointer();
	void slotMakeConst();
	void slotUntype();
	void slotMakeArray();
	void slotMakeGap();
	void slotApplyType();
	void slotReconstructClassHierarchy();

	void slotInheritPublic();
	void slotInheritProtected();
	void slotInheritPrivate();
	void slotDisinherit();
	void slotToggleVFTablePtr();
	void slotToggleThisPtr();
	void slotToggleMethodVirtual();

	void slotConvertNamespace();
	void slotConvertClass();
	void slotConvertStruct();
	void slotConvertEnum();
	void slotConvertUnion();

	void slotToggleRoot();
	void slotToggleIf();
	void slotToggleElse();
	void slotToggleSwitch();
	void slotToggleWhile();
	void slotForLoop();
	void slotCombineLogic();
	void slotKillLogic();
	void slotExpand();
	void slotCollapse();
	void slotFlip();
	void slotBind();
	void slotUnbind();
	void slotAcquire();
	//void slotAcquireConstant();
	void slotCut();
	void slotPaste();
	void slotDelete();
	void slotEditName();
	//void slotCancel();
	//void slotViewDisassembly();
	void slotShowStrucLocs(bool bOn);
	void slotDecompile();
	//void slotRedecompile();
	void slotAnalyze();
	void slotGoToDeclaration();
	void slotGoToDefinition();
	void slotGoToDeclarationRecoil(int, bool, bool);
	void slotGoToLine();
	void slotFind();
	void slotGoBack();
	void slotForceSynchronize();
	//for debugging
	void slotDumpExpression();
	void slotPrintObjInfo();
	void slotPrintDumpInfo();
	void slotToggleDraftMode();
	void slotToggleVarOps();
	void slotTest();
	void slotSaveListing();

signals:
	void signalPostCommand(const QString &, bool);
	void signalRequestModel(ADCSourceView &, QString);
	void signalEnableToggleRoot(bool);
	void signalEnableToggleIf(bool);
	void signalEnableToggleElse(bool);
	void signalEnableToggleWhile(bool);
	void signalEnableCombineLogic(bool);
	void signalEnableKillLogic(bool);
	void signalEnableDelete(bool);
	//void signalEnableSynchronize();
	void signalSynchronize(int, bool);
	//void signalGotFocus();
	void signalViewDisassembly();
	void signalViewDisassembly(QWidget *);
	void signalOverrideSyncMode(bool);
	void signalCaretPosChanged(int, int, const ADCCurInfo &);
	void signalProfileChanged();
	void signalSaveListing1(QString, QString);
	void signalReleaseModel(QString);
	void signalContextIdInq(uint &);
	void signalQuickPrototype();
	void signalJumpSourceBack(bool);
	
private:
	QList<MyAction *>	mActions;

	enum ScopingFlags {//must match ContextIdEnum!
		SCOPING_NULL,
		//scope
		SCOPING_STRUC__MASK			= 0x00FF,//a mask
			SCOPING_STRUC_HEADER	= 0x0001,
			SCOPING_STRUC_DATA		= 0x0002,
			SCOPING_STRUC_METHOD	= 0x0004,
			SCOPING_STRUC_STATIC	= 0x0008,//v-tables, rtti?
			SCOPING_STRUC_BODY		= 0x0080,//anywhere else

		SCOPING_FUNC__MASK			= 0xFF00,//a mask
			SCOPING_FUNC_HEADER		= 0x0100,
			SCOPING_FUNC_OP			= 0x0200,
			SCOPING_FUNC_LOCAL		= 0x0400,
			SCOPING_FUNC_LABEL		= 0x0800,
			SCOPING_FUNC_BODY		= 0x8000,//anywhere else

		SKOPING__MASK				= 0xFFFF
	};

	friend class MyAction;
	static ScopingFlags ToScopingFlags(unsigned);
	QAction *newAction(QString name, const char *slot, const QKeySequence & = QKeySequence(), const QIcon & = QIcon(), QWidget * = nullptr);
	QAction *newAction_F(QString name, const char *slot, const QKeySequence &, const QIcon &, ScopingFlags);//code only
	//QAction *newAction_S(QString name, const char *slot, const QKeySequence & = QKeySequence(), const QIcon & = QIcon(), QWidget * = nullptr);//data only
	//QAction *newAction_SH(QString name, const char *slot, const QKeySequence & = QKeySequence(), const QIcon & = QIcon(), QWidget * = nullptr);//data only
	QAction *newActionCheck(QString name, const char *slot, const QKeySequence & = QKeySequence(), const QIcon & = QIcon(), QWidget * = nullptr);

	//code
	QAction	*mpDecompileAction;
	QAction	*mpAnalyzeAction;
	QAction	*mpQuickProtoAction;
	QAction *mpRootAction;
	QAction *mpIfAction;
	QAction *mpElseAction;
	QAction *mpWhileAction;
	QAction *mpForAction;
	QAction *mpLogicAction;
	QAction *mpLogicUndoAction;
	QAction *mpExpandAction;
	QAction *mpCollapseAction;
	QAction *mpFlipAction;
	QAction	*mpSwitchAction;
	QAction	*mpBindAction;
	QAction	*mpUnbindAction;
	QAction* mpAcquireAction;
	//QAction* mpAcquireConstAction;
	QAction	*mpDeleteAction;
	QAction	*mpCutAction;
	QAction	*mpPasteAction;

	//data
	QAction	*mpMakeIntAction;
	QAction	*mpMakeRealAction;
	QAction	*mpMakePointerAction;
	QAction	*mpMakeConstAction;
	QAction	*mpUntypeAction;
	QAction	*mpMakeArrayAction;
	QAction	*mpApplyTypeAction;
	QAction	*mpHandleRuntimeTypeInfoAction;
	QAction	*mpMakeGapAction;

	QAction	*mpAddPublicAction;//inheritance
	//QAction	*mpAddProtectedAction;
	//QAction	*mpAddPrivateAction;
	QAction	*mpLowerInherLevelAction;
	QAction	*mpToggleVFTablePtrAction;
	QAction	*mpToggleThisPtrAction;
	QAction	*mpToggleVirtualAction;
	
	QAction	*mpMakeNamspaceAction;
	QAction	*mpMakeClassAction;
	QAction	*mpMakeStructAction;
	QAction	*mpMakeEnumAction;
	QAction	*mpMakeUnionAction;

	//other
	QAction *mpNameAction;
	QAction *mpGotoDeclarationAction;
	QAction *mpGotoDefinitionAction;
	QAction *mpGotoLineAction;
	QAction *mpFindAction;
	QAction *mpGoBackAction;
	//QAction *mpCancelAction;
	QAction	*mpSaveListingAction;
	//QAction *mpViewDisassemblyAction;
	//QAction *mpViewLowProfileAction;
	//QAction *mpViewHighProfileAction;
	//QAction *mpViewToggleProfileAction;
	//QAction *mpViewToggleBlockedAction;
	QAction	*mpFlatViewAction;
	QAction	*mpShowXRefsAction;
	QAction *mpToggleInflowAction;
	QAction	*mpToggleOutflowAction;
	QAction	*mpToggleLostCodeAction;
	QAction	*mpToggleLostLabelsAction;
	QAction	*mpToggleArgsAction;
	//QAction *mpSyncAction;
	QAction *mpToggleLinesAction;
	QAction *mpToggleStackAction;
	QAction *mpToggleFpuAction;
	QAction *mpTogglePathsAction;
	QAction *mpRefreshAction;
	//QAction	*mpDecompileAction;
	QAction	*mpSyncAction;
#ifdef ENABLE_DEBUG_CONTROLS
	QAction	*mpDumpExprAction;
	QAction	*mpShowStrucLocsAction;
	QAction	*mpPrintObjInfoAction;
	QAction	*mpPrintDumpInfoAction;
	QAction *mpToggleDraftModeAction;
	QAction *mpToggleVarOpsAction;
	QAction *mpTestAction;
#endif

private:

	int		mFileId;
//?	QString	mFileName;

	ADCModelData *mpModelData;//weak
	QList<QAction *>	mExtraActions;
	unsigned muContextId;
};



////////////////////////////////////////////////////
// (toolbars)

class ADCSrcWinToolbar : public ADCSplitToolbar
{
	Q_OBJECT
public:
	ADCSrcWinToolbar(ADCSourcePane *);
	bool isSyncPanesOn();
	void setSyncPanes(bool);
signals:
	void signalSyncToggled(bool);
private:
	/*QToolButton *mpToggleInflowTBtn;
	QToolButton *mpToggleOutflowTBtn;
	QToolButton *mpToggleLostCodeTBtn;
	QToolButton *mpToggleLinesTBtn;
	QToolButton *mpToggleStackTBtn;
	QToolButton *mpToggleFpuTBtn;
	QToolButton *mpTogglePathsTBtn;*/
	//QToolButton *mpRefreshTbtn;
	//QMenu *mpLostCodePopup;
public:
	virtual void populate(ADCSplitWin *, int);
	virtual void cleanup();
};



/////////////////////////////////////////////////
// (panes)

class ADCSourcePane : public ADCSplitPane
{
Q_OBJECT
public:
	ADCSourcePane(QWidget *parent);
	ADCSourceView* view(){ return (ADCSourceView *)mpView; }
	ADCSrcWinToolbar* toolbar(){ return (ADCSrcWinToolbar *)mpToolbar; }
	virtual void updateFont(const QFont&);
protected:
	virtual void createView(ADCModelData *);
	virtual void createToolbar(ADCModelData *);
	virtual void updateActions(int);
public slots:
	void slotSwitchToBinaryView();
signals:
	void signalSwitchToBinaryView(QWidget *, int, bool);
public:
	//QAction	*mpViewToggleProfileAction;
	//QAction *mpSwitchToBinaryAction;
};


typedef ADCSplitWin ADCSourceWinBase;

//--------------------------------------------
class ADCSourceWin : public ADCSourceWinBase
{
Q_OBJECT
	friend class ADCSplitToolbar;
public:
	ADCSourceWin(QWidget *parent, const char *name);
	virtual ~ADCSourceWin();
	//ADCSourceView *view(){ return mpView; }
	//void setModels(adcui::ISrcViewModel *, adcui::ISrcViewModel *);
	int fileId() const { return mFileId; }
	int fileKind() const { return mFileKind; }
	void init(int fileId, int fileKind);//0:binary, 1:header, 2:source
	virtual ADCModelData *setModel(ADCModelDataMap &, adcui::IADCTextModel *, int);
	void redump();
	void updateContents();//repaint
	void locatePosition(QString);
	void goToDeclarationRequest(bool bIsDefinintion);
	void goToDeclarationRecoil(int linesFromTop, bool fwd, bool flip);
	void updateActions(int fileKind);
	void assureSourcePane(QString);
	const QString &fileName(){ return mFileName; }
	void setFileName(const QString &s){ mFileName = s; }
	void setFont(const QFont &f){ updateOutputFont(f); }
	void aboutToClose();
	virtual bool setModelDataAt(int, ADCModelData *);

	void createBinaryPane(ADCBinaryPane *, ADCModelData *);
	//virtual ADCModelData *setModel(ADCModelDataMap &, adcui::IADCTextModel *, int) = 0;
	static SxFindTextDlg  *getFindDlg(QWidget *);
	static void destroyFindDlg();

protected:
	virtual void OnAboutToClose(QWidget *);
	
protected:
	virtual void closeEvent(QCloseEvent * e);
	virtual void keyPressEvent(QKeyEvent *);
	virtual void keyReleaseEvent(QKeyEvent *);

	virtual void openAlternativePane(int, bool);
	virtual bool closeAlternativePane(bool);
	//virtual void OnAboutToClose(QWidget *);

private:
	ADCSplitPane *binView(int iPane) const;//returns ptr if a view at iPane is binary
	ADCSplitPane *srcView(int iPane) const;//returns ptr if a view at iPane is source
	//QWidget *activeView();
	int activeViewIndex(QWidget *);
	void createExtraActions(ADCSplitPane *, bool);
	virtual PaneInfo &pane(uint) const;

	enum NewPaneFlags
	{
		NPF_NONE		= 0,
		NPF_FORCE		= 0x01,
		NPF_FROM_UNFLD	= 0x02,
		NPF_SET_FOCUS	= 0x04,
		NPF_USE_LOCUS	= 0x08,//try positioning at current locus (do not reset locus)
		NPF_HINT		= 0x10
	};

	struct NewPane_t
	{
		int index;
		PaneKindEnum iType;
		uint flags;

		NewPane_t(int _index, PaneKindEnum _iType, uint _flags)
			: index(_index), iType(_iType), flags(_flags)
		{
		}
	};
	ADCSplitPane *openAltPane(const NewPane_t &);
	ADCSourcePane *createSourcePane(ADCSourcePane *, ADCModelData *);
	//ADCBinaryPane *createBinaryPane();
	bool isActiveWidget(QWidget *) const;

	ADCModelData *srcModelData(int) const;
	ADCModelData *binModelData(int) const;

//public slots:
	//void slotFileInvalidated(int);
	//void slotFocusTaskTop(int);

private slots:
	//void slotToggleColumn(int);
	void slotCurrentViewChanged();
	//void slotEnableSynchronize();
	//void slotViewSource(QWidget *);
	//void slotViewDisassembly(QWidget *);
	//void slotSwitchToBinView();
	//void slotOverrideSyncMode(bool);
	void slotSwitchToBinaryView(QWidget *, int, bool);
	void slotSwitchToSourceView(QWidget *, int);
	void slotSwitchToSourceViewLow(QWidget *, int);
	//void slotProfileChanged();
	void slotSaveListing(QString, QString);

	void slotDebuggerBreak();
	void slotCurOpChanged();
	void slotCaretPosChanged(int, int, const ADCCurInfo &);

signals:
	void signalRequestModel(ADCSourceWin &, QString, int, int);
	void signalCloseDisplay(int);
	//void signalRefresh();
	void signalFileInvalidated(int);
	//void signalExprInvalidated();
	void signalRedump();
	void signalQuickPrototype();
	void signalJumpSourceBack(bool);
	
	void signalSynchronizeWith(ADCSourceView *);
	void signalGoToLocation(QString, int);
	void signalCaretPosChanged(int, int, const ADCCurInfo &);
	void signalContextIdInq(uint &);
	//void signalGotFocus(ADCSourceWin &);
	void signalContextIdChanged(unsigned);
	void signalSaveListing2(QString, QString);
	void signalLocusAdjusted();
	void signalClose(QString);
	void signalReleaseModel(QString);

	void signalRefreshBinaryDump(int);
	void signalPostCommand(const QString &, bool = true);
	void signalGlobalsModified();
	//void signalLocusAdjusted();
	void signalAnalysisStarted();
	void signalFocusTaskTopAtLine(QString, int);
	void signalGoToDeclaration();
	void signalGoToDefinition();
	void signalGoToDeclarationRecoil(int, bool, bool);
	void signalUpdateContents();
	void signalDebuggerBreak();
	void signalCurOpChanged();
	void signalLocusInfo(QString, int);

private:
	int	mFileId;
	int	mFileKind;
	QString	mFileName;
	static SxFindTextDlg* spFindTextDlg;
};



class ADCSourceWin0 : public QWidget
{
	Q_OBJECT
public:
	ADCSourceWin0(QWidget *parent, const char *name);
	virtual ~ADCSourceWin0();
	void setWin(ADCSourceWin *);
	ADCSourceWin *win() const { return mpWin; }
	bool moveWinTo(ADCSourceWin0 *);
	void redump();
	void updateContents();
	void updateActions(int);
	void assureSourcePane(QString);
	void locatePosition(QString);
	void goToDeclarationRequest(bool bIsDefinintion);
	void goToDeclarationRecoil(int linesFromTop, bool fwd, bool flip);
	const PaneInfo (&panes() const)[2] { return mPanes; }
	PaneInfo (&panes())[2]{ return mPanes; }

protected:
	virtual void emitAllSignals(){}
	virtual void updateOutputFont(const QFont &);
	virtual void closeEvent(QCloseEvent *);

signals:
	//bogus ones to shut Qt debug messaging up
	void signalPageClosed();
	void signalClosePage();
	void signalFileSaved(QWidget *, const QString&);
	void signalHandlePageMenu(QWidget *, const QPoint &);
private:
	ADCSourceWin *mpWin;
	PaneInfo mPanes[2];
};


