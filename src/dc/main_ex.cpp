#include "main_ex.h"
#include "shared/data_source.h"
#include "files_ex.h"
#include "path.h"
#include "compile.h"
#include "op.h"
#include "proj_ex.h"
#include "save_ex.h"
#include "savex_impl.h"
#include "probe_ex.h"
#include "clean_ex.h"
#include "ui_main_ex.h"

static IGuiEx_t guiProxy;

Mainx_t::Mainx_t(int argc, char ** argv, const char *appName, const char *appCodeName, const char *appVersion, const char* companyName, const StartupInfo_t &si)
	: Main_t(argc, argv, appName, appCodeName, appVersion, companyName, si)
{
	setGui(&guiProxy);
	mpCompiler = new Compiler_t;
}

Mainx_t::~Mainx_t()
{
	delete mpCompiler;
}

const IGuiEx_t &Mainx_t::guix() const
{
	return dynamic_cast<const IGuiEx_t &>(gui());
}

void Mainx_t::NewGlobalMemMgr()
{
	assert(!mpMemoryMgr);
	mpMemoryMgr = new MemoryMgr2_t();
#ifdef _DEBUG
	mpMemoryMgr->mName = "$GLOBAL";
#endif

}

My::IUnk *Mainx_t::createGuiBroker(My::IGui *pIGui, bool bEnableOutputCapture)
{
	GuiEx_t *pGui(CreateADCGuiBroker(pIGui, *this, bEnableOutputCapture));
	setGui(pGui);
	return pGui->core();
}

bool Mainx_t::OpenScript(const MyString &path, bool bAbortIf)
{
	if (!Main_t::OpenScript(path, bAbortIf))
		return false;
	ProbeEx_t *ctx(new ProbeEx_t);
	mpScript->setContext(ctx);
	ctx->Release();
	return true;
}

void Mainx_t::StartCompiler(MyString wdir, MyString command)
{
	if (mpCompiler->isRunning())
		return;
	//MyString wdir("D:\\andreys\\test\\cl");//QxDir(argv()[0]).Dir(true)
	//mpCompilerProcess = new Q_Process("\"C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\bin\\cl.exe\" /c src\\file1.cxx /Itmp", wdir);
	mpCompiler->compile(command, wdir);
}

Project_t *Mainx_t::newProject(MemoryMgr_t &rMemMgr)
{
	return new ProjectEx_t(*this, rMemMgr);
}

int Mainx_t::LoadProject(const char* fname)// , I_Context* pICtx)
{
	int ret(Main_t::LoadProject(fname));// , pICtx));
//	if (projectx().DcRef())
		guix().GuiOnDcNew();
	return ret;
}

void Mainx_t::SaveToStream(std::ostream &os, MyString path, unsigned flags)
{
#if(NEW_MEMMGR)
	assert(mpMemoryMgr);

	MemmoryAccessorGG_t<GlobalSerializerEx_t> SAVELOAD(SR_Saving, *mpMemoryMgr, *mpProject, path);

	GlobalSerializerEx_t::SaveMode mode(GlobalSerializerEx_t::SaveMode_None);
	if (flags == 1)
		mode = GlobalSerializerEx_t::SaveMode_Solid;
	else if (flags == 2)
		mode = GlobalSerializerEx_t::SaveMode_Dispersed;
	SAVELOAD.SaveEx(os, mode);
#else
	PrintError() << "SAVE not supported" << std::endl;
#endif
}

void Mainx_t::LoadFromStream(std::istream &is, MyString path)
{
#if(NEW_MEMMGR)

	assert(!mpProject);
	mpProject = newProject(*mpMemoryMgr);

	MemmoryAccessorGG_t<GlobalSerializerEx_t> SAVELOAD(SR_Loading, *mpMemoryMgr, *mpProject, path);

//	mpProject->setTypeRef(mpMemoryMgr->NewTypeRef(mpProject));

	SAVELOAD.LoadEx(is, true);//enable 'quick' to force postponed mode
#else
	PrintError() << "LOAD not supported" << std::endl;
#endif
}

#define MYCOMMAND_OVERRIDE_IMPL(name) \
	int Mainx_t::OnCommand_##name(Cmd_t &args)
#define MYCOMMAND_BASE_CALL(name) \
	Main_t::OnCommand_##name(args)

//Synopsis: save [[-d] path]
//		Save project to file
// Options:
//		-s : solid mode
//		-d : dispersed mode (save contents of source files in secondary files (.dc), the infrastructure yet saved in master file (.adc));
//				otherwise, a solid mode by default (save whole thing into a single .adc file);
//				If no path specified, take one from existing project.
//				If the project was never saved - ask the user (recoil)
MYCOMMAND_OVERRIDE_IMPL(save)
{
	MyArgs2 args2(args);

	unsigned flags(0);

	if (args2.RemoveOpt("-s"))
		flags = 1;//solid

	//projectx().setDisperseMode(args2.RemoveOpt("-d"));
	if (args2.RemoveOpt("-d"))
		flags = 2;//dispersed

	if (args2.size() > 2)
		return -1;//incorrect command syntax

	MyString path;
	if (args2.size() > 1)
		path = args2[1];

	MainReadLocker lock(this);

	if (!validateProjectPath(path))
		return 0;

	if (!SaveToFile(path, flags))
		return 0;

	gui().GuiOnProjectSaved();

	return 1;//MYCOMMAND_BASE_CALL(save);
}

