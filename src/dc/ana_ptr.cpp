#include "ana_ptr.h"
#include "prefix.h"
#include "shared/defs.h"
#include "shared/link.h"
#include "shared/action.h"
#include "db/mem.h"
#include "db/obj.h"
#include "db/type_struc.h"
#include "db/field.h"
#include "db/proj.h"
#include "type_funcdef.h"
#include "info_dc.h"
#include "op.h"
#include "expr_ptr.h"
#include "expr_dump.h"
#include "path.h"

#define NEW_TRACEPTR 1
/*
Heap_t	ArrPtrs(20, sizeof(PTR_t));


Struc_t *Op_t::GetPtrStruc()
{
	assert(isPtr());
	if (tra ce_PTR)
		return 0;
	trace _PTR = 1;

	OpPtr pOpEntry = 0;
	if (!IsCodeOp())
		pOpEntry = this;
	else if (Is Addr())//mloc's offset
		pOpEntry = m_pData->m_pOp;
	else if (m_pData && !IsLo cal())//global ptr
		pOpEntry = this;

	Struc_t *pStruc = 0;
	if (pOpEntry)
		pStruc = pOpEntry->m_pStruc_;
	else
//	if (IsPrimeOp())
	{
		for (XRef_t *pXIn = GetA rgs(); pXIn; pXIn = pXIn->Ne xt())
		{
			OpPtr pOp = pXIn->pOp;
			if (pOp->isPtr())
				pStruc = pOp->GetPtrStruc();
			if (pStruc)
				break;
		}
	}

	trac e_PTR = 0;
	return pStruc;
}
*/

///////////////////////////////////

#if(0)
int PTR_t::AgreeBasePtr(TypePtr pStrucBase, FieldPtr pOpEntry)
{
//	m_pStrucBase = pStruc;
	if (!pStrucBase)
		return 0;

	if (!getBaseStruc())
		return 1;

		if (!m_pOpIndir)
		{
			if (m_nOffs != 0)
			{
				if (pStrucBase)
				{
					StrucModifier_t an;
					FieldPtr pField2 = an.AddF ield(pStrucBase, m_nOffs);
					pField2->ApplyStruct(getBaseStruc());
					if (m_nIndexSz != 0)
					{
						int nStrucSz = getBaseStruc()->size();
						if (m_nIndexSz == nStrucSz)
						{
							TypesTracer_t tt(*G DC.typeMgr());
							pField2->setT ype(tt.arrayOf(pField2->type(), 1) );//pField2->m_nArray = 1;
						}
					}
				}
			}
			else
			{
				if (getBaseStruc())
				{
					if (pStrucBase)
					{
					}
					else
					{
						//assert((uint8_t)pStrucBase & OPTYP _PTR);
						if (pOpEntry)
						{
							//!assert(pOpEntry->isPtr());
							if (pOpEntry->type()->typePtr())
							{
								pOpEntry->setT ype( nullptr );
								pOpEntry->SetStruc0(getBaseStruc());
							}
						}
					}
				}
				else
				{
				}
			}

			return 1;
		}

		if (!pStrucBase)
		{
			if (m_pOpIndir)
			{
//				assert(!m_pOpSrc->m_pStruc);
//				if (!bPtr && m_pOpSrc->m_pData)
//					m_pOpSrc->m_pData->ApplyStruct(m_pUp->m_pStrucBase);
//				else
//					m_pOpSrc->SetStruc0(m_pUp->m_pStruc);
			}
			else
			{
//				assert(!m_pFieldEntry->m_pStruc);
//				m_pUp->m_pStruc->AddFldRef(m_pFieldEntry);
			}
		}
		else
		{
			if (getBaseStruc() != pStrucBase)
			{
				if (getBaseStruc())
				{
					if (pStrucBase)
					{
						if (getBaseStruc()->typeStruc()->MergeWith(pStrucBase))
							return -1;
//							prime Op()->ReplaceBaseStruc(pStrucBase, getBaseStruc());
					}
					else
					{
						StrucModifier_t an;
						FieldPtr pField = an.AddFi eld(getBaseStruc(), 0, 2);
						//?uint8_t t = (uint8_t)pStrucBase;
assert(0);//?						AgreeTypes(pField->m_optyp, t);
					}
					pStrucBase = 0;//made invalid here
				}
				else if (pStrucBase)
				{
					StrucModifier_t an;
					FieldPtr pField = an.AddF ield(pStrucBase, 0, 2);
					//?uint8_t t = (uint8_t)getBaseStruc();
assert(0);//?					AgreeTypes(pField->m_optyp, t);
setBaseStruc(pStrucBase);
				}
				else
				{
				}
			}
		}

		if (m_nIndexSz != 0)
		{
			int offs = GetOffset();
			FieldPtr pField = getBaseStruc()->typeStruc()->GetFieldEx(offs, 1);
			if (pField)
			{
				assert(pField->Offset() == offs);
				TypesTracer_t tt(*G DC.typeMgr());
				pField->setT ype(tt.arrayOf( pField->type(), 1 ) );//m_nArray = 1;
			}
		}

//	if (!m_pStrucBase)
//		m_pStrucBase = pStruc;

	return 1;
}

/*?int PTR_t::CheckArray(PTR_t *p)
{
	if (GetSource() != p->GetSource())
		return 0;

	if (!m_pOpIndir)
		return 1;
	if (!mpField || !p->mpField)
		return 1;
	int ofs1 = GetOffset();
	int ofs2 = p->GetOffset();
	int d = ofs2 - ofs1;
	FieldPtr pField = 0;
	if (d < 0)
	{
		pField = p->mpField;
		d = -d;
		delete mpField;
	}
	else if (d > 0)
	{
		pField = mpField;
		delete p->mpField;
	}
	else
		return 1;
	if (d % m_pOpIndir->Size() != 0)
		return 1;
	int arr = d / m_pOpIndir->Size();
	pField->ApplyArray(arr+1);
	return -1;
}*/

bool PTR_t::IsFirst()
{
	if (m_pParent)
		if (m_pParent->m_pChilds != this)
			return false;
		return true;
}

void PTR_t::Unlink()
{
	PTR_t * pPrev = prev();
	if (pPrev)
		pPrev->m_pNext = m_pNext;
	else if (m_pParent)
		m_pParent->m_pChilds = m_pNext;
	
	m_pNext = 0;
	m_pParent = 0;
}

void PTR_t::InsertAfter(PTR_t * p)
{
	Unlink();
	m_pNext = p->m_pNext;
	p->m_pNext = this;
	m_pParent = p->m_pParent;
}

int PTR_t::__adjustTree()
{
	if (!IsFirst())
		if (!m_pParent->isRoot())
		{
			PTR_t * pParent = (PTR_t *)ArrPtrs.AddNew();
			pParent->Clear();
			m_pParent->CopyTo(*pParent);
			pParent->InsertAfter(m_pParent);
			pParent->AddChild(this);
			return 1;
		}
		
		for (PTR_t * p = m_pChilds; p; p = p->next())
		{
			if (p->__adjustTree())
				return 1;
		}
		
		return 0;
}

int PTR_t::AgreeBaseStrucs(PTR_t *p)
{
//	if (CheckArray(p) == -1)
//		return -1;
	if (AgreeBasePtr(p->getBaseStruc()) == -1)
		return -1;
	p->setBaseStruc(getBaseStruc());
	return 1;
}

/////////////////////////////
int PTR_t::__registerFields()
{
	for (PTR_t *p = m_pChilds; p; p = p->next())
	{
		if (p->__registerFields() == -1)
			return -1;

		SetSource(p->mpField);
	}

	if (isRoot())
		return 1;

	if (RegisterField() == -1)
		return -1;

	return (mpField == 0)?(0):(1);
}

/////////////////////////////
int PTR_t::RegisterFields0()
{
	assert(isRoot());

	if (__registerFields() == -1)
		return -1;

	PTR_t *pr = 0;
	for (PTR_t *p = m_pChilds; p; p = p->next())
	{
/*		if (!p->IsFirst())
		{
			int delta = p->Offset() - m_pChilds->GetOffset();
			if (m_pChilds->m_t.CombineWith(p->m_t, delta))
				return -1;
		}
*/
		if (1)
		if (getPtrLevel())
		if (!p->m_pOpIndir)
		{
			if (p->getBaseStruc())
			if (p->getBaseStruc() != getBaseStruc())
			{
				int ofs = p->GetOffset();
				if (ofs == 0)
				{
					if (getBaseStruc())
						if (p->getBaseStruc()->typeStruc()->MergeWith(getBaseStruc()))
						return -1;
				}
				else
				{
					//struc can't enacapsulate itself, so..
					if (getBaseStruc() != p->getBaseStruc())
					{
						if (!p->mpField)
							return 0;
						p->mpField->ApplyStruct(getBaseStruc());
					}
				}
			}
		}
				
		if (pr)
		if (p->m_pOpIndir == pr->m_pOpIndir)
		if (p->m_pOpIndir)
		{
			if (p->AgreeBaseStrucs(pr) == -1)
				return -1;
			//m_pStrucBase = 0;
		}
			
//		if (!pOpEntry)
//			continue;
						
//		SetSource(p->mpField);
						
//		if (!IsRoot())
//		if (RegisterField() == -1)
//			return -1;
							
		int nLevel;
		FieldPtr pOpEntry = GetOpEntry(nLevel);
		setPtrLevel(nLevel);
							
		if (pOpEntry)
		{
			if (!getBaseStruc())
				setBaseStruc(pOpEntry->type());
			else if (getBaseStruc() != pOpEntry->type())
			{
				if (AgreeBasePtr(pOpEntry->type(), pOpEntry) == -1)
					return -1;
			}
		}
							
		if (p->mpField)
			pr = p;
	}

	return 1;
}

PTR_t::PTR_t()
{
	Clear();
}

void PTR_t::Clear()
{
	mpField = 0;
	m_nOffs = 0;
	m_pOpIndir = 0;
	m_t.Clear();
//m_nPtrLevel = 0;
//m_pStrucBase = 0;
	m_pOpEntry_ = 0;
	m_pNext = 0;
	m_pChilds = m_pParent = 0;
	m_bSign = false;
	m_nIndexSz = 0;
}

void PTR_t::CopyTo(PTR_t &p)
{
	p.mpField = mpField;
	p.m_nOffs = m_nOffs;
	p.m_pOpIndir = m_pOpIndir;
	m_t.CopyTo(p.m_t);
//p.m_nPtrLevel = m_nPtrLevel;
//p.m_pStrucBase = m_pStrucBase;	//struc to apply
	p.m_pOpEntry_ = m_pOpEntry_;
	p.m_bSign = m_bSign;
	p.m_nIndexSz = m_nIndexSz;
}

void PTR_t::AddChild(PTR_t *pChild)
{
	pChild->Unlink();
	PTR_t *p = m_pChilds;
	if (!p)
		m_pChilds = pChild;
	else
	{
		while (p->m_pNext)
			p = p->m_pNext;
		p->m_pNext = pChild;
	}
	pChild->m_pParent = this;
}

PTR_t *PTR_t::next()
{
	return m_pNext;
}

PTR_t *PTR_t::prev()
{
	if (isRoot())
		return 0;
	
	PTR_t *p = m_pParent->m_pChilds;
	while (p != this)
	{
		if (p->m_pNext == this)
			return p;
		p = p->next();
	}

	return 0;
}

PTR_t *PTR_t::prim eOp()
{
	PTR_t *pPtr = this;
	while (pPtr->m_pParent)
		pPtr = pPtr->m_pParent;
	assert(!pPtr->m_pNext);
	return pPtr;
}

void PTR_t::ReplaceBaseStruc(TypePtr pStruc1, TypePtr pStruc2)
{
	if (m_pChilds)
		m_pChilds->ReplaceBaseStruc(pStruc1, pStruc2);
	if (m_pNext)
		m_pNext->ReplaceBaseStruc(pStruc1, pStruc2);
	if (getBaseStruc() == pStruc1)
		setBaseStruc(pStruc2);
}

void PTR_t::Invalidate()
{
	PTR_t *pr = prev();
	if (pr)
		pr->m_pNext = next();
	if (m_pParent->m_pChilds == this)
		m_pParent->m_pChilds = next();
}

void PTR_t::Append(PTR_t *pPtrInf)
{
	PTR_t *p = this;
	while (p->m_pNext)
		p = p->next();
	p->m_pNext = pPtrInf;
	pPtrInf->m_pParent = m_pParent;
}


int PTR_t::GetOffset()
{
	int nOffs = m_nOffs;
/*!	if (m_pOpIndir)
		nOffs += m_pOpIndir->i32;
	if (GetSource())
		if (GetSource()->IsA ddr())
//if (!GetSource()->IsL ocal())
//			nOffs += GetSource()->i32;
			nOffs += GetSource()->Offset0();*/
	return nOffs;
}

TYPEz_t PTR_t::GetType()
{
	TYPEz_t T;
	if (m_pOpIndir)
		T.m_optyp = m_pOpIndir->OpType();
	if (getBaseStruc())
	{
		T.mpStruc = getBaseStruc();
		T.mnPtr = m_t.mnPtr;
	}
	return T;
}

void PTR_t::GetPtrInfo(TYPEz_t &T)
{
	T.Clear();
	if (getPtrLevel() != 0)
		T.SetPtrLevel(getPtrLevel());
	else
		T.m_optyp = 0;
	T.mpStruc = getBaseStruc();
	T.mnPtr = m_t.mnPtr;
}

/*
int PTR_t::GetPtrLevel()
{
	bool bInc = true;
	int nLevel = 0;
	PTR_t *p = this;
	while (1)
	{
//		if (p->m_pOpSrc || p->m_bThis)
		if (bInc)
			nLevel++;
		else
			bInc = true;

		PTR_t *pD = p->m_pDown;
		if (!pD)
			break;	

		if (p->m_pOpSrc == pD->m_pOpSrc)
			bInc = false;
		if (p->m_pOpEntry != pD->m_pOpEntry)
			break;

		p = pD;
	}
	return nLevel;
}*/

int PTR_t::GetPtrLevel()
{
	int nLevel = 0;
	PTR_t *p = this;
	do {
		nLevel++;
		if (p->GetSource())
		{
			if (m_pParent && m_pParent->getPtrLevel() > nLevel)
				nLevel = m_pParent->getPtrLevel();
			break;
		}
		p = p->m_pChilds;
	} while (p);
	return nLevel;
}

FieldPtr PTR_t::GetOpEntry(int &nLevel)
{
	nLevel = 0;
	PTR_t *p = this;
	do {
if (!p->isRoot())
		nLevel++;
		if (p->GetSource())
//		if (!p->m_pOpEntry->IsCode() || (p->m_pOpEntry->IsIndirect() && ))
		{
			if (m_pParent && m_pParent->getPtrLevel() > nLevel)
				nLevel = m_pParent->getPtrLevel();
			return p->GetSource();
		}
		p = p->m_pChilds;
	} while (p);
	return 0;
}

int PTR_t::RegisterField2()
{/*!
	int nLevel;//level of indirection passed to reach entry op
	FieldPtr  pOpEntry = GetOpEntry(nLevel);
	if (!pOpEntry)
		return 0;

	int nOffs = GetOffset();
	if (nOffs < 0)
		return 0;

	if (nOffs == 0)
	{
		if (pOpEntry->Is Addr())
		{
			if (!m_pOpIndir)
				return 0;
			FieldPtr  pData = (FieldPtr )pOpEntry->m_pM Loc->AssureObjType(OBJ_ DATA);
			//pData->AssureOp(m_pOpIndir->OpType());
			pData->SetOpType(m_pOpIndir->OpType());
			return 1;
		}
		pOpEntry->SetPtr(nLevel);
		if (m_pOpIndir)
			pOpEntry->m_nPtr = m_pOpIndir->OpType();
		return 1;
	}

	if (!IS_ PTR(pOpEntry->m_pStruc))
	{
		Struc_t * pStruc = G DC.Make Struct(m_pOpIndir);
		pOpEntry->SetPtr(nLevel);
		pOpEntry->SetStruc0(pStruc);
	}

	int n = (!m_pOpIndir)?(0):(2);
	mpField = pOpEntry->m_pStruc->typeStruc()->AddF ield(nOffs, n, 0);
	if (!mpField)
		return 1;

	if (m_pOpIndir)
	{
		uint8_t t = m_pOpIndir->OpType();
		if (!AgreeTypes(mpField->m_optyp, t))
			mpField->OvercastType(m_pOpIndir->OpType());
	}
*/
	return 1;
}

int PTR_t::RegisterField()
{/*!
	mpField = 0;

//	assert(m_pOpSrc || m_bThis);//indirection
	int nLevel;//level of indirection passed to reach entry op
	FieldPtr pOpEntry = GetOpEntry(nLevel);
//assert(pOpEntry);
	if (!pOpEntry)
		return 0;

//	if (!(pOpEntry->optyp & OPTYP _PTR))
//		pOpEntry->SetOpType(OPTYP _PTR|G DC.PtrSize());

	setBaseStruc(0);//Struc_t *pStrucBase = 0;
	FieldPtr pOpDest = pOpEntry;

	if (pOpDest->IsCode())
	{
		if (pOpDest->IsA ddr())
		{
			if (!pOpDest->CheckLocal0())//AttachToData())
				return 0;

			TYPEz_t T;
			if (m_pParent->getPtrLevel())
			{
				T.Setup(0, m_pParent->getBaseStruc());
				if (m_pParent->getPtrLevel() > 1)
					T.SetPtrLevel(m_pParent->getPtrLevel() - 1);
			}
			if (T.IsNull())
			{
				uint8_t t = (m_pOpIndir)?(m_pOpIndir->OpType()):(0);
				T.Setup(t, (Struc_t *)0);//m_pUp->m_pStrucBase);
			}
			mpField = (FieldPtr )pOpDest->m_p MLoc->RegisterFetch(GetOffset(), T);
			if (mpField)
			{
				setBaseStruc(mpField->m_pStruc);
				return 1;
			}
			else 
			{
				if (pOpDest->fieldRef()->IsD ata())
				if (pOpDest->fieldRef())
					setBaseStruc(pOpDest->fieldRef()->m_pStruc);
				return 0;
			}

		}
		else 
		{
//			if (pOpDest->i32)//!!!
//				return 0;
			if (!pOpDest->fieldRef())
			{
				if (!m_pOpIndir)
					return 0;
				if (!pOpDest->CheckLocal0())//Ex())
					return 0;
			}
			
			if (pOpDest->i32)
				return 0;

			assert(!pOpDest->i32);//!!!
			if (!pOpDest->IsL ocal() || !IsArglOp(pOpDest))
				//?pOpDest = pOpDest->fieldRef()->AssureOp(OPTYP _PTR|GD C.PtrSize());
				pOpDest->fieldRef()->SetOpType(OPTYP _PTR|G DC.PtrSize());
					pOpDest = pOpDest->fieldRef();//?
		}
	}

	Struc_t *pStrucThis = 0;
	if (m_pParent && IS _PTR(m_pParent->getBaseStruc()))
		pStrucThis = m_pParent->getBaseStruc()->typeStruc();

	if (!pOpDest->isPtr())
		pOpDest->SetOpType(OPTYP _PTR | G DC.PtrSize());

	int nLevelD = pOpDest->PtrLevel();//level of indirection of destination op
	assert(nLevel <= 0x8);
	
	if (nLevel == nLevelD)
		setBaseStruc(pOpDest->m_pStruc);

	int nOffs = GetOffset();
	if (nOffs < 0)
		return 0;

	if (nOffs == 0)// && !m_bThis
	{
		if (nLevel > nLevelD)
		{
			pOpDest->SetPtr(nLevel);
		}
		else if (nLevel < nLevelD)
		{
			if (pStrucThis)
			{
			Struc_t *pStruc = pOpDest->m_pStruc->typeStruc();
			if (IS _PTR(pStruc))
				pStruc->ReleaseOpRef(pOpDest);
			pOpDest->m_nPtr = 0;
			setBaseStruc(G DC.Make Struct(m_pOpIndir));
				pOpDest->SetPtr(nLevel);
			FieldPtr pOp0 = getBaseStruc()->typeStruc()->AddF ield(0, 2);

			int n = nLevelD-nLevel;
			pOp0->SetPtr(n);
			if (IS _PTR(pStruc))
				pOp0->SetStruc0(pStruc);
			}
		}

		if (pStrucThis)
		if (!getBaseStruc())
		{
			if (!m_pOpIndir)
			{
				setBaseStruc(pStrucThis);
				if (I S_PTR(getBaseStruc()))
					pOpDest->SetStruc0(getBaseStruc());
			}
			else if (m_pParent->getPtrLevel() > 0)
			{
				nLevelD++;
				pOpDest->SetPtr(nLevelD);
			}
		}
	}
	else 
	{
		if (!I S_PTR(getBaseStruc()))//l > 1)
		{
//			if (nOffs >= 0)
			if (nLevel < nLevelD)
			{
				Struc_t *pStruc = pOpDest->m_pStruc->typeStruc();
				if (IS _PTR(pStruc))
					pStruc->ReleaseOpRef(pOpDest);
				pOpDest->m_nPtr = 0;
				setBaseStruc(G DC.MakeStr uct(m_pOpIndir));
					pOpDest->SetPtr(nLevel);
				FieldPtr pOp0 = getBaseStruc()->typeStruc()->AddF ield(0, 2);
				int n = nLevelD-nLevel;
				pOp0->SetPtr(n);
				if (IS _PTR(pStruc))
					pOp0->SetStruc0(pStruc);
			}
			else if (nLevel == nLevelD)
			{
				uint8_t t = (uint8_t)pOpDest->m_nPtr;
				assert(!IS _PTR(pOpDest->m_pStruc));
				pOpDest->m_nPtr = 0;
				if (m_pOpIndir)// || m_bThis
				{
					setBaseStruc(GD C.MakeSt ruct(m_pOpIndir));
					if (t) 
					{
						OpPtr pOp0 = getBaseStruc()->typeStruc()->AddF ield(0, 2);
						pOp0->SetOpType(t);
					}
				}
			}
			else if (nLevel > nLevelD)
			{
//!!!			assert(!IS _PTR(pOpDest->m_pStruc));
				if (IS _PTR(pOpDest->m_pStruc))
					return 0;

				uint8_t t = (uint8_t)pOpDest->m_nPtr;
				pOpDest->m_nPtr = 0;
				setBaseStruc(G DC.MakeStr uct(m_pOpIndir));
			
				assert(nLevel - nLevelD == 1);
				nLevelD++;
				pOpDest->SetPtr(nLevelD);
			}
		}

		if (m_pOpIndir)
		if (!getBaseStruc())
			setBaseStruc(G DC.MakeSt ruct(m_pOpIndir));
	}

	if (!IS _PTR(getBaseStruc()))
	{
		if (m_pOpIndir)
		{
			if (!pOpDest->m_nPtr)
				pOpDest->m_nPtr = m_pOpIndir->OpType();
		}
		return 0;//m_pOpEntry;
	}

	if (pOpDest)
	{
		if (0)
		if (pOpDest->IsCode())//IsCallOutOp()
		{
			if (!pOpDest->m_p MLoc)
				//pOpDest->AssureLocalEx();
				if (!pOpDest->CheckLocal0())//Ex())
					return 0;
			pOpDest = pOpDest->fieldRef();
		}
		if (pOpDest->m_pStruc != getBaseStruc())
			pOpDest->SetStruc0(getBaseStruc());
	}
*/
//	if (nOffs < 0)
//	{
//		{
//			if (pOpDest->disp > nOffs)
//			{
//				pOpDest->m_pStruc->ShiftFields(pOpDest->disp - nOffs);
//				pOpDest->disp = nOffs;
//			}
//			nOffs -= pOpDest->disp;
//		}
//	}

/*	if (0)
	if (nOffs == 0)
	if (m_pUp && m_pUp->m_nPtrLevel && !m_pOpIndir)
	{
		if (IS _PTR(m_pUp->m_pStrucBase))
		if (m_pStrucBase != m_pUp->m_pStrucBase)
		{
			assert(!m_pOpIndir);
			if (m_pUp->m_pStrucBase->MergeWith(m_pStrucBase))
				return -1;
		}

		return 1;
	}
*/	
/*!	if ((m_pParent && m_pParent->getPtrLevel()) || m_pOpIndir)
	if (m_pOpIndir || nOffs != 0)
	{
		int n = (!m_pOpIndir)?(0):(2);
		mpField = getBaseStruc()->typeStruc()->AddF ield(nOffs, n, 0);//m_bThis);
		if (mpField)
		{
//			if (!m_pOpIndir)
//			{
//				if (m_pUp && m_pUp->m_pStrucBase)
//				{
//					//struc can't enacapsulate itself, so..
//					if (m_pUp->m_pStrucBase == m_pStrucBase)
//						return 0;
//					mpField->ApplyStruct(m_pUp->m_pStrucBase);
//				}
//			}
//			else 
			if (m_pOpIndir)
			{
				if (mpField->IsComplex0())
					mpField = getBaseStruc()->typeStruc()->GetFieldEx(nOffs, 1);

				if (mpField)
				{
					assert(0);
					uint8_t t = m_pOpIndir->OpType();
					if (!AgreeTypes(mpField->m_optyp, t))
						mpField->OvercastType(m_pOpIndir->OpType());
					if (((FieldPtr)m_pOpIndir)->m_pStruc && m_pOpIndir->isPtr())//?
						if (!mpField->m_pStruc)
							mpField->SetStruc0(((FieldPtr)m_pOpIndir)->m_pStruc);//?
				}
			}

			if (m_nIndexSz != 0)
			{
	//			int nStrucSz = m_pStrucSrc->Size();
	//			if (m_nIndexSz == nStrucSz)
					mpField->m_nArray = 1;
			}
		}
	}

	if (mpField)
	if (IS _PTR(mpField->m_pStruc))
		assert(mpField->isPtr() || mpField->m_pStruc->ObjT ype() == OBJID_TYPE_STRUC);
*/
	return 1;
}

FieldPtr PTR_t::GetBaseField()
{
//	Struc_t *pStrucBase = 0;
	assert(!getBaseStruc());
//	assert(m_pOpEntry);

	FieldPtr pOpDest = GetSource();
	if (!pOpDest)
		return 0;
/*!
	if (pOpDest->m_p MLoc)
	{
		if (!pOpDest->m_pM Loc->IsD ata())
			return 0;

		if (!pOpDest->m_pStruc)
		{
			FieldPtr pField = pOpDest->fieldRef();
			if (!pField)
				return 0;

			if (pOpDest->IsAd dr())
			{
				if (!pField->IsComplex0())
					return pField;
			}
			else
			{
				m_nOffs -= pField->m_disp;
			}

			pOpDest = pField;
		}
	}

	setPtrLevel(0);
	if (pOpDest->isPtr())
		setPtrLevel(pOpDest->PtrLevel());
	setBaseStruc(pOpDest->m_pStruc);*/
	return pOpDest;
}

FieldPtr PTR_t::SetupBaseStruc(TYPEz_t &Type)
{
	FieldPtr pField = nullptr;
/*!	if (!m_pChilds)
	{
		FieldPtr pField = GetBaseField();
		return pField;
	}

	for (PTR_t *p = m_pChilds; p; p = p->N ext())
	{
		//Type_t T;
//		OpPtr pField = p->GetBaseStruc(Type);
//		if (!pField)
//			continue;

		FieldPtr pOp = p->SetupBaseStruc(Type);
		Struc_t *pStruc = p->getBaseStruc()->typeStruc();//pField->m_pStruc;
		if (!IS _PTR(pStruc))
		{
			if (pOp)
			if (IS _PTR(pOp->m_pStruc) && pOp->isPtr())
				setBaseStruc(pOp->m_pStruc);
			continue;
		}

		if (p->getPtrLevel() > 1)
		{
			setBaseStruc(p->getBaseStruc());
			setPtrLevel(p->getPtrLevel() - 1);
		}
		else
		{
			//assert(p->m_nPtr == 1);
			int nOffs = p->GetOffset();
			pField = pStruc->GetFieldEx(nOffs);
			if (!pField)
				continue;

			Type.Clear();
			Type.m_optyp = pField->OpType();

			if (IS _PTR(pField->m_pStruc))
			{
				Type.m_pStruc = pField->m_pStruc;
				uint8_t ptrmask;
				uint8_t ptrlevel = Type.PtrLevel(&ptrmask);
				Type.SetPtrLevel(MAKEPTR(ptrlevel+1, ptrmask));

				if (!getBaseStruc())
					setBaseStruc(pField->m_pStruc);
			}
		}

		if (getBaseStruc())
			break;
	}
	
	if (!pField)
		return 0;
	if (pField->isPtr())
		setPtrLevel(pField->PtrLevel());*/
	return pField;
}

#endif

/*int Op_t::GetBasePtrStruc(TypePtr *ppStruc, int *pnPtr)
{
	if (!IsPrimeOp())
	if (!XIn())
	{
		//if (0)
		if (m_ pMLoc)
		if (m_p MLoc->IsD ata())
		if (!IsL ocal() || !XIn())//global ptr
		{
			assert(m_disp == 0);

			FieldPtr pField = fieldRef()->m_pOp_;
			assert(pField);
			if (pField->isPtr())
			{
				if (IS _PTR(pField->m_pStruc))
				{
					*ppStruc = pField->m_pStruc->typeStruc();
					*pnPtr = pField->PtrLevel();
					return 1;
				}
			}
		}

	
		*ppStruc = nullptr;//?m_pStruc;
		*pnPtr = 0;
		if (isPtr())
			*pnPtr = PtrLevel();
		return 1;
	}

	ArrPtrs.Reset();
	PTR_t *pPtrStart = (PTR_t *)ArrPtrs.AddNew();
	pPtrStart->Clear();
		PTR_t *pPtr = (PTR_t *)ArrPtrs.AddNew();
		pPtr->Clear();
		pPtrStart->AddChild(pPtr);
	if (!__tracePtr(pPtr))
		return 0;

	TYPEz_t T;
	pPtr->SetupBaseStruc(T);

	pPtr->GetPtrInfo(T);

	Struc_t *pStruc = T.m_pStruc->typeStruc();
	if (IS _PTR(pStruc))
	{
		int nOffset = pPtr->GetOffset();
		while (nOffset != 0)
		{
			FieldPtr pField = pStruc->GetFieldEx(nOffset);
			if (!pField)
				return 0;
			if (!((FieldPtr )this)->IsComplex0())//???
				return 0;
			pStruc = pField->m_pStruc->typeStruc();
			nOffset -= pField->Offset();
		}
	}

	*ppStruc = pStruc;

	uint8_t ptrmask;
	*pnPtr = T.PtrLevel(&ptrmask);

	return 1;
}*/



int AnlzTracePtr::TraceIndexStep(HOP pSelf) const
{
	assert(IsPrimeOp(pSelf));

	int nStep = 0;
	if (ActionOf(pSelf) == ACTN_MUL)
		if (pSelf->arg2()->IsScalar())
	{
		nStep = OpDisp(pSelf->arg2());
		if (pSelf->arg1()->m_xin.check_count(1) == 0)
		{
			OpPtr pOp = pSelf->arg1()->XIn()->data();
			if (IsPrimeOp(pOp))
			{
				int nStep2 = 0;
				nStep2 = TraceIndexStep(pOp);
				if (nStep2 != 0)
					nStep *= nStep2;
			}
		}

		return nStep;
	}

	return 0;
}

#if(0)
int AnlzTracePtr::__traceBase(PTR_t *pPtrInf) const
{
	assert(IsIndirectB());

	if (pPtrInf->GetSource())
		if (pPtrInf->GetSource() != this)
			return 0;

	int nResult = 1;
	int nOk = 0;
	PTR_t *p = 0;

	for (XRef_t *pXDep = XIn();
	pXDep; 
	pXDep = pXDep-> N ext())
	{
		if (!p)
		{
			p = (PTR_t *)ArrPtrs.AddNew();
			p->Clear();
			if (pPtrInf)
				pPtrInf->AddChild(p);
		}
		else
		{
			p->Clear();
			p->m_pParent = pPtrInf;
		}
		p->m_pOpIndir = this;

		OpPtr pOp = pXDep->pOp;

		if (opTracer().cell(pSelf).trace_ptr > 1)
		if (opTracer().cell(pOp).trace_ptr > 0)
			continue;

		nResult = pOp->__tracePtr(p);
		if (nResult != 1)
			continue;
		p = 0;
		nOk++;
	}

	if (p)
		p->Invalidate();

	if (nOk == 0)
		return 0;
	return 1;
}

#define RETURN(arg) {assert(tr.trace_ptr > 0); tr.trace_ptr--; return arg;}
int AnlzTracePtr::__tracePtr(PTR_t *pPtrInf) const
{
	int nResult = 1;
	op_tracer_cell_t &tr(opTracer().cell(mpOp));
	assert(tr.trace_ptr < TRACE_PTREX);
	tr.trace_ptr++;

	if (mpOp->OpSize() != GD C.PtrSize())
		RETURN(0);

	if (!mpOp->IsCodeOp())
	{
		pPtrInf->SetSource((FieldPtr)mpOp);//?
		RETURN(1);
	}

	if (mpOp->IsIndirectB())
	{
		pPtrInf->SetSource((FieldPtr)mpOp);//?
		nResult = __traceBase(pPtrInf);
		RETURN(nResult);
	}
	
	if (mpOp->Is Addr())//mloc's offset
	{
		pPtrInf->SetSource((FieldPtr)mpOp);//?
		RETURN(1);
	}

	if (mpOp->IsScalar())
	{
		for (PTR_t *p = pPtrInf; p; p = p->m_pNext)
			p->m_nOffs += (p->m_bSign)?(-OpDisp(mpOp)):(OpDisp(mpOp));
	//	if (i32 != 0)
	//		RETURN(3);//FIXME:not ptr?
		RETURN(2);
	}

	if (IsRhsOp(mpOp) && !mpOp->XIn())
	{
		pPtrInf->SetSource((FieldPtr)mpOp);//?
		RETURN(1);
	}
	
/*	if (m_pData)
	if (!IsL ocal() && !m_pData->IsLo calEx())// || !XIn())//global ptr
	{
//		assert(disp == 0);
//		assert(m_pData->m_pOp);
//		pPtrInf->m_pOpEntry = m_pData->m_pOp;
		pPtrInf->SetSource(this);
		RETURN(1);
	}*/

	if (IsPrimeOp(mpOp) && !mpOp->IsCall())
	{
		OpPtr pOp0, *pOp1;
		bool bSign = false, bSignSv;
//			int nOffsSv = pPtrInf->m_nOffs;

		switch (mpOp->mAction()) {
		case ACTN_MOV:
			{
			pOp0 = &mpOp->arg1();
			AnlzTracePtr a(mrFunc, pOp0);
			if ((nResult = a.__tracePtr(pPtrInf)) != 1)
				RETURN(nResult);
			}
			break;

		case ACTN_SUB:
			bSign = true;

		case ACTN_ADD:
			{
			pOp0 = &mpOp->arg1();
			pOp1 = &mpOp->arg2();
	
			AnlzTracePtr a(mrFunc, pOp0);
			int res0 = a.__tracePtr(pPtrInf);

			int nOffsSv = pPtrInf->m_nOffs;
			bSignSv = pPtrInf->m_bSign;
			pPtrInf->m_bSign = bSign;
			AnlzTracePtr b(mrFunc, pOp1);
			int res1 = b.__tracePtr(pPtrInf);
			pPtrInf->m_bSign = bSignSv;
//				if (opTracer().cell(pSelf).trace_ptr > 1)
//					pPtrInf->m_nOffs = nOffsSv;
			if (res0 == 1 && res1 != 1)//2nd op failed, 1st - did not
				nResult = 1;
			else if (res0 != 1 && res1 == 1)
				nResult = 1;
			if (0){
			int offs = pPtrInf->GetOffset();
			if (offs < 0)
			{
				pPtrInf->m_pChilds = 0;
				pPtrInf->m_nOffs = nOffsSv;
				pPtrInf->SetSource((FieldPtr)mpOp);//?
			}
			}
			nResult = 1;
			break;
			}

		default:
			if (!pPtrInf->m_nIndexSz)
				pPtrInf->m_nIndexSz = TraceIndexStep(mpOp);
			//assert(false);
			RETURN(0);
		}//switch
	}
	else
	{
		int nOffsSv = pPtrInf->m_nOffs;
		PTR_t *p = pPtrInf;
		int nOk = 0;
		for (XOpList_t::Iterator i(mpOp->m_xin); i; i++)
		{
			OpPtr pOp = i.data();
			if (tr.trace_ptr > 1)
			if (opTracer().cell(pOp).trace_ptr > 0)
				continue;

			if (!p)
			{
				p = (PTR_t *)ArrPtrs.AddNew();
				p->Clear();
				pPtrInf->CopyTo(*p);
				pPtrInf->Append(p);
			}
			
			p->m_nOffs = nOffsSv;
			AnlzTracePtr a(mrFunc, pOp);
			if ((nResult = a.__tracePtr(p)) == 1)
			{
				nOk++;
				p = 0;
			}
		}

		if (p && p != pPtrInf)
			p->Invalidate();

		if (nOk == 0)
			RETURN(0);

		nResult = 1;
	}

	uint8_t t1 = mpOp->m_optyp;
	uint8_t t2 = OPTYP _PTR|GD C.PtrSize();
	if (!AgreeTypes(t1, t2))
		RETURN(0);

/*	if (0)
	if (IsCodeOp())
	if (!IsPrimeOp())
	if (m_pRoot)
	{
		for (XRef_t *pXIn = m_pRoot->Get Args(); pXIn; pXIn = pXIn->Ne xt())	
		{
			OpPtr pOp = pXIn->pOp;
			if (pOp != this)
			{
				uint8_t _optyp = G DC.PtrSize();
*//*				if (AgreeTypes(pOp->optyp, _optyp))
					pOp->TraceType();
			}
		}
	}*/

	RETURN(nResult);
}
#undef RETURN
#endif


int AnlzTracePtr::TracePtrZ(FieldPtr pEntryOp) const//, bool bPtr)
{/*!
	ArrPtrs.Reset();
	PTR_t *pPtrStart = (PTR_t *)ArrPtrs.AddNew();
	pPtrStart->Clear();
		PTR_t *pPtr = (PTR_t *)ArrPtrs.AddNew();
		pPtr->Clear();
		pPtrStart->AddChild(pPtr);
	__tracePtr(pPtr);
	while (pPtrStart->__adjustTree());

#ifdef _DEBUG
	__Dptr.Log(pPtrStart, pri meOp()->No());
#endif

	TYPEz_t Te;
	pEntryOp->GetType(Te);

	if (Te.isPtr())
	{	
		pPtrStart->setBaseStruc(Te.m_pStruc);
		pPtrStart->setPtrLevel(Te.PtrLevel());
	}
	else
		pPtrStart->setPtrLevel(1);

	pPtrStart->StorePtrs();
	if (pPtrStart->RegisterFields0() == -1)
		return -1;

	if (pPtrStart->getPtrLevel())
	if (!Te.isPtr())
	{
		if (!pEntryOp->SetPtr(pPtrStart->getPtrLevel()))
			return 0;
	}

	if (pPtr->mpField)
	{
		TYPEz_t Tf;
		pPtr->mpField->GetType(Tf);
		Tf.SetPtrTo(Tf);
		if (Tf.AreCompliant(Te))
		{
			Tf.Comply With(Te);
			pEntryOp->SetType(Te);
			return 1;
		}
	}

	if (IS _PTR(pPtr->getBaseStruc()))
	if (pPtr->GetOffset() == 0)// || bPtr
	if (!pEntryOp->m_pStruc)
	{
		pEntryOp->SetStruc0(pPtr->getBaseStruc());
		if (!pEntryOp->isPtr())
			pEntryOp->SetPtr(1);
	}*/
	return 1;
}

/*void PTR_t::StorePtrs()
{
	if (GetSource())
		G DC.arrPtrs.Add(GetSource());

	for (PTR_t *p = m_pChilds; p; p = p->N ext())
		p->StorePtrs();
}*/

int AnlzTracePtr::TracePtr1() const
{assert(0);/*?
	assert(IsIndirectB());

	ArrPtrs.Reset();
	PTR_t *pPtrStart = (PTR_t *)ArrPtrs.AddNew();
	pPtrStart->Clear();

	PTR_t *pPtr = pPtrStart;
	if (0)
	{
		pPtr = (PTR_t *)ArrPtrs.AddNew();
		pPtr->Clear();
		pPtrStart->AddChild(pPtr);
	}

	if (!__traceBase(pPtr))
		return 0;

	while (pPtrStart->__adjustTree());


#ifdef _DEBUG
	__Dptr.Log(pPtrStart, prim eOp()->No());
#endif

//	if (prim eOp()->No() < 18)
//	if (pri meOp()->No() != 50)
//if (No() != 140 && No() != 143 && No() != 146)
	if (isPtr())
	{
		pPtrStart->setBaseStruc(nullptr);//?m_pStruc);
		pPtrStart->setPtrLevel(PtrLevel());
	}

//if (pri meOp()->No() < 92)
//	__Ddep.Log(this);

	pPtrStart->StorePtrs();
 	return pPtrStart->RegisterFields0();*/return 0;
}

void AnlzTracePtr::TracePtrArgs() const
{
//CHECK(No() == 6)
//STOP

	if (!IsCall(mpOp))
		return;

//	if (No() != 205)// && No() != 168)// && No() != 153)
	if (!IsAddr(mpOp))
	{
//?		assert(0);
//?		while (TracePtrZ(this/*, true*/) == -1);
	}

	TraceArgPtrs();
}

/*int Op_t::PtrLevel()
{
	assert(m_optyp & OPTYP _PTR);
	return ((m_optyp&0x70)>>4)+1; 
}

int Op_t::SetPtr(int nLevel)
{
	assert(nLevel > 0 && nLevel <= 8);
	int ptrsz = GD C.PtrSize();
	uint8_t sz = OpSize();
	if (sz && sz != ptrsz) 
		return 0;
	m_optyp = OPTYP _PTR | ptrsz;
	m_optyp &= ~0x70;
	m_optyp |= (nLevel - 1) << 4;
	return 1;
}*/

/*int FuncInfo_t::RegisterMemoryFetch(OpPtr pSelf) const
{
	if (!pSelf->IsCodeOp())
		return 0;
	FieldPtr pFieldRef(findFieldRef(pSelf));
	if (!pFieldRef)
		return 0;
	if (pSelf->Is Addr())
		return 0;

//CHECK(m_disp == 0x0100109c)
//STOP

	/ *TYPEz_t T;
	T.Setup(pSelf->m_optyp, 0, 0);//?m_pStruc);
	pFieldRef->RegisterFetch(pSelf->m_disp, T);* /
	pFieldRef->RegisterFetch(pSelf->m_disp, pSelf->m_optyp);
	return 1;
}*/

void AnlzTracePtr::TracePtrBase(AnalyzePtrs_t &aTop)
{
#if(0)
	if (mpOp->No() == 15)
		return;
#endif
CHECK(OpNo(PrimeOp(mpOp)) == 129)
STOP

	if (0)
	if (mpOp->IsIndirect() && !LocalRef(mpOp))
	{
		ptrinfo_t p(*this);
		int res = p.TracePtrSource(mpOp);
		STOP;
	}

//	if (mrFunc.CompName("_fn_5105B0")) 
//		if (No() > 83)
//			return;

//	if (!isPtr())
//		TraceType();

	if (IsAddr(mpOp))
	{
		aTop.addOp(mpOp);
	}
	else if (mpOp->IsIndirect())
	{
		if (mpOp->IsIndirectB() || mpOp->SSID() == SSID_GLOBAL)
		{

#if(NEW_TRACEPTR)
			ExprCacheEx_t aCache(PtrSize());
			EXPRptr_t expr(*this, mpOp, OpNo(PrimeOp(mpOp)), aCache);
			Out_t *pOut(expr.TracePtr(mpOp, pathOpTracer()));

			expr.Simplify(pOut, mpOp, pathOpTracer());
#else
			while (TracePtr1() == -1);
#endif
		}
		/*else
		{
			RegisterMemoryFetch(mpOp);
		}*/
	}

	if (!IsPrimeOp(mpOp))
		return;

	for (OpList_t::Iterator i(mpOp->argsIt()); i; i++)
	{
		AnlzTracePtr an(*this, i.data());
		//an.setOpTracer(&opTracer());
		an.TracePtrBase(aTop);
	}
}

void EXPRptr_t::Simplify(Out_t* pOut, HOP hOp, PathOpTracer_t &tr)
{
#if(TRACK_PTRS2VIEW || 0)
	MyStream ss(1024);
	ss.SetEndOfLineChar('\n');
	int line(OpNo(PrimeOp(hOp)));
	TExprDump2view<EXPRPtrSimpl_t> ES(*this, hOp, 0, static_cast<ExprCacheEx_t &>(exprCache()), line, ss);
	ES.SimplifyPtr0(pOut, tr);
	ss.WriteChar(0);//eos!
	mrDC.NewPtrDump(DockAddr(), line, ss);
#else
	EXPRPtrSimpl_t ES(*this);
	ES.SimplifyPtr0(pOut, tr);
#endif
}

#define RETURN(arg) { tr.trace &= ~TRACE_FORWARD; return;}
void AnlzTracePtr::__tracePtrForward() const
{
	uint8_t t1 = mpOp->m_optyp;
	uint8_t t2 = MAKETYP_PTR(mrDC.PtrSize());
	if (!AgreeTypes(t1, t2))
		return;

	op_tracer_cell_t &tr(opTracer().cell(mpOp));
	if (tr.trace & TRACE_FORWARD)
		return;
	tr.trace |= TRACE_FORWARD;

	if (IsPrimeOp(mpOp))
	if (ActionOf(mpOp) != ACTN_MOV)
	if (ActionOf(mpOp) != ACTN_ADD)
	if ((ActionOf(mpOp) != ACTN_SUB) || mpOp->isCPUSW())
		RETURN(0);

	if (IsRhsOp(mpOp))
	{
		OpPtr pOpRoot(PrimeOp(mpOp));

		if (IsCall(pOpRoot))
		{
			FieldPtr pField(nullptr);
			if (IsCallArg(mpOp))
			{
				pField = FindCalleeArg(mpOp);
				if (!pField)
					RETURN(0);//vararg?
			}
			else if (!mpOp->IsIndirect() || IsLocalOp(mpOp))
				RETURN(0)

			assert(pField);
			while (TracePtrZ(pField/*, false*/) == -1);
			RETURN(0);
		}
		else if (IsRetOp(pOpRoot))
		{
			while (TracePtrZ(0) == -1);
			RETURN(0);
		}

//		if (pOp->isPtr())
//			continue;//already
		if (pOpRoot->IsIndirectB())
//			if (!pOpRoot->m_pData)
				RETURN(0);
//		if (IsPrimeOp(pOp))
//			if (pOp->Action() != ACTN_MOV)
//				if (pOp->Action() != ACTN_ADD)
//					if ((pOp->Action() != ACTN_SUB) || (pOp->OpC() == OPC_ CPUSW))
//						RETURN(0);
		AnlzTracePtr a(*this, pOpRoot);
		a.__tracePtrForward();
	}
	else
	{
		for (XOpList_t::Iterator i(mpOp->m_xout); i; i++)
		{
			OpPtr pOp = i.data();
	//		if (pOp->isPtr())
	//			continue;//already
			if (pOp->IsIndirectB())
	//			if (!pOp->m_pData)
					continue;
//			if (IsPrimeOp(pOp))
//				if (pOp->Action() != ACTN_MOV)
//					if (pOp->Action() != ACTN_ADD)
//						if ((pOp->Action() != ACTN_SUB) || (pOp->OpC() == OPC_ CPUSW))
//							continue;

			AnlzTracePtr a(*this, pOp);
			a.__tracePtrForward();
		}

	}

	RETURN(0);
}
#undef RETURN

void AnlzTracePtr::TracePtrForward() const
{
//CHECK(pri meOp()->No() == 54)
//STOP

//	assert(IsPrimeOp());
	if (IsPrimeOp(mpOp) && IsCall(mpOp))
	{
		for (XOpList_t::Iterator i(mpOp->m_xin); i; i++)
		{
			AnlzTracePtr an(*this, i.data());
			an.TracePtrForward();
		}
		return;
	}

//	if (!isPtr())
//		return;
//return;
	__tracePtrForward();
}

void AnlzTracePtr::TraceArgPtrs() const
{return;/*?assert(0);
	assert(IsCall());

CHECK(No() == 101)
STOP

	int i = 0;
	for (OpPtr pArg = Get Args(); pArg; pArg = pArg->Ne xt())
	{
		OpPtr pEntryOp = pArg->FindCalleeArg();
		if (!pEntryOp)
			continue;//call with argsexs?

		if (pArg->OpSize() != pEntryOp->OpSize())
			continue;

		if (!pEntryOp->isPtr())
			continue;

//		bool bThis = false;
//		if (pArg->OpC() == OPC_CPUREG)
//		{
//			OpPtr pThisPtr = GetFuncDef()->m_pThisPtr;
//			if (pThisPtr)
//				if (pThisPtr->EqualTo(pArg))
//					bThis = true;
//		}

		if (NEW_TRACEPTR)
		{
			EXPRptr_t expr(mrFunc, 0);
			expr.TracePtr(pArg);
		}
		else
		{
			while (pArg->Trace Ptr((FieldPtr)pEntryOp) == -1);
		}

		i++;
	}*/
}

/*OpPtr Op_t::GetExitOp()
{
	assert(IsCallOutOp());
	OpPtr pOpCall = prim eOp();
	FuncDef_t *pfDef = pOpCall->GetFuncDef();
	return pfDef->FindExitOp(this, pOpCall);
}*/

#define RETURN(arg) { tr.trace &= ~TRACE_OVERLAPPED; return(arg); };
int ptrinfo_t::__tracePtrSource(HOP pSelf, ptrinfo_t &pi) const
{
	op_tracer_cell_t &tr(opTracer().cell(pSelf));
	if (tr.trace & TRACE_OVERLAPPED)
		return 0;
	tr.trace |= TRACE_OVERLAPPED;

	if (pSelf->OpSize() != mrDC.PtrSize())
		RETURN(0);

	OpPtr pOpSrc = 0;
	if (!IsCodeOp(pSelf))
	{
//		if (IsCallOutOp())
//			pOpSrc = GetExitOp();
//		else
			pOpSrc = pSelf;
	}
	else if (IsAddr(pSelf))//mloc's offset
	{
		pOpSrc = pSelf;
	}
	else if (pSelf->IsScalar())
	{
		pi.disp += (pi.sign)?(-OpDisp(pSelf)):(OpDisp(pSelf));
	}
	else if (pSelf->IsIndirectB())
	{
		pOpSrc = pSelf;
	}
	else if (LocalRef(pSelf) && (!IsLocalOp(pSelf) || !pSelf->XIn()))//global ptr
	{
		pOpSrc = pSelf;
	}
	else 
	{
		if (IsPrimeOp(pSelf))
		{
			OpPtr pOp0, pOp1;
			bool bOp1 = true;
			bool bSign = false, bSignSv;

			switch (ActionOf(pSelf)) {
			case ACTN_MOV:
				pOp0 = pSelf->arg1();
				if (!__tracePtrSource(pOp0, pi))
					RETURN(0);
				break;

			case ACTN_SUB:
				bSign = true;

			case ACTN_ADD:
				pOp0 = pSelf->arg1();
				pOp1 = pSelf->arg2();
		
				if (!__tracePtrSource(pOp0, pi))
					RETURN(0);

				bSignSv = pi.sign;
				pi.sign = bSign;
				if (!__tracePtrSource(pOp1, pi))
					RETURN(0);
				pi.sign = bSignSv;
				if (!pi.pOp)
					RETURN(0);
				break;

			default:
				RETURN(0);
			}//switch
		}
		else
		{
			if (pSelf->m_xin.check_count(1) != 0)
				RETURN(0);
			if (!__tracePtrSource(pSelf->XIn()->data(), pi))
				RETURN(0);
		}
	}

	if (pOpSrc)
	{
		if (pi.pOp)
			RETURN(0);
		pi.pOp = (OpPtr)pOpSrc;
	}

	RETURN(1);
}
#undef RETURN

int ptrinfo_t::TracePtrSource(HOP pOp)
{
	assert(pOp->IsIndirectB());

	if (pOp->m_xin.empty())
		return 0;

	ptrinfo_t pi2(*this);
	for (XOpList_t::Iterator i(pOp->m_xin); i; i++)
	{
		OpPtr pOpIn = i.data();
		if (!__tracePtrSource(pOpIn, pi2))
			return 0;

		if (i.is_first())
		{
			pi2.CopyTo(*this);
		}
		else
		{
			if (!pi2.EqualTo(*this))
				return 0;
		}
	
		pi2.Clear();
	}

	return 1;
}

STAGE_STATUS_e AnalyzePtrs_t::run()//FieldMap &cachedArgs)
{
	if (!testChangeInfo(FUI_PTRS))
		return STAGE_STATUS_e::SKIPPED;//not dirty
//mrFunc.ReNumberize();
#ifdef _DEBUG
	MyString sFunc(OwnerFuncName());
	ADDR vaFunc(DockAddr());
	//TRACE1("%s: Reconstructing data structures...\n", sFunc.c_str());
#endif

	m_arrPtrs.clear();

#if(0)
	{//make sure all op args are attached to the fields
		LocalsTracer_t an(*this);
		//an.setCahchedArgs(&cachedArgs);
		for (PathOpList_t::Iterator i(mrFuncDef.entryOps()); i; i++)
			an.CheckLocal0(PRIME(i.data()));
	}
#endif

	//OpTracer_t op_tracer;
	//setOpTracer(&op_tracer);

	opTracer().reset(*this);//clear all

//CHECK(CompName("sub_20791D30"))
//STOP

	for (PathTree_t::LeafIterator i(mrPathTree.tree()); i; i++)
	{
		HPATH pPath(i.data());
		for (PathOpList_t::Iterator j(pPath->ops()); j; j++)
		{
			AnlzTracePtr an(*this, PRIME(j.data()));
			an.TracePtrBase(*this);
			an.TracePtrArgs();
		}
	}

	//int z = mrDC.arrPtrs.m.count();
	//XOpLink_t *ll = mrDC.arrPtrs.m.tail();

	for (std::set<HOP>::const_iterator i(m_arrPtrs.begin()); i != m_arrPtrs.end(); i++)
	{
		HOP pOp(*i);
		//CHECK(pOp->IsArgOp())
		//STOP
		assert(mrFuncDef.Body() == GetOwnerBody(pOp));
		AnlzTracePtr an(*this, pOp);
		an.TracePtrForward();//if this call adds xref, it goes to tail
	}
	/*
		for (HOP pArg = GetEntryOps(); pArg; pArg = pArg->Nex t())
		{
			AnlzTrace Ptr a(pArg);
			a.TracePtrForward();
		}

	//	for (int i = 0; i < 2; i++)
		for (PathTree_t::LeafIterator i(func def()->pathTree().tree()); i; i++)
		{
			HPATH pPath(i.data());
			for (OpList_t::Iterator j(pPath->ops()); j; j++)
			{
				AnlzTrace Ptr a(j.data());
				a.TracePtrForward();
			}
		}
	*/

	clearChangeInfo(FUI_PTRS);
	return STAGE_STATUS_e::DONE;
}




