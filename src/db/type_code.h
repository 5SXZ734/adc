#pragma once

#include "types.h"
#include "interface/IADCFront.h"

class I_Code;

class TypeCode_t : public Complex_t
{
	typedef	Complex_t	Base_t;
	I_Code *mpDevice;

public:
	TypeCode_t();
	virtual ~TypeCode_t();
	I_Code *dis();
	virtual int ObjType() const { return OBJID_TYPE_CODE; }
	//virtual PNameRef name() const;//? { return TYPEID_CODE; }
	virtual void aka(MakeAlias&) const;
	virtual void namex(MyString &s) const { s = name()->c_str(); }
	virtual int size(CTypePtr) const { return -1; }//variable
	virtual TypeCode_t * typeCode() const { return const_cast<TypeCode_t *>(this); }
	virtual void releaseDevice();
	int unassemble(DataStream_t &, ADDR64, ADDR, ins_desc_t &);
	int print(DataStream_t &, ADDR64, ADDR, IOutpADDR2Name *, ins_desc_t &);
	int skip(DataStream_t &, ADDR64, ADDR, ins_desc_t &);
	int generate(DataStream_t &, ADDR64, ADDR, ins_desc_t &, IPCode_t &, const I_FrontDC &);
};


class TypeThunk_t : public Type_t
{
	TypePtr mpCode;
public:
	TypeThunk_t() : mpCode(nullptr){}
	virtual TypePtr	baseType() const { return mpCode; }
	virtual void setBaseType(TypePtr p){ mpCode = p; }

	virtual int ObjType() const { return OBJID_TYPE_THUNK; }
	virtual TypeThunk_t* typeThunk() const { return const_cast<TypeThunk_t *>(this); }
	//virtual TypeCode_t* typeCode() const { return const_cast<TypeCode_t *>(mpCode->typeCode()); }
	virtual const char *printType() const { return "thunk"; }
	virtual void aka(MakeAlias&) const;
	virtual int size(CTypePtr) const { return -1; }//variable
};
