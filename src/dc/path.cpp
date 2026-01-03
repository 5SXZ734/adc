#include "path.h"
#include "prefix.h"
#include "shared/defs.h"
#include "shared/link.h"
#include "shared/misc.h"
#include "db/mem.h"
#include "db/field.h"
#include "db/proj.h"
#include "arglist.h"
#include "op.h"
#include "ana_data.h"
#include "ana_ptr.h"
#include "ana_expr.h"
#include "ana_switch.h"
#include "ana_branch.h"
#include "ana_local.h"
#include "info_dc.h"
#include "expr.h"
#include "clean_ex.h"
#include "cc.h"

//int g_zzz = 0;

#ifdef _DEBUG
static unsigned g_pathCount = 0;
#endif

Path_t::Path_t()//BlockTyp_t nType)
#if(PATH_ID)
	: mID(g_pathCount++)
#endif
{
#if(PATH_ID)
CHECK(mID == 0x4)
STOP
#endif

	m_flags = 0;
#if(!NEW_PATH_TRACER)
		m_traceInfo.flags = 0;
#endif
	_mVA = 0;//setAnchor(0);

MEMTRACE_ADD;
}

Path_t::~Path_t()
{
	assert(ops().empty());
	assert(mInflow.empty());

MEMTRACE_RELEASE;
}



bool FuncInfo_t::CheckNamesClash(CHOP, CFieldPtr pData) const
{
	return false;
}

bool FuncInfo_t::__checkNamesClash(CHPATH pSelf, CFieldPtr pData) const
{
	for (PathTree_t::ChildrenIterator i(pSelf); i; i++)
	{
		HPATH pPath(i.data());
		int n = CheckContainer(pPath);
		if (n != 0)
			continue;
		if (__checkNamesClash(pPath, pData))
			return true;//conflict!
	}

	for (Path_t::OpIterator i(pSelf->ops()); i; i++)
	{
		if (CheckNamesClash(PRIME(i.data()), pData))
			return true;
	}

	return false;
}

bool FuncInfo_t::CheckNamesClash(CHPATH pSelf, CFieldPtr pData) const
{
	//resolve name conflicts
	HPATH pPath0 = pSelf;//GetContainerPath();
	int n0 = 0;
	while (1)
	{
		n0 = CheckContainer(pPath0);
		if (n0 != 0)
			break;
		pPath0 = pPath0->Parent();
	}

	return __checkNamesClash(pPath0, pData);
}

/*FieldPtr  Path_t::FindLocal(int offset)
{
	if (hasLocals())
	for (FieldMapCIt it = locals().begin(); it != locals().end(); it++)
	{
		FieldPtr  pField = VALUE(it);
		if (pField->Offset() == offset)
			return pField;
	}

	return nullptr;
}*/

/*HPATH Path_t::NextExValid()
{
	HPATH pPath = lin ked();
	do {
		pPath = pPath->NextEx();
	} while (pPath && !pPath->IsDead_());
	return pPath;
}

HPATH Path_t::PrevExValid()
{
	HPATH pPath = lin ked();
	do {
		pPath = pPath->PrevEx();
	} while (pPath && !pPath->IsDead_());
	return pPath;
}*/


int FuncInfo_t::IsRedundant(CHPATH pSelf) const
{
	if (!IsNull(pSelf))
		return 0;//only empty and not terminal blocks may be redundant
	if (!pSelf->Parent())
		return 1;
	if (PathType(pSelf->Parent()) == BLK_IFELSE || PathType(pSelf->Parent()) == BLK_IF)
		if (pSelf->CheckChildrenCount(1) > 0)
			return 0;
	return 1;
}




int FuncInfo_t::IsRetPath(CHPATH pSelf) const
{
	if (PathType(pSelf) != BLK_JMP)
		return 0;
	if (GetGotoPath(pSelf)->Type() == BLK_EXIT)
		return 1;
	return 0;
}

bool FuncInfo_t::IsPathDegenerate(CHPATH hPath) const
{
	return mrFuncDef.pathTree().isPathDegenerate(hPath);
}

HPATH FuncInfo_t::__CheckBlock(CHPATH hSelf, CHPATH hPath0) const
{
	HPATH hPath(hPath0);
	if (hSelf->Parent() == hPath->Parent())
		return hPath;

	while (hPath->IsTerminal())
	{
		if (!IsPathDegenerate(hPath))
			return HPATH();
		hPath = TreePrevSibling(hPath);
		if (!hPath)
			return HPATH();
	}

	if (PathType(hPath) == BLK_IFELSE)
	{
		HPATH pPathFirst(hPath->FirstChild());
		HPATH pPathD(__CheckBlock(hSelf, pPathFirst));
		if (pPathD && pPathD != pPathFirst)
			return pPathD;
	}
	else if (PathType(hPath) & BLK_LOOP)
	{
		return HPATH();
	}

	return __CheckBlock(hSelf, hPath->Children().back());
}

HPATH FuncInfo_t::GetClosingPath(CHPATH hSelf, CHPATH pPath0) const//destination path
{
	assert(pPath0->IsTerminal());
//	return pPath->PrevEx();

	if (pPath0->Type() == BLK_EXIT)
		return HPATH();

	HPATH pPath(pPath0);
	do {
		HPATH pParent(pPath->Parent());
		//if (!pPath->IsFirst())
		if (pPath != pParent->FirstChild())
			break;
//		if (!pPath->Parent())
//			break;
		pPath = pParent;
	} while (pPath);

	if (!pPath)
		return HPATH();

	pPath = TreePrevSibling(pPath);
	return __CheckBlock(hSelf, pPath);
}





int FuncInfo_t::CheckValid(CHPATH hSelf)
{
	for (PathTree_t::ChildrenIterator i(hSelf); i; i++)
		if (!CheckValid(i.data()))
			return 0;
	if (!hSelf->HasChildren() && hSelf->ops().empty())
		return 0;
	return 1;
}


void FuncInfo_t::LinkOpHead(HPATH hSelf, HOP pOp) const
{
	//bool b = IsPrimeOp(pOp);
	assert(IsPrimeOp(pOp));
	assert(!INSPTR(pOp)->isLinked());
	hSelf->pushOpFront(INSPTR(pOp));
	pOp->ins().mpPath = hSelf;
}

void FuncInfo_t::LinkOpTail(HPATH hSelf, HOP pOp) const
{
	//bool b = IsPrimeOp(pOp);
	assert(IsPrimeOp(pOp));
	assert(!INSPTR(pOp)->isLinked());
	hSelf->pushOpBack(INSPTR(pOp));
	pOp->ins().mpPath = hSelf;
}

void FuncInfo_t::LinkOpAfter(HPATH hSelf, HOP pOp, HOP pOpAfter) const
{
	assert(hSelf->ops().contains(INSPTR(pOpAfter)));
	hSelf->insertOpAfter(INSPTR(pOp), INSPTR(pOpAfter));
	assert(IsPrimeOp(pOp));
	pOp->ins().mpPath = hSelf;
}

void FuncInfo_t::LinkOpBefore(HPATH hSelf, HOP pOp, HOP pOpBefore) const
{
	assert(hSelf->ops().contains(INSPTR(pOpBefore)));
	hSelf->insertOpBefore(INSPTR(pOp), INSPTR(pOpBefore));
	assert(IsPrimeOp(pOp));
	pOp->ins().mpPath = hSelf;
}







/*FieldPtr FuncInfo_t::LabelPrev(HPATH hSelf)
{
	assert(hSelf->IsTerminal());

	for (HPATH pPath = hSelf; pPath; pPath = TreePrevEx(pPath))
	{
		if (pPath->m.La belAt())
			return pPath->m.Lab elAt();
	}
	return 0;
}*/

MyString FuncInfo_t::Name(CHPATH hSelf, bool bShort) const
{
	static const TypStr_t block_str[] = {
	{BLK_IF,			"IF",			"i"},
	{BLK_IFELSE,		"IFELSE",		"e"},
	{BLK_IFWHILE,		"IFWHILE",		"w"},
	{BLK_IFFOR,			"IFFOR",		"f"},
	{BLK_LOOPDOWHILE,	"LOOPDOWHILE",	"dw"},
	{BLK_LOOPENDLESS,	"LOOPENDLESS",	"wx"},
	{BLK_SWITCH_0,		"SWITCH_0",		"s0"},
	{BLK_SWITCH,		"SWITCH",		"s"},
	{BLK_DEFAULT,		"DEFAULT",		"d"},
	{BLK_CASE,			"CASE",			"c"},
	{BLK_AND,			"AND",			"a"},
	{BLK_OR,			"OR",			"o"},
	{BLK_BODY,			"BODY",			"BD"},
	{BLK_ENTER,			"ENTER",		"NT"},
	{BLK_EXIT,			"EXIT",			"XT"},
	{BLK_DATA,          "DATA",         "D"},
//	{BLK_NULL,			"NULL",			""},
	{-1}
	};

	MyString s;

	const TypStr_t *p(block_str->get(hSelf->Type()));
	if (p)
	{
		if (bShort)
			s = p->str2;
		else
			s = p->str;
	}
	
	if (!hSelf->ops().empty() && hSelf->Type() != BLK_ENTER)
	{
		HOP pOp(GetFirstOp(hSelf));
		assert(pOp);
		assert(IsPrimeOp(pOp));
		if (!s.empty())
			s.append(":");
		s.append("[" + StrNo(pOp) + "]");
	}

	if (s.empty())
	{
		if (hSelf->Type() == BLK_NULL)
			s = "NULL";
		else
			s = unk();
	}
	return s;
}

/*bool Path_t::IsMineLocal(FieldPtr  pField)
{
	if (!mpLocals2)
		return false;
	return mpLocals2->typeStrucLoc()->IsMineField(pField);
}*/

/*int LocalsTracer_t::__checkLocalsTypes(Path_t& rSelf)
{
	if (rSelf.m.hasLocals())
	for (FieldMapCIt i(rSelf.m.locals().begin()); i != rSelf.m.locals().end(); i++)
	{
		FieldPtr pData(VALUE(i));
		if (pData->IsLo cal() || pData->IsLo calEx())
		{
			TraceTypeEx(*pData);
		}
	}

	for (PathTree_t::ChildrenIterator i(rSelf); i; i++)
	{
		__checkLocalsTypes(*i.data());
	}

	return 1;
}*/



/*void FuncInfo_t::SetPathAnchor(HPATH pPath, ADDR va)
{
	pPath->setAnchor(va);
}*/

int FuncInfo_t::Compare(HPATH pSelf, HPATH pPath) const//per-instruction comparison
{
	assert(pSelf->IsTerminal());
	assert(pPath->IsTerminal());

	Path_t::OpIterator iOp1(pSelf->ops());
	Path_t::OpIterator iOp2(pPath->ops());
	do {
		if (!Compare(PRIME(iOp1.data()), PRIME(iOp2.data())))
			return 0;

		iOp1++;
		iOp2++;

		if (iOp1 && PathOf(PRIME(iOp1.data())) != pSelf)
			iOp1.clear();
		if (iOp2 && PathOf(PRIME(iOp2.data())) != pPath)
			iOp2.clear();
	} while (iOp1 && iOp2);

	if (!iOp1 && !iOp2)
		return 1;//both paths fully passed

	return 0;
}

bool FuncInfo_t::IsMineOp(CHPATH hSelf, CHOP pOp) const
{
	if (pOp)
	if (PathOf(pOp) == hSelf)
		return true;
	return false;
}

int FuncInfo_t::CheckOpsCount(CHPATH hSelf, int nTotal) const
{
	int nCount = 0;
	HPATH pPath = hSelf;
	if (!hSelf->IsTerminal())
	{
		assert(hSelf->Type() & BLK_LOGIC || hSelf->Type() == BLK_IF || hSelf->Type() == BLK_IFELSE);
		pPath = TreeTerminalFirst(hSelf);
	}

	if (pPath->ops().empty())
		return 1;

	for (Path_t::OpIterator j(pPath->ops()); j; j++)
	{
		HOP pOp(PRIME(j.data()));
		if (pOp->isRoot())
		if (!pOp->isHidden())
			nCount++;
		if (nCount > nTotal)
			return 1;
	}
	if (nCount < nTotal)
		return -1;

	return 0;
}

bool FuncInfo_t::IsActual(HPATH pSelf) const//is there are any actual ops?
{
	assert(pSelf->IsTerminal());

	for (Path_t::OpIterator i(pSelf->ops()); i; i++)
	{
		if (IsActual(PRIME(i.data())))
			return true;
	}

	return false;
}







int FuncInfo_t::CheckGOTOStatus(CHPATH pSelf) const//0-not needed,1-goto,2-break,3-continue,4-switch,5-return
{
	int status = CheckGOTOStatus0(pSelf);
	if (status == JUMP_GOTO)
	{
		HOP pOpLast = GetLastOp(pSelf);
		if (pOpLast)
		{
			if (IsRetOp(pOpLast))//FIXME!!!
			{
				if (IsLastExx(pOpLast) && !pOpLast->hasArgs())
					return JUMP_NULL;
				//check if any of the ops on the path are visible, then the return itself must be present
				const PathOpList_t &l(pSelf->ops());
				for (PathOpList_t::ReverseIterator j(l, l.Prev(INSPTR(pOpLast))); j; j++)//start from one above the last
				{
					HOP pOp(PRIME(j.data()));
					if (pOp->isRoot() && !pOp->isHidden() && !pOp->isSplit())
						return JUMP_RET;
				}
				//no ops are visible, all jumps to this path fall through, except the one right above
				HPATH pPathPr(TreePrevEx(pSelf));
				if (!pPathPr || pSelf->Parent() != pPathPr->Parent())//the very first? or the prev is under if condition?
					return JUMP_RET;
				if (!(pPathPr->Type() == BLK_JMP || pPathPr->Type() == BLK_JMPSWITCH))//no flow break above
					return JUMP_RET;
				if (pOpLast->hasArgs())//has something to return
					return JUMP_RET;
				return JUMP_NULL;
			}
//!			if (!g_ pDI || !g_ pDI->isUnfoldMode())
			{
				pOpLast = GetGotoDestOp(pOpLast);
				if (pOpLast)
					if (IsRetOp(pOpLast))
						return JUMP_RET;
			}
		}
	}
	return status;
}

int FuncInfo_t::CheckActualXRefs(CHPATH pSelf) const
{
//	assert(IsTerminal());
	HPATH pPath(TreeTerminalFirst(pSelf));
	for (XOpList_t::Iterator i(PathOpRefs(pPath)); i; i++)
	{
		HOP pOp = i.data();
		assert(IsPrimeOp(pOp));
		if (!IsCodeOp(pOp))
			return 1;
		
		if (!pOp->isHidden())
		if (CheckGOTOStatus(PathOf(pOp)) == JUMP_GOTO)
			return 1;

/*		if (pOp->IsGoto())
			return 1;

		HPATH pPathNo;
		HPATH pPathYes;
		if (pOp->Path()->GetOutPaths(&pPathNo, &pPathYes) != 2)
			assert(false);
		if (!Equal(pPathNo, lin ked()))
			return 1;
*/	}

	return 0;
}

/*
int Path_t::CountOps()//counts all ops
{
	assert(m_pOps);

	int nCount = 0;
	HOP pOp = m_pOps;
	while (IsMi neOp(pOp))
	{
		nCount++;
		pOp = pOp->Ne xt();
	}

	return nCount;
}*/

HPATH FuncInfo_t::GetJumpTablePath(CHPATH pSwitchPath) const
{
	assert(PathType(pSwitchPath) == BLK_JMPSWITCH);
	HPATH pPath(TreeNext(pSwitchPath));
	if (pPath && PathType(pPath) == BLK_DATA)
		return pPath;
	return HPATH();
}

HPATH FuncInfo_t::GetIndexTablePath(CHPATH pSwitchPath) const
{
	assert(PathType(pSwitchPath) == BLK_DATA && PathType(TreePrev(pSwitchPath)) != BLK_DATA);
	HPATH pPath(TreeNext(pSwitchPath));
	if (pPath && PathType(pPath) == BLK_DATA)//index table?
		return pPath;
	return HPATH();
}

FieldPtr FuncInfo_t::GetJumpTable(CHPATH pSwitchPath/*FieldPtr *ppIndexTable*/) const
{
	assert(PathType(pSwitchPath) == BLK_JMPSWITCH);

	HOP pOpJump = GetLastOp(pSwitchPath);

//CHECK(No(pOpJump) == 760)
//STOP

	return GetDataSwitch(pOpJump);

	/*
	if (pOpIndex)
	{
		HOP pOpIndexTable = pOpIndex->GetIndexTableOp0(0);
		if (pOpIndexTable)
		{
			*ppIndexTable = pOpIndexTable->m_pData;
			assert(*ppIndexTable);
		}
	}
*/
	//if (pOpJumpTable)
		//if (pOpJumpTable->fieldRef()->IsD ata())
			//return pOpJumpTable->fieldRef();
	return nullptr;
}

/*int TypeProc_t::ValidatePathsTree()
{
	if (!fun cdef()->Body())
		return 1;
	if (!fun cdef()->Body()->HasChildren())
		return 1;

	int nTotal;
	do {
		nTotal = 0;
		HPATH pPath = fun cdef()->Body()->TreeTerminalFirst();
		do {
			HPATH pPathNx = pPath->NextEx();

//			pPath->AdjustGoto();
//			pPath->AdjustGoto2();
			BranchTracer_t an(*fun cdef());

			if (pPath->Type() == BLK_JMP || pPath->Type() == BLK_JMPIF)
				if (an.GetGotoPath(pPath) == pPath->NextEx())
					if (!an.IsRetOp(pPath->m.GetLastOp()))
					{
						an.AdjustGoto(pPath);
						an.InvalidateGoto(pPath);//jump to next instruction
					}

			//if (0)
			if (pPath->Type() != BLK_ENTER)
			if (pPath->Type() != BLK_EXIT)
			if ( pPath->ops().empty()
				|| (!pPath->m.IsAnalized() && pPath->Type() == BLK_JMP && pPath->m.CheckOpsCount(1) == 0) )
			{
				FieldPtr pLabel = pPath->m.Lab elAt();
				if (pLabel)
				{
					assert(pLabel->xr efs().empty());
					delete pLabel;
				}
				delete pPath;
				nTotal++;
			}
			else
			{
				if (an.MergeWith(pPath, false))
					nTotal++;
			}

			pPath = pPathNx;
		} while (pPath);
	} while (nTotal);

	return 1;
}*/


FieldPtr  FuncInfo_t::FindOverlappedLocalVar(CHOP hOpRef, CHPATH hPath, int d) const
{
	assert(hPath);
	{
CHECKID(hOpRef, 3705)
STOP

		HOP hStartOp(PrimeOp(hOpRef));//or the preceeding one?
		if (PathOf(hStartOp) != hPath)
			hStartOp = hPath->tailOp();

		//assert(PathType(hPath) != BLK_ENTER);
		for (PathOpList_t::ReverseIterator j(hPath->ops(), hStartOp); j; ++j)
		{
			HOP pOp(PRIME(j.data()));

			FieldPtr pLocal(pOp->localRef());
			if (pLocal && IsOverlap(pLocal, hOpRef->SSID(), d, hOpRef->Size()))
				return pLocal;

			if (!IsCall(pOp))
			{
				for (OpList_t::Iterator i(pOp->argsIt()); i; ++i)
				{
					HOP hOp2(i.data());
					FieldPtr pLocal(hOp2->localRef());
					if (pLocal && IsOverlap(pLocal, hOpRef->SSID(), d, hOpRef->Size()))
						return pLocal;
				}
			}
			else
			{
				for (XOpList_t::Iterator i(pOp->m_xout); i; ++i)
				{
					HOP hOp2(i.data());
					FieldPtr pLocal(hOp2->localRef());
					if (pLocal && IsOverlap(pLocal, hOpRef->SSID(), d, hOpRef->Size()))
						return pLocal;
				}
			}
		}
	}

	return nullptr;
}

FieldPtr  FuncInfo_t::FindOverlappedLocalArg(CHOP pOpRef, int d) const
{
/*	HPATH hPath(PathHead());
	if (hPath)
	{
		assert(PathType(hPath) == BLK_ENTER);
		for (PathOpList_t::ReverseIterator j(hPath->ops()); j; j++)
		{
			HOP pOp(PRIME(j.data()));
			assert(IsArgOp(pOp));
			FieldPtr pData(pOp->localRef());
			if (pData && pData->IsOverlap(pOpRef->SSID(), d, pOpRef->Size()))
				return pData;
		}
	}*/

	for (FuncArgsIt i(mrFuncDef); i; ++i)
	{
		FieldPtr pLocal(VALUE(i));
		if (pLocal && IsOverlap(pLocal, pOpRef->SSID(), d, pOpRef->Size()))
			return pLocal;
	}

	return nullptr;
}

PathPtr PathTreeEx_t::pathHead() const
{
	PathPtr pBody(body());
	if (pBody)
	{
		HPATH p(pBody->FirstChild());
		if (p && FuncInfo_t::PathType(p) == BLK_ENTER)
			return p;
	}

	return PathPtr();
}

PathPtr PathTreeEx_t::pathTail() const
{
	PathPtr pBody(body());
	if (pBody)
	{
		const Path_t &rBody(*pBody);
		if (rBody.HasChildren())
		{
			HPATH p(rBody.LastChild());
			if (FuncInfo_t::PathType(p) == BLK_EXIT)
				return p;
		}
	}

	return PathPtr();
}

//goes through parents till top logic block reached, that is returned.
//n==1 : if current block is not first in list, func exits with 0.
//n == -1 : if current block is not last in list, func exits with 0 at once.
HPATH FuncInfo_t::GetLogicsTop(CHPATH pSelf, int n) const
{
	HPATH pPath = pSelf;
	while (pPath->Parent() && PathType(pPath->Parent()) & BLK_LOGIC)
	{
		HPATH pParent(pPath->Parent());
		if (n == -1)//check last
		{
			if (pPath != pParent->LastChild())
			//if (!pPath->IsLast())
				return HPATH();
		}
		else if (n == 1)//check first
		{
			if (pPath != pParent->FirstChild())
			//if (!pPath->IsFirst())
				return HPATH();
		}
		pPath = pPath->Parent();
	}
	return pPath;
}

HPATH FuncInfo_t::BlockIn(HPATH pBlock)
{
	assert(pBlock->IsLinked());
	assert(pBlock->HasChildren());//is this realy branch

	HPATH pParent = pBlock->Parent();

	SetPathType(pBlock, BLK_NULL);

	if (IsRedundant(pBlock))
	{
		__KillRedundantChilds(pBlock);

		//		while (pBlock->m_pLocals)
		//		{
		//			if (!pBlock->m_pLocals->MoveUp())
		//				delete pBlock->m_pLocals;
		//		}

		HPATH pChilds = pBlock->FirstChild();
		/*if (pBlock->m.hasLocals())
		{
			assert(0);//?pBlock->m_pLocals->Redirect((Link_t **)&pChilds->m_pLocals);
		}*/

		while (pBlock->HasChildren())
		{
			HPATH pChild(pBlock->TakeFirstChild());
			TreeInsertChildBefore(pParent, pChild, pBlock);
		}

		FuncCleaner_t h(*this);
		h.ClearPath(pBlock);
		pParent->UnlinkChild(pBlock);
		memMgrEx().Delete(pBlock);

		//		if (pParent)
		//			pParent->__KillRedundantChilds();
	}
	else
		__KillRedundantChilds(pBlock);

	return HPATH();//block no more
}

HPATH FuncInfo_t::BlockOut(HPATH pPath1, HPATH pPath2)
{
	HPATH pParent(pPath1->Parent());
	const PathTree_t &l(pPath1->Parent()->Children());
	//assert(pPath1->Parent()->IsMineChild(pPath1) && l.contains(pPath2));//check if they are on the same branch)

	if (pParent->ChildIndex(pPath1) > pParent->ChildIndex(pPath2))
		XChg(pPath1, pPath2);
	/*	else
	{
	pPath1 = pPath1->Ne xt();
	assert(!pPath1->IsFirstEx());
	assert(!pPath2->IsFirstEx());
	pPath2 = pPath2->pPrev;
	}
	*/

	HPATH pBranch = NewPath();
	//pBranch->SetParent(pPath1->Parent());
	//pBranch->LinkBefore(pPath1);
	TreeInsertChildBefore(pParent, pBranch, pPath1);
	while (1)
	{
		TreeAddChild(pBranch, pPath1);
		if (pPath1 == pPath2)
			break;

		pPath1 = TreeNextSibling(pBranch);
		assert(pParent->FirstChild() != pPath1);
	}

	return pBranch;
}

int FuncInfo_t::CheckWhile(CHPATH pSelf) const
{
	if (PathType(pSelf) != BLK_LOOPDOWHILE)
		return 0;

	HPATH pParent = pSelf->Parent();
	if (!pParent)
		return 0;
	if (PathType(pParent) == BLK_NULL)
		pParent = pParent->Parent();
	if (!pParent || !pParent->IsIfWhileOrFor())
		return 0;

	return 1;
}

HPATH FuncInfo_t::CheckForCondition(CHOP pOp) const
{
	if (IsCondJump(pOp))
	{
		HPATH pSelf(PathOf(pOp));
		pSelf = GetLogicsTop(pSelf, 1);
		if (pSelf)
			if (PathType(pSelf->Parent()) == BLK_IFFOR)
				return pSelf->Parent();
	}
	return HPATH();
}

HPATH FuncInfo_t::CheckForOrWhileCondition(CHOP pOp) const
{
	if (IsCondJump(pOp))
	{
		HPATH pSelf(PathOf(pOp));
		pSelf = GetLogicsTop(pSelf, 1);
		if (pSelf)
		{
			HPATH pParent(pSelf->Parent());
			if ((PathType(pParent) == BLK_IFFOR) || (PathType(pParent) == BLK_IFWHILE))
				return pParent;
		}
	}
	return HPATH();
}

int FuncInfo_t::CheckDoWhile0(CHPATH pSelf) const
{
	//!	if (!g_ pDI)
	//!		return 1;

	if (PathType(pSelf) != BLK_LOOPDOWHILE)
		return 0;
	//!	if (!g_ pDI->showBlocks())
	//!		return 0;

	//[	if (!m_pParent)
	//		return 1;
	//!	if (g_ pDI->isUnfoldMode())
	//!	{
	//!		HPATH pPathLast = Childs()->Last();
	//!		if (PathType(pPathLast) & BLK_LOGIC)
	//!			return 0;
	//!	}
	//!	else
	{
		if (pSelf->Parent()->IsIfWhileOrFor())
			return 0;
	}

	return 1;
}

int FuncInfo_t::CheckBreak(CHPATH pSelf) const
{
	if (!(PathType(pSelf) == BLK_JMP || PathType(pSelf) == BLK_JMPIF))
		return 0;

	HPATH pPathLoop = pSelf;
	do {
		if (PathType(pPathLoop) & BLK_LOOP)
		{
			if (PathType(pPathLoop) != BLK_LOOPDOWHILE)
				break;
			if (CheckDoWhile0(pPathLoop))
				break;
			if (CheckWhile(pPathLoop))
				break;
			return 0;
		}
		pPathLoop = pPathLoop->Parent();
	} while (pPathLoop);

	if (pPathLoop)
	{
		//!		if (g_ pDI && g_ pDI->isUnfoldMode())
		//!			if (PathType(pPathLoop) == BLK_SWITCH_0)
		//!				return 0;

		HPATH pPathD = GetGotoPathEx(pSelf);

		//get first valid path after the loop block
		HPATH pPathN = pPathLoop;//never last!!!
		while (1)
		{
			pPathN = TreeNextEx(pPathN);
			if (pPathD == pPathN)
				return 1;
			if (IsActual(pPathN))
				break;//some valid path met
			if (TreeIsLastEx(pPathN))
				break;
		}
	}

	return 0;
}

int FuncInfo_t::CheckElse(CHPATH pSelf) const
{
	assert(!pSelf->IsRoot());
	HPATH pParent(pSelf->Parent());
	if (PathType(pParent) == BLK_IFELSE)
		if (pParent->IsLastChild(pSelf))
			return 1;

	return 0;
}

int FuncInfo_t::CheckIf(CHPATH pSelf) const
{
	if ((PathType(pSelf) == BLK_JMPIF) || (PathType(pSelf) & BLK_LOGIC))
		return 1;

	if (PathType(pSelf) == BLK_IFELSE)
		return 1;

	if (PathType(pSelf) == BLK_IF)
		return 1;

	return 0;
}


/*?bool Path_t::IsDead_()
{
assert(IsTerminal());
for (
HOP pOp = Ops();
IsMineOp(pOp);
pOp = pOp->Ne xt())
{
if (!pOp->IsDeadEx())
return false;
}

return true;
}*/

int FuncInfo_t::CheckElseIf(CHPATH pSelf0) const
{
#if(0)
	int iPath(PathNo(TreeTerminalFirst(pSelf0)));
CHECK(iPath == 984)
STOP
#endif
	//	if (g_ pDI->isUnfoldMode())
	//		return 0;
	HPATH pSelf(pSelf0);

	BlockTyp_t eType(PathType(pSelf));
	/*if (eType == BLK_NULL)//check for degenerate children
	{
		pSelf = pSelf->FirstChild();
		while (IsPathDegenerate(pSelf))
		{
			pSelf = TreeNextSibling(pSelf);//skip
			assert(pSelf);
		}
		eType = PathType(pSelf);
	}*/

	if (eType != BLK_JMPIF)
		if (!(eType & BLK_LOGIC))
			if (eType != BLK_IF)
				if (eType != BLK_IFELSE)
					return 0;

	HPATH pParent(pSelf->Parent());
	if (pParent->Type() != BLK_IFELSE)
		return 0;
	
	if (pParent->IsLastChild(pSelf))
	{
		if (CheckOpsCount(pSelf, 1) != 0)
			return 0;
		if (CheckActualXRefs(pSelf))
			return 0;
		return 1;
	}

	//HPATH pPath = pSelf->Parent();
	HPATH pGrandParent = pParent->Parent();
	if (!pGrandParent || PathType(pGrandParent) != BLK_IFELSE)
		return 0;

	if (pGrandParent->IsLastChild(pParent))
	{
		if (CheckOpsCount(pParent, 1) != 0)
			return 0;
		if (CheckActualXRefs(pParent))
			return 0;
		return 1;
	}

	if (pGrandParent->Parent())
		if (PathType(pGrandParent->Parent()) == BLK_IFELSE)
		{
			HPATH pGrandGrandParent(pGrandParent->Parent());
			if (pGrandGrandParent->IsLastChild(pGrandParent))
				return 1;
		}

	return 0;
}

int FuncInfo_t::CheckContainer(CHPATH pSelf) const//0:not container, 1:normal container, 2-container but last op
{
	if (TreeIsBody(pSelf))
		return 1;
	if (PathType(pSelf) == BLK_LOOPDOWHILE)
		return 2;
	HPATH pParent(pSelf->Parent());
	if (!pParent)
		return 0;
	if (pParent->Type() == BLK_IF)
		if (pParent->IsLastChild(pSelf))
			return 1;
	if (CheckElse(pSelf))
		if (!CheckElseIf(pSelf))
			return 1;
	return 0;
}


int FuncInfo_t::CheckGOTOStatus0(CHPATH pSelf) const//0-degenerate,1-goto,2-break,3-continue,4-switch,5-return
{
	HPATH pPath = pSelf;
	if (!pSelf->IsTerminal())
		pPath = TreeTerminalLast(pPath);
	assert(pPath->IsTerminal());// && pPath->m_pOps);
	if (pPath->Type() == BLK_JMP)
	{
#if(0)
		if (CheckDegenerateJump(pPath))
			return JUMP_NULL;
#endif

		HPATH pPathS = pPath;//SRC
		HPATH pPathD = GetGotoPathEx(pPathS);
		if (!pPathD)
			return JUMP_GOTO;

		HPATH pPath2 = GetClosingPath(pPathS, pPathD);
		if (pPath2 && pPath2 == pPathS)
			//if (!GD C.isUnfoldMode())
			return JUMP_NULL;
		if (CheckBreak(pPath))
			return JUMP_BREAK;
		HPATH pParent(pPath->Parent());
		if (pParent->Type() == BLK_LOOPENDLESS)
		{
			if (pParent->IsLastChild(pPath))
				return JUMP_NULL;
		}
		return JUMP_GOTO;
	}

	if (pPath->Type() == BLK_JMPSWITCH)
	{
		return JUMP_SWITCH;
	}

	if (pPath->Type() != BLK_JMPIF)
		return 1;//dead???
	//assert(false);

	//BLK_JMPIF!

	HPATH pPathSv = pPath;
	pPath = GetLogicsTop(pPath, -1);//check last
	if (!pPath)
	{
		//!		if (!g_ pDI || !g_ pDI->isUnfoldMode())
		return JUMP_NULL;//not visible at all
		pPath = pPathSv;
	}

	HPATH pParent = pPath->Parent();
	assert(pParent);

	if (pParent->Type() == BLK_IF || pParent->IsIfWhileOrFor())
	{
		if (pParent->IsFirstChild(pPath))
			return JUMP_NULL;
		return JUMP_GOTO;
	}
	if (pParent->Type() & BLK_LOOP)
	{
		if (pParent->IsFirstChild(pPath))
		{
			if (pParent->IsIfWhileOrFor())
			{
				if (pParent->IsFirstChild(pPath))
					return JUMP_NULL;
				return JUMP_GOTO;
			}
		}

		if (pParent->IsLastChild(pPath))
		{
			if (CheckDoWhile0(pParent))
				return JUMP_NULL;
			if (CheckWhile(pParent))
				return JUMP_NULL;
			return JUMP_GOTO;
		}
	}

	/*	if (PathType(pParent) & BLK_LOOP)
	if (pPath->IsLast())//<while>
	{
	if (pParent->CheckDoWhile0())
	return JUMP_NULL;
	}*/

	if (CheckBreak(pPath))
		return JUMP_BREAK;

	return JUMP_GOTO;
}


int BlocksTracer_t::UndoPathTree()
{
	assert(m_BI.empty());
	PathTree_t::ChildrenIterator i(mrPathTree.body());
	while (i)
	{
		PathTree_t::ChildrenIterator j(i);
		++i;
		__unbrickCodeTree(j);
	}
	return 1;
}

int BlocksTracer_t::__unbrickCodeTree(PathTree_t::ChildrenIterator &i)
{
	HPATH pSelf(i.data());
	if (pSelf->IsTerminal())
		return 0;

	if (!pSelf->IsRoot())
	{
		BlockTyp_t nType(PathType(pSelf));
		switch (nType)
		{
		case BLK_IF:
		case BLK_IFELSE:
		case BLK_IFWHILE:
		case BLK_IFFOR:
		case BLK_SWITCH:
		case BLK_AND:
		case BLK_OR:
		case BLK_LOOPDOWHILE:
		case BLK_LOOPENDLESS:
			{
			BlockInfo_t bi;
			if (nType == BLK_LOOPDOWHILE || nType == BLK_LOOPENDLESS)
				bi.pPath = TreeTerminalFirst(pSelf->GetChildLast());
			else
				bi.pPath = TreeTerminalFirst(pSelf->GetChildFirst());
			bi.nType = nType;

			if (nType == BLK_AND || nType == BLK_OR)
				m_BI.push_front(bi);
			else
				m_BI.push_back(bi);
			}
			break;
		}
	}

	PathTree_t::ChildrenIterator j(pSelf);
	while (j)
	{
		PathTree_t::ChildrenIterator iPathNx(j);
		++iPathNx;
		__unbrickCodeTree(j);
		j = iPathNx;
	}

	HPATH pParent(pSelf->Parent());
	//if (!pSelf->IsRoot())
	if (pParent)
	{
		while (pSelf->HasChildren())
		{
			HPATH pPath = pSelf->TakeFirstChild();
			TreeInsertChildBefore(pParent, pPath, pSelf);
			//pPath->m_pParent = pSelf->Parent();
			//assert(pPath->Parent());
		}
		pParent->TakeChild(pSelf);
		memMgrEx().Delete(pSelf);
	}
	
	return 1;
}

int BlocksTracer_t::RedoPathTree(PathOpTracer_t &tr)
{
	OpTracer_t &otr(tr.opTracer());
	BranchTracer_t aBranch(*this, tr);
	SwitchTracer_t aSwitch(*this, tr);

	int nCount;
	do {
		nCount = 0;
		std::list<BlockInfo_t>::iterator i(m_BI.begin());
		while (i != m_BI.end())
		{
			const BlockInfo_t &iBI(*i);
			std::list<BlockInfo_t>::iterator i_nx(i);
			i_nx++;
			HPATH  pPath = iBI.pPath;

			int res;
			switch (iBI.nType)
			{
			case BLK_IF://down
			case BLK_LOOPDOWHILE://up
				res = aBranch.TurnBlockIf_On(pPath);
				break;
			case BLK_IFELSE:
				res = aBranch.TurnBlockElse_On(pPath, true);
				break;
			case BLK_IFWHILE:
			case BLK_IFFOR:
				res = aBranch.TurnWhile_On(pPath);
				break;
			case BLK_SWITCH:
				res = aSwitch.TurnSwitch_On(pPath);
				break;
			case BLK_AND:
			case BLK_OR:
				res = aBranch.TurnLogics_On(pPath, 0);
				break;
			case BLK_LOOPENDLESS:
				res = aBranch.TurnEndlessLoop_On(pPath);
				break;
			default:
				assert(false);
			}

			if (res > 0)
			{
				m_BI.erase(i);
				nCount++;
			}

			i = i_nx;
		}
	} while (nCount != 0);

	if (!m_BI.empty())
		fprintf(stdout, "%d block(s) obliterated by last operation\n", (int)m_BI.size());
	m_BI.clear();

	return 1;
}


//split current path into two. new path is inserted
//before this one.
HPATH FuncInfo_t::SplitBefore(HPATH pSelf)
{
	HPATH pPathNew = NewPath();


	//seek for location
	//needs to be inserted before this but not to be first 
	//('cos blocks tree recreation error possibility)
	HPATH  pPath = pSelf;
	while (pPath->Parent()->IsFirstChild(pPath))
	{
		if (TreeIsBody(pPath->Parent()))
			break;
		pPath = pPath->Parent();
	}

	HPATH pParent = pPath->Parent();
	TreeInsertChildBefore(pParent, pPathNew, pPath);
	//pPathNew->SetParent(pPath->Parent());
	SetPathType(pPathNew, BLK_NULL);
	pPathNew->setAnalized(pSelf->isAnalized());
	//if (PathLabel(pSelf))
		//SetPathLabel(pPathNew, PathLabel(pSelf));
	//SetPathLabel(pSelf, nullptr);
	return pPathNew;
}

//split current path into two. new path is inserted
//after this one.
HPATH FuncInfo_t::SplitAfter(HPATH pSelf, uint32_t nType)
{
	HPATH pPathNew = NewPath();
	HPATH pParent(pSelf->Parent());
	pPathNew->setFlags(nType);
	TreeInsertChildAfter(pParent, pPathNew, pSelf);
	//pPathNew->SetParent(pSelf->Parent());
	pPathNew->setAnalized(pSelf->isAnalized());
	return pPathNew;
}

///////////////////
//		[A]
//{L:}	[B]	;<-self,L is unrefed
///////////////////=>
//		[A]
int FuncInfo_t::CheckDanglingPath(CHPATH pSelf)
{
	if (!PathOpRefs(pSelf).empty())
		return 0;

	HPATH pPathPr = TreePrevEx(pSelf);
	assert(pPathPr->IsTerminal());
	if (!pPathPr)
		return 0;
	if (pPathPr->Parent() != pSelf->Parent())
		return 0;
	if (pPathPr->Type() != BLK_NULL)
	{
		if (pPathPr->Type() != BLK_JMP)
			return 0;
		FuncCleaner_t CH(*this);
		CH.DestroyPath(pSelf);
		return 1;
	}

	while (pSelf->hasOps())
	{
		HOP pOp(PRIME(pSelf->takeOpFront()));
		pPathPr->pushOpBack(pOp);//append
		//pPathPr->m.insertOpAfter(pOp, pPathPr->m.tailOp());
	}
	SetPathType(pPathPr, PathType(pSelf));//preserve type
	//SetPathLabel(pSelf, pPathPr->m.Lab elAt());
	//SetPathLabel(pPathPr, nullptr);
	//pPathPr->Parent()->UnlinkChild(pPathPr);
	//mrMemMgr.Delete(pPathPr);
	FuncCleaner_t CH(*this);
	CH.DestroyPath(pSelf);
	return 1;
}

int PathTreeEx_t::isPathDegenerate(CHPATH pSelf) const
{
	if (!pSelf->IsTerminal())
		return 0;

	if (pSelf->ops().empty())
		return 0;//?

	for (Path_t::OpIterator i(pSelf->ops()); i; i++)
	{
		HOP rOp(PRIME(i.data()));
		if (rOp->isRoot())
			if (!rOp->isHidden())
//				if (!rOp->isSplit())
					return 0;
	}

	return 1;
}

/*HOP PathTreeEx_t::__GetOp(HPATH pSelf, int &nId)
{
	for (PathTree_t::ChildrenIterator i(*pSelf); i; i++)
	{
		HOP pOp = __GetOp(i.data(), nId);
		if (pOp)
			return pOp;
	}

	for (OpList_t::Iterator i(pSelf->ops()); i; i++)
	{
		if (nId-- == 0)
			return i.data();
	}

	return 0;
}*/

void FuncInfo_t::__KillRedundantChilds(HPATH pSelf)
{
	HPATH pPath = pSelf->FirstChild();
	while (pPath)
	{
		if (IsRedundant(pPath))
		{
//			pPath->__KillRedundantChilds();
			HPATH pPathToKill = pPath;
			pPath = TreeNextSibling(pPath);
			BlockIn(pPathToKill);
		}
		else
			pPath = TreeNextSibling(pPath);
	}
}



int FuncInfo_t::__checkActualOpsCount(CHPATH pSelf, int nCount) const//counts only actual ops
{
	HPATH pPath = pSelf;
	if (!pSelf->IsTerminal())
	{
		if (PathType(pSelf) == BLK_LOOPDOWHILE)
		{
			if (!CheckWhile(pSelf))
				return nCount-1;
//			if (!m_pParent || !m_pParent->m.IsIfWhileOrFor())
//				return 1;
			//nCount--;//exclude last condjump
			for (PathTree_t::ChildrenIterator i(pSelf); i; i++)
			{
				nCount = __checkActualOpsCount(i.data(), nCount);
				if (nCount < 0)
					break;
			}
			return nCount;
		}
		if (IsNull(pSelf))
		{
			for (PathTree_t::ChildrenIterator i(pSelf); i; i++)
			{
				nCount = __checkActualOpsCount(i.data(), nCount);
				if (nCount < 0)
					break;
			}
			return nCount;
		}
		
		pPath = TreeTerminalFirst(pSelf);
	}

CHECK(pPath->ops().empty())
STOP

	//assert(!pPath->ops().empty());
	//if (!pPath->ops().empty())//?
	for (Path_t::OpIterator i(pPath->ops()); i; i++)
	{
		if (IsActual(PRIME(i.data())))
			nCount--;
		if (nCount < 0)
			break;
	}

	return nCount;
}

//check if the target is the line right below
bool FuncInfo_t::CheckDegenerateJump(CHPATH pSelf) const
{
CHECK(PathNo(pSelf) == 97)
STOP

	assert(PathType(pSelf) == BLK_JMP || PathType(pSelf) == BLK_JMPIF || (PathType(pSelf) & BLK_LOGIC) );
	HPATH pPathTo(GetGotoPath(pSelf));
	if (pPathTo)
	{
		for (HPATH pPath(TreeNextEx(pSelf)); pPath; pPath = TreeNextEx(pPath))
		{
			if (pPath == pPathTo)
				return (pPath->Type() != BLK_EXIT);//all paths between jump and its destination are degenerate
			if (IsActual(pPath))
				break;
		}
		//if target not reached, it was a jump back
	}
	return false;
}

PathPtr FuncInfo_t::GetGotoPathEx(CHPATH pSelf) const
{
	assert(PathType(pSelf) == BLK_JMP || PathType(pSelf) == BLK_JMPIF || (PathType(pSelf) & BLK_LOGIC) );
	HOP pOp = GetLastOp(pSelf);
	if (!pOp)
		return PathPtr();
	if (!PathRef(pOp))
		return PathPtr();//FIXME!!!

	const PathOpList_t &lOp0(PathOps(PathRef(pOp)));
	if (lOp0.empty())
		return PathPtr();

	pOp = PRIME(lOp0.front());
	while (1) 
	{
		if (pOp->isRoot())//IsRootEx())
			if (!pOp->isHidden())
				if (!pOp->isSplit())
					break;
//if (pOp->IsLastEx())
//	break;
		//		assert(!pOp->IsLastEx());
		if (IsLastExx(pOp))//?
			return PathPtr();//? - remove!

		HOP pOpNx(NextPrimeEx(pOp));
		if (!pOpNx)
		{
			assert(0);return PathPtr();
		}
		pOp = pOpNx;
	}

	return PathOf(pOp);
}

PathPtr FuncInfo_t::GetGotoPath(CHPATH pSelf) const
{
	if (PathType(pSelf) != BLK_JMP)
		if (PathType(pSelf) != BLK_JMPIF)
			return PathPtr();
	OpPtr pOpLast(GetLastOp(pSelf));
	return GetJumpTargetPath(pOpLast);
		//the target path not generated yet?
}

/*int PathTreeEx_t::ChangeGotoPath(HPATH pSelf, HPATH pPathNew)
{
	HPATH pPathOld = GetGotoPath(pSelf);
	if (!pPathOld)
		return 0;
	if (pPathOld == pPathNew)
		return -1;//already
	FieldPtr pLabel = pPathOld->m.Lab elAt();
	assert(pLabel);

	HOP pOpLast = pSelf->m.GetLastOp();
	pLabel->ReleaseXRef(pOpLast);
	
	pPathNew->m.Ensu reLabel();
	pPathNew->m.Labe lAt()->AddXRef(pOpLast);
	return 1;
}*/

void FuncInfo_t::dump(PathTreeEx_t &rSelf, std::ostream &os) const
{
	for (PathTree_t::Iterator i(rSelf.m); i; ++i)
	{
		HPATH hPath(*i);
		os << TABS(i.level()+1) << Name(hPath, false) << std::endl;
	}
}

void FuncInfo_t::dump(PathTreeEx_t &rSelf, MyStreamBase &ss) const
{
	MyStreamUtil ssh(ss);
	for (PathTree_t::Iterator i(rSelf.m); i; ++i)
	{
		HPATH hPath(*i);
		std::ostringstream s;
		int id(hPath->IsTerminal() ? PathNo(hPath) : (-1));
		//assert(r.Type() != BLK_DATA);
		s << id << TABS(i.level()+1) << Name(hPath, false);
		ssh.WriteString(s.str());
	}
}

//dumpfunc sub_50AED0


HOP FuncInfo_t::GetGotoDestOp(CHOP pSelf) const
{
	if (!IsGoto(pSelf) && !IsCondJump(pSelf))
		return HOP();

	assert(IsGoto(pSelf) || IsCondJump(pSelf));

	if (!PathRef(pSelf))
		return HOP();

	HOP pOp = PRIME(PathOps(PathRef(pSelf)).front());
	while (pOp)
	{
		if (!IsDeadEx(pOp))//IsRootVisible())
		{
			if (!pOp->isSplit())
				if (!pOp->isHidden())
					break;
		}

		if (IsRetOp(pOp))
		{
			break;
		}

		if (IsGoto(pOp))
			if (IsAddr(pOp))
			{
				pOp = PRIME(PathOps(PathRef(pOp)).front());
				continue;
			}
		
		if (IsLastExx(pOp))
			return HOP();
		assert(!IsLastExx(pOp));//!

		pOp = NextPrimeEx(pOp);
	}

	if (0)
		if (pOp)
			if (IsRetOp(pOp))
				if (pOp->hasArgs())
					pOp = PRIME(PathOps(PathRef(pSelf)).front());

	return pOp;
}

