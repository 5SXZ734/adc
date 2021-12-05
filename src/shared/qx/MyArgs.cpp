#include <stdlib.h>
#include <string.h>
#include "MyArgs.h"

static void RemoveQuotesAroundFname(char** ppname)
{
	size_t len;
	char * name = *ppname;
	char * p = name;

	if (name == nullptr )
		return;

	p = name;
	while ( *p )
	{
		if ( *p != '\'' && *p != '"' && *p != '`')
			break;
		p++;
	}

	len = p-name;
	if (len > 0)
		for ( ; ; p++ ) 
		{
			p[-(long)len] = *p;
			if ( *p == '\0' )
				break;
		}

		while ( (len = strlen(name)) > 0 )
		{
			char c = name[len-1];
			if ( c != '\'' && c != '"' && c != '`')
				break;
			name[len-1] = '\0';
		}
}

//////////////////////////////////////////////
// MyArgs

MyArgs::MyArgs(char *pszFullCmdLine)
{
	char *pczcurr = nullptr;
	char *pczarg_start = nullptr;
	int narg;
	int narg_len;
	bool bquotes = false;

	/* initialize return values */
	argc = 1; /* last argv is always null */

	if(pszFullCmdLine)
	{
		pczcurr      = pszFullCmdLine;
		pczarg_start = pszFullCmdLine;
	}

	/* determine number of arguments */
	while(*pczcurr != '\0')
	{
		++argc;

		/* find next seperator */
		bquotes = (*pczcurr == '"');

		++pczcurr;
		while(   *pczcurr != '\0'
			&& (   (bquotes && *pczcurr != '"')
			|| (!bquotes && *pczcurr != ' ')))
		{
			++pczcurr;
		}

		if(bquotes)
		{
			/* don't forget closing quote */
			++pczcurr;
		}

		/* skip argument spacing */
		while(*pczcurr == ' ')
			++pczcurr;
	}

	/* create argument pointers */
	argv = new char* [argc];

	/* copy arguments */
	pczcurr  = pczarg_start;
	for(narg = 0; narg < argc - 1; ++narg)
	{
		narg_len = 1;

		/* find next seperator */
		bquotes = (*pczcurr == '"');

		++pczcurr;
		while(   *pczcurr != '\0'
			&& (   (bquotes && *pczcurr != '"')
			|| (!bquotes && *pczcurr != ' ')))
		{
			++pczcurr;
			++narg_len;
		}

		if(bquotes)
		{
			/* don't forget closing quote */
			++pczcurr;
			++narg_len;
		}

		/* skip argument spacing */
		while(*pczcurr == ' ')
			++pczcurr;

		/* copy into new buffer */
//		argv[narg] = new char [narg_len + 1];
//		strncpy(argv[narg], pczarg_start, narg_len);
		argv[narg] = pczarg_start;
		argv[narg][narg_len] = '\0';
		RemoveQuotesAroundFname(&argv[narg]);

		/* reset for next argument */
		pczarg_start = pczcurr;
	}
	argv[narg] = nullptr;
	--argc;
}

/*MyArgs::MyArgs(const MyArgs2 &m, bool bAppendNull)
{
	argc = (int)m.size();
	if (!argc)
	{
		argv = nullptr;
		return;
	}

	if (bAppendNull)
		argc++;
	
	argv = new char * [argc];
	memset(argv, 0, sizeof(char *)*argc);

	for (int i = 0; i < (int)m.size(); i++)
	{
		argv[i] = new char [m[i].length()+1];
		strcpy(argv[i], m[i].c_str());
	}
}*/

MyArgs::~MyArgs()
{
	/*for (int i = 0; i < argc; i++)
	{
		delete argv[i];
	}*/
	delete [] argv;
}

int MyArgs::Find(const char *s)
{
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(s, argv[i]) == 0)
			return i;
	}
	return -1;
}

bool MyArgs::Insert(unsigned index, char* buf)
{
	if (index > unsigned(argc) || !buf)
		return false;
	char** argv_old(argv);
	argv = new char* [argc + 1];
	unsigned i;
	for (i = 0; i < index; i++)
		argv[i] = argv_old[i];
	argv[i] = buf;
	for (; i < unsigned(argc); i++)
		argv[i + 1] = argv_old[i];
	++argc;
	delete[] argv_old;
	return true;
}

////////////////////////////////////////////////////////
// MyArgs2

MyArgs2::MyArgs2(const char *psz)
{
	Init(psz);
}

MyArgs2::MyArgs2(int argc, char **argv)
{
	for (int i = 0; i < argc; i++)
		push_back(argv[i]);
}

void MyArgs2::Add(const MyString &s)
{
	push_back(s);
}

static bool isWSpace(char c)
{
	return (c == ' ');
}

static const char *skipWSpace(const char *psz)
{
	while (*psz && isWSpace(*psz))
		++psz;
	return psz;
}

static char isQuote(char c)
{
	if (c == '\'' || c == '"' || c == '`')
		return c;
	return 0;
}

static const char *skipUntilQuote(const char *psz, char c)
{
	for (; *psz && *psz != c; psz++)
	{
		char q = isQuote(*psz);
		if (q)
			psz = skipUntilQuote(++psz, q);
	}
	return psz;
}

static const char *skipNext(const char *psz, char &q)
{
	for (; *psz && !isWSpace(*psz); psz++)
	{
		if ((q = isQuote(*psz)) != 0)
		{
			psz = skipUntilQuote(++psz, q);
			if (*psz)
				++psz;
			break;
		}
	}

	return psz;
}

void MyArgs2::Init(const char *psz)
{
	clear();
	if (psz)
	do {
		const char *psz0 = skipWSpace(psz);
		char q(0);
		psz = skipNext(psz0, q);
		size_t n = psz-psz0;
		if (n > 0)
		{
			std::string s;
			do {
				if (isQuote(*psz0))
				{
					++psz0;
					--n;
				}
				if (n > 0)
				{
					if (isQuote(*(psz - 1)))
						--n;
					if (n > 0)
					{
						s.append(std::string(psz0, n));
						psz0 += n;
					}
				}
				if (!*psz || *psz != q)
					break;
				s += q;//provision for double quotes as escaped symbols
				++psz0;
				psz = skipNext(psz0, q);
				n = psz - psz0;
			} while (n > 0);
			push_back(s);
		}
	} while (*psz);
}

static MyString checkQuotes(MyString s)
{
	if (s.find(' ') != MyString::npos)//contains spaces?
		s = "\"" + s + "\"";//wrap in quotes
	return s;
}

MyString MyArgs2::AsString(size_t from) const
{
	std::string s;
	for (size_t i(from); i < size(); i++)
	{
		if (!s.empty())
			s.append(" ");
		s.append(checkQuotes(at(i)));
	}
	return s;
}


int MyArgs2::Find(const char *s) const
{
	for (size_t i(0); i < size(); i++)
		if (std::string(s) == at(i))
			return (int)i;
	return -1;
}

size_t MyArgs2::findOpt(const char *s) const
{
	for (size_t i(0); i < size(); i++)
		if (std::string(s) == at(i))
			return i;
	return -1;
}

MyString MyArgs2::FindOpt(const char *s) const
{
	for (size_t i(0); i < size(); i++)
		if (std::string(s) == at(i))
		{
			if (i < size() - 1)
				return at(i + 1);
			break;
		}
	return "";
}

size_t MyArgs2::findOptPfx(const char *ps) const
{
	MyString s(ps);
	for (size_t i(0); i < size(); i++)
		if (at(i).startsWith(s))
			return i;
	return -1;
}

MyString MyArgs2::FindOptPfx(const char *ps) const
{
	MyString s(ps);
	for (size_t i(0); i < size(); i++)
		if (at(i).startsWith(s))
			return at(i).substr(s.length());
	return "";
}

MyString MyArgs2::RemoveOptOrPfx(const char *ps)
{
	MyString s;
	size_t i(findOpt(ps));
	if (i != -1)
	{
		if (i < size() - 1)
		{
			RemoveAt((int)i);//option
			s = at(i);
			RemoveAt((int)i);//value
		}
	}
	else
	{
		i = findOptPfx(ps);
		if (i != -1)
		{
			s = at(i).substr(std::string(ps).length());
			RemoveAt((int)i);
		}
	}
	return s;
}

bool MyArgs2::RemoveOpt(const char *ps)
{
	size_t i(findOpt(ps));
	if (i != -1)
	{
		RemoveAt((int)i);//option
		return true;
	}
	return false;
}

bool MyArgs2::RemoveOptEx(const char *ps, MyString &s)
{
	s.clear();
	size_t i(findOpt(ps));
	if (i != -1)
	{
		RemoveAt((int)i);//option
		if (i < size())
		{
			s = at(i);
			RemoveAt((int)i);//value
		}
		return true;
	}
	return false;
}

MyString MyArgs2::FindOptOrPfx(const char *ps) const
{
	MyString s(FindOpt(ps));
	if (s.empty())
		s = FindOptPfx(ps);
	return s;
}

bool MyArgs2::RemoveAt(int i)
{
	if (!((unsigned)i < size()))
		return false;
	iterator it(begin());
	while (i--)
		it++;
	erase(it);
	return true;
}

bool MyArgs2::InsertAt(int i, const char *s)
{
	if (i < 0)
		return false;
	iterator it(begin());
	while (i--)
	{
		if (it == end())
			return false;
		it++;
	}
	it = insert(it, std::string(s));
	if (it == end())
		return false;
	return true;
}


