#include <QAction>
#include <QMenu>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include "ADCTemplWin.h"

template <typename T>
struct ModelPtr
{
	QSortFilterProxyModel*	proxy;
	T*	model;
	ModelPtr(QAbstractItemModel *p)
	{
		proxy = dynamic_cast<QSortFilterProxyModel *>(p);
		model = proxy ? dynamic_cast<T *>(proxy->sourceModel()) : nullptr;
		if (!model)
			model = static_cast<T *>(p);
	}
};


////////////////////////////////////////////////////////////
// ADCTemplView

ADCTemplView::ADCTemplView(QWidget *parent, QString path)
	: QTreeView(parent),
	mbMode(true),
	msPath(path)
{
	mpRefreshAction = new QAction(QIcon(":refresh.png"), tr("Refresh"), this);
	connect(mpRefreshAction, SIGNAL(triggered()), SIGNAL(signalRefresh()));

	setFrameStyle(QFrame::Panel | QFrame::Sunken);
	setLineWidth(0);
	setMidLineWidth(0);
	setUniformRowHeights(true);
	setWordWrap(false);
	setSortingEnabled(true);

	viewport()->installEventFilter(new QToolTipper(this));

	connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(slotCustomContextMenuRequested(const QPoint &)));
}

ADCTemplView::~ADCTemplView()
{
	QAbstractItemModel *pModel(model());
	QTreeView::setModel(nullptr);
	delete pModel;
}

void ADCTemplView::slotCustomContextMenuRequested(const QPoint &pos)
{
	QMenu *menu(new QMenu(this));
	menu->addAction(QIcon(":.png"), tr("Toggle Types/Fields"), this, SLOT(slotToggleTypesFields()));
	menu->popup(viewport()->mapToGlobal(pos));
}

void ADCTemplView::slotToggleTypesFields()
{
	mbMode = !mbMode;
	emit signalRequestModel(*this, msPath, mbMode);
}

/*class ADCTypesViewProxyModel : public QSortFilterProxyModel
{
public:
	ADCTypesViewProxyModel(QWidget *parent)
		: QSortFilterProxyModel(parent)
	{
	}
	virtual ~ADCTypesViewProxyModel()
	{
		QAbstractItemModel *source(sourceModel());
		setSourceModel(nullptr);
		delete source;
	}
protected:
	virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
	{
		//ADCNamesViewModel *pSourceModel(static_cast<ADCNamesViewModel *>(sourceModel()));
		//QModelIndex index0 = pSourceModel->index(sourceRow, 0, sourceParent);
		return true;//pSourceModel->isRowVisible(sourceRow);
	}
};*/

void ADCTemplView::setModel(adcui::ITemplViewModel *pIModel)
{
	//destroy old model
	QAbstractItemModel *pOldModel(model());
	//QTreeView::setModel(nullptr);
	if (pIModel)
	{
#if(1)
		QSortFilterProxyModel *proxyModel(new QSortFilterProxyModel(this));
		//proxyModel->setSortingEnabled(true);
		proxyModel->setSourceModel(new ADCTemplViewModel(this, pIModel));
		proxyModel->setFilterKeyColumn(-1);
		proxyModel->setDynamicSortFilter(true);
		QTreeView::setModel(proxyModel);
#else
		//header()->setStretchLastSection(true);
		QTreeView::setModel(new ADCTemplViewModel(this, pIModel));
#endif
		//header()->setSectionResizeMode(QHeaderView::Interactive);
		int n(header()->count());
		header()->resizeSection(n - 3, 400);//substitute
		header()->resizeSection(n - 2, 600);//scope
		//header()->resizeSection(n - 1, 800);//scope
		header()->setSectionResizeMode(n - 1, QHeaderView::Stretch);
		//header()->setSectionResizeMode(n - 1, QHeaderView::ResizeToContents);//SLOW!
		//header()->setStretchLastSection(true);
	}
	delete pOldModel;
}

void ADCTemplView::keyPressEvent(QKeyEvent *e)
{
	//if (e->modifiers() & Qt::ShiftModifier)
	//return;
	QTreeView::keyPressEvent(e);
}

/*void ADCTemplView::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
	emit signalCurrentChanged(current, previous);
}*/

void ADCTemplView::populate()
{
	ModelPtr<ADCTemplViewModel> mod(model());
	if (mod.model)
	{
		mod.model->reset();
		update();
	}
}

void ADCTemplView::populate(ADCStream &/*ss*/)
{
	/*	blockSignals(true);
		clear();

		QTreeWidgetItem *lviAfter(nullptr);

		QString s;
		while (ss.ReadString(s))
		{
		QTreeWidgetItem *lvi(new QTreeWidgetItem(this, lviAfter));
		lvi->setText(0, s);
		lviAfter = lvi;
		}

		sortItems(0, Qt::AscendingOrder);
		blockSignals(false);*/
	update();
}




///////////////////////////////////////////////////////////////////// ADCTemplWin

ADCTemplWin::ADCTemplWin(QWidget *parent, const char *name)
	: ADCTabsWin(parent, name, Qt::Vertical),//new ADCWinToolbar(this)),//QSplitter(Qt::Vertical, parent)//, name)
	mbDirty(true)
{
	//ADCNamesToolbar *pToolbar(new ADCNamesToolbar(this));
	//connect(pToolbar, SIGNAL(signalFilterChanged(int)), SLOT(slotFilterType(int)));
	//setToolbar(pToolbar);
	//populateToolbar();
}

ADCTemplWin::~ADCTemplWin()
{
}

QWidget* ADCTemplWin::createView(QString path)
{
	ADCTemplView *pView(new ADCTemplView(nullptr, path));
	pView->setContextMenuPolicy(Qt::CustomContextMenu);
	pView->setFont(font());
	connect(pView, SIGNAL(signalRefresh()), SLOT(slotUpdate()));
	//connect(pView, SIGNAL(clicked(const QModelIndex &)), SLOT(slotTypeActivated(const QModelIndex &)));
	//connect(pView, SIGNAL(activated(const QModelIndex &)), SLOT(slotDoubleClicked(const QModelIndex &)));
	connect(pView, SIGNAL(signalRequestModel(ADCTemplView &, QString, bool)), SIGNAL(signalRequestModel(ADCTemplView &, QString, bool)));

	pView->slotToggleTypesFields();
	pView->populate();
	
//	pView->setFilter(filterValue());
	return pView;
}


/*void ADCTemplWin::slotTypeActivated(const QModelIndex &)
{
	//QString s(index.data(Qt::DisplayRole).toString());
	//signalRequestBinModel(*this, s);
}

void ADCTemplWin::slotDoubleClicked(const QModelIndex &index)
{
	QString sType(index.data(Qt::DisplayRole).toString());

	ModelPtr mod(currentView());//index.model());
	QString sModule(mod.model ? mod.model->moduleName() : "");

	QString s(sModule + MODULE_SEP + NAMES_EXT + "/" + sType);

	CHECK_QSTRING(s, z);

	emit signalItemDoubleClicked(s);

	//if (!mpBinView)
	//mpBinView = new ADCBinView(this);
	//signalRequestBinModel(*this, s);
}*/

ADCTemplView* ADCTemplWin::currentView()
{
	return dynamic_cast<ADCTemplView *>(mpTabWidget->currentWidget());
}

void ADCTemplWin::setModel(adcui::ITemplViewModel *pIModel)
{
	Q_ASSERT(0);
	(void)pIModel;
	//mpView->setModel(pIModel);
}

/*void ADCTemplWin::populateToolbar()
{
	if (mpView)
		mpToolbar->addButton(mpView->mpRefreshAction);
}*/

void ADCTemplWin::populate()
{
}

void ADCTemplWin::showEvent(QShowEvent *e)
{
	if (mbDirty)
		slotUpdate();
	QWidget::showEvent(e);
}

void ADCTemplWin::slotUpdate()
{
	if (!isVisible())
	{
		mbDirty = true;
		return;
	}
	ADCTemplView *pView(currentView());
	if (pView)
		pView->populate();
	/*ADCStream ss;
	emit signalDumpTypes(ss);
	mpView->populate(ss);*/
	mbDirty = false;
}

void ADCTemplWin::slotContextIdChanged(unsigned )
{
	//mpView->setContextId(u);
}

void ADCTemplWin::slotAboutToClose()
{
	depopulate();
	//	QAbstractItemModel *pModel(mpView->model());
	//	mpView->setModel(nullptr);
	//	delete pModel;
}


void ADCTemplWin::slotProjectNew()
{
	//?emit signalRequestModel(*this);
}

