#ifndef __SEDITEXDOCKWINDOW_H__
#define __SEDITEXDOCKWINDOW_H__

//#include <q3dockwindow>

class SxDockTabWidget;
class QPushButton;
class SxDockWindow;
class SxTabWidgetEx;
class DocumentObject;
#if(0)
class SxDockWindowBottomBar : public QWidget
{
Q_OBJECT
public:
	SxDockWindowBottomBar(SxDockWindow *);
	~SxDockWindowBottomBar(){}
signals:
	void signalHide();
private:
	QPushButton *mpHidePBtn;
};



class SxDockWindow : public Q3DockWindow
{
Q_OBJECT
public:
	SxDockWindow(QWidget *, const char *);
	~SxDockWindow();

	void init(DocumentObject *);
	QWidget * win(){ return mpWin; }
	void setWin(QWidget * pWin);
	void setBar(SxTabWidgetEx *pWin){ mpTabBar = pWin; }
	SxTabWidgetEx * tabBar(){ return mpTabBar; }
	void removeDoc();
	virtual void hide();
	void enableBottomBar();
	
	virtual void show();

protected:
	virtual bool eventFilter(QObject *, QEvent *);
	virtual void childEvent(QChildEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void contextMenuEvent(QContextMenuEvent *);

private:
	void showBottomBar(bool);
	void OnSetWin(bool);

protected slots:
	void slotHide();
	void slotPlaceChanged(Q3DockWindow::Place);
	void slotPageOpened();
	void slotPageClosed();

signals:
	void signalSetAppropriate(Q3DockWindow *, bool);
	void signalHandlePageMenu(QWidget *, const QPoint &);
	void signalHide(QWidget *);

protected:
	QWidget			*mpWin;
	SxTabWidgetEx	*mpTabBar;
	SxDockWindowBottomBar	*mpBottomBar;
};

#else
#include <QDockWidget>

class SxDockWindow : public QDockWidget
{
	Q_OBJECT
public:
	SxDockWindow(QWidget *parent, const char *)
		: QDockWidget(parent)//, Qt::WState_CreatedHidden)
	{
		connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), SIGNAL(signalCustomContextMenuRequested(const QPoint &)));
	}
	void init(DocumentObject *);
	void removeDoc()
	{
		setVisible(false);
		QWidget *pWin(widget());
		if (pWin)
		{
			setWidget(nullptr);
			pWin->disconnect(this);
			disconnect(pWin);
		}
	}

protected:
	virtual void closeEvent(QCloseEvent *);
	virtual void contextMenuEvent(QContextMenuEvent *);

signals:
	void signalHide();
	void signalHandlePageMenu(QWidget *, const QPoint&);
	void signalSetAppropriate(QDockWidget *, bool);
	void signalCustomContextMenuRequested(const QPoint &);
};

#endif

#endif//__SEDITEXDOCKWINDOW_H__
