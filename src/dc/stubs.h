#pragma once

#include <map>
#include <list>
#include <string>
#include <fstream>
#include <algorithm>
#include "qx/MyPath.h"
#include "qx/MyStringList.h"
#include "shared/defs.h"
#include "shared/misc.h"
#include "shared/sbtree.h"
#include "db/mem.h"
#include "info_dc.h"

//class Dc_t;
class Main_t;

enum REFMODE_t
{
	REFMODE_DIRECT,
	REFMODE_GLOBAL_PTR,
	REFMODE_UNK_PTR,
	REFMODE__MASK = 0x3,
	STUB_RECOVER_FIELD = 0x10,//for save/restore only
	STUB_MODIFIED = 0x20,
	REFMODE__BAD = (unsigned)-1
};

class ValueVec : public std::vector<MyString>
{
public:
	ValueVec(){}
	ValueVec(MyString s)
	{
		MyStringList l(MyStringList::split(" ", s));
		for (MyStringList::iterator i(l.begin()); i != l.end(); i++)
		{
			MyString s(*i);
			while (!s.empty() && s.at(0) == '0')
				s.erase(s.begin());
			push_back(s);
		}
		while (!empty() && back().empty())
			pop_back();
	}
	void setAt(size_t index, MyString s)
	{
		if (!(index < size()))
		{
			if (s.empty())
				return;
			resize(index + 1);
		}
		at(index) = s;
	}
	MyString toString() const
	{
		MyString s;
		for (size_t i(0); i < size(); i++)
		{
			if (i > 0)
				s.append(" ");
			if (at(i).empty())
				s.append("0");
			else
				s.append(at(i));
		}
		return s;
	}
};

///////////////////////////////////// StubBase_t
struct StubBase0_t
{
protected:
	ADDR mAtAddr;
	REFMODE_t mRefMode;
public:

	StubBase0_t()
		: mAtAddr(0),
		mRefMode(REFMODE_UNK_PTR)
	{
	}
	StubBase0_t(ADDR atAddr, REFMODE_t refMode)
		: mAtAddr(atAddr),
		mRefMode(refMode)
	{
	}
	MyString atAsString(ADDR64 ib) const
	{
		MyString s;
		REFMODE_t refm(REFMODE_t(mRefMode & REFMODE__MASK));
		if (refm == REFMODE_GLOBAL_PTR)
			s.append("[");
		else if (refm == REFMODE_UNK_PTR)
			s.append("{");
		s.append(Int2Str(ib + mAtAddr, I2S_HEX));
		if (refm == REFMODE_GLOBAL_PTR)
			s.append("]");
		else if (refm == REFMODE_UNK_PTR)
			s.append("}");
		return s;
	}

	ADDR atAddr() const { return mAtAddr; }
	REFMODE_t refMode() const { return REFMODE_t(mRefMode & REFMODE__MASK); }
	void setRefMode(REFMODE_t refm)
	{
		mRefMode = REFMODE_t(mRefMode & ~REFMODE__MASK);
		mRefMode = REFMODE_t(mRefMode | (refm & REFMODE__MASK));
	}
	static REFMODE_t toRefMode(CFieldPtr pField)
	{
		REFMODE_t refMode(REFMODE_DIRECT);
		if (pField && DcInfo_t::IsGlobal(pField))
		{
			if (!pField->type() || !pField->type()->typeProc())
				refMode = REFMODE_GLOBAL_PTR;
		}
		else
			refMode = REFMODE_UNK_PTR;
		return refMode;
	}
};

struct StubBase_t : public StubBase0_t
{
protected:
	CFieldPtr mpField;
public:

	StubBase_t()
		: mpField(nullptr)
	{
	}
	StubBase_t(CFieldPtr p, ADDR atAddr)
		: StubBase0_t(atAddr, p ? toRefMode(p) : REFMODE_UNK_PTR),
		mpField(p)
	{
		if (mRefMode < REFMODE_UNK_PTR)
			mAtAddr = p->_key();
	}
	StubBase_t(CFieldPtr p, REFMODE_t refMode, ADDR atAddr)
		: StubBase0_t(atAddr, refMode),
		mpField(p)
	{
	}
	FieldPtr field() const { return (FieldPtr)mpField; }
};

/////////////////////////////////////// Stub_t
class Stub_t : public StubBase0_t
{
	MyString	mValue;
public:
	friend class StubMgr_t;

	Stub_t()
	{
	}
	Stub_t(ADDR atAddr, REFMODE_t refMode)//, FieldPtr pField)
		: StubBase0_t(atAddr, refMode)
	{
	}
	Stub_t(ADDR atAddr, REFMODE_t refMode, MyString value)
		: StubBase0_t(atAddr, refMode),
		mValue(value)
	{
	}

	void setValue2(MyString s)//, bool bClearField= 1)
	{
		//FieldPtr p(mpField);
		//if (bClearField)
			//mpField	= nullptr;
		mValue = s;
		//return p;
	}
	/*void setField2(FieldPtr p)//, bool bClearValue =1)
	{
//CHECK(p && (p->ID() == 0x9a9))
//STOP
		mpField = p;
		//if (bClearValue)
			//mValue.clear();
	}*/

	const MyString &value() const { return mValue; }
	ValueVec valueVec() const {
		return ValueVec(mValue);
	}

	//bool isMismatched() const { return field() && !mValue.empty(); }
	void setModified(bool b){
		if (b)
			mRefMode = REFMODE_t(mRefMode | STUB_MODIFIED);
		else
			mRefMode = REFMODE_t(mRefMode & ~STUB_MODIFIED);
	}
	bool isModified() const { return (mRefMode & STUB_MODIFIED) != 0; }
};

#define SBTREE_STUBS	1

#if(!SBTREE_STUBS)
typedef std::map<ADDR, Stub_t>	StubsMap;
#else
typedef sbtree_multimap<ADDR, Stub_t>	StubsMap;
#endif
typedef StubsMap::iterator	StubIt;
typedef StubsMap::const_iterator	StubCIt;

class StubMgr_t : private StubsMap
{
	typedef StubsMap	B;
	bool	mbDirty;
public:
	StubMgr_t() : mbDirty(false){}
	~StubMgr_t() {}
	void setDirty(bool b){ mbDirty = b; }
	//ADDR64	mImageBase;
	StubCIt begin() const { return B::begin(); }
	StubCIt end() const { return B::end(); }
	void clear() { B::clear(); }
	size_t size() const { return B::size(); }
	int rank(ADDR a) const
	{
		StubCIt it(B::find(a));
		if (it != B::end())
			return (int)B::rank(it);
		return -1;
	}
	const Stub_t* at(size_t i) const
	{
		StubCIt it(B::at(i));
		return (const Stub_t *)&(it->second);
	}
	Stub_t& insert(ADDR atAddr, REFMODE_t refMode, const MyString &value)
	{
		assert((refMode & REFMODE__MASK) <= REFMODE__MASK);
		StubIt i(B::find(atAddr));
		if (i == B::end())
		{
			assert(atAddr != 0);
			StubIt it(B::insert(std::make_pair(atAddr, Stub_t(atAddr, refMode, value))));
			assert(it != B::end());
			return it->second;


		}
		Stub_t& a(i->second);
		assert(a.atAddr() == atAddr && (a.refMode() & REFMODE__MASK) == refMode);
		a.setValue2(value);
		a.setModified(true);
		setDirty(true);
		return a;
	}
	Stub_t* findStub(ADDR addr) const
	{
		StubCIt i(B::find(addr));
		if (i != B::end())
			return (Stub_t*)&i->second;
		return nullptr;
	}
	Stub_t& insertStubFromField(ADDR atAddr, FieldPtr pField)
	{
		StubIt i(B::find(atAddr));
		if (i == B::end())
		{
			assert(atAddr != 0);
			StubIt it(B::insert(std::make_pair(atAddr, Stub_t(pField->_key(), REFMODE_GLOBAL_PTR))));//, pField
			assert(it != B::end());
			return it->second;
		}
		Stub_t& a(i->second);
		assert(a.atAddr() == atAddr && a.refMode() == REFMODE_GLOBAL_PTR);
		a.setModified(true);
		return a;
	}
	bool disconnectStub(ADDR atAddr)
	{
		StubIt i(B::find(atAddr));
		if (i != B::end())
		{
			Stub_t& aStub(i->second);
			/*if (aStub.field())
			{
				GlobPtr ifDef(Func DefAttached(aStub.field()));
				if (ifDef)
				{
					aStub.setValue2(FuncDefToStubStr(ifDef));
					aStub.setModified(true);
					return true;
				}
			}*/
		}
		return false;
	}
	const Stub_t* touchStub(ADDR atAddr, const MyString& sValue) const
	{
		Stub_t* pStub(findStub(atAddr));
		if (pStub)
		{
			pStub->setModified(true);
			if (pStub->value() != sValue)
				pStub->setValue2(sValue);
		}
		return pStub;
	}
	bool update(ADDR atAddr, REFMODE_t refMode, const MyString& sValue)
	{
		StubIt stubIt(B::find(atAddr));
		if (stubIt != B::end())
		{
			stubIt->second.setValue2(sValue);
			stubIt->second.setRefMode(refMode);
			return true;
		}

/*#if(1)
	StubIt it(std::lower_bound(begin(), end(), addr, compi_t()));
	if (it != end())
		if ((*it).mAtAddr != addr)
			return end();
	return it;
#else
	if (!empty())
	{
		assert(addr);
		for (StubIt it(begin()); it != end(); it++)
		{
			if (addr == (*it).mAtAddr)
				return it;
		}
	}
	return end();
#endif*/
		return false;
	}

};


class StubFault_t : public std::exception
{
	FieldPtr mpField;
	bool mbCheck;
public:
	StubFault_t(FieldPtr p, bool bCheck = false) : mpField(p), mbCheck(bCheck){}
	//Stub_t &stub() const { return mrStub; }
	//bool isCheck() const { return mbCheck; }
	FieldPtr field() const {
		return mpField;
	}
protected:
	virtual const char * what() const throw (){
		return "No Stub Fault";
	}
};

class StubParser_t
{
	MyPath		&mrPath;
	std::ifstream m_fs;
	//int mPhase;
	MyString	msModule;
	MyString	mKey;
	MyString	mValue;
	//REFMODE_t	mRefMode;
	//ADDR		mAtAddr;

public:
	StubParser_t(MyPath &);
	~StubParser_t();

	operator bool() const { return !mKey.isEmpty(); }
	StubParser_t& operator ++();
	//StubParser_t& operator ++(int){ return operator++(); }

	const char *module() const { return msModule.c_str(); }
	const char *key() const { return mKey.c_str(); }
	const char *value() const { return mValue.c_str(); }

	//void run();
	static REFMODE_t parseAtAddr(const char *p1, ADDR &atAddr, ADDR64 imageBase);
private:
	int handleLine(char *buf);
	bool parseStubOut(char *p1);
	bool isValid() const { return m_fs.is_open() && !m_fs.eof(); }
	
};


#include "db/files.h"

class FileStubs_t : public File_t
{
	StubMgr_t	mStubsMgr;
public:
	FileStubs_t(){}
	virtual ~FileStubs_t()
	{
	}

	//void setStubsPtr(StubMgr_t *p){ mpStubMgr = p; }

	StubMgr_t &stubs(){ return mStubsMgr; }
	const StubMgr_t &stubs() const { return mStubsMgr; }

protected:
	virtual FILEID_t fileId() const { return FILEID_STUBS; }
	virtual FileStubs_t *fileStubs() const { return const_cast<FileStubs_t *>(this); }
};

class node_t;
struct StubComparator {
	bool operator()(const node_t *a, const node_t *b) const;
	//bool operator()(const node_t *a, const MyString &) const;
};

class StubSet : public std::set<node_t *, StubComparator>
{
	std::map<unsigned, CxxSymbMapCIt> mOrdMap;//mangled=>node
	std::map<unsigned, MyString> mCSymb;//non-C++ symbols
public:
	StubSet(){}
	~StubSet();
	void dump(std::ostream &os, int &index) const;
	bool add(node_t* pn) {
		return insert(pn).second;
	}
	bool addOrdinal(unsigned ord, CxxSymbMapCIt it) {
		return mOrdMap.insert(std::make_pair(ord, it)).second;
	}
	bool addOrdinal(unsigned ord, MyString s) {
		return mCSymb.insert(std::make_pair(ord, s)).second;
	}
	/*bool addMangled(const MyString &mangled, unsigned ord, node_t* pn){
		std::pair<iterator, bool> ret(insert(pn));
		if (ret.second)
			if (mMangledMap.insert(std::make_pair(mangled, ret.first)).second)
				return true;
		erase(ret.first);
		return false;
	}*/
	node_t *findProto(const MyStringEx&) const;
	node_t *findProtoEx(MyStringEx&) const;
};

class StubPool_t : public StubSet//global
{
	typedef std::map<MyString, StubSet>	PerModuleStubs;
	PerModuleStubs	mPerModule;
	MyString mName;
public:
	StubPool_t(){}
	void setName(MyString name) { mName = name; }
	const MyString& name() const { return mName; }
	bool hasModule(MyString module) const;
	StubSet& assureModule(MyString module);
	StubSet& findModule(MyString module);
	node_t *findProto(MyString symbol, MyString module) const;
	node_t *findProtoEx(MyString symbol, MyString module) const;//first in module
	void dump(std::ostream &) const;
};

class StubInfo_t : public DcInfo_t
{
public:
	StubInfo_t(const DcInfo_t &DI)
		: DcInfo_t(DI)
	{
	}

	bool LoadStubs(MyString path = MyString());
	bool SaveStubs(MyString path = MyString()) const;
	bool SaveOne(const Stub_t &) const;

	//int FromStub(const Stub_t *, ProbeEx_t &);
	bool ProcessStub(const char* key, const char* value, int phase);// , const ImpLookupMap&);
	bool AddStub(ADDR atAddr, REFMODE_t refMode, const char *value);
	MyString FuncDefToStubStr(GlobPtr) const;
	bool	DisconnectStub(ADDR);

	MyString ValueFromStub(const Stub_t &) const;
	MyString name(const Stub_t &) const;
	//StubIt	FindStubByNameIt(const char *stub_name) const;
	//StubIt	FindStubIt(ADDR) const;
	Stub_t*	FindStub(ADDR) const;
	const Stub_t* MakeStub(ADDR, REFMODE_t refMode, const MyString& sValue);
	const Stub_t* TouchStub(ADDR, const MyString&);
	FieldPtr FindFieldRef(ADDR atAddr, REFMODE_t refMode, CTypePtr pScope) const;
	Stub_t&	InsertStub(ADDR atAddr, REFMODE_t, const MyString &value);
	Stub_t &InsertStub(ADDR atAddr, FieldPtr pField, REFMODE_t refMode, const MyString &value);
	static Stub_t &InsertStub0(StubMgr_t &, ADDR atAddr, REFMODE_t refMode, const MyString &value);
	Stub_t&	InsertStubFromField(ADDR);
	Stub_t&	InsertStubFromField(ADDR, FieldPtr);
	ADDR nameToField(const char *, MyString &module, const ImpLookupMap &lImp) const;
	//const char *fieldToName(ADDR, OFF_t *) const;
	//bool DumpStubImported(const Stub_t &, std::ostream &, std::string &, const ImpLookupMap &) const;
	bool dumpz(const Stub_t &, std::ostream &/*, MyString &*/) const;
	//void BuildImpLookupMap(ImpLookupMap &) const;
	bool CreateFuncProfile(const Stub_t *, FuncProfile_t &si);
	const Stub_t *FindStubOrThrow(CFieldPtr, ADDR) const;
	void StubFault(CFieldPtr) const;

	//static void dump(const FuncProfile_t &, std::ostream &os, char sep = ' ');
	void dump_trimmed(const FuncProfile_t &, std::ostream &os, char sep = ' ') const;
	MyString ToString(const GPRs_t &, const GPRs_t &) const;//retvals+spoiled
	MyString ToString1(const GPRs_t &) const;//args
};




