// //////////////////////////////////////////////////////////
// MemoryMapped.cpp
// Copyright (c) 2013 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
//

#include "mmap.h"

#include <stdexcept>
#include <cstdio>

// OS-specific
#ifdef _MSC_VER
// Windows
#include <windows.h>
#else
// Linux
// enable large file support on 32 bit systems
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#ifdef  _FILE_OFFSET_BITS
#undef  _FILE_OFFSET_BITS
#endif
#define _FILE_OFFSET_BITS 64
// and include needed headers
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#endif


/// do nothing, must use open()
MemoryMapped::MemoryMapped()
	: _filename(),
	_filesize(0),
	_hint(Normal),
	_mappedBytes(0),
	_file(0),
#ifdef _MSC_VER
	_mappedFile(nullptr),
#endif
	_mappedView(nullptr)
{
}


/// open file, mappedBytes = 0 maps the whole file
MemoryMapped::MemoryMapped(const std::string& filename, size_t mappedBytes, CacheHint hint)
	: _filename(filename),
	_filesize(0),
	_hint(hint),
	_mappedBytes(mappedBytes),
	_file(0),
#ifdef _MSC_VER
	_mappedFile(nullptr),
#endif
	_mappedView(nullptr)
{
	open(filename, mappedBytes, hint);
}


/// close file (see close() )
MemoryMapped::~MemoryMapped()
{
	close();
}


#ifdef WIN32
#include <string>
#include <stdexcept>
#include <Windows.h>

// UTF-8 -> UTF-16 (std::wstring on Windows)
static std::wstring utf8_to_wstring(const std::string& s)
{
    if (s.empty()) return {};

    int needed = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                     s.data(), (int)s.size(),
                                     nullptr, 0);
    if (needed <= 0)
        throw std::runtime_error("MultiByteToWideChar(CP_UTF8) failed");

    std::wstring w(needed, L'\0');
    int written = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                      s.data(), (int)s.size(),
                                      w.data(), needed);
    if (written != needed)
        throw std::runtime_error("MultiByteToWideChar(CP_UTF8) failed (2)");

    return w;
}

// UTF-16 -> UTF-8 (often you also want the reverse)
static std::string wstring_to_utf8(const std::wstring& w)
{
    if (w.empty()) return {};

    int needed = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                                     w.data(), (int)w.size(),
                                     nullptr, 0, nullptr, nullptr);
    if (needed <= 0)
        throw std::runtime_error("WideCharToMultiByte(CP_UTF8) failed");

    std::string s(needed, '\0');
    int written = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                                      w.data(), (int)w.size(),
                                      s.data(), needed, nullptr, nullptr);
    if (written != needed)
        throw std::runtime_error("WideCharToMultiByte(CP_UTF8) failed (2)");

    return s;
}
#endif

/// open file
bool MemoryMapped::open(const std::string& filename, size_t mappedBytes, CacheHint hint)
{
	// already open ?
	if (isValid())
		return false;

	_file = 0;
	_filesize = 0;
	_hint = hint;
#ifdef _MSC_VER
	_mappedFile = nullptr;
#endif
	_mappedView = nullptr;

#ifdef _MSC_VER
	// Windows

	DWORD winHint = 0;
	switch (_hint)
	{
	case Normal:         winHint = FILE_ATTRIBUTE_NORMAL;     break;
	case SequentialScan: winHint = FILE_FLAG_SEQUENTIAL_SCAN; break;
	case RandomAccess:   winHint = FILE_FLAG_RANDOM_ACCESS;   break;
	default: break;
	}

	// open file
#ifdef WIN32
	std::wstring wfilename = utf8_to_wstring(filename);
#else
	std::wstring wfilename = filename;
#endif
	_file = ::CreateFileW(wfilename.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, winHint, nullptr);
	if (_file == INVALID_HANDLE_VALUE)
		return false;

	// file size
	LARGE_INTEGER result;
	if (!GetFileSizeEx(_file, &result))
		return false;
	_filesize = static_cast<uint64_t>(result.QuadPart);

	// convert to mapped mode
	_mappedFile = ::CreateFileMapping(_file, nullptr, PAGE_READONLY, 0, 0, nullptr);
	if (!_mappedFile)
		return false;

#else

	// Linux

	// open file
	_file = ::open(filename.c_str(), O_RDONLY | O_LARGEFILE);
	if (_file == -1)
	{
		_file = 0;
		return false;
	}

	// file size
	struct stat64 statInfo;
	if (fstat64(_file, &statInfo) < 0)
		return false;

	_filesize = statInfo.st_size;
#endif

	// initial mapping
	remap(0, mappedBytes);

	if (!_mappedView)
		return false;

	// everything's fine
	_filename = filename;
	return true;
}


/// close file
void MemoryMapped::close()
{
	// kill pointer
	if (_mappedView)
	{
#ifdef _MSC_VER
		::UnmapViewOfFile(_mappedView);
#else
		::munmap(_mappedView, _filesize);
#endif
		_mappedView = nullptr;
	}

#ifdef _MSC_VER
	if (_mappedFile)
	{
		::CloseHandle(_mappedFile);
		_mappedFile = nullptr;
	}
#endif

	// close underlying file
	if (_file)
	{
#ifdef _MSC_VER
		::CloseHandle(_file);
#else
		::close(_file);
#endif
		_file = 0;
	}

	_filesize = 0;
}


/// access position, no range checking (faster)
unsigned char MemoryMapped::operator[](size_t offset) const
{
	return ((unsigned char*)_mappedView)[offset];
}


/// access position, including range checking
unsigned char MemoryMapped::at(size_t offset) const
{
	// checks
	if (!_mappedView)
		throw std::invalid_argument("No view mapped");
	if (offset >= _filesize)
		throw std::out_of_range("View is not large enough");

	return operator[](offset);
}


/// raw access
const unsigned char* MemoryMapped::getData() const
{
	return (const unsigned char*)_mappedView;
}


/// true, if file successfully opened
bool MemoryMapped::isValid() const
{
	return _mappedView != nullptr;
}


/// get file size
uint64_t MemoryMapped::size() const
{
	return _filesize;
}


/// get number of actually mapped bytes
size_t MemoryMapped::mappedSize() const
{
	return _mappedBytes;
}


/// replace mapping by a new one of the same file, offset MUST be a multiple of the page size
bool MemoryMapped::remap(uint64_t offset, size_t mappedBytes)
{
	if (!_file)
		return false;

	if (mappedBytes == WholeFile)
		mappedBytes = (size_t)_filesize;

	// close old mapping
	if (_mappedView)
	{
#ifdef _MSC_VER
		::UnmapViewOfFile(_mappedView);
#else
		::munmap(_mappedView, _mappedBytes);
#endif
		_mappedView = nullptr;
	}

	// don't go further than end of file
	if (offset > _filesize)
		return false;
	if (offset + mappedBytes > _filesize)
		mappedBytes = size_t(_filesize - offset);

#ifdef _MSC_VER
	// Windows

	DWORD offsetLow = DWORD(offset & 0xFFFFFFFF);
	DWORD offsetHigh = DWORD(offset >> 32);
	_mappedBytes = mappedBytes;

	// get memory address
	_mappedView = ::MapViewOfFile(_mappedFile, FILE_MAP_READ, offsetHigh, offsetLow, mappedBytes);

	if (_mappedView == nullptr)
	{
		_mappedBytes = 0;
		_mappedView = nullptr;
		return false;
	}

	return true;

#else

	// Linux
	// new mapping
	_mappedView = ::mmap64(nullptr, mappedBytes, PROT_READ, MAP_SHARED, _file, offset);
	if (_mappedView == MAP_FAILED)
	{
		_mappedBytes = 0;
		_mappedView = nullptr;
		return false;
	}

	_mappedBytes = mappedBytes;

	// tweak performance
	int linuxHint = 0;
	switch (_hint)
	{
	case Normal:         linuxHint = MADV_NORMAL;     break;
	case SequentialScan: linuxHint = MADV_SEQUENTIAL; break;
	case RandomAccess:   linuxHint = MADV_RANDOM;     break;
	default: break;
	}
	// assume that file will be accessed soon
	//linuxHint |= MADV_WILLNEED;
	// assume that file will be large
	//linuxHint |= MADV_HUGEPAGE;

	::madvise(_mappedView, _mappedBytes, linuxHint);

	return true;
#endif
}


/// get OS page size (for remap)
int MemoryMapped::getpagesize()
{
#ifdef _MSC_VER
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	return sysInfo.dwAllocationGranularity;
#else
	return sysconf(_SC_PAGESIZE); //::getpagesize();
#endif
}
