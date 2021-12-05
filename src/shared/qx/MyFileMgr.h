#pragma once

#include <stdio.h>
#include <sys/types.h> 
#include <sys/stat.h>

#include "MyPath.h"

class MyFile : public MyPath
{
public:
	//MyFile(const char *);
	MyFile(const MyPath &);
#ifdef WIN32
	typedef __int64	fsize_t;
#else
	typedef off_t	fsize_t ;
#endif
	bool Exists();
	FILE *Open(const char *);
	int Access(int);
	fsize_t Size();
	int Unlink();
	FILE *Reopen(const char *, FILE *);
	int ChMod(const char *, int);

	bool DirExists();
	bool EnsureDirExists();
	int MakeDir();
	int ChDir();
};

class MyFileMgr
{
public:
	MyFileMgr();

	static MyFileMgr *Instance();
	static MyFileMgr *Override(MyFileMgr *);
	
	virtual bool GetCwd(MyPath &);
	virtual bool GetHome(MyPath &);

	virtual bool DirExists(const char *);
	virtual int MakeDir(const char *);
	virtual char *FileGetCwd(char *path, int);
	virtual int FileChDir(const char *);

	virtual bool FileExists(const char *);
	virtual FILE *FileOpen(const char *, const char *);
	virtual int FileAccess(const char *, int);
	virtual char *FileFullPath(char *absPath, const char *relPath, size_t maxLength);
	virtual char *FileRealPath(const char *file_name, char *resolved_name);
	virtual MyFile::fsize_t FileSize(const char *);
	virtual int FileUnlink(const char *);
	virtual FILE *FileReopen(const char *, const char *, FILE *);
	virtual int FileChMod(const char *, int);

	static bool NativeOS();
	static bool DefaultOS();
	static bool IsWindowsOS();
	static bool IsUNIxOS();
	static bool SetDefaultOS(bool);
	static bool AutodetectOS(const char *);
	static bool IsRelativePath(const char *, bool = NativeOS());
private:
	static bool gbDefaultOS;//0:win,1:unix
};


