#pragma once

#include "qsil/xresmgr.h"

#define	DECLARE_PROPERTY(name, type) \
	virtual type get##name() const; \
	virtual void set##name(type)

class ADBResManager : public QSilResManager
{
public:
	ADBResManager(QString, QString);
	virtual ~ADBResManager();

	const int gDefaultOnBarConfig;
	const int gDefaultWinDockConfig;
	const int gDefaultPageHideConfig;


	DECLARE_PROPERTY(TabbedWindows, bool);
	DECLARE_PROPERTY(ABarConfig, int);
	DECLARE_PROPERTY(WinDockConfig, int);
	DECLARE_PROPERTY(MDIWinLayout, QString);
	DECLARE_PROPERTY(SaveEditDocs, bool);
	DECLARE_PROPERTY(OutputFont, QFont);
//	DECLARE_PROPERTY(DispersedMode, bool);

	DECLARE_PROPERTY(CallDepth, int);

	//bool GetPageHideConfig( int pageID );
	//void SetPageHideConfig( int pageID, bool bSet );

	void loadWindow(QMainWindow *);

	virtual int __defaultABarConfig() const;
	virtual int __defaultWinDockConfig() const;
	virtual QString __defaultWinOpenConfig() const;
};


