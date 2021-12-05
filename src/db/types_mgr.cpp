#include "types_mgr.h"
#include "type_obj.h"
#include "type_proxy.h"


#ifdef _DEBUG
int TypesMgr_t::g_ID = 0;
#endif

TypesMgr_t::TypesMgr_t()
: mpHead(nullptr),
mpTypeCode(nullptr),
mpOwnerStruc(nullptr)
{
#ifdef _DEBUG
	m_ID = ++g_ID;
#endif
CHECK(m_ID == 6)
STOP
}

TypesMgr_t::~TypesMgr_t()
{
CHECK(m_ID == 8)
STOP
	assert(empty());
}

void TypesMgr_t::setOwner(TypePtr p)
{
	assert(!p || (p->typeStruc() && !p->typeProxy()));
	mpOwnerStruc = p;
//#ifdef _DEBUG
//	m_ID = p->ID();
//CHECKID(p, -60)
//STOP
//#endif
}


TypeInfo_t *TypesMgr_t::newNameAlias(TypePtr pType, TypeInfo_t *pPrev)
{
	if (!pPrev && !empty())
		pPrev = head();//add to the end

	TypeInfo_t *pSelf;
	if (pPrev)
	{
		TypeInfo_t *pNext(pPrev->pNext);
		pSelf = new TypeInfo_t(pType, pPrev, pNext);
		pPrev->pNext = pSelf;
		pNext->pPrev = pSelf;
	}
	else
	{
		assert(!mpHead);
		pSelf = new TypeInfo_t(pType, nullptr, nullptr);
		pSelf->pPrev = pSelf;
		pSelf->pNext = pSelf;
		mpHead = pSelf;
	}
	return pSelf;
}


bool TypesMgr_t::insertNameAlias(TypeInfo_t *p)
{
	TypePtr pType(p->pSelf);
	MakeAlias buf(pType);
	assert(p->_key().empty());
	p->overrideKey(buf);
	if (!insert(p).second)
		return false;
	pType->addRef();
	return true;
}

TypeInfo_t *TypesMgr_t::addNameAlias(TypePtr pType, TypeInfo_t *pPrev)
{
	TypeInfo_t *pSelf(newNameAlias(pType, pPrev));
	pType->setOwner(owner());
	if (!insertNameAlias(pSelf))
	{
		removeNameAlias(pSelf);
		pType->setOwner(nullptr);
		return nullptr;
	}
	return pSelf;
}

TypePtr TypesMgr_t::removeNameAlias(TypeInfo_t *pSelf)
{
	//TypeInfo_t *pSelf(i.pvt());
	TypeInfo_t *pPrev(pSelf->pPrev);
	TypeInfo_t *pNext(pSelf->pNext);

	TypePtr pType(pSelf->pSelf);
	assert(pType->owner() == owner());
//CHECKID(p, 168206)
//STOP

	if (mpHead == pSelf)
		mpHead = pNext;
	
	if (!empty())
	{
		pPrev->pNext = pNext;
		pNext->pPrev = pPrev;
	}
	else
		mpHead = nullptr;

	delete pSelf;
	return pType;
}

TypePtr TypesMgr_t::removeIt(TypesMapIt i)
{
	TypeInfo_t *pSelf(take(i));

	TypePtr pType(removeNameAlias(pSelf));

	pType->releaseRef();
	//pType->setOwner(nullptr);
	return pType;
}

TypePtr TypesMgr_t::removeNameAlias(TypePtr p)
{
	//assert(contains(pType));

	MakeAlias buf(p);
	TypesMapIt i(find(buf));
	if (i == end())
		return nullptr;
	
	TypePtr p2(i->pSelf);//may be a proxy
	assert(p2 == p || (p2->typeProxy() && p2->typeProxy()->incumbent() == p));
	removeIt(i);

	return p2;
}

/*void TypesMgr_t::takeTypes(TakeList &l)
{
	while (!empty())
	{
		TypePtr p(removeIt(begin()));
		p->setOwner(nullptr);//no longer owned by anyone
		MyString s;
		ProjectInfo_t::TypeNameFull2(p, s);
		l.push_back(std::make_pair(p, s));
	}
}*/

TypePtr TypesMgr_t::findProxyOf(CTypePtr p)
{
	assert(!p->typeProxy());
	MakeAlias buf(p);
	TypesMapIt i(find(buf));
	if (i == end())
		return nullptr;
	p = i->pSelf;
	if (!p->typeProxy())
		return nullptr;
	assert(p->typeProxy());
	return (TypePtr)p;
}


void TypesMgr_t::takeDerivedTypes(std::list<TypePtr> &l)
{
	for (TypesMapIt i(begin()); i != end(); )
	{
		TypePtr p(i->pSelf);

		if (!p->typeStruc()/* && !ProjectInfo_t::IsUglyLocum(p)*/)
		{
			//assert(!p->isExpo rting());
			//TypePtr iProxy0(p->stripToProxy());
			//assert(iProxy0);
			TypePtr iType0(p->absBaseType());
			assert(iType0->ownerTypeMgr() == this);

			l.push_back(p);
			removeIt(i++);
			p->setOwner(nullptr);//no longer owned by anyone

			//iProxy0->ownerTypeMgr()->addNameAlias(p);
			continue;
		}
		i++;
	}
}


TypePtr TypesMgr_t::findTypeByAlias(const MyString &s) const
{
	TypesMapCIt i(find(s));
	if (i != end())
		return i->pSelf;
	return nullptr;
}

TypesMapCIt TypesMgr_t::findDirect(TypePtr p) const
{
	for (TypesMapCIt i(begin()); i != end(); i++)
	{
		if (i->pSelf == p)
			return i;
	}
	return end();
}

TypePtr	TypesMgr_t::findTypeByNameSlow(const MyString &s) const
{
	for (TypesMapCIt i(begin()); i != end(); i++)
	{
		TypePtr iType(i->pSelf);
		if (!iType->nameless())
			if (iType->name()->c_str() == s)
				return iType;
	}
	return nullptr;
}


bool TypesMgr_t::isStockType(TypePtr pType) const
{
/*	if (pType->ObjType() == OBJID_TYPE_BIT)
		return (mStockTypes.find(OPTYP_BIT) != mStockTypes.end());*/

	MakeAlias buf;
	if (pType->ObjType() != OBJID_TYPE_SIMPLE)
	{
		if ((pType->ObjType() != OBJID_TYPE_PTR) || pType->typePtr()->pointee())
			return false;
		
		buf.forTypeSimple(OPTYP_PTR32);
		//return (mStockTypes.find(OPTYP_PTR32) != mStockTypes.end());
	}
	else
		buf.forTypeSimple(pType->typeSimple()->typeID());
	return aliases().find(buf) != aliases().end();
	//return (mStockTypes.find(t) != mStockTypes.end());
}

bool TypesMgr_t::contains(TypePtr pType) const
{
	if (isStockType(pType))
		return true;

#if(0)
	MakeAlias buf(pType);
	TypesMapCIt i(find(buf.get()));
	if (i != end())
		if (i->second.pSelf == pType)
			return true;
	return false;
#else
	return pType->owner() == owner();
#endif

//	setTYPE_cit it = types2().find(pType);
	//return it != types2().end();
}

TypePtr TypesMgr_t::findTypeByCompId(int compId) const
{
	MakeAlias buf;
	buf.forTypeComplex(compId);
	return findTypeByAlias(buf);
}

TypePtr TypesMgr_t::getStockType(OpType_t t) const
{
#if(SHARED_STOCK_TYPES)
	assert(mpOwnerStruc->typeProject());//stock types are only in root t-map
#endif

	MakeAlias buf;
	buf.forTypeSimple(t);

	MyString s(buf);

	TypesMapCIt i(aliases().find(s));
	if (i != aliases().end())
		return i->pSelf;
	return nullptr;
}

int TypesMgr_t::addTypeRef(TypePtr pType) const
{
CHECKID(pType, 17672)
STOP
	assert(contains(pType));
	return pType->addRef();
}

int TypesMgr_t::releaseTypeRef(TypePtr pType, bool bUnLink)
{
CHECKID(pType, 168206)
STOP
#ifdef _DEBUG
	if (!contains(pType))
	{
		TypesMapCIt i(findDirect(pType));
		assert(i == end());
		assert(0);
	}
#endif
	int iRefs(pType->releaseRef());
	assert(iRefs > 0);
	if (iRefs == 1)//holding a ref for itself
	{
		if (bUnLink && !isStockType(pType))
		{
CHECKID(pType, 168206)
STOP

#if(0)
			if (!removeNameAlias(pType))
				ASSERT0;
#else
				MakeAlias buf;
				pType->aka(buf);
//CHECK(strcmp(buf.get(), "$61[22]") == 0)
//STOP
				TypesMapIt i(find(buf));
				assert(i != end());
				removeIt(i);
				

				pType->setOwner(nullptr);
				//pType->releaseRef();//release a hold itself
				iRefs = 0;//was 1
#endif
		}
	}
	return iRefs;
}

int TypesMgr_t::typeRefsNum(TypePtr pType) const
{
	assert(contains(pType));
	return pType->refsNum();
}

/*TypesMgr_t *TypesMgr_t::getParentTypeMgr()
{
	if (mpTSeg->typedC())
	{
		return mpTSeg->ownerProject()->typeMgr();
	}
	return nullptr;
}*/

/*void TypesMgr_t::rebuildAliases()
{
	assert(mAliases.empty());

	for (setTYPE_cit i(mTypes2.begin()); i != mTypes2.end(); i++)
	{
		TypePtr pType(*i);
		addNam eAlias(pType);
	}

	for (StockTypesCIt_t i(mStockTypes.begin()); i != mStockTypes.end(); i++)
	{
		TypePtr pType(i->second);
		addNam eAlias(pType);
	}
}*/

