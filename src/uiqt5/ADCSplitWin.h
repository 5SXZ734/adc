#pragma once

#include <QFrame>
#include <QLayout>
#include <QSplitter>

#include "ADCUtils.h"

class ADCSplitWin;
class ADCSplitToolbar;
class ADCBinView;
class QToolButton;
class QComboBox;
class ADCModelData;
class ADCModelDataMap;

namespace adcui { class IADCTextModel; }

/////////////////////////////////////////
// (bases)

class ADCSplitPane : public QWidget
{
	Q_OBJECT
	friend class ADCSplitToolbar;
protected:
	QWidget* mpToolbar;
	QWidget* mpView;
	QList<QAction *> mActions;
public:
	ADCSplitPane(QWidget *);
	void create(ADCModelData *);
	
	QWidget *view() const { return mpView; }
	QWidget* toolbar0(){ return mpToolbar; }
	virtual void updateActions(int) = 0;
	virtual void paneToggled(int, bool);
	virtual void clear();
	virtual void updateFont(const QFont& f) { setFont(f); }
	QAction *action(uint i) const { return mActions.at(i); }
	void addSwitchAction(QAction *p, bool bPrepend = false)
	{
		if (bPrepend)
			mActions.push_front(p);
		else
			mActions.push_back(p);
	}
	void addToolbarAction(QAction *);
	QAction *newAction(QString name, const char *slot, const QKeySequence &key, const QIcon &icon);

protected:
	virtual void createView(ADCModelData *) = 0;
	virtual void createToolbar(ADCModelData *) = 0;
	virtual void closeEvent(QCloseEvent *);
public:
	virtual void repopulateTollbar(ADCSplitWin *){}

public slots:
	virtual void slotModelChanged() {}
};





class ADCSplitToolbar : public ADCWinToolbar
{
public:
	QToolButton *mpToggleLeftPaneTbtn;
	QToolButton *mpToggleRightPaneTbtn;
	QToolButton *mpSyncPanesTBtn;

	QList<QToolButton *>	mSwitchButtons;

public:
	ADCSplitToolbar(ADCSplitPane *parent);

	void addToolbarAction(QAction *);
	void showPaneToggleButtons(bool);
	void showSyncPanesButton(bool);
	void clearPaneToggleButtons();

	virtual void populate(ADCSplitWin *, int index);
	virtual void cleanup();
};


//////////////////////////////////////////////////
/*class ADCBinaryPane;

class ADCBinaryPaneToolbar : public ADCSplitToolbar
{
	Q_OBJECT
public:
	ADCBinaryPaneToolbar(ADCBinaryPane *);
	virtual void populate(ADCSplitWin *, int index);
	virtual void cleanup();
	void enableControls(bool bEnable);
private slots:
	void slotGo2Location();
signals:
	void signalGoToLocation(QString, int);
public:
	//QToolButton*	mpPackViewTbtn;
	QToolButton*	mpResolveRefsTbtn;
	QToolButton*	mpGoBackTbtn;
	QToolButton*	mpGotoEntryTbtn;
	//QToolButton*	mpRefreshTbtn;
	QComboBox*		mpGotoCbx;
};*/

class ADCBinWinToolbar;
typedef ADCBinWinToolbar	ADCBinaryPaneToolbar;

enum PaneKindEnum { PANE_BINARY_0, PANE_SOURCE, PANE_BINARY, PANE_SOURCE_LOW };

struct PaneInfo
{
	ADCModelData *pModelData;
	PaneInfo();
	~PaneInfo();
	void set(ADCModelData *);
};

typedef QSplitter ADCSplitWinBase;

//--------------------------------------------
class ADCSplitWin : public ADCSplitWinBase
{
Q_OBJECT
	friend class ADCSplitToolbar;
public:
	ADCSplitWin(QWidget *parent, const char *name);
	virtual ~ADCSplitWin();
	void aboutToClose();
	virtual bool setModelDataAt(int, ADCModelData *) { return false; }
	bool isLeftPaneOn() const { return mViews[0] != nullptr; }
	bool isRightPaneOn() const { return mViews[1] != nullptr; }
	void toggleLeftPane();
	void toggleRightPane();
	//void synchronizeAtLine(int);
	//void setSyncMode(bool b){ mbSyncMode = b; }

protected:
	virtual void emitAllSignals(){}
	virtual void updateOutputFont(const QFont &);
	virtual void openAlternativePane(int, bool){}
	virtual bool closeAlternativePane(bool bSize) { return closeAltPane(bSize); }//false:left,true:right
	virtual void OnAboutToClose(QWidget *) {}
	virtual void closeEvent(QCloseEvent *);
	virtual void keyPressEvent(QKeyEvent*);
	virtual void focusInEvent(QFocusEvent*);
	virtual bool event(QEvent*);
	virtual bool focusNextPrevChild(bool);
	
	void openPane(int index, ADCSplitPane *pView1, ADCSplitPane *pView2, bool bSetFocus);
	bool closeAltPane(int);

private:
	//void createActions();
	virtual PaneInfo &pane(uint) const = 0;

public slots:
	//void slotSyncModeChanged(bool);
	void slotSwitchPane();

protected slots:
	//void slotRefLineChanged(int);
	void slotSyncPanesRequest2(int, bool);
	void slotSyncPanesResponce2(int, bool);

private slots:
	void slotToggleLeftPane();
	void slotToggleRightPane();

signals:
	void signalSetFont(const QFont &);
//?	void signalToggleSyncMode(bool);
	void signalSyncPanesRequest2(int, bool);
	void signalSyncPanesResponce2(int, bool);
	//void signalSyncModeInquiry(bool &);

	//bogus ones to shut Qt debug messaging up
	void signalPageClosed();
	void signalClosePage();
	void signalFileSaved(QWidget *, const QString&);
	void signalHandlePageMenu(QWidget *, const QPoint &);

protected:
	QSplitter* mpSplitter;
//	QAction* mpToggleLeftPaneAction;
//	QAction* mpToggleRightPaneAction;
//	QAction* mpSyncPanesAction;
	//QAction	*mpRefreshAction;
	//int mSyncLine;
	QList<int> mLastSplitSize;
	int mDefaultPanes[2];//kinds
	//PaneInfo (&mPanes)[2];
	ADCSplitPane *mViews[2];
	//bool	mbSyncMode;
};



///////////////////////////////////////

class ADCBinaryPane : public ADCSplitPane
{
Q_OBJECT
public:
	ADCBinaryPane(QWidget *parent);
	ADCBinView* view() const { return (ADCBinView*)mpView; }
	ADCBinaryPaneToolbar* toolbar() const { return (ADCBinaryPaneToolbar*)mpToolbar; }
	void clearExtraActions();
	void createExtraActions();
	virtual void updateFont(const QFont&);
protected:
	virtual void createView(ADCModelData *);
	virtual void createToolbar(ADCModelData*);
	virtual void repopulateTollbar(ADCSplitWin *);
public slots:
	//void slotSwitchToSourceView();
	//void slotSwitchToSourceViewLow();
	virtual void slotModelChanged();
//signals:
	//void signalSwitchToSourceView(QWidget *, int);
	//void signalSwitchToSourceViewLow(QWidget *, int);
protected:
	virtual void updateActions(int){}
public:
	//QAction *mpSwitchToSourceAction;
	//QAction *mpSwitchToLowAction;
};



/*class ADCDualWin : public ADCSplitWin
{
Q_OBJECT
public:
	ADCDualWin(QWidget *parent, const char *name);
	virtual ~ADCDualWin();


protected slots:



};*/


