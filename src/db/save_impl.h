#pragma once





template <typename T>
class MemmoryAccessor_t : public T
{
	typedef T Base_t;
protected:
	MemoryMgr_t &mrMemMgr;

public:
	TMemoryPool<Field_t>::Helper mrF;
	TMemoryPool<NameRef_t>::Helper mrN;
	TMemoryPool<TypeObj_t>::Helper mrT;


	friend class StrucSerializer_t;
	friend class FileSerializer_t;

public:
	template <typename... Args>
	MemmoryAccessor_t(unsigned flags, MemoryMgr_t& mm, Args&&... args)
		: T(std::forward<Args>(args)...),
		mrMemMgr(mm),
		mrF(mrMemMgr.mFields.pool(), flags),
		mrN(mrMemMgr.mNameRefs.pool(), flags),
		mrT(mrMemMgr.mTypeRefs.pool(), flags)
	{
	}

	virtual ~MemmoryAccessor_t(){}


	virtual INDEXTYPE fieldToIdx(CFieldPtr p) const  override { return this->idxFromPtr(p, mrF); }
	virtual FieldPtr fieldFromIdx(INDEXTYPE n) const override { return this->idxToPtr<FieldPtr>(mrF, n); }

	virtual INDEXTYPE nameToIdx(CPNameRef p) const  override { return this->idxFromPtr(p, mrN); }
	virtual PNameRef nameFromIdx(INDEXTYPE n) const override { return this->idxToPtr<NameRefPtr>(mrN, n); }

	virtual INDEXTYPE typeToIdx(CTypePtr p) const { return this->idxFromPtr(p, mrT); }
	virtual TypePtr typeFromIdx(INDEXTYPE n) const  override { return this->idxToPtr<TypePtr>(mrT, n); }

	virtual bool isEmpty() const {
		return mrF.isEmpty() && mrN.isEmpty() && mrT.isEmpty();
	}

	template <typename U, typename THELPER>
	void __allocate_chunks(THELPER &a, CHUNKINDEXTYPE n) const
	{
		while (n-- > 0)
		{
			U *p(a.pool().allocate());
			new(p)U();
			//memset(p, 0, sizeof(U));//can't do that - obj ids are assigned here (to structs!)
		}
		a.reset(true);
	}

	virtual void saveExtra(std::ostream& os)
	{
		uint32_t len(0);//how many bytes to skip (provision for derivatives)
		this->write(os, len);
	}

	virtual void loadExtra(std::istream& is)
	{
		uint32_t len;
		this->read(is, len);
		if (len > 0)
		{
			is.seekg(std::ios_base::cur, len);
			if (is.eof())
				throw (-SR_MALFORMAT);
		}
	}

	virtual size_t NCount() const { return mrN.count(); }
	virtual size_t TCount() const { return mrT.count(); }
	virtual size_t FCount() const { return mrF.count(); }

	virtual void saveNPool(std::ostream& os)
	{
		for (TMemoryPool<NameRef_t>::EltIterator i(mrN); i; ++i)
			StrucSerializer_t::saveNameRef(os, &(*i));
	}

	virtual void saveTPool(std::ostream& os)
	{
		int n(0);
		for (TMemoryPool<TypeObj_t>::EltIterator i(mrT); i; ++i, n++)
		{
//CHECK(n == 324)
//STOP
			TypeObj_t& rTypeObj(*i);
			StrucSerializer_t::saveTypeObj(os, &rTypeObj);
		}
	}

	virtual void saveFPool(std::ostream& os)
	{
		for (TMemoryPool<Field_t>::EltIterator i(mrF); i; ++i)
		{
			StrucSerializer_t::saveField(os, &(*i));
		}
	}

	virtual void save(std::ostream& os)
	{
		saveExtra(os);

		this->writeIdx(os, INDEXTYPE(NCount()));
		this->writeIdx(os, INDEXTYPE(TCount()));
		this->writeIdx(os, INDEXTYPE(FCount()));

		saveNPool(os);
		saveTPool(os);
		saveFPool(os);
	}

	virtual void loadNCount(std::istream& is){ __allocate_chunks<NameRef_t>(mrN, this->readIdx(is)); }
	virtual void loadTCount(std::istream& is){ __allocate_chunks<TypeObj_t>(mrT, this->readIdx(is)); }
	virtual void loadFCount(std::istream& is){ __allocate_chunks<Field_t>(mrF, this->readIdx(is)); }

	virtual void loadNPool(std::istream& is)
	{
		int n(0);
		for (TMemoryPool<NameRef_t>::EltIterator i(mrN); i; ++i, n++)
			StrucSerializer_t::loadNameRef(is, &(*i));

		assert(mrN.count() == n);
	}

	virtual void loadTPool(std::istream& is)
	{
		int t(0);
		for (TMemoryPool<TypeObj_t>::EltIterator i(mrT); i; ++i, t++)
		{
//CHECK(t == 324)
//STOP
			TypeObj_t& rTypeObj(*i);
#if(!NO_OBJ_ID)
			rTypeObj.setId();
#endif
			StrucSerializer_t::loadTypeObj(is, &rTypeObj);
		}
	}

	virtual void loadFPool(std::istream& is)
	{
		int f(0);
		for (TMemoryPool<Field_t>::EltIterator i(mrF); i; ++i, f++)
		{
			Field_t& rField(*i);
#if(!NO_OBJ_ID)
			rField.setId();
#endif
			StrucSerializer_t::loadField(is, &rField);
		}
	}

	virtual void load(std::istream& is)
	{
		//allocate all chunks beforehand as there are references to them during the load
		loadExtra(is);

		loadNCount(is);
		loadTCount(is);
		loadFCount(is);

		loadNPool(is);
		loadTPool(is);
		loadFPool(is);
	}

	virtual INDEXTYPE fieldTypeToIdx(CTypePtr iType) const override {
		bool bNegate(!iType || iType->isShared());
		return idxFromPtrEx(iType, bNegate, mrT, mrT);
	}

	virtual TypePtr fieldTypeFromIdx(INDEXTYPE n) const override {
		return idxToPtrEx<TypeObj_t>(mrT, n, mrT);
	}

	virtual INDEXTYPE fieldRefToIdx(CFieldPtr p) const override {
		return fieldToIdx(p);
	}
	virtual FieldPtr fieldRefFromIdx(INDEXTYPE n) const override {
		return fieldFromIdx(n);
	}

	template <typename TPTR, typename THELPER>
	INDEXTYPE idxFromPtr(const TPTR p, const THELPER &a) const
	{
		return (INDEXTYPE)a.toSqueezedIndex(p);
	}

	template <typename TPTR, typename THELPER>
	TPTR idxToPtr(const THELPER &a, INDEXTYPE n) const
	{
		return a.fromSqueezedIndex(n);
	}

	template <typename U, typename THELPER>
	U* idxToPtrEx(const THELPER &a, INDEXTYPE n, const THELPER &b) const
	{
        if ((SINDEXTYPE)n < 0)
		{
			n = -(SINDEXTYPE)n;
			return b.fromSqueezedIndex(n);
		}
		return a.fromSqueezedIndex(n);
	}

	template <typename U, typename THELPER>
	INDEXTYPE idxFromPtrEx(U *p, bool bNegate, const THELPER &a, const THELPER &b) const
	{
		if (bNegate)
			return (INDEXTYPE)-(SINDEXTYPE)b.toSqueezedIndex(p);
		return (INDEXTYPE)a.toSqueezedIndex(p);
	}

};






template <typename T>
class MemmoryAccessorG_t : public MemmoryAccessor_t<T>
{
	typedef MemmoryAccessor_t<T> Base_t;
	TMemoryPool<Folder_t>::Helper mrL;
	TMemoryPool<DataObj_t>::Helper mrD;
public:
	template <typename... Args>
	MemmoryAccessorG_t(unsigned flags, MemoryMgr_t& mm, Args&& ...args)
		: Base_t(flags, mm, std::forward<Args>(args)...),
		mrL(Base_t::mrMemMgr.mFiles.pool()),
		mrD(Base_t::mrMemMgr.mDataObjs.pool())
	{
	}

	virtual bool isEmpty() const {
		return Base_t::isEmpty() && mrL.isEmpty() && mrD.isEmpty();
	}

	virtual INDEXTYPE folderToIdx(CFolderPtr p) const override { return this->idxFromPtr(p, mrL); }
	virtual FolderPtr folderFromIdx(INDEXTYPE n) const override { return this->template idxToPtr<FolderPtr>(mrL, n); }

	virtual INDEXTYPE dataToIdx(CDataPtr p) const override { return this->idxFromPtr(p, mrD); }
	virtual DataPtr dataFromIdx(INDEXTYPE n) const override { return this->template idxToPtr<DataPtr>(mrD, n); }


	virtual void save(std::ostream& os) override
	{
		Base_t::writeIdx(os, INDEXTYPE(mrD.count()));
		Base_t::writeIdx(os, INDEXTYPE(mrL.count()));//alocate file chunks here, but save later

		for (TMemoryPool<DataObj_t>::EltIterator i(mrD); i; i++)
			T::save(os, *i);

		Base_t::save(os);

		for (TMemoryPool<Folder_t>::EltIterator i(mrL); i; i++)
			T::save(os, *i);
	}

	virtual void load(std::istream& is) override
	{
		Base_t::template __allocate_chunks<DataObj_t>(mrD, this->readIdx(is));
		Base_t::template __allocate_chunks<Folder_t>(mrL, this->readIdx(is));//alocate file chunks here, but load later, so it could recover ptrs to types

		for (TMemoryPool<DataObj_t>::EltIterator i(mrD); i; i++)
			T::load(is, *i);

		Base_t::load(is);

		for (TMemoryPool<Folder_t>::EltIterator i(mrL); i; i++)
		{
			T::load(is, *i);
		}
	}
};



