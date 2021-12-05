#include "type_alias.h"
#include "info_types.h"
#include "types_mgr.h"
#include "type_code.h"

TypesTracer_t::TypesTracer_t(const ModuleInfo_t &r, TypesMgr_t &rTypesMgr)
	: ModuleInfo_t(r, r.memMgr()),
	mrTypesMgr(rTypesMgr)
{
}

TypesTracer_t::TypesTracer_t(const ModuleInfo_t &r, MemoryMgr_t &rMemMgr, TypesMgr_t &rTypesMgr)
	: ModuleInfo_t(r, rMemMgr),
	mrTypesMgr(rTypesMgr)
{
}

/*TypesTracer_t::TypesTracer_t(TypePtr pType)
	: ProjectInfo_t(PROJ),
	mrTypesMgr(*findTypeMgr(pType))
{
}*/

TypePtr TypesTracer_t::addTypeNew0(TypePtr pType) const
{
	if (!pType)
		return nullptr;
	
	assert(pType->refsNum() == 0);
/*	if (!mrTypesMgr.insertType0(pType))
		return 0;*/

	if (!mrTypesMgr.addNameAlias(pType))
		return nullptr;
	//pType->setOwner(mrTypesMgr.owner());

	if (!pType->typeProxy() && pType->typeComplex())
		AssureNamespace(pType);

	mrProject.markDirty(DIRTY_TYPES);
	return pType;
}

bool TypesTracer_t::removeType(TypePtr pType)
{
	if (!mrTypesMgr.removeNameAlias(pType))
		return false;
	pType->setOwner(nullptr);
	return true;
}


TypePtr TypesTracer_t::stringOf(TypePtr pType, OFF_t o, unsigned range)
{
	const I_DataSource &aRaw(GetDataSource()->pvt());
	const Simple_t* pSimple(pType->typeSimple());
	assert(pSimple);
	unsigned n(1);
	if (pSimple->optype() == OPTYP_CHAR8)
	{
		char ch;
		for (; n < range; n++, o += sizeof(char))
			if (aRaw.dataAt(o, sizeof(char), (PDATA)&ch) == 0 || ch == 0)
				break;
	}
	else if (pSimple->optype() == OPTYP_CHAR16)
	{
		char16_t wch;
		range /= sizeof(char16_t);
		for (; n < range; n++, o += sizeof(char16_t))
			if (aRaw.dataAt(o, sizeof(char16_t), (PDATA)&wch) < sizeof(char16_t) || wch == 0)
				break;
	}
	else
		return pType;
	return arrayOf(pType, n);
}


TypePtr TypesTracer_t::addTypeNew(Type_t *pType) const
{
	return addTypeNew0(mrMemMgr.NewTypeRef(pType));
}

/*TypePtr TypesTracer_t::addTypeNew(Type_t *pType, int id) const
{
	return addTypeNew0(mrMemMgr.NewTypeRef(pType, id), nullptr);
}*/


TypePtr TypesTracer_t::arrayOf(TypePtr pBaseType, unsigned dim) const
{
	assert(pBaseType);
	assert(!pBaseType->typeStrucLoc());
	MakeAlias buf;
	buf.forTypeArray(pBaseType, dim);

#if(NO_TYPE_PROXIES)
	TypesMgr_t *ptm(mrProject.typeMgr());
#else
	TypesMgr_t *ptm(&mrTypesMgr);
#endif
	assert(ptm);

	TypePtr iType(ptm->findTypeByAlias(buf));
	if (!iType)
	{
		bool bTemp(mrTypesMgr.owner() == nullptr);
		iType = mrMemMgr.NewTypeRef(new Array_t);
		Array_t &r(*iType->typeArray());
		r.setTotal0((int)dim);
		SetType(r, pBaseType, bTemp);//type obj must be valid before making this call
		if (bTemp)
			mrTypesMgr.addNameAlias(iType);
		else
			ptm->addNameAlias(iType);
	}
	return iType;
}

TypePtr TypesTracer_t::arrayOfIndex(TypePtr pArrayRef, TypePtr pIndexRef) const
{
	MakeAlias buf;
	buf.forTypeArrayIndex(pArrayRef, pIndexRef);

#if(NO_TYPE_PROXIES)
	TypesMgr_t *ptm(mrProject.typeMgr());
#else
	TypesMgr_t *ptm(&mrTypesMgr);
#endif
	assert(ptm);

	TypePtr iType(ptm->findTypeByAlias(buf));//&mrTypesMgr
	if (!iType)
	{
		bool bTemp(mrTypesMgr.owner() == nullptr);
		iType = mrMemMgr.NewTypeRef(new TypeArrayIndex_t());
		TypeArrayIndex_t &r(*iType->typeArrayIndex());
		SetType(r, pArrayRef, pIndexRef, bTemp);//type obj must be valid before making this call
		if (bTemp)
			mrTypesMgr.addNameAlias(iType);
		else
			ptm->addNameAlias(iType);
	}
	return iType;
}

TypePtr TypesTracer_t::constOf(TypePtr pBaseType) const
{
	assert(pBaseType);
	assert(!pBaseType->typeStrucLoc());
	if (pBaseType->typeConst())
		return pBaseType;//already
	MakeAlias buf;
	buf.forTypeConst(pBaseType);
	TypesMgr_t *ptm(mrProject.typeMgr());
	TypePtr iType(ptm->findTypeByAlias(buf));
	if (!iType)
	{
		bool bTemp(mrTypesMgr.owner() == nullptr);
		iType = mrMemMgr.NewTypeRef(new TypeConst_t);
		TypeConst_t &r(*iType->typeConst());
		SetType(r, pBaseType, bTemp);//type obj must be valid before making this call
		if (bTemp)
			mrTypesMgr.addNameAlias(iType);
		else
			ptm->addNameAlias(iType);
	}
	return iType;
}

TypePtr TypesTracer_t::thunkOf(TypePtr pBaseType) const
{
	assert(pBaseType);
	assert(pBaseType->typeCode());
	if (pBaseType->typeThunk())
		return pBaseType;//already
	MakeAlias buf;
	buf.forTypeThunk(pBaseType);
	TypesMgr_t *ptm(mrProject.typeMgr());
	TypePtr pType(ptm->findTypeByAlias(buf));
	if (!pType)
	{
		bool bTemp(mrTypesMgr.owner() == nullptr);
		pType = mrMemMgr.NewTypeRef(new TypeThunk_t);
		TypeThunk_t &r(*pType->typeThunk());
		SetType(r, pBaseType);//type obj must be valid before making this call
		ptm->addNameAlias(pType);
	}
	return pType;
}

TypePtr TypesTracer_t::enumOf(TypePtr pBaseType, OpType_t typ) const
{
	assert(OPTYP_IS_INT(typ));
	assert(pBaseType);
	assert(!pBaseType->typeStrucLoc());

	MakeAlias buf;
	buf.forTypeEnum(typ, pBaseType);

#if(NO_TYPE_PROXIES)
	TypesMgr_t *ptm(mrProject.typeMgr());
#else
	TypesMgr_t *ptm(&mrTypesMgr);
#endif
	assert(ptm);

	TypePtr iType(ptm->findTypeByAlias(buf));
	if (!iType)
	{
		bool bTemp(mrTypesMgr.owner() == nullptr);
		iType = mrMemMgr.NewTypeRef(new TypeEnum_t(typ));
		TypeEnum_t &r(*iType->typeEnum());
		SetType(r, pBaseType, bTemp);//type obj must be valid before making this call
		if (bTemp)
			mrTypesMgr.addNameAlias(iType);
		else
			ptm->addNameAlias(iType);
	}
	return iType;
}


/*TypePtr TypesTracer_t::varArrayOf(TypePtr iBaseType)
{
	char buf[NAMELENMAX];
	VarArray_t::makeAlias(iBaseType, buf);
	TypePtr iVarArray(mrTypesMgr.findTypeByAlias(buf));
	if (iVarArray)
		return iVarArray;
	TypePtr iType(mrMemMgr.NewTypeRef(new VarArray_t));
	VarArray_t *pVarArray(iType->typeVarArray());
	set Type(*pVarArray, iBaseType);//type obj must be valid before making this call
	return addTypeNew(iType);
}*/

TypePtr	TypesTracer_t::ptrOf(TypePtr pBaseType, OpType_t ptrType)
{
//	assert(!pBaseType || !pBaseType->typeStrucLoc());
	if (!ptrType)
		ptrType = PtrSizeOf(mrTypesMgr);

	MakeAlias buf;
	buf.forTypePtr(pBaseType, ptrType);

#if(NO_TYPE_PROXIES)
	TypesMgr_t *ptm(mrProject.typeMgr());
#else
	TypesMgr_t *ptm(&mrTypesMgr);
#endif
	assert(ptm);

	TypePtr iType(ptm->findTypeByAlias(buf));
	if (!iType)
	{
		bool bTemp(mrTypesMgr.owner() == nullptr);
		iType = mrMemMgr.NewTypeRef();
		TypePtr_t *pPtr(new TypePtr_t(ptrType));
		iType->SetPvt(pPtr);
		if (pBaseType)
			SetType(*pPtr, pBaseType, bTemp);
		ON_OBJ_CREATED(iType);
		if (bTemp)
			mrTypesMgr.addNameAlias(iType);
		else
			ptm->addNameAlias(iType);
	}
	return iType;
}

TypePtr	TypesTracer_t::AcquireTypePtrOf(TypePtr pBaseType, FieldPtr pField, OpType_t ptrType)//no-sharing version
{
	//assert(!pBaseType || (!pBaseType->isShared() && pBaseType->parentField() == pField));
	TypePtr iType(mrMemMgr.NewTypeRef());
	TypePtr_t *pPtr(new TypePtr_t(ptrType));
	iType->SetPvt(pPtr);

	if (pBaseType)
		SetType(*pPtr, pBaseType);

	//?iType->setParentField(pField);
	return iType;
}

TypePtr TypesTracer_t::AcquireTypeThisPtrOf(TypePtr pBaseType, OpType_t ptrType)
{
	MakeAlias buf;
	buf.forTypeThisPtr(pBaseType, ptrType);

#if(NO_TYPE_PROXIES)
	TypesMgr_t *ptm(mrProject.typeMgr());
#else
	TypesMgr_t *ptm(&mrTypesMgr);
#endif
	assert(ptm);

	TypePtr iType(ptm->findTypeByAlias(buf));
	if (!iType)
	{
		bool bTemp(mrTypesMgr.owner() == nullptr);
		iType = mrMemMgr.NewTypeRef();
		TypePtr_t *pPtr(new TypeThisPtr_t(ptrType));
		iType->SetPvt(pPtr);
		if (pBaseType)
			SetType(*pPtr, pBaseType, bTemp);
		ON_OBJ_CREATED(iType);
		if (bTemp)
			mrTypesMgr.addNameAlias(iType);
		else
			ptm->addNameAlias(iType);
	}
	return iType;
}

TypePtr TypesTracer_t::AcquireTypeVPtr(OpType_t ptrType)
{
#if(NO_TYPE_PROXIES)
	TypesMgr_t *ptm(mrProject.typeMgr());
#else
	TypesMgr_t *ptm(&mrTypesMgr);
#endif
	assert(ptm);

	TypePtr iType(ptm->findTypeByAlias(ptrType == OPSZ_QWORD ? TYPEID_VPTR64 : TYPEID_VPTR32));
	if (!iType)
	{
		iType = mrMemMgr.NewTypeRef();
		TypeVPtr_t *pPtr(new TypeVPtr_t(ptrType));
		iType->SetPvt(pPtr);

		ON_OBJ_CREATED(iType);
		assert(mrTypesMgr.owner());//no tmps
		ptm->addNameAlias(iType);
	}
	return iType;
}

TypePtr TypesTracer_t::impOf(TypePtr pBaseType)
{
#if(NO_TYPE_PROXIES)
	TypesMgr_t *ptm(mrProject.typeMgr());
#else
	TypesMgr_t *ptm(&mrTypesMgr);
#endif
	assert(ptm);

	MakeAlias buf;
	buf.forTypeImp(pBaseType);

	TypePtr iType(ptm->findTypeByAlias(buf));
	if (!iType)
	{
		iType = mrMemMgr.NewTypeRef();
		TypeImp_t *p(new TypeImp_t);
		iType->SetPvt(p);
		if (pBaseType)
			SetType(*p, pBaseType);

		ON_OBJ_CREATED(iType);
		assert(mrTypesMgr.owner());//no tmps
		ptm->addNameAlias(iType);
	}
	return iType;
}

TypePtr TypesTracer_t::expOf(TypePtr pBaseType)
{
#if(NO_TYPE_PROXIES)
	TypesMgr_t *ptm(mrProject.typeMgr());
#else
	TypesMgr_t *ptm(&mrTypesMgr);
#endif
	assert(ptm);

	MakeAlias buf;
	buf.forTypeExp(pBaseType);

	TypePtr iType(ptm->findTypeByAlias(buf));
	if (!iType)
	{
		iType = mrMemMgr.NewTypeRef();
		TypeExp_t *p(new TypeExp_t);
		iType->SetPvt(p);
		if (pBaseType)
			SetType(*p, pBaseType);

		ON_OBJ_CREATED(iType);
		assert(mrTypesMgr.owner());//no tmps
		ptm->addNameAlias(iType);
	}
	return iType;
}

TypePtr	TypesTracer_t::refOf(TypePtr pBaseType, OpType_t baseType)
{
	MakeAlias buf;
	buf.forTypeRef(pBaseType, baseType);

#if(NO_TYPE_PROXIES)
	TypesMgr_t *ptm(mrProject.typeMgr());
#else
	TypesMgr_t *ptm(&mrTypesMgr);
#endif
	assert(ptm);

	TypePtr iType(ptm->findTypeByAlias(buf));
	if (!iType)
	{
		bool bTemp(!mrTypesMgr.owner());
		iType = mrMemMgr.NewTypeRef();
		TypePtr_t *pPtr(new TypeRef_t(baseType));
		iType->SetPvt(pPtr);
		if (pBaseType)
			SetType(*pPtr, pBaseType, bTemp);
		ON_OBJ_CREATED(iType);
		if (bTemp)
			mrTypesMgr.addNameAlias(iType);
		else
			ptm->addNameAlias(iType);
	}
	return iType;
}

TypePtr	TypesTracer_t::refOf(TypePtr pBaseType, FieldPtr pField, OpType_t ptrType)//non-share version
{
	TypePtr iType(mrMemMgr.NewTypeRef());
	TypePtr_t *pPtr(new TypeRef_t(ptrType));
	iType->SetPvt(pPtr);

	if (pBaseType)
		SetType(*pPtr, pBaseType);

	//?iType->setParentField(pField);
	return iType;
}

TypePtr	TypesTracer_t::rvalRefOf(TypePtr pBaseType, OpType_t ptrType)
{
	MakeAlias buf;
	buf.forTypeRRef(pBaseType, ptrType);

#if(NO_TYPE_PROXIES)
	TypesMgr_t *ptm(mrProject.typeMgr());
#else
	TypesMgr_t *ptm(&mrTypesMgr);
#endif
	assert(ptm);

	TypePtr iType(ptm->findTypeByAlias(buf));
	if (!iType)
	{
		bool bTemp(!mrTypesMgr.owner());
		iType = mrMemMgr.NewTypeRef();
		TypePtr_t *pPtr(new TypeRRef_t(ptrType));
		iType->SetPvt(pPtr);

		if (pBaseType)
			SetType(*pPtr, pBaseType, bTemp);
		ON_OBJ_CREATED(iType);
		if (bTemp)
			mrTypesMgr.addNameAlias(iType);
		else
			ptm->addNameAlias(iType);
	}
	return iType;
}

TypePtr	TypesTracer_t::pairOf(TypePtr left, TypePtr right)
{
	MakeAlias buf;
	buf.forTypePair(left, right);

#if(NO_TYPE_PROXIES)
	TypesMgr_t *ptm(mrProject.typeMgr());
#else
	TypesMgr_t *ptm(&mrTypesMgr);
#endif
	assert(ptm);

	TypePtr iType(ptm->findTypeByAlias(buf));
	if (!iType)
	{
		bool bTemp(!mrTypesMgr.owner());
		iType = mrMemMgr.NewTypeRef();
		TypePair_t *pType(new TypePair_t);
		iType->SetPvt(pType);

		SetType(*pType, left, right, bTemp);
		ON_OBJ_CREATED(iType);
		if (bTemp)
			mrTypesMgr.addNameAlias(iType);
		else
			ptm->addNameAlias(iType);
	}
	return iType;

}

TypePtr	TypesTracer_t::funcTypeOf(TypePtr ret, TypePtr args, unsigned flags)
{
	MakeAlias buf;
	buf.forTypeFunc(ret, args, flags);

#if(NO_TYPE_PROXIES)
	TypesMgr_t *ptm(mrProject.typeMgr());
#else
	TypesMgr_t *ptm(&mrTypesMgr);
#endif
	assert(ptm);

	TypePtr iType(ptm->findTypeByAlias(buf));
	if (!iType)
	{
		bool bTemp(!mrTypesMgr.owner());
		iType = mrMemMgr.NewTypeRef();
		TypeFunc_t *pType(new TypeFunc_t);
		iType->SetPvt(pType);

		SetType(*pType, ret, args, flags, bTemp);
		ON_OBJ_CREATED(iType);
		if (bTemp)
			mrTypesMgr.addNameAlias(iType);
		else
			ptm->addNameAlias(iType);
	}
	return iType;
}

TypePtr TypesTracer_t::findTypeByName(const MyString &aTypeName) const
{
	//try a N-map
	CTypePtr iScope(mrTypesMgr.owner());
	ObjPtr pObj;
	if ((pObj = FindObjByAutoName(aTypeName, iScope)) == nullptr)
		pObj = FindObjByName(aTypeName, iScope);
	if (!pObj)
	{
		//check for array subscript
		//MyStringList l(MyStringList::split("[", type_name));
		if (aTypeName.endsWith("]"))
		{
			int i(aTypeName.findRev('['));
			if (i > 0)
			{
				TypePtr iType(findTypeByName(aTypeName.substr(0, i)));
				if (iType)
				{
					MyString s(aTypeName.substr(i + 1, aTypeName.length() - i - 2));
					return arrayOf(iType, atoi(s.c_str()));
				}
			}
		}
		return nullptr;
	}

	TypePtr pType(pObj->objTypeGlob());
CHECKID(pType, 0x790c)
STOP
	if (!pType || (pType->typeFuncDef() && !pType->isShared()))
		return nullptr;

	assert(mrTypesMgr.contains(pType));
	return pType;

	/*?setTYPE_it it = mTypes.find(pType);
	if (it != mTypes.end())
	return it->first;

	return nullptr;*/
}

void TypesTracer_t::setTypeCode(TypeObj_t *pCode)
{
	//mpTypeCode = pCode;
	assert(pCode);
	mrTypesMgr.setTypeCode(pCode);//addTypeNew(pCode, nullptr);//TYPEID_CODE);must be unnamed
	//?mrTypesMgr.mpTypeCode->setOwner(mrTypesMgr.mpTSeg);
}

