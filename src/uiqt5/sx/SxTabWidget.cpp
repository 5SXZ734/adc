#include <QtGui/QMouseEvent>
#include "SxTabWidget.h"
#include "SxDockBar.h"

//////////////////////////
// SxTabBar

SxTabBar::SxTabBar(QWidget * parent, const char * /*name*/)
: QTabBar( parent)//, name )
{
}

/*QTab *SxTabBar::selectTab(const QPoint &p) const
{
	return QTabBar::selectTab( p );
}*/

void SxTabBar::mousePressEvent(QMouseEvent *e)
{
	QTabBar::mousePressEvent(e);
	if (!e->isAccepted())
	{
		if (e->button() != Qt::RightButton) 
		{
			e->ignore();
			return;
		}

		int pos = tabAt(e->pos());
		if (pos != -1 && isTabEnabled(pos))
		{
			setCurrentIndex(pos);

			SxTabWidget *pTabWidget = dynamic_cast<SxTabWidget *>(parent());				
			pTabWidget->handleRightButton(mapToGlobal(e->pos()));
		}
	}
}


///////////////////////////////////////
// SxTabWidget

SxTabWidget::SxTabWidget( QWidget *parent, const char * /*name*/ )
	: QTabWidget(parent)//, name?name:"SEDITEXTABWIDGET")
{
	setTabBar( new SxTabBar( this, "SXTABBAR" ) );
}

void SxTabWidget::handleRightButton(const QPoint& pt)
{
	QWidget * pPage = currentWidget();
	if (!pPage)
		return;

	emit signalHandlePageMenu(pPage, pt);
}


