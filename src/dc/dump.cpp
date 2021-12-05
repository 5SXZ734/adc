#include "dump.h"
#include <set>
#include "shared/defs.h"
#include "shared/link.h"
#include "shared/misc.h"
//#include "shared/action.h"
#include "db/mem.h"
#include "interface/IADCGui.h"
#include "dump_file.h"
#include "expr_term.h"

using namespace std;

//////////////////////////////////////DumpInfo_t

/*Complex_t *DumpInfo_t::contAtLine(unsigned line) const
{
	dump_pair_t z(line, nullptr);
	std::vector<dump_pair_t>::const_iterator i(std::upper_bound(mvCplx.begin(), mvCplx.end(), z));
	while (i != mvCplx.begin())//why a loop?
	{
		i--;
		const DumpChunk_t *a((*i).pChunk);
		if (a->chunkLine() <= line && line < a->chunkLine() + a->lines())
			return a->cplx();
	}
	return nullptr;
}*/

#ifdef _DEBUG
bool Display_t::gbDraftTest = false;
#endif
bool Display_t::gbNoColorMode = false;


void FileInfo_t::SetupLocus(ProbeExIn_t& aProbe)
{
	Locus_t& aLoc(aProbe.locus());
	assert(aLoc.empty());

	GlobPtr g(aProbe.scopeFuncDef());
	if (g)
	{
		if (!ProtoInfo_t::IsStub(g))
		{
			FuncInfo_t FI(*this, *g);
			if (aProbe.opLine())
				FI.LocusFromOp(aProbe.opLine(), aLoc);
			else
				FI.LocusFromVA_2(FI.OwnerProc(), FI.DockAddr(), aLoc);
		}
		else
		{
			ProtoInfo_t PI(*this, g);
			LocusFromVA_2(PI.DcInfo_t::OwnerProc(DockField(g)), DockAddr(g), aLoc);
		}
	}
	else
	{
		if (aProbe.atPos().is_set())
		{
			TypePtr pScope(aProbe.scope());
			if (!pScope)
				pScope = PrimeSeg();//?
			if (!pScope->typeSeg())
				LocusFromVA_2(pScope, aProbe.atPos().get(), aLoc);
			else
				LocusFromVA_1(pScope, aProbe.atPos().get(), aLoc);
			//should I care about a picked field?
		}
		else
		{
			CFieldPtr pField(aProbe.pickedFieldDecl());
			if (pField)
			{
				if (pField->isExported())
				{
					FieldPtr pImpField(FromExportedField(pField));
					if (pImpField)
						pField = pImpField;
				}
				LocusFromVA((FieldPtr)pField, aLoc);//do not change a folder
			}
		}
	}

	if (aLoc.empty())
	{
		TypePtr iFrontSeg(PrimeSeg());
		if (aProbe.locality().scoping == adcui::LocusId_STRUC_HEADER)
		{
			aLoc.push_front(Frame_t(aProbe.scope(), 0, nullptr));
			terminalFieldAt(aLoc);
		}
		else if (aProbe.locality().scoping & adcui::LocusId_STRUC__MASK)
		{
			ROWID r(0);
			if (aProbe.pickedFieldDecl())
				r = aProbe.pickedFieldDecl()->_key();
			else if (aProbe.atPos().is_set())
				r = aProbe.atPos().get();
			else if (aProbe.obj())
			{
				CFieldPtr pField(aProbe.obj()->objField());
				if (pField && IsGlobal(pField))
				{
					LocusFromVA_2(pField->owner(), pField->_key(), aLoc);
					return;
				}
			}
			//DA_t da(r, 0, 0); 
			//Block_t rb;
			aLoc.push_back(Frame_t(aProbe.scope(), (ADDR)r, nullptr));
			terminalFieldAt(aLoc);//scope(), da, *this, rb);
//?			setTypesMap(findTypeMgr(iFrontSeg));
		}
		else
		{
			LocusFromVA_1(iFrontSeg, iFrontSeg->base(), aLoc);//?at the seg base?
			unsigned offs;
			aLoc.stripToSeg(offs);
			assert(offs == 0);
		}
	}
}


////////////////////////////////////// DisplayUI_t

DisplayUI_t::DisplayUI_t(CFolderPtr pFile, adcui::UDispFlags flags, bool bDraftMode, OpType_t ptrSize)
	: Display_t(flags, bDraftMode),
	mpFile(pFile),
	mpdi(new DumpInfo_t(true)),
	mpSyncOpLine(HOP()),
	miSeekObjLine(-1),
	mCtrlNum(0),
	mLineBegPos(0),
	mrExprCache(ptrSize)
{
	//if (mpExprCache)
		//mpExprCache->AddRef();
}

DisplayUI_t::~DisplayUI_t()
{
	delete mpdi;
	//if (mpExprCache)
		//mpExprCache->Release();
}

void DisplayUI_t::ClearX(bool bAll)
{
	mrExprCache.reset();
	setCommentLevel(0);
	if (bAll)
		mpdi->clear();
}

ProblemInfo_t DisplayUI_t::problemWith(CHOP pOp) const
{
//	if (!mpExprCache)
//		return 0;
	return mrExprCache.findProblem(pOp);
}

int DisplayUI_t::checkAddress(CHOP pOp, ADDR va) const
{
//	if (!mpExprCache)
//		return 0;
	return mrExprCache.qualifyAddress(pOp, va);
}

void DisplayUI_t::newLine()
{
	os() << std::endl;
	//reset C++ comment ('//') for the new line
	setCommentLevel(0);
	mLineBegPos = (long)os().tellp();
	mCtrlNum = 0;//reset
	AddDump();
}

const char *DisplayUI_t::GetDataAtLine(int line, DumpContext_t *pctx) const
{
	return mpdi->dataAtLine(line, pctx);
}

bool DisplayUI_t::dumpSymOp(OpPtr pOp)
{
#ifdef _DEBUG
	if (IsDraftTest())
	{
#if(NO_OBJ_ID)
		os() << "<OPREF:@" << std::hex << FuncInfo_t::OpVA(pOp);
		os() << ">";
#else
		os() << "<OPREF:" << std::dec << pOp->ID() << ">";
#endif
		return true;
	}
#endif
	if (draftMode())
	{
		os() << (char)adcui::SYM_OPREF;
		os().write((char *)&pOp, sizeof(OpPtr));
		checkSeekOp(pOp);
		return true;
	}
	return false;
}

bool DisplayUI_t::dumpSym(adcui::SYM_e sym, CTypePtr pTypeObj)
{
#ifdef _DEBUG
	if (IsDraftTest())
	{
		const char *ps("?");
		switch (sym)
		{
		case adcui::SYM_TYPEREF: ps = "TYPEREF"; break;
		case adcui::SYM_TYPEDEF: ps = "TYPEDEF"; break;
		case adcui::SYM_STRUCEND: ps = "STRUCEND"; break;
		case adcui::SYM_IMPTYPEDEF: ps = "IMPTYPEDEF"; break;
		default:
			break;
		}
		os() << "<" << ps << ":" << pTypeObj->ID() << ">";
		return true;
	}
#endif

	if (draftMode())
	{
		char c((char)sym);
		os() << c;
		os().write((char *)&pTypeObj, sizeof(Obj_t *));
		return true;
	}
	return false;
}

/*bool DisplayUI_t::DumpExtSym(const ExpFieldInfo_t &a)
{
	adcui::SYM_e sym(adcui::SYM_EXTCLSFLD);
	if (DRAFT_TEST)
	{
	const char *ps("?");
	switch (sym)
	{
		case adcui::SYM_EXTCLSFLD: ps = "EXTCLSFLD"; break;
		default:
			break;
	}
	}
	if (draftMode())
	{
		mos << (char)sym;
		mos.write((char *)&a, sizeof(ExpFieldInfo_t));
		return true;
	}
	return false;
}*/

bool DisplayUI_t::dumpSym(adcui::SYM_e sym, CGlobPtr iGlob)
{
#ifdef _DEBUG
	if (IsDraftTest())
	{
		const char *ps("?");
		switch (sym)
		{
		case adcui::SYM_FUNCDECL: ps = "FUNCDECL"; break;
		case adcui::SYM_FUNCDEF: ps = "FUNCDEF"; break;
		case adcui::SYM_GLBDECL: ps = "GLBDECL"; break;
		case adcui::SYM_GLBDEF: ps = "GLBDEF"; break;
		case adcui::SYM_IMPGLB: ps = "IMPGLB"; break;
		case adcui::SYM_IMPCLSGLB: ps = "IMPCLSGLB"; break;
		default:
			break;
		}
		os() << "<" << ps << ":" << iGlob->ID() << ">";
		return true;
	}
#endif
	if (draftMode())
	{
		os() << (char)sym;
		os().write((char *)&iGlob, sizeof(FieldPtr ));
		return true;
	}
	return false;
}

bool DisplayUI_t::dumpSym(adcui::SYM_e sym, CGlobPtr iGlob, int off)
{
#ifdef _DEBUG
	if (IsDraftTest())
	{
		const char *ps("?");
		switch (sym)
		{
		case adcui::SYM_VFUNCDECL: ps = "VFUNCDECL"; break;
		case adcui::SYM_VTBLDECL: ps = "VTBLDECL"; break;
		case adcui::SYM_IMPVTBLDECL: ps = "IMPVTBLDECL"; break;
		default:
			break;
		}
		os() << "<" << ps << ":" << iGlob->ID() << ">";
		return true;
	}
#endif
	if (draftMode())
	{
		os() << (char)sym;
		os().write((char*)&iGlob, sizeof(FieldPtr));
		os().write((char*)&off, sizeof(off));
		return true;
	}
	return false;
}

bool DisplayUI_t::dumpSym(adcui::SYM_e sym, CFieldPtr pField)
{
#ifdef _DEBUG
	if (IsDraftTest())
	{
		const char *ps("?");
		switch (sym)
		{
		case adcui::SYM_FLDREF: ps = "FLDREF"; break;
		case adcui::SYM_CONSTREF: ps = "CONSTREF"; break;
		case adcui::SYM_IMPFLDREF: ps = "IMPFLDREF"; break;
		//case adcui::SYM_LABELDECL: ps = "LABELDECL"; break;
		case adcui::SYM_FLDDECL: ps = "FLDDECL"; break;
		case adcui::SYM_FLDDECL0: ps = "FLDDECL0"; break;
		case adcui::SYM_FLDDEF: ps = "FLDDEF"; break;
		//case adcui::SYM_FLDINST: ps = "FLDINST"; break;
		case adcui::SYM_GAP: ps = "GAP"; break;
		default:
			break;
		}
		//	mos << "<" << ps << ":" << fileInfo().VA2STR(pField->address()) << ">";
		os() << "<" << ps << ":" << pField->ID() << ">";
		return true;
	}
#endif
	if (draftMode())
	{
		os() << (char)sym;
		os().write((char *)&pField, sizeof(FieldPtr ));
		return true;
	}
	return false;
}

bool DisplayUI_t::dumpSymPath(PathPtr pPath)
{
#ifdef _DEBUG
	if (IsDraftTest())
	{
		os() << "<PATHREF:" << FuncInfo_s::StrNo(pPath) << ">";
		return true;
	}
#endif
	if (draftMode())
	{
		os() << (char)adcui::SYM_PATHREF;
		os().write((char *)&pPath, sizeof(PathPtr));
		return true;
	}
	return false;
}

















/*
//FONT

struct _Font { int n; };
inline _Font FONT(int __n)
{
_Font t;
t.n = __n;
return t;
}
template<typename _CharT, typename _Traits>
std::basic_ostream<_CharT, _Traits>& operator<<(std::basic_ostream<_CharT, _Traits>& __os, _Font __f)
{
if (g_ pDI)
g_ pDI->PushFont(__f.n);
return __os;
}

//TABS
std::ostream &TAB(std::ostream &stream);

struct _Tabs { int num; };
inline _Tabs TABS(int __n)
{
_Tabs t;
t.num = __n;
return t;
}
template<typename _CharT, typename _Traits>
std::basic_ostream<_CharT, _Traits>& operator<<(std::basic_ostream<_CharT, _Traits>& __os, _Tabs __f)
{
if (__f.num > 0)
while (__f.num--)
__os << TAB;
return __os;
}


//COLOR
struct _Color { int num; };
inline _Color COLOR(int __n)
{
_Color t;
t.num = __n;
return t;
}
template<typename _CharT, typename _Traits>
std::basic_ostream<_CharT, _Traits>& operator<<(std::basic_ostream<_CharT, _Traits>& __os, _Color __f)
{
if (g_ pDI)
g_ pDI->PushColor(__f.num);
return __os;
}*/

//OMANIP_int TABS(int);
//OMANIP_int COLOR(int);
//OMANIP_int FONT(int);


struct flags_t
{
	const char * name;
	unsigned flag;
	const char * desc;
};

static flags_t dispflags[] =
{
	{ "b", adcui::DUMP_BLOCKS, "(+) blocks (IR view only)" },
	{ "d", adcui::DUMP_DEADCODE, "(-) show dead code" },
	{ "i", adcui::DUMP_INDATA, "(-) show data inflow" },
	{ "f", adcui::DUMP_FPUTOP, "(-) FPU register top" },
	{ "l", adcui::DUMP_LNUMS, "(-) lines" },
	{ "ns", adcui::DUMP_NOSTRUCS, "(-) no structures" },
	//{ "nt", adcui::DUMP_NOTYPES, "(-) no types" },
	//{ "o", adcui::DUMP_OUTDATA, "(-) show data outflow" },
	{ "p", adcui::DUMP_PATHS, "(-) numberize paths" },
	{ "s", adcui::DUMP_STACKTOP, "(-) program stack top" },
	{ "t", adcui::DUMP_TABS, "(+) tabs" },
	{ "u", adcui::DUMP_UNFOLD, "(-) unfolded backend" },//no pass 2
	{ "x", adcui::DUMP_NULL, "(-) cached" },
	{ "y", adcui::DUMP_CASTS, "(-) types" },
	{ "&", adcui::DUMP_LOGICONLY, "(-) logics only" },
	{ "loc", adcui::DUMP_STRUCLOCS, "(+) dump locals strucs" }
};

void dispflags_usage(std::ostream &os)
{
	for (int i = 0; i < sizeof(dispflags) / sizeof(flags_t); i++)
	{
		os << "\t/" << dispflags[i].name << "\t - " << dispflags[i].desc << std::endl;
	}
}

#define checkopt(arg)	(strcmp(opt, arg) == 0)

unsigned dispflag_check(const char * opt)
{
	unsigned f = 0;

	for (int i = 0; i < sizeof(dispflags) / sizeof(flags_t); i++)
	{
		if (checkopt(dispflags[i].name))
		{
			f = dispflags[i].flag;
			break;
		}
	}

	return f;
}

char * dispflags2str(unsigned f, const char * sep)
{
	static char buf[256];
	buf[0] = 0;

	int j = 0;
	while (f)
	{
		if (f & 1)
		{
			for (int i = 0; i < sizeof(dispflags) / sizeof(flags_t); i++)
			{
				if (dispflags[i].flag == (1 << j))
				{
					if (sep != nullptr)
						strcat(buf, sep);
					strcat(buf, dispflags[i].name);
					break;
				}
			}
		}
		j++;
		f >>= 1;
	}

	return buf;
}

/*std::ostream &TAB(std::ostream &os)
{
//if (1)
if (!g_ pDI || g_ pDI->testOpt1(adcui::DUMP_TABS))
{
os << '\t';
}
else
{
do {
os << " ";
} while (os.tellp() & TABMASK);
}

return os;
}*/

/*std::ostream& TABS(std::ostream& os, int nNum)
{
if (nNum > 0)
while (nNum--)
os << TAB;
return os;
}

OMANIP_int TABS(int nNum)
{
return OMANIP_int(TABS, nNum);
}

std::ostream & COLOR(std::ostream &os, int nColorId)
{
if (g_ pDI)
g_ pDI->PushColor( nColorId );
return os;
}

OMANIP_int COLOR(int nColorId)
{
return OMANIP_int(COLOR, nColorId);
}

std::ostream &FONT(std::ostream &os, int nFontId)
{
if (g_ pDI)
g_ pDI->PushFont(nFontId);

return os;
}

OMANIP_int FONT(int nFontId)
{
return OMANIP_int(FONT, nFontId);
}
*/

int FuncInfo_t::GetInvolvedOps(HOP pSelf, set<HOP> &m) const
{
	int count(0);
	HOP pOp = pSelf;
	while (1)
	{
		if (!IsCodeOp(pOp) && !IsArgOp(pOp))
		{
			if (IsCallOutOp(pOp))
			{
				for (XOpList_t::Iterator i(pOp->m_xout); i; i++)
					count += GetInvolvedOps(i.data(), m);
				//m.insert(PrimeOp(pOp));
			}
			break;
		}
		else if (IsPrimeOp(pOp))
		{
			if (IsOpRootEx(pOp) || !pOp->XOut())
			{
				m.insert(pOp);
			}
			else
			{
				for (XOpList_t::Iterator i(pOp->m_xout); i; i++)
					count += GetInvolvedOps(i.data(), m);
			}
			break;
			//if (pOp->m_xout.check_count(1) > 0)
			//return 0;//!!?
			//pOp = pOp->XOut()->data();
		}
		else
			pOp = PrimeOp(pOp);
	}
	return count;
}

/*#ifdef _DEBUG
void LogExp(Out_t * pSelf, int strN)
{
	//	assert(IsRoot());
	//	if (mpL)
	{
		__Dout.Log(pSelf, strN);
	}
}
#endif*/

/*int DisplayUnfold_t::CheckLabel(Func Info_t *pfi, HPATH  pSelf) const
{
	assert(pSelf->IsTerminal());
	if (pSelf->IsFirstEx())
		return 0;

	if (!pSelf->m.Lab elAt())
		return 0;

	if (isUnfoldMode())
	{
		if (IsObj Visible(pfi, pSelf->m.Lab elAt()))
			return 1;
		return 0;
	}

	HPATH pPath = pSelf;
	while (1)
	{
		if (!IsObj Visible(pfi, pPath->m.Lab elAt()))
			break;

		for (OpList_t::Iterator j(pPath->ops()); j; j++)
		{
			HOP pOp(j.data());
			if (!pOp->isHidden())
				if (IsOpRootEx(pOp))
					return 1;
		}

		pPath = pPath->NextEx();
		if (!pPath)
			break;
		if (pPath->m.Lab elAt())
			break;

	}

	return 0;
}*/

/*Out_t *DisplayUI_t::cachedExpression(HOP pSelf)
{
	return mpExprCache->findOpExpr(pSelf);
}

void DisplayUI_t::addCachedExpression(HOP pSelf, TypePtr pCFunc, Out_t *pOut, const std::set<ADDR> &addresses)
{
	mpExprCache->addOpExpr(pSelf, pCFunc, pOut, addresses);
}

void DisplayUI_t::addProblem(HOP pOp, ProblemInfo_t pi)
{
	mpExprCache->addProblem(pOp, pi);
}*/

void DisplayUI_t::pushColor(int colID)
{
	char c = (char)adcui::SYM_COLOR;
	char d = (char)colID;
	os() << c << d;
	addExtraChars(2);
}

void DisplayUI_t::pushWString(const wchar_t* ws, uint16_t ulen)
{
	char c = (char)adcui::SYM_WSTRING;
	os() << c;
	addExtraChars(1);

	//os().write((const char*)&ulen, sizeof(ulen));
	//os().write((const char*)ws, ulen * sizeof(wchar_t));
	//addExtraChars(ulen);
}

/*int DisplayUnfold_t::CheckLabelDead(const FuncInfo_t &rfi, FieldPtr  pSelf) const
{
	for (XFieldList_t::Iterator i(pSelf->xr efs()); i; i++)
	{
		HOP pOp(i.data());
		if (pOp->IsCodeOp())
			if (pOp->isHidden())//valid!!!
				continue;

		if (pOp->IsDataOp())
			return 0;

		HPATH pPath = pOp->Path();
		if (!pPath)
			return 0;

		if (CheckGOTOStatus5(rfi, pPath) == JUMP_GOTO)
			return 0;
	}

	return 1;//not visible
}*/


/*?
int Op_t::MoveOpUp()
{
	if (g_ pDI && !g_ pDI->isUnfoldMode())
		return 0;
	return MoveOpUp0();
}

int Op_t::MoveOpDown()
{
	if (g_ pDI && !g_ pDI->isUnfoldMode())
		return 0;
	return MoveOpDown0();
}*/




/*?uint32_t _A(uint32_t action)
{
	if (ISJMPIF(action))
		action = ACTN_JMPIF;
	else if (ISC ALL(action))
		action = ACTN_CALL;

	return action;
}*/





//////////////////////////
// O U T P U T


#if(0)
/*int Op_t::__checkOpenedGOTOs(bool bGotoOnly)
{
	assert(IsPri meOp());

	HOP pOp = this;
	while (1)
	{
		if (pOp->IsFirstEx())//first achieved
		{
			break;
		}

		if (pOp->Label())
		{
			for (XRefList_t *pXRef = pOp->Label()->m_pXRefs; pXRef; pXRef = pXRef->Nex t())
			{
				bool bSwitch = false;
				HOP pOp = pXRef->pOp;
				if ( pOp->IsDataOp() )
				{
					bSwitch = true;
					pOp = ((FieldPtr )pOp)->GetSwitchOp();
				}

//				if ( !pOp->opnd_BLOCK )
				{
					if (!pOp->CheckBreak())
					{
						if ( pOp->IsCondJump() )
						{
							if (!bGotoOnly)
							if (pOp->arg s()->CheckCount(1) <= 0)
								return 1;//actual
						}
						else
						{
							assert(pOp->IsGoto());
							if (bSwitch)
								return 1;
//							if (->GetGotoDestOp(pOp)->Label() == this)
								return 1;
						}
					}
				}


/ *				HOP __pOp = pXRef->pOp;
				if (__pOp->IsD ata())
					__pOp = __pOp->GetSwitchOp();
				else
				{
					if (__pOp->IsGoto())
						return 1;
					assert(ISJMPIF(__pOp->Action()));
					if (!__pOp->opnd_BLOCK)
						return 1;
					continue;
				}

				if (__pOp->__checkOpenedGOTOs())
					return 1;
* /			}
		}

		pOp = pOp->PrevEx();
		if (!pOp->isHidden())
			if (pOp->isRoot())
				if (!pOp->isSplit())
					break;

		if ( (pOp->IsGoto())
			|| (IsRetOp(pOp)) )
			break;//logic block end achived
	}

	return 0;
}*/
#endif








