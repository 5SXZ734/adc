#pragma once

#include "db/anlz.h"
#include "db/types.h"
#include "path.h"
#include "info_dc.h"
#include "files_ex.h"
#include "main_ex.h"
#include "mem_ex.h"


/////////////////////////////////////////////////////// locals reside in here
class TypeStrucLoc_t : public Struc_t
{
	typedef	Struc_t	Base_t;
public:
	TypeStrucLoc_t(){}
	~TypeStrucLoc_t(){}
	virtual TypeStrucLoc_t * typeStrucLoc() const { return const_cast<TypeStrucLoc_t *>(this); }
	virtual int ObjType() const { return OBJID_TYPE_STRUCLOC; }
	virtual const char *printType() const { return "strucloc"; }
	virtual ADDR base(CTypeBasePtr) const { return 0; }
	virtual bool maybeUnion() const override { return false; }
};

enum ProtoProfileFlags {//see TypeFunc_t::E_FLAGS
	PPF_Variardic		= 0x01,
	PPF_StackPurge		= 0x02,
	PPF_Thiscall		= 0x10,
	PPF_Usercall		= 0x20
};

struct ProtoProfile_t
{
	GPRs_t	cpuin;
	int		stackin;
	int		fpuin;
	unsigned	_flags;					//ProtoProfileFlags

	ProtoProfile_t()
		: stackin(0),
		fpuin(0),
		_flags(0)
	{
		//clear();
	}
	void operator=(const ProtoProfile_t &o){
		cpuin = o.cpuin;
		stackin = o.stackin;
		fpuin = o.fpuin;
		_flags = o._flags;
	}
	bool isThisCall() const	{
		return !cpuin.empty() && cpuin[0].isValid();
	}
	const REG_t& thisReg() const {
		assert(isThisCall());
		return cpuin[0];
	}
	bool operator==(const ProtoProfile_t &o) const {
		return cpuin == o.cpuin 
			&& stackin == o.stackin
			&& fpuin == o.fpuin
			&& _flags == o._flags;
	}
	void applyStackIn(int d, unsigned stackAddSize)
	{
		if (d > stackin)
		{
			stackin = d;
			unsigned r(stackin % stackAddSize);
			if (r != 0)
			{
				unsigned d(stackin / stackAddSize);
				stackin = (d + 1) * stackAddSize;
			}
		}
	}
};

// FPU stack (grows down with push)
//             ___top
//	ST0:	|0|
//	ST1:	|1|
//	ST2:	|2|
//	ST3:	|3|
//	ST4:	|4|
//	ST5:	|5|
//	ST6:	|6|
//	ST7:	|7|
//	ST8:	|8|


// FUNCTION STUB format :
// <stackin> <stackout> <fpuin> <fpuout> <cpuregsin> <savedregs> <flagsin> <savedflags> 
//	1)	stackin:		how many bytes a function expects as arguments at enter (unsigned)
//	2)	stackout:		how may bytes a function purges at exit (unsigned)
//	3)	fpuin:			how many entries a function expects on FPU stack as arguments at enter (unsigned)
//	4)	fpuout:			by how many entries a functon modifies a FPU stack at exit (signed)
//	5)	{GPregsin}			:a string describing GP register(s) the function takes as arguments in format {retval1!,retval2!..,spoilt1,spoilt2,..} or a single entry
//	6)	{GPregsOut/Spoiled}	:a list of GP register(s) the function returns or spoils in format {thisptr^,arg1,arg2..} or a single entry
//	7)	flagsin				:some flags: 1-function has extra arguments (varargs)						; WRONG: a string describing CPU flags function's arguments






struct FuncProfile_t : public ProtoProfile_t
{
	int		pstackPurge;//args
	int		pstackPurgeRet;//by return statement
	int		fpudiff;
	GPRs_t	spoiltRegs;
	//FlagMaskType	spoiledFlags;
	GPRs_t	cpuout;

	FuncProfile_t()
		: pstackPurge(0),
		pstackPurgeRet(0),
		fpudiff(0),
		spoiltRegs(0)//,
		//spoiledFlags(0)
	{
	}
	void operator=(const FuncProfile_t &o)
	{
		ProtoProfile_t::operator=(o);
		pstackPurge = o.pstackPurge;
		pstackPurgeRet = o.pstackPurgeRet;
		fpudiff = o.fpudiff;
		spoiltRegs = o.spoiltRegs;
		//spoiledFlags = o.spoiledFlags;
		cpuout = o.cpuout;
	}
	int PStackPurge() const { return pstackPurgeRet + pstackPurge; }
};

//func update info
enum FUInfo_t
{
	FUI_TOPS		= 0x0001,	//stacktop,fputop,fpuin
	FUI_LOCALS		= 0x0002,	//valid1,locals
	FUI_XDEPSINS	= 0x0004,	//
	FUI_CALLOUTS	= 0x0008,	//
	FUI_LOCALSPOS	= 0x0010,	//
	FUI_SAVREGS		= 0x0020,
	FUI_XOUTS		= 0x0040,
	FUI_PTRS		= 0x0080,
	FUI_ROOTS		= 0x0100,
	FUI_BASE0		= 0x0200,	//simple base anlysys (only recheck analized status)
		FUI__BLOCKS		= 0x7C00,
		FUI_SWITCHES	= 0x0400,
		FUI_LOGICS		= 0x0800,
		FUI_IFS			= 0x1000,
		FUI_ELSES		= 0x2000,
		FUI_WHILES		= 0x4000,
	FUI_NUMBERS		= 0x8000,	//ops need to be numbered
FUI_BASE		= FUI_TOPS|FUI_BASE0,
FUI_ALL			= 0xFFFF,
};


typedef	FieldMap::iterator			ArgFieldMapIt;
typedef	FieldMap::const_iterator	ArgFieldMapCIt;
typedef	FieldMap::iterator			RetFieldMapIt;
typedef	FieldMap::const_iterator	RetFieldMapCIt;

//#define FNC_REXOUTS	0x0200	//xouts must be re-checked
//#define FNC_REROOTS	0x0400	//roots must be re-checked
//#define FNC_RENUMBER	0x0800	//needs it's ops to be renumbered
//#define	FNC_USERDEF		0x0400	//defined with user input (not from script)
//#define	FNC_ARGSEX		0x0800	//takes variable number of args (must be __cdecl only)

class FuncDef_t : public TypeStrucLoc_t
{
	typedef TypeStrucLoc_t	Base_t;
public:

	//FieldPtr	mpThisPtr;
	//RegMaskType	mSpoiltRegs;
	int16_t		m_stackOut;
	uint16_t		mSpoiltFlags;		//CPU status word flags saved (EFlags_t)
	int8_t		m_nFPUOut;		//how the func changes FPU top 
	//FieldMap	mRetFields;
#if(NEW_LOCAL_VARS)
	TypePtr		mpLocals;
#endif

	//these are valid for funcdefs with body
	PathTreeEx_t	mPathTree;
	OpList_t mCallRets;	//list of ops returned with calls
	//TypeUnion_t*	mpLocals;
	uint16_t		m_dc;	//? not saved

public:
	FuncDef_t();// MemoryMgr_t *);
	~FuncDef_t();
	FuncDef_t(const FuncDef_t &);
	FuncDef_t &operator=(const FuncDef_t &);
	void CopyFrom(const FuncDef_t &);

	virtual int ObjType() const { return OBJID_TYPE_FUNCDEF; }
	virtual FuncDef_t * typeFuncDef() const { return const_cast<FuncDef_t *>(this); }
	//virtual void aka(MakeAlias &) const { assert(0); }
	const PathTreeEx_t &pathTree() const { return mPathTree; }
	PathTreeEx_t &pathTree(){ return mPathTree; }
	TypePtr locals() const { return mpLocals; }
	void setLocals(TypePtr p){ mpLocals = p; }

	//void setThisPtr(FieldPtr p){ mpThisPtr = p; }
	//bool	isThisCall() const { return mpThisPtr != nullptr; }

	//bool	isClassMember() const;
	bool	IsMineArg(CFieldPtr ) const;
	bool	IsMineRet(CFieldPtr ) const;

	bool	isFastCall() const { return false; }

	PathPtr 	Body() const;
	bool		isStub() const;

	const PathOpList_t &entryOps() const;
	const PathOpList_t &exitOps() const;

	bool hasArgFields() const;
	bool hasRetFields() const;

	FieldMap &argFields(){
		return mFields;
	}
	const FieldMap &argFields() const {
		return mFields;
	}
	FieldMap	&retFields(){
		return mFields;
	}
	const FieldMap	&retFields() const {
		return mFields;
	}

	FieldMap &fields() { /*assert(0);*/ return mFields; }
	const FieldMap &fields() const { /*assert(0);*/ return mFields; }

	FieldPtr takeFrontArg();
	FieldPtr takeFrontRet();
	void takeArgFields(FieldMap &);
	void takeRetFields(FieldMap &);
	void takeSpoiltFields(FieldMap &);
	//int		AddEntryOp(HOP);
//	void	SetSelection(bool);
	int16_t	getPStackPurge() const {
		return m_stackOut;
	}
	void	setPStackPurge(int16_t s){ m_stackOut = s; }
	int		getFStackPurge() const { return m_nFPUOut; }
	void	setFStackPurge(uint8_t f){ m_nFPUOut = f; }
	//void		GetArgProfile(ProtoProfile_t &);

	virtual void namex(MyString &) const;
	virtual const char *printType() const { return "cfunc"; }

	int		ValidateRetList();
	//int		ValidateArgList(Ar g_t *pArgs);
	//int		SetupExitAttribs(int nSTACKout, int nFPUout);
	//void	RedirectExitOps(HOP *ppL ist);
	//void	AddExitOp(HOP pOp);
	//const char	*GetSavedRegsStr() const;

	uint16_t	GetSpoiltFlags() const;
	//FieldPtr getTailLabel() const;
	int		CheckUnresolvedLabels();

	void ClearAnalized();
	FieldPtr thisCallArg(CTypePtr scope) const;

	static bool isLocalArg(CFieldPtr);
	static bool isRetVal(CFieldPtr);
};



//for locals
struct UserDataLocal_t
{
	//PathPtr		pPath;//label's path
	XOpList_t	xrefs;//list of ops referencing a label
	UserDataLocal_t(){}// : pPath(nullptr){}
};

inline UserDataLocal_t *NEW_USERDATA_LOCAL(FieldPtr pField)
{
	return pField->SetUserData<UserDataLocal_t>(new UserDataLocal_t);
}







