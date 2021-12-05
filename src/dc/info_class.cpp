#include "info_class.h"

///////////////////////////////////////////////////
// ClassInfo_t

ClassInfo_t::ClassInfo_t(const DcInfo_t &r, TypePtr pClass)
	: DcInfo_t(r),
	mrClass(*pClass),
	mrClassPvt(*mrClass.typeClass())
{
CHECK(!mrClass.typeClass())
STOP
}

ClassInfo_t::ClassInfo_t(const DcInfo_t &r, TypePtr pClass, MemoryMgr_t &rMM)
	: DcInfo_t(r, rMM),
	mrClass(*pClass),
	mrClassPvt(*mrClass.typeClass())
{
CHECK(!mrClass.typeClass())
STOP
}

ClassMemberList_t &ClassInfo_t::MethodsOf(CTypePtr pType)
{
	assert(pType->typeClass());
	return pType->typeClass()->methods();
}

ClassVTables_t &ClassInfo_t::VTablesOf(CTypePtr pType)
{
	assert(pType->typeClass());
	return pType->typeClass()->vtables();
}

bool ClassInfo_t::IsVTable(CGlobPtr g, int& vptr_off)
{
	if (!g)
		return false;
	//CFieldPtr p(g->dockField());
	CTypePtr pClass(OwnerScope(g));
	if (pClass)
	{
		const ClassVTables_t& vtables(VTablesOf(pClass));
		for (ClassVTables_t::const_iterator i(vtables.begin()); i != vtables.end(); ++i)
		{
			const ClassVTable_t& vtable(i->second);
			if (vtable.self == g)
			{
				vptr_off = i->first;
				return true;
			}
		}
	}
	return false;
}

bool ClassInfo_t::IsMethodVirtual(CGlobPtr g)
{
	if (g->func())
	{
		if (g->flags() & FDEF_VIRTUAL)
			return true;
	}
	return false;
}

bool ClassInfo_t::RemoveClassVirtualMember(GlobPtr pGlob) const
{
	TypePtr pClass = ClassPtr();
	ClassVTables_t& vtables(VTablesOf(pClass));
	for (ClassVTables_t::iterator i(vtables.begin()); i != vtables.end(); ++i)
	{
		ClassVTable_t& vtable(i->second);
		ClassVirtMembers_t& vmemb(vtable.entries);
		for (ClassVirtMembers_t::const_iterator j(vmemb.begin()); j != vmemb.end(); ++j)
		{
			if (j->second == pGlob)
			{
//CHECKID(pField, 0xf2693)
//STOP
				vmemb.erase(j);
				pGlob->flags() &= ~FDEF_VIRTUAL;
				if (!vtable.self && vtable.entries.empty())
					vtables.erase(i);//remove empty unknown v-table
				pGlob->setOwnerScope(nullptr);
				return true;
			}
		}
	}
	return false;
}

bool ClassInfo_t::RemoveVirtualTable(GlobPtr pGlob) const
{
	TypePtr pClass = ClassPtr();
	ClassVTables_t& vtables(VTablesOf(pClass));
	for (ClassVTables_t::iterator i(vtables.begin()); i != vtables.end(); ++i)
	{
		ClassVTable_t& vtable(i->second);
		if (vtable.self == pGlob)
		{
			//unmake virtual entries (should I move them to unknown v-table if it exists?)
			while (!vtable.entries.empty())
			{
				ClassVirtMembers_t::iterator i(vtable.entries.begin());
				//FieldPtr pField(i->second);
				GlobPtr pGlob(i->second);// GlobObj(pField));
				pClass->typeClass()->addMember(pGlob);
				vtable.entries.erase(i);
				pGlob->flags() &= ~FDEF_VIRTUAL;
			}

			vtables.erase(i);
			return true;
		}
	}
	return false;
}

bool ClassInfo_t::RemoveClassMember(GlobPtr pGlob0) const//iGlob no longer attached to the field!
{
	TypePtr pClass = ClassPtr();
	PreRemoveClassMember(pGlob0);

	bool bTaken(false);
	ClassMemberList_t &l(MethodsOf(pClass));
	for (ClassMemberListIt it(l.begin()); it != l.end(); ++it)
	{
		GlobPtr pGlob(*it);
		if (pGlob == pGlob0)//i.data()
		{
			l.erase(it);
			if (l.empty())
			{
				//?AssureTypeStruc(iClass);//revert from class
				//iClass->flags() &= ~TYPEOBJ_CLASS;//no longer a class
			}
			//SetOwnerScope(pField, nullptr);
			pGlob0->setOwnerScope(nullptr);
			bTaken = true;
			break;
		}
	}

	if (!bTaken)
		return false;

#if(0)
	if (!pField->nameless())
	{
		NamesMgr_t *pNS(iClass->typeClass()->namesMgr());
		ProjectInfo_t PI(mrProject);//in global memmgr!
		PI.RemoveNameRef(*pNS, pField->name());
		pField->setName(nullptr);
		if (pField->hasU glyName())//recover a ref to a original name?
		{
			STOP
			//pField->clearUglyName();
		}
	}
#endif
	return true;
}

void ClassInfo_t::PreRemoveClassMember(GlobPtr iGlob) const
{
//CHECKID(pField, 11339)
//STOP
	TypePtr pClass = ClassPtr();
	assert(pClass == iGlob->ownerScope1());
	//TypePtr iClass(OwnerScope(pField));
	//if (!iClass)
		//return false;

	NamesMgr_t *pNS(mrClassPvt.namesMgr());

	if (iGlob->hasPrettyName())
	{
		PNameRef pn(TakePrettyName(iGlob));//may have a pretty name
		if (pn)
		{
			if (!pNS->removen(pn))
				ASSERT0;
			memMgrGlob().Delete(pn);
		}
		assert(!DcInfo_s::DockName(iGlob) || !pNS->contains(DcInfo_s::DockName(iGlob)));//must be in globals NS
	}
	else
	{
		FieldPtr pField(DockField(iGlob));
		if (pField->name())//no pretty name (but scoped)
		{
			if (!pField->hasUglyName())//ugly names are always in global NS
			{
				//assert(pNS->contains(pField->name()));
				ClearFieldName(pField, nullptr);//transition from local scope to global one - name is not expected to be appropriate(?)
			}
			else
			{
				assert(!pNS->contains(pField->name()));
			}
		}
	}

	if (iGlob->func())
	{
		GlobPtr iFuncDef(iGlob);//GlobFuncObj(pField));
		PNameRef pn(iFuncDef->name());
		if (pn)
		{
			assert(0);
			if (!pNS->removen(pn))
				ASSERT0;
			memMgrGlob().Delete(pn);
			iFuncDef->setName(nullptr);
		}
//?		iFuncDef->typeFuncDef()->setThisPtr(nullptr);
	}
}

bool ClassInfo_t::ClearClassVTable(ClassVTable_t &vtable) const
{
	TypePtr pClass = ClassPtr();
	while (!vtable.entries.empty())
	{
		ClassVirtMembers_t::iterator i(vtable.entries.begin());
		//FieldPtr pField(i->second);
		GlobPtr pGlob(i->second);// GlobObj(pField));
		PreRemoveClassMember(pGlob);
		pGlob->setOwnerScope(nullptr);
		vtable.entries.erase(i);
		pGlob->flags() &= ~FDEF_VIRTUAL;
	}
	GlobPtr pSelf(vtable.self);
	if (pSelf)
		PreRemoveClassMember(pSelf);
	return true;
}

bool ClassInfo_t::ClearMemberList() const
{
	TypePtr pSelf(ClassPtr());
	if (!pSelf->hasUserData())
		return false;//?

CHECKID(pSelf, -132)
STOP

	MemoryMgrEx_t &rMemMgrEx(memMgrGlobEx());
	ClassMemberList_t &l(MethodsOf(pSelf));
	while (!l.empty())
	{
		ClassMemberListIt it(l.begin());
		GlobPtr pGlob(*it);
		//FieldPtr pField(pGlob->dockField());
		//assert(pField);
//CHECKID(pField, 0x922)
//STOP
		if (!pGlob->func())//pField->isStaticMember())
		{
			if (!RemoveClassMember(pGlob))
				ASSERT0;
		}
		else
		{
			assert(pGlob->typeFuncDef());
CHECKID(pGlob, 0x1e785)
STOP
			if (!UnmakeMemberMethod(pGlob))
				break;
		}
		//rMemMgrEx.Delete(Methods Of(pSelf).take(i));
	}

	ClassVTables_t& vtables(VTablesOf(pSelf));
	while (!vtables.empty())
	{
		ClassVTables_t::iterator i(vtables.begin());
		ClassVTable_t& vtable(i->second);
		if (!ClearClassVTable(vtable))
			ASSERT0;
		if (vtable.self)
		{
			GlobPtr pGlob(vtable.self);
			pGlob->setOwnerScope(nullptr);
		}
		vtables.erase(i);
	}

	return false;
}

bool ClassInfo_t::MakeClassMemberOf(GlobPtr pSelf) const
{
	TypePtr iClass(ClassPtr());
	assert(pSelf);

//CHECKID(iFuncDef, 0x1c8c)
//STOP

	TypePtr iClassOld(OwnerScope(pSelf));
	if (iClassOld && iClassOld != iClass)
	{
		PrintError() << "failed to make '" << GlobNameFull(pSelf, E_PRETTY, CHOP_SYMB)
			<< "' a member of class " << TypePrettyNameFull(iClass, CHOP_SYMB)
			<< ". Already a member of " << TypePrettyNameFull(iClassOld, CHOP_SYMB) << std::endl;
		return false;
	}

	if (!AssureTypeClass(iClass, false))//enforce - must be not a namespace
		return false;

	return AddClassMember(pSelf);
}

bool ClassInfo_t::AddClassMember(GlobPtr pGlob) const
{
	FieldPtr pField(DockField(pGlob));
	TypePtr iClass(ClassPtr());
//CHECKID(ClassPtr(),	0x1c2b)
//CHECKID(pField, 7212)
CHECK(!iClass->typeClass())
STOP

	assert(iClass->typeClass());//or namespace!
	//assert(!pField->isCloneMaster());
	assert(ModuleOf(iClass) == ModuleOfEx(pGlob));

	TypePtr iClassOld(OwnerScope(pGlob));//already in scope?
	if (iClassOld)
	{
		if (iClassOld == iClass)
		{
			assert(HasMember(iClass, pGlob));
			return true;//reference of this object already registered!
		}
		//RemoveClassMember(pField);
//?		assert(0);//should be unmembered first
		return false;
	}

	bool bWasEmpty(!HasMethods(iClass));
	if (!iClass->typeClass()->addMember(pGlob))
		return false;

	if (bWasEmpty)
	{
		if (pField->isExported())
			iClass->setExporting(true);
		else
		{
			assert(!iClass->isExporting());
		}
	}
	else
	{
		if (iClass->isExporting())
		{
			if (!pField->isExported())
				iClass->setExporting(false);
		}
	}

	SetOwnerScope(pField, iClass);

	if (pField->name())
		pField->setUglyName();//the global name stays in old scope, not migrated into new scope - must mark it

	return true;
}

bool ClassInfo_t::AddClassVirtualMember(GlobPtr pGlob) const
{
	FieldPtr pField(DockField(pGlob));
//CHECKID(pField, 11339)
//STOP

	TypePtr iClass(ClassPtr());
/*?	if (!iClass->typeClass())//namespaces allowed - it s a static member
		if (!AssureTypeNamespace(iClass, false))
			if (!AssureTypeClass(iClass))
				return false;*/

	assert(iClass->typeClass());//or namespace!
	//assert(!pField->isCloneMaster());
	assert(ModuleOf(iClass) == ModuleOfEx(pGlob));

	TypePtr iClassOld(OwnerScope(pGlob));//already in scope?
	if (iClassOld)
	{
		if (iClassOld == iClass)
		{
			assert(HasMember(iClass, pGlob));
			if (IsVirtMemberMethod(pGlob))
				return true;//reference of this object already registered!
			return MakeMethodVirtual(pGlob);
		}
		//RemoveClassMember(pField);
		assert(0);//should be unmembered first
		return false;
	}

	bool bWasEmpty(!HasMethods(iClass));
	if (!RegisterVirtualMember(pGlob))
		return false;
	//if (!iClass->typeClass()->addMember(pField))
	//	return false;

	if (bWasEmpty)
	{
		if (pField->isExported())
			iClass->setExporting(true);
		else
		{
			assert(!iClass->isExporting());
		}
	}
	else
	{
		if (iClass->isExporting())
		{
			if (!pField->isExported())
				iClass->setExporting(false);
		}
	}

	SetOwnerScope(pField, iClass);

	if (pField->name())
		pField->setUglyName();//the global name stays in old scope, not migrated into new scope - must mark it

	return true;
}

bool ClassInfo_t::HasMember(TypePtr iClass, CGlobPtr pGlob)
{
#ifdef _DEBUG
	if (HasMethods(iClass))
	{
		const ClassMemberList_t& l(MethodsOf(iClass));
		for (ClassMemberListCIt i(l.begin()); i != l.end(); ++i)
			if (*i == pGlob)
			{
				assert(pGlob->ownerScope() == iClass);
				return true;
			}
	}

	const ClassVTables_t& vtables(VTablesOf(iClass));
	for (ClassVTables_t::const_iterator i(vtables.begin()); i != vtables.end(); ++i)
	{
		const ClassVTable_t& vtable(i->second);
		if (vtable.self == pGlob)
		{
			assert(pGlob->ownerScope() == iClass);
			return true;
		}
		const ClassVirtMembers_t& vmemb(vtable.entries);
		for (ClassVirtMembers_t::const_iterator j(vmemb.begin()); j != vmemb.end(); ++j)
		{
			if (j->second == pGlob)
			{
				assert(pGlob->ownerScope() == iClass);
				return true;
			}
		}
	}
	return false;
#else
	if (!iClass->typeClass())
		return false;
	return pGlob->ownerScope() == iClass;
#endif
}

int ClassInfo_t::MoveMethodsFrom(TypePtr pSelf, const TypePtr pSrc)
{
	//assert(pSelf.isValid());
	//assert(pSrc.isValid());
	//assert(IS _PTR(pSrc) && pSrc->isValid());

	assert(pSrc != pSelf);
	//	assert(m_pXRefs);

		//redirect ops ptrs
	while (HasMethods(pSrc))
	{
		//Obj_t * pObj = MethodsOf(pSrc).head()->data();
		//assert(pObj->type() == pSrc);

		//?pSrc->Releas eObjRef(pObj);
		assert(0);//?		pObj->setT ype( this );
		//?AddO bjRef(pObj);
	}

	//assert(rSelf.isValid());
	return 1;
}

bool ClassInfo_t::IsVirtMemberMethod(CGlobPtr pGlob)
{
	CTypePtr iClass(pGlob->ownerScope());
	if (!iClass)
		return false;
	assert(iClass->typeClass());
	//find in non-virtual member list
	const ClassMemberList_t& l(MethodsOf(iClass));
	for (ClassMemberListCIt i(l.begin()); i != l.end(); ++i)
	{
		if (*i == pGlob)
			return false;
	}
	//assume it is in virtual list
	return true;
}

ClassVTable_t& ClassInfo_t::AssureUnknownVTable(TypePtr iClass)
{
	assert(iClass->typeClass());
	ClassVTables_t& vtables(VTablesOf(iClass));
	for (ClassVTables_t::iterator i(vtables.begin()); i != vtables.end(); ++i)
	{
		ClassVTable_t& vtable(i->second);
		if (!vtable.self)
		{
			assert(i->first == UNK_VTABLE_OFF);
			return vtable;
		}
	}
	return iClass->typeClass()->addVTable(nullptr, UNK_VTABLE_OFF);
}

bool ClassInfo_t::MakeMethodVirtual(GlobPtr pGlob) const
{
//CHECKID(pField, 11339)
//STOP
	/*TypePtr iClass(OwnerScope(pField));
	if (!iClass)
		return false;*/
	TypePtr iClass(ClassPtr());
	assert(iClass->typeClass());

	//find in non-virtual list and relocate
	ClassMemberList_t& l(MethodsOf(iClass));
	for (ClassMemberListIt i(l.begin()); i != l.end(); ++i)
	{
		if (*i == pGlob)
		{
			l.erase(i);
			RegisterVirtualMember(pGlob);
			return true;
		}
	}
	return false;
}

bool ClassInfo_t::UnMakeMethodVirtual(GlobPtr pGlob) const
{
	if (RemoveClassVirtualMember(pGlob))
	{
		pGlob->setOwnerScope(ClassPtr());
		if (mrClassPvt.addMember(pGlob))
			return true;
	}
	return false;
}

bool ClassInfo_t::MakeMemberVTable(GlobPtr pGlob) const
{
/*	TypePtr iClass(OwnerScope(pField));
	if (!iClass)
		return false;*/
	TypePtr iClass(ClassPtr());
	assert(iClass->typeClass());

	//find in non-virtual list and relocate
	ClassMemberList_t& l(MethodsOf(iClass));
	for (ClassMemberListIt i(l.begin()); i != l.end(); ++i)
	{
		if (*i == pGlob)
		{
			l.erase(i);
			ClassVTable_t& vtable(AddVirtualTable(pGlob, UNASSOC_VTABLE_OFF));
			return true;
		}
	}
	return false;
}

bool ClassInfo_t::RegisterVirtualMember(GlobPtr pGlob) const
{
//CHECKID(pField, 2679)
//STOP
	TypePtr iClass(ClassPtr());
	assert(pGlob);
	assert(iClass->typeClass());
	assert(!iClass->typeNamespace());
	if (!IsPhantomModule(ModulePtr()))
	{
		//try to find the reference in vtables of the class
		ClassVTables_t& vtables(VTablesOf(ClassPtr()));
		for (ClassVTables_t::iterator i(vtables.begin()); i != vtables.end(); ++i)
		{
			ClassVTable_t& vtable(i->second);
			//find a field in vtable, matching a given class member
			GlobPtr pVField(MethodToVTableField(pGlob, vtable));
			if (pVField)
			{
				iClass->typeClass()->addVirtMember(pGlob, DockAddr(pGlob), vtable);
				return true;
			}
		}
	}

	//add a new entry to the unknown v-table
	ClassVTable_t& vtable(AssureUnknownVTable(iClass));
	iClass->typeClass()->addVirtMember(pGlob, UNK_VENTRY_OFF, vtable);
	return true;
}

class StrucFieldRawIterator : public ModuleInfo_t
{
	FieldPtr mpSelf;
	TypePtr mpVTable;
	TypePtr mpSeg;
	DataSubSource_t mData;
	ADDR mOffBase;
	FieldMap& mrFields;
	FieldMapIt mIt;
public:
	StrucFieldRawIterator(const ModuleInfo_t &r, FieldPtr pSelf)
		: ModuleInfo_t(r),
		mpSelf(pSelf),
		mpVTable(mpSelf->type()),
		//assert(mpVtable->typeStruc())
		mpSeg(OwnerSeg(mpSelf->owner())),
		//?assert(CheckDataAtVA(pSeg, vtable.self->address()));
		mData(GetDataSource()->pvt(), mpSeg->typeSeg()->rawBlock()),
		mOffBase(mpSelf->offset()),
		mrFields(mpVTable->typeStruc()->fields()),
		mIt(mrFields.begin())
	{
	}
	operator bool() const {
		return mIt != mrFields.end();
	}
	void operator++(){
		++mIt;
	}
	FieldPtr field() const {
		return (FieldPtr)VALUE(mIt);
	}
	VALUE_t& value(VALUE_t& v)
	{
		CFieldPtr pVTableField(field());
		DataStream_t aRaw(mData, mOffBase + pVTableField->offset());
		v.typ = (uint8_t)aRaw.read(pVTableField->size(), (PDATA)&v);
		return v;
	}
	TypePtr seg() const { return mpSeg; }
};

GlobPtr ClassInfo_t::MethodToVTableField(GlobPtr pGlob, ClassVTable_t& vtable) const
{
	if (!vtable.self)
		return nullptr;//unknown v-table: skip
	if (!DockField(vtable.self)->isTypeStruc())
		return nullptr;

	//iterate through v-table's fields looking for pointers to functions
	StrucFieldRawIterator iter(*this, DockField(vtable.self));
	for (; iter; ++iter)
	{
		CFieldPtr pVTableField(iter.field());//structure field
		if (!pVTableField->isTypePtr())
			continue;
		VALUE_t v;
		iter.value(v);
		CFieldPtr pTargetField(GetFieldFromValue(v, iter.seg()));//global
		if (!pTargetField)
			continue;
		/*if (pTargetField->isCloneMaster())
		{
			ADDR va(pTargetField->_key());
			CTypePtr pSeg2(ProjectInfo_t::OwnerSeg(pTargetField->owner()));
			const ConflictFieldMap& m(pSeg2->typeSeg()->conflictFields());
			for (ClonedFieldMapCIt k(m.lower_bound(va)); k != m.end() && KEY(k) == va; ++k)
			{
				CFieldPtr pCloneField(VALUE(k));
				if (GlobObj(pCloneField) == pGlob)
					return GlobObj(pVTableField);
			}
		}
		else*/ if (GlobObj(pTargetField) == pGlob)
		{
			return GlobObj(pVTableField);
		}
	}
	return nullptr;
}

ClassVTable_t& ClassInfo_t::AddVirtualTable(GlobPtr pSelf, int voffs) const
{
	TypePtr iClass(ClassPtr());
	assert(pSelf);
	assert(iClass->typeClass());
	assert(!iClass->typeNamespace());
	ClassVTable_t& vtable(iClass->typeClass()->addVTable(pSelf, voffs));

	//populate with virtual methods
	if (DockField(pSelf)->isTypeStruc())
	{
		//iterate through all entries in v-table looking for addresses, which are to be mapped into virtual class members
		StrucFieldRawIterator iter(*this, DockField(vtable.self));
		for (; iter; ++iter)
		{
			CFieldPtr pSField(iter.field());//structure field
			if (!pSField->isTypePtr())
				continue;
			VALUE_t v;
			iter.value(v);
			CFieldPtr pGField(GetFieldFromValue(v, iter.seg()));//global
			if (!pGField)
				continue;
			if (OwnerScope(pGField) != iClass)//relocate only those who are in given class
				continue;

			//look for the method already in another v-table (unknown one?) to reloacate
			bool bTaken(false);
			ClassVTables_t& vtables(VTablesOf(iClass));
			for (ClassVTables_t::iterator i(vtables.begin()); i != vtables.end(); ++i)
			{
				ClassVTable_t& vtable2(i->second);
				if (&vtable2 == &vtable)
					continue;
				ClassVirtMembers_t& vmemb(vtable2.entries);
				for (ClassVirtMembers_t::const_iterator k(vmemb.begin()); k != vmemb.end(); ++k)
				{
					if (k->second == pSelf)//?
					{
						vmemb.erase(k);
						bTaken = true;
						break;
					}
				}
				if (bTaken)
				{
					if (!vtable2.self && vtable2.entries.empty())
						vtables.erase(i);//remove empty unknown v-table
					break;
				}
			}

			if (!bTaken)//search among non-virtuals
			{
				ClassMemberList_t& l(MethodsOf(iClass));
				for (ClassMemberListIt k(l.begin()); k != l.end(); ++k)
				{
					if (*k == pSelf)//?
					{
						l.erase(k);
						bTaken = true;
						break;
					}
				}
			}

			if (bTaken)
				iClass->typeClass()->addVirtMember(pSelf, pSField->offset(), vtable);
		}
	}

	return vtable;
}

static GlobPtr takeGlobByVA(ADDR va, std::multimap<ADDR, GlobPtr>& m)
{
	std::multimap<ADDR, GlobPtr>::iterator i(m.lower_bound(va));
	if (i != m.end() && i->first == va)
	{
		GlobPtr p(i->second);
		m.erase(i);
		return p;
	}
	return nullptr;
}

GlobPtr ClassInfo_t::TakeMethodByVA(ADDR va) const
{
	ClassMemberList_t& m(MethodsOf(ClassPtr()));
	for (ClassMemberListIt i(m.begin()); i != m.end(); ++i)
	{
		GlobPtr pGlob(*i);
		if (DockAddr(pGlob) == va && pGlob->func())//methods only
		{
			m.erase(i);
			return pGlob;
		}
	}
	return nullptr;
}

void ClassInfo_t::AnalyzeClass() const
{
	TypePtr pClass(ClassPtr());
	assert(pClass->typeClass());

	//collect all vptrs offsets
	std::vector<ADDR> vptrsOffsets;
	//go deep into inherited classes to build vptrs offsets table
	CollectVPtrsOffsets(pClass, 0, vptrsOffsets);

	std::vector<GlobPtr> m;
	std::multimap<ADDR, GlobPtr> l;

	//take out all v-tables (discard the unknown one, if exists)
	ClassVTables_t& vtables(VTablesOf(pClass));
	while (!vtables.empty())//ClassVTables_t::iterator i(vtables.begin()); i != vtables.end(); ++i)
	{
		ClassVTable_t& vtable(vtables.begin()->second);
		//take out all virtual entries from v-tables
		ClassVirtMembers_t& vmemb(vtable.entries);
		while (!vmemb.empty())
		{
			GlobPtr pGlob(vmemb.begin()->second);
			vmemb.erase(vmemb.begin());
			l.insert(std::make_pair(DockAddr(pGlob), pGlob));
		}
		if (vtable.self)
			m.push_back(vtable.self);
		vtables.erase(vtables.begin());
	}

	//re-introduce v-tables back to the class, associate each with existing vptr (according to order)
	for (size_t i(0); i < m.size(); i++)
	{
		int voffs(i < vptrsOffsets.size() ? vptrsOffsets[i] : UNASSOC_VTABLE_OFF);//assure it goes before the unknow one
		mrClassPvt.addVTable(m[i], voffs);
	}

	//repopulate v-tables
	for (ClassVTables_t::iterator i(vtables.begin()); i != vtables.end(); ++i)
	{
		ClassVTable_t& vtable(i->second);
		assert(vtable.self);//no the unknown vtable expected
		StrucFieldRawIterator iter(*this, DockField(vtable.self));
		for (; iter; ++iter)
		{
			CFieldPtr ps(iter.field());//structure field
			if (!ps->isTypePtr())
				continue;
			VALUE_t v;
			iter.value(v);//a pointer
			ADDR va;
			if (!GetVAfromValue(v, iter.seg(), va))
				continue;
			GlobPtr pg(takeGlobByVA(va, l));//first try an existing virtual method
			if (!pg)
				pg = TakeMethodByVA(va);//then look among non-virtuals (for new entries)
			if (pg)
				mrClassPvt.addVirtMember(pg, ps->offset(), vtable);
		}
	}

	//now, if there are remaining former v-methods, re-introduce them into the unknown v-table
	while (!l.empty())
	{
		GlobPtr pGlob(l.begin()->second);
		l.erase(l.begin());
		//RegisterVirtualMember(pField);
		ClassVTable_t& vtable(AssureUnknownVTable(pClass));
		mrClassPvt.addVirtMember(pGlob, UNK_VENTRY_OFF, vtable);
	}
}

ClassVTable_t* ClassInfo_t::OwnerVTable(CGlobPtr pSelf)
{
	CTypePtr pClass(OwnerScope(pSelf));
	assert(pSelf && pClass);
	ClassVTables_t& vtables(VTablesOf(pClass));
	for (ClassVTables_t::iterator i(vtables.begin()); i != vtables.end(); ++i)
	{
		ClassVTable_t& vtable(i->second);
		const ClassVirtMembers_t& vmemb(vtable.entries);
		for (ClassVirtMembers_t::const_iterator j(vmemb.begin()); j != vmemb.end(); ++j)
			if (j->second == pSelf)
				return &vtable;
	}
#if(0)
	//such among non-virtual entries
	ClassMemberList_t &l(MethodsOf(pClass));
	for (ClassMemberListIt i(l.begin()); i != l.end(); ++i)
		if (*i == pSelf)
			return nullptr;
#endif
	return nullptr;
}

bool ClassInfo_t::IsMethodPseudoVirtual(CGlobPtr pSelf)
{
	ClassVTable_t* pVTable(OwnerVTable(pSelf));
	assert(pVTable);
	return pVTable->self == nullptr;
}









