#pragma once

#include <set>
#include <list>
#include <map>
#include <string>
#include <iostream>

#include "names.h"

/////////////////////////////
// T Y P E _ t

class FuncDef_t;
class TypeFunc_t;
class TypePair_t;
class Struc_t;
class Type_t;
class Field_t;
class TypeObj_t;
class Simple_t;
class Complex_t;
class Array_t;
class TypePtr_t;
class TypeImp_t;
class TypeExp_t;
class TypeThisPtr_t;
class TypeVPtr_t;
class TypesMgr_t;
class Strucvar_t;
//class TypeUnion_t;
class TypeThunk_t;
//class TypeFuncEnd_t;
class TypeCode_t;
class Dc_t;
//class VarArray_t;
class Project_t;
class Typedef_t;
class Module_t;
class Bit_t;
class Bitset_t;
class TypeProxy_t;
class TypeEnum_t;
class TypeModifier_t;
class TypeArrayIndex_t;
class TypeClass_t;
//class TypeVFTable_t;
//class TypeVBTable_t;
//class TypeCVFTable_t;
//class TypeLVFTable_t;
class TypeRef_t;
class TypeRRef_t;
class TypeNamespace_t;
class TypeConst_t;
//class TypeRTTI_TD_t;
//class TypeRTTI_BCD_t;
//class TypeRTTI_CHD_t;
//class TypeRTTI_BCA_t;
//class TypeRTTI_COL_t;
#if(!NEW_LOCAL_VARS)
class TypeUnionLoc_t;
#endif
class TypeStrucLoc_t;

#ifdef _DEBUG
typedef	int	COMP_ID;
#else
typedef	unsigned	COMP_ID;
#endif

template <typename T> class TMakeAlias;
typedef TMakeAlias<CTypePtr>	MakeAlias;

int AgreeTypes(uint8_t &typ1, uint8_t &typ2);

////////////////////////////////////////
class Type_t
{
	friend class TypesMgr_t;

public:
	Type_t();
	virtual ~Type_t();
	Type_t(const Type_t &);
	Type_t &operator=(const Type_t &);

	Type_t &asType() const { return const_cast<Type_t &>(*this); }
	bool	nameless() const { return name() == nullptr; }

	virtual void		setName(PNameRef){}//just an assignment
	virtual void		setNameRef(PNameRef){}//just an assignment
	virtual PNameRef	name() const { return nullptr; }//for named objects
	virtual void		namex(MyString &s) const { s = "?"; }//if unnamed - generate name (autoname)
	virtual void		defaultName(MyString &s) const { namex(s); }
	virtual void		aka(MakeAlias &) const = 0;

	virtual Project_t	*typeProject() const { return nullptr; }
	virtual Simple_t	*typeSimple() const { return nullptr; }
	virtual Complex_t	*typeComplex() const { return nullptr; }
	virtual Struc_t		*typeStruc() const { return nullptr; }
	virtual TypeClass_t	*typeClass() const { return nullptr; }
	//virtual TypeUnion_t	*typeUnion() const { return nullptr; }//Complex_t
	virtual FuncDef_t	*typeFuncDef() const { return nullptr; }
	virtual TypeFunc_t	*typeFunc() const { return nullptr; }
	virtual TypePair_t	*typePair() const { return nullptr; }
	virtual TypeProc_t	*typeProc() const { return nullptr; }
	virtual Array_t		*typeArray() const { return nullptr; }
	virtual TypePtr_t	*typePtr() const { return nullptr; }
	virtual TypeVPtr_t	*typeVPtr() const { return nullptr; }
	virtual TypeRef_t	*typeRef() const { return nullptr; }
	virtual TypeRRef_t	*typeRRef() const { return nullptr; }
	virtual TypeImp_t	*typeImp() const { return nullptr; }
	virtual TypeExp_t	*typeExp() const { return nullptr; }
	virtual TypeThisPtr_t	*typeThisPtr() const { return nullptr; }
	virtual Typedef_t	*typeTypedef() const { return nullptr; }
	virtual Seg_t		*typeSeg() const { return nullptr; }
	//virtual Dc_t		*typedC() const { return nullptr; }
	virtual	Module_t	*typeModule() const { return nullptr; }
	virtual TypeCode_t		*typeCode() const { return nullptr; }
	virtual Strucvar_t	*typeStrucvar() const { return nullptr; }
	virtual TypeThunk_t *	typeThunk() const { return nullptr; }
	//virtual TypeFuncEnd_t	*typeFuncEnd() const { return nullptr; }
	//virtual Bit_t *		typeBit() const { return nullptr; }
	virtual Bitset_t * typeBitset() const { return nullptr; }
	virtual TypeProxy_t * typeProxy() const { return nullptr; }
	virtual TypeEnum_t * typeEnum() const { return nullptr; }
	//modifiers
	virtual TypeModifier_t * typeModifier() const { return nullptr; }
	virtual TypeArrayIndex_t * typeArrayIndex() const { return nullptr; }
	//virtual TypeVFTable_t * typeVFTable() const { return nullptr; }
	//virtual TypeVBTable_t * typeVBTable() const { return nullptr; }
	//virtual TypeCVFTable_t * typeCVFTable() const { return nullptr; }
	//virtual TypeLVFTable_t * typeLVFTable() const { return nullptr; }
	virtual TypeNamespace_t * typeNamespace() const { return nullptr; }
	virtual TypeConst_t * typeConst() const { return nullptr; }
#if(!NEW_LOCAL_VARS)
	virtual TypeUnionLoc_t * typeUnionLoc() const { return nullptr; }
#endif
	virtual TypeStrucLoc_t * typeStrucLoc() const { return nullptr; }

	//virtual TypeRTTI_TD_t * typeRTTI_TD() const { return nullptr; }
	//virtual TypeRTTI_BCD_t * typeRTTI_BCD() const { return nullptr; }
	//virtual TypeRTTI_CHD_t * typeRTTI_CHD() const { return nullptr; }
	//virtual TypeRTTI_BCA_t * typeRTTI_BCA() const { return nullptr; }
	//virtual TypeRTTI_COL_t * typeRTTI_COL() const { return nullptr; }

	//virtual unsigned sizeBytes(unsigned n) const { return n; }
	virtual unsigned sizeBytes(CTypePtr p) const {
//CHECK(this ==0)
//STOP
		return size(p);
	}
	virtual int size(CTypePtr) const { return 0; }
	//virtual uint8_t optype() const { return OPTYP_NULL; }
	virtual ADDR	base(CTypeBasePtr) const { return 0; }
	//virtual TypePtr type() const { return 0; }
	virtual void releaseDevice(){}//for those with exclusive ownership of a device
	virtual int ObjType() const = 0;
	virtual TypesMgr_t * typeMgr() const { return nullptr; }
	virtual const char *printType() const { return nullptr; }
	virtual TypePtr	baseType() const { return nullptr; }
	virtual void setBaseType(TypePtr){}

	virtual COMP_ID		ID() const { return 0; }
};

class Complex_t : public Type_t
{
	COMP_ID		mID;
	static COMP_ID	sIdSeed;
public:
	virtual COMP_ID		ID() const { return mID; }
	//void	setId(COMP_ID id){ mID = id; }
//	bool	checkId(COMP_ID id) const { return (id == mID); }
#ifdef _DEBUG
	static COMP_ID	sIdSeedNeg;
	static COMP_ID nextNegativeId() { return --sIdSeedNeg; }
#endif
	static void resetUniqueId() {
		sIdSeed = 0;
#ifdef _DEBUG
		sIdSeedNeg = 0;
#endif
	}

protected:
	typedef	Type_t	Base_t;
	NamesMgr_t* mpNamespace;

public:
	Complex_t();
	Complex_t(COMP_ID);//id override
	virtual ~Complex_t();
	Complex_t(const Complex_t&);
	Complex_t& operator=(const Complex_t&);
	void moveFrom(Complex_t&);

	void deleteNamespace(MemoryMgr_t&);

	virtual Complex_t* typeComplex() const { return const_cast<Complex_t*>(this); }
	Complex_t& asComplex() const { return const_cast<Complex_t&>(*this); }

	//Type_t *	AssureObjType(ObjId_t objid);
	int			isValid() const;

	NamesMgr_t* assureNamespace();
	NamesMgr_t* namesMgr() const { return mpNamespace; }
	//virtual NamesMgr_t *owner namesMgr();

	virtual void aka(MakeAlias&) const;
	virtual PNameRef name() const;
	virtual void setName(PNameRef);
	virtual void setNameRef(PNameRef);

	//void recoverNamespace();

	//bool nameless() const { return name() == nullptr; }
	//MyString namexx() const;//a helper
};

class TypeModifier_t : public Type_t
{
public:
	TypeModifier_t(){}
	virtual TypePtr	incumbent() const = 0;
protected:
	virtual TypeModifier_t * typeModifier() const { return const_cast<TypeModifier_t *>(this); }
};

// just a bit
/*class Bit_t : public Type_t
{
public:
	Bit_t()
	{
	}
	virtual int ObjType() const { return OBJID_TYPE_BIT; }
	virtual void aka(MakeAlias &) const;
	virtual void namex(MyString &) const;
	virtual Bit_t * typeBit() const { return const_cast<Bit_t *>(this); }
	virtual int size(CTypePtr) const { return 1; }//IN BITS!
};*/

class Simple_t : public Type_t
{
	typedef	Type_t	Base_t;
	OpType_t	mTypeID;
public:
	Simple_t() : mTypeID(OPTYP_NULL){}
	Simple_t(OpType_t typeID);
	Simple_t &asSimple() const { return const_cast<Simple_t &>(*this); }
	virtual void namex(MyString &) const;
	virtual int size(CTypePtr) const { return opsize(); }
	virtual uint8_t optype() const { return (uint8_t)mTypeID; }
	OpType_t typeID() const { return mTypeID; }
	void setTypeID(OpType_t t){ mTypeID = t; }
	uint8_t opsize() const { return uint8_t(mTypeID & 0xF); }
protected:
	virtual int ObjType() const { return OBJID_TYPE_SIMPLE; }
	virtual void aka(MakeAlias &) const;
	virtual Simple_t * typeSimple() const { return const_cast<Simple_t *>(this); }
};

///////////////////////////////////////////////////// TypePtr_t
class TypePtr_t : public Simple_t
{
	typedef	Simple_t	Base_t;
protected:
	TypePtr mpBaseType;
public:
	TypePtr_t() : mpBaseType(nullptr){}
	TypePtr_t(OpType_t t) : Simple_t(t), mpBaseType(nullptr){}
	virtual ~TypePtr_t();
	virtual void namex(MyString &) const;
	int		level();
	virtual TypePtr	pointee() const { return baseType(); }
	void setPointee0(TypePtr p){ mpBaseType = p; }
	virtual uint8_t optype() const { return MAKETYP_PTR(Simple_t::opsize()); }
	virtual TypePtr	baseType() const { return mpBaseType; }
	virtual void setBaseType(TypePtr p){
		mpBaseType = p;
	}
protected:
	virtual int ObjType() const { return OBJID_TYPE_PTR; }
	virtual void aka(MakeAlias&) const;
	virtual TypePtr_t *	typePtr() const { return const_cast<TypePtr_t *>(this); }
};

///////////////////////////////////////////////////// TypeImp_t
class TypeImp_t : public Type_t
{
	TypePtr mpBaseType;
public:
	TypeImp_t() : mpBaseType(nullptr){}
	void namex(MyString& s) const;
	virtual TypePtr	baseType() const { return mpBaseType; }
	virtual void setBaseType(TypePtr p){
		mpBaseType = p;
	}
protected:
	virtual TypeImp_t *	typeImp() const { return const_cast<TypeImp_t *>(this); }
	virtual int size(CTypePtr) const;
	virtual int ObjType() const { return OBJID_TYPE_IMP; }
	virtual void aka(MakeAlias&) const;
	virtual Simple_t* typeSimple() const;
	virtual Struc_t* typeStruc() const;
	virtual Array_t* typeArray() const;
	virtual TypePtr_t* typePtr() const;
};

///////////////////////////////////////////////////// TypeExp_t
class TypeExp_t : public Type_t
{
	TypePtr mpBaseType;
public:
	TypeExp_t() : mpBaseType(nullptr){}
	void namex(MyString& s) const;
	virtual TypePtr	baseType() const { return mpBaseType; }
	virtual void setBaseType(TypePtr p){
		mpBaseType = p;
	}
protected:
	virtual TypeExp_t *	typeExp() const { return const_cast<TypeExp_t *>(this); }
	virtual int size(CTypePtr) const;
	virtual int ObjType() const { return OBJID_TYPE_EXP; }
	virtual void aka(MakeAlias&) const;
	virtual Simple_t* typeSimple() const;
	virtual Struc_t* typeStruc() const;
	virtual Array_t* typeArray() const;
	virtual TypePtr_t* typePtr() const;
};

///////////////////////////////////////////////////// TypeThisPtr_t
class TypeThisPtr_t : public TypePtr_t
{
public:
	TypeThisPtr_t(){}
	TypeThisPtr_t(OpType_t t) : TypePtr_t(t){}
	void namex(MyString &s) const {
		if (typeID() == OPSZ_DWORD)
			s.append("__thisptr32");
		else if (typeID() == OPSZ_QWORD)
			s.append("__thisptr64");
		else
			ASSERT0;
	}
protected:
	virtual TypeThisPtr_t *	typeThisPtr() const { 
		return const_cast<TypeThisPtr_t *>(this); }
	virtual int ObjType() const { return OBJID_TYPE_THISPTR; }
	virtual void aka(MakeAlias&) const;
};

///////////////////////////////////////////////////// TypeVPtr_t
class TypeVPtr_t : public TypePtr_t
{
public:
	TypeVPtr_t(){}
	TypeVPtr_t(OpType_t t) : TypePtr_t(t){}
	void namex(MyString &s) const {
		if (typeID() == OPSZ_QWORD)
			s.append("__vptr64");
		else if (typeID() == OPSZ_DWORD)
			s.append("__vptr32");
		else
			ASSERT0;
	}
protected:
	virtual TypeVPtr_t *	typeVPtr() const { 
		return const_cast<TypeVPtr_t *>(this); }
	virtual int ObjType() const { return OBJID_TYPE_VPTR; }
	virtual void aka(MakeAlias&) const;
};

///////////////////////////////////////////////////// TypeRef_t
class TypeRef_t : public TypePtr_t
{
public:
	TypeRef_t(){}
	TypeRef_t(OpType_t t) : TypePtr_t(t){}
	void namex(MyString &s) const;
protected:
	virtual TypeRef_t *	typeRef() const { return const_cast<TypeRef_t *>(this); }
	virtual int ObjType() const { return OBJID_TYPE_REF; }
	virtual void aka(MakeAlias &) const;
};

///////////////////////////////////////////////////// TypeRRef_t
class TypeRRef_t : public TypeRef_t
{
public:
	TypeRRef_t(){}
	TypeRRef_t(OpType_t t) : TypeRef_t(t){}
	void namex(MyString &s) const;
protected:
	virtual TypeRRef_t *typeRRef() const { return const_cast<TypeRRef_t *>(this); }
	virtual int ObjType() const { return OBJID_TYPE_RREF; }
	virtual void aka(MakeAlias &) const;
};

/////////////////////////////////////////////////////////////////////// TypeEnum_t
class TypeEnum_t : public Simple_t
{
	TypePtr		mpBaseType;
public:
	TypeEnum_t() : mpBaseType(nullptr){}
	TypeEnum_t(OpType_t t) : Simple_t(t), mpBaseType(nullptr){}
	TypePtr	enumRef() const { return baseType(); }
	void setEnumRef(TypePtr p){ mpBaseType = p; }
	//virtual PNameRef name() const;
	virtual void namex(MyString &) const;
	virtual TypePtr	baseType() const { return mpBaseType; }
	virtual void setBaseType(TypePtr);
protected:
	virtual int ObjType() const { return OBJID_TYPE_ENUM; }
	virtual TypeEnum_t * typeEnum() const { return const_cast<TypeEnum_t *>(this); }
	virtual void aka(MakeAlias&) const;
	virtual const char *printType() const { return "enum"; }
};


//////////////////////////////////////////////////////////
class Array_t : public Type_t
{
	typedef	Type_t	Base_t;
	TypePtr		mpBaseType;
	unsigned			mTotal;//if ARRAY_BYTES_MASK - number of elements, otherwise - size in bytes
public:
	Array_t() : mpBaseType(nullptr), mTotal(0){}
	//Array_t(size_t total) : mpBaseType(0), mTotal(total){}
	Array_t(TypePtr pType, int total);
	virtual ~Array_t();
	virtual void namex(MyString &) const;
	virtual int size(CTypePtr) const;

	TypePtr absBaseType() const;//absolute base (non-array)
	unsigned total0() const { return mTotal; }
	void setTotal0(unsigned n){ mTotal = n; }
	unsigned total() const;
	virtual TypePtr baseType() const { return mpBaseType; }
	virtual void setBaseType(TypePtr p){ mpBaseType = p; }
protected:
	virtual int ObjType() const { return OBJID_TYPE_ARRAY; }
	virtual Array_t * typeArray() const { return const_cast<Array_t *>(this); }
	virtual void aka(MakeAlias&) const;
};


///////////////////////////////////////////////////////////////////////TypeArrayIndex_t
class TypeArrayIndex_t : public TypeModifier_t
{
	TypePtr	mpArrayRef;//arrays only!
	TypePtr	mpIndexRef;//index - enums only!
public:
	TypeArrayIndex_t() 
		: mpArrayRef(nullptr),
		mpIndexRef(nullptr)
	{
	}
	TypePtr setArrayRef(TypePtr p);
	TypePtr arrayRef() const { return mpArrayRef; }
	TypePtr setIndexRef(TypePtr p);
	TypePtr indexRef() const { return mpIndexRef; }
protected:
	virtual TypeArrayIndex_t * typeArrayIndex() const { return const_cast<TypeArrayIndex_t *>(this); }
	virtual int ObjType() const { return OBJID_TYPE_ARRAY_INDEX; }
	virtual void aka(MakeAlias&) const;
	virtual int size(CTypePtr) const;
	virtual TypePtr	incumbent() const { return mpArrayRef; }
	virtual void namex(MyString &) const;
};


///////////////////////////////////////////////////////////////////////TypeConst_t
class TypeConst_t : public TypeModifier_t
{
	typedef	TypeModifier_t	Base_t;
public:
	TypePtr	mpBaseType;
public:
	TypeConst_t();
	TypeConst_t(TypePtr);
	~TypeConst_t();
	virtual TypePtr baseType() const { return mpBaseType; }
	virtual void setBaseType(TypePtr p){ mpBaseType = p; }
	void setBaseType0(TypePtr p){ mpBaseType = p; }
	virtual int ObjType() const { return OBJID_TYPE_CONST; }
protected:
	//Type_t
	virtual TypeConst_t * typeConst() const { return const_cast<TypeConst_t *>(this); }
	virtual void aka(MakeAlias &) const;
	virtual void namex(MyString &) const;
	virtual int size(CTypePtr) const;
	//TypeModifier_t
	virtual TypePtr	incumbent() const { return mpBaseType; }
};


///////////////////////////////////////////////////////////////////////Typedef_t
class Typedef_t : public TypeModifier_t
{
	typedef	TypeModifier_t	Base_t;
public:
	TypePtr	mpBaseType;
	PNameRef mpName;
	int	mID;//unique id
	static COMP_ID gID;
public:
	Typedef_t();
	Typedef_t(TypePtr);
	~Typedef_t();

	virtual COMP_ID		ID() const { return mID; }
	//void	SetID(COMP_ID id){ mID = id; }
	static void resetUniqueId(){ gID = 0; }

	//PNameRef Name() const { return mpName; }
	virtual TypePtr baseType() const { return mpBaseType; }
	virtual void setBaseType(TypePtr p){ mpBaseType = p; }
	void setBaseType0(TypePtr p){ mpBaseType = p; }
	virtual int ObjType() const { return OBJID_TYPE_TYPEDEF; }
	virtual const char *printType() const { return "typedef"; }
	virtual PNameRef name() const { return mpName; }
	virtual void setName(PNameRef p){ mpName = p; }
	virtual void setNameRef(PNameRef p){
		mpName = p; 
#if(SHARED_NAMES)
		if (mpName)
			mpName->addRef();
#endif
	}
protected:
	virtual Typedef_t * typeTypedef() const { return const_cast<Typedef_t *>(this); }
	virtual void aka(MakeAlias &) const;
	virtual void namex(MyString &) const;
	virtual int size(CTypePtr) const;
	//uint8_t optype() const;
	virtual TypePtr	incumbent() const { return mpBaseType; }
};


//for types binding
///////////////////////////////////////////////////////////////////////TypeComma_t
class TypePair_t : public Type_t
{
	TypePtr mpLeft;
	TypePtr mpRight;
public:
	TypePair_t()
		: mpLeft(nullptr),
		mpRight(nullptr)
	{
	}
	TypePair_t(TypePtr l, TypePtr r)
		: mpLeft(l),
		mpRight(r)
	{
	}

	void setLeft(TypePtr p){ mpLeft = p; }
	TypePtr left() const { return mpLeft; }
	void setRight(TypePtr p){ mpRight = p; }
	TypePtr right() const { return mpRight; }
protected:
	virtual int size(CTypePtr) const { /*?assert(0);*/ return 0; }
	virtual int ObjType() const { return OBJID_TYPE_PAIR; }
	virtual void aka(MakeAlias&) const;
	virtual TypePair_t * typePair() const { return const_cast<TypePair_t *>(this); }
	virtual void namex(MyString &s) const;
};

///////////////////////////////////////////////////////////////////////TypeFunc_t
class TypeFunc_t : public TypePair_t
{
	unsigned mFlags;
public:
	enum E_FLAGS {//see ProtoProfileFlags
		E_NULL,
		E_CLEANARG	= 0x01,	//a caller supposed to clenup the stack (like stdcall)
		E_VARIARDIC	= 0x02,	//variable args (cdecl)
		E_THISCALL	= 0x10,	//illigal in HLL but here is to typify vtables' entries
		E_USERCALL	= 0x20
	};
	TypeFunc_t() 
		: mFlags(0)
	{
	}
	TypeFunc_t(TypePtr r, TypePtr a, unsigned f)
		: TypePair_t(r, a),
		mFlags(f)
	{
	}
	void setRetVal(TypePtr p){ setLeft(p); }
	TypePtr retVal() const { return left(); }
	void setArgs(TypePtr p){ setRight(p); }
	TypePtr args() const { return right(); }
	void setFlags(unsigned f){ mFlags = f; }
	unsigned flags() const { return mFlags; }
	bool isStdCall() const {
		return ((mFlags & 0xF) == E_CLEANARG);
	}
	bool isUserCall() const {
		return ((mFlags & 0xF) == E_USERCALL);
	}
	bool isFastCall() const {
		return false;//later
	}
	bool isThisCall() const {
		return ((mFlags & E_THISCALL) != 0);
	}
	uint8_t retValOpType() const;
protected:
	virtual TypeFunc_t * typeFunc() const { return const_cast<TypeFunc_t *>(this); }
	virtual int ObjType() const { return OBJID_TYPE_FUNC; }
	virtual void aka(MakeAlias&) const;
	//virtual int size(CTypePtr) const { assert(0); return 0; }
	virtual void namex(MyString &s) const;
};


/*
typedef std::map<std::string, Strucvar_t *>	id2fe_t;
typedef id2fe_t::iterator					id2fe_it;
class RegisterFrontend_t
{
public:
	RegisterFrontend_t(Strucvar_t * t)
	{
		if (!m)
			m = new id2fe_t;

		std::string s = t->name();
		std::pair<id2fe_it, bool> it = m->insert(
			std::pair<std::string, Strucvar_t *>(s, t));
	}
	~RegisterFrontend_t()
	{
		if (m)
		{
			for (id2fe_it it = m->begin(); it != m->end(); ++it)
			{
				Strucvar_t * t = it->second;
				delete t;
			}
			m->clear();
			delete m;
			m = nullptr;
		}
	}
	static Strucvar_t * getTypeCreator(const std::string& s)
	{
		id2fe_it it = m->find(s);
		if (it == m->end())
			return nullptr;
		return it->second;
	}
private:
	static id2fe_t *	m;
}; */


/*class TypeFuncEnd_t : public Type_t
{
public:
	virtual void		namex(MyString &s) const { s = "endp"; }
	virtual int size(){ return 0; }
	virtual TypeFuncEnd_t * typeFuncEnd() const { return const_cast<TypeFuncEnd_t *>(this); }
	virtual int ObjType() const { assert(0); return OBJID_NULL; }
	virtual void		aka(MakeAlias &) const { assert(0); }
};*/


MyString TypeID2Str(OpType_t typeID);






