#include <QTabWidget>
#include <QToolButton>
#include <QAction>

#include "SxDocument.h"
#include "SxDockBar.h"

//////////////////////////
// SxTabWidgetEx

SxTabWidgetEx::SxTabWidgetEx(QWidget *parent, const char *name)
: SxTabWidget(parent, name)
{
    setTabPosition(QTabWidget::South);
    //setTabShape(QTabWidget::Triangular);

    mpCloseTabBtn = new QToolButton(this);
    mpCloseTabBtn->setIcon(QIcon("std_close_16.png"));
    mpCloseTabBtn->setAutoRaise(true);
    mpCloseTabBtn->setMaximumSize(16, 16);
	connect(mpCloseTabBtn, SIGNAL(released()), SLOT(slotClosePage()));
	mpCloseTabBtn->hide();

	connect(this, SIGNAL(currentChanged(int)), 
		SLOT(slotTabChanged(int)) );

	setCornerWidget(mpCloseTabBtn, Qt::TopRightCorner);
}

void SxTabWidgetEx::slotTabChanged(int index)
{
	if (index >= 0)
		widget(index)->setFocus();
}

void SxTabWidgetEx::slotClosePage()
{
	hideTab(currentWidget());
}

int SxTabWidgetEx::showTab(QWidget *pTab, int atIndex)
{
	int iRet(0);
	DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pTab);
	if (pDoc)
	{
		iRet = -1;
		bool bEmpty(count() == 0);

		int index = indexOf(pTab);
		if (index < 0)
		{
			iRet = 1;
			QStringList lst = pDoc->mstrID.split('\n', QString::KeepEmptyParts);
			index = insertTab(atIndex, pTab, QIcon( QPixmap(pDoc->mstrIcon)), lst[1]);
		}

		setCurrentWidget(pTab);
		//?showPage(pTab);
		//show();

		if (bEmpty && (count() > 0))
		{
			mpCloseTabBtn->show();
			emit signalPageOpened();
		}
	}

	return iRet;
}

int SxTabWidgetEx::hideTab(QWidget *pTab)
{
	if (!pTab)
		return 0;

	int index = indexOf(pTab);
	if (index < 0)
		return -1;//allready

	bool bEmpty(count() == 0);

	pTab->close();
	pTab->hide();

	removeTab(index);

	if (!bEmpty && (count() == 0))
	{
		mpCloseTabBtn->hide();
		emit signalPageClosed();
	}

	return 1;
}

bool SxTabWidgetEx::contains(QWidget *pTab)
{
	if (indexOf(pTab) >= 0)
		return true;
	return false;
}






