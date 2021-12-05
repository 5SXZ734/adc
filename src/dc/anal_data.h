#pragma once

#include "xref.h"
#include "type_funcdef.h"
#include "expr.h"

struct RedumpCache_t;

struct TraceInfo_t : private FuncInfo_t
{
	HOP	pOpFrom;
	HOP	pOpBase;
//	Path_t	*pBlock;
	XOpList_t	lXIn;
	TraceInfo_t(const FuncInfo_t &, HOP _pOp, HOP _pOpBase);

	GlobPtr FuncDefPtr() const {
		return FuncInfo_t::FuncDefPtr(); }
};

//data flow tracer
class AnlzXDeps_t : public FuncTracer_t
{
	//OpTracer_t *mpOpTracer;
	bool mbRangedOnly;//scan only ranged storages?
	bool mbLocalsOnly;
	RedumpCache_t *mpRedumpCache;
public:
	AnlzXDeps_t(const FuncTracer_t &, bool b);
	AnlzXDeps_t(const FuncInfo_t &, bool b, PathOpTracer_t &);

	void setRedumpCache(RedumpCache_t *p) { mpRedumpCache = p; }
	
	STAGE_STATUS_e run();
	void setRangedOnly( bool b ){ mbRangedOnly = b; }
	void PathTouchXDeps();
private:
	STAGE_STATUS_e	TouchXDeps();
};

// def-use chains builder
class AnlzXDepsIn_t : public FuncTracer_t
{
	HOP  mpOp;
	bool mbRangedOnly;
	bool mbLocalsOnly;
	ExprCacheEx_t	&mrExprCache;
	static unsigned bInside;
	//PathTracer_t *mpPathTracer;
	//OpTracer_t *mpOpTracer;
	RedumpCache_t *mpRedumpCache;
public:
	AnlzXDepsIn_t(const FuncTracer_t &, HOP pOp, ExprCacheEx_t &);
	AnlzXDepsIn_t(const FuncInfo_t &, PathOpTracer_t &, HOP pOp, ExprCacheEx_t &);
	//void setPathTracer(PathTracer_t *p){ /*mpPathTracer = p;*/ }
	//PathTracer_t &pathTracer() const { assert(mpPathTracer); return *mpPathTracer; }
	//void setOpTracer(OpTracer_t *p) { /*mpOpTracer = p;*/ }
	//OpTracer_t &opTracer() const { return *mpOpTracer; }
	void setRedumpCache(RedumpCache_t *p) { mpRedumpCache = p; }

	void	TraceXDepsIn();//initiates xdeps scan starting from itself
	int		ScanXDepsOut();
	int		ScanXDepsBase(bool bMakeLocal);
	void	setRangedOnly( bool b ){ mbRangedOnly = b; }
	void	setLocalsOnly( bool b ){ mbLocalsOnly = b; }
//	int		TouchXDeps();
	//int		OrderXOuts(HOP);
	int		TraceXDepsOut();
	int		ScanXDeps0(CHOP hOpFrom, CHOP hOpBase = HOP(), bool bRelOnly = false);
	int		ScanXDeps1(CHOP hFrom, CHOP hOpBase = HOP());
	int		MatchXDep(CHOP hOp, uint32_t &mask);
	int		CheckExtraXOut();
	int		__tracePtrToLocal(HOP pOpLoc);
	int		CheckIfArgsOverlapLocal(HOP, HOP pOpLoc);
	void	CheckIrreducibleExpressions();
	static bool IsInside() { return bInside > 0; }
private:
	int		MatchXDepIn(CHOP pOp, TraceInfo_t &ti, uint32_t &mask);
	int		MatchXDepOut(HOP pOp, uint32_t &mask);
	int		MatchXDepOut2(HOP pOp, uint32_t mask);
	void	__traceXDepsUp(TraceInfo_t &ti, uint32_t mask, CHPATH pPath);
	int		TraceXOutExtra();
	void	__traceXOutExtra(CHPATH pPath, uint32_t &mask);
	int		MatchXDepExtra(HOP pOp, uint32_t mask);
	int		EstablishLink(TraceInfo_t &ti, HOP pOpIn);
	HOP		RegisterCallOutOp(CHOP, CHOP pOp);
	int		ExpandSpecialAND(HOP);
	int		ExpandSpecialOR(HOP);
};


