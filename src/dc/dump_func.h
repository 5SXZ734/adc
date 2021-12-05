#pragma once

#include "dump_file.h"
#include "cc.h"

class ProtoDumper_t : public DumperBase_t<ProtoInfo_t>
{
public:
	ProtoDumper_t(const ProtoInfo_t&, FileDumper_t*, int indent, Display_t&, ProbeEx_t*, const MyLineEditBase*);
};

/////////////////////////////////////////
class FuncDumper_t : public DumperBase_t<FuncInfo_t>//,
	//public FuncInfo_t
{
public:
	FuncDumper_t(const FuncInfo_t&, FileDumper_t*, int indent, Display_t&, ProbeEx_t*, const MyLineEditBase*);
	//virtual const FileInfo_t &fileInfo() const { return *this; }
	//virtual const DcInfo_t &dcInfo() const { return *this; }

	void	DumpNoFuncdef();
	void	OutputStubRet();

	void	DumpFuncUnfold();
	void	DumpFunc();
	//void	OutputFuncDefScript();
	void	dumpCallType();

	const Out_t *	GetTop2(const Out_t * pSelf, bool &bIsRight) const;
	bool	CheckTypeOutput(const Out_t *) const;
	bool	CheckParenthesis(const Out_t *) const;
	int		CheckGOTOStatus5(CHPATH ) const;
	int		GetPathIndent(CHPATH) const;
	bool	IsConjumpVisible(CHPATH)const;
	int		CheckBraces(CHPATH) const;
	void	OutputPathCondition3(CHPATH);
	void	OutputPathGoto(CHPATH);
	void	OutputPathCondjump(CHPATH);
	int		Path_DumpCasesOf(CHPATH, CHPATH, bool bRedirected = false);//must be terminal
	void 	OutputAddrRef(ADDR ofs, OpType_t) const;
	void	OutputSimple0(const Out_t *);

	//void	dumpFuncArg(CFieldPtr 
	void	DumpLabelRef(CHPATH);
	void	DumpLabelDecl(CHPATH);
	bool	IsLabelDead(CHPATH) const;
	int		CheckLabelDead(CHPATH) const;
	//void	DumpLocal(CFieldPtr );
	void	drawLabelDecl(PathPtr);
	void	drawLabelDecl0(PathPtr);
	bool	IsJumpTableVisible(CHPATH) const;
	int		CheckDoWhile(CHPATH) const;

	void	DumpOpeningBrace(TreePathHierIterator);
	void	DumpPath(TreePathHierIterator);
	void	DumpPathLowProfile(TreePathHierIterator);
	void	DumpPathCases(TreePathHierIterator);//must be BLK_CASE!
	void	DumpDefaultCase(TreePathHierIterator);
	void	DumpPathOps(CHPATH, bool bInRow);
	void	DumpPathLabel(TreePathHierIterator);
	void	dumpFuncBannerComment(bool bIlfakLike);
	void	DumpStrucLocs(bool bStub);

	//void	DumpFuncDecl(unsigned);
	//int IsLabelVisible(CFieldPtr pSelf) const;
	//void	dumpField_local2(CFieldPtr);//full declaration with indent and semicolon
	//CFieldPtr Field_DumpUnion(CFieldPtr);
	//CFieldPtr Field_DumpStruc(CFieldPtr);
	//int		GetDataIndent(CFieldPtr) const;
	//void	OutputElse(TreePathHierIterator);
	//void	OutputDo(TreePathHierIterator);
	//void	Path_DumpRedirectedCases(TreePathHierIterator);
	//void	OutputPath(TreePathHierIterator);
	//void	OutputBreak(TreePathHierIterator);
	//void	OutputArg(CHOP);
	//void	OutputArg2(CHOP);

	void	OutputOpType(CHOP);
	void	OutputOperand(CHOP);
	unsigned OpState(CHOP) const;
	void	OutputCondjump(CHOP);
	void	OutputExpression3(CHOP);
	void	OutputRet(CHOP);
	void	OutputGoto(CHOP);
	void	OutputSwitch(CHOP);
	void	OutputPseudolabel0(CHOP);
	//void	OutputExpression4(CHOP);
	void	OutputPrefix(CHOP);
	void	drawCodeLine0(CHOP);
	void	drawCodeLine(CHOP);
	void	DumpOp0(CHOP);
	void	OutputPseudolabel(CHOP);
	void	OutputSimple(CHOP);
	void	OutputOpName(CHOP);
	void	OutputIndirect(CHOP);
	void	OutputCase(CHOP);
	void	OutputIGoto(CHOP);
	int		CheckPseudoLabel(CHOP) const;
	int		IsOpVisible(CHOP) const;
	HOP		GetLabelOp(CHOP) const;
	int		IsRootVisible(CHOP) const;
	bool	IsDataDead(CFieldPtr) const;
	int		GetIndent(CHOP) const;
	void	OutputOpNameUnfold(CHOP);

	bool	IsIndirectCall(const Out_t *) const;
	void	OutputOutType(const TYP_t &t);
	void	OutExprT(const Out_t *);
	void	OutExpr(const Out_t *);
	void	OutAction(const Out_t *, bool = false);
	void	OutputOp(const Out_t *);
	void	OutputTermName(const Out_t *);

	void	CloseStatement();
	void	OutputBodyStub();
	ProblemInfo_t	CheckProblem(Out_t *) const;
	void	DumpDefinition();

	void DumpXRefs(const XOpList_t &, bool bExtra = false);
	void DumpXIns(const XOpList_t &, bool bExtra = false);
	void DumpXOuts(const XOpList_t &, bool bExtra = false);

protected:
	const FileInfo_t& fileInfo() const { return *this; }
};



//for locals vars dumping
template <typename T_Dumper>
class ProtoImpl4V_t : public ProtoImpl_t<T_Dumper>
{
	typedef ProtoImpl_t<T_Dumper> BASE;
protected:
	using BASE::mrDumper;
public:
	ProtoImpl4V_t(T_Dumper& r)
		: BASE(r)
	{
	}
	virtual void drawFieldName(CFieldPtr pLocal) {
		BASE::drawFieldName(pLocal);
		if (ProtoInfo_t::IsSpoiltReg(pLocal))
		{
			assert(FuncInfo_t::LocalRefs(pLocal).empty());
			mrDumper.dumpComment("spoilt", false);
		}
		else if (ProtoInfo_t::IsRetVal(pLocal))
		{
			assert(FuncInfo_t::LocalRefs(pLocal).empty());
			mrDumper.dumpComment("returned", false);
		}
		else
		{
			const XOpList_t& refs(FuncInfo_t::LocalRefs(pLocal));
			if (!refs.empty())
				mrDumper.DumpXRefs(refs);
			else //if (FuncInfo_t::IsAnyLocalVar(pLocal) || FuncInfo_t::IsLocalArg(pLocal)))
				mrDumper.dumpComment("unused", false);
		}
	}
};



/////////////////////////////////////////////////////// Dumper2_t
template <typename T_Dumper/* = FuncDumper_t*/>
class ProtoImpl4F_t : public ProtoImpl2_t<T_Dumper>//for glob-funcs dumping
{
	typedef ProtoImpl2_t<T_Dumper> BASE;
protected:
	typedef		ProtoImpl4F_t	ARGDUMPER;
	using typename BASE::ARGINFO;
	using BASE::mrDumper;
public:
	using BASE::mbIsDef;
public:
	ProtoImpl4F_t(T_Dumper& r)
		: BASE(r)
	{
	}

	virtual bool getFunctionArguments(CTypeBasePtr pSelf, std::vector<ARGINFO>& args) const
	{
		const FuncDef_t* pf(pSelf->typeFuncDef());
		if (pf)
		{
			//FuncInfo_t FI(mrDumper, *pSelf->objGlob());
			CallingConv_t CC(mrDumper, pSelf->objGlob());
			if (pf->hasArgFields())
			{
				FuncCCArgsCIt<> i(*pf, CC);
				if (ProtoInfo_t::IsThisPtrArg(VALUE(i)))
					++i;//skip this ptr
				for (; i; ++i)
				{
					CFieldPtr pArg(VALUE(i));
					MyString sReg;
					if (FuncInfo_s::IsLocalReg(pArg))
					{
						ri_t r0(FuncInfo_s::toR_t(pArg));
						ri_t r(i.toR_t(true));//ignore actual storage
						if (r0 != r)
							sReg = mrDumper.LocalRegToString(pArg);//display explicit (register) storage if the field doesn't comply with CC
					}
					CTypePtr pArgType(pArg->type());
					//if (pArgType && pArgType->typeEnum())
						//pArgType = pArgType->baseType();
					if (!isArgVisible(pArg))
						pArg = nullptr;
					args.push_back(ARGINFO(pArgType, pArg, sReg));
				}
			}
			return ProtoInfo_s::IsFuncVarArged((CGlobPtr)pSelf);
		}
		return BASE::getFunctionArguments(pSelf, args);
	}

	virtual void getFunctionReturnValues(CTypePtr pSelf, std::vector<ARGINFO>& rets)
	{
		const FuncDef_t* pfDef(pSelf->typeFuncDef());
		if (pfDef)
		{
			//FuncInfo_t FI(mrDumper, *pSelf->objGlob());
			CallingConv_t CC(mrDumper, pSelf->objGlob());
			for (FuncCCRetsCIt i(*pfDef, CC); !i.isAtEnd(); ++i)
			{
				CFieldPtr pRet(i.field());
				MyString sReg;
				if (FuncInfo_s::IsLocalReg(pRet))
				{
					ri_t r0(FuncInfo_s::toR_t(pRet));
					ri_t r(i.toR_t(true));//ignore actual storage
					if (r0 != r)//display explicit (register) storage if the field doesn't fit into CC
						sReg = mrDumper.LocalRegToString(pRet);
				}
				CTypePtr pRetType(pRet->type());
				//if (pRetType && pRetType->typeEnum())
					//pRetType = pRetType->baseType();
				rets.push_back(ARGINFO(pRetType, pRet, sReg));
			}
		}
		else//dumping a retval?
		{
			BASE::getFunctionReturnValues(pSelf, rets);
		}
	}

	virtual void drawFieldName(CFieldPtr pSelf)
	{
		if (FuncInfo_s::isLocalArg(pSelf))
		{
			adcui::Color_t iColor(adcui::COLOR_NULL);

			if (pSelf->nameless())
			{
				FuncDef_t* pfDef(pSelf->owner()->typeFuncDef());
				if (pfDef->isStub() || !mbIsDef)
					iColor = adcui::COLOR_UNEXPLORED;
			}

			mrDumper.drawFieldName(pSelf, iColor);
			
			const XOpList_t& refs(FuncInfo_t::LocalRefs(pSelf));
			if (!refs.empty())
				mrDumper.DumpXRefs(refs);
		}
		else
			BASE::drawFieldName(pSelf);
	}

private:
	bool isArgVisible(CFieldPtr pArg) const {
		//if (!bIsArg)
			//return true;
		assert(FuncInfo_t::IsLocalArg(pArg));

		CGlobPtr pCFunc((CGlobPtr)pArg->owner());
		FuncDef_t* pfDef(pCFunc->typeFuncDef());
		assert(pfDef);

		bool bFieldHidden = true;
		if (DcInfo_t::DockField(pCFunc))//no args for proto types
		{
			//suppress unnamed args (for func declarartions and stubs)
			if (mrDumper.disp().testOpt1(adcui::DUMP_ALL_ARGS))//all args
				bFieldHidden = false;
			else if (!pArg->nameless())
				bFieldHidden = false;
			else if (!pfDef->isStub() && mbIsDef)//dumping definition, not a stub
			{
				const XOpList_t& refs(FuncInfo_t::LocalRefs(pArg));
				if (!refs.empty())
					bFieldHidden = false;
			}

			/*		if (!bFieldHidden && pArg->nameless())
						if (pfDef->isStub() || !mbIsDef)
						miColor = adcui::COLOR_UNEXPLORED;*/
		}
		return !bFieldHidden;
	}
};





