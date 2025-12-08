
#ifndef QSIL_RESMGR_H
#define QSIL_RESMGR_H

//#include <QAccel>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtCore/QMap>
#include <QMenu>
#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>

#ifdef WIN32
#pragma warning(disable: 4251)
#endif

class QAction;
class QDialog;
class QMainWindow;
class QWidget;
class QToolBar;

class QSilAccel;

class QSilResourceBase;
class QSilBoolResource;
class QSilIntResource;
class QSilDoubleResource;
class QSilStringResource;
class QSilStringListResource;
class QSilColorResource;
class QSilFontResource;

class QSilResManager : public QSettings
{
	Q_OBJECT
public:
	static const QString USE_WINDOW_NAME;
	typedef QList<QSilResourceBase*> ResourceList;

	~QSilResManager();
	void loadWindowGeometry(QMainWindow* topLevel,
		const QString & = USE_WINDOW_NAME,
		const QString & = USE_WINDOW_NAME);
	void loadWindowGeometry(QDialog* topLevel,
		const QString & = USE_WINDOW_NAME,
		const QString & = USE_WINDOW_NAME);

	void loadShortcuts(QMainWindow* topLevel,
		const QString & = USE_WINDOW_NAME,
		const QString & = USE_WINDOW_NAME);

	QAction* createMruMenu(QMainWindow* topLevel,
		const QString& rMenuText,
		QObject* receiver,
		const char* member,
		QMenu* menu,
		QAction* before,
		int size = 4,
		const QString & = USE_WINDOW_NAME,
		const QString & = USE_WINDOW_NAME);

	void setMruSize(QMainWindow*, const QString&, int);
	void updateMru(QMainWindow*, const QString&, const QString&);
	void removeFromMru(QMainWindow*, const QString&, const QString&);
	QString getFileMru(QMainWindow*, const QString&);


	QSilBoolResource* newResource(const QString&, bool, bool = false);
	QSilIntResource* newResource(const QString&, int, bool = false);
	QSilDoubleResource* newResource(const QString&, double, bool = false);
	QSilStringResource* newResource(const QString&, const QString&, bool = false);
	QSilStringListResource* newResource(const QString&, const QStringList&, bool = false);
	QSilColorResource* newResource(const QString&, const QColor&, bool = false);
	QSilFontResource* newResource(const QString&, const QFont&, bool = false);

	QSilResourceBase* resourceByKey(const QString&) const;
	void registerResource(QSilResourceBase*);

	bool setAccelsForAction(QAction*, const QStringList&);
	QStringList accelsForAction(QAction*) const;
	QStringList entryList(const QString&);
	QStringList subkeyList(const QString&);
	bool keyExists(const QString&);
	bool removeEntryRecursive(const QString & = ALL_ENTRIES);
	bool importFromRoot(const QString&);
	void saveResources();
	void saveWindowGeometry(QMainWindow*);
	void saveWindowGeometry(QDialog*);

	QSettings* settings() { return static_cast<QSettings*>(this); }

	typedef QMap<QAction*, QSilAccel*> ShortcutMap;
signals:
	void resourceImported(int, int);
	void resourceExported(int, int);
private slots:
	void setSaveOnDestroy(bool saveOnDestroy) { mSaveOnDestroy = saveOnDestroy; }
	bool importResources(const QString&, const QSilResManager::ResourceList&);
	bool exportResources(const QString&, const QSilResManager::ResourceList&);
	void resourcesInFile(const QString&, QSilResManager::ResourceList&);
public:
	// Disabled copy constructor and operator=
	QSilResManager(const QSilResManager&) = delete;
	QSilResManager& operator=(const QSilResManager&) = delete;
	QSilResManager(const QString&, const QString&, const QString & = QString());
private:
	static const char* ALL_ENTRIES;

    bool saveOnDestroy(){ return mSaveOnDestroy; }
    void resetResources();

	void resetGroup();

	void saveShortcuts(QMainWindow*);
	void reloadGeometries();
	void reloadShortcuts();

	void clearMrus();

	static ShortcutMap& getShortcutMap(QMainWindow*);

	void saveWidgetGeometry(QWidget*);
	void loadWidgetGeometry(QWidget*, const QString&, const QString&);

	QSilStringListResource* resourceForAction(QMainWindow*, QAction*);

	void setVersionModeOn(bool);
	bool isVersionModeOn() { return mVersionModeOn; }
	ResourceList resources() const { return mResourceList; }
	QString versionedRoot(QSettings::Format = QSettings::NativeFormat);
	QString nonVersionedRoot(QSettings::Format = QSettings::NativeFormat);

	QString mProductName;
	QString mVersion;
	bool mVersionModeOn;
	bool mSaveOnDestroy;
	typedef QMap<QString, QSilResourceBase*> ResourceMap;
	ResourceMap mResources; // All resources created in the app
	ResourceList mResourceList;
};


#endif /* QSIL_RESMGR_H */
