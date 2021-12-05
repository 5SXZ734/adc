#pragma once

#include "savex_impl.h"

// function contents moving between v-files
template <typename T>
class MoveAccessor_t : public T
{
	typedef T Base_t;

	GlobPtr mpGlob;
	FolderPtr mpFolder;
	MemoryMgrEx_t& mm;

	template <typename U>//remaping from old obj -> new obj
	struct remap_t : public std::vector<std::pair<U, U>>
	{
		typedef std::pair<U, U> Elt;
		struct less_than_key {
			inline bool operator() (const Elt& a, const Elt& b) { return (a.first < b.first); }
		};
		remap_t() {
			this->push_back(Elt());//1-based
		}
		void add(U a, U b) {
			this->push_back(std::make_pair(a, b));
		}
		void sort() {
			std::sort(this->begin(), this->end(), less_than_key());
		}
		size_t find(U p) const {
			typename std::vector<std::pair<U, U>>::const_iterator i(std::lower_bound(this->begin(), this->end(), Elt(p, U()), less_than_key()));
			assert(i != this->end());
			return std::distance(this->begin(), i);
		}
		U asOne(size_t i) const {
			if (this->at(i).first == this->at(i).second)
				return this->at(i).first;
			return U();
		}
	};

	remap_t<TypePtr> mT;
	remap_t<FieldPtr> mF;
	remap_t<PNameRef> mN;
	remap_t<PathPtr> mP;
	remap_t<OpPtr> mO;

public:

	MoveAccessor_t(GlobPtr pGlob, FolderPtr pFolder, MemoryMgrEx_t& _mm)
		: mpGlob(pGlob),
		mpFolder(pFolder),
		mm(_mm)
	{
		preprocess();
	}

	void blockTypes(bool bBlock){
		for (size_t i(1); i < mT.size(); i++)
		{
			TypePtr p(mT.asOne(i));
			if (p)
			{
				bBlock ? p->addRef() : p->releaseRef();
			}
		}
	}

	void addPub(TypePtr p) { mT.add(p, p); }
	void addPub(FieldPtr p) { mF.add(p, p); }
	void addPub(PNameRef p) { mN.add(p, p); }
	void add(PNameRef p) { mN.add(p, memMgrEx().NewNameRef()); }
	void add(PathPtr p) { mP.add(p, memMgrEx().NewPath()); }
#if(NO_OBJ_ID)
	void add(FieldPtr p) { mF.add(p, memMgrEx().NewFieldNoId()); }
	void add(TypePtr p) { mT.add(p, memMgrEx().NewTypeRefNoId()); }
	void add(OpPtr p) { mO.add(p, memMgrEx().NewOp()); }
#else
	void add(FieldPtr p) { mF.add(p, memMgrEx().NewFieldId(p->ID())); }
	void add(TypePtr p) { mT.add(p, memMgrEx().NewTypeRefId(p->ID())); }
	void add(OpPtr p) { mO.add(p, memMgrEx().NewOpId(p->ID())); }
#endif

	void save(std::ostream& os)
	{
		for (size_t i(1); i < mN.size(); i++)
			if (mN[i].first != mN[i].second)
				this->saveNameRef(os, mN[i].first);
		for (size_t i(1); i < mT.size(); i++)
			if (mT[i].first != mT[i].second)
				this->saveTypeObj(os, mT[i].first);
		for (size_t i(1); i < mF.size(); i++)
			if (mF[i].first != mF[i].second)
				this->saveField(os, mF[i].first);
		for (size_t i(1); i < mP.size(); i++)
			this->savePath(os, mP[i].first);
		for (size_t i(1); i < mO.size(); i++)
			this->saveOp(os, mO[i].first);
	}

	void load(std::istream& is)
	{
		for (size_t i(1); i < mN.size(); i++)
			if (mN[i].first != mN[i].second)
				this->loadNameRef(is, mN[i].second);
		for (size_t i(1); i < mT.size(); i++)
			if (mT[i].first != mT[i].second)
				this->loadTypeObj(is, mT[i].second);
		for (size_t i(1); i < mF.size(); i++)
			if (mF[i].first != mF[i].second)
				this->loadField(is, mF[i].second);
		for (size_t i(1); i < mP.size(); i++)
			this->loadPath(is, mP[i].second);
		for (size_t i(1); i < mO.size(); i++)
			this->loadOp(is, mO[i].second, true);//order could be broken
	}

protected:
	MemoryMgrEx_t& memMgrEx() { return mm; }

	// (O)
	virtual INDEXTYPE opToIdx(OpPtr p) const override {
		return (INDEXTYPE)mO.find(p);
	}
	virtual HOP opFromIdx(INDEXTYPE n) const override {
		return mO[n].second;
	}

	// (P)
	virtual INDEXTYPE pathToIdx(PathPtr p) const override {
		return (INDEXTYPE)mP.find(p);
	}
	virtual PathPtr pathFromIdx(INDEXTYPE n) const override {
		return mP[n].second;
	}

	// (F)
	virtual INDEXTYPE fieldToIdx(CFieldPtr p) const override {
		return (INDEXTYPE)mF.find((FieldPtr)p);
	}
	virtual FieldPtr fieldFromIdx(INDEXTYPE n) const override {
		return mF[n].second;
	}

	// (N)
	virtual INDEXTYPE nameToIdx(CPNameRef p) const override {
		return (INDEXTYPE)mN.find((PNameRef)p);
	}
	virtual PNameRef nameFromIdx(INDEXTYPE n) const override {
		return mN[n].second;
	}

	// (T)
	virtual INDEXTYPE typeToIdx(CTypePtr p) const override {
		return (INDEXTYPE)mT.find((TypePtr)p);
	}
	virtual TypePtr typeFromIdx(INDEXTYPE n) const override  {
		return mT[n].second;
	}

	// (2)
	virtual INDEXTYPE fieldRefToIdx(CFieldPtr p) const override {
		return fieldToIdx(p);//no need to negate
	}
	virtual FieldPtr fieldRefFromIdx(INDEXTYPE n) const override {
		return this->fieldFromIdx(n);
	}

	// (3)
	virtual INDEXTYPE fieldTypeToIdx(CTypePtr p) const override {
		return this->typeToIdx(p);
	}
	virtual TypePtr fieldTypeFromIdx(INDEXTYPE n) const override {
		return this->typeFromIdx(n);
	}

	// (4)
	//virtual INDEXTYPE objToIdx(CObjPtr) const override { assert(0); return 0; }
	//virtual TypeBasePtr objFromIdx(INDEXTYPE) const override { assert(0); return nullptr; }

private:
	void preprocess()//fill up the re-maps
	{
		CTypePtr pModule(DcInfo_s::ModuleOfEx(mpGlob));
		FuncDef_t& rFuncDef(*mpGlob->typeFuncDef());

		FieldMap& m0(rFuncDef.fields());//register args - there are can be refs from ops
		for (FieldMapIt i(m0.begin()); i != m0.end(); ++i)
		{
			FieldPtr p(VALUE(i));
			addPub(p);//all args are pub mem
			if (p->name())
				addPub(p->name());//all args names are pub mem
			if (p->type())
				addPub(p->type());//all types are pub mem
		}

		TypePtr pLocals(rFuncDef.locals());
		if (pLocals)
		{
			add(pLocals);

			FieldMap& m(pLocals->typeStruc()->fields());
			for (FieldMapIt i(m.begin()); i != m.end(); ++i)
			{
				FieldPtr p(VALUE(i));
				add(p);
				if (p->name())
					add(p->name());//all local vars names are pvt mem
				if (p->type())
					addPub(p->type());//all types are pub mem
			}
		}

		FileInfo_t GI(*DCREF(pModule), *mpFolder->fileDef());

		{//RAII-block (for dereferencing mm-indexes)
			FuncInfo_t FI(GI, mpGlob);

			for (PathTree_t::Iterator i(rFuncDef.pathTree().tree()); i; ++i)
			{
				HPATH hPath(*i);
				Path_t& rPath(*hPath);
				add(hPath);
				for (PathOpList_t::Iterator j(rPath.ops()); j; ++j)
				{
					Path_t::OpEltPtr pOp(j.data());
					add(pOp);
					HOP hOp(PRIME(pOp));
					Ins_t& ins(hOp->ins());
					for (OpList_t::Iterator k(ins.mArgs); k; ++k)//op args
						add(k.data());
				}
			}
			for (OpList_t::Iterator i(rFuncDef.mCallRets); i; ++i)
				add(i.data());
		}
		//sort the arrays
		mT.sort();
		mF.sort();
		mN.sort();
		mP.sort();
		mO.sort();
	}
};



