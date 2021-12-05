
#include "MyPath.h"
#include "MyFileMgr.h"

namespace My {

	void __StripDir(MyString &s)
	{
		while (!s.empty() && (s[s.length() - 1] == '/' || s[s.length() - 1] == '\\'))
			s.resize(s.length() - 1);
	}

	MyString __DirPath(MyString s)
	{
		if (!s.empty())
		{
			if (!s.endsWith("\\") && !s.endsWith("/"))
				s.append("/");
		}
		return s;
	}

	bool __IsWindowsOS()
	{
		return MyFileMgr::IsWindowsOS();
	}

	bool __GetHome(MyString &sHome)
	{
		MyPath f;
		if (!MyFileMgr::Instance()->GetHome(f))
			return false;
		sHome = f.Dir();
		return true;
	}

	bool __IsRelativePath(const char *path, bool bWinOS)
	{
		return MyFileMgr::IsRelativePath(path, bWinOS);
	}

	bool __GetCwd(MyString &sDir)
	{
		MyPath f;
		if (!MyFileMgr::Instance()->GetCwd(f))
			return false;
		sDir = f.Dir();
		return true;
	}

}