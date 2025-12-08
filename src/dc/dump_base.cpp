#include "dump_base.h"
#include "shared/misc.h"
#include "dump_func.h"
#include "info_class.h"
#include "stubs.h"
#include "expr_term.h"

#define SCREEN_WIDTH_LIMIT	100

#define TEMPL	template <typename T>

////////////////////////////////////////////////////////
// ObjNameDumper_t

template <typename T_Dumper>
class ObjNameDumper_t : public Range_t
{
	T_Dumper& m_dumper;
	ProbeEx_t* mpCtx;
	const MyLineEditBase* mpEd;
	CObjPtr mpObj;
	adcui::Color_t m_eColor;
	adcui::Color_t m_eFont;
	adcui::Color_t m_eSquiggle;
	adcui::Color_t m_eSuffix;
	bool mbUnicode;
	bool mbQuotes;//if quotes required
	char mEos;//bad strings terminator
public:
	MyString sName;
	MyString sSuffix;
	bool bEditing;

public:
	ObjNameDumper_t(T_Dumper& dumper, CObjPtr pObj)
		: m_dumper(dumper),
		mpCtx(m_dumper.ctx()),
		mpEd(m_dumper.ed()),
		mpObj(pObj),
		m_eColor(adcui::COLOR_NULL),
		m_eFont(adcui::COLOR_NULL),
		m_eSquiggle(adcui::COLOR_NULL),
		m_eSuffix(adcui::COLOR_NULL),
		mbUnicode(false),
		bEditing(false),
		mbQuotes(false),
		mEos(0)
	{
		if (mpCtx)
		{
			bool bCurLine(m_dumper.disp().curLine() == mpCtx->line());
			if (bCurLine)//expr will handle this
				setBegin(m_dumper.currentPosInLine());
			bEditing = (mpEd && bCurLine && begin() == mpCtx->rangeObj().begin());
			if (bEditing)
			{
				sName = mpEd->editName();
				m_eColor = adcui::COLOR_CUR_EDIT;
			}
		}
	}
	void pickObj(CObjPtr p) { mpObj = p; }
	void setColor(adcui::Color_t e) { m_eColor = e; }
	void setFont(adcui::Color_t e) { m_eFont = e; }
	void setUnicode() { mbUnicode = true; }
	void setColorSquiggle(adcui::Color_t e) { m_eSquiggle = e; }
	void setColorSuffix(adcui::Color_t e) { m_eSuffix = e; }
	void setQuotes() { mbQuotes = true; }
	void setEos() {
		assert(!sName.empty());
		mEos = sName.back();
		assert(mEos != 0);
		sName.chop(1);
	}
	bool checkCur(CObjPtr)
	{
		CObjPtr pObjCur(m_dumper.curObj());
		if (!pObjCur)
			return false;
		if (checkObj(pObjCur) != checkObj(mpObj))//skip hidden types
			return false;
		bool bEditing(mpEd != nullptr);
		if (!bEditing)//no highlighting during editing
		if (!m_eColor)//not being edited?
		{
			m_eColor = adcui::COLOR_CUR;
			//sName.append(sSuffix);
			//sSuffix.clear();
		}
		return true;
	}
	static ObjPtr checkObj(CObjPtr pObj)
	{
		CTypePtr pType(pObj->objType());
		if (pType && pType->typeEnum())
			pObj = pType->baseType();
		return (ObjPtr)pObj;
	}
	~ObjNameDumper_t()
	{
		bool bForce(m_eColor == adcui::COLOR_CUR || m_eColor == adcui::COLOR_CUR_EDIT);
		if (m_eColor)
			m_dumper.PushColor(m_eColor, bForce);
		if (m_eFont)
			m_dumper.PushColor(m_eFont, true);//PushFont
		if (m_eSquiggle)
			m_dumper.PushColor(m_eSquiggle, true);
		if (mbQuotes)
		{
			if (mbUnicode)
				m_dumper.dumpChar('L');
			m_dumper.dumpChar('\"');
		}
		if (mbUnicode)
		{
			assert((sName.length() % 2) == 0);
			const char* ps(&(*sName.begin()));
			size_t ulen(sName.length() / sizeof(uint16_t));
			assert(ulen < 0x10000);
			m_dumper.dumpWStr((const wchar_t*)ps, uint16_t(ulen));
		}
		else
			m_dumper.dumpStr(sName);
		//if (mbUnicode)
		//	m_dumper.PopColor(true);
		if (!sSuffix.empty())
		{
			if (m_eSuffix == adcui::COLOR_NULL)
				m_eSuffix = adcui::COLOR_DUP_SUFFIX;

			if (m_dumper.PushColor(m_eSuffix, true) || 1)//no chop symb in src
			{
				m_dumper.dumpStr(sSuffix);
				m_dumper.PopColor(true);
			}
			else
			{
				m_dumper.dumpChar(CHOP_SYMB);
				m_dumper.dumpStr(sSuffix);
			}
		}
		if (mbQuotes)
			m_dumper.dumpChar('\"');
		if (m_eSquiggle)
			m_dumper.PopColor(true);
		if (m_eFont)
			m_dumper.PopColor(true);//PopFont
		if (m_eColor)
			m_dumper.PopColor(bForce);

		if (mEos != 0)//bad string terminator
		{
			m_dumper.PushColor(adcui::COLOR_SQUIGGLE_RED, true);
			m_dumper.dumpChar('\'');
			m_dumper.dumpChar(mEos);
			m_dumper.dumpChar('\'');
			m_dumper.PopColor(true);
		}

		if (begin() >= 0)
		{
			setEnd(m_dumper.currentPosInLine());
			ProbeExIn_t* pProbe(probe());
			if (pProbe && inside2(pProbe->x()))//including the upper bound
			{
				pProbe->pickRange(begin(), unsigned(size()));
				if (mpObj)
					pProbe->pickObj(mpObj);
			}
		}
	}
private:
	ProbeExIn_t* probe() const { return dynamic_cast<ProbeExIn_t*>(mpCtx); }
};



////////////////////////////////////////////////////////
// DumperBase_t

/*int DumperBase_t<T>::indent() const
{
	int i(0);
	for (const DumperBase_t *p(this); p->parentDumper(); p = p->parentDumper())
		i++;
	if (i > 0)
		i--;
	return i;
}*/

TEMPL void DumperBase_t<T>::dumpSpace(int n)
{
	while (n-- > 0)
		mos << ' ';
}

TEMPL void DumperBase_t<T>::dumpSep(bool bTab)
{
	if (bTab)
		dumpTab();
	else
		dumpSpace();
}

TEMPL void DumperBase_t<T>::dumpComma(bool bSpace)
{
	mos << ",";
	if (bSpace)
		dumpSpace();
}

TEMPL void DumperBase_t<T>::dumpStr(const char* pc, size_t n)
{
	if (n == 0)
		mos << (char*)pc;
	else
		mos.write(pc, n);
}

TEMPL void DumperBase_t<T>::dumpWStr(const wchar_t* ws, uint16_t ulen)
{
	m_disp.pushWString(ws, ulen);
	//char c = (char)adcui::SYM_WSTRING;
	//mos << c;
	//addExtraChars(1);

	/*mos.write((const char*)&ulen, sizeof(ulen));
	m_disp.addExtraChars(sizeof(ulen));
	mos.write((const char*)ws, ulen * sizeof(wchar_t));
	m_disp.addExtraChars(ulen);*/

}

TEMPL void DumperBase_t<T>::dumpChar(char ch)
{
	mos << ch;
}

TEMPL void DumperBase_t<T>::dumpStrUntilEol(const char* pc)
{
	const char* p = pc;
	while (*p) //get string length
	{
		if ((*p == 0xD) || (*p == 0xA))
			break;
		p++;
	}
	mos.write(pc, (int)(p - pc));
}

TEMPL void DumperBase_t<T>::OutputTabT(int n)
{
	while (!(--n < 0))
		mos << '\t';
}

TEMPL void DumperBase_t<T>::OutputWs(int n, const char* ch)
{
	while (!(--n < 0))
		mos << ch;
}

TEMPL void DumperBase_t<T>::dumpUnk()
{
	PushColor(adcui::COLOR_TAG_ERROR);
	mos << "?";
	PopColor();
}

TEMPL void DumperBase_t<T>::dumpSemi()
{
	mos << ";";
}

TEMPL void DumperBase_t<T>::dumpTab(int n)
{
	if (n < 0)
		n = 0;

	if (!m_disp.draftMode())
	{
		while (n--)
		{
			long pos(currentPosInLine(false));//why no shift?
			if (!m_disp.testOpt1(adcui::DUMP_TABS))
				OutputWs(tab2spaces(pos));
			else
				OutputTabT();
		}

		return;
	}

	OutputTabT(n);
	return;

	/*	if (n < 1)
	return;

	int pos = mstream.tellp();
	pos -= m_pos;

	int t = 0;
	while (n--)
	{
	do {
	t++;
	} while ((pos + t) & TABMASK);
	}

	for (int i = 0; i < t; i++)
	mstream << ' ';*/
}

TEMPL bool DumperBase_t<T>::PushColor(int colID, bool force)
{
	if (Display_t::IsNoColorMode())
		return false;
	if (!m_disp.testOpt1(adcui::DUMP_COLORS))
		return false;

	int comment(m_disp.commentLevel());//commnt level will prevent colors pushing in
	if (colID == adcui::COLOR_POP && comment > 0)//pop color
		comment--;

	if (force || /*(m_disp.commentLevel() == 0 ||*/ comment == 0)
	{
		m_disp.pushColor(colID);
	}

	/*	if (!force)
		{
			if (colID == adcui::COLOR_COMMENT || colID == adcui::COLOR_UNANALIZED || colID == adcui::COLOR_SEL || colID == adcui::COLOR_SELAUX || colID == adcui::COLOR_CUR)
			{
				comment++;
			}
			else if (m_disp.commentLevel())//all other
			{*/
	if (colID != adcui::COLOR_POP)
		comment++;
	/*		}
		}*/

	m_disp.setCommentLevel(comment);
	return true;
}

/*TEMPL void DumperBase_t<T>::PushFont(int fontID)
{
	if (!m_disp.testOpt1(adcui::DUMP_FONTS))
		return;

	char c = (char)adcui::SYM_FONT;
	char d = (char)fontID;
	this->mos << c << d;
	probeSrc()->adjustDelta(-2);
}*/

//int gLine = 0;

TEMPL void DumperBase_t<T>::NewLine(int indent)
{
	//	printf("%d\n", ++gLine);
	//CHECK(gLine == 28139)
	//STOP
	m_disp.newLine();
	if (indent != 0)
	{
		if (indent < 0)
			dumpTab(m_indent + indent);//start new line at indent less than current level
		else
			dumpTab(m_indent);
	}
}

TEMPL void DumperBase_t<T>::drawTypeName(CTypeBasePtr iType0, bool bUgly)//see TypeDisplayNameEx()
{
	assert(iType0);
CHECKID(iType0, 0x1017f)
STOP
	assert(!iType0->typeProxy());

	ObjNameDumper_t<DumperBase_t<T>> aRange(*this, iType0);

	if (aRange.bEditing)
	{
		aRange.checkCur(iType0);//may change the color
		//dump in destructor...
		return;
	}

	CTypePtr iType(iType0->objTypeGlob());
	//	if (iType->typeProxy())
		//	iType = ProjectInfo_t::SkipProxy(iType);

	const DcInfo_t& DI0(*this);

	if (iType->typeEnum())
		iType = iType->baseType();

	TypePtr iModule;
	CGlobPtr iGlob(iType->objGlob());
	if (iGlob)
	{
		iModule = DcInfo_t::ModuleOfEx(iGlob);
		if (!iModule)//is it non-sharing func ptr?
		{
			if (iType->typeFuncDef())
				iModule = DcInfo_s::ModuleOfEx(iType->objGlob());
		}
	}
	else
		iModule = DcInfo_t::ModuleOf(iType);

	FolderPtr pTypeFolder(nullptr);//in current module
	bool bImported(false);
	bool bCheckProxyName(false);
	MyString sIncumb;
	if (iModule)
	{
		DcInfo_t DI(*DCREF(iModule));

		MyString s;
		if (iModule != DI0.ModulePtr())//still may be in another module
		{
			bImported = true;
			if (!iType->isNested())//? || iType->owner()->typeNamespace())//nested types are named in context of their container
			{
				TypePtr iProxy(DI0.FindProxyOf(iType));//look for a proxy in current module
				if (iProxy)
				{
					aRange.pickObj(iProxy);
					pTypeFolder = USERFOLDER(iProxy);
					assert(pTypeFolder);//?
					if (!iType->isNested())
					{
						bCheckProxyName = true;
						//if (iProxy->name())
							s = DI0.TypePrettyName(iProxy);//imported top-level types are named in current module
						//else
							//s = DI.StrucNameless(iType);
						ProjectInfo_t::ChopName(DI.TypePrettyName(iType), sIncumb);
					}
				}
				else
					s = DI.StrucNameless(iType);//even if it is named
			}
			else
				pTypeFolder = DcInfo_t::OwnerFolder(iType);
		}
		else
			pTypeFolder = DcInfo_t::OwnerFolder(iType);

		if (s.isEmpty())
		{
			if (bUgly)
				s = DI.TypeName0(iType);
			else
				s = DI.TypePrettyName(iType);
		}

		unsigned n(ProjectInfo_t::ChopName(s, aRange.sName));//no chopping!
		if (n > 0)
			aRange.sSuffix = NumberToString(n);
	}
	else
		aRange.sName = "?";

	//CHECK(aRange.sName == "std~")
	//STOP
	if (!aRange.checkCur(iType0))//may change the color
	{
		if (iType->nameless())
			aRange.setColor(adcui::COLOR_UNNAMED);
	}

	if (bImported)
		aRange.setFont(adcui::COLOR_FONT_ITALIC);
	if (!pTypeFolder)
		aRange.setColorSquiggle(adcui::COLOR_SQUIGGLE_RED);//COLOR_ERROR;//not declared in current module
	else if (bCheckProxyName && sIncumb != aRange.sName)
		aRange.setColorSquiggle(adcui::COLOR_SQUIGGLE_GREEN);
	//dump in destructor...
}

TEMPL void DumperBase_t<T>::dumpTypeNameFull(CTypePtr iType)
{
	TypePtr iOwner(iType->ownerScope());
	if (iOwner)
	{
		dumpTypeNameFull(iOwner);
		mos << "::";
	}

	dumpTypeRef(iType);
}

TEMPL void DumperBase_t<T>::dumpTypeRef(CTypePtr iType)
{
	if (!m_disp.dumpSym(adcui::SYM_TYPEREF, iType))
		drawTypeName(iType, false);
}

TEMPL void DumperBase_t<T>::dumpFieldNameFull(CGlobPtr pGlob)
{
	assert(pGlob);
	TypePtr iScope(DcInfo_t::OwnerScope(pGlob));
	if (iScope)
	{
		dumpTypeNameFull(iScope);
		mos << "::";
	}
	dumpFieldRef(DcInfo_s::DockField(pGlob));
}

TEMPL void DumperBase_t<T>::dumpTypeNameScoped0(CTypePtr pSelf, CTypePtr pScopeFrom)
{
	if (pSelf != pScopeFrom)
	{
		TypePtr pOwner(DcInfo_t::OwnerScope(pSelf));
		if (pOwner && !pOwner->typeSeg() && pOwner != pScopeFrom)
		{
			dumpTypeNameScoped0(pOwner, pScopeFrom);
			dumpStr("::");
		}
	}
	dumpTypeRef(pSelf);
}

TEMPL void DumperBase_t<T>::dumpTypeNameScoped(CTypePtr pSelf, CTypePtr pScope)
{
//	if (!pScope || pSelf == pScope)
//		return dumpTypeRef(pSelf);
	dumpTypeNameScoped0(pSelf, ProjectInfo_t::CommonScope(pScope, pSelf));
}

TEMPL void DumperBase_t<T>::dumpFieldNameScoped(CFieldPtr pSelf, CTypePtr pScope)
{
	/*if (pScope)
	{
		TypePtr pSelfScope(DcInfo_t::OwnerScope(pSelf));
		TypePtr pScopeFrom(ProjectInfo_t::CommonScope(pScope, pSelfScope));
		dumpTypeNameScoped2(pSelfScope, pScopeFrom);
		mos << "::";
	}
	dumpFieldRef(pSelf);*/
	TypePtr pFieldScope;
	if (pSelf->isGlobal() && this->GlobObj(pSelf))
		pFieldScope = DcInfo_t::OwnerScope(this->GlobObj(pSelf));
	else
		pFieldScope = DcInfo_t::OwnerScope(pSelf);
	if (pFieldScope && pScope == pFieldScope)
		pFieldScope = nullptr;//reset
	if (pFieldScope)
	{
		dumpTypeNameScoped(pFieldScope, pScope);
		dumpStr("::");
	}
	dumpFieldRef(pSelf);
}

TEMPL void DumperBase_t<T>::drawFieldName(CFieldPtr pField, adcui::Color_t iColor)
{
	//assert(!IsGlobal(pField) || FuncInfo_t::IsL abel(pField));
	ObjNameDumper_t<DumperBase_t<T>> aRange(*this, pField);
	if (!aRange.bEditing)
	{
		MyString s;
		s = T::FieldDisplayNameEx(pField, nullptr);//autoname if no pretty name
		unsigned n(ProjectInfo_t::ChopName(s, aRange.sName));
		if (n > 0)
			aRange.sSuffix = NumberToString(n);
	}
	if (!aRange.checkCur(pField))
	{
		if (iColor != adcui::COLOR_NULL)
			aRange.setColor(iColor);
		else if (pField->nameless())
			aRange.setColor(adcui::COLOR_UNNAMED);

	}
	//dump in destructor!
}

TEMPL adcui::Color_t DumperBase_t<T>::GlobalColor2(CGlobPtr pGlob, CGlobPtr pImpGlob) const
{
	CFieldPtr pField(DcInfo_s::DockField(pGlob));
	//CFieldPtr pImpField();
	const FileDumper_t& FD(root());
	if (pImpGlob || FD.IsImporting())
		//if (FD.IsImporting())//IsTypeImp(pField))//never this flag
		return adcui::COLOR_IMPORT_REF;
	if (FD.IsExported(pField))
		return adcui::COLOR_EXPORTED;
	if (pField->isTypeExp())
		return adcui::COLOR_EXPORT_REF;
	if (pField->isTypeProc())
		return adcui::COLOR_USER_FUNCTION;
	return adcui::COLOR_NULL;
}

TEMPL adcui::Color_t DumperBase_t<T>::GlobalFont(CFieldPtr pField) const
{
	if (T::IsGlobal(pField))
	{
		CGlobPtr pGlob(DcInfo_t::GlobObj(pField));
		if (pGlob && pGlob->func() && !ProtoInfo_t::IsStub(pGlob))
			return adcui::COLOR_FONT_BOLD;
	}
	return adcui::COLOR_NULL;
}

TEMPL adcui::Color_t DumperBase_t<T>::GlobalColor(CFieldPtr pField, CFieldPtr pImpField) const
{
	if (T::IsGlobal(pField))
	{
		CGlobPtr pGlob(DcInfo_s::GlobObj(pField));
		if (!pGlob)
		{
			if (!FuncInfo_s::IsLabel(pField))
				return adcui::COLOR_SQUIGGLE_RED;
		}
		else
		{
			CGlobPtr pImpGlob(pImpField ? DcInfo_s::GlobObj(pImpField) : nullptr);
			return GlobalColor2(pGlob, pImpGlob);
		}
	}
	else if (FuncInfo_s::IsLocal(pField))
	{
		if (!FuncInfo_s::GetVarOp(pField))
			return adcui::COLOR_SQUIGGLE_RED;
	}
	return adcui::COLOR_NULL;
}

TEMPL void DumperBase_t<T>::drawFieldName(CFieldPtr pField, CFieldPtr pImpField, bool bUgly, adcui::Color_t eColor, adcui::Color_t eFont)
{
CHECKID(pField, 0x1128)
STOP
//CHECK(pField->name() && !strcmp(pField->name()->c_str(), "?SetContainerInfo@CDocTemplate@@QAEXI@Z"))
//STOP

	CFieldPtr pField2(pImpField ? pImpField : pField);

	ObjNameDumper_t<DumperBase_t<T>> aRange(*this, pField2);

	if (!aRange.bEditing)
	{
		MyString s;
		if (bUgly)
		{
			assert(T::IsGlobal(pField));
			//TypePtr ifDef(FUNC DEFREF(pField));
			//if (ifDef && !ifDef->typeFuncDef()->nameless())
			s = T::SymbolName(pField);//non-cacheable
			if (s.isEmpty())
				ProjectInfo_t::ChopName(T::FieldDisplayNameEx(pField, pImpField), s);//cut a suffix (if any) off
		}
		else
			s = T::FieldDisplayNameEx(pField, pImpField);//autoname if no pretty name
		unsigned n(ProjectInfo_t::ChopName(s, aRange.sName));
		if (n > 0)
			aRange.sSuffix = NumberToString(n);
	}

	aRange.setFont(eFont);

	if (!aRange.checkCur(pField2))
	{
		if (eColor != adcui::COLOR_NULL)
			aRange.setColor(eColor);
		else if (pField2->nameless())
			aRange.setColor(adcui::COLOR_UNNAMED);
	}

	if (eColor == adcui::COLOR_SQUIGGLE_RED)
		aRange.setColorSquiggle(eColor);

	//dump in destructor!
}

TEMPL void DumperBase_t<T>::dumpReserved(const char* str)
{
	PushColor(adcui::COLOR_KEYWORD);
	mos << str;
	PopColor();
}

TEMPL void DumperBase_t<T>::dumpPreprocessor(const char* str)
{
	PushColor(adcui::COLOR_PREPROCESSOR);
	mos << str;
	PopColor();
}

/*TEMPL void DumperBase_t<T>::dumpProtoAttribute(ProtoAttrEnum at)
{
	if (at == PROTO_ATTR_UNDEFINED)
		dumpColorTerm("__noproto", adcui::COLOR_TERM_LGREY);
	else if (at == PROTO_ATTR_STUB)
		dumpColorTerm("__stub", adcui::COLOR_TERM_LGREY);
	else if (at == PROTO_ATTR_PROCESSING)
		dumpColorTerm("__processing", adcui::COLOR_TAG_PROCESSING);
	else if (at == PROTO_ATTR_INCOMPLETE)
		dumpColorTerm("__incomplete", adcui::COLOR_TAG_ERROR);
	else if (at == PROTO_ATTR_INTRINSIC)
		dumpColorTerm("__intrinsic", adcui::COLOR_TERM_DMAGENTA);
	else
		ASSERT0;
}*/

TEMPL bool DumperBase_t<T>::dumpProtoAttribute(CTypePtr pSelf, FuncStatusEnum eStatus)
{
	//FuncStatusEnum eStatus(mrDumper.functionStatus(pSelf));
	if (eStatus == FUNCSTAT_OK)
		return false;

	dumpSpace();

	if (eStatus == FUNCSTAT_UNDEFINED)
		dumpColorTerm("__noproto", adcui::COLOR_TAG_NOPROTO);
	else if (eStatus == FUNCSTAT_STUB)
		dumpColorTerm("__stub", adcui::COLOR_TAG_STUB);
	else if (eStatus == FUNCSTAT_PROCESSING)
		dumpColorTerm("__processing", adcui::COLOR_TAG_PROCESSING);
	else if (eStatus == FUNCSTAT_INCOMPLETE)
		dumpColorTerm("__incomplete", adcui::COLOR_TAG_ERROR);
	else if (eStatus == FUNCSTAT_INTRINSIC)
		dumpColorTerm("__intrinsic", adcui::COLOR_TAG_INTRINSIC);
	else
		ASSERT0;

	/*CFieldPtr pField(pSelf->objGlob()->dockField());
	if (pField)
	{
		char buf[32];
		sprintf(buf, "<%08X>", pField->address());
		dumpStr(buf);
	}*/

	return true;
}

TEMPL void DumperBase_t<T>::drawImpClsGlobDecl(CGlobPtr pGlob, CGlobPtr pImpGlob, CTypePtr pScope)
{
	CFieldPtr pField(DcInfo_s::DockField(pGlob));
	CFieldPtr pImpField(pImpGlob ? DcInfo_s::DockField(pImpGlob) : nullptr);
CHECKID(pField, 0x1128)
STOP
	//assert(pField->isExported());//on exporting side!
	assert(!pField->isTypeImp());

	if (!pScope)
		pScope = DcInfo_t::OwnerScope(pGlob);

	if (pField->isExported())//some methods may not be imported
	{
		//get a local field
		if (!pImpField)
		{
			pImpField = root().FromExportedField(pField);
			if (pImpField)
				pImpGlob = this->GlobObj(pImpField);
		}

		if (!pImpField || !pImpField->isTypeImp())//a phantom binary?
		{
			//for those fields in other module having no imported entry in the current module
			TypePtr iModuleExp(DcInfo_s::ModuleOfEx(pGlob));
			const Dc_t& dc2(*DCREF(iModuleExp));
			FileInfo_t FI2(dc2, root().FileDef());
			FileDumper_t FD2(FI2, disp(), ctx(), ed(), true);
			assert(DcInfo_t::IsStaticMember(pGlob));
			root().PushColor(adcui::COLOR_UNANALIZED);//should be commented out
			mos << comment();
			FD2.drawGlobal(pGlob, nullptr, DcInfo_t::OwnerScope(pGlob));
			if (pImpField)
				dumpComment("cloned", false);
		}
		else
		{
			//if (exp.empty())//some methods of external class may not being used in current module 
			/*if (!pImpField->isTypeImp())
			{
				mos << "ERROR<" << pField->name()->c_str() << ">";
				return;
			}*/
			FileInfo_t FI(dc(), root().FileDef());
			FileDumper_t FD(FI, disp(), ctx(), ed(), true);
			if (pImpField->isTypeImp())
			{
				FD.drawGlobal(pGlob, pImpGlob, pScope);
			}
			else
			{
				FD.drawGlobal(pImpGlob, nullptr, nullptr);
			}
		}
	}
	else
	{
		TypePtr iScope(DcInfo_s::OwnerScope(pGlob));
		TypePtr iModuleExp(DcInfo_s::ModuleOfEx(pGlob));
		Dc_t& dc2(*DCREF(iModuleExp));
		FileInfo_t FI2(dc2, root().FileDef());
		FileDumper_t FD2(FI2, disp(), ctx(), ed(), true);
		root().PushColor(adcui::COLOR_UNANALIZED);//should be commented out
		FD2.drawGlobal(pGlob, nullptr, iScope);
		dumpComment("internal", false);
	}
}

TEMPL void DumperBase_t<T>::drawImpFieldRef(CFieldPtr pImpField, CFieldPtr pExpField)
{
	assert(pImpField->isTypeImp() || pImpField->isTypeExp());
	if (!m_disp.dumpSym(adcui::SYM_IMPFLDREF, pImpField))
	{
		if (!pExpField)
			pExpField = root().ToExportedField(pImpField);

		adcui::Color_t eColor(GlobalColor(pExpField, pImpField));
		adcui::Color_t eFont(GlobalFont(pExpField));
		drawFieldName(pExpField, pImpField, false, eColor, eFont);

//?		if (pImpField && IsProbing())
	//?		probeSrc()->pickObj(pImpField);
			//pickField(pImpField);
	}
}

TEMPL void DumperBase_t<T>::dumpFieldRef(CFieldPtr pField)
{
	if (!m_disp.dumpSym(adcui::SYM_FLDREF, pField))
	{
		adcui::Color_t eColor(GlobalColor(pField));
		adcui::Color_t eFont(GlobalFont(pField));
		drawFieldName(pField, nullptr, false, eColor, eFont);
	}
}

TEMPL void DumperBase_t<T>::dumpVTableDecl(CGlobPtr pGlob, int vptr_off, bool bImporting)
{
	if (!pGlob)
	{
		dumpPreprocessor("VTABLE_BEGIN");
		mos << "()";
		return;
	}
//CHECK(pGlob->dockAddress() == 0x206adccc)
//STOP
	if (bImporting)
	{
		if (!m_disp.dumpSym(adcui::SYM_IMPVTBLDECL, pGlob, vptr_off))
			drawImpVTableDeclaration(pGlob);
	}
	else
	{
		if (!m_disp.dumpSym(adcui::SYM_VTBLDECL, pGlob, vptr_off))
			drawVTableDeclaration(pGlob);
	}
}

TEMPL void DumperBase_t<T>::drawVTableDeclaration(CGlobPtr pGlob)
{
	dumpPreprocessor("VTABLE_BEGIN");
	mos << "(";
	CFieldPtr pField(DcInfo_s::DockField(pGlob));
	drawFieldName(pField, nullptr, false, GlobalColor2(pGlob));
	pickField(pField);
	mos << ")";
}

TEMPL void DumperBase_t<T>::drawImpVTableDeclaration(CGlobPtr pGlob)
{
	CFieldPtr pField(DcInfo_s::DockField(pGlob));
	CFieldPtr pImpField(root().FromExportedField(pField));
	//CGlobPtr pImpGlob(pImpField ? GlobObj(pImpField) : nullptr);
	if (!pImpField)
	{
		//for those fields in other module having no imported entry in the current module
		root().PushColor(adcui::COLOR_UNANALIZED);//should be commented out
		mos << comment();
	}
	else if (!pImpField->isTypeImp())
		pImpField = nullptr;
	dumpPreprocessor("VTABLE_BEGIN");
	mos << "(";
	if (pImpField)
		drawImpFieldRef(pImpField, pField);
	else
		drawFieldName(pField, nullptr, false, GlobalColor2(pGlob));
	mos << ")";
}

TEMPL void DumperBase_t<T>::dumpConstRef(CFieldPtr pField)
{
	if (!m_disp.dumpSym(adcui::SYM_CONSTREF, pField))
	{
		drawConstRef(pField);
	}
}

TEMPL void DumperBase_t<T>::dumpFieldDecl(CFieldPtr pField, CFieldPtr pImpField, CTypePtr pScope)
{
CHECKID(pField, 0x30dd)
STOP
	assert(pField);
	if (!m_disp.dumpSym(adcui::SYM_FLDDECL, pField))
		drawFieldDefinition(pField, pImpField, pScope, false);
}

TEMPL void DumperBase_t<T>::drawFieldDef(CFieldPtr pField, CFieldPtr pImpField, CTypePtr pScope)
{
CHECKID(pField, 0x6d3)
STOP
	if (!m_disp.dumpSym(adcui::SYM_FLDDEF, pField))
	{
		drawFieldDefinition(pField, pImpField, pScope, true);
	}
}

TEMPL bool DumperBase_t<T>::openScope(CTypeBasePtr pSelf)
{
	bool bCont(m_disp.openScope(pSelf, indent()));
#ifdef _DEBUG
	FullName_t s;
	GlobPtr pGlob(pSelf->objGlob());
	if (pGlob)
	{
		s = this->GlobNameFull(pGlob, DcInfo_t::E_PRETTY, CHOP_SYMB);
	}
	else
	{
		CTypePtr pType(pSelf->objTypeGlob());
		assert(pType);
		s = this->TypePrettyNameFull(pType, CHOP_SYMB);
	}
	m_disp.setScopeName(s.join());
#endif
	return bCont;
}

TEMPL void DumperBase_t<T>::Type_Dump(CTypePtr pSelf)
{
	CTypePtr pSelf0(ProjectInfo_t::SkipProxy(pSelf));
	if (pSelf0->typeStruc())
	{
		bool bCont(!DcInfo_t::IsEmptyStruc(pSelf0) && openScope(pSelf0));
		DumpStrucDef(pSelf);
		if (bCont)
			m_disp.closeScope(false);
		NewLine();
	}
}

TEMPL void DumperBase_t<T>::output_optype(const char* pstr, bool bColor)
{
	//int n;
	//const char *pstr = OpTyp2Str(typ, nMode, &n);
	//bool bColor = (n == 1);//std

	if (bColor)//standard types
	{
		PushColor(adcui::COLOR_KEYWORD);
		mos << pstr;
		PopColor();
	}
	else
	{
		PushColor(adcui::COLOR_PREPROCESSOR);
		mos << pstr;
		PopColor();
	}
}

TEMPL void DumperBase_t<T>::output_optype(uint8_t typ, int nMode)
{
	int n;
	const char* pstr = OpTyp2Str(typ, nMode, &n);
	bool bColor = (n == 1);//std
	output_optype(pstr, bColor);
}


/*FileDumper_t& DumperBase_t<T>::root() const
{
	const DumperBase_t* p(this);
	while (p->parentDumper())
		p = p->parentDumper();
	return *(FileDumper_t*)p;
}*/

/*TEMPL void DumperBase_t<T>::OutputTYPE0(const TYP_t &rSelf, int flags)
{
	CExpr Name o(*this);
	MyString s(rSelf.name(o));
	int key(0);
	for (size_t n(0); n < s.length(); n++)
	{
		char ch(s[n]);
		if (ch == '%')//placeholder
		{
			const CExpr Name::elt &a(o.mv[key]);
			if (a.pObj)
			{
				dumpObj Name(a.pObj);
			}
			else if (a.color > adcui::COLOR_NULL)
			{
				PushColor(a.color);
				mos << a;
				PopColor();
			}
			else
				mos << a;
			key++;
		}
		else
			mos << ch;
	}
}*/

static char escape_chars[] = "\a\b\f\n\r\t\v";//\'\"\?\\";
static char escape_symbs[] = { 'a', 'b', 'f', 'n', 'r', 't', 'v' };//,'\'','"','?','\\'};

TEMPL void DumperBase_t<T>::dumpString(CTypePtr pType, DataStream_t& aRaw)
{
	MyString s;
	char eos(0);
	if (!fetchString(pType, aRaw, s))
	{
		eos = s.back();
		assert(eos != 0 && !s.empty());
		s.chop(1);
	}
	PushColor(adcui::COLOR_STRING);
	mos << '\"' << s << '\"';
	PopColor();
	if (eos != 0)//bad string terminator
	{
		PushColor(adcui::COLOR_SQUIGGLE_RED, true);// COLOR_TAG_ERROR);
		mos << "\'" << eos << "\'";//incomplete
		PopColor(true);
	}
}

TEMPL bool DumperBase_t<T>::fetchString(CTypePtr pType, DataStream_t& aRaw, MyString &s)//false if incomplete
{
	int n(pType->size());
	assert(n > 0);
	const Array_t* pArray(pType->typeArray());
	uint8_t optyp(pArray->baseType()->typeSimple()->optype());
	bool bUnicode(optyp == OPTYP_CHAR16);
	assert(optyp == OPTYP_CHAR8 || bUnicode);
	int sz(optyp & OPSZ_MASK);

	value_t v;
#define STR_LEN_MAX 256u//256: max?
	for (size_t i(0); i < std::min(STR_LEN_MAX, pArray->total()); i++)
	{
		//if (!aRaw.dataAt(oPos++, 1, (PDATA)&ch))
		if (aRaw.read(sz, (PDATA)&v) != sz)
			ASSERT0;
		if (v.ui32 == 0)//hi bytes shuold be clean
			break;
		if (bUnicode)
		{
			s.std::string::append((const char *)&v.ui16, sizeof(char16_t));//use regular string to contain wide chars
		}
		else
		{
			if (v.ui32 < adcui::__SYM__TAGS)//0x80
			{
				char* pc = strchr(escape_chars, v.i32);
				if (pc && *pc)
				{
					s += "\\";
					v.i32 = escape_symbs[pc - escape_chars];
					i++;
				}
				s += v.i8;
			}
			else
			{
				char buf[8];
				sprintf(buf, "\\x%x", v.i8);
				s.append(buf);
				i += strlen(buf) - 1;
			}
		}
	}
#undef STR_LEN_MAX
	if (v.ui32 != 0)//was eos reached?
		return false;
	return true;
}

TEMPL void DumperBase_t<T>::drawConstRef(CFieldPtr pField)
{
	ObjNameDumper_t<DumperBase_t<T>> aRange(*this, pField);

	if (!aRange.bEditing)
	{
		OFF_t oPos;
		if (!T::GetRawOffset(pField, oPos))
			ASSERT0;
		TypePtr pType(pField->type());
		assert(pType);
		Array_t* pArrayPvt(pType->typeArray());
		if (pArrayPvt)
		{
			const Simple_t* pSimple(pArrayPvt->baseType()->typeSimple());
			assert(pSimple);
			bool bUnicode(pSimple->optype() == OPTYP_CHAR16);
			assert(bUnicode || pSimple->optype() == OPTYP_CHAR8);
			if (bUnicode)
				aRange.setUnicode();

			aRange.setQuotes();

			DataStream_t aRaw(T::GetDataSource()->pvt(), oPos);
			if (!fetchString(pType, aRaw, aRange.sName))
				aRange.setEos();

			//if (!aRange.checkCur(pField))
				aRange.setColor(adcui::COLOR_STRING);
		}
		else
		{
			assert(pType->typeSimple());
			value_t v;
			T::GetDataSource()->pvt().dataAt(oPos, pField->size(), (PDATA)&v);
			aRange.sName = VALUE_t(pType->typeSimple()->optype(), v).toString();
			aRange.checkCur(pField);
		}
	}

	//dump in destructor...
}

TEMPL void DumperBase_t<T>::dumpSimpleInst(CTypePtr pType, DataStream_t& aRaw, AttrIdEnum attr)
{
	Simple_t* pSimp(pType->typeSimple());
	assert(pSimp);
	VALUE_t v;
	aRaw.read(pSimp->opsize(), (PDATA)&v);
	v.typ = pSimp->optype();
	mos << v.toString();

	/*?	if (pField->m_nFlags & FLD_INVERTED)
	{
	mos << "1 / ";
	v.invert();
	}
	mos << v.toString();*/
}

TEMPL void DumperBase_t<T>::dumpColorTerm(const char* s, adcui::Color_t eColor, bool bForce)
{
	PushColor(eColor, bForce);
	mos << s;
	PopColor(bForce);
}

static bool isTypeOkToDump(TypePtr pSelf)
{
	if (!pSelf)
		return false;
#if(1)
	if (pSelf->typeComplex() && !pSelf->hasUserData())//not in file?
		//if (pSelf->typeCode() || pSelf->typeStrucvar())
		if (!pSelf->typeFuncDef())
			//if (pSelf->isShared())
			if (!pSelf->isVTable())
				return false;
#endif
	return true;
}

TEMPL typename DumperBase_t<T>::LASTSYM_enum
DumperBase_t<T>::dumpRawData(CTypePtr pType0, DataStream_t& aRaw, CTypePtr iContext, AttrIdEnum attr, CTypePtr pSeg, int level)
{
	bool bBraced(false);
	TypePtr pType(ProjectInfo_t::SkipModifier(pType0));
	if (!isTypeOkToDump(pType))
	{
		//if (!aRaw.isAtEnd())
		{
			VALUE_t v;
			aRaw.read(OPSZ_BYTE, (PDATA)&v);
			v.typ = OPSZ_BYTE;
			mos << v.toString();
		}
		//else
			//dumpUnk();
		return LASTSYM_VAL;
	}

	if (pType)
	{
		bool bDataFault(false);
		LASTSYM_enum eLastSym;

		if (pType->typeArray())
		{
			mos << "{";
			IncreaseIndent();
			if (level == 0)
				NewLine();
			try
			{
				eLastSym = dumpRawData0(pType, aRaw, iContext, attr, pSeg, level + 1);
			}
			catch (const DataAccessFault_t & e)
			{
				if (e.code() != 0)
					dumpColorTerm("...", adcui::COLOR_TAG_ERROR);
				bDataFault = true;
			}
			if (level == 0)
			{
				DecreaseIndent();
				NewLine();
				mos << "}";
			}
			else
			{
				if (eLastSym == LASTSYM_PUN)
				{
					DecreaseIndent();
					NewLine();
					mos << "}";
				}
				else
				{
					mos << "}";
					DecreaseIndent();
				}
			}
			if (bDataFault)
				throw DataAccessFault_t();
			return LASTSYM_PUN;
		}

		if (pType->typeStruc())
		{
			mos << "{";
			if (!pType->typeStruc()->hasFields() && level == 0)
			{
				dumpColorTerm("...", adcui::COLOR_TAG_ERROR);
				mos << "}";
				return LASTSYM_PUN;
			}
			IncreaseIndent();
			if (level == 0)
				NewLine();
			try
			{
				eLastSym = dumpRawData0(pType, aRaw, iContext, attr, pSeg, level + 1);
			}
			catch (const DataAccessFault_t & e)
			{
				if (e.code() != 0)
					dumpColorTerm("...", adcui::COLOR_TAG_ERROR);
				bDataFault = true;
			}
			if (level == 0)
			{
				DecreaseIndent();
				NewLine();
				mos << "}";
			}
			else
			{
				if (eLastSym == LASTSYM_PUN)
				{
					DecreaseIndent();
					NewLine();
					mos << "}";
				}
				else
				{
					mos << "}";
					DecreaseIndent();
				}
			}
			if (bDataFault)
				throw DataAccessFault_t();
			return LASTSYM_PUN;
		}
	}

	return dumpRawData0(pType, aRaw, iContext, attr, pSeg, level);
}

TEMPL typename DumperBase_t<T>::LASTSYM_enum
DumperBase_t<T>::dumpRawData0(CTypePtr pType, DataStream_t& aRaw, CTypePtr iContext, AttrIdEnum attr, CTypePtr pSeg, int level)
{
	assert(pType);
	//TypePtr pType(ProjectInfo_t::SkipModifier(pType0));
	/*if (!isTypeOkToDump(pType))
	{
		if (!aRaw.isAtEnd())
		{
			VALUE_t v;
			aRaw.read(OPSZ_BYTE, (PDATA)&v);
			v.typ = OPSZ_BYTE;
			mos << v.toString();
		}
		else
			dumpUnk();
		return LASTSYM_VAL;
	}*/

	if (pType->typeArray())
	{
		LASTSYM_enum eLastSym(LASTSYM_NULL);
		Array_t* pArray(pType->typeArray());
		size_t nArray0 = pArray->total();
		size_t nArray(std::min(nArray0, (size_t)100));
		TypePtr pType2(pArray->baseType());
		for (size_t n(0); n < nArray; n++)
		{
			if (n > 0)
				mos << ", ";
			if (pType2->typePtr() || pType2->typeStruc())
				NewLine();
			eLastSym = dumpRawData(pType2, aRaw, iContext, attr, pSeg, level);
		}
		return eLastSym;
	}

	if (pType->typePtr())
	{
		//if (!aRaw.isAtEnd())
		{
			VALUE_t v;
			v.typ = (uint8_t)aRaw.read(pType->size(), (PDATA)&v);
			CFieldPtr pField(ModuleInfo_t::GetFieldFromValue(v, pSeg));
			if (pField)
			{
				dumpStr("&");
				dumpFieldNameScoped(pField, iContext);
			}
			else if ((v.typ == OPSZ_DWORD && v.ui32 == 0) || (v.typ == OPSZ_QWORD && v.ui64 == 0))
				dumpPreprocessor("NULL");
			else
				mos << v.toString();
		}
		//else
			//dumpUnk();
		return LASTSYM_VAL;
	}

	if (pType->typeSimple())
	{
		if (m_disp.currentPosInLine() > SCREEN_WIDTH_LIMIT)
			NewLine();
		if (aRaw.isAtEnd())
		{
			STOP
		}
		//if (!aRaw.isAtEnd())
		dumpSimpleInst(pType, aRaw, attr);
		//else
		//	dumpUnk();
		return LASTSYM_VAL;
	}

	if (pType->typeStruc())
	{
		return dumpRawDataStruc(pType, aRaw, iContext, attr, pSeg, level);
	}

	assert(0);

	/*if (pArray)
	{
		if (nArray < nArray0)
		{
			PushColor(adcui::COLOR_STRING);
			mos << "...";//incomplete
			PopColor();
		}
		mos << "}";
	}*/
	return LASTSYM_NULL;
}

TEMPL typename DumperBase_t<T>::LASTSYM_enum
DumperBase_t<T>::dumpRawDataStruc(CTypePtr pType, DataStream_t& aRaw, CTypePtr iContext, AttrIdEnum attr, CTypePtr pSeg, int level)
{
//CHECKID(pType, 0x103a6)
//STOP
	LASTSYM_enum eLastSym(LASTSYM_NULL);
	const FieldMap& m(pType->typeStruc()->fields());

	ADDR uOff(0);
	FieldMapCIt iBeg(m.begin());
	FieldMapCIt iEnd(m.end());
	int iCount(0);
	for (FieldMapCIt i(iBeg); i != iEnd; ++i, iCount++)
	{
		CFieldPtr pField(VALUE(i));
		TypePtr pType2(pField->type());

		if (pField->offset() > uOff)
		{
			unsigned uGap(unsigned(pField->offset() - uOff));
			for (int l(0); uGap > 0 && l < 10; ++l, --uGap, uOff += OPSZ_BYTE)//set a limit
			{
				VALUE_t v;
				if (aRaw.read(OPSZ_BYTE, (PDATA)&v) != OPSZ_BYTE)
					break;
				v.typ = OPSZ_BYTE;
				if (iCount > 0)
					mos << ", ";
				//if (l == 0)
				//NewLine();
				mos << v.toString();
			}
			if (uGap > 0)
			{
				mos << "...";
				aRaw.skip(uGap);
				uOff += uGap;
				uGap = 0;
			}
			iCount++;
			eLastSym = LASTSYM_VAL;
		}

		if (this->IsEosField(pField))
			break;

		if (iCount > 0)
			mos << ", ";

		if (pType2 && pType2->typePtr())
		{
			if (iCount > 0)
				NewLine();
			eLastSym = dumpRawData(pType2, aRaw, iContext, ATTR_NULL, pSeg, level);
		}
		else if (pType2 && pType2->typeBitset())
		{
			//NewLine();
			mos << "{bitset}";
			aRaw.skip(pType2->sizeBytes());
			/*FieldMap& m2(pField->type()->typeBitset()->fields());
			for (FieldMapCIt i(m2.begin()); i != m2.end(); i++)
			{
			CFieldPtr pField2(VALUE(i));
			NewLine();
			dumpSemi();
			}*/
		}
		else
		{
			if (pType2 && (pType2->typeStruc() || pType2->typeArray()))
				NewLine();
			eLastSym = dumpRawData(pType2, aRaw, iContext, pField->attrib(), pSeg, level);
		}

		uOff += (pType2 ? pType2->size() : OPSZ_BYTE);
	}
	return eLastSym;
}

TEMPL void DumperBase_t<T>::DumpVirtualEntry(CGlobPtr pGlob, int vmeth_off, bool bImporting)
{
	if (bImporting)
	{
		if (!m_disp.dumpSym(adcui::SYM_IMPCLSGLB, pGlob))
			drawImpClsGlobDecl(pGlob, nullptr, nullptr);
	}
	else
	{
		assert(pGlob && pGlob->func());
		DumpVirtualMethodDeclaration(pGlob, vmeth_off, FUNCDUMP_DECL_FLAGS, nullptr, DcInfo_s::OwnerScope(pGlob));
	}
}

TEMPL void DumperBase_t<T>::dumpRawDataVTable(const ClassVTable_t &vtable, DataStream_t& aRaw, CTypePtr pSeg, bool bImporting)
{
//CHECKID(pType, 0x103a6)
//STOP

	const ClassVirtMembers_t& vmemb(vtable.entries);
	ClassVirtMembers_t::const_iterator itMethod(vmemb.begin());
	int methodOffset(itMethod != vmemb.end() ? itMethod->first : -1);

	CTypePtr pType(DcInfo_s::DockField(vtable.self)->type());
	const FieldMap& m(pType->typeStruc()->fields());

	int lastUpper(0);
	FieldMapCIt iBeg(m.begin());
	FieldMapCIt iEnd(m.end());
	int iCount(0);
	bool bAfterMethod(false);
	for (FieldMapCIt i(iBeg); i != iEnd; ++i, iCount++)
	{
		CFieldPtr pField(VALUE(i));
		TypePtr pType2(pField->type());

		int newLower(int(pField->offset()));
		if (newLower > lastUpper)
		{
			unsigned uGap(unsigned(pField->offset() - lastUpper));
			for (int l(0); uGap > 0 && l < 10; ++l, --uGap, lastUpper += OPSZ_BYTE)//set a limit
			{
				VALUE_t v;
				if (aRaw.read(OPSZ_BYTE, (PDATA)&v) != OPSZ_BYTE)
					break;
				v.typ = OPSZ_BYTE;
				if (iCount > 0)
					mos << ", ";
				//if (l == 0)
				//NewLine();
				mos << v.toString();
			}
			if (uGap > 0)
			{
				mos << "...";
				aRaw.skip(uGap);
				lastUpper += uGap;
				uGap = 0;
			}
			iCount++;
		}

		if (this->IsEosField(pField))
			break;

		if (iCount > 0)
		{
			if (bAfterMethod)
			{
				dumpSemi();
				bAfterMethod = false;
			}
			else
				mos << ", ";
		}

		if (pType2 && pType2->typePtr())
		{
//			if (iCount > 0)
				NewLine();

			CGlobPtr pMethod(nullptr);
			if (methodOffset >= 0)
			{
				if (newLower == methodOffset)
				{
					pMethod = itMethod->second;
					++itMethod;
					methodOffset = itMethod != vmemb.end() ? itMethod->first : -1;
				}
				else if (newLower > methodOffset)//stepped over?
				{
					methodOffset = -1;//stop looking for a method match
				}
			}
			if (pMethod)
			{
				DumpVirtualEntry(pMethod, methodOffset, bImporting);
				aRaw.skip(pType2->size());
				bAfterMethod = true;
			}
			else
			{
				dumpComment(nullptr, true);
				dumpRawData(pType2, aRaw, nullptr, ATTR_NULL, pSeg, 0);
			}
		}
		else if (pType2 && pType2->typeBitset())
		{
			mos << "{bitset}";
			aRaw.skip(pType2->sizeBytes());//later
		}
		else
		{
			if (pType2 && (pType2->typeStruc() || pType2->typeArray()))
				NewLine();
			dumpRawData(pType2, aRaw, nullptr, pField->attrib(), pSeg, 0);
		}

		lastUpper += (pType2 ? pType2->size() : OPSZ_BYTE);
	}
	//dump unresolved method entries
	for (; itMethod != vmemb.end(); ++itMethod)
	{
		NewLine();
		DumpVirtualEntry(itMethod->second, itMethod->first, bImporting);
	}
}

TEMPL const ProbeExIn_t* DumperBase_t<T>::IsProbing() const
{
	//assert(!mpProbeSrc || mpProbeSrc->isProbing());
	return dynamic_cast<const ProbeExIn_t*>(mpCtx);// mpProbeSrc&& mpProbeSrc->isProbing();
}

TEMPL void DumperBase_t<T>::DumpFieldGap(CFieldPtr pField)
{
	assert(pField->objField());
	assert(!this->Storage(FuncInfo_s::SSIDx(pField)).isDiscrete());
	TypePtr iStruc(pField->owner());
	StrucDumper_t dumper(*this, proot(), indent(), disp(), ctx(), ed(), *iStruc);
	FieldMapCIt i(iStruc->typeStruc()->fields().find(pField->_key()));//address()));
	size_t gap(this->CheckUnderlap(i));
	assert(gap > 0);
	dumper.OutputFieldGap0(pField, ADDR(gap));
	if (IsProbing())
	{
		//probeSrc()->pickAddr() = pField->checkGap();
		probeSrc()->pickAddr(ADDR(pField->_key() - gap));
	}
}

TEMPL void DumperBase_t<T>::drawFieldDefinition0(CFieldPtr pField)
{
	drawFieldName(pField, nullptr, false, GlobalColor(pField));
	pickField(pField);
}

TEMPL void DumperBase_t<T>::pickField(CFieldPtr pField)
{
	if (!pField || !IsProbing())
		return;
	if (pField->isExported())
	{
		FieldPtr pImpField(this->FromExportedField(pField));
		if (pImpField)
			pField = pImpField;
	}
	probeSrc()->pickFieldDecl(pField);
}

TEMPL void DumperBase_t<T>::drawFieldDefinition(/*FuncDump er_t *pfd,*/ CFieldPtr pField, CFieldPtr pImpField, CTypePtr pScope, bool bDefinition)
{
	assert(!T::IsGlobal(pField));

	pickField(pField);

	/*if (pfd && pfd->IsDataDead(pField))
	{
		//		assert(IsOutputDeadData());//otherwise was not dumped
		PushColor(adcui::COLOR_COMMENT);
		mos << "//";
	}*/

	FileDumper_t& rRoot(root());

CHECKID(pField, 27702)
STOP

	if (pField->owner()->isEnum())
	{
		drawFieldName(pField, nullptr, false);// , GlobalColor(pField, pImpField));
		return;
	}
	
	if (FuncInfo_s::IsLocal(pField))
	{
		FuncInfo_t funcInfo(*this, *FuncInfo_t::GetLocalOwner(pField));
		FuncDumper_t funcDumper(funcInfo, proot(), m_indent, disp(), ctx(), ed());
		TProtoDumper<ProtoImpl4V_t<FuncDumper_t>> proto(funcDumper);//*reinterpret_cast<FuncDumper_t *>(this));
		proto.mbTabSep = true;
		proto.dumpFieldDeclaration(pField, pField->type(), nullptr);
		if (!FuncInfo_s::isLocalArg(pField))// || (IsLocalArg(pField) && pField->order() <= LOCAL_ORDER_ARG_UPPER))//only vars and args
		{
			if (FuncInfo_t::LocalRefs(pField).empty())
				dumpComment("unreferenced", false);
#ifdef _DEBUG
			else if (FuncInfo_t::FindDanglingOp(pField))
				dumpComment("dangling!", false);
#endif
		}
		return;
	}

	TProtoDumper<ProtoImpl2_t<DumperBase_t>> proto(*this);
	proto.mbTabSep = true;
	proto.mpImpField = nullptr;
	proto.dumpFieldDeclaration(pField, pField->type(), pScope);
}

TEMPL void DumperBase_t<T>::drawGlobDefinition(CGlobPtr pGlob, CGlobPtr pImpGlob, CTypePtr pScope, bool bDefinition)
{
	CFieldPtr pField(DcInfo_s::DockField(pGlob));
	assert(pField);
	CFieldPtr pImpField(pImpGlob ? DcInfo_s::DockField(pImpGlob) : nullptr);
	assert(DcInfo_s::IsGlobal(pField));
	assert(!FuncInfo_s::IsLocal(pField));
	
	pickField(pField);

	if (!pScope)
		pScope = pField->owner();

	/*if (pfd && pfd->IsDataDead(pField))
	{
		//		assert(IsOutputDeadData());//otherwise was not dumped
		PushColor(adcui::COLOR_COMMENT);
		mos << "//";
	}*/

	FileDumper_t& rRoot(root());

CHECKID(pGlob, 7575)
//CHECKID(pField, 1117)
STOP

	TypePtr pOwnerClass(nullptr);
	if (DcInfo_s::IsStaticMemberFunction(pGlob))
		pOwnerClass = DcInfo_s::OwnerScope(pGlob);

	if (pScope && pScope->isVTable())//v-tables are dumped with declaration
	{
		OFF_t oData(OFF_NULL);
		if (rRoot.GetRawOffset(pField, oData))
		{
			DataStream_t aRaw(rRoot.GetDataSource()->pvt(), oData);
			TypePtr pSeg(ProjectInfo_t::OwnerSeg(pField->owner()));
			dumpRawData(pField->type(), aRaw, nullptr, ATTR_NULL, pSeg, 0);
		}
		else
			dumpUnk();
		return;
	}

	/*?if (pField->type() && !pField->type()->type Proc() && !pField->type()->isShared())
	{
		drawFieldName(pField, pImpField);//should be dumped with declaration (but not for functions)
		//return;
	}*/

	if (ProjectInfo_t::IsExported(pField))
	{
		if (!pOwnerClass || pOwnerClass->typeNamespace() || !pOwnerClass->isExporting())
		{
			OutputDeclspecExpImp(pImpField != nullptr);//rRoot.IsImporting());
			dumpSpace();
		}
	}
	else if (!bDefinition)
	{
		if (!pOwnerClass && pField->owner()->typeSeg())
		{
			dumpReserved("extern");
			dumpSpace();
		}
	}

	/*if (rRoot.IsImporting() || rRoot.IsTypeImp(pField))
	{
	assert(!bDefinition);
	OutputDeclspecExpImp(true);
	mos << " ";
	}
	else*/
	{
		if (T::IsStaticMember(pGlob) && !bDefinition)
		{
			dumpReserved("static");
			dumpSpace();
		}

		if (rRoot.IsConst(pField))
		{
			dumpReserved("const");
			dumpSpace();
		}
	}

	assert(!pField->owner()->isEnum());
	assert(!FuncInfo_s::IsLocal(pField));

	TProtoDumper<ProtoImpl2_t<DumperBase_t>> proto(*this);
	proto.mbTabSep = true;
	proto.mpImpField = pImpField;
	proto.dumpFieldDeclaration(pField, pField->type(), pScope);
	//DumpVTableFor(pField);
}

TEMPL void DumperBase_t<T>::DumpGlobInitialization(CGlobPtr pGlob)
{
	CFieldPtr pField(DcInfo_s::DockField(pGlob));
	//initialized section
CHECKID(pField, 2475)
STOP

	assert(T::IsGlobal(pField));
	//if (IsProbing())
		//mpProbeSrc->pickField(pField);

//??		if (!pfd || !pField->isRet())
	{

		TypePtr pSeg(ProjectInfo_t::OwnerSeg(pField->owner()));
		if (T::CheckDataAtVA(pSeg, DcInfo_s::DockAddr(pGlob)))
		{
			DataSubSource_t data(T::GetDataSource()->pvt(), pSeg->typeSeg()->rawBlock());
			DataStream_t aRaw(data, pField->offset());
			mos << " = ";

			try
			{
				if (pField->isStringOf(OPTYP_CHAR8))
					dumpString(pField->type(), aRaw);
				else
					dumpRawData(pField->type(), aRaw, nullptr, ATTR_NULL, pSeg, 0);
			}
			catch (const DataAccessFault_t & e)
			{
				if (e.code() != 0)
					dumpColorTerm("...", adcui::COLOR_TAG_ERROR);
			}
			catch (...)
			{
				dumpColorTerm("<ERROR>", adcui::COLOR_TAG_ERROR);
			}
		}
	}


	/*?	if (pfd->IsStackLocal(pField) && pField->Offset() < 0)
		{
			int d = pField->size() + pField->Offset();
			if (d > 0)
			{
				PushColor(adcui::COLOR_COMMENT);
				mos << "//WARNING: OVERLAPPED @RETADDR!";
				mos << " (+" << d << ")";
			}
		}*/
}

TEMPL void DumperBase_t<T>::dumpComment(const char* pc, bool bCxxStyle)
{
	PushColor(adcui::COLOR_COMMENT);
	if (!bCxxStyle)
	{
		assert(pc);
		mos << "/*" << pc << "*/";
		PopColor();
	}
	else
	{
		mos << comment();
		if (pc)
			mos << pc;
	}
}

TEMPL void DumperBase_t<T>::OutputDeclspecExpImp(bool bImport)
{
	dumpReserved("__declspec");
	mos << "(";
	if (bImport)
		dumpReserved("dllimport");
	else
		dumpReserved("dllexport");
	mos << ")";
}

/*TEMPL void DumperBase_t<T>::DumpVTableFor(CFieldPtr pExtField)
{
	TypePtr iType(pExtField->type());
	if (iType)
	{
		TypeVFTable_t* pVFTable(iType->typeVFTable());
		if (pVFTable && pVFTable->baseType())
		{
			dumpComment("for ", true);
			dumpTypeRef(pVFTable->baseType());
		}
	}
}*/

TEMPL void DumperBase_t<T>::dumpStrucDecl(CTypePtr iSelf, bool bImporting)
{
	TypePtr pType0(ProjectInfo_t::SkipProxy(iSelf));
	if (pType0->typeTypedef())
	{
		CTypePtr pScope(DcInfo_t::OwnerScope(pType0));
		if (pScope && pScope->typeSeg())
			pScope = nullptr;
		TProtoDumper<ProtoImpl_t<DumperBase_t>> proto(*this);
		proto.mbTabSep = true;
		//?			proto.mbFieldHidden = true;
		proto.dumpTypedefDeclaration(pType0, pScope);
		mos << ";";
		return;
	}
	/*if (iSelf->typeProxy())
	{
		Dc_t &rDC(*DCREF(ModuleInfo_t::ModuleOf(iSelf->typeProxy()->incumbent())));
		FileInfo_t FI(rDC, fileInfo().FileDef());
		StrucDumper_t dumper(this, FI, *iSelf);
		dumper.DumpDeclaration();
	}
	else*/
	{
		assert(!iSelf->typeProxy() || bImporting);
		StrucDumper_t dumper(*this, proot(), indent(), disp(), ctx(), ed(), *iSelf);
		dumper.DumpDeclaration(bImporting);
	}
}

TEMPL void DumperBase_t<T>::dumpStrucEnd(CTypePtr pSelf)
{
	if (probeSrc())
		probeSrc()->pickAddr(pSelf->size());
}

TEMPL void DumperBase_t<T>::DumpStrucDef(CTypePtr pSelf)
{
	if (pSelf->typeProxy())
	{
		/*if (pSelf->nameless())
		{
			mos << "ERROR<?>";
			return;
		}*/
		CTypePtr pSelf0(ProjectInfo_t::SkipProxy(pSelf));
		//		iSelf0 = ProjectInfo_t::SkipUglyLocum(iSelf0);
		//		FolderPtr pFolder0(USERFOLDER(pSelf0));

#if(0)
		assert(!pSelf->nameless());
		ExpTypeInfo_t exp(root().GetExportedTypeInfo(pSelf->name()->c_str()));
		assert(exp.typeObj());
		assert(exp.typeObj() == iSelf0);
		pFolder0 = exp.folder();
		iSelf0 = exp.typeObj();
#endif
		//CHECKID(exp.typeObj(), -131)
		//STOP
				//Folder_t *pExpFolderTop(root().TopFolder());
		//!		Dc_t *pDC2(root().DcFromFolder(*pFolder0));
		//!		FileInfo_t FI0(*pDC2, *DcInfo_t::FILEDEF(pFolder0));
		StrucDumper_t strucDumper(*this, proot(), indent(), disp(), ctx(), ed(), *pSelf);//iSelf?
//		strucDumper.setImporting(pFolder0);
		strucDumper.NewLine();
		strucDumper.Dump(true);//pFolder0 != nullptr);
		strucDumper.dumpSemi();
	}
	else
	{
		StrucDumper_t strucDumper(*this, proot(), indent(), disp(), ctx(), ed(), *pSelf);
		strucDumper.NewLine();
		//strucDumper.dumpTab(indent());
		strucDumper.Dump(false);
		strucDumper.dumpSemi();
	}
}

TEMPL bool DumperBase_t<T>::IsGlobalVisible(CFieldPtr  pSelf) const
{
	//	pSelf->m_nFlags &= ~DAT_HIDDEN;

	if (FuncInfo_t::IsLocal(pSelf))
	{
		return 1;//always
		/*		if (!m_pXRefs)
		return 1;//not hidden

		for (XRef_t *pXRef = pSelf->m_pXRefs; pXRef; pXRef = pXRef->Ne xt())
		{
		HOP pOp = pXRef->pOp;
		if (pOp->IsOpVisible())
		return 1;
		}

		//		pSelf->m_nFlags |= DAT_HIDDEN;
		return 0;*/
	}

	if (m_disp.IsOutputDead())
		return 1;

	if (m_disp.isUnfoldMode())
		return 1;

	//test visibility of jump & index tables
	if (pSelf->isConst())
		if (!T::IsThruConst(pSelf))
			return 1;

	//?	if (FuncInfo_t::LabelXRefs(pSelf).check_count(1) != 0)
	//?		return 1;//only single reference of jump/index table is legal
	//assert(0);
	return 1;

	/*?	HOP  pOp = pSelf->xrefs().head()->data();
		if (!IsCodeOp(pOp))
			return 1;

		//CHECK(PrimeOp(pOp)->No() == 513)
		//STOP
		if (!pOp->CheckSwitch())
			return 1;//not hidden
			*/
			//	m_nFlags |= DAT_HIDDEN;
	return 0;
}

TEMPL void DumperBase_t<T>::DumpFunctionStubInfo(CGlobPtr pSelf)
{
	ProtoInfo_t TI(dc(), (GlobPtr)pSelf);
	FuncProfile_t fp;
	TI.GetFuncProfile(fp);
	StubInfo_t SI(*this);
	SI.dump_trimmed(fp, mos, '/');
}

TEMPL void DumperBase_t<T>::DumpVirtualMethodDeclaration(CGlobPtr iGlob, int off, FuncDumpFlags_t flags, CGlobPtr pImpGlob, CTypePtr pScope, bool bIsDefinition)
{
	if (draftMode())
	{
		if (!bIsDefinition)
			m_disp.dumpSym(adcui::SYM_VFUNCDECL, iGlob, off);
		else
			m_disp.dumpSym(adcui::SYM_FUNCDEF, iGlob);
		return;
	}
	DumpFunctionDeclaration0(iGlob, flags, pImpGlob, pScope, bIsDefinition);
}

TEMPL void DumperBase_t<T>::DumpFunctionDeclaration(CGlobPtr iGlob, FuncDumpFlags_t flags, CGlobPtr pImpGlob, CTypePtr pScope, bool bIsDefinition)
//1:owner_struc,2:arguments,4:returned,8:func_type,0x10:open_brace,0x20:close_brace
{
	//CHECKID(iGlob, 0x1038c)
	//STOP
	//CFieldPtr pImpField(pImpGlob ? pImpGlob->dockField() : nullptr);

	if (draftMode())
	{
		if (pImpGlob)
		{
			assert(0);
			m_disp.dumpSym(adcui::SYM_IMPCLSGLB, pImpGlob);
		}
		else if (!bIsDefinition)
			m_disp.dumpSym(adcui::SYM_FUNCDECL, iGlob);//no class
		else
			m_disp.dumpSym(adcui::SYM_FUNCDEF, iGlob);//all flags
		return;
	}

	DumpFunctionDeclaration0(iGlob, flags, pImpGlob, pScope, bIsDefinition);
}

TEMPL void DumperBase_t<T>::DumpFunctionDeclaration0(CGlobPtr pSelf, FuncDumpFlags_t flags, CGlobPtr pImpGlob, CTypePtr pScope, bool bIsDefinition)
{
	CFieldPtr pField(DcInfo_s::DockField(pSelf));
	if (m_disp.isUnfoldMode())
	{
		dumpFieldNameFull(pSelf);
		mos << ":";
		return;
	}

	const FuncDef_t& rFuncDef(*pSelf->typeFuncDef());

	CFieldPtr pImpField(pImpGlob ? DcInfo_s::DockField(pImpGlob) : nullptr);

	bool bIsHeader(!bIsDefinition);

	if (IsProbing())
	{
		probeSrc()->pickFuncDecl(pSelf);
		pickField(pField);
		if (pImpField)
			probeSrc()->pickImpFieldDecl(pImpField);
	}

	TypePtr pOwnerClass(DcInfo_t::OwnerScope(pSelf));
	/*TypePtr pOwnerClass(rFuncDef.isClassMember() ? rFuncDef.ownerClass() : nullptr);
	if (DcInfo_s::IsStaticMemberFunction(pField))
	{
		assert(!pOwnerClass);
		pOwnerClass = DcInfo_t::OwnerScope(pField);
	}
	else
	{
		assert(pOwnerClass == DcInfo_t::OwnerScope(pField));
	}*/

	if (bIsHeader)
	{
		if (pField && DcInfo_t::IsExported(pField))
			if (!pOwnerClass || pOwnerClass->typeNamespace() || !pOwnerClass->isExporting())
			{
				OutputDeclspecExpImp(pImpField != nullptr);//FD.IsImporting() || pImpField);
				mos << " ";
			}
	}

	if (DcInfo_s::IsStaticMemberFunction(pSelf))
	{
//?		assert(!(pSelf->flags() & FDEF_VIRTUAL));
		if (!pOwnerClass->typeNamespace())
		{
			dumpReserved("static");
			mos << " ";
		}
		else if (pSelf->flags() & FDEF_STATIC)
		{
			dumpPreprocessor("STATIC");//this should be a warning
			mos << " ";
		}
	}
	else if (bIsHeader && ClassInfo_t::IsMethodVirtual(pSelf))
	{
//CHECKID(pField, 1114)
//STOP
		if (!ClassInfo_t::IsMethodPseudoVirtual(pSelf))//O(n)
			dumpReserved("virtual");
		else
			dumpPreprocessor("VIRTUAL");//not yet real 'virtual'
		mos << " ";
	}

	//if (!pScope)
		//pScope = PrimeSeg();// OwnerSeg(pField->owner()));

	TProtoDumper<ProtoImpl4F_t<DumperBase_t>> proto(*this);
	proto.mpField = pField;
	proto.mpImpField = pImpField;
	proto.mbTabSep = true;
	proto.mbIsDef = bIsDefinition;
	proto.mbUglyName = (flags & FUNCDUMP_UGLY_NAME) != 0;
	proto.mbDockAddr = !bIsHeader;
	proto.dumpFunctionDeclaration(pField, (GlobToTypePtr)pSelf, pScope);

	if (flags & FUNCDUMP_MODIFIER)
	{
		ProtoInfo_t TI(*this, pSelf);
		if (TI.IsConstClassMember())//pSelf->flags() & FDEF_CONST
		{
			mos << " ";
			if (rFuncDef.isStub())
				dumpReserved("const");
			else
				dumpPreprocessor("CONST");//not yet real 'const'
		}
	}
}

TEMPL void DumperBase_t<T>::DumpFunctionClosingInfo(CGlobPtr pSelf)
{
	return;
#ifdef _DEBUG
	const FuncDef_t& rFuncDef(*pSelf->typeFuncDef());

	if (!m_disp.IsOutputDead())
		if (!rFuncDef.isStub())
			return;

	if (m_disp.dumpSym(adcui::SYM_STUBINFO, (GlobToTypePtr)pSelf))
		return;

	PushColor(adcui::COLOR_COMMENT);
	mos << "//";

	dumpFieldRef(DcInfo_t::DockField(pSelf));

	mos << "(";

	if (ProtoInfo_t::IsFuncStatusAborted(pSelf))
	{
		mos << "###INVALID###";
	}
	else
	{
		if (!m_disp.testOpt1(adcui::DUMP_LOGICONLY))
		{
			//FuncDef_t * pFuncDef = mrFunc.funcd ef();
			DumpFunctionStubInfo(pSelf);
			if (!rFuncDef.isStub())
			{
				int iExitsNum(FuncInfo_t::GetExitPoints(rFuncDef).count());
				if (iExitsNum == 0)//stub!
					if (!rFuncDef.exitOps().empty())
						iExitsNum = 1;
				mos << ":[" << iExitsNum << "]";
			}
		}
	}

	mos << ")";
#endif
}

TEMPL void DumperBase_t<T>::DumpOpenBrace()
{
	mos << "{";
	IncreaseIndent();
}

TEMPL void DumperBase_t<T>::DumpCloseBrace()//TreePathHierIterator iPath)
{
	//HPATH pPath(iPath.data());
	if (m_disp.isUnfoldMode())
	{
		NewLine();
		//dumpTab(GetPathIndent(pPath));
		mos << "]";
		return;
	}

	DecreaseIndent();
	NewLine();

	mos << "}";
}

TEMPL void DumperBase_t<T>::DumpFunctionStub(CGlobPtr pSelf)
{
	NewLine();
	DumpFunctionDeclaration(pSelf, FUNCDUMP_IMPL_FLAGS, nullptr, PrimeSeg(), true);
	mos << "{";
	NewLine(false);
	mos << "}";
	DumpFunctionClosingInfo(pSelf);
	//NewLine();
}




//explicit template instantiation
template class DumperBase_t<DcInfo_t>;
template class DumperBase_t<FileInfo_t>;
template class DumperBase_t<FuncInfo_t>;
template class DumperBase_t<ProtoInfo_t>;




