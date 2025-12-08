#include <QtCore/QObjectList>
#include <QtCore/QEvent>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QPixmap>
#include <QPushButton>
#include <QLayout>
#include <QMainWindow>



#include "SxDocument.h"
#include "SxDockWindow.h"
#include "SxDockBar.h"

void SxDockWindow::init(DocumentObject *pDoc)
{
	if (pDoc)
	{
		QStringList lst = pDoc->mstrID.split('\n', Qt::KeepEmptyParts);
		QString s = lst[1];
		setWindowTitle(s);
		setWindowIcon(QIcon(pDoc->mstrIcon));
	}
}

void SxDockWindow::closeEvent(QCloseEvent *e)
{
	//emit signalHide();
	QDockWidget::closeEvent(e);
}

void SxDockWindow::contextMenuEvent(QContextMenuEvent *e)
{
	//emit signalCustomContextMenuRequested(e->globalPos());
	QDockWidget::contextMenuEvent(e);
}