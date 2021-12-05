#include "c_pp.h"
#include <stack>
#include "qx/MyString.h"


static bool readSymbol(std::istream &is, MyString &s)
{
	s.clear();
	is >> std::ws;
	while (!is.eof())
	{
		char ch(is.peek());
		if (!iscsym(ch))
			break;
		s += ch;
		is.ignore();
	}
	s.TrimRight();
	return !s.isEmpty();
}

static bool readUntil(std::istream &is, const char *dels, MyString &s, bool eatDel = false)
{
	bool bDelimSeen(false);
	s.clear();
	is >> std::ws;
	while (!is.eof())
	{
		char ch(is.peek());
		bool delim = false;
		for (const char *p(dels); *p; p++)
			if (ch == *p)
			{
				delim = true;
				break;
			}
		if (delim)
		{
			if (eatDel)
				is.ignore();
			s.TrimRight();
			return true;
		}
		s += ch;
		is.ignore();
	}
	s.TrimRight();
	return false;
}

////////////////////////////////////// C_PP_t
C_PP_t::C_PP_t()
{
}

C_PP_t::C_PP_t(MyPath path)
{
	parse(path);
}

void C_PP_t::parse(MyPath path)
{
	push(new CCP_unit_t(*this, path));
	operator++();
}

void C_PP_t::addSymbol(std::string s)
{
	m_defSymbols.insert(s);
}

bool C_PP_t::hasSymbol(const std::string &s) const
{
	return m_defSymbols.find(s) != m_defSymbols.end();
}

void C_PP_t::include(const MyString &s)
{
	assert(!empty());
	MyPath path(s, top()->m_path);
	CCP_unit_t *pUnit(new CCP_unit_t(*this, path));
	if (pUnit->isValid())
	{
		push(pUnit);
		//operator++();
		return;
	}
	delete pUnit;
	top()->error("can't open file");
}

void C_PP_t::operator ++()
{
	while (!empty())
	{
		mStatement.clear();
		if (!top()->isValid() || !process())
		{
			delete top();
			pop();
		}
		if (!mStatement.isEmpty())
			break;
	}
}
bool C_PP_t::process()
{
	for (;;)
	{
		CCP_unit_t &cur(*top());

		if (cur.eof())
		{
			if (!cur.m_condStack.empty())
				cur.error("mismatched ifdef directive");
			cur.close();
			return false;
		}
		++cur.m_line;
		MyString s;
		std::getline(cur, s);
		size_t n(s.find("//"));//cut off the comment
		if (n != std::string::npos)
			s.resize(n);
		s.TrimLeft();
		s.TrimRight();
		if (s.empty())
			continue;
		std::istringstream ss(s);

		MyString sDirective;
		if (ss.peek() == '#')
		{
			//tokenizer_t tkn(s.c_str());
			ss.ignore();//'#'
			ss >> std::ws >> sDirective;
		}

		if (!cur.m_condStack.empty())
		{
			const CCP_unit_t::cond_t &cond(cur.m_condStack.back());
			if (!cond.isTrue)
			{
				if (!cond.processingElse)
				{
					if (sDirective != "else" && sDirective != "endif")
						continue;
				}
			}
			else
			{
				if (cond.processingElse)
				{
					if (sDirective != "endif")
						continue;
				}
			}
		}

		if (!sDirective.empty())
		{
			if (sDirective == "ifdef")
			{
				ss >> s;
				bool isTrue(hasSymbol(s));
				cur.m_condStack.push_back(CCP_unit_t::cond_t(isTrue));
				continue;
			}
			if (sDirective == "else")
			{
				if (cur.m_condStack.empty() || cur.m_condStack.back().processingElse)
					return cur.error("syntax error");
				cur.m_condStack.back().processingElse = true;
				continue;
			}
			if (sDirective == "endif")
			{
				if (cur.m_condStack.empty())
					return cur.error("sintax error");
				cur.m_condStack.pop_back();
				continue;
			}
		}

		if (!sDirective.empty())
		{
			if (sDirective == "include")
			{
				if (readUntil(ss, "<\"", s, true) && s.isEmpty()//eat delim
					&& readUntil(ss, ">\"", s, true) && !s.isEmpty())//eat delim
				{
					if (s != mSkipInclude)
						include(s);
					continue;
				}
			}
			if (sDirective == "pragma")
			{
				if (readSymbol(ss, s))
				{
					if (s == "once")
						continue;//later
					/*if (s == "module")
					{
						if (readUntil(ss, "(", s, true) && s.isEmpty()//eat delim
							&& readUntil(ss, ")", s, true) && !s.isEmpty())
						{
							setModule(s);
							continue;
						}
					}*/
				}
			}
			return cur.error("invalid directive");
		}

		setStatement(ss.str());
		break;
	}
	return true;
}






