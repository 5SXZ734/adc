#pragma once

#include "type_struc.h"

// variable size structure
class Strucvar_t : public Struc_t
{
	I_DynamicType *mpDevice;
	FieldPtr mpBitfield0;//non-serializable
public:
	Strucvar_t()
		: mpDevice(nullptr),
		mpBitfield0(nullptr)
	{
	}
	Strucvar_t(I_DynamicType *p)
		: mpDevice(p),
		mpBitfield0(nullptr)
	{
	}
	virtual ~Strucvar_t();

	///////////////////////////////////////
	FieldPtr bitfield(TypePtr);
	TypePtr bitset(TypePtr);
	void insertField(FieldPtr);
	bool hasFields() const { return !fields().empty(); }

	I_DynamicType *device();
	virtual Strucvar_t * typeStrucvar() const { return const_cast<Strucvar_t *>(this); }
	virtual int size(CTypePtr) const { return -1; }
	virtual bool maybeUnion() const override { return false; }
	Type_t * custom(ADDR addr);
	//virtual void aka(MakeAlias &) const;
	virtual int ObjType() const { return OBJID_TYPE_STRUCVAR; }
	virtual void releaseDevice();

	//int querySize(PDATA);
	FieldPtr findFieldByName(const char *);
	//FieldsIt findFieldByNameIt(const char *);
};

