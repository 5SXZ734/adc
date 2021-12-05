#pragma once

#include <list>
#include <set>
#include "qx/MyString.h"
#include "interface/IADCGui.h"

#include "field.h"
#include "data.h"

struct Script_t;
class MyStreamBase;

class Frame_t : public frame_t
{
	unsigned	_extra;//for arrays offsetting
	FieldPtr pField;
public:
	Frame_t(TypePtr p, ADDR a, FieldPtr f) : frame_t(p, a), _extra(0), pField(f) {}
	Frame_t(const Block_t&, TypePtr, ADDR, FieldPtr);
	Frame_t(const Block_t&, TypePtr, ADDR, unsigned, FieldPtr);
	Frame_t(const Frame_t& o)
		: frame_t(o)
	{
		_extra = o._extra;
		pField = o.pField;
	}

	FieldPtr field() const { return pField; }
	void setField(CFieldPtr p, bool bAddr = false) {
		pField = (FieldPtr)p;
		if (pField && bAddr)
			setAddr(pField->_key());
	}
	ADDR setField2(CFieldPtr);//sets a field and _extra
	Struc_t* struc() const;
	OFF_t rawoff() const;

	ADDR addrx() const { return m_addr + _extra; }
	unsigned offsx() const { return offs() + _extra; }
	bool operator==(const Frame_t& o) const {
		return frame_t::operator==(o) && _extra == o._extra && pField == o.pField;
	}
	bool operator!=(const Frame_t& o) const { return !operator==(o); }
	void clearExtra() { _extra = 0; }
	void setExtra(unsigned u) { _extra = u; }
	unsigned extra() const { return _extra; }
};

class Locus_t : public std::list<Frame_t>
{
	typedef std::list<Frame_t> Base_t;

public:
	Locus_t()//Folder_t *pFolder = nullptr)
		//: mpFolder(pFolder),
		//mpTypesMap(nullptr)
	{
	}
	Locus_t(const Locus_t& l)
		: Base_t(l)
	{
	}
	void assign0(const Locus_t& a)
	{
		clear();
		for (const_iterator i(a.begin()); i != a.end(); i++)
			push_back(*i);
	}
	void assign(const Locus_t& a)
	{
		assign0(a);
		//setFolder(a.folder());
	}

	TypePtr module() const;//front cont is module
	TypePtr module2() const;//front cont is some seg
	TypePtr struc() const;
	TypePtr proc() const;
	TypePtr seg() const;
	TypePtr frontSeg() const;
	TypePtr frontSeg2() const;
	ADDR addr() const { return empty() ? -1 : back().addrx(); }
	unsigned offs() const { return empty() ? 0 : back().offs(); }
	unsigned offsx() const { return empty() ? 0 : back().offsx(); }
	FieldPtr field0() const { return empty() ? 0 : back().field(); }//warning: there is override in derived class!
	bool setAddr(ADDR va) {
		if (empty())
			return false;
		back().setAddr(va);
		return true;
	}
	bool setField(CFieldPtr p, bool bAddr = false) {
		if (empty())
			return false;
		back().setField(p, bAddr);
		return true;
	}
	FieldPtr asProc() const;//exact pick
	TypePtr funcOwner() const;
	ROWID da() const;
	ADDR va() const;
	OFF_t ra() const;
	MyString toString() const;
	MyString toStringSeg(const Project_t&) const;
	bool operator==(const Locus_t& o) const {
		const Base_t& a(*this);
		if (a != o)
			return false;
		return true;
	}
	bool operator!=(const Locus_t& o) const { return !operator==(o); }
	void clear0() { clear(); }
	ADDR range(bool bRalaxed) const;
	bool reduce();

	Frame_t* upframe(unsigned) const;//go up to the top
	bool stripToField(FieldPtr);
	bool stripToNamedField();
	bool stripToSeg();
	bool stripToProc();
	FieldPtr stripToSeg(unsigned& offs);
	bool stripToUserField(const char* resolveName);//arg is for resolving cloned fields
	void wrapUp();
	void add(CTypeBasePtr, ADDR, FieldPtr);
	void add0(CTypeBasePtr, ADDR, FieldPtr);
};

/*struct cmpByFieldOffset {
	bool operator()(CFieldPtr a, CFieldPtr b) const {
		return a->offset() < b->offset();
	}
};
*/

class I_Context : public My::IUnk
{
	//int m_hint;
public:
	I_Context()
		//: m_hint(0)
	{
	}
	virtual ~I_Context()
	{
	}
	virtual void clear_all() = 0;
	void operator=(const I_Context&)
	{
		assert(0);
	}
	//int hint() const { return m_hint; }
	//void setHint(int n) { m_hint = n; }
};


struct Locality_t
{
	union
	{
		unsigned short u;
		struct
		{
			unsigned short locality : 4;
			unsigned short scoping : 8;
			//unsigned short decl_ : 4;
			unsigned short obj : 4;
		};
	};
	Locality_t(unsigned short _u) { u = _u; }
	Locality_t() { u = 0; }
	explicit Locality_t(unsigned short loc, unsigned short scp) {
		u = 0;
		locality = loc;
		scoping = scp;
	}
	void setScoping(adcui::LocusIdEnum e) {
		scoping = e;// (u >> adcui::CXTID_SCOPING_OFF);
	}
	bool checkScoping(adcui::LocusIdEnum e) const {
		//return ((u & adcui::CXTID_SCOPING_MASK) >> adcui::CXTID_SCOPING_OFF) == scoping;
		return e == scoping;
	}
};

class Range_t
{
	long	lBeg;	//start pos
	long	lEnd;	//end pos
public:
	Range_t()
		: lBeg(-1),
		lEnd(-1)
	{
	}
	long begin() const { return lBeg; }
	long end() const { return lEnd; }
	size_t size() const {
		if (empty())
			return 0;
		return lBeg - lEnd;
	}
	void setBegin(long n) { lBeg = n; }
	void setEnd(long n) { lEnd = n; }

	bool empty() const {
		return !(lBeg < lEnd);
	}
	void clear() {
		lBeg = lEnd = -1;
	}
	void set(long p1, long p2)
	{
		lBeg = p1;
		lEnd = p2;
	}
	bool inside(long x) const {
		if (!empty())
			return false;
		return (lBeg <= x && x < lEnd);
	}
	bool inside2(long x) const {//inclusive
		if (empty())
			return false;
		return (lBeg <= x && x <= lEnd);
	}
};

class Probe_t : public I_Context
{
protected:
	Locus_t		mLocus;
	ObjPtr		mpObj;			//a precise object picked (e.g to rename)
	FieldPtr	mpFieldDecl;	//a field picked on a current line (there may be few at the same address at struc's lower boundary or in union)
	Locality_t	mLocality;		//see adcui::ContextIdEnum
	value_t		mValue;
	adcui::Color_t mEntityId;
	int			mx;		//cursor's horizontal position
	DA_t		mDA;	//position in document (vert)
	Range_t		mRange;
	TypePtr		mpModule;
public:
	Probe_t()
		: mpObj(nullptr),
		mpFieldDecl(nullptr),
		mEntityId(adcui::COLOR_NULL),
		mx(-1),
		mpModule(nullptr)
	{
	}
	Probe_t(const Locus_t& l)
		: mLocus(l),
		mpObj(nullptr),
		mpFieldDecl(nullptr),
		mEntityId(adcui::COLOR_NULL),
		mx(-1),
		mpModule(nullptr)
	{
	}
	Probe_t(const Probe_t& o)
		: mLocus(o.locus()),
		mpObj(o.mpObj),
		mpFieldDecl(o.mpFieldDecl),
		mLocality(o.mLocality),
		mValue(o.mValue),
		mEntityId(o.mEntityId),
		mx(o.mx),
		mDA(o.mDA),
		mRange(o.mRange),
		mpModule(o.mpModule)
	{
	}

	Locus_t& locus() { return mLocus; }
	const Locus_t& locus() const { return mLocus; }

	TypePtr module() const { return mpModule ? mpModule : mLocus.module2(); }
	void setModule(TypePtr p) { mpModule = p; }
	
	const DA_t& da() const { return mDA; }

	virtual void clear_all() {
		clear();
	}

	TypePtr moduleFromLocus() const;

	ObjPtr obj() const { return mpObj; }
	virtual void pickObj(CObjPtr p) {
		mpObj = (ObjPtr)p;
#if(0)
		fprintf(stdout, "setObj(%X)\n", p);
		fflush(stdout);
#endif
	}
	Locality_t locality() const { return mLocality; }
	void setLocality(Locality_t u) {
#if(0)
		fprintf(stdout, "setLocality(%X)\n", u.u);
		fflush(stdout);
#endif
		mLocality = u;
	}
	void operator=(const Probe_t& o)
	{
		I_Context::operator=(o);
		mLocus = o.locus();
		mpObj = o.mpObj;
		mpFieldDecl = o.mpFieldDecl;
		mLocality = o.mLocality;
		mValue = o.mValue;
		mEntityId = o.mEntityId;
		mDA = o.mDA;
		mx = o.mx;
		mRange = o.mRange;
	}
	void clear() {
		mLocus.clear0();
		mpObj = nullptr;
		mpFieldDecl = nullptr;
		mLocality = 0;
		mValue.clear();
		mEntityId = adcui::COLOR_NULL;
		mDA.clear();
		mx = -1;
		mRange.clear();
	}
	FieldPtr pickedFieldDecl() const { return mpFieldDecl; }
	void resetFieldDecl() { mpFieldDecl = nullptr; }
	void update();
	const value_t& value() const { return mValue; }
	ROWID daFromFieldDecl() const;
	adcui::Color_t entityId() const { return mEntityId; }
	FieldPtr field() const { return mpObj ? mpObj->objField() : nullptr; }
	TypePtr typeObj() const { return mpObj ? mpObj->objTypeGlob() : nullptr; };

	int x() const { return mx; }

	const Range_t& rangeObj() const { return mRange; }
};


class ProbeIn_t : public Probe_t
{
public:
	ProbeIn_t(int x)
	{
		pickX(x);
	}
	void pickDA(const DA_t& _da) {
		mDA = _da;
	}
	void pickFieldDecl(CFieldPtr p) {
		mpFieldDecl = (FieldPtr)p;
	}
	void pickValue(const value_t& v) {
		mValue = v;
	}
	void pickRange(int x, unsigned len){
		mRange.set(x, x + len);
	}
	void pickEntityId(adcui::Color_t e){
		mEntityId = e;
	}
	void pickX(int x) {
		mx = x;
	}
};


