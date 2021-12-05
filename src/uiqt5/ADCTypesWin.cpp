#include <QtCore/QSignalMapper>
#include <QtCore/QVariant>
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

#include "ADCTypesWin.h"
#include "ADCBinWin.h"


struct ModelPtr
{
	ADCRecentTypesModel* recent;
	QSortFilterProxyModel*	proxy;
	ADCTypesViewModel*	model;
	ModelPtr(ADCTypesView *p)
	{
		QAbstractItemModel* pModel(p->model());
		recent = dynamic_cast<ADCRecentTypesModel *>(pModel);
		if (recent)
			pModel = recent->saved();

		proxy = dynamic_cast<QSortFilterProxyModel *>(pModel);
		model = proxy ? dynamic_cast<ADCTypesViewModel *>(proxy->sourceModel()) : nullptr;
		if (!model)
			model = dynamic_cast<ADCTypesViewModel *>(pModel);
	}
};


ADCTypesDlg::ADCTypesDlg(QWidget* parent, const char *name)
	: ADCTypesDlgBase(parent),
	miArray(0),
	mpCompleter(nullptr)
{
	setObjectName(name);
	setWindowTitle(tr("Edit Type"));
	setWindowIcon(QIcon(":type_edit_24.png"));

	setModal(false);

	createContents(this);

	mpCompleter = new ADCTypesCompleter(this);
	mpCompleter->setCompletionMode(QCompleter::PopupCompletion);//UnfilteredPopupCompletion
	mpCompleter->setCaseSensitivity(Qt::CaseSensitive);
	//mpCompleter->setSeparator(QLatin1String("::"));

	//connect(mpCompleter, SIGNAL(highlighted(QModelIndex)), SLOT(slotHighlight(QModelIndex)));
	connect(mpCompleter, QOverload<const QModelIndex &>::of(&QCompleter::highlighted),
		[=](const QModelIndex& index) { slotHighlight(index); });

	mpLineEdit->setCompleter(mpCompleter);

	resize(QSize(640, 480).expandedTo(minimumSizeHint()));
	//clearWState(Qt::WState_Polished );
}

void ADCTypesDlg::createContents(QWidget *parent)
{
	//mpNameLbl = new QLabel(tr("Context:"), parent);
	//mpContextLbl = new QLabel(tr("Context:"), parent);

	mpLineEdit = new QLineEdit(parent);
	mpLineEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
	//connect(mpLineEdit, SIGNAL(returnPressed()), SLOT(slot Apply()));
	mpLineEdit->setToolTip(tr("Target Type"));
	//mpLineEdit->setReadOnly(true);
	//mpLineEdit->setDisabled(true);
	mpLineEdit->installEventFilter(this);
	mpLineEdit->setWhatsThis(tr("Type a type name here."));

	mpToolbar = new ADCTypesToolbar(parent);
	connect(mpToolbar, SIGNAL(signalFilterChanged(int)), SLOT(slotFilterType(int)));

	mpApplyPBtn = new QPushButton(tr("Apply"), parent);
	connect(mpApplyPBtn, SIGNAL(clicked()), SLOT(slotApply()));

	//mpOkPBtn = new QPushButton(tr("OK"), parent);
	//connect(mpOkPBtn, SIGNAL(clicked()), SLOT(slotOK()));

	mpHidePBtn = new QPushButton(tr("Hide"), parent);
	connect(mpHidePBtn, SIGNAL(clicked()), SLOT(slotHide()));

	mpListView = new ADCTypesView2(parent);
	mpListView->setHeaderHidden(true);
	mpListView->setFont(font());
	//mpListView->setSelectionMode(QItemSelectionModel::SingleSelection);
	connect(mpListView, SIGNAL(activated(const QModelIndex &)), SLOT(slotItemActivated(const QModelIndex &)));
	connect(mpListView, SIGNAL(signalCurrentChanged(const QModelIndex &, const QModelIndex &)), SLOT(slotItemSelected(const QModelIndex &, const QModelIndex &)));
	//connect(mpListView, SIGNAL(signalRefresh()), SLOT(slotUpdate()));
	//connect(mpListView, SIGNAL(activated(const QModelIndex &)), SLOT(slotDoubleClicked(const QModelIndex &)));
	//	emit signalRequestModel(*mpListView, path);
	//	mpListView->populate();
	//	mpListView->setFilter(filterValue());
	mpListView->installEventFilter(this);
	mpListView->setWhatsThis(tr("Double-click to apply a type at current location."));

	// ACTIONS

	mpApplyPtrAction = new QAction(QIcon(":apply_ptr.png"), tr("Apply Pointer"), this);
	//mpApplyPtrAction->setShortcut(QKeySequence("Ctrl+P"));
	//mpApplyPtrAction->setShortcutContext(Qt::WindowShortcut);
	connect(mpApplyPtrAction, SIGNAL(triggered()), SLOT(slotApplyPointer()));

	mpApplyConstAction = new QAction(QIcon(":apply_const.png"), tr("Apply Const"), this);
	//mpApplyPtrAction->setShortcut(QKeySequence("Ctrl+C"));
	//mpApplyPtrAction->setShortcutContext(Qt::WindowShortcut);
	connect(mpApplyConstAction, SIGNAL(triggered()), SLOT(slotApplyConst()));

	mpApplyArrayAction = new QAction(QIcon(":apply_array.png"), tr("Apply Array"), this);
	//mpApplyPtrAction->setShortcut(QKeySequence("Ctrl+R"));
	//mpApplyPtrAction->setShortcutContext(Qt::WindowShortcut);
	connect(mpApplyArrayAction, SIGNAL(triggered()), SLOT(slotApplyArray()));

	mpApplyUndoAction = new QAction(QIcon(":apply_undo.png"), tr("Undo Tier"), this);
	//mpApplyPtrAction->setShortcut(QKeySequence(Qt::Key_Backspace));
	//mpApplyPtrAction->setShortcutContext(Qt::WindowShortcut);
	connect(mpApplyUndoAction, SIGNAL(triggered()), SLOT(slotApplyUndo()));

	/*mpApplyAction = new QAction(QIcon(":apply_type.png"), tr("Set at Context"), this);
	mpApplyPtrAction->setShortcut(QKeySequence(Qt::Key_Return));
	mpApplyPtrAction->setShortcutContext(Qt::WindowShortcut);
	connect(mpApplyAction, SIGNAL(triggered()), SLOT(slotApply()));*/

	mpToolbar->addStretch();
	mpToolbar->addButton(mpApplyPtrAction);
	mpToolbar->addButton(mpApplyConstAction);
	mpArrayNumSBx = mpToolbar->addSpinBox();
	mpArrayNumSBx->installEventFilter(this);
	mpToolbar->addButton(mpApplyArrayAction);
	mpToolbar->addButton(mpApplyUndoAction);

	enableButtons(false);

	QVBoxLayout* pVBox(new QVBoxLayout);
	//pVBox->setContentsMargins(0, 0, 0, 0);
	pVBox->setSpacing(0);
	//pVBox->addWidget(mpContextLbl);
	pVBox->addWidget(mpListView);
	pVBox->addWidget(mpToolbar);
	//pVBox->addWidget(mpNameLbl);

	QFrame* pLine(new QFrame);
	pLine->setFrameShape(QFrame::HLine);
	pLine->setFrameShadow(QFrame::Sunken);

	QHBoxLayout* pHBox(new QHBoxLayout);
	pHBox->addStretch();
	pHBox->addWidget(mpApplyPBtn);
	//pHBox->addStretch();
	//pHBox->addWidget(mpOkPBtn);	
	pHBox->addWidget(mpHidePBtn);

	QVBoxLayout* pBaseLayout(new QVBoxLayout(parent));
	pBaseLayout->addWidget(mpLineEdit);
	pBaseLayout->addLayout(pVBox);
	pBaseLayout->addWidget(pLine);
	pBaseLayout->addLayout(pHBox);

	setTabOrder(mpLineEdit, mpListView);
}

ADCTypesDlg::~ADCTypesDlg()
{
}

void ADCTypesDlg::slotHighlight(const QModelIndex &index)
{
    QAbstractItemModel *completionModel = mpCompleter->completionModel();
    QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel *>(completionModel);
    if (!proxy)
        return;
    QModelIndex sourceIndex = proxy->mapToSource(index);
    mpListView->selectionModel()->select(sourceIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    mpListView->scrollTo(sourceIndex);
}

bool ADCTypesDlg::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::KeyPress)
	{
		QKeyEvent *ke(static_cast<QKeyEvent *>(e));
		//bool bShift(ke->modifiers() & Qt::ShiftModifier);
		bool bControl(ke->modifiers() & Qt::ControlModifier);

		if (bControl)
		{
			//if (obj == mpListView || obj == mpArrayNumSBx)
			{
				switch (ke->key())
				{
				case Qt::Key_P:
					slotApplyPointer();
					break;
				case Qt::Key_C:
					slotApplyConst();
					break;
				case Qt::Key_R:
					slotApplyArray();
					break;
				case Qt::Key_T:
					mpLineEdit->setFocus();
					break;
				case Qt::Key_Backspace:
					slotApplyUndo();
					break;
				case Qt::Key_Return:
					slotApply();
					break;
				}
				return true;
			}
			/*if (ke->key() == Qt::Key_A)
			{
				if (obj == mpListView)
				{
					mpArrayNumSBx->setFocus();
					mpArrayNumSBx->selectAll();
				}
				else if (obj == mpArrayNumSBx)
				{
					mpListView->setFocus();
				}
			}
			return true;*/
		}
		//if (obj == mpListView)
		{
			/*if (ke->key() == Qt::Key_Backspace)
			{
				slotApplyUndo();
				return true;
			}*/
			if (ke->key() == Qt::Key_Return)
			{
				slotApply();
				return true;
			}
		}
	}

	return ADCTypesDlgBase::eventFilter(obj, e);
}

void ADCTypesDlg::depopulate()
{
	mpCompleter->setModel(nullptr);
	mpListView->setModel(nullptr);
	//mpNameLbl->setText(tr("Context:"));
	mpLineEdit->clear();
	mpArrayNumSBx->setValue(1);
	mTypeChain.clear();
	enableButtons(false);
}

void ADCTypesDlg::updateOutputFont(const QFont &f)
{
	mpCompleter->popup()->setFont(f);
	mpListView->setFont(f);
	mpLineEdit->setFont(f);
	mpArrayNumSBx->setFont(f);
}

void ADCTypesDlg::showEvent(QShowEvent *)
{
	//if (!mGeometry.isEmpty())
	//setGeometry(mGeometry);
	//ADCTypesDlgBase::showEvent(e); - THIS RESETS A POSITION
	slotUpdateContextTypes();

	mpLineEdit->setFocus();
}

void ADCTypesDlg::setModel(adcui::ITypesViewModel* pIModel)
{
	mpListView->setModel(pIModel);
	mpCompleter->setModel(view().model());
}

void ADCTypesDlg::slotUpdateContextTypes()
{
	//emit signalRequestModel(view(), QString());//from context
	emit signalRequestModel(*this, QString());//from context
	view().populate();
	slotFilterType(0);

	//mpListView->setFocus();
}

void ADCTypesDlg::closeEvent(QCloseEvent *e)
{
	//mGeometry = geometry();
	//view().setModel(nullptr);
	ADCTypesDlgBase::closeEvent(e);
}

void ADCTypesDlg::slotApplyArray()
{
	Q_ASSERT(!mTypeChain.isEmpty());
	mTypeChain.push_front(QString("[%1]").arg(mpArrayNumSBx->value()));
	updateTypeString();
}

void ADCTypesDlg::slotApplyPointer()
{
	Q_ASSERT(!mTypeChain.isEmpty());
	mTypeChain.push_front("*");
	updateTypeString();
}

void ADCTypesDlg::slotApplyConst()
{
	Q_ASSERT(!mTypeChain.isEmpty());
	mTypeChain.push_front("const");
	updateTypeString();
}

void ADCTypesDlg::slotApplyUndo()
{
	Q_ASSERT(!mTypeChain.isEmpty());
	mTypeChain.pop_front();
	updateTypeString();
	if (!mpApplyUndoAction->isEnabled())
		mpListView->setFocus();//otherwise a lineedit somehow becomes selected (why?)
}

void ADCTypesDlg::updateTypeString()
{
	QString s(typeString());
	s.remove("%1");
	mpLineEdit->setText(s);
	//	mpLineEdit->setSelection(0, 0);
	enableButtons(!s.isEmpty());
}

void ADCTypesDlg::slotItemSelected(const QModelIndex &index, const QModelIndex &)
{
	QString s(index.data(Qt::DisplayRole).toString());
	mTypeChain.clear();
	mTypeChain.push_back(s);
	updateTypeString();
}

QString ADCTypesDlg::typeString() const
{
	if (mTypeChain.isEmpty())
		return QString();

	QString rs("%1");//placeholder for a data

	int left(0), right(0);//track associativity
	for (QStringList::ConstIterator i(mTypeChain.constBegin()); i != mTypeChain.constEnd(); i++)
	{
		const QString &s(*i);
		//if (rs.isEmpty())
		//rs = *i;
		if (s == "*")
		{
			rs.prepend(s);
			left++;
			right = 0;
		}
		else if (s == "const")
		{
			rs.prepend(s + " ");
			left++;
			right = 0;
		}
		else if (s.startsWith("["))
		{
			if (left > 0)
				rs = "(" + rs + ")";
			rs.append(s);
			right++;
			left = 0;
		}
		else
		{
			if (!rs[0].isPunct())
				rs.prepend(" ");
			rs.prepend(s);
			break;
		}
	}

	return rs;
}

void ADCTypesDlg::enableButtons(bool)
{
	bool bEnable(!mTypeChain.isEmpty());
	bool bUndoEnable(mTypeChain.length() > 1);
	bool bConstEnable(!mTypeChain.isEmpty() && mTypeChain.front() != "const");

	mpApplyArrayAction->setEnabled(bEnable);
	mpArrayNumSBx->setEnabled(bEnable);
	mpApplyPtrAction->setEnabled(bEnable);
	mpApplyConstAction->setEnabled(bConstEnable);
	mpApplyUndoAction->setEnabled(bUndoEnable);
	//mpApplyAction->setEnabled(bEnable);
	mpApplyPBtn->setEnabled(bEnable);
}

void ADCTypesDlg::slotItemActivated(const QModelIndex &)//index)
{
	slotApply();
	//accept();
	/*mpLineEdit->setReadOnly(true);
	QString s(index.data(Qt::DisplayRole).toString());
	mpLineEdit->setText(s);
	enableButtons(true);*/
}

//	return static_cast<ADCTypesToolbar *>(mpToolbar)->filterValue();
void ADCTypesDlg::slotFilterType(int)// f)
{
	ModelPtr mod(mpListView);
	if (!mod.model)
		return;

	uint u(mpToolbar->filterValue());
	if (u == (1 << adcui::ITypesViewModel::E__TYPES_TOTAL))//favorites clicked
	{
		mpListView->setFilter(0);//destroy proxy model
		mpListView->QTreeView::setModel(mod.model->recentTypesModel());
	}
	else
	{
		if (mod.recent)
			mpListView->QTreeView::setModel(mod.recent->saved());//go back to normal model
		mpListView->setFilter(u);
	}
}

void ADCTypesDlg::slotLocusChanged(QString s, int)
{
	//mpNameLbl->setText(tr("Context: <font color=darkRed>%1</font>").arg(s));
}

/*void ADCTypesDlg::slotOKclicked()
{
//hide();
signalClosePage();
}*/

void ADCTypesDlg::slotOK()
{
	slotApply();
	hide();
}

void ADCTypesDlg::slotApply()
{
	ModelPtr mod(mpListView);
	if (mod.model)
		mod.model->applyType(mpLineEdit->text(), mpListView->currentIndex());
}

void ADCTypesDlg::slotHide()
{
	hide();
}

/*void ADCTypesDlg::slotCurrentChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
{
if (current == mpCurItem)
mpArrayNumSBx->setValue(miArray);
else
mpArrayNumSBx->setValue(0);
}

void ADCTypesDlg::slotDoubleClicked(QTreeWidgetItem * lvi, int)
{
if (lvi)
accept();
}*/

void ADCTypesDlg::setField(QString s)
{
	/*QString s0;
	int n(s.lastIndexOf("."));
	if (n >= 0)
	{
	s0 = s.left(n);
	s.remove(0, n + 1);
	}
	mpNameLbl->setText(tr("Object: %1").arg(s));*/
	//mpContextLbl->setText(tr("Context: %1").arg(s0));
}

void ADCTypesDlg::setContents(const QStringList&)//l)
{
	/*	mpCurItem = nullptr;
		mpListView->clear();
		QTreeWidgetItem *pRootItem(nullptr);
		for (QStringList::ConstIterator it(l.begin()); it != l.end(); it++)
		{
		QTreeWidgetItem *pItem;

		QString s(*it);
		if (s.startsWith("[") && s.endsWith("]"))
		{
		pItem = pRootItem = new QTreeWidgetItem(mpListView);
		pRootItem->setExpanded(true);
		}
		else
		{
		if (pRootItem)
		pItem = new QTreeWidgetItem(pRootItem);
		else
		pItem = new QTreeWidgetItem(mpListView);
		}
		if (s.startsWith("->"))
		{
		s.remove(0, 2);
		mpListView->setCurrentItem(pItem);
		pItem->setIcon(0, QIcon(":lightning.png"));
		mpCurItem = pItem;
		}
		pItem->setText(0, s);
		}*/
}

void ADCTypesDlg::setArray(int i)
{
	miArray = i;
	mpArrayNumSBx->setValue(i);
}

int ADCTypesDlg::array() const
{
	return mpArrayNumSBx->value();
}

QString ADCTypesDlg::getSelected()
{
	/*?QTreeWidgetItem * p = mpListView->currentItem();
	if (!p)
	return "";
	return p->text(0);*/return "";
}





////////////////////////////////////////////////////////////
// ADCTypesView

ADCTypesView::ADCTypesView(QWidget *parent)
	: QTreeView(parent)
{
	setRootIsDecorated(false);
	/*  QStandardItemModel *pModel = new QStandardItemModel( 5, 2 );
	  for (int r = 0; r < 5; r++)
	  for (int c = 0; c < 2; c++)
	  {
	  QStandardItem *item = new QStandardItem(QString("Row:%0, Column:%1").arg(r).arg(c));

	  if (c == 0)
	  for (int i = 0; i < 3; i++)
	  {
	  QStandardItem *child = new QStandardItem(QString("Item %0").arg(i));
	  child->setEditable(false);
	  item->appendRow(child);
	  }

	  pModel->setItem(r, c, item);
	  }
	  */

	//TreeModel *pModel = new TreeModel(g_data, this);

	//ADCTypesViewModel *pModel = new ADCTypesViewModel(this);
	//setModel(pModel);
	/*	setRootIsDecorated(true);
		//setAllColumnsShowFocus(false);
		//setItemsExpandable(true);
		setExpandsOnDoubleClick(false);
		setSelectionMode(QAbstractItemView::SingleSelection);
		setSelectionBehavior(QAbstractItemView::SelectItems);
		//setFocusPolicy(QListView::NoFocus);
		//setSelectionMode(QListView::NoSelection);
		//setSortingEnabled(true);
		setColumnCount(1);
		setHeaderHidden(true);
		headerItem()->setText(0, tr("Name"));
		//headerItem()->setText(1, tr("VA"));
		//header()->moveSection(0, 1);
		header()->setSectionResizeMode(QHeaderView::ResizeToContents);
		*/
	mpRefreshAction = new QAction(QIcon(":refresh.png"), tr("Refresh"), this);
	connect(mpRefreshAction, SIGNAL(triggered()), SIGNAL(signalRefresh()));

	//QLayout *l(layout());

	//setLineWidth(0);
	//setFrameStyle(QFrame::NoFrame);
	setFrameStyle(QFrame::Panel | QFrame::Sunken);
	//setLineWidth(0);
	//setMidLineWidth(0);

	//header()->resizeSection(0, 200);
	viewport()->installEventFilter(new QToolTipper(this));

	//setUniformItemSizes(true);//for QListView
	setUniformRowHeights(true);
}

ADCTypesView::~ADCTypesView()
{
	QAbstractItemModel *pModel(model());
	QTreeView::setModel(nullptr);
	delete pModel;
}

class ADCTypesViewProxyModel : public QSortFilterProxyModel
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
	virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
	{
		ADCTypesViewModel *pSourceModel(static_cast<ADCTypesViewModel *>(sourceModel()));
		//QModelIndex index0 = pSourceModel->index(sourceRow, 0, sourceParent);
		QModelIndex i(pSourceModel->index(sourceRow, 0, sourceParent));
		return pSourceModel->isRowVisible(i);
	}
};

void ADCTypesView::setModel(adcui::ITypesViewModel *pIModel)
{
	//destroy old model
	QAbstractItemModel *pOldModel(model());
	QTreeView::setModel(nullptr);
	delete pOldModel;

	if (pIModel)
	{
#if(1)
		QTreeView::setModel(new ADCTypesViewModel(this, pIModel));
		int n(header()->count());
		header()->resizeSection(n - 2, 600);
		header()->setSectionResizeMode(n - 1, QHeaderView::Stretch);

#else
		QSortFilterProxyModel *proxyModel(new ADCTypesViewProxyModel(this));
		//proxyModel->setSortingEnabled(true);
		proxyModel->setSourceModel(new ADCTypesViewModel(this, pIModel));
		proxyModel->setFilterKeyColumn(0);
		proxyModel->setDynamicSortFilter(true);
		QTreeView::setModel(proxyModel);
		//mpView->setSortingEnabled(true);
		//mpView->sortByColumn(0, Qt::AscendingOrder);
#endif
	}
}

void ADCTypesView2::setModel(adcui::ITypesViewModel * pIModel)
{
	ModelPtr mod(this);
	//destroy old model
	QAbstractItemModel* pOldModel(model());
	QTreeView::setModel(nullptr);
	if (mod.recent != pOldModel)
		delete pOldModel;
	if (pIModel)
	{
		ADCTypesViewModel* pModel(new ADCTypesViewModel(this, pIModel));
		pModel->setColumnCount(1);
		QTreeView::setModel(pModel);
	}
}

void ADCTypesView::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Delete)
	{
		ModelPtr mod(this);
		if (mod.recent)
			mod.recent->removeRow(currentIndex().row());
	}

	//if (e->modifiers() & Qt::ShiftModifier)
	//return;
	QTreeView::keyPressEvent(e);
}

void ADCTypesView::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
	ADCTypesView::QTreeView::currentChanged(current, previous);
	emit signalCurrentChanged(current, previous);
}

void ADCTypesView::populate()
{
	ModelPtr mod(this);
	if (mod.model)
	{
		mod.model->reset();
		if (mod.proxy)
			mod.proxy->sort(0);
		update();
	}
}

void ADCTypesView::setContextId(unsigned u)
{
	ModelPtr mod(this);
	if (mod.model && mod.model->setContextId(u))
		viewport()->update();
}

void ADCTypesView::setFilter(unsigned u)
{
	ModelPtr mod(this);
	if (mod.model)
	{
		if (mod.model->setFilter(u))
		{
			if (!mod.proxy)
			{
				QSortFilterProxyModel* proxyModel(new ADCTypesViewProxyModel(this));
				//proxyModel->setSortingEnabled(true);
				proxyModel->setSourceModel(mod.model);
				proxyModel->setFilterKeyColumn(0);
				proxyModel->setDynamicSortFilter(true);
				QTreeView::setModel(proxyModel);
				mod.proxy = proxyModel;
				mod.proxy->invalidate();
				//mod.model->reset();
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
				//mod.proxy->invalidate();
			}
		}
	}
}


void ADCTypesView::populate(ADCStream &/*ss*/)
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




///////////////////////////////////////////////////////////////////// ADCTypesToolbar

ADCTypesToolbar::ADCTypesToolbar(QWidget *parent)
	: ADCWinToolbar(parent)
{
	QSignalMapper *signalMapper = new QSignalMapper(parent);
	//connect(signalMapper, SIGNAL(mapped(int)), SLOT(slotFilterType(int)));
	connect(signalMapper, SIGNAL(mapped(int)), SLOT(slotFilterChanged(int)));

	PQAction &a0(m_typesFilter[adcui::ITypesViewModel::E_PRIMITIVE]);
	a0 = new QAction(QIcon(":type_3.png"), tr("Primitive"), this);

	PQAction &a1(m_typesFilter[adcui::ITypesViewModel::E_COMPOUND]);
	a1 = new QAction(QIcon(":type_4.png"), tr("Compound"), this);

	PQAction &a6(m_typesFilter[adcui::ITypesViewModel::E_ENUM]);
	a6 = new QAction(QIcon(":enum.png"), tr("Enumeration"), this);

	PQAction &a7(m_typesFilter[adcui::ITypesViewModel::E_FUNC]);
	a7 = new QAction(QIcon(":function.png"), tr("Function"), this);

	PQAction &a2(m_typesFilter[adcui::ITypesViewModel::E_CLASS]);
	a2 = new QAction(QIcon(":type_class.png"), tr("Class"), this);

	PQAction &a5(m_typesFilter[adcui::ITypesViewModel::E_TYPEDEF]);
	a5 = new QAction(QIcon(":type_0.png"), tr("Typedef"), this);

	PQAction &a3(m_typesFilter[adcui::ITypesViewModel::E_CODE]);
	a3 = new QAction(QIcon(":type_code.png"), tr("Code"), this);

	PQAction &a4(m_typesFilter[adcui::ITypesViewModel::E_CONTEXT_DEPENDENT]);
	a4 = new QAction(QIcon(":type_9.png"), tr("Context Dependent"), this);

	PQAction &a9(m_typesFilter[E_FAVORITE]);
	a9 = new QAction(QIcon(":favorite_16.png"), tr("Recent (Favorite)"), this);

	//populate toolbar
	for (int i(0); i < E_TOTAL; i++)
	{
		m_typesFilter[i]->setCheckable(true);
		if (i != adcui::ITypesViewModel::E_CLASS)
		{
			if (i == E_FAVORITE)
				addSeparator();
			addButton(m_typesFilter[i]);
			connect(m_typesFilter[i], SIGNAL(triggered()), signalMapper, SLOT(map()));
			signalMapper->setMapping(m_typesFilter[i], i);
		}
	}

	

	//mpFavoriteAction = new QAction(QIcon(":favorite_16.png"), tr("Recent (Favorite)"), this);
	//addButton(mpFavoriteAction);

	addStretch();
}

void ADCTypesToolbar::slotFilterChanged(int u)
{
	PQAction &a(m_typesFilter[E_FAVORITE]);

	if (u != E_FAVORITE)
		a->setChecked(false);
	else
		for (int i(0); i < E_FAVORITE; i++)
			m_typesFilter[i]->setChecked(false);

	emit signalFilterChanged(u);
}

uint ADCTypesToolbar::filterValue() const
{
	uint u(0);
	for (int i(0); i < E_TOTAL; i++)
		if (m_typesFilter[i]->isChecked())
			u |= (1 << i);
	return u;
}



///////////////////////////////////////////////////////////////////// ADCTypesWin

ADCTypesWin::ADCTypesWin(QWidget *parent, const char *name)
	: ADCTabsWin(parent, name),//new ADCWinToolbar(this)),//QSplitter(Qt::Vertical, parent)//, name)
	mbDirty(true),
	mpView(nullptr),
	mpBinView(nullptr)
{
	ADCTypesToolbar *pToolbar(new ADCTypesToolbar(this));
	connect(pToolbar, SIGNAL(signalFilterChanged(int)), SLOT(slotFilterType(int)));
	setToolbar(pToolbar);

	////////////////////////////////
	//mpView = createView();
	//mpTabWidget->addTab(mpView, tr("test"));


	//	connect(mpView, SIGNAL(signalRefresh()), SLOT(slotUpdate()));
	//	connect(mpView, SIGNAL(clicked(const QModelIndex &)), SLOT(slotTypeActivated(const QModelIndex &)));
	//connect(this, SIGNAL(signalContextIdChanged(unsigned)), mpView, SLOT(slotContextIdChanged(unsigned)));
	//connect(mpView, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(slotItemDoubleClicked(QTreeWidgetItem *, int)));
	//connect(mpView, SIGNAL(signalCurLineChanged(int)), SLOT(slotItemClicked(int)));
	//connect(mpView, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), SLOT(slotCurrentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

	populateToolbar();
}

ADCTypesWin::~ADCTypesWin()
{
}

QWidget* ADCTypesWin::createView(QString path)
{
	ADCTypesView *pView(new ADCTypesView(nullptr));
	pView->setFont(font());
	connect(pView, SIGNAL(signalRefresh()), SLOT(slotUpdate()));
	//connect(pView, SIGNAL(clicked(const QModelIndex &)), SLOT(slotTypeActivated(const QModelIndex &)));
	connect(pView, SIGNAL(activated(const QModelIndex &)), SLOT(slotDoubleClicked(const QModelIndex &)));

	emit signalRequestModel(*pView, path);
	pView->populate();
	pView->setFilter(filterValue());
	return pView;
}

void ADCTypesWin::setBinModel(ADCModelDataMap &rOwner, adcui::IBinViewModel *pIModel)
{
	if (!mpBinView)
		return;
	if (pIModel)
	{
		ADCDocPos *pCur(new ADCDocTablePos(pIModel));
		ADCModelData *old(mpBinView->setModelData(new ADCModelData(rOwner, pIModel, nullptr, pCur)));
		delete old;
	}
	else
	{
		assert(0);
	}
}

void ADCTypesWin::onCurrentChanged()
{
	slotFilterType(0);
}

void ADCTypesWin::slotTypeActivated(const QModelIndex &)
{
	//QString s(index.data(Qt::DisplayRole).toString());
	//signalRequestBinModel(*this, s);
}

void ADCTypesWin::slotDoubleClicked(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	//get type name
	QModelIndex index2(index.model()->index(index.row(), 0));
	QString type(index2.data(Qt::DisplayRole).toString());

	//get scope
	QModelIndex index3(index.model()->index(index.row(), 1));
	QString scope(index3.data(Qt::DisplayRole).toString());

	if (scope != ATTIC_NAME)
	{
		ModelPtr mod(currentView());
		QString sModule(mod.model ? mod.model->moduleName() : "");
		Q_ASSERT(!sModule.isEmpty());
		scope = sModule + MODULE_SEP;
		//scope = sModule + MODULE_SEP TYPES_EXT FOLDER_SEP;
	}

	QString s(scope + type);

	emit signalItemDoubleClicked(s);

	//if (!mpBinView)
	//mpBinView = new ADCBinView(this);


	//signalRequestBinModel(*this, s);
}

/*void ADCTypesWin::init()
{
emit signalRequestModel(*this);
}*/

uint ADCTypesWin::filterValue() const
{
	return static_cast<ADCTypesToolbar *>(mpToolbar)->filterValue();
}

void ADCTypesWin::slotFilterType(int)// f)
{
	//Q_ASSERT(f == filterValue());
	ADCTypesView *pView(currentView());
	if (pView)
		pView->setFilter(filterValue());
}

ADCTypesView* ADCTypesWin::currentView()
{
	return dynamic_cast<ADCTypesView *>(mpTabWidget->currentWidget());
}

void ADCTypesWin::setModel(adcui::ITypesViewModel *pIModel)
{
	mpView->setModel(pIModel);
}

void ADCTypesWin::populateToolbar()
{
	if (mpView)
		mpToolbar->addButton(mpView->mpRefreshAction);
}

void ADCTypesWin::populate()
{
}

void ADCTypesWin::showEvent(QShowEvent *e)
{
	if (mbDirty)
		slotUpdate();
	QWidget::showEvent(e);
}

void ADCTypesWin::slotUpdate()
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

void ADCTypesWin::slotContextIdChanged(unsigned u)
{
	mpView->setContextId(u);
}

void ADCTypesWin::slotAboutToClose()
{
	depopulate();
	//	QAbstractItemModel *pModel(mpView->model());
	//	mpView->setModel(nullptr);
	//	delete pModel;
}


void ADCTypesWin::slotProjectNew()
{
	//?emit signalRequestModel(*this);
}


