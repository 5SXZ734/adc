#include "type_alias.h"
#include "info_strucvar.h"
#include "type_strucvar.h"
#include "types_mgr.h"
#include "info_types.h"



StrucvarTracer0_t::StrucvarTracer0_t(const ModuleInfo_t &rMI, TypeObj_t &r, unsigned scopeRange)
	: ModuleInfo_t(rMI),
	mrSelf(r),
	mRange(scopeRange),
	m_sizez(r.base()),
	mCurentDepth(0)
{
}

///////////////////////////////////////////////////

StrucvarTracer_t::StrucvarTracer_t(const ModuleInfo_t &rMI, TypeObj_t &r, unsigned scopeRange)
	: StrucvarTracer0_t(rMI, r, scopeRange),
	m_scope(*this)
{
}

TypePtr StrucvarTracer0_t::declTypedef(const char *, TypePtr)
{
	assert(0);
	return nullptr;
}

TypePtr StrucvarTracer0_t::NewScope(const char *pTypeName, SCOPE_enum e, AttrScopeEnum attr2)
{
	return newScope(pTypeName, e, nullptr, 0, attr2);
}

TypePtr StrucvarTracer0_t::NewScope(FieldPtr p, SCOPE_enum e, AttrScopeEnum attr2)
{
	return newScope(p, e, nullptr, 0, attr2);
}

TypePtr StrucvarTracer_t::newScope(const char *pTypeName, SCOPE_enum e, const char *fldName, unsigned, AttrScopeEnum)
{
	//?assert(mrSelf.namexx().endsWith(pTypeName));
	if (m_scope.empty())
	{
		assert(pTypeName);
		m_scope.push_scope(&mrSelf, 0, true);
		mCurentDepth++;
		Block_t &sc(m_scope.back());
		sc = Block_t(0, -1);
	}
	else
	{
		assert(!pTypeName);
		frame_t &top(m_scope.back());
		TypePtr iOwner(top.cont());
		FieldPtr pField;
		if (iOwner->typeStrucvar())
			pField = iOwner->typeStrucvar()->findFieldByName(fldName);
		else
			pField = findFieldByName(iOwner, fldName);
		assert(pField);
		TypePtr iType(pField->type());
		assert(iType && iType->typeStruc());
		unsigned range(-1);//?
//?		OnField(pField, range, iType);
		m_scope.push_scope(iType, 0, true);
		mCurentDepth++;
		//frame_t &cur(m_scope.back());
		//cur.r = Block_t(0, -1, mrbc.pData);
		
	}
	return &mrSelf;
}

TypePtr StrucvarTracer_t::newScope(FieldPtr p, SCOPE_enum e, const char* fldName, unsigned, AttrScopeEnum)
{
	assert(0);
	return 0;
}

void StrucvarTracer_t::Leave()
{
	//m_scope.pop_back();
	m_sizez = m_scope.pop_scope();
	mCurentDepth--;
}

POSITION StrucvarTracer0_t::align(unsigned powerOf2)
{
	ADDR scp_o(scopeOffs());
	//frame_t &scp(m_scope.back());
	ADDR o(scp_o);
	ADDR u(((unsigned)-1) << powerOf2);
	u = ~u;
	if (!(o & u))
		return 0;//already
	o >>= powerOf2;
	o++;
	o <<= powerOf2;
	OnAlign(o/* - scp_o*/);
	//scp.o = o;
	return o;
}

unsigned StrucvarTracer0_t::skip(int bytes)
{
	if (bytes < 0)
		error("negative value for skip");
	ADDR scp_o(scopeOffs());
	OnAlign(scp_o + bytes);
	return bytes;
}

unsigned StrucvarTracer0_t::skipBits(int bits)
{
	if (bits < 0)
		error("negative value for skipBits");
	assert(scopeContainer()->typeBitset());
	OnAlign(bits);
	return bits;
}

void StrucvarTracer_t::OnAlign(unsigned o)
{
	frame_t &scp(m_scope.back());
	scp.m_addr = o;
}

FieldPtr StrucvarTracer0_t::DeclareBitField(HNAME name, TypePtr iType, AttrIdEnum attr, ADDR at)
{
	CTypePtr pScope(scopeContainer());
	if (!pScope->typeBitset())
	{
		//New Scope(nullptr, SCOPEX_BITSET, nullptr, ATTR_NULL);
	}
	else
	{
		if (!iType)
			throw FieldBadTypeException();
		unsigned uTypeSize(iType->size());
		unsigned uBaseType(SkipArray(iType)->size());
		unsigned uBitsNum(uTypeSize / uBaseType);
		unsigned uScopeOffs(scopeOffs());
		if (uScopeOffs + uBitsNum > uBaseType * CHAR_BIT)
			newScope((HNAME)nullptr, (SCOPE_enum)SCOPEX_BITSET, nullptr, 0, AttrScopeEnum::null);
	}
	SET_ATTR_TYPE(attr, __ATTR_BITSET);
	return DeclareField(name, iType, attr, at);
}

FieldPtr StrucvarTracer0_t::DeclareEnumField(HNAME)//, AttrIdEnum)
{
	assert(0);
	return 0;
}

FieldPtr StrucvarTracer0_t::DeclareEnumField(HNAME, ADDR)//, AttrIdEnum)
{
	assert(0);
	return 0;
}

FieldPtr StrucvarTracer0_t::DeclareUnionField(HNAME name, TypePtr iType, AttrIdEnum attr)
{
	assert(0);
	return 0;
}

FieldPtr StrucvarTracer0_t::DeclareUnionField(HNAME name, SCOPE_enum e, AttrIdEnum attr)
{
	assert(0);
	return 0;
}

FieldPtr StrucvarTracer0_t::DeclareField(HNAME name, TypePtr iType0, AttrIdEnum attr, ADDR at)
{
//	if (!iType0)
//		throw FieldBadTypeException();

	assert(at == -1);

	TypePtr iScope(scopeContainer());

	if (iType0 && iType0 == iScope)
	{
		assert(0); abort();
		/*discardLastFrame();
		newScope((HNAME)nullptr, SCOPE_ STRUCVAR, name, 0, AttrScopeEnum::null);
		static FieldTmp_t dummy;
		return &dummy;*/
	}

	bool bIsBitfied(CHECK_ATTR_TYPE(attr, __ATTR_BITSET));

	if (iScope->typeBitset() && !bIsBitfied)
	{
		//close a bitset
		leave1();
		iScope = scopeContainer();
	}

	ADDR iScopeOffs(scopeOffs());
	unsigned uRange(scopeRange());

	//frame_t &scope(m_scope.back());
	//if (!(iScopeOffs <= uRange))
	if (uRange == 0)
	{
		//assert(iScopeOffs == mrbc.range);
		//throw E_FIELD_OVERLAP;
		throw FieldOverlapException(uRange);
	}

	if (bIsBitfied && !iScope->typeBitset())
	{
		unsigned uBitsetSizeBytes(SkipArray(iType0)->size());
		iScope = newScope((HNAME)nullptr, (SCOPE_enum)SCOPEX_BITSET, nullptr, uBitsetSizeBytes, AttrScopeEnum::null);//here we got 0 range and extent!
		setScopeRange(uBitsetSizeBytes);//extent?
		uRange = uBitsetSizeBytes * CHAR_BIT;
	}

	mLastField.setType0(iType0);
	mLastField.setName(name);
	mLastField.setOwnerComplex(iScope);
	mLastField.setAttr(attr);
	//mLastField.setKey();

	OnField(&mLastField, uRange, iType0);
	return &mLastField;

/*	if (!bIsBitfied)
	if (!pField->type() || pField->type()->typeSimple())
	{
		unsigned sz(pField->type()->size());
		assert(sz > 0);
		if (sz > uRange)
			throw FieldOverlapException(uRange);
	}
	TypePtr iType(pField->type());
	if (iType != iType0)
	{
		Array_t *pArray1(pField->type()->typeArray());
		Array_t *pArray2(iType0->typeArray());
		if (!pArray1 || !pArray2
			|| pArray1->baseType() != pArray2->baseType())
		{
			//can't bee
			assert(0);
		}
		iType = iType0;
	}

	//uRange -= iScopeOffs;
	//if (!bIsBitfied)
	{
		OnField(pField, uRange, iType);
	}
	//else
	{

	}

	return pField;*/
}

void StrucvarTracer_t::OnField(FieldPtr pField, unsigned range, TypePtr iType)
{
	frame_t &scope(m_scope.back());
	ROWID da;
	ADDR sz((ADDR)Size(iType, &da));
	scope.m_addr += sz;
//?	if (m_sizez < scope.o)
	//?	m_sizez = scope.o;
}

TypePtr StrucvarTracer_t::scopeContainer() const
{
	return m_scope.back().cont();
}

TypePtr StrucvarTracer_t::scopeSeg() const
{
	assert(0);
	return nullptr;
}

ADDR StrucvarTracer_t::scopeOffs() const
{
	return m_scope.back().m_addr;
}

unsigned StrucvarTracer_t::scopeRange() const
{
	return mRange;
}

bool StrucvarTracer_t::isLarge() const
{
	return false;
}

void StrucvarTracer_t::leave1()
{
	assert(0);
}

TypePtr StrucvarTracer0_t::type(OpType_t optype)
{
	return GetStockType(optype);
}

TypePtr StrucvarTracer0_t::type(HNAME name)
{
/*	MyString s(name);
	if (s.startsWith(EXSYM_PFX))
		s = FrontInfo_t::toFrontKey(s, mrFront.name());

	TypesMgr_t *pTypesMgr;
	return findType(scope().p, s, &pTypesMgr);
	*/

	return findType(scopeContainer(), name);
}

TypesMgr_t &StrucvarTracer_t::typesMgrNS(TypePtr p) const
{
	assert(p->owner());
	return *p->ownerTypeMgr();
}

MemoryMgr_t &StrucvarTracer_t::memMgrNS() const
{
	return memMgr();
}

TypePtr StrucvarTracer0_t::arrayOf(TypePtr iType, unsigned num, bool bytes)
{
	if (!iType)
		return nullptr;
	assert(!typesMgrNS(iType).owner());
	TypesTracer_t TT(*this, memMgrNS(), typesMgrNS(iType));
	assert(num >= 0);
	if (bytes)
	{
		if (iType->typeSimple())//try to exclude simple types
		{
			int sz(iType->size());
			assert(num % sz == 0);
			num /= sz;
		}
		else
			num |= ARRAY_BYTES_MASK;
	}
	iType = TT.arrayOf(iType, num);
	iType->flags() |= TYP_NO_REF;
	return iType;
}

TypePtr StrucvarTracer0_t::arrayOfIndex(TypePtr iType, TypePtr iTypeIndex)
{
	assert(!typesMgrNS(iType).owner());
	TypesTracer_t TT(*this, memMgrNS(), typesMgrNS(iType));
	iType = TT.arrayOfIndex(iType, iTypeIndex);
	iType->flags() |= TYP_NO_REF;
	return iType;
}

TypePtr StrucvarTracer0_t::enumOf(TypePtr iType, OpType_t typ)
{
	if (!iType)
		return nullptr;//error("no underlying type for enum");
	if (!typ || !OPSIZE(typ))
		typ = OPTYP_UINT32;//?
	else if (!OPTYPE(typ))
		typ = MAKETYP_UINT(typ);
	else if (!OPTYP_IS_INT(typ))
		ASSERT0;
	assert(!typesMgrNS(iType).owner());
	TypesTracer_t TT(*this, memMgrNS(), typesMgrNS(iType));
	iType = TT.enumOf(iType, OpType_t(typ));
	iType->flags() |= TYP_NO_REF;
	return iType;
}

TypePtr StrucvarTracer0_t::ptrOf(TypePtr iType, I_Module::PTR_TYPE_t eType, I_Module::PTR_MODE_t eMode)
{
	TypesMgr_t *pTypesMgr;
	if (!iType)
	{
		iType = GetStockType(OPTYP_NULL, &pTypesMgr);
		assert(iType && pTypesMgr == iType->ownerTypeMgr());
		iType = nullptr;
	}
	else
		pTypesMgr = iType->ownerTypeMgr();

	OpType_t ptrSize(from_PTR_TYPE(eType));
	if (ptrSize == OPSZ_NULL)
	{
		TypePtr iSeg(scopeSeg());// OwnerSeg(scopeContainer()));
		ptrSize = iSeg->typeSeg()->ptrSize();
	}

	TypesTracer_t TT(*this, *pTypesMgr);
	return TT.ptrOf(iType, ptrSize);
}

POSITION StrucvarTracer0_t::setcp(POSITION)
{
	assert(0);
	return 0;
}

POSITION StrucvarTracer_t::cp() const
{
	POSITION pos(0);
	ScopeStack_t::const_reverse_iterator cri(m_scope.rbegin());

	for (; cri != m_scope.rend(); cri++)
	{
		const frame_t &f(*cri);
		assert(f.cont()->typeStrucvar());
		pos += f.m_addr;
	}

	assert(0);//?pos += mrbc.va;
	/*POSITION pos(scopeOffs());//scp.o);
	TypePtr p(scopeContainer());//scp.p);
	while (p && !p->typeSeg())
	{
		pos -= p->base();
		FieldPtr f(p->parentField());
		if (!f)
			break;
		pos += f->_key();
		p = f->owner();
	}*/
	return pos;
}


void StrucvarTracer0_t::error(const char *msg)
{
	PrintError() << msg << std::endl;
}

/*unsigned StrucvarTracer0_t::calcSize(I_Module& iface)
{
	assert(m_sizez == 0);
//?	assert(m_scope.empty());
	try
	{
		I_DynamicType *device(mrSelf.typeStrucvar()->device());
		if (!device)
			return -1;//emplaced?
		if (NewScope(device->name(), SCOPE _STRUCVAR))
		{
			device->createz(iface, -1);
			Leave();
		}
	}
	catch (...)
	{
		STOP
	//	assert(0);
	}
	return m_sizez;
}*/


I_Front* StrucvarTracer0_t::frontend() const
{
	//assert(!m_cont.empty());
	CTypePtr pFromSeg(OwnerSeg(scopeContainer()));
	CTypePtr pFrontSeg(FindFrontSegUp(pFromSeg));
	if (pFrontSeg)
	{
		FRONT_t *pF(FrontEnd(pFrontSeg));
		return pF->device(GetDataSource());
	}
	return nullptr;

}