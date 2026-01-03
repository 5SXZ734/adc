#include "op.h"
#include "prefix.h"
#include "shared/defs.h"
#include "shared/link.h"
#include "shared/misc.h"
#include "shared/front.h"
#include "shared/action.h"
#include "db/mem.h"
#include "db/field.h"
#include "db/type_struc.h"
#include "path.h"
#include "ana_data.h"
#include "ana_expr.h"
#include "ana_branch.h"
#include "ana_local.h"
#include "ana_ptr.h"
#include "info_dc.h"
#include "interface/IADCGui.h"
#include "expr.h"
#include "expr_ptr.h"
#include "proj_ex.h"
#include "arglist.h"
#include "clean_ex.h"
#include "flow.h"
#include "cc.h"

#ifdef OPTEST
uint32_t Op_t::nOpsTotal = 0;
#endif

//MEMTRACE0_IMPL(Op_t);


/*uint8_t Level2Ptr(int nLevel, int ptrsz)
{
	if (!ptrsz)
		ptrsz = G DC.PtrSize();
	assert(ptrsz && ptrsz <= 8);

	uint8_t t(OPTYP _PTR | ptrsz);
	t &= ~0x70;
	t |= (nLevel - 1) << 4;
	return t;
}*/


//////////////////////////Ins0_t


Ins0_t::Ins0_t()
	: mVA(0),
#ifdef _DEBUG
	mOff(-1),
#endif
	mAction(ACTN_NULL),
	mPStackIn(0),
	mPStackDiff(0),
	mFStackIn(0),
	mFStackDiff(0),
	mFlags(0),
//	opnd_NOTANLZ = 1;
	mEFlagsTested(0),
	mEFlagsModified(0),
	mFFlagsAffected(0),
	mpPath(HPATH()),
	mpPathRef(HPATH()),
	mpPrimeOp(HOP())
{
}

Ins0_t::~Ins0_t()
{
	assert(!mpPathRef);
	assert(mArgs.empty());
	//KillArgs();
}

/*void Ins_t::KillArgs()
{
	mArgs.clear(G_ MemMgr);
	//while (mArgs)
		//delete m_pArgs;
}*/

void FuncInfo_t::InitFrom(Ins_t &dst, const Ins_t &src, HOP pSrcOp) const
{
	dst.setVA(src.VA());
	dst.mAction = src.mAction;
	dst.mFStackIn = src.mFStackIn;
	dst.mFStackDiff = src.mFStackDiff;
	dst.mFlags = src.mFlags;
	
	dst.mPStackIn = src.mPStackIn;
	dst.mPStackDiff = src.mPStackDiff;
	
	dst.mEFlagsTested = src.mEFlagsTested;
	dst.mEFlagsModified = src.mEFlagsModified;
	dst.mFFlagsAffected = src.mFFlagsAffected;

	//?pRI->mpPath
	assert(!src.mpPathRef);
	
	for (OpList_t::Iterator i(src.args()); i; i++)
	{
		HOP pOp = NewOp();
		InitFrom(pOp, i.data());
		pOp->setInsPtr(pSrcOp->insPtr());
		dst.args().LinkTail(pOp);
	}
}


/*int Ins_t::SetupTo(INS_t *p)
{
	p->action = mAction;
	if (mPStackDiff % G DC.stackAddrSize())
		return 0;
	p->stackdiff = mPStackDiff / G DC.stackAddrSize();
	p->cpuflags_t = mEFlagsTested;
	p->cpuflags = mEFlagsModified;
	if (mFStackDiff % OPSZ_QWORD)
		return 0;
	p->fpudiff = mFStackDiff / OPSZ_QWORD;
	p->mFFlagsAffected = mFFlagsAffected;
	return 1;
}*/

/*int Op_t::SetupTo(OPND_t *p)
{
	uint8_t id;
	if (!Offset2Id(m_opc, OpType(), mOffs, id ))
	{
		assert(false);
	}
	if (id)
		return 0;

//	p->Setup(m_opc, optyp, id);
//	p->disp = m_disp;
//	p->opseg = opseg;
	
	return 1;
}*/


void FuncInfo_t::LinkOpAfter(HOP pOp, HOP pOpPr) const
{
	HPATH pPath(PathOf(pOpPr));
	assert(pPath);
	LinkOpAfter(pPath, pOp, pOpPr);
}

void FuncInfo_t::LinkOpBefore(HOP pOp, HOP pOpNx) const
{
	HPATH pPath(PathOf(pOpNx));
	assert(pPath);
	LinkOpBefore(pPath, pOp, pOpNx);
}




#ifdef _DEBUG
static int g_ops_created = 0;
static int g_ops_destroyed = 0;
#endif
void print_ops_stats()
{
#ifdef _DEBUG
	std::cout << "ops_created = " << g_ops_created 
		<< " ops_alive = " << g_ops_created - g_ops_destroyed
		<< std::endl;
#endif
}



/////////////////
// O p _ t //////


Op_t::Op_t()
{
	Clear();
CHECKID(this, 0xda0)
STOP
#ifdef _DEBUG
g_ops_created++;
#endif
}

#if(!NO_OBJ_ID)
Op_t::Op_t(int id)
	: OpBase_t(id)
{
	Clear();
#ifdef _DEBUG
g_ops_created++;
#endif
}
#endif

Op_t::~Op_t()
{
	//?assert(!isLinked());
	//assert(!IsPrimeOp());
	assert(m_xin.empty());
	assert(m_xout.empty());
#ifdef OLD_FIELDREF
	assert(!localRef());
#endif
	//?assert(!m_pRoot);
	assert(!m_pRI);
#ifdef _DEBUG
g_ops_destroyed++;
m_nFlags = -1;
#endif

//MEMTRACE_RELEASE;
}

OpType_t DcInfo_t::PtrSize() const
{
	return PrimeSeg()->typeSeg()->ptrSize();
}

FieldPtr  FuncInfo_t::fieldRefEx(CHOP pSelf) const
{
	return LocalRef(pSelf);
	FieldPtr pField(pSelf->localRef());
	if (!pField)
	{
		if (IsRhsOp(pSelf))
		{
			for (XOpList_t::Iterator j(pSelf->m_xin); j; j++)
			{
				HOP pOpIn(j.data());
				pField = pOpIn->localRef();
				if (pField)
					break;
			}
		}
	}
	return pField;
}

/*FieldPtr  FuncInfo_t::LabelRef(CHOP pSelf)
{
	assert(0);
	return 0;//pSelf->labelRef();
}*/

/*void FuncInfo_t::SetLabelRef(HOP pOp, FieldPtr pLabel)
{
	//Ins_t &ri(InsRef(pOp));
	assert(!pLabel || !LabelRef(pOp));
	pOp->setLabelRef(pLabel);
}*/

FieldPtr FuncInfo_t::FindFieldRef(CHOP pSelf) const
{
	FieldPtr pFieldRef(pSelf->localRef());
	if (pFieldRef)
		return pFieldRef;

	Locus_t aLoc;
	if (pSelf->SSID() == SSID_GLOBAL)
	{
		pFieldRef = FindFieldInSubsegs(PrimeSeg(), pSelf->m_disp, aLoc);
		if (pFieldRef)
		{
			std::list<Frame_t>::reverse_iterator i(aLoc.rbegin());
			if ((*i).cont()->typeProc())
			{
				ADDR addr((*i).addr());
				if (++i != aLoc.rend())
				{
					if ((*i).addr() == addr)
						pFieldRef = (*i).field();
				}
			}
		}
	}
	else if (pSelf->IsIndirectB() && IsGoto(pSelf))
	{
		pFieldRef = FindFieldInSubsegs(PrimeSeg(), pSelf->m_disp, aLoc);
	}
	else
		return nullptr;

	//assert(pFieldRef == aLoc.field());
	unsigned off;
	pFieldRef = aLoc.stripToSeg(off);
	if (!pFieldRef || off != 0)
		return nullptr;
/*?	else if (PrimeOp(pSelf) && IsPrimeOp(PrimeOp(pSelf)))//no fieldrefs for tmp or invalid ops!
	{
		//if (pSelf->IsOnTop())
		{
			LocalsTracer_t an(mrFuncDef);
			pFieldRef = an.GetAttachData(pSelf);
		}
	}

	if (!pFieldRef)
	{
		if (pSelf->m_disp == 0)//is it a jump to pseudo exit label?
		{
			if (pSelf->IsGoto() && IsAddr(pSelf))
			{
				HPATH pTail(mrPathTree.pathTail());
				assert(pTail->Type() == BLK_EXIT);
				FieldPtr pExitLabel(pTail->m.Lab elAt());
				if (pExitLabel)
				{
					for (XFieldList_t::Iterator i(pExitLabel->xre fs()); i; i++)
					{
						HOP pOp(i.data().op());
						if (pOp == pSelf)
						{
							pFieldRef = pExitLabel;
							break;
						}
					}
				}
			}
		}
	}*/

	return pFieldRef;
}

FieldPtr FuncInfo_t::GetAttachedLocalFromOp(CHOP pSelf) const
{
	if (!pSelf->XIn())
		return 0;
	if (pSelf->isCPUSW() || pSelf->OpC() == OPC_FPUSW)
		return 0;
	HOP pOp = pSelf->XIn()->data();
//	if (!pOp->IsArgOp())
//		if (!pOp->IsRhsOp())
//			if (!pOp->IsCallOutOp())
//				return 0;
	FieldPtr pField(LocalRef(pOp));
	if (!pField || !IsLocal0(pField))
		return 0;
	return pField;
}

#if(0)
void Op_t::GetType(TYPE _t &t) const
{
	t.m_optyp = OpType();
/*?	if (isPtr())
	if (OpSize() == G DC.PtrSizeEx())
		t.m_ptrmask = 1;*/
}

void Op_t::SetType(TYPE _t &t)
{
CHECK(this == (HOP)0x00951518)
STOP
	SetOpType(t.OpType());
}
#endif

void Op_t::Clear()
{
#ifdef _DEBUG
	__isPrime = false;
#endif

	m_nFlags = 0;

	m_pRI = 0;
	setInsPtr(nullptr);
//	SetNo(-1);

	m_opc = OPC_NULL;

	m_optyp = OPTYP_NULL;
	mOffs = 0;

	m_xin = nullptr;
	m_xout = nullptr;

	//m_refs = nullptr;
	//m_pVarOp = nullptr;

	//SetO pSeg(OPSEG_NULL0);
#ifdef OLD_FIELDREF
	setLocalRef0(nullptr);
#endif

	//m_seg = 0;
	m_disp = 0;
	m_disp0 = 0;

#if(!NEW_OP_TRACER)
	mtr.trace = 0;
#endif
}

void Op_t::setAction(Action_t a){ ins().mAction = a; }

int Op_t::Size() const
{
/*?	if (IsDataOp())
	{
		assert(0);
		//?return ((HOP)this)->Size();
	}*/

	return OpSize();
}

bool Op_t::IsIndirectEx() const
{
	if (IsIndirectB() && SSID() == (SSID_t)OPC_AUXREG)
	if (mOffs == OFS(R_W))
		return true;
	return false;
}

/*HOP Op_t::NextRoot()
{
	assert(IsPrimeOp());

	HOP pOp = this; 
	do {
//		if (pOp->IsLast())
//			return 0;
		pOp = pOp->NextEx();
	} while (pOp && (!pOp->IsRootEx() || pOp->isHidden()));

	return pOp;
}

HOP Op_t::PrevRoot()
{
	assert(IsPrimeOp());

	HOP pOp = this;
	do {
		pOp = pOp->PrevEx();
	} while (pOp && (!pOp->IsRootEx() || pOp->isHidden()));

	return pOp;
}*/

void Op_t::GetVALUE(VALUE_t &v)
{
	v.typ = OpType();
	v.i64 = 0;
	v.i32 = m_disp;
}

/*HOP Op_t::GetCalloutNext() const
{
	assert(IsCallOutOp());
	HOP pOpCall = pri meOp();
	
	for (XOpList_t::Iterator i(pOpCall->m_xout); i; i++)
	{
		if (i.data() == this)
		{
			if (++i)
				return i.data();
		}
	}
	return 0;
}*/

HOP FuncInfo_t::AddOpArg(CHOP pSelf, CHOP pOp) const
{
	assert(pOp);
	assert(IsPrimeOp(pSelf));
	pSelf->args().LinkTail(pOp);
	pOp->setInsPtr(pSelf->insPtr());
	return pOp;
}

HOP FuncInfo_t::AddCallArg(HOP pSelf, HOP pOp) const
{
	assert(IsCall(pSelf));
	return AddOpArg(pSelf, pOp);
}

XOpLink_t * FuncInfo_t::InsertXDep(XOpList_t &l, HOP pOp) const
{
//CHECKID(pOp, 0x12b0)
//STOP
#if(NEW_OP_PTR)
	XOpLink_t *pr(l.lower_bound(pOp));
	if (pr && pr->data() == pOp)
		return nullptr;//no duplicates
	XOpLink_t *pLink(NewXOpLink2(pOp));
	l.insert(pLink, pr);
#else
	if (l.findz(pOp))
		return nullptr;//no duplicates
	XOpLink_t *pLink(NewXOpLink2(pOp));
	l.push_back(pLink);
#endif
	return pLink;
}

bool FuncInfo_t::InsertXDep(XOpList_t &l, HOP pOp, XOpLink_t *pLink) const
{
	assert(pLink);
#if(NEW_OP_PTR)
	XOpLink_t *pr(l.lower_bound(pOp));
	assert(!pr || pr->data() < pOp);//no duplicates
	l.insert(pLink, pr);
#else
	assert(!l.findz(pOp));//no duplicates
	l.push_back(pLink);
#endif
	pLink->setData(pOp);
	return pLink;
}

bool FuncInfo_t::MakeUDLink(HOP pOpU, HOP pOpD) const
{
CHECK(OpNo(pOpU) == 57)
STOP
CHECKID(pOpD, 18781)
STOP
//CHECK(pOpD->Path()->Type() == BLK_ENTER)
//STOP
	assert(pOpD);
	XOpLink_t *pLink(InsertXDep(pOpU->m_xin, pOpD));//doubled
	if (pLink)
	{
		if (InsertXDep(pOpD->m_xout, pOpU, &pLink[1]))
			return true;
		assert(0);
	}
	assert(!InsertXDep(pOpD->m_xout, pOpU));
	return false;
}

XOpLink_t *FuncInfo_t::TakeXDep(XOpList_t &l, OpPtr pOp) const
{
#if(NEW_OP_PTR)
	XOpLink_t *pr(nullptr);
	for (XOpLink_t *p(l.head()); p; pr = p, p = l.next(p))
	{
		if (!(p->data() < pOp))
		{
			if (p->data() == pOp)
				return l.take(p, pr);
			break;
		}
	}
#else
	for (XOpList_t::Iterator i(l); i; i++)
	{
		if (i.data() == pOp)
			return l.take(i);
	}
#endif
	return nullptr;
}



bool FuncInfo_t::ReleaseXDepIn(HOP pSelf, OpPtr pOp) const
{
//CHECKID(pSelf, 0x143d)
//STOP
	XOpLink_t *pLink(TakeXDep(pSelf->m_xin, pOp));
	if (!pLink)
		return false;
	memMgrEx().Delete(pLink);
	return true;
}

bool FuncInfo_t::ReleaseXDepOut(HOP pSelf, OpPtr pOp) const
{
//CHECKID(pSelf, 0x143d)
//STOP
	if (TakeXDep(pSelf->m_xout, pOp))
		return true;
	return false;
}

/*void FuncInfo_t::RedirectXDepIn(HOP pSelf, HOP pOpFrom, HOP pOpTo)
{
	ReleaseXDepIn(pSelf, pOpFrom);
	ReleaseXDepOut(pOpFrom, pSelf);
	MakeUDLink(pSelf, pOpTo);
}*/

typedef XOpLink_t zz_t[2];
bool FuncInfo_t::UnmakeUDLink(HOP hUse, HOP hDef) const
{
	XOpLink_t *pLink(TakeXDep(hUse->m_xin, hDef));
	if (!pLink)
		return false;
	XOpLink_t *pLink2(TakeXDep(hDef->m_xout, hUse));
	assert(pLink2 == &pLink[1]);
	memMgrEx().Delete2(pLink);

	/*if (!ReleaseXDepIn(hUse, hDef))
		return false;
	if (!ReleaseXDepOut(hDef, hUse))
		ASSERT0;*/
	return true;

}

void FuncInfo_t::RedirectXIns(HOP pSelf, HOP pOpTo) const
{
	if (pOpTo == pSelf)
		return;

	while (!pSelf->m_xin.empty())
	{
		HOP hDef(pSelf->m_xin.head()->data());
		UnmakeUDLink(pSelf, hDef);
		MakeUDLink(pOpTo, hDef);
	}
}

/*int Op_t::GetOpInNum()
{
	int n = args().count();
	assert(n < 0x100);
	return n;
}*/


void Op_t::clearInsPtr()
{
	if (m_pRI)
	{
#if(NEW_OP)
		if (IsPrimeOp())
			m_pRI->args().Unlink(li nked());
		assert(m_pRI->args().empty());
#endif
		m_pRI = nullptr;
	}
}

//OpList_t &Op_t::args() const { return ins().m_ops; }

bool Op_t::EqualTo(CFieldPtr p)
{
	if (OpC() != (OPC_t)FuncInfo_s::SSIDx(p))
		return false;
	if (Offset0() != FuncInfo_s::address(p))
		return false;
	if (OpSize() != p->size())
		return false;	

	return true;
}


/*int8_t Op_t::FPURegID()
{return OpOffs();
	assert(OpC() == OPC_FPUREG);

	if (!IsCode())//Op())
	{
		return OpOffs();
	}

	if (IsCallOutOp())
		return prime Op()->FpuOut() + OpOffs();

	int8_t fputop = prim eOp()->FpuTop();
	return fputop + OpOffs();
}*/

/*int8_t Op_t::FpuTop()
{
	int8_t fputop = FpuIn();
	if (fstackDiff() < 0)//load
		fputop += fstackDiff();
	return fputop;
}*/

int8_t FuncInfo_t::FpuOut(CHOP hSelf) const
{
	assert(IsPrimeOp(hSelf));
	int8_t fpuout = hSelf->ins().FStackIn() + hSelf->ins().FStackDiff();
	return fpuout;
}

/*static uint16_t CPUFID2MASK(uint8_t id)
{
	static const uint16_t eflags[] = { F_CF, F_PF, F_AF, F_ZF, F_SF, F_IF, F_DF, F_OF };
	if (id > 0)
		if (id <= 8)
			return eflags[id - 1];
	assert(false);
	return 0;
}*/

const STORAGE_t &FuncInfo_t::StorageOf(CHOP pSelf) const 
{
	return mrFE.STORAGE[pSelf->SSID()];
}

uint32_t FuncInfo_t::CPUFlags(CHOP pSelf) const
{
	if (!(pSelf->OpC() & OPC_FLAGS))
	{
		assert(mrFE.STORAGE[pSelf->SSID()].isFlag());
		{
			uint32_t flags(BITMASK(pSelf->OpSize()) << pSelf->OpOffsU());
			return flags;
		}
	}
	assert(pSelf->m_disp == 0);
	uint32_t flags;
	if (IsRhsOp(pSelf))
		flags = InsRefPrime(pSelf).mEFlagsTested;
	else
		flags = InsRefPrime(pSelf).mEFlagsModified;

	return flags;
}

uint32_t FuncInfo_t::FPUFlags(CHOP pSelf) const
{
	if (!(pSelf->OpC() & OPC_FLAGS))
	{
		assert(mrFE.STORAGE[pSelf->SSID()].isFlag());
		{
			uint32_t flags(BITMASK(pSelf->OpSize()) << pSelf->OpOffsU());
			return flags;
		}
	}

	/*if (pSelf->OpC() == OPC_FPUSW)
	{
		//if (pSelf->OpOffsU())
		if (pSelf->OpSize() == 0)//not flags - a reg!
		{
			REG_t R(mrDC.fromReg(OPC_FPUSW, pSelf->OpOffsU()));
			uint32_t flags(BITMASK(R.m_siz) << R.m_ofs);
			return flags;
		}
	}*/

	int flags;
	if (IsRhsOp(pSelf))
		flags = FPUSW_C0 | FPUSW_C1 | FPUSW_C2 | FPUSW_C3;
	else
		flags = InsRefPrime(pSelf).mFFlagsAffected;

	return flags;
}

int Op_t::checkThisPtr()
{/*?
	HOP pOp = this;
	while (1)
	{
		if (!pOp->isPtr())
			return 0;
		
		if (!pOp->IsCodeOp())
			break;

		if (pOp->IsPr imeOp())
		{
			if (pOp->Action() != ACTN_MOV)
				return 0;
			if (pOp->isRoot())
				return 0;
			pOp = pOp->GetA rgs();
		}
		else
		{
			if (pOp->XIn()->CheckCount(1) != 0)
				return 0;
			pOp = pOp->XIn()->pOp;
		}
	
	}

	if (pOp->IsT hisPtr())
		return 1;*/

	return 0;
}

void FuncInfo_t::ClearXIns(HOP pSelf) const
{
#if(1)
	while (pSelf->XIn())
	{
		HOP pOp = pSelf->XIn()->data();
		UnmakeUDLink(pSelf, pOp);
		//ReleaseXDepIn(pSelf, pOp);
		//ReleaseXDepOut(pOp, pSelf);
	}
#else
	while (!pSelf->m_xin.empty())
	{
		XOpList_t::Iterator i(pSelf->m_xin[0]->m_xout);
		do {
			if (i.data() == pSelf)
			{
				//delete pXDep;
				memMgrEx().Delete(pSelf->m_xin[0]->m_xout.take(i));
				
				if (pSelf->m_xin[0]->m_xout.empty()//last out xdep killed
					&& (!pSelf->m_xin[0]->isLinked()))//not linked somewhere
					delete pSelf->m_xin[0];//kill recursive
				break;
			}
			i++;
		} while (i);
		memMgrEx().Delete(pSelf->m_xin.take(pSelf->m_xin.head()));
	}
#endif
}

bool FuncInfo_t::CheckLValueVisibility(CHOP hSelf) const
{
	assert(IsPrimeOp(hSelf));
	assert(!hSelf->isRoot());
	if (hSelf->m_xout.check_count(1) == 0)
	{
		if (IsCondJump(PrimeOp(hSelf->XOut()->data())))
		if (hSelf->IsIndirect())
		{
			//assert(false);
			return false;
		}
	}
	return true;
}

/*int Op_t::IsThisPtrAtCall()
{
	assert(IsCallArg());

	FuncDef_t *pfDef = pr imeOp()->GetFuncDef();
	if (pfDef->m_pThisPtr)
		if (EqualTo(pfDef->m_pThisPtr))
			return true;
	return 0;
}*/



////////////////////////////////////////////////////////////


bool Op_t::isCombined() const
{return false;/*!
	if (!fieldRef())
		return false;
	if (!fieldRef()->IsL ocal0())
		return false;
	if (fieldRef() != this)
		return false;
	return true;*/
}

int Op_t::Offset0() const
{
	int opc = OpC();// & 0xF;
	if (opc == OPC_CPUREG)
	{
		return OpOffsU();
	}
	
	if (opc == OPC_FPUREG)
	{
		return OpOffs();//FPURegID();
		//return id<<3;
	}
	
	if (opc == OPC_AUXREG)
	{
		return OpOffsU();
	}
	
	if (opc == OPC_SEGREG)
	{
		return OpOffsU();
	}
	
	if ((opc & SSID_MASK) == OPC_LOCAL)
	{
		return m_disp;
	}

//	if (IsScalar())
//	{
//		return 0;
//	}
	

/*	if (!IsIndirect())
	{
		if (!IsAddr())
			return 0;
	}

	if (m_pData)
	{
		int d_offs = m_pData->Offset();
		if (d_offs == BAD_ADDR)
			return d_offs;
	}*/

	return m_disp;
}


//generate func's name for indirect calls
/*char *Op_t::MakeIndirCallName()
{
	assert( IsPrimeOp() );
	assert( IsCall());
	assert( (OpC() == OPC_CPUREG) || IsIndirect() );

	static char buf[NAMELENMAX+1];
	HPATH pPath = Path();
	if (1)
	{
		FieldPtr pLabel = 0;
		while (pPath) 
		{
			if (pPath->Label())
			{
				pLabel = pPath->Label();
				break;
			}
			pPath = pPath->PrevEx();
		}

		if (!pLabel)
			pLabel = GetOwner Func();

		pLabel->nam ex(buf);
	}
	else
	{
		GetOwner Func()->na mex(buf);
	}

	int n = strlen(buf);
	int n_max = NAMELENMAX-1-10-1;
	if (n > n_max)
		n = n_max;
	strcpy(buf+n, "#");
	ltoa(No(), buf+strlen(buf), 10);

	return buf;
}*/

/*
int Op_t::InvalidateGoto()
{
	if (Set Hidden(1) == -1)
		return -1;
	//SetPathType(Path(), BLK_NULL);

	HPATH pPath = Path();
	HPATH pPathNx = pPath->NextEx();
	assert(pPathNx);
#if(0)
	HOP pOp = this;
	do {
		HOP pOpPr = pOp->Pr ev();
		pOp->SetPath(pPathNx);//, pPathNx->m_pOps = pOp;
		pOp = pOpPr;
	} while (pPath->IsMineOp(pOp));
	delete pPath;
#else
	if (0)
	if (pPath->IsDead_())
	{
		assert(pPath->Parent()->m_pChilds != pPath);
		pPath->Unlink();
		pPath->LinkHead((Link_t **)&pPathNx->m_pChilds);
		pPath->SetParent(pPathNx);
	}
#endif
	PathType(pPath) = BLK_NULL;
	return 1;
}


int Op_t::Adjust Goto()
{
	assert(IsPrimeOp());
	if (!IsAddr())
		return 0;
	if (IsRetOp())
		return 0;
	if (!IsGoto())
		if (!IsCondJump())
			return 0;

	int res = 0;
	FieldPtr pLabelNew = __traceGoto();//new goto destination
	if (pLabelNew->Ops() == NextEx())//NextRoot())
	{
		res = InvalidateGoto();//jump to next instruction
		if (res != 1)
			res = 0;
	}

	if (pLabelNew != m_pGoto)
	{
		m_pGoto->ReleaseXRef(this);
		pLabelNew->AddXRef(this);//new destination
		res = 1;
	}

	if (!IsGoto())
		return res;

	//this <=> unconditional jumps only
	if (Label())
	{//redirect jumps from this goto op to m_pGoto
		FieldPtr pLabel = Label();
		XRef_t *pXRef = pLabel->m_pXRefs;
		while (pXRef)
		{
			XRef_t *pXRefNx = pXRef->Nex t();
			HOP pOp = pXRef->pOp;
			pLabel->ReleaseXRef(pOp);
			m_pGoto->AddXRef(pOp);
			pXRef = pXRefNx;
			res = 1;
		}
		
		assert(!Label()->m_pXRefs);//no more xref to this op!
//		res = 1;
	}

	if (IsFirstEx())
		return res;

	HOP pOpPr = PrevEx();//PrevRoot();
	if ( !pOpPr->IsCondJump() )
	{
		if (pOpPr->IsGoto() && IsAddr(pOpPr))
		{//prev op is goto! so invalidate this one
			int r = InvalidateGoto();
			if (r == 1)
				res = r;
			r = pOpPr->AdjustGoto();//?
			if (r == 1)
				res = r;
		}
		return res;
	}

	//pOpPr <=> conditional jumps only
	FieldPtr pGotoPr = pOpPr->m_pGoto;
	if ( pGotoPr->Ops() != NextEx() )//NextRoot() )
	{
		return res;
	}

	pGotoPr->ReleaseXRef(pOpPr);
	__traceGoto()->AddXRef(pOpPr);
	m_pGoto->ReleaseXRef(this);
	pGotoPr->AddXRef(this);

	pOpPr->InvertAction();
	InvalidateGoto();
	res = 2;
	return res;
}
*/


bool Op_t::CheckScatteredDependency() const
{
#if(1)
	return (m_xin.check_count(1) > 0);
#else
	int iCount(0);
	for (XOpList_t::Iterator i(m_xin); i; i++)
	{
		HOP pOp(i.data());
		if (pOp->pri meOp()->isRoot())
			return false;
		if (pOp->SSID() != SSID())
			return false;
	}
	return (iCount > 1);
#endif
}

bool Op_t::CheckConditionDependency() const
{
	return false;
	if (OpC() == (OPC_t)SSID_CPUSW)
		return true;
#if(0)
	for (XOpList_t::Iterator j(m_xout); j; j++)
	{
		HOP pOp1(j.data());
		if (pOp1->SSID() == SSID_CPUSW)
		{
			//if (m_xout.check_count(1) == 0)
				return true;
		}
		/*{
				HOP pOp2(m_xout.head()->data());
				for (XOpList_t::Iterator i(pOp2->m_xin); i; i++)
				{
					HOP pOp2(i.data());
					if (pOp2 == this)
						return (pOp2->SSID() == SSID_CPUSW);
				}
		}*/
	}
#endif
	return false;
}

int FuncTracer_t::SetShown(HOP pSelf) const
{
	assert(IsCode(pSelf));
	assert(IsPrimeOp(pSelf));

	if (!pSelf->isHidden())
		return -1;//already

	pSelf->ins().opnd_HIDDEN = 0;

	ExprCacheEx_t aExprCache(PtrSize());
	AnlzXDepsIn_t anlzxdepsin(*this, pSelf, aExprCache);
	anlzxdepsin.TraceXDepsIn();
	//		TraceXDepsIn();

	//if (!(mrFuncDefRef.flags() & FDEF_REANLZ))
	if (ProtoInfo_t::IsFuncStatusFinished(&mrFuncDefRef))
		SetRoot(pSelf, false);//TurnRoot_Off(true);
	SetChangeInfo(FUI_ROOTS | FUI_LOCALS);
	//		CheckLocals0();//Ex();
	return 1;
}

int FuncTracer_t::SetHidden(HOP pSelf) const
{
	assert(0);
#if(0)
	assert(IsCode(pSelf));
	assert(IsPrimeOp(pSelf));
//CHECK(No() == 2836)
//STOP

//	if (isHidden())
//		return -1;//already

	for (OpList_t::Iterator iOpIn(pSelf->argsIt()); iOpIn;  iOpIn++)
	{
		while (1)
		{
			HOP pOp = HOP();
			if (iOpIn.data()->XIn())
				pOp = iOpIn.data()->XIn()->data();
			if (!pOp)
				break;

			ReleaseXDepOut(pOp, iOpIn.data());
			ReleaseXDepIn(iOpIn.data(), pOp);

			if (IsCode(pOp))
			{
				HOP  pOpR = PrimeOp(pOp);
				//AnlzX DepsIn_t anlzxdepsin(*this, pOpR);
				//anlzxdepsin.CheckExtraXOut();
				CheckHidden(pOpR);
			}

			if (!pOp->XOut())
			{
				if (IsCodeOp(pOp))
				{
					if (!pSelf->isRoot())
					{
						AnlzRoots_t anlz(*this, pSelf, true);
						anlz.TurnRoot_On();
					}
					//pOp->SetRoot(true);
				}
				else if (IsCallOutOp(pOp))
				{
					FuncCleaner_t h(*this);
					mrFuncDef.mCallRets.Unlink(pOp);
					h.DestroyOp(pOp);
				}
				else if (IsArgOp(pOp))
				{
//?					if (pOp->m_nFlags & OPND_NCONF)
					{
//?						FieldPtr  pData = pOp->GetOwnerData();
//?						if (pData && pData->IsLoc alEx())
//?							delete pData;//->Delete();

						/*?FieldPtr pLocalRef(fieldRef(pOp));
						if (pLocalRef)
						{
//CHECKID(pLocalRef,2616)
//STOP
							//HPATH pPath(pOp->Path());// pLocalRef->GetOwner Path());
							//assert(pPath && pPath->Type() == BLK_ENTER && pOp->Path() == pPath);
							//if (!pPath)//?
							//	pPath = PathHead();

							LocalsTracer_t an(*this);
							if (!an.TakeLocal(pLocalRef))
							{
								assert(0);
							}
							pLocalRef->destruct(mrMemMgr);
							mrMemMgr.Delete(pLocalRef);
						}*/

						FuncCleaner_t h(*this);
						h.DestroyPrimeOp(pOp);
					}
				}
			}
		}
	}

	pSelf->ins().opnd_HIDDEN = 1;
	LocalsTracer_t an(*this);
	an.CheckLocals0(pSelf);//Ex();
#endif
	return 1;
}

int HiddenTracer_t::CheckHidden0(CHOP hSelf)
{
//CHECKID(hSelf, 2836)
CHECK(OpNo(hSelf) == 108)
STOP

	if (!IsCode(hSelf))
		return 0;
	
	assert(!IsCallOutOp(hSelf));
		//return 1;

	assert(IsPrimeOp(hSelf));
//CHECK(hSelf->No() == 8)
//STOP
	if (ActionOf(hSelf) == ACTN_INVALID)
		return 0;

	assert(!IsVarOp(hSelf));

	//		if (IsDead())
	//			return 0;
	if (ISCALL(ActionOf(hSelf)))
		return 0;

	if (ISINTRINSIC(ActionOf(hSelf)))
	{
		if (!hSelf->OpC())
			return 0;
	}

	if (IsGoto(hSelf))
		return 0;
	if (IsCondJump(hSelf))
		return 0;
	if (IsRetOp(hSelf))
		return 0;
	//		if (Action() == ACTN_MOV)
	//			if (EqualTo(&arg1()))
	//			{
	//				Set Hidden(1);
	//				return 1;
	//			}

	if (hSelf->IsIndirect())
	{
		if (!IsLocalOp(hSelf))
			return 0;
		if (LocalRef(hSelf) && LocalRef(hSelf)->IsComplexOrArray())
		{
			SetShown(hSelf);
			return 0;
		}
	}

	if (hSelf->ins().opnd_BAD_STACKOUT)
		return 0;


	if (hSelf->XOut())
	{
		if (!hSelf->isHidden())
			return 0;
	}

	if (hSelf->m_nFlags & OPND_XOUTEX)
	{
		SetShown(hSelf);
		return 0;
	}

#if(0)
	for (OpList_t::Iterator i(hSelf->argsIt()); i; ++i)
	{
		HOP hRhsOp(i.data());
		for (XOpList_t::Iterator j(hRhsOp->m_xin); j; ++j)
		{
			HOP hDefOp(j.data());
			if (IsArgOp(hDefOp) && LocalRef(hDefOp))
			{
				if (hSelf->isIndirect())//a local
				{
					//a hack for situations like this:
					//	push ecx
					//	fstp dword ptr [esp]
					HOP hOpNx(NextPrime(hSelf));
					if (!hOpNx || !EqualTo(hOpNx, hSelf))
						return 0;
				}
			}
		}
	}
#endif

	//SetHidden(hSelf);
	return 1;
}

void HiddenTracer_t::CheckHidden(CHOP pOp0, FuncCleaner_t &FD)
{
	assert(IsPrimeOp(pOp0));

	ExprCacheEx_t aExprCache(PtrSize());

	for (HOP pOp(pOp0); pOp; pOp = FD.TakeOrphanOp())
	{
		if (!IsDataOp(pOp))
		{
			AnlzXDepsIn_t an(*this, pOp, aExprCache);
			an.CheckExtraXOut();

			if (CheckHidden0(pOp))
			{
				if (IsVarOp(pOp))
				{
					FD.DestroyPrimeOp(pOp);
				}
				else
				{
					FD.DisconnectOp(pOp, false);
					pOp->ins().opnd_HIDDEN = 1;
				}
			}
			else if (!pOp->XOut())
			{
				if (IsCodeOp(pOp))
				{
					if (!pOp->isRoot())
					{
						AnlzRoots_t an(*this, pOp, true);
						an.TurnRoot_On();
					}
				}
			}
		}
	}
}

/*int Op_t::isInMineBlock(HOP pOp)//returns 1 if pOp is in the same block with this
{
	assert(Path() && pOp->Path());
	HPATH pPath = pOp->Path()->Parent();
	while (1)
	{
		if (pPath == Path()->Parent())
			return 1;
		if (!pPath)
			break;
		pPath = pPath->Parent();
	}

	return 0;
}*/


/////////////////////
// s p l i t t i n g

int Op_t::Displace(int d)
{
	assert(d != 0);
	if (IsIndirect())
	{
		m_disp0 += d;
		m_disp += d;
	}
	else if (IsScalar())
	{
		//?assert(abs(d) < sizeof(ui64));
		if (d > 0)
			(*(uint32_t*)&m_disp) >>= d;//?ui64 >>= d;
		else
			(*(uint32_t*)&m_disp) <<= -d;//?ui64 <<= -d;
	}
	else
		mOffs += d;
	return 1;
}

/*#if(0)
int Op_t::CanSplit(HOP *pOpStart)
{
	assert(IsPrimeOp());
	assert(!isSplit());

//	if (opnd_BLOCK)
//		return 0;
	if (!isRoot())
		return 0;

	HOP pOp = this;
	while (1) 
	{
		*pOpStart = pOp;
		if (pOp->Label())
			if (pOp->Label()->CountGotoXRefs() > 0)
				return 1;
		pOp = pOp->Pr ev();
		if (pOp->Path() != Path())
			break;
		if (!pOp->isHidden())
			if (pOp->isRoot())
			{
				if (pOp->isSplit())
					return 1;
				else
					break;
			}
	}

	*pOpStart = 0;
	return 0;
}

int Op_t::TurnSplitOn()
{
	HOP pOpStart = 0;
	if (!CanSplit(&pOpStart))
		return 0;

	assert(pOpStart);
	HOP pOp = pOpStart;
	do {
		pOp->setSplit(1);
		if (pOp == this)
			return 1;
		pOp = pOp->N ext();
	} while (pOp);

	assert(false);
	return 1;
}

int Op_t::TurnSplitOff()
{
	HOP pOp = this;
	while (1)
	{
		HOP pOpPr = pOp->PrevEx();
		if (!pOpPr || pOpPr->IsRootEx())
			break;
		pOp = pOpPr;
	}

	while (1)
	{
		pOp->setSplit(0);
		if (IsRetOp(pOp))
			break;//logic block end achived
		assert(!pOp->IsLastEx());
		assert(!pOp->IsGoto());

		pOp = pOp->Ne xt();
//		if (!pOp->isHidden())
//			if (pOp->isRoot())
				if (!pOp->isSplit())
					break;
	}

	return 1;
}
#endif*/


/*HOP Op_t::GetThisPtrAtCall()
{
	assert(IsCall());

	FuncDef_t *pfDef = GetFuncDef();
	assert(pfDef->m_pThisPtr);

	for (HOP pArg = Get Args(); pArg; pArg = pArg->Ne xt())
	{ 
		if (pArg->EqualTo(pfDef->m_pThisPtr))
		{
			assert(pArg->isPtr());
			assert(pArg->PtrLevel() == 1);
			return pArg;
		}
	}
//	assert(false);
	return 0;
}*/

/*HOP Op_t::toOp(const ObjFlags_t *pObj)
{
#if(NEW_OP_PTR)
	assert(0);
#else
	if (pObj->objId() == OBJID_OP)
		return reinterpret_cast<HOP>((ObjFlags_t *)pObj);
#endif
	return HOP();
}*/


/*int Op_t::CheckTraceInfo(uint32_t mask, bool bState)
{
	if (IsPrimeOp())
	{
//CHECK(No() == 49)
//STOP

		HOP pOp = lin ked();
		do {
			for (OpList_t::Iterator i(pOp->argsIt()); i; i++)
			{
				HOP pIn(i.data());
				if (pIn->IsPr imeOp())
					continue;
				if (!pIn->CheckTraceInfo(mask, bState))
					return 0;
			}

			if (bState)
			{
				if (!(pOp->trace & mask))
					return 0;
			}
			else
			{
				if (pOp->trace & mask)
					return 0;
			}

			pOp = pOp->pNext->link ed();
		} while (pOp != this);

		return 1;
	}

	if ( (bState && !(trace & mask))
		|| (!bState && (trace & mask)) )
		return 0;

	return 1;
}*/


/////////////////////////////////////////////OpTracer_t
//?int OpTracer_t::s_here = 0;

#if(!NEW_OP_TRACER)
void OpTracer_t::__set(const FuncInfo_t &rFI, OpPtr pSelf)
{
	pSelf->mtr.trace = 0;

	if (rFI.IsPrimeOp(pSelf))
	{
		for (OpList_t::Iterator i(pSelf->argsIt()); i; i++)
			__set(rFI, i.data());
	}

	for (XOpList_t::Iterator i(pSelf->m_xin); i; i++)
	{
		HOP pIn(i.data());
		if (!rFI.IsPrimeOp(pIn))
			__set(rFI, pIn);
	}
}

void OpTracer_t::reset(const FuncInfo_t &rFI)
{
	for (PathTree_t::LeafIterator i(rFI.FuncDef().pathTree().tree()); i; i++)
	{
		HPATH pPath(i.data());
		for (PathOpList_t::Iterator i(pPath->ops()); i; i++)
		{
			__set(rFI, PRIME(i.data()));
		}
	}
}
#endif



/*int Op_t::MoveOpUp0()
{
	if (!IsPrimeOp())
		return 0;
	HOP  pOpPr = P rev();
	if (!pOpPr || pOpPr->IsCall())
		return 0;

	if (!IsStackPtr())
		return 0;

	int sign = 0;
	if (Action() == ACTN_SUB)
		sign = -1;
	else if (Action() == ACTN_ADD)
		sign = 1;
	else
		return 0;

	if (!arg1()->IsStackPtr())
	if (arg1()->IsAddr() && (arg1()->SSID() == SSID_LOCAL))
	{
		if (arg1()->OpOffs() != GF E.stack_ptr->ofs)
			return 0;
	}
	else
		return 0;

	HOP  pOp = &arg2();
	if (!pOp->IsScalar())
		return 0;

	int sign_pr = 0;
	if (pOpPr->Action() == ACTN_SUB)
		sign_pr = 1;
	else if (pOpPr->Action() == ACTN_ADD)
		sign_pr = -1;
	else if (pOpPr->StackDiff())
		if (pOpPr->Action() != ACTN_MOV)
			return 0;

	TypeProc_t * pFunc = ownerFunc();
	RevertLocals();
	pOpPr->RevertLocals();

	bool bMerge = false;
	if (pOpPr->IsStackPtr())
		bMerge = true;

	if (bMerge)
	{
		if (sign_pr)
		{
			SetStackDiff(StackDiff() + pOpPr->StackDiff());
			pOp->m_disp += pOpPr->arg2()->m_disp * sign_pr / (-sign);
			delete pOpPr;

			if (pOp->m_disp == 0)
				delete this;
		}
		else
		{
			assert(!pOpPr->arg1()->IsStackPtr());
			//ins().m_stackdiff += pOpPr->ins().m_stackdiff;
			SetStackDiff(pOp->m_disp + pOpPr->StackDiff());
			HOP  pOpD = arg1()->Replace(&pOpPr->arg1());//replace op
			arg1()->m_pRoot = this;
			delete pOpPr;
			delete pOpD;
		}
	}
	else
	{
		int d = pOp->m_disp * (-sign);

		if (pOpPr->IsStackPtrB())
			pOpPr->m_disp += d;//Displace(d);
		for (HOP  pOpIn = pOpPr->GetA rgs(); 	pOpIn; 	pOpIn = pOpIn->Ne xt())
		{
			if (pOpIn->IsStackPtrB())
				pOpIn->m_disp += d;//Displace(d);
		}
		pOpPr->LinkAfter(this);
	}

	pFunc->SetChangeInfo(FUI_ALL);//FUI_BASE|FUI_SAVREGS|FUI_XOUTS|FUI_CALLOUTS|FUI_LOCALS|FUI_NUMBERS);
	return 1;
}

int Op_t::MoveOpDown0()
{
	if (!IsPrimeOp())
		return 0;
	HOP  pOpNx = Ne xt();
	if (!pOpNx || pOpNx->IsCall())
		return 0;

	return 0;
}*/

#if(0)
int Op_t::InjectFunc()
{
	if (!IsCall())
		return 0;
	if (!IsAddr())
		return 0;

/*	TypeProc_t * pCallee = GetFuncAtCall();
	assert(pCallee);
	if (!pCallee->IsDecompiled())
		return 0;//fixlater

	TypeProc_t * pCaller = GetOw nerFunc();
//	pCaller->UndoPathTree();

//	XRef_t * pXOutSv = XOut();
//	XOut() = 0;
	HOP  pOpNx = NextEx();
	assert(pOpNx);

	FieldPtr  pLabelExit = 0;
//	if (pCallee->tailPath()->Label())
		pLabelExit = pOpNx->Ensure Label();

	for (HPATH  pPath = pFunc->Body()->TreeTerminalFirst();
	pPath;
	pPath = pPath->NextEx())
	{
		if (pPath->Type() == BLK_EXIT)
			break;

		HPATH  pPathN = pPath->Clone();
		Path()->SplitAfter()
	}*/

	return 1;
}
#endif

FieldPtr Op_t::localRef() const
{
	assert(!mpLocalRef || !mpLocalRef->owner() || FuncInfo_s::IsLocal(mpLocalRef));
	return mpLocalRef;
}

void Op_t::setLocalRef0(FieldPtr p)
{
	mpLocalRef = p;
	assert(!mpLocalRef || !mpLocalRef->owner() || FuncInfo_s::IsLocal(mpLocalRef));
}

/*char Op_t::GetFPUStatusChar()
{
	uint8_t _fpu = FpuIn();
	if (_fpu < 8)//ok
		return '0' + (8 - _fpu);
	else if (_fpu == 8)
		return '-';//empty
	else if (_fpu < 0x7F)
		return 'u';//underflow
	else
		return 'o';//overflow
}*/

/*
HOP Op_t::__filterOp()
{
if (!IsCodeOp())
return 0;

if (IsPrimeOp())
{
if (Action() != ACTN_MOV)
return 0;
}
else
{
if (XIn()->CheckCount(1) != 0)
return this;
}

HOP pOp = __filterOp()
}*/


/*HOP Op_t::__FilterDumpOp(XRef_t *pXIns, HPATH pPath_, VALUE_t &v)
{
if (pXIns->CheckCount(1) == 0)
{
HOP pOp = pXIns->data();
//if (pOp->IsArgOp())//?
//	return 0;
return pOp;
}

HPATH pPath = 0;
HOP pDumpOp = 0;
for (XRef_t *pXIn = pXIns; pXIn; pXIn = pXIn->Ne xt())
{
HOP pOp = pXIn->data();
if (pOp->IsCode())
if (!pPath)
pPath = pOp->Path();
if ( pOp->CheckScatteredAssignment(pPath) == 1)
continue;

if (pDumpOp)
return 0;//alredy?
pDumpOp = pOp;
}
return pDumpOp;
}*/

/*
int Op_t::Select(bool bSel)
{
if (!IsCode())
return 0;

if (bSel)
{
if (m_nType & OPND_SEL)
return 0;
}
else
{
if (!(m_nType & OPND_SEL))
return 0;
}

int nCount = 0;
if (bSel)
{
if (!(m_nType & OPND_SEL))
{
m_nType |= OPND_SEL;
nCount++;
}
}
else
{
if (m_nType & OPND_SEL)
{
m_nType &= ~OPND_SEL;
nCount++;
}
}

for (XRef_t *pXIn = XIn(); pXIn; pXIn = pXIn->N ext())
{
HOP pOp = pXIn->pOp;
if (bSel)
{
if (!pOp->IsRootEx())
nCount += pOp->Select(bSel);
}
else if (pOp->m_nFlags & OPND_SEL)
nCount += pOp->Select(bSel);
}

if (IsPrimeOp())
for (HOP pOp = GetA rgs(); pOp; pOp = pOp->N ext())
{
if (bSel)
{
//			if (!pOp->IsRootEx())
nCount += pOp->Select(bSel);
}
else if (pOp->m_nFlags & OPND_SEL)
nCount += pOp->Select(bSel);
}

return nCount;
}*/

void Op_t::MoveArgsTo(OpList_t &l)
{
	while (!args().empty())
	{
		HOP hOp(args().take_front());
		//hOp->setInsPtr(nullptr);
		l.push_back(hOp);
	}

/*#if(!NEW_OP)
	args().move_to(l);
#else
	OpList_t::Iterator i(argsIt());
	while (i)
	{
		OpList_t::Iterator j(i++);
		HOP pOp(j.data());
		args().erase(j);
		l.push_back(pOp);
	}
#endif*/
}
void Ins0_t::mergeFrom(const Ins0_t &o)
{
	if (mVA == -1)
		mVA = o.mVA;
	assert(!mPStackIn || (mPStackIn == o.mPStackIn));
	mPStackDiff += o.mPStackDiff;

	assert(!mFStackIn || (mFStackIn == o.mFStackIn));
	mFStackDiff += o.mFStackDiff;

	mEFlagsTested |= o.mEFlagsTested;
	mEFlagsModified |= o.mEFlagsModified;
	mFFlagsAffected |= o.mFFlagsAffected;
}

bool Ins0_t::destruct(MemoryMgrEx_t &rMemMgr)
{
	while (!mArgs.empty())
	{
		HOP pOp(mArgs.Head());
		mArgs.Unlink(pOp);
		rMemMgr.Delete(pOp);
	}
	return true;
}



