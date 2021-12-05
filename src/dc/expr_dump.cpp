
#include "expr_dump.h"
#include "expr_simpl.h"
#include "expr_ptr.h"
#include "dump.h"
#include "dump_file.h"
#include "dump_func.h"

///////////////////////////////////
// TExprDump

template <typename TBASE>
TExprDump<TBASE>::TExprDump(const FuncInfo_t &r, CHOP pOp, ExprCacheEx_t &rExpr, int id, bool bExpanded)
	: TBASE(r, /*pOp,*/ 0, rExpr),
	mOpID(id),
	//mrFunc(rFunc),
	//mrFuncDef(*rFunc.func def()),
	mbExpanded(bExpanded)
{
}

template <typename TBASE>
void TExprDump<TBASE>::dump(int i, const Out_t *pSelf, ESimplifyOutcome eOut) const
{
	assert(!pSelf->mpU);

	ExprCache_t ex(TBASE::PtrSize());
	DisplayLine_t di(ex);
	adcui::UDispFlags f(di.flags());
	f.clearL(adcui::DUMP_COLORS | adcui::DUMP_FONTS);
	f.setL(adcui::DUMP_SUBEXPR);
	di.setFlags(f);

	//?LogExp(pSelf, ID);

	if (i == 0)
	{
		std::cout << "\n\t*** ";
		std::cout << TBASE::FieldName(DockField());
		std::cout << ":" << mOpID;
		std::cout << " ***\n\n";
	}

	FileInfo_t dci(this->DcRef(), this->mrFileDef);
	FileDumper_t fd(dci, di, nullptr, nullptr, false);

	//FileDumper_t dumper0(di, false, false);
	FuncDumper_t dumper(*this, &fd, fd.indent(), fd.disp(), fd.ctx(), fd.ed());

	dumper.OutExpr(pSelf->mpR);
	std::string str(di.finalizeAsString());

	std::cout << i << ") ";
	if (eOut != SIMPL_NULL)
		std::cout << " " << ESimplifyOutcome2string(eOut) << ":\t";
	else
		std::cout << "\t";
	std::cout << str;
CHECK(i == 13)
STOP
	std::cout << std::endl;

	if (mbExpanded)
		writeExpr(pSelf, dumper);

	TBASE::dump(i, pSelf, eOut);
}

template <typename TBASE>
void TExprDump<TBASE>::writeExprTerm(const Out_t *pOut, int &i, int &indent, FuncDumper_t &dumper) const
{
	if (!pOut)
		return;

	if (pOut->mpU)//do not dump a root
	{
		Display_t &di(dumper.disp());
		di.clearg();
		dumper.OutputTabT(1);
		dumper.OutputWs(indent, ". ");
		dumper.OutputTermName(pOut);
		std::cout << di.asString() << std::endl;
	}

	assert(indent >= 0);
	i++;

	indent++;
	writeExprTerm(pOut->mpL, i, indent, dumper);
	writeExprTerm(pOut->mpR, i, indent, dumper);
	indent--;
}

template <typename TBASE>
void TExprDump<TBASE>::writeExpr(const Out_t *pOut, FuncDumper_t &dumper) const
{
#if(0)
	//DeleteAllItems();
	int indent(0);
	int i(0);
	writeExprTerm(pOut, i, indent, dumper);
	//Invalidate();
#else
	for (Out_t::Iterator i(pOut); i; i++)
	{
		Out_t &rOut(*i);
		if (rOut.mpU)//do not dump a root
		{
			//Display_t &di(dumper.disp());
			DisplayLine_t &di(dynamic_cast<DisplayLine_t &>(dumper.disp()));
			di.clearg();
			dumper.OutputTabT(1);
			dumper.OutputWs(i.level(), ". ");
			dumper.OutputTermName(&rOut);
			//std::cout << di.asString() << std::endl;
			std::cout << di.finalizeAsString() << std::endl;
		}
	}
#endif
}

//instantiation
template class TExprDump<EXPRSimpl_t>;
template class TExprDump<EXPRPtrSimpl_t>;





///////////////////////////////////////////////////////
// TExprDump2view

template <typename TBASE>
TExprDump2view<TBASE>::TExprDump2view(const FuncInfo_t &r, CHOP pOp, adcui::UDispFlags flags, ExprCacheEx_t &rExpr, int id, MyStreamUtil &ssu)
	: TBASE(r, /*pOp,*/ flags, rExpr),
	ID(id),
	m_ssu(ssu)
{
	if (pOp)
	{
		TBASE::SetNo(TBASE::OpNo(TBASE::PrimeOp(pOp)));
		if (TBASE::IsDeadEx(pOp))
			TBASE::mbDeadOp = true;
	}
}

template <typename TBASE>
void TExprDump2view<TBASE>::dump(int i, const Out_t *pSelf, ESimplifyOutcome eOut) const
{
	ExprCache_t ex(TBASE::PtrSize());
	DisplayLine_t di(ex);// , nullptr);
	adcui::UDispFlags f(di.flags());
	f.clearL(adcui::DUMP_COLORS | adcui::DUMP_FONTS);
	f.setL(adcui::DUMP_SUBEXPR);
	f.setL(adcui::DUMP_PCALLS);
	di.setFlags(f);

	FileInfo_t fileInfo(this->DcRef(), this->mrFileDef);
	FileDumper_t gd(fileInfo, di, nullptr, nullptr, nullptr);

	FuncDumper_t dumper(*this, &gd, gd.indent(), gd.disp(), gd.ctx(), gd.ed());

	if (pSelf->mpR)
		dumper.OutExpr(pSelf->mpR);
	else if (pSelf->mpL)
		dumper.OutExpr(pSelf->mpL);//GOTO,RET?
	else
		return;

	std::string str(di.finalizeAsString());

	std::ostringstream ss;

	ss << i << "\t";
	if (eOut != SIMPL_NULL)
		ss << ESimplifyOutcome2string(eOut);
	ss << "\t";
	ss << str;

CHECK(i == 19-1)
STOP
	//ss << endl;

	m_ssu.WriteString(ss.str());
	writeExpr(pSelf, dumper);
	
	TBASE::dump(i, pSelf, eOut);
}

template <typename TBASE>
void TExprDump2view<TBASE>::writeExprTerm(const Out_t *pOut, int &i, int &indent, FuncDumper_t &dumper) const
{
	if (!pOut)
		return;

	DisplayLine_t &di(dynamic_cast<DisplayLine_t &>(dumper.disp()));
	di.clearg();
	dumper.OutputTabT(indent + 2);
	dumper.OutputTermName(pOut);
	dumper.OutputTabT(1);
	dumper.OutputOutType(TBASE::TypOf(pOut));
	if (pOut->dockOp())
	{
		dumper.OutputTabT(1);
		MyString s;
#if(0)
		ADDR va(OpVA(pOut->dockOp()));
		s = Int2Str(va, I2S_HEXA);
#else
		if (TBASE::IsArgOp(pOut->dockOp()))
			s = "e";
		else
			s = NumberToString(TBASE::OpNo(pOut->dockOp()));
#endif
		dumper.dumpStr(s.c_str());
	}

	if (pOut->mpU)//do not dump a root
		m_ssu.WriteString(di.finalizeAsString());

	assert(indent >= 0);
	i++;

	indent++;
	writeExprTerm(pOut->mpL, i, indent, dumper);
	writeExprTerm(pOut->mpR, i, indent, dumper);
	indent--;
}

template <typename TBASE>
void TExprDump2view<TBASE>::writeExpr(const Out_t *pOut, FuncDumper_t &dumper) const
{
	//DeleteAllItems();
	int indent(0);
	int i(0);
	writeExprTerm(pOut, i, indent, dumper);
	//Invalidate();
}

//instantiation
//template class TExprDump2view<EXPRptr_t>;
template class TExprDump2view<EXPRSimpl_t>;
template class TExprDump2view<EXPRPtrSimpl_t>;


