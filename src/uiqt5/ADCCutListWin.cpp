#include "ADCCutListWin.h"
#include "ADCUtils.h"
#include <QTreeWidget>
#include <QLayout>
#include <QMenu>
#include <QShortcut>
#include <QHeaderView>

ADCCutListWin::ADCCutListWin(QWidget* parent, const char* name)
	: QWidget(parent)
{
	setObjectName(name);

	mpView = new QTreeWidget(this);
	mpView->setUniformRowHeights(true);
	mpView->setColumnCount(2);

	QStringList l;
	l << tr("Name") << tr("Location");
	mpView->setHeaderLabels(l);
	mpView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	mpView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//mpView->header()->setStretchLastSection(false);

	QVBoxLayout * vbox(new QVBoxLayout(this));
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addWidget(mpView);

	mpView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(mpView, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(slotCustomContextMenuRequested(const QPoint &)));

	QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), mpView);
    connect(shortcut, SIGNAL(activated()), this, SLOT(slotRemoveSelected()));
}

void ADCCutListWin::slotUpdate()
{
	ADCStream ss;
	emit signalDumpCutList(ss);

	//provide for current item recovery
	QStringList lCur;//cur,next,prev
	QTreeWidgetItem* cur(mpView->currentItem());
	if (cur)
	{
		lCur.append(cur->text(0) + "\n" + cur->text(1));
		QTreeWidgetItem* next(mpView->itemBelow(cur));
		if (next)
			lCur.append(next->text(0) + "\n" + next->text(1));
		QTreeWidgetItem* prev(mpView->itemAbove(cur));
		if (prev)
			lCur.append(prev->text(0) + "\n" + prev->text(1));
	}

	//bool wasClean(mpView->topLevelItemCount() == 0);
	mpView->clear();

	QString sModule;
	while (ss.ReadString(sModule) > 0)
	{
		QString sName(ss.ReadString());
		QTreeWidgetItem* item(new QTreeWidgetItem(mpView));
		item->setText(0, sName);
		item->setText(1, sModule);
		//item->setFirstColumnSpanned(true);
	}

	mpView->resizeColumnToContents(0);

	//recover a current item
	while (!lCur.isEmpty())
	{
		for (QTreeWidgetItemIterator it(mpView); *it; ++it)
		{
			QString s((*it)->text(0) + "\n" + (*it)->text(1));
			if (s == lCur.front())
			{
				mpView->setCurrentItem(*it);
				return;
			}
		}
		lCur.pop_front();
	}
}

void ADCCutListWin::updateOutputFont(const QFont &f)
{
	setFont(f);
}

void ADCCutListWin::slotItemClicked(const QModelIndex&)
{
}

void ADCCutListWin::slotItemActivated(const QModelIndex&)
{
}

void ADCCutListWin::slotRemoveSelected()
{
	QTreeWidgetItem* item(mpView->currentItem());
	if (!item)
		return;

	int id(mpView->indexOfTopLevelItem(item));
	if (id >= 0)
		emit signalUncut(id);
}

void ADCCutListWin::slotCustomContextMenuRequested(const QPoint& pos)
{
	//QTreeWidgetItem* p(mpView->itemAt(pos));
	QMenu menu;
	QAction* action(menu.addAction(QIcon(":delete_16.png"), tr("Remove"), this, SLOT(slotRemoveSelected())));// , QKeySequence(Qt::Key_Delete)));
	addAction(action);
	menu.exec(mpView->viewport()->mapToGlobal(pos));
}

void ADCCutListWin::slotClean()
{
	mpView->clear();
}







