#pragma once

#include <QtGui/QIcon>
#include <QSplitter>
#include <QLayout>

//#include "qx/MyStream.h"
#include "ADCTableView.h"
#include "interface/IADCGui.h"
#include "ADCUtils.h"

class QPoint;
class QFont;
class QPaintEvent;
class QPixmap;
class QPainter;
class QScrollBar;
class QMenu;
class QLineEdit;
class QAction;
class ADCStream;
class ADCTypesView;
class ADCTypeCompleter;

#define SELWORD 0




typedef	ADCTableView	ADCBinViewBase;
//typedef	QAbstractScrollArea	ADCBinViewBase;


class ADCBinView : public ADCBinViewBase
{
	Q_OBJECT
private:
	typedef TModelLocker<ADCBinView> ModelLocker;
	//friend class ADCBinWinToolbar;

	adcui::IBinViewModel *binModel() const { return static_cast<adcui::IBinViewModel *>(modelData().pIModel); }//mpIModel; }
public:
	ADCBinView(QWidget * parent);
	virtual ~ADCBinView();
	//void init();
	adcui::IBinViewModel *reset2(QString);

	static bool checkActiveWin(QWidget *);

	virtual bool hasModel() const { return mpModelData != nullptr; }
	virtual ADCModelData &modelData() const { return *mpModelData; }
	typedef	adcui::IBinViewModel	ModelType;
	adcui::IBinViewModel *safeModel() const { return hasModel() ? binModel() : nullptr; }

	int	modelHint(){ return mModelHint; }
	void setModelHint(int n){ mModelHint = n; }
	//bool	go2Pos(adcui::ADDR addrD, int line = 0, bool bSaveJumpFrom = true);
	//bool	go2AddrV(adcui::ADDR addrV, int line = 0, bool bSaveJumpFrom = true);
	void	updateViewFromLocus();

	virtual int LineUp(int count);
	virtual int LineDown(int count);
	virtual void	SetTopAddrAbs(int);
	ADCModelData *setModelData(ADCModelData *);

	QAction *addContextAction(QAction *);// const QIcon &, QString, QString);
	template<typename Func>
	QAction* newAction(const QString& name, Func&& handler,
		const QKeySequence& key = QKeySequence(),
		const QIcon& icon = QIcon(),
		QWidget* parent = nullptr);
	QAction *newAction(QString name, const char *slot, const QKeySequence & = QKeySequence(), const QIcon & = QIcon(), QWidget * = nullptr);
	QAction *newActionCheck(QString name, const char *slot, const QKeySequence & = QKeySequence(), const QIcon & = QIcon(), QWidget * = nullptr);
	void setExtraMargin(QSize a){ mMarginExtra = a; }
	void updateDataFromLocus(int);
	void updateActions();

	void populateMenu(QMenu *) const;
	void populateColumnsMenu(QMenu*, QWidget* parent);

// Overrides
protected:
	virtual void	createContents();
	virtual QSize sizeHint () const { return QSize( 320, 240 ); }
	virtual void contextMenuEvent( QContextMenuEvent * );

	virtual bool event(QEvent* pEvent);
	virtual void showEvent(QShowEvent *);
	virtual void focusInEvent(QFocusEvent *);
	virtual void focusOutEvent(QFocusEvent *);
	virtual void paintEvent(QPaintEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void mouseDoubleClickEvent(QMouseEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void keyPressEvent(QKeyEvent *);
	virtual bool eventFilter(QObject *, QEvent *);
	virtual void wheelEvent(QWheelEvent *);
	virtual void closeEvent(QCloseEvent *);
	virtual void customEvent(QEvent *);

	int PrepareLine();

	//virtual int GetContentsWidth(){ return 256; }
	//virtual int GetContentsHeight(){ return SB_MAX; }

	virtual int scrollUpDown(int delta);
	
//	virtual void onNextLine();
//	virtual void onPrevLine();
//	virtual void onNextPage();
//	virtual void onPrevPage();
	//virtual void createEditor();

private:
	QList<QAction *>	mActions;
	virtual void enableContextActions(bool) override;
	virtual bool startInplaceEdit();
	virtual bool stopInplaceEdit();
	virtual void updateCurPosInDoc();
	void startInplaceTypeEdit();
	void stopInplaceTypeEdit();
	void emitSynchronize(bool bForce);
	void editName();

public slots:
	void slotReset();
	void slotRefresh();
	void slotRefresh0();//with echo to the output
	void slotGlobalsModified();
	void slotLocusAdjusted();
	//void slotGo2EntryPoint(){ go2EntryPoint(); }
	void slotSetCompact(bool);
	void slotHideTypes(bool);
	void slotResolveRefs(bool);
	//void slotTogglePackView(bool);
	void slotGoBack();
	void slotAnalysisStarted();
	void slotFocusTaskTopAtLine(QString, int);
	void slotRedraw();
	void slotCurOpChanged();
	void slotDebuggerBreak();

	//split win support
	void slotSyncPanesResponce3(int, bool);
	void slotSetFont(const QFont &);
	//void slotToggleSyncMode(bool);
	void slotContextIdChanged(unsigned);
	void slotClose(QString);

protected slots:
	virtual void slotVSliderValueChanged(int){}
	virtual void slotCopySelection();

	void slotVSBarAction(int);
	void slotHSBarAction(int);

	void slotDelete();
	void slotMakeCode();
	void slotMakeThunk();
	void slotMakeData();
	void slotToggleSign();
	void slotMakeReal();
	void slotMakeBit();
	void slotMakeAscii();
	void slotMakeText();
	void slotMakeUnicode();
	void slotMakeUntyped();
	void slotMakeClone();
	void slotMakeFunction();
	void slotSetFunctionEnd();
	void slotGoToLocation(QString, int);
	void slotName();
	
	void slotOffset();
	void slotDecompile();
	void slotProperties();
	void slotArray();
	void slotToggleCollapsed();

	void slotMap(int);
	void slotScopeView(QString);
	void slotSaveListing();
	void slotBuildType();
	void slotEnterType();
	void slotMakeArray();
	void slotMakeGap();
	void slotInstantiateType();
	void slotToggleExported();
	void slotToggleImported();
	void slotProjectModified();
	void slotDecompileFunction();
	void slotDisassemblyViewToggled(bool);
	void slotNothing();
	//void slotCancel();
	void slotGoToSelected();
	//void slotJumpTarget();
	void slotPrintObjInfo();
	void slotForceSynchronize();

	void slotNewScopeStruc();
	//void slotNewScopeUnion();
	//void slotNewScopeBitset();
	void slotNewScopeSeg();
	
signals:
	void signalSaveListing(QString);
	void signalPostCommand(const QString&, bool = true);
	void signalCallCommand(const QString&, ADCStream&);
	void signalJumpInfoStatusChanged(bool);
	void signalDisassemblyViewToggled(bool);
	void signalToggleView(QWidget *);
	void signalModelChanged();
	//void signalContextIdInq(uint &);
	void signalPickModuleType(QString);
	void signalRequestModel(ADCBinView &, QString);
	void signalRequestModel(ADCTypeCompleter &, QString);

	//split win support
	void signalSynchronize(int line, bool force);
	void signalRefreshBinaryDump(int);
	void signalLocusInfo(QString, int);
	void signalReleaseModel(QString);
	void signalOpenTypeDlg();
	void signalGoToLocation();

public:
	QAction* mpEditNameAction;
	QAction* mpMakeCodeAction;
	QAction* mpMakeThunkAction;
	QAction* mpMakeIntAction;
	QAction* mpToggleSignAction;
	QAction* mpMakeRealAction;
	QAction* mpMakeBitAction;
	QAction* mpSetFunctionEndAction;
	QAction* mpMakeAsciiAction;
	QAction* mpMakeTextAction;
	QAction* mpMakeUnicodeAction;
	QAction* mpMakeFunctionAction;
	QAction* mpMakeUntypedAction;
	QAction* mpMakeCloneAction;
	QAction* mpDeleteAction;
	QAction* mpGoToSelAction;
	QAction* mpGoBackAction;
	QAction* mpToggleCollapsedAction;
	QAction* mpToggleCompactViewAction;
	QAction* mpToggleValuesOnlyAction;
	QAction* mpMakeGapAction;
	QAction* mpInstantiateTypeAction;
	QAction* mpGoToAction;
	QAction* mpToggleExportedAction;
	QAction* mpToggleImportedAction;

	QAction* mpNewScopeSegAction;
	QAction* mpNewScopeStrucAction;
	//QAction* mpNewScopeUnionAction;
	//QAction* mpNewScopeBitsetAction;

	QAction* mpSaveListingAction;
	//QAction		*mpBuildTypeAction;
	QAction* mpEnterTypeAction;
	QAction* mpMakeArrayAction;
	QAction* mpMakeOffsetAction;
	//QAction	*mpDecompileFuncAction;
	QAction* mpResolveRefsAction;
	QAction* mpGotoEntryAction;
	QAction* mpRefreshAction;
	//QAction	*mpDisassemblyViewAction;
	//QAction	*mpJumpTargetAction;
	QAction* mpPrintObjInfoAction;
	QAction* mpSyncAction;

private:
#if(SELWORD)
	QString	m_sSelWord;
	QRect	m_rcSel;
#endif
	ADCModelData	*mpModelData;//weak
	int	mModelHint;
	QList<QAction *>	mExtraActions;
	//bool	m_bSyncMode;
	unsigned muContextId;

	static QWidget*	gpActiveWin;
};



////////////////////////////////////////////////////// ADCBinWinToolbar

class QComboBox;
class QLineEdit;
class QToolButton;

typedef ADCWinToolbar	ADCBinWinToolbarBase;

class ADCBinWinToolbar : public ADCBinWinToolbarBase
{
Q_OBJECT
public:
	ADCBinWinToolbar(QWidget *parent, ADCBinView*);
	void enableGoBackButton(bool bNonEmpty);
	void focusLocationBox();
	void enableControls(bool bEnable);
	void setFont(const QFont&);
protected:
	virtual bool eventFilter(QObject *, QEvent *);
protected slots:
	void slotGo2Location();
signals:
	void signalGoToLocation(QString, int);
public:
	//QToolButton*	mpPackViewTbtn;
	//QToolButton*	mpResolveRefsTbtn;
	QToolButton*	mpGotoEntryTbtn;
	QToolButton*	mpGoBackTbtn;
	QToolButton*	mpColumnsTbtn;
	QToolButton*	mpOptionsTbtn;
	//QLineEdit*		mpGotoLed;
	QComboBox*		mpGotoCbx;
	//QToolButton *	mpRefreshTbtn;
};

/*class ADCBinFuncDefBar : public QFrame
{
public:
	ADCBinFuncDefBar(QWidget *parent);
};*/




///////////////////////////////////////////////////////// ADCBinWin

typedef	QFrame	ADCBinWinBase;

class ADCBinWin : public ADCBinWinBase
{
Q_OBJECT
public:
	ADCBinWin(QWidget * parent, const char *name);
	virtual ~ADCBinWin();

	void createView(ADCModelData *);
	//bool init(adcui::IBinViewModel *);
	ADCBinView * view() const { return mpView; }
	ADCModelData *setModel(ADCModelDataMap &, adcui::IBinViewModel *);
	void setModelData(ADCModelData *);
	void takeModel();
	void locatePosition(QString);

protected:
	virtual void emitAllSignals(){}
	virtual void updateOutputFont(const QFont &);
	//virtual bool isClosable() { return false; }
	virtual void closeEvent(QCloseEvent *);

public slots:
	//void slotGoToLocation(QString, int);
	//void slotHighlightLocationAt(QString, QString, QString, int);
	//void slotProjectNew();
	void slotAboutToClose();
	//void slotReset();
	//void slotSync PanesResponce(int, bool);
	void slotUpdateViewFromLocus();

protected slots:
	void slotJumpStackStatusChanged(bool);
	void slotShowLocationBox();

signals:
	void signalSyncRequestAtLine(int, bool);
	void signalAnalysisStarted();
	//void signalRequestModel(ADCBinWin &, QString);
	void signalPostCommand(const QString &, bool);
	void signalCallCommand(const QString &, ADCStream&);
	void signalSaveListing(QString);
	void signalGoToLocation(QString, int);
	void signalHighlightLocationAt(QString, QString);
	void signalLocusInfo(QString, int);
	void signalGlobalsModified();
	void signalLocusAdjusted();
	void signalRefreshBinaryDump(int);
	void signalSyncPanesResponce2(int, bool);
	//void signalContextIdInq(uint &);
	//void signalRequestModel(ADCTypesView &, QString);
	void signalPickModuleType(QString);
	void signalRequestModel(ADCTypeCompleter &, QString);
	void signalReleaseModel(QString);
	void signalOpenTypeDlg();

	//bogus ones to shut Qt debug messaging up
	void signalPageClosed();
	void signalClosePage();
	void signalFileSaved(QWidget *, const QString&);
	void signalHandlePageMenu(QWidget *, const QPoint &);

protected:
	ADCBinView			*mpView;
	ADCBinWinToolbar	*mpToolbar;
	//ADCBinFuncDefBar	*mpFuncDefBar;
	//adcui::IBinViewModel *mpIModel;
	ADCModelData* mpModelData;
};

