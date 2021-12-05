#pragma once

#include "info_file.h"

class FileInfoCmdServer_t : public FileInfo_t,
	public CMDServer_t
{
public:
	FileInfoCmdServer_t(CMDServerCommandMap &, const FileInfo_t &);

private:
	friend class ProjExCmdServer_t;

#define MYCOMMAND(name) \
	static int COMMAND_##name(CMDServer_t *, Cmd_t &); \
	int OnCommand_##name(Cmd_t &);

	//MYCOMMAND(setcurop);
	MYCOMMAND(toggle_root);
	MYCOMMAND(toggle_if);
	MYCOMMAND(toggle_else);
	MYCOMMAND(toggle_switch);
	MYCOMMAND(toggle_while);
	MYCOMMAND(toggle_for);
	MYCOMMAND(do_logic);
	MYCOMMAND(undo_logic);
	MYCOMMAND(bind);
	MYCOMMAND(unbind);
	MYCOMMAND(acquire);
//	MYCOMMAND(acquire_constant);
	MYCOMMAND(setnam);
	MYCOMMAND(dumpexpr);
	MYCOMMAND(meminfo);
	MYCOMMAND(del);
	MYCOMMAND(compile);
	MYCOMMAND(analyze);
	MYCOMMAND(decompile);
	MYCOMMAND(dcfile);
	MYCOMMAND(xpnd);
	MYCOMMAND(clps);
	MYCOMMAND(flip);
	MYCOMMAND(reconclsh);
	MYCOMMAND(addheir);
	MYCOMMAND(rmheir);
	MYCOMMAND(togvptr);
	MYCOMMAND(togthisptr);
	MYCOMMAND(toggle_virtual);
	//conversion
	MYCOMMAND(convns);
	MYCOMMAND(convcls);
	MYCOMMAND(convstr);
	MYCOMMAND(convenu);
	MYCOMMAND(convuni);
	//MYCOMMAND(newfile);
	MYCOMMAND(draftest);
	MYCOMMAND(load_stubs);
	MYCOMMAND(cut);
	MYCOMMAND(paste);
	MYCOMMAND(uncut);

#undef MYCOMMAND

#define MYCOMMAND_REGISTER(arg)	RegisterCommand(#arg, FileInfoCmdServer_t::COMMAND_##arg);
#define MYCOMMAND_REGISTER2(arg,alias)	RegisterCommand(#arg, FileInfoCmdServer_t::COMMAND_##arg); \
	RegisterCommand(#alias, (CMDServerHandlerPtr)FileInfoCmdServer_t::COMMAND_##arg);

	class CommandMap_t : public CMDServerCommandMap
	{
	public:
		CommandMap_t()
		{
			MYCOMMAND_REGISTER(toggle_root);
			MYCOMMAND_REGISTER(toggle_if);
			MYCOMMAND_REGISTER(toggle_else);
			MYCOMMAND_REGISTER(toggle_switch);
			MYCOMMAND_REGISTER(toggle_while);
			MYCOMMAND_REGISTER(toggle_for);
			MYCOMMAND_REGISTER(do_logic);
			MYCOMMAND_REGISTER(undo_logic);
			MYCOMMAND_REGISTER(bind);
			MYCOMMAND_REGISTER(unbind);
			MYCOMMAND_REGISTER2(acquire, acq);
//			MYCOMMAND_REGISTER2(acquire_constant, acqcnt);
			MYCOMMAND_REGISTER(setnam);//DC must have it's own set_name ha
			MYCOMMAND_REGISTER(dumpexpr);
			//MYCOMMAND_REGISTER(setcurop);
			MYCOMMAND_REGISTER(meminfo);
			MYCOMMAND_REGISTER(del);
			MYCOMMAND_REGISTER(compile);
			MYCOMMAND_REGISTER2(analyze, anlz);
			MYCOMMAND_REGISTER2(decompile, dc);
			MYCOMMAND_REGISTER(dcfile);
			MYCOMMAND_REGISTER(xpnd);
			MYCOMMAND_REGISTER(clps);
			MYCOMMAND_REGISTER(flip);
			MYCOMMAND_REGISTER(reconclsh);
			MYCOMMAND_REGISTER(addheir);
			MYCOMMAND_REGISTER(rmheir);
			MYCOMMAND_REGISTER(togvptr)
			MYCOMMAND_REGISTER(togthisptr)
			MYCOMMAND_REGISTER2(toggle_virtual, togv)
			//conversion
			MYCOMMAND_REGISTER(convns);
			MYCOMMAND_REGISTER(convcls);
			MYCOMMAND_REGISTER(convstr);
			MYCOMMAND_REGISTER(convenu);
			MYCOMMAND_REGISTER(convuni);
			//MYCOMMAND_REGISTER(newfile);
			MYCOMMAND_REGISTER2(draftest, dt);
			MYCOMMAND_REGISTER2(load_stubs, ldstb);
			MYCOMMAND_REGISTER(cut);
			MYCOMMAND_REGISTER(paste);
			MYCOMMAND_REGISTER(uncut);
		}
	};
#undef MYCOMMAND_REGISTER

};


#define DECLARE_DC_CONTEXT(ctx) \
	ProbeEx_t *pctx(dynamic_cast<ProbeEx_t *>(args.context())); \
	if (!pctx) { \
		/*fprintf(STDERR, "%s: No appropriate context\n", args[0].c_str());*/ \
		return -1; } \
	ProbeEx_t &ctx(*pctx);



