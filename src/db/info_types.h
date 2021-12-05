#pragma once

#include "info_module.h"

class TypesTracer_t : public ModuleInfo_t
{
	TypesMgr_t &mrTypesMgr;
public:
	TypesTracer_t(const ModuleInfo_t &, TypesMgr_t &rTypesMgr);
	TypesTracer_t(const ModuleInfo_t &, MemoryMgr_t &rMemMgr, TypesMgr_t &rTypesMgr);
	//TypesTracer_t(TypePtr pType);

	TypePtr addTypeNew0(TypePtr) const;
	TypePtr addTypeNew(Type_t *) const;
	//TypePtr addTypeNew(Type_t *, int id) const;
	bool removeType(TypePtr);

	TypePtr stringOf(TypePtr, OFF_t ptr, unsigned range);
	TypePtr arrayOf(TypePtr, unsigned n) const;
	TypePtr arrayOfIndex(TypePtr, TypePtr index) const;
	TypePtr constOf(TypePtr) const;
	TypePtr thunkOf(TypePtr) const;
	TypePtr enumOf(TypePtr, OpType_t base) const;
	//TypePtr varArrayOf(TypePtr iType);
	TypePtr ptrOf(TypePtr, OpType_t = OPTYP_NULL);
	TypePtr	AcquireTypePtrOf(TypePtr, FieldPtr, OpType_t);//no sharing
	TypePtr AcquireTypeThisPtrOf(TypePtr, OpType_t);
	TypePtr AcquireTypeVPtr(OpType_t);
	TypePtr impOf(TypePtr);
	TypePtr expOf(TypePtr);
	TypePtr refOf(TypePtr, OpType_t);
	TypePtr refOf(TypePtr, FieldPtr, OpType_t);//no sharing
	TypePtr rvalRefOf(TypePtr, OpType_t);
	TypePtr	pairOf(TypePtr left, TypePtr right);
	TypePtr	funcTypeOf(TypePtr ret, TypePtr args, unsigned flags);
	void setTypeCode(TypePtr);
	//bool SetFieldType(FieldPtr, uint8_t optyp, TypePtr iStruc, int nPtr, int nArray);
	TypePtr findTypeByName(const MyString &) const;
};


