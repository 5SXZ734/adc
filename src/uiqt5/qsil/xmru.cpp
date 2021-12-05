
#include <QtCore/QDir>
#include <QtCore/QSignalMapper>
#include "xmru.h"
#include "xshared.h"

void QSilMRUData::slotMRU(int id)
{
	emit mruSelected(m_pMRU->m_MRU[id]);
}

QAction *QSilMRU::createMostRecentUsedMenu(QObject *reciever, const char *member, QMenu *menu, QAction *before)
{
	QSilMRUPopupMenu *pMenu = new QSilMRUPopupMenu(this, menu->parentWidget());
	m_Data[pMenu] = new QSilMRUData(this, pMenu, reciever, member);

	//id = menu->insertItem(m_Text, pMenu, id, index);
	QAction *pid = menu->insertMenu(before, pMenu);
	pid->setText(m_Text);

	if (m_MRU.size())
		pMenu->setEnabled(true);
	else
		pMenu->setEnabled(false);

	connect(m_Data[pMenu], SIGNAL(mruSelected(const QString &)), reciever, member);

	return pid;
}

void QSilMRU::updateMostRecentUsedMenu(QString item)
{

#ifdef WIN32
	// fix for lowercase drive letters
	if ((item.length() > 1) && (':' == item[1]))
	{
		item[0] = item[0].toUpper();
	}
#endif

	QStringList::Iterator it;// = m_MRU.find(item);
	int i = m_MRU.indexOf(item);
	it = (i == -1 ? m_MRU.end() : (m_MRU.begin() + i));

	if (it == m_MRU.end())
		m_MRU.prepend(item);

	else
	{
		if (m_MRU.size() > 1 && it != m_MRU.begin())
		{
			m_MRU.erase(it);
			m_MRU.prepend(item);
		}
	}

	if (m_MRU.size() > m_MaxMRU)
		m_MRU.pop_back();

	for (QSilMRUDataMap::Iterator data_it = m_Data.begin();
		data_it != m_Data.end();
		++data_it)
	{
		(*data_it)->m_pMenu->setEnabled(true);
	}
}

void QSilMRU::removeFromMostRecentUsedMenu(QString item)
{

#ifdef WIN32
	// fix for lowercase drive letters
	if ((item.length() > 1) && (':' == item[1]))
	{
		item[0] = item[0].toUpper();
	}
#endif

	QStringList::Iterator it;// = m_MRU.find(item);
	int i = m_MRU.indexOf(item);
	it = (i == -1 ? m_MRU.end() : (m_MRU.begin() + i));

	if (it == m_MRU.end())
		return;

	else
		m_MRU.erase(it);

	for (QSilMRUDataMap::Iterator data_it = m_Data.begin();
		data_it != m_Data.end();
		++data_it)
	{
		if (m_MRU.size())
			(*data_it)->m_pMenu->setEnabled(true);

		else
			(*data_it)->m_pMenu->setEnabled(false);
	}
}

void QSilMRU::clearAndUpdate(QSilMRUPopupMenu *pMenu)
{
	QSilMRUDataMap::Iterator data_it = m_Data.find(pMenu);

	if (data_it != m_Data.end())
	{
		// Clear (delete) old actions
		//m_Actions.setAutoDelete(true);
		(*data_it)->m_Actions.clear();
		//m_Actions.setAutoDelete(false);
		pMenu->clear();

		if (m_MRU.size())
		{
			int item_number = 0;

			for (QStringList::Iterator it = m_MRU.begin(); (it != m_MRU.end()) && (item_number < m_MaxMRU); ++it, ++item_number)
			{
				/*
						id = pMenu->insertItem( QString("&%1 %2").arg(item_number).arg(QSil::shortenFilePath( *it )),
						*data_it,
						SLOT( slotMRU( int ) ) );

						pMenu->setItemParameter( id, item_number );
						*/
				QAction* new_action = new QAction(QString("&%1 %2").arg(item_number + 1).arg(QSil::shortenFilePath(QDir::toNativeSeparators(*it))), pMenu->parent());
				QSignalMapper* signal_mapper = new QSignalMapper(new_action);// , "signal_mapper");

				signal_mapper->setMapping(new_action, item_number);
				connect(new_action, SIGNAL(triggered()), signal_mapper, SLOT(map()));
				connect(signal_mapper, SIGNAL(mapped(int)), *data_it, SLOT(slotMRU(int)));

				// Show full path in statusbar
				new_action->setStatusTip(tr("Open ") + *it);

				pMenu->addAction(new_action);
				// Store QAction, so we can delete it on next call
				(*data_it)->m_Actions.append(new_action);
			}

			pMenu->setEnabled(true);
		}
		else
			pMenu->setEnabled(false);
	}
}

void QSilMRU::remove(QSilMRUPopupMenu *pMenu)
{
	QSilMRUDataMap::Iterator data_it = m_Data.find(pMenu);
	if (data_it != m_Data.end())
		m_Data.erase(data_it);

	//m_Actions.clear();
}
