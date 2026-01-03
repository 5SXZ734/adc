#pragma once

#include "qx/MyRedirect2.h"

#include "qx/MyGuiThread.h"
#include "qx/IGui.h"
#include "qx/MyString.h"
#include "qx/MyPath.h"
#include "qx/MyFileMgr.h"
#include "shared/INIReader.h"

#include "db/main.h"
#include "loadui.h"


#define PRODUCT_NAME		"Andrey's Debugger"
#define PRODUCT_CODENAME	"ADB"
#define COMPANY_NAME		"Komok Ink"
#define PRODUCT_VERSION		2,0,0
//version history:
//v0.61.0 (Nov 1, 2004) - first public release!

//Configurations:
//1)DEMO - Release, NoScript,NoFrontEnd,NoSave etc.. 
//2)USER - Realease, for users - no technology features exposed
//3)DEBUG - Debug, for me - all included
//4)RELEASE - Release,  for me - all included


//=================================================== (Version_t)
struct Version_t
{
	const uint32_t	m_nMajor;
	const uint32_t	m_nMinor;
	const uint32_t	m_nBuild;

	Version_t(uint32_t nMajor, uint32_t nMinor, uint32_t nBuild) :
		m_nMajor(nMajor), m_nMinor(nMinor), m_nBuild(nBuild)
	{
	}

	const char *asString(bool bBuild = false) const
	{
		static char buf[30];
		if (bBuild || m_nBuild)
			sprintf(buf, "%d.%d.%d", m_nMajor, m_nMinor, m_nBuild);
		else
			sprintf(buf, "%d.%d", m_nMajor, m_nMinor);
/*		_ultoa(m_nMajor, buf, 10);/
		strcpy(buf+strlen(buf), ".");
		_ultoa(m_nMinor, buf+strlen(buf), 10);
		if (bBuild)
		{
			strcpy(buf+strlen(buf), ".");
			_ultoa(m_nBuild, buf+strlen(buf), 10);
		}*/
		return buf;
	}
};

//=================================================== (ADBGuiThread)
class ADBGuiThread : public My::GuiThread
{
public:
	ADBGuiThread(int argc, char **argv)
		: My::GuiThread(argc, argv)
	{
	}

	virtual ~ADBGuiThread()
	{
		//mpICore->Release();
		Stop();
	}

protected:
	virtual IGui *NewApplication(IUnk* pICore)
	{
		return CreateADBGui(m_argc, m_argv, pICore);
	}

	virtual void Unload() override
	{
		UnloadGui();
	}

	virtual int EventId(int n)
	{
		switch (n)
		{
		case EV_READY: return adcui::MSGID_READY;
		case EV_RUN: return adcui::MSGID_RUN;
		case EV_QUIT: return adcui::MSGID_QUIT;
		default: break;
		}
		assert(0);
		return -1;
	}
};



//=================================================== (ADB_t)
class ADB_t
{
protected:
	Version_t G_Version;
	SturtupInfo_t startup;
private:
#ifdef WIN32
	bool bUseParentsConsole;
#endif
	int iRedirect;
	My::XRedirect fStdOut;
	My::XRedirect fStdErr;
public:

	//===================================================================
	ADB_t(int argc, char** argv)
		: G_Version(PRODUCT_VERSION),
		startup(argc, argv)
#ifdef WIN32
		, bUseParentsConsole(false)
#endif
#if(1)
		, iRedirect(0)
#else
		, iRedirect(-1)//no redirect at all
#endif
		, fStdOut(stdout)
		, fStdErr(stderr)

	{
		readIni(startup.parseIni(argv[0]), startup);

		if (!startup.parse())
			throw (-1);//error should've been reported

#ifdef WIN32
		if (startup.bConsole)
		{
			BOOL allocConsole2(void);
			BOOL attachOutputToConsole(void);
			iRedirect = -1;
			bUseParentsConsole = attachOutputToConsole();//check if started from a terminal
			if (!bUseParentsConsole)
				allocConsole2();//no, needs to allocate it's own console
		}
#endif

		if (!startup.fOutPath.IsNull())
		{
			if (startup.fErrPath.IsNull())
				startup.fErrPath = startup.fOutPath;
		}
		else if (!startup.fErrPath.IsNull())
		{
			startup.fOutPath = startup.fErrPath;
		}

		if (!startup.fOutPath.IsNull())
		{
			MyFile f(startup.fOutPath);
			if (f.EnsureDirExists() && fStdOut.toFile(startup.fOutPath.Path().c_str()))
			{
				iRedirect |= 1;
			}
			else
			{
				std::cerr << "Error: Unable to redirect stdout to " << startup.fOutPath.Path() << "." << std::endl
					<< "Check directory permissions" << std::endl;
			}
		}

		if (!startup.fErrPath.IsNull())
		{
			MyFile f(startup.fErrPath);
			if (!fStdErr.toFile(startup.fErrPath.Path().c_str()))
			{
				std::cerr << "Error: Unable to redirect stderr to " << startup.fErrPath.Path() << "." << std::endl
					<< "Check directory permissions" << std::endl;
			}
			else
			{
				iRedirect |= 2;
			}
		}
	}

	//===================================================================
	virtual ~ADB_t()
	{
#ifdef WIN32
		if (bUseParentsConsole)
		{
			void sendEnterKey(void);
			//Send "enter" to release application from the console 
			//This is a hack, but if not used the console doesn't know the application has returned 
			//"enter" only sent if the console window is in focus
			if (GetConsoleWindow() == GetForegroundWindow())
				sendEnterKey();
		}
#endif
	}

	virtual const char* productName() const { return PRODUCT_NAME; }
	virtual const char* productCodeName() const { return PRODUCT_CODENAME; }

	//===================================================================
	virtual Main_t* createMain(int argc, char** argv)
	{
		return new Main_t(argc, argv, productName(), productCodeName(), G_Version.asString(), COMPANY_NAME, startup);
	}

	//===================================================================
	void hello(const Main_t& rMain)
	{
		if (rMain.options().bNoLogo)
			return;
		std::cout << rMain.colorTag(adcui::COLORTAG_DARKRED) << productName() << rMain.colorTag(adcui::COLORTAG_OFF) << std::endl
			<< "Version " << G_Version.asString()
#ifndef _DEBUG//avoid diffs in regressions
			<< " (" << __DATE__ << ")"
#endif
			<< std::endl
			<< "Copyright (C) 2001-2021 by " COMPANY_NAME ". All rights reserved." << std::endl
			<< std::endl;
#ifdef _DEMO
		cout << "[DEMO]" << endl << endl;
#else
#ifdef _DEBUG
		std::cout << "[DEBUG]" << std::endl << std::endl;
#endif
#endif
		fflush(stdout);
	}

	//===================================================================
	virtual ADBGuiThread* newGUI(int argc, char** argv)
	{
		return new ADBGuiThread(argc, argv);
	}

	//===================================================================
	My::IGui* startGUI(int argc, char** argv, Main_t& rMain)
	{
		if (startup.bBatchMode)
			return nullptr;
		bool bEnableOutputCapture(iRedirect == 0);// !startup.bConsole);
		ADBGuiThread* pGui(newGUI(argc, argv));
		if (pGui->Start(rMain.createGuiBroker(pGui, bEnableOutputCapture)))
			return pGui;
		rMain.setGui(nullptr);//WARNING: does not delete the object (why?)
		delete pGui;
		return nullptr;
	}

	//===================================================================
	int run(Main_t& rMain)
	{
		rMain.postEvent(new adc::CEventCreate(startup));

		int iExitStatus(rMain.exec());

		rMain.CloseProject();// nullptr);
		return iExitStatus;
	}

	bool readIni(const MyPath& path, Options_t& opt)
	{
		INIReader reader(path.Path());
		if (reader.ParseError() != 0)
			return false;

		//std::string s = reader.Get("test", "_string", "here");
		//bool b = reader.GetBoolean("test", "_bool", false);*/

		opt.bTimeStats = reader.GetBoolean("general", "NoLogo", false);

		opt.nCallDepth = reader.GetInteger("disassembler", "CallDepth", -1);

		opt.bScriptAbortIf = reader.GetBoolean("script", "AbortIf", false);

		opt.nProtoMode = reader.GetInteger("stubs", "ProtoMode", 0);

		opt.nLocalAutoNamesMode = reader.GetInteger("decompiler", "LocalAutoNamesMode", false);
		opt.bNoAutoIds = reader.GetBoolean("decompiler", "NoAutoIds", false);

		opt.bNoDbg = reader.GetBoolean("symbols", "NoDbg", false);

		return true;
	}
};



