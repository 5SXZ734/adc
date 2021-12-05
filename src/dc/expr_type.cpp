#include "expr_type.h"
#include "prefix.h"
#include "db/types.h"
#include "db/field.h"
#include "type_funcdef.h"
#include "info_dc.h"
#include "expr.h"

using namespace std;


TYP_t::TYP_t(expr::GType *p)
	: mpType(p)
{
}

TYP_t::TYP_t()
	: mpType(nullptr)
{
}

TYP_t::TYP_t(CTypePtr iType, const ExprInfoBase_t *pe)
{
	mpType = pe->typesMgr().fromTypeObj(iType);
}



/*TYP_t::TYP_t(CFieldPtr pField, EXPR_t *pe)
	: mpTypePtr(nullptr)
{
	mpType = pe->typesMgr().fromTypeObj(pField->type());
}

TYP_t::TYP_t(OpPtr pOp, EXPR_t *pe)
	: mpTypePtr(nullptr)
{
	mpType = init(pOp->OpType(), nullptr, 0, pe);
}

TYP_t::TYP_t(uint8_t typ, EXPR_t *pe)
	: mpTypePtr(nullptr)
{
	mpType = init(typ, nullptr, 0, pe);
}*/

//expr::CTypeMgr &TYP_t::typesMgr() const { return mpExpr->typesMgr(); }

/*TYP_t::TYP_t(uint8_t typ, uint32_t nPtr, int nArray)//ptr to a simple
{
	const expr::GType *pType(0);
	assert(OPTYP_IS_PTR(typ));
	if (nPtr)
		pType = typesMgr().fromOpType((OpType_t)nPtr);
	pType = init(typ, pType, nArray);
	mpType = pType;
}

TYP_t::TYP_t(uint8_t typ, TypePtr iStruc, int nArray)//ptr to a struc
{
	const expr::GType *pType(0);
	assert(OPTYP_IS_PTR(typ));
	if (iStruc)
		pType = typesMgr().fromTypeObj(iStruc);
	pType = init(typ, pType, nArray);
	mpType = pType;
}*/

bool TYP_t::isReal() const
{
	if (mpType
		&& mpType->typeSimple()
		&& OPTYP_IS_REAL(mpType->typeSimple()->optype()))
		return true;
	return false;
}

bool TYP_t::isInt() const
{
	if (mpType && mpType->typeSimple())
	{
		if (OPTYP_IS_INT(mpType->typeSimple()->optype()))
			return true;
		if (mpType->typePtr())
			return true;
	}
	return false;
}


TYP_t& TYP_t::operator=(const TYP_t& o)
{
	//?mrTypesMgr = o.mrTypesMgr;
	//assert(mpExpr != nullptr && mpExpr == o.mpExpr);
	//mpExpr = o.mpExpr;
	mpType = o.mpType; return *this;
}

bool TYP_t::operator==(const TYP_t& o) const
{
	///assert(&mrTypesMgr == &o.mrTypesMgr);
	return (mpType == o.mpType);
}

int TYP_t::size() const
{
	if (!mpType)
		return 0;
	return (int)mpType->size();
}

/*void TYP_t::MakeVoid()
{
	const expr::GType *pType(skipArray(mpType));
	int l;
	pType = skipPtrs(pType, l);
	if (l)
	{
		pType = 0;
		while (l--)
			pType = typesMgr().PtrOf(pType);
	}
	mpType = pType;
}*/

uint8_t TYP_t::optyp0() const
{
	if (!mpType)
		return 0;
	assert(mpType->typeSimple());
	uint8_t t(mpType->typeSimple()->optype());
	if (mpType->typePtr())
		return t |= OPTYP_PTR;
	return t;
/*	const expr::GType *pType(skipArray(mpType));
	int l;
	pType = skipPtrs(pType, l);
	if (l)
		return Level2Ptr(l);
	if (pType && pType->typeSimple())
		return pType->typeSimple()->optype();
	return 0;*/
}

/*TypePtr TYP_t::TypeRef() const
{
	const expr::GType *pType(skipArray(mpType));
	pType = skipPtrs(pType);
	if (pType && pType->typeStruc())
		//return typesMgr().toTypeRef(pType);
		return expr().toTypeRef(TYP_t(pType, mpExpr));
	return 0;
}*/

/*uint32_t TYP_t::PtrBaseType() const
{
	const expr::GType *pType(skipArray(mpType));
	int l;
	pType = skipPtrs(pType, l);
	if (l)
	{
		if (pType && pType->typeSimple())
			return pType->typeSimple()->optype();
	}
	return 0;
}

bool TYP_t::IsVoid() const
{
	const expr::GType *pType(skipArray(mpType));
	pType = skipPtrs(pType);
	return (pType == nullptr);
}*/

bool TYP_t::isArray() const
{
	return mpType && (mpType->typeArray() != nullptr);
}

unsigned TYP_t::arraySize() const
{
	if (mpType)
	{
		expr::GTypeArray *pArray(mpType->typeArray());
		if (pArray)
			return pArray->total();
	}
	return 0;
}

bool TYP_t::isPtr() const
{
	const expr::GType* pType(mpType);// skipArray(mpType));
	return pType && pType->typePtr() != nullptr;
}

bool TYP_t::isRef() const
{
	const expr::GType *pType(mpType);// skipArray(mpType));
	return pType && pType->typeRef() != nullptr;
}

bool TYP_t::isTypedef() const
{
	return mpType && mpType->typeTypedef() != nullptr;
}

bool TYP_t::isModifier() const
{
	return mpType && mpType->typeTypedef() != nullptr;//const?
}

TYP_t TYP_t::incumbent() const
{
	return TYP_t(mpType->baseType());
}

TYP_t TYP_t::stripTypedef() const
{
	if (isTypedef())
		return incumbent().stripTypedef();
	return *this;
}

TYP_t TYP_t::stripModifier() const
{
	if (isModifier())
		return incumbent().stripModifier();
	return *this;
}

/*uint8_t TYP_t::PtrLevel() const
{
	int l;
	skipPtrs(mpType, l);
	return l;
}*/

bool TYP_t::isNull() const
{
	return mpType == nullptr;
}

/*const expr::GType *TYP_t::skipArray(const expr::GType *pType) const
{
	if (pType && pType->typeArray())
	{
		pType = pType->typeArray()->baseType();
		assert(pType);
	}
	return pType;
}

const expr::GType *TYP_t::skipPtrs(const expr::GType *pType) const
{
	while (pType && pType->typePtr())
		pType = pType->typePtr()->baseType();
	return pType;
}

const expr::GType *TYP_t::skipPtrs(const expr::GType *pType, int &l) const
{
	l = 0;//ptr level
	for (; pType && pType->typePtr(); pType = pType->typePtr()->baseType())
		l++;
	return pType;
}*/

/*Struc_t	*TYP_t::Struc() const
{
	const expr::GType *pType(skipArray(mpType));
	pType = skipPtrs(pType);
	if (pType && pType->typeStruc())
		return pType->typeStruc();
	return nullptr;
}*/

/*FuncDef_t *TYP_t::FuncDef() const
{
	const expr::GType *pType(skipArray(mpType));
	pType = skipPtrs(pType);
	if (pType && pType->typeFuncDef())
		return pType->typeFuncDef();
	return nullptr;
}*/

bool TYP_t::isSimple() const
{
	return mpType && mpType->typeSimple();
}

bool TYP_t::isComplex() const
{
	return mpType && mpType->typeComplex();
}

bool TYP_t::isFunc() const
{
	return mpType && mpType->typeFunc();
}

bool TYP_t::isCallPtr() const
{
	return mpType && mpType->typeCallPtr();
}

bool TYP_t::isGlob() const
{
	return mpType && mpType->typeGlob();
}

bool TYP_t::isPrivate() const
{
	return mpType && mpType->isPrivate();
}

bool TYP_t::canPromoteTo(const TYP_t &typeTo, bool bMaybeUnk, bool bStrict) const
{
	const TYP_t& typeFrom(*this);
	if (!typeTo)// || !typeFrom)//cast from non void ptr to void ptr is OK?
		return true;
	if (!typeFrom)
		return false;
	if (typeTo == typeFrom)
		return true;

	if (typeFrom.isTypedef())
		return typeFrom.incumbent().canPromoteTo(typeTo, bMaybeUnk, bStrict);
	if (typeTo.isTypedef())
		return typeFrom.canPromoteTo(typeTo.incumbent(), bMaybeUnk, bStrict);

	if (typeTo.isComplex())
	{
		if (typeTo.size() != typeFrom.size())
			return false;
		if (typeFrom.isSimple())
		{
			uint8_t t(typeFrom.optyp0());
			if (!t)
				return false;//unk
			if (!OPTYPE(t))
				return true;
		}
		return false;
	}
	else if (typeFrom.isComplex())
	{
		if (typeTo.size() != typeFrom.size())
			return false;
		if (typeTo.isSimple())
		{
			uint8_t t(typeTo.optyp0());
			if (!t)
				return false;//unk
			if (!OPTYPE(t))
				return true;
		}
		return false;
	}
	if (typeTo.isPtr())
	{
		if (!typeFrom.isPtr())
			return false;
		TYP_t TfromB(typeFrom.ptrBase());
		TYP_t TtoB(typeTo.ptrBase());
		return TfromB.canPromoteTo(TtoB, bMaybeUnk, true);//ptrs must match exactly!
	}
	if (typeTo.isArray() || typeFrom.isArray())
		return false;

	if (!typeFrom.isSimple())
		return false;
	if (!typeTo.isSimple())
		return false;

	//check if 'from' decays to 'to'
	uint8_t from(typeFrom.optyp0());
	uint8_t to(typeTo.optyp0());
	if (bMaybeUnk)
	{
		if (!OPTYPE(from))
			from = MAKETYP_UINT(from);
		if (!OPTYPE(to))
			to = MAKETYP_UINT(to);
	}

	if (from != to)
		if (bStrict)
			return false;

	uint8_t t_from(OPTYPE(from));
	uint8_t t_to(OPTYPE(to));

	if (t_to != t_from)
	{
		if (t_from)
		{
			if (OPTYP_IS_PTR(from))
				return false;
			if (!OPTYP_IS_INT(from))
				return false;
			if (!OPTYP_IS_INT(to))
				return false;
		}
	}
	if (OPSIZE(from) > OPSIZE(to))
	{
		if (from == OPTYP_REAL80 && to == OPTYP_REAL64)
			return true;
		return false;
	}

	return true;
}

TypePtr TYP_t::asTypeRef() const
{
	if (!mpType)
		return nullptr;
	assert(mpType->typeComplex());
	return mpType->typeComplex()->typeObj();
}

GlobPtr TYP_t::asGlobRef() const
{
	if (!mpType)
		return nullptr;
	assert(mpType->typeGlob());
	return mpType->typeGlob()->globref();
}

TYP_t TYP_t::arrayBase() const
{
	assert(mpType && mpType->typeArray());
	return TYP_t(mpType->typeArray()->baseType());
}

TYP_t TYP_t::ptrBase() const
{
	assert(mpType && mpType->typePtr());
	return TYP_t(mpType->typePtr()->baseType());
}

/*TYP_t TYP_t::BaseType() const
{
	if (mpType->typePtr());
	return TYP_t(mpType->typePtr()->baseType());
}*/

MyString CallID2Str(const Out_t &rOut)
{
	char buf[32];
	//int id(rOut.mId);
		ADDR id(rOut.mpOp->ins().VA());//shouldn't be glob_id:op_off???
		//ADDR id(pOut->mpOp->ID());//don't have IDs in opt compile
	assert(id > 0);
	sprintf(buf, "CALL@%X", id);
	//sprintf(buf, "@CALL(%X)", id);
	return buf;
}

MyString GlobID2Str(CGlobPtr g)
{
	char buf[32];
	if (DcInfo_s::DockField(g))
		sprintf(buf, "GLOB@%X", DcInfo_s::DockAddr(g));//bad!
	else
		sprintf(buf, "GLOB$%X", g->ID2());
	return buf;
}

///////////////////////////////////////////////////////
// CTypeMgr

namespace expr {

	GType *CTypeMgr::fromTypeObj(CTypePtr iType0) const
	{
		if (!iType0 || iType0->typeCode() || iType0->typeThunk())
			return nullptr;
		CTypePtr iType(iType0);
		if (iType->typeConst())
			iType = iType->typeConst()->baseType();
		//iType = ProjectInfo_t::SkipModifier(iType);
#if(0)
		if (iType->typeTypedef())
			iType = iType->typeTypedef()->baseType();
#endif
//		assert(!iType->typeImp());
		MakeAlias sAka;
		iType->aka(sAka);
		CBaseCIt i(CBase::find(sAka));
		if (i != end())
			return (GType*)i->second;
		GType *pType(0);
		if (iType->typeArray())
			pType = new GTypeArray(fromTypeObj(iType->typeArray()->baseType()), iType->typeArray()->total());
#if(1)
		else if (iType->typeImp())
			pType = new GTypePtr(mPtrSize, fromTypeObj(nullptr));
#endif
		else if (iType->typeRef())
			pType = new GTypeRef(mPtrSize, fromTypeObj(iType->typeRef()->pointee()));
		else if (iType->typePtr())
			pType = new GTypePtr(mPtrSize, fromTypeObj(iType->typePtr()->pointee()));
		else if (iType->typeComplex())
			pType = new GTypeComplex(iType);
		else if (iType->typeTypedef())
			pType = new GTypeTypedef(iType, fromTypeObj(iType->typeTypedef()->baseType()));
		else if (iType->typeFunc())
		{
			const TypeFunc_t &r(*iType->typeFunc());
			pType = new GTypeFunc(fromTypeObj(r.retVal()), fromTypeObj(r.args()), r.flags());
		}
		else if (iType->typePair())//must be after typeFunc
		{
			const TypePair_t &r(*iType->typePair());
			pType = new GTypePair(fromTypeObj(r.left()), fromTypeObj(r.right()));
		}
		else
		{
			assert(iType->typeSimple());
			pType = new GTypeSimple(iType->typeSimple()->typeID());
		}
		assert(pType);
		if (!const_cast<CTypeMgr *>(this)->insert(make_pair(sAka, pType)).second)
		{
			assert(0);
		}
		pType->setFast(iType);
		return pType;
	}

	GType *CTypeMgr::fromImpField(CFieldPtr pField, CFieldPtr pExpField, bool skipModifier) const
	{
		assert(pExpField);
		TypePtr pType(pField->type());
		if (skipModifier)
			pType = ProjectInfo_t::SkipModifier(pType);
		return fromTypeObj(pType);
	}

	GType *CTypeMgr::fromCall(const Out_t *pOut) const
	{
		assert(pOut->isCallLike());
		MyString s(CallID2Str(*pOut));
		CBaseCIt i(CBase::find(s));
		if (i != end())
			return (GType *)i->second;
		GType *pType(new GTypeCallPtr(mPtrSize, *pOut));
		if (!const_cast<CTypeMgr *>(this)->insert(make_pair(s, pType)).second)
			ASSERT0;
		return pType;
	}

	GType *CTypeMgr::fromGlob(CGlobPtr iGlob) const
	{
		MyString s(GlobID2Str(iGlob));
		CBaseCIt i(CBase::find(s));
		if (i != end())
			return (GType*)i->second;
CHECKID(iGlob, 0x6e0)
STOP
		GType *pType(new GTypeGlob(mPtrSize, iGlob));
		if (!const_cast<CTypeMgr *>(this)->insert(make_pair(s, pType)).second)
		{
			assert(0);
		}
		pType->setFast(iGlob);
		return pType;
	}

	MyString CTypeMgr::alias2(const GType *pType) const
	{
		MakeAlias2 s;
		pType->aka(s);
		return s;
	}

	/*TypePtr CTypeMgr::toTypeRef(const GType *pType) const
	{
#if(1)
		MyString s(alias(pType, ""));
		TypePtr iType(GD C.typeMgr()->findTypeByAlias(s.c_str()));
		if (!iType)
		{
			assert(pType->typeSimple());
			//iType = mrExprCache.typeMgr()->getStockType(pType->typeSimple()->optype());
			iType = GetStockType(pType->typeSimple()->optype());
			assert(iType);
		}
		return iType;
#else
		assert(pType->fast());
		return pType->fast();
#endif
	}*/

	GType *CTypeMgr::fromOpType(OpType_t t) const
	{
		assert(!OPTYP_IS_PTR(t));
		MyString s(TypeID2Str(t));
		CBaseCIt i(CBase::find(s));
		if (i != end())
			return (GType*)i->second;
		GType *pType(new GTypeSimple(t));
//CHECK(s.find(' ') != string::npos)
//STOP
		if (!const_cast<CTypeMgr *>(this)->insert(make_pair(s, pType)).second)
		{
			assert(0);
		}
		return pType;
	}

	GType *CTypeMgr::ptrOf(const GType *pBaseType) const
	{
		MyString s;
		if (pBaseType)
		{
CHECK(pBaseType->fast() && pBaseType->fast()->ID() == 0x6e0)
STOP
//CHECK(!contains(pBaseType))
//STOP
			assert(contains(pBaseType));
			s.append(alias2(pBaseType));//, ""));
		}
		else
		{
			s.append(TypeID2Str(OPTYP_NULL));
		}
		assert(ptrSize() != 0);
		s.append(MyStringf("*%d", ptrSize()));
		CBaseCIt i(CBase::find(s));
		if (i != end())
		{
			assert(i->second != pBaseType);
			return (GType*)i->second;
		}
		GType *pType(new GTypePtr(mPtrSize, pBaseType));
		//CHECK(s.find(' ') != string::npos)
		//STOP
		if (!const_cast<CTypeMgr *>(this)->insert(make_pair(s, pType)).second)
		{
			assert(0);
		}
		assert(pType != pBaseType);
		return pType;
	}

	GType *CTypeMgr::derefOf(const GType *p) const
	{
		if (p && p->typePtr())
			return p->typePtr()->baseType();
		return 0;
	}

	GType *CTypeMgr::arrayOf(const GType *pBaseType, unsigned total) const
	{
		assert(pBaseType);
		assert(contains(pBaseType));

		MakeAlias2 s;
		s.forTypeArray(pBaseType, total);

		//MyString s(alias2(pBaseType));
		//s.append(MyStringf("[%d]", total));

		CBaseCIt i(CBase::find(s));
		if (i != end())
		{
			assert(i->second != pBaseType);
			return (GType *)i->second;
		}
		GType *pType(new GTypeArray(pBaseType, total));
		if (!const_cast<CTypeMgr *>(this)->insert(make_pair(s, pType)).second)
		{
			assert(0);
		}
		assert(pType != pBaseType);
		return pType;
	}

	GType *CTypeMgr::find(const GType *p) const
	{
		MyString s(alias2(p));//, ""));
		CBaseCIt i(CBase::find(s));
		if (i != end())
			return (GType*)i->second;
		return 0;
	}

	bool CTypeMgr::contains(const GType *p) const
	{
		return (find(p) != 0);
	}
}//namespace expr

