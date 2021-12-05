#include "SxDocument.h"

/////////////////////////////////////////////////////////////
// D o c u m e n t O b j e c t

DocumentObject::DocumentObject()
: mbPermanent(false),
mID(-1)
{
}

DocumentObject::~DocumentObject() 
{
}


///////////////////////////////////////////////////////////////////

DocumentObject * DocumentObject::createDoc(QWidget *parent,
	const char * clname, QString &path, QString &extra)
{
	T_createDocFunc *pf = DocumentCreator::findDocFunc(clname);
	if (pf)
	{
		for (QWidget *pWin(parent); pWin; pWin = pWin->parentWidget())
		{
			SxMainWindow *pMainWin = dynamic_cast<SxMainWindow *>(pWin);
			if (pMainWin)
				return (*pf)(pMainWin, parent, path, extra);
		}
	}
	return nullptr;
}

///////////////////////////////////////////////////////////////////

QMap< QString, T_createDocFunc * >	* DocumentCreator::mpMap = nullptr;

DocumentCreator::DocumentCreator( const char * clname, T_createDocFunc * pf )
{
	mClName = clname;
	if ( clname != nullptr && pf != nullptr )
	{
		if ( mpMap == nullptr )
			mpMap = new QMap<QString, T_createDocFunc *>;
		mpMap->insert( QString( clname ), pf );
	}
}

DocumentCreator::~DocumentCreator()
{
	if ( mpMap != nullptr )
	{
		mpMap->remove( QString(mClName) );
		if ( mpMap->isEmpty() )
			delete mpMap;
	}
}

T_createDocFunc * DocumentCreator::findDocFunc( const char * clname )
{
	if ( mpMap == nullptr )
		return nullptr;
	QMap<QString, T_createDocFunc *>::iterator it;
	it = mpMap->find( QString(clname) );
	if ( it == mpMap->end() )
		return nullptr;
	return it.value();
}


