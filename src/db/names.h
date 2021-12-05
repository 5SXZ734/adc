#pragma once

#define SBTREE_NAMES	1

#if(!SBTREE_NAMES)
#include <map>
#else
#include "shared/sbtree2.h"
#endif

#include "qx/MyString.h"
#include "obj.h"
#include "shared/misc.h"

#define SEPA_NAMPFX1	'['
#define SEPA_NAMPFX2	']'
#define SEPA_NAMPFX0	"[]"	//nonames
#define SEPA_TAG		'#'
#define	SEPA_MODID		'|'

typedef const char*		CNamePtr;
typedef char*		NamePtr;

class NameRef_t : public sbtree::node_s<NameRef_t, NamePtr>
#if(SHARED_NAMES)
	: public RefCounter_t
#endif
{
	typedef sbtree::node_s<NameRef_t, NamePtr> Base_t;
private:
	ObjPtr	mpObj;
public:
	NameRef_t() : mpObj(nullptr) {}
	NameRef_t(NamePtr p) : Base_t(p), mpObj(nullptr) {}// setPvt(p); }
	NameRef_t(CNamePtr p) : Base_t(copyof(p)), mpObj(nullptr) {}// setPvt(p); }
	NameRef_t(NameRef_t&& o) : Base_t(std::move(o)), mpObj(o.mpObj) { mpObj = nullptr; }//?
	//NameRef_t(const NameRef_t& o) : Base_t(o), mpObj(o.mpObj){}
	~NameRef_t() {
#if(SHARED_NAMES)
		assert(!refsNum());
#endif
		delete [] key; }
	const char *c_str() const { return /*skipLenPfx*/(key); }
	//NamePtr key_() const { return key; }
	ObjPtr obj() const { return mpObj; }
	void setObj(ObjPtr p) { mpObj = p; }
	//void setPvt0(char *p){ mp = p; }
	//void clearPvt(){ mp = nullptr; }
	//void setPvt(const char *);
	static char* copyof(const char* pc)
	{
		char* p(new char[strlen(pc) + 1]);
		strcpy(p, pc);
		return p;
	}
	void overrideKey_(const char* pc) { overrideKey(copyof(pc)); }
#define mystrcmp	strcmp2
	struct CompObj {
		bool operator()(CNamePtr pa, CNamePtr pb) const
		{
			const char* a(skipLenPfx(pa));
			const char* b(skipLenPfx(pb));
			return (mystrcmp(a, b) < 0);
		}
	};
	bool operator<(const NameRef_t& o) const
	{
		const char* a(skipLenPfx(key));
		const char* b(skipLenPfx(o.key));
		return (mystrcmp(a, b) < 0);
	}
	/*bool operator<(const char *pc) const
	{
		return (mystrcmp(mp, pc) < 0);
	}
	bool operator==(const char *pc) const
	{
		return (mystrcmp(mp, pc) == 0);
	}*/
#undef mystrcmp
	//operator const char *() const { return mp; }
	bool theSame(const NameRef_t &o) const { return key == o.key; }
private:
	//case-preferred compare: Ab,aC,Bb,bA
	inline static int strcmp1(const char *a, const char *b)
	{
		while (*a)
		{
			if (*a != *b)
				break;
			a++;
			b++;
		}
		int x(tolower(*a));
		int y(tolower(*b));
		if (x == y)
			return *(const unsigned char *)a - *(const unsigned char *)b;
		return x - y;
	}
	//lexicographical compare: aC,Ab,bA,Bb
	inline static int strcmp2(const char *a, const char *b)
	{
#ifdef WIN32
		int res(stricmp(a, b));
#else
		int res(strcasecmp(a, b));
#endif
		if (res == 0)
			res = strcmp(a, b);
		return res;
	}
public:
	static const char* skipLenPfx(const char *pc, int& n)//some entries may be prepended with length info (TODO: unsigned LEB128? for all)
	{
#if(1)
		n = -1;//no length prefix
		if (*pc != SEPA_NAMPFX1)
			return pc;
		++n;//reset
		while (*(++pc) != SEPA_NAMPFX2)//exported/imported entries must be named but may not have a name portion (tag only)
		{
			assert(*pc != 0);
			n *= 10;
			n += (*pc) - '0';
		}
		return ++pc;//skip SEPA_NAMPFX2
#else
		if (pc)
		{
			size_t l(::skip_LEB128((const uint8_t*)pc));
			n = (int)l;
			assert(l > 0);
			return pc + l;
		}
		n = 0;
		return nullptr;
#endif
	}
	static const char* skipLenPfx(const char *pc)
	{
#if(1)
		if (*pc != SEPA_NAMPFX1)
			return pc;
		while (*(++pc) != SEPA_NAMPFX2);
		return ++pc;
#else
		if (pc)
		{
			size_t l(::skip_LEB128((const uint8_t*)pc));
			assert(l > 0);
			return pc + l;
		}
		return nullptr;
#endif
	}
	const char *name(int& n) const {
		const char* pc(skipLenPfx(key, n));
		if (n == 0)
			return nullptr;
		return pc;
	}
	const char* name() const {
		int n;
		const char* pc(skipLenPfx(key, n));
		if (n == 0)
			return nullptr;
		return pc;
	}
	static const char* skipToTag(const char *pc)//extract
	{
		int n;
		pc = skipLenPfx(pc, n);
		if (n < 0)//not a length-prefixed
			return nullptr;
		//now pc points to the name's first character
		pc += n;//skip to tag
		if (*pc != SEPA_TAG)
			return nullptr;//no tag?
		return pc;
	}
	const char* tag() const
	{
		return skipToTag(key);
	}
	const char* tag(int& n) const
	{
		n = -1;//no MOD sepa
		const char* pc(skipToTag(key));
		if (pc)
		{
			assert(*pc == SEPA_TAG);
			++pc;//SEPA_TAG
			for (++n; pc[n] && pc[n] != SEPA_MODID; n++);
			return pc;
		}
		return nullptr;
	}
	int mid() const
	{
		const char* pc(tag());
		if (pc && *pc == SEPA_TAG)
		{
			++pc;//SEPA_TAG
			pc = strchr(pc, SEPA_MODID);
			if (pc)
				return atoi(++pc);//skip SEPA_MODID
		}
		return -1;
	}
	/*bool hasName() const {
		if (!mp)
			return false;
		if (mp[0] == SEPA_NAMPFX1)
			if (mp[1] == SEPA_NAMPFX2)
				return false;
		return true;
	}*/
	bool checkName(const char *pc) const//find out if the string differ only by tags
	{
		const char* pc0(key);
		while (*pc0 == *pc)
		{
			pc0++;
			pc++;
		}
		return (*pc0 == SEPA_TAG && *pc == 0);//TAG
	}
};

typedef	NameRef_t*	PNameRef;
typedef	const NameRef_t*	CPNameRef;

MyString FromCompoundName0(const MyStringEx& aName, int iMode);
MyString FromCompoundName(const MyStringEx& aName, int iMode = 0);
MyStringEx ToCompoundName(CPNameRef);

/*struct NameElt_t
{
	Obj_t *pObj;
	NameElt_t(Obj_t *p) : pObj(p){}
};*/


#if(!SBTREE_NAMES)
typedef std::map<PNameRef, NameElt_t, NamesMapCompare>	NamesMap;
#else
//typedef sbtree_multimap<PNameRef, NameElt_t, NamesMapCompare>	NamesMap;

typedef sbtree::config1_s<NameRef_t, NameRef_t::CompObj>	NamesMapConfig;
typedef sbtree::size_balanced_tree<NamesMapConfig>	NamesMap;

#endif
typedef NamesMap::iterator			NamesMapIt;
typedef NamesMap::const_iterator	NamesMapCIt;

class NamesMgr_t 
{
public:
	NamesMgr_t();
	~NamesMgr_t();
	void clear(MemoryMgr_t &);
	void check(const char *) const;
	bool initialize();//TypePtr);

	PNameRef	name() const { return mpName; }
	void		setName(PNameRef p){ mpName = p; }
	void		setNameRef(PNameRef p){
		mpName = p;
#if(SHARED_NAMES)
		if (mpName)
			mpName->addRef();
#endif
	}

	bool isInitialized() const { return mp != 0; }
	const NamesMap &namesMap() const { return *mp; }
	bool insertName(PNameRef);
	NamesMapIt insertIfNotExist(PNameRef);
	PNameRef removen(PNameRef);
	PNameRef removeIt(NamesMapIt);

	NamesMapIt end() const { return mp->end(); }
	PNameRef findn(CNamePtr p) const {
		if (!mp)
			return nullptr;
		NamesMapIt it(mp->find((NamePtr)p));
		if (it == mp->end())
			return nullptr;
		return &(*it);
	}
	ObjPtr	findObj(CNamePtr p) const
	{
		PNameRef pn(findn(p));
		if (!pn)
			return nullptr;
		return pn->obj();
	}
	ObjPtr	findObj(CNamePtr, unsigned maxLen) const;//by prefix
	ObjPtr	findObjEx(CNamePtr) const;//try find by tag as well
	//NamesMapIt findRev(Obj_t *);

	bool contains(PNameRef pn) const
	{
		if (!mp)
			return false;
		NamesMapIt i(mp->find(pn->key));
		if (i == mp->end())
			return false;
		return (*i).theSame(*pn);
	}

	bool empty() const { return mp == 0; }
	size_t bias() const
	{
		//NameRef_t n;
		char ch[] = { SEPA_TAG + 1, '\0' };
		//n.setPvt0(ch);
		NamesMapIt i(mp->lower_bound(ch));
		//n.setPvt0(nullptr);
		if (i == mp->end())
			return size_t(-1);//max
		/*const char* pc = i->first->c_str();
		--i;
		pc = i->first->c_str();*/
		return mp->rank(i);
	}
	size_t size() const { return mp->size(); }
	PNameRef at(size_t i) const {
		return &(*mp->at(i));
	}
	ObjPtr objAt(size_t i) const {
		return mp->at(i)->obj();
	}
	NamesMapIt atIt(size_t i) const {
		return mp->at(i);
	}
	size_t rank(PNameRef pn) const {
		NamesMapIt i(pn);
		if (i == mp->end())//?
			return size_t(-1);
		return mp->rank(i);
	}

protected:
	PNameRef	mpName;
	NamesMap	*mp;
};


///////////////////////////////////////////////////////
#if(0)
typedef std::map<std::string, NamesMgr_t *>	mapNamespace_t;
typedef mapNamespace_t::iterator			mapNamespace_it;

class NamesMgr_t
{
public:
	NamesMgr_t();
	~NamesMgr_t();
	NamesMgr_t * nameSpace( const char * name = nullptr );

protected:
	mapNamespace_t	m;
	NamesMgr_t *	mpMainNS;
};
#endif




////////////////////////////////////////////////////
#if(0)
template <typename K, typename V, typename COMPARE = std::less>
class BiMap : public sbtree_multimap<K, V, NamesMapCompare>//to find a field having a mangled name
{
	std::map<V, K>		mRevMap;//to find a ugly name, having a filed'd ptr (the name is weak)
public:
	BiMap()
	{
	}
	bool add(PNameRef pc, V p)
	{
		if (insert(std::make_pair(pc, p)) == end())
			ASSERT0;
		if (!mRevMap.insert(std::make_pair(p, pc)).second)
			ASSERT0;
		return true;
	}
	/*bool remove(K pn, V p)//later
	{
		iterator i(lower_bound(pn));
		if (i == end())
			return false;
		V p(i->second);
		erase(i);
		if (mRevMap.erase(p) != 1)
			ASSERT0;
		return true;
	}*/
	K remove(V p)
	{
		std::map<V, K>::const_iterator i(mRevMap.find(p));
		if (i == mRevMap.end())
			return nullptr;
		K pn(i->second);
		for (iterator j(lower_bound(pn)); j != end(); j++)
		{
			//assert(j->first == pn);//later!
			if (j->second == p)
			{
				erase(j);
				mRevMap.erase(i);
				return pn;
			}
		}
		return nullptr;
	}
	V field(K pn) const
	{
		for (const_iterator i(lower_bound(pn)); i != end(); i++)
		{
			if (i->first == pn)
				return i->second;
		}
		return nullptr;
	}
	V field(const char *n) const
	{
		NameRef_t a;
		a.setPvt0((char *)n);
		const_iterator i(find(&a));
		a.clearPvt();
		if (i == end())
			return nullptr;
		return i->second;
	}
	K name(V p) const
	{
		std::map<V, K>::const_iterator i(mRevMap.find(p));
		if (i == mRevMap.end())
			return false;
		return i->second;
	}
};
#endif

