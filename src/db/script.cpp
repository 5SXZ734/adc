#include "script.h"
#include "prefix.h"
#include <fstream>

#include "qx/MyStringList.h"
#include "qx/MyPath.h"
#include "mem.h"
#include "main.h"
#include "command.h"


ScriptMgr_t::ScriptMgr_t(const MyString &path, bool bAbortIf)
	: mbAbortIf(bAbortIf)
{
	m_timer.start();
	load(path);
}

ScriptMgr_t::~ScriptMgr_t()
{
	if (MAIN.options().bTimeStats)
	{
		fprintf(stdout, "*** script time: %g sec\n", m_timer.elapsed() / 1000.0);
		fflush(stdout);
	}
}

int CheckEmpty(char *buf)
{
	char *p = buf;
	while (*p)
	{
		if (!isspace(*p))
			return 0;
		p++;
	}

	return 1;//empty!
}

int CutCommentOff(char *buf)
{
	char *p = buf;
	while (*p)
	{
		if (*p == ';')
		{
			*p = 0;
			return 1;
		}
		p++;
	}

	return 0;
}

bool ScriptMgr_t::load(const MyString &path)
{
	MyPath fPath(path);

	std::ifstream ifs;
	ifs.open(fPath.Path());
	if (ifs.fail())
	{
		if (fPath.Ext().empty())
		{
			fPath.SetExt(SCRIPT_EXT);
			ifs.open(fPath.Path());
		}
	}

	if (!ifs.is_open())
	{
		MAIN.printError() << "Failed to open script file: " << fPath.Path() << std::endl;
		return false;
	}

	int line(0);
	while (!ifs.eof())
	{
		std::string s;
		std::getline(ifs, s);
		if (!parseLine(s.c_str(), fPath, line))
		{
			MAIN.printError() << "Failed to parse script " << fPath.Path() << " at line " << line << std::endl;
			break;
		}
		if (!empty())
		{
			if (back() == "$abort")//abort reading a script
			{
				MAIN.printWarning() << "Script aborted by directive at line " << line + 1 << std::endl;
				pop_back();
				break;
			}
			else if (back() == "$abortif")
			{
				pop_back();
				if (mbAbortIf)
				{
					MAIN.printWarning() << " Script aborted by directive at line " <<  line + 1 << std::endl;
					break;
				}
			}
		}
		line++;
	}
	
	std::cout << "Opened script file: " << fPath.Path() << std::endl;
	mPath = fPath;

	ifs.close();
	return true;
}

bool ScriptMgr_t::parseLine(const char *line, const MyPath &fPath, int atLine)
{
	MyString sLine(line);
	int n(sLine.find(';'));
	if (n >= 0)
		sLine.truncate(n);

	sLine = sLine.stripWhiteSpace();
	if (sLine.isEmpty())
		return true;

	if (sLine.startsWith("%include"))
	{
		MyStringList l(MyStringList::split(" ", sLine));
		if (l.size() != 2)
		{
			std::cerr << fPath.Path() << "(" << atLine << ") "
				<< "Error: Ivalid include directive" << std::endl;
			return false;
		}
		l.pop_front();
		return load(l.front());
	}

	push_back(sLine);
	return true;
}
