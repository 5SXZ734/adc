#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QToolButton>
#include <QTabBar>
#include <QLayout>
#include <QLabel>
#include <QPushButton>

#include "SxDocument.h"
#include "SxSignalMultiplexer.h"
#include "SxViewMgrTabs.h"


SxViewMgrTabs::SxViewMgrTabs( QWidget * parent, const char * name )
	: SxTabWidget( parent, name )
{
	setMovable(true);
	//setElideMode(Qt::ElideRight);

#ifdef OLD_CLOSEBTN
    // Close Tab Button.
    mpCloseTabBtn = new QToolButton(this);
	mpCloseTabBtn->hide();
	mpCloseTabBtn->setIcon(QIcon(":std_close_16.png"));
	mpCloseTabBtn->setAutoRaise(true);
	mpCloseTabBtn->setMaximumSize(16, 16);
	setCornerWidget(mpCloseTabBtn, Qt::TopRightCorner);

	connect(mpCloseTabBtn, SIGNAL(released()), SLOT(slotClosePage()));
#else
	setTabsClosable(true);
	connect(this, SIGNAL(tabCloseRequested(int)), SLOT(slotTabCloseRequested(int)));
#endif
	connect(this, SIGNAL(currentChanged(int)), SLOT(slotCurrentChanged(int)));

	mpSignalMultiplexer = new SignalMultiplexer;

	mpSignalMultiplexer->connect(SIGNAL(signalPageClosed()), this, SLOT(slotClosePage()));
	mpSignalMultiplexer->connect(SIGNAL(signalClosePage()), this, SLOT(slotClosePage()));
	mpSignalMultiplexer->connect(SIGNAL(signalFileSaved(QWidget *, const QString&)), this, SLOT(slotFileSaved(QWidget *, const QString &)));
	mpSignalMultiplexer->connect(SIGNAL(signalHandlePageMenu(QWidget *, const QPoint &)), this, SIGNAL(signalHandlePageMenu(QWidget *, const QPoint &)));

	//QLayout *l(layout());
	//vbox->setSpacing(0);
	//vbox->setContentsMargins(0, 0, 0, 0);

	show();
}


SxViewMgrTabs::~SxViewMgrTabs()
{
	delete mpSignalMultiplexer;
}

void SxViewMgrTabs::paintEvent(QPaintEvent *)
{
	//do not draw anything to display a background
}

void SxViewMgrTabs::take(QList<QWidget *> &l)
{
	while (QTabWidget::count() > 0)
	{
		QWidget *w(widget(0));
		removeTab(0);
		//if (plst)
		{
			w->setParent(parentWidget());
			w->move(QPoint(0, 0));
			l.append(w);
		}
		//else
		//	delete w;
	}
	mTabsStack.clear();
}

void SxViewMgrTabs::release()
{
	delete this;
}

void SxViewMgrTabs::slotCurrentChanged(int index)
{
	QWidget *pWidget(widget(index));

	if (!mTabsStack.isEmpty() && mTabsStack.back() != pWidget)
	{
		if (mTabsStack.removeOne(pWidget))
			mTabsStack.append(pWidget);
	}

	mpSignalMultiplexer->setCurrentObject(pWidget);

//	mpCloseTabBtn->setEnabled( bEnableClose );
	emit enableClose(pWidget != nullptr);

	setStyleSheet("QTabBar::tab:selected { color: #000080; }");//will trigger a polish procedure (resetting view's margins)!

	emit signalCurrentChanged(pWidget);

	DocumentObject *pDoc(dynamic_cast<DocumentObject *>(pWidget));
	if (pDoc)
		emit signalCurrentChanged(pDoc);
}

int SxViewMgrTabs::closeDoc( QWidget * pPage )
{
	if ( !pPage || indexOf( pPage ) < 0 )
		return 0;

	DocumentObject * pDoc = dynamic_cast<DocumentObject *>( pPage );
	if (pDoc && !pDoc->canClose() )
		return 0;

	//pPage->close();

	return removeDoc(pPage);
}

void SxViewMgrTabs::updateIcon(QWidget *pWin, const char *pszIcon)
{
	if (!pWin || indexOf(pWin) < 0)
		return;

	DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pWin);
	if (pDoc)
	{
		pDoc->mstrIcon = pszIcon?pszIcon:"";
		setTabIcon(indexOf(pWin), QIcon(pDoc->mstrIcon));
	}
}

void SxViewMgrTabs::restoreWinLayout(const SxWinLayout &)
{
}

void SxViewMgrTabs::getWinLayout(SxWinLayout &wl, bool bSaveEditDocs) const
{
	for ( int i = 0; i < QTabWidget::count(); i++ )
	{
		QWidget *pPage(widget(i));
		DocumentObject *pDoc(dynamic_cast<DocumentObject *>(pPage));
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
				wl.Add(key, QRect(), -1);
			}
		}
	}
}

int SxViewMgrTabs::removeDoc(QWidget *pPage)
{
	if (!pPage)
		return 0;
	
	int pageIndex = indexOf(pPage);
	if (pageIndex < 0)
		return 0;

	emit signalAboutToClose(pPage);

	blockSignals(true);
	QTabWidget::removeTab(pageIndex);
	blockSignals(false);

	mTabsStack.removeOne(pPage);

	DocumentObject *pDoc = dynamic_cast<DocumentObject *>( pPage );
	if (pDoc && pDoc->isPermanent())
	{
		pPage->setParent(parentWidget());
		pPage->move(QPoint(0, 0));
	}
	else
	{
		//delete pPage;
		pPage->deleteLater();
	}

	if (QTabWidget::count() == 0)
	{
#ifdef OLD_CLOSEBTN
		mpCloseTabBtn->hide();
#endif
		mTabsStack.clear();//?
	}
	else
	{
		int pageIndex(indexOf(mTabsStack.back()));
		setCurrentIndex(pageIndex);
	}

	emit currentChanged(currentIndex());
	return 1;
}

void SxViewMgrTabs::slotClosePage()
{
	QWidget *pPage(currentWidget());
	if (!pPage)
		return;

	closeDoc(pPage);
}

void SxViewMgrTabs::slotTabCloseRequested(int index)
{
	QWidget *pPage(widget(index));
	if (pPage)
		closeDoc(pPage);
}

static QString makeTabTitle( const QString &_path, const QString &_extra )
{
	QString path(_path);
	QString title = _extra;
	if (!path.isEmpty())
	{
		path = QDir::toNativeSeparators(path);
		QStringList l(path.split(QDir::separator(), Qt::SkipEmptyParts));
		title = l.back();
		if (title.endsWith(":"))
			title.truncate(title.length() - 1);

		if (!_extra.isEmpty())
			title.append(" - " + _extra);
	}

	return title;
}

QWidget *SxViewMgrTabs::openView(QWidget *pWin, bool bActivate)
{
	DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pWin);
	if (!pDoc)
		return nullptr;

	QString path(KEY2PATH(pDoc->mstrID));
	QString title(KEY2TITLE(pDoc->mstrID));

	bool bJustAdded(false);
	int index(indexOf(pWin));
	if (index == -1)
	{
		QString title2 = makeTabTitle(path, title);

		if (pDoc->mstrIcon.isNull())
			index = addTab(pWin, title2);
		else
			index = addTab(pWin, QIcon(pDoc->mstrIcon), title2);
		bJustAdded = true;
	}

	//QLayout *l(layout());
	//QSize fsz = frameSize();
	
	QString sTooltip(!path.isEmpty() ? path : title);
	setTabToolTip(index, sTooltip);

	if (bActivate)
	{
		if (bJustAdded)
			mTabsStack.append(pWin);
		blockSignals(true);
		setCurrentIndex(indexOf(pWin));
		blockSignals(false);
		pWin->show();
#ifdef OLD_CLOSEBTN
		mpCloseTabBtn->show();
#endif
	}
	else if (bJustAdded)
	{
		mTabsStack.prepend(pWin);
	}

	emit currentChanged(currentIndex());

	return pWin;
}

QWidget * SxViewMgrTabs::createView(const char * cl_name, const QString &_path, const QString &_title)
{
	QString path(_path);
	QString title(_title);
	DocumentObject *pDoc(DocumentObject::createDoc(this, cl_name, path, title));
	if (!pDoc)
		return nullptr;
	return dynamic_cast<QWidget *>(pDoc);
}

void SxViewMgrTabs::renameView(QWidget *pWin, QString path, QString title)
{
	QString title2(makeTabTitle(path, title));

	int index(indexOf(pWin));
	if (index != -1)
	{
		setTabText(index, title2);
		setTabToolTip(index, path);
	}
}

bool SxViewMgrTabs::addDoc( QWidget * pWidget )
{
	QString title = ":-)";
	DocumentObject * pDoc = dynamic_cast<DocumentObject *>( pWidget );
	if ( pDoc != nullptr )
	{
		QStringList lst(pDoc->mstrID.split('\n', Qt::KeepEmptyParts));
		title = makeTabTitle(lst[0], lst[1]);
	}

	if (pDoc->mstrIcon.isNull())
		addTab(pWidget, title);
	else
		addTab(pWidget, QIcon( pDoc->mstrIcon ), title);
	mTabsStack.append(pWidget);

	setCurrentIndex(indexOf(pWidget));
	
#ifdef OLD_CLOSEBTN
	mpCloseTabBtn->show();
#endif
	emit currentChanged(currentIndex());
	return true;
}

bool SxViewMgrTabs::checkModifiedFiles()
{
    if( QTabWidget::count() == 0 )
		return true;

	for (int i = 0; i < QTabWidget::count(); i++)
	{
		QWidget *pPage(widget(i));
		DocumentObject *pDoc(dynamic_cast<DocumentObject *>(pPage));
		if (!pDoc)
			continue;
		if (!pDoc->canClose())
			return false;
	}

	return true;
}

void SxViewMgrTabs::slotFileSaved(QWidget *pWin, const QString &path)
{
	DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pWin);
	if (pDoc)
		pDoc->mstrID = path + QString("\n");
	
	QFileInfo fi(path);
	setTabText(indexOf(pWin), fi.fileName());
	setTabToolTip(indexOf(pWin), path);
}

QWidget * SxViewMgrTabs::getActiveView()
{
	QWidget *pWin(currentWidget());
	return pWin;
}

bool SxViewMgrTabs::isEmpty()
{
	return QTabWidget::count() == 0;
}

int SxViewMgrTabs::count()
{
	return QTabWidget::count();
}

QWidget *SxViewMgrTabs::getView(int index)
{
	return widget(index);
}

QWidget *SxViewMgrTabs::getViewById(int docId)
{
	for (int i = 0; i < QTabWidget::count(); i++)
	{
		QWidget *pPage(widget(i));
		DocumentObject * pDoc = dynamic_cast<DocumentObject *>(pPage);
		if (pDoc && pDoc->mID == docId)
			return pPage;
	}
	return nullptr;
}

bool SxViewMgrTabs::isDocOpened(QWidget * pWin)
{
	int index = indexOf( pWin );
	return (index >= 0);
}

void SxViewMgrTabs::slotHandlePageMenu(QWidget * /*pWin*/, const QPoint &/*pt*/)
{
}





/////////////////////////////////////////////////////////////

SxViewMgrTabsEx::SxViewMgrTabsEx(QWidget * parent, const char *)
	: QWidget(parent)
{
	mpTabBar = new QTabBar(this);
	mpTabBar->setMovable(true);
		mpTabBar->addTab("One");
	//mpTabBar->addTab("Two");
	//mpTabBar->addTab("Three");

	QHBoxLayout *hLay = new QHBoxLayout();
	hLay->addWidget(mpTabBar);
	hLay->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

	QVBoxLayout * vbox(new QVBoxLayout);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->setSpacing(0);
	vbox->addLayout(hLay);
	//vbox->addWidget(mpToolbar);

	setLayout(vbox);

	/*setMovable(true);

	

	// Close Tab Button.
	mpCloseTabBtn = new QToolButton(this);
	mpCloseTabBtn->hide();
	mpCloseTabBtn->setIcon(QIcon(":std_close_16.png"));
	mpCloseTabBtn->setAutoRaise(true);
	mpCloseTabBtn->setMaximumSize(16, 16);
	setCornerWidget(mpCloseTabBtn, Qt::TopRightCorner);*/
#if(0)

	// Here some first widget
	QWidget *wid1 = new QWidget(this);
	QHBoxLayout *wid1Lay = new QHBoxLayout(wid1);
	wid1Lay->addWidget(new QLabel(tr("Widget1")));

	// Here some second widget
	QWidget *wid2 = new QWidget(this);
	QHBoxLayout *wid2Lay = new QHBoxLayout(wid2);
	wid2Lay->addWidget(new QLabel(tr("Widget2")));

	// Here some third widget
	QWidget *wid3 = new QWidget(this);
	QHBoxLayout *wid3Lay = new QHBoxLayout(wid3);
	wid3Lay->addWidget(new QLabel(tr("Widget3")));

	// Here your Tab bar with only bars
	QTabBar *bar = new QTabBar(this);
	bar->addTab("One");
	bar->addTab("Two");
	bar->addTab("Three");

	// Here some label (for example, current time) and button
	QLabel *lab = new QLabel(tr("Some text"), this);
	QPushButton *but = new QPushButton(tr("Push"), this);

	// Main layouts
	QVBoxLayout *vLay = new QVBoxLayout(this);//ui->centralWidget);
	QHBoxLayout *hLay = new QHBoxLayout();

	vLay->addLayout(hLay);
	hLay->addWidget(bar);
	// Spacer for expanding left and right sides
	hLay->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
	hLay->addWidget(lab);
	hLay->addWidget(but);

	vLay->addWidget(wid1);
	vLay->addWidget(wid2);
	vLay->addWidget(wid3);

	// Some simple connect with lambda for navigation
	/*connect(bar, &QTabBar::currentChanged, [=](int index) {

		wid1->setVisible(false);
		wid2->setVisible(false);
		wid3->setVisible(false);

		switch (index) {
		case 0: wid1->setVisible(true);
			break;
		case 1: wid2->setVisible(true);
			break;
		case 2: wid3->setVisible(true);
			break;
		default:{}
		}

	});

	emit bar->currentChanged(0);*/
#endif

	show();
}

SxViewMgrTabsEx::~SxViewMgrTabsEx()
{
}


void SxViewMgrTabsEx::release()
{
	delete this;
}

void SxViewMgrTabsEx::take(QList<QWidget *> & /*l*/)
{
	/*while (QTabWidget::count() > 0)
	{
		QWidget *w(widget(0));
		removeTab(0);
		//if (plst)
		{
			w->setParent(parentWidget());
			w->move(QPoint(0, 0));
			l.append(w);
		}
		//else
		//	delete w;
	}*/
}

int SxViewMgrTabsEx::closeDoc(QWidget *)
{
	return 0;
}

QWidget *SxViewMgrTabsEx::openView(QWidget *pWin, bool)
{
	DocumentObject *pDoc = dynamic_cast<DocumentObject *>(pWin);
	if (!pDoc)
		return nullptr;

	QString path(KEY2PATH(pDoc->mstrID));
	QString title(KEY2TITLE(pDoc->mstrID));

	
	bool bJustAdded(false);
	//DocumentMgr().findDoc(pDoc->mstrID);
	//int index(indexOf(pWin));
	//if (index == -1)
	{
		QString title2 = makeTabTitle(path, title);

		if (pDoc->mstrIcon.isNull())
			mpTabBar->addTab( title2);
		else
			mpTabBar->addTab(QIcon(pDoc->mstrIcon), title2);
		bJustAdded = true;
	}

	mpTabBar->show();

	/*setTabToolTip(index, (!path.isEmpty()) ? path : title);

	if (bActivate)
	{
		if (bJustAdded)
			mTabsStack.append(pWin);
		setCurrentIndex(indexOf(pWin));
		pWin->show();
		mpCloseTabBtn->show();
	}
	else if (bJustAdded)
	{
		mTabsStack.prepend(pWin);
	}*/

	update();
	return nullptr;
}

QWidget *SxViewMgrTabsEx::createView(const char * cl_name, const QString &rpath, const QString &rextra)
{
	QString path(rpath);
	QString title(rextra);
	DocumentObject *pDoc = DocumentObject::createDoc(this, cl_name, path, title);
	QWidget *pWin = dynamic_cast<QWidget *>(pDoc);
	return pWin;
}

void SxViewMgrTabsEx::renameView(QWidget *, QString, QString)
{
}

bool SxViewMgrTabsEx::addDoc(QWidget *)
{
	return false;
}

QWidget *SxViewMgrTabsEx::getView(int)
{
	return nullptr;
}

QWidget *SxViewMgrTabsEx::getActiveView()
{
	return nullptr;
}

QWidget *SxViewMgrTabsEx::getViewById(int)
{
	return nullptr;
}

int SxViewMgrTabsEx::count()
{
	return 0;
}

bool SxViewMgrTabsEx::isEmpty()
{
	return 0;
}

bool SxViewMgrTabsEx::isDocOpened(QWidget *)
{
	return false;
}



