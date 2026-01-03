
#include "expr_ptr.h"
#include <assert.h>
#include "shared/defs.h"
#include "shared/action.h"
#include "db/mem.h"
#include "db/field.h"
#include "db/type_struc.h"
#include "db/proj.h"
#include "info_dc.h"
#include "path.h"
#include "ana_expr.h"
#include "ana_local.h"
#include "ana_data.h"
#include "dump.h"
#include "clean_ex.h"
#include "expr_term.h"

EXPRptr_t::EXPRptr_t(const FuncInfo_t &r, HOP pOp, uint32_t flags, ExprCacheEx_t &rExpr)
	: EXPR_t(r, pOp, flags, rExpr),
	//mrPathTracer(r.pathTracer()),
	//mrOpTracer(r.opTracer()),
	mbTraceXIns(false)//,
	//mpCachedArgs(nullptr)
{
}

/*int EXPRptr_t::MergeTypes(Out_t * pSelf, Out_t * pOut)
{
	if (!pSelf->is(ACTN_PTR))
		return 0;
	if (!pOut->is(ACTN_PTR))
		return 0;
	
	Out_t * pOutFld1 = pSelf->mpR;
	Out_t * pOutFld2 = pOut->mpR;
	
	FieldPtr  pField1 = pOutFld1->mpField;
	FieldPtr  pField2 = pOutFld2->mpField;

	TypePtr pStruc1 = pField1->type();
	TypePtr pStruc2 = pField2->type();
		
	TypePtr pStruc = pStruc1;
	if (!pStruc)
	{
		pStruc = pStruc2;
	}
	else if (pStruc2)
	{
		if (pStruc != pStruc2)
		{
			pStruc->typeStruc()->MergeWith(pStruc2);
			return 0;
		}
	}
	
	if (!pStruc)
		return 0;
	
	if ( (pField1->SetStruc0(pStruc) == 1
		|| pField2->SetStruc0(pStruc) == 1) )
		return 1;
	
	return 0;
}

int EXPRptr_t::SimplifyPtr6(Out_t * pSelf)
{
	if (!pSelf->mpU || !pSelf->mpU->is(ACTN_COMMA2))
		return 0;
	if (!pSelf->is(ACTN_PTR))
		return 0;
	if (!pSelf->isLeft())
		return 0;
	
	Out_t * pOutComma = pSelf->mpU;
	while (pOutComma->is(ACTN_COMMA2))
	{
		assert(!pOutComma->mpR->is(ACTN_COMMA2));
		Out_t * pOut = pOutComma->mpR;
		if (pOut->is(ACTN_PTR))
		{
			if (MergeTypes(pSelf, pOut) == 1)
				return 1;
		}
		
		pOutComma = pOutComma->mpU;
	}
	
	return 0;
}

int EXPRptr_t::__SimplifyPtr6(Out_t * pSelf)
{
	if ( !pSelf )
		return 0;

	if (__SimplifyPtr6(pSelf->mpL))
		return 1;
		
	if (__SimplifyPtr6(pSelf->mpR))
		return 1;
		
	if (SimplifyPtr6(pSelf))
		return 1;

	return 0;
}*/


#define BZZ 1

#define RETURN(arg) {assert(tr.trace_ptr > 0); tr.trace_ptr--; return arg;}
Out_t * EXPRptr_t::__outPtr(HOP pSelf, Out_t *mpU, PathOpTracer_t &tr0)
{
	op_tracer_cell_t &tr(tr0.opTracer().cell(pSelf));
	assert(tr.trace_ptr < TRACE_PTREX);
	tr.trace_ptr++;

	if (!IsCodeOp(pSelf) || IsAddr(pSelf) || pSelf->IsScalar() || IsGlobalOp(pSelf) || pSelf->isCPUSW())
	{
		mpU = Add(mpU, pSelf);
		RETURN(mpU);
	}

	if (pSelf->IsIndirectB())
	{
		Out_t *pOutI = Add(*mpU, pSelf, ACTN_INDIR);
		TYP_t T(fromOp(pSelf));
		//pOutI->setTyp(T);

		mpU = AddB(pOutI, pSelf);

		Out_t *pOutJ(InsertParent(mpU, ACTN_TYPE));
		pOutJ->setTyp(PtrOf(T));

		if (pSelf->m_xin.empty() && mbTraceXIns)
		{
			AnlzXDepsIn_t an(*this, tr0, pSelf, static_cast<ExprCacheEx_t &>(mrExprCache));
			an.TraceXDepsIn();
		}

		for (XOpList_t::Iterator i(pSelf->m_xin); i; i++)
		{
			HOP pOp = i.data();
			if (tr.trace_ptr > 1)
				if (tr0.opTracer().cell(pOp).trace_ptr > 0)
					continue;
			if (mpU->mpL && mpU->mpR)
				mpU = InsertParent(mpU->mpR, ACTN_COMMA2);
			else
				mpU = InsertParent(mpU, ACTN_GET);
			__outPtr(pOp, mpU, tr0);
		}

		if (pSelf->m_disp)
		{
			Out_t *pOutA = InsertParent(pOutJ->mpR, ACTN_ADD);
			Add(*pOutA, pSelf, pSelf->m_disp);
		}

		RETURN(pOutI);
	}

	Out_t *pOutT = Add(mpU, pSelf);
	mpU = pOutT;

#ifdef BZZ
	if (IsPrimeOp(pSelf))
	if (!IsCall(pSelf) || (pSelf != mpStartOp))//not a call?
#else
	if (IsPrimeOp(pSelf))//not a call?
#endif
	{
CHECK(OpNo(pSelf) == 1092)
STOP

		switch (ActionOf(pSelf))
		{
		case ACTN_MOV:
			mpU = InsertParent(pOutT, ACTN_MOV);
			break;
		case ACTN_ADD:
		case ACTN_SUB:
			mpU = InsertParent(pOutT, ACTN_MOV);
			mpU = Add(*mpU, pSelf, ActionOf(pSelf));
			mpU->m_bNoReduce = (pSelf->m_nFlags & OPND_NO_REDUCE) != 0;
			break;
#if(1)
		case ACTN_MUL:
			if (mpStartOp->IsIndirectB())//must be sure if we're tracing a ptr!
			{
				mpU = InsertParent(pOutT, ACTN_MOV);
				mpU = Add(*mpU, pSelf, ActionOf(pSelf));
				break;
			}
		default:
#if(0)
			mpU = InsertParent(pOutT, ACTN_MOV);
			mpU = Add(*mpU, pSelf->mAction());
			//assert(false);
#endif
			RETURN(pOutT);
#endif
		}

		for (OpList_t::Iterator i(pSelf->argsIt()); i; i++)
		{
			__outPtr(i.data(), mpU, tr0);
		}

		RETURN(pOutT);
	}

	const XOpList_t &l(pSelf->m_xin);
	if (l.empty() && mbTraceXIns)
	{
		if (!AnlzXDepsIn_t::IsInside())
		{
			AnlzXDepsIn_t an(*this, tr0, pSelf, static_cast<ExprCacheEx_t &>(mrExprCache));
			an.TraceXDepsIn();
		}
	}
	
	for (XOpList_t::Iterator i(l); i; i++)
	{
		HOP rOp(i.data());
		if (tr.trace_ptr > 1)
			if (tr0.opTracer().cell(rOp).trace_ptr > 0)
				continue;
		if (mpU->mpL && mpU->mpR)
			mpU = InsertParent(mpU->mpR, ACTN_COMMA2);
		else
			mpU = InsertParent(mpU, ACTN_GET);
		__outPtr(rOp, mpU, tr0);
	}

	RETURN(pOutT);
}
#undef RETURN


Struc_t *Field_t::typePtrToStruc()
{
	if (mpType)
		if (mpType->typePtr())
			if (mpType->typePtr()->pointee())
				return mpType->typePtr()->pointee()->typeStruc();
	return 0;
}

int EXPRPtrSimpl_t::SimplifyOp2Field(Out_t * pSelf, PathOpTracer_t &tr0) const
{
	if (!pSelf->isOpKind())
		return 0;

	HOP pOp(pSelf->mpOp);
	if (!pOp)
		return 0;

	FieldPtr pField0(FindFieldRef(pOp));
	if (!pField0)
	{
		if (!PrimeOp(pOp)->isRoot())
		{
			if (!IsArgOp(pOp))
			{//?
				return 0;
				//AnlzRoots_t an1(mrFunc, PrimeOp(pOp), true);
				//an1.TurnRoot_On();
			}
		}
		LocalsTracer_t an2(*this, tr0);
//		an2.setCahchedArgs(mpCachedArgs);
		an2.CheckLocal0(pOp);
		pField0 = LocalRef(pOp);
		if (!pField0)
			return 0;
	}

	assert(pField0);

	//convert op into field

	pSelf->mKind = OUT_FIELD;// OUT_ MLOC;
	pSelf->setField(pField0);
	pSelf->mSsid = OPC_NULL;
	pSelf->mOffs = 0;
	//pSelf->typ().Clear();
	
#if(1)
	if (pOp->SSID() != SSID_GLOBAL)
		pSelf->setTyp(fromField(pField0));
	else
#endif
	{
		Out_t *pOutIndir(InsertParent(pSelf, ACTN_INDIR));
		TYP_t T(TypOf(pSelf));
		//pOutIndir->setTyp(T);
		Out_t *pOutTypePtr(InsertParent(pSelf, ACTN_TYPE));
		pOutTypePtr->setTyp(PtrOf(T));
		Out_t *pOutOffs(InsertParent(pSelf, ACTN_OFFS));

		if (pField0->isTypeImp())
		{
			FieldPtr pExpField(ToExportedField(pField0));
			if (pExpField)
			{
				CTypePtr iType(pExpField->type());
				if (pExpField->isTypeProc())
					iType = (GlobToTypePtr)GlobFuncObj(pExpField);
				TYP_t Tt(iType, this);
				pSelf->setTyp(PtrOf(Tt));
				pSelf->setField(pField0);//imported
				pSelf->mpExpField = pExpField;//exported
			}
			else
				pSelf->setTyp(fromTypeObj(pField0->type()));
		}
		else
			pSelf->setTyp(fromField(pField0));

		//pOutOffs->setTyp(PtrOf(pSelf));
	}

	return RETURN_1(DAT9);
}

void EXPRPtrSimpl_t::SetFieldTyp(Out_t* p, const TYP_t& T) const
{
	assert(p->isFieldKind());
	TypePtr pType(toTypeRef(T));
	FieldPtr pField(p->fieldEx());
	if (pField->type())
	{
		DcCleaner_t<> DC(DcInfo_t(*this, memMgrGlob()));
		DC.ClearType(pField);
	}
	SetType(pField, pType);
	p->setTyp(T);
}

//create structure + add field (don't set type of field)
int EXPRPtrSimpl_t::SimplifyIndir5(Out_t * pSelf)
{
	if (!pSelf->is(ACTN_INDIR))
		return 0;

	Out_t *pOut(pSelf->mpR);
	for (;;)
	{
		if (pOut->is(ACTN_TYPE) && TypOf(pOut).isPtr())
		{
			pOut = pOut->mpR;
			continue;
		}
		if (pOut->isActionAddOrSub())
		{
			pOut = pOut->mpL;
			continue;
		}
		break;
	}

	pSelf = pOut;

	//if (pSelf->mAction)
		//return 0;
	if (pSelf->isNumKind())
		return 0;
	if (pSelf->isOpKind())
		return 0;
	//if (pSelf->mKind == OUT_NULL)
		//return 0;
	//if (!pSelf->m_p Op0)
		//return 0;

	Out_t * pOutTop = pSelf;
	
	if (pSelf->isFieldKind())
	{
		if (pSelf->mpU->is(ACTN_PTR))
		{
			if (pSelf->mpU->mpU->is(ACTN_PTR))
				return 0;//only top ptrs
			if (pSelf->isLeft())
				return 0;
			pOutTop = pSelf->mpU;
		}
	}
		
	assert(pOutTop->mpU);

#if(0)
	if (!TypOf(pOutTop->mpU).isPtr())
		if (!pOutTop->mpU->is(ACTN_INDIR))
			return 0;
#endif

	if (pOutTop->mpU->is(ACTN_OFFS))
		return 0;

	int disp = 0;
	if (!ExtractDisplacement(pOutTop, disp))
		return 0;

	Out_t *pOutIndir(pOutTop->CheckIndirParent());
	if (pOutIndir)
		if (!pOutTop->CheckIndirReady())
			return 0;

CHECK(No() == 251)
STOP
//CHECK(miStep == 6)
//STOP

	Out_t* pOutField0(nullptr);
	TYP_t T;

	bool bModified(false);
/*	if (disp == 0 && pOutTop->CheckIndirParent())
	{
		if (!pOutTop->mpU->is(ACTN_INDIR))
			return 0;
		TYP_t T(DerefOf(pSelf));
		if (!T.isComplex())
			return 0;
		T = TypOf(pSelf);
	}
	else*/
	{
		if (pSelf->isFieldKind())
		{
			pOutField0 = pSelf;
		}
		else if (pSelf->is(ACTN_PTR))
		{
			//if (disp == 0)
				//return 0;
			pOutField0 = pSelf->mpR;
		}
		else if (pSelf->is(ACTN_INDIR))
		{
			// *a->b => *(type *)&((a->b)->c)	, offs(c)==0
			if (pSelf->mpR->is(ACTN_PTR))
			{
				Out_t *pOutPtr(pSelf->mpR);
				pOutField0 = pOutPtr->mpR;
				TypePtr iType(nullptr);
				FieldPtr pField0(pOutField0->fieldEx());
				if (pField0->type() && pField0->type()->typePtr())
					iType = pField0->type()->typePtr()->pointee();
				//Struc_t *pStruc(nullptr);
				//if (iType)
					//pStruc = iType->typeStruc();
				//if (!pStruc)
				if (!iType || !iType->typeStruc())
				{
					iType = MakeStruct(MyString(), nullptr, 0);//add to file before last
					AddTypeObj(iType, FileDef_t::AT_BEFORE_LAST);
					T = PtrOf(TYP_t(iType, this));
					SetFieldTyp(pOutField0, T);
				}
				else
				{
					T = PtrOf(TYP_t(iType, this));
				}
				if (iType)
				{
					ProjectInfo_t PI(mrProject);
					FieldPtr pField2(PI.AddField(iType, 0, 2, 0));//in global memmgr!
					if (pField2)
						SetType(pField2, iType);

					Out_t *pOutPtr2(InsertParent(pOutPtr, ACTN_PTR));
					Out_t *pOutField2(Add(*pOutPtr2, pField2));
					Out_t *pOutOffs2(InsertParent(pOutPtr2, ACTN_OFFS));
					return RETURN_1(EXP9);
				}
				else
				{
					return 0;
					assert(0);
				}
			}
			else if (pSelf->mpR->isFieldKind())
			{
				pOutField0 = pSelf->mpR;
			}
			else if (pSelf->mpR->is(ACTN_TYPE))
			{
				return 0;
			}
			else if (pSelf->mpR->is(ACTN_OFFS))
			{
				return 0;
			}
			else
			{
				assert(0);
			}
		}
#if(0)
		else
		{
			HOP pOp = pSelf->m_p Op0;
			if (!pOp)
				return 0;
			pField0 = FindFieldRef(pOp);
			if (!pField0)
			{
				if (!PrimeOp(pOp)->isRoot())
				{
					if (!pOp->IsArgOp())
					{//?
						return 0;
						AnlzRoots_t an1(mrFunc, PrimeOp(pOp), true);
						an1.TurnRoot_On();
					}
				}
				LocalsTracer_t an2(mrFuncDef);
				an2.CheckLocal0(pOp);
				pField0 = fieldRef(pOp);
			}
		}
#endif

		if (pOutField0)
		{
//CHECKID(pOutField0->fieldEx(), 0xe91)
//STOP
			T = TYP_t(fromField(pOutField0->fieldEx()));
			if (disp != 0 && pOutIndir)
			{
				//if (!T.Struc())
				if (!T.isPtr() || !TYP_t(DerefOf(T)).isComplex())
				{
					TypePtr iStruc(MakeStruct(MyString(), nullptr, 0));//add to file before last
					AddTypeObj(iStruc, FileDef_t::AT_BEFORE_LAST);
#if(0)
					fprintf(stdout, "New structure created: %s\n", TypeName(iStruc).c_str());
#endif
					T = PtrOf(TYP_t(iStruc, this));
					FieldPtr pField0(pOutField0->fieldEx());
					if (pField0->isTypePtr())
					{
						//if the field is a pointer, create a new field at zero offset
						TypePtr_t* pPtr(pField0->type()->typePtr());
						if (pPtr && pPtr->pointee())
						{
							ProjectInfo_t PI(mrProject);
							FieldPtr pField2(PI.AddField(iStruc, (ADDR)0, 2, false));//global memmgr
							if (pField2)
								SetType(pField2, pPtr->pointee());
						}
					}
					SetFieldTyp(pOutField0, T);
				}
			}
		}
	}

	TYP_t T0(DerefOf(T));

	if (T0.isComplex())// && disp != 0)//pOutTop->CheckIndirParent())
	{
		bool bCompliant(true);
		if (pOutIndir)
		{
			assert(T.isPtr());
			//TYP_t T2(DerefOf(T));
			if (!AreCompliant(T0, TypOf(pOutIndir)))
				bCompliant = false;
		}
		else if (disp != 0)
			bCompliant = false;

		if (!bCompliant)
		{
			FieldPtr pField = T0.asTypeRef()->typeStruc()->GetFieldEx(disp);//, 1);
			if (!pField)
			{
				ProjectInfo_t PI(mrProject);
				if (disp >= 0)
					pField = PI.AddField((TypePtr)T0.asTypeRef(), disp, 2, false);
				//?assert(pField);
				if (!pField)//?
					return 0;

				SetType(pField, toTypeRef(fromOpType(OPTYP_NULL)));
#if(0)
				if (pOutTop->CheckIndirParent())
					pField->setT ype(TypOf(pOutTop->CheckIndirParent()).toTypeRef());
#endif
			}

			if (disp)
			{
				Out_t * pOutAdd = InsertParent(pOutTop, ACTN_ADD);
				Add(*pOutAdd, HOP(), -disp);
			}

			Out_t * pOutOffs = InsertParent(pOutTop, ACTN_OFFS);
			Out_t * pOutPtr = InsertParent(pOutTop, ACTN_PTR);
			Out_t * pOutFld = Add(*pOutPtr, pField);

			/*if (pOutIndir)
			{
				TYP_t T3;
				T3.SetPtrTo(TypOf(pOutIndir));
				TYP_t T4(pField);
				if (AreCompliant(T4, T3))
					pField->setT ype(T3.toTypeRef());
			}*/

			bModified = true;
		}
	}
	else
	{
		/*if (pOutIndir)
			if (pOutIndir->mpR->is(ACTN_PTR) || pOutIndir->mpR->isFieldKind())
			{
				TYP_t T(DerefOf(pOutIndir->mpR));
				TYP_t Tproxy(ProxyOf(T));
				if (!AreCompliant(Tproxy, TypOf(pOutIndir)))
				{
					FieldPtr pField;
					if (pOutIndir->mpR->is(ACTN_PTR))
						pField = pOutIndir->mpR->mpR->mpField;
					else
						pOutIndir->mpR->mpField;
					if (pField->type()->typePtr)
				}
			}*/
	}

	if (!pSelf->isFieldKind())
	{
#if(0)
		//convert into field
		pSelf->mKind = OUT_FIELD;// OUT_ MLOC;
		pSelf->mpField = pField0;// pSelf->setMLoc(pMLoc);
		assert(pField0);
		pSelf->mSsid = OPC_NULL;
		pSelf->m_offs = 0;
		//pSelf->typ().Clear();
		pSelf->setTyp(T);

		//TYP_t T(pField);
		//assert(AreCompliant(TypOf(pSelf), T));
		//TYP_Set(*pSelf, pField0);
#endif
	}
	if (!bModified)
		return 0;

	return RETURN_1(EXP1);
}

#if(0)
void simplify_ptr(node)
{
	if (!node.is_op || !node.is_field || node.action != ACTN_PTR || node.action != ACTN_ARRAY)
		return 0;
}
#endif

//set field's type
int EXPRPtrSimpl_t::SimplifyIndir6(Out_t * pSelf)
{
	if (!pSelf->is(ACTN_INDIR))
		return 0;

	//eliminate invalid indirection
	if (pSelf->mpR->isImmidiate() && pSelf->mpU->is(ACTN_COMMA2))
	{
		DetachParent(pSelf->sibling());//ACTN_COMMA
		fprintf(STDERR, "Warning: Suspicious indirection at %d\n", No());
		return RETURN_1(EXP5);
	}

	Out_t *pOutType(nullptr);
	Out_t * pOut(pSelf->mpR);
	if (pOut->is(ACTN_TYPE))
	{
		pOutType = pOut;
		pOut = pOut->mpR;
	}

	if (!pOut->is(ACTN_OFFS))
	{
		Out_t *pOutField(pOut);
		if (pOut->is(ACTN_PTR))
			pOutField = pOut->mpR;
		if (!pOutField->isFieldKind())
			return 0;

		TYP_t T(PtrOf(pSelf));

		FieldPtr pField(pOutField->field());
		if (pOutField->expField())
			pField = pOutField->expField();
		assert(pField);
		TYP_t T2(fromField(pField));
		//TYP_t T2(TypOf(pOutField));

		TYP_t T3(DerefOf(T2));
		TYP_t Tproxy(ProxyOf(T3));
		if (AreCompliant(Tproxy, TypOf(pSelf)))
		//if (AreCompliant(T, T2))
		{
			if (pOutType)
			{
				DetachParent(pOut);//ACTN_TYPE
				//pOut->DetachParent();//ACTN_INDIR
				return RETURN_1(EXP8);
			}
			return 0;
			/*T.Comply With(T2);
			pField->setT ype(T.toTypeRef());
			pOut->DetachParent();
			return RETURN_1(EXP4);*/
		}

CHECK(pSelf->mpR->isFieldKind() && pSelf->mpR->field()->ID() == 18375)
STOP

		// *{non-compliant}a => *(&a->_0)
		if (T2.isPtr() && TYP_t(DerefOf(T2)).isComplex())
		{
			TYP_t T3(DerefOf(T2));
			CTypePtr iStruc(T3.asTypeRef());
			//if (!T3.isPtr() && T3.TypeRef())
			if (iStruc)
			{
				FieldPtr pField2(iStruc->typeStruc()->GetFieldEx(0));
				if (pField2)
				{
					Out_t *pOutPtr(InsertParent(pOutField, ACTN_PTR));
					Out_t *pOutField2(Add(*pOutPtr, pField2));
					Out_t *pOutOffs(InsertParent(pOutPtr, ACTN_OFFS));
					return RETURN_1(EXP6);
				}
			}
		}
		
		if (pOutType)
		{
			TYP_t T4(T2);// PtrOf(T2));
			TYP_t T5(TypOf(pOutType));
			if (AreCompliant(T4, T5))
			{
				SetFieldTyp(pOutField, T5);
				/*DcCleaner_t<> DC(DcInfo_t(*this, memMgrGlob()));
				DC.ClearType(pField);
				SetType(pField, (TypePtr)toTypeRef(T5));//T4
				*/
				DetachParent(pOut);//ACTN_TYPE
				//pOut->DetachParent();//ACTN_INDIR
				return RETURN_1(EXP7);
			}
		}
		return 0;
	}

/*	if (!pOut->is(ACTN_OFFS))
	{return 0;
		if (!pOut->is(ACTN_PTR))
			return 0;
		Out_t * pOutPtr = pOut;
		Out_t * pOutField = pOutPtr->mpR;
		
		if (!TypOf(pOutField).isPtr())
			return 0;
		
		TYP_t T(DerefOf(pOutField));
		if (!T.isComplex())
			return 0;
		if (0)
		{
			//			if (!AgreeTypes(TypOf(pOutField).m_optyp, pOutField->m_p Op0->m_optyp))
			//				return 0;
			pOutPtr->DetachParent();//ACTN_INDIR
			return 1;
		}
		
		//		if (!AreCompliant(TypOf(pSelf), T))
		{
			//			if (T.isComplex())
			{
				FieldPtr pField = T.m_pStruc->GetFieldEx(1);
				if (!pField)
				{
					pField = T.m_pStruc->AddF ield(0, 2, 0);
					assert(pField);
				}
				pOutPtr->InsertParent(ACTN_OFFS);
				Out_t * pOutPtr2 = pOutPtr->InsertParent(ACTN_PTR);
				pOutPtr2->Add(pField);
				return 1;
			}
			return 0;
		}
	}*/

	Out_t *pOutOffs(pOut);
	
	pOut = pOutOffs->mpR;
	if (!pOut->is(ACTN_PTR))
	{
		if (!pOut->isFieldKind())
			return 0;
		
		FieldPtr pData = pOut->fieldRef();//?
		if (!pData)
			return 0;

		//pData = pData->AssureObjType(OBJID_FIELD);
		if (pData != pOut->fieldRef())
			pOut->setFieldRef(pData);

		if (ComplyWithFieldType(pSelf, pData) == 0)
			return 0;

		DetachParent(pOut);//ACTN_OFFS
		if (pOutType)
			DetachParent(pOut);//ACTN_TYPE
		DetachParent(pOut);//ACTN_INDIR
		return RETURN_1(EXP3);
	}

//CHECK(mNo==723)
//STOP
	
	Out_t * pOutPtr = pOutOffs->mpR;
	Out_t * pOutRValue = pOutPtr->mpR;

	ComplyWithFieldType(pSelf, pOutRValue->field());

	DetachParent(pOutPtr);//ACTN_OFFS
	if (pOutType)
		DetachParent(pOut);//ACTN_TYPE
	DetachParent(pOutPtr);//ACTN_PTR

	return RETURN_1(EXP2);
}

int EXPRPtrSimpl_t::SimplifyCommaUp(Out_t * pSelf)
{
	// ((a , b) @ c) => (a @ c) , (b @ c) | @ - some action
	//The comma goes up; it's parent is cloned/propagated to comma's children, becoming their new parents
	if (!pSelf->is(ACTN_COMMA2))
		return 0;
	if (pSelf->mpU->is(ACTN_COMMA2))
		return 0;
	if (pSelf->mpU->isRoot())
		return 0;

//		if (++z_step > 10)
	//		return 0;

	Out_t* pOutT = DetachParent(pSelf);// _ @ c
	Out_t* pOutC = Clone(*pOutT);//with sibling branch

	Out_t* pL = pSelf->mpL;
	Detach(pSelf->mpL);
	Add(*pOutT, *pL);//a @ c
	Add(*pSelf, *pOutT);//(a @ c) , _

	Out_t* pR = pSelf->mpR;
	Detach(pSelf->mpR);
	Add(*pOutC, *pR);//b @ c
	Add(*pSelf, *pOutC);//(a @ c) , (b @ c)
	return RETURN_1(COM1);
}

int EXPRPtrSimpl_t::SimplifyRoot(Out_t* pSelf)
{
	if (!pSelf->isRoot())
		return 0;
	if (!pSelf->mpR->is(ACTN_TYPE))
		return 0;
	//(type)expr => expr	// top type removeal
	DetachParent(pSelf->mpR->mpR);
	return 1;
}

int EXPRPtrSimpl_t::SimplifyOp4Ptr(Out_t * pSelf) const
{
	if (!pSelf->isOpKind())
		return 0;
	if (!IS_ADDR(pSelf->mSsid))//if (!(pSelf->mSsid & OPC_ADDRESS))
		return 0;

	*(int *)&pSelf->mSsid &= ~OPC_ADDRESS;
	InsertParent(pSelf, ACTN_OFFS);

	if (pSelf->opc() == OPC_GLOBAL)
	{
		pSelf->mKind = OUT_FIELD;
		pSelf->setField(LocalRef(pSelf->mpOp));
	}

	return RETURN_1(OPD1);
}

bool Out_t::isExtendedBy(const Out_t *pOther) const
{
	assert(isOpFieldKind());
	assert(pOther->isOpFieldKind());
	if (mSsid == pOther->mSsid)
		if (mOffs == pOther->mOffs)
			if (size() < pOther->size())
				return true;
	return false;
}

int EXPRPtrSimpl_t::SimplifyAssignPtr(Out_t *pSelf)
{
	// ((a=b),(ax=0)) => ZEROEXT(b), where 'ax' is extended 'a'
	if (pSelf->is(ACTN_COMMA2))
	{
		Out_t *mpL(pSelf->mpL);
		Out_t *mpR(pSelf->mpR);
		if (mpL->is(ACTN_MOV) && mpR->is(ACTN_MOV))
		{
			Out_t *pOut2(mpL->mpL);
			Out_t *pOut3(mpR->mpL);
			if (pOut2->isOpFieldKind() && pOut3->isOpFieldKind() && mpR->mpR->isNumZero())
			{
				if (pOut2->isExtendedBy(pOut3))
				{
					uint8_t t(MAKETYP_UINT(pOut3->size()));
					Out_t *pOut(pSelf->mpL->mpR);
					DetachParent(pOut);//ACTN_MOV
					DetachParent(pOut);//ACTN_COMMA2
					Out_t *pOutZ(InsertParent(pOut, ACTN_ZEROEXT));
					pOutZ->setTyp(fromOpType(t));
					return RETURN_1(ZER5);
				}
			}
		}
		return 0;
	}

	if (!pSelf->is(ACTN_MOV))
		return 0;
	if (pSelf->mpU->is(ACTN_COMMA2))
		return 0;

	//if (pSelf->mpL->Size() != pSelf->mpR->Size())
		//return 0;

	if (!pSelf->mpL->isOpFieldKind())
		return 0;

	DetachParent(pSelf->mpR);//ACTN_MOV + lvalue
	return RETURN_1(ASS7);
}

int EXPRPtrSimpl_t::DispatchSimplify_Ptr(Out_t * pSelf)
{
	assert(pSelf);
	CHECK_NODE(SimplifyGet(pSelf, 1));
	CHECK_NODE(SimplifyAssign(pSelf));
	CHECK_NODE(SimplifyAssignPtr(pSelf));
	CHECK_NODE(SimplifyFract(pSelf));
	CHECK_NODE(SimplifySign(pSelf));
	CHECK_NODE(SimplifyAdd(pSelf));
	CHECK_NODE(SimplifySub(pSelf));
	CHECK_NODE(SimplifyMultiply(pSelf));
	CHECK_NODE(SimplifyDivide(pSelf));
	CHECK_NODE(SimplifyShiftLeft(pSelf));
	CHECK_NODE(SimplifyShiftRight(pSelf));
	CHECK_NODE(SimplifyBitwiseAnd(pSelf));//main simplification
	CHECK_NODE(SimplifyHalf1(pSelf));
	CHECK_NODE(SimplifyAndOr(pSelf));
	CHECK_NODE(SimplifyCheck(pSelf));
	CHECK_NODE(SimplifyHalf2(pSelf));
	CHECK_NODE(SimplifyCommonMult(pSelf));
	//CHECK_NODE(SimplifyType(pSelf));
	CHECK_NODE(SimplifyOp4Ptr(pSelf));
	//CHECK_NODE(SimplifyAssCheck(pSelf));

	CHECK_EXPR(pSelf->mpL, DispatchSimplify_Ptr(pSelf->mpL));
	CHECK_EXPR(pSelf->mpR, DispatchSimplify_Ptr(pSelf->mpR));

	//CHECK_NODE(AttachGlob2(pSelf));
	CHECK_NODE(SimplifyCommaUp(pSelf));//NP
	CHECK_NODE(SimplifyRoot(pSelf));

	return 0;
}

int EXPRPtrSimpl_t::DispatchSimplify_PtrFields(Out_t * pSelf, PathOpTracer_t &tr)
{
	assert(pSelf);

	CHECK_NODE(SimplifyAssignPtr(pSelf));
	CHECK_NODE(SimplifySign(pSelf));
	CHECK_NODE(SimplifyAdd(pSelf));
	CHECK_NODE(SimplifySub(pSelf));
	CHECK_NODE(SimplifyMultiply(pSelf));
	CHECK_NODE(SimplifyDivide(pSelf));
	CHECK_NODE(SimplifyShiftLeft(pSelf));
	CHECK_NODE(SimplifyShiftRight(pSelf));
	CHECK_NODE(SimplifyBitwiseAnd(pSelf));//main simplification
	CHECK_NODE(SimplifyIndir5(pSelf));
	CHECK_NODE(SimplifyIndir6(pSelf));
	CHECK_NODE(SimplifyOp2Field(pSelf, tr));

	CHECK_EXPR(pSelf->mpL, DispatchSimplify_PtrFields(pSelf->mpL, tr));
	CHECK_EXPR(pSelf->mpR, DispatchSimplify_PtrFields(pSelf->mpR, tr));
			
	return 0;
}

static const int LOOP_MAX = 1000;//to prevent endless loops
void EXPRPtrSimpl_t::SimplifyPtr0(Out_t * pSelf, PathOpTracer_t &tr0)
{
#if(SIMPL_NEW)
	SimplifyNew(pSelf);
	return;
#endif
	unsigned i(0);
	i = PreSimplify(pSelf, i, LOOP_MAX);

	unsigned j(i);
	for (; j < LOOP_MAX; j++)
	{
		if (j > i)
			__dump(j, pSelf);
CHECK(j==10)
STOP
		if (DispatchSimplify_Ptr(pSelf))
			continue;
		break;
	}

	unsigned k(j);
	for (; k < LOOP_MAX; k++)
	{
		if (k > j)
			__dump(k, pSelf);
CHECK(k==10)
STOP
		if (DispatchSimplify_PtrFields(pSelf, tr0))
			continue;
		break;
	}
}

Out_t *EXPRptr_t::TracePtr(HOP pSelf, PathOpTracer_t &tr, bool bResetArrays)
{
	//assert(pSelf->IsIndirect());

/*	int id = -1;
#ifdef _DEBUG
	id = OpNo(PrimeOp(pSelf));
#endif
	int ID = -1;*/
CHECK(OpNo(PrimeOp(pSelf)) == 283)
STOP
//STOP2(ID = id)

//if (id>32)
//return 1;

	if (bResetArrays)
		ResetArrays();

	Out_t *pOutRoot(AddRoot());
	SetDockOp(pOutRoot, pSelf);

	__outPtr(pSelf, pOutRoot, tr);

	return pOutRoot;
}

