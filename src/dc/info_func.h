#pragma once

#include "config.h"
#include "info_file.h"
#include "info_proto.h"

class FuncCleaner_t;
class FuncTracer_t;
enum class STAGE_STATUS_e;
class FuncArgsMap;
class ExprCacheEx_t;

class PathTracer_t
#if(NEW_PATH_TRACER)
	: public MemPathPool::TSurrogate<uint8_t>
#endif
{
//?	static int s_here;
public:
	PathTracer_t(){
//?		assert(s_here == 0);//not re-entrant
//?		s_here++;
	}
	~PathTracer_t(){
//?		s_here--;
	}
#if(!NEW_PATH_TRACER)
	void reset(FuncInfo_t &);
	path_trace_cell_t &cell(HPATH hPath){
		return hPath->m_traceInfo;
	}
#else
	void reset(FuncInfo_t &rFI){
		clear();
	}
	path_trace_cell_t &cell(PathPtr hPath){
		return (path_trace_cell_t &)MemPathPool::TSurrogate<uint8_t>::cell(hPath);
	}
#endif
};


class OpTracer_t
#if(NEW_OP_TRACER)
	: public MemOpPool::TSurrogate<uint16_t>
#endif
{
//?	static int s_here;
public:
	OpTracer_t(){
//?		assert(!s_here);
//?		s_here++;
	}
	~OpTracer_t(){
//?		s_here--;
	}
#if(!NEW_PATH_TRACER)
	void reset(const FuncInfo_t &);
	op_tracer_cell_t &cell(OpPtr hOp){
		return hOp->mtr;
	}
private:
	void __set(const FuncInfo_t &, OpPtr);
#else
	void reset(const FuncInfo_t &){
		clear();
	}
	op_tracer_cell_t &cell(OpPtr hOp){
		return (op_tracer_cell_t &)MemOpPool::TSurrogate<uint16_t>::cell(hOp);
	}
#endif
};



class PathOpTracer_t : public PathTracer_t
{
	OpTracer_t mOpTracer;
public:
	PathOpTracer_t(){}
	OpTracer_t &opTracer(){ return mOpTracer; }
};

//////////////////////////////////////////////////DcDestroyer_t
/*class DcDestroyer_t : public FileInfo_t
{
public:
	DcDestroyer_t(FileInfo_t &a)
		: FileInfo_t(a)
	{
	}
	DcDestroyer_t(TypeObj_t &rDcRef, MemoryMgr_t &b)
		: FileInfo_t(rDcRef, b)
	{
	}

	virtual void destroy(FieldRef) const;
	virtual void destroy(TypeObj_t &) const;

	void destroy() const;
	void destroy(FuncDef_t &) const;

protected:
	void disconnect(FieldPtr , MemoryMgrEx_t &) const;
};
*/



// ARGS/REGS
// |      OFFS:16      | SSID:8  | ORDER:7 |	BIT[31]=1 => means VAR (negative)
// | 00 | 04 | 08 | 12 | 16 | 20 | 24 | 28 |

#define LOCAL_SSID_MASK			0xF
#define LOCAL_SSID_SHIFT		16
#define LOCAL_ORDER_MASK		0x7F
#define LOCAL_ORDER_SHIFT		24
#define LOCAL_ORDER_MAX			LOCAL_ORDER_MASK

#define	LOCAL_ORDER_THIS		1			//always the first (hidden) one
#define	LOCAL_ORDER_REG			2			//for regs
#define	LOCAL_ORDER_STACK		0x3F		//ordering for stack args, regs always go before
#define	LOCAL_ORDER_UNKNOWN		0x70		//for the args with unassigned storage other than 'this'
#define	LOCAL_ORDER_ARG_UPPER	0x70		//upper limit for arguments
#define	LOCAL_ORDER_SPOILT		0x71		//for spoilt registers (not considered retvals)
#define	LOCAL_ORDER_RET_LOWER	0x72		//lowe limit for retvals
#define	LOCAL_ORDER_RETVAL		0x7F		//ordering for return val(s) - tailed

#define	SET_ARG_SSID(arg)		(((arg) & LOCAL_SSID_MASK) << LOCAL_SSID_SHIFT)
#define	ARG_SSID_FROM_KEY(arg)	SSID_t(((arg) >> LOCAL_SSID_SHIFT) & LOCAL_SSID_MASK)
#define	SET_ARG_ORDER(arg)		(((arg) & LOCAL_ORDER_MASK) << LOCAL_ORDER_SHIFT)
#define	ARG_ORDER_FROM_KEY(arg)	(((arg) >> LOCAL_ORDER_SHIFT) & LOCAL_ORDER_MASK)

#define LOCAL_VAR_FLAG			0x80000000	//or just the negative value
#define LOCAL_ARG_RANGE			0x0000FFFF	//offset range mask for local arguments
#define LOCAL_REG_RANGE			0x0000FFFF	//offset range for register variables (maybe cast to a short)

#define	ARG_OFFS_FROM_KEY(arg)	int((arg) & LOCAL_ARG_RANGE)

#define	LOCAL_MAKE_KEY(order,ssid,off)		((ADDR)SET_ARG_ORDER(order)) | ((ADDR)SET_ARG_SSID(ssid)) | ((ADDR)(off & LOCAL_ARG_RANGE))




class FuncInfo_s : public FileInfo_s
{
public:
	//static FieldPtr	DockField(CGlobPtr);

	static unsigned order(CFieldPtr p) {
		return ARG_ORDER_FROM_KEY(p->_key());
	}

	static int localOffset(CFieldPtr p)//this is the local for sure
	{
		if (isLocalVar0(p))
			return (int)p->_key();
		return (p->_key() & LOCAL_ARG_RANGE);
	}

	static bool isLocalVar0(CFieldPtr p) {//stacked var, not reh
		return (p->_key() & LOCAL_VAR_FLAG) != 0;
	}//int32_t(_key()) < 0
	static bool isLocalVar(CFieldPtr p) {//not arg
		return IsLocal(p) && isLocalVar0(p);
	}
	static bool isLocalArg(CFieldPtr);//can be a ret/spoilt

	r_t toR(CFieldPtr p) const {
		return r_t(uint8_t(SSIDx(p)), uint8_t(address(p)), uint8_t(p->OpTypeZ()));
	}
	r_t toR2(CFieldPtr p) const {//no optyp
		return r_t(uint8_t(SSIDx(p)), uint8_t(p->offset()), uint8_t(p->OpTypeZ() & OPSZ_MASK));
	}
	ri_t toRi2(CFieldPtr p) const {//no optyp
		return ri_t(uint8_t(SSIDx(p)), uint8_t(p->offset()), uint8_t(p->OpTypeZ() & OPSZ_MASK));
	}

	static ADDR loc2key(int ssid, int offs)
	{
		assert(ssid);//locals only

		ADDR uKey = offs;
		if (ssid != SSID_LOCAL)
		{
			assert(abs(offs) <= LOCAL_REG_RANGE);//is it correct?
			//assert(offs >= 0 && offs <= LOCAL_REG_RANGE);
			uKey &= LOCAL_REG_RANGE;
			uKey |= SET_ARG_SSID(ssid);
			uKey |= SET_ARG_ORDER(LOCAL_ORDER_REG);//1?
		}
		else if ((int)offs >= 0)
		{
			assert(offs <= LOCAL_ARG_RANGE);
			//uKey |= SET_ARG_SSID(ssid);
			uKey |= SET_ARG_ORDER(LOCAL_ORDER_STACK);//?LOCAL_ORDER_MAX);
		}
		return uKey;
	}

	ADDR loc2key(CFieldPtr p) const
	{
		return loc2key(SSIDx(p), address(p));
	}

	static ADDR setupKey(uint8_t ssid, int offs, int order = -1);

	static ri_t toR_t(CFieldPtr);

	/*void key2loc(ADDR uKey)
	{
		uint8_t opc;
		int32_t offs;
		if ((int)uKey > 0)//local arg or reg
		{
			opc = ARG_SSID_FROM_KEY(uKey);
			if (uKey & LOCAL_ARG_FLAG)
			{
				assert(opc == SSID_LOCAL);
				offs = (uKey & LOCAL_ARG_RANGE);
			}
			else
			{
				offs = int32_t(*(short *)&uKey);
			}
		}
		else
		{
			opc = SSID_LOCAL;
			offs = (int)uKey;
		}
		assert(SSID() == opc);
		assert(Offset() == offs);
	}*/


	static ADDR address(CFieldPtr);
	static ADDR address(ADDR _key);
	static SSID_t SSIDx(ADDR _key);
	static SSID_t SSIDx(CFieldPtr p) { return SSIDx(p->_key()); }
	static bool	IsStackLocal(CFieldPtr);//any stack args/vars, NOT registers
	static bool	IsLocalReg(CFieldPtr);//ONLY registers (args or vars)
	static bool	IsLocal0(CFieldPtr);//any locals (with SSID set)
	static bool IsAnyLocalVar(CFieldPtr);
	static bool IsLocalVar(CFieldPtr);
	static bool IsLocalArg(CFieldPtr);//excluding spoilts and rets
	static bool IsLocal(CFieldPtr);
	static bool IsLabel(CFieldPtr);
	static PathPtr LabelPath(CFieldPtr);
	static HOP 	GetVarOp(CFieldPtr);
	static void DestroyLocalUserData(FieldPtr);
#if(NO_EXTRA_USER)
	static XOpList_t& LocalRefs(CFieldPtr);
	//static const XOpList_t& LocalRefs(CFieldPtr);
	static HOP FindDanglingOp(CFieldPtr);
	static void PushLocalRefBack(FieldPtr, XOpLink_t*);
	static XOpLink_t* TakeLocalRefFront(FieldPtr);
#else
	static XOpList_t& xrefs(FieldPtr p) {
		return *(XOpList_t*)& p->m_user2;
	}
	static const XOpList_t& xrefs(CFieldPtr p) {
		return *(const XOpList_t*)& p->m_user2;
	}
#endif
	static bool AreEqual(CFieldPtr, CHOP);
	static bool AreEqual(CFieldPtr, CFieldPtr);
	static bool IsOverlap(CFieldPtr, SSID_t ssid2, int nOffs2, int nSize);
	////////////////////////////////////////// ops
	static OpPtr PrimeOp(CHOP h) {
		return h->ins().mpPrimeOp; }
		//return h->PrimeOp(); }
	static bool	IsPrimeOp(CHOP h) {
		return (PrimeOp(h) == h); }
	static PathPtr  PathOf(CHOP pSelf){
		return InsRefPrime(pSelf).mpPath; }
	static OpPtr PrevOp(CHOP p){
		return PathOf(p)->ops().Prev(p);
	}
	static OpPtr NextOp(CHOP p){
		return PathOf(p)->ops().Next(p);
	}
	static Ins_t &InsRef(CHOP h){ return h->ins(); }
	static Ins_t *InsPtr(CHOP h){ return h->insPtr(); }
	static Ins_t &InsRefPrime(CHOP h){ return *h->m_pRI; }//PrimeOp(pSelf)->ins()//to indicate h is not a prime
	static Ins_t *InsPtrPrime(CHOP h){ return h->m_pRI; }

	static void SetPathRef(HOP, PathPtr);
	//static void SetLabelRef(HOP pOp, FieldPtr pLabel);
	static void	SetLocalRef(HOP, FieldPtr);
	static FieldPtr LocalRef(CHOP);
	//static FieldPtr LabelRef(CHOP);//PathRef
	static HPATH	PathRef(CHOP);
	static bool		IsJointOp(HOP);
	static int StackTop(CHOP);
	static int StackOut(CHOP);
	static Action_t	ActionOf(CHOP);
	static OpPtr NextPrime(CHOP, bool bSkipVarOps = false);
	static OpPtr PrevPrime(CHOP);
	static OpPtr	NextPrimeEx(CHOP, bool bSkipVarOps = false);
	static OpPtr	PrevPrimeEx(CHOP);
	static ADDR		OpVA(CHOP);
	static ADDR		OpVA(CHOP, int &idx);//index for duplicates
	static ADDR		PathVA(CHPATH);
	static MyString StrNo(CHPATH);
	static	int8_t FpuIn(CHOP); 
	static	bool IsCode(CHOP);
	static bool	IsCall(CHOP);
	static bool	IsCallIntrinsic(CHOP);
	static bool	IsGoto(CHOP);
	static HOP	PrevAuxOp(CHOP);
	static HOP	NextAuxOp(CHOP);
	static	bool IsArgOp(CHOP);
	static	bool	IsVarOp(CHOP);
	static uint32_t	Overlap(const REG_t &, const REG_t &);
	static const PathOpList_t& PathOps(HPATH);
	static const XOpList_t& PathOpRefs(HPATH);
	static OpPtr	GetFirstOp(CHPATH);
	static OpPtr	GetLastOp(CHPATH);
	static BlockTyp_t PathType(CHPATH);
	static void SetPathType(HPATH, BlockTyp_t);
	//static ADDR PathVA(HPATH);
	static bool PathIsEmpty(CHPATH);
	static const XOpList_t &GetExitPoints(const FuncDef_t &);
};

class FuncInfo_t : public FileInfo_t, public FuncInfo_s
{
protected:
	GlobObj_t	&mrFuncDefRef;
	FuncDef_t	&mrFuncDef;
	PathTreeEx_t	&mrPathTree;
	TypeObj_t		&mrOwnerSeg;
public:
	FuncInfo_t(const FuncInfo_t &);
	FuncInfo_t(const DcInfo_t&, const GlobObj_t&);
	FuncInfo_t(const FileInfo_t &, CGlobPtr);
	~FuncInfo_t();

	FieldPtr	DockField() const;
	ADDR		DockAddr() const;
	TypePtr		OwnerProc() const;//proc
	TypePtr		OwnerSeg() const;
	MyString	OwnerFuncName() const;
	TypePtr		LocalVars() const;
	TypePtr		LocalArgs() const;
	bool		HasArgs() const;
	bool		IsFar() const;

	GlobPtr	FuncDefPtr() const { return &mrFuncDefRef; }
	GlobObj_t &FuncDefRef() const { return mrFuncDefRef; }
	FuncDef_t &FuncDef() const { return mrFuncDef; }
	bool HasLocalVars() const;
	FieldMap &LocalVarsMap();
	const FieldMap &LocalVarsMap() const;

	HPATH createBodyPath() const;
	HPATH CreateHeadPath() const;
	HPATH CreateTailPath() const;
	PathPtr PathHead() const {
		return mrPathTree.pathHead();
	}
	PathPtr PathTail() const {
		return mrPathTree.pathTail();
	}
	//TypePtr GetLocalsTop() const;
	bool OwnsMemMgr() const;

	unsigned FuncStatus() const { return ProtoInfo_t::FuncStatus(FuncDefPtr()); }
	bool SetFuncStatus(unsigned status) const { return ProtoInfo_t::SetFuncStatus(FuncDefPtr(), status); }
	void SetFuncStatusFinished() const;
	bool IsFuncStatusAborted() const { return ProtoInfo_t::IsFuncStatusAborted(FuncDefPtr()); }
	bool IsFuncDecompiled() const { return ProtoInfo_t::IsFuncDecompiled(FuncDefPtr()); }//was ever?
	bool IsFuncUnfold() const { return FuncStatus() > FDEF_DC_FINISHED_OK && FuncStatus() != FDEF_DC_PHASE2; }
	bool IsStub() const { return ProtoInfo_t::IsStub(FuncDefPtr()); }

	bool		IsThisCallType() const;
	FieldPtr	ThisPtrArg() const;

	TypePtr OwnerScope() const;
	bool	IsClassMember() const;
	bool	testChangeInfo(uint16_t) const;
	uint16_t	changeInfo() const;
	void	setChangeInfo(uint16_t) const;
	void	clearChangeInfo(uint16_t) const;
	void	SetChangeInfo(uint16_t) const;//func
	void	SetChangeFlags(uint16_t) const;//funcdef
	//int		SetSavedRegs(uint32_t nSavedRegs);
	STAGE_STATUS_e		UpdateRetOps() const;
	//int		AdjustStackArgs();
	STAGE_STATUS_e		AssureArgList() const;
	STAGE_STATUS_e		AnalizeIrreducibleExpressions() const;
	STAGE_STATUS_e		CheckCallOuts() const;
	STAGE_STATUS_e		TouchRoots(bool);
	STAGE_STATUS_e		TouchIfs() const;
	STAGE_STATUS_e		TouchWhiles() const;
	STAGE_STATUS_e		TouchElses() const;
	STAGE_STATUS_e		TouchLogics() const;
	STAGE_STATUS_e		TouchSwitches() const;
	//STAGE_STATUS_e		ReAnalize() const;
	STAGE_STATUS_e		ReCheckRoots() const;
	STAGE_STATUS_e		ReCheckBlocks() const;
	bool Redecompile(bool fromStub) const;
	
	///////////////////////////////////////// fields
	int		AddLocalRef(FieldPtr, HOP) const;
	int		ReleaseLocalRef(FieldPtr, HOP) const;


	void LinkOpAfter(HOP, HOP) const;
	void LinkOpBefore(HOP, HOP) const;
	FieldPtr fieldRefEx(CHOP) const;
	FieldPtr FindFieldRef(CHOP) const;
	FieldPtr 	RegisterExitField(HOP pOpRef, HOP pOpCall);
	int32_t	OpDisp(CHOP) const;
	HPATH	GetJumpTargetPath(CHOP) const;
	int		ConvertOPC(HOP) const;
	int		RevertFPUREG(HOP) const;
	int		RevertLocal(HOP, FieldPtr *ppLoc) const;
	int		RevertLocals(HOP) const;
	HPATH 	GetOwnerBody(HOP) const;
	//FieldPtr 	GetOwnerData(HOP) const;
	BlockTyp_t	CheckPathBreak(CHOP) const;
	FieldPtr GetDataSwitch(CHOP) const;
	HOP GetSwitchOp(CHOP, HOP *ppOpTblRef = 0) const;
	bool IsCaseOf(CHOP, CHPATH) const;
	bool IsDefaultOf(CHOP, CHPATH) const;
	HPATH CheckForCondition(CHOP) const;
	HPATH CheckForOrWhileCondition(CHOP) const;
	int CanBeBound(CHOP, CHOP) const;
	FieldPtr 	GetAttachedLocalFromOp(CHOP) const;//do not trace
	FieldPtr GetCalleeDockField(CHOP) const;
	GlobPtr GetCalleeFuncDef(CHOP) const;//direct
	GlobPtr GetCalleeFuncDefEx(CHOP, ExprCacheEx_t&, PathOpTracer_t&) const;//indirect
	GlobPtr	GetFuncAtCall(CHOP) const;
	int		GetVarArgSize(HOP) const;
	bool	EqualTo(CHOP, CHOP pOp) const;		//simple compare
	HPATH	AttachToPath(HOP, /*TypeProc_t *pOwnerFunc, */HPATH pPath = HPATH()) const;
	int		InsertEntryOp(HOP pOp0, HOP pOpPr) const;
	HOP 	RegisterEntryOp(HOP pOpRef) const;

	bool	IsLastExx(CHOP pSelf) const { 
			return NextPrimeEx(pSelf) == HOP(); }
	bool	IsFirstExx(CHOP pSelf) const { 
			return PrevPrimeEx(pSelf) == HOP(); }

	bool	CheckLValueVisibility(CHOP) const;
	int		CheckStackBreak(CHOP, CHOP prev) const;
	int		CheckFStackBreak(CHOP, CHOP prev) const;
	int OpNo(CHOP) const;
	void	SetOpVA(HOP, ADDR) const;
	MyString StrNo(CHOP) const;//offset in proc
	int OpOff(CHOP) const;//offset in path
	void	Numberize(HOP, ADDR) const;
	MyString findProblemStr(ProblemInfo_t f, HOP pOp, bool bExpanded) const;
	MyString LineInfoStr(CHOP) const;
	MyString FTopInfoStr(CHOP, bool bArrows) const;
	MyString StackTopInfoStr(CHOP, bool bArrows) const;
	MyString PathInfoStr(CHOP) const;
	HOP	CheckIndir(CHOP) const;
	HOP	GetSwitchOp2(CHOP) const;
	int	CheckSwitch(CHOP) const;
	HOP	AddOpArg(CHOP, CHOP) const;
	HOP	AddCallArg(HOP, HOP) const;
	void	InvertAction(HOP) const;
	int		CheckTestAction(CHOP) const;
	int		SetRoot(HOP, bool b) const;
	uint32_t	CPUFlags(CHOP) const;
	uint32_t	FPUFlags(CHOP) const;
	int8_t	FpuOut(CHOP) const;
	SSID_t		StorageId(CHOP) const;
	const STORAGE_t &StorageOf(CHOP) const;
	unsigned StorageFlags(CHOP) const;
	bool	IsMineOp(CHOP) const;
	bool	IsUnrefed(CHOP) const;
	bool	IsLocalOp(CHOP) const;
	bool	IsLocalOp2(CHOP) const;
	bool	IsGlobalOp(CHOP) const;
	bool	IsLValue(CHOP) const;
	bool	IsRetOp(CHOP) const;
	bool	IsBadRetOp(CHOP) const;
	bool	IsAnalized(CHOP) const;
	bool	IsThisPtrOp(CHOP) const;
	bool	IsCodeRoot(CHOP) const;
	bool	IsCallDirect(CHOP) const;
	bool	IsCondJump(CHOP) const;
	bool	IsRootEx0(CHOP) const;
	bool	IsOpRootEx(CHOP) const;
	bool	IsDataOp(CHOP) const;
	bool	IsExitOp(CHOP) const;
	bool	IsRhsOp(CHOP) const;
	bool	IsCodeOp(CHOP) const;
	bool	IsCallOutOp(CHOP) const;
	bool	IsCallArg(CHOP) const;
	bool	IsGotoArg(CHOP) const;
	bool	IsOnTop(CHOP) const;
	bool	IsStackPtrOp(CHOP) const;
	bool	IsStackPtrB(CHOP) const;
	bool	IsDeadEx(CHOP) const;
	bool 	IsActual(CHOP) const;//visible & not dead or hidden
	bool	IsOpSavedByCallee(CHOP, CGlobPtr) const;//for CPU regs
	FieldPtr	IsCallToThunkAt(CHOP) const;
	bool	IsAddr(CHOP) const;
	HOP IsBoundToVar(CHOP) const;
	HOP GetOpToBindWith(CHOP) const;
	HOP		GetGotoDestOp(CHOP) const;
	bool	IsMineOp(CHPATH, CHOP) const;
	FieldPtr 	FindOverlappedLocalVar(CHOP, CHPATH, int d = 0) const;
	FieldPtr 	FindOverlappedLocalArg(CHOP, int d = 0) const;
	bool		LocusFromOp(CHOP, Locus_t &) const;
	void	ReleaseXDeps(HOP) const;
	bool	MakeUDLink(HOP use, HOP def) const;//establish use-def link
	bool	UnmakeUDLink(HOP use, HOP def) const;//returns - def
	void	RedirectXIns(HOP, HOP pOpTo) const;
	//void	RedirectXDepIn(HOP, HOP pOpFrom, HOP pOpTo);
	void	ClearXIns(HOP) const;
	bool	ReleaseXDepOut(HOP, OpPtr) const;
	bool	ReleaseXDepIn(HOP, OpPtr) const;
	int		__tracePtrDispl(HOP, int32_t& displ) const;
	int		CalcDispl(HOP) const;
	bool	CheckTopOp(HOP, HOP pOpTop, bool bNoPaths = false) const;

	MyString MsgPStackBadValue(CHOP) const;
	MyString MsgPStackRetMismatch(CHOP) const;
	MyString MsgPStackRetNotReset(CHOP) const;
	MyString MsgStackTopNotConverged(CHOP pSelf, CHOP pOpPr) const;
	MyString MsgFpuTopNotConverged(CHOP pSelf, CHOP pOpPr) const;
	MyString MsgFpuTopNotConverged(CHOP) const;//at exit

	int GetInvolvedOps(HOP pSelf, std::set<HOP> &) const;
	void	SetupPrimeOp(HOP) const;
	HOP		CloneOperation(HOP, bool bOuts) const;
	int		Compare(HOP, HOP) const;		//instruction compare
	int		CheckCallout(HOP) const;
	bool	IsCallArgEx(HOP) const;
	void	CopyTo(HOP, HOP pOp) const;
	void	InitFrom(HOP, HOP pOpFrom) const;
	int		MergeWith(HOP, HOP pOp) const;
	int		Bind(HOP) const;
	int		Unbind(HOP) const;
	int		ToggleInRowStatement(HOP) const;
	int		CheckProblemUnfold(HOP) const;
	bool	CheckNamesClash(CHOP, CFieldPtr) const;
	int		CheckBreak(HOP) const;
	//int		RegisterMemoryFetch(HOP) const;
	HPATH	EnsurePathBreakAt(HOP);
	uint32_t	BitMask(CHOP, bool bBase = false) const;
	int		CheckAlreadyAnalized(CHOP, CHOP pOpPr) const;
	bool	IsCallOpConformantWithArg(CHOP, const Arg2_t &) const;
	REG_t	REG(CHOP) const;
	REG_t	REGex(CHOP pSelf) const;//rid off indir flag
	RegMaskType GetSpoiledRegsMask(CGlobPtr, bool retsOnly) const;

	/////////////////////////////////////// paths
	int		ReleasePathRef(HPATH, HOP) const;
	int		AddPathRef(HPATH, HOP) const;
	HOP		GetSwitchOp(HPATH, HOP *ppOpTblRef) const;
	int CheckGOTOStatus0(CHPATH) const;
	int CheckActualXRefs(CHPATH) const;
	int CheckGOTOStatus(CHPATH) const;
	int CheckElseIf(CHPATH) const;
	FieldPtr GetJumpTable(CHPATH) const;
	HPATH GetJumpTablePath(CHPATH switchPath) const;
	HPATH GetIndexTablePath(CHPATH jumpTablePath) const;
	HPATH BlockOut(HPATH , HPATH);
	HPATH BlockIn(HPATH);
	void __KillRedundantChilds(HPATH);
	int Compare(HPATH, HPATH) const;
	bool IsActual(HPATH) const;
	int CheckBreak(CHPATH) const;
	int CheckWhile(CHPATH) const;
	int CheckDoWhile0(CHPATH) const;
	HPATH GetLoopWhileBackwardCondition(CHPATH) const;
	PathPtr GetGotoPath(CHPATH) const;
	PathPtr GetGotoPathEx(CHPATH) const;
	bool IsPathDegenerate(CHPATH) const;
	bool CheckDegenerateJump(CHPATH) const;
	int		IsRetPath(CHPATH) const;
	int CheckElse(CHPATH) const;
	HPATH AddPath(HPATH pPath = HPATH()) const;
	int CheckIf(CHPATH) const;
	int	CheckContainer(CHPATH) const;
	bool	__checkNamesClash(CHPATH, CFieldPtr) const;
	bool	CheckNamesClash(CHPATH , CFieldPtr pData) const;
	HPATH GetLogicsTop(CHPATH, int) const;//0-normal,1-check first,-1:check last
	int __checkActualOpsCount(CHPATH, int) const;
	int CheckActualOpsCount(CHPATH pSelf, int n) const { return -__checkActualOpsCount(pSelf, n); }
	int		CheckDanglingPath(CHPATH);
	HPATH SplitBefore(HPATH);
	HPATH SplitAfter(HPATH , uint32_t nType);
	int		IsRedundant(CHPATH) const;
	int		CheckOpsCount(CHPATH, int) const;
	int		CheckValid(CHPATH);
	HPATH GetClosingPath(CHPATH, CHPATH) const;
	HPATH __CheckBlock(CHPATH, CHPATH) const;
	//FieldPtr LabelPrev(HPATH);//retrieve first previos label
	void	LinkOpHead(HPATH, HOP) const;
	void	LinkOpTail(HPATH, HOP) const;
	void	LinkOpAfter(HPATH, HOP, HOP) const;
	void	LinkOpBefore(HPATH, HOP, HOP) const;
	HOP		TakeOp(HPATH, HOP) const;
	bool	IsNull(CHPATH hSelf) const { return (!hSelf->Type() && hSelf->HasChildren()); }
	MyString	Name(CHPATH, bool bShort) const;
	int	PathNo(CHPATH) const;
	FieldPtr	PathLabel(CHPATH) const;
	int	GetSwitchInfo(HPATH, SwitchQuery_t &si) const;
	//void	SetPathAnchor(HPATH, ADDR);

	//int Create FuncStub(const MyString &funcName, const char *pBuf);
	
	void InitFrom(Ins_t &, const Ins_t &, HOP pSrcOp) const;
	bool InitiateRedecompile() const;
	bool FromFuncProfile() const;
	
	void GetArgProfileFromCall(ProtoProfile_t &, CHOP) const;
	void GetFuncProfileFromCall(FuncProfile_t &, CHOP) const;

	HOP	FindOpByVA(ADDR) const;
	HOP	FindOpFromRetField(HOP hOpRet, CFieldPtr pField) const;
	HOP FindEntryOp(CHOP pOpRef, HOP* ppOpPrev) const;
	FieldPtr FindArgFromOp(CHOP pOpRef) const;
	FieldPtr FindRetField(CHOP hOpPrime, CHOP hOp) const;
	FieldPtr FindCalleeRetField(CGlobPtr callee, CHOP pOpRef, CHOP pOpCall) const;
	FieldPtr FindCalleeArg(HOP) const;

	//void	dump(FuncDef_t &, std::ostream &);
	void	DumpBlocks(FuncDef_t &, MyStreamBase &);
	int		ChangeArgType(FuncDef_t &, PathOpList_t::Iterator &, uint8_t) const;

	MyString LocalName(CFieldPtr) const;//no labels
	MyString LabelNameExx(CFieldPtr) const;
	bool LocalAutoName(CFieldPtr, MyString &) const;//real locals vars (or args)
	bool LocalVarAutoName(CFieldPtr, MyString &) const;//no rets/spoilt
	MyString RegName(CFieldPtr, bool upperCase = false) const;//must be a register
	int	GetTopOp(bool bOutputUnfold, HOP, std::set<HOP > &) const;
	
	bool LocusFromOpDisp(HOP, Locus_t &) const;
	
	void AddToSpoiltList(CHOP) const;
	
	ProblemInfo_t	CheckProblem(CHOP) const;//gotos
	ProblemInfo_t	CheckProblem(CHOP from, CHOP to) const;//transition from -> to
	HOP InsertVarOpAt(HOP hOpVar, HOP hOpRef);

	void VAListFromOp(VAList&, CHOP);
	void VAListFromOp0(VAList&, CHOP);
	void VAListFromOp(VAList&, const Out_t*, ADDR base);

	///////////////////////////////////// misc
	XOpLink_t *InsertXDep(XOpList_t &, HOP) const;//for XIns - doubled
	bool InsertXDep(XOpList_t &, HOP, XOpLink_t *) const;//for XOuts - shared
	XOpLink_t *TakeXDep(XOpList_t &, OpPtr) const;

	void dump(PathTreeEx_t &, std::ostream &) const;
	void dump(PathTreeEx_t &, MyStreamBase &) const;
	void strxdeps(std::ostream&, const XOpList_t&, bool bExtra, bool bUnfold);

	bool WriteFuncProfileFromCall(HOP, MyStreamBase&);//from anal
};




///////////////////////////////////////////////////FuncTracer_t
class FuncTracer_t : public FuncInfo_t
{
protected:
	PathOpTracer_t &mrPathOpTracer;
public:
	FuncTracer_t(const FuncInfo_t &fi, PathOpTracer_t &tr)
		: FuncInfo_t(fi),
		mrPathOpTracer(tr)
	{
	}
	FuncTracer_t(const FuncTracer_t &ftr)
		: FuncInfo_t(ftr),
		mrPathOpTracer(ftr.pathOpTracer())
	{
	}
	PathTracer_t &pathTracer() const { return mrPathOpTracer; }
	OpTracer_t &opTracer() const { return mrPathOpTracer.opTracer(); }
	PathOpTracer_t &pathOpTracer() const { return mrPathOpTracer; }

	int		SetHidden(HOP) const;
	int		SetShown(HOP) const;
};


class HiddenTracer_t : public FuncTracer_t
{
public:
	HiddenTracer_t(const FuncTracer_t &);
	HiddenTracer_t(const FuncInfo_t &, PathOpTracer_t &);
	STAGE_STATUS_e AnalizeHidden(FuncCleaner_t &);
	void CheckHidden(CHOP, FuncCleaner_t &);
	int CheckHidden0(CHOP);
};

class BlocksTracer_t : public FuncInfo_t
{
	std::list<BlockInfo_t> m_BI;//only refs to terminal paths remain
public:
	BlocksTracer_t(const FuncInfo_t &r)
		: FuncInfo_t(r)
	{
	}
	int UndoPathTree();
	int	RedoPathTree(PathOpTracer_t &ptr);
private:
	int __unbrickCodeTree(PathTree_t::ChildrenIterator &);
};









