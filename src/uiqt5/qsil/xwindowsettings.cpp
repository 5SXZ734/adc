
#include <QtCore/QTextStream>
#include <QtCore/QList>
#include <QtCore/QObjectList>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QSplitter>
#include <QToolTip>
#include <QToolBar>
#include <QShortcut>

#include "xwindowsettings.h"
#include "xresmgr.h"
#include "xresource.h"

GeomSettings::GeomSettings() :
	mpXRes(0),
	mpYRes(0),
	mpWidthRes(0),
	mpHeightRes(0)
{
}

GeomSettings::GeomSettings(QSilResManager* resMgr, QWidget* widget, const QString& rKey, const QString& rDisplayName)
{
	QString PREFIX = QString("/") + rKey + "/";
	QString DISPLAY_PREFIX = QString("/") + rDisplayName + "/";

	QDesktopWidget desktop;
	const QRect& SCREEN_GEOM = desktop.screenGeometry(widget);

	mpXRes = resMgr->newResource(PREFIX + "x", double(widget->x()) / SCREEN_GEOM.width(), true);
	mpXRes->setDisplayName(DISPLAY_PREFIX + QSilResManager::tr("x"));
	mpYRes = resMgr->newResource(PREFIX + "y", double(widget->y()) / SCREEN_GEOM.height(), true);
	mpYRes->setDisplayName(DISPLAY_PREFIX + QSilResManager::tr("y"));
	mpWidthRes = resMgr->newResource(PREFIX + "width", double(widget->width()) / SCREEN_GEOM.width(), true);
	mpWidthRes->setDisplayName(DISPLAY_PREFIX + QSilResManager::tr("width"));
	mpHeightRes = resMgr->newResource(PREFIX + "height", double(widget->height()) / SCREEN_GEOM.height(), true);
	mpHeightRes->setDisplayName(DISPLAY_PREFIX + QSilResManager::tr("height"));


	// Create resources for any splitters
	DISPLAY_PREFIX = DISPLAY_PREFIX + QSilResManager::tr("Splitters") + "/";
	PREFIX = PREFIX + "Splitters/";

	QList<QSplitter*> splitters = widget->findChildren<QSplitter*>();
	for (QList<QSplitter*>::iterator it(splitters.begin()); it != splitters.end(); it++)
	{
		QSplitter* splitter(*it);
		QString data;
		QTextStream stream(&data, QIODevice::WriteOnly);
		stream << *splitter->saveState();
		QSilStringResource* res = resMgr->newResource(PREFIX + splitter->objectName(), data, true);
		res->setDisplayName(DISPLAY_PREFIX + splitter->objectName());
		mSplittersRes.insert(splitter->objectName(), res);
	}

	//delete list
//?	delete splitters;
}

GeomSettings::GeomSettings(const GeomSettings& that) :
	mpXRes(that.mpXRes),
	mpYRes(that.mpYRes),
	mpWidthRes(that.mpWidthRes),
	mpHeightRes(that.mpHeightRes),
	mSplittersRes(that.mSplittersRes)
{

}

const GeomSettings& GeomSettings::operator=(const GeomSettings& that)
{
	mpXRes = that.mpXRes;
	mpYRes = that.mpYRes;
	mpWidthRes = that.mpWidthRes;
	mpHeightRes = that.mpHeightRes;
	mSplittersRes = that.mSplittersRes;

	return *this;
}

void GeomSettings::load(QWidget* widget) const
{
	//Adjust window
	QDesktopWidget desktop;
	const QRect& SCREEN_GEOM = desktop.screenGeometry(widget);

	double virtual_x = mpXRes->value();
	double virtual_y = mpYRes->value();
	double virtual_width = mpWidthRes->value();
	double virtual_height = mpHeightRes->value();

	if (virtual_x == -1)
		widget->setWindowState(Qt::WindowMaximized);
	else
	{
		widget->setWindowState(Qt::WindowNoState);
		if ((virtual_x > -1) && (virtual_y > -1))
			widget->move(int(virtual_x * SCREEN_GEOM.width()),
				int(virtual_y * SCREEN_GEOM.height()));

		if ((virtual_width > -1) && (virtual_height > -1))
			widget->resize(int(virtual_width * SCREEN_GEOM.width()),
				int(virtual_height * SCREEN_GEOM.height()));
	}

	// Restore splitters
	QList<QSplitter*> splitters = widget->findChildren<QSplitter*>();

	for (QList<QSplitter*>::iterator it(splitters.begin()); it != splitters.end(); it++)
	{
		QSplitter* splitter(*it);

		QSilStringResource* res = mSplittersRes[splitter->objectName()];
		if (res)
		{
			QString data = res->value();
			QTextStream stream(&data, QIODevice::ReadOnly);
			QByteArray state;
			stream >> state;
			splitter->restoreState(state);
		}
	}

	//delete list
//?	delete splitters;
}

void GeomSettings::save(QWidget* widget)
{
	QDesktopWidget desktop;

	const QPoint& POS = widget->pos();
	const QSize& SIZE = widget->size();
	const QRect& SCREEN_GEOM = desktop.screenGeometry(widget);

	double virtual_x = double(POS.x()) / SCREEN_GEOM.width();
	double virtual_y = double(POS.y()) / SCREEN_GEOM.height();
	double virtual_width = double(SIZE.width()) / SCREEN_GEOM.width();
	double virtual_height = double(SIZE.height()) / SCREEN_GEOM.height();

	if (widget->isMaximized())
		virtual_x = -1;

	//Write settings
	mpXRes->setValue(virtual_x);
	mpYRes->setValue(virtual_y);
	mpWidthRes->setValue(virtual_width);
	mpHeightRes->setValue(virtual_height);

	QList<QSplitter*> splitters = widget->findChildren<QSplitter*>();
	for (QList<QSplitter*>::iterator it(splitters.begin()); it != splitters.end(); it++)
	{
		QSplitter* splitter(*it);
		QSilStringResource* res = mSplittersRes[splitter->objectName()];
		if (res)
		{
			QString data;
			QTextStream stream(&data, QIODevice::WriteOnly);
			stream << *splitter->saveState();
			res->setValue(data);
		}
	}

	//delete list
//?	delete splitters;
}


ToolbarSettings::ToolbarSettings() :
	mpLargeIconsRes(0),
	mpDisplayTextRes(0),
	mpDisplayHintsRes(0),
	mpLayoutsRes(0)
{
}

bool QToolTip__isGloballyEnabled() { return true; }

ToolbarSettings::ToolbarSettings(QSilResManager* resMgr, QMainWindow* widget, const QString& rKey, const QString& rDisplayName)
{
	QString PREFIX = QString("/%1/%2/").arg(rKey).arg("Toolbars");
	QString DISPLAY_PREFIX = QString("/%1/%2/").arg(rDisplayName).arg(QSilResManager::tr("Toolbars"));

	//?mpLargeIconsRes = resMgr->newResource(PREFIX + "Large icons", widget->usesBigPixmaps(), true);
	//mpLargeIconsRes->setDisplayName(DISPLAY_PREFIX + QSilResManager::tr("Large icons"));
	mpDisplayTextRes = resMgr->newResource(PREFIX + "Text labels", widget->toolButtonStyle() == Qt::ToolButtonTextUnderIcon, true);
	mpDisplayTextRes->setDisplayName(DISPLAY_PREFIX + QSilResManager::tr("Text labels"));
	mpDisplayHintsRes = resMgr->newResource(PREFIX + "Display hints", QToolTip__isGloballyEnabled(), true);
	mpDisplayHintsRes->setDisplayName(DISPLAY_PREFIX + QSilResManager::tr("Display hints"));

	PREFIX += "Toolbar/";
	DISPLAY_PREFIX += QSilResManager::tr("Toolbar") + "/";
	// Store toolbar contents
	for (int n = 0; n < PLACE_MAX; n++)
	{
		QList<QToolBar*> tool_bars;//? = widget->toolBars(QSil::DOCK_PLACES[n]);

		for (QList<QToolBar*>::iterator it(tool_bars.begin()); it != tool_bars.end(); it++)
		{
			QToolBar* toolbar(*it);
			//Store original toolbar contents
			QStringList current = QSil::getToolbarContents(toolbar);

			QSilStringListResource* res = resMgr->newResource(PREFIX + toolbar->windowTitle(), current, true);
			res->setDisplayName(DISPLAY_PREFIX + toolbar->windowTitle());
			mContentsRes[toolbar->windowTitle()] = res;
		}
	}
	// Store window layout
	PREFIX = QString("/") + rKey + "/";
	DISPLAY_PREFIX = QString("/") + rDisplayName + "/";
	QString old_data;
	QTextStream out_stream(&old_data, QIODevice::WriteOnly);
	out_stream << widget->saveState();

	mpLayoutsRes = resMgr->newResource(PREFIX + "Dock Window Layouts", old_data, true);
	mpLayoutsRes->setDisplayName(DISPLAY_PREFIX + QSilResManager::tr("Dock Window Layouts"));
}

ToolbarSettings::ToolbarSettings(const ToolbarSettings& that) :
	mpLargeIconsRes(that.mpLargeIconsRes),
	mpDisplayTextRes(that.mpDisplayTextRes),
	mpDisplayHintsRes(that.mpDisplayHintsRes),
	mContentsRes(that.mContentsRes),
	mpLayoutsRes(that.mpLayoutsRes)
{
}

const ToolbarSettings& ToolbarSettings::operator=(const ToolbarSettings& that)
{
	mpLargeIconsRes = that.mpLargeIconsRes;
	mpDisplayTextRes = that.mpDisplayTextRes;
	mpDisplayHintsRes = that.mpDisplayHintsRes;
	mContentsRes = that.mContentsRes;
	mpLayoutsRes = that.mpLayoutsRes;

	return *this;
}

void ToolbarSettings::load(QMainWindow* widget) const
{
	//?	widget->setUsesBigPixmaps(mpLargeIconsRes->value());
	widget->setToolButtonStyle(mpDisplayTextRes->value() ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
	//?	QToolTip_setGloballyEnabled(mpDisplayHintsRes->value());

		// update toolbar contents
	for (int n = 0; n < PLACE_MAX; n++)
	{
		QList<QToolBar*> tool_bars;//? = widget->toolBars(QSil::DOCK_PLACES[n]);

		for (QList<QToolBar*>::iterator it(tool_bars.begin()); it != tool_bars.end(); it++)
		{
			QToolBar* toolbar(*it);
			if (mContentsRes.contains(toolbar->windowTitle()))
			{
				QStringList toolbar_contents = mContentsRes[toolbar->windowTitle()]->value();
				QSil::setToolbarContents(toolbar, widget, toolbar_contents);
			}
		}
	}

	// Update window layout
	QString new_data = mpLayoutsRes->value();
	QTextStream in_stream(&new_data, QIODevice::ReadOnly);
	QByteArray state;
	in_stream >> state;
	widget->restoreState(state);
}

void ToolbarSettings::save(QMainWindow* widget)
{
	//?	mpLargeIconsRes->setValue(widget->usesBigPixmaps());
	mpDisplayTextRes->setValue(widget->toolButtonStyle() == Qt::ToolButtonTextUnderIcon);
	mpDisplayHintsRes->setValue(QToolTip__isGloballyEnabled());

	// save toolbar contents
	for (int n = 0; n < PLACE_MAX; n++)
	{
		QList<QToolBar*> tool_bars;//? = widget->toolBars(QSil::DOCK_PLACES[n]);

		for (QList<QToolBar*>::iterator it(tool_bars.begin()); it != tool_bars.end(); it++)
		{
			QToolBar* toolbar(*it);
			if (mContentsRes.contains(toolbar->windowTitle()))
			{
				QStringList toolbar_contents = QSil::getToolbarContents(toolbar);
				mContentsRes[toolbar->windowTitle()]->setValue(toolbar_contents);
			}
		}
	}

	// Save window layout
	QString data;
	QTextStream out_stream(&data, QIODevice::WriteOnly);
	out_stream << widget->saveState();

	mpLayoutsRes->setValue(data);
}

/////////////////////////////////////////////////////////////////////////////
// QSilAccel Stuff
/////////////////////////////////////////////////////////////////////////////

QSilAccel::QSilAccel(QAction* action, QWidget* parent, const char* /*name*/) :
	QObject(parent),//, name),
	mpAction(action),
	mMainAccel(action->shortcut()),
	mRemoved(false),
	mpParentWidget(0)
{
	// Error action is null
	Q_ASSERT(mpAction);

	// Find parent widget so that shortcuts are created with the correct
	// parameters in insertItem()
	QObject* prev = mpAction;
	QObject* parent_object = prev;
	while ((prev = prev->parent()) != 0)
	{
		parent_object = prev;
	}
	mpParentWidget = dynamic_cast<QWidget*>(parent_object);
	//#ifdef DEBUG
#if 0
	if (mpParentWidget)
	{
		qDebug() << "QSil accel parent widget = " <<
			mpParentWidget->objectName();
	}
#endif
}

QSilAccel::~QSilAccel()
{
	//qDeleteAll(mShortcuts);
	//clear();
}

void QSilAccel::insertItem(const QKeySequence& key)
{
	if (0 == count())
	{
		mMainAccel = key;
		//mpAction->setShortcut(mMainAccel);
	}
	else
	{
		QShortcut* shortcut = new QShortcut(key, mpParentWidget);
		shortcut->setContext(mpAction->shortcutContext());

		//mpAction->connect(shortcut, SIGNAL(activated()), SLOT(trigger()));

		mShortcuts.append(shortcut);
	}
	connect();
}
void QSilAccel::removeItem(const QKeySequence& key)
{
	if (key == mMainAccel)
	{
		if (mShortcuts.count() > 0)
		{
			QShortcut* shortcut = mShortcuts.takeFirst();

			mMainAccel = shortcut->key();
			mpAction->setShortcut(mMainAccel);
			delete shortcut;
		}
		else
		{
			mMainAccel = QKeySequence();
			mpAction->setShortcut(mMainAccel);
		}
	}
	else
	{
		const QList<QShortcut*>::Iterator itr_end = mShortcuts.end();
		QList<QShortcut*>::Iterator itr = mShortcuts.begin();

		while (itr != itr_end)
		{
			if (key == (*itr)->key())
			{
				delete (*itr);
				(void)mShortcuts.erase(itr);
				break;
			}
			++itr;
		}
	}
}

void QSilAccel::clear()
{
	mMainAccel = QKeySequence();
	//mpAction->setShortcut(mMainAccel);

	qDeleteAll(mShortcuts);
	mShortcuts.clear();
}

void QSilAccel::connect()
{
	mpAction->setShortcut(mMainAccel);// Main accel
	for (int n = 0; n < mShortcuts.count(); n++)
	{
		mShortcuts[n]->disconnect(this); // ensure this is disconnected first
		mpAction->connect(mShortcuts[n], SIGNAL(activated()), SLOT(trigger()));
		mShortcuts[n]->setEnabled(true);
	}
}

void QSilAccel::disconnect()
{
	mpAction->setShortcut(QKeySequence());// Empty accel
	for (int n = 0; n < mShortcuts.count(); n++)
	{
		mShortcuts[n]->disconnect(this);
	}
}

QStringList QSilAccel::getAccels()
{
	QStringList result;

	if (count() >= 1)
		result.push_back(mMainAccel.toString());

	const QList<QShortcut*>::ConstIterator itr_end = mShortcuts.constEnd();
	QList<QShortcut*>::ConstIterator itr = mShortcuts.constBegin();

	while (itr != itr_end)
	{
		result.push_back((*itr)->key().toString());
		++itr;
	}

	return result;
}

/*
void QSilAccel::setAccels(const QStringList& rAccels)
{
	clear();// Clear all present accels
	mpAction->setShortcut(QKeySequence());

	for (int n = 0; n < rAccels.count(); ++n)
	{
		QString key_seq = comma_bugfix(rAccels[n]);
		insertItem(QKeySequence(key_seq));
	}
}
*/

uint QSilAccel::count() const
{
	if (mMainAccel.isEmpty())
		return 0;
	else
		return mShortcuts.count() + 1;// +1 for the shortcut belonging to the action
}

QKeySequence QSilAccel::key(int index)
{
	Q_ASSERT(index >= 0 && index < (int)count());
	if (0 == index)
		return mMainAccel;
	else
		return mShortcuts[index - 1]->key();
}

