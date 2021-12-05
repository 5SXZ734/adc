#include <QLayout>
#include <QMenu>
#include <QHeaderView>
#include <QApplication>

#include "ADCExprWin.h"
#include "ADCUtils.h"

/*class ADCExprView : public QTreeWidget
{
public:
	ADCExprView(QWidget *parent)
		: QTreeWidget(parent)
	{
	}

	virtual void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QTreeWidgetItem *node = static_cast<QTreeWidgetItem *>(index.internalPointer());
		if (node && !node->parent())
		{
			QStyleOptionViewItem opt = option;

			if (selectionModel()->isSelected(index)) {
				opt.state |= QStyle::State_Selected;
			}

			int firstSection = header()->logicalIndex(0);
			int lastSection = header()->logicalIndex(header()->count() - 1);
			int left = header()->sectionViewportPosition(firstSection);
			int right = header()->sectionViewportPosition(lastSection) + header()->sectionSize(lastSection);
			int indent = 10 * indentation();

			left += indent;

			opt.rect.setX(left);
			opt.rect.setWidth(right - left);

			itemDelegate(index)->paint(painter, opt, index);
		}
		else {
			QTreeView::drawRow(painter, option, index);
		}
	}
	};*/


void ADCExprView::setModel(adcui::IADCExprViewModel *pIModel)
{
	//destroy old model
	QAbstractItemModel *pOldModel(model());
	QTreeView::setModel(nullptr);
	delete pOldModel;
	if (pIModel)
	{
		QTreeView::setModel(new ADCExprViewModel(this, pIModel));
		header()->resizeSection(0, 400);//
		header()->resizeSection(1, 30);//#
		header()->resizeSection(2, 40);//id
		header()->moveSection(0, 2);
	}
}

static void expandChildren(const QModelIndex &index, QTreeView *view)
{
    if (!index.isValid())
        return;
    int childCount = view->model()->rowCount(index);//index.model()
    for (int i = 0; i < childCount; i++) {
        const QModelIndex &child = view->model()->index(i, 0, index);
        // Recursively call the function for each child node.
        expandChildren(child, view);
    }
    if (!view->isExpanded(index))
        view->expand(index);
}

void ADCExprView::update()
{
	blockSignals(true);
	ADCExprViewModel *pModel(reinterpret_cast<ADCExprViewModel *>(model()));
	Qt::KeyboardModifiers key(QApplication::keyboardModifiers());
	bool bPtrDump((key & Qt::ShiftModifier) != 0);
	pModel->resetEx(bPtrDump ? (adcui::IADCExprViewModel::DUMPEXPR_PTRS) : adcui::IADCExprViewModel::DUMPEXPR_NULL);
	resizeColumnToContents(0);
	resizeColumnToContents(1);
	resizeColumnToContents(2);
	for (int i = 0; i < model()->rowCount(QModelIndex()); i++)
	{
//		setFirstColumnSpanned(i, QModelIndex(), true);//? WARNING: suppresses other columns' labels
		QModelIndex index(model()->index(i, 0));
		expandChildren(index, this);
		collapse(index);
	}
	blockSignals(false);
	QTreeView::update();
}

////////////////////////////////////////////////////////////
ADCExprWin::ADCExprWin(QWidget * parent, const char *name)
	: QSplitter(parent)//, name)
{
	setObjectName(name);

	mpListView = new ADCExprView(this);
	mpListView->setRootIsDecorated(true);

	//mpListView->setAllColumnsShowFocus(false);
//	mpListView->setItemsExpandable(false);
	mpListView->setSelectionMode(QAbstractItemView::SingleSelection);
//	mpListView->setSelectionBehavior(QAbstractItemView::SelectRows);

	//mpListView->setFocusPolicy(QListView::NoFocus);
	//mpListView->setSelectionMode(QListView::NoSelection);
	//mpListView->setSorting(-1);
/*	
	mpPtrExprView = new QTreeWidget(this);
	mpPtrExprView->hide();*/

	//QVBoxLayout * vbox(new QVBoxLayout(this));
	//vbox->setContentsMargins(0, 0, 0, 0);
	//vbox->addWidget(mpListView);
}

void ADCExprWin::setModel(adcui::IADCExprViewModel *model)
{
	mpListView->setModel(model);
}

void ADCExprWin::updateOutputFont(const QFont &f)
{
	mpListView->setFont(f);
}

void ADCExprWin::slotProjectNew()
{
//	mpListView->clear();
	emit signalRequestModel(*this);
}

void ADCExprWin::slotAboutToClose()
{
	mpListView->setModel(nullptr);
}

void ADCExprWin::slotCurExprChanged()
{
	if (!isVisible())
		return;
}

void ADCExprWin::slotCurInfoChanged(const ADCCurInfo &)
{
	if (!isVisible())
		return;

/*	ADCStream ss;
	emit signalDumpPtrExprList(ss);

	QString s;
	while (ss.ReadString(s))
	{
		QStringList l(s.split('\t'));
		QTreeWidgetItem *lvi(new QTreeWidgetItem(mpPtrExprView));
		lvi->setText(0, QString("%1:%2").arg(l[0]).arg(l[1]));
	}*/
}

void ADCExprWin::slotSyncResponce(bool)
{
	if (!isVisible())
		return;
	/*bool bSync(bForce);
	if (!bSync)
		emit signalSyncModeInquiry(bSync);
	if (bSync)*/
		mpListView->update();
}

