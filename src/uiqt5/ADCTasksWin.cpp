#include <QLayout>

#include "ADCUtils.h"
#include "ADCTasksWin.h"
#include "sx/SxDocument.h"

ADCTasksWin::ADCTasksWin(QWidget * parent, const char *name)
	: QWidget(parent),
	mbDirty(false)
{
	setObjectName(name);

	mpListView = new QListView(this);
	mpListView->setUniformItemSizes(true);//otherwise it is gonna choke on large amount of data!!!
	//connect(mpListView, SIGNAL(itemClicked(QTreeWidgetItem *, int)), SLOT(slotItemClicked(QTreeWidgetItem *, int)));
	//connect(mpListView, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(slotItemDoubleClicked(QTreeWidgetItem *, int)));
	connect(mpListView, SIGNAL(clicked(const QModelIndex &)), SLOT(slotItemClicked(const QModelIndex &)));
	connect(mpListView, SIGNAL(activated(const QModelIndex &)), SLOT(slotItemActivated(const QModelIndex &)));

	QVBoxLayout * vbox(new QVBoxLayout(this));
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addWidget(mpListView);
}

void ADCTasksWin::slotItemActivated(const QModelIndex &index)
{
	ADCTasksViewModel *pModel(static_cast<ADCTasksViewModel *>(mpListView->model()));
	QString sModule(pModel->module(index));
	QString sPos(pModel->viewPos(index));
	emit signalItemActivated(sModule + sPos);

	/*QString s(mpListView->model()->data(index, Qt::DisplayRole).toString());

	QStringList l(s.split(" ", QString::SkipEmptyParts));
	for (int i(0); i < l.size(); i++)
	{
		//if (!l[i].startsWith("-@"))
		//continue;
		if (l[i] == "-@")
		{
			if (++i < l.length())
				emit signalLocationSelected(sModule + "@" + l[i]);
			break;
		}
	}*/
}

void ADCTasksWin::slotItemClicked(const QModelIndex &index)
{
	ADCTasksViewModel *pModel(static_cast<ADCTasksViewModel *>(mpListView->model()));
	QString sModule(pModel->module(index));
	QString sPos(pModel->viewPos(index));
	emit signalItemClicked(sModule + sPos);
}

/*void ADCTasksWin::slotItemClicked(QTreeWidgetItem *lvi, int)
{
	if (lvi)
	{
		QStringList l(lvi->text(0).split(" ", QString::SkipEmptyParts));
		for (int i(0); i < l.size(); i++)
		{
			//if (!l[i].startsWith("-@"))
				//continue;
			if (l[i] == "-@")
			{
				if (++i < l.length())
					emit signalItemClicked(ModuleName + "@" + l[i]);
				break;
			}
		}
		//if (l.size() > 1 && l[1].startsWith("~"))
			//emit signalLocationSelected(l[1]);
	}
}

void ADCTasksWin::slotItemDoubleClicked(QTreeWidgetItem *lvi, int)
{
	if (lvi)
	{
		QStringList l(lvi->text(0).split(" ", QString::SkipEmptyParts));
		for (int i(0); i < l.size(); i++)
		{
			//if (!l[i].startsWith("-@"))
				//continue;
			if (l[i] == "-@")
			{
				if (++i < l.length())
					emit signalLocationSelected(mBinaryName + "@" + l[i]);
				break;
			}
		}
		//if (l.size() > 1 && l[1].startsWith("~"))
			//emit signalLocationSelected(l[1]);
	}
}*/


void ADCTasksWin::showEvent(QShowEvent *e)
{
	if (mbDirty)
		slotUpdate();

	QWidget::showEvent(e);
}


void ADCTasksView::paintEvent(QPaintEvent *e)
{
	ModelLocker lock(this);
	QListView::paintEvent(e);
}


void ADCTasksWin::setModel(adcui::ITasksViewModel *pIModel)
{
	mpListView->setModel(new ADCTasksViewModel(this, pIModel));
}

void ADCTasksWin::slotUpdate()
{
	if (!isVisible())
	{
		mbDirty = true;
		return;
	}

	mpListView->reset();
	mpListView->update();

#if(0)
	
	ADCStream ss;
		//CORE.GetToDoList(ss);
	emit signalRequestData(ss);

	mpListView->clear();
	QString s;
	if (ss.ReadString(s))//first goes a binary name
	{
		mBinaryName = s;
		CHECK_QSTRING(s, z);
		for (QTreeWidgetItem *lvi(nullptr); ss.ReadString(s);)
		{
			lvi = new QTreeWidgetItem(mpListView, lvi);
			lvi->setText(0, s.section('\t', 0, 0));
			//lvi->setText(1, s.section('\t', 1));
		}
	}
#endif

	mbDirty = false;
}

/*void ADCTasksWin::slotPushFront(QString a, QString b)
{
	QListViewItem *lvi = new QListViewItem(mpListView);
	lvi->setText(0, a);
	lvi->setText(1, b);
}

void ADCTasksWin::slotPushBack(QString a, QString b)
{
	QListViewItem *lvi = new QListViewItem(mpListView, mpListView->lastItem());
	lvi->setText(0, a);
	lvi->setText(1, b);
}

void ADCTasksWin::slotPopFront()
{
	QListViewItem *lvi = mpListView->firstChild();
	mpListView->takeItem(lvi);
	delete lvi;
}*/




