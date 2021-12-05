#ifndef __SEDITEXWORKSPACE_H__
#define __SEDITEXWORKSPACE_H__

#include <QtCore/QString>
#include <QMdiArea>

#include "SxViewMgrTabs.h"

class QWidget;
class QAction;
class QMenu;

class SxViewMgrMDI : public QMdiArea,
	public IViewMgr
{
	Q_OBJECT

public:
	SxViewMgrMDI(QWidget *);
	virtual ~SxViewMgrMDI();

	virtual void release();
	virtual void take(QList<QWidget *> &);

	virtual QWidget *openView(QWidget *, bool);
	virtual QWidget *createView(const char *cl_name, const QString &rpath, const QString &rextra = QString());
	virtual void renameView(QWidget *, QString, QString){}

	virtual bool addDoc(QWidget *);
	virtual QWidget *getView(int index);
	virtual QWidget *getActiveView();
	virtual QWidget *getViewById(int);
	virtual bool isDocOpened(QWidget *);

	virtual int removeDoc(QWidget *);
	//virtual int removeDoc(const QString &path, const QString &title );

	virtual int closeDoc(QWidget *);
	//virtual int closeDoc( const QString &path, const QString &_extra );

	virtual int count();
	virtual bool isEmpty();
	virtual bool checkModifiedFiles();
	virtual void fillWindowsMenu(QMenu *);
	virtual void updateIcon(QWidget *, const char *);

	virtual void restoreWinLayout(const SxWinLayout &);
	virtual void getWinLayout(SxWinLayout &, bool) const;

protected:
	void childEvent(QChildEvent *);

public slots:
	void languageChange();

	void slotClosePage();
	void slotFileSaved(QWidget * pEditor, const QString &path);
	void slotCurrentChanged(QWidget *);
	void slotTileHorizontal();
	void slotWindowsMenuActivated(int);

signals:
	void signalCurrentChanged(QWidget *);
	void enableClose(bool);
	void signalHandlePageMenu(QWidget *, const QPoint&);

protected:
	SignalMultiplexer * mpSignalMultiplexer;

	QAction* mpCascadeAction;
	QAction* mpTileAction;
	QAction* mpTileHorzAction;
};


#endif//__SEDITEXWORKSPACE_H__
