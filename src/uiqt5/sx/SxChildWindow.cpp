#include <QtCore/QObjectList>
#include <QtCore/QFile>
#include <QtGui/QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QPushButton>


#include "SxDocument.h"
#include "SxChildWindow.h"

SeditexChildWin::SeditexChildWin(QWidget* pParent)
	: QMdiSubWindow(pParent)
{
	resize(640, 480);
}

SeditexChildWin::~SeditexChildWin()
{
	if (getWidget())
	{
		DocumentObject * pDoc = dynamic_cast<DocumentObject *>(getWidget());
		if (pDoc && pDoc->isPermanent())
		{
			getWidget()->setParent(window());//((SeditexApp *)qApp)->mainWidget());//, QPoint( 0, 0 ) );
		}
	}
}

void SeditexChildWin::setWidget(QWidget * pWidget)
{
	if (!pWidget)
		return;

	if (pWidget->parentWidget() != this)
	{
		pWidget->setParent(this);
		pWidget->move(QPoint(0, 0));
		pWidget->show();
	}

	setFocusProxy(pWidget);
	QMdiSubWindow::setWidget(pWidget);
}

int SeditexChildWin::canClose()
{
	if (getWidget())
		if (getWidget()->parentWidget() == this)
		{
			DocumentObject * pDoc = dynamic_cast<DocumentObject *>(getWidget());
			if (pDoc)
			{
				if (!pDoc->canClose())
					return 0;
			}
		}

	return 1;
}

void SeditexChildWin::closeEvent(QCloseEvent * pevent)
{
	QMdiSubWindow::closeEvent(pevent);
	int n = canClose();
	if (n == 1)
	{
		pevent->accept();
		return;
	}

	pevent->ignore();
}

void SeditexChildWin::mouseReleaseEvent(QMouseEvent * e)
{
	QMdiSubWindow::mouseReleaseEvent(e);
}

void SeditexChildWin::contextMenuEvent(QContextMenuEvent * e)
{
	QMdiSubWindow::contextMenuEvent(e);
}

bool SeditexChildWin::eventFilter(QObject * o, QEvent * e)
{
	if (e->type() == QEvent::ContextMenu)
	{
		QContextMenuEvent * ce = (QContextMenuEvent*)e;
		if (QString(o->objectName()) == "qt_ws_titlebar")
		{
			QWidget * w = getWidget();
			emit signalHandlePageMenu(w, ce->globalPos());
			return true;
		}
	}

	return false;
}






