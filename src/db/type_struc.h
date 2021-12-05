#pragma once

#include <list>
#include <map>

#include "shared/misc.h"
#include "types.h"
#include "field.h"
//#include "info_proj.h"
#include "interface/IADCMain.h"


enum SCOPEX_enum
{
	SCOPEX_BITSET = SCOPE__LAST + 1//must not be use in FE
};


class NamesMgr_t;
template <typename T> class XRef_t;
class Field_t;
class Locus_t;
class I_Code;
class I_DynamicType;
struct ins_t;
struct ins_desc_t;
class IPCode_t;
class I_FrontDC;







////////////////////////////////////////////////Struc_t

class Struc_t : public Complex_t
{
	typedef	Complex_t	Base_t;
protected:
	//unsigned		mSize;
	FieldMap		mFields;

	TypesMgr_t *	mpTypesMgr;		//strucs and function types definitions

public:
	Struc_t();
	Struc_t(int);//id override
	//Struc_t(unsigned size);
	virtual ~Struc_t();
	Struc_t(const Struc_t &);
	Struc_t &operator=(const Struc_t &);
	void moveFrom(Struc_t &);
	//void CopyStrucFrom(const Struc_t &, MemoryMgr_t &);

	Struc_t &asStruc() const { return const_cast<Struc_t &>(*this); }

	virtual bool maybeUnion() const { return true; }
	virtual ADDR base(CTypeBasePtr) const;
	virtual void namex(MyString &) const;
	//int		Type() const;
	//int		offset() const;
	virtual int ObjType() const { return OBJID_TYPE_STRUC; }
	virtual Struc_t * typeStruc() const { return const_cast<Struc_t *>(this); }
	FieldMap &fields() { return mFields; }
	const FieldMap &fields() const { return mFields; }
	//int		fieldsCount() const	{ return (int)mFields.size(); }
	bool	hasFields() const { return !mFields.empty(); }
	bool	hasChildren() const{ return !mFields.empty(); }
	bool	IsMineField(FieldPtr) const;
	CFieldPtr	hasOnlyChild() const;

	//virtual void setFieldsOwner(TypePtr);
	
	FieldPtr takeField(ADDR);
	FieldPtr takeField(FieldPtr);//slow!
	FieldPtr takeField0(ADDR);

	FieldPtr takeField0(FieldMapIt);
	FieldMapIt insertUniqueFieldIt(ADDR, FieldPtr, FieldPtr *ppEos);
	FieldMapIt insertFieldIt(FieldPtr, FieldPtr *ppEos);//not unique key

	int		MergeWith(TypePtr pStruc);
	//int		MergeWith(const char *);
//	F ile_t *Get OwnerFile(int16_t *pnFileId = 0);
	int IsIncapsulatedWith(Struc_t *, int *);
	//void ShiftFields(int dOffs);
	bool	IsEqualTo(Struc_t *);
	bool isUnion(CTypePtr) const;

	//int size0() const { return mSize; }
	//void setSize0(int n){ mSize = n; }
	//bool isSizeFixed() const { return mSize != (unsigned)-1;}

	//virtual int		setSize(TypePtr, int);
	virtual int		size(CTypePtr) const;
	//virtual int		sizex() const { return size(); }//special for dc

	FieldPtr 	GetFieldEx(ADDR nOffset, int nMode = 0);


	int ApplyStruct(int nOffset, Struc_t *pStruc);
//	int SetStruct(Struc_t *pStruc);
	int OnSizeIncrease();
	CFieldPtr  GetOverlappingField(int &nLevel, FieldMapCIt it) const;

	virtual TypesMgr_t * typeMgr() const { return mpTypesMgr; }
	void setTypesMgr0(TypesMgr_t *p){ mpTypesMgr = p; }

	//Struc_t *root() const;

	//virtual void Print(std::ostream &os, int);
	//virtual void printContents(std::ostream &, int) const;
	virtual const char *printType() const { return "struct"; }

	//this will step not into a shared strucs
	class HierIterator : public std::vector<std::pair<CTypePtr, FieldMapIt> >
	{
		bool	mbStepIntoShared;
		std::vector<CTypePtr>	mScoping;
	public:
		HierIterator(CTypePtr iStruc, bool bStepIntoShared = false)
			: mbStepIntoShared(bStepIntoShared)
		{
			push(iStruc);
		}
		CFieldRef operator*() const {
			return *(back().second);
		}
		FieldRef operator*(){
			return *(back().second);
		}
		operator bool() const { return !empty(); }
		HierIterator& operator ++();
		HierIterator& operator ++(int){ return operator++(); }
		TypePtr NamespaceStrucSlow() const;
		TypePtr NamespaceStruc() const
		{
			CTypePtr pScope(!mScoping.empty() ? mScoping.back() : nullptr);
			assert(pScope == NamespaceStrucSlow());
			return (TypePtr)pScope;
		}
	private:
		bool push(CTypePtr iStruc)
		{
			FieldMap &l(iStruc->typeStruc()->fields());
			if (l.empty())
				return false;
			push_back(std::make_pair(iStruc, l.begin()));
			if (iStruc->typeStruc()->namesMgr())
				mScoping.push_back(iStruc);
			return true;
		}
		void pop()
		{
			if (!mScoping.empty() && back().first == mScoping.back())
				mScoping.pop_back();
			pop_back();
			if (!empty())
				back().second++;
		}
	};

	//this will step into a shared strucs
	class HierIterator2 : public std::vector<std::pair<TypePtr, FieldMapCIt> >
	{
	public:
		HierIterator2(TypePtr iStruc)
		{
			push(iStruc);
		}
		CFieldRef operator*() const {
			return *VALUE(back().second);
		}
		//FieldRef operator*(){ return *VALUE(back().second); }
		operator bool() const { return !empty(); }
		HierIterator2& operator ++();
		HierIterator2& operator ++(int){ return operator++(); }
		TypePtr NamespaceStruc() const;
	private:
		bool push(TypePtr iStruc)
		{
			FieldMap &l(iStruc->typeStruc()->fields());
			if (l.empty())
				return false;
			push_back(std::make_pair(iStruc, l.begin()));
			return true;
		}
		void pop()
		{
			pop_back();
			if (!empty())
				back().second++;
		}
	};
};




/*class TypeUnion_t : public Struc_t
{
	typedef	Struc_t	Base_t;
public:
	TypeUnion_t(){}
	TypeUnion_t(int id) : Struc_t(id) {}//id override
	~TypeUnion_t(){}

	virtual int size(CTypePtr) const;
	virtual TypeUnion_t * typeUnion() const { return const_cast<TypeUnion_t *>(this); }
	virtual int ObjType() const { return OBJID_TYPE_UNION; }
	virtual const char *printType() const { return "union"; }
};*/

struct ADDRX0
{
	TypePtr pStruc;
	ADDR addr;
};

struct ADDRX : public ADDRX0
{
	ADDRX(){ pStruc = 0; addr = ADDR(-1); }
	ADDRX(TypePtr p, ADDR a){ pStruc = p; addr = a; }
	ADDRX(const ADDRX0 &a){ pStruc = a.pStruc; addr = a.addr; }
};


/////////////////////////////////////////////////////////////////////// Bitset
#define BITS2BYTES(arg)	(arg / CHAR_BIT + ((arg % CHAR_BIT) ? 1 : 0))

// bitset array
class Bitset_t : public Struc_t
{
public:
	Bitset_t(){}
	/*virtual unsigned sizeBytes(CTypePtr iSelf) const {
		unsigned sz(size(iSelf));
		return BITS2BYTES(sz);
	}*/

	virtual int ObjType() const { return OBJID_TYPE_BITSET; }
	virtual Bitset_t * typeBitset() const { return const_cast<Bitset_t *>(this); }
	virtual int size(CTypePtr iSelf) const {
		return sizeBytes(iSelf) * CHAR_BIT;
	}
	virtual unsigned sizeBytes(CTypePtr iSelf) const;
	virtual ADDR	base(CTypeBasePtr) const { return 0; }
	virtual bool maybeUnion() const override { return false; }
	//virtual void aka(MakeAlias &) const;
	void insertField(FieldPtr);
	unsigned upperGap(TypePtr) const;
	unsigned upperBound() const;
};





