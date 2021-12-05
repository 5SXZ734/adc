#include <QApplication>
#include <QtCore/QEventLoop>
#include <QDesktopWidget>
#include <QDialog>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QLayout>
#include <QMainWindow>
#include <QMessageBox>
#include <QtCore/QMap>
#include <QMenuBar>
#include <QtCore/QObjectList>
#include <QtCore/QPair>
#include <QMenu>
#include <QSplitter>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QToolButton>
#include <QToolTip>
//#include <Q3UrlOperator>
#include <QtCore/QRegExp>

#include "xresmgr.h"
#include "xresource.h"
#include "xwindowsettings.h"

// Most Recently Used...
#include "xmru.h"
static QSilMRUMap* sStoredMRUs = 0;

#include <assert.h>
#include <iostream>
#include <set>

typedef QMap<QWidget*, QPair<QString, QString> > WindowKeyMap;
static WindowKeyMap* spWindowKeys = 0;

typedef QMap<QWidget*, GeomSettings> GeomSettingsMap;
static GeomSettingsMap* spGeomSettings = 0;

typedef QMap<QMainWindow*, ToolbarSettings> WindowToolbarSettingsMap;
static WindowToolbarSettingsMap* spToolbarSettings = 0;

typedef QMap<QMainWindow*, QSilResManager::ShortcutMap> WindowShortcutMap;
static WindowShortcutMap* spShortcuts = 0;


static QSilWindowTracker* spWindowTracker = 0;

const QString QSilResManager::USE_WINDOW_NAME = QString();

const char* QSilResManager::ALL_ENTRIES = 0;

static QSilResManager * sResManager = nullptr;

QSilResManager::QSilResManager(const QString& companyName, const QString& productName, const QString& version)
	: QSettings(QSettings::UserScope, companyName, productName),
	mProductName(productName),
	mVersion(version),
	mSaveOnDestroy(true),
	mVersionModeOn(false)
{
	//set Name("QSilResManager");

	assert(!sResManager);
	sResManager = this;

	// If first construction of a QSilResManager object, initialise static data.
	if (!spWindowTracker)
	{
		//spWindowTracker = QSilWindowTracker::instance();
	}

	if (!spWindowKeys)
		spWindowKeys = new WindowKeyMap;

	// Ensure contents of static dict are deleted
	if (!sStoredMRUs)
	{
		sStoredMRUs = new QSilMRUMap;
		//QT4		sStoredMRUs->setAutoDelete(true);
	}


	if (mVersion.isEmpty())
	{
		/*?    static const DW_MakeVersion* make_version =
		DW_MakeVersion::getNonLibraryVersion();

		if (make_version && make_version->isProgram())
		mVersion = make_version->getVersionNumber();

		// Get just the major and minor version nos.  We don't want the revision
		mVersion = mVersion.section('.', 0, 1);*/
	}
	// Replace all spaces with underscores, as QSettings requires
	mProductName = mProductName.replace(" ", "_");

	//For Windows
	//QT4	insertSearchPath(QSettings::Windows, QString("/Silvaco"));
	//For Unix
	//QT4	insertSearchPath(QSettings::Unix, QDir::home().absPath() + "/.silvaco");

	//Save settings on a user basis
	// 
	// Adds the right prefix for all resources
	resetGroup();
}

QSilResManager::~QSilResManager()
{
	// Save all resources on exit
	if (mSaveOnDestroy)
		saveResources();

	//Delete all resources
	ResourceMap::const_iterator itr = mResources.constBegin();
	const ResourceMap::const_iterator itr_end = mResources.constEnd();
	while (itr != itr_end)
	{
		delete (*itr); // delete object
		++itr;
	}

	// Probably not required
	QSettings::endGroup();

	assert(sResManager == this);
	sResManager = nullptr;

	delete spWindowKeys;

	for (QSilMRUMap::iterator i(sStoredMRUs->begin()); i != sStoredMRUs->end(); i++)
		delete i.value();
	delete sStoredMRUs;
}

void QSilResManager::saveResources()
{
	// Save all settings if necesary
	ResourceMap::const_iterator itr = mResources.constBegin();
	const ResourceMap::const_iterator itr_end = mResources.constEnd();
	while (itr != itr_end)
	{
		QSilResourceBase* resource = *itr;

		// Only write "dirty" settings
		if (resource->isDirty())
		{
#ifdef _DEBUG
			//qDebug("Writing out dirty resource - %s", (*itr)->key().toLatin1());
#endif
			setVersionModeOn(resource->isVersioned());

			resource->write(this);
		}
		++itr;
	}
	setVersionModeOn(false);
}

QString QSilResManager::versionedRoot(QSettings::Format format)
{
	if (QSettings::NativeFormat == format)
	{
#ifdef WIN32
		return QString("/") + mVersion;
#else
		// Always have product name as prefix.  It dictates name of 
		// resource file on UNIX
		return QString("/") + mProductName + "/" + mVersion;
#endif
	}
	else
	{
		// Always have product name as prefix.  It dictates name of 
		// resource file
		return /*QString("/") + mProductName +*/ "/" + mVersion;
	}
}

QString QSilResManager::nonVersionedRoot(QSettings::Format format)
{
	if (QSettings::NativeFormat == format)
	{
#ifdef WIN32
		return QString("/All Versions");
#else
		// Always have product name as prefix.  It dictates name of 
		// resource file
		return QString("/") + mProductName + "/All Versions";
#endif
	}
	else
	{
		// Always have product name as prefix.  It dictates name of 
		// resource file
		return /*QString("/") + mProductName +*/ "/All Versions";
	}
}

void QSilResManager::setVersionModeOn(bool versioningOn)
{
	if (versioningOn != mVersionModeOn)
	{
		mVersionModeOn = versioningOn;

		// This function does the right thing
		resetGroup();
	}
}

QSilBoolResource* QSilResManager::newResource(const QString& key, bool defaultVal, bool isVersioned)
{
	// Check if resource already exists first
	QSilResourceBase* result = resourceByKey(key);

	if (!result)
	{
		result = new QSilBoolResource(key, defaultVal, isVersioned);

		registerResource(result);
	}

	return dynamic_cast<QSilBoolResource*>(result);
}

QSilIntResource* QSilResManager::newResource(const QString& key, int defaultVal, bool isVersioned)
{
	// Check if resource already exists first
	QSilResourceBase* result = resourceByKey(key);

	if (!result)
	{
		result = new QSilIntResource(key, defaultVal, isVersioned);

		registerResource(result);
	}

	return dynamic_cast<QSilIntResource*>(result);
}

QSilDoubleResource* QSilResManager::newResource(const QString& key, double defaultVal, bool isVersioned)
{
	// Check if resource already exists first
	QSilResourceBase* result = resourceByKey(key);

	if (!result)
	{
		result = new QSilDoubleResource(key, defaultVal, isVersioned);

		registerResource(result);
	}

	return dynamic_cast<QSilDoubleResource*>(result);
}

QSilStringResource* QSilResManager::newResource(const QString& key, const QString& defaultVal, bool isVersioned)
{
	// Check if resource already exists first
	QSilResourceBase* result = resourceByKey(key);

	if (!result)
	{
		result = new QSilStringResource(key, defaultVal, isVersioned);

		registerResource(result);
	}

	return dynamic_cast<QSilStringResource*>(result);
}

QSilStringListResource* QSilResManager::newResource(const QString& key, const QStringList& defaultVal, bool isVersioned)
{
	// Check if resource already exists first
	QSilResourceBase* result = resourceByKey(key);

	if (!result)
	{
		result = new QSilStringListResource(key, defaultVal, isVersioned);

		registerResource(result);
	}

	return dynamic_cast<QSilStringListResource*>(result);
}

QSilColorResource* QSilResManager::newResource(const QString& key, const QColor& defaultVal, bool isVersioned)
{
	// Check if resource already exists first
	QSilResourceBase* result = resourceByKey(key);

	if (!result)
	{
		result = new QSilColorResource(key, defaultVal, isVersioned);

		registerResource(result);
	}

	return dynamic_cast<QSilColorResource*>(result);
}

QSilFontResource* QSilResManager::newResource(const QString& key, const QFont& defaultVal, bool isVersioned)
{
	// Check if resource already exists first
	QSilResourceBase* result = resourceByKey(key);

	if (!result)
	{
		result = new QSilFontResource(key, defaultVal, isVersioned);

		registerResource(result);
	}

	return dynamic_cast<QSilFontResource*>(result);
}

void QSilResManager::registerResource(QSilResourceBase* resource)
{
	if (resource)
	{
		if (!mResources.contains(resource->key()))
		{
			mResources[resource->key()] = resource;

			// Initialise the resource by telling it to read from the file/registry
			setVersionModeOn(resource->isVersioned());

			resource->read(this);
			// Reset dirty flag - the first read is mandatory and initialises the
			// resource, therefore it is not dirty.
			resource->resetDirty();

			setVersionModeOn(false);
			// Add to list
			mResourceList.append(resource);
		}
		else
		{
			qDebug("Error in QSilResManager::registerResource().  A resource with the"
				" key %s already exists", resource->key().toLatin1().data());
		}
	}
}

QSilResourceBase* QSilResManager::resourceByKey(const QString& rKey) const
{
	if (mResources.contains(rKey))
		return mResources[rKey];
	else
		return 0;
}

void QSilResManager::loadWindowGeometry(QMainWindow* topLevel,
	const QString& rWinResName,
	const QString& rDisplayName)
{
	// Ensure required strings have sensible values
	QString win_res_name = rWinResName.isNull() ? QString(topLevel->objectName()) : rWinResName;
	QString win_display_name = rDisplayName.isNull() ? win_res_name : rDisplayName;

	(*spWindowKeys)[topLevel].first = win_res_name;
	(*spWindowKeys)[topLevel].second = win_display_name;

	if (!spToolbarSettings->contains(topLevel))
	{
		// Create a toolbar settings object for this window
		ToolbarSettings settings(this,
			topLevel,
			win_res_name,
			win_display_name);

		// Store the object
		spToolbarSettings->insert(topLevel, settings);
	}
	const ToolbarSettings& settings = (*spToolbarSettings)[topLevel];

	// Load the settings
	settings.load(topLevel);

	// Load widget geometry
	loadWidgetGeometry(topLevel, win_res_name, win_display_name);
}

void QSilResManager::loadWindowGeometry(QDialog* topLevel,
	const QString& rWinResName,
	const QString& rDisplayName)
{
	// Ensure required strings have sensible values
	QString win_res_name = rWinResName.isNull() ? QString(topLevel->objectName()) : rWinResName;
	QString win_display_name = rDisplayName.isNull() ? win_res_name : rDisplayName;

	loadWidgetGeometry(topLevel, win_res_name, win_display_name);
}

void QSilResManager::saveWindowGeometry(QMainWindow* topLevel)
{
	if (spToolbarSettings->contains(topLevel))
	{
		// Find the toolbar settings object for this window
		ToolbarSettings& settings = (*spToolbarSettings)[topLevel];

		// Load the settings
		settings.save(topLevel);
	}
	else
	{
		qDebug("No settings for this widget.");
	}
	// load widget settings
	saveWidgetGeometry(topLevel);
}

void QSilResManager::saveWindowGeometry(QDialog* topLevel)
{
	saveWidgetGeometry(topLevel);
}

void QSilResManager::loadWidgetGeometry(QWidget* topLevel,
	const QString& rWinResName,
	const QString& rDisplayName)
{
	if (!spGeomSettings->contains(topLevel))
	{
		// Monitor the lifetime of this object
		spWindowTracker->track(topLevel);

		// Create a toolbar settings object for this window
		GeomSettings settings(this, topLevel, rWinResName, rDisplayName);

		// Store the object
		spGeomSettings->insert(topLevel, settings);
	}
	GeomSettings& settings = (*spGeomSettings)[topLevel];
	// Load the settings
	settings.load(topLevel);
}

void QSilResManager::saveWidgetGeometry(QWidget* topLevel)
{
	if (spGeomSettings->contains(topLevel))
	{
		// Find the toolbar settings object for this window
		GeomSettings& settings = (*spGeomSettings)[topLevel];

		// Save the settings
		settings.save(topLevel);
	}
	else
	{
		qDebug("No settings for this widget.");
	}
}

QSilStringListResource* QSilResManager::resourceForAction(QMainWindow* win, QAction* action)
{
	QString original_accel = action->shortcut().toString();
	QStringList original_accel_list;

	if (!original_accel.isEmpty())
		original_accel_list += original_accel;

	const QString PREFIX = QString("/%1/%2/").arg((*spWindowKeys)[win].first).arg("Shortcuts");
	const QString DISPLAY_PREFIX =
		QString("/%1/%2/").arg((*spWindowKeys)[win].second).arg(tr("Shortcuts"));

	QSilStringListResource* res = newResource(PREFIX + action->objectName(),
		original_accel_list);
	// We replace any / here with \\ because '/' is a special character and we need to
	// know in the import/export dialogs if it is literally intended.
	res->setDisplayName(DISPLAY_PREFIX + QString(action->toolTip()).replace("/", "\\\\"));

	return res;
}

template <class T>
static void getActionsForMenus(QMainWindow* /*?win*/, T* menuData, QList<QAction*>& /*rActions*/)
{
	if (!menuData)
		return;

	// Recurse through submenu
	/*?	for (uint i = 0; i < menuData->count(); i++)
		{
		int id = menuData->idAt(i);
		QAction* action = menuData->findItem(id); //QT4
		QMenu* sub_menu = action->menu();//null unless this is a sub menu

		if (!sub_menu && menuData->isItemVisible(id))
		{
		QAction* action = QSil::locateActionByMenuData(win, menuData, id);

		if (action)
		{
		rActions.append(action);
		}
		}
		else if (!dynamic_cast<QSilMRUPopupMenu*>(sub_menu))
		{
		// Recurse into submenus
		getActionsForMenus<QMenu>(win, sub_menu, rActions);
		}
		}*/
}

static const char NO_SHORTCUT[] = "NONE";

void QSilResManager::loadShortcuts(QMainWindow* topLevel,
	const QString& rWinResName,
	const QString& rDisplayName)
{
	// Ensure required strings have sensible values
	QString win_res_name = rWinResName.isNull() ? QString(topLevel->objectName()) : rWinResName;
	QString win_display_name = rDisplayName.isNull() ? win_res_name : rDisplayName;


	// Empty map - Delete all QSilAccels
	if (spShortcuts->contains(topLevel))
	{
		ShortcutMap& old_map = (*spShortcuts)[topLevel];
		ShortcutMap::iterator it = old_map.begin();
		const ShortcutMap::iterator it_end = old_map.end();
		while (it != it_end)
		{
			QSilAccel* accel = it.value();
			delete accel;
			++it;
		}
	}
	else
	{
		// Monitor the lifetime of this object
		spWindowTracker->track(topLevel);
	}

	//QObjectList* children = topLevel->queryList("QAction");
	QList<QAction*> actions;
	getActionsForMenus<QMenuBar>(topLevel, topLevel->menuBar(), actions);
	QAction* action = 0;
	ShortcutMap shortcut_map;

	QMap<QString, QAction*> used_shortcuts;// Prevents duplicates

	// First log all shortcuts that were defined for the actions by default (in Designer)
	const QList<QAction*>::const_iterator itr_end(actions.constEnd());
	QList<QAction*>::const_iterator itr(actions.constBegin());
	while (itr != itr_end)
	{
		action = *itr;
		QKeySequence key_sequence = action->shortcut();// Store first keystroke
		QKeySequence first_keystroke(key_sequence[0]);// Store first keystroke
		if (!first_keystroke.isEmpty() && !used_shortcuts.contains(first_keystroke.toString()))
			used_shortcuts[first_keystroke.toString()] = action;

		// Create QSilAccel to store new shortcuts later on
		QString accel_name = QString(action->objectName()) + "_accels";
		QSilAccel* new_accel = new QSilAccel(action, topLevel, accel_name.toLatin1());
		shortcut_map[action] = new_accel;
		++itr;
	}

	// Now parse any user settings, resolving duplicates along the way
	itr = actions.constBegin();
	while (itr != itr_end)
	{
		action = *itr;
		QString original_accel = action->shortcut().toString();

		//QStringList key_sequence_list = readListEntry(PREFIX+"/"+action->name());
		QSilStringListResource* res = resourceForAction(topLevel, action);

		if (res)
		{
			QStringList key_sequence_list = res->value();

			const QStringList::ConstIterator itr_end = key_sequence_list.constEnd();
			for (QStringList::ConstIterator itr = key_sequence_list.constBegin();
				itr != itr_end;
				++itr)
			{
				QString key_sequence_str = *itr;

				if (key_sequence_str.isEmpty() || (key_sequence_str == NO_SHORTCUT))
				{
					// Clear any present accel
					//action->setAccel(QKeySequence());
					shortcut_map[action]->clear();
					break;
				}
				else
				{
					// ianw 3/31/05 handle bug when keysym contains comma keystrokes
					key_sequence_str.replace(QRegExp(",([,^]|$)"), ",,");

					QKeySequence key_sequence(key_sequence_str);// Convert to QKeySequence
					QKeySequence first_keystroke(key_sequence[0]); // Extract first keystroke

					// Remove accel from the action that it collides with if necessary
					if (used_shortcuts.contains(first_keystroke.toString()))
					{
						QAction* other_action = used_shortcuts[first_keystroke.toString()];

						// Update other action
						if (shortcut_map.contains(other_action))
						{
							//shortct_map[other_action]->removeItem(key_sequence);
							// Find the key sequence that matches the first keystroke, e.g.
							// "CTRL+X,CTRL+F" matches "CTRL+X" and is therefore a collision
							QSilAccel* victim = shortcut_map[other_action];
							if (victim->count() > 0)
							{
								for (uint n = 0; n < victim->count(); n++)
								{
#ifdef DEBUG
									//qDebug("checking key sequence %s.", ((QString)victim->key(n)).latin1());
#endif
									QKeySequence possible_match = victim->key(n);
									QKeySequence::SequenceMatch match = first_keystroke.matches(possible_match);
									if (QKeySequence::ExactMatch == match || QKeySequence::PartialMatch == match)
									{
										victim->removeItem(possible_match);

#ifdef DEBUG
										//qDebug("Removing shortcut %s from action %s.", ((QString)possible_match).latin1(), other_action->name());
#endif
										// Should only be one match, so break
										break;
									}
								}
							}
							else
							{
								other_action->setShortcut(QKeySequence());
							}
						}
					}

					if (shortcut_map.contains(action))
						shortcut_map[action]->insertItem(key_sequence);
				}
			}
		}
		++itr;
	}
	//delete children;
	// Store shortcut map
	(*spShortcuts)[topLevel] = shortcut_map;
}

void QSilResManager::saveShortcuts(QMainWindow* topLevel)
{
	if (spShortcuts->contains(topLevel))
	{
		const ShortcutMap& SHORTCUT_MAP = (*spShortcuts)[topLevel];

		QList<QAction*> actions;
		getActionsForMenus(topLevel, topLevel->menuBar(), actions);
		QAction* action = 0;
		ShortcutMap shortcut_map;

		QMap<QString, QAction*> used_shortcuts;// Prevents duplicates

		// First log all shortcuts that were defined for the actions by default (in Designer)
		const QList<QAction*>::const_iterator itr_end(actions.constEnd());
		QList<QAction*>::const_iterator itr(actions.constBegin());
		while (itr != itr_end)
		{
			action = *itr;
			// Store all key sequences in a qstringlist
			QStringList key_sequence_list;

			if (SHORTCUT_MAP.contains(action))
			{
				QSilAccel* accel = SHORTCUT_MAP[action];
				if (accel)
				{
					key_sequence_list = accel->getAccels();

					if (key_sequence_list.isEmpty() && accel->removed())
						key_sequence_list.append(NO_SHORTCUT);
				}
			}

			//(void)writeEntry(PREFIX+"/"+action->name(), key_sequence_list);
			QSilStringListResource* res = resourceForAction(topLevel, action);
			if (res)
			{
				// Remove users setting if it is empty (shortcut removed?)
				if (key_sequence_list.isEmpty())
					static_cast<QSilResourceBase*>(res)->reset(this);
				else
					res->setValue(key_sequence_list);
			}
			++itr;
		}
		//delete children;
	}
}

static const int RGB_MAX = 255;

void QSilResManager::resetGroup()
{
//?	QSettings::endGroup();

	if (mVersionModeOn)
		beginGroup(versionedRoot());
	else
		beginGroup(nonVersionedRoot());
}

bool QSilResManager::removeEntryRecursive(const QString& key)
{
	QString actual_key = key;
	QString initial_group = group();
	if (key.isNull())
	{
		QSettings::endGroup();// Go to the top of the tree
		if (mVersionModeOn)
			actual_key = versionedRoot();
		else
			actual_key = nonVersionedRoot();
	}

	QStringList entries = entryList(actual_key);
	QStringList::ConstIterator it = 0;
	for (it = entries.begin(); it != entries.end(); ++it)
	{
		if (!(*it).isEmpty())
		{
			QString entry = QString("/") + *it;

			beginGroup(actual_key);
			remove(entry);// Inherited method removes a single entry
			endGroup();
		}
	}
	QStringList sub_keys = subkeyList(actual_key);
	for (it = sub_keys.begin(); it != sub_keys.end(); ++it)
	{
		if (!(*it).isEmpty())
		{
			QString entry = QString("/") + *it;

			beginGroup(actual_key);

			removeEntryRecursive(entry);

			endGroup();
		}
	}
	// Remove this entry last
	bool result = contains(actual_key);
	if (result)
		remove(actual_key);
	if (key.isNull())
		beginGroup(initial_group);// Go back to initial group
	return result;
}

void QSilResManager::resetResources()
{
	// Save all settings if necesary
	ResourceMap::const_iterator itr = mResources.constBegin();
	const ResourceMap::const_iterator itr_end = mResources.constEnd();
	while (itr != itr_end)
	{
		QSilResourceBase* resource = *itr;
		setVersionModeOn(resource->isVersioned());

		resource->reset(this);

		++itr;
	}
	setVersionModeOn(false);

	// Attention. reloadShortcuts() should be called before reloadGeometries(),
	// otherwise reload is not in effect until restart of application.
	reloadShortcuts();
	// Now reload toolbars
	reloadGeometries();
	// Clear all mru menus to empty
	clearMrus();
}

void QSilResManager::reloadGeometries()
{
	if (!spGeomSettings)
		return;

	GeomSettingsMap::const_iterator itr = spGeomSettings->constBegin();
	const GeomSettingsMap::const_iterator itr_end = spGeomSettings->constEnd();

	while (itr != itr_end)
	{
		QMainWindow* mw = dynamic_cast<QMainWindow*>(itr.key());
		if (mw)
		{
			loadWindowGeometry(mw);
		}
		else
		{
			QDialog* dlg = dynamic_cast<QDialog*>(itr.key());
			if (dlg)
			{
				loadWindowGeometry(dlg);
			}
		}
		++itr;
	}
}

void QSilResManager::reloadShortcuts()
{
	//qDebug("Reload shortcuts");
	if (!spShortcuts)
		return;

	WindowShortcutMap::const_iterator itr = spShortcuts->constBegin();
	WindowShortcutMap::const_iterator itr_end = spShortcuts->constEnd();

	while (itr != itr_end)
	{
		loadShortcuts(itr.key());
		++itr;
	}
}

QAction *QSilResManager::createMruMenu(QMainWindow* topLevel, const QString& rMenuText,
	QObject *receiver, const char *member,
	QMenu *menu, QAction *before,
	int size,
	const QString& rWinResName, const QString& rDisplayName)
{
	// Ensure required strings have sensible values
	QString win_res_name = rWinResName.isNull() ? QString(topLevel->objectName()) : rWinResName;
	QString win_display_name = rDisplayName.isNull() ? win_res_name : rDisplayName;

	(*spWindowKeys)[topLevel].first = win_res_name;
	(*spWindowKeys)[topLevel].second = win_display_name;

	const QString PREFIX = QString("/%1/%2/").arg(win_res_name).arg("MRU");
	const QString DISPLAY_PREFIX = QString("/%1/%2/").arg(win_display_name).arg(tr("MRU"));

	// Remove any ampersand in menu text
	QString rText = rMenuText;
	rText = rText.remove('&');

	QString key((QString)PREFIX + "|" + rText);

	QSilMRU* mru = sStoredMRUs->value(key); //QT4
	if (!mru)
	{
		mru = new QSilMRU(0, rText, size);

		// Create resource if it doesn't exist
		QSilStringListResource* res = newResource(PREFIX + rText, QStringList());
		if (res)
		{
			res->setDisplayName(DISPLAY_PREFIX + rText);
			mru->m_MRU = res->value();
		}

		sStoredMRUs->insert(key, mru);
	}

	return mru->createMostRecentUsedMenu(receiver, member, menu, before);
}

void QSilResManager::updateMru(QMainWindow* topLevel, const QString& rTextDisplay, const QString& item)
{
	const QString PREFIX = QString("/%1/%2/").arg((*spWindowKeys)[topLevel].first).arg("MRU");

	// Remove any ampersand in menu text
	QString menu_text = rTextDisplay;
	menu_text = menu_text.remove('&');

	QString key((QString)PREFIX + "|" + menu_text);
	//std::string zo = key.toStdString();

	QSilMRU* mru = sStoredMRUs->value(key);
	if (mru)
	{
		mru->updateMostRecentUsedMenu(QDir::toNativeSeparators(item));

		QSilStringListResource* res =
			dynamic_cast<QSilStringListResource*>(resourceByKey(PREFIX + menu_text));
		if (res)
		{
			res->setValue(mru->m_MRU);
		}
	}
}

void QSilResManager::removeFromMru(QMainWindow* topLevel, const QString& text, const QString& item)
{
	const QString PREFIX = QString("/%1/%2/").arg((*spWindowKeys)[topLevel].first).arg("MRU");

	// Remove any ampersand in menu text
	QString menu_text = text;
	menu_text = menu_text.remove('&');

	QSilMRU* mru = sStoredMRUs->value((QString)PREFIX + "|" + menu_text);
	if (mru)
	{
		mru->removeFromMostRecentUsedMenu(QDir::toNativeSeparators(item));

		QSilStringListResource* res =
			dynamic_cast<QSilStringListResource*>(resourceByKey(PREFIX + menu_text));
		if (res)
		{
			res->setValue(mru->m_MRU);
		}
	}
}

void QSilResManager::setMruSize(QMainWindow* topLevel, const QString& text, int newSize)
{
	const QString PREFIX = QString("/%1/%2/").arg((*spWindowKeys)[topLevel].first).arg("MRU");

	// Remove any ampersand in menu text
	QString menu_text = text;
	menu_text = menu_text.remove('&');

	QSilMRU* mru = sStoredMRUs->value((QString)PREFIX + "|" + menu_text);
	if (mru)
	{
		mru->m_MaxMRU = newSize;
	}
}

// Clear all mru menus to empty
void QSilResManager::clearMrus()
{
	for (QSilMRUMap::Iterator itr(sStoredMRUs->begin()); itr != sStoredMRUs->end(); ++itr)
	{
		QSilMRU* mru = itr.value();
		// clear stringlist
		mru->m_MRU.clear();

		// clear all menus
		const QSilMRU::QSilMRUDataMap::const_iterator map_itr_end = mru->m_Data.constEnd();
		QSilMRU::QSilMRUDataMap::const_iterator map_itr = mru->m_Data.constBegin();
		while (map_itr != map_itr_end)
		{
			QSilMRUPopupMenu* menu = map_itr.key();
			if (menu)
				mru->clearAndUpdate(menu);

			++map_itr;
		}


	}
}

QString QSilResManager::getFileMru(QMainWindow* topLevel, const QString& text)
{
	const QString PREFIX = QString("/%1/%2/").arg((*spWindowKeys)[topLevel].first).arg("MRU");

	// Remove any ampersand in menu text
	QString menu_text = text;
	menu_text = menu_text.remove('&');

	QSilMRU* mru = sStoredMRUs->value((QString)PREFIX + "|" + menu_text);

	if (mru && mru->m_MRU.size())
		return mru->m_MRU[0];

	return "";
}

static void copySettings(QSettings& rIn,
	QSettings& rOut,
	const QStringList& rSubKeys,
	const QStringList& rEntries)
{
	QStringList::ConstIterator it = 0;
	for (it = rEntries.begin(); it != rEntries.end(); ++it)
	{
		if (!(*it).isEmpty())
		{
			bool ok = false;

			QString key = *it;

#ifdef WIN32
			// not all values can be read as strings
			{
				// It might be an int val from the windows registry
				// try this instead
				ok = rIn.contains(key);
				int num_val = rIn.value(key, 0).toInt();
				if (ok)
				{
					rOut.setValue(key, num_val);
				}
				else
				{
					// It might be a double val from the windows registry
					// try this instead
					ok = rIn.contains(key);
					double double_val = rIn.value(key, 0.0).toDouble();
					if (ok)
					{
						rOut.setValue(key, double_val);
					}
					else
					{
						// It might be a bool value from the windows registry
						// try this instead
						ok = rIn.contains(key);
						bool bool_val = rIn.value(key, false).toBool();
						if (ok)
							rOut.setValue(key, bool_val);
					}
				}
			}
#endif
			if (!ok)
			{
				ok = rIn.contains(key);
				QString val = rIn.value(key, QString()).toString();
				if (ok)
					rOut.setValue(key, val);
			}

			//qDebug("%s/%s\t%s", rIn.group().toLatin1(), key.toLatin1(), val.toLatin1());
			//status += rIn.group() + "/" + key + "\t" + val + "\n";
		}
	}
	for (it = rSubKeys.begin(); it != rSubKeys.end(); ++it)
	{
		if (!(*it).isEmpty())
		{
			QString entry = QString("/") + *it;
			rIn.beginGroup(entry);
			QStringList sub_keys = rIn.childKeys();
			QStringList entries = rIn.childGroups();//?entryList(entry);
			rIn.endGroup();
			rIn.beginGroup(entry);
			rOut.beginGroup(entry);
			// Recurse now
			copySettings(rIn, rOut, sub_keys, entries);
			rIn.endGroup();
			rOut.endGroup();
		}
	}
}

static bool canRead(QSettings& rIn, const QString& rKey)
{
	return rIn.contains(rKey);
}

bool QSilResManager::importResources(const QString& rFileName, const QSilResManager::ResourceList& rResources)
{
	QFileInfo file_info(rFileName);
	if (!file_info.exists() || !file_info.isReadable())
		return false;

	resetGroup();// Resets group prefix to /productname/version
/*#ifndef WIN32
	// Copy file to the naming convention that QSettings expects
	QString copy_file = "/tmp/" + mProductName.toLower() + "rc";
#else
	QString win_home(getenv("APPDATA"));

	if (win_home.isEmpty())
		win_home = QDir::homePath();

	QString copy_file = win_home + "/" + mProductName.toLower() + "rc";
#endif*/
/*	QFileInfo copy_file_info(copy_file);
	if (copy_file_info.exists() && !copy_file_info.isWritable())
		return false;*/

	// Copy input file to our special destination
	/*QT4		QUrlOperator url_op;
			QList<Q3NetworkOperation> net_ops = url_op.copy(rFileName, copy_file, false, false);
			Q3NetworkOperation* last_op = net_ops.getLast();
			while ((last_op->state() != Q3NetworkProtocol::StDone) &&
			(last_op->state() != Q3NetworkProtocol::StFailed))
			{
			// Must spin the event loop for the copy to work.
			qApp->processEvents(QEventLoop::ExcludeUserInput);
			}*/

	//const QString IN_ROOT = QString("/") + mProductName + "/All Versions";

	QSettings* pSettings = new QSettings(rFileName, QSettings::IniFormat);
	//pSettings->setPath(QString("silvaco.com"), mProductName, QSettings::UserScope);

	//QT4		pSettings->insertSearchPath(QSettings::Unix, "/tmp");// For UNIX

	ResourceList all_resources = mResources.values();
	ResourceList::const_iterator itr;
	ResourceList::const_iterator itr_end;
	int total = 0;

	if (rResources.isEmpty())
	{
		itr = all_resources.constBegin();
		itr_end = all_resources.constEnd();
		total = all_resources.count();
	}
	else
	{
		itr = rResources.constBegin();
		itr_end = rResources.constEnd();
		total = rResources.count();
	}
	int i = 0;
	while (itr != itr_end)
	{
		QSilResourceBase* resource = *itr;

#ifdef DEBUG	    
		//qDebug("Importing resource - %s", (*itr)->key().toLatin1());
#endif
		// Set version mode for this resource
		if (resource->isVersioned())
			pSettings->beginGroup(versionedRoot(QSettings::IniFormat));
		else
			pSettings->beginGroup(nonVersionedRoot(QSettings::IniFormat));

		resource->read(pSettings);
		emit resourceImported(++i, total); // emit signal
		// Reset group
		pSettings->endGroup();

		++itr;
	}

	// Deleting object ensures that settings are flushed to the file before we 
	// remove it
	delete pSettings;

	// Remove temporary file
/*	QDir dir;
	(void)dir.remove(copy_file);*/

	// Now reload all the shortcuts and toolbars
	reloadGeometries();
	reloadShortcuts();

	return true;
}

bool QSilResManager::exportResources(const QString& rFileName, const QSilResManager::ResourceList& rResources)
{
	QFileInfo file_info(rFileName);
	if (file_info.exists() && !file_info.isWritable())
		return false;

	QDir dir;
	(void)dir.remove(rFileName);//, true);// Remove original file if it exists

	//sync();  // CAUTION - undocumented Qt function, makes sure that the file is up to date

	//const QString IN_ROOT = group();
	//const QString OUT_ROOT = QString("/") + mProductName + "/All Versions";

	QSettings* pSettings = new QSettings(rFileName, QSettings::IniFormat);

/*#ifndef WIN32	
	//QT5	pSettings->insertSearchPath(QSettings::Unix, file_info.absolutePath());
	QString source = file_info.absolutePath() + "/" + mProductName.toLower() + "rc";
#else
	pSettings->setPath(QSettings::IniFormat, QSettings::UserScope, QString("silvaco.com"));//?, mProductName);
	// Doesn't work properly on Win32
	//pSettings->insertSearchPath(QSettings::Windows, file_info.absolutePath());

	// Do this instead
	QString win_home(getenv("APPDATA"));

	if (win_home.isEmpty())
		win_home = QDir::homePath();

	QString source = win_home + "/" + mProductName.toLower() + "rc";
#endif*/

	// Do this instead
	ResourceList all_resources = mResources.values();
	ResourceList::const_iterator itr;
	ResourceList::const_iterator itr_end;
	int total = 0;

	if (rResources.isEmpty())
	{
		itr = all_resources.constBegin();
		itr_end = all_resources.constEnd();
		total = all_resources.count();
	}
	else
	{
		itr = rResources.constBegin();
		itr_end = rResources.constEnd();
		total = rResources.count();
	}

	int i = 0;
	while (itr != itr_end)
	{
		QSilResourceBase* resource = *itr;

#ifdef DEBUG	    
		//qDebug("Exporting resource - %s", (*itr)->key().toLatin1());
#endif
		// Set version mode for this resource
		if (resource->isVersioned())
			pSettings->beginGroup(versionedRoot(QSettings::IniFormat));
		else
			pSettings->beginGroup(nonVersionedRoot(QSettings::IniFormat));

		resource->write(pSettings);
		emit resourceExported(++i, total); // emit signal
		// Reset group
		pSettings->endGroup();

		++itr;
	}

	// Deleting object ensures that settings are flushed to the file before we 
	// copy it
	delete pSettings;

	// Move file to required location

/*#ifndef WIN32
	return dir.rename(source, rFileName);
#else
	file_info.setFile(source);
	if (!file_info.exists())
	{
		// Because of some environment settings or something else 
		// it could be under this location instead of that one.
		// Now resources are correctly exported on Windows, in Debug environment.
		source = win_home
			+ QDir::separator() + "Silvaco"
			+ QDir::separator() + mProductName
			+ QDir::separator() + mProductName.toLower() + "rc";
		file_info.setFile(source);
	}
	if (!file_info.exists())
		return false;

	// Fix Linux style EOL on Windows
	QSilSedArea* sed_area = new QSilSedArea;
	bool success = sed_area->openDocument(source);
	if (success)
		success = sed_area->saveDocument(rFileName, CRLF_STYLE_DOS);
	delete sed_area;
	return success;
#endif*/
	return true;
}

inline QString normalizeKey(const QString& key)
{
	QString result = key;
	result.replace(QChar('\\'), QChar('/'));
	result.replace("//", QChar('/'));
	while (result.length() > 0 && result.at(0) == QChar('/'))
		result.remove(0, 1);
	while (result.length() > 0 && result.at(result.length() - 1) == QChar('/'))
		result.remove(result.length() - 1, 1);
	if (!result.isEmpty())
		result = "/" + result;
	return result;
}

inline QString normalizeKey(const QString& key1, const QString& key2)
{
	return normalizeKey(key1 + "/" + key2);
}

static void readKeys(QSettings& settings, const QString& masterKey, std::set<QString>& keys)
{
	{
		settings.beginGroup(masterKey);
		QStringList entries = settings.childKeys();
		settings.endGroup();
		QStringList::ConstIterator it_end = entries.constEnd();
		for (QStringList::ConstIterator it = entries.constBegin(); it != it_end; ++it)
			keys.insert(normalizeKey(masterKey, *it));
	}
	{
		settings.beginGroup(masterKey);
		QStringList subKeys = settings.childKeys();
		settings.endGroup();
		QStringList::ConstIterator it_end = subKeys.constEnd();
		for (QStringList::ConstIterator it = subKeys.constBegin(); it != it_end; ++it)
			readKeys(settings, normalizeKey(masterKey, *it), keys);
	}
}

void QSilResManager::resourcesInFile(const QString& rFileName, ResourceList& rResources)
{
	// Clear return value
	rResources.clear();

	QFileInfo file_info(rFileName);
	if (file_info.exists() && file_info.isReadable())
	{
		resetGroup();// Resets group prefix to /productname/version
#ifdef Q_WS_X11
		// Copy file to the naming convention that QSettings expects
		QString copy_file = "/tmp/" + mProductName.toLower() + "rc";
#else
		QString win_home(getenv("APPDATA"));

		if (win_home.isEmpty())
			win_home = QDir::homePath();

		QString copy_file = win_home + "/" + mProductName.toLower() + "rc";
#endif	
		QFileInfo copy_file_info(copy_file);

		if (copy_file_info.exists() && !copy_file_info.isWritable())
			return;

		// Copy input file to our special destination
		/*?		Q3UrlOperator url_op;
				Q3PtrList<Q3NetworkOperation> net_ops = url_op.copy(rFileName, copy_file, false, false);
				Q3NetworkOperation* last_op = net_ops.getLast();
				while ((last_op->state() != Q3NetworkProtocol::StDone) &&
				(last_op->state() != Q3NetworkProtocol::StFailed))
				{
				// Must spin the event loop for the copy to work.
				qApp->processEvents(QEventLoop::ExcludeUserInput);
				}*/

		QSettings* pSettings = new QSettings(QString(), QSettings::IniFormat);//?

#ifdef Q_WS_X11	
		//?pSettings->insertSearchPath(QSettings::Unix, "/tmp");// For UNIX
#else
		//?		pSettings->setPath(QString("silvaco.com"), mProductName, QSettings::UserScope);
#endif

		ResourceList all_resources = mResources.values();
		ResourceList::const_iterator itr = all_resources.constBegin();
		const ResourceList::const_iterator itr_end = all_resources.constEnd();

		// read versioned keys into map
		std::set<QString> versionedKeys;
		pSettings->beginGroup(versionedRoot(QSettings::IniFormat));
		readKeys(*pSettings, QString(), versionedKeys);
		pSettings->endGroup();

		// read non-versioned keys into map
		std::set<QString> nonVersionedKeys;
		pSettings->beginGroup(nonVersionedRoot(QSettings::IniFormat));
		readKeys(*pSettings, QString(), nonVersionedKeys);
		pSettings->endGroup();

		while (itr != itr_end)
		{
			QSilResourceBase* resource = *itr;
			QString key = normalizeKey(resource->key());

			// If the key exists in this file, add it to the list
			if (
				(resource->isVersioned() && versionedKeys.find(key) != versionedKeys.end()) ||
				(!resource->isVersioned() && nonVersionedKeys.find(key) != nonVersionedKeys.end())
				)
				rResources.append(resource);

			++itr;
		}

		delete pSettings;

		// Remove temporary file
		QDir dir;
		(void)dir.remove(copy_file);
	}
}


// Don't document
QSilResManager::ShortcutMap& QSilResManager::getShortcutMap(QMainWindow* win)
{
	return (*spShortcuts)[win];
}

bool QSilResManager::setAccelsForAction(QAction* action, const QStringList& rAccels)
{
	QSilAccel* accel = 0;

	// Go through all windows and find action.
	WindowShortcutMap::iterator itr = spShortcuts->begin();
	const WindowShortcutMap::iterator itr_end = spShortcuts->end();
	QMainWindow* win = 0;

	while (itr != itr_end)
	{
		ShortcutMap& shortcut_map = *itr;

		if (shortcut_map.contains(action))
		{
			win = itr.key();
			accel = shortcut_map[action];
			break;
		}

		++itr;
	}

	if (accel)
	{
		// Find QSilAccel object
		accel->clear();

		const QStringList::ConstIterator itr_end = rAccels.constEnd();

		for (QStringList::ConstIterator itr = rAccels.constBegin();
			itr != itr_end;
			itr++)
		{
			QString key_sequence_str = *itr;
			// ianw 3/31/05 handle bug when keysym contains comma keystrokes
			key_sequence_str.replace(QRegExp(",([,^]|$)"), ",,");

			QKeySequence key_seq(key_sequence_str);
			if (!key_seq.isEmpty())
				accel->insertItem(QKeySequence(key_seq));
		}

		QStringList accels = rAccels;
		if (accels.isEmpty())
		{
			// Explicitly mark accel as removed
			accels.append(NO_SHORTCUT);
		}

		// Set the resource values for the new shortcuts
		QSilStringListResource* res = resourceForAction(win, action);
		if (res)
		{
			res->setValue(rAccels);
		}

		return true;
	}
	// Accel not found in map
	return false;
}

QStringList QSilResManager::accelsForAction(QAction* action) const
{
	QSilAccel* accel = 0;
	QStringList result;

	// Go through all windows and find action.
	WindowShortcutMap::iterator itr = spShortcuts->begin();
	const WindowShortcutMap::iterator itr_end = spShortcuts->end();

	while (itr != itr_end)
	{
		ShortcutMap& shortcut_map = *itr;

		if (shortcut_map.contains(action))
		{
			accel = shortcut_map[action];
			break;
		}

		++itr;
	}

	if (accel)
		result = accel->getAccels();
	else
		result += action->shortcut().toString();

	return result;
}

QStringList QSilResManager::entryList(const QString& key)
{
	// Call inherited
	beginGroup(key);
	QStringList l(QSettings::childKeys());//?
	endGroup();
	return l;
}

QStringList QSilResManager::subkeyList(const QString& key)
{
	// Call inherited
	beginGroup(key);
	QStringList l(QSettings::childKeys());
	endGroup();
	return l;
}

bool QSilResManager::keyExists(const QString& key)
{
	return canRead(*this, key);
}

bool QSilResManager::importFromRoot(const QString& rNewRoot)
{
	Q_ASSERT(!rNewRoot.isEmpty());

	bool success = false;

	QString old_group = group();

	QSettings::endGroup();
#ifdef WIN32
	// Set registry path for Windows
	QString first_section = rNewRoot.section('/',
		0, 0,
		QString::SectionSkipEmpty);
	// Do what we did in the constructor - required
	first_section = first_section.replace(" ", "_");

	QString second_section = rNewRoot.section('/',
		1, 1,
		QString::SectionSkipEmpty);

#ifdef _DEBUG
	qDebug("In importFromRoot(), First section = %s, second section = %s",
		first_section.toLatin1(),
		second_section.toLatin1());
#endif


	//?	setPath(QString("silvaco.com"), first_section, QSettings::UserScope);
	if (!second_section.isEmpty())
		beginGroup(QString("/") + second_section);
#else
	beginGroup(rNewRoot);
#endif
	// Read all resources now
	ResourceMap::const_iterator itr = mResources.constBegin();
	const ResourceMap::const_iterator itr_end = mResources.constEnd();
	while (itr != itr_end)
	{
		QSilResourceBase* resource = *itr;
		if (keyExists((*itr)->key()))
		{
#ifdef _DEBUG
			qDebug("Reading resource from root - %s", (*itr)->key().toLatin1().data());
#endif
			//setVersionModeOn(resource->isVersioned());

			resource->read(this);
			success = true;
		}
		++itr;
	}
#ifdef WIN32
	// Reset registry path for Windows
	//	setPath(QString("silvaco.com"), mProductName, QSettings::UserScope);
#endif
	// Reset to old group
	beginGroup(old_group);

	return success;
}

// -----------------------------------------------------------------------------
// QSilWindowTracker Stuff
// -----------------------------------------------------------------------------

QSilWindowTracker* QSilWindowTracker::instance()
{
	// Create singleton object, owned by application object (so should die at shutdown).
	static QSilWindowTracker* window_tracker = new QSilWindowTracker(qApp, "Window destruction monitor");
	return window_tracker;
}

QSilWindowTracker::QSilWindowTracker(QObject* parent, const char* /*name*/) :
QObject(parent)// name)
{
	// Initialise static data
	spGeomSettings = new GeomSettingsMap;
	spToolbarSettings = new WindowToolbarSettingsMap;
	//spWindowKeys = new WindowKeyMap;

	spShortcuts = new WindowShortcutMap;

	//qDebug("Creating shared shortcut/toolbar maps.");
}

QSilWindowTracker::~QSilWindowTracker()
{
	// delete static data
	delete spGeomSettings;
	delete spToolbarSettings;
	delete spWindowKeys;

	delete spShortcuts;

	//qDebug("Destroying shared shortcut/toolbar maps.");
}

void QSilWindowTracker::track(QWidget* win)
{
	// Disconnect in case already connected
	//bool was_connected = win->disconnect(this);
	win->removeEventFilter(this);

	connect(win, SIGNAL(destroyed(QObject*)), SLOT(handleWidgetDestroyed(QObject*)));

	if (dynamic_cast<QMainWindow*>(win))
		connect(win, SIGNAL(destroyed(QObject*)), SLOT(handleWindowDestroyed(QObject*)));

	win->installEventFilter(this);

	/*
	if (was_connected)
	qDebug("QSilWindowTracker adding lifetime tracker to window %s", win->name());
	*/
}

bool QSilWindowTracker::eventFilter(QObject* watched, QEvent* e)
{
	if (QEvent::Close == e->type() ||
		QEvent::Resize == e->type() ||
		QEvent::Move == e->type() ||
		(QEvent::Hide == e->type() && watched->inherits("QDialog")))
	{
		QMainWindow* win = dynamic_cast<QMainWindow*>(watched);
		QDialog* dlg = dynamic_cast<QDialog*>(watched);
		if (win)
		{
			// Save window geometry
			sResManager->saveWindowGeometry(win);
		}
		else if (dlg)
		{
			// Save dialog geometry
			sResManager->saveWindowGeometry(dlg);
		}
	}

	// Call inherited
	return QObject::eventFilter(watched, e);
}

void QSilWindowTracker::handleWidgetDestroyed(QObject* o)
{
	// Can't use dynamic_cast as object is being destructed and it might not work
	QWidget* widget = static_cast<QWidget*>(o);
	// Remove from maps
	if (widget)
	{
		spWindowKeys->remove(widget);
		spGeomSettings->remove(widget);
	}
}

void QSilWindowTracker::handleWindowDestroyed(QObject* o)
{
	// Can't use dynamic_cast as object is being destructed and it might not work
	QMainWindow* win = static_cast<QMainWindow*>(o);
	// Remove from maps
	if (win)
	{
		spShortcuts->remove(win);
		spToolbarSettings->remove(win);
	}
}
