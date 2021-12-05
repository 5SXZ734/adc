
#include <assert.h>

#include "MyPath.h"
#include "MyFileMgr.h"
#include "DynLib.h"

using namespace std;

#ifndef WIN32
	#define DLL_EXT	"so"
#else
	#define DLL_EXT	"dll"
#endif


HDYNLIB MyDynLib::Load(const char *szLibName, int Mode)
{
	assert(!m_Handle);
	assert(szLibName && *szLibName);

	MyPath f(MyFileMgr::NativeOS(), szLibName);

#ifndef WIN32
	std::string sPath(f.Path());
	const char *pcPath(sPath.c_str());
	m_Handle = dlopen(sPath.c_str(), RTLD_LAZY|Mode);
	if (!m_Handle)
		setLastErrorString(dlerror());

#else

	// Two step Load to avoid Windows error message box to appear when 
	// doing a LoadLibrary on a file which binary is not an executable code
	// (ie PE code)
	// The LoadLibraryEx with LOAD_LIBRARY_AS_DATAFILE does not display dialog box
	// in case of error; but only returns a nullptr handle.

	std::string sPath(f.Path());
	if (f.Ext().empty())
		sPath.append(".");

	m_Handle = LoadLibraryEx(sPath.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);
	if (m_Handle)
		// If LoadLibraryEx succeed, we are sure that szLibName2 is a DLL like file
	{
		// Free the pseudo loaded DLL
		FreeLibrary(m_Handle);

		// Really load the DLL
		m_Handle = LoadLibrary(sPath.c_str());
	}

	if (!m_Handle)
		setLastError(GetLastError());

#endif

	if (!m_Handle)
	{
		if (f.Ext().empty())
		{
			f.SetExt(DLL_EXT);
			return Load(f.Path().c_str(), Mode);
		}
	}

	return m_Handle;
}

bool MyDynLib::Unload()
{
	if (m_Handle)
	{
#ifndef WIN32

		if (dlclose(m_Handle))
			setLastErrorString(dlerror());
		else
			m_Handle = nullptr;

#else

		if (!FreeLibrary((HMODULE)m_Handle))
			// An error occur
			setLastError(GetLastError());
		else
			m_Handle = nullptr;

#endif
	}

	// If the method suucced, m_Handle becomes nullptr
	return m_Handle ? false : true;
}


HDYNFUNC MyDynLib::GetFunction(const char *szFuncName)
{
	assert(szFuncName && *szFuncName);

	HDYNFUNC pFunc;

#ifndef WIN32

	pFunc = dlsym(m_Handle, szFuncName);
	if (!pFunc)
		setLastErrorString(dlerror());

#else

	pFunc = GetProcAddress(m_Handle, szFuncName);
	if (!pFunc)
		setLastError(GetLastError());

#endif

	return pFunc;
}

#if defined(_WIN32) || defined(WIN32)
void MyDynLib::setLastError(DWORD dwError)
{
	LPVOID lpMsgBuf;
	FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER 
		| FORMAT_MESSAGE_FROM_SYSTEM 
		| FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		nullptr 
		);

	setLastErrorString((char *)lpMsgBuf);

	LocalFree(lpMsgBuf);
}
#endif




#if (1)



//////////////////////////////////////////////////////////////////////////////
// DynLibMgr

//DynLibMgr DynLibMgr::gSelf;

//DynLibMgr *DynLibMgr::Instance()
//{
	//return &gSelf;
//}

MyDynLib *DynLibMgr::findHandle(HDYNLIB handle)
{
	Handle2MyDynLib::iterator p = mh2MgrMap.find(handle);
	return p != mh2MgrMap.end() ? p->second : nullptr;
}

bool DynLibMgr::removeHandle(HDYNLIB handle)
{
	Handle2MyDynLib::iterator p = mh2MgrMap.find(handle);

	bool bFound = p != mh2MgrMap.end();
	if (bFound)
	{
		//MyDynLib *pMgr(p->second);
		mh2MgrMap.erase(p);
		//delete pMgr;
	}

	return bFound;
}


HDYNLIB DynLibMgr::Load(const char *szLibName)
{
	return load(szLibName, 0);
}

HDYNLIB DynLibMgr::Load(const char *szLibName, int Mode)
{
	return load(szLibName, Mode);
}

HDYNLIB DynLibMgr::load(const char *szLibName)
{
	return load(szLibName, 0);
}

HDYNLIB DynLibMgr::load(const char *szLibName, int _Mode)
{
	MyDynLib *pMgr = new MyDynLib(szLibName, _Mode);

	HDYNLIB handle = pMgr->GetHandle();

	if (handle)
	{
		MyDynLib *pExistingSLibMgr;

		if (!(pExistingSLibMgr = findHandle(handle)))
			// If this handle do not exist, add it to the map

			mh2MgrMap[handle] = pMgr;

		else
			// If this handle already exist, just delete the newly created object
			// and consider the one found in the map.
			// In order the SLibUnload knows how many occurence of Load had been 
			// done, we increment a counter. Unload will destroy the object when 
			// the counter fall to zero.
		{
			delete pMgr;

			pMgr = pExistingSLibMgr;

			pMgr->m_Refs++;

			handle = pMgr->GetHandle();
		}
	}
	else
	{
		mLoadError = pMgr->GetLastErrorString();

		// Can now delete the object
		delete pMgr;

		// No lib loaded; no handle returned!
		handle = nullptr;
	}

	return handle;
}

bool DynLibMgr::Unload(HDYNLIB handle)
{
	assert(handle);

	bool bRet = false;

	MyDynLib *pExistingSLibMgr = findHandle(handle);

	if (pExistingSLibMgr)
	{
		if (pExistingSLibMgr->m_Refs > 0)
			if (--pExistingSLibMgr->m_Refs == 0)
			{
				bRet = pExistingSLibMgr->Unload();

				removeHandle(handle);

				delete pExistingSLibMgr;
			}
	}

	return bRet;
}

HDYNFUNC DynLibMgr::GetFunction(HDYNLIB handle, const char *szFuncName)
{
	assert(handle);
	assert(szFuncName && *szFuncName);

	MyDynLib *pExistingSLibMgr = findHandle(handle);

	if (pExistingSLibMgr)
	{
		HDYNFUNC pf(pExistingSLibMgr->GetFunction(szFuncName));
		if (pf)
			pExistingSLibMgr->m_Refs++;
		return pf;
	}

	return nullptr;
}

const char *DynLibMgr::GetLastErrorString(HDYNLIB handle)
{
	if (!handle)
	{
		if (!mLoadError.empty())
			return mLoadError.empty()?nullptr:mLoadError.c_str();

		return "Invalid (nullptr) HDYNLIB.";
	}

	MyDynLib *pExistingSLibMgr = findHandle(handle);

	if (pExistingSLibMgr)
		return pExistingSLibMgr->GetLastErrorString().empty()?
			nullptr:pExistingSLibMgr->GetLastErrorString().c_str();

	return "Invalid HDYNLIB.";
}

/*

SLibHandle SLibLoad(const char *path)
{
	return (SLibHandle)DynLibMgr::Instance()->Load(path);
}

SLibFunction SLibGetFunction(SLibHandle hLib, const char *func)
{
	return (SLibFunction)DynLibMgr::Instance()->GetFunction((HDYNLIB)hLib, func);
}

bool SLibUnload(SLibHandle hLib)
{
	return DynLibMgr::Instance()->Unload((HDYNLIB)hLib);
}
*/

#endif

