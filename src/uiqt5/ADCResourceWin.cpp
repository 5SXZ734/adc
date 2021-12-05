#include <QtCore/QTime>
#include <QtGui/QPainter>
#include <QLayout>
#include <QMenu>
#include <QHeaderView>
#include <QToolButton>

#include "ADCResourceWin.h"
#include "ADCUtils.h"



ADCResView::ADCResView(QWidget *parent)
	: QTreeView(parent)
{
	setFrameStyle(QFrame::Panel | QFrame::Sunken);
	setLineWidth(0);
	setMidLineWidth(0);
}

ADCResView::~ADCResView()
{
	QAbstractItemModel *pModel(model());
	QTreeView::setModel(nullptr);
	delete pModel;
}

void ADCResView::setModel(adcui::IResViewModel *pIModel)
{
	ADCResViewModel *model(new ADCResViewModel(this, pIModel));
	QTreeView::setModel(model);
	update();
}



//////////////////////////////////////////////////////////////////////////

ADCResWin::ADCResWin(QWidget * parent, const char *name)
	: ADCTabsWin(parent, name),//new ADCWinToolbar(this)),
	mpListView(nullptr)
{
#if(1)
	setToolbar(new ADCWinToolbar(this));
	//mpToolbar = new ADCWinToolbar(this);

	QAction* pExpandAction = new QAction(QIcon(":expand.png"), tr("Expand All"), this);
	connect(pExpandAction, SIGNAL(triggered()), SLOT(slotExpandChildren()));

	QAction* pCollapseAction = new QAction(QIcon(":collapse.png"), tr("Collapse All"), this);
	connect(pCollapseAction, SIGNAL(triggered()), SLOT(slotCollapseChildren()));

	QAction* pViewResourceAction = new QAction(QIcon(":canvas.png"), tr("View Resource"), this);
	connect(pViewResourceAction, SIGNAL(triggered()), SIGNAL(signalShowCanvas()));

	mpToolbar->addButton(pCollapseAction);
	mpToolbar->addButton(pExpandAction);
	mpToolbar->addSeparator();
	mpToolbar->addButton(pViewResourceAction);
	mpToolbar->addStretch();
#endif

/*	QVBoxLayout * vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addWidget(mpToolbar);
	vbox->addWidget(mpListView);*/

	//populateToolbar();
}

QWidget* ADCResWin::createView(QString path)
{
	ADCResView *pView(new ADCResView(this));
	pView->setFont(font());
	pView->setRootIsDecorated(true);
	pView->setAllColumnsShowFocus(true);
	pView->setHeaderHidden(true);

	connect(pView, SIGNAL(activated(const QModelIndex &)), SLOT(slotItemDoubleClicked(const QModelIndex &)));
	connect(pView, SIGNAL(clicked(const QModelIndex &)), SLOT(slotCurrentItemChanged(const QModelIndex &)));

	emit signalRequestModel(*pView, path);

	return pView;
}

void ADCResWin::populate()
{
	ADCStream ss;
	emit signalDumpResources(ss);
	//mpListView->populate(ss);
}

/*void ADCResWin::updateOutputFont(const QFont &f)
{
	mpListView->setFont(f);
}*/

void ADCResWin::slotReset()
{
	ADCResViewModel *model(static_cast<ADCResViewModel *>(mpListView->model()));
	if (!model)
		return;

	//populate();
	model->invalidate();
	//update();
}

void ADCResWin::slotCollapseChildren()
{
	if (currentView())
		currentView()->collapseAll();
/*	QTreeWidgetItemIterator it(this);
	it++;//except a root one
	for (; *it; ++it)
	{
		if ((*it)->childCount() > 0)
			(*it)->setExpanded(false);
	}*/
}

void ADCResWin::slotExpandChildren()
{
	if (currentView())
		currentView()->expandAll();
/*	int iCount(0);
	QTreeWidgetItemIterator it(this);
	for (; (*it); ++it)
	{
		if ((*it)->childCount() > 0)
//		{
			(*it)->setExpanded(true);
			iCount++;
		}
//	}
	//blockSignals(false);*/
}

void ADCResWin::slotItemDoubleClicked(const QModelIndex &index)
{
	if (index.isValid())
	{
		const ADCResViewModel *pModel(static_cast<const ADCResViewModel *>(index.model()));

		QString s(pModel->itemPath(index));
		const QByteArray cs(MODULE_SEP);
		QString sModule(s.section(cs, 0, 0));
		QString sPos(pModel->viewPos(index));

		//if (!model->isFolderItem(index))
			emit signalItemDoubleClicked(sModule + cs + sPos);//pModel->itemPath(index));
	}
	//emit signalItemDoubleClicked(path(current));
	//slotSyncPanesRequest(0);
}


/*QString ADCResWin::path(QTreeWidgetItem *lvi) const
{
	QString s;
	if (!lvi->text(1).isEmpty())
		s = lvi->text(1);
	while (lvi)
	{
		if (!lvi->parent())
			s.prepend(lvi->text(0) + MODULE_SEP);
		lvi = lvi->parent();
	}
	CHECK_QSTRING(s, z);
	return s;
}*/

void ADCResWin::slotCurrentItemChanged(const QModelIndex &index)
{
	if (index.isValid())
	{
		const ADCResViewModel *pModel(static_cast<const ADCResViewModel *>(index.model()));

		QString s(pModel->itemPath(index));
		const QByteArray cs(MODULE_SEP);
		QString sModule(s.section(cs, 0, 0));
		QString sPos(pModel->viewPos(index));

		//QString sPos(s.section('/', -1));
		emit signalItemClicked(sModule + cs + sPos);
	}
	//emit signalItemClicked(path(current));
}

void ADCResWin::setModel(adcui::IResViewModel *)//pIModel)
{
//	ADCResViewModel *model(new ADCResViewModel(this, pIModel));
	//mpListView->setModel(model);
}

void ADCResWin::slotAboutToClose()
{
	depopulate();
	//QAbstractItemModel *pModel(mpListView->model());
	//mpListView->setModel(nullptr);
	//delete pModel;
}

void ADCResWin::slotProjectNew()
{
	//Q_ASSERT(!mpListView->model());
	emit signalRequestModel(*this);
}

ADCCanvasWin::ADCCanvasWin(QWidget * parent, const char *name)
: QWidget(parent)
{
	setObjectName(name);
	//mPixmap.load(":canvas.png");
}

void ADCCanvasWin::slotUpdate()
{
	if (!isVisible())
		return;
	mPixmap = QPixmap();
	emit signalUpdatePixmap(mPixmap);
	update();
}

void ADCCanvasWin::paintEvent(QPaintEvent *)
{
	QRect rcClient(contentsRect());
	QPainter P(this);
	P.fillRect(rcClient, QBrush(Qt::white));//lightGray));
	int x(rcClient.width() / 2);
	int y(rcClient.height() / 2);
	x -= mPixmap.width() / 2;
	y -= mPixmap.height() / 2;
	P.drawPixmap(x, y, mPixmap);
}