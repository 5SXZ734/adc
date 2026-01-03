#pragma once

#include "info_func.h"
#include "db/mem.h"

class Field_t;


class MemoryMgr_t;

class BranchTracer_t : public FuncTracer_t
{
	bool mbForce;
public:
	BranchTracer_t(const FuncTracer_t &);
	BranchTracer_t(const FuncInfo_t &, PathOpTracer_t &ptr);

	void enforce(){ mbForce = true; }
	int TurnEndlessLoop_On(HPATH);
	int TurnEndlessLoop_Off(HPATH);
	int ToggleEndlessLoop(HPATH);
	int CheckIfValid(HPATH);
	int TurnBlockIf_On(HPATH);
	int TurnBlockIf_Off(HPATH);
	int ToggleBlockIf(HPATH);
	int CheckElseValid(HPATH);
	int TurnBlockElse_On(HPATH , bool bManual = false);
	int TurnBlockElse_Off(HPATH , bool bManual = false);
	int ToggleBlockElse(HPATH , bool bManual = false);
	int CheckLogicsValid(HPATH);
	int TurnLogics_On(HPATH , HPATH *ppPathNew = 0);
	int TurnLogics_Off(HPATH);
	int ToggleLogics(HPATH);
	int CheckWhileValid(HPATH);

	int TurnWhile_On(HPATH);
	int TurnWhile_Off(HPATH);
	int ToggleWhile(HPATH);

	int TurnForLoop_On(HPATH);
	int TurnForLoop_Off(HPATH);
	int ToggleForLoop(HPATH);

	void CheckChildsValidity(HPATH);
	int FlipIfElse(HPATH);
	int Expand(HPATH);
	int ExpandEx(HPATH , bool);
	int Collapse(HPATH);
	int AdjustGoto(HPATH), AdjustGoto2(HPATH), AdjustGoto3(HPATH);
	HPATH __traceGoto(HOP);
	int		InvalidateGoto(HPATH);
	void RemoveRedundantParent(HPATH);
private:
	bool JumpDir(HPATH , HPATH) const;
	HPATH __rebuildLogics(HPATH);
	int	GetOutPaths(HPATH , HPATH *ppPathNo, HPATH *ppPathYes) const;
	int	__GetOutPaths(HPATH , HPATH *ppPathNo, HPATH *ppPathYes) const;
	static int Equal(HPATH , HPATH);
	int FlipCondJump(HOP);
	//int		DuplicateXIns(HOP pOpRef);
	int		DuplicateXOuts(HOP, HOP pOpRef);
	int GetEnterPathsNum(HPATH);
};



