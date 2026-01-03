#pragma once

#include <set>
#include <list>
#include "shared/defs.h"
#include "shared/front.h"
#include "shared/data_source.h"
#include "shared/action.h"
#include "db/anlzbin.h"
#include "info_func.h"

class GenerateStage_t;

struct OPND_t
{
	int		m_opc;
	int		m_optyp;
	int		m_dptr;		//if optyp=ptr then m_dptr is data type of *m_optyp
	union {
		int			mOffs;
		int			m_opid;
		unsigned	m_flags;
	};
	//int		opseg;
	union {
		int		disp;
		char	i8;
		short	i16;
		long	i32;
	};
	std::string	name;

	OPND_t()
		: m_opc(0),
		m_optyp(0),
		m_dptr(0),
		mOffs(0),
		//opseg(0),
		disp(0)
	{
	}

	OPND_t(int c, int t, int o)
		: m_opc(c),
		m_optyp(t),
		m_dptr(0),
		mOffs(o),
		//opseg(0),
		disp(0)
	{
	}

	OPND_t(const OPND_t &o)
		: m_opc(o.m_opc),
		m_optyp(o.m_optyp),
		m_dptr(o.m_dptr),
		mOffs(o.mOffs),
		//opseg(o.opseg),
		disp(o.disp),
		name(o.name)
	{
	}

	void setName(const char *p){ name = p; }

	int		opc() const { return m_opc; }
	int		ssid() const { return m_opc & 0xF; }
	int		OpSize() const { return m_optyp & OPSZ_MASK; }
	int		OpType() const { return m_optyp; }
	int		offs() const { return mOffs; }
	int		opid() const { return m_opid; }

	bool	isIndir() const { return (m_opc & OPC_INDIRECT) != 0; }
	bool	isAddr() const { return ((m_opc & OPC_ADDRESS) != 0) && (ssid() != OPC_CPUSW); }
	bool	IsScalar() const { return m_opc == OPC_IMM; }
	//void	SetOp Seg(int s){ opseg = s; }
	void	SetOffs(int o){ mOffs = o; }
	void	SetScalar(int i){ disp = i; }
};

typedef std::list<OPND_t>	lstOPND_t;
typedef lstOPND_t::iterator	lstOPND_it;
typedef lstOPND_t::const_iterator	lstOPND_cit;


struct INS_t
{
	Action_t	action;
	lstOPND_t	ops;
	INS_pvt_t	pvt;

	INS_t(Action_t a)
		: action(a)
	{
	}

	INS_t(Action_t act, const INS_pvt_t &_pvt)
		: action(act),
		pvt(_pvt)
	{
	}

	INS_t(const INS_t &o)
		: action(o.action),
		pvt(o.pvt)
	{
	}

	OPND_t *newop(){
		ops.push_back(OPND_t());
		return &ops.back();
	}

	OPND_t *newop(int c, int t, int o){
		ops.push_back(OPND_t(c, t, o));
		return &ops.back();
	}

	void		SetAction(Action_t a){ action = a; }
	Action_t	Action(){ return action; }
	void		SetStackDiff(int d){ pvt.stackdiff = d; }

};

class PCode_t : public IPCode_t,
	public std::list<INS_t>
{
	ADDR64	mImageBase;
	bool mbInvalid;
public:

	PCode_t(ADDR64 imageBase)
		: mImageBase(imageBase),
		mbInvalid(false)
	{
	}
	void setInvalid(){ mbInvalid = true; }

	HINS newins(Action_t a) {
		push_back(INS_t(a));
		return &back();
	}

	virtual HINS newins(Action_t action, const INS_pvt_t &pvt, bool bPrepend) override {
		if (bPrepend)
		{
			push_front(INS_t(action, pvt));
			return &front();
		}
		push_back(INS_t(action, pvt));
		return &back();
	}
	virtual unsigned insCount() const override
	{
		return (unsigned)size();
	}
	//ins
	virtual HOPND newop(HINS h, int c, int t, int o) override
	{
		INS_t *p = (INS_t *)h;
		return p->newop(c, t, o);
	}
	virtual HOPND newopp(HINS h, int c, int t, int64_t va) override
	{
		INS_t *p = (INS_t *)h;
		va -= mImageBase;
		OPND_t* op(p->newop(c, t, 0));
		op->SetScalar(ADDR(va));
		return h;
	}
	virtual HOPND newop(HINS h) override
	{
		INS_t *p = (INS_t *)h;
		return p->newop();
	}
	virtual HOPND firstOp(HINS h) const override
	{
		INS_t *p = (INS_t *)h;
		return &p->ops.front();
	}
	virtual HOPND lastOp(HINS h) const override
	{
		INS_t *p = (INS_t *)h;
		return &p->ops.back();
	}
	virtual void setAction(HINS h, Action_t act) override {
		INS_t *p = (INS_t *)h;
		p->SetAction(act);
	}
	virtual Action_t getAction(HINS h) const  override {
		INS_t *p = (INS_t *)h;
		return p->Action();
	}
	//ops
	virtual OPC_t opClass(HOPND h) const  override {
		OPND_t *p = (OPND_t *)h;
		return (OPC_t)p->m_opc;
	}

	virtual void setScalar(HOPND h, int i)  override {
		OPND_t *p = (OPND_t *)h;
		p->SetScalar(i);
	}
	virtual int getScalar(HOPND h)  override {
		OPND_t *p = (OPND_t *)h;
		return p->disp;
	}
	virtual void setOpType(HOPND h, uint8_t t)  override {
		OPND_t *p = (OPND_t *)h;
		p->m_optyp = t;
	}
	virtual uint8_t opType(HOPND h) const  override {
		OPND_t *p = (OPND_t *)h;
		return p->m_optyp;
	}
	virtual void setDPtr(HOPND h, int d) override {
		OPND_t *p = (OPND_t *)h;
		p->m_dptr = d;
	}
	virtual void setOpName(HOPND h, const char *pc) override {
		OPND_t *p = (OPND_t *)h;
		p->setName(pc);
	}
	virtual bool isInvalid() const  override {
		return mbInvalid;
	}
};

class PathMap : public std::map<ADDR, HPATH>
{
public:
	PathMap(){}
	/*void setExtent(ADDR lower, ADDR upper)
	{
		assert(lower < upper);
		iterator i(find(lower));
		assert(i != end());
		i->second.second = upper;
	}*/
};
typedef PathMap::iterator	PathMapIt;
typedef PathMap::const_iterator	PathMapCIt;

struct PCodeTraceData
{
	TypePtr	miSeg;
	HOP		mpCurOp;
	HOP		mpLastOp;
	ADDR		mBaseAddr;
	//ADDR		mCurAddr;
	ADDR		mEndAddr;
	//long		mCurOffs;//mCurAddr-mBaseAddr
	TypePtr		miTypeCode;
	PathMap		&mPathMap;
	PathMapIt	mPathMapIt;
	PCodeTraceData(TypePtr seg, /*ADDR base_addr,*/ PathMap	&pm)
		: miSeg(seg),
		mpCurOp(HOP()),
		mpLastOp(HOP()),
		mBaseAddr(0),//base_addr),
		//mCurAddr(-1),
		mEndAddr(0),
		//mCurOffs(-1),
		miTypeCode(nullptr),
		mPathMap(pm),
		mPathMapIt(pm.end())
	{
	}
	void operator=(const PCodeTraceData &o)
	{
		miSeg = o.miSeg;
		mpCurOp = o.mpCurOp;
		mpLastOp = o.mpLastOp;
		mBaseAddr = o.mBaseAddr;
		//mCurAddr = o.mCurAddr;
		mEndAddr = o.mEndAddr;
		//mCurOffs = o.mCurOffs;
		miTypeCode = o.miTypeCode;
		assert(&mPathMap == &o.mPathMap);
		mPathMapIt = o.mPathMapIt;
	}
	HPATH CurrentPath() const
	{
		if (mPathMapIt == mPathMap.end())
			return HPATH();
		return mPathMapIt->second;
	}
	void ResetCurrentPath()
	{
		mPathMapIt = mPathMap.end();
	}
	void SetCurrent(PathMapIt i)
	{
		mPathMapIt = i;
	}
};


class PCodeTracer_t : public FuncTracer_t,
	public PCodeTraceData
{
	GenerateStage_t&	mrStage;
	//FieldMap *mpCachedArgs;
	
public:
	PCodeTracer_t(const FuncTracer_t &, GenerateStage_t &, PCodeTraceData &);
	PCodeTracer_t(const FuncInfo_t &, PathOpTracer_t &, GenerateStage_t &, PCodeTraceData &);

	~PCodeTracer_t();
	//void setCahchedArgs(FieldMap *p){ mpCachedArgs = p; }
	int GeneratePseudoCode(const Locus_t & addrx, ADDR vaRef, int iThunkLevel = 0);
	ADDR LowerBound() const;//func beg
	ADDR UpperBound() const;//func end
	bool CheckCodeBounds(ADDR) const;
	int SetOps(const INS_t &, ADDR, int stackPtrShift);
	void CheckSwitchStatement(ADDR);
	FieldPtr GetJumpTableEntries(HPATH, std::vector<ADDR> &o);
	int		SetupOpFrom(HOP, const OPND_t &, ADDR, int stackPtrShift);//from
	int		SetupFrom(Ins_t &, const INS_t &);
	bool	RemoveLastOps(int);
	PathMapIt	AssurePathBeginsAt(ADDR, ADDR vaCur);
	HPATH	CheckPathChangeAt(HPATH, ADDR);
	HPATH	InsertPathBefore(ADDR, PathPtr before);
	HPATH	InsertPathAfter(HPATH);
	//PathMap	&pathMap(){ return mrStage.pathMap(); }
	//const PathMap &pathMap() const { return mrStage.pathMap(); }
	PathMapIt GetPathAt(ADDR) const;
	bool IsProcessed(HPATH);
	bool IsSeen(HPATH);
	BlockTyp_t FinalizePath(HPATH);
	HOP GetRefOp(ADDR) const;
	HPATH SplitPath(HPATH, ADDR);
};

class CodeIterator2 : public CodeIterator
{
	const I_FrontDC &mrFrontDC;
public:
	CodeIterator2(const I_DataSourceBase &, CTypePtr, CTypePtr seg, ADDR, const I_FrontDC &);
	bool generate(PCode_t &);
};








