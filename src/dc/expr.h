#pragma once

#include <set>
#include "shared/misc.h"
#include "db/mem.h"
#include "db/types.h"
#include "info_func.h"
#include "expr_type.h"
#include "arglist.h"
#include "expr_cache.h"

struct SwitchQuery_t;
enum ESimplifyOutcome : unsigned;

template <typename T> class XRef_t;

enum
{
	EXPR_UNFOLD		= 0x00000001,
	//EXPR_NOEXTEND	= 0x00000010,
	EXPR__LAST		= 0xFFFFFFFF
};

/*class IExprDump
{
public:
	virtual void dump(int, Out_t *, ESimplifyOutcome){}
};*/


class ExprInfoBase_t : public FuncInfo_t
{
	int	mNo;
public:
	ExprCache_t	&mrExprCache;
	adcui::UDispFlags	m_flags;
	bool	mbDeadOp;
public:
	ExprInfoBase_t(const FuncInfo_t& r, adcui::UDispFlags flags, ExprCache_t &ec)
		: FuncInfo_t(r),
		mNo(-1),
		mrExprCache(ec),
		m_flags(flags),
		mbDeadOp(false)
	{
	}
	ExprInfoBase_t(const ExprInfoBase_t& r)
		: FuncInfo_t(r),
		mNo(r.mNo),
		mrExprCache(r.mrExprCache),
		m_flags(r.m_flags),
		mbDeadOp(r.mbDeadOp)
	{
	}

	ExprCache_t& exprCache() { return mrExprCache; }
	expr::CTypeMgr &typesMgr(){ return mrExprCache.mTypesMgr; }
	const expr::CTypeMgr &typesMgr() const { return mrExprCache.mTypesMgr; }

	void SetNo(int n) const { ((ExprInfoBase_t *)this)->mNo = n; }
	int No() const { return mNo; }

	Out_t *NewTerm() const;
	Out_t *InsertParent(Out_t *, Action_t, bool bDockOpFromParent = false) const;
	Out_t *InsertParent(Out_t *, const TYP_t &, bool bDockOpFromParent = false) const;
	Out_t *InsertParent(Out_t *, Out_t *) const;
	Out_t *Detach(Out_t *) const;
	Out_t *DetachParent(Out_t *) const;
	Out_t *Clone(Out_t &, Out_t *pOut = 0) const;
	void Dispose(Out_t *) const {}

	void SetDockOp(Out_t *, CHOP) const;

	Out_t *Add(Out_t &) const;
	Out_t *Add(Out_t &, Out_t &, bool bCopy = false) const;
	Out_t *Add(Out_t &, CHOP, Action_t) const;
	Out_t *Add(Out_t &, CHOP, int) const;
	Out_t *Add(Out_t &, FieldPtr, CHOP = HOP()) const;
	Out_t *Add(Out_t &, CHOP, const TYP_t &) const;
	//Out_t *Add(Out_t &, CHOP, HPATH) const;
	Out_t *Add(Out_t &, CHOP, VALUE_t v) const;
	Out_t *Add(Out_t *, CHOP) const;
	Out_t *Add(Out_t *, CHOP, uint8_t cl, uint8_t typ, uint8_t id) const;
	Out_t *Add2(Out_t *, CHOP, uint8_t cl, uint8_t off, uint8_t siz) const;
	Out_t *AddB(Out_t *, CHOP) const;

	TYP_t TypOf(const Out_t* p) const;
	expr::GType *FromTyp(Out_t*) const;

	expr::GType* PtrOf(expr::GType*) const;
	expr::GType* DerefOf(expr::GType*) const;
	expr::GType* ProxyOf(const TYP_t&) const;

	expr::GType* PtrOf(Out_t* p) const { return PtrOf(TypOf(p)); }
	expr::GType* DerefOf(Out_t* p) const { return DerefOf(TypOf(p)); }
	expr::GType* ProxyOf(Out_t* p) const { return ProxyOf(TypOf(p)); }

	expr::GType *fromGlob(CGlobPtr) const;
	expr::GType *fromTypeObj(CTypePtr) const;
	expr::GType *fromField(CFieldPtr, bool skipModifier = false) const;
	expr::GType *fromOp(CHOP) const;
	expr::GType *fromOpType(uint8_t) const;
	expr::GType *fromCall(const Out_t *) const;

	int ComplyWithFieldType(Out_t *, FieldPtr) const;
	TypePtr toTypeRef(const TYP_t &) const;
	//bool IsNull(const expr::GType *) const;
	TYP_t	Complied0(const TYP_t &, const TYP_t &) const;//returns one of args if not complied(?)
	TYP_t	Complied(const TYP_t &, const TYP_t &) const;//returns nil if not complied(!)
	bool	AreCompliant(const TYP_t &, const TYP_t &) const;
	bool IsImplicitCastOf(const TYP_t& from, const TYP_t& to) const { return from.isImplicitCastOf(to); }

	bool	IsUnfoldMode() const;

	bool	IsRootEx(CHOP) const;
	int		CheckFlagsUsage(CHOP) const;
	int		GetSwitchInfo(Out_t * pSelf, SwitchQuery_t &si) const;
	bool	IsOpTerminal(CHOP) const;
	Out_t *	CheckTypeCast(CHOP, Out_t *) const;
	int		CalcGetOffset(CHOP, CHOP pOpR) const;
	//Out_t*	CheckPostCall(HOP, Out_t *);
	//Out_t*	CheckCombinedStatement(HOP, Out_t *);
	FieldPtr CheckThruConst(const Out_t*, OFF_t &) const;

	int __getDisp2(Out_t *, int &d, bool bDetach) const;
	int ExtractDisplacement(Out_t *, int &d, bool bDetach = false) const;
	int ExtractDisplacement2(Out_t *, int &d, bool bDetach = false) const;
	Out_t *CheckDisplacement(Out_t *) const;
	TypePtr CallRetType(const Out_t*) const;
	std::pair<FieldPtr, GlobPtr> ExtractCallTarget(const Out_t*) const;

protected:
	//virtual void dump(int, Out_t *, ESimplifyOutcome) const {};// { ((EXPRSimpl_t*)this)->meResult = SIMPL_NULL; }
};

////////////////////////////////////
class EXPR_t : public ExprInfoBase_t
{
protected:
	HOP		mpStartOp;
	//IExprDump	*mpIDump;
	std::set<ADDR>	m_addresses;

public:
	EXPR_t(const FuncInfo_t &, HOP, adcui::UDispFlags flags, ExprCache_t &);
	//EXPR_t(const FuncInfo_t &, PathTracer_t &ptr, OpTracer_t &otr, HOP, uint32_t flags, ExprCache_t &);

	EXPR_t &self() const { return const_cast<EXPR_t &>(*this); }
	const std::set<ADDR> &addresses() const { return m_addresses; }

	//uint32_t	testf(uint32_t f, bool f2 = false);
	//void	setDumpHandler(IExprDump *pIDump){ mpIDump = pIDump; }
	bool	CompareConditions(CHPATH, CHPATH) const;


	Out_t *	DumpExpression2(CHOP) const;
	Out_t *	DumpOp(CHOP) const;
	Out_t *	DumpExpression(CHOP, Out_t *mpU, int flags = 0) const;//1:XIns,2:cpu flags only
	Out_t *	DumpGoto(CHOP, Out_t *mpU) const;
	Out_t *	DumpCallUnfold(CHOP, Out_t *mpU) const;
	Out_t *	DumpCall(CHOP, Out_t *mpU) const;
	Out_t *	DumpIntrinsic(CHOP, Out_t *mpU) const;
	Out_t *	DumpRet(CHOP, Out_t *mpU) const;
	Out_t *	DumpThrough0(CHOP, Out_t *pOut) const;
	Out_t *	DumpB(CHOP, Out_t *mpU) const;
	Out_t *	DumpIndirect2(CHOP, Out_t *mpU) const;
	Out_t *	DumpTerminal(CHOP, Out_t *mpU) const;
	Out_t *	DumpFlagsOnly(CHOP, Out_t *mpU) const;
	Out_t *	DumpPreCondition(CHOP, Out_t *mpU) const;
	void 	DumpCond(CHOP, Out_t *mpU) const;
	Out_t *	DumpCondJump(CHOP, Out_t *mpU) const;
	void	DumpIFCond(CHOP pOpLast, Out_t *mpU) const;
	void	DumpCondition2(CHPATH rSelf, Out_t *mpU, bool bInvert) const;
	Out_t * DumpCondition4(CHPATH) const;
	Out_t *	DumpOpsInRow(CHPATH, Out_t *mpU) const;


	////////////////////
	void ResetArrays() const;
	Out_t *AddRoot() const;//root to start from

};


