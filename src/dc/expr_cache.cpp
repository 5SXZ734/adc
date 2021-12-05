#include "expr_cache.h"
#include "type_funcdef.h"
#include "expr_term.h"
#include "cc.h"

//ExprCache_t

void ExprCache_t::resetz()
{
	mTypesMgr.clear();
	mOuts.Reset();
	mpSelOut = nullptr;
	mSelSet.reset();
	mNoSSIDArgs.clear();
	//mpRootOut = nullptr;
}

void ExprCache_t::setSelOut(const Out_t *pOut)
{
	mpSelOut = pOut;
	mSelSet.reset();
}

void ExprCache_t::updateSelSet(FuncInfo_t &FI, ADDR funcBase)
{
	assert(mSelSet.empty());
	FI.VAListFromOp(mSelSet, mpSelOut, funcBase);
}

const Arg2_t *ExprCache_t::findNoSSIDArg(const DcInfo_t &DI, CGlobPtr g, CFieldPtr pField)
{
	assert(FuncInfo_t::IsLocalArg(pField) && FuncInfo_s::SSIDx(pField) == SSID_NULL);
	NoSSIDArgsMapCIt it(mNoSSIDArgs.find(pField));
	const Arg2_t *pArg(nullptr);
	if (it != mNoSSIDArgs.end())
	{
		pArg = &it->second;
	}
	else
	{
		//go thru all args
		CallingConv_t CC(DI, g);
		for (FuncCCArgsCIt<> i(*g->typeFuncDef(), CC); i; ++i)
		{
			if (FuncInfo_s::SSIDx(i.field()))
				continue;
			if (i.field() != pField)
				if (mNoSSIDArgs.find(i.field()) != mNoSSIDArgs.end())//already there?
					continue;
			std::pair<NoSSIDArgsMapCIt, bool> ret(mNoSSIDArgs.insert(std::make_pair(i.field(), Arg2_t((OPC_t)i.ssid(), i.offset(), i.size()))));
			assert(ret.second);
			if (i.field() == pField)
				pArg = &ret.first->second;
		}
	}
	assert(pArg);
	return pArg;
}

Arg2_t ExprCache_t::fieldToArg(const FuncInfo_t &fi, CFieldPtr pField)
{
	if (!FuncInfo_t::IsLocalArg(pField) || FuncInfo_s::SSIDx(pField))
		return Arg2_t((OPC_t)FuncInfo_s::SSIDx(pField), FuncInfo_s::address(pField), pField->size());
	return *findNoSSIDArg(fi, fi.FuncDefPtr(), pField);
}


//////////////////////////////////////////////////////// ExprCacheEx_t

void ExprCacheEx_t::reset()
{
	mOpCache.clear();
	ExprCache_t::resetz();
}

Out_t *ExprCacheEx_t::findOpExpr(HOP pOp) const
{
	OpCacheMapCIt i(mOpCache.find(pOp));
	if (i != mOpCache.end())
		return i->second.pOut;
	return nullptr;
}

ExprCacheEx_t::OpCacheMapIt
ExprCacheEx_t::addOpExpr(HOP pOp, GlobPtr iCFunc, Out_t *pOut, const std::set<ADDR> &addresses)
{
CHECK(pOp->ins().off() == 97)
STOP
	std::pair<OpCacheMapIt, bool> ret;
	ret = mOpCache.insert(std::make_pair(pOp, Elt(iCFunc, pOut, 0, addresses)));
	assert(ret.second);
	return ret.first;
}

void ExprCacheEx_t::addProblem(HOP pOp, ProblemInfo_t f)
{
	if (f)
	{
		OpCacheMapIt i(mOpCache.find(pOp));
		if (i == mOpCache.end())
			i = addOpExpr(pOp, nullptr, nullptr, std::set<ADDR>());
		uint32_t g(i->second.problem | f);
		i->second.problem = (ProblemInfo_t)g;
	}
}

ProblemInfo_t ExprCacheEx_t::findProblem(HOP pOp) const
{
	OpCacheMapCIt i(mOpCache.find(pOp));
	if (i != mOpCache.end())
		return i->second.problem;
	return 0;
}

int ExprCacheEx_t::qualifyAddress(CHOP pOp, ADDR va) const
{
	OpCacheMapCIt i(mOpCache.find(pOp));
	if (i != mOpCache.end())
	{
		const std::set<ADDR> &a(i->second.addresses);
		if (a.find(va) != a.end())
			return 1;
	}
	return 0;
}




