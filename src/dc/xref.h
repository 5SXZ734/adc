#pragma once

#include <assert.h>

#include "shared/misc.h"
#include "shared/link.h"



//unidirectional linear list
template <typename T>
struct LinkU_t
{
	T	*pNext;
	LinkU_t() : pNext(0) {}
	T *Next() const { return pNext; }
};


template <typename T>
class XRef_t : public LinkU_t<XRef_t<T> >
{
	T	m_data;
public:
	typedef	T	DataType;
public:
	XRef_t() : m_data(0){}
	XRef_t(T a) : m_data(a){}
	T data() const { return m_data; }
	void setData(T a){ m_data = a; }
};


template <typename T>
class XRef2_t : public XRef_t<T>//in
{
	XRef_t<T>	mOut;
public:
	XRef2_t(){}
	XRef_t<T> &out(){ return mOut; }
	const XRef_t<T> &out() const { return mOut; }
};


template <typename T>
class XRefList_t
{
	typedef T	NodeType;
	typedef typename NodeType::DataType	DataType;

	typedef NodeType *	NodePtr;
	NodePtr	mpTail;
public:
	class Iterator_t
	{
		const XRefList_t&	mr;
		NodeType*		mp;
		
	public:
		Iterator_t(const XRefList_t &r) : mr(r), mp(r.head()){}
		Iterator_t(const Iterator_t &o) : mr(o.mr), mp(o.mp){}
		void operator=(const Iterator_t &o){ assert(&mr == &o.mr); mp = o.mp; }//?
		operator bool() const { return (mp != 0); }
		Iterator_t& operator ++(){ mp = mr.next(mp); return *this; }
		Iterator_t operator ++(int){//postfix
			Iterator_t t(*this);
			operator++();
			return t;
		}
		const DataType& operator*() const { return *mp->data(); }
		DataType operator*() { return mp->data(); }
		DataType data() const { return mp->data(); }
		//DataType *data(){ return mp->Op(); }
		NodeType* self() const { return mp; }
		bool operator==(const Iterator_t& o) const { assert(mr == o.mr); return mp == o.mp; }
		bool operator !=(const Iterator_t& o) const { return !operator ==(o); };
		bool is_first() const { return mp == mr.head(); }
	};
	typedef Iterator_t	Iterator;
public:
	XRefList_t() : mpTail(NodePtr()){}
	XRefList_t(NodePtr p) : mpTail(p){}
	XRefList_t(const XRefList_t &o) : mpTail(o.mpTail){}
//?	void operator=(NodeType *p){ mpTail = p; }

	//operator NodePtr () const { return mp; }
	operator bool() const{ return (mpTail != 0); }
	DataType operator[] (int i) const
	{
		NodePtr p(head());
		while (i-- > 0)
			p = next(p);
		assert(p);
		return p->data();
	}

	bool empty() const { return (mpTail == NodePtr()); }

	void push_front(NodePtr p)
	{
		if (mpTail)
		{
			p->pNext = mpTail->pNext;
			mpTail->pNext = p;
		}
		else
		{
			p->pNext = p;
			mpTail = p;
		}
	}

	void pop_front()
	{
		assert(mpTail);
		NodePtr p(mpTail->pNext);
		if (p != mpTail)
			mpTail->pNext = p->pNext;
		else
			mpTail = NodePtr();
		p->pNext = NodePtr();
	}

	void push_back(NodePtr p)
	{
		if (mpTail)
		{
			p->pNext = mpTail->pNext;
			mpTail->pNext = p;
		}
		else
			p->pNext = p;
		mpTail = p;
	}

	NodePtr lower_bound(DataType a) const//assume sorted!
	{
		NodePtr pr(nullptr);
		for (NodePtr p(head()); p; pr = p, p = next(p))
			if (!(p->data() < a))
				break;
		return pr;
	}

	NodePtr findz(DataType a) const
	{
		for (NodePtr p(head()); p; p = next(p))
			if (p->data() == a)
				return p;
		return NodePtr();
	}

	NodePtr next(NodeType *pSelf) const
	{
		//assert(pSelf && mpTail);
		if (pSelf != mpTail)
			return pSelf->pNext;
		return NodePtr();
	}

	NodePtr prev(NodeType *pSelf) const
	{
		NodePtr pr(nullptr);
		for (NodePtr p(head()); p; pr = p, p = next(p))
			if (p == pSelf)
				return pr;
		return NodePtr();//not found
	}

	NodePtr take(NodePtr pSelf, NodePtr pr)//, bool bChain = false)
	{
		if (!pr)
		{
			if (pSelf != head())
				return NodePtr();//not found
			pop_front();
		}
		else
		{
			assert(pr->pNext == pSelf);
			pr->pNext = pSelf->pNext;
			pSelf->pNext = NodePtr();
			if (mpTail == pSelf)
				mpTail = pr;
		}

		return pSelf;
	}

	NodePtr take(NodeType *p)
	{
		return take(p, prev(p));
	}

	NodePtr take_front()
	{
		NodePtr p(head());
		pop_front();
		return p;
	}

	NodePtr take(Iterator i)
	{
		return take(i.self());
	}

	void insert(NodeType *p, NodeType *pAfter)
	{
		assert(p);
		if (!pAfter)
		{
			push_front(p);
			return;
		}
		//assert(p != pAfter);
		p->pNext = pAfter->pNext;
		pAfter->pNext = p;
		if (mpTail == pAfter)
			mpTail = p;
	}

	void rearrange(NodePtr p, NodePtr pAfter)//move p after pAfter
	{
		assert(p != pAfter);
		take(p);
		insert(p, pAfter);
		/*p->pNext = pAfter->pNext;
		pAfter->pNext = p;
		if (mpTail == pAfter)
			mpTail = p;*/
	}

	unsigned count() const
	{
		assert(this);
		unsigned i(0);
		for (NodePtr p(head()); p; p = next(p))
			i++;
		return i;
	}

	int check_count(int i) const
	{
		for (NodePtr p(head()); p; p = next(p))
			if (--i < 0)
				break;
		return -i;
	}

	NodePtr head() const
	{
		if (mpTail)
			return mpTail->pNext;
		return NodePtr();
	}
	DataType front() const { return head()->data(); }
	//const DataType& front() const { return *head()->Op(); }

	NodePtr tail() const { return mpTail; }
};










