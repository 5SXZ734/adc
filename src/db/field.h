#pragma once

#include "qx/MyString.h"
#include "shared/misc.h"
#include "shared/avl_tree2.h"
#include "mem.h"
#include "obj.h"
#include "names.h"
#include "type_obj.h"

class Type_t;
class TypeCode_t;
class FuncDef_t;

#define NO_EXTRA_USER	1

/*struct FieldKeyType
{
	union
	{
		uint32_t	key;	//for globals
		struct//for locals
		{
			uint32_t	offs:24;
			uint32_t	opc:8;
		};
	};
	FieldKeyType(uint32_t k)
		: key(k)
	{
	}
	FieldKeyType(uint32_t o, uint32_t c)
		: offs(o),
		opc(c)
	{
	}
	bool operator<(const FieldKeyType &o){
		return key < o.key;
	}
};*/
//#define	ADDR_STRUCVAR	-1	//dummy offset of non-fixed strucvar's fields

typedef uint32_t FieldKeyType;

#include "shared/sbtree2.h"
typedef sbtree::node_s<Field_t, FieldKeyType>			FieldNodeBase;

#define KEY(a)		a->_key()
#define VALUE(a)	(&(*a))


class Field_t : public Obj_t, public UserData_t, public FieldNodeBase
{
public:
	typedef	FieldNodeBase	TreeLinkInfo;

	// UserData_t is used for:
	//1) an attached path for locals
	//2) funcdefs for fields of type 'func'

	TypePtr	mpType;
	PNameRef	mpName;

	//uint8_t		m_opc;		//class
	//int32_t		mOffset;	//offset from base (if any)

	TypePtr/*TypeBasePtr*/	mpOwner;	//for structure fields

	//MEMTRACE_DECL(Field_t);

public:
	Field_t();
	//Field_t(FieldKeyType);
//	Field_t(const Field_t &);
#if(!NO_OBJ_ID)
	Field_t(int id);
	Field_t(Field_t &&, int id);
#else
	Field_t(Field_t &&);
#endif
	~Field_t();

	bool destruct(MemoryMgr_t &);
	void CopyFrom(CFieldRef, MemoryMgr_t &);

	inline FieldKeyType _key() const { return key; }
	/*int compare(FieldKeyType o) const
	{
		if (_key() < o)
			return -1;
		if (_key() == o)
			return 0;
		return 1;
	}*/

	Field_t & operator=(const Field_t& o)
	{
		Obj_t::operator=(o);
		UserData_t::operator=(o);
		FieldNodeBase::operator=(o);
		mpType = o.mpType;
		mpName = o.mpName;
		//m_opc = o.m_opc;
		//mOffset = o.mOffset;
		mpOwner = o.mpOwner;
		return *this;
	}

	bool	nameless() const { return name() == nullptr; }
#if(!NO_EXTRA_USER)
	Folder_t *folder() const { return mpFolder2; }
	void setFolder(Folder_t *p){ mpFolder2 = p; }
#endif

	//bool nameless() const { return name() == nullptr; }

	uint8_t		OpSize() const;
	OpType_t	OpTypeZ() const;


	//ADDR address() const;

//	void		setOrdinal(uint16_t u){ m_ordinal = u; }
//	uint16_t		ordinal() const { return m_ordinal; }

	AttrIdEnum attrib() const { return (AttrIdEnum)((m_nFlags & FLD_ATTRIB_MASK) >> FLD_ATTRIB_OFFS); }
	void setAttribute(AttrIdEnum);
	void setAttributeFromId(AttrIdEnum);
	void toggleAttribute(AttrIdEnum);

#if(NO_OBJ_ID)
	int		ID() const { return ((~1) >> 1); }
	//void	SetID(int){}
#endif

	TypePtr parentSeg();
	TypePtr parentSegList() const;
	//TypeProc_t *	ownerProcPvt() const;
	TypePtr ownerProc() const;
	void setOwnerComplex(TypePtr);
	TypePtr	OwnerComplex() const;
	Struc_t		*OwnerStruc() const;
	
	TypePtr owner() const { return (TypePtr)mpOwner; }
	FieldPtr parentField();//the field obove in the hierarhy

	int ObjType() const { return OBJID_FIELD; }
	//virtual FieldPtr  objField() const { return const_cast<FieldPtr >(this); }

	void	setName(PNameRef p);
	void	setName0(PNameRef p){
		if (mpName)
			mpName->setObj(nullptr);
		mpName = p; 
		if (mpName)
			mpName->setObj(this);
	}
	PNameRef name() const { return mpName; }
	const char* safeName(int& n) const {
		if (mpName)
			return mpName->name(n);
		n = -1;
		return nullptr;
	}
	/*MyStringEx compoundName() const {
		return mpName ? mpName->compoundName() : MyStringEx();
	}*/

	void	Clear();
	ADDR		checkGap() const;

	bool is_strucvar() const;
	ADDR	addressHi() const {//size maybe 0
		return _key() + size();
	}
	ADDR	addressHi2() const {//size never 0
		int sz(size());
		if (sz == 0)
			sz = 1;
		return _key() + sz;
	}
	//ADDR	addressEx() const;
	ADDR	offset() const;
	ADDR	offset_hi() const {
		return offset() + size();
	}
	void	OnSizeIncrease();

	int		size() const;
	int		sizeBytes() const;
	int		OvercastType(uint8_t);
	bool	IsEqualTo(FieldPtr p);

	//int		OffsetHi();//for unnamed unions && strucs
	int		CheckOverwrite(ADDR nOffset);

	//Field_t	*AssureObjType(ObjId_t objid);

	bool	isGlobal() const;
	//bool	IsData() const;
	//bool	isArg() const;
	//bool	isRet() const;
	
	//bool	IsLocalField() const;//var,regvar,pseudolabel 

	void	setExported(bool b) {// m_nFlags |= FLD_EXPORTED;
		m_nFlags = ChangeBit(m_nFlags, FLD_EXPORTED, b);
	}
	bool	isExported() const { return (m_nFlags & FLD_EXPORTED) != 0; }
	//bool	isClone() const { return (m_nFlags & FLD_CLONED) != 0; }
	//bool	isCloneMaster() const { return (m_nFlags & FLD_MASTER) != 0; }
	//bool	isStaticMember() const { return (m_nFlags & FLD_STATIC) != 0; }
	//bool	isVirtual() const { return (m_nFlags & FLD_VIRTUAL) != 0; }
	bool	isConst() const { return (m_nFlags & FLD_CONST) != 0; }
	bool	isHardNamed() const { return (m_nFlags & FLD_HARDNAMED) != 0; }
	void	setHardNamed(bool b){
#if(1)
		m_nFlags = ChangeBit(m_nFlags, FLD_HARDNAMED, b);
#else
		if (b)
			m_nFlags |= FLD_HARDNAMED;
		else
			m_nFlags &= ~FLD_HARDNAMED;
#endif
	}
	void	setUglyName(){
CHECKID(this, 886)
STOP
		m_nFlags |= FLD_UGLY_NAME; }
 	void	clearUglyName(){ m_nFlags &= ~FLD_UGLY_NAME; }
	bool	hasUglyName() const { return (m_nFlags & FLD_UGLY_NAME) != 0; }
	
	bool	isEos() const;
	void	setEos();

	bool	isTemp() const { return (m_nFlags & FLD_TEMP) != 0; }

	bool	isCollapsed() const { return (m_nFlags & FLD_COLLAPSED) != 0; }
	void	setCollapsed(bool b){
#if(0)
		m_nFlags = ChangeBit(m_nFlags, FLD_COLLAPSED, b);
#else
		if (b)
			m_nFlags |= FLD_COLLAPSED;
		else
			m_nFlags &= ~FLD_COLLAPSED;
#endif
	}

	//FieldPtr	RegisterFetch(int nOffs, TYPE _t &);//, bool);
	//FieldPtr	RegisterFetch(int nOffs, uint8_t);//, bool);

	//static FieldPtr Find(FieldPtr pList, const char *pszName);
	//TypeProc_t	*IsExitLabel();
	//bool	IsCodeLoc();
	const char *	GetDefaultName(int type, uint32_t addr);
	static ObjId_t	IsDefaultName(const char *name, ADDR& addr);

	//int		AdjustFilePos();//on offset change

	
	//int		SetStruc0(TypePtr);

	TypePtr type() const { return mpType; }
	TypePtr typex() const;//skip proxies
	void setType0(TypePtr p){
CHECKID(this, 0x1404)
STOP
		mpType = p;
	}
	//type checking
	TypePtr isPtrToStruc(bool bSkipModifier = true) const;
	TypePtr isPtrToConstStruc(bool bSkipModifier = true) const;
	TypePtr isConstPtrToStruc(bool bSkipModifier = true) const;
	TypePtr isConstPtrToConstStruc(bool bSkipModifier = true) const;
	//TypePtr IsPtrToFunc() const;
	Struc_t *typePtrToStruc();
	TypePtr	isTypeSeg() const;
	TypePtr	isTypeProc() const;
	//TypePtr	isTypeVFTable() const;
	TypePtr	isTypeStruc() const;
	TypePtr	isTypePtr() const;
	TypePtr	isTypeImp() const;
	TypePtr	isTypeExp() const;
	TypePtr	isTypeCode() const;
	TypePtr	isTypeThunk() const;
	TypePtr	isTypeCodeEx() const;//thunks also
	TypePtr	isTypeSimple() const;
	TypePtr	isTypeArray() const;
	TypePtr	isTypeBitset() const;
	bool	isTypeSimple(OpType_t) const;
	bool	isStringOf(OpType_t) const;
	bool	isData() const;//no code or strucvar
	bool	isCombined() const;
	bool	isComplex();
	bool	IsComplexOrArray();
	bool	IsComplex0();
	//int		CheckXRefs();
	//int		CheckXRefStatus();
	FieldPtr 	GetField(int);
	int			MoveUp();//for locals
	int			MoveDown();
	int			CountGotoXRefs();
	FieldPtr	ConvertToFunc(bool bFar);
	int		ExpandStruc();
	int		OnSizeChange(int nSizePr);
	int		CombineWithCodeOp();
	int		SeparateFromCodeOp();
	//int		SetInverted(bool bSet);

	int convertObjType(TypePtr pType );
};



typedef sbtree::config1_s<Field_t>					FieldMapConfig;
typedef sbtree::size_balanced_tree<FieldMapConfig>	FieldMapBase;

class FieldMap : public FieldMapBase
{
public:
	typedef FieldMapBase::iterator Iterator;
	typedef FieldMapBase::const_iterator ConstIterator;
	typedef FieldMapBase::reverse_iterator ReverseIterator;
	typedef FieldMapBase::const_reverse_iterator ConstReverseIterator;

	FieldMap(){}

	iterator Prior(iterator i)
	{
		if (i != end())
			return --i;
		return last();
	}
	const_iterator Priorz(const_iterator i) const
	{
		if (i != end())
			return --i;
		return clast();
	}
	void MoveFrom(FieldMap &o)
	{
		moveFrom(o);
		//(*this) = std::move(o);
	}
};

typedef FieldMap::Iterator			FieldMapIt;
typedef FieldMap::ConstIterator	FieldMapCIt;
typedef FieldMap::ReverseIterator	FieldMapRIt;
typedef FieldMap::ConstReverseIterator	FieldMapCRIt;


bool IsLegalCName(const char *p, unsigned n = 0);//n - max len (if 0) or until eof symbol





