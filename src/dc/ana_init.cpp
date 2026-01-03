#include "anal_init.h"
#include "anal_data.h"
#include "anal_expr.h"
#include "anal_branch.h"
#include "anal_local.h"
#include "stubs.h"
#include "anal_ptr.h"
#include "db/info_types.h"
#include "expr_ptr.h"
#include "clean_ex.h"
#include "flow.h"
#include "cc.h"



///////////////////////////////////////////////////////
// InitialTracer_t

InitialTracer_t::InitialTracer_t(const FuncTracer_t &r)
	: FuncTracer_t(r)//,
	//mpCachedArgs(nullptr)
{
}

InitialTracer_t::InitialTracer_t(const FuncInfo_t &fi, PathOpTracer_t &tr)
	: FuncTracer_t(fi, tr)//,
	//mpCachedArgs(nullptr)
{
}

#define ON_EXIT(a)	if (a) fprintf(STDERR, "%d\n", OpNo(a));

int InitialTracer_t::Analize0(HPATH pPath, HOP pOpPr) const
{
	PathOpList_t::Iterator jOp(pPath->ops());
	//HOP pOp = GetFirstOp(pPath);
	if (pPath->isAnalized())
	{
		if (testChangeInfo(FUI_BASE^FUI_BASE0))
		if (jOp)
		{
			if (IsAnalized(PRIME(jOp.data())))//?
			if (!CheckAlreadyAnalized(PRIME(jOp.data()), pOpPr))
			{
				ON_EXIT(pOpPr);
				return 0;
			}
		}
		return 1;
	}

	pPath->setAnalized(true);

	if (!ProtoInfo_t::IsFuncStatusFinished(&mrFuncDefRef))
	{
		BranchTracer_t an(*this);
		an.AdjustGoto(pPath);
		an.AdjustGoto2(pPath);
	}

	while (jOp)
	{
		if (testChangeInfo(FUI_BASE^FUI_BASE0))
			if (Analize(PRIME(jOp.data()), pOpPr) < 0)//critical error
			{
				ON_EXIT(pOpPr);
				return 0;
			}
		pOpPr = PRIME(jOp.data());
		//pOp = pOp->N ext();
		jOp++;
	}

	HPATH pPathNx = HPATH();
	if (PathType(pPath) == BLK_JMP)
	{
		pPathNx = GetGotoPath(pPath);
	}
	else if (PathType(pPath) == BLK_JMPSWITCH)//switch
	{
		if (!AnalizeJumpTable(pPath, pOpPr))
		{
			ON_EXIT(pOpPr);
			return 0;
		}
		return 1;
	}
	else if (PathType(pPath) == BLK_JMPIF)
	{
		pPathNx = GetGotoPath(pPath);
		if (!Analize0(pPathNx, pOpPr))
		{
			ON_EXIT(pOpPr);
			return 0;
		}
		pPathNx = TreeNextEx(pPath);
	}
	else if (PathType(pPath) == BLK_EXIT)
	{
		return 1;
	}
	else
		pPathNx = TreeNextEx(pPath);

	if (!pPathNx)
	{
		fprintf(STDERR, "Internal Error: Return from function missed\n");
		ON_EXIT(pOpPr);
		return 0;
	}

	if (!Analize0(pPathNx, pOpPr))
	{
		ON_EXIT(pOpPr);
		return 0;
	}

	return 1;
}

int InitialTracer_t::AnalizeJumpTable(HPATH pSwitchPath, HOP pOpPr) const
{
	HPATH pDataPath(GetJumpTablePath(pSwitchPath));
	if (!pDataPath)
	{
		PrintError() << "Unrecognized jump table at " << OwnerFuncName() << "<" << PathNo(pSwitchPath) << ">" << std::endl;
		return 0;
	}

	if (PathOps(pDataPath).empty())
	{
		//			assert(false);
		PrintError() << "Malformed jump table" << std::endl;
		//!			pSwitchPath->GetLastOp()->Goto2Call();
		//			return 1;
	}
	for (PathOpList_t::Iterator i(PathOps(pDataPath)); i; i++)
	{
		HPATH pPathNx(PathRef(PRIME(i.data())));
		//assert(IsLab el(pLabel));
		//assert(IsMineLabel(pLabel));
		//HPATH pPathNx(Lab elPath(pLabel));
		if (!Analize0(pPathNx, pOpPr))
		{
			ON_EXIT(pOpPr);
			return 0;
		}
	}
	return 1;
}

/*int InitialTracer_t::SetString(FieldPtr pSelf, bool bSet, bool bUser)
{
	bool bOldState(pSelf->isStringOf());
	pSelf->setAttribute(ATTR_NULL);

	if (!bSet)//clear
	{
		if (!bOldState)
			return -1;//already
		SetPenetrated(pSelf, false);
		return 1;
	}

	//recheck
	if (!Fie ldOps(pSelf).empty())
		return 0;
	if (!CheckString(pSelf, bUser))
		return 0;
	pSelf->setAttribute(ATTR_ ASCII);
	if (bOldState)
		return -1;//already
	pSelf->SetInverted(false);
	return 1;
}*/

/*int InitialTracer_t::SetConst(FieldPtr pSelf, bool bSet) const
{
	bool bOldState = (pSelf->m_nFlags & FLD_CONST) != 0;

	if (!bSet)
	{
		if (!bOldState)
			return -1;//already
		pSelf->m_nFlags &= ~FLD_CONST;
		return 1;
	}

	if (!CheckReadOnly(pSelf))
	{
		pSelf->m_nFlags &= ~FLD_CONST;
		return 0;
	}

	pSelf->m_nFlags |= FLD_CONST;
	if (bOldState)
		return -1;//already
	return 1;
}*/

/*bool InitialTracer_t::IsJumpTable(FieldPtr pSelf)
{
	HOP pOp;
	HOP pOpGoto = GetSwitchOp(pSelf, &pOp);
	if (pOpGoto)
	{
		HOP pOpJTab = GetDataSwitch(pOpGoto, nullptr);
		if (LabelRef(pOpJTab) == pSelf)
			return true;
	}
	return false;
}*/

bool InitialTracer_t::CheckFunc(FieldPtr pSelf) const
{
	for (XOpList_t::Iterator i(LocalRefs(pSelf)); i; i++)
	{
		HOP op(i.data());
		if (!IsCode(op))
			continue;
		if (!IsCall(op))//anyway otherwise
			if (IsGoto(op) || IsCondJump(op))
				if (GetOwnerBody(op) == mrPathTree.body())//pOwnerFunc)
					continue;//dont convert label into func

		return true;
	}

	return false;
}

int InitialTracer_t::CheckReadOnly(FieldPtr pSelf) const
{
	assert(IsLocal(pSelf));
	//?AssureOp();
	for (XOpList_t::Iterator i(LocalRefs(pSelf)); i; i++)
	{
		HOP op(i.data());
		if (!IsCodeOp(op))
			continue;
		//?		pOp = pOp->CheckIndir();
		//?		if (!pOp)
		//?			return 0;
		if (IsLValue(op))
			return 0;
	}
	return 1;
}

int InitialTracer_t::Preanalize(HOP pSelf) const
{
	assert(IsPrimeOp(pSelf));

CHECK(OpNo(pSelf) == 323)
STOP
//fprintf(stdout, " * %d\n", No(pSelf));

/*	if (IsJointOp(pSelf))
	{
		FieldPtr pLabel(PathLabel(PathOf(pSelf)));
		if (CheckFunc(pLabel))
			pLabel->AssureObjType(OBJID_TYPE_PROC);
	}*/

//?	CheckCallPtr(pSelf);
	SetRoot(pSelf, 1);//reset some flags
	CheckCall(pSelf);
//	CheckStackDiff();
	CheckSpecial(pSelf);//check for special ops combinations
	CheckShift(pSelf);
	CheckReadOnly(pSelf);
//	CheckValid1();
//	CheckIndirGoto();
	return 1;
}

//returns: 0: OK;
//	<0:critical error, stage must abort;
//	>0:some non critical error code bit mask, stage may continue, but pipeline cleared;
int InitialTracer_t::Analize(HOP pSelf, CHOP pOpPr) const
{
	assert(IsPrimeOp(pSelf));
CHECK(OpNo(pSelf) == 58)
STOP

	bool bDecompiled = ProtoInfo_t::IsFuncStatusFinished(&mrFuncDefRef) != 0;

	int res(0);//no error
//	if (!bDecompiled)
//		AdjustGoto();

	int32_t stackin_new = 0;
	int8_t fpuin_new = 0;
	if (pOpPr)// && pOpPr != pSelf)//first
	{
		fpuin_new = FpuOut(pOpPr);
		if (pOpPr->ins().opnd_BAD_STACKOUT)
			pSelf->ins().opnd_BAD_STACKOUT = 1;
		else
			stackin_new = StackOut(pOpPr);
	}

	int32_t stackin_old = pSelf->setPStackIn(stackin_new);
	int8_t fpuin_old = pSelf->setFStackIn(fpuin_new);

	ConvertOPC(pSelf);

	if (bDecompiled)
	{
		if (stackin_new != stackin_old)
		{
			LocalsTracer_t an(*this);
			if (an.RevertLocals(pSelf))
				SetChangeInfo(FUI_XDEPSINS);
		}
	}

#if(0)
	if (pSelf->IsCall())
	{
		FuncDef_t *pfCallee(GetCalleeFuncDef(pSelf));
		AdjustCallEx(pSelf, *pfCallee, GetVarArgSize(pSelf));//if pf==0, that should be indirect call
		if (!pfCallee)
		{
			fprintf(STDERR, "Internal Error: nullptr function call at (%d)\n", pSelf->No());
			return -1;
		}
		//pSelf->SetSta ckDiff(pfCallee->Stac kOut());
		//pSelf->ins().setFStackDiff(pfCallee->getFStackPurge());//ok
		//pSelf->ins().mEFlagsModified = pfCallee->GetSpoiltFlags();
	}
#endif

	//explicit stack pointer change
	if (IsStackPtrOp(pSelf))
	{
		//OpTracer_t optr(*this);
		LocalsTracer_t an(*this);
		//an.setOpTracer(&opTracer());
		if (!an.CheckStackDiff(pSelf))
		{
			PrintError() << MsgPStackBadValue(pSelf) << std::endl;
			SetFuncStatus(FDEF_DC_ERROR);
			res |= 0x20;
			//?return -1;//critical error
		}
	}

	if (pSelf->fstackDiff())
	{
		if (pSelf->fstackDiff() > 0)//store !!!FIXME
		{
			assert(pSelf->fstackDiff() <= 8 * FTOP_STEP);
			if (FpuOut(pSelf) > 8 * FTOP_STEP)//underflow check
			{
				mrMain.setFlags(adcui::DUMP_FPUTOP);
				PrintError() << "FPU top underflow at L" << OpNo(pSelf) << std::endl;
				res |= 0x1;
			}
		}
	}

	////////////////////////////////////
	if (IsRetOp(pSelf) && !pSelf->ins().opnd_BAD_STACKOUT)
	{
		if (pSelf->pstackIn() != 0 && !ProtoInfo_t::IsFuncUserCall(FuncDefPtr()))
		{
			PrintError() << MsgPStackRetNotReset(pSelf) << std::endl;
			SetFuncStatus(FDEF_DC_ERROR);//TYP_FDEF_PSNRESET);
			res |= 0x2;
		}

		int psdiff(StackOut(pSelf));
		int psdiff_adj(psdiff);// -RetAddrSize());//minus sizeof ret addr
		if (mrFuncDef.getPStackPurge() == 0)
		//if (ProtoInfo_t::FuncStatus(&mrFuncDefRef) == 0)//never defined
		{//SetStackOut
			int sz(mrDC.stackAddrSize());
			int nStackOut((psdiff_adj / sz) * sz);
			assert(mrFuncDef.getPStackPurge() == 0);
			if (nStackOut != 0)//?mrFuncDef.getPStackPurge())
			{
				mrFuncDef.setPStackPurge(nStackOut);
				if (nStackOut < 0 && !ProtoInfo_t::IsFuncUserCall(FuncDefPtr()))
				{
					if (DockField())
					{
						fprintf(STDERR, "Warning: Function '%s' has negative program stack top change value\n", OwnerFuncName().c_str());
						SetFuncStatus(FDEF_DC_ERROR);//TYP_FDEF_PSNEG);
						res |= 0x4;//error
					}
				}
			}
		}
		else
		{
			//psdiff_adj -= RetAddrSize();//minus sizeof ret addr
			if (mrFuncDef.getPStackPurge() != psdiff_adj)
			{
				PrintError() << "Program stack top did not converge to S" << psdiff_adj << " at return point L" << StrNo(pSelf) << std::endl;
				SetFuncStatus(FDEF_DC_ERROR);//TYP_FDEF_PSNCONV);//let it be not that severe
				res |= 0x8;//return 0;
			}
		}

		//setup func fpuout
		int nFPUStackCleanup = FpuOut(pSelf)/* - pOwnerFuncDef->GetFPUIn()*/;
		if (mrFuncDef.getFStackPurge() == 0)
			SetFPUOut0(nFPUStackCleanup);
		else
		{
			if (mrFuncDef.getFStackPurge() != nFPUStackCleanup)
			{
				PrintError() << "FPU stack top did not converge to F" << nFPUStackCleanup << " at return point L" << StrNo(pSelf) << std::endl;
				SetFuncStatus(FDEF_DC_ERROR);//TYP_FDEF_FSNCONV);
				res |= 0x10;
				//assert(f == nFPUStackCleanup);
			}
		}
	}

//	if (!bDecompiled)
//	{
//		if (!res)
//			CheckFPUIn();
//	}

CHECK(OpNo(pSelf) == 1373)
STOP
	//CheckLocals();
	//CheckValid2();
//!	TraceXDepsIn();
#if(0)
	AnlzXD epsIn_t an(*this, pSelf);
	an.TraceXDepsIn();
#endif
//!	if (!bDecompiled)
//!		FixDisplacements();

	if (IsLValue(pSelf) && !pSelf->IsIndirectB() && !IsStackPtrOp(pSelf))
	{
		if (StorageFlags(pSelf) & SS_SPOILT)
		{
			AddToSpoiltList(pSelf);
		}
	}

	return res;
}

int InitialTracer_t::Goto2Call(HOP pSelf) const
{
	assert(IsGoto(pSelf));//fix later

	int retaddrsz = RetAddrSize();

	pSelf->setAction(ACTN_CALL);
	HOP pOpRet = NewPrimeOp();//add ret

	PathOf(pSelf)->insertOpAfter(pOpRet, pSelf);
	AttachToPath(pOpRet, PathOf(pSelf));

	//make ret
	pOpRet->setAction(ACTN_GOTO);
	pOpRet->Setup3(OPC_ADDRESS|OPC_GLOBAL, 0, 0);

	PathPtr pPathRet = PathHead();
	//FieldPtr pLabel = Ensur eLabel(pPathRet);
	AddPathRef(pPathRet, pOpRet);
	pOpRet->setPStackDiff(mrFuncDef.getPStackPurge());//? +retaddrsz);
	Preanalize(pOpRet);

	if (IsAddr(pSelf))
	{
		//?FieldPtr pLabel = LabelRef(pSelf);
		//?TypeProc_t *pFunc = (TypeProc_t *)pLabel->AssureObjType(OBJID_TYPE_PROC);
	}
	else
	{
//?		if (LocalRef(pSelf))
//?			SetThruConst(LocalRef(pSelf), false);
	}

	CheckCall(pSelf);
	return 1;
}

int InitialTracer_t::SplitAssignment(HOP pSelf) const
{
	assert(IsPrimeOp(pSelf));
	if (ActionOf(pSelf) != ACTN_MOV)
		return 0;

	if (pSelf->OpSize() != pSelf->arg1()->OpSize())
		return 0;
	if (pSelf->OpSize() == OPSZ_BYTE)
		return 0;
	if ((pSelf->OpType() & 0xF0) || (pSelf->arg1()->OpType() & 0xF0))
		return 0;

	int sz = pSelf->OpSize() >> 1;
	int dstack = pSelf->pstackDiff() / 2;
	int dfpu = pSelf->ins().FStackDiff() / 2;

	pSelf->SetOpSize(sz);
	pSelf->arg1()->SetOpSize(sz);
//	SetStackDiff( dstack);
//	ins().setFStackDiff(dfpu);

	HOP pOpHi = CloneOperation(pSelf, false);
	pOpHi->Displace(sz);
	pOpHi->arg1()->Displace(sz);
	LinkOpAfter(pOpHi, pSelf);
	pOpHi->setPStackDiff(0);
	pOpHi->ins().setFStackDiff(0);
	Analize(pOpHi, pSelf);

	ExprCacheEx_t aExprCache(PtrSize());
	AnlzXDepsIn_t anlzxdepsin(*this, pSelf, aExprCache);
	anlzxdepsin.TraceXDepsIn();
	//!TraceXDepsIn();

	//!pOpHi->TraceXDepsIn();
	AnlzXDepsIn_t an1(*this, pOpHi, aExprCache);
	an1.TraceXDepsIn();

	LocalsTracer_t an2(*this);
	an2.CheckLocals0(pOpHi);

	//rebuild xouts
	XOpList_t lXOuts(pSelf->m_xout);
	pSelf->m_xout = nullptr;
	for (XOpList_t::Iterator i(lXOuts); i; i++)
		ReleaseXDepIn(i.data(), pSelf);

	for (XOpList_t::Iterator i(lXOuts); i; i++)
	{
		AnlzXDepsIn_t an(*this, i.data(), aExprCache);
		an.TraceXDepsIn();
		//!pXOut->pOp->TraceXDepsIn();
	}

	while (!lXOuts.empty())
		memMgrEx().Delete(lXOuts.take_front());

//	Check Hidden();
//	pOpHi->Check Hidden();

	SetChangeInfo(FUI_XOUTS);
	SetChangeInfo(FUI_NUMBERS);
	return 1;
}

void InitialTracer_t::CheckCallArg(HOP pSelf) const
{
	assert(IsCallArg(pSelf));

	int bSplit;
	do {
		bSplit = 0;
		for (XOpList_t::Iterator i(pSelf->m_xin); i; i++)
		{
			HOP rOp(*i);
			if (pSelf->Offset0() == rOp->Offset0())//?
			if ((pSelf->OpSize() << 1) == rOp->OpSize())
			if (SplitAssignment(rOp))
			{
				bSplit = true;
				break;
			}
		}
	} while (bSplit);
}

int InitialTracer_t::AdjustCallEx(HOP pSelf, CGlobPtr iCallee, int nArgsEx) const
{
	//const FuncInfo_t& rfCallee

	assert(IsPrimeOp(pSelf));
	assert(ISCALL(ActionOf(pSelf)));

//	if (nArgsEx == -1)//preserve those allready here
	//	nArgsEx = GetVarArgSize(pSelf);

	int iRet(AdjustCall(pSelf, iCallee, nArgsEx));

	ProtoInfo_t TI(*this);
	if (ProtoInfo_t::IsStub(iCallee))
		TI.ApplyDefaultSpoiltRegs();

	TI.ApplyCalleeSpoiltRegs(iCallee);
	return iRet;
}

/*int InitialTracer_t::ChangeArgsExNumber(HOP pSelf, int nArgsNum)
{
	TypePtr ifDef = GetCalleeFuncDef(pSelf);
	if (!ifDef || !IsFuncVarArged(ifDef))
	{
		if (nArgsNum)
			return 0;
		return 1;
	}

	FuncInfo_t rCallee(DcRef(), *ifDef, FindFileDefOf(*ifDef->typeFuncDef()));

	return AdjustCallEx(pSelf, rCallee, nArgsNum);
}*/


//pOp1 is in caller context
//pOp2 is in callee context
bool InitialTracer_t::IsEqualed1(HOP pOp1, SSID_t ssid, int ofs2) const
{
	//assert(IsArgOp(pOp2));

	int retaddrsz = RetAddrSize();

	if (pOp1->OpC() != (OPC_t)ssid)//pOp2->SSID())
		return false;

	int ofs1 = pOp1->Offset0();
	//int ofs2 = pOp2->Offset();
	if (pOp1->OpC() == OPC_LOCAL)
	{
		int nBase = StackTop(PrimeOp(pOp1));
		if (ofs1 - nBase + retaddrsz != ofs2)
			return false;
	}
	else if (pOp1->OpC() == OPC_FPUREG)
	{
		int nBase = 0;assert(0);//?pOp1->pri meOp()->FpuTop();
		if (nBase - ofs1 != ofs2)
			return false;
	}
	else
	{
		if (ofs1 != ofs2)
			return false;
	}
#if(0)//?
	if (pOp1->OpSize() != pOp2->size())
		return false;
#endif
	return true;
}

//both pOp1 && pOp2 are in the same context
bool InitialTracer_t::IsEqualed2(HOP pOp1, HOP pOp2) const
{
	if (pOp1->OpC() != pOp2->OpC())
		return false;
	if (pOp1->Offset0() != pOp2->Offset0())
		return false;
	if (pOp1->OpSize() != pOp2->OpSize())
		return false;
	return true;
}

HOP InitialTracer_t::TakeCallArg(OpList_t& l, SSID_t ssid, int off) const
{
	if (l.empty())
		return HOP();

	//HOP pOpCall = l.Head()->prim eOp();
	//	int nStackTop = pOpCall->StackTop();

	//assert(pOpCall->IsCall());
	//assert(IsLocalArg(pField));

	for (OpList_t::Iterator i(l); i; i++)
	{
		if (IsEqualed1(i.data(), ssid, off))//this accesses caller context(!)
		{
			HOP pOp(i.data());
			l.take(pOp);//erase?
			return pOp;
		}
	}
	return HOP();
}

//searches for arg in the call
//IN CALLEE CONTEXT
#if(0)
HOP InitialTracer_t::FindCallArg1(OpList_t& lArgList, CFieldPtr pFieldArg)
{
	if (lArgList.empty())
		return HOP();

	HOP pOpCall = PrimeOp(lArgList.Head());
//	int nStackTop = pOpCall->StackTop();

	assert(IsCall(pOpCall));
	assert(IsLocalArg(pFieldArg));

	for (OpList_t::Iterator i(lArgList); i; i++)
	{
		if (IsEqualed1(i.data(), pFieldArg->SSID(), pFieldArg->opoff()))
			return i.data();
/*		if (pFieldArg->IsLo cal())
		{
			if (pArg->IsL ocal())
			{
				int nOpOffset = pFieldArg->Offset0();
				nOpOffset -= pFieldArg->m_pOwnerFuncDef->RetAddrSize();
			
				int nArgOffset = pArg->Offset0() - nStackTop;
				if (nArgOffset == nOpOffset)
				{
					if (pFieldArg->OpSize() == pArg->OpSize())
						return pArg;
				}
			}
		}
		else 
		{
			if (pArg->OpC() == pFieldArg->OpC())
			{
				assert (pFieldArg->OpC() == OPC_CPUREG || pFieldArg->OpC() == OPC_FPUREG);
				int ofs1 = pArg->Offset0();
				int ofs2 = pFieldArg->Offset0();
				if (pArg->OpC() == OPC_FPUREG)
					ofs2 += nFPUTop;
				if (ofs1 == ofs2)
				{
					if (pFieldArg->OpSize() == pArg->OpSize())
						return pArg;
				}
			}
		}*/
	}

	return HOP();
}
#endif

//makes search for the arg among those of call's ones 
//to find a such that matches pOp
//IN CALLER CONTEXT
HOP InitialTracer_t::FindCallArg2(OpList_t& lArgList, HOP pOp) const
{
	if (lArgList.empty())
		return HOP();

	HOP pOpCall = PrimeOp(lArgList.Head());

	assert(IsCall(pOpCall));

	for (OpList_t::Iterator i(lArgList); i; i++)
	{
		if (IsEqualed2(i.data(), pOp))
			return i.data();
/*
		if (pArg->OpC() != pOp->OpC())
			continue;
		assert(pArg->OpC() == OPC_LOCAL || pArg->OpC() == OPC_CPUREG || pArg->OpC() == OPC_FPUREG);

		if (pOp->Offset0() != pArg->Offset0())
			continue;
		if (pOp->OpSize() == pArg->OpSize())
			return pArg;*/
	}

	return HOP();
}


/*int InitialTracer_t::ToggleHidden(HOP hSelf)
{
	assert(IsPrimeOp(hSelf));
	if (hSelf->isHidden())
		Set Shown(hSelf);
	else
		Set Hidden(hSelf);
	return 1;
}*/

#define RETURN(arg) { tr.trace &= ~TRACE_MARK; return arg; }
int InitialTracer_t::__traceUsage(CHOP pSelf, CHOP hOpStart, bool bDir) const//1 - if used
{
	if (IsArgOp(pSelf))
	{
		if (!EqualTo(pSelf, hOpStart))
			return 1;
		return 0;
	}

	op_tracer_cell_t &tr(opTracer().cell(pSelf));
	if (tr.trace & TRACE_MARK)
		return 0;

	tr.trace |= TRACE_MARK;

	if (pSelf->IsIndirect())
	{
		if (!IsLocalOp(pSelf))
			RETURN(1);

		if (LocalRef(pSelf))
			if (LocalRef(pSelf)->isComplex())
				RETURN(1);
	}

	if (IsPrimeOp(pSelf))
		if (ISCALL(ActionOf(pSelf)) || ISINTRINSIC(ActionOf(pSelf)))
			RETURN(1);

	XOpList_t *plXDep(0);
	HOP pOp = HOP();

	if (!IsPrimeOp(pSelf) || !ActionOf(pSelf))
	{
		if (!bDir)//up
		{
			if (!pSelf->m_xin.empty())
			{
				plXDep = &pSelf->m_xin;
			}
			else
			{
				if (IsCode(pSelf))
				{
					ExprCacheEx_t aExprCache(PtrSize());
					AnlzXDepsIn_t an(*this, pSelf, aExprCache);
					an.ScanXDeps0(PrimeOp(pSelf));
				}
				
				if (!pSelf->m_xin.empty())
				{
					plXDep = &pSelf->m_xin;
				}
				else
				{
					if (!IsArgOp(pSelf))
						RETURN(1);
					if (hOpStart && !EqualTo(pSelf, hOpStart))
						RETURN(1);
					RETURN(0);
				}
			}
		}
		else//down
		{
			if (IsRhsOp(pSelf))
				pOp = PrimeOp(pSelf);
			else if (!pSelf->m_xout.empty())
				plXDep = &pSelf->m_xout;
		}

	}
	else
	{
		if (ActionOf(pSelf) != ACTN_MOV)
			RETURN(1);

		if (!bDir)
			pOp = pSelf->args().Head();
		else if (!pSelf->m_xout.empty())
			plXDep = &pSelf->m_xout;
	}

	if (!pOp)
	{
		for (XOpList_t::Iterator i(*plXDep); i; ++i)
		{
			HOP pOpIn(i.data());
			assert(!IsPrimeOp(pOpIn) || !pOpIn->isHidden());
			if (__traceUsage(pOpIn, hOpStart, bDir))
				RETURN(1);	
		}
	}
	else
	{
		if (IsPrimeOp(pOp))
		{
			if (pOp->isHidden())
				SetShown(pOp);
		}
		
		if (__traceUsage(pOp, hOpStart, bDir))
			RETURN(1);
	}

	RETURN(0);
}
#undef RETURN

/*int InitialTracer_t::MergeAssignment(HOP hSelf)
{
	assert(IsPrimeOp(hSelf));

	HOP pOp1 = hSelf;
	HOP pOp2 = pOp1->list().Next(pOp1);

	if (!pOp2)
		return 0;
	if ((pOp1->Action() != ACTN_MOV) || (pOp2->Action() != ACTN_MOV))
		return 0;
//	if (pOp1->OpSize() != pOp2->OpSize())//?
//		return 0;
	if ((pOp1->OpSize() != pOp1->arg1()->OpSize()) || (pOp2->OpSize() != pOp2->arg1()->OpSize()))
		return 0;//no sign extention

	if (pOp1->Offset0() > pOp2->Offset0())
		XChg(pOp1, pOp2);

	Op_t opL;
	CopyTo(pOp1, &opL);
	if (!MergeWith(&opL, pOp2))
		return 0;
	Op_t opR;
	CopyTo(&pOp1->arg1(), &opR);
	if (!MergeWith(&opR, &pOp2->arg1()))
		return 0;

	assert(fieldRef(&opL) == fieldRef(pOp1));
	CopyTo(&opL, pOp1);
	opL.Clear();
	assert(fieldRef(&opR) == fieldRef(&pOp1->arg1()));
	CopyTo(&opR, &pOp1->arg1());
	opR.Clear();

	pOp1->SetStackDiff(pOp1->StackDiff() + pOp2->StackDiff());
	pOp1->ins().m_nFpuDiff += pOp2->ins().fstackDiff();

	XOpList_t lXOuts(pOp2->m_xout);
	pOp2->m_xout = nullptr;//move?
	for (XOpList_t::Iterator i(lXOuts); i; i++)
		i.data()->ReleaseXDepIn(pOp2, mrMemMgr);

	delete pOp2;

	for (XOpList_t::Iterator i(lXOuts); i; i++)
	{
		AnlzXD epsIn_t an(*this, i.data());
		an.TraceXDepsIn();
		//!pXOut->pOp->TraceXDepsIn();
	}

	if (!lXOuts.empty())
		//lXOuts.erase(lXOuts.head(), true);//clear?
		lXOuts.destruct(mrMemMgr);

	AnlzX DepsIn_t an(*this, pOp1);
	an.TraceXDepsIn();
	//pOp1->TraceXDepsIn();
	CheckHidden(pOp1);

	SetChangeInfo(FUI_NUMBERS);
	return 1;
}*/


int InitialTracer_t::AdjustCall(HOP pSelf, CGlobPtr iCallee, int nArgsEx) const
{
	assert(IsCall(pSelf));

CHECK(OpNo(pSelf)==79)
//CHECK(OpVA(pSelf)==0x5a7f10)
STOP
	
	bool bDirty = false;

	OpList_t lOldArgs;
	pSelf->MoveArgsTo(lOldArgs);

	OpList_t::Iterator iOldArg(lOldArgs);

	ProtoInfo_t TI2(*this, (GlobPtr)iCallee);

	CallingConv_t CC(*this, iCallee);// , DcInfo_t::IsThisCallType(iCallee), false);

	FuncCCArgsCIt<> itArgs(*iCallee->typeFuncDef(), CC);
	for (; itArgs; ++itArgs)
	{
		SSID_t ssid(itArgs.ssid());
		int off(itArgs.offset());
//		if (ssid == SSID_LOCAL)
//			off -= CC.retAddrSize();
		uint8_t optyp((uint8_t)itArgs.size2());
		HOP pOp(TakeCallArg(lOldArgs, ssid, off));
		if (!pOp)
			pOp = CreateCallArg(pSelf, ssid, off, optyp);
		pSelf->args().LinkTail(pOp);
	}

	//append extra stack args
	nArgsEx /= mrDC.stackAddrSize();
	if (nArgsEx)
	{
		//get last stack arg
		HOP pArgLast = HOP();
		OpList_t::ReverseIterator j(pSelf->args());
		for (; j /*&& !j.is_first()*/; j++)
		{
			if (IsPrimeOp(j.data()))
				break;
			if (j.data()->IsIndirect())
			{
				pArgLast = j.data();
				break;
			}
		}
		/*for (pArgLast = &pSelf->args().back(); pArgLast; pArgLast = pSelf->args().Prev(pArgLast))
			if (pArgLast->IsIndirect())
				break;*/

		//get arg's displacement
		int nDisp = StackTop(pSelf);
		if (pArgLast)
		{
			nDisp = pArgLast->Offset0();
//			nDisp -= StackTop();
		}

		for (int i = 0; i < nArgsEx; i++)
		{
			nDisp += pArgLast->Size();
			HOP pArgEx = CreateStackArg(pSelf, nDisp);
			bool bNew = true;

			HOP pArg = HOP();
			if (iOldArg)
			{
				if (IsEqualed2(iOldArg.data(), pArgEx))
				{
					pArg = iOldArg.data();
					iOldArg++;
					bNew = false;
				}
				else
				{
					iOldArg.clear();
					bDirty = true;
				}
			}
			else
				bDirty = true;

			if (!pArg)
			{
				pArg = FindCallArg2(lOldArgs, pArgEx);
				if (pArg)
					bNew = false;
			}

			if (pArg)
			{
				PathOf(pArg)->takeOp(pArg);
				memMgrEx().Delete(pArgEx);
				pArgEx = pArg;
			}

			pSelf->args().LinkTail(pArgEx);
			pArgEx->setInsPtr(pSelf->insPtr());
			if (bNew)
			if (ProtoInfo_t::IsFuncStatusFinished(iCallee))
			{
				ExprCacheEx_t aExprCache(PtrSize());
				AnlzXDepsIn_t an(*this, pArgEx, aExprCache);
				an.ScanXDeps0(pSelf);
				CheckCallArg(pArgEx);
			}
		}
	}

	if (!lOldArgs.empty())
	{
		bDirty = true;
		FuncCleaner_t h(*this);
		h.ClearOpList(lOldArgs);
	}

	const FuncDef_t &rfCallee(*iCallee->typeFuncDef());
	if (TI2.IsFuncCleanArged())
		pSelf->addPStackDiff(itArgs.stackInTotal() + RetAddrSize());
	else
		pSelf->addPStackDiff(TI2.PStackPurge());

	Ins_t &rop(pSelf->ins());
	rop.setFStackDiff(rfCallee.getFStackPurge());//ok
	rop.mEFlagsModified = rfCallee.GetSpoiltFlags();

	if (!bDirty)
		return 1;

	if (ProtoInfo_t::IsFuncStatusFinished(iCallee))
	{
/*		CheckLocals0();
		TraceXDepsIn();

		//unroot xdeps of new args
		for (HOP pArg = GetA rgs(); pArg; pArg = pArg->Ne xt())
		{
			if (!pArg->IsSelected())
				continue;

			for (XRef_t *pXIn = pArg->XIn(); pXIn; pXIn = pXIn->Ne xt())
			{
				HOP pOp = pXIn->pOp;
				if (pArg->Offset0() == pOp->Offset0())//?
				if ((pArg->OpSize() << 1) == pOp->OpSize())
					pOp->SplitAssignment();

				if (pOp->IsCode())
				{
					HOP pOpRoot = PrimeOp(pOp);
					if (!pOpRoot->ReCheckRoot())
						pOpRoot->TurnRoot_Off(true);
//					else
//						pOpRoot->Chec kHidden();
				}
			}

			pArg->SetSelected(false);
		}
*/

//???		mrFuncDef.m_dc.SAVREGS = 1;//set dirty
		//GetOwn erFunc()->CheckSavedRegs();
		SetChangeInfo(FUI_ROOTS|FUI_XOUTS);
	}

	return 2;//dirty
}

int InitialTracer_t::AdjustCall(HOP pSelf, CTypePtr iFuncType, int) const
{
	assert(IsCall(pSelf));

	bool bDirty = false;

	OpList_t lOldArgs;
	pSelf->MoveArgsTo(lOldArgs);

	OpList_t::Iterator iOldArg(lOldArgs);
	unsigned stackInTotal(0);

	const TypeFunc_t &rf(*iFuncType->typeFunc());

	CallingConv_t CC(*this, iFuncType);// rf.isThisCall(), rf.isFastCall());
	for (FuncTypeArgsCIt2 it(iFuncType, CC); it; ++it)
	{
		CTypePtr iArg(*it);

		SSID_t ssid(it.ssid());
		int off(it.offset());
//		if (ssid == SSID_LOCAL)
	//		off -= CC.retAddrSize();
		uint8_t optyp(it.optype());
		HOP pOp(TakeCallArg(lOldArgs, ssid, off));
		if (!pOp)
			pOp = CreateCallArg(pSelf, ssid, off, optyp);
		pSelf->args().LinkTail(pOp);
		if (ssid == SSID_LOCAL)
			stackInTotal += (optyp & OPSZ_MASK);//align
	}

	if (!lOldArgs.empty())
	{
		bDirty = true;
		FuncCleaner_t h(*this);
		h.ClearOpList(lOldArgs);
	}
	
	int pstack_purge(RetAddrSize());
	if (rf.isStdCall())
		pstack_purge += stackInTotal;

	pSelf->addPStackDiff(pstack_purge);

	Ins_t &rop(pSelf->ins());
	rop.setFStackDiff(CC.fpuDiff(rf.retValOpType()));//?	rfCallee.getFStackPurge();//ok

	if (!bDirty)
		return 1;

//?	if (IsFuncDecompiled(rFICallee.FuncDefPtr()))
//?		SetChangeInfo(FUI_ROOTS|FUI_XOUTS);

	return 2;
}

int InitialTracer_t::AdjustCall(HOP pSelf, FuncProfile_t &fp, int) const
{
	assert(IsCall(pSelf));

	bool bDirty = false;

	OpList_t lOldArgs;
	pSelf->MoveArgsTo(lOldArgs);

	OpList_t::Iterator iOldArg(lOldArgs);
	unsigned stackInTotal(0);

	if (!lOldArgs.empty())
	{
		bDirty = true;
		FuncCleaner_t h(*this);
		h.ClearOpList(lOldArgs);
	}
	

	if (fp._flags & PPF_StackPurge)
		pSelf->setPStackDiff(stackInTotal);
	else
		pSelf->setPStackDiff(0);

//	Ins_t &rop(pSelf->ins());
	//rop.setFStackDiff(CC.fpuDiff(rf.retValOpType()));//?	rfCallee.getFStackPurge();//ok

	return 1;
}

int InitialTracer_t::ToggleSplit(HOP pSelf) const
{
	assert(IsPrimeOp(pSelf));

	return SplitAssignment(pSelf);
/*	if (isSplit())
		return TurnSplitOff();
	else
		return TurnSplitOn();*/
}

int InitialTracer_t::FixDisplacements(HOP pSelf) const
{//return 1;
	assert(IsPrimeOp(pSelf));

	for (OpList_t::Iterator i(pSelf->argsIt()); i ; i++)
	{
		HOP pOp(i.data());
		if (pOp->OpC() == OPC_CPUREG)
		if (pOp->m_disp != 0)
		{
			assert(false);
			HOP pOpTemp = CheckIndir(pOp);
			if (pOpTemp)
			{
				pOpTemp->m_disp += pOp->m_disp;
			}
			else
			{
				//ebp = eax.8 => ebp = eax + 8
				if (ActionOf(pSelf) == ACTN_MOV)
				{
					HOP pOp2 = NewOp();
					pOp2->Setup4(OPC_IMM, MAKETYP_SINT(pOp->OpSize()), 0, pOp->m_disp);
					pOp2->setInsPtr(pSelf->insPtr());
					AddOpArg(pSelf, pOp2);
					pSelf->setAction(ACTN_ADD);
				}
				else
				{
					//T32=ebp.8Eh+eax => T32=eax+8Eh,T32=T32+ebp
					assert(pSelf->args().check_count(2) == 0);
					
					HOP pOp2 = pSelf->arg2();//unlink 2nd op
					ReleaseXDepIn(pSelf, pOp2);
					ReleaseXDepOut(pOp2, pSelf);

					HOP pAux = NewPrimeOp();
					pAux->Setup3(OPC_AUXREG, pOp2->m_optyp, OFS(R_T64));

					HOP pOpI = NewOp();
					pOpI->Setup4(OPC_IMM, pOp2->m_optyp, 0, pOp->m_disp);

					AddOpArg(pAux, pOp2);
					AddOpArg(pAux, pOpI);
					pAux->setAction(ACTN_ADD);
					LinkOpBefore(pAux, pSelf);
					
					pOp2->setInsPtr(pAux->insPtr());
					pOpI->setInsPtr(pAux->insPtr());
					
					assert(!IsJointOp(pSelf) && !IsFirstExx(pSelf));
//					pAux->SetAnalized(0); 
					//SetRoot(pAux, 1); 
					
					Analize(pAux, PrevOp(pSelf));
//					pAux->SetAnalized(1); 

					pOp2 = NewOp();
					InitFrom(pOp2, pAux);
					pOp2->setInsPtr(pSelf->insPtr());
					AddOpArg(pSelf, pOp2);
					MakeUDLink(pOp2, pAux);//pOp2->ScanXDeps0(this);
					pOp->m_disp = 0;

					SetChangeInfo(FUI_NUMBERS);
					break;
				}
			}
			pOp->m_disp = 0;
		}
	}
#if(1)
	//u32=ebx+ecx,eax=u32 => eax=ebx+ecx
	if (ActionOf(pSelf) == ACTN_MOV)
	if (pSelf->arg1()->OpC() == OPC_AUXREG)
		if (pSelf->arg1()->m_xin.check_count(1) == 0)
	{
		HOP pOp0(pSelf->arg1());
		HOP pOp = pOp0->XIn()->data();
		if (pOp == PrevPrime(pSelf))
		if (EqualTo(pOp0, pOp))
		{
			FuncCleaner_t h(*this);
			pSelf->args().Unlink(pOp0);
			h.DestroyOp(pOp0);
//?			Ins_t *pri = pSelf->m_pRI;
//?			pSelf->m_pRI = pOp->m_pRI;//all args have been reallocated by this operation
			pOp->MoveArgsTo(pSelf->args());//re-locate args
			pSelf->setAction(ActionOf(pOp));
			pSelf->ins().mergeFrom(pOp->ins());

//?			pOp->m_pRI = pri;//? nullptr;

			h.DestroyPrimeOp(pOp);

			for (OpList_t::Iterator i(pSelf->argsIt()); i; i++)
				i.data()->setInsPtr(pSelf->insPtr());

			SetChangeInfo(FUI_NUMBERS);
		}
	}
#endif

	return 1;
}

FieldPtr InitialTracer_t::GetCalleeFieldEx(CHOP pSelf) const
{
	assert(IsPrimeOp(pSelf));
	if (!IsCall(pSelf))
		return nullptr;

	assert(!IsCallIntrinsic(pSelf));
	if (!IsAddr(pSelf))
	{
		//if (bDirectOnly)
		//return false;
		FieldPtr pField(nullptr);
		if (pSelf->IsIndirect())
		{
			//OpTracer_t optr(*this);
			//AnlzTracePtr an(*this, pSelf);
			//an.TracePtr Base();
			ExprCacheEx_t aCache(PtrSize());
			EXPRptr_t expr(*this, pSelf, OpNo(PrimeOp(pSelf)), aCache);
			//expr.setCahchedArgs(mpCachedArgs);
			expr.enableTraceXIns(true);
			//expr.setOpTracer(&opTracer());
			Out_t *pOut(expr.TracePtr(pSelf, pathOpTracer()));

			expr.Simplify(pOut, pSelf, pathOpTracer());

			if (pOut)
			{
				if (pOut->mpR->isFieldKind())
					pField = pOut->mpR->field();
				else if (pOut->mpR->is(ACTN_PTR))
					pField = pOut->mpR->mpR->field();
			}
		}


		if (!pField)
		{
			pField = FindFieldRef(pSelf);
			//assert(!pField || IsData(pField));
			if (!pField)
			{
				ExprCacheEx_t aCache(PtrSize());
				EXPRptr_t expr(*this, pSelf, OpNo(PrimeOp(pSelf)), aCache);
				expr.enableTraceXIns(true);
				Out_t *pOut(expr.TracePtr(pSelf, pathOpTracer(), false));

				expr.Simplify(pOut, pSelf, pathOpTracer());

				if (pOut && pOut->mpR->isFieldKind())
					pField = pOut->mpR->field();
			}
		}

		return pField;
	}

	return GetCalleeDockField(pSelf);
}

InitialTracer_t::CheckCallResult InitialTracer_t::CheckCall(HOP pSelf) const//Define
{
	if (!IsCall(pSelf))
		return CALL_NULL;

	ADDR curAddr(OpVA(pSelf));
	assert((int)curAddr > 0);

CHECK(curAddr == 0x100644f)
//CHECK(OpNo(pSelf) == 607)
STOP

	if (IsCallIntrinsic(pSelf))
	{
		GlobPtr iCallee(dc().getIntrinsic(pSelf->m_disp));
		if (!iCallee)
			return CALL_ERROR;
		//FuncDef_t &rfDef(*iType->typeFuncDef());
		//FuncInfo_t rCallee(DcRef(), *iCallee);
		AdjustCallEx(pSelf, iCallee, GetVarArgSize(pSelf));
		return CALL_INTRINSIC;
	}

	FuncProfile_t si;
	//InitFuncProfile(si);
	if (IsCallDirect(pSelf))
	{
		//define and build this function's arguments list.
		GlobPtr iCallee(GetFuncAtCall(pSelf));
		if (!iCallee)
		{
			Locus_t aLoc;
			if (!LocusFromOpDisp(pSelf, aLoc))
				return CALL_ERROR;
#if(0)
			ModuleInfo_t MI(*this, memMgrGlob());
			if (MI.MakeCode(aLoc, true) != RESULT_FAILED)//could have been just a code
			{
				if (MI.MakeThunk(aLoc) == RESULT_FAILED)//this is a function
					MI.MakeProcedure(aLoc);
			}
#endif
			FieldPtr pField(aLoc.field0());
			if (!pField)
				return CALL_ERROR;
			if (pField->isTypeProc())
			{
				FieldExPtr pFieldx(AddGlobToFile2(pField, FileInfo_t::OwnerFolder()));
				iCallee = GlobObj(pFieldx);
			}
			else
			{
				if (pField->isTypeThunk())
					return CALL_THUNK;//if we've got a thunk - imbed it into current func
				return CALL_ERROR;
			}
		}
		
		assert(iCallee);
		if (!iCallee->func() || ProtoInfo_t::IsStubUndefined(iCallee))
		{
			FieldPtr pField(DcInfo_s::DockField(iCallee));
			
			AssureFuncDef(iCallee);
			//FuncInfo_t rCallee(DcRef(), *iCallee);// , FileDef());
			StubInfo_t SI(DcRef());

			if (!ProtoInfo_t::IsFuncDecompiled(iCallee))
			{
				const Stub_t *pStub(SI.FindStubOrThrow(pField, curAddr));
				SI.CreateFuncProfile(pStub, si);
			}

			ProtoInfo_t TI(*this, iCallee);
			TI.FromFuncProfile(si);

			//FileInfo_t GI(*this, *iCallee->folder()->fileDef());
			//GI.
			FromFuncProfileEx(iCallee, si);
		}
		
		if (iCallee->func())
		{
			//FuncInfo_t rCallee(DcRef(), *iCallee);
			AdjustCallEx(pSelf, iCallee, GetVarArgSize(pSelf));
		}

		//later, when analizing first time and call of this func is met,
		//we need to adjust them acordingly
		return CALL_DIRECT;
	}

	FieldPtr pField(GetCalleeFieldEx(pSelf));
	if (pField)
	{
		if (pField->isTypeExp())
		{
			pField = ToExportedField(pField);
			if (!pField)
				return CALL_ERROR;
		}

		if (IsGlobal(pField))
			if (!GlobObj(pField))//not in file?
			{
				FieldExPtr pFieldx(AddGlobToFile2(pField, FileInfo_t::OwnerFolder()));
				pField = pFieldx;
			}

//CHECK(pField->address() == 0x1001148)
//STOP
		if (pField->isTypeImp())
		{
			FieldPtr pExpField(ToExportedField(pField));
			if (!pExpField)
				return CALL_ERROR;

			GlobPtr iCallee(GlobObj(pExpField));
			assert(iCallee);
			if (!iCallee->func() || ProtoInfo_t::IsStubUndefined(iCallee))
			{
				Dc_t* pDC2(DcFromFolder(*iCallee->folder()));
				DcInfo_t DC2(*pDC2);
				DC2.AssureFuncDef(iCallee);
				pSelf->SetOpType(OPTYP_PTR32);//?
				StubInfo_t SI2(DC2);
				const Stub_t* pStub(SI2.FindStub(StubBase_t(pExpField, -1).atAddr()));
				if (!pStub)
				{
					if (IsPhantomModule(pDC2->module()))
					{
						StubInfo_t SI(*this);
						pStub = SI.FindStub(pField->_key());
						if (!pStub)
						{
							StubInfo_t SI(*this);//in current module
							pStub = SI.FindStubOrThrow(pField, curAddr);
							//throw StubFault_t(stubFault.field());//override with imported field and propagate
						}
					}
					else
					{
						throw StubFault_t(pExpField);
					}
				}
				SI2.CreateFuncProfile(pStub, si);
				ProtoInfo_t TI2(*pDC2, iCallee);
				TI2.FromFuncProfile(si);
				FileInfo_t GI2(*this, *iCallee->folder()->fileDef());
				GI2.FromFuncProfileEx(iCallee, si);
				AdjustCallEx(pSelf, iCallee, 0);
			}
			else
			{
				AdjustCallEx(pSelf, iCallee, GetVarArgSize(pSelf));
			}
		}
		else
		{
CHECK(curAddr == 0x477c6f)
STOP
			TypePtr iTypeFunc(nullptr);
			StubInfo_t SI(*this);
			const Stub_t *pStub(SI.FindStub(curAddr));//code first
			if (!pStub)
				pStub = SI.FindStub(pField->_key());//data second

			if (!pStub)
			{
				iTypeFunc = IsPtrToFuncType(pField);
				if (!iTypeFunc)
					SI.StubFault(pField);//exception here
				AdjustCall(pSelf, iTypeFunc, GetVarArgSize(pSelf));
			}
			else//stubs got precedence!
			{
				DcInfo_t DI(*this, memMgrGlob());
				DcCleaner_t<> DC(DI);
				
				if (SI.CreateFuncProfile(pStub, si))
				{
					iTypeFunc = DcInfo_t::FuncTypeFromProfile(si);
					AdjustCall(pSelf, iTypeFunc, 0);

					ProtoInfo_t TI(*this);
					TI.ApplyDefaultSpoiltRegs();

					TypePtr iOldType(TakeTypeOf(pField));

					TypesTracer_t TT(*this, memMgrGlob(), *PrimeSeg()->typeMgr());
					TypePtr iPtr(TT.ptrOf(iTypeFunc, PtrSizeOf(PrimeSeg())));
					SetType(pField, iPtr);
					DC.ReleaseTypeRef(iOldType);
//					DC.ReleaseTypeRef(iTypeFunc);

				}
			}
		}
	}
	else
	{
CHECK(OpNo(pSelf) == 0x21)
STOP

		//FileDef_t &fileDef(mrFileDef);
/*#if(1)//use own mem mgr for temp object(s), but be sure to clean up
		MemoryMgr_t &memMgr(mrMemMgr);
		FileDef_t &fileDef(mrFileDef);
#else
		FileDef_t fileDef;
		MemoryMgr_t &memMgr(fileDef.memMgr());
#ifdef _DEBUG
		memMgr.mName = "$TMP";
#endif
#endif*/

		DcInfo_t DI(mrDC, memMgrGlob());

		//GlobPtr iCallee(DI.NewFuncDef());
		class TmpFieldEx_t : public FieldEx_t
		{
			DcInfo_t& DI;
		public:
			TmpFieldEx_t(DcInfo_t& _DI)
				: DI(_DI)
			{
				glob().SetPvt(new FuncDef_t())->setPStackPurge(DI.RetAddrSize());
				//glob().flags() |= TYP_FDEF_TEMP;
			}
			~TmpFieldEx_t()
			{
				DI.UnmakeMemberMethod(globPtr());
				DcCleaner_t<> DC(DI);
				DC.destroyFuncDef(globPtr());
				glob().destruct(DI.memMgrGlob());
			}
		};

		TmpFieldEx_t aTmp(DI);
		GlobObj_t& aCallee(*aTmp.globPtr());

		pSelf->SetOpType(OPTYP_PTR32);

		ProtoInfo_t TI(DI, &aCallee);//under the same TLS hood!

		StubInfo_t SI(DI);
		const Stub_t *pStub(SI.FindStubOrThrow(nullptr, curAddr));
		if (SI.CreateFuncProfile(pStub, si))
		{
			TI.FromFuncProfile(si);
			FromFuncProfileEx(&aCallee, si);//may create a new type (struc)! AND PUT IN FILE!
			AdjustCallEx(pSelf, &aCallee, 0);
		}
	}

	return CALL_INDIRECT;
}

/*int InitialTracer_t::CheckIndirGoto(HOP pSelf, ADDR curAddr)
{
	if (!IsGoto(pSelf))
		return 0;
	if (IsAddr(pSelf))
		return 0;
	FieldPtr pField(GetDataSwitch(pSelf));
	HPATH pPath(Lab elPath(pField));

	if (!PathOps(pPath).empty())
		return 0;

	Goto2Call(pSelf);
	return 1;
}*/

/*int InitialTracer_t::CheckCallPtr(HOP pSelf)
{
	if (pSelf->m_opc & OPC _ADDRESS)
	{
		FieldPtr pFieldRef(LabelRef(pSelf));
		if (pFieldRef)
		{
			if (IsLabel(pFieldRef))
			{
				if (IsPrimeOp(pSelf) && (IsGoto(pSelf) || IsCondJump(pSelf)))
					return 0;

				if (pFieldRef->ownerProcPvt())
				{
					assert(pFieldRef->address() == pFieldRef->owner()->base());
					return 0;
				}
			}
			else if (!pFieldRef->type())
			{
				if (!IsCall(pSelf))
					return 0;
			}
			else
				return 0;

			pFieldRef->AssureObjType(OBJID_TYPE_PROC);//convert if not a fiunc?
		}
	}

	if (!IsPrimeOp(pSelf))
		return 0;

	for (OpList_t::Iterator i(pSelf->argsIt()); i; i++)
		CheckCallPtr(i.data());

	return 1;
}*/

int InitialTracer_t::CheckSpecial(HOP pSelf) const
{
	if (ActionOf(pSelf) == ACTN_XOR)
	{
		if (pSelf->args().check_count(2) == 0 )
			if (EqualTo(pSelf->arg1(), pSelf->arg2()))
		if (EqualTo(pSelf, pSelf->arg1()) )
		{
			pSelf->arg1()->Setup4(OPC_IMM, pSelf->Size(), 0, 0);
			HOP pOp0(pSelf->arg2());
			FuncCleaner_t h(*this);
			pSelf->args().Unlink(pOp0);
			h.DestroyOp(pOp0);
			pSelf->setAction(ACTN_MOV);
			pSelf->m_optyp &= OPSZ_MASK;
			return 1;
		}
	}
	else if (ActionOf(pSelf) == ACTN_AND)
	{
		if (pSelf->args().check_count(2) == 0 )
		if (EqualTo(pSelf->arg1(), pSelf->arg2()) )
		if (pSelf->arg1()->OpSize() == pSelf->arg2()->OpSize() )
		{
			pSelf->Setup3(FOPC_CPUSW, OPTYP_UINT16, 0);
			HOP pOp0(pSelf->arg2());
			FuncCleaner_t h(*this);
			pSelf->args().Unlink(pOp0);
			h.DestroyOp(pOp0);
			return 1;
		}
	}
	else if (ActionOf(pSelf) == ACTN_OR )
	{
		if (pSelf->args().check_count(2) == 0 )
		{
			assert(EqualTo(pSelf, pSelf->arg1()));
			if (EqualTo(pSelf->arg1(), pSelf->arg2()))
			{
				if (pSelf->arg1()->OpSize() == pSelf->arg2()->OpSize() )
				{
					pSelf->Setup3(FOPC_CPUSW, OPTYP_UINT16, 0);
					HOP pOp0(pSelf->arg2());
					FuncCleaner_t h(*this);
					pSelf->args().Unlink(pOp0);
					h.DestroyOp(pOp0);
					pSelf->setAction(ACTN_AND);
					return 1;
				}
			}
			else if (pSelf->arg2()->IsScalar())
			{
				if (OpDisp(pSelf->arg2()) == -1)//0xffffffff
				{
					pSelf->setAction(ACTN_MOV);
					HOP pOp0(pSelf->arg1());
					FuncCleaner_t h(*this);
					pSelf->args().Unlink(pOp0);
					h.DestroyOp(pOp0);
					return 1;
				}
			}
		}
	}
	else if (ActionOf(pSelf) == ACTN_SUB )
	{
		HOP pOp0 = pSelf->arg1();
		HOP pOp1 = pSelf->arg2();
		if (pOp1->IsScalar())// && !pOp1->Is Addr())
		{
			if (EqualTo(pSelf, pOp0) )
			{
				if (OpDisp(pOp1) == 0)
				{
					pSelf->Setup3(FOPC_CPUSW, OPTYP_UINT16, 0);
					pSelf->setAction(ACTN_AND);
					FuncCleaner_t h(*this);
					pSelf->args().Unlink(pOp1);
					h.DestroyOp(pOp1);
					return 1;
				}
				
				if (IsStackPtrOp(pSelf))
				if (OpDisp(pOp1) < 0 )
				{
					pSelf->setAction(ACTN_ADD);
					pOp1->m_disp = -OpDisp(pOp1);
					return 1;
				}
				
/*				if (0)
				if (pOp0->OpC() == OPC_CPUREG)
//				if (pOp0->IsStackPtr())
				{
					assert(!pOp0->m_disp);
					setAction(ACTN_MOV);
					pOp0->m_disp = pOp1->OpDisp();
					delete pOp1;
					return 1;
				}*/
			}
		}
	}
	else if (ActionOf(pSelf) == ACTN_ADD)
	{
		HOP pOp0 = pSelf->arg1();
		HOP pOp1 = pSelf->arg2();
		if (pOp1->IsScalar())// && !pOp1->Is Addr())
		{
			if (EqualTo(pSelf, pOp0) )
			{
				if (OpDisp(pOp1) == 0)
				{
					pSelf->Setup3(FOPC_CPUSW, OPTYP_UINT16, 0);
					pSelf->setAction(ACTN_AND);
					FuncCleaner_t h(*this);
					pSelf->args().Unlink(pOp1);
					h.DestroyOp(pOp1);
					return 1;
				}
				
				if (IsStackPtrOp(pSelf) )
				if (OpDisp(pOp1) < 0 )
				{
					pSelf->setAction(ACTN_SUB);
					pOp1->m_disp = -OpDisp(pOp1);
					return 1;
				}
			}

/*			if (0)
			if (pOp0->OpC() == OPC_CPUREG)
			{
				assert(!pOp0->m_disp);
				setAction(ACTN_MOV);
				pOp0->m_disp = pOp1->OpDisp();
				delete pOp1;
				return 1;
			}*/
		}
	}

	return 0;
}

int InitialTracer_t::CheckShift(HOP pSelf) const
{
	assert(IsPrimeOp(pSelf));
	if (ActionOf(pSelf) == ACTN_SHL)
	{
		HOP op2 = pSelf->arg2();
		if (!op2->IsScalar())
			return 0;

		pSelf->setAction(ACTN_MUL);

		int nCount = OpDisp(op2);
		op2->m_disp = 1;
		op2->m_disp <<= nCount;
		//adjust type's size
		HOP op1 = pSelf->arg1();
		op2->SetOpSize(op1->OpSize());
	}
	else if (ActionOf(pSelf) == ACTN_SHR)
	{
		HOP op1 = pSelf->arg1();
		if (OPTYP_IS_SINT(op1->m_optyp))//?
			return 0;
		HOP op2 = pSelf->arg2();
		if (!op2->IsScalar())
			return 0;

		pSelf->setAction(ACTN_DIV);
		
		int nCount = op2->m_disp;
		op2->m_disp = 1;
		op2->m_disp <<= nCount;
		op2->SetOpSize(op1->OpSize() >> 1);
	}

	return 1;//ok
}

int InitialTracer_t::CheckReadOnly(HOP pSelf) const
{
	assert(IsPrimeOp(pSelf));
/*?
	if (IsLValue(pSelf))// || IsCall())
		if (pSelf->IsIndirect())
		{
			FieldPtr pLabel(LabelRef(pSelf));
			if (pLabel)//FIXME:check only simple datas?
				if (IsConst(pLabel))
					if (IsThrouConst(pLabel))
					{
						SetThruConst(pLabel, false);
						pLabel->m_nFlags &= ~FLD_INVERTED;
						return 1;
					}
		}*/
	return 0;
}

STAGE_STATUS_e InitialTracer_t::CheckSpoiltRegs() const
{
	if (!testChangeInfo(FUI_SAVREGS))
		return STAGE_STATUS_e::SKIPPED;//skipped

	if (!mrFE.spoilt_regs_check)
		return STAGE_STATUS_e::SKIPPED;

	if (!GetExitPoints(mrFuncDef))
		return STAGE_STATUS_e::SKIPPED;

	//OpTracer_t op_tracer;
	//setOpTracer(&op_tracer);

	opTracer().reset(*this);//clear
	//mrFuncDef.mSpoiltRegs = 0;//xFFF0FFFF;

	HPATH pTail(mrPathTree.pathTail());

	Ins_t ri;
	HOP op(NewPrimeTmpOp());
	AttachToPath(op, pTail);

	ExprCacheEx_t aExprCache(PtrSize());

	ProtoInfo_t TI(*this);

	for (FuncSpoiltIt it(mrFuncDef); it; )
	{
		FieldPtr pField(it.field());
		assert(!IsStackPtr(pField));

		RegMaskType rmask(TOREGMASK(address(pField), pField->OpSize()));

		op->Setup3(SSIDx(pField), pField->OpSize(), address(pField));

		AnlzXDepsIn_t anlzxdepsin(*this, op, aExprCache);
		anlzxdepsin.ScanXDeps1(op);
		for (XOpList_t::Iterator i(op->m_xin); i; ++i)
		{
			HOP hOp(*i);
			if (IsCode(hOp))
			{
				HOP hOpRoot(PrimeOp(hOp));
				SetShown(hOpRoot);
			}
		}

		if (__traceUsage(op, op, false))//used upward!
		{
			//mrFuncDef.mSpoiltRegs |= rmask;//&= ~rmask;
			++it;
		}
		else
		{
			TI.ClearLocalType(pField);
			memMgrGlob().Delete(mrFuncDef.argFields().take(it));//iterator advanced here!
		}

		//op.m_pRI = 0;
		//ClearOp(&op, true);
		FuncCleaner_t h(*this);
		h.DisconnectOp(op, false);
	}

	assert(pTail->ops().contains(INSPTR(op)));
	pTail->takeOp(INSPTR(op));
	//op.clearInsPtr();
	//op.setInsPtr(nullptr);
	DeleteRootInfo(op);
	Delete(op);

	clearChangeInfo(FUI_SAVREGS);
	return STAGE_STATUS_e::DONE;
}

HOP InitialTracer_t::CreateStackArg(HOP pSelf, int nDisp) const
{
	assert(IsPrimeOp(pSelf));
	HOP pArg = NewOp();
	pArg->Setup4(OPC_LOCAL, mrDC.stackAddrSize(), 0, nDisp);
	//!	pArg->SetO pSeg(OPSEG_SS);
	return pArg;
}

HOP InitialTracer_t::CreateCallArg(HOP pSelf, SSID_t ssid, int off, uint8_t siz) const
{
	assert(IsCall(pSelf));
	//assert(IsLocalArg(pCalleeField));

	HOP  pArg;
	if (ssid == SSID_LOCAL)
	{
		int nOffset = off;

		//nOffset -= rFICallee.RetAddrSize();

		pArg = NewOp();

		pArg->Setup4((uint8_t)(OPC_SS | SSID_CPUREG), siz, mrFE.stack_ptr->ofs, nOffset);
		pArg->SetOpSeg(OPSEG_STACK);
	}
	else if (ssid == SSID_FPUREG)
	{
		pArg = NewOp();
		int offs(off);
		pArg->Setup4((uint8_t)(OPC_INDIRECT | SSID_FPUSW), siz, OFS(R_FPUSW_TOP), offs / FR_SLOT_SIZE);
	}
	else
	{
		pArg = NewOp();
		pArg->Setup3(ssid, siz, off);
	}

	pArg->setInsPtr(pSelf->insPtr());
	return pArg;
}

int InitialTracer_t::SetFPUOut0(int nFPUOut) const
{
	if (nFPUOut == mrFuncDef.getFStackPurge())
		return -1;

	int n(nFPUOut / FTOP_STEP);

	if (n > 8 || nFPUOut < -8)
		return 0;

	mrFuncDef.setFStackPurge(nFPUOut);
	return 1;
}

int InitialTracer_t::UpdateCallers() const
{
		assert(0);

#if(0)
	if (!(mrFuncDef.flags() & FDEF_UPDCALLERS))
		return 0;

//	TRACE1("%s: Updating callers...\n", na mex());

//CHECK(CompName("sub_220292A0"))
//STOP
	
	assert(mrFuncDef.Methods Of().empty());
	FieldPtr  pField = DockField();

	for (XOpList_t::Iterator i(pField->xre fs()); i; i++)
	{
		HOP  pOp = i.data();
		if (!pOp->ob jOp())
			continue;
		if (!pOp->IsCall())
			continue;

		assert(0);
		TypeProc_t *pFunc(0);//? i.data().pFunc);//pOp->GetO wnerFunc();
		FuncInfo_t fi(dc(), *pFunc->func def());
//		if (!pFunc->IsDecompiled())
//			continue;

		if (mrFuncDef.flags() & FDEF_CHGARGLIST)
		{
			InitialTracer_t an(*this);//?
			FuncInfo_t rCallee(dc(), *an.GetCalleeFuncDef(pOp));
			an.AdjustCallEx(pOp, rCallee, GetVarArgSize(pOp));
		}

		//re-check callrets
		if (mrFuncDef.flags() & FDEF_CHGRETLIST)
		{
			for (XOpList_t::Iterator j(pOp->m_xout); j; j++)
			{
				CheckCallout(j.data());
			}
		}

		if (mrFuncDef.flags() & FDEF_CHGFRAME)
			fi.SetChangeInfo(FUI_BASE);

		if (mrFuncDef.flags() & FDEF_CHGSAVREGS)
			fi.SetChangeInfo(FUI_ALL);//FUI_XDEPSINS|FUI_SAVREGS);
	}

	for (XOpList_t::Iterator i(pField->xre fs()); i; i++)
	{
		HOP pOp = i.data();
		if (!pOp->IsCall())
			continue;

//CHECK(pOp->No() == 70)
//STOP
		
		assert(0);
		TypeProc_t *pFunc(0);// i.data().pFunc);//pOp->GetOw nerFunc();
		if (mrFuncDef.flags() & FDEF_CHGARGLIST)
		{
			AnlzTracePtr a(*this, pOp);//?
			a.TraceArgPtrs();
		}

	}

	mrFuncDef.flags() &= ~FDEF_UPDCALLERS;
#endif
	return 1;
}




