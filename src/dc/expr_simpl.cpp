#include "expr_simpl.h"
#include "prefix.h"
#include "shared/defs.h"
#include "shared/link.h"
#include "shared/misc.h"
#include "shared/front.h"
#include "shared/action.h"
#include "shared/data_source.h"
#include "front/front_IA.h"
#include "expr.h"
#include "expr_term.h"
#include "arglist.h"
#include "type_funcdef.h"
#include "cc.h"

std::string ESimplifyOutcome2string(ESimplifyOutcome e)
{
	std::string s;
#define SIMPL(p) case (SIMPL_##p): s = #p; break;
    switch (e){
		SIMPL(NULL);
		SIMPL(ADD1); SIMPL(ADD2); SIMPL(ADD3); SIMPL(ADD4); SIMPL(ADD5); SIMPL(ADD6);
		SIMPL(AND1);
		SIMPL(ARR1); SIMPL(ARR2);
		SIMPL(ASS1); SIMPL(ASS2); SIMPL(ASS3); SIMPL(ASS4); SIMPL(ASS5); SIMPL(ASS6); SIMPL(ASS7); SIMPL(ASS8); SIMPL(ASS9);
		SIMPL(BIN1); SIMPL(BIN2);
		SIMPL(CAL0); SIMPL(CAL1); SIMPL(CAL2);  SIMPL(CAL3); SIMPL(CAL4); SIMPL(CAL8); SIMPL(CAL9);
		SIMPL(CHK1); SIMPL(CHK2); SIMPL(CHK3); SIMPL(CHK4); SIMPL(CHK5);
		SIMPL(CHS1); SIMPL(CHS2); SIMPL(CHS3); SIMPL(CHS4); SIMPL(CHS5); SIMPL(CHS6); SIMPL(CHS7); SIMPL(CHS8); SIMPL(CHS9);
		SIMPL(CMP1);
		SIMPL(CND1);
		SIMPL(COM1);
		SIMPL(DAT1); SIMPL(DAT2); SIMPL(DAT3); SIMPL(DAT4); SIMPL(DAT5); SIMPL(DAT6); SIMPL(DAT7); SIMPL(DAT8); SIMPL(DAT9);
		SIMPL(DIV1); SIMPL(DIV2); SIMPL(DIV3); SIMPL(DIV4); SIMPL(DIV5); SIMPL(DIV6); SIMPL(DIV7);
		SIMPL(EXP1); SIMPL(EXP2); SIMPL(EXP3); SIMPL(EXP4); SIMPL(EXP5); SIMPL(EXP6); SIMPL(EXP7); SIMPL(EXP8); SIMPL(EXP9);
		SIMPL(EXT1); SIMPL(EXT2); SIMPL(EXT3); SIMPL(EXT4); SIMPL(EXT5); SIMPL(EXT6);
		SIMPL(FRB1); SIMPL(FRB2); SIMPL(FRB3);
		SIMPL(FRA1); SIMPL(FRA2); SIMPL(FRA3); SIMPL(FRA4); SIMPL(FRA5); SIMPL(FRA6); SIMPL(FRA7); SIMPL(FRA8); SIMPL(FRA9);
		SIMPL(GET1); SIMPL(GET2); SIMPL(GET3);
		SIMPL(HAL1);
		SIMPL(HIG1); SIMPL(HIG2);
		SIMPL(IMM1); SIMPL(IMM2); SIMPL(IMM3); SIMPL(IMM4); SIMPL(IMM5); SIMPL(IMM6);
		SIMPL(IND1); SIMPL(IND2); SIMPL(IND3); SIMPL(IND4); SIMPL(IND5); SIMPL(IND6); SIMPL(IND7);
		SIMPL(LOG1); SIMPL(LOG2); SIMPL(LOG3); SIMPL(LOG4); SIMPL(LOG5); SIMPL(LOG6); SIMPL(LOG7);
		SIMPL(LOH1); SIMPL(LOH2);
		SIMPL(MUL1); SIMPL(MUL2); SIMPL(MUL3); SIMPL(MUL4); SIMPL(MUL5); SIMPL(MUL6); SIMPL(MUL7); SIMPL(MUL8);
		SIMPL(NEG1); SIMPL(NEG2); SIMPL(NEG3); SIMPL(NEG4); SIMPL(NEG5); SIMPL(NEG6); SIMPL(NEG7);
		SIMPL(OFF1); SIMPL(OFF2); SIMPL(OFF3); SIMPL(OFF4);
		SIMPL(OPD1);
		SIMPL(PRE1); SIMPL(PRE2); SIMPL(PRE3);
		SIMPL(PST1); SIMPL(PST2); SIMPL(PST3); SIMPL(PST4); SIMPL(PST5); SIMPL(PST6); SIMPL(PST7); SIMPL(PST8); SIMPL(PST9);
			SIMPL(PSTA); SIMPL(PSTB); SIMPL(PSTC); SIMPL(PSTD); SIMPL(PSTE); SIMPL(PSTF); SIMPL(PSTG); SIMPL(PSTH); SIMPL(PSTI); SIMPL(PSTJ); SIMPL(PSTK);
		SIMPL(PTR1); SIMPL(PTR2); SIMPL(PTR3); SIMPL(PTR4); SIMPL(PTR5);  SIMPL(PTR6); SIMPL(PTR7);
		SIMPL(REC1);
		SIMPL(RET1); SIMPL(RET2);
		SIMPL(ROW1);
		SIMPL(SHL1); SIMPL(SHL2);
		SIMPL(SHR1); SIMPL(SHR2);
		SIMPL(SUB1); SIMPL(SUB2); SIMPL(SUB3); SIMPL(SUB4); SIMPL(SUB5); SIMPL(SUB6);
		SIMPL(SWA1); SIMPL(SWA2); SIMPL(SWA3); SIMPL(SWA4); SIMPL(SWA5);
		SIMPL(THS1); SIMPL(THS2); SIMPL(THS3); SIMPL(THS4); SIMPL(THS5); SIMPL(THS6); SIMPL(THS7);
		SIMPL(TYP1); SIMPL(TYP2); SIMPL(TYP3); SIMPL(TYP4); SIMPL(TYP5); SIMPL(TYP6); SIMPL(TYP7); SIMPL(TYP8); SIMPL(TYP9); 
			SIMPL(TY10); SIMPL(TY11); SIMPL(TY12); SIMPL(TY13); SIMPL(TY14); SIMPL(TY15); SIMPL(TY16); SIMPL(TY17); SIMPL(TY18); SIMPL(TY19);
			SIMPL(TY20); SIMPL(TY21); SIMPL(TY22);
		SIMPL(UNI1);
		SIMPL(UNK1); SIMPL(UNK2);
		SIMPL(VAR1); SIMPL(VAR2); SIMPL(VAR3);
		SIMPL(ZER1); SIMPL(ZER2); SIMPL(ZER3); SIMPL(ZER4); SIMPL(ZER5); SIMPL(ZER6);
    }
#undef SIMPL
	return s;
}

int	EXPRSimpl_t::SimplifyLogic(Out_t * pSelf) const
{
	if (pSelf->isComparAction())
	{
		if (pSelf->mpU->is(ACTN_LOGNOT))
		{
			pSelf->mAction = (Action_t)(pSelf->mAction ^ 1);
			DetachParent(pSelf);
			return RETURN_1(LOG1);
		}
	}
	else if (pSelf->is(ACTN_LOGOR))
	{
		//!(a||b) => ((!a)&&(!b))
		if (pSelf->mpU->is(ACTN_LOGNOT))
		{
			pSelf->mAction = ACTN_LOGAND;
			InsertParent(pSelf->mpL, ACTN_LOGNOT);
			InsertParent(pSelf->mpR, ACTN_LOGNOT);
			DetachParent(pSelf);
			return RETURN_1(LOG2);
		}
	}
	else if (pSelf->is(ACTN_LOGAND))
	{
		//!(a&&b) => ((!a)||(!b))
		if (pSelf->mpU->is(ACTN_LOGNOT))
		{
			pSelf->mAction = ACTN_LOGOR;
			InsertParent(pSelf->mpL, ACTN_LOGNOT);
			InsertParent(pSelf->mpR, ACTN_LOGNOT);
			DetachParent(pSelf);
			return RETURN_1(LOG3);
		}

/*		//ZERO(a) && ZERO(b) => ZERO(a && b)
		if (pSelf->mpL->is(ACTN_ZERO) || mpL->is(ACTN_NZERO))
		if (pSelf->mpR->is(pSelf->mpL->mAction))
		if (!pSelf->mpL->mpR->is(ACTN_GET) && !pSelf->mpR->mpR->is(ACTN_GET))
		if (!pSelf->mpL->mpR->is(ACTN_CHECK) && !pSelf->mpR->mpR->is(ACTN_CHECK))
		{
			pSelf->InsertParent(mpL->mAction);
			pSelf->mpL->mpR->DetachParent();
			pSelf->mpR->mpR->DetachParent();
			return 1;
		}

		if (pSelf->mpL->is(ACTN_SUB) || ppSelf->OutL->is(ACTN_ADD))
		if (pSelf->mpR->is(mpL->mAction))
		{
			Out_t *pOut1(NewTerm());
			if (pOut1->Unite(mpL->mpL, mpR->mpL))
			{
				Out_t *pOut2(NewTerm());
				if (pOut2->Unite(pSelf->mpL->mpR, pSelf->mpR->mpR))
				{
					return 1;
				}
			}
		}*/
	}
	else if (pSelf->is(ACTN_LOGNOT))
	{
		//!!a => a
		if (pSelf->mpU->is(ACTN_LOGNOT))
		{
			DetachParent(pSelf);
			DetachParent(pSelf->mpR);
			return RETURN_1(LOG4);
		}
	}
	return 0;
}

int	EXPRSimpl_t::SimplifyLogic2(Out_t * pSelf) const
{
	if (pSelf->is(ACTN__EQUAL))
	{
		//if(a==0) => if(!a)
		if (pSelf->mpL->isNumZero())
			pSelf->flipChilds();

		if (pSelf->mpR->isNumZero())
		if (pSelf->mpU->isTop() || ISLOGIC(pSelf->mpU->mAction) || pSelf->mpU->is(ACTN_GOTO))
		if (!pSelf->mpL->is(ACTN_INDIR))//?
		{
			Out_t *pOut = pSelf->mpL;
			DetachParent(pOut);
			InsertParent(pOut, ACTN_LOGNOT);
			return RETURN_1(LOG5);
		}
	}
	else if (pSelf->is(ACTN__NOTEQUAL))
	{
		//if(a!=0) => if(a)
		if (pSelf->mpL->isNumZero())
			pSelf->flipChilds();

		if (pSelf->mpR->isNumZero())
		if (pSelf->mpU->isTop() || ISLOGIC(pSelf->mpU->mAction) || pSelf->mpU->is(ACTN_GOTO))
		if (!pSelf->mpL->is(ACTN_INDIR))//?
		{
			Out_t *pOut = pSelf->mpL;
			DetachParent(pOut);
			return RETURN_1(LOG6);
		}
	}
	else if (pSelf->is(ACTN_QUERY))
	{
		if (pSelf->mpU->isTop() || ISLOGIC(pSelf->mpU->mAction))
		{
			Out_t *pOutT = pSelf->mpR->mpL;
			Out_t *pOutF = pSelf->mpR->mpR;
			if (!pOutT->isNumZero() && pOutF->isNumZero())
			{
				DetachParent(pSelf->mpL);
				return RETURN_1(LOG7);
			}
		}
	}

	return 0;
}

int	EXPRSimpl_t::SimplifyLogicPost(Out_t * pSelf) const
{
	if (pSelf->is(ACTN_SEMI))
	{
		if (pSelf->mpL->is(ACTN_VAR))
		{
			if (pSelf->mpR->is(ACTN_VAR))
			{
				// var a; var b; => var a, b; | {typeof(a) == typeof(b)}
				if (TypOf(pSelf->mpL) == TypOf(pSelf->mpR))
				{
					Out_t *pOutVar(DetachParent(pSelf->mpL->mpR));
					DetachParent(pSelf->mpR->mpR);
					pSelf->mAction = ACTN_COMMA;
					InsertParent(pSelf, pOutVar);
					return RETURN_1(VAR1);
				}
			}
			/*else
			{
				Out_t *pOutVar(pSelf->mpL->mpR->DetachParent());
				InsertParent(pSelf, pOutVar);
				return RETURN_1(VAR2);
			}*/
		}
	}
	return 0;
}

int EXPRSimpl_t::SimplifyIndir2(Out_t* pSelf) const
{
	// *&OBJ => OBJ
	if (pSelf->is(ACTN_INDIR))
	{
		if (pSelf->mpR->is(ACTN_OFFS))
		{
			Out_t* pOut = pSelf->mpR->mpR;
			/*?			if (pOut->isOpKind())
						{
							OpPtr pOp = pOut->m_p Op0;
							//if (pOp->m_ pMLoc->IsD ata())
							if (pOp->IsDataOp())
							{
								//pOp = pOp->m_pData->m_p Ops;
								FieldPtr pField = (FieldPtr )pOp;
								if (pField->IsComplex0())
									return 0;
							}
							else if (pOp->IsCodeOp())
							{
								if (pOp->m_ pMLoc)
								{
			//						if (pOp->m_pM Loc->IsD ata())
			//						if (pOp->m_pData->isComplex())
			//							return 0;
								}
							}
						}
			*/			DetachParent(pOut);//ACTN_OFFS
			DetachParent(pOut);//ACTN_INDIR
			if (TypOf(pOut).isArray())
			{
				Out_t* pArray = InsertParent(pOut, ACTN_ARRAY);
				Add(*pArray, HOP(), (int)0);
			}
			return RETURN_1(PTR3);
		}
	}

	return 0;
}

int EXPRSimpl_t::SimplifyOffs(Out_t *pSelf) const
{
	//*(type*)&OBJ => (type*)OBJ

	//(&*PTR) => PTR
	if (!pSelf->is(ACTN_OFFS))
		return 0;

	if (pSelf->mpR->is(ACTN_INDIR))
	{
		DetachParent(pSelf->mpR->mpR);
		DetachParent(pSelf->mpR);
		return RETURN_1(PTR4);
	}

	if (pSelf->mpU->is(ACTN_TYPE))
	{
		// *(type*)&OBJ => OBJ
		if (pSelf->mpU->mpU->is(ACTN_INDIR))
		{
			Out_t* pIndir(pSelf->mpU->mpU);

			TYP_t T(TypOf(pSelf));
			TYP_t T2(ProxyOf(pIndir));
			//if (AreCompliant(Tproxy, TypOf(pIndir)))

			if ((T == T2) || T.isImplicitCastOf(T2))
				//if (AreCompliant(T, TypOf(pIndir)))
			{
				DetachParent(pSelf);//ACTN_TYPE
		//		SetTyp(pIndir, T);
				return RETURN_1(PTR6);
			}
		}
	}

	// &OBJ => OBJ		| typef(OBJ) is array (array ref decays to ptr)
	if (TypOf(pSelf->mpR).isArray())
	{
		TYP_t T(TypOf(pSelf->mpR).arrayBase());
		pSelf->setTyp(PtrOf(T));
		DetachParent(pSelf->mpR);
		return RETURN_1(OFF4);
	}

	return 0;
}

int EXPRSimpl_t::SimplifyPtr2(Out_t *pSelf) const
{
	// PSTRUC->(*PFUNC)() => PSTRUC->*PFUNC()
/*	if (pSelf->IsCall())
	if (mpU->is(ACTN_PTR))
	if (mpL->is(ACTN_INDIR))
	if (!mpL->mpR->is(ACTN_OFFS))
	{
		mpL->mpR->DetachParent();
		mpU->mAction = ACTN_PTREX;
		return 1;
	}
*/
	if (pSelf->is(ACTN_PTR))
	{
		// (&OBJ1)->OBJ2 => OBJ1.OBJ2
		if (pSelf->mpL->is(ACTN_OFFS))
		{
			DetachParent(pSelf->mpL->mpR);
			pSelf->mAction = ACTN_DOT;
			return RETURN_1(PTR5);
		}

		if (pSelf->mpR->is(ACTN_INDIR))
			//	if (!mpL->mpR->is(ACTN_OFFS))
		{
			DetachParent(pSelf->mpR->mpR);
			pSelf->mAction = ACTN_PTREX;
			return RETURN_1(PTR2);
		}
	}

	// (&OBJ1)->*OBJ2 => OBJ1.*OBJ2
	if (pSelf->is(ACTN_PTREX))
	if (pSelf->mpL->is(ACTN_OFFS))
	{
		DetachParent(pSelf->mpL->mpR);
		pSelf->mAction = ACTN_DOTEX;
		return RETURN_1(OFF3);
	}

	return 0;
}

int EXPRSimpl_t::SimplifyEqu(Out_t* pSelf) const
{
	if (!pSelf->is(ACTN__EQUAL))
		if (!pSelf->is(ACTN__NOTEQUAL))
			return 0;

	//((-a) == 0) => (a == 0)
	//((-a) != 0) => (a != 0)
	if (pSelf->mpL->is(ACTN_CHS))
	{
		if (pSelf->mpR->isNumZero())
		{
			DetachParent(pSelf->mpL->mpR);//sign
			return RETURN_1(NEG3);
		}
	}

	return 0;
}

int EXPRSimpl_t::SimplifySign2(Out_t *pSelf) const//check sign
{
	if (pSelf->is(ACTN_CHS))
	{
		if (pSelf->mpR->is(ACTN_SUB))
		{
			//-(a-b) => (-a)+b => b-a
			pSelf->mpR->flipChilds();
			DetachParent(pSelf->mpR);//kill this
			return RETURN_1(NEG4);
		}
		
		if (pSelf->mpR->is(ACTN_ADD))
		{
			//-(a+b) => (-a)-b
			Out_t *pL = pSelf->mpR->mpL;
			pSelf->mpR->mAction = ACTN_SUB;
			DetachParent(pSelf->mpR);//kill this
			InsertParent(pL, ACTN_CHS);
			return RETURN_1(NEG5);
		}

		/*if (0)//(-a)+b => b-a
		if (pSelf->mpU->is(ACTN_ADD))
		if (pSelf->isLeft())
		{
			pSelf->mpU->mAction = ACTN_SUB;
			pSelf->mpU->flipChilds();
			return RETURN_1(NEG6);
		}*/

		//move down
		//(-(OBJ1*OBJ2)) => ((-OBJ1)*OBJ2)
		if (pSelf->mpR->is(ACTN_MUL) || pSelf->mpR->is(ACTN_DIV))
		{
			Out_t *pOut = pSelf->mpR;
			DetachParent(pOut);
			InsertParent(pOut->mpL, ACTN_CHS);
			return RETURN_1(NEG7);
		}
	}
	
	return 0;
}

/*int EXPRSimpl_t::ReplaceReal80Types(Out_t* pSelf) const
{
	if (!pSelf->is(ACTN_TYPE))
		return 0;
	if (TypOf(pSelf).optyp() != OPTYP_REAL80)
		return 0;
	pSelf->setTyp(fromOpType(OPTYP_REAL64));
	return RETURN_1(TYP0);
}*/

#define FUCK	1

//00:
//((a+b)+c):11	=>	(a+(b+c)):11
//((a-b)+c):10	=>	(a-(b-c)):00
//((a+b)-c):01	=>	(a+(b-c)):10
//((a-b)-c):00	=>	(a-(b+c)):01
//01:
//((b+a)+c):11	=>	((b+c)+a):11
//((b-a)+c):10	=>	((b+c)-a):01
//((b+a)-c):01	=>	((b-c)+a):10
//((b-a)-c):00	=>	((b-c)-a):00
//02:
//(c+(a+b)):11	=>	(a+(b+c)):11
//(c+(a-b)):10	=>	(a+(b-c)):10
//(c-(a+b)):01	=>	-(a-(b-c)):00*
//(c-(a-b)):00	=>	-(a-(b+c)):01*
//03:
//(c+(b+a)):11	=>	((c+b)+a):11
//(c+(b-a)):10	=>	((c+b)-a):01
//(c-(b+a)):01	=>	((c-b)-a):00
//(c-(b-a)):00	=>	((c-b)+a):10

int EXPRSimpl_t::MoveHigher(Out_t &rSelf) const//move (a) one level higher
{
	Out_t *pU = rSelf.mpU;
	if (!pU->is(ACTN_ADD))
		if (!pU->is(ACTN_SUB))
			return 0;

	Out_t *pUU = pU->mpU;
	if (!pUU->is(ACTN_ADD))
		if (!pUU->is(ACTN_SUB))
			return 0;

	int x[4][4] = {
		{1,2,0,3},
		{0,2,1,3},
		{5,4,2,3},
		{2,0,1,3}
	};

	int j = 0;
	if (rSelf.isRight())
		j |= 1;
	if (pU->isRight())
		j |= 2;

	int i = 0;
	if (pU->is(ACTN_ADD))
		i |= 1;
	if (pUU->is(ACTN_ADD))
		i |= 2;

	Action_t A1 = ((x[j][i]&1)?(ACTN_ADD):(ACTN_SUB));
	Action_t A2 = ((x[j][i]&2)?(ACTN_ADD):(ACTN_SUB));
	bool sign = ((x[j][i]&4)?(1):(0));

	TYP_t Ttop(TypOf(pUU));
	Out_t* pUold(DetachParent(rSelf.sibling()));
	pUU->mAction = A1;
	pUU->clearTyp();//went down
	Out_t *pOutA(InsertParent(pUU, A2));
	Add(*pOutA, rSelf);
	pOutA->setTyp(Ttop);//went up

	if (j&1)//initialy was right
	{
		if (!rSelf.isRight())
			rSelf.mpU->flipChilds();
	}
	else
	{
		if (!rSelf.isLeft())
			rSelf.mpU->flipChilds();
	}

	if (sign)
		InsertParent(pOutA, ACTN_CHS);

	return 1;
}

int Out_t::isImmidiateEx() const
{
	if (isImmidiate() && !isNumZero())
		return 1;
	if (isOpKind())
		if (mSsid == (OPC_ADDRESS|SSID_LOCAL))
			return 1;
	return 0;
}

int Out_t::CheckEqualedD(Out_t *pOut)
{
	if (!is(ACTN_ADD))
		if (!is(ACTN_SUB))
		{
//			if (isImmidiate() && !isNumZero())
//			if (pOut->isImmidiate() && !pOut->isNumZero())
			if (isImmidiateEx() && pOut->isImmidiateEx())
				return 1;
			if (isEqualTo(pOut))
				return 1;
			return 0;
		}

	if (mpL->CheckEqualedD(pOut))//U
		return 1;
	if (mpR->CheckEqualedD(pOut))//U
		return 1;

	return 0;
}

int Out_t::CheckEqualedU(Out_t *pOut)
{
	if (!mpU->is(ACTN_ADD))
		if (!mpU->is(ACTN_SUB))
			return 0;

	//check sibling first
	if (sibling()->CheckEqualedD(pOut))
		return 2;//there is an equal on the sibling branch

	if (mpU->CheckEqualedU(pOut))
		return 1;

	return 0;
}

int EXPRSimpl_t::SimplifyS0(Out_t *pSelf) const
{
	if (!pSelf->mpU)
		return 0;

	if (!pSelf->isTerm())
		return 0;

	if (!pSelf->mpU->is(ACTN_ADD))
		if (!pSelf->mpU->is(ACTN_SUB))
			return 0;

	//check sibling first
	if (pSelf->sibling()->CheckEqualedD(pSelf))
		return 0;//there is an equal on siblings branch, so exit

	//check for equaled on higher branch
	int n = pSelf->mpU->CheckEqualedU(pSelf);
	if (n == 1)////equal on the parent's parent branch
	{
		if (MoveHigher(*pSelf))
			return RETURN_1(HIG1);
	}
	else if (n == 2)//equal on the parent's sibling branch
	{
		if (MoveHigher(*pSelf->sibling()))
			return RETURN_1(HIG2);
	}

	return 0;
}

int EXPRSimpl_t::SimplifySign(Out_t* pSelf) const
{
	if (!pSelf->is(ACTN_CHS))
		return 0;

	//-i1 => i2 | i2==i1*(-1)
	if (pSelf->mpR->isImmidiate())
	{
		pSelf->mpR->m_value.i32 *= -1;
		DetachParent(pSelf->mpR);
		return RETURN_1(CHS1);
	}

	//--a => a
	if (pSelf->mpR->is(ACTN_CHS))
	{
		DetachParent(pSelf->mpR->mpR);
		DetachParent(pSelf->mpR);//self
		return RETURN_1(CHS9);
	}

	/*if (pSelf->mpR->is(ACTN_SUB))
	{
		//-(a-b) => b-a
		pSelf->mpR->flipChilds();
		DetachParent(pSelf->mpR);//self
		return RETURN_1(NEG4);
	}*/

	return 0;
}

int EXPRSimpl_t::SimplifyAdd(Out_t* pSelf) const
{
	if (!pSelf->is(ACTN_ADD))
		return 0;

//CHECK(TypOf(pSelf->mpL).isPtr() && TypOf(pSelf->mpR).isPtr())
//STOP

	if (TypOf(pSelf->mpR).isPtr() && !TypOf(pSelf->mpL).isPtr())//if both are ptrs - this is ptr diff
	{
		pSelf->flipChilds();
		return RETURN_1(SWA3);
	}

	// children adds/subs must be found only on right side of expression
	//((a+-b)+c) => (a+-(b+-c))
	if (pSelf->mpL->isActionAddOrSub())
		if (!pSelf->mpL->m_bNoReduce)
			if (!pSelf->mpL->mpL->isActionAddOrSub())//will work from bottom to top
			{
				MoveHigher(*pSelf->mpL->mpL);
				return RETURN_1(ADD3);
			}
#if(0)
	//get add/sub ops aligned
	//((a+-b)+(c+-d)) => (a+-(b+-(c+-d)))
	if (pSelf->mpL->is(ACTN_ADD) || pSelf->mpL->is(ACTN_SUB))
		if (pSelf->mpR->is(ACTN_ADD) || pSelf->mpR->is(ACTN_SUB))
		{
			if (MoveHigher(*pSelf->mpL->mpL))
				return RETURN_1(ADD3);
		}
#endif

	// move immidiate down
	// (i+(a+-b)) => (a+(i+-b))
	if (pSelf->mpL->isImmidiate() && !TypOf(pSelf->mpL).isPtr())
		if (pSelf->mpR->isActionAddOrSub())
			if (!pSelf->mpR->mpL->isImmidiate())
			{
				if (MoveHigher(*pSelf->mpR->mpL))
					return RETURN_1(ADD6);
			}

	if (pSelf->mpL->isImmidiate())
	{
		if (pSelf->mpR->isImmidiate())
		{
			//I1+I2 => I3 | I3 == I1+I2
			Detach(pSelf);
			int n = pSelf->mpL->m_value.i32 + pSelf->mpR->m_value.i32;
			Out_t* pOut = Add(*pSelf->mpU, pSelf->dockOp(), n);
			return RETURN_1(ADD1);
		}

		if (!pSelf->mpR->isActionAddOrSub())//otherwise will conflict with ADD3
			if (!TypOf(pSelf->mpL).isPtr())
			{
				//i+a => a+i
				pSelf->flipChilds();
				return RETURN_1(SWA2);
			}

		/*			//0+OBJ -> OBJ
					if (mpL->isNumZero())
					{
						mpR->DetachParent();
						return 1;
					}*/
	}

	if (pSelf->mpR->isImmidiate())
	{
		//a+(-i) => a-i
		if (pSelf->mpR->Value().isInt())
			if (!pSelf->mpR->Value().isUnsignedInt())
				if (pSelf->mpR->m_value.i32 < 0 && pSelf->mpR->m_value.ui32 != 0x80000000)//multiplying by '-1' - the value remains the same!
				{
					pSelf->mpR->m_value.i32 *= -1;
					pSelf->mAction = ACTN_SUB;
					return RETURN_1(SUB1);
				}

		//&a+i => &b
		if (pSelf->mpL->isOpKind())
			if (pSelf->mpL->mSsid == (OPC_ADDRESS | SSID_LOCAL))
			{
				pSelf->mpL->mOffs += pSelf->mpR->m_value.i32;
				pSelf->mpL->setFieldRef(pSelf->mpL->fieldRef());// LocalRef(pSelf->mpL->mpOp));
				pSelf->mpL->mpOp = HOP();
				DetachParent(pSelf->mpL);
				return RETURN_1(OFF1);
			}

		//&(a)+i => &(b)
		if (pSelf->mpL->is(ACTN_OFFS))
			if (pSelf->mpL->mpR->isOpKind())
				if (!pSelf->mpL->mpR->mpOp)//?
				{
					pSelf->mpL->mpR->mOffs += pSelf->mpR->m_value.i32;
					DetachParent(pSelf->mpL);
					return RETURN_1(OFF2);
				}

#if(!FUCK)
		if (mpL->is(ACTN_ADD))
		{
			// ((OBJ+I2)+I1) => OBJ+I3 |I3=I2+I1
			if (mpL->mpR->isImmidiate())
			{
				mpL->mpR->m_value.i32 += mpR->m_value.i32;
				mpL->DetachParent();
				return 1;
			}
		}
		else if (mpL->is(ACTN_SUB))
		{
			// ((OBJ-I2)+I1) => OBJ-I3 |I3=I2-I1
			if (mpL->mpR->isImmidiate())
			{
				mpL->mpR->m_value.i32 -= mpR->m_value.i32;
				mpL->DetachParent();
				return 1;
			}
		}
#endif
	}

	//OBJ+0 -> OBJ
	if (pSelf->mpR->isNumZero())
	{
		DetachParent(pSelf->mpL);
		return RETURN_1(ZER1);
	}

	if (pSelf->mpR->isImmidiate())
	{
		if (pSelf->mpR->Value().isSignedInt())
			if (pSelf->mpR->m_value.i32 < 0)
			{
				pSelf->mAction = ACTN_SUB;
				pSelf->mpR->m_value.i32 *= -1;
				return RETURN_1(CHS3);
			}
#if(!FUCK)
		//move higher
		if (1)//((a+i)+b) => ((a+b)+i)
			if (1)//((a+i)-b) => ((a-b)+i)
				if (mpU->is(ACTN_ADD) || mpU->is(ACTN_SUB))
				{
					Out_t* pOut = mpU;
					mpL->DetachParent();//this
					pOut = pOut->InsertParent(ACTN_ADD);
					pOut->Add(mpR);
					return 1;
				}
#endif

		//			if (mpR->MoveHigher())
		//				return 1;
	}

	if (pSelf->mpL->is(ACTN_CHS))
	{
		//((-a)+b) -> -(a-b)
		pSelf->mAction = ACTN_SUB;
		InsertParent(pSelf, ACTN_CHS);
		DetachParent(pSelf->mpL->mpR);
		return RETURN_1(CHS5);
	}

	if (pSelf->mpR->is(ACTN_CHS))
	{
		//(a+(-b)) -> (a-b)
		pSelf->mAction = ACTN_SUB;
		DetachParent(pSelf->mpR->mpR);
		return RETURN_1(CHS6);
	}

	/*if (TypOf(pSelf->mpL).isPtr() ^ TypOf(pSelf->mpR).isPtr())
	{
		if (TypOf(pSelf->mpR).isPtr())
		{
			pSelf->flipChilds();
			return RETURN_1(SWA3);
		}

		pSelf->setTyp(PtrOf(nullptr));//?
	}*/

	return 0;
}

int EXPRSimpl_t::SimplifySub(Out_t* pSelf) const
{
	if (!pSelf->is(ACTN_SUB))
		return 0;

	/*if (TypOf(pSelf->mpR).isPtr() && !TypOf(pSelf->mpL).isPtr())//if both are ptrs - this is ptr diff
	{
		pSelf->flipChilds();
		return RETURN_1(SWA3);
	}*/

	// children adds/subs must be found only on right side of expression
	//((a+-b)-c) => (a+-(b+-c))
	if (pSelf->mpL->isActionAddOrSub())
		if (!pSelf->mpL->m_bNoReduce)
			if (!pSelf->mpL->mpL->isActionAddOrSub())//will work from bottom to top
			{
				MoveHigher(*pSelf->mpL->mpL);
				return RETURN_1(ADD3);
			}

#if(0)
	//get add/sub ops aligned
	//((a+-b)-(c+-d)) => (a+-(b+-(c+-d)))
	if (pSelf->mpL->is(ACTN_ADD) || pSelf->mpL->is(ACTN_SUB))
		if (pSelf->mpR->is(ACTN_ADD) || pSelf->mpR->is(ACTN_SUB))
		{
			if (MoveHigher(*pSelf->mpL->mpL))
				return RETURN_1(ADD3);
		}
#endif

	//0-a -> -a
	if (pSelf->mpL->isNumZero())
	{
		pSelf->mAction = ACTN_CHS;
		Detach(pSelf->mpL);
		return RETURN_1(SUB2);
	}

	//0-a => -a
	/*if (pSelf->mpL->isNumZero())
	{
		Out_t* pOut = pSelf->mpR;
		DetachParent(pOut);
		Out_t* pOutSign = InsertParent(pOut, ACTN_CHS);
		TypOf(pOutSign) = TypOf(pSelf);
		return RETURN_1(ZER3);
	}*/

	//a-0 -> a
	if (pSelf->mpR->isNumZero())
	{
		Out_t* pL(pSelf->mpL);
		DetachParent(pL);
		return RETURN_1(SUB3);
	}

	//a-a => 0
	if (pSelf->mpL->isTerm())
		if (pSelf->mpL->isEqualTo(pSelf->mpR))
		{
			Out_t* pU = pSelf->mpU;
			Detach(pSelf);
			Add(*pU, pSelf->dockOp(), (int)0);
			Dispose(pSelf);
			return RETURN_1(SUB4);
		}

#if(0)
	if (mpR->is(ACTN_CHS))
	{
		//a-(-b) => a+b
		mpR->mpR->DetachParent();
		mAction = ACTN_ADD;
		return 1;
	}
#endif
#if(!FUCK)
	else if (mpR->is(ACTN_ADD))
	{
		// ((a-(b+c)) => (a-b)-c
		Out_t* pOut = mpR->mpL;
		mpR->mpR->DetachParent();
		Out_t* pOutA = mpL->InsertParent(ACTN_SUB);
		pOutA->Add(pOut);
		return 1;
	}
	else
#endif
		if (pSelf->mpR->isComparAction0())
		{
			//a-CMPRSN(b) => a-(1-(!CMPRSN(b))) => ((a-1)+(!CMPRSN(b))) => (a+(!CMPRSN(b)))-1)
			pSelf->mpR->mAction = (Action_t)(pSelf->mpR->mAction ^ 1);//invert action
			pSelf->mAction = ACTN_ADD;
			Out_t* pSub = InsertParent(pSelf, ACTN_SUB);
			assert(pSelf->isLeft());
			Add(*pSub, HOP(), (int)1);
			return RETURN_1(CMP1);
		}

	// move immidiate down
	// (i-(a+-b)) => -(a+(i+-b))
	if (pSelf->mpL->isImmidiate())
		if (pSelf->mpR->isActionAddOrSub())
			if (!pSelf->mpR->mpL->isImmidiate())
			{
				if (MoveHigher(*pSelf->mpR->mpL))
					return RETURN_1(SUB6);
			}

	if (pSelf->mpR->isImmidiate())
	{
		//&a-i => &b
		if (pSelf->mpL->isOpKind())
			if (pSelf->mpL->mSsid == (OPC_ADDRESS | SSID_LOCAL))
			{
				pSelf->mpL->mOffs -= pSelf->mpR->m_value.i32;
				pSelf->mpL->mpOp = HOP();
				DetachParent(pSelf->mpL);
				return RETURN_1(IMM1);
			}

		//&(a)-i => &(b)
		if (pSelf->mpL->is(ACTN_OFFS))
			if (pSelf->mpL->mpR->isOpKind())
				if (!pSelf->mpL->mpR->mpOp)//?
				{
					pSelf->mpL->mpR->mOffs -= pSelf->mpR->m_value.i32;
					DetachParent(pSelf->mpL);
					return RETURN_1(IMM2);
				}

		//(a-(-i)) => a+i
		if (pSelf->mpR->Value().isInt())
			if (!pSelf->mpR->Value().isUnsignedInt())
				if (pSelf->mpR->m_value.i32 < 0)
				{
					pSelf->mpR->m_value.i32 *= -1;
					pSelf->mAction = ACTN_ADD;
					return RETURN_1(NEG1);
				}

#if(!FUCK)
		if (mpL->is(ACTN_SUB))
		{
			// ((OBJ-I2)-I1) => OBJ-I3 |I3=I2+I1
			if (mpL->mpR->isImmidiate())
			{
				mpL->mpR->m_value.i32 += mpR->m_value.i32;
				mpL->DetachParent();
				return 1;
			}
		}
		else if (mpL->is(ACTN_ADD))
		{
			// ((OBJ+I2)-I1) => OBJ+I3 |I3=I2-I1
			if (mpL->mpR->isImmidiate())
			{
				mpL->mpR->m_value.i32 -= mpR->m_value.i32;
				mpL->DetachParent();
				return 1;
			}
		}
#endif

		//I1-I2 => I3 | I3 == I1-I2
		if (pSelf->mpL->isImmidiate())
		{
			Detach(pSelf);
			int n = pSelf->mpL->m_value.i32 - pSelf->mpR->m_value.i32;
			Out_t* pOut = Add(*pSelf->mpU, HOP(), n);
			return RETURN_1(IMM3);
		}

		if (pSelf->mpR->isNumZero())
		{
			DetachParent(pSelf->mpL);
			return RETURN_1(ZER2);
		}

		/*			if (0)
					if (mpU)
					if (mpU->isComparAction0())
					if (mpU->mpR->isNumZero())
					{
						mpU->mpL = mpL;
						mpU->mpR = mpR;
						mpL->mpU = mpR->mpU = mpU;
						return 1;
					}*/

					//move higher
#if(!FUCK)
		if (1)
			if (mpU->is(ACTN_ADD))
			{
				//(a+(b-i)) => (a+b)-i
				Out_t* pOut = mpU;
				mpL->DetachParent();//this
				pOut = pOut->InsertParent(ACTN_SUB);
				pOut->Add(mpR);
				return 1;
			}
			else if (mpU->is(ACTN_SUB))
			{
				//1: (a-(b-i)) => ((a-b)+i)
				//2: ((a-i)-b) => ((a-b)-i)
				Action_t A = isLeft() ? (ACTN_SUB) : (ACTN_ADD);
				Out_t* pOut = mpU;
				mpL->DetachParent();//this
				pOut = pOut->InsertParent(A);
				pOut->Add(mpR);
				return 1;
			}
#endif
		//			if (mpR->MoveHigher())
		//				return 1;

	}

	//((-a)-b) -> -(a+b)
	if (pSelf->mpL->is(ACTN_CHS))
	{
		pSelf->mAction = ACTN_ADD;
		InsertParent(pSelf, ACTN_CHS);
		DetachParent(pSelf->mpL->mpR);
		return RETURN_1(CHS7);
	}

	//(a-(-b)) -> (a+b)
	if (pSelf->mpR->is(ACTN_CHS))
	{
		pSelf->mAction = ACTN_ADD;
		DetachParent(pSelf->mpR->mpR);
		return RETURN_1(CHS8);
	}

	return 0;
}

int EXPRSimpl_t::SimplifyMultiply(Out_t* pSelf) const
{
	if (!pSelf->is(ACTN_MUL))
		return 0;

	if (pSelf->mpL->isImmidiate())
	{
		//1*OBJ => OBJ
		if (pSelf->mpL->m_value.i32 == 1)
		{
			DetachParent(pSelf->mpR);
			return RETURN_1(MUL1);
		}
	}

	if (pSelf->mpR->isImmidiate())
	{
		//OBJ*1 => OBJ
		if (pSelf->mpR->m_value.i32 == 1)
		{
			DetachParent(pSelf->mpL);
			return RETURN_1(MUL2);
		}

		//OBJ*1 => OBJ
		if (pSelf->mpR->m_value.i32 == 1)
		{
			DetachParent(pSelf->mpL);
			return RETURN_1(MUL3);
		}


		if (pSelf->mpL->isImmidiate())
		{
			//I1*I2 => I3 | I3=I1*I2
			pSelf->mpL->m_value.i32 *= pSelf->mpR->m_value.i32;
			DetachParent(pSelf->mpL);
			return RETURN_1(IMM4);
		}

		//((OBJ*I1)*I2) => (OBJ*I3) |I3=I1*I2
		if (pSelf->mpL->is(ACTN_MUL))
			if (pSelf->mpL->mpR->isImmidiate())
			{
				pSelf->mpL->mpR->m_value.i32 *= pSelf->mpR->m_value.i32;
				DetachParent(pSelf->mpL);
				return RETURN_1(MUL4);
			}

		//((OBJ/I1)*I2) => (OBJ*I3) |I3=I1*I2
		if (pSelf->mpL->is(ACTN_DIV))
			if (pSelf->mpL->mpR->isImmidiate())
			{
				int d1 = pSelf->mpR->m_value.i32;
				int d2 = pSelf->mpL->mpR->m_value.i32;
				div_t d;
				d = div(d1, d2);
				if (d.rem == 0)
				{
					DetachParent(pSelf->mpL->mpL);
					pSelf->mpR->m_value.i32 = d.quot;
					return RETURN_1(DIV1);
				}
			}

		/*			//(OBJ1+OBJ2)*I => OBJ1*I+OBJ2*I
					if (mpL->is(ACTN_ADD) || mpL->is(ACTN_SUB))
					if (1)
					{
						Out_t *pL = mpL;
						OpPtr p = mpR->m_pOp;

						Out_t *pOut = pL->mpL->InsertParent(ACTN_MUL);
						pOut->Add(p);

						pL->DetachParent();//Detach
						pOut = pL->mpR->InsertParent(ACTN_MUL);
						pOut->Add(p);
						return 1;
					}
		*/
		if (pSelf->mpU)
		{
			if (pSelf->mpU->is(ACTN_ADD))
			{//(OBJ+(OBJ*I1)) => OBJ*I2 | I2=I1+1
				if (pSelf->mpU->mpR == pSelf)
					if (pSelf->mpL->isEqualTo(pSelf->mpU->mpL))
					{
						DetachParent(pSelf);
						pSelf->mpR->m_value.i32++;
						return RETURN_1(ADD2);
					}
			}
			else if (pSelf->mpU->is(ACTN_SUB))
			{//((OBJ*I1)-OBJ) => OBJ*I2 | I2=I1-1
				if (pSelf->mpU->mpL == pSelf)
					if (pSelf->mpL->isEqualTo(pSelf->mpU->mpR))
					{
						DetachParent(pSelf);
						pSelf->mpR->m_value.i32--;
						return RETURN_1(SUB5);
					}
			}
		}

		//(OBJ1+OBJ2)*I => OBJ1*I+OBJ2*I		(open braces)
		if (pSelf->mpL->is(ACTN_ADD) || pSelf->mpL->is(ACTN_SUB))
		{
			Out_t* pL = pSelf->mpL;
			Out_t* pI = pSelf->mpR;

			Out_t* pOut = InsertParent(pL->mpL, ACTN_MUL);
			Add(*pOut, *pI, true);

			DetachParent(pL);//Detach
			pOut = InsertParent(pL->mpR, ACTN_MUL);
			Add(*pOut, *pI, true);
			return RETURN_1(IMM5);
		}
	}

	if (pSelf->mpL->is(ACTN_CHS))
	{
		//((-a)*b) => -(a*b)
		InsertParent(pSelf, ACTN_CHS);
		DetachParent(pSelf->mpL->mpR);//sign
		return RETURN_1(MUL7);
	}

	if (pSelf->mpR->is(ACTN_CHS))
	{
		//(a*(-b)) => -(a*b)
		InsertParent(pSelf, ACTN_CHS);
		DetachParent(pSelf->mpR->mpR);//sign
		return RETURN_1(MUL8);
	}

	return 0;
}

int EXPRSimpl_t::SimplifyDivide(Out_t* pSelf) const
{
	if (!pSelf->is(ACTN_DIV))
		return 0;

	/*if(0)
	if (mpU->is(ACTN_TYPE))
	{
		Out_t *pType = DetachParent();
		Out_t *pTypeL = mpL->InsertParent(ACTN_TYPE);
		TypOf(pTypeL)  = TypOf(pType);
		Out_t *pTypeR = mpR->InsertParent(ACTN_TYPE);
		TypOf(pTypeR)  = TypOf(pType);
		return 1;
	}*/

	//OBJ1*(1/OBJ2) => OBJ1/OBJ2
	if (pSelf->mpL->isImmidiate())
		if (pSelf->mpL->m_value.i32 == 1)
			if (pSelf->mpU->is(ACTN_MUL))
			{
				Out_t* pOutS = pSelf->sibling();
				DetachParent(pSelf);
				Detach(pSelf->mpL);
				Add(*pSelf, *pOutS);
				return RETURN_1(MUL5);
			}

	//((a/b)/c) => (a/(b*c))
	if (pSelf->mpL->is(ACTN_DIV))
	{
		Out_t* pOutB = pSelf->mpL->mpR;
		Out_t* pOutC = pSelf->mpR;
		DetachParent(pSelf->mpL);
		Out_t* pOutMul = InsertParent(pOutB, ACTN_MUL);
		Add(*pOutMul, *pOutC);
		return RETURN_1(DIV2);
	}

	//(a/(b/c)) => ((a*c)/b)
	if (pSelf->mpR->is(ACTN_DIV))
	{
		Out_t* pOutA = pSelf->mpL;
		Out_t* pOutB = pSelf->mpR->mpL;
		Out_t* pOutC = pSelf->mpR->mpR;
		DetachParent(pOutB);
		Out_t* pOutMul = InsertParent(pOutA, ACTN_MUL);
		Add(*pOutMul, *pOutC);
		return RETURN_1(DIV3);
	}

	if (pSelf->mpR->isImmidiate())
	{
		//OBJ/1 = > OBJ
		if (pSelf->mpR->m_value.i32 == 1)
		{
			DetachParent(pSelf->mpL);
			return RETURN_1(DIV4);
		}

		//OBJ/IMM
		if (!pSelf->mpR->isNumZero())
			if (DivideWith(*pSelf->mpL, pSelf->mpR->m_value.i32))
			{
				DetachParent(pSelf->mpL);
				return RETURN_1(DIV5);
			}
	}

	//((-a)/b) => -(a/b)
	if (pSelf->mpL->is(ACTN_CHS))
	{
		InsertParent(pSelf, ACTN_CHS);
		DetachParent(pSelf->mpL->mpR);//sign
		return RETURN_1(DIV6);
	}

	//(a/(-b)) => -(a/b)
	if (pSelf->mpR->is(ACTN_CHS))
	{
		InsertParent(pSelf, ACTN_CHS);
		DetachParent(pSelf->mpR->mpR);//this
		return RETURN_1(DIV7);
	}

	return 0;
}

int EXPRSimpl_t::SimplifyShiftLeft(Out_t* pSelf) const
{
	if (pSelf->is(ACTN_SHL))
	{
		if (pSelf->mpR->isImmidiate())
		{
			/*			int nCount = mpR->m_value.i32;
						if (nCount < 32)
						{
							mAction = ACTN_MUL;
							mpR->m_pOp->i32 = 1;
							for (int i = 0; i < nCount; i++)
								mpR->m_value.i32 *= 2;
							return 1;
						}
			*/
			if (!pSelf->mpL->isTerm())
			{
				if (pSelf->mpL->is(ACTN_MUL))
				{
					// ((OBJ*I1)<<I2) => (OBJ*I3) |I3=I1*(2^I2)
					if (pSelf->mpL->mpR->isImmidiate())
					{
						for (int i = 0; i < pSelf->mpR->m_value.i32; i++)
							pSelf->mpL->mpR->m_value.i32 *= 2;
						DetachParent(pSelf->mpL);
						return RETURN_1(SHL1);
					}
				}
			}
			else
			{//assert(false);
				// (I1<<I2) => (I3) |I3=I1*(2^I2)
				if (pSelf->mpL->isImmidiate())
				{
					pSelf->mpL->m_value.i32 <<= pSelf->mpR->m_value.i32;
					DetachParent(pSelf->mpL);
					return RETURN_1(SHL2);
				}
			}
		}
	}
	return 0;
}

int EXPRSimpl_t::SimplifyShiftRight(Out_t* pSelf) const
{
	if (pSelf->is(ACTN_SHR))
	{
		// ((OP>>0x1F)<<0x20)+OP >> SIGNEXTEND(OP){64}, OP{32}
		//if (mpL->GetType() == OPTYP_INT32)
		if (pSelf->mpR->isImmidiate())
			if (pSelf->mpR->m_value.i32 == 0x1F)
			{
				if (pSelf->mpU->is(ACTN_MUL))
					if (pSelf->mpU->mpL == pSelf)
						if (pSelf->mpU->mpR->isImmidiate())
							if (pSelf->mpU->mpR->m_value.i64 == 0x100000000)
								if (pSelf->mpU->mpU->is(ACTN_ADD))
									if (pSelf->mpL->isEqualTo(pSelf->mpU->sibling()))
									{
										DetachParent(pSelf);
										DetachParent(pSelf);
										DetachParent(pSelf->mpL);
										return RETURN_1(SHR1);
									}
			}
			else
				//		if (mpL->isTerm())
				//		if (!(mpL->m_pOp->IsSignedInt())
				if (pSelf->mpR->m_value.i32 < 0x1F)
				{//assert(false);
					pSelf->mAction = ACTN_DIV;
					int nCount = pSelf->mpR->m_value.i32;
					pSelf->mpR->m_value.i64 = 1;
					pSelf->mpR->m_value.i64 <<= nCount;
					return RETURN_1(SHR2);
				}
	}
	return 0;
}

int EXPRSimpl_t::SimplifyBitwiseAnd(Out_t * pSelf) const
{
	if (pSelf->is(ACTN_LOHALF))
	{
		return 0;
		//(hi(OBJ)<<0x20)+lo(OBJ) => OBJ
		/*if (mpU->is(ACTN_ADD))
		{
			Out_t *pR = sibling();
			if (pR->is(ACTN_MUL))
			if (pR->mpL->is(ACTN_HIHALF))
			if (pR->mpR->isImmidiate())
			if (pR->mpR->m_value.i64 == 0x100000000)
			if (pR->mpL-mpR->isEqualTo(mpR))
			{
				mpR->DetachParent();
				mpR->DetachParent();
				return 1;
			}
		}*/
	}
	
	if (pSelf->is(ACTN_AND))
	{
		//i1&i2 => i3
		if (pSelf->mpL->isImmidiate())
		{
			if (pSelf->mpR->isImmidiate())
			{
				pSelf->mpL->m_value.i64 &= pSelf->mpR->m_value.i64;
				DetachParent(pSelf->mpL);
				return RETURN_1(AND1);
			}
		}
	}
	/*else if (pSelf->is(ACTN_AND) || pSelf->is(ACTN_OR))
	{
		if (0)
		if (mpU->is(ACTN_TYPE))
		{
			Out_t *pType = DetachParent();
			Out_t *pTypeL = mpL->InsertParent(ACTN_TYPE);
			TypOf(pTypeL)  = TypOf(pType);
			Out_t *pTypeR = mpR->InsertParent(ACTN_TYPE);
			TypOf(pTypeR)  = TypOf(pType);
			return 1;
		}
	}*/

#if(0)
#if(FUCK)
	if (pSelf->isTerm())
	{
		if (SimplifyS0(pSelf))
			return 1;
	}
#endif
#endif
	return 0;
}

int EXPRSimpl_t::DivideWith(Out_t &rSelf, int n, int *pRem) const
{
	assert(n);
	if (rSelf.isTerm())
	{
		if (rSelf.isImmidiate())
		{
			div_t d = div((int)rSelf.m_value.i32, n);
			if (d.rem == 0)
			{
				rSelf.m_value.i32 = d.quot;
				return 1;
			}
			else if (pRem)
			{
				rSelf.m_value.i32 = d.quot;
				*pRem = d.rem;
				return 1;
			}
		}
		return 0;
	}

	if (rSelf.is(ACTN_MUL))
	{
		if (DivideWith(*rSelf.mpL, n) || DivideWith(*rSelf.mpR, n))
			return 1;
	}
//	else if (is(ACTN_CHS))
//	{
//		if (mpR->DivideWith(n))
//			return 1;
//	}
	else if (rSelf.is(ACTN_ADD) || rSelf.is(ACTN_SUB))
	{
		if (!DivideWith(*rSelf.mpL, n))
		{
			if (!DivideWith(*rSelf.mpR, n))
				return 0;

			Out_t *pDiv = InsertParent(rSelf.mpL, ACTN_DIV);
			Add(*pDiv, HOP(), n);
			return 1;
		}
		if (!DivideWith(*rSelf.mpR, n))
		{
			Out_t *pDiv = InsertParent(rSelf.mpR, ACTN_DIV);
			Add(*pDiv, HOP(), n);
		}
		return 1;
	}

	return 0;
}

int EXPRSimpl_t::SimplifyCompMov(Out_t * pSelf) const
{
	if (IsUnfoldMode())
		return 0;
	//if (testf(adcui::DUMP_INDATA + adcui::DUMP_CASTS))
		//return 0;

	Out_t *mpL = pSelf->mpL;
	Out_t *mpR = pSelf->mpR;

	if (pSelf->is(ACTN_MOV))
	{
		if (!pSelf->mpU->is(ACTN_CHECK))//pre-condition?
		{
			if (mpR->is(ACTN_MUL))
			{
				// a=a*b => a*=b
				if (mpL->isEqualTo(mpR->mpL))
				{
					pSelf->mAction = ACTN_MOVMUL;
					DetachParent(mpR->mpR);
					return RETURN_1(PST1);
				}
				// a=b*a => a*=b
				if (mpL->isEqualTo(mpR->mpR))
				{
					pSelf->mAction = ACTN_MOVMUL;
					DetachParent(mpR->mpL);
					return RETURN_1(PST2);
				}
			}
			else if (mpR->is(ACTN_ADD))
			{
				// a=a+b => a+=b
				if (mpL->isEqualTo(mpR->mpL))
				{
					pSelf->mAction = ACTN_MOVADD;
					DetachParent(mpR->mpR);
					return RETURN_1(PST3);
				}
				// a=b+a => a+=b
				if (mpL->isEqualTo(mpR->mpR))
				{
					pSelf->mAction = ACTN_MOVADD;
					DetachParent(mpR->mpL);
					return RETURN_1(PST4);
				}
			}
			else if (mpR->is(ACTN_AND))
			{
				// a=a&b => a&=b
				if (mpL->isEqualTo(mpR->mpL))
				{
					pSelf->mAction = ACTN_MOVAND;
					DetachParent(mpR->mpR);
					return RETURN_1(PST5);
				}
				// a=b&a => a&=b
				if (mpL->isEqualTo(mpR->mpR))
				{
					pSelf->mAction = ACTN_MOVAND;
					DetachParent(mpR->mpL);
					return RETURN_1(PST6);
				}
			}
			else if (mpR->is(ACTN_OR))
			{
				// a=a|b => a|=b
				if (mpL->isEqualTo(mpR->mpL))
				{
					pSelf->mAction = ACTN_MOVOR;
					DetachParent(mpR->mpR);
					return RETURN_1(PST7);
				}
				// a=b|a => a|=b
				if (mpL->isEqualTo(mpR->mpR))
				{
					pSelf->mAction = ACTN_MOVOR;
					DetachParent(mpR->mpL);
					return RETURN_1(PST8);
				}
			}
			else if (mpR->is(ACTN_XOR))
			{
				// a=a^b => a^=b
				if (mpL->isEqualTo(mpR->mpL))
				{
					pSelf->mAction = ACTN_MOVXOR;
					DetachParent(mpR->mpR);
					return RETURN_1(PST9);
				}
				// a=b^a => a^=b
				if (mpL->isEqualTo(mpR->mpR))
				{
					pSelf->mAction = ACTN_MOVXOR;
					DetachParent(mpR->mpL);
					return RETURN_1(PSTA);
				}
			}
			else if (mpR->is(ACTN_DIV))
			{
				// a=a/b => a/=b
				if (mpL->isEqualTo(mpR->mpL))
				{
					pSelf->mAction = ACTN_MOVDIV;
					DetachParent(mpR->mpR);
					return RETURN_1(PSTB);
				}
			}
			else if (mpR->is(ACTN_MOD))
			{
				// a=a%b => a%=b
				if (mpL->isEqualTo(mpR->mpL))
				{
					pSelf->mAction = ACTN_MOVMOD;
					DetachParent(mpR->mpR);
					return RETURN_1(PSTC);
				}
			}
			else if (mpR->is(ACTN_SUB))
			{
				// a=a-b => a-=b
				if (mpL->isEqualTo(mpR->mpL))
				{
					pSelf->mAction = ACTN_MOVSUB;
					DetachParent(mpR->mpR);
					return RETURN_1(PSTD);
				}
			}
			else if (mpR->is(ACTN_SHL))
			{
				// a=a<<b => a<<=b
				if (mpL->isEqualTo(mpR->mpL))
				{
					pSelf->mAction = ACTN_MOVSHL;
					DetachParent(mpR->mpR);
					return RETURN_1(PSTE);
				}
			}
			else if (mpR->is(ACTN_SHR))
			{
				// a=a>>b => a>>=b
				if (mpL->isEqualTo(mpR->mpL))
				{
					pSelf->mAction = ACTN_MOVSHR;
					DetachParent(mpR->mpR);
					return RETURN_1(PSTF);
				}
			}
		}
		else// if(pSelf->mpU->checkAction(ACTN_CHECK))
		{
			if (mpR->is(ACTN_ADD))
			{
				if (mpR->mpR->isNumUnit())
				{
					// csw~a=a+1 => csw~++a
					if (mpL->isEqualTo(mpR->mpL))
					{
						Dispose(DetachParent(mpL));
						InsertParent(mpL, ACTN_INCPRE);
						return RETURN_1(PSTG);
					}
				}
				assert(!mpR->mpL->isNumUnit());//must have been reduce at this point
			}
			else if (mpR->is(ACTN_SUB))
			{
				// csw~a=a-1 => csw~--a
				if (mpR->mpR->isNumUnit())
				{
					if (mpL->isEqualTo(mpR->mpL))
					{
						Dispose(DetachParent(mpL));
						InsertParent(mpL, ACTN_DECPRE);
						return RETURN_1(PSTH);
					}
				}
			}
		}
	}

	// a+=a+1 => a++
	if (pSelf->is(ACTN_MOVADD))
	{
		if (mpR->isNumUnit())
		{
			pSelf->mAction = ACTN_INCPOST;
			Detach(mpR);
			return RETURN_1(PSTI);
		}
	}

	// a=a-1 => a--
	if (pSelf->is(ACTN_MOVSUB))
	{
		if (mpR->isNumUnit())
		{
			pSelf->mAction = ACTN_DECPOST;
			Detach(mpR);
			return RETURN_1(PSTJ);
		}
	}

	return 0;
}

void Out_t::flipChilds()
{
	Out_t *p = mpL;
	mpL = mpR;
	mpR = p;
}

//search for integer displacement component
/*
Out_t *Out_t::GetDisp()
{
	if (is(ACTN_ADD))
	{
		Out_t *p;
		if ((p = mpL->GetDisp()) || (p = mpR->GetDisp()))
			return p;
	}
	else if (isTerm())
	{
		if (m_pOp)
		if (m_pOp->IsScalar())
			return this;
	}

	return 0;
}*/

int ExprInfoBase_t::__getDisp2(Out_t *pSelf, int &d, bool bDetach) const
{
	int res;
	if (pSelf->is(ACTN_ADD))
	{
		if (res = __getDisp2(pSelf->mpL, d, bDetach))
			return res;
		if (res = __getDisp2(pSelf->mpR, d, bDetach))
			return res;
	}
	else if (pSelf->is(ACTN_SUB))
	{
		if (res = __getDisp2(pSelf->mpL, d, bDetach))
			return res;
		int d2 = 0;
		if (res = __getDisp2(pSelf->mpR, d2, bDetach))
		{
			d -= d2;
			return res;
		}
	}
	else if (pSelf->is(ACTN_GET) || pSelf->is(ACTN_MOV))
	{
		return -1;//abort
	}
	else
//	if (isTerm())
	{
		if (pSelf->isImmidiate())
		{
			d += pSelf->m_value.i32;
			if (bDetach)
				DetachParent(pSelf->sibling());
			return 1;
		}
	}

	return 0;
}

int ExprInfoBase_t::ExtractDisplacement2(Out_t *pSelf, int &d, bool bDetach) const
{
	Out_t *p = pSelf;
	while (p->is(ACTN_ADD) || p->is(ACTN_SUB))
		p = p->mpL;

	return ExtractDisplacement(p, d, bDetach);
}

int ExprInfoBase_t::ExtractDisplacement(Out_t *pSelf, int &d, bool bDetach) const
{
	int res;
	for (Out_t *p = pSelf->mpU; p; p = p->mpU)
	{
		int d2 = 0;
		if (p->is(ACTN_ADD))
		{
			if (res = __getDisp2(p, d2, bDetach))
			{
				if (res == -1)
					return 0;
				d += d2;
			}
		}
		else if (p->is(ACTN_SUB))
		{
			if (res = __getDisp2(p, d2, bDetach))
			{
				if (res == -1)
					return 0;
				d += d2;
			}
		}
		else
			break;
	}

	return 1;
}

//get lowermost offset value or nullptr if expression does not comply
Out_t *ExprInfoBase_t::CheckDisplacement(Out_t *pSelf) const
{
	Out_t *pTop(pSelf);
	for (;;)
	{
		if (!pTop->isActionAddOrSub())
			break;
		if (pTop->mpL->isActionAddOrSub())
			break;
		pTop = pTop->mpR;
		if (pTop->isImmidiate())
			return pTop;
	}
	return nullptr;
}

int EXPRSimpl_t::Simplify6(Out_t *pSelf) const
{
	if (!pSelf->isTerm())
		return 0;

	if (pSelf->isOpKind())
	{
		if (!TypOf(pSelf).isPtr())
			return 0;
	}
	else if (pSelf->mKind == OUT_NULL)
		return 0;
	else
		//assert(false);
		return 0;


	assert(pSelf->mpU);
	if (pSelf->mpU->is(ACTN_ADD))
	{
		//set base ptr as left of <+> action
		if (pSelf->mpU->mpR == pSelf)
		{
			if (!pSelf->mpU->mpL->isOpKind() || !TypOf(pSelf->mpU->mpL).isPtr())
			{
				assert(0);
				pSelf->mpU->flipChilds();
				return 1;
			}
		}

/*		//set base ptr as first component in address expression
		if (mpU->mpU)
		if (mpU->mpU->is(ACTN_ADD))
		{
			//mpU<+> may be in left position of mpU->mpU<+>, 
			//so flip it
			if (mpU->mpU->mpL == mpU)
			{
				mpU->mpU->flipChilds();
				return 1;
			}

			mpU->mpL = mpU->mpU->mpL;
			mpU->mpL->mpU = mpU;
			mpU->mpU->mpL = this;
			mpU = mpU->mpU;
			return 1;
		}*/
	}
	else if (pSelf->mpU->is(ACTN_SUB))
	{
		if (pSelf->mpU->mpU && pSelf->mpU->mpU->is(ACTN_CHECK))
			return 0;
//???		assert(mpU->mpL == this);
		if (pSelf->mpU->mpR->isImmidiate())
		{
			pSelf->mpU->mAction = ACTN_ADD;
			pSelf->mpU->mpR->m_value.i32 = -pSelf->mpU->mpR->m_value.i32;
		}
		else//???
		{
//			mpU->mpR->InsertParent(ACTN_CHS);
//			mpU->mAction = ACTN_ADD;
//			return 1;
		}
	}
	else 
	{
		if (!pSelf->mpU->is(ACTN_INDIR))
			if (!pSelf->mpU->is(ACTN_TYPE))
				return 0;
	}

	return 0;
}

/*
int Out_t::CheckTypeCast(FieldPtr pField)
{
	if (!is(ACTN_TYPE))
	{
		if (!is(ACTN_ARRAY))
			return 0;
		return mpU->CheckTypeCast(pField);
	}

	if (pType->isComplex())
	{
		if (!pField->IsComplex0())
			return 0;
		if (pType->m_pStruc == pField->m_pStruc)
			return 1;
	}
	return 0;
}
*/

int Out_t::__extractMultiplier(int &mult, bool)
{
	if (isTerm())
	{
		if (isImmidiate())
		{
			mult *= m_value.i32;
			return 1;
		}
		return 0;
	}

	if (is(ACTN_ADD) || is(ACTN_SUB))
	{
		int iL = 1;
		mpL->__extractMultiplier(iL);

		int iR = 1;
		mpR->__extractMultiplier(iR);
		
		int nod = NOD(iL, iR);
		mult = nod;
		return 1;
	}
	else 
	{
		if (is(ACTN_SHL))
		{
			if (mpR->isImmidiate())
			{
				int nCount = mpR->m_value.i32;
				if (nCount < 32)
				{
					mAction = ACTN_MUL;
					mpR->m_value.i32 = 1;
					for (int i = 0; i < nCount; i++)
						mpR->m_value.i32 *= 2;
				}
			}
		}

		if (is(ACTN_MUL))
		{
			mpL->__extractMultiplier(mult);
			mpR->__extractMultiplier(mult);
		}
	}
	
	return 0;
}

int Out_t::ExtractMultiplier(int &mult, bool bDetach)
{
	mult = 1;
	__extractMultiplier(mult, bDetach);
	return 1;
}

// a+i => &a->f+i-i
int EXPRSimpl_t::SimplifyIndir(Out_t * pSelf) const
{
	if (IsUnfoldMode())
		return 0;
	//if (testf(adcui::DUMP_NOSTRUCS))
		//return 0;

	bool bSub(false);
	if (!pSelf->is(ACTN_ADD))
	{
		if (!pSelf->is(ACTN_SUB))
			return 0;
		bSub = true;
	}

	Out_t* pPtrBase0(pSelf->mpL);
	Out_t* pPtrBase(pPtrBase0->is(ACTN_TYPE) ? pPtrBase0->mpR : pPtrBase0);

	TYP_t Tb(TypOf(pPtrBase));
	if (!Tb.isPtr())
		return 0;
	if (Tb.isArray())
		return 0;
	
	Out_t *mpU = pSelf;

	int disp(0);
	if (!ExtractDisplacement(pPtrBase0, disp))
		return 0;
	/*if (pSelf->mpR->isImmidiate())
	{
		disp = pSelf->mpR->m_value.i32;
		if (bSub)
			disp = -disp;
	}*/

//CHECK(disp != 0)
//STOP

	if (disp == 0)
	{
		if (!mpU->is(ACTN_INDIR))
			if (!mpU->is(ACTN_TYPE))
				return 0;

		TYP_t T(TypOf(mpU));
		if (mpU->is(ACTN_INDIR))
			T = PtrOf(T);//increase ptr level

		if (AreCompliant(Tb, T))
			return 0;

		if (!pPtrBase->is(ACTN_OFFS))
			if (!mpU->is(ACTN_INDIR))
				if (!mpU->mpU->is(ACTN_INDIR))
					return 0;
	}

	TYP_t Td(DerefOf(Tb));
	if (!Td.isComplex())
		return 0;
	
	CTypePtr iStruc(Td.asTypeRef());
	if (!iStruc)
		return 0;

	assert(iStruc->typeStruc());
	Struc_t* pStruc(iStruc->typeStruc());
	FieldPtr pField(pStruc->GetFieldEx(disp));
	if (!pField)
		return 0;

	Out_t* peType(nullptr);
	if (pPtrBase0->is(ACTN_TYPE))
	{
		peType = DetachParent(pPtrBase);
		//Tb = TypOf(pPtrBase);
	}

	/////////////////////////////////////////no return point
	Out_t* pOutPtr(InsertParent(pPtrBase, ACTN_PTR, true));//dock op from parent
	Add(*pOutPtr, pField);
	pOutPtr->setTyp(fromField(pField));
	Out_t* pOutOffs(InsertParent(pOutPtr, ACTN_OFFS));
	pPtrBase = pOutOffs;

	if (peType)
		pPtrBase = InsertParent(pPtrBase, peType);
#if(0)
	else
		pPtrBase = InsertParent(pPtrBase, PtrOf(fromOpType(OPTYP_INT8)));
#endif

	Out_t *pOutUpPtr(pPtrBase);

	Out_t* pTop(pPtrBase);
	int offs = GetDataOffset(pField);
	disp = -offs;
	if (disp > 0)
	{
		pTop = InsertParent(pTop, ACTN_ADD);
		Add(*pTop, pTop->dockOp(), disp);
	}
	else if (disp < 0)
	{
		pTop = InsertParent(pTop, ACTN_SUB);
		Add(*pTop, pTop->dockOp(), -disp);
	}
	else
	{
		if (pTop->mpU->is(ACTN_TYPE))
			if (pTop->mpU->mpU->is(ACTN_INDIR))
			{
				Out_t* pTypeCast(pTop->mpU);
				Out_t* pIndir(pTypeCast->mpU);
				//type cast must be eliminated by complience
				TYP_t T(DerefOf(pTop));
				TYP_t Tproxy(ProxyOf(T));
				if (AreCompliant(Tproxy, TypOf(pIndir)))
				{
					DetachParent(pTop);
		//			SetTyp(pIndir, T);
				}
			}
	}

	//if (pOutUpPtr)
		//PropagatePtrUp(*pOutUpPtr);

	return RETURN_1(IND4);
}

int EXPRSimpl_t::SimplifyIndir3(Out_t* pSelf) const
{
	if (IsUnfoldMode())
		return 0;

	if (!pSelf->isTerm())
	{
		if (!pSelf->is(ACTN_PTR))
			if (!pSelf->is(ACTN_OFFS))
				if (!pSelf->is(ACTN_ARRAY))
					return 0;
	}
	else if (!pSelf->isFieldKind())
		return 0;

	if (!TypOf(pSelf).isPtr())
		return 0;

	Out_t *pU = pSelf->mpU;

	if (TypOf(pSelf).isArray())
		return 0;
	if (pU->is(ACTN_PTR))
		return 0;//already
	if (pU->is(ACTN_OFFS))
		return 0;//already

	if (!pU->is(ACTN_INDIR))
		if (!pU->is(ACTN_TYPE))
			return 0;

	TYP_t T(TypOf(pU));
	if (pU->is(ACTN_INDIR))
		T = PtrOf(T);//increase ptr level

	if (AreCompliant(TypOf(pSelf), T))
		return 0;

	if (!pSelf->is(ACTN_OFFS))
		if (!pU->is(ACTN_INDIR))
			if (!pU->mpU->is(ACTN_INDIR))
				return 0;

	if (!TypOf(pSelf).isPtr())
		return 0;
	
	TYP_t T2(DerefOf(pSelf));
	if (!T2.isComplex())
		return 0;

	CTypePtr iStruc(T2.asTypeRef());
	if (!iStruc)
		return 0;

	assert(iStruc->typeStruc());
	Struc_t* pStruc(iStruc->typeStruc());
	FieldPtr pField(pStruc->GetFieldEx(0));
	if (!pField || IsEosField(pField))
		return 0;

	Out_t* pOutPtr(InsertParent(pSelf, ACTN_PTR, true));
	Add(*pOutPtr, pField);
	//SetTyp(pOutPtr, fromField(pField));
	Out_t* pOutOffs(InsertParent(pOutPtr, ACTN_OFFS));

#if(0)
	Out_t* pTop(pOutOffs);
	Out_t* pOutUpPtr(pTop);
	//int offs = GetDataOffset(pField);
	if (pTop->mpU->is(ACTN_TYPE))
		if (pTop->mpU->mpU->is(ACTN_INDIR))
		{
			Out_t* pTypeCast(pTop->mpU);
			Out_t* pIndir(pTypeCast->mpU);
			//type cast must be eliminated by complience
			TYP_t T(DerefOf(pTop));
			TYP_t Tproxy(ProxyOf(T));
			if (AreCompliant(Tproxy, TypOf(pIndir)))
			{
				DetachParent(pTop);
				//TypOf(pIndir) = T;
			}
		}

	if (pOutUpPtr)
		PropagatePtrUp(*pOutUpPtr);
#endif

	return RETURN_1(IND5);
}

/*void EXPRSimpl_t::PropagatePtrUp(Out_t& rSelf) const
{
	Out_t *pOut(&rSelf);
	if (!TypOf(pOut).isPtr())
		return;
	while (pOut->mpU && !pOut->mpU->isRoot() && !pOut->mpU->is(ACTN_INDIR) && TypOf(pOut->mpU).isNull())
	{
		pOut->mpU->setTyp(Complied0(TypOf(pOut->mpU), PtrOf(TYP_t())));
		pOut = pOut->mpU;
	}
}*/

// &A+index*scale => &A[index], if 1) typeof(obj) is ptr to obj of size 'scale'
// *(&A+index*scale) =>A[index] if 2) typeof(A) is array of elements of size 'scale'
int EXPRSimpl_t::SimplifyArrays(Out_t * pSelf) const
{
	if (IsUnfoldMode())
		return 0;
	//if (testf(adcui::DUMP_NOSTRUCS))
		//return 0;

	bool bSub(false);
	if (!pSelf->is(ACTN_ADD))//array?
	{
		if (!pSelf->is(ACTN_SUB))
		{
			/*if (pSelf->mpL->is(ACTN_OFFS) && TypOf(pSelf->mpL->mpR).isArray())
			{
				if (!pSelf->is(ACTN_ARRAY))
				{
					Out_t* pArray = InsertParent(pSelf, ACTN_ARRAY);
					Add(*pArray, HOP(), (int)0);
					InsertParent(pArray, ACTN_OFFS);
					return RETURN_1(ARR1);
				}*/
			return 0;
		}
		bSub = true;
	}

	Out_t* mpL(pSelf->mpL);
	if (!TypOf(mpL).isPtr())
	{
		if (!TypOf(pSelf->mpR).isPtr())
			return 0;
		/*Out_t* pOutUU = pSelf->mpU;
		//		if (pOutUU->isComparAction0())
		//			return 0;
		if (bSub)
			return 0;
		mpU->flipChilds();
		return RETURN_1(SWA5);*/
	}

	if (mpL->is(ACTN_INDIR))
		if (mpL->mpR->is(ACTN_OFFS))
			return 0;// *&a+i

	if (pSelf->mpU->is(ACTN_MOV))
	{
		//avoid applying arrays where simple pointer increment is expected
		if (pSelf->sibling()->isEqualTo(pSelf))
			return 0;
	}

	if (mpL->is(ACTN_TYPE))//(type*)&base+i
	{
		TYP_t T(DerefOf(mpL));
		if (!T.isSimple() || T.opsiz() != OPSZ_BYTE)
			return 0;
		mpL = mpL->mpR;
	}

	TYP_t T(DerefOf(mpL));
	if (T.isNull())
		return 0;
	TYP_t Tbase(T);
	if (T.isArray())
		Tbase = T.arrayBase();//?
	else if (mpL->is(ACTN_OFFS))
		return 0;
		
	Out_t* pOutBase = mpL;

	int mult(0);
	Out_t* pSibling = pSelf->mpR;
	pSibling->ExtractMultiplier(mult);

	int nStrucSz(Tbase.size());
	if (nStrucSz == 0)
		return 0;

	if (mult)
		if (mult < nStrucSz)
			return 0;

	div_t d = div(mult, nStrucSz);
	if (d.rem)
	{
		//		return 0;
	}

	/*		if (abs(disp) >= mult)
			{
				div_t d = div(disp, mult);
				disp = d.rem;
				if (d.quot)
				{
					Out_t *pOut = pOutBase->mpU->mpR;
					pOut = pOut->InsertParent(ACTN_ADD);
					pOut->Add(d.quot);
				}
			}
	*/

	if (pOutBase->mpU->is(ACTN_TYPE))
		DetachParent(pOutBase);

	Out_t* pArray = pSelf;
	pArray->mAction = ACTN_ARRAY;
	/*if (pArray->mpU->is(ACTN_INDIR))
		pArray->setTyp(pArray->mpU);//?
	else*/
	//if (!T.arrayBase().isNull())
			pArray->setTyp(Tbase);

	if (d.rem)
	{
		Out_t* pSubRem = InsertParent(pArray->mpR, ACTN_SUB);
		Add(*pSubRem, HOP(), d.rem);
	}

	Out_t* pDiv = InsertParent(pArray->mpR, ACTN_DIV);
	Add(*pDiv, HOP(), nStrucSz);

	if (bSub)
		InsertParent(pArray->mpR, ACTN_CHS);

	// (&a[j])[i] => &a[i+j]	???
	if (pOutBase->is(ACTN_OFFS) && pOutBase->mpR->is(ACTN_ARRAY))
	{
		Out_t* pIndex1 = pOutBase->mpR->mpR;
		Out_t* pIndex2 = pArray->mpR;
		DetachParent(pOutBase);
		Out_t* pAdd = InsertParent(pIndex1, ACTN_ADD);
		Add(*pAdd, *pIndex2);
	}
	else
	{
		pOutBase = InsertParent(pArray, ACTN_OFFS);
	}

	assert(pOutBase->is(ACTN_OFFS));
	if (d.rem)
	{
		Out_t* pAddRem = InsertParent(pOutBase, ACTN_ADD);
		Add(*pAddRem, HOP(), d.rem);
	}

	return RETURN_1(ARR2);
}

/*int EXPRSimpl_t::SetupTypes2(Out_t *pSelf) const
{
	if (pSelf->is(ACTN_UNITE))
	{
		uint8_t t = pSelf->mpR->size() + pSelf->mpL->size();
		pSelf->setTyp(fromOpType(t));
		if (TypOf(pSelf->mpR).isPtr() && pSelf->mpR->size() == mrDC.PtrSize())//near
			if (t == mrDC.PtrSizeEx())
		{
			pSelf->setTyp(TypOf(pSelf->mpR));
			//TypOf(pSelf).opsz = t;
		}
	}
	return 0;
}*/

int EXPRSimpl_t::SimplifyMulDiv(Out_t *pSelf) const//mul&div => shl&shr
{
	if (pSelf->is(ACTN_MUL))
	if (!pSelf->mpL->is(ACTN_CHS))
	if (pSelf->mpR->isImmidiate())
	{
		unsigned val = pSelf->mpR->m_value.ui32;
		if (CountBits(val) == 1)
		{
			int nCount = -1;
			do {
				val >>= 1;
				nCount++;
			} while (val);
			pSelf->mpR->m_value.ui32 = nCount;
			pSelf->mAction = ACTN_SHL;
			return RETURN_1(BIN1);
		}
	}

	if (pSelf->is(ACTN_DIV))
	if (pSelf->mpR->isImmidiate())
	{
		unsigned val = pSelf->mpR->m_value.ui32;
		if (CountBits(val) == 1)
		{
			int nCount = -1;
			do {
				val >>= 1;
				nCount++;
			} while (val);
			pSelf->mpR->m_value.ui32 = nCount;
			pSelf->mAction = ACTN_SHR;
			return RETURN_1(BIN2);
		}
	}

	return 0;
}

int EXPRSimpl_t::SimplifyType(Out_t* pSelf, bool bSecondary) const//primary
{
	if (!pSelf->is(ACTN_TYPE))
		return 0;

	TYP_t T(TypOf(pSelf));
	//TYP_t T1(T.stripTypedef());
	TYP_t Tm(T.stripModifier());
	TYP_t Tpxy(ProxyOf(Tm));

	Out_t* pR(pSelf->mpR);
	TYP_t TR(TypOf(pR));
	TYP_t TRm(TR.stripModifier());
	TYP_t TRpxy(ProxyOf(TRm));

	Out_t* pU(pSelf->mpU);
	TYP_t TU(TypOf(pU));
	TYP_t TUm(TU.stripModifier());
	TYP_t TUpxy(ProxyOf(TUm));

	if (Tm.isNull())
	{
		if (!bSecondary)
			if (pU->mpU->isRoot() || pU->mpU->is(ACTN_SEMI))//only for top expressions
				return 0;
		assert(pU->is(ACTN_MOV) && pSelf->isRight());
		TYP_t TUL(TypOf(pSelf->sibling()));
		if (TUL.isNull())
			return 0;
		if (pR->is(ACTN_INDIR))
		{
			TYP_t Tz(Complied(TR, TUL));
			if (!Tz.isNull())
				if (Tz.canPromoteTo(TUL))
				{
					DetachParent(pR);//self
					InsertParent(pR->mpR, PtrOf(TUL));
					return RETURN_1(TY22);//dup
				}
		}
		pSelf->setTyp(TUL);
		return RETURN_1(TY22);
	}

	// (type)expr => expr		| if typeof(expr)==type
	if (!pU->is(ACTN_GOTO))
	if (!pR->is(ACTN_MOV))
	if (!T.isCallPtr())
	if (TRm == Tm)
	{
		DetachParent(pR);
		return RETURN_1(TYP7);
	}

	// (type)expr => expr		| if actionof(expr) is bool
	if (ISBOOL(pR->mAction))
	{
		pR->setTyp(TypOf(pSelf));
		DetachParent(pR);
		return RETURN_1(TYP1);
	}

	if (pU->is(ACTN_TYPE))
	if (TUm.isReal())
	if (T.isReal80())
	{
		DetachParent(pR);
		return RETURN_1(TYP2);
	}
	
	if (pR->is(ACTN_TYPE))
	{
		//(type_ptr1)(type_ptr2) => (type1_ptr)
		if (T.isPtr() && TR.isPtr())
		{
			DetachParent(pR->mpR);
			return RETURN_1(TYP3);
		}

		// (type1_ptr)(type_ptr) => (type1_ptr)
		if (Tpxy.isInt())//or ptr
		{
			TYP_t Tch_pxy(ProxyOf(TR.stripTypedef()));
			if (Tch_pxy.isInt())//or ptr
				if (Tpxy.size() == Tch_pxy.size())
				{
					DetachParent(pR->mpR);
					return RETURN_1(TYP4);
				}
		}
		else if (Tpxy.isCallPtr())//or ptr
		{
			TYP_t Tch_pxy(ProxyOf(TypOf(pR).stripTypedef()));
			if (Tch_pxy.isInt())//or ptr
				if (PtrSize() == Tch_pxy.size())
				{
					DetachParent(pR->mpR);
					return RETURN_1(TYP5);
				}
		}
	}

	if (pR->is(ACTN_INDIR))
	{
		Out_t* pRR(pR->mpR);
		if (pRR->is(ACTN_TYPE))
		{
			assert(TypOf(pRR).isPtr());
			TYP_t TRRd(DerefOf(pRR));
			if (TRRd.size() == pSelf->size())
			{
				if (TRRd.canPromoteTo(T))
				{
					DetachParent(pR);//self
					InsertParent(pRR, PtrOf(T));
					return RETURN_1(TYP5);//dup
				}
			}
		}
	}

	if (pR->isImmidiate())
	{
		TYP_t Timm(TypOf(pR));

		if (Tm.isPtr())
		{
			if (pR->isNumZero())
			{
				if (Timm.size() == Tm.size())
				{
					//(type_ptr)0 => nullptr
					pR->setTyp(Tm);//ptr will be displayed as `nullptr`
					DetachParent(pR);
					return RETURN_1(TYP6);
				}
			}
			return 0;//immidiates never shuld represent pointers in HLL
		}

		//check the value against the type
		if (!Timm.isReal())
		{
			int d(Timm.size() - Tm.size());
			if (d > 0)
			{
				bool bIsNarrowing(true);
				for (int i(Tm.size()); i < Timm.size(); i++)
					if (pSelf->m_value.buf[i] != 0)
					{
						bIsNarrowing = false;
						break;
					}
				if (bIsNarrowing)
					Timm = TYP_t(fromOpType(MAKETYP(Timm.optyp(), Tm.size())));
			}
		}

		if (!Tm.isCallPtr() && Timm.canPromoteTo(Tm, false))
		{
			//long double ~ double (on Windows)
			//TYP_t Timm2(Complied0(TypOf(pR), Tm));
			TYP_t Timm2(T);
			if (Timm2.isReal80())
				Timm2 = TYP_t(fromOpType(OPTYP_REAL64));
			pR->setTyp(Timm2);
			DetachParent(pR);
			return RETURN_1(TYP8);
		}
	}

	/////////////////////
	//checking the upper

	if (pU->is(ACTN_INDIR))
	{
		if (T.isPtr())//?
		{
			TYP_t Td(DerefOf(T));
			if (TR.isPtr())
			{
				TYP_t TRd(DerefOf(TR));
				if (!TRd.isNull())
				{
					if (pU->mpU->is(ACTN_MOV) && pU->isLeft())
					{
						//  *(type*)a=b => a=b
						TYP_t TUR(pU->mpU->mpR->typ());//b
						if (TUR.size() == Td.size())
							if (TUR.canPromoteTo(Td))
							{
								if (TUR.size() == TRd.size())
									if (TUR.canPromoteTo(TRd))
									{
										DetachParent(pR);//self
										return RETURN_1(TYP9);
									}
							}
					}
				}
			}
		}
		else
		{
			STOP
		}
#if(0)
		if (TR.isPtr())
		{
			if (Td.size() == TRd.size())
			{
				if (pU->mpU->is(ACTN_MOV) && pU->isLeft())
				{
					DetachParent(pR);//self
					if (pU->isRight())
					{
						pSelf->setTyp(Td);
						InsertParent(pU, pSelf);
					}
					return RETURN_1(TYP0);
				}
			}
		}
#endif

		if (pR->is(ACTN_OFFS))
		{
			Out_t* peObj(pR->mpR);
			assert(peObj->isFieldKind() || peObj->is(ACTN_PTR) || peObj->is(ACTN_ARRAY) || peObj->is(ACTN_DOT));
			{
#if(1)
				TYP_t Tobj_pxy(ProxyOf(peObj));
				TYP_t Tobj_pxy_ptr(PtrOf(Tobj_pxy));
				if (AreCompliant(Tm, Tobj_pxy_ptr))
				{
					// *(type_ptr)&a => *&a  | typof(a)<->typeof(self)
					DetachParent(pR);//self
					return RETURN_1(TY10);
				}
#endif
				// *(type_ptr)&a => (type)*&a

				/*TYP_t T_d(DerefOf(pSelf));
				if (Tobj_pxy.isPtr())//move out of deref
				if (T_d.isPtr())
				{
					assert(Tm.isPtr());
					DetachParent(pR);//self
					pSelf->setTyp(T_d);
					InsertParent(pU, pSelf);
					return RETURN_1(TYP0);
				}*/
			}
		}
	}

	if (pU->isComparAction())
	{
		if (T.isPtr())
		{
			DetachParent(pR);//self
			return RETURN_1(TY11);
		}
	}

	//if (!(type)expr) => !expr
	if (Tpxy.isInt())
	{
		if (!pR->is(ACTN_CALL) || !pR->mpL->typ().isCallPtr())//preserve retval of call cast
		{
			if (pU->is(ACTN_LOGNOT) || pU->is(ACTN_LOGOR) || pU->is(ACTN_LOGAND))
			{
				DetachParent(pR);//self
				return RETURN_1(TY12);
			}
			else if (pU->is(ACTN_GOTO))
			{
				if (!pSelf->isRight())//not RET
				{
					DetachParent(pR);//self
					return RETURN_1(TY13);
				}
			}
		}
	}

	// (type)obj => obj, if (typeof(obj) == type)
	//if (Tpxy == TR)
	if (!T.isCallPtr())
	if (!pSelf->mpU->is(ACTN_HIHALF))
	//if (pSelf->mpU->is(ACTN_MOV) || pSelf->mpU->is(ACTN_TYPE) || pSelf->mpU->is(ACTN_ADD) || pSelf->mpU->is(ACTN_SUB) || pSelf->mpU->is(ACTN_MUL))
//?	if (!pSelf->mpR->is(ACTN_TYPE))
	if (TRpxy.canPromoteTo(Tpxy))
	{
		if (Tpxy.isPtr() && pR->isImmidiate())
			return 0;

		if (Tpxy.canPromoteTo(TUpxy))
		{
			DetachParent(pR);
			return RETURN_1(TY14);
		}
	}

	//move ptr typecast down for add & sub (ptr modification)
	//(type1*)((type2*)a+b) => (type1)(type2)a+b
	if (Tm.isPtr() && TR.isPtr())
	if (pR->is(ACTN_ADD) || pR->is(ACTN_SUB))
	//if (pR->mpL->is(ACTN_TYPE) && TypOf(pR->mpL).isPtr())
	{
		TYP_t Tm_d(DerefOf(Tm));
		TYP_t TRm_d(DerefOf(TRm));
		if (Tm_d.size() == TRm_d.size())
		{
			Out_t* pRL(pR->mpL);
			DetachParent(pR);//self
			InsertParent(pRL, pSelf);
			return RETURN_1(TY15);
		}
	}

	//return SimplifyType4(pSelf);

	return 0;
}

int EXPRSimpl_t::SimplifyType4(Out_t* pSelf) const
{
	if (!pSelf->is(ACTN_TYPE))
		return 0;

	TYP_t T(TypOf(pSelf));
	TYP_t Tm(T.stripModifier());
	TYP_t Tpxy(ProxyOf(Tm));

	Out_t* pR(pSelf->mpR);
	TYP_t TR(TypOf(pR));
	TYP_t TR1(TR.stripModifier());
	TYP_t TRpxy(ProxyOf(TR1));

	Out_t* pU(pSelf->mpU);
	TYP_t TU(TypOf(pU));
	TYP_t TUm(TU.stripModifier());
	TYP_t TUpxy(ProxyOf(TUm));


	if (!Tpxy.isCallPtr())
	if (!pSelf->mpU->is(ACTN_HIHALF))
	//if (!pSelf->mpR->is(ACTN_TYPE))
	if (TRpxy.canPromoteTo(Tpxy))
	{
		if (Tpxy.isPtr() && pSelf->mpR->isImmidiate())
			return 0;//cannot convert a number to ptr

#if(0)//LATER!
		if (Tpxy.canPromoteTo(TUpxy))
#endif
		{
			DetachParent(pSelf->mpR);//self
			return RETURN_1(TY16);
		}
	}


	/*if (T.isReal())
		if (ISBOOL(pSelf->mpU->mAction))
		{
			DetachParent(pSelf->mpR);
			return RETURN_1(TYP0);
		}*/

	/*if (!pSelf->mpU->is(ACTN_MOV) || pSelf->isLeft() || TypOf(pSelf->mpU) == T)//is right
	{
		if (T.isReal())
		{
			if (T.isReal80())
			{
				if (T.isImplicitCastOf(TR))
				{
					// *** a=(REAL80)b

					DetachParent(pSelf->mpR);
					return RETURN_1(TYP0);
				}
			}
			if (T == TypOf(pSelf->mpU))
				if (T.isImplicitCastOf(TR))
				{
					DetachParent(pSelf->mpR);
					return RETURN_1(TYP0);
				}
		}
	}*/
	return 0;
}

int EXPRSimpl_t::SimplifyThis(Out_t* pSelf) const
{
	if (IsUnfoldMode())
		return 0;

	//<this> ptr removal 
	if (!pSelf->is(ACTN_PTR))
		return 0;

	if (pSelf->mpL->isOpKind())
	{
		if (pSelf->mpL->mSsid & SSID_THISPTR)//if (mpL->m_p Op0 && mpL->m_p Op0->IsT hisPtr())
		{
			pSelf->mpR->setTyp(TypOf(pSelf));
			DetachParent(pSelf->mpR);
			return RETURN_1(THS2);
		}
	}
	else if (pSelf->mpL->isFieldKind())
	{
		if (ThisPtrArg() == pSelf->mpL->field())
		{
			pSelf->mpR->setTyp(TypOf(pSelf));
			DetachParent(pSelf->mpR);
			return RETURN_1(THS3);
		}
	}
	return 0;
}

int EXPRSimpl_t::SimplifyArrCall(Out_t * pSelf) const
{
	if (pSelf->is(ACTN_ARRAY))
	{
		if (pSelf->mpL->is(ACTN_OFFS))
		if (TypOf(pSelf->mpL->mpR).isArray())
		{
			DetachParent(pSelf->mpL->mpR);
			return RETURN_1(THS4);
		}
	}

	if (pSelf->is(ACTN_CALL))
	{
		if (pSelf->mpL && pSelf->mpL->is(ACTN_OFFS))
		{
			DetachParent(pSelf->mpL->mpR);
			return RETURN_1(THS5);
		}
	}

	/*?if (pSelf->is(ACTN_INDIR))
	{
		if (pSelf->mpU->is(ACTN_TYPE))
			if (TypOf(pSelf->mpU).isPtr())
		{
			Out_t *pOut = pSelf->DetachParent();
			Out_t *pType = InsertParent(pSelf->mpR, PtrOf(pOut));
			return RETURN_1(THS6);
		}
	}*/

	return 0;
}

int EXPRSimpl_t::SimplifyCompar(Out_t *pSelf) const
{
static const Action_t ifcond2cmp[0x10] = {
	ACTN_NULL,				
	ACTN_NULL,				
	ACTN__LESS,			
	ACTN__GREATEROREQUAL,	
	ACTN__EQUAL,			
	ACTN__NOTEQUAL,		
	ACTN__LESSOREQUAL,		
	ACTN__GREATER,			
	ACTN_NULL,				
	ACTN_NULL,				
	ACTN_NULL,				
	ACTN_NULL,				
	ACTN__LESS,			
	ACTN__GREATEROREQUAL,	
	ACTN__LESSOREQUAL,		
	ACTN__GREATER,	
	};

	if (pSelf->isComparAction0())
		if (!pSelf->mpR->is(ACTN_GET))
		{
			IfCond_t ifcond = (IfCond_t)(pSelf->mAction & 0xf);
			Action_t A = ifcond2cmp[ifcond];
			if (A != ACTN_NULL)
			{
				pSelf->mAction = A;
				Add(*pSelf, pSelf->dockOp(), (int)0);
				pSelf->flipChilds();
				return RETURN_1(SWA4);
			}
		}

	if (!pSelf->isComparAction())
		return 0;

//	if (mpR->isImmidiate())
//	if (mpR)
	if (pSelf->mpR->isNumZero())
	{
		if (pSelf->mpL->is(ACTN_SUB))
//		if (!(TypOf(mpL->mpL).isPtr() ^ TypOf(mpL->mpR).isPtr()))//not ptrs or ptr compare
//			if (!mpL->mpL->isImmidiate() || !mpL->mpL->isNumZero())
		{
			Out_t *pOut = pSelf->mpL->mpR;
			DetachParent(pSelf->mpL->mpL);//kill SUB action
			Detach(pSelf->mpR);
			Add(*pSelf, *pOut);
			return RETURN_1(ZER4);
		}

		if (pSelf->mpL->is(ACTN_ADD))
			if (!(TypOf(pSelf->mpL->mpL).isPtr() ^ TypOf(pSelf->mpL->mpR).isPtr()))//not ptrs or ptr compare
			{
				Out_t *pOut = pSelf->mpL->mpR;//kill ADD action
				DetachParent(pSelf->mpL->mpL);//kill SUB action
				Detach(pSelf->mpR);
				Out_t *pOutS = Add(*pSelf, HOP(), ACTN_CHS);//change sign
				Add(*pOutS, *pOut);
				return RETURN_1(CHS4);
			}
	}

#if(0)
	// pre-increment with condition check
	// a++, CMPRSN(a, b) => CMPRSN(++a, b)
	Out_t *mpU(pSelf->mpU);
	if (mpU->is(ACTN_COMMA))
	if (pSelf->isRight())
	{
		Out_t *pOutSibling(pSelf->sibling());
		Action_t eAction(pOutSibling->mAction);
		if (eAction == ACTN_INCPOST || eAction == ACTN_DECPOST)
		if (pOutSibling->mpL->isFieldKind())
		{
			for (Out_t::Iterator i(pSelf); i; ++i)
			{
				Out_t *pOut(i.top());
				if (pOutSibling->mpL->isEqualToTerm(pOut))
				{
					Detach(pOutSibling);
					Detach(pOut)->AddChild(pOutSibling);//parent returned
					Dispose(pOut);
					Dispose(DetachParent(pSelf));
					pOutSibling->mAction = (eAction == ACTN_INCPOST) ? ACTN_INCPRE : ACTN_DECPRE;
					pOutSibling->flipChilds();
					return RETURN_1(PRE1);
				}
			}
		}
	}
#endif

	return 0;
}

//check if post increment can be combined with the preceeding statement
int EXPRSimpl_t::SimplifyPostIncrement(Out_t *pSelf) const
{
	if (!(pSelf->is(ACTN_INCPOST) || pSelf->is(ACTN_DECPOST)))
		return 0;
	if (!pSelf->mpU->isActionSepar())
		return 0;
	if (!pSelf->mpL->isFieldKind())
		return false;
	Out_t *pTarget(nullptr);
	if (pSelf->isRight())
	{
		// a=b+c, b+- => a=(b+-)+c
		pTarget = pSelf->sibling();
	}
	else if (pSelf->mpU->mpU->isActionSepar())
	{
		//a=b+c, b++, d => a=(b++)+c, d
		pTarget = pSelf->mpU->sibling();
	}
	if (pTarget)
	{
		for (Out_t::Iterator i(pTarget); i; ++i)
		{
			Out_t *pOut(i.top());
			if (pSelf->mpL->isEqualToTerm(pOut))
			{
				Out_t *pSibling(pSelf->sibling());
				Detach(pSelf);
				Out_t *mpU(DetachParent(pSibling));
				Dispose(mpU);
				Detach(pOut)->AddChild(pSelf);
				Dispose(pOut);
				return RETURN_1(PRE1);
			}
		}
	}
	return 0;
}

//handle <this> ptr at func's call
// (&foo)(thisptr, arg2, arg3) => thisptr->(&foo)(arg2, arg3)
int EXPRSimpl_t::SimplifyThis0(Out_t * pSelf) const
{
	return 0;
	if (!pSelf->is(ACTN_CALL))
		return 0;
//	if (pSelf->mpU->is(ACTN_PTR) || pSelf->mpU->is(ACTN_DOT))
//		return 0;//already

	bool bModified(false);
	if (!pSelf->mpOp)
		return 0;

	/*assert(pSelf->m_p Op0->IsCall());
	if (!pSelf->m_p Op0->IsA ddr() && pSelf->mpL)
	{
		Out_t *pOutL2(pSelf->mpL);
		if (!pOutL2->is(ACTN_INDIR))
		{
			//dummy indirection
			//pOutL2 = pOutL2->mpR;
			Out_t *pOutIndir = InsertParent(pOutL2, ACTN_INDIR);
			//pOutIndir->m_p Op0 = pSelf->m_p Op0;
			bModified = true;
		}
	}*/

	if (!IsUnfoldMode())
	{
		if (!pSelf->mpL->is(ACTN_TYPE))//no call conforming?
		{
			//setup <this> calling convention
			CGlobPtr ifDef = GetCalleeFuncDef(pSelf->mpOp);
			if (ifDef)
			{
				//FuncDef_t &rfDef(*ifDef->typeFuncDef());
				//if (rfDef.thisPtr())
				if (ProtoInfo_s::IsThisCallType(ifDef))
				{
					Out_t *pOutThis = pSelf->mpR;
					//!assert(pOutThis);
					if (pOutThis)
					{
						if (pOutThis->is(ACTN_COMMA))
						{
							pOutThis = pOutThis->mpL;
							DetachParent(pOutThis->sibling());
						}
						else
							Detach(pOutThis);

						Out_t *pOutPtr = InsertParent(pSelf->mpL, ACTN_PTR);
						//	Out_t *pOutPtr = InsertParent(ACTN_PTR);
						Add(*pOutPtr, *pOutThis);
						pOutPtr->flipChilds();
						pSelf->mpOp = HOP();
						return RETURN_1(THS7);
					}
				}
			}
		}
	}

	//pSelf->m_p Op0 = 0;
	if (bModified)
		return RETURN_1(CAL9);
	return 0;
}

int EXPRSimpl_t::SimplifyExt(Out_t * pSelf) const
{
	if (IsUnfoldMode())
		return 0;

//	if (testf(EXPR_NOEXTEND, true))
	//	return 0;

	if (pSelf->is(ACTN_ZEROEXT))
	{
		//ZEROEXT(imm1) => imm2		;
		if (pSelf->mpR->isNumKind())
		{
			int size = TypOf(pSelf).size();
			pSelf->mpR->setTyp(fromOpType(MAKETYP_UINT(size)));
			RetypeTerm(pSelf->mpR, MAKETYP(pSelf->typ().optyp(), size));
			DetachParent(pSelf->mpR);//kill this
			return RETURN_1(EXT1);
		}

		//ZEROEXT(s) => *(unsigned type *)&s	;typeof(s) is not unsigned
		TYP_t t(TypOf(pSelf->mpR));
		if (!OPTYP_IS_UINT(t.optyp()))
		{
#if(1)
			OpType_t optyp(MAKETYP_UINT(t.size()));//-> to unsigned
			InsertParent(pSelf->mpR, fromOpType(optyp));//smaller type's signed=>unsigned cast
#else
			Out_t *pOffs = InsertParent(pSelf->mpR, ACTN_OFFS);//mpR is now ACTN_OFFS
			OpType_t optyp(MAKETYP_UINT(TypOf(pOffs).size()));
			Out_t *pTyp = InsertParent(pOffs, PtrOf(fromOpType(optyp)));//mpR is now ACTN_TYPE
			Out_t *pIndir = InsertParent(pTyp, ACTN_INDIR);//mpR is now ACTN_INDIR
#endif
			pSelf->mAction = ACTN_TYPE;//convert to type
			return RETURN_1(EXT2);
		}

		//mpR is unsigned!
		//ZEROEXT(s) => (unsigned type)s	;typeof(s) is not unsigned
		pSelf->mAction = ACTN_TYPE;
		return RETURN_1(EXT2);
	}
	if (pSelf->is(ACTN_SIGNEXT))
	{
		//if (!OPTYP_IS_SINT(TypOf(pSelf->mpR).OpType()))
		{
#if(0)
			pSelf->mpR->InsertParent(ACTN_OFFS);
			TYP_SetPtrTo(*pSelf->mpR, pSelf->mpR->mpR);
			pSelf->mpR->InsertParent(ACTN_TYPE);
			OpType_t optyp(MAKETYP_SINT(TypOf(pSelf->mpR->mpR).optype())));
			TYP_SetPtrTo(*pSelf->mpR, optyp);
			pSelf->mpR->InsertParent(ACTN_INDIR);
			TYP_Set(*pSelf->mpR, optyp);
#else
			//OpType_t optyp(MAKETYP_SINT(TypOf(pSelf->mpR).OpType())));
			//TYP_Set(*pSelf, optyp);
#endif
			pSelf->mAction = ACTN_TYPE;
			return RETURN_1(EXT3);
		}

		//DetachParent(pSelf->mpR);//just remove it
		//return RETURN_1(EXT4);
	}
	return 0;
}

Out_t *EXPRSimpl_t::Simplify8_3(Out_t *pSelf, Out_t *pOut) const
{
	if (pSelf->is(ACTN_MUL))
	{
		Out_t *pL = Simplify8_3(pSelf->mpL, pOut);
		if (pL)
			return pL;
		Out_t *pR = Simplify8_3(pSelf->mpR, pOut);
		if (pR)
			return pR;
		return nullptr;
	}

	if (pSelf->isEqualTo(pOut))
		return pSelf;

	return nullptr;
}

Out_t *EXPRSimpl_t::Simplify8_2(Out_t *pSelf, Out_t *pOut) const
{
	if (pSelf->is(ACTN_MUL))
	{
		Out_t *pL = Simplify8_2(pSelf->mpL, pOut);
		if (pL)
			return pL;
		Out_t *pR = Simplify8_2(pSelf->mpR, pOut);
		if (pR)
			return pR;
		return nullptr;
	}

	Out_t *p = Simplify8_3(pOut, pSelf);
	if (!p || p->isImmidiate())
		return nullptr;
	if (!p->mpU->is(ACTN_MUL))
	{
		if (p->isImmidiate())
			if (p->m_value.i32 == 1)
				return 0;
		Detach(p);
		Add(*p->mpU, HOP(), 1);
	}
	else
		DetachParent(p->sibling());
	return pSelf;
}

/*int EXPRSimpl_t::SimplifyMultiplier(Out_t *pSelf) const
{
	if (!pSelf->mpU)
		return 0;

	if (!pSelf->mpU->is(ACTN_ADD))
		if (!pSelf->mpU->is(ACTN_SUB))
			return 0;

	if (pSelf->isRight())
		return 0;

//	if (pSelf->isImmidiate())
	//	return 0;
	
	Out_t *p = Simplify8_2(pSelf, pSelf->mpU->mpR);
	if (!p)
		return 0;

	Out_t *pOutUpper = pSelf->mpU;
	if (!p->mpU->is(ACTN_MUL))
	{
		if (p->isImmidiate())
			if (p->m_value.i32 == 1)
				return 0;

		Detach(p);
		Add(*p->mpU, HOP(), 1);
	}
	else
		DetachParent(p->sibling());

	Out_t *pMul = InsertParent(pOutUpper, ACTN_MUL);
	Add(*pMul, *p);
	return RETURN_1(MUL6);
}*/

int EXPRSimpl_t::SimplifyCommonMult(Out_t *pSelf) const
{
	if (!pSelf->isTerm())
		if (!pSelf->is(ACTN_SIGNEXT))
			if (!pSelf->is(ACTN_ZEROEXT))
				return 0;
	if (pSelf->isImmidiate())
		return 0;
	if (!pSelf->isOpKind() && !pSelf->isFieldKind())
		if (!pSelf->is(ACTN_SIGNEXT))
			if (!pSelf->is(ACTN_ZEROEXT))
				return 0;

//return 0;
#if(0)
	if (!pSelf->mpU->is(ACTN_MUL))
		return 0;
#endif

	Out_t *pSelfU(pSelf);
	while (pSelfU->mpU->is(ACTN_MUL))
		pSelfU = pSelfU->mpU;

	if (pSelfU->isRight())
		return 0;

	if (!pSelfU->mpU->isActionAddOrSub())
		return 0;

//CHECK(mNo == 112)
//STOP

	Out_t *pPeer(FindPeerTerm(pSelfU->sibling(), pSelf));//find a peer on the sibling branch
	if (!pPeer)
		return 0;

	Out_t *pPeerU(pPeer);
	while (pPeerU->mpU->is(ACTN_MUL))
		pPeerU = pPeerU->mpU;

	//			if (z_step++ > 10)
	//			return 0;

	if (pPeerU->mpU != pSelfU->mpU)//are the peers on the same level?
	{
		assert(pPeerU->mpU->isActionAddOrSub());
		if (pPeerU->mpU != pSelfU->sibling())
		{
			if (!MoveHigher(*pPeerU))
				return 0;
			return RETURN_1(ADD4);
		}

//if (mNo == 112)
//	return 0;

		// bring the peers to the same level
		//(c+(b+a)):11	=>	((c+b)+a):11
		if (!MoveHigher(*pPeerU->sibling()))//a
		{
			assert(0);//why should this ever happen?
			return 0;
		}
	}

	// a+a*i => a*(1+1*i)

//CHECK(mNo == 112)
//STOP

	Out_t *pTop(pSelfU->mpU);//common top
	assert(pTop == pPeerU->mpU);

//	if (pSelf->mpU->is(ACTN_MUL))
//		pSelf->sibling()->DetachParent();//lower mul gone
//	else
	{
		Detach(pSelf);
		Add(*pSelf->mpU, HOP(), 1);
	}
//	if (pPeer->mpU->is(ACTN_MUL))
//		pPeer->sibling()->DetachParent();//lower mul
//	else
	{
		Detach(pPeer);
		Add(*pPeer->mpU, HOP(), 1);
	}

	Out_t *pMul(InsertParent(pTop, ACTN_MUL));
	Add(*pMul, *pSelf);
	pMul->setTyp(TypOf(pSelf));
	pMul->flipChilds();
	return RETURN_1(ADD5);
}

Out_t *EXPRSimpl_t::FindPeerTerm(const Out_t *pSelf, const Out_t *pOrigin) const//go down throug add/sub
{
	if (pSelf->isActionAddOrSub())
	{
		if (pSelf->mpL->isActionAddOrSub())
			return 0;//not ready yet
		if (!pSelf->mpL->is(ACTN_MUL))
			return 0;
		Out_t *pL(FindPeerTerm(pSelf->mpL, pOrigin));
		if (pL)
			return pL;
		Out_t *pR(FindPeerTerm(pSelf->mpR, pOrigin));
		if (pR)
			return pR;
		return 0;
	}
#if(0)
	if (!pSelf->is(ACTN_MUL))
		return 0;
#endif

	return FindPeerTermImpl(pSelf, pOrigin);

/*	if (!pSelf->isEqualTo(pOrigin))
		return 0;
	return pSelf;*/
}

Out_t *EXPRSimpl_t::FindPeerTermImpl(const Out_t *pSelf, const Out_t *pOrigin) const//go down throug add/sub
{
	if (pSelf->is(ACTN_MUL))
	{
		Out_t *pL = FindPeerTermImpl(pSelf->mpL, pOrigin);
		if (pL)
			return pL;
		Out_t *pR = FindPeerTermImpl(pSelf->mpR, pOrigin);
		if (pR)
			return pR;
		return nullptr;
	}
	if (!pSelf->isEqualTo(pOrigin))
		return nullptr;
	return (Out_t *)pSelf;
}

int EXPRSimpl_t::CheckThrough(Out_t &rSelf, Out_t *pOpL, Out_t *pOpR) const
{
//?	if (pOpR->IsCall())
//?		return 0;

	Out_t *pOut = &rSelf;

	int j = 0;
	uint32_t r1, r2;

	SSID_t ssidL = pOpL->SSId();
	SSID_t ssidR = pOpR->SSId();
	if (ssidL != ssidR)
		return 0;
/*
	if (pOpL->opc() == OPC_CPUREG)
	{
		if (pOpR->opc() != pOpL->opc())
			return 0;
	}
	else if (pOpL->opc() == OPC_AUXREG)
	{
		if (pOpR->opc() != pOpL->opc())
			return 0;
	}
	else if (pOpL->IsLoc al())
		assert(pOpR->IsLoc al());
	else
		return 0;//assert(false);
*/
//CHECK((pOpR->isOpKind()) && (pOpR->m_p Op0->ID() == 0x26ef))
//STOP

	int d = pOpL->opoff() - pOpR->opoff();//bytes
//	if (pOpL->OpSize() > pOpR->OpSize())
//		return pOut;//?
	r1 = ~((~0) << pOpL->size());//1maskbit == 1Byte of size
	r2 = ~((~0) << pOpR->size());
	if (d > 0)
		r1 <<= d;
	else if (d < 0)
		r2 <<= d;

	if ( !(r2 & (~r1)) )
		return 1;//total overlap
	if (!(r2 & 1))
		return 0;//overlap low boundaries are not aligned (r2 higher than r1)
	if (!(r1 & r2))
		return 0;//?no overlap

//	struct {
//		Action_t A;
//		int sz;
//	} ActionStack[3];
//	int nActionIndex = 0;//action stack index

	int _sz = pOpR->size();
	int n = 4;
	int m = 2;
	int mask = 0xf;
	do {
		if (r1 == r2)
			break;

		Action_t A = ACTN_NULL;

		int r = r1 & r2;
		if (r & mask)
		{
			if (r2 & ~mask)
				A = ACTN_LOHALF;
			r2 &= mask;
		}
		else
		{
			A = ACTN_HIHALF;
			r1 >>= n;
			r2 >>= n;
		}

		if (A != ACTN_NULL)//insertion must be inverted
		{
			_sz /= 2;
//			assert(nActionIndex < 3);
//			ActionStack[nActionIndex].A = A;
//			ActionStack[nActionIndex].sz = _sz;
//			nActionIndex++;
			pOut = InsertParent(pOut, A);//Add(A);
			pOut->setTyp(fromOpType(_sz));
		}

		mask >>= m;
		m--;
		n >>= 1;
	} while (n);

//	while (nActionIndex)
//	{
//		nActionIndex--;
//		Action_t A = ActionStack[nActionIndex].A;
//		int sz = ActionStack[nActionIndex].sz;
//		pOut = pOut->InsertParent(A);//Add(A);
//		TypOf(pOut).opsz = sz;
//	}

	return 1;
}

int EXPRSimpl_t::CheckThrough3(Out_t &rSelf, Out_t *pOpL, Out_t *pOpR) const
{
	Out_t *pOut = &rSelf;

	int j = 0;
	uint32_t r1, r2;

	SSID_t ssidL = pOpL->SSId();
	SSID_t ssidR = pOpR->SSId();
	if (ssidL != ssidR)
		return 0;

	int d = pOpL->opoff() - pOpR->opoff();//bytes
//	if (pOpL->OpSize() > pOpR->OpSize())
//		return pOut;//?
	r1 = ~((~0) << pOpL->size());//1maskbit == 1Byte of size
	r2 = ~((~0) << pOpR->size());
	if (d > 0)
		r1 <<= d;
	else if (d < 0)
		r2 <<= d;

	if ( !(r2 & (~r1)) )
		return 1;//total overlap

	return 0;
}

int EXPRSimpl_t::SimplifyGet(Out_t* pSelf, bool b) const
{//return 0;
	if (!pSelf->is(ACTN_GET))
		return 0;

	TYP_t tL(TypOf(pSelf->mpL));
	TYP_t tR(TypOf(pSelf->mpR));
	if (tL.size() == tR.size())
		//if (pSelf->mpL->isEqualTo(pSelf->mpR))
		if (!pSelf->mpU->isComparAction0())
		{
			Dispose(DetachParent(pSelf->mpR));
			return RETURN_1(GET1);//get rid of all of them!
		}

	// a) A := (B = C) => (B = C)
	// b) CMPRSN0(A := (B = C)) => (B = C)
	//this kind of MOV is going to be eliminated
	if (pSelf->mpR->is(ACTN_MOV))
	{
		Out_t* pMov = pSelf->mpR;
		if (pSelf->mpL->isEqualTo(pMov->mpL))
		{
			TYP_t T1(TypOf(pSelf->mpL));
			//				if (T1.isPtr())
			//					t = OPTYP_DWORD;

			if (pSelf->mpL->opc() == OPC_t(OPC_FLAGS | SSID_CPUSW))
				if (pSelf->mpU->isComparAction0())
					DetachParent(pSelf);

			DetachParent(pMov);//ACTN_GET/this

			TYP_t T2(TypOf(pMov->mpL));
			uint8_t t1(T1.optyp());
			uint8_t t2(T2.optyp());
			if (!AgreeTypes(t1, t2))
			{
				InsertParent(pMov, fromOpType(t1));
			}
			else
			{
				pMov->mpL->setTyp(fromOpType(t2));
			}

			return RETURN_1(GET2);
		}

		/*			if (mpL->isOpKind())
					if (mpR->mpL->isOpKind())
					{
						OpPtr pOpL = mpL->m_p Op0;
						OpPtr pOpR = mpR->mpL->m_p Op0;
						if (mpR->CheckThrough(pOpL, pOpR))
						{
							DetachParent(mpR);
							return 1;
						}
					}*/
	}

	if (pSelf->mpL->isOpKind())
	{
		Out_t* pOut2 = pSelf->mpR;
		if (pOut2->is(ACTN_CHECK))
		{
		}
		else
		{
			if (pOut2->is(ACTN_MOV))
				pOut2 = pOut2->mpL;

			if (pOut2->isOpKind())
			{
				int res = 0;
				if (!b)
					res = CheckThrough3(*pSelf->mpR, pSelf->mpL, pOut2);
				else
					res = CheckThrough(*pSelf->mpR, pSelf->mpL, pOut2);
				if (res)
				{
					DetachParent(pSelf->mpR);
					return RETURN_1(GET3);
				}
			}
		}
	}

	//		if (mpR->is(ACTN_CHECK))
	{
		if (pSelf->mpL->isOpKind()
			&& pSelf->mpR->mpL
			&& pSelf->mpR->mpL->isOpKind())
		{
			Out_t* pOut2 = pSelf->mpR->mpL;

			if (!pSelf->mpL->mpOp)
				return 0;

			Action_t A = ActionOf(PrimeOp(pSelf->mpL->mpOp));

			if (pSelf->mpL->opc() == OPC_t(OPC_FLAGS | OPC_CPUSW))
			{
				if (pOut2->opc() == OPC_t(OPC_FLAGS | OPC_CPUSW))
				{
					if ((pSelf->mpL->m_eflags & pOut2->m_eflags) == pSelf->mpL->m_eflags)
					{
						DetachParent(pSelf->mpR);//this
						pOut2->m_eflags = pSelf->mpL->m_eflags;
						//?pOut2->SetDockOp(nullptr);
						return RETURN_1(IND2);
					}

					/*						if (pOut2->opid() == CSWID_CF)
											if (A == ACTN_LESS)
											{
												DetachParent(mpR);//ACTN_GET
												return 1;
											}*/
				}

				/*					if (ISJMPIF(A))
									{
										Out_t *pOutC = 0;//mpU;
										Action_t A2 = IFCOND2ACTN((Action_t)A);
										if (A2 == ACTN_NULL)//fix later
										{
											uint32_t ifcond = A&0xf;
											bool bInvert = ifcond & 1;
											ifcond &= ~1;
											if (ifcond == IFCOND_P)
											{
				//								assert(pOp->opc() == OPC_ CPUSW);
				//								if (bInvert)
				//									mpU = mpU->Add(ACTN_LOGNOT);
				//								mpU = mpU->Add(ACTN_PARITY);
				//								pOp->OutExpression(mpU);
				//								return outsv.get();
											}
											else
												assert(false);
										}
										else
										{
											Out_t *pOut = mpR;
											if (pOut->is(ACTN_CHECK))
											{
												pOut = pOut->mpR;
												DetachParent(pOut);//ACTN_CHECK
											}
											DetachParent(pOut);//ACTN_GET

											pOutC = pOut->InsertParent(A2);//mpU->Add(A2);
										//	if (pOp)
										//		pOp->OutExpression(pOutC);
										}

										return 1;
									}*/
			}
			else
				if (pSelf->mpL->opc() == (OPC_FLAGS | OPC_FPUSW))
				{
					DetachParent(pSelf->mpR);//ACTN_MOV
					return RETURN_1(IND3);
				}
		}
	}

	return 0;
}

int EXPRSimpl_t::SimplifyAssign(Out_t *pSelf) const
{//return 0;

	if (!pSelf->is(ACTN_MOV))
		return 0;
	if (pSelf->mpU->isTerm())//is root?
		return 0;
	if (pSelf->mpU->is(ACTN_VAR))
		return 0;
	if (pSelf->mpU->is(ACTN_GET))
		return 0;
	if (pSelf->mpU->is(ACTN_COMMA2))
		return 0;
	if (pSelf->mpU->is(ACTN_SEMI))
		return 0;
	if (pSelf->mpU->is(ACTN_CHECK))
		return 0;

//CHECK(pSelf->mpU->is(ACTN_FRACT))
//STOP
	if (!pSelf->mpL->isTerm())
		return 0;
	if (pSelf->mpL->isArgKind())
		return 0;
	if (pSelf->mpR->is(ACTN_GET))
		return 0;

	if (pSelf->m_bPostCall)
		return 0;
	//	Out_t *pOut = mpR;
	//	if (pOut->is(ACTN_GET) || pOut->is(ACTN_MOV))
	//		pOut = pOut->mpL;

	TYP_t TL(TypOf(pSelf->mpL));
	TYP_t TL_m(TL.stripModifier());

	TYP_t tR(TypOf(pSelf->mpR).stripModifier());

	if (!TL_m.optyp() || !tR.optyp())
		return 0;
	/*if(0)
	if (TL_m.size() != tR.size())
	{
		Out_t* pType(InsertParent(pSelf->mpR, fromOpType(TL_m.optyp())));
	}*/

	TYP_t TR(TypOf(pSelf->mpR));
	if (pSelf->mpR->is(ACTN_TYPE))
	{
		if (TR != TL)
		{
			//make sure the typecast is exact (w typedefs)
			//the inner typecast is going to be eliminated at further steps
			//a=(type)b => a(type2)(type)b
			InsertParent(pSelf->mpR, TL);
			return RETURN_1(ASS1);
		}
		if (!TR.canPromoteTo(TL_m))
			return 0;
		if (pSelf->mpL->isArgKind())
			TR = TypOf(pSelf->mpR->mpR);
	}
/*	else if (pSelf->mpR->is(ACTN_CALL))
	{
		if (pSelf->mpR->mpL->is(ACTN_TYPE))
			return 0;
	}*/
	else if (!pSelf->mpR->isTerm())
		if (pSelf->mpL->isArgKind())
			return 0;
		
	if (!TR.canPromoteTo(TL_m))
		return 0;

	DetachParent(pSelf->mpR);//ACTN_MOV
	return RETURN_1(ASS2);
}

int EXPRSimpl_t::SimplifyAssign2(Out_t *pSelf) const
{
	if (!pSelf->is(ACTN_MOV))
		return 0;

	if (pSelf->mpU->is(ACTN_GET))
		return 0;

	TYP_t T(TypOf(pSelf));

	Out_t *pL(pSelf->mpL);
	assert(pL);
	Out_t *pR(pSelf->mpR);
	if (pL->is(ACTN_COMMA))
		pL = pL->mpL;

	TYP_t TL(TypOf(pL));
	TYP_t TR(TypOf(pR));

	if (TL.isNull() || TR.isNull())
		return 0;

	TYP_t TL_pxy(ProxyOf(pL));
	TYP_t TR_pxy(ProxyOf(pR));

	if (TR_pxy.canPromoteTo(TL_pxy))
		return 0;

	if (TL.size() == TR.size())
//	if (!TL.isReal() && !TR.isReal())
	{
		if (pSelf->mpR->isNumZero())
		{
			pR->setTyp(TL);
			return RETURN_1(ASS3);
		}
	}

	// *a = b   =>   *(typeof_b*)a = b
	if (pL->is(ACTN_INDIR))
	{
		TYP_t TLR_d(DerefOf(pL->mpR));
		if (!AreCompliant(TL, TLR_d))
		{
			InsertParent(pL->mpR, PtrOf(TR));
			return RETURN_1(ASS4);
		}
	}
#if(1)
	//a=b => a=(typeof_a)b
	//if (!pSelf->mpR->is(ACTN_TYPE))
	//if (!TR.canPromoteTo(TL))
	if (!IsUnfoldMode() && !mbDeadOp)
	{
		if (pR->is(ACTN_INDIR))
		{
			// a=(*b)   =>   a=(*(typeof_a*)b)
			InsertParent(pR->mpR, PtrOf(TL));
			return RETURN_1(ASS5);
		}

		//InsertParent(pSelf->mpR, TL);
		//return RETURN_1(ASS6);
	}
#endif

	return 0;
}

int EXPRSimpl_t::SimplifyAssCheck(Out_t *pSelf) const//can it be done at dumping phase???
{//return 0;
	if (!pSelf->is(ACTN_ASSCHK))
		return 0;

	assert(pSelf->mpR->opc() == OPC_t(OPC_FLAGS|OPC_CPUSW));

	// (a`csw(f)) = expr => csw(f) ~~ a = expr
	Out_t *mpU(pSelf->mpU);
	if (mpU->mpU->is(ACTN_VAR))
		return 0;

	if (mpU->is(ACTN_MOV))
		if (pSelf->isLeft())
	{
		Out_t *mpR(pSelf->mpR);
		Detach(mpR);
		Dispose(DetachParent(pSelf->mpL));
		mpU = InsertParent(mpU, ACTN_CHECK);
		mpU->flipChilds();
		mpU->AddChild(mpR);
		return RETURN_1(PRE3);
	}

	return 0;
}

int EXPRSimpl_t::SimplifyFract(Out_t *pSelf) const
{
	if (pSelf->is(ACTN_FRACT))//try to shift it down
	{
		if (ShiftFractal(pSelf))
			return 1;
		if (AdjustFractal(pSelf))
			return 1;
//		if (UniteFractal(pSelf))
//			return 1;
	}

	if (pSelf->is(ACTN_UNITE))
	{
		if (UniteFractal(pSelf))
			return 1;
	}

	return 0;
}

int EXPRSimpl_t::ShiftFractal(Out_t *pSelf) const
{
	if (!pSelf->is(ACTN_FRACT))
		return 0;

	Out_t *pOutOffs1 = 0;
	Out_t *pOut1 = pSelf->mpL;
	if (pOut1->is(ACTN_SHIFT))
	{
		pOutOffs1 = pOut1;
		pOut1 = pOut1->mpR;
	}

	Out_t *pOutOffs2 = 0;
	Out_t *pOut2 = pSelf->mpR;
	if (pOut2->is(ACTN_SHIFT))
	{
		pOutOffs2 = pOut2;
		pOut2 = pOut2->mpR;
	}

	Action_t action = pOut1->mAction;
//	if (!pOut1->isTerm())
//	if (!pOut1->is(ACTN_GET))
	if (!pOut1->is(ACTN_MOV))
		return 0;
	if (!pOut1->is(pOut2->mAction))
		return 0;

	DetachParent(pOut1->mpL);
	DetachParent(pOut2->mpL);

	Out_t *pOut = InsertParent(pSelf, action);
	pOut = Add(*pOut, HOP(), ACTN_FRACT);

	Out_t *pOut_1 = pOut1->mpR;
	Out_t *pOut_2 = pOut2->mpR;
	DetachParent(pOut_1);
	DetachParent(pOut_2);

	if (pOutOffs1)
	{
		pOut1 = Add(*pOut, *pOutOffs1, true);
		Add(*pOut1, *pOut_1);
	}
	else
		Add(*pOut, *pOut_1);

	if (pOutOffs2)
	{
		pOut2 = Add(*pOut, *pOutOffs2, true);
		Add(*pOut2, *pOut_2);
	}
	else
		Add(*pOut, *pOut_2);

	return RETURN_1(FRA1);
}

int Out_t::setOffset(int ofs)
{	
	if (isOpKind())
	{
		assert(opc() != OPC_GLOBAL);
		mOffs = ofs;
	}
	else if (isNumKind())
	{
		if (ofs > 0)
			m_value.ui64 >>= abs(ofs)*8;
		else if (ofs < 0)
			m_value.ui64 <<= abs(ofs)*8;
	}
	return 1;
}

int EXPRSimpl_t::RetypeTerm(Out_t *pSelf, OpType_t t) const
{
	assert(OPSIZE(t) > 0);
	pSelf->setTyp(fromOpType(t));
	if (pSelf->isNumKind())
	{
		uint64_t mask = -1;
		mask <<= OPSIZE(t) * 8;
		pSelf->m_value.ui64 &= ~mask;
	}
	return 1;
}

int EXPRSimpl_t::ReSize(Out_t &rSelf, int db, int dt) const
{
//	if (d == 0)
//		return 1;
//	assert(d < Size());
	if (!rSelf.isTerm())
		return 0;

	int ofs0 = rSelf.opoff();
	int siz0 = rSelf.size();

	int ofs = ofs0 + db;
	int siz = (ofs0 + siz0) + dt - ofs;
	if (siz <= 0)
		return 0;

	if (rSelf.isOpKind())
	{
		rSelf.setOffset(ofs);
		RetypeTerm(&rSelf, MAKETYP(rSelf.typ().optyp(), siz));
	}
	else if (rSelf.isNumKind())
	{
		rSelf.setOffset(ofs);
		RetypeTerm(&rSelf, MAKETYP(rSelf.typ().optyp(), siz));
	}
	else
		ASSERT0;

	return 1;
}

int EXPRSimpl_t::AdjustFractal(Out_t *pSelf) const
{
	assert(pSelf->is(ACTN_FRACT));

	Out_t *pOut1 = pSelf->mpL;
	Out_t *pOut2 = pSelf->mpR;

	int ofs1 = 0;
	if (pOut1->is(ACTN_SHIFT))
	{
		ofs1 = pOut1->opoff();
		pOut1 = pOut1->mpR;
	}

	int ofs2 = 0;
	if (pOut2->is(ACTN_SHIFT))
	{
		ofs2 = pOut2->opoff();
		pOut2 = pOut2->mpR;
	}

	TYP_t t1(TypOf(pOut1));
	TYP_t t2(TypOf(pOut2));

	int siz1(t1.size());
	int siz2(t2.size());

//if (!(siz1 && siz2))
//return 0;

	if (!(siz1 != 0 && siz2 != 0))
		return 0;

	int d_bottom = ofs1;
	int d_top = ofs1 + siz1;

	if (ofs2 <= ofs1)
		d_bottom = ofs2 + siz2;
	else //if (ofs2 > ofs1)
		d_top = ofs2;

	d_bottom -= ofs1;
	d_top -= (ofs1 + siz1);

	assert(d_bottom >= 0 && d_top <= 0);//shrink only!

	if (!d_bottom && !d_top)
	{
		if (ofs1 > ofs2)
		{
			pSelf->flipChilds();
			return RETURN_1(FRA2);
		}

		pSelf->mAction = ACTN_UNITE;
		pSelf->flipChilds();
		if (pSelf->mpL->is(ACTN_SHIFT))
			DetachParent(pSelf->mpL->mpR);
		if (pSelf->mpR->is(ACTN_SHIFT))
			DetachParent(pSelf->mpR->mpR);
		return RETURN_1(FRA3);;
	}

	if (!ReSize(*pOut1, d_bottom, d_top))
		return 0;

	if (d_bottom)
	{
		Out_t *pOutOffs = InsertParent(pOut1, ACTN_SHIFT);
		pOutOffs->mOffs = d_bottom;
		pSelf->flipChilds();
		return RETURN_1(FRA4);
	}

	return 0;
}

//T64 = (hi((int64_t)eax):eax)
int EXPRSimpl_t::AlignFractal(Out_t *pSelf, Out_t *pOutHi, int offs) const
{
	if (!pSelf->isOpKind())
		return 0;

	if (pOutHi->is(ACTN_HIHALF))
	{
		assert(offs > 0);
		if (pOutHi->mpR->size() != offs*2)
			return 0;

		offs -= pSelf->size();
		if (AlignFractal(pSelf, pOutHi->mpR, offs))
		{
			DetachParent(pOutHi->mpR);//killself
			return RETURN_1(FRB1);
		}
	}
	else if (pOutHi->is(ACTN_TYPE))
	{//check if pOut->mpR's part of size <offs> is changed with type action
		if (pOutHi->mpR->size() < pSelf->size())
			return 0;
		if (TypOf(pOutHi).isReal())
			return 0;
		//not changed
		return AlignFractal(pSelf, pOutHi->mpR, offs);
	}
	else if (pOutHi->isOpKind())
	{
		if (offs != 0)
			return 0;
		if (pSelf->opc() == pOutHi->opc())
		if (pSelf->opoff() == pOutHi->opoff())
		if (pSelf->size() <= pOutHi->size())
		{
			assert(0);
			return 1;//?
		}
	}

	return 0;
}

int EXPRSimpl_t::IsUnitableWith(Out_t *pSelf, Out_t *pOut1, Out_t *pOut2) const
{
	assert(pSelf->is(ACTN_UNITE));
	assert(pSelf->mpL == pOut2);
	assert(pSelf->mpR == pOut1);

	TYP_t t(TypOf(pSelf));
	TYP_t t1(TypOf(pOut1));
	TYP_t t2(TypOf(pOut2));

//	REG_t r1(offs1, pOut1->Size());
//	REG_t r2(offs2, pOut2->Size());
//	r1.MergeWith(r2);

	int sz_new = t1.size() + t2.size();
	int doffs = t1.size();//offs2 - offs1;

	if (pOut1->isOpKind())
	{
/*		if (pOut2->SSID() == SSID_SEGREG)
		{
			int offs = pOut2->opoff();
			OpSeg_t seg = (OpSeg_t)((offs / OPSZ_WORD) + 1);
			assert(!pOut1->m_seg);
			pOut1->m_seg = seg;

			if (pOut1->mpU->is(ACTN_SHIFT))
				pOut1->DetachParent();//ACTN_SHIFT
			pOut1->DetachParent();//ACTN_FRACT
			return 1;
		}*/

		if (pOut2->isOpKind())
		{
//			if (pOut1->m_pOp->IsL ocal())
//			if (pOut2->m_pOp->IsLo cal())
			if (pOut1->opc() == pOut2->opc())
			{
				if (pOut1->mpU->is(ACTN_SHIFT))
					DetachParent(pOut1);//ACTN_SHIFT
				DetachParent(pOut1);//ACTN_FRACT
				RetypeTerm(pOut1, MAKETYP(t1.optyp(), sz_new));//r1.m_siz);
//				if (is(ACTN_UNITE))
//					mpR->DetachParent();
				return RETURN_1(FRA6);
			}
		}
	}
	
	if (pOut2->isNumKind())
	{
		if (pOut2->isNumZero())
		{
			if (pOut1->mpU->is(ACTN_SHIFT))
				DetachParent(pOut1);//ACTN_SHIFT
			DetachParent(pOut1);//ACTN_FRACT
//			pOut1->SetSize(r1.m_siz);
#if(0)
			Out_t *pOutType = pOut1->InsertParent(ACTN_TYPE);
			TypOf(pOutType).Setup(MAKETYP_UINT(r1.m_siz);
#else
			Out_t *pOutZ = InsertParent(pOut1, ACTN_ZEROEXT);
			pOutZ->setTyp(fromOpType(MAKETYP_UINT(sz_new)));//r1.m_siz);
#endif
//			if (is(ACTN_UNITE))
//				mpR->DetachParent();
			return RETURN_1(FRA7);
		}
		if (pOut1->isNumKind())
		{
			int size_new = t1.size() + t2.size();
			assert(size_new <= sizeof (OPSZ_QWORD));
			memcpy((char *)&pOut1->m_value + t1.size(), &pOut2->m_value, t2.size());
			RetypeTerm(pOut1, MAKETYP(t1.optyp(), size_new));
			DetachParent(pOut1);
			return RETURN_1(FRA8);
		}
	}

	if (pOut1->is(ACTN_INDIR))
	if (pOut2->is(ACTN_INDIR))
	{
		int d1 = 0;
		int d2 = 0;
		if (ExtractDisplacement2(pOut1->mpR, d1))
		if (ExtractDisplacement2(pOut2->mpR, d2))
		{
			if (d2 - d1 != doffs)
				return 0;
			d1 = 0;
			if (!ExtractDisplacement2(pOut1->mpR, d1, true))
				ASSERT0;
			d2 = 0;
			if (!ExtractDisplacement2(pOut2->mpR, d2, true))
				ASSERT0;
			if (!pOut1->mpR->isEqualTo(pOut2->mpR))
				ASSERT0;

			if (pOut1->mpU->is(ACTN_SHIFT))
				DetachParent(pOut1);//ACTN_SHIFT
			DetachParent(pOut1);//ACTN_FRACT
			if (d1)
			{
				Out_t *pOutAdd = InsertParent(pOut1->mpR, ACTN_ADD);
				Add(*pOutAdd, HOP(), d1);
			}

			pOut1->setTyp(fromOpType((t.optyp() & OPTYP_MASK) | (sz_new & OPSZ_MASK)));//r1.m_siz;

//			if (is(ACTN_UNITE))
//				pOut1->DetachParent();
			return RETURN_1(FRA9);
		}
	}

	if (pOut1->is(ACTN_LOHALF))
	if (pOut2->is(ACTN_HIHALF))
	{
		if (pOut1->mpR->isEqualTo(pOut2->mpR))
		{
			DetachParent(pOut1);
			DetachParent(pOut1->mpR);
			return RETURN_1(FRB2);
		}
	}

	if (pOut2->is(ACTN_HIHALF))
		if (pOut2->mpR->is(ACTN_SIGNEXT) || pOut2->mpR->is(ACTN_ZEROEXT))
			if (pOut2->mpR->mpR->isEqualTo(pOut1))
			{
				Out_t *pSignExt(pOut2->mpR);
				DetachParent(pSignExt);//HIHALF
				DetachParent(pSignExt);//UNITE
				return RETURN_1(FRB3);
			}

	return 0;
}
/*
//a(:)b => @(b,a)
int Out_t::UniteFractal()
{
	assert(is(ACTN_FRACT));

	Out_t *pOut1 = mpL;
	Out_t *pOut2 = mpR;

	int offs1 = 0;
	int offs2 = 0;

  if (pOut1->is(ACTN_SHIFT))
	{
		offs1 = pOut1->opoff();
		pOut1 = pOut1->mpR;
	}
	if (pOut2->is(ACTN_SHIFT))
	{
		offs2 = pOut2->opoff();
		pOut2 = pOut2->mpR;
	}

	if (1)
	if (->AlignFractal(pOut1,pOut2, offs2-offs1))
	{
		if (mpR->is(ACTN_SHIFT))
			mpR->mpR->DetachParent();//ACTN_SHIFT
		mpR->DetachParent();//killself
		if (is(ACTN_UNITE))
			pOut1->DetachParent();
		return 1;
	}

	if (IsUnitableWith(pOut1, pOut2))
	{
		return 1;
	}

	return 0;
}
*/
//a(:)b => 
int EXPRSimpl_t::UniteFractal(Out_t *pSelf) const
{
	assert(pSelf->is(ACTN_UNITE));

	Out_t *pOut1 = pSelf->mpL;
	Out_t *pOut2 = pSelf->mpR;

//	int offs1 = 0;
//	int offs2 = 0;

//	if (is(ACTN_UNITE))
	{
		pOut1 = pSelf->mpR;//ofs
		pOut2 = pSelf->mpL;//seg
//		offs2 = pOut1->Size();
	}
/*	else
	{
		if (pOut1->is(ACTN_SHIFT))
		{
			offs1 = pOut1->opoff();
			pOut1 = pOut1->mpR;
		}
		if (pOut2->is(ACTN_SHIFT))
		{
			offs2 = pOut2->opoff();
			pOut2 = pOut2->mpR;
		}
	}*/

	TYP_t t1(TypOf(pOut1));

	int doffs = t1.size();
	if (!doffs)
		return 0;

/*	if (!pOut1->isTerm())
	if (!pOut1->is(ACTN_INDIR))
		return 0;
	if (!pOut2->isTerm())
	if (!pOut2->is(ACTN_INDIR))
		return 0;
*/
	if (1)
	if (AlignFractal(pOut1, pOut2, doffs))//offs2-offs1))
	{
		if (pSelf->mpR->is(ACTN_SHIFT))
			DetachParent(pSelf->mpR->mpR);//ACTN_SHIFT
		DetachParent(pSelf->mpR);//killself
//		if (is(ACTN_UNITE))
//			pOut1DetachParent();
		return RETURN_1(FRA5);
	}

	if (IsUnitableWith(pSelf, pOut1, pOut2))
	{
		return 1;
	}

	return 0;
}

int EXPRSimpl_t::SimplifyRawCondition(Out_t *pSelf) const
{
	if (!pSelf->isComparAction0())
		return 0;
	if (!pSelf->mpR->isOpKind())
		return 0;
	if (pSelf->mpR->opc() != OPC_t(OPC_FLAGS|OPC_CPUSW))
		return 0;

	if (!pSelf->mpR->mpOp)
		return 0;
	if (IsPrimeOp(pSelf->mpR->mpOp))
		return 0;

	IfCond_t cond = (IfCond_t)(pSelf->mAction & 0xF);
	XOpLink_t *pXIns = pSelf->mpR->mpOp->XIn();

	Out_t *pOut = pSelf->mpU;
	Detach(pSelf);
	pSelf->mpU = pOut;

	Out_t *pOutTop;
	Out_t *mpL, *mpR;
	HOP pDockOp(pSelf->dockOp());
	
	switch (cond)
	{
	case IFCOND_O://+0//(OF != 0):
		pOutTop = Add(*pSelf->mpU, pDockOp, ACTN__NOTEQUAL);
		Add2(pOutTop, pDockOp, OPC_CPUSW, CSW_OF, OPSZ_BIT)->setXIn(pXIns);
		Add(*pOutTop, pDockOp, (int)0);
		break;
	case IFCOND_NO://+1//(OF == 0):
		pOutTop = Add(*pSelf->mpU, pDockOp, ACTN__EQUAL);
		Add2(pOutTop, pDockOp, OPC_CPUSW, CSW_OF, OPSZ_BIT)->setXIn(pXIns);
		Add(*pOutTop, pDockOp, (int)0);
		break;
	case IFCOND_B://+2//(CF != 0):{<}
		pOutTop = Add(*pSelf->mpU, pDockOp, ACTN__NOTEQUAL);
		Add2(pOutTop, pDockOp, OPC_CPUSW, CSW_CF, OPSZ_BIT)->setXIn(pXIns);
		Add(*pOutTop, pDockOp, (int)0);
		break;
	case IFCOND_AE://+3//(CF == 0):{>=}
		pOutTop = Add(*pSelf->mpU, pDockOp, ACTN__EQUAL);
		Add2(pOutTop, pDockOp, OPC_CPUSW, CSW_CF, OPSZ_BIT)->setXIn(pXIns);
		Add(*pOutTop, pDockOp, (int)0);
		break;
	case IFCOND_E://+4//(ZF != 0):{==}
		pOutTop = Add(*pSelf->mpU, pDockOp, ACTN__NOTEQUAL);
		Add2(pOutTop, pDockOp, OPC_CPUSW, CSW_ZF, OPSZ_BIT)->setXIn(pXIns);
		Add(*pOutTop, pDockOp, (int)0);
		break;
	case IFCOND_NE://+5//(ZF == 0):{!=}
		pOutTop = Add(*pSelf->mpU, pDockOp, ACTN__EQUAL);
		Add2(pOutTop, pDockOp, OPC_CPUSW, CSW_ZF, OPSZ_BIT)->setXIn(pXIns);
		Add(*pOutTop, pDockOp, (int)0);
		break;
	case IFCOND_BE://+6//[(CF != 0) || (ZF != 0)]:{<=}
		pOutTop = Add(*pSelf->mpU, pDockOp, ACTN_LOGOR);
		mpL = Add(*pOutTop, pDockOp, ACTN__NOTEQUAL);
		Add2(mpL, pDockOp, OPC_CPUSW, CSW_CF, OPSZ_BIT)->setXIn(pXIns);
		Add(*mpL, pDockOp, (int)0);
		mpR = Add(*pOutTop, pDockOp, ACTN__NOTEQUAL);
		Add2(mpR, pDockOp, OPC_CPUSW, CSW_ZF, OPSZ_BIT)->setXIn(pXIns);
		Add(*mpR, pDockOp, (int)0);
		break;
	case IFCOND_A://+7//[(CF == 0) && (ZF == 0)]:{>}
		pOutTop = Add(*pSelf->mpU, pDockOp, ACTN_LOGAND);
		mpL = Add(*pOutTop, pDockOp, ACTN__EQUAL);
		Add2(mpL, pDockOp, OPC_CPUSW, CSW_CF, OPSZ_BIT)->setXIn(pXIns);
		Add(*mpL, pDockOp, (int)0);
		mpR = Add(*pOutTop, pDockOp, ACTN__EQUAL);
		Add2(mpR, pDockOp, OPC_CPUSW, CSW_ZF, OPSZ_BIT)->setXIn(pXIns);
		Add(*mpR, pDockOp, (int)0);
		break;
	case IFCOND_S://+8//(SF != 0):
		pOutTop = Add(*pSelf->mpU, pDockOp, ACTN__NOTEQUAL);
		Add2(pOutTop, pDockOp, OPC_CPUSW, CSW_SF, OPSZ_BIT)->setXIn(pXIns);
		Add(*pOutTop, pDockOp, (int)0);
		break;
	case IFCOND_NS://+9//(SF == 0):
		pOutTop = Add(*pSelf->mpU, pDockOp, ACTN__EQUAL);
		Add2(pOutTop, pDockOp, OPC_CPUSW, CSW_SF, OPSZ_BIT)->setXIn(pXIns);
		Add(*pOutTop, pDockOp, (int)0);
		break;
	case IFCOND_P://+A//(PF != 0):
		pOutTop = Add(*pSelf->mpU, pDockOp, ACTN__NOTEQUAL);
		Add2(pOutTop, pDockOp, OPC_CPUSW, CSW_PF, OPSZ_BIT)->setXIn(pXIns);
		Add(*pOutTop, pDockOp, (int)0);
		break;
	case IFCOND_NP://+B//(PF == 0):
		pOutTop = Add(*pSelf->mpU, pDockOp, ACTN__EQUAL);
		Add2(pOutTop, pDockOp, OPC_CPUSW, CSW_PF, OPSZ_BIT)->setXIn(pXIns);
		Add(*pOutTop, pDockOp, (int)0);
		break;
	case IFCOND_L://+C//(SF != OF):{<}
		pOutTop = Add(*pSelf->mpU, pDockOp, ACTN__NOTEQUAL);
		Add2(pOutTop, pDockOp, OPC_CPUSW, CSW_SF, OPSZ_BIT)->setXIn(pXIns);
		Add2(pOutTop, pDockOp, OPC_CPUSW, CSW_OF, OPSZ_BIT)->setXIn(pXIns);
		break;
	case IFCOND_GE://+D//(SF == OF):{>=}
		pOutTop = Add(*pSelf->mpU, pDockOp, ACTN__EQUAL);
		Add2(pOutTop, pDockOp, OPC_CPUSW, CSW_SF, OPSZ_BIT)->setXIn(pXIns);
		Add2(pOutTop, pDockOp, OPC_CPUSW, CSW_OF, OPSZ_BIT)->setXIn(pXIns);
		break;
	case IFCOND_LE://+E//((ZF != 0) || (SF != OF)):{<=}
		pOutTop = Add(*pSelf->mpU, pDockOp, ACTN_LOGOR);
		mpL = Add(*pOutTop, pDockOp, ACTN__NOTEQUAL);
		Add2(mpL, pDockOp, OPC_CPUSW, CSW_ZF, OPSZ_BIT)->setXIn(pXIns);
		Add(*mpL, pDockOp, (int)0);
		mpR = Add(*pOutTop, pDockOp, ACTN__NOTEQUAL);
		Add2(mpR, pDockOp, OPC_CPUSW, CSW_SF, OPSZ_BIT)->setXIn(pXIns);
		Add2(mpR, pDockOp, OPC_CPUSW, CSW_OF, OPSZ_BIT)->setXIn(pXIns);
		break;
	case IFCOND_G://+F//((ZF == 0) && (SF == OF)):{>}
		pOutTop = Add(*pSelf->mpU, pDockOp, ACTN_LOGAND);
		mpL = Add(*pOutTop, pDockOp, ACTN__EQUAL);
		Add2(mpL, pDockOp, OPC_CPUSW, CSW_ZF, OPSZ_BIT)->setXIn(pXIns);
		Add(*mpL, pDockOp, (int)0);
		mpR = Add(*pOutTop, pDockOp, ACTN__EQUAL);
		Add2(mpR, pDockOp, OPC_CPUSW, CSW_SF, OPSZ_BIT)->setXIn(pXIns);
		Add2(mpR, pDockOp, OPC_CPUSW, CSW_OF, OPSZ_BIT)->setXIn(pXIns);
		break;
	default:
		assert(false);
	}

	return RETURN_1(CND1);
}

int EXPRSimpl_t::SimplifyHalf1(Out_t* pSelf) const
{
	if (pSelf->is(ACTN_LOHALF) || pSelf->is(ACTN_HIHALF))
	{
		//hi(I1) => I2	|I1==imm16,I2==I1>>sizeof(I1)/2
		Out_t* mpR(pSelf->mpR);
		if (mpR->isImmidiate())
		{
			uint8_t t0(mpR->typ().optyp());
			uint8_t t1(t0 & OPTYP_MASK);
			uint8_t t2(t0 & OPSZ_MASK);

			int i = ((t0 & OPSZ_MASK) << 2);
			int mask((1 << i) - 1);
			if (pSelf->is(ACTN_HIHALF))
				mpR->m_value.i64 >>= i;
			mpR->m_value.i64 &= mask;
			uint8_t t3(t2 >> 1);//new opsz
			if (t1 && !OPTYP_IS_INT(t1))
				t1 = 0;//?
			mpR->setTyp(fromOpType(t1 | t2));
			DetachParent(mpR);
			return RETURN_1(LOH1);
		}

		//(hi(OP)&I1) => OP & I2	|I2==I1<<xx
		//(lo(OP)&I1) => OP & I2	|I2==I1&xx
		if (pSelf->mpU->is(ACTN_AND))
		{
		}
	}

	if (pSelf->is(ACTN_LOHALF))
	{
		//lo(OP1+OP2) == lo(OP1)+lo(OP2)
		//hi(OP1+OP2) != hi(OP1)+hi(OP2)	!!!
		if (pSelf->mpR->is(ACTN_ADD) || pSelf->mpR->is(ACTN_SUB))
		{
			Out_t* pOut = pSelf->mpR;
			DetachParent(pOut);
			Out_t* pOut1 = InsertParent(pOut->mpL, pSelf->mAction);
			pOut1->setTyp(TypOf(pSelf));
			Out_t* pOut2 = InsertParent(pOut->mpR, pSelf->mAction);
			pOut2->setTyp(TypOf(pSelf));
			return RETURN_1(LOH2);
		}
	}

	return 0;
}

int EXPRSimpl_t::SimplifyAndOr(Out_t* pSelf) const
{
	if (pSelf->is(ACTN_AND) || pSelf->is(ACTN_OR))
	{
		if (pSelf->mpR->isImmidiate())
		{
/*			if (mpL->is(ACTN_HIHALF))
			{//hi(a)&i1 => hi(a&i2); <i2=i1<<(sizeof(a)/2)>
				Out_t *pOut = mpL;
				int sz = TypOf(mpL).Size();
				VALUE_t v(sz*2, value_t(mpR->m_value.ui64 << sz*8));
				pOut->DetachParent();//kill this
				Out_t *pOutAnd = pOut->mpR->InsertParent(ACTN_AND);
				pOutAnd->Add(v);
				return 1;
			}
			else */if (pSelf->mpL->is(ACTN_CHECK))
			{//(fpusw~~a)&i => fpusw.c0.c1~~a
				if (pSelf->mpL->mpL->SSId() == SSID_FPUSW)
				{
					pSelf->mpL->mpL->m_eflags &= pSelf->mpR->m_value.ui32;
					//?pSelf->mpL->mpL->SetDockOp(nullptr);
					DetachParent(pSelf->mpL);//kill this
					return RETURN_1(CHK1);
				}
			}
		}
	}

	return 0;
}

int	EXPRSimpl_t::SimplifyCheck(Out_t *pSelf) const
{
	if (!pSelf->is(ACTN_CHECK))
		return 0;

	if (!pSelf->mpL->isOpKind())
		return SIMPL_NULL;

	// CMPRSN0(csw(a) ~~ expr)
	if (pSelf->mpL->opc() == OPC_t(OPC_FLAGS|OPC_CPUSW))
	{
		if (pSelf->mpU->isComparAction0() || pSelf->mpU->is(ACTN_LOGNOT))
		{
			DetachParent(pSelf->mpR);
			return RETURN_1(CHK2);
		}
	}

	if (pSelf->mpU->is(ACTN_HIHALF))
	{
		//hi(csw~~a) => csw(..).1~~a
		Out_t *mpL(pSelf->mpL);
//		if (!(mpL->opc() == OPC_CPUSW || mpL->opc() == OPC_FPUSW))
	//		return 0;//?
		uint8_t t0(mpL->typ().optyp());
		uint8_t t1(t0 & OPTYP_MASK);
		uint8_t t2(t0 & OPSZ_MASK);
		//int sz = mpL->Size() / 2;
		int sz(t2 >> 1);
		mpL->mOffs += sz;
		mpL->m_eflags >>= sz * 8;
		mpL->setTyp(fromOpType(t1 | t2));
		//?mpL->SetDockOp(nullptr);
		DetachParent(pSelf);//hi
		return RETURN_1(CHK3);
	}

	if (pSelf->mpL->SSId() == SSID_FPUSW)
	if (pSelf->mpU->is(ACTN_ZERO))
	{
		int mask = pSelf->mpL->m_eflags;
		if (mask)
		{
			if (pSelf->mpL->mOffs)
			{
				assert(pSelf->mpL->mOffs > 0);
				mask <<= pSelf->mpL->mOffs * 8;
			}

			Action_t action;
			switch (mask)
			{
			case FPUSW_C0://0x0100
				//C0==0	=> (>=)
				action = ACTN__GREATEROREQUAL; break;
			case FPUSW_C3://0x4000
				//C3==0	=> (!=)
				action = ACTN__NOTEQUAL;	break;	
			case FPUSW_C3|FPUSW_C0://0x4100
				//C3+C0==0 => (>)
				action = ACTN__GREATER; break;
			default:
				assert(false);
			}

			Out_t *pOut = pSelf->mpU;
			DetachParent(pSelf->mpR);//this
			pOut->mAction = action;
			Add(*pOut, pOut->dockOp(), (int)0);
			pOut->flipChilds();
			return RETURN_1(CHK4);
		}
	}
	else if (pSelf->mpU->is(ACTN_PARITY))
	{
		int mask = pSelf->mpL->m_eflags & 0xFF;//low byte only
		if (pSelf->mpL->mOffs)
		{
			assert(pSelf->mpL->mOffs > 0);
			mask <<= pSelf->mpL->mOffs * 8;
		}

		if (mask)
		{
			Action_t action;
			switch (mask)
			{
			case FPUSW_C2|FPUSW_C0:
				//PARITY((fsw ~~ a) & 0x0500) => (C0==0 && C2==0) => (a >= 0)
				action = ACTN__GREATEROREQUAL; break;
			case FPUSW_C3|FPUSW_C2:
				//PARITY((fsw ~~ a) & 0x4400) => (C2==0 && C3==0) => (a != 0)
				action = ACTN__NOTEQUAL; break;
			case FPUSW_C3|FPUSW_C0:
				//PARITY((fsw ~~ a) & 0x4100) => (C0==0 && C3==0) => (a > 0)
				action = ACTN__GREATER; break;
			default:
				assert(false);
			}

			Out_t *pOut = pSelf->mpU;
			DetachParent(pSelf->mpR);//this
			pOut->mAction = action;
			Add(*pOut, HOP(), (int)0);
			pOut->flipChilds();
			return RETURN_1(CHK5);
		}
	}
	else
	{
		assert(!pSelf->mpU->is(ACTN_NZERO));
		assert(!pSelf->mpU->is(ACTN_NPARITY));
	}
	
	return 0;
}

//replace code op with data one
int EXPRSimpl_t::SimplifyOp2Field(Out_t * pSelf) const
{
	if (IsUnfoldMode())
		return 0;

	OFF_t oData;
	FieldPtr pData(CheckThruConst(pSelf, oData));
	if (!pData)
		return false;

	pSelf->m_value.ui64 = 0;
	GetDataSource()->pvt().dataAt(oData, pData->size(), (PDATA)&pSelf->m_value);
	if (pSelf->m_value.ui64 != 0)
		return 0;

/*	if (pData->m_nFlags & FLD_INVERTED)
	{
		Out_t *pOutDiv = InsertParent(pSelf, ACTN_DIV);
		Add(*pOutDiv, HOP(), 1);
		pOutDiv->flipChilds();

		if (pSelf->OpType() == OPTYP_REAL64)
			pSelf->m_value.r64 = 1/pSelf->m_value.r64;
		else if (pSelf->OpType() == OPTYP_REAL32)
			pSelf->m_value.r32 = 1/pSelf->m_value.r32;
		else
		{
			assert(0);
		}
	}*/

	pSelf->mKind = OUT_IMM;
	pSelf->mSsid = OPC_NULL;//SSID_IMM;
//	pSelf->m_p Op0 = 0;
	return RETURN_1(DAT1);
}


FieldPtr EXPRSimpl_t::GetAttachedLocal(Out_t &rOut) const
{
	if (rOut.xin().empty())
		return 0;
	if (rOut.opc() == OPC_t(OPC_FLAGS|OPC_CPUSW) || rOut.opc() == OPC_FPUSW)
		return 0;

	FieldPtr pData = nullptr;
	for (XOpList_t::Iterator i(rOut.xin()); i; i++)
	{
		OpPtr pOp = i.data();
//		if (!pOp->IsArgOp())
//			if (!pOp->IsRhsOp())
//				if (!pOp->IsCallOutOp())
//					return 0;
		FieldPtr pFieldRef(fieldRefEx(pOp));
		if (!pFieldRef)
			continue;//break;//?
		if (!IsLocal0(pFieldRef))
			break;

		if (!pData || GetDataOffset(pData) == GetDataOffset(pFieldRef))
			pData = pFieldRef;
	}

	return pData;
}

Out_t* EXPRSimpl_t::AttachGlob(Out_t* pSelf) const
{
	//need to make sure there is no other integral up the tree
	Locus_t aLoc;
	ADDR va(ADDR(pSelf->m_value.ui64 - ImageBase()));
	FieldPtr pFieldRef(FindFieldInSubsegs(PrimeSeg(), va, aLoc));
	if (!pFieldRef)
		return nullptr;
//CHECK(pFieldRef->address() == 0x1001398)
CHECKID(pFieldRef, 0x3d)
STOP
	assert(pFieldRef == aLoc.field0());
	aLoc.stripToSeg();
	if (aLoc.field0()->_key() != aLoc.back().addr())
		return nullptr;
	pFieldRef = aLoc.field0();

	if (IsEntryLabel(pFieldRef))
		pFieldRef = pFieldRef->owner()->parentField();//this is a func

	ADDR va2(GetDataOffset(pFieldRef));
	int delta;
	if (va > va2)
		delta  = va - va2;
	else
		delta  = -int(va2 - va);

	pSelf->mKind = OUT_FIELD;
	pSelf->setField(pFieldRef);
	pSelf->mSsid = OPC_GLOBAL;
	pSelf->m_value.clear();
	pSelf->clearTyp();
	
	CGlobPtr pGlob(GlobObj(pFieldRef));
	//if (pGlob)
	{
		if ((pFieldRef->isTypeImp() && pGlob) || pFieldRef->isTypeExp())
		{
			FieldPtr pExpField(ToExportedField(pFieldRef));
			if (pExpField)
			{
				pSelf->mpExpField = pExpField;
				//if (!pGlob->func())
					pSelf->setTyp(fromField(pSelf->mpExpField));
				//move type cast out of deref
				Out_t* peType(pSelf->mpU);
				if (peType->is(ACTN_TYPE))
				{
					if (TypOf(peType).isPtr())//?assert
					{
						//DetachParent(pSelf);
						Out_t* pOutIndir(peType->mpU);
						if (pOutIndir->is(ACTN_INDIR))
						{
							DetachParent(peType);//get rid of indir
							peType->setTyp(DerefOf(peType));
							pSelf->setTyp(TypOf(peType));
						}
					}
				}
				return pSelf;
			}
		}

		//convert into a field
		//if (pGlob && !pGlob->func())
			pSelf->setTyp(fromField(pFieldRef));
		/*else
		{
			OFF_t off;
			if (DcInfo_t::CheckThruConst(pFieldRef, off))
			{
				pSelf->setTyp(fromField(pFieldRef));
			}
		}*/
	}

/*	OFF_t oData;
	if (delta == 0 && FuncInfo_t::CheckThruConst(pFieldRef, oData))
	{
		if (!pSelf->mpU->is(ACTN_TYPE))
		{
			pSelf->m_value.ui64 = 0;
			GetDataSource()->pvt().dataAt(oData, pFieldRef->size(), (PDATA)&pSelf->m_value);
			//if (pSelf->m_value.ui64 != 0)
				//return 0;
			pSelf->mKind = OUT_IMM;
			DetachParent(pSelf);
			return pSelf;
		}
	}*/

	Out_t* peOffs(InsertParent(pSelf, ACTN_OFFS));
	if (delta == 0)
	{
		Out_t* mpU(peOffs->mpU);
		if (mpU->is(ACTN_CALL))
		{
			//Out_t* peType(InsertParent(peOffs, fromCall(mpU)));
			return peOffs;
		}
//CHECK(TypOf(mpU).IsCall())
//STOP
		//if (TypOf(mpU).isImplicitCastOf(TypOf(peOffs)))
			//return peOffs;
		if (mpU->is(ACTN_TYPE) && TypOf(mpU).isCallPtr())//already a ptr cast
			return peOffs;

		//start with char*
		Out_t* peType(InsertParent(peOffs, PtrOf(fromOpType(OPTYP_INT8))));
		return peType;
	}

	Out_t* peType(InsertParent(peOffs, PtrOf(fromOpType(OPTYP_INT8))));

	Out_t* pOutAdd(InsertParent(peType, ACTN_ADD));
	Add(*pOutAdd, HOP(), delta);
	return pOutAdd;
}

int EXPRSimpl_t::SimplifyThruConst(Out_t* pSelf) const
{
	if (IsUnfoldMode())
		return 0;
	if (!pSelf->isFieldKind())
		return 0;
	if (pSelf->mpU->is(ACTN_OFFS))
		return 0;
	CFieldPtr pField(pSelf->field());
	if (!IsGlobal(pField))
		return 0;
CHECK(pField->_key() == 0x50bffc)
STOP
	OFF_t oPos;
	if (!FuncInfo_t::CheckThruConst(pField, oPos))
		return 0;
	pSelf->mKind = OUT_IMM;
	pSelf->m_value.ui64 = 0;
	GetDataSource()->pvt().dataAt(oPos, pField->size(), (PDATA)&pSelf->m_value);
	//DetachParent(pSelf);
	return RETURN_1(IMM6);
}

int EXPRSimpl_t::AttachGlob2(Out_t * pSelf) const
{
	if (IsUnfoldMode() || mbDeadOp)
		return 0;
	if (!pSelf->isTerm())
		return 0;

	if (!pSelf->isOpKind())
	{
		if (!pSelf->isImmidiate())
			return 0;

		if (pSelf->CheckIndirReady2())//run some tests to see if this a reference to a global (not just an offset from some global)
		{
			if (!pSelf->isRight())
				return 0;
		}
		else if (pSelf->opc() != OPC_ADDRESS)
		{
			if (!pSelf->mpU->is(ACTN_TYPE))
				return 0;
			TYP_t T(pSelf->mpU->typ().stripModifier());
			if (!T.isPtr() && !T.isCallPtr())
				return 0;
		}

		if (!AttachGlob(pSelf))
			return 0;
		return RETURN_1(DAT7);
	}

	assert(pSelf->isOpKind());

	//if (pSelf->mpU->is(ACTN_VAR))
	if (pSelf->mpOp)
	if (IsVarOp(pSelf->mpOp))
	{
		FieldPtr pField(LocalRef(pSelf->mpOp));
		TYP_t T(pField->type(), this);
		pSelf->setTyp(T);
		pSelf->mKind = OUT_FIELD;
		pSelf->setField(pField);
		pSelf->mSsid = OPC_NULL;
		pSelf->mOffs = 0;
		return RETURN_1(DAT3);
	}

	FieldPtr pMLoc0(pSelf->fieldRef());
	if (!pMLoc0)
	{
		if (pSelf->SSId() == SSID_GLOBAL)
		{
			Out_t* peCall(pSelf->isLeft() && pSelf->mpU->is(ACTN_CALL) ? pSelf->mpU : nullptr);
			if (pSelf->isOpIndirect())
			{
				Out_t* pIndir(InsertParent(pSelf, ACTN_INDIR));
				pIndir->m_seg = pSelf->m_seg;
				TYP_t Tself;
				if (!peCall)
					Tself = TypOf(pSelf);
				pSelf->mKind = OUT_IMM;
				pSelf->mSsid = OPC_NULL;
				pSelf->m_value.i64 = pSelf->mOffs + ImageBase();
				pSelf->mOffs = 0;
				pSelf->mpOp = HOP();
				pSelf->m_seg = 0;
				pSelf->setTyp(fromOpType(MAKETYP_UINT(PtrSize())));
				InsertParent(pSelf, PtrOf(Tself));
			}
			else
			{
				if (pSelf->mpOp && IsPrimeOp(pSelf->mpOp) && IsCall(pSelf->mpOp) && IsCallIntrinsic(pSelf->mpOp))
					return 0;
				assert(IS_ADDR(pSelf->mSsid));
				pSelf->mKind = OUT_IMM;
				pSelf->mSsid = OPC_NULL;
				pSelf->m_value.i64 = pSelf->mOffs + ImageBase();
				pSelf->mOffs = 0;
				pSelf->mpOp = HOP();
				pSelf->m_seg = 0;
				pSelf->setTyp(PtrOf(TYP_t()));
			}
			return RETURN_1(DAT8);
		}
		if (pSelf->mpOp)
			pMLoc0 = LocalRef(pSelf->mpOp);
		else
		{
			pMLoc0 = GetAttachedLocal(*pSelf);
			if (!pMLoc0 && pSelf->mpOp)
				pMLoc0 = GetAttachedLocalFromOp(pSelf->mpOp);
		}
		/*if (pMLoc0)
		{
			pSelf->setField(pMLoc0);
			pSelf->mKind = OUT_FIELD;
			return RETURN_1(DAT3);
		}*/
	}

	if (!Storage(pSelf->SSId()).isFlag() && IS_ADDR(pSelf->mSsid))
	{
		if (pMLoc0)
		{
			Out_t* peAdd(nullptr);
			int disp = pSelf->opoff() - GetDataOffset(pMLoc0);
			if (disp != 0)//check for string
				peAdd = InsertParent(pSelf, ACTN_ADD);

			pSelf->setTyp(fromField(pMLoc0));//?

			Out_t* peOffs(InsertParent(pSelf, ACTN_OFFS));
			if (peAdd)
				Add(*peAdd, HOP(), disp);


			pSelf->mKind = OUT_FIELD;// MLOC;
			assert(pMLoc0);
			pSelf->setField(pMLoc0);
			pSelf->mSsid = OPC_NULL;
			pSelf->mOffs = 0;

			return RETURN_1(DAT4);
		}

		/*Out_t* peOffs(InsertParent(pSelf, ACTN_OFFS));
		pSelf->mSsid = (OPC_t)(pSelf->mSsid & SSID_MASK);
		pSelf->mpOp = HOP();
		//pSelf->mKind = OUT_NULL;
		return RETURN_1(DAT5);*/
	}

	if (Storage(pSelf->SSId()).isFlag())
		return 0;

	if (pSelf->mpU->is(ACTN_GET))
		return 0;
	if (pSelf->mpU->is(ACTN_FRACT))
		return 0;
	if (pSelf->mpU->mpU)
	if (pSelf->mpU->mpU->is(ACTN_GET))
	if (pSelf->isLeft())
		return 0;

	if (pSelf->mpOp)
	{
		if (IsCode(pSelf->mpOp))
		if (PrimeOp(pSelf->mpOp)->isHidden())
			return 0;
		if (!IsOnTop(pSelf->mpOp))
			return 0;
	}

	FieldPtr pMLoc(pMLoc0);//always of this module
	FieldPtr pMLoc2(pMLoc);//may be external
	if (pMLoc)
	{
		if (pMLoc->isTypeImp())
		{
			FieldPtr pExpField(ToExportedField(pMLoc));
			if (pExpField)
				pMLoc2 = pExpField;
		}
		assert(!pSelf->mpOp || !pSelf->mpOp->IsScalar());
	}
	else 
	{
		pMLoc = GetAttachedLocal(*pSelf);
		if (!pMLoc)
		{
			if (pSelf->mpOp)
				pMLoc = GetAttachedLocalFromOp(pSelf->mpOp);

			if (!pMLoc)
			{
				if (pSelf->mSsid & SSID_THISPTR)
					pSelf->setTyp(fromOp(pSelf->mpOp));
				return 0;
			}
		}

		assert(pMLoc);
		pMLoc2 = pMLoc;
	}

	//if (TypOf(pSelf).isNull())
		//return 0;//?assert

	TYP_t T_fld(pMLoc2->type(), this);
#if(1)//DO IT LATER: notepad(ex1):0x1006468
	if (pMLoc2 != pMLoc)//imported
		T_fld = PtrOf(T_fld);
#endif

	TYP_t T_fld_pxy(ProxyOf(T_fld));
	
	TYP_t Tself(TypOf(pSelf));

	int d(0);
	if (pSelf->mpOp)
		d = - (int)GetDataOffset(pMLoc) + pSelf->opoff();

	//does op's type comply with that of the field?
	if (d != 0)
	{
		//example:
		//struct s_t { double a; int b; double c; } s_t;
		//*(double*)((char*)&s + 12) = 0.666;

		Out_t* peOffs(InsertParent(pSelf, ACTN_OFFS));

		TYP_t Tchar(fromOpType(OPTYP_INT8));

		Out_t* pOutType1(InsertParent(peOffs, PtrOf(Tchar)));

		Out_t* peAdd(InsertParent(pOutType1, ACTN_ADD));
		Add(*peAdd, HOP(), d);

		Out_t* peTop(peAdd);
		if (!AreCompliant(Tself, Tchar))
			peTop = InsertParent(peTop, PtrOf(Tself));

		InsertParent(peTop, ACTN_INDIR);
	}
	else if (!AreCompliant(Tself, T_fld_pxy))
	{
		//example:
		// union { float f; int i; };
		//*(int*)&f = 5;
		Out_t* peOffs(InsertParent(pSelf, ACTN_OFFS));

		Out_t* peType(InsertParent(peOffs, PtrOf(Tself)));

		InsertParent(peType, ACTN_INDIR);
	}

	pSelf->setTyp(T_fld);

	pSelf->mKind = OUT_FIELD;// OUT_ MLOC;
	//pSelf->m_bPostCall = (pSelf->m_p Op0->m_nFlags & OPND_POST) != 0;

	pSelf->setField(pMLoc);//local
	pSelf->mpExpField = pMLoc2;//exported
	assert(pMLoc);

	pSelf->mSsid = OPC_NULL;
	pSelf->mOffs = 0;

	if (!pSelf->mpU->is(ACTN_OFFS))
	{
		if (pSelf->isLeft())
		{
			if (pSelf->mpU->is(ACTN_ADD) && pSelf->typ().isPtr())
			{
				InsertParent(pSelf->mpU, Tself);
			}
			//a=b => a=(typeof_a)b
/*			if (pSelf->mpU->is(ACTN_MOV))
			{
				InsertParent(pSelf->mpU->mpR, T);
			}*/
		}
		else if (pSelf->isRight())
		{
			if (TypOf(pSelf).isPtr())
				if (TypOf(pSelf->mpU) != TypOf(pSelf))
				{
					InsertParent(pSelf, PtrOf(TYP_t(fromOpType(OPTYP_INT8))));
				}
		}
	}

	return RETURN_1(DAT6);

}

ADDR EXPRSimpl_t::GetDataOffset(CFieldPtr pField) const
{
	if (!IsLocal(pField))
		return pField->_key();
	if (!IsLocalArg(pField))
		return address(pField);
	assert(pField->owner() == (GlobToTypePtr)FuncDefPtr());
	if (SSIDx(pField))//assigned storage
		return address(pField);
	const Arg2_t *pArg(mrExprCache.findNoSSIDArg(*this, FuncDefPtr(), pField));
	assert(pArg);
	return pArg->offs();
}

/*void EXPRSimpl_t::Adjust(const expr::GType *&pType)
{
	if (!pType->typeSimple())
		return;
	if (pType->typePtr())
		return;
	uint8_t t(pType->typeSimple()->optype());
	if (!OPSIZE(t))
		t |= OPSZ_BYTE;
	if (!OPTYP(t))
		t = MAKETYP_SINT(t);
	pType = mTypesMgr.fromOpType((OpType_t)t);
}*/

/*bool EXPRSimpl_t::ReconcileTypes1(const expr::GType *&rpType1, const expr::GType *&rpType2, bool b)
{
	if (rpType1 == rpType2)
		return false;

	if (!rpType1 && rpType2)
	{
		rpType1 = rpType2;
		return true;//right can be null too
	}
	if (!rpType2 && rpType1)
	{
		rpType2 = rpType1;
		return true;//left isn't null
	}

	bool bComplex1(rpType1->typeComplex() != 0);
	bool bComplex2(rpType2->typeComplex() != 0);

	if (bComplex1 ^ bComplex2)
		return false;//one simple, other - complex

	if (bComplex1)
		//return (rpType1->typeComplex() == rpType2->typeComplex());
		return false;

	//?	if (mpStruc)
	//?	if (!T.mpStruc)
	//?	return false;//left is struc, right is not

	if (rpType1->typePtr() && rpType2->typePtr())
	{
		const expr::GType *p1(rpType1->typePtr()->baseType());
		assert(p1 != rpType1);
		const expr::GType *p2(rpType2->typePtr()->baseType());
		assert(p2 != rpType2);
		bool b(ReconcileTypes1(p1, p2));
		if (p1 != rpType1->typePtr()->baseType())
			rpType1 = mTypesMgr.PtrOf(p1);
		if (p2 != rpType2->typePtr()->baseType())
			rpType2 = mTypesMgr.PtrOf(p2);
		return b;
	}

	if (!(rpType1->typeSimple() && rpType2->typeSimple()))
		return false;

	uint8_t t1 = rpType1->typeSimple()->optype();
	uint8_t t2 = rpType2->typeSimple()->optype();
	//if (t1 == t2)
		//return false;

	if (!AgreeTypes(t1, t2))
		return false;

	CHECK(b)
		STOP

	if (rpType1->typePtr())
		rpType2 = rpType1;
	else if (rpType2->typePtr())
		rpType1 = rpType2;
	else
	{
		rpType1 = mTypesMgr.fromOpType((OpType_t)t1);
		rpType2 = mTypesMgr.fromOpType((OpType_t)t2);
	}
	return true;
}*/

/*bool EXPRSimpl_t::IsNull(const expr::GType *pType) const
{
	if (!pType)
		return true;
	return pType->typeSimple() && (pType->typeSimple()->optype() == OPTYP_NULL);
}*/

int EXPRSimpl_t::SimplifyIndir4(Out_t* pSelf) const
{
	if (!pSelf->is(ACTN_INDIR))
		return 0;

	// *&OBJ => OBJ
	if (pSelf->mpR->is(ACTN_OFFS))
	{
		Out_t* pOut(pSelf->mpR->mpR);
		DetachParent(pOut);//ACTN_OFFS
		DetachParent(pOut);//ACTN_INDIR
		return RETURN_1(PTR7);
	}

	TYP_t T(TypOf(pSelf));
//	if (T.isNull())
	//	return 0;

	TYP_t TR(TypOf(pSelf->mpR));
	if (!TR.isPtr())
	{
		InsertParent(pSelf->mpR, PtrOf(TYP_t()));
		return RETURN_1(IND7);
	}

	TYP_t TR_d(DerefOf(TR));

	if (!AreCompliant(T, TR_d))
	{
		InsertParent(pSelf->mpR, PtrOf(T));
		return RETURN_1(IND1);
	}

	return 0;
}

int EXPRSimpl_t::SimplifyInrowCheck(Out_t* pSelf) const
{
	if (pSelf->isActionSepar())
	{
		Out_t* pMov1(pSelf->mpR);
		Out_t* pSepar3(nullptr);
		if (pMov1->isActionSepar())
		{
			pSepar3 = pMov1->mpR;
			pMov1 = pMov1->mpL;
		}
		//check right part
		if (pMov1->is(ACTN_MOV))
		{
			Out_t* pOut1(pMov1->mpR);
			if (pOut1->isFieldKind())
			{
				//check left part
				Out_t* pCheck(pSelf->mpL);
				if (pCheck->is(ACTN_CHECK))
				{
					Out_t* pMov2(pCheck->mpR);
					if (pMov2->is(ACTN_MOV))
					{
						Out_t* pOut2(pMov2->mpL);
						if (pOut2->isFieldKind())
						{
							if (pOut1->isEqualToTerm(pOut2))
							{
								Out_t* pOut3(pMov1->mpL);
								Detach(pOut3);
								if (pSepar3)
									Dispose(DetachParent(pSepar3));
								else
									Dispose(DetachParent(pCheck));
								Detach(pOut2);//replace
								Dispose(pOut2);
								pMov2->AddChild(pOut3);
								return RETURN_1(ROW1);
							}
						}
					}
				}
			}
		}
	}

	// CSW(1) ~~ expr, CMPRSN0(CSW(2)) => CSW(2) := CSW(1) ~~ expr
	if (pSelf->isComparAction0())
	{
		Out_t* pSelf2(pSelf);
		if (pSelf2->mpU->is(ACTN_LOGNOT))
			pSelf2 = pSelf2->mpU;
		if (pSelf2->mpU->isActionSepar())
		{
			if (pSelf2->isRight())
			{
				Out_t* pSibling(pSelf2->sibling());
				if (pSibling->is(ACTN_CHECK))
				{
					assert(pSibling->mpL->isOpOf(SSID_CPUSW));
					assert(pSelf->mpR->isOpOf(SSID_CPUSW));
					Detach(pSibling);
					Dispose(DetachParent(pSelf2));
					Out_t* pOutG(InsertParent(pSelf->mpR, ACTN_GET));
					pOutG->AddChild(pSibling);
					return RETURN_1(PRE2);
				}

				//if (pSibling->checkAction(ACTN_MOV) && pSibling->mpL->checkAction(ACTN_ASSCHK))
				{
					STOP
				}
			}
		}
	}
	return 0;
}

int EXPRSimpl_t::SimplifyVar(Out_t *pSelf) const
{
	if (!pSelf->is(ACTN_VAR))
		return 0;

	Out_t* mpR(pSelf->mpR);
	if (mpR->isFieldKind())
		if (pSelf->isLeft() && pSelf->mpU->isActionSepar())
		{
			Out_t* pMov(pSelf->sibling());
			Out_t* pSepar(nullptr);
			if (pMov->isActionSepar())
			{
				pSepar = pMov;
				pMov = pSepar->mpL;
			}
			if (pMov->is(ACTN_MOV) || pMov->isMovExAction())
			{
				if (pMov->mpL->isFieldKind() && pMov->mpL->isEqualToTerm(mpR))
				{
					if (!pSepar)
					{
						// var a, a=expr => var a=expr
						Detach(pMov);
						Dispose(DetachParent(pSelf));
						Detach(mpR);
						Dispose(mpR);
						pSelf->AddChild(pMov);
						return RETURN_1(VAR3);
					}

					// var a; a = b; c => var a = b; c
					Dispose(DetachParent(pMov->sibling()));
					Detach(mpR);
					Dispose(mpR);
					pSelf->AddChild(pMov);
					return RETURN_1(VAR2);
				}
			}
		}

	return 0;
}

int EXPRSimpl_t::SimplifyHalf2(Out_t *pSelf) const
{//return 0;
	if (!pSelf->is(ACTN_LOHALF))
		if (!pSelf->is(ACTN_HIHALF))
			return 0;

	int sz = TypOf(pSelf).size();
	if (!sz)
		return 0;
	
	if (!pSelf->mpR->isTerm())
	if (!pSelf->mpR->is(ACTN_INDIR))
		return 0;
#if(0)

	Out_t *pAddr = mpR->InsertParent(ACTN_OFFS);

	if (is(ACTN_HIHALF))
	{
		Out_t *pAdd = mpR->InsertParent(ACTN_ADD);
		pAdd->Add(sz);
	}

//	Out_t *pType = mpR->InsertParent(pSelf->mpR, PtrOf(pSelf));

	mAction = ACTN_INDIR;
#else

	if (pSelf->is(ACTN_HIHALF))
	{
		if (pSelf->mpR->is(ACTN_INDIR))
		{
			Out_t *pAdd = InsertParent(pSelf->mpR->mpR, ACTN_ADD);
			Add(*pAdd, HOP(), sz);
		}
		else
			pSelf->mpR->mOffs += sz;
	}

	uint8_t t0(pSelf->mpR->typ().optyp());
	uint8_t t1(t0 & 0xF0);
	uint8_t t2(sz);
	pSelf->mpR->setTyp(fromOpType(t1 | t2));
	DetachParent(pSelf->mpR);
#endif

	assert(pSelf->size());
	return RETURN_1(HAL1);
}

int EXPRSimpl_t::SimplifySwitchEx(Out_t * pSelf) const
{
	if (!pSelf->is(ACTN_SWITCH))
		return 0;

	Out_t *pTop = pSelf;
	pTop = pTop->mpL;
	//DetachParent(pTop);//ACTN_GOTO
	if (pTop->is(ACTN_ARRAY))
	{
		if (!pTop->mpL->isFieldKind())
			return 0;
//		OpPtr pJumpTableOp = pTop->mpL->m_pOp;
		FieldPtr pJumpTable;
//		if (pJumpTableOp->IsDataOp())
//			pJumpTable = pJumpTableOp->GetOwnerData();
///		else
//			pJumpTable = pJumpTableOp->m_pData;
		pJumpTable = pTop->mpL->field();
		assert(pJumpTable);
		if (1)//IsThruConst(pJumpTable)
		{
			pTop = pTop->mpR;
			DetachParent(pTop);//remove jump table
			int nIndexOffset1;
			ExtractDisplacement2(pTop, nIndexOffset1, true);

			while (pTop->is(ACTN_INDIR)
				&& pTop->mpR->is(ACTN_TYPE)
				&& pTop->mpR->mpR->is(ACTN_OFFS))
				pTop = pTop->mpR->mpR->mpR;

			while (pTop->is(ACTN_TYPE) || pTop->is(ACTN_ZEROEXT))
				pTop = pTop->mpR;

			if (pTop->is(ACTN_ARRAY))//check for index table
			{
				assert(pTop->mpL->isFieldKind() || pTop->mpL->isNumKind());
//				OpPtr pIndexTableOp = pTop->mpL->m_pOp;
				FieldPtr pIndexTable;
//				if (pIndexTableOp->IsDataOp())
//					pIndexTable = pIndexTableOp->GetOwnerData();
//				else
//					pIndexTable = pIndexTableOp->m_pData;
				pIndexTable = pTop->mpL->field();
				assert(pIndexTable);
				if (1)//IsThruConst(pIndexTable))
				{
					pTop = pTop->mpR;
					DetachParent(pTop);//remove index table
					int nIndexOffset2;
					ExtractDisplacement2(pTop, nIndexOffset2, true);
				}
			}
		}
	}

	return 1;
}

//walk call's args in expression, sync'd with fields in funcdef and ops in call
class CallArgIterator
{
	Out_t *m_pOutCall;
	Out_t *m_pOut;
	OpList_t::Iterator m_ita;
public:
	CallArgIterator(Out_t *pOutCall)
		: m_pOutCall(pOutCall),
		m_pOut(m_pOutCall->mpR),
		m_ita(m_pOutCall->mpOp->argsIt())
	{
		if (m_pOut && m_pOut->is(ACTN_COMMA))
			m_pOut = m_pOut->mpL;
	}
	void reset()
	{
		m_pOut = m_pOutCall->mpR;
		if (m_pOut && m_pOut->is(ACTN_COMMA))
			m_pOut = m_pOut->mpL;
		m_ita.assign(m_pOutCall->mpOp->argsIt().data());
	}
	operator bool() const { return (m_pOut != nullptr); }
	CallArgIterator& operator ++()
	{
		if (m_pOut->isLeft())
		{
			if (m_pOut->mpU->is(ACTN_COMMA))
			{
				m_pOut = m_pOut->mpU->mpR;
				if (m_pOut->is(ACTN_COMMA))
					m_pOut = m_pOut->mpL;
			}
			else
				m_pOut = nullptr;
		}
		else
			m_pOut = nullptr;
		if (m_ita)
			m_ita++;
		return *this;
	}
	//CallArgIterator& operator ++(int){ return operator++(); }
	OpPtr op() const { return m_ita ? m_ita.data() : OpPtr(); }
	Out_t &out() const { return *m_pOut; }
};

//for funcdef
class CallArgIterator2 : private CallArgIterator
{
	FuncCCArgsCIt<> m_itf;
public:
	using CallArgIterator::op;
	using CallArgIterator::out;
	using CallArgIterator::operator bool;
	CallArgIterator2(Out_t* pOutCall, FuncDef_t& rf, const CallingConv_t& rcc)
		: CallArgIterator(pOutCall),
		m_itf(rf, rcc)
	{
	}
	CFieldPtr field() const {
		if (m_itf)
			return m_itf.field();
		return nullptr;
	}
	CallArgIterator2& operator ++()
	{
		CallArgIterator::operator++();
		if (m_itf)
			++m_itf;
		return *this;
	}
	void reset()
	{
		m_itf.reset();
		CallArgIterator::reset();
	}
	Arg2_t fieldToArg() const
	{
		CFieldPtr pField(field());
		if (FuncInfo_s::SSIDx(pField))
			return Arg2_t((OPC_t)FuncInfo_s::SSIDx(pField), FuncInfo_s::address(pField), pField->size());
		return Arg2_t((OPC_t)m_itf.ssid(), m_itf.offset(), m_itf.size2());
	}
};

//call expression argument iterator with func type
class CallArgIterator3 : private CallArgIterator
{
	FuncTypeArgsCIt2 m_itf;
public:
	using CallArgIterator::op;
	using CallArgIterator::out;
	using CallArgIterator::operator bool;
	CallArgIterator3(Out_t* pOutCall, CTypePtr pFuncType, const CallingConv_t& rcc)
		: CallArgIterator(pOutCall),
		m_itf(pFuncType, rcc)
	{
	}
	CallArgIterator3& operator ++()
	{
		CallArgIterator::operator++();
		if (m_itf)
			++m_itf;
		return *this;
	}
	CTypePtr argType() const
	{
		if (m_itf)
			return *m_itf;
		return nullptr;
	}
	Arg2_t fieldToArg() const
	{
		return Arg2_t((OPC_t)m_itf.ssid(), m_itf.offset(), m_itf.size());
	}
	void reset()
	{
		m_itf.reset();
		CallArgIterator::reset();
	}
};

int EXPRSimpl_t::CheckConformantCallType(Out_t* pSelf) const//can't execute this rule during dumping - a func target is not explicit?
{
	if (!pSelf->is(ACTN_CALL))
		return 0;
	Out_t* pOutType(pSelf->mpL);
	if (!pOutType->is(ACTN_TYPE))
		return 0;
	assert(TypOf(pOutType).isCallPtr());

	int iConforms(CheckConformant(pSelf));
	if (iConforms)
	{
		Out_t* mpR(pOutType->mpR);
		DetachParent(mpR);//no need
		if (mpR->is(ACTN_OFFS))
			DetachParent(mpR->mpR);
		switch (iConforms)
		{
		default: return RETURN_1(CAL0);
		case -1: return RETURN_1(CAL1);
		case -2: return RETURN_1(CAL2);
		case -3: return RETURN_1(CAL3);
		case -4: return RETURN_1(CAL4);
		}
	}
	return 0;
}

/*int EXPRSimpl_t::CheckConformantCall(Out_t *pOutCall) const//can't execute this rule during dumping - a func target is not explicit?
{
	if (IsUnfoldMode())
		return 0;
	if (!ProtoInfo_t::IsFuncStatusFinished(FuncDefPtr()))
		return 0;
	if (!pOutCall->IsCall())
		return 0;
	if (pOutCall->mpL->IsIntrinsicRef())
		return 0;

	int iConforms(CheckConformant(pOutCall));
	if (!iConforms)
	{
		InsertParent(pOutCall->mpL, fromCall(pOutCall));
		return RETURN_1(CAL0);
	}
	switch (iConforms)
	{
	case -1: return RETURN_1(CAL1);
	case -2: return RETURN_1(CAL2);
	case -3: return RETURN_1(CAL3);
	case -4: return RETURN_1(CAL4);
	}
	return 0;
}*/

int EXPRSimpl_t::CheckConformant(Out_t* pOutCall) const
{
	int iConforms(1);

	std::pair<FieldPtr, GlobPtr> par(ExtractCallTarget(pOutCall));

	if (par.second)//glob
		return CheckConformant(par.second, pOutCall);
	if (par.first)//field
		return CheckConformant(par.first, pOutCall);
	return 0;
}

int EXPRSimpl_t::CheckConformant(CFieldPtr pFuncPtr, Out_t *pOutCall) const
{
	int iConforms(1);
	CallingConv_t CC(*this, FuncDefPtr());
	CTypePtr pFuncType(IsPtrToFuncType(pFuncPtr));
	if (pFuncType)
	{
		//check if a call is conformant with a funcdef by ARGS
		CallArgIterator3 i(pOutCall, pFuncType, CC);
		for (; i; ++i)
		{
			CTypePtr pArgType(i.argType());
			if (!pArgType)//number of args in funcdef is less
			{
				iConforms = 0;
				break;
			}

			OpPtr pOp(i.op());
			assert(pOp);//out list is build from op list!
			//if (!rCallee.IsCallOpConformantWithArg(pOp, mrExprCache.fieldToArg(rCallee, pField)))
			if (!IsCallOpConformantWithArg(pOp, i.fieldToArg()))
			{
				iConforms = 0;
				break;
			}
		}

		//check if a call is conformant with functype by RETVAL
		if (!pOutCall->mpU->isRoot())
		//if (!pOutCall->mpU->is(ACTN_MOV) && pOutCall->isRight())
		if (iConforms)
		{
			TypePtr pRetType(pFuncType->typeFunc()->retVal());
			if (pRetType)
			{
				TYP_t T_ret(fromTypeObj(pRetType));
				if (!T_ret.canPromoteTo(pOutCall->mpU->typ()))
					iConforms = 0;
			}
			else
				iConforms = 0;
		}

		//now type-cast each expr argument into one of funcdef
		if (iConforms)
		{
			if (!i.argType())
			{
				i.reset();
				Out_t* pOutThis(nullptr);
				while (i)
				{
					Out_t* pOut(&i.out());
					CTypePtr pArgType(i.argType());
					assert(pArgType);
					++i;//now can safely insert type casts!

					TYP_t T(fromTypeObj(pArgType));
					if (T != TypOf(pOut))
					{
						pOut = InsertParent(pOut, T);//now it is a top
						iConforms = -3;
					}
					//?						if (ThisPtrArg(ifDef) == pArgType)
						//?						pOutThis = pOut;
				}
				// and handle 'this' ptr
				if (pOutThis)
				{
					if (pOutThis->mpU->is(ACTN_COMMA))
						DetachParent(pOutThis->sibling());
					else
						Detach(pOutThis);

					Out_t* pOutPtr(InsertParent(pOutCall->mpL, ACTN_PTR));
					Add(*pOutPtr, *pOutThis);
					pOutPtr->flipChilds();
					iConforms = -4;
				}
			}
			else
				iConforms = 0;//number of args in funcdef is greater
		}
	}
	else
		iConforms = 0;

	return iConforms;
}

int EXPRSimpl_t::CheckConformant(CGlobPtr ifDef, Out_t *pOutCall) const
{
	int iConforms(1);

	CallingConv_t CC(*this, FuncDefPtr());
	//FuncInfo_t rCallee(DcRef(), *ifDef);

	//check if a call is conformant with a funcdef
	CallArgIterator2 i(pOutCall, *ifDef->typeFuncDef(), CC);
	for (; i; ++i)
	{
		CFieldPtr pField(i.field());
		if (!pField)//number of args in funcdef is less
		{
			iConforms = 0;
			break;
		}

		HOP hOp(i.op());
		assert(hOp);//out list is build from op list!
		//if (!rCallee.IsCallOpConformantWithArg(pOp, mrExprCache.fieldToArg(rCallee, pField)))
		if (!IsCallOpConformantWithArg(hOp, i.fieldToArg()))
		{
			iConforms = 0;
			break;
		}
	}

	//now type-cast each expr argument into one of funcdef
	if (iConforms)
	{
		if (!i.field())
		{
			i.reset();
			Out_t* peThis(nullptr);
			while (i)
			{
				Out_t* pOut(&i.out());
				CFieldPtr pField(i.field());
				assert(pField);
				++i;//now can safely insert type casts!

				TYP_t T0(TypOf(pOut));
				TYP_t T(fromField(pField));
#if(1)
				//args constitutes the sub-expressions!
				pOut = InsertParent(pOut, ACTN_MOV);//now it is a top
				pOut->flipChilds();
				Out_t* peArg(Add(*pOut));
				if (ProtoInfo_t::ThisPtrArg(ifDef) == pField)
					peArg->mKind = OUT_ARG_THIS;
				else
					peArg->mKind = OUT_ARG;//these are invisible!
				peArg->setTyp(T);
				iConforms = -1;
				if (T != T0)
					InsertParent(peArg->sibling(), TYP_t());//type cast
#else
				if (!IsImplicitCastOf(TypOf(pOut), T))//.stripTypedef(), T.stripTypedef()))//if (T != TypOf(pOut))
				{
					pOut = InsertParent(pOut, T);//now it is a top
					iConforms = -1;
				}
#endif
				if (ProtoInfo_t::ThisPtrArg(ifDef) == pField)
					peThis = pOut;
			}
		}
		else
			iConforms = 0;//number of args in funcdef is greater
	}
	return iConforms;
}

int EXPRSimpl_t::CheckThisCall(Out_t* pSelf) const
{
	if (!pSelf->is(ACTN_CALL))
		return 0;
	if (!pSelf->mpR)
		return 0;
	Out_t* peThis(pSelf->mpR);
	if (peThis->is(ACTN_COMMA))
		peThis = peThis->mpL;
	if (!peThis->is(ACTN_MOV))
		return 0;
	if (peThis->mpL->mKind != OUT_ARG_THIS)
		return 0;
	// and handle 'this' ptr
	if (peThis->mpU->is(ACTN_COMMA))
		DetachParent(peThis->sibling());
	else
		Detach(peThis);
	Out_t* pePtr(InsertParent(pSelf->mpL, ACTN_PTR));
	Add(*pePtr, *peThis);
	pePtr->flipChilds();
	return RETURN_1(THS1);
}

int EXPRSimpl_t::CheckConformantRet(Out_t *pOutRet) const
{
	if (!pOutRet->is(ACTN_RET))
		return 0;
	if (!pOutRet->mpR)
		return 0;//?
	if (pOutRet->mpR->is(ACTN_TYPE))
	{
		TYP_t T(TypOf(pOutRet->mpR->mpR));
		//if (AreCompliant(TypOf(pOutRet->mpR), T))
		if (TypOf(pOutRet->mpR) == T)
		{
			Detach(pOutRet->mpR);
			return RETURN_1(RET2);
		}
	}
	return 0;
}

//goto => return
int EXPRSimpl_t::CheckRet(Out_t* pSelf) const
{
	if (IsUnfoldMode())
		return 0;
	if (pSelf->is(ACTN_GOTO))
	{
		if (pSelf->mpL->isOpKind())
		{
			HOP hOp(pSelf->mpL->mpOp);//target op
			if (hOp && IsRetOp(hOp) && !IsBadRetOp(hOp))
			{
				Detach(pSelf->mpL);
				pSelf->mAction = ACTN_RET;
				return RETURN_1(RET1);
			}
		}
	}
	return 0;
}

int EXPRSimpl_t::CheckImpCall(Out_t* pSelf) const
{
	if (pSelf->is(ACTN_INDIR))
	{
		if (pSelf->mpR->isFieldKind())
		{
			if (pSelf->mpR->field()->isTypeImp() && pSelf->mpR->expField())
			{
				DetachParent(pSelf->mpR);
				return RETURN_1(CAL8);
			}
		}
	}
	return 0;
}




///////////////////////////////////////////////////////////////////////

int EXPRSimpl_t::DispatchSimplifyAssign(Out_t * pSelf, bool b) const
{
	CHECK_NODE(SimplifyGet(pSelf, b));

	CHECK_EXPR(pSelf->mpL, DispatchSimplifyAssign(pSelf->mpL, b));
	CHECK_EXPR(pSelf->mpR, DispatchSimplifyAssign(pSelf->mpR, b));

	return 0;
}

int EXPRSimpl_t::DispatchSimplifyPrimary(Out_t * pSelf) const
{
	assert(pSelf);
	CHECK_NODE(SimplifyGet(pSelf, 1));
	CHECK_NODE(SimplifyAssign(pSelf));
	//CHECK_NODE(SimplifyAssignPtr(pSelf));
	CHECK_NODE(SimplifyFract(pSelf));
	CHECK_NODE(SimplifySign(pSelf));
	CHECK_NODE(SimplifyAdd(pSelf));
	CHECK_NODE(SimplifySub(pSelf));
	CHECK_NODE(SimplifyMultiply(pSelf));
	CHECK_NODE(SimplifyDivide(pSelf));
	CHECK_NODE(SimplifyShiftLeft(pSelf));
	CHECK_NODE(SimplifyShiftRight(pSelf));
	CHECK_NODE(SimplifyBitwiseAnd(pSelf));
	CHECK_NODE(SimplifyHalf1(pSelf));
	CHECK_NODE(SimplifyAndOr(pSelf));
	CHECK_NODE(SimplifyCheck(pSelf));
	CHECK_NODE(SimplifyHalf2(pSelf));
	CHECK_NODE(SimplifyCommonMult(pSelf));
	CHECK_NODE(SimplifyType(pSelf, false));
	//CHECK_NODE(SimplifyOp4Ptr(pSelf));
	CHECK_NODE(SimplifyAssCheck(pSelf));
	
	CHECK_EXPR(pSelf->mpL, DispatchSimplifyPrimary(pSelf->mpL));
	CHECK_EXPR(pSelf->mpR, DispatchSimplifyPrimary(pSelf->mpR));

	CHECK_NODE(AttachGlob2(pSelf));
	CHECK_NODE(SimplifyThruConst(pSelf));
	CHECK_NODE(SimplifyArrays(pSelf));

	return 0;
}

int EXPRSimpl_t::DispatchSimplifyPrimary2(Out_t *pSelf) const
{
	assert(pSelf);
	
	//CHECK_NODE(AttachGlob2(pSelf));
	//CHECK_NODE(SimplifyArrays(pSelf));
	CHECK_NODE(SimplifyIndir(pSelf));//ptrs
	CHECK_NODE(SimplifyIndir3(pSelf));//ptrs
	CHECK_NODE(SimplifyIndir4(pSelf));
	CHECK_NODE(SimplifyType4(pSelf));

	CHECK_EXPR(pSelf->mpL, DispatchSimplifyPrimary2(pSelf->mpL));
	CHECK_EXPR(pSelf->mpR, DispatchSimplifyPrimary2(pSelf->mpR));

	CHECK_NODE(SimplifyAssign2(pSelf));
	
	return 0;
}


int EXPRSimpl_t::DispatchSimplifySecondary(Out_t* pSelf) const
{
	assert(pSelf);

//	CHECK_NODE(SimplifyGet(pSelf, 1));
//	CHECK_NODE(SimplifyAssign(pSelf));
	CHECK_NODE(SimplifyInrowCheck(pSelf));
	CHECK_NODE(SimplifyVar(pSelf));
	//	CHECK_NODE(SimplifyRawCondition(pSelf));
	CHECK_NODE(SimplifyExt(pSelf));//zeroextend,signextend
	CHECK_NODE(SimplifyIndir2(pSelf));
	CHECK_NODE(SimplifyOffs(pSelf));
	CHECK_NODE(SimplifyPtr2(pSelf));
	CHECK_NODE(SimplifyCompMov(pSelf));//compound assignments, ++, --
	CHECK_NODE(SimplifyType(pSelf, true));//types?
	CHECK_NODE(SimplifyThis(pSelf));//this
	//CHECK_NODE(SimplifyArrCall(pSelf));
	//CHECK_NODE(SimplifyThis0(pSelf));
	CHECK_NODE(SimplifyMulDiv(pSelf));//mul&div => shl&shr
	CHECK_NODE(SimplifyEqu(pSelf));
//	CHECK_NODE(SimplifySign2(pSelf));//signs
	CHECK_NODE(SimplifyCheck(pSelf));
	CHECK_NODE(SimplifyPostIncrement(pSelf));
	//CHECK_NODE(SimplifyRawCondition(pSelf));
	//CHECK_NODE(SimplifyCompar(pSelf));//comparisons

	CHECK_EXPR(pSelf->mpL, DispatchSimplifySecondary(pSelf->mpL));
	CHECK_EXPR(pSelf->mpR, DispatchSimplifySecondary(pSelf->mpR));

	//CHECK_NODE(ReplaceReal80Types(pSelf));

	return 0;
}

int	EXPRSimpl_t::DispatchSimplifyPost(Out_t * pSelf) const
{
	assert(pSelf);

	CHECK_NODE(SimplifyArrCall(pSelf));
	CHECK_NODE(SimplifySign2(pSelf));//signs
	CHECK_NODE(SimplifyRawCondition(pSelf));
	CHECK_NODE(SimplifyCompar(pSelf));//comparisons
	CHECK_NODE(SimplifyLogic(pSelf));
	CHECK_NODE(SimplifyLogic2(pSelf));
	CHECK_NODE(SimplifyLogicPost(pSelf));
	CHECK_NODE(CheckImpCall(pSelf));
	
	CHECK_EXPR(pSelf->mpL, DispatchSimplifyPost(pSelf->mpL));
	CHECK_EXPR(pSelf->mpR, DispatchSimplifyPost(pSelf->mpR));

	return 0;
}

/*int	EXPRSimpl_t::DispatchSimplifyCall(Out_t* pSelf) const
{
	assert(pSelf);
	
	CHECK_NODE(CheckConformantCallType(pSelf));

	CHECK_EXPR(pSelf->mpL, DispatchSimplifyCall(pSelf->mpL));
	CHECK_EXPR(pSelf->mpR, DispatchSimplifyCall(pSelf->mpR));

	CHECK_NODE(CheckConformantRet(pSelf));

	return 0;
}*/

////////////////////////////////////////////////////

int EXPRSimpl_t::PreSimplify(Out_t *pSelf, unsigned i, unsigned imax) const
{
	for (; i < imax; i++)
	{
		__dump(i, pSelf);
		if (DispatchSimplifyAssign(pSelf, false))
			continue;
		break;
	}
	return i;
}

int EXPRSimpl_t::PrimarySimplify(Out_t *pSelf, unsigned i, unsigned imax) const
{
	unsigned i0(i);
	for (; i < imax; i++)
	{
		if (i > i0)
			__dump(i, pSelf);

		if (DispatchSimplifyPrimary(pSelf))
			continue;
		if (DispatchSimplifyPrimary2(pSelf))
			continue;

		break;
	}
	return i;
}

int EXPRSimpl_t::SecondarySimplify(Out_t *pSelf, unsigned i, unsigned imax) const
{
	unsigned i0(i);
	for (; i < imax; i++)
	{
		if (i > i0)
			__dump(i, pSelf);
		if (DispatchSimplifySecondary(pSelf))
			continue;
		break;
	}
	return i;
}

int EXPRSimpl_t::PostSimplify(Out_t *pSelf, unsigned i, unsigned imax) const
{
	unsigned i0(i);
	while (i < imax)
	{
		if (i > i0)
			__dump(i++, pSelf);
		if (DispatchSimplifyPost(pSelf))
			continue;
		if (CheckRet(pSelf))
			continue;
		break;
	}

	//check calls compliance
	/*for (;; m++)
	{
		if (m > l)
			__dump(m, pSelf);
		if (DispatchSimplify_Call(pSelf))
			continue;
		break;
	}*/
	
	for (Out_t::Iterator it(pSelf); it; ++it)//want check each call just once!
	{
		Out_t *pe(it.top());

		if (CheckConformantCallType(pe))
			__dump(++i, pSelf);

		if (CheckThisCall(pe))
			__dump(++i, pSelf);

		if (CheckConformantRet(pe))
			__dump(++i, pSelf);
	}

	return i;
}

void EXPRSimpl_t::SetTyp(Out_t* p, const TYP_t& t) const
{
	if (p->isTerm())
	{
		p->setTyp(t);
	}
	else
	{
		assert(p->mAction == ACTN_TYPE);
		p->setTyp(t);
	}
}


void EXPRSimpl_t::SetupTypes(Out_t *p) const
{
	if (!p)
		return;
	if (p->mpL)
		SetupTypes(p->mpL);
	if (p->mpR)
		SetupTypes(p->mpR);
	p->setTyp(TypOf(p));//post-order
}

int	EXPRSimpl_t::SimplifyArgs(Out_t* pSelf) const
{
	if (pSelf->isArgKind())
	{
		assert(pSelf->mpU->is(ACTN_MOV) && pSelf->isLeft());
		Out_t* pUR(pSelf->sibling());
		if (pUR->is(ACTN_CALL))
		{
			if (pUR->mpL->typ().isCallPtr())
				return 0;//preserve retval of call cast
		}
		DetachParent(pSelf->sibling());
		return 1;
	}
	return 0;
}

int EXPRSimpl_t::SimplifyFinal(Out_t* pSelf) const
{
	CHECK_NODE(SimplifyArgs(pSelf));
	CHECK_NODE(SimplifyThis(pSelf));
	CHECK_EXPR(pSelf->mpL, SimplifyFinal(pSelf->mpL));
	CHECK_EXPR(pSelf->mpR, SimplifyFinal(pSelf->mpR));
	return 0;
}

void EXPRSimpl_t::Simplify(Out_t* pSelf) const
{
#if(SIMPL_NEW)
	SimplifyNew(pSelf);
	return;
#endif

	int i(PreSimplify(pSelf, 0));
	i = PrimarySimplify(pSelf, i);
	i = PostSimplify(pSelf, i);
	i = PrimarySimplify(pSelf, i);
	i = SecondarySimplify(pSelf, i);
	while (SimplifyFinal(pSelf));
	SetupTypes(pSelf);
}


///////////////////////////////////////////////////////////////////////////////////


int EXPRSimpl_t::DispatchSimplifyNew(Out_t* pSelf) const
{
	CHECK_NODE(SimplifyGet(pSelf, false));

	CHECK_EXPR(pSelf->mpL, DispatchSimplifyNew(pSelf->mpL));
	CHECK_EXPR(pSelf->mpR, DispatchSimplifyNew(pSelf->mpR));

	return 0;
}

void EXPRSimpl_t::SimplifyNew(Out_t* pSelf) const
{
	for (int i(0);; i++)
	{
		__dump(i, pSelf);
		if (DispatchSimplifyNew(pSelf))
			continue;
		break;
	}
}


