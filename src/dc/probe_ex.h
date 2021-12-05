#pragma once

#include <set>
#include "shared/defs.h"
#include "db/mem.h"
#include "db/command.h"
#include "dc/op.h"
#include "dc/globs.h"

class Out_t;
class Obj_t;
class TypeProc_t;
class FuncDef_t;
class FileDef_t;
class Struc_t;
class Dc_t;
class Complex_t;
class ProjectEx_t;
class GlobObj_t;

class VAList : public std::set<ADDR>
{
	ADDR m_base;
	ADDR m_cur;
public:
	VAList() : m_base(-1), m_cur(-1) {}
	void setBase(ADDR a) { m_base = a; }
	void setCur(ADDR a) { m_cur = a; }
	VAList& operator=(const VAList& o)
	{
		std::set<ADDR>::operator=(o);
		m_base = o.m_base;
		m_cur = o.m_cur;
		return *this;
	}
	bool operator==(const VAList& o) const
	{
		if (size() != o.size()
			|| !std::equal(begin(), end(), o.begin()))
			return false;
		return (m_base == o.m_base && m_cur == o.m_cur);
	}
	bool operator!=(const VAList& o) const { return !operator==(o); }
	void reset() { return clear(); m_base = -1; m_cur = -1; }
	int check(ADDR a) const
	{
		if (a == m_cur)
			return 1;
		if (find(a) != end())
			return 2;
		return 0;
	}
	bool empty() const { return std::set<ADDR>::empty(); }
	int size() const { return (int)std::set<ADDR>::size(); }
	bool add(ADDR va) { return insert(va).second; }
	bool add1(ADDR offs)//return false if this branch can be skipped
	{
		if (offs == 0)
			return false;//skip entry ops
	//	if (FuncInfo_t::IsAuxOp(pOp))//never consider tmp ops addresses
	//		return true;
		return insert(offs).second;
	}
	void print() const
	{
		for (iterator i(begin()); i != end(); i++)
			fprintf(stdout, "%08X\n", (ADDR)*i);
		fprintf(stdout, "\n");
	}
};


struct ProbeEx_t : public Probe_t
{
protected:
	FolderPtr	mpFolder;
	int			my;				//line number
	OpPtr		mpOpLine;		//selection of a code line
	bool		mbUnfold;
	VAList		mCurVAs;

	Range_t		mRangeExpr;
	TypePtr		mpTypeDecl;		//indicate selection of struct's header
	GlobPtr		mpFuncDecl;		//indicate selection of func's header
	TypeBasePtr	mpScope;		//?
	//CFieldPtr	mpFieldDecl;		//if clicked on field
	CFieldPtr	mpImpFieldDecl;	//if the field is imported
	SetVal_t<ADDR>	mAtPos;		//probe's offset
	
public:
	ProbeEx_t()
		: mpFolder(nullptr),
		my(-1),
		mpOpLine(OpPtr()),
		mbUnfold(false),
		mpTypeDecl(nullptr),
		mpFuncDecl(nullptr),
		mpScope(nullptr),
		mpImpFieldDecl(nullptr)
	{
	}
	ProbeEx_t(const Locus_t &l)
		: Probe_t(l),
		mpFolder(nullptr),
		my(-1),
		mpOpLine(OpPtr()),
		mbUnfold(false),
		mpTypeDecl(nullptr),
		mpFuncDecl(nullptr),
		mpScope(nullptr),
		mpImpFieldDecl(nullptr)//,
		//mbProbing(false)
	{
	}
	ProbeEx_t(const Probe_t &o)
		: Probe_t(o),
		mpFolder(nullptr),
		my(-1),
		mpOpLine(OpPtr()),
		mbUnfold(false),
		mpTypeDecl(nullptr),
		mpFuncDecl(nullptr),
		mpScope(nullptr),
		mpImpFieldDecl(nullptr)//,
		//mbProbing(false)
	{
	}
	ProbeEx_t(const ProbeEx_t &o)
		: Probe_t(o),
		mpFolder(o.mpFolder),
		my(o.my),
		mpOpLine(o.mpOpLine),
		mbUnfold(o.mbUnfold),
		mCurVAs(o.mCurVAs),
		mRangeExpr(o.mRangeExpr),
		mpTypeDecl(o.mpTypeDecl),
		mpFuncDecl(o.mpFuncDecl),
		mpScope(o.mpScope),
		mpImpFieldDecl(o.mpImpFieldDecl),
		mAtPos(o.mAtPos)
	{
	}

	void setupLocality(bool);

	TypePtr pickedTypeDecl() const { return mpTypeDecl; }
	GlobPtr pickedFuncDecl() const { return mpFuncDecl; }

	void clear()
	{
		Probe_t::clear();
		mpFolder = nullptr;
		my = -1;
		mpOpLine = OpPtr();
		mbUnfold = false;
		mpTypeDecl = nullptr;
		mpFuncDecl = nullptr;
		mpScope = nullptr;
		mpImpFieldDecl = nullptr;
		//mbProbing = false;
	}
	//void print(std::ostream &os);
	ADDR VA(const Dc_t &) const;

	GlobPtr scopeFunc() const;
	TypePtr scopeStruc() const;
	TypePtr scope() const {
		//assert(mpScope->objTypeGlob());
		return (TypePtr)mpScope;
	}
	void setScope(CTypeBasePtr p){
		mpScope = (TypeBasePtr)p;
	}

	bool isUnfoldMode() const { return mbUnfold; }

	OpPtr opLine() const { return mpOpLine; }

	VAList& curVAs() { return mCurVAs; }
	const VAList& curVAs() const { return mCurVAs; }

	FileDef_t *fileDef() const;
	Dc_t *dcRef() const;

	virtual void clear_all(){
		clear();
	}
	int line() const { return my; }
	FolderPtr folder() const {
		return mpFolder;
	}
	void setFolder(CFolderPtr p) {
		mpFolder = (FolderPtr)p;
	}

	const SetVal_t<ADDR>& atPos() const { return mAtPos; }

	GlobPtr scopeFuncDef() const {
		return (scope() && scope()->typeFuncDef()) ? (GlobPtr)scope() : nullptr;
	}
	const Range_t& rangeExpr() const { return mRangeExpr; }
};


struct ProbeExIn_t : public ProbeEx_t//writer
{
public:
	ProbeExIn_t(int x, int y, bool unfold = false) {
		pickX(x);
		pickLine(y);
		mbUnfold = unfold;
	}

	void pickOpLine(OpPtr p) {
		mpOpLine = p;
	}
	void pickTypeDecl(CTypePtr p) {
		mpTypeDecl = (TypePtr)p;
	}
	void pickFuncDecl(CGlobPtr g) {
		mpFuncDecl = (GlobPtr)g;
	}
	void pickFieldDecl(CFieldPtr p) {
		mpFieldDecl = (FieldPtr)p;
	}
	void pickImpFieldDecl(CFieldPtr p) {
		mpImpFieldDecl = p;
	}
	void pickAddr(ADDR va) {
		mAtPos = va;
	}
	void pickValue(const value_t& v) {
		mValue = v;
	}
	void pickRange(int x, unsigned len){
		mRange.set(x, x + len);
	}
	void pickExprRange(int x, unsigned len){
		mRangeExpr.set(x, x + len);
	}
	void pickEntityId(adcui::Color_t e){
		mEntityId = e;
	}
	void pickLine(int y) {
		my = y;
	}
	void pickScope(CTypeBasePtr p){
		assert(!p || !p->typeProc());
		mpScope = (TypeBasePtr)p;
	}
	void pickX(int x) {
		mx = x;
	}
	SetVal_t<ADDR>& atPos() {
		return mAtPos;
	}
};



enum RedumpFlags { REDUMP_SRC = 1, REDUMP_H = 2, REDUMP_ALL = 3 };




