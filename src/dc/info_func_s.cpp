#include "info_func.h"

#include "prefix.h"

#include "shared/defs.h"
#include "shared/link.h"
#include "shared/front.h"
#include "shared/misc.h"
#include "shared/action.h"
#include "db/mem.h"
#include "db/obj.h"
#include "db/type_struc.h"
#include "db/types.h"
#include "db/field.h"
#include "arglist.h"
#include "info_dc.h"
#include "path.h"
#include "op.h"
#include "ana_ptr.h"
#include "ana_data.h"
#include "ana_expr.h"
#include "ana_local.h"
#include "ana_switch.h"
#include "ana_branch.h"
#include "ana_pcode.h"
#include "expr.h"
#include "reg.h"
#include "ui_main_ex.h"
#include "clean_ex.h"
#include "expr_ptr.h"
#include "ana_main.h"
#include "cc.h"
#include "info_class.h"

OpPtr FuncInfo_s::GetFirstOp(CHPATH hSelf)
{
	HPATH pPath(hSelf);
	while (pPath && !pPath->IsTerminal())
		pPath = pPath->FirstChild();

	if (pPath)
		return pPath->headOp();

	return OpPtr();
}

OpPtr FuncInfo_s::GetLastOp(CHPATH hSelf)
{
	HPATH pPath(hSelf);
	if (!pPath->IsTerminal())
		return GetLastOp(pPath->LastChild());

	if (hSelf->ops().empty())
		return OpPtr();

	return PRIME(hSelf->ops().back());
}

void FuncInfo_s::SetLocalRef(HOP pSelf, FieldPtr pField)
{
CHECKID(pSelf, 7075)
STOP
#ifdef OLD_FIELDREF
//	assert(!pField || IsLocalSSID(pField) || pField->typeCode());
	pSelf->setLocalRef0(pField);
#else
#if(0)
	GD C.setGlobalRef(this, pField);
#endif
#endif
}

FieldPtr  FuncInfo_s::LocalRef(CHOP pSelf)
{
#ifdef OLD_FIELDREF
	return pSelf->localRef();
#else
	return FindFieldRef(pSelf);
#endif
}

void FuncInfo_s::SetPathRef(HOP pOp, PathPtr pPath)
{
	//FieldPtr pLabel(PathLabel(pPath));

	assert(!pPath || !pOp->pathRef());
	pOp->setPathRef(pPath);
}

HPATH FuncInfo_s::PathRef(CHOP pSelf)
{
	return pSelf->pathRef();
}

int8_t FuncInfo_s::FpuIn(CHOP hSelf)
{
	assert(IsPrimeOp(hSelf));
	return hSelf->ins().mFStackIn;
}

/*FieldPtr FuncInfo_s::DockField(CGlobPtr iGlob)
{
	return (FieldPtr)iGlob->dockField();
}*/


//1bit==1byte, overlapped:0, !overlapped:1
uint32_t FuncInfo_s::Overlap(const REG_t &r1, const REG_t &r2)
{
	int nSize1 = r1.m_siz;
	int nSize2 = r2.m_siz;// pOp->OpSize();
	if (!nSize2)
		nSize2 = OPSZ_BYTE;
//	assert(nSize1 && nSize2);

	//LocalsTracer_t an(*this, PathTracer_t(), OpTracer_t());

	int nOfs1 = r1.m_ofs;				// + disp;
	int nOfs2 = r2.m_ofs;// pOp->Offset0() + CalcDispl(pOp);			// + pOp->disp;
	int dOfs = nOfs2 - nOfs1;//delta offset

	uint32_t u = 0;
	uint32_t m1 = ~(~0 << nSize1);
	uint32_t m2 = ~(~0 << nSize2);;

	if (dOfs > 0)
	{
		if (nSize1 > dOfs)
		{
			m2 <<= dOfs;
			u = m1 & m2;
		}
	}
	else if (dOfs < 0) 
	{
		if (nSize2 > -dOfs)
		{
			m2 >>= -dOfs;
			u = m1 & m2;
		}
	}
	else//if (dOfs == 0)
	{
		u = m1 & m2;
	}

	return u;
}


bool FuncInfo_s::IsStackLocal(CFieldPtr pField)//stack args/vars, NOT registers
{
	if (SSIDx(pField) != SSID_LOCAL)
		return false;
	return true;
}

bool FuncInfo_s::IsLocalReg(CFieldPtr pField)
{
	SSID_t ssid(SSIDx(pField));
	if (ssid == SSID_NULL || ssid == SSID_LOCAL)// || ssid == SSID_GLOBAL)// || ssid == SSID_LOCAL_TOP)
		return false;
	//assert(GetLocalOwner());
	return true;
}

bool FuncInfo_s::IsLocal0(CFieldPtr pField)
{
	SSID_t ssid(SSIDx(pField));
	if (ssid == SSID_NULL)
		return false;
	return true;
}

bool FuncInfo_s::isLocalArg(CFieldPtr p) {
		//return isLocal() && !isLocalVar0();
	return p->owner() && p->owner()->typeFuncDef() != nullptr;
}

ADDR FuncInfo_s::address(CFieldPtr p)
{
	assert(IsLocal(p));
	if (isLocalVar0(p))//local var - negative
		return p->_key();
	return address(p->_key());
}

/*ADDR Field_t::address() const
{
	if (!isLocal() || FuncInfo_s::isLocalVar0(this))//local var - negative
		return _key();
	if (ARG_SSID_FROM_KEY(_key()) == SSID_LOCAL)
		return (_key() & LOCAL_ARG_RANGE);
	FieldKeyType k(_key());
	return ADDR(*(short*)&k);//may be negative
}*/


SSID_t FuncInfo_s::SSIDx(ADDR _key)
{
	//assumet to be a local

	if (_key & LOCAL_VAR_FLAG)//isLocalVar0?
		return SSID_LOCAL;//stack var (negative)
	SSID_t ssid(ARG_SSID_FROM_KEY(_key));
	if (ssid != SSID_NULL)
	{
		assert(ssid != SSID_LOCAL);
		return ssid;//stack arg or a register arg/var 
	}
	unsigned order(ARG_ORDER_FROM_KEY(_key));
	if (order == LOCAL_ORDER_STACK)
		return SSID_LOCAL;

	return SSID_NULL;//or global
}

ADDR FuncInfo_s::address(ADDR _key)
{
	//assume a local
	// 
	//if (!isLocal() || isLocalVar0(this))//local var - negative
		//return _key();

	if (ARG_SSID_FROM_KEY(_key) == SSID_LOCAL)
		return (_key & LOCAL_ARG_RANGE);
	FieldKeyType k(_key);
	return ADDR(*(short*)&k);//may be negative
}

bool FuncInfo_s::IsLocalArg(CFieldPtr pSelf)
{
	return (ProtoInfo_s::IsLocalArgAny(pSelf) && FuncInfo_s::order(pSelf) <= LOCAL_ORDER_ARG_UPPER);
}

bool FuncInfo_s::IsLocalVar(CFieldPtr pSelf)
{
	return pSelf->owner()->typeStrucLoc() && !IsLocalArg(pSelf);
}

bool FuncInfo_s::IsAnyLocalVar(CFieldPtr pSelf)
{
	return pSelf->owner()->typeStrucLoc() && !ProtoInfo_t::IsLocalArgAny(pSelf);
}

/*bool	isLocal() const;//any locals (SSID or no)
bool Field_t::isLocal() const
{
	return owner() && owner()->typeStrucLoc() != nullptr;//funcdefs are struclocs
}*/

bool FuncInfo_s::IsLocal(CFieldPtr pSelf)
{
	assert(pSelf->owner());
	return pSelf->owner()->typeStrucLoc() != nullptr;//can be a funcdef
}

HOP FuncInfo_s::GetVarOp(CFieldPtr pSelf)
{
	if (!(IsStackLocal(pSelf) || IsLocalReg(pSelf)))
		return HOP();
	for (XOpList_t::Iterator i(LocalRefs(pSelf)); i; i++)
	{
		HOP pOp(i.data());
		if (IsVarOp(pOp) || IsArgOp(pOp))
			return pOp;
	}
	return HOP();
}

bool FuncInfo_s::AreEqual(CFieldPtr pField, CHOP pOp2)
{
	if (SSIDx(pField) == pOp2->SSID())
	{
		if (address(pField) == pOp2->Offset0())
			if (pField->size() == pOp2->Size())
				return true;
	}
	return false;
}

bool FuncInfo_s::AreEqual(CFieldPtr p1, CFieldPtr p2)
{
	if (SSIDx(p1) == SSIDx(p2))
	{
		//		if (pOp1->EqualTo(pOp2))
		//			return true;
		//	}
		//	else if (pOp1->IsIndirect() && pOp2->IsIndirect())
		//	{
		if (address(p1) == address(p2))
			if (p1->size() == p2->size())
				return true;
	}
	return false;
}

bool FuncInfo_s::IsLabel(CFieldPtr pSelf)
{
	if (!pSelf->isTypeCode())
		return false;
	//assert(pSelf->owner()->type Proc() || pSelf->owner()->typeSeg());
	assert(IsGlobal(pSelf));
#if(0)
	if (pSelf->owner())
		if (!pSelf->owner()->type Proc())
			return false;
	PathPtr hPath(LabelPath(pSelf));
	if (!hPath)
		return false;
	if (PathType(hPath) == BLK_DATA)
		return false;
	assert(PathLabel(hPath) == pSelf);
#endif
	return true;
}

const PathOpList_t& FuncInfo_s::PathOps(HPATH pPath)
{
	if (!pPath)
	{
		const static PathOpList_t z;
		return z;
	}
	return pPath->ops();
}

ADDR FuncInfo_s::PathVA(CHPATH pPath)
{
	for (OpPtr pOp(PathOps(pPath).Head()); pOp; pOp = NextPrime(pOp))
	{
		if (!IsVarOp(pOp))
			return OpVA(pOp);
	}
	return -1;
}

BlockTyp_t FuncInfo_s::PathType(CHPATH pPath)
{
	return pPath->Type();
}

void FuncInfo_s::SetPathType(HPATH pPath, BlockTyp_t typ)
{
	pPath->SetType(typ);
}

bool FuncInfo_s::PathIsEmpty(CHPATH pPath)
{
	return PathOps(pPath).empty();
}

const XOpList_t &FuncInfo_s::GetExitPoints(const FuncDef_t &rSelf)
{
	HPATH hPath(rSelf.pathTree().pathTail());
	if (hPath)
		return PathOpRefs(hPath);
	const static XOpList_t z;
	return z;
}

void FuncInfo_s::DestroyLocalUserData(FieldPtr pLocal)
{
	assert(IsLocal(pLocal)); 
	UserDataLocal_t *o(pLocal->UserData<UserDataLocal_t>()); 
	pLocal->SetUserData<void>(nullptr);
	delete o;
}

PathPtr FuncInfo_s::LabelPath(CFieldPtr pField)
{
	assert(0);
	return PathPtr();
}


#if(NO_EXTRA_USER)

const XOpList_t & FuncInfo_s::PathOpRefs(HPATH pPath)
{
	return pPath->inflow();
}

HOP FuncInfo_s::FindDanglingOp(CFieldPtr pSelf)
{
	assert(IsLocal(pSelf));
	for (XOpList_t::Iterator i(LocalRefs(pSelf)); i; ++i)
	{
		HOP hOp(i.data());
		if (!hOp->isLinked())
			return hOp;
	}
	return HOP();
}

/*const XOpList_t & FuncInfo_s::LocalRefs(CFieldPtr pField)
{
	if (pField)
	{
		const UserDataLocal_t *o(pField->UserData<UserDataLocal_t>());
		if (o && pField->userDataType() == FUDT_ LOCAL)
			return o->xrefs;
	}
	const static XOpList_t j_xrefs;
	return j_xrefs;
}*/

XOpList_t & FuncInfo_s::LocalRefs(CFieldPtr pField)
{
	if (pField->hasUserData())
	{
		assert(IsLocal(pField));
		const UserDataLocal_t *o(pField->UserData<UserDataLocal_t>()); 
		if (o)//&& pField->userDataType() == FUDT_LOCAL)
			return (XOpList_t &)o->xrefs;
	}
	static XOpList_t proxy;
	return proxy;
}

void FuncInfo_s::PushLocalRefBack(FieldPtr pField, XOpLink_t *pXRef)
{
	UserDataLocal_t *o(pField->UserData<UserDataLocal_t>());
	if (!o)
	{
		o = NEW_USERDATA_LOCAL(pField);
		//assert(o->pPath == PathPtr());
	}
	assert(IsLocal(pField));
	o->xrefs.push_back(pXRef);
}

XOpLink_t *FuncInfo_s::TakeLocalRefFront(FieldPtr pField)
{
	UserDataLocal_t *o(pField->UserData<UserDataLocal_t>());
	if (!o)
		return nullptr;
	assert(IsLocal(pField));
	assert(!o->xrefs.empty());
	XOpLink_t *pXRef(o->xrefs.take_front());
	if (o->xrefs.empty())// && o->pPath == PathPtr())
	{
		pField->SetUserData<void>(nullptr);
		delete o;
	}
	return pXRef;
}

#endif


OpPtr FuncInfo_s::NextPrime(CHOP hSelf, bool bSkipVarOps)
{
	Path_t::OpEltPtr p(NextOp(hSelf));
	while (p)
	{
		if (!bSkipVarOps || !IsVarOp(p))
			return PRIME(p);
		p = NextOp(p);
	}
	return OpPtr();
}

OpPtr FuncInfo_s::PrevPrime(CHOP hSelf)
{
	Path_t::OpEltPtr p(PrevOp(hSelf));
	if (p)
		return PRIME(p);
	return OpPtr();
}

ADDR FuncInfo_s::OpVA(CHOP pSelf)
{
	if (!IsPrimeOp(pSelf))
		return 0;//-1
	return pSelf->ins().VA();
}

ADDR FuncInfo_s::OpVA(CHOP pSelf, int &idx)
{
	//idx = 0;
	if (!IsPrimeOp(pSelf))
		return 0;//-1
	ADDR va(pSelf->ins().VA());
	for (HOP pOp(pSelf); ; idx++)
	{
		pOp = PrevPrime(pOp);
		if (!pOp)
			break;
		ADDR va2(pOp->ins().VA());
		if (va2 != 0 && va2 != va)
			break;
	}
/*	if (n == -1)
	{
		int i(1);
		HOP pOp = pSelf;
		for (;;)
		{
			pOp = PrevPrimeEx(pOp);
			if (!pOp || !IsAuxOp(pOp))
				break;
			i++;
		}
		if (pOp)
			n = pOp->ins().VA();
		if (n == -1)//go down
		{
			i = -1;
			pOp = pSelf;
			for (;;)
			{
				pOp = NextPrime(pOp);
				if (!pOp || (pOp->ins().VA() > 0))
					break;
				i--;
			}
			if (pOp)
				n = pOp->ins().VA();
		}
		if (ex)
			*ex = i;
	}*/
	return va;
}

MyString FuncInfo_s::StrNo(CHPATH pSelf)
{
	ADDR n(PathVA(pSelf));
	return MyStringf("%d", n);
}

bool FuncInfo_s::IsCode(CHOP hSelf)
{
	if (IsPrimeOp(hSelf))
	{
		HPATH pPath(PathOf(hSelf));

		if (pPath->Type() == BLK_DATA//not data op
			|| pPath->Type() == BLK_ENTER//not entry op
			|| pPath->Type() == BLK_EXIT)//not exit op
			return false;

		if (hSelf->ins().mAction == ACTN_NULL)
			return false;//not var op
	}

	return true;
}

bool FuncInfo_s::IsArgOp(CHOP hSelf)
{
	if (!IsPrimeOp(hSelf))
		return false;

	HPATH pPath = PathOf(hSelf);
	if (pPath->Type() == BLK_ENTER)
		return true;
	return false;
}

bool FuncInfo_s::IsVarOp(CHOP hSelf)
{
	if (!IsPrimeOp(hSelf))
		return false;

	if (hSelf->ins().mAction != ACTN_NULL)
		return false;

	HPATH pPath = PathOf(hSelf);
	if (pPath->Type() == BLK_DATA
		|| pPath->Type() == BLK_ENTER
		|| pPath->Type() == BLK_EXIT)
		return false;
	return true;
}

bool FuncInfo_s::IsCall(CHOP hSelf)
{
	assert(IsPrimeOp(hSelf));
	if (ActionOf(hSelf) == ACTN_CALL)
		return true;
	return false;
}

bool FuncInfo_s::IsCallIntrinsic(CHOP hSelf)
{
	assert(IsCall(hSelf));
	return hSelf->ins().opnd_INTRINSIC;
}

bool FuncInfo_s::IsGoto(CHOP hSelf)
{
	if (IsPrimeOp(hSelf))
	{
		if (ActionOf(hSelf) == ACTN_GOTO)
//			if (PathType(PathOf(hSelf)) == BLK_JMP)			//???
				return true;//data ops don't have action
	}
	return false;
}

HOP FuncInfo_s::NextAuxOp(CHOP pOp)
{
	assert(IsPrimeOp(pOp));
	HOP pOpNx(NextPrime(pOp));
	if (pOpNx && OpVA(pOpNx) == OpVA(pOp))
		return pOpNx;
	return HOP();
}

HOP FuncInfo_s::PrevAuxOp(CHOP pOp)
{
	assert(IsPrimeOp(pOp));
	HOP pOpPr(PrevPrime(pOp));
	if (pOpPr && OpVA(pOpPr) == OpVA(pOp))
		return pOpPr;
	return HOP();
}


OpPtr FuncInfo_s::PrevPrimeEx(CHOP pSelf)
{
//	assert(IsPrimeOp());
	OpPtr pOpPr(PrevPrime(pSelf));
	if (!pOpPr)
	{
		PathPtr pPathPr = TreePrevEx(PathOf(pSelf));
		if (pPathPr)
		{
			HPATH hPathPr(pPathPr);
			if (hPathPr->Type() != BLK_ENTER)
				pOpPr = GetLastOp(hPathPr);
		}
	}
	return pOpPr;
}

OpPtr FuncInfo_s::NextPrimeEx(CHOP pSelf, bool bSkipVarOps)
{
//	assert(IsPrimeOp());
	OpPtr pOpNx(NextPrime(pSelf, bSkipVarOps));
	if (!pOpNx)
	{
		PathPtr pPathNx(TreeNextEx(PathOf(pSelf)));
		if (pPathNx)
		{
			pOpNx = GetFirstOp(pPathNx);
			if (bSkipVarOps && pOpNx)
			{
				if (IsVarOp(pOpNx))
					pOpNx = NextPrime(pOpNx, bSkipVarOps);
				assert(!IsVarOp(pOpNx));
			}
		}
	}
	return pOpNx;
}

Action_t FuncInfo_s::ActionOf(CHOP hSelf)
{
	if (!IsPrimeOp(hSelf))
		return ACTN_NULL;
	return hSelf->ins().mAction;
}

bool FuncInfo_s::IsJointOp(HOP pOp)
{
	HPATH pPath(PathOf(pOp));
	if (PRIME(PathOps(pPath).Head()) != pOp)
		return false;
	return !PathOpRefs(pPath).empty();
}

int32_t FuncInfo_s::StackTop(CHOP hSelf)
{
	if (!IsPrimeOp(hSelf))
	{
		if (IsCode(hSelf))
			return StackTop(PrimeOp(hSelf));
		assert(IsArgOp(hSelf));
		return 0;
	}
	if (hSelf->ins().opnd_BAD_STACKOUT)
		return STACK_INVALID;
	int32_t stacktop = hSelf->pstackIn();
	if (hSelf->pstackDiff() < 0)//push
		stacktop += hSelf->pstackDiff();
	return stacktop;
}

int32_t FuncInfo_s::StackOut(CHOP hSelf)
{
	assert(IsPrimeOp(hSelf));
	if (hSelf->ins().opnd_BAD_STACKOUT)
		return STACK_INVALID;
	int32_t stackout = hSelf->pstackIn();
#ifdef _DEBUG
	int32_t stackout_rev = -stackout;
#endif
	stackout += hSelf->pstackDiff();
	return stackout;
}



