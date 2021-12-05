#pragma once

#include "db/mem.h"
#include "info_func.h"

class PathTreeEx_t;
class Field_t;
struct FE_t;
class FieldMap;
struct RedumpCache_t;

//local variables recreation
class LocalsTracer_t : public FuncTracer_t
{
	//FieldMap *mpCachedArgs;
	RedumpCache_t *mpRedumpCache;
public:
	LocalsTracer_t(const FuncTracer_t &);
	LocalsTracer_t(const FuncInfo_t &, PathOpTracer_t &);
	//void setOpTracer(OpTracer_t *p) { /*mpOpTracer = p;*/ }
	//OpTracer_t &opTracer() const { return *mpOpTracer; }
	//void setCahchedArgs(FieldMap *p){ mpCachedArgs = p; }
	void setRedumpCache(RedumpCache_t *p){ mpRedumpCache = p; }

	int MakeLocal(HOP);
	STAGE_STATUS_e MakeLocals();
	int	CheckStackDiff(HOP);
	int	CheckLocal0(HOP, bool bForce = false);
	int	CheckLocals0(HOP);
	void AttachToLocal(HOP);
	STAGE_STATUS_e CheckLocals();
	int	AttachLocalToOp(FieldPtr, HOP);// , int);
	FieldPtr DetachMLoc(HOP);
	//FieldPtr	AddLocal2(HPATH , int opc, int offs);

	//int		__recheckLocals(HPATH );
	int		CheckLocalsPos(HOP);
	//HPATH 	CheckOwnerBlock(HPATH , FieldPtr  pData);
	int			DetachLocals(HPATH );
	HPATH	GetLocalPath(HPATH, FieldPtr) const;//0-from bottom,1-from top
	HPATH	GetLocalImplantPath(CHPATH) const;//0-from bottom,1-from top
	HPATH 	GetLocalsBlock(CHPATH) const;//returns the root block, first terminal path of which may contain the locals
	FieldPtr 	GetAttachData(CHOP) const;//if not attached - trace for it
	//int		CreateArgList0(List_t<Ar g_t> &);//w/h callers updation
	int			TraceTypeEx(FieldRef);
	TypePtr	TraceLocalsType(HOP, FieldPtr);
	TypePtr	TraceLocalsType(HOP);
	void OnLocalReference(FieldPtr, HOP);
	void SetLocalsTypeFromOp(FieldPtr, uint8_t);//optype

	int ImplantLocals();
	bool RelocateVarOp(HOP pOp, HOP pOpRef, HPATH pPath);
	//void insertLocal2(HPATH rSelf, FieldPtr );
	int AddLocalVar(FieldPtr);
	int AddLocalArg(FieldPtr);

	HPATH GetResideBlock(FieldPtr) const;
	bool CheckUsageBoundaryPath(HOP pVarOp, HPATH pPath) const;
	HOP CreateVarOp(FieldPtr);
	FieldPtr AttachLocalVar(HOP pOpRef);
	HOP GetLocalTopOp(FieldPtr) const;

private:
	int	__traceLocal(HOP, int32_t *stacktop);
	//int		__checkLocalsTypes(Path_t &);
};