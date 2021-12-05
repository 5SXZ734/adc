#include <QApplication>
#include "ADCUserPrefs.h"
#include "ADCMainWin.h"
//#include "qsil/xresmgr.h"
#include "qsil/xresource.h"

ADBResManager::ADBResManager(QString companyName, QString productName)
	: QSilResManager(companyName, productName),
	gDefaultOnBarConfig(0),
	gDefaultWinDockConfig(0),
	gDefaultPageHideConfig(0)
{
}

ADBResManager::~ADBResManager()
{
}

void ADBResManager::loadWindow(QMainWindow *pWin)
{
	loadWindowGeometry(pWin);
	loadShortcuts(pWin);
}


int ADBResManager::__defaultABarConfig() const { return ADBMainWin::DEFAULT_ASSBARCONFIG(); }
int ADBResManager::__defaultWinDockConfig() const { return ADCMainWin::DEFAULT_WINDOCKCONFIG(); }
QString ADBResManager::__defaultWinOpenConfig() const { return ADBMainWin::DEFAULT_PAGESHOWCONFIG(); }


typedef QSilIntResource		XRESOURCETYPE_int;
typedef QSilBoolResource	XRESOURCETYPE_bool;
typedef QSilDoubleResource	XRESOURCETYPE_double;
typedef QSilStringResource	XRESOURCETYPE_QString;
typedef QSilFontResource	XRESOURCETYPE_QFont;

#define	IMPLEMENT_PROPERTY(name, type, defval, category) \
	static const QString UserPref##name = "/"#category"/"#name; \
	type ADBResManager::get##name() const { \
	XRESOURCETYPE_##type *res = const_cast<ADBResManager*>(this)->newResource(UserPref##name, defval); \
	if (res) return res->value(); \
	return defval; } \
	void ADBResManager::set##name(type rprop){ \
	XRESOURCETYPE_##type *res = newResource(UserPref##name, defval); \
	if (res) res->setValue(rprop); }

//IMPLEMENT_PROPERTY(OnBarConfig, int, ADBResManager::gDefaultOnBarConfig, Windows);
//IMPLEMENT_PROPERTY(WinDockConfig, int, ADBResManager::gDefaultWinDockConfig, Windows);
//IMPLEMENT_PROPERTY(PageHideConfig, int, ADBResManager::gDefaultPageHideConfig, Windows);


IMPLEMENT_PROPERTY(TabbedWindows, bool, true, ADC);
IMPLEMENT_PROPERTY(ABarConfig, int, __defaultABarConfig(), ADC);
IMPLEMENT_PROPERTY(WinDockConfig, int, __defaultWinDockConfig(), ADC);
IMPLEMENT_PROPERTY(MDIWinLayout, QString, __defaultWinOpenConfig(), ADC);
IMPLEMENT_PROPERTY(SaveEditDocs, bool, false, ADC);
//IMPLEMENT_PROPERTY(DispersedMode, bool, false, ADC);

static QFont defaultOutputFont()
	{
		QFont f;
		f.setStyleHint(QFont::TypeWriter);
		f.setFamily("Courier");

		QFont appl_font = QApplication::font();
		int ptsize = appl_font.pointSize();

		if (ptsize < 0)
			f.setPixelSize(appl_font.pixelSize());
		else
			f.setPointSize(ptsize);
		return f;
	}

IMPLEMENT_PROPERTY(OutputFont, QFont, defaultOutputFont(), Views);

/*bool ADBResManager::GetPageHideConfig( int pageID )
{
	int def = gDefaultPageHideConfig;
	int phc = getPageHideConfig(def);
	bool rprop = (phc & (1 << pageID)) != 0;
    return rprop;
}

void ADBResManager::SetPageHideConfig( int pageID, bool bSet )
{
	int def = gDefaultPageHideConfig;
 	int phc = getPageHideConfig(def);
	if ( bSet )
		phc |= (1 << pageID);
	else
		phc &= ~(1 << pageID);
	setPageHideConfig( phc );
}*/

IMPLEMENT_PROPERTY(CallDepth, int, -1, Analysis);//unlimited




