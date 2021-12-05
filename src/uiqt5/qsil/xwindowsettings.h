#ifndef QSIL_WINDOW_SETTINGS_H
#define QSIL_WINDOW_SETTINGS_H

#include <QAction>
#include <QShortcut>
//#include <QDict>
#include <QtCore/QMap>
#include "xresource.h"

class QMainWindow;
class QWidget;

class QSilBoolResource;
class QSilDoubleResource;
class QSilResManager;
class QSilStringResource;
class QSilStringListResource;


//Private class for saving geometry settings
class GeomSettings
{
public:
    GeomSettings();
    ~GeomSettings(){}
    GeomSettings(QSilResManager*, QWidget*, const QString&, const QString&);
    GeomSettings(const GeomSettings&);
    const GeomSettings& operator=(const GeomSettings&);

    void load(QWidget* widget) const;
    void save(QWidget* widget);
private:
    QSilDoubleResource* mpXRes;
    QSilDoubleResource* mpYRes;
    QSilDoubleResource* mpWidthRes;
    QSilDoubleResource* mpHeightRes;
    QHash<QString, QSilStringResource*> mSplittersRes;
};

//Private class for saving toolbar settings
class ToolbarSettings
{
public:
    ToolbarSettings();
    ~ToolbarSettings(){}
    ToolbarSettings(QSilResManager*, QMainWindow*, const QString&, const QString&);
    ToolbarSettings(const ToolbarSettings&);
    const ToolbarSettings& operator=(const ToolbarSettings&);

    void load(QMainWindow* widget) const;
    void save(QMainWindow* widget);
private:
    QSilBoolResource* mpLargeIconsRes;
    QSilBoolResource* mpDisplayTextRes;
    QSilBoolResource* mpDisplayHintsRes;    
    QMap<QString, QSilStringListResource*> mContentsRes;
    QSilStringResource* mpLayoutsRes;
};


// for private use only
class QSilAccel : protected QObject
{
public:
    QSilAccel(QAction*, QWidget*, const char* = 0);
    virtual ~QSilAccel();
    void clear();
    void insertItem(const QKeySequence& key);
    void removeItem(const QKeySequence& key);
    void connect();
    void disconnect();
    QStringList getAccels();
    QKeySequence key(int);
    uint count() const; //{ return mHighestId; }
    void setFullActionName(const QString& actName) { mFullActionName = actName; }
    const QString& fullActionName() const { return mFullActionName; }
    QAction* action(){ return mpAction; }
    void setRemoved(bool removed) { mRemoved = removed; }
    bool removed() const { return mRemoved; }
protected:
private:
    // Disabled copy constructor and operator=
    QSilAccel(const QSilAccel&);
    QSilAccel& operator=(const QSilAccel&);
    
	//int mHighestId;
    QAction* mpAction;
    QKeySequence mMainAccel;
    QString mFullActionName;
    bool mRemoved;
    QList<QShortcut*> mShortcuts;
    QWidget* mpParentWidget;
};

// for private use only, singleton object
class QSilWindowTracker : public QObject
{
Q_OBJECT
public:
    virtual ~QSilWindowTracker();
    static QSilWindowTracker* instance();
    void track(QWidget*);
protected :
    QSilWindowTracker(QObject*, const char*);
private :

    // Disabled copy constructor and operator=
    QSilWindowTracker(const QSilWindowTracker&);
    QSilWindowTracker& operator=(const QSilWindowTracker&);

    bool eventFilter(QObject* watched, QEvent* e);
private slots :
    void handleWindowDestroyed(QObject*);
    void handleWidgetDestroyed(QObject*);
};

#endif /* QSIL_WINDOW_SETTINGS_H */
