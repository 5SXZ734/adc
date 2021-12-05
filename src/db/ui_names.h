#pragma once

#include "ui_main.h"

#define PTR_LSB	1	//a way to distinguish a duplicated keys (ptrs) in a associative container


/////////////////////////////////////////////// NamesViewModel_t (flat view, knows nothing about noname entries)
class NamesViewModel_t : public adcui::INamesViewModel
{
protected:
	Core_t& mrCore;
	FolderPtr	mpNFolder;//names folder
	NamesMgr_t* mpNMgr;
#ifdef _DEBUG
	enum { COL_NAME, COL_ID, COL_TOTAL };
#else
	enum { COL_NAME, COL_TOTAL };
#endif
public:
	NamesViewModel_t(Core_t& rCore, FolderPtr pFolder)
		: mrCore(rCore),
		mpNFolder(pFolder),
		mpNMgr(mpNFolder->fileNames()->namesMgr())
	{
	}

	FolderPtr namesFolder() const {
		return mpNFolder;
	}

	virtual const char* moduleName()
	{
		return ProjectInfo_t::ModuleTitle(mpNFolder).c_str();
	}

	virtual bool apply(const char* typeStr)
	{
		//MAIN.postEvent(new adc::CEventCommand(MyStringf("makeobj %s", typeStr), false));
		return true;
	}

	virtual bool goToDefinition(ITEMID item) const
	{
		return false;
	}
protected:
	virtual int columnCount() const
	{
		return COL_TOTAL;
	}

	virtual void reset()
	{
		if (mpNFolder)
			mpNMgr = mpNFolder->fileNames()->namesMgr();
	}

	virtual void fetch(ITEMID) const
	{
	}

	virtual bool empty() const {
		return mpNMgr->empty();
	}
	virtual size_t size() const {
		return mpNMgr->size();
	}
	virtual size_t rank(PNameRef pn) const
	{
		return mpNMgr->rank(pn);
	}
	virtual NamesMapIt atIt(size_t i) const
	{
		return mpNMgr->atIt(i);
	}
protected:
	virtual bool hasChildren(ITEMID item, bool /*bFetched*/) const
	{
		if (item == 0)
			return !empty();
		if (item & PTR_LSB)
			return false;//dups do not expand
		CObjPtr pObj(toObjPtr(item));
		FieldPtr pField(pObj->objField());
		if (pField)
			return 0;//fields do not expand
		TypePtr iType(pObj->objTypeGlob());
		assert(iType);
		Complex_t* pCplx(iType->typeComplex());
		if (!pCplx)
			return 0;
		NamesMgr_t* pnm(pCplx->namesMgr());
		if (!pnm)
			return 0;
		return !pnm->empty() && pnm->size() > 0;
	}

	virtual unsigned childrenNum(ITEMID item) const
	{
		if (!item)
			return (unsigned)size();
		assert(!(item & PTR_LSB));
		CObjPtr pObj((ObjPtr)item);
		CTypePtr pType(pObj->objTypeGlob());
		assert(pType);
		Complex_t* pCplx(pType->typeComplex());
		assert(pCplx);
		const NamesMgr_t* pnm(pCplx->namesMgr());
		assert(pnm);
		return (unsigned)pnm->size();
	}
	virtual ITEMID idOfChild(ITEMID parent, unsigned childIndex) const
	{
		//CHECK(childIndex == 0x6c93)
		//STOP
		NamesMgr_t* pnm;
		PNameRef pn;
		if (!parent)
		{
			pnm = mpNMgr;
			pn = &(*atIt(childIndex));
		}
		else
		{
			ObjPtr pParentObj(toObjPtr(parent));
			TypePtr iType(pParentObj->objTypeGlob());
			assert(iType);
			Complex_t* pCplx(iType->typeComplex());
			pnm = pCplx->namesMgr();
			pn = &(*pnm->atIt(childIndex));
		}
		//PNameRef pn(nit->first);
		assert(pn);
		ObjPtr pObj(pn->obj());
//CHECKID(pObj, 864)
//STOP
		ITEMID item = ITEMID(pObj);
		FieldPtr pField(pObj->objField());
		if (pField)
		{
			assert(!(item & PTR_LSB));//in case if a pointer is unaligned(!)
			if (pField->name() != pn)
				item |= PTR_LSB;
		}
		else
		{
			TypePtr pType(pObj->objTypeGlob());
			assert(pType);
			if (!pType->hasPvt() || pType->name() != pn)
				item |= PTR_LSB;
		}
		//assert(idOfParent(item) == (ITEMID)parent);
		return item;
	}
	virtual ITEMID idOfParent(ITEMID child) const//agnostic of pretty names
	{
		ObjPtr pObj(toObjPtr(child));
		assert(pObj);

		TypePtr iOwner;
		FieldPtr pField(pObj->objField());
		if (pField)
		{
			iOwner = nameOwner(pField, (child & PTR_LSB) != 0);
		}
		else
		{
			TypePtr pType(pObj->objTypeGlob());
			iOwner = pType->owner();
		}
		if (!iOwner || iOwner->typeSeg())//why !iOwner?
			return 0;//global
		assert(iOwner->isShared());
		return (ITEMID)iOwner;
	}
	virtual unsigned indexOf(ITEMID item) const//this knows nothing about pretty names
	{
		ObjPtr pObj(toObjPtr(item));
		assert(pObj);
		PNameRef pn;
		NamesMgr_t* pns0;
		FieldPtr pField(pObj->objField());
		if (pField)
		{
			pns0 = ProjectInfo_t::OwnerNamesMgr(pField->owner(), nullptr);
			pn = pField->name();
			assert(pn);
		}
		else
		{
			TypePtr pType(pObj->objTypeGlob());
			pns0 = ProjectInfo_t::OwnerNamesMgr(pType->owner(), nullptr);
			pn = pType->name();
			//assert(iOwner);
		}
		return (unsigned)rankImpl(pns0, pn);
	}
	virtual NameTypeEnum type(ITEMID item, size_t column) const
	{
		NameTypeEnum eKind(E_NONE);
		if (column == COL_NAME)
		{
			ObjPtr pObj(toObjPtr(item));
			assert(pObj);
			FieldPtr pField(pObj->objField());
			if (pField)
			{
				if (pField->owner()->typeSeg())
				{
					if (pField->isTypeImp())
						eKind = E_IMPORTED;
					else
					{
						if (pField->isTypeProc())
							eKind = E_FUNCTION;
						else
							eKind = E_GLOBAL;
						if (pField->isExported())
							eKind = NameTypeEnum(eKind | E_EXPORTED);
					}
				}
				else
					eKind = E_FIELD;
			}
			else
				eKind = E_TYPE;
			if (item & PTR_LSB)
				eKind = NameTypeEnum(eKind | E_ALIAS);
		}
		return eKind;
	}
	virtual void writeHeaderData(size_t column, MyStreamUtil& ssu) const
	{
#ifdef _DEBUG
		if (column == COL_ID)
			ssu.WriteString("Id");
		else
#endif
			if (column == COL_NAME)
				ssu.WriteString("Name");
	}
	virtual void writeRowData(ITEMID item, size_t column, MyStreamUtil& ssu) const
	{
		ObjPtr pObj(toObjPtr(item));
#ifdef _DEBUG
		if (column == COL_ID)
			ssu.WriteStringf("%X", pObj->ID());
		else
#endif
			if (column == COL_NAME)
			{
				FieldPtr pField(pObj->objField());
				if (pField)
					writeFieldName(ssu, pField, (item & PTR_LSB) != 0);
				else
					writeTypeName(ssu, pObj->objTypeGlob(), (item & PTR_LSB) != 0);
			}

	}
	virtual void data(ITEMID item, size_t column, MyStreamBase& ss) const
	{
		MyStreamUtil ssu(ss);
		if (item == 0)
			writeHeaderData(column, ssu);
		else
			writeRowData(item, column, ssu);
	}
	virtual void writeFieldName(MyStreamUtil& ssu, CFieldPtr pField, bool bDup) const
	{
		MyString s;
		if (!bDup)
		{
			assert(pField->name());
			int n;
			const char* pc(NameRef_t::skipLenPfx(pField->name()->c_str(), n));
			if (n < 0)
				ProjectInfo_t::ChopName(pField->name()->c_str(), s, CHOP_SYMB);
			else
				s.assign(pc, n);
		}
		if (s.isEmpty())
			s = "<noname>";
		ssu.WriteString(s);
	}

	virtual void writeTypeName(MyStreamUtil& ssu, TypePtr pType, bool) const
	{
		assert(pType);
		ProjectInfo_t PI(mrCore.project());
		ssu.WriteString(PI.TypeName(pType, CHOP_SYMB));
	}
	virtual void path(ITEMID, MyStreamBase&) const
	{
	}
	virtual int uniqueOf(ITEMID) const
	{
		return 0;
	}
	virtual ITEMID IdFromUnique(int) const
	{
		return 0;
	}
	virtual void rename(ITEMID, const char*)
	{
	}
protected:
	ObjPtr toObjPtr(ITEMID item) const
	{
		if (item & PTR_LSB)
			item &= ~PTR_LSB;
		return (ObjPtr)item;
	}
	virtual TypePtr nameOwner(FieldPtr pField, bool /*bDuplicated*/) const
	{
		TypePtr iOwner(pField->owner());
		assert(iOwner);
		while (!iOwner->typeComplex()->namesMgr())
		{
			if (iOwner->typeSeg())
				return nullptr;
			//Bitset_t *pBitset(iOwner->typeBitset());
			//if (pBitset)
			if (!iOwner->owner())
				iOwner = iOwner->parentField()->owner();
			else
				iOwner = iOwner->owner();
			assert(iOwner);
		}
		return iOwner;
	}
	const Project_t& project() const
	{
		return mrCore.project();
	}
	const NamesMgr_t* namesMgr() const
	{
		FileNames_t* pFile(mpNFolder->fileNames());
		return pFile->namesMgr();
	}
	size_t rankImpl(NamesMgr_t* pns, PNameRef pn) const
	{
		unsigned r;
		if (pns == mpNMgr)
			r = (unsigned)rank(pn);
		else
			r = (unsigned)pns->rank(pn);
		assert(r != -1);
		return r;
	}
};


/////////////////////////////////////////////// NamesViewModelBiased_t - knows about nonamed entries (adjusts size via biasing)
class NamesViewModelBiased_t : public NamesViewModel_t
{
	size_t mBias;
public:
	NamesViewModelBiased_t(Core_t& rCore, FolderPtr pFolder)
		: NamesViewModel_t(rCore, pFolder),
		mBias(mpNMgr->bias())
	{
	}

protected:
	virtual bool empty() const {//no nonames
		if (mBias == -1)//no named entries - all are nonames
			return true;
		return NamesViewModel_t::empty();//some/all are named
	}
	virtual size_t size() const {//no nonames
		if (mBias == -1)
			return 0;
		size_t sz(NamesViewModel_t::size());
		assert(sz >= mBias);
		return sz - mBias;
	}
	virtual size_t rank(PNameRef pn) const
	{
		size_t r(mpNMgr->rank(pn));
		size_t bsz(mBias);
		assert(mBias != -1);
		return r - mBias;
	}
	virtual NamesMapIt atIt(size_t i) const
	{
		assert(mBias != -1);
		return mpNMgr->atIt(mBias + i);
	}

protected:
	virtual void reset()
	{
		NamesViewModel_t::reset();
		if (mpNMgr)
			mBias = mpNMgr->bias();
	}
};
