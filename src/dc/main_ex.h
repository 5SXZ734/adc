#pragma once

#include "db/main.h"

class Compiler_t;
class ProjectEx_t;
class IGuiEx_t;

class Mainx_t : public Main_t
{
	Compiler_t *mpCompiler;
public:
	Mainx_t(int argc, char ** argv, const char *appName, const char *appCodeName, const char *appVersion, const char* companyName, const StartupInfo_t &si);
	~Mainx_t();

	const IGuiEx_t &guix() const;
	void StartCompiler(MyString workDir, MyString command);
	ProjectEx_t &projectx() const { return reinterpret_cast<ProjectEx_t &>(project()); }

protected:
	virtual void NewGlobalMemMgr();

	virtual My::IUnk *createGuiBroker(My::IGui *pIGui, bool bEnableOutputCapture);
	virtual Project_t *newProject(MemoryMgr_t &);
	virtual int	LoadProject(const char* fname);// , I_Context*);
	virtual bool OpenScript(const MyString &, bool);

	virtual void SaveToStream(std::ostream &, MyString path, unsigned flags);
	virtual void LoadFromStream(std::istream &, MyString path);

#define MYCOMMAND_OVERRIDE(name) \
	protected: virtual int OnCommand_##name(Cmd_t &);

	MYCOMMAND_OVERRIDE(save);
};



