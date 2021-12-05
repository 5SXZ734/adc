#include "type_proc.h"
#include "type_alias.h"
#include "prefix.h"

#if(0)
PathPtr  TypeProc_t::Body()
{
//?	assert( !mpPath || (mpPath->IsFirst() && mpPath->IsLast()));
	if ( m_pfDef )
		return m_pfDef->mpPath;
	return nullptr;
//	PathPtr pPath = Path();
//	if (!pPath)
//		return 0;
//	assert(pPath->IsRoot());
//	return pPath;
/*	for (PathPtr pPath = Path();
	pPath;
	pPath = pPath->Ne xt())
	{
		if (pPath->Type() == BLK_BODY)
			return pPath;
	}
	return 0;*/
}
#endif

/*
int TypeProc_t::MakeStatic(FieldPtr pData)
{
	if (pData->pp List == (Link_t **)&Statics())
		return -1;//already
	if (!pData->IsDa ta())
		return 0;
	if (pData->IsLo cal())
		return 0;

	//check if data used only by this func
	for (XRef_t *pXRef = pData->m_pXRefs; pXRef; pXRef = pXRef->Ne xt())
	{
		OpPtr pOp = pXRef->pOp;
		if (pOp->GetOw nerFunc() != this)
			return 0;
	}

	//check if all data's m_pIOps ptrs are labels of this func
	for (
		OpPtr pOp = pData->m_pIOps;
		pOp;
		pOp = pOp->N ext())
	{
		if (!pOp->isPtr())
			continue;

		FieldPtr pMLoc = pOp->m_ pMLoc;
		if (pMLoc->IsMLo cUn())
			continue;
		if (!IsMine Label(pMLoc))
			return 0;
	}

	//all OK
	pData->LinkTail((Link_t **)&Statics());
	assert(!pData->Get OwnerPath());
	pData->m_pOwnerFunc = this;
	return 1;
}

FieldPtr TypeProc_t::FindStatic(const char *pName)
{
	FieldPtr pData = Statics();
	while (pData)
	{
		if (pData->CompNamex(pName))//is there already such one?!
			return pData;//just another reference registered
		pData = (FieldPtr )pData->Ne xt();
	}
	return 0;
}*/

/*FieldPtr TypeProc_t::FindLabel(const char *pName)
{
	FieldPtr pLabelExit = FUN CDEF(this)->getTailLabel();
	if (!pLabelExit)
		return 0;

	for (FieldMapIt it = mFields.begin(); ; it++)
	{
		FieldPtr  f = VALUE(it);
		if (f == pLabelExit)
			break;
		if ( !f->IsLa bel() )
			continue;
		FieldPtr  pLabel = f;
		if (pLabel->CompNamex(pName))
			return pLabel;
	}
	return 0;
}*/

/*void TypeProc_t::OrderLabels()
{
	if (!funcdef()->Body()->Childs())
		return;

	Label_t *pLabelPr =  0;
	for (PathTree_t::LeafIterator i(fun cdef()->pathTree().tree()); i; i++)
	{
		PathPtr pPath(i.data());
		Label_t *pLabel = pPath->Label();
		if (!pLabel)
			continue;
		if (pLabelPr)
			pLabel->LinkAfter((Link_t *)pLabelPr);
		else
			pLabel->LinkHead((Link_t **)&m_pFields);//Labels_);//LinkAfter(this);
		pLabelPr = pLabel;
	}
}*/



/*bool TypeProc_t::IsMineOp(OpPtr pOp)
{
	if (pOp->GetOwne rFunc() != this)
		return false;
	return true;
}*/

/*
XRef_t *TypeProc_t::RegisterExitPoint(OpPtr pRet)
{
	assert(pRet && isMineOp(pRet));
	assert(pRet->IsRetOp());

	for (
		XRef_t *pExitPt = m_pExitPts;
		pExitPt;
		pExitPt = pExitPt->N ext())
	{
		assert(pExitPt->pOp);
		assert(isMineOp(pExitPt->pOp));
		if (pExitPt->pOp == pRet)
			return pExitPt;//already
	}

	pExitPt = new XRef_t();
	pExitPt->LinkTail((LinkU_t **)&m_pExitPts);
	pExitPt->pOp = pRet;

	return pExitPt;
}
*/

/*
int TypeProc_t::SetupAttribs()
{
	if (!m_dc.ATTRIBS)
		return 0;

CHECK(CompName("sub_20791640"))
STOP

	for (
		XRef_t *pExitPt = m_pExitPts;
		pExitPt;
		pExitPt = pExitPt->Ne xt())
	{
		OpPtr pOp = pExitPt->pOp;
		assert(pOp);
		assert(isMineOp(pOp));

		//setup func stackout
		if (pOp->St ackOut() != 0)
		{
			G DC.m_dwDumpFlags |= DUMP_STACKTOP;
			ERRORMSG(pOp->MAKEMSG("Stack top (%d) don't evaluate to 0 at exit point", pOp->Stac kOut()));
		}

		if (GetStackOut() == 0)
			SetStackOut(pOp->ui16);
		else
			assert(GetStackOut() == pOp->ui16);

		//setup func fpuout
		int nFPUStackCleanup = pOp->FpuOut()-(8-m_pfDef->FpuIn());
		if (m_pfDef->getFStackPurge() == 0)
			SetFPUOut(nFPUStackCleanup);
		else
			assert(m_pfDef->getFStackPurge() == nFPUStackCleanup);
	}

	m_dc.ATTRIBS = 0;
	return 1;
}
*/


/*int TypeProc_t::GetReturnType(TYPE _t *t)
{
	if (!fu ncdef()->exitOps().empty())
	{
		OpPtr rOp(fun cdef()->exitOps().front());
		rOp->GetType(*t);
		return 1;
	}	
	return 0;
}*/

/*int	TypeProc_t::RetAddrSize()
{
	return func def()->RetAddrSize();
}*/

/*
	if (!m_pExitPts)
		return 0;
	OpPtr pOpRet = m_pExitPts->pOp;
	assert(pOpRet->IsRetOp());

	if (!pOpRet->GetA rgs())
		return 0;
	if (pOpRet->Get Args()->CheckCount(1) != 0)
		return 0;
	return pOpRet->GetA rgs()->pOp->optyp;
}*/


/*int TypeProc_t::ReNumberize()
{return 0;
	if (!(m_dc & FUI_NUMBERS))
		return -1;//not dirty

	int nId = 0;
	PathPtr pBody = func def()->Body();
	if (pBody)
	{
		fun cdef()->pathTree().__NumberizeOps(pBody, nId);
		TRACE1("%s: Renumbering ops...\n", typ eobj()->na mexx().c_str());
	}

	m_dc &= ~FUI_NUMBERS;
	return 1;
}*/

/*bool check_cpuin(uint32_t cpuin, uint32_t pid)
{
	int i = 0;
	while (cpuin)
	{
		uint32_t id = (cpuin & 0xF);
		if (id)
		if (id  == (pid & 0xF) || id == ((pid & 0xF0) >> 4))
			return true;
		cpuin >>= 4;
	}

	return false;
}*/



/*uint32_t TypeProc_t::GetFuncEndAddr()
{
	uint32_t addr1 = offset();
	if (addr1 != BAD_ADDR)
	{
		FieldPtr pLabel(FUNCD EF(this)->getTailLabel());
		if (pLabel)
		{
			uint32_t addr2 = pLabel->offset();
			if ((addr2 != BAD_ADDR) && (addr2 > addr1))
				return addr2;
		}
	}

	return BAD_ADDR;
}*/

/*int TypeProc_t::SetFuncEndAddr(uint32_t func_end)
{
	uint32_t func_beg = offset();
	if (func_beg == BAD_ADDR)
		return 0;

	if (func_end <= func_beg)
		return 0;

	for (FieldMapIt it = mFields.begin(); it != mFields.end(); it++)
	{
		FieldPtr  pLoc = VALUE(it);
		if ((uint32_t)pLoc->offset() >= func_end)
			break;

		if (pLoc->IsObjFunc())
		{
			func_end = pLoc->offset();
			break;
		}
	}

	assert(!FUNC DEF(this));
	/ *?FuncInfo_t an(G DC, *func def());
	FieldPtr pLabel = an.CreateTailPath();
	pLabel->setOffset(func_end);* /
	return 1;
}*/





//////////////////////////////////////////////////

void TypeProc_t::aka(MakeAlias &buf) const
{
	buf.forTypeProc();
}

void TypeProc_t::namex(MyString &s) const
{
	s = TYPEID_PROC;
}

//@@@
/*int TypeProc_t::size(CTypePtr iSelf) const
{
	int sz = Struc_t::size(iSelf);
	assert(sz != 0);
	//if (sz == 0)
		//return 1;
	return sz;
}*/


/*ADDR TypeProc_t::base() const
{
	//assert(0);
	FieldPtr pField(type obj()->parentField());
	assert(pField);
	//assert(pField->_key() == mBase);
	return pField->_key();
}*/










