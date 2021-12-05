#pragma once

#include <stack>
#include <queue>
#include <algorithm>
#include <list>
#include <vector>

#include <stdlib.h>
#include <assert.h>

//#include "MyPath.h"

#include "MyString.h"

namespace My {

	void __StripDir(MyString &);
	bool __IsWindowsOS();
	bool __IsRelativePath(const char *path, bool bWinOS);
	bool __GetHome(MyString &fHome);
	bool __GetCwd(MyString &fPath);

	template <class T, class Alloc = std::allocator<T> >
	class CPath : public std::list<T, Alloc>
	{
	public:
		CPath()
			: mbWinOS(__IsWindowsOS())
		{
		}

		CPath(int bWinOS)
			: mbWinOS(bWinOS != 0)
		{
		}

		CPath(const T &fn, const CPath &refDir = CPath())
			: mbWinOS(__IsWindowsOS())
		{
			SetPath(fn, refDir);
		}

		CPath(int bWinOS, const T &fn, const CPath &refDir = CPath())
			: mbWinOS(bWinOS != 0)
		{
			SetPath(fn, refDir);
		}

		bool IsNull() const { return this->empty(); }
		const char *DirSep() const { return (mbWinOS) ? ("\\") : ("/"); }
		const char *DirSepSeq() const { return (mbWinOS) ? ("\\/") : ("/"); }

		bool SetPath(const T &rPath, const CPath &rRefDir = CPath())
		{
			T fn("*");
			if (!IsNull())
			{
				fn = this->back();
				this->clear();
			}

			T sPath(rPath);

			sPath.TrimLeft();
			sPath.TrimRight();

			if (!mbWinOS && sPath.startsWith("~"))
			{
				MyString sHome;
				if (!__GetHome(sHome))
					return false;
				sPath.replace(0, 1, sHome.ascii());
			}

			bool bRelPathGiven = __IsRelativePath(sPath.c_str(), mbWinOS);
			if (bRelPathGiven)
			{
				T sRefDir;
				if (rRefDir.IsNull())
				{
					MyString s;
					if (!__GetCwd(s))
						return false;
					sRefDir = s.ascii();
				}
				else
				{
					assert(mbWinOS == rRefDir.mbWinOS);
					sRefDir = rRefDir.Dir();
				}

				sPath.insert(0, sRefDir);
			}

			size_t found(sPath.find_first_not_of(DirSepSeq()));//skip leading dir sep(s)
			if (found != T::npos)
			{
				found = sPath.find_first_of(DirSepSeq(), found);
				while (found != T::npos)
				{
					T item(sPath.left((unsigned)found));
					if (!item.empty())
					{
						if (item != ".")
						{
							if (item == "..")
							{
								if (IsNull())
									return false;//error!
								this->pop_back();
							}
							else
								this->push_back(item);
						}
					}

					sPath.erase(0, found + 1);

					found = sPath.find_first_of(DirSepSeq());
				}
			}

			if (!sPath.empty())
				this->push_back(sPath);
			else if (!fn.empty())//dir was specified
				this->push_back(fn);

			return true;
		}

		typedef typename std::list<T, Alloc>::const_iterator	const_iterator;
		typedef typename std::list<T, Alloc>::iterator iterator;

		T Path() const
		{
			T s;
			if (!IsNull())
			{
				const_iterator it = this->begin();
				do {
					const T &item = *it;
					/*			if (mbWinOS)
					{
					if (it == begin())
					{
					if ((item.length() > 1) && (item[item.length()-1] != ':'))
					s = ("\\\\");
					}
					else
					s.append(DirSep());
					}
					else
					{
					s.append(DirSep());
					}*/
					if (it != this->begin())
						s.append(DirSep());
					s.append(item);
					it++;
				} while (it != this->end());
			}
			return s;
		}

		T Name() const
		{
			if (IsNull())
				return "";
			return this->back();
		}

		bool SetName(const char *name)
		{
			if (!IsNull())
				this->pop_back();
			if (name)
				this->push_back(name);
			return true;
		}

		T Ext() const
		{
			T s(this->back());
			size_t n(s.find_last_of('.'));
			if (n != T::npos)
				return s.substr(n + 1);
			s.clear();
			return s;
		}

		bool SetExt(const char *ext)
		{
			if (IsNull())
				return false;

			T s(this->back());
			this->pop_back();

			size_t n(s.find_last_of('.'));
			if (n != T::npos)
				s.resize(n);

			if (ext)
			{
				if (*ext != '.')
					s.append(".");
				s.append(ext);
			}

			this->push_back(s);
			return true;
		}

		T Basename() const
		{
			T s(this->back());
			size_t n(s.find_last_of('.'));
			if (n != T::npos)
				return s.substr(0, n);
			return s;
		}

		bool SetBasename(const char *fn)
		{
			if (IsNull())
				return false;

			T s(this->back());
			this->pop_back();

			T name;
			if (fn)
				name = fn;

			size_t n(s.find_last_of('.'));
			if (n != T::npos)
				s.replace(0, (unsigned)n, name);
			else
				s = name;

			this->push_back(s);
			return true;
		}

		T Dir(bool bStripSep = false) const
		{
			T s;
			if (!IsNull())
			{
				for (const_iterator it(this->begin()); it != --this->end(); it++)
				{
					//			if (!mbWinOS || (it != begin()))
					if (it != this->begin())
						s.append(DirSep());
					const T &r = *it;
					s.append(r);
				};

				if (!s.empty() && !bStripSep)
					s.append(DirSep());
			}

			return s;
		}

		bool SetDir(const char *pcDir);

		bool StartsWith(const CPath &f0) const
		{
			CPath f(f0);
			if (!f.empty())
				f.pop_back();

			const_iterator it1(this->begin());
			const_iterator it2(f.begin());
			if (it2 == f.end())
				return false;

			while (it1 != this->end() && it2 != f.end())
			{
				if (compare(*it1, *it2) != 0)
					return false;
				it1++;
				it2++;
			}

			return (it2 == f.end());
		}

		bool Splice(const CPath &f0, const CPath &g0)
		{
			CPath f(f0);
			if (!f.empty())
				f.pop_back();

			iterator it1(this->begin());
			const_iterator it2(f.begin());
			if (it2 == f.end())
				return false;

			while (it1 != this->end() && it2 != f.end())
			{
				if (compare(*it1, *it2) != 0)
					return false;
				it1++;
				it2++;
			}

			if (it2 != f.end())
				return false;

			this->erase(this->begin(), it1);
			this->insert(this->begin(), g0.begin(), --g0.end());

			mbWinOS = g0.mbWinOS;
			return true;
		}

		bool Cygwin()
		{
			if (IsNull() || !mbWinOS)
				return false;

			T &s(this->front());
			std::transform(s.begin(), s.end(), s.begin(), ::tolower);
			this->push_front("cygdrive");

			mbWinOS = false;
			return true;
		}

		T RelPath(const CPath &rRefDir) const
		{
			//	if (rRefDir.IsNull())
			//		return CPath();

			//	MyString sRefDir(refDir);
			//	if (sRefDir.find_last_of(DirSepSeq()) == sRefDir.length()-1)
			//		sRefDir.append("<dummy>");//dummy file name

			CPath fRef(rRefDir.Path());//sRefDir.c_str());

			const_iterator it = this->begin();
			iterator itRef = fRef.begin();
			if (compare(*it, *itRef) != 0)
				return Path();

			do {
				it++;
				if (it == this->end())
				{
					it--;
					break;
				}
				itRef++;
				if (itRef == fRef.end())
				{
					it--;
					itRef--;
					break;
				}
			} while (compare(*it, *itRef) == 0);

			T s;

			while (++itRef != fRef.end())
			{
				if (!s.empty())
					s.append(DirSep());
				s.append("..");
			}

			do {
				const T &item = *it;
				if (!s.empty())
					s.append(DirSep());
				s.append(item);
				it++;
			} while (it != this->end());

			return s;
		}

		CPath & operator= (const CPath &o)
		{
			if (this != &o)
			{
				this->assign(o.begin(), o.end());
				mbWinOS = o.mbWinOS;
			}
			return *this;
		}

		bool operator==(const CPath &o) const
		{
			if (mbWinOS != o.mbWinOS)
				return false;

			const_iterator it, it2;
			for (it = this->begin(), it2 = o.begin(); it != this->end() && it2 != o.end(); it++, it2++)
			{
				if (compare(*it, *it2) != 0)
					return false;
			}
			return (it == this->end() && it2 == o.end());
		}

		bool operator<(const CPath &o) const
		{
			const_iterator it1, it2;
			for (it1 = this->begin(), it2 = o.begin(); it1 != this->end() && it2 != o.end(); ++it1, ++it2)
				if (compare(*it1, *it2) < 0)
					return true;
				else if (compare(*it2, *it1) < 0)
					return (false);

			return (it1 == this->end() && it2 != o.end());
		}

	private:
		int compare(T s1, T s2) const
		{
			if (mbWinOS)
			{
				std::transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
				std::transform(s2.begin(), s2.end(), s2.begin(), ::tolower);
			}
			return s1.compare(s2);
		}

	protected:
		bool mbWinOS;
	};

	MyString __DirPath(MyString s);

	template <class T, class Alloc = std::allocator<T> >
	class DirPath : public CPath<T, Alloc>
	{
		typedef CPath<T, Alloc> Base;
	public:
		DirPath()
		{
		}

		DirPath(int iOS)
			: Base(iOS)
		{
		}

		DirPath(const MyString &path, const Base &refDir = Base())
			: Base(__DirPath(path), refDir)
		{
		}

		DirPath(int iOS, const MyString &path, const Base &refDir = Base())
			: Base(iOS, __DirPath(path), refDir)
		{
		}
	};

	template <class T, class Alloc>
	bool CPath<T, Alloc>::SetDir(const char *pcDir)
	{
		T s;
		if (pcDir)
		{
			DirPath<T, Alloc> f(pcDir);
			s = f.Dir();
		}
		SetPath(s);
		return true;
	}


}//namespace

typedef	My::CPath<MyString>	MyPath;
typedef	My::DirPath<MyString>	MyDirPath;


