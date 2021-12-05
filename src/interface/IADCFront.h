#pragma once

#include <set>

#include "qx/IUnk.h"
#include "shared/defs.h"
#include "shared/misc.h"
#include "shared/action.h"
#include "IADCMain.h"

//#define FRONT_NAME	"front.dll"
#define FRONT_NAME	"komok"

struct ins_t;
struct FE_t;
class I_Module;
class MyStreamBase;
class I_ModuleCB;
class IPCode_t;

struct RegInfo_t
{
	const char *name;
	unsigned	opc;
	unsigned	opsz;
	unsigned	offs;
};

struct reg_name_compare {
	bool operator() (const RegInfo_t* lhs, const RegInfo_t* rhs) const {
		return STRICMP(lhs->name, rhs->name) < 0;
	}
};

struct reg_compare {
	bool operator() (const RegInfo_t* lhs, const RegInfo_t* rhs) const {
		if (lhs->opc < rhs->opc)
			return true;
		if (lhs->opc > rhs->opc)
			return false;
		if (lhs->offs < rhs->offs)
			return true;
		if (lhs->offs > rhs->offs)
			return false;
		return lhs->opsz < rhs->opsz;
	}
};

typedef std::multiset<const RegInfo_t *, reg_name_compare>	S2R_t;	//some reg/flag names may collide

class R2S_t : public std::set<const RegInfo_t *, reg_compare>
{
	typedef std::set<const RegInfo_t *, reg_compare> base;
public:
	const RegInfo_t *find(unsigned opc, unsigned offs) const {
		RegInfo_t r = { nullptr, (unsigned)opc, 0, offs };
		const_iterator i(base::lower_bound(&r));
		if (i == end())
			return nullptr;
		return (*i);
	}
	const RegInfo_t *find(unsigned opc, unsigned offs, unsigned opsz) const {
		RegInfo_t r = { nullptr, (unsigned)opc, opsz, offs };
		const_iterator i(base::find(&r));
		if (i == end())
			return nullptr;
		return (*i);
	}
};

class RegInfoLookup_t : private S2R_t
{
	R2S_t m_rev;
public:
	RegInfoLookup_t(){}
	RegInfoLookup_t(const RegInfo_t *table)
	{
		create(table);
	}
	void create(const RegInfo_t *table)
	{
		assert(empty());
		for (const RegInfo_t *p(table); p->name != nullptr; p++)
		{
			insert(p);
			if (!m_rev.insert(p).second)
				ASSERT0;
		}
	}
	const char *toRegName(OPC_t opc, uint8_t ofs, uint8_t opsz) const {
		const RegInfo_t *p(m_rev.find(opc, ofs, opsz));
		if (p)
			return p->name;
		return nullptr;
	}
	const R2S_t &rev() const { return m_rev; }
	const RegInfo_t *fromReg(OPC_t opc, unsigned ofs, unsigned opsz) const {
		return m_rev.find(opc, ofs, opsz);
	}
	const RegInfo_t *fromReg(OPC_t opc, unsigned ofs) const {//don't care abou size
		return m_rev.find(opc, ofs);
	}
	const RegInfo_t *fromRegName(const char *s, OPC_t opc) const {
		RegInfo_t k = { s };
		for (S2R_t::const_iterator i(lower_bound(&k)); i != end(); ++i)
		{
			if (STRICMP(s, (*i)->name) != 0)
				break;
			if (opc == OPC_NULL || opc == OPC_t((*i)->opc))
				return *i;
		}
		return nullptr;
	}
	void clear()
	{
		m_rev.clear();
		S2R_t::clear();
	}
};

class I_FrontDC : public My::IUnk
{
public:
	I_FrontDC(){}
	virtual ~I_FrontDC(){}

	virtual const char *Name() const = 0;

	virtual int PtrSize() const { return 0; }
	virtual int PtrNear() const { return 1; }
	virtual int PtrFar() const { return 4; }
	virtual int	StackAddrSize() const { return 4; }

	//virtual void SetDefSavedRegs(unsigned sr){}
	virtual const FE_t * GetFE() const { return nullptr; }
	//virtual int Str2Reg(const char *) const { return 0; }
	virtual const char **Symbols() const { return nullptr; }
	virtual const RegInfo_t *RegInfoTable() const { return nullptr; }

	virtual const char *RegName(SSID_t, int /*sz*/, int /*ofs*/, int /*flags*/, char[80]) const { return nullptr; }

	//CC support
	enum { F_ThisCall = 1, F_FastCall = 2 };
	virtual SSID_t ArgSSID(unsigned index, unsigned fflags) const { return SSID_NULL; }
	virtual SSID_t RetSSID(unsigned index, OpType_t retType) const { return SSID_NULL; }
	virtual unsigned RetOff(unsigned index, OpType_t retType) const { return 0; }
	virtual unsigned ArgOff(unsigned index, unsigned fflags) const { return 0; }
	virtual unsigned ArgSize(unsigned index, unsigned u) const { return 0; }
};


class I_DebugInfoDumper : public My::IUnk
{
public:
	//virtual bool processNextType(I_ModuleCB &, unsigned &progress) = 0;
	virtual bool processNext(I_ModuleCB &, unsigned &progress) = 0;
protected:
	I_DebugInfoDumper(){}
	virtual ~I_DebugInfoDumper(){}
};

////////////////////////////////// I_Front
class I_Front
{
public:
	virtual ~I_Front(){}
	virtual void release(){ delete this; }
	virtual I_FrontDC *createFrontDC() const { return nullptr; }

	typedef bool (*PFDumpResCallback)(size_t level, const char *name, ADDR va, unsigned dataSize, void *pvUser);
	virtual void dumpResources2(PFDumpResCallback, void * /*pvUser*/, const char * /*rootName*/ = nullptr){}
	virtual void dumpResources(MyStreamBase &, const char * = nullptr){}

	enum Dump_IE_Flags { DUMP_IE_ALL, DUMP_IE_NAMED_ONLY, DUMP_IE_MODULES };

	enum SymbolKind
	{
		SYMK_NULL,
		SYMK_OK,
		SYMK_FUNC,
		SYMK_THUNK,
		SYMK_SPECIAL = 0x10,
		SYMK_VFTABLE,
		SYMK_VBTABLE,	//m$-specific
		SYMK_LVFTABLE,	//m$-specific
		SYMK_CVFTABLE,	//construction vftable (gcc)
		SYMK_TYPEINFO,
		// *** m$-specific
		SYMK_RTTI_TD,	// type descriptor
		SYMK_RTTI_BCD,	// base class descriptor
		SYMK_RTTI_CHD,	// class hierarchy descriptor
		SYMK_RTTI_COL,	// complete object locator
		SYMK_RTTI_BCA,	// base class array
		//---
		SYMK_STRING,
		SYMK_VFTABLE_ADDRP = 0x100,	// requires a special construction(!)
			//Itanium API (c): In general, what we consider the address of a virtual table (i.e. the address contained in objects pointing to a virtual table)
			//may not be the beginning of the virtual table. We call it the address point of the virtual table.
			//The virtual table may therefore contain components at either positive or negative offsets from its address point.
		SYMK__LAST
	};

	class I_DumpSymbol
	{
	public:
		virtual void setVA(ADDR) = 0;
		virtual void setKind(SymbolKind) = 0;
		virtual void setName(const char*) = 0;
		virtual void setName(OFF_t) = 0;
		//virtual void setTag(const char*) = 0;
		//virtual void setTag(OFF_t) = 0;
		//virtual void setModule(const char*) = 0;
		//virtual void setModule(OFF_t) = 0;
		//virtual void setOrdinal(unsigned) = 0;
		virtual void flush() = 0;
	};

	virtual void dumpImports(I_DumpSymbol*, Dump_IE_Flags){}
	virtual void dumpExports(I_DumpSymbol*, Dump_IE_Flags){}

	virtual I_DebugInfoDumper *createDebugInfoDumper(const char *){ return nullptr; }

	virtual void dumpDebugInfo(I_ModuleCB::Dump_D_Flags, I_ModuleCB &){}

	enum AKindEnum { AKIND_NULL, AKIND_RAW, AKIND_VA, AKIND_MODULE_MASK = 0x7FFF, AKIND_OTHER = 0x8000 };
	virtual AKindEnum translateAddress(const I_DataStreamBase &, int /*moduleTag*/, ADDR &, AttrIdEnum){ return AKIND_NULL; }

	virtual ADDR getImportPtrByName(const char *, OFF_t *) const { return 0; }
	virtual bool getImportInfo(ADDR, I_DumpSymbol*) const { return false; }

	virtual bool getExportInfo(const char *, unsigned, I_DumpSymbol*) const { return false; }

	virtual SymbolKind demangleName(const char *, MyStreamBase &){ return I_Front::SYMK_NULL; }

	class IDumpClassHierarchy
	{
	public:
		virtual bool add(int level, OFF_t name, ADDR vaBCD, unsigned offset, bool bVirtual/*int mdisp, int pdisp, int vdisp, unsigned attrib*/) = 0;
	};
	enum RTTI_Method { RTTI_NULL, RTTI_GCC, RTTI_GCC_SI, RTTI_GCC_VMI, RTTI_MSVC };
	virtual bool dumpClassHierachy(IDumpClassHierarchy*, ADDR, RTTI_Method, ADDR) { return false; }
	virtual bool getEntryVA(ADDR &){ return false; }

	virtual bool processSpecial(I_Module&, I_Front::SymbolKind, const char*) { return false; }
};


class I_FrontMain
{
public:
	I_FrontMain(){}
	virtual const char *name() const = 0;
	virtual void RegisterTypes(I_ModuleEx &) = 0;
	virtual const char * RecognizeFormat(const I_DataSourceBase &, const char *ext) = 0;
	virtual I_Front *CreateFrontend(const char *, const I_DataSourceBase *, const I_Main*) = 0;
};



#ifdef WIN32
#define ADCFRONT_EXPORT __declspec(dllexport)
#define ADCFRONT_EXTERN extern
#else//WIN32
#define ADCFRONT_EXPORT
#define ADCFRONT_EXTERN extern __attribute__((visibility("default")))
#endif//WIN32

//#define __CDT(name)	DynamicType_##name
	//context-dependet type

#define DECLARE_DYNAMIC_TYPE(type,name)			extern "C" { ADCFRONT_EXTERN ADCFRONT_EXPORT type imp_##name; type imp_##name; }
#define DECLARE_DYNAMIC_TYPE1(type,name,args)	extern "C" { ADCFRONT_EXTERN ADCFRONT_EXPORT type imp_##name; type imp_##name (args); }
//#define _PFX(arg)	EXSYM_PFX arg
#define _PFX(arg)	FENAME_PFX FRONT_NAME EXSYM_PFX arg

#define BEGIN_DYNAMIC_TYPE_0(ARG, BASE)	\
	class DynamicType_##ARG : public BASE { \
	static const char *pcName; \
	typedef	BASE	Super; \
public: \
	DynamicType_##ARG(){} \
	virtual const char *name() const { return pcName; } \
	virtual void createz(I_Module &mr, unsigned long dataSize)

#define BEGIN_DYNAMIC_TYPE(ARG)	BEGIN_DYNAMIC_TYPE_0(ARG, I_DynamicType)
	
#define END_DYNAMIC_TYPE(ARG) \
	}; const char *DynamicType_##ARG::pcName = _PFX(#ARG); \
	DECLARE_DYNAMIC_TYPE(DynamicType_##ARG, ARG);


#define DECLARE_FORMATTER(type,name)		DECLARE_DYNAMIC_TYPE(type, name)
#define DECLARE_FORMATTER1(type,name,args)	DECLARE_DYNAMIC_TYPE1(type, name, args)
#define DECLARE_CODE_TYPE(type,name)		DECLARE_DYNAMIC_TYPE(type, name)


#define	EXSYM_PFX	"/"
#define	FENAME_PFX	"~"

#define FRONT_NAME_PFX	FENAME_PFX FRONT_NAME

class I_DynamicType
{
public:
	virtual const char *name() const = 0;
	virtual void createz(I_Module &, unsigned long nSize) = 0;
};

class I_FormatterType
{
public:
	virtual const char *name() const = 0;
	virtual void createz(I_SuperModule &, unsigned long nSize) = 0;
};

struct ins_desc_t
{
	enum { CT_NONE, CT_JUMP, CT_CALL, CT_RET };
	enum { FB_NONE, FB_JUMP, FB_RET, FB_NRET_CALL };

	ins_desc_t()
		: imageBase(0),
		vaCur(0),
		vaRef(0),
		length(0),
		opSize(0),
		addrSize(0),
		indirectAccess(0),
		controlTransfer(CT_NONE),
		flowBreak(FB_NONE)
	{
	}

	ADDR64	imageBase;
	ADDR	vaCur;
	ADDR	vaRef;
	uint8_t	length;
	uint8_t	opSize;
	uint8_t	addrSize;
	uint8_t	indirectAccess;//:1;//when 
	uint8_t	controlTransfer;//:2;//CT_JUMP:any jumps (conditional or not), CT_CALL:call, CT_RET:ret(?)
	uint8_t	flowBreak;//:2;//FB_JUMP:jmp, FN_RET:ret, FB_NRET_CALL: nonreturning call(?)

	bool isIndirect() const {
		return indirectAccess != 0;
	}
	bool isDirect() const {
		return !isIndirect() && vaRef != 0;//opSize != 0;//not a register
	}
	//control
	void setControlTransfer(uint8_t u){
		controlTransfer = u;
	}
	bool isControlTransfer() const {
		return controlTransfer != CT_NONE;
	}
	bool isJump() const {//any
		return controlTransfer == CT_JUMP;
	}
	bool isUnconditionalJump() const {
		return controlTransfer == CT_JUMP && isFinalized();
	}
	bool isConditionalJump() const {
		return controlTransfer == CT_JUMP && !isFinalized();
	}
	bool isCall() const {
		return controlTransfer == CT_CALL;
	}
	//flow break
	void setFlowBreak(uint8_t u){
		flowBreak = u;
	}
	bool isFinalized() const {
		return flowBreak != FB_NONE;
	}
	bool isGoto() const {
		return flowBreak == FB_JUMP;
	}
	void setRefVA(ADDR va, bool bIndir = false){
		vaRef = va;
		indirectAccess = bIndir ? 1 : 0;
	}
	void setOpSize(uint8_t u){
		opSize = u;
	}
	ADDR64	vaCur64() const { return imageBase + vaCur; }
};

//all FEs use this as a medium to produce IR

typedef void *	HINS;
typedef void *	HOPND;

struct INS_pvt_t
{
	int			stackdiff;
	unsigned	cpuflags;
	unsigned	cpuflags_tested;
	int			fpudiff;
	unsigned	fpuflags;
	INS_pvt_t()
		: stackdiff(0),
		cpuflags(0),
		cpuflags_tested(0),
		fpudiff(0),
		fpuflags(0)
	{
	}
	INS_pvt_t(const INS_pvt_t &o)
		: stackdiff(o.stackdiff),
		cpuflags(o.cpuflags),
		cpuflags_tested(o.cpuflags_tested),
		fpudiff(o.fpudiff),
		fpuflags(o.fpuflags)
	{
	}
};

class IPCode_t
{
public:
	IPCode_t(){}
	virtual ~IPCode_t(){}

	virtual HINS newins(Action_t action, const INS_pvt_t &, bool bPrepend) = 0;
	virtual unsigned insCount() const = 0;
	virtual HOPND newop(HINS, int c, int t, int o) = 0;
	virtual HOPND newopp(HINS, int c, int t, int64_t o) = 0;
	virtual HOPND newop(HINS) = 0;
	virtual HOPND firstOp(HINS) const = 0;
	virtual HOPND lastOp(HINS) const = 0;
	virtual void setAction(HINS, Action_t) = 0;
	virtual Action_t getAction(HINS) const = 0;

	virtual OPC_t opClass(HOPND) const = 0;
	virtual void setScalar(HOPND, int) = 0;
	virtual int getScalar(HOPND) = 0;
	virtual void setOpType(HOPND, uint8_t) = 0;
	virtual uint8_t opType(HOPND) const = 0;
	virtual void setDPtr(HOPND, int) = 0;
	virtual void setOpName(HOPND, const char *) = 0;
	virtual bool isInvalid() const = 0;
};

class I_Code
{
public:
	virtual int Unassemble(DataStream_t &, ADDR64 base, ADDR va, ins_desc_t &) = 0;
	virtual int Print(DataStream_t &, ADDR64 base, ADDR va, IOutpADDR2Name *cb, ins_desc_t &) = 0;
	virtual int Skip(DataStream_t &, ADDR64 base, ADDR va, ins_desc_t &) = 0;
	virtual int Generate(DataStream_t &, ADDR64 base, ADDR va, ins_desc_t &, IPCode_t &, const I_FrontDC &) = 0;
	virtual ~I_Code(){}
};


class __SafeScope
{
	I_Module	&mr;
public:
	__SafeScope(I_Module &r)
		: mr(r)
	{
	}
	~__SafeScope()
	{
		mr.Leave();
	}
};

#define SAFE_SCOPE_HERE(arg)	__SafeScope here(arg)

#define ENTER_PORTAL(core, va) \
	if (EnterSegment(va)) { try	//the block follows

#define ENTER_SCOPE(core, va) \
	if (core.EnterScope(va)) { try	//the block follows

#define EXIT_SCOPE(core) \
	catch (int) {} \
	catch (const std::exception &){} \
	core.Leave(); }

//simple types
#define DECLDATA(type, name)	DataFetch_t<type> name(mr, mr.cpr())
#define DECLDATA2(type, name, offs)	DataFetch_t<type> name(mr, mr.cpr() + offs)

//compound types
#define DECLDATAEX(type, name)	DataFetch2_t<type> name(mr, mr.cpr())
#define DECLDATAEX2(type, name, offs)	DataFetch2_t<type> name(mr, mr.cpr() + offs)

//ptr for simple types
#define DECLDATAPTR(type, name)	DataFetchPtr_t<type> name(mr, mr.cpr())
#define DECLDATAPTR2(type, name, offs)	DataFetchPtr_t<type> name(mr, mr.cpr() + offs)

//ptr for compound types
#define DECLDATAPTREX(type, name)	DataFetchPtr2_t<type> name(mr, mr.cpr())

//#define DECLDATAOFFS(type, name, offs)	type &name(*(type *)((char *)mr.data() + offs))

//#define	SYMBOL_TAG_SEPARATOR	"@"

