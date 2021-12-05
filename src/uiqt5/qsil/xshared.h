
#ifndef QSIL_H
#define QSIL_H

//#include <QNamespace>
#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QtCore/QString>
#include <QtCore/QRegExp>

// Handly macro to disable copy constructor and operator=
#define QSIL_DISABLE_COPY(C) \
    C(const C&); \
    C& operator=(const C&);

#define PLACE_MAX 6

#define QSilStyle_DialogNoWhatsThis WStyle_Customize | \
    	WStyle_NormalBorder | \
	WStyle_Title | \
	WStyle_SysMenu

//Namespace to contain utility functions.

namespace QSil
{
    //?extern const Qt::Dock DOCK_PLACES[PLACE_MAX];	

    QAction* locateActionByLabel(QMainWindow* mainWindow, const QString& actionLabel);
    QAction* locateActionByMenuData(QMainWindow* mainWindow, const QMenu* menu, const int id);
    QAction* locateActionByName(QMainWindow* mainWindow, const QString& actionName);
    QWidget* locateWidgetByName(QMainWindow* mainWindow, const QString& widgetName);
    
    QStringList getToolbarContents(QToolBar* toolBar); 
    void setToolbarContents(QToolBar* toolBar, 
	    QMainWindow* window, 
	    const QStringList& contents);  

    
    enum GuiStandardCols
    {
    	GUICOL_START,
	    
    	GUICOL_FOREGROUND = GUICOL_START,
    	GUICOL_BACKGROUND,
    	GUICOL_TOP_SHADOW,
    	GUICOL_BOTTOM_SHADOW,
    	GUICOL_SELECT,
	    
    	GUICOL_TOTAL
    };    
    QColor getGuiColor(const int guiIndex);
    
    QString shortenFilePath(const QString& filePath);

    extern const QString ACCEL_DELIMETER;
}

#endif /* QSIL_H */
