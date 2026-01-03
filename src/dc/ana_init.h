#pragma once

#include "info_func.h"

///////////////////////////////////////////////////InitialTracer_t
class InitialTracer_t : public FuncTracer_t
{
	//FieldMap *mpCachedArgs;
public:
	InitialTracer_t(const FuncTracer_t &);
	InitialTracer_t(const FuncInfo_t &, PathOpTracer_t &);

	//void setCahchedArgs(FieldMap *p){ mpCachedArgs = p; }
	//void setOpTracer(OpTracer_t *p) { /*assert(!mpOpTracer);  mpOpTracer = p;*/ }
	//OpTracer_t &opTracer() const { return *mpOpTracer;  }
	int Analize(HOP, CHOP pOpPr) const;
	int Preanalize(HOP) const;
	int Goto2Call(HOP)  const;
	int SplitAssignment(HOP) const;
	void CheckCallArg(HOP) const;
	int AdjustCall(HOP, CGlobPtr, int extra_args) const;
	int AdjustCall(HOP, CTypePtr typefunc, int extra_args) const;
	int AdjustCall(HOP, FuncProfile_t &, int extra_args) const;
	int	AdjustCallEx(HOP, CGlobPtr, int) const;
	//int	ChangeArgsExNumber(HOP, int nArgsNum);
	int	ToggleSplit(HOP) const;
	int		FixDisplacements(HOP) const;
	enum CheckCallResult { CALL_ERROR = -1, CALL_NULL = 0, CALL_THUNK, CALL_DIRECT, CALL_INDIRECT, CALL_INTRINSIC };
	CheckCallResult	CheckCall(HOP) const;
	//int		CheckIndirGoto(HOP, ADDR);
	//int		CheckCallPtr(HOP);
	int		CheckSpecial(HOP) const;
	int		CheckShift(HOP) const;
	int		CheckReadOnly(HOP) const;
	int		CheckReadOnly(FieldPtr) const;
	int Analize0(HPATH pPath, HOP pOpPr) const;
	int AnalizeJumpTable(HPATH pPath, HOP pOpPr) const;
	STAGE_STATUS_e		CheckSpoiltRegs() const;
	HOP	CreateStackArg(HOP, int nDisp) const;
	HOP	CreateCallArg(HOP, SSID_t ssid, int off, uint8_t siz) const;
	//int		ToggleHidden(HOP);
	//int		MergeAssignment(HOP);
	int		__traceUsage(CHOP, CHOP pOp, bool bDir) const;//0-up,1-down
	bool		CheckFunc(FieldPtr) const;
	HOP TakeCallArg(OpList_t&, SSID_t ssid, int off) const;
	bool IsEqualed1(HOP, SSID_t ssid, int off) const;
	bool IsEqualed2(HOP, HOP) const;
	HOP FindCallArg2(OpList_t&, HOP) const;
	//int		SetString(FieldPtr , bool bSet, bool bUser = false);
	//int		SetPenetrated(FieldPtr , bool bSet);
	//bool 	IsJumpTable(FieldPtr );
	//int		SetConst(FieldPtr , bool bSet) const;
	int		UpdateCallers() const;
	int		SetFPUOut0(int) const;
	FieldPtr GetCalleeFieldEx(CHOP) const;
};



