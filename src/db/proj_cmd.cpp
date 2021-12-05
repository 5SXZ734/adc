#include "proj_cmd.h"
#include "mem.h"
#include "main.h"
#include "obj.h"
#include "field.h"
#include "type_struc.h"
#include "names.h"
#include "script.h"
#include "command.h"
#include "type_seg.h"
#include "anlzbin.h"
#include "debug.h"
#include "clean.h"
#include "ui_main.h"
#include "print.h"
#include "front_impl.h"
#include "symbol_map.h"

//////////////////////////////////////////////////////////////////////////
// Commands


#define MYCOMMAND_IMPL(name) \
	int ProjCmdServer_t::COMMAND_##name(CMDServer_t *pSelf, Cmd_t &args){ return static_cast<ProjCmdServer_t*>(pSelf)->OnCommand_##name(args); } \
	int ProjCmdServer_t::OnCommand_##name(Cmd_t &args)


// @ADDR:  [-@ addr|-@addr], addr = <~row id>|<.raw offs>|<virtual addr>

// Synopsis: $makeobj [@ADDR] [-n <name>] <type_name>
MYCOMMAND_IMPL(makeobj)
{
	if (!(args.size() > 1))
		return 0;

	DECLARE_CONTEXT(ctx);

	TypePtr pModule(ctx.moduleFromLocus());

	ProjModifier_t PJ(mrProject, *pModule);

	LocusFromStr(AtStr(args, true), ctx.locus(), PJ.FindFrontSeg());

	ctx.locus().stripToSeg();

	bool bForce(args.RemoveOpt("-f"));

	MyString sName;
	args.RemoveOptEx("-n", sName);

	MyString sType;
	args.RemoveOptEx("-t", sType);

	//MyString sArgsTo;//for recepient
	//args.RemoveOptEx("-args", sArgsTo);

	//if (sName.empty())
	//	return 0;

	MyString sOpt(args.AsString(1));

	TypePtr pType(nullptr);
	if (!sType.isEmpty())
	{
		pType = PJ.TypeInContext(sType, ctx.locus().struc());
		if (!pType)
			pType = FindAtticTypeByName(sType);
	}

	if (!pType)
	{
		//recognize file format at locus and initiate preformat
		if (sType.isEmpty())
			sType = PJ.FormatterAtLocus(ctx);
		if (!sType.isEmpty())
		{
			if (!mrMain.createExternalType(sType, ctx.locus(), sOpt))
				return 0;
			return 1;
		}
		PrintWarning() << "No such type in context: " << sType << std::endl;
	}

	if (!PJ.makeObjOfType(ctx.locus(), pType, sName, bForce))
		return 0;

	return 1;
}

// Synopsis: $makeobj [@ADDR] [-n <name>] <type_name>
MYCOMMAND_IMPL(preformat)
{
	//DECLARE_CONTEXT(ctx);

	//Locus_t addrx;
	//if (!LocusFromArgs(args, addrx))
		//addrx = ctx;

	WriteLocker lock;
	if (!OnCommand_makeobj(args))
		return 0;

	gui().GuiOnPreanalized();
	return 1;
}

//Synopsis: $makefunc [-f] [@ADDR]
MYCOMMAND_IMPL(makefunc)
{
	DECLARE_CONTEXT(ctx);

	MyString sAt(AtStr(args, true));
	LocusFromStr(sAt, ctx.locus());

#ifdef _DEBUG
ADDR _va(ctx.locus().va());
CHECK(_va == 0x68DD50)
STOP
CHECK(_va == 0x1d98bd)
STOP
#endif

	//queueAnlzBinEvent(TODO_MAKEFUNC, args);
	//?mrMain.OnAnlzReady();
	bool bUserAction(args.RemoveOpt("-f"));
	bool bPropagateName(args.RemoveOpt("-nppg"));
CHECK(bPropagateName)
STOP

	int i;
	int depth(mrMain.options().nCallDepth);//-1
	if ((i = args.Find("-d")) >= 0)
	{
		args.RemoveAt(i);
		if (i < (int)args.size())
		{
			depth = atoi(args[i]);
			args.RemoveAt(i);
		}
	}

	bool bSweepOnly(args.RemoveOpt("-sweep"));
	bool bTryThunk(args.RemoveOpt("-trythunk"));//try thunk first
	bool bWeak(args.RemoveOpt("-weak"));//the command was originated not in a call?
CHECK(bWeak)
STOP

	ProjModifier_t MI(mrProject, *ctx.moduleFromLocus());

	int iSweep(0);
	int iSplit(bUserAction ? 0 : MI.SplitFunction(ctx.locus(), bWeak));//for a user action, the split is not nessecery, makeFunc may fail
	if (iSplit >= 0)//no split required
	{
		if (!bSweepOnly)
		{
			//if (iSplit > 0)//function has changed
				//MI.LocusFromStr(sAt, ctx.locus());//recalculate location

			RESULT_e eRes(MI.MakeCode(ctx.locus(), true));
			if (eRes == RESULT_FAILED)
				return 0;
			if (eRes == RESULT_OK)//just created
				iSweep = 1;//code
			if (bTryThunk)
			{
				if (MI.MakeThunk(ctx.locus(), bUserAction) == RESULT_OK)//just created
					iSweep = 2;//thunk
			}
			if (iSweep != 2)//not a thunk
				if (MI.MakeProcedure(ctx.locus(), bUserAction) == RESULT_OK)
					iSweep = 3;
		}
		else
		{
			ctx.locus().stripToSeg();
			iSweep = 3;
		}
	}

	unsigned flags(0);
	if (bPropagateName)
		flags |= ProjModifier_t::SweepCode_NamePpg;

	if (iSweep == 1)
		MI.SweepCode(ctx.locus().field0(), depth, flags);
	else if (iSweep == 2)
		MI.SweepThunk(ctx.locus().field0(), depth, flags);
	else if (iSweep == 3)
		MI.SweepProc(ctx.locus().field0(), depth, flags);
	else
		return 0;

	//MI.TriggerCodeSweep(ctx.seg(), ctx.va(), depth);
	return 1;
}

MYCOMMAND_IMPL(sweepfunc)
{
	DECLARE_CONTEXT(ctx);

	LocusFromStr(AtStr(args), ctx.locus());

	int i;
	int depth(mrMain.options().nCallDepth);//-1
	if ((i = args.Find("-d")) >= 0)
	{
		args.RemoveAt(i);
		if (i < (int)args.size())
		{
			depth = atoi(args[i]);
			args.RemoveAt(i);
		}
	}

	ProjModifier_t MI(mrProject, *ctx.moduleFromLocus());
	MI.SweepProc(ctx.locus().field0(), depth);
	return 1;
}

//Synopsis: funcend [@ADDR]
MYCOMMAND_IMPL(funcend)
{
	DECLARE_CONTEXT(ctx);

	//Locus_t loc;
	LocusFromStr(AtStr(args), ctx.locus());

	ProjModifier_t PJ(mrProject, *ctx.moduleFromLocus());
	if (!PJ.SetFunctionEnd(ctx.locus()))
		return 0;
	//gui().GuiOnProjectModified();
	mrProject.markDirty(DIRTY_GLOBALS);
	return 1;
}

// Resize a structure
//Synopsis: rsz [@ADDR] <new_size>
MYCOMMAND_IMPL(rsz)
{
	DECLARE_CONTEXT(ctx);

	//Locus_t loc;
	LocusFromStr(AtStr(args), ctx.locus());

	if (ctx.locus().empty())
		return 0;

	if (!(args.size() > 1))
		return 0;

	value_t v;
	Str2Int(args.back().c_str(), v);

	ProjModifier_t PJ(mrProject, *ctx.moduleFromLocus());
	if (!PJ.ResizeStruc(ctx.locus().back().cont(), v.ui32, ctx.locus().upframe(1), true))
		return 0;
	//gui().GuiOnProjectModified();
	mrProject.markDirty(DIRTY_GLOBALS);
	return 1;
}


// Command: makecode
// Alias: mkcode
// Synopsis: makecode [flags] [@ADDR] [-ref <addr_ref>]
// flags:
//		-f	: force
//		-d	: call depth
//		-i	: indirect, apply the command to the referenced location(s)
MYCOMMAND_IMPL(makecode)
{
	DECLARE_CONTEXT(ctx);

	TypePtr pModule(ctx.moduleFromLocus());
	if (!pModule)
		return 0;

	ProjModifier_t MI(mrProject, *pModule);

	LocusFromStr(AtStr(args), ctx.locus(), MI.FindFrontSeg());

	if (args.RemoveOpt("-i"))
	{
		CFieldPtr pField(ctx.locus().field0());
		if (!pField)
			return 0;
		return (MI.ForwardCommand(args.AsString(), pField) > 0) ? 1 : 0;
		//for (DataValueIterator i(*this, pModule, pField); i; ++i){}
	}

	bool bForce(args.RemoveOpt("-f"));

	int i;
	int depth(mrMain.options().nCallDepth);//-1
	if ((i = args.Find("-d")) >= 0)
	{
		args.RemoveAt(i);
		if (i < (int)args.size())
		{
			depth = atoi(args[i]);
			args.RemoveAt(i);
		}
	}

	//check if we can extend owner's function size
//CHECK(ctx.addr() == 0x100263ff)
//STOP

	Locus_t aLocRef;
	TypePtr iSrc(nullptr);
	MyString sAtRef(args.FindOpt("-ref"));
	if (!sAtRef.empty())
	{
		//ADDR64 vaRef(strtoull(sAtRef.c_str(), nullptr, 16));
		if (LocusFromStr(sAtRef, aLocRef, MI.FindFrontSeg()))
		//if (VA2Locus(ctx.locus().seg(), vaRef, aLocRef))
			iSrc = aLocRef.struc();
		else
			ASSERT0;//aLocRef.clear();
	}

	//if (!sAtRef.empty() && (Str2DA(sAtRef, &da) && LocusFromDA(da, aLocRef)))
		//iSrc = aLocRef.struc();

	TypePtr iFunc(nullptr);
	if (iSrc && iSrc->typeProc())
		iFunc = iSrc;//are we jumping out of the func?

	/*if (!iFunc)
	{
		FieldMapCIt it(StrucFieldIt(iSrc, aLocRef.addr()));
		if (it != iSrc->typeStruc()->fields().end())
		{
			FieldPtr pField(VALUE(it));
			iFunc = pField->IsFunc();//are we jumping into the func?
			assert(!iFunc);
		}
	}*/

	if (iFunc)//does the func expands over the selected location?
	{
		unsigned fsz(FuncSizeLimited(iFunc));
		if ((ADDR)(iFunc->parentField()->offset() + fsz) > ctx.locus().addr())
			iFunc = nullptr;//if it does - no need to expand later
		bForce = true;
	}

	if (MI.MakeCode(ctx.locus(), bForce) == RESULT_FAILED)
		return 0;

	ADDR end_addr(0);
	if (ctx.locus().field0())
		end_addr = MI.SweepCode(ctx.locus().field0(), depth);

	//				if (end_addr > func_end)
	//					func_end = end_addr;

	if (iFunc)
		MI.ExpandFunc(aLocRef, end_addr);

	//OUTPUT.printf("__makeCode:%X\n", addr0);
		//gui().GuiOnProjectModified();
	return 1;
}

// Command: makethunk
// Alias: mkthk
// Synopsis: makecode [flags] [@ADDR] [-ref <addr_ref>]
MYCOMMAND_IMPL(makethunk)
{
	DECLARE_CONTEXT(ctx);

	LocusFromStr(AtStr(args), ctx.locus());

	TypePtr pModule(ctx.moduleFromLocus());
	ProjModifier_t MI(mrProject, *pModule);

	RESULT_e eRes(RESULT_FAILED);
	if ((eRes = MI.MakeCode(ctx.locus(), true)) == RESULT_FAILED)
		return 0;
	if (eRes == RESULT_OK)//a field just created?
		MI.TriggerCodeSweep(ctx.locus().seg(), ctx.locus().va());
	if (MI.MakeThunk(ctx.locus(), false) != RESULT_OK)
		return 0;

	return 1;
};

//Synopsis: $makedata [@ADDR] [-f] [<typename> | <typeid>]
// -f : force (user action)
MYCOMMAND_IMPL(makedata)
{
	DECLARE_CONTEXT(ctx);

	LocusFromStr(AtStr(args), ctx.locus());

#ifdef _DEBUG
	ADDR _va(ctx.locus().va());
CHECK(_va == 0x29180)
STOP
	MyString sAtRef(args.FindOpt("-ref"));
#endif

	bool bForce(args.RemoveOpt("-f"));

	ProjModifier_t PJ(mrProject, *ctx.moduleFromLocus());

	TypePtr pType(nullptr);
	if (args.size() > 1)
	{
		MyString s(args.back());
		if (!s.empty() && isdigit(s.at(0)))
		{
			OpType_t tid((OpType_t)atoi(s.c_str()));
			if (tid != OPTYP_NULL)
				pType = PJ.GetStockType(tid);
		}
	}

	if (!PJ.MakeData(ctx.locus(), pType, ATTR_NULL, bForce))
		return 0;

	//gui().GuiOnProjectModified();
	return 1;
}

//Synopsis: $makeint [@ADDR] [-f] [-a | -u]
// -f : force (user action)
// -a : ascii
// -u : unicode (16 bits)
// -t : ascii text
MYCOMMAND_IMPL(makeint)
{
	DECLARE_CONTEXT(ctx);

	LocusFromStr(AtStr(args), ctx.locus());

	bool bForce(args.RemoveOpt("-f"));

	ProjModifier_t MI(mrProject, *ctx.moduleFromLocus());

	bool bMakeString(false);
	TypePtr pType(nullptr);
	AttrIdEnum attr(ATTR_NULL);
	if (args.RemoveOpt("-a"))
	{
		pType = MI.GetStockType(OPTYP_CHAR8);
		bMakeString = true;
	}
	else if (args.RemoveOpt("-u"))
	{
		pType = MI.GetStockType(OPTYP_CHAR16);
		bMakeString = true;
	}
	if (args.RemoveOpt("-t"))
	{
		pType = MI.GetStockType(OPTYP_CHAR8);
		attr = ATTR_ASCII_TEXT;
		bMakeString = true;
	}

	FieldPtr pField(MI.MakeData(pType, attr, bForce, ctx.locus()));
	if (!pField)
		PrintError() << "Failed to make integer at " << VA2STR(ctx.locus().va()) << std::endl;
	else if (bForce && bMakeString)
		pField = MI.MakeString(attr, ctx.locus());
//	MI.terminalFieldAt(ctx.locus());//update locus

	return pField ? 1 : 0;
}

//Command: make_signed
//Synopsis: make_signed [@ADDR]
//Alias: mks
MYCOMMAND_IMPL(make_signed)
{
	DECLARE_CONTEXT(ctx);

	LocusFromStr(AtStr(args), ctx.locus());

	bool bForce(args.RemoveOpt("-f"));

	ProjModifier_t MI(mrProject, *ctx.moduleFromLocus());

	if (!MI.MakeSigned(ctx.locus(), bForce))
		return 0;

	return 1;
}

//Command: makereal
//Synopsis: $makereal [@ADDR] [-f]
// -f : force (user action)
MYCOMMAND_IMPL(makereal)
{
	DECLARE_CONTEXT(ctx);

	LocusFromStr(AtStr(args), ctx.locus());

	AttrIdEnum attr(ATTR_NULL);
	bool bForce(args.RemoveOpt("-f"));

	ProjModifier_t PJ(mrProject, *ctx.moduleFromLocus());
	if (!PJ.MakeReal(ctx.locus(), bForce))
		return 0;

	return 1;
}

//Synopsis: $makebit [@ADDR] [<typeid>]
MYCOMMAND_IMPL(makebit)
{
	DECLARE_CONTEXT(ctx);

	LocusFromStr(AtStr(args), ctx.locus());

	TypePtr pType(nullptr);
	AttrIdEnum attr(ATTR_NULL);
	bool bForce(args.RemoveOpt("-f"));

	ProjModifier_t PJ(mrProject, *ctx.moduleFromLocus());
	if (!PJ.makeBit(ctx.locus(), pType, attr, bForce))
		return 0;

	return 1;
}

MYCOMMAND_IMPL(mkptr)
{
	return 0;
}

MYCOMMAND_IMPL(mkconst)
{
	return 0;
}

MYCOMMAND_IMPL(applytype)
{
	return 0;
}

//Synopsis: mkgap [-f]
//	Make gap in a structure (extend)
MYCOMMAND_IMPL(mkgap)
{
	DECLARE_CONTEXT(ctx);
	LocusFromStr(AtStr(args), ctx.locus());

	ProjModifier_t PJ(mrProject, *ctx.moduleFromLocus());
	if (PJ.MakeGap(ctx, false))
		return 1;

	return 0;
}

//Synopsis: instantiate [@ADDR] [-u]
//Desc: Instantiate field's type (make it non-shared)
//Alias: inst
//Options:
//	-u :	unnamed (use names map of parent container)
MYCOMMAND_IMPL(instantiate)
{
	DECLARE_CONTEXT(ctx);
	LocusFromStr(AtStr(args), ctx.locus());

	bool bUnnamed(args.FindOpt("-u"));

	//ctx.stripToSeg();
	FieldPtr pField(ctx.locus().field0());
	if (pField)
	{
		TypePtr pModule(ctx.locus().module());// ModuleOf(ctx.field()));

		FrontImpl_t fimp(mrProject, ctx.locus(), pModule);
		FrontIface_t iface(fimp);
		if (fimp.InstantiateType(pField, bUnnamed, iface))
		{
			mrProject.markDirty(DIRTY_GLOBALS);
			return 1;
		}
	}
	return 0;
}

//Synopsis: toggle_exported [@ADDR]
//Desc: Toggle exported entry (backed by FE). Should also work on export refs?
//Alias: tgexp
MYCOMMAND_IMPL(toggle_exported)
{
	DECLARE_CONTEXT(ctx);
	LocusFromStr(AtStr(args), ctx.locus());
	ProjModifier_t PM(mrProject, *ctx.moduleFromLocus());
	if (PM.ToggleExported(ctx.locus()))
		return 1;
	return 0;
}

//Synopsis: toggle_imported [@ADDR]
//Desc: Toggle exported entry (backed by FE)
//Alias: tgexp
MYCOMMAND_IMPL(toggle_imported)
{
	DECLARE_CONTEXT(ctx);
	LocusFromStr(AtStr(args), ctx.locus());
	ProjModifier_t PM(mrProject, *ctx.moduleFromLocus());
	if (PM.ToggleImported(ctx.locus()))
		return 1;
	return 0;
}

//Synopsis: $makeoff [-f]
//	Make Offset
MYCOMMAND_IMPL(makeoff)
{
	DECLARE_CONTEXT(ctx);

	LocusFromStr(AtStr(args), ctx.locus());

	bool bForce(args.RemoveOpt("-f"));

	//queueAnlzBinEvent(TODO_MAKEOFF, args);
	//?mrMain.OnAnlzReady();
	ProjModifier_t PJ(mrProject, *ctx.moduleFromLocus());
	FieldPtr pField(PJ.makeOffset(ctx.locus(), bForce));
	if (!pField)//, node.pType, node.bForce))
		return 0;
	//gui().GuiOnProjectModified();
	return 1;
}

//Synopsis: $makearr [@ADDR] [<total>]
MYCOMMAND_IMPL(makearr)
{
	DECLARE_CONTEXT(ctx);

	LocusFromStr(AtStr(args), ctx.locus());
	int n(-1);
	if (args.size() > 1)
		n = atoi(args.back());
	ProjModifier_t PJ(mrProject, *ctx.moduleFromLocus());
	if (!PJ.MakeArray(ctx.locus(), n))
	{
		PrintError() << "Failed to make array at VA=" << VA2STR(ctx.locus().va()) << std::endl;
		return 0;
	}
	//gui().GuiOnProjectModified();
	return 1;
}

//Synopsis: $editarr [@ADDR] [<total>]
MYCOMMAND_IMPL(editarr)
{
	DECLARE_CONTEXT(ctx);

	LocusFromStr(AtStr(args), ctx.locus());
	int n(-1);
	if (args.size() > 1)
		n = atoi(args.back());
	FieldPtr pField(ctx.locus().field0());
	if (!pField)
		return 0;

	TypePtr iStruc(ctx.locus().back().cont());
	Struc_t& rStruc(*iStruc->typeStruc());
	//FieldMapCIt itf(FieldIt(iStruc, ctx.addr(), nullptr, FieldIt_Overlap));
	ProjModifier_t PJ(mrProject, *ctx.moduleFromLocus());
	if (!PJ.MakeArray(ctx.locus(), n))
		return 0;
	//gui().GuiOnProjectModified();
	return 1;
}

//Command: makeunk
//Synopsis: $makeunk [@ADDR]
MYCOMMAND_IMPL(makeunk)
{
	DECLARE_CONTEXT(ctx);

	LocusFromStr(AtStr(args), ctx.locus());
	//queueAnlzBinEvent(TODO_UNDEFINE, args);
	//?mrMain.OnAnlzReady();
	ProjModifier_t PJ(mrProject, *ctx.moduleFromLocus());
	if (!PJ.makeUndefined(ctx.locus()))
		return 0;
	//gui().GuiOnProjectModified();
	return 1;
}

//Command: make_clone (mkcln)
//Synopsis: $make_clone [@ADDR]
MYCOMMAND_IMPL(make_clone)
{
	DECLARE_CONTEXT(ctx);

	LocusFromStr(AtStr(args), ctx.locus());

	ProjModifier_t MI(mrProject, *ctx.moduleFromLocus());
	if (!MI.MakeClone(ctx.locus()))
		return 0;

	return 1;
}

MYCOMMAND_IMPL(del)
{
	DECLARE_CONTEXT(ctx);
	LocusFromStr(AtStr(args), ctx.locus());

	if (!adjustPick(ctx.locus()))
		return 0;

	WriteLocker lock;
	if (!mrProject.deleteField(ctx.locus()))
		return 0;

	//gui().GuiOnProjectModified();
	return 1;
}

MYCOMMAND_IMPL(objinfo)
{
	DECLARE_CONTEXT(ctx);
	Obj_t* pObj(ctx.obj());
	if (pObj)
	{
		mrProject.PrintObjInfo(std::cout, pObj, ctx.moduleFromLocus());
	}
	return 1;
}

static bool parseLoc(const char* buf, ROWID& d, int& level)
{
	level = -1;

	char* pa = strdup(buf);
	char* p = strtok(pa, "(");
	if (p)
	{
		p = strtok(nullptr, ")");
		if (p)
			if (!sscanf(p, "%d", &level) == 1)
				level = 0;
	}

	if (pa[0] == '~')//rowid
	{
		unsigned int r;
		if (sscanf(&pa[1], "%x", &r) == 1)
		{
			d = r;
			return true;
		}
	}
	else if (pa[0] == '.')
	{
		ADDR r;
		if (sscanf(&pa[1], "%x", &r) == 1)
		{
			d = r;
			return true;
		}
	}

	ADDR v;
	if (sscanf(pa, "%x", &v) == 1)
	{
		d = v;
		return true;
	}

	return false;
}

MYCOMMAND_IMPL(typeslist)
{
	if (args.size() > 1)
	{
		//pSelf->lockRead(true);
		ROWID row;
		int level;
		if (parseLoc(args[1].c_str(), row, level))
		{
			//pSelf->PrintTypesList(pStruc, a);
		}
		//pSelf->lockRead(false);
	}
	else
	{
		assert(0);/*?
		typeMgr()->PrintTypesList(cout, 1);
		dc()->typeMgr()->PrintTypesList(cout, 1);*/
		std::cout.flush();
	}
	return 1;
}


//////////////////////////////////////////////
// Synopsis: $unloadraw
MYCOMMAND_IMPL(unloadraw)
{
	//	if (!ModuleOf()->typeModule()->unloadRaw())
		//	return 0;
		//gui().GuiOnProjectModified();
	mrProject.markDirty(DIRTY_GLOBALS);
	return 1;
}

//////////////////////////////////////////////
// Synopsis: $show <filename|$task> [N_lines_around]
//		$task  - current task's top
MYCOMMAND_IMPL(show)
{
	if (args.size() > 1)
	{
		if (args[1] == "$task")
		{
			mrProject.OnShowTaskTop();
		}
		else
		{
			bool bAsk(args.RemoveOpt("-ask"));
			MyString sAt(args.FindOptOrPfx("-@"));
			gui().GuiShowFile(args.back(), sAt, bAsk);
		}
	}
	return 1;
}


// Syntax: $listing [flags] [object name]
// flags:
//		-cols <column flags>
//		-c	: compact
MYCOMMAND_IMPL(listing)
{
	DECLARE_CONTEXT(ctx);

	unsigned cols(0);
	bool bCompact(false);
	TypePtr iScope(nullptr);
	if (args.size() > 1)
	{
		int i;
		MyArgs2 args2(args);
		if ((i = args2.Find("-cols")) >= 0)
		{
			args2.RemoveAt(i);
			if (i < (int)args2.size())
			{
				cols = atoi(args2[i].c_str());
				args2.RemoveAt(i);
			}
		}
		if (args2.RemoveOpt("-c"))
			bCompact = true;

		if (args2.size() > 1)
		{
			MyString s(args2[1]);

			//retrieve a binary scope to dump
			if (ctx.locus().module())//ModuleCur())
			{
				FieldPtr pField(FindFieldByNameInSegs3(ctx.locus().module(), s.c_str()));//ModuleCur()
				if (pField && pField->type())
					iScope = pField->type();
			}
			else if (s.endsWith(MODULE_SEP))
			{
				iScope = FindModuleEx(s, true);
			}
		}
	}
	if (iScope)
	{
		PrintListing(iScope, cols, bCompact);
	}
	else
	{
		const FoldersMap& m(mrProject.rootFolder().children());
		for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
		{
			CFolderRef rFolder(*i);
			if (!IsPhantomFolder(rFolder))
				PrintListing(ModuleOf(&rFolder), cols, bCompact);
		}
	}
	return 1;
}

class UserData : public DumpSymbolBase_t
{
public:
	Project_t& mrProject;
	CTypePtr mpModule;
	std::ofstream& ofs;
	int named, unnamed, errors;
	std::set<CFieldPtr> clones;//keep track of examined clones
	MyString sPfx, sSfx;
	bool bShared;
public:
	UserData(ModuleInfo_t& r, std::ofstream& _ofs)
		: DumpSymbolBase_t(r.GetDataSource()->pvt()),
		mrProject(r.Project()),
		mpModule(r.ModulePtr()),
		ofs(_ofs),
		named(0), unnamed(0), errors(0),
		bShared(false)
	{
	}
protected:
	virtual void setNameOff(OFF_t oSymbolName)
	{
		if (oSymbolName != OFF_NULL)
		{
			try {
				DumpSymbolBase_t::setName(oSymbolName);
				if (msSymbolName)
					named++;
			}
			catch (DataAccessFault_t&)
			{
				sPfx = "; <error>";
				errors++;
			}
		}
		else
		{
			CFieldPtr pField(nullptr);
			CTypePtr pFrontSeg(ProjectInfo_s::FindFrontSegIn(mpModule));
			ModuleInfo_t MI(mrProject, *mpModule);
			CTypePtr pSeg(MI.FindSegAt(pFrontSeg, mva, pFrontSeg->typeSeg()->affinity()));
			if (pSeg)
				pField = MI.Field(pSeg, mva, nullptr, ProjectInfo_t::FieldIt_Exact);
			/*if (pField && pField->isCloneMaster())
			{
				bShared = true;
				pField = nullptr;
				const ConflictFieldMap& m(pSeg->typeSeg()->conflictFields());
				for (ClonedFieldMapCIt j(m.lower_bound(mva)); j != m.end(); j++)
				{
					CFieldPtr pClone(VALUE(j));
					if (pClone->_key() != mva)
					{
						clones.clear();//reset
						break;
					}
					if (clones.find(pClone) != clones.end())
						continue;//considered before
					clones.insert(pClone);//register
					if (pClone->name())
					{
						pField = pClone;
						break;
					}
				}
			}*/
			if (pField && pField->name())
			{
				msSymbolName = MyString(pField->name()->c_str());
				named++;
			}
			else
			{
				msSymbolName = MyString("; ?");
				unnamed++;
			}
			sSfx = " NONAME";
		}
	}
	virtual void flush()
	{
		ModuleInfo_t MI(mrProject, *mpModule);
		ofs << sPfx << msSymbolName << " @ " << msSymbolName[1];// muOrdinal;
		if (!sSfx.isEmpty())
			ofs << sSfx;
		if (bShared)
			ofs << " ; CLASH(@" << MI.VA2STR(mva) << ")";
		ofs << std::endl;
		DumpSymbolBase_t::flush();
	}
};


// Syntax: dump_exports [flags] <path>
// flags:
//		-module <module>
MYCOMMAND_IMPL(dump_exports)
{
	MyArgs2 args2(args);
	MyString sModule;
	TypePtr pModule(nullptr);

	DECLARE_CONTEXT(ctx);

	if (args2.RemoveOptEx("-module", sModule))
		pModule = FindModuleEx(sModule, true);
	if (!pModule)
		pModule = ctx.locus().module();// ModuleCur();
	if (!pModule)
	{
		PrintError() << "No module to dump exports\n";
		return 0;
	}
	MyString sPath;
	if (args2.size() <= 1)
	{
		PrintError() << "File name is not specified\n";
		return 0;
	}
	sPath = args2.back();
	ModuleInfo_t MI(*this, *pModule);
	DataPtr pData(MI.GetDataSource());
	if (!pData)
	{
		PrintError() << "Module " << MI.ModuleName() << "has no data\n";
		return 0;
	}
	FRONT_t* pFRONT(nullptr);
	TypePtr pSeg(MI.FindFrontSeg());
	if (pSeg)
		pFRONT = FrontEnd(pSeg);
	if (!pFRONT)
	{
		PrintError() << "Module " << MI.ModuleName() << " has no exports\n";
		return 0;
	}
	std::ofstream ofs(sPath);
	if (!ofs.is_open())
	{
		PrintError() << "Could not open file :" << sPath << "\n";
		return 0;
	}

	ofs << "LIBRARY " << sModule << "\n\nEXPORTS\n";

	UserData user(MI, ofs);
	pFRONT->device(MI.GetDataSource())->dumpExports(&user, I_Front::DUMP_IE_ALL);

	PrintInfo() << user.named << " named exports dumped";
	if (user.unnamed > 0)
		PrintInfo() << " (" << user.unnamed << " unnamed)";
	PrintInfo() << " : " << sPath << "(" << MI.ModuleName() << ")" << std::endl;
	return 1;
}

// Command: print <options>
//	Options:
//		-secmap	: sections map
//		-segmap	: segments map
MYCOMMAND_IMPL(print)
{
	if (args.RemoveOpt("-sectm"))
		return COMMAND_dump_sections_map(this, args);

	if (args.RemoveOpt("-segm"))
		return COMMAND_dump_segments_map(this, args);

	/*if (argc > 2)
	{
		if (MyString(argv[1]) == MyString(">"))//re-direct to file?
		{
			MyString sRef(pSelf->path());
			if (sRef.empty())
				sRef = pSelf->binaryPath();
			MyPath f(argv[2], MyPath(sRef));
			if (f.Ext().empty())
				f.SetExt("out");
			ofstream ofs(f.Path());
			pSelf->Print(ofs, 0);
			cout << "Printed to file: " << f.Path() << endl;
			return;
		}
	}*/
	//pSelf->Print(cout, 0);

	HierPrinter a(*this);
	a.print(std::cout);
	return 1;
}


// 'debug': Start debugging current executable
// Synopsis: debug
MYCOMMAND_IMPL(debug)
{
	if (!startDebugger())
		return 0;
	return 1;
}

// 'step': Step (into) to next instruction
// Synopsis: step
MYCOMMAND_IMPL(step)
{
	if (!mrProject.debugger())
		return 0;
	mrProject.debugger()->resume(Resume_StepIn);
	return 1;
}

// 'next': Step (over) to the next instruction
// Synopsis: next
MYCOMMAND_IMPL(next)
{
	if (!mrProject.debugger())
		return 0;
	mrProject.debugger()->resume(Resume_StepOver);
	return 1;
}

// 'bp': Sets/Clears a breakpoint at context address
// Synopsis: bp
MYCOMMAND_IMPL(bp)
{
	DECLARE_CONTEXT(ctx);
	if (!mrProject.debugger())
		return 0;
	PVOID pv(Seg_t::ADDR2PV(ctx.locus().seg(), ctx.locus().back().addr()));
	if (!mrProject.debugger()->toggleBP(pv))
		return 0;

	gui().GuiOnProjectModified();//redraw view
	return 1;
}

// 'cont': Continue dubugging
// Synopsis: cont
MYCOMMAND_IMPL(cont)
{
	if (!mrProject.debugger())
		return 0;
	mrProject.debugger()->resume(Resume_Continue);
	return 1;
}

Probe_t* Project_t::NewLocus(const Locus_t& aLoc)
{
	return new Probe_t(aLoc);
}

// 'setcp': Set current position
// Synopsis: setcp <location>
/*MYCOMMAND_IMPL(setcp)
{
	DECLARE_CONTEXT(ctx);
	MyArgs2 a(args);
	if (a.size() > 1)
	{
		TypePtr iModule(nullptr);

		ZPath_t l(fixFileName(a[1], iModule));
		if (l.empty() || l.front() == l.back())
			return 0;

		if (!iModule)
		{
			Folder_t* pFolder(Files().FindFileByStem(l.root()));
			if (!pFolder)
				return 0;
			iModule = ModuleOf(pFolder);
		}
		MyString sAt(l.back());
		if (sAt.startsWith("@"))
			sAt = sAt.mid(1);

		ModuleInfo_t MI(*this, *iModule);

		DA_t da;
		if (!sAt.empty() && !(MI.Str2DA(sAt, &da.row) && LocusFromDA(da.row, ctx)))
			return 0;

		//ADDR va(ctx.addr());
		//ADDR va(strtoul(sVA.c_str(), nullptr, 16));
		//TypePtr iSeg(FindSegAt(iModule, va));
		//if (iSeg)
		{
			//DA_t da(iSeg->typeSeg()->viewOffs(iSeg, va), 0, 0);
			//Locus_t aLoc;
			//terminalFieldAtSeg(iModule, da, aLoc, iModule->typeModule()->rawBlock());
			mrProject.setLocus(mrProject.NewLocus(ctx));
			mrProject.markDirty(DIRTY_LOCUS_ADJUSTED);
		}
	}
	return 1;
}*/

// Synopsis: dcping <line>
/*MYCOMMAND_IMPL(dcping)
{
	MyArgs2 a(args);
	if (a.size() > 1)
	{
		int line(atoi(a[1].c_str()));
		if (line >= 0)
			guix().GuiOnSyncPanes(line);
	}
	return 1;
}*/

#if(0)
// 'setlocus': Setup a new locus
MYCOMMAND_IMPL(setlocus)
{
	DECLARE_CONTEXT(ctx);

	//?	assert(mrProject.hasContext());
			//mrProject.setContext(&ctx);

		/*MyArgs2 a(args);
		if (a.size() > 1)
		{
			Folder_t *pFolder(Files().FindFileByStem(a[1]));
			if (!pFolder)
				return 0;
			assert(ctx.folder() == pFolder);
			//mFiles.SelectFileByName(a[1]);
			ctx.setFolder(pFolder);
			//pctx->setBinaryIndex(ModuleOf(pFolder));//).findBynaryIndex(a[1]));
		}*/

#if(0)
	WriteLocker lock;

	mrProject.setLocus(pctx);
#endif
	return 1;
}
#endif

MYCOMMAND_IMPL(setnam)
{
	DECLARE_CONTEXT(ctx);
	Obj_t* pObj(ctx.obj());// mrProject.GetContextObj());
	if (!pObj)
		return -1;

	ProjModifier_t PJ(mrProject, *ctx.moduleFromLocus());
	FieldPtr pField(pObj->objField());
	if (pField)
	{
		//assert(!pField->isCloneMaster());

		//if (IsImpOrExp(pField))
		if (pField->isHardNamed())
		{
			assert(!pField->nameless());
			PrintError() << "Field '" << pField->name()->c_str() << "' cannot be renamed" << std::endl;
			return 0;
		}

		if (!PJ.SetFieldName(pField, args.size() > 1 ? args[1].c_str() : nullptr))
			return -2;
	}
	else
	{
		TypePtr iType(pObj->objTypeGlob());
		if (!PJ.SetTypeName(iType, args.size() > 1 ? args[1].c_str() : nullptr))
			return -2;
	}

	mrProject.markDirty(DIRTY_NAMES);
	//gui().GuiOnNameChanged();
	return 1;
}

// Command: dump_segments_map
//	Alias: segm
MYCOMMAND_IMPL(dump_segments_map)
{
	const FoldersMap& m(mrProject.rootFolder().children());
	for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
	{
		CTypePtr pModule(ModuleOf(&(*i)));
		if (!IsPhantomModule(pModule))
		{
			ModuleInfo_t MI(mrProject, *pModule);
			MI.DumpSegments(pModule, std::cout, 0);
		}
	}
	return 1;
}

// Command: dump_sections_map
// Alias: sctm
MYCOMMAND_IMPL(dump_sections_map)
{
	const FoldersMap& m(mrProject.rootFolder().children());
	for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
	{
		CTypePtr pModule(ModuleOf(&(*i)));
		if (!IsPhantomModule(pModule))
		{
			ModuleInfo_t MI(mrProject, *pModule);
			MI.DumpSections(std::cout);
		}
	}
	return 1;
}

// toggle collapsed
MYCOMMAND_IMPL(togl)
{
	DECLARE_CONTEXT(ctx);
	Locus_t& l(ctx.locus());//can be modified
	FieldPtr pField(l.field0());
	TypePtr iType(pField ? SkipModifier(pField->type()) : nullptr);
	if (!iType || !(iType->typeComplex() || iType->typeArray()))
		if (!l.empty())
			l.pop_back();
	if (l.empty())
		return 0;
	pField = l.field0();
	//	if (pField->type()->typeCode())
		//	return 0;
	if (pField->type()->typeBitset())//bitsets are invisible, so no point in doing this
	{
		l.pop_back();
		if (l.empty())
			return 0;
		pField = l.field0();
	}
	assert(pField);
	l.back().clearExtra();
	pField->setCollapsed(!pField->isCollapsed());
	//gui().GuiOnProjectModified();
	mrProject.markDirty(DIRTY_GLOBALS);
	mrProject.markDirty(DIRTY_LOCUS_ADJUSTED);
	return 1;
}

MYCOMMAND_IMPL(undname)
{
	DECLARE_CONTEXT(ctx);
	if (ctx.locus().empty())
		return 0;
	if (!(args.size() > 1))
		return 0;
	MyString s(args[1]);
	I_Front* pIFront(IFrontOf(ctx.locus().frontSeg()));
	if (!pIFront)
	{
		PrintError() << "undname: No appropriate context" << std::endl;
		return 0;
	}
	MyStream ss;
	pIFront->demangleName(s.c_str(), ss);
	ss.ReadString(s);
	if (s.empty())
		return 0;
	std::cout << s.c_str();
	return 1;
}

void Project_t::printMemInfo(std::ostream& os) const
{
	memMgr().print_stats(os);
}

MYCOMMAND_IMPL(meminfo)
{
	mrProject.printMemInfo(std::cout);

	return 1;
}

MYCOMMAND_IMPL(namfile)
{
	if (!(args.size() > 2))
		return 0;
	MyString src(args[1]);
	bool bFolder(src.endsWith("/"));
	MyString name(args[2]);
	if (bFolder && !name.endsWith("/"))
		name.append("/");
	Folder_t* pFolder(Files().FindFileByStem(src));
	if (!pFolder)
		return 0;
	if (!RenameSubItem(*pFolder, name, bFolder))
		return 0;

	gui().GuiOnFileListChanged();
	MyString s2(Files().relPath(pFolder));
	gui().GuiOnFileRenamed(src, s2);
	return 1;
}

//Synopsis: click <path>
//	Activates/opens a file in GUI
MYCOMMAND_IMPL(click)
{
	if (args.size() < 2)
		return 0;
	// a file may require an activation

	ZPath_t zPath(args[1]);//expected absolute?

	adcui::FolderTypeEnum iKind;
	FolderPtr pFolder(Files().FindFileByPath(zPath.toString(), iKind));
	if (!pFolder)
		return 0;

	TypePtr iModule(ProjectInfo_t::ModuleOf(pFolder));
	if (iModule)
	{
		Module_t& aModule(*iModule->typeModule());
		if (aModule.delayedFormat())
		{
			ModuleInfo_t MI(mrProject, *iModule);
			MI.FireDelayedFormat();
		}
	}

	gui().GuiShowFile(args[1], nullptr, false);
	return 1;
}


// Synopsis: mkscope [flags] [@ADDR] [-ref <addr_ref>]
// flags:
//		-s	: structure
//		-u	: union
//		-b	: bitset
MYCOMMAND_IMPL(mkscope)
{
	DECLARE_CONTEXT(ctx);

	LocusFromStr(AtStr(args), ctx.locus());

	ProjModifier_t PJ(mrProject, *ctx.moduleFromLocus());

	int i;
	if ((i = args.Find("-s")) >= 0)
	{
		if (PJ.makeScope(ctx.locus(), OBJID_TYPE_STRUC) == 0)
			return 1;
	}
	else if ((i = args.Find("-u")) >= 0)
	{
		if (PJ.makeScope(ctx.locus(), OBJID_TYPE__UNION) == 0)
			return 1;
	}
	else if ((i = args.Find("-b")) >= 0)
	{
		if (PJ.makeScope(ctx.locus(), OBJID_TYPE_BITSET) == 0)
			return 1;
	}

	//	if (PJ.makeScope(ctx, end_addr, iFunc != nullptr, depth) == 0)
		//	return 0;

	return 0;
}

// Synopsis: mkseg [-n <name>] [-ib <image_base>] [va [sz [tsz]]] [@ADDR]
// flags:
//		-ib 	: image base (uint64)
//		-n 	: name
//		va	: virtual base address
//		sz	: virtual size
//		tsz	: trace size (in parent)
MYCOMMAND_IMPL(mkseg)
{
	DECLARE_CONTEXT(ctx);

	LocusFromStr(AtStr(args), ctx.locus());

	if (ctx.locus().empty())
		return 0;

	const Frame_t& aTop(ctx.locus().back());
	if (!aTop.cont()->typeSeg())
		return 0;

	TypePtr iSelf(aTop.cont());
	Seg_t& rSelf(*iSelf->typeSeg());

	MyString name;

	int i(0);
	if ((i = args.Find("-n")) >= 0)
	{
		args.RemoveAt(i);
		if (!(i < (int)args.size()))
			return 0;
		name = args[i];
		args.RemoveAt(i);
	}

	value_t v;

	uint64_t imageBase(0);
	if ((i = args.Find("-ib")) >= 0)
	{
		args.RemoveAt(i);
		if (!(i < (int)args.size()))
			return 0;
		if (Str2Int(args[i], v))
			imageBase = v.ui64;
		args.RemoveAt(i);
	}

	ADDR base(0);
	if (args.size() > 1)
		if (Str2Int(args[1], v))
			base = v.ui32;

	unsigned size(0);
	if (args.size() > 2)
		if (Str2Int(args[2], v))
			size = v.ui32;

	unsigned tsize(0);
	if (args.size() > 3)
		if (Str2Int(args[3], v))
			tsize = v.ui32;

	ProjModifier_t MI(mrProject, *ctx.moduleFromLocus());
	TypePtr hRangeSet(MI.NewRangeSet(imageBase, nullptr));
	//TypePtr pRange(rSelf.newRange(hRangeSet, base, size));
	//if (!pRange)
		//return 0;

	if (!MI.AddSubRange(hRangeSet, base, size, MI.MakeSegment(aTop.cont(), aTop.addr(), tsize, nullptr, 0)))
	{
		assert(0);//delete range set!
		BinaryCleaner_t<> BC(MI);
		//?BC.DeleteRangeSeg(nullptr, pRange);
		return 0;
	}

	MI.UpdateViewGeometry2();
	return 1;
}

//struct ProbeEx_t;
MYCOMMAND_IMPL(close)
{
	if (mrMain.CloseProject())
	{
		if (args.context())
			args.context()->clear_all();
		return 1;
	}
	return 0;
}


