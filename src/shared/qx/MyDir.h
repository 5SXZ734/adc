#pragma once

#include "MyPath.h"

class MyDirIterator : public MyPath
{
public:
	enum FilterSpec 
	{
		Dirs	    = 0x001,
		Files	    = 0x002,
		Drives	    = 0x004,
		NoSymLinks    = 0x008,
		All	    = 0x007,
		TypeMask	    = 0x00F,

		Readable	    = 0x010,
		Writable	    = 0x020,
		Executable    = 0x040,
		RWEMask	    = 0x070,

		Modified	    = 0x080,
		Hidden	    = 0x100,
		System	    = 0x200,
		AccessMask    = 0x3F0,

		DefaultFilter = -1
	};

	MyDirIterator(const MyPath &f)
		: MyPath(f)
	{
	}
	bool readDirEntries(int);
protected:
	virtual void OnEntry(const MyPath &f, int) = 0;
private:
#ifdef WIN32
	bool MyDirIterator::processEntry(int, wchar_t *, int);
#else
	bool processEntry(int, const char *);
#endif
};


