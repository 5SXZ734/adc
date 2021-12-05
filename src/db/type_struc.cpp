#include "type_struc.h"
#include "prefix.h"

#include "shared/defs.h"
#include "shared/link.h"
#include "shared/misc.h"
#include "mem.h"
#include "obj.h"
#include "field.h"
#include "type_seg.h"
#include "type_strucvar.h"
#include "type_module.h"
#include "info_proj.h"
#include "main.h"
#include "names.h"
#include "clean.h"

////////////////////////////////////////////Struc_t::HierIterator

Struc_t::HierIterator& Struc_t::HierIterator::operator ++()
{
	CFieldRef a(*VALUE(back().second));
	TypePtr iType(a.type());
	if (!iType)
		back().second++;
	else
	{
		iType = ProjectInfo_t::SkipArray(iType);
		if (iType->isShared() && !mbStepIntoShared)
			back().second++;
		else if (!iType->typeStruc() || !push(iType))
			back().second++;
	}
	while (!empty() && back().second == back().first->typeStruc()->fields().end())
		pop();
	return *this;
}

TypePtr Struc_t::HierIterator::NamespaceStrucSlow() const
{
	CTypePtr iStruc(nullptr);
	if (!empty())
	{
		for (const_reverse_iterator ri(rbegin()); ri != rend(); ri++)
		{
			CTypePtr p((*ri).first);
			if (p->typeComplex()->namesMgr())
			{
				iStruc = p;
				break;
			}
		}
	}
	assert(!iStruc || !iStruc->typeStruc()->namesMgr() || (!mScoping.empty() && mScoping.back() == iStruc));//later
	return (TypePtr)iStruc;
}


///////////////////////////////////////////////////////// Struc_t::HierIterator2

TypePtr Struc_t::HierIterator2::NamespaceStruc() const
{
	if (!empty())
	{
		for (const_reverse_iterator ri(rbegin()); ri != rend(); ri++)
		{
			TypePtr iStruc((*ri).first);
			if (iStruc->typeComplex()->namesMgr())
				return iStruc;
		}
	}
	return nullptr;
}

Struc_t::HierIterator2& Struc_t::HierIterator2::operator ++()
{
	CFieldRef a(*VALUE(back().second));
	TypePtr iType(a.type());
	if (!iType)
		back().second++;
	else
	{
		iType = ProjectInfo_t::SkipArray(iType);
		if (!iType->typeStruc() || !push(iType))
			back().second++;
	}
	while (!empty() && back().second == back().first->typeStruc()->fields().end())
		pop();
	return *this;
}


////////////////////////////////////////////////////////////////
// Struc_t

Struc_t::Struc_t()
: mpTypesMgr(nullptr)
{
}

Struc_t::Struc_t(int id)
: Complex_t(id),
mpTypesMgr(nullptr)
{
}

Struc_t::~Struc_t()
{
	assert(mFields.empty());
	assert(!mpTypesMgr);
	//assert(mMethods.empty());
	//assert(!hasUserData());

//?	delete mpNa mespace;

//	while (m_pMethods)
//	{
//		m_pMethods->pFunc->m_pfDef->KillThisPtr();
//		//delete m_pMethods;
//	}
}

Struc_t::Struc_t(const Struc_t &)
{
	assert(0);
}

ADDR Struc_t::base(CTypeBasePtr pSelf0) const
{
	CTypePtr pSelf((CTypePtr)pSelf0);
	if (pSelf->parentField())
	{
		assert(!pSelf->isShared());
		return pSelf->parentField()->_key();
	}
	return Base_t::base(pSelf);
}

Struc_t &Struc_t::operator=(const Struc_t &o)
{
	assert(0);
	this->Complex_t::operator=(o);

//	mSize = o.mSize;

	mpTypesMgr = o.mpTypesMgr;
	assert(!mpTypesMgr);
	//assert(mMethods.empty() && o.mMethods.empty());
	//assert(!hasUserData() && !o.hasUserData());
	return *this;
}

void Struc_t::moveFrom(Struc_t &o)
{
	Base_t::moveFrom(o);
	mFields.MoveFrom(o.fields());
	//mFields = std::move(o.fields());

	mpTypesMgr = o.mpTypesMgr;
	o.mpTypesMgr = nullptr;
}

/*void Struc_t::CopyStrucFrom(const Struc_t &o, MemoryMgr_t &rMemMgr)
{
	//this->Complex_t::operator=(o);
	assert(0);

	mSize = o.mSize;
	assert(mFields.empty());
	for (FieldMapCIt i(o.mFields.begin()); i != o.mFields.end(); i++)
	{
		FieldPtr pField(rMemMgr.NewFieldNoId());
		pField->CopyFrom(*i->second, rMemMgr);
		mFields.insert(std::make_pair(i->first, pField));
		pField->setOwnerComplex(type obj());
	}

	mpTypesMgr = o.mpTypesMgr;
	assert(!mpTypesMgr);
	//assert(mMethods.empty() && o.mMethods.empty());
	assert(!hasUserData() && !o.hasUserData());
}*/

/*bool Struc_t::clearFieldList(MemoryMgr_t &rMemMgr, FieldMap &mFields)
{
	if (!mFields.empty())
	{
		NamesMgr_t *pNS(OwnerNamesMgr(type obj(), nullptr));

		FuncDef_t *pfDef(nullptr);
		FieldMapIt it(mFields.begin());
		if (it != mFields.end())
		{
			FieldPtr pField(VALUE(it));
			if (pField->type())
				pfDef = pField->type()->typeFuncDef();
			if (pfDef)
				it++;//funcdef must be deleted last!
		}

		for (; it != mFields.end();)
		{
			FieldPtr pField(VALUE(it));
			//assert(pField->OwnerStruc() == this);
			assert(!pField->type() || !pField->type()->typeFuncDef());

CHECKID(pField, 0x13f)
STOP

			FieldMapIt it_p(it++);

			pNS->ClearObjName(pField, rMemMgr);
			if (pField->destruct(rMemMgr))
			{
				mFields.erase(it_p);
				rMemMgr.Delete(pField);
			}
		}

		if (!mFields.empty())
		{
			if (pfDef)
			{
				FieldPtr pField(VALUE(mFields.begin()));
				assert(pField->nameless());
				pField->takeType();
				mFields.erase(mFields.begin());
				rMemMgr.Delete(pField);
			}
		}
		if (!mFields.empty())
			return false;
	}
	return true;
}*/

int Struc_t::IsIncapsulatedWith(Struc_t *pStruc, int *pOffs)//check if this struc contained in pStruc
{
	assert(pStruc);

	ADDR d(0);

	for (FieldMapIt it = pStruc->mFields.begin(), E = pStruc->mFields.end(); it != E; ++it)
	{
		FieldPtr  pField = VALUE(it);
		if (pOffs)
		{
			if (pField->_key() > d)
				*pOffs += pField->_key() - d;
			else
				*pOffs -= d - pField->_key();
			d = pField->_key();
		}

		if ( pField->IsComplex0() )
		{
			assert(!pField->OpTypeZ());
			assert(0);//?if (pField->type() == this)
				return 1;
			
				if (IsIncapsulatedWith(pField->type()->typeStruc(), pOffs))
				return 1;
		}
	}

	return 0;
}

/*int Struc_t::size()
{
	assert(ObjType() == OBJID_TYPE_STRUC);
#ifdef _DEBUG
	static Struc_t * p = nullptr;
	if (!p)
		p = this;
	else if (p == this)
		assert(false);//to prevent stack overflow
#endif

	int nSize = 0;
	if (!mFields.empty())
	{
		FieldPtr  pFieldLast = VALUE(mFields.rbegin());
		nSize = pFieldLast->offset() + pFieldLast->size();
	}
#ifdef _DEBUG
	if (p == this)
		p = nullptr;
#endif
	return nSize;
}*/

/*int Struc_t::offset() const
{
	if (typ eobj()->parentField())
		return typ eobj()->parentField()->offset(); 
	return 0;
}

int Struc_t::Type() const
{
	return ((typ eobj()->flags() >> 8) & 0xFF);
}*/

int Struc_t::size(CTypePtr iSelf) const
{
CHECKID(iSelf, 0x7a2e3)
STOP
//	if ((int)mSize > 0)
	//	return mSize;//???


	if (isUnion(iSelf))
	{
		int sz_max(0);
		for (FieldMapCIt i = mFields.begin(), E = mFields.end(); i != E; ++i)
		{
			int sz(i->sizeBytes());
			if (sz > sz_max)
				sz_max = sz;
		}
		return sz_max;
	}

	ADDR lower(0);
	ADDR sz(0);
	if (!mFields.empty())
	{
		FieldMapCRIt it(mFields.rbegin());
		//--it;
		ADDR offs(KEY(it));
		lower = offs - iSelf->base();
		CFieldPtr pField(VALUE(it));
		if (!ProjectInfo_s::IsEosField(pField))
		{
			for (;;)
			{
				TypePtr iType(pField->type());
				if (iSelf->typeBitset())
				{
					Array_t* pArray(iType->typeArray());
					if (pArray)
						sz = std::max(sz, ADDR(pArray->total()));
					else
						sz = std::max(sz, ADDR(OPSZ_BYTE));//BIT
				}
				else
				{
					int f_sz(iType ? iType->sizeBytes() : 0);
					if (f_sz > 0)
						sz = std::max(sz, ADDR(f_sz));
					else if (f_sz == 0)
						sz = std::max(sz, ADDR(OPSZ_BYTE));//?
					else
						return f_sz;
				}

				if (++it == mFields.rend())
					break;
				ADDR offs2(KEY(it));
				if (offs2 != offs)
				{
					assert(offs2 < offs);
					break;
				}
				pField = VALUE(it);
			}
		}
	}

	//if (sz == 0)
		//return 1;
	return lower + sz;
}

//nMode:
//0 - scan root list for first field, 
//		that will inline with nOffset or overlap it;
//1 - go deep into tree to reach terminal node that exactly inline with nOffset,
//		if there is no such one, so return with 0;
//2 - go deep into tree to reach first node inline with nOffset; 
//		note - this may not be a terminal one;
FieldPtr Struc_t::GetFieldEx(ADDR nOffset, int nMode)
{
//	assert(isValid());
	if (mFields.empty())
		return nullptr;

	FieldPtr  pField = nullptr;
	FieldPtr pFieldPr = nullptr;
	for (FieldMapIt it = mFields.begin(), E = mFields.end(); it != E; ++it)
	{
		pField = VALUE(it);
		if (pField->_key() >= nOffset)
			break;
		pFieldPr = pField;
	}

	if (pField)
	if (pField->_key() == nOffset)
	{
		if (nMode == 1)
			if (pField->IsComplex0())
				return pField->type()->typeStruc()->GetFieldEx(0, nMode);
		return pField;//nMode==0 or nMode==2
	}

	if (pFieldPr)
	if (pFieldPr->_key() + pFieldPr->size() > nOffset)
	{
		if (nMode == 1 || nMode == 2)
			if (pFieldPr->IsComplex0())
				return pFieldPr->type()->typeStruc()->GetFieldEx(nOffset - pFieldPr->_key());
		return pFieldPr;//nMode==0
	}
	
	return 0;//nothing at all!
}

bool Struc_t::IsMineField(FieldPtr  pField) const
{
	return pField->OwnerStruc() == this;
}

CFieldPtr Struc_t::hasOnlyChild() const
{
	FieldMapCIt i(mFields.begin());
	if (i != mFields.end())
	{
		FieldMapCIt j(i++);
		if (i == mFields.end())
			return VALUE(j);
	}
	return nullptr;
}

FieldPtr Struc_t::takeField(ADDR va)
{
	FieldPtr pField(takeField0(va));
	if (!pField)
		return nullptr;
	pField->setOwnerComplex(nullptr);
	return pField;
}

FieldPtr Struc_t::takeField(FieldPtr pField)
{
	if (!IsMineField(pField))//this is cheap
		return nullptr;

	for (FieldMapIt it = mFields.begin(), E = mFields.end(); it != E; ++it)
	{
		if (VALUE(it) == pField)
		{
			takeField0(it);
			pField->setOwnerComplex(nullptr);
			return pField;
		}
	}

	assert(0);
	return nullptr;
}

FieldPtr Struc_t::takeField0(ADDR va)
{
	FieldMap &m(fields());
	FieldMapIt it(m.find(va));
	if (it == m.end())
		return nullptr;
	return takeField0(it);
}

FieldPtr Struc_t::takeField0(FieldMapIt it)
{
	FieldMap &m(fields());
	return m.take(it);
}

int Struc_t::MergeWith(TypePtr pStruc)
{
	/*
#if(0)//!
	assert(IS _PTR(this));

	assert(pStruc != this);
	assert(isValid());
	assert(pStruc->isValid());

	bool bfDef = false;
	FuncDef_t *pf1 = (FuncDef_t *)this;
	FuncDef_t *pf2 = (FuncDef_t *)pStruc;

	if (IsFuncDef())
	{
		if (pStruc->IsFuncDef())
			bfDef = true;
		else
			return 0;
			//assert(false);
	}
	else
		if (pStruc->IsFuncDef())
			return 0;

	if (bfDef)
	{
		if (!pf1->IsEqualTo(pf2))
			return 0;//?

/ *		if (!pf1->m_pThisPtr)
		{
			if (pField == pf2->m_pThisPtr)
				pf1->m_pThisPtr = pFieldNew;
		}

		if (!pf1->m_nSavedRegs && pf2->m_nSavedRegs)
			pf1->m_nSavedRegs = pf2->m_nSavedRegs;
		if (!pf1->Sta ckOut() && pf2->Sta ckOut())
			pf1->SetSt ackOut(pf2->Stac kOut());
		if (!pf1->getFStackPurge() && pf2->getFStackPurge())//ok
			pf1->setFStackPurge(pf2->getFStackPurge());//ok
* /	}
	else
	{

		for (XRef_t *pXRef = pStruc->m_pXRefs; pXRef; pXRef = pXRef->Ne xt())
		{
			FieldPtr pField = pXRef->pField;
			if (!pField->IsField())
				continue;
			if (pField->isPtr())
				continue;
			if (pField->CheckOwner(this))
				return 0;
		}

		for (FieldPtr pField = pStruc->m_pFields; pField; pField = (FieldPtr )pField->N ext())
		{
			uint8_t t = pField->OpType();

			FieldPtr pFieldNew;
			if (IS _PTR(pField->m_pStruc))
			{
				pFieldNew = AddF ield(pField->_key());
				if (!pFieldNew)
					continue;

				if (AgreeTypes(pFieldNew->m_optyp, t))
				{
					if (pField->IsComplex0())//complex
					{
						pFieldNew->ApplyStruct(pField->m_pStruc->typeStruc());
					}
					else
					{
//!						assert(pField->isPtr());
if (pField->isPtr())
pField->SetPtr(1);
						if (!pFieldNew->m_pStruc)
							pFieldNew->SetStruc0(pField->m_pStruc);
						else if (pFieldNew->m_pStruc != pField->m_pStruc)
						{
							//assert(pField->m_pStruc != this);
							if (pField->m_pStruc != pStruc)
							if (pField->m_pStruc != this)
								pFieldNew->m_pStruc->typeStruc()->MergeWith(pField->m_pStruc->typeStruc());
						}
					}
				}
				else
				{
					pFieldNew->OvercastType(pField->OpType() & 0xF0);
				}

	/ *			if (pFieldNew->OpSize() == 0)
					pFieldNew->optyp |= pField->OpSize();
				else if (pFieldNew->OpSize() != pField->OpSize())
					assert(false);

				if (!(pFieldNew->OpType() & 0xF0))
					pFieldNew->SetOpType(pField->OpType());
				else if (pFieldNew->OpType() != pField->OpType())
				{
					if (pFieldNew->isPtr() && pField->isPtr())
					{
						if (!(pFieldNew->optyp & 0x70))
							pFieldNew->SetOpType(pField->OpType());
	//?					else
	//?						assert(false);
					}
					else
						assert(false);
				}
	* /		}
			else
			{
				pFieldNew = AddF ield(pField->offset(), true);
				if (!pFieldNew)
					continue;
				AgreeTypes(pFieldNew->m_optyp, t);
			}

			assert(!pFieldNew->OpC());
			assert(!pField->OpC());

			if (IS _PTR(pFieldNew->m_pStruc))
				assert(pFieldNew->isPtr() || pFieldNew->m_pStruc->ObjType() == OBJID_TYPE_STRUC);
		}
	}


	TRACE2("Structure %s merged into %s\n", pStruc->Name(), Name());

	RedirectXRefs(pStruc);
	delete pStruc;

	CheckInclusion();
	assert(isValid());
#endif*/
	return 1;
}

/*void Struc_t::ShiftFields(int dOffs)
{
	for (FieldMapIt it = mFields.begin(); it != mFields.end(); it++)
	{
		FieldPtr  pField = VALUE(it);
		pField->setOffset( pField->offset() + dOffs );
	}
}*/

/*bool Struc_t::IsUnnamed()
{
	if ( mpParent )
		return mpParent->nameless();
	if ( mpName )
		return false;
	return true;
}*/

void Struc_t::namex(MyString &s) const
{
	assert(!nameless());
	s.append(name()->c_str());
}

int Struc_t::ApplyStruct(int nOffset, Struc_t *pStruc)
{
	return 0;
}

/*void Fi le_t::CheckStrucInclusion(Struc_t *pSelf)
{
	Fi le_t *pOwnerFile(this);// = pSelf->GetOwnerFile();
	//if (!pOwnerFile)
	//	return;

	for (XRefObjList_t::Iterator i(pSelf->xre fs_()); i; i++)
	{
		Obj_t *pOp = i.data().pObj;
		assert(0);
		Fil e_t *pFile = 0;//?pOp->GetOwnerFile();
		if (pFile)
		if (pFile->IsHeader())
		{
		}
		else//declared not in header
		{
			if (pFile == pOwnerFile)
				continue;

			Fil e_t *pHeader = pOwnerFile;
			if (!pHeader->IsHeader())
			{
				Files().MoveToDefaultHeader(this);
				assert(0);
				//?pHeader = GetOwnerFile();
				pOwnerFile->addIncludeList(pHeader);
			}
			pFile->addIncludeList(pHeader);
		}
	}
}*/

int Struc_t::OnSizeIncrease()
{/*?
	for (XRef_t *pXRef = m_pXRefs; pXRef; pXRef = pXRef->Nex t())
	{
		FieldPtr  pField = pXRef->pField;
		if (!pField->IsDataOp())
			continue;
		if (pField->isPtr())
			continue;
		FieldPtr pData = pField->GetOwnerData();
		if (pData->IsL ocal())
			continue;
		pData->OnSizeIncrease();
	}
*/
	return 1;
}

bool Struc_t::IsEqualTo(Struc_t *p)
{
	if (ObjType() != p->ObjType())
		return false;

	FieldMapIt it1 = mFields.begin(); 
	FieldMapIt it2 = p->fields().begin(); 
	
	FieldMapIt E1(mFields.end());
	FieldMapIt E2(p->fields().end());
	while (it1 != E1 && it2 != E2)
	{
		FieldPtr  pField1 = VALUE(it1);
		FieldPtr  pField2 = VALUE(it2); 

		if (!pField1->IsEqualTo(pField2))
			return false;

		it1++;
		it2++;
	}

	if (it1 != E1 || it2 != E2)
		return false;

	return true;
}


bool Struc_t::isUnion(CTypePtr pSelf) const
{
//	if (typeUnion())
	//	return true;
	if (!maybeUnion())
		return false;
	if (pSelf->isEnum())
		return false;
	if (mFields.empty())
		return false;
	ADDR uBase(base(pSelf));
	CFieldPtr pFirst(VALUE(mFields.begin()));
	if (pFirst->flags() & FLD_HIER_PUBLIC)
		return false;
	if (pFirst->_key() != uBase)
		return false;
	CFieldPtr pLast(VALUE(mFields.rbegin()));
	if (pFirst == pLast)
		return false;//just 1 member - not a union
	if (pLast->_key() != uBase)
		return false;
	return true;
}





//////////////////////////////////////////////////
// TypeUnion_t

/*int TypeUnion_t::size(CTypePtr) const
{
	int sz_max(0);
	//assert(mFields.front()->offset() == 0);
	for (FieldMapCIt i = mFields.begin(), E = mFields.end(); i != E; ++i)
	{
		int sz(i->size());
		if (sz > sz_max)
			sz_max = sz;
	}
	return sz_max;
}*/





//////////////////////////////////////////////////

void Bitset_t::insertField(FieldPtr p)
{
	if (!fields().insert_unique(p).second)
		ASSERT0;
}

/*int Bitset_t::size(CTypePtr iSelf) const
{
	int sz(Struc_t::size(iSelf));
	int sz2(BITS2BYTES(sz) * CHAR_BIT);//size by last field (or fixed)
	if (fields().empty())
		return sz2;
	FieldPtr f(VALUE(fields().begin()));//first
	TypePtr iType(f->type());
	Array_t *pArray(iType->typeArray());
	if (pArray)
		iType = pArray->baseType();
	assert(iType->typeSimple());
	int sz3(iType->size() * CHAR_BIT);
	return std::max(sz2, sz3);
}*/

unsigned Bitset_t::sizeBytes(CTypePtr) const
{
#if(1)
	if (fields().empty())
		return 0;
	CFieldRef f(*(fields().begin()));//last
	TypePtr iType(f.type());
	if (!iType)
		return (unsigned)-1;//when dumping under strucvars
	Array_t *pArray(iType->typeArray());
	if (pArray)
		iType = pArray->baseType();
	assert(iType->typeSimple() && !iType->typePtr());
	return iType->size();
#else
	return upper_power_of_two(BITS2BYTES(n));
#endif
}

unsigned Bitset_t::upperGap(TypePtr iSelf) const
{
	unsigned sz(size(iSelf));
	if (fields().empty())
		return sz;
	CFieldRef f(*(fields().rbegin()));//last
	unsigned sz2(f._key());
	TypePtr iType(f.type());
	Array_t *pArray(iType->typeArray());
	if (pArray)
		sz2 += unsigned(pArray->total());
	else
		sz2 += OPSZ_BYTE;//bit!
	if (sz2 > sz)//?
		sz2 = sz;
	return sz2 - sz;
}

unsigned Bitset_t::upperBound() const
{
	if (fields().empty())
		return 0;
	CFieldRef f(*(fields().rbegin()));//last
	unsigned sz(f._key());
	TypePtr iType(f.type());
	Array_t *pArray(iType->typeArray());
	if (pArray)
		sz += unsigned(pArray->total());
	else
		sz += OPSZ_BYTE;//bit!
	return sz;
}



// insert field ///////////////////////////////////////////

FieldMapIt Struc_t::insertUniqueFieldIt(ADDR key, FieldPtr pField, FieldPtr *ppEos)
{
//CHECK(key == 0x10061e3)
//STOP

	FieldPtr pEos(nullptr);
	if (!mFields.empty())//before insertion
	{
		FieldPtr pLast(VALUE(mFields.rbegin()));
		if (ProjectInfo_s::IsEosField(pLast))
			pEos = pLast;
	}

	pField->overrideKey(key);
	std::pair<FieldMapIt, bool> res(mFields.insert_unique(pField));
	if (!res.second)
	{
		FieldPtr pOther(VALUE(res.first));
		if (ProjectInfo_s::IsEosField(pOther, mFields))
		{
			mFields.take(res.first);
			res = mFields.insert_unique(pField);
			assert(res.second);
			*ppEos = pOther;//let the caller get rid of it
			return res.first;
		}
		return mFields.end();
	}

	//check the predecessor if it's an eos field

#if(0)
	if (pEos && &mFields.back() == VALUE(res.first))
	{
		FieldMapIt itEos(pEos);
		mFields.take(itEos);
		*ppEos = pEos;//let the caller get rid of it
	}
#endif

	return res.first;
//#endif
}

FieldMapIt Struc_t::insertFieldIt(FieldPtr pField, FieldPtr *ppEos)
{
	FieldMapIt i(mFields.insert(pField));
	assert(i != mFields.end());
	if (ppEos)
	{
		//check the predicessor if it's an eos field
		FieldMapRIt rit(i);
		if (++rit != mFields.rend())
		{
			FieldPtr pPrior(VALUE(rit));
			if (ProjectInfo_s::IsEosField(pPrior))
			{
				mFields.take(rit);
				*ppEos = pPrior;//let the caller get rid of it
			}
		}
	}
	return i;
}


/*FieldPtr  TypeUnion_t::getFieldByIndex( int index, FieldPtr * ppFieldNx ) const
{
	for ( std::list< FieldPtr  >::const_iterator it = mFields.begin(); it != mFields.end();	it++ )
	{
		if ( index == 0 )
		{
			std::list< FieldPtr  >::const_iterator it_pr = it++;
			if ( ppFieldNx != nullptr )
			{
				if ( it != mFields.end() )
					*ppFieldNx = *it;
			}
			return *it_pr;
		}
		index--;
	}

	return nullptr;
}*/






