#include "anal_branch.h"
#include "prefix.h"
#include "shared/defs.h"
#include "shared/action.h"
#include "path.h"
#include "op.h"
#include "type_funcdef.h"
#include "anal_switch.h"
#include "anal_local.h"
#include "anal_data.h"
#include "anal_init.h"
#include "clean_ex.h"


BranchTracer_t::BranchTracer_t(const FuncTracer_t &r)
	: FuncTracer_t(r),
	mbForce(false)
{
}

BranchTracer_t::BranchTracer_t(const FuncInfo_t &fi, PathOpTracer_t &tr)
	: FuncTracer_t(fi, tr),
	mbForce(false)
{
}

/////////////////////////////
// E N D L E S S   L O O P

int BranchTracer_t::TurnEndlessLoop_On(HPATH pSelf)
{
	if (PathType(pSelf) != BLK_JMP)
		return 0;//toggle is possible only on jumps

	if (PathType(pSelf->Parent()) == BLK_LOOPENDLESS)
		return -1;//already

	if (PathType(pSelf->Parent()) == BLK_IF)
		return 0;

	HPATH pPath1 = GetGotoPath(pSelf);
	HPATH pPath2 = pSelf;

	bool jmpback(JumpDir(pSelf, pPath1));
	if (!jmpback)
		return 0;

	pPath1 = pPath2->GetOnLevelPath(pPath1, -1);
	if (!pPath1)
		return 0;

	HPATH pBlock = BlockOut(pPath1, pPath2);
	assert(pBlock);
	SetPathType(pBlock, BLK_LOOPENDLESS);
	//pBlock->RemoveRedundantParent();

	SetChangeInfo(FUI_LOCALSPOS);
	return 1;
}

int BranchTracer_t::TurnEndlessLoop_Off(HPATH pSelf)
{
	if (PathType(pSelf) != BLK_JMP)
		return 0;//toggle is possible only on jumps

	if (PathType(pSelf->Parent()) != BLK_LOOPENDLESS)
		return -1;//already

	BlockIn(pSelf->Parent());
	return 1;
}

int BranchTracer_t::ToggleEndlessLoop(HPATH pSelf)
{
	assert(pSelf->IsTerminal());

	int nResult = TurnEndlessLoop_On(pSelf);
	if (nResult < 0)//already
		nResult = TurnEndlessLoop_Off(pSelf);

	return (nResult > 0) ? (1) : (0);
}


/////////////////////////////
// < I F >

int BranchTracer_t::CheckIfValid(HPATH pSelf)
{
	assert(PathType(pSelf) == BLK_IF);

	HPATH pPath1 = pSelf->Children().Head();			//condition path
	HPATH pPath2 = pSelf->Children().Tail();	//block path

	HPATH pPathNo;
	HPATH pPathYes;
	if (__GetOutPaths(pPath1, &pPathNo, &pPathYes) != 2)
	{
		assert(false);
	}

	HPATH pPath = GetClosingPath(pPath1, pPathYes);
	if (!pPath)
		return 0;

	return 1;
}


int BranchTracer_t::__GetOutPaths(HPATH pSelf, HPATH *ppPathNo, HPATH *ppPathYes) const
{
	if (!pSelf->IsTerminal())
	{
		if (PathType(pSelf) & BLK_LOGIC)
			return __GetOutPaths(pSelf->GetChild(1), ppPathNo, ppPathYes);
		if (PathType(pSelf) == BLK_IF)
			return __GetOutPaths(pSelf->GetChild(0), ppPathNo, ppPathYes);
		return 0;
//			assert(false);
	}

	assert(pSelf->IsTerminal());

//	if (IsDead())
//		return NextEx()->__GetOutPaths(ppPathNo, ppPathYes);

	if (PathType(pSelf) == BLK_JMPIF)
	{
		*ppPathNo = TreeNextEx(pSelf);
		*ppPathYes = GetGotoPath(pSelf);
		return 2;
	}
//	else if (PathType(pSelf) == BLK_RET)
//	{
//		*ppPathNo = 0;
//		return -1;
//	}
	if (PathType(pSelf) == BLK_JMP)
	{
		*ppPathNo = GetGotoPath(pSelf);
		return 3;//GOTO
	}
	if (PathType(pSelf) == BLK_JMPSWITCH)
	{
		*ppPathNo = HPATH();
		return -1;
	}
	*ppPathNo = TreeNextEx(pSelf);
	return 1;
}

int BranchTracer_t::GetOutPaths(HPATH pSelf, HPATH *ppPathNo, HPATH *ppPathYes) const
{
	*ppPathNo = HPATH();
	*ppPathYes = HPATH();

	HPATH pPath = pSelf;
	while (/*pPath->Parent() && */PathType(pPath->Parent()) & BLK_LOGIC)
		pPath = pPath->Parent();

	return __GetOutPaths(pPath, ppPathNo, ppPathYes);
}

int BranchTracer_t::TurnBlockIf_On(HPATH pSelf)
{
	assert(pSelf->IsTerminal());
	if (PathType(pSelf) != BLK_JMPIF)
		return 0;//toggle is possible only on condjumps

	//	CHECK(No() == 20)
	//		STOP

	HPATH pPath = GetLogicsTop(pSelf, 1);//check first
	if (!pPath)
		return 0;

	HPATH pPathNo;
	HPATH pPathYes;
	if (GetOutPaths(pSelf, &pPathNo, &pPathYes) != 2)
	{
		assert(false);
	}

	if (!pPathNo || !pPathYes)
		return 0;

	HPATH pPath1 = pPath;
	HPATH pPath2 = pPathYes;

	bool jmpback(JumpDir(pSelf, pPath2));
	if (jmpback)
	{//backward
		if (pPath->Parent())
			if (PathType(pPath->Parent()) == BLK_LOOPDOWHILE)
				if (pPath == pPath->Parent()->Children().back())//is pPath really of that loop?
					return -2;
		if (!pPath1)
			return 0;

		pPath2 = pPath1->GetOnLevelPath(pPath2, -1);

		if (!pPath2)
			return 0;

		HPATH pBlock = BlockOut(pPath1, pPath2);
		assert(pBlock);
		SetPathType(pBlock, BLK_LOOPDOWHILE);
		RemoveRedundantParent(pBlock);
		return 2;
	}

	//forward
	HPATH pParent(pPath->Parent());
	if (pParent)
	{
		if (PathType(pParent) == BLK_IF)
			return -1;//already
		if (pParent->IsIfWhileOrFor())
			if (pParent->FirstChild() == pPath)
				return -1;
	}

	pPath1 = TreeNextSibling(pPath1);
	if (!pPath1)
		return 0;//last
	//?	pPath2 = pPath2->PrevEx();
	pPath2 = GetClosingPath(pPath, pPathYes);
	if (!pPath2)
		return 0;

	HPATH pBlock = pPath1;
	if (pPath1 != pPath2)
	{
		pBlock = BlockOut(pPath1, pPath2);
		assert(pBlock);
	}
//?	pBlock->m.m_bBraced = 1;
	pBlock = BlockOut(pPath, pBlock);
	assert(pBlock);
	SetPathType(pBlock, BLK_IF);
	RemoveRedundantParent(pBlock);
	return 1;
}

int BranchTracer_t::TurnBlockIf_Off(HPATH pSelf)
{
	assert(pSelf->IsTerminal());
	if (PathType(pSelf) != BLK_JMPIF)
		return 0;//toggle is possible only on condjumps

	HPATH pPath = GetLogicsTop(pSelf, 1);//check first
	if (!pPath)
		return 0;

	HPATH pBlock = pPath->Parent();
	if (!pBlock)
		return 0;

	if (PathType(pBlock) == BLK_IF)
	{
		if (pBlock->FirstChild() != pPath)
			return 0;

		assert((PathType(pPath) == BLK_JMPIF) || (PathType(pPath) & BLK_LOGIC));
		assert(pBlock->CheckChildrenCount(2) == 0);
		//		HPATH pBlockIf = pBlock->Childs()->Last();
		//		if (!PathType(pBlockIf) && pBlockIf->Childs())
		//			BlockIn(pBlockIf);
	}
	else if (PathType(pBlock) != BLK_LOOPDOWHILE)
		return 0;

	HPATH pParent = pBlock->Parent();
	/*	if ( pParent
	&& (PathType(pParent) == BLK_IFELSE || PathType(pParent) == BLK_IF) )
	{
	if (pBlock->IsFirst())
	{
	BlockIn(pBlock);
	pBlock = pParent;
	}
	if (pBlock->Parent()
	&& (PathType(pBlock->Parent()) == BLK_IF || PathType(pBlock->Parent()) == BLK_IFELSE))
	SetPathType(pBlock, BLK_NULL);
	else
	BlockIn(pBlock);
	}
	else
	{
	BlockIn(pBlock);
	}*/

	if (PathType(pParent) == BLK_IFELSE)
	{
		if (!TreeIsLast(pBlock))
			TurnBlockElse_Off(pSelf);
	}
	else if (PathType(pParent) == BLK_SWITCH_0)
	{
		HPATH pSwitch = pBlock->GetChildLast();
		HPATH pJmpSwitch = pSwitch->GetChildFirst();
		SwitchTracer_t anlz(*this);
		anlz.TurnSwitch_Off(pJmpSwitch);
	}

	BlockIn(pBlock);
	//	pBlock->RemoveRedundantParent();
	return 1;
}

int BranchTracer_t::ToggleBlockIf(HPATH pSelf)
{
	assert(pSelf->IsTerminal());
	int nResult = TurnBlockIf_On(pSelf);
	if (nResult < 0)//already
		nResult = TurnBlockIf_Off(pSelf);

	nResult = (nResult > 0) ? (1) : (0);
	//?	if (nResult)
	//?		mrFunc.Redump();
	return nResult;
}

////////////////////
// < W H I L E >

int BranchTracer_t::TurnWhile_On(HPATH pSelf)
{
	assert(pSelf->IsTerminal());
	if (PathType(pSelf) != BLK_JMPIF)
		return 0;

	HPATH pPath = GetLogicsTop(pSelf, 1);//check first
	if (!pPath)
		return 0;

	HPATH pBlockIf = pPath->Parent();
	if (!pBlockIf)
		return 0;

	if (PathType(pBlockIf) == BLK_IFWHILE)
		return -1;//already

	if (PathType(pBlockIf) != BLK_IF)
		return 0;

	HPATH pBlockDoWhile = pBlockIf->LastChild();
	if (IsNull(pBlockDoWhile))
	{
		pBlockDoWhile = pBlockDoWhile->FirstChild();
		while (!TreeIsLast(pBlockDoWhile))
		{
			if (CheckActualOpsCount(pBlockDoWhile, 0) != 0)
				return 0;
			pBlockDoWhile = TreeNext(pBlockDoWhile);
		}
	}

	if (PathType(pBlockDoWhile) != BLK_LOOPDOWHILE)
		return 0;

	//enter condition
	HPATH rCondEnter = pBlockIf->FirstChild();
	//exit condition
	HPATH rCondExit = pBlockDoWhile->LastChild();

	//compare conditions

	ExprCache_t a(PtrSize());
	EXPR_t expr(*this, HOP(), adcui::DUMP_BLOCKS, a);
	if (!expr.CompareConditions(rCondEnter, rCondExit))
		return 0;

	SetPathType(pBlockIf, BLK_IFWHILE);

	SetChangeInfo(FUI_LOCALSPOS);
	return 1;
}

int BranchTracer_t::TurnWhile_Off(HPATH pSelf)
{
	assert(pSelf->IsTerminal());
	if (PathType(pSelf) != BLK_JMPIF)
		return 0;

	HPATH pPath = GetLogicsTop(pSelf, 1);//check first
	if (!pPath)
		return 0;

	HPATH pBlockWhile = pPath->Parent();
	if (!pBlockWhile)
		return 0;

	if (PathType(pBlockWhile) != BLK_IFWHILE)
		return -1;//already

	SetPathType(pBlockWhile, BLK_IF);
	return 1;
}

int BranchTracer_t::ToggleWhile(HPATH pSelf)
{
	assert(pSelf->IsTerminal());

	int nResult = TurnWhile_On(pSelf);
	if (nResult == -1)//already
		nResult = TurnWhile_Off(pSelf);

	return (nResult == 1) ? (1) : (0);
}


////////////////////////////////
// < F O R >

int BranchTracer_t::TurnForLoop_On(HPATH pSelf)
{
	assert(pSelf->IsTerminal());
	if (PathType(pSelf) != BLK_JMPIF)
		return 0;

	HPATH pPath = GetLogicsTop(pSelf, 1);//check first
	if (!pPath)
		return 0;

	HPATH pBlockIf = pPath->Parent();
	if (!pBlockIf)
		return 0;

	if (PathType(pBlockIf) != BLK_IFWHILE)
	{
		if (PathType(pBlockIf) != BLK_IFFOR)
			return 0;
		return -1;
	}

	SetPathType(pBlockIf, BLK_IFFOR);
	return 1;
}

int BranchTracer_t::TurnForLoop_Off(HPATH pSelf)
{
	assert(pSelf->IsTerminal());
	if (PathType(pSelf) != BLK_JMPIF)
		return 0;

	HPATH pPath = GetLogicsTop(pSelf, 1);//check first
	if (!pPath)
		return 0;

	HPATH pBlockIfFor = pPath->Parent();
	if (!pBlockIfFor)
		return 0;

	if (PathType(pBlockIfFor) != BLK_IFFOR)
		return -1;//already

	SetPathType(pBlockIfFor, BLK_IFWHILE);
	return 1;
}

int BranchTracer_t::ToggleForLoop(HPATH pSelf)
{
	assert(pSelf->IsTerminal());

	int nResult = TurnForLoop_On(pSelf);
	if (nResult == -1)//already
		nResult = TurnForLoop_Off(pSelf);

	return (nResult == 1) ? (1) : (0);
}

////////////////////////////////
// < E L S E >

int BranchTracer_t::TurnBlockElse_On(HPATH pSelf, bool bManual)
{
	assert(pSelf->IsTerminal());
	if (PathType(pSelf) != BLK_JMPIF)
		return 0;//toggle is possible only on condjumps

	//	CHECK(No() == 12)
	//	STOP

	HPATH pPath = GetLogicsTop(pSelf, 1);//check first
	if (!pPath)
		return 0;

	pPath = pPath->Parent();
	if (!pPath)
		return 0;//no TRUE block

	if (PathType(pPath) != BLK_IF)
	{
		if (!bManual)
			return 0;
		if (PathType(pPath) != BLK_IFELSE)
			return 0;
		SetPathType(pPath, BLK_IF);
		return 1;
	}

	assert(pPath->CheckChildrenCount(2) == 0);

	//	if (PathType(pPath->m_pChilds->Last()) == BLK_SWITCH)
	//		return 0;//not needed - parent is BLK_SWITCH_0

	if (TreeIsLast(pPath))
		return 0;

	if (pPath->Parent())
		if (PathType(pPath->Parent()) == BLK_IFELSE)
			return -1;//already

	HPATH pPathLast = TreeTerminalLast(pPath);
	if (PathType(pPathLast) != BLK_JMP)
	{
		if (!bManual)
			return 0;
		SetPathType(pPath, BLK_IFELSE);
		return 1;
	}

	HPATH pPath1 = pPath;
	HPATH pPath2 = HPATH();

	HPATH pPathD(GetGotoPath(pPathLast));
	if (!pPathD)
		return 0;
	if (PathType(pPathD) == BLK_EXIT)
		if (!bManual)
			return 0;//return path

	bool bJmpBack(JumpDir(pPathLast, pPathD));
	if (bJmpBack)
		return 0;
	pPath2 = GetClosingPath(pPath, pPathD);
	if (!pPath2)
		return 0;

	if (pPath2 == pPath1)
		return 0;//<else> block is ultimately dead!

	pPath1 = TreeNext(pPath1);

	HPATH pBlock = pPath1;//ELSE
	if (pPath1 != pPath2)
	{
		pBlock = BlockOut(pPath1, pPath2);
		assert(pBlock);
	}

	pBlock = BlockOut(pPath, pBlock);
	assert(pBlock);
	SetPathType(pBlock, BLK_IFELSE);

	RemoveRedundantParent(pBlock);
	return 1;
}

/*
if (0)
{
pPath2 = pPathLast->GetGotoPath();
pPath2 = pPath2->PrevEx();
while (pPath2)
{
if (pPath2->pp List == pPath1->pp List)
break;
if ( !pPath2->IsLast() )
return 0;//jump inside of block
pPath2 = pPath2->Parent();
}

if (!pPath2)
return 0;

if (!pPath->LevelOff(&pPath2))
return 0;
}
else
{*/

void BranchTracer_t::CheckChildsValidity(HPATH pSelf)
{
	HPATH pPath = pSelf->FirstChild();
	while (pPath)
	{
		HPATH pPathNx = TreeNext(pPath);
		CheckChildsValidity(pPath);
		if (PathType(pPath) == BLK_IF)
		{
			if (!CheckIfValid(pPath))
				BlockIn(pPath);
		}
		//else if (PathType(pPath) == BLK_LOOPDOWHILE)
		pPath = pPathNx;
	}
}

int BranchTracer_t::TurnBlockElse_Off(HPATH pSelf, bool bManual)
{
	assert(pSelf->IsTerminal());
	if (PathType(pSelf) != BLK_JMPIF)
		return 0;//toggle is possible only on condjumps

	HPATH pPath = GetLogicsTop(pSelf, 1);//check first
	if (!pPath)
		return 0;

	HPATH pPathIf = pPath->Parent();
	if (!pPathIf || PathType(pPathIf) != BLK_IF)
		return 0;
	assert(pPathIf->CheckChildrenCount(2) == 0);

	HPATH pPathIfElse = pPathIf->Parent();
	if (!pPathIfElse || PathType(pPathIfElse) != BLK_IFELSE)
		return 0;
	assert(pPathIfElse->CheckChildrenCount(2) == 0);

	HPATH pBlockElse = pPathIfElse->LastChild();
	if (!PathType(pBlockElse) && pBlockElse->HasChildren())
		BlockIn(pBlockElse);
	/*
	pParent = pPathIfElse->Parent();
	assert(pParent);
	if (PathType(pParent) == BLK_IFELSE)
	{
	if (!pPathIfElse->IsFirst())
	SetPathType(pPathIfElse, BLK_NULL);
	else
	assert(false);
	}
	else
	BlockIn(pPathIfElse);
	*/
	HPATH pPathParent = pPathIfElse->Parent();
	BlockIn(pPathIfElse);

	CheckChildsValidity(pPathParent);
	//	HPATH pPathElse = pParent->m_pChilds->Last();
	//	pPathElse->Unlink();
	//	pPathElse->LinkAfter(pParent);
	//	pPathElse->m_pParent = pParent->Parent();
	return 1;
}

int BranchTracer_t::ToggleBlockElse(HPATH pSelf, bool bManual)
{
	assert(pSelf->IsTerminal());
	int res = TurnBlockElse_On(pSelf, bManual);
	if (res == -1)//already
	{
		if (!TurnBlockElse_Off(pSelf, bManual))
			return 0;
	}
	else if (res > 0)
	{

	}
	else //if (res == 0)
		return 0;

	//?	mrFunc.Redump();
	return 1;
}

///////////////////////////=>
//		if (A) goto L1;
//		[B]
//L1:	[C]
//////////////////////////=>
//		if (!A) goto L2;
//		goto L1;
//L2:	[B]
//L1:	[C]
//reverse/////////////////=>
//		if (A) goto L3;
//		goto L2;
//L3:	goto L1;
//L2:	[B]
//L1:	[C]
int BranchTracer_t::FlipCondJump(HOP pSelf)
{
	assert(IsCondJump(pSelf));

	HPATH pPath = GetLogicsTop(PathOf(pSelf), 1);//check first
	if (!pPath)
		return 0;

	if (pPath != PathOf(pSelf))
		return 0;//not supported with complex conditions

	if (PathType(pPath->Parent()) == BLK_IF)
		return 0;

	HPATH pPathY(PathRef(pSelf));
	HPATH pPathN = PathOf(NextPrimeEx(pSelf));
	bool bAdjustGoto2(PathType(pPathN) == BLK_JMP && GetLastOp(pPathN) == GetFirstOp(pPathN));

	HPATH pPathNew = SplitAfter(PathOf(pSelf), BLK_JMP);

	HOP pOp = NewPrimeOp();
	pOp->Setup3(OPC_ADDRESS|OPC_GLOBAL, MAKETYP_PTR(mrDC.PtrSize()), 0);
	pOp->setAction(ACTN_GOTO);
	LinkOpTail(pPathNew, pOp);
	//SetRoot(pOp, 1);

	InvertAction(pSelf);

	ReleasePathRef(pPathY, pSelf);

	AddPathRef(pPathY, pOp);
	//Ensu reLabel(pPathN);
	AddPathRef(pPathN, pSelf);

	assert(PrevPrimeEx(pOp) == pSelf);
	InitialTracer_t an(*this);
	an.Analize(pOp, pSelf);

/*	pOp = this;
	while (0)
	{
		if (pOp->AdjustGoto())
		{
			pOp = this;
			continue;
		}
		pOp = pOp->NextEx();
		if (!pOp)
			break;
		if (!pOp->IsGoto())
			if (!pOp->IsCondJump())
				break;
	}*/

	AdjustGoto(PathOf(pSelf));
//?	AdjustGoto(pOp->Path());
	//if (bAdjustGoto2)
		//AdjustGoto2(pOp->Path());
	AdjustGoto3(PathOf(pOp));

	SetChangeInfo(FUI_BASE0|FUI_NUMBERS);
	return 1;
}

int BranchTracer_t::FlipIfElse(HPATH pSelf)
{
	assert(pSelf->IsTerminal());

	if (PathType(pSelf) != BLK_JMPIF)
		return 0;//toggle is possible only on condjumps

	HPATH pPath = GetLogicsTop(pSelf, 1);//check first
	if (!pPath)
		return 0;

	if (PathType(pPath->Parent()) != BLK_IF)
	{
		HOP pOp = GetLastOp(pSelf);
		assert(IsCondJump(pOp));
		return FlipCondJump(pOp);
	}

	HPATH pParent(pSelf->Parent());
	if (pParent->CheckChildrenCount(3) != 0)
		return 0;

	HPATH pPathIf = pParent->GetChild(1);
	pParent->TakeChild(pPathIf);
	TreePushChildBack(pParent, pPathIf);

	pPath->invert();
	return 1;
}

//////////////////////////
// L O G I C S

HPATH BranchTracer_t::__rebuildLogics(HPATH pPath)
{
	pPath = GetLogicsTop(pPath, 0);
	if (!(PathType(pPath) & BLK_LOGIC))
		return pPath;

	HPATH pPath1 = pPath->FirstChild();
	HPATH pPath2 = pPath->LastChild();

	TurnLogics_Off(pPath);

	pPath1 = __rebuildLogics(pPath1);
	if (PathType(pPath2->Parent()) & BLK_LOGIC)
		pPath = HPATH();
	else
		pPath2 = __rebuildLogics(pPath2);

	HPATH pPathNew;
	if (pPath1 && TurnLogics_On(pPath1, &pPathNew))
		return pPathNew;
	else if (pPath2 && TurnLogics_On(pPath2, &pPathNew))
		return pPathNew;

	return HPATH();
}

int BranchTracer_t::CheckLogicsValid(HPATH pSelf)
{
	if (PathType(pSelf) != BLK_JMPIF)
		return 0;
	__rebuildLogics(pSelf);
	return 1;
}

int BranchTracer_t::TurnLogics_On(HPATH pSelf, HPATH *ppPathNew)
{
	if (PathType(pSelf) != BLK_JMPIF)
		if (!(PathType(pSelf) & BLK_LOGIC))
			return 0;

	HPATH pPath = GetLogicsTop(pSelf, 1);//check first
	if (!pPath)
		return 0;

	HPATH pPath1No;
	HPATH pPath1Yes;
	if (GetOutPaths(pPath, &pPath1No, &pPath1Yes) != 2)
		return 0;

	//CHECK(pPath->m_pOps->No() == 1051)
	//STOP

	HPATH pPathNx = TreeNext(pPath);
	if (!pPathNx)
		return 0;
	if (CheckActualXRefs(pPathNx))
		return 0;

	HPATH pPath2No;
	HPATH pPath2Yes;
	if (GetOutPaths(pPathNx, &pPath2No, &pPath2Yes) != 2)//IF
		return 0;

	if (!pPath2No || !pPath2Yes)//?
		return 0;

	if (CheckOpsCount(pPathNx, 1) != 0)
		return 0;

	if (!pPath1Yes)
		return 0;

	BlockTyp_t nType = BLK_NULL;
	if (Equal(pPath1Yes, pPath2No))
		nType = BLK_AND;
	else if (Equal(pPath1Yes, pPath2Yes))
		nType = BLK_OR;
	else
		return 0;

	if (PathType(pPath->Parent()) == BLK_IF)
	{
		HPATH pPathI = pPath;
		if (!pPathI->IsTerminal())//m_pChilds)
			pPathI = TreeTerminalFirst(pPathI);
		TurnBlockIf_Off(pPathI);
	}

	HPATH pPath1 = pPath;
	HPATH pPath2 = pPathNx;
	if (PathType(pPath2) == BLK_IF)
	{
		pPath2 = pPath2->FirstChild();
		HPATH pParent1(pPath1->Parent());
		HPATH pParent2(pPath2->Parent());
		pParent1->UnlinkChild(pPath1);
		TreeInsertChildBefore(pParent2, pPath1, pPath2);
	}

	HPATH pPathNew(BlockOut(pPath1, pPath2));
	SetPathType(pPathNew, nType);
	if (ppPathNew)
		*ppPathNew = pPathNew;

	RemoveRedundantParent(pPathNx);
	return 1;
}

int BranchTracer_t::TurnLogics_Off(HPATH pSelf)
{
	HPATH pPath = GetLogicsTop(pSelf, 1);//check first
	if (!pPath)
		return 0;

	if (!(PathType(pPath) & BLK_LOGIC))
		return 0;

	//	HPATH pPathParent = pPath->Parent();
	//	if ( !pPathParent 
	//		|| PathType(pPathParent) != BLK_IF
	//		|| !pPath->IsFirst() )
	//	{
	//		BlockIn(pPath);
	//		return 1;
	//	}

	if (PathType(pPath->Parent()) == BLK_IF)
		TurnBlockIf_Off(TreeTerminalFirst(pPath));

	HPATH pPath1 = pPath->FirstChild();
	HPATH pPath2 = pPath->LastChild();

	BlockIn(pPath);

	//	pPath1->Unlink();
	//	pPath2->Unlink();
	//	pPath2->SetParent(pPath->Parent());
	//	pPath2->LinkBefore(pPath);
	//	pPath->Unlink();
	//	delete pPath;

	//find to where insert pPath1
	pPath = pPath2;
	while (pPath->Parent())
	{
		if (PathType(pPath->Parent()) != BLK_IF && PathType(pPath->Parent()) != BLK_IFELSE)
			break;
		if (!TreeIsFirst(pPath))
			break;
		pPath = pPath->Parent();
	}

	HPATH pParent1(pPath->Parent());
	TreeInsertChildBefore(pParent1, pPath1, pPath);
	//pPath1->SetParent(pPath->Parent());

	if (pPath1->Parent())//?
	{
		HPATH pBlock = BlockOut(pPath1, pPath);
		if (pBlock->Parent())
			__KillRedundantChilds(pBlock->Parent());
	}

	return 1;
}

int BranchTracer_t::ToggleLogics(HPATH pSelf)
{
	assert(pSelf->IsTerminal());
	if (TurnLogics_On(pSelf) == -1)//already
		if (!TurnLogics_Off(pSelf))
			return 0;
	return 1;
}

int BranchTracer_t::Equal(HPATH pPath1, HPATH pPath2)
{
	while (pPath1->FirstChild())
		pPath1 = pPath1->FirstChild();
	while (pPath2->FirstChild())
		pPath2 = pPath2->FirstChild();
	return pPath1 == pPath2;
}

bool BranchTracer_t::JumpDir(HPATH pSelf, HPATH pPathDest) const//true if backward
{
	assert(pPathDest);
	HPATH pPath = pPathDest;
	do {
		if (pSelf->Parent() == pPath->Parent())
		{
			do {
				if (pPath == pSelf)
					return true;//backward
				pPath = TreeNext(pPath);
			} while (pPath);
			return false;//forward!
		}
		pPath = pPath->Parent();
	} while (pPath);
	
	assert(pSelf->Parent());
	return JumpDir(pSelf->Parent(), pPathDest);
}


int BranchTracer_t::Expand(HPATH pSelf)
{
	if (PathType(pSelf) != BLK_JMP)
		return 0;
	
	HOP pOpFrom = GetLastOp(pSelf);

	HPATH pPathDest = GetGotoPath(pSelf);
	//FieldPtr pLabelDest = pPathDest->m.Labe lAt();
	HOP pOpDest = GetFirstOp(pPathDest);

	if (PathType(pPathDest) == BLK_EXIT)
		return 0;

/*	switch (PathType(pPathDest->Parent()))
	{
	case BLK_BODY:
	case BLK_NULL:
	case BLK_CASE:
	case BLK_DEFAULT:
	case BLK_LOOPDOWHILE:
		break;
	default:
		return 0;
	}*/

//	UndoPathTree();
	LocalsTracer_t anLocals(*this);
//?	anLocals.DetachLocals(pPathDest);

	int res = 1;
	if (1)//!g_ pDI->isUnfoldMode())
	{
		if (!pOpDest->isHidden() && pOpDest->isRoot())
			res = 2;//stop expand info
	}

	ReleasePathRef(pPathDest, pOpFrom);
	//if (CheckDanglingPath(pPathDest))

	HPATH pPathDestNew;
	if (IsGoto(pOpDest))
	{
		pPathDestNew = PathRef(pOpDest);
		//HOP pOpDestNew(PRIME(PathOps(pPathDestNew).front()));
		//Ensur eLabel(pPathDestNew);
	}
	else
	{
		HOP pOpDestNew(NextPrimeEx(pOpDest));
		EnsurePathBreakAt(pOpDestNew);//split can occure here
		pPathDestNew = PathOf(pOpDestNew);
	}

	/*if (pOpDestNew)
	{
		pLabelDestNew = Ensure Label(pOpDestNew);
	}
	else
	{
		pPathDestNew = TreeNextEx(pPathDest);
		assert(pPathDestNew);
		pLabelDestNew = Ensur eLabel(pPathDestNew);
	}*/

	AddPathRef(pPathDestNew, pOpFrom);//pLabelDestNew

	if (PathOf(pOpDest)->headOp() != pOpDest || (GetEnterPathsNum(PathOf(pOpDest)) > 0))
	{
		HOP pOpClone = HOP();
		if (!IsGoto(pOpDest))//do not clone a goto
		{
			pOpClone = CloneOperation(pOpDest, false);
			LinkOpBefore(pOpClone, pOpFrom);
			pOpClone->setCloned(true);
		/*	if (CheckPa thBreak(pOpClone))
			{
				HPATH pPath2 = SplitAfter(pOpClone->Path(), BLK_JMP);
				OpList_t &l(pOpFrom->list());
				l.take(pOpFrom);
				pPath2->m.LinkOpTail(pOpFrom);
			}
*/
			//		while (pOpClone->AdjustGoto());

			//reanalize jump op
			InitialTracer_t an(*this);
			an.Analize(pOpFrom, pOpClone);

			if (!pOpDest->isHidden())
			{
				ExprCacheEx_t aExprCache(PtrSize());
				AnlzXDepsIn_t an1(*this, pOpDest, aExprCache);
				an1.TraceXDepsIn();
				//!pOpDest->TraceXDepsIn();//update xins of pOpDest
				AnlzXDepsIn_t an2(*this, pOpClone, aExprCache);
				an2.TraceXDepsIn();
				//!pOpClone->TraceXDepsIn();//update xins of pOpClone
				//			pOpClone->TraceXOutExtra();
				DuplicateXOuts(pOpDest, pOpClone);//clone xouts - just copy
			}
		}

		CheckDanglingPath(pPathDest);
	}
	else
	{
		/*if (pOpDest->IsGoto())
		{
			AdjustGoto3(pSelf);
		}
		else*/
		{
			//pPathDest = pOpDest->Path();
			if (PRIME(pPathDest->takeOpFront()) != pOpDest)
				ASSERT0;

			assert(!pPathDest->hasOps());
			LinkOpBefore(pOpDest, pOpFrom);//insert before pOpFrom

			/*if (pPathDest->m.Lab elAt())
			{
			assert(pPathDest->m.Labe lAt()->xrefs().empty());
			assert(0);//?			pPathDest->Lab elAt()->Unlink();
			delete pPathDest->m.Lab elAt();
			}
			delete pPathDest;
			LinkOpBefore(pOpDest, pOpFrom);
			if (CheckPa thBreak(pOpDest))
			{
			HPATH pPath2 = SplitAfter(pOpDest->Path(), BLK_JMP);
			pPath2->m.LinkOpTail(pOpFrom);
			}*/
		}

		CheckDanglingPath(pPathDest);

		if (IsGoto(pOpDest))
		{
			assert(0);
			//?if (IsAuxOp(pOpFrom))//?
			{
				FuncCleaner_t FD(*this);
				FD.DestroyPrimeOp(pOpFrom);
			}
			//else
			//	pOpFrom->ins().opnd_HIDDEN = 1;
		}
	}

//?	anLocals.CheckLocalsPos(pOpDest);
//	pOpFrom->Path()->AdjustGoto();

	SetChangeInfo(FUI_BASE0|FUI__BLOCKS|FUI_NUMBERS);
	return res;
}

int BranchTracer_t::DuplicateXOuts(HOP pSelf, HOP pOpRef)
{
	assert(pOpRef);

	for (XOpList_t::Iterator i(pSelf->m_xout); i; i++)
	{
		HOP pOp(i.data());
		if (IsCallOutOp(pOp))
		{
			HOP pOpOut = NewOp();
			InitFrom(pOpOut, pOp);
			MakeUDLink(pOpOut, pOpRef);
			pOpOut->setInsPtr(pOpRef->insPtr());
			PathOf(pOp)->insertOpAfter(INSPTR(pOpOut), INSPTR(pOp));
			DuplicateXOuts(pOp, pOpOut);
			continue;
		}
		MakeUDLink(pOp, pOpRef);
	}

	return 1;
}

int BranchTracer_t::GetEnterPathsNum(HPATH pSelf)
{
	assert(pSelf->IsTerminal());

	int n = 0;
	if (TreeIsFirstEx(pSelf))
		n++;
	else 
	switch (PathType(TreePrevEx(pSelf)))
	{
	case BLK_JMP:
	case BLK_JMPSWITCH:
//	case BLK_RET:
		break;
	default:
		n++;
	}

	n += PathOpRefs(pSelf).count();//CountGotoXRefs();
	return n;
}

int BranchTracer_t::ExpandEx(HPATH pSelf, bool bStep)
{
	bool bMulti = true;//!g_ pDI && !g_ pDI->isUnfoldMode();
	if (!bMulti)
		return Expand(pSelf);

	//HPATH pPathDest0 = 0;

	int res0 = 0;
	for (;;)
	{
		/*?HPATH pPathDest = GetGotoPath(pSelf);
		if (!pPathDest)
			break;
		if (!pPathDest0)
			pPathDest0 = pPathDest;
		else if (pPathDest != pPathDest0)
			break;*/

		int res = Expand(pSelf);
		if (res != 0)
			res0++;
		if (res != 1)
			break;//op is not root or error
		if (bStep)
			break;
	}

	return res0 > 0;
}

int BranchTracer_t::Collapse(HPATH pSelf)
{
	if (PathType(pSelf) != BLK_JMP)
		return 0;

	return 0;
}

////////////////////////
//		if (A) goto L1;
//		...
//L1:	goto L2;
//		...
//L2:
////////////////////////=>
//		if (A) goto L2;
//		...
//L1:	goto L2;	; <= path may become dangling
//		...
//L2:
int BranchTracer_t::AdjustGoto(HPATH pSelf)
{
	if (!pSelf->IsTerminal())
		return 0;

	if (PathType(pSelf) != BLK_JMP)
		if (PathType(pSelf) != BLK_JMPIF)
			return 0;

	HOP pOpLast = GetLastOp(pSelf);
	if (IsRetOp(pOpLast))
		return 0;

	HPATH pPathOld(PathRef(pOpLast));
	if (!pPathOld || PathOps(pPathOld).empty())
		return 0;

	if (/*!IsLa bel(pLabelOld) ||*/ !pPathOld || TreeRoot(pPathOld) != TreeRoot(pSelf))//?
	{
		//		assert(false);
		InitialTracer_t an(*this);
		an.Goto2Call(pOpLast);
		return 1;
	}

	int res = 0;
	HPATH pPathNew = __traceGoto(pOpLast);//new goto destination
	/*	if (pLabelNew->Path() == NextEx())
	{
	InvalidateGoto();//jump to next instruction
	res = 1;
	}*/

	if (pPathNew != pPathOld)
	{
		ReleasePathRef(pPathOld, pOpLast);
		AddPathRef(pPathNew, pOpLast);//new destination

		CheckDanglingPath(pPathOld);//the path may have become dangling
		//assume pLabelOld is no longer valid below
		//pLabelOld = nullptr;
		return 1;
	}

	//	if (!IsGoto())
	if (PathType(pSelf) != BLK_JMP)
		return res;

#if(0)//wtf is this???
	//this <=> unconditional jumps only
	FieldPtr pLabel = pOpLast->Label();
	if (pLabel)
	{//redirect jumps from this goto op to m_pGoto
		XOpList_t::Iterator i(pLabel->xrefs());
		while (i)
		{
			XOpList_t::Iterator iNx(i);
			iNx++;
			HOP pOp = i.data();
			ReleaseLabelRef(pLabel, pOp);
			AddLocalRef(pLabelOld, pOp);
			i = iNx;
			res = 1;
		}

		assert(pLabel->xrefs().empty());//no more xref to this op!
		//		res = 1;
	}
#endif

	HPATH pPathPr = TreePrevEx(pSelf);
	//	if (pOpLast->IsFirstEx())
	if (!pPathPr)
		return res;

	//	HOP pOpPr = PrevEx();//PrevRoot();
	//	if ( !pOpPr->IsCondJump() )
	/*	if (pPathPr->Type() != BLK_JMPIF)
	{
	//if (pOpPr->IsGoto() && IsAddr(pOpPr))
	if (GetEnterPathsNum() == 0)
	if (pPathPr->Type() == BLK_JMP || pPathPr->Type() == BLK_JMPSWITCH)
	{//prev op is goto! so invalidate this one
	int r = InvalidateGoto();
	if (r == 1)
	res = r;
	r = pPathPr->AdjustGoto();//?
	if (r == 1)
	res = r;
	}
	return res;
	}*/

	return res;
}

//		if (A) goto L1;
//		goto L2;
//L1:	[B];
//		...
//L2:	[C];
///////////////////////=>
//		if (!A) goto L2
//		goto L1;
//L1:	[B]
//		...
//L2:	[C]
///////////////////////=>
//		if (!A) goto L2
//		[B]
//		...
//L2:	[C]
int	BranchTracer_t::AdjustGoto2(HPATH pSelf)
{
	if (PathType(pSelf) != BLK_JMP)
		return 0;

	if (pSelf->ops().check_count(1) != 0)
		return 0;

	HPATH pPathPr = TreePrevEx(pSelf);
	if (!pPathPr || PathType(pPathPr) != BLK_JMPIF)
		return 0;

	HPATH pPathNx(GetGotoPath(pPathPr));
	if (!pPathNx || pPathNx != TreeNextEx(pSelf))
		return 0;

	HOP pOpPr = GetLastOp(pPathPr);
	HOP pOp = GetLastOp(pSelf);

	ReleasePathRef(pPathNx, pOpPr);
	//__traceGoto()->AddXRef(pOpPr);
	//pOp->__traceGoto()->AddXRef(pOpPr);
	AddPathRef(GetGotoPath(pSelf), pOpPr);
	InvertAction(pOpPr);

	ReleasePathRef(PathRef(pOp), pOp);
	AddPathRef(pPathNx, pOp);

	//	InvalidateGoto();
	return 1;
	//	return GetLastOp()->AdjustGoto(); 
}

///////////////////////=>
//		[A
//		goto L1;
//		]
//{L:}	[B]		;L-aux,unref'd
///////////////////////=>
//		[A+B]
int	BranchTracer_t::AdjustGoto3(HPATH pSelf)
{
	if (PathType(pSelf) != BLK_JMP)
		return 0;

	HPATH pPathTo(GetGotoPath(pSelf));
	assert(pPathTo);
	if (TreeNextEx(pSelf) != pPathTo)
		return 0;

	HOP pOp(GetLastOp(pSelf));

	{//RAII-block
	FuncCleaner_t CH(*this);
	CH.DestroyPrimeOp(pOp);

	//ReleaseLabelRef(pPathTo, pOp);
	if (!pSelf->hasOps())
		CH.DestroyPath(pSelf);
	}

	//assert(!pPathTo->m.Lab elAt());//fix later
	return CheckDanglingPath(pPathTo);
}

//returns terminal goto destination
HPATH BranchTracer_t::__traceGoto(HOP pSelf)
{
	assert(IsPrimeOp(pSelf));

	const PathOpList_t &l(PathOps(PathRef(pSelf)));
	if (!l.empty())
	{
		HOP rOp(PRIME(l.front()));
		if (IsGoto(rOp))
		{
			if (IsAddr(rOp))
			{
				if (!IsRetOp(rOp))
				{
					HPATH pPath(__traceGoto(rOp));
					if (pPath)
						return pPath;
				}
			}
		}
	}
	return PathRef(pSelf);
}

int BranchTracer_t::InvalidateGoto(HPATH pSelf)
{
	assert(PathType(pSelf) == BLK_JMP || PathType(pSelf) == BLK_JMPIF);
	HOP pOpLast = GetLastOp(pSelf);
	InitialTracer_t an(*this);
	an.SetHidden(pOpLast);
	Delete(pOpLast);

	SetPathType(pSelf, BLK_NULL);
	return 1;
}

void BranchTracer_t::RemoveRedundantParent(HPATH pSelf)
{
	HPATH  pParent = pSelf->Parent();
	if (!pParent)
		return;
	
	if (PathType(pParent) != BLK_NULL)
		return;

	if (pParent->CheckChildrenCount(1) == 0)
	{
		pParent->TakeChild(pSelf);
		HPATH pGrandParent(pParent->Parent());
		TreeInsertChildAfter(pGrandParent, pSelf, pParent);
		//pSelf->SetParent(pParent->Parent());
		//?assert(!pParent->m.hasLocals());
//		pParent->DetachLocals();
		pGrandParent->TakeChild(pParent);
		Delete(pParent);
	}
}