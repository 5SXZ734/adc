#pragma once

#include "db/mem.h"
#include "info_func.h"
#include "xref.h"

class PathTreeEx_t;
class Field_t;
//class Op_t;
class MemoryMgr_t;

class SwitchTracer_t : public FuncTracer_t
{
	int	meErrorCode;
public:
	enum ErrorCodes {
		ERR_NONE,
		ERR_CONTEXT,
		ERR_PATTERN,
		ERR_UNKNOWN
	};
public:
	SwitchTracer_t(const FuncTracer_t &);
	SwitchTracer_t(const FuncInfo_t &, PathOpTracer_t &);
	
	int GotError(ErrorCodes e, int retCode = 0) {//setting
		meErrorCode = e;
		return retCode;
	}
	bool GotError() const {
		return meErrorCode != ERR_NONE;
	}
	MyString ErrorAsString() const;

	// switch
	int ToggleSwitch(HPATH, bool bVerbose);
	int TurnSwitch_On(HPATH);
	int TurnSwitch_Off(HPATH);
	int ExpandSwitch(HPATH);
	int CollapseSwitch(HPATH);
	HOP addIOp(HPATH, HOP pOp);
	HPATH AssureSwitchTablePath(HPATH pSwitchTablePath);//, FieldPtr pSwitchTable);
	HPATH AssureIndexTablePath(HPATH pIndexTablePath);//, FieldPtr pIndexTable);
	
	static HOP FindIOp(HPATH, ADDR);
private:
	int RecoverDefaultBranch(HPATH);
	int BlockOutCases(HPATH, HPATH);
	int	CheckBetterLocation(HPATH, HPATH pBlockSwitch = HPATH());
	int	RelocateBadCases(HPATH);
	bool CheckDefaultPath(HPATH);
	int CheckSwitchValid(HPATH);
	int CreateSwitchIndexTable(FieldPtr, HPATH pJumpTablePath);//replace array of bytes with list of ops
	int ChangeGotoDestination(HPATH, HOP pOpTo, bool bAfter = false);
	HPATH GetSwitchBlock(HOP);
	int	GetSwitchInfo2View(HPATH, SwitchQuery_t &si) const;
	//int		CheckCase(FieldPtr, FieldPtr *ppSwitchTab = 0);
	int		CheckCase2(HPATH, HPATH pJumpTablePath);
};


