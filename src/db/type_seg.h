#pragma once

#include <map>

#include "type_struc.h"
#include "types.h"
#include "files.h"
#include "data.h"

class I_Front;

//typedef std::multimap<ADDR, TypePtr>	SubSegMap;//segs
typedef std::list<TypePtr>	SubSegMap;//segs
typedef SubSegMap::iterator  SubSegMapIt;
typedef SubSegMap::const_iterator  SubSegMapCIt;

//#define IKEY(a)		(a->first)
//#define	IVALUE(a)	(a->second)
#define	IVALUE(a)	(*a)

class FRONT_t : public My::IUnk
{
	int	m_id;
	MyString m_name;
	I_Front *mpIFront;
public:
	FRONT_t(const char *name, int id);
	~FRONT_t();
	int id() const { return m_id; }
	const MyString &name() const { return m_name; }
	I_Front *device(DataPtr);
	I_Front *device(const I_DataSourceBase *);
	void releaseDevice();
};

//conflicting fields
//typedef std::multimap<ADDR, FieldPtr>		ConflictField Map;
//typedef FieldMap	ConflictFieldMap;
//typedef ConflictFieldMap::iterator			ClonedFieldMapIt;
//typedef ConflictFieldMap::const_iterator	ClonedFieldMapCIt;


#define	PHANTOMSEG_FLAG	0x80000000
#define RANGESET_FLAG	0x40000000

#define	USE_SEG_RANGES	1

//class TypeObj_t;
typedef size_t	HRangeSet;

/*struct RangeElt_t
{
	HRangeSet iOwner;
	ADDR base;
	TypePtr	iSeg;
	RangeElt_t(HRangeSet p, ADDR a, TypePtr s = nullptr) : iOwner(p), base(a), iSeg(s){}
	void setSeg(TypePtr p){
		assert(!iSeg);
		iSeg = p;
	}
	TypePtr seg() const { return iSeg; }
};


struct RangeEltCompare {
	bool operator()(const RangeElt_t &a, const RangeElt_t &b) const { return a.base < b.base; }
};

class TypeObj_t : public std::set<RangeElt_t, RangeEltCompare>
{
public:
	uint64_t		imageBase;
public:
	TypeObj_t(uint64_t b)
		: imageBase(b)
	{
	}
};*/


typedef FieldMap::iterator		RangeIt;
typedef FieldMap::const_iterator	RangeCIt;

typedef std::set<TypePtr>			RangeSetMap;
typedef RangeSetMap::iterator		RangeSetMapIt;
typedef RangeSetMap::const_iterator	RangeSetMapCIt;

////////////////////////////Seg_t
class Seg_t : public Struc_t
{
	typedef	Struc_t	Base_t;
	friend class ProjectInfo_t;

protected:
	MyString	mTitle;

//#if(USE_SEG_RANGES)
	//TypePtr	mpRange;//the container is in a super seg
	
//#else
	ADDR64		mBase64;//used in range sges only!
	//ADDR		mBase;
//#endif

	Block_t	mRaw;

	uint32_t		mFlags;
	//bool		mbLittleEnd;

	//seg-seg interlink
	TypePtr		mpTraceLink;//Seg_t
	ADDR		mAddress;//attachment point in super seg (a trace's address if it has one)
	//unsigned	mValidRange;//for all VAs falling in this range a condition is true: mValidRange == rawOffs

	unsigned	miFront;//front end index
	ADDR		mEntryPoint;
	//TypePtr		mpSegRange;//for VA resolving (weak)

	SubSegMap	mSubSegs;
	TypePtr		mpSuperLink;

	ROWID		mViewOffs;
	ROWID		mViewSize0;//not including trace oversize

	//ConflictFieldMap	mConflictFields;//fields mapped to the same address

#if(NEW_VARSIZE_MAP)
	std::map<ADDR, unsigned>	mCodeSzMap;//cached code chunks sizes
#endif

	//int mSize;

public:
	Seg_t();// TypePtr);
	//Seg_t(TypePtr pSuperior, int type = 0);
	virtual ~Seg_t();
	void setTitle(const MyString &s) { mTitle = s; }
	const MyString &title() const { return mTitle; }
	void setSuperLink(TypePtr p) { mpSuperLink = p; }
	TypePtr superLink() const { return mpSuperLink; }
	Seg_t &asSeg() const { return const_cast<Seg_t &>(*this); }
	virtual Seg_t * typeSeg() const { return const_cast<Seg_t *>(this); }
	virtual bool maybeUnion() const override { return false; }
	//int frontId() const {
		//return frontEnd() ? frontEnd()->id() : 0; }
	//MyString frontName() const {
		//return frontEnd() ? frontEnd()->name() : ""; }
	const SubSegMap &subsegs() const { return mSubSegs; }
	SubSegMap &subsegs() { return mSubSegs; }
	bool hasSubsegs() const { return !mSubSegs.empty(); }

#if(NEW_VARSIZE_MAP)
	void setVarObjSize(ADDR, unsigned sz);
	unsigned varObjSize(ADDR) const;
	void clearVarObjMap(){ mCodeSzMap.clear(); }
#endif

	//virtual void releaseDevice();
	//void setFrontEnd(const char *, int id);
	//void clearFrontEnd();
	//FRONT_t *frontEnd() const { return mpFront; }
	unsigned frontIndex() const { return miFront; }
	void setFrontIndex(unsigned i){ miFront = i;  }
	//int addEntryPointV(ADDR addr);
	Block_t &rawBlock() { return mRaw; }
	const Block_t &rawBlock() const { return mRaw; }

//	bool isInside(ADDR va) const {
	//	return (base(nullptr) <= va && va < base(nullptr) + size(nullptr));
	//}

	TypePtr traceLink() const { return mpTraceLink; }
	//TypePtr	inferLink() const { return mpInferior; }
	//void setInferLink(TypePtr p){ assert(!p || p->typeSeg()); mpInferior = p; }
	void setTraceLink(TypePtr p){ assert(!p || p->typeStruc()); mpTraceLink = p; }
	//void setInferLink0(TypePtr p){ mpInferior = p; }//needed for the load
	void setTraceLink0(TypePtr p){ mpTraceLink = p; }//needed for the load
	void setAddressP(ADDR va){ mAddress = va; }
	ADDR addressP() const { return mAddress; }
	ADDR addressP2(TypePtr) const;
	unsigned affinity() const { 
		return isLarge() ? 2 : (isSmall() ? 1 : 0) ; }

	virtual int ObjType() const { return OBJID_TYPE_SEG; }
	virtual void	namex(MyString &) const;
	
	//bool	isInside(ADDR va) const { return (mBase <= va && va < mBase + size0()); }
	int		AdjustSegPos(FieldPtr );//on offset change
	FieldMapIt	NextNL(FieldMapIt it);
	int				CountNL();//count mlocs but labels

	Seg_t *		Split(uint32_t addr);

	////////////////////////////////
	uint32_t uflags() const { return mFlags; }
	void setUFlags(uint32_t f){ mFlags = f; }
	bool isReadOnly() const { return (mFlags & I_SuperModule::ISEG_CONST) != 0; }
	void setPhantom(bool b){ if (b) mFlags |= PHANTOMSEG_FLAG; else mFlags &= ~PHANTOMSEG_FLAG; }
	bool isPhantom() const { return (mFlags & PHANTOMSEG_FLAG) != 0; }
	bool isRangeSet() const { return (mFlags & RANGESET_FLAG) != 0; }
	void setRangeSet(bool b){ if (b) mFlags |= RANGESET_FLAG; else mFlags &= ~RANGESET_FLAG; }

	//int setSize(TypePtr iSelf, int sz);
	virtual int size(CTypePtr) const;//with trace
	int size0(CTypePtr) const;//without trace
	static TypePtr ownerRangeSet(CTypePtr iSelf);
	bool addRange(TypePtr, ADDR addr, unsigned size);//seg
	bool deleteRange(HRangeSet, TypePtr);
	bool removeSubseg(TypePtr);

#if(USE_SEG_RANGES)
	//void setRange(TypePtr p);// { assert(!p || !mpRange); mpRange = p; }
	TypePtr range(TypePtr p) const { return p; } //mpRange;}
	virtual ADDR base(CTypeBasePtr p) const;// { return mpRange ? mpRange->base : 0; }
	//void setBase(ADDR n){ mBase = n; }
	void setBase64(ADDR64 n){ mBase64 = n; }
	ADDR64 base64() const { return mBase64; }
	ADDR64 addr64(ADDR va, CTypePtr self) const { return imageBase(self) + va; }
	ADDR64 imageBase(CTypePtr self) const;
#else
	virtual ADDR base(CTypeBasePtr) const { return mBase; }
	void setBase(ADDR n){ mBase = n; }
	void setImageBase(uint64_t n){ mImageBase = n; }
	uint64_t imageBase() const { return mImageBase; }
#endif

	//bool isLarge() const { return (mImageBase & 0xFFFFFFFF00000000) != 0; }
	bool isLarge() const { return (mFlags & I_SuperModule::ISEG_BITMASK) == I_SuperModule::ISEG_64BIT; }
	bool isSmall() const { return (mFlags & I_SuperModule::ISEG_BITMASK) == I_SuperModule::ISEG_16BIT; }
	bool isLittleEnd() const { return (mFlags & I_SuperModule::ISEG_MSB) == 0; }

	OpType_t ptrSize() const { return isLarge() ? OPSZ_QWORD : OPSZ_DWORD; }

	TypePtr		findSubseg(ADDR, unsigned) const;
	TypePtr		findSubseg64(ADDR64) const;
	FieldPtr	FindFieldV(ADDR);
	FieldMapIt field_itx(ADDR &addr, FieldMapIt *ppNext = nullptr, int * pnOversize = nullptr);

	//ConflictFieldMap	&conflictFields(){ return mConflictFields; }
	//const ConflictFieldMap	&conflictFields() const { return mConflictFields; }

	//void setEntryPoint(ADDR a){ mEntryPoint = a; }
	//ADDR entryPoint() const { return mEntryPoint; }
	//Seg_t *entryPointV(ADDR &) const;
	//virtual ROWID VA2DA(ADDR) const;
	ROWID viewSize0() const {
		//assert(mViewOffs != (ROWID)-1);
		return mViewSize0;
	}
	ROWID viewOffs() const { /*assert(mViewOffs != (ROWID)-1);*/ return mViewOffs; }
	ROWID viewOffsAt(CTypePtr iSelf, ADDR va) const;
	void setView(ROWID off, ROWID sz){
		mViewOffs = off;
		mViewSize0 = sz;
	}

	ADDR segSize(CTypePtr) const;

	//bool isSegList() const;
	//FieldMap &superLink() const { return mpInferior->typeSeg()->fields(); }
	bool registerSubseg0(TypePtr);//for loading
	//bool unregisterSubseg(TypePtr);
	//Seg_t *seglistOwner() const;

	//virtual Seg_t *owner Seg(){ return this; }

	//virtual void printContents(std::ostream &, int) const;
	virtual const char *printType() const { return "seg"; }
	//virtual void Print(std::ostream &os, int l);
	static char *printVA(CTypePtr iSelf, ADDR va, char buf[32], unsigned minw = 0);
	static void* ADDR2PV(CTypePtr, ADDR);
	static ADDR PV2ADDR(CTypePtr, void*);
};



/*class TypeSegate_t : public Struc_t
{
	//int		mSize;
	TypePtr	mpSeg;//Seg_t
public:
	TypeSegate_t() : mpSeg(nullptr){}
	TypeSegate_t(unsigned size) : Struc_t(size), mpSeg(nullptr){}
	virtual ~TypeSegate_t();
	virtual void namex(MyString &s) const { s = "segate"; }
	void setSeg(TypePtr pSeg)
	{
		assert(!pSeg || pSeg->typeSeg());
		if (mpSeg)
			mpSeg->typeSeg()->setSegRef(nullptr);
		mpSeg = pSeg;
		if (mpSeg)
			mpSeg->typeSeg()->setSegRef(typ eobj());
	}
	void setSeg0(TypePtr p){ mpSeg = p; }
	TypePtr seg() const { return mpSeg; }
	//int size() const { return mSize; }
	//void setSize(int n){ mSize = n; }
	virtual int ObjType() const { return OBJID_TYPE_SEGREF; }
protected:
	virtual	TypeSegate_t *typeSegRef() const { return const_cast<TypeSegate_t *>(this); }
};*/



