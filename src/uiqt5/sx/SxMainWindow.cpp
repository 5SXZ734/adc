#include <assert.h>

#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QList>
#include <QtCore/QObjectList>
#include <QtCore/QMetaObject>
#include <QtGui/QClipboard>
#include <QApplication>
#include <QAction>
#include <QMenuBar>
#include <QtCore/QSignalMapper>
#include <QStyleFactory>
#include <QHBoxLayout>
#include <QToolTip>
#include <QLabel>
#include <QStatusBar>
#include <QTextEdit>
#include <QLayout>
#include <QSplitter>
#include <QLineEdit>
#include <QActionGroup>

#include "SxDocument.h"
#include "SxMainWindow.h"
#include "SxDockWindow.h"
#include "SxViewMgrTabs.h"
#include "SxViewMgrMDI.h"
#include "SxSignalMultiplexer.h"
#include "SxDockBar.h"

#if(0)
#ifdef Q_WS_X11
#include <QtGui/QImage>
#include <QBitmap>
#include <QtGui/QPixmap>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif
#endif

#ifndef QT_DEBUG
static void __qtMsgHandler(QtMsgType msgType, const char * msgText)
{
	Q_UNUSED(msgType)
		Q_UNUSED(msgText)
		// suppress all Qt generated messages (q<Debug|Warning|Fatal>())
		return;
}
#endif


/////////////////////////////////////////////////////////////
// D o c u m e n t L i s t

QList<DocumentObject *>::ConstIterator DocumentMgr::findDoc(QString key) const
{
	//key = QDir::toNativeSeparators(key);

	QList<DocumentObject *>::ConstIterator it;
	for (it = constBegin(); it != constEnd(); it++)
	{
		DocumentObject *pDoc = *it;
		if (pDoc)
		{
			if (pDoc->mstrID == key)
				return it;
		}
	}
	return end();
}

DocumentObject * DocumentMgr::findDoc(const QString &path0, const QString &title) const
{
	QString path(path0);//QDir::toNativeSeparators(path0));
	QString sKey(DOC2KEY(path, title));
	QList<DocumentObject *>::ConstIterator it(findDoc(sKey));
	if (it != end())
		return *it;
	return nullptr;
}

DocumentObject *DocumentMgr::renameDoc(QString keyOld, QString keyNew)
{
	QList<DocumentObject *>::ConstIterator it(findDoc(keyOld));
	if (it == constEnd())
		return nullptr;
	DocumentObject *pDoc(*it);
	pDoc->mstrID = keyNew;
	return pDoc;
}

void DocumentMgr::cleanUp()
{
	while (!isEmpty())
	{
		DocumentObject *pDoc = front();
		delete pDoc;
	}
}


//////////////////////////////////////////////////////////////////////


void SxActionDispatcher::slotPlaceHome()
{
	mpMainWin->slotReplacePage(mpPage);
}

void SxActionDispatcher::slotPlaceOnABar()
{
	mpMainWin->slotPlacePageOnABar(mpPage);
}

void SxActionDispatcher::slotTabDock()
{
	mpMainWin->slotDockPage(mpPage);
}

void SxActionDispatcher::slotTabUndock()
{
	mpMainWin->slotUndockWin(mpPage);
}

void SxActionDispatcher::slotTabClose()
{
	mpMainWin->closePage(mpPage);
}

//////////////////////////////////////////////////////////////////////
#include <QPushButton>

SxMainWindow::SxMainWindow(QWidget * pparent, const char * pczname, Qt::WindowFlags f)
: QMainWindow(pparent, f),
mABarConfig(0),
//mPageHideConfig(0),
mWinDockConfig(0),
mpViewMgr(nullptr)
{
	if (pczname)
		setObjectName(pczname);

/*#ifndef QT_DEBUG
	qInstallMsgHandler(__qtMsgHandler);
#endif*/

#if(1)
	QWidget * pCentralWidget = new QWidget;
	setCentralWidget(pCentralWidget);
	QGridLayout *layout = new QGridLayout();
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	pCentralWidget->setLayout(layout);

	//pCentralWidget->setFrameStyle(/* QFrame::StyledPanel |*/ QFrame::Sunken );
	//pCentralWidget->setMargin( 2 );
	//pCentralWidget->setSpacing( 2 );
	//pCentralWidget->setFrameStyle( QFrame::NoFrame );
#else
	QSplitter *pCentralWidget = new QSplitter(Qt::Vertical, this);
	//QWidget *pCentralWidget = new QWidget(this);
	setCentralWidget(pCentralWidget);
#endif
	
	//pCentralWidget->setBackgroundRole(QPalette::Background);
	//pCentralWidget->setAutoFillBackground(true);

	for (int i = 0; i < STATUSSLOT_MAX; i++)
		mpStatusSlots[i] = nullptr;

	mpDockWinMapper = new QSignalMapper(this);

	mpActions = new QActionGroup(this);//, "mpActions", false);
	//mpActions->setExclusive(false);

	mpNewAction = new QAction(QIcon(":std_new_file_24.png"), tr("New"), this);
	mpNewAction->setToolTip(tr("Create a new document"));
	mpNewAction->setShortcut(tr("Ctrl+N"));
	connect(mpNewAction, SIGNAL(triggered()), SLOT(slotFileNew()));

	mpOpenAction = new QAction(QIcon(":std_open_file_24.png"), tr("Open..."), this);
	mpOpenAction->setToolTip(tr("Open an existing document"));
	mpOpenAction->setShortcut(tr("Ctrl+O"));
	connect(mpOpenAction, SIGNAL(triggered()), SLOT(slotFileOpen()));

	mpExitAction = new QAction(QIcon(":std_exit_24.png"), tr("Exit"), this);
	mpExitAction->setToolTip(tr("Exit application"));
	connect(mpExitAction, SIGNAL(triggered()), SLOT(slotExit()));

	mpCloseAction = new QAction(QIcon(":std_close_24.png"), tr("Close"), this);
	mpCloseAction->setToolTip(tr("Close the opened document"));

	mpTabbedWindowsAction = new QAction(tr("Tabbed Windows"), this);
	mpTabbedWindowsAction->setToolTip(tr("Open documents as tabs"));
	mpTabbedWindowsAction->setCheckable(true);
	connect(mpTabbedWindowsAction, SIGNAL(toggled(bool)), SLOT(slotTabbedWindowsToggled(bool)));

	mpFileMenu = new QMenu(tr("&File"), this);
	mpFileMenu->addAction(mpExitAction);
	mpEditMenu = new QMenu(tr("&Edit"), this);
	mpViewMenu = new QMenu(tr("&View"), this);
	mpViewMenu->addAction(mpTabbedWindowsAction);
	mpWindowMenu = new QMenu(tr("&Window"), this);
	mpHelpMenu = new QMenu(tr("&Help"), this);

	//	mpMainMenuBar = new QMenuBar(this);
	//    mpMainMenuBar->setFrameShape( QMenuBar::MenuBarPanel );
	//  mpMainMenuBar->setFrameShadow( QMenuBar::Raised );
	//    mpMainMenuBar->setLineWidth( 0 );
	//mpMainMenuBar->setMargin( 0 );
	//mpMainMenuBar->setGeometry( QRect( 0, 0, 896, 29 ) );
	menuBar()->addMenu(mpFileMenu);
	menuBar()->addMenu(mpEditMenu);
	menuBar()->addMenu(mpViewMenu);
	mpWindowsMenuAction = menuBar()->addMenu(mpWindowMenu);//, WINDOW_MENU_ID );
	menuBar()->addMenu(mpHelpMenu);

	// - status bar
	mpCopyright = addStatusSlot();
	setStatusSlotText(mpCopyright, productInfoStr("Test", "1.0.0", "Test"));

	//?    QToolTip::setGloballyEnabled(true);
	//setDockOptions(VerticalTabs|AnimatedDocks);//|AllowNestedDocks|AllowTabbedDocks);

	connect(mpWindowMenu, SIGNAL(aboutToShow()),
		SLOT(slotWindowMenuAboutToShow()));
	connect(this, SIGNAL(signalReplacePage(QWidget *)),
		SLOT(slotPlacePageOnABar(QWidget *)));
	connect(this, SIGNAL(signalDockPage(QWidget *)),
		SLOT(slotDockPage(QWidget *)));
}

SxMainWindow::~SxMainWindow()
{
	hide();
	DocMgr().cleanUp();
	if (mpViewMgr)
	{
		mpViewMgr->release();
		mpViewMgr = nullptr;
	}
}

QAction *SxMainWindow::NewAction(const char *pixmap, const QString &/*menuText*/,
	const QString &toolTip, const QString &statusTip,
	const QString &/*accel*/,
	const char *slot)
{
#if(0)
	QAction *pAction = new QAction(this, name);
#else
	QAction *pAction = new QAction(this);// mpActions, name );
#endif
	if (pixmap)
	{
		QString s(":");
		s += pixmap;
		int n(s.indexOf(QChar('%')));
		if (n > 0)
		{
			QString s2(s);
			s.replace(n, 1, "16");
			s2.replace(n, 1, "24");
			QIcon icon(s);
			icon.addFile(s2);
			pAction->setIcon(icon);
		}
		else
			pAction->setIcon(QIcon(s));
	}
	//	if (!menuText.isNull())
	//?	pAction->setMenuText( menuText );
	if (!toolTip.isNull())
		pAction->setToolTip(toolTip);
	if (!statusTip.isNull())
		pAction->setStatusTip(statusTip);
	//	if (!accel.isNull())
	//	pAction->setAccel(accel);
	if (slot)
		connect(pAction, SIGNAL(triggered()), slot);

	return pAction;
}

QAction *SxMainWindow::NewToggleAction(const char *pixmap, const QString &/*menuText*/,
	const QString &toolTip, const QString &statusTip,
	const QString &/*accel*/,
	const char *slot)
{
	QAction *pAction = new QAction(this);// mpActions, name );
	pAction->setCheckable(true);
	if (pixmap)
		pAction->setIcon(QIcon(pixmap));
	//	if (!menuText.isNull())
	//		pAction->setMenuText( menuText );
	if (!toolTip.isNull())
		pAction->setToolTip(toolTip);
	if (!statusTip.isNull())
		pAction->setStatusTip(statusTip);
	//	if (!accel.isNull())
	//	pAction->setAccel(accel);
	if (slot)
		connect(pAction, SIGNAL(toggled(bool)), slot);

	return pAction;
}

void SxMainWindow::slotStatusMsg(const QString &)
{
	//?	statusBar()->message(s);
}

QWidget * SxMainWindow::addStatusSlot(QWidget *pCustomWidget, const char *objName)
{
	QVector<bool> hid(STATUSSLOT_MAX);//after removeWidget(), some(!) widgets become hidden even after re-insertion
	
	for (int i = 0; i < STATUSSLOT_MAX; i++)
	{
		if (mpStatusSlots[i])
			continue;

		//found a slot.
		QStatusBar * pStatusBar = statusBar();
		pStatusBar->blockSignals(true);

		//now we need to remove all widgets from the status bar
		//and re-insert them in a reverse order - so the last 
		//created widget appeared at the left. 
		for (int j = 0; j < STATUSSLOT_MAX; j++)
		{
			if (mpStatusSlots[j])
			{
				hid[j] = mpStatusSlots[j]->isHidden();
				pStatusBar->removeWidget(mpStatusSlots[j]);
			}
		}

		QWidget * pWidget = nullptr;
		if (pCustomWidget)
		{
			/*?QLineEdit * pLineEdit = new QLineEdit(this);
			pLineEdit->setAlignment( Qt::AlignCenter );
			pLineEdit->setReadOnly(true);
			//pLineEdit->setEnabled(false);
			pLineEdit->setFrameStyle(QFrame::NoFrame | QFrame::Sunken);
			pLineEdit->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred ) );
			pLineEdit->setMinimumSize( QSize(10, 10) );
			pCustomWidget = pLineEdit;*/
			pWidget = pCustomWidget;
		}
		else
		{
			QLabel * pLabel = new QLabel(this);
			//pLabel->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred ) );
			pLabel->setAlignment(Qt::AlignCenter);
			pLabel->setTextFormat(Qt::RichText);
			pLabel->setWordWrap(false);
			//pLabel->setMinimumSize( QSize(80, 11) );
			pWidget = pLabel;
		}
		mpStatusSlots[i] = pWidget;
		pWidget->setObjectName(objName);

		//re-add all widgets to status bar
		for (int k = 0; k < STATUSSLOT_MAX; k++)
		{
			int n = STATUSSLOT_MAX - (k + 1);
			QWidget * w = mpStatusSlots[n];
			if (w)
			{
				pStatusBar->addPermanentWidget(w, 0);
				w->setHidden(hid[n]);
			}
		}

		setStatusSlotText(pWidget, QString());
		pStatusBar->blockSignals(false);
		return pWidget;
	}

	return nullptr;
}

void SxMainWindow::closeStatusSlot(QWidget * pWidget)
{
	if (!pWidget)
		return;

	for (int i = 0; i < STATUSSLOT_MAX; i++)
	{
		if (mpStatusSlots[i] != pWidget)
			continue;

		QStatusBar * pStatusBar = statusBar();
		pStatusBar->removeWidget(mpStatusSlots[i]);
		pWidget->deleteLater();
		mpStatusSlots[i] = nullptr;
		break;
	}
}

void SxMainWindow::setStatusSlotText(QWidget * pWidget, const QString& s)
{
	QStatusBar * pStatusBar = statusBar();
	pStatusBar->blockSignals(true);
	if (pWidget)
	{
		pWidget->setVisible(!s.isEmpty());
		QLabel * pLabel = dynamic_cast<QLabel *>(pWidget);
		if (pLabel)
		{
			pLabel->blockSignals(true);
			pLabel->setText(s);
			pLabel->blockSignals(false);
			//pLabel->setMinimumSize( pLabel->sizeHint() );
		}
		else
		{
			QLineEdit * pLineEdit = dynamic_cast<QLineEdit *>(pWidget);
			if (pLineEdit)
			{
				pLineEdit->setText(s);
			}
		}
	}
	pStatusBar->blockSignals(false);
}

void SxMainWindow::setStatusSlotPixmap(QWidget *pWidget, const QPixmap &pix)
{
	QStatusBar *pStatusBar = statusBar();
	pStatusBar->blockSignals(true);
	if (pWidget)
	{
		pWidget->setVisible(!pix.isNull());
		QLabel *pLabel = dynamic_cast<QLabel *>(pWidget);
		if (pLabel)
		{
			pLabel->blockSignals(true);
			pLabel->setPixmap(pix);
			pLabel->blockSignals(false);
		}
	}
	pStatusBar->blockSignals(false);
}

bool SxMainWindow::createSelf()
{
	createViewMgrMDI(centralWidget());
	return true;
}

QString SxMainWindow::productInfoStr(const QString &productName, const QString &versionStr, const QString &companyName)
{
	return QString("<nobr> %1 %2 %3 %4 %5 </nobr>")
		.arg(productName)
		.arg(versionStr)
		.arg(QChar(169))// copyright symbol
		.arg(companyName)
		.arg(QDate::currentDate().year())
		;
}

#ifndef WIN32
/* With enterprise 5 update 4 and above the linux window manager now
displays the small program icon on the window title line. (Like MS
Windows). At the moment most tools do not set this icon and as such most
applications just show a default icon. To set the correct Icon you need
to add the code in the attached message (changing th png for the one for
your app). */

void SxMainWindow::setIcon(const QPixmap &p)
{
#if(0)
	if (!isTopLevel())
		return;  // Bail out if widget is not toplevel

	Window   x11_root_window      = QX11Info::appRootWindow();
	int      x11_screen           = QX11Info::screen();
	Display* x11_default_display  = QX11Info::display();
	Colormap x11_default_colormap = DefaultColormap(QX11Info::display(), x11Screen());

	int required_depth = DefaultDepth(QX11Info::display(), x11Screen());
	Visual* x11_visual = (Visual*)QPaintDevice::x11AppVisual(x11_screen);

	int image_depth = 24 == required_depth ? 32 : required_depth;
	int num_colors  = 8 == required_depth ? 256 : 0;

	// If depths match, just call inherited func and bail out
	if (required_depth == x11Depth())
	{
		QWidget::setIcon(p);
		return;
	}

	if (p.isNull())
		return;

	QPixmap *pIcon = new QPixmap(p);
	if (!pIcon->mask())
		pIcon->setMask(pIcon->createHeuristicMask());

	QImage image(pIcon->convertToImage());
	QImage new_image(image.width(), image.height(), image_depth, num_colors);

	// Copy colors into new reduced image.  Use default,
	// colormap to allocate the colors
	for (int y = 0; y < image.height(); y++)
	{
		for (int x = 0; x < image.width(); x++)
		{
			QRgb color = image.pixel(x, y);
			XColor x11_color = { 0, qRed(color) << 8, qGreen(color) << 8, qBlue(color) << 8, 0, 0 };
			Status status = XAllocColor(x11_default_display, x11_default_colormap, &x11_color);

			if ( status && ((int)x11_color.pixel != -1) )
			{
				// Need host-to-network conversion between client and server
				new_image.setPixel(x, y, x11_color.pixel);
			}
			else
			{
				new_image.setPixel(x, y, 0);
			}
		}
	}

	// Create X11 Pixmap to use as window icon
	// We never free this pixmap.  Oh well!
	Pixmap XPixmap = XCreatePixmap(x11_default_display, x11_root_window,
		new_image.width(), new_image.height(), required_depth);

	if (!XPixmap)
		return;

	// Create image for icon
	XImage* x11_image = XCreateImage(x11_default_display,
		x11_visual,
		required_depth,
		ZPixmap,
		0 /* offset */,
		(char*)new_image.bits(),
		new_image.width(),
		new_image.height(),
		image_depth /* bitmap_pad */,
		0 /* bytes_per_line */);

	if (!x11_image)
		return;

	// Copy image to Pixmap
	XPutImage(x11_default_display,
		XPixmap,
		DefaultGC(x11_default_display, x11_screen),
		x11_image,
		0 /* src_x */, 0 /* src_y */,
		0 /* dst_x */, 0 /* dst_y */,
		new_image.width(), new_image.height());

	// Set WM hint for icon
	XWMHints* wm_hints = XGetWMHints(QX11Info::display(), winId());
	XWMHints temp_hints = {0};
	bool got_hints = wm_hints != 0;
	if (!got_hints)
		wm_hints = &temp_hints;

	wm_hints->icon_pixmap = XPixmap;
	wm_hints->icon_mask = pIcon->mask()->handle();
	wm_hints->flags = IconPixmapHint | IconMaskHint;

	XSetWMHints(x11_default_display, winId(), wm_hints);

	// Clean up
	if (got_hints)
		XFree((char *)wm_hints);
	x11_image->data = 0;
	XDestroyImage(x11_image);

	QEvent e(QEvent::IconChange);
	QApplication::sendEvent(this, &e);
#endif
}
#endif

void SxMainWindow::slotTabPlace()
{
	DocumentObject * pDoc = getActiveDoc();
	QWidget * pWidget = dynamic_cast<QWidget *>(pDoc);
	if (!pWidget)
		return;

	slotPlacePageOnABar(pWidget);
}

void SxMainWindow::slotTabDock()
{
	DocumentObject * pDoc = getActiveDoc();
	QWidget * pWidget = dynamic_cast<QWidget *>(pDoc);
	if (!pWidget)
		return;

	slotDockPage(pWidget);
}

void SxMainWindow::slotCustomContextMenuRequested(const QPoint &pt)
{
	SxDockWindow* pDockWin(dynamic_cast<SxDockWindow *>(sender()));
	if (!pDockWin)
		return;

	QWidget *pPage(pDockWin->widget());

	QMenu* pTabMenu(new QMenu(this));
	const SxActionDispatcher * actnDisp = new SxActionDispatcher(this, pPage);

	bool bDocked = (isPageDocked(pPage) != nullptr);

	QAction * pTabDockAction = new QAction(this);//, "mpTabDockAction" );
	//pTabDockAction->setIconSet( QIconSet( QPixmap::fromMimeSource(".png") ) );
	pTabDockAction->setText(tr("Dockable"));
	//pTabDockAction->setMenuText( tr("&Dockable") );
	pTabDockAction->setToolTip(tr("Dockable"));
	pTabDockAction->setCheckable(true);
	if (bDocked)
	{
		pTabDockAction->setChecked(true);
		connect(pTabDockAction, SIGNAL(triggered()), actnDisp, SLOT(slotTabUndock()));
	}
	else
		connect(pTabDockAction, SIGNAL(triggered()), actnDisp, SLOT(slotTabDock()));
	pTabMenu->addAction(pTabDockAction);

	pTabMenu->exec(pDockWin->mapToGlobal(pt));
}

void SxMainWindow::slotHandlePageMenu(QWidget * pPage, const QPoint& pt)
{
	if (!pPage)
		return;

	SxDockWindow * pDockWin = dockWin(pPage);

	SxTabWidgetEx *pTabBar = nullptr;
	//	if (pDockWin)
	//?		pTabBar = pDockWin->tabBar();

	QMenu * pTabMenu(new QMenu(this));
	const SxActionDispatcher * actnDisp = new SxActionDispatcher(this, pPage);

	int nCount = 0;
	if (pTabBar)
	{
		DocumentObject *pTabDoc = dynamic_cast<DocumentObject *>(pTabBar);
		if (pTabDoc)
		{
			QString aCaption(pTabDoc->extra());// = pDockBar->caption();
			if (aCaption.isEmpty())
				aCaption = tr("a Bar");

			bool bOnABar = pTabBar->contains(pPage);

			QAction * pTabPlaceAction = new QAction(this);//, "mpTabPlaceAction" );
			//pTabPlaceAction->setIconSet( QIconSet( QPixmap::fromMimeSource(".png") ) );
			pTabPlaceAction->setText(tr("Tab on %1").arg(aCaption));
			//pTabPlaceAction->setMenuText( tr("&Tab on %1").arg(aCaption) );
			pTabPlaceAction->setToolTip(tr("Tab on %1").arg(aCaption));
			pTabPlaceAction->setCheckable(true);
			if (bOnABar)
			{
				pTabPlaceAction->setChecked(true);
				connect(pTabPlaceAction, SIGNAL(triggered()), actnDisp, SLOT(slotPlaceHome()));
			}
			else
				connect(pTabPlaceAction, SIGNAL(triggered()), actnDisp, SLOT(slotPlaceOnABar()));
			pTabMenu->addAction(pTabPlaceAction);
			nCount++;
		}
	}

	if (pDockWin)
	{
		bool bDocked = (isPageDocked(pPage) != nullptr);

		QAction * pTabDockAction = new QAction(this);//, "mpTabDockAction" );
		//pTabDockAction->setIconSet( QIconSet( QPixmap::fromMimeSource(".png") ) );
		pTabDockAction->setText(tr("Dockable"));
		//pTabDockAction->setMenuText( tr("&Dockable") );
		pTabDockAction->setToolTip(tr("Dockable"));
		pTabDockAction->setCheckable(true);
		if (bDocked)
		{
			pTabDockAction->setChecked(true);
			connect(pTabDockAction, SIGNAL(triggered()), actnDisp, SLOT(slotTabUndock()));
		}
		else
			connect(pTabDockAction, SIGNAL(triggered()), actnDisp, SLOT(slotTabDock()));
		pTabMenu->addAction(pTabDockAction);
		nCount++;
	}

	if (nCount > 0)
		pTabMenu->addSeparator();

	bool bClosable = true;
	if (bClosable)
	{
		QAction * pTabHideAction = new QAction(this);//, "mpTabCloseAction" );
		pTabHideAction->setIcon(QIcon(":std_close_16.png"));
		pTabHideAction->setText(tr("Close"));
		//pTabHideAction->setMenuText( tr("&Close") );
		pTabHideAction->setToolTip(tr("Close"));
		connect(pTabHideAction, SIGNAL(triggered()), actnDisp, SLOT(slotTabClose()));
		pTabMenu->addAction(pTabHideAction);
		nCount++;
	}

	if (nCount > 0)
		pTabMenu->exec(pt);

	delete actnDisp;
	delete pTabMenu;
}


void SxMainWindow::slotReplacePage(QWidget * pWin)
{
	DocumentObject * pDoc = dynamic_cast<DocumentObject *>(pWin);
	if (!pDoc || pDoc->mID < 0)
		return;

	mABarConfig &= ~(1 << pDoc->mID);

	showPageEx(pWin);
}

void SxMainWindow::slotDockPage(QWidget * pWin)
{
	DocumentObject * pDoc = dynamic_cast<DocumentObject *>(pWin);
	if (!pDoc || pDoc->mID < 0)
		return;

	mWinDockConfig |= (1 << pDoc->mID);

	showPageEx(pWin);
}

void SxMainWindow::slotPlacePageOnABar(QWidget * pWin)
{
	DocumentObject * pDoc = dynamic_cast<DocumentObject *>(pWin);
	if (!pDoc || pDoc->mID < 0)
		return;

	//undock it first
	mWinDockConfig &= ~(1 << pDoc->mID);

	mABarConfig |= (1 << pDoc->mID);

	showPageEx(pWin);
}

void SxMainWindow::slotUndockWin(QWidget * pWin)
{
	DocumentObject * pDoc = dynamic_cast<DocumentObject *>(pWin);
	if (!pDoc || pDoc->mID < 0)
		return;

	mWinDockConfig &= ~(1 << pDoc->mID);

	showPageEx(pWin);
}

void SxMainWindow::restoreWinLayout(const SxWinLayout &wl)
{
	if (mpViewMgr)
		mpViewMgr->restoreWinLayout(wl);
}

void SxMainWindow::RestorePagesConfig(const SxWinLayout &wl, uint phc, uint abc, uint wdc)
{
	mABarConfig = abc;
	mWinDockConfig = wdc;

	for (QMap<int, QWidget *>::Iterator it(mPages.begin()); it != mPages.end(); it++)
	{
		int pageId(it.key());
		QWidget *pWin(it.value());
		closePage(pWin);

		DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pWin);
		if (pDoc)
		{
			QRect r;
			int state;
			QString extra(pDoc->extra());
			bool bShow(!extra.isEmpty() && wl.Find(extra, r, state) && ((1 << pageId)&phc));
			showPageEx(pWin, true, bShow);
		}
	}

	for (SxWinLayout::ConstIterator it(wl.begin()); it != wl.end(); it++)
	{
		QString s((*it).name);
		if (s.startsWith("*"))
		{
			QString path(s.mid(1));
			QFileInfo fi(path);
			if (fi.exists())
				launchEditor(path);
		}
	}

	restoreWinLayout(wl);
	updateDockedWidgetTabs();
}

void SxMainWindow::updateDockedWidgetTabs()
{
	for (QMap<QWidget *, SxDockWindow *>::ConstIterator j = mw.begin(); j != mw.end(); j++)
	{
		SxDockWindow *pDockWin(j.value());
		updateIcon(pDockWin);
	}
}

void SxMainWindow::getWinLayout(SxWinLayout &wl, bool b)
{
	if (mpViewMgr)
		mpViewMgr->getWinLayout(wl, b);
}

QString SxMainWindow::GetMDIWinLayout(bool bSaveEditDocs)
{
	SxWinLayout wl;
	getWinLayout(wl, bSaveEditDocs);

	for (QMap<int, QWidget *>::Iterator it = mPages.begin(); it != mPages.end(); it++)
	{
		QWidget *pWin(it.value());
		if (isPageVisible(pWin))
		{
			DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pWin);
			if (pDoc)
			{
				QString extra(pDoc->extra());
				if (!extra.isEmpty())
				{
					int st(0);
					QRect r;
					if (!wl.Find(extra, r, st))
						wl.Add(extra, r, st);
				}
			}
		}
	}
	return wl.toString();
}


IViewMgr *SxMainWindow::createViewMgrTabbed(QWidget *parent)
{
	QList<QWidget *> lst;

	if (mpViewMgr)
	{
		mWinLayout = GetMDIWinLayout();
		mpViewMgr->take(lst);
		mpViewMgr->release();
	}

	SxViewMgrTabs *pViewMgr = new SxViewMgrTabs(parent, "SEDITEXVIEWMGRTABS");
	//pViewMgr->setBackgroundRole(QPalette::Background);
	//pViewMgr->setAutoFillBackground(true);

	//?pViewMgr->setTabBarMenu( mpTabMenu );

	while (!lst.isEmpty())
	{
		QWidget * w = lst.front();
		lst.pop_front();
		if (w)
			pViewMgr->addDoc(w);
	}

	connect(pViewMgr, SIGNAL(signalHandlePageMenu(QWidget *, const QPoint&)),
		this, SLOT(slotHandlePageMenu(QWidget *, const QPoint&)));

//	connect(this, SIGNAL(showDocument(QWidget *)), pViewMgr, SLOT(showPage(QWidget *)));
	connect(pViewMgr, SIGNAL(signalCurrentChanged(QWidget *)), SLOT(slotTabChanged(QWidget *)));
	connect(pViewMgr, SIGNAL(signalCurrentChanged(DocumentObject *)), SLOT(slotTabChanged(DocumentObject *)));
	connect(pViewMgr, SIGNAL(enableClose(bool)), mpCloseAction, SLOT(setEnabled(bool)));
	connect(mpCloseAction, SIGNAL(triggered()), pViewMgr, SLOT(slotClosePage()));
	connect(pViewMgr, SIGNAL(signalAboutToClose(QWidget *)), SLOT(slotPageAboutToClose(QWidget *)));


	mpViewMgr = pViewMgr;
	mpWindowsMenuAction->setVisible(false);
	return mpViewMgr;
}

IViewMgr * SxMainWindow::createViewMgrTabbedEx(QWidget *parent)
{
	QList<QWidget *> lst;

	if (mpViewMgr)
	{
		mWinLayout = GetMDIWinLayout();
		mpViewMgr->take(lst);
		mpViewMgr->release();
	}

	SxViewMgrTabsEx *pViewMgr = new SxViewMgrTabsEx(parent, "SEDITEXVIEWMGRTABSEX");

	while (!lst.isEmpty())
	{
		QWidget * w = lst.front();
		lst.pop_front();
		if (w)
			pViewMgr->addDoc(w);
	}

	mpViewMgr = pViewMgr;
	mpWindowsMenuAction->setVisible(false);
	return mpViewMgr;
}

IViewMgr *SxMainWindow::createViewMgrMDI(QWidget *parent)
{
	QList<QWidget *> lst;

	if (mpViewMgr)
	{
		mpViewMgr->take(lst);
		mpViewMgr->release();
	}

	SxViewMgrMDI *pViewMgr(new SxViewMgrMDI(parent));
	//pViewMgr->setBackgroundRole(QPalette::Background);
	//pViewMgr->setAutoFillBackground(true);
	//pViewMgr->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	mpViewMgr = pViewMgr;
//	pViewMgr->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
//	pViewMgr->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	while (!lst.isEmpty())
	{
		QWidget *w = lst.front();
		lst.pop_front();
		if (w)
			pViewMgr->addDoc(w);
	}

	SxWinLayout wl(mWinLayout);
	restoreWinLayout(wl);

	connect(pViewMgr, SIGNAL(signalHandlePageMenu(QWidget *, const QPoint&)), this, SLOT(slotHandlePageMenu(QWidget *, const QPoint&)));
//	connect(this, SIGNAL(showDocument(QWidget *)), pViewMgr, SLOT(showPage(QWidget *)));
	connect(pViewMgr, SIGNAL(signalCurrentChanged(QWidget *)), SLOT(slotTabChanged(QWidget *)));
	connect(pViewMgr, SIGNAL(enableClose(bool)), mpCloseAction, SLOT(setEnabled(bool)));
	connect(mpCloseAction, SIGNAL(triggered()), pViewMgr, SLOT(slotClosePage()));

	//mpViewMgr = pViewMgr;
	mpWindowsMenuAction->setVisible(true);
	return mpViewMgr;
}

QWidget * SxMainWindow::clientWidget()
{
	return centralWidget();
}

void SxMainWindow::slotTabChanged(QWidget *)
{
}

void SxMainWindow::slotTabChanged(DocumentObject *)
{
}

void SxMainWindow::slotTabbedWindowsToggled(bool bTabs)
{
	IViewMgr *pIViewMgr;
	QWidget *pParent = clientWidget();
	if (!bTabs)
		pIViewMgr = createViewMgrMDI(pParent);
	else
		pIViewMgr = createViewMgrTabbed(pParent);

	QWidget *pWidget = dynamic_cast<QWidget *>(pIViewMgr);

	QSplitter *pSplitter = dynamic_cast<QSplitter *>(pParent);
	if (pSplitter)
		pSplitter->insertWidget(0, pWidget);
	else
		pParent->layout()->addWidget(pWidget);
}

void SxMainWindow::createViewMgrTabbedEx()
{
	QWidget *pParent = clientWidget();
	IViewMgr *pIViewMgr = createViewMgrTabbedEx(pParent);

	QWidget *pWidget = dynamic_cast<QWidget *>(pIViewMgr);
	pParent->layout()->addWidget(pWidget);
}

void SxMainWindow::updateIcon(QWidget *pWin, const char *pszIcon)
{
	if (mpViewMgr)
		mpViewMgr->updateIcon(pWin, pszIcon);
}

SxDockWindow *SxMainWindow::dockWin(QWidget *pWin)
{
	QMap<QWidget *, SxDockWindow *>::iterator it = mw.find(pWin);
	if (it != mw.end())
		return it.value();
	return nullptr;
}

SxTabWidgetEx *SxMainWindow::tabBar(QWidget * /*pWin*/)
{
	//SxDockWindow *pDockWin = dockWin(pWin);
	//	if (pDockWin)
	//	return pDockWin->tabBar();
	return nullptr;
}

void SxMainWindow::setAppropriate(QDockWidget *, bool)//qt3 compat
{
}

#if(0)
void SxMainWindow::setAppropriate(QDockWidget *dw, bool a)
{

	SxDockWindow *pDockWin = dynamic_cast<SxDockWindow *>(dw);
	if (pDockWin)
	{
		if (!pDockWin->win())
		{
			a = false;
		}
		else
		{
			SxTabWidgetEx *pTabWin = dynamic_cast<SxTabWidgetEx *>(pDockWin->win());
			if (pTabWin && !pTabWin->count())
				a = false;
		}
	}
	return QMainWindow::setAppropriate(dw, a);
}
#endif

void SxMainWindow::showPageEx0(QWidget *pWin, int bDock, int bBar, bool bRestoring, bool bShow)
{
	DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pWin);
	if (!pDoc)
		return;

	SxDockWindow *pDockWin = dockWin(pWin);
	SxTabWidgetEx *pTabBar = tabBar(pWin);
	bool bRemoved(false);

	if (bDock)
	{
		if (!pDockWin)
			return;

		if (mpViewMgr && (mpViewMgr->closeDoc(pWin) == 1))
			bRemoved = true;

		if (!bRemoved)
		{
			if (pTabBar)
			{
				if (pTabBar->hideTab(pWin) > 0)
				{
					if (pTabBar->count() == 0)
					{
						if (!mpViewMgr || (mpViewMgr->closeDoc(pTabBar) != 1))
							dockWin(pTabBar)->hide();
					}
					bRemoved = true;
				}
			}
		}

		if (!pDockWin->widget())
		{
			pDockWin->setWidget(pWin);
			pDockWin->connect(pWin, SIGNAL(signalClosePage()), pDockWin, SLOT(close()));
			setAppropriate(pDockWin, true);
		}
		else
		{
			Q_ASSERT(pDockWin->widget() == pWin);
		}
		
		if (bShow)
		{
			pDockWin->show();
			pDockWin->raise();
			updateDockedWidgetTabs();
		}
		return;
	}

	if (pDockWin)
	{
		if (pDockWin->widget() == pWin)
		{
			pDockWin->removeDoc();
			setAppropriate(pDockWin, false);
		}
	}

	if (bBar)
	{
		if (pTabBar)
		{
			if (mpViewMgr && (mpViewMgr->closeDoc(pWin) == 1))
				bRemoved = true;

			//if (!checkOnly || bRemoved)
			if (bShow)
			{
				if (pTabBar->showTab(pWin, -1))
				{
					if (!bRestoring)
						showPageEx(pTabBar);
					return;
				}
			}
		}
	}

	if (pTabBar)
	{
		if (pTabBar->hideTab(pWin) > 0)
		{
			if (pTabBar->count() == 0)
			{
				if (!mpViewMgr || (mpViewMgr->closeDoc(pTabBar) != 1))
					dockWin(pTabBar)->hide();
			}
			bRemoved = true;
		}
	}

	//if (!checkOnly || bRemoved)
	if (bShow)
	{
		QStringList lst = pDoc->mstrID.split('\n');//, true);
		if (openDoc(nullptr, lst[0], lst[1]))
		{
			SxTabWidgetEx *pTabWin = dynamic_cast<SxTabWidgetEx *>(pWin);
			if (pTabWin)
			{
				pTabWin->setTabPosition(QTabWidget::North);
				QWidget *pWinCur = pTabWin->currentWidget();
				if (pWinCur)
					pWinCur->setFocus();
			}
		}
	}
}

void SxMainWindow::showPageEx(uint pageId, bool bRestoring)
{
	showPageEx(page(pageId), bRestoring);
}

void SxMainWindow::showPageEx(QWidget *pWin, bool bRestoring, bool bShow)
{
	DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pWin);
	if (!pDoc || pDoc->mID < 0)
		return;

	int pageID = pDoc->mID;
	int abc = mABarConfig;
	int wdc = mWinDockConfig;

	showPageEx0(pWin,
		(wdc&(1 << pageID)) != 0,
		(abc&(1 << pageID)) != 0,
		bRestoring,
		bShow);
}

void SxMainWindow::closeFloatingPage(QWidget *pWin)
{
	DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pWin);
	if (!pDoc || pDoc->mID < 0)
		return;

	int pageID = pDoc->mID;
	if (mWinDockConfig & (1 << pageID))
	{
		closePage(pWin);
	}
}

/*int SxMainWindow::RegisterTabBar(uint pageId, SxTabWidgetEx *pBar, bool bNoDock)
{
	int iRet(RegisterPage(pageId, pBar, nullptr, bNoDock ? 1 : 0));
	if (iRet > 0)
		mDockBars.append(pBar);
	return iRet;
}*/

int SxMainWindow::RegisterPage(uint pageId, QWidget *pWin, SxTabWidgetEx * /*pBarWin*/, Qt::DockWidgetArea iDock)
{
	DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pWin);
	if (!pDoc)
		return 0;

	QMap<QWidget *, SxDockWindow *>::iterator it;
	it = mw.find(pWin);
	if (it != mw.end())
		return -1;//allready there

	setPage(pageId, pWin);

	pWin->setVisible(false);

	SxDockWindow *pDockWin = nullptr;
	if (iDock != Qt::NoDockWidgetArea )
	{
		pDockWin = new SxDockWindow(this, "SxDockWindow");
		pDockWin->setObjectName(pWin->objectName());
		setAppropriate(pDockWin, false);
		//pDockWin->setWin(pWin);
		pDockWin->init(pDoc);
		//pDockWin->setVisible(false);
		pDockWin->hide();
		addDockWidget(iDock, pDockWin);
//?QT4		if (!(iDock > 1))		pDockWin->setFloating(true);
		pDockWin->setContextMenuPolicy(Qt::CustomContextMenu);

		connect(pDockWin, SIGNAL(signalHide()), SLOT(slotHideDockWin()));

		connect(pDockWin, SIGNAL(signalHandlePageMenu(QWidget *, const QPoint&)), SLOT(slotHandlePageMenu(QWidget *, const QPoint&)));
		connect(pDockWin, SIGNAL(signalCustomContextMenuRequested(const QPoint &)), SLOT(slotCustomContextMenuRequested(const QPoint &)));

		connect(pDockWin, SIGNAL(signalSetAppropriate(QDockWidget *, bool)), SLOT(setAppropriate(QDockWidget *, bool)));

		connect(pDockWin, SIGNAL(topLevelChanged(bool)), SLOT(slotDockTopLevelChanged(bool)));
		connect(pDockWin, SIGNAL(visibilityChanged(bool)), SLOT(slotDockVisibilityChanged(bool)));
	}

	it = mw.insert(pWin, pDockWin);
	if (it == mw.end())
	{
		delete pDockWin;
		return 0;
	}

	/*?	if (pDockWin)
		{
		pDockWin->setBar(pBarWin);
		pDockWin->undock();
		}*/

	return 1;
}

void SxMainWindow::updateIcon(SxDockWindow *pDockWin)
{
	const QObjectList &l = children();
	QObjectList::const_iterator i;
	for (i = l.constBegin(); i != l.constEnd(); ++i)
	{
		const QObject *p = *i;
		const QTabBar *ptb = dynamic_cast<const QTabBar *>(p);
		if (ptb)
		{
			for (int i(0); i < ptb->count(); i++)
			{
				QVariant v = ptb->tabData(i);
				if (v.value<quintptr>() == (quintptr)pDockWin)
				{

					for (QMap<QWidget *, SxDockWindow *>::ConstIterator j = mw.begin(); j != mw.end(); j++)
					{
						SxDockWindow *pv(j.value());
						if (pv && pv->widget() == pDockWin->widget())
						{
							DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pDockWin->widget());
							if (pDoc)
							{
								const_cast<QTabBar *>(ptb)->setTabIcon(i, QIcon(pDoc->mstrIcon));
								break;
							}
						}
					}
					break;
				}
			}
		}
	}
}

void SxMainWindow::slotDockTopLevelChanged(bool bFloating)
{
	//if (!pWin->isFloating())
	if (!bFloating)
	{
		//updateIcon(dynamic_cast<SxDockWindow *>(sender()));
		updateDockedWidgetTabs();
	}
}

void SxMainWindow::slotDockVisibilityChanged(bool)
{
}

void SxMainWindow::slotHideDockWin()
{
	SxDockWindow *pDockWin(dynamic_cast<SxDockWindow *>(sender()));
	if (pDockWin)
		closePage(pDockWin->widget());
}

bool SxMainWindow::deletePage(QWidget *pWin)
{
	bool bClosed = false;
	if (mpViewMgr && (mpViewMgr->closeDoc(pWin) == 1))
		bClosed = true;

	if (!bClosed)
	{
		SxTabWidgetEx *pTabBar = tabBar(pWin);
		if (pTabBar && pTabBar->hideTab(pWin) > 0)
		{
			if (pTabBar->count() == 0)
			{
				if (!mpViewMgr || (mpViewMgr->closeDoc(pTabBar) != 1))
					dockWin(pTabBar)->hide();
			}
			bClosed = true;
		}
	}

	if (!bClosed)
	{
		QMap<QWidget *, SxDockWindow *>::iterator it = mw.find(pWin);
		if (it != mw.end())
		{
			SxDockWindow * pDockWin = it.value();
			if (pDockWin)
			{
				removeEventFilter(pDockWin);
				pDockWin->removeDoc();
				delete pDockWin;
				bClosed = true;
			}

			mw.erase(it);
		}
	}

	if (pWin)
	{
		removeEventFilter(pWin);
		pWin->deleteLater();
	}

	return bClosed;
}

bool SxMainWindow::EnablePage(QWidget * pWin, bool bEnable)
{
	if (bEnable)
	{
		showPageEx(pWin);
	}
	else
	{
		closePage(pWin);
	}

	QMap<QWidget *, SxDockWindow *>::iterator it = mw.find(pWin);
	if (it != mw.end())
	{
		SxDockWindow * pDockWin = it.value();
		setAppropriate(pDockWin, bEnable && pDockWin->widget());
	}

	return true;
}

int SxMainWindow::closePage(QWidget * pWin)
{
#ifdef _DEBUG
	DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pWin);
#endif

	if (mpViewMgr && (mpViewMgr->closeDoc(pWin) == 1))
		return 1;

	SxTabWidgetEx *pTabBar = tabBar(pWin);
	if (pTabBar && (pTabBar->hideTab(pWin) > 0))
	{
		if (pTabBar->count() == 0)
		{
			if (!mpViewMgr || (mpViewMgr->closeDoc(pTabBar) != 1))
			{
				dockWin(pTabBar)->hide();
			}
		}
		return 1;
	}

	SxDockWindow *pDockWin = dockWin(pWin);
	if (pDockWin)
	{
#ifdef _DEBUG
		Q_ASSERT(!pDoc || pDoc->isPermanent());
#endif
		pDockWin->hide();
		return 1;
	}

	return 0;
}

bool SxMainWindow::isPageOnABar(QWidget * pWin)
{
	QList<SxTabWidgetEx *>::Iterator it;
	for (it = mDockBars.begin(); it != mDockBars.end(); it++)
	{
		SxTabWidgetEx *pBar = *it;
		if (pBar->contains(pWin))
			return true;
	}

	return false;
}

SxDockWindow *SxMainWindow::isPageDocked(QWidget * pWin)
{
	SxDockWindow *pDockWin = dockWin(pWin);
	if (pDockWin->widget() == pWin)
		return pDockWin;

	return nullptr;
}

bool SxMainWindow::isPageVisible(QWidget * pWin)
{
	if (mpViewMgr)
		if (mpViewMgr->isDocOpened(pWin))
			return true;

	if (isPageOnABar(pWin))
		return true;

	QMap<QWidget *, SxDockWindow *>::iterator it = mw.find(pWin);
	if (it == mw.end())
		return false;

	SxDockWindow *pDockWin = it.value();
	if (!pDockWin || !pDockWin->widget())
		return false;

	return pDockWin->isVisible();
}

/*int SxMainWindow::calcPageIndex( QTabWidget * pTabWidget, int tabID )
{
int index = 0;

QMap<QWidget *, SxDockWindow *>::iterator it;
for ( it = mw.begin(); it != mw.end(); it++ )
//for ( int i = 0; i < PAGE_TOTAL; i++ )
{

if ( i == tabID )
break;
QWidget * pWidget = mPages[ i ].win;
if ( pWidget != nullptr )
if ( pTabWidget->indexOf( pWidget ) >= 0 )//not hidden
index++;
}
return index;
}*/

DocumentObject * SxMainWindow::getActiveDoc()
{
	if (!mpViewMgr)
		return nullptr;
	QWidget *pWin = mpViewMgr->getActiveView();
	return dynamic_cast<DocumentObject *>(pWin);
}

DocumentObject * SxMainWindow::openDoc(const char * cl_name, const QString &_path, const QString &_title, bool bActivate)
{
	//if (pbJustCreated)
	//	*pbJustCreated = false;

	if (!mpViewMgr)
		return nullptr;

	QString path(_path);
	QString title(_title);

/*?	if (!path.isEmpty())
	{
		QFileInfo fi(path);
		path = fi.absoluteFilePath();
	}*/

	//?path = QDir::toNativeSeparators(path);

	DocumentObject * pDoc = DocMgr().findDoc(path, title);
	QWidget *pWin = dynamic_cast<QWidget *>(pDoc);

	if (!pWin)
	{
		pWin = mpViewMgr->createView(cl_name, path, title);
		if (!pWin)
			return nullptr;
		//		if (pWin && pbJustCreated)
		//			*pbJustCreated = true;
	}
	//	else
	pWin = mpViewMgr->openView(pWin, bActivate);

	return dynamic_cast<DocumentObject *>(pWin);
}

bool SxMainWindow::renameDoc(DocumentObject *pDoc, QString path, QString title)
{
	pDoc->mstrID = DOC2KEY(path, title);//rename
	QWidget *pWin(dynamic_cast<QWidget *>(pDoc));
	mpViewMgr->renameView(pWin, path, title);
	return true;
}

DocumentObject * SxMainWindow::renameDoc(QString pathOld, QString titleOld, QString pathNew, QString titleNew)//titles only?
{
	DocumentObject *pDoc(DocMgr().findDoc(pathOld, titleOld));
	//DocumentObject *pDoc(DocMgr().renameDoc(DOC2KEY(pathOld, titleOld), DOC2KEY(pathNew, titleNew)));
	if (pDoc)
	{
		if (!renameDoc(pDoc, pathNew, titleNew))
			return nullptr;
		//QWidget *pWin(dynamic_cast<QWidget *>(pDoc));
		//mpViewMgr->renameView(pWin, pathNew, titleNew);
	}
	return pDoc;
}

bool SxMainWindow::showWin(QWidget *pWin)
{
	if (mpViewMgr->openView(pWin, true))
		return true;
	return false;
}

int SxMainWindow::getDocsCount()
{
	if (mpViewMgr)
		return mpViewMgr->count();
	return 0;
}

DocumentObject * SxMainWindow::getDoc(int index)
{
	if (mpViewMgr)
	{
		QWidget *pWin = mpViewMgr->getView(index);
		return dynamic_cast<DocumentObject *>(pWin);
	}
	return nullptr;
}

DocumentObject * SxMainWindow::findDocById(int id)
{
	if (mpViewMgr)
	{
		QWidget *pWin = mpViewMgr->getViewById(id);
		return dynamic_cast<DocumentObject *>(pWin);
	}
	return nullptr;
}

void SxMainWindow::slotWindowMenuAboutToShow()
{
	FillWindowsMenu(mpWindowMenu);
}

void SxMainWindow::FillWindowsMenu(QMenu * pMenu)
{
	if (mpViewMgr)
		mpViewMgr->fillWindowsMenu(pMenu);
}

int SxMainWindow::checkModifiedFiles()
{
	if (mpViewMgr == nullptr)
		return 1;
	if (!mpViewMgr->checkModifiedFiles())
		return 0;//modifed!
	return 1;
}

void SxMainWindow::slotFileNew()
{
}

void SxMainWindow::slotFileOpen()
{
}

void SxMainWindow::slotExit()
{
	// generate close event to go through common close routine
	close();
}

void SxMainWindow::DeletePage(int pageID)
{
	QWidget *pWin = page(pageID);
	if (pWin && deletePage(pWin))
	{
		mPages[pageID] = nullptr;
	}
}

QWidget *SxMainWindow::page(uint pageID)
{
	return mPages[pageID];
}

void SxMainWindow::setPage(uint pageID, QWidget *pWin)
{
	mPages[pageID] = pWin;
}

SxTabWidgetEx *SxMainWindow::tabWin(uint n)
{
	QMap<int, QWidget *>::Iterator it = mPages.find(n);
	if (it != mPages.end())
		return dynamic_cast<SxTabWidgetEx *>(it.value());
	return nullptr;

}


