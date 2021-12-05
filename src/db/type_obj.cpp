#include "type_obj.h"
#include "type_seg.h"
#include "field.h"
#include "interface/IADCFront.h"

//int TypeObj_t::zid = 0;

//////////////////////////////////////////////
// TypeObj_t

bool TypeObj_t::isTypeSimple(OpType_t t) const {
	const Simple_t *pt(typeSimple());
	if (!pt)
		return false;
	if (t == OPTYP_NULL)
		return true;
	return pt->optype() == t;
}

bool TypeObj0_t::destruct(MemoryMgr_t& rMemMgr)
{
	ClearPvt();
	return true;
}

const char *TypeObj0_t::printType() const
{
	if (typeStruc())
	{
		uint16_t t(tag());
		switch (tag())
		{
		case I_Front::SYMK_VFTABLE: return "vftable";
		case I_Front::SYMK_CVFTABLE: return "cvftable";
		case I_Front::SYMK_LVFTABLE: return "lvftable";
		case I_Front::SYMK_VBTABLE: return "vbtable";
		default: break;
		}
	}
	return mp->printType();
}

bool TypeObj0_t::isVTable() const {
	if (typeStruc())
	{
		uint16_t t(tag());
		switch (tag())
		{
		case I_Front::SYMK_VFTABLE:
		case I_Front::SYMK_CVFTABLE:
		case I_Front::SYMK_LVFTABLE:
		case I_Front::SYMK_VBTABLE:
			return true;
		default:
			break;
		}
	}
	return false;
}

Struc_t* TypeObj0_t::typeUnion() const
{
	const Struc_t* p(mp->typeStruc());
	if (p && p->isUnion((CTypePtr)this))
		return (Struc_t*)p;
	return nullptr;
}


void TypeObj0_t::setName(PNameRef p)
{
//CHECKID(this, 0xedf4)
//STOP
	setName0(p);
	if (mp->name() && !IsLegalCName(mp->name()->c_str()))
		flags() |= TYP_UGLY_NAME;
}

uint64_t TypeObj0_t::imageBase() const
{
	assert(typeSeg());
	return typeSeg()->imageBase((TypePtr)this);
}

TypePtr TypeObj0_t::absBaseType() const
{
	TypePtr pb;
	TypePtr p((TypePtr)this);
	while ((pb = p->baseType()) != nullptr)
		p = pb;
	return p;
}

TypePtr TypeObj0_t::stripToProxy() const
{
	TypePtr p((TypePtr)this);
	while (p && !p->typeProxy())
		p = p->baseType();
	return p;
}

ADDR TypeObj0_t::segSize() const {
	return typeSeg()->segSize((TypePtr)this);
}

bool TypeObj0_t::isShared() const
{
	if (mpOwnerCplx && !mpOwnerCplx->typeStrucLoc())
		return true;
	return false;//pTypeMgr->contains(this);
	//	return IsLinked();
}

/*void TypeObj0_t::replaceId(int newId)
{
	MEMTRACE_RELEASE2(ID());
	SetID(newId);
	MEMTRACE_ADD2(ID());
}*/


bool TypeObj_t::canConform(CTypePtr type, CTypePtr target)
{
	if (type)
	{
		if (!target)
			return true;
		//if (type->size() == target->size())
		{
			if (target->typeSimple() && !OPTYPE(target->typeSimple()->optype()))
				return true;
		}
	}
	return false;
}


