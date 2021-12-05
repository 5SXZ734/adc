#pragma once

#include "expr.h"

enum ESimplifyOutcome : unsigned;
class FuncDumper_t;

template <typename TBASE>
class TExprDump : public TBASE
{
	int mOpID;
	bool	mbExpanded;
public:
	TExprDump(const FuncInfo_t &, CHOP, ExprCacheEx_t &, int id, bool bExpanded);
protected:
	virtual void dump(int i, const Out_t *pSelf, ESimplifyOutcome eOut) const;
private:
	void writeExprTerm(const Out_t *pOut, int &i, int &indent, FuncDumper_t &dumper) const;
	void writeExpr(const Out_t *pOut, FuncDumper_t &dumper) const;
};

template <typename TBASE>
class TExprDump2view : public TBASE
{
	int ID;
	MyStreamUtil&	m_ssu;
public:
	TExprDump2view(const FuncInfo_t &, CHOP, adcui::UDispFlags uflags, ExprCacheEx_t &, int id, MyStreamUtil &);
protected:
	virtual void dump(int i, const Out_t *pSelf, ESimplifyOutcome eOut) const;
	virtual bool checkDump(int a, int b) const { return (a == b); }
private:
	void writeExprTerm(const Out_t *pOut, int &i, int &indent, FuncDumper_t &dumper) const;
	void writeExpr(const Out_t *pOut, FuncDumper_t &dumper) const;
};



