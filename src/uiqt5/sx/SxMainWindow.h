#ifndef __SXMAINWINDOW_H__
#define __SXMAINWINDOW_H__

#include <QMainWindow>
#include <QAction>
#include <QtCore/QList>

class QVBox;
class QLabel;
class QMenu;
class QAction;
class QTabWidget;
class QPixmap;

class IViewMgr;
class DocumentObject;
class SignalMultiplexer;
//class SxDockWindow;
#include "SxDockWindow.h"
class DocumentMgr;
class SxTabWidgetEx;
class SxWinLayout;
class QSignalMapper;

//#define WINDOW_MENU_ID	100
#define STATUSSLOT_MAX	10

class DocumentMgr : public QList<DocumentObject *>
{
public:
	DocumentMgr(){}
	DocumentObject *findDoc( const QString &path, const QString &title ) const;
	DocumentObject *renameDoc(QString keyOld, QString keyNew);
	void cleanUp();
private:
	QList<DocumentObject *>::ConstIterator findDoc(QString key) const;
};

class SxMainWindow : public QMainWindow
{
Q_OBJECT
public:
	friend class SxActionDispatcher;

	SxMainWindow(QWidget * pparent = nullptr, const char * pczname = nullptr, Qt::WindowFlags f = Qt::Window);//WType_TopLevel);
	virtual ~SxMainWindow();

	virtual bool createSelf();
	IViewMgr * createViewMgrTabbed(QWidget *);
	IViewMgr * createViewMgrMDI(QWidget *);
	IViewMgr * createViewMgrTabbedEx(QWidget *);

protected:
	void createViewMgrTabbedEx();
	void DeletePage(int pageID);
	QWidget *page(uint);
	SxTabWidgetEx *tabWin(uint);
	void setPage(uint, QWidget *);
	void RestorePagesConfig(const SxWinLayout &, uint, uint, uint);
	//uint getABarConfig(){ return mABarConfig; }
	uint getWinDockConfig(){ return mWinDockConfig; }
	bool showWin(QWidget *);

	QString GetMDIWinLayout(bool = true);

	QString productInfoStr(const QString &productStr, const QString &versionStr, const QString &copyrightStr);

#ifndef WIN32
	virtual void setIcon(const QPixmap& p);
#endif
	virtual QWidget *clientWidget();

	DocumentObject * getActiveDoc();
	DocumentObject * openDoc(const char *cl_name, const QString &rpath, const QString &rextra, bool bActivate = true);
	DocumentObject * renameDoc(QString, QString, QString, QString);
	bool renameDoc(DocumentObject *, QString, QString);

	//int removeDoc(QWidget *);
	//int removeDoc(const QString &path, const QString &_extra);
	int getDocsCount();
	DocumentObject * getDoc( int index );
	DocumentObject * findDocById(int id);

	void showPageEx(uint, bool = false);
	void showPageEx(QWidget *, bool = false, bool bShow = true);
	int calcPageIndex(QTabWidget *, int tabID);

	SxDockWindow *isPageDocked(QWidget * pWin);
	bool isPageOnABar(QWidget * pWin);
	bool isPageVisible(QWidget * pWin);
	bool deletePage(QWidget * pWin);
	bool EnablePage(QWidget * pWin, bool bEnable);
	virtual int closePage( QWidget * pWin );
	int RegisterPage(uint, QWidget *, SxTabWidgetEx *pBar = nullptr, Qt::DockWidgetArea iDock = Qt::NoDockWidgetArea);
	//int RegisterTabBar(uint pageId, SxTabWidgetEx *pBar, bool bNoDock = false);
	void closeFloatingPage(QWidget *);
	void updateIcon(QWidget *, const char *);

	void FillWindowsMenu(QMenu * pMenu);
	int checkModifiedFiles();

	QWidget * addStatusSlot(QWidget *pCustomWidget = nullptr, const char *objName = nullptr);
	void setStatusSlotText(QWidget * pWidget, const QString& s);
	void setStatusSlotPixmap(QWidget * pWidget, const QPixmap &);
	void closeStatusSlot(QWidget * pWidget);

	QAction *NewAction(const char *pixmap, const QString &menuText,
					const QString &toolTip = QString(), const QString &statusTip = QString(),
					const QString &accel = QString(),
					const char *slot = nullptr);

	QAction *NewToggleAction(const char *pixmap, const QString &menuText,
					const QString &toolTip = QString(), const QString &statusTip = QString(),
					const QString &accel = QString(),
					const char *slot = nullptr);

	SxDockWindow *dockWin(QWidget *);
	virtual void launchEditor(const QString &){}
public:
	DocumentMgr &DocMgr(){ return mDocMgr; }

private:
	void showPageEx0(QWidget *, int bDock, int bBar, bool bRestoring, bool bShow);
	SxTabWidgetEx *tabBar(QWidget *);
	void getWinLayout(SxWinLayout &, bool);
	void restoreWinLayout(const SxWinLayout &);
	void updateIcon(SxDockWindow *);
	void updateDockedWidgetTabs();

protected slots:
	void slotStatusMsg(const QString &);
	void slotWindowMenuAboutToShow();

	void slotHandlePageMenu(QWidget *, const QPoint&);
	void slotCustomContextMenuRequested(const QPoint &);

	void slotTabPlace();
	void slotTabDock();

	virtual void slotTabChanged(QWidget *);
	virtual void slotTabChanged(DocumentObject *);
	virtual void slotTabbedWindowsToggled( bool mdi );

	virtual void slotFileNew();
	virtual void slotFileOpen();
	virtual void slotExit();

	virtual void slotReplacePage(QWidget *);
	virtual void slotDockPage(QWidget *);
	virtual void slotPlacePageOnABar(QWidget *);
	virtual void slotUndockWin(QWidget *);
	
	void slotHideDockWin();
	void slotDockTopLevelChanged(bool);
	void slotDockVisibilityChanged(bool);

	virtual void setAppropriate(QDockWidget *, bool);
	virtual void slotPageAboutToClose(QWidget *){}

signals:
	void showDocument(QWidget *);
	void signalReplacePage(QWidget *);
	void signalDockPage(QWidget *);

protected:
	//QMenuBar *mpMainMenuBar;

	QWidget * mpStatusSlots[STATUSSLOT_MAX];

	QMenu *mpFileMenu;
	QMenu *mpEditMenu;
	QMenu *mpViewMenu;
	QMenu *mpWindowMenu;
	QMenu *mpHelpMenu;

    QAction* mpNewAction;
    QAction* mpOpenAction;
	QAction	*mpExitAction;
	QAction	*mpCloseAction;
	QAction	*mpTabbedWindowsAction;
	QAction	*mpWindowsMenuAction;

	int mABarConfig;
	int mWinDockConfig;

	QActionGroup	*mpActions;

protected:
	QWidget	*mpCopyright;

private:
	IViewMgr *mpViewMgr;
	QList<SxTabWidgetEx *>	mDockBars;
	QMap<QWidget *, SxDockWindow *>	mw;
	QMap<int, QWidget *> mPages;
	DocumentMgr mDocMgr;
	QString	mWinLayout;

	QSignalMapper *mpDockWinMapper;
};


class SxActionDispatcher : public QObject
{
	Q_OBJECT
public:
	SxActionDispatcher(SxMainWindow * pMainWin, QWidget * pPage)
		: QObject(pMainWin),
		mpMainWin(pMainWin),
		mpPage(pPage)
	{
	}

public slots:
	void slotPlaceHome();
	void slotPlaceOnABar();
	void slotTabDock();
	void slotTabUndock();
	void slotTabClose();

private:
	SxMainWindow *	mpMainWin;
	QWidget *		mpPage;
};


class SxIcon : public QIcon
{
public:
	SxIcon(const QString& path)
	{
		int n(path.indexOf('{'));
		if (n > 0)
		{
			int m(path.indexOf('}', n));
			if (m > n)
			{
				QString s(path.mid(n + 1, m - n - 1));
				QStringList l(s.split('|', QString::SkipEmptyParts));
				for (int i(0); i < l.size(); i++)
				{
					s = path;
					addFile(s.replace(n, m - n + 1, l[i]));
				}
				return;
			}
		}
		addFile(path);
	}
};


#endif//__SXMAINWINDOW_H__
