#pragma once

#include <map>
#include "qx/MyPath.h"
#include "qx/DynLib.h"

#include "shared/data_source.h"
#include "shared/misc.h"
#include "mem.h"
#include "types_mgr.h"
#include "info_proj.h"
#include "info_module.h"


class Main_t;
class Probe_t;

typedef std::map<std::string, I_DynamicType *>		FormatterMap;
typedef FormatterMap::iterator						CreateTypeMapIt;
typedef FormatterMap::const_iterator				CreateTypeMapCIt;

class FrontInfo_t
{
	MyPath fPath;
	HDYNLIB hModule;
	int		miRefs;//each load/unload affects this
public:
	MyString	sName;
	FormatterMap	mFmtFactory;//top level formatters
public:
	FrontInfo_t(const MyPath &path)
		: fPath(path),
		hModule(nullptr),
		miRefs(0)
	{
	}
	~FrontInfo_t()
	{
	}
	void setName(const char *s){ sName = s; }
	const MyString &name() const { return sName; }
	bool loadInitial(Main_t &);
	I_FrontMain *loadMain(Main_t &);
	HDYNFUNC loadSymbol(Main_t &, const char *);
	I_FrontMain *load(Main_t &);
	bool unload(Main_t &, bool = false);
	//I_Front *front(){ return pIFront; }
	const MyPath &path(){ return fPath; }
	HDYNLIB handle() const { return hModule; }
	bool storeDynamicTypeRef(const char *, I_DynamicType *);
	bool storeDynamicTypeRef0(const char *, I_DynamicType *);

	static MyString toFrontKey(const char *pc, const char *frontDll);
	static MyString fromFrontKey(const MyString &s, MyString &frontDll);
};

class FrontImplBase_t : public ModuleInfo_t
{
	ScopeStack_t	m_cont;//containers stack
	MemoryMgr_t		mMemMgrNS;//for non-serializable entities (such as strucvar's contents)
	TypesMgr_t		mTypesMgrNS;
	MyString		mLastError;
	//bool			mbInstMode;//instantiating a strucvar mode
	bool			mbSignalsEnabled;
public:
	FrontImplBase_t(Project_t &, TypePtr);
	~FrontImplBase_t();

	TypePtr	NewScopeImpl0(SCOPE_enum, Locus_t&, MyString& sTypeName = MyString());
	TypePtr	NewBitsetImpl(Locus_t&);
	TypePtr NewBitsetImpl();
	//TypePtr NewProcImpl(Locus_t&);
	TypePtr	NewStrucvarImpl(TypePtr);
	TypePtr	NewScopeImpl(const char *, SCOPE_enum);
	TypePtr	NewScopeImpl(FieldPtr, SCOPE_enum);
	TypePtr	NewTypedefImpl(const char *name, TypePtr type);
	void	LeaveImpl();
	bool	EnterScopeImpl(ADDR);
	bool	EnterScopeImpl(TypePtr, ADDR);
	bool	EnterSegmentImpl(TypePtr, ADDR);
	TypePtr	TraceOf(TypePtr) const;
	void	selectFileImpl(const char *, const char *folder);
	void	installNamespaceImpl();
	void	installTypesMgrImpl();
	FieldPtr	declFieldImpl(const MyStringEx&, TypePtr, AttrIdEnum);
	FieldPtr	declUnifieldImpl(const MyStringEx&, TypePtr, AttrIdEnum);
	FieldPtr	declBitfieldImpl(HNAME, TypePtr, AttrIdEnum, ADDR at);
	FieldPtr	declEnumfieldImpl(HNAME/*, AttrIdEnum*/, I_Module&);
	FieldPtr	declEnumfieldImpl(HNAME, ADDR/*, AttrIdEnum*/, I_Module&);
	HFIELD		declCodeFieldImpl(HNAME, I_Module::CODE_TYPE_e, AttrIdEnum, I_Module&);
	TypePtr	typeImpl(OpType_t);
	TypePtr	typeImpl(HNAME);
	TypePtr	arrayOfImpl(TypePtr, unsigned, bool bytes);
	TypePtr	arrayOfIndexImpl(TypePtr, TypePtr);
	TypePtr	constOfImpl(TypePtr);
	TypePtr	enumOfImpl(TypePtr, int);
	TypePtr	ptrOfImpl(TypePtr, I_Module::PTR_TYPE_t, I_Module::PTR_MODE_t);
	TypePtr	impOfImpl(TypePtr);
	TypePtr	expOfImpl(TypePtr);
	TypePtr	pairOfImpl(TypePtr l, TypePtr r);
	TypePtr	funcOfImpl(TypePtr r, TypePtr a, unsigned f);
	POSITION	setcpImpl(POSITION);
	POSITION	cpImpl() const;
	OFF_t	cprImpl() const;
	void errorImpl(const char *);
	unsigned	skipImpl(int);
	unsigned	skipBitsImpl(int);
	POSITION	alignImpl(unsigned);

	void blockSignals(bool b){ mbSignalsEnabled = !b; }
	const MyString &getLastError() const { return mLastError; }
	void closeBitset() {
		if (scope().cont()->typeBitset())
			popScope();
	}
protected:
	const ScopeStack_t& scopeStack() const { return m_cont; }
	frame_t& pushScope(TypePtr p, ADDR a, bool b) { return m_cont.push_scope(p, a, b); }
	void popScope(bool bForceNoAdvance = false) {
//CHECK(m_cont.size() == 2)
//STOP
		m_cont.pop_scope(bForceNoAdvance);
	}
	bool isScopeEmpty() const { return m_cont.empty(); }
	const frame2_t &scope() const { return m_cont.back(); }
	frame2_t &scope(){ return m_cont.back(); }
	frame2_t &scope(size_t up){
		ScopeStack_t::reverse_iterator it(m_cont.rbegin());
		while (up--)
			it++;
		return *it;
	}
	void updateFrameSize(TypePtr);
	const MyString &signalError(const char *fmt, ...);
	static void printError(const MyString &);
	std::ostream& printError() const;
	std::ostream& printWarning() const;
	FullName_t hyperLinked(const FullName_t&) const;
	POSITION	fp(TypePtr) const;//frame position
	FieldPtr	declUnionField(const MyStringEx& name, TypePtr iType, AttrIdEnum attr);
};


//template <typename T_MAIN>//T_MAIN must be derived from I_MAin
template <typename T>
class IFront_Base : public T
{
	FrontImplBase_t& mr;
#if(F_DEBUG)
public:
	OFF_t _FCP;
#endif
public:
	IFront_Base(FrontImplBase_t& r)
		: mr(r)
	{
	}
	//I_Module
	virtual HTYPE	NewScope(const char *pTypeName, SCOPE_enum eScope, AttrScopeEnum) override { return mr.NewScopeImpl(pTypeName, eScope); }
	virtual HTYPE	NewScope(HFIELD hField, SCOPE_enum eScope, AttrScopeEnum) override { return mr.NewScopeImpl((FieldPtr)hField.pvt, eScope); }
	virtual void	Leave() override { mr.LeaveImpl(); }
	virtual bool	EnterScope(ADDR va) override { return mr.EnterScopeImpl(va); }
	virtual bool	EnterScope(HTYPE t, ADDR a) override { return mr.EnterScopeImpl((TypePtr)t.pvt, a); }
	virtual bool	EnterSegment(HTYPE type, ADDR va) override { return mr.EnterSegmentImpl((TypePtr)type.pvt, va); }
	virtual void	selectFile(const char *path, const char *folder) override { mr.selectFileImpl(path, folder); }
	virtual void	installNamespace() override { mr.installNamespaceImpl(); }
	virtual	void	installTypesMgr() override { mr.installTypesMgrImpl(); }

	virtual HFIELD	declField(HNAME name, HTYPE type, AttrIdEnum attr, ADDR at) override { 
		TypePtr pType((TypePtr)type.pvt);
		MyStringEx aName(name);
		assert(aName.size() == 1);
		mr.closeBitset();
		if (at != -1)
			mr.setcpImpl(at);
		HFIELD h(mr.declFieldImpl(aName, pType, attr));
		F_CHECK(mr.cprImpl());
		return h;
	}
	virtual HFIELD	declUField(HNAME name, HTYPE type, AttrIdEnum attr) override {
		TypePtr pType((TypePtr)type.pvt);
		MyStringEx aName(name);
		assert(aName.size() == 1);
		mr.closeBitset();
		HFIELD h(mr.declUnifieldImpl(aName, pType, attr));
		F_CHECK(mr.cprImpl());
		return h;
	}
	virtual HFIELD	declUField(HNAME name, AttrIdEnum attr) override {
		assert(0);
		return 0;
	}
	virtual HFIELD	declField(HNAME, AttrIdEnum, ADDR) override
	{
		assert(0);
		return 0;
	}
	virtual HFIELD	declBField(HNAME name, HTYPE type, AttrIdEnum attr, ADDR at) override {
		HFIELD h(mr.declBitfieldImpl(name, (TypePtr)type.pvt, attr, at));
		F_CHECK(mr.cprImpl());
		return h;
	}
	virtual HFIELD	declEField(HNAME name) override {//, AttrIdEnum attr){
		HFIELD h(mr.declEnumfieldImpl(name/*, attr*/, *this));
		F_CHECK(mr.cprImpl());
		return h;
	}
	virtual HFIELD	declEField(HNAME name, ADDR val) override {//, AttrIdEnum attr){
		HFIELD h(mr.declEnumfieldImpl(name, val/*, attr*/, *this));
		F_CHECK(mr.cprImpl());
		return h;
	}
	virtual HFIELD	declCField(HNAME name, I_Module::CODE_TYPE_e eType, AttrIdEnum eAttr) override { return mr.declCodeFieldImpl(name, eType, eAttr, *this); }

	virtual HTYPE	traceOf(HTYPE seg) const override { return mr.TraceOf((TypePtr)seg.pvt); }
	virtual HTYPE	declTypedef(const char *name, HTYPE type) override { return mr.NewTypedefImpl(name, (TypePtr)type.pvt); }
	virtual HTYPE	type(OpType_t typeID) override { return mr.typeImpl(typeID); }
	virtual HTYPE	type(HNAME name) override { return mr.typeImpl(name); }
	virtual HTYPE	arrayOf(HTYPE t, unsigned n, bool bytes) override { return mr.arrayOfImpl((TypePtr)t.pvt, n, bytes); }
	virtual HTYPE	arrayOfIndex(HTYPE t, HTYPE t2) override { return mr.arrayOfIndexImpl((TypePtr)t.pvt, (TypePtr)t2.pvt); }
	virtual HTYPE	enumOf(HTYPE t, OpType_t e) override { return mr.enumOfImpl((TypePtr)t.pvt, e); }
	virtual HTYPE	constOf(HTYPE t) override { return mr.constOfImpl((TypePtr)t.pvt); }
	virtual HTYPE	ptrOf(HTYPE t, I_Module::PTR_TYPE_t eType, I_Module::PTR_MODE_t eMode) override { return mr.ptrOfImpl((TypePtr)t.pvt, eType, eMode); }
	virtual HTYPE	impOf(HTYPE t) override { return mr.impOfImpl((TypePtr)t.pvt); }
	virtual HTYPE	expOf(HTYPE t) override { return mr.expOfImpl((TypePtr)t.pvt); }
	virtual HTYPE	pairOf(HTYPE l, HTYPE r) override { return mr.pairOfImpl((TypePtr)l.pvt, (TypePtr)r.pvt); }
	virtual HTYPE	funcTypeOf(HTYPE r, HTYPE a, unsigned f) override { return mr.funcOfImpl((TypePtr)r.pvt, (TypePtr)a.pvt, f); }
	virtual POSITION	setcp(POSITION p) override { return mr.setcpImpl(p); }
	virtual POSITION	cp() const override { return mr.cpImpl(); }
	virtual OFF_t		cpr() const override { return mr.cprImpl(); }
	virtual void	error(const char *msg) override { mr.errorImpl(msg); }
	virtual void	blockSignals(bool b) override { mr.blockSignals(b); }
	virtual unsigned	skip(int bytes) override { return mr.skipImpl(bytes); }
	virtual unsigned	skipBits(int bits) override { return mr.skipBitsImpl(bits); }
	virtual POSITION	align(unsigned powerOf2) override { return mr.alignImpl(powerOf2); }
};





//fronted parsing
/////////////////////////////////////////////////////FrontImpl_t
class FrontImpl_t : public FrontImplBase_t//IFront_Base//<I_SuperModule>
{
	const Locus_t		&mrLocusz;
	bool		mbGeomChanged;
	MyString	mOptions;

	SlotVector<PatchList_t>	mPatchies;
public:
	FrontImpl_t(Project_t&, const Locus_t &, TypePtr);
	~FrontImpl_t();
	void setOptions(MyString s) { mOptions = s; }
	FieldPtr field() const;
	//const MyString &getLastError() const { return getLastError(); }
	bool geomChanged() const { return mbGeomChanged; }
	bool		InstantiateType(FieldPtr, bool bUnnamed, I_Module&);

	friend class FrontIface_t;
protected:
	bool RegisterFormatterType(const char*) { return false; }
	bool DeclareContextDependentType(const char *);
	bool DeclareCodeType(const char *);

	FieldPtr	declField(HNAME, TypePtr, AttrIdEnum, ADDR at, I_Module&);
	FieldPtr	instField(HNAME, TypePtr, AttrIdEnum, I_Module&);//{ return instFieldImpl(name, _type, attr); }

	FieldPtr	declUField(HNAME, TypePtr, AttrIdEnum);
	FieldPtr	declUField(HNAME, SCOPE_enum, AttrIdEnum);

	TypePtr	NewRangeSet(ADDR64 base64, const char *name);
	TypePtr	NewSegment(unsigned sizeP, const char *name, unsigned flags);
	TypePtr	AddSubRange(TypePtr hRangeSet, ADDR addrV, ADDR_RANGE sizeV, TypePtr hSeg);
	
	bool	EnterAttic();

	void	installFrontend(const char *, int id);
	I_Front* frontendImpl() const;
	void	reuseFrontend(int id);
	bool	setDefaultCodeType(TypePtr);
	//void	addFileView(const char *);

	TypePtr	code();
	void	setEndianness(bool lsb);
	//bool	enqueEntryPoint(ADDR);
	void	setAuxData(PDATA, size_t);
	bool	setSize(unsigned);

	HPATCHMAP	newPatchMap();
	bool		addPatch(HPATCHMAP, OFF_t offs, unsigned size);
	I_DataSourceBase	*stitchDerivativeModule(HPATCHMAP, const char *name, const char *type, bool bDelayed, int moduleId);
	I_DataSourceBase	*module(const char *);
	I_DataSourceBase	*module(int moduleId);
	bool preformatModule(int moduleId, const char *typeStr, bool delayed);

protected:
	//I_DataSourceBase
	size_t dataAt(OFF_t off, OFF_t siz, PDATA) const;
	OFF_t size() const {
		return ModuleRef().size(ModulePtr());
	}
		//return mrLocusz.back().m_size; }
	const I_AuxData *aux() const {
		DataPtr pData(ModuleRef().dataSourcePtr0());
		if (!pData)
			return nullptr;
		return pData->pvt().aux(); }//???
	void clearAuxData(){
		if (ModuleRef().dataSourcePtr0())
			ModuleRef().dataSourcePtr0()->pvt().clearAuxData();
	}
	const I_DataSourceBase *host() const;
	I_DataSourceBase *openFile(const char *, bool) const;

private:
	//FieldPtr declFieldImpl2(const MyStringEx&, TypePtr, AttrIdEnum, ADDR at, I_Module&);
	void	declStrucvar(TypePtr, I_Module&);
	
private:
	DataPtr dataHost() const;
};




class FrontIface_t : public IFront_Base<I_SuperModule>
{
	FrontImpl_t& mr;
public:
	FrontIface_t(FrontImpl_t& r)
		: IFront_Base<I_SuperModule>(r),
		mr(r)
	{
	}

protected:
	virtual bool RegisterFormatterType(const char* pc) override { return mr.RegisterFormatterType(pc); }
	virtual bool DeclareContextDependentType(const char *pc) override { return mr.DeclareContextDependentType(pc); }
	virtual bool DeclareCodeType(const char *pc) override { return mr.DeclareCodeType(pc); }

	virtual HFIELD	declField(HNAME n, HTYPE t, AttrIdEnum a, ADDR at) override {
		return mr.declField(n, (TypePtr)t.pvt, a, at, *this);
	}
	virtual HFIELD	declField(HNAME n, AttrIdEnum a, ADDR at) override {
		return mr.declField(n, nullptr, a, at, *this);
	}
	virtual HFIELD	instField(HNAME n, HTYPE t, AttrIdEnum a) override { return mr.instField(n, (TypePtr)t.pvt, a, *this); }

	virtual HFIELD	declUField(HNAME n, HTYPE t, AttrIdEnum a) override { return mr.declUField(n ,(TypePtr)t.pvt, a); }
	virtual HFIELD	declUField(HNAME n, AttrIdEnum a) override { return mr.declUField(n, nullptr, a); }


	virtual HTYPE	NewRangeSet(ADDR64 base64, const char *name) override { return mr.NewRangeSet(base64, name); }
	virtual HTYPE	NewSegment(unsigned sizeP, const char *name, unsigned flags) override { return mr.NewSegment(sizeP, name, flags); }
	virtual HTYPE	AddSubRange(HTYPE hRangeSet, ADDR addrV, ADDR_RANGE sizeV, HTYPE hSeg) override { return mr.AddSubRange((TypePtr)hRangeSet.pvt, addrV, sizeV, (TypePtr)hSeg.pvt); }
	
	virtual bool	EnterAttic() override { return mr.EnterAttic(); }

	virtual void	installFrontend(const char *pc, int id) override { return mr.installFrontend(pc, id); }
	virtual I_Front* frontend() const override { return mr.frontendImpl(); }
	virtual void	reuseFrontend(int id) override { return mr.reuseFrontend(id); }
	virtual bool	setDefaultCodeType(HTYPE t) override { return mr.setDefaultCodeType((TypePtr)t.pvt); }
	//virtual void	addFileView(const char *pc) override { mr.addFileView(pc); }

	virtual HTYPE	code() override { return mr.code(); }
	virtual void	setEndianness(bool lsb) override { return mr.setEndianness(lsb); }
	//bool	enqueEntryPoint(ADDR a) override { return mr.enqueEntryPoint(a); }
	virtual void	setAuxData(PDATA p, size_t s) override { return mr.setAuxData(p, s); }
	virtual bool	setSize(unsigned l) override { return mr.setSize(l); }

	virtual HPATCHMAP	newPatchMap() override { return mr.newPatchMap(); }
	virtual bool		addPatch(HPATCHMAP h, OFF_t offs, unsigned size) override { return mr.addPatch(h, offs, size); }
	virtual I_DataSourceBase	*stitchDerivativeModule(HPATCHMAP h, const char *name, const char *type, bool bDelayed, int moduleId) override { return mr.stitchDerivativeModule(h, name, type, bDelayed, moduleId); }
	virtual I_DataSourceBase	*module(const char *pc) override { return mr.module(pc); }
	virtual I_DataSourceBase	*module(int moduleId) override { return mr.module(moduleId); }
	virtual bool preformatModule(int moduleId, const char *typeStr, bool delayed) override { return mr.preformatModule(moduleId, typeStr, delayed); }

protected:
	//I_DataSourceBase
	virtual size_t dataAt(OFF_t off, OFF_t siz, PDATA p) const override { return mr.dataAt(off, siz, p); }
	virtual OFF_t size() const override { return mr.size(); }

	virtual const I_AuxData *aux() const override { return mr.aux(); }
	virtual void clearAuxData() override { return mr.clearAuxData(); }
	virtual const I_DataSourceBase *host() const override { return mr.host(); }
	virtual I_DataSourceBase *openFile(const char *pc, bool b) const override { return mr.openFile(pc, b); }
};







/////////////////////////////////////////////////////FrontImplEx_t (formatters support)
class FrontImplEx_t : public FrontImpl_t
{
	FrontInfo_t& mrFront;
public:
	FrontImplEx_t(Project_t& rProj, const Locus_t& rLoc, FrontInfo_t& rFI, TypePtr pModule)
		: FrontImpl_t(rProj, rLoc, pModule),
		mrFront(rFI)
	{
	}
protected:
	friend class FrontIfaceEx_t;
	bool RegisterFormatterType(const char *);
};




class FrontIfaceEx_t : public FrontIface_t
{
	FrontImplEx_t& mr;
public:
	FrontIfaceEx_t(FrontImplEx_t& r)
		: FrontIface_t(r),
		mr(r)
	{
	}
protected:
	virtual bool RegisterFormatterType(const char* pc) { return mr.RegisterFormatterType(pc); }
};




#define	ORDINAL_BIAS	0
#define	ORDINAL_NULL	0xFFFF
#define	NULL_ORDINAL_BIAS	0x10000

#define NO_TRY	0
#if(!NO_TRY)
#define TRY try
#define CATCH(arg)	catch(arg)
#else
#define TRY	if(1)
#define CATCH(arg) else
#endif

