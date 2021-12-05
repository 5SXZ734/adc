#include "type_strucvar.h"
#include "types_mgr.h"
#include "main.h"


/////////////////////////////////////////
// Strucvar_t

std::map<ADDR, Type_t *> gCustTypes;

/*void Strucvar_t::dump( DisplayCache_t &D )
{
//	Type_t * pType = create(D);
//	pType->dump(D);
//	delete pType;
}*/

/*int Strucvar_t::size()
{
Type_t * pType = create( maddr );
return pType->size();
}*/

Strucvar_t::~Strucvar_t()
{
	//if (mpDevice)
	if (mpBitfield0)
	{
		mpBitfield0->type()->ClearPvt();
		mpBitfield0->type()->setParentField(nullptr);
		delete mpBitfield0->type();
		mpBitfield0->setType0(nullptr);
		delete mpBitfield0;
		mpBitfield0 = nullptr;
	}
	assert(mFields.empty()); 
}

//int Strucvar_t::ID() const { return type obj()->ID(); }

Type_t * Strucvar_t::custom(ADDR va)
{
	Type_t * pType = nullptr;
	std::map<ADDR, Type_t *>::iterator it = gCustTypes.find(va);
	if (it != gCustTypes.end())
	{
		pType = it->second;
	}
	else
	{
		assert(false);
		//DCACHE.setOffs(va);
		//?		pType = create( va );
		std::pair<std::map<ADDR, Type_t *>::iterator, bool> it = gCustTypes.insert(std::pair<ADDR, Type_t *>(va, pType));
	}
	return pType;
}

/*void Strucvar_t::aka(int id, MakeAlias &buf) const
{
	buf.forTypeStrucvar(id);
}*/

/*bool Strucvar_t::terminalFieldAt(ROWID &a, Locus_t &, Block_t)
{
	//?assert(0);
	return false;
}*/

I_DynamicType *Strucvar_t::device() 
{
	if (!mpDevice)
	{
		PNameRef pName(name());
		if (pName)
			mpDevice = MAIN.getContextDependentType(pName->c_str());
	}
	return mpDevice;
}

void Strucvar_t::releaseDevice()
{
	if (mpDevice)
	{
		MAIN.releaseFrontend(name()->c_str());
		mpDevice = nullptr;
	}
}

/*int Strucvar_t::querySize(PDATA pData)
{
#if(1)
	SizeQuery_t q(*this, pData);
	device()->create(q, -1);
	return -1;
#else
	return device()->size(pData);
#endif
}*/

FieldPtr Strucvar_t::findFieldByName(const char *name)
{
	NamesMgr_t *pNs(namesMgr());
	if (pNs->isInitialized())
	{
		//pNs->NamespaceInitialized(this);
		Obj_t *pObj(pNs->findObj(name));
		if (pObj)
			return pObj->objField();
	}
	return nullptr;
}


//////////////////////////


/*PSTRUC Strucvar_t::SizeQuery_t::NewScope(const char *, SCOPE_enum, const char *fldName, AttrIdEnum)
{
	if (m_scope.empty())
	{
		m_scope.push_scope(mr.typ eobj(), 0);
		frame_t &sc(m_scope.back());
		sc.r = Block_t(0, -1, mpData);
	}
	else
	{
		assert(0);
	}
	return m_scope.back().p;
}

void Strucvar_t::SizeQuery_t::Leave()
{
	m_scope.pop_back();
}

void* Strucvar_t::SizeQuery_t::data()
{
	return m_scope.back().rawdata();
}

HFIELD Strucvar_t::SizeQuery_t::declField(HNAME name, HTYPE iType, AttrIdEnum)
{
	FieldPtr pField(mr.findFieldByName(name));
	if (!pField)
	{
		StrucModifier_t an(*MAIN.project());
		//pField = an.AppendFieldOfType(&mr, name, nullptr);
		Struc_t *pStruc(m_scope.back().p->typeStruc());
		assert(!pStruc->Field(m_scope.back().o));
		pField = an.InsertField OfType(m_scope.back(), name, iType);
	}
	//else
		//pField = mr.fields().at(i);
	m_scope.back().o += pField->size();
	//size_t j((size_t)iType - 1);
	//assert(j < m_types.size());
	//assert(!m_types.at(j).pField);
	//m_types.at(j).pField = pField;
	return pField;
}

/ *HFIELD Strucvar_t::SizeQuery_t::registerField(unsigned, HNAME, HTYPE, unsigned)
{
	return 0;
}* /

HTYPE Strucvar_t::SizeQuery_t::type(OpType_t typeID)
{
	TypesMgr_t *pTypesMgr;
	TypePtr iType(GetStockType(typeID, &pTypesMgr));
	if (iType)
	{
		assert(pTypesMgr == iType->ownerTypeMgr());
		return iType;
	}
	return nullptr;
//	m_types.push_back(type());
//	return (HTYPE)m_types.size();
}

HTYPE	Strucvar_t::SizeQuery_t::type(HNAME)
{
	return 0;
}

PARRAY	Strucvar_t::SizeQuery_t::arrayOf(HTYPE typeIndex, unsigned len)
{
	m_types.push_back(type());
	m_types.back().iSubtype = (size_t)typeIndex;
	m_types.back().iArray = len;
	return (HTYPE)m_types.size();
}

PPTR	Strucvar_t::SizeQuery_t::ptrOf(HTYPE)
{
	return 0;
}*/

FieldPtr Strucvar_t::bitfield(TypePtr iSelf)
{
	if (!mpBitfield0)
	{
		mpBitfield0 = new Field_t;
		TypePtr p(new TypeObj_t);
		p->SetPvt0(new Bitset_t);
		p->setParentField(mpBitfield0);
		mpBitfield0->setType0(p);// (0);
		mpBitfield0->setOwnerComplex(iSelf);
	}
	return mpBitfield0;
}

TypePtr Strucvar_t::bitset(TypePtr iSelf)
{
	return bitfield(iSelf)->type();
}

void Strucvar_t::insertField(FieldPtr p)
{
	//assert(fields().empty() || p->key == VALUE(fields().rbegin())->_key() + 1);//dummy offset
	if (!fields().insert_unique(p).second)
		ASSERT0;
}



//////////////////////////////////////
/*class StrucVarPicker_t : public StrucvarTracer_t
{
	ROWID m_va;
public:
	StrucVarPicker_t(const ModuleInfo_t &rMI, TypeObj_t &r, const BinaryC ontext_t &rbc, ROWID va)
		: StrucvarTracer_t(rMI, r, rbc),
		m_va(va)
	{
	}
	FieldPtr pick(ADDR &at)
	{
		try
		{
			I_DynamicType *device(mrSelf.typeStrucvar()->device());
			if (NewScope(device->name(), SCOPE_ STRUCVAR, nullptr, ATTR_NULL))
			{
				device->createz(*this, -1);
				Leave();
			}
		}
		catch (const std::pair<ADDR, FieldPtr > &e)
		{
			at = e.first;
			return e.second;
		}
		catch(...)
		{
		}
		return nullptr;
	}
protected:
	//it's called before the scope incremented
	virtual void OnField(FieldPtr pField, unsigned range, TypePtr iType)
	{
		frame_t &scope(m_scope.back());
		Binar yContext_t bc(scope.r.ptr + scope.o, 0, range, 0);
		ADDR uSize((ADDR)Size(iType, &bc));

		if (scope.o <= m_va && m_va < scope.o + uSize)
			throw std::make_pair(scope.o, pField);

		scope.o += uSize;
	}
	virtual void OnAlign(ADDR){}
};*/



