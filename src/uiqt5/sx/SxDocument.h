#ifndef __DOCUMENTOBJECT_H__
#define __DOCUMENTOBJECT_H__

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QList>
#include "SxMainWindow.h"

class QWidget;
class QFont;

class DocumentObjectBase 
{
public:
	DocumentObjectBase(){}
	virtual ~DocumentObjectBase(){}

	virtual void activated(){}
	virtual void updateOutputFont(const QFont &){}
};

inline QString DOC2KEY(QString path, QString title){ return path + "\n" + title; }
inline QString KEY2PATH(QString key){ return key.section("\n", 0, 0); }
inline QString KEY2TITLE(QString key){ return key.section("\n", 1); }

class DocumentObject : public DocumentObjectBase
{
public:
	DocumentObject();
    virtual ~DocumentObject();
	virtual bool isPermanent() { return mbPermanent; }
	virtual int canClose(){ return 1; }
	virtual int canHide(){ return 1; }

	void setPermanent(bool b){ mbPermanent = b; }
	//void select();
	void setStrID( const QString &path, const QString &extra )
	{
		mstrID = path + QString( "\n" ) + extra;
	}

	QString path(){ return mstrID.section('\n', 0, 0); }
	QString extra(){ return mstrID.section('\n', 1); }

	static DocumentObject * createDoc( QWidget * pParent,
		const char * clname, QString &path, QString &_extra );

public:
	int mID;
	QString mstrID;
	QString mstrIcon;
	bool mbPermanent;	//preserved when closed
};

typedef DocumentObject * T_createDocFunc(SxMainWindow *, QWidget *, QString &, QString &);

class DocumentCreator
{
public:
	DocumentCreator( const char * clname, T_createDocFunc * pf );
	~DocumentCreator();
	static T_createDocFunc * findDocFunc( const char * clname );
private:
	static QMap< QString, T_createDocFunc * >	* mpMap;
	const char * mClName;
};

template <class T> class DocumentWin : public T, public DocumentObject
{
public:
	DocumentWin(SxMainWindow *pMainWin, const char *name)
		: T(pMainWin, name),
		mpMainWin(pMainWin)
	{
		mpMainWin->DocMgr().push_back(this);
	}

	virtual ~DocumentWin()
	{
		mpMainWin->DocMgr().removeAll(this);
	}

protected:
	virtual void activated()
	{
		T::emitAllSignals();
	}
	virtual void updateOutputFont(const QFont &f)
	{
		T::updateOutputFont(f);
	}
private:
	SxMainWindow *mpMainWin;
};

template <class T> class DocumentWin2 : public T
{
public:
	DocumentWin2(SxMainWindow *pMainWin, const char *name)
		: T(pMainWin, name),
		mpMainWin(pMainWin)
	{
		mpMainWin->DocMgr().push_back(this);
	}

	virtual ~DocumentWin2()
	{
		mpMainWin->DocMgr().removeAll(this);
	}

protected:
	virtual void activated()
	{
		T::emitAllSignals();
	}
	virtual void updateOutputFont(const QFont &f)
	{
		T::updateOutputFont(f);
	}
private:
	SxMainWindow *mpMainWin;
};


#endif//_DOCUMENTOBJECT_H_
