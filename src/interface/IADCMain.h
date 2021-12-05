#pragma once

#include <string>
#include <sys/types.h>
//#include "shared/defs.h"
#include "shared/data_source.h"


struct HTYPE {
	void* pvt;
	HTYPE() : pvt(nullptr) {}
	HTYPE(void *p) : pvt(p) {}
	operator bool() const { return pvt != nullptr; }
};
struct HFIELD {
	void* pvt;
	HFIELD() : pvt(nullptr) {}
	HFIELD(void *p) : pvt(p) {}
	operator bool() const { return pvt != nullptr; }
};
typedef const char * HNAME;

class I_Front;

enum SCOPE_enum
{
	SCOPE_STRUC,
	SCOPE_CLASS,
	SCOPE_NAMESPACE,
	//SCOPE_UNION,
	SCOPE_ENUM,
	SCOPE_FUNC,//source-level function declaration
	SCOPE_VFTABLE,
	SCOPE__LAST
};

enum FuncModFlags
{
	FUNCMOD_STATIC		= 0x01,	//static (non-static by default)
	FUNCMOD_VIRTUAL		= 0x02,	//invalid with static
	FUNCMOD_CONST		= 0x04	//invalid with static
};

//field's attributes
enum AttrIdEnum : unsigned int
{
	ATTR_NULL,	//relaxed hex by default
	//for compound types
	ATTR_NEWLINE,//an empty line before declaration
	ATTR_GUID,//separate fields in this like: XX-XXXX-XXX

	//for primitive types
	ATTR_HEX = ATTR_NULL + 1,	//hex of fixed width 
	ATTR_DECIMAL,
	ATTR_BINARY,
	ATTR_OCTAL,
	ATTR___ASCII,
	ATTR_ASCII_TEXT,//ascii formatted as text
	ATTR___UNICODE,//zero-terminated
	ATTR_NUNICODE,//counted
	ATTR_UTF8,

	//attributes used by front end only //always are hex
		ATTR_MISC_BEGIN,
	ATTR_FILEPTR = ATTR_MISC_BEGIN,		//pointer to the raw data in binary
	ATTR_OFFS,							//the same as ATTR_VA
	ATTR_VA = ATTR_OFFS,				//address in current seg range (image base is added)
	ATTR_RVA,							//RVA
	ATTR_RVA_OR_ORDINAL,				//RVA
		ATTR___MISC_BEGIN,
//...
		ATTR_MISC_END = 0xFF,
		ATTR__MASK = 0xFF,//255 allowed
	//ATTR__TOTAL,
	//extra attributes
	//ATTR_IMPORTED		= 0x00000100,
	ATTR_EXPORTED		= 0x00000200,
	//flags
		ATTR__INT_MASK	= 0x00003000,
	ATTR_COLLAPSED		= 0x00008000,			//for binary view display only

	__ATTR_LAST,
	//__ATTR_COLLAPSED	= 0x20000000,
	__ATTR_TYPE_MASK	= 0xF0000000,
	__ATTR_GLOBAL		= 0x00000000,	//all above is for this attrib type!
	__ATTR_ENUM			= 0x10000000,	//unused by FE (declEField)
	__ATTR_BITSET		= 0x20000000,	//unused by FE (declBField)
//	__ATTR_PROC			= 0x40000000,
	__ATTR_FUNC			= 0x80000000,
	__ATTR_CLASS		= 0xC0000000,

	//__ATTR_ENUM
	//ATTRE_CONFN			= __ATTR_ENUM + 1,	//confirm a negative e-field (by default, -1 is special to denote the next in row)
			
	//__ATTR_FUNC
	ATTRF_THISPTR		= __ATTR_FUNC + 1,
	ATTRF_RETVAL		= __ATTR_FUNC + 2,
	ATTRF_REGISTER		= __ATTR_FUNC + 3,

	//__ATTR_CLASS
	ATTRC_VPTR			= __ATTR_CLASS + 1,
	ATTRC_HEIR			= __ATTR_CLASS + 2,
	//ATTRC_VFTABLE		

	__ATTRX_LAST
};
#define ATTR_UNICODE	ATTR_NULL
enum class AttrScopeEnum : unsigned int
{
	null,
	//__ATTR_PROC
	//ATTRP__MASK			= 0xF,
	ATTRP_STATIC		= FUNCMOD_STATIC,		//static (non-static by default)
	ATTRP_VIRTUAL		= FUNCMOD_VIRTUAL,	//invalid with static
	ATTRP_CONST			= FUNCMOD_CONST		//invalid with static
};

#define CHECK_ATTR_TYPE(attr, e)	(((attr) & __ATTR_TYPE_MASK) == e)
#define CLEAR_ATTR_TYPE(attr, e)	attr = AttrIdEnum(attr & ~__ATTR_TYPE_MASK);
#define SET_ATTR_TYPE(attr, e)		attr = AttrIdEnum(attr | e)

#define MISC_COLOR_CHECK_RANGE(iColor)	(adcui::COLOR_ATTRIB_BEGIN <= iColor && iColor < adcui::COLOR_ATTRIB_END)
#define MISC_COLOR_TO_ATTR(iColor)		AttrIdEnum(ATTR_MISC_BEGIN + (iColor - adcui::COLOR_ATTRIB_BEGIN))

#define MISC_ATTR_CHECK_RANGE(iAttr)	(ATTR_MISC_BEGIN <= iAttr && iAttr <= ATTR_MISC_END)
#define MISC_ATTR_TO_COLOR(iAttr)		(adcui::COLOR_ATTRIB_BEGIN + (iAttr - ATTR_MISC_BEGIN))

enum AlignEnum
{
	ALIGN_BYTE = 0,
	ALIGN_WORD,//1
	ALIGN_DWORD,//2
	ALIGN_QWORD//3
};

typedef unsigned	POSITION;
typedef unsigned	HPATCHMAP;

class I_DataStreamBase : public I_DataSourceBase
{
public:
	virtual POSITION	setcp(POSITION) = 0;
	virtual POSITION	cp() const = 0;
	virtual OFF_t	cpr() const = 0;
	virtual unsigned	skip(int) = 0;
	virtual unsigned	skipBits(int) = 0;
	virtual POSITION	align(unsigned) = 0;
};

#define F_DEBUG	(__DEBUGGING && 1)

#if(F_DEBUG)
#define F_CHECK(a) this->_FCP = a
#define F_CHECK2(a) iface._FCP = a
#else
#define F_CHECK(a)
#define F_CHECK2(a)
#endif

class I_Module : public I_DataStreamBase
{
#if(F_DEBUG)
public:
	OFF_t _FCP;
#endif
public:
	enum CODE_TYPE_e { CODE_TYPE_CODE, CODE_TYPE_PROC, CODE_TYPE_THUNK, CODE_TYPE_PROC_FORCE };
	enum PTR_TYPE_t { PTR_AUTO, PTR_16BIT = 2, PTR_32BIT = 4, PTR_64BIT = 8 };
	enum PTR_MODE_t { PTR_MODE_PTR, PTR_MODE_REF, PTR_MODE_RVREF, PTR_MODE_PMEM, PTR_MODE_PMFUNC };

	virtual HTYPE	NewScope(const char *, SCOPE_enum = SCOPE_STRUC, AttrScopeEnum = AttrScopeEnum::null) = 0;
	virtual HTYPE	NewScope(HFIELD, SCOPE_enum = SCOPE_STRUC, AttrScopeEnum = AttrScopeEnum::null) = 0;
	virtual void	Leave() = 0;
	virtual bool	EnterScope(ADDR) = 0;
	virtual bool	EnterScope(HTYPE, ADDR) = 0;
	virtual bool	EnterSegment(HTYPE, ADDR) = 0;
	virtual HTYPE	traceOf(HTYPE) const = 0;
	virtual void	selectFile(const char *, const char *folder = nullptr) = 0;
	virtual void	installNamespace() = 0;
	virtual	void	installTypesMgr() = 0;
	virtual HTYPE	declTypedef(const char *, HTYPE) = 0;

	virtual HFIELD	declField(HNAME, HTYPE, AttrIdEnum = ATTR_NULL, ADDR = (ADDR)-1) = 0;
	virtual HFIELD	declField(HNAME = nullptr, AttrIdEnum = ATTR_NULL, ADDR = (ADDR)-1) = 0;
	virtual HFIELD	declUField(HNAME, HTYPE, AttrIdEnum = ATTR_NULL) = 0;
	virtual HFIELD	declUField(HNAME = nullptr, AttrIdEnum = ATTR_NULL) = 0;
	//virtual HFIELD	instField(HNAME, HTYPE, AttrIdEnum = ATTR_NULL) = 0;
	virtual HFIELD	declBField(HNAME, HTYPE, AttrIdEnum = ATTR_NULL, ADDR at = (ADDR)-1) = 0;
	virtual HFIELD	declEField(HNAME/*, AttrIdEnum = ATTR_NULL*/) = 0;
	virtual HFIELD	declEField(HNAME, ADDR/*, AttrIdEnum = ATTR_NULL*/) = 0;
	virtual HFIELD	declCField(HNAME, CODE_TYPE_e = CODE_TYPE_PROC, AttrIdEnum = ATTR_NULL) = 0;
	virtual HFIELD	declPField(HNAME n, AttrIdEnum a = ATTR_NULL) { return declCField(n, CODE_TYPE_PROC_FORCE, a); }

	virtual HTYPE	type(OpType_t) = 0;
	virtual HTYPE	type(HNAME) = 0;
	virtual HTYPE	arrayOf(HTYPE, unsigned n, bool bytes = false) = 0;// if 'bytes'=true, 'n' is number of bytes for array type, otherwise - a number of elements
	virtual HTYPE	arrayOfIndex(HTYPE, HTYPE) = 0;
	virtual HTYPE	enumOf(HTYPE, OpType_t) = 0;
	virtual HTYPE	constOf(HTYPE) = 0;
	virtual HTYPE	ptrOf(HTYPE, PTR_TYPE_t = PTR_AUTO, PTR_MODE_t = PTR_MODE_PTR) = 0;
	virtual HTYPE	impOf(HTYPE) = 0;//a ref to another module
	virtual HTYPE	expOf(HTYPE) = 0;//a ref to this module
	virtual HTYPE	pairOf(HTYPE, HTYPE) = 0;
	virtual HTYPE	funcTypeOf(HTYPE, HTYPE, unsigned) = 0;
	virtual void	error(const char * = 0) = 0;
	virtual void	blockSignals(bool) = 0;
	virtual I_Front* frontend() const = 0;
};

class I_ModuleEx
{
public:
	virtual bool DeclareContextDependentType(const char *){ return false; }
	virtual bool RegisterFormatterType(const char *){ return false; }
	virtual bool DeclareCodeType(const char *){ return false; }
};

class I_SuperModule : public I_Module,
	public I_ModuleEx
{
public:
	virtual HFIELD	instField(HNAME, HTYPE, AttrIdEnum = ATTR_NULL) = 0;
	virtual HTYPE	code() = 0;
	virtual bool	setDefaultCodeType(HTYPE) = 0;
	//virtual bool	EnterScope(const char *){ return false; }
	virtual bool	EnterAttic() = 0;
	//virtual void	setFlags(unsigned){}
	//virtual PCODE	CreateCode(PSEG){ return nullptr; }
	virtual void	installFrontend(const char *, int id = 0) = 0;
	virtual void	reuseFrontend(int id) = 0;
	virtual HTYPE	NewRangeSet(ADDR64 base64, const char *name = nullptr) = 0;
	//virtual HRANGE		NewRange(HTYPE, ADDR addrV, ADDR_RANGE sizeV){ return 0; }
	//virtual	PSEG	createSegment(PSEG iSegOwner, ADDR addrBaseV, ADDR_RANGE segSizeV, int flags, bool, bool){ return nullptr; }
	enum SEG_FLAGS_t { ISEG_NULL = 0, ISEG_CODE = 1, ISEG_CONST = 2,
		ISEG_BITMASK	= 0x0F00,
			ISEG_16BIT	= 0x0100,
			ISEG_32BIT	= 0x0200,
			ISEG_64BIT	= 0x0300,
			ISEG_MSB	= 0x1000,//most-significant byte first (big-endian)
			ISEG_NCASE	= 0x2000};//case insensitive arch (Windows)
	virtual HTYPE	NewSegment(unsigned rawSize, const char *name, unsigned flags = 0) = 0;
	virtual HTYPE	AddSubRange(HTYPE, ADDR addrV, ADDR_RANGE sizeV, HTYPE) = 0;
	virtual bool	setSize(unsigned) = 0;
	virtual void	setEndianness(bool) = 0;
	//virtual bool	setEntryPoint(ADDR) = 0;
	//virtual bool	enqueEntryPoint(ADDR) = 0;
	//virtual void	AddCxxSymbol(ADDR, const char *mangled, const char *demangled){}
	virtual HPATCHMAP	newPatchMap() = 0;
	virtual bool	addPatch(HPATCHMAP, OFF_t offs, unsigned size) = 0;
	virtual I_DataSourceBase	*stitchDerivativeModule(HPATCHMAP, const char *name, const char *type, bool bDelayed = false, int moduleId = 0) = 0;
	virtual I_DataSourceBase	*module(const char *) = 0;
	virtual I_DataSourceBase	*module(int moduleId) = 0;
	virtual bool	preformatModule(int moduleId, const char *typeStr, bool delayed) = 0;
};

class I_ModuleCB : public I_Module
{
public:
	enum Dump_D_Flags { DUMP_D_ALL, DUMP_D_FUNCTIONS_ONLY };
	virtual void dump(ADDR va, OFF_t oSymbolName, unsigned iNameMax, unsigned uFlags) = 0;
	virtual void dump(ADDR va, const char *pSymbolName, unsigned uFlags) = 0;
	virtual void dumpSrc(const char *) = 0;
	virtual void resetProgress(const char *, unsigned) = 0;
	//virtual I_Front* frontend() const { assert(0); return nullptr; }
};

OpType_t from_PTR_TYPE(I_Module::PTR_TYPE_t e);

class I_Main
{
public:
	virtual const char* executablePath() const = 0;
	virtual const char* frontPathFromName(const char* frontName) const = 0;
	virtual const char* protoPath(const char* fileName) const = 0;
};