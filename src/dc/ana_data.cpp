#include "ana_data.h"
#include "prefix.h"

#include "shared/defs.h"
#include "shared/link.h"
#include "shared/misc.h"
#include "shared/action.h"
#include "front/front_IA.h"
#include "db/mem.h"
#include "db/obj.h"
#include "db/field.h"
#include "db/type_struc.h"
#include "path.h"
#include "op.h"
#include "ana_local.h"
#include "ana_init.h"
#include "ana_main.h"
#include "reg.h"
#include "expr_ptr.h"
#include "clean_ex.h"

AnlzXDeps_t::AnlzXDeps_t(const FuncTracer_t &r, bool b)
	: FuncTracer_t(r),
	mbRangedOnly(false),
	mbLocalsOnly(b),
	mpRedumpCache(nullptr)
{
}

AnlzXDeps_t::AnlzXDeps_t(const FuncInfo_t &ri, bool b, PathOpTracer_t &tr)
	: FuncTracer_t(ri, tr),
	mbRangedOnly(false),
	mbLocalsOnly(b),
	mpRedumpCache(nullptr)
{
}

STAGE_STATUS_e AnlzXDeps_t::run()
{
//	if ( !mpFunc )
	//	return 0;

	return TouchXDeps();
}

STAGE_STATUS_e AnlzXDeps_t::TouchXDeps()
{
	if (!testChangeInfo(FUI_XDEPSINS))
		return STAGE_STATUS_e::SKIPPED;//not dirty

	TRACE1("%s: Checking x-deps...\n", OwnerFuncName().c_str());

	PathTouchXDeps();

	clearChangeInfo(FUI_XDEPSINS);

	setChangeInfo(FUI_SAVREGS);//?
	return STAGE_STATUS_e::DONE;
}

void AnlzXDeps_t::PathTouchXDeps()
{
#if(1)
	ExprCacheEx_t	aExprCache(PtrSize());
	for (PathTree_t::LeafIterator i(mrFuncDef.pathTree().tree()); i; i++)
	{
		HPATH pPath(i.data());
		if (!pPath->ops().empty())
			ProgressInfo("XDEPS", PRIME(pPath->ops().front()));
		for (PathOpList_t::Iterator j(pPath->ops()); j; j++)
		{
//CHECK(pOp->No() == 11)
//STOP
			//OpTracer_t optr(*this);
			HOP pOp(PRIME(j.data()));
			AnlzXDepsIn_t an(*this, pOp, aExprCache);
			an.setRedumpCache(mpRedumpCache);
			//an.setOpTracer(&opTracer());
			an.setRangedOnly(mbRangedOnly);
			an.setLocalsOnly(mbLocalsOnly);
			an.TraceXDepsIn();
			//!pOp->TraceXDepsIn(mbRangedOnly);
//if (pOp->No() == 436)
//pOp->TraceXOutExtra();
			if (!mbLocalsOnly)
			{
				InitialTracer_t an2(*this);
				an2.FixDisplacements(pOp);
			}
		}
	}
#else
	for (PathTree_t::LeafIterator i(mrFuncDef.pathTree().tree()); i; i++)
	{
		HPATH pPath(i.data());
		for (OpList_t::Iterator j(pPath->ops()); j; j++)
		{
			j.data()->TraceXOutExtra();
		}
	}
#endif
}

//////////////////////////////////////////////////


TraceInfo_t::TraceInfo_t(const FuncInfo_t &r, HOP _pOp, HOP _pOpBase)
	: FuncInfo_t(r)
{
	pOpFrom = HOP();
//	pBlock = 0;
	if (_pOpBase)
	{
		pOpBase = _pOpBase;
		lXIn = _pOpBase->m_xin;
		_pOpBase->setXIn(nullptr);
	}
	else
	{
		pOpBase = HOP();
		lXIn = _pOp->m_xin;
		_pOp->setXIn(nullptr);
	}
}

/*
int Op_t::OrderXOuts()
{
	if (!IsCode())
		return 0;

	HPATH  pPath = Path();
	assert(pPath);

	for (XRef_t * pXOut = XOut();
	pXOut;
	pXOut = pXOut->Ne xt())
	{
		bool bSeen = false;
		HOP  pOpOut = pXOut->pOp;
		for (XRef_t * pXIn = pOpOut->XIn(); 
		pXIn;
		pXIn = pXIn->Nex t())
		{
			HOP  pOpIn = pXIn->pOp;
			if (pOpIn == this)
			{
				bSeen = true;
				continue;
			}
			if (pOpIn->Path() != pPath)
				break;//this will not be unrooted
			int dpos = prim eOp()->No() - pOpIn->PrimeOp()->No();
			if (dpos > 0 && bSeen)//other is higher
				continue;
			else if (dpos < 0 && !bSeen)//this is higher
				continue;

			break;
		}
	}

	return 1;
}*/

AnlzXDepsIn_t::AnlzXDepsIn_t(const FuncTracer_t &r, HOP pOp, ExprCacheEx_t &rExprCache)
	: FuncTracer_t(r),
	mpOp(pOp),
	mbRangedOnly(false),
	mbLocalsOnly(false),
	mrExprCache(rExprCache),
	mpRedumpCache(nullptr)
{
}

AnlzXDepsIn_t::AnlzXDepsIn_t(const FuncInfo_t &fi, PathOpTracer_t &tr, HOP pOp, ExprCacheEx_t &rExprCache)
	: FuncTracer_t(fi, tr),
	mpOp(pOp),
	mbRangedOnly(false),
	mbLocalsOnly(false),
	mrExprCache(rExprCache),
	mpRedumpCache(nullptr)
{
}

int AnlzXDepsIn_t::EstablishLink(TraceInfo_t &ti, HOP pOpIn)
{
CHECK(OpNo(PrimeOp(mpOp)) == 27)
STOP

	//check if there is one already among ti.pXIn list
	for (XOpList_t::Iterator i(ti.lXIn); i; ++i)
	{
		XOpLink_t *pLink(i.self());
		if (pLink->data() == pOpIn)
		{
			ti.lXIn.take(i);
			if (ti.pOpBase)
				InsertXDep(ti.pOpBase->m_xin, pLink->data(), pLink);
			else
				InsertXDep(mpOp->m_xin, pLink->data(), pLink);
//			pXIn->pOp->OrderXOuts();
			return -1;
		}
	}

	if (ti.pOpBase)
		MakeUDLink(ti.pOpBase, pOpIn);
	else
		MakeUDLink(mpOp, pOpIn);
//	pOpIn->OrderXOuts();
	return 1;
}

static bool Overlap(CHOP pOp, SSID_t ssid, const REG_t &r0)
{
	if (pOp->SSID() == ssid)
	{
		REG_t r(pOp->Offset0(), pOp->OpSize());
		if (r.intersects(r0))
			return true;
	}
	return false;
}

HOP AnlzXDepsIn_t::RegisterCallOutOp(CHOP hSelf, CHOP hOpRef)
{
	assert(IsCall(hSelf));
	assert(!hSelf->isHidden());

	assert(hOpRef->SSID() == SSID_CPUREG || hOpRef->SSID() == SSID_FPUREG || hOpRef->isCPUSW());

CHECK(OpNo(hSelf) == 323)
STOP

	REG_t r0(REGex(hOpRef));
	HOP pOp = HOP();

	if (mpRedumpCache)
	{
		auto &m(mpRedumpCache->callout);
		for (auto i(m.begin()); i != m.end(); ++i)
		{
			if (i->second == hSelf)//of this call op?
			{
				HOP hOutOp(i->first);
CHECKID(hOutOp, 19812)
STOP
				if (::Overlap(hOutOp, hOpRef->SSID(), r0))
				{
					FieldPtr pLocal(mpRedumpCache->takeCallout(i));//`i` invalidated
					//assert(IsVarOp(hVarOp));//may be dangling yet
					//FieldPtr pLocal(LocalRef(hVarOp));
					assert(pLocal);
					AddLocalRef(pLocal, hOutOp);
					mrFuncDef.mCallRets.push_back(hOutOp);
					hOutOp->setInsPtr(hSelf->insPtr());
					MakeUDLink(hOutOp, hSelf);

#if(0)
					LocalsTracer_t an(*this, pathOpTracer());
					HOP pVarOp = an.CreateVarOp(pLocal);
					an.InsertVarOpAt(pVarOp, hOutOp);
#endif
				
					pOp = hOutOp;
					break;
				}
			}
		}
	}

	if (!pOp)
	{
		for (XOpList_t::Iterator i(hSelf->m_xout); i; ++i)
		{
			if (::Overlap(i.data(), hOpRef->SSID(), r0))
			{
				pOp = i.data();
				break;
			}
		}
	}

	int nDiff = 0;
	//	if (hOpRef->OpC() == OPC_FPUREG)
	//		nDiff = -FpuOut();
	uint8_t optyp(hOpRef->IsIndirectB() ? MAKETYP_PTR(PtrSize()) : hOpRef->OpType());

	if (!pOp)
	{
		pOp = NewOp();
		mrFuncDef.mCallRets.push_back(pOp);
		pOp->Setup3(hOpRef->SSID(), optyp, r0.m_ofs + nDiff);
		pOp->setInsPtr(hSelf->insPtr());
		MakeUDLink(pOp, hSelf);
		return pOp;
	}

	REG_t r(pOp->Offset0(), pOp->OpSize());
	r0.MergeWith(r);
	if (r0.m_siz != r.m_siz)
		pOp->Setup3(pOp->OpC(), optyp, r0.m_ofs + nDiff);
	return pOp;
}

int AnlzXDepsIn_t::MatchXDepIn(CHOP hOpRef, TraceInfo_t &ti, uint32_t &mask)//1:end up scan
{
	HOP pOp(hOpRef);
	assert(IsPrimeOp(pOp));//only output ops examined

//	if (pBlock)//attempt to change tracing branch
//		break;
//	if ( pBlock && (ti.pOp->Path() != pBlock))
//		break;

//	if (pOp->IsDead())
//		return 0;//continue

#if(1)
	if (!IsAnalized(pOp))//?
		if (Storage(mpOp->SSID()).isStacked())
			return 1;//skip not analized path
#endif

	op_tracer_cell_t &tr(opTracer().cell(mpOp));
	if (!(tr.trace & TRACE_PASSFUNC))
	{
		if (mpOp->IsIndirect())
			if (!mpOp->IsIndirectB())
				if (IsCall(pOp))
					return 1;
	}

	int ret = MatchXDep(pOp, mask);
	if (!ret)//establish linkage
		return 0;//continue scan

	if (ret == -1)
	{
		mpOp->m_nFlags |= OPND_XINEX;
		return 1;
	}

	if (ret == 2 || ret == 3 || ret ==4)
	{
//		if (ti.pBlock)
//			return 1;
		assert(IsCall(pOp));
		pOp = RegisterCallOutOp(pOp, mpOp);//external op
	}
//			else if (pBlock)
//			{
//				if ( (pOp->Action() != ACTN_MOV)
//					|| !pOp->arg1()->IsScalar() 
//					|| (pOp->arg1()->i32 != 0) )
//					break;
//			}
	
	if (EstablishLink(ti, pOp) == 1)
	{
		assert(ti.FuncDefPtr() == FuncDefPtr());
		//if (!(ti.FuncDefRef().flags() & FDEF_REANLZ))
		if (ProtoInfo_t::IsFuncStatusFinished(ti.FuncDefPtr()))
			if (IsCode(pOp))
			{
				SetRoot(PrimeOp(pOp), 0);
//?				ti.SetChangeInfo(FUI_ROOTS);
			}
	}

/*	if (pOp->isPtr())
	{
*//*		AgreeTypes(m_optyp, pOp->m_optyp);
		if (!pr imeOp()->IsCall())
			if (pri meOp()->SSID() != OPC_ CPUSW)
				if (prim eOp()->OpC() != OPC_FPUSW)
					if (!ti.pOpBase)
*//*						AgreeTypes(m_optyp, prim eOp()->m_optyp);
	}*/

	if (!mask)
		return 1;//stop scanning

//	ti.pBlock = pOp->Path();

	//if (DuplicateXIns(pOp))
	//	return 1;

	return 0;
}

class RR_t : public r_t
{
	RR_t(){}
public:
	RR_t(uint8_t _ssid, RegMaskType mask)
	{
		ssid = _ssid;
		for (ofs = 0; (mask && !(mask & 1)); ofs++)
			mask >>= 1;
		for (siz = 0; (mask && (mask & 1)); siz++)
			mask >>= 1;
	}
};

void AnlzXDepsIn_t::__traceXDepsUp(TraceInfo_t &ti, uint32_t mask, CHPATH hPathRef)
{
	HPATH pPath(hPathRef);
	HOP pOpFrom = PrimeOp(mpOp);
	for (;;)
	{
		HOP pOp = HOP();
		if (!pPath)//starting from this
		{
//			if (!pOpFrom->IsFirst())
				pOp = PrevPrimeEx(pOpFrom);
			pPath = PathOf(pOpFrom);
		}
		else
		{
			path_trace_cell_t &trp(pathTracer().cell(pPath));
			if (trp._traced || pPath->Type() == BLK_ENTER)
				return;
			pOp = GetLastOp(pPath);
			assert(trp._traced < 3);
			trp._traced++;
		}

#ifdef _DEBUG
		int _pathNo(PathNo(pPath));
#endif

		while (IsMineOp(pPath, pOp))//the same path
		{
			if (!IsVarOp(pOp))
			{
				if (pOp == mpOp)
					if (ti.pOpBase)//modified base!
						return;
CHECK(OpNo(PrimeOp(mpOp)) == 51 && OpNo(PrimeOp(pOp)) == 51)
STOP
				if (MatchXDepIn(pOp, ti, mask))
				{
					//CHECK(OpNo(PrimeOp(mpOp)) == 57)
					//STOP
					return;//stop tracing
				}
				if (pOp == pOpFrom)
					return;
			}

			Path_t::OpEltPtr pElt(PrevOp(pOp));
			if (pElt)
				pOp = PRIME(pElt);
			else
				pOp = HOP();
		}

		//get previous path
		HPATH pPathPr = TreePrevEx(pPath);
		if (!pPathPr || pPathPr->Type() == BLK_ENTER)
		{
#if(1)//A WTF TO PREVENT APPEARENCE OF FALSE ARGS DUE TO LOOSE DATA INFLOW DEPENDENCIES

CHECKID(mpOp, 0x1323)
STOP
			const STORAGE_t &ST(Storage(mpOp->SSID()));
			if (ST.isRegister())
			{
				REG_t R;
				if (!R.fromMask(mask, mpOp->OpOffsU()) || !mrDC.isRegValid(mpOp->SSID(), R))
				{
					PrintWarning() << "Suspicious input dependency (" << RegToString(mpOp->SSID(), R, true) << ") at L" << StrNo(mpOp) << " on argument rejected" << std::endl;
					return;
				}
			}
#endif

			HOP pOpIn = RegisterEntryOp(mpOp);
			if (pOpIn)
				EstablishLink(ti, pOpIn);
			return;//so this is a first leaf of the tree
		}

//		if (ti.pBlock)
//			return;//can't change tracing path?

		for (XOpList_t::Iterator i(PathOpRefs(pPath)); i; ++i)
		{
			HOP pOpRef(i.data());
			if (IsDataOp(pOpRef))
			{
				HOP pIOpRef(pOpRef);
				pOpRef = GetSwitchOp(pIOpRef);
			}

			if (!pOpRef->isHidden())
			{
				HPATH pPathJ(PathOf(pOpRef));
				__traceXDepsUp(ti, mask, pPathJ);
			}
		}

		switch (pPathPr->Type())
		{
		case BLK_JMP:
		case BLK_JMPSWITCH:
//		case BLK_RET:
			return;
		}
		
		pPath = pPathPr;
	}
}

unsigned AnlzXDepsIn_t::bInside = 0;

int AnlzXDepsIn_t::ScanXDeps1(CHOP pFrom, CHOP pOpBase)
{
	assert(IsPrimeOp(pFrom));
//	pFrom->SetTraceInfo(TRACE_MARK, false);//clear

CHECK(OpNo(pFrom) == 547)
STOP

//	if (!mask)
//		return 1;
//	pFrom->CheckTraceInfo(TRACE_MARK, false);

	//assert(!bInside);
	if (bInside)
	{
#ifdef _DEBUG
		PrintWarning() << "data flow analysis failed for L" << StrNo(pFrom) << std::endl;
#endif
		return 0;
	}

	bInside++;


	TraceInfo_t ti(*this, mpOp, pOpBase);
	{//RAII-block
		//PathTracer_t aTracer;
		PathTracer_t &aTracer(pathTracer());
		aTracer.reset(*this);
		//setPathTracer(&aTracer);

		uint32_t mask = BitMask(mpOp, true);
		if (!mask)
		{
			//assert(0);
			bInside--;
			return 1;
		}

		__traceXDepsUp(ti, mask, HPATH());

#if(0)
		if (pOpBase)
		{		//trace segment part of ptr
			assert(pOpBase == this);
			m_opc = OPC_SEGREG;
			m_optyp = OPSZ_WORD;
			OpSeg_t seg = (OpSeg_t)OpSeg();
			assert(seg);
			mOffs = (seg - 1) * OPSZ_WORD;
			mask = BitMask();
			__traceXDepsUp(ti, mask);//assure tracer is cleared
		}
#endif

		bInside--;
	}

	while (!ti.lXIn.empty())//get rid of unclaimed DU-links
	{
		XOpLink_t *pLink(ti.lXIn.take_front());
		HOP pOpDef = pLink->data();//ti.lXIn.head()->data();
		//ti.pXIn->UnlinkAndDestroy(&ti.pXIn);
		HOP pOpTo = ((pOpBase)?(pOpBase):(mpOp));
		assert(pOpTo == pLink[1].data());
		if (!ReleaseXDepOut(pOpDef, pOpTo))
		{
			assert(0);
		}

		memMgrEx().Delete(pLink);//ti.lXIn.take(ti.lXIn.head()));

		if (IsCallOutOp(pOpDef) && !pOpDef->XOut())
		{
			/*if (LocalRef(pOpDef))
			{
				DcInfo_t DI(mrDC, memMgrGlob());//delete from the global mem 
				DcCleaner_t<> DC(DI);
				DC.ClearType(pField);
				//SetType(pField, nullptr);
				DC.destroyUnrefedTypes();//?DC);
			}*/
			//UnmakeUDLink(pOpDef, PrimeOp(pOpDef));
			//memMgrEx().Delete(pOpDef);
			mrFuncDef.mCallRets.Unlink(pOpDef);

			FuncCleaner_t FD(*this);
			FD.DestroyOp(pOpDef);
			FD.Cleanup();
			FD.CleanupFinal();
		}

		//if (!(mrFuncDefRef.flags() & FDEF_REANLZ))
		if (ProtoInfo_t::IsFuncStatusFinished(&mrFuncDefRef))
			for (XOpList_t::Iterator i(mpOp->m_xin); i; i++)
		{
			HOP  pOpIn = i.data();
			if (IsCode(pOpIn))
			{
				SetRoot(PrimeOp(pOpIn), 0);
				SetChangeInfo(FUI_ROOTS);
			}
		}
	}

	return 1;
}


int AnlzXDepsIn_t::ScanXDeps0(CHOP pFrom, CHOP pOpBase, bool bRangedOnly)
{
	assert(mpOp);

	if (mpOp->IsScalar())
		return 0;
	if (IsAddr(mpOp))
		return 0;

#if(1)
	if (bRangedOnly)
		if (!StorageOf(mpOp).isStacked())
			return 0;
#endif
	if (mbLocalsOnly && mpOp->SSID() != SSID_LOCAL)
		return 0;

//CHECK(OpNo(PrimeOp(mpOp)) == 3209)
CHECKID(mpOp, 0x1377)
STOP

	if (!IsStackPtrOp(mpOp))
	{
		if (!ScanXDeps1(pFrom, pOpBase))//? || mpOp->m_xin.empty())
			return 0;
	}

	assert(!pOpBase || pOpBase == mpOp);
	if (!pOpBase)
	{
		LocalsTracer_t LT(*this);
		LT.MakeLocal(mpOp);
	}

//	int n = pXIn->Count();
	for (XOpList_t::Iterator i(mpOp->m_xin); i; i++)
	{
		HOP pOp(i.data());
		if (!IsCode(pOp))
			continue;

		HOP pOpRoot(PrimeOp(pOp));
		InitialTracer_t an(*this);
		an.SetShown(pOpRoot);
		if (ProtoInfo_t::IsFuncStatusFinished(&mrFuncDefRef))
		{
//			if (CanBeUnrooted(pOpRoot))
//?				pOpRoot->SetRoot(false);
//?			pFunc->SetChangeInfo(FUI_ROOTS);
			if (IsCallOutOp(pOp))
				SetChangeInfo(FUI_CALLOUTS);
		}
	}

	if (!ExpandSpecialAND(mpOp))
		ExpandSpecialOR(mpOp);
	return 1;
}

int AnlzXDepsIn_t::ScanXDepsBase(bool bMakeLocal)
{
	assert(IsCode(mpOp));
	HOP pOpFrom = PrimeOp(mpOp);

//CHECK(pOpFrom->No() == 1162)
//STOP

/*	FieldPtr  pLoc = 0;
//	if (bMakeLocal)
	if (mrFuncDef.m_dc & FUI_TOPS)
		RevertLocal(&pLoc);
*/

	OpSeg_t opseg = (OpSeg_t)mpOp->OpSeg();

	uint8_t opc = mpOp->m_opc;
	uint8_t optyp = mpOp->m_optyp;
	int8_t opid = mpOp->mOffs;
	int disp = mpOp->m_disp;
#if(0)//1 more!
	mpOp->m_opc &= SSID_MASK;
	mpOp->m_optyp = MAKETYP_PTR(mrDC.PtrSize());
	mpOp->m_disp = 0;
	//mpOp->m_seg = 0;
#endif
	ScanXDeps0(pOpFrom, mpOp);

	if (mpOp->XIn())
	if ((mpOp->SSID() != SSID_AUXREG) || (mpOp->mOffs != OFS(R_W)))
	if (opseg && opseg != (OPC_INDIRECT >> 4))
	{
		//check segment reg
//		assert(opseg);
		HOP pOpSeg = NewOp();
		pOpSeg->Setup3(OPC_SEGREG, OPSZ_WORD, (opseg - 1) * OPSZ_WORD);
		pOpSeg->setInsPtr(InsPtrPrime(mpOp));
		AnlzXDepsIn_t an(*this, pOpSeg, mrExprCache);
		//an.setOpTracer(&opTracer());
		an.ScanXDeps0(pOpFrom);
		if (!pOpSeg->XIn())
		{
			FuncCleaner_t h(*this);
			h.DisconnectOp(pOpSeg, false);
			//ClearOp(pOpSeg);
			//pOpSeg->setInsPtr(nullptr);
			//Delete(pOpSeg);
			h.DestroyOp(pOpSeg);
		}
		else
		{
			//es:[bx+d] => w32=@(es,bx),[w32+d]
			HOP pOpPtr = NewPrimeOp();

			pOpPtr->Setup3(OPC_AUXREG, MAKETYP_PTR(mrDC.PtrSizeEx()), OFS(R_W));

			HOP pOpOfs = NewOp();
			InitFrom(pOpOfs, mpOp);
			RedirectXIns(mpOp, pOpOfs);
			mpOp->Setup4(OPC_INDIRECT|OPC_AUXREG, optyp, OFS(R_W), disp);

			AddOpArg(pOpPtr, pOpSeg);
			AddOpArg(pOpPtr, pOpOfs);

			if (pOpFrom != mpOp && EqualTo(pOpFrom, mpOp))
			{
				Delete(pOpPtr);
			}
			else
			{
				LinkOpBefore(pOpPtr, pOpFrom);
				pOpPtr->setAction(ACTN_UNITE);
				//SetRoot(pOpPtr, 1); 
				InitialTracer_t an(*this);
				an.Analize(pOpPtr, PrevPrimeEx(pOpPtr));
				SetChangeInfo(FUI_NUMBERS);
			}
			goto $exit;
			//return 1;
		}
	}
		
#if(0)
	mpOp->m_opc = opc;
	mpOp->mOffs = opid;
	mpOp->m_optyp = optyp;
	mpOp->m_disp = disp;
#endif
	//mpOp->SetOpSeg(opseg);
	if (!mpOp->XIn() && !(IsStackPtrOp(mpOp) || IsStackPtrB(mpOp)))
		return 0;

$exit:
//	if (bMakeLocal)
/*	if (Make Local())
		if (mrFuncDef.m_dc & FUI_TOPS);
*/		

#if(0)
	LocalsTracer_t an(*this);
	an.MakeLocal(mpOp);
#endif
	return 1;
}

void AnlzXDepsIn_t::TraceXDepsIn()
{
	assert(mpOp);
//?	assert(mpOp->IsCode());
	if (IsArgOp(mpOp) || IsRetOp(mpOp))
		return;

CHECK(OpNo(PrimeOp(mpOp)) == 547)
STOP
//CHECK(mrFunc.parent()->CompName("_D2R"))
//STOP
	

	if (!IsAnalized(PrimeOp(mpOp)))
		return;

	if (IsPrimeOp(mpOp))
	{
		if (!mbRangedOnly && mpOp->IsIndirectB())
			ScanXDepsBase(true);//find xdeps for its base

		if (IsGoto(mpOp) || IsCall(mpOp))
		{
			if (!mpOp->IsIndirectB())
				ScanXDeps0(mpOp, HOP(), mbRangedOnly);
		}

		for (OpList_t::Iterator i(mpOp->argsIt()); i; i++)
		{
			HOP pOp(i.data());
			bool bWasLocal(pOp->OpC() == OPC_LOCAL);
			AnlzXDepsIn_t an(*this, pOp, mrExprCache);
			an.setRedumpCache(mpRedumpCache);
			//an.setOpTracer(&opTracer());
			an.setRangedOnly(false);//mbRangedOnly);
			an.setLocalsOnly(mbLocalsOnly);
			an.TraceXDepsIn();
			if (!bWasLocal && pOp->OpC() == OPC_LOCAL)
			{
				assert(!pOp->XIn());
				an.TraceXDepsIn();//opc changed - trace xdeps
			}
			//pOpIn->TraceXDepsIn(bRelOnly);
		}
	}
	else
	{
		if (!mbRangedOnly && mpOp->IsIndirectB())
			ScanXDepsBase(true);//find xdeps for its base
		else
		{
			if (IsLocalOp(mpOp))
				opTracer().cell(mpOp).trace |= TRACE_PASSFUNC;
			ScanXDeps0(PrimeOp(mpOp), HOP(), mbRangedOnly);
		}
	}
}

////////////////////////////////////////////////

int AnlzXDepsIn_t::MatchXDepOut(HOP pOp, uint32_t &mask)
{
	int res = 0;
	if (pOp->IsIndirect())
	{
		HOP op(NewTmpOp());
		SetOpVA(op, OpVA(PrimeOp(pOp)));
		op->Setup3(pOp->OpC()&0xf, MAKETYP_PTR(mrDC.PtrSize()), pOp->OpOffsU());
		op->setInsPtr(InsPtrPrime(pOp));
		AnlzXDepsIn_t anlzxdepsin(*this, op, mrExprCache);
		res = anlzxdepsin.MatchXDep(mpOp, mask);
		if (res == 1)
			MakeUDLink(pOp, mpOp);
		Delete(op);
	}
	else 
	{
		AnlzXDepsIn_t anlzxdepsin(*this, pOp, mrExprCache);
		res = anlzxdepsin.MatchXDep(mpOp, mask);
		if (res == 1)
			MakeUDLink(pOp, mpOp);
	}

	if (res == 0)
		return 0;
	if (res == -1)
		mpOp->m_nFlags |= OPND_XOUTEX;

	return 1;
}

int AnlzXDepsIn_t::ScanXDepsOut()
{
	assert(IsPrimeOp(mpOp));

	uint32_t mask = BitMask(mpOp);//?
	HOP pOp = mpOp;
	while (1)
	{
//		if (pOp->IsLast())
//			break;

		pOp = NextPrimeEx(pOp);
		if (!pOp)
			break;

		op_tracer_cell_t &tr(opTracer().cell(pOp));
		if (tr.trace & TRACE_MARK)
			break;//already done

		tr.trace |= TRACE_MARK;

		if (IsGoto(pOp))
		{
//			if (!pOp->MatchXDep(this, mask))
//				assert(false);
//			MakeUDLink(pOp, mOp);
			MatchXDepOut(pOp, mask);
			return 1;
		}
		else
		{
			//look up through all xins
			for (OpList_t::Iterator i(pOp->argsIt()); i; i++)
				MatchXDepOut(i.data(), mask);
		}
	}

	return 1;
}

int AnlzXDepsIn_t::TraceXDepsOut()
{
	assert(IsPrimeOp(mpOp));
//	assert(OpC() == OPC_CPUREG);
//	assert(Action() == ACTN_MOV);

	opTracer().reset(*this);//clear
	ScanXDepsOut();

	return 1;
}

///////////////////////////////

int AnlzXDepsIn_t::MatchXDepOut2(HOP pOp, uint32_t mask0)
{
	if (mpOp->IsScalar())
		return 0;
	if (IsAddr(mpOp))
	{
		/*if (mpOp->Is Lo cal())
		if (mpOp->SSID() == pOp->SSID())
		if (mpOp->m_pData && pOp->m_pData)
		if (mpOp->m_pData->Overlap(pOp->m_pData))
		{
			mpOp->m_nFlags |= OPND_XOUTEX;
			return 1;
		}*/
		return 0;
	}

	int res = 0;
	if (mpOp->IsIndirectB())
	{
		if (!IsLValue(mpOp))
		{
			res = __tracePtrToLocal(pOp);
			if (res == 1)
				pOp->m_nFlags |= OPND_XOUTEX;
			return res;
		}

#if(0)
		res = op.MatchXDep(pOp, mask0);
//		if (res == 1)
//			MakeUDLink(pOp);
		return 0;
#endif
	}
	else
	{
		uint32_t mask = mask0;//BitMask();
		int d = pOp->Offset0() - mpOp->Offset0();
		mask <<= d;

		res = MatchXDep(pOp, mask);
//		if (res == 1)
//			MakeUDLink(pOp);
	}

	if (!res)
		return 0;
	if (res == -1)
		mpOp->m_nFlags |= OPND_XOUTEX;
	return 1;
}

int AnlzXDepsIn_t::MatchXDepExtra(HOP pOp, uint32_t mask)
{
	assert(IsPrimeOp(pOp));
CHECK(OpNo(pOp) == 100)
STOP

//	if (pOp->IsDead())
//		return 0;

	if (mpOp->OpC() == OPC_FPUREG)
	{
		assert(0);
		uint8_t ofs = 0;//?mpOp->FPURegID();
		if (ofs < 0)//?pOp->FpuTop())
			return 1;
	}

	for (OpList_t::Iterator i(pOp->argsIt()); i; i++)
	{
		AnlzXDepsIn_t anlzxdepsin(*this, i.data(), mrExprCache);
		anlzxdepsin.MatchXDepOut2(mpOp, mask);
	}
	
	if (pOp->IsIndirectB())//indirect l-values
	{
		AnlzXDepsIn_t anlzxdepsin(*this, pOp, mrExprCache);
		anlzxdepsin.MatchXDepOut2(mpOp, mask);
	}

	//check for l-value overwrite
	int res = MatchXDep(pOp, mask);
	if (!res)//no overwrite
		return 0;
	if (res == -1)//extra xout, but no link
		mpOp->m_nFlags |= OPND_XOUTEX;
	mask = 0;
	return 1;
}

void AnlzXDepsIn_t::__traceXOutExtra(CHPATH hPath0, uint32_t &mask)
{
CHECK(OpNo(mpOp) == 10)
STOP
	HPATH pPath(hPath0);
	HOP pOpRoot = PrimeOp(mpOp);
	HOP pOpNx = HOP();
	if (!pPath)
	{
		pPath = PathOf(pOpRoot);//do not mark start path
		pOpNx = NextPrimeEx(pOpRoot);
	}
	else
	{
		path_trace_cell_t &trp(pathTracer().cell(pPath));
		if (trp._traced)
			return;
		assert(trp._traced < 3);
		trp._traced++;
		pOpNx = GetFirstOp(pPath);
	}

	if (!pOpNx)//LAST!
	{
		return;
	}

	while (IsMineOp(pPath, pOpNx))
	{
		if (MatchXDepExtra(pOpNx, mask))
			return;
		if (pOpNx == PrimeOp(mpOp))
			return;
		pOpNx = NextPrime(pOpNx);
	}

	HPATH pPathNx = HPATH();
	if (PathType(pPath) == BLK_JMP)
	{
		pPathNx = GetGotoPath(pPath);
	}
	else if (PathType(pPath) == BLK_JMPSWITCH)//switch
	{
		HPATH pJumpTablePath(GetJumpTablePath(pPath));
		if (pJumpTablePath)
		{
			for (PathOpList_t::Iterator i(PathOps(pJumpTablePath)); i; i++)
			{
				pPathNx = PathRef(PRIME(i.data()));
				__traceXOutExtra(pPathNx, mask);
			}
		}
		return;
	}
	else if (PathType(pPath) == BLK_JMPIF)
	{
		pPathNx = GetGotoPath(pPath);
		if (!pPathNx)
			return;
		__traceXOutExtra(pPathNx, mask);
		pPathNx = TreeNextEx(pPath);
	}
	else if (PathType(pPath) == BLK_EXIT)//BLK_RET)
	{
		return;
	}
	else
		pPathNx = TreeNextEx(pPath);

	//assert(pPathNx);
	if (!pPathNx)
		return;
	__traceXOutExtra(pPathNx, mask);
}

int AnlzXDepsIn_t::TraceXOutExtra()
{
CHECK(OpNo(mpOp) == 13)
STOP

	if (mpOp->IsIndirectB())
		return 1;

	//if (XOut())
	//	return 1;
	//if (IS CALL(Action()))
	//	return 1;
//	if (ISJMPIF(Action()))
//		return 1;
//	if (Action() == ACTN_JMP)
//		return 1;
//	if (IsRetOp())
//		return 1;

	//RAII-block
	//PathTracer_t aTracer;
	PathTracer_t &aTracer(pathTracer());
	aTracer.reset(*this);
	//setPathTracer(&aTracer);

	uint32_t mask;
	if (mpOp->isCPUSW())
	{
		mask = InsRefPrime(mpOp).mEFlagsTested;
//		else if (OpC() == OPC_CPUF)
//			mask = CPUFID2MASK(OpId());
	}
	else
		mask = ~((~0) << mpOp->OpSize());//1mask

	mpOp->m_nFlags &= ~OPND_XOUTEX;
	__traceXOutExtra(HPATH(), mask);

	return 1;
}

int AnlzXDepsIn_t::CheckExtraXOut()
{
	if (IsPrimeOp(mpOp))
	if (!mpOp->XOut())
		if (IsLocalOp(mpOp) && !IsAddr(mpOp))
			TraceXOutExtra();
	return 1;
}

#define RETURN(arg) { tr.trace &= ~TRACE_OVERLAPPED; return (arg); }
int AnlzXDepsIn_t::__tracePtrToLocal(HOP pOpLoc)
{
	op_tracer_cell_t &tr(opTracer().cell(mpOp));
	if (tr.trace & TRACE_OVERLAPPED)
		return 0;
	tr.trace |= TRACE_OVERLAPPED;

//	assert(isPtr());
	if (!IsCodeOp(mpOp))
		RETURN(0);

	if (IsPrimeOp(mpOp))
	{
		switch (ActionOf(mpOp))
		{
		case ACTN_MOV:
		case ACTN_ADD:
		case ACTN_SUB:
			break;
		default:
			RETURN(0);
		}
	}

	if (IsAddr(mpOp) && IsLocalOp(mpOp))//already ptr!
	{
		//LocalsTracer_t an(*this);

		int ofs1 = mpOp->Offset0() + CalcDispl(mpOp);//m_pData->Offset();
		FieldPtr pFieldRef1(LocalRef(mpOp));
		int sz1 = (pFieldRef1) ? (pFieldRef1->size()) : (OPSZ_BYTE);//(OpSize());

		int ofs2 = pOpLoc->Offset0();
		FieldPtr pFieldRef2(LocalRef(pOpLoc));
		int sz2 = (pFieldRef2) ? (pFieldRef2->size()) : (pOpLoc->OpSize());

//		if (m_pData == pOpLoc->m_pData)//FIXME:realy overlapped???
		if (checkoverlap(ofs1, sz1, ofs2, sz2))
			RETURN(1);
	}

	if (IsPrimeOp(mpOp))
	{
		for (OpList_t::Iterator i(mpOp->argsIt()); i; i++)
		{
			//			if ( pOpIn->isPtr() )
			AnlzXDepsIn_t an(*this, i.data(), mrExprCache);
			//an.setOpTracer(&opTracer());
			if (an.__tracePtrToLocal(pOpLoc))
				RETURN(1);
		}
	}
	else
	{
		for (XOpList_t::Iterator i(mpOp->m_xin); i; i++)
		{
			HOP pOp = i.data();
//			if ( pOp->isPtr() )
			AnlzXDepsIn_t an(*this, pOp, mrExprCache);
			//an.setOpTracer(&opTracer());
			if (an.__tracePtrToLocal(pOpLoc))
				RETURN(1);
		}
	}

	RETURN(0);
}
#undef RETURN

int AnlzXDepsIn_t::CheckIfArgsOverlapLocal(HOP pSelf, HOP pOpLoc)
{
	if (!IsCall(pSelf))
		return 0;

	assert(pOpLoc);
	assert(IsLocalOp(pOpLoc));
//if (!pOpLoc->IsL ocal())
//return 0;
	//TODO: check if this op & pOpLoc has the same owner func

//	CheckTraceInfo(TRACE_OVERLAPPED, false);

	for (OpList_t::Iterator i(pSelf->argsIt()); i; i++)
	{
//CHECK(No() == 829)
//STOP
//		if (pArg->IsLo cal())
		//if ( pArg->isPtr() )
		{
			AnlzXDepsIn_t an(*this, i.data(), mrExprCache);
			//an.setOpTracer(&opTracer());
			if (an.__tracePtrToLocal(pOpLoc))
				return 1;
		}
	}

	return 0;
}

int AnlzXDepsIn_t::MatchXDep(CHOP pOp, uint32_t &mask)
{
	assert(!IsAddr(mpOp));

	REG_t rq(pOp->Offset0() + CalcDispl(pOp), pOp->OpSize());

	switch (mpOp->SSID())
	{
	//case OPC_IMM:
	case SSID_GLOBAL:
		break;

	case SSID_LOCAL:
//		if (!IsLo cal())//locals only!!!
//			break;

		if (IsCall(pOp))
		{
			if (CheckIfArgsOverlapLocal(pOp, mpOp))
			{
				return -1;//there is no need to establish link
				mask = 0;
			}
			return 0;
		}
		
		if (IsLocalOp(pOp))
		{
			if (uint32_t m = FuncInfo_s::Overlap(REG(mpOp), rq))
			if (m & mask)
			{
				mask &= ~m;
				return 1;
			}
		}
		break;

	case SSID_CPUREG:
		{
			if (IsStackPtrOp(mpOp)
				&& (pOp->pstackDiff() != 0))
			{
				mask = 0;
				return 1;
			}

			if (pOp == mpOp)
			{
				//tracing an indirect call; looped back to a callsite location!
				//it is possible the register used to call the function is about to be modified by function.
				//not sure if such dependencies are usefull; for now - just drop the branch
				//mask = 0;
				return 0;
			}

			if (IsCall(pOp))
			{
				GlobPtr iGlob(GetCalleeFuncDef(pOp));
				if (!iGlob)//?
				{
					iGlob = GetCalleeFuncDefEx(pOp, mrExprCache, pathOpTracer()); 
				}

				RegMaskType fmask;
				if (!iGlob)
				{
					fmask = GetSpoiledRegsMask(nullptr, false);
				}
				else
				{
					if (iGlob == FuncDefPtr())//recursion!
					{
						//the callee is also a caller.
						//The 'saved regs' analysis have not been run yet, so a proto may contain spoilt regs which are to be eliminated
						//But the ret regs yet must be considered;
						fmask = GetSpoiledRegsMask(iGlob, true);
					}
					else
					{
						fmask = GetSpoiledRegsMask(iGlob, false);
					}
				}

				REG_t r0(REGex(mpOp));
				FlagMaskType rmask(FlagMaskType(mask) << r0.m_ofs);
				FlagMaskType m(rmask & fmask);
				if (m)
				{
					m >>= r0.m_ofs;
					mask &= ~m;
					return 2;
				}
			}
			else if (pOp->OpC() == OPC_CPUREG)
			{
				uint32_t m(FuncInfo_s::Overlap(REGex(mpOp), rq));
				if (m & mask)
				{
					mask &= ~m;
					return 1;
				}
			}
		}
		break;

	case SSID_FPUREG://only input ops!!!
		if (pOp->OpC() == OPC_FPUREG)
		{
			uint32_t m(FuncInfo_s::Overlap(REGex(mpOp), rq));
			if (m & mask)
			{
				mask &= ~m;
				return 1;
			}
		}
		else if (IsCall(pOp))
		{
			//FuncDef_t *pfDef = GetCalleeFuncDef(pOp);
			ProtoProfile_t si;
			GetArgProfileFromCall(si, pOp);
			int _fpu_max = FpuIn(pOp) + si.fpuin;//pfDef->FpuIn();
			int _fpu_min = FpuIn(pOp) + pOp->fstackDiff();// pOp->FpuIn() + m_nFPUOut;// pfDef->getFStackPurge();
			int _fpuid = mpOp->OpOffs() / FR_SLOT_SIZE;

			if ((_fpu_min <= _fpuid) && (_fpuid < _fpu_max))
			{
				mask = 0;
				return 3;
			}
		}
		break;

	case SSID_FPUCW:
		if (pOp->OpC() == OPC_FPUCW)
		{
			mask = 0;
			return 1;
		}
		break;

	case SSID_FPUSW:
		if (pOp->ins().mFFlagsAffected != 0)
		{
			mask = 0;//FIXME
			return 1;
		}
		break;

	case SSID_CPUSW:
		if (IsCall(pOp))
		{
			GlobPtr ifDef(GetCalleeFuncDef(pOp));
			uint32_t modified = mask & uint32_t(ifDef->typeFuncDef()->GetSpoiltFlags());
			if (modified)
			{
				mask = mask ^ modified;
				return 4;
			}
			break;
		}
		else
		{
			uint32_t modified = mask & uint32_t(pOp->ins().mEFlagsModified);
			if (modified)
			{
				mask = mask ^ modified;
				return 1;
			}
		}
		break;

	case SSID_AUXREG:
		if (pOp->OpC() == OPC_AUXREG)
		{
			uint32_t m(FuncInfo_s::Overlap(REGex(mpOp), rq));
			if (m & mask)
			{
				mask &= ~m;
				return 1;
			}
		}
		break;

	case SSID_SEGREG:
		if (pOp->OpC() == OPC_SEGREG)
		{
			uint32_t m(FuncInfo_s::Overlap(REGex(mpOp), rq));
			if (m & mask)
			{
				mask &= ~m;
				return 1;
			}
		}
		break;

	default:
		assert(false);
	}

	return 0;
}

void AnlzXDepsIn_t::CheckIrreducibleExpressions()
{
	if (ActionOf(mpOp) != ACTN_SUB)
		return;

	for (XOpList_t::Iterator i(mpOp->m_xout); i; i++)
	{
		HOP pOpOut(i.data());
		if (ActionOf(PrimeOp(pOpOut)) == ACTN_ABOVE)
		{
			for (OpList_t::Iterator i(mpOp->argsIt()); i; i++)
			{
				HOP pOpArg(i.data());
				for (XOpList_t::Iterator i(pOpArg->m_xin); i; i++)
				{
					HOP pOpIn(i.data());
					PrimeOp(pOpIn)->m_nFlags |= OPND_NO_REDUCE;
				}
			}
			break;
		}
	}
}



//ax = cx
//al = al & 0F0h => ax = ax & 0FFF0h
//var:2 = ax<0,1> => var:2 = ax<1>
int AnlzXDepsIn_t::ExpandSpecialAND(HOP hSelf)
{
	HOP pOpRoot = PrimeOp(hSelf);
	if (ActionOf(pOpRoot) == ACTN_AND)
	if (pOpRoot->args().Head() == hSelf)
	if (pOpRoot->args().check_count(2) == 0)
	if (pOpRoot->args()[1]->IsScalar())
	if (hSelf->OpC() == OPC_CPUREG)
	if (hSelf->XIn())
	{
		HOP pOpIn = hSelf->XIn()->data();
		assert(pOpIn->OpC() == hSelf->OpC());
		
		if (hSelf->OpSize() < (pOpIn->OpSize()))
		{
			if (hSelf->Offset0() == pOpIn->Offset0())
			{
				if (!pOpRoot->isCPUSW())
				{
					int mask = ~0 << (hSelf->OpSize() << 3);
					pOpRoot->SetOpSize(pOpIn->OpSize());
					hSelf->SetOpSize(pOpIn->OpSize());
					pOpRoot->arg2()->SetOpSize(pOpIn->OpSize());
					pOpRoot->arg2()->m_disp |= mask;	
				}
				else//bits test - simple zero expand
				{
					pOpRoot->SetOpSize(pOpIn->OpSize());
					hSelf->SetOpSize(pOpIn->OpSize());
					pOpRoot->arg2()->SetOpSize(pOpIn->OpSize());
				}
			}
		}
	}

	return 0;
}

//ax = cx
//al = al | 0Fh => ax = ax | 000Fh
int AnlzXDepsIn_t::ExpandSpecialOR(HOP hSelf)
{
	HOP pOpRoot = PrimeOp(hSelf);
	if (ActionOf(pOpRoot) == ACTN_OR)
	if (pOpRoot->args().Head() == hSelf)
	if (pOpRoot->args().check_count(2) == 0)
	if (pOpRoot->args()[1]->IsScalar())
	if (hSelf->OpC() == OPC_CPUREG)
	if (hSelf->XIn())
	{
		HOP pOpIn = hSelf->XIn()->data();
		assert(pOpIn->OpC() == hSelf->OpC());
		if (hSelf->OpSize() < (pOpIn->OpSize()))
		{
			if (hSelf->Offset0() == pOpIn->Offset0())
			{
//				int mask = ~0 << (m_opsz << 3);
				pOpRoot->SetOpSize(pOpIn->OpSize());
				hSelf->SetOpSize(pOpIn->OpSize());
				pOpRoot->arg2()->SetOpSize(pOpIn->OpSize());
//				pOpRoot->arg2()->i32 |= mask;	
			}
		}
	}

	return 0;
}



