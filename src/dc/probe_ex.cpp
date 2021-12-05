#include "probe_ex.h"
#include "db/type_struc.h"
#include "db/main.h"
#include "type_funcdef.h"
#include "dump.h"
#include "info_dc.h"
#include "expr_term.h"


////////////////////////////////////////////////////
// VAList

void FuncInfo_t::VAListFromOp(VAList& self, CHOP pOp)
{
	self.setBase(DockAddr());
	self.setCur(OpVA(pOp));
	VAListFromOp0(self, pOp);
	if (FuncInfo_t::PathOf(pOp)->tailOp() == pOp)
	{
		HPATH pLogicsTop(GetLogicsTop(PathOf(pOp), 1));
		if (pLogicsTop && pLogicsTop->Type() & BLK_LOGIC)
		{
			PathTree_t::LeafIterator i(pLogicsTop->Children());
			++i;//skip openning path of logic expression
			for (; i; i++)
			{
				HPATH pPath(i.data());
				for (PathOpList_t::Iterator i(pPath->ops()); i; i++)
					VAListFromOp0(self, PRIME(i.data()));
			}
		}
	}
	//print();
}

void FuncInfo_t::VAListFromOp(VAList& self, const Out_t *pOut, ADDR base)
{
	self.setBase(base);
	self.setCur(-1);
	self.clear();
	if (pOut)
	{
		if (pOut->dockOp())
			self.setCur(OpVA(pOut->dockOp()));
		for (Out_t::Iterator i((Out_t *)pOut); i; ++i)
		{
			Out_t *pOut(i.top());
			if (pOut->dockOp())
			{
				ADDR va(OpVA(pOut->dockOp()));
				self.insert(va);
			}
		}
	}
}

void FuncInfo_t::VAListFromOp0(VAList& self, CHOP hOp)
{
	if (IsPrimeOp(hOp))
	{
		if (self.add1(OpVA(hOp)))//is this branch has already been processed?
		{
			for (XOpList_t::Iterator j(hOp->m_xin); j; j++)
			{
				OpPtr pOp2 = j.data();
				if (!IsPrimeOp(pOp2) || !pOp2->isRoot())
					VAListFromOp0(self, pOp2);
			}

			for (OpList_t::Iterator i(hOp->argsIt()); i; i++)
			{
				OpPtr pOp(i.data());
				for (XOpList_t::Iterator j(pOp->m_xin); j; j++)
				{
					OpPtr pOp2 = j.data();
					if (!IsPrimeOp(pOp2) || !pOp2->isRoot())
						VAListFromOp0(self, pOp2);
				}
			}
		}
	}
	else if (IsCallOutOp(hOp))
	{
		OpPtr pOp(PrimeOp(hOp));
		if (!pOp->isRoot())
			VAListFromOp0(self, pOp);
	}
}

///////////////////////////////////////////////////
// ProbeEx_t

/*void ProbeEx_t::print(std::ostream &os)
{
os << (pFile ? pFile->m.theName() : "?");
os << ", " << (pCplx ? pCplx->na mexx() : "?");
os << ", " << std::hex << VA();
}*/

void ProbeEx_t::setupLocality(bool bHeader)
{
	using namespace adcui;
	Locality_t locality(CXTID_SOURCE, 0);
	if (scope())
	{
		LocusIdEnum e;
		if (bHeader)
		{
			if (pickedTypeDecl())
				e = LocusId_STRUC_HEADER;
			else if (pickedFuncDecl())
				e = LocusId_STRUC_METHOD;
			else if (pickedFieldDecl())
			{
				if (DcInfo_t::IsGlobal(pickedFieldDecl()))
					e = LocusId_STRUC_STATIC;
				else
					e = LocusId_STRUC_DATA;
			}
			else if (mAtPos.is_set())
				e = LocusId_STRUC_DATA;//on a gap/strucend
			else
				e = LocusId_STRUC_BODY;
		}
		else
		{
			if (scope()->typeFuncDef())
			{
				if (pickedFuncDecl())
					e = LocusId_FUNC_HEADER;
				else if (opLine())
					e = LocusId_FUNC_OP;
				else if (pickedFieldDecl())
				{
					if (DcInfo_t::IsGlobal(pickedFieldDecl()))
						e = LocusId_FUNC_LABEL;
					else
						e = LocusId_FUNC_LOCAL;
				}
				else
					e = LocusId_FUNC_BODY;
			}
			else//struclocs in func
			{
				if (pickedTypeDecl())
					e = LocusId_STRUC_HEADER;
				else if (pickedFieldDecl())
				{
					if (DcInfo_t::IsGlobal(pickedFieldDecl()))
					{
						CGlobPtr pGlob(DcInfo_t::GlobObj(pickedFieldDecl()));
						if (pGlob->func())
							e = LocusId_STRUC_METHOD;
						else
							e = LocusId_STRUC_STATIC;
					}
					else
						e = LocusId_STRUC_DATA;
				}
				else
					e = LocusId_STRUC_BODY;
			}
		}
		locality.setScoping(e);
	}
	if (obj())
		locality.obj = 1;
	setLocality(locality);
}

ADDR ProbeEx_t::VA(const Dc_t &rDC) const
{
	if (pickedFieldDecl())
		return pickedFieldDecl()->_key();
	if (!mpScope)
		return 0;
	if (!mpScope->typeProc())
		return 0;
	ADDR base(scope()->parentField()->_key());
	if (!mpOpLine)
	{
		//if (bIsFuncDecl)
		if (locality().scoping == adcui::LocusId_FUNC_HEADER)
			return base;
		return 0;
	}
	CGlobPtr iGlob(DcInfo_s::GlobFuncObj(scope()->parentField()));
	FuncInfo_t FI(rDC, *iGlob);
	ADDR offs(FI.OpVA(mpOpLine));
	if (offs == -1)//?
		return 0;
	return offs;
}

/*TypePtr ProbeEx_t::func() const
{
	return mpScope && mpScope->type Proc() ? mpScope : nullptr;
}*/

GlobPtr ProbeEx_t::scopeFunc() const
{
	if (mpScope && mpScope->typeFuncDef())
		return mpScope->objGlob();
	return nullptr;
}

TypePtr ProbeEx_t::scopeStruc() const
{
	if (mpScope && mpScope->typeStruc())
		return mpScope->objTypeGlob();
	return nullptr;
}

FileDef_t *ProbeEx_t::fileDef() const
{
	CFolderPtr pFolder(folder());
	if (!pFolder)
		return nullptr;
	return pFolder->fileDef();
}

/*bool ProbeEx_t::setDcRef()
{
	pDcRef = nullptr;
	for (const_reverse_iterator i(rbegin()); i != rend(); i++)
	{
		const Frame_t &f(*i);
		assert(f.cont());
		if (f.cont()->typeModule())
		{
			pDcRef = DCREF(f.cont());
			if (pDcRef)
				return true;
		}
	}
	return false;
}*/

Dc_t *ProbeEx_t::dcRef() const
{
	if (locus().empty())
		return nullptr;
	CTypePtr iModule(locus().module());
	if (!iModule)
		return nullptr;
	return (Dc_t *)DCREF(iModule);
}






