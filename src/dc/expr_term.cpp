#include "expr_term.h"

Out_t::Out_t(ExprInfoBase_t* pExpr)
	:
	mAction((Action_t)0),
	mpU(nullptr),
	mpL(nullptr),
	mpR(nullptr),
	mpDockOp(HOP()),
	mpOp(HOP()),
	mpField(nullptr),
	mpExpField(nullptr),
	mpPath(HPATH()),
	mKind(OUT_NULL),
	mSsid(OPC_NULL),
	mOffs(0),
	mDisp(0),
	m_mult(0),
	m_eflags(0),
	m_seg(0),
	m_xin(nullptr),
	m_xout(nullptr),
	m_bExtraIn(false),
	m_bExtraOut(false),
	m_bNoReduce(false),
	m_bPostCall(false)
{
}

void Out_t::cloneFrom(const Out_t& o)
{
	mSsid = o.mSsid;
	mOffs = o.mOffs;
	mDisp = o.mDisp;
	m_mult = o.m_mult;
	m_eflags = o.m_eflags;
	m_seg = o.m_seg;
	mAction = o.mAction;
	mKind = o.mKind;
	mTyp = o.mTyp;
	mpDockOp = o.mpDockOp;
	mpOp = o.mpOp;
	mpField = o.mpField;
	mpExpField = o.mpExpField;
	//mpFieldRef = o.mpFieldRef;
	//m_nOffset = m_nOffset;
	m_value.i64 = o.m_value.i64;
	m_bExtraIn = o.m_bExtraIn;
	m_bExtraOut = o.m_bExtraOut;
	m_bNoReduce = o.m_bNoReduce;
	m_bPostCall = o.m_bPostCall;
}

int Out_t::depth()
{
	int i = 0;
	Out_t *pOut = this;
	while (pOut->mpU)
	{
		i++;
		pOut = pOut->mpU;
	}

	return i;
}


bool Out_t::AddChild(Out_t *pOut)
{
	assert(this);
//	if (!this)
//		return;

	if (ISUNARYPRE(mAction) || ISINTRINSIC(mAction))//to left
	{
		assert(!mpL && !mpR);
		mpR = pOut;
	}
	else if (ISUNARYPOST(mAction))
	{
		assert(!mpL && !mpR);
		mpL = pOut;
	}
	else
	{
		if (!mpL)
			mpL = pOut;
		else if (!mpR)
			mpR = pOut;
//		else if (is(ACTN_GET))
//			mpR->AddNx(pOut);
		else
		{
			assert(0);
			return false;
		}
	}

	pOut->mpU = this;
	return true;
}

VALUE_t Out_t::Value() const
{
	uint8_t t(mTyp.stripTypedef().optyp0());
	if (OPSIZE(t) > 8)
	{
		//abort();
	}
	return VALUE_t(t, m_value);
}

bool Out_t::isIntrinsicRef() const
{
	if (mTyp.isGlob())
	{
		CGlobPtr iGlob(mTyp.asGlobRef());
		if (iGlob && iGlob->typeFuncDef() && iGlob->flags() && FDEF_INTRINSIC)
			return true;
	}
	return false;
}


/*Out_t *Out_t::Add(FieldPtr pMLoc)
{
	Out_t *pOut(NewTerm());
	pOut->m_p MLoc = pMLoc;
	pOut->mKind = OUT_ MLOC;
	AddChild(pOut);
	return pOut;
}*/

/*
Out_t *Out_t::Add(VALUE_t *v)
{
	Out_t *pOut(NewTerm());
	pOut->mKind = OUT_IMM;
	pOut->m_value.i64 = v->i64;
	pOut->mTyp.Setup(v->typ);
	AddChild(pOut);
	return pOut;
}
*/


/*
Out_t *Out_t::AddNx(Out_t *pOut)
{
	Out_t *p = this;
	while (p->pOutNx)
		p = p->pOutNx;
	p->pOutNx = pOut;
	pOut->mpU = p->mpU;
	return pOut;
}
*/

bool Out_t::isCallLike() const
{
	if (is(ACTN_CALL))
		return true;
	if (ISINTRINSIC(mAction))
		return true;
	if (ISJMPIF(mAction))
		return true;
	//if (is(ACTN_RET))
		//return true;
	if (is(ACTN_LOHALF) || is(ACTN_HIHALF))
		return true;
	return false;
}

bool Out_t::isBadRet() const
{
	assert(is(ACTN_RET));
	assert(mpL->isOpKind());
	return (mpL->SSId() != SSID_LOCAL || mpL->mOffs != 0);
}

/*
void Out_t::MoveTo(int nPos)
{
	Out_t &p = *(Out_t *)mrDC.ArrOuts.Get(nPos);
	p.mAction = mAction;
	p.mpU = mpU;
	p.mpL = mpL;
	p.mpR = mpR;
	p.pOp = pOp;

	if (mpL)
		mpL->mpU = &p;
	if (mpR)
		mpR->mpU = &p;
	if (mpU)
	{
		if (mpU->mpL == this)
			mpU->mpL = &p;
		else if (mpU->mpR == this)
			mpU->mpR = &p;
	}
}
*/

Out_t *Out_t::CheckIndirParent()
{
	//check if there is a redirection
	Out_t *pOut(this);
	while (pOut->mpU)
	{
		if (pOut->mpU->is(ACTN_INDIR))
			return pOut->mpU;
		pOut = pOut->mpU;
	}
	return 0;
}

bool Out_t::CheckIndirReady()
{
	//check if there is a redirection
	Out_t *pOut(this);
	while (pOut->mpU)
	{
		Out_t *pU(pOut->mpU);
		if (pU->is(ACTN_INDIR))
			return true;
		if (!pU->is(ACTN_ADD))
			if (!pU->is(ACTN_SUB))
			{
				if (!pU->is(ACTN_TYPE) || !pU->mpU->is(ACTN_INDIR))
					break;
			}
		pOut = pU;
	}
	return false;
}

bool Out_t::CheckIndirReady2()
{
	//check if there is a redirection
	Out_t *pOut(this);
	while (pOut->mpU)
	{
		Out_t *pU(pOut->mpU);
		if (pU->is(ACTN_INDIR))
			return true;
		if (!pU->is(ACTN_ADD))
			if (!pU->is(ACTN_SUB))
			{
				if (!pU->is(ACTN_TYPE))
					break;
				if (!pU->mTyp.isPtr())
					break;
			}
		pOut = pU;
	}
	return false;
}

bool Out_t::isEqualTo(const Out_t *pOut) const
{
	if (!pOut)
		return false;

	if (!is(pOut->mAction))
		return false;

	if (mAction)
	{
		if (mpL)
		{
			if (!mpL->isEqualTo(pOut->mpL))
				return false;
		}
		else
		{ assert(!pOut->mpL); }

		if (mpR)
		{
			if (!mpR->isEqualTo(pOut->mpR))
				return false;
		}
		else
			assert(!pOut->mpR);
	}
	else
	{
		if (!isEqualToTerm(pOut))
			return false;
	}

	return true;
}

bool Out_t::isEqualToTerm(const Out_t *pOut) const
{
	if (mKind != pOut->mKind)
		return false;
	switch (mKind)
	{
	case OUT_OP:
		if (opc() != pOut->opc())
		{
			if (SSId() == pOut->SSId())
			{
				if (pOut->opc() == OPC_CPUSW)
				{
					uint8_t off(pOut->opoff());
					uint8_t siz(pOut->mTyp.size());
					assert(off + siz < mTyp.size());
					unsigned f(unsigned(siz) << off);
					if (m_eflags == f)
						return true;
				}
			}
			return false;
		}
		if (opoff() != pOut->opoff())
			return false;
		if (mTyp.size() != pOut->mTyp.size())
			return false;
		break;
	case OUT_FIELD:
		assert(field() && pOut->field());
		if (field() != pOut->field())
			return false;
		break;
	case OUT_IMM:
		if (m_value.i64 != pOut->m_value.i64)
			return false;
		break;
	case OUT_ARG:
		if (mTyp.size() != pOut->mTyp.size())
			return false;
		break;
	default:
		return false;
	}
	return true;
}

Out_t *Out_t::sibling() const
{
	assert(mpU);
	if (isLeft())
		return mpU->mpR;
	return mpU->mpL;
}




/*int Out_t::CombineTypes(TYP_t tL, TYP_t tR, TYP_t &T)
{
	if (tL.isComplex 5())
	{
		if (!tR.isComplex 5())
		{
			T.mpStruc = tL.mpStruc;
			T.mnPtr = tL.mnPtr;
		}
		else
			assert(tL.mpStruc == tR.mpStruc);
	}
	else if (tR.isComplex 5())
	{
		T.mpStruc = tR.mpStruc;
		T.mnPtr = tR.mnPtr;
	}

	uint8_t ptrmask1, ptrmask2;
	if (tL.PtrLevel(&ptrmask1) != tR.PtrLevel(&ptrmask2))
		assert(false);

	return 1;
}*/

/*
int Out_t::GetType(TYP_t &Type)
{
	Type.optyp = mTyp.optyp;
	Type.m_nPtr = mTyp.m_nPtr;
	Type.m_nArray = mTyp.m_nArray;
	return 1;
}
*/





