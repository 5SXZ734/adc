
#pragma once

#include <string>
#include <map>

#if defined(_WIN32) || defined(WIN32)
	#include <windows.h>
    typedef HINSTANCE HDYNLIB;
    typedef FARPROC HDYNFUNC;
#else
	#include <dlfcn.h>
	#define UNIX_LIKE
    typedef void * HDYNLIB;
    typedef void * HDYNFUNC;
#endif


//////////////////////////////////////////////////////////////
// MyDynLib

class MyDynLib
{
public:
	MyDynLib()
		: m_Handle(nullptr),
		m_Refs(0)
	{
	}

	MyDynLib(const char *szLibName, int Mode)
		: m_Handle(nullptr),
		m_Refs(0)
	{
		Load(szLibName, Mode);
	}

	~MyDynLib()
	{
		Unload();
	}

public:
	HDYNLIB GetHandle() const { return m_Handle; }
	HDYNLIB Load(const char *szLibName, int _Mode = 0);
	bool Unload();
	HDYNFUNC GetFunction(const char *szFuncName);
	const std::string &GetLastErrorString() const { return mErrorString; }
public:
	unsigned m_Refs;

protected:
	void setLastErrorString(const char *szErrorString)
	{
		if (!szErrorString)
			szErrorString = "";
		mErrorString = szErrorString;
	}

#if defined(_WIN32) || defined(WIN32)
	void setLastError(DWORD dwError);
#endif

protected:
	HDYNLIB m_Handle;

private:
	std::string mErrorString;
};


#if(1)

#include "QxSingleton.h"

class DynLibMgr : public QxSingleton<DynLibMgr>
{
//	static DynLibMgr gSelf;

public:
	//static DynLibMgr *Instance();

    HDYNLIB Load(const char *szLibName);
    HDYNLIB Load(const char *szLibName, int _Mode);
 
    bool Unload(HDYNLIB handle);
    
    HDYNFUNC GetFunction(HDYNLIB handle, const char *szFuncName);
    
    const char *GetLastErrorString(HDYNLIB handle);

	DynLibMgr(){}
	~DynLibMgr()
	{
		for (Handle2MyDynLib::iterator i(mh2MgrMap.begin()); i != mh2MgrMap.end(); i++)
			delete i->second;
	}

private:
	HDYNLIB load(const char *);
    HDYNLIB load(const char *, int);
	class MyDynLib *findHandle(HDYNLIB);
	bool removeHandle(HDYNLIB);

private:
	typedef std::map<HDYNLIB, class MyDynLib *> Handle2MyDynLib;
	Handle2MyDynLib mh2MgrMap;
	std::string mLoadError;
};

typedef	HDYNLIB SLibHandle;
typedef	HDYNFUNC SLibFunction;
SLibHandle SLibLoad(const char *);
SLibFunction SLibGetFunction(SLibHandle, const char *);
bool SLibUnload(SLibHandle);

#endif



