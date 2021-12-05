#pragma once

#include "info_module.h"


class FieldTmp_t : public Field_t//a wrapper to reset field's type if exception is thrown in client code
{
	MyString mName;
	NameRef_t aNameRef;
public:
	FieldTmp_t()//avoid objid increment
	{
		m_nFlags |= FLD_TEMP;
	}
	void setName(const char* name)
	{
		mName = name;
		if (!mName.empty())
		{
			aNameRef.overrideKey((char *)mName.c_str());
			setName0(&aNameRef);
		}
		else
		{
			aNameRef.overrideKey(nullptr);
			setName0(nullptr);
		}
	}
	void setKey(FieldKeyType a)
	{
		overrideKey(a);
	}
	void setAttr(AttrIdEnum a)
	{
		m_nFlags &= (FLD_TEMP | OBJ_TYPE);//reset all except these
		setAttributeFromId(a);
	}
	~FieldTmp_t()
	{
		setType0(nullptr);
		setName0(nullptr);
		aNameRef.overrideKey(nullptr);
	}
};


///////////////////////////////////////////////// StrucvarTracer_t
//template <typename T>
class StrucvarTracer0_t : public ModuleInfo_t
{
protected:
	TypeObj_t &mrSelf;//Strucvar
	unsigned mRange;
	unsigned m_sizez;
	int mCurentDepth;//true after run if no problem
	FieldTmp_t mLastField;
protected:
	class FieldOverlapException : std::exception
	{
		unsigned _delta;
	public:
		FieldOverlapException(unsigned d) : _delta(d){}
		virtual char const* what() const noexcept { return "field overlap"; }
		unsigned delta() const { return _delta; }
	};
	struct FieldBadTypeException : std::exception
	{
	public:
		FieldBadTypeException(){}
		virtual char const* what() const noexcept { return "bad type"; }
	};
public:
	StrucvarTracer0_t(const ModuleInfo_t &, TypeObj_t &r, unsigned scopeRange);
	virtual ~StrucvarTracer0_t() {}
	unsigned sizez() const { return m_sizez; }
	//unsigned calcSize(I_Module&);
protected:
	friend class StrucvarIface_t;
	TypePtr	NewScope(const char *sTypeName, SCOPE_enum, AttrScopeEnum = AttrScopeEnum::null);
	TypePtr	NewScope(FieldPtr, SCOPE_enum, AttrScopeEnum = AttrScopeEnum::null);
	unsigned	skip(int);
	unsigned	skipBits(int);
	POSITION	align(unsigned powerOf2);
	TypePtr	declTypedef(const char *, TypePtr);
	FieldPtr	DeclareField(HNAME, TypePtr, AttrIdEnum, ADDR at);
	FieldPtr	DeclareUnionField(HNAME, TypePtr, AttrIdEnum);
	FieldPtr	DeclareUnionField(HNAME, SCOPE_enum, AttrIdEnum);
	FieldPtr	DeclareBitField(HNAME, TypePtr, AttrIdEnum, ADDR at);
	FieldPtr	DeclareEnumField(HNAME);// , AttrIdEnum);
	FieldPtr	DeclareEnumField(HNAME, ADDR);//, AttrIdEnum);
	TypePtr	type(OpType_t optype);
	TypePtr	type(HNAME name);
	TypePtr	arrayOf(TypePtr t, unsigned num, bool bytes);
	TypePtr	arrayOfIndex(TypePtr, TypePtr);
	TypePtr	enumOf(TypePtr, OpType_t);
	TypePtr	ptrOf(TypePtr, I_Module::PTR_TYPE_t, I_Module::PTR_MODE_t);
	POSITION	setcp(POSITION);
	void error(const char *);
	I_Front* frontend() const;
protected:
	virtual POSITION cp() const = 0;
	virtual void Leave() = 0;
	virtual void OnField(FieldPtr pField, unsigned range, TypePtr iType) = 0;
	virtual void OnAlign(ADDR) = 0;
	virtual TypePtr scopeContainer() const = 0;
	virtual TypePtr scopeSeg() const = 0;
	virtual ADDR scopeOffs() const = 0;
	virtual void setScopeRange(unsigned) = 0;
	virtual unsigned scopeRange() const = 0;
	virtual bool isLarge() const = 0;
	virtual void leave1() = 0;
	virtual TypePtr newScope(const char *sTypeName, SCOPE_enum e, const char *pFieldName, unsigned range, AttrScopeEnum) = 0;
	virtual TypePtr newScope(FieldPtr, SCOPE_enum e, const char *pFieldName, unsigned range, AttrScopeEnum) = 0;
	virtual TypesMgr_t &typesMgrNS(TypePtr) const = 0;
	virtual MemoryMgr_t &memMgrNS() const = 0;
	virtual void discardLastFrame() = 0;
};


class StrucvarTracer_t : public StrucvarTracer0_t
{
	ScopeStack_t m_scope;
public:
	StrucvarTracer_t(const ModuleInfo_t &, TypeObj_t &r, unsigned scopeRange);
protected:
	virtual POSITION cp() const;
	virtual void Leave();
	virtual void OnField(FieldPtr pField, unsigned range, TypePtr iType);
	virtual void OnAlign(ADDR);
	virtual TypePtr scopeContainer() const;
	virtual TypePtr scopeSeg() const;
	virtual ADDR scopeOffs() const;
	virtual void setScopeRange(unsigned){}
	virtual unsigned scopeRange() const;
	virtual bool isLarge() const;
	virtual void leave1();
	virtual TypePtr newScope(const char *sTypeName, SCOPE_enum e, const char *pFieldName, unsigned range, AttrScopeEnum) override;
	virtual TypePtr newScope(FieldPtr, SCOPE_enum e, const char *pFieldName, unsigned range, AttrScopeEnum) override;
	virtual TypesMgr_t &typesMgrNS(TypePtr) const;
	virtual MemoryMgr_t &memMgrNS() const;
};



class StrucvarIface_t : public I_Module
{
	StrucvarTracer0_t& mr;
public:
	StrucvarIface_t(StrucvarTracer0_t& r)
		: mr(r)
	{
	}
protected:
	virtual HTYPE	NewScope(const char* sTypeName, SCOPE_enum e, AttrScopeEnum a = AttrScopeEnum::null) override { return mr.NewScope(sTypeName, e, a); }
	virtual HTYPE	NewScope(HFIELD hField, SCOPE_enum e, AttrScopeEnum a = AttrScopeEnum::null) override { return mr.NewScope((FieldPtr)hField.pvt, e, a); }
	virtual void	Leave() override { mr.Leave(); }
	virtual bool	EnterScope(ADDR) override { return false; }
	virtual bool	EnterScope(HTYPE, ADDR) override { return false; }
	virtual bool	EnterSegment(HTYPE, ADDR) override { return false; }
	virtual HTYPE	traceOf(HTYPE) const override { assert(0); return nullptr; }
	virtual void	selectFile(const char *, const char*) override {}
	virtual void	installNamespace() override {}
	virtual	void	installTypesMgr() override {}
//	virtual bool	setDefaultCodeType(HTYPE) override { return false; }
	virtual unsigned	skip(int n) override { return mr.skip(n); }
	virtual unsigned	skipBits(int n) override { return mr.skipBits(n); }
	virtual POSITION	align(unsigned powerOf2) override { return mr.align(powerOf2); }
	virtual HTYPE	declTypedef(const char * pc, HTYPE t) override { return mr.declTypedef(pc, (TypePtr)t.pvt); }
	virtual HFIELD	declField(HNAME n, HTYPE t, AttrIdEnum a, ADDR at) override { return mr.DeclareField(n, (TypePtr)t.pvt, a, at); }
	virtual HFIELD	declField(HNAME n,  AttrIdEnum a, ADDR at) override { return mr.DeclareField(n, nullptr, a, at); }
	//?virtual HFIELD	instField(HNAME, HTYPE, AttrIdEnum) override { assert(0); return 0; }
	virtual HFIELD	declUField(HNAME n, HTYPE t, AttrIdEnum a) override { return mr.DeclareUnionField(n, (TypePtr)t.pvt, a); }
	virtual HFIELD	declUField(HNAME n, AttrIdEnum a) override { return mr.DeclareUnionField(n, nullptr, a); }
	virtual HFIELD	declBField(HNAME n, HTYPE t, AttrIdEnum a, ADDR at) override { return mr.DeclareBitField(n, (TypePtr)t.pvt, a, at); }
	virtual HFIELD	declEField(HNAME n/*, AttrIdEnum a*/) override { return mr.DeclareEnumField(n/*, a*/); }
	virtual HFIELD	declEField(HNAME n, ADDR va/*, AttrIdEnum a*/) override { return mr.DeclareEnumField(n, va/*, a*/); }
	virtual HFIELD	declCField(HNAME n, CODE_TYPE_e, AttrIdEnum) override { assert(0); return 0; }

	virtual HTYPE	type(OpType_t optype) override { return mr.type(optype); }
	virtual HTYPE	type(HNAME name) override { return mr.type(name); }
	virtual HTYPE	arrayOf(HTYPE t, unsigned num, bool bytes) override { return mr.arrayOf((TypePtr)t.pvt, num, bytes); }
	virtual HTYPE	arrayOfIndex(HTYPE t, HTYPE t2) override { return mr.arrayOfIndex((TypePtr)t.pvt, (TypePtr)t2.pvt); }
	virtual HTYPE	enumOf(HTYPE t, OpType_t t2) override { return mr.enumOf((TypePtr)t.pvt, t2); }
	virtual HTYPE	constOf(HTYPE t) override { return t; }
	virtual HTYPE	ptrOf(HTYPE t, PTR_TYPE_t pt, PTR_MODE_t pm) override { return mr.ptrOf((TypePtr)t.pvt, pt, pm); }
	virtual HTYPE	impOf(HTYPE) override { assert(0); return nullptr; }
	virtual HTYPE	expOf(HTYPE) override { assert(0); return nullptr; }
	virtual HTYPE	pairOf(HTYPE, HTYPE) override { assert(0); return nullptr; }
	virtual HTYPE	funcTypeOf(HTYPE, HTYPE, unsigned) override { assert(0); return nullptr; }
//	virtual HTYPE	code() override { return nullptr; }
	virtual POSITION	setcp(POSITION p) override { return mr.setcp(p); }
	virtual POSITION	cp() const override { return mr.cp(); }
	virtual void error(const char *pc) override { return mr.error(pc); }
	virtual void	blockSignals(bool) override {}
	virtual I_Front* frontend() const override { return mr.frontend(); }

protected:
	virtual size_t dataAt(OFF_t, OFF_t, PDATA) const override { assert(0);  return 0; }
	virtual OFF_t size() const override { assert(0); return 0; }
	virtual const I_AuxData *aux() const override {
		DataPtr pData(mr.ModuleRef().dataSourcePtr0());
		if (!pData)
			return nullptr;
		return pData->pvt().aux(); }
};