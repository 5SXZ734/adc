#pragma once

#include <string>

#include "expr.h"

#define SIMPL_NEW	0

enum ESimplifyOutcome : unsigned
{
#define SIMPL(a) SIMPL_##a
	SIMPL_NULL,
	SIMPL(ADD1), SIMPL(ADD2), SIMPL(ADD3), SIMPL(ADD4), SIMPL(ADD5), SIMPL(ADD6),
	SIMPL(AND1),
	SIMPL(ARR1), SIMPL(ARR2),
	SIMPL(ASS1), SIMPL(ASS2), SIMPL(ASS3),	SIMPL(ASS4), SIMPL(ASS5), SIMPL(ASS6), SIMPL(ASS7), SIMPL(ASS8), SIMPL(ASS9),
	SIMPL(BIN1), SIMPL(BIN2),
	SIMPL(CAL0), SIMPL(CAL1), SIMPL(CAL2), SIMPL(CAL3), SIMPL(CAL4), SIMPL(CAL8), SIMPL(CAL9),
	SIMPL(CHK1), SIMPL(CHK2), SIMPL(CHK3), SIMPL(CHK4), SIMPL(CHK5),
	SIMPL(CHS1), SIMPL(CHS2), SIMPL(CHS3), SIMPL(CHS4), SIMPL(CHS5), SIMPL(CHS6), SIMPL(CHS7), SIMPL(CHS8), SIMPL(CHS9),
	SIMPL(CMP1),
	SIMPL(CND1),
	SIMPL(COM1),
	SIMPL(DAT1), SIMPL(DAT2), SIMPL(DAT3), SIMPL(DAT4), SIMPL(DAT5), SIMPL(DAT6), SIMPL(DAT7), SIMPL(DAT8), SIMPL(DAT9),
	SIMPL(DIV1), SIMPL(DIV2), SIMPL(DIV3), SIMPL(DIV4), SIMPL(DIV5), SIMPL(DIV6), SIMPL(DIV7),
	SIMPL(EXP1), SIMPL(EXP2), SIMPL(EXP3), SIMPL(EXP4), SIMPL(EXP5), SIMPL(EXP6), SIMPL(EXP7), SIMPL(EXP8), SIMPL(EXP9),
	SIMPL(EXT1), SIMPL(EXT2), SIMPL(EXT3), SIMPL(EXT4), SIMPL(EXT5), SIMPL(EXT6),
	SIMPL(FRA1), SIMPL(FRA2), SIMPL(FRA3), SIMPL(FRA4), SIMPL(FRA5), SIMPL(FRA6), SIMPL(FRA7), SIMPL(FRA8), SIMPL(FRA9),
	SIMPL(FRB1), SIMPL(FRB2), SIMPL(FRB3),
	SIMPL(GET1), SIMPL(GET2), SIMPL(GET3),
	SIMPL(HAL1),
	SIMPL(HIG1), SIMPL(HIG2),
	SIMPL(IMM1), SIMPL(IMM2), SIMPL(IMM3), SIMPL(IMM4), SIMPL(IMM5), SIMPL(IMM6),
	SIMPL(IND1), SIMPL(IND2), SIMPL(IND3), SIMPL(IND4), SIMPL(IND5), SIMPL(IND6), SIMPL(IND7),
	SIMPL(LOG1), SIMPL(LOG2), SIMPL(LOG3), SIMPL(LOG4), SIMPL(LOG5), SIMPL(LOG6), SIMPL(LOG7),
	SIMPL(LOH1), SIMPL(LOH2),
	SIMPL(MUL1), SIMPL(MUL2), SIMPL(MUL3), SIMPL(MUL4), SIMPL(MUL5), SIMPL(MUL6), SIMPL(MUL7), SIMPL(MUL8),
	SIMPL(NEG1), SIMPL(NEG2), SIMPL(NEG3), SIMPL(NEG4), SIMPL(NEG5), SIMPL(NEG6), SIMPL(NEG7),
	SIMPL(OFF1), SIMPL(OFF2), SIMPL(OFF3), SIMPL(OFF4),
	SIMPL(OPD1), 
	SIMPL(PRE1), SIMPL(PRE2), SIMPL(PRE3),
	SIMPL(PST1), SIMPL(PST2), SIMPL(PST3), SIMPL(PST4), SIMPL(PST5), SIMPL(PST6), SIMPL(PST7), SIMPL(PST8), SIMPL(PST9), 
		SIMPL(PSTA), SIMPL(PSTB), SIMPL(PSTC), SIMPL(PSTD), SIMPL(PSTE), SIMPL(PSTF), SIMPL(PSTG), SIMPL(PSTH), SIMPL(PSTI), SIMPL(PSTJ), SIMPL(PSTK),
	SIMPL(PTR1), SIMPL(PTR2), SIMPL(PTR3), SIMPL(PTR4), SIMPL(PTR5), SIMPL(PTR6), SIMPL(PTR7),
	SIMPL(REC1),
	SIMPL(RET1), SIMPL(RET2),
	SIMPL(ROW1),
	SIMPL(SHL1), SIMPL(SHL2),
	SIMPL(SHR1), SIMPL(SHR2),
	SIMPL(SUB1), SIMPL(SUB2), SIMPL(SUB3), SIMPL(SUB4), SIMPL(SUB5), SIMPL(SUB6),
	SIMPL(SWA1), SIMPL(SWA2), SIMPL(SWA3), SIMPL(SWA4), SIMPL(SWA5),
	SIMPL(THS1), SIMPL(THS2), SIMPL(THS3), SIMPL(THS4), SIMPL(THS5), SIMPL(THS6), SIMPL(THS7),
	SIMPL(TYP1), SIMPL(TYP2), SIMPL(TYP3), SIMPL(TYP4), SIMPL(TYP5), SIMPL(TYP6), SIMPL(TYP7), SIMPL(TYP8), SIMPL(TYP9), 
		SIMPL(TY10), SIMPL(TY11), SIMPL(TY12), SIMPL(TY13), SIMPL(TY14), SIMPL(TY15), SIMPL(TY16), SIMPL(TY17), SIMPL(TY18), SIMPL(TY19),
		SIMPL(TY20), SIMPL(TY21), SIMPL(TY22),
	SIMPL(UNI1),
	SIMPL(UNK1), SIMPL(UNK2),
	SIMPL(VAR1), SIMPL(VAR2), SIMPL(VAR3),
	SIMPL(ZER1), SIMPL(ZER2), SIMPL(ZER3), SIMPL(ZER4), SIMPL(ZER5), SIMPL(ZER6),
	SIMPL_LAST
#undef SIMPL
};

std::string ESimplifyOutcome2string(ESimplifyOutcome e);


////////////////////////////////
// EXPRESSION REFACTORING

#define RETURN_1(a)	__done(SIMPL_##a, 1);
#define RETURN_M1(a)	__done(SIMPL_##a, -1);
#define RETURN_M2(a)	__done(SIMPL_##a, -2);
#define RETURN_M3(a)	__done(SIMPL_##a, -3);
#define CHECK_EXPR(branch, call) \
	while (branch) \
	{ \
		int n = call; \
		if (n == 0) \
			break; \
		if (n < 0) \
		{ \
			if (++n == 0) \
				continue; \
		} \
		return n; \
	}
#define CHECK_NODE(call) \
	if (int n = call) \
		return n;


class EXPRSimpl_t : public ExprInfoBase_t
{
	int			miStep;//simpification step no
#ifdef _DEBUG
protected:
	int z_step;
#endif

protected:
	ESimplifyOutcome	meResult;//last result of expression simplification

public:
	EXPRSimpl_t(const ExprInfoBase_t& r)
		: ExprInfoBase_t(r),
		miStep(0),
#ifdef _DEBUG
		z_step(0),
#endif
		meResult(SIMPL_NULL)
	{
	}

	EXPRSimpl_t(const FuncInfo_t& r, adcui::UDispFlags flags, ExprCache_t& ec)
		: ExprInfoBase_t(r, flags, ec),
		miStep(0),
#ifdef _DEBUG
		z_step(0),
#endif
		meResult(SIMPL_NULL)
	{
	}


protected:
	virtual void dump(int, const Out_t *, ESimplifyOutcome) const { ((EXPRSimpl_t*)this)->meResult = SIMPL_NULL; }
	virtual bool checkDump(int, int) const { return false; }

	ADDR GetDataOffset(CFieldPtr) const;
	FieldPtr GetAttachedLocal(Out_t &) const;

	int MoveHigher(Out_t &) const;
	int DivideWith(Out_t &, int n, int *pRem = 0) const;
	int CheckThrough(Out_t &, Out_t *pOpL, Out_t *pOpR) const;
	int CheckThrough3(Out_t &, Out_t *pOpL, Out_t *pOpR) const;
	int RetypeTerm(Out_t *, OpType_t) const;
	int	ReSize(Out_t &, int dbottom, int dtop) const;
	//void PropagatePtrUp(Out_t &) const;

public:
	void	SimplifyNew(Out_t *) const;
	void	Simplify(Out_t *) const;//int id = -1);
	bool	SimplifySwitch(Out_t*, SwitchQuery_t &si, CHPATH swch) const;//switch
	int PreSimplify(Out_t *, unsigned, unsigned = -1) const;
	int PrimarySimplify(Out_t *, unsigned, unsigned = -1) const;
	int SecondarySimplify(Out_t *, unsigned, unsigned = -1) const;
	int PostSimplify(Out_t *, unsigned, unsigned = -1) const;
	//int CheckConformantCall(Out_t *) const;
	int CheckThisCall(Out_t *) const;//move `this` ptr out of args list
	int CheckConformantCallType(Out_t *) const;
	int CheckConformant(CGlobPtr, Out_t *call) const;
	int CheckConformant(CFieldPtr, Out_t *call) const;
	int CheckConformant(Out_t *call) const;
	int CheckConformantRet(Out_t *) const;
	int CheckRet(Out_t*) const;
	int CheckImpCall(Out_t*) const;

	int		DispatchSimplifyNew(Out_t*) const;

	int		DispatchSimplifyPrimary(Out_t *) const;
	int		DispatchSimplifyPrimary2(Out_t *) const;
	int		DispatchSimplifyAssign(Out_t *, bool) const;
	int		DispatchSimplifySecondary(Out_t *) const;
	int		DispatchSimplifyPost(Out_t *) const;
	//int		DispatchSimplifyCall(Out_t *) const;

	int		AttachGlob2(Out_t *) const;
	Out_t*	AttachGlob(Out_t*) const;

	Out_t *Simplify8_2(Out_t *, Out_t *) const;
	Out_t *FindPeerTerm(const Out_t *, const Out_t *) const;
	Out_t *FindPeerTermImpl(const Out_t *, const Out_t *) const;
	Out_t *Simplify8_3(Out_t *, Out_t *) const;

	int ShiftFractal(Out_t *) const;
	int AdjustFractal(Out_t *) const;
	int UniteFractal(Out_t *) const;
	int IsUnitableWith(Out_t *pSelf, Out_t *pOut1, Out_t *pOut2) const;
	int AlignFractal(Out_t *, Out_t *, int) const;
	//int NormalizeIndex(Out_t *pSelf, int nCellSz, int *pRem = 0) const;

	void SetupTypes(Out_t *) const;
	int SimplifyType(Out_t *, bool bSecondary) const;
	int SimplifyType4(Out_t *) const;

	void SetTyp(Out_t* p, const TYP_t& t) const;
	int SimplifyIndir4(Out_t *) const;

	int	SimplifyAssign(Out_t *) const;
	int SimplifyAssign2(Out_t *) const;
	int SimplifyAssCheck(Out_t *) const;
	int	SimplifyHalf1(Out_t *) const;
	int SimplifyHalf2(Out_t *) const;
	int SimplifyAndOr(Out_t*) const;
	int	SimplifyCheck(Out_t *) const;
	int SimplifyRawCondition(Out_t *) const;
	int	SimplifyGet(Out_t*, bool) const;
	int	SimplifySign(Out_t *) const;
	int	SimplifyAdd(Out_t *) const;
	int	SimplifySub(Out_t *) const;
	int	SimplifyMultiply(Out_t*) const;
	int	SimplifyDivide(Out_t*) const;
	int	SimplifyShiftLeft(Out_t*) const;
	int	SimplifyShiftRight(Out_t*) const;
	int	SimplifyBitwiseAnd(Out_t*) const;
	int	SimplifyS0(Out_t *) const;
	int SimplifyPtr2(Out_t *) const;
	int SimplifyIndir2(Out_t *) const;
	int SimplifyOffs(Out_t *) const;
	int Simplify6(Out_t *) const;
	int SimplifyMulDiv(Out_t *) const;
	int SimplifyCompar(Out_t *) const;
	int SimplifyPostIncrement(Out_t *) const;
	int SimplifyEqu(Out_t*) const;
	int SimplifySign2(Out_t *) const;//sign
	//int ReplaceReal80Types(Out_t *) const;
	int SimplifyInrowCheck(Out_t *) const;
	int SimplifyVar(Out_t *) const;
	//int	SimplifyMultiplier(Out_t *) const;
	int	SimplifyCommonMult(Out_t *) const;
	int	SimplifyFract(Out_t *) const;
	int	SimplifyCompMov(Out_t *) const;
	int	SimplifyLogic(Out_t *) const;
	int	SimplifyLogicPost(Out_t *) const;
	int	SimplifyLogic2(Out_t *) const;
	int	SimplifyIndir(Out_t *) const;
	int	SimplifyIndir3(Out_t*) const;
	int	SimplifySwitchEx(Out_t *) const;
	int	SimplifyArrays(Out_t *) const;
	int	SimplifyThis(Out_t *) const;
	int	SimplifyArrCall(Out_t *) const;
	int	SimplifyThis0(Out_t *) const;
	int	SimplifyExt(Out_t *) const;
	int	SimplifyThruConst(Out_t*) const;
	int	SimplifyOp2Field(Out_t *) const;
	int SimplifyFinal(Out_t *) const;//remove invisible terms (OUT_ARGs..)
	int	SimplifyArgs(Out_t *) const;

	int __done(ESimplifyOutcome e = SIMPL_NULL, int ret = 1) const
	{
		((EXPRSimpl_t*)this)->meResult = e;
		return (e != SIMPL_NULL)?ret:0;
	}

	void __dump(int i, Out_t *pOut) const
	{
		dump(i, pOut, meResult);
		((EXPRSimpl_t*)this)->meResult = SIMPL_NULL;
		((EXPRSimpl_t*)this)->miStep = i + 1;
	}
};



