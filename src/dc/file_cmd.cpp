#include "file_cmd.h"
#include "info_func.h"
#include "files_ex.h"
#include "ana_pcode.h"
#include "ana_expr.h"
#include "ana_switch.h"
#include "ana_branch.h"
#include "ana_local.h"
#include "ana_main.h"
#include "path.h"
#include "expr_term.h"
#include "dump.h"
#include "proj_ex.h"
#include "main_ex.h"
#include "clean_ex.h"
#include "ui_main_ex.h"
#include "sym_parse.h"
#include "dump_file.h"
#include "arglist.h"
#include "expr_simpl.h"
#include "info_class.h"
#include "expr_dump.h"

FileInfoCmdServer_t::FileInfoCmdServer_t(CMDServerCommandMap &rcm, const FileInfo_t &rDcInfo)
: FileInfo_t(rDcInfo),
CMDServer_t(rcm)
{
}

#define MYCOMMAND_IMPL(name) \
	int FileInfoCmdServer_t::COMMAND_##name(CMDServer_t *pSelf, Cmd_t &args){ return static_cast<FileInfoCmdServer_t*>(pSelf)->OnCommand_##name(args); } \
	int FileInfoCmdServer_t::OnCommand_##name(Cmd_t &args)

/*MYCOMMAND_IMPL(setcurop)
{
DECLARE_DC_CONTEXT(ctx);
assert(mrDC.hasContext());

projx().setLocus(pctx);

#if(0)
cout << "setcurop(";
mcx.print(cout);
cout << ")\n";
#endif
return 1;
}*/

MYCOMMAND_IMPL(toggle_root)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.opLine() || !ctx.scopeFunc())
		return 0;
	FuncInfo_t f0(mrDC, *ctx.scopeFunc());// , * ctx.fileDef());
	PathOpTracer_t tr;
	AnlzRoots_t an(f0, tr, ctx.opLine(), false, true);
	if (args.Find("-force") > 0)
		an.enforce();
	int iRet;
	{//RAII-block
		WriteLocker lock;
		iRet = an.ToggleRoot();
	}
	if (iRet)
		redump(ctx, REDUMP_SRC);
	return 1;
}

MYCOMMAND_IMPL(toggle_if)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.opLine() || !ctx.scopeFunc())
		return 0;
	FuncInfo_t f0(mrDC, *ctx.scopeFunc());// , * ctx.fileDef());
	PathPtr pPath(f0.PathOf(ctx.opLine()));
	PathOpTracer_t tr;
	BranchTracer_t an(f0, tr);
	int iRet;
	{//RAII-block
		WriteLocker lock;
		iRet = an.ToggleBlockIf(pPath);
	}
	if (iRet)
	{
		redump(ctx, REDUMP_SRC);
		gui().GuiOnCurFuncModified();
	}
	return 1;
}

MYCOMMAND_IMPL(toggle_else)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.opLine() || !ctx.scopeFunc())
		return 0;
	int iRet;
	{//RAII-block
		WriteLocker lock;
		FuncInfo_t f0(mrDC, *ctx.scopeFunc());// , * ctx.fileDef());
		PathPtr pPath(f0.PathOf(ctx.opLine()));
		PathOpTracer_t tr;
		BranchTracer_t an(f0, tr);
		iRet = an.ToggleBlockElse(pPath, true);
	}
	if (iRet)
	{
		redump(ctx, REDUMP_SRC);
		gui().GuiOnCurFuncModified();
	}
	return 1;
}

// Command: toggle_switch
// Alias: 
// Desc: Toggle `switch` statement ON/OFF;
// Synopsis: toggle_switch -v
//		-v	: verbose mode (print progress/errors)
MYCOMMAND_IMPL(toggle_switch)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.opLine() || !ctx.scopeFunc())
		return 0;
	bool bVerbose(args.RemoveOpt("-v"));
	FuncInfo_t f0(mrDC, *ctx.scopeFunc());// , * ctx.fileDef());
	PathPtr pPath(f0.PathOf(ctx.opLine()));
	PathOpTracer_t tr;
	SwitchTracer_t an(f0, tr);
	int iRet;
	{//RAII-block
		WriteLocker lock;
		iRet = an.ToggleSwitch(pPath, bVerbose);
	}
	if (iRet)
	{
		redump(ctx, REDUMP_SRC);//?
		gui().GuiOnCurFuncModified();
	}
	return iRet;
}

MYCOMMAND_IMPL(toggle_while)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.opLine() || !ctx.scopeFunc())
		return 0;
	FuncInfo_t f0(mrDC, *ctx.scopeFunc());// , * ctx.fileDef());
	PathPtr pPath(f0.PathOf(ctx.opLine()));
	PathOpTracer_t tr;
	BranchTracer_t anlz(f0, tr);
	if (FuncInfo_t::PathType(pPath) == BLK_JMP)
	if (!anlz.ToggleEndlessLoop(pPath))
		return 1;
	int iRet;
	{//RAII-block
		WriteLocker lock;
		iRet = anlz.ToggleWhile(pPath);
	}
	if (iRet)
	{
		redump(ctx, REDUMP_SRC);
		gui().GuiOnCurFuncModified();
	}
	return 1;
}

MYCOMMAND_IMPL(toggle_for)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.opLine() || !ctx.scopeFunc())
		return 0;
	FuncInfo_t f0(mrDC, *ctx.scopeFunc());// , * ctx.fileDef());
	PathPtr pPath(f0.PathOf(ctx.opLine()));
	PathOpTracer_t tr;
	BranchTracer_t anlz(f0, tr);
	//if (PathType(pPath) == BLK_JMP)
	//if (!anlz.ToggleEndlessLoop(pPath))
	//return 1;
	int iRet;
	{//RAII-block
		WriteLocker lock;
		iRet = anlz.ToggleForLoop(pPath);
		if (iRet)
		{
			redump(ctx, REDUMP_SRC);
			gui().GuiOnCurFuncModified();
		}
	}
	return 1;
}

MYCOMMAND_IMPL(do_logic)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.opLine() || !ctx.scopeFunc())
		return 0;
	FuncInfo_t f0(mrDC, *ctx.scopeFunc());// , * ctx.fileDef());
	PathPtr pPath(f0.PathOf(ctx.opLine()));
	PathOpTracer_t tr;
	BranchTracer_t anlz(f0, tr);
	int iRet;
	{//RAII-block
		WriteLocker lock;
		iRet = anlz.TurnLogics_On(pPath);
	}
	if (iRet)
	{
		redump(ctx, REDUMP_SRC);
		gui().GuiOnCurFuncModified();
	}
	return 1;
}

MYCOMMAND_IMPL(undo_logic)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.opLine() || !ctx.scopeFunc())
		return 0;
	FuncInfo_t f0(mrDC, *ctx.scopeFunc());// , * ctx.fileDef());
	PathPtr pPath(f0.PathOf(ctx.opLine()));
	PathOpTracer_t tr;
	BranchTracer_t anlz(f0, tr);
	int iRet;
	{//RAII-block
		WriteLocker lock;
		iRet = anlz.TurnLogics_Off(pPath);
	}
	if (iRet)
	{
		redump(ctx, REDUMP_SRC);
		gui().GuiOnCurFuncModified();
	}
	return 1;
}

MYCOMMAND_IMPL(bind)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.opLine() || !ctx.scopeFunc())
	{
		PrintError() << "No context (" << args[0] << ")" << std::endl;
		return 0;
	}
	FuncInfo_t FI(mrDC, *ctx.scopeFunc());// , * ctx.fileDef());
	int iRet;
	{//RAII-block
		WriteLocker lock;
		iRet = FI.Bind(ctx.opLine());
		if (iRet)
		{
			redump(ctx, REDUMP_SRC);
			gui().GuiOnCurFuncModified();
		}
	}
	return iRet;
}

// Command: unbind
MYCOMMAND_IMPL(unbind)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.opLine() || !ctx.scopeFunc())
		return 0;
	FuncInfo_t FI(mrDC, *ctx.scopeFunc());// , * ctx.fileDef());
	int iRet;
	{//RAII-block
		WriteLocker lock;
		iRet = FI.Unbind(ctx.opLine());
		if (iRet)
		{
			redump(ctx, REDUMP_SRC);
			gui().GuiOnCurFuncModified();
		}
	}
	return iRet;
}

// Command: acquire
// Alias: acq
// Desc: Acquire (bring in) an object into virtual (source) file
MYCOMMAND_IMPL(acquire)//acq
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.folder() || !ctx.folder()->fileDef())
	{
		PrintError() << "No context file for command `" << args[0] << "`" << std::endl;
		return 0;
	}
	FieldPtr pField(nullptr);
	MyString sAt(args.FindOptOrPfx("-@"));
	if (!sAt.empty())
	{
		ADDR va(strtoul(sAt.c_str(), nullptr, 16));
		TypePtr iSeg(FindSegAt(PrimeSeg(), va, PrimeSeg()->typeSeg()->affinity()));
		if (iSeg)
			pField = Field(iSeg, va, nullptr, FieldIt_Exact);//exact
		if (!pField)
		{
			PrintError() << "No object at VA=" << VA2STR2(va) << " in current file" << std::endl;
			return 0;
		}
	}
	else// if (args.size() < 2)//from context
	{
		if (ctx.obj())
			pField = ctx.obj()->objField();
	}
	if (!pField || !IsGlobal(pField))
	{
		fprintf(STDERR, "No object to acquire\n");
		return 0;
	}
	//bool bConst(args.RemoveOpt("-cnt"));
	/*if (pField->isCloneMaster())
	{
		pField = FindCloneOf(pField, true);//prefer a stub
		if (!pField)
		{
			PrintError() << "Could not resolve on cloned field to acquire VA=" << VA2STR2(ctx.locus().va()) << std::endl;
			return 0;
		}
	}*/

	if (GlobObj(pField) && GlobObj(pField)->folder() == ctx.folder())
		return -1;

	FieldExPtr pFieldx(AddGlobToFile2(pField, ctx.folder()));
	GlobPtr pGlob(GlobObj(pFieldx));
	std::cout << "Object '"
		<< GlobNameFull(pGlob, E_PRETTY, CHOP_SYMB) << "' moved to file '"
		<< FilesMgr0_t::relPath(ctx.folder()) << "'"
		<< std::endl;
	redump(ctx, REDUMP_ALL);
	gui().GuiOnCurFuncModified();
	return 1;
}

// Command: acquire_constant
// Alias: acqcnt
// Desc: Acquire a constant object (in $constants file)
/*MYCOMMAND_IMPL(acquire_constant)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.opLine() || !ctx.scopeFunc())
		return -1;
	Obj_t* pObj(mrProject.GetContextObj());
	if (!pObj)
		return -2;
	FieldPtr pField(pObj->objField());
	if (!pField || !pField->isGlobal())
		return -3;
	GlobPtr pGlob(GlobObj(pField));
	FolderPtr pFolder(AssureSpecialFile(FTYP_CONST));
	AssureFileDef(pFolder, false);
	if (pGlob && pGlob->folder() == pFolder)
		return 0;
	AddGlobToFile(pField, pFolder);
	redump(ctx, REDUMP_SRC);
	gui().GuiOnCurFuncModified();
	return 1;
}*/

MYCOMMAND_IMPL(setnam)
{
	DECLARE_DC_CONTEXT(ctx);
	Folder_t *pFolder(ctx.folder());
	if (!pFolder)
		return -1;//not in this context

	Obj_t* pObj(ctx.obj());//GetContextObj());
	if (!pObj)
		return -1;

	FieldPtr pField(pObj->objField());
	TypePtr pType(pObj->objTypeGlob());
	const char *pcName(args.size() > 1 ? args[1].c_str() : nullptr);

	bool bSet(false);
	GlobPtr iCFunc(ctx.scopeFunc());
	//if (!iCFunc && ctx.scope())
		//iCFunc = IsLocalsTop(ctx.scope());

	if (iCFunc)
	{
		FuncInfo_t FI(mrDC, *iCFunc);/// , * ctx.fileDef());
		if (pField)
		{
			if (FI.memMgrOf(pField) != &memMgrGlob())
			if (!(bSet = FI.SetFieldName(pField, pcName)))
				return -2;
		}
		else
		{
			if (FI.memMgrOf(pType) != &memMgrGlob())
			if (!(bSet = FI.SetTypeName(pType, pcName)))
				return -2;
		}
	}

	if (!bSet)
	{
		DcInfo_t DI(mrDC, memMgrGlob());
		if (pField)
		{
			if (pField->isHardNamed())
			{
				assert(!pField->nameless());
				PrintError() << "Field '" << FieldName(pField, CHOP_SYMB) << "' cannot be re-named" << std::endl;
				return 0;
			}
			if (!(bSet = DI.SetFieldName(pField, pcName)))
				return -2;
		}
		else
		{
			if (ModuleOf(pType) != ModulePtr())
			{
				if (!DI.FindProxyOf(pType))//acquire type
				{
					WriteLocker lock;
					TypePtr pTypePxy(DI.MakeProxyTypeTo(pType));
					if (!pTypePxy)
						return -3;//?
					if (!(bSet = DI.RenameProxyType(pTypePxy, pcName)))
						return -4;
					if (AcquireProxyType(pTypePxy))
					{
						PrintInfo() << "Type '" << TypeNameFull(pType, E_PRETTY, CHOP_SYMB) << "' acquired in file " << FilesMgr0_t::relPath(OwnerFolder()) << std::endl;
						redump(ctx, REDUMP_H);
					}
				}
			}
			if (!bSet && !(bSet = DI.SetTypeName(pType, pcName)))
				return -2;
		}
	}

	//gui().GuiOnNameChanged();
	mrProject.markDirty(DIRTY_NAMES);
	return 1;
}


#define OBJINFO(o) std::cout << "sizeof("#o")=" << sizeof(o) << std::endl

MYCOMMAND_IMPL(meminfo)
{
	if (args.Find("-d"))
	{
		OBJINFO(Op_t);
#if(NEW_OP_PTR)
		std::cout << "\top chunk size:" << MemOpPool::entrySize() << std::endl;
#endif
		OBJINFO(Path_t);
#if(NEW_PATH_PTR)
		std::cout << "\tpath chunk size:" << MemPathPool::entrySize() << std::endl;
#endif
		OBJINFO(Field_t);
		OBJINFO(TypeObj_t);
	}
	else
	{
		DECLARE_DC_CONTEXT(ctx);
		if (ctx.fileDef())
			ctx.fileDef()->memMgr().print_stats(std::cout);
		else
			memMgr().print_stats(std::cout);
	}
	return 1;
}


MYCOMMAND_IMPL(del)
{
	DECLARE_DC_CONTEXT(ctx);
	Folder_t *pFolder(ctx.folder());
	if (!pFolder)
		return -1;//not in this context

	if (TopFolder(*pFolder) == pFolder)
		//if (!dynamic_cast<ProbeEx_t *>(args.context()))//pick on bin view?
	{
		assert(0);
		LocusFromStr(AtStr(args), ctx.locus());

		if (!adjustPick(ctx.locus()))
			return 0;

		Frame_t &aTop(ctx.locus().back());

		//check if the object is in some Cpp file
		GlobPtr pGlob(IsGlobal(aTop.field()) ? GlobObj(aTop.field()) : nullptr);
		FolderPtr pFolder(pGlob ? pGlob->folder() : nullptr);
		if (!pFolder)
			return -1;

		{//RAII-block
			WriteLocker lock;
			//delete in stubs
			/*StubInfo_t SI(*this);
			StubMgr_t &stubs(dc().stubs());
			StubIt stubIt(SI.FindStubIt(aTop.field()->address()));
			if (stubIt != stubs.end())
			{
				Stub_t &aStub(stubIt->second);
				MyString s(SI.ValueFromStub(aStub));
				aStub.setValue(s);//and clear the field ptr
				aStub.setModified(true);
			}*/
			//delete in file
			FileInfo_t FL(mrDC, *FILEDEF(pFolder));
			
			bool iRet(FL.RecallGlob(GlobObj(aTop.field())));
			assert(iRet);
			//delete in globals tree
			if (!DeleteField(ctx.locus()))
				return 0;

			//update gui
			//?ProbeEx_t ctx;
			redump(ctx, REDUMP_ALL);//all
			//gui().GuiOnProjectModified();
			mrProject.markDirty(DIRTY_GLOBALS);
			return 1;
		}
	}

	int iRet(0);
	TypePtr pProc(ctx.locus().proc());
	if (pProc && ctx.locality().scoping == adcui::LocusId_FUNC_HEADER)
	{
		//RAII-block

		WriteLocker lock;
		FieldPtr pField(pProc->parentField());
		GlobPtr pCFunc(GlobFuncObj(pField));
		if (!pCFunc || ProtoInfo_t::IsStub(pCFunc))
		{
			FileInfo_t FIL(mrDC, *FILEDEF(pFolder));
			if (FIL.RecallGlob(pCFunc))
			{
				redump(ctx, REDUMP_ALL);
				//gui().GuiOnProjectModified();//binary view must update too (function's status)
				mrProject.markDirty(DIRTY_GLOBALS);
				iRet = 1;
			}
		}
		else//make a stub
		{
			FileInfo_t FIL(mrDC, *FILEDEF(pFolder));
			FuncInfo_t FI(FIL, *pCFunc);
			FuncCleaner_t FD(FI);
			FD.DetachArgsRefs();//prevent args to be deleted
			FD.DestroyBody();
			FD.Cleanup();
			FD.CleanupFinal();
			redump(ctx, REDUMP_SRC);
			iRet = 1;
		}
	}
	else
	{
		FieldPtr pField(ctx.pickedFieldDecl());
		if (pField)
		{
			Locus_t aLoc2;
			aLoc2.add(pField->owner(), pField->_key(), pField);//fix later

			//RAII-block
			WriteLocker lock;
			FileInfo_t FIL(mrDC, *pFolder->fileDef(), memMgrGlob());
			if (FIL.DeleteFieldGlobal(aLoc2, ctx.scope()))
			{
				redump(ctx, REDUMP_ALL);//if a struc has been deleted - it was unreferenced by any source
				iRet = 1;
			}
			else if (FuncInfo_s::IsLocalArg(pField))
			{
				assert(0);
			}
			else if (FIL.DeleteField(aLoc2))//struc member
			{
				redump(aLoc2.struc());
				iRet = 1;
			}
		}
		else
		{
			ObjPtr pObj(ctx.obj());
			if (pObj)
			{
				TypePtr pType(pObj->objType());
				if (pType)
				{
					assert(pType->typeStruc() || pType->typeTypedef() || pType->typeProxy());
					pFolder = FolderOf(pType);
					if (pFolder)//otherwise should be deleted from bin view
					{//RAII-block
						WriteLocker lock;
						assert(pType->refsNum() > 0);//at least typemgr keeps 1
						TypePtr pModule(ModuleOf(pType));
						DcInfo_t DI(*DCREF(pModule));
						if (pType->refsNum() == 1)
						{
							DcCleaner_t<> DC(DI);
							pType->addRef();//types mgr thinks whoever keeps a ref to the type, wants off with it
							if (DC.ReleaseTypeRef0(pType, true))
							{
								//FileInfo_t GI(DI, *pFolder->fileDef());
								//GI.TakeType(pType);
								DC.DestroyTypeRef(pType);
								//DI.memMgr().Delete(pType);
								ctx.setScope(nullptr);
								ctx.pickObj(nullptr);
								redump(ctx, REDUMP_H);//if a struc has been deleted - it was unreferenced by any source
								iRet = 1;
							}
							else
							{
								//pType->releaseRef();
								PrintWarning() << "Could not delete object: " << TypeName(pType) << std::endl;
							}
						}
						else
							PrintWarning() << "Could not delete object in use: " << TypeName(pType) << std::endl;
					}
				}
				//else let the project to handle this command in it's own context
			}
		}
	}
	return iRet;
}

// Synopsis: compile <path>
MYCOMMAND_IMPL(compile)
{
	//DECLARE_DC_CONTEXT(ctx);
	MyArgs2 a(args);
	if (a.size() > 1)
	{
		MyString sOutDir(Files().GetRootDirectory());
		if (sOutDir.empty())
			return RecoilNoOutDir(args.AsString());

		Folder_t *pFolder(FindFileByName(a[1].c_str()));
		if (!pFolder)
			return 0;

		adcui::UDispFlags dispflags(adcui::DUMP_BLOCKS | adcui::DUMP_FUNCEX);

		std::set<FolderPtr> m;

		//prepare a list of dependencies
		FolderPtr pPrefix(mrDC.folderPtr(FTYP_PREFIX));
		if (pPrefix)
			m.insert(pPrefix);
		m.insert(pFolder);//own header
		FileDef_t *pFileDef(pFolder->fileDef());
		for (IncludeListIt i(pFileDef->includes().begin()); i != pFileDef->includes().end(); i++)
			m.insert(*i);

		for (std::set<FolderPtr>::const_iterator i(m.begin()); i != m.end(); ++i)
		{
			FolderPtr pInclude(*i);
			FileInfo_t FI(mrDC, (*pInclude->fileDef()));
			//Folder_t *pStubs(Files().FindFileByStem(ADC_DIR FILE_EXTERN));
			//if (pStubs)
			MyPath path(pInclude->name(), MyDirPath(sOutDir));
			path.SetExt(HEADER_EXT);
			SrcDumpFile ofs(path);
			adcui::UDispFlags dispflags2(dispflags);
			dispflags2.setL(adcui::DUMP_HEADER);
			FI.DumpToFile(ofs, dispflags2);//header
		}

		//dump source only
		MyString sFile;
		{
			FileInfo_t FI(mrDC, *pFileDef);
			MyPath path(pFolder->name(), MyDirPath(sOutDir));
			path.SetExt(SOURCE_EXT);
			SrcDumpFile ofs(path);
			if (!FI.DumpToFile(ofs, dispflags))//source
				return 0;
			sFile = ofs.path();
		}

		//MyString sFile(Files().relPath(pFolder) + SOURCE_EXT);

		//reinterpret_cast<Mainx_t &>(mrMain).StartCompiler(sOutDir, MyStringf("cl.exe /c %s /I" ADC_DIR, sFile.c_str()));
		reinterpret_cast<Mainx_t &>(mrMain).StartCompiler(sOutDir, MyStringf("g++ -c -fpermissive %s", sFile.c_str()));
	}
	return 1;
}

// Synopsis: $dumpexpr [options]
//		Options:
//		-p - pointer trace, or -u if no trace at locus found
//		-u - intercode (unfolded) line, ignored with -p
//		-x - expanded info (detailed reduction steps)
MYCOMMAND_IMPL(dumpexpr)
{
	DECLARE_DC_CONTEXT(ctx);
	if (ctx.opLine())
	{
		using namespace adcui;
		IADCExprViewModel::Flags eMode(IADCExprViewModel::DUMPEXPR_NULL);
		if (args.RemoveOpt("-p"))
			eMode = IADCExprViewModel::DUMPEXPR_PTRS;
		else if (args.RemoveOpt("-u"))
			eMode = IADCExprViewModel::DUMPEXPR_UNFOLD;
		
		bool bExpanded(args.RemoveOpt("-x"));

		FuncInfo_t FI(mrDC, *ctx.scopeFunc());

		if (eMode == IADCExprViewModel::DUMPEXPR_PTRS)
		{
			MyStream ss;
			if (mrDC.ReadPtrDump(FI.DockAddr(), FI.OpNo(ctx.opLine()), ss))
			{
				MyString s;
				while (ss.ReadString(s, "\n"))
					std::cout << s << std::endl;
				return 1;
			}
			eMode = IADCExprViewModel::DUMPEXPR_UNFOLD;
		}

		UDispFlags uflags;
		if (eMode == IADCExprViewModel::DUMPEXPR_UNFOLD)
			uflags.setL(DUMP_UNFOLD);
		
		ExprCacheEx_t a(FI.PtrSize());

		EXPR_t expr(FI, ctx.opLine(), 0, a);//, f0.OpNo(ctx.op()), bExpanded);
		//expr.setDumpHandler(&a);
		Out_t *pOut(expr.DumpOp(ctx.opLine()));

		TExprDump<EXPRSimpl_t> ES(FI, ctx.opLine(), a, FI.OpNo(ctx.opLine()), bExpanded);
		//EXPRSimpl_t ES(expr);
		ES.Simplify(pOut);

	}
	return 1;
}

// expand something
// Synopsis: $xpnd [options]
//		Options:
//		-s - single step
MYCOMMAND_IMPL(xpnd)
{
	DECLARE_DC_CONTEXT(ctx);
	int iRet(0);
	OpPtr pOp(ctx.opLine());
	if (pOp)
	{//RAII-block
		WriteLocker lock;
		FuncInfo_t FI(mrDC, *ctx.scopeFunc());// , * ctx.fileDef());
		PathPtr pPath(FI.PathOf(pOp));
		if (pPath->Type() == BLK_JMPSWITCH)
		{
			PathOpTracer_t tr;
			SwitchTracer_t an(FI, tr);
			iRet = an.ExpandSwitch(pPath);
		}
		else
		{
			bool bStep(false);
			if (args.size() > 1)
				bStep = (args[1] == "-s");
			PathOpTracer_t tr;
			BranchTracer_t an(FI, tr);
			iRet = an.ExpandEx(pPath, bStep);
		}
		if (iRet)
		{
			redump(ctx, REDUMP_SRC);
			//?			ctx.clear();//reset context - path could have been deleted
		}
	}
	if (iRet)
		gui().GuiOnCurFuncModified();
	return iRet;
}

//collapse
MYCOMMAND_IMPL(clps)
{
	DECLARE_DC_CONTEXT(ctx);
	int iRet(0);
	OpPtr pOp(ctx.opLine());
	if (pOp)
	{
		FuncInfo_t FI(mrDC, *ctx.scopeFunc());// , * ctx.fileDef());
		PathPtr pPath(FI.PathOf(pOp));
		if (pPath->Type() == BLK_JMPSWITCH)
		{
			PathOpTracer_t tr;
			SwitchTracer_t anlz(FI, tr);
			iRet = anlz.CollapseSwitch(pPath);
		}
	}
	return iRet;
}

MYCOMMAND_IMPL(flip)
{
	DECLARE_DC_CONTEXT(ctx);
	int iRet(0);
	OpPtr pOp(ctx.opLine());
	if (pOp)
	{
		WriteLocker lock;
		FuncInfo_t FI(mrDC, *ctx.scopeFunc());// , * ctx.fileDef());
		PathOpTracer_t tr;
		BranchTracer_t an(FI, tr);
		PathPtr pPath(FI.PathOf(pOp));
		iRet = an.FlipIfElse(pPath);
		if (iRet)
		{
			//?ctx.clear();//disable context
			redump(ctx, REDUMP_SRC);
			gui().GuiOnCurFuncModified();
		}
	}
	return iRet;
}

//#include "front/shared.h"

class RefTypesMap : private std::map<TypePtr, I_Front::RTTI_Method>
{
public:
	RefTypesMap(){}
	void insert(TypePtr p, I_Front::RTTI_Method id){
		if (p) std::map<TypePtr, I_Front::RTTI_Method>::insert(std::make_pair(p, id));
	}
	I_Front::RTTI_Method find(TypePtr p){
		iterator i(std::map<TypePtr, I_Front::RTTI_Method>::find(p));
		if (i == end())
			return I_Front::RTTI_NULL;
		return i->second;
	}
};

#include "info_class.h"

// Automatic Class Reconstruction Hierarchy through RTTI
MYCOMMAND_IMPL(reconclsh)
{
	DECLARE_DC_CONTEXT(ctx);

	TypePtr	iClass(ctx.scope());
	if (!iClass || !iClass->typeClass())
		return 0;

	RefTypesMap refsMap;
	refsMap.insert(findType(PrimeSeg(), "__class_type_info"), I_Front::RTTI_GCC);//gcc no inheritance
	refsMap.insert(findType(PrimeSeg(), "__si_class_type_info"), I_Front::RTTI_GCC_SI);//gcc single inheritance
	refsMap.insert(findType(PrimeSeg(), _PFX("__vmi_class_type_info")), I_Front::RTTI_GCC_VMI);//gcc multiple inheritance
	refsMap.insert(findType(PrimeSeg(), "__RTTIClassHierarchyDescriptor"), I_Front::RTTI_MSVC);//msvc

	I_Front::RTTI_Method eMethod(I_Front::RTTI_NULL);
	CGlobPtr pTypeInfo(nullptr);
	const ClassMemberList_t &l(ClassInfo_t::MethodsOf(ctx.scope()));
	for (ClassMemberListCIt it(l.begin()); it != l.end(); ++it)
	{
		CGlobPtr pGlob(*it);
		if (!IsStaticMemberFunction(pGlob))
			continue;
		FieldPtr pField(DockField(pGlob));
		eMethod = refsMap.find(pField->type());
		if (eMethod != I_Front::RTTI_NULL)
		{
			pTypeInfo = pGlob;
			break;
		}
	}

	if (pTypeInfo)
	{
		ClassInfo_t CI(*this, iClass);
		const ClassVTables_t& vtables(CI.VTablesOf(iClass));
		ClassVTables_t::const_iterator i(vtables.begin());
		for (;; ++i)
		{
			CGlobPtr pVTable(nullptr);
			if (i != vtables.end())
				pVTable = i->second.self;

			std::cout << "Attempting to recover class hierarchy of "
				<< TypePrettyNameFull(iClass, CHOP_SYMB) << " through "
				<< GlobNameFull(pTypeInfo, E_PRETTY, CHOP_SYMB) << "..."
				<< std::endl;
			FRONT_t* pFRONT(FrontEnd(PrimeSeg()));
			ClassHierarchyInterim_t cb(mrDC, iClass);
			if (pFRONT->device(GetDataSource())->dumpClassHierachy(&cb, DockAddr(pTypeInfo), eMethod, pVTable ? DockAddr(pVTable) : 0))
			{
				cb.print(std::cout);
				cb.reconstruct();
				redump(ctx, REDUMP_ALL);
				gui().GuiOnCurStrucModified();
				return 1;
			}
			if (!pVTable)
				break;
		}
	}

	PrintError() << "Class " << TypePrettyNameFull(iClass, CHOP_SYMB) << " does not contain type info object as a static memeber" << std::endl;
	return 0;
}


// Command: addheir
// Desc: Add inheritance to a class;
// Synopsis: addheir [-x|-y|-z]
//		-x: public
//		-y: protected
//		-z: private
MYCOMMAND_IMPL(addheir)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.scope() || ctx.locality().scoping != adcui::LocusId_STRUC_HEADER)
		return 0;
	MyArgs2 args2(args);
	int iHeir(1);//0:public,1:protected:2:private
	if (args2.RemoveOpt("-x"))
		iHeir = 1;
	else if (args2.RemoveOpt("-y"))
		iHeir = 2;
	else if (args2.RemoveOpt("-z"))
		iHeir = 3;

	if (AddInheritance(ctx.scope(), nullptr, iHeir))
	{
		redump(ctx, REDUMP_H);
		gui().GuiOnCurStrucModified();
		return 1;
	}

	return 0;
}

// Command: rmheir
// Desc: Remove inheritance (level) in class
MYCOMMAND_IMPL(rmheir)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.scope() || ctx.locality().scoping != adcui::LocusId_STRUC_HEADER)
		return 0;
	if (RemoveInheritance(ctx.scope()))
	{
		redump(ctx, REDUMP_H);
		gui().GuiOnCurStrucModified();
		return 1;
	}
	return 0;
}

// Command: togvptr
// Desc: Toggle virtual function table pointer (vptr) in class
MYCOMMAND_IMPL(togvptr)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.scope())
		return 0;

	//TypePtr pType();
	if (ctx.locality().scoping != adcui::LocusId_STRUC_HEADER)
		return 0;
	
//	if (ctx.locality() != adcui::CONTEXTID_SOURCE_DATA) || ctx.locality() != adcui::CONTEXTID_STRUC_HEADER)
	//	return 0;
	//if (ctx.back().offs() != 0)//at zero offsets only
		//return 0;

	ProjModifier_t PJ(*this, memMgrGlob());
	FieldPtr pField(PJ.makeOffset(ctx.locus(), true));
	if (pField)
	{
		{//RAII-block (may dispose of the unrefed vptr type)
			BinaryCleaner_t<> PC(PJ);
			PC.ClearType(pField);
		}

		if (MakeTypeVPtr(pField))
		{
			redump(ctx, REDUMP_H);
			gui().GuiOnCurStrucModified();
			return 1;
		}
	}
	PrintError() << "Failed to make v-ptr" << std::endl;
	return 0;
};

bool DcInfo_t::ToggleThisPtr(FieldPtr pField)
{
	GlobPtr pGlob(GlobObj(pField));
	if (!pGlob)
		return false;
	ProtoInfo_t TI(*this, pGlob);
	if (TI.IsThisCallType())
	{//undo
		if (!TI.UnmakeThisCall())
			return false;
	}
	else if (TI.IsClassMember())
	{
		FieldPtr pArg(TI.FirstArg());
		if (!pArg || !TI.MakeThisCallFromArg(pArg))
			return false;
	}
	else
		return false;
	return true;
}

// Command: togthisptr
// Desc: Toggle member function's this pointer (thiscall)
MYCOMMAND_IMPL(togthisptr)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.scope())
		return 0;

	//TypePtr pType();
	if (ctx.locality().scoping == adcui::LocusId_FUNC_HEADER || ctx.locality().scoping == adcui::LocusId_STRUC_METHOD)
	{
		FieldPtr pField(ctx.pickedFieldDecl());
		if (pField)
		{
			if (ToggleThisPtr(pField))
			{
				redump(ctx, REDUMP_H);
				gui().GuiOnCurStrucModified();
				return 1;
			}
		}
	}
	PrintError() << "Failed to toggle 'this' ptr" << std::endl;
	return 0;
}

/*?MYCOMMAND_IMPL(newfile)//dc
{
DECLARE_DC_CONTEXT(ctx);
MyArgs2 a(args);
if (a.size() > 1)
{
Folder_t *pFolder(AddFileEx(a[1].c_str(), FTYP_SOURCE));
if (pFolder)
{
ctx.setFolder(pFolder);
return 1;
}
}
return 0;
}*/

// Command: convns
// Desc: Convert a (compound) type to a namespace 
MYCOMMAND_IMPL(convns)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.scope() || ctx.locality().scoping != adcui::LocusId_STRUC_HEADER)
		return 0;
	if (!AssureTypeNamespace(ctx.scope()))
		return 0;
	redump(ctx, REDUMP_ALL);
	mrProject.markDirty(DIRTY_TYPES);
	mrProject.markDirty(DIRTY_GLOBALS);
	return 1;
}

// Command: convcls
// Desc: Convert a (compound) type to a class 
MYCOMMAND_IMPL(convcls)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.scope() || ctx.locality().scoping != adcui::LocusId_STRUC_HEADER)
		return 0;
	return 0;
}

// Command: convstr
// Desc: Convert a (compound) type to a structure 
MYCOMMAND_IMPL(convstr)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.scope() || ctx.locality().scoping != adcui::LocusId_STRUC_HEADER)
		return 0;
	return 0;
}

// Command: convenu
// Desc: Convert a (compound) type to an enumeration 
MYCOMMAND_IMPL(convenu)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.scope() || ctx.locality().scoping != adcui::LocusId_STRUC_HEADER)
		return 0;
	return 0;
}

// Command: convuni
// Desc: Convert a (compound) type to a union 
MYCOMMAND_IMPL(convuni)
{
	DECLARE_DC_CONTEXT(ctx);
	if (!ctx.scope() || ctx.locality().scoping != adcui::LocusId_STRUC_HEADER)
		return 0;
	return 0;
}

// Command: draftest
MYCOMMAND_IMPL(draftest)
{
	DECLARE_DC_CONTEXT(ctx);
#ifdef _DEBUG
	Display_t::EnableDraftTest(!Display_t::IsDraftTest());
	redump(ctx, REDUMP_ALL);
#endif
	return 1;
}

MYCOMMAND_IMPL(load_stubs)
{
	DECLARE_DC_CONTEXT(ctx);

	StubInfo_t SI(mrDC);
	if (!SI.LoadStubs())
		return 0;

	projx().guix().GuiOnStubsModified();
	return 1;
}

// Command: analyze
// Desc: Analyze class declaration
// Synopsis: analyze
MYCOMMAND_IMPL(analyze)
{
	DECLARE_DC_CONTEXT(ctx);

	FolderPtr pFolder(ctx.folder());
	if (!pFolder || !pFolder->fileDef())
	{
		PrintError() << "No context file" << std::endl;
		return 0;
	}

	TypePtr pClass(nullptr);
	if (ctx.scope())
		pClass = ctx.scopeStruc();
	if (!pClass || !pClass->typeClass())
	{
		PrintError() << "No class to analyze" << std::endl;
		return 0;
	}

	ClassInfo_t classInfo(*this, pClass);
	classInfo.AnalyzeClass();

	FullName_t sClass(TypeNameFull(pClass, E_PRETTY, CHOP_SYMB));
	std::cout << "Analyzing class: " << sClass << std::endl;

	redump(ctx, REDUMP_ALL);

	return 1;
};

// Command: dcfile
// Desc: Decompile all (non-stub) function in a file
// Synopsis: dcfile [file]
MYCOMMAND_IMPL(dcfile)
{
	DECLARE_DC_CONTEXT(ctx);

	FolderPtr pFolder(nullptr);
	if (args.size() > 1)
		pFolder = FindFileByName(args[1]);
	else
		pFolder = ctx.folder();

	if (!pFolder || !pFolder->fileDef())
	{
		PrintError() << "No context file" << std::endl;
		return 0;
	}

	ScriptMgr_t* pScript(main().OpenScript());
	pScript->setContext(&ctx);

	const GlobMap& m(pFolder->fileDef()->globs());
	ScriptMgr_t::iterator j(pScript->begin());
	for (GlobMapCIt i(m.begin()); i != m.end(); ++i)
	{
		CGlobPtr g(&(*i));
		if (g->func() && g->func()->isStub())//do stubs only and the cloned ones (-firststub)
		{
			MyStringf s("dc -firststub -@%08X", DockAddr(g));
			pScript->insert(j, s);
		}
	}
	return 1;
}

// Command: decompile
// Alias: dc
// Desc: Decompile/undecompile a function;
// Synopsis: decompile [-fromstub] [-purge] [-n <name>] [-@<addr>]
//		-fromstub	: check and apply a stub definition (if exists) before processing
//		-purge		: purge (undecompile) the function's body; if the function is a stub - do nothing
//		-firststub	: pick a first stubbed entry in context file
//		-n <name>	: rename a function as a favor
//		-@<addr>	: use a function at address `addr` instead the one from context
MYCOMMAND_IMPL(decompile)
{
	DECLARE_DC_CONTEXT(ctx);

	if (Project().analyzer())
	{
		mrMain.resumeAnalysis(StopFlag::RESET);
		return 1;
	}

	FolderPtr pFolder(ctx.folder());
	if (!pFolder || !pFolder->fileDef())
	{
		PrintError() << "No context file" << std::endl;
		return 0;
	}

	//TypePtr pProc(nullptr);
	FieldPtr pField0(nullptr);
	FieldPtr pProcOrThunk(nullptr);

	//bool bForce(args.RemoveOpt("-f"));
	bool bFromStub(args.RemoveOpt("-fromstub"));
	bool bPurge(args.RemoveOpt("-purge"));
	bool bFirstStub(args.RemoveOpt("-firststub"));
	MyString sAt(AtStr(args, true));
	if (!sAt.empty())
	{
		//first try a first stubbed entry with specified address in a context file
		//ADDR va(strtoul(sAt.c_str(), nullptr, 16));
		Locus_t& aLoc(ctx.locus());
		LocusFromStr(sAt, aLoc, FindFrontSeg());

		ADDR va(aLoc.va());
//CHECK(va == 0x20611CC0)
//STOP
		if (bFirstStub)
		{
			GlobMap& m(pFolder->fileDef()->globs());
			for (GlobMapIt i(m.lower_bound(va)); i != m.end() && i->_key() == va; ++i)
			{
				GlobPtr g(&(*i));
				if (g->func() && g->func()->isStub())
				{
					pField0 = DockField(g);
					break;
				}
			}
			if (!pField0)
				PrintWarning() << "no stub at VA=" << VA2STR2(va) << " in current file" << std::endl;
		}
		if (!pField0)
		{
			//find a procedure in a module at given address
			TypePtr iSeg(FindSegAt(PrimeSeg(), va, PrimeSeg()->typeSeg()->affinity()));
			if (iSeg)
			{
				//get a sutable field
				FieldMap& m(iSeg->typeStruc()->fields());
				for (FieldMapIt it(m.lower_bound(va)); it != m.end() && KEY(it) == va; ++it)
				{
					FieldPtr pField(VALUE(it));
					if (pField->isTypeProc() || pField->isTypeThunk())
					{
						assert(!pProcOrThunk);
						pProcOrThunk = pField;
					}

					if (!pField0)
					{
						GlobPtr pGlob(GlobObj(pField));
						if (!pGlob || !pGlob->func() || pGlob->func()->isStub())
						{
							if (pField == pProcOrThunk || !pField->type())
							{
								pField0 = pField;
								if (pProcOrThunk)
									break;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		if (!(args.size() > 1))//from context
		{
			GlobPtr iGlob(ctx.scope() ? ctx.scopeFunc() : nullptr);
			if (!iGlob)
			{
				PrintError() << "No function at context to decompile" << std::endl;
				return 0;
			}
			pField0 = DockField(iGlob);
		}
	}

	if (!pField0)
	{
		PrintError() << "Could not resolve between cloned entries to decompile at VA=" << VA2STR2(ctx.locus().va()) << std::endl;
		return 0;
	}

	WriteLocker lock;
	DcInfo_t DI0(mrDC, memMgrGlob());//public memmgr!


		/*if (pField0->isCloneMaster())
		{
			//so we wanted to decompile some cloned field, but which one? 
			pField = FindCloneOf(pField0, bFirstStub);//if `bFirstStub` is true, locate a first stubbed entry, otherwise - pick the very first one
			if (!pField)
			{
				PrintError() << "Could not resolve between cloned entries to decompile at VA=" << VA2STR2(ctx.locus().va()) << std::endl;
				return 0;
			}
		}*/
CHECK(pField0->_key() == 0x01003ac0)
STOP

	GlobPtr iGlob(GlobObj(pField0));
	if (!iGlob || !iGlob->func())
		iGlob = DI0.AcquireFunction(pField0, pFolder);

	DI0.MoveGlobToFile(iGlob, pFolder);

	int i;
	if ((i = args.Find("-n")) >= 0)
	{
		ModuleInfo_t MI(mrProject, mrModuleRef);//global memmgr
		if (!((i + 1) < (int)args.size()) || !MI.SetFieldName(DockField(iGlob), args[i + 1].c_str()))
			PrintWarning() << "Invalid function name argument at VA=" << VA2STR2(DockAddr(iGlob)) << std::endl;
	}

	assert(iGlob);
	FuncInfo_t FI(*this, *iGlob);

	if (bPurge)
	{
		if (!ProtoInfo_t::IsStub(iGlob))
		{
			//a direct request to cleanup the function's body
			FuncCleaner_t FD(FI);
			FD.DestroyBody();
			FD.Cleanup();
			FD.CleanupFinal();//types
			FI.SetFuncStatus(FDEF_DEFINED);
			//leave pstack/fstack diff/spoilt list entact
			if (bFromStub)
				FI.FromFuncProfile();
		}
		else
		{
			if (!bFromStub)
				return 0;//-purge on a stub does nothing..
			FI.FromFuncProfile();
		}
		//no decompilation is gonna take place..
	}
	else
	{
		if (!FI.Redecompile(bFromStub))
			return 0;
	}

	//now, when the func is purged, adjust the proto from stub, if requested

	//if (iRet > 0)
	{
		redump(ctx, REDUMP_SRC);

		MyString sFile(Project().files().relPath(pFolder) + SOURCE_EXT);
		guix().GuiOnDecompileFunction(sFile, "$task");
	}

	return 1;
}




// Command: toggle_virtual
// Desc: Toggle function 'virtuallity' status in a class
// Synopsis: toggle_virtual
MYCOMMAND_IMPL(toggle_virtual)
{
	DECLARE_DC_CONTEXT(ctx);

	FolderPtr pFolder(ctx.folder());
	if (!pFolder || !pFolder->fileDef())
	{
		PrintError() << "No context file" << std::endl;
		return 0;
	}

	/*TypePtr pClass(nullptr);
	if (ctx.scope())
		pClass = ctx.scopeStruc();
	if (!pClass || !pClass->typeClass())
	{
		PrintError() << "No class to analyze" << std::endl;
		return 0;
	}*/

	if (ctx.locality().scoping == adcui::LocusId_STRUC_METHOD)
	{
		TypePtr pClass(ctx.scopeStruc());
		assert(pClass->typeClass());
		ClassInfo_t classInfo(*this, pClass);

		FieldPtr pField(ctx.pickedFieldDecl());
		GlobPtr pGlob(GlobObj(pField));
		if (ClassInfo_t::IsMethodVirtual(pGlob))
		{
			classInfo.UnMakeMethodVirtual(pGlob);
		}
		else
		{
			classInfo.MakeMethodVirtual(pGlob);
		}
		redump(ctx, REDUMP_H);
		return 1;
	}
	PrintError() << "Class method expected" << std::endl;
	return 0;
}



// Command: cut
// Desc: Cut an object (at locus) and appends a cut list
// Synopsis: cut [obj_qname?]
MYCOMMAND_IMPL(cut)
{
	DECLARE_DC_CONTEXT(ctx);

	if (ctx.locality().scoping == adcui::LocusId_FUNC_HEADER)
	{
		TypePtr pProc(ctx.locus().proc());
		FieldPtr pField;
		if (pProc)
			pField = pProc->parentField();
		else
			pField = ctx.pickedFieldDecl();
		if (!pField)
			return 0;
		assert(GlobFuncObj(pField));
		if (projx().addCutList(pField))
		{
			guix().OnCutListChanged();
			return 1;
		}
	}
	else
	{
		FieldPtr pField(ctx.pickedFieldDecl());
		if (pField)
		{
			STOP
		}
	}

	return 0;
}


// Command: paste
// Desc: Paste a contents of a cut list to location at locus
// Synopsis: paste [obj_qname?]
MYCOMMAND_IMPL(paste)
{
	DECLARE_DC_CONTEXT(ctx);

	FolderPtr pFolderTo(ctx.folder());
	if (!pFolderTo || !pFolderTo->fileDef())
		return 0;

	int count(0);
	ObjPtr pObj;
	while ((pObj = projx().takeCutListItem(0)) != nullptr)
	{
		FieldPtr pField(pObj->objField());
		if (pField)
		{
			GlobPtr pGlob(DcInfo_t::GlobObj(pField));
			assert(pGlob);

			FolderPtr pFolderFrom(pGlob->folder());
			TypePtr pModule(ModuleOf(pFolderFrom));
			assert(pModule);

			Dc_t* pDC(DCREF(pModule));
			FileInfo_t GI(*pDC, *pFolderFrom->fileDef());
			if (GI.MoveGlobToFile(pGlob, pFolderTo))
				++count;
		}
		else
			break;
	}

	if (count > 0)
	{
		redump();//everything!
		guix().OnCutListChanged();
		return 1;
	}

	PrintWarning() << "Nothing to paste - a Cut list is empty" << std::endl;
	return 0;
}

// Command: uncut
// Desc: Remove an item at <index> from the cut list
// Synopsis: uncut <index>
MYCOMMAND_IMPL(uncut)
{
	DECLARE_DC_CONTEXT(ctx);

	if (args.size() > 1)
	{
		size_t id(atoi(args[1]));
		if (projx().takeCutListItem(id))
		{
			guix().OnCutListChanged();
			return 1;
		}
	}
	return 0;
}





