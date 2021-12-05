#include <iostream>
#include "startup.h"


SturtupInfo_t::SturtupInfo_t(int argc, char ** argv)
	: StartupInfo_t(argc, argv)
{
}

bool SturtupInfo_t::parse()
{
	for (size_t i(1); i < size(); i++)
	{
		if (at(i) == "-gui")
		{
			if (!(++i < size()))
				return error(E_NO_ARG, i - 1);
			sGuiDll = at(i);
			bBatchMode = false;
			//bInteractiveMode = true;
		}
		else if (at(i) == "-b")
		{
			bBatchMode = true;
			bConsole = true;
		}
		/*else if (at(i) == "-i")
		{
			bInteractiveMode = true;
		}*/
		else if (at(i) == "-c")
		{
			bConsole = true;
		}
		else if (at(i) == "-n")
		{
			bForceNew = true;
		}
		else if (at(i) == "-fast")//do not analyze funcs other than entry points 
		{
			nCallDepth = 1;//bFastDisasm = true;
		}
		else if (at(i) == "-ini")
		{
			if (++i < size())
			{
				fIniPath = MyPath(at(i));
				if (fIniPath.Ext().empty())
					fIniPath.SetExt("ini");
			}
			else
				error(E_NO_ARG, i - 1);
		}
		else if (at(i) == "-o")
		{
			if (++i < size())
			{
				fOutPath = MyPath(at(i));
				if (fOutPath.Ext().empty())
					fOutPath.SetExt("out");
			}
			else
				error(E_NO_ARG, i - 1);
		}
		else if (at(i) == "-e")
		{
			if (++i < size())
			{
				fErrPath = MyPath(at(i));
				if (fErrPath.Ext().empty())
					fErrPath.SetExt("err");
			}
			else
				error(E_NO_ARG, i - 1);
		}
		else if (at(i) == "-script" || at(i) == "-s" || at(i) == "-s~")
		{
			bScriptAbortIf = (at(i) == "-s~");
			if (++i < size())
				sScript = at(i);
			else
				error(E_NO_ARG, i - 1);
		}
		else if (at(i) == "-t")//try stubs - no save
		{
			nProtoMode = 1;
		}
		else if (at(i) == "-ts")//try stubs AND save
		{
			nProtoMode = 2;
		}
		else if (at(i) == "-stat" || at(i) == "-stats")
		{
			bTimeStats = true;
		}
#ifdef _DEBUG
		else if (at(i) == "-cc")
		{
			bConsole = true;
			bSymParseTest = true;
		}
#endif
		else if (at(i) == "-platform")//Qt5 aware
		{
			++i;//skip
		}
		else if (at(i) == "-v0")
		{
			nLocalAutoNamesMode = 0;
		}
		else if (at(i) == "-v1")
		{
			nLocalAutoNamesMode = 1;
		}
		else if (at(i) == "-v2")
		{
			nLocalAutoNamesMode = 2;
		}
		else if (at(i) == "-nodbg")
		{
			bNoDbg = true;
		}
		/*else if (at(i) == "-nomap")
		{
			bNoMap = true;
		}*/
		else if (at(i) == "-naid")
		{
			bNoAutoIds = true;
		}
		else if (at(i) == "-nlogo")
		{
			bNoLogo = true;
		}
		//no more options...
		else if (at(i).at(0) == '-')//option?
		{
			return error(E_BAD_OPT, i);
		}
		else if (sTarget.empty())
		{
			sTarget = at(i);
		}
		else
		{
			std::cerr << "Warning: Multiple targets ignored: " << at(i) << std::endl;
		}
	}
	return true;
}

MyPath SturtupInfo_t::parseIni(const char* exePath) const
{
	MyPath f(exePath);
	f.SetExt("ini");
	for (size_t i(1); i < size(); i++)
	{
		if (at(i) == "-ini")
		{
			if (++i < size())
			{
				f = MyPath(at(i));
				if (f.Ext().empty())
					f.SetExt("ini");
				break;
			}
			else
				error(E_NO_ARG, i - 1);
		}
	}
	return f;
}

bool SturtupInfo_t::error(int eId, size_t i) const
{
	switch (eId)
	{
	case E_NO_ARG:
		std::cerr << "Error: No argument for an option: " << at(i) << std::endl;
		break;
	case E_BAD_OPT:
		std::cerr << "Error: Invalid option: " << at(i) << std::endl;
		break;
	default:
		break;
	}
	return false;
}
