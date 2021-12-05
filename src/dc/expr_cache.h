#pragma once

#include <set>
#include "shared/misc.h"
#include "db/mem.h"
#include "db/types.h"
#include "info_func.h"
#include "expr_type.h"
#include "expr_term.h"
#include "arglist.h"

enum ProblemEnum
{
	PROBLEM_PSTACK_NRESET,
	PROBLEM_PSTACK_NCONV,
	PROBLEM_PSTACK_RET_MISMATCH,
	PROBLEM_PSTACK_BAD,
	PROBLEM_FSTACK_NCONV,
	PROBLEM_INTERCODE_EXPOSED,
	PROBLEM_DIRECT_ADDRRESSING,
	PROBLEM_CONST_CONDITION,
	PROBLEM_INDIRECT_GOTO,
	PROBLEM_BAD_CODE,
	PROBLEM_TOTAL//32 MAX!
};

typedef std::map<CFieldPtr, Arg2_t>	NoSSIDArgsMap;
typedef NoSSIDArgsMap::iterator							NoSSIDArgsMapIt;
typedef NoSSIDArgsMap::const_iterator					NoSSIDArgsMapCIt;


/////////////////////////////////// ExprCache_t
class ExprCache_t : public My::IUnk //shared among multiple views (memory reduction sake) ???
{
public:
	expr::CTypeMgr mTypesMgr;
	TPoolWrapper<Out_t> mOuts;
	const Out_t	*mpSelOut;
	VAList mSelSet;
	NoSSIDArgsMap	mNoSSIDArgs;//unassigned storage args mapping (->offset)
public:
	ExprCache_t(OpType_t ptrSize)
		: mTypesMgr(ptrSize),
		mpSelOut(nullptr)
	{
	}
	void resetz();
	void setSelOut(const Out_t *);
	void updateSelSet(FuncInfo_t &, ADDR);
	const Arg2_t *findNoSSIDArg(const DcInfo_t &, CGlobPtr, CFieldPtr);
	Arg2_t fieldToArg(const FuncInfo_t &, CFieldPtr);
};

///////////////////////////////////////// ExprCacheEx_t (adds ops->expr mapping)
class ExprCacheEx_t : public ExprCache_t
{
	struct Elt
	{
		GlobPtr iCFunc;	//owner func
		Out_t *pOut;	//expression tree
		ProblemInfo_t	problem;		//associated problem(s)
		std::set<ADDR>	addresses;		//addresses which made a contribution to the expression
		//Elt() : iFunc(nullptr), pOut(nullptr), problem(0){}
		Elt(GlobPtr _iCFunc, Out_t *_pOut, ProblemInfo_t _problem, const std::set<ADDR> &_addresses)
			: iCFunc(_iCFunc), pOut(_pOut), problem(_problem), addresses(_addresses)
		{
			assert(!iCFunc || iCFunc->typeFuncDef());
		}
	};
	typedef std::map<HOP, Elt>	OpCacheMap;
	typedef OpCacheMap::iterator OpCacheMapIt;
	typedef OpCacheMap::const_iterator OpCacheMapCIt;
	OpCacheMap	mOpCache;
public:
	ExprCacheEx_t(OpType_t ptrSize)
		: ExprCache_t(ptrSize)
	{
	}
	void reset();
	Out_t *findOpExpr(HOP) const;
	OpCacheMapIt addOpExpr(HOP, GlobPtr, Out_t *, const std::set<ADDR> &);
	void addProblem(HOP, ProblemInfo_t);
	ProblemInfo_t findProblem(HOP) const;
	//MyString findProblemStr(HOP, bool bExpanded) const;
	int qualifyAddress(CHOP, ADDR) const;//0:no contribution
};


