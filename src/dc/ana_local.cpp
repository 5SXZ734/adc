#include "anal_local.h"
#include "prefix.h"
#include "shared/defs.h"
#include "shared/link.h"
#include "shared/misc.h"
#include "shared/front.h"
#include "shared/action.h"
#include "db/mem.h"
#include "db/field.h"
#include "db/info_types.h"
#include "op.h"
#include "info_dc.h"
#include "path.h"
#include "anal_data.h"
#include "expr_ptr.h"
#include "reg.h"
#include "clean_ex.h"
#include "anal_main.h"
#include "cc.h"

LocalsTracer_t::LocalsTracer_t(const FuncTracer_t &r)
	: FuncTracer_t(r),
	mpRedumpCache(nullptr)
	//mpCachedArgs(nullptr)
{
}

LocalsTracer_t::LocalsTracer_t(const FuncInfo_t &fi, PathOpTracer_t &tr)
	: FuncTracer_t(fi, tr),
	mpRedumpCache(nullptr)
	//mpCachedArgs(nullptr)
{
}


bool FuncInfo_t::IsStackPtrOp(CHOP pSelf) const
{
	if (pSelf->SSID() == mrFE.stack_ptr->ssid)
		if (pSelf->OpOffsU() == mrFE.stack_ptr->ofs)
			if (pSelf->OpSize() == mrFE.stack_ptr->siz)
				return true;
	return false;
}

bool FuncInfo_t::IsStackPtrB(CHOP pSelf) const
{ 
	if (pSelf->IsIndirectB())
		if (pSelf->SSID() == mrFE.stack_ptr->ssid)
			if (pSelf->OpOffsU() == mrFE.stack_ptr->ofs)
				return true;
	return false;
}

int LocalsTracer_t::__traceLocal(HOP pSelf, int32_t *stacktop)
{
	if (!IsCode(pSelf))
	{
		if (!IsArgOp(pSelf))
		{
			fprintf(STDERR, "*** bad code ***\n");
			return 0;
		}
		assert(IsArgOp(pSelf));//!
		return 0;
	}

	HOP pOp;
	if (pSelf->IsIndirectB())
	{
		if (!pSelf->XIn())
		{
			ExprCacheEx_t aExprCache(PtrSize());
			AnlzXDepsIn_t anlzxdepsin(*this, pSelf, aExprCache);
			anlzxdepsin.ScanXDepsBase(false);
			//!ScanXDepsBase(false);
		}
		if (pSelf->m_xin.check_count(1) != 0)
			return 0;
		pOp = pSelf->XIn()->data();
		if (pOp == PrimeOp(pSelf))
			return 0;
	}
//	else if (OpC() == OPC_CPUREG)
	else if (pSelf->OpSize() == mrDC.PtrSize())
	{
		if (!pSelf->XIn())
		{
			if (IsLocalOp(pSelf))
				opTracer().cell(pSelf).trace |= TRACE_PASSFUNC;
			ExprCacheEx_t aExprCache(PtrSize());
			AnlzXDepsIn_t an(*this, pSelf, aExprCache);
			//an.setOpTracer(&opTracer());
			an.ScanXDeps0(PrimeOp(pSelf));
		}

		if (pSelf->m_xin.check_count(1) != 0)
		{
			if (IsAddr(pSelf) && IsLocalOp(pSelf))
			{
				*stacktop = pSelf->Offset0();
				return 1;
			}
			return 0;
		}
		pOp = pSelf->XIn()->data();
	}
//	else if (OpC() == OPC_IND IRECT+OPC_CPUREG)
//	else if (IsStackPtrB())
	else 
		return 0;
	
	if (!IsCodeOp(pSelf))
		return 0;

	if (IsCallOutOp(pOp))
	{
		HOP hOpCall(PrimeOp(pOp));
		GlobPtr iGlob(GetCalleeFuncDef(hOpCall));
		if (!iGlob)
		{
			ExprCacheEx_t aExprCache(PtrSize());
			iGlob = GetCalleeFuncDefEx(hOpCall, aExprCache, pathOpTracer());
			if (!iGlob)
				return 0;
		}
		if (iGlob != FuncDefPtr())//sanity check (later: recursion!)
		{
			FieldPtr pRetField(FindCalleeRetField(iGlob, pOp, hOpCall));
			if (!pRetField)
				return 0;

			CFolderPtr pFolder2(iGlob->folder());
			CTypePtr iModule2(ModuleOf(pFolder2));

			FileDef_t& rCalleeFileDef(*pFolder2->fileDef());

			bool bResult(false);
			{//RAII - ENTERING A CALLEE!
				tls::MemPoolSwapper<MemOpPool> opPoolSwapper(nullptr);// &rCalleeFileDef.memMgrEx().mOps);
				tls::MemPoolSwapper<MemPathPool> pathPoolSwapper(nullptr);// &rCalleeFileDef.memMgrEx().mPaths);

				FuncInfo_t FI2(*DCREF(iModule2), *iGlob);

				const FuncDef_t& rfDef(*iGlob->typeFuncDef());
				for (XOpList_t::Iterator i(GetExitPoints(rfDef)); i; ++i)
				{
					HOP pOpRet(i.data());//RETURN op
					HOP hOp(FindOpFromRetField(pOpRet, pRetField));
					if (hOp)
					{
						PathOpTracer_t tr2;
						LocalsTracer_t LT2(FI2, tr2);

						int32_t stackout;
						if (LT2.__traceLocal(hOp, &stackout))
						{
							stacktop += stackout;
							bResult = true;
							break;
						}
					}
				}
			}

			if (bResult)
			{
				//*stacktop += StackTop(hOpCall);
				*stacktop += hOpCall->pstackIn();
				return 1;
			}

/*			LocalsTracer_t an(*this);
			//an.setOpTracer(&opTracer());
			if (!an.CheckStackDiff(pSelf))
			{
			}*/
		}
		return 0;
	}

	if (!IsPrimeOp(pOp))
		return 0;

	if (ActionOf(pOp) == ACTN_MOV)
	{
		HOP op0 = pOp->arg1();
		if (IsStackPtrOp(op0))
		{
			*stacktop = StackTop(pOp);
			return 1;
		}
		else if (IsLocalOp(op0) && IsAddr(op0))
		{
			*stacktop = op0->Offset0();
			return 1;
		}
		else
		{
			if (__traceLocal(op0, stacktop))
			{
		//		*stacktop += op0.m_disp;
				return 1;
			}
		}
	}
	else if (ActionOf(pOp) == ACTN_ADD)
	{
		HOP op0(pOp->arg1());
		if (IsStackPtrOp(op0))
		{
			*stacktop = StackTop(pOp);
			HOP op1(pOp->arg2());
			if (op1->IsScalar())
			{
				*stacktop += op1->m_disp;
				return 1;
			}
			assert(0);
		}
		else if (IsLocalOp(op0) && IsAddr(op0))
		{
			HOP op1(pOp->arg2());
			if (op1->IsScalar())
			{
				*stacktop = op0->Offset0();
				*stacktop += op1->m_disp;
				return 1;
			}
		}
	}
	return 0;
}

STAGE_STATUS_e LocalsTracer_t::MakeLocals()
{
	for (PathTree_t::LeafIterator i(mrPathTree.tree()); i; i++)
	{
		HPATH rPath(i.data());
		for (PathOpList_t::Iterator j(rPath->ops()); j; j++)
		{
			HOP pOp0(PRIME(j.data()));
CHECK(OpNo(pOp0) == 40)
STOP
			MakeLocal(pOp0);

			for (OpList_t::Iterator k(pOp0->argsIt()); k; k++)
			{
				HOP pOp2(k.data());
				MakeLocal(pOp2);
			}
		}
	}
	return STAGE_STATUS_e::DONE;
}

int LocalsTracer_t::MakeLocal(HOP pSelf)
{
	if (pSelf->SSID() != mrFE.stack_ptr->ssid)//SSID_CPUREG)
		return 0;

//CHECK(pSelf->IsCode() && PrimeOp(pSelf)->No() == 18)
CHECK(OpNo(PrimeOp(pSelf)) == 108)
STOP

	int32_t _stacktop = 0;
	if (!IsArgOp(pSelf))
	{
//CHECK(!pSelf->IsCode())
//STOP
	//?	assert(pSelf->IsCode());
		if (!IsAnalized(PrimeOp(pSelf)))
			return 0;
		
		if (IsPrimeOp(pSelf))
			_stacktop = StackTop(pSelf);
		else
			_stacktop = PrimeOp(pSelf)->pstackIn();
	}

	if (pSelf->IsIndirectB())
	{
		if (!IsStackPtrB(pSelf))//if not [esp]
		{
			if (!__traceLocal(pSelf, &_stacktop))
				return 0;
		}

		if (InsRefPrime(pSelf).opnd_BAD_STACKOUT)
			return 0;

		int offs = _stacktop + pSelf->m_disp;

//		assert(OpSeg() == OPSEG_STACK);
//		SetO pSeg(OPSEG_NULL);
		ClearXIns(pSelf);
		pSelf->m_opc = OPC_LOCAL;
		pSelf->m_disp0 = pSelf->m_disp;
		pSelf->m_disp = offs;
		return 1;
	}
	else
	{
		if (IsLValue(pSelf))
			return 0;
	}

	assert(!pSelf->OpSeg());

	if (IsStackPtrOp(pSelf))
	{
		HOP pRootOp = PrimeOp(pSelf);
		if (ActionOf(pRootOp) == ACTN_MOV)
		if (pRootOp->OpC() == OPC_CPUREG)
			if (pRootOp->OpOffsU() == mrFE.stack_ptr->ofs)
			if (pRootOp->OpSize() == mrFE.stack_ptr->siz)
				return 1;
			
		if (InsRefPrime(pSelf).opnd_BAD_STACKOUT)
			return 0;
	}
	else
	{
		if (pSelf->OpOffsU() != mrFE.stackb_ptr->ofs || pSelf->OpSize() != mrFE.stackb_ptr->siz)
			return 0;

		_stacktop = 0;
		if (!__traceLocal(pSelf, &_stacktop))
			return 0;
	}

	if (IsCallArg(pSelf) || IsGotoArg(pSelf))
		return 0;

	int offs = _stacktop + pSelf->m_disp;

	ClearXIns(pSelf);
	pSelf->m_opc = OPC_ADDRESS | OPC_LOCAL;

#if(0)
	HOP pRoot = PrimeOp(pSelf);
	int m = 0;
	if (pRoot->mAction() == ACTN_ADD)
		m = 1;
	else if (PrimeOp(pSelf)->mAction() == ACTN_SUB)
		m = -1;
	if (m)
	{
		assert(&pRoot->arg1() == pSelf);
		if (pRoot->arg2()->IsScalar())
		{
			int d = pRoot->arg2()->i32;
			pSelf->m_disp += d * m;
			offs += d * m;
			Op_t::Delete(&pRoot->arg2());
			pRoot->setAction(ACTN_MOV);
		}
	}
#endif

	pSelf->m_disp0 = pSelf->m_disp;
	pSelf->m_disp = offs;
	return 1;
}


void LocalsTracer_t::AttachToLocal(HOP pSelf)
{//for locals only

	if (LocalRef(pSelf))
	{
		//assert(IsData(LocalRef(pSelf)));
		assert(IsLocal0(LocalRef(pSelf)));
		return;//already
	}

	if (pSelf->IsIndirectB())
		return;

	if (!mrFuncDef.Body())
		return;

#if(0)
	assert(!ProtoInfo_t::IsFuncStatusFinished(&mrFuncDefRef));

	/*?FieldPtr pData = nullptr;
	int ssid = SSID_LOCAL;
	int offs = pSelf->Offset0() + CalcDispl(pSelf);
	int sz = pSelf->OpSize();

	if (mrFuncDef.Body())
	{
		if (mrFuncDef.Body()->m.hasL ocals())
		{
			const FieldMap &l = mrFuncDef.Body()->m.locals();
			for (FieldMapCIt it = l.begin(); it != l.end(); it++)
			{
				FieldPtr  pLocal = VALUE(it);
				int ssid2 = pLocal->StorageId();
				if (ssid2 < ssid)
					continue;
				if (ssid2 > ssid)
					break;

				int offs2 = pLocal->Offset();
				if (offs < offs2)
					break;
				int sz2 = pLocal->size();
				if (checkoverlap(offs2, sz2, offs, sz))
				{
					pData = pLocal;
					offs = offs-offs2;
					break;
				}
			}
		}
	}

	if (offs == 0)//retaddr?
		return 0;*/
#endif

	FieldPtr pLocal(AttachLocalVar(pSelf));

#if(0)
	if (IsArgOp(pSelf))
		return;

	HOP pVarOp(CreateVarOp(pLocal));
	if (!pVarOp)
		return;

	InsertVarOpAt(pVarOp, pSelf);

	HPATH pPath(GetResideBlock(pLocal));
	pPath = GetLocalImplantPath(pPath);

	if (PathOf(pVarOp) != pPath)
	{
		TakeOp(PathOf(pVarOp), pVarOp);
		RelocateVarOp(pVarOp, pSelf, pPath);
	}
#endif
}

FieldPtr LocalsTracer_t::AttachLocalVar(HOP hOpRef)
{
CHECKID(hOpRef, 0x182e)
STOP
	FieldPtr pLocal(nullptr);
	int disp(0);
	r_t r(hOpRef->toR());
	//if (hOpRef->IsIndirect() || IsAddr(hOpRef))
	if (hOpRef->SSID() == SSID_LOCAL)
	{
		disp = hOpRef->Offset0() + CalcDispl(hOpRef);
		r.ofs = 0;
	}

	if (!IsArgOp(hOpRef))
	{
		if (!IsCodeOp(hOpRef))
			if (!IsCallOutOp(hOpRef))
				return nullptr;

		if (mpRedumpCache)
		{
			//OpPtr pOpAdjourned(mpRedumpCache->findTransOp(hOpRef));
			pLocal = mpRedumpCache->findPermOp(hOpRef);
			//if (pOpAdjourned)
			{
//CHECKID(pOpAdjourned, 0x2fdd)
//STOP
				//pLocal = LocalRef(pOpAdjourned);
				if (pLocal)
				{
					assert(SSIDx(pLocal) == r.ssid);
					int off((int8_t)r.ofs);
					if (SSIDx(pLocal) == SSID_LOCAL)
						off = disp;
					int addr(address(pLocal));
					if (addr == off)
					{
/*?						hOpVar = pOpAdjourned;
						if (!hOpVar->isLinked())//dangling?
						{
							SetupPrimeOp(hOpVar);//link below
							assert(hOpVar->OpSize() == hOpRef->OpSize());//ignore optype
							assert(hOpVar->m_disp == disp);
						}*/
					}
					else
					{
						assert(0);//later - local has shifted!
						pLocal = nullptr;
					}
				}
			}
		}
	}

	if (pLocal)
	{
		AddLocalRef(pLocal, hOpRef);
		return pLocal;
	}

	if (r.ssid == SSID_LOCAL)
	{

		ADDR uKey(loc2key(r.ssid, disp));

		if (0 < disp + hOpRef->OpSize() && disp < RetAddrSize())
		{
//			assert(0);
			fprintf(STDERR, "Warning: Return address overwrite at L%s\n", StrNo(PrimeOp(hOpRef)).c_str());
			return nullptr;
		}
		else
		{
			if (IsArgOp(hOpRef))
			{
				pLocal = FindArgFromOp(hOpRef);
				if (pLocal)
				{
					AddLocalRef(pLocal, hOpRef);
					return pLocal;
				}
				pLocal = dc().memMgr().NewField(uKey);
			}
			else
				pLocal = mrMemMgr.NewField(uKey);
		}
	}
	else
	{
		if (!r.ssid)
			return nullptr;//?
		ADDR uKey(loc2key(r.ssid, (int8_t)r.ofs));
		if (IsArgOp(hOpRef))
		{
			pLocal = FindArgFromOp(hOpRef);
			if (pLocal)
			{
				AddLocalRef(pLocal, hOpRef);
				return pLocal;
			}
			pLocal = dc().memMgr().NewField(uKey);
		}
		else
			pLocal = mrMemMgr.NewField(uKey);
	}

//	assert(pLocal->Offset() == nOffset);//pLocal->setOffset(nOffset);
	TypePtr iType(nullptr);
	if (!(hOpRef->m_opc & OPC_ADDRESS))
		iType = findOpType((OpType_t)hOpRef->m_optyp);
	DcCleaner_t<> DC(*this);
	DC.ClearType(pLocal);
	if (iType)
		SetType(pLocal, iType);
	else if (localOffset(pLocal) == r.ofs)//hOpVar->Offset0())
		SetLocalsTypeFromOp(pLocal, r.typ);// hOpVar->OpType());

	assert(!pLocal->owner());
	//assert(IsVarOp(hOpVar) || IsArgOp(hOpVar));// rPath.Type() != BLK_EXIT);
	if (IsArgOp(hOpRef))
		AddLocalArg(pLocal);
	else
		AddLocalVar(pLocal);

	AddLocalRef(pLocal, hOpRef);

	return pLocal;
}

HOP LocalsTracer_t::CreateVarOp(FieldPtr pLocal)
{
//CHECK(OpNo(PrimeOp(hOpRef)) == 152)
//CHECKID(hOpRef, 0x167e)
//STOP

	if (!pLocal)
		return HOP();

	HOP hOpVar(GetVarOp(pLocal));
		
	if (!hOpVar)
	{
		uint8_t ssid(SSIDx(pLocal));
		uint8_t off(ssid == SSID_LOCAL ? 0 : address(pLocal));
		uint8_t optyp(pLocal->OpTypeZ());
		int disp(ssid == SSID_LOCAL ? address(pLocal) : 0);


		hOpVar = NewPrimeOp();
		hOpVar->Setup4(ssid, optyp, off, disp);
	
	}
	else
	{
		assert(hOpVar->isLinked());
	}


CHECKID(pLocal, 0x1fff)
STOP

	AddLocalRef(pLocal, hOpVar);

	return hOpVar;
}

/*FieldPtr LocalsTracer_t::TakeCachedArg(CHOP hOpRef)
{
	if (mpCachedArgs)
	{
		for (FieldMapIt it(mpCachedArgs->begin()); it != mpCachedArgs->end(); ++it)
		{
			FieldPtr pField(VALUE(it));
			ri_t r(pField->toR_t());
			if (IsOpOverlappedByField(hOpRef, r))
			{
				mpCachedArgs->take(it);
				pField->setOwnerComplex(nullptr);//now
				return pField;
			}
		}
	}
	return nullptr;
}*/


int LocalsTracer_t::AttachLocalToOp(FieldPtr pSelf, HOP pOp)//, int d)
{
CHECKID(pSelf, 0x10fa)
STOP
	assert(pOp);
	assert(!ProtoInfo_t::IsSpoiltReg(pSelf));

//	if (pOp->IsL ocal())
//		return 1;

	int retaddrsize = RetAddrSize();

	int nOffset = address(pSelf);

	int sz = pOp->OpSize();
	if (nOffset+sz > 0 && nOffset < retaddrsize) 
	{
		fprintf(STDERR, "Warning: Return address overwrite warning detected");
		return 0;
	}

	if (pOp->OpC() == OPC_CPUREG)
	{
		assert(0);
		/*?Add OpRef(pSelf, pOp);
		pOp->ClearXIns(mrMemMgr);
		pOp->Setup3(OPC_ ADDRESS|OPC_LOCAL, OPTYP _PTR|mrDC.PtrSize(), 0);
		pOp->m_disp0 = pOp->m_disp;
		pOp->m_disp = d;*/
	}

	if (pOp->SSID() == SSID_LOCAL)
	{
		AddLocalRef(pSelf, pOp);
//!		d = pOp->m_disp-nOffset;
//!		pOp->m_disp = d;

		if (IsArgOp(pOp))
		{
			//assert(d == 0);
//?			SetOp(pOp);
		}
	}
	else
		return 0;

/*	if (0)
	if (m_nOffset >= 0+retaddrsize)//arg!
	if (!m_pOp_)
	{
		HOP pOpEntry = GetOwner Func()->RegisterEntryOp(pOp);
		AddXRef(pOpEntry);
//		pOpEntry->Setup3(pOpEntry->OpC(), pOpEntry->optyp, 0);
		d = pOpEntry->m_disp - nOffset;
		assert(d == 0);
		pOpEntry->m_disp = d;
		m_pOp_ = (FieldPtr )pOpEntry;
	}*/

/*?	if (!m_pOp_)
	if (IsAddr(pOp))
	{
		AssureOp(0);
		return 1;
	}
	else
	{
		AssureOp(pOp->OpType());
	}*/

	if (localOffset(pSelf) == pOp->Offset0())
		SetLocalsTypeFromOp(pSelf, pOp->OpType());
	return 1;
}

void LocalsTracer_t::SetLocalsTypeFromOp(FieldPtr pSelf, uint8_t t)
{
	uint8_t opsz(t & OPSZ_MASK);
	if (OPTYP_IS_PTR(t))
	{
		t = OPSIZE(t);
	}
	TypePtr iType(GetStockType((OpType_t)t));
	if (!pSelf->type())
	{
		SetType(pSelf, iType);
	}
	else
	{

		int d(pSelf->type()->size() - opsz);
		if (d < 0)
		{
			if (!pSelf->type())
				SetType(pSelf, iType);
		}
		else if (d == 0)
		{
			//TypesMgr_t *pTypesMgr(findTypeMgr(iType));
			//TypesTracer_t TT(*this, *pTypesMgr);
			TypePtr iType2(ReconcileTypes(pSelf->type(), iType));
			if (iType2 && iType2 != pSelf->type())
			{
				DcInfo_t DI(*this, memMgrGlob());
				DcCleaner_t<> DC(DI);
				DC.ClearType(pSelf);
				SetType(pSelf, iType2);
			}
		}
	}
}

FieldPtr LocalsTracer_t::DetachMLoc(HOP pSelf)
{
	FieldPtr pData(LocalRef(pSelf));
	if (!pData)
		return 0;

	if (IsStackLocal(pData))
	{
//		m_disp += pData->Offset();
		//m_opc = OPC_LOCAL;
		assert(pSelf->SSID() == SSID_LOCAL);
	}
	else 
	{
		if (!IsLocalReg(pData))
			return 0;
	}

	bool bDecompiled = ProtoInfo_t::IsFuncStatusFinished(&mrFuncDefRef) != 0;

	if (pSelf->isCombined() && !IsArgOp(pSelf))
		pData->SeparateFromCodeOp();

	ReleaseLocalRef(pData, pSelf);
/*	if (!(pOwnerFunc->m_nFlags & FNC_REANLZ))
	if (pData->IsUnrefed())
	{
		pData->Delete();
		if (bDecompiled)
			INFOMSG(MSG_INFO, "%s: Local variable was destroyed", buf);
	}*/

	return pData;
}

int FuncInfo_t::__tracePtrDispl(HOP pSelf, int32_t& displ) const
{
	if (pSelf->IsIndirectB())
	{
		displ += pSelf->m_disp;
		if (!pSelf->XIn())
			return 0;
		return __tracePtrDispl(pSelf->XIn()->data(), displ);
	}

	if (pSelf->OpSize() != OPSZ_DWORD)
		return 1;

	if (IsPrimeOp(pSelf))
	{
		if (ActionOf(pSelf) == ACTN_ADD)
		{
			__tracePtrDispl(pSelf->arg1(), displ);
			__tracePtrDispl(pSelf->arg2(), displ);
		}
		else if (ActionOf(pSelf) == ACTN_SUB)
		{
			__tracePtrDispl(pSelf->arg1(), displ);
			int32_t displ2 = 0;
			__tracePtrDispl(pSelf->arg2(), displ2);
			displ -= displ2;
		}
		else if (ActionOf(pSelf) == ACTN_MOV)
		{
			__tracePtrDispl(pSelf->arg1(), displ);
		}

		return 1;
	}
	
	if (pSelf->m_xin.check_count(1) == 0)
	{
		__tracePtrDispl(pSelf->XIn()->data(), displ);
		return 1;
	}

	if (pSelf->IsScalar())
	{
		displ += pSelf->m_disp;
		return 1;
	}

	if (IsAddr(pSelf))
	{
		assert(IsLocalOp(pSelf));
		displ += pSelf->Offset0();
	}

	return 1;
}

int FuncInfo_t::CalcDispl(HOP pSelf) const
{
	if (IsPrimeOp(pSelf))
		return 0;

	if (pSelf->OpC() != (OPC_ADDRESS | OPC_LOCAL))
		return 0;

	//trace for expression root
	HOP pOp(pSelf);
	HOP pOpPr(pOp);
	while (1)
	{
		if (pOp->IsIndirectB())
			break;

		if (IsPrimeOp(pOp))
		{
			if (pOp->m_xout.check_count(1) != 0)
				break;
			pOpPr = pOp;
			pOp = pOp->XOut()->data();
			continue;
		}
		else
			if (pOp != pSelf)
				if (pOp->m_xin.check_count(1) != 0)
				{
					pOp = pOpPr;
					break;
				}

		HOP pOpR(PrimeOp(pOp));
		if (ActionOf(pOpR) != ACTN_MOV)
		if (ActionOf(pOpR) != ACTN_ADD)
		if (ActionOf(pOpR) != ACTN_SUB)
			break;
		pOpPr = pOp;
		pOp = pOpR;
	}

	//trace for ptr displacement

	int32_t displ = 0;
	__tracePtrDispl(pOp, displ);

/*	int d = 0;
	if (pRoot->Action() == ACTN_ADD)
	{
		assert(&pRoot->arg1() == this);
		if (pRoot->arg2()->IsScalar())
			d = pRoot->arg2()->i32;
	}
	else if (PrimeOp(pRoot)->Action() == ACTN_SUB)
	{
		assert(&pRoot->arg1() == this);
		if (pRoot->arg2()->IsScalar())
			d = -pRoot->arg2()->i32;
	}*/

	return displ - pSelf->Offset0();
}

int LocalsTracer_t::AddLocalArg(FieldPtr pField)
{
	ProtoInfo_t TI(*this);
	TI.InsertLocalArg(pField);
	return 1;
}

int LocalsTracer_t::AddLocalVar(FieldPtr pField)
{
//CHECK(uOffset == (ADDR)-0x1C)
//STOP

	TypePtr iStrucVars(nullptr);
#if(!NEW_LOCAL_VARS)
	TypePtr iUnionTop(nullptr);
	FieldPtr pParentField(mrFuncDefRef.parentField());
	if (!pParentField)
	{
		TypeUnion_t *pUnion(new TypeUnionLoc_t());
		iUnionTop = memMgr().NewTypeRef(pUnion);
		ON_OBJ_CREATED(iUnionTop);
		pParentField = AppendUnionFieldOfType(iUnionTop, &mrFuncDefRef);
	}
	else
	{
		iUnionTop = pParentField->OwnerComplex();//union
		assert(iUnionTop);
	}
	TypeUnion_t *pUnionTop(iUnionTop->type()->typeUnion());

	FieldMapIt it(pUnionTop->fields().begin());
	it++;//skip funcdef

	//find the strucloc
	for (FieldMapIt E(pUnionTop->fields().end()); it != E; it++)
	{
		FieldPtr pField2(VALUE(it));
		Struc_t *pStrucLoc(pField2->type()->typeStruc());
		if (pStrucLoc->fields().find(uOffset) == pStrucLoc->fields().end())//not found, use it
			break;
	}

	FieldPtr pFieldVars(nullptr);
	if (it != pUnionTop->fields().end())
		pFieldVars = VALUE(it);

	//assert(VALUE(it) == pParentField);
	//assert(VALUE(it)->type()->typeFuncDef());

	if (!pFieldVars)
	{
		Struc_t *pStrucVars(new TypeStrucLoc_t());
		iStrucVars = memMgr().NewTypeRef(pStrucVars);
		ON_OBJ_CREATED(iStrucVars);
		//pStrucVars->SetOwnerPath(&rPath);
		pFieldVars = AppendUnionFieldOfType(iUnionTop, iStrucVars);
	}
	else
	{
		iStrucVars = pFieldVars->type();
		assert(iStrucVars);
	}
	assert(!pField->owner());
	FieldMapIt i(InsertUniqueFieldIt(iStrucVars, uOffset, pField));
	assert(i != iStrucVars->typeStruc()->fields().end());
	//?AddLocalVar(pField, pOpVar, true);
#else
	iStrucVars = mrFuncDef.locals();
	if (!iStrucVars)
	{
		Struc_t *pStrucLoc(new TypeStrucLoc_t());
		iStrucVars = memMgr().NewTypeRef(pStrucLoc);
		iStrucVars->setOwner((GlobToTypePtr)FuncDefPtr());
		mrFuncDef.setLocals(iStrucVars);
		ON_OBJ_CREATED(iStrucVars);
	}

	assert(!pField->owner());
	FieldMapIt i(InsertFieldIt(iStrucVars, pField));
	assert(i != iStrucVars->typeStruc()->fields().end());

#endif
	return 1;
}

FieldPtr  LocalsTracer_t::GetAttachData(CHOP pSelf) const
{
//?	if (pSelf->IsArgOp())
//?		return 0;

	int d = CalcDispl(pSelf);

	HPATH pPath0 = HPATH();
	HPATH  pPath = PathOf(pSelf);
	while (pPath)
	{
		FieldPtr pData = FindOverlappedLocalVar(pSelf, TreeTerminalFirst(pPath), d + pSelf->Offset0());
		if (pData)
			return pData;

		while (pPath) 
		{
			pPath0 = pPath;
			pPath = TreePrev(pPath0);
			if (pPath)
				break;
			pPath = pPath0->Parent();
		}

		if (pPath)
			pPath = GetLocalsBlock(pPath);//where locals may reside
	}

	assert(pPath0 && TreeIsBody(pPath0));
	return FindOverlappedLocalVar(pSelf, pPath0, d + pSelf->Offset0());
}

void LocalsTracer_t::OnLocalReference(FieldPtr pData, HOP pOp)
{//return;
	if (pData->type())
		if (!pData->type()->typeSimple())
			return;

	if (ProtoInfo_t::IsFuncStatusFinished(&mrFuncDefRef))
		return;

	if (pOp->isAddr())
		return;

	int off1(address(pData));
	int off2(pOp->Offset0());
	if (off1 != off2)
		return;

	uint8_t t0(0);
	if (pData->type())
		t0 = pData->type()->typeSimple()->optype();
	uint8_t t1(t0);
	uint8_t t2(pOp->OpType());
CHECKID(pData, 0x17fe)
STOP
	if (AgreeTypes(t1, t2) && t1 != t0)
	{
		//if (OPTYP_IS_PTR(t1))
			//t1 &= OPSZ_MASK;

		TypePtr iType(GetStockType((OpType_t)t1));
		if (!iType)
		{
			TypesTracer_t TT(*this, *mrModule.typeMgr());
			iType = TT.ptrOf(nullptr, PtrSize());
		}
		DcCleaner_t<> DC(*this);
		DC.ClearType(pData);
		SetType(pData, iType);
	}
}

int LocalsTracer_t::CheckLocal0(HOP pSelf, bool bForce)
{//return 1;
	if (pSelf->IsScalar())
		return 0;

CHECKID(pSelf, 6195)
//CHECK(OpVA(pSelf) == 0x50bfb8)
STOP

	if (IsCode(pSelf) || IsCallOutOp(pSelf))
	{
CHECK(OpNo(PrimeOp(pSelf)) == 76)
STOP
//		if (IsAddr())
//			return 0;

		HOP pOpRoot = PrimeOp(pSelf);
		if (pOpRoot->isHidden() || !IsOnTop(pSelf))//!pOpRoot->isRoot())
		{
			FieldPtr pField(LocalRef(pSelf));
			if (pField)
			{
				assert(!IsVarOp(pSelf) && !IsArgOp(pSelf));
				FuncCleaner_t FC(*this);
				//FC.RemoveUnrefedVarOp(pSelf);
				FC.DetachLocalRef(pSelf);
				FC.Cleanup();
				FC.CleanupFinal();//watever unref'd types
			}
			return 0;
		}
	}

	if (IsThisPtrOp(pSelf))
	{
		assert(LocalRef(pSelf));
		return 0;
	}

	if (!pSelf->OpC() || IsGlobalOp(pSelf))
		return 0;
	if (IsPrimeOp(pSelf))
	{
		if (IsCall(pSelf))
			return 0;
		if (IsGoto(pSelf))
			return 0;
		if (IsCondJump(pSelf))
			return 0;
	}

	if (pSelf->IsIndirectB())
		return 0;
	if (IsStackPtrOp(pSelf))
		return 0;

CHECK(IsArgOp(pSelf))
STOP

/*	if ((OpC()&0xf) == OPC_LOCAL)
	{
//CHECK(Offset0() == -0xB4)
//STOP
		if (!AttachToData())
			return 0;
	}*/

	FieldPtr pFieldRef(LocalRef(pSelf));
	if (!pFieldRef)
	{
		if (PathType(PathOf(pSelf)) == BLK_DATA)
		{
			pFieldRef = PathLabel(PathOf(pSelf));
		}
	}
	if (pFieldRef)
	{
//CHECK(pFieldRef->Is Global())
//STOP
		if (IsArgOp(pSelf))
			if (localOffset(pFieldRef) == pSelf->Offset0())
				SetLocalsTypeFromOp(pFieldRef, pSelf->OpType());
//		if (!fieldRef()->IsDa ta())
//			return 1;
//		if (fieldRef()->m_pOp_->Overlap(this))
			return 1;//already
	}

	if (StorageFlags(pSelf) & SS_NOMLOC)
		return 0;
/*	switch (OpC())
	{
	case OPC_ CPUSW:
	case OPC_FPUSW:
	case OPC_SEGREG:
		assert(!m_pData);
		return 0;
	}*/


	bool bDecompiled = ProtoInfo_t::IsFuncStatusFinished(&mrFuncDefRef) != 0;

	if (IsRhsOp(pSelf))
	{
		if (pSelf->XIn())
		{
			FieldPtr pData(GetAttachData(pSelf));
			if (pData)//just adjust data's type
				OnLocalReference(pData, pSelf);
			return 0;
		}
		//something like: EDX = &VAR_14<no refs>
//		else if (m_nFlags & OPND_XINEX)
		{
			FieldPtr pData(GetAttachData(pSelf));
			if (pData)
			{
				AddLocalRef(pData, pSelf);
				OnLocalReference(pData, pSelf);
				SetChangeInfo(FUI_XOUTS);
				return 1;
			}

		}
		/*if (!bDecompiled  && IsLocalOp(pSelf))
			AttachToLocal0(pSelf);
		else*/
			AttachToLocal(pSelf);
		return 1;
	}

//	if (!XOut())
//		return 0;

	REG_t REG;
	for (XOpList_t::Iterator i(pSelf->m_xout); i; i++)
	{
		HOP pOpOut = i.data();

		if (pOpOut->SSID() != pSelf->SSID())
			continue;//flags usage case

		if (pOpOut->IsIndirectB())
			REG.MergeWith( REG_t(pOpOut->OpOffs(), mrDC.PtrSize()) );
		else
			REG.MergeWith( REG_t(pOpOut->Offset0(), pOpOut->OpSize()) );

		for (XOpList_t::Iterator j(pOpOut->m_xin); j; j++)
		{
			HOP pOpIn = j.data();
//			if (pOpIn == this)
//				continue;

			if (IsCallOutOp(pOpIn))
			if (PrimeOp(pOpIn)->m_xout.check_count(1) != 0)
				continue;

			REG.MergeWith( REG_t(pOpIn->Offset0(), pOpIn->Size()) );

			if (!LocalRef(pOpIn))
				continue;

			if (IsLocalOp(pSelf))
			{
				FieldPtr pData = LocalRef(pOpIn);
				if (!pData->IsComplexOrArray())
				{
					if (!ProtoInfo_t::IsFuncStatusFinished(&mrFuncDefRef))
					{
						if (REG.m_ofs < (int)address(pData))
						{
							//?assert(0);//?							pData->SetOffset(REG.m_ofs);
						}
						else if (REG.m_ofs == address(pData) && REG.m_siz == pData->OpSize())
						{
							uint8_t t0(pData->type()->typeSimple()->optype());
							uint8_t t1(t0);
							uint8_t t2(pSelf->OpType());
							if (AgreeTypes(t1, t2) && t1 != t0)
							{
								TypePtr iType(GetStockType((OpType_t)t1));
								DcCleaner_t<> DC(*this);
								DC.ClearType(pData);
								SetType(pData, iType);
							}
						}
						else if (REG.m_siz > pData->OpSize())
						{
							TypePtr iType(GetStockType((OpType_t)REG.m_siz));
							SetType(pData, iType);
							//TYPE _t T;
							//T.Setup(REG.m_siz);
							//pData->setTy pe(T.toTypeRef());
						}
					}
				}
				AttachLocalToOp(LocalRef(pOpIn), pSelf);// , 0);
			}
			else
				AddLocalRef(LocalRef(pOpIn), pSelf);
			return 1;
		}
	}

/*	if (!bForce)
	if (pOwnerFunc->IsDecompiled())
	{
		FieldPtr  pData = GetAttachData();
		if (pData)
		{
			pData->AddXRef(this);
			return 1;
		}
	}*/

	FieldPtr  pData = 0;
	//if (!pSelf->IsRhsOp())
	{
#if(0)
		if (!bDecompiled && IsLocalOp(pSelf))
		{
			AttachToLocal0(pSelf);
			//TraceLocalsType(pSelf, pData);
/*			if (pData)
			{
				assert(pData->type());
				TypePtr pType(TraceLocalsType(pSelf));
				if (pType)
				{
					TypesTracer_t tt(*findTypeMgr(pType));
					if (tt.ReconcileTypes(pData->type(), pType))
						pData->setTy pe(pType);
				}
			}*/
			return 0;
		}
		else
#endif
		{
			AttachToLocal(pSelf);
			//TraceLocalsType(pSelf, pData);
/*			if (pData)
			{
				assert(pData->type());
				TypePtr pType(TraceLocalsType(pSelf));
				if (pType)
				{
					TypesTracer_t tt(*findTypeMgr(pType));
					if (tt.ReconcileTypes(pData->type(), pType))
						pData->SetType(pType);
				}
			}
*/			return 0;
		}
	}

	if (!pData)
		return 0;

	if (ProtoInfo_t::IsFuncStatusFinished(&mrFuncDefRef))
		return 0;

	uint8_t id, sz;
	if (REG.CheckReg(pSelf->OpC(), sz, id))
	{
		SetType(pData, findOpType((OpType_t)REG.m_siz));
		assert(address(pData) == REG.m_ofs);
	}
	else
	{
		STOP
	}

	return 1;
}

int LocalsTracer_t::CheckLocals0(HOP pSelf)
{
	assert(IsPrimeOp(pSelf));
CHECK(OpNo(pSelf) == 22)
STOP

	CheckLocal0(pSelf);

	for (OpList_t::Iterator i(pSelf->argsIt()); i; ++i)
	{
		CheckLocal0(i.data());
	}

	if (IsCall(pSelf))
	{
		for (XOpList_t::Iterator i(pSelf->m_xout); i; ++i)
		{
			CheckLocal0(i.data());
		}
	}

	return 1;
}

/*int LocalsTracer_t::attachToData(HOP pSelf, FieldPtr pData)
{
	FieldPtr pFieldRef(LocalRef(pSelf));
	if (pFieldRef)
	{
		if (!pFieldRef->IsD ata())
			return 0;//can not
		if (pFieldRef == pData)
			return -1;//already
	}

	if (pData->Is Global())
		return 0;

	DetachMLoc(pSelf);
	int disp = pSelf->Offset0() - (int)address(pData);
	AttachLocalToOp(pData, pSelf, disp);
	return 1;
}

int LocalsTracer_t::AttachToData(HOP pSelf, FieldPtr pData)
{
	assert(pData);

	attachToData(pSelf, pData);

	for (XOpList_t::Iterator i(pSelf->m_xout); i; i++)
	{
		HOP pOpOut(i.data());
		if (IsLo cal(pOpOut))
			attachToData(pOpOut, pData);

		for (XOpList_t::Iterator j(pOpOut->m_xin); j; j++)
		{
			HOP pOpIn(j.data());
			if (pOpIn == pSelf)
				continue;

			attachToData(pOpIn, pData);
		}
	}

	return 1;
}*/

TypePtr	LocalsTracer_t::TraceLocalsType(HOP pSelf, FieldPtr pData)
{
	if (!pData)
		return 0;
	if (!pData->type())
		return 0;//unknown type?
	TypePtr pType(TraceLocalsType(pSelf));
	if (pType)
	{
		if (ReconcileTypes(pData->type(), pType))
		{
			if (pData->type() != pType)
			{
				DcCleaner_t<> DC(*this);
				DC.ClearType(pData);
				SetType(pData, pType);
			}
		}
	}
	return pType;
}

TypePtr	LocalsTracer_t::TraceLocalsType(HOP pOp)
{
CHECK(OpVA(pOp) == 0x1005e84)
//CHECKID(pOp, 0x178b)
//CHECK(pOp->No() == 86)
STOP

	ExprCacheEx_t aCache(PtrSize());
	EXPRptr_t expr(*this, pOp, 0, aCache);
	Out_t *pOut(expr.TracePtr(pOp, pathOpTracer()));

	expr.Simplify(pOut, pOp, pathOpTracer());

	CTypePtr pType(expr.toTypeRef(expr.TypOf(pOut->mpR)));
	if (pType && pOp->Size() != pType->size())
		pType = nullptr;
	return (TypePtr)pType;
}


int LocalsTracer_t::TraceTypeEx(FieldRef rSelf)
{
	if (!IsLocalReg(&rSelf) && !IsStackLocal(&rSelf))
		return 0;

//	TYPE _t T;
//	T.Setup(this);

	if (LocalRefs(&rSelf).empty())
		return 0;

	uint8_t t1 = rSelf.OpTypeZ();
	uint8_t t2 = MAKETYP_PTR(mrDC.PtrSize());
	if (AgreeTypes(t1, t2))
	{
		HOP pOp(LocalRefs(&rSelf).head()->data());
CHECK(OpNo(PrimeOp(pOp)) == 9)
STOP

		TraceLocalsType(pOp, &rSelf);
		/*int nPtr;
		TypePtr pStruc;
		if (pOp->GetBasePtrStruc(&pStruc, &nPtr))
		if (nPtr)
		{
			T.mpStruc = pStruc;
			T.mnPtr = 0;
			T.SetPtrLevel(nPtr);
		}*/
	}
	
//	if (T.opsz == OpSize())
//		ApplyType(T);

	return 1;
}


HPATH LocalsTracer_t::GetResideBlock(FieldPtr pLocal) const//downmost possible
{
	assert(IsLocal(pLocal));
	if (pLocal->owner()->typeFuncDef())//arg?
		return mrFuncDef.Body(); //PathHead();

	HPATH pBlock = HPATH();
	for (XOpList_t::Iterator i(LocalRefs(pLocal)); i; i++)
	{
		HOP pOp(i.data());
		if (IsVarOp(pOp))
			continue;//skip
		if (IsArgOp(pOp))
			return mrFuncDef.Body();

		HPATH pPath0 = PathOf(pOp);//starting path
		if (!pBlock)
			pBlock = pPath0;
		else
			pBlock = TreeCommonLowermost(pBlock, pPath0);//at least body is

		assert(pBlock);

		for (XOpList_t::Iterator j(pOp->m_xout); j && pBlock; j++)
		{
			HOP pOpOut = j.data();
			HPATH pPathOut = PathOf(pOpOut);
			if (pPathOut == pPath0)//the same path, ignore
				continue;

			//continue search
			pBlock = TreeCommonLowermost(pBlock, pPathOut);//at least body is
		}
	}

	return pBlock;
}

//check if all refs to the var are withing the given path
bool LocalsTracer_t::CheckUsageBoundaryPath(HOP pVarOp, HPATH pTargetPath) const
{
	//assert(pVarOp->IsVarOp());
	FieldPtr pLocal(LocalRef(pVarOp));
	assert(pLocal);

	for (XOpList_t::Iterator i(LocalRefs(pLocal)); i; i++)
	{
		HOP pOp(i.data());

		HPATH pPath(PathOf(pOp));
		for (; pPath; pPath = pPath->Parent())
			if (pPath == pTargetPath)
				break;
		if (!pPath)
			return false;

		for (XOpList_t::Iterator j(pOp->m_xout); j; j++)
		{
			pOp = j.data();
			pPath = PathOf(pOp);
			for (; pPath; pPath = pPath->Parent())
				if (pPath == pTargetPath)
					break;
			if (!pPath)
				return false;
		}
	}

	return true;
}

int Field_t::MoveUp()//for locals
{
assert(0);/*?	if (!IsLo cal0())
		return 0;

	if (!IsFirst())
	{
		FieldPtr pDataPr = (FieldPtr )Pr ev();
		if (pDataPr->StorageId() == StorageId())
		if (pDataPr->_key() == _key())
		if (!IsLocalArg(pDataPr))
			return Shift(-1);
	}

	HPATH pPath = Get OwnerPath();
	if (pPath)
	{
		while (pPath) 
		{
			HPATH pPath0 = pPath;
			pPath = (HPATH )pPath0->P rev();
			if (pPath)
				break;
			pPath = pPath0->Parent();
		}

		if (pPath)
		{
//			if (PathType(pParent) == BLK_IF)
//			{
//				if (pPath->IsLast())
//				if (!pPath->IsFirst())
//					pPathTo = (HPATH )pPath->Pr ev();
//			}

			//Unlink();
			pPath = pPath->GetLocal Path(this);
			pPath->AddL ocal0(this, StorageId(), _key());
			return 1;
		}
	}*/

	return 0;
}

int Field_t::MoveDown()//for locals
{
assert(0);/*?	if (!IsLo cal0())
		return 0;

	if (!IsLast())
	{
		FieldPtr pDataNx = (FieldPtr )Ne xt();
		if (pDataNx->StorageId() == StorageId())
		if (pDataNx->_key() == _key())
			return Shift(+1);
	}

	HPATH pPath = Get OwnerPath();
	while (1)
	{
		if (!pPath->IsFirst())
			break;
		if (!pPath->Parent())
			break;
		pPath = pPath->Parent();
	}

	if (pPath)
	{
		HPATH pPathTo = pPath;
		HPATH pPath0 = GetResi dePath();//pPathTo can't be lower than this

		while (1)
		{
			HPATH pPathCh = pPathTo->Childs();
			if (pPathCh && (pPath0 == pPathCh || pPath0->IsLowerThan2(pPathCh)))//higher or the same
			{//go down
				pPathTo = pPathCh;
			}
			else 
			{
				while (1)
				{
					HPATH pPathNx = (HPATH )pPathTo->Ne xt();
					if (pPathNx)//go up
					{
						pPathTo = pPathNx;
						break;
					}
					pPathTo = pPathTo->Parent();
					if (!pPathTo)
						break;
				}
			}

			if (!pPathTo)
				break;

			HPATH pPath2 = pPathTo->GetLocalsBlock();
			if (pPath2 != pPath)
			{
				if (!pPathTo->IsLowerThan(pPath0))//higher or the same
				{
					//Unlink();//?
					pPathTo = pPathTo->GetL ocalPath(this);
					if (pPathTo->AddLoc al0(this, StorageId(), _key(), true))//add from top
						return 1;
				}
				break;
			}
		}//while
	}*/

	return 0;
}

#if(0)
FieldPtr LocalsTracer_t::AddLocal3(HOP pSelf)
{
#if(0)
	if (!IsPrimeOp(pSelf))
		return 0;

	HPATH  pPath = pSelf->Path();

	SSID_t ssid = pSelf->SSID();
	if (ssid != SSID_LOCAL)//fixme
		if (ssid != SSID_CPUREG)
			if (ssid != SSID_FPUREG)
				if (ssid != SSID_AUXREG)
					return 0;

	bool bNew = false;
	FieldPtr  pData = pPath->AddLocal(ssid, pSelf->Offset0(), bNew);

	if (bNew)
		mrFunc.SetChangeInfo(FUI_LOCALS);

#else
	if (!IsLValue(pSelf))
		return 0;

	if (!LocalRef(pSelf))
		return 0;//?

	if (!IsLo cal(pSelf))
		if (!LocalRef(pSelf)->IsLo calEx())
		return 0;

	//check if there is any sence of doing this
	bool bSense = false;
	for (XFieldList_t::Iterator i(LocalRef(pSelf)->xr efs()); i; i++)
	{
		HOP pOp = i.data().op();
		if (pOp == pSelf)
			continue;

		bSense = true;
		for (XOpList_t::Iterator j(pOp->m_xout); j; j++)
		{
			HOP pOpOut = j.data();

			for (XOpList_t::Iterator k(pOpOut->m_xin); k; k++)
			{
				HOP pOpIn = k.data();
				if (pOpIn == pSelf)
				{
					bSense = false;
					break;
				}
			}
			if (!bSense)
				break;
		}
		if (bSense)
			break;
	}

	if (!bSense)
		return 0;

	//detach old data
	for (XOpList_t::Iterator i(pSelf->m_xout); i; i++)
	{
		HOP pOpOut(i.data());

		for (XOpList_t::Iterator j(pOpOut->m_xin); j; j++)
		{
			HOP pOpIn(j.data());
			DetachMLoc(pOpIn);
		}
	}

	FieldPtr pData = nullptr;

	//attach the new one
	for (XOpList_t::Iterator i(pSelf->m_xout); i; i++)
	{
		HOP pOpOut(i.data());

		for (XOpList_t::Iterator j(pOpOut->m_xin); j; j++)
		{
			HOP pOpIn(j.data());
			CheckLocal0(pOpIn, true);
			if (!pData)
				pData = LocalRef(pOpIn);
			assert(pData && pData == LocalRef(pOpIn));
		}
	}

//	HPATH pPath = pData->GetR esidePath();
//	if (pPath)
//		pPath->AddLocal(pData);//here relink is possible!
#endif
	return pData;
}
#endif

int LocalsTracer_t::CheckLocalsPos(HOP pSelf)
{
	assert(0);/*
	FieldPtr pFieldRef(fieldRef(pSelf));
	if (!pFieldRef || !pFieldRef->IsD ata() || !pFieldRef->IsLo cal0())
		return 0;

	HPATH  pPath = pFieldRef->Get OwnerPath();
	HPATH  pPathR = GetResi dePath(*pFieldRef);

	if (pPath->IsLowerThan(pPathR))
	{
		FieldPtr  pData = pFieldRef;
		pPathR = GetLoca lPath(pPathR, pData);
		AddLo cal0(pPathR, pData);
		return 1;
	}*/

	return 0;
}

STAGE_STATUS_e LocalsTracer_t::CheckLocals()
{
	if (!(mrFuncDef.m_dc & FUI_LOCALS))
		return STAGE_STATUS_e::SKIPPED;//skipped

	HPATH pBody = mrFuncDef.Body();
	if (pBody)
	{
		LocalsTracer_t &an(*this);

		//is this loop needed?
		/*for (OpList_t::Iterator i(mrFuncDef.entryOps()); i; i++)
		{
			an.CheckLocal0(i.data());//Ex();
		}*/

		for (PathTree_t::LeafIterator i(mrFuncDef.pathTree().tree()); i; i++)
		{
			HPATH pPath(i.data());
			for (PathOpList_t::Iterator i(pPath->ops()); i; i++)
			{
				an.CheckLocals0(PRIME(i.data()));
			}
		}

		if (HasLocalVars())
		if (!ProtoInfo_t::IsFuncStatusFinished(&mrFuncDefRef))
		{
			//__checkLocalsTypes(*pBody);

#if(1)
			FieldMap& m(LocalVarsMap());
			for (FieldMapIt i(m.begin()); i != m.end(); ++i)
			{
				FieldPtr pLocal(VALUE(i));
				assert(IsStackLocal(pLocal) || IsLocalReg(pLocal));
				TraceTypeEx(*pLocal);
			}
#else

			for (PathTree_t::Iterator i(mrFuncDef.pathTree().tree()); i; ++i)
			{
				HPATH hPath(*i);
				//if (rPath.m.hasLocals())
				//for (FieldMapCIt j(rPath.m.locals().begin()); j != rPath.m.locals().end(); j++)
				for (PathOpList_t::Iterator j(hPath->ops()); j; j++)
				{
					HOP pOp(PRIME(j.data()));
					if (IsVarOp(pOp))
					{
						FieldPtr pData(LocalRef(pOp));
						if (pData)//?
						{
//CHECKID(pData, 7852)
//STOP
							assert(IsStackLocal(pData) || IsLocalReg(pData));
							TraceTypeEx(*pData);
						}
					}
				}
			}
#endif
		}
		//		Body()->__typeLocalsEx();
	}

	mrFuncDef.m_dc &= ~FUI_LOCALS;
	return STAGE_STATUS_e::DONE;
}


#if(0)
FieldPtr LocalsTracer_t::AddLocal2(HOP pVarOp, int ssid, int nOffset)//Path_t* pSelf, bool &bNewLocal)
{
//	bNewLocal = false;


/*	FieldMapIt itPr = locals().end();
	FieldMapIt it = locals().begin();
	while (it != locals().end())
	{
		FieldPtr  pLocal = VALUE(it);

		int ssid2 = pLocal->StorageId();
		assert(ssid2);
		if (ssid2 > ssid)
			break;
		if (ssid2 == ssid)
		{
			int offs2 = address(pLocal);
			if (offs2 > nOffset)
				break;
			if (offs2 == nOffset)
				return pLocal;
		}
		itPr = it++;
	}*/

	//ADDR addr(loc2key(ssid, nOffset));
	/*if (ssid == SSID_FPUREG)
	{
		assert(-0xFF < addr && addr < 0xFF);
		addr = 0xFF + addr;
		addr |= ((ssid&0x7F) << 24);
	}
	else if (ssid != SSID_LOCAL)
	{
		assert(addr >= 0 && addr < 0xFFFFFF);
		addr |= ((ssid&0x7F) << 24);
	}
	else if (addr >= 0)
	{
		assert(addr < 0xFFFFFF);
	}*/
	
	//bNewLocal = true;
	FieldPtr  pLocal = mrMemMgr.NewF ield();//NEW!!!
	pLocal->setOffset(nOffset);
	pLocal->m_opc = (uint8_t)ssid;

	assert(!pLocal->owner());
	AddLocalVar(pLocal, pVarOp, false);

	return pLocal;
}
#endif

/*
int Path_t::__typeLocalsEx()
{
	for (HPATH pPath = m_pChilds;
	pPath;
	pPath = pPath->Ne xt())
	{
		pPath->__typeLocalsEx();
	}

	FieldPtr pData = m_pLocals; 
	while (pData)
	{
		FieldPtr pDataNx = (FieldPtr )pData->Ne xt();
		pData->TraceTypeEx();
//		pData->CheckOwnerBlock();//here relink is possible!
		pData = pDataNx;
	}

	return 1;
}
*/


HOP LocalsTracer_t::GetLocalTopOp(FieldPtr pLocal) const
{
	assert(IsLocal(pLocal));

	HOP hOpRef = HOP();
	for (XOpList_t::Iterator i(LocalRefs(pLocal)); i; i++)
	{
		HOP pOp(PrimeOp(i.data()));
		if (IsVarOp(pOp))
			continue;
		if (!hOpRef)
		{
			hOpRef = pOp;
			continue;
		}
		if (TreeIsLowerThan(PathOf(hOpRef), PathOf(pOp)))
			hOpRef = pOp;
	}

	return hOpRef;
}

HOP FuncInfo_t::TakeOp(HPATH hSelf, HOP pOp) const
{
	assert(IsPrimeOp(pOp));
	hSelf->takeOp(INSPTR(pOp));
	pOp->ins().mpPath = HPATH();
	return pOp;
}

bool LocalsTracer_t::RelocateVarOp(HOP hVarOp, HOP pOpAt, HPATH pPath)
{
	if (pOpAt && PathOf(pOpAt) != pPath)
		pOpAt = HOP();

	assert(!hVarOp->isLinked());
	
	if (!pOpAt)
	{
		if (pPath->Type() == BLK_JMPIF)
		{
			HOP pOpLast(GetLastOp(pPath));
			assert(IsCondJump(pOpLast));
			LinkOpBefore(pPath, hVarOp, pOpLast);
		}
		else
			LinkOpTail(pPath, hVarOp);//link at the bottom of the path
	}
	else
	{
		assert(PathOf(pOpAt) == pPath);
		LinkOpBefore(pPath, hVarOp, pOpAt);
	}

	//hOpVar->setPStackIn(pOpAt->pstackIn());
	//hOpVar->setFStackIn(FpuIn(pOpAt));
	return true;
}

int LocalsTracer_t::ImplantLocals()
{
	//return 0;
#if(NEW_LOCAL_VARS)
	if (!HasLocalVars())
		return 1;
#else
	if (!iGetLocalsTop())
		return 1;
#endif

	FieldMap& m(LocalVarsMap());

	for (FieldMapIt i(m.begin()); i != m.end(); ++i)
	{
		FieldPtr pLocal(VALUE(i));
/*		HOP hVarOp = GetVarOp(pLocal);
		if (!hVarOp)
			continue;*/

		//InsertVarOpAt(hVarOp, pSelf);

		HPATH pPath(GetResideBlock(pLocal));
		//if (!pPath) continue;
		assert(pPath);
		pPath = GetLocalImplantPath(pPath);
		assert(pPath);

		HOP hOpAt(GetLocalTopOp(pLocal));

		HOP hVarOp(CreateVarOp(pLocal));
		if (!hVarOp)
			continue;

		//if (PathOf(hVarOp) != pPath)
		{
			
			//TakeOp(PathOf(hVarOp), hVarOp);
			RelocateVarOp(hVarOp, hOpAt, pPath);
		}

		if (hVarOp->isRoot())
			Bind(hVarOp);
	}

	return 1;
}

/*int LocalsTracer_t::__recheckLocals(HPATH pSelf)
{
	int nCount = 0;

	if (pSelf->m.has Locals())
	for (FieldMapCIt it = pSelf->m.locals().begin(); it != pSelf->m.locals().end(); )
	{
		FieldMapCIt itNx = it;
		itNx++;

		FieldPtr  pLocal = VALUE(it);
		if (pLocal->IsUnrefed())
		{
			if (IsLocalArg(pLocal))
			{
				if ( pLocal->m_nFlags & FLD_NCONF )
				{
					TakeL ocal(pLocal);
					delete pLocal;
					nCount++;
					if (!pSelf->m.hasL ocals())
						break;
					it = itNx;
					continue;
				}
			}
			else if (pLocal->isRet())
			{
			}
			else 
			{
				TakeL ocal(pLocal);
				if (!pSelf->m.hasLo cals())
					break;
				delete pLocal;
				nCount++;
				it = itNx;
				continue;
			}

		}

		CheckOwnerBlock(pSelf, pLocal);
		it = itNx;
	}

	for (PathTree_t::ChildrenIterator i(*pSelf); i; i++)
	{
		nCount += __recheckLocals(i.data());
	}

	return nCount;
}*/

/*HPATH  LocalsTracer_t::CheckOwnerBlock(HPATH pSelf, FieldPtr  pData)
{
	if (!pData->IsLoc al0())
		return 0;

	HPATH  pPathR = GetLocalsBlock(GetRes idePath(*pData, true));
	HPATH  pPathB = pData->GetOwnerPath();//->GetLocalsBlock();

	if (pPathR == pPathB)
		return 0;

	if (pPathB->IsLowerThan(pPathR))
	{
		pPathR = GetLoca lPath(pPathR, pData);
		AddL ocal0(pPathR, pData);
		return pData->GetOwner Path();
	}
	
	return nullptr;
}*/

int LocalsTracer_t::DetachLocals(HPATH pSelf)
{
	HPATH pPathBody = mrFuncDef.Body();
	if (pPathBody == pSelf)
		return -1;

	assert(0);/*?
	while (pSelf->m.hasLo cals())
	{
		FieldPtr  pData = VALUE(pSelf->m.locals().begin());
		mrDC.arrLocals.Add(pData);
		HPATH  pPath = GetLoc alPath(pPathBody, pData);
		AddLo cal0(pPath, pData);
	}*/

	return 1;
}

HPATH  LocalsTracer_t::GetLocalPath(HPATH pSelf, FieldPtr pData) const
{
	HPATH pPathTo = GetLocalsBlock(pSelf);
	if (pPathTo != pSelf)
		return GetLocalPath(pPathTo, pData);

	//?	if (IsLocalArg(pData))
	//?		return GetRoot();
	return TreeTerminalFirst(pSelf);

/*	HPATH pPath(pSelf->TreeTerminalFirst());
	if (pPath->Type() == BLK_ENTER)
	{
		pPath = pPath->Nex t();
		pPath = pPath->TreeTerminalFirst();
		assert(pPath->Type() != BLK_EXIT);
	}
	return pPath;*/
}

HPATH  LocalsTracer_t::GetLocalImplantPath(CHPATH pSelf) const
{
	HPATH pPathTo(GetLocalsBlock(pSelf));
	if (pPathTo != pSelf)
		return GetLocalImplantPath(pPathTo);
	return TreeTerminalFirst(pSelf);
}

HPATH LocalsTracer_t::GetLocalsBlock(CHPATH pSelf) const
{
	//adjust position for local
	HPATH pPath = pSelf;
	while (1)
	{
		if (!TreeIsFirst(pPath))
			break;
		if (!pPath->Parent())
			break;
		pPath = pPath->Parent();
	}

	if (pPath->Type() == BLK_LOOPENDLESS)
		return GetLocalsBlock(TreePrev(pPath));

	HPATH pPathParent = pPath->Parent();
	if (pPathParent)
	{
		if (PathType(pPath) == BLK_SWITCH)
			return GetLocalsBlock(pPathParent);

		if (pPath->Type() == BLK_IFELSE)
			if (CheckElseIf(pPath))
				return GetLocalsBlock(pPathParent);

		if (PathType(pPathParent) == BLK_OR || PathType(pPathParent) == BLK_AND)
			return GetLocalsBlock(pPathParent);
	}

	return pPath;
}

int LocalsTracer_t::CheckStackDiff(HOP pSelf)
{
	assert(IsPrimeOp(pSelf));
	/////////////////////////////////
	//explicit stack ptr modification
	if (!IsStackPtrOp(pSelf))
		return 1;

CHECK(OpNo(pSelf) == 3)
STOP

	pSelf->setPStackDiff(0);

	if (ActionOf(pSelf) == ACTN_MOV)
	{
//		SetTraceInfo(TRACE_MARK, false);//clear
//		HOP pOp = (HOP)Get(156);
		int32_t stackout;
		if (__traceLocal(pSelf->arg1(), &stackout))
		{
			pSelf->setPStackDiff(stackout - pSelf->pstackIn());
			return 1;
		}
	}
	else if (ActionOf(pSelf) == ACTN_SUB)
	{
		HOP pOp1(pSelf->arg1());
		if (IsStackPtrOp(pOp1))
		{
			HOP pOp2(pSelf->arg2());
			if (pOp2->IsScalar())
			{
				assert(abs(OpDisp(pOp2)) < 0x80000000);
				pSelf->addPStackDiff(-OpDisp(pOp2));
				return 1;
			}
		}
	}
	else if (ActionOf(pSelf) == ACTN_ADD)
	{
		int stackDiff(0);
		HOP pOp1(pSelf->arg1());
		HOP pOp2(pSelf->arg2());
		if (!IsStackPtrOp(pOp1))
		{
			int32_t stackout;
			if (__traceLocal(pOp1, &stackout))//assume this traced into stack ptr?
				pSelf->setPStackDiff(stackout - pSelf->pstackIn());
		}
		if (pOp2->IsScalar())
		{
			assert(abs(OpDisp(pOp2)) < 0x80000000);
			pSelf->addPStackDiff(+OpDisp(pOp2));
			return 1;
		}
	}
	else if (ActionOf(pSelf) == ACTN_AND)
	{
		HOP pOp1(pSelf->arg1());
		if (IsStackPtrOp(pOp1))
		{
			HOP pOp2(pSelf->arg2());
			if (pOp2->IsScalar())
			{
				int32_t pstackVal(pSelf->pstackIn());
				//if (__traceLocal(pOp1, &pstackVal))
				{
					pstackVal &= pOp2->m_disp;
					pSelf->setPStackDiff(pstackVal - pSelf->pstackIn());
					return 1;
				}
			}
		}
	}

	pSelf->ins().opnd_BAD_STACKOUT = 1;
	return 0;
}

