#pragma once

#include <list>
#include "qx/QxTime.h"
#include "qx/MyPath.h"
#include "qx/MyString.h"
#include "command.h"

struct Script_t;
struct Cmd_t;

class ScriptMgr_t : std::list<MyString>
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
	bool empty() const { return std::list<MyString>::empty(); }
	MyString pop() { 
		MyString s(front()); pop_front(); return s;
	}
	template <typename Producer>
	void populateBefore(Producer&& producer)// Insert producer-generated lines before current script content, preserving order.
	{
		auto pos = begin();
		auto emit = [&](const MyString& s)
			{
				pos = insert(pos, s);
				++pos;
			};
		producer(emit);
	}

private:
	bool load(const MyString &);
	bool parseLine(const char *, const MyPath &, int);
};

int CheckEmpty(char *buf);
int CutCommentOff(char *buf);



