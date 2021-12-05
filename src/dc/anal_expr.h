#pragma once

#include "info_func.h"

//expression recreation from intercode
class AnlzRoots_t : public FuncTracer_t
{
	HOP 	mpOp;
	bool	mbAuto;
	bool	mbUser;
	bool	mbPreserveVars;//for uprooting only
	bool	mbForce;
	std::set<HOP> arrRootsSrc;
public:
	AnlzRoots_t(const FuncTracer_t &r, HOP pOp, bool bAuto = false, bool bUser = false);
	AnlzRoots_t(const FuncInfo_t &, PathOpTracer_t &ptr, HOP pOp, bool bAuto = false, bool bUser = false);

	void setPreserveVars(bool b){ mbPreserveVars = b; }
	void enforce(){ mbForce = true; }

	int		TurnRoot_Off();
	int		TurnRoot_On();
	int		ToggleRoot();//if autoprocessing -> bAuto == true;

private:
	bool	CanUnroot1();
	bool	CanUnroot2();
	int		CanUnrootSpecial();
	int		OrderXIns(HOP, bool b);
	int		CheckOrderEx(HOP, HOP);	
				//-2:	failed to determine
				//-1:	this is higher than pOp (No < pOp->No)
				//0:	this == pOp
				//1:	this is lower than pOp (No > pOp->No)
	int turnRoot_On();
	bool	UnrootXIns(CHOP);
	bool	PrepareSrcOps(CHOP);
	void	RestoreSrcOps();
};

//Op_t::
	//int		__traceRootAssureDown(HPATH pPathUp, HPATH pPath);
	//int		__traceRootAssureUp(HPATH pPath, HOP pOpEnd, HOP pOp0);
	//void	CheckRoots(HOP pOpEnd, HOP pOp0);
	//void	__checkRoots(HOP pOp0);

