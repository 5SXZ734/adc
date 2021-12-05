#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QLayout>
#include <QIcon>
#include <QAction>
#include <QTabWidget>
#include "ADCStubsWin.h"
#include "ADCUtils.h"//toolbar
#include "colors.h"

using namespace adcui;

ADCStubsView::ADCStubsView(QWidget *parent)
: ADCStubsViewBase(parent, "ADCStubsView"),
mpModelData(nullptr)
{
	setFrameShape(NoFrame);
	//setFrameShadow(Plain);
	//stubsModel()->seekLineIt(topIt().line(), 0);
	//setBackgroundRole(QPalette::Base);
	//setAutoFillBackground(true);
	//mpViewport->setBackgroundRole(QPalette::Base);
	//mpViewport->setAttribute(Qt::WA_OpaquePaintEvent, true);
	//mpViewport->setAutoFillBackground(true);
	syncViewportMargins();
	//showScrollBars(false);
}

ADCStubsView::~ADCStubsView()
{
	delete mpModelData;
	mpModelData = nullptr;
}

void ADCStubsView::closeEvent(QCloseEvent *e)
{
	ADCStubsViewBase::closeEvent(e);
}

#if(0)
void ADCStubsView::setModel(ADCModelData *pModelData)//adcui::IStubsViewModel *pIModel)
{
	killEditor();
	mpModelData = pModelData;
	/*if (mpIModel)
	{
		delete mpTopIt;
		delete mpCurIt;
		//mpIModel->deleteIterator(mInfoIt);
		mpIModel->Release();
	}
	mpIModel = pIModel;
	if (mpIModel)
	{
		mpIModel->AddRef();
		mpTopIt = new ADCDocLine(mpIModel);
		mpCurIt = new ADCDocPos(mpIModel);
	}
	else
	{
		mpTopIt = mpCurIt = nullptr;
	}*/
	syncViewportMargins();
	reset();
	showScrollBars(mpModelData != nullptr);//mpIModel != 0);
}
#endif

void ADCStubsView::mouseDoubleClickEvent(QMouseEvent *e)
{
	//int nLine(e->y() / lineHeight());
	int nChar(e->x() / charWidth());
	if (nChar <= leftMargin())
	{
		const char *pCellData(tableModel()->cellDataIt(0, curIt().line()));
		if (pCellData)
		{
			QString s(pCellData);
			if (s.startsWith("[") && s.endsWith("]"))
				s = s.mid(1, s.length() - 2);
			emit signalGoToLocation(s);
		}
		return;
	}

	ADCStubsViewBase::mouseDoubleClickEvent(e);
}

void ADCStubsView::showEvent(QShowEvent *e)
{
	syncViewportMargins();
	ADCStubsViewBase::showEvent(e);
}

bool ADCStubsView::checkAutoexpand()
{
	if (!ADCStubsViewBase::checkAutoexpand())
		return false;
	placeEditor();
	return true;
}

void ADCStubsView::paintEvent(QPaintEvent *event)
{
	ModelLocker lock(this);
	ADCStubsViewBase::paintEvent(event);
}

int ADCStubsView::DrawMarginCell(QPainter &p, const QRect &rc,const ADCCell &aCell)
{
/*	if (*(unsigned char *)pcData == adcui::SYM_COLOR)
	{
		int iCol(*(++pcData));
		if (iCol == adcui::COLOR_USER_FUNCTION)
			p.setPen(Qt::darkBlue);
		++pcData;
	}*/
	return ADCStubsViewBase::DrawMarginCell(p, rc, aCell);
}

void ADCStubsView::OnNewRow(ADCTextRow &aRow)
{
	if (aRow.lineColor() == COLOR_NULL)
		if (aRow.isCurrent())//equal?
			aRow.setLineColor(COLOR_CUR_STUB);
}

void ADCStubsView::reset()
{
#if(1)
	//?if (mpIModel)
	{
		stubsModel()->invalidate(false);
		OnContentsChanged();
		//stubsModel()->seekLineIt(topIt().line(), 0);
		updateContents();
	}
#endif
}

void ADCStubsView::reload()
{
	stubsModel()->reload();
	//OnContentsChanged();
	//updateContents();
}

void ADCStubsView::OnContentsChanged()
{
	//stubsModel()->invalidate(false);
	ModelLocker lock(this);
	stubsModel()->Redump(curIt().line());
	ADCStubsViewBase::OnContentsChanged();
}

void ADCStubsView::editStub()
{
	//setVisibleIt(mInfoIt, 2);
/*?	QRect r;
	ensureCellVisible(mInfoIt, 0, r);
	{
		mpIModel->copyIt(curIt().line(), mInfoIt);
		curIt().set(*mpTopIt, r.left(), r.top());
		setFocus();
		slotEditCell();
	}*/
}

void ADCStubsView::highlightStub(QString sAt)
{
	ModelLocker lock(this);
	if (!stubsModel()->seekPosIt("$task", topIt().line()))//position at the top
		return;

	QRect r;
	ensureCellVisible(topIt().line(), 0, r);
	{
		curIt().copyFrom(&topIt());
		curIt().set(topIt(), r.left(), r.top());//put cursor into cell
		//setFocus();
		slotEditCell();
	}
}

void ADCStubsView::onAnalysisStarted()
{
	updateContents();
}

void ADCStubsView::onSelFuncChanged()
{
	ModelLocker lock(this);
	if (stubsModel()->seekPosIt("@", topIt().line()))//position at the top
	{
		setVScrollBar();
		//updateContents();
	}
}


void ADCStubsView::setModel(ADCModelDataMap &rOwner, adcui::IStubsViewModel *pIModel)
{
	Q_ASSERT(!mpModelData);
	mpModelData = new ADCModelData(rOwner, pIModel);
	createContents();
}

//////////////////////////////

ADCStubsWin::ADCStubsWin(QWidget *parent, const char *name)
: ADCTabsWin(parent, name),
mpView(nullptr)
{
	setObjectName(name);
	//setBackgroundRole(QPalette::Window);
	//setAutoFillBackground(false);
	//createView();
	setFrameShape(NoFrame);

	setToolbar(new ADCWinToolbar(this));

	QAction* pReloadAction = new QAction(QIcon(":reload_stubs.png"), tr("Reload from PROTO file"), this);
	connect(pReloadAction, SIGNAL(triggered()), SLOT(slotReload()));

	QAction* pSaveAction = new QAction(QIcon(":diskette.png"), tr("Save PROTO file and generate IMPORTS header"), this);
	connect(pSaveAction, SIGNAL(triggered()), SLOT(slotSaveAll()));

	mpToolbar->addButton(pReloadAction);
	mpToolbar->addStretch();
	mpToolbar->addButton(pSaveAction);
}

void ADCStubsWin::createView()
{
/*	mpView = new ADCStubsView(this, mpModelData);
	mpView->setFont(font());
	mpView->show();
	connect(mpView, SIGNAL(signalGoToLocation(QString)), SIGNAL(signalGoToLocation(QString)));
	mpView->setDrawGrid(true);
	mpView->setShowCaret(false);
	QVBoxLayout * vbox(new QVBoxLayout);
	vbox->addWidget(mpView);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->setSpacing(0);
	setLayout(vbox);*/
}

void ADCStubsWin::destroyView()
{
	if (mpView)
	{
		delete mpView;
		delete layout();
		mpView = nullptr;
	}
}

ADCStubsWin::~ADCStubsWin()
{
}

void ADCStubsWin::setModel(ADCModelDataMap &mr, adcui::IStubsViewModel *pIModel)
{
	createView();
	mpView->setModel(mr, pIModel);
}

ADCStubsView* ADCStubsWin::currentView()
{
	return dynamic_cast<ADCStubsView*>(mpTabWidget->currentWidget());
}

void ADCStubsWin::slotReload()
{
	ADCStubsView* pView(currentView());
	pView->reload();
}

void ADCStubsWin::slotSaveAll()
{

}

void ADCStubsWin::slotReset()
{
	if (mpView)
		mpView->reset();
}

void ADCStubsWin::slotUpdate()
{
	currentView()->OnContentsChanged();
}

void ADCStubsWin::slotEditStub()
{
	if (mpView)
		mpView->editStub();
}

/*void ADCStubsWin::slotHighlightLocationAt(QString sAtAddr, QString sRefAddr, QString, int)
{
	mpView->highlightStub(sAtAddr);
}*/

void ADCStubsWin::slotGoToTaskTop()
{
	if (mpView)
		mpView->highlightStub(QString());
}

void ADCStubsWin::slotAnalysisStarted()
{
	if (mpView)
		mpView->onAnalysisStarted();
}

void ADCStubsWin::updateOutputFont(const QFont &f)
{
	setFont(f);
	if (mpView)
		mpView->setFont(f);
}

void ADCStubsWin::slotSelFuncChanged()
{
	if (mpView)
		mpView->onSelFuncChanged();
}

void ADCStubsWin::slotProjectNew()
{
	destroyView();
}

void ADCStubsWin::slotAboutToClose()
{
	destroyView();
	//mpView->setModel(nullptr);
}

void ADCStubsWin::slotDcNew()
{
/*	if (mpModelData)
		return;
	emit signalRequestModel(*this);
	if (mpModelData)
	{
		createView();
		slotUpdate();
	}*/
}

void ADCStubsWin::showEvent(QShowEvent *)
{
	slotReset();
}

QWidget* ADCStubsWin::createView(QString path)
{
	ADCStubsView *pView(new ADCStubsView(this));
	pView->setFont(font());
//	pView->show();
	connect(pView, SIGNAL(signalGoToLocation(QString)), SIGNAL(signalGoToLocation(QString)));
	pView->setDrawGrid(true);
	pView->setShowCaret(false);

	//connect(pView, SIGNAL(signalRefresh()), SLOT(slotUpdate()));

	emit signalRequestModel(*pView, path);
	//pView->populate();
	
	return pView;
}
