#pragma once

#include "shared/defs.h"
#include "info_func.h"

class Field_t;
class Struc_t;
class AnalyzePtrs_t;
class ArgsMap;

#if(0)
typedef TYPE _t	TYPEz_t;
struct PTR_t
{
//private:
	PTR_t	*m_pParent;
	PTR_t	*m_pNext;
	PTR_t	*m_pChilds;
	int32_t		m_nOffs;
	int			m_nIndexSz;
	FieldPtr	mpField;
	Op_t		*m_pOpIndir;	//memory read/write op
	FieldPtr	m_pOpEntry_;	//ptr's entry point (Op_t)
	TYPEz_t		m_t;			//base ptr's type	
//	int			m_nPtrLevel;			//pointer level
//	Struc_t		*m_pStrucBase;	//base struc
	bool		m_bSign;
//	bool		m_bThis;

public:
	PTR_t();
	virtual void	Clear();
	void	CopyTo(PTR_t &p);
	void	 AddChild(PTR_t *pChild);
	PTR_t *	next();
	PTR_t *	prev();
	PTR_t *	prim eOp();
	void	Invalidate();
	void	Append(PTR_t *pPtrInf);
//	void	Log(std::ostream &, int);
	int		RegisterField();
	int		RegisterField2();
	int		__registerFields(), RegisterFields0();
	int		AgreeBasePtr(TypePtr, FieldPtr pOpEntry = 0);
	int		AgreeBaseStrucs(PTR_t *pi);
	//int		CheckArray(PTR_t *p);
	void	ReplaceBaseStruc(TypePtr pStruc1, TypePtr pStruc2);
	bool	isRoot(){ return !m_pParent; }
	FieldPtr	SetupBaseStruc(TYPEz_t &);
	int		GetOffset();
	FieldPtr	GetBaseField();
	TYPEz_t	GetType();
	int		GetPtrLevel();
	void	GetPtrInfo(TYPEz_t &);
	FieldPtr	GetOpEntry(int &nLevel);
	int		__adjustTree();
	bool	IsFirst();
	void	InsertAfter(PTR_t *);
	void	Unlink();
	TypePtr	getBaseStruc(){
		return m_t.mpStruc; }
	void		setBaseStruc(TypePtr p){
		m_t.mpStruc = p; }
	int			getPtrLevel(){
		return m_t.PtrLevel(); }
	void		setPtrLevel(int n){
		m_t.SetPtrLevel(n); }

	FieldPtr	GetSource(){
//			if (!m_pDown)
				return m_pOpEntry_; 
//			return m_pDown->m_pOpIndir;
	}
	void	SetSource(FieldPtr pOp){
			m_pOpEntry_ = pOp; }
	//void	StorePtrs();
};
#endif

struct ptrinfo_t : public FuncTracer_t
{
	HOP	pOp;		//ptr source
	int		disp;		//displacement
	bool	sign;
	//OpTracer_t *mpOpTracer;

	ptrinfo_t(const FuncTracer_t &rFuncDef)
		: FuncTracer_t(rFuncDef)//,
		//mpOpTracer(0)
	{ 
		Clear();
	}
	ptrinfo_t(const FuncInfo_t &fi, PathOpTracer_t &tr)
		: FuncTracer_t(fi, tr)
	{
		assert(0);
	}
	//void setOpTracer(OpTracer_t *p) { /*mpOpTracer = p;*/ }
	//OpTracer_t &opTracer() const { return *mpOpTracer; }
	void Clear(){
		pOp = HOP(); disp = 0; sign = false; }
	void CopyTo(ptrinfo_t &pi) const {
		pi.pOp = pOp; pi.disp = disp; pi.sign = sign; }
	bool EqualTo(const ptrinfo_t &pi) const{
		return (pOp == pi.pOp && disp == pi.disp); }
	int TracePtrSource(HOP);
private:
	int	__tracePtrSource(HOP pOp, ptrinfo_t &pi) const;
};

//recovery of structure fields (via pointer/indirection tracing)
class AnlzTracePtr : public FuncTracer_t
{
	HOP mpOp;
	//OpTracer_t *mpOpTracer;
public:
	AnlzTracePtr(const FuncTracer_t &r, HOP pOp)
		: FuncTracer_t(r),
		mpOp(pOp)//,
		//mpOpTracer(0)
	{
	}
	AnlzTracePtr(const FuncInfo_t &fi, HOP pOp, PathOpTracer_t &tr)
		: FuncTracer_t(fi, tr),
		mpOp(pOp)
	{
	}
	//void setOpTracer(OpTracer_t *p) { /*mpOpTracer = p;*/ }
	//OpTracer_t &opTracer() const { return *mpOpTracer; }

	void TracePtrForward() const;
	void	TraceArgPtrs() const;
	void	TracePtrBase(AnalyzePtrs_t &);
	void	TracePtrArgs() const;
private:
	void __tracePtrForward() const;
	int	TracePtrZ(FieldPtr pEntryOp) const;//, bool bThis);
	//int __tracePtr(PTR_t *pPtrInf) const;
	//int	__traceBase(PTR_t *pPtrInf) const;
	int		TracePtr1() const;
	int		TraceIndexStep(HOP) const;

};

class AnalyzePtrs_t : public FuncTracer_t
{
	std::set<HOP>		m_arrPtrs;
public:
	AnalyzePtrs_t(const FuncTracer_t &ft) : FuncTracer_t(ft){}
	AnalyzePtrs_t(const FuncInfo_t &fi, PathOpTracer_t &tr)
		: FuncTracer_t(fi, tr)
	{
	}
	STAGE_STATUS_e run();//FieldMap &);
	void addOp(HOP hOp)
	{
		m_arrPtrs.insert(hOp);
	}
};