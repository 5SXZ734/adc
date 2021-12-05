#pragma once

#include <list>
#include "qx/QxTime.h"
#include "qx/MyPath.h"
#include "qx/MyString.h"
#include "command.h"

struct Script_t;
struct Cmd_t;

class ScriptMgr_t : public std::list<MyString>
{
	MyPath	mPath;
	bool	mbAbortIf;
	RefPtr_t<I_Context>	mcx;
	QxTime m_timer;
public:
	ScriptMgr_t() : mbAbortIf(false){}
	ScriptMgr_t(const MyString &path, bool bAbortIf);
	~ScriptMgr_t();
	const MyPath &path(){ return mPath; }
	void setContext(I_Context *p){ mcx.set(p); }
	I_Context	*context(){ return mcx.ptr(); }
private:
	bool load(const MyString &);
	bool parseLine(const char *, const MyPath &, int);
};

int CheckEmpty(char *buf);
int CutCommentOff(char *buf);



