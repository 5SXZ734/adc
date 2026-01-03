#include "ana_switch.h"
#include "prefix.h"

#include "shared/defs.h"
#include "shared/action.h"
#include "shared/data_source.h"
#include "db/proj.h"
#include "path.h"
#include "type_funcdef.h"
#include "ana_branch.h"
#include "info_dc.h"
#include "expr_simpl.h"
#include "expr_dump.h"

//////////////////////////
// S W I T C H 

SwitchTracer_t::SwitchTracer_t(const FuncTracer_t &fi)
	: FuncTracer_t(fi),
	meErrorCode(ERR_NONE)
{
}

SwitchTracer_t::SwitchTracer_t(const FuncInfo_t &fi, PathOpTracer_t &tr)
	: FuncTracer_t(fi, tr),
	meErrorCode(ERR_NONE)
{
}

MyString SwitchTracer_t::ErrorAsString() const
{
	switch (meErrorCode)
	{
	case ERR_CONTEXT: return "No unconditional JUMP at given context";
	case ERR_PATTERN: return "Unrecognized SWITCH statement pattern";
	case ERR_UNKNOWN: return "Operation failed";
	default: break;
	}
	return "No error";
}

int SwitchTracer_t::ToggleSwitch(HPATH pSelf, bool bVerbose)
{
	assert(pSelf->IsTerminal());
	int res = TurnSwitch_On(pSelf);
	if (res == -1)//already
		res = TurnSwitch_Off(pSelf);
	if (res == 0 && bVerbose && GotError())
		PrintError() << ErrorAsString() << std::endl;
	assert(res != -1);
	return res;
}

int SwitchTracer_t::GetSwitchInfo2View(HPATH pSelf, SwitchQuery_t &si) const
{
	HOP pOpSwitch = GetLastOp(pSelf);

	MyStream ss(1024);
	ss.SetEndOfLineChar('\n');
	int line(OpNo(PrimeOp(pOpSwitch)));
	ExprCacheEx_t a(PtrSize());

	EXPR_t expr(*this, pOpSwitch, 0, a);
	Out_t *pOut(expr.DumpExpression2(pOpSwitch));

	TExprDump2view<EXPRSimpl_t> ES(*this, pOpSwitch, 0, a, line, ss);
	ES.Simplify(pOut);
	ES.SimplifySwitch(pOut, si, pSelf);

	ss.WriteChar(0);//eos!
	mrDC.NewPtrDump(DockAddr(), line, ss);
	//expr.GetSwitchInfo(pOut->mpR, si);
	return si.pJumpTable ? 1 : 0;
}

int FuncInfo_t::GetSwitchInfo(HPATH pSelf, SwitchQuery_t &si) const
{
	assert(PathType(pSelf) == BLK_JMPSWITCH);
	HOP pOpSwitch(GetLastOp(pSelf));

	ExprCacheEx_t a(PtrSize());
	EXPR_t expr(*this, pOpSwitch, 0, a);
	Out_t* pOut(expr.DumpExpression2(pOpSwitch));

	EXPRSimpl_t ES(expr);
	ES.Simplify(pOut);
	ES.SimplifySwitch(pOut, si, PathOf(pOpSwitch));
	return si.pJumpTable ? 1 : 0;
}

int SwitchTracer_t::TurnSwitch_On(HPATH pSelf)
{
	if (PathType(pSelf) != BLK_JMPSWITCH)
		return GotError(ERR_CONTEXT);

	int id = PathNo(pSelf);
	//CHECK(id == 363)
	//STOP

	//[	if (lin ked()->Parent())
	if (PathType(pSelf->Parent()) == BLK_SWITCH)
		return -1;//already

	if (CheckOpsCount(pSelf, 1) != 0)
		return GotError(ERR_UNKNOWN);//only switch goto!


	SwitchQuery_t si;
#if(!TRACK_PTRS2VIEW)
	if (!GetSwitchInfo(pSelf, si))
#else
	if (!GetSwitchInfo2View(pSelf, si))
#endif
		return GotError(ERR_PATTERN);

	HPATH pJumpTablePath(GetJumpTablePath(pSelf));
	if (!pJumpTablePath)//?
		return GotError(ERR_UNKNOWN);
	const PathOpList_t &l(PathOps(pJumpTablePath));
	if (l.empty())
		return GotError(ERR_UNKNOWN);

	HOP pPtr(PRIME(l.front()));

	//HPATH pIndexTablePath(GetIndexTablePath(pJumpTablePath));
	if (si.pIndexTable)
		if (!CreateSwitchIndexTable(si.pIndexTable, pJumpTablePath))
		{
			MyString s(FieldName(si.pIndexTable));
			fprintf(STDERR, "Warning: %s: Bad index table\n", s.c_str());
		}

	BranchTracer_t anBranch(*this);
	if (!anBranch.TurnBlockIf_On(si.pPathDefault))
	{
		//assert(CheckValid(TreeRoot(pSelf)));
		if (!CheckValid(TreeRoot(pSelf))) return GotError(ERR_UNKNOWN);//?

		if (!RecoverDefaultBranch(si.pPathDefault))
		{
			return GotError(ERR_UNKNOWN);
		}
		else
		{
			assert(CheckValid(TreeRoot(pSelf)));
			if (!anBranch.TurnBlockIf_On(si.pPathDefault))//try again
			{
				anBranch.AdjustGoto(si.pPathDefault);
				return GotError(ERR_UNKNOWN);
			}
		}
	}

	//get closing label
	HPATH pGotoPath(GetGotoPath(si.pPathDefault));
	assert(pGotoPath);

	//get starting block
	HPATH pPath1(GetJumpTablePath(pSelf));
	pPath1 = TreeNext(pPath1);
	if(PathType(pPath1) == BLK_DATA)//index table?
		pPath1 = TreeNext(pPath1);
	assert(PathType(pPath1) != BLK_DATA);

	HPATH pPath2 = GetClosingPath(pPath1, pGotoPath);
	if (!pPath2)
		return GotError(ERR_UNKNOWN);

	//block out switch cases container block
	HPATH pBlockCases = BlockOut(pPath1, pPath2);
	assert(pBlockCases);

	//create <BLK_SWITCH> block
	HPATH pBlockSwitch = BlockOut(pSelf, pBlockCases);
	assert(pBlockSwitch);
	SetPathType(pBlockSwitch, BLK_SWITCH);
	anBranch.RemoveRedundantParent(pBlockSwitch);

	//create <BLK_SWITCH_0> block
	HPATH pBlockIf = si.pPathDefault->Parent();
	HPATH pBlockSwitch_0 = BlockOut(pBlockIf, pBlockIf);
	assert(pBlockSwitch_0);
	SetPathType(pBlockSwitch_0, BLK_SWITCH_0);

	//assert(LabelP ath(si.pJumpTable) == pJumpTablePath);

	//extract the case-blocks out
	BlockOutCases(pBlockCases, pJumpTablePath);

	RelocateBadCases(pBlockSwitch);
	return 1;
}

int SwitchTracer_t::TurnSwitch_Off(HPATH pSelf)
{
	if (PathType(pSelf) != BLK_JMPSWITCH)
		return 0;

	//[	if (!m_pParent)
	//		return -1;//no staff

	if (PathType(pSelf->Parent()) != BLK_SWITCH)
		return -1;

	HPATH pJumpTablePath(GetJumpTablePath(pSelf));
	const PathOpList_t &l(PathOps(pJumpTablePath));
	if (l.empty())
		return 0;

	HOP pPtr(PRIME(l.front()));

	HPATH pBlockSwitch = pSelf->Parent();
//?	assert(pBlockSwitch->CheckChildrenCount(3) == 0);

	HPATH pBlockIf = pBlockSwitch->Parent();
	assert(PathType(pBlockIf) == BLK_IF);

	//retract <BLK_SWITCH_0> block
	HPATH pBlockDefault = pBlockIf->Parent();
	assert(PathType(pBlockDefault) == BLK_SWITCH_0);
	BlockIn(pBlockDefault);

	//retract cases <BLK_CASE> marks
	HPATH pBlockCase = pBlockSwitch->LastChild();
	if (PathType(pBlockCase) == BLK_NULL)
		pBlockCase = pBlockCase->FirstChild();

	while (pBlockCase)
	{
		assert(PathType(pBlockCase) == BLK_CASE);
		HPATH pCaseBlockNx = TreeNext(pBlockCase);
		BlockIn(pBlockCase);
		pBlockCase = pCaseBlockNx;
	}

	//retract <BLK_SWITCH> block
	BlockIn(pBlockSwitch);
	return 1;
}

int SwitchTracer_t::RecoverDefaultBranch(HPATH pSelf)
{
	assert(PathType(pSelf) == BLK_JMPIF);
	HPATH pPathLast = TreeTerminalLast(pSelf->Parent());

	if (PathType(pPathLast) != BLK_JMP)
	{
		HOP pOpLast2 = GetLastOp(pPathLast);
		if (!ChangeGotoDestination(pSelf, pOpLast2, true))
			return 0;
		return 1;
	}

	if (pPathLast == pSelf)
		return 0;
	HOP pOpLast2 = GetLastOp(pPathLast);
	if (!ChangeGotoDestination(pSelf, pOpLast2))
		return 0;
	return 1;
}

int SwitchTracer_t::ExpandSwitch(HPATH pSelf)
{
	if (PathType(pSelf) != BLK_JMPSWITCH)
		return 0;
	if (PathType(pSelf->Parent()) != BLK_SWITCH)
		return 0;//there is no switch!
	//get outer switch block (with default)
	HPATH pBlockSwitch = pSelf;
	while (PathType(pBlockSwitch) != BLK_SWITCH_0)
		pBlockSwitch = pBlockSwitch->Parent();

	if (TreeIsLast(pBlockSwitch))
		return 0;

	HPATH pBlockDefault;
	if (pBlockSwitch->CheckChildrenCount(2) < 0)
	{
		pBlockDefault = NewPath();
		pBlockDefault->setFlags(BLK_DEFAULT);
		TreePushChildBack(pBlockSwitch, pBlockDefault);
		//pBlockDefault->SetParent(pBlockSwitch);
	}
	else
		pBlockDefault = pBlockSwitch->LastChild();

	HPATH pBlockNx = TreeNext(pBlockSwitch);
	pBlockDefault->UnlinkChild(pBlockNx);// pBlockNx->Unlink();
	TreePushChildBack(pBlockDefault, pBlockNx);
	//pBlockNx->SetParent(pBlockDefault);

	return 1;
}

int SwitchTracer_t::CollapseSwitch(HPATH pSelf)
{
	if (PathType(pSelf) != BLK_JMPSWITCH)
		return 0;
	if (PathType(pSelf->Parent()) != BLK_SWITCH)
		return 0;//there is no switch!

	//get outer switch block (with default)
	HPATH pBlockSwitch = pSelf;
	while (PathType(pBlockSwitch) != BLK_SWITCH_0)
		pBlockSwitch = pBlockSwitch->Parent();

	if (pBlockSwitch->CheckChildrenCount(2) < 0)
		return 0;//there is no default block

	HPATH pBlockDefault = pBlockSwitch->LastChild();
	HPATH pBlock = pBlockDefault->LastChild();
	pBlockDefault->UnlinkChild(pBlock);// pBlock->Unlink();
	HPATH pParent(pBlockSwitch->Parent());
	TreeInsertChildAfter(pParent, pBlock, pBlockSwitch);
	//pBlock->SetParent(pBlockSwitch->Parent());

	if (!pBlockDefault->HasChildren())
		memMgrEx().Delete(pBlockDefault);

	return 1;
}

/*int SwitchTracer_t::CheckCase(FieldPtr pSelf, FieldPtr *ppSwitchTab)
{
	FieldPtr pJmpTab = 0;
	for (XOpList_t::Iterator i(LocalRefs(pSelf)); i; i++)
	{
		HOP pOp = i.data();

		if (IsDataOp(pOp))
		{
			HOP pIOp = pOp;
			assert(GetOwnerData(pIOp));
			if (!pJmpTab)
			{
				pJmpTab = GetOwnerData(pIOp);
#ifndef _DEBUG
				break;
			}
#else
		}
			else
				assert(pJmpTab == GetOwnerData(pIOp));
#endif
	}
}

	if (ppSwitchTab)
		*ppSwitchTab = pJmpTab;
	return (pJmpTab != 0);
}*/

int SwitchTracer_t::CheckCase2(HPATH pPath, HPATH pJumpTablePath)
{
	//FieldPtr pSelf(pPath->m.Lab elAt());
	//if (pSelf)
	{
		assert(pJumpTablePath);
		for (XOpList_t::Iterator i(PathOpRefs(pPath)); i; i++)
		{
			HOP pOp(i.data());
			if (IsDataOp(pOp))
			{
				HOP pIOp(pOp);
				if (PathOf(pIOp) == pJumpTablePath)
					return 1;
			}
		}
	}

	return 0;
}

HOP SwitchTracer_t::FindIOp(HPATH pSelf, ADDR vba)
{
	for (PathOpList_t::Iterator i(PathOps(pSelf)); i; i++)
	{
		HOP hIOp(PRIME(i.data()));
		if (hIOp->VA() == vba)
			return hIOp;
	}
	return HOP();
}


int SwitchTracer_t::BlockOutCases(HPATH pSelf, HPATH pJumpTablePath)
{
	assert(PathType(pSelf->Parent()) == BLK_SWITCH);
	assert(IsNull(pSelf) && TreeIsLast(pSelf));//container of cases!

	HPATH pPathStart = HPATH();
	HPATH pPathEnd = HPATH();
	HPATH pPath = pSelf->GetChildFirst();
	while (1)
	{
		if (pPath)
		{
			if (PathType(pPath) == BLK_CASE)
			{//case blocks already may to be there
				pPathEnd = TreePrev(pPath);
			}
			else
			{
				if (CheckCase2(pPath, pJumpTablePath))
				{
					if (!pPathStart)
						pPathStart = pPath;
					else
						pPathEnd = TreePrev(pPath);
				}
			}
		}

		if (pPathStart && pPathEnd)
		{
			HPATH pPathCase = BlockOut(pPathStart, pPathEnd);
			SetPathType(pPathCase, BLK_CASE);
			pPathStart = HPATH();
			pPathEnd = HPATH();

			if (pPath && PathType(pPath) != BLK_CASE)
				pPathStart = pPath;
		}

		if (!pPath)
			break;
		if (TreeIsLast(pPath))
			pPathEnd = pPath;
		pPath = TreeNext(pPath);
	}

	return 1;
}

int SwitchTracer_t::RelocateBadCases(HPATH pSelf)
{
	assert(PathType(pSelf) == BLK_SWITCH);

	int nTotal = 0;
	HPATH pJumpTablePath(GetJumpTablePath(pSelf->GetChildFirst()));
	for (PathOpList_t::Iterator i(PathOps(pJumpTablePath)); i; i++)
	{
		HPATH pPathTo(PathRef(PRIME(i.data())));
		if (CheckBetterLocation(pPathTo, pSelf))
			nTotal++;
	}

	if (nTotal > 0)
	{
		BlockOutCases(pSelf->GetChildLast(), pJumpTablePath);
		SetChangeInfo(FUI_NUMBERS);
	}

	return nTotal;
}

int SwitchTracer_t::CheckBetterLocation(HPATH pSelf, HPATH pBlockSwitch)
{
	assert(pSelf->IsTerminal());
//	if (!pSelf->m.Labe lAt())
//		return 0;
	if (PathOpRefs(pSelf).check_count(1) != 0)
		return 0;
	if (PathType(pSelf) != BLK_JMP)
		//		if (PathType(pSelf) != BLK_RET)
		return 0;

	HOP pOpRef0 = PathOpRefs(pSelf).head()->data();
	if (IsCodeOp(pOpRef0))
		return 0;//NR

	assert(IsDataOp(pOpRef0));
	HOP pOpRef = pOpRef0;

	if (!pBlockSwitch)
	{
		pBlockSwitch = GetSwitchBlock(pOpRef);
		if (!pBlockSwitch)
			return 0;
	}
	assert(PathType(pBlockSwitch) == BLK_SWITCH);

	if (IsCaseOf(pOpRef, pBlockSwitch))
		return 0;
	HPATH pPathPr = TreePrevEx(pSelf);
	assert(pPathPr);
	if (PathType(pPathPr) != BLK_JMP)
		//	if (PathType(pPathPr) != BLK_RET)
		return 0;

	HPATH pBlockBefore = pBlockSwitch->GetChildLast();
	assert(IsNull(pBlockBefore));
	pBlockBefore = pBlockBefore->GetChildFirst();

	HPATH pParent(pSelf->Parent());
	assert(pParent->FirstChild() != pSelf);
	pParent->UnlinkChild(pSelf);
	TreeInsertChildBefore(HPATH(pBlockBefore->Parent()), pSelf, pBlockBefore);
	//pSelf->SetParent(pBlockBefore->Parent());
	pSelf->Parent()->SetChildren(pSelf);

	SetChangeInfo(FUI_NUMBERS);
	return 1;
}

int SwitchTracer_t::CheckSwitchValid(HPATH pSelf)
{
	if (PathType(pSelf) != BLK_JMPSWITCH)
		return 0;
	if (PathType(pSelf->Parent()) != BLK_SWITCH)
		return 0;

	if (CheckOpsCount(pSelf, 1) != 0)
		TurnSwitch_Off(pSelf);

	return 1;
}

int SwitchTracer_t::ChangeGotoDestination(HPATH pSelf, HOP pOpTo, bool bAfter)
{
	assert(PathType(pSelf) == BLK_JMP || PathType(pSelf) == BLK_JMPIF);

	HOP pOpFrom = GetLastOp(pSelf);
	if (GetOwnerBody(pOpFrom) != GetOwnerBody(pOpTo))//????
		return 0;//assure they are inside of the same function
	if (pOpTo == pOpFrom)
		return 0;

	HPATH pPathDest = PathOf(pOpTo);
	HPATH pPathDest2 = PathRef(pOpFrom);

	if (bAfter)
	{
		HOP pOpPr = pOpTo;
		if (!IsGoto(pOpPr))
			if (!IsRetOp(pOpPr))//?
				return 0;//only after code flow break is allowed!

		HPATH pPathNew = SplitAfter(pPathDest, BLK_JMP);

		//insert new goto op
		HOP pOp = NewPrimeOp();

		pOp->Setup3(OPC_ADDRESS | OPC_GLOBAL, MAKETYP_PTR(mrDC.PtrSize()), 0);
		pOp->setAction(ACTN_GOTO);
		AddPathRef(pPathDest2, pOp);
		LinkOpTail(pPathNew, pOp);

		SetChangeInfo(FUI_NUMBERS);
		pOpTo = pOp;
		pPathDest = pPathNew;
	}
	else
	{
		if (!IsGoto(pOpTo))
			return 0;
		if (!IsAddr(pOpTo))
			return 0;

		BranchTracer_t an(*this);
		if (an.__traceGoto(pOpFrom) != an.__traceGoto(pOpTo))
			return 0;//is destination the same?

		if (PRIME(pPathDest->ops().Head()) != pOpTo)//check for need to create a new path
		{
			HPATH pPathNew = SplitAfter(pPathDest, BLK_JMP);
			LinkOpTail(pPathNew, pOpTo);
			SetPathType(pPathDest, BLK_NULL);
			pPathDest = pPathNew;
		}
	}
	//	else 
	//	{
	//		assert(false);
	//		HPATH pPathOld = m_pGoto->Path();
	//		if (PathType(pPathOld) == BLK_RET)
	//		{
	//			if (!pPathOld->Compare(pPathDest))
	//				return 0;
	//		}
	//		else
	//			return 0;
	//	}


	//	pPathDest->RelinkOps();

	if (PathRef(pOpFrom) != pPathDest)
	{
		ReleasePathRef(PathRef(pOpFrom), pOpFrom);
		AddPathRef(pPathDest, pOpFrom);
	}

	return 1;
}

int SwitchTracer_t::CreateSwitchIndexTable(FieldPtr pIndexTable, HPATH pJumpTablePath)
{
	assert(pJumpTablePath);
	if (GetIndexTablePath(pJumpTablePath))
		return -1;//already?

	OFF_t oTable;
	if (!GetRawOffset(pIndexTable, oTable))
		ASSERT0;

	Array_t *pTypeITable(pIndexTable->type()->typeArray());
	if (!pTypeITable)
		return 0;

	TypePtr iTypeEl = pTypeITable->baseType();

	//first make sure all entries are valid
	size_t nJumpEntriesNum(1);

	FieldPtr pJumpTable(PathLabel(pJumpTablePath));
	if (pJumpTable && pJumpTable->type()->typeArray())
		nJumpEntriesNum = pJumpTable->type()->typeArray()->total();

//	PDATA pRawData(mr)
	//assert(pRawData);

	OFF_t o(oTable);
	int sz(iTypeEl->size());
	assert(sz <= sizeof(value_t));

	for (size_t i(0); i < pTypeITable->total(); i++, o += sz)
	{
		value_t v;
		v.clear();
		GetDataSource()->pvt().dataAt(o, sz, (PDATA)&v);
		if (v.i32 >= (int)nJumpEntriesNum)
			return 0;
	}

	//	m_pIData.seekg(m_pIData, ios::beg);

	int offs(0);
	o = oTable;
	for (size_t i(0); i < pTypeITable->total(); i++, o += sz, offs += sz)
	{
		value_t v;
		GetDataSource()->pvt().dataAt(o, sz, (PDATA)&v);
		HOP pOpNew(NewPrimeOp());
		ADDR va(pIndexTable->_key() + offs);
		SetOpVA(pOpNew, va);
		HPATH pDataPath(AssureIndexTablePath(pJumpTablePath));//, pIndexTable));
		addIOp(pDataPath, pOpNew);
		pOpNew->Setup4(OPC_IMM, pIndexTable->OpTypeZ(), 0, v.ui32);
	}

	return 1;
}

HPATH SwitchTracer_t::GetSwitchBlock(HOP pSelf)
{
	assert(IsDataOp(pSelf));
	HOP *ppOpTblRef = 0;
	HOP pOpSw = GetSwitchOp(PathOf(pSelf), ppOpTblRef);
	assert(pOpSw);
	HPATH pBlock = PathOf(pOpSw);
	if (PathType(pBlock->Parent()) == BLK_SWITCH)
		return pBlock;
	return HPATH();
}

HOP SwitchTracer_t::addIOp(HPATH pPath, HOP pOp)
{
	assert(PathType(pPath) == BLK_DATA);
	assert(!pOp->isLinked());
	assert(IsPrimeOp(pOp));
	LinkOpTail(pPath, pOp);
	return pOp;
}

HPATH SwitchTracer_t::AssureSwitchTablePath(HPATH pSwitchPath)//, FieldPtr pTable)
{
	assert(pSwitchPath);
	HPATH pPath(GetJumpTablePath(pSwitchPath));
	if (!pPath)
	{
		pPath = NewPath();//this path must be under control of some function
		pPath->setFlags(BLK_DATA);
		//SetPathAnchor(pPath, pTable->address());
		HPATH pParent(pSwitchPath->Parent());
		TreeInsertChildAfter(pParent, pPath, pSwitchPath);
	}
	return pPath;
}

HPATH SwitchTracer_t::AssureIndexTablePath(HPATH pSwitchTablePath)//, FieldPtr pTable)
{
	assert(pSwitchTablePath);
	HPATH pPath(GetIndexTablePath(pSwitchTablePath));
	if (!pPath)
	{
		pPath = NewPath();//this path must be under control of some function
		pPath->setFlags(BLK_DATA);
		//SetPathAnchor(pPath, pTable->address());
		HPATH pParent(pSwitchTablePath->Parent());
		TreeInsertChildAfter(pParent, pPath, pSwitchTablePath);
	}
	return pPath;
}


