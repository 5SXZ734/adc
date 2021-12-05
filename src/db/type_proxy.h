#pragma once

#include "type_alias.h"


//////////////////////////////////////////////////////////////////////TypeProxy_t
// a proxy type, representing another type from different module
class TypeProxy_t : public Type_t
{
	typedef	Type_t	Base_t;
	TypePtr		mpIncumb;
	PNameRef	mpName;
public:
	TypeProxy_t() : mpIncumb(0), mpName(0)
	{
	}
	TypeProxy_t(TypePtr pBaseType) : mpIncumb(pBaseType), mpName(0)
	{
		assert(!mpIncumb->typeProxy());
	}
	virtual ~TypeProxy_t()
	{
		assert(!mpIncumb);
		assert(!mpName);
	}
	TypePtr incumbent() const { return mpIncumb; }
	void setIncumbent(TypePtr p)
	{
		assert(!p || !p->typeProxy());
		mpIncumb = p;
	}
	void setIncumbent0(TypePtr p)//for loading
	{
		mpIncumb = p;
	}

	virtual PNameRef	name() const { return mpName; }//for named objects
	virtual void	setName(PNameRef p){ mpName = p; }
	virtual void	setNameRef(PNameRef p){
		mpName = p; 
#if(SHARED_NAMES)
		if (mpName)
			mpName->addRef();
#endif
	}
protected:

	//everything else must be forwarded

/*#if(NO_OBJ_ID)
	virtual COMP_ID ID() const { return mpIncumb->ID(); }
	virtual void SetID(COMP_ID id){ mpIncumb->SetID(id); }
#endif*/

	//virtual void		setName(PNameRef p){ mpIncumb->setName(p); }
	//virtual void		setNameRef(PNameRef p){ mpIncumb->setNameRef(p); }
	//virtual PNameRef	name() const { return mpIncumb->name(); }
	virtual void		namex(MyString &s) const { mpIncumb->namex(s); }
	virtual void		defaultName(MyString &s) const { mpIncumb->defaultName(s); }
	virtual void		aka(MakeAlias &buf) const { buf.forTypeProxy(mpIncumb); }

/*	virtual Project_t*		typeProject() const { assert(0); return nullptr; }
	virtual Simple_t*		typeSimple() const { return mpIncumb->typeSimple(); }
	virtual Complex_t*		typeComplex() const { return mpIncumb->typeComplex(); }
	virtual Struc_t*		typeStruc() const { return mpIncumb->typeStruc(); }
	virtual TypeClass_t*	typeClass() const { return mpIncumb->typeClass(); }
	virtual TypeUnion_t*		typeUnion() const { return mpIncumb->typeUnion(); }//Complex_t
	virtual FuncDef_t*		typeFuncDef() const { return mpIncumb->typeFuncDef(); }
	virtual Array_t*		typeArray() const { return mpIncumb->typeArray(); }
	virtual TypePtr_t*			typePtr() const { return mpIncumb->typePtr(); }
	virtual TypeImp_t*	typeImp() const { assert(!mpIncumb->typeImp()); return nullptr; }//not allowed
	virtual Typedef_t*		typeTypedef() const { return mpIncumb->typeTypedef(); }
	virtual Seg_t*			typeSeg() const { return mpIncumb->typeSeg(); }
	virtual	Module_t*		typeModule() const { return mpIncumb->typeModule(); }
	virtual TypeCode_t*			typeCode() const { return mpIncumb->typeCode(); }
	virtual Strucvar_t*		typeStrucvar() const { return mpIncumb->typeStrucvar(); }
	//virtual TypeFuncEnd_t*	typeFuncEnd() const { return mpIncumb->typeFuncEnd(); }
	//virtual Bit_t*		typeBit() const { return mpIncumb->typeBit(); }
	virtual Bitset_t*		typeBitset() const { return mpIncumb->typeBitset(); }
	virtual TypeEnum_t*		typeEnum() const { return mpIncumb->typeEnum(); }
	virtual TypeArrayIndex_t* typeArrayIndex() const { return mpIncumb->typeArrayIndex(); }*/
	virtual TypeProxy_t*	typeProxy() const { return const_cast<TypeProxy_t *>(this); }

	virtual int size(CTypePtr p) const { return mpIncumb->type()->size(p); }
	virtual ADDR base(CTypeBasePtr p) const { return mpIncumb->type()->base(p); }
	virtual void releaseDevice(){ mpIncumb->releaseDevice(); }
	virtual int ObjType() const { return OBJID_TYPE_PROXY; }// mpIncumb->ObjType();}
	virtual TypePtr	baseType() const { return mpIncumb/*->baseType()*/; }

	//?
	//virtual uint8_t optype() const { return mpIncumb->optype(); }
	//virtual TypePtr type() const { return mpIncumb->type()->type(); }
	virtual TypesMgr_t * typeMgr() const { return mpIncumb->type()->typeMgr(); }
	virtual const char *printType() const { return mpIncumb->printType(); }
};





