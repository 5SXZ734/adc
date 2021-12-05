#include <QtCore/QFile>
#include <QtGui/QDragEnterEvent>
#include <QLabel>
#include <QComboBox>
#include <QLayout>
#include <QToolTip>
#include <QToolButton>
#include <QApplication>
#include <QMenu>

#include "SxCommandWin.h"
#include "SxCommandLine.h"


SeditexCommListBox::SeditexCommListBox(QWidget *parent, const char *name)
	:QListWidget(parent)
{
	setObjectName(name);

	setFrameShape(QListWidget::StyledPanel);
	setFrameShadow(QListWidget::Sunken);
	//setAcceptDrops(true);

	connect(this, SIGNAL(contextMenuRequested(QListBoxItem *, const QPoint &)),
		SLOT(slotContextMenuRequested(QListBoxItem *, const QPoint &)) );
}

SeditexCommListBox::~SeditexCommListBox()
{
}

void SeditexCommListBox::dragEnterEvent(QDragEnterEvent * /*e*/)
{
	/*?    if (Q3UriDrag::canDecode(e))
	e->acceptAction();
	else
	e->ignore();*/
}

void SeditexCommListBox::dragMoveEvent(QDragMoveEvent * /*e*/)
{
	/*?    if (Q3UriDrag::canDecode(e))
	e->acceptAction();
	else
	e->ignore();*/
}

void SeditexCommListBox::dropEvent(QDropEvent * e)
{
	qApp->sendEvent(window(),  e);//? qApp->mainWidget(), e );
}

void SeditexCommListBox::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Delete)
		removeItemWidget(currentItem());

	QListWidget::keyPressEvent(e);
}

void SeditexCommListBox::slotContextMenuRequested(QListWidgetItem * /*item*/, const QPoint &pt)
{
	QMenu menu(this);

	QAction *pAction(new QAction(tr("Clear"), this));
	connect(pAction, SIGNAL(activate()), SLOT(clear()));
	menu.addAction(pAction);
	emit signalCreatePopupMenuRequested(&menu);

	menu.exec(pt);
}

void SeditexCommListBox::addCommand(const QString & /*rcommand*/)
{
/*?	if (rcommand.isEmpty())
		return;

	// no specific functionality required to add/manage command in pCommCbx

	int nindex    = -1;
	bool bremoved = false;
	QList<QListWidgetItem *> items(findItems(rcommand, Qt::MatchExactly));

	// check that item has not already been added
	if(!items.isEmpty())
	{
		// remove item if it is not last item in list
		nindex = index(pitem);
		if(nindex < (int)count() - 1)
		{
			removeItemWidget(nindex);
			bremoved = true;
		}
	}

	// only display last 20 commands
	if (count() == 100)
		removeItem(0);

	// add command to bottom of list
	if(!pitem || bremoved)
	{
		insertItem( rcommand );

		int nlast_index = count() - 1;
		setBottomItem(nlast_index);
		clearSelection();
	}*/
}


/////////////////////////////////////////////////////////////
// SeditexCommandHistoryWin


SeditexCommandHistoryWin::SeditexCommandHistoryWin(QWidget *parent, const char *name)
	: SeditexCommandWin(parent, name, false)
{
	mpListBox = new SeditexCommListBox(this, "SEDITEXCOMMLISTBOX");
	connect(mpListBox, SIGNAL(selected(const QString &)), SLOT(slotCommand(const QString &)));
	connect(mpListBox, SIGNAL(highlighted(const QString &)), SLOT(slotHighlighted(const QString &)));
	connect(mpListBox, SIGNAL(signalCreatePopupMenuRequested(QMenu *)), SLOT(slotCreatePopupMenuRequested(QMenu *)));

	addWidget(mpListBox);
	EnableCommandLine(true);
	//setClientWidget(mpListBox);

}

SeditexCommandHistoryWin::~SeditexCommandHistoryWin()
{
}

void SeditexCommandHistoryWin::slotCreatePopupMenuRequested(QMenu *popup)
{
	OnPopupMenu(popup);
}

void SeditexCommandHistoryWin::updateOutputFont(const QFont &f)
{
	OnFontChange(f);
}

void SeditexCommandHistoryWin::slotClear()
{
	mpListBox->clear();
}

void SeditexCommandHistoryWin::slotHighlighted(const QString &s)
{
	slotDisplayCommand(s);
}

void SeditexCommandHistoryWin::slotLogCommand(const QString &s)
{
	mpListBox->addCommand(s);
}



