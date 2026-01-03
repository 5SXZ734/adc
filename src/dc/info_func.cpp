#include "info_func.h"

#include "prefix.h"

#include "shared/defs.h"
#include "shared/link.h"
#include "shared/front.h"
#include "shared/misc.h"
#include "shared/action.h"
#include "db/mem.h"
#include "db/obj.h"
#include "db/type_struc.h"
#include "db/types.h"
#include "db/field.h"
#include "arglist.h"
#include "info_dc.h"
#include "path.h"
#include "op.h"
#include "ana_ptr.h"
#include "ana_data.h"
#include "ana_expr.h"
#include "ana_local.h"
#include "ana_switch.h"
#include "ana_branch.h"
#include "ana_pcode.h"
#include "ana_main.h"
#include "expr.h"
#include "reg.h"
#include "ui_main_ex.h"
#include "clean_ex.h"
#include "expr_ptr.h"
#include "cc.h"
#include "info_class.h"


///////////////////////////////////////////////////
// FuncInfo_t

FuncInfo_t::FuncInfo_t(const FuncInfo_t &r)
	: FileInfo_t(r),
	mrFuncDefRef(r.mrFuncDefRef),
	mrFuncDef(r.mrFuncDef),
	mrPathTree(r.mrPathTree),
	mrOwnerSeg(*OwnerSegEx((GlobToTypePtr)&mrFuncDefRef))
{
#if(NEW_PATH_PTR)
	assert(OwnsMemMgr());
	tls::SetPoolPtr(&memMgrEx().mPaths);
#endif
#if(NEW_OP_PTR)
	assert(OwnsMemMgr());
	tls::SetPoolPtr(&memMgrEx().mOps);
#endif
}

FuncInfo_t::FuncInfo_t(const DcInfo_t &rDI, const GlobObj_t &rFuncRef)//, FileDef_t &rFileDef)
	: FileInfo_t(rDI, *rFuncRef.folder()->fileDef()),//rFileDef),
	mrFuncDefRef((GlobObj_t &)rFuncRef),
	mrFuncDef(*mrFuncDefRef.typeFuncDef()),
	mrPathTree(mrFuncDef.pathTree()),
	mrOwnerSeg(*OwnerSegEx((GlobToTypePtr)&mrFuncDefRef))
{
#if(NEW_PATH_PTR)
	assert(OwnsMemMgr());
	tls::SetPoolPtr(&memMgrEx().mPaths);
#endif
#if(NEW_OP_PTR)
	assert(OwnsMemMgr());
	tls::SetPoolPtr(&memMgrEx().mOps);
#endif
}

FuncInfo_t::FuncInfo_t(const FileInfo_t &rFI, CGlobPtr pGlob)
	: FileInfo_t(rFI),
	mrFuncDefRef((GlobObj_t &)*pGlob),
	mrFuncDef(*mrFuncDefRef.typeFuncDef()),
	mrPathTree(mrFuncDef.pathTree()),
	mrOwnerSeg(*OwnerSegEx((GlobToTypePtr)&mrFuncDefRef))
{
#if(NEW_PATH_PTR)
	assert(OwnsMemMgr());
	tls::SetPoolPtr(&memMgrEx().mPaths);
#endif
#if(NEW_OP_PTR)
	assert(OwnsMemMgr());
	tls::SetPoolPtr(&memMgrEx().mOps);
#endif
}

FuncInfo_t::~FuncInfo_t()
{
#if(NEW_PATH_PTR)
	assert(OwnsMemMgr());
	tls::SetPoolPtr((MemPathPool *)nullptr);
#endif
#if(NEW_OP_PTR)
	assert(OwnsMemMgr());
	tls::SetPoolPtr((MemOpPool *)nullptr);
#endif
}

bool FuncInfo_t::OwnsMemMgr() const
{
	return (&mrMemMgr != &dc().memMgr());
}

FieldPtr FuncInfo_t::FindArgFromOp(CHOP pOpRef) const
{
	if (mrFuncDef.hasArgFields())
	{
		ri_t src(pOpRef->SSID(), pOpRef->Offset0(), pOpRef->OpSize());
		CallingConv_t CC(*this, FuncDefPtr());
		for (FuncCCArgsCIt<> it(FuncDef(), CC); it; ++it)
		{
			ri_t tgt(it.toR_t());
			if (ProtoInfo_t::IsOpOverlappedByField(src, tgt))
				return (FieldPtr)it.field();
		}
	}
	return nullptr;
}

HOP FuncInfo_t::FindOpFromRetField(HOP hOpRet, CFieldPtr pField) const
{
	assert(IsRetOp(hOpRet));
	for (OpList_t::Iterator i(hOpRet->args()); i; ++i)
	{
		if (i.data()->EqualTo(pField))
			return i.data();
	}
	return HOP();
}

int FuncInfo_t::CheckBreak(HOP pSelf) const
{
	if (PathOf(pSelf))
		if (CheckGOTOStatus(PathOf(pSelf)) == JUMP_BREAK)
			return 1;

	return 0;
}

bool FuncInfo_t::IsDeadEx(CHOP pSelf) const
{ 
	if (pSelf->isHidden() || !PathOf(pSelf)->isAnalized())
	{
		if (!ProtoInfo_t::IsFuncStatusFinished(&mrFuncDefRef))
			return false;
		return true;
	}
	if (IsGoto(pSelf))
	{
//		if ( g_ pDI)
		HPATH pPath = PathOf(pSelf);
		if (pPath && CheckGOTOStatus(pPath) == 0)
			return true;
//		if ( CheckGOTOStatus() == 0 )
//			return true;
	}
	if (IsRetOp(pSelf) && IsLastExx(pSelf) && !pSelf->hasArgs())
		return true;
	return false;
}//actual only when output is assemblerized

bool FuncInfo_t::IsDefaultOf(CHOP pSelf, CHPATH rPath) const
{
	assert(PathType(rPath) == BLK_SWITCH);

	HPATH pPathTo = PathRef(pSelf);
	HPATH pPathNx = TreeNextEx(rPath);//path next after switch
	assert(pPathNx);
	if (pPathNx == pPathTo)
		return 1;

	if (PathType(pPathNx) == BLK_JMP)
		if (CheckActualOpsCount(pPathNx, 1) == 0)
			if (PathRef(GetLastOp(pPathNx)) == PathRef(pSelf))
				return 1;
//	if (PathType(pPathNx) == BLK_RET)
//		if (pPathNx->Compare(pPathTo))
//			return 1;
	return 0;
}

bool FuncInfo_t::IsCaseOf(CHOP pSelf, CHPATH rPath) const
{
	assert(PathType(rPath) == BLK_SWITCH);
	assert(PathRef(pSelf));

	//FieldPtr pLabel = PathRef(pSelf);
	HPATH pBlockCase = HPATH();
	//check for case block at this label
	for (HPATH pBlock(PathRef(pSelf)); pBlock; pBlock = pBlock->Parent())
	{
		if (PathType(pBlock) == BLK_CASE)
		{
			pBlockCase = pBlock;
			break;
		}
		if (!TreeIsFirst(pBlock))
			break;
	}

	if (pBlockCase)
	{//check if pBlockCase reffers to this switch
		HPATH pBlockSwitch = pBlockCase->Parent();
		if (IsNull(pBlockSwitch))
			pBlockSwitch = pBlockSwitch->Parent();
		if (pBlockSwitch == rPath)
		{
			if (GetJumpTablePath(pBlockSwitch->GetChildFirst()) == PathOf(pSelf))
				return 1;//ok
		}
	}

	return 0;
}

//given a data op, returns a switch jump op, if succeeds
HOP  FuncInfo_t::GetSwitchOp(CHOP pSelf, HOP *ppOpTblRef) const
{
	assert(IsDataOp(pSelf));
	HPATH pPath(PathOf(pSelf));
	if (!pPath)
		return HOP();
	assert(pPath->Type() == BLK_DATA);
	HPATH pPathPr(TreePrevSibling(pPath));
	assert(pPathPr->Type() == BLK_JMPSWITCH);
	HOP pLastOp(GetLastOp(pPathPr));
	return pLastOp;
	//return GetSwitchOp(pSelf->GetOwnerData(), ppOpTblRef);
}

int32_t FuncInfo_t::OpDisp(CHOP pSelf) const
{
	FieldPtr pFieldRef(LocalRef(pSelf));
	if (!pFieldRef)
		return pSelf->m_disp;
	return pSelf->m_disp - address(pFieldRef);
}

int FuncInfo_t::CheckProblemUnfold(HOP pSelf) const
{
	if (!IsPrimeOp(pSelf))
		return 0;
	if (!IsAnalized(pSelf))
		return 0;

	int32_t nStackOut(StackOut(pSelf));
	int8_t nFpuOut(FpuOut(pSelf));

	HOP pOp1 = HOP();
	HOP pOp2 = HOP();
	if (IsCondJump(pSelf) || IsGoto(pSelf))
		if (PathRef(pSelf))//fixme:indirect goto
			pOp1 = PRIME(PathOps(PathRef(pSelf)).front());
	if (!IsGoto(pSelf))
		pOp2 = NextPrimeEx(pSelf);

	if (pOp1 && IsAnalized(pOp1))
	{ 
		if (pOp1->pstackIn() != nStackOut)
			return 1;
		if (FpuIn(pOp1) != nFpuOut)
			return 2;
	}

	if (pOp2 && IsAnalized(pOp2))
	{ 
		if (pOp2->pstackIn() != nStackOut)
			return 1;
		if (FpuIn(pOp2) != nFpuOut)
			return 2;
	}

	return 0;
}

HPATH FuncInfo_t::EnsurePathBreakAt(HOP pSelf)
{
	assert(IsPrimeOp(pSelf));
	if (IsJointOp(pSelf))
		return PathOf(pSelf);

	HPATH pPath(PathOf(pSelf));
	assert(pPath);
	if (PRIME(pPath->ops().Head()) != pSelf)//not first
	{//split the path

		HPATH pPathNx(SplitAfter(PathOf(pSelf), PathType(pPath)));
		SetPathType(pPath, BLK_NULL);
		//redirect prev ops to new path
		PathOpList_t::Iterator i(pPath->opsRef(), INSPTR(pSelf));
		while (i)
		{
			HOP pOp(PRIME(i.data()));//get first op
			i++;
			pPath->takeOp(INSPTR(pOp));
			LinkOpTail(pPathNx, pOp);//add to the tail
		}		
	}

	return PathOf(pSelf);
}

bool FuncInfo_t::IsLValue(CHOP pSelf) const
{
	if (!IsCodeOp(pSelf))
	{
		if (IsCallOutOp(pSelf))
			return true;//all callouats are L-values
	}
	else
	{
		if (IsPrimeOp(pSelf))
			if (!IsGoto(pSelf))//not indirect goto
				if (!IsCall(pSelf))//not indirect call
					if (!IsRetOp(pSelf))
						if (!IsCondJump(pSelf))
							return true;
	}
	return false;//not L-value
}

/*void FuncInfo_t::dump(FuncDef_t &rSelf, std::ostream &os)
{
	dump(rSelf.pathTree(), os);
}*/


bool FuncInfo_t::IsActual(CHOP pSelf) const//0-if not
{
	assert(IsPrimeOp(pSelf));
CHECK(IsVarOp(pSelf))
STOP

	if (!pSelf->isRoot())
		return false;

	//if (pSelf->isInRow())
		//return false;

	if (IsDeadEx(pSelf))
		return false;

//?	if (IsRootVisible())
	{
//		if (!IsDead())
		if (!pSelf->isHidden())
			return true;
	}

	if (!ProtoInfo_t::IsFuncStatusFinished(&mrFuncDefRef))
		return true;
	return false;
}

int FuncInfo_t::Unbind(HOP pSelf) const
{
//	if (!pSelf->IsVarOp())
	//	return ToggleInRowStatement(pSelf);
	if (pSelf->isHidden() || !pSelf->isRoot())
		return 0;
	//if (pSelf->isInRow())
		//return 0;

	const PathOpList_t &l(PathOf(pSelf)->ops());
	PathOpList_t::Iterator i(l, INSPTR(pSelf));
	for (++i; i; i++)
	{
		HOP pOp(PRIME(i.data()));
		if (pOp->isHidden() || !pOp->isRoot())
			continue;
		if (!pOp->isInRow())
			break;
		if (IsVarOp(pOp))
		{
			HOP pOpRef(GetOpToBindWith(pOp));
			assert(pOpRef);//otherwise it wouldn't be inrow'd
			assert(PrimeOp(pOpRef)->isRoot());

			//don't want to unbind a varop unless there is no inrow'd op above
			for (++i; i; i++)
			{
				HOP pOp2(PRIME(i.data()));
				if (pOp2->isHidden() || !pOp2->isRoot())
					continue;
				if (!pOp2->isInRow())
					break;
				pOp = pOp2;
				break;
			}
		}
		pOp->setInRow(false);

		HPATH pIfForPath(CheckForCondition(pSelf));
		if (!pIfForPath)
			return 1;

		HPATH pPathDoWhile(GetLoopWhileBackwardCondition(pIfForPath));
		assert(pPathDoWhile);
		HOP pOp2(GetLastOp(pPathDoWhile));
		Unbind(pOp2);
		return 2;
	}
	return 0;
}

int FuncInfo_t::Bind(HOP pSelf) const
{
CHECKID(pSelf, 0xe9f)
STOP

	if (pSelf->isHidden())
		return 0;
	if (!pSelf->isRoot())
		return 0;

	if (IsVarOp(pSelf))
	{
		if (pSelf->isInRow())
			return 0;//already
		HOP pOpRef(GetOpToBindWith(pSelf));
		if (!pOpRef)
			return 0;
		pOpRef = PrimeOp(pOpRef);//maybe callout
		if (pOpRef->isInRow())
		{
			const PathOpList_t &l(PathOf(pOpRef)->ops());
			PathOpList_t::Iterator i(l, INSPTR(pOpRef));
			for (++i; i; i++)
			{
				pOpRef = PRIME(i.data());
				if (!pOpRef->isHidden() && pOpRef->isRoot() && !pOpRef->isInRow())
					break;
				pOpRef = HOP();
			}
		}
		if (!pOpRef)
			return 0;
		if (CanBeBound(pOpRef, pSelf) == 0)
		{
			PrintError() << "Bind failed - " << LocalName(LocalRef(pSelf)) << " used outside of block boundary" << std::endl;
			return 0;
		}
		assert(pOpRef->isRoot());//?
		pSelf->setInRow(true);
		return 1;
	}
	//	return ToggleInRowStatement(pSelf);

	const PathOpList_t &l(PathOf(pSelf)->ops());
	PathOpList_t::Iterator i(l, INSPTR(pSelf));
	for (++i; i; i++)
	{
		HOP pOp(PRIME(i.data()));
		if (pOp->isHidden() || !pOp->isRoot())
			continue;

		//if moving op with var under a loop, make sure the var is ref'd only in that loop
		HOP pVarOp(IsBoundToVar(pSelf));
		if (pVarOp)
		{
			HPATH pIfPath(CheckForOrWhileCondition(pOp));
			if (pIfPath)
			{
				PathOpTracer_t tr;
				LocalsTracer_t an(*this, tr);
				if (an.CheckUsageBoundaryPath(pVarOp, pIfPath))
				{
					pSelf->setInRow(true);
					return 2;
				}
				PrintError() << "Bind failed - " << LocalName(LocalRef(pVarOp)) << " used outside of block boundary" << std::endl;
				return 0;//?
			}
		}

		pSelf->setInRow(true);
		return 1;
	}

	return 0;
}

void FuncInfo_t::SetFuncStatusFinished() const
{
	//warning: current status shuld be greater!
	ObjFlagsType &f(FuncDefPtr()->flags());
	f &= ~FDEF_STATUS_MASK;
	f |= FDEF_DC_FINISHED_OK;
}

int FuncInfo_t::CanBeBound(CHOP pOp, CHOP pOpVar) const
{
	HPATH pIfPath(CheckForOrWhileCondition(pOp));
	if (!pIfPath)
		return -1;//n/a
	PathOpTracer_t tr;
	LocalsTracer_t an(*this, tr);
	if (an.CheckUsageBoundaryPath(pOpVar, pIfPath))
		return 1;
	return 0;//?
}


int FuncInfo_t::MergeWith(HOP pSelf, HOP pOp) const//this offset must be less than pOp's one
{
	if (pSelf->OpC() != pOp->OpC())
		return 0;
//	if (OpSize() != pOp->OpSize())//?
//		return 0;
	if ((pSelf->OpType() & 0xF0) || (pOp->OpType() & 0xF0))
		return 0;
	if (pSelf->OpSize() + pOp->OpSize() > sizeof(pSelf->m_disp))//?sizeof(ui64))
		return 0;

	if (pSelf->IsScalar())
	{
		value_t v;
		memcpy((char *)&v, &pSelf->m_disp/*?&ui64*/, pSelf->OpSize());
		memcpy((char *)&v + pSelf->OpSize(), &pOp->m_disp/*&pOp->ui64*/, pOp->OpSize());
		pSelf->m_disp = v.ui32;//?ui64 = v.ui64;
	}
	else 
	{
		if (pSelf->Offset0() + pSelf->OpSize() != pOp->Offset0())
			return 0;

		if (pSelf->IsIndirect())
		{
			if (!LocalRef(pSelf))
			{
				if (pSelf->OpOffsU() != pOp->OpOffsU())
					return 0;
				if (pSelf->OpSize() != pOp->OpSize())
					return 0;
			}
			else 
			{
				/*if (0)
				if (IsLo cal())
				{
				}
				else
				{
					if (fieldRef() != pOp->fieldRef())
						return 0;
				}*/
			}
		}
	}

	pSelf->SetOpSize(pSelf->Size() + pOp->Size());
	return 1;
}

int FuncInfo_t::ToggleInRowStatement(HOP pSelf) const
{
	//if (!pSelf->isRoot())
		//return 0;

	const PathOpList_t &l(PathOf(pSelf)->ops());

	bool bAll(false);
	int iCount(0); 

	PathOpList_t::Iterator i(l, INSPTR(pSelf));
	if (!PRIME(i.data())->isInRow())//first guess - should we undo
	{
		//get the actual op to be inrow'ed
		do {
			if (IsActual(PRIME(i.data())))
				break;
			assert(!PRIME(i.data())->isInRow());//inrow ops must start from the actual
			i++;
		} while (i);

		if (!i)
			return 0;

		if (!PRIME(i.data())->isInRow())//selected op is above inrow'd sequence but below actual op - UNDO request!
		{
			//get the next actual op to stop at
			PathOpList_t::Iterator j(i);
			do {
				j++;
			} while (j && !PRIME(IsActual(j.data())));

			if (j)//no actual ops below - try UNDO
			{
				//put ops inrow until the next actual
				while (i != j)
				{
					PRIME(i.data())->setInRow(true);
					iCount++;
					i++;
				}

				return (iCount != 0) ? 1 : 0;
			}
		}

		/*for (; i; i++)
		{
			HOP pOp(i.data());
			if (pOp != pSelf)
			{
				if (!pOp->isInRow())
				{
					if (pOp->isRoot() && !pOp->isHidden())
					{
						if (iCount > 0)
							return 1;
						pSelf = pOp;
						bAll = true;
						break;//we may require to backup if there is no a stopping statement in a path
					}
				}
				else
					return 1;
			}
			pOp->setInRow(true);
			iCount++;
		}*/
	}
	else
	{
		//if selected op is actual - undo down, otherwise go up and find an actual op to start with
		PathOpList_t::ReverseIterator j(l, i.data());
		while (!PRIME(IsActual(j.data())))
		{
			assert(PRIME(j.data())->isInRow());
			j++;
			assert(j);
		}
		i = PathOpList_t::Iterator(l, j.data());
	}
	
	// UNDO section

	assert(i && IsActual(PRIME(i.data())));
	if (!PRIME(i.data())->isInRow())
	{
		//the inrow'd sequence must be right above this op
		PathOpList_t::ReverseIterator j(l, i.data());
		j++;
		if (!PRIME(j.data())->isInRow())
			return 0;//nope
		
		//get the first op in inrow sequence - UNDO only the top one
		do {
			i.assign(j.data());
			j++;
		} while (j && PRIME(j.data())->isInRow());

		assert(i && IsActual(PRIME(i.data())));
	}
	
	//perform undo down
	do
	{
		PRIME(i.data())->setInRow(false);
		iCount++;
		i++;
		assert(i);
	} while (!IsActual(PRIME(i.data())));


/*	if (bAll)// || !pSelf->isInRow())
	{
		for (OpList_t::ReverseIterator i(l, pSelf); i; i++)
		{
			HOP pOp(i.data());
			if (pOp == pSelf)
				continue;
			if (!pOp->isInRow())
			{
				//assert(!pOp->isRoot());
				break;
			}
			pOp->setInRow(false);
			if (--iCount == 0)
				break;
		}
	}
	else
	{
		HOP pOp2(nullptr);
		for (OpList_t::ReverseIterator i(l, pSelf); ++i;)
		{
			HOP pOp(i.data());
			if (!pOp->isInRow())
			{
				pOp2 = l.Next(pOp);
				break;
			}
		}
		if (!pOp2)
			pOp2 = l.Head();
		iCount = 0;
		for (OpList_t::Iterator j(l, pOp2); j; j++)
		{
			HOP pOp(j.data());
			if (!pOp->isInRow())
				break;
			if (pOp->isRoot() && !pOp->isHidden())
				if (iCount++ == 1)
					break;
			pOp->setInRow(false);
		}
	}*/

	return (iCount != 0) ? 1 : 0;
}


//searches for op which holds reference to jump table
//called by op with switch jump
FieldPtr FuncInfo_t::GetDataSwitch(CHOP pSelf) const
{
	assert(IsGoto(pSelf));
	if (IsAddr(pSelf))
		return nullptr;

CHECK(OpNo(pSelf) == 383)
STOP

//	if (ppOpIndex)
//		*ppOpIndex = 0;

	bool bIndir = false;
	HOP pOp = pSelf;
	for (;;)
	{
		if (pOp->IsIndirect())
		{
			bIndir = true;
			if (pOp->IsIndirectB())
			{
				if (pOp->m_disp)
				{
					Locus_t loc;
					FieldPtr pField(FindFieldInSubsegs(PrimeSeg(), pOp->m_disp, loc));
					if (pField)
					{
						//if (ppField)
							//*ppField = pField;
						//return Lab elPath(pField);
						return pField;
					}
				}

				if (!pOp->XIn())
				{
					ExprCacheEx_t aExprCache(PtrSize());
					PathOpTracer_t tr;
					AnlzXDepsIn_t anlzxdepsin(*this, tr, pOp, aExprCache);
					anlzxdepsin.ScanXDepsBase( true );
					//!pOp->ScanXDepsBase(true);
				}
				assert(pOp->m_xin.check_count(1) == 0);
				pOp = pOp->XIn()->data();
			}
			else
				return nullptr;
		}
		else
		{
			if (!IsPrimeOp(pOp) || IsGoto(pOp))
			{
				if (!pOp->XIn())
				{
					ExprCacheEx_t aExprCache(PtrSize());
					PathOpTracer_t tr;
					AnlzXDepsIn_t anlzxdepsin(*this, tr, pOp, aExprCache);
					anlzxdepsin.ScanXDeps0(PrimeOp(pOp));
				}
				if (!pOp->XIn())
					return nullptr;
				pOp = pOp->XIn()->data();
			}
			else
			{
				if (ActionOf(pOp) == ACTN_ADD || ActionOf(pOp) == ACTN_SUB)
				{
//					HOP pOpIndex = pOp->arg2()->GetJumpTableIndexOp();
//					if (pOpIndex && ppOpIndex)
//						*ppOpIndex = pOpIndex;
				}
				else if (ActionOf(pOp) == ACTN_MOV)
				{
				}
				else
				{
					return nullptr;
					assert(0);//?
				}
				pOp = pOp->arg1();
			}
		}

		if (LocalRef(pOp))
			break;
	}

	//assert(0);
	/*?	if (bIndir && LocalRef(pOp) && IsData(LocalRef(pOp)))
	{
		if (ppField)
			*ppField = LocalRef(pOp);
		return pOp;
	}*/

	return nullptr;
}

TypePtr FuncInfo_t::OwnerProc() const
{
	return DcInfo_t::OwnerProc(DockField());
}

TypePtr FuncInfo_t::OwnerSeg() const
{
	FieldPtr pField(DockField());
	if (pField)
		return ProjectInfo_t::OwnerSeg(pField->owner());
	return nullptr;
}

/*TypeProc_t *FuncInfo_t::OwnerFuncPtr() const
{
	TypePtr iFunc(OwnerFuncRef());
	if (iFunc)
		return iFunc->type Proc();
	return nullptr;
}*/

FieldPtr FuncInfo_t::DockField() const
{
	return DcInfo_s::DockField(FuncDefPtr());
}

REG_t FuncInfo_t::REG(CHOP pSelf) const
{
	REG_t r;
	r.m_siz = pSelf->OpSize();
	if (!r.m_siz)
		r.m_siz = OPSZ_BYTE;
	r.m_ofs = pSelf->Offset0();
	return r;
}

REG_t FuncInfo_t::REGex(CHOP pSelf) const
{
	if (pSelf->IsIndirectB())
	{
		assert(pSelf->SSID() != SSID_LOCAL);
		REG_t r(pSelf->OpOffsU(), PtrSize());
		return r;
	}
	return REG(pSelf);
}

uint32_t FuncInfo_t::BitMask(CHOP pOp, bool bBase) const
{
	uint32_t mask;
	if (bBase && pOp->IsIndirectB())
	{
		//if (pOp->IsIndirectOf(SSID_CPUREG))
			mask = ~((~0) << mrDC.PtrSize());
		//else if (pOp->IsIndirectOf(SSID_AUXREG))
			//mask = ~((~0) << mrDC.PtrSize());
		//else
			//assert(false);
	}
	else if (pOp->isCPUSW())
		mask = CPUFlags(pOp);
	else if (pOp->OpC() == OPC_FPUSW)
		mask = FPUFlags(pOp);
	else
		mask = ~((~0) << pOp->OpSize());//1maskbit == 1Byte of size
	return mask;
}

MyString FuncInfo_t::MsgPStackBadValue(CHOP pSelf) const
{
	return MyStringf("Can't determine stack top value at L%s", StrNo(pSelf).c_str());
}

MyString FuncInfo_t::MsgPStackRetMismatch(CHOP pSelf) const
{
	assert(IsRetOp(pSelf));
	ProtoInfo_t TI(*this);
	int argsSize(TI.StackIn());
	int releaseSize(pSelf->pstackDiff() - RetAddrSize());//minus sizeof ret addr
	return MyStringf("Number of released bytes (%d) on program stack does not match a size of function parameters (%d) at L%s", releaseSize, argsSize, StrNo(pSelf).c_str());
}

MyString FuncInfo_t::MsgPStackRetNotReset(CHOP pSelf) const
{
	assert(IsRetOp(pSelf));
	return MyStringf("Program stack top did not reset (to zero) at return point L%s", StrNo(pSelf).c_str());
}

MyString FuncInfo_t::MsgStackTopNotConverged(CHOP pSelf, CHOP pOpPr) const
{
	MyString s1(Int2Str(StackOut(pOpPr), I2S_HEX | I2S_SIGNED));
	MyString s2(Int2Str(pSelf->pstackIn(), I2S_HEX | I2S_SIGNED));
	if (!PathOf(pSelf)->isAnalized())
		s2 = "?";
	return MyStringf("Program stack top did not converge with transition from L%s (S=%s) to L%s (S=%s)",
		StrNo(pOpPr).c_str(), s1.c_str(),
		StrNo(pSelf).c_str(), s2.c_str());
}

MyString FuncInfo_t::MsgFpuTopNotConverged(CHOP pSelf, CHOP pOpPr) const
{
	MyString s1(NumberToString((int)FpuIn(pOpPr)));
	MyString s2(NumberToString((int)FpuIn(pSelf)));
	if (!PathOf(pSelf)->isAnalized())
		s2 = "?";
	return MyStringf("FPU stack top did not converge with transition from L%s (F=%s) to L%s (F=%s)",
		StrNo(pOpPr).c_str(), s1.c_str(), 
		StrNo(pSelf).c_str(), s2.c_str());
}

MyString FuncInfo_t::MsgFpuTopNotConverged(CHOP pSelf) const
{
	MyString s(NumberToString((int)FpuIn(pSelf)));
	MyString s2(NumberToString(mrFuncDef.getFStackPurge()));
	return MyStringf("FPU stack top did not converge at exit from L%s (F=%s) to F(exit)=%s",
		StrNo(pSelf).c_str(), s.c_str(),
		s2.c_str());
}

int FuncInfo_t::CheckAlreadyAnalized(CHOP pSelf, CHOP pOpPr) const
{
//?	if (!IsAnalized())
//?		return -1;

	if (CheckStackBreak(pSelf, pOpPr) != 0)
	{
		mrMain.setFlags(adcui::DUMP_STACKTOP);
		//PrintError() << "[" << OwnerFuncName() << "] " << MsgStackTopNotConverged(pSelf, pOpPr) << std::endl;
		PrintError() << MsgStackTopNotConverged(pSelf, pOpPr) << std::endl;
		return 0;
	}

	if (CheckFStackBreak(pSelf, pOpPr) != 0)
	{
		mrMain.setFlags(adcui::DUMP_FPUTOP);
		//PrintError() << "[" << OwnerFuncName() << "] " << MsgFpuTopNotConverged(pSelf, pOpPr) << std::endl;
		PrintError() << MsgFpuTopNotConverged(pSelf, pOpPr) << std::endl;
		return 0;
	}

	return 1;
}

HPATH  FuncInfo_t::AttachToPath(HOP rSelf, /*TypeProc_t *pFunc, */HPATH pPathPr) const
{
	assert(IsPrimeOp(rSelf));

	HPATH pPath;
	if (!pPathPr)
		pPath = AddPath(NewPath());
	else
		pPath = pPathPr;

	LinkOpTail(pPath, rSelf);

	BlockTyp_t blkType(CheckPathBreak(rSelf));
	if (blkType != BLK_NULL)
	{
		SetPathType(PathOf(rSelf), blkType);
		return HPATH();
	}

	return PathOf(rSelf);
}

FieldPtr FuncInfo_t::FindCalleeArg(HOP pSelf) const
{
	assert(IsCallArg(pSelf));

	HOP pOpCall = PrimeOp(pSelf);
	GlobPtr iCallee(GetCalleeFuncDef(pOpCall));
	if (!iCallee)
		return nullptr;

	//FuncInfo_t rCallee(DcRef(), *iCallee);// dc().FindFileDefOf(*pfDef));

	CallingConv_t CC(*this, FuncDefPtr());
	for (FuncCCArgsCIt<> i(*iCallee->typeFuncDef(), CC); i; ++i)
	{
		Arg2_t arg((OPC_t)i.ssid(), i.offset(), i.size());
		if (IsCallOpConformantWithArg(pSelf, arg))
			return (FieldPtr)i.field();
	}

	return nullptr;
}

bool FuncInfo_t::IsLocalOp(CHOP pSelf) const
{
	if (!IsCodeOp(pSelf))
		if (!IsArgOp(pSelf))
			return false;
	if (pSelf->SSID() == SSID_LOCAL)
		return true;
//	if (LocalRef(pSelf))//!!!
//		if (LocalRef(pSelf)->IsL ocal())
	//		return 1;
	return false;
}

bool FuncInfo_t::IsGlobalOp(CHOP pSelf) const
{
	return pSelf->isGlobal_();
}

bool FuncInfo_t::IsLocalOp2(CHOP pSelf) const
{
	if (!IsCodeOp(pSelf))
		if (!IsArgOp(pSelf))
			return 0;
	if (LocalRef(pSelf))
	{
		if (IsStackLocal(LocalRef(pSelf)))
			return 1;
	}
	else
	{
		if (pSelf->OpC() == OPC_LOCAL)
			return 1;
		if (pSelf->IsIndirectOf(SSID_CPUREG))
			if (IsStackPtrB(pSelf))
				return 1;
	}
	return 0;
}

bool FuncInfo_t::HasLocalVars() const
{
	return mrFuncDef.locals() != nullptr;
}

FieldMap& FuncInfo_t::LocalVarsMap() 
{
	return mrFuncDef.locals()->typeStruc()->fields();
}

const FieldMap& FuncInfo_t::LocalVarsMap() const
{
	return mrFuncDef.locals()->typeStruc()->fields();
}

/*bool FuncInfo_t::IsMineLabel(CFieldPtr pLabel) const
{
	assert(IsLa bel(pLabel));
	return (pLabel->ownerProcPvt() == OwnerFuncPtr());
}*/

/*bool FuncInfo_t::IsData(CFieldPtr pSelf)
{
	return pSelf->IsData();
}*/

/*bool Field_t::IsData() const
{
	CFieldPtr pSelf(this);
//?	if (FuncInfo_t::IsLa bel(pSelf))
//?		return false;//label?
	if (!pSelf->owner())
	{
		assert(pSelf->isExported());
		return true;//exit labels?
	}
	//assert(pSelf->owner());
	if (pSelf->isLocal())
		return true;//local
	if (pSelf->owner()->type Proc())
		return true;//globals inside of func
	if (pSelf->owner()->typeSeg())
		return true;//global
	assert(0);
	return false;
}*/

bool FuncInfo_t::IsCallArgEx(HOP pSelf) const
{
	if (!pSelf->IsIndirect())
		return false;
	if (!IsCallArg(pSelf))
		return false;
	HOP pOpRoot = PrimeOp(pSelf);
	GlobPtr ifDef(GetCalleeFuncDef(pOpRoot));
	if (!ProtoInfo_s::IsFuncVarArged(ifDef))
		return false;

	ProtoInfo_t rCallee(DcRef(), ifDef);// , FindFileDefOf(ifDef));

	int nOffs = pSelf->Offset0() - StackTop(pOpRoot);
	if (nOffs < rCallee.StackIn())
		return false;

	return true;
}

bool FuncInfo_t::IsCallOpConformantWithArg(CHOP pOp, const Arg2_t &arg) const
{
	HOP pOpCall(PrimeOp(pOp));

	//		assert(OpSize() == pArg->OpSize());
	if (pOp->IsIndirect() && arg.ssid() == SSID_LOCAL)//locals only!!
	{
		int nStackTop = StackTop(pOpCall);
		int retaddrsz = RetAddrSize();

		int offset = pOp->Offset0() - nStackTop;
//		offset += retaddrsz;
		/*			if (pField->m_pData)
		{
		if (offset == pField->m_pData->m_nOffset)
		return pArg;
		}
		else
		{
		assert(pField->IsStackPtrB());
		if (offset == pField->m_disp)
		return true;
		}
		*/
		if (offset == arg.offs())
			return true;
	}
	else if (pOp->OpC() == OPC_FPUREG)
	{
		int ftop = FpuIn(pOpCall) * FR_SLOT_SIZE;
		int offset = pOp->Offset0() - ftop;
		if (offset == arg.offs())
			if (pOp->OpSize() == arg.size())
				return true;
	}
	else if (pOp->OpC() == (OPC_t)arg.ssid())
	{
		int ofs1 = pOp->Offset0();
		int ofs2 = arg.offs();
		//			if (pField->SSID() == OPC_FPUREG)
		//				ofs2 += nFpuTop;
		if (ofs1 == ofs2)
			if (pOp->OpSize() == arg.size())
				return true;
	}
	else
	{
		//assert(0);
	}
	return false;
}

HPATH  FuncInfo_t::AddPath(HPATH pPath) const
{
	assert(pPath);
//	unsigned zzz = pPath.memRef();
//	HPATH pPath = pOp->Path();
//	if (pPath)
//	{
//		assert(isMineOp(pOp));
//		return pPath;
//	}
/*	if (!pPath)
	{
		pPath = Ne wPath();
//	CHECK(pPath.memRef() == 33)
//		STOP

	}
	else if (pPath->isLinked())
		pPath->Parent()->UnlinkChild(pPath);*/
	
	HPATH pPathTail(mrPathTree.pathTail());
	if (pPathTail)
		TreeInsertChildBefore(HPATH(mrFuncDef.Body()), pPath, pPathTail);
	else
		TreePushChildBack(HPATH(mrFuncDef.Body()), pPath);

	//pPath->SetParent(Body());
	return pPath;
}

HPATH FuncInfo_t::createBodyPath() const
{
	PathPtr pBody(mrFuncDef.Body());
	if (pBody)
		return pBody;
	HPATH hBody(NewPath());//ready made HPATH
	hBody->setFlags(BLK_BODY);
	mrPathTree.m.AddRoot(hBody);
	return hBody;
}

HPATH FuncInfo_t::CreateHeadPath() const
{
	PathPtr pPath(PathHead());
	if (pPath)
		return pPath;

	createBodyPath();

	HPATH hPath(NewPath());//ready made HPATH
	hPath->setFlags(BLK_ENTER);
	TreeAddChild(HPATH(mrFuncDef.Body()), hPath, true);//at the head
	hPath->setAnalized(true);
	return hPath;
}

HPATH FuncInfo_t::CreateTailPath() const
{
	PathPtr pPath(mrPathTree.pathTail());
	if (pPath)
		return pPath;

	createBodyPath();

	HPATH hPath(NewPath());//ready made HPATH
	hPath->setFlags(BLK_EXIT);
	TreeAddChild(HPATH(mrFuncDef.Body()), hPath, false);//at the tail
	hPath->setAnalized(true);
	return hPath;
}

RegMaskType FuncInfo_t::GetSpoiledRegsMask(CGlobPtr g, bool retsOnly) const
{
	if (g)
	{
		if (!g->func())
			return 0;

		const FuncDef_t& rf(*g->typeFuncDef());

		//check rets and spoilt
		RegMaskType spoiltRegsMask(0);
		for (FuncRetsCIt it(rf); it; ++it)//goes towards lower keys
		{
			CFieldPtr pField(VALUE(it));
			if (order(pField) < LOCAL_ORDER_RET_LOWER)
			{
				if (retsOnly)
					break;
				if (!(order(pField) > LOCAL_ORDER_ARG_UPPER))
					break;
			}
			if (SSIDx(pField) != SSID_CPUREG)
				continue;
			REG_t r(address(pField), pField->OpSize());
			RegMaskType m(BITMASK(pField->OpSize()));
			m <<= address(pField) & 0xFF;//?
			spoiltRegsMask |= m;
		}

		if (spoiltRegsMask != 0 || !ProtoInfo_t::IsStub(g))
			return spoiltRegsMask;
	}

	return ToRegMask(mrFE.spoilt_regs_def);
}

int FuncInfo_t::CheckCallout(HOP pSelf) const
{	return 0;
	assert(IsCallOutOp(pSelf));
	HOP pOpCall = PrimeOp(pSelf);
	GlobPtr ifDef = GetCalleeFuncDef(pOpCall);
	if (!ifDef)//?
		return 0;
	assert(ifDef);
	FuncInfo_t rCallee(DcRef(), *ifDef);// dc().FindFileDefOf(*pfDef));
	return rCallee.RegisterExitField(pSelf, pOpCall) != 0;
}

int FuncInfo_t::Compare(HOP pSelf, HOP pOp) const
{
	assert(IsPrimeOp(pSelf));
	assert(IsPrimeOp(pOp));
	assert(!IsCall(pSelf));//fix later

	if (ActionOf(pSelf) != ActionOf(pOp))
		return 0;

	//compare l-ops
	if (!EqualTo(pSelf, pOp))
		return 0;

	//compare r-ops
	OpList_t::Iterator it1(pSelf->argsIt());
	OpList_t::Iterator it2(pOp->argsIt());
	for (;  it1 && it2; it1++, it2++)
	{
		if (!EqualTo(it1.data(), it2.data()))
			return 0;
	}

	if (!it1 && !it2)
		return 1;
	return 0;
}

bool FuncInfo_t::EqualTo(CHOP pSelf, CHOP p) const
{
//	assert(!IsD ata());
//	assert(!p->IsD ata());

	if (pSelf->OpC() != p->OpC())
		return false;
	
	SSID_t ssid(pSelf->SSID());

	if (pSelf->IsIndirect())
	{
		if (LocalRef(p) != LocalRef(pSelf))
			return false;
		if (p->OpSize() != pSelf->OpSize())
			return false;
		if (p->OpOffs() != pSelf->OpOffs())
			return false;

		if (!LocalRef(pSelf))
		{
			if (pSelf->OpSeg() != p->OpSeg())
				return false;
		}
		if (pSelf->m_disp != p->m_disp)
			return false;
	}
	else if (Storage(ssid).isRegister())
	{
//		if (OpC() == OPC_FPUREG)
//		{
//			if (Offset0() != p->Offset0())
//				return false;
//		}
//		else if (OpC() == OPC_CPUREG)
//		{
		if (pSelf->Offset0() != p->Offset0())
				return false;
		if (pSelf->OpSize() != p->OpSize())
				return false;
//		}
	}
	else
	{
		if (/* (OpSize() != p->OpSize())
			||*/ (pSelf->OpOffs() != p->OpOffs())
			|| (OpDisp(pSelf) != OpDisp(p)))
			return false;
	}

	return true;
}

void FuncInfo_t::CopyTo(HOP pSelf, HOP pOp) const//simple copy
{
	pOp->m_opc = pSelf->m_opc;
	pOp->m_optyp = pSelf->m_optyp;
	pOp->mOffs = pSelf->mOffs;
	SetLocalRef(pOp, LocalRef(pSelf));
	pOp->m_disp = pSelf->m_disp;//?pOp->ui64 = ui64;
}

void FuncInfo_t::InitFrom(HOP pSelf, HOP pOther) const
{
	pSelf->Setup4(pOther->OpC(), pOther->m_optyp, pOther->OpOffs(), pOther->m_disp);
	pSelf->m_disp0 = pOther->m_disp0;
	pSelf->SetOpSeg(pOther->OpSeg());

	FieldPtr pFieldRef(LocalRef(pOther));
	if (pFieldRef)
		AddLocalRef(pFieldRef, pSelf);
	else
		SetLocalRef(pSelf, nullptr);
}

int	FuncInfo_t::GetVarArgSize(HOP pSelf) const
{
	assert(IsCall(pSelf));
	GlobPtr ifDef(GetCalleeFuncDef(pSelf));
	if (!ifDef)
		return 0;

	if (!ProtoInfo_s::IsFuncVarArged(ifDef))
		return 0;

	ProtoInfo_t rCallee(mrDC, ifDef);// , mrFileDef);

	//get last stack arg
	HOP pArgLast = HOP();
	for (OpList_t::Iterator i(pSelf->argsIt()); i; i++)
	{
		HOP pOp(i.data());
		if (!pOp->IsIndirect())
			continue;
		assert(!pArgLast || pArgLast->Offset0() < pOp->Offset0());
		pArgLast = pOp;
	}

	if (!pArgLast)
		return 0;//???

	//get actual stackin
	int nStackIn = pArgLast->Offset0() - StackTop(pSelf) + pArgLast->OpSize();
	int nStackInExtra = nStackIn - rCallee.StackIn();
	assert(nStackInExtra >= 0);
	return nStackInExtra;
}

HPATH FuncInfo_t::GetJumpTargetPath(CHOP pSelf) const
{
	//if (!(pSelf->IsGoto() && IsAddr(pSelf))
	//return nullptr;

	HPATH pPath(PathRef(pSelf));
	if (pPath)
		return pPath;

	if (pSelf->m_disp == 0)//is it a jump to pseudo exit label?
	{
		HPATH pTail(mrPathTree.pathTail());
		if (pTail)
		{
			assert(pTail->Type() == BLK_EXIT);
			for (XOpList_t::Iterator i(PathOpRefs(pTail)); i; i++)
			{
				HOP pOp(i.data());
				if (pOp == pSelf)
					return pTail;
			}
		}
	}
	return HPATH();
}

unsigned FuncInfo_t::StorageFlags(CHOP pSelf) const
{
	return Storage(pSelf->SSID()).flags;
}

bool FuncInfo_t::IsMineOp(CHOP pOp) const
{
	if (GetOwnerBody(pOp) != mrFuncDef.Body())
		return false;
	return true;
}

bool FuncInfo_t::IsRetOp(CHOP pSelf) const
{
	if (IsDataOp(pSelf) )
		return false;

	if (IsGoto(pSelf) && PathRef(pSelf))//IsAddr(pSelf))
	{
		HPATH pPath(GetJumpTargetPath(pSelf));
		if (pPath && pPath->Type() == BLK_EXIT)
			return true;
	}

	return false;
}

bool FuncInfo_t::IsBadRetOp(CHOP pSelf) const
{
	assert(IsRetOp(pSelf));
	assert(pSelf->IsIndirect());
	return pSelf->m_disp != 0;
}

/*bool FuncInfo_t::IsRet(CFieldPtr pSelf) const
{
	//just find it in the list of ret fields
	for (FuncRetsCIt i(mrFuncDef); !i.isAtEnd(); ++i)
	{
		CFieldPtr pField(VALUE(i));
		if (pField == pSelf)
			return true;
	}
	return false;
}*/


FieldPtr FuncInfo_t::GetCalleeDockField(CHOP pSelf) const
{
	assert(!IsCallIntrinsic(pSelf));

	HOP pOp = pSelf;
	if (IsCallOutOp(pSelf))
		pOp = PrimeOp(pSelf);

	if (!IsCall(pOp))
		return nullptr;

	CFieldPtr pField(FindFieldRef(pOp));
	if (!pField)
		return nullptr;

	CTypePtr pType(pField->type());
	if (!pType->typeProc())//some procedures may not start with entry label?
	{
		if (pType->typeCode())//an entry label at func's address
		{
			TypePtr iOwner(pField->owner());
			assert(iOwner);
			if (!iOwner->typeProc())
				return nullptr;
			assert(pField->_key() == iOwner->base());
			pField = iOwner->parentField();
			assert(pField);
		}
	}

	return (FieldPtr)pField;
}

GlobPtr FuncInfo_t::GetCalleeFuncDef(CHOP pSelf) const
{
	assert(IsCall(pSelf));
	if (IsCallIntrinsic(pSelf))
	{
		GlobPtr iGlob(dc().getIntrinsic(pSelf->m_disp));
		if (iGlob && iGlob->typeFuncDef())
			return iGlob;
		return nullptr;
	}
	if (IsCallDirect(pSelf))
		return GetFuncAtCall(pSelf);

	FieldPtr pField(FindFieldRef(pSelf));
	if (!pField)
		return nullptr;
	if (!pField->isTypeImp())
		return IsPtrToCFunc(pField);
	CFieldPtr pExpField(ToExportedField(pField));
	if (!pExpField)
		return nullptr;
	return FuncDefAttached(pExpField);
}

GlobPtr FuncInfo_t::GetCalleeFuncDefEx(CHOP pOp, ExprCacheEx_t& exprCache, PathOpTracer_t& tr) const
{
	GlobPtr iGlob(nullptr);
	HOP pSelf(pOp);
	Out_t* pOut(exprCache.findOpExpr(pSelf));
	if (!pOut)
	{
CHECK(OpNo(pOp) == 144)
STOP
		//ExprCache_t aCache;
		EXPRptr_t expr(*this, pSelf, OpNo(PrimeOp(pSelf)), exprCache);
		expr.enableTraceXIns(true);
		pOut = expr.TracePtr(pSelf, tr, false);

		expr.Simplify(pOut, pSelf, tr);

		exprCache.addOpExpr(pSelf, expr.FuncDefPtr(), pOut, expr.addresses());
	}
	if (pOut && pOut->mpR->isFieldKind())
	{
		FieldPtr pField(pOut->mpR->field());
		if (pField)
		{
			iGlob = IsCFuncOrStub(pField);
			if (!iGlob)
				iGlob = IsPtrToCFunc(pField);
		}
	}
	return iGlob;
}

GlobPtr FuncInfo_t::GetFuncAtCall(CHOP pSelf) const
{
	CFieldPtr pField(GetCalleeDockField(pSelf));
	if (pField)
		return GlobObj(pField);
	return nullptr;
}

SSID_t FuncInfo_t::StorageId(CHOP pSelf) const
{
	bool bIndir = pSelf->IsIndirect();
	bool bAddr = IsAddr(pSelf);
	

	if (!bIndir)
	{
		OPC_t opc = pSelf->OpC();
		SSID_t ssid = (SSID_t)(opc & 0xf);
		if (opc)
			return ssid;
	}
	else
	{
		if (IsLocalOp2(pSelf))
			return SSID_LOCAL;
		if (LocalRef(pSelf))
			return SSID_GLOBAL;
	}
	
	if (IsDataOp(pSelf))
	{
/*?		HOP pIOp = pSelf;
		if (IsStackLocal(GetOwnerData(pIOp)))
			return SSID_LOCAL;*/
		return SSID_GLOBAL;
	}
//?	else if (IsField())
//?		return SSID_FIELD;

	return SSID_NULL;
}



/*int FuncInfo_t::AdjustStackArgs()
{
	LocalsTracer_t an(*this);
	for (OpList_t::Iterator i(mrFuncDef.entryOps()); i; i++)
	{
		HOP pArg(i.data());
		if (pArg->OpC() == OPC_INDIRECT+OPC_CPUREG)
		{
			assert(0);
			assert(IsStackPtrB(pArg));
			an.Make Local(pArg);
		}
	}

	return 1;
}*/

STAGE_STATUS_e FuncInfo_t::AssureArgList() const
{
#if(0)
	FuncProfile_t si;
	m_pfDef->GetArgProfile(si);

	//setup func stackin
	if (Body())
	if (Body()->m_pLocals)
	{
		//FieldPtr pArg = (FieldPtr )m_pLocals->pPrev;
		if (GetStackOut() != 0)//__stdcall
		{
			if (si.stackin > GetStackOut())
			{
				WARNINGMSG("%s: Stack arguments range exceeds stack cleanup value (__stdcall)", nam ex());
			}
			else if (si.stackin < GetStackOut())
				si.stackin = GetStackOut();
		}
	}

	ProtoInfo_t TI(*this);
	TI.CreateArgList(si.GetTempArgs(m_pfDef));
#endif

	PathOpTracer_t tr;
	LocalsTracer_t an(*this, tr);
	for (PathOpList_t::Iterator i(mrFuncDef.entryOps()); i; i++)
	{
		an.MakeLocal(PRIME(i.data()));
//?		pArg->m_nFlags &= ~OPND_NCONF;
	}
	return STAGE_STATUS_e::SKIPPED;
}

/*FieldPtr  LocalsTracer_t::Regist erLocal(int32_t nOffset)
{
	if (nOffset == 0)
		return 0;

	FieldPtr pLocal = Add Local1(mrFuncDef.Body(), OPC_LOCAL, nOffset);

//CHECK(nOffset == -0x1c)
//STOP

	return pLocal;
}*/


void FuncInfo_t::GetArgProfileFromCall(ProtoProfile_t &fp, CHOP hOpCall) const
{
	fp = ProtoProfile_t();//clear

	//fp.extraArgs = ((mrFuncDef.flags() & FDEF_VARIARDIC) != 0);

//	if (!mrFuncDef.hasArgFields())
	//	return;

	int nCount(0);//reg args counter
	for (OpList_t::Iterator i(hOpCall->argsIt()); i; i++)
	{
		HOP pArg(i.data());
		const Op_t &rArg(*pArg);
		//FieldPtr pArg = VALUE(it);

		if (rArg.SSID() == SSID_CPUREG)
		{//ignores this call?

			if (IsStackPtrB(pArg))//[esp]
			{
				fp.applyStackIn(rArg.Offset0() + rArg.OpSize(), mrFE.stackaddrsz);
			}
			else
			{
				REG_t R(rArg.OpOffs(), rArg.OpSize());
				/*?if (IsThisPtrOp(pArg))
				{
					fp.cpuin[nCount] = c;
					continue;
				}*/
				fp.cpuin.push_back(R);
			}
		}
		else if (rArg.SSID() == SSID_FPUREG)
		{
			/*?int f(rArg.OpOffs() + rArg.OpSize());
			f /= FR_SLOT_SIZE;//?
			int g(f - FpuIn(hOpCall));
			if (g > fp.fpuin)
				fp.fpuin = g;*/
			fp.fpuin++;
		}
		else if (rArg.SSID() == SSID_LOCAL)
		{
			int off(rArg.Offset0());
			off -= hOpCall->pstackIn();
			assert(off >= 0);
			off += rArg.OpSize();
			fp.applyStackIn(off, mrFE.stackaddrsz);
		}
		else
		{
			//?assert(false);
		}
	}
}

void FuncInfo_t::GetFuncProfileFromCall(FuncProfile_t &fp, CHOP hOpCall) const
{
	GetArgProfileFromCall(fp, hOpCall);

	const Ins_t &ins(InsRef(hOpCall));

	fp.stackin -= RetAddrSize();

	fp.pstackPurge = ins.PStackDiff();
	fp.pstackPurge -= RetAddrSize();
	fp.pstackPurgeRet = RetAddrSize();

	fp.fpudiff = ins.FStackDiff();
}

ADDR FuncInfo_s::setupKey(uint8_t ssid, int offs, int order)//for args only (or w no storage)
{
	ADDR key(0);
	if (order < 0)//auto
	{
		if (ssid)
		{
			if (ssid == SSID_LOCAL)
			{
				assert(offs > 0);//no vars
				order = LOCAL_ORDER_STACK;
				ssid = 0;
			}
			else//a reg
				order = LOCAL_ORDER_REG;
		}
		else
		{
			ASSERT0;//crash
			//order = LOCAL_ORDER_UNKNOWN;
		}
	}
	else if (order == LOCAL_ORDER_STACK)
	{
		assert(!ssid || ssid == SSID_LOCAL);
		ssid = 0;
	}

	assert(ssid != SSID_LOCAL);

	key |= SET_ARG_ORDER(order);
	key |= SET_ARG_SSID(ssid);
	if (!ssid)// || ssid == SSID_LOCAL)
		key |= (offs & LOCAL_ARG_RANGE);
	else
		key |= (offs & LOCAL_REG_RANGE);
	return key;
}

ri_t FuncInfo_s::toR_t(CFieldPtr pf)
{
	ri_t r;
	r.ssid = ARG_SSID_FROM_KEY(pf->_key());
	r.ofs = ARG_OFFS_FROM_KEY(pf->_key());
	CTypePtr p(ProjectInfo_t::SkipModifier(pf->type()));
	if (p && p->typeSimple())
		r.typ = (OpType_t)p->typeSimple()->optype();
	else
		r.typ = OPTYP_NULL;
	return r;
}

bool FuncInfo_s::IsOverlap(CFieldPtr pSelf, SSID_t ssid2, int nOfs2, int nSize2)
{
	SSID_t ssid1 = SSIDx(pSelf);
	if (ssid1 != ssid2)
		return false;

	int nSize1 = pSelf->size();
	if (!nSize1)
		nSize1 = OPSZ_BYTE;
	if (!nSize2)
		nSize2 = OPSZ_BYTE;
//	assert(nSize1 && nSize2);

	int nOfs1 = address(pSelf);				// + disp;

	return checkoverlap(nOfs1, nSize1, nOfs2, nSize2);
}

FieldPtr FuncInfo_t::FindRetField(CHOP hOpRetPrime, CHOP hOp) const
{
	for (FuncRetsCIt i(mrFuncDef); !i.isAtEnd(); ++i)
	{
		CFieldPtr pField(VALUE(i));
		ri_t r1(toRi2(pField));//no optyp
		if (r1.ssid == SSID_FPUREG)
		{
			r1.siz = OPSIZE(OPTYP_REAL64);
		}
		ri_t r2(hOp->toRi());
		if (r2.ssid == SSID_FPUREG)
		{
			//ADJUST FOR A RET FIELD
			int a(r2.ofs);
			int b = FpuIn(hOpRetPrime) * FR_SLOT_SIZE;
			r2.ofs = int8_t(a - b);//see UpdateRetOps()
			r2.siz = OPSIZE(OPTYP_REAL64);
		}
		//if (hOp->EqualTo(pField))
		if (r1 == r2)
			return (FieldPtr)pField;
	}
	return nullptr;
}

//pOpRef - from caller context
FieldPtr FuncInfo_t::FindCalleeRetField(CGlobPtr iCallee, CHOP pOpRef, CHOP pOpCall) const
{
	assert(pOpRef);
	assert(pOpCall && IsCall(pOpCall));
	const FuncDef_t& rfDef(*iCallee->typeFuncDef());

	//if (!rfDef.hasRetFields())
		//return nullptr;

	for (FuncRetsCIt i(rfDef); !i.isAtEnd(); ++i)
	{
		CFieldPtr pField(VALUE(i));
		if (SSIDx(pField) != (SSID_t)pOpRef->OpC())
			continue;
		int nDiff = 0;
		if (SSIDx(pField) == (SSID_t)OPC_FPUREG)
			nDiff = -FpuIn(pOpCall);

		if (checkoverlap(address(pField), pField->size(), pOpRef->Offset0() + nDiff, pOpRef->OpSize()))
			return (FieldPtr)pField;
	}

	return nullptr;
}

/*
//add pOpRef to func's return ops list
HOP TypeProc_t::RegisterExitOp(HOP pOpRef, HOP pOpCall)
{
	assert((m_nFlags & FNC_DECOMP) >= FNC_DEFINED);

	HOP pOp = m_pfDef->RegisterExitField(pOpRef, pOpCall);
	if (!pOp)
		return 0;

	Update RetOps();
	return pOp;
}
*/


FieldPtr  FuncInfo_t::RegisterExitField(HOP pOpRef, HOP pOpCall)//from caller context!!!
{
	assert(pOpCall && IsCall(pOpCall));

//CHECK(m_pOwnerFunc->parent()->CompName("_vec_len"))
//STOP

	int nDiff = 0;//offset from caller context to callee one
	if (pOpRef->OpC() == OPC_FPUREG)
		nDiff = -FpuIn(pOpCall);

	FieldPtr  pField(FindCalleeRetField(FuncDefPtr(), pOpRef, pOpCall));
	if (pField)
	{
		if (address(pField) == pOpRef->Offset0() + nDiff)
			if (pField->size() == pOpRef->OpSize())
				return pField;
		int sz1 = pField->size();
		int ofs1 = address(pField);
		REG_t r1(ofs1, sz1);

		int sz2 = pOpRef->OpSize();
		int ofs2 = pOpRef->Offset0() + nDiff;
		REG_t r2(ofs2, sz2);
		assert(r2.m_siz < 0x10);

		r1.MergeWith(r2);
		assert(nDiff == 0);

		if (r1.m_siz != sz1)
		{
	//?		assert(0);
			//?pField->Setup3(pOp->OpC(), r1.m_siz, r1.m_ofs);
		}
	}
	else
	{
		ProtoInfo_t TI(*this);
		pField = TI.AddRetField((OpType_t)pOpRef->OpType(), (uint8_t)pOpRef->OpC(), pOpRef->Offset0() + nDiff);
		//HPATH  pPath = mrFuncDef.CreateTailPath();
		//LocalsTracer_t an(mrFuncDef);
		//an.AddLocal 0(pPath, pField);// , pOpRef->OpC());// , pOpRef->Offset0() + nDiff );
	}

	if (ProtoInfo_t::IsFuncStatusFinished(&mrFuncDefRef))
	{
		UpdateRetOps();
		SetChangeInfo(FUI_NUMBERS);//?
	}
	return pField;
}


bool FuncInfo_t::IsOpSavedByCallee(CHOP pOp, CGlobPtr iCallee) const
{
	assert(pOp->OpC() == OPC_CPUREG);
#if(0)
	REG_t q(pOp->OpOffs(), pOp->OpSize());
	for (FuncSpoiltIt it((FuncDef_t &)mrFuncDef, SSID_CPUREG); it; ++it)
	{
		CFieldPtr pField(it.field());
		REG_t r(pField->address(), pField->OpSize());
		if (q.intersects(r))
			return false;
	}
	return true;//no intersection with spoiled regs - SAVED!
#else
	RegMaskType m(BitMask(pOp));
	m <<= pOp->Offset0();
	assert(m);

	RegMaskType fmask(GetSpoiledRegsMask(iCallee, false));
	return (m & (~fmask)) != 0;
#endif

}

void FuncInfo_t::SetChangeFlags(uint16_t f) const
{
//	if (!m_pOwnerFunc)
//		return;//?
	if ((mrFuncDefRef.flags() & f) == f)
		return;//already/still
	mrFuncDefRef.flags() |= f;
	//mrDC.arrFuncDefs.Add(&mrFuncDef);
}

/*int FuncInfo_t::SetSavedRegs(uint32_t nSavedRegs)
{
	if (nSavedRegs == mrFuncDef.m_nSavedRegs)
		return -1;
	if (IsFuncStatusFinished(&mrFuncDefRef))
		return 0;
	mrFuncDef.m_nSavedRegs = nSavedRegs;
	//SetChangeFlags(FDEF_CHGSAVREGS);
	return 1;
}*/

int FuncInfo_t::PathNo(CHPATH pPath) const
{
	OpPtr pOp(PathOps(pPath).Head());
	if (pOp && !IsArgOp(pOp))
	{
		for (; pOp; pOp = NextPrime(pOp))
		{
			if (!IsVarOp(pOp))
				return OpNo(pOp);
		}
	}
	return -1;
}

// UNICODE symbols:
//
// U+23F4	: left		"\xE2\x8F\xB4"
// U+23F5	: right		"\xE2\x8F\xB5"
// U+23F6	: up		"\xE2\x8F\xB6"
// U+23F7	: down		"\xE2\x8F\xB7"

MyString FuncInfo_t::LineInfoStr(CHOP hOp) const
{
	MyString s;
	if (PathType(PathOf(hOp)) != BLK_DATA)
		s = StrNo(hOp);
	else
		s = NumberToString(OpOff(hOp));
/*	if (IsCall(hOp))
		//s += "\xE2\x8F\xB5";//U+23F4 - right
		s += "\xe2\x86\x92";//U+2192 Rightwards Arrow
		//s += "\xe2\x96\xb6";//U+25B6 - Black Right-Pointing Triangle
	*/return s;
}

static void appArrowUp(MyString &s)
{
	//s += '+';
	//s += "\xe2\x86\x91";//U+2193 - upward arrow
	s += "\xe2\x96\xbc";//U+25BC - Black Down-Pointing Triangle
	//s += "\xe2\x96\xbd";//U+25BD - White Down-Pointing Triangle
}

static void appArrowDown(MyString &s)
{
	//s += '-';
	//s += "\xe2\x86\x93";//U+2193 - Downwards Arrow
	s += "\xe2\x96\xb2";//U+25b2 - Black Up-Pointing Triangle
	//s += "\xe2\x96\xb3";//U+25B3 - White Up - Pointing Triangle
}

MyString FuncInfo_t::FTopInfoStr(CHOP hOp, bool bArrows) const
{
	if (!IsAnalized(hOp))
		return "?";
	if (IsVarOp(hOp))
		return "-";
	MyString s;
	int8_t fputop(FpuIn(hOp));
	int8_t d(hOp->fstackDiff());
	if (d < 0)//load
		fputop += hOp->fstackDiff();
	if (bArrows && d < 0)
		appArrowUp(s);
	s += NumberToString((int)fputop);
	if (bArrows && d > 0)
		appArrowDown(s);
	return s;
}

MyString FuncInfo_t::StackTopInfoStr(CHOP hOp, bool bArrows) const
{
	if (!IsAnalized(hOp) || hOp->ins().opnd_BAD_STACKOUT)
		return "?";
	if (IsVarOp(hOp))
		return "-";
	MyString s;
	int d(hOp->pstackDiff());
	if (bArrows && d < 0)
		appArrowUp(s);
	s += Int2Str(hOp->pstackIn(), I2S_HEX | I2S_SIGNED);
	if (bArrows && d > 0)
		appArrowDown(s);
	return s;
}

MyString FuncInfo_t::PathInfoStr(CHOP hOp) const
{
	HPATH hPath(PathOf(hOp));
	assert(hPath);
	int n(PathNo(hPath));
	if (n < 0)
		return "-";
	return NumberToString(n);// TreeIndexEx(hPath));
}

/*int FuncInfo_t::PathNo(HPATH hSelf) const
{
	OpPtr pOpFirst = GetFirstOp(hSelf);
	return (pOpFirst) ? OpNo(pOpFirst) : (-1);
}*///?

FieldPtr FuncInfo_t::PathLabel(CHPATH pPath) const
{
	ADDR va(PathVA(pPath));//pPath->anchor());
	if (va == -1)
		return nullptr;
	/*{
		int iNo(PathNo(pPath));
		if (iNo < 0)
			return nullptr;
		ADDR va(DockAddr() + iNo);
	}*/
	return FindGlobalAtVA(va, FieldIt_Exact, true);
}

TypePtr DcInfo_t::UnmakeMemberMethod(GlobPtr g) const
{
	//FieldPtr pFuncField(DockField());
	TypePtr iClass(g->owner());
	if (!iClass)
	{
	//if (!mrFuncDef.mpThisPtr)
		assert(!ProtoInfo_s::IsThisCallType(g));
		return nullptr;
	}

	ClassInfo_t classInfo(*this, iClass, memMgrGlob());
	//assert(pFuncField);
	assert(!ProtoInfo_s::IsThisCallType(g) || ProtoInfo_s::ThisPtrArg(g)->isConstPtrToStruc() == iClass);
	if (!g->typeFuncDef()->nameless())
	{
		//NamesMgr_t &rNS0(NamespaceInitialized(iClass));
		NamesMgr_t& rNS0(*iClass->typeComplex()->namesMgr());
		ClearTypeName(rNS0, (GlobToTypePtr)g);
	}
	if (!classInfo.RemoveClassMember(g))
		if (!classInfo.RemoveClassVirtualMember(g))
			ASSERT0;
//	mrFuncDef.setThisPtr(nullptr);
	return iClass;
}

HPATH FuncInfo_t::GetLoopWhileBackwardCondition(CHPATH pPathTop) const
{
	if (pPathTop->Type() != BLK_IFWHILE)
		if (pPathTop->Type() != BLK_IFFOR)
			return HPATH();

	HPATH pPath(pPathTop);
	for (;;)
	{
		pPath = pPath->LastChild();
		if (!pPath)
			break;
		if (pPath->Type() == BLK_LOOPDOWHILE)
			return pPath;
	}
	return HPATH();
}

bool FuncInfo_t::IsClassMember() const
{
	return DockField() && DcInfo_t::OwnerScope(DockField());
}

int DcInfo_t::RetAddrSize() const
{
	int sz;
/*	if (mrFuncDefRef.flags() & FDEF_FAR)
		sz = mrFE.ptr_far;
	else*/
		sz = mrFE.ptr_near;
	assert(sz != -1);
	return sz;
}

bool FuncInfo_t::IsFar() const
{
	int sz = RetAddrSize();
	if (sz == mrFE.ptr_far)
		return true;
	return false;
}

/*TypePtr FuncInfo_t::SetClassMembership(char cpuin)
{
	if (mrFuncDefRef.flags() & TYP_FDEF_TEMP)
		return nullptr;

	if (cpuin == '0')//remove
	{
		if (!IsThisCallType())
			return nullptr;//already
		assert(0);
		FieldPtr pThisPtr(ThisPtrArg());
		//mrFuncDef.setThisPtr(nullptr);
		//assert(0);
		//?LocalsTracer_t an(*Owner Proc());
		//?an.CheckLocal0(pThisPtr);
		return nullptr;//1
	}

	FieldPtr pField(FindFieldFromCPUinChar(cpuin));
	if (!pField)
		return nullptr;
	
	assert(0);//?SetConstPtrToStruc(pField, (GlobToTypePtr)&mrFuncDefRef);
	return MakeNonStaticMember(pField);
}*/

HOP FuncInfo_t::FindEntryOp(CHOP pOpRef, HOP * ppArgPrev) const
{
	REG_t rRef(REGex(pOpRef));
	
	REG_t rRefx(rRef);
	rRefx.m_ofs += CalcDispl(pOpRef);//?

	*ppArgPrev = HOP();
	for (PathOpList_t::Iterator i(mrFuncDef.entryOps()); i; i++)
	{
		HOP pArg(PRIME(i.data()));
		assert(IsArgOp(pArg));
//?		assert(pArg->m_pOwnerFuncDef == this);
		assert(!pArg->IsIndirectB());

		if (pArg->SSID() > pOpRef->SSID())
			break;

		if (pArg->SSID() == pOpRef->SSID())
		{
			if (pArg->SSID() == SSID_CPUREG)
			{
				if (FuncInfo_s::Overlap(REG(pArg), rRefx))
					return pArg;
				if (pArg->Offset0() > rRef.m_ofs)// pOpRef->Offset0())
					break;
			}
			else if (pArg->SSID() == SSID_FPUREG)
			{
				if (pArg->Offset0() < 0)
				{
					assert(0);
					return pArg;
				}
				if (FuncInfo_s::Overlap(REG(pArg), rRefx))
					return pArg;
				if (pArg->Offset0() > rRef.m_ofs)//pOpRef->Offset0())
					break;
			}
			else if (IsLocalOp(pOpRef))
			{
				if (rRef.m_ofs <= 0)//arg
					fprintf(STDERR, "Warning: Negative offset for local variable at %d\n", OpNo(PrimeOp(pOpRef)));

				//if ( pArg->IsLo cal() )
				if (pArg->IsIndirect())
				{
					//?				assert(pArg->Offset0() > 0);//arg

					assert(IsMineOp(pOpRef));
					int offs = rRef.m_ofs;// pOpRef->Offset0();
					/*else
					{
						assert(pOpRef->Offset0() < 0);//var?
						offs = pOpRef->Offset0() - StackTop(PrimeOp(pOpRef)) + RetAddrSize();
					}*/

					int d = offs - pArg->Offset0();
					if (d < 0)
					{
						if (rRef.m_siz > -d)//overwrite from beneath
							//assert(false);//fix later
							return pArg;
						//break;//early exit? are entries sorted?
						if (pArg->Offset0() > rRef.m_ofs)//pOpRef->Offset0())
							break;
					}
					else if (d > 0)
					{
						if (pArg->OpSize() > d)//overwrite from above
							//assert(false);//fix later
							return pArg;
					}
					else// if ( d == 0 )
					{
						if (rRef.m_siz > pArg->OpSize())
							if (pArg->m_xin.empty())
								ChangeArgType(mrFuncDef, i, rRef.m_siz);//??? pOpRef->OpType());
						//?AgreeTypes(pArg->m_optyp, pOpRef->m_optyp);
						return pArg;
					}
				}
				else
					ASSERT0;
			}
			else
				ASSERT0;
		}

		*ppArgPrev = pArg;
	}

	return HOP();
}

int FuncInfo_t::ChangeArgType(FuncDef_t &rSelf, PathOpList_t::Iterator &iSelf, uint8_t typ) const
{
//CHECK(GetOwner Func()->CompName("?sub_22016520"))
//STOP
	HOP pSelf(PRIME(iSelf.data()));

	pSelf->SetOpType(typ);
	PathOpList_t::Iterator i(iSelf);
	++i;
	while (i)//ARGS MUST BE SORTED!
	{
		HOP pOp(PRIME(i.data()));
		assert(pOp->OpC() == pSelf->OpC());
		int d = pSelf->Offset0() + pSelf->OpSize() - pOp->Offset0();
		if (d <= 0)
			break;
		if (!pOp->XIn())//d > 0
		{
			++i;
			memMgrEx().Delete(pOp);
		}
		else
			++i;
	}

	return 1;
}


/*TypePtr FuncInfo_t::GetLocalsTop() const
{
	FieldPtr pParentField(DockField());
	if (!pParentField)
		return nullptr;
	TypePtr iUnionTop(pParentField->OwnerComplex());//union 
	assert(iUnionTop->typeUnion());
	assert(!iUnionTop->parentField());
	return iUnionTop;
}*/

void FuncInfo_t::DumpBlocks(FuncDef_t &rSelf, MyStreamBase &ss)
{
	dump(rSelf.pathTree(), ss);
}

int FuncInfo_t::InsertEntryOp(HOP pOp0, HOP pOpPr) const
{
	HPATH pPath(CreateHeadPath());

/*	HOP  pOpPr = nullptr;
	for (HOP  pOp = pPath->Ops();
	pOp;
	pOp = (HOP)pOp->Ne
	xt())
	{
		int opc = pOp->OpC();
		int offs = pOp->Offset0();

		if (opc > pOp0->OpC())
			break;
		if (opc == pOp0->OpC())
		{
			if (pOp0->Offset0() > offs)
				break;
			if (pOp0->Offset0() == offs)
				return 0;
		}
		pOpPr = pOp;
	}*/

	if (!pOpPr)
		LinkOpHead(pPath, pOp0);
	else
		LinkOpAfter(pPath, pOp0, pOpPr);

	return 1;
}


HOP FuncInfo_t::RegisterEntryOp(HOP pOpRef) const
{
	assert(pOpRef);

	REG_t rRef(REGex(pOpRef));

	if (pOpRef->SSID() != SSID_CPUREG) 
		if (pOpRef->SSID() != SSID_FPUREG) 
			if (!IsLocalOp(pOpRef) || (pOpRef->Offset0() < 0))
				return HOP();

//CHECK(m_pOwnerFunc->parent()->CompName("_vec_len"))
CHECK(OpNo(PrimeOp(pOpRef)) == 0)
//CHECK(pOpRef->OpC() == OPC_LOCAL)// && pOpRef->OpOffs() == 8)
STOP

	int offs(0);
	int disp(0);

	HOP pOpPr;
	HOP hArg(FindEntryOp(pOpRef, &pOpPr));
	if (hArg)
		return hArg;
	
	//if (pOpRef->IsIndirect())
	if (pOpRef->OpC() == OPC_LOCAL)
	{
		disp = pOpRef->Offset0();
		offs = 0;
	}
	else
	{
		offs = pOpRef->OpOffs();
	}

	ri_t ri(pOpRef->SSID(), rRef.m_ofs, rRef.m_siz);

	ProtoInfo_t TI(*this);

	FieldPtr pLocal(TI.FindArgFromOp(ri));
	if (!pLocal)
	{
		int dd(CalcDispl(pOpRef));
		pLocal = FindOverlappedLocalArg(pOpRef, dd + pOpRef->Offset0());//looks among var ops only!
	}
	
	if (pLocal)//check if there is a dangling op!
	{
		hArg = FindDanglingOp(pLocal);
		if (hArg)
		{
			//assert(hArg->toRi() == ri);
			if (hArg->m_disp != disp)//?
				return HOP();
			assert(hArg->m_disp == disp);//
			SetupPrimeOp(hArg);
			InsertEntryOp(hArg, pOpPr);
		}
	}

	if (!hArg)
	{
		uint8_t optyp(pOpRef->IsIndirectB() ? MAKETYP_PTR(PtrSize()) : pOpRef->OpType());
		ri_t rix(pOpRef->SSID(), rRef.m_ofs, optyp);// offs, pOpRef->OpType());

		hArg = NewPrimeOp();
		hArg->Setup4(rix, disp);
		InsertEntryOp(hArg, pOpPr);
		if (pLocal)
			AddLocalRef(pLocal, hArg);
	}
	
	if (ProtoInfo_t::IsFuncStatusFinished(&mrFuncDefRef))
	{
		PathOpTracer_t tr;
		LocalsTracer_t an(*this, tr);
		an.CheckLocal0(hArg);
		//SetChangeFlags(FDEF_CHGARGLIST);
	}
	return hArg;
}

void FuncInfo_t::SetupPrimeOp(HOP hOp) const
{
	assert(!hOp->insPtr());
	hOp->setInsPtr(NewRootInfo());
	hOp->ins().mpPrimeOp = hOp;
#ifdef _DEBUG
	hOp->__isPrime = true;
#endif
}

HOP FuncInfo_t::CloneOperation(HOP pSelf, bool bOuts) const
{
	assert(IsPrimeOp(pSelf));

	HOP pOpRoot = NewPrimeOp();
	InitFrom(pOpRoot, pSelf);

	InitFrom(pOpRoot->ins(), pSelf->ins(), pSelf);

	if (bOuts)
		for (XOpList_t::Iterator i(pSelf->m_xout); i; i++)
	{
		HOP pOpOut = i.data();
		if (!IsCallOutOp(pOpOut))
			continue;

		HOP pOp = NewOp();
		InitFrom(pOp, pOpOut);
		pOp->setInsPtr(pOpRoot->insPtr());
		MakeUDLink(pOp, pOpRoot);
	}

	return pOpRoot;
}

/*HOP  FuncInfo_t::RegisterEntryOp(uint8_t cl, uint8_t id, uint8_t typ)//for cpu/fpu reg
{
	Op_t op(OBJ_ID_ZERO);
	op.Setup3(cl, typ, id);
	return RegisterEntryOp(&op);
}*/


HOP FuncInfo_t::FindOpByVA(ADDR addr0) const
{
	ADDR base(DockAddr());
	for (PathTree_t::LeafIterator i((PathTree_t&)mrPathTree.tree()); i; i++)
	{
		HPATH pPath(i.data());
		if (pPath->Type() != BLK_ENTER)
			if (pPath->Type() != BLK_EXIT)
				if (!pPath->ops().IsEmpty())
				{
					HOP rOp1(PRIME(pPath->ops().front()));
					ADDR a1(base + OpNo(rOp1));
					if (a1 <= addr0)
					{
						HOP rOp2(PRIME(pPath->ops().back()));
						ADDR a2(base + OpNo(rOp2));
						if (a2 >= addr0)
						{
							for (PathOpList_t::Iterator j(pPath->ops()); j; j++)
							{
								HOP pOp(PRIME(j.data()));
								ADDR a(base + OpNo(pOp));
								if (a == addr0)
									return pOp;
							}
						}
					}
				}
	}

	return HOP();
}

MyString FuncInfo_t::OwnerFuncName() const
{
	return mrProject.fieldName(DockField());
}

TypePtr FuncInfo_t::LocalVars() const
{
	return mrFuncDef.locals();
}

TypePtr FuncInfo_t::LocalArgs() const
{
	return 0;
}

bool FuncInfo_t::HasArgs() const
{
	return mrFuncDef.hasFields();
}

ADDR FuncInfo_t::DockAddr() const
{
	return DockField()->_key();
}

STAGE_STATUS_e FuncInfo_t::ReCheckRoots() const
{
	if (!testChangeInfo(FUI_ROOTS))
		return STAGE_STATUS_e::SKIPPED;

	HPATH pSelf = mrFuncDef.Body();
	if (pSelf)
	{
		TRACE1("%s: Re-analizing roots...\n", OwnerFuncName().c_str());

		//reset root flag
		for (PathTree_t::LeafIterator i(mrPathTree.tree()); i; i++)
		{
			HPATH pPath(i.data());
			for (PathOpList_t::Iterator j(pPath->ops()); j; j++)
			{
				HOP pOp(PRIME(j.data()));
				Ins_t &ri(InsRef(pOp));
				ri.opnd_ROOTPREV = ri.opnd_REDUCED;
				ri.setRoot(true);//reset
			}
		}

		int nCount;
		int nFailed;
		do {
			nCount = 0;
			nFailed = 0;

			for (PathTree_t::LeafIterator i(mrPathTree.tree()); i; i++)
			{
				HPATH pPath(i.data());
				for (PathOpList_t::Iterator j(pPath->ops()); j; j++)
				{
					HOP pOp(PRIME(j.data()));
					if (pOp->ins().isRoot() && !pOp->ins().isRootPr())
					{
						//initially was un-rooted
						PathOpTracer_t tr;
						AnlzRoots_t anlz(*this, tr, pOp, true, true);
						int res = anlz.TurnRoot_Off();
						if (res == 1)//so try to unroot it again
							nCount++;
						else if (res == 0)//failed
							nFailed++;
					}
				}
			}

		} while (nCount);

		if (nFailed > 0)
		{
			SetChangeInfo(FUI_LOCALS);
			fprintf(stdout, "%d lines were rooted back\n", nFailed);
		}

		/*	for (PathTree_t::LeafIterator i(func def()->pathTree().tree()); i; i++)
		{
		HPATH pPath(i.data());
		for (OpList_t::Iterator j(pPath->ops()); j; j++)
		{
		HOP pOp(j.data());
		if ((pOp->ins().opnd_ROOTPREV == 0) ^ (pOp->ins().opnd_REDUCED == 1))//was un-rooted + failed to be re-rooted
		GD C.arrNewRoots.Add(pOp);
		}
		}
		*/


	}

	clearChangeInfo(FUI_ROOTS);
	return STAGE_STATUS_e::DONE;
}

bool FuncInfo_t::testChangeInfo(uint16_t f) const
{
	return (mrFuncDef.m_dc & f) != 0;
}

uint16_t	FuncInfo_t::changeInfo() const
{
	return mrFuncDef.m_dc;
}

void FuncInfo_t::setChangeInfo(uint16_t f) const
{
	mrFuncDef.m_dc |= f;
}

void FuncInfo_t::clearChangeInfo(uint16_t f) const
{
	mrFuncDef.m_dc &= ~f;
}

void FuncInfo_t::SetChangeInfo(uint16_t f) const
{
	if (f & FUI_ROOTS)
		f |= FUI__BLOCKS;
//	if ((mrFuncDef.m_dc & f) == f)
//		return;
	mrFuncDef.m_dc |= f;
	//mrDC.arrFuncDefs.Add(&mrFuncDef);
}

/*int FuncInfo_t::ReCheckXOuts()
{
	return AnalizeHidden();
}*/

//update func's return-lists to conform with one of funcdef
STAGE_STATUS_e FuncInfo_t::UpdateRetOps() const
{
//	if (!IsDecompiled())
//		return;

	mrFuncDef.ValidateRetList();//update fpu regs returned 

	if (mrFuncDef.isStub())
		return STAGE_STATUS_e::SKIPPED;//stub - do nothing

	CallingConv_t CC(*this, FuncDefPtr());

	ExprCacheEx_t aExprCache(PtrSize());
	for (XOpList_t::Iterator i(GetExitPoints(mrFuncDef)); i; ++i)
	{
		HOP pOpRet(i.data());//RETURN op

		OpList_t lXList;
		pOpRet->MoveArgsTo(lXList);
		assert(!pOpRet->hasArgs());

		//for (FuncRetsCIt it(mrFuncDef); !it.isAtEnd(); ++it)
		for (FuncCCRetsCIt it(mrFuncDef, CC); !it.isAtEnd(); ++it)
		{
			CFieldPtr pFieldExit(it.field());
			//CFieldPtr pFieldExit(VALUE(it));
			
			HOP pOp = HOP();
			for (OpList_t::Iterator i(lXList); i; ++i)
			{
				if (i.data()->EqualTo(pFieldExit))
				{
					pOp = lXList.take(i.data());
					break;//assume no more of that kind
				}
			}

			if (pOp)
			{
				//PathOf(PrimeOp(pOp))->takeOp(pOp);
				AddOpArg(pOpRet, pOp);
			}
			else
			{
				//ri_t r(pFieldExit->toRi2());
				ri_t r(it.toR_t());

				if (r.ssid == SSID_FPUREG)
				{
					r.ofs += FpuIn(pOpRet) * FR_SLOT_SIZE;//THE RANGED RET FIELDS ARE RELATIVE TO THE EXIT POINT
					r.typ = OPTYP_REAL80;
				}

				/*if (r.ssid == SSID_NULL)
				{
					 if (r.typ == OPTYP_REAL64)
					 {
						 r.ssid = SSID_FPUREG;
					 }
					 else if ((r.typ & OPSZ_MASK) == OPSZ_DWORD)
					 {
						 r.ssid = RCL(R_EAX);
						 r.ofs = OFS(R_EAX);
					 }
				}*/

				pOp = NewOp();
				pOp->Setup3(r.ssid, r.typ, r.ofs);
				AddOpArg(pOpRet, pOp);
				pOp->setInsPtr(pOpRet->insPtr());
				PathOpTracer_t tr;
				AnlzXDepsIn_t AN(*this, tr, pOp, aExprCache);
				AN.ScanXDeps0(pOpRet);
			}
		}

		//delete rest
		if (!lXList.empty())
		{
			FuncCleaner_t FD(*this);
			FD.ClearOpList(lXList);
		}
	}

	return STAGE_STATUS_e::SKIPPED;//fake skipped
}

STAGE_STATUS_e FuncInfo_t::AnalizeIrreducibleExpressions() const
{
	//if (!(mrFuncDef.m_dc & FUI_XOUTS))
		//return -1;
	for (PathTree_t::LeafIterator i(mrPathTree.tree()); i; i++)
	{
		HPATH rPath(i.data());
		for (PathOpList_t::Iterator j(rPath->ops()); j; j++)
		{
			HOP pOp(PRIME(j.data()));
CHECK(OpNo(pOp) == 1095)
STOP
			ExprCacheEx_t aExprCache(PtrSize());
			PathOpTracer_t tr;
			AnlzXDepsIn_t an1(*this, tr, pOp, aExprCache);
			an1.CheckIrreducibleExpressions();
		}
	}

	//mrFuncDef.m_dc &= ~FUI_XOUTS;
	return STAGE_STATUS_e::DONE;
}

int FuncInfo_t::GetTopOp(bool bOutputUnfold, HOP pSelf, std::set<HOP> &m) const
{
	if (bOutputUnfold || IsFuncStatusAborted())
	{
		m.insert(PrimeOp(pSelf));
		return 1;
	}

	return GetInvolvedOps(pSelf, m);
}

TypePtr FuncInfo_t::OwnerScope() const
{
	return mrFuncDefRef.owner();
}

HiddenTracer_t::HiddenTracer_t(const FuncTracer_t &r)
	: FuncTracer_t(r)
{
}

HiddenTracer_t::HiddenTracer_t(const FuncInfo_t &fi, PathOpTracer_t &tr)
	: FuncTracer_t(fi, tr)
{
}

STAGE_STATUS_e HiddenTracer_t::AnalizeHidden(FuncCleaner_t &FD)
{
	if (!testChangeInfo(FUI_XOUTS))
		return STAGE_STATUS_e::SKIPPED;

	//TRACE1("%s: Analizing x-outs...\n", OwnerFuncName().c_str());

	for (PathTree_t::LeafIterator i(mrPathTree.tree()); i; i++)
	{
		HPATH rPath(i.data());
		PathOpList_t::Iterator j(rPath->ops());
		while (j)
		{
			HOP pOp(PRIME((j++).data()));
			//PathOpList_t::Iterator k(j);
			//k++;
//CHECK(pOp->No() == 1095)
//STOP
			//AnlzXDepsIn_t an1(*this, pOp);
			//an1.CheckExtraXOut();
			//an1.CheckIrreducibleExpressions();
			CheckHidden(pOp, FD);//pOp can be deleted here!
			//j = k;
		}
	}

	clearChangeInfo(FUI_XOUTS);
	return STAGE_STATUS_e::DONE;
}


FieldPtr FuncInfo_t::ThisPtrArg() const
{
	return ProtoInfo_t::ThisPtrArg(FuncDefPtr());
}

bool FuncInfo_t::IsThisCallType() const
{
	return ProtoInfo_s::IsThisCallType(FuncDefPtr());
}

STAGE_STATUS_e FuncInfo_t::TouchRoots(bool bPreserveVars)//0: preserve ops with vars, 1:don't care
{
	if (!bPreserveVars)//special run
	if (!testChangeInfo(FUI_ROOTS))
		return STAGE_STATUS_e::SKIPPED;//not dirty

	int nCount;
	do {
		nCount = 0;
		for (PathTree_t::LeafIterator i(mrPathTree.tree()); i; i++)
		{
			HPATH pPath(i.data());
			switch (pPath->Type())
			{
			case BLK_ENTER:
			case BLK_EXIT:
				continue;
			}

			if (!pPath->ops().empty())
			{
				ProgressInfo("ROOTS", PRIME(pPath->ops().front()));
				for (PathOpList_t::Iterator j(pPath->ops()); j; j++)
				{
					PathOpTracer_t tr;
					AnlzRoots_t an(*this, tr, PRIME(j.data()), true);// , true);
					an.setPreserveVars(bPreserveVars);
					if (an.TurnRoot_Off() == 1)//unroot
						nCount++;
				}
			}
		}
	} while (nCount);

	if (!bPreserveVars)
		clearChangeInfo(FUI_ROOTS);
	return STAGE_STATUS_e::DONE;
}

STAGE_STATUS_e FuncInfo_t::TouchIfs() const
{
	if (!testChangeInfo(FUI_IFS))
		return STAGE_STATUS_e::SKIPPED;//not dirty

	//fun cdef()->mPathTree.TouchIfs();

	PathOpTracer_t tr;
	BranchTracer_t anlz(*this, tr);
	for (PathTree_t::LeafIterator i(mrPathTree.tree()); i; i++)
	{
		if (anlz.TurnBlockIf_On(i.data()) > 0)
			i.rebuild(i.data());
	}

	clearChangeInfo(FUI_IFS);
	return STAGE_STATUS_e::DONE;
}

STAGE_STATUS_e FuncInfo_t::TouchWhiles() const
{
	if (!testChangeInfo(FUI_WHILES))
		return STAGE_STATUS_e::SKIPPED;

	PathOpTracer_t tr;
	BranchTracer_t anlz(*this, tr);
	for (PathTree_t::LeafIterator i(mrPathTree.tree()); i; i++)
	{
		anlz.TurnWhile_On(i.data());
	}

	clearChangeInfo(FUI_WHILES);
	return STAGE_STATUS_e::DONE;
}

STAGE_STATUS_e FuncInfo_t::TouchElses() const
{
	if (!testChangeInfo(FUI_ELSES))
		return STAGE_STATUS_e::SKIPPED;

	PathOpTracer_t tr;
	BranchTracer_t anlz(*this, tr);
	for (PathTree_t::LeafIterator i(mrPathTree.tree()); i; i++)
	{
		if (anlz.TurnBlockElse_On(i.data()) > 0)
			i.rebuild(i.data());
	}

	clearChangeInfo(FUI_ELSES);
	return STAGE_STATUS_e::DONE;
}

STAGE_STATUS_e FuncInfo_t::TouchLogics() const
{
	if (!testChangeInfo(FUI_LOGICS))
		return STAGE_STATUS_e::SKIPPED;//not dirty

	PathOpTracer_t tr;
	BranchTracer_t an(*this, tr);
	bool bContinue;
	do {
		HPATH pPath = TreeTerminalFirst(mrPathTree.body());
		bContinue = false;
		while (!TreeIsLastEx(pPath))
		{
			HPATH pPathNew = HPATH();
			an.TurnLogics_On(pPath, &pPathNew);
			if (pPathNew)
			{
				HPATH pPath/*?*/ = an.GetLogicsTop(pPathNew, 1);//check first
				bContinue = true;
			}
			else
				pPath = TreeNextEx(pPath);
		}
	} while (bContinue);

	clearChangeInfo(FUI_LOGICS);
	return STAGE_STATUS_e::DONE;
}

STAGE_STATUS_e FuncInfo_t::TouchSwitches() const
{
	if (!testChangeInfo(FUI_SWITCHES))
		return STAGE_STATUS_e::SKIPPED;//not dirty

	PathOpTracer_t tr;
	SwitchTracer_t anlz(*this, tr);
	for (PathTree_t::LeafIterator i(mrPathTree.tree()); i; )//i++)
	{
		if (anlz.TurnSwitch_On(i.data()) > 0)
			i.rebuild(i.data());
//CHECK(i.data()->m.zzz==0x42)
//STOP
		++i;
	}

	clearChangeInfo(FUI_SWITCHES);
	return STAGE_STATUS_e::DONE;
}

/*int FuncInfo_t::ReCheckLocals()
{
	setChangeInfo(FUI_LOCALSPOS);//!!
	if (!testChangeInfo(FUI_LOCALSPOS))
		return -1;//not dirty

	LocalsTracer_t an(*this);
	int nCount = an.__recheckLocals(mrFuncDef.Body());
	if (mrFuncDef.IsDecompiled())
	{
		if (nCount > 0)
			fprintf(stdout, "%d local variable(s) was(were) destroyed\n", nCount);
	}
	
	clearChangeInfo(FUI_LOCALSPOS);
	return 1;
}*/

int FuncInfo_t::AddPathRef(HPATH pPath, HOP pOp) const
{
//CHECKID(pField, 2248)
//STOP
	for (XOpList_t::Iterator i(PathOpRefs(pPath)); i; i++)
		if (i.data() == pOp)
			return -1;//reference to this mloc already registered??!
	XOpLink_t *pXRef(NewXOpLink(pOp));
	pPath->pushRefBack(pXRef);
	HPATH pPathOld(PathRef(pOp));
	if (pPathOld)
		ReleasePathRef(pPathOld, pOp);
	SetPathRef(pOp, pPath);
	return 1;
}

int FuncInfo_t::AddLocalRef(FieldPtr pLocal, HOP pOp) const
{
CHECKID(pLocal, 0x10fa)
STOP
	for (XOpList_t::Iterator i(LocalRefs(pLocal)); i; i++)
		if (i.data() == pOp)
			return -1;//reference to this mloc already registered??!
	XOpLink_t *pXRef(NewXOpLink(pOp));
	PushLocalRefBack(pLocal, pXRef);
	FieldPtr pLocalOld(LocalRef(pOp));
	if (pLocalOld)
		ReleaseLocalRef(pLocalOld, pOp);
	SetLocalRef(pOp, pLocal);
	return 1;
}

int FuncInfo_t::ReleasePathRef(HPATH pPath, HOP pOp) const
{
	assert(PathRef(pOp) == pPath);
//CHECKID(pOp, 2248)
//STOP

	if (pPath->hasRefs())
	{
		//const XOpList_t &xrefs(pPath->m.inflow());
		for (XOpList_t::Iterator i(pPath->inflow()); i; i++)
		{
			if (i.data() == pOp)
			{
				XRef_t<OpPtr> *pXRef(pPath->takeRef(i));
				memMgrEx().Delete(pXRef);//xrefs.take(i));
				SetPathRef(pOp, HPATH());
				return 1;
			}
		}
	}
	return 0;
}

int FuncInfo_t::ReleaseLocalRef(FieldPtr pField, HOP pOp) const
{
	assert(pOp->localRef() == pField);
CHECKID(pOp, 0x5743)
STOP
	if (pField->hasUserData())
	{
		XOpList_t &xrefs(LocalRefs(pField));
		for (XOpList_t::Iterator i(xrefs); i; i++)
		{
			if (i.data() == pOp)
			{
				memMgrEx().Delete(xrefs.take(i));
				SetLocalRef(pOp, nullptr);
				if (xrefs.empty())
					DestroyLocalUserData(pField);
				return 1;
			}
		}
	}
	return 0;
}

HOP  FuncInfo_t::GetSwitchOp(HPATH pPath, HOP *ppOpTblRef) const
{
	//FieldPtr pSelf;
	assert(PathType(pPath) == BLK_DATA);
	HOP pOpSwitch = HOP();

	for (XOpList_t::Iterator i(PathOpRefs(pPath)); i; i++)
	{
		HOP pOp(i.data());
		assert(PathRef(pOp) == pPath);

		if (ppOpTblRef)
		{
			if (!IsAddr(pOp))
				break;
			*ppOpTblRef = pOp;
		}

		//looking for goto...
		while (1)
		{
			if (IsPrimeOp(pOp))
			{
				if (IsGoto(pOp))//first (and last?!!)
					return pOp;//is there no more switches on that table?
				else
				{
					if (ActionOf(pOp) != ACTN_MOV)
						if (ActionOf(pOp) != ACTN_ADD)
							if (ActionOf(pOp) != ACTN_SUB)
								break;
				}
				if (!pOp->XOut())
				{
					assert(0);
					/*?FuncInfo_t f0(dc(), *i.data().pFunc->fun cdef());
					AnlzXD epsIn_t anlzxdepsin(f0, pOp);//*pOp->ownerProcPvt(), pOp);
					anlzxdepsin.TraceXDepsOut();*/
				}
				if (!pOp->XOut())
					break;
				pOp = pOp->XOut()->data();
			}
			else if (IsRhsOp(pOp))
			{
				pOp = PrimeOp(pOp);
			}
			else
				pOp = pOp->XOut()->data();
		}

		assert(!pOpSwitch);
		pOpSwitch = pOp;

	}

	//	assert(false);
	return HOP();
}


bool DcInfo_t::IsStackPtr(CFieldPtr pSelf) const
{
	if (FuncInfo_s::SSIDx(pSelf) == mrFE.stack_ptr->ssid)
		if (FuncInfo_s::address(pSelf) == mrFE.stack_ptr->ofs)
			if (pSelf->OpSize() == mrFE.stack_ptr->siz)
				return true;
	return false;
}

//update each call's return-lists
STAGE_STATUS_e FuncInfo_t::CheckCallOuts() const
{
	if (!testChangeInfo(FUI_CALLOUTS))
		return STAGE_STATUS_e::SKIPPED;

	TRACE1("%s: Checking call-outs...\n", OwnerFuncName().c_str());

	//FuncInfo_t fi(*this);

	for (OpList_t::Iterator i(mrFuncDef.mCallRets); i; i++)
	{
		CheckCallout(i.data());
	}

	clearChangeInfo(FUI_CALLOUTS);
	return STAGE_STATUS_e::DONE;
}






////////////////////////////////////////////////////////PathTracer_t

//?int PathTracer_t::s_here = 0;

#if(!NEW_PATH_TRACER)
void PathTracer_t::reset(FuncInfo_t &rFI)
{
	for (PathTree_t::LeafIterator i(rFI.FuncDef().pathTree().tree()); i; i++)
	{
		HPATH pPath(i.data());
		pPath->m_traceInfo.flags = 0;
		for (PathTree_t::ChildrenIterator j(pPath); j; j++)
		{
			assert(0);
			j.data()->m_traceInfo.flags = 0;
		}
	}
}
#endif





/*void PathTracer_t::Clear2()
{
	for (PathTree_t::LeafIterator i(mrFuncDef.pathTree().tree()); i; i++)
	{
		HPATH pPath(i.data());
		//pPath->m.m_bTraced1 = 0;
		pPath->m.m_bTraced2 = 0;
		for (PathTree_t::ChildrenIterator j(pPath); j; j++)
		{
			//j.data()->m.m_bTraced1 = 0;
			j.data()->m.m_bTraced2 = 0;
		}
	}
}

void PathTracer_t::Clear3()
{
	for (PathTree_t::LeafIterator i(mrFuncDef.pathTree().tree()); i; i++)
	{
		HPATH pPath(i.data());
		pPath->m.m_bTraced2 = 0;
		for (PathTree_t::ChildrenIterator j(pPath); j; j++)
		{
			j.data()->m.m_bTraced2 = 0;
		}
	}
}*/

/*
FieldPtr TypeProc_t::Locals()
{
	HPATH pBody = Body();
	if (pBody)
		return pBody->m_pLocals;
	return 0;
}*/

/*FieldPtr  TypeProc_t::Index2Local(int nIndex)
{
	for (HPATH pPath = m_pfDef->Body(); pPath; pPath = pPath->N ext())
	{
		FieldPtr  pData = pPath->m.__index2local(nIndex);
		if (pData)
			return pData;
	}

	return 0;
}

int TypeProc_t::Local2Index(FieldPtr pData)
{
	int nIndex = -1;

	for (HPATH pPath = m_pfDef->Body(); pPath; pPath = pPath->N ext())
	{
		if (pPath->m.__local2index(pData, nIndex))
			return nIndex;
	}

	return -1;
}*/

//////////////////////////////////////

/*FieldPtr TypeProc_t::Index2Label(int nIndex)
{
	for (HPATH pPath = m_pfDef->Body(); pPath; pPath = pPath->N ext())
	{
		FieldPtr pLabel = pPath->m.__index2label(nIndex);
		if (pLabel)
			return pLabel;
	}

	return 0;
}

int TypeProc_t::Label2Index(FieldPtr pLabel)
{
	int nIndex = -1;

	for (HPATH pPath = m_pfDef->Body(); pPath; pPath = pPath->N ext())
	{
		if (pPath->m.__label2index(pLabel, nIndex))
			return nIndex;
	}

	return -1;
}*/

int FileInfo_t::FromFuncProfileEx(GlobPtr g, const FuncProfile_t &fp) const
{
	FieldPtr pField(nullptr);
	if (fp.isThisCall())
	{
		pField = FindFieldFromCPUinChar(g, fp.thisReg());
		if (pField && !ProtoInfo_s::IsThisCallType(g) && g->folder())
		{
			//FileInfo_t GI(*this, *g->folder()->fileDef());
			TypePtr iClass(MakeStruct(MyString(), nullptr, 0, 0, E_KIND_STRUC/*E_KIND_CLASS*/));//add to file before last
			AddTypeObj(iClass, FileDef_t::AT_BEFORE_LAST);
			SetConstPtrToStruc(pField, iClass);
			MakeNonStaticMember(g, pField);
		}
	}
	return 1;
}

bool FuncInfo_t::CheckTopOp(HOP pSelf, HOP pOpTop, bool bNoPaths) const
{
	if (!pOpTop)
		return false;

CHECK(OpNo(pSelf) == 29)
STOP

	HOP pOp = pSelf;
	while (1)
	{
		if (pOp == pOpTop)
			return true;

		if (!IsCodeOp(pSelf))
		{
			if (!IsCallOutOp(pOp))
				break;
		}

		if (IsPrimeOp(pOp))
		{
			//if (IsOpRootEx(pOp))
			if (pOp->isRoot() || !pOp->XOut())
			{
				if (!bNoPaths)
				{
					//check if pOp & pOpTop are on the same logics branch
					HPATH pPathTop = PathOf(pOpTop);
					if (PathOf(pOp) == pPathTop)
						break;
					pPathTop = GetLogicsTop(pPathTop, 1);
					if (pPathTop)
						if (PathType(pPathTop) & BLK_LOGIC)
							if (PathOf(pOpTop)->tailOp() == pOpTop)
								if (TreeIsChildOf(HPATH(PathOf(pOp)), pPathTop))
									return true;
				}
				break;
			}
			//			if (!pOp->XOut())
			//				break;
			if (pOp->m_xout.check_count(1) != 0)
			{
				for (XOpList_t::Iterator i(pOp->m_xout); i; i++)
					if (CheckTopOp(i.data(), pOpTop))
						return true;
				break;
			}
		/*}
		if (IsPrimeOp(pOp))
		{*/
			if (!pOp->XOut())
				break;
			pOp = pOp->XOut()->data();

		}
		else if (IsRhsOp(pOp))
		{
			pOp = PrimeOp(pOp);
		}
		else
		{
			//			if (!pOp->IsCallOutOp())
			//			{
			//				pOp = PrimeOp(pOp);
			//				continue;
			//			}	
			if (!pOp->XOut())
				break;
			pOp = pOp->XOut()->data();
		}
	}

	return false;
}

bool FuncInfo_t::IsOpRootEx(CHOP pSelf) const
{
	assert(IsPrimeOp(pSelf));

	if (!pSelf->isInRow())
	{
		if (pSelf->isRoot())
			return true;

		if (!pSelf->XOut())
			return true;
	}

	return false;
}

static RegMaskType toRegMask(const GPRs_t &v)
{
	RegMaskType ff(0);
	for (size_t i(0); i < v.size(); i++)
	{
		const REG_t &r(v[i]);
		RegMaskType f(TOREGMASK(r.m_ofs, r.m_siz));
		ff |= f;
	}
	return ff;
}

HOP FuncInfo_t::InsertVarOpAt(HOP hOpVar, HOP hOpRef)
{
	HOP pOpAt(PrimeOp(hOpRef));
	HPATH pPath(PathOf(pOpAt));
	if (PathOf(hOpRef) != pPath)
	{
		pOpAt = PRIME(pPath->ops().Tail());
		LinkOpTail(pPath, hOpVar);//link at the bottom of the path
	}
	else
		LinkOpBefore(PathOf(pOpAt), hOpVar, pOpAt);

	hOpVar->setPStackIn(pOpAt->pstackIn());
	hOpVar->setFStackIn(FpuIn(pOpAt));
	return hOpVar;
}

bool FuncInfo_t::InitiateRedecompile() const
{
	Decompiler_t* pAnalyzer(CreateDecompiler(FuncDefPtr()));
	if (!pAnalyzer)
		return false;
	FuncCleaner_t FI(*this);
	FI.Purge(pAnalyzer->redumpCache());
	pAnalyzer->populate();
	//going to re-analyze...
	return true;
}

bool FuncInfo_t::FromFuncProfile() const
{
	StubInfo_t SI(*this);
	const Stub_t* pStub(SI.FindStub(DockAddr()));
	if (!pStub)
		return false;
	FuncProfile_t fp;
	if (ToFuncProfile(pStub->value(), fp))
	{
		ProtoInfo_t TI(*this);
		TI.FromFuncProfile(fp);
	}
	return true;
}

bool FuncInfo_t::Redecompile(bool bFromStub) const
{
	Decompiler_t* pAnalyzer(CreateDecompiler(FuncDefPtr()));
	if (!pAnalyzer)
		return false;

	FullName_t sFunc(GlobNameFull(FuncDefPtr(), DcInfo_t::E_PRETTY, CHOP_SYMB));
	if (!IsStub())
	{
		//a purge required
		FuncCleaner_t FD(*this);
		FD.Purge(pAnalyzer->redumpCache());
		if (bFromStub)
			FromFuncProfile();
		std::cout << "Re-decompiling " << sFunc << " ..." << std::endl;
	}
	else
	{
		if (bFromStub)
			FromFuncProfile();
		std::cout << "Decompiling " << sFunc << " ..." << std::endl;//a brand new decompile
	}

	pAnalyzer->populate();
	return true;
}

STAGE_STATUS_e FuncInfo_t::ReCheckBlocks() const
{
	if (!testChangeInfo(FUI__BLOCKS))
		return STAGE_STATUS_e::SKIPPED;//not dirty

	TRACE1("%s: Re-building blocks tree...\n", OwnerFuncName().c_str());

	PathOpTracer_t pathOpTracer;
	BlocksTracer_t BI(*this);
	BI.UndoPathTree();
	BI.RedoPathTree(pathOpTracer);

	clearChangeInfo(FUI__BLOCKS);
	return STAGE_STATUS_e::DONE;
}

/*int FuncInfo_t::SetupFromOp(Arg_ t *pSelf, OpPtr pOp, FuncDef_t *pfDef) const
{
	pSelf->m_ssid = StorageId(pOp);
	pSelf->mOffs = pOp->Offset0();

	pOp->GetType(pSelf->m_type);

	if (pOp->IsExitOp())
		pSelf->m_flags |= ARG_RETURNED;

	if (pOp->IsThi sPtr(pfDef))
		pSelf->m_ssid |= SSID_THISPTR;//m_flags |= ARG_THISPTR;

	if (pOp->objField())
	{
		FieldPtr pField = (FieldPtr )pOp;
		if (pField->nameless())
			return 1;
	}
	else
	{
		FieldPtr  pMLoc = fieldRef(pOp);
		if (!pMLoc || pMLoc->nameless())
			return 1;
	}
/ *?
	static char str[NAMELENMAX+1];
	std::ostrstream os(str, sizeof(str));
	pOp->name(os);
	os << '\0';
	Set Name(str);
* /
	return 1;
}*/

static FieldPtr findFuncArg(const FuncDef_t& rf, SSID_t ssid, int ofs, int sz)
{
	for (FuncArgsCIt i(rf); i; ++i)
	{
		CFieldPtr pArg(VALUE(i));
		if (FuncInfo_s::SSIDx(pArg) == ssid && FuncInfo_s::address(pArg) == ofs && pArg->size() == sz)
			return (FieldPtr)pArg;
	}
	return nullptr;
}

FieldPtr DcInfo_t::FindFieldFromCPUinChar(GlobPtr g, const REG_t &r) const
{
	assert(r.isValid());

	if ((r.m_siz & OPSZ_MASK) != RetAddrSize())
		return nullptr;

	FieldPtr pArg0(findFuncArg(*g->typeFuncDef(), SSID_CPUREG, r.m_ofs, (r.m_siz & OPSZ_MASK)));
	if (!pArg0)
		return nullptr;

	if (IsStackPtr(pArg0))
		return nullptr;

	return pArg0;
}

void DcInfo_t::ClearFieldMap(FieldMap &m) const
{
	DcCleaner_t<> DC(*this);
	while (!m.empty())
	{
		FieldMapIt i(m.begin());
		FieldPtr pField(VALUE(i));
		m.take(i);
		pField->setOwnerComplex(nullptr);
		if (pField->type())
			DC.ClearType(pField);
		mrDC.memMgr().Delete(pField);
	}
}


void FuncInfo_t::AddToSpoiltList(CHOP pSelf) const
{
//CHECK(OpNo(pSelf) == 170)
//STOP

	ProtoInfo_t TI(*this);
	TI.AddToSpoiltList(pSelf->SSID(), REG_t(pSelf->OpOffs(), pSelf->OpSize()));
}






MyString FuncInfo_t::findProblemStr(ProblemInfo_t f, HOP pOp, bool bExpanded) const
{
	if (f)
	{
		if (!bExpanded)
			return MyStringf("%d", (unsigned)f);
		if (f & (1 << PROBLEM_INTERCODE_EXPOSED))
			return MyStringf("L%s: Intercode statement exposed", StrNo(pOp).c_str());
		if (f & (1 << PROBLEM_PSTACK_NCONV))
		{
			if (IsGoto(pOp) || IsCondJump(pOp))
			{
				if (PathRef(pOp))
				{
					HPATH hPath(PathOf(pOp));
					HPATH hPathTo(PathRef(pOp));
					if (IsGoto(pOp) || CheckProblem(pOp, hPathTo->headOp()))
						return MsgStackTopNotConverged(hPathTo->headOp(), pOp);
					HPATH hPathNx(TreeNextEx(hPath));
					return MsgStackTopNotConverged(hPathNx->headOp(), pOp);
				}
			}
			else
			{
				HOP pOpNx(NextPrimeEx(pOp));
				if (pOpNx)
					return MsgStackTopNotConverged(pOpNx, pOp);
			}
			return "?";
		}
		if (f & (1 << PROBLEM_PSTACK_NRESET))
			return MsgPStackRetNotReset(pOp);
		if (f & (1 << PROBLEM_PSTACK_RET_MISMATCH))
			return MsgPStackRetMismatch(pOp);
		if (f & (1 << PROBLEM_PSTACK_BAD))
			return MsgPStackBadValue(pOp);
		if (f & (1 << PROBLEM_FSTACK_NCONV))
		{
			if (!IsCall(pOp))
			{
				if (IsGoto(pOp) || IsCondJump(pOp))
				{
					if (PathRef(pOp))
					{
						HPATH hPath(PathOf(pOp));
						HPATH hPathTo(PathRef(pOp));
						if (hPathTo->Type() != BLK_EXIT)
						{
							if (IsGoto(pOp) || CheckProblem(pOp, hPathTo->headOp()))
								return MsgFpuTopNotConverged(hPathTo->headOp(), pOp);
							HPATH hPathNx(TreeNextEx(hPath));
							if (hPathNx->Type() != BLK_EXIT)
								return MsgFpuTopNotConverged(hPathNx->headOp(), pOp);
						}
						else
							return MsgFpuTopNotConverged(pOp);
					}
				}
				else
				{
					HOP hOpTo(NextPrimeEx(pOp));
					if (CheckProblem(pOp, hOpTo))
						return MsgFpuTopNotConverged(hOpTo, pOp);
				}
			}
		}
		if (f & (1 << PROBLEM_CONST_CONDITION))
			return MyString("Conditional expression reduced to a constant");
		return "Some problem on line";
	}
	return "";
}


void FuncInfo_t::strxdeps(std::ostream& os, const XOpList_t& xdeps, bool bExtra, bool bUnfold)
{
	if (xdeps.empty() && !bExtra)
		return;

	std::set<HOP > m;
	for (XOpList_t::Iterator i(xdeps); i; i++)
	{
		HOP pOp(i.data());
		if (!pOp->isLinked())
			return;//dagling
		if (!IsVarOp(pOp))
		{
			GetTopOp(bUnfold, pOp, m);
			if (!bUnfold)
				if (!IsFuncStatusAborted())
				{
					//xouts as well
					for (XOpList_t::Iterator j(pOp->m_xout); j; j++)
						GetTopOp(bUnfold, j.data(), m);
				}
		}
		else
			m.insert(PrimeOp(pOp));
	}

	os << "<";
	if (bExtra)
	{
		os << "x";
		if (!xdeps.empty())
			os << ",";
	}

	//sort for display
	std::multimap<int, HOP > m2;
	for (std::set<HOP >::iterator i(m.begin()); i != m.end(); i++)
	{
		HOP pOp(*i);
		m2.insert(std::make_pair(OpNo(pOp), pOp));
	}

	for (std::map<int, HOP >::iterator i(m2.begin()); i != m2.end(); i++)
	{
		if (i != m2.begin())
			os << ",";

		HOP pOp(i->second);
		if (IsArgOp(pOp))
			os << "e";//entry
		/*else if (!IsCode(pOp))
		{
			if (IsDataOp(pOp))
				os << "d";//exit
			else
				os << unk();
		}*/
		else
		{
			/*int i;
			int n(PrimeOp(pOp)->No(&i));
			char buf[10];
			if (i > 0)
				sprintf(buf, "%d+%d", n, i);
			else
				sprintf(buf, "%d", n);
			mos << buf;*/
			os << StrNo(pOp);
		}
	}

	os << ">";
}

bool FuncInfo_t::WriteFuncProfileFromCall(HOP hOp, MyStreamBase& ss)//, IAnalyzer *pAnal)
{
	assert(IsCall(hOp));
	
	GlobPtr iGlob(GetCalleeFuncDef(hOp));

	CFieldPtr pField(iGlob ? FuncInfo_s::DockField(iGlob) : nullptr);
	CFolderPtr pFolder(FileInfo_t::OwnerFolder());
	ADDR va(OpVA(hOp));

	/*ADDR va(pAnal->currentVA());
	FolderPtr pFolder(pAnal->currentFile());
	FieldPtr pField(pAnal->currentOpField());*/
	//if (!pField)//?
		//return false;

	CTypePtr iModule(DcInfo_t::ModuleOf(pFolder));

	DcInfo_t DI(*DCREF(iModule));
	if (pField && pField->isTypeImp())
	{
		FieldPtr pExpField(DI.ToExportedField(pField));
		DcInfo_t DI2(*DcInfo_t::DcFromFolder(*GlobObj(pExpField)->folder()));
		DI2.WriteFuncProfile(ss, pFolder, StubBase_t(pExpField, -1));
	}
	else
		DI.WriteFuncProfile(ss, pFolder, StubBase_t(pField, va));

	return true;
}

MyString FuncInfo_t::LocalName(CFieldPtr pSelf) const
{
	assert(IsLocal(pSelf));
	if (!pSelf->nameless())
	{
		assert(!pSelf->nameless());
		return pSelf->mpName->c_str();
	}

	MyString s;
	if (ProtoInfo_t::IsLocalArgAny(pSelf))
	{
		ProtoInfo_t TI(*this);
		if (TI.LocalAutoName(pSelf, s))
			return s;

	}

	if (IsLocalVar(pSelf))
		if (LocalVarAutoName(pSelf, s))
			return s;

	return AutoName(pSelf, nullptr);
}

MyString FuncInfo_t::LabelNameExx(CFieldPtr pSelf) const
{
	assert(!IsLocal(pSelf));
	if (!pSelf->nameless())
	{
		assert(!pSelf->nameless());
		return pSelf->mpName->c_str();
	}
	TypePtr iType(pSelf->type());
	if (iType && iType->isVTable())
		return MyStringf("%s_%d", iType->printType(), iType->typeStruc()->ID());
//?	MyString s;
//?	if (LabelNameEx(pSelf, s))
//?		return s;
	return AutoName(pSelf, nullptr);
}

MyString FuncInfo_t::RegName(CFieldPtr pSelf, bool upperCase) const
{
	assert(IsLocalReg(pSelf));
	int offs(address(pSelf));
	char buf[80];
	MyString s(mrDC.toRegName(SSIDx(pSelf), offs, pSelf->OpSize(), 1, buf));
	return s.lower();
}

bool FuncInfo_t::LocalVarAutoName(CFieldPtr pSelf, MyString &s) const
{
	assert(IsLocalVar(pSelf));
	assert(!ProtoInfo_t::IsRetVal(pSelf) && !ProtoInfo_t::IsSpoiltReg(pSelf));
	FieldMapCIt cit((FieldPtr)pSelf);
	const FieldMap& m(LocalVarsMap());

	if (main().options().nLocalAutoNamesMode == 0)
	{//simplified
		size_t rank(1 + m.rank(cit));//1-biased
		s += "v";
		s.append(NumberToString(rank));
	}
	else//with offset info
	{
		MyString sfx;
		FieldMapCIt it0(m.lower_bound(pSelf->_key()));//must be cit or greater
		size_t index(m.rank(cit) - m.rank(it0));
		if (index > 0)
		{
			index -= 1;
			sfx = std::string(1 + index / 26, 'a' + index % 26);
		}

		int off(pSelf->offset());
		if (IsStackLocal(pSelf))
		{
			if (off < 0)
			{
				s = "var";
				off = -off;
			}
			else
			{
				s = "avar";
			}
			s += sfx + "_" + Int2Str(off, I2S_HEXA);
		}
		else if (SSIDx(pSelf) == SSID_AUXREG)
		{
			assert(!IsLocalArg(pSelf));
			s = "tvar" + sfx + "_" + Int2Str(off, I2S_HEXA);
		}
		else
		{
			assert(IsLocalReg(pSelf));
			s = "rvar" + sfx + "_" + LocalRegToString(pSelf).upper();
		}
	}
	return true;
}

#if(!NEW_LOCAL_VARS)
bool FuncInfo_t::LocalAutoName(CFieldPtr pSelf, MyString &s) const
{
	TypePtr iStrucLoc(pSelf->owner());//can be funcdef
	FieldPtr pField0(iStrucLoc->parentField());
	int index(0);
	if (pField0)
	{
		TypePtr iUnionLoc(pField0->owner());
		if (iUnionLoc)//func may not have local vars
			index = (int)iUnionLoc->typeStruc()->fields().rank(FieldMapCIt(pField0));// +1;//biased?
	}

	if (IsStackLocal(pSelf))//stack variable
	{
		static const char *pfx("abcde"/*fg*/"hijkl"/*m*/"nopqrs"/*t*/"uvwxyz");
		static const size_t lpfx(strlen(pfx));
		assert((size_t)index < lpfx);
		std::string t(1, pfx[index]);

		//char * str = mrDC.regName(SSID_LOCAL, offs, -1, 1);
		int offs(pSelf->Offset());
		if (offs < 0)
			offs = -offs;
		else if (offs > 0 && index > 0)
			t.push_back('a');

		s = MyStringf("%s%s", t.c_str(), Int2Str(offs, I2S_HEXA).c_str());
		if (*s.rbegin() != 'h')
			s.push_back('h');
		//if (s.back() == 'h')
		{
			//s.pop_back();
			//s.back() = 'H';
		}

		return true;
	}

	if (IsLocalReg(pSelf))//register variables
	{
		int offs(pSelf->Offset());
		s = RegName(pSelf);
		if (pSelf->SSID() == SSID_FPUREG)
		{
			if (index > 0)//non-args
			{
				int c('a' + index - 1);
				assert(c <= 'z');
				s.insert(1, MyStringf("%c", (char)c));
			}
		}
		else if (pSelf->SSID() == SSID_AUXREG)
		{
			assert(index > 0);
			int c('a' + index - 1);
			assert(c <= 'z');
			s = MyStringf("t%c%s", (char)c, Int2Str(offs, I2S_HEXA).c_str());
		}
		else
		{
			//if (index > 0)
			s.append(MyStringf("%d", index));
		}
		return true;
	}
	return false;
}
#endif

void FuncInfo_t::InvertAction(HOP hSelf) const
{
	assert(IsPrimeOp(hSelf));
	assert(IsCondJump(hSelf));
	Action_t A = (Action_t)(ActionOf(hSelf) ^ 1);
	hSelf->setAction(A);
}

HOP FuncInfo_t::CheckIndir(CHOP pSelf) const//FIXED:how about complex instructions?
{
	assert(IsCodeOp(pSelf));
	if (pSelf->IsIndirect())
		return pSelf;
//	if (!IsAddr())
//		return 0;
	assert(!IsPrimeOp(pSelf));

	HOP pOp = pSelf;
	while (1)
	{
		if (IsPrimeOp(pOp))
		{
			if (pOp->m_xout.check_count(1) != 0)
				break;
			pOp = pOp->XOut()->data();
		}
		else if (IsRhsOp(pOp))
		{
			pOp = PrimeOp(pOp);
		}
		else
		{
			if (pOp->m_xout.check_count(1) != 0)
				break;
			pOp = pOp->XOut()->data();
		}
		
		if (pOp->OpC() != OPC_AUXREG)
		{
			if (pOp->IsIndirectOf(SSID_AUXREG))
				return pOp;
			break;
		}
	}

	return HOP();
}

void FuncInfo_t::ReleaseXDeps(HOP pSelf) const
{
	assert(IsCode(pSelf));

	if (IsPrimeOp(pSelf))
	{
		for (OpList_t::Iterator i(pSelf->argsIt()); i; i++)
			ReleaseXDeps(i.data());
	}

	while (pSelf->XIn())
	{
		HOP pOp = pSelf->XIn()->data();
		ReleaseXDepIn(pSelf, pOp);
		ReleaseXDepOut(pOp, pSelf);
	}
}

HPATH FuncInfo_t::GetOwnerBody(HOP hOp) const
{
	if (IsDataOp(hOp))
	{
		HPATH pPath(PathOf(hOp));
		if (!pPath)
			return HPATH();
		return TreeRoot(pPath);// GetOwnerData()->ownerProcPvt()->fun cdef()->Body();
	}

	HPATH  pPath(PathOf(PrimeOp(hOp)));
	pPath = TreeRoot(pPath);
	assert(pPath->Type() == BLK_BODY);
	return pPath;
}


void FuncInfo_t::SetOpVA(HOP pSelf, ADDR va) const
{
	pSelf->ins().setVA(va);
#ifdef _DEBUG
	pSelf->ins().setOff(va - DockAddr());
#endif
}

int FuncInfo_t::OpNo(CHOP hOp) const
{
	if (PathType(PathOf(hOp)) == BLK_DATA)
		return -1;
	ADDR bva(DockAddr());
	ADDR va(OpVA(hOp));
	if (va != 0)
		return (int)(va - bva);
	return -1;
}

MyString FuncInfo_t::StrNo(CHOP pSelf) const
{
	int i(0);
	HOP hOp(PrimeOp(pSelf));

	if (IsExitOp(hOp))
		return "$ret";//NOTE:exit ops are temporary!

	for (; hOp && IsArgOp(hOp); i--)
		hOp = NextPrime(hOp);

	if (!hOp)//entry op?
		return NumberToString(i);
	
	for (; hOp && IsVarOp(hOp); i--)//skip var ops
		hOp = NextPrime(hOp);
	if (!hOp)
		return "?";//?
	ADDR va(OpVA(hOp, i));
	int n(0);
	assert(va != -1);
	{
		ADDR vba(DockAddr());
		if (!(va >= vba))//?
			return "?";
		n = int(va - vba);
	}
	//MyString s(Int2Str(n, I2S_HEX | I2S_SIGNED));
	MyString s(NumberToString(n));
	if (i < 0)
		s += 'z' - (i + 1);
	else if (i > 0)
		s += 'a' + (i - 1);
	return s;
}

int FuncInfo_t::OpOff(CHOP hSelf) const
{
	ADDR bva(PathVA(PathOf(hSelf)));
	return int(OpVA(hSelf) - bva);
}

bool FuncInfo_t::IsCodeOp(CHOP pSelf) const//not xout
{
	if (!IsCode(pSelf))
		return false;
	if (IsCallOutOp(pSelf))
		return false;
	return true;
}

bool FuncInfo_t::IsExitOp(CHOP hSelf) const
{
	if (!IsPrimeOp(hSelf))
		return false;

	HPATH pPath = PathOf(hSelf);
	if (pPath->Type() == BLK_EXIT)
		return true;

	return false;
}

bool FuncInfo_t::IsCallArg(CHOP hSelf) const
{
	if (IsPrimeOp(hSelf))
		return false;

//	HPATH pPath = PathOf(hSelf);
	//if (pPath && pPath->Type() == BLK_DATA)
		//return false;
	//if (IsDataOp(pSelf))
		//return false;

	//HOP pOp = PrimeOp(hSelf);
	//if (pOp != hSelf)//not root op
	if (IsCall(PrimeOp(hSelf)))//not simple xin
			if (InsRefPrime(hSelf).args().contains(hSelf))//call arg
				return true;
	return false;
}

bool FuncInfo_t::IsGotoArg(CHOP hSelf) const
{
	if (IsPrimeOp(hSelf))
		return false;
	if (hSelf->ins().mAction == ACTN_GOTO)
		//if (InsRefPrime(hSelf).args().contains(hSelf))//call arg
			return true;
	return false;
}

bool FuncInfo_t::IsRhsOp(CHOP hSelf) const
{
	if (IsPrimeOp(hSelf))
		return false;

//	HPATH pPath = PathOf(hSelf);
	//if (pPath && pPath->Type() == BLK_DATA)
		//return false;

	//if (IsDataOp(pSelf))
		//return false;
	if (!hSelf->isLinked())//tmp?
		return false;

	//HOP pOp = PrimeOp(hSelf);
	//if (pOp != hSelf)//not root op
	if (InsRefPrime(hSelf).args().contains(hSelf))//call arg
		return true;
	return false;
}

bool FuncInfo_t::IsCallOutOp(CHOP hSelf) const
{
	if (IsPrimeOp(hSelf))
		return false;
	//if (IsDataOp(pSelf))
		//return false;

	if (!hSelf->isLinked())//tmp?
		return false;

	HOP pOp(PrimeOp(hSelf));
	assert(pOp);
	if (IsCall(pOp))//eliminate xins
	{
		//find it among xouts
		for (XOpList_t::Iterator i(pOp->m_xout); i; i++)
			if (i.data() == hSelf)
				return true;
	}
	return false;
}

bool FuncInfo_t::IsCodeRoot(CHOP hSelf) const
{
	//if (IsDataOp(hSelf))
		//return false;

	if (!IsPrimeOp(hSelf))
		return false;

	HPATH pPath = PathOf(hSelf);
	if (pPath && pPath->Type() == BLK_DATA)
		return false;//not the data op

	return true;
}

FieldPtr FuncInfo_t::IsCallToThunkAt(CHOP pSelf) const
{
	if (IsCall(pSelf) && IsCallDirect(pSelf))
	{
		CFieldPtr pField(GetCalleeDockField(pSelf));
		if (pField && pField->isTypeThunk())
			return (FieldPtr)pField;
	}
	return nullptr;
	//CGlobPtr iGlob(GetFuncAtCall(pSelf));
	//assert(iGlob);
	//if (!iGlob->isThunk())
		//return nullptr;
	//return (GlobPtr)iGlob;
}

bool FuncInfo_t::IsAddr(CHOP pSelf) const
{
	if (StorageOf(pSelf).isFlag())
		return false;
	return pSelf->isAddr();
}

bool FuncInfo_t::IsCallDirect(CHOP hSelf) const
{
	assert(IsCall(hSelf));
	if (!IsAddr(hSelf))
		return false;
	return !IsCallIntrinsic(hSelf);
}

bool FuncInfo_t::IsAnalized(CHOP hSelf) const 
{
	return PathOf(hSelf)->isAnalized();
}

bool FuncInfo_t::IsCondJump(CHOP hSelf) const
{
	if (IsPrimeOp(hSelf))//?
	{
		if (ISJMPIF(ActionOf(hSelf)))
			if (IsAddr(hSelf))//to distinguish it from setifs
//				if (PathType(PathOf(hSelf)) == BLK_JMPIF)	//???
					return true;//data ops are not actions
	}
	return false;
}

bool FuncInfo_t::LocusFromOpDisp(HOP pSelf, Locus_t &aLoc) const
{
	ADDR va(pSelf->m_disp);
	CTypePtr pSeg(OwnerSeg());
	TypePtr pSeg2(VA2Seg(pSeg, va));
	/*CTypePtr pSeg2(FindSegAt(pSeg, va, pSeg->typeSeg()->affinity()));
	if (!pSeg2)
	{
		if ((pSeg = pSeg->typeSeg()->ownerRangeSet(pSeg)) != nullptr)
			pSeg2 = FindSegAt(pSeg, va);
	}*/
	if (!pSeg2)
		return false;
	return LocusFromVA_1(pSeg2, va, aLoc);
}

bool FuncInfo_t::IsDataOp(CHOP hSelf) const
{
	if (!IsPrimeOp(hSelf))
		return false;
	assert(PathOf(hSelf));
	return PathType(PathOf(hSelf)) == BLK_DATA;
}

bool FuncInfo_t::IsThisPtrOp(CHOP pOp) const
{
	assert(pOp);
	if (!IsArgOp(pOp))
		return false;
	FieldPtr pThisArg(ThisPtrArg());
	if (!pThisArg)
		return false;
	HPATH pPath(PathHead());
	if (!pPath)
		return false;
	if (pPath->ops().empty())
		return false;
	if (!IsMineOp(pPath, pOp))
		return false;
	FieldPtr pArg(FindArgFromOp(pOp));
	return (pArg == pThisArg);
}

HOP FuncInfo_t::GetOpToBindWith(CHOP hSelf) const
{
	assert(IsVarOp(hSelf));
	FieldPtr pField(hSelf->localRef());
	//if (!pField || pField->xre fs().check_count(2) != 0)//itself and opref
		//return nullptr;
	PathOpList_t::Iterator i(PathOf(hSelf)->ops(), INSPTR(hSelf));
	if (!(++i))//advance to the next
		return HOP();
	HOP pOpRef(PRIME(i.data()));
	if (pOpRef->localRef() != pField)
	{
		if (IsCall(pOpRef))
		{
			XOpList_t::Iterator j(pOpRef->m_xout);
			if (j)
			{
				pOpRef = j.data();
				if (pOpRef->localRef() == pField)
				if (!(++j))//the only one
					return pOpRef;
			}
		}
		return HOP();
	}
	return pOpRef;
}

HOP FuncInfo_t::IsBoundToVar(CHOP hSelf) const
{
	//assert(!IsVarOp());
	if (!IsCallOutOp(hSelf))
	{
		if (!IsPrimeOp(hSelf))
			return HOP();
		if (!IsCode(hSelf))
			return HOP();
	}
	HOP pOp0(PrimeOp(hSelf));
	if (!pOp0->isRoot())
		return HOP();
	FieldPtr pField(hSelf->localRef());
	if (!pField)
		return HOP();
	//if (!pField || pField->xr efs().check_count(2) != 0)//itself and varop
		//return nullptr;
	PathOpList_t::ReverseIterator i(PathOf(pOp0)->ops(), INSPTR(pOp0));
	if (!(++i))//advance to the prev
		return HOP();
	HOP pOpVar(PRIME(i.data()));
	if (!IsVarOp(pOpVar))
		return HOP();
	if (pOpVar->localRef() != pField)
		return HOP();
	if (!pOpVar->isInRow())
		return HOP();
	return pOpVar;
}

//checks if op lies on top when output is in HLL
bool FuncInfo_t::IsOnTop(CHOP hSelf) const
{
	if ( !IsCode(hSelf) )
		return true;

	if (IsPrimeOp(hSelf))
	{
		if (IsCall(hSelf))
			return true;
		if (hSelf->isRoot())//Ex())
			return true;
		return false;
	}

	if (!hSelf->XIn())
	{
//		if (IsStackPtr())
//			return false;
		return true;
	}

	if (hSelf->IsIndirectB())
		return true;

	bool bSingle = (hSelf->m_xin.check_count(1) == 0);

	HOP pOpUp = HOP();
	for (XOpList_t::Iterator i(hSelf->m_xin); i; i++)
	{
		bool bEntry = false;

		HOP pOp(i.data());
		if (IsArgOp(pOp))
		{
			bEntry = true;
		}
		else if (IsVarOp(pOp))
		{
			return true;
		}
		else if (!IsCodeOp(pOp))
		{
			assert(IsCallOutOp(pOp));
			pOp = pOp->XIn()->data();
			if (pOp->m_xout.check_count(1) > 0)
			{
				//assert(pOp->isRoot());
				return true;
			}
		}

		if (!bEntry)
			if (pOp->isRoot())
				return true;

		if (!bSingle && !bEntry && (ActionOf(pOp) == ACTN_MOV) && pOp->arg1()->IsScalar())
		{
			HOP op = pOp->arg1();
			assert(!IsAddr(op));
//why?			assert(op.i32 == 0);
		}
		else
		{
			if (pOpUp)
				return true;
			else
				pOpUp = pOp;
		}
	}

	return (pOpUp == HOP());
}

int FuncInfo_t::SetRoot(HOP hSelf, bool b) const
{ 
	assert(IsPrimeOp(hSelf));
//CHECK(No() == 60)
CHECKID(hSelf, 0xd89)
STOP
	if (hSelf->ins().isRoot() == b) 
		return -1;
	hSelf->ins().setRoot(b); 
	return 1; 
}

BlockTyp_t FuncInfo_t::CheckPathBreak(CHOP hSelf) const
{
	if (IsGoto(hSelf))
	{
		if (IsAddr(hSelf))
			return BLK_JMP;
		if (IsRetOp(hSelf))
			return BLK_JMP;
		return BLK_JMPSWITCH;
	}
	if (IsCondJump(hSelf))
		return BLK_JMPIF;
#if(0)
	if (IsCall(hSelf))
		return BLK_CALL;
#endif

	return BLK_NULL;
}

HOP FuncInfo_t::GetSwitchOp2(CHOP pSelf) const
{
	HOP pOp = pSelf;

	while (1) 
	{
		if (IsPrimeOp(pOp))
		{
			if (IsGoto(pOp))
				return pOp;
			if (pOp->m_xout.check_count(1) != 0)
				break;
			pOp = pOp->XOut()->data();
		}
		else if (IsRhsOp(pOp))
		{
			pOp = PrimeOp(pOp);
		}
		else
		{
			if (pOp->m_xout.check_count(1) != 0)
				break;
			pOp = pOp->XOut()->data();
		}

		if (pOp == pSelf)//?
			break;
	}

	return HOP();
}

int FuncInfo_t::CheckSwitch(CHOP pSelf) const
{
	HOP pOp = GetSwitchOp2(pSelf);
	if (!pOp)
		return 0;

	HPATH pPath = PathOf(pOp);
	if (pPath)
	{
		if (PathType(pPath) != BLK_JMPSWITCH)
			return 0;
		//!assert(PathType(pPath) == BLK_JMPSWITCH);
//[		if (pPath->Parent())
		if (PathType(pPath->Parent()) == BLK_SWITCH)
			return 1;
	}

	return 0;
}

/*FieldPtr Op_t::GetPrevLabel()
{
	assert(IsPrimeOp());
	HOP pOp = lin ked();
	while (1)
	{
		if (pOp->Label())
			return pOp->Label();
		pOp = pOp->list().Prev(pOp->insP tr())->pr ime();
	}
	assert(false);
	return 0;
}*/

int FuncInfo_t::CheckStackBreak(CHOP pSelf, CHOP pOpPr) const
{
	assert(pOpPr);
	assert(IsPrimeOp(pSelf) && IsPrimeOp(pOpPr));
	return (StackOut(pOpPr) - pSelf->pstackIn());
}

int FuncInfo_t::CheckFStackBreak(CHOP pSelf, CHOP pOpPr) const
{
	assert(pOpPr);
	assert(IsPrimeOp(pSelf) && IsPrimeOp(pOpPr));
	return (FpuOut(pOpPr) - FpuIn(pSelf));
}

void FuncInfo_t::Numberize(HOP pSelf, ADDR va) const
{
	assert(IsPrimeOp(pSelf));
	SetOpVA(pSelf, va);
}

int FuncInfo_t::ConvertOPC(HOP hSelf) const
{
	int res(0);

	Op_t &rSelf(*hSelf);

	if (IsPrimeOp(hSelf))
	{
		for (OpList_t::Iterator i(rSelf.argsIt()); i; i++)
		{
			if (ConvertOPC(i.data()))
				res = 1;
		}
	}

	if (rSelf.IsIndirectOf(SSID_FPUSW))
	{
		if (uint8_t(rSelf.OpOffs()) == OFS(R_FPUSW_TOP))
		{
			const Ins_t& rIns(rSelf.ins());
			assert(!rSelf.m_xin);
			rSelf.m_opc = OPC_FPUREG;
			int off((rIns.mFStackIn + rSelf.m_disp) / FTOP_STEP);
			rSelf.mOffs = (int8_t)off * FR_SLOT_SIZE;
			rSelf.m_disp0 = rSelf.m_disp;
			rSelf.m_disp = 0;
			res = 1;
		}
	}
	else if (IsStackPtrB(hSelf))//only [esp] are converted here!
	{
		PathOpTracer_t tr;
		LocalsTracer_t LT(*this, tr);
		if (LT.MakeLocal(hSelf))
			res = 1;
	}

	return res;
}

int FuncInfo_t::RevertFPUREG(HOP hSelf) const
{
	int res(0);

	Op_t &rSelf(*hSelf);

	if (IsPrimeOp(hSelf))
	{
		for (OpList_t::Iterator i(rSelf.argsIt()); i; i++)
		{
			if (RevertFPUREG(i.data()))
				res = 1;
		}
	}

	if (rSelf.OpC() == OPC_FPUREG)
	{
		const Ins_t &rIns(rSelf.ins());
		assert(!rSelf.m_xin);
		rSelf.m_opc = OPC_INDIRECT | SSID_FPUSW;
		rSelf.m_disp = (rSelf.mOffs / FR_SLOT_SIZE - rIns.mFStackIn) / FTOP_STEP;
		rSelf.mOffs = OFS(R_FPUSW_TOP);
		rSelf.m_disp0 = 0;
		res = 1;
	}

	return res;
}

int FuncInfo_t::RevertLocal(HOP pSelf, FieldPtr *ppLoc) const
{
	if (pSelf->SSID() != SSID_LOCAL)
		return 0;

	assert(!pSelf->IsIndirectB());//?

	ReleaseXDeps(pSelf);

	bool bAddr = IsAddr(pSelf);
	pSelf->m_opc = mrFE.stack_ptr->ssid;//OPC_CPUREG;

	if (!bAddr)
	{
		pSelf->m_opc |= OPC_INDIRECT;
//		SetOp Seg(OPSEG_STACK);
	}
		
	pSelf->m_disp = pSelf->m_disp0;
	pSelf->m_disp0 = 0;

	if (ppLoc)
		*ppLoc = LocalRef(pSelf);

//	if (m_p MLoc)
//		m_pM Loc->ReleaseLabelRef(this);

	return 1;
}

int FuncInfo_t::RevertLocals(HOP pSelf) const
{
	assert(IsPrimeOp(pSelf));

	int res = 0;

	for (OpList_t::Iterator i(pSelf->argsIt()); i; i++)
	{
		if (RevertLocal(i.data(), nullptr))
			res = 1;
	}

	if (RevertLocal(pSelf, nullptr))
		res = 1;
	return res;
}

bool FuncInfo_t::IsUnrefed(CHOP hSelf) const// { return m_xrefs.empty(); }
{
	assert(IsVarOp(hSelf) || IsArgOp(hSelf));
	XOpList_t::Iterator i(LocalRefs(hSelf->localRef()));
	assert(i);
	if (i.data() != hSelf)
		return false;
	i++;
	return (!i);
}

int FuncInfo_t::CheckTestAction(CHOP hSelf) const
{
	assert(IsPrimeOp(hSelf));
	if (ActionOf(hSelf) == ACTN_MOV)
		return 1;

	if (hSelf->isCPUSW())
	if (ActionOf(hSelf) == ACTN_AND)
	if (hSelf->args().check_count(1) == 0)
		return 1;
	return 0;
}

bool FuncInfo_t::IsRootEx0(CHOP hSelf) const
{
	if (IsPrimeOp(hSelf))
	{
		//?		if ( g_ pDI )//outputting
		//?		if ( g_ pDI->isUnfoldMode() )
		//?			return true;

		if (hSelf->isRoot())
			return true;
		if (!hSelf->XOut())
			return true;
	}

	return false;
}


















