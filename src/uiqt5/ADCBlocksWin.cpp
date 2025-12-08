#include <QLayout>
#include <QMenu>
#include <QHeaderView>
#include <QStack>

#include "ADCBlocksWin.h"
#include "ADCUtils.h"

#define BLOCK_NUMS 0

ADCBlocksWin::ADCBlocksWin(QWidget * parent, const char *name)
	: ADCInterWin(parent, name)
{
	mpView = new QTreeWidget(this);
	mpView->setRootIsDecorated(true);
	//mpView->setAllColumnsShowFocus(false);
	//mpView->setItemsExpandable(true);
	mpView->setSelectionMode(QAbstractItemView::SingleSelection);
	mpView->setSelectionBehavior(QAbstractItemView::SelectItems);
	mpView->setContextMenuPolicy(Qt::CustomContextMenu);

	//mpView->setFocusPolicy(QListView::NoFocus);
	//mpView->setSelectionMode(QListView::NoSelection);
	//mpView->setSorting(-1);
#if(BLOCK_NUMS)
	mpView->setColumnCount(2);
	mpView->headerItem()->setText(0, tr("Blocks"));
	mpView->headerItem()->setText(1, tr("#"));
	mpView->header()->moveSection(0, 1);
#else
	mpView->setColumnCount(1);
	mpView->header()->hide();
#endif

	connect(mpView, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), SLOT(slotCurrentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
	connect(mpView, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(slotCustomContextMenuRequested(const QPoint &)));

	toolbarMenu()->addAction(new QAction(QIcon(":refresh.png"), tr("Refresh"), this));//dummy action

	QVBoxLayout *vbox(dynamic_cast<QVBoxLayout *>(layout()));
	vbox->addWidget(mpView);
}

static bool isParentOf(QTreeWidgetItem *p, QTreeWidgetItem *parent)
{
	while (p)
	{
		p = p->parent();
		if (p == parent)
			return true;
	}
	return false;
}

void ADCBlocksWin::slotCurrentItemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous)
{
	if (previous)
	{
		QTreeWidgetItemIterator i(previous);
		for (++i; *i && isParentOf(*i, previous); ++i)
			(*i)->setForeground(0, QBrush(Qt::black));
	}
	if (current)
	{
		QTreeWidgetItemIterator i(current);
		for (++i; *i && isParentOf(*i, current); ++i)
			(*i)->setForeground(0, QBrush(Qt::red));
	}
}

void ADCBlocksWin::updateOutputFont(const QFont &f)
{
	setFont(f);
	//?mpView->setFont(f);
}

void ADCBlocksWin::showEvent(QShowEvent *e)
{
	//populate();
	QWidget::showEvent(e);
}

void ADCBlocksWin::populate(ADCStream &ss)
{
	mpView->blockSignals(true);
	mpView->clear();

	QList<QTreeWidgetItem *> stack;//keeps last item on each level

	QString s;
	while (ss.ReadString(s))
	{
		QStringList o(s.split('\t', Qt::KeepEmptyParts));

		QString sId;
		if (!o.isEmpty())
		{
			sId = o.front();
			o.pop_front();
		}
		//std::string z(s.toStdString());

		int l(1);
		for (; !o.isEmpty() && o.front().isEmpty(); l++)
			o.pop_front();
		//s.remove(0, 1);

		s = o.join("?");//bugus separator

		QTreeWidgetItem *lviAfter(nullptr);

		if (l > stack.size())
		{
			Q_ASSERT(l - stack.size() == 1);
		}
		else if (l == stack.size())
		{
			if (!stack.isEmpty())
			{
				lviAfter = stack.back();
				stack.pop_back();
			}
		}
		else
		{
			while (l <= stack.size())
			{
				lviAfter = stack.back();
				stack.pop_back();
			}
		}

		QString q;
		if (lviAfter && lviAfter->childCount())
			q = lviAfter->child(0)->text(0);

		QTreeWidgetItem *lvi;
		if (stack.isEmpty())
		{
			lvi = new QTreeWidgetItem(mpView, lviAfter);
			//lvi->setIcon(0, QIcon(":project.png"));
		}
		else
		{
			lvi = new QTreeWidgetItem(stack.back(), lviAfter);
			/*if (s.endsWith("/"))
			{
			lvi->setExpanded(true);
			lvi->setIcon(0, QIcon(":std_folder_opened_16.png"));
			}
			else
			lvi->setIcon(0, QIcon(":file_cpp.16.png"));*/
		}

		lvi->setText(0, s);
#if(BLOCK_NUMS)
		lvi->setText(1, sId.startsWith('-') ? "-" : sId);
#endif
		lvi->setExpanded(true);

		stack.push_back(lvi);
	}

	for (int i(0); i < 2; i++)
		mpView->resizeColumnToContents(i);

	mpView->blockSignals(false);
}

void ADCBlocksWin::applySubject()
{
	applySubject(getToolbarText());
	mpView->setFocus();
}

void ADCBlocksWin::applySubject(QString s)
{
	ADCStream ss;
	emit signalDumpBlocks(s, ss);
	QString subjectName(ss.ReadString());
	QString filePath(ss.ReadString());

	populate(ss);

	setToolbarEnabled(!subjectName.isEmpty());
	setToolbarData(subjectName, filePath, true);//apply
}

void ADCBlocksWin::slotSyncPanesResponce2(int iLine, bool bForce)
{
	(void)iLine;
	if (mpView->isVisible())
	{
		if (!mpView->hasFocus())//not from itself
		{
			if (bForce)
				applySubject(getToolbarText());
			//emit signalSyncPanesResponce4(iLine, bForce);
		}
	}
}

void ADCBlocksWin::slotReset()
{
	mpView->clear();
}

void ADCBlocksWin::slotCurInfoChanged(const ADCCurInfo &ci)
{
	if (!isVisible())
		return;
	//populate();

	ADCModelInfo data(ci.scope, ci.scopePath);
	if (!ci.obj.isEmpty())
		data = ADCModelInfo(ci.obj, ci.objPath);

	if (!data.dispText().isEmpty())
		updateToolbarState(data.dispText());
}

static QTreeWidgetItem *nextSibling(QTreeWidgetItem *current)
{
	QTreeWidgetItem *parent(current->parent());
	if (parent)
		return parent->child(parent->indexOfChild(current) + 1);
	QTreeWidget *treeWidget = current->treeWidget();
	return treeWidget->topLevelItem(treeWidget->indexOfTopLevelItem(current) + 1);
}

void ADCBlocksWin::slotCollapseChildren()
{
	QTreeWidgetItem *p(mpView->currentItem());
	if (p)
	{
		//locate an item to stop at
		QTreeWidgetItem *pStop(nullptr);
		for (QTreeWidgetItem *q(p); q && !pStop; q = q->parent())
		{
			pStop = nextSibling(q);
		}

		QTreeWidgetItemIterator i(p);
		i++;//except the root
		for (; *i && *i != pStop; ++i)
		{
			if ((*i)->childCount() > 0)
				mpView->collapseItem(*i);
		}
	}
}

void ADCBlocksWin::slotExpandChildren()
{
	if (mpView->currentItem())
	{
		mpView->blockSignals(true);
#if QT_VERSION >= 0x050D00//05.13.00
		mpView->expandRecursively(mpView->currentIndex());
#endif
		mpView->blockSignals(false);
		/*QTreeWidgetItemIterator it(mpView->currentItem());
		for (; (*it); ++it)
		{
			if ((*it)->childCount() > 0)
				mpView->expandItem(*it);
		}*/
	}
}

void ADCBlocksWin::slotCustomContextMenuRequested(const QPoint& pos)
{
	//QTreeWidgetItem* p(mpView->itemAt(pos));
	QMenu menu;
	menu.addAction(QIcon(":expand.png"), tr("Expand Children"), this, SLOT(slotExpandChildren()));
	menu.addAction(QIcon(":collapse.png"), tr("Collapse Children"), this, SLOT(slotCollapseChildren()));
	menu.exec(mpView->viewport()->mapToGlobal(pos));
}
