#include "ui_src_view.h"
#include <algorithm>
#include "qx/ConvertUTF.h"

#include "shared/table.h"
#include "shared/misc.h"
#include "db/ui_main.h"
#include "db/main.h"
#include "db/debug.h"
#include "db/type_proxy.h"

#include "proj_ex.h"
#include "dump_file.h"
#include "dump_func.h"
#include "info_dc.h"
#include "ana_main.h"
#include "probe_ex.h"
#include "ui_main_ex.h"
#include "info_class.h"



class ContextSrc_t : public ProbeEx_t
{
	ProbeEx_t mProbe;
public:
	ContextSrc_t(const Locus_t &aLoc, const ProbeEx_t& aProbe)
		: ProbeEx_t(aLoc),
		mProbe(aProbe)
	{
	}
	const ProbeEx_t& probe() const { return mProbe; }
};




SrcViewModel_t::SrcViewModel_t(CoreEx_t &rCore, const Dc_t &rDcRef, const Folder_t &rFile, adcui::UDispFlags flags)
	: mrCore(rCore),
	mrDcRef((Dc_t &)rDcRef),
	mrFile(rFile),
	mDispId(0),
	//mpCurentCodeLine(nullptr)
//	mbSyncMode(false),
	mpSubject(nullptr)
{
	using namespace adcui;
#define COLW(a, b)	(flags.testL(a) /*&& !bHeader*/ ? b : -b)
	mcols.setup(ISrcViewModel::CLMN_LNUMS, "L", COLW(DUMP_LNUMS, 3), CLMNFLAGS_HEADER);//line
	mcols.setup(CLMN_STACKTOP, "S", COLW(DUMP_STACKTOP, 3), CLMNFLAGS_HEADER);//conventional stack
	mcols.setup(CLMN_FPUTOP, "F", COLW(DUMP_FPUTOP, 3), CLMNFLAGS_HEADER);//fpu stack
	mcols.setup(CLMN_PATHS, "P", COLW(DUMP_PATHS, 3), CLMNFLAGS_HEADER);//paths
	mcols.setup(CLMN_CODE, "", 80);//source
	mcols.setup(CLMN_EXTRA, "", -8);//problems
#undef COLW

	//these must not appear in output
	flags.clearL(adcui::DUMP_LNUMS | adcui::DUMP_STACKTOP | adcui::DUMP_FPUTOP | adcui::DUMP_PATHS);
#if(0)
	flags.setL(adcui::DUMP_NOREAL80);
#endif
	setMode(flags, true, DUMPOS(0));

	//newIterator(IT_TOP);
	//newIterator(IT_CUR);
	//invalidate(0);
	//seekLineIt(0, 0);
	mFilePath = project().files().relPath(&mrFile);
}

SrcViewModel_t::~SrcViewModel_t()
{
//	if (probeSrc())
//		mrCore.setProbe(nullptr);
	deleteDisplay();// true);
	//delete mpLine;
	//mrGui.CloseDisplay(mFileId, false);
}

FileDef_t &SrcViewModel_t::FileDef() const
{
	return *DcInfo_t::FILEDEF(mrFile);
}

static MyString toLocationString(FolderPtr pFolder)
{
	MyStringList l(MyStringList::split(MODULE_SEP, FilesMgr0_t::relPath(pFolder)));
	if (l.empty())
		return "<ERROR>";
	std::ostringstream ss;
	ss << "<b>" << l.front() << "</b>";//module
	l.pop_front();
	if (!l.empty())
		ss << " (" + l.front() + HEADER_EXT + ")";
	return ss.str();
}

void SrcViewModel_t::tipInfoIt(adcui::DUMPOS it, int x, MyStreamBase &ss)
{
	MyStreamUtil ssh(ss);
	DisplayUI_t *pDisp(display());
	if (x <= 0)
	{
		MyDumpVisitor a;
		if (visit(it, a, true))
		{
			FileInfo_t GI(mrDcRef, FileDef());
			MyString sReason;
			if (a.op())
			{
				CTypePtr pCont((CTypePtr)a.cont());
				CGlobPtr iGlob(pCont->objGlob());// DcInfo_s::GlobFuncObj(pCont->parentField()));
				assert(iGlob && iGlob->func());
				FuncInfo_t FI(mrDcRef, *iGlob);// , FileDef());
				ProblemInfo_t f(display()->problemWith(a.op()));
				sReason = FI.findProblemStr(f, a.op(), true);
			}
			else if (a.field())
			{
				GI.CheckProblem(a.field(), sReason, IsHeader());
			}
			if (!sReason.empty())
				ssh.WriteString(sReason);
		}
		return;
	}

	ProbeExIn_t aTip(x, dumpos(it));

	//if (!mbSyncMode)
		//pDisp->setSyncOpLine(HOP());

	FileInfo_t fileInfo(mrDcRef, FileDef());

	//stage 1 - file level
	FileDumper_t dumper(fileInfo, *pDisp, &aTip, nullptr, false);

	dumper.Probe(aTip);

	//if (mbSyncMode)
	//pDisp->setSyncOpLine(pHit->pCodeLine);

	CObjPtr pObj(aTip.obj());
	if (pObj)
	{
		CFieldPtr pField(pObj->objField());
		if (pField)
		{
			/*?if (aTip.scope() && aTip.scope()->type Proc())
			{
			FuncInfo_t FI(mrDcRef, *GlobFuncObj(aTip.scope()->parentField()), FileDef());
			if (FuncInfo_t::IsLab el(pField))
			{
			MyString s(FI.DumpFieldDeclaration(pField));
			ssh.WriteString(s);
			return;
			}
			}*/

			FolderPtr pFolder(nullptr);
			FolderPtr pImpFolder(nullptr);
			TypePtr iModule(mrDcRef.module());
			MyString sExpName;
			if (!FuncInfo_s::IsLocal(pField))
			{
				CGlobPtr pGlob(DcInfo_s::IsGlobal(pField) ? DcInfo_s::GlobObj(pField) : nullptr);
				if (pGlob)
				{
					pFolder = pGlob->folder();//may be in another module
					if (pField->isTypeImp())
					{
						FieldPtr pExpField(fileInfo.ToExportedField(pField));
						if (pExpField)
						{
							//pField = pExpField;
							pImpFolder = DcInfo_s::GlobObj(pExpField)->folder();
							TypePtr pImpModule(ModuleInfo_t::ModuleOf(pImpFolder));
							if (pExpField->nameless())
							{
								DcInfo_t DI2(*DCREF(pImpModule));
								sExpName = fileInfo.AutoName(pExpField, nullptr);
							}
						}
					}
				}
				else
				{
					iModule = ModuleInfo_t::ModuleOf(pField->owner());
				}
			}

			std::ostringstream ss;
			ss << "<p style='white-space:pre'>";
			Dc_t *pDC(DCREF(iModule));
			FileInfo_t FI(*pDC, FileDef());
			ss << FI.DumpFieldDeclaration(pField);

			if (FuncInfo_s::IsLocal(pField))
			{
				GlobPtr ifDef(FileInfo_t::ContextFuncDef(pField));
				FuncInfo_t FI(mrDcRef, *ifDef);
				ss << "\t@ ";
				if (FuncInfo_s::IsLocalReg(pField))
					ss << FI.RegName(pField, true);
				else
				{
					int iOff(FuncInfo_s::localOffset(pField));
					if (iOff < 0)
					{
						ss << "RETADDR";
						ss << Int2Str(iOff, I2S_HEXA | I2S_SIGNED);
					}
					else
					{
						ss << "RETADDR+";
						ss << Int2Str(iOff, I2S_HEXA);
					}
				}
			}
			else if (pField->isGlobal())
			{
				if (!pFolder)
					if (!FuncInfo_t::IsLabel(pField))
						ss << " (undeclared identifier)";//not acquired object
			}

			if (pImpFolder)//if (pFolder && pDC != &mrDcRef)
			{
				ss << "\n\tImported from: " << toLocationString(pImpFolder);
				if (!sExpName.empty())
					ss << " => " << sExpName;
			}
			ss << "</p>";
			ssh.WriteString(ss.str());
		}
		else
		{
			TypePtr iType(pObj->objTypeGlob());
			if (iType)
			{
				std::ostringstream ss;
				ss << "<p style='white-space:pre'>";
				bool bImported(false);
				bool bUndeclared(false);
				TypePtr iModule(DcInfo_t::ModuleOfEx(iType));
				DcInfo_t DI(*DCREF(iModule));
				if (iType->typeProxy())
				{
					if (iModule == fileInfo.ModulePtr())
					{
						bImported = true;
						TypePtr iProxy(iType);
						iType = iProxy->typeProxy()->incumbent();
						if (!iType->isNested())//? || iType->owner()->typeNamespace())
						{
							MyString s, sIncumb;
							ProjectInfo_t::ChopName(fileInfo.TypePrettyName(iProxy), s);
							ProjectInfo_t::ChopName(DI.TypePrettyName(iType), sIncumb);
							ss << "<i>" << s << "</i>";
							if (s != sIncumb)
								ss << " (<font color=red>inconsistent naming</font>) ";
							ss << " ";
						}
						ss << "=> ";//for nested proxies, this will start a line
					}
					else
						ss << " *** ERROR! ***";
				}
				else if (iModule != fileInfo.ModulePtr())
				{
					bImported = true;
					if (!iType->isNested())
					{
						ss << "<i>" << fileInfo.StrucNameless(iType) << "</i>";
						ss << " (<font color=red>undeclared</font>) => ";
						bUndeclared = true;
					}
				}
				
				if (iModule)
				{
					const char *tag;
					if ((tag = iType->printType()) != nullptr)
					{
						if (iType->isEnum())
							tag = "enum";
						ss << tag;
						ss << " ";
					}
					MyString st(DI.TypeNameFull(iType, DcInfo_t::E_PRETTY_SCOPE, CHOP_SYMB).join());
					st = ReplaceAll(st, "<", "&lt;");
					st = ReplaceAll(st, ">", "&gt;");
					ss << st;
				}
				if (bImported)
				{
					MyString sLocat;
					FolderPtr pFolder(USERFOLDER(iType));
					if (pFolder)
						sLocat = toLocationString(pFolder);
					else
						sLocat = ModuleInfo_t::ModuleTitle(iModule);
					ss << "\n\tImported from: " << sLocat;
				}
				ss << "</p>";
				ssh.WriteString(ss.str());
			}
		}
	}
}

int SrcViewModel_t::lineFromItEx(adcui::DUMPOS it, MyStreamBase &ss)
{
	FileInfo_t FI(mrDcRef, FileDef());

	ContextSafeEx_t safe(mrCore);
	if (!safe.get())
		return -1;

	CTypeBasePtr iScope(safe.get()->scope());
	assert(!iScope || !iScope->typeProxy());
	FI.WriteGlobInfo(iScope, ss);

	ObjPtr pObj(safe.get()->obj());//pProbe->obj()
	if (pObj)
	{
		CTypeBasePtr iType(pObj->objType());
		if (!iType)
		{
			CGlobPtr pGlob(nullptr);
			CFieldPtr pField(pObj->objField());
			if (pField && DcInfo_s::IsGlobal(pField))
			{
				if (pField->isTypeImp())
				{
					FieldPtr pExpField(FI.ToExportedField(pField));
					pGlob = DcInfo_t::GlobObj(pExpField);
				}
				else
					pGlob = DcInfo_t::GlobObj(pField);

				if (pGlob)
					FI.WriteGlobInfo(pGlob, ss);
			}
		}
		else
		{
			if (iType->typeProxy())
				iType = iType->typeProxy()->incumbent();
			FI.WriteGlobInfo(iType, ss);
		}
	}

	return lineFromIt(it);
}

void SrcViewModel_t::lockRead(bool bLock)
{
	mrCore.main().lockProjectRead(bLock);
}

void SrcViewModel_t::lockWrite(bool bLock)
{
	mrCore.main().lockProjectWrite(bLock);
}

void SrcViewModel_t::deleteDisplay()
{
	mrCore.closeDisplay(mDispId);
}

DisplayUI_t *SrcViewModel_t::newDisplay(CFolderPtr pFile, adcui::UDispFlags flags)
{
	mDispId = mrCore.openDisplay(pFile, flags, mrDcRef.primeSeg()->typeSeg()->ptrSize());
	return display();
}

DisplayUI_t *SrcViewModel_t::display() const
{
	return mrCore.displayAt(mDispId);
}

bool SrcViewModel_t::InvalidateDump()
{
	bool bRet(display()->InvalidateDump(nullptr));
	return bRet;
}

adcui::DUMPOS SrcViewModel_t::newPosition()
{
	return mPos.newPos(new Data());
}

void SrcViewModel_t::deletePosition(adcui::DUMPOS iPos)
{
	delete mPos.deletePos(iPos);
}

adcui::DUMPOS SrcViewModel_t::posFromIter(ITER it)
{
	return mPos.posFromIter(it);
}

adcui::ITER SrcViewModel_t::newIterator(adcui::DUMPOS iPos)
{
	return mPos.newIter(iPos, new Data());
}

void SrcViewModel_t::deleteIterator(ITER it, bool bUpdatePos)
{
	delete mPos.deleteIter(it, bUpdatePos);
}

void SrcViewModel_t::copyIt(adcui::DUMPOS itTo, adcui::DUMPOS itFrom)
{
	dumpos(itTo) = dumpos(itFrom);
}

void SrcViewModel_t::seekLineIt(adcui::DUMPOS it, int line)
{
	int &rline(dumpos(it));
	rline = line;
	//validateIt(it);
}

int SrcViewModel_t::lineFromIt(adcui::DUMPOS it)
{
	return dumpos(it);
}

bool SrcViewModel_t::backwardIt(adcui::ITER it)
{
	if (iter(it) > 0)
	{
		iter(it)--;
		return true;
	}
	return false;
}

bool SrcViewModel_t::forwardIt(adcui::ITER it)
{
	assert(iter(it) >= 0);
	if (iter(it) < linesNum())//?
	{
		iter(it)++;
		return true;
	}
	return false;
}

void SrcViewModel_t::invalidate(bool b)
{
	if (b)
	{
		//#if(_INTERNAL)
		//			fprintf(stdout, "[INTERNAL] %s: flushing cache ...\n", __FUNCTION__);
		fprintf(stdout, "Refreshed.\n");
		fflush(stdout);
		//#endif
	}
	InvalidateDump();
}

const char *SrcViewModel_t::getDumpPosAtIt(adcui::DUMPOS it, DumpContext_t *pctx) const
{
	return display()->GetDataAtLine(dumpos(it), pctx);
}

/*bool SrcViewModel_t::validateIt(adcui::DUMPOS it)
{
	//?		long pos;
	//?		return display()->GetPosAtLine(dumpos(it), pos);
	return false;
}*/

const char *SrcViewModel_t::dataIt(adcui::DUMPOS it, bool bPlain)
{
	assert(0);
	/*		DumpContext_t dctx;
			const char *line(getDumpPosAtIt(it, &dctx));
			if (!line)
			return nullptr;

			//pd->SetCurObj(pd->hit()->obj());

			//TypePtr iFunc(nullptr);
			//if (pCplx && pCplx->type Proc())
			//iFunc = pCplx;

			FileInfo_t fileInfo(mrDcRef, FileDef());
			FileDumper_t fileDumper(fileInfo, *display(), &mProbe);

			int len;
			if (bPlain)
			mData = fileDumper.ExpandLinePlain(dumpos(it), line, len, dctx);
			else
			mData = fileDumper.ExpandLine(dumpos(it), line, len, dctx);
			return mData.c_str();*/
	return nullptr;
}

int SrcViewModel_t::checkEqual(adcui::DUMPOS it1, adcui::DUMPOS it2)
{
	if (!mPos[it1])//check why?
		return 0;
	int a(dumpos(it1));
	int b(dumpos(it2));
	if (a < b)
		return -1;
	if (a > b)
		return 1;
	return 0;
}

bool SrcViewModel_t::atEndIt(adcui::DUMPOS it)
{
	return !(dumpos(it) < linesNum());
}

//IADCTextModel

const char *SrcViewModel_t::cellDataIt(COLID col, adcui::DUMPOS it, bool bPlain)//, cell_info_t *)
{
	//ReadLocker lock(this);
	assert(0);
	/*
			if (col == CLMN_CODE)
			return dataIt(it, bPlain);
			if (atEndIt(it))
			return nullptr;

			MyDumpVisitor a;
			if (!visit(it, a, true))
			return nullptr;

			mData.clear();
			if (!a.op())
			{
			if (col == CLMN_LNUMS)
			if (a.offs().is_set())
			mData = Int2Str(a.offs().get(), I2S_HEX | I2S_SIGNED);
			return mData;
			}

			if (!a.cont())
			return nullptr;
			if (!a.cont()->type Proc())
			return nullptr;

			FieldPtr pFuncField(a.cont()->parentField());
			if (!FUNC DEF(pFuncField))
			return nullptr;

			FuncInfo_t FI(mrDcRef, *GlobFuncObj(pFuncField), FileDef());

			switch (col)
			{
			case CLMN_LNUMS:
			/ *int i;
			int n(a.op()->No(&i));
			char buf[10];
			if (i > 0)
			sprintf(buf, "%d%c", n, 'a'+i-1);
			else
			sprintf(buf, "%d", n);
			mData = buf;* /

			mData = FI.StrNo(a.op());
			return mData.c_str();
			case CLMN_STACKTOP:
			if (!FI.IsAnalized(a.op()) || a.op()->ins().opnd_BAD_STACKOUT)
			mData = "?";
			else
			mData = Int2Str(a.op()->StackIn(), I2S_HEX | I2S_SIGNED);
			return mData.c_str();
			case CLMN_FPUTOP:
			if (!FI.IsAnalized(a.op()))
			mData = "?";
			else
			{
			int8_t fputop = FI.FpuIn(a.op());
			if (a.op()->fstackDiff() < 0)//load
			fputop += a.op()->fstackDiff();
			mData = MyStringf("%d", (int)fputop);
			}
			return mData.c_str();
			case CLMN_PATHS:
			if (FI.PathOf(a.op()))
			mData = MyStringf("%d",  PathNo(FI.PathOf(a.op())));
			else
			mData = "";
			return mData.c_str();
			case CLMN_EXTRA:
			{
			ProblemInfo_t f(display()->problemWith(a.op()));
			mData = FI.findProblemStr(f, a.op(), false);
			return mData.c_str();
			}
			default:
			break;
			}*/
	return nullptr;
}

std::string decodeWString0(std::istream& is, uint16_t ulen)//length-prefixed
{
	size_t ulen2(ulen * sizeof(char16_t));

	std::u16string ws;
	for (uint16_t i(0); i < ulen; i++)
	{
		char16_t wc;
		is.read((char*)&wc, sizeof(wc));
		switch (wc)
		{
		case u'\n': ws += u"\\n"; break;
		case u'\r': ws += u"\\r"; break;
		case u'\t':	ws += u"\\t"; break;
		default:
			ws += wc;	break;
		}
	}

	const char16_t* sourceStart(ws.c_str());
	const char16_t* sourceEnd(sourceStart + ulen);

	std::string s;
	s.resize(ulen2);//a safety buf
	UTF8* targetStart0((UTF8*)&(*s.begin()));
	UTF8* targetStart(targetStart0);
	UTF8* targetEnd(targetStart + ulen2);
	ConvertUTF16toUTF8((const UTF16**)&sourceStart, (const UTF16*)sourceEnd, &targetStart, targetEnd, strictConversion);
	s.resize(targetStart - targetStart0);
	return std::move(s);
}

std::string decodeWString(std::istream& is)
{
	uint16_t ulen;
	is.read((char*)&ulen, sizeof(ulen));
	return std::move(decodeWString0(is, ulen));
}

void SrcViewModel_t::getRowDataIt(adcui::DUMPOS it, adcui::IADCTableRow &aTab)
{
	if (atEndIt(it))
		return;

	DumpContext_t dctx;
	const char *pc(getDumpPosAtIt(it, &dctx));
	if (!pc)
		return;

	FileInfo_t FIL(mrDcRef, FileDef());

	for (int i(0); i < CLMN_TOTAL; i++)
	{
		aTab.setCellWidth(i, columnWidth(i));
		aTab.setCellFlags(i, colFlags(i));
	}

	Display_t &disp(*display());

	if (columnWidth(CLMN_CODE) > 0)
	{
		ContextSafeEx_t safe(mrCore);

		/*I_ProbeSrc* pIProbe(nullptr);
		Probe_t* pCtx(mrCore.getContext());
		if (pCtx)
		{
			ContextSrc_t* pCtx(dynamic_cast<ContextSrc_t*>(pCtx));
			if (pCtx)
				pIProbe = &pCtx->probe();
			pCtx->Release();
		}*/

		FileDumper_t FD(FIL, disp, safe.get(), mrCore.ed());
		int len;
		std::stringstream ss(FD.ExpandLine(dumpos(it), pc, len, dctx));

		adcui::Color_t fgnd_color(adcui::COLOR_NULL);
		aTab.addColor(CLMN_CODE, fgnd_color);

		bool bQuit = false;
		//size_t i(0);
		while (!ss.eof() && !bQuit)
		{
			MyString s;
			adcui::Color_t iNextColor(adcui::COLOR_NULL);
			int fontID = -1;

			do {
				char c(ss.get());

				if (c == char(0xFF))
					break;//?

				if (c == '\n')
				{
					//i++;
					bQuit = true;
					break;
				}

				if (c == (char)adcui::SYM_WSTRING)
				{
					//i++;
					s.append(decodeWString(ss));// &(*s0.begin()) + i));
					break;
				}

				if (c == (char)adcui::SYM_COLOR)
				{
					iNextColor = (adcui::Color_t)ss.get();// s0[++i];
					//i++;
					break;
				}

				if (c == (char)adcui::SYM_FONT)
				{
					fontID = ss.get();// s0[++i];
					//i++;
					break;
				}

				if (c == (char)adcui::SYM_TAB)
				{
					int n(tab2spaces((int)s.length()));
					while (n--)
						s += ' ';
				}
				else
					s += c;

			} while (!ss.eof());

			aTab.addCell(CLMN_CODE, s);

			if (iNextColor != adcui::COLOR_NULL)
				aTab.addColor(CLMN_CODE, iNextColor);
		}

	}

	MyDumpVisitor a;
	if (!visit(pc, a, dctx.asFunc2()))
		return;

	if (a.field())
	{
		MyString sReason;
		if (FIL.CheckProblem(a.field(), sReason, IsHeader()))
		{
			if (!sReason.empty())
				aTab.addCell(CLMN_EXTRA, sReason);
			aTab.setMarginColor(adcui::COLOR_MARGIN_ERROR);
		}
	}

	if (!dctx.mpCplx)
		return;

	if (!a.op())
	{
		if (a.offs().is_set())
		{
			MyString s;
			ADDR off(a.offs().get());
			if (a.field() && FuncInfo_s::IsLocal(a.field()))
			{
				const STORAGE_t &stor(mrDcRef.SS(FuncInfo_s::SSIDx(a.field())));
				if (stor.isRegister())
				{
					//FieldPtr pFuncField(dctx.mpCplx->parentField());
					//assert(FUN CDEF(pFuncField));//must be a cfunc
					GlobPtr ifDef(FileInfo_t::ContextFuncDef(a.field()));//*GlobFuncObj(pFuncField)
					assert(ifDef);
					{
						FuncInfo_t FI(mrDcRef, *ifDef);// , FileDef());
						s = FI.RegName(a.field(), true);
					}
				}
			}
			if (s.empty())
			{
				if (a.isOffsSigned())
					s = Int2Str((int32_t)off, I2S_HEX | I2S_SIGNED);
				else
					s = Int2Str(off, I2S_HEX);
			}
			aTab.addCell(CLMN_LNUMS, s);//data
		}
		return;
	}

	CGlobPtr pFuncDef(dctx.asFunc2());
	if (!pFuncDef)
		return;

	FuncInfo_t FI(mrDcRef, *pFuncDef);// , FileDef());
	if (a.op())//?
	{
		HOP hOp(a.op());
		if (columnWidth(CLMN_LNUMS) > 0)
			aTab.addCell(CLMN_LNUMS, FI.LineInfoStr(hOp));

		bool bUnfold(disp.flags().testL(adcui::DUMP_UNFOLD) || FI.IsFuncUnfold());
		bool bIsCall(FI.IsCall(hOp));

		if (columnWidth(CLMN_STACKTOP) > 0)
		{
			if (bUnfold && bIsCall)
				aTab.addColor(CLMN_STACKTOP, adcui::COLOR_CALL_BREAK);
			aTab.addCell(CLMN_STACKTOP, FI.StackTopInfoStr(hOp, bUnfold));
		}

		if (columnWidth(CLMN_FPUTOP) > 0)
		{
			if (bUnfold && bIsCall)
				aTab.addColor(CLMN_FPUTOP, adcui::COLOR_CALL_BREAK);
			aTab.addCell(CLMN_FPUTOP, FI.FTopInfoStr(hOp, bUnfold));
		}

		if (columnWidth(CLMN_PATHS) > 0)
			aTab.addCell(CLMN_PATHS, FI.PathInfoStr(hOp));

		//if (columnWidth(CLMN_EXTRA) > 0)
		{
			ProblemInfo_t f(display()->problemWith(hOp));
			if (f != 0)
			{
				MyString s(FI.findProblemStr(f, hOp, false));
				if (!s.isEmpty())
					aTab.addCell(CLMN_EXTRA, s);
				aTab.setMarginColor(adcui::COLOR_MARGIN_ERROR);
			}
		}

		if (checkTaskTop(it))
			aTab.setLineColor(adcui::COLOR_TASK_TOP);

		if (bUnfold)
		{
			if (FI.IsCall(hOp))
			{
				aTab.setUnderLineColor(adcui::COLOR_CALL_BREAK);
			}
			else if (!FI.IsLastExx(hOp))
			{
				if (FI.IsCondJump(hOp))
					aTab.setUnderLineColor(adcui::COLOR_FLOW_SPLIT);
				else if (FI.IsGoto(hOp))
					aTab.setUnderLineColor(adcui::COLOR_FLOW_BREAK);
			}
		}
	}
}

void SrcViewModel_t::getColumnInfo(adcui::IADCTableRow &aRow, bool bNoData)
{
	for (int col(0); col < colsNum(); col++)
	{
		int w(columnWidth(col));
		if (w > 0)
		{
			if (!bNoData)
				aRow.addCell(col, colName(col));
			aRow.setCellFlags(col, colFlags(col));
			aRow.setCellWidth(col, w);
		}
	}
}


void SrcViewModel_t::setCurPosIt(adcui::DUMPOS it, int x)
{
	DisplayUI_t *pDisp(display());

	bool bHeader(pDisp->IsHeader());
	pDisp->setSyncOpLine(HOP());
	pDisp->exprCache().setSelOut(nullptr);

	ProbeExIn_t aProbe(x, dumpos(it), display()->flags().testL(adcui::DUMP_UNFOLD));

	FileInfo_t fileInfo(mrDcRef, FileDef());
	FileDumper_t fileDumper(fileInfo, *pDisp, &aProbe, nullptr);

	fileDumper.Probe(aProbe);

	fileInfo.SetupLocus(aProbe);
	aProbe.locus().wrapUp();//complete the locus
	if (aProbe.scope())
		aProbe.setModule(DcInfo_t::ModuleOfEx(aProbe.scope()));

	//if (ctx.dcRef() && ctx.fileDef())
	{
		//FileInfo_t dcInfo(*ctx.dcRef(), *ctx.fileDef());
		fileInfo.setSyncOp(aProbe);
	}

	mrCore.setLocus(new ProbeEx_t(aProbe))->Release();

#if(0)
	adc::CEventCommand *cmd(new adc::CEventCommand("setlocus", false));
	//		cmd->setContextZ(cx);
	//		cx->Release();
	mrCore.main().postEvent(cmd);
#endif
}

bool SrcViewModel_t::seekPosIt(const char *pc, adcui::DUMPOS it, adcui::DUMPOS)
{
	MyString s;
	if (pc)
		s = pc;

	if (s == "$sync")// a responce
	{
		//mcxRef.set(nullptr);
		ContextSafeEx_t safe(mrCore);
		if (safe)
		{
			GlobPtr ifDef(safe->scopeFunc());
			if (ifDef)
			{
				FuncInfo_t FI(mrDcRef, *ifDef);// , FileDef());//must have a func context
				//			if (!mbSyncMode)
				//			return false;
				//miCurLine = -1;
				OpPtr pOp(safe->opLine());
				//OpPtr pOp(pDC->syncOp());
				if (pOp)
				{
					if (seekOpRef(pOp, it))
					{
						display()->setSyncOpLine(pOp);
						return true;
					}
				}
			}
		}
	}
	else if (s == "$task")
	{
		Decompiler_t *pan(decompiler());
		if (!pan)
			return false;
		FuncInfo_t FI(mrDcRef, pan->FuncRef());// , pan->FileDef());//must have a func context
		OpPtr pOp(pan->currentOp());
		if (!pOp)
		{
			FieldPtr pField(pan->currentOpField());
			if (pField)
			{
				GlobPtr ifDef(FI.FuncDefAttached(pField));
				if (!ifDef)
					return false;
				return seekTypeRef((GlobToTypePtr)ifDef, it);
			}
		}
		return seekOpRef(pOp, it);
	}
	else if (s == "$dbg")
	{
		Debugger_t *pDbg(project().debugger());
		if (pDbg)
		{
			const ProbeEx_t &cx(dynamic_cast<ProbeEx_t &>(pDbg->context()));
			if (cx.opLine())
				return seekOpRef(cx.opLine(), it);
		}
	}
	//else if (s == "??")
	//display()->setSyncOpLine(pOp);//active view - just update syncOpLine

	display()->setSyncOpLine(HOP());
	return false;
}

/*bool SrcViewModel_t::setSyncMode(bool bOn)
{
	if (bOn == mbSyncMode)
		return false;
	mbSyncMode = bOn;
	if (mbSyncMode)
	{
		ContextSafeEx_t safe(mrCore);
		if (safe)
		{
			display()->setSyncOpLine(safe->op());
			return true;
		}
	}
	display()->setSyncOpLine(HOP());
	return true;
}*/

adcui::IADCTextEdit *SrcViewModel_t::startEditIt(adcui::DUMPOS, int /*x*/)
{
	DisplayUI_t *pDisp(display());
	if (!pDisp)
		return nullptr;

	Probe_t* pProbe(mrCore.getContext());
	assert(pProbe);

	if (!pProbe->obj())
		if (pProbe->entityId() != adcui::COLOR_ARRAY)
			return nullptr;

	MyString s;
	if (pProbe->obj())
	{
		DcInfo_t DI(mrDcRef);
		CFieldPtr pField(pProbe->field());
		unsigned n;
		if (pField)
		{
			if (pField->isTypeImp())
			{
				FieldPtr pExpField(DI.ToExportedField(pField));
				if (!pExpField)
					return nullptr;//error
				TypePtr iModule2(ProjectInfo_t::ModuleOf(DcInfo_s::GlobObj(pExpField)->folder()));
				DcInfo_t DI2(*DCREF(iModule2));
				n = ProjectInfo_t::ChopName(DI2.FieldDisplayNameEx(pExpField), s);
			}
			else
				n = ProjectInfo_t::ChopName(DI.FieldDisplayNameEx(pField), s);
		}
		else
		{
			MyString s2;
			TypePtr pType(pProbe->typeObj());
			CTypePtr pModule2(ProjectInfo_t::ModuleOf(pType));
			if (pModule2 != DI.ModulePtr())//no proxy type?
			{
				if (!DI.FindProxyOf(pType))//look for a proxy in current module?
				{
					DcInfo_t DI2(*DCREF(pModule2));
					s2 = DI2.TypePrettyName(pType);
				}
			}
			if (s2.isEmpty())
			{
				if (pType->typeProxy() && pType->nameless())
				{
					CTypePtr pIncumb(pType->typeProxy()->incumbent());
					pModule2 = ProjectInfo_t::ModuleOf(pIncumb);
					DcInfo_t DI2(*DCREF(pModule2));
					s2 = DI2.TypePrettyName(pIncumb);
				}
				else
					s2 = DI.TypeDisplayNameEx(pType);
			}
			n = ProjectInfo_t::ChopName(s2, s);//do not chop!
		}
		//if (n > 0)
			//s.append(NumberToString(n));
	}
	else if (pProbe->entityId() == adcui::COLOR_ARRAY)
		s = Int2Str(pProbe->value().ui64);

	pProbe->Release();

	return mrCore.startEd(s);
}

//ISrcViewModel

SyncObj_t SrcViewModel_t::getClosestObj(adcui::DUMPOS it0)
{
	assert(it0);

	adcui::AutoIter t1(this, it0);

	MyDumpVisitor a, b;

	int iDown(0);
	for (;; iDown++)
	{
		if (visit(posFromIter(t1), a, true))
			break;
		if (!forwardIt(t1))
			break;
	}

	adcui::AutoIter t2(this, it0);
	int iUp(0);
	for (;; iUp++)
	{
		if (visit(posFromIter(t2), b, true))
			break;
		if (!backwardIt(t2))
			break;
	}

	if (a.checkObj() && (!b.checkObj() || iDown >= iUp))
		return a;

	return b;
}

int SrcViewModel_t::setMode(adcui::UDispFlags flags, bool bSet, adcui::DUMPOS itSeek)
{
	using namespace adcui;
	UDispFlags oldFlags;
	DisplayUI_t *pDisp(display());
	if (pDisp)
		oldFlags = pDisp->flags();

	UDispFlags newFlags(oldFlags);
	if (bSet)
		newFlags.set(flags);
	else
		newFlags.clear(flags);

	if (oldFlags == newFlags)
		return 0;

	bool bNewDisplay(false);
	if (!pDisp)//? || (oldFlags.testL(DUMP_UNFOLD) != newFlags.testL(DUMP_UNFOLD)))
	{
		SyncObj_t aSeekObj;
		if (itSeek)
			aSeekObj = getClosestObj(itSeek);

//?		deleteDisplay();

		DisplayUI_t *pDisp(newDisplay(&mrFile, newFlags));

		//if switching to the collapsed view, we gotta get a root seek op
		if (aSeekObj && !pDisp->flags().testL(DUMP_UNFOLD))
		{
			if (aSeekObj.cont() && aSeekObj.cont()->typeProc())
			{
				CTypePtr pCont((CTypePtr)aSeekObj.cont());
				FieldPtr pFuncField(pCont->parentField());
				HOP pOp(aSeekObj.op());
				if (pOp && !pOp->isRoot())
				{
					FuncInfo_t FI(mrDcRef, *DcInfo_s::GlobFuncObj(pFuncField));// , FileDef());

					std::set<HOP> m;
					FI.GetTopOp(pDisp->isUnfoldMode(), pOp, m);
					if (!m.empty())//TODO: get a closest one?
						aSeekObj.setOp(*m.begin());
				}
			}
		}
		pDisp->setSeekObj(aSeekObj);

		bNewDisplay = true;
	}
	else
		display()->setFlags(newFlags);

	if (!bNewDisplay)
	{
		if ((oldFlags & DUMP_REDUMP_MASK) != (newFlags & DUMP_REDUMP_MASK))//toggled?
			InvalidateDump();
		return 1;
	}

	InvalidateDump();
	return 2;
}

class ChunkRedumper_t : public FileDumper_t
{
public:
	ChunkRedumper_t(const FileInfo_t& fi, Display_t& d, ProbeEx_t* pCtx, const MyLineEditBase* ped, unsigned indent)
		: FileDumper_t(fi, d, pCtx, ped)
	{
		m_indent = indent;
	}
	void operator()(CTypeBasePtr pType)
	{
		if (pType->typeFuncDef())
			DumpFuncDef((CGlobPtr)pType);
		else if (pType->typeStruc())
			DumpStrucDef((CTypePtr)pType);
		else
			ASSERT0;
	}
};

int SrcViewModel_t::Redump(adcui::DUMPOS itSeek, bool bResetIterators)
{
	DisplayUI_t *pDisplay(display());
	if (!pDisplay)
		return 0;

	if (!pDisplay->IsRedumpPending())
		return 0;

	//FileInfo_t dcInfo(mrDcRef, FileDef());
	FileInfo_t fileInfo(mrDcRef, FileDef());

#if(1)//TURN OFF PARTIAL REDUMP
	if (pDisplay->hasDirtyChunk())
	{
		pDisplay->ClearX(false);//expression cache only
		ChunkRedumper_t chunkDumper(fileInfo, *pDisplay, nullptr, nullptr, pDisplay->dirtyIndent());
		pDisplay->redumpDirtyChunk(chunkDumper);
	}
	else
#endif
	{
		pDisplay->ClearX(true);//all

		bool bCont(pDisplay->openScope(nullptr, 0));
		assert(bCont);
		FileDumper_t fileDumper(fileInfo, *pDisplay, nullptr, nullptr);
		assert(pDisplay->file() == FileDef().ownerFolder());
		if (!mpSubject)
			fileDumper.DumpFile();
		else
			fileDumper.DumpChosen(mpSubject);
#if(1)
		{
			static int z = 0;
			fprintf(stdout, "Redumped: all (%d)\n", ++z);
			fflush(stdout);
		}
#endif
		pDisplay->closeScope(true);
	}

	mPos.mIts.clear();//clear iterators
	if (bResetIterators)
	for (size_t i(0); i < mPos.size(); ++i)//reset positions
	{
		if (mPos[i])
			dumpos(adcui::DUMPOS(i)) = 0;
			//validateIt((adcui::DUMPOS)(short)i);
	}

	int iSeekLine(pDisplay->seekObjLine());
	if (iSeekLine >= 0)
	{
		seekLineIt(itSeek, iSeekLine);
		pDisplay->clearSeekLine();
	}
	return 1;
}

bool SrcViewModel_t::IsRedumpPending()
{
	DisplayUI_t *pDisplay(display());
	if (!pDisplay || pDisplay->IsRedumpPending())
		return true;
	return false;
}

int SrcViewModel_t::fileId() const
{
	return mrFile.ID();
}

bool SrcViewModel_t::getModuleName(MyStreamBase &ss) const
{
	MyStreamUtil ssu(ss);
	return ssu.WriteString(mrDcRef.moduleName()) != 0;
}

bool SrcViewModel_t::visit(const char *pc, MyDumpVisitor &a, GlobPtr iFuncDef) const
{
	if (iFuncDef)
	{
		if (ProtoInfo_t::IsStub(iFuncDef))
		{
			ProtoInfo_t FI(mrDcRef, iFuncDef);
			if (a.scan(pc) < 0)
				return false;
		}
		else
		{
			FuncInfo_t FI(mrDcRef, *iFuncDef);
			if (a.scan(pc) < 0)
				return false;
		}
	}
	else
	{
		if (a.scan(pc) < 0)
			return false;
	}
	return true;
}

bool SrcViewModel_t::visit(adcui::DUMPOS it, MyDumpVisitor &a, bool bCont) const
{
	DumpContext_t dctx;
	const char *pc = getDumpPosAtIt(it, bCont ? &dctx : nullptr);
	if (!pc)
		return false;
	//		validate It(it);
	if (bCont)
		a.setContainer(dctx.mpCplx);
	return visit(pc, a, dctx.asFunc2());
}

bool SrcViewModel_t::seekOpRef(OpPtr pOp, adcui::DUMPOS it)
{
	if (pOp)
	{
		DisplayUI_t* pDisp(display());
		int linesTotal(pDisp->linesTotal());
		for (int line(0); line < linesTotal; line++)
		{
//CHECK(line == 10)
//STOP
			MyDumpVisitor b;//(true);
			if (b.scan(pDisp->GetDataAtLine(line)) < 0)
				continue;
			if (pOp == b.op())
			{
				seekLineIt(it, line);
				return true;
			}
		}
	}
	return false;
}

bool SrcViewModel_t::seekFieldRef(CFieldPtr pField, adcui::DUMPOS it)
{
	if (pField)
	{
		DisplayUI_t *pDisp(display());
		//pDisp->CheckUpdated();
		int linesTotal(pDisp->linesTotal());
		for (int line(0); line < linesTotal; line++)
		{
//CHECK(line == 10)
//STOP
			MyDumpVisitor b;//(true);
			if (b.scan(pDisp->GetDataAtLine(line)) < 0)
				continue;
			if (pField == b.field())
			{
				seekLineIt(it, line);
				return true;
			}
		}
	}
	return false;
}

bool SrcViewModel_t::seekTypeRef(CTypePtr pType, adcui::DUMPOS it)
{
	if (pType)
	{
		DisplayUI_t* pDisp(display());
		//pDisp->CheckUpdated();
		int linesTotal(pDisp->linesTotal());
		for (int line(0); line < linesTotal; line++)
		{
//CHECK(line == 10)
//STOP
			MyDumpVisitor b;//(true);
			if (b.scan(pDisp->GetDataAtLine(line)) < 0)
				continue;
			if (pType == b.type())
			{
				seekLineIt(it, line);
				return true;
			}
		}
	}
	return false;
}

bool SrcViewModel_t::seekGlobRef(CGlobPtr pGlob, adcui::DUMPOS it)
{
	if (pGlob)
	{
		DisplayUI_t *pDisp(display());
		//pDisp->CheckUpdated();
		int linesTotal(pDisp->linesTotal());
		for (int line(0); line < linesTotal; line++)
		{
//CHECK(line == 10)
//STOP
			MyDumpVisitor b;//(true);
			if (b.scan(pDisp->GetDataAtLine(line)) < 0)
				continue;
			if (pGlob == b.glob())
			{
				seekLineIt(it, line);
				return true;
			}
		}
	}
	return false;
}

/*bool SrcViewModel_t::synchronizeIt(adcui::DUMPOS it, adcui::ISrcViewModel *pIOther, adcui::DUMPOS otherIt)
{
SrcViewModel_t *pOther(static_cast<SrcViewModel_t *>(pIOther));
if (!pOther)
{
display()->set SyncObj(nullptr);
return false;
}

MyDumpVisitor a;
if (!pOther->visit(otherIt, a))
return false;

Obj_t *pObj(a.obj());
if (!pObj)
return false;

pOther->display()->set SyncObj(pObj);

if (seekObj(pObj, it))
{
display()->set SyncObj(pObj);
return true;
}
/ *for (int line(0); line < (int)display()->dumps().size(); line++)
{
long pos(display()->dumps().pos(line));

MyDumpVisitor b;
if (b.expand(display()->GetStringAt(pos)) < 0)
continue;

Obj_t *pObjB(b.obj());

//if ((a.op() && a.op() == b.op()) || (a.field() && a.field() == b.field()))
if (pObj == pObjB)
{
display()->set SyncObj(pObjB);
seekLineIt(it, line);
return true;
}
}* /

display()->set SyncObj(nullptr);
return false;
}*/

/*ADDR SrcViewModel_t::getVA(adcui::DUMPOS it) const
{
	MyDumpVisitor a;
	if (!visit(it, a))
		return 0;

	const ObjFlags_t *pObj(a.obj());
	if (!pObj)
		return false;

	if (!Op_t::toOp(pObj))
		return false;

	return true;
}

bool SrcViewModel_t::getAddress(adcui::DUMPOS it, MyStreamBase &ss)
{
	ADDR va(getVA(it));
	if (!va)
		return false;
	return true;
}*/

void SrcViewModel_t::gotFocus(bool bGot)
{
	if (bGot)
	{
	}
}

bool SrcViewModel_t::checkTaskTop(adcui::DUMPOS it)
{
	Decompiler_t *pan(decompiler());
	if (!pan || !pan->currentOp())
		return false;
	FuncInfo_t FI(mrDcRef, pan->FuncRef());// , pan->FileDef());//must have a func context
	MyDumpVisitor a;
	if (!visit(it, a, true) || !a.op())
		return false;
	//FuncInfo_t FI(pan->dc(), *FUNC DEF(a.func()), FileDef());
	return (pan->currentOp() == a.op());
}

adcui::PixmapEnum SrcViewModel_t::pixmapIt(adcui::DUMPOS it)
{
	Debugger_t *pDbg(project().debugger());
	if (pDbg)
	{
		ADDR vaDbg(pDbg->current());
		MyDumpVisitor o;
		if (visit(it, o, true) && o.op() && o.cont() && o.cont()->typeProc())
		{
			CTypePtr pCont((CTypePtr)o.cont());
			CGlobPtr iGlob(DcInfo_s::GlobFuncObj(pCont->parentField()));
			FuncInfo_t FI(mrDcRef, *iGlob);// , FileDef());
			ADDR va(FI.OpVA(o.op()));
			if (va == vaDbg)
				return adcui::PIXMAP_DBG_NEXT;
			if (display()->checkAddress(o.op(), vaDbg))
				return adcui::PIXMAP_DBG_NEXT_HAZY;
		}
	}
	return adcui::PIXMAP_NULL;
}

static MyString checkQuotes(MyString s)
{
	if (s.find(' ') != std::string::npos)//contains spaces?
		s = "\"" + s + "\"";//wrap in quotes
	return s;
}

//1: found in current file (and jumped)
//0: failed to locate (abort)
//-1: found in different file/module/counterpart
bool SrcViewModel_t::initializeJumpTarget(SrcJumpData_t &jumpTarget, const SrcJumpData_t &jumpBack)
{
	DcInfo_t DI(mrDcRef);
	FieldPtr pField(jumpTarget.obj->objField());
	if (pField)
	{
		if (!DcInfo_t::IsGlobal(pField))
		{
			if (FuncInfo_s::IsLocal(pField))
			{
				GlobPtr pScope(FileInfo_t::GetLocalOwner(pField));
				jumpTarget.scope = pScope;
				jumpTarget.module = ModuleInfo_t::ModuleOf(DcInfo_s::DockField(pScope));
				jumpTarget.folder = pScope->folder();
				jumpTarget.isHeader = jumpBack.isHeader;
				{//RAII
					FuncInfo_t funcInfo(DI, *pScope);
					HOP hOp(FuncInfo_t::GetVarOp(pField));
					if (FuncInfo_t::IsArgOp(hOp))
					{
						jumpTarget.obj = pScope;//go to the header
					}
					else
					{
						assert(FuncInfo_t::IsVarOp(hOp));
						if (hOp->isInRow())
						{
							//get anchor op
							const PathOpList_t& l(FuncInfo_t::PathOf(hOp)->ops());
							PathOpList_t::Iterator i(l, hOp);
							do {
								hOp = (++i).data();
							} while (hOp->isInRow());
						}
						jumpTarget.op = hOp;
						assert(!jumpTarget.op->isInRow());
						jumpTarget.obj = nullptr;
					}
				}
				return true;
			}

			TypePtr pScope(pField->owner());
			jumpTarget.scope = pScope;//structure field - look for the owner
			jumpTarget.module = ModuleInfo_t::ModuleOf(pScope);
			jumpTarget.folder = USERFOLDER(pScope);
			jumpTarget.isHeader = true;
			return true;
		}

		jumpTarget.folder = DcInfo_s::IsGlobal(pField) ? DcInfo_s::GlobObj(pField)->folder() : nullptr;
		if (!jumpTarget.folder)
			return false;//not acquired

		jumpTarget.module = ModuleInfo_t::ModuleOf(pField);
		assert(jumpTarget.module == mrDcRef.module());

		if (pField->isTypeImp())
		{
			//if (ModuleInfo_t::ModuleOf(pField) == mrDcRef.module())
			//{
			if (!jumpTarget.isHeader//if at declaration, go directly to the exported field
				|| (jumpBack.pickId == adcui::LocusId_STRUC_HEADER))//????		adcui::CXTID_NULL && locality.decl))//this means - a declaration of a global object
			{//should a phantom exported cpp be open??
				FieldPtr pExpField(DI.ToExportedField(pField));
				if (!pExpField)
					return 0;
				jumpTarget.folder = DcInfo_s::GlobObj(pExpField)->folder();
				jumpTarget.module = ModuleInfo_t::ModuleOf(pField);
				pField = pExpField;
			}
			//}
			else
			{
				DcInfo_t DI2(*DCREF(ModuleInfo_t::ModuleOf(pField)));
				FieldPtr pExpField(DI2.ToExportedField(pField));
				if (!pExpField)// || exp.pFolder != &mrFile)
					return 0;
				pField = pExpField;
			}
		}

		jumpTarget.obj = pField;
		return true;//another file
	}

	TypePtr pType(jumpTarget.obj->objType());
	if (pType)
	{
		jumpTarget.module = ModuleInfo_t::ModuleOf(pType);
		if (jumpTarget.module != mrDcRef.module())//undeclared type
		{
			/*if (jumpBack.pickId != adcui::LocusId_STRUC_HEADER)
			{
				if (DI.FindProxyOf(pType))
				{
					STOP
				}
				//if (pProxy)
				{
					//pType = pProxy;//if struc's a header clicked, go directly to the incumbent, otherwise - to the proxy
					//jumpTarget.module = ModuleInfo_t::ModuleOf(pType);
				}
				//if the proxy doesn't exist, proceed with the incumbent
			}*/
		}
		else
		{
			if (pType->typeProxy())
			{
				if (jumpBack.pickId == adcui::LocusId_STRUC_HEADER)
				{
					pType = pType->typeProxy()->incumbent();
					jumpTarget.obj = pType;
					jumpTarget.module = ModuleInfo_t::ModuleOf(pType);
				}
			}
			else if (pType->typeEnum())
				pType = pType->baseType();
		}
		
		jumpTarget.folder = USERFOLDER(pType);
		if (!jumpTarget.folder)
			return false;

		jumpTarget.scope = pType;
		jumpTarget.pickId = adcui::LocusId_STRUC_HEADER;
		jumpTarget.isHeader = true;

		return true;
	}

	//globs?

	return false;//it wasn't changed
}

bool SrcViewModel_t::initiateJump(bool bToDefinition, int linesFromTop)//from locus
{
	ContextSafeEx_t safe(mrCore);
	if (!safe)
		return false;

	bool bInHeader(true);

	//jump source information used for possible jump backs
	SrcJumpData_t jumpBack;
	jumpBack.folder = folder();
	jumpBack.isHeader = IsHeader();
	jumpBack.module = mrDcRef.module();
	jumpBack.linesFromTop = linesFromTop;
	jumpBack.pickId = (adcui::LocusIdEnum)safe->locality().scoping;
	jumpBack.scope = safe->scope();
	jumpBack.op = safe->opLine();
	if (safe->pickedFieldDecl())//can be a func header
		jumpBack.obj = safe->pickedFieldDecl();
	else if (safe->locality().scoping == adcui::LocusId_STRUC_HEADER)
		jumpBack.obj = safe->scope();//at struc header

	SrcJumpData_t jumpTarget;
	jumpTarget.linesFromTop = linesFromTop;

	jumpTarget.obj = safe->obj();
	if (!jumpTarget.obj)
		return false;
#ifdef _DEBUG
	CTypePtr pType(jumpTarget.obj->objType());
	CFieldPtr pField(jumpTarget.obj->objField());
#endif
	jumpTarget.isHeader = !bToDefinition;

	if (!initializeJumpTarget(jumpTarget, jumpBack))
		return false;

//	if (jumpBack.folder == jumpTarget.folder && jumpBack.isHeader == jumpTarget.isHeader)//in the same file?
	//	jumpTarget.linesFromTop = -1;//just assure visible

	//first push a jump back
	mrCore.pushJumpBack(jumpBack);

	//if the target object declaration was not found in current source file. 
	//Ask GUI to re-issue the request, with special hint to use info at the top of jump stack.
	mrCore.pushJumpBack(jumpTarget);
	//mrCore.recoilJumpRequest();
	//MyString s(FilesMgr0_t::relPath(pFolder));
	//s.append(bInHeader ? HEADER_EXT : SOURCE_EXT);
	//main().postEvent(new adc::CEventCommand(MyStringf("show -@locus %s", checkQuotes(s).c_str())));
	mrCore.clearJumpForward();
	return true;
}

bool SrcViewModel_t::jump(const SrcJumpData_t& target, adcui::DUMPOS it)
{
	if (target.obj)
	{
		CGlobPtr pGlob(target.obj->objGlob());
		if (pGlob)
			return seekGlobRef(pGlob, it);

		CFieldPtr pField(target.obj->objField());
		if (pField)
			return seekFieldRef(pField, it);

		CTypePtr pType(target.obj->objType());
		if (pType)
			return seekTypeRef(pType, it);
	}
	else if (target.op)
	{
		return seekOpRef(target.op, it);
	}
	return false;
}

bool SrcViewModel_t::jump(adcui::DUMPOS it, bool bFwd, bool bFlip)
{
	SrcJumpData_t target;
	//whenever a jump (at jump stack's top) requested, the jump data expected to be invalidated (or flipped)
	if (bFwd)
	{
		if (!mrCore.popJumpForward(target))
			return false;
	}
	else
	{
		if (!mrCore.popJumpBack(target))
			return false;
	}
	if (!jump(target, it))
		return false;
	//if (bFlip)
	{
		if (bFwd)
			mrCore.pushJumpBack(target);
		else
			mrCore.pushJumpForward(target);
	}
	return true;
}

void SrcViewModel_t::printDumpInfo()
{
	display()->print(std::cout);
}

void SrcViewModel_t::setSubject(CGlobPtr pGlob)
{
	DcInfo_t DI(mrDcRef);
	mpSubject = (TypeBasePtr)pGlob;
	mSubjectName = DI.GlobNameFull(pGlob, DcInfo_t::E_PRETTY, CHOP_SYMB).join();
	InvalidateDump();
}

bool SrcViewModel_t::setSubject(const char *objName)
{
	MyString sVName(objName);//module!<name>
	if (sVName.empty())
	{
		if (mpSubject)
		{
			mpSubject = nullptr;
			mSubjectName.clear();
			InvalidateDump();
			return true;
		}
	}
	else
	{
		DcInfo_t DI(mrDcRef);
		MyString sObjName(DI.extractVName(sVName));
		if (!sObjName.empty())
		{
			ObjPtr pObj(DI.FindObjByScopedNameEx(sObjName, DI.PrimeSeg()));
			if (!pObj)
			{
				pObj = mrCore.objAtLocus(&mrDcRef);
				if (pObj)
				{
					mrCore.main().printWarning() << "Object '" << sObjName << "' could not be found in module '" << DI.ModuleName() << "'.\n\tThe object at locus was applied instead." << std::endl;
					fflush(STDERR);//this method is called from GUI dll with separated RTL
				}
			}
			if (pObj)
			{
				CGlobPtr pGlob(DI.CheckGlob(pObj));
				if (pGlob)
				{
					if (pGlob->folder() == &mrFile)
					{
						setSubject(pGlob);
						return true;
					}
				}
			}
#if(0)//makes sence to enable on user's input
			else
			{
				main().printError() << "Object '" << sObjName << "' could not be found in module '" << DI.ModuleName() << "'" << std::endl;
				fflush(STDERR);//!
			}
#endif
		}
	}
	return false;
}

Project_t &SrcViewModel_t::project() const
{
	return mrCore.main().project();
}

ProjectEx_t &SrcViewModel_t::projectx() const
{
	return reinterpret_cast<ProjectEx_t &>(project());
}

Decompiler_t *SrcViewModel_t::decompiler() const
{
	return dynamic_cast<Decompiler_t *>(project().analyzer());
}

Main_t &SrcViewModel_t::main() const
{
	return mrCore.main();
}






