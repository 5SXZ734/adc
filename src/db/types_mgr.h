#pragma once

#include "types.h"
#include "obj.h"

#include "shared/avl_tree2.h"

struct TypeInfo_t : public AVLTreeNode<TypeInfo_t, std::string>
{
	typedef	AVLTreeNode<TypeInfo_t, std::string>	Base;
	TypePtr	pSelf;
	//a circular list with a sole purpose to maintain order of elements
	TypeInfo_t	*pPrev;
	TypeInfo_t	*pNext;
	TypeInfo_t(TypePtr s, TypeInfo_t *p, TypeInfo_t *n)
		: Base(std::string()),
		pSelf(s),
		pPrev(p),
		pNext(n)
	{
	}
	int compare(const std::string &o) const
	{
		if (_key() < o)
			return -1;
		if (_key() == o)
			return 0;
		return 1;
	}
};

typedef AVLTree<TypeInfo_t>	TypesMap_t;
//typedef std::map<std::string, TypeInfo_t>	TypesMap_t;
typedef TypesMap_t::iterator	TypesMapIt;
typedef TypesMap_t::const_iterator	TypesMapCIt;

typedef TypeObj_t *	TypePtr;
///////////////////////////////////////////// TypesMgr_t
class TypesMgr_t : protected TypesMap_t
{
public:
	static int g_ID;

public:
	TypesMgr_t();
	~TypesMgr_t();
	void setOwner(TypePtr);
	TypePtr owner() const { return mpOwnerStruc; }
	bool empty() const { return TypesMap_t::empty(); }

	TypePtr getStockType(OpType_t) const;

	TypePtr getTypeCode() const { return mpTypeCode; }
	void setTypeCode(TypePtr p){ mpTypeCode = p; }

	bool isStockType(TypePtr) const;
	bool contains(TypePtr) const;
	int addTypeRef(TypePtr) const;
	int releaseTypeRef(TypePtr, bool bUnLink);
	int typeRefsNum(TypePtr) const;

	//void Print(std::ostream&, int);

	const TypesMap_t	&aliases() const { return *this; }
	//TypesMap_t &aliases(){ return mAliases; }

	TypePtr	findTypeByAlias(const MyString &) const;
	TypePtr	findTypeByNameSlow(const MyString &) const;
	TypePtr findProxyOf(CTypePtr);
	TypePtr findTypeByCompId(int) const;

	TypeInfo_t *newNameAlias(TypePtr, TypeInfo_t *pPrev = nullptr);
	TypeInfo_t *addNameAlias(TypePtr, TypeInfo_t *pPrev = nullptr);
	bool insertNameAlias(TypeInfo_t *);
	TypePtr removeNameAlias(TypePtr);//may return a proxy
	TypePtr removeNameAlias(TypeInfo_t *);

	typedef std::pair<TypePtr, std::string>	TakeElt;
	typedef std::list<TakeElt>	TakeList;
	//void takeTypes(TakeList &);
	TypePtr takeFront()
	{
		assert(!empty());
		return removeIt(begin());
	}
	void takeDerivedTypes(std::list<TypePtr> &);

	class OrderIterator
	{
		const TypeInfo_t *mp0;
		const TypeInfo_t *mp;
	public:
		OrderIterator(const TypesMgr_t &r)
			: mp0(r.head()),
			mp(mp0)
		{
		}
		operator bool() const { return (mp != 0); }
		OrderIterator& operator ++(){
			mp = mp->pNext;
			if (mp == mp0)
				mp = nullptr;
			return *this;
		}
		OrderIterator& operator ++(int){ return operator++(); }
		TypePtr operator*() const { return mp->pSelf; }
		const TypeInfo_t *typeInfo() const { return mp; }
	};

private:
	TypeInfo_t *head() const { return mpHead; }
	TypePtr removeIt(TypesMapIt);
	TypesMapCIt findDirect(TypePtr) const;

private:
	TypeInfo_t *mpHead;
	TypePtr		mpOwnerStruc;//Struc_t
	TypePtr		mpTypeCode;

#ifdef _DEBUG
	int		m_ID;
#endif
};

