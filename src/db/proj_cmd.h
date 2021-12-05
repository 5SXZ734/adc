#pragma once

#include "info_proj.h"

class ProjCmdServer_t : public ProjectInfo_t,
	public CMDServer_t
{
public:
	ProjCmdServer_t(CMDServerCommandMap &rcm, const ProjectInfo_t &rpi)
		: ProjectInfo_t(rpi),
		CMDServer_t(rcm)
	{
	}

private:
	friend class ProjCmdMap_t;
#define MYCOMMAND(name) \
	static int COMMAND_##name(CMDServer_t *, Cmd_t &); \
	int OnCommand_##name(Cmd_t &);

	MYCOMMAND(listing);
	MYCOMMAND(dump_exports);
	MYCOMMAND(preformat);
	MYCOMMAND(makeobj);
	MYCOMMAND(makefunc);
	MYCOMMAND(sweepfunc);
	MYCOMMAND(funcend);
	MYCOMMAND(makecode);
	MYCOMMAND(makethunk);
	MYCOMMAND(makedata);
	MYCOMMAND(makeint);
	MYCOMMAND(make_signed);
	MYCOMMAND(makereal);
	MYCOMMAND(makebit);
	MYCOMMAND(mkptr);
	MYCOMMAND(mkconst);
	MYCOMMAND(makeoff);
	MYCOMMAND(makearr);
	MYCOMMAND(editarr);
	MYCOMMAND(makeunk);
	MYCOMMAND(make_clone);
	MYCOMMAND(applytype);
	MYCOMMAND(instantiate);
	MYCOMMAND(toggle_exported);
	MYCOMMAND(toggle_imported);
	MYCOMMAND(del);
	MYCOMMAND(typeslist);
	MYCOMMAND(print);
	MYCOMMAND(unloadraw);
	MYCOMMAND(show);
	MYCOMMAND(debug);
	MYCOMMAND(step);
	MYCOMMAND(next);
	MYCOMMAND(bp);
	MYCOMMAND(cont);
	//MYCOMMAND(setcp);
	//MYCOMMAND(dcping);
	//MYCOMMAND(setlocus);
	MYCOMMAND(setnam);
	MYCOMMAND(dump_segments_map);
	MYCOMMAND(dump_sections_map);
	MYCOMMAND(togl);
	MYCOMMAND(undname);
	MYCOMMAND(meminfo);
	MYCOMMAND(namfile);
	MYCOMMAND(rsz);
	MYCOMMAND(mkscope);
	MYCOMMAND(mkseg);
	MYCOMMAND(objinfo);
	MYCOMMAND(mkgap);
	MYCOMMAND(click);
	MYCOMMAND(close);

#undef MYCOMMAND
};

class ProjCmdMap_t : public CMDServerCommandMap
{
public:
	ProjCmdMap_t()
	{
#define MYCOMMAND_REGISTER(arg)	RegisterCommand(#arg, ProjCmdServer_t::COMMAND_##arg);
#define MYCOMMAND_REGISTER2(arg,alias)	RegisterCommand(#arg, ProjCmdServer_t::COMMAND_##arg); \
	RegisterCommand(#alias, (CMDServerHandlerPtr)ProjCmdServer_t::COMMAND_##arg);

		MYCOMMAND_REGISTER(preformat);
		MYCOMMAND_REGISTER(makeobj);
		MYCOMMAND_REGISTER2(makefunc, mkfunc);
		MYCOMMAND_REGISTER(sweepfunc);
		MYCOMMAND_REGISTER(funcend);
		MYCOMMAND_REGISTER2(makecode, mkcode);
		MYCOMMAND_REGISTER2(makethunk, mkthk);
		MYCOMMAND_REGISTER(makedata);
		MYCOMMAND_REGISTER2(makeint, mkn);
		MYCOMMAND_REGISTER2(make_signed, mks);
		MYCOMMAND_REGISTER(makereal);
		MYCOMMAND_REGISTER(makebit);
		MYCOMMAND_REGISTER(mkptr);
		MYCOMMAND_REGISTER(mkconst);
		MYCOMMAND_REGISTER2(makeoff, mkoff);
		MYCOMMAND_REGISTER2(makearr, mkarr);
		MYCOMMAND_REGISTER(editarr);
		MYCOMMAND_REGISTER(makeunk);
		MYCOMMAND_REGISTER2(make_clone, mkcln);
		MYCOMMAND_REGISTER(applytype);
		MYCOMMAND_REGISTER2(instantiate, inst);
		MYCOMMAND_REGISTER2(toggle_exported, tgexp);
		MYCOMMAND_REGISTER2(toggle_imported, tgimp);
		MYCOMMAND_REGISTER(del);
		MYCOMMAND_REGISTER(typeslist);
		MYCOMMAND_REGISTER(listing);
		MYCOMMAND_REGISTER(dump_exports);
		MYCOMMAND_REGISTER(print);
		//MYCOMMAND_REGISTER(dumpfunc);
		MYCOMMAND_REGISTER(unloadraw);
		MYCOMMAND_REGISTER(show);
		MYCOMMAND_REGISTER(debug);
		MYCOMMAND_REGISTER(step);
		MYCOMMAND_REGISTER(next);
		MYCOMMAND_REGISTER(bp);
		MYCOMMAND_REGISTER(cont);
		//MYCOMMAND_REGISTER(setcp);
		//MYCOMMAND_REGISTER(dcping);
		//MYCOMMAND_REGISTER(setlocus);
		MYCOMMAND_REGISTER(setnam);
		MYCOMMAND_REGISTER2(dump_segments_map, segm);
		MYCOMMAND_REGISTER2(dump_sections_map, sctm);
		MYCOMMAND_REGISTER(togl);
		MYCOMMAND_REGISTER(undname);
		MYCOMMAND_REGISTER(meminfo);
		MYCOMMAND_REGISTER(namfile);
		MYCOMMAND_REGISTER(rsz);//resize
		MYCOMMAND_REGISTER(mkscope);
		MYCOMMAND_REGISTER(mkseg);
		MYCOMMAND_REGISTER(objinfo);
		MYCOMMAND_REGISTER(mkgap);
		MYCOMMAND_REGISTER(click);
		MYCOMMAND_REGISTER(close);
#undef MYCOMMAND_REGISTER
#undef MYCOMMAND_REGISTER2
	}
};




