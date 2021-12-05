#pragma once

#include "MyString.h"
#include <list>

namespace My {

	class CStringList : public std::list<MyString>
	{
	public:
		/*CStringList(){}
		unsigned count() const;
		unsigned contains(const MyString &) const;
		iterator append(const MyString &);
		iterator prepend(const MyString &);
		MyString join(const MyString &sep) const;

		static MyStringList split(const MyString &, const MyString &, bool = false);*/
		typedef iterator	Iterator;

		unsigned count() const
		{
			return (unsigned)size();
		}

		unsigned contains(const MyString &s) const
		{
			unsigned count = 0;
			for (const_iterator it = begin(); it != end(); it++)
			{
				if (*it == s)
					++count;
			}
			return count;
		}

		iterator append(const MyString &s)
		{
			push_back(s);
			return --end();
		}

		iterator prepend(const MyString &s)
		{
			push_front(s);
			return begin();
		}

		static CStringList split(const MyString &sep, const MyString &str, bool allowEmptyEntries = false);

		MyString join(const MyString &sep) const
		{
			MyString res;
			bool alredy = false;
			for (const_iterator it = begin(); it != end(); ++it)
			{
				if (alredy)
					res += sep;
				alredy = true;
				res += *it;
			}

			return res;
		}

		MyString &operator[](int i)
		{
			for (iterator it(begin()); it != end(); it++, i--)
			{
				if (!i)
					return *it;
			}
			return *end();//?avoid this
		}

	};
}

typedef My::CStringList MyStringList;


