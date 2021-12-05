#include <QtGui/QCloseEvent>
#include <QAction>
#include <QToolButton>
#include <QComboBox>
#include <QLineEdit>

#include "ADCSplitWin.h"
#include "ADCBinWin.h"




//////////////////////////////////////////////////////////////
// ADCSplitPane

ADCSplitPane::ADCSplitPane(QWidget *parent)
	: QWidget(parent),
	mpToolbar(nullptr),
	mpView(nullptr)
{
	QLayout* vbox(new QVBoxLayout);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->setSpacing(0);
	setLayout(vbox);

//	setFocusPolicy(Qt::TabFocus);
}

void ADCSplitPane::create(ADCModelData *pModelData)
{
//	assert(!layout());
	QLayout* vbox = layout();

	createView(pModelData);

	//connect(mpView, SIGNAL(signalModelChanged()), SLOT(slotModelChanged()));

	//mpView->setModelData(pModelData);

	createToolbar(pModelData);

	if (mpToolbar)
		vbox->addWidget(mpToolbar);
	if (mpView)
		vbox->addWidget(mpView);
}

QAction *ADCSplitPane::newAction(QString name, const char *slot, const QKeySequence &key, const QIcon &icon)
{
	QAction *pAction(new QAction(icon, name, this));
	mActions.append(pAction);
	pAction->setShortcut(key);
	pAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(pAction, SIGNAL(triggered()), this, slot);
	return pAction;
}


void ADCSplitPane::paneToggled(int /*index*/, bool /*bSplitMode*/)
{
	/*if (mpToolbar)
	if (!bSplitMode)
	{
		mpToolbar->showPaneToggleButtons(true);
		mpToolbar->showSyncPanesButton(true);
	}
	else
	{
		mpToolbar->showPaneToggleButtons(index == 0);
		mpToolbar->showSyncPanesButton(index == 0);
		//view()->emitSynchronize();
	}*/
}

void ADCSplitPane::clear()
{
	mActions.clear();
	delete mpToolbar;
	delete mpView;
	mpToolbar = nullptr;
	mpView = nullptr;
	//delete takeLayout();
}

void ADCSplitPane::addToolbarAction(QAction *)//p)
{
	/*if (mpToolbar)
		mpToolbar->addToolbarAction(p);*/
}

void ADCSplitPane::closeEvent(QCloseEvent *e)
{
	mpView->close();
	e->ignore();
}





/////////////////////////////////////////////////////////////
// ADCSplitToolbar

ADCSplitToolbar::ADCSplitToolbar(ADCSplitPane *parent)
	: ADCWinToolbar(parent),
	mpToggleLeftPaneTbtn(nullptr),
	mpToggleRightPaneTbtn(nullptr),
	mpSyncPanesTBtn(nullptr)
{
}

void ADCSplitToolbar::populate(ADCSplitWin *pMainWin, int )
{
	(void)pMainWin;
	//if (index == 0)
/*	{
		mpToggleLeftPaneTbtn = addButton(pMainWin->mpToggleLeftPaneAction);
		//mpToggleLeftPaneTbtn->setShown(index == 0);

		mpToggleRightPaneTbtn = addButton(pMainWin->mpToggleRightPaneAction);
		//mpToggleRightPaneTbtn->setShown(index == 0);

		mpSyncPanesTBtn = addButton(pMainWin->mpSyncPanesAction);
		//mpSyncPanesTBtn->setShown(index == 0);
	}*/

	addStretch();
}

void ADCSplitToolbar::cleanup()
{
	ADCWinToolbar::cleanup();
	clearPaneToggleButtons();
}

void ADCSplitToolbar::clearPaneToggleButtons()
{
	delete mpToggleLeftPaneTbtn;
	mpToggleLeftPaneTbtn = nullptr;
	delete mpToggleRightPaneTbtn;
	mpToggleRightPaneTbtn = nullptr;
	delete mpSyncPanesTBtn;
	mpSyncPanesTBtn = nullptr;
}

void ADCSplitToolbar::showPaneToggleButtons(bool b)
{
	if (mpToggleLeftPaneTbtn)
		mpToggleLeftPaneTbtn->setVisible(b);
	if (mpToggleRightPaneTbtn)
		mpToggleRightPaneTbtn->setVisible(b);
}

void ADCSplitToolbar::showSyncPanesButton(bool b)
{
	if (mpSyncPanesTBtn)
		mpSyncPanesTBtn->setVisible(b);
}








/////////////////////////////////////////////////////////////
// ADCBinaryPaneToolbar

/*ADCBinaryPaneToolbar::ADCBinaryPaneToolbar(ADCBinaryPane *parent)
	: ADCSplitToolbar(parent)
{
}

void ADCBinaryPaneToolbar::populate(ADCSplitWin *pMainWin, int)// bSwap)
{
	ADCSplitToolbar::populate(pMainWin, 0);

	ADCBinaryPane *pWin(dynamic_cast<ADCBinaryPane *>(parent()));
	ADCBinView* pView(pWin->view());

	//mpPackViewTbtn = addButton(pView->mpTogglePackViewAction);
	//mpResolveRefsTbtn = addButton(pView->mpResolveRefsAction);
	mpGoBackTbtn = addButton(pView->mpGoBackAction);

	mpGotoCbx = addComboBox();
	mpGotoCbx->setEditable(true);
	mpGotoCbx->setSizePolicy(QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed));
	mpGotoCbx->setMinimumSize(QSize(120, 0));
	mpGotoCbx->setDuplicatesEnabled(false);
	mpGotoCbx->setInsertPolicy(QComboBox::InsertAtTop);
	//?mpGotoCbx->installEventFilter(this);
	connect(mpGotoCbx->lineEdit(), SIGNAL(returnPressed()), SLOT(slotGo2Location()));
	mpGotoCbx->setToolTip(tr("Go To Location"));

	mpGotoEntryTbtn = addButton(pView->mpGotoEntryAction);
	connect(mpGotoEntryTbtn, SIGNAL(clicked()), SLOT(slotGo2Location()));

	connect(this, SIGNAL(signalGoToLocation(QString, int)), pView, SLOT(slotGoToLocation(QString, int)));

	/ *QAction *pAction1(pWin->mpSwitchToSourceAction);
	QAction *pAction2(pWin->mpSwitchToLowAction);

	if (bSwap)
		qSwap(pAction1, pAction2);

	pWin->addToolbarAction(pAction1);
	pWin->addToolbarAction(pAction2);* /

	enableControls(false);
}


void ADCBinaryPaneToolbar::cleanup()
{
	ADCSplitToolbar::cleanup();
	while (QWidget* w = findChild<QWidget*>())
		delete w;
}

void ADCBinaryPaneToolbar::slotGo2Location()
{
	QString s(mpGotoCbx->currentText().simplified());
	emit signalGoToLocation(s, 0);
	if (!s.isEmpty())
		mpGotoCbx->clearEditText();
}

void ADCBinaryPaneToolbar::enableControls(bool bEnable)
{
	mpGotoEntryTbtn->setEnabled(bEnable);
	mpGoBackTbtn->setEnabled(bEnable);
	//mpGotoLed->setEnabled(bEnable);
	mpGotoCbx->setEnabled(bEnable);
}*/










////////////////////////////////////////////////////////////////
// ADCBinaryPane

ADCBinaryPane::ADCBinaryPane(QWidget *parent)
	: ADCSplitPane(parent)//,
	//mpSwitchToSourceAction(nullptr),
	//mpSwitchToLowAction(nullptr)
{
	createExtraActions();
}

void ADCBinaryPane::createExtraActions()
{
	/*if (!mpSwitchToSourceAction)
		mpSwitchToSourceAction = newAction(tr("Switch To Source (Low Profile)"), SLOT(slotSwitchToSourceViewLow()), QKeySequence(tr("Shift+D")), QIcon(":source_low.png"));
	if (!mpSwitchToLowAction)
		mpSwitchToLowAction = newAction(tr("Switch To Source"), SLOT(slotSwitchToSourceView()), QKeySequence(tr("Shift+S")), QIcon(":source_high.png"));*/
}

void ADCBinaryPane::updateFont(const QFont& f)
{
	view()->setFont(f);
	static_cast<ADCBinaryPaneToolbar*>(mpToolbar)->setFont(f);
	//?mpToolbar->updateGeometry();
}

void ADCBinaryPane::clearExtraActions()
{
	/*delete mpSwitchToSourceAction;
	mpSwitchToSourceAction = nullptr;
	delete mpSwitchToLowAction;
	mpSwitchToLowAction = nullptr;*/
}

void ADCBinaryPane::repopulateTollbar(ADCSplitWin *)//pMainWin)
{
	/*toolbar0()->cleanup();
	createExtraActions();
	toolbar0()->populate(pMainWin, 0);*/
}

void ADCBinaryPane::createView(ADCModelData *pModelData)
{
	ADCBinView *pView(new ADCBinView(this));
	pView->setModelData(pModelData);
	mpView = pView;
}

void ADCBinaryPane::createToolbar(ADCModelData*)
{
	ADCBinaryPaneToolbar *pToolbar(new ADCBinaryPaneToolbar(this, view()));
	pToolbar->enableControls(view()->hasModel());
	mpToolbar = pToolbar;
}

/*ADCTextView *ADCBinaryPane::textView()
{
	return view(); 
}*/

/*void ADCBinaryPane::slotSwitchToSourceView()
{
	emit signalSwitchToSourceView(this, -1);
}

void ADCBinaryPane::slotSwitchToSourceViewLow()
{
	emit signalSwitchToSourceViewLow(this, -1);
}*/

void ADCBinaryPane::slotModelChanged()
{
	toolbar()->enableControls(view()->hasModel());
}


///////////////////////////////////////////////////PaneInfo

PaneInfo::PaneInfo()
	: pModelData(nullptr)
{
}

PaneInfo::~PaneInfo()
{
	if (pModelData)
		pModelData->Release();
}

void PaneInfo::set(ADCModelData *p)
{
	pModelData = p;
	pModelData->AddRef();
}

///////////////////////////////////////////////////
// ADCSplitWin

ADCSplitWin::ADCSplitWin(QWidget *parent, const char *name)
	: ADCSplitWinBase(parent)//, name)//, Qt::Horizontal)
	//mSyncLine(-1),
	//mbSyncMode(false)
{
	setObjectName(name);
	mDefaultPanes[0] = mDefaultPanes[1] = 0;
	mViews[0] = mViews[1] = nullptr;

	mpSplitter = this;
	mpSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

//	createActions();

	/*QAction *pTabAction = new QAction(tr("Switch Pane"), this);
	pTabAction->setShortcut(QKeySequence(Qt::Key_Tab));
	pTabAction->setShortcutContext(Qt::ApplicationShortcut);
	connect(pTabAction, SIGNAL(triggered()), SLOT(slotSwitchPane()));*/

//	setFocusPolicy(Qt::TabFocus);
}

ADCSplitWin::~ADCSplitWin()
{
}

void ADCSplitWin::slotSwitchPane()
{
}

void ADCSplitWin::aboutToClose()
{
	for (int i(0); i < mpSplitter->count(); i++)
	{
		ADCBinaryPane *pWin(dynamic_cast<ADCBinaryPane *>(mpSplitter->widget(i)));
		if (pWin)
			OnAboutToClose(pWin->view());
	}
}

/*void ADCSplitWin::createActions()
{
	///////////////////////////////////////////////////////toggle panes
	mpToggleLeftPaneAction = new QAction(QIcon(":left_pane.png"), tr("Toggle left Pane"), this);
	//mpToggleLeftPaneAction->setEnabled(true);
	//mpToggleLeftPaneAction->setCheckable(true);
	connect(mpToggleLeftPaneAction, SIGNAL(triggered()), SLOT(slotToggleLeftPane()));

	mpToggleRightPaneAction = new QAction(QIcon(":right_pane.png"), tr("Toggle right Pane"), this);
	//mpToggleRightPaneAction->setEnabled(true);
	//mpToggleRightPaneAction->setCheckable(true);
	connect(mpToggleRightPaneAction, SIGNAL(triggered()), SLOT(slotToggleRightPane()));

	mpSyncPanesAction = new QAction(QIcon(":sync_mode.png"), tr("Synchronize Panes"), this);//clock.png
	mpSyncPanesAction->setCheckable(true);
	mpSyncPanesAction->setAutoRepeat(false);
	connect(mpSyncPanesAction, SIGNAL(toggled(bool)), SIGNAL(signalToggleSyncMode(bool)));
	mpSyncPanesAction->setChecked(true);
	mpSyncPanesAction->setEnabled(false);
}*/

void ADCSplitWin::closeEvent(QCloseEvent *e)
{
	for (int i(0); i < 2; i++)
		if (mViews[i])
			mViews[i]->close();
	e->ignore();
}


bool ADCSplitWin::focusNextPrevChild(bool next)
{
	if (mpSplitter->count() == 2)
	{
		ADCSplitPane *pWin0(static_cast<ADCSplitPane *>(mpSplitter->widget(0)));
		ADCSplitPane *pWin1(static_cast<ADCSplitPane *>(mpSplitter->widget(1)));

		if (focusWidget() == pWin0->view())
		{
			pWin1->view()->setFocus();
			return true;
		}
		if (focusWidget() == pWin1->view())
		{
			pWin0->view()->setFocus();
			return true;
		}
	}
	
	return ADCSplitWinBase::focusNextPrevChild(next);
}

bool ADCSplitWin::event(QEvent* e)
{
	return ADCSplitWinBase::event(e);
}

void ADCSplitWin::keyPressEvent(QKeyEvent* e)
{
	ADCSplitWinBase::keyPressEvent(e);
}

void ADCSplitWin::focusInEvent(QFocusEvent* e)
{
	ADCSplitWinBase::focusInEvent(e);
}

void ADCSplitWin::openPane(int index, ADCSplitPane *pWin1, ADCSplitPane *pWin2, bool bSetFocus)
{
	mpSplitter->insertWidget(index, pWin1);

	if (mpSplitter->count() == 2)
	{
		int w(mpSplitter->width());
		QList<int> l(mLastSplitSize);
		if (l.empty())
			l << 1 / .3*w << 2 / .3*w;
		mpSplitter->setSizes(l);
		//setTabOrder(pWin1->view(), pWin2->view());
		//setTabOrder(pWin2->view(), pWin1->view());
	}

	QWidget *pView1(pWin1->view());

	pWin1->paneToggled(index, pWin2 != 0);
	if (pView1)
		pView1->setFont(font());
	pWin1->show();

	if (pWin2)
	{
		pWin2->paneToggled((~index) & 1, true);
/*		mpSyncPanesAction->setEnabled(true);
		if (mpSyncPanesAction->isChecked())
			emit signalToggleSyncMode(true);*/
		if (bSetFocus && pView1)
			pView1->setFocus();
	}
	else if (pView1)
		pView1->setFocus();//do not change focus if alt pane is opened
}

void ADCSplitWin::updateOutputFont(const QFont &f)
{
	setFont(f);
	for (int i(0); i < 2; i++)
		if (mViews[i])
			mViews[i]->updateFont(f);
	//emit signalSetFont(f);
}

bool ADCSplitWin::closeAltPane(int index)
{
	//bool bHadFocus(false);
	Q_ASSERT(index < 2);//mPanes.count());
	ADCSplitPane *pWin1(mViews[index]);
	if (!pWin1)
		return false;
	ADCSplitPane *pWin2(mViews[(~index) & 1]);//other view
//	ADCTextView *pView1(dynamic_cast<ADCTextView *>(pWin1->view()));
	Q_ASSERT(pWin1);
	{
		//bHadFocus = pView1->hasFocus();
//?		ADCModelData *pModelData(mPanes[index].takeModel(pView1->textModel()));
//?		setModelDataAt(index, pModelData);//push to the front - to keep track of a last mode
		if (mpSplitter->count() > 1)
			mLastSplitSize = mpSplitter->sizes();
		else
			mLastSplitSize.clear();
		mViews[index]->view()->close();
		mViews[index] = nullptr;
		pWin1->disconnect();
		pWin1->setParent(nullptr);
		pWin1->deleteLater();
		//mpToggleLeftPaneAction->setIcon(QIcon(":right_pane.png"));
//		mpSyncPanesAction->setEnabled(false);
		//?static_cast<ADCSourceView *>(mpSplitter->widget(0))->updateContents();
	}
	if (pWin2)
	{
		pWin2->paneToggled((~index) & 1, false);
//?		emit signalToggleSyncMode(false);
	}
	//return bHadFocus;
	return true;
}

void ADCSplitWin::toggleLeftPane()
{
	slotToggleLeftPane();
}

void ADCSplitWin::toggleRightPane()
{
	slotToggleRightPane();
}

void ADCSplitWin::slotToggleLeftPane()
{
	if (mpSplitter->count() == 1)//right pane is on
	{
		openAlternativePane(mDefaultPanes[0], false);
//?		mpToggleRightPaneAction->setEnabled(true);
	}
	else
	{
		Q_ASSERT(mpSplitter->count() == 2);
		if (closeAlternativePane(false))
		{}//mpToggleRightPaneAction->setEnabled(false);//can't close right pane
	}
	//mpSyncPanesAction->setEnabled(bOn);
}

void ADCSplitWin::slotToggleRightPane()
{
	if (mpSplitter->count() == 1)//left pane is on
	{
		openAlternativePane(mDefaultPanes[1], true);
		//mpToggleLeftPaneAction->setEnabled(true);
	}
	else//TOGGLE OFF
	{
		Q_ASSERT(mpSplitter->count() == 2);
		if (closeAlternativePane(true))
		{}//?mpToggleLeftPaneAction->setEnabled(false);//can't close left pane
	}
}

/*void ADCSplitWin::synchronizeAtLine(int line)
{
	emit signalSync PanesResponce(line, true);
}*/

/*void ADCSplitWin::slotRefLineChanged(int line)
{
	//'setcurop' should have been sent already!
	//if (line != mSyncLine)
	{
		mSyncLine = line;
		//fprintf(stdout, "SyncLine set: %d\n", mSyncLine); fflush(stdout);

//!		if (mpSyncPanesAction->isChecked())
//!			if (mpSplitter->count() > 1)
			{
				emit signalSyncPanesRequest2(mSyncLine, false);
			}
	}
}*/

/*void ADCSplitWin::slotSyncModeChanged(bool bOn)
{
	mbSyncMode = bOn;
}*/

void ADCSplitWin::slotSyncPanesRequest2(int iPos, bool force)
{
	//mSyncLine = iPos;
 	emit signalSyncPanesRequest2(iPos, force);
#if(0)
	if (mpSplitter->count() > 1)
	{
		//slotRefLineChanged(iPos);
		bool bSync(force);
		if (!bSync)
			bSync = mbSyncMode;
			//emit signalSyncModeInquiry(bSync);
		if (bSync)
			emit signalSyncPanesResponce(qMax(line, 0), true);//bNoFocus
	}
#endif
}

void ADCSplitWin::slotSyncPanesResponce2(int iPos, bool bNoFocus)
{
	//check if the tab is visible (isVisible doesn't work)
	for (QWidget* p(parentWidget()); p; p = p->parentWidget())
	{
		QTabWidget* pp(dynamic_cast<QTabWidget*>(p->parentWidget()));
		if (pp &&  pp->currentWidget() != parentWidget())
			return;
	}

	if (/*mpSyncPanesAction->isChecked() &&*/ isActiveWindow())
	{
		//iPos = (mSyncLine > 0) ? mSyncLine : 0;
		emit signalSyncPanesResponce2(iPos, bNoFocus);
	}
}






//////////////////////////////////////////

/*ADCDualWin::ADCDualWin(QWidget *parent, const char *name)
	: ADCSplitWin(parent, name)
{
}

ADCDualWin::~ADCDualWin()
{
}

*/






