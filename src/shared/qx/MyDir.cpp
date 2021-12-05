
#ifdef WIN32
#define UNICODE
#define _UNICODE
#include <windows.h>
#endif

#include "MyString.h"
#include "ConvertUTF.h"
#include "MyPath.h"
#include "MyDir.h"

#ifdef WIN32

bool MyDirIterator::processEntry(int filterSpec, wchar_t *cFileName, int attrib)
{
	bool doDirs	    = (filterSpec & Dirs)	!= 0;
	bool doFiles    = (filterSpec & Files)	!= 0;
	bool noSymLinks = (filterSpec & NoSymLinks) != 0;
	bool doReadable = (filterSpec & Readable)	!= 0;
	bool doWritable = (filterSpec & Writable)	!= 0;
	bool doExecable = (filterSpec & Executable) != 0;
	bool doHidden   = (filterSpec & Hidden)	!= 0;
	// show hidden files if the user asks explicitly for e.g. .*
//?	if ( !doHidden && !nameFilter.isEmpty() && nameFilter[0] == '.' )
//?		doHidden = true;
	bool doModified = (filterSpec & Modified)	!= 0;
	bool doSystem   = (filterSpec & System)	!= 0;

#undef	IS_SUBDIR
#undef	IS_RDONLY
#undef	IS_ARCH
#undef	IS_HIDDEN
#undef	IS_SYSTEM

#define IS_SUBDIR   FILE_ATTRIBUTE_DIRECTORY
#define IS_RDONLY   FILE_ATTRIBUTE_READONLY
#define IS_ARCH	    FILE_ATTRIBUTE_ARCHIVE
#define IS_HIDDEN   FILE_ATTRIBUTE_HIDDEN
#define IS_SYSTEM   FILE_ATTRIBUTE_SYSTEM

	//int  attrib = finfo.dwFileAttributes;
	bool bIsDir	= (attrib & IS_SUBDIR) != 0;
	bool bIsFile	= !bIsDir;
	bool isSymLink	= FALSE;
	bool isReadable = TRUE;
	bool isWritable = (attrib & IS_RDONLY) == 0;
	bool isExecable = FALSE;
	bool isModified = (attrib & IS_ARCH)   != 0;
	bool isHidden	= (attrib & IS_HIDDEN) != 0;
	bool isSystem	= (attrib & IS_SYSTEM) != 0;

	char fname[1024];
	if (!UTF16toUTF8(cFileName, fname, sizeof(fname)))//?
		return false;

	//?		if ( !qt_matchFilterList(filters, fname) && !(allDirs && isDir) )
	//?			continue;

	if ( (doDirs && bIsDir) || (doFiles && bIsFile) )
	{
		MyPath name(fname, *this);
		if ( doExecable )
		{
			MyString ext(name.Ext());
			if ( ext == "exe" || ext == "com" || ext == "bat" ||
				ext == "pif" || ext == "cmd" )
				isExecable = true;
		}

		if ( noSymLinks && isSymLink )
			return false;

		if ( (filterSpec & RWEMask) != 0 )
			if ( (doReadable && !isReadable) ||
				(doWritable && !isWritable) ||
				(doExecable && !isExecable) )
				return false;

		if ( doModified && !isModified )
			return false;

		if ( !doHidden && isHidden )
			return false;

		if ( !doSystem && isSystem )
			return false;

		int flags(0);
		if (bIsDir)
			flags |= Dirs;
		if (bIsFile)
			flags |= Files;
		OnEntry(name, flags);
		return true;
	}

#undef	IS_SUBDIR
#undef	IS_RDONLY
#undef	IS_ARCH
#undef	IS_HIDDEN
#undef	IS_SYSTEM
#undef	FF_ERROR

	return false;
}

bool MyDirIterator::readDirEntries(int filterSpec)
{
	if (IsNull())
		return false;

	MyPath f(*this);
	f.SetName("*.*");

	wchar_t wpath[1024];
	UTF8toUTF16(f.Path().c_str(), wpath, 1024);
	WIN32_FIND_DATA finfo;
	ZeroMemory(&finfo, sizeof(finfo));
	HANDLE ff = FindFirstFile( wpath, &finfo );

	MyString sDrive(front());

	/*bool bWildcards(Name().contains("*") || Name().contains("?"));
	if (!bWildcards)
	{
		bool bRet(processEntry(filterSpec, finfo.cFileName, finfo.dwFileAttributes));
		FindClose( ff );
		return bRet;
	}

	MyString p = Dir();
	int	plen = p.length();
	if (plen == 0)
		return false;

	if ( p.at(plen-1) != '/' && p.at(plen-1) != '\\' )
		p += '/';
	p += MyString("*.*");

	wchar_t wpath[1024];
	UTF8toUTF16(p.c_str(), wpath, 1024);
	WIN32_FIND_DATA finfo;
	HANDLE ff = FindFirstFile( wpath, &finfo );*/

#undef	FF_ERROR
#define FF_ERROR    INVALID_HANDLE_VALUE

	if ( ff == FF_ERROR )
	{
		// if it is a floppy disk drive, it might just not have a file on it
		if ( !sDrive.empty() && sDrive.at(1) == ':' &&
			( sDrive.at(0)=='A' || sDrive.at(0)=='a' || sDrive.at(0)=='B' || sDrive.at(0)=='b' ) )
		{
			return true;
		}
		return false;
	}

	bool first = true;
	for ( ;; )
	{
		if ( first )
			first = FALSE;
		else
		{
//			if (!bWildcards)
//				break;
			if (FindNextFile(ff, &finfo) == FALSE)
				break;
		}
		processEntry(filterSpec, finfo.cFileName, finfo.dwFileAttributes);
	}
	FindClose( ff );

	return true;
}

#else//WIN32

#include <sys/types.h> 

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

static bool isFile(struct stat &st)
{
    return ((st.st_mode & S_IFMT) == S_IFREG);
}

static bool isDir(struct stat &st)
{
    return ((st.st_mode & S_IFMT) == S_IFDIR);
}

static bool isSymLink(struct stat &st)
{
#if defined(S_IFLNK)
	if ( S_ISLNK( st.st_mode ) )
	    return true;
#endif
	return false;
}


static bool isReadable(const char *fn)
{
	return (::access( fn, R_OK ) == 0);
}

static bool isWritable(const char *fn)
{
	return (::access( fn, W_OK ) == 0);
}

static bool isExecutable(const char *fn)
{
	return (::access( fn, X_OK ) == 0);
} 

bool MyDirIterator::processEntry(int filterSpec, const char *d_name)
{
	bool doDirs	    = (filterSpec & Dirs)	!= 0;
	bool doFiles    = (filterSpec & Files)	!= 0;
	bool noSymLinks = (filterSpec & NoSymLinks) != 0;
	bool doReadable = (filterSpec & Readable)	!= 0;
	bool doWritable = (filterSpec & Writable)	!= 0;
	bool doExecable = (filterSpec & Executable) != 0;
	bool doHidden   = (filterSpec & Hidden)	!= 0;
	bool doSystem   = (filterSpec & System)     != 0;

	MyString fn(d_name);
	MyPath f(fn, *this);
	MyString sPath(f.Path());

	//?		if ( !qt_matchFilterList(filters, fn) && !(allDirs && fi.isDir()) )
	//?			continue;

	struct stat st;
	if (lstat(sPath.c_str(), &st) != 0)
		return false;

	bool bIsDir(isDir(st));
	bool bIsFile(isFile(st));

	if  ( (doDirs && bIsDir) || (doFiles && bIsFile) ||
		(doSystem && (!bIsFile && !bIsDir)) )
	{
		if ( noSymLinks && isSymLink(st) )
			return false;

		if ( (filterSpec & RWEMask) != 0 )
			if ( (doReadable && !isReadable(sPath.c_str())) ||
				(doWritable && !isWritable(sPath.c_str())) ||
				(doExecable && !isExecutable(sPath.c_str())) )
				return false;

		if ( !doHidden && fn[0] == '.' &&
			fn != "."
			&& fn != ".." )
			return false;

		int flags(0);
		if (bIsDir)
			flags |= Dirs;
		if (bIsFile)
			flags |= Files;
		OnEntry(f, flags);
		return true;
	}
	return false;
}

bool MyDirIterator::readDirEntries(int filterSpec)
{
	bool bWildcards(Path().contains("*") || Path().contains("?"));
	if (!bWildcards)
	{
		return processEntry(filterSpec, Path().c_str());
	}

	DIR *dir = opendir(Dir().c_str());
	if ( !dir )
		return false; // cannot read the directory

	dirent   *file;
	while ( (file = readdir(dir)) )
	{
		processEntry(filterSpec, file->d_name);
	}

	closedir(dir);
	return true;
}

#endif
