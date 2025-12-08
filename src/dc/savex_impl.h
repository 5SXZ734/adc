#pragma once

#include "db/save_impl.h"
#include "info_func.h"

#if(FUNC_SAVE_ENABLED)
template <typename T, typename U>//for files
class MemmoryAccessorEx_t : public MemmoryAccessor_t<T>
{
	typedef MemmoryAccessor_t<T> B;
protected:
	MemoryMgrEx_t::PathHelper mrP;
	MemoryMgrEx_t::OpHelper mrO;

	MemmoryAccessor_t<U>& mrMemAccG;

	using B::mrT;
	using B::mrF;
	using B::mrN;
	using B::mrMemMgr;
	using B::mbLocal;

public:
	MemmoryAccessorEx_t(unsigned flags, MemoryMgr_t& mm, MemmoryAccessor_t<U>& rMemAccG)
		: B(flags, mm),
		mrP(memMgrEx().mPaths.pool(), flags),
		mrO(memMgrEx().mOps.pool(), flags),
		mrMemAccG(rMemAccG)
	{
	}

	MemoryMgrEx_t& memMgrEx() {
		return reinterpret_cast<MemoryMgrEx_t&>(mrMemMgr);
	}

	virtual bool isEmpty() const {
		return B::isEmpty() && mrP.isEmpty() && mrO.isEmpty();
	}

	virtual INDEXTYPE opToIdx(OpPtr p) const override { return this->idxFromPtr(p, mrO); }
	virtual HOP opFromIdx(INDEXTYPE n) const override { return B::template idxToPtr<HOP>(mrO, n); }

	virtual INDEXTYPE pathToIdx(PathPtr p) const override { return this->idxFromPtr(p, mrP); }
	virtual PathPtr pathFromIdx(INDEXTYPE n) const override { return B::template idxToPtr<PathPtr>(mrP, n); }


	virtual void save(std::ostream& os)
	{
#if(FUNC_SAVE_ENABLED)
		B::save(os);

		B::writeIdx(os, INDEXTYPE(mrP.count()));
		B::writeIdx(os, INDEXTYPE(mrO.count()));

		int p(0);
		for (MemoryMgrEx_t::PathEltIterator i(mrP); i; i++, p++)
			B::savePath(os, i.data());
		for (MemoryMgrEx_t::OpEltIterator i(mrO); i; i++)
			B::saveOp(os, i.data());
#endif
	}

	virtual void load(std::istream& is)
	{
#if(FUNC_SAVE_ENABLED)
		mbLocal = true;

		B::load(is);
		mrT.reset(1);//request sorting
		mrF.reset(1);
		mrN.reset(1);

		B::template __allocate_chunks<Path_t>(mrP, this->readIdx(is));
		B::template __allocate_chunks<Op_t>(mrO, this->readIdx(is));

		int p(0);
		for (MemoryMgrEx_t::PathEltIterator i(mrP); i; i++, p++)
			B::loadPath(is, i.data());
		for (MemoryMgrEx_t::OpEltIterator i(mrO); i; i++)
		{
			HOP pOp(i.data());
#if(!NO_OBJ_ID)
			pOp->setId();
#endif
			B::loadOp(is, pOp, false);//order should be inherent
		}
#endif
	}

	virtual INDEXTYPE fieldRefToIdx(CFieldPtr pField) const override
	{
		bool bNegate(false);//pick a right memmgr by negating an index!
		//remember: tail labels do not have an owner!
		if (pField && pField->owner() && (DcInfo_t::IsGlobal(pField) || FuncInfo_t::IsLocalArg(pField)))//all global mlocs must have owner struc?
			bNegate = true;
		return B::idxFromPtrEx(pField, bNegate, mrF, mrMemAccG.mrF);
	}

	virtual FieldPtr fieldRefFromIdx(INDEXTYPE n) const override
	{
		return B::template idxToPtrEx<Field_t>(mrF, n, mrMemAccG.mrF);
	}

	virtual INDEXTYPE fieldTypeToIdx(CTypePtr iType) const override
	{
		//pick a right memmgr by negating index (for shared types)
		bool bNegate(!iType || iType->isShared());// || iType->typeFuncDef());
		assert(!iType || !iType->typeFuncDef());
		return B::idxFromPtrEx(iType, bNegate, mrT, mrMemAccG.mrT);
	}

	virtual TypePtr fieldTypeFromIdx(INDEXTYPE n) const override
	{
		return B::template idxToPtrEx<TypeObj_t>(mrT, n, mrMemAccG.mrT);
	}

};
#else
//typedef MemmoryAccessor_t	MemmoryAccessorEx_t;
#endif





#if(FUNC_SAVE_ENABLED)
template <typename T, typename U>//for dispersed!
class MemmoryAccessorExx_t : public MemmoryAccessorEx_t<T, U>
{
	typedef MemmoryAccessorEx_t<T, U> B;

	using B::mrT;
	using B::mDeferredTypes;
public:
	MemmoryAccessorExx_t(unsigned flags, MemoryMgrEx_t& mm, MemmoryAccessor_t<U>& rMemAccG)
		: B(flags, mm, rMemAccG)
	{
	}
protected:
	virtual INDEXTYPE fieldTypeToIdx(CTypePtr iType) const override
	{
		/*if (!mbDispersedMode)
		{
			writeIdx(os, StrucSerializer_t::fieldTypeToIdx(os, iType));
			return;
		}*/
		if (iType && (iType->isShared() || iType->typeFuncDef()))
		{
			assert(!iType->typeFuncDef());
			SINDEXTYPE u(const_cast<MemmoryAccessorExx_t*>(this)->addDifferedRef(iType));
			return (INDEXTYPE)-u;//global type refs are negated!
		}
		return (INDEXTYPE)mrT.toSqueezedIndex(iType);//local
	}

	virtual TypePtr fieldTypeFromIdx(INDEXTYPE n) const override
	{
		//if (!mbDispersedMode)
			//return StrucSerializer_t::fieldTypeFromIdx(n);

		CTypePtr pTypeRef;
		if ((long)n < 0)
		{
			n = -(long)n;
//CHECK(n == 25)
//STOP
			pTypeRef = mDeferredTypes[n];
		}
		else
			pTypeRef = mrT.fromSqueezedIndex(n);
		return (TypePtr)pTypeRef;
	}
};
#endif





template <typename T>
class MemmoryAccessorGG_t : public MemmoryAccessorG_t<T>//globs support (extended fields in SRC context)
{
	typedef MemmoryAccessorG_t<T> Base_t;
	TMemoryPool<FieldEx_t>::Helper mrG;

	INDEXTYPE mGBias;//where the extended field indices begin

public:
	template <typename... Args>
	MemmoryAccessorGG_t(unsigned flags, MemoryMgr_t& mm, Args&& ...args)
		: Base_t(flags, mm, std::forward<Args>(args)...),
		mrG(reinterpret_cast<MemoryMgr2_t&>(Base_t::mrMemMgr).mFields2.pool()),
		mGBias(0)
	{
	}

protected:
	virtual void saveExtra(std::ostream& os)
	{
		CHUNKINDEXTYPE n(CHUNKINDEXTYPE(this->mrF.count()));//how many bytes to skip (a provision for the overriders)
		uint32_t len(sizeof(n));
		write(os, len);
		writeIdx(os, n);
		mGBias = n;
	}

	virtual void loadExtra(std::istream& is)
	{
		uint32_t len;
		this->read(is, len);
		CHUNKINDEXTYPE n;//extended fields count (!)
		if (len != sizeof(n))
			throw (-SR_MALFORMAT);
		n = this->readIdx(is);
		mGBias = n;
	}

	size_t FCount() const override
	{
		assert(mGBias == this->mrF.count());
		//insure a forward compatibility (if a database, saved with ADC, is loaded by ADB)
		return mGBias + mrG.count();//WARNING:platform-dependent
	}

	virtual void loadFCount(std::istream& is)
	{
		CHUNKINDEXTYPE n(this->readIdx(is));//total
		this->__allocate_chunks<Field_t>(this->mrF, mGBias);
		if (n < mGBias)
			throw (-SR_MALFORMAT);
		this->__allocate_chunks<FieldEx_t>(mrG, n - mGBias);
	}

	virtual void saveFPool(std::ostream& os)
	{
		Base_t::saveFPool(os);
		for (TMemoryPool<FieldEx_t>::EltIterator i(mrG); i; ++i)
		{
			StrucSerializer_t::saveField(os, &(*i));
		}
	}

	virtual void loadFPool(std::istream& is)
	{
		Base_t::loadFPool(is);
		for (TMemoryPool<FieldEx_t>::EltIterator i(mrG); i; ++i)
		{
			Field_t& rField(*i);
#if(!NO_OBJ_ID)
			rField.setId();
#endif
			StrucSerializer_t::loadField(is, &rField);
		}
	}

	virtual INDEXTYPE fieldToIdx(CFieldPtr p) const override
	{
		if (!ProtoInfo_s::IsLocalArgAny(p))//local args also make use of user ptr, local vars are out-of-the-loop in given context
		{
			FieldExPtr px(DcInfo_s::AsFieldEx(p));
			if (px)
				return INDEXTYPE(mGBias + (INDEXTYPE)mrG.toSqueezedIndex(px));
		}
		return Base_t::fieldToIdx(p);
	}

	virtual FieldPtr fieldFromIdx(INDEXTYPE n) const override
	{
		if (this->mbLocal || n <= mGBias)//indices are 1-biased!
			return this->mrF.fromSqueezedIndex(n);
		return mrG.fromSqueezedIndex(n - mGBias);
	}

	virtual INDEXTYPE globIdx(CGlobPtr g)
	{
		FieldExPtr px(FieldEx_t::dockField(g));
		INDEXTYPE idx(mGBias + (INDEXTYPE)mrG.toSqueezedIndex(px));
		assert(idx);
		return idx;
	}

	virtual INDEXTYPE objToIdx(CObjPtr p) const//field or type (WARNING: high bit of index is utilized)
	{
		INDEXTYPE n;
		CGlobPtr pGlob(p->objGlob());
		if (pGlob)
			return fieldToIdx(FieldEx_t::dockField(pGlob));
		CTypePtr pType(p->objType());
		n = (INDEXTYPE)this->mrT.toSqueezedIndex(pType);
		n |= INDEXTYPE_HIBIT;//hi bit set!
		return n;
	}

	virtual TypeBasePtr objFromIdx(INDEXTYPE n) const
	{
		TypeBasePtr pObj;
		if (n != 0)
		{
			if (n & INDEXTYPE_HIBIT)
			{
				n &= ~INDEXTYPE_HIBIT;
				pObj = this->mrT.fromSqueezedIndex(n);
				assert(pObj->objType());
			}
			else
			{
				CFieldPtr pField(fieldFromIdx(n));
				//pObj = mrF.fromSqueezedIndex(n);
				assert(pField->objField());
				pObj = reinterpret_cast<CFieldExPtr>(pField)->globPtr();
			}
		}
		else
			pObj = nullptr;
		return pObj;
	}
};

