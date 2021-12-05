
#include <conio.h>
#include <list>
#include <string>
#include <fstream>

#include "shared/defs.h"
#include "shared/link.h"
#include "shared/heap.h"
#include "shared/misc.h"
#include "shared/front.h"
#include "shared/console.h"
#include "dc/globals.h"
#include "dc/obj.h"
#include "dc/path.h"
#include "dc/func.h"
#include "dc/op.h"
#include "dc/field.h"
#include "dc/struc.h"
#include "dc/file.h"
#include "dc/dc.h"
#include "dc/main.h"
#include "dc/debug.h"
#include "dc/msg.h"
#include "back/back.h"
#include "back/display.h"
#include "back/asm.h"

/*#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif*/

void __FATALERROR(char *fmt, ... )
{
	char buf[256];

    va_list args;
    va_start(args, fmt);
	vsprintf(buf, fmt, args);
    va_end(args);

	EmergencyClose();
	
	printf("ERROR: ");
	printf("%s", buf);
	printf("\nany key...");

	getch();
	exit(1);
}

void About()
{
	printf("%s CONSOLE. Version %s (%s)\nCopyright (C) 2001-2015 by "__COMPANY__". All rights reserved.\n",
		G_ProjectName, 
		G_Version.Version2Str(), 
		__DATE__);
}

static void Help()
{
	std::cout << "\nusage: dc [ +|-options...] files...\n";
	std::cout << "\n\toptions:\n\n";

	dispflags_usage( std::cout );

	_getch();
//	exit(0);//ok
}

struct StartupInfo_t
{
	FEID_t		feid;
//	UInt32		flags_0;
//	UInt32		flags_1;
	UInt32		dumpflags_0;
	UInt32		dumpflags_1;
	std::list<std::string>	fnames;

	StartupInfo_t( int argc, char *argv[] );
	bool handleOption(char *opt, bool raise);
};

StartupInfo_t::StartupInfo_t( int argc, char *argv[] )
{
	feid = FE_NULL;
//?	flags_0 = flags_1 = 0;
	dumpflags_0 = dumpflags_1 = 0;

	int i;
	for (i = 1; i < argc; i++)
	{
		char c = argv[i][0];
		char *p = &argv[i][1];
		if (c == '/')//common
		{
			if (strcmp(p, "help") == 0)//help info
				Help();
		}
		else if (c == '+')//set flag
		{
			if (!handleOption(p, true))
				goto $ERROR;
		}
		else if (c == '-')//reset flag
		{
			if (!handleOption(p, false))
				goto $ERROR;
		}
		else
		{
			fnames.push_back( argv[i] );
		}
	}

	return;

$ERROR:
	__FATALERROR("Unrecognized command-line token: %c", argv[i][0]);
	return;
}

bool StartupInfo_t::handleOption(char *opt, bool raise)
{
#define checkopt(arg)	(strcmp(opt, arg) == 0)

	if (checkopt("#1"))				feid = FE_X86_32;
	else if (checkopt("#2"))		feid = FE_X86_16;
	else if (checkopt("#3"))		feid = FE_MPL;
	else if (checkopt("#4"))		feid = FE_JAVA;
//?	else if (checkopt("noecho"))	flags_1 |= DC_NOECHO;
	else
	{

		UInt32 f = 0;

		f = dispflag_check( opt );

		if ( f == 0 )
			return false;

		if ( raise )
			dumpflags_1 |= f;
		else
			dumpflags_0 |= f;
	}

	return true;
}

static void do_output(UInt32 dispflags, Main_t * pMain)
{
	char * buf = dispflags2str( dispflags&(DUMP_UNFOLD|DUMP_ASM), "." );
	char * path = pMain->GetOutputPath(NULL, /*dispflgas*/0, buf);

	pMain->Files_OutputToFile(path, dispflags);

//	}
//	catch (...)
//	{
//		__ERROR("output failed");
//	}
}

extern int G_c;
void Disassemble(const char *fname, int, int);
int main(int argc, char *argv[])
{
#ifdef _DEBUG
	extern void test();
	test();
#endif

	About();

	if (argc < 2)
	{
		Help();
		return 1;
	}

	DWORD tiStart = GetTickCount();

#ifdef _DEBUG
	__Droot.Activate(0);
	__Dout.Activate(1);
	__Dptr.Activate(1);
	__Ddep.Activate(1);
#endif

	Output_t * pOutput = new Output_t;

//	Console_t * pCon = new Console_t;
//	pCon->Init();
	StartupInfo_t sui( argc, argv );

	Main_t * pMain = new Main_t(argc, argv);
//	CMDQueue_t * pCMDQueue = pMain->createCMDQueue();
	pMain->clearFlags(sui.dumpflags_0);
	pMain->setFlags(sui.dumpflags_1);
	//pMain->setOutput(pOutput);

#ifndef _DEBUG
	try {
#endif
		while ( !sui.fnames.empty() )
		{
			std::string fname = sui.fnames.front();
			sui.fnames.pop_front();

//			pCMDQueue->ExecuteCommand( fmt("new %s", fname.c_str()) );
//			pCMDQueue->ExecuteCommand( fmt("listing %s", fname.append(".lst").c_str()) );
			//pMain->NewProject( fname.c_str() );
			//pMain->OpenFile( fname.c_str() );
//?			pMain->PrintListing(fname.append(".lst").c_str());
		}
#ifndef _DEBUG
	} catch (...)
	{
		//TRACE0("UNHANDLED ERROR CAUGHT!\n");
	}
#endif

	DWORD tiEnd = GetTickCount();

	int nErr = 0;
	printf("* * * DUMP LOG * * *\n");
	for (
	Msg_t *pMsg = pOutput->m_pMsgs;
	pMsg;
	pMsg = (Msg_t *)pMsg->Next())
	{
		if (pMsg->m_nType == 1)//error
			nErr++;
		printf("%s\n", pMsg->GetStr2());
		TRACE1("%s\n", pMsg->GetStr2());
	}

//	try {
//	if (!TestOpt(DC_GUI))//console output

	UInt32 dispflgas = pMain->flags();
#ifdef _DEBUG
	dispflgas |= DUMP_NOLOGO;
#endif

	do_output(dispflgas, pMain);
#if(1)
	do_output(dispflgas^DUMP_UNFOLD, pMain);
#endif

//	pdC->Store("dcp/$out.dcp");
	MemTrace_t::print();

	TRACE1("ADC TIME ELAPSED: %fs\n", (tiEnd-tiStart)/1000.0f);

	if (nErr)
		getch();

//return 0;
	delete pMain;
//	delete pCon;
	delete pOutput;

	MemTrace_t::check();

	return 0;
}







