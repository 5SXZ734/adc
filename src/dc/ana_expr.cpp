#include "ana_expr.h"
#include "prefix.h"

#include "front/front_IA.h"

#include "shared/defs.h"
#include "shared/link.h"
#include "shared/action.h"

#include "db/mem.h"
#include "db/obj.h"
#include "db/field.h"
#include "db/type_struc.h"

#include "path.h"
#include "op.h"
#include "info_dc.h"
#include "ana_ptr.h"
#include "ana_data.h"
#include "ana_local.h"
#include "xref.h"
#include "expr.h"
#include "flow.h"

#define ITARATIVE_ROOT_TRACER

#ifdef _DEBUG
#ifdef WIN32
#define DBGOUT(s)	if (mbUser) OutputDebugString(s.c_str())
#endif
#else
#define DBGOUT(s)
#endif

////////////////////////////////////

class RootsTracer_t : public FuncTracer_t
{
	//PathTracer_t *mpTracer;
public:
	RootsTracer_t(const FuncTracer_t &, HOP  pOp, bool bAuto = false, bool bUser = false);
	RootsTracer_t(const FuncInfo_t &, HOP  pOp, bool bAuto, bool bUser, PathOpTracer_t &);

	void setPathTracer(PathTracer_t *p){ /*mpTracer = p;*/ }
	//PathTracer_t &pathTracer() const { assert(mpTracer); return *mpTracer; }


	int		TryUnroot(HOP  pOp0, HOP  pOpOut);
	int		TryUnroot0(HOP  pOp0, HOP pOpUp, bool bPad);
	int		TryUnroot2();

	friend class UnrootTracer_t;

	enum { REASON_NONE, REASON_INADVISED, REASON_INTERLACED };
	struct Failure_t
	{
		int iReason;//0:
		HOP pAtOp;
		Failure_t() : iReason(0), pAtOp(HOP()){}
		Failure_t(int reason, HOP atOp) : iReason(reason), pAtOp(atOp){}
	};

	const Failure_t &GetFailure() const { return mFailure; }
	void ReportError();

protected:
	void	PrepareBranchInfo(HOP  pOp0);
	int		PrepareRootsDest(HOP  pOp0);
	int		IsSplitRootOffAdvised(HOP  pOp0);
	int		CanUnroot(HOP  pOp0, bool bPad);
	int		CheckXIn(HOP  pOp0, HOP pOpUp);
	int		CheckDependentLeavesOf(HOP  pOp0, HOP pOp);
	int		CheckScatteredAssignment(HOP, HPATH pPath);
	void	InsertOp(HOP);

	int		CheckIntersection(CHOP, CHOP p) const;
	int		CheckIntersectionIndirect(CHOP, CHOP pOp) const;
	void	SetFailure(int iReason, HOP pOp = HOP())
	{
		if (!mFailure.iReason)
		{
			mFailure.iReason = iReason;
			mFailure.pAtOp = pOp;
		}
	}

	STAGE_STATUS_e CheckStatus(CHPATH) const;

protected:
	HOP 	mpOp;
	bool	mbAuto;
	bool	mbUser;

	Failure_t	mFailure;

	std::set<HOP >	mArrRoots;
	std::set<HOP >	mArrRootsDest;
};


class UnrootTracer_t : public FuncInfo_t
{
public:
	RootsTracer_t &m_r;
	PathOpTracer_t &mrPathOpTracer;

	HOP	mpOpStart;
	HOP	mpOpEnd;
	bool	mbXSected;
	//bool	mbAuto;
	HOP	mpOpNx;
	std::list<std::pair<HPATH , int> > m_trace;
public:
	UnrootTracer_t(const RootsTracer_t &r, PathOpTracer_t &tr, HOP start, HOP end)//, bool b)
		: FuncInfo_t(r),
		m_r((RootsTracer_t &)r),
		mrPathOpTracer(tr),
		mpOpStart(start),
		mpOpEnd(end),
		mbXSected(false),
		//mbAuto(b),
		mpOpNx(HOP())
	{
	}
	//auxiliary object to trace xout absence on following routes after xsection found
	UnrootTracer_t(const UnrootTracer_t &o)
		: FuncInfo_t(o),
		m_r(o.m_r),
		mrPathOpTracer(o.pathOpTracer()),
		mpOpStart(o.mpOpStart),
		mpOpEnd(o.mpOpEnd),
		mbXSected(true),//!
		//mbAuto(o.mbAuto),
		mpOpNx(HOP())
	{
	}
	~UnrootTracer_t()
	{
		assert(m_trace.empty());
	}

	PathOpTracer_t &pathOpTracer() const { return mrPathOpTracer; }
	PathTracer_t &pathTracer() const { return mrPathOpTracer; }
	OpTracer_t &opTracer() const { return mrPathOpTracer.opTracer(); }

	int		__traceUnroot(HPATH pPath, HOP);
	int		traceOp(HPATH pPath, HOP);
	int		traceOp2(CHPATH hPath);
	int		matchUnroot(CHOP pOp, HOP &pOpXDep);
	int		CheckDependentLeaves(CHOP  pOp0);//if some leaf of pOp is depends on this one
	bool	IsThereCallOnBranch(CHOP  pOp0);

	void addTrace(HPATH pPath)
	{
		m_trace.push_back(std::make_pair(pPath, PathNo(pPath)));
	}
	bool is_auto() const { return m_r.mbAuto; }
};

RootsTracer_t::RootsTracer_t(const FuncTracer_t &r, HOP  pOp, bool bAuto, bool bUser)
	: FuncTracer_t(r),
	mpOp(pOp),
	mbAuto(bAuto),
	mbUser(bUser)//,
	//mpTracer(0)
{
	assert(IsPrimeOp(pOp));
	//mArrRoots.m_nID = 2;
}

RootsTracer_t::RootsTracer_t(const FuncInfo_t &fi, HOP pOp, bool bAuto, bool bUser, PathOpTracer_t &tr)
	: FuncTracer_t(fi, tr),
	mpOp(pOp),
	mbAuto(bAuto),
	mbUser(bUser)
{
}

int RootsTracer_t::IsSplitRootOffAdvised(HOP pOp0)
{
	if (!IsCode(pOp0))
		return 1;

	if (IsPrimeOp(pOp0))
	{
		if (IsCall(pOp0))
			return 0;
		if (!mbUser)
		{
			if (ActionOf(pOp0) == ACTN_MUL)
				return 0;
			if (ActionOf(pOp0) == ACTN_DIV)
				return 0;
			if (ActionOf(pOp0) == ACTN_MOD)
				return 0;
		}

		for (OpList_t::Iterator i(pOp0->argsIt()); i; i++)
		{
			if (!IsSplitRootOffAdvised(i.data()))
				return 0;
		}
	}

	for (XOpList_t::Iterator i(pOp0->m_xin); i; i++)
	{
		HOP pOp = i.data();
		if (!IsRootEx0(pOp))
			if (!IsSplitRootOffAdvised(pOp))
				return 0;
	}

	return 1;
}

#define RETURN(arg) { AntiLock--; return arg; }
bool UnrootTracer_t::IsThereCallOnBranch(CHOP  pOp0)
{
static int AntiLock = 0;
AntiLock++;
if (AntiLock > 100)
RETURN(1);

	if (!IsCode(pOp0))
		RETURN(0);

	if (IsPrimeOp(pOp0))
	{
		if (IsCall(pOp0))
			RETURN(1);

		for (OpList_t::Iterator i(pOp0->argsIt()); i; i++)
		{
			HOP pArg(i.data());
			if (!IsRootEx0(pArg))
				if (IsThereCallOnBranch(pArg))
					RETURN(1);
		}
	}

	for (XOpList_t::Iterator i(pOp0->m_xin); i; i++)
	{
		HOP rOp(*i);
		if (!IsRootEx0(rOp))
			if (IsThereCallOnBranch(rOp))
				RETURN(1);
	}

	RETURN(0);
}
#undef RETURN

int RootsTracer_t::CheckDependentLeavesOf(HOP  pOp0, HOP pOp)
{
	assert(IsPrimeOp(pOp0));

	if (IsPrimeOp(pOp))
	{
		if (pOp0->ins().mEFlagsModified & pOp->ins().mEFlagsTested)
			return 1;

		for (OpList_t::Iterator i(pOp->argsIt()); i; i++)
		{
			if (CheckDependentLeavesOf(pOp0, i.data()))
				return 1;
		}
	}
	else
	{
		if (pOp == pOp0)//depend on itself//????
			return 1;

		if ( !IsCode(pOp) )
			return CheckIntersection(pOp0, pOp);

//		if (pOp->IsCallOutOp())
//		{
//			if (pOp->pr imeOp()->isRoot())
//				return CheckIntersection(pOp0, pOp);
//		}

		if (IsOnTop(pOp))
		{
			if (pOp->IsScalar())
				return 0;

			if (pOp->IsIndirect())
			{
				if (IsCall(pOp0) && !IsLocalOp(pOp))
					return 1;//non-locals can't step over the func call

				if (pOp->IsIndirectB())
				{
					for (XOpList_t::Iterator i(pOp->m_xin); i; i++)
					{
						HOP pOpx = i.data();
						if (IsCode(pOpx) && !PrimeOp(pOpx)->isRoot())
						{
							if (CheckDependentLeavesOf(pOp0, pOpx))
								return 1;
						}
						else
						{
							if (CheckIntersection(pOp0, pOpx))
								return 1;
						}
					}
				}

				if (!pOp0->IsIndirect())
					return 0;
			}

CHECK(OpNo(pOp0) == 459)
STOP

			return CheckIntersection(pOp0, pOp);
		}

		for (XOpList_t::Iterator i(pOp->m_xin); i; i++)
		{
			if (CheckDependentLeavesOf(pOp0, i.data()))
				return 1;
		}
	}

	return 0;
}

static int _G;
int UnrootTracer_t::matchUnroot(CHOP pOp, HOP &pOpXDep)
{
	if (mbXSected)
		return 1;//continue - there is no need to check intersection further

	if (pOp->isHidden())
		return 1;//skip

	if (!pOp->isRoot())
		return 1;//skip

	if (IsVarOp(pOp))
		return 1;

	if (CheckDependentLeaves(pOp))
	{
//		if (bXOutSeen)
//			return 0;
//		if (pOp->Path() == Path())
//			return 0;//on the same path!
		pOpXDep = pOp;	//will unroot in case if other branches of execution will go to the return of function
	}

#if(1)
	if (!IsLocalOp(mpOpStart))
		return 1;

	ExprCacheEx_t aExprCache(PtrSize());
	AnlzXDepsIn_t an(*this, pathOpTracer(), HOP(), aExprCache);
	if (an.CheckIfArgsOverlapLocal(pOp, mpOpStart))
	{
		pOpXDep = pOp;//will unroot if other branches of execution return from the function
		return 0;//do not unroot
//		if (bXOutSeen)
//			return 0;
	}
#endif

	return 1;//continue
}


#define RETURN(arg)	{ return arg; }
int UnrootTracer_t::traceOp(HPATH pPath, HOP pOp)
{
	assert(pPath);
	mpOpNx = HOP();
	if (pOp)//just started
	{
		assert(IsMineOp(pPath, pOp));
		//assert(pOp == PrimeOp(mpOpStart));
		mpOpNx = NextPrime(pOp);;
		if (!mpOpNx)
			return -1;//proceed with the next path in a flow
		//do not mark a starting path
	}
	else
	{
		path_trace_cell_t &trp(pathTracer().cell(pPath));
		if (trp._traced > 0)
		{
			if (trp._xoutseen)
				if (mbXSected)
					RETURN(0);
			RETURN(1);
		}
		assert(trp._traced < 3);
		trp._traced++;
		mpOpNx = GetFirstOp(pPath);
	}

	return traceOp2(pPath);
}

static int m_return(bool b, bool bStage, int iRet, int iOp)
{
#if(0)
	if (!b)
	{
		if (!bStage)
			printf("e(%d)=%d\n", iOp, iRet);
		else
			printf("i(%d)=%d\n", iOp, iRet);
	}
#endif
	return iRet;
}

#define M_CODE(i) m_return(is_auto(), mbXSected, i, OpNo(mpOpNx));

int UnrootTracer_t::traceOp2(CHPATH hPath)
{
	if (!mpOpNx)//LAST!
	if (PathType(hPath) != BLK_EXIT)
	{
		assert(0);
		RETURN(0);
	}

#ifdef _DEBUG
	//	__Droot.Log(pPath);
	//fprintf(stdout, "path=%d\n", rPath.IndexEx());
#endif

	assert(!mpOpNx || IsMineOp(hPath, mpOpNx));

	for (PathOpList_t::Iterator i(hPath->ops(), INSPTR(mpOpNx)); i; ++i)
	{
		mpOpNx = PRIME(i.data());

CHECK(OpNo(mpOpNx) == 123)
//CHECK(tu.pOpEnd->No() == 376)
STOP

		HOP pOpXDep = HOP();

		if (mpOpNx == PrimeOp(mpOpEnd))//destination reached
		{
			if (mbXSected)
			{
				M_CODE(4);
//				if (!is_auto())//manual
//					fprintf(STDERR, "UPROOT rejected : Source terminals interlaced\n");
				return 0;//abort - do not uproot (despite getting at destination, has been interlaced at some place)
			}
			assert(!pathTracer().cell(hPath)._xoutseen);
			M_CODE(3);
			return 3;
		}

		int result = matchUnroot(mpOpNx, pOpXDep);
		if (pOpXDep)
		{
			if (mpOpStart->CheckConditionDependency())//mpOpStart->OpC() == SSID_CPUSW)
			{
				mpOpNx->m_nFlags |= OPND_POST;
			}
			else
			{

				m_r.SetFailure(RootsTracer_t::REASON_INTERLACED, pOpXDep);
				M_CODE(2);
				return 2;
			}
		}


		if (result == 0)
		{
//			if (!mbAuto && mpOpXDep)//manual
	//			fprintf(stdout, "ROOT OFF failed : Can't pass over line <%d>\n", mpOpXDep->No());
			return M_CODE(0);//ABORT! (continue(1) )
		}
		if (mpOpNx == PrimeOp(mpOpStart))
			return M_CODE(1);

		M_CODE(-1);
	}

//CHECK(pPath->m_pOps->No() == 424)
//STOP
	mpOpNx = HOP();
	return -1;//should continue
}

//#define zzz

int UnrootTracer_t::__traceUnroot(HPATH pPath, HOP pOp)
{
	int iRet(traceOp(pPath, pOp));
	if (iRet >= 0)
		return iRet;

	HPATH pPathNx = HPATH();
	if (PathType(pPath) == BLK_JMP)
	{
		//lTrace.push_back(pPath);
		pPathNx = GetGotoPath(pPath);
	}
	else if (PathType(pPath) == BLK_JMPSWITCH)//switch
	{
		addTrace(pPath);
		HPATH pJumpTablePath(GetJumpTablePath(pPath));
		for (PathOpList_t::Iterator i(PathOps(pJumpTablePath)); i; i++)
		{
			pPathNx = PathRef(PRIME(i.data()));
			if (!__traceUnroot(pPathNx, HOP()))
			{
				m_trace.pop_back();
				RETURN(0);//don't unroot
			}
#ifdef zzz
			if (pPathNx->m.m_bXOutSeen)
				if (pPath != mpOpStart->Path())
					pPath->m.m_bXOutSeen = 1;
#endif
		}
		m_trace.pop_back();
		return 1;
	}
	else if (PathType(pPath) == BLK_JMPIF)
	{
		pPathNx = GetGotoPath(pPath);
		addTrace(pPath);
		if (!__traceUnroot(pPathNx, HOP()))
		{
			m_trace.pop_back();
			RETURN(0);
		}
		m_trace.pop_back();
#ifdef zzz
		if (pPathNx->m.m_bXOutSeen)
			if (pPath != mpOpStart->Path())
				pPath->m.m_bXOutSeen = 1;
#endif
		pPathNx = TreeNextEx(pPath);
//CHECK(!pPathNx)
//STOP
	}
	else if (PathType(pPath) == BLK_EXIT)
	{
		RETURN(1);
	}
	else
		pPathNx = TreeNextEx(pPath);

	assert(pPathNx);
	addTrace(pPath);
	if (!__traceUnroot(pPathNx, HOP()))
	{
		m_trace.pop_back();
		RETURN(0);
	}
	m_trace.pop_back();
#ifdef zzz
	if (pPathNx->m.m_bXOutSeen)
		if (pPath != mpOpStart->Path())
			pPath->m.m_bXOutSeen = 1;
#endif
	RETURN(1);
}
#undef RETURN

void RootsTracer_t::InsertOp(HOP pOp)
{
	CHECK(IsVarOp(pOp))
		STOP
	mArrRoots.insert(pOp);
}

void RootsTracer_t::PrepareBranchInfo(HOP  pOp0)
{
//CHECK(IsPrimeOp(pOp0) && pOp0->No() == 114)
//STOP

	if (IsPrimeOp(pOp0) && !IsArgOp(pOp0))
	{
		if (IsCall(pOp0))
			InsertOp(pOp0);
	}
	else
	{
		if (IsOnTop(pOp0))
		{
			if (pOp0->IsScalar())
				return;

			if (IsThisPtrOp(pOp0))
				return;

			if (pOp0->IsIndirect())
			{
	//			if (pOp0->IsCall() && !pOp->isLocal())
	//				return 1;//non-locals can't step over the func call

				if (pOp0->IsIndirectB())
				{
					for (XOpList_t::Iterator i(pOp0->m_xin); i; i++)
					{
						HOP pOp = i.data();
						if (IsCode(pOp) && !PrimeOp(pOp)->isRoot())
							PrepareBranchInfo(pOp);
						else
							InsertOp(pOp);
					}
				}

			}

			InsertOp(pOp0);
			return;
		}
	}

	if (IsPrimeOp(pOp0))
	{	
		for (OpList_t::Iterator i(pOp0->argsIt()); i; i++)
			PrepareBranchInfo(i.data());
	}
	else
	{
		for (XOpList_t::Iterator i(pOp0->m_xin); i; i++)
			PrepareBranchInfo(i.data());
	}
}

int UnrootTracer_t::CheckDependentLeaves(CHOP pOp0)
{
	bool bCall = IsThereCallOnBranch(pOp0);

	for (std::set<HOP >::const_iterator i(m_r.mArrRoots.begin()); i != m_r.mArrRoots.end(); i++)
	{
		HOP pOp(*i);
		if (bCall)
		if (IsPrimeOp(pOp) && IsCall(pOp))
			return 1;
		if (m_r.CheckIntersection(pOp0, pOp))
			return 1;
	}
	return 0;
}

int RootsTracer_t::TryUnroot(HOP pOp0, HOP pOpOut)
{
//	assert(IsPrimeOp(pOpUp));

/*
	if (pOpUp->IsCall())
	{
		assert( pOpUp->XOut()->CheckCount(1) == 0 );
		pOpUp = pOpUp->XOut()->pOp;
		assert(pOpUp->IsCallOutOp());
		assert( pOpUp->XOut()->CheckCount(1) == 0 );
	}
*/

//...

	if (!IsCodeOp(pOpOut))
		return 0;

	mArrRoots.clear();
	PrepareBranchInfo(pOp0);


	int res;
	{//RAII-block
		//PathTracer_t aTracer;
		PathTracer_t &aTracer(pathTracer());
		aTracer.reset(*this);
		UnrootTracer_t tu(*this, pathOpTracer(), pOp0, pOpOut/*, tracer*/);// , mbAuto);
		res = tu.__traceUnroot(HPATH(), HOP());
	}

	return res;
}

int RootsTracer_t::CheckScatteredAssignment(HOP pSelf, HPATH pPath)
{
	HOP pOp = pSelf;
	while (1)
	{
		if (!IsCodeOp(pOp))
			return -1;//abort

		assert(IsPrimeOp(pOp));
		if (pPath)
		if (pPath != PathOf(pOp))
			return -1;//abort

//		if (pOp->IsRootEx())
//			return 0;

		if (ActionOf(pOp) != ACTN_MOV)
			return 0;

		HOP pOp0 = pOp->arg1();
		if (IsAddr(pOp0))
			return 0;
		if (pOp0->IsScalar())
		{
			if (OpDisp(pOp0) == 0)
				return 1;
			return 2;//FIXME
		}

		if (pOp0->IsIndirect() && !IsLocalOp(pOp0))
			return 0;
		if (pOp0->m_xin.check_count(1) != 0)
			return 0;
		pOp = pOp0->XIn()->data();
	}

	return 1;
}

int RootsTracer_t::CheckXIn(HOP  pOp0, HOP pOpUp)
{
	XOpList_t lXIns = pOp0->m_xin;

	if (lXIns.check_count(1) > 0)
	{
		//return 0;
		bool bVar = false;
		bool bNonZero = false;

		for (XOpList_t::Iterator i(lXIns); i; i++)
		{
			HOP pOp = i.data();
//			if (pOp == pOpUp)
//				continue;

			int res = CheckScatteredAssignment(pOp, PathOf(pOpUp));
			if (res == -1)//another path
			{
				if (pOp != pOpUp)
					return 0;//abort right now
				res = 0;
			}
			if (res == 0)
			{
				if (bVar || bNonZero)
					return 0;
				bVar = true;
			}
			else if (res == 1)//zero padding
			{
			}
			else if (res == 2)//non-zero padding
			{
				if (bVar)
					return 0;
				bNonZero = true;
			}
		}

		for (XOpList_t::Iterator i(lXIns); i; i++)
		{
			HOP pOp(i.data());
			if (pOp != pOp0)
			if (!CanUnroot(pOp, false))
				return 0;
		}
	}
	assert(!lXIns.empty()/* && (pXIn->pOp == pOpUp)*/);

	return 1;
}

int RootsTracer_t::TryUnroot0(HOP  pOp0, HOP pOpUp, bool bPad)
{
	for (XOpList_t::Iterator i(pOpUp->m_xout); i; i++)
	{
		HOP pOpOut = i.data();

		if (pOpUp == pOp0)
		{
			if (pOpOut->m_nFlags & OPND_XINEX)
				return 0;//dest op may have implicit xdeps

			if (bPad)
			if (!CheckXIn(pOpOut, pOpUp))
				return 0;
		}

		if (IsCallOutOp(pOpOut))
		{
			if (!TryUnroot0(pOp0, pOpOut, bPad))
				return 0;
		}
		else
		{
			HOP pOpOutR = PrimeOp(pOpOut);
			if (!pOpOutR->isRoot())
			{
				if (!TryUnroot0(pOp0, pOpOutR, bPad))
					return 0;
			}
			else
			{
				if (!TryUnroot(pOp0, pOpOut))
					return 0;
			}
		}
	}

	return 1;
}

int RootsTracer_t::CanUnroot(HOP  pOp0, bool bPad)
{
	assert(IsPrimeOp(pOp0));

CHECK(OpNo(pOp0) == 114)
STOP

//	if (nPaths)
//		*nPaths = 0;

//	if (IsLastEx())//???
//		return 0;

	if (!pOp0->XOut())
		return 0;

	if (pOp0->IsIndirect())
		if (!IsLocalOp(pOp0))
			return 0;//do not unroot globals

	HOP  pOpBase = pOp0;
	if (IsCall(pOpBase))
	{
		if (pOp0->m_xout.check_count(1) != 0)
			return 0;

		pOpBase = pOp0->XOut()->data();
		assert(IsCallOutOp(pOpBase));
		//?assert(pOpBase->XOut());
		if (pOpBase->m_xout.check_count(1) != 0)
			return 0;//do not unroot 
	}

	if (mbAuto)
	{
		FieldPtr pFieldRef(LocalRef(pOpBase));
		if (pFieldRef)
			//if (IsData(pFieldRef))
				if (IsLocalReg(pFieldRef))
					return 0;
	}

	if (pOpBase->m_xout.check_count(1) > 0)
		if (!IsSplitRootOffAdvised(pOp0))
			return 0;

	return TryUnroot0(pOpBase, pOpBase, bPad);
}

int AnlzRoots_t::turnRoot_On()
{
	if (mpOp->isRoot())
		return -1;//already

	SetRoot(mpOp, 1);
	mpOp->setLValueVisible(1);

	return 1;
}


int AnlzRoots_t::CheckOrderEx(HOP pSelf, HOP pOp)
{
	if (pSelf == pOp)
		return 0;

	bool bEntry1 = IsArgOp(pSelf);
	bool bEntry2 = IsArgOp(pOp);

	if (bEntry1)
	{
		if (bEntry2)
			return PathOf(pSelf)->ops().CheckRelation(INSPTR(pSelf), INSPTR(pOp));

		if (!IsPrimeOp(pOp))
			return -2;
		return -1;//entry(this) always higher than body op!
	}

	if (bEntry2)
		return 1;//entry(pOp) is lower than this body op!

	if (!IsPrimeOp(pSelf) || !IsPrimeOp(pOp))
		return -2;

	if (PathOf(pSelf) != PathOf(pOp))
	{
		if (TreeIsLowerThan(PathOf(pSelf), PathOf(pOp)))
			return 1;
		return -1;
	}

	return PathOf(pSelf)->ops().CheckRelation(INSPTR(pSelf), INSPTR(pOp));
}

int AnlzRoots_t::OrderXIns(HOP pSelf, bool b)
{
	for (XOpList_t::Iterator i(pSelf->m_xin); i; i++)
	{
		HOP pOp1 = PrimeOp(i.data());

		XOpList_t::Iterator j(i);
		j++;
		for (; j; j++)
		{
			HOP pOp2 = PrimeOp(j.data());
			int res = CheckOrderEx(pOp1, pOp2);
			if (res == -2)
				return 0;//failed = don't mess with it

			assert(res != 0);

			if ((!b && res == 1)//increased order & pOp1 > pOp2
				|| (b && res == -1))//decreased order & pOp1 < pOp2
			{
				pSelf->m_xin.rearrange(i.self(), j.self());//pXIn1->LinkAfter(&pSelf->m_pXIn, pXIn2);
				return 1;
			}
		}
	}

	return 0;
}


int AnlzRoots_t::ToggleRoot()//bool bAuto)
{
//CHECK(No() == 367)
//STOP
	//?AnlzRoots_t anlz(this, bAuto, !bAuto);
	int res = TurnRoot_Off();
	if (res == 0)
		return 0;

	if (res == -1)
	{
		res = TurnRoot_On();
		if (res == 0)
			return 0;
	}
	
	return 1;
}

/*#if(0)
#define RETURN(arg) { / *pPath->m_bTraced2 = 0;* / return arg; }
int Op_t::__traceRootAssureUp(HPATH pPath, HOP pOpEnd, HOP pOp0)
{
	HOP pOpFrom = pr imeOp();
	while (1)
	{
		if (!pPath)//starting from this
		{
			pPath = pOpFrom->Path();
		}
		else
		{
			if (pPath->m_bTraced2)
				return 0;
			pPath->m_bTraced2 = 1;
		}

		if (pPath == pOp0->Path())
		{
			if (pOpEnd->Path() != pPath)
				RETURN(1);//not reached else

			HOP pOp = pOp0;
			while (1)//the same path
			{
				pOp = pOp->P rev();
				if (!pOp)
					RETURN(0);
				if (pOp == pOpFrom)
					RETURN(0);
				if (pOp == pOpEnd)
					RETURN(1)
			}
		}
		else if (pPath == pOpEnd->Path())
		{
			RETURN(0);//dest reached
		}

		//get previous path
		HPATH pPathPr = pPath->PrevEx();
		if (!pPathPr)
		{
			RETURN(0);//so this is a first leaf of the tree
		}

		if (pPath->Label())
		{
			for (XRef_t *pXRef = pPath->Label()->m_pXRefs; pXRef; pXRef = pXRef->N ext())
			{
				HOP pOpRef = pXRef->Op();
				if (pOpRef->IsDataOp())
					pOpRef = (OpPtrpOpRef)->GetSwitchOp();

				if (!pOpRef->isHidden())
				{
					HPATH pPathJ = pOpRef->Path();
					if (__traceRootAssureUp(pPathJ, pOpEnd, pOp0))
						RETURN(1);
				}
			}
		}

		switch (pPathPr->Type())
		{
		case BLK_JMP:
		case BLK_JMPSWITCH:
//		case BLK_RET:
			RETURN(0);//control flow break
		}
		
		pPath = pPathPr;
	}

	RETURN(1);
}
#undef RETURN

#define RETURN(arg)	{ / *pPath->m_bTraced1 = 0;* / return arg; }
int Op_t::__traceRootAssureDown(HPATH pPathUp, HPATH pPath)
{
	HOP pOpRoot = prim eOp();
	HOP pOpNx = 0;
	if (!pPath)
	{
		pPath = pOpRoot->Path();//do not mark start path
		pOpNx = pOpRoot->NextEx();
	}
	else
	{
		if (pPath->m_bTraced1)
			return 1;
		pPath->m_bTraced1 = 1;
		pOpNx = GetFirstOp(pPath);
	}

	if (!pOpNx)//LAST!
	{
		RETURN(1);
	}

	while (pPath->IsMineOp(pOpNx))
	{
		if (pOpNx == pri meOp())
			RETURN(1);
		pOpNx->__checkRoots(this);

//CHECK(pOpNx->No() == 473)
//STOP
		pOpNx = pOpNx->Next();
	}

	HPATH pPathNx = 0;
	if (PathType(pPath) == BLK_JMP)
	{
		pPathNx = pPath->GetGotoPath();
	}
	else if (PathType(pPath) == BLK_JMPSWITCH)//switch
	{
		FieldPtr pJumpTable = ->GetJumpTable(pPath);
		for (OpList_t::Iterator i(pJumpTable->Ops()); i; i++)
		{
			pPathNx = i.data()->lab elRef()->Path();
			__traceRootAssureDown(pPath, pPathNx);
		}
		RETURN(1);
	}
	else if (PathType(pPath) == BLK_JMPIF)
	{
		pPathNx = pPath->GetGotoPath();
		__traceRootAssureDown(pPath, pPathNx);
		pPathNx = pPath->NextEx();
	}
	else if (PathType(pPath) == BLK_EXIT)
	{
		RETURN(1);
	}
	else
		pPathNx = pPath->NextEx();

	assert(pPathNx);
	__traceRootAssureDown(pPath, pPathNx);

	RETURN(1);
}
#undef RETURN

//checks if <pOp0> lies on the link between <this> and <pOpEnd>
void Op_t::CheckRoots(HOP pOpEnd, HOP pOp0)
{
	if (!pOpEnd->IsCode())
		return;
	if (PrimeOp(pOpEnd)->isRoot())
		return;

	GetOwne rFunc()->ClearTracer3();
	if (!__traceRootAssureUp(0, pOpEnd, pOp0))
		return;//does not pass through pOp0

	HOP pOp = PrimeOp(pOpEnd);
	if (G DC.arrReRoots.Add(pOp) != -1)
		TRACE1("ROOT status of Op<%d> needs to be re-checked\n", pOp->No());
}

void Op_t::__checkRoots(HOP pOp0)
{
	assert(IsPrimeOp(pOp0));

	if (IsPrimeOp())
	{	
		for (OpList_t::Iterator i(argsIt()); i; i++)
			i.data()->__checkRoots(pOp0);
	}
	else
	{
		for (XRefList_t::Iterator i(m_xin); i; i++)
			CheckRoots(i.data(), pOp0);
	}
}

#endif*/

void RootsTracer_t::ReportError()
{
	if (!mbAuto)//manual
	{
		switch (GetFailure().iReason)
		{
		case RootsTracer_t::REASON_INADVISED:
			fprintf(STDERR, "REDUCE unadvised : Expression may increase in complexity\n");
			break;
		case RootsTracer_t::REASON_INTERLACED:
			fprintf(STDERR, "REDUCE rejected : Can't step over L%s (terminal overwrite en route)\n", StrNo(GetFailure().pAtOp).c_str());
			break;
		default:
			fprintf(STDERR, "REDUCE rejected : Reason unknown\n");
		}
	}
}

int RootsTracer_t::CheckIntersectionIndirect(CHOP pSelf, CHOP pOp) const
{
//CHECK(No() == 0)
//STOP
	HOP pOp1 = pSelf;
	HOP pOp2 = pOp;

	int disp1 = 0; 
	int disp2 = 0;
	int sz1 = pOp1->OpSize();
	int sz2 = pOp2->OpSize();
	bool loc1 = false;
	bool loc2 = false;

	ptrinfo_t p1(*this);
	if (!IsLocalOp(pOp1) && !IsGlobalOp(pOp1))//!fieldRef(pOp1))
	{
		if (!p1.TracePtrSource(pOp1) || !p1.pOp)
			return -1;//failed to determine
		disp1 += p1.disp;
		disp1 += pOp1->m_disp;
		if (IsAddr(p1.pOp))
		{
			pOp1 = p1.pOp;
			p1.pOp = HOP();
		}
	}

	ptrinfo_t p2(*this);
	if (!IsLocalOp(pOp2) && !IsGlobalOp(pOp2))//?!fieldRef(pOp2))
	{
		if (!p2.TracePtrSource(pOp2))
			return -1;
		disp2 += p2.disp;
		disp2 += pOp2->m_disp;
		if (p2.pOp && IsAddr(p2.pOp))
		{
			pOp2 = p2.pOp;
			p2.pOp = HOP();
		}
	}

	if (p1.pOp && p2.pOp)
	{
		if (p1.pOp != p2.pOp)
			return -1;
	}
	
	if (LocalRef(pOp1) || IsLocalOp(pOp1))
	{
		disp1 += pSelf->m_disp;
		if (IsLocalOp(pOp1))
		{
			loc1 = true;
			disp1 = pOp1->Offset0();
		}
	}

	if (LocalRef(pOp2) || IsLocalOp(pOp2))
	{
		disp2 += pOp2->m_disp;
		if (IsLocalOp(pOp2))
		{
			loc2 = true;
			disp2 = pOp2->Offset0();
		}
	}

	if (loc1 ^ loc2)
		return 0;//one is local, another is not - never intersect!
	
	if (!loc1 && !loc2)
	{
		if (LocalRef(pOp1) != LocalRef(pOp2))
			return 0;
	}

	return checkoverlap(disp1, sz1, disp2, sz2);
}

int RootsTracer_t::CheckIntersection(CHOP pSelf, CHOP pOp) const
{
	REG_t r(pOp->Offset0() + CalcDispl(pOp), pOp->OpSize());

	if (IsPrimeOp(pSelf))
	{
		if (IsCall(pSelf))
		{
			GlobPtr iCallee(GetCalleeFuncDef(pSelf));
			if (!iCallee)//?
				return 0;

			//FuncInfo_t rfCallee(DcRef(), *iCallee);// dc().FindFileDefOf(*pfDef));

			if (pOp->OpC() == OPC_CPUREG)
			{
				if (!IsOpSavedByCallee(pOp, iCallee))
					return 1;
			}
			else if (pOp->OpC() == OPC_FPUREG)
			{
#if(1)
				ProtoProfile_t si;
				GetArgProfileFromCall(si, pSelf);
				int _fpu_max = FpuIn(pSelf) + si.fpuin;
				int _fpu_min = FpuIn(pSelf) + pSelf->fstackDiff();
				int _fpuid = pOp->OpOffs() / FR_SLOT_SIZE;

				if ((_fpu_min <= _fpuid) && (_fpuid < _fpu_max))
					return 1;
#else
				if (pOp->OpOffs() <= iCallee->typeFuncDef()->getFStackPurge())
					return 1;
#endif
			}
			else if (pOp->IsIndirect())
			{
				if (!IsLocalOp(pOp))
				{
					if (IsGlobalOp(pOp))
						if (IsReadOnlyVA(PrimeSeg(), pOp->m_disp))
							return 0;//referencing an import table
					return 1;
				}
			}
			else if (pOp->OpC() == OPC_AUXREG)
			{
				return 0;
			}
//			else
//				assert(false);
			return 0;
		}
		
		if (IsAddr(pSelf))
			return 0;

		if (pOp->isCPUSW())
		{
			if (BitMask(pOp) & pSelf->ins().mEFlagsModified)
				return 1;
		}
	}

	if (pSelf->IsIndirect())
	{
		if (IsPrimeOp(pOp) && IsCall(pOp))
//?			if (!IsLo cal())
				return 1;

		if (!pOp->IsIndirect())
			return 0;

		if (IsGlobalOp(pOp))
			if (IsReadOnlyVA(PrimeSeg(), pOp->m_disp))
				return 0;//referencing an import table?
		
		//return 1;
		return (CheckIntersectionIndirect(pSelf, pOp) == 1);
	}

	switch (pSelf->OpCf())
	{
	case OPC_CPUREG:
		if (IsPrimeOp(pOp) && IsCall(pOp))
		{
			GlobPtr ifDef(GetCalleeFuncDef(PrimeOp(pOp)));
			if (!ifDef)//?
				return 0;
			//FuncInfo_t rfCallee(DcRef(), *ifDef);// , dc().FindFileDefOf(*pfDef));
			if (!IsOpSavedByCallee(pSelf, ifDef))
				return 1;
		}
		else if ( pOp->OpC() == OPC_CPUREG )
		{
			if (FuncInfo_s::Overlap(REG(pSelf), r))
				return 1;
		}
//		else if ( pOp->IsIndirect() ) 
//		{
//			if (OverlapB(pOp))
//			{
//				assert(!pOp->m_p MLoc);
//				return 1;
//			}
//		}
		return 0;

	case OPC_FPUREG:
		if (pOp->OpC() != pSelf->OpC())
			return 0;
		if (FuncInfo_s::Overlap(REG(pSelf), r))
			return 1;
		return 0;

	case OPC_AUXREG:
		if (pOp->OpC() != pSelf->OpC())
			return 0;
		if (FuncInfo_s::Overlap(REG(pSelf), r))
			return 1;
		return 0;

	case OPC_CPUSW:
		{
		if (IsArgOp(pOp))
			if (!pOp->isCPUSW())
				return 0;
		uint32_t f = (InsRefPrime(pSelf).mEFlagsModified & InsRefPrime(pOp).mEFlagsTested);
		return  (f != 0);
		}

	case OPC_FPUSW:
		if (IsArgOp(pOp))
			if (pOp->OpC() != OPC_FPUSW)
				return 0;
		return (InsRefPrime(pOp).mFFlagsAffected != 0);

	default:;
		//assert(false);
	}

	return 0;
}



///////////////////////////////////////////


int RootsTracer_t::PrepareRootsDest(HOP  pOp0)
{
	int nSplit = 0;

	if (IsRhsOp(pOp0))
	{
		HOP  pOpOut = PrimeOp(pOp0);

		if (pOpOut->isRoot())
			mArrRootsDest.insert(pOpOut);
		else
			nSplit += PrepareRootsDest(pOpOut);
	}
	else
		for (XOpList_t::Iterator i(pOp0->m_xout); i; i++)
	{
		if (i.self() != pOp0->m_xout.head())
			nSplit++;
		HOP pOpOut = i.data();

		if (IsPrimeOp(pOpOut))
		{
			if (pOpOut->isRoot())
			{
				mArrRootsDest.insert(pOpOut);
				continue;
			}
		}
		
		nSplit += PrepareRootsDest(pOpOut);
	}

	return nSplit;
}

/*
int Op_t::CheckFractalAssign2()
{
	HOP pOp = this;
	while (1)
	{
		if (!pOp->IsCodeOp())
			return -1;//abort

		if (pOp->isRoot())
			return 0;

		assert(IsPrimeOp(pOp));
		if (pPath)
		if (pPath != pOp->Path())
			return -1;//abort


		if (pOp->Action() != ACTN_MOV)
			return 0;

		HOP pOp0 = &pOp->arg1();
		if (pOp0->IsA ddr())
			return 0;
		if (pOp0->IsScalar())
		{
			if (pOp0->i64 == 0)
				return 1;
			return 2;//FIXME
		}

		if (pOp0->IsIndirect() && !pOp0->IsLo cal())
			return 0;
		if (pOp0->XIn()->CheckCount(1) != 0)
			return 0;
		pOp = pOp0->XIn()->pOp;
	}

	return 1;
}
*/
bool AnlzRoots_t::UnrootXIns(CHOP pSelf)
{
	assert(!pSelf->m_xin.empty());

	HPATH pPath = HPATH();

	for (XOpList_t::Iterator i(pSelf->m_xin); i; i++)
	{
//		if (pXIn != pXIns)
//			return false;
		HOP pOpIn = i.data();
		if (!IsCode(pOpIn))
			return false;
		pOpIn = PrimeOp(pOpIn);
		if (!pPath)
			pPath = PathOf(pOpIn);
		else if (PathOf(pOpIn) != pPath)
		{
			if (!mbAuto)
				fprintf(stdout, "UPROOT rejected : Forked inflow dependency of L%d\n", OpNo(PrimeOp(pSelf)));
			return false;
		}
		//...
		
		arrRootsSrc.insert(pOpIn);
	}

	return true;
}

bool AnlzRoots_t::PrepareSrcOps(CHOP pOp)//pOp - initiator
{
	arrRootsSrc.clear();
	for (XOpList_t::Iterator i(pOp->m_xout); i; ++i)
	{
		HOP pOpOut(i.data());
		if (pOpOut->m_nFlags & OPND_XINEX)
			return false;//dest op may have implicit xdeps

		if (!IsCallOutOp(pOp))
		{
			int d(pOpOut->Offset0() - pOp->Offset0());
			if (d != 0)
			{
				//check if op args can be forked, example:
				// VQ8=G1    <= cannot fork!
				// call(VD8,VD4)
				assert(IsPrimeOp(pOp));
				for (OpList_t::Iterator j(pOp->args()); j; ++j)
				{
					HOP pOpIn(j.data());
					if (pOpIn->OpC() == OPC_FPUREG)
						return false;
				}
			}
		}

		if (!UnrootXIns(pOpOut))
			return false;
	}

	return true;
}

void AnlzRoots_t::RestoreSrcOps()
{
	for (std::set<HOP>::const_iterator i(arrRootsSrc.begin()); i != arrRootsSrc.end(); i++)
	{
		HOP pOp(*i);
		SetRoot(pOp, 1);
	}
}

int RootsTracer_t::TryUnroot2()
{
	//assert(IsPrimeOp());

	mArrRoots.clear();
	PrepareBranchInfo(mpOp);

	mArrRootsDest.clear();
	int nSplitCount = PrepareRootsDest(mpOp);

	if (nSplitCount > 0)
//	if ( mArrRootsDest.m_pXRefs->CheckCount(1) > 0 )
	if (!IsSplitRootOffAdvised(mpOp))
	{
		SetFailure(RootsTracer_t::REASON_INADVISED);
		return 0;
	}

	for (std::set<HOP >::const_iterator i(mArrRootsDest.begin()); i != mArrRootsDest.end(); i++)
	{
		//RAII-block
		//PathTracer_t aTracer;
		PathTracer_t &aTracer(pathTracer());
		aTracer.reset(*this);
		//setPathTracer(&aTracer);
		
		HOP hOpDst(*i);
//DBGOUT(MyStringf("\topDst=%s\n", StrNo(hOpDst).c_str()));
		UnrootTracer_t ut1(*this, pathOpTracer(), mpOp, hOpDst/*, aTracer*/);// , mbAuto);

		HOP pStartOp(PrimeOp(mpOp));
		HPATH pStartPath(PathOf(pStartOp));

CHECK(OpNo(pStartOp) == 1842)
STOP

		//stage 1

		FlowIteratorEx j(*this, pStartPath);
		while (j)
		{
			HPATH hPath(*j);
			if (!hPath)
				return 0;
#ifdef _DEBUG
int iNo(PathNo(hPath));
CHECK(iNo == 44)
STOP
//DBGOUT(MyStringf("\t\tpath=%d\n", PathNo(hPath)));
#endif

			path_trace_cell_t &trp1(pathTracer().cell(hPath));

			bool bReentered(false);
			if (pStartPath)
			{
				ut1.mpOpNx = NextPrime(pStartOp);
				pStartPath = HPATH();
			}
			else
			{
				if (trp1._traced > 0)
				{
					j.drop();//abort a branch
					++j;
					continue;
				}

				if (hPath == PathOf(pStartOp))//entered the starting path from above
					bReentered = true;

				assert(trp1._traced < 3);
				trp1._traced++;//a starting path never marked
				ut1.mpOpNx = GetFirstOp(hPath);
			}

			if (ut1.mpOpNx)
			{
				int iRet(ut1.traceOp2(hPath));
				if (iRet == 0)
					return 0;//can't unroot
				if (iRet == 1)
					break;//unroot confirmed at stage 1
				if (iRet == 2)
				{
					//EXTRA PASS: when terminal overwrite detected, proceed tracing to find out if the target is going to be hit on some path.
					// If the targit is hit, reject such case, OK - otherwise.

					//return 0;
					/////////////////////////////////////////////////////////
					pStartPath = hPath;

					UnrootTracer_t ut2(ut1);
					for (FlowIterator k(*this, pStartPath); k; ++k)
					{
						if (pStartPath)
						{
							ut2.mpOpNx = NextPrime(ut1.mpOpNx);
							pStartPath = HPATH();
						}
						else
						{
							STAGE_STATUS_e e;
							while (*k && (e = CheckStatus(*k)) != STAGE_STATUS_e::CONTINUE)
							{
								if (e == STAGE_STATUS_e::FAILED)
									return 0;
								k.drop();
							}

							ut2.mpOpNx = GetFirstOp(*k);
						}

						HPATH hPath(*k);
						if (!hPath)
							break;// return 0;

/*#ifdef _DEBUG
int iNo(PathNo(hPath));
CHECK(iNo == 2)
STOP
DBGOUT(MyStringf("\t\t\tpath=%d\n", PathNo(hPath)));
#endif*/

						if (ut2.mpOpNx)
						{
							int iRet(ut2.traceOp2(hPath));
							if (iRet == 0)
								return 0;//can't unroot
							if (iRet == 1)
								break;//unroot confirmed at stage 2
						}

						assert(!ut2.mpOpNx);
						//++k;
					}

					//?++j;
					continue;
					//break;//stage 2 required
				}
				else if (iRet == 3)
				{
					if (bReentered)
						return 0;//reject this case to break cyclic dependency

					//Target x-out has been reached. Mark the route from the starting path down to the current one 
					//	and keep tracing to see if all other paths exit the function without interlacing.
					for (size_t k(j.backtrace().size()); k > 0; k--)
					{
						HPATH pPath(j.backtrace()[k - 1]);
						if (pPath != PathOf(ut1.mpOpStart))//in case there is a loop, we want to enter the starting path from above
							pathTracer().cell(pPath)._xoutseen = 1;
					}
					j.drop();//skip the rest of the current branch
					++j;
					continue;
				}
				assert(iRet < 0);
			}
			assert(!ut1.mpOpNx);
			++j;
		}
	}

	return 1;
}

STAGE_STATUS_e RootsTracer_t::CheckStatus(CHPATH hPath) const
{
	//if (!hPath)
		//return STAGE_STATUS_e::FAILED;

	path_trace_cell_t& trp2(pathTracer().cell(hPath));
	if (trp2._traced > 0)
	{
		if (trp2._xoutseen)
			return STAGE_STATUS_e::FAILED;
		//k.drop();//abort a branch
		//++k;
		return STAGE_STATUS_e::SKIPPED;
	}
	assert(trp2._traced < 3);
	trp2._traced++;//a starting path never marked
	return STAGE_STATUS_e::CONTINUE;
}











///////////////////////////////////////AnlzRoots_t


AnlzRoots_t::AnlzRoots_t(const FuncTracer_t &r, HOP  pOp, bool bAuto, bool bUser)
	: FuncTracer_t(r),
	mpOp(pOp),
	mbAuto(bAuto),
	mbUser(bUser),
	mbForce(false),
	mbPreserveVars(false)
{
	assert(IsPrimeOp(mpOp));
}

AnlzRoots_t::AnlzRoots_t(const FuncInfo_t &fi, PathOpTracer_t &tr, HOP pOp, bool bAuto, bool bUser)
	: FuncTracer_t(fi, tr),
	mpOp(pOp),
	mbAuto(bAuto),
	mbUser(bUser),
	mbForce(false),
	mbPreserveVars(false)
{
	assert(IsPrimeOp(mpOp));
}

bool AnlzRoots_t::CanUnroot2()
{
CHECK(OpNo(mpOp) == 808)
STOP
//if (mpOp->No() == 808)
//return false;


	HOP pOp(mpOp);
	if (IsCall(pOp))
	{
		if (pOp->m_xout.check_count(1) != 0)
			return false;
		pOp = pOp->XOut()->data();
	}

	if (!pOp->XOut())
	{
		if (!mbAuto)
			fprintf(stdout, "UPROOT rejected : Expression at <%d> has no outflow dependencies\n", OpNo(PrimeOp(pOp)));
		return false;
	}

	if (pOp->IsIndirect())
		if (!IsLocalOp(pOp))
			return false;//do not unroot globals

	if (mbAuto)
	{
		FieldPtr pFieldRef(LocalRef(pOp));
		if (pFieldRef)
			//if (IsData(pFieldRef))
				//	if (pOp->m_pData->IsLoc alEx())
				if (pFieldRef->isComplex())//IS _PTR(pOp->fieldRef()->type()->isComplex())
					return false;
	}

	bool bOk(PrepareSrcOps(pOp));//turn roots off for all src ops

	//all roots are off - now check'em up
	if (bOk)
	{
		for (auto i(arrRootsSrc.begin()); i != arrRootsSrc.end(); ++i)
		{
			HOP pOp(*i);
			//DBGOUT(MyStringf("opSrc=%s\n", StrNo(pOp).c_str()));
			RootsTracer_t an(*this, pOp, mbAuto, mbUser);
			if (!an.TryUnroot2())
			{
				an.ReportError();
				bOk = false;
				break;
			}
		}
	}

	RestoreSrcOps();//revert everything back

	return bOk;
}

int AnlzRoots_t::TurnRoot_Off()
{
	if (!mpOp->isRoot())
		return -1;//already

	if (mpOp->ins().opnd_BAD_STACKOUT)
		return 0;

	HOP pOpThis(mpOp);
	if (IsCall(pOpThis) && mpOp->XOut())
		pOpThis = pOpThis->XOut()->data();

	if (mbPreserveVars)
		if (LocalRef(pOpThis))
			return 0;

//CHECK(mpOp->No() == 4304 && !mbPreserveVars)
//CHECK(mpOp->No() == 880 && ++z == 2)
CHECK(OpNo(mpOp) == 76)
STOP
#if(0)
if (mbAuto)
{
	static int z(0);
	if (mpOp->No() == 880)
		z++;
	if (z >= 2)
		return 0;
}
#endif

	if (!mbForce && !CanUnrootSpecial())
	{
		if (!CanUnroot2())
//		if (!CanUnroot(bAuto, bUser, true))
			return 0;
//		if (bAuto)
//		{
//			if ( !CheckUnrootAdvisability() )
//				return 0;
//		}
	}
	for (XOpList_t::Iterator i(pOpThis->m_xout); i; i++)
	{
		HOP pOpOut(i.data());
		for (XOpList_t::Iterator j(pOpOut->m_xin); j; j++)
		{
			HOP pOpIn(j.data());
			HOP pOpInRoot(PrimeOp(pOpIn));
			if (SetRoot(pOpInRoot, 0) == 1)
			{
				pOpInRoot->m_nFlags &= ~OPND_POST;
				if (CheckLValueVisibility(pOpInRoot))
					pOpInRoot->setLValueVisible(0);
				SetChangeInfo(FUI_LOCALS);
				// now detach a data
				LocalsTracer_t loc(*this);
				loc.CheckLocal0(pOpIn);
				//if (mrFunc.IsDecompiled())
				//	pOpIn->CheckLocals0();//remove attached data if pOpIn is local(ex)
			}
		}
#if(!NEW_OP_PTR)
		while (OrderXIns(pOpOut, false));
#endif
		//break;//?
	}

/*	if (0)
	if (Action() == ACTN_MOV)
	if (XOut()->CheckCount(1) == 0)
	{
		HOP pOp = XOut()->pOp;
		if (pOp->XIn()->CheckCount(1) == 0)
		{
			HOP pOpFrom = PrimeOp(pOp);
			if (pOpFrom->XOut())
			if (pOpFrom->Action() == ACTN_MOV)
			{
				HOP pOpIn = &arg1();
				if (pOpFrom->EqualTo(pOpIn))
				{
					for (XRef_t *pXIn = pOpIn->XIn(); pXIn; pXIn = pXIn->Ne xt())
						pXIn->pOp->TurnRoot_On();

					for (XRef_t *pXOutFrom = pOpFrom->XOut(); pXOutFrom; pXOutFrom = pXOutFrom->N ext())
					{
						HOP pOpInNew = pXOutFrom->pOp;
						bool bBase;
						if (pOpInNew->ReleaseXDepIn(pOpFrom, &bBase) != 1)
							assert(false);
						for (XRef_t *pXIn = pOpIn->XIn(); pXIn; pXIn = pXIn->Ne xt())
						{
							HOP pOpTo = pXIn->pOp;
							pOpTo->ReleaseXDepOut(pOpIn);
							MakeUDLink(pOpInNew, pOpTo);
						}
					}

					pOpFrom->XOut()->UnlinkAndDestroy(&pOpFrom->XOut(), true);
					pOpIn->XIn()->UnlinkAndDestroy(&pOpIn->XIn(), true);
					pOpFrom->Set Hidden(1);
				}
			}
		}
	}
*/
	return 1;
}

int AnlzRoots_t::TurnRoot_On()
{
CHECK(OpNo(mpOp) == 773)
STOP
	if (!IsPrimeOp(mpOp))
		mpOp = PrimeOp(mpOp);//->TurnRoot_On();

	if (mpOp->isRoot())
		return -1;//already

	HOP pOpThis = mpOp;
	if (IsCall(mpOp))
		pOpThis = mpOp->XOut()->data();

	for (XOpList_t::Iterator i(pOpThis->m_xout); i; i++)
	{
		HOP pOpOut = i.data();
		for (XOpList_t::Iterator i(pOpOut->m_xin); i; i++)
		{
			HOP pOpIn = PrimeOp(i.data());
			if (SetRoot(pOpIn, 1) == 1)
				pOpIn->setLValueVisible(1);
		}

/*		for (pXIn = pXIns; 
		pXIn; 
		pXIn = pXIn->Ne xt())
		{
			HOP pOpIn = PrimeOp(pXIn->pOp);
//			G DC.arrNewRoots.Add(pOpIn);
			mrFunc.SetChangeInfo(FUI_ROOTS);
		}*/
		break;//?
	}

	SetChangeInfo(FUI_ROOTS);
//	if (turnRoot_On() == -1)
//		return -1;
	return 1;
}

bool AnlzRoots_t::CanUnroot1()
{
	if (!IsPrimeOp(mpOp))
		return false;
	if (IsCall(mpOp))
		if (mpOp->m_xout.check_count(1) > 0)
			return false;
	return true;
}

int AnlzRoots_t::CanUnrootSpecial()
{return 0;
	if (!IsPrimeOp(mpOp))
		return 0;

CHECK(OpNo(mpOp) == 224)
STOP

	if (ActionOf(mpOp) != ACTN_MOV)
		return 0;

	if (!mpOp->XOut())
		return 0;

	HOP pOp0 = mpOp->arg1();
//	if ( pOp0->checkThisPtr() )
//	{
//		return 1;
//	}

	if ( mpOp->IsIndirect() )
		return 0;//only registers

	if ( !pOp0->IsScalar() )
		if ( !pOp0->checkThisPtr() )
			return 0;

	if (mpOp->m_xout.check_count(1) == 0)
	{
		if (PathOf(mpOp) == PathOf(mpOp->XOut()->data()))
			return 1;
	}

	for (XOpList_t::Iterator i(mpOp->m_xout); i; i++)
	{
		HOP pOp = i.data();
		if (pOp->OpC() != mpOp->OpC())
			return 0;//memory indirect
		if (pOp->m_xin.check_count(1) > 0)//at least one
			return 0;
	}

	return 1;//all passed ok
}

