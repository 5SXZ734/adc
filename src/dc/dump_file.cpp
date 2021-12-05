#include "dump_file.h"
//#include <iostream>
//#include <iomanip>
#include <set>
#include "qx/MyStringList.h"
#include "shared/defs.h"
#include "shared/link.h"
#include "shared/misc.h"
#include "shared/front.h"
#include "shared/action.h"
#include "shared/data_source.h"
#include "db/mem.h"
#include "db/obj.h"
#include "db/field.h"
#include "db/command.h"
#include "db/type_seg.h"
#include "db/types.h"
#include "db/proj.h"
#include "db/types_mgr.h"
#include "info_dc.h"
#include "files_ex.h"
#include "dump_func.h"
#include "dump_scan.h"
#include "info_class.h"
#include "interface/IADCGui.h"

int FileDumper_t::globStatus(CGlobPtr pGlob)//0:hidden,1:variable,2:function
{
	if (!IsStaticMember(pGlob))
	{
		if (!pGlob->func())//variables
			return 1;
		//if (!ifDef->typeFuncDef()->isClassMember())
		if (!ProtoInfo_t::ThisPtrArg(pGlob))
			return 2;
	}
	return 0;//skip class members (incl statics), dumped with class declaration
}

void FileDumper_t::dumpGlobal(CGlobPtr pGlob, CGlobPtr pImpGlob, CTypePtr pScope)
{
	CFieldPtr pImpField(DockField(pImpGlob));
	/*if (globStatus(pGLob) == 1)//variables
	{
		dumpFi eldDecl(pField, pImpField, pScope);
	}
	else*/
	{
		if (pGlob->func())
		{
			DumpFunctionDeclaration(pGlob, FUNCDUMP_DECL_FLAGS, pImpGlob, pScope);
		}
		else
		{
			//dumpF ieldDecl(pField, pImpField, pScope);
			if (!m_disp.dumpSym(adcui::SYM_GLBDECL, pGlob))
				drawGlobDefinition(pGlob, pImpGlob, pScope, !IsHeader());
		}
	}
	pickField(pImpField);
}

void FileDumper_t::drawGlobal(CGlobPtr pGlob, CGlobPtr pImpGlob, CTypePtr pScope)
{
	CFieldPtr pImpField(pImpGlob ? DockField(pImpGlob) : nullptr);
	if (pGlob->func())
	{
		DumpFunctionDeclaration0(pGlob, FUNCDUMP_DECL_FLAGS, pImpGlob, pScope);
	}
	else
	{
		drawGlobDefinition(pGlob, pImpGlob, pScope, !IsHeader());
	}

	pickField(pImpField);
}

////////////////////////////////////////////////////////
// ImpDumper_t

/*class ImpDumper_t : public FileDumper_t
{
	CFieldPtr		mpField;
	TypePtr			mpScope;
	CFieldPtr		mpImpField;
public:
	ImpDumper_t(DumperBase_t &from, Dc_t &dcRef, FileDef_t &fileDef, CFieldPtr pField, TypePtr pScope)
		: FileDumper_t(from, dcRef, fileDef),
		mpField(pField),
		mpScope(pScope),
		mpImpField(nullptr)
	{
	}
	void setImpField(CFieldPtr p){
		mpImpField = p;
		assert(mpImpField && mpImpField->isTypeImp());
	}
	FileDumper_t &dumper() { return *this; }
	CFieldPtr expField() const { return mpField; }
	void dump()
	{
		dumpGlobal(mpField, mpImpField, mpScope);
	}
};*/







////////////////////////////////////////////////////

FileDumper_t::FileDumper_t(const FileInfo_t &o, Display_t &rDisp, ProbeEx_t* pCtx, const MyLineEditBase* ped, bool bImporting)
	: DumperBase_t(nullptr, 0, rDisp, pCtx, ped, o),
	//FileInfo_t(r),
	mbImporting(bImporting)//,
	//mpFileTempl(nullptr)
{
	m_pRoot = this;
	/*FolderPtr iFolderTempl(mrDC.folderPtr(FTYP_TEMPLATES));//ugly types reside only in this file
	if (iFolderTempl)
		mpFileTempl = iFolderTempl->fileTempl();*/
}

/*FileDumper_t::FileDumper_t(const FileInfo_t &r, const DumperBase_t<FileInfo_t> &from)//, Dc_t &dcRef, FileDef_t &fileDef)
	: DumperBase_t(from),
//	FileInfo_t(r),//from, rDC, from.root().FileDef()),//!file of a different binary
	mbImporting(true)
{
	//assert(exp.pField->isExported());
}*/

/*TypePtr FileDumper_t::FromUglyType(TypePtr iType)
{
	if (!mpFileTempl)
		return nullptr;
	CheckTemplatesMappings(false);
	return mpFileTempl->backType(iType);
}*/

void FileDumper_t::DumpContents(CFolderPtr pSelf)
{
	FileDef_t *pFileData(FILEDEF(pSelf));
	if (!pFileData)
		return;

	CTypePtr iCurNS(nullptr);//namspace tracking
	if (IsHeader())
	{
		for (TypesListCIt i(pFileData->types().begin()); i != pFileData->types().end(); i++)
		{
			TypePtr pType(*i);
CHECKID(pType, 0xf39e)
STOP
			DumpTypeDecl(pType, iCurNS);
		}
	}

	bool bFuncBefore(false);
	for (GlobMapCIt i(pFileData->globs().begin()); i != pFileData->globs().end(); i++)
	{
		CGlobPtr pGlob(&(*i));
//		CFieldPtr pField(i->parentField());
//CHECKID(pField, 4332)
//STOP
		if (IsHeader())
			DumpGlobDecl(pGlob, iCurNS);
		else
			bFuncBefore = DumpGlobDef(pGlob, bFuncBefore);
	}

	DumpNamespaceSwitch(nullptr, iCurNS);
}

void FileDumper_t::DumpChosen(CTypeBasePtr pChosen)
{
	CTypePtr iCurNS(nullptr);//namspace tracking
	if (pChosen->objGlob())
	{
		DumpGlobDef(pChosen->objGlob(), false);
	}
	else
	{
		assert(pChosen->objTypeGlob());
		DumpTypeDecl(pChosen->objTypeGlob(), iCurNS);
	}
	DumpNamespaceSwitch(nullptr, iCurNS);
}

void FileDumper_t::dumpNamespaceOpen0(CTypePtr p)
{
	NewLine();
	dumpReserved("namespace");
	mos << " ";
	dumpTypeRef(p);
	mos << " {";
	IncreaseIndent();
}

void FileDumper_t::dumpNamespaceClose0(CTypePtr p)
{
	DecreaseIndent();
	NewLine();
	mos << "}";
	dumpComment(nullptr, true);
	dumpTypeRef(p);
	NewLine();
}

CTypePtr FileDumper_t::dumpNamespaceOpen(CTypePtr iSelf, CTypePtr iNS)
{
	if (iSelf && iSelf != iNS)
	{
		CTypePtr iScope(iSelf->ownerScope());
		if (iScope != iNS)
			dumpNamespaceOpen(iScope, iNS);
		dumpNamespaceOpen0(iSelf);
	}
	return iSelf;
}

CTypePtr FileDumper_t::DumpNamespaceSwitch(CTypePtr iSelf, CTypePtr iNS)
{
	//find a common ancestor, or redurn iNS (unchanged)

	if (iSelf == iNS)
		return iNS;//not changed

	if (!iNS)
		return dumpNamespaceOpen(iSelf, iNS);//nothing to exit, going to enter some
	
	CTypePtr p(CommonScope(iSelf, iNS));
	if (p)
	{
		//exit until a common ancestor
		for (; iNS != p; iNS = iNS->ownerScope())
			dumpNamespaceClose0(iNS);
		return dumpNamespaceOpen(iSelf, iNS);
	}

	//no common ancestor found - a totally new scope hierarchy (or entering a global scope) - do exit all scopes
	for (; iNS; iNS = iNS->ownerScope())
		dumpNamespaceClose0(iNS);
	return dumpNamespaceOpen(iSelf, iNS);
}

void FileDumper_t::DumpTypeDecl(CTypePtr iType, CTypePtr &iNamespace)
{
CHECKID(iType, 0x846)
STOP

	//TypePtr iType(pSelf->objTypeGlob());
	assert(iType);

	if (m_disp.isUnfoldMode())
		return;

/*	if (iType->isUgly())
	{
		assert(FileDef().ownerFolder() == mrDC.folderPtr(FTYP_TEMPLATES));//ugly types reside only in this file
		FileTempl_t *pFileTempl(CheckTemplatesMappings(false));
		TypePtr iTypedef(pFileTempl->backType(iType));
#if(0)
		NewLine();
		dumpTypedefDeclaration(iTypedef);
		mos << ";";
#else
		NewLine();
		dumpReserved("#define");
		mos << " ";
		if (!iTypedef)
		{
			FolderPtr pf(USERFOLDER(iType));
			TypePtr p(ModuleOf(iType));
			dumpPreprocessor("<unknown>");
		}
		else if (!dumpObjRef(iTypedef, adcui::SYM_TYPEDEF))
			mos << TypeName(iTypedef).c_str();
		dumpTab();
		dumpTypeRef(iType);
#endif
		return;
	}*/

	if (iType->typeNamespace())
		return;

	CTypePtr iType0(SkipProxy(iType));
	assert(!IsProc(iType0));
	if (iType0->typeNamespace())
		return;
	if (iType0->isNested() && !iType0->owner()->typeNamespace())//nested types are dumped by its container
		return;

	//first check if some of namespaces should be closed
	iNamespace = DumpNamespaceSwitch(iType0->ownerScope(), iNamespace);

	//if (iOwner != iNamespace)
		if (dumpNamespaceOpen(iType0->ownerScope(), iNamespace))
			iNamespace = iType0->ownerScope();

	if (iType->typeFuncDef())//intrinsics!
	{
		NewLine();
		TProtoDumper<ProtoImpl4F_t<FileDumper_t>> proto(*this);
		proto.dumpFuncdefDeclaration(iType, nullptr);
		mos << ";";
		return;
	}

	if (iType0->typeStruc())
	{
		if (!(m_disp.testOpt1(adcui::DUMP_LOGICONLY | adcui::DUMP_NOSTRUCS)))
		{
//CHECKID(iType, -131)
//STOP
			bool bCont(openScope(iType0));//???, nullptr, m_disp.linesInChunk()));
			DumpStrucDef(iType);
			if (bCont)
				m_disp.closeScope(false);
		}
		NewLine();
	}
	else if (iType0->typeTypedef())
	{
		NewLine();
		if (iType->typeProxy())
		{
			if (m_disp.dumpSym(adcui::SYM_IMPTYPEDEF, iType))
				return;
		}
		else
		{
			if (m_disp.dumpSym(adcui::SYM_TYPEDEF, iType))
				return;
		}

		TProtoDumper<ProtoImpl_t<FileDumper_t>> proto(*this);
		proto.mbTabSep = true;
		//?			proto.mbFieldHidden = true;
		proto.dumpTypedefDeclaration(iType0, DcInfo_t::OwnerScope(iType0));
		mos << ";";
	}
	else
	{
		Type_Dump(iType);
	}
}

bool FileDumper_t::drawImpGlobDecl(CGlobPtr pImpGlob, CTypePtr pScope)
{
	CFieldPtr pImpField(DockField(pImpGlob));
	assert(pImpField->isTypeImp());
	FieldPtr pExpField(root().ToExportedField(pImpField));
	if (!pExpField)
	{
		mos << "?";
		return false;//should not happen
	}
	GlobPtr pExpGlob(GlobObj(pExpField));
	if (!pScope)
		pScope = DcInfo_t::OwnerScope(pExpGlob);
	if (IsStaticMember(pExpGlob))
	{
		//if (!DumpSym(adcui::SYM_IMPCLSGLB, GlobObj(pExpField)))
			drawImpClsGlobDecl(pExpGlob, pImpGlob, pScope);
	}
	else
	{
		//ImpDumper_t impDumper(*this, dc(), FileDef(), pExpField, pScope);//declaratioons are always in object's own scope
		//impDumper.setImpField(pImpField);
		//impDumper.dump();
		if (!pExpGlob)
		{
			mos << "?";
			return false;
		}
		drawGlobal(pExpGlob, pImpGlob, pScope);
	}
	return true;
}

void FileDumper_t::DumpGlobDecl(CGlobPtr pGlob, CTypePtr &iNamespace)
{
	CFieldPtr pField(DockField(pGlob));
//CHECKID(pField, 11945)
//STOP

	//variable?
	assert(pField);

	if (IsTypeImp(pField))
	{
CHECKID(pField, 2907)
STOP
		CFieldPtr pExpField(root().ToExportedField(pField));
		if (pExpField)//? check it
		{
			CGlobPtr pExpGlob(GlobObj(pExpField));
			TypePtr pScope(OwnerScope(pExpGlob));
			//ImpDumper_t impDumper(*this, dc(), FileDef(), pExpField, OwnerScope(pExpField));
			//impDumper.setImpField(pField);
			//assert(impDumper.expField());
			if (!IsStaticMember(pExpGlob))//not hidden
			{
				iNamespace = DumpNamespaceSwitch(pScope, iNamespace);
				NewLine();
				if (!m_disp.dumpSym(adcui::SYM_IMPGLB, pGlob))
					//impDumper.dump();
					dumpGlobal(pExpGlob, pGlob, pScope);
				mos << ";";
			}
			return;
		}
		else if (0)
		{
			NewLine();
			mos << "<ERROR:2>";
			return;
		}
	}

	TypePtr iScope(OwnerScope(pGlob));
	//if (IsStaticMemberFunction(pField))
	if (iScope && !iScope->typeNamespace())
		return;//dumped in class

	iNamespace = DumpNamespaceSwitch(iScope, iNamespace);

	if (!pGlob->func())
	{
		if (!pField->type() || IsProc(pField->type()) || !IsConst(pField) || pField->hasUserData())//display the funcs, skip const datas
		{
			NewLine();
			//dumpFi eldDecl(pField, nullptr, OwnerScope(pField));
			if (!m_disp.dumpSym(adcui::SYM_GLBDECL, pGlob))
				drawGlobDefinition(pGlob, nullptr, iScope, false);
			mos << ";";
		}
	}
	else
//	if (!ifDef->typeFuncDef()->isClassMember())//skip class members
	{
		NewLine();
		DumpFunctionDeclaration(pGlob, FUNCDUMP_DECL_FLAGS, nullptr, OwnerScope(pGlob));
		mos << ";";
	}
}

bool FileDumper_t::DumpGlobDef(CGlobPtr pGlob, bool bFuncBefore)//variables or functions definitions (no types)
{
	//bool bEmptyLine = true;
	CFieldPtr pField(DockField(pGlob));
//CHECKID(pField,864)
//STOP
	
//	if (!m_disp.IsOut putDead() && m_disp.Is ObjDead(0, pSelf))
	//	return;

	if (pField->isTypeImp())
		return false;

	if (pGlob->func())
	{
		NewLine();
		DumpFunc(pGlob);
		return !m_disp.isUnfoldMode() || !ProtoInfo_t::IsStub(pGlob);
	}
	if (1 && pField->isTypeThunk())
	{
		DumpThunk(pGlob);
		return false;
	}

	if (bFuncBefore)
		NewLine();

	if (!m_disp.isUnfoldMode())
	{
		//CFieldPtr pField0(CloneLead(pField));

		/*TypePtr iType(pField->type());
		if (iType)
		{
			/ *if (!iType->isShared())
			{
				if (!iType->type Proc())
					return false;//dumped with declaration
			}
			else * /if (iType->typeSimple())
			{
				if (IsThruConst(pField))
					if (OwnerFolder() != mrDC.folderPtr(FTYP_CONST))
						return false;//thru consts are dumped at site of ref
			}
		}*/

		if (IsGlobalVisible(pField))
		{
			NewLine();
			//drawFieldDef(pField, nullptr, nullptr);
			if (!m_disp.dumpSym(adcui::SYM_GLBDEF, pGlob))
				drawGlobDefinition(pGlob, nullptr, nullptr, true);

			DumpGlobInitialization(pGlob);
			mos << ";";
		}
	}
	return false;
}

void FileDumper_t::File_OutIDStr(CFolderPtr pSelf, bool bHeader)
{
	MyFolderKey s(pSelf->name());
	//_strupr(buf);
	char *p(&(*s.begin()));
	for (; *p; ++p) *p = toupper(*p);

	mos << "__";
	mos << s;
	if (bHeader)//pSelf->IsHeader())
		mos << "_H_INCLUDED__";
	else
		mos << "_C_INCLUDED__";
}

void FileDumper_t::DumpFunc(CGlobPtr pSelf)
{
	assert(!pSelf->typeProxy());
	assert(pSelf->typeFuncDef());
CHECKID(pSelf,0x242f)
STOP

	const FuncDef_t &rFuncDef(*pSelf->typeFuncDef());
	if (rFuncDef.isStub())
		//if (m_disp.isUnfoldMode())
		if (m_disp.testOpt1(adcui::DUMP_NOSTUBS))
			return;
	
//	NewLine();

	bool bCont(openScope(pSelf));//???, pField, m_disp.linesInChunk()));
	DumpFuncDef(pSelf);
	if (bCont)
		m_disp.closeScope(false);
}

void FileDumper_t::DumpThunk(CGlobPtr pSelf)
{
	NewLine();
	assert(DockField(pSelf)->isTypeThunk());
	dumpReserved("thunk");
	dumpSpace();
	dumpFieldNameFull(pSelf);
	mos << " {";
	NewLine();
	mos << "}";
	//NewLine();
}

void FileDumper_t::DumpPrologueInfo(CFolderPtr pSelf)
{
	dumpComment("-----------------------------------------------------------------------------", 1);
	NewLine();

	OutputFileBeg(pSelf);
	NewLine();

	dumpComment("", true);
	NewLine();

	OutputDescBeg(pSelf->mDesc.c_str());
	NewLine();

	const char *p = pSelf->mDesc.c_str();
	while (p)
	{
		while (*p && !iscntrl(*p)) p++;
		if (*p == 0)
			break;
		if (*p == 0xD)
		{
			p++;
			if (*p == 0xA)
				p++;

			OutputDescRest(p);
			NewLine();
		}
	}

	if (!m_disp.testOpt1(adcui::DUMP_NOLOGO))
	{
		dumpComment("", true);
		NewLine();

		OutputLogoStr(MyStringf(" This file was generated with %s.", mrMain.appName().c_str()));
		NewLine();

		OutputLogoStr(MyStringf(" Copyright (c) 2001-2020 by %s.", mrMain.companyName().c_str()));
		NewLine();

		OutputLogoStr(" All Rights Reserved.");
		NewLine();
	}

	dumpComment("-----------------------------------------------------------------------------", 1);
	NewLine();
}

/*void FileDumper_t::DumpTypedef(const char *source, const char *target)
{
	NewLine();
	dumpReserved("typedef");
	mos << " ";
	dumpReserved(source);
	mos << " ";
	dumpPreprocessor(target);
	mos << ";";
}*/

void FileDumper_t::DumpPrefixContents(CFolderPtr pSelf)
{
	/*DumpTypedef("unsigned char", "UNK");
	DumpTypedef("unsigned char", "BYTE");
	DumpTypedef("unsigned short", "WORD");
	DumpTypedef("unsigned int", "DWORD");
	DumpTypedef("char", "INT8");
	DumpTypedef("short", "INT16");
	DumpTypedef("int", "INT32");*/

	NewLine();

	NewLine();
	dumpReserved("#define");
	mos << " ";
	dumpPreprocessor("unexplored");
	mos << "(offset, count)	";
	dumpPreprocessor("BYTE");
	mos << " _##offset[count]";

#if(1)
	NewLine();

	NewLine();
	dumpComment("Intrinsic functions", false);

	NewLine();
	
	NewLine();
	dumpReserved("double");
	mos << " ";
	dumpReserved("sqrt");
	mos << "(";
	dumpReserved("double");
	mos << ");";
	
	NewLine();
	dumpReserved("double");
	mos << " ";
	dumpReserved("cos");
	mos << "(";
	dumpReserved("double");
	mos << ");";

	NewLine();
	dumpReserved("double");
	mos << " ";
	dumpReserved("sin");
	mos << "(";
	dumpReserved("double");
	mos << ");";
#endif

	//dump registered intrinsics
	CTypePtr ns(nullptr);
	for (size_t i(0); i < mrDC.mIntrinsics.size(); i++)
		DumpTypeDecl((GlobToTypePtr)mrDC.mIntrinsics[i], ns);
}

int FileDumper_t::DumpIncludes(CFolderPtr pSelf)
{
	int ret(0);
	FileDef_t *pFileDef(pSelf->fileDef());
	if (pFileDef && !pFileDef->includes().empty())
	{
		//NewLine();
		for (IncludeListIt it(pFileDef->includes().begin()); it != pFileDef->includes().end(); it++)
		{
			FolderPtr pFolder(*it);
			DumpInclude(pFolder);
			ret++;
		}
	}
	return ret;
}

void FileDumper_t::DumpFile()
{
	FolderPtr pSelf(OwnerFolder());

#if(!NO_FILE_PROLOGUE)
	DumpPrologueInfo(pSelf);
#endif

	/*if (IsHeader())
		if (pSelf == mrDC.folderPtr(FTYP_CONST))
			return;*/

	if (!m_disp.isUnfoldMode())
	{
		if (IsHeader())
		{
			DumpHeaderGuardOpen(pSelf);
			DumpHeaderGuard(pSelf);
			NewLine();
			if (DumpIncludes(pSelf) > 0)
				NewLine();
		}
		else
		{
			CFolderPtr pPrefix(mrDC.folderPtr(FTYP_PREFIX));
			if (pPrefix)
				DumpInclude(pPrefix);
			DumpInclude(OwnerFolder());
			//NewLine();
		}
	}

	DumpContents(pSelf);

	if (!m_disp.isUnfoldMode())
	{
		if (mrDC.isFolderOfKind(*pSelf, FTYP_PREFIX))
		{
			DumpPrefixContents(pSelf);
		}

		NewLine();
		if (IsHeader())
		{
			//NewLine();
			DumpHeaderGuardClose(pSelf);
		}
	}

	//NewLine();
	
	//file end
	NewLine();
	OutputFileEnd(pSelf);
	//NewLine();
}

void TYP_t::dump(std::ostream &os, expr::IOut &o) const
{
	if (mpType)
	{
		MyString s;
		mpType->dump(os, s, o);
	}
	else
		o.dump(os, OPTYP_NULL);
}


MyString FileDumper_t::ExpandLine(int lineNo, const char * line0, int &len0, const DumpContext_t &dctx)
{
	//m_disp.setCurLine(lineNo);
CHECK(lineNo == 806)
STOP

	DisplayUI_t &dispUi(static_cast<DisplayUI_t &>(m_disp));
	assert(OwnerFolder() == dispUi.file());

	DisplayLineUI_t di(dispUi);//COLORS!
	di.setCurLine(lineNo);

	FileDumper_t gd(*this, di, ctx(), ed());

	CGlobPtr iFuncDef(dctx.asFunc2());
	if (iFuncDef)
	{
		assert(iFuncDef->typeFuncDef());
		if (!ProtoInfo_t::IsStub(iFuncDef))
		{
			FuncInfo_t funcInfo(DcRef(), *iFuncDef);// , mrFileDef);
			FuncDumper_t fd(funcInfo, &gd, gd.indent(), gd.disp(), gd.ctx(), gd.ed());// , funcInfo);
			FuncScanner_t<FuncDumper_t> fs(fd);
			//fs.mr.DumpNoFuncdef();
			len0 = fs.scan(line0);
			return di.finalizeAsString();
		}
		ProtoInfo_t TI(DcRef(), iFuncDef);
		ProtoDumper_t PD(TI, &gd, gd.indent(), gd.disp(), gd.ctx(), gd.ed());
		ProtoScanner_t<ProtoDumper_t> fs(PD);
		len0 = fs.scan(line0);
		return di.finalizeAsString();
	}

	FileScanner_t<FileDumper_t> gs(gd);
	len0 = gs.scan(line0);
	return di.finalizeAsString();
}

MyString FileDumper_t::ExpandLinePlain(int lineNo, const char * line0, int &len0, const DumpContext_t &dctx)
{
	//m_disp.setCurLine(lineNo);

	DisplayUI_t &dispUi(static_cast<DisplayUI_t &>(m_disp));
	assert(OwnerFolder() == dispUi.file());

	DisplayLineUI0_t di(dispUi);//NO COLORS!
	di.setCurLine(lineNo);

	FileDumper_t gd(*this, di, ctx(), nullptr);

	CGlobPtr iFuncDef(dctx.asFunc2());
	if (iFuncDef)
	{
		assert(iFuncDef->typeFuncDef());
		if (!ProtoInfo_t::IsStub(iFuncDef))
		{
			FuncInfo_t fi(DcRef(), *iFuncDef);
			FuncDumper_t fd(fi, &gd, gd.indent(), gd.disp(), gd.ctx(), nullptr);
			FuncScanner_t<FuncDumper_t> fs(fd);
			len0 = fs.scan(line0);
			return di.finalizeAsString();
		}
		ProtoInfo_t fi(DcRef(), iFuncDef);
		ProtoDumper_t fd(fi, &gd, gd.indent(), gd.disp(), gd.ctx(), nullptr);
		ProtoScanner_t<ProtoDumper_t> fs(fd);
		len0 = fs.scan(line0);
		return di.finalizeAsString();
	}

	FileScanner_t<FileDumper_t> gs(gd);
	len0 = gs.scan(line0);
	return di.finalizeAsString();
}

void FileDumper_t::write(std::ostream &os)
{
	int lines(dynamic_cast<DisplayUI_t&>(m_disp).linesTotal());
	for (int i(0); i < lines; i++)
	{
		DumpContext_t dctx;
		const char *line(dynamic_cast<DisplayUI_t &>(m_disp).GetDataAtLine(i, &dctx));
		int len;
		std::string s(ExpandLine(i, line, len, dctx));
		if (!s.empty() && s[s.length() - 1] == 0)
			s.resize(s.length() - 1);
		os << s << std::endl;
	}
}

void FileDumper_t::drawFieldDecl(CFieldPtr pField, bool bNameOnly)
{
	if (bNameOnly)
		drawFieldDefinition0(pField);
	else
		drawFieldDefinition(pField, nullptr, DcInfo_t::OwnerScope(pField), false);

	pickField(pField);
}

void FileDumper_t::drawGlobDecl(CGlobPtr pGlob)
{
	drawGlobDefinition(pGlob, nullptr, DcInfo_s::OwnerScope(pGlob), false);

//	pickField(pField);
}


/*bool FileDumper_t::IsObjVisible(const FuncInfo_t *pfi, Obj_t * pObj) const
{
if (m_disp.IsOutp utDead() || !IsObj Dead(pfi, pObj))
return true;
return false;
}*/

/*bool FileDumper_t::IsObjDead(const FuncInfo_t *pfi, Obj_t * pObj) const
{
//if (m_disp.isUnfoldMode())
{
CFieldPtr pField(pObj->objField());
if (!pField)
return 0;
return Is FieldDead(*pfi, pField);
}
}*/

void FileDumper_t::OutputLogoStr(const char * pc)
{
	PushColor(adcui::COLOR_COMMENT);
	mos << comment();
	PushColor(adcui::COLOR_FONT_BOLD);
	mos << pc;
}

void FileDumper_t::OutputFileBeg(CFolderPtr pFile)
{
	PushColor(adcui::COLOR_COMMENT);
	mos << comment();
	mos << " File: ";

	mos << pFile->name();
	PopColor();
}

void FileDumper_t::OutputFileEnd(CFolderPtr pFile)
{
#if(0)
	PushColor(adcui::COLOR_COMMENT);
	mos << comment();
	mos << " End Of File: ";

	mos << pFile->theName();
	PopColor();
	NewLine();
#endif
}

static void output_string(std::ostream &os, const char *p)
{
	if (!p)
	{
		os << "<n/a>";
		return;
	}
	while (*p && !iscntrl(*p))
	{
		os << *p;
		p++;
	}
}

void FileDumper_t::OutputDescBeg(const char * pc)
{
	PushColor(adcui::COLOR_COMMENT);
	mos << comment();
	mos << " Desc: ";
	output_string(mos, pc);
}

void FileDumper_t::OutputDescRest(const char * pc)
{
	PushColor(adcui::COLOR_COMMENT);
	mos << comment();
	mos << "       ";
	output_string(mos, pc);
}



//////////////////////////////////////////// StrucDumper_t

CFieldPtr Struc_t::GetOverlappingField(int &nLevel, FieldMapCIt it0) const//field that overlaps this one
{
	CFieldRef rField0(*it0);
	ADDR target(rField0._key());
	
	FieldMapCIt it(it0);

	for (;;)
	{
		if (it == mFields.begin())
			return nullptr;

		--it;
		CFieldRef rField(*it);
		if (FuncInfo_s::SSIDx(&rField) != FuncInfo_s::SSIDx(&rField0))
			return nullptr;

		ADDR upper(rField._key() + rField.size());
		if (upper > target)//overlaps
		{//immidiate overlapping
			nLevel++;
			GetOverlappingField(nLevel, it);
			break;
		}
	}

	return VALUE(it);
}

void StrucDumper_t::DumpField(FuncInfo_t *pfi, FieldMapCIt it, FieldMapCIt itPrior)
{
	CFieldPtr pSelf(VALUE(it));
//CHECK(pSelf->Offset() == 0x2BA)
//STOP
	if (mrTypeRef.isEnum())
	{
		//dump FieldDecl(pSelf, nullptr, OwnerScope(pSelf));
		if (!m_disp.dumpSym(adcui::SYM_FLDDECL, pSelf))
			drawFieldDefinition(pSelf, nullptr, OwnerScope(pSelf), false);

		if (itPrior == mrStruc.fields().end())
		{
			if (pSelf->_key() != 0)
				mos << " = " << Int2Str(pSelf->_key(), I2S_HEXC);
		}
		else
		{
			if (pSelf->_key() - itPrior->_key() > 1)
				mos << " = " << Int2Str(pSelf->_key(), I2S_HEXC);
		}
		if (++it != mrStruc.fields().end())
			mos << ",";
	}
	else
	{
		unsigned uOverlap(0);
		if (FuncInfo_s::SSIDx(pSelf) == SSID_GLOBAL)
		{
			if (!mrTypeRef.typeUnion())
			{
				FieldMapCIt itnx(it);
				if (++itnx != mrStruc.fields().end())
				{
					CFieldPtr pField2(VALUE(itnx));
					if (pSelf->_key() != pField2->_key())//not a union
						if (pSelf->addressHi() > pField2->_key())
							uOverlap = pSelf->addressHi() - pField2->_key();
				}

#if(0)
				if (uOverlap > 0)
				{
					PushColor(adcui::COLOR_ERROR);
					mos << "(+" << Int2Str(uOverlap, I2S_HEXC) << ") ";
					PopColor();
				}
#endif
			}
		}
		else if (!mrStruc.typeStrucLoc())//nor funcdefs?
		{
			int nLevel = 0;
			CFieldPtr pFieldTop = mrStruc.GetOverlappingField(nLevel, it);
			if (pFieldTop)
			{
				int dOffset(FuncInfo_s::address(pSelf) - FuncInfo_s::address(pFieldTop));
				assert(dOffset > 0);
				//?		dumpTab(nLevel);
				mos << "(+" << std::hex << dOffset << std::dec << ")";
			}
		}


		//dump FieldDecl(pSelf, nullptr, OwnerScope(pSelf));
		if (!m_disp.dumpSym(adcui::SYM_FLDDECL, pSelf))
			drawFieldDefinition(pSelf, nullptr, OwnerScope(pSelf), false);

		if (pSelf->owner()->isVTable())
			dumpComma();
		else
			dumpSemi();

#if(1)
		if (uOverlap > 0)
		{
			PushColor(adcui::COLOR_ERROR);
			mos << " (!) ERROR: overlap " << Int2Str(uOverlap, I2S_HEXC) << " byte(s)";
		}
#endif

	}
	/*if (!pSelf->nameless())
	{
		PushColor(adcui::COLOR_COMMENT);
		mos << "//" << Int2Str(pSelf->Offset(), I2S_HEXC);
	}*/
}

void StrucDumper_t::OutputFieldGap(FieldMapCIt it)
{
	OutputFieldGap0(VALUE(it), CheckUnderlap(it));
}

void StrucDumper_t::OutputFieldGap0(CFieldPtr pSelf, ADDR nGap)
{
	PushColor(adcui::COLOR_UNEXPLORED);
	mos << "unexplored(";

	assert(nGap > 0);
	ADDR va(pSelf->offset() - nGap);
	if (FuncInfo_s::isLocalVar(pSelf))
		mos << Int2Str((int32_t)va, I2S_HEXC | I2S_SIGNED);
	else
		mos << Int2Str(va, I2S_HEXC);
	mos << ", ";
	//mos << Int2Str(pSelf->Offset() - nGap, I2S_HEXC) << ", ";
	mos << Int2Str(nGap, I2S_HEXC);

	mos << ");";// << std::dec;
}

		//the file being dumped belongs to one module;
		//the struc (and given field) belong to another module.
		//have to map given exported field to the imported counterpart; the dumper expects this way;

void StrucDumper_t::DumpDeclaration(bool bImporting)
{
	if (IsProbing())
	{
		probeSrc()->pickTypeDecl(&mrTypeRef);
	}

	if (mrTypeRef.isEnum())
		dumpReserved("enum");
	else if (mrTypeRef.typeUnion())
		dumpReserved("union");
	else
		dumpReserved(mrTypeRef.printType());

	TypePtr iSelf0(SkipProxy(&mrTypeRef));
//CHECKID(iSelf0, 7030)
//STOP
	//?bImporting = isImporting();

	if (iSelf0->isExporting())// || bImporting)
		if (HasMethods(iSelf0) && !iSelf0->typeNamespace())
		{
			mos << " ";
			OutputDeclspecExpImp(bImporting);
		}

#if(!NEW_LOCAL_VARS)
	bool bStrucLocOrTop(mrStruc.typeStrucLoc() || mrStruc.typeUnionLoc());
#else
	bool bStrucLocOrTop(mrStruc.typeFuncDef() != nullptr);//?
#endif

	if (!bStrucLocOrTop)
	{
		mos << " ";
		dumpTypeRef(iSelf0);//!isImporting() ? adcui::SYM_TYPEREF : adcui::SYM_IMPTYPEREF

		// inheritance list

		FieldMapCIt itf(mrStruc.fields().begin());
		while (itf != mrStruc.fields().end())
		{
			CFieldPtr pField(VALUE(itf));
			//unsigned iKind((pField->flags() & FLD_HIER__MASK));
			//if (!iKind)
			if (!(pField->flags() & FLD_HIER_PUBLIC))
				break;
			TypePtr iClass0(pField->type());
			assert(iClass0->typeStruc());
			mos << " : ";
			//		if (iKind == FLD_HIER_PUBLIC)
			dumpReserved("public");
			//		else if (iKind == FLD_HIER_PROTECTED)
			//			dumpPreprocessor("PROTECTED");//not realy 'protected'
			//		else //if (iKind == TYP_HIER_PUBLIC)
			//			dumpPreprocessor("PRIVATE");//not realy 'private'
			mos << " ";
			dumpTypeNameScoped(iClass0, OwnerScope(&mrTypeRef));
			itf++;
		}
	}
}

void StrucDumper_t::CheckEos()
{
	CTypePtr pSelf(SkipProxy(&mrTypeRef));
	CFieldPtr pEos(EosField(pSelf));
	if (pEos)
	{
		NewLine();
		PushColor(adcui::COLOR_COMMENT);
		mos << "//";
		if (!m_disp.dumpSym(adcui::SYM_FLDDECL, pEos))
			drawFieldDefinition(pEos, nullptr, pSelf, false);
	}
}

void StrucDumper_t::Dump(bool bImporting)
{
	CTypePtr iSelf0(&mrTypeRef);
CHECKID(iSelf0, 6291)
STOP

	/*?if (pSelf->typeFuncDef() && pSelf->typ eobj()->isShared())
	{
		ProtoDumper_t proto(*this);
		proto.dumpTypedef(pSelf->dumpFuncdefDeclaration());
		mos << ";";
		return;
	}*/
	//bool bImporting(false);
	//if (mrTypeRef.typeProxy())
	if (mrTypeRef.isShared())
	{
		if (bImporting)
		{
			//bImporting = true;
			if (!m_disp.dumpSym(adcui::SYM_IMPTYPEDEF, iSelf0))
				DumpDeclaration(bImporting);
			iSelf0 = SkipProxy(iSelf0);//may be a proxy or not (for nested types)
		}
		else
		{
			if (!m_disp.dumpSym(adcui::SYM_TYPEDEF, &mrTypeRef))
				DumpDeclaration(bImporting);
			assert(!mrTypeRef.typeProxy());
		}

		if (IsEmptyStruc(iSelf0))
		{
			mos << " {}";
			return;//forward dclaration (no usage)
		}
	}
	else if (mrTypeRef.typeUnion())
		dumpReserved("union");
	else
		dumpReserved(mrTypeRef.printType());

	bool bIsClass(mrStruc.ObjType() == OBJID_TYPE_CLASS);
	bool bUnion(mrTypeRef.typeUnion() != nullptr);

	mos << " {";

	/*if (!mrStruc.xrefs_() && !mrStruc.typeStrucLoc())
	{
		PushColor(adcui::COLOR_COMMENT);
		mos << "//UNREFERENCED!";
	}*/

	IncreaseIndent();

	bool bStrucEnd(false);
	bool bPublicAccess(!bIsClass || bUnion);

	// nested types
	DumpNestedTypes(bImporting, bPublicAccess, bStrucEnd);

	// skip inheritance list
	FieldMapCIt itf(mrStruc.fields().begin());
	while (itf != mrStruc.fields().end())
	{
		CFieldPtr pField(VALUE(itf));
		if (!(pField->flags() & FLD_HIER_PUBLIC))
			break;
		++itf;
	}

	//dump fileds

	DumpStrucFields(itf, bImporting, bPublicAccess, bStrucEnd);

#if(NEW_LOCAL_VARS)
	if (iSelf0->typeFuncDef())
	{
		TypePtr iLocals(iSelf0->typeFuncDef()->locals());
		if (iLocals)
		{
			assert(iLocals->typeStruc()->hasFields());
			StrucDumper_t strucDumper(*this, proot(), indent(), disp(), ctx(), ed(), *iLocals);
			strucDumper.NewLine();
			strucDumper.Dump(false);
			strucDumper.dumpSemi();
		}
	}
#endif

	//dump methods
	if (iSelf0->typeClass())
	{
		//non-virtual functions and static data
		int nMethods(DumpClassMethods(bImporting, bStrucEnd));

		// virtual functions & v-tables
		DumpClassVTables(bImporting, nMethods, bStrucEnd);

	}

	//virtual function table list
	/*for (size_t i(0); i < vptrsOffsets.size(); i++)
	{
		NewLine();
		mos << "[" << i << "] virtual function table slot for vptr at offset " << vptrsOffsets[i];
	}*/

	//if (!bStrucEnd)
		//CheckEos();
	
	DecreaseIndent();
	NewLine();
	m_disp.dumpSym(adcui::SYM_STRUCEND, iSelf0);//put one at the struc end as well
	mos << "}";
	//DumpCloseBrace();

	if (!mrTypeRef.isShared() && !(mrTypeRef.typeStrucLoc() /*|| mrTypeRef.typeUnionLoc()*/))
	{
		dumpSpace();
		CFieldPtr pField(mrTypeRef.parentField());
		CTypePtr pScope(&mrTypeRef);
		if (!m_disp.dumpSym(adcui::SYM_FLDDECL, pField))
			drawFieldDefinition(pField, nullptr, pScope, false);
	}

	/*if (0)
	if (!bStrucLocOrTop)
	{
		PushColor(adcui::COLOR_COMMENT);
		int nSize = iSelf0->size();
		mos << "//SIZE:" << nSize;// << pStruc->m_cName;
		if (nSize > 9)
			mos << "(" << Int2Str(nSize, I2S_HEXC) << ")";
	}*/
}

void StrucDumper_t::DumpNestedTypes(bool bImporting, bool& bPublicAccess, bool &bStrucEnd)
{
	if (!mrStruc.typeMgr())
		return;

	CTypePtr iSelf0(SkipProxy(&mrTypeRef));
	bool bIsClass(mrStruc.ObjType() == OBJID_TYPE_CLASS);//no namespaces
	bool bUnion(mrTypeRef.typeUnion() != nullptr);
	int nTypes(0);
	for (TypesMgr_t::OrderIterator i(*mrStruc.typeMgr()); i; i++)
	{
		CTypePtr iStruc(*i);
//CHECKID(iStruc, 41972)
//STOP
		assert(!iStruc->typeProxy());
		Struc_t* pStruc(iStruc->typeStruc());
		if (pStruc)
		{
			if (nTypes == 0 && bIsClass && !bUnion)
			{
				NewLine(-1);
				if (!bStrucEnd)
				{
					m_disp.dumpSym(adcui::SYM_STRUCEND, iSelf0);
					bStrucEnd = true;
				}
				dumpReserved("public");
				mos << ":";
				//dumpComment("types", true);
				bPublicAccess = true;
			}
#if(NO_SUBDUMPS)
			bool bNewDump(false);
#else
			bool bNewDump(openScope(iStruc));
#endif
			//if (!bNewDump)
				//NewLine();
			StrucDumper_t strucDumper(*this, proot(), indent(), disp(), ctx(), ed(), *iStruc);
			//				strucDumper.setImporting(isImporting());
			strucDumper.NewLine();
			strucDumper.Dump(bImporting);
			strucDumper.dumpSemi();
			if (bNewDump)
				m_disp.closeScope(false);

			nTypes++;
		}
		else
		{
			Typedef_t* pTypedef(iStruc->typeTypedef());
			if (pTypedef)
			{
				NewLine();
				TProtoDumper<ProtoImpl_t<StrucDumper_t>> proto(*this);
				proto.mbTabSep = true;
				proto.dumpTypedefDeclaration(iStruc, DcInfo_t::OwnerScope(iStruc));
				mos << ";";
			}
		}
	}
}

void StrucDumper_t::DumpStrucFields(FieldMapCIt it, bool bImporting, bool bPublicAccess, bool &bStrucEnd)
{
	CTypePtr iSelf0(SkipProxy(&mrTypeRef));
	bool bIsClass(mrStruc.ObjType() == OBJID_TYPE_CLASS);//no namespaces
	bool bUnion(mrTypeRef.typeUnion() != nullptr);//whole struc is union
	int iUnion(0);
	int nFields(0);
	const FieldMap& m(mrStruc.fields());
	FieldMapCIt itpr(m.end());
	for (; it != m.end(); itpr = it++, nFields++)
	{
		CFieldPtr pField(VALUE(it));
//CHECKID(pField, 17632)
//STOP
		FieldMapCIt itnx(it);
		++itnx;
		CFieldPtr pFieldNx(itnx != m.end() ? VALUE(itnx) : nullptr);
		if (!bUnion)
		{
			if (pFieldNx && pField->_key() == pFieldNx->_key())
				iUnion++;
			else if (iUnion > 0)
				iUnion = -iUnion;//mark as closed
		}

		if (bIsClass && !bUnion)
		{
			if (nFields == 0 || !bPublicAccess)// && bIsClass)// && mrStruc.hasFields())
			{
				if (pField->type() && pField->type()->typeVPtr())
				{
					if (bPublicAccess)
					{
						NewLine(-1);
						if (!bStrucEnd)
						{
							m_disp.dumpSym(adcui::SYM_STRUCEND, iSelf0);
							bStrucEnd = true;
						}
						dumpReserved("private");
						mos << ":";
						bPublicAccess = false;
					}
				}
				else
				{
					NewLine(-1);
					dumpReserved("public");
					mos << ":";
					bPublicAccess = true;
				}
				//dumpComment("fields", true);
			}
		}

		if (!mrTypeRef.isEnum() && !bUnion && !mrStruc.typeFuncDef())
		{
			ADDR nGap = CheckUnderlap(it);
			if (nGap > 0)
			{
				NewLine();
				if (!m_disp.dumpSym(adcui::SYM_GAP, pField))
					DumpFieldGap(pField);
			}
		}

		if (iUnion == 1)
		{
			NewLine();
			dumpReserved("union");
			mos << " {";
			IncreaseIndent();
		}

		/*if (IsEosField(pField))
		{
			assert(++it == mrStruc.fields().end());
			break;
		}*/
		if (!pField->type() || pField->type()->isShared())
		{
			NewLine();
			if (IsEosField(pField))
			{
				PushColor(adcui::COLOR_COMMENT);
				mos << "//";
			}
			//if (!DumpSym(adcui::SYM_FLDDECL, pSelf))
			DumpField(0, it, itpr);
		}
		else if (pField->type()->typeBitset())
		{
			if (bUnion || iUnion != 0)
			{
				NewLine();
				dumpReserved("struct");
				mos << " {";
				IncreaseIndent();
			}
			FieldMap& m2(pField->type()->typeBitset()->fields());
			for (FieldMapCIt i = m2.begin(), E = m2.end(); i != E; ++i)
			{
				CFieldPtr pField2(VALUE(i));
				assert(!pField2->owner()->parentField() || pField2->owner()->parentField() == pField);
				NewLine();
				//dump FieldDecl(pField2, nullptr, nullptr);
				if (!m_disp.dumpSym(adcui::SYM_FLDDECL, pField2))
					drawFieldDefinition(pField2, nullptr, nullptr, false);
				dumpSemi();
			}
			if (bUnion || iUnion != 0)
			{
				DecreaseIndent();
				NewLine();
				mos << "}";
				if (pField->name())
				{
					dumpSpace();
					drawFieldName(pField, nullptr, false);
				}
				dumpSemi();
			}
		}
		else
		{
			NewLine();
			assert(pField->type()->typeStruc());
			StrucDumper_t nested(*this, proot(), indent(), disp(), ctx(), ed(), *pField->type());
			//NewLine();
			//strucDumper.dumpTab(indent());
			nested.Dump(bImporting);
			nested.dumpSemi();
			//mos << "???";
		}

		if (iUnion < 0)
		{
			DecreaseIndent();
			NewLine();
			mos << "}";
			dumpSemi();
			iUnion = 0;
		}
	}
}

int StrucDumper_t::DumpClassMethods(bool bImporting, bool &bStrucEnd)
{
	CTypePtr iSelf0(SkipProxy(&mrTypeRef));
	bool bIsClass(mrStruc.ObjType() == OBJID_TYPE_CLASS);//no namespaces
	bool bUnion(mrTypeRef.typeUnion() != nullptr);
	int nMethods(0);
	const ClassMemberList_t& m(iSelf0->typeClass()->methods());
	for (ClassMemberListCIt it(m.begin()); it != m.end(); ++it, nMethods++)
	{
		CGlobPtr pGlob(*it);

		assert(ModuleOf(iSelf0) == ModuleOfEx(pGlob));

		CFieldPtr pField(DockField(pGlob));
		assert(pField);
CHECKID(pField, 1117)
STOP

		if (bIsClass && nMethods == 0 && !bUnion)//first
		{
			NewLine(-1);
			if (!bStrucEnd)
			{
				m_disp.dumpSym(adcui::SYM_STRUCEND, iSelf0);
				bStrucEnd = true;
			}
			dumpReserved("public");
			mos << ":";
			//dumpComment("methods", 1);
		}

		if (pGlob->func())
		{
			NewLine();
			//DumpClassMethodDecl(iFuncDef, pField);
			if (bImporting)
			{
				if (!m_disp.dumpSym(adcui::SYM_IMPCLSGLB, pGlob))
					drawImpClsGlobDecl(pGlob, nullptr, nullptr);
			}
			else
			{
				DumpFunctionDeclaration(pGlob, FUNCDUMP_DECL_FLAGS, nullptr, &mrTypeRef);
			}
			dumpSemi();
		}
		else
		{
			OFF_t oData(OFF_NULL);
			CTypePtr iStruc(pField->isTypeStruc());
			if (iStruc && !iStruc->isShared() && GetRawOffset(pField, oData))
			{
				assert(!iStruc->typeProxy());

#if(NO_SUBDUMPS)
				bool bNewDump(false);
#else
				bool bNewDump(openScope(iStruc));
#endif
				StrucDumper_t strucDumper(*this, proot(), indent(), disp(), ctx(), ed(), *iStruc);
				strucDumper.NewLine();
#if(0)
				strucDumper.Dump(bImporting);
#else
				DataStream_t aRaw(GetDataSource()->pvt(), oData);
				strucDumper.DumpRaw(aRaw, bImporting);
				//strucDumper.DumpRaw(bImporting);
#endif
				strucDumper.dumpSemi();
				//strucDumper.DumpVTableFor(pField);
				if (bNewDump)
					m_disp.closeScope(false);
			}
			else
			{
				NewLine();
				if (bImporting)
				{
					if (!m_disp.dumpSym(adcui::SYM_IMPCLSGLB, pGlob))
						drawImpClsGlobDecl(pGlob, nullptr, nullptr);
				}
				else
				{
					if (!m_disp.dumpSym(adcui::SYM_GLBDECL, pGlob))
						drawGlobDefinition(pGlob, nullptr, &mrTypeRef, false);
				}
				dumpSemi();
			}
		}
	}
	return nMethods;
}

void StrucDumper_t::DumpClassVTables(bool bImporting, int& nMethods, bool &bStrucEnd)
{
	CTypePtr iSelf0(SkipProxy(&mrTypeRef));
	const ClassVTables_t& vtables(ClassInfo_t::VTablesOf(iSelf0));
	for (ClassVTables_t::const_iterator i(vtables.begin()); i != vtables.end(); ++i)
	{
		if (nMethods == 0)//first
		{
			NewLine(-1);
			if (!bStrucEnd)
			{
				m_disp.dumpSym(adcui::SYM_STRUCEND, iSelf0);
				bStrucEnd = true;
			}
			dumpReserved("public");
			mos << ":";
			nMethods = -1;//no longer needed
		}

		NewLine();
		int vptr_off(i->first);//vptr offset
		const ClassVTable_t& vtable(i->second);
		
		CGlobPtr pVTGlob(vtable.self);
		dumpVTableDecl(pVTGlob, vptr_off, bImporting);
		IncreaseIndent();

		bool bDone(false);
		if (pVTGlob && m_disp.IsOutputDead())
		{
			CFieldPtr pVTField(DockField(pVTGlob));
			CTypePtr pModule2(ModuleOf(pVTField));
			ModuleInfo_t MI2(*this, *pModule2);
			TypePtr pSeg(ProjectInfo_t::OwnerSeg(pVTField->owner()));
			if (MI2.CheckDataAtVA(pSeg, pVTField->_key()))
			{
				DataSubSource_t data(MI2.GetDataSource()->pvt(), pSeg->typeSeg()->rawBlock());
				DataStream_t aRaw(data, pVTField->offset());//oData);
				dumpRawDataVTable(vtable, aRaw, pSeg, bImporting);
				bDone = true;
			}
		}
		if (!bDone)
		{
			const ClassVirtMembers_t& vmemb(vtable.entries);
			for (ClassVirtMembers_t::const_iterator j(vmemb.begin()); j != vmemb.end(); ++j)
			{
				NewLine();
				int vmeth_off(j->first);//method offset in vtable
				DumpVirtualEntry(j->second, vmeth_off, bImporting);
				dumpSemi();
			}
		}

		DecreaseIndent();
		NewLine();
		dumpPreprocessor("VTABLE_END");
	}
}

void StrucDumper_t::DumpRaw(bool bImporting)
{
	CTypePtr iSelf0(&mrTypeRef);
	dumpReserved(mrTypeRef.printType());
	dumpSpace();
	CFieldPtr pField(mrTypeRef.parentField());
	CGlobPtr pGlob(GlobObj(pField));
	CTypePtr pScope(OwnerScope(pField));
	DataSourceNull_t nodata;
	DataStream_t aRaw(nodata);
	dumpRawData(pField->type(), aRaw, OwnerScope(pField), pField->attrib(), nullptr, 0);
	if (!mrTypeRef.isShared() && !(mrTypeRef.typeStrucLoc() /*|| mrTypeRef.typeUnionLoc()*/))
	{
		dumpSpace();
		if (bImporting)
		{
			if (!m_disp.dumpSym(adcui::SYM_IMPCLSGLB, pGlob))
				drawImpClsGlobDecl(pGlob, nullptr, pScope);
		}
		else
		{
			if (!m_disp.dumpSym(adcui::SYM_GLBDECL, pGlob))
				drawGlobDefinition(pGlob, nullptr, pScope, false);
		}
	}

}

void StrucDumper_t::DumpRaw(DataStream_t &aRaw, bool bImporting)
{
	CTypePtr iSelf0(&mrTypeRef);

	CFieldPtr pField(mrTypeRef.parentField());
	CGlobPtr pGlob(GlobObj(pField));
	CTypePtr pSeg(OwnerSeg(pField->owner()));
	CTypePtr pScope(pGlob ? OwnerScope(pGlob) : OwnerScope(pField));

	const FileDumper_t &fileDumper(root());

	if (IsStaticMember(pGlob) && fileDumper.IsHeader())
	{
		dumpReserved("static");
		dumpSpace();
	}

	if (IsConst(pField))
	{
		dumpReserved("const");
		dumpSpace();
	}

	dumpReserved(mrTypeRef.printType());

	dumpSpace();

CHECKID(pField, 5210)
STOP

	dumpRawData(pField->type(), aRaw, pScope, pField->attrib(), pSeg, 0);

	if (!mrTypeRef.isShared() && !(mrTypeRef.typeStrucLoc() /*|| mrTypeRef.typeUnionLoc()*/))
	{
		dumpSpace();
		if (bImporting)
		{
			if (!m_disp.dumpSym(adcui::SYM_IMPCLSGLB, pGlob))
				drawImpClsGlobDecl(pGlob, nullptr, pScope);
		}
		else
		{
			if (!m_disp.dumpSym(adcui::SYM_FLDDECL0, pField))//ref?
				drawFieldName(pField, nullptr, false, GlobalColor2(pGlob));
		}
	}

}

void FileDumper_t::DumpFuncDef(CGlobPtr iFuncDef)
{
	if (ProtoInfo_t::IsStub(iFuncDef) && !m_disp.isUnfoldMode())
	{
		ProtoInfo_t TI(*this, (GlobPtr)iFuncDef);
		ProtoDumper_t PD(TI, proot(), indent(), disp(), ctx(), ed());
		NewLine();
		PD.DumpFunctionDeclaration(iFuncDef, FUNCDUMP_IMPL_FLAGS, nullptr, PrimeSeg(), true);
		PD.dumpStr("{");
		PD.dumpStr("}");
		PD.DumpFunctionClosingInfo(iFuncDef);
		return;
	}
	FuncInfo_t funcInfo(DcRef(), *(GlobPtr)iFuncDef);
	FuncDumper_t funcDumper(funcInfo, proot(), indent(), disp(), ctx(), ed());

	/*if (m_disp.isUnfoldMode())
	{
		funcDumper.DumpFuncUnfold();
	}
	/ *else if (iFuncDef->typeFuncDef()->isStub())
	{
		funcDumper.DumpFunctionStub(iFuncDef);
	}* /
	else*/
	{
#if(0)
		funcDumper.NewLine();
		funcDumper.dumpFuncBannerComment(true);
#endif
		funcDumper.DumpFunc();
	}
}

void FuncDumper_t::dumpFuncBannerComment(bool bIlfakLike)
{
	CGlobPtr pSelf = FuncDefPtr();
	PushColor(adcui::COLOR_COMMENT);
	mos << "//";

	//const FuncDef_t &rFuncDef(*pSelf->typeFuncDef());

	if (bIlfakLike)
	{
		ADDR addr(FileInfo_t::DockAddr(pSelf));
		MyStringf s("----- (%08X) --------------------------------------------------------", (unsigned)addr);
		mos << s;
	}
	else
	{
		MyString s(GlobNameFull(pSelf, E_PRETTY, CHOP_SYMB).join());
		const char * p = s.c_str();
		while (*p)
		{
			mos << " " << *p;
			p++;
		}
	}
}

void FileDumper_t::DumpInclude(CFolderPtr pFile)
{
	NewLine();
	MyString s(pFile->name());
	dumpReserved("#include");
	mos << " ";
	PushColor(adcui::COLOR_STRING);
	mos << "\"" << s << HEADER_EXT << "\"";
	PopColor();
	//NewLine();
}

void FileDumper_t::DumpHeaderGuardOpen(CFolderPtr pFile)
{
	NewLine();
	dumpReserved("#ifndef");
	mos << " ";
	File_OutIDStr(pFile, true);
	//NewLine();
}

void FileDumper_t::DumpHeaderGuard(CFolderPtr pFile)
{
	NewLine();
	dumpReserved("#define");
	mos << " ";
	File_OutIDStr(pFile, true);
	//NewLine();
}

void FileDumper_t::DumpHeaderGuardClose(CFolderPtr pFile)
{
	NewLine();
	dumpReserved("#endif");
	PushColor(adcui::COLOR_COMMENT);
	mos << "//";
	File_OutIDStr(pFile, true);
	PopColor();
	//NewLine();
}

void FileDumper_t::Probe(ProbeExIn_t& aProbe)//size_t modelId, long x0, long y0)
{
	assert(!mpEd);

	long x(aProbe.x());
	long line(aProbe.line());

	if (x < 0 || line < 0)
		return;

	DumpContext_t dctx;// (mpProbeSrc->dumpScope());
	const char* strLine(dynamic_cast<DisplayUI_t&>(disp()).GetDataAtLine(line, &dctx));
	aProbe.pickScope(dctx.mpCplx);
	if (!strLine)
		return;

	//assert(mpProbeSrc->empty());
	/*if (dctx.mpCplx)
	{
		CTypeBasePtr pScope(dctx.mpCplx);
		if (pScope->typeFuncDef())
			pScope = ((CGlobPtr)pScope)->dockField()->owner();//proc
		mpProbeSrc->push_back(Frame_t((TypePtr)pScope, (ADDR)-1, nullptr));
	}*/

	int len;
	//mpProbeSrc->setProbing(true);
	MyString s(ExpandLinePlain(line, strLine, len, dctx));//seemingly dummy call, but it is going to initialize hit object in pDI!
	//mpProbeSrc->setProbing(false);

	aProbe.setFolder(OwnerFolder());
	aProbe.setupLocality(disp().IsHeader());
}




