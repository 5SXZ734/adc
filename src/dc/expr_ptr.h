#pragma once

#include "expr_simpl.h"
#include "config.h"

class EXPRptr_t : public EXPR_t
{
	//PathTracer_t &mrPathTracer;
	//OpTracer_t &mrOpTracer;
	bool mbTraceXIns;
	//FieldMap *mpCachedArgs;
public:
	EXPRptr_t(const FuncInfo_t &, HOP, uint32_t flags, ExprCacheEx_t &);
	//void setOpTracer(OpTracer_t *p) { /*mpOpTracer = p;*/ }
//	PathTracer_t &pathTracer() const { return mrPathTracer; }
//	OpTracer_t &opTracer() const { return mrOpTracer; }
	//void setCahchedArgs(FieldMap *p){ mpCachedArgs = p; }

	void enableTraceXIns(bool b){ mbTraceXIns = b; }
	//int		TracePtr3(HOP, PathOpTracer_t &);
	Out_t *	TracePtr(HOP, PathOpTracer_t &, bool bResetArrays = true);
	//Out_t *traceLocalsType(HOP, PathOpTracer_t &);
	//Out_t *tracePtrToGlobal(HOP, PathOpTracer_t &);
	void Simplify(Out_t*, HOP, PathOpTracer_t&);

private:
	Out_t* __outPtr(HOP, Out_t* mpU, PathOpTracer_t&);
};


class EXPRPtrSimpl_t : public EXPRSimpl_t
{
public:
	EXPRPtrSimpl_t(const EXPRptr_t& r)
		: EXPRSimpl_t(r)
	{
	}
	EXPRPtrSimpl_t(const FuncInfo_t& r, adcui::UDispFlags flags, ExprCache_t& ec)
		: EXPRSimpl_t(r, flags, ec)
	{
	}
	void	SimplifyPtr0(Out_t *, PathOpTracer_t &);
	int SimplifyCommaUp(Out_t*);
	int SimplifyRoot(Out_t*);
	int DispatchSimplify_PtrFields(Out_t *, PathOpTracer_t &);
	int DispatchSimplify_Ptr(Out_t *);
	int SimplifyPtr3(Out_t *);
	int SimplifyIndir5(Out_t *);
	int SimplifyIndir6(Out_t *);
	int SimplifyAssignPtr(Out_t *);
	int SimplifyOp2Field(Out_t *, PathOpTracer_t &) const;
	int SimplifyOp4Ptr(Out_t *) const;

	void SetFieldTyp(Out_t* p, const TYP_t& t) const;
};



