#include <QApplication>
#include <QLibraryInfo>

#include "qsil/xresmgr.h"
#include "qsil/xresource.h"
#include "ADCApp.h"
#include "ADCMainWin.h"
#include "ADCUserPrefs.h"

#include <QtCore/QProcess>
#include <QtGui/QStandardItemModel>
#include <QMessageBox>
#include <QItemDelegate>
#include <QTreeView>
#include <QStyleFactory>
#include <QtPlugin>

using namespace adcui;


#ifdef WIN32
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
Q_IMPORT_PLUGIN(QWindowsPrinterSupportPlugin);
#else
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#endif

//extern void qInitImages_adc();

ADBApp::ADBApp(int& argc, char** argv, IADBCore* pICore)//const QString& productName)
	: QApplication(argc, argv),
	mrADB(*pICore),
	mpMainWin(nullptr)
	//mProductName(productName)
{
	QString path = QLibraryInfo::location(QLibraryInfo::PluginsPath);

	QStringList stypes(QStyleFactory::keys());
	setStyle(QStyleFactory::create("Windows"));
//	setStyle(QStyleFactory::create("Fusion"));
//	setStyle("windowsxp");

	mrADB.AddRef();
	//app.connect( this, SIGNAL(lastWindowClosed()), this, SLOT(quit()) );

	mCompanyName = mrADB.GetCompanyName();
	mProductName = mrADB.GetProductCodeName();
	
	Q_INIT_RESOURCE(images);
	//qInitImages_ADC();

	mpResMgr = new ADBResManager(mCompanyName, mProductName);

#if(0)
	QProcess* proc = new QProcess(this);
	proc->setWorkingDirectory("D:\\andreys\\test\\cl");
	QString program = "cl.exe";
	QStringList arguments;
	arguments << "/c" << "/Itmp" << "src\\file1.cxx";
	proc->start(program, arguments);
	proc->waitForFinished();
	QString strOut = proc->readAllStandardOutput();

	QMessageBox::information(0,
		QString("z"),
		strOut,
		QMessageBox::Ok | QMessageBox::Default, 0, 0);
#endif

	//no this crap!
	setEffectEnabled(Qt::UI_AnimateMenu, false);
	setEffectEnabled(Qt::UI_FadeMenu, false);
	setEffectEnabled(Qt::UI_AnimateTooltip, false);
	setEffectEnabled(Qt::UI_FadeTooltip, false);
	setEffectEnabled(Qt::UI_AnimateCombo, false);
}

ADBApp::~ADBApp()
{
	delete mpMainWin;
	delete mpResMgr;
	//	QWidget *pWin(mainWidget());
		//setMainWidget(nullptr);
		//delete pWin;
	mrADB.Release();
}

bool ADBApp::createMainWidget()
{
	mpMainWin = new ADBMainWin(*resManager(), mrADB);
	//setMainWidget(pMainWin);
	mpMainWin->setWindowTitle(ProductName());
	mpMainWin->resize(QSize(1440, 900).expandedTo(mpMainWin->minimumSizeHint()));
	mpMainWin->raise();

	//	mpResMgr->loadWindowGeometry(mpMainWin);
	//	mpResMgr->loadShortcuts(mpMainWin);

	//	pMainWin->show();

	//	connect(this, SIGNAL(signal FileListChanged()),
	//		pMainWin, SLOT(slot FileListChanged()));

	return true;
}

bool ADCApp::createMainWidget()
{
	mpMainWin = new ADCMainWin(*resManager(), mrADC);
	//setMainWidget(pMainWin);
	mpMainWin->setWindowTitle(ProductName());
	mpMainWin->resize(QSize(1440, 900).expandedTo(mpMainWin->minimumSizeHint()));
	mpMainWin->raise();
	return true;
}

#if(0)
void ADBApp::customEvent(QCustomEvent* e)
{
	switch (e->type())
	{
	case QEvent::User + 0://com processing started
		if (mpServer)
			mpServer->OnComStarted();
		break;

	case QEvent::User + 1://com processing stopped
		if (mpServer)
			mpServer->OnComStopped();
		break;

	case QEvent::User + 2://process command line args
		if (mpServer)
		{
			QString fn = argv()[1];
			mpServer->PostCommand(QString("process %1)").arg(fn));
		}
		break;

		//	case QEvent::User+UIMSG_PROJECT_NEW:
		//		break;

		//	case QEvent::User+UIMSG_PROJECT_CLOSED:
		//		break;

	case GUI_PROCESS_MESSAGE:
	{
		ADC_ProcessMessageData* pevent_data = (ADC_ProcessMessageData*)e->data();
		int nrc = processNotification(pevent_data->getId(), pevent_data->getData());

		// CANNOT set non-blocking event return code as variable probably no longer valid
		if (pevent_data->isBlocking())
			pevent_data->setReturnCode(nrc);
		else
			delete pevent_data;
	}
	break;

	default:
		break;
	}
}


void ADBApp::postCommand(const QString& cmd)
{
	if (mpServer)
		mpServer->PostCommand(cmd);
}


int ADBApp::processNotification(int eid, void* pvdata)
{
	int  nrc = TRUE;
	bool bprocess_events = true;

	// all subsequent messages require valid main widget
	ADBMainWin* pMainWin = dynamic_cast<ADBMainWin*>(mainWidget());
	// - ignore message if GUI is shutting down
//?	if( pMainWin && pMainWin->closingDown() )
//?		return FALSE;


	/********************* IMPORTANT **************************

	QWidget derived objects can generate confusing Qt crashes
	if they are called directly from a thread other than the
	main (GUI) thread - please apply the following rules:

	main thread - if Utilities::isMainThread() == true,
					it is safe to access GUI object directly.

	otherwise   - MUST queue message using only thread safe
					API, QApplication::postEvent() or create a
					custom queue.

	**********************************************************/

	if (!isMainThread())
	{
		// use semaphore to block this thread until message has been processed by GUI thread
		bool         bblock = true;
		QSemaphore* pwait_sem = nullptr;

		// - some message can override the blocking mechanism
//?		if ( eid > GUIMSG__NOBLOCK_BEGIN && eid < GUIMSG__NOBLOCK_END )
//?			bblock = false;

		if (bblock)
		{
			pwait_sem = new QSemaphore(1);
			(*pwait_sem)++;
		}

		// post event data to GUI thread for processing
		ADC_ProcessMessageData* pev_data = new ADC_ProcessMessageData(eid, pvdata, pwait_sem, nrc);
		postEvent(this, new QCustomEvent((QEvent::Type)GUI_PROCESS_MESSAGE, pev_data));

		// blocking event is deallocated once unblocked to ensure synchronization
		// - non-blocking event will be deallocated once it has been processed
		if (bblock)
		{
			// - this will block until semaphore has been unflagged (by GUI thread)
			(*pwait_sem)++;

			delete pev_data;
			delete pwait_sem;
		}
		return nrc;
	}

	// process message
	switch (eid)
	{
		/*		case GUIMSG_PROCESS_EVENTS:
				// used to explicitly allow GUI to process events (see below)
				break;

			case GUIMSG_QUIT:
				bprocess_events = false;
				if ( pMainWin != nullptr )
					pMainWin->close();
				break;

			case GUIMSG_UPDATE:
				processEvents();
				bprocess_events = false;
				break;*/

	case 0://UIMSG_NULL:
	default:
		if (pMainWin && !pMainWin->processMessage(eid, pvdata))
			bprocess_events = false;
		// ignore message as it may have already been handled
		break;

	}

	// yield thread on regular basis
	// - MUST do this otherwise GUI becomes unresponsive
	static QTime time;
	static bool  bstarted = false;

	if (!bstarted)
	{
		time.start();
		bstarted = true;
	}
	else if (bprocess_events && time.elapsed() > 250)
	{
		// to make GUI look responsive, explicitly process events
		// - can't use ExcludeUserInput for run time dialog as it has action buttons
		//   that should always be accessible
		processEvents();

		// restart the timer after processing the events
		time.restart();
	}

	return nrc;
}
#endif

void ADBApp::OnReady()
{
}

void ADBApp::OnShow()
{
	ADBMainWin* pMainWin(static_cast<ADBMainWin*>(mpMainWin));
	if (!pMainWin)
		return;

	mrADB.EnableOutputCapture();

	pMainWin->show();
}

void ADBApp::PostBlockingEvent(void* pData)
{
	//	postEvent(this, new QCustomEvent(5555, nullptr));
	postEvent(this, new ADCCustomEvent((QEvent::Type)GUI_BLOCKING_EVENT, pData));
}

int ADBApp::ProcessEvent(int eid, void* pvData)
{
	if (closingDown())
		return 0;

	int nrc = 1;
	switch (eid)
	{
	case MSGID_SHOW:
		OnShow();
		break;

	case MSGID_READY:
		OnReady();
		break;

	case MSGID_RUN:
		return exec();

	case MSGID_QUIT:
		this->exit(0);
		return 1;

	default:
	{
		if (mpMainWin)
			nrc = mpMainWin->HandleEvent(eid, pvData);
	}
	break;
	}

	return nrc;
}

void ADBApp::customEvent(QEvent* pEvent)
{
	int EventType = pEvent->type();
	//void* vEventData = pEvent->data();

	switch (EventType)
	{
	case GUI_EVENT_EXIT:
		this->exit(0);
		break;

	case GUI_BLOCKING_EVENT:
	{
		ADCCustomEvent* pCustomEvent(static_cast<ADCCustomEvent*>(pEvent));
		ClearBlockingEvent(pCustomEvent->data());
	}
	break;

	default:
		// Ignore unhandled custom events
		break;
	}
}


My::IGui* CreateADBGui(int& argc, char** argv, My::IUnk* pIUnk)
{
	IADBCore* pICore(dynamic_cast<IADBCore*>(pIUnk));
	if (!pICore)
		return nullptr;
	ADBApp* pApp(new ADBApp(argc, argv, pICore));
	pApp->createMainWidget();
	return pApp;
}

My::IGui* CreateADCGui(int& argc, char** argv, My::IUnk* pIUnk)
{
	//QApplication::setDesktopSettingsAware(false);
	//QCoreApplication::addLibraryPath("c:\\Users\\Andrey\\work\\adc\\.build\\v142\\Win32\\Debug\\platforms");
	IADCCore* pICore(dynamic_cast<IADCCore*>(pIUnk));
	if (!pICore)
		return nullptr;
	ADCApp* pApp(new ADCApp(argc, argv, pICore));
	pApp->createMainWidget();
	return pApp;
}



////////////////////
#if(0)

#include <QApplication>
#include <QPushButton>
#include <QMainWindow>
#include <QThread>


class MyApp : public QApplication
{
public:
	MyApp(int argc, char** argv) : QApplication(argc, argv)
	{
		QMainWindow w;
		w.setCentralWidget(new QPushButton("NewButton"));
		w.show();
	}
protected:
	virtual void customEvent(QEvent* e)
	{
		int z(e->type());
		if (z == 5555)
		{
			int b(0);
		}

	}
};

//#include "qx/qthread.h"

class MyThread : public QxThread
{
	int m_argc;
	char** m_argv;
public:

	MyThread(int argc, char** argv) : m_argc(argc), m_argv(argv) {}
	virtual void run()
	{
		MyApp app(m_argc, m_argv);

		app.postEvent(&app, new ADCCustomEvent((QEvent::Type)5555, nullptr));
		bool b = app.hasPendingEvents();
		app.exec();
	}
};

int testmain(int argc, char* argv[])
{
	return 0;
	MyThread a(argc, argv);
	a.start();
	//  return;

	Sleep(1000000);
	QCoreApplication b(argc, argv);
	b.exec();
	return 1;
}

#endif

#if(0)
class Delegate : public QItemDelegate
{
public:
	Delegate(QObject* parent = 0)
		: QItemDelegate(parent)
	{

	}

protected:
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QItemDelegate::paint(painter, option, index);
		// highlight the "item" which should be spanned
		/ *painter->setBrush(QColor(200, 200, 200, 50));
		if (index.child(0, 0).isValid())
			painter->drawRect(option.rect);* /
	}
};

int __main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	QStandardItemModel model(2, 2);
	for (int a = 0; a < 2; ++a) {
		QStandardItem* itemA = new QStandardItem(QString("A: %0").arg(a));
		for (int b = 0; b < 2; ++b) {
			QStandardItem* itemB = new QStandardItem(QString("A: %0; B: %1").arg(a).arg(b));
			for (int c = 0; c < 2; ++c) {
				for (int d = 0; d < 4; ++d) {
					QStandardItem* itemC = new QStandardItem(QString("(%0;%1) C: %2/%3").arg(a).arg(b).arg(c).arg(d));
					itemB->setChild(c, d, itemC);
				}
			}
			itemA->setChild(b, 0, itemB);
		}
		model.setItem(a, itemA);
	}
	QTreeView tree;
	tree.setModel(&model);
	tree.setItemDelegate(new Delegate(&a));

	// this I want to avoid because normally the displayed data is much more...
	tree.setFirstColumnSpanned(0, model.index(-1, 0), true);
	tree.setFirstColumnSpanned(1, model.index(-1, 0), true);
	tree.setFirstColumnSpanned(0, model.index(0, 0), true);
	tree.setFirstColumnSpanned(1, model.index(0, 0), true);
	tree.setFirstColumnSpanned(0, model.index(1, 0), true);
	tree.setFirstColumnSpanned(1, model.index(1, 0), true);

	tree.show();
	tree.expandAll();
	return a.exec();
}
#endif

#ifndef ADCUI_EXPORT
#ifdef WIN32
#define ADCUI_EXPORT extern __declspec(dllexport)
#else
#define ADCUI_EXPORT extern
#endif
#endif


#include <QFile>
#include <QTextStream>
#include <QDir>

extern "C"
{
	ADCUI_EXPORT My::IGui* CreateADBGui_imp(int& argc, char** argv, My::IUnk* pICore)
	{
		_putenv("QT_QPA_PLATFORM=windows");
		return CreateADBGui(argc, argv, pICore);
	}

	ADCUI_EXPORT My::IGui* CreateADCGui_imp(int& argc, char** argv, My::IUnk* pICore)
	{
		//QString path = QLibraryInfo::location(QLibraryInfo::PluginsPath);

		_putenv("QT_QPA_PLATFORM=windows");
		return CreateADCGui(argc, argv, pICore);
	}
}

