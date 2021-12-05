#include <QtCore/QVariant>
#include <QtCore/QSignalMapper>
#include <QSortFilterProxyModel>
#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtGui/QPixmap>

#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QListView>
#include <QLayout>
#include <QHBoxLayout>
#include <QToolTip>
#include <QWhatsThis>
#include <QHeaderView>
#include <QAction>
#include <QToolButton>
#include <QLineEdit>
#include <QMenu>

#include "ADCNamesWin.h"

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
// ADCNamesView

ADCNamesView::ADCNamesView(QWidget *parent)
	: QTreeView(parent)
{
	mpRefreshAction = new QAction(QIcon(":refresh.png"), tr("Refresh"), this);
	connect(mpRefreshAction, SIGNAL(triggered()), SIGNAL(signalRefresh()));

	setFrameStyle(QFrame::Panel | QFrame::Sunken);
	setLineWidth(0);
	setMidLineWidth(0);
	setUniformRowHeights(true);
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(slotCustomContextMenuRequested(const QPoint &)));

	viewport()->installEventFilter(new QToolTipper(this));
}

ADCNamesView::~ADCNamesView()
{
	QAbstractItemModel *pModel(model());
	QTreeView::setModel(nullptr);
	delete pModel;
}

void ADCNamesView::slotCustomContextMenuRequested(const QPoint &pos)
{
	QModelIndex index(indexAt(pos));
	if (index.isValid())
	{
		QMenu menu;
		menu.addAction(QIcon(":trace.png"), tr("Go To Definition"), this, SLOT(slotGoToDeclaration()));
		menu.addSeparator();
		menu.addAction(QIcon(":refresh.png"), tr("Refresh"), this, SLOT(slotRefresh()));
		menu.exec(viewport()->mapToGlobal(pos));
	}
}

void ADCNamesView::slotGoToDeclaration()
{
	//const ADCNamesViewModel *model(static_cast<const ADCNamesViewModel *>(index.model()));
	ModelPtr<ADCNamesViewModel> mod(model());
	if (mod.model)
		mod.model->goToDefinition(currentIndex());
}

void ADCNamesView::slotRefresh()
{
	if (model())
		namesModel()->invalidate();
}

class ADCNamesViewProxyModel : public QSortFilterProxyModel
{
public:
	ADCNamesViewProxyModel(QWidget *parent)
		: QSortFilterProxyModel(parent)
	{
	}
	virtual ~ADCNamesViewProxyModel()
	{
		QAbstractItemModel *source(sourceModel());
		setSourceModel(nullptr);
		delete source;
	}
protected:
	virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
	{
		ADCNamesViewModel *pSourceModel(static_cast<ADCNamesViewModel *>(sourceModel()));
		QModelIndex i(pSourceModel->index(sourceRow, 0, sourceParent));
		return pSourceModel->isRowVisible(i);
	}
};

void ADCNamesView::setModel(adcui::INamesViewModel *pIModel)
{
	//destroy old model
	QAbstractItemModel *pOldModel(model());
	QTreeView::setModel(nullptr);
	delete pOldModel;

	if (pIModel)
	{
#if(0)
		QSortFilterProxyModel *proxyModel(new ADCNamesViewProxyModel(this));
		//proxyModel->setSortingEnabled(true);
		proxyModel->setSourceModel(new ADCNamesViewModel(this, pIModel));
		proxyModel->setFilterKeyColumn(0);
		proxyModel->setDynamicSortFilter(true);
		QListView::setModel(proxyModel);
		//mpView->setSortingEnabled(true);
		//mpView->sortByColumn(0, Qt::AscendingOrder);
#else
		QTreeView::setModel(new ADCNamesViewModel(this, pIModel));
#ifdef _DEBUG
		header()->moveSection(0, 1);
#endif
#endif
	}
}

void ADCNamesView::keyPressEvent(QKeyEvent *e)
{
	//if (e->modifiers() & Qt::ShiftModifier)
	//return;
	QTreeView::keyPressEvent(e);
}

void ADCNamesView::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
	QTreeView::currentChanged(current, previous);
	emit signalCurrentChanged(current, previous);
}

void ADCNamesView::populate()
{
	ModelPtr<ADCNamesViewModel> mod(model());
	if (mod.model)
	{
		mod.model->reset();
//		mod.proxy->sort(0);
		update();
	}
}

void ADCNamesView::setContextId(unsigned )
{
	/*ModelPtr mod(this);
	if (mod.model && mod.model->setContextId(u))
		viewport()->update();*/
}

void ADCNamesView::setFilter(unsigned u)
{
	ModelPtr<ADCNamesViewModel> mod(model());
	if (mod.model)
	{
		if (mod.model->setFilter(u))//has filter changed?
		{
			if (!mod.proxy)
			{
				QSortFilterProxyModel* proxyModel(new ADCNamesViewProxyModel(this));
				//proxyModel->setSortingEnabled(true);
				proxyModel->setSourceModel(mod.model);
				proxyModel->setFilterKeyColumn(0);
				proxyModel->setDynamicSortFilter(true);
				QTreeView::setModel(proxyModel);
				mod.proxy = proxyModel;
				mod.proxy->invalidate();
			}
			else
			{
				if (!mod.model->isFltered())
				{
					QTreeView::setModel(mod.model);
					mod.proxy->setSourceModel(nullptr);
					delete mod.proxy;
				}
				else
					mod.proxy->invalidate();
			}
		}
	}
}


void ADCNamesView::populate(ADCStream &/*ss*/)
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


///////////////////////////////////////////////////////////////////// ADCTypesWinTBar

/*class ADCTypesWinTBar : public ADCWinToolbar
{
public:
ADCTypesWinTBar(QWidget *parent)
: ADCWinToolbar(parent)
{
}
};*/

///////////////////////////////////////////////////////////////////// ADCNamesToolbar

ADCNamesToolbar::ADCNamesToolbar(QWidget *parent)
	: ADCWinToolbar(parent)
{
	QSignalMapper *signalMapper(new QSignalMapper(parent));
	connect(signalMapper, SIGNAL(mapped(int)), SIGNAL(signalFilterChanged(int)));

#define _ICON(e)	QIcon(ADCNamesViewModel::nameKindToIconStr(adcui::INamesViewModel::e))
	add(E_TYPES, _ICON(E_TYPE), tr("Types"), signalMapper);
	add(E_FIELDS, _ICON(E_FIELD), tr("Data Fields"), signalMapper);
	add(E_FUNCTIONS, _ICON(E_FUNCTION), tr("Functions"), signalMapper);
#undef _ICON
	addSeparator();
	add(E_IMPORTS, QIcon(":import_24.png"), tr("Imports"), signalMapper);
	add(E_EXPORTS, QIcon(":export_24.png"), tr("Exports"), signalMapper);
	addStretch();
}

void ADCNamesToolbar::add(int index, const QIcon& icon, QString toolTip, QSignalMapper* signalMapper)
{
	PQAction& a(m_filters[index]);
	a = new QAction(icon, toolTip, this);
	a->setCheckable(true);
	addButton(a);
	connect(a, SIGNAL(triggered()), signalMapper, SLOT(map()));
	signalMapper->setMapping(a, index);
}

uint ADCNamesToolbar::filterValue() const
{
	uint u(0);
	for (int i(0); i < E__FILTER_MAX; i++)
		if (m_filters[i]->isChecked())
			u |= (1 << i);
	return u;
}



///////////////////////////////////////////////////////////////////// ADCNamesWin

ADCNamesWin::ADCNamesWin(QWidget *parent, const char *name)
	: ADCTabsWin(parent, name),//new ADCWinToolbar(this)),//QSplitter(Qt::Vertical, parent)//, name)
	mbDirty(true),
	mpView(nullptr)
{
	ADCNamesToolbar *pToolbar(new ADCNamesToolbar(this));
	connect(pToolbar, SIGNAL(signalFilterChanged(int)), SLOT(slotFilterType(int)));
	setToolbar(pToolbar);

	populateToolbar();
}

ADCNamesWin::~ADCNamesWin()
{
}

QWidget* ADCNamesWin::createView(QString path)
{
	ADCNamesView *pView(new ADCNamesView(nullptr));
	pView->setFont(font());
	pView->header()->setFont(font());
	connect(pView, SIGNAL(signalRefresh()), SLOT(slotUpdate()));
	//connect(pView, SIGNAL(clicked(const QModelIndex &)), SLOT(slotTypeActivated(const QModelIndex &)));
	connect(pView, SIGNAL(activated(const QModelIndex &)), SLOT(slotDoubleClicked(const QModelIndex &)));

	emit signalRequestModel(*pView, path);
	pView->populate();
	pView->setFilter(filterValue());
	return pView;
}

void ADCNamesWin::onCurrentChanged()
{
	slotFilterType(0);
}

void ADCNamesWin::updateOutputFont(const QFont& f)
{
	ADCTabsWin::updateOutputFont(f);
}

void ADCNamesWin::slotTypeActivated(const QModelIndex &)
{
	//QString s(index.data(Qt::DisplayRole).toString());
	//signalRequestBinModel(*this, s);
}

void ADCNamesWin::slotDoubleClicked(const QModelIndex &index)
{
	QString sType(index.data(Qt::DisplayRole).toString());

	ModelPtr<ADCNamesViewModel> mod(currentView()->model());//index.model());
	QString sModule(mod.model ? mod.model->moduleName() : "");
	if (sModule.isEmpty())
		return;

	QString s(sModule + MODULE_SEP + NAMES_EXT + "/" + sType);

	emit signalItemDoubleClicked(s);
}

/*void ADCNamesWin::init()
{
emit signalRequestModel(*this);
}*/

uint ADCNamesWin::filterValue() const
{
	return static_cast<ADCNamesToolbar *>(mpToolbar)->filterValue();
}

void ADCNamesWin::slotFilterType(int)// f)
{
	//Q_ASSERT(f == filterValue());
	ADCNamesView *pView(currentView());
	if (pView)
		pView->setFilter(filterValue());
}

ADCNamesView* ADCNamesWin::currentView()
{
	return dynamic_cast<ADCNamesView *>(mpTabWidget->currentWidget());
}

void ADCNamesWin::setModel(adcui::INamesViewModel *pIModel)
{
	mpView->setModel(pIModel);
}

void ADCNamesWin::populateToolbar()
{
	if (mpView)
		mpToolbar->addButton(mpView->mpRefreshAction);
}

void ADCNamesWin::populate()
{
}

void ADCNamesWin::showEvent(QShowEvent *e)
{
	if (mbDirty)
		slotUpdate();
	QWidget::showEvent(e);
}

void ADCNamesWin::slotUpdate()
{
	if (!isVisible())
	{
		mbDirty = true;
		return;
	}
	if (mpView)
		mpView->populate();
	/*ADCStream ss;
	emit signalDumpTypes(ss);
	mpView->populate(ss);*/
	mbDirty = false;
}

void ADCNamesWin::slotContextIdChanged(unsigned u)
{
	mpView->setContextId(u);
}

void ADCNamesWin::slotAboutToClose()
{
	depopulate();
	//	QAbstractItemModel *pModel(mpView->model());
	//	mpView->setModel(nullptr);
	//	delete pModel;
}


void ADCNamesWin::slotProjectNew()
{
	//?emit signalRequestModel(*this);
}





class ADCExportsViewProxyModel : public QSortFilterProxyModel
{
public:
	ADCExportsViewProxyModel(QWidget *parent)
		: QSortFilterProxyModel(parent)
	{
	}
	virtual ~ADCExportsViewProxyModel()
	{
		QAbstractItemModel *source(sourceModel());
		setSourceModel(nullptr);
		delete source;
	}
protected:
	virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
	{
		ADCExportsViewModel *pSourceModel(static_cast<ADCExportsViewModel *>(sourceModel()));
		QModelIndex i(pSourceModel->index(sourceRow, 0, sourceParent));
		return pSourceModel->isRowVisible(i);
	}
	virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const override
	{
		if (left.column() == (int)adcui::ExportViewColumns::ORD && right.column() == (int)adcui::ExportViewColumns::ORD)//by ordinals
		{
			QString l(sourceModel()->data(left).toString());
			QString r(sourceModel()->data(right).toString());
			bool ok;
			int a(l.toInt(&ok));
			if (ok)
			{
				int b(r.toInt(&ok));
				if (ok)
					return a < b;
			}
		}
		return QSortFilterProxyModel::lessThan(left, right);
	}
};

typedef ADCExportsViewProxyModel	ADCImportsViewProxyModel;


////////////////////////////////////////////////////////////
// ADCExportsView

ADCExportsView::ADCExportsView(QWidget *parent)
	: QTreeView(parent)
{
	//mpRefreshAction = new QAction(QIcon(":refresh.png"), tr("Refresh"), this);
	//connect(mpRefreshAction, SIGNAL(triggered()), SIGNAL(signalRefresh()));

	setFrameStyle(QFrame::Panel | QFrame::Sunken);
	setLineWidth(0);
	setMidLineWidth(0);
	setUniformRowHeights(true);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setSortingEnabled(true);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(slotCustomContextMenuRequested(const QPoint &)));
}

ADCExportsView::~ADCExportsView()
{
	QAbstractItemModel *pModel(model());
	QTreeView::setModel(nullptr);
	delete pModel;
}

QAbstractItemModel* ADCExportsView::newModel(adcui::IExportsViewModel* pIModel)
{
	ADCExportsViewModel* pModel(new ADCExportsViewModel(this, pIModel));
	QSortFilterProxyModel* proxyModel(new ADCExportsViewProxyModel(this));
	proxyModel->setSourceModel(pModel);
	proxyModel->setFilterKeyColumn(-1);
	proxyModel->setDynamicSortFilter(true);
	return proxyModel;
}

QAbstractItemModel* ADCImportsView::newModel(adcui::IExportsViewModel* pIModel)
{
	ADCImportsViewModel* pModel(new ADCImportsViewModel(this, pIModel));
	QSortFilterProxyModel* proxyModel(new ADCImportsViewProxyModel(this));
	proxyModel->setSourceModel(pModel);
	proxyModel->setFilterKeyColumn(-1);
	proxyModel->setDynamicSortFilter(true);
	return proxyModel;
}

void ADCExportsView::setModel(adcui::IExportsViewModel *pIModel)
{
	//destroy old model
	QAbstractItemModel *pOldModel(model());
	QTreeView::setModel(nullptr);
	delete pOldModel;
	if (pIModel)
		QTreeView::setModel(newModel(pIModel));
}

void ADCExportsView::populate()
{
	ModelPtr<ADCExportsViewModel> mod(model());
	if (mod.model)
	{
		mod.model->reset();
//		mod.proxy->sort(0);
		update();
	}
}

void ADCExportsView::slotCustomContextMenuRequested(const QPoint &pos)
{
	QModelIndex index(indexAt(pos));
	if (index.isValid())
	{
		QMenu menu;
		menu.addAction(QIcon(":trace.png"), tr("Go To Definition"), this, SLOT(slotGoToDeclaration()));
		menu.addSeparator();
		menu.addAction(QIcon(":refresh.png"), tr("Refresh"), this, SLOT(slotRefresh()));
		menu.exec(viewport()->mapToGlobal(pos));
	}
}

void ADCExportsView::slotGoToDeclaration()
{
	ModelPtr<ADCExportsViewModel> mod(model());
	if (mod.model)
		mod.model->goToDefinition(currentIndex());
}



///////////////////////////////////////////////////////////////////// ADCExportsWin

ADCExportsWin::ADCExportsWin(QWidget *parent, const char *name)
	: ADCTabsWin(parent, name),
	mbImp(false),
	mbDirty(true)
{

}

QWidget* ADCExportsWin::createView(QString path)
{
	ADCExportsView* pView;
	if (mbImp)
	{
		pView = new ADCImportsView(nullptr);
		pView->sortByColumn((int)adcui::ImportViewColumns::MODULE, Qt::AscendingOrder);
	}
	else
	{
		pView = new ADCExportsView(nullptr);
		pView->sortByColumn((int)adcui::ExportViewColumns::ORD, Qt::AscendingOrder);
	}
	pView->setFont(font());
	pView->header()->setFont(font());
	connect(pView, SIGNAL(signalRefresh()), SLOT(slotUpdate()));
	connect(pView, SIGNAL(activated(const QModelIndex &)), SLOT(slotDoubleClicked(const QModelIndex &)));

	emit signalRequestModel(*pView, path);
	pView->populate();
	//pView->setFilter(filterValue());
	return pView;
}

void ADCExportsWin::setModel(adcui::IExportsViewModel *)//pIModel)
{
	//mpView->setModel(pIModel);
}

ADCExportsView* ADCExportsWin::currentView() const
{
	return dynamic_cast<ADCExportsView *>(mpTabWidget->currentWidget());
}

void ADCExportsWin::slotDoubleClicked(const QModelIndex &index)
{
	QString sType(index.data(Qt::DisplayRole).toString());

	ADCExportsView* pView(currentView());

	ModelPtr<ADCExportsViewModel> mod(pView->model());//index.model());
	QString sModule(mod.model ? mod.model->moduleName() : "");
	if (sModule.isEmpty())
		return;

	QString s(sModule + MODULE_SEP + EXPORTS_EXT + "/" + sType);

	emit signalItemDoubleClicked(s);
}

void ADCExportsWin::slotUpdate()
{
	if (!isVisible())
	{
		mbDirty = true;
		return;
	}
	if (currentView())
		currentView()->populate();
	mbDirty = false;
}

void ADCExportsWin::showEvent(QShowEvent *e)
{
	if (mbDirty)
		slotUpdate();
	ADCTabsWin::showEvent(e);
}

