#pragma once

#include <assert.h>
#include <algorithm>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <sstream>

namespace My {


	template <class T, class Traits, class Alloc>
	class CString : public std::basic_string<T, Traits, Alloc >
	{
	public:
		typedef std::basic_string<T, Traits, Alloc >	Base;
		CString(){}
/*		CString(const char *);
		CString(const std::string &);

		CString simplifyWhiteSpace() const;
		CString stripWhiteSpace() const;
		bool startsWith(const CString &, bool = true) const;
		bool endsWith(const CString &, bool = true) const;
		CString lower() const;
		CString upper() const;
		int contains(const char *, bool = true) const;
		bool isEmpty() const;
		const char *ascii() const;
		CString &sprintf(const char *, ...);
		CString left(unsigned len) const;
		CString right(unsigned len) const;
		CString &append(const CString &);
		CString &prepend(const CString &);
		CString &remove(unsigned, unsigned);
		CString &remove(const CString &, bool = true);
		CString &remove(char);
		CString &replace(unsigned, unsigned, const CString &);
		CString &replace(char, char);
		int find(char, int = 0) const;

		int findRev(char c, int index = -1, bool cs = true) const;
		CString mid(unsigned index, unsigned len = 0xffffffff) const;
		void truncate(unsigned newLen);

		void TrimLeft();
		void TrimRight();
		CString StripWhiteSpace() const;
		bool StartsWith(const CString &, bool = true) const;
		bool EndsWith(const CString &b, bool cs) const;*/


		CString(const char *str)
			: Base(str ? str : "")
		{
		}

		CString(const char *str, size_t n)
			: Base(str, n)
		{
		}

		/*CString(const std::string &str)
			: Base(str)
		{
		}*/

		CString(const Base &str)
			: Base(str)
		{
		}

		CString(const CString& o)
			: Base(o)
		{
		}

		CString(CString&& o)
			: Base(std::move(o))
		{
		}

		CString& operator=(const CString& x)
		{
			Base::operator=(x);
			return *this;
		}

		CString& operator=(CString&& x)   // rvalues bind here
		{
			Base::operator=(std::move(x));
			return *this;
		}

		CString simplifyWhiteSpace() const
		{
			return *this;
		}

		CString stripWhiteSpace() const
		{
			return StripWhiteSpace();
		}

		bool startsWith(const CString &s, bool cs = true) const
		{
			return StartsWith(s, cs);
		}

		bool endsWith(const CString &s, bool cs = true) const
		{
			return EndsWith(s, cs);
		}

		CString lower() const
		{
			CString s(*this);
			std::transform(s.begin(), s.end(), s.begin(), ::tolower);
			return s;
		}

		CString upper() const
		{
			CString s(*this);
			std::transform(s.begin(), s.end(), s.begin(), ::toupper);
			return s;
		}

		int contains(const char *str, bool cs = true) const
		{
			if (cs)
				return (Base::find(str) != Base::npos);
			CString s1(lower());
			CString s2(CString(str).lower());
			return (s1.Base::find(s2) != Base::npos);
		}

		bool isEmpty() const
		{
			return this->empty();
		}

		const char *ascii() const
		{
			return this->c_str();
		}

		CString &sprintf(const char *fmt, ...)
		{
			va_list va;
			va_start(va, fmt);

			char buf[0x1000];
#ifdef WIN32
			if (vsprintf_s(buf, sizeof(buf), fmt, va) < 0)
#else
			if (vsprintf(buf, fmt, va) < 0)
#endif
			{
				assert(0);
			}
			va_end(va);

			*this = buf;
			return *this;
		}

		CString left(unsigned len) const
		{
			return this->substr(0, len);
		}

		CString right(unsigned len) const
		{
			if (len < this->length())
				return this->substr(this->length() - len);
			return *this;
		}

		CString &append(const CString &s)
		{
			Base::append(s);
			return *this;
		}

		CString &prepend(const CString &s)
		{
			this->insert(0, s);
			return *this;
		}

		CString &remove(unsigned index, unsigned len)
		{
			this->erase(index, len);
			return *this;
		}

		CString &remove(const CString &s, bool cs = true)
		{
			if (!cs)
			{
				CString s0((*this).lower());
				CString s1(s.lower());
				size_t pos = 0;
				while ((pos = s0.Base::find(s1, pos)) != Base::npos)
				{
					s0.Base::erase(pos, s1.length());
					Base::erase(pos, s1.length());
				}
				return *this;
			}

			size_t pos = 0;
			while ((pos = Base::find(s, pos)) != Base::npos)
				Base::erase(pos, s.length());
			return *this;
		}

		CString &remove(char c)
		{
			size_t pos = 0;
			while ((pos = Base::find(c, pos)) != Base::npos)
				Base::erase(pos, 1);
			return *this;
		}

		CString &replace(unsigned index, unsigned len, const CString &s)
		{
			Base::replace(index, len, s);
			return *this;
		}

		CString &replace(char c1, char c2)
		{
			std::replace(this->begin(), this->end(), c1, c2);
			return *this;
		}

		int find(char c, int index = 0) const
		{
			size_t n(Base::find(c, index));
			if (n != Base::npos)
				return (int)n;
			return -1;
		}

		int find(CString s, int index = 0) const
		{
			size_t n(Base::find(s, index));
			if (n != Base::npos)
				return (int)n;
			return -1;
		}

		int findAnyOf(CString s, int index = 0) const
		{
			for (size_t i(index); i < this->length(); i++)
			{
				for (size_t j(0); j < s.length(); j++)
					if (this->at(i) == s.at(j))
						return (int)i;
			}
			return -1;
		}

		////////////////////////////////////////////////////

#define ISSPACE(c)	((!(c&0x80))&&isspace(c))

		void TrimLeft()
		{
			size_t n = 0;
			for (; n < this->length(); n++)
			{
				if (!ISSPACE((*this)[n]))
					break;
			}
			if (n > 0)
				this->erase(0, n);
		}

		void TrimRight()
		{
			size_t n = this->length();
			for (; n > 0; n--)
			{
				if (!ISSPACE((*this)[n - 1]))
					break;
			}
			this->resize(n);
		}

#undef ISSPACE

		CString StripWhiteSpace() const
		{
			CString s(*this);
			s.TrimLeft();
			s.TrimRight();
			return s;
		}

		bool StartsWith(const CString &b, bool cs = true) const
		{
			const CString &a = *this;
			unsigned i;
			size_t sz = b.size();
			if (cs)
			{
				for (i = 0; i < sz; ++i)
					if (a[i] != b[i])
						return false;
				return (i == sz);
			}

			for (i = 0; i < sz; ++i)
				if (tolower(a[i]) != tolower(b[i]))
					return false;

			return (i == sz);
		}

		bool EndsWith(const CString &b, bool cs = true) const
		{
			const CString &a = *this;
			size_t asz = a.size();
			size_t sz = b.size();
			int offset = (int)(asz - sz);
			if (offset < 0)
				return false;

			unsigned i;
			if (cs)
			{
				for (i = 0; i < sz; ++i)
					if (a[offset + i] != b[i])
						return false;
				return (i == sz);
			}

			for (i = 0; i < sz; ++i)
				if (tolower(a[offset + i]) != tolower(b[i]))
					return false;

			return (i == sz);
		}

		int findRev(char c, int = -1, bool = true) const
		{
			return (int)this->find_last_of(c);
		}

		CString mid(unsigned index, unsigned len = 0xffffffff) const
		{
			return this->substr(index, len);
		}

		void truncate(unsigned newLen)
		{
			this->resize(newLen);
		}

		void chop(unsigned by)
		{
			if (by > this->length())
				by = 0;
			this->resize(this->length() - by);
		}

		operator const char *() const { return ascii(); }
	};

	template <class T, class Traits, class Alloc>
	class CStringf : public CString<T, Traits, Alloc>
	{
	public:
		CStringf(const char *fmt, ...)
		{
			va_list va;
			va_start(va, fmt);

			char buf[0x1000];
#ifdef WIN32
			vsprintf_s(buf, sizeof(buf), fmt, va);
#else
			vsprintf(buf, fmt, va);
#endif
			va_end(va);

			this->assign(buf);
		}
	};

	class WString : public std::wstring
	{
	public:
		WString(const std::string& s)
		{
			std::wstringstream cls;
			cls << s.c_str();
			assign(cls.str());
		}
	};

}//namespace My

typedef	My::CString<char, std::char_traits<char>, std::allocator<char> >	MyString;
typedef	My::CStringf<char, std::char_traits<char>, std::allocator<char> >	MyStringf;


