#include "MyFileMgr.h"

#include <stdlib.h>
#include <string.h>
#include "unistd.h"
#include <assert.h>
#include <string>
#include <memory>

#include "ConvertUTF.h"

using namespace std;

static auto_ptr<MyFileMgr> gptrFileMgr(new MyFileMgr);

static MyFileMgr *gpFileMgr(gptrFileMgr.get());

////////////////////////////////////////

MyFileMgr *MyFileMgr::Instance(){ return gpFileMgr; }

MyFileMgr *MyFileMgr::Override(MyFileMgr *fileMgr)
{
	MyFileMgr *tmp(gpFileMgr);
	if (fileMgr)
	{
		gpFileMgr = fileMgr;
	}
	else
	{
		SetDefaultOS(MyFileMgr::NativeOS());
		gpFileMgr = gptrFileMgr.get();
	}
	return tmp;
}

bool MyFileMgr::gbDefaultOS(MyFileMgr::NativeOS());

MyFileMgr::MyFileMgr()
{
}

bool MyFileMgr::NativeOS()
{
#ifdef WIN32
	return true;
#else
	return false;
#endif
}

bool MyFileMgr::DefaultOS()
{
	return gbDefaultOS;
}

bool MyFileMgr::IsWindowsOS()
{
	return DefaultOS();
}

bool MyFileMgr::IsUNIxOS()
{
	return !DefaultOS();
}

bool MyFileMgr::SetDefaultOS(bool bWinOS)
{
	bool b(gbDefaultOS);
	gbDefaultOS = bWinOS;
	return b;
}

bool MyFileMgr::AutodetectOS(const char *path)
{
	if (path && path[0])
	{
		if (path[0] == '~' || path[0] == '/')
			return false;//unixOS

		if (path[1])
			if (path[1] == ':' || strncmp(path, "\\\\", 2) == 0)
				return true;//winOS
	}
	return DefaultOS();
}

bool MyFileMgr::IsRelativePath(const char *path, bool bWinOS)
{
	string s(path);
	if (s.empty())
		return true;

	if (bWinOS)
	{
		if ((s.length() > 1) && (s[1] == ':') || (s.substr(0, 2) == "\\\\") || (s.substr(0, 2) == "//"))
			return false;
		return true;
	}

	if ((s[0] == '/') || (s[0] == '~'))
		return false;

	return true;
}

////////////////////////////////////////

static bool __FileExists(const char *path)
{
#ifdef WIN32
	wchar_t wpath[1024];
	UTF8toUTF16(path, wpath, 1024);
	struct __stat64 buf;
	if (_wstat64(wpath, &buf) == 0)
		if ((buf.st_mode & S_IFMT) == S_IFREG)
			return true;
#else
	struct stat buf;
	if (stat(path, &buf) == 0)
		if ((buf.st_mode & S_IFMT) == S_IFREG)
			return true;
#endif
	return false;
}

static bool __DirExists(const char *path)
{
#ifdef WIN32
	wchar_t wpath[1024];
	UTF8toUTF16(path, wpath, 1024);
    struct __stat64 buf;
    if (_wstat64(wpath, &buf) == 0)
		if ((buf.st_mode & S_IFMT) == S_IFDIR)
			return true;
#else
    struct stat     buf;
    if (stat(path, &buf) == 0)
		if ((buf.st_mode & S_IFMT) == S_IFDIR)
			return true;
#endif

	return false;
}

static FILE *__FileOpen(const char *path, const char *mode)
{
	FILE *f = nullptr;
#ifdef WIN32
	wchar_t wpath[1024];
	UTF8toUTF16(path, wpath, 1024);
	wchar_t wmode[32];
	UTF8toUTF16(mode, wmode, 32);
	f = _wfopen(wpath, wmode);
#else
#ifdef fopen
#undef fopen
#endif
	f = fopen(path, mode);
#endif
	return f;
}

static FILE *__FileReopen(const char *path, const char *mode, FILE *f)
{
#ifdef WIN32
	wchar_t wpath[1024];
	UTF8toUTF16(path, wpath, 1024);
	wchar_t wmode[32];
	UTF8toUTF16(mode, wmode, 32);
	return _wfreopen(wpath, wmode, f);
#else
#ifdef freopen
#undef freopen
#endif
	return freopen(path, mode, f);
#endif
}

static int __FileChMod(const char *path, int mode)
{
#ifdef WIN32
	wchar_t wpath[1024];
	UTF8toUTF16(path, wpath, 1024);
	return _wchmod(wpath, mode);
#else
	return chmod(path, mode);
#endif
}

static int __FileAccess(const char *path, int mode)
{
	int ret = 0;
#ifdef WIN32
	wchar_t wpath[1024];
	UTF8toUTF16(path, wpath, 1024);
	ret = _waccess(wpath, mode);
#else
	ret = access(path, mode);
#endif
	return ret;
}

char *__FileFullPath(char *absPath, const char *relPath, size_t maxLength)
{
#ifdef WIN32
	wchar_t wRelPath[1024];
	UTF8toUTF16(relPath, wRelPath, 1024);
	wchar_t *wAbsPath = new wchar_t [maxLength];
	wchar_t *wRet(_wfullpath(wAbsPath, wRelPath, maxLength));
	if (wRet)
	{
		UTF16toUTF8(wAbsPath, absPath, (unsigned)maxLength);
		delete [] wAbsPath;
		return absPath;
	}
	delete [] wAbsPath;
	return nullptr;
#else
	return realpath(relPath, absPath);
	(void)maxLength;
#endif
}

char *__FileRealPath(const char *file_name, char *resolved_name)
{
#ifdef WIN32
	return __FileFullPath(resolved_name, file_name, _MAX_PATH);
#else
	return realpath(file_name, resolved_name);
#endif
}

static char *__FileGetCwd(char *path, int len)
{
	char *ret = nullptr;
#ifdef WIN32
	wchar_t *wpath = new wchar_t [len];
	wchar_t *wret = _wgetcwd(wpath, len);
	if (wret)
		if (UTF16toUTF8(wret, path, len))
			ret = path;
	delete [] wpath;
#else
	ret = getcwd(path, len);
#endif
	return ret;
}

static int __FileChDir(const char *dirname)
{
	int ret = 0;
#ifdef WIN32
	wchar_t wdirname[1024];
	UTF8toUTF16(dirname, wdirname, 1024);
	ret = _wchdir(wdirname);
#else
	ret = chdir(dirname);
#endif
	return ret;
}

static MyFile::fsize_t __FileSize(const char *path)
{
	if (path)
	{
#ifdef WIN32
		wchar_t wpath[1024];
		UTF8toUTF16(path, wpath, 1024);
		struct __stat64 buf;
		if (_wstat64(wpath, &buf) == 0)
			return buf.st_size;
#else
		struct stat buf;
		if (stat(path, &buf) == 0)
			return (off_t)buf.st_size;
#endif
	}
	return -1;
}

static int __MakeDir(const char *dir)
{
	if (!dir)
		return -1;

#ifndef WIN32
	return mkdir(dir, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
#else
	wchar_t wdir[1024];
	UTF8toUTF16(dir, wdir, 1024);
	return _wmkdir(wdir);
#endif
}

static int __FileUnlink(const char *path)
{
#ifndef WIN32
	return unlink(path);
#else
	wchar_t wpath[1024];
	UTF8toUTF16(path, wpath, 1024);
	return _wunlink(wpath);
#endif
}

//////////////////////////////////////////////////
// MyFile

/*MyFile::MyFile(const char *path)
: MyPath(path)
{
	assert(mbWinOS == MyFileMgr::NativeOS());
}*/

MyFile::MyFile(const MyPath &f)
: MyPath(f)
{
	assert(mbWinOS == MyFileMgr::NativeOS());
}

bool MyFile::Exists()
{
	return __FileExists(Path().c_str());
}

FILE *MyFile::Open(const char *mode)
{
	return __FileOpen(Path().c_str(), mode);
}

FILE *MyFile::Reopen(const char *mode, FILE *f)
{
	return __FileReopen(Path().c_str(), mode, f);
}

int MyFile::ChMod(const char *filename, int pmode)
{
	return __FileChMod(filename, pmode);
}

int MyFile::Access(int mode)
{
	return __FileAccess(Path().c_str(), mode);
}

MyFile::fsize_t MyFile::Size()
{
	return __FileSize(Path().c_str());
}

int MyFile::Unlink()
{
	return __FileUnlink(Path().c_str());
}

bool MyFile::DirExists()
{
	return __DirExists(Dir(true).c_str());
}

int MyFile::MakeDir()
{
	return __MakeDir(Dir(true).c_str());
}

int MyFile::ChDir()
{
	return __FileChDir(Dir(true).c_str());
}

bool MyFile::EnsureDirExists()
{
	MyFile f(*this);
	if (f.empty())
		return false;

	if (++f.begin() == f.end())
		return true;//do not test a root

	if (!f.DirExists())
	{
		string s(f.back());
		f.pop_back();
		if (!f.EnsureDirExists())
			return false;

		f.push_back(s);
		if (f.MakeDir() != 0)
			return false;
	}
	return true;
}

//////////////////////////////////////////////////
// MyFileMgr

bool MyFileMgr::FileExists(const char *path)
{
	return __FileExists(path);
}

bool MyFileMgr::DirExists(const char *dir)
{
	return __DirExists(dir);
}

FILE *MyFileMgr::FileOpen(const char *path, const char *mode)
{
	return __FileOpen(path, mode);
}

FILE *MyFileMgr::FileReopen(const char *path, const char *mode, FILE *f)
{
	return __FileReopen(path, mode, f);
}

int MyFileMgr::FileAccess(const char *path, int mode)
{
	return __FileAccess(path, mode);
}

int MyFileMgr::FileChMod(const char *path, int mode)
{
	return __FileChMod(path, mode);
}

char *MyFileMgr::FileFullPath(char *absPath, const char *relPath, size_t maxLength)
{
	return __FileFullPath(absPath, relPath, maxLength);
}

char *MyFileMgr::FileRealPath(const char *file_name, char *resolved_name)
{
	return __FileRealPath(file_name, resolved_name);
}

char *MyFileMgr::FileGetCwd(char *dir, int len)
{
	return __FileGetCwd(dir, len);
}

int MyFileMgr::FileChDir(const char *dir)
{
	return __FileChDir(dir);
}

MyFile::fsize_t MyFileMgr::FileSize(const char *path)
{
	return __FileSize(path);
}

int MyFileMgr::MakeDir(const char *dir)
{
	return __MakeDir(dir);
}

int MyFileMgr::FileUnlink(const char *path)
{
	return __FileUnlink(path);
}

#define	MAX___PATH	1024

bool MyFileMgr::GetCwd(MyPath &f)
{
	char buf[MAX___PATH];
	if (FileGetCwd(buf, MAX___PATH))
	{
		size_t l(strlen(buf));
		if (l > 0)
		{
			if (buf[l-1] != '\\' || buf[l-1] != '/')
			{
				if (l < sizeof(buf)-1)
				{
					buf[l++] = '/';
					buf[l] = 0;
				}
				else
					return false;
			}
			return f.SetDir(buf);
		}
	}
	return false;
}

bool MyFileMgr::GetHome(MyPath &f)
{
	string s;
	char *pcHome = getenv("HOME");
	if (pcHome)
		s = pcHome;
	if (s.empty() || f.SetPath(s.c_str()))
		return false;
	return true;
}


