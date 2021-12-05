#include "info_proto.h"
#include "info_func.h"
#include "type_funcdef.h"
#include "cc.h"
#include "clean_ex.h"
#include "front/front_IA.h"

///////////////////////////////////////////////// FuncArgsMap
class FuncArgsMap : public FieldMap
{
public:
	FuncArgsMap() {}
	FieldPtr takeField(uint8_t ssid, int offs, unsigned opsz, int order)
	{
		for (FieldMapIt i(begin()); i != end(); i++)
		{
			FieldPtr pArg(VALUE(i));
			if (FuncInfo_s::SSIDx(pArg) == ssid && FuncInfo_s::address(pArg) == offs)
				if (opsz == 0 || pArg->size() == opsz)
				{
					//order = pArg->order();
					take(i);
					if (order != 0)
					{
						ADDR key(FuncInfo_s::setupKey(ssid, offs, order));
						pArg->overrideKey(key);
					}
					pArg->setOwnerComplex(nullptr);//should have been set for SSIDx() to work!
					return pArg;
				}
		}
		return nullptr;
	}

	FieldPtr PopFront()
	{
		if (empty())
			return nullptr;
		FieldPtr pField(VALUE(begin()));
		FieldMapIt it(begin());
		take(it);
		pField->setOwnerComplex(nullptr);
		return pField;
	}

	FieldPtr  FindField(CFieldPtr pField) const
	{
		for (FieldMapCIt i(begin()); i != end(); i++)
		{
			CFieldPtr pArg(VALUE(i));
			if (FuncInfo_s::AreEqual(pField, pArg))
				return (FieldPtr)pArg;
		}

		return 0;
	}
};

////////////////////////////////////////////////////////////////


ProtoInfo_t::ProtoInfo_t(const DcInfo_t& r, CGlobPtr g)
	: DcInfo_t(r.dc(), r.memMgrGlob()),
	mrFuncDefRef(*(GlobPtr)g),
	mrFuncDef(*mrFuncDefRef.typeFuncDef())
{
}

ProtoInfo_t::ProtoInfo_t(const FuncInfo_t& r)
	: DcInfo_t(r.dc(), r.memMgrGlob()),
	mrFuncDefRef(r.FuncDefRef()),
	mrFuncDef(*mrFuncDefRef.typeFuncDef())
{

}

unsigned ProtoInfo_s::FuncStatus(CGlobPtr g)
{
	return g->flags() & FDEF_STATUS_MASK;
}

bool ProtoInfo_s::IsFuncStatusAborted(CGlobPtr g)
{
	ObjFlagsType f(g->flags());
	return (f & FDEF_STATUS_MASK) >= FDEF_DC_ERROR;
}

bool ProtoInfo_s::IsFuncStatusProcessing(CGlobPtr g)
{
	if (FuncStatus(g) > FDEF_DC_FINISHED_OK)
		if (FuncStatus(g) < FDEF_DC_ERROR)
			return true;
	return false;
}

/*void DcInfo_t::SetFuncInvalid(GlobPtr g)
{
ObjFlagsType &f(g->flags());
f &= ~FDEF_STATUS_MASK;
f |= FDEF_DC_ERROR;
}*/

bool ProtoInfo_s::IsFuncVarArged(CGlobPtr g)
{
	ObjFlagsType f(g->flags());
	return (f & FDEF_VARIARDIC) != 0;
}

void ProtoInfo_s::SetFuncVarArged(GlobPtr g, bool b)
{
	assert(g->func());
	ObjFlagsType& f(g->flags());
	f &= ~FDEF_VARIARDIC;
	if (b)
		f |= FDEF_VARIARDIC;
}

bool ProtoInfo_s::IsFuncUserCall(CGlobPtr g)
{
	ObjFlagsType f(g->flags());
	return (f & FDEF_USERCALL) != 0;
}

void ProtoInfo_s::SetFuncUserCall(GlobPtr g, bool b)
{
	assert(g->func());
	ObjFlagsType& f(g->flags());
	f &= ~FDEF_USERCALL;
	if (b)
		f |= FDEF_USERCALL;
}

bool ProtoInfo_s::IsFuncCleanArged(CGlobPtr g)
{
	assert(g->func());
	ObjFlagsType f(g->flags());
	return (f & FDEF_PURGE) != 0;
}

void ProtoInfo_s::SetFuncCleanArged(GlobPtr g, bool b)
{
	assert(g->func());
	ObjFlagsType& f(g->flags());
	f &= ~FDEF_PURGE;
	if (b)
		f |= FDEF_PURGE;
}

bool ProtoInfo_s::IsFuncStatusFinished(CGlobPtr g)
{
	assert(g->func());
	if (IsStub(g))
		return false;
	ObjFlagsType f(g->flags());
	return (f & FDEF_STATUS_MASK) == FDEF_DC_FINISHED_OK;
}

bool ProtoInfo_s::SetFuncStatus(GlobPtr g, unsigned status)
{
	assert(g->func());
	ObjFlagsType& f(g->flags());
	ObjFlagsType statusOld(f & FDEF_STATUS_MASK);
	if (status == 0)
	{
		f &= ~FDEF_STATUS_MASK;//reset
	}
	else
	{
		//assert(status >= FDEF_DC_PHASE1);
		if (status < statusOld)
			return false;//never lower! except reset
		f &= ~FDEF_STATUS_MASK;//reset
		f |= (status & FDEF_STATUS_MASK);
	}
	return true;
}

bool ProtoInfo_s::IsStub(CGlobPtr g)
{
	const FuncDef_t* pf(g->func());
	return pf && pf->isStub();
}

bool ProtoInfo_s::IsStubUndefined(CGlobPtr g)
{
	return IsStub(g) && FuncStatus(g) == 0;
}

bool ProtoInfo_s::IsThisCallType(CGlobPtr pFuncDef)
{
	assert(pFuncDef->typeFuncDef());
	return ThisPtrArg(pFuncDef) != nullptr;
}

bool ProtoInfo_t::IsThisCallType() const
{
	return ThisPtrArg(FuncDefPtr()) != nullptr;
}

bool ProtoInfo_t::IsClassMember() const
{
	return DockField() && OwnerScope(DockField());
}

bool ProtoInfo_t::IsConstClassMember() const
{
	FieldPtr pThisArg(ThisPtrArg(FuncDefPtr()));
	return pThisArg && pThisArg->isConstPtrToConstStruc(false);
}

FieldPtr ProtoInfo_t::FirstArg() const
{
	const FieldMap &m(mrFuncDef.argFields());
	if (m.empty())
		return nullptr;
	return (FieldPtr)VALUE(m.begin());
}

FieldPtr ProtoInfo_t::DockField() const
{
	return DcInfo_t::DockField(FuncDefPtr());
}

FieldPtr ProtoInfo_s::ThisPtrArg(CGlobPtr pSelf)
{
	FieldPtr pFuncField(DockField(pSelf));
	if (pFuncField)
		return pSelf->typeFuncDef()->thisCallArg(OwnerScope(pSelf));
	return nullptr;
}

void ProtoInfo_t::FromFuncProfile(const FuncProfile_t& fp) const
{
	GlobPtr g(FuncDefPtr());
	//SetFuncStatus(0);

//	TypePtr iClassOld(UnmakeMemberMethod(DockField()));
//	assert(!iClassOld);
//	AcquireTypeRef(iClassOld);//it can to be deleted in calls below, so preserve it for a while

	FuncDef_t& rFuncDef(*g->typeFuncDef());

	bool bIsStub(ProtoInfo_t::IsStub(g));

	assert(bIsStub || ProtoInfo_t::FuncStatus(g) == FDEF_DC_PHASE0);
	{
		int pstackPurge(fp.pstackPurge);
		if (!(g->flags() & FDEF_INTRINSIC))
			pstackPurge += RetAddrSize();
		rFuncDef.setPStackPurge(pstackPurge);
		rFuncDef.setFStackPurge(fp.fpudiff);
	}

	SetFuncUserCall(g, (fp._flags & PPF_Usercall) != 0);
	SetFuncVarArged(g, (fp._flags & PPF_Variardic) != 0);
	if (fp.stackin > 0 && fp.stackin == fp.pstackPurge)
		SetFuncCleanArged(g, true);
	else
		SetFuncCleanArged(g, (fp._flags & PPF_StackPurge) != 0);

	Arg1List_t lArgs;
	bool bThiscall(GetTempArgs(fp, lArgs));
	CreateArgList(lArgs, bThiscall);//what's for indirect calls when this is a temp func?!!

//CHECKID(mrFuncDef.type obj(), 1809)//2600)//1768)
//STOP

	Arg1List_t lRets;
	GetTempRets(fp, lRets);
	CreateRetList(lRets);

	//if (bIsStub)//???should be cleaned?
	CreateSpoiltList(fp.spoiltRegs);

	//rFuncDef.mSpoiltFlags = (uint16_t)fp.spoiledFlags;

	//TypePtr iClass(0);//SetClassMembership(fp.classMemberCh()));
	//int iRet((iClass == iClassOld) ? 1 : 2);

	//assert(FuncStatus() == 0);//? && IsStub(FuncDefPtr()));
	if (bIsStub)
		ProtoInfo_t::SetFuncStatus(g, FDEF_DEFINED);

	//	DcCleaner_t<> DC(*this);
	//	DC.ReleaseTypeRef(iClassOld);//release a taken ref
}

FieldPtr ProtoInfo_t::AddRetField(FieldPtr pField) const
{
	//CHECKID((&mrFuncDefRef), 1809)
	//STOP
		//pField->overrideKey(key);
		//assert(pField->order() == LOCAL_ORDER_RETVAL);
	if (!mrFuncDef.retFields().insert_unique(pField).second)
		ASSERT0;
	pField->setOwnerComplex((TypePtr)FuncDefPtr());
	return pField;
}

FieldPtr ProtoInfo_t::AddArgField(FieldPtr pField) const
{
	//CHECKID((&mrFuncDefRef), 1809)
	//STOP
	if (!mrFuncDef.argFields().insert_unique(pField).second)
		ASSERT0;
	pField->setOwnerComplex((TypePtr)FuncDefPtr());
	return pField;
}

int ProtoInfo_t::CreateArgList(const Arg1List_t& l, bool bThiscall) const
{
	FuncArgsMap lOld;
	mrFuncDef.takeArgFields(lOld);//keep the `owner` ptr for SSID() to work

	FieldPtr pThisArg(nullptr);
	for (auto i(l.begin()); i != l.end(); ++i)
	{
		const Arg1_t& a(*i);
		//order = -1;//?
		ADDR key(FuncInfo_s::setupKey(a.ssid(), a.offs(), -1));
		//int order(-1);
		FieldPtr pField(lOld.takeField(a.ssid(), a.offs(), 0/*a.opsz()*/, ARG_ORDER_FROM_KEY(key)));//any size
		if (!pField)
			pField = memMgr().NewField(key);
		if (!pField->type())
			SetType(pField, GetStockType(a.optyp()));
		assert(pField->type()->size() == a.opsz());
		if (i == l.begin() && bThiscall)
			pThisArg = pField;
		if (a.ssid() == SSID_LOCAL)
		{
			//check if an existing arg spans more than 1 slot
			for (auto j(i); j != l.end(); ++j)
			{
				const Arg1_t& b(*j);
				int n(b.offs() + b.opsz() - a.offs());
				if (pField->size() <= n)
				{
					i = j;
					break;
				}
			}
		}
		//InsertField((GlobToTypePtr)FuncDefPtr(), pField, key);
		AddArgField(pField);
	}

	if (pThisArg)
		MakeThisCallFromArg(pThisArg);

	ClearFieldMap(lOld);

	return 1;
}

int ProtoInfo_t::CreateRetList(const Arg1List_t& l) const
{
	FuncArgsMap lOld;
	mrFuncDef.takeRetFields(lOld);

	//create a new list
	for (Arg1List_t::const_iterator i(l.begin()); i != l.end(); i++)
	{
		const Arg1_t& arg(*i);
		//int order(0);
		FieldPtr pField(lOld.takeField(arg.ssid(), arg.offs(), 0/*arg.opsz()*/, LOCAL_ORDER_RETVAL));
		//assert(order == 0 || order == LOCAL_ORDER_RETVAL);
		if (!pField)
		{
			ADDR key(FuncInfo_s::setupKey(arg.ssid(), arg.offs(), LOCAL_ORDER_RETVAL));
			pField = memMgr().NewField(key);
		}
		SetType(pField, GetStockType(arg.optyp()));

		//pField->overrideKey(key);
		AddRetField(pField);
	}

	ClearFieldMap(lOld);

	return 1;
}

FieldPtr ProtoInfo_t::RepositionArg(FieldPtr pArg, const REG_t &r) const
{
	assert(!r.isNull());
	assert(pArg->type() && pArg->type()->typeSimple());
	assert(!pArg->name());//later
	REG_t q(FuncInfo_s::address(pArg), pArg->OpSize());
	if (q == r)
		return pArg;

	ClearLocalType(pArg);

	FieldMap& m(mrFuncDef.retFields());
	if (FuncInfo_s::address(pArg) != r.lower())//adjust size
	{
		ADDR key(FuncInfo_s::setupKey(FuncInfo_s::SSIDx(pArg), r.lower(), FuncInfo_s::order(pArg)));
		FieldMapIt it(pArg);
		m.take(it);//iterator advanced!
		//memMgr().Delete(pArg);

		pArg->overrideKey(key);
	}
	
	TypePtr iType(GetStockTypeEx(OpType_t(r.m_siz)));
	assert(iType);
	SetType(pArg, iType);

	return pArg;
}

void ProtoInfo_t::AddToSpoiltList(SSID_t ssid, REG_t q, FuncArgsMap* plOld) const
{
	assert(!q.isNull());

	//first search if there is a match in `ret list'
	for (FuncRetsIt i(mrFuncDef); i; ++i)
	{
		FieldPtr pField(VALUE(i));
		if (FuncInfo_s::order(pField) < LOCAL_ORDER_SPOILT)
			break;//no args

		if (FuncInfo_s::SSIDx(pField) != ssid)
			continue;

		REG_t r(FuncInfo_s::address(pField), pField->OpSize());
		assert(!r.isNull());
		if (r == q)
			return;

		bool bIntersects(q.intersects(r));
		bool bAdjacent(q.adjacent(r));
		
		if (bIntersects || bAdjacent)
		{
			if (IsSpoiltReg(pField))//don't modify retvals
			{
				REG_t t(q.united(r));
				if (mrDC.isRegValid(ssid, t))//check if a new register exists
				{
					RepositionArg(pField, t);
					return;
				}
			}
			if (bIntersects)
			{
				REG_t t;
				q = q.difference(r, t);
				assert(t.isNull());
				if (q.isNull())
					return;//eliminated
			}
		}


		//int d(r.lower() - q.lower());
/*		if (r.lower() == q.lower())
		{
			if (r.upper() >= q.upper())
				q.m_siz = 0;
			else
			{
				q.m_ofs += r.m_siz;
				q.m_siz -= r.m_siz;
			}
		}
		else if (r.lower() > q.lower())//combine
		{
			if (r.lower() < q.upper())//got inside with lower bound
			{
				if (r.upper() >= q.upper())
					q.m_siz = r.lower() - q.lower();//shrink at top
				else
					ASSERT0;//a disjoint set - LATER!
			}
		}
		else if (r.upper() > q.lower())//got inside with upper bound
		{
			int dd(r.upper() - q.lower());
			q.m_ofs += dd;//shrink at bottom
			q.m_siz -= dd;
		}*/
	}
	//check if there are any fields intersect or match exactly
/*	for (FuncSpoiltIt it(mrFuncDef, ssid); it; )// !it.isAtMapEnd(); )//test retvals as well - these are the spoiled ones either
	{
		FieldPtr pField(VALUE(it));
		if (FuncInfo_s::SSIDx(pField) != ssid)
		{
			++it;
			continue;//skip
		}
		REG_t r(pField->address(), pField->OpSize());
		if (r == q)
		{
			ASSERT0;
			return;//if this worked out - there must be no another match
		}
		if (r.lower() > q.upper())
			break;
		if (r.lower() == q.upper())
			break;
		if (r.upper() > q.lower())//intersects, remove it and update original limits
		{
			int lower(std::min(r.lower(), q.lower()));
			int upper(std::max(r.upper(), q.upper()));
			q = REG_t(lower, upper - lower);//adjust
			if (q == r)
				return;//a subset
			if (!FuncInfo_t::IsSpoiltReg(pField))
				break;//should not touch retvals
			assert(pField->type() && !pField->name());
			ClearLocalType(pField);
			FieldMap& m(mrFuncDef.retFields());
			m.take(it);//iterator advanced!
			memMgr().Delete(pField);
		}
		else
			++it;
	}*/

	//ADDR key(LOCAL_MAKE_KEY(LOCAL_ORDER_SPOILT, ssid, q.lower()));
	ADDR key(FuncInfo_s::setupKey(ssid, q.lower(), LOCAL_ORDER_SPOILT));

	FieldPtr pField(nullptr);
	if (plOld)
	{
		//int order(0);
		pField = plOld->takeField(ssid, q.lower(), 0/*arg.opsz()*/, LOCAL_ORDER_SPOILT);
	}
	if (!pField)
		pField = memMgr().NewField(key);

	TypePtr iType(GetStockTypeEx(OpType_t(q.m_siz)));
	assert(iType);

	SetType(pField, iType);

	//if (!m.insert_unique(pField).second)
		//ASSERT0;
	//pField->setOwnerComplex(FuncDefPtr());
	AddRetField(pField);
}

int ProtoInfo_t::CreateSpoiltList(const GPRs_t& v) const
{
	FuncArgsMap lOld;
	mrFuncDef.takeSpoiltFields(lOld);

	for (size_t i(0); i < v.size(); i++)//after rets are done, to avoid redundant items
	{
		AddToSpoiltList(SSID_CPUREG, v[i], &lOld);
	}

	ClearFieldMap(lOld);
	return 1;
}

void ProtoInfo_t::GetArgProfile(ProtoProfile_t& si) const
{
	//update CPUIn
	si = ProtoProfile_t();//clear

	if ((mrFuncDefRef.flags() & FDEF_VARIARDIC) != 0)
		si._flags |= PPF_Variardic;
	if ((mrFuncDefRef.flags() & FDEF_USERCALL) != 0)
		si._flags |= PPF_Usercall;
	if (IsFuncCleanArged())
		si._flags |= PPF_StackPurge;

	if (!mrFuncDef.hasArgFields())
		return;

	assert(si.cpuin.empty());
	si.cpuin.push_back(REG_t());

	CallingConv_t CC(*this, FuncDefPtr());
	for (FuncCCArgsCIt<> it(mrFuncDef, CC); it; ++it)
	{
		//CFieldPtr pArg = VALUE(it);

		if (it.ssid() == SSID_CPUREG)
		{
			if (ThisPtrArg(FuncDefPtr()) == it.field())
			{
				assert(si.cpuin.size() == 1);
				si.cpuin.back() = REG_t(it.offset(), it.size());
				continue;
			}
			si.cpuin.push_back(REG_t(it.offset(), it.size()));
		}
		else if (it.ssid() == SSID_FPUREG)
		{
			int f(it.offset() / FR_SLOT_SIZE);
			f += (it.size() / FR_SIZE);
			if (f > si.fpuin)
				si.fpuin = f;
		}
		else if (it.ssid() == SSID_LOCAL)
		{
			int sz = it.size2();
			int offs = it.offset() - RetAddrSize();
			si.applyStackIn(offs + sz, mrFE.stackaddrsz);
		}
		else
		{
			//?assert(false);
		}
	}
	if (si.cpuin.size() == 1 && !si.cpuin[0].isValid())
		si.cpuin.clear();//reserved for 'this' reg
}

void ProtoInfo_t::GetFuncProfile(FuncProfile_t& fp) const
{
	//InitFuncProfile(fp);

	GetArgProfile(fp);

	if (fp._flags & PPF_StackPurge)
	{
		fp.pstackPurge = fp.stackin;
		fp.pstackPurgeRet = RetAddrSize();
	}
	else
	{
		fp.pstackPurge = mrFuncDef.getPStackPurge();
		if (!IsIntrinsic())
		{
			fp.pstackPurge -= RetAddrSize();//ignored for stack purge flags set
			fp.pstackPurgeRet = RetAddrSize();
		}
		else
			fp.pstackPurgeRet = 0;
	}
	fp.fpudiff = mrFuncDef.getFStackPurge();
	//RegMaskToGPRs(mrFuncDef.mSpoiltRegs, fp.spoiltRegs);
	for (FuncSpoiltIt it(mrFuncDef, SSID_CPUREG); it; ++it)
	{
		FieldPtr pField(VALUE(it));
		fp.spoiltRegs.push_back(REG_t(FuncInfo_s::address(pField), pField->OpSize()));
	}

	//fp.spoiledFlags = mrFuncDef.mSpoiltFlags;

	CallingConv_t CC(*this, FuncDefPtr());

	//int nCount(0);
	for (FuncCCRetsCIt it(mrFuncDef, CC); !it.isAtEnd(); ++it)
	{
		CFieldPtr pRet(it.field());
		ri_t r(FuncInfo_s::toR_t(pRet));
		if (it.ssid() == SSID_CPUREG)
		{
			REG_t R(it.offset(), it.optyp() & OPSZ_MASK);
			fp.cpuout.push_back(R);
		}
		else if (it.ssid() != SSID_FPUREG)
		{
			//?			fp.cpuout[nCount++] = '?';
						//assert(0);
		}
	}
}

uint16_t ProtoInfo_t::StackIn() const
{
	ProtoProfile_t si;
	GetArgProfile(si);
	return (uint16_t)si.stackin;
}

int ProtoInfo_t::FpuIn() const
{
	ProtoProfile_t si;
	GetArgProfile(si);
	return si.fpuin;
}


// THIS FUNCTION DIREGARDS A CALLING CONVENTION!
FieldPtr ProtoInfo_t::AppendFuncArg(TypePtr iType, AppendFuncArgFlags flags)
{
	bool bThisPtr(flags == AFA_THISPTR);
	bool bRegister(flags == AFA_REGISTER);

CHECKID((&mrFuncDefRef), 0x6444)
//CHECK(bRegister)
STOP

	FieldPtr pField(nullptr);
	int order(LOCAL_ORDER_UNKNOWN);//LOCAL_ORDER_STACK
	if (bThisPtr)
		order = LOCAL_ORDER_THIS;
	else if (bRegister)
		order = LOCAL_ORDER_REG;
	int off(0);// mrDC.PtrSize());//a room for a return ptr
	SSID_t ssid(SSID_NULL);
	FuncArgsCRIt rit(mrFuncDef);
	if (rit)
	{
		FieldPtr pLast(rit.field());
		assert(!FuncInfo_s::isLocalVar(pLast));
		if (bThisPtr)
		{//this possible when a namespace converted into a class, making all its methods non-static
			if (!IsThisPtrArg(pLast))
				return nullptr;//should be an error?
			if (!iType)
				return nullptr;
			TypePtr pClass(OwnerClass());
			if (IsConstPtrToStruc(iType, false) != pClass)
				if (IsConstPtrToConstStruc(iType, false) != pClass)
					return nullptr;//requesting a thisptr with wrong type?
			if (pLast->type() != iType)
			{
				BinaryCleaner_t<> PC(*this);
				PC.ClearType(pLast);
				SetType(pLast, iType);//adjust atype (if need be)
			}
			return pLast;
		}
		//assert(FuncInfo_s::SSIDx(pLast) == SSID_NULL);
		off = ARG_OFFS_FROM_KEY(FuncInfo_s::address(pLast));
		int sz(pLast->size());
		off = (ALIGNED(off + std::max(sz, 1), mrDC.PtrSize()));//the stack args should be aligned to machine's word size?
		assert(off < LOCAL_ARG_RANGE);
	}

	ADDR key(FuncInfo_s::setupKey(ssid, off, order));

	if (!pField)
	{
		pField = memMgrGlob().NewField(0);
		if (!InsertFieldAt((GlobToTypePtr)FuncDefPtr(), pField, key))
			ASSERT0;
	}
	//int delta(0);
	if (iType)
	{
		SetType(pField, iType);
		/*		delta = iType->size();
				if (delta == 0)
				{
					delta = mrDC.PtrSize();
		//CHECK(va == 0xC4948)
		//STOP
					MyString sArg(TypePrettyNameFull(iType, CHOP_SYMB));
					fprintf(STDERR, "Warning: Applying function argument of uncertain size at %s (%d assumed): %s\n", VA2STR(va).c_str(), delta, sArg.c_str());//type_name
				}*/
	}
	//return delta;
	return pField;
}

FieldPtr ProtoInfo_t::AppendFuncRet(TypePtr iType)
{
	if (!iType)
		return nullptr;

	int off(0);//a room for a return ptr
	if (mrFuncDef.hasRetFields())
	{
		FieldPtr pLast(VALUE(mrFuncDef.retFields().rbegin()));
		off = (ALIGNED(pLast->_key() + pLast->size(), mrDC.PtrSize()));//the stack args should be aligned to machine's word size?
	}

	//pass return value in RAX/EAX/AX/AL register, according to a calling convention (must do it via FE!)
//	int iOffs = OFS(R_EAX);
//	if (iType->size() <= GPR_SLOT_SIZE)//dc().PtrSize())
	{
		//TODO: doubles
		//TODO: EDX:EAX
		//ADDR at(FuncInfo_s::loc2key(SSID_CPUREG, iOffs));
		ADDR key(FuncInfo_s::setupKey(SSID_NULL, off, LOCAL_ORDER_RETVAL));//order?
		FieldPtr pField(memMgrGlob().NewField(key));
		ProtoInfo_t TI(*this);
		TI.AddRetField(pField);
		SetType(pField, iType);
		return pField;
	}
	return nullptr;
}

bool ProtoInfo_s::IsThisPtrArg(CFieldPtr pArg)
{
	CGlobPtr pCFunc((CGlobPtr)pArg->owner());
	assert(pCFunc->typeFuncDef());
	return ProtoInfo_t::ThisPtrArg(pCFunc) == pArg;
}

bool ProtoInfo_s::MakeThisPtrArg(FieldPtr pArg)
{
	CGlobPtr pOwner((CGlobPtr)pArg->owner());
	assert(pOwner->typeFuncDef());
	FieldPtr pFuncField(DcInfo_t::DockField(pOwner));
	if (pFuncField)
	{
		TypePtr pScope(DcInfo_t::OwnerScope(pFuncField));
		if (pScope && pScope == pArg->isConstPtrToStruc(false))
		{
			assert(false);//later
			return true;
		}
	}
	return false;
}

bool ProtoInfo_t::InsertLocalArg(FieldPtr pField)
{
	ADDR uOffset(pField->_key());//FuncInfo_s::loc2key());
	assert(!pField->owner());
	FieldMapIt i(InsertUniqueFieldIt(FuncDefPtr()->objTypeGlob(), uOffset, pField));
	assert(i != mrFuncDef.argFields().end());
	return true;
}

FieldPtr ProtoInfo_t::AddRetField(OpType_t optype, uint8_t ssid, int offset) const
{
	ADDR key(FuncInfo_s::setupKey(ssid, offset));
	FieldPtr pField(memMgr().NewField(key));

	SetType(pField, findOpType(optype));
	//assert(pField->Offset() == offset);//?pField->setOffset(offset);

	return AddRetField(pField);
}

void ProtoInfo_t::ClearLocalType(FieldPtr pField) const
{
	if (pField->type())
	{
		DcCleaner_t<> DC(*this);
		DC.ClearType(pField);
	}
}

bool ProtoInfo_s::IsOpOverlappedByField(const ri_t& source, const ri_t& target)
{
	assert(target.ssid < 0x10);
	if (target.ssid == source.ssid)
	{
		int off0(source.ofs);
		if (off0 < target.ofs)
		{
			if (off0 + source.siz > target.ofs)
				return true;
		}
		else if (off0 > target.ofs)
		{
			if (target.ofs + (int)target.siz > off0)
				return true;
		}
		else
			return true;
	}
	return false;
}

FieldPtr ProtoInfo_t::FindArgFromOp(const ri_t& src) const
{
	if (mrFuncDef.hasArgFields())
	{
		CallingConv_t CC(*this, FuncDefPtr());
		for (FuncCCArgsCIt<> it(FuncDef(), CC); it; ++it)
		{
			ri_t tgt(it.toR_t());
			if (IsOpOverlappedByField(src, tgt))
				return (FieldPtr)it.field();
		}
	}
	return nullptr;
}

int ProtoInfo_t::PStackPurge() const
{
	return /*RetAddrSize() +*/ mrFuncDef.getPStackPurge();
}

bool ProtoInfo_t::IsIntrinsic() const
{
	return mrFuncDefRef.isIntrinsic();
}

void ProtoInfo_t::ApplyDefaultSpoiltRegs() const
{
	if (mrFE.spoilt_regs_def)
	{
		for (const r_t* pr(mrFE.spoilt_regs_def); pr->ssid; pr++)
			AddToSpoiltList(SSID_t(pr->ssid), REG_t(pr->ofs, pr->siz));
	}
}

void ProtoInfo_t::ApplyCalleeSpoiltRegs(CGlobPtr pCallee) const
{
	for (FuncSpoiltIt it(*pCallee->typeFuncDef()); it; ++it)
	{
		CFieldPtr pField(it.field());
		AddToSpoiltList(FuncInfo_s::SSIDx(pField), REG_t(FuncInfo_s::address(pField), pField->OpSize()));
	}
}

bool ProtoInfo_t::UnmakeThisCall() const
{
	assert(IsThisCallType());
	FieldMap &m(mrFuncDef.argFields());
	assert(!m.empty());
	FieldMapIt itArg(m.begin());
	//FieldMapIt itArgNx(itArg);
	//++itArgNx;
	FieldPtr pArg(VALUE(itArg));//try to not change order
	ADDR oldKey(pArg->_key());
	//ADDR refKey(itArgNx != m.end() ? VALUE(itArgNx)->_key() : 0);
	int newOrder;
	SSID_t ssid(ARG_SSID_FROM_KEY(oldKey));
	if (ssid && Storage(ssid).isDiscrete())
		newOrder = LOCAL_ORDER_REG;
	else
		newOrder = LOCAL_ORDER_STACK;
	int offs(ARG_OFFS_FROM_KEY(oldKey));
	ADDR newKey(FuncInfo_s::setupKey(ssid, offs, newOrder));
	if (newKey == oldKey)
		return false;
	FieldMapIt it(pArg);
	FieldPtr pArg2(m.take(it));
	assert(pArg2 == pArg);
	pArg->overrideKey(newKey);
	if (!m.insert_unique(pArg).second)
		ASSERT0;
	return true;
}

bool ProtoInfo_s::IsSpoiltReg(CFieldPtr pSelf)
{
	return IsLocalArgAny(pSelf) && FuncInfo_s::order(pSelf) == LOCAL_ORDER_SPOILT;
}

bool ProtoInfo_s::IsRetVal(CFieldPtr pSelf)
{
	return (IsLocalArgAny(pSelf) && FuncInfo_s::order(pSelf) >= LOCAL_ORDER_RET_LOWER);
}

bool ProtoInfo_t::LocalAutoName(CFieldPtr pSelf, MyString& s) const
{
	size_t rank(1);//bias for vars or args without thisptr

	FieldMapCIt cit((FieldPtr)pSelf);

	if (IsRetVal(pSelf))
	{
		assert(pSelf->owner()->objGlob() == FuncDefPtr());
		const FieldMap& m(mrFuncDef.retFields());
		s = "$ret";
		//count rets before current
		while (cit != m.begin())
		{
			--cit;
			CFieldPtr pField(VALUE(cit));
			if (!IsRetVal(pField))
				break;
			rank++;
		}
		s.append(NumberToString(rank));
		return true;
	}

	if (IsSpoiltReg(pSelf))
	{
		const FieldMap& m(mrFuncDef.argFields());
		s = "$spoi";
		//count spoilt before current
		while (cit != m.begin())
		{
			--cit;
			CFieldPtr pField(VALUE(cit));
			if (!IsSpoiltReg(pField))
				break;
			rank++;
		}
		s.append(NumberToString(rank));
		return true;
	}

	assert(FuncInfo_t::IsLocalArg(pSelf));

	assert(pSelf->owner()->objGlob() == FuncDefPtr());

	const FieldMap& m(mrFuncDef.argFields());
	FieldPtr pThisArg(ThisPtrArg(FuncDefPtr()));
	if (pThisArg == pSelf)
	{
		s = "this";
	}
	else if (main().options().nLocalAutoNamesMode == 0)
	{//simplified
		
		s = "a";
		size_t rank(pThisArg ? 0 : 1);//bias for vars or args without thisptr
		rank += m.rank(cit);
		s.append(NumberToString(rank));
	}
	else
	{
		MyString sfx;
		FieldMapCIt cit0(m.lower_bound(pSelf->_key()));//must be cit or greater
		size_t index(m.rank(cit) - m.rank(cit0));
		if (index > 0)
		{
			index -= 1;
			sfx = std::string(1 + index / 26, 'a' + index % 26);
		}
		int off(pSelf->offset());
		if (FuncInfo_t::IsStackLocal(pSelf))
		{
			assert(off >= 0);
			s = "arg" + sfx + "_" + Int2Str(off, I2S_HEXA);
		}
		else
		{
			assert(FuncInfo_s::IsLocalReg(pSelf) && FuncInfo_s::SSIDx(pSelf) != SSID_AUXREG);
			assert(sfx.empty());
			s = "rarg_" + LocalRegToString(pSelf).upper();
		}
	}
	return true;
}

bool ProtoInfo_s::IsLocalArgAny(CFieldPtr pSelf)
{
	return pSelf->owner()->typeFuncDef();
}

MyString ProtoInfo_t::LocalName(CFieldPtr pSelf) const
{
	assert(FuncInfo_s::IsLocal(pSelf));
	assert(IsLocalArgAny(pSelf));
	if (!pSelf->nameless())
	{
		assert(!pSelf->nameless());
		return pSelf->mpName->c_str();
	}

	MyString s;
	if (!LocalAutoName(pSelf, s))
		ASSERT0;
	return s;
}









