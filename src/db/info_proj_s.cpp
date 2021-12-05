#include "info_proj.h"
#include "type_seg.h"
#include "type_proc.h"
#include "type_proxy.h"
#include "clean.h"
#include "main.h"
#include "ui_main.h"
#include "anlzbin.h"
#include "dump_visit.h"
#include "dump_bin.h"

bool ProjectInfo_s::gTraceObjLifetime = false;

TypePtr ProjectInfo_s::TypeTop(TypePtr p0)
{
	TypePtr p(p0);
	while (!p->owner()->typeSeg())
		p = p->owner();
	return p;
}


bool ProjectInfo_s::IsImpOrExp(CFieldPtr pField)
{
	CFieldPtr pField2(pField);
	if (IsTypeImp(pField2) || IsExported(pField2))
		return true;
	return false;
}

bool ProjectInfo_s::IsPhantomModule(CTypeBasePtr iModule)
{
	assert(iModule->typeModule());
	return iModule->typeSeg()->rawBlock().empty();
}

bool ProjectInfo_s::IsTypeImp(CFieldPtr pField)
{
	return pField->isTypeImp();
}

bool ProjectInfo_s::IsExported(CFieldPtr pField)
{
	CFieldPtr pField2(pField);
	if (IsEntryLabel(pField2))
	{
		assert(!pField2->isExported());
		//if (pField2->isExported())
			//abort();
		pField2 = pField2->owner()->parentField();
	}
	return pField2->isExported();
}

FieldPtr ProjectInfo_s::FindFieldByNameInSegs3(CTypePtr iSelf, const char *name)
{
	Seg_t &rSelf(*iSelf->typeSeg());

	ADDR a;
	ObjId_t objId(Field_t::IsDefaultName(name, a));
	if (objId != OBJID_NULL)
	{
		FieldPtr pField(FindFieldByAddrInSegs(iSelf, a));
		//FieldPtr pField(dc()->primeSeg()->FindFieldV(a));
		while (pField)
		{
			ADDR a(pField->_key());
//?			pField = pField->parentField();
	//?		if (!pField || pField->_key() != a)
		//?		return nullptr;
			TypePtr pType(pField->type());
			if (pType && pType->ObjType() == objId)
				break;
		}
		return pField;
	}
	assert(!isempty(name));
	return FindFieldByNameInSegs2(iSelf, name);
}

const MyString &ProjectInfo_s::ModuleTitle(CTypePtr iModule)
{
	const Module_t &rModule(*iModule->typeModule());
	if (!rModule.subTitle().empty())
		return rModule.subTitle();
	return rModule.title();//the derivative modules are having the same title as their progenitor
}

const MyString &ProjectInfo_s::ModuleTitle(CFolderPtr pFolder)
{
	return ModuleTitle(ModuleOf(pFolder));
}

FieldPtr ProjectInfo_s::CloneLead(CFieldPtr pField)
{
	const FieldMap& m(pField->owner()->typeStruc()->fields());
	FieldMapCIt it(pField);
	if (it == m.begin())
		return (FieldPtr)pField;
	--it;
	if (KEY(it) != pField->_key())
		return (FieldPtr)pField;
	return (FieldPtr)VALUE(m.lower_bound(pField->_key()));
}


unsigned ProjectInfo_s::ChopName(const MyString &s0, MyString &s, char chopSymb)//ref: ForceName
{
	unsigned u(0);
	s = s0;
	if (chopSymb != DUB_SEPARATOR)//means: do not chop!
	{
		if (s0.back() == DUB_TERMINATOR)
		{
			size_t n(s0.rfind(DUB_SEPARATOR));
			assert(n != std::string::npos);
			s.resize(n);
			u = atoi(s0.substr(n + 1).c_str());
			assert(u != 0);
		}
		if (u > 0 && chopSymb)
		{
			std::ostringstream ss;
			ss << chopSymb << u;
			s.append(ss.str());
			return 0;
		}
	}
	return u;
}


ROWID ProjectInfo_s::VA2DA(CTypePtr iSelf, ADDR a)
{
	Seg_t *pSeg(iSelf->typeSeg());
	if (pSeg)
		return pSeg->viewOffsAt(iSelf, a);
	if (!iSelf->parentField())
		return ROWID_INVALID;//? a;
	a -= iSelf->base();
	a += iSelf->parentField()->_key();
	return VA2DA(iSelf->parentField()->OwnerComplex(), a);
}

TypePtr ProjectInfo_s::OwnerSeg(CTypePtr iType)
{
	assert(iType);
	while (!iType->typeSeg())
	{
		FieldPtr pField(iType->parentField());
		if (pField)
			iType = pField->owner();
		else
			iType = iType->owner();
		if (!iType)
			break;
	}
	return (TypePtr)iType;
}

TypePtr ProjectInfo_s::OwnerSeg0(CTypePtr iType)
{
	assert(iType);
	while (!iType->typeSeg())
	{
		FieldPtr pField(iType->parentField());
		if (!pField)
			return nullptr;
		iType = pField->owner();
		if (!iType)
			break;
	}
	return (TypePtr)iType;
}

unsigned ProjectInfo_s::SegOffset(CFieldPtr pField)
{
	return pField->_key() - OwnerSeg(pField->owner())->base();
}

TypePtr ProjectInfo_s::ModuleOf(CFieldPtr pField)
{
	TypePtr iType(pField->owner());
	assert(iType);
	for (;;)
	{
		Seg_t *pSeg(iType->typeSeg());
		if (pSeg)
		{
			if (pSeg->typeModule())
				break;
			if (pSeg->typeProject())
				return nullptr;
			iType = pSeg->superLink();
			assert(iType);
			continue;
		}

		pField = iType->parentField();
		if (!pField)
		{
			if (iType->isShared())
			{
				if (!iType->ownerTypeMgr())
					return nullptr;
				iType = iType->ownerTypeMgr()->owner();
			}
			else
			{
				if (!pSeg)
					return nullptr;//some local var?
				if (!pSeg->superLink())
				{
					assert(iType->typeModule());
					break;
				}
				iType = pSeg->superLink();
			}
		}
		else
			iType = pField->owner();
	}
	return iType;
}

TypePtr ProjectInfo_s::FrontSeg(CTypePtr iSeg)
{
	if (!iSeg->typeSeg())
		iSeg = OwnerSeg(iSeg);
	while (iSeg)
	{
		if (iSeg->typeSeg()->frontIndex() > 0)
			return (TypePtr)iSeg;
		iSeg = iSeg->typeSeg()->superLink();
	}
	return (TypePtr)iSeg;
}


ROWID ProjectInfo_s::ViewOffset(CTypeBasePtr pType)
{
	if (pType->typeSeg())
	{
		if (pType->typeModule())
		{
			if (IsPhantomModule(pType))
				return 0;
		}
		return pType->typeSeg()->viewOffs();
	}
	return 0;
}

ROWID ProjectInfo_s::ViewSize(CTypeBasePtr pSelf, ROWID *pda)
{
	Seg_t *pSeg(pSelf->typeSeg());
	if (pSeg)
	{
		if (pSelf->typeModule())
		{
			if (IsPhantomModule(pSelf))
			{
				CTypePtr pSeg(FindFrontSegIn(pSelf));
				if (pSeg)
					return Size(pSeg);
			}
		}
		return pSeg->viewSize0();
		/*ROWID vsz(pSeg->viewSize0());
		if (vsz == -1)
			return 0;
		ROWID tsz(SegTraceSize(pSelf));
		ROWID sz(pSeg->size(pSelf));
		if (tsz > sz)
			vsz += (tsz - sz);
		//if (vsz == 0)
			//vsz = tsz;
		return vsz;*/
	}
	return Size(pSelf, pda);
}

bool ProjectInfo_s::CheckStrucEnd(FieldMapIt i, FieldMap &m)
{
	if (i == m.end())
		return true;
	if (IsEosField(VALUE(i)))
	{
		assert(++i == m.end());
		return true;
	}
	return false;
}

bool ProjectInfo_s::CheckStrucEnd(FieldMapCIt i, const FieldMap &m)
{
	if (i == m.end())
		return true;
	if (IsEosField(VALUE(i)))
	{
		assert(++i == m.end());
		return true;
	}
	return false;
}

TypePtr ProjectInfo_s::SkipBitset(CTypePtr p)
{	
	if (p && p->typeBitset())
		return p->parentField()->owner();
	return (TypePtr)p;
}

TypePtr ProjectInfo_s::CommonScope(CTypePtr p0, CTypePtr q0)
{
#if(0)
	for (CTypePtr p(p0); p; p = p->ownerScope())
		for (CTypePtr q(q0); q; q = q->ownerScope())
			if (p == q)
				return (TypePtr)p;
#else//faster! (but uglier)
	int i(0), j(0);
	for (CTypePtr p(p0); p; p = p->ownerScope())//count nodes in 1st list
		++i;
	for (CTypePtr q(q0); q; q = q->ownerScope())//count nodes in 2nd list
		++j;
	CTypePtr pz(p0), qz(q0);//determine a longer one (pz) and get nodes count difference
	int d(i - j);
	if (d < 0)
	{
		std::swap(pz, qz);
		d = -d;
	}
	if (qz)//if a shorter one is empty - there is no common node
	{
		for (; d > 0; d--)//skip dif of nodes for larger list
			pz = pz->ownerScope();
		for (; pz && qz; pz = pz->ownerScope(), qz = qz->ownerScope())//iterate 2 lists until an intersection is found
			if (pz == qz)
				return (TypePtr)pz;
	}
#endif
	return nullptr;
}

FieldPtr ProjectInfo_s::EntryField(CTypePtr iFunc)//may be not a func
{
	if (!iFunc)
		return nullptr;
	const Struc_t &rStruc(*iFunc->typeStruc());
	FieldMapCIt it(rStruc.fields().begin());
	if (it == rStruc.fields().end())
		return nullptr;
	CFieldPtr pField(VALUE(it));
	if (pField->_key() != iFunc->base())
		return nullptr;
	return (FieldPtr)pField;
}

bool ProjectInfo_s::IsProc(CTypeBasePtr iSelf)
{
/*	if (iSelf->typeStruc())
	{
		FieldPtr pField(iSelf->parentField());
		//if (!iSelf->isShared())
		if (pField && pField->owner()->typeSeg())
		{
			assert(!iSelf->isShared());
			FieldPtr pEntry(EntryField(iSelf));
			if (pEntry && pEntry->type() && pEntry->type()->typeCode())
				return true;
		}
	}
	return false;*/
	return iSelf->typeProc() != nullptr;
}

bool ProjectInfo_s::IsEntryLabel(CFieldPtr pSelf)
{
	if (!pSelf->isTypeCode())
		return false;
	TypePtr iOwner(pSelf->owner());
	if (iOwner->typeSeg())
		return false;
	assert(!iOwner->isShared() && iOwner->parentField()->owner()->typeSeg() && iOwner->typeStruc());
	FieldPtr pEntry(EntryField(iOwner));
	return (pEntry == pSelf);
}


TypePtr ProjectInfo_s::SkipModifier(CTypePtr pType)
{
	while (pType)
	{
		TypeModifier_t *pModif(pType->typeModifier());
		if (!pModif)
			break;
		pType = pModif->incumbent();
	}
	return (TypePtr)pType;
}

FieldMapIt ProjectInfo_s::StrucFieldIt(CTypePtr iSelf, ADDR offs, FieldMapIt *ppNext)
{
	Struc_t &rSelf(*iSelf->typeStruc());
	FieldMap &m(rSelf.fields());

	if (ppNext != nullptr)
		*ppNext = m.end();

/*?	if (rSelf.size0() > 0)//size()
		if (offs - iSelf->base() >= (ADDR)iSelf->size())
			return l.end();
*/
	FieldMapIt it_l(m.lower_bound(offs));
	if (it_l != m.end() && IsEosField(VALUE(it_l)))
		it_l++;//skip eos?


	FieldMapIt it_pr = m.end();

	if (it_l != m.end())
	{
		ADDR addr_l = KEY(it_l);

		if (addr_l == offs)
		{
			if (ppNext != nullptr)
			{
				FieldMapIt it_nx(it_l);
				it_nx++;
				//it_nx = m.SkipEos(it_nx);
				if (it_nx != m.end() && IsEosField(VALUE(it_nx)))
					it_nx++;
				if (it_nx != m.end())
					*ppNext = it_nx;
			}
			return it_l;
		}

		//addr_l > offs

		if (ppNext != nullptr)
		{
			*ppNext = it_l;
		}

		if (it_l != m.begin())
		{
			it_pr = it_l;
			it_pr = EosAwarePrior(m, it_pr);
		}
	}
	else
	{
		if (!m.empty())
			it_pr = EosAwarePrior(m, it_pr);
	}

	return it_pr;
}

FieldMapIt ProjectInfo_s::FieldIt(CTypePtr iSelf, ADDR va, FieldMapIt * pitnx, FieldIt_Mode mode)
{
	Struc_t &rSelf(*iSelf->typeStruc());
	FieldMap &l(rSelf.fields());

	FieldMapIt itnx(l.end());
	FieldMapIt it = StrucFieldIt(iSelf, va, &itnx);

	if (it != l.end())
	{
		FieldPtr  f = VALUE(it);
		ADDR offs = KEY(it);
		assert(offs <= va);

		switch (mode)
		{
		default:
		case FieldIt_Prev://field may not overlap the address
			break;
		case FieldIt_Overlap://field may not locate at the address but must overlap it
			{
				int sz = f->size();
				if (sz <= 0)
					sz = 1;
				if (va >= offs+sz)
					it = l.end();
			}
			break;
		case FieldIt_Exact://field must locate at the address
			if (offs != va)
				it = l.end();
			break;
		}
	}

	if (pitnx)
		*pitnx = itnx;
	return it;
}

FieldPtr ProjectInfo_s::Field(CTypePtr iSelf, ADDR va, FieldPtr * ppNext, FieldIt_Mode mode)
{
	FieldMap &l(iSelf->typeStruc()->fields());

	FieldMapIt itnx(l.end());
	FieldMapIt it = FieldIt(iSelf, va, &itnx, mode);
	FieldPtr  f = nullptr;
	if (it != l.end())
		f = VALUE(it);
	if (ppNext)
	{
		*ppNext = nullptr;
		if (itnx != l.end())
			*ppNext = VALUE(itnx);
	}
	return f;
}


TypesMgr_t *ProjectInfo_s::NewTypesMgr(TypePtr iSelf)
{
	Struc_t &rSelf(*iSelf->typeStruc());
	assert(!rSelf.typeMgr());
	rSelf.setTypesMgr0(new TypesMgr_t);
	rSelf.typeMgr()->setOwner(iSelf);
	iSelf->flags() |= TYP_HAS_TMAP;
	return rSelf.typeMgr();
}

void ProjectInfo_s::DeleteTypesMgr(TypePtr iSelf)
{
	Struc_t &rSelf(*iSelf->typeStruc());
	if (rSelf.typeMgr())
	{
		assert(rSelf.typeMgr()->empty());
		delete rSelf.typeMgr();
		rSelf.setTypesMgr0(nullptr);
		iSelf->flags() &= ~TYP_HAS_TMAP;
	}
}


FieldPtr ProjectInfo_s::__findFieldV(CTypePtr iStruc, ADDR addr0, FieldIt_Mode mode, bool bDeep)
{
	const Struc_t &rSelf(*iStruc->typeStruc());

#if(1)
	const FieldMap& m(rSelf.fields());
	FieldMapCIt it(m.upper_bound(addr0));//always greater
	if (it == m.begin())
		return nullptr;//no field above

	it = m.Priorz(it);

	CFieldPtr f(VALUE(it));
	assert(f);

	FieldMapCIt it1(m.lower_bound(f->_key()));//check U-field
	if (it1 != it)
	{
		f = VALUE(it1);
	}

	if (bDeep)
	{
		if (f->type() && !f->type()->isShared())
		{
			TypePtr iStruc(f->isTypeStruc());
			if (iStruc)
			{
				CFieldPtr f2(__findFieldV(iStruc, addr0, mode, true));
				if (f2)
					return (FieldPtr)f2;
			}
		}
	}

	if (f->_key() == addr0)
		return (FieldPtr)f;

	//field's address is less
	if (mode != FieldIt_Exact)
	{
		if (mode == FieldIt_Prev)
			return (FieldPtr)f;
		assert(mode == FieldIt_Overlap);
		if (f->size() > 0)
			if (addr0 < f->addressHi())
				return (FieldPtr)f;
	}

	return nullptr;

#else
	for (FieldMapIt it = mFields.begin(); it != mFields.end(); it++)
	{
		FieldPtr f(VALUE(it));
		TypePtr iStruc(f->typeStruc());
		if (iStruc)
		{
			FieldPtr pLoc(__findFieldV(iStruc, addr0, mode));
			if (pLoc)
				return pLoc;
		}
	}

	for (FieldMapIt it = mFields.begin(); it != mFields.end(); it++)
	{
		FieldPtr f(VALUE(it));
		int va = f->_key();
		if (va == addr0)
			return f;
	}
#endif
	return nullptr;
}


// T * <name>	=> name is a ptr to T
// <name>
//   \_ *
//       \_ T
TypePtr ProjectInfo_s::IsPtrToStruc(CTypePtr pType, bool bSkipModifier)
{
	if (!pType)
		return nullptr;
	TypePtr_t *pPtr(pType->typePtr());
	if (!pPtr)
		return nullptr;
	TypePtr iBase(pPtr->pointee());
	if (!iBase)
		return nullptr;
	if (bSkipModifier)
		iBase = ProjectInfo_t::SkipModifier(iBase);
	assert(iBase);
	//iBase = ProjectInfo_t::SkipProxy(iBase);
	assert(!iBase->typeProxy());
	if (!iBase->typeStruc())
		return nullptr;
	return iBase;
}

// const T * <name>	=> name is a ptr to const T
// <name>
//   \_ *
//       \_ const
//               \_ T
TypePtr ProjectInfo_s::IsPtrToConstStruc(CTypePtr p, bool bSkipModifier)
{
	if (!p)
		return nullptr;
	TypePtr_t *pPtr(p->typePtr());
	if (!pPtr)
		return nullptr;
	TypeConst_t *pConst(pPtr->pointee()->typeConst());
	if (!pConst)
		return nullptr;
	p = pConst->baseType();
	if (bSkipModifier)
		p = SkipModifier(p);
	assert(p);
	//p = SkipProxy(p);
	assert(!p->typeProxy());
	if (!p->typeStruc())
		return nullptr;
	return (TypePtr)p;
}


// T * const <name>	=> name is a const ptr to T	(x)		<- check for this
// <name>
//   \_ const
//           \_ *
//               \_ T
TypePtr ProjectInfo_s::IsConstPtrToStruc(CTypePtr p, bool bSkipModifier)
{
	if (!p)
		return nullptr;
	TypeConst_t *pConst(p->typeConst());
	if (!pConst)
		return nullptr;
	TypePtr_t *pPtr(pConst->baseType()->typePtr());
	if (!pPtr)
		return nullptr;
	p = pPtr->pointee();
	if (!p)
		return nullptr;
	if (bSkipModifier)
		p = SkipModifier(p);
	assert(p);
	//p = SkipProxy(p);
	assert(!p->typeProxy());
	if (!p->typeStruc())
		return nullptr;
	return (TypePtr)p;
}


// T * const <name>	=> name is a const ptr to T	(x)		<- check for this
// <name>
//   \_ const
//           \_ *
//               \_ const
//                       \_ T
TypePtr ProjectInfo_s::IsConstPtrToConstStruc(CTypePtr p, bool bSkipModifier)
{
	if (!p)
		return nullptr;
	TypeConst_t *pConst(p->typeConst());
	if (!pConst)
		return nullptr;
	TypePtr_t *pPtr(pConst->baseType()->typePtr());
	if (!pPtr)
		return nullptr;
	p = pPtr->pointee();
	if (!p)
		return nullptr;
	pConst = p->typeConst();
	if (!pConst)
		return nullptr;
	p = pConst->baseType();
	assert(p);
	if (bSkipModifier)
		p = SkipModifier(p);
	//p = SkipProxy(p);
	assert(!p->typeProxy());
	if (!p->typeStruc())
		return nullptr;
	return (TypePtr)p;
}


bool ProjectInfo_s::IsInside(CTypePtr iSelf, ADDR va)
{
	return (iSelf->base() <= va && va < iSelf->base() + iSelf->size());
}


void ProjectInfo_s::AcquireTypeRef(TypePtr iType)
{
	if (!iType)
		return;

CHECKID(iType, 0x9485)
//OutputDebugString("+\n");
STOP
#if(NO_TYPE_PROXIES)
	assert(!iType->typeProxy());
#endif

	TypesMgr_t *pTypeMgr(iType->ownerTypeMgr());
	//if (mpType->isShared())
	if (pTypeMgr)
	{
		pTypeMgr->addTypeRef(iType);
	}
	else
	{
		iType->addRef();
	}

	//mpBaseType->addRef();
	//if (mpBaseType->typeComplex() && type obj())
	//mpBaseType->typeComplex()->AddObjRef(type obj(), G_Me mMgr);
}


/*bool ProjectInfo_s::IsUglyType(TypePtr p)
{
	return p->isUgly();
}

TypePtr ProjectInfo_s::IsUglyLocum(TypePtr p)
{
	Typedef_t *pTypedef(p->typeTypedef());
	if (!pTypedef)
		return nullptr;
	TypePtr iBase(SkipProxy(pTypedef->baseType()));//may be a proxy while relocating
	if (iBase->isUgly())
		return iBase;
	return nullptr;
}

TypePtr ProjectInfo_s::SkipUglyLocum(TypePtr p)
{
	TypePtr p2(IsUglyLocum(p));
	return p2 ? p2 : p;
}*/

void ProjectInfo_s::terminalFieldAt(Locus_t &aLoc, unsigned bitOffs)
{
	while (terminalFieldAt0(aLoc, bitOffs))
	{
		STOP
	}
}

bool ProjectInfo_s::terminalFieldAt0(Locus_t &aLoc, unsigned bitOffs)//TypePtr iSelf, DA_t &va, Locus_t &l, Block_t rb) const
{
	Frame_t &aTop(aLoc.back());
//CHECKID(aTop.cont(), -105)
CHECK(aTop.addr() == 0x1008bf1)
STOP
	assert(aTop.struc());
	assert(!aTop.cont()->typeProxy());
	Struc_t &rSelf(*aTop.struc());
	if (rSelf.typeStrucvar())
		return false;

	//find a field at given address or the one preceeding it - MAY NOT OVERLAP!
	FieldMapCIt it(rSelf.fields().upper_bound(aTop.addr()));//always upper
	if (it == rSelf.fields().begin())//no fields above
//		if (rSelf.fields().empty())
			return false;
	--it;//now it is at addr or lower

	it = rSelf.fields().lower_bound(KEY(it));//ajustment for u-fields (--?)

	CFieldPtr pField(VALUE(it));
//CHECKID(pField, 0x40ff)
//STOP
	if (aTop.addr() == pField->_key())
		aTop.setField((FieldPtr)pField);

	TypePtr pType(pField->type());
	if (!pType)
		return false;//no type?

	//ADDR iAddr(pField->_key());

//?	if (va.adjust != 0 && !pType->typeStruc())//if selected outside of struc's range, go deep as long as there is a compound type
//?		return false;

	int iSize;
	if (aTop.cont()->typeBitset())
		iSize = BitSize(pType);
	else
		iSize = pField->sizeBytes();
	if (iSize == 0)
		iSize = OPSZ_BYTE;
	if (iSize > 0)//not the code and not of a zero size
	{
		if (pField->_key() + iSize <= aTop.addrx())//va.rowj())
		{
			//l.push_back(Frame_t(rb, iSelf, (ADDR)va.row, nullptr));
			return false;//underlaps
		}
	}
	else//iSize < 0
	{
		TypeProc_t *pFunc(pType->typeProc());
		if (pFunc)
		{
			FieldMap &m(pFunc->fields());
			if (!m.empty())
			{
				FieldMapRIt i(m.rbegin());
				ADDR addr_end(i->_key());
				if (!IsEosField(VALUE(i)))
					addr_end += OPSZ_BYTE;//one more
				if (aTop.addrx() >= addr_end)
					return false;
			}
		}
	}

	TypePtr iStrucvar(pType);
	Strucvar_t *pStrucvar(iStrucvar->typeStrucvar());
	if (pStrucvar)
	{
		if (aTop.Block_t::empty())
			return false;//?
		Block_t rb2(aTop);
		rb2.subspace(pField->offset(), iSize);
		//va.row = (va.row - pField->_key()) + iStrucvar->base();
		//ROWID va2(va.row + iStrucvar->base());
#if(0)
		//ProjectInfo_t projInfo(PROJ);
		Binar yContext_t bc(rb2.ptr, 0, -1, 0);
		StrucVarPicker_t probe(*this, *iStrucvar, bc, va2);
		ADDR at2;
		FieldPtr pField2(probe.pick(at2));
		if (pField2)
		{
			rb2.size = probe.sizez();
			l.push_back(Frame_t(rb, iSelf, pField->offset(), pField));
			l.push_back(Frame_t(rb2, pField->type(), at2, pField2));
			//return pStrucvar->terminalFieldAt(va, l, rb2);
			return true;
		}
#endif
		//if (va.row != pField->_key())
		//	pField = nullptr;
		//l.push_back(Frame_t(rb, iSelf, (ADDR)pField->offset(), pField));
		ADDR iOver(aTop.setField2((FieldPtr)pField));
		aLoc.push_back(Frame_t(rb2, iStrucvar, iOver/* + iStrucvar->base()*/, nullptr));
		return false;//no more
	}

	TypePtr iStruc(pField->typex());
	Struc_t *pStruc(iStruc->typeStruc());
	if (pStruc)
	{
		Bitset_t *pBitset(pStruc->typeBitset());
		if (pBitset)
		{
			unsigned bytes(iStruc->sizeBytes());
			ADDR iOver(aTop.setField2((FieldPtr)pField));
			Block_t rb2(aTop);
			rb2.subspace(pField->offset(), bytes);
			//aLoc.push_back(Frame_t(rb2, iSelf, iAddr, pField));
			//va.row -= iAddr;
			//va.row += iStruc->base();
//			DA_t bit(iOver * CHAR_BIT + va.bit, 0, 0);
			ADDR iOverBits(iOver * CHAR_BIT + bitOffs);
//?			if (!terminalFieldAt(iStruc, bit, l, rb))
			aLoc.push_back(Frame_t(rb2, iStruc, iOverBits, nullptr));
			return true;
		}
		/*l.push_back(Frame_t(rb.subspace(offs - iSelf->base(), iSize), iSelf, iAddr, pField));
		va.row -= iAddr;
		va.row += iStruc->base();
		if (!terminalFieldAt(iStruc, va, l, rb))
		l.push_back(Frame_t(rb, iStruc, (ADDR)va.row, nullptr));*/
		ADDR iOver(aTop.setField2((FieldPtr)pField));
		Block_t rb2(aTop);
		rb2.subspace(pField->offset(), iSize);
		aLoc.push_back(Frame_t(rb2, iStruc, iOver + iStruc->base(), nullptr));
		return true;
	}

	Array_t *pArray(iStruc->typeArray());
	if (pArray)
	{
		ADDR iOver(aTop.setField2((FieldPtr)pField));

		iStruc = pArray->absBaseType();
		//va.row -= iAddr;//bytes to the target
		unsigned iSize(iStruc->size());
		if (iSize == 0)
			iSize = OPSZ_BYTE;
		unsigned uExtra(unsigned(iOver / iSize) * iSize);//num of elements to skip
		aTop.setExtra(uExtra);

		if (!iStruc->typeStruc())
			return false;//no more //the 'o' is discarded

		unsigned o(unsigned(iOver % iSize));//offset in a base
		Block_t rb2(aTop);
		rb2.subspace(pField->offset() + uExtra, iSize);
		aLoc.push_back(Frame_t(rb2, iStruc, o + iStruc->base(), nullptr));
		return true;
	}


	if (pField->_key() < aTop.addr())
	{
		if (pType->typeCode() || pType->typeThunk())
			return false;

//		if (pField->addressHi() > aTop.addr())
	//		return false;
	}

	//Block_t rb2(rb.subspace(pField->_key() - iSelf->base(), iSize));
	//l.push_back(Frame_t(rb2, iSelf, (ADDR)va.row, pField));
	//return true;
	ADDR iOver(aTop.setField2((FieldPtr)pField));//may have gotten inside of a primitive type
	aTop.setExtra(iOver);
	return false;
}

TypesMgr_t *ProjectInfo_s::findTypeMgr(CTypePtr iType)
{
	TypesMgr_t *pTypesMgr(nullptr);
	while (iType)
	{
		Struc_t *pStruc(iType->typeStruc());
		if (pStruc)
			pTypesMgr = pStruc->typeMgr();
		if (pTypesMgr)
			break;

		if (iType->owner())
			pTypesMgr = iType->owner()->typeComplex()->typeMgr();
		if (pTypesMgr)
			break;

		if (iType->typeSeg())
			iType = iType->typeSeg()->traceLink();
		else if (iType->parentField())
			iType = iType->parentField()->OwnerComplex();
		else
			break;
	}
	return pTypesMgr;
}

TypePtr ProjectInfo_s::SkipProxy(CTypePtr p)
{
	while (p)
	{
		TypeProxy_t *proxy(p->typeProxy());
		if (!proxy)
			break;
		p = proxy->incumbent();
	}
	return (TypePtr)p;
}

ROWID ProjectInfo_s::Size(CFieldPtr pField, ROWID *pda)
{
	if (!pField->type())
		return 1;//?
	return Size(pField->type(), pda);
}

//duplication of Struc_t::size!
ROWID ProjectInfo_s::Size(CTypeBasePtr pSelf0, ROWID *pda)//BinaryContext_t *pbc)
{
	CTypePtr pSelf((CTypePtr)pSelf0);
	if (pSelf->typeCode())
		return ROWID_INVALID;

	Struc_t *pStruc(pSelf->typeStruc());
	if (!pStruc || pSelf->typeUnion())
	{
		//assert(!pSelf->typeBit());
		return pSelf->size();
	}

	if (pStruc->typeBitset())
	{
		if (pSelf->parentField() && pSelf->parentField()->owner()->typeStrucvar())
			return ROWID_INVALID;
		return pSelf->sizeBytes();
	}

//	if (pStruc->size0() > 0)
	//	return pStruc->size0();

	if (pStruc->typeStrucvar())
	{
		//StrucvarTracer_t ST(*this, *pSelf, *pbc);
		//return ST.calcSize();
		return ROWID_INVALID;
	}

	ROWID sz(0);
	const FieldMap &m(pStruc->fields());
	if (!m.empty())
	{
		FieldMapCIt it(m.end());
		it = m.Priorz(it);//--it;
		int offs(KEY(it));
		CFieldPtr pFieldLast(VALUE(it));
		if (IsEosField(pFieldLast))
			return offs - pStruc->base(pSelf);

		CTypePtr iType2(pFieldLast->type());
		if (pSelf->typeBitset())
		{
			sz = offs;
			Array_t *pArray(iType2->typeArray());
			if (pArray)
				sz += (int)pArray->total();
			else
				sz += OPSZ_BYTE;//BIT
		}
		else
		{
			sz = offs - pSelf->base();
			ROWID f_sz(0);
			if (iType2)
			{
#if(1)
				//if (!IsTypeBit(iType2))
				{
					if (pda)
					{
						//BinaryContext_t bc(*pbc);
						//bc.advance(offs);
						ROWID da(*pda + offs);
						f_sz = Size(iType2, &da);
					}
					else
						f_sz = Size(iType2);
				}
				//else
					//f_sz = 1;//bit
#else
				f_sz = pFieldLast->size();
#endif
			}
			if (f_sz == ROWID_INVALID)
			{
				return f_sz;
				//assert(0);
			}
			if (f_sz > 0)
				sz += f_sz;
			else
				sz += OPSZ_BYTE;//?
		}
	}

	return sz;
}

NamesMgr_t *ProjectInfo_s::AssureNamespace0(TypePtr p)
{
	assert(p);
	NamesMgr_t *pns(p->typeComplex()->assureNamespace());
	p->flags() |= TYP_HAS_NMAP;
	return pns;
}


TypePtr ProjectInfo_s::OwnerSegRange(CTypePtr iSelf)
{
	assert(iSelf->typeSeg());
	if (!iSelf->parentField())
		return (TypePtr)iSelf;//itself
	return iSelf->parentField()->owner();
}

bool ProjectInfo_s::IsRangeSeg(CTypePtr iSelf)
{
	if (!iSelf->typeSeg())
		return false;
	if (iSelf->parentField())
		return false;
	if (iSelf->typeSeg()->superLink())
		return false;
	if (iSelf->typeModule() || iSelf->typeProject())
		return false;
	return true;
}


TypePtr ProjectInfo_s::OwnerSegList(CTypePtr iType)
{
	if (iType->typeSeg())
		return iType->typeSeg()->superLink();

	FieldPtr pField(iType->parentField());
	if (pField)
		return OwnerSegList(pField->owner());
	//TypePtr iTrace(iType->typeSeg()->traceLink());
	//if (iTrace)
		//return OwnerSegList(iTrace);//?
	return nullptr;
}

bool ProjectInfo_s::CheckDataAtVA(CTypePtr iSeg, ADDR addr)
{
	assert(iSeg->typeSeg());
	if (addr >= iSeg->base())
	{
		ADDR d(addr - iSeg->base());
		if (0 <= d && d < (ADDR)iSeg->typeSeg()->rawBlock().m_size)
		{
			return true;
		}
	}
	return false;
}


MyString ProjectInfo_s::VA2STR(CTypePtr iSeg, ADDR va, int w)
{
	if (iSeg->typeSeg()->isLarge())
		return VA2STR(va, true, iSeg->imageBase(), w);
	assert(iSeg->imageBase() == 0);
	return VA2STR(va, false, 0, w);
}

MyString ProjectInfo_s::VA2STR(ADDR va, bool bLarge, ADDR64 imageBase, int w)
{
	char buf[32];
	if (bLarge)
	{
		ADDR64 va64(imageBase + va);
		if (w <= 0)
			w = HexWidthMax(va64);
		char fmt[8];
		sprintf(fmt, "%%0%d" PRIX64, w);
		sprintf(buf, fmt, va64);
	}
	else
	{
		if (w < 0)
			w = 8;
		else if (w == 0)
			w = HexWidthMax(va);
		assert(imageBase == 0);
		//sprintf(buf, "%08X", va);
		char fmt[8];
		sprintf(fmt, "%%0%dX", w);
		sprintf(buf, fmt, va);
	}
	return buf;
}

MyString ProjectInfo_s::VA2STR(ADDR va)
{
	return MyStringf("%08X", va);
}

MyString ProjectInfo_s::VA2STR(CFieldPtr pField)
{
	CTypePtr pSeg(ProjectInfo_s::OwnerSeg(pField->owner()));
	MyString sVA(ProjectInfo_s::VA2STR(pSeg, pField->_key()));

	return VA2STR(pSeg, pField->_key());
}

size_t ProjectInfo_s::SegTraceSize(CTypeBasePtr iSeg)
{
	const Seg_t &rSeg(*iSeg->typeSeg());
	if (rSeg.traceLink())
		return rSeg.traceLink()->size();
//?	FieldPtr pField(iSeg->parentField());
//?	if (pField)
//?		return rSeg.size(iSeg);
	return (size_t)rSeg.rawBlock().m_size;//?
}


bool ProjectInfo_s::RegisterSubseg(TypePtr iSegMain, ADDR vaInParent, TypePtr iSeg)
{
	Seg_t *pSegMain(iSegMain->typeSeg());
	Seg_t *pSeg(iSeg->typeSeg());
	if (pSeg->superLink() == iSegMain)
	{
		bool bFound(false);
		for (SubSegMapCIt i(pSegMain->subsegs().begin()); i != pSegMain->subsegs().end(); i++)
			if (IVALUE(i) == iSeg)
			{
				bFound = true;
				break;
			}
		return bFound;//already
	}
	pSeg->setSuperLink(iSegMain);
	pSeg->setAddressP(vaInParent);
	if (!iSegMain->typeSeg()->registerSubseg0(iSeg))
		return false;
	iSeg->addRef();
	return true;
}

FieldPtr ProjectInfo_s::FindFieldByAddrInSegs(CTypePtr iSelf, ADDR addr)
{
	Seg_t &rSelf(*iSelf->typeSeg());
	for (SubSegMapCIt i(rSelf.subsegs().begin()); i != rSelf.subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));
		FieldPtr pField(FindFieldByAddrInSegs(iSeg, addr));
		if (pField)
			return pField;
	}
	return Field(iSelf, addr, nullptr, FieldIt_Exact);
}

FieldPtr ProjectInfo_s::FindFieldByName(const char *name, CTypePtr iSelf)
{
	NamesMgr_t *pNS(iSelf->typeComplex()->namesMgr());
	if (pNS)
	{
/*#ifdef _DEBUG
		static bool z(0);
		if (z)
			pNS->check(0);
#endif*/
		Obj_t *pObj(pNS->findObj(name));
		if (pObj)
		{
			FieldPtr pField(pObj->objField());
			assert(pField);
			return pField;
		}
	}
	return nullptr;
}

FieldPtr ProjectInfo_s::FindFieldByName(MyString s, const Locus_t &l)
{
	for (Locus_t::const_reverse_iterator rit(l.rbegin()); rit != l.rend(); rit++)
	{
		const Frame_t &f(*rit);
		Obj_t *pObj(nullptr);
		NamesMgr_t *pNS(f.cont()->typeComplex()->namesMgr());
		if (pNS)
		{
			if (!f.cont()->typeStrucvar())
			{
				//?NamespaceInitialized(f.cont());
				pObj = pNS->findObj(s.c_str());
			}
		}
		if (!pObj)
		{
			if (f.cont()->typeSeg())
			{//check in seglist (seglist are not in a frame list)
				TypePtr iSegList(nullptr);
				FieldPtr pSegField(f.cont()->parentField());
				if (pSegField)
					iSegList = OwnerSeg(pSegField->OwnerComplex());
				if (iSegList)
				{
					pNS = iSegList->typeSeg()->namesMgr();
					if (pNS)
						pObj = pNS->findObj(s.c_str());
				}
			}
		}
		if (pObj)
			return pObj->objField();
	}
	return nullptr;
}

bool ProjectInfo_s::IsEosField(CFieldPtr pField)
{
	TypePtr pOwner(pField->owner());
	if (pOwner->typeSeg())
		return false;
	return IsEosField(pField, pOwner->typeStruc()->fields());
}

bool ProjectInfo_s::IsEosField(CFieldPtr pField, const FieldMap& m)
{
#if(0)
	return (pField->flags() & FLD_EOS) != 0;
#else
	if (VALUE(m.rbegin()) != pField)
		return false;//last field in a struc
	if (pField->type() || pField->name())
		return false;//untyped, unnamed
	return true;
#endif
}

void ProjectInfo_s::SetEosField(FieldPtr pField)
{
#if(0)
	pField->flags() |= FLD_EOS;
#endif
}

FieldPtr ProjectInfo_s::EosField(CTypePtr pType)
{
	assert(pType->typeStruc());
	if (pType->typeStruc()->hasFields())
	{
		CFieldPtr pField(&pType->typeStruc()->fields().back());
		if (IsEosField(pField))
			return (FieldPtr)pField;
	}
	return nullptr;
}


FieldPtr ProjectInfo_s::__findFieldByNameInSegs(CTypePtr iSelf, const char *name)//seg
{
	Seg_t &rSelf(*iSelf->typeSeg());
	for (SubSegMapCIt i(rSelf.subsegs().begin()); i != rSelf.subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));
		FieldPtr pField(__findFieldByNameInSegs(iSeg, name));
		if (pField)
			return pField;
	}
	return FindFieldByName(name, iSelf);//in scope
}

FieldPtr ProjectInfo_s::FindFieldByNameInSegs2(CTypePtr iSelf, const char *name)
{
	MyString s;
	ChopName(name, s);
	return __findFieldByNameInSegs(iSelf, s);
}


ROWID ProjectInfo_s::SegOverSize0(CTypePtr iSelf)
{
	ROWID sz(iSelf->size());
	if (sz == -1)
		return 0;
	//ROWID sz(ViewSize(iSelf));
	ROWID szp(SegTraceSize(iSelf));
	if (sz > szp)
		return (ROWID)(sz - szp);
	return 0;
}

ROWID ProjectInfo_s::SegOverSize(CTypePtr iSelf)
{
	ROWID iOver(0);
	Seg_t &rSelf(*iSelf->typeSeg());
	for (SubSegMapCIt i(rSelf.subsegs().begin()); i != rSelf.subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));
		iOver += SegOverSize(iSeg);
	}
	iOver += SegOverSize0(iSelf);
	return iOver;
}


TypePtr ProjectInfo_s::FindFrontSegIn(CTypeBasePtr iSeg)
{
	Seg_t &rSeg(*iSeg->typeSeg());
	for (SubSegMapCIt i(rSeg.subsegs().begin()); i != rSeg.subsegs().end(); i++)
	{
		TypePtr iSeg2(FindFrontSegIn(IVALUE(i)));//look only in real segments (not seg traces)
		if (iSeg2)
			return iSeg2;
	}
	if (rSeg.frontIndex())
		return (TypePtr)iSeg;
	return nullptr;
}

TypePtr ProjectInfo_s::FindFrontSegUp(CTypePtr pStartSeg)
{
	for (CTypePtr pSeg(pStartSeg); pSeg; pSeg = pSeg->typeSeg()->superLink())
	{
		if (pSeg->typeSeg()->frontIndex())
			return (TypePtr)pSeg;
	}
	return nullptr;
}

#if(0)
NamesMgr_t &ProjectInfo_s::NamespaceInitialized(TypePtr pSelf)
{
	NamesMgr_t &rNS(*pSelf->typeComplex()->namesMgr());
	//assert(rNS.isInitialized());
	//if (!rNS.initialize(pSelf))
		return rNS;
#if(0)
	/*TypeUnion_t *pTypeUnion(pSelf->typeUnion());
	if (pTypeUnion)
	{
		for (TypeUnion_t::FieldsListCIt i(pTypeUnion->fields().begin()); i != pTypeUnion->fields().end(); i++)
		{
			Field_t &rField(*VALUE(i));
			if (rField.nameless())
				continue;
			PNameRef pName(rField.name());
			NamesMgr_t *pNs(pSelf->namesMgr());//this may be different namespace?
			pNs->insertName(pName, &rField);
		}
	}
	else */
	if (pSelf->typeStruc())
	{
		for (Struc_t::HierIterator i(pSelf); i; i++)
		{
			Field_t &rField(*i);
			if (!rField.nameless())
			{
				TypePtr iOwner2(i.NamespaceStruc());
				NamesMgr_t *pNs(iOwner2->typeStruc()->namesMgr());//this may be different namespace
				if (pNs != &rNS)
				{
					//pOwner2->recoverNamespace();
					//NamespaceInitialized(iOwner2);
				}
				PNameRef pName(rField.name());
//!				assert(0);
				if (!pNs->insertName(pName, &rField))//, iOwner2))
				{
					//?			assert(0);
				}
			}
		}

		TypesMgr_t *pTypeMgr(pSelf->typeMgr());
		if (pTypeMgr)
		{
			const TypesMap_t &m(pTypeMgr->aliases());
			for (TypesMapCIt i(m.begin()); i != m.end(); i++)
			{
				TypePtr pType(i.pvt()->pSelf);
				//assert(pType->isShared());//?
				if (!pType->nameless())
				{
					assert(0);
					rNS.insertName(pType->name(), pType, pSelf);
				}
				if (pType->typeComplex())
					//pType->typeComplex()->recoverNamespace();
					NamespaceInitialized(pType);
			}
		}

		/*if (pSelf->typeClass())
		{
			for (XFieldList_t::Iterator i(pSelf->typeClass()->methods()); i; i++)
			{
				Obj_t *pObj(i.data().pObj);
				assert(pObj);
			}
		}*/
	}
	else
	{
		//assert(0);
	}
	return rNS;
#endif
}
#endif


NamesMgr_t* ProjectInfo_s::OwnerNamesMgr(CTypePtr pSelf, TypePtr* ppOwner)
{
	if (pSelf->typeComplex())
	{
		NamesMgr_t* pNS(pSelf->typeComplex()->namesMgr());
		if (pNS)
		{
			if (ppOwner)// && pSelf->typeComplex())
				*ppOwner = (TypePtr)pSelf;
			return pNS;
		}
	}
	Seg_t* pSeg(pSelf->typeSeg());
	if (pSeg)
	{
		if (!pSeg->superLink())
		{
			if (pSelf->typeProject())
			{
				NamesMgr_t* pNS(AssureNamespace0((TypePtr)pSelf));
				if (ppOwner)
					*ppOwner = (TypePtr)pSelf;
				return pNS;
			}
			return nullptr;//seg range?
		}
		return OwnerNamesMgr(pSeg->superLink(), ppOwner);
	}
	FieldPtr pField(pSelf->parentField());
	if (pField)
	{
		//CHECKID(pField, 0x87b)
		//STOP
		return OwnerNamesMgr(pField, ppOwner);
	}
	return nullptr;
}


TypePtr ProjectInfo_s::IsBitvar(CTypePtr iSelf)
{
	Bitset_t *pBitset(iSelf->typeBitset());
	if (pBitset)
	{
		TypePtr iStrucvar(iSelf->parentField()->owner());//always
		if (iStrucvar->typeStrucvar())
			return iStrucvar;
	}
	return nullptr;
}

TypePtr ProjectInfo_s::ModuleOf(CTypeBasePtr pSelf0)
{
	CTypePtr pSelf((CTypePtr)pSelf0);
	assert(!pSelf->typeSimple() || pSelf->typeEnum());
	assert(!pSelf->typeArray());
	Seg_t *pSeg(pSelf->typeSeg());
	if (pSeg)
	{
		if (pSeg->superLink())
			return ModuleOf(pSeg->superLink());
		FieldPtr pField(pSelf->parentField());
		if (pField)
			return ModuleOf(pField->owner());
		if (!pSeg->typeModule())
		{
			if (!pSeg->typeProject())
			{
				assert(IsRangeSeg(pSelf));
				//just take field's type, which is seg and trace it up to the module
				FieldPtr pFirst(VALUE(pSeg->fields().begin()));
				assert(pFirst && pFirst->isTypeSeg());
				return ModuleOf(pFirst->type());
			}
			return nullptr;
		}
		return (TypePtr)pSelf;
	}
	if (pSelf->typeEnum())
		return ModuleOf(pSelf->baseType());
	assert(!pSelf->typeFuncDef());
	/*{
		FieldPtr pField(DockField(pSelf));
		if (pField)
			return ModuleOf(pField->owner());
	}*/
//CHECK(pSelf->typeBitset())
//STOP
	Field_t *pField(pSelf->parentField());
	if (pField)
		return ModuleOf(pField);
	if (pSelf->ownerTypeMgr())
		return ModuleOf(pSelf->ownerTypeMgr()->owner());
	//return mrProject.OnModuleRef(pSelf);
	return nullptr;
}

bool ProjectInfo_s::IsPhantomFolder(const Folder_t &rFolder)
{
	assert(&rFolder == TopFolder(rFolder));//only top-levels
	if (!rFolder.fileModule())
		return true;
	if (IsPhantomModule(rFolder.fileModule()->module()))
		return true;
	return false;
}

Folder_t *ProjectInfo_s::TopFolder(const Folder_t &rFolder)
{
	//return next folder right below the root
	const Folder_t *pr(nullptr);
	const Folder_t *p(&rFolder);
	while (p->Parent())
	{
		pr = p; p = p->Parent();
	}
	//assert(!pr || pr->m.miBinary);
	return (Folder_t *)pr;
}

bool ProjectInfo_s::IsTopFolder(const Folder_t &rFolder)
{
	return (&rFolder == TopFolder(rFolder));
}

TypePtr ProjectInfo_s::ModuleOf(CFolderPtr pFolder)
{
	FolderPtr pFolder0(TopFolder(*pFolder));
	return pFolder0->fileModule()->module();
}

unsigned ProjectInfo_s::FuncSizeLimited(TypePtr iFunc)
{
	FieldMap &m(iFunc->typeStruc()->fields());
	if (m.empty())
		return OPSZ_BYTE;
	FieldMapRIt i(m.rbegin());//TODO: duplicated in size()
	unsigned offs(KEY(i) - iFunc->base());
	unsigned fsz(offs);
	FieldPtr pField(VALUE(i));
	if (!IsEosField(pField))
	{
		TypePtr iType(pField->type());
		if (!iType || iType->typeCode())
			fsz += OPSZ_BYTE;
		else
		{
			unsigned sz((unsigned)Size(iType));
			if (sz == -1)
				sz = 0;
			if (sz == 0)
				fsz += OPSZ_BYTE;
			else
				fsz += sz;
		}
	}
	return fsz;
}


FieldMapIt ProjectInfo_s::EosAwarePrior(FieldMap &m, FieldMapIt i)
{
	assert(i == m.end() || !IsEosField(VALUE(i)));
	FieldMapIt j(m.begin());//begin is expensive
	if (i == j)
		return m.end();//end iterator is always cheap
	i = m.Prior(i);
	if (IsEosField(VALUE(i)))
	{
		if (i == j)
			return m.end();
		i = m.Prior(i);
	}
	return i;
}

FieldMapIt ProjectInfo_s::EosAwareNext(FieldMap &m, FieldMapIt i)
{
	assert(i != m.end() && !IsEosField(VALUE(i)));
	i++;
	if (i != m.end())
		if (IsEosField(VALUE(i)))
			i++;
	return i;
}

FieldMapCIt ProjectInfo_s::EosAwareNext(const FieldMap &m, FieldMapCIt i)
{
	assert(i != m.end() && !IsEosField(VALUE(i)));
	i++;
	if (i != m.end())
		if (IsEosField(VALUE(i)))
			i++;
	return i;
}

unsigned ProjectInfo_s::BitSize(TypePtr iType)
{
	if (!iType)
		return 1;
	Array_t *pArray(iType->typeArray());
	if (!pArray)
	{
		assert(iType->typeSimple());
		return 1;
	}
	return (unsigned)pArray->total();
}

NamesMgr_t *ProjectInfo_s::OwnerNamesMgr(CFieldPtr pSelf, TypePtr *ppOwner)
{
	TypePtr pCplx(pSelf->OwnerComplex());
	if (!pCplx)
		return nullptr;
	if (ppOwner)
		*ppOwner = pCplx;

	Complex_t &rSelf(*pCplx->typeComplex());
	if (rSelf.namesMgr())
		return rSelf.namesMgr();

	return OwnerNamesMgr(pCplx, ppOwner);
}

MyString ProjectInfo_s::fixFileName(MyString s, CTypePtr iModule)
{
	//ZPath_t z0("one/two");
	//ZPath_t z1("fck:\\one/two");
	//ZPath_t z2("fck:\\one/two/");
	//ZPath_t z3("fck:\\one/two\\three");
	//ZPath_t z4("fck:\\one/two\\three\\");
	//ZPath_t z5("\\one/two");
	//ZPath_t z6("/one/two");

	ZPath_t z(s);
	if (iModule)
		AssurePathAbsolute(z, iModule);

	if (z.isFile())
		z.removeExt();

	return z.toString();
}

void ProjectInfo_s::AssurePathAbsolute(ZPath_t& z, CTypePtr iModule)
{
	assert(iModule);
	if (!z.isAbsolute())
		z.push_front(ModuleTitle(iModule) + MODULE_SEP);
}


