#ifndef QSIL_MRU_H
#define QSIL_MRU_H

#include <QtCore/QObject>
#include <QMenu>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>

class QSilMRUPopupMenu;
class QSilMRUData;

class QSilMRU : public QObject
{
	Q_OBJECT

public:
	typedef QList <QSilMRUData *> QSilMRUDataList;
	typedef QMap <QSilMRUPopupMenu *, QSilMRUData *> QSilMRUDataMap;

	QSilMRU(QObject* parent, QString text, int size = 4) :
		QObject(parent),// "qsil_MRU_internal_object"),
		m_Text(text),
		m_MaxMRU(size)
	{}

	~QSilMRU() {}
	QAction *createMostRecentUsedMenu(QObject *reciever, const char *member, QMenu *menu, QAction *before);
	void updateMostRecentUsedMenu(QString item);
	void removeFromMostRecentUsedMenu(QString item);

	void clearAndUpdate(QSilMRUPopupMenu *);
	void remove(QSilMRUPopupMenu *);

	QSilMRUDataMap    m_Data;
	QStringList       m_MRU;
	int               m_MaxMRU;
	QString           m_Text;
private:
	// Disabled copy constructor and operator=
	QSilMRU(const QSilMRU&);
	QSilMRU& operator=(const QSilMRU&);
};

class QSilMRUData : public QObject
{
	Q_OBJECT
public:
	QSilMRUData(QSilMRU *mru,
		QSilMRUPopupMenu *menu,
		QObject *receiver,
		const char *member) : QObject(mru),//, "qsil_mru_data_internal"), 
		m_pMRU(mru),
		m_pMenu(menu),
		m_pReciever(receiver),
		m_Member(member)
	{} //QT4 { m_Actions.setAutoDelete(true); }
	~QSilMRUData(){} //QT4 {  m_Actions.setAutoDelete(false); }

	QSilMRU          *m_pMRU;
	QSilMRUPopupMenu *m_pMenu;
	QObject          *m_pReciever;
	const char       *m_Member;
	QList<QAction*>   m_Actions;

signals:
	void mruSelected(const QString &);

	public slots:

	virtual void slotMRU(int id);
private:
	// Disabled copy constructor and operator=
	QSilMRUData(const QSilMRUData&);
	QSilMRUData& operator=(const QSilMRUData&);
};


class QSilMRUPopupMenu : public QMenu
{
	Q_OBJECT

public:

	QSilMRUPopupMenu(QSilMRU *pMRU, QWidget *parent = 0) : QMenu(parent),
		m_pMRU(pMRU)
	{
		connect(this, SIGNAL(aboutToShow()), this, SLOT(menuAboutToShow()));
	}

	virtual ~QSilMRUPopupMenu()
	{
		m_pMRU->remove(this);
	}

	QSilMRU *m_pMRU;

	public slots:

	void menuAboutToShow()
	{
		m_pMRU->clearAndUpdate(this);
	}
private:
	// Disabled copy constructor and operator=
	QSilMRUPopupMenu(const QSilMRUPopupMenu&);
	QSilMRUPopupMenu& operator=(const QSilMRUPopupMenu&);

};
typedef QHash<QString, QSilMRU*> QSilMRUMap;

#endif
