
#include "expr.h"
#include "interface/IADCGui.h"
#include "shared/defs.h"
#include "shared/link.h"
#include "shared/misc.h"
#include "shared/front.h"
#include "shared/action.h"
#include "db/mem.h"
#include "db/obj.h"
#include "db/field.h"
#include "db/type_struc.h"
#include "db/info_types.h"
#include "files_ex.h"
#include "clean_ex.h"
#include "path.h"
#include "info_dc.h"
#include "op.h"
#include "ana_ptr.h"
#include "dump.h"
#include "type_funcdef.h"
#include "expr_simpl.h"
#include "expr_term.h"
#include "cc.h"

struct outsv_t
{
	Out_t *p;
	Out_t *outL;
	Out_t *outR;
	outsv_t(Out_t *pOut)
	{
		p = pOut;
		outL = pOut->mpL;
		outR = pOut->mpR;
	}
	Out_t* get()
	{
		if (p->mpL != outL)
		{
			assert(p->mpR == outR);
			return p->mpL;
		}
		if (p->mpR != outR)
		{
			assert(p->mpL == outL);
			return p->mpR;
		}
		//assert(false);
		return nullptr;
	}
};

//////////////////////////////////////////////////

EXPR_t::EXPR_t(const FuncInfo_t &r, HOP pOp, adcui::UDispFlags flags, ExprCache_t &rExpr)
	: ExprInfoBase_t(r, flags, rExpr),
	mpStartOp(pOp)
{
	if (mpStartOp)
	{
		SetNo(OpNo(PrimeOp(mpStartOp)));
		if (IsDeadEx(mpStartOp))
			mbDeadOp = true;
	}
}

/*EXPR_t::EXPR_t(const FuncInfo_t &fi, PathTracer_t &ptr, OpTracer_t &otr, HOP pOp, uint32_t flags, ExprCache_t &rExpr)
	: FuncTracer_t(fi, ptr, otr),
	mpStartOp(pOp),
	m_flags(flags),
	meResult(SIMPL_NULL),
	mNo(-1),
	miStep(0),
#ifdef _DEBUG
	z_step(0),
#endif
	mrExprCache(rExpr)
{
	if (mpStartOp)
		mNo = FuncInfo_t::No(PrimeOp(mpStartOp));
}*/

/*static int Ptr2Level(OpType_t t)
{
	assert(OPTYP_IS_PTR(t));
	return ((t & 0x70) >> 4) + 1;
}*/

static expr::GType * TYPE_init(uint8_t typ, expr::GType *pType, int nArray, const ExprInfoBase_t *pe)
{
	if (OPTYP_IS_PTR(typ))
	{
		//for (int l(Ptr2Level((OpType_t)typ)); l > 0; l--)
			pType = pe->typesMgr().ptrOf(pType);
	}
	else// if (typ)
	{
		assert(!pType);
		pType = pe->typesMgr().fromOpType((OpType_t)typ);
	}
	/*else
	{
		//?assert(pType);
		if (!pType)
			return nullptr;
	}*/
	if (nArray)
		pType = pe->typesMgr().arrayOf(pType, nArray);
	return pType;
}

std::pair<FieldPtr, GlobPtr> ExprInfoBase_t::ExtractCallTarget(const Out_t* pOutCall) const
{
	Out_t* pOutField(nullptr);
	FieldPtr pField(nullptr);
	GlobPtr pGlob(nullptr);

	Out_t* pCallTarget(pOutCall->mpL);
	if (pCallTarget->isFieldKind())
		pOutField = pCallTarget;
	else if (pCallTarget->is(ACTN_INDIR))
		pOutField = pCallTarget->mpR;
	else if (pCallTarget->is(ACTN_PTR))
		pOutField = pCallTarget->mpR;
	else if (pCallTarget->is(ACTN_TYPE))
	{
		pCallTarget = pCallTarget->mpR;
		if (pCallTarget->isFieldKind())
			pOutField = pCallTarget;
		else if (pCallTarget->isOpKind())
		{
			HOP hOp(PrimeOp(pCallTarget->mpOp));
			if (IsCall(hOp) && IsCallIntrinsic(hOp))
				pGlob = dc().getIntrinsic(hOp->m_disp);
		}
		else if (pCallTarget->is(ACTN_PTR))
			pOutField = pCallTarget->mpR;
		else if (pCallTarget->is(ACTN_OFFS))
			pOutField = pCallTarget->mpR;
	}

	if (pOutField && pOutField->isFieldKind())
	{
		pField = pOutField->expField();
		if (!pField)
			pField = pOutField->field();
	}

	if (pField)
	{
		pGlob = IsCFuncOrStub(pField);
		if (!pGlob)
			pGlob = IsPtrToCFunc(pField);
	}

	return std::make_pair(pField, pGlob);
}

TypePtr ExprInfoBase_t::CallRetType(const Out_t* peCall) const
{
	TypePtr pType(nullptr);
	assert(peCall->is(ACTN_CALL));
	std::pair<FieldPtr, GlobPtr> par = ExtractCallTarget(peCall);
	if (par.second)
	{
		GlobPtr pGlob(par.second);//can be an intrinsic
		assert(pGlob && pGlob->func());
		CallingConv_t CC(*this, pGlob);
		for (FuncCCRetsCIt i(*pGlob->typeFuncDef(), CC); !i.isAtEnd(); ++i)
		{
			CFieldPtr pRet(i.field());
			if (pType)
				break;//multiple retvals?
			pType = pRet->type();
		}
	}
	else if (par.first)
	{
		CFieldPtr pFuncPtr(par.first);
		//assert(pFuncPtr->isTypePtr());//!!! cfs2(ex03)
		if (!pFuncPtr->isTypePtr())
			return nullptr;
		CTypePtr pTypeFunc(pFuncPtr->isTypePtr()->absBaseType());
		if (pTypeFunc)
		{
			TypeFunc_t* pTypeFuncPvt(pTypeFunc->typeFunc());
			if (pTypeFuncPvt)
				pType = pTypeFuncPvt->retVal();
		}
	}
	return pType;//no (valid) retval in proto?
}

TYP_t ExprInfoBase_t::TypOf(const Out_t* p) const
{
	switch (p->mAction)
	{
	default:
		break;
	case ACTN_CALL:
	{
		TYP_t T(fromTypeObj(CallRetType(p)));
		if (T.isNull())
			T = fromCall(p);
		return T;
	}
	case ACTN_MOV:
		return TypOf(p->mpL);
	case ACTN_GET:
		return TypOf(p->mpL);
	case ACTN_CHECK:
		return TypOf(p->mpL);
	case ACTN_UNITE:
	{
		TYP_t tR(TypOf(p->mpR));
		TYP_t tL(TypOf(p->mpL));
		uint8_t t = tR.size() + tL.size();
		if (tR.isPtr() && (tR.size() == mrDC.PtrSize()) && (t == mrDC.PtrSizeEx()))//near
			return tR;
		return fromOpType(t);
	}
	case ACTN_DOT:
	case ACTN_PTR:
		assert(p->mpR->isFieldKind());
		return fromField(p->mpR->field());
	case ACTN_OFFS:
		return PtrOf(p->mpR);
#if(1)
	case ACTN_INDIR:
		return DerefOf(p->mpR);
#endif
	case ACTN_CHS:
		return TypOf(p->mpR);
	case ACTN_ADD:
	case ACTN_SUB:
	{
		TYP_t t(TypOf(p->mpL));
		if (t.isPtr() && TypOf(p->mpR).isPtr())//ptr diff?
			t = fromOpType(MAKETYP_SINT(PtrSize()));
		return t;
	}
	case ACTN_MUL:
	{
		TYP_t tL(TypOf(p->mpL));
		TYP_t tR(TypOf(p->mpR));
		if (tL.isReal() && tR.isReal())
		{
			return tL.size() > tR.size() ? tL : tR;
		}
	}
	case ACTN_VAR:
		return TypOf(p->mpR);
	}
	return p->typ();
}

TYP_t Out_t::typx() const
{
	/*switch (mAction)
	{
	default:
		break;
	case ACTN_ADD:
		return mpL->typx();
	case ACTN_SUB:
	{
		TYP_t t(mpL->typx());
		//if (t.isPtr() && TypOf(p->mpR).isPtr())
			//t = fromOpType(OPTYP_INT32);
		return t;
	}
	}*/
	return typ();
}

expr::GType* ExprInfoBase_t::FromTyp(Out_t* p) const
{
	return TypOf(p);
}

expr::GType *ExprInfoBase_t::PtrOf(expr::GType *p) const
{
	//if (T.typePtr())
		//return TYP_t(T.typePtr());
//	if (T.IsCall())
	//	return PtrOf(nullptr);
	expr::GType *pTypePtr(typesMgr().ptrOf(p));
	//T.setTypePtr(pTypePtr);
	return pTypePtr;
}

expr::GType *ExprInfoBase_t::DerefOf(expr::GType *p) const
{
	return typesMgr().derefOf(p);
}

static uint8_t toProxyType(uint8_t t)
{
	switch (t)
	{
	case OPTYP_NULL:
		return OPTYP_UINT8;
	case OPSZ_BYTE:
	case OPTYP_BYTE: return OPTYP_UINT8;
	case OPSZ_WORD:
	case OPTYP_WORD: return OPTYP_UINT16;
	case OPSZ_DWORD:
	case OPTYP_DWORD: return OPTYP_UINT32;
	case OPSZ_QWORD:
	case OPTYP_QWORD: return OPTYP_UINT64;
	}
	return t;
}

expr::GType *ExprInfoBase_t::ProxyOf(const TYP_t &T0) const
{
#if(1)
	if (T0.isNull())
		return fromOpType(toProxyType(0));
	if (T0.isArray())
	{
		expr::GType *pT(ProxyOf(T0.arrayBase()));
		return typesMgr().arrayOf(pT, T0.arraySize());
	}
	if (T0.isCallPtr())
	{
		return T0;
	}
	if (T0.isPtr())
	{
		TYP_t Tb(T0.ptrBase());
		if (Tb.isNull())
			return T0;//ptr to void has no proxy
		expr::GType *pT(ProxyOf(Tb));
		return typesMgr().ptrOf(pT);
	}
	if (T0.isSimple())
	{
		return fromOpType(toProxyType(T0.optyp0()));
	}
	return T0;
#else
	const expr::GType *pType(T0.skipArray(T0.type()));
	int l;//ptr level
	pType = T0.skipPtrs(pType, l);
	if (!pType)
	{
		return TYP_t(toProxyType(0), this);
	}
	if (pType->typeSimple())
	{
		uint8_t t(pType->typeSimple()->optype());
		uint8_t s(toProxyType(t));
		if (s != t)
		{
			TYP_t T2(s, this);
			pType = T2.type();
		}
	}
	while (l-- > 0)
		pType = typesMgr().ptrOf(pType);
	if (pType->typeArray())
		pType = typesMgr().arrayOf(pType, T0.type()->typeArray()->total());

	return TYP_t(pType, this);
#endif
}

//IsReconcilableWith
#if(1)
bool ExprInfoBase_t::AreCompliant(const TYP_t &T0, const TYP_t &T) const//this is L-action
{
	if (T0.isNull())
		return true;//right can be null too
	if (T.isNull())
		return true;//left isn't null

	if (T0.isTypedef())
		return AreCompliant(T0.incumbent(), T);
	if (T.isTypedef())
		return AreCompliant(T0, T.incumbent());

	if (T0.isArray())
	{
		if (!T.isArray())
			return false;//one array other is not
		if (T0.arraySize() != T.arraySize())
			return false;
		return AreCompliant(T0.arrayBase(), T.arrayBase());
	}
	else if (T.isArray())
		return false;

	if (T0.isComplex())
	{
		if (!T.isComplex())
			return false;

		if (T0.asTypeRef() != T.asTypeRef())
			return false;
		return true;
	}
	else if (T.isComplex())
		return false;

	if (T0.isFunc() || T.isFunc())
		return false;

	if (T0.isCallPtr())
	{
		if (!T.isCallPtr())
			return false;
		return T0.type() == T.type();
	}
	else
	{
		if (T.isCallPtr())
			return false;
	}
	 

/*	if (T0.TypeRef() && !T0.isPtr())
		if (!T.TypeRef())
			return false;//left is struc, right is not
*/
	if (T0.isPtr() && T.isPtr())
	{
		TYP_t T1(DerefOf(T0)), T2(DerefOf(T));
		return AreCompliant(T1, T2);
	}

	uint8_t t1 = T0.optyp0();
	uint8_t t2 = T.optyp0();
	if (!AgreeTypes(t1, t2))
		return false;

#if(0)
	if ((T0.PtrBaseType() || T0.TypeRef()) && (T.PtrBaseType() || T.TypeRef()))
	{
		if (T0.TypeRef() && !T0.isPtr())
		{
			if (!T.TypeRef())
				return false;//one is struc, other is not
			if (T0.TypeRef() == T.TypeRef())
				return true;
			if (T0.FuncDef())
			{
				Func Info_t an(*T0.FuncDef()->owner Func());
				if (an.IsEqualTo(T.FuncDef()))
					return true;
			}

			return false;
		}
		else if (T.TypeRef())
			return false;


		//		if (m_nPtr != T.m_nPtr)
		//			return false;
		t1 = (uint8_t)T0.PtrBaseType();
		t2 = (uint8_t)T.PtrBaseType();
		if (!AgreeTypes(t1, t2))
			return false;
	}
#endif

	return true;
}
#else
TYPE_t ExprInfoBase_t::AreCompliant(const TYPE_t &T0, const TYPE_t &rType2)//this is L-action
{
	const TYPE_t &rType1(*this);
	const TYPE_t nul;

	if (rType1.IsNull())
		return rType2;//right can be null too
	if (rType2.IsNull())
		return nul;//left isn't null

	bool bComplex1(rType1.isComplex() != 0);
	bool bComplex2(rType2.isComplex() != 0);

	if (bComplex1 ^ bComplex2)
		return nul;//one simple, other - complex

	if (bComplex1)
	{
		if (rType1 == rType2)
			return rType1;
		return nul;
	}

	//?	if (mpStruc)
	//?	if (!T.mpStruc)
	//?	return false;//left is struc, right is not

	if (!(rType1.isSimple() && rType2.isSimple()))
		return nul;

	uint8_t t1 = rType1.optyp0();
	uint8_t t2 = rType2.optyp0();
	if (!AgreeTypes(t1, t2))
		return nul;

	if (rType1.isPtr())
	{
		if (rType2.isPtr())
		{
			TYPE_t rType(AreCompliant(DerefOf(rType1), DerefOf(rType2)));
			if (rType.IsNull())
				return nul;
			return rType.ptr();
		}
		else
		{
			TYPE_t rType(AreCompliant(DerefOf(rType1), nul));
			if (rType.IsNull())
				return nul;
			return rType.ptr();
		}
	}
	else if (rType2.isPtr())
	{
		TYPE_t rType(AreCompliant(nul, DerefOf(rType2)));
		if (rType.IsNull())
			return nul;
		return rType.ptr();
	}


	/*?	if ((mnPtr || mpStruc) && (T.mnPtr || mpStruc))
	{
	if (mpStruc)
	{
	if (!T.mpStruc)
	return false;//one is struc, other is not
	if (mpStruc == T.mpStruc)
	return true;
	if (FuncDef())
	{
	Func Info_t an(*FuncDef()->owner Func());
	if (an.IsEqualTo(T.FuncDef()))
	return true;
	}

	return false;
	}
	else if (T.mpStruc)
	return false;


	//		if (m_nPtr != T.m_nPtr)
	//			return false;
	t1 = (uint8_t)mnPtr;
	t2 = (uint8_t)T.mnPtr;
	if (!AgreeTypes(t1, t2))
	return false;
	}*/

	return TYPE_t((OpType_t)t1);//?
}
#endif

#if(1)
TYP_t ExprInfoBase_t::Complied0(const TYP_t &T0, const TYP_t &T) const
{
	if (T0.isPtr() && T.isPtr())
	{
		TYP_t _T0(DerefOf(T0));
		TYP_t _T(DerefOf(T));
		TYP_t T(Complied0(_T0, _T));// DerefOf(T0), DerefOf(T)));	<= WHY CRASH ON UBUNTU (flanker:ex00) ???
		return PtrOf(T);
	}

	if (T0.isNull())
		return T;
	if (T.isNull())
		return T0;

	if (!T0.isSimple() || !T.isSimple())
		return T0;

	uint8_t t1(T0.optyp0());
	uint8_t t2(T.optyp0());
	if (!AgreeTypes(t1, t2))
		return T0;

	if (!t1)
		return T;

	if (!T0.isPtr())
	{
		if (T.isPtr())
			return T;
		/*TYP_t T2(T.PtrBaseType(), this);
		for (int l(Ptr2Level((OpType_t)t1)); l > 0; l--)
		T2 = T2.ptr();
		return T2;*/
	}
	else
		return T0;

	/*if (!T0.TypeRef() && T.TypeRef())
	{
		TYP_t T2(T.TypeRef(), this);
		for (int l(Ptr2Level((OpType_t)t1)); l > 0; l--)
			T2 = T2.ptr();
		return T2;
	}

	if (OPTYP_IS_PTR(t1))
	{
		TYP_t T2(this);//?
		for (int l(Ptr2Level((OpType_t)t1)); l > 0; l--)
			T2 = T2.ptr();
		return T2;
	}*/

	return TYP_t(fromOpType(t1));
	// m_nArray);

	/*if (IsVoid())
	{
	mnPtr = T.mnPtr;
	mpStruc = T.mpStruc;
	}
	else if (T.IsVoid())
	{
	T.mnPtr = mnPtr;
	T.mpStruc = mpStruc;
	}*/
}
#else

TYP_t EXPR_t::Complied0(const TYP_t &T0, const TYP_t &rOther)
{
	const TYP_t &rSelf(*this);

	if (!rSelf)
		return rOther;

	if (rSelf == rOther)
		return rOther;

	if (rSelf.isPtr() && rOther.isPtr())
	{
		TYP_t rType(Complied0(DerefOf(rSelf), DerefOf(rOther)));
		if (rType == DerefOf(rSelf))
			return rSelf;//nothing changed
		return rType.ptr();
	}

	/*?if (rSelf.mpType && pOther)
	{
	if (rSelf.mpType->typeStruc() && pOther->typeSimple())
	{
	int offs(0);
	FieldPtr pField = rSelf.mpType->typeStruc()->GetFieldEx(offs);
	if (!pField)
	{
	StrucModifier_t an(mrFunc);
	pField = an.AddF ield(rSelf.mpType->typeStruc()->type obj(), offs, 2, 0);
	pField->setT ype(mTypesMgr.toType(pOther));
	}
	}
	}*/

	if (rSelf.isSimple() && rOther && rOther.isSimple())
	{
		uint8_t t1(rSelf.optype());
		uint8_t t2(rOther.optype());
		uint8_t t3(t1);
		AgreeTypes(t1, t2);
		if (t1 == t3)
			return rSelf;
		if (rOther.isPtr())
			return rOther;
		return TYP_t((OpType_t)t1);
	}

	/*?if (IsVoid())
	{
	mnPtr = T.mnPtr;
	mpStruc = T.mpStruc;
	}
	else if (T.IsVoid())
	{
	T.mnPtr = mnPtr;
	T.mpStruc = mpStruc;
	}

	assert(0);*/
	//TypOf(pSelf->mpR).Comply With(T);

	return rSelf;
}
#endif

TYP_t ExprInfoBase_t::Complied(const TYP_t &T0, const TYP_t &T) const
{
	if (T0 == T)
		return T0;

	if (T0.isPtr() && T.isPtr())
	{
		TYP_t T(Complied0(DerefOf(T0), DerefOf(T)));
		return PtrOf(T);
	}

	if (T0.isNull())
		return T;
	if (T.isNull())
		return T0;

	if (!T0.isSimple())
	{
		if (!T.isSimple())
			return TYP_t();
		uint8_t t(T.optyp0());
		if (OPTYPE(t))
			return TYP_t();
		return T0;
	}

	if (!T.isSimple())
	{
		uint8_t t(T0.optyp0());
		if (OPTYPE(t))
			return TYP_t();
		return T;
	}

	uint8_t t1(T0.optyp0());
	uint8_t t2(T.optyp0());
	if (!AgreeTypes(t1, t2))
		return TYP_t();

	if (!t1)
		return T;

	if (!T0.isPtr())
	{
		if (T.isPtr())
			return T;
	}
	else
		return T0;

	return TYP_t(fromOpType(t1));
}

TypePtr ExprInfoBase_t::toTypeRef(const TYP_t &rSelf) const
{
	if (rSelf.isNull())
		return nullptr;

	if (rSelf.isArray())
	{
		TypePtr iType(toTypeRef(rSelf.arrayBase()));
		TypesTracer_t tt(*this, memMgrGlob(), *findTypeMgr(iType));
		return tt.arrayOf(iType, rSelf.arraySize());
	}
	if (rSelf.isPtr())
	{
		TypePtr iType(toTypeRef(rSelf.ptrBase()));
		if (iType && !iType->isShared())
			iType = nullptr;
		if (iType)
		{
			TypesMgr_t *pTypeMgr(findTypeMgr(iType));
			if (pTypeMgr)
			{
				TypesTracer_t tt(*this, memMgrGlob(), *pTypeMgr);
				return tt.ptrOf(iType, PtrSizeOf(*pTypeMgr));
			}
		}
		TypesTracer_t tt(*this, memMgrGlob(), *PrimeSeg()->typeMgr());
		return tt.ptrOf(iType, PtrSizeOf(PrimeSeg()));
	}
	if (rSelf.isSimple())
	{
		return GetStockType((OpType_t)rSelf.optyp0());
	}
	if (!rSelf.isComplex())
		return nullptr;
	return rSelf.asTypeRef();
}

expr::GType *ExprInfoBase_t::fromGlob(CGlobPtr iGlob) const
{
	return typesMgr().fromGlob(iGlob);
}

expr::GType *ExprInfoBase_t::fromTypeObj(CTypePtr iType) const
{
	return typesMgr().fromTypeObj(iType);
}

expr::GType *ExprInfoBase_t::fromField(CFieldPtr pField, bool skipModifier) const
{
#if(1)
	if (!pField)
		return nullptr;
	if (pField->isTypeImp())
	{
		CFieldPtr pExpField(ToExportedField(pField));
		if (!pExpField)
			return nullptr;
		return typesMgr().fromImpField(pField, pExpField);
	}
#endif
	TypePtr pType(pField->type());
	if (skipModifier)
		pType = ProjectInfo_t::SkipModifier(pType);
	if (pType && pType->typeProc())
		return typesMgr().ptrOf(nullptr);
	return typesMgr().fromTypeObj(pType);
}

expr::GType *ExprInfoBase_t::fromOp(CHOP pOp) const
{
	uint8_t t(pOp->OpType());
	if (pOp->OpC() == OPC_CPUSW)
	{
		assert(pOp->OpSize() < 8);//bits in bytes 
		t = OPSZ_BYTE;
	}
	return TYPE_init(t, nullptr, 0, this);
}

expr::GType *ExprInfoBase_t::fromOpType(uint8_t t) const
{
	return TYPE_init(t, nullptr, 0, this);
}

expr::GType *ExprInfoBase_t::fromCall(const Out_t *pOut) const
{
	//COMP_ID globId(mrFuncDef.ID());
	return typesMgr().fromCall(pOut);
}

int ExprInfoBase_t::ComplyWithFieldType(Out_t *pSelf, FieldPtr pField) const
{
	TYP_t Tself(TypOf(pSelf));
	//?assert(Tself.isPtr());
	if (pField->isTypeImp() || pField->isTypeExp())
		return 1;
	TYP_t T(fromField(pField));
	if (Tself == T)
		return 1;
	Tself = Complied0(Tself, T);
	pSelf->setTyp(Tself);
	if (!AreCompliant(Tself, T))
		return 0;
	//if (Tself.TypeRef() && !Tself.TypeRef()->isShared())
	if (Tself.isPrivate())
		return 0;

	CTypePtr iType(toTypeRef(Tself));
	if (iType != pField->type())
	{
		DcCleaner_t<> DC(*this);
		DC.ClearType(pField);
		SetType(pField, (TypePtr)iType);
	}
	return 1;
}

/*uint32_t EXPR_t::testf(uint32_t f, bool f2)
{
	if ( f2 )
		return m_flags2 & f;
	return m_flags & f; 
}*/

bool ExprInfoBase_t::IsUnfoldMode() const
{
	return m_flags.testL(adcui::DUMP_UNFOLD);
}

int Out_t::sID = 0;
Out_t *ExprInfoBase_t::NewTerm() const
{
	Out_t *pOut(mrExprCache.mOuts.New<EXPR_t *>((EXPR_t *)this));
#ifdef _DEBUG
	pOut->mId = ++Out_t::sID;
CHECK(pOut->mId == 0x3236)
STOP
#endif
	return pOut;
}

Out_t *ExprInfoBase_t::Clone(Out_t &rSelf, Out_t *pU) const
{
	Out_t *pOut(NewTerm());
	*pOut = rSelf;

	if (pU)
		Add(*pU, *pOut);

	if (rSelf.mpL)
		Clone(*rSelf.mpL, pOut);

	if (rSelf.mpR)
		Clone(*rSelf.mpR, pOut);

	return pOut;
}

Out_t * EXPR_t::AddRoot() const
{
	Out_t *pOut(NewTerm());
	return pOut;
}

Out_t *ExprInfoBase_t::Add(Out_t &rSelf) const
{
	Out_t *pOut(NewTerm());
	rSelf.AddChild(pOut);
	return pOut;
}

Out_t *ExprInfoBase_t::Add(Out_t &rSelf, CHOP hDockOp, Action_t _action) const
{
	Out_t *pOut(NewTerm());
	pOut->mAction = _action;
	SetDockOp(pOut, hDockOp);
	rSelf.AddChild(pOut);
	return pOut;
}

Out_t *ExprInfoBase_t::InsertParent(Out_t *pSelf, Action_t _action, bool bDockOpFromParent) const
{
	Out_t *pOut(NewTerm());
	pOut->mAction = _action;
	if (pSelf->mpU->mpL == pSelf)
		pSelf->mpU->mpL = pOut;
	else
		pSelf->mpU->mpR = pOut;
	pOut->mpU = pSelf->mpU;
	pOut->AddChild(pSelf);

	if (!bDockOpFromParent)
		SetDockOp(pOut, pSelf->dockOp());
	else//from self
		SetDockOp(pOut, pOut->mpU->dockOp());

	return pOut;
}

Out_t* ExprInfoBase_t::InsertParent(Out_t* pSelf, const TYP_t &T, bool bDockOpFromParent) const
{
	Out_t* peOut(InsertParent(pSelf, ACTN_TYPE, bDockOpFromParent));
	peOut->setTyp(T);
	return peOut;
}

Out_t *ExprInfoBase_t::InsertParent(Out_t *pSelf, Out_t *pParent) const
{
	Out_t *pOut(pParent);
	if (pSelf->mpU->mpL == pSelf)
		pSelf->mpU->mpL = pOut;
	else
		pSelf->mpU->mpR = pOut;
	SetDockOp(pOut, pSelf->dockOp());
	pOut->mpU = pSelf->mpU;
	pOut->AddChild(pSelf);
	return pOut;
}

void ExprInfoBase_t::SetDockOp(Out_t *pOut, CHOP pOp) const
{
	if (!pOp)
	{
		pOut->mpDockOp = HOP();
		return;
	}
	pOut->mpDockOp = PrimeOp(pOp);
//CHECKID(pOp, 0xd8d)
//STOP
}

Out_t *ExprInfoBase_t::Detach(Out_t* pSelf) const
{
	if (!pSelf->mpU)
		return nullptr;

	if (pSelf->isLeft())
		pSelf->mpU->mpL = nullptr;
	else if (pSelf->isRight())
		pSelf->mpU->mpR = nullptr;

	return pSelf->mpU;//return parent
}

Out_t *ExprInfoBase_t::DetachParent(Out_t *pSelf) const
{
	assert(pSelf->mpU);
	Out_t *pParent = pSelf->mpU;

	Out_t *pOutUU = pSelf->mpU->mpU;
	if (pOutUU)
	{
		if (pOutUU->mpL == pSelf->mpU)
			pOutUU->mpL = pSelf;
		else
			pOutUU->mpR = pSelf;

		if (pSelf->mpU->mpL == pSelf)
			pSelf->mpU->mpL = nullptr;
		else
			pSelf->mpU->mpR = nullptr;
	}
	pSelf->mpU->mpU = nullptr;
	//if (!pSelf->is(ACTN_CALL))
//?		SetDockOp(pSelf, pParent->dockOp());
	
	pSelf->mpU = pOutUU;
	return pParent;
}

Out_t *ExprInfoBase_t::Add(Out_t *pSelf, CHOP pOp) const
{
	//HOP pOp(hOp);

//	assert(!pOp->IsField());
//	assert(!pOp->IsDataOp() || pOp->IsArgOp());
	bool bCallIntrinsic(IsPrimeOp(pOp) && IsCall(pOp) && IsCallIntrinsic(pOp));

	Out_t *pOut(NewTerm());
	if (!bCallIntrinsic)
		pOut->mSsid = pOp->OpC();//StorageId();
	bool bIsAddr(pOut->mSsid == (OPC_ADDRESS|OPC_GLOBAL));

	switch(pOp->OpC())
	{
	case OPC_CPUSW:
	case OPC_FLAGS|OPC_CPUSW:
		pOut->m_eflags = CPUFlags(pOp);
		break;
	
	case OPC_FLAGS|OPC_FPUSW:
	case OPC_FPUSW:
		pOut->m_eflags = FPUFlags(pOp);
		break;

	default:
		if (pOp->IsScalar() || bIsAddr)
		{
			pOut->mOffs = 0;
		}
		else 
		{
			if (pOp->IsIndirectB())
			{
				if ((pOp->OpC() & 0xF) == OPC_CPUREG)//fix 4 x64
					pOut->mOffs = (uint8_t)pOp->OpOffs();
				else
					pOut->mOffs = pOp->OpOffs();
				pOut->mDisp = pOp->m_disp;
				pOut->m_seg = pOp->OpSeg();
			}
			else if (pOp->IsIndirect())
			{
				pOut->mOffs = pOp->Offset0();
				pOut->mDisp = 0;
				pOut->m_seg = pOp->OpSeg();
			}
			else if (bCallIntrinsic)
				;//pOut->mOffs = pOp->Offset0();
			else
				pOut->mOffs = pOp->Offset0();
		}
	}

//	pOut->m_seg = pOp->OpSeg();

	if (pOp->IsScalar() || bIsAddr)
	{
		pOut->m_value.i64 = OpDisp(pOp);
		if (bIsAddr)
			pOut->m_value.i64 += ImageBase();
		if (pOp->OpOffs() != 0)//special constants (see OPCONST_e)
		{
			assert(pOp->OpType() == OPTYP_REAL64);
			if (pOp->OpOffsU() == OPID_0R64)
				pOut->m_value.r64 = 0.0;
			else if (pOp->OpOffsU() == OPID_1R64)
				pOut->m_value.r64 = 1.0;
		}
		pOut->mKind = OUT_IMM;
	}
	else
	{
//#		pOut->m_pOp = AddOp(pOp);
		pOut->mKind = OUT_OP;
	}

	FieldPtr pFieldRef(nullptr);
	if (bCallIntrinsic)
	{
		GlobPtr iGlob(dc().getIntrinsic(pOp->m_disp));
		assert(iGlob);
		pOut->setTyp(fromGlob(iGlob));
	}
	else
	{
		pFieldRef = LocalRef(pOp);
#if(0)
		pFieldRef = FindFieldRef(pOp);

		if (pFieldRef && pFieldRef->isProcType())
		{
			GlobPtr ifDef(IsCFuncOrStub(pFieldRef));
			if (!ifDef)
			{
				//convert to imm
				pOut->mKind = OUT_IMM;
				pOut->m_value.i64 = OpDisp(pOp);
			}
		}
#endif

#if(DUMP_OP_FIELD_ATTACH)
		if (pFieldRef)
		{
			TypePtr pType(pFieldRef->type());
			if (pType && pType->typeCode())
			{
				TypePtr iFunc(pFieldRef->owner());
				if (iFunc->type Proc())
					if (pFieldRef->address() == iFunc->type Proc()->base())
						pFieldRef = iFunc->parentField();
			}
			//		assert(pOp->OpSize() == pOp->fieldRef()->OpSize());
			TYP_ Set(*pOut, pFieldRef);
		}
		else
#endif
			pOut->setTyp(fromOp(pOp));
	}

	if (!pSelf->AddChild(pOut))
	{
		Dispose(pOut);
		return pSelf;
	}

	pOut->setXIn(pOp->XIn());
	pOut->setXOut(pOp->XOut());
	pOut->m_bExtraIn = (pOp->m_nFlags & OPND_XINEX) != 0;
	pOut->m_bExtraOut = (pOp->m_nFlags & OPND_XOUTEX) != 0;
	if (IsPrimeOp(pOp) && ISCALL(ActionOf(pOp)))
		pOut->setXOut(nullptr);

	pOut->mpOp = (HOP)pOp;
	pOut->setFieldRef(pFieldRef);
	SetDockOp(pOut, pOp);
	return pOut;
}

#if(NEW_OP_PTR)
class TMPHOP : public HOP
{
	Op_t	m_op;
public:
	TMPHOP() : HOP(0, &m_op){}
	~TMPHOP(){ m_op.setInsPtr(nullptr); }
};
#else
class TMPHOP : public Op_t
{
	
public:
	TMPHOP() : Op_t(/*OBJ_ID_ZERO*/){}
	~TMPHOP(){ setInsPtr(nullptr); }
	Op_t* operator->(){ return this; }
	operator Op_t*(){ return this; }
};
#endif

Out_t *ExprInfoBase_t::AddB(Out_t *pSelf, CHOP pOp) const
{
	TMPHOP op;
	op->setInsPtr(PrimeOp(pOp)->insPtr());
	op->Setup3(pOp->OpC() & 0xF, mrDC.PtrSize(), pOp->OpOffsU());
	Out_t *pOut = Add(pSelf, op);
	pOut->setXIn(pOp->m_xin);//m_pXIn);
	pOut->m_bExtraIn = (pOp->m_nFlags & OPND_XINEX) != 0;
	pOut->mpOp = HOP();
	SetDockOp(pOut, pOp);
	return pOut;
}

Out_t *ExprInfoBase_t::Add(Out_t &rSelf, CHOP pOp, int _i) const
{
	Out_t *pOut(NewTerm());
//	pOut->m_pOp = AddOp(_i);
	pOut->mKind = OUT_IMM;
	SetDockOp(pOut, pOp);
	pOut->setTyp(fromOpType(MAKETYP_SINT(mrDC.PtrSize())));
	rSelf.AddChild(pOut);
	pOut->m_value.i32 = _i;
	//assert(!pOut->mpOp);
	return pOut;
}

Out_t *ExprInfoBase_t::Add(Out_t &rSelf, CHOP pOp, VALUE_t v) const
{
	Out_t *pOut(NewTerm());
	pOut->mKind = OUT_IMM;
	SetDockOp(pOut, pOp);
	pOut->setTyp(fromOpType(v.typ));
	rSelf.AddChild(pOut);
	pOut->m_value.ui64 = v.ui64;
	return pOut;
}

Out_t *ExprInfoBase_t::Add(Out_t *pSelf, CHOP pOp, uint8_t cl, uint8_t typ, uint8_t id) const
{
	TMPHOP op;
	op->Setup3(cl, typ, id);
	op->setInsPtr(pOp ? PrimeOp(pOp)->insPtr() : nullptr);
	Out_t *pOut = Add(pSelf, op);
	pOut->mpOp = HOP();
	SetDockOp(pOut, pOp);
	return pOut;
}

Out_t *ExprInfoBase_t::Add2(Out_t *pSelf, CHOP pOp, uint8_t cl, uint8_t off, uint8_t siz) const
{
	TMPHOP op;
	op->Setup3(cl, siz, off);
	op->setInsPtr(pOp ? PrimeOp(pOp)->insPtr() : nullptr);
	Out_t *pOut = Add(pSelf, op);
	pOut->mpOp = HOP();
	SetDockOp(pOut, pOp);
	return pOut;
}

Out_t *ExprInfoBase_t::Add(Out_t &rSelf, FieldPtr _pField, CHOP pDockOp) const
{
	Out_t *pOut(NewTerm());
	pOut->setField(_pField);
	assert(_pField);
	pOut->mKind = OUT_FIELD;
	rSelf.AddChild(pOut);

	if (!pDockOp)
		SetDockOp(pOut, rSelf.dockOp());
	else
		SetDockOp(pOut, pDockOp);
	return pOut;
}

Out_t *ExprInfoBase_t::Add(Out_t &rSelf, CHOP pOp, const TYP_t &rType) const
{
	Out_t *pOut(NewTerm());
	pOut->mAction = ACTN_TYPE;
	SetDockOp(pOut, pOp);
	pOut->setTyp(rType);
	//	pOut->mKind = OUT_TYPE;
	rSelf.AddChild(pOut);
	return pOut;
}

Out_t *ExprInfoBase_t::Add(Out_t &rSelf, Out_t &rOut, bool bCopy) const
{
	if (bCopy)
	{
		Out_t *pOutN(NewTerm());
		*pOutN = rOut;
		rSelf.AddChild(pOutN);
		return pOutN;
	}

	rSelf.AddChild(&rOut);
	return &rOut;
}

/*Out_t *ExprInfoBase_t::Add(Out_t &rSelf, CHOP pOp, HPATH rPath) const
{
	Out_t *pOut(NewTerm());
	pOut->mpPath = rPath;
	pOut->mKind = OUT_PATH;
	SetDockOp(pOut, pOp);
	rSelf.AddChild(pOut);
	return pOut;
}*/

int ExprInfoBase_t::GetSwitchInfo(Out_t * pSelf, SwitchQuery_t &si) const
{
	if (!pSelf->is(ACTN_GOTO))
		return 0;

	Out_t *pTop = pSelf;
	pTop = pTop->mpL;
//	pTop->DetachParent();//ACTN_GOTO
	if (!pTop->is(ACTN_ARRAY))
		return 0;

	if (!pTop->mpL->isFieldKind())
		return 0;

//		HOP pJumpTableOp = pTop->mpL->m_pOp;
	FieldPtr pJumpTable;
	//		if (pJumpTableOp->IsDataOp())
	//			pJumpTable = pJumpTableOp->GetOwnerData();
	//		else
	//			pJumpTable = pJumpTableOp->m_pData;
	pJumpTable = (FieldPtr )pTop->mpL->field();// getMLoc();
	assert(pJumpTable);
	{
		si.pJumpTable = pJumpTable;
		pTop = pTop->mpR;
		//			pTop->DetachParent();//remove jump table
		ExtractDisplacement2(pTop, si.nIndexOffset1);

		while (pTop->is(ACTN_INDIR)
			&& pTop->mpR->is(ACTN_TYPE)
			&& pTop->mpR->mpR->is(ACTN_OFFS))
			pTop = pTop->mpR->mpR->mpR;

		while (pTop->is(ACTN_TYPE) || pTop->is(ACTN_ZEROEXT))
			pTop = pTop->mpR;

		if (pTop->is(ACTN_ARRAY))//check for index table
		{
			assert(pTop->mpL->isFieldKind() || pTop->mpL->isNumKind());
//				HOP pIndexTableOp = pTop->mpL->m_pOp;
			FieldPtr pIndexTable;
			//				if (pIndexTableOp->IsDataOp())
			//					pIndexTable = pIndexTableOp->GetOwnerData();
			//				else
			//					pIndexTable = pIndexTableOp->m_pData;
			pIndexTable = (FieldPtr )pTop->mpL->field();// getMLoc();
			assert(pIndexTable);
			{
				si.pIndexTable = pIndexTable;
				pTop = pTop->mpR;
				//					pTop->DetachParent();//remove index table
				ExtractDisplacement2(pTop, si.nIndexOffset2);
			}
		}
		else if (pTop->is(ACTN_INDIR))//unrecognized index table (due to wrong offseting?)
		{
			do { pTop = pTop->mpR; } while (pTop->is(ACTN_TYPE));//skip type casts
			if (pTop->isActionAddOrSub())
				if (pTop->mpL->is(ACTN_OFFS))
				{
					si.pOutWrongTable = pTop->mpL->mpR;
					assert(si.pOutWrongTable->field());
					si.pOutWrongOffset = CheckDisplacement(pTop);
				}
		}
	}

	return 1;
}

////////////////////////////////////////////////////////////

Out_t * EXPR_t::DumpB(CHOP pSelf, Out_t *mpU) const
{
	assert(pSelf->IsIndirect());

	uint8_t opc = pSelf->m_opc;
	uint8_t optyp = pSelf->m_optyp;
	int8_t opid = pSelf->mOffs;
	int disp = pSelf->m_disp;
	FieldPtr pMLoc = LocalRef(pSelf);

	uint8_t t = mrDC.PtrSize();
	if (pSelf->IsIndirectEx())
		t = mrDC.PtrSizeEx();
	
	SSID_t ssid(pSelf->SSID());
	const STORAGE_t &st(Storage(ssid));

	pSelf->m_opc = ssid;
	pSelf->m_optyp = st.isFlag() ? 0 : t;
	pSelf->m_disp = 0;
	SetLocalRef(pSelf, nullptr);//wtf??

	mpU = DumpThrough0(pSelf, mpU);

	pSelf->m_opc = opc;
	pSelf->m_optyp = optyp;
	pSelf->mOffs = opid;
	pSelf->m_disp = disp;
	SetLocalRef(pSelf, pMLoc);

/*
	VALUE_t v;
	HOP pOp = pSelf->__FilterDumpOp(pSelf->m_pXIn, Path(), v);
	if (pOp)
	{
		if ( pOp->IsCode() )
		{
			if ( !pOp->IsRootEx() )
			{
				mpU = mpU->Add(ACTN_GET);
				Out_t *pOutB = mpU->AddB(pSelf);
				return pOp->OutExpression(mpU);
			}
		}
		else if (pOp->IsArgOp())
		{
			mpU = mpU->Add(ACTN_GET);
			Out_t *pOutB = mpU->AddB(pSelf);
			return pOp->DumpTerminal(mpU);
		}
	}

	//terminal
	mpU = mpU->AddB(pSelf);
//	if (action == ACTN_INDIR)
//		os << "*";
	if (0)
	if (TypOf(mpU).isPtr())
	{
		int nPtr;
		if (mpU->m_p Op0->GetBasePtrStruc(&TypOf(mpU).m_pStruc, &nPtr))
			if (nPtr)
				TypOf(mpU).SetPtr(nPtr);
	}*/
	return mpU;
}

Out_t * EXPR_t::DumpIndirect2(CHOP pSelf, Out_t *mpU) const
{
	if (IsUnfoldMode())
	{
		return Add(mpU, pSelf);
	}

CHECK(OpNo(PrimeOp(pSelf)) == 990)
STOP

	outsv_t outsv(mpU);

	if (!pSelf->IsIndirectB())
	{
		Out_t *pOutThis;
/*		if (IsAddr(pSelf) && pSelf->m _pMLoc)
		{//ptr
			if (pSelf->m_disp != 0)
				mpU = mpU->Add(ACTN_ADD);
			Out_t *pOutUsv = mpU->Add(ACTN_OFFS);
			if (pSelf->m_disp != 0)
				mpU->Add(pSelf->m_disp);
			mpU = pOutUsv;

			pOutThis = mpU->Add(pSelf->m_p MLoc);
			if (pSelf->m_p MLoc->IsD ata() && pSelf->m_pData->m_pOp)
				TypOf(pOutThis).Setup(pSelf->m_pData->m_pOp);
		}
		else
		{
			if (IsAddr(pSelf))
				mpU = mpU->Add(ACTN_OFFS);
			pOutThis = mpU->Add(this);
		}*/

		pOutThis = Add(mpU, pSelf);
	}
	else
	{
		mpU = Add(*mpU, pSelf, ACTN_INDIR);
		TYP_t Tself(fromOp(pSelf));
		//mpU->setTyp(Tself);

		//if (!IsFuncStatusAborted(&mrFuncDefRef))
		if (ProtoInfo_t::IsFuncStatusFinished(FuncDefPtr()))
		{
			Out_t *pIndir(mpU);
			Out_t *pOutType(Add(*pIndir, pSelf, ACTN_TYPE));
			pOutType->setTyp(PtrOf(Tself));
			mpU = pOutType;
		}

		FieldPtr pFieldRef(LocalRef(pSelf));
		if (pFieldRef)//?
		{
			mpU = Add(*mpU, pSelf, ACTN_ADD);
			Out_t *pOutOffs(Add(*mpU, pSelf, ACTN_OFFS));
			Out_t *pOutMLoc(Add(*pOutOffs, pFieldRef, pSelf));
			pOutMLoc->setTyp(fromField(pFieldRef));
		}

		Out_t* pOutB;
		if (OpDisp(pSelf) != 0)
		{
			Out_t* peAddSub;
			if (OpDisp(pSelf) > 0)
				peAddSub = Add(*mpU, pSelf, ACTN_ADD);
			else
				peAddSub = Add(*mpU, pSelf, ACTN_SUB);
			peAddSub->setTyp(PtrOf(TYP_t()));

			/*if (!mbDeadOp)
			{
				//at this point don't know what the deref type is, so make char*
				TYP_t Tchar(TYP_t(fromOpType(OPTYP_INT8)));

				peAddSub->setTyp(PtrOf(Tchar));
				Out_t* pOutType(Add(*peAddSub, pSelf, ACTN_TYPE));
				pOutType->setTyp(peAddSub);
				pOutB = DumpB(pSelf, pOutType);
			}
			else*/
				pOutB = DumpB(pSelf, peAddSub);

			Add(*peAddSub, pSelf, abs(OpDisp(pSelf)));//displacement
		}
		else
		{
			pOutB = DumpB(pSelf, mpU);
		}
		if (pSelf->IsIndirectEx())
			pOutB->setTyp(fromOpType(mrDC.PtrSizeEx()));
		else
			pOutB->setTyp(fromOpType(mrDC.PtrSize()));
	}

	return outsv.get();
}

Out_t * EXPR_t::DumpFlagsOnly(CHOP pSelf, Out_t *mpU) const
{
	Out_t *pFlags = Add(mpU, pSelf, FOPC_CPUSW, OPTYP_UINT16, 0);
	return pFlags;
}

Out_t * EXPR_t::DumpTerminal(CHOP pSelf, Out_t *mpU) const
{
//	char str[128];

	if ( /*pSelf->IsCodeOp() &&*/ (pSelf->IsIndirect() || IsAddr(pSelf)) )
	{
//		if (PrimeOp(pSelf) )
		{
			return DumpIndirect2(pSelf, mpU);
		}
	}
	else
/*	if (pSelf->OpC() == OPC_ CPUSW)
	{
		if (!IsPrimeOp(pSelf))
		{
			Action_t A = PrimeOp(pSelf)->mAction();
			if (ISJMPIF(A))
			{
				return pSelf->OutRawCondition(mpU);
			}
		}
	}
	else*/
	if (pSelf->IsScalar())
	{
		if (IsRetOp(pSelf))
		{
			if (OpDisp(pSelf) == 0)//??
				return 0;
		}
	}
	else
	if (pSelf->OpC() == OPC_CPUREG)
		if (pSelf->m_disp != 0)
		{
			assert(0);
			mpU = Add(*mpU, pSelf, ACTN_ADD);
			Add(mpU, pSelf);
			Add(*mpU, pSelf, pSelf->m_disp);
			return mpU;
		}

	mpU = Add(mpU, pSelf);
/*?	if (0 && isPtr())
	{
		int nPtr;
		if (pSelf->GetBasePtrStruc(&TypOf(mpU).m_pStruc, &nPtr))
			if (nPtr)
				TypOf(mpU).SetPtrLevel(nPtr);
	}
	else*/
	{
		if (OPTYP_IS_PTR(pSelf->m_optyp))
		{
			TYP_t T;
			//for (int l(Ptr2Level((OpType_t)pSelf->m_optyp)); l > 0; l--)
				T = PtrOf(T);
			mpU->setTyp(T);
		}
		else
			mpU->setTyp(fromOpType(pSelf->m_optyp));
	}

	if (IsThisPtrOp(pSelf))
		mpU->mSsid = (OPC_t)(mpU->mSsid | SSID_THISPTR);

	return mpU;
}

Out_t * EXPR_t::DumpGoto(CHOP pSelf, Out_t *pOutRoot) const
{
	/*if (IsRetOp(pSelf))
	{
		return DumpRet(pSelf, mpU);
	}*/

	Out_t* mpU(pOutRoot);
	//outsv_t outsv(mpU);

//	if (!mpU->is(ACTN_SWITCH))
	{
//		if (IsRetOp(pSelf))
	//		mpU = Add(*mpU, pSelf, ACTN_RET);
		//else
			//mpU = Add(*mpU, pSelf, ACTN_GOTO);
			assert(mpU->isTerm());
			mpU->mAction = ACTN_GOTO;
	}

	if (IsAddr(pSelf))
		DumpTerminal(pSelf, mpU);
	else if (pSelf->IsIndirect())
		DumpIndirect2(pSelf, mpU);
	else
		DumpExpression(pSelf, mpU, 1);

	if (pSelf->hasArgs())
	{
		for (OpList_t::Iterator i(pSelf->argsIt()); i; i++)
		{
			HOP pOp(i.data());
			if (i != pSelf->argsIt())
				mpU = InsertParent(mpU, ACTN_COMMA);

			FieldPtr pField(FindRetField(pSelf, pOp));
			assert(pField);
			mpU = DumpExpression(pOp, mpU);

			if (!IsUnfoldMode())
			{
				mpU = InsertParent(mpU, ACTN_TYPE);
				mpU->setTyp(fromField(pField));
			}
		}
	}

	//return outsv.get();
	return mpU;
}

Out_t * EXPR_t::DumpCallUnfold(CHOP pSelf, Out_t *mpU) const
{
	outsv_t outsv(mpU);

	assert(ISCALL(ActionOf(pSelf)));

	Out_t *pOutPr = 0;
	for (XOpList_t::Iterator i(pSelf->m_xout); i; i++)
	{
		HOP pOp = i.data();
		//assert(pOp->IsCallOutOp());
		if (!pOutPr)//first
		{
			mpU = Add(*mpU, pSelf, ACTN_MOV);
			pOutPr = DumpTerminal(pOp, mpU);//l-value
		}
		else
		{
			Out_t *pComma = InsertParent(pOutPr, ACTN_COMMA);
			pOutPr = Add(pComma, pOp);
		}
	}
	

	mpU = Add(*mpU, pSelf, ActionOf(pSelf));
	mpU->mpOp = pSelf;//for easy access
	DumpTerminal(pSelf, mpU);

	//args
	for (OpList_t::Iterator i(pSelf->argsIt()); i; i++)
	{
		HOP pArg(i.data());
		if (!i.is_last())//> 1
			mpU = Add(*mpU, pSelf, ACTN_COMMA);
		DumpExpression(pArg, mpU);	
	}

	return outsv.get();
}

Out_t *EXPR_t::DumpOpsInRow(CHPATH hPath, Out_t *mpU) const
{
	outsv_t outsv(mpU);

	if (hPath)
	{
		for (PathOpList_t::Iterator i(hPath->ops()); i; ++i)
		{
			HOP pArg(i.data());
			if (!i.is_last())//> 1
				mpU = Add(*mpU, pArg, ACTN_COMMA);
			DumpExpression(pArg, mpU);
		}
	}

	return outsv.get();
}

Out_t *EXPR_t::DumpIntrinsic(CHOP pSelf, Out_t *mpU) const
{
	assert(ISINTRINSIC(ActionOf(pSelf)));//special action

	outsv_t outsv(mpU);

	if (IsUnfoldMode())
	{
		if (pSelf->OpC())
		{
			mpU = Add(*mpU, pSelf, ACTN_MOV);
			mpU->setTyp(fromOp(pSelf));
			Add(mpU, pSelf);
		}

		mpU = Add(*mpU, pSelf, ActionOf(pSelf));
		mpU->setTyp(fromOp(pSelf));

		for (OpList_t::Iterator i(pSelf->argsIt()); i; i++)
		{
			HOP pOpIn(i.data());
			if (!i.is_last())//> 1
				mpU = Add(*mpU, pSelf, ACTN_COMMA);
			DumpExpression(pOpIn, mpU);
		}
	}
	else
	{
		if (pSelf->OpC())
		{
			mpU = Add(*mpU, pSelf, ACTN_MOV);
			mpU->setTyp(fromOp(pSelf));
			Out_t *pOut = DumpTerminal(pSelf, mpU);
		}
		mpU = Add(*mpU, pSelf, ActionOf(pSelf));
		mpU->setTyp(fromOp(pSelf));
		for (OpList_t::Iterator i(pSelf->argsIt()); i; i++)
		{
			HOP pOpIn(i.data());
			if (!i.is_last())//> 1
				mpU = Add(*mpU, pSelf, ACTN_COMMA);
			DumpExpression(pOpIn, mpU);
		}
	}
	return outsv.get();
}

Out_t * EXPR_t::DumpCall(CHOP pSelf, Out_t *mpU) const
{
	if (IsUnfoldMode())
		return DumpCallUnfold(pSelf, mpU);

	outsv_t outsv(mpU);
	
	assert(ISCALL(ActionOf(pSelf)));

	FieldPtr pRetField(nullptr);
	if (IsRootEx(pSelf))
	{
		Out_t * pOutPr(nullptr);
		for (XOpList_t::Iterator i(pSelf->m_xout); i; i++)
		{
			HOP pOp(i.data());
			//assert(pOp->IsCallOutOp());
			if (!pOutPr)//first
			{
				/*if (!IsUnfoldMode())
				{
					HOP pVarOp(pOp->IsBoundToVar());
					if (pVarOp)// && !pVarOp->isRoot())
						mpU = Add(*mpU, pSelf, ACTN_VAR);
				}*/

 				mpU = Add(*mpU, pSelf, ACTN_MOV);
				pOutPr = DumpTerminal(pOp, mpU);//l-value
				assert(!mpU->is(ACTN_TYPE));
				mpU = Add(*mpU, pSelf, ACTN_TYPE);//self-adapting
				CGlobPtr iCallee(GetCalleeFuncDef(pSelf));
				if (iCallee)
					pRetField = FindCalleeRetField(iCallee, pOp, pSelf);
			}
			else
			{
				Out_t *pComma = InsertParent(pOutPr, ACTN_COMMA);
				pOutPr = Add(pComma, pOp);
			}
		}
	}

	if (mpU->is(ACTN_MOV))
		mpU = Add(*mpU, pSelf, ACTN_TYPE);//self-adapting
	
	Out_t* pOutCall(Add(*mpU, pSelf, ActionOf(pSelf)));
	if (pRetField)
		pOutCall->setTyp(fromField(pRetField));
	pOutCall->mpOp = pSelf;//for easy access

	//dump call target address (l-value)
	if (IsAddr(pSelf))
	{
		Out_t *pOut = DumpTerminal(pSelf, pOutCall);
	}
	else
	{
		Out_t* pOut;
		if (pSelf->IsIndirect())
			pOut = DumpIndirect2(pSelf, pOutCall);
		else
			pOut = DumpExpression(pSelf, pOutCall, 1);
	}

	//insert a call type
	Out_t* pCallType(InsertParent(pOutCall->mpL, ACTN_TYPE));
	pCallType->setTyp(fromCall(pOutCall));

	mpU = pOutCall;

	for (OpList_t::Iterator i(pSelf->argsIt()); i; )
	{
		HOP hArg(i.data());
		//FieldPtr pField(nullptr);
		i++;
		if (i)
			mpU = Add(*mpU, pSelf, ACTN_COMMA);

		DumpExpression(hArg, mpU);
	}

	return outsv.get();
}

Out_t * EXPR_t::DumpRet(CHOP pSelf, Out_t *mpU) const
{
	assert(IsRetOp(pSelf));
	if (!pSelf->hasArgs())
		return mpU;

	outsv_t outsv(mpU);

	assert(mpU->is(ACTN_RET));

	for (OpList_t::Iterator i(pSelf->argsIt()); i; i++)
	{
		HOP pOp(i.data());
		if (i != pSelf->argsIt())
			mpU = InsertParent(mpU, ACTN_COMMA);

		FieldPtr pField(FindRetField(pSelf, pOp));
		assert(pField);
		mpU = DumpExpression(pOp, mpU);

		mpU = InsertParent(mpU, ACTN_TYPE);
		mpU->setTyp(fromField(pField));
	}
	
	return outsv.get();
}

int ExprInfoBase_t::CalcGetOffset(CHOP pSelf, CHOP pOpR) const
{
	HOP pOpL = pSelf;

	if (pOpR->OpC() == OPC_SEGREG)
	if (pOpL->OpC() != OPC_SEGREG)
	{
		OpSeg_t seg = (OpSeg_t)pOpL->OpSeg();
		int offs = (seg-1) * OPSZ_WORD;
		assert(pOpR->mOffs == offs);
		assert(pOpR->OpSize() == OPSZ_WORD);
		return pOpL->Size();
	}

	int d = 0;
	switch (pOpL->OpC())
	{
	case OPC_CPUREG:
	case OPC_AUXREG:
	case OPC_SEGREG:
		assert(pOpR->OpC() == pOpL->OpC());
		break;
	default:
		if (!IsLocalOp(pOpL))
			return 0;
		assert(IsLocalOp(pOpR));
	}

	d = (pOpR->Offset0()) - (pOpL->Offset0());//bytes
	return d;
}

bool ExprInfoBase_t::IsOpTerminal(CHOP pSelf) const
{
	if ( IsUnfoldMode() )
		return true;

	if (!pSelf->XIn())
		return true;
	if (pSelf->IsIndirectB())
		return true;

	return false;
}

Out_t * EXPR_t::DumpThrough0(CHOP pSelf, Out_t *mpU) const
{
	if (IsOpTerminal(pSelf))
		return DumpTerminal(pSelf, mpU);

	bool bScattered(pSelf->CheckScatteredDependency());

	outsv_t	outsv(mpU);

	if (!bScattered)
	{
		HOP pOp = pSelf->XIn()->data();

		if ( !IsArgOp(pOp) )
			if ( IsRootEx(PrimeOp(pOp)))
				return DumpTerminal(pSelf, mpU);

		/*if (0)
		if (isEqualTo(pOp, pSelf))
			if (pSelf->OpC() != OPC_ CPUSW)
				if (pSelf->OpC() != OPC_FPUSW)
					return DumpExpression(pOp, mpU);*/
		
		if (IsCallOutOp(pSelf))
		{
			mpU = Add(*mpU, pSelf, ACTN_MOV);
			mpU->setTyp(fromOp(pSelf));
			DumpTerminal(pSelf, mpU);
		}
		else //if (!isEqualTo(pSelf, pOp))
		{
			mpU = Add(*mpU, pSelf, ACTN_GET);
			DumpTerminal(pSelf, mpU);
		}

/*		if (pSelf->OpC() == OPC_ CPUSW)
		if (pOp->OpC() != OPC_ CPUSW)
		if (!(pOp->CPUFlags() & F_CF))
		{
			//(a,csw)=b-1;if(csw)...; => a=b;csw~~eax;if(csw)...
			mpU = mpU->Add(ACTN_CHECK);
			mpU->Add(OPC_ CPUSW, OPTYP_UINT16, 0, pOp);
			mpU = mpU->Add(ACTN_GET);
			pOp->Out2(mpU);
		}
*/
		int flags = 0;
		if (pSelf->isCPUSW())
		if (!pOp->isCPUSW())
		{//(a,csw)=b;if(csw)...; => csw~~b;if(csw)...
			flags = 2;
		}

		Out_t *pOut(DumpExpression(pOp, mpU, flags));
		if (pOut->is(ACTN_CALL))
			pOut->setTyp(fromOp(pSelf));

		/*if (!mpU->mpU->isComparAction0())
		{
			Dispose(DetachParent(mpU->mpR));
			return pOut;
		}*/

		return mpU;//outsv.get();
	}


	Out_t *pOutThis = 0;
	XOpList_t::Iterator iXIn(pSelf->m_xin);
	do {
		HOP pOp(iXIn.data());

#if(0)
		if (pOp == PrimeOp(pSelf))//depenency on itself?
		{
			iXIn++;
			continue;//????
		}
#endif

		if (!IsArgOp(pOp))
			if (IsRootEx(PrimeOp(pOp)))
			{
//				if (pXIn != pSelf->XIn())
//					STOP
//				while(mpU->is(ACTN_GET))
//					mpU = Detach(mpU);
//				return Out2(mpU);
				if (pOutThis)
				{
					DetachParent(pOutThis);
					return pOutThis;
				}
				return DumpTerminal(pSelf, mpU);
			}

		if (iXIn.is_first())//first
		{
			mpU = Add(*mpU, pSelf, ACTN_GET);
			pOutThis = DumpTerminal(pSelf, mpU);
		}

		XOpList_t::Iterator iXInNx(iXIn);
		iXInNx++;
		if (iXInNx)
			mpU = Add(*mpU, pSelf, ACTN_FRACT);
		
		int n = CalcGetOffset(pSelf, pOp);
		if (n != 0)
		{
			Out_t *pOutOffs = Add(*mpU, pSelf, ACTN_SHIFT);
			pOutOffs->mOffs = n;
//			TYP_t T;
//			pOp->GetType(T);
//			pOutOffs->setTyp(T);
			Out_t *pOut = DumpExpression(pOp, pOutOffs);
		}
		else
		{
			Out_t *pOut = DumpExpression(pOp, mpU);
			pOut->mOffs = n;
	//		if (pOut->mAction)
	//			if (pOut->is(ACTN_MOV))
	//			{
	//				pOut = pOut->mpL;
	//				pOut->m_nOffset = n;
	//			}
		}
//		if (n)
//			pOut->m_nOffset = n;

		iXIn = iXInNx;
	} while (iXIn);

	mpU = outsv.get();
//	mpU = CheckFractal(mpU);
	return mpU;
}

Out_t * ExprInfoBase_t::CheckTypeCast(CHOP pSelf, Out_t *mpU) const
{
	if (IsPrimeOp(pSelf))
		return mpU;

	if (IsUnfoldMode())
		return mpU;

	HOP hPrime(PrimeOp(pSelf));
	if (IsDeadEx(hPrime))
		return mpU;
	
	Action_t Action(ActionOf(hPrime));

	if (Action == ACTN_DIV)//sizeof(lOp) == 2*sizeof(rOp)
		return mpU;

	//if (is(ACTN_MUL))//sizeof(lOp) == 2*sizeof(rOp)
		//return mpU;

	if (Action == ACTN_UNITE)
		return mpU;

	if (ISBOOL(Action))
		return mpU;

	uint8_t t1 = hPrime->OpType();
	uint8_t t2 = pSelf->OpType();

	if (Action != ACTN_MOV)
	{
		if (hPrime->isCPUSW() || hPrime->OpC() == (OPC_FLAGS|OPC_FPUSW))
			t1 = hPrime->arg1()->OpType();
		else if (pSelf->isCPUSW())
			t2 = hPrime->OpType();
	}

	if (Action == ACTN_MOV)
	{
		if (hPrime->isRoot())
		{
			if (mpU->is(ACTN_TYPE))
				return mpU;
			return Add(*mpU, pSelf, ACTN_TYPE);//self-adapting
		}
	}
//	else
//		return mpU;

	if (OPTYP_IS_PTR(t1) && !OPSIZE(t1))
		t1 = mrDC.PtrSize();
	if (OPTYP_IS_PTR(t2) && !OPSIZE(t2))
		t2 = mrDC.PtrSize();

	if (!AgreeTypes(t1, t2))
	{
		Out_t *pType = Add(*mpU, pSelf, ACTN_TYPE);
		pType->setTyp(fromOpType(t1));
		mpU = pType;
	}

	return mpU;
}

int ExprInfoBase_t::CheckFlagsUsage(CHOP pSelf) const
{
	if (!IsPrimeOp(pSelf))
		return 0;
	if (!IsRootEx(pSelf))
		return 0;

	if (pSelf->isCPUSW())
		return 0;

	for (XOpList_t::Iterator i(pSelf->m_xout); i; i++)
	{
		HOP pOp = i.data();
		if (pOp->isCPUSW())
			return 1;
	}

	return 0;
}

bool ExprInfoBase_t::IsRootEx(CHOP pSelf) const
{
	if (!IsPrimeOp(pSelf))
		return false;

	if (IsUnfoldMode())
		return true;

	if (pSelf->isRoot())
		return true;

	if (!pSelf->XOut())
		return true;

	return false;
}

Out_t * EXPR_t::DumpExpression(CHOP pSelf, Out_t *mpU, int flags) const
{
	outsv_t	outsv(mpU);

	if (IsVarOp(pSelf))
	{
		if (IsUnfoldMode() || pSelf->isRoot())
		{
			mpU = Add(*mpU, pSelf, ACTN_VAR);
//			mpU->setTyp(fromField(LocalRef(pSelf)));
			return DumpTerminal(pSelf, mpU);
		}
	}
	else if (IsArgOp(pSelf))
	{
		if (IsUnfoldMode())// || pSelf->isRoot())
		{
			mpU = Add(*mpU, pSelf, ACTN_ARG);
			mpU->setTyp(fromField(LocalRef(pSelf)));
			return DumpTerminal(pSelf, mpU);
		}
	}

	if (!IsPrimeOp(pSelf) || (flags & 1))
	{
		return DumpThrough0(pSelf, mpU);
	}

	assert(IsPrimeOp(pSelf));
	((EXPR_t *)this)->m_addresses.insert(DockField()->_key() + OpNo(pSelf));

//CHECK(pSelf->IsCode() && PrimeOp(pSelf)->No() == 1092)
//STOP

	if (IsCondJump(pSelf))
	{
		return DumpCondJump(pSelf, mpU);
	}

	/////////////////////////////
	if (ISCALL(ActionOf(pSelf)))
	{
 		return DumpCall(pSelf, mpU);
	}

	if (ISINTRINSIC(ActionOf(pSelf)))
	{
		return DumpIntrinsic(pSelf, mpU);
	}

	if (IsGoto(pSelf))
	{
		return DumpGoto(pSelf, mpU);
	}

	if (IsArgOp(pSelf) || IsExitOp(pSelf) || IsDataOp(pSelf))
	{
		return DumpTerminal(pSelf, mpU);
	}

	Action_t __Action = ACTN_MOV;
//	bool bDumpLValue = false;
//	if (IsRootEx())
//	{
//		if (!IsGoto())
//			bDumpLValue = true;
//	}
//	else 
//	{
//		if (isLValueVisible())
//			bDumpLValue = true;
//	}
//	
	if (ActionOf(pSelf) != ACTN_MOV)
		if (!mpU->is(ACTN_PARITY))
	{
		if (flags & 2)
			__Action = ACTN_CHECK;
		else
		{
			if ((pSelf->OpC() == (OPC_FLAGS|OPC_FPUSW)) && (pSelf->arg1()->OpC() != (OPC_FLAGS|OPC_FPUSW)))
 				__Action = ACTN_CHECK;
			else if (pSelf->isCPUSWflags() && (!pSelf->OpOffsU()) && !pSelf->arg1()->isCPUSWflags())
				__Action = ACTN_CHECK;
		}
	}

//	if (bDumpLValue)
//	{
/*		if (bFlagsEx)
		if (!IsRootEx())
		{
			//(eax,csw)=eax-1;if(csw)gotoL; => eax=eax-1;csw~~eax;if(csw)gotoL
			mpU = mpU->Add(ACTN_CHECK);
			mpU->Add(OPC_ CPUSW, OPTYP_UINT16, 0, this);
			mpU = mpU->Add(ACTN_GET);
			Out2(mpU);
		}*/

#if(0)
	if (!IsUnfoldMode() && !mpU->mpU && !pSelf->isRoot())
		;
	else
#endif
	{
		mpU = Add(*mpU, pSelf, __Action);
		if (pSelf->isInRow() && pSelf->isRoot() && !pSelf->isHidden())
			mpU->m_bPostCall = true;
		Out_t *pLeft;
		if (flags & 2)
			pLeft = DumpFlagsOnly(pSelf, mpU);
		else
		{
			pLeft = DumpTerminal(pSelf, mpU);

			if (CheckFlagsUsage(pSelf))
			{
				Out_t *pComma = InsertParent(pLeft, ACTN_ASSCHK);
				Out_t *pFlags = DumpFlagsOnly(pSelf, pComma);
				//?	pFlags->m_p Op0 = this;//->m_pRoot = prim eOp();
			}
		}

		if (__Action != ACTN_CHECK)
			mpU->setTyp(fromOp(pSelf));
		if (__Action == ACTN_MOV)
			if (pSelf->isRoot())
				if (!IsUnfoldMode() && !IsDeadEx(pSelf))
				{
					assert(!mpU->is(ACTN_TYPE));
					mpU = Add(*mpU, pSelf, ACTN_TYPE);//self-adapting
				}
	}

	if (ISSETIF(ActionOf(pSelf)))
	{
		DumpCond(pSelf, mpU);
		return outsv.get();
	}

/*	if (0)
	if (mAction() == ACTN_QUERY)
	{
		Out_t *pOutQ = mpU->Add(mAction());
		arg1()->OutExpression(pOutQ);
		Out_t *pOutE = pOutQ->Add(ACTN_ELSE);
		pOutE->Add((int)1);
		pOutE->Add((int)0);
		return outsv.get();
	}*/

	bool bAction = true;
//	if (isComparAction0())
//	if (arg1()->OpC() == OPC_ CPUSW)
//		bAction = false;

	assert(ActionOf(pSelf) < ACTN_CALL);
	if (bAction)
	if (CheckTestAction(pSelf))
		bAction = false;

	if (bAction)
	{
		mpU = Add(*mpU, pSelf, ActionOf(pSelf));
		mpU->m_bNoReduce = (pSelf->m_nFlags & OPND_NO_REDUCE) != 0;

		if (__Action != ACTN_CHECK)
			mpU->setTyp(fromOp(pSelf));
		else
			mpU->setTyp(fromOp(pSelf->arg1()));
	}

	for (OpList_t::Iterator i(pSelf->argsIt()); i; i++)
	{
		HOP pOpIn(i.data());
		Out_t *pOut(CheckTypeCast(pOpIn, mpU));
		DumpExpression(pOpIn, pOut);
	}

	return outsv.get();
}

IfCond_t XlateCond(uint32_t cond)
{
	IfCond_t result;
	if (cond == IFCOND_BE)//(CF|ZF=1)[<=]
		result = IFCOND_BE;
	else if (cond == IFCOND_AE)//(CF=0)[>=]
		result = IFCOND_AE;
	else if (cond == IFCOND_B)//(CF=1)[<]
		result = IFCOND_B;
	else if (cond == IFCOND_A)//(CF|ZF=0)[>]
		result = IFCOND_A;
	else if (cond == IFCOND_E)//(ZF=1)[==]
		result = IFCOND_E;
	else if (cond == IFCOND_NE)//(ZF=0)[!=]
		result = IFCOND_NE;
	else
	{
		assert(0);
	}
	return result;
}

Out_t * EXPR_t::DumpCondJump(CHOP pSelf, Out_t *mpU) const
{
	/*if (testf(adcui::DUMP_LOGICONLY))
	{
		//if (bInvert)
		//	os << "!";
		//os << GetFirstOp(Path())->No();
		//return mpU->Add(GetFirstOp(Path()));
		return Add(*mpU, *pSelf->Path());
	}*/

	assert(IsCondJump(pSelf));
	HOP pOpIn = pSelf->argsIt().data();
	assert(pOpIn && pOpIn->isCPUSW());
	return DumpExpression(pOpIn, mpU);
}

int ggg = 0;

// example:
//1: a = a + 1			;pOp3
//2: b ~~ a<1> - 2		;pOp1,pOp2
//3: c = a<1>;			;pOp5,pOp4
//4: if (d<2>) goto L;	;pOp0{cpusw}
/*Out_t *EXPR_t::CheckPostCall(HOP pOp0, Out_t *mpU)
{
	assert(pOp0->SSID() == SSID_CPUSW);
	//if (ggg)
	if (!IsUnfoldMode())
	if (pOp0->m_xin.check_count(1) == 0)
	{
		HOP pOp1(pOp0->XIn()->Op());
		if (!pOp1->isRoot())
		{
			HOP pOp2(&pOp1->arg1());
			if (pOp2->m_xin.check_count(1) == 0)
			{
				HOP pOp3(pOp2->XIn()->Op());
				for (XOpList_t::Iterator i(pOp3->m_xout); i; i++)
				{
					HOP pOp4(i.data());
					if (pOp4 != pOp2)
					{
						HOP pOp5(PrimeOp(pOp4));
						if (pOp5->Path() != pOp0->Path())
							break;
						if (pOp5->m_nFlags & OPND_POST)
						{
							assert(!mpU->is(ACTN_POSTCALL));
							mpU = Add(*mpU, pOp0, ACTN_POSTCALL);
							//Add(mpU, pOp4);
							DumpExpression(pOp5, mpU);
							mpU->flipChilds();
						}
					}
				}
			}
		}
	}
	return mpU;
}*/

/*Out_t* EXPR_t::CheckCombinedStatement(HOP pOp, Out_t *mpU)
{
	if (IsUnfoldMode())
		return mpU;

	HOP pOp4(nullptr);
	if (pOp->IsCondJump())
	{
		HOP pOp3(&pOp->arg1());
		if (pOp3->m_xin.check_count(1) == 0)
			pOp4 = pOp3->XIn()->Op();
	}
			
	assert(pOp->isRoot() && !pOp->isInRow());
	OpList_t::ReverseIterator i(pOp->Path()->ops(), pOp);
	i++;
	while (i)
	{
		HOP pOp2(i.data());
		if (!pOp2->isInRow())
			break;
		if (!pOp2->isRoot())
			continue;
		if (pOp2 == pOp4)
		{
			/ *mpU = Add(*mpU, ACTN_GET);
			DumpExpression(pOp2, mpU);
			mpU->flipChilds();* /
			break;
		}

		mpU = Add(*mpU, pOp, ACTN_COMMA);
		DumpExpression(pOp2, mpU);
		i++;
	}
	return mpU;
}*/

Out_t * EXPR_t::DumpExpression2(CHOP pSelf) const
{
#ifdef _DEBUG
	SetNo(OpNo(PrimeOp(pSelf)));
#endif
	Action_t A = ACTN_NULL;
	//if (IsRetOp(pSelf))
		//A = ACTN_RET;

CHECK(OpNo(pSelf) == 707)
STOP

	Out_t *pOutRoot = AddRoot();
	pOutRoot->mAction = A;
	pOutRoot->mpOp = pSelf;

	Out_t *mpU(pOutRoot);

	if (!IsUnfoldMode())
	{
		A = ACTN_SEMI;//ACTN_COMMA3;
#if(0)
		if (!(pSelf->isRoot() && !pSelf->isInRow()))
			return nullptr;
#else
		//?assert(pSelf->isRoot() && !pSelf->isInRow());
#endif
		PathOpList_t::ReverseIterator i(PathOf(pSelf)->ops(), INSPTR(pSelf));
		Out_t *pOutU2(mpU);
		for (++i; i; ++i)
		{
			HOP pOp2(PRIME(i.data()));
			if (pOp2->isHidden())
				continue;
			if (!pOp2->isRoot())
				continue;
			if (!pOp2->isInRow())
				break;
			if (pOp2->isRoot())
			{
				if (!pOutU2->is(A))
				{
					mpU = Add(*pOutU2, pSelf, A);
					pOutU2 = mpU;
				}
				else
				{
					pOutU2 = InsertParent(pOutU2, A);
					pOutU2->flipChilds();
				}
				DumpExpression(pOp2, pOutU2);
			}
		}
	}

	mpU = DumpExpression(pSelf, mpU);

#if(0)
	//check for combined statements
	if (!IsUnfoldMode())
	{

		assert(mrFuncDef.IsInvalid() || (pSelf->isRoot() && !pSelf->isInRow()));
		OpList_t::ReverseIterator i(pSelf->Path()->ops(), pSelf);
		for (++i; i; i++)
		{
			HOP pOp2(i.data());
			if (pOp2->isHidden())
				continue;
			if (!pOp2->isRoot())
				continue;
			if (!pOp2->isInRow())
				break;
			mpU = InsertParent(mpU, ACTN_SEMI);
			mpU->flipChilds();
			DumpExpression(pOp2, mpU);
		}
	}
#endif

	return pOutRoot;
}

Out_t *EXPR_t::DumpPreCondition(CHOP pSelf, Out_t *mpU) const
{
	if (IsUnfoldMode())
		return mpU;

	//check pre-condition
	assert(pSelf->isRoot() && !pSelf->isInRow());
	PathOpList_t::ReverseIterator i(PathOps(PathOf(pSelf)), INSPTR(pSelf));
	Out_t *pOutU2(mpU);
	for (++i; i; i++)
	{
		HOP pOp2(PRIME(i.data()));
		if (pOp2->isHidden())
			continue;
		if (!pOp2->isRoot())
			continue;
		if (!pOp2->isInRow())
			break;
		if (pOp2->isRoot())
		{
			if (!pOutU2->is(ACTN_COMMA3))
			{
				mpU = Add(*pOutU2, pSelf, ACTN_COMMA3);
				pOutU2 = mpU;
			}
			else
			{
				pOutU2 = InsertParent(pOutU2, ACTN_COMMA3);
				pOutU2->flipChilds();
			}
			DumpExpression(pOp2, pOutU2);
		}
	}
	return mpU;
}

void EXPR_t::DumpCond(CHOP pSelf, Out_t *mpU) const
{
	assert(IsCondJump(pSelf));

CHECK(OpNo(pSelf) == 209)
STOP

	//Out_t *pOutU0(mpU);
	//bool bForCond(CheckForCondition(pSelf));
	HOP pOp0(pSelf->arg1());


	/*if (bForCond)
	{
		if (mpU->is(ACTN_COMMA3))
			Dispose(mpU->mpL->DetachParent());
	}*/

	//mpU = CheckPostCall(pOp0, mpU);
	//mpU = CheckCombinedStatement(pSelf, mpU);
#if(0)
	Out_t *pOut(mpU);
	while (pOut->is(ACTN_COMMA))
	{
		pOut->flipChilds();
		pOut = pOut->mpU;
	}
#endif

	Out_t *pOut5(nullptr);
	Action_t A = ActionOf(pSelf);
	assert(ISJMPIF(A) || ISSETIF(A));
	A = (Action_t)(ACTN_JMPIF + (A & 0xf));
	if (A & 1)//inverted?
	{
		mpU = Add(*mpU, pSelf, ACTN_LOGNOT);
		A = (Action_t)(A & (~1));
		pOut5 = mpU;
	}
	
	mpU = Add(*mpU, pSelf, A);
	if (!pOut5)
		pOut5 = mpU;

#if(0)
	HOP pOp5(nullptr);
	//post-condition
	if (pOp5)
	{
		mpU = Add(*mpU, pSelf, ACTN_GET);
		DumpExpression(pOp0, mpU);
		DumpExpression(pOp5, mpU);
		//mpU->flipChilds();
		OpList_t::ReverseIterator i(pSelf->Path()->ops(), pOp5);
		i++;
		while (i)
		{
			HOP pOp2(i.data());
			if (!pOp2->isInRow())
				break;
			if (pOp2->isRoot() && !pOp2->isHidden())
			{
				pOut5 = InsertParent(pOut5, ACTN_COMMA);
				pOut5 = DumpExpression(pOp2, pOut5);
			}
			i++;
		}
	}
	else
#endif
	{
		DumpExpression(pOp0, mpU);
	}
}

void EXPR_t::DumpIFCond(CHOP pOpLast, Out_t *mpU) const
{
	if (!pOpLast)
	{
		assert(0);
		//?Add(*mpU, pOpLast, rSelf);
		return;
	}

	/*if (testf(adcui::DUMP_LOGICONLY))
	{
		assert(rSelf.IsTerminal());
		Add(*mpU, rSelf);
		return;
	}*/

	DumpCond(pOpLast, mpU);
}

void EXPR_t::DumpCondition2(CHPATH rSelf, Out_t *mpU, bool bInvert) const
{
	if (!rSelf->IsTerminal())
	{
		if (PathType(rSelf) & BLK_LOGIC)
		{
			assert(rSelf->CheckChildrenCount(2) == 0);

			if (IsUnfoldMode())
			{
				HPATH rPathLast = rSelf->LastChild();
				DumpCondition2(rPathLast, mpU, bInvert);
				return;
			}

			bool bInvert1 = bInvert;
			bool bInvert2 = bInvert;
			Action_t Action;
			if (PathType(rSelf) == BLK_AND)
			{
				Action = ((!bInvert)?(ACTN_LOGAND):(ACTN_LOGOR));
				bInvert1 = !bInvert1;
//					mAction = ACTN_LOGAND;
			}
			else if (PathType(rSelf) == BLK_OR)
			{
				Action = ((!bInvert)?(ACTN_LOGOR):(ACTN_LOGAND));
					//mAction = ACTN_LOGOR;
			}

//			if (bInvert)
//				mpU = mpU->Add(ACTN_LOGNOT);

			mpU = Add(*mpU, GetLastOp(rSelf), Action);
			DumpCondition2(rSelf->FirstChild(), mpU, bInvert1);
			DumpCondition2(rSelf->LastChild(), mpU, bInvert2);
			return;
		}
	}
	else
	{
//CHECK(pSelf->m.No() == 248)
//STOP
		HOP pOp(GetLastOp(rSelf));

		Out_t *pOutSemi1(nullptr), *pOutSemi2(nullptr);
		HPATH pIfForPath(CheckForCondition(pOp));
		if (pIfForPath)
		{
			// (pre-cond ; cond ; post-cond)
			pOutSemi2 = Add(*mpU, pOp, ACTN_SEMI2);
			pOutSemi1 = Add(*pOutSemi2, pOp, ACTN_SEMI2);
			Out_t *pOut(DumpPreCondition(pOp, pOutSemi1));
			if (pOut != pOutSemi1)
				if (pOut->is(ACTN_COMMA3))
				{
					assert(!pOut->mpR);
					Dispose(DetachParent(pOut->mpL));
				}
			mpU = pOutSemi1;
		}
		else
			mpU = DumpPreCondition(pOp, mpU);

		if (bInvert)
			mpU = Add(*mpU, pOp, ACTN_LOGNOT);

		DumpIFCond(pOp, mpU);

		if (pOutSemi1)
		{
			if (!pOutSemi1->mpR)//empty pre-condition in 'for' loop?
				pOutSemi1->flipChilds();

			HPATH pPathDoWhile(GetLoopWhileBackwardCondition(pIfForPath));
			assert(pPathDoWhile);
			HOP pOp2(GetLastOp(pPathDoWhile));
			Out_t *pOut(DumpPreCondition(pOp2, pOutSemi2));
			if (pOut != pOutSemi2)
				if (pOut->is(ACTN_COMMA3))
				{
					assert(!pOut->mpR);
					Dispose(DetachParent(pOut->mpL));
				}
		}
	}
}


bool EXPR_t::CompareConditions(CHPATH rSelf, CHPATH rPath2) const
{
	ResetArrays();

	Out_t *pOut1(DumpCondition4(rSelf));
	EXPRSimpl_t ES(*this);
	ES.Simplify(pOut1);

	Out_t *pOut2(DumpCondition4(rPath2));
	EXPRSimpl_t ES2(*this);
	ES2.Simplify(pOut2);

	return pOut1->mpL->isEqualTo(pOut2->mpL);
}

Out_t * EXPR_t::DumpOp(CHOP pSelf) const
{
	assert(IsPrimeOp(pSelf));
	
	Out_t *pOut = 0;
	if (IsCondJump(pSelf))//ISJMPIF(mAction()))
	{
		HPATH pPath = PathOf(pSelf);
		if (pPath)
		{
			pPath = GetLogicsTop(pPath, 0);
			if (pPath)
			{
				pOut = DumpCondition4(pPath);
				//EXPRSimpl_t ES(*this);
				//ES.Simplify(pOut);
			}
		}
	}
	else if (IsGoto(pSelf))
	{
		HPATH pPath = PathOf(pSelf);
		if (pPath && PathType(pPath) == BLK_JMPSWITCH)
		{
			//pOut = DumpSwitch(pSelf);
			pOut = DumpExpression2(pSelf);
		}
	}

	if (!pOut)
	{
		pOut = DumpExpression2(pSelf);
	}

	return pOut;
}

void EXPR_t::ResetArrays() const
{
//	ArrOps.Reset(); 
	mrExprCache.mOuts.Reset();
//	ArrTypes.Reset();
}




Out_t * EXPR_t::DumpCondition4(CHPATH rSelf) const
{
	bool bInvert = false;

	if (m_flags.testL(adcui::DUMP_BLOCKS))
	{
		if (PathType(rSelf->Parent()) == BLK_IF)
		{
			if (TreeIsFirst(rSelf))
				bInvert = true;
		}
		else if (rSelf->Parent()->IsIfWhileOrFor())
		{
			assert(TreeIsFirst(rSelf));
			bInvert = true;
		}
	}

	if (rSelf->isInverted())
		bInvert = !bInvert;

	Out_t *pOutRoot = AddRoot();
	pOutRoot->mAction = ACTN_GOTO;

//	int ID = -1;
//CHECK(id == 23)
//STOP2(ID = id)*/

	Out_t *mpU = pOutRoot;
	DumpCondition2(rSelf, mpU, bInvert);

	return pOutRoot;
}

bool EXPRSimpl_t::SimplifySwitch(Out_t* pOut, SwitchQuery_t& si, CHPATH pPath) const
{
	assert(pPath->Type() == BLK_JMPSWITCH);
	HPATH pPathDefault(TreePrevEx(pPath));
	if (!pPathDefault || pPathDefault->Type() != BLK_JMPIF)
		return false;//no default branch - indirect jump (out of the function)

	if (GetSwitchInfo(pOut, si))
	{
		si.pPathDefault = pPathDefault;
		if (!si.pIndexTable && si.pOutWrongTable)
		{
			// maybe the index table was incorrectly offseted?
			// try to find out the correct index displacement
			HOP pOpDefault(GetLastOp(pPathDefault));
			if (ActionOf(pOpDefault) == ACTN_ABOVE)//unsigned comparison - will cut off a lower boundary
			{
				//this instruction checks if adjusted index falls into the index table's boundary
				HOP pOpSub(pOpDefault->arg1()->XIn()->data());
				if (!pOpSub->isRoot() && ActionOf(pOpSub) == ACTN_SUB)
				{
					//this instruction unbiases the index
					//if the index's value is below lower margin, it is treated as a large unsigned by comparison instruction
					HOP pOpIdxS(pOpSub->arg1()->XIn()->data());
					if (!pOpIdxS->isRoot())
					{
						ExprCache_t rExpr(PtrSize());
						EXPR_t expr2(*this, pOpIdxS, 0, rExpr);
						Out_t *pOut2(expr2.DumpExpression2(pOpIdxS));
						EXPRSimpl_t ES(expr2);
						ES.Simplify(pOut2);

						if (pOut2->mpR->is(ACTN_SUB))
						{
							Out_t *pOutI(pOut2->mpR->mpR);
							if (pOutI->isImmidiate())
							{
								int nIndexOffset(pOutI->m_value.i32);
								assert(nIndexOffset != 0);

								//shifted address of the index table used in jump expression
								ADDR nTiAddrShifted(si.pOutWrongTable->field()->_key());
								if (si.pOutWrongOffset)
									nTiAddrShifted += si.pOutWrongOffset->m_value.i32;

								//get a correct index table
								ADDR nTiAddr(nTiAddrShifted + nIndexOffset);
								Locus_t loc;
								FieldPtr pFieldRef(FindFieldInSubsegs(PrimeSeg(), nTiAddr, loc));
								if (pFieldRef)
								{
									si.pOutWrongTable->setField(pFieldRef);
									si.pOutWrongTable->setTyp(fromField(pFieldRef));
									si.pIndexTable = pFieldRef;
									si.pOutWrongTable = nullptr;
									if (si.pOutWrongOffset)
									{
										si.pOutWrongOffset->m_value.clear();//make it zero
										si.pOutWrongOffset = nullptr;
									}
									{
										EXPRSimpl_t ES(*this);
										ES.__dump(777, pOut);
										ES.Simplify(pOut);
									}
								}
							}
						}
					}
				}
			}

			/*			EXPR_t expr2(pOpDefault, rFunc, 0);
			Out_t *pOut(expr2.Dump Expression2(pOpDefault));
			if (pOut->mpR->is(ACTN_CHECK))
			if (pOut->mpR->mpR->is(ACTN_SUB))
			{
			Out_t *pOutImm(pOut->mpR->mpR->mpR);
			if (pOutImm->isImmidiate())
			{
			//index table's expected size
			int nTiSize(pOutImm->m_value.i32);
			//shifted address of the index table used in jump expression
			ADDR nTiAddrShifted(si.pOutWrongTable->mpField->Offset());
			if (si.pOutWrongOffset)
			nTiAddrShifted += si.pOutWrongOffset->m_value.i32;
			STOP
			}
			}*/
		}
	}

	assert(pOut->is(ACTN_GOTO));
	pOut->mAction = ACTN_SWITCH;
	return true;
}

FieldPtr ExprInfoBase_t::CheckThruConst(const Out_t* pSelf, OFF_t& oPos) const
{
	if (!pSelf->isOpKind())
		return nullptr;
	if (!pSelf->mpOp)
		return nullptr;
	if (IsDataOp(pSelf->mpOp))
		return nullptr;
	if (!pSelf->mpOp->IsIndirect())
		return nullptr;

	if (pSelf->opc() == OPC_IMM)
		return nullptr;

	if (pSelf->mpU->is(ACTN_OFFS))
		return nullptr;

	if (pSelf->isLeft() && pSelf->mpU->is(ACTN_CALL))
		return nullptr;

	FieldPtr pData(fieldRefEx(pSelf->mpOp));
	if (!pData)
	{
		if (IsLocalOp(pSelf->mpOp))
			return nullptr;
		pData = FindFieldRef(pSelf->mpOp);
		if (!pData)
			return nullptr;
	}

	return FuncInfo_t::CheckThruConst(pData, oPos);
}


