#include "dump_func.h"
#include <set>
#include "reg.h"
#include "ana_switch.h"
#include "expr_ptr.h"



////////////////////////////////////////////////////////
// CExprName
class CExprName : public expr::IOut
{
	FuncDumper_t& mrDumper;
public:
	struct elt : public MyString
	{
		elt(const char* pc, int _color = adcui::COLOR_NULL) : MyString(pc), color(_color), pObj(0) {}
		elt(const Obj_t* p) : color(0), pObj(p) {}
		int	color;
		const Obj_t* pObj;
	};
	std::vector<elt> mv;
public:
	CExprName(FuncDumper_t& rDumper)
		: mrDumper(rDumper)
	{
	}
	virtual void dump(std::ostream& os, OpType_t t)
	{
		if (t == OPTYP_REAL80)
			if (mrDumper.disp().flags().testL(adcui::DUMP_NOREAL80))
				t = OPTYP_REAL64;
		mrDumper.output_optype(t, 0);
	}
	virtual MyString OpType2Str(OpType_t t)
	{
		int n;
		const char* pc(OpTyp2Str(t, 0, &n));
		int color((n == 1) ? adcui::COLOR_KEYWORD : adcui::COLOR_PREPROCESSOR);
		mv.push_back(elt(pc, color));
		return "%";
	}
	virtual MyString Space()
	{
		return " ";
	}
	virtual void dumpTypeRef(std::ostream& os, const TypeObj_t* pType)
	{
		mrDumper.drawTypeName(pType, false);
	}
	virtual void dumpTypeRef(std::ostream& os, const expr::GType* pType)
	{
		TProtoDumper<ProtoImpl4Expr_t<FuncDumper_t>> proto(mrDumper);
		//proto.dumpFuncdefDeclaration(mpType, nullptr);
		proto.dumpType((expr::GType *)pType, nullptr);
		//mrDumper.drawTypeName(pType, false);
	}
	virtual MyString ObjName2Str(const TypeObj_t* pCplx)
	{
		mv.push_back(elt(pCplx));
		return "%";
	}
	virtual MyString CallDef2Str(const Out_t* pOut)
	{
		return "CALLTYPE";
	}
	virtual MyString Glob2Str(CGlobPtr)
	{
		return "GLOB";
	}
	virtual void DumpCallDef2Str(std::ostream& os, const Out_t* pOut)
	{
		assert(pOut->isCallLike());
		Out_t* mpL(pOut->mpL);
		expr::GType* pType(mpL->typ().type());
		if (!pType || !pType->typeCallPtr())
		{
			if (mpL->is(ACTN_INDIR))
			{
				mpL = mpL->mpR;
				//os << "???";
				//return;
			}
			if (!mpL->is(ACTN_TYPE))
			{
				os << "?";
				return;
			}
			assert(mpL->is(ACTN_TYPE));
			while (pType && pType->typePtr())
				pType = pType->typePtr()->pointee();
		}
		if (pType && pType->typeCallPtr())
		{
			TProtoDumper<ProtoImpl4Expr_t<FuncDumper_t>> proto(mrDumper);
			proto.dumpFuncdefDeclaration(pType, nullptr);
		}
		else
			os << "?";
	}
	virtual void DumpGlob2Str(std::ostream& os, CGlobPtr)
	{
		os << "glob";
	}
};




////////////////////////////////////////////////////////
// ProtoDumper_t

ProtoDumper_t::ProtoDumper_t(const ProtoInfo_t &o, FileDumper_t *root, int indent, Display_t& disp, ProbeEx_t* pCtx, const MyLineEditBase* ped)
	: DumperBase_t(root, indent, disp, pCtx, ped, o)
{
}




////////////////////////////////////////////////////////
// FuncDumper_t

FuncDumper_t::FuncDumper_t(const FuncInfo_t &o, FileDumper_t *root, int indent, Display_t& disp, ProbeEx_t* pCtx, const MyLineEditBase* ped)
	: DumperBase_t(root, indent, disp, pCtx, ped, o)
{
}

void FuncDumper_t::OutputSwitch(CHOP pSelf)
{
	Out_t *pOut(m_disp.cachedExpression(pSelf));
	if (!pOut)
	{
		EXPR_t expr(*this, pSelf, m_disp.flags(), m_disp.exprCache());
		//pOut = expr.DumpSwitch(pSelf);
		pOut = expr.DumpExpression2(pSelf);
		if (pOut)
		{
			EXPRSimpl_t ES(expr);
			ES.Simplify(pOut);
			SwitchQuery_t si;
			if (ES.SimplifySwitch(pOut, si, PathOf(pSelf)))
				ES.SimplifySwitchEx(pOut);//extra polish for output
		}
		m_disp.addCachedExpression(pSelf, expr.FuncDefPtr(), pOut, expr.addresses());
	}
	OutExpr(pOut->mpL);
}


void FuncDumper_t::OutExpr(const Out_t * pSelf)
{
	ExprCache_t &rExprCache(m_disp.exprCache());

	Range_t range;
	/*if (!mpProbeSrc)
	{
		OutExprT(pSelf);
		return;
	}*/

	bool bEditing(mpEd != nullptr);

	if (!m_disp.isUnfoldMode())
	{
		STOP
	}

	//check expression selection
	bool bSel(false);
	if (mpCtx && !m_disp.isUnfoldMode())
	{
		if (!IsProbing())
		{
			if (m_disp.curLine() == mpCtx->line())//only if the current line
			{
				if (rExprCache.mpSelOut == pSelf && !mpCtx->obj())
					bSel = true;
			}
			else if (pSelf->dockOp())
			{
				if (curOpLine())//from intercode view
				{
					if (CheckTopOp(pSelf->dockOp(), curOpLine(), true))//no paths
						bSel = true;
				}
			}
		}
	}
	{
		//else
		{
			//if (!m_disp.isUnfoldMode())
			{
				if (!rExprCache.mpSelOut && !bEditing)
					range.setBegin(currentPosInLine());
			}
		}
	}

	if (bSel && !bEditing)
	{
		PushColor(adcui::COLOR_CUR_EXPR);
		OutExprT(pSelf);
		PopColor();
	}
	else
		OutExprT(pSelf);

	if (probeSrc() && range.begin() >= 0)
	{
		range.setEnd(currentPosInLine());
		if (probeSrc()->rangeExpr().empty() && range.inside(probeSrc()->x()))
		{
			probeSrc()->pickExprRange(range.begin(), unsigned(range.size()));
			rExprCache.setSelOut(pSelf);
			rExprCache.updateSelSet(*this, DockAddr());
			if (pSelf->isImmidiate())
				probeSrc()->pickValue(pSelf->m_value);
			//if (pSelf->isFieldKind())
			//pHit->setObj(pSelf->mpField);
			//fprintf(stdout,"pos1=%d, pos2=%d\n", pos1, pos2);
			//fflush(stdout);
		}
	}
}

int FuncDumper_t::IsRootVisible(CHOP pSelf) const//0-if not
{
	assert(IsPrimeOp(pSelf));
	assert(PathOf(pSelf));

CHECK(OpNo(pSelf) == 360)
STOP

	if (m_disp.isUnfoldMode())
		return 1;

	if (IsFuncStatusAborted() || FuncStatus() == 0)
		return 1;

	if (IsDeadEx(pSelf))
	{
		int f(m_disp.IsOutputDead());
		if (f)
		{
			if (!IsGoto(pSelf) || IsRetOp(pSelf))
				return 1;//visible, if not a goto (or a ret)
			if (f == 3)
				return 1;//visible, goto, but no degenerate labels (and referencing them gotos as well)
		}
		return 0;
	}

	if (!m_disp.isUnfoldMode())
		if (!IsOpRootEx(pSelf))
			return 0;
	/*?	{
	if (pSelf->IsDeadEx())
	return 0;

	if (!pSelf->isRoot())
	return 0;
	}*/

	if (IsRetOp(pSelf))
	{
		if (!m_disp.IsOutputDead())
		{
			//if (pSelf->IsLastExx() && pSelf->arg s().IsEmpty())
				//return 0;
			HPATH pPath(PathOf(pSelf));
			assert(pPath);
			if (CheckGOTOStatus(pPath) == JUMP_NULL)
				return 0;
		}
	}
	else if (IsGoto(pSelf))
	{
		if (!m_disp.IsOutputDead())
		{
			HPATH pPath = PathOf(pSelf);
			if (pPath && CheckGOTOStatus(pPath) == JUMP_NULL)
				return 0;
			//			if ( CheckGOTOStatus() == 0 )
			//				return 0;
		}
	}
	else if (IsCondJump(pSelf))
	{
		//?		if ( this )
		{
			if (!IsConjumpVisible(PathOf(pSelf)))
				return 0;
		}
	}
	else
	{
		//		if (Action() == ACTN_MOV)
		//		{
		//			if (pSelf->isHidden())
		//				return 0;
		//			if (pSelf->arg1()->IsStackPtr())
		//				return 0;
		//		}
		//
		//		if (!CheckOverwrite())
		//			return 0;
		if (pSelf->isHidden())
		{
			if (!ProtoInfo_t::IsFuncStatusFinished(&mrFuncDefRef))
				return 1;
			if (!m_disp.IsOutputDead())
				return 0;
		}
	}

	return 1;
}

bool FuncDumper_t::IsJumpTableVisible(CHPATH pPath) const
{
	//if (!IsGlobalVisible(GetOwnerData(pIOp))
		//return true;

	if (m_disp.IsOutputDead())
	{

		if (m_disp.isUnfoldMode())
		{
			CFieldPtr pJumpTable(PathLabel(pPath));
			//test visibility of jump & index tables
			if (IsConst(pJumpTable))
			{
				if (!IsThruConst(pJumpTable))
				{
					//?if (IsDataDead(GetOwnerData(pIOp)))
						return true;
				}
			}
		}
	}
	return false;
}

int FuncDumper_t::CheckLabelDead(CHPATH pPath) const
{
	for (XOpList_t::Iterator i(PathOpRefs(pPath)); i; i++)
	{
		HOP pOp(i.data());
		if (IsCodeOp(pOp))
			if (pOp->isHidden())//valid!!!
				continue;

		if (IsDataOp(pOp))
		{
			if (m_disp.isUnfoldMode())
				return 0;

			HOP pIOp = pOp;

			if (IsJumpTableVisible(PathOf(pIOp)))
			//if (IsGlobalVisible(GetOwnerData(pIOp)) && !IsDataDead(GetOwnerData(pIOp)))
				return 0;

			HOP pOpSwitch = GetSwitchOp(pIOp);
			HPATH pBlockSwitch = PathOf(pOpSwitch)->Parent();
			if (pBlockSwitch->Type() != BLK_SWITCH)
				return 0;

			if (!IsCaseOf(pIOp, pBlockSwitch))
				if (!IsDefaultOf(pIOp, pBlockSwitch))
					return 0;//visible

			continue;//FIXME
		}

		HPATH pPath = PathOf(pOp);
		if (!pPath)
			return 0;

		if (CheckGOTOStatus5(pPath) == JUMP_GOTO)
			return 0;//actual - not dead!
	}

	return 1;//not visible
}


bool FuncDumper_t::IsLabelDead(CHPATH pPath) const
{
	//CFieldPtr pSelf()
	//assert(pSelf);
	//if (LabelP ath(pSelf))
	if (pPath)
	{
		//assert(!pSelf->IsObjFunc());
		if (!CheckLabelDead(pPath))
			return false;
	}

	return true;
}

bool FuncDumper_t::IsDataDead(CFieldPtr pSelf) const
{
	if (LocalRefs(pSelf).empty())
	{
		if (IsLocalArg(pSelf))
			return false;
		if (ProtoInfo_t::IsRetVal(pSelf))
			return false;
		return true;//dead
	}

	if (!IsStackLocal(pSelf))
	{
		if (!m_disp.isUnfoldMode())
			if (IsThruConst(pSelf))
				return 1;
		return 0;//alive
	}

	for (XOpList_t::Iterator i(LocalRefs(pSelf)); i; i++)
	{
		HOP pOp(i.data());
		if (!IsCode(pOp))
		{
			assert(IsArgOp(pOp) || IsVarOp(pOp));
			return 0;
		}

		HOP pOpRoot = PrimeOp(pOp);
		if (pOpRoot->isHidden())
			continue;
		if (IsOpVisible(pOp))
			return 0;
	}

	return 1;
}


/*int FuncDumper_t::IsLabelVisible(CFieldPtr  pSelf) const
{
	if (!m_disp.isUnfoldMode())
	{
		if (LabelP ath(pSelf)->Type() == BLK_ENTER)
			return 0;
		if (LabelPa th(pSelf)->Type() == BLK_EXIT)
			return 0;
	}

	//?	if (pSelf->Is Func())
	//?		return 0;//function

	//	if (pSelf->m_nFlags & LBL_HIDDEN)
	if (root().IsObj Dead(this, pSelf))
		if (!m_disp.IsOutp utDead())
			return 0;

	return 1;
}*/


bool FuncDumper_t::IsConjumpVisible(CHPATH  pSelf) const
{
	if (m_disp.isUnfoldMode())
		return true;

	if (PathType(pSelf) == BLK_JMPIF)
	{
		if (pSelf->Parent()->Type() == BLK_IF)
		{
			if (!TreeIsLast(pSelf))//check for default branch of switch statement
				if (TreeNext(pSelf)->Type() == BLK_SWITCH)
					return 0;//not visible
		}
		else if (pSelf->Parent()->Type() == BLK_LOOPDOWHILE)
		{
			if (CheckWhile(pSelf->Parent()))
				if (TreeIsLast(pSelf))
					return 0;
		}
	}

	return 1;
}


int FuncDumper_t::CheckBraces(CHPATH pSelf) const
{
	if (m_disp.isUnfoldMode())
		return 0;
	//	if (PathType(pSelf) == BLK_SWITCH_0)
	//		return 0;
	if (!m_disp.showBlocks())
		return 0;
	int n = CheckActualOpsCount(pSelf, 1);
	if (n != 0)//needed (no ops or >1)
		return 1;

#if(0)
	HPATH  pPath = pSelf;
	while (pPath)
	{
		if (pPath->m.hasLocals())
			return 1;//needed
		pPath = pPath->FirstChild();
	}
#endif

	return 0;
}

/*int	FuncDumper_t::GetDataIndent(CFieldPtr  pSelf) const
{
	if (!m_disp.isUnfoldMode())
	{
		if (pSelf->isRet())
			return 0;
	}

	assert(pSelf->IsLo cal() || pSelf->IsLoc alEx());

	int nIndent = 1;
	HPATH pBlock = pSelf->GetOwner Path();
	if (pBlock)
	{
		switch (PathType(pBlock))
		{
		case BLK_LOOPDOWHILE:
			pBlock = pBlock->LastChild();
			break;
		case BLK_LOOPENDLESS:
		case BLK_CASE:
			pBlock = pBlock->FirstChild();
		}
		nIndent = GetPathIndent(pBlock);
	}

	return nIndent;
}*/

int FuncDumper_t::CheckPseudoLabel(CHOP  pSelf) const
{
	if (m_disp.isUnfoldMode())
		return 0;
//?	assert(IsPrimeOp(pSelf));

CHECK(OpNo(pSelf) == 24)
STOP

	if (IsFirstExx(pSelf))
		return 0;

/*?	if (IsOutput Unfold())
	{
		if (Label())
			return 0;
		return 0;
	}*/

	if ( pSelf->isHidden()
		|| !/*m_disp.*/IsOpRootEx(pSelf)
			|| pSelf->isSplit() )
				return 0;

	HOP pOp = pSelf;
	for (;;)
	{
		//if (!pOp->IsFirst())
		if (IsJointOp(pOp))
		{
			if (m_disp.IsOutputDead() || !IsLabelDead(PathOf(pOp)))
			//if (root().IsObjVisible(this, pOp->Label()))
			{
				return 0;
			}
		}

//		if (pOp->IsFirst())
//			break;

		pOp = PrevPrimeEx(pOp);

		if (!pOp)
			break;

		if (!pOp->isHidden())
			if (/*m_disp.*/IsOpRootEx(pOp))
				if (!pOp->isSplit())
					break;
	}

	return 0;
}

HOP FuncDumper_t::GetLabelOp(CHOP pSelf) const
{
	if (m_disp.isUnfoldMode())
		return HOP();

	assert(IsPrimeOp(pSelf));

	HOP pOp = pSelf;
	while (pOp)
	{
		if (IsJointOp(pOp))
			break;

		pOp = PrevPrimeEx(pOp);

		if (IsRootVisible(pOp))
		{
			if (!pOp->isSplit())
			if (!pOp->isHidden())
			{
				pOp = HOP();
				break;
			}
		}
	}

	if (pOp)
		if (!IsJointOp(pOp))
			return HOP();

	return pOp;
}

int FuncDumper_t::CheckGOTOStatus5(CHPATH  pSelf) const
{
	if (m_disp.isUnfoldMode())
	{
		HOP pOpLast = GetLastOp(pSelf);
		if (IsRetOp(pOpLast))
			return JUMP_RET;
		return JUMP_GOTO;
	}

	if (m_disp.showBlocks())
		return CheckGOTOStatus(pSelf);

	int status(JUMP_GOTO);
	{
		HOP pOpLast = GetLastOp(pSelf);
		if (pOpLast)
		{
			if (IsRetOp(pOpLast))//FIXME!!!
				return JUMP_RET;
			//if (!isUnfoldMode())
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

//test visibility of leaf op while outputting
int FuncDumper_t::IsOpVisible(CHOP  pSelf)  const
{
	if (m_disp.isUnfoldMode())
		return 1;
//	if (IsCodeOp())
//	{
//		if (PrimeOp(pSelf)->isHidden())
//			return 0;
//	}

	if (IsArgOp(pSelf))
	{
//?		if (!(pSelf->m_nFlags & OPND_NCONF))
		if (!m_disp.testOpt1(adcui::DUMP_LOGICONLY))
		{
			if (!IsThisPtrOp(pSelf))
				return 1;
			if (m_disp.IsOutputDead())
				return 1;
		}
	}
	else 
	{
		if (IsPrimeOp(pSelf))
		{
			if (IsRootVisible(pSelf))
				return 1;
		}
		else if (IsOnTop(pSelf))
			return 1;
	}

	return 0;
}

void FuncDumper_t::drawLabelDecl0(PathPtr pPath)
{
	CFieldPtr pLabel(PathLabel(pPath));
	if (pLabel)
	{
		drawFieldName(pLabel, nullptr, false, GlobalColor(pLabel));
		pickField(pLabel);
	}
	else
		mos << "$label";
	DumpXRefs(PathOpRefs(pPath));
	mos << ":";
}

void FuncDumper_t::drawLabelDecl(PathPtr pPath)
{
	if (m_disp.dumpSymPath(pPath))
		return;
	drawLabelDecl0(pPath);
}

void FuncDumper_t::drawCodeLine0(CHOP pOp)
{
	if (IsProbing())
	{
		probeSrc()->pickOpLine(pOp);
		ADDR va(OpVA(pOp));
		if (va != -1)
			probeSrc()->pickAddr(va);
	}

	OutputOperand(pOp);
}

void FuncDumper_t::drawCodeLine(CHOP pOp)
{
	assert(pOp);
	assert(IsPrimeOp(pOp));
CHECKID(pOp, 0xdcd)
STOP

	m_disp.newLine();
	OutputPrefix(pOp);
	if (OpState(pOp) == 0 || m_disp.isUnfoldMode() || IsFuncStatusAborted() || FuncStatus() == 0)
		dumpTab(indent());

	if (m_disp.dumpSymOp(pOp))
		return;

	drawCodeLine0(pOp);
}

void FuncDumper_t::DumpPathLabel(TreePathHierIterator iSelf)
{
	HPATH pPath(iSelf.data());
//CHECK(!PathOps(pPath).empty() && No(PathOps(pPath).front()) == 1160)
//STOP
	CFieldPtr pLabel(PathLabel(pPath));
	if (pLabel)
	{
		if (pLabel->isTypeThunk())
			return;
	}
	else if (pPath->inflow().empty())
		return;
	if ((m_disp.IsOutputDead() == 3) || !CheckLabelDead(pPath))
	{
		NewLine(0);
		DumpLabelDecl(pPath);
	}
}

void FuncDumper_t::DumpPathOps(CHPATH hSelf, bool bInRow)
{
	//assert(iSelf.isTerminal());

	//HPATH pSelf(iSelf.data());
	/*	if (m_disp.testOpt1(adcui::DUMP_LOGICONLY))
		{
		assert(0);
		//?OutputPath( ofs, pSelf );
		return;
		}

		if (pSelf->ops().empty())
		return;*/
	if (bInRow)
	{
		ExprCache_t eCache(PtrSize());
		EXPR_t expr(*this, HOP(), m_disp.flags(), eCache);
		Out_t *pOutRoot(expr.AddRoot());
		//pOutRoot->Action = A;
		//pOutRoot->mpOp = pSelf;
		Out_t *pOut(expr.DumpOpsInRow(hSelf, pOutRoot));
		if (pOut)
			OutExpr(pOut);
	}
	else
	{
		for (PathOpList_t::Iterator i(hSelf->ops()); i; i++)
			DumpOp0(PRIME(i.data()));
	}
}

void FuncDumper_t::DumpPathLowProfile(TreePathHierIterator iSelf)
{
	HPATH  pSelf(iSelf.data());

	TreePathHierIterator iBlockOpen;
	TreePathHierIterator iBlockClose;
	if (pSelf->IsRoot())
	{
		iBlockOpen = iBlockClose = iSelf;
	}
	else if (PathType(pSelf) == BLK_ENTER)
	{
		if (!m_disp.showBlocks())
		{
			if (PathIsEmpty(pSelf))
				return;
		}
		else
			iBlockOpen = iBlockClose = iSelf;
	}
	else if (PathType(pSelf) == BLK_EXIT)
	{
		if (!m_disp.showBlocks())
		{
			if (PathIsEmpty(pSelf))
				return;
		}
		else
			iBlockOpen = iBlockClose = iSelf;
	}
	else if (m_disp.showBlocks())
	{
		iBlockOpen = iBlockClose = iSelf;
	}

	if (iBlockOpen)
	{
		NewLine();
		mos << "[";
		if (pSelf && m_disp.showBlocks())
			mos << "{" << Name(pSelf, false) << "}";
		IncreaseIndent();
	}

	for (TreePathHierIterator i(true, iSelf); i; i++)
	{
		DumpPathLowProfile(i);
	}

	if (pSelf->IsTerminal())
	{
CHECK(PathType(pSelf) == BLK_DATA)
STOP

		DumpPathLabel(iSelf);
		//Path_DumpLocals0(iSelf);

		DumpPathOps(iSelf.data(), false);
	}

	if (iBlockClose)
	{
		DecreaseIndent();
		//NewLine();
		DumpCloseBrace();//iBlockClose);
	}
}

void FuncDumper_t::DumpNoFuncdef()
{
	assert(0);
#if(0)
	mos << mrFunc.type obj()->parent()->name xx();
	mos << "()";

	/////////////////////////////////////
	//?DumpOpeningBrace( os, nullptr );

	for (FieldMapIt it = mrFunc.fields().begin(); it != mrFunc.fields().end(); it++)
	{
		CFieldPtr  pField = VALUE(it);

		int nGap = mrFunc.CheckU nderlap(it);
		if (nGap > 0)
		{
			//OutputFieldGap( pField );
		}

		//dumpField_struc(pField);
		//root().Output StrucFld(this, pSelf, it);
		assert(pField->IsLa bel());
		{
			DumpLa belDecl(pField);
			NewLine();
		}
	}

	/*?	if (pSelf->func def() && !pSelf->func def()->Body())//stub
	{
	OutputBodyStub();
	if (pSelf->m_pfDef)
	if (pSelf->m_pfDef->exitOps())
	OutputStubRet( );
	}
	else if (pSelf->IsInvalid())
	{
	dumpComment(" < < < I N V A L I D   B O D Y > > > ");
	}*/

	mos << comment();
	mos << "End Of Func: ";
	mos << mrFunc.typ eobj()->parent()->na mexx();
	mos << "()";
	//mos << "}";
	//DumpClosingInfo();
#endif
}

#if(0)
void FuncDumper_t::strxdeps(std::ostream &os, const XFieldList_t &xdeps, bool bExtra)
{
	std::set<HOP > m;
	for (XFieldList_t::Iterator i(xdeps); i; i++)
	{
		assert(0);
/*?		HOP pOp(Op_t::toOp(i.data()));
		GetTopOp(m_disp.isUnfoldMode(), pOp, m);
*/	}

	mos << "<";
	if (bExtra)
	{
		mos << "x";
		if (!xdeps.empty())
			mos << ",";
	}
	
	for (std::set<HOP >::iterator i(m.begin()); i != m.end(); i++)
	{
		if (i != m.begin())
			mos << ",";

		HOP pOp(*i);
		if (IsArgOp(pOp))
			mos << "e";//entry
		else if (!IsCode(pOp))
		{
			if (IsDataOp(pOp))
				mos << "d";//exit
			else
				mos << unk();
		}
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
			mos << StrNo(pOp);
		}
	}

	mos << ">";
}
#endif

void FuncDumper_t::DumpXRefs(const XOpList_t &xrefs, bool bExtra)
{
	if (!m_disp.testOpt1(adcui::DUMP_XREFS))
		return;
	if (xrefs.empty() && !bExtra)
		return;
	PushColor(adcui::COLOR_XDEPS, true);
	strxdeps(mos, xrefs, bExtra, m_disp.isUnfoldMode());
	PopColor(true);
}

void FuncDumper_t::DumpXIns(const XOpList_t &xin, bool bExtra)
{
	if (!m_disp.testOpt1(adcui::DUMP_INDATA))
		return;
	if (xin.empty() && !bExtra)
		return;
	PushColor(adcui::COLOR_XINS, true);
	strxdeps(mos, xin, bExtra, m_disp.isUnfoldMode());
	PopColor(true);
}

void FuncDumper_t::DumpXOuts(const XOpList_t &xout, bool bExtra)
{
	if (!m_disp.testOpt1(adcui::DUMP_OUTDATA))
		return;
	if (xout.empty() && !bExtra)
		return;
	PushColor(adcui::COLOR_XOUTS, true);
	strxdeps(mos, xout, bExtra, m_disp.isUnfoldMode());
	PopColor(true);
}

void FuncDumper_t::OutputIndirect(CHOP pSelf)
{
	static char cbuf[NAMELENMAX + 1];

	assert(pSelf->IsIndirectB());

	if (pSelf->SSID() == SSID_IMM)
	{
		if (pSelf->OpSeg())
		{
			Reg_t rs;
			rs.assign(pSelf->OpSeg() * OPSZ_WORD, OPSZ_WORD);
			char buf[80];
			mos << mrDC.toRegName(SSID_SEGREG, rs.offset(), rs.m_sz, 0, buf);
			mos << ":";
		}
		mos << "[" << Int2Str(OpDisp(pSelf), I2S_HEXC | I2S_SIGNED) << "]";
		return;
	}

	int siz = pSelf->OpSize();
	int ofs = pSelf->Offset0();

	Reg_t rb;
	int ptrsz = mrDC.PtrSize();
	if (pSelf->IsIndirectEx())
		ptrsz = mrDC.PtrSizeEx();
	rb.assign(pSelf->OpOffsU(), ptrsz);//Id(OpC() & 0xF));
//?	rb.m_xdeps = pSelf->m_xin;//m_pXIn;
	int LDisp = pSelf->m_disp;

	if (pSelf->OpSeg())
	{
		Reg_t rs;
		rs.assign((pSelf->OpSeg() - 1) * OPSZ_WORD, OPSZ_WORD);
		char buf[80];
		mos << mrDC.toRegName(SSID_SEGREG, rs.offset(), rs.m_sz, 0, buf);
		mos << ":";
	}

	mos << "[";

	if (rb.size())
	{
		SSID_t ssid = (SSID_t)(pSelf->OpC() & 0xf);
		char buf[80];
		mos << mrDC.toRegName(ssid, rb.offset(), rb.m_sz, 0, buf);
		DumpXIns(pSelf->m_xin);//rb.m_xdeps);
	}

	if (LDisp)
	{
		if (LDisp > 0)
			mos << "+";
		else if (LDisp < 0)
			mos << "-";
		LDisp = abs(LDisp);
		mos << Int2Str(LDisp, I2S_HEXC | I2S_SIGNED);
	}

	mos << "]";
}

void FuncDumper_t::OutputAddrRef(ADDR va, OpType_t t) const
{
	const char *pfx = "GLOB";
	switch (t)
	{
	case OPSZ_BYTE:
	case OPTYP_INT8:
	case OPTYP_UINT8:
		pfx = "BYTE";
		break;
	case OPSZ_WORD:
	case OPTYP_INT16:
	case OPTYP_UINT16:
		pfx = "WORD";
		break;
	case OPSZ_DWORD:
	case OPTYP_INT32:
	case OPTYP_UINT32:
		pfx = "DWORD";
		break;
	case OPSZ_QWORD:
	case OPTYP_INT64:
	case OPTYP_UINT64:
		pfx = "QWORD";
		break;
	case OPTYP_REAL32:
		pfx = "FLT";
		break;
	case OPTYP_REAL64:
		pfx = "DBL";
		break;
	default:
		if (OPTYP_IS_PTR(t))
			pfx = "DWORD";
	}

	mos << pfx << "_" << Int2Str(ImageBase() + va, I2S_HEX);

}

void FuncDumper_t::OutputSimple0(const Out_t * pSelf)
{
	if (m_disp.isUnfoldMode() && pSelf->isNumKind() && pSelf->isLeft() && pSelf->mpU->is(ACTN_CALL))
	{
		mos << "SUB_" << Int2Str(pSelf->m_value.ui64, I2S_HEX);
		return;
	}
	if (pSelf->isNumKind())
	{
		TYP_t T(pSelf->typ());
		if (pSelf->isNumZero() && T.stripModifier().isPtr())
			dumpPreprocessor("NULL");
		else if (T.stripTypedef().isSimple())
		{
			VALUE_t v(pSelf->Value());
			if (v.typ == 0)
				mos << "?";
			else
			{
				TYP_t T_up(pSelf->mpU->typ());
				if (pSelf->mpU->is(ACTN_TYPE) && T_up.stripModifier().isPtr())
					dumpColorTerm(v.toString(), adcui::COLOR_SQUIGGLE_RED, true);
				else
					mos << v.toString();
			}
		}
		else
			mos << "?";
		return;
	}

	int siz = pSelf->size();
	int ofs = pSelf->opoff();


	OPC_t opc = pSelf->opc();
	SSID_t ssid = pSelf->SSId();

	const STORAGE_t &ST(Storage(ssid));

	if (!ST.isFlag() && IS_ADDR(opc))//opc & OPC_ ADDRESS)
		if (!pSelf->mpU->isCallLike())//? || !pSelf->isLeft())//no offsets expected in a call
			mos << "&";

	//const Dc_t &rDC(dcInfo().dc());
	if (opc & OPC_INDIRECT)
	{
		siz = mrDC.PtrSize();

		if ((ssid != SSID_GLOBAL) || !pSelf->fieldRef())
			mos << "[";
	}


	int tsize = 0;
	int tdisp = 0;
	if (ST.isFlag())
	{
		if (!m_disp.isUnfoldMode() && ProtoInfo_s::IsFuncStatusFinished(FuncDefPtr()))
			mos << "$";
		if (opc & OPC_FLAGS)
		{
			if (pSelf->m_eflags)
				mos << FlagsToStr(opc, pSelf->m_eflags, false);
			else
				mos << RegToString(ssid, REG_t(ofs, siz));
		}
		else
		{
			if (pSelf->m_eflags)
				mos << FlagsToStr(opc, pSelf->m_eflags, true);
			else
				mos << RegToString(ssid, REG_t(ofs, siz));
		}
	}
	else if (ST.isRegister())
	{
		if (!m_disp.isUnfoldMode() && ProtoInfo_s::IsFuncStatusFinished(FuncDefPtr()))
			mos << "$";
		mos << RegToString(ssid, REG_t(ofs, siz));
	}
	else if (ssid == SSID_LOCAL)
	{
		mos << LocalToString(ofs + pSelf->mDisp, siz);

/*		if (!IS_ADDR(opc))//if (!(opc & OPC_ADDRESS))
			if (siz != OPSZ_DWORD)
				tsize = siz;*/
	}
	else if (ssid == SSID_GLOBAL)
	{
		CFieldPtr pFieldRef(pSelf->fieldRef());
		if (pFieldRef && (IsProc(pFieldRef->type())
			|| pFieldRef->isTypeCode()
			/*|| IsPtrToCFunc(pFieldRef)*/))//->isPtrToFunc()))
		{
			if (m_disp.isUnfoldMode())
				dumpFieldNameFull(GlobObj(pFieldRef));
			else
				dumpFieldRef(pFieldRef);
		}
		else
		{
			if (pSelf->isIntrinsicRef())
			{
				CGlobPtr iType(pSelf->typ().asGlobRef());
				mos << TypeName((GlobToTypePtr)iType);
			}
			else
			{
				//CTypePtr pSeg(OwnerSeg());
				//ADDR64 imageBase(pSeg->typeSeg()->imageBase(pSeg));
				OutputAddrRef(ofs + pSelf->mDisp, OpType_t(pSelf->typ().optyp()));
			}
		}

		int siz0 = siz;
		int ofs0 = ofs;
		if (pSelf->mpOp)
		{
			CFieldPtr pFieldRef(LocalRef(pSelf->mpOp));
			if (pFieldRef)
			{
				if (!IS_ADDR(opc))//if (!(opc & OPC_ADDRESS))
					siz0 = pFieldRef->size();
				ofs0 = pFieldRef->_key();
			}
		}

		if (siz != siz0)
			tsize = siz;

		if (ofs0 != ofs)
			tdisp = ofs - ofs0;

	}
	else
	{
		assert(0);
	}

	if (opc & OPC_INDIRECT)
	{
		if (ssid != SSID_IMM)
		{
			if (!pSelf->m_xin.empty() || pSelf->m_bExtraIn)
				DumpXIns(pSelf->m_xin, pSelf->m_bExtraIn);

			if (pSelf->mDisp > 0)
				mos << "+";
			else if (pSelf->mDisp < 0)
				mos << "-";
		}

		if (pSelf->mDisp != 0)
			mos << Int2Str(abs(pSelf->mDisp), I2S_HEXC | I2S_SIGNED);

		if ((ssid != SSID_GLOBAL) || !pSelf->fieldRef())
			mos << "]";
	}

	if (tdisp)
		mos << "." << tdisp;

	if (tsize)
		mos << ":" << tsize;
}

void FuncDumper_t::OutputOp(const Out_t * pSelf)
{
	CFieldPtr pExpField(nullptr);
	CFieldPtr pField(nullptr);

	int hit = 0;
	switch (pSelf->mKind)
	{
	case OUT_FIELD:
		pExpField = pSelf->expField();
		pField = pSelf->field();
		if (pExpField)
		{
			if (pExpField == pField)
				pExpField = nullptr;
		}
		break;

	case OUT_IMM:
		pField = pSelf->field();
		break;

	case OUT_ARG:
		mos << "$arg";
		return;

	case OUT_ARG_THIS:
		mos << "$this";
		return;

	default:
		if (!pSelf->isOpKind())
		{
			mos << "?";
			return;
		}
	}

	if (!pSelf->m_xout.empty() || pSelf->m_bExtraOut)
		DumpXOuts(pSelf->m_xout, pSelf->m_bExtraOut);

	//	if (bSel)
	//		PushColor(COLOR_SEL);
	//	if (bSel2)
	//		PushColor(COLOR_SELAUX);
	//	if (bBold)
	//		mos << FONT(FONT_BOLD);//output in bold

	if (pField)
	{
CHECKID(pField, 18193)
STOP
		if (pExpField)
		{
			drawImpFieldRef(pField, nullptr);
			return;
		}
		if (ThisPtrArg() == pField)
		{
			dumpReserved("this");
			return;
		}

		if (!pSelf->mpU->is(ACTN_OFFS) && IsGlobal(pField))
		{
			OFF_t oPos;
			if (CheckThruConst(pField, oPos))
			{
				dumpConstRef(pField);
				return;
			}
		}
		dumpFieldRef(pField);
	}
	else if (!m_disp.isUnfoldMode() && !pSelf->isNumKind())
	{
		PushColor(adcui::COLOR_UNKNOWN);
		OutputSimple0(pSelf);
		PopColor();
	}
	else
		OutputSimple0(pSelf);

	//	if (bSel2)
	//		PopColor();
	//	if (bSel)
	//		PopColor();
	//	if (bBold)
	//		mos << FONT(0);

	if (m_disp.isUnfoldMode() && pSelf->fieldRef() && IsLocal(pSelf->fieldRef()))
		if (pSelf->mpU->is(ACTN_ARG) || pSelf->mpU->is(ACTN_VAR))
			DumpXRefs(LocalRefs(pSelf->fieldRef()));

	if (!(pSelf->mSsid & OPC_INDIRECT))
		if (!pSelf->m_xin.empty() || pSelf->m_bExtraIn)
			DumpXIns(pSelf->m_xin, pSelf->m_bExtraIn);
}

/*void FuncDumper_t::dumpField_local2(CFieldPtr  pData)
{
	int nIndent = GetDataIndent(pData);

	bool bDead = false;
	if (IsDataDead(pData))//m_nFlags & DAT_HIDDEN)
		if (!m_disp.isUnfoldMode())
			bDead = true;

	if (bDead)
		nIndent = 0;

	if (nIndent)
		dumpTab(nIndent);

	if (bDead)
	{
		PushColor(adcui::COLOR_COMMENT);
		mos << "//? ";
	}
	DumpLocal(pData);
}*/

/*void FuncDumper_t::DumpLocal(CFieldPtr pData)
{
	dumpFie ld_data(this, pData);
	if (!IsLocalArg(pData) && !pData->isRet() || m_disp.isUnfoldMode())
		mos << ";";

	if (xrefs(pData).empty())
		if (!mrFuncDef.isStub())
			if (!pData->isRet())
			{
				PushColor(adcui::COLOR_COMMENT);
				mos << "//UNREFERENCED";
			}
}*/


void FuncDumper_t::OutputBodyStub()
{
//?	dumpTab(3);
	PushColor(adcui::COLOR_COMMENT);
	mos << "// . . . (no body)";
}

void FuncDumper_t::OutputStubRet()
{
//?	dumpTab();
	dumpReserved("return");
	mos << " ";

	assert(!mrFuncDef.exitOps().empty());
	for (PathOpList_t::Iterator i(mrFuncDef.exitOps()); i; i++)
	{
		if (!i.is_first())
			mos << ", ";
		OutputSimple(PRIME(i.data()));
	}

	CloseStatement();
}

void FuncDumper_t::OutputPrefix(CHOP  pSelf)
{
//CHECK(OpNo(pSelf) == 23)
//STOP

	MyString s;
	if (m_disp.testOpt1(adcui::DUMP_LNUMS))
	{
		s += LineInfoStr(pSelf);
	}

	if (m_disp.testOpt1(adcui::DUMP_PATHS))
	{
		if (!s.isEmpty())
			s += ":";
		s += PathInfoStr(pSelf);
#if(0)
		if (0)//test tracer
		{
			int n = pSelf->Path()->m.m_bTraced;
			s += MyStringf("(%d)", n);
		}
#endif
	}

	if (!IsAnalized(pSelf))
	{
		return;
	}

	if (m_disp.testOpt1(adcui::DUMP_STACKTOP))
	{
		if (!s.isEmpty())
			s += ":";
		s += StackTopInfoStr(pSelf, false);
	}

	if (m_disp.testOpt1(adcui::DUMP_FPUTOP))
	{
		if (!s.isEmpty())
			s += ":";
		s += FTopInfoStr(pSelf, false);
	}

	if (0)
		if (IsAnalized(pSelf))
			s += ":1";
		else
			s+= ":0";

	mos << s;
}

void FuncDumper_t::OutputExpression3(CHOP pOp)
{
	Out_t *pOut(m_disp.cachedExpression(pOp));
	if (!pOut)
	{
CHECKID(pOp, 0x38b2)
STOP
		EXPR_t expr(*this, pOp, m_disp.flags(), m_disp.exprCache());
		pOut = expr.DumpExpression2(pOp);// , false);
		EXPRSimpl_t ES(expr);
		ES.Simplify(pOut);
		//expr.Simplify(pOut);
		pOut->mpOp = pOp;
		m_disp.addCachedExpression(pOp, expr.FuncDefPtr(), pOut, expr.addresses());
		m_disp.addProblem(pOp, CheckProblem(pOut));
	}
	OutExpr(pOut);
}

void FuncDumper_t::OutputPathCondition3(CHPATH pSelf)
{
	//HPATH  pSelf(iSelf.data());
	int id = -1;
	HOP pOp = GetLastOp(pSelf);

#ifdef _DEBUG
	if (pOp)
		id = OpNo(pOp);
#endif

CHECK(id == 63)
STOP

	//	Out_t::ResetArrays(); 

	Out_t *pOut(m_disp.cachedExpression(pOp));
	if (!pOut)
	{
		EXPR_t expr(*this, HOP(), m_disp.flags(), m_disp.exprCache());
		pOut = expr.DumpCondition4(pSelf);
		EXPRSimpl_t ES(expr);
		ES.Simplify(pOut);
		pOut->mpOp = pOp;
		m_disp.addCachedExpression(pOp, expr.FuncDefPtr(), pOut, expr.addresses());
		m_disp.addProblem(pOp, CheckProblem(pOut));
	}

	OutExpr(pOut->mpL);
}

ProblemInfo_t FuncInfo_t::CheckProblem(CHOP pOpFrom, CHOP pOpTo) const
{
	if (CheckStackBreak(pOpTo, pOpFrom) != 0)
		return (1 << PROBLEM_PSTACK_NCONV);
	if (CheckFStackBreak(pOpTo, pOpFrom) != 0)
		return (1 << PROBLEM_FSTACK_NCONV);
	return 0;
}

ProblemInfo_t FuncInfo_t::CheckProblem(CHOP pOpFrom) const
{
	ProblemInfo_t f(0);
	HPATH hPathFrom(PathOf(pOpFrom));
	if (!hPathFrom->isAnalized())
		return f;

	if (IsVarOp(pOpFrom))
		return f;

	//if (IsCall(pOpFrom))
	if (!IsGoto(pOpFrom) && !IsCondJump(pOpFrom))
	{
		HOP hOpNx(NextPrimeEx(pOpFrom, true));
		if (!hOpNx)
			return (1 << PROBLEM_BAD_CODE);
		if (f = CheckProblem(pOpFrom, hOpNx))
			return f;

		return f;
	}

	ProtoInfo_t TI(*this);

	HPATH hPathTo(PathRef(pOpFrom));
	if (hPathTo)
	{
		//const PathOpList_t &l(PathOps(hPathTo));
		//HOP pOpTo(PRIME(l.Head()));
		//HOP pOpTo(hPathTo->headOp());
		//if (pOpTo)
		if (hPathTo->hasOps())
		{
			HOP pOpTo(hPathTo->headOp());
			if (IsVarOp(pOpTo))
				pOpTo = NextPrimeEx(pOpTo, true);
			if (f = CheckProblem(pOpFrom, pOpTo))
				return f;
		}
		if (IsCondJump(pOpFrom))
		{
			HPATH hPathNext(TreeNextEx(hPathFrom));
			if (hPathNext && hPathNext->hasOps())
			{
				HOP pOpTo(hPathNext->headOp());
				if (IsVarOp(pOpTo))
					pOpTo = NextPrimeEx(pOpTo, true);
				if (f = CheckProblem(pOpFrom, pOpTo))
					return f;
			}
		}
		if (hPathTo->Type() == BLK_EXIT)
		{
			if (pOpFrom->pstackIn() != 0)
				return (1 << PROBLEM_PSTACK_NRESET);

			/*int nStackOut = StackOut(pOpFrom) - RetAddrSize();
			if (nStackOut != mrFuncDef.getPStackPurge())
				return (1 << PROBLEM_PSTACK_NCONV);*/

			int nFPUStackCleanup = FpuOut(pOpFrom);
			if (mrFuncDef.getFStackPurge() != nFPUStackCleanup)
				return (1 << PROBLEM_FSTACK_NCONV);

			int rlsSize(pOpFrom->pstackDiff() - RetAddrSize());
			int argsSize(TI.StackIn());//slow!
			if (rlsSize != 0 && rlsSize != argsSize)
				return (1 << PROBLEM_PSTACK_RET_MISMATCH);
			else if (rlsSize != mrFuncDef.getPStackPurge() - RetAddrSize())
				return (1 << PROBLEM_PSTACK_RET_MISMATCH);
		}
	}
	return f;
}

ProblemInfo_t FuncDumper_t::CheckProblem(Out_t * pSelf) const
{
	ProblemInfo_t f(0);
	if (!pSelf)
		return f;

	HOP hOp(pSelf->mpOp);
	//if (hOp && ActionOf(hOp) == ACTN_INVALID)
		//return (1 << PROBLEM_BAD_CODE);

	if (hOp)
	{
CHECK(OpNo(hOp) == 1064)
STOP

		if (m_disp.isUnfoldMode() || IsFuncStatusAborted())
		{
			//if (pSelf->Action == ACTN_GOTO || IsCall(hOp))
			{
				if ((f = FuncInfo_t::CheckProblem(hOp)) != 0)
					return f;
			}
			if (hOp->ins().opnd_BAD_STACKOUT)
				return (1 << PROBLEM_PSTACK_BAD);
			return f;
		}

		if (pSelf->is(ACTN_GOTO))
		{
			if (pSelf->mpL->isImmidiate())
				if (IsCondJump(hOp))
					return (1 << PROBLEM_CONST_CONDITION);
		}

		if (IsDeadEx(hOp))
			return f;//?
		if (ActionOf(hOp) == ACTN_GOTO && hOp->IsIndirect())
			if (!IsRetOp(hOp) || IsBadRetOp(hOp))
				return (1 << PROBLEM_INDIRECT_GOTO);
	}

	for (Out_t::Iterator it(pSelf); it; ++it)
	{
		Out_t *pCur(it.top());

		if (pCur->isOpKind())
		{
			if (/*FuncStatus() == 0 || */ProtoInfo_t::IsFuncStatusFinished(FuncDefPtr()))
				return (1 << PROBLEM_INTERCODE_EXPOSED);
		}
	}

	return f;
}

void FuncDumper_t::OutputRet(CHOP pSelf)
{
	assert(IsRetOp(pSelf));

	dumpReserved("return");
	if (pSelf->hasArgs())
	{
		mos << " ";
		OutputExpression3(pSelf);
		//		for (XRef_t *pXIn = pSelf->ar gs(); pXIn; pXIn = pXIn->Ne xt())
		//		{
		//			if (pXIn != Get Args())
		//				mos << ", ";
		//			HOP pOp = pXIn->pOp;
		//			HOP pOpExit = pfd->FindExitOp(pOp);
		//			assert(pOpExit);
		//			OutputExpression3(pOp);
		//		}
	}
	else
		m_disp.addProblem(pSelf, FuncInfo_t::CheckProblem(pSelf));
	CloseStatement();
	if (0)
		if (OpDisp(pSelf) != 0)
		{
			PushColor(adcui::COLOR_COMMENT);
			mos << "//";
			OutputSimple(pSelf);
		}
}


void FuncDumper_t::OutputIGoto(CHOP pIOp)
{
	assert(IsDataOp(pIOp));

	//?	assert( pIOp->isPtr() );
	HOP pSwitchOp = GetSwitchOp(pIOp);
//?	int nIndents = GetPathIndent(PathOf(pSwitchOp));
//?	dumpTab(nIndents + 1);
	//	mos << "goto " << pOp->m_pGoto->Name() << ";";
	OutputGoto((HOP)pIOp);
}

void FuncDumper_t::OutputPathGoto(CHPATH pSelf)
{
	//HPATH  pSelf(iSelf.data());
	int nStatus = CheckGOTOStatus5(pSelf);
	if (nStatus == JUMP_NULL)
		return;

	HOP pOpLast = GetLastOp(pSelf);
	if (!pOpLast)
		mos << "/";
	else
	{
		if (IsCondJump(pOpLast))
			mos << " ";
		else if (IsGoto(pOpLast))
			mos << "/";
	}

	if (pOpLast && !m_disp.testOpt1(adcui::DUMP_LOGICONLY))
	{
		OutputGoto(pOpLast);
		return;
	}

	if (nStatus == JUMP_BREAK)
		dumpReserved("break");
	else if (nStatus == JUMP_CONTINUE)
		dumpReserved("continue");
	else if (nStatus == JUMP_SWITCH)
		dumpReserved("switch");
	else//JUMP_GOTO
	{
		dumpReserved("goto");
		mos << " ";
		//		if (testOpt1(DUMP_LOGICONLY))
		HPATH pPathD = GetGotoPathEx(pSelf);
		if (pPathD)
			mos << OpNo(PRIME(pPathD->ops().Head()));
		else
			mos << unk();
	}
}


void FuncDumper_t::OutputCondjump(CHOP pSelf)
{
	assert(IsCondJump(pSelf));

	if (!PathOf(pSelf))
	{
		assert(0);/*
		mos << "if (";
		OutputExpression3(pSelf);
		MyString s(FieldName(PathRef(pSelf)));
		mos << ") goto " << s << ";";*/
	}

	OutputPathCondjump(PathOf(pSelf));
}

void FuncDumper_t::CloseStatement()
{
	if (!m_disp.isUnfoldMode())
		mos << ";";
}

int FuncDumper_t::GetPathIndent(CHPATH pSelf) const
{
	if (!m_disp.showBlocks())
		return 1;

	if (m_disp.isUnfoldMode())
	{
		int nLevel = -1;

		HPATH  pPath = pSelf;
		while (pPath)
		{
			nLevel++;
			pPath = pPath->Parent();
		}

		return nLevel;
	}

//	assert(IsTerminal());
	int nLevel = 0;

	HPATH pPath = pSelf;
	while (1)//pPath->Parent())
	{
		if (pPath->IsRoot())
		{
			nLevel++;
			break;
		}

		HPATH pParent = pPath->Parent();
		if (PathType(pParent) == BLK_IF)
		{
			if (!TreeIsFirst(pPath))
			{
				if (PathType(pPath) != BLK_SWITCH)
					nLevel++;
//				if (pPath->m_pParent->Parent())
//				if (PathType(pPath->m_pParent->Parent()) == BLK_IFELSE)
//					nLevel--;
			}
		}
		else if (PathType(pParent) == BLK_IFELSE)
		{
			if (!TreeIsFirst(pPath))
				if (!CheckElseIf(pPath))
					nLevel++;
		}
		else if (PathType(pParent) == BLK_CASE)
		{
			nLevel++;
		}
		else if (PathType(pParent) == BLK_DEFAULT)
		{
			nLevel++;
		}
		else if (PathType(pParent) == BLK_LOOPENDLESS)
		{
			nLevel++;
		}
		else if (PathType(pParent) == BLK_LOOPDOWHILE)
		{
			if (CheckDoWhile(pParent))
				nLevel++;
		}
		else if (pParent->IsIfWhileOrFor())
		{
			if (!TreeIsFirst(pPath))
				nLevel++;
		}

		pPath = pParent;
	}

	return nLevel;
}

int FuncDumper_t::CheckDoWhile(CHPATH pSelf) const
{
	if (m_disp.isUnfoldMode())
		return false;

	if (FuncInfo_t::PathType(pSelf) != BLK_LOOPDOWHILE)
		return 0;

	if (!m_disp.showBlocks())
		return 0;

	HPATH  pParent = pSelf->Parent();
//	if (!pParent)
//		return 1;

	if (pParent->IsIfWhileOrFor()) 
		return 0;

	if (FuncInfo_t::PathType(pParent) == BLK_NULL)
	{
		if (!TreeIsLast(pParent))
			return 1;

		pParent = pParent->Parent();
		if ( !pParent )
			return 1;

		if (pParent->IsIfWhileOrFor())
			return 0;
	}

	return 1;
}

int FuncDumper_t::GetIndent(CHOP pSelf) const
{
	if (m_disp.isUnfoldMode())
	{
		HPATH  pPath = PathOf(pSelf);
		return GetPathIndent(pPath);
	}

	assert(PathOf(pSelf));

	if (!m_disp.showBlocks())
		return 1;

	HPATH  pPath = PathOf(pSelf);
	int nIndent = GetPathIndent(pPath);

	if (PathType(pPath) == BLK_JMPIF)
		if (GetLastOp(pPath) == pSelf)
		{
			HPATH pPathTop = GetLogicsTop(pPath, 1);//check first
			if (pPathTop)
				if (CheckDoWhile(pPathTop->Parent()))//PathType(pself) == BLK_LOOPDOWHILE)
					if (TreeIsLast(pPathTop))
						nIndent--;
		}

	return nIndent;
}


enum {
	OP_COMMENT	= 0x01,
	OP_DEAD		= 0x02,
	OP_HIDDEN	= 0x04,
	OP_UNANALIZED	= 0x08
};

unsigned FuncDumper_t::OpState(CHOP pOp) const
{
	unsigned u(0);
	if (!IsAnalized(pOp))
		u |= OP_UNANALIZED;
	if (IsFuncStatusAborted())
		u |= OP_DEAD;
	if (m_disp.isUnfoldMode())
	{
		if (IsDeadEx(pOp))
			u |= OP_DEAD;
		if (/*!pOp->isRoot() || */pOp->isHidden())
			u |= OP_HIDDEN;
	}
	else
	{
		if (IsDeadEx(pOp))
			u |= OP_COMMENT;
		if (pOp->isCloned())
			u |= OP_HIDDEN;
	}
	return u;
}

void FuncDumper_t::OutputOperand(CHOP pOp0)
{
CHECK(OpNo(pOp0) == 359)
STOP
		//int nIndent = -1;
	/*	if ( showBlocks() )
	if ( pOp0->Path() )
	if ( !isUnfoldMode() )
	if ( pOp0->isSplit() )
	{
	HOP pOpPr = 0;
	Dump_t *pDump = m_pCurDump;
	while (1)
	{
	assert(!pDump->IsFirst());
	pDump = (Dump_t *)pDump->pPrev;
	if (!pDump->isOp())
	break;
	pOpPr = (HOP)pDump->m_pCtx;
	if (!pOpPr->isSplit())
	{
	nIndent = pOpPr->GetIndent();
	break;
	}
	}
	}*/

	if (ActionOf(pOp0) == ACTN_INVALID)
	{
		dumpColorTerm("(!) Error: Unsupported instruction...", adcui::COLOR_TAG_ERROR);
		return;
	}

//	if (nIndent == -1)
	//	nIndent = GetIndent(pOp0);

	unsigned uState(OpState(pOp0));
	//bool bComment = false;
	bool bItalics((uState & OP_HIDDEN) != 0);


	/*if (!m_disp.isUnfoldMode())
	{
		if (IsDeadEx(pOp0))
			bComment = true;
		if (pOp0->isCloned())
			bItalics = true;
	}
	else
	{
		if (IsDeadEx(pOp0))
			bDead = true;
		if (!pOp0->isRoot() || pOp0->isHidden())
			bItalics = true;
	}*/

	if (uState & (OP_UNANALIZED | OP_DEAD))//!bAnalized || bDead)
	{
		//	bComment = true;
//?		dumpTab(nIndent);
		PushColor(adcui::COLOR_UNANALIZED);
	}
	else
	{
//?		if (!bComment)
//?			dumpTab(nIndent);

		if (uState & OP_COMMENT)//bComment)
		{
			PushColor(adcui::COLOR_COMMENT);
			mos << "//";
			if (uState & OP_UNANALIZED)//!bAnalized)
				mos << "<NA>:";
			else
				mos << "? ";
		}
	}

	if (bItalics)
		PushColor(adcui::COLOR_FONT_ITALIC);

	if (m_disp.isUnfoldMode())
	{
		if (m_disp.syncOpLine())
		{
			if (pOp0 == m_disp.syncOpLine())
			{
//				if (mpProbeSrc && !mpProbeSrc->isEditing())
					PushColor(adcui::COLOR_SEL);
				//?PushFont(adcui::FONT_BOLD);
			}
			else if (CheckTopOp(pOp0, m_disp.syncOpLine()))
				PushColor(adcui::COLOR_SELAUX);
		}
		else if (curOpLine())
		{
			if (pOp0 == curOpLine())
				PushColor(adcui::COLOR_SEL);
			else if (CheckTopOp(pOp0, curOpLine()))
				PushColor(adcui::COLOR_SELAUX);
		}
	}

	if (IsCondJump(pOp0))
	{
		OutputCondjump(pOp0);
	}
	else if (IsGoto(pOp0))
	{
		OutputGoto(pOp0);
	}
	else
	{
		OutputExpression3(pOp0);
		CloseStatement();
	}
}

void FuncDumper_t::dumpCallType()
{
	//if (!pSelf)
		//return;
	//if (!mrFuncDef.thisPtr())//__stdcall is default for non-static class members 
	if (ProtoInfo_t::IsFuncUserCall(FuncDefPtr()))
	{
		dumpReserved("__usercall");
		mos << " ";
	}
	else if (!IsThisCallType())
	{
		//if (mrFuncDef.isStdCall())
		if (ProtoInfo_s::IsFuncCleanArged(FuncDefPtr()))//not enough!
		{
			dumpReserved("__stdcall");
			mos << " ";
		}
	}
}

/*void FuncDumper_t::OutputArg2(CHOP  pSelf)
{
	mos << "?";
	/ *?assert(0);
	assert(pSelf->IsArgOp() || pSelf->objField());
	//	assert(!pSelf->IsIndirect() || (pSelf->IsLo cal() && pSelf->m_pData->m_nOffset > 0));//arg

	if (!isUnfoldMode())
	if (!testOpt1(DUMP_NOTYPES))
	{
	OpType_t t = (OpType_t)pSelf->OpType();
	Type_t * pType = TYPEMGR.type(t);
	if ( !pType && t != OPTYP_NULL )
	{
	t = (OpType_t)pSelf->OpSize();
	pType = TYPEMGR.type(t);
	}
	dumpT ype WithField(pType, nullptr);
	mos << " ";
	//?		TYPE _t t;
	//?		pSelf->GetType(t);
	//?		OutputTYPE0(&t, 1);
	}

	//	DumpXOuts(XOut());

	std::streampos pos1;
	std::streampos pos2;
	bool bSel = false;

	if (!m_hi.IsLocked())
	pos1 = mos.tellp();

	if (m_hi.pObj)
	if (m_hi.pObj == pSelf->fieldRef() || m_hi.pObj == pSelf)
	bSel = true;
	if (bSel)
	{
	PushColor(COLOR_SEL);
	mos << FONT(FONT_BOLD);
	}

	EXPR_t expr(m_dwFlags);
	expr.ResetArrays();
	Out_t * pOutRoot = expr.AddRoot();
	pOutRoot->m_p Op0 = pSelf;
	expr.DumpExpression(pSelf, pOutRoot);
	expr.Simplify(pOutRoot);
	OutExpr(pOutRoot);

	//OutputSimple(os);

	if (bSel)
	PopColor(); << FONT(0);

	if (!m_hi.IsLocked())
	{
	pos2 = mos.tellp();
	if (m_hi.Check(pos1, pos2))
	{
	if (isUnfoldMode())
	m_hi.pObj = pSelf;
	else
	{
	if ( pSelf->IsDataOp() )
	{
	HOP pIOp = pSelf;
	m_hi.pObj = pIOp->GetOwnerData();
	}
	else
	m_hi.pObj = nullptr;
	if (!m_hi.pObj)
	m_hi.pObj = pSelf;
	}
	}
	}* /
}*/

void FuncDumper_t::DumpDefinition()
{
	CGlobPtr pSelf(FuncDefPtr());
	if (m_disp.isUnfoldMode())
	{
		dumpFieldNameFull(pSelf);
		//mos << "(";
		//DumpPathOps(PathHead(), true);
		//mos << ")";
		mos << ":";
		return;
	}
	//DumpFunctionDefinition0(FUNCDUMP_ALL, nullptr, PrimeSeg());
	//void FuncDumper_t::DumpFunctionDefinition0(FuncDumpFlags_t dumpFlags, CFieldPtr pImpField, CTypePtr pScope)

	FuncDumpFlags_t dumpFlags(FUNCDUMP_ALL);

	CFieldPtr pField(FuncInfo_s::DockField(pSelf));

	if (IsProbing())
	{
		probeSrc()->pickFuncDecl(pSelf);
		pickField(pField);
		//if (pImpField)
			//mpProbeSrc->pickImpField(pImpField);
	}

	TypePtr pOwnerClass(OwnerScope());

	if (IsStaticMemberFunction(pSelf))
	{
//?		assert(!(pSelf->flags() & FDEF_VIRTUAL));
		if (!pOwnerClass->typeNamespace())
		{
			dumpReserved("static");
			mos << " ";
		}
		else if (pSelf->flags() & FDEF_STATIC)
		{
			dumpPreprocessor("STATIC");//this should be a warning
			mos << " ";
		}
	}

	TProtoDumper<ProtoImpl4F_t<FuncDumper_t>> proto(*this);
	proto.mpField = pField;
	proto.mpImpField = nullptr;// pImpField;
	proto.mbTabSep = true;
	proto.mbIsDef = true;
	proto.mbUglyName = (dumpFlags & FUNCDUMP_UGLY_NAME) != 0;
	proto.mbDockAddr = true;
	proto.dumpFunctionDeclaration(pField, (GlobToTypePtr)pSelf, PrimeSeg());// pScope);

	if (dumpFlags & FUNCDUMP_MODIFIER)
	{
		ProtoInfo_t TI(*this);
		if (TI.IsConstClassMember())//(pSelf->flags() & FDEF_CONST)
		{
			mos << " ";
			if (mrFuncDef.isStub())
				dumpReserved("const");
			else
				dumpPreprocessor("CONST");//not yet real 'const'
		}
	}
}

void FuncDumper_t::DumpLabelRef(CHPATH pPath)
{
	CFieldPtr pLabel(PathLabel(pPath));
	if (pLabel)
		dumpFieldRef(pLabel);
	else
		mos << "label$";
}

void FuncDumper_t::DumpLabelDecl(CHPATH pPath)
{
	if (IsFuncStatusAborted())
	{
		PushColor(adcui::COLOR_UNANALIZED);
	}
	else if (IsLabelDead(pPath))
	{
		if (m_disp.isUnfoldMode())
		{
			PushColor(adcui::COLOR_DEAD);
		}
		else
		{
			//SA		assert(IsOutputDeadLabels());//otherwise - they'll be not dumped
			PushColor(adcui::COLOR_COMMENT);
			mos << "//";
		}
	}

	drawLabelDecl(pPath);
}

void FuncDumper_t::OutputTermName(const Out_t *pSelf)
//const char *Out_t::Name()
{
	//std::ostringstream str;

	/*if (IsRoot())
	{
	if (m_p Op0)
	{
	TypeProc_t *pFunc = m_p Op0->GetOwner Func();
	os << "{* " << (pFunc?pFunc->nam exx():"?") << ":" << m_p Op0->No() << " *}";
	}
	}
	else*/
	{
		if (pSelf->mAction)
		{
			OutAction(pSelf, true);
		}
		else if (pSelf->mKind != OUT_NULL)
		{
			OutputOp(pSelf);
		}
		else
			mos << unk();
	}

	//mos << '\0';

	//return str;
}

void FuncDumper_t::OutAction(const Out_t * pSelf, bool bExtra)
{
	assert(pSelf->mAction);
	//	if (IsCall())
	//		return;

	switch (pSelf->mAction)
	{
	case ACTN_TYPE:
	{
		mos << "(";
		if (!pSelf->typ().isCallPtr() || !disp().flags().testL(adcui::DUMP_PCALLS))
			OutputOutType(pSelf->typ());
		else
			mos << "PCALL";
		//CExprName o(*this);
		//pSelf->typ().dump(mos, o);
		mos << ")";
		break;
	}
	case ACTN_SHIFT:
	{
		//		os << "<";
		//		os << ((m_nOffset > 0)?("+"):("")) << m_nOffset;
		//		os << ":" << Size();
		//		os << ">";
		int offs = pSelf->opoff();
		assert(offs);
		if (offs > 0)
			mos << "+";
		else//if offs < 0
			mos << "-";
		mos << abs(offs) << ":";
		break;
	}
	default:
		if (pSelf->is(ACTN_VAR) || pSelf->is(ACTN_ARG))
		{
			PushColor(adcui::COLOR_KEYWORD_EX);
			mos << ACTN2STR2(pSelf->mAction);
			PopColor();
		}
		else
			mos << ACTN2STR2(pSelf->mAction);
	}

	if (bExtra)
	{
		if (pSelf->m_bNoReduce)
			mos << " (no reduce)";
		else if (pSelf->m_bPostCall)
			mos << " (postcall)";
	}
}

const Out_t * FuncDumper_t::GetTop2(const Out_t * pSelf, bool &bIsRight) const
{
	const Out_t *pOutTop = pSelf;
	while (1)
	{
		bIsRight = pOutTop->isRight();
		pOutTop = pOutTop->mpU;
		if (pOutTop)
			if (pOutTop->is(ACTN_TYPE))
				if (m_disp.testOpt1(adcui::DUMP_NOTYPES))
					continue;

		break;
	}

	return pOutTop;
}

bool FuncDumper_t::CheckParenthesis(const Out_t * pSelf) const
{
	bool bIsRight;
	const Out_t * pOutTop = GetTop2(pSelf, bIsRight);
	if (!pOutTop || pOutTop->isRoot())
		return false;

	assert(pSelf->mAction);
	Action_t ThisAction = pSelf->mAction;
	if (pSelf->isCallLike())
		ThisAction = ACTN_CALL;
	else if (pSelf->is(ACTN_RET))
		ThisAction = ACTN_RET;

	if (ThisAction == ACTN_TYPE)
		if (m_disp.testOpt1(adcui::DUMP_NOTYPES))
			return false;//ignored

	Action_t TopAction = pOutTop->mAction;
	if (pOutTop->isCallLike())
	{
		if (bIsRight)
			return false;//inside of braces
		TopAction = ACTN_CALL;
	}
	else if (TopAction == ACTN_ARRAY)
	{
		if (bIsRight)
			return false;
	}
	else if (TopAction == ACTN_INDIR)
	{
		if (m_disp.testOpt1(adcui::DUMP_NOSTRUCS))
			return false;//brackets will be instead
	}

	int32_t prior;
	if (!ComparePrior(ThisAction, TopAction, prior))
		return false;//fail to determine precedence

	if (prior < 0) //Action < TopAction
		return true;

	//special case
	if (TopAction == ACTN_SUB)
	{
		if (pSelf->is(ACTN_ADD) || pSelf->is(ACTN_SUB))
			if (bIsRight)
				return true;
	}
	else if (TopAction == ACTN_DIV)
	{
		if (ThisAction == ACTN_MUL)
			if (bIsRight)
				return true;
	}

	return false;
}

bool FuncDumper_t::CheckTypeOutput(const Out_t * pSelf) const
{
	if (!m_disp.testOpt1(adcui::DUMP_CASTS))
		return false;

	if (pSelf->isOpKind())
	{
		if (pSelf->opc() == OPC_CPUSW || pSelf->opc() == OPC_FPUSW)
			return false;
	}

	if (m_disp.isUnfoldMode())
	{
		if (pSelf->mAction)
			return false;
		return true;
	}

	if (pSelf->is(ACTN_TYPE))
		return false;

	switch (pSelf->mAction)
	{
	case ACTN_MOV:
	case ACTN_COMMA:
	case ACTN_COMMA3:
	case ACTN_CHECK:
	case ACTN_ASSCHK:
		return false;
	}
	if (pSelf->isComparAction())
		return false;//bool
	if (ISLOGIC(pSelf->mAction) || pSelf->is(ACTN_LOGNOT))
		return false;//bool
	if (!pSelf->mpU)
		return false;
	//	if (pSelf->mpU->Action == ACTN_TYPE)
	//		return false;
	if (pSelf->mpU->is(ACTN_OFFS))
		return false;
	if (pSelf->mpU->is(ACTN_CALL) && pSelf->isLeft())
		return false;//pfunc
	if (pSelf->mpU->is(ACTN_ARRAY) && pSelf->isLeft())
		return false;//ptr
	if (pSelf->isRight())
	{
		if (pSelf->mpU->is(ACTN_PTR) || pSelf->mpU->is(ACTN_DOT))
			return false;//field
	}
	if (pSelf->is(ACTN_CALL) && !pSelf->mpU->is(ACTN_MOV))
		return false;
	return true;
}

bool FuncDumper_t::IsIndirectCall(const Out_t * pSelf) const
{
	if (!pSelf->is(ACTN_CALL) || !pSelf->mpOp || IsAddr(pSelf->mpOp))
		return false;
	if (pSelf->mpL->isFieldKind() && pSelf->mpL->mpExpField)
		return false;
	return true;
}

bool g_AllBraces = false;
void FuncDumper_t::OutExprT(const Out_t * pSelf)
{
	bool bType = false;
	if (CheckTypeOutput(pSelf))
		bType = true;

	if (!pSelf->isTerm())
	{
		bool bAction(true);//should we dump the operation token?
		bool bLeft(pSelf->mpL != nullptr);//should we dump the left child?
		int lbr = 0;
		int rbr = 0;

		if (g_AllBraces || CheckParenthesis(pSelf) || bType)
			lbr = 1;
		if (pSelf->isCallLike())
			rbr = 1;
		else if (pSelf->is(ACTN_ARRAY))
			rbr = 2;
		/*else if (pSelf->Action == ACTN_RET && pSelf->mpR)
		{
			if (m_disp.isUnfoldMode())
				rbr = 1;//ret with args
			else if (pSelf->isBadRet())
				rbr = 1;
		}*/

		if (bType)
		{
			PushColor(adcui::COLOR_TYPES);
			mos << "{";
			OutputOutType(pSelf->typ());
			mos << "}";
			PopColor();
		}

		switch (pSelf->mAction)
		{
		case ACTN_POSTCALL:
			rbr = 1;
			break;
		case ACTN_CALL:
		case ACTN_ARRAY:
			bAction = false;
			break;
		case ACTN_GOTO:
			dumpReserved("goto");
			assert(pSelf->mpL);
			dumpSpace();
			bAction = false;
			if (pSelf->mpR)
				rbr = 1;
			break;
		case ACTN_RET:
			dumpReserved("return");
			if (pSelf->mpL || pSelf->mpR)
				dumpSpace();
			bAction = false;
			break;
		case ACTN_INDIR:
			if (m_disp.testOpt1(adcui::DUMP_NOSTRUCS))
			{
				lbr = 0;
				rbr = 2;
				bAction = false;
			}
			break;
		case ACTN_VAR:
			if (!m_disp.isUnfoldMode())
			{
				if (pSelf->mpR->isFieldKind())//pure declaration
				{
					CFieldPtr pField(pSelf->mpR->field());
					TProtoDumper<ProtoImpl_t<FuncDumper_t>> proto(*this);
					proto.mbTabSep = true;
					proto.setReplaceExtDoubles(true);
					proto.dumpFieldDeclaration(pField, pField->type(), nullptr);
					return;
				}
				if ( !(pSelf->mpR->is(ACTN_MOV) || pSelf->mpR->isMovExAction()) || !pSelf->mpR->mpL->isFieldKind())
				{
					OutputOutType(pSelf->typ());//some ugly forms
					mos << " ";
				}
				bAction = false;
			}
			break;
		case ACTN_MOV:
			if (pSelf->mpL->isArgKind() && !disp().flags().testL(adcui::DUMP_SUBEXPR))
				bAction = bLeft = false;
			break;
		}

		if (lbr)
			mos << "(";

		if (bLeft)
		{
			if (!pSelf->is(ACTN_POSTCALL))
			{
				if (!pSelf->is(ACTN_RET) || m_disp.isUnfoldMode() || pSelf->isBadRet())
				{
//					int icall(IsIndirectCall(pSelf));
//					if (icall)//indirect call
//						mos << "(* ";
					OutExpr(pSelf->mpL);
//					if (icall)
//						mos << " )";
				}
			}
		}

		if (bAction)
			OutAction(pSelf);

		if (rbr)
			mos << ((rbr == 1) ? ("(") : ("["));//call|array open
		if (pSelf->is(ACTN_POSTCALL))
		{
			OutExpr(pSelf->mpL);
			mos << ", ";
		}
		if (pSelf->mpR)
			OutExpr(pSelf->mpR);
		if (rbr)
			mos << ((rbr == 1) ? (")") : ("]"));//call|array close

		if (lbr == 1)
			mos << ")";//br[1];//close brace
	}
	else if (pSelf->isRoot())
	{
		assert(!pSelf->mpU && !pSelf->mpL);// && mpR);
		if (pSelf->mpR)
			OutExpr(pSelf->mpR);
	}
	else if (pSelf->mKind == OUT_PATH)
	{
		assert(m_disp.testOpt1(adcui::DUMP_LOGICONLY));

		if (pSelf->mpPath)
			mos << OpNo(GetFirstOp(pSelf->mpPath));
		else
			mos << unk();
	}
	else
	{
		if (bType)
		{
			assert(0);//?
			PushColor(adcui::COLOR_TYPES);
			mos << "{";
			OutputOutType(pSelf->typ());
			mos << "}";
			PopColor();
		}

		if (!m_disp.isUnfoldMode())
		{
			//declaration with initialization
			if (pSelf->isFieldKind())
				if (pSelf->isLeft())
					if (pSelf->mpU->is(ACTN_MOV) || pSelf->mpU->isMovExAction())
						if (pSelf->mpU->mpU->is(ACTN_VAR))
						{
							CFieldPtr pField(pSelf->field());
							TProtoDumper<ProtoImpl4V_t<FuncDumper_t>> proto(*this);
							proto.mbTabSep = true;
							proto.setReplaceExtDoubles(true);
							proto.dumpFieldDeclaration(pField, pField->type(), nullptr);
							return;
						}
		}

		OutputOp(pSelf);
	}
}

/*CFieldPtr FuncDumper_t::Field_DumpUnion(CFieldPtr  pSelf)
{
	int ofsend = pSelf->Offset() + pSelf->size();
	CFieldPtr pFieldNx = (CFieldPtr )pSelf->Ne xt();
	bool bUnion = false;
	if (pFieldNx && (ofsend > pFieldNx->Offset()))
	{
		CFieldPtr pFieldPr = (CFieldPtr )pSelf->P rev();
		if (!pFieldPr || pFieldPr->Offset() != pSelf->Offset())
			bUnion = true;
	}

	if (bUnion)
	{
		OutputUnionBeg(pSelf);
		OutputS trucFld(pField);
		do {
			pFieldNx = Field_DumpStruc(pFieldNx);
		} while (pFieldNx && (ofsend > pFieldNx->Offset()));
		OutputUnionEnd(pSelf);
	}
	else
	{
		OutputStr ucFld(pField);
	}

	return pFieldNx;
}*/

/*CFieldPtr FuncDumper_t::Field_DumpStruc(CFieldPtr  pSelf)
{
	CFieldPtr pFieldPr0 = (CFieldPtr )pSelf->P rev();
	if (pFieldPr0)
	if (pFieldPr0->Offset() < pSelf->Offset())
	{
	CFieldPtr pFieldPr = pFieldPr0;
	while (pFieldPr)
	{
	assert(pFieldPr->Offset() < pSelf->Offset());
	//			if (pFieldPr != pFieldPr0)
	//			if (pFieldPr->Offset() < pFieldPr0->Offset())
	//				break;//only ==

	int ofsend = pFieldPr->Offset() + pFieldPr->size();
	if (ofsend > pSelf->Offset())
	{
	OutputStrucBeg2( os, pSelf );
	OutputFieldGap( os, pField );
	CFieldPtr pFieldNx = Field_DumpUnion(pSelf);
	while (pFieldNx && (pFieldNx->Offset() < ofsend))
	pFieldNx = Field_DumpStruc(pFieldNx);

	OutputStrucEnd2( os, pSelf );
	return pFieldNx;
	}

	pFieldPr = (CFieldPtr )pFieldPr->Pr ev();
	}
	}

	int nGap = pSelf->CheckUn derlap();
	if (nGap > 0)
	{
	OutputFieldGap( os, pField );
	}

	CFieldPtr pFieldNx = Field_DumpUnion(pSelf);
	return pFieldNx;
	return 0;
}*/



void FuncDumper_t::DumpPathCases(TreePathHierIterator iSelf)
{
	HPATH  pSelf(iSelf.data());
	assert(PathType(pSelf) == BLK_CASE);

	TreePathHierIterator iPathSwitch(iSelf.parent());
	if (PathType(iPathSwitch.data()) == BLK_NULL)
		iPathSwitch = iPathSwitch.parent();
	if (PathType(iPathSwitch.data()) != BLK_SWITCH)
		return;
	assert(PathType(iPathSwitch.data()) == BLK_SWITCH);

	Path_DumpCasesOf(TreeTerminalFirst(iSelf.data()), iPathSwitch.data());
}

void FuncDumper_t::OutputSimple(CHOP  pSelf)
{

	if (!m_disp.isUnfoldMode())
	{
		/*?	if (pSelf->IsArgOp())
		{
		if (pSelf->isPtr() && pSelf->IsTh isPtr())// && !pSelf->IsRootEx())
		{
		dumpReserved("this");
		return;
		}
		}*/
	}

	/*	if (IsAddr(pSelf))
	{
	mos << "&" << pSelf->m_pData->Name();
	return;
	}*/

	/*?	if (0)
	if (pSelf->IsDataOp() && !pSelf->IsArgOp())
	{
	if (pSelf->IsScalar())
	{
	VALUE_t v;
	pSelf->GetVALUE(v);
	v.Out(mos);
	return;
	}
	else
	{
	CFieldPtr pData0 = pSelf->GetOwnerData();
	if (!pData0)
	{
	mos << "???";
	return;
	}
	else
	{
	if ((!isUnfoldMode()) || pData0->IsLo cal())
	{
	char buf[NAMELENMAX];
	pData0->nam ex(buf);
	mos << buf;
	return;
	}
	}
	}
	}
	*/

	/*?if (0)
		if (pSelf->IsIndirect() || IsAddr(pSelf))
		{
			if (!pSelf->IsLo cal())//OpC() != OPC_LOCAL)
			{
				OutputIndirect(pSelf);
				return;
			}
		}*/

	if (pSelf->IsScalar())
	{
		if (pSelf->OpOffsU())
		{
			switch (pSelf->OpOffsU())
			{
				//			case OPID_0:	mos << 0; break;
				//			case OPID_1:	mos << 1; break;
				//			case OPID_CF0:	mos << "~CF"; break;
				//			case OPID_CF1:	mos << "CF"; break;
			//case OPID_0R64: mos << "0.0"; break;
			//case OPID_1R64: mos << "1.0"; break;
			case OPID_L2T:	mos << "log2(10)"; break;
			case OPID_L2E:	mos << "log2(e)"; break;
			case OPID_PI:	mos << "pi"; break;
			case OPID_LG2:	mos << "log10(2)"; break;
			case OPID_LN2:	mos << "loge(2)"; break;
			default:
				mos << "???";
			}
		}
		else
		{
			VALUE_t v;
			pSelf->GetVALUE(v);
			mos << v.toString();
		}
		return;
	}

	OutputOpName(pSelf);
}

void FuncDumper_t::DumpOpeningBrace(TreePathHierIterator iSelf)
{
	HPATH pPath(iSelf.data());
/*	if (pPath)
	{
//?		int nIndent = 0;
//?		if (!pPath->IsRoot())
//?			nIndent = GetPathIndent(pPath);

		bool bDoWhile = CheckDoWhile(pPath) != 0;
		//			if (bDoWhile)
		//				nIndent--;

//?		dumpTab(nIndent);

		if (bDoWhile)
		{
			dumpReserved("do");
			mos << " ";
		}
		else if (PathType(pPath) == BLK_LOOPENDLESS)
		{
			dumpReserved("while");
			mos << " (1) ";
		}
	}*/

	DumpOpenBrace();

	if (0)
		if (pPath)
			mos << "//<" << Name(pPath, false) << ">";
}

void FuncDumper_t::OutputOpNameUnfold(CHOP  pSelf)
{
	char buf[80];
	const char *str = "";

	int flags = 0;
	switch (pSelf->OpCf())
	{
	case OPC_CPUSW:
		flags = CPUFlags(pSelf); goto $L0;
	case OPC_FPUSW:
		flags = FPUFlags(pSelf); goto $L0;
	case OPC_FPUCW:
	$L0 :
		str = mrDC.flagsToStr(StorageId(pSelf), flags);
		break;

	case OPC_CPUREG:
	case OPC_FPUREG:
	case OPC_AUXREG:
		str = mrDC.toRegName(StorageId(pSelf), pSelf->Offset0(), pSelf->OpSize(), 0, buf);
		break;

	default:
		if (pSelf->IsIndirect() || IsAddr(pSelf))
		{
			bool bSize = true;

			if (IsAddr(pSelf))
				mos << "&";

			int disp = 0;
			CFieldPtr pField(LocalRef(pSelf));
			if (pField)
			{
				dumpFieldRef(LocalRef(pSelf));

				//if (IsData(pField))
					if (pField->size() != pSelf->OpSize())
						bSize = true;
				disp = pSelf->m_disp;
				//				strcat(buf, str);
			}
			else
			{
				if (IsLocalOp(pSelf))//OpC() == OPC_LOCAL)
					str = mrDC.toRegName(SSID_LOCAL, pSelf->Offset0(), pSelf->OpSize(), 0, buf);
				else
				{
					OutputIndirect(pSelf);
					goto $L1;
				}
				//			else if (pSelf->OpC() == OPC_GL OBAL)
				//				str = mrDC.regName(SSID_G LOBAL, pSelf->Offset0(), pSelf->OpSize());
				mos << str;
			}


			if (IsAddr(pSelf))
				bSize = false;

		$L1:
			if (disp)
			{
				mos << ".";
				mos << Int2Str(disp);
			}

			if (bSize)
			{
				mos << ":";
				mos << Int2Str(pSelf->OpSize());
			}

			return;
		}
	}//switch

	if (isempty(str))
		str = "???";

	mos << str;
}

void FuncDumper_t::OutputOpName(CHOP pSelf)
{
	if (m_disp.isUnfoldMode())
	{
		OutputOpNameUnfold(pSelf);
		return;
	}

	char buf[80];
	const char *str = "";

//	assert(Op_t::toOp((OpPtr)pSelf));//?
	/*?if (pSelf->objField())
	{
		dumpFieldRef(pSelf->objField());
		return;
	}*/

	switch (pSelf->OpCf())
	{
	case OPC_CPUSW:
		str = mrDC.flagsToStr(StorageId(pSelf), CPUFlags(pSelf));
		break;
	case OPC_FPUSW:
		str = mrDC.flagsToStr(StorageId(pSelf), FPUFlags(pSelf));
		break;
	case OPC_FPUCW:
		str = mrDC.flagsToStr(StorageId(pSelf), 0);
		break;

	case OPC_CPUREG:
		/*?		if (this)
		if (!isUnfoldMode())
		if (pSelf->IsArgOp())
		if (pSelf->isPtr())
		if (pSelf->IsTh isPtr())
		{
		dumpReserved("this");
		return;
		}
		*/
	case OPC_FPUREG:
	case OPC_AUXREG:
		//		if (pSelf->m_pData)
		//			str = pSelf->m_pData->Name();
		//		else
		str = mrDC.toRegName(StorageId(pSelf), pSelf->Offset0(), pSelf->OpSize(), 0, buf);
		break;

	default:
		if (pSelf->IsIndirect() || IsAddr(pSelf))
		{
			if (IsAddr(pSelf))
				mos << "&";

			if (LocalRef(pSelf))
			{
				dumpFieldRef(LocalRef(pSelf));
				//pSelf->fieldRef()->nam ex(buf+strlen(buf));
				//				strcat(buf, str);
			}
			else
			{
				if (IsLocalOp(pSelf))
				{
					str = mrDC.toRegName(SSID_LOCAL, pSelf->Offset0(), pSelf->OpSize(), 0, buf);
					mos << str;
				}
				else
				{
					OutputIndirect(pSelf);
				}
			}

			//			buf[NAMELENMAX] = 0;
			//			mos << buf;
			return;
		}
	}//switch

	if (isempty(str))
		str = "???";
	strncpy(buf, str, NAMELENMAX);
	buf[NAMELENMAX - 1] = 0;

	mos << buf;
}

void FuncDumper_t::OutputOpType(CHOP pSelf)
{
	if (!IsAddr(pSelf))
	{
		if (m_disp.testOpt1(adcui::DUMP_CASTS))
		{
			//if ( !IsCodeOp() /*|| !PrimeOp(pSelf)->isHidden()*/ )
			{
				PushColor(adcui::COLOR_TYPES);
				mos << "{";
				output_optype(pSelf->OpType(), 1);
				mos << "}";
				PopColor();
			}
		}
	}
}


void FuncDumper_t::OutputPseudolabel0(CHOP pSelf)
{
	assert(IsPrimeOp(pSelf));

	//	CFieldPtr pLabel = 0;
	HOP pOp = pSelf;
	while (!IsRootVisible(pOp))
	{
		//		if (pOp->Label())
		//			pLabel = pOp->Label();
		pOp = NextPrimeEx(pOp);
	}
	/*
	if (!pLabel)
	{
	//find last real label
	HOP pOp2 = pOp;
	while (pOp2)
	{
	if (pOp2->Label())
	{
	pLabel = pOp2->Label();
	break;
	}
	pOp2 = pOp2->PrevEx();
	if (!pOp2)//first
	pLabel = pOp->GetOwner Func();
	}

	//?pLabel = LabelPrev(pOp2->Path());
	}
	*/
	//	if (!pLabel || !pLabel->Name())
	{
		static char buf[20];
		sprintf(buf, "%d", OpNo(pOp));
		mos << "$loc_" << buf;
	}
	//	else
	//	{
	//		mos << pLabel->Name();
	//	}
}

void FuncDumper_t::DumpOp0(CHOP pSelf)
{
CHECK(OpNo(pSelf) == 108)
STOP

	if (m_disp.isUnfoldMode())
	{
		if (CheckPseudoLabel(pSelf))
		{
			assert(0);
			//?OutputPseudolabel( ofs, pSelf );
		}

		if (!IsRootVisible(pSelf))
			return;

		if (m_disp.testOpt1(adcui::DUMP_NOVAROPS))
			if (IsVarOp(pSelf))
				return;

		//NewLine();
		drawCodeLine(pSelf);
		if (IsGoto(pSelf) && !IsLastExx(pSelf))
			NewLine();
		return;
	}

	//?	assert(IsPrimeOp(pSelf));

	//	if (isSplit())
	//		return;


	if (IsVarOp(pSelf))
	{
		if (!pSelf->isInRow())
		{
			//NewLine();
			drawCodeLine(pSelf);
		}
		return;
	}


	if (IsGoto(pSelf) && IsAddr(pSelf))
	{
		if (PathRef(pSelf))
		{
			const PathOpList_t &l(PathOps(PathRef(pSelf)));
			if (!l.empty())
			for (HOP pOp = PRIME(l.front()); pOp; pOp = NextPrimeEx(pOp))
			{
				if (IsRootVisible(pOp))
				{
					if (!pOp->isSplit())
						break;

					//NewLine();
					drawCodeLine(pOp);
				}
				//				if (pOp->IsLast())
				//					break;
			}
		}
	}

	if (pSelf->isGap())
		NewLine();

	if (CheckPseudoLabel(pSelf))
	{
		OutputPseudolabel(pSelf);
	}

	if (!IsRootVisible(pSelf))
		return;

	/*	if (!IsOut putDead())
	if (pSelf->IsDeadEx())
	return;

	if (isUnfoldMode())
	{
	}
	else
	{
	if (!IsRootVisible(pSelf))
	return;
	}*/

	drawCodeLine(pSelf);
}

void FuncDumper_t::DumpFuncUnfold()
{
//CHECKID(pField, 0x1038c)
//STOP

	NewLine();

	//dumpFieldRef(pField);

	if (!m_disp.dumpSym(adcui::SYM_FUNCDEF, FuncDefPtr()))//all flags
	{
		DumpDefinition();
	}

	if (!mrFuncDef.isStub())
	{
		//IncreaseIndent();
		TreePathHierIterator i(mrPathTree.tree());
		DumpPath(i);
		//DecreaseIndent();
	}
	else
	{
		NewLine();
		mos << "[";
		if (m_disp.showBlocks())
			mos << "{BODY}";
		NewLine();
		mos << "]";
	}
}

void FuncDumper_t::DumpFunc()
{
	NewLine();

	FuncDumpFlags_t flags(FUNCDUMP_IMPL_FLAGS);
	//if (m_disp.isUnfoldMode())
		//flags = FuncDumper_t::unfoldFlags();

	DumpFunctionDeclaration(FuncDefPtr(), flags, nullptr, PrimeSeg(), true);
	/*if (mrFunc.IsInvalid())
	{
	dumpComment("INVALID BODY", 0);
	}
	else*/
	{
		TreePathHierIterator i(mrPathTree.tree());
		if (i)
		{
			DumpPath(i);
		}
		else if (!m_disp.isUnfoldMode()) //a stub
		{
			mos << "{";
			//NewLine(false);
			DumpStrucLocs(true);
			mos << "}";
			DumpFunctionClosingInfo(FuncDefPtr());
		}
		else
		{
			NewLine();
			mos << "[";
			if (m_disp.showBlocks())
				mos << "{BODY}";
			NewLine();
			mos << "]";
		}
		//NewLine();
	}
}

void FuncDumper_t::DumpPath(TreePathHierIterator iSelf)
{
	if (m_disp.isUnfoldMode())
	{
		DumpPathLowProfile(iSelf);
		return;
	}

	HPATH pSelf(iSelf.data());
	HPATH pParent(iSelf.parentData());

//CHECK(pSelf->m.No() == 106)
//STOP
//CHECK(PathType(pSelf) == BLK_IFFOR)
//STOP

	TreePathHierIterator iBlockOpen;
	TreePathHierIterator iBlockClose;
	TreePathHierIterator iBlockDoWhile;

	switch (PathType(pSelf))
	{
	case BLK_ENTER:
	case BLK_EXIT:
	case BLK_DATA:
		return;
	default:
		break;
	}

	if (PathType(pSelf) == BLK_LOOPDOWHILE)
	{
		if (!CheckWhile(pSelf))
			if (CheckDoWhile(pSelf))
				iBlockDoWhile = iSelf;
	}

	bool bLessIndent(false);
	if (pSelf->IsRoot())
	{
		iBlockOpen = iBlockClose = iSelf;
	}
	else if (m_disp.showBlocks())
	{
		if (CheckElse(pSelf))
		{
			if (!CheckElseIf(pSelf))
			{
				NewLine();
//?				dumpTab(GetPathIndent(iSelf.parent().data()));
				dumpReserved("else");

				if (CheckBraces(pSelf))
					iBlockOpen = iBlockClose = iSelf.parent();
				else
					bLessIndent = (IncreaseIndent() != 0);
			}
		}
		else if ((pParent->Type() == BLK_IF || pParent->IsIfWhileOrFor()) && iSelf.isLast())
		{
			if (CheckBraces(pSelf))
			{
				iBlockOpen = iBlockClose = iSelf.parent();
			}
			else
			{
				//one line if/while block
				if (pSelf->IsIfWhileOrFor())
				{
				}
				else if (PathType(pSelf) == BLK_SWITCH)
				{
				}
				else if (PathType(pSelf) == BLK_LOOPDOWHILE)
				{
					if (!TreeIsFirst(pSelf))
						bLessIndent = (IncreaseIndent() != 0);
				}
				else if (PathType(pSelf) == BLK_IF || PathType(pSelf) == BLK_IFELSE)
				{
					if (iSelf.parent().parent() && iSelf.parent().parentData()->Type() == BLK_IFELSE)
						iBlockOpen = iBlockClose = iSelf.parent();
					else
						bLessIndent = (IncreaseIndent() != 0);
				}
				else
					bLessIndent = (IncreaseIndent() != 0);
			}
		}
		else if (PathType(pSelf) == BLK_SWITCH_0)
		{
			iBlockClose = iSelf;
		}
		else if (pParent->Type() == BLK_SWITCH)
		{
			if (!iSelf.isFirst())
				iBlockOpen = iSelf.parent();
		}
		else if (PathType(pSelf) == BLK_CASE)
		{
			DumpPathCases(iSelf);
		}
		else if (PathType(pSelf) == BLK_DEFAULT)
		{
			DumpDefaultCase(iSelf);
		}
		else if (PathType(pSelf) == BLK_LOOPENDLESS)
		{
			iBlockOpen = iBlockClose = iSelf;
		}
	}

	if (iBlockOpen)
	{
		NewLine();
		if (PathType(pSelf) == BLK_LOOPENDLESS)
		{
			dumpReserved("while");
			mos << " (1) ";
		}
		DumpOpeningBrace(iBlockOpen);

			bool bInvalidBody(false);
		if (IsFuncStatusAborted())
			bInvalidBody = true;
#if(0)
		if (FuncStatus() > 0)
		{
			if (FuncStatus() > FDEF_DC_FINISHED_OK)
			{
				//dumpComment(nullptr, true);//C++
				//disp().setCommentLevel(0);
				if (FuncStatus() < FDEF_DC_ERROR)
					dumpColorTerm("// PROCESSING...", adcui::COLOR_TAG_PROCESSING);
				//else
					//dumpColorTerm("// UNFINISHED!", adcui::COLOR_TAG_ERROR);
			}
			/*NewLine();
			dumpReserved("#if");
			mos << "(0)";
			dumpComment("THIS FUNCTION IS UNDER RECONSTRUCTION", 1);*/
		}
#endif
	}
	if (iBlockDoWhile)
	{
		NewLine();
		dumpReserved("do");
		mos << " ";
		DumpOpeningBrace(iBlockDoWhile);
		//bLessIndent = true;//will be decreased in second child
	}

	if (iBlockOpen && iBlockOpen.data()->IsRoot())//funcbeg
	{
		//?		TypeProc_t *pFunc = pSelf->GetOwner Func();
		//		if (pFunc->DumpLocals())
		//			NewLine();
		//		if (DumpLocalsEx())
		//			NewLine();
		//	if (pFunc->DumpStatics())
		//		NewLine();
	}

	if (pSelf->IsRoot())
	{
		DumpStrucLocs(false);
	}

	for (TreePathHierIterator i(true, iSelf); i; i++)
	{
		DumpPath(i);
	}

	if (pSelf->IsTerminal())// || Type() & BLK_LOGIC)
	{
		//		if (Label())
		//			Label()->Dump();

		DumpPathLabel(iSelf);
//		Path_DumpLocals(iSelf);

		HPATH hTop(GetLogicsTop(pSelf, 1));
		if (hTop)//check first
		{
			bool bLastOpIndent(false);
			if (hTop->Parent())//?
			if (hTop->Parent()->Type() == BLK_LOOPDOWHILE && hTop->Parent()->IsLastChild(hTop))
			{
				if (!CheckWhile(hTop->Parent()))
					if (CheckDoWhile(hTop->Parent()))
						bLastOpIndent = true;
			}
			//DumpPathOps(iSelf);
			PathOpList_t::Iterator i(pSelf->ops()); 
			while (i)
			{
				PathOpList_t::Iterator j(i++);
				if (!i && bLastOpIndent)//deacrease indent on a last op - a "while" condition
				{
					DecreaseIndent();
					bLessIndent = false;
				}
				DumpOp0(PRIME(j.data()));
			}
		}
	}

	/*if (PathType(pSelf) == BLK_SWITCH)
	{
		Path_DumpRedirectedCases(iSelf);
	}*/

	//	if (pBlockDoWhile)
	//		DumpCloseBrace( pBlockDoWhile );
	if (iBlockClose)// && showBlocks())
	{
		/*if (bInvalidBody)
		{
			NewLine();
			dumpReserved("#endif");
		}*/
		//NewLine();
		DumpCloseBrace();//iBlockClose);
		if (pSelf->IsRoot())
		{
			//output some debug info for function block
			DumpFunctionClosingInfo(FuncDefPtr());
		}

	}
	
	if (bLessIndent)
		DecreaseIndent();
}

void FuncDumper_t::DumpStrucLocs(bool bStub)
{
	if (!m_disp.testOpt1(adcui::DUMP_STRUCLOCS))
		return;
	CTypePtr iLocals((GlobToTypePtr)FuncDefPtr());//args+vars
	if (!HasArgs())
		iLocals = LocalVars();//vars only
	if (iLocals)
	{
		if (bStub)
			IncreaseIndent();
#if(NO_SUBDUMPS)
		bool bCont(false);
#else
		bool bCont(openScope(iLocals));
#endif
		DumpStrucDef(iLocals);
		if (bCont)
			m_disp.closeScope(false);
		if (bStub)
			DecreaseIndent();
		NewLine();
	}
}

int FuncDumper_t::Path_DumpCasesOf(CHPATH hSelf, CHPATH rPathSwitch, bool bRedirected)
{
	//HPATH pSelf(iSelf.data());
	assert(hSelf->IsTerminal());

//CHECK(hSelf.m.No() == 1062)
//STOP

		//get label
	//CFieldPtr pLabel = hSelf->m.Labe lAt();
	//if (!pLabel)
		//return 0;

	if (PathOpRefs(hSelf).empty())
		return 0;

	HPATH pSwitchPath(rPathSwitch->GetChildFirst());
	HPATH pJumpTablePath(GetJumpTablePath(pSwitchPath));
	HPATH pIndexTablePath(GetIndexTablePath(pJumpTablePath));

	int nCases = 0;
	for (XOpList_t::Iterator i(PathOpRefs(hSelf)); i; i++)
	{
		HOP pOpRef(i.data());

		if (!IsDataOp(pOpRef))
			continue;

		//		HOP pOpTblRef; 
		//		HOP pOpSwitch = pOpRef->GetSwitchOp(&pOpTblRef);
		//		assert(pOpSwitch);

		if (!bRedirected)
		{
			if (!IsCaseOf(pOpRef, rPathSwitch))
				continue;
		}
		else
		{
			//			if (pOpSwitch->Path() != pPathJmpSwitch)
			//				continue;
		}

		if (IsDefaultOf(pOpRef, rPathSwitch))
			continue;

		if (pIndexTablePath)
		{
			//assert(pIndexTable->m_pOps->OpSize() == 1);//byte
			for (PathOpList_t::Iterator i(PathOps(pIndexTablePath)); i; i++)
			{
				HOP pOp(PRIME(i.data()));
				ADDR vba(pJumpTablePath->anchor());
				ADDR va(vba + OpDisp(pOp) * mrDC.PtrSize());
				HOP pPtr = SwitchTracer_t::FindIOp(pJumpTablePath, va);
				assert(pPtr);
				if (pPtr == pOpRef)
				{
					NewLine(-1);//one less indent
					OutputCase(pOp);
				}
			}
		}
		else
		{
			NewLine(-1);//one less indent
			OutputCase(pOpRef);
		}

		nCases++;
	}

	return nCases;
}

/*void FuncDumper_t::Path_DumpRedirectedCases(TreePathHierIterator iSelf)
{
	assert(PathType(pSelf) == BLK_SWITCH);

	CFieldPtr pJumpTable = ->GetJumpTable(pSelf->GetChildFirst());

	int nCasesTotal = 0;
	Link_t *pDumpSv = m_pDumps->Last();
	for (
	HOP pPtr = pJumpTable->IOps();
	pPtr;
	pPtr = pPtr->N ext())
	{
	if (pPtr->IsCaseOf(pSelf))
	continue;

	CFieldPtr pLabel = pPtr->lab elRef();
	HPATH pPath = pLabel->Path();
	int nCases = Path_DumpCasesOf(pPath, pSelf, true);

	if (nCases > 0)
	{
	OutputGoto(pPtr);
	nCasesTotal += nCases;
	}
	}

	if (nCasesTotal > 0)
	{
	HPATH pPathLast = pSelf->TreeTerminalLast();
	if (PathType(pPathLast) != BLK_JMP)
	//		if (PathType(pPathLast) != BLK_RET)
	{
	assert(0);
	Link_t *pDump = 0;//?AddDump(DID_BREAK, pSelf);
	OutputBreak( pSelf );
	pDump->Unlink();
	pDump->LinkAfter(pDumpSv);
	}
	}
}*/

void FuncDumper_t::OutputGoto(CHOP pSelf)
{
	if (m_disp.isUnfoldMode())
	{
#if(0)
		if (IsRetOp(pSelf))
		{
			if (!m_disp.showBlocks())
			{
				OutputRet(pSelf);
				return;
			}
		}
#endif

		if (!IsRetOp(pSelf) && PathRef(pSelf))
		{
			dumpReserved("goto");
			dumpSpace();
			DumpLabelRef(PathRef(pSelf));
		}
		else if (pSelf->isAddr())
		{
			dumpReserved("goto");
			dumpSpace();
			dumpColorTerm(Int2Str(pSelf->m_disp, I2S_HEXC), adcui::COLOR_SQUIGGLE_RED);
		}
		else
			OutputExpression3(pSelf);

		/*if (!IsCondJump(pSelf))
		{
			if (pSelf->hasArgs())
			{
				//mos << ", ";
				mos << " (";
				OutputExpression3(pSelf);
				mos << ")";
			}
		}*/
		CloseStatement();
		return;
	}

	/*if (IsRetOp(pSelf))
		//	if (!IsObj Visible(pSelf->l abelRef()))
	{
		assert(IsLocalOp(pSelf));
		if (pSelf->m_disp == 0)
		{
			OutputRet(pSelf);
			return;
		}
	}*/

	/*if (IsDataOp(pSelf))
	{
		assert(0);
		dumpReserved("goto");
		mos << " ";
		DumpLabelRef(PathRef(pSelf));
		CloseStatement();
		return;
	}*/

CHECK(OpNo(pSelf) == 229)
STOP

	if (pSelf->isHidden())
	{
		dumpReserved("goto");
		mos << " ";
		DumpLabelRef(PathRef(pSelf));
		CloseStatement();
		return;
	}

	if (IsGoto(pSelf))
	{
		if (PathOf(pSelf) && !m_disp.isUnfoldMode())
		{
			if (PathType(PathOf(pSelf)) == BLK_JMPSWITCH)
			{
				if (m_disp.showBlocks() && PathType(PathOf(pSelf)->Parent()) == BLK_SWITCH)
				{
					dumpReserved("switch");
					mos << " (";
					OutputSwitch(pSelf);

					mos << ")";
				}
				else
				{
					//dumpReserved("goto");
					//mos << " ";
					OutputExpression3(pSelf);
					CloseStatement();
				}
				return;
			}
		}
	}
	else
	{
		assert(IsCondJump(pSelf));
	}

	/*if (IsRetOp(pSelf) && !IsBadRetOp(pSelf))
	{
		dumpReserved("return");
		//if (pSelf->hasArgs())
		{
			mos << " ";
			OutputExpression3(pSelf);
		}
		//else
			//m_disp.addProblem(pSelf, FuncInfo_t::CheckProblem(pSelf));
		CloseStatement();
		return;
	}*/

	/*if (IsRetOp(pSelf))
	{
		dumpReserved("goto");
		mos << " ";
		DumpLabelRef(PathRef(pSelf));
	}
	else */
	if (CheckBreak(pSelf))
	{
		dumpReserved("break");
		CloseStatement();
		return;
	}
	
	if (!IsAddr(pSelf))//like "goto eax" : probably switch jump
	{
		/*if (IsRetOp(pSelf) && !IsBadRetOp(pSelf))
			dumpReserved("return");
		else
			dumpReserved("goto");
		mos << " ";*/
		OutputExpression3(pSelf);
		CloseStatement();
		return;
	}
	
	if (PathRef(pSelf))
	{
		HOP pOpDest = PRIME(PathOps(PathRef(pSelf)).front());
		if (!pOpDest)
		{
			mos << "?";
			return;
		}
		if (!m_disp.isUnfoldMode() /*&& IsGoto()*/)
		{
			pOpDest = GetGotoDestOp(pSelf);
			if (IsRetOp(pOpDest))
			{
	//?			PushFont(adcui::COLOR_FONT_ITALIC);//not a real return
				//OutputRet(pOpDest);
				OutputExpression3(pOpDest);
				CloseStatement();
				return;
			}
		}

		dumpReserved("goto");
		mos << " ";
		HOP pOpGoto = GetLabelOp(pOpDest);
		if (!pOpGoto)
		{
			OutputPseudolabel0(pOpDest);
		}
		else
		{
#if(1)
			DumpLabelRef(PathOf(pOpGoto));
			m_disp.addProblem(pSelf, FuncInfo_t::CheckProblem(pSelf));
#else
			Out_t *pOut(m_disp.exprCache()->findOpExpr(pSelf));
			if (!pOut)
			{
				EXPR_t expr(*this, pSelf, m_disp.flags(), *m_disp.exprCache());
				pOut = expr.DumpExpression2(pSelf);
				EXPRSimpl_t ES(*this);
				ES.Simplify(pOutRoot);
				pOut->mpOp = pSelf;
				m_disp.addCachedExpression(pSelf, expr.FuncDefPtr(), pOut, expr.addresses());
				//m_disp.addProblem(pSelf, CheckProblem(pSelf));
			}
			OutExpr(pOut);
#endif
		}
	}
	else if (LocalRef(pSelf))//jumping to unanalized code branch?
	{
		dumpReserved("goto");
		mos << " ";
		dumpColorTerm("?", adcui::COLOR_TAG_ERROR);
		CFieldPtr pField(LocalRef(pSelf));
		mos << FieldName(pField);
	}
	else
	{
		dumpReserved("goto");
		mos << " ";

		MyString s;
		Locus_t loc;
		CFieldPtr pFieldRef(FindFieldInSubsegs(PrimeSeg(), pSelf->m_disp, loc));
		if (pFieldRef)
		{
			drawFieldName(pFieldRef, nullptr, false, adcui::COLOR_SQUIGGLE_RED);
		}
		else
		{
			s = Int2Str(pSelf->m_disp, I2S_HEXC);
			//ProjectInfo_t::VA2STR(OwnerSeg(), pSelf->m_disp)
			dumpColorTerm(s, adcui::COLOR_SQUIGGLE_RED);
		}
	}

	CloseStatement();
}

void FuncDumper_t::OutputCase(CHOP pIOp)
{
	assert(IsDataOp(pIOp));

	HOP pOp2;

	if (PathRef(pIOp))//isPtr())
	{
		pOp2 = GetSwitchOp(pIOp);
	}
	else
	{
		HPATH pPath(PathOf(pIOp));
		assert(PathType(pPath) == BLK_DATA);
		pPath = TreePrevSibling(pPath);
		if (PathType(pPath) != BLK_JMPSWITCH)//index table?
			pPath = TreePrevSibling(pPath);
		assert(PathType(pPath) == BLK_JMPSWITCH);
		pOp2 = GetLastOp(pPath);
		//assert(pTable->xrefs().check_count(1) == 0);
		//pOp2 = pTable->xrefs().head()->Op().op();
		//pOp2 = pOp2->GetSwitchOp2();
	}

	HPATH pPathJmpSwitch = PathOf(pOp2);
	assert(PathType(pPathJmpSwitch) == BLK_JMPSWITCH);
	SwitchQuery_t si;
	assert(TreeRoot(pPathJmpSwitch) == mrPathTree.body());
	//pPathJmpSwitch->m.pathTree()->GetSwitchInfo(pPathJmpSwitch, si);
	GetSwitchInfo(pPathJmpSwitch, si);

	if (!si.pJumpTable)
	{
		dumpColorTerm("case ?:", adcui::COLOR_TAG_ERROR);
		return;
	}

	int nValue;
	if (si.pIndexTable)
	{
		assert(!si.nIndexOffset1);
		int off(pIOp->VA() - si.pIndexTable->_key());
		nValue = off / pIOp->OpSize() - si.nIndexOffset2;
	}
	else
	{
		int off(pIOp->VA() - si.pJumpTable->_key());
		nValue = off / pIOp->OpSize() - si.nIndexOffset1;
	}

	//?	int nIndent = GetPathIndent(PathOf(pOp2));
	//?	dumpTab(nIndent);
	dumpReserved("case");
	mos << " ";
	mos << Int2Str(nValue, I2S_HEXC);
	mos << ":";
}

void FuncDumper_t::DumpDefaultCase(TreePathHierIterator iSelf)
{
//?	int nIndent = GetPathIndent(iSelf.data());
//?	dumpTab(nIndent);
	dumpReserved("default");
	mos << ":";
}

void FuncDumper_t::OutputPathCondjump(CHPATH pSelf)
{
	assert(PathType(pSelf) == BLK_JMPIF);

	if (m_disp.isUnfoldMode())
	{
		dumpReserved("if");
		mos << " (";
		OutputPathCondition3(pSelf);
		mos << ")";
		OutputPathGoto(pSelf);
		return;
	}

	HPATH pPath = pSelf;
	HPATH pPathN = GetLogicsTop(pPath, 1);//check first
	if (pPathN)
		pPath = pPathN;

	int res = 0;
	HPATH pParent = pPath->Parent();
	if (pParent)
	{
		if (m_disp.showBlocks())
		{
			if (pParent->Type() == BLK_IFWHILE)
			{
				dumpReserved("while");
				mos << " (";
				OutputPathCondition3(pPath);
				mos << ")";
				OutputPathGoto(pPath);
				return;
			}

			if (pParent->Type() == BLK_IFFOR)
			{
				dumpReserved("for");
				mos << " (";
				OutputPathCondition3(pPath);
				mos << ")";
				OutputPathGoto(pPath);
				return;
			}

			if (pParent->IsLastChild(pPath) && pParent->Type() == BLK_LOOPDOWHILE)//pParent->CheckDoWhile())
			{
				mos << "} ";
				dumpReserved("while");
				mos << " (";
				OutputPathCondition3(pPath);
				mos << ");";
				return;
			}

			if (CheckElseIf(pParent))
			{
				dumpReserved("else");
				mos << " ";
			}
		}
	}

	dumpReserved("if");
	mos << " (";
	OutputPathCondition3(pPath);
	mos << ")";
	OutputPathGoto(pPath);
	return;
}

void FuncDumper_t::OutputPseudolabel(CHOP  pOp)
{
	OutputPseudolabel0(pOp);
	mos << ":";
}

/*const char * FuncDumper_t::GetDeclStr(A rg_t * pSelf)
{
	if (!pSelf->mSsid)
		return "";

	static char str[256];
	std::ostrstream os(str, sizeof(str));

	os.seekp(0);
#if(TYP_ OLD)
	root().Output TYPE0(&pSelf->m_type, 1);
#else
	EXPR_t ex(mrFunc, 0);
	expr::GType *pType = ex.mTypesMgr.fromOpType((OpType_t)pSelf->m_type.OpType());
	root().Outpu tTYPE0(pType, 1);
#endif
	mos << pSelf->GetName();
	if (pSelf->m_type.m_nArray)
		mos << "[" << pSelf->m_type.m_nArray << "]";
	mos << '\0';
	return str;
}*/


/*void FuncDumper_t::OutputFuncDefScript()
{
	FuncProfile_t fp;
	GetFuncProfile(fp);
	StubInfo_t::dump(fp, mos);
}*/

void FuncDumper_t::OutputOutType(const TYP_t &t)
{
	CExprName o(*this);
	t.dump(mos, o);
}




