#include <QtCore/QDir>
#include <QtCore/QChildEvent>
#include <QAction>
#include <QMenu>

#include "SxDocument.h"
#include "SxViewMgrMDI.h"
#include "SxChildWindow.h"
#include "SxSignalMultiplexer.h"

SxViewMgrMDI::SxViewMgrMDI(QWidget * parent)//SmspiceMainWin * parent )
	: QMdiArea(parent)//, "VIEWMGRMDI" )
{
	connect(this, SIGNAL(windowActivated(QWidget *)),
		SLOT(slotCurrentChanged(QWidget *)));

	mpSignalMultiplexer = new SignalMultiplexer;

	mpSignalMultiplexer->connect(SIGNAL(signalPageClosed()),
		this, SLOT(slotClosePage()));

	mpSignalMultiplexer->connect(SIGNAL(signalClosePage()),
		this, SLOT(slotClosePage()));

	mpSignalMultiplexer->connect(SIGNAL(signalFileSaved(QWidget *, const QString&)),
		this, SLOT(slotFileSaved(QWidget *, const QString&)));

	mpSignalMultiplexer->connect(SIGNAL(signalHandlePageMenu(QWidget *, const QPoint &)),
		this, SIGNAL(signalHandlePageMenu(QWidget *, const QPoint &)));

	mpCascadeAction = new QAction(QIcon(":std_cascade_windows_24.png"), tr("Cascade"), this);
	mpCascadeAction->setStatusTip(tr("Arrange windows so they overlap"));
	connect(mpCascadeAction, SIGNAL(triggered()), SLOT(cascadeSubWindows()));

	mpTileAction = new QAction(QIcon(":std_tile_24.png"), tr("Tile"), this);
	mpTileAction->setStatusTip(tr("Arrange windows as non-overlapping tiles"));
	connect(mpTileAction, SIGNAL(triggered()), SLOT(tileSubWindows()));

	mpTileHorzAction = new QAction(QIcon(":std_tile_horizontal_24.png"), tr("Tile Horizontally"), this);
	mpTileHorzAction->setStatusTip(tr("Tile windows in horizontal, non-overlapping pattern"));
	connect(mpTileHorzAction, SIGNAL(triggered()), SLOT(slotTileHorizontal()));

	languageChange();
	setVisible(true);
}

SxViewMgrMDI::~SxViewMgrMDI()
{
	delete mpSignalMultiplexer;
}

void SxViewMgrMDI::languageChange()
{
}

void SxViewMgrMDI::restoreWinLayout(const SxWinLayout &a)
{
	for (int i = 0; i < (int)subWindowList().count(); i++)
	{
		SeditexChildWin *pChildWin = dynamic_cast<SeditexChildWin *>(subWindowList().at(i));
		if (pChildWin && pChildWin->getWidget())
		{
			DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pChildWin->getWidget());
			if (pDoc)
			{
				QString key(pDoc->extra());
				if (key.isEmpty())
				{
					key = QDir::toNativeSeparators(pDoc->path());
					if (!key.isEmpty())
						key.prepend("*");
				}
				if (!key.isEmpty())
				{
					int st;
					QRect r;
					if (a.Find(key, r, st))
					{
						if (!r.isEmpty())
						{
							if (r.left() < 0)
								r.moveLeft(0);
							if (r.top() < 0)
								r.moveTop(0);
							pChildWin->parentWidget()->setGeometry(r);
						}
						if (st == 1)
							pChildWin->showMaximized();
						else if (st == 2)
							pChildWin->showMinimized();
					}
					else
						pChildWin->showNormal();
				}
			}
		}
	}
}

void SxViewMgrMDI::getWinLayout(SxWinLayout &wl, bool bSaveEditDocs) const
{
	for (int i = 0; i < (int)subWindowList().count(); i++)
	{
		SeditexChildWin *pChildWin = dynamic_cast<SeditexChildWin *>(subWindowList().at(i));
		if (pChildWin && pChildWin->getWidget())
		{
			DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pChildWin->getWidget());
			if (pDoc)
			{
				QString key(pDoc->extra());
				if (key.isEmpty())
				{
					key = QDir::toNativeSeparators(pDoc->path());
					if (!key.isEmpty() && bSaveEditDocs)
						key.prepend("*");
				}
				if (!key.isEmpty())
				{
					QRect r(pChildWin->parentWidget()->geometry());
					int st(0);
					if (pChildWin->isMaximized())
						st = 1;
					else if (pChildWin->isMinimized())
						st = 2;
					wl.Add(key, r, st);
				}
			}
		}
	}
}

void SxViewMgrMDI::take(QList<QWidget *> &l)
{
	for (int i = 0; i < subWindowList().count(); i++)
	{
		SeditexChildWin *pChildWin = dynamic_cast<SeditexChildWin *>(subWindowList().at(i));
		if (pChildWin)
		{
			QWidget *w = pChildWin->getWidget();
			//if (plst)
			{
				w->setParent(parentWidget());
				w->move(QPoint(0, 0));
				l.push_back(w);
			}
			//else
			//	delete w;
		}
	}
}

void SxViewMgrMDI::release()
{
	closeAllSubWindows();

	delete this;
}

static QString makeTabTitle(const QString &path, const QString &extra)
{
	QString title = extra;
	if (!path.isEmpty())
	{
		if (!extra.isEmpty())
		{
			QFileInfo fi(path);
			if (!title.isEmpty())
				title = fi.fileName() + QString(" - ") + title;
			else
				title = fi.fileName();
		}
		else
		{
			title = path;
		}
	}

	return title;
}

bool SxViewMgrMDI::addDoc(QWidget * pWidget)
{
	QString title = ":-(";
	DocumentObject * pDoc = dynamic_cast<DocumentObject *>(pWidget);
	if (pDoc != nullptr)
	{
		QStringList lst = pDoc->mstrID.split('\n', QString::KeepEmptyParts);
		title = makeTabTitle(lst[0], lst[1]);
	}

	SeditexChildWin * w = nullptr;

	w = new SeditexChildWin(this);
	w->setWidget(pWidget);

	connect(w, SIGNAL(signalHandlePageMenu(QWidget *, const QPoint&)),
		this, SIGNAL(signalHandlePageMenu(QWidget *, const QPoint&)));

	w->setWindowTitle(title);
	if (pDoc && !pDoc->mstrIcon.isEmpty())
		w->setWindowIcon(QIcon(pDoc->mstrIcon));
	w->showNormal();
	pWidget->show();
	return true;
}

int SxViewMgrMDI::removeDoc(QWidget *pWin)
{
	SeditexChildWin * pChild = dynamic_cast<SeditexChildWin *>(pWin->parent());
	if (!pChild)
		return 0;

	pChild->close();//?true );

	return 1;
}

QWidget *SxViewMgrMDI::openView(QWidget *pWin, bool bActivate)
{
	DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pWin);
	if (!pDoc)
		return nullptr;

	SeditexChildWin *w = dynamic_cast<SeditexChildWin *>(pWin->parent());
	if (!w)
	{
		w = new SeditexChildWin(this);
		w->setWidget(pWin);
		connect(w, SIGNAL(signalHandlePageMenu(QWidget *, const QPoint&)),
			SIGNAL(signalHandlePageMenu(QWidget *, const QPoint&)));
		//		connect( pWin, SIGNAL(signalHandlePageMenu(QWidget *, const QPoint&)), 
		//			SIGNAL(signalHandlePageMenu(QWidget *, const QPoint&)) );
	}

	QStringList lst = pDoc->mstrID.split('\n', QString::KeepEmptyParts);
	QString path = lst[0];
	QString title = lst[1];

	if (!path.isEmpty())
	{
		QFileInfo fi(path);
		if (!title.isEmpty())
			title = fi.fileName() + QString(" - ") + title;
		else
			title = fi.fileName();
	}

	w->setWindowTitle(title);

	if (!pDoc->mstrIcon.isNull())
		w->setWindowIcon(QIcon(pDoc->mstrIcon));

	// show the very first window in maximized mode
//	if ( subWindowList().isEmpty() )
//		w->showMaximized();
//	else
	w->show();

	if (bActivate)
	{
		w->activateWindow();
		w->setFocus();
	}

	return pWin;
}

QWidget * SxViewMgrMDI::createView(
	const char * cl_name,
	const QString &_path,
	const QString &_title)
{
	QString path(_path);
	QString title(_title);

	SeditexChildWin *w = new SeditexChildWin(this);
	DocumentObject *pDoc = DocumentObject::createDoc(w, cl_name, path, title);
	QWidget *pWin = dynamic_cast<QWidget *>(pDoc);
	w->setWidget(pWin);

	connect(w, SIGNAL(signalHandlePageMenu(QWidget *, const QPoint&)),
		SIGNAL(signalHandlePageMenu(QWidget *, const QPoint&)));

	return pWin;
}

QWidget * SxViewMgrMDI::getActiveView()
{
	SeditexChildWin * pChild = dynamic_cast<SeditexChildWin *>(currentSubWindow());
	if (!pChild)
		return nullptr;

	return pChild->getWidget();
}

bool SxViewMgrMDI::isEmpty()
{
	return subWindowList().count() == 0;
}

bool SxViewMgrMDI::checkModifiedFiles()
{
	QList<QMdiSubWindow *> windows = subWindowList();
	for (int i = 0; i < windows.count(); ++i)
	{
		SeditexChildWin * pChildWin = dynamic_cast<SeditexChildWin *>(windows.at(i));
		DocumentObject * pDoc = dynamic_cast<DocumentObject *>(pChildWin->getWidget());
		if (pDoc == nullptr)
			continue;
		if (!pDoc->canClose())
			return false;
	}

	return true;
}

/////////////////////

void SxViewMgrMDI::slotClosePage()
{
	SeditexChildWin * pChild = dynamic_cast<SeditexChildWin *>(currentSubWindow());
	if (pChild == nullptr)
		return;

	DocumentObject * pDoc = dynamic_cast<DocumentObject *>(pChild->getWidget());
	if (pDoc != nullptr)
	{
		if (!pDoc->canClose())
			return;
	}

	pChild->close();

	//emit windowActivated( pChild->getWidget() );
}

void SxViewMgrMDI::slotFileSaved(QWidget *pWin, const QString &path)
{
	DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pWin);
	if (pDoc)
		pDoc->mstrID = path + QString("\n");

	SeditexChildWin *pChildWin = dynamic_cast<SeditexChildWin *>(pWin->parent());
	if (pChildWin)
		pChildWin->setWindowTitle(path);
}

void SxViewMgrMDI::slotCurrentChanged(QWidget * pWidget)
{
	QWidget * pView = nullptr;
	SeditexChildWin * pChildWin = dynamic_cast<SeditexChildWin *>(pWidget);
	if (pChildWin != nullptr)
		pView = pChildWin->getWidget();

	mpSignalMultiplexer->setCurrentObject(pView);
	//	DocumentObject * pDoc = dynamic_cast<DocumentObject *>( pWidget );
	emit enableClose(pWidget != nullptr);
	emit signalCurrentChanged(pView);
}

void SxViewMgrMDI::fillWindowsMenu(QMenu *pMenu)
{
	pMenu->clear();

	pMenu->addAction(mpCascadeAction);
	pMenu->addAction(mpTileAction);
	pMenu->addAction(mpTileHorzAction);

	mpCascadeAction->setEnabled(!subWindowList().isEmpty());
	mpTileAction->setEnabled(!subWindowList().isEmpty());
	mpTileHorzAction->setEnabled(!subWindowList().isEmpty());

	for (int i(0); i < subWindowList().count(); ++i)
	{
		if (i == 0)
			pMenu->addSeparator();

		QAction *pAction(new QAction(subWindowList().at(i)->windowTitle(), pMenu));
		pAction->setCheckable(true);
		pAction->setChecked(currentSubWindow() == subWindowList().at(i));
		connect(pAction, SIGNAL(activated()), SLOT(slotWindowsMenuActivated(int)));//?
		pMenu->addAction(pAction);
		//?pMenu->setItemParameter( id, i );
	}
}

void SxViewMgrMDI::slotTileHorizontal()
{
	// primitive horizontal tiling
	if (!subWindowList().count())
		return;

	int heightForEach = height() / subWindowList().count();
	int y = 0;
	for (int i = 0; i < subWindowList().count(); ++i)
	{
		QWidget *window = subWindowList().at(i);
		//?		if ( window->testWState( WState_Maximized ) ) 
		{
			// prevent flicker
			window->hide();
			window->showNormal();
		}

		int preferredHeight = window->minimumHeight() + window->parentWidget()->baseSize().height();
		int actHeight = qMax(heightForEach, preferredHeight);

		window->parentWidget()->setGeometry(0, y, width(), actHeight);
		y += actHeight;
	}
}

void SxViewMgrMDI::slotWindowsMenuActivated(int id)
{
	QWidget * w = subWindowList().at(id);
	if (w)
	{
		//w->show();
		w->activateWindow();
		//activateNextWindow();
		w->setFocus();
	}
}

int SxViewMgrMDI::count()
{
	return subWindowList().count();
}

QWidget *SxViewMgrMDI::getView(int index)
{
	SeditexChildWin * pChildWin = dynamic_cast<SeditexChildWin *>(subWindowList().at(index));
	return pChildWin->getWidget();
}

QWidget *SxViewMgrMDI::getViewById(int docId)
{
	QList<QMdiSubWindow *> windows = subWindowList();
	for (int i = 0; i < windows.count(); ++i)
	{
		SeditexChildWin *pChildWin = dynamic_cast<SeditexChildWin *>(windows.at(i));
		if (pChildWin)
		{
			DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pChildWin->getWidget());
			if (pDoc && pDoc->mID == docId)
				return pChildWin->getWidget();
		}
	}
	return nullptr;
}

bool SxViewMgrMDI::isDocOpened(QWidget * pWin)
{
	QList<QMdiSubWindow *> windows = subWindowList();
	for (int i = 0; i < windows.count(); ++i)
	{
		SeditexChildWin *pChildWin = dynamic_cast<SeditexChildWin *>(windows.at(i));
		if (pChildWin)
		{
			QWidget *pWin2(pChildWin->getWidget());
			if (pWin2 == pWin)
				return true;
		}
	}
	return false;
}

void SxViewMgrMDI::updateIcon(QWidget *pWin, const char *pszIcon)
{
	DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pWin);
	if (pDoc)
	{
		pDoc->mstrIcon = pszIcon ? pszIcon : "";
		QList<QMdiSubWindow *> windows = subWindowList();
		for (int i = 0; i < windows.count(); ++i)
		{
			SeditexChildWin *pChildWin = dynamic_cast<SeditexChildWin *>(windows.at(i));
			if (pChildWin && pChildWin->getWidget() == pWin)
			{
				pChildWin->setWindowIcon(QIcon(pDoc->mstrIcon));
				return;
			}
		}
	}
}

int SxViewMgrMDI::closeDoc(class QWidget * pWin)
{
	QList<QMdiSubWindow *> windows = subWindowList();
	for (int i = 0; i < windows.count(); ++i)
	{
		SeditexChildWin * pChildWin = dynamic_cast<SeditexChildWin *>(windows.at(i));
		if (pChildWin && pChildWin->getWidget() == pWin)
		{
			DocumentObject * pDoc = dynamic_cast<DocumentObject *>(pWin);
			if (pDoc)
			{
				if (!pDoc->canClose())
					return 0;
			}

			pChildWin->close();
			return 1;
		}
	}

	return 0;
}

void SxViewMgrMDI::childEvent(QChildEvent *e)
{
	QMdiArea::childEvent(e);

	if (e->added())
	{
		QWidget * w = dynamic_cast<QWidget *>(e->child());
		if (w)
		{
			QWidget * p = dynamic_cast<QWidget *>(w->parent());
			if (p && QString(p->objectName()) == QString("qt_workspacechild"))
			{
				QWidget *t(p->findChild<QWidget *>("qt_ws_titlebar"));//, 0, false));
				if (t)
				{
					t->installEventFilter(w);
				}
			}
		}
	}
}

