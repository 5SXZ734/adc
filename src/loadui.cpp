#include <iostream>
#include "qx/DynLib.h"
#include "qx/IGui.h"
#include "qx/MyString.h"
#include "qx/MyArgs.h"
#include "qx/MyPath.h"

static HDYNLIB gpUIlib = nullptr;

//#define	USE_UIQ5

static HDYNLIB __loadGuiDll(const char *file)
{
	HDYNLIB h = DynLibMgr::instance()->Load(file);
	if (!h)
		std::cerr << "Error: Could not load library: " << file << std::endl;
	return h;
}

static void __unloadGuiDll()
{
	if (gpUIlib)
	{
		DynLibMgr::instance()->Unload(gpUIlib);
		gpUIlib = nullptr;
	}
}

void UnloadGui()
{
	__unloadGuiDll();
}

typedef	My::IGui *P_CreateADCGui(int &argc, char ** argv, My::IUnk *pICore);

static MyString GetEnv(const char *var)
{
	MyString s;
#ifdef _WIN32
	char* buf(nullptr);
	size_t sz(0);
	if (_dupenv_s(&buf, &sz, var) == 0 && buf != nullptr)
	{
		s = buf;
		free(buf);
	}
#else
	char* buf(getenv(var));
	if (buf)
		s = buf;
#endif
	return s;
}

static My::IGui *LoadGui(int &argc, char **argv, My::IUnk *pICore, const char *pszFuncName)
{
	gpUIlib = nullptr;

	MyString env(GetEnv("ADCUI_PATH"));
	if (!env.empty())
		gpUIlib = __loadGuiDll(env);

#ifdef WIN32
	MyString GUI_DLL_NAME("uiqt5.dll");
#else
	MyString GUI_DLL_NAME("libuiqt5.so");
#endif

	MyArgs2 args(argc, argv);
	int i(args.Find("-gui"));
	if (i >= 0)
	{
		args.RemoveAt(i);
		if (i < (int)args.size())
		{
			GUI_DLL_NAME = args[i];
			args.RemoveAt(i);
		}
	}

	MyPath f(args[0]);
	f.SetName(GUI_DLL_NAME);
	gpUIlib = __loadGuiDll(f.Path().c_str());

	//first try to load from current dir?
	if (!gpUIlib)
		gpUIlib = __loadGuiDll(GUI_DLL_NAME);

	if (!gpUIlib)
	{
		const char *errmsg = DynLibMgr::instance()->GetLastErrorString(gpUIlib);
		if (errmsg)
			fprintf(stderr, "Error: Failed to load %s library: %s\n", GUI_DLL_NAME.c_str(), errmsg);
		return nullptr;
	}

	P_CreateADCGui *pf = (P_CreateADCGui *)DynLibMgr::instance()->GetFunction(gpUIlib, pszFuncName);
	if (!pf)
	{
		const char *errmsg = DynLibMgr::instance()->GetLastErrorString(gpUIlib);
		fprintf(stderr, "Error: Could not locate an entry point to %s: %s\n", GUI_DLL_NAME.c_str(), errmsg?errmsg:"unknown reason");
		__unloadGuiDll();
		return nullptr;
	}

	return (*pf)(argc, argv, pICore);
}

My::IGui *CreateADBGui(int &argc, char ** argv, My::IUnk *pICore)
{
	return LoadGui(argc, argv, pICore, "CreateADBGui_imp");
}

My::IGui *CreateADCGui(int &argc, char ** argv, My::IUnk *pICore)
{
	return LoadGui(argc, argv, pICore, "CreateADCGui_imp");
}



