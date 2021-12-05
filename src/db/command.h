#pragma once

#include <map>
#include <sstream>
#include <set>
#include "qx/MyString.h"
#include "qx/MyArgs.h"
#include "qx/MyPath.h"

#include "locus.h"

struct Script_t;
class MyStreamBase;

template <typename T>
class RefPtr_t
{
	T* mp;
public:
	RefPtr_t()
		: mp(nullptr)
	{
	}
	RefPtr_t(const RefPtr_t& o)
		: mp(o.mp)
	{
		if (mp)
			mp->AddRef();
	}
	~RefPtr_t()
	{
		if (mp)
			mp->Release();
	}
	operator bool() const { return mp != nullptr; }
	T* set(T* p)
	{
		if (mp)
			mp->Release();
		mp = p;
		if (mp)
			mp->AddRef();
		return mp;
	}
	//const T *ptr() const { return mp; }
	T* ptr() const { return mp; }
	RefPtr_t& operator=(const RefPtr_t& o)
	{
		mp = o.mp;
		if (mp)
			mp->AddRef();
	}
	T *get() const {
		if (mp)
			mp->AddRef();
		return mp;
	}
};

struct Cmd_t : public MyArgs2
{
public:
	MyString	mResponse;
	MyString	sFile;

	MyPath		mRefPath;

	RefPtr_t<I_Context>	mcx;

	MyStreamBase	*mpResponce;

public:
	Cmd_t();
	Cmd_t(const char * command);
	//Cmd_t(const Locus_t &);
	~Cmd_t();

	void clear();
	void setContext(I_Context *ctx)
	{
		mcx.set(ctx);
	}
	I_Context	*context(){ return mcx.ptr(); }
	const I_Context	*context() const { return mcx.ptr(); }
};


const char *cmd2str(uint32_t nID);
bool cmd2id(const char *str, uint32_t &nID);

class CMDServer_t;
typedef int (*CMDServerHandlerPtr)(CMDServer_t *, Cmd_t &);

class CMDServerCommandMap : public std::map<std::string, CMDServerHandlerPtr>
{
public:
	CMDServerCommandMap(){}
	bool RegisterCommand(const char * name, CMDServerHandlerPtr pf)
	{
		std::pair<iterator, bool> ita;
		ita = insert(std::pair<std::string, CMDServerHandlerPtr>(std::string(name), pf));
		return ita.second;
	}
};

class CMDServer_t
{
public:
	CMDServer_t *	mpParent;
	CMDServerCommandMap		&mCommands;
	std::ostringstream	mos;
	int mRet;//last command's return code

public:
	CMDServer_t(CMDServerCommandMap &);
	virtual ~CMDServer_t(){}

	virtual bool ExecuteCommand(Cmd_t &);

	//virtual int PushCommand(const std::string&, bool bFront = false);
	//virtual int PopCommand(std::string&);

protected:
	static int COMMAND_cmdlist(CMDServer_t * pCMDServer, Cmd_t &);
};



