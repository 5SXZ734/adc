#pragma once

#include "info_dc.h"

/////////////////////////////////////////////////////DcInfo_t
class ClassInfo_t : public DcInfo_t
{
protected:
	TypeObj_t& mrClass;
	TypeClass_t& mrClassPvt;

public:
	ClassInfo_t(const DcInfo_t&, TypePtr);
	ClassInfo_t(const DcInfo_t&, TypePtr, MemoryMgr_t&);

	TypePtr ClassPtr() const { return &mrClass; }
	bool RemoveClassVirtualMember(GlobPtr) const;
	bool RemoveVirtualTable(GlobPtr) const;
	bool RemoveClassMember(GlobPtr) const;
	void PreRemoveClassMember(GlobPtr) const;
	bool ClearClassVTable(ClassVTable_t&) const;
	bool ClearMemberList() const;
	bool MakeClassMemberOf(GlobPtr) const;//fails if allready
	bool AddClassMember(GlobPtr) const;
	bool AddClassVirtualMember(GlobPtr) const;
	bool MakeMethodVirtual(GlobPtr) const;
	bool UnMakeMethodVirtual(GlobPtr) const;
	bool MakeMemberVTable(GlobPtr) const;
	bool RegisterVirtualMember(GlobPtr) const;
	ClassVTable_t &AddVirtualTable(GlobPtr, int voffs) const;
	GlobPtr MethodToVTableField(GlobPtr, ClassVTable_t& vtable) const;
	GlobPtr TakeMethodByVA(ADDR) const;
	void AnalyzeClass() const;

	static ClassMemberList_t &MethodsOf(CTypePtr);
	static ClassVTables_t &VTablesOf(CTypePtr);
	static bool	HasMember(TypePtr, CGlobPtr);
	static int	MoveMethodsFrom(TypePtr, const TypePtr);
	static bool IsVirtMemberMethod(CGlobPtr);//O(n')
	static ClassVTable_t& AssureUnknownVTable(TypePtr);
	static bool IsVTable(CGlobPtr, int& vptr_off);//O(m), m - num of vtables
	static bool IsMethodVirtual(CGlobPtr);//O(1)
	static ClassVTable_t* OwnerVTable(CGlobPtr);//O(n)
	static bool IsMethodPseudoVirtual(CGlobPtr);//O(n)
};



class ClassHierarchyInterim_t : public I_Front::IDumpClassHierarchy,
	public DcInfo_t
{
	struct class_info_t
	{
		OFF_t name;
		MyString fullName;
		TypePtr	iClass;
		//int mdisp, pdisp, vdisp;
		//unsigned attributes;
		unsigned offset;
		bool bVirtual;

		std::list<class_info_t*>	v;
		class_info_t(OFF_t n, TypePtr _p) : name(n), iClass(_p), offset(0), bVirtual(false)/*mdisp(0), pdisp(-1), vdisp(0), attributes(0)*/{}
		class_info_t(OFF_t n, TypePtr _p, unsigned _o/*int md, int pd, int vd, unsigned att*/, bool virt) : name(n), iClass(_p), offset(_o), bVirtual(virt)/*mdisp(md), pdisp(pd), vdisp(vd), attributes(att)*/{}
		~class_info_t()
		{
			for (std::list<class_info_t*>::iterator i(v.begin()); i != v.end(); i++)
				delete (*i);
		}
		class_info_t *add(class_info_t *_p)
		{
			v.push_back(_p);
			return _p;
		}
		void print(std::ostream &os, int level, const I_DataSource &aRaw) const
		{
			assert(iClass && !fullName.empty());
			for (int j(0); j < level; j++)
				os << '\t';
			os << fullName;
			if (!aRaw.isNull(name))
			{
				std::ostringstream sName;
				DataStream_t aName(aRaw, name);
				aName.fetchString(sName);
				os << " (" << sName.str() << "," << offset/*mdisp*/ << "," /*<< pdisp << "," << vdisp << ":" << attributes*/ << bVirtual << ")";
			}
			os << std::endl;
			for (std::list<class_info_t*>::const_iterator i(v.begin()); i != v.end(); i++)
				(*i)->print(os, level + 1, aRaw);
		}
		bool operator==(const class_info_t &o) const
		{
			if (name == o.name && offset == o.offset/*mdisp == o.mdisp && pdisp == o.pdisp && vdisp == o.vdisp*/ && bVirtual == o.bVirtual)
			{
				//assert(attributes == o.attributes);
				return true;
			}
			return false;
		}
		bool contains(const class_info_t &o) const
		{
			for (std::list<class_info_t*>::const_iterator i(v.begin()); i != v.end(); i++)
			{
				class_info_t *p(*i);
				if (*p == o)
					return true;
				if (p->contains(o))
					return true;
			}
			return false;
		}
	};
	std::vector<class_info_t	*>	mStack;

public:
	ClassHierarchyInterim_t(Dc_t &rDC, TypePtr pSelf)
		: DcInfo_t(rDC)
	{
		mStack.push_back(new class_info_t(OFF_NULL, pSelf));
		mStack.back()->fullName = TypeNameFull(pSelf, E_PRETTY, CHOP_SYMB).join();
	}
	~ClassHierarchyInterim_t()
	{
		//orphans need to be deleted (may happen in case of some error)
		for (size_t i(mStack.size() - 1); i > 0; i--)
		{
			class_info_t *p(mStack.at(i));
			class_info_t *parent(mStack.at(i - 1));
			if (parent->v.empty() || parent->v.back() != p)
				delete p;
		}
		delete mStack.front();//root only
	}
protected:
	virtual bool add(int l, OFF_t oName, ADDR vaBCD, unsigned offset/*int mdisp, int pdisp, int vdisp, unsigned attributes*/, bool bVirtual)
	{
#if(0)
		class_info_t ci(oName, nullptr, mdisp, pdisp, vdisp, attributes);
		ci.print(std::cout, l);
#else
		size_t level(l + 1);
		assert(level <= mStack.size());
		if (level < mStack.size())
		{
			do {
				class_info_t *p(mStack.back());
				mStack.pop_back();
				if (mStack.back()->v.empty() || mStack.back()->v.back() != p)//already contained in some other class?
					delete p;
			} while (level < mStack.size());
		}

		Locus_t aLoc;
		FindFieldInSubsegs(PrimeSeg(), vaBCD, aLoc);
		if (!aLoc.stripToUserField(nullptr))
			return false;
		CGlobPtr pGlob(GlobObj(aLoc.field0()));
		if (!pGlob)
			return false;
		TypePtr iBaseClass(OwnerScope(pGlob));
		if (!iBaseClass)
			return false;
		mStack.push_back(add(new class_info_t(oName, iBaseClass, offset, bVirtual/*mdisp, pdisp, vdisp, attributes*/)));
		mStack.back()->fullName = TypeNameFull(iBaseClass, E_PRETTY, CHOP_SYMB).join();
#endif
		return true;
	}
	class_info_t *add(class_info_t *pNew)
	{
		if (!mStack.front()->contains(*pNew))
			mStack.back()->add(pNew);//do not add duplicates
		return pNew;
	}
	bool reconstruct_NV(class_info_t *p)
	{
		for (std::list<class_info_t*>::const_iterator i(p->v.begin()); i != p->v.end(); i++)
		{
			class_info_t *p2(*i);
			if (p2->bVirtual)
				continue;
			if (!reconstruct_NV(p2))
				return false;
			Locus_t aLoc;
			aLoc.add(p->iClass, p2->offset/*p2->mdisp*/, nullptr);
			if (!InsertField(aLoc))
				continue;//break;
			if (p2->iClass->typeNamespace())
				AssureTypeClass(p2->iClass);
			if (!SetTypeEx(aLoc.field0(), p2->iClass))
				continue;
				//return false;
			AddInheritance(aLoc.struc(), aLoc.field0());
		}
		return true;
	}
	bool reconstruct_V(class_info_t *p)
	{
		class_info_t* p0(mStack.front());
		for (std::list<class_info_t*>::const_iterator i(p->v.begin()); i != p->v.end(); i++)
		{
			class_info_t *p2(*i);
			if (p2->bVirtual)
			{
				Locus_t aLoc;
				aLoc.add(p0->iClass, p2->offset, nullptr);
				terminalFieldAt(aLoc);
				if (aLoc.struc() != p0->iClass)//known offset of v-base in starter class
					if (aLoc.struc() != p->iClass || aLoc.addr() != 0)//check if falling into a super class, right at the boundary, otherwise - no way to tell where a v-base resides in super class
						continue;
				if (!InsertField(aLoc))
					continue;//break;
				if (p2->iClass->typeNamespace())
					AssureTypeClass(p2->iClass);
				if (!SetTypeEx(aLoc.field0(), p2->iClass))
					continue;
			}
			if (!reconstruct_V(p2))
				return false;
		}
		return true;
	}
public:
	void print(std::ostream &os)
	{
		mStack.front()->print(os, 0, GetDataSource()->pvt());
	}
	bool reconstruct()
	{
		//non-virtual bases first
		if (!reconstruct_NV(mStack.front()))
			return false;
		//all virtual bases are applied to most-derived class
		return reconstruct_V(mStack.front());
	}
};



