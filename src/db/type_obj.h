#pragma once

#include "types.h"

class TypeObj0_t : public Obj_t, public UserData_t
{
	Type_t *mp;
						//refs >0 if type-managed
public:
	TypePtr		mpOwnerCplx;//Complex_t	//the struc where this type belongs to

public:
	TypeObj0_t(ObjFlagsType f)
		: Obj_t(f),
		mp(nullptr),
		mpOwnerCplx(nullptr)
	{
	}

	TypeObj0_t(TypeObj0_t&& o)
		: Obj_t(std::move(o)),
		mp(o.mp),
		mpOwnerCplx(o.mpOwnerCplx)
	{
		o.mp = nullptr;
		o.mpOwnerCplx = nullptr;
	}


#if(!NO_OBJ_ID)
	TypeObj0_t(ObjFlagsType f, int id)//to make struc-based autonames immune to object id changes
		: Obj_t(f, id),
		mp(nullptr),
		mpOwnerCplx(nullptr)
	{
	}
#endif

	~TypeObj0_t()
	{
		assert(!hasUserData());
		assert(!mp);
		assert(!mpOwnerCplx);
	}

	//TypeObj_t(const TypeObj_t &);
	//void replaceId(int);

	void SetPvt0(Type_t *p){ mp = p; }
	template <typename T>
	Type_t *SetPvt(){
		assert(!mp);
		mp = new T();
		return mp;
	}
	template <typename T>
	T *SetPvt(T *p){
//CHECKID(this,0x20)
//STOP
		assert(!mp && p);//no nuls
		mp = p;
		return p;
	}
	void ClearPvt(){
		delete mp;
		mp = nullptr;
	}
	void SwapPvts(TypeObj0_t &o){
		Type_t *tmp(mp);
		mp = o.mp;
		o.mp = tmp;
	}
	Type_t *TakePvt(){
		Type_t *p(mp);
		mp = nullptr;
		return p;
	}
	Type_t &pvt() const { return *mp; }
	bool hasPvt() const { return mp != nullptr; }

	Type_t		*type() const { return mp; }
	Project_t	*typeProject() const { return mp->typeProject(); }
	Simple_t	*typeSimple() const { return mp->typeSimple(); }
	Complex_t	*typeComplex() const { return mp->typeComplex(); }
	Struc_t		*typeStruc() const { return mp->typeStruc(); }
	TypeClass_t	*typeClass() const { return mp->typeClass(); }
	Struc_t* typeUnion() const;
#if(!NEW_LOCAL_VARS)
	TypeUnionLoc_t	*typeUnionLoc() const { return mp->typeUnionLoc(); }
#endif
	TypeStrucLoc_t* typeStrucLoc() const { return mp->typeStrucLoc(); }
	FuncDef_t* typeFuncDef() const { return mp->typeFuncDef(); }
	TypeThunk_t* typeThunk() const { return mp->typeThunk(); }
	TypeProc_t* typeProc() const { return mp->typeProc(); }
	Array_t* typeArray() const { return mp->typeArray(); }
	TypePtr_t* typePtr() const { return mp->typePtr(); }
	TypeRef_t* typeRef() const { return mp->typeRef(); }
	TypeRRef_t* typeRRef() const { return mp->typeRRef(); }
	TypeImp_t* typeImp() const { return mp->typeImp(); }
	TypeExp_t* typeExp() const { return mp->typeExp(); }
	TypeVPtr_t* typeVPtr() const { return mp->typeVPtr(); }
	Typedef_t* typeTypedef() const { return mp->typeTypedef(); }
	Seg_t* typeSeg() const { return mp->typeSeg(); }
	Module_t* typeModule() const { return mp->typeModule(); }
	TypeCode_t* typeCode() const { return mp->typeCode(); }
	Strucvar_t* typeStrucvar() const { return mp->typeStrucvar(); }
	//TypeFuncEnd_t	*typeFuncEnd() const { return mp->typeFuncEnd(); }
	//Bit_t			*typeBit() const { return mp->typeBit(); }
	TypeEnum_t* typeEnum() const { return mp->typeEnum(); }
	Bitset_t* typeBitset() const { return mp->typeBitset(); }
	TypeProxy_t* typeProxy() const;//below
	TypeArrayIndex_t* typeArrayIndex() const { return mp->typeArrayIndex(); }
	TypeModifier_t* typeModifier() const { return mp->typeModifier(); }
	//TypeVFTable_t* typeVFTable() const { return mp->typeVFTable(); }
	//TypeCVFTable_t* typeCVFTable() const { return mp->typeCVFTable(); }
	//TypeLVFTable_t* typeLVFTable() const { return mp->typeLVFTable(); }
	//TypeVBTable_t* typeVBTable() const { return mp->typeVBTable(); }
	TypeNamespace_t* typeNamespace() const { return mp->typeNamespace(); }
	TypeConst_t* typeConst() const { return mp->typeConst(); }
	TypePair_t* typePair() const { return mp->typePair(); }
	TypeFunc_t* typeFunc() const { return mp->typeFunc(); }

	void		setName0(PNameRef p){
		mp->setNameRef(p);
		if (mp->name())
			mp->name()->setObj(this);
	}
	void		setName(PNameRef);
	PNameRef	name() const { return mp->name(); }
	void		namex(MyString &s) const { mp->namex(s); }
	void		aka(MakeAlias &buf) const { mp->aka(buf); }
	void		defaultName(MyString &s) const { mp->defaultName(s); }
	TypePtr		baseType() const { return mp->baseType(); }
	void		setBaseType(TypePtr p){ mp->setBaseType(p); }
	TypePtr		absBaseType() const;
	TypePtr		stripToProxy() const;

	int ObjType() const { return mp->ObjType(); }

	bool		nameless() const { return mp->name() == nullptr; }
	//unsigned	sizeBytes(unsigned n) const { return mp->sizeBytes(n); }
	unsigned	sizeBytes() const { return mp->sizeBytes((TypePtr)this); }
	int	size() const { return mp->size((TypePtr)this); }//unsigned causes a crash!
	ADDR	segSize() const;
	ADDR	base() const { return mp->base((TypeBasePtr)this); }
	unsigned	attic() const {
		assert(size() != -1);
		return mp->base((TypeBasePtr)this) + (unsigned)mp->size((TypePtr)this); 
	}

	TypesMgr_t * typeMgr() const { return mp->typeMgr(); }

	void releaseDevice(){ return mp->releaseDevice(); }

	uint16_t tag() const {
		return uint16_t((m_nFlags & STRU__TAG_MASK) >> STRU__TAG_SHFT);
	}
	void setTag(uint16_t u){
		m_nFlags &= ~STRU__TAG_MASK;
		m_nFlags |= (ObjFlagsType(u) << STRU__TAG_SHFT) & STRU__TAG_MASK;
	}
	bool hasUglyName() const {
		return (m_nFlags & TYP_UGLY_NAME) != 0;
	}
	bool hasPrettyName() const {
		return (m_nFlags & TYP_PRETTY_NAME) != 0;
	}
	bool isEnum() const {
		return (m_nFlags & TYP_ENUM) != 0;
	}
	bool isShared() const;
	bool isExporting() const {
		return (m_nFlags & TYP_EXPORTED)  != 0;
	}
	void setExporting(bool b){
CHECKID(this, 0xcb2)
STOP
		if (b)
		{
			m_nFlags |= TYP_EXPORTED;
		}
		else
			m_nFlags &= ~TYP_EXPORTED;
	}
	/*bool isUgly() const {
		return (m_nFlags & TYP_UGLY) != 0; }
	void setUgly(bool b){
		if (b)
			m_nFlags |= TYP_UGLY;
		else
			m_nFlags &= ~TYP_UGLY;
	}*/
	inline TypesMgr_t *ownerTypeMgr() const;
	inline TypePtr ownerScope() const;
	inline bool isScopeOf(TypePtr p0) const;//direct or indirect, also true if p0==this
	TypePtr owner() const { return mpOwnerCplx; }
	void setOwner(TypeBasePtr p){
CHECKID(this,0x000f2696)
STOP
		mpOwnerCplx = (TypePtr)p;
	}
	inline bool isNested() const;

	uint64_t imageBase() const;

	bool destruct(MemoryMgr_t &);

	const char* printType() const;

	bool isVTable() const;
};


typedef TypeObj0_t* TypeBasePtr;
typedef const TypeObj0_t* CTypeBasePtr;



class TypeObj_t : public TypeObj0_t
{
	FieldPtr		mpParentField;		//if no parent, then the type must be under control of type manager
	RefCounter_t	mRefs;			//refs >0 if type-managed
public:
	TypeObj_t()
		: TypeObj0_t(OBJID_TYPEOBJ),
		mpParentField(nullptr)
	{
	}
#if(!NO_OBJ_ID)
	TypeObj_t(int id)
		: TypeObj0_t(OBJID_TYPEOBJ, id),
		mpParentField(nullptr)
	{
	}
#endif
	~TypeObj_t()
	{
		assert(!mpParentField);
	}
	int addRef() {
		return mRefs.addRef();
	}
	int releaseRef() {
		return mRefs.releaseRef();
	}
	int refsNum() const {
		return mRefs.refsNum();
	}
	bool isTypeSimple(OpType_t) const;

	FieldPtr parentField() const { return mpParentField; }
	void setParentField(CFieldPtr p){
CHECKID(this, 0x3464)
STOP
		mpParentField = (FieldPtr)p;
	}
	static bool canConform(CTypePtr type, CTypePtr target);
};

inline TypeProxy_t	*TypeObj0_t::typeProxy() const { return mp->typeProxy(); }


TypesMgr_t* TypeObj0_t::ownerTypeMgr() const {
	return mpOwnerCplx ? mpOwnerCplx->pvt().typeMgr() : 0;
}

TypePtr TypeObj0_t::ownerScope() const {
	return mpOwnerCplx ? (mpOwnerCplx->typeSeg() ? nullptr : mpOwnerCplx) : nullptr;
}

bool TypeObj0_t::isScopeOf(TypePtr p0) const//direct or indirect, also true if p0==this
{
	for (const TypeObj_t* p(p0); p; p = p->ownerScope())//search this among scopes of p0
		if (p == this)
			return true;
	return false;
}

bool TypeObj0_t::isNested() const {
	if (!owner())
		return false;
	return owner()->typeSeg() == nullptr;
}


