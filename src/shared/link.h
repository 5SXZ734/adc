#pragma once

#include <assert.h>
#include "defs.h"
#include <type_traits>
#include "db/mem.h"

//template <typename T> class List_t;

//bidirectional circular list's element
template <typename T>//, typename TPTR = void>
class ListNode_t// : public T
{
	//typedef	typename LinkT_t<T>	Self_t;
	//T *This(){ return static_cast<T *>(this); }
//public:
	//using SelfPtr = std::conditional_t<std::is_same<void, TPTR>::value, LinkT_t*, TPTR>;
	typedef T SelfPtr;
public:
	SelfPtr	pPrev;
	SelfPtr	pNext;
	ListNode_t() : pPrev(SelfPtr()), pNext(SelfPtr()) {}
	//LinkT_t(int n) : T(n), pPrev(SelfPtr()), pNext(SelfPtr()) {}
	~ListNode_t()
	{
		assert(!isLinked());
	}

	void clear()
	{
		pPrev = pNext = SelfPtr();
	}

	bool isLinked() const
	{
		if (pPrev && pNext)
			return true;
		assert(!pPrev && !pNext);
		return false;
	}

};








template <typename T>//, typename TPTR = void>
class List_t
{
	//typedef T DataType;
	//typedef DataType* DataPtr;
	//using EltPtr = std::conditional_t<std::is_same<void, TPTR>::value, T*, TPTR>;
	typedef T EltPtr;
	//typedef typename T::SelfPtr EltPtr;
	//typedef List_t<T, TPTR>	ListType;
	typedef List_t<T>	ListType;
public:
	EltPtr mp;
public:
	List_t() : mp(EltPtr()){}
	~List_t()
	{
		//clear(G _MemMgr);
	}
	void operator=(EltPtr p)
	{
		mp = p;
	}

	template<typename U>
	class Iterator_t
	{
		typedef ListType	MyList;
		//typedef List_t<U>	MyList;
		typedef typename MyList::EltPtr	EltPtr;
		const MyList	*mpList;
		EltPtr		mp;
	public:
		Iterator_t() : mpList(0), mp(EltPtr()){}
		Iterator_t(const MyList &r) : mpList(&r), mp(r.Head()){}
		Iterator_t(const MyList &r, EltPtr p) : mpList(&r), mp(p){ assert(!p || mpList->contains(p)); }
		Iterator_t(const Iterator_t &o): mpList(o.mpList), mp(o.mp){}
		void operator=(const Iterator_t &o){ mpList = o.mpList; mp = o.mp; }
		void assign(EltPtr p){ assert(!p || mpList->contains(p)); mp = p; }
		operator bool() const { return (mp != EltPtr()); }
		Iterator_t& operator ++(){ mp = mpList->Next(mp); return *this; }
		Iterator_t operator ++(int){//post
			Iterator_t t(*this);
			operator++(); 
			return t;
		}
//		const U& operator*() const { return *mp; }
//		U& operator*() { return *mp; }
		EltPtr data() const { return mp; }
		//EltPtr data(){ return mp; }
		void clear(){ mp = EltPtr(); }
		bool is_first() const { return mp == mpList->front(); }
		bool is_last() const { return mp == mpList->back(); }
		const MyList& list() const { return *mpList; }
		bool operator==(const Iterator_t &o) const {
			assert(mpList == o.mpList);
			return mp == o.mp;
		}
		bool operator!=(const Iterator_t &o) const { return !operator==(o); }
	};
	typedef Iterator_t<T>	Iterator;

	template<typename U>
	class ReverseIterator_t
	{
		typedef ListType	MyList;
		//typedef List_t<U>	MyList;
		typedef typename MyList::EltPtr	EltPtr;
		const MyList	*mpList;
		EltPtr		mp;
	public:
		ReverseIterator_t() : mpList(0), mp(EltPtr()){}
		ReverseIterator_t(const MyList &r) : mpList(&r), mp(r.Tail()){}
		ReverseIterator_t(const MyList &r, EltPtr p) : mpList(&r), mp(p){ assert(!p || mpList->contains(p)); }
		void operator=(const ReverseIterator_t &o){ mpList = o.mpList; mp = o.mp; }
		operator bool() const { return (mp != EltPtr()); }
		ReverseIterator_t& operator ++(){ mp = mpList->Prev(mp); return *this; }
		ReverseIterator_t& operator ++(int){ return operator++(); }
//		const U& operator*() const { return *mp; }
	//	U& operator*() { return *mp; }
		EltPtr data() const { return mp; }
		//EltPtr data(){ return mp; }
		void clear(){ mp = EltPtr(); }
		bool is_first() const { return mp == mpList->back(); }
		const MyList& list() const { return *mpList; }
	};
	typedef ReverseIterator_t<T>	ReverseIterator;


	EltPtr Next(EltPtr p) const
	{
		//const DataType &r(*p);
		assert(mp);
		assert(p->pNext);
		if (mp == p->pNext)
			return EltPtr();
		return p->pNext;
	}

	EltPtr Prev(EltPtr p) const
	{
		//const DataType &r(*p);
		assert(mp);
		assert(p->pPrev);
		if (mp == p)
			return EltPtr();
		return p->pPrev;
	}

	bool IsLast(EltPtr p) const
	{
		//const DataType &r(*p);
		assert(mp);
		assert(p->pNext);
		return (p->pNext == mp);
	}

	bool IsFirst(EltPtr p) const
	{
		assert(mp);
		assert(p);
		return (p == mp);
	}

	/*void clear(MemoryMgr_t &rMemMgr)
	{
		while (mp)
		{
			EltPtr p(mp);
			p->clear(rMemMgr);
			Unlink(p);
			rMemMgr.Delete(p);
		}
	}*/
	EltPtr Head() const { return mp; }
	EltPtr Tail() const { return mp ? mp->pPrev : EltPtr(); }
	bool IsEmpty() const { return (mp == EltPtr()); }
	bool empty() const { return (mp == EltPtr()); }

	void LinkHead(EltPtr p)
	{
		push_front(p);
	}
	void LinkTail(EltPtr p)
	{
		push_back(p);
	}
	bool contains(EltPtr p) const
	{
		return (Index(p) >= 0);
	}
	int count() const
	{
		//return mp->Count();
		int i(0);
		if (mp)
		{
			EltPtr p(mp);
			do {
				i++;
				p = p->pNext;
			} while (p != mp);
		}
		return i;
	}
	int check_count(int i) const
	{
		return check_count(mp, i);
	}

	EltPtr operator[] (int i) const//rollover if i>=count
	{
		EltPtr p(mp);
		for (; i-- != 0; p = p->pNext);
		return p;
	}

	void move_to(ListType &o)
	{
		Redirect(o);
	}

	EltPtr front() const { return mp; }
	EltPtr back() const { return mp->pPrev; }

	void push_front(EltPtr p)
	{
		//DataType &r(*p);
		assert(!p->isLinked());
		if (!mp)
		{
			mp = p;
			p->pNext = p->pPrev = p;
		}
		else
		{
			p->pNext = mp;
			p->pPrev = p->pNext->pPrev;
			p->pNext->pPrev = p;
			p->pPrev->pNext = p;
			mp = p;
		}
	}

	void pop_front()
	{
		Unlink(mp);
	}

	EltPtr take_front()
	{
		EltPtr p(mp);
		Unlink(p);
		return p;
	}

	void push_back(EltPtr p)
	{
		//DataType &r(*p);
		assert(!p->isLinked());
		if (!mp)
		{
			mp = p;
			p->pNext = p->pPrev = p;
		}
		else
		{
			p->pNext = mp;
			p->pPrev = p->pNext->pPrev;
			p->pNext->pPrev = p;
			p->pPrev->pNext = p;
		}
	}

	void pop_back()
	{
		Unlink(mp->pPrev);
	}

	void Redirect(ListType &o)
	{
		assert(this && (&o != this));
		while (mp)
		{
			EltPtr p(mp);
			Unlink(p);
			o.push_back(p);
		}
	}

	int LinkBefore(EltPtr p, EltPtr pLink)//link this before pLink
	{
		//DataType &r(*p);
		//DataType &rLink(*pLink);
		assert(!p->isLinked());
		assert(contains(pLink));
		if (mp == pLink)
			mp = p;
		pLink->pPrev->pNext = p;
		p->pPrev = pLink->pPrev;
		pLink->pPrev = p;
		p->pNext = pLink;
		return 1;
	}

	int	LinkAfter(EltPtr p, EltPtr pLink)//link this after pLink
	{
		//DataType &r(*p);
		//DataType &rLink(*pLink);
		assert(!p->isLinked());
		assert(contains(pLink));
		pLink->pNext->pPrev = p;
		p->pNext = pLink->pNext;
		pLink->pNext = p;
		p->pPrev = pLink;
		return 1;
	}

	EltPtr Replace(EltPtr p, EltPtr pLink)
	{
		if (pLink == p)
			return EltPtr();
		LinkAfter(pLink, p);//replace op
		Unlink(p);
		return p;
	}

	/*int Shift(EltPtr p, int count)
	{
		if (!p->isLinked())//not linked
			return EltPtr();

		while (count)
		{
			if (count > 0)
			{
				if (!LinkAfter(p, pNext))
					return EltPtr();
				count--;
			}
			else if (count < 0)
			{
				if (!LinkBefore(p, pPrev))
					return EltPtr();
				count++;
			}
		}

		return 1;
	}*/

	//get ptr of num-link from mp
	EltPtr Get(int i) const
	{
		EltPtr p(mp);
		if (p)
			do {
				if (i == 0)
					return p;
				i--;
				p = p->pNext;
			} while (p != mp);

		return EltPtr(); // num is too large
	}

	//<0 if less, ==0 if equal, >0 if greater
	int check_count(EltPtr p, int i) const
	{
		if (p)
		{
			EltPtr pLink(p);
			do {
				if (--i < 0)
					break;
				pLink = pLink->pNext;
			} while (pLink != p);
		}

		return -i;
	}

		//-1:p1 is first; 0:the same; p2 is first
	int CheckRelation(EltPtr p1, EltPtr p2) const
		//-2:not applicable (not chained)
	{
		assert(p1 && p2);
		if (p1 == p2)
			return 0;

		EltPtr p(mp);
		do {
			if (p == p1)
			{
				assert(index_of(p2, p1->pNext) >= 0);
				return -1;
			}
			if (p == p2)
			{
				assert(index_of(p1, p2->pNext) >= 0);
				return 1;
			}
			p = p->pNext;
		} while (p != mp);

		return -2;//neither one
	}

	int Index(EltPtr p) const { return index_of(p, mp); }

	static int index_of(EltPtr p0, EltPtr pFrom)
	{
		if (pFrom)
		{
			int i(0);
			EltPtr p(pFrom);
			do {
				if (p == p0)
					return i;
				++i;
				p = p->pNext;
			} while (p != pFrom);
		}
		return -1;//failed
	}

	int Unlink(EltPtr p)
	{
		//DataType &r(*p);
		if (!p->isLinked())
			return 0;

		assert(contains(p));
		assert(mp);

		if (p->pNext != p)
			p->pNext->pPrev = p->pPrev;

		if (p->pPrev != p)
			p->pPrev->pNext = p->pNext;

		if (mp == p)
		{
			if (p->pNext != p)
				mp = p->pNext;
			else
				mp = EltPtr();
		}

		p->pNext = p->pPrev = EltPtr();
		return 1;
	}

	EltPtr take(EltPtr p)
	{
		Unlink(p);
		return p;
	}

	void erase(Iterator &i)
	{
		Unlink(i.data());
		i.assign(0);
	}
};



/*template <typename U>
class Link_t : public LinkT_t<Link_t<U>>
{
//public:
//	U m;
public:
	Link_t()
	{
	}
//	U &data(){ return m; }
	//const U &data() const { return m; }
};*/






