#pragma once

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QTabWidget>

#include "SxTabWidget.h"

class QToolButton;
class QMenu;
class SignalMultiplexer;
class DocumentObject;

struct SxWinLayoutElt
{
	SxWinLayoutElt()
		: state(0)
	{
	}
	QString	name;
	QRect	r;
	int state;
};

class SxWinLayout : public QList<SxWinLayoutElt>
{
public:
	SxWinLayout(){}
	SxWinLayout(QString phc)
	{
		QStringList l0 = phc.split(QChar('\n'));
		for (QStringList::Iterator it = l0.begin(); it != l0.end(); it++)
		{
			QString s(*it);
			QStringList l;
			int n(s.indexOf(QChar('<')));
			if (n > 0)
			{
				int n2(s.indexOf(QChar('>'), -1));
				if (n < n2)
				{
					QString sr(s.mid(n + 1, n2 - (n + 1)));
					l = sr.split(QChar(','));
					s.truncate(n);
				}
			}

			QRect r;
			int state(0);
			if (l.count() > 0)
			{
				state = l[0].toInt();
				if (l.count() > 4)
					r = QRect(QPoint(l[1].toInt(), l[2].toInt()), QSize(l[3].toInt(), l[4].toInt()));
			}

			Add(s.left(n), r, state);
		}
	}

	void Add(const QString &s, const QRect &r, int state)
	{
		Iterator it(find(s));
		if (it != end())
		{
			SxWinLayoutElt &e(*it);
			e.r = r;
			e.state = state;
		}
		else
		{
			SxWinLayoutElt e;
			e.name = s;
			e.r = r;
			e.state = state;
			append(e);
		}
	}
	bool Find(const QString &s, QRect &r, int &st) const
	{
		ConstIterator it(begin());
		for (; it != end(); it++)
		{
			const SxWinLayoutElt &e(*it);
			if (e.name == s)
			{
				r = e.r;
				st = e.state;
				return true;
			}
		}
		return false;
	}
	QString toString() const
	{
		QString s;
		for (ConstIterator it = begin(); it != end(); it++)
		{
			const SxWinLayoutElt &e(*it);
			s.append(e.name);
			if (e.state != 0 || !e.r.isEmpty())
			{
				const QRect &r(e.r);
				if (r.isEmpty())
					s.append(QString("<%1>").arg(e.state));
				else
					s.append(QString("<%1,%2,%3,%4,%5>").arg(e.state).arg(r.left()).arg(r.top()).arg(r.width()).arg(r.height()));
			}
			s.append("\n");
		}
		return s;
	}

	SxWinLayout &removePaths(bool bRemove)
	{
		if (bRemove)
		{
			SxWinLayout::Iterator it(begin());
			while (it != end())
			{
				QString s((*it).name);
				if (s.startsWith("*"))
				{
					SxWinLayout::Iterator itnx(it);
					itnx++;
					erase(it);
					it = itnx;
					continue;
				}
				it++;
			}
		}
		return *this;
	}
private:
	Iterator find(const QString &s)
	{
		Iterator it(begin());
		for (; it != end(); it++)
		{
			SxWinLayoutElt &e(*it);
			if (e.name == s)
				break;
		}
		return it;
	}
};

///////////////////////////////////
// IViewMgr

class IViewMgr
{
public:
	virtual void release() = 0;
	virtual void take(QList<QWidget *> &) = 0;

	virtual int closeDoc(QWidget *) = 0;

	virtual QWidget *openView(QWidget *, bool) = 0;
	virtual QWidget *createView(const char * cl_name, const QString &rpath, const QString &rextra = QString()) = 0;
	virtual void renameView(QWidget *, QString, QString) = 0;

	virtual bool addDoc(QWidget *) = 0;
	virtual QWidget *getView(int index) = 0;
	virtual QWidget *getActiveView() = 0;
	virtual QWidget *getViewById(int) = 0;

	virtual int count() = 0;
	virtual bool isEmpty() = 0;
	virtual bool checkModifiedFiles() = 0;
	virtual bool isDocOpened(QWidget *) = 0;

	virtual void fillWindowsMenu(QMenu *){}
	virtual void updateIcon(QWidget *, const char *) = 0;

	virtual void restoreWinLayout(const SxWinLayout &) = 0;
	virtual void getWinLayout(SxWinLayout &, bool) const = 0;
};

////////////////////////////////////////
class SxViewMgrTabs : public SxTabWidget,
	public IViewMgr
{
	Q_OBJECT

public:
	SxViewMgrTabs(QWidget * parent, const char * name = 0);
	virtual ~SxViewMgrTabs();

	virtual void paintEvent(QPaintEvent *);

	virtual QWidget *openView(QWidget *, bool);
	virtual QWidget *createView(const char * cl_name, const QString &path, const QString &extra = QString());
	virtual void renameView(QWidget *, QString, QString);

	virtual void release();
	virtual void take(QList<QWidget *> &);

	virtual bool addDoc(QWidget *);
	virtual bool checkModifiedFiles();

	virtual int removeDoc(QWidget *);
	//virtual int removeDoc( const QString &path, const QString &title );

	virtual bool isEmpty();
	virtual int count();

	virtual QWidget *getView(int index);
	virtual QWidget *getViewById(int);
	virtual QWidget *getActiveView();
	virtual bool isDocOpened(QWidget *);

	virtual int closeDoc(QWidget *);
	//virtual int closeDoc( const QString &path, const QString &_extra );
	virtual void updateIcon(QWidget *, const char *);

	virtual void restoreWinLayout(const SxWinLayout &);
	virtual void getWinLayout(SxWinLayout &, bool) const;

public slots:
	void slotClosePage();
	void slotTabCloseRequested(int);

private slots:
	void slotFileSaved(QWidget *, const QString &);
	void slotCurrentChanged(int);
	void slotHandlePageMenu(QWidget *, const QPoint &);

signals:
	void enableClose(bool);
	void signalCurrentChanged(QWidget *);
	void signalCurrentChanged(DocumentObject *);
	void signalAboutToClose(QWidget *);

protected:
#ifdef OLD_CLOSEBTN
	QToolButton *		mpCloseTabBtn;
#endif
	QToolButton *		mpNewTabBtn;
	SignalMultiplexer * mpSignalMultiplexer;
	QList<QWidget *>	mTabsStack;
};



////////////////////////////////////////
// SxViewMgrTabsEx

class SxViewMgrTabsEx : public QWidget,
	public IViewMgr
{
	Q_OBJECT

public:
	SxViewMgrTabsEx(QWidget * parent, const char * name = 0);
	virtual ~SxViewMgrTabsEx();

public:
	virtual void release();
	virtual void take(QList<QWidget *> &);

	virtual int closeDoc(QWidget *);

	virtual QWidget *openView(QWidget *, bool);
	virtual QWidget *createView(const char * cl_name, const QString &rpath, const QString &rextra = QString());
	virtual void renameView(QWidget *, QString, QString);

	virtual bool addDoc(QWidget *);
	virtual QWidget *getView(int index);
	virtual QWidget *getActiveView();
	virtual QWidget *getViewById(int);

	virtual int count();
	virtual bool isEmpty();
	virtual bool checkModifiedFiles(){ return false; }
	virtual bool isDocOpened(QWidget *);

	//virtual void fillWindowsMenu(QMenu *){}
	virtual void updateIcon(QWidget *, const char *){}

	virtual void restoreWinLayout(const SxWinLayout &){}
	virtual void getWinLayout(SxWinLayout &, bool) const{}
private:

	QTabBar *mpTabBar;
};