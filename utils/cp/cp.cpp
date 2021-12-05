// cp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdarg.h>
#include <windows.h>
#include <string>
#include <algorithm>

static int error(int err, const char *fmt, ...)
{
	va_list args;
    char buf[1024];
    va_start(args, fmt);
    vsnprintf_s(buf, sizeof(buf), fmt, args);
    va_end(args);
	fprintf(stderr, buf);
	return err;
}

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return std::string(); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc == 1)
	{
		fprintf(stdout, "Usage: cp [OPTIONS] SOURCE DEST\n");
		fprintf(stdout, "\tOPTIONS:\n\t\t-f\tforce overwrite of DEST if exists\n");
		return 0;
	}
	int i(1);//skip program name
	BOOL bFailIfExists(TRUE);
	for (; i < argc && argv[i][0] == '-'; i++)
	{
		if (_tcscmp(argv[i], _T("-f")) == 0)
			bFailIfExists = FALSE;
		else
			return error(-1, "cp: (error) invalid option: %s\n", argv[i]);
	}

	if (!(i < argc))
		return error(-2, "cp: (error) no source file\n");

	std::wstring sExistingFileName = argv[i++];
	std::replace(sExistingFileName.begin(), sExistingFileName.end(), '/', '\\');

#define BUFSIZE 4096
	TCHAR  buffer[BUFSIZE]=TEXT(""); 
	TCHAR  buf[BUFSIZE]=TEXT(""); 
	TCHAR** lppPart={NULL};

	if (!GetFullPathName(sExistingFileName.c_str(), BUFSIZE, buffer, lppPart))
		return error(-11, "cp: (error) invalid SOURCE path");

	sExistingFileName = buffer;

	if (!(i < argc))
		return error(-3, "cp: (error) no target file\n");

	std::wstring sNewFileName = argv[i++];
	std::replace(sNewFileName.begin(), sNewFileName.end(), '/', '\\');

	if (!GetFullPathName(sNewFileName.c_str(), BUFSIZE, buffer, lppPart))
		return error(-11, "cp: (error) invalid DEST path");

	sNewFileName = buffer;

	if (argc < 3)
		fprintf(stderr, "cp: (warning) exra arguments ignored\n");

	if (!CopyFile(sExistingFileName.c_str(), sNewFileName.c_str(), bFailIfExists))
		return error(-9, "cp: (error) operation failed\n\t%s\n", GetLastErrorAsString().c_str());

	return 0;
}

