#include "command.h"
#include "prefix.h"
#include <stdarg.h>

#include "qx/MyFileMgr.h"
#include "shared/link.h"
#include "shared/misc.h"
#include "mem.h"
#include "obj.h"
#include "field.h"
#include "type_seg.h"
#include "script.h"
#include "main.h"
#include "proj.h"
#include "ui_main.h"


Cmd_t::Cmd_t()
	: mpResponce(nullptr)
{
}

Cmd_t::Cmd_t(const char * buf)
	: mpResponce(nullptr)
{
	bool bAppend(false);
	MyString s(buf);
	int n(s.find('>'));
	if (n >= 0)
	{
		if (s.startsWith(">>"))
		{
			bAppend = true;
			sFile = s.mid(n + 2).stripWhiteSpace();
		}
		else
			sFile = s.mid(n + 1).stripWhiteSpace();
		s.truncate(n);
	}

	Init(s.c_str());

	//nID = CMD_NULL;
	//pValue = 0;
}

Cmd_t::~Cmd_t()
{
	clear();
//	delete pValue;
	//nID = CMD_NULL;
}

void Cmd_t::clear()
{
	MyArgs2::clear();
	mResponse = "";
}



/////////////////////////////////////////////////////


CMDServer_t::CMDServer_t(CMDServerCommandMap &r)
	: mCommands(r),
	mpParent(nullptr),
	mRet(0)
{
}

bool CMDServer_t::ExecuteCommand(Cmd_t &cmd)
{
	//assert(cmdstr);

	bool ret(false);
	mos.clear();
	mos.str("");

	if (cmd.size() > 0)
	{
		std::map<std::string, CMDServerHandlerPtr>::iterator it;
		std::string s = cmd[0];
		it = mCommands.find(s);
		if (it != mCommands.end())
		{
			My::XRedirect *mpOut(nullptr);
			bool bRedirected(false);
			if (!cmd.sFile.empty())
			{
				if (ADCRedirect::instance())
					mpOut = &ADCRedirect::instance()->out();
				else
				{
					mpOut = new My::XRedirect(stdout, true);
					bRedirected = true;
				}

				MyFile f(cmd.sFile);
				if (f.Ext().empty())
					f.SetExt("out");
				cmd.sFile = f.Path();
				if (!f.EnsureDirExists() || !mpOut->toFile(cmd.sFile.c_str()))
				{
					fprintf(STDERR, "Warning: %s\n\tFailed to redirect output to file: %s\n", cmd.AsString().c_str(), cmd.sFile.c_str());
					if (bRedirected)
					{
						delete mpOut;
						mpOut = nullptr;
					}
				}
			}
			CMDServerHandlerPtr pf = it->second;
			mRet = (*pf)(this, cmd);
			if (mpOut)
			{
				mpOut->Restore(1);
				std::cout << "Printed to file: " << cmd.sFile << std::endl;
				if (bRedirected)
					delete mpOut;
			}
			ret = true;//command handler exists an has been executed
		}
	}

	if (cmd.mpResponce)
	{
		MyStreamUtil ssh(*cmd.mpResponce);
		ssh.WriteString(mos.str());
	}

//?	return mModifiedFlags;
	return ret;
}

//"cmdlist"
int CMDServer_t::COMMAND_cmdlist(CMDServer_t * pCMDServer, Cmd_t &)
{
	std::map<std::string, CMDServerHandlerPtr>::iterator it;
	it = pCMDServer->mCommands.begin();
	while (it != pCMDServer->mCommands.end())
	{
		std::string cmd_name = it->first;
//?		pCMDServer->mos << cmd_name.c_str() << std::endl;
		std::cout << cmd_name.c_str() << std::endl;
		it++;
	}
	return 1;
}







