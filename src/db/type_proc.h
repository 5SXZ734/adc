#pragma once

//#include "shared/anlz.h"
#include "type_struc.h"

class TypeProc_t : public Struc_t
{
	typedef	Struc_t	Base_t;

public:
	TypeProc_t(){}
	virtual int ObjType() const { return OBJID_TYPE_PROC; }
	virtual void aka(MakeAlias &) const;
	//virtual int size(CTypePtr) const;
	virtual TypeProc_t *	typeProc() const { return const_cast<TypeProc_t *>(this); }
	virtual void namex(MyString&) const;
	//int		SetFuncEndAddr(uint32_t addr);
	virtual const char *printType() const { return "func"; }
	virtual bool maybeUnion() const override { return false; }
};











