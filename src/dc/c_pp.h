// Simple C-preprocessor
#pragma once

#include <set>
#include <vector>
#include <fstream>
#include "qx/MyPath.h"
#include "db/mem.h"
#include "db/main.h"

class C_PP_t;

//////////////////////////
class CCP_unit_t : public std::ifstream
{
	C_PP_t &m_cpp;
public:
	const MyPath	m_path;
	int m_line;

	struct cond_t
	{
		bool isTrue;//condition evaluation status
		bool processingElse;//currently processing IF block (false) or ELSE block (true)
		cond_t(bool a) : isTrue(a), processingElse(false){}//always start from IF block
	};

	std::vector<cond_t> m_condStack;//stack of IF-ELSE blocks
	CCP_unit_t(C_PP_t &r, MyPath path)
		: m_cpp(r),
		m_path(path),
		m_line(0)
	{
		open(m_path.Path());
	}
	~CCP_unit_t()
	{
		if (is_open())
			close();
	}
	bool error(const char *msg)
	{
		MAIN.printError() << "in " << m_path.Path() << ", line " << m_line << ": " << msg << std::endl;
		close();
		return false;
	}
	bool isValid() const {
		if (!is_open())
			return false;
		if (eof())
			return false;
		return true;
	}

};



//////////////////////////////////////
class C_PP_t : std::stack<CCP_unit_t *>//simple C pre-processir
{
public:
	std::set<std::string>	m_defSymbols;//defined symbols
	//MyString	mModule;
	MyString	mStatement;
	MyString	mSkipInclude;
public:
	C_PP_t();
	C_PP_t(MyPath path);
	~C_PP_t(){}
	void parse(MyPath path);
	void skipInclude(MyString s) { mSkipInclude = s; }
	void addSymbol(std::string);
	bool hasSymbol(const std::string &s) const;
	void setStatement(const MyString &s){
		mStatement = s;
	}
/*	void setModule(const MyString &s){
		mModule = s;
	}*/
	void include(const MyString &s);
	operator bool() const { return !empty(); }
	void operator ++();
	bool process();
};

