#pragma once

#include "qx/MyArgs.h"
#include "qx/MyPath.h"


//********************************************************* (Options_t)
struct Options_t
{
	bool		bScriptAbortIf;	//enable script abortion (in presence of $abortif directive)
	int			nProtoMode;		//1:load; 2:load&append
	bool		bTimeStats;		//gather processing statistics
	//bool		bFastDisasm;	//call depth
	int			nCallDepth;
	int			nLocalAutoNamesMode;	//choise of naming of locals (1|2|3)
	bool		bNoDbg;		//don't post-analyze debug info
	bool		bNoAutoIds;	//EXPERT: replace unnamed strucs ids with '0' (enfource unanimity)
	bool		bNoLogo;	//suppress a logo (product, version etc.)
	//bool		bNoMap;

	Options_t()
		: nCallDepth(-1)//unlimited
		,bScriptAbortIf(false)
		,nProtoMode(0)
		,bTimeStats(false)
		,nLocalAutoNamesMode(0)
		//,bFastDisasm(false)
		,bNoDbg(false)
		,bNoAutoIds(false)
		,bNoLogo(false)
		//,bNoMap(false)
	{
	}
};


class StartupInfo_t : public MyArgs2,
	public Options_t
{
public:
	bool		bBatchMode;//if true, batch mode - otherwise
	//bool		bInteractiveMode;
	bool		bConsole;
#ifdef _DEBUG
	bool		bSymParseTest;
#endif
	bool		bForceNew;
	MyString	sGuiDll;
	MyString	sTarget;
	MyString	sScript;
	MyPath		fIniPath;
	MyPath		fOutPath;
	MyPath		fErrPath;
public:
	StartupInfo_t(int argc, char ** argv)
		: MyArgs2(argc, argv)
		,bBatchMode(false)
//		,bInteractiveMode(true)
		,bConsole(false)
#ifdef _DEBUG
		,bSymParseTest(false)
#endif
		,bForceNew(false)
	{
	}
};


class SturtupInfo_t : public StartupInfo_t
{
public:
	enum { E_NO_ARG, E_BAD_OPT };
	SturtupInfo_t(int argc, char ** argv);
	bool parse();
	MyPath parseIni(const char*) const;
	bool error(int eId, size_t i) const;
};


