
#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <QApplication>

#include "qsil/xresmgr.h"

#include "sx/SxGUI.h"
#include "interface/IADCGui.h"

class QSilResManager;
class SciServer;
class ADBResManager;


class ADBMainWin;

class ADBApp : public QApplication, public SxGUI
{
	Q_OBJECT
public:
	ADBApp(int& argc, char** argv, adcui::IADBCore* pICore);//const QString& productName );
	virtual ~ADBApp();

	virtual bool	createMainWidget();
	ADBResManager* resManager() { return mpResMgr; }

	//void	postCommand(const QString &);
	//int		processNotification(int eID, void * pData);

	const QString& ProductName() { return mProductName; }
	void postCoreEvent(int eid) { PostEvent(eid, nullptr); }

	adcui::IADBCore& ADB() { return mrADB; }

protected:
	//ADCGUI
	virtual int ProcessEvent(int, void*);
	virtual void PostBlockingEvent(void*);
	virtual bool isClosingDown() { return closingDown(); }

protected:
	virtual void customEvent(QEvent* pevent);

private:
	void OnReady();
	void OnShow();

signals:
	//void signal FileListChanged();
	void signalAppendText(const char*);

protected:
	adcui::IADBCore& mrADB;
	QString	mCompanyName;
	QString	mProductName;

	ADBResManager* mpResMgr;
	ADBMainWin* mpMainWin;
};


class ADCApp : public ADBApp
{
	Q_OBJECT
public:
	ADCApp(int& argc, char** argv, adcui::IADCCore* pICore)
		: ADBApp(argc, argv, pICore),
		mrADC(*pICore)
	{
	}
	virtual ~ADCApp() {}
	virtual bool createMainWidget();

	adcui::IADCCore& ADC() { return mrADC; }

protected:
	adcui::IADCCore& mrADC;
};


class ADCCustomEvent : public QEvent
{
	void* mpData;
public:
	ADCCustomEvent(QEvent::Type e, void* p)
		: QEvent(e),
		mpData(p)
	{
	}
	void* data() { return mpData; }
};

//#undef UPREFS
//#define UPREFS	(*(dynamic_cast<ADCApp *>(qApp)->resManager()))

//#undef CORE
//#define ADC (*(static_cast<ADCApp *>(qApp))->core())

#endif//__APPLICATION_H__
