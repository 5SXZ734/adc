#pragma once

#define NEW_OP	0//failed to finish due to the save/restore (no easy way to deal with a list of root infos)

#if(NEW_OP)
#ifdef _DEBUG
#define NEW_OP_DEBUG	1
#endif
#endif

#include "shared/misc.h"
#include "shared/action.h"
//#include "shared/front.h"
#include "db/obj.h"
#include "xref.h"


template <typename T> class XRef_t;
class Path_t;
//struct TYPE _t;
struct VALUE_t;
struct value_t;
enum SSID_t;
enum IfCond_t;
class MemoryMgrEx_t;
enum OpSeg_t : unsigned;



///////////////////////////////////////////////Op_t
class Op_t;
#if(NEW_OP_PTR)

class OpPtr;
typedef mem::HMEM<Op_t>	HOP;
typedef const HOP&		CHOP;

class OpPtr
{
	mem::MemRef mhRef;
public:
	OpPtr()
		: mhRef(0)
	{
	}
	OpPtr(const OpPtr &o)
		: mhRef(o.mhRef)
	{
		//assert(mhRef != 1);
	}
	OpPtr(mem::MemRef hRef)
		: mhRef(hRef)
	{
		//assert(mhRef != 1);
	}
	OpPtr(const HOP &o)
		: mhRef(o.memRef())
	{
		//assert(mhRef != 1);
	}
	//operator void *() { return (void *)mhRef; }
	operator mem::MemRef() const { return mhRef; }
	operator HOP() const {
		return HOP(mhRef, toOp(mhRef)); }
	OpPtr &operator=(OpPtr o) { mhRef = o.mhRef; return *this; }
	//OpPtr &operator=(mem::MemRef o) { mhRef = o; return *this; }
	/*OpPtr &*/void operator=(const HOP &o) { mhRef = o.memRef(); /*return *this;*/ }
	Op_t* operator->() const { return toOp(mhRef); }
	operator Op_t *() const { return toOp(mhRef); }
	bool operator==(const OpPtr &o) const { return mhRef == o.mhRef; }
	bool operator!=(const OpPtr &o) const { return !operator==(o); }
	operator bool() const { return mhRef != 0; }
	bool operator<(const OpPtr &o) const { return mhRef < o.mhRef; }
public:
	const mem::MemRef &memRef() const { return mhRef; }
private:
	static Op_t *toOp(mem::MemRef);
};
typedef List_t<OpPtr>	OpList_t;

#else//NEW_OP_PTR
typedef Op_t*	OpPtr;
typedef List_t<OpPtr>	OpList_t;
typedef OpPtr	HOP;
typedef HOP		CHOP;
#endif



//template <typename T> class XRef_t;
typedef XRef_t<OpPtr> XOpLink_t;
typedef XRefList_t<XOpLink_t> XOpList_t;

typedef XRef2_t<OpPtr> XOpLink2_t;
typedef XRefList_t<XOpLink2_t> XOpList2_t;


////////////////////////////////////////////PathPtr
class Path_t;
#if(NEW_PATH_PTR)

class PathPtr;
typedef mem::HMEM<Path_t>	HPATH;
typedef const HPATH&		CHPATH;

class PathPtr
{
	mem::MemRef mhRef;
public:
	PathPtr() : mhRef(0) {}
	PathPtr(const PathPtr &o)
		: mhRef(o.mhRef)
	{
	}
	PathPtr(mem::MemRef hRef) : mhRef(hRef) {}
	PathPtr(const HPATH &o)
		: mhRef(o.memRef())
	{
		//assert(mhRef != 1);
	}
	operator HPATH() const {
		return HPATH(mhRef, toPath(mhRef)); }
	//operator void *(){ return (void *)mhRef;  }
	operator mem::MemRef() const { return mhRef;  }
	PathPtr &operator = (PathPtr o) { mhRef = o.mhRef; return *this; }
	Path_t* operator->() const { return toPath(mhRef); }
	operator Path_t *() const { return toPath(mhRef); }
	bool operator==(const PathPtr &o) const { return mhRef == o.mhRef; }
	bool operator!=(const PathPtr &o) const { return !operator==(o); }
	operator bool() const { return mhRef != 0; }
	bool operator<(const PathPtr &o) const { return mhRef < o.mhRef; }
public:
	const mem::MemRef &memRef() const { return mhRef; }
private:
	static Path_t *toPath(mem::MemRef);
};
typedef List_t<PathPtr>	PathList_t;
typedef Tree_t<PathPtr>	PathTree_t;

#else//NEW_PATH_PTR

typedef List_t<Path_t>	PathList_t;
typedef Path_t* 	PathPtr;
typedef PathPtr	HPATH;
typedef HPATH	CHPATH;
typedef Tree_t<PathPtr>	PathTree_t;

#endif//NEW_PATH_PTR



//Op_t::flags

#define OLD_FIELDREF

#define TRACE_PTREX			0x000F
#define	TRACE_PTR			0x0001	//<baseptrs>
//#define TRACE_OPTYPE		0x0020	//<types>
#define TRACE_PASSFUNC		0x0040	//<xins>
#define	TRACE_XOUTSEEN		0x0080	//<unroot>xdepout dest was reached
#define TRACE_OVERLAPPED	0x0100	//<xins>
#define TRACE_FIELD			0x0200
#define	TRACE_FORWARD		0x0400
//#define	TRACE_WINDUP		0x4000	//for winding up
#define	TRACE_MARK			0x8000	//trace passes count mask
//#define TRACE_WINDUPENUM	0x7FFF	//thread for winding up


#define	FPUSTACK_INVALID	0x80
#define	STACK_INVALID		0x80000000L

//////////////////////////////////////////////Ins0_t

struct Ins0_t
{
private:
#ifdef _DEBUG
	int			mOff;
public:
	void		setOff(int off){ mOff = off; }
	int			off() const { return mOff; }
private:
#endif
	ADDR		mVA;
public:
	Action_t	mAction;
	union {
		int8_t		mFlags;
		struct {
			uint8_t	opnd_HIDDEN:1;
			uint8_t	opnd_BAD_STACKOUT:1;
			uint8_t	opnd_REDUCED:1;			//set, if reduced/propagated
			uint8_t	opnd_CLONED:1;//SPLIT:1;//r		//splitted
			uint8_t	opnd_NOLVAL:1;//r	//C	//lvalue is invalid when unrooted
			uint8_t	opnd_INTRINSIC:1;		//intrinsic call
			uint8_t	opnd_GAP:1;//r			//when dumping - insert gap immidiatly before
			uint8_t	opnd_ROOTPREV:1;
		};
	};

	PathPtr		mpPath;//owner
	PathPtr		mpPathRef;//jump to
#if(NEW_OP)
	HOP prime(){ assert(this); return mArgs.Head(); }
	Ins0_t *insPtr() const { return (Ins_t *)this; }
#else
	OpPtr		mpPrimeOp;
#endif
	OpList_t	mArgs;		//op list

	int32_t		mPStackIn;
	int32_t		mPStackDiff;	//current stack pointer value

	int8_t		mFStackIn;
	int8_t		mFStackDiff;			//FPU register stack top value

	uint16_t		mEFlagsTested;
	uint16_t		mEFlagsModified;
	uint16_t		mFFlagsAffected;		//FPU flags affected

	Ins0_t();
	~Ins0_t();

	void setRoot(bool b){ opnd_REDUCED = !b; }
	bool isRoot() const { return !opnd_REDUCED; }
	bool isRootPr() const { return !opnd_ROOTPREV; }

	void reset(int retAddrSize) {
		bool bIntrinsic(opnd_INTRINSIC);//LATER
		mFlags = 0;
		opnd_INTRINSIC = bIntrinsic;
		mPStackIn = 0;
		mFStackIn = 0;
		if (mAction == ACTN_CALL)
		{
			mPStackDiff = bIntrinsic? 0 : retAddrSize;//?!
			mFStackDiff = 0;
			mEFlagsTested = 0;
			mEFlagsModified = 0;
			mFFlagsAffected = 0;
		}
	}

	bool destruct(MemoryMgrEx_t &);
	OpList_t	&args(){ return mArgs; }
	const OpList_t& args() const { return mArgs; }
	//Ins_t *lin ked() const { return (Ins_t *)(this); }
	void mergeFrom(const Ins0_t &);
	ADDR VA() const { return mVA; }
	void setVA(ADDR va){
		assert(va != -1);
		mVA = va; }
	bool isAux() const { return mVA == -1; }

	int32_t PStackIn() const{ return mPStackIn; }
	int32_t PStackDiff() const{ return mPStackDiff; }
	void setPStackIn(int32_t n){ mPStackIn = n; }
	void setPStackDiff(int32_t n){ mPStackDiff = n; }
	void addPStackDiff(int32_t n){ mPStackDiff += n; }

	int8_t FStackIn() const{ return mFStackIn; }
	int8_t FStackDiff() const{ return mFStackDiff; }
	void setFStackIn(int8_t n){ mFStackIn = n; }
	void setFStackDiff(int8_t n){ mFStackDiff = n; }
};


struct Ins0_t;
#if(NEW_OP)
typedef ListNode_t<Ins0_t>	Ins_t;
#else
typedef Ins0_t	Ins_t;
#endif


union op_tracer_cell_t
{
	uint16_t		trace;
	uint16_t		trace_ptr : 4;
};


class OpBase_t : public ObjFlags_t,
#if(!NO_OBJ_ID)
	public IdCheck_t,
#endif
	public ListNode_t<OpPtr>
{
public:
	OpBase_t()
	{
	}
#if(!NO_OBJ_ID)
	OpBase_t(int id)
	: IdCheck_t(id)
	{
	}
#endif
};

//////////////////////////////////////////////// Op_t

class Op_t : public OpBase_t
{
public:
	Ins_t *	m_pRI;

#ifdef _DEBUG
	bool	__isPrime;
#endif
	uint8_t	m_opc;	//class
	int8_t	mOffs;
	union {
		uint8_t	m_optyp;	//size+type
		uint8_t	m_opsz:4;
	};

	//uint8_t	m_seg;
	int32_t	m_disp;		//scalar part of displacement
	int32_t	m_disp0;	//old disp - for locals after mloc attachment

	XOpList_t	m_xin;//list of objects, on whom THIS one depends
	XOpList_t	m_xout;//list of objects, who depends on THIS one

#ifdef OLD_FIELDREF
	FieldPtr	mpLocalRef;		//relocatable part of displacement
#endif
	
#if(!NEW_OP_TRACER)
	op_tracer_cell_t mtr;
#endif

#ifdef OPTEST
	static uint32_t	nOpsTotal;//total ops being created till date
	int	nTestId;
#endif

	//implementation

	Op_t();
#if(!NO_OBJ_ID)
	Op_t(int);//for tmp ops
#endif
	~Op_t();
	
	//int ObjType() const { return OBJID_OP; }
	void	Clear();
	//void na mex(MyString &s) const { s = "op"; }
	//static HOP toOp(const ObjFlags_t *);

#if(NO_OBJ_ID)
	int		ID() const { return ((~1) >> 1); }
	//void	SetID(int){}
#endif

	OPC_t OpC() const { return (OPC_t)m_opc; }
	OPC_t OpCf() const {
		if ((OPC_t)SSID() == OPC_CPUSW)
			return (OPC_t)SSID();
		return (OPC_t)m_opc;
	}
	bool isCPUSWflags() const { return OpC() == OPC_t(OPC_FLAGS|OPC_CPUSW); }
	bool isCPUSW() const { return (OPC_t)SSID() == OPC_CPUSW; }
	SSID_t SSID() const { return (SSID_t)(m_opc & SSID_MASK); }
	uint8_t OpType() const { return m_optyp; }
	uint8_t OpSize() const { return (m_opsz); }
	int8_t OpOffs() const { return mOffs; }
	uint8_t OpOffsU() const { return (uint8_t)mOffs; }
	OpSeg_t	OpSeg() const { return (OpSeg_t)((m_opc & 0xF0) >> 4); }
	bool isAddr() const { return (m_opc & OPC_ADDRESS) && !isCPUSW(); }

	void clearInsPtr();
	OpList_t &args(){ return ins().mArgs; }
	const OpList_t &args() const { return ins().mArgs; }

#if(NEW_OP)
	const List_t<Ins_t> &list() const;
	List_t<Ins_t> &list();
	OpList_t::Iterator argsIt(){ return ++OpList_t::Iterator(args()); }
	OpList_t::Iterator argsIt() const { return ++OpList_t::Iterator(args()); }
	bool hasArgs() const { return (args().Next(lin ked()) != nullptr); }
	HOP arg1() const { return args()[1]; }
	HOP arg2() const { return args()[2]; }
	Ins_t *insPtr(){ assert(this); return m_pRI; }
	const Ins_t *insPtr() const { assert(this); return m_pRI; }

	Op_t	*PrimeOp() const {
		return m_pRI->mArgs.Head();
	}
	void setInsPtr(HOP p){
		if (p) {
			assert(p->IsPrimeOp());
#ifdef _DEBUG
			__isPrime = (p == this);
#endif
			m_pRI = p->m_pRI; 
		}
		else
			m_pRI = nullptr;
	}
	

#else
	OpList_t::Iterator argsIt(){ return OpList_t::Iterator(args()); }
	OpList_t::Iterator argsIt() const { return OpList_t::Iterator(args()); }
	bool hasArgs() const { return !args().empty(); }
	OpPtr arg1() const { return args()[0]; }
	OpPtr arg2() const { return args()[1]; }

	//OpPtr	PrimeOp() const { 
		//return m_pRI->mpPrimeOp; }
	void setInsPtr(Ins_t *pri){
		m_pRI = pri; }

#endif
	//bool	IsPrimeOp() const { return (PrimeOp() == this); }
	Ins_t &ins(){ return *m_pRI; }
	const Ins_t &ins() const { return *m_pRI; }
	Ins_t *insPtr() const { return m_pRI; }

	XOpLink_t *XIn() const { return m_xin.head(); }
	void setXIn(XOpLink_t *p) { m_xin = p; }
	XOpLink_t *XOut() const { return m_xout.head(); }	
	void setXOut(XOpLink_t *p) { m_xout = p; }
	void	MoveArgsTo(OpList_t &);

	void	Setup0(const ri_t& r) {
		m_opc = (uint8_t)r.ssid;
		mOffs = (int8_t)r.ofs;
		m_optyp = (uint8_t)r.typ;
	}
	void	Setup4(const r_t& r, int disp) {
		m_opc = r.ssid;
		mOffs = r.ofs;
		m_optyp = r.typ;
		m_disp = disp;
	}
	void	Setup4(const ri_t& r, int disp) {
		m_opc = (uint8_t)r.ssid;
		mOffs = (int8_t)r.ofs;
		m_optyp = (uint8_t)r.typ;
		m_disp = disp;
	}
	void	Setup3(uint8_t opc, uint8_t typ, uint8_t offs){
		m_opc = opc;
		mOffs = (int8_t)offs;
		m_optyp = typ;
	}
	void	Setup4(uint8_t opc, uint8_t typ, uint8_t offs, int disp){
		m_opc = opc;
		mOffs = (int8_t)offs;
		m_optyp = typ;
		m_disp = disp;
	}
	void SetOpId(uint8_t id) {
		mOffs = (int8_t)id;
	}
	void SetOpType(uint8_t tp) {
		m_optyp = tp;
	}
	void SetOpSize(uint8_t sz) {
		uint8_t tp = OpType() & 0xF0;
		SetOpType(tp | sz);
	}
	void	SetOpSeg(OpSeg_t opseg){
		m_opc &= 0x8F;
		m_opc |= (opseg << 4);
	}

	FieldPtr	localRef() const;
	void		setLocalRef0(FieldPtr p);

	PathPtr		pathRef() const { return m_pRI->mpPathRef; }
	void		setPathRef(PathPtr p){ m_pRI->mpPathRef = p; }

	int32_t pstackIn() const {
		return ins().mPStackIn;
	}
	int32_t pstackDiff() const {
		return ins().PStackDiff();
	}
	int32_t setPStackIn(int32_t n){
		int32_t t(pstackIn());
		ins().setPStackIn(n);
		return t;
	}
	void addPStackDiff(int value) {
		Ins_t& a(ins());
		a.setPStackDiff(a.PStackDiff() + value);
	}
	void setPStackDiff(int value){
//CHECK(m_pRI->off() == 79)
//STOP
		ins().setPStackDiff(value);
	}
	
	int8_t	fstackDiff() const {
			return ins().FStackDiff();
	}
	int8_t	setFStackIn(int8_t f){
		int8_t t(ins().mFStackIn);
		ins().mFStackIn = f;
		return t;
	}
		
	//int8_t	FPURegID();

	bool	isCombined() const;
	
	bool	isRoot() const { return ins().isRoot(); }
	bool	isInRow() const {
		return (m_nFlags & OPND_INROW) != 0;
	}
	void setInRow(bool b){
		if (b)
			m_nFlags |= OPND_INROW;
		else
			m_nFlags &= ~OPND_INROW;
	}

	void	setCloned(bool b){ ins().opnd_CLONED = b; }
	bool	isCloned() const { return ins().opnd_CLONED; }
//			ins().opnd_SPLIT = b; }
	bool	isSplit() const { return 0; }//ins().opnd_SPLIT != 0; }
//	void	setSplit(bool b){ 
//			ins().opnd_SPLIT = b; }
//	bool	IsDead(){ 
//			return ins().opnd_DEAD == 1; }	
	bool	isHidden() const { return ins().opnd_HIDDEN == 1; }//actual only when output is assemblerized
	
//	void	SetDead(bool b){ 
//			ins().opnd_DEAD = b; }
//	bool	isHidden(){ 
//			return (m_nFlags & OPND_HIDDEN) != 0; }//actual when output is unfold

	bool	isGap() const { 
			return ins().opnd_GAP != 0; }
	void	setGap(bool b){ 
			ins().opnd_GAP = b; }
	bool	isLValueVisible(){ 
			return ins().opnd_NOLVAL == 0; }
	void	setLValueVisible(bool b){ 
			ins().opnd_NOLVAL = !b; }//1-visible,0-temporary


//	Op_t	*GetExitOp();
//	int		ResetLocals(), ResetLocal();
		
//	int		Offset2();
	int		Size() const;
	void	setAction(Action_t a);


	bool	IsIndirect0() const { return (m_opc & OPC_INDIRECT) && !isAddr(); }//(m_opc & OPC_ADDRESS); }//any indirect but not address
	bool	isGlobal_() const { return IsIndirect0() && (SSID() == (SSID_t)OPC_GLOBAL); }
	bool	IsIndirect() const { return IsIndirect0() || (OpC() == OPC_LOCAL); }//any indirect or local
	bool	IsIndirectB() const { return IsIndirect0() && (SSID() != SSID_GLOBAL); }//all but global
	bool	IsIndirectOf(SSID_t ssid) const { return ((SSID() == ssid) && IsIndirectB()); }
	bool	IsIndirectEx() const;
	bool	IsScalar() const { return (OpC() == OPC_IMM); }
	
	void	GetVALUE(VALUE_t &);
	int		Displace(int d);
	bool	CheckScatteredDependency() const;
	bool	CheckConditionDependency() const;
	
	//int		InvalidateGoto();

	bool	EqualTo(CFieldPtr  pField);		//simple compare
	int		Offset0() const;
	ADDR	VA(){ return ins().VA(); }
	OpPtr	OPOut(int n) const { return m_xout[n]; }
	int		checkThisPtr();
	r_t toR(bool bSize = false) const {//for regs
		return r_t(SSID(), OpOffs(), bSize ? OpSize() : OpType());
	}
	ri_t toRi() const {//for regs + locals
		return ri_t(SSID(), Offset0(), OpSize());
	}

	void reset() {
		m_nFlags = OBJID_OP;
	}
};

#define INSPTR(a)	(a)
#define PRIME(a)	(a)

//uint8_t Level2Ptr(int nLevel, int ptrsz = 0);
void print_ops_stats();

