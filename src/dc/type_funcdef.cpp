#include "type_funcdef.h"
#include "info_func.h"
#include "cc.h"
#include "front/front_IA.h"


FuncDef_t::FuncDef_t()
	: //mSpoiltRegs(0),
	m_stackOut(0),
	mSpoiltFlags(0),
	m_nFPUOut(0),
#if(NEW_LOCAL_VARS)
	mpLocals(nullptr),
#endif
	m_dc(0)//all dirty
{
//CHECK(this == (FuncDef_t *)0x00923bf8)
//STOP
}


FuncDef_t::~FuncDef_t()
{
	//assert(!mpThisPtr);
#if(NEW_LOCAL_VARS)
	assert(!mpLocals);
#endif
	assert(mPathTree.m.empty());
	//ClearPathTree();
	//must not delete a memory mgr - it's owner's task
	//delete mpMemMgr;
}

/*HOP FuncDef_t::GetOp(int num)
{
	return pathTree().__GetOp(Body(), num);
}*/

FuncDef_t::FuncDef_t(const FuncDef_t &)
{
	assert(0);
}

bool FuncDef_t::isStub() const
{
	return !Body();
}

const PathOpList_t &FuncDef_t::entryOps() const
{
	HPATH pPath(mPathTree.pathHead());
	if (pPath)
		return pPath->ops();
	{
		const static PathOpList_t z;
		return z;
	}
}

const PathOpList_t &FuncDef_t::exitOps() const
{
	HPATH pPath(mPathTree.pathTail());
	if (!pPath)
	{
		const static PathOpList_t z;
		return z;
	}
	assert(pPath->ops().empty());
	return pPath->ops();
}

bool FuncDef_t::hasArgFields() const
{
	return !mFields.empty() && isLocalArg(&mFields.front());
}

bool FuncDef_t::hasRetFields() const
{
	return !mFields.empty() && isRetVal(&mFields.back());
}

bool FuncDef_t::isLocalArg(CFieldPtr pSelf)
{
	return FuncInfo_t::IsLocalArg(pSelf);
}

bool FuncDef_t::isRetVal(CFieldPtr pSelf)
{
	return ProtoInfo_t::IsRetVal(pSelf);
}

FieldPtr FuncDef_t::takeFrontArg()
{
	FuncArgsIt i(*this);
	if (i)
		return i.take();
	return nullptr;
}

FieldPtr FuncDef_t::takeFrontRet()
{
	FuncRetsIt i(*this);
	if (!i.isAtEnd())
		return i.take();
	return nullptr;
}

void FuncDef_t::takeArgFields(FieldMap &l)
{
	FuncArgsIt i(*this);
	while (i)
	{
		FieldPtr pField(i.take());//iterator advanced here by ref (i++)
		l.insert_unique(pField);
	}
}

void FuncDef_t::takeRetFields(FieldMap &l)
{
	FuncRetsIt i(*this);
	while (!i.isAtEnd())
	{
		FieldPtr pField(i.take());//iterator advanced here by ref (i++)
		l.insert_unique(pField);
	}
}

void FuncDef_t::takeSpoiltFields(FieldMap& m)
{
	FuncSpoiltIt i(*this);
	while (i)
	{
		FieldPtr pField(i.take());//iterator advanced here by ref (i++)
		m.insert_unique(pField);
	}
}

void FuncDef_t::namex(MyString &s) const
{
	Base_t::namex(s);
}

PathPtr FuncDef_t::Body() const
{
	return mPathTree.body();
}

/*FieldPtr FuncDef_t::getTailLabel() const
{
	HPATH pPathExit = mPathTree.pathTail();
	if (pPathExit)
		return pPathExit->m.La belAt();
	return 0;
}*/


bool FuncDef_t::IsMineArg(CFieldPtr pField) const
{
	return (pField->OwnerStruc() == this);
}

bool FuncDef_t::IsMineRet(CFieldPtr pSelf) const
{
	//just find it in the list of ret fields
	for (FuncRetsCIt i(*this); !i.isAtEnd(); ++i)
	{
		CFieldPtr pField(VALUE(i));
		if (pField == pSelf)
			return true;
	}
	return false;
}

FuncDef_t &FuncDef_t::operator=(const FuncDef_t &o)
{
	this->Struc_t::operator=(o);
	return *this;
}

void FuncDef_t::CopyFrom(const FuncDef_t &o)
{
	assert(0);
	/*assert(mpMemMgr);

	Struc_t::CopyFrom(o, *mpMemMgr);

	assert(!miOwnerFuncRef);
	
	assert(!mpThisPtr);
	if (o.mpThisPtr)
	{
		FieldMapCIt j(mFields.begin());
		for (FieldMapCIt i(o.mFields.begin()); i != o.mFields.end(); i++, j++)
			if (i->second == o.mpThisPtr)
			{
				mpThisPtr = j->second;
				break;
			}
	}

	m_nSavedRegs = o.m_nSavedRegs;
	m_nStackOut = o.m_nStackOut;
	m_nCPUFSv = o.m_nCPUFSv;
	m_nFPUOut = o.m_nFPUOut;

	assert(!hasExitFields());
	for (FuncRetsCIt i(o); !i.isAtEnd(); ++i)
	{
		FieldPtr pField(mpMemMgr->NewF ield());
		pField->CopyFrom(*i->second, *mpMemMgr);
		mRet Fields.insert(std::make_pair(i->first, pField));
		//pField->setOwnerComplex(typ eobj());
	}

	assert(mPathTree.m.empty() && o.mPathTree.m.empty());
	assert(mCallRets.empty() && o.mCallRets.empty());*/
}

/*int FuncDef_t::AddEntryOp(HOP pArg)
{
	assert(pArg);
//?	pArg->m_pOwnerFuncDef = this;

	HOP pArgPr = 0;
	HOP pArgNx = 0;
	for (OpList_t::Iterator i(entryOps()); i; i++)
	{
		HOP pOp(i.data());
		if (pOp->IsIndirect())
		{
			if (pArg->IsIndirect())
			{
				pArgPr = pOp;
				if (pOp->Offset0() > pArg->Offset0())
				{
					pArgNx = pOp;
					pArgPr = 0;
					break;
				}
			}
		}
		else if (pOp->OpC() == pArg->OpC())
		{
			pArgPr = pOp;
			if (pOp->OpC() == OPC_CPUREG)
			{
//				if (pOp->Offset0() > pArg->Offset0())
//				{
//					pArgNx = pOp;
//					pArgPr = 0;
//					break;
//				}
			}
			else if (pOp->OpC() == OPC_FPUREG)
			{
				pArgPr = pOp;
				if (pOp->Offset0() < pArg->Offset0())
				{
					pArgNx = pOp;
					pArgPr = 0;
					break;
				}
			}
		}
	}

	if (pArgPr)
		pArgPr->list().LinkAfter(pArg, pArgPr);
	else if (pArgNx)
		pArgNx->list().LinkBefore(pArg, pArgNx);
	else
	{
		HPATH  pPath = CreateHeadPath();
		pPath->m.LinkOpTail(pArg);
	}

	return 1;
}*/

/*const char *FuncDef_t::GetSavedRegsStr() const
{
	static char buf[40];

#if(X64_SUPPORT)
	sprintf(buf, "%I64X", ~mSpoiltRegs);
#else
	sprintf(buf, "%X", m_nSavedRegs);
#endif
	return buf;
}*/


/*void FuncDef_t::GetArgProfile(ProtoProfile_t &si)
{
	FuncInfo_t a(*this);
	a.GetArgProfile(si);
}*/

uint16_t FuncDef_t::GetSpoiltFlags() const
{
	return (mSpoiltFlags & CPUSW_MASK);
}

int FuncDef_t::ValidateRetList()
{return 1;
/*?	int nHi = FpuIn();
	int nLo = getFStackPurge();
	if (nLo == nHi)
		return 1;//fpu returns nothing
	assert(nLo < nHi);//how much exit fpu regs
	int n = nHi - nLo;
//	n >>= 3;
*/
	//update exit list accordingly
/*?	Op_t OpRef;
	OpRef.m_pOwnerFuncDef = this;
	OpRef.ppL ist = (Link_t **)&m_pExitOps;

	if (0)
	for (int ofs = 0; ofs < n; ofs += OPSZ_QWORD)
	{
		OpRef.Setup3(OPC_FPUREG, OPTYP_REAL80, ofs);
		RegisterExitField(&OpRef, 0);//?
	}

	OpRef.ppL ist = 0;*/
	return 1;
}


/*int FuncDef_t::SetupExitAttribs(int nSTACKout, int nFPUout)
{
	uint32_t nDirty = 0;

	int res1 = SetStackOut(nSTACKout);
	if (res1 == 1)
		//nDirty |= FICI_STACKOUT;
		SetChangeInfo(FDEF_CHGFRAME);

	int res2 = SetFPUOut0(nFPUout);
	if (res2 == 1)
		//nDirty |= FICI_FPUOUT;
		SetChangeInfo(FDEF_CHGFRAME);

//	UpdateCallers(nDirty);

	if (res1 == 0 || res2 == 0)
		return 0;
	return 1;
}*/


void FuncDef_t::ClearAnalized()
{
	for (PathTree_t::LeafIterator i(pathTree().tree()); i; i++)
	{
		HPATH pPath(i.data());
		pPath->setAnalized(false);
		for (PathTree_t::ChildrenIterator j(pPath); j; j++)
		{
			assert(0);
			j.data()->setAnalized(false);
		}
	}
}

FieldPtr FuncDef_t::thisCallArg(CTypePtr pScope) const
{
	if (pScope)
	{
		FuncArgsCIt i(*this);
		if (i)
		{
			CFieldPtr pArg(i.field());
			if (FuncInfo_s::order(pArg) == LOCAL_ORDER_THIS)
			{
				assert(pArg->isConstPtrToStruc(false) == pScope || pArg->isConstPtrToConstStruc(false) == pScope);
				return (FieldPtr)pArg;			}
		}
	}
	return nullptr;
}

/*int FuncDef_t::CountOps(int *locex)
{
	int l = 0;
	if (!Body())
		return 0;
	int c = pathTree().__CountOps(Body(), l);
	if (locex)
		*locex = l;
	return c;
}*/



/*int FuncDef_t::OpsNum()
{
	if (!Body()->ops().empty())
		return Body()->ops().back().No() + 1;
	return 0;
}*/


int FuncDef_t::CheckUnresolvedLabels()
{/*?
	FieldPtr  f = ownerFunc()->m_pFields;//Labels_;
	while (f)
	{
		FieldPtr fnx = (FieldPtr )f->Ne xt();
		if (!f->Path())
		{
			f->Unlink();
			while (f->m_pXRefs)
			{
				HOP pOp = f->m_pXRefs->pOp;
				pOp->Goto2Call();
			}
			//f->Delete();
			delete f;
		}

		f = fnx;
	}*/
	return 1;
}




