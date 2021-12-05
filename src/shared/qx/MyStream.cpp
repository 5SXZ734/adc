#include "MyStream.h"
#include <stdarg.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////
// MyStreamBase

static unsigned scan(void *p, unsigned len, const char *pDelims)
{
	if (!p || !len)
		return 0;

	if (!pDelims)
	{
		static const char ends = 0;
		if (!pDelims)
			pDelims = &ends;
	}

	for (unsigned i = 0; i < len; i++)
	{
		char c = ((char *)p)[i];
		for (const char *pDelim = pDelims; ; ++pDelim)
		{
			char delim = *pDelim;
			if (c == delim)
			{
				return i;
			}
			if (!delim)
				break;
		}
	}

	return len;
}

unsigned MyStreamUtil::WriteStringf(const char *fmt, ...)
{
	char buf[1024];
	va_list va; 
	va_start(va, fmt);
#ifdef WIN32
	vsprintf_s(buf, sizeof(buf), fmt, va); 
#else
	vsprintf(buf, fmt, va); 
#endif
	va_end(va);
	return WriteString(buf);
}

unsigned MyStreamUtil::WriteStringf0(const char *fmt, ...)
{
	char buf[1024];
	va_list va; 
	va_start(va, fmt);
#ifdef WIN32
	vsprintf_s(buf, sizeof(buf), fmt, va); 
#else
	vsprintf(buf, fmt, va); 
#endif
	va_end(va);
	return WriteString0(buf);
}

//Read

unsigned MyStreamUtil::ReadInt(int *p, int sz)
{
	return mss.Read((char *)p, ((sz < 1)?1:sz)*sizeof(int));
}

unsigned MyStreamUtil::ReadDouble(double *p, int sz)
{
	return mss.Read((char *)p, ((sz < 1)?1:sz)*sizeof(double));
}

unsigned MyStreamUtil::ReadBool(bool *p, int sz)
{
	return mss.Read((char *)p, ((sz < 1)?1:sz)*sizeof(bool));
}

unsigned MyStreamUtil::ReadChar(char *p, int sz)
{
	return mss.Read((char *)p, ((sz < 1)?1:sz)*sizeof(char));
}

char MyStreamUtil::ReadChar()
{
	char c;
	if (!mss.Read(&c, sizeof(char)))
		return 0;
	return c;
}

// Write

unsigned MyStreamUtil::WriteStream(MyStreamBase &ss, unsigned bytes, int iReset)
{
	if (bytes == 0)
		bytes = (unsigned)-1;
	unsigned total = 0;
	while (bytes > 0)
	{
		unsigned sz;
		void *pData = ss.Data(sz);
		if (!pData || !sz)
			break;
		if (sz > bytes)
			sz = bytes;
		mss.Write(pData, sz);
		ss.Skip(sz, iReset);
		total += sz;
		bytes -= sz;
	}
	return total;
}

unsigned MyStreamUtil::WriteString0(MyStreamBase &ss)
{
	unsigned bytes = 0;
	for (;;)
	{
		unsigned limit;
		void *pData = ss.Data(limit);
		if (!pData || !limit)
			break;

		unsigned sz = scan(pData, limit, nullptr);
		if (sz < limit)
		{
			mss.Write(pData, sz++);
			bytes += ss.Skip(sz);
			break;
		}

		if (sz > 0)
		{
			mss.Write(pData, sz);
			bytes += ss.Skip(sz);
		}
	}
	return bytes;
}

unsigned MyStreamUtil::WriteString(MyStreamBase &ss)
{
	unsigned bytes(WriteString0(ss));
	bytes += WriteChar(mcEol);
	return bytes;
}

unsigned MyStreamUtil::WriteString(const char *s)
{
	if (!s)
		s = "";
	unsigned len((unsigned)strlen(s));
	unsigned bytes = mss.Write((void *)s, len);
	if (bytes == len)
		bytes += WriteChar(mcEol);
	return bytes;
}

unsigned MyStreamUtil::WriteString0(const char *s)
{
	if (!s)
		s = "";
	unsigned len((unsigned)strlen(s));
	return mss.Write((void *)s, len);
}

unsigned MyStreamUtil::WriteInt(int n)
{
	return mss.Write((void *)&n, sizeof(int));
}

unsigned MyStreamUtil::WriteDouble(double d)
{
	return mss.Write((void *)&d, sizeof(double));
}

unsigned MyStreamUtil::WriteChar(char b)
{
	return mss.Write((void *)&b, sizeof(char));
}

unsigned MyStreamUtil::WriteBool(bool b)
{
	return mss.Write((void *)&b, sizeof(bool));
}

unsigned MyStreamUtil::WriteInt(const int *pn, int num)
{
	if (pn && num > 0)
		return mss.Write((void *)pn, num*sizeof(int));
	return false;
}

unsigned MyStreamUtil::WriteDouble(const double *pd, int num)
{
	if (pd && num > 0)
		return mss.Write((void *)pd, num*sizeof(double));
	return false;
}

unsigned MyStreamUtil::WriteBool(const bool *pb, int num)
{
	if (pb && num > 0)
		return mss.Write((void *)pb, num*sizeof(bool));
	return false;
}


unsigned MyStreamUtil::WriteString(const std::string &s)
{
	unsigned len((unsigned)s.length());
	unsigned bytes = mss.Write((void *)s.c_str(), len);
	if (bytes == len)
		bytes += WriteChar(eol());
	return bytes;
}

unsigned MyStreamUtil::WriteString0(const std::string &s)
{
	return mss.Write((void *)s.c_str(), (unsigned)s.length());
}

unsigned MyStreamUtil::ReadString(std::string &s, const char *delims, bool bAppendDelim)
{
	int bytes(0);
	s.resize(0);
	for (;;)
	{
		unsigned limit;
		char *pData((char *)mss.Data(limit));
		if (!pData || !limit)
			break;

		unsigned sz(scan(pData, limit, delims));
		if (sz < limit)
		{
			if (bAppendDelim && pData[sz] != 0)
				s.append(pData, ++sz);
			else
				s.append(pData, sz++);
			bytes += mss.Skip(sz);
			break;
		}

		if (sz > 0)
		{
			s.append(pData, sz);
			bytes += mss.Skip(sz);
		}
	}
	return bytes;
}


unsigned MyStreamUtil::ReadStdStream(std::istream &is, unsigned sz)
{
	unsigned uTotal(0);
	while (is.good() && sz > 0)
	{
		char buf[1024];
		std::streamsize bytes(sizeof(buf));
		if (sz < (unsigned)bytes)
			bytes = sz;
		is.read(buf, bytes);
		bytes = is.gcount();
		if (bytes > 0)
		{
			uTotal += mss.Write(buf, (unsigned)bytes);
			sz -= (unsigned)bytes;
		}
	}
	return uTotal;
}

unsigned MyStreamUtil::WriteStdStream(std::ostream &os, unsigned sz)
{
	unsigned uTotal(0);
	while (os.good())
	{
		unsigned limit;
		void *pData = mss.Data(limit);
		if (!pData || !limit)
			break;

		std::streamsize bytes(limit);
		os.write((char *)pData, bytes);
		uTotal += mss.Skip((unsigned)bytes);
	}
	return uTotal;
}

unsigned MyStreamUtil::WriteToFile(FILE *pf)
{
	unsigned total = 0;
	for (;;)
	{
		unsigned sz;
		void *pData = mss.Data(sz);
		if (!pData || !sz)
			break;
		fwrite(pData, sz, 1, pf);
		mss.Skip(sz);
		total += sz;
	}
	return total;
}

/*bool MyStreamBase::ReadFile(const char *pcFilename)
{
#ifdef WIN32
	std::ifstream ifs(FileOpen(pcFilename, "r"));
#else
	std::ifstream ifs(pcFilename);
#endif
	if (!ifs.is_open())
		return false;

	SetEndOfLineChar('\n');

	std::string line;
	while (std::getline(ifs, line))
	{
		WriteString(line);
	}

	ifs.close();
	return true;
}*/
