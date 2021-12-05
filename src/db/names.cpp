#include "names.h"
#include "prefix.h"
#include <string>

#include "mem.h"
#include "obj.h"
#include "type_struc.h"
#include "type_module.h"

NamesMgr_t::NamesMgr_t()
	: mpName(0),
	mp(0)
{
}

NamesMgr_t::~NamesMgr_t()
{
	assert(!mp);
	//clear();
}

/*void NameRef_t::setPvt(const char *p)
{
	//CHECK(MyString(p).startsWith("_FSCloseModeless"))
	//STOP
	delete[] mp;
#if(1)
	mp = new char[strlen(p) + 1];
	strcpy(mp, p);
#else
	size_t n(strlen(p));
	uint8_t buf[4];
	size_t l(encode_ULEB128(n, buf, sizeof(buf)));
	mp = new char[l + n + 1];//+eos
	strncpy(mp, (const char*)buf, l);
	strcpy(mp + l, p);
#endif
}*/

MyString FromCompoundName0(const MyStringEx& aName, int iMode)//iMode: -1:no name; 0:all; 1:no tag
{
	MyString s;
	s += SEPA_NAMPFX1;
	if (iMode >= 0)
	{
		if (!aName[0].isEmpty())
			s += NumberToString(aName[0].length());//maybe a hex?
	}
	s += SEPA_NAMPFX2;
	if (iMode >= 0)
		s += aName[0];
	if (iMode <= 0 && aName.size() > 1)//! no TAG
	{
		s += SEPA_TAG;
		s += aName[1];//TAG
		if (aName.size() > 2)
		{
			s += SEPA_MODID;
			s += aName[2];// modUq;
		}
	}
	if (s == SEPA_NAMPFX0)
		return MyString();
	return s;
}

MyString FromCompoundName(const MyStringEx& aName, int iMode)//iMode: -1:no name; 0:all; 1:no tag
{
	if (aName.size() < 2)
		return aName;
	return FromCompoundName0(aName, iMode);
}

MyStringEx ToCompoundName(CPNameRef pn)
{
	const char* pc(nullptr);
	int n(-1);
	if (pn)
		pc = pn->name(n);

	MyStringEx sx;
	if (pc)
	{
		if (n >= 0)
		{
			sx.push(MyString(pc, n));//NAME
			pc += n;//skip to TAG
		}
		else
		{
			sx.push(pc);//NAME only
			pc = 0;
		}
	}
	else if (n >= 0)
	{
		const char* pc1(pn->c_str());
		assert(pc1 && pc1[0] == SEPA_NAMPFX1 && pc1[1] == SEPA_NAMPFX2);
		sx.push("");//no NAME
		pc = pc1 + 2;//skip '[]'
	}
	if (pc)//TAG id
	{
		assert(*pc == SEPA_TAG);
		++pc;
		const char* pc2(strchr(pc, SEPA_MODID));//MODULE id
		if (pc2)
		{
			sx.push(MyString(pc, pc2 - pc));//TAG
			pc2++;//skip SEPA_MODID
			/*int un(atoi(pc2));//module unique
			FolderPtr pFolder(FindModuleFolderByUnique(un));
			TypePtr pModule(ModuleOf(pFolder));
			assert(pModule);
			sx.push(ModuleTitle(pModule));*///MODULE id
			sx.push(pc2);
		}
		else
			sx.push(pc);//TAG (no module?)
	}
	return sx;
}

void NamesMgr_t::clear(MemoryMgr_t &rMemMgr)
{
	if (mp)
	{
		//only stock names must have left
		for (NamesMapIt i(mp->begin()); i != mp->end(); i++)
		{
			CNamePtr pName(i->c_str());
CHECK(MyString(pName) == "BOOL")
STOP
#ifdef _DEBUG
			ObjPtr pObj(i->obj());
#endif
			//TypePtr pType(pObj ? pObj->objTypeGlob() : nullptr);
#if(SHARED_NAMES)
			if (pName->refsNum() == 0)
#endif
				delete pName;
				//rMemMgr.Delete(pName);
		}
		delete mp;
		mp = 0;
	}
}

void NamesMgr_t::check(const char *pc) const
{
	if (mp)
	{
		for (NamesMapIt i(mp->begin()); i != mp->end(); i++)
		{
			CNamePtr pn(i->c_str());
			ObjPtr pObj(i->obj());
			if (!pc || strncmp(pn, pc, strlen(pc)) == 0)
				fprintf(stdout, " --> %s\n", pn);
		}
	}
}

bool NamesMgr_t::insertName(PNameRef pName)
{
//CHECK(strcmp(pName->c_str(), "ifstream") == 0)
//STOP
	initialize();//iOwner);
#if(SBTREE_NAMES)
	std::pair<NamesMapIt, bool> itp = mp->insert_unique(pName);

#else
	pair<NamesMapIt, bool> itp = mp->insert(make_pair(pName, NameElt_t(pObj)));
#endif
	return itp.second;
}

NamesMapIt NamesMgr_t::insertIfNotExist(PNameRef pn)
{
//CHECKID(pObj, 0x2225f)
//STOP
#if(SBTREE_NAMES)
		std::pair<NamesMapIt, bool> itp(mp->insert_unique(pn));
#else
		std::pair<NamesMapIt, bool> itp(mp->insert(std::make_pair(pn, NameElt_t(pObj))));
#endif
	if (itp.second)
		return itp.first;
	return end();
}

PNameRef NamesMgr_t::removen(PNameRef iName)
{
CHECK(MyString(iName->c_str()) == "afxChNil")
STOP
	//WARNING: iName may reside in different module
	NamesMapIt i(iName);// findn(iName));
//?	assert(findn(iName->c_str()) == iName);

//CHECKID(i->second.pObj, -49)
//STOP
	//NamePtr pName(i->key_());
	//	mp->erase(i);
#if(SHARED_NAMES)
	if (iName2->releaseRef() > 0)
	{
		i->second.pObj = nullptr;//some refs remain, but which obj does it ref to?
		return nullptr;
	}
#endif
//CHECKID(i->second.pObj, 0xa23c)
//STOP
	mp->take(i);
	if (mp->empty())
	{
		delete mp;
		mp = nullptr;
	}
	return iName;
}

PNameRef NamesMgr_t::removeIt(NamesMapIt i)
{
	//assert(!i->c_str());
//CHECKID(i->second.pObj, 0xa23c)
//STOP
	PNameRef pn(mp->take(i));
	if (mp->empty())
	{
		delete mp;
		mp = nullptr;
	}
	return pn;
}

bool NamesMgr_t::initialize()//TypePtr p)
{
//CHECKID(p, 0x337)
//STOP
	if (mp)
		return false;
	mp = new NamesMap;
	return true;
}

/*ObjPtr NamesMgr_t::findObj(PNameRef pn) const
{
	NamesMapIt i(findn(pn));
	if (i == mp->end())
		return nullptr;
	return i->obj();
}*/

/*NamesMapIt NamesMgr_t::findRev(Obj_t * pObj)
{
	assert(mp);
	NamesMapIt i;
	for (i = mp->begin(); i != mp->end(); i++)
		if (i->second.pObj == pObj)
			break;
	return i;
}*/

/*ObjPtr NamesMgr_t::findObj(CNamePtr pc0) const
{
	if (!mp)
		return nullptr;
	NamePtr pc((NamePtr)pc0);
	NamesMapCIt it(mp->find(pc));
	if (it == mp->end())
		return nullptr;
	return it->obj();
}*/

ObjPtr NamesMgr_t::findObj(CNamePtr pc0, unsigned n) const
{
	if (!mp)
		return nullptr;
	MyString s(pc0, n);
	NamePtr pc((NamePtr)s.c_str());
	//NameRef_t tmp;
	//tmp.setPvt0((char *)s.c_str());
	NamesMapIt it(mp->upper_bound(pc));
	//tmp.clearPvt();
	if (it != mp->end())
	{
		CNamePtr pn(it->c_str());
		if (strncmp(pn, s.c_str(), n) == 0)
			return it->obj();
	}
	if (it != mp->begin())
	{
		--it;
		CNamePtr pn(it->c_str());
		if (strncmp(pn, s.c_str(), n) == 0)
			return it->obj();
	}
	/*static int z(0);
	if (z)
		check(s.c_str());*/
	return nullptr;	
}

ObjPtr NamesMgr_t::findObjEx(const char* pc0) const
{
	ObjPtr pObj(findObj(pc0));
	if (!pObj)
	{
		//if haven't found by compound name - this means the symbol is expoted by ordinal (OR ordinal mismatch)

		//extract tag position
		const char* pc(pc0);
		pc = NameRef_t::skipToTag(pc);
		if (!pc)
			return nullptr;//no TAG - the name have been already tried

		pObj = findObj(pc0, unsigned(pc - pc0));
		if (!pObj)
		{
			//now try a tag only as a last resort
			MyString s(SEPA_NAMPFX0);
			s.append(pc);
			pObj = findObj(s);
		}
	}
	return pObj;
}


/*const char * NamesMgr_t::name0()
{
	if ( mpName )
		return mpName;
	return nullptr;
}

void NamesMgr_t::set Name(const char * pName)
{
	mpName = pName;
}*/

////////////////////////////////////////////////////////

/*#if(0)
NamesMgr_t::NamesMgr_t()
{ 
	mpMainNS = nullptr; 
}

NamesMgr_t::~NamesMgr_t()
{
	while ( !m.empty() )
	{
		mapNamespace_it it = m.begin();
		NamesMgr_t * p = it->second;
		p->release();
		m.erase(it);
	}
}

NamesMgr_t * NamesMgr_t::nameSpace( const char * name )
{
	if ( !name )
	{
		if ( !mpMainNS )
		{
			mpMainNS = new Name space_t( -1, nullptr );
			pair<mapNamespace_it,bool> it 
				= m.insert( pair<string, NamesMgr_t *>("@GLOBAL", mpMainNS) );
			if ( !it.second )
			{
				delete mpMainNS;
				mpMainNS = nullptr;
			}
		}
		return mpMainNS;
	}

	mapNamespace_it it = m.find( name );
	if ( it != m.end() )
		return it->second;


	NamesMgr_t * pNS = new Name space_t( nameSpace()->add(name), nameSpace() );

	pair<mapNamespace_it,bool> itp 
		= m.insert( pair<string, NamesMgr_t *>(name, pNS) );
	if ( !itp.second )
	{
		delete pNS;
		pNS = nullptr;
	}
	return pNS;
}
#endif*/

bool ValidName(const char *pName)
{
	const char *p = pName;
	while (*p && isspace(*p)) p++;//trim left
	if (!(*p))
		return false;

	if (!iscsymf(*p))//first must not be digit!
		return false;

	while (*p && !isspace(*p)) 
	{
		if (!iscsym(*p))
			return false;
		p++;
	}

	while (*p && isspace(*p)) p++;//trim right

	if (*p)
		return false;//bad name

	return true;
}


