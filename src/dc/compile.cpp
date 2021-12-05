#include "compile.h"
#include "dump_func.h"
#include <iostream>
#include "qx/MyArgs.h"
#include "qx/MyPath.h"


Compiler_t::Compiler_t()
{
}

void Compiler_t::compile(MyString sCommand, MyString sWorkDir)
{
	//setCommunication(Stdout | Stderr);

	MyString s(sCommand);

	MyDirPath aWorkDir = MyDirPath(sWorkDir);

	MyArgs2 args(s);
	if (args[0].find(' ') != std::string::npos)//contains spaces?
		args[0] = "\"" + args[0] + "\"";

	s = args.AsString();
	sWorkDir = aWorkDir.Dir(true);

	//setNotifyOnExit(true);

	std::thread& self(*this);
	self = std::thread(&Compiler_t::run, std::ref(*this), sWorkDir, s);//start the thread
	detach();
}

void Compiler_t::run(MyString workDir, MyString command)
{
	MyStream& ss(mss);
	TinyProcessLib::Process proc(command, workDir,
		[&ss](const char* bytes, size_t n) {
			ss.Write((void*)bytes, int(n));
		}, [&ss](const char* bytes, size_t n) {
			ss.Write((void*)bytes, int(n));
			if (bytes[n - 1] != '\n')
				ss.WriteChar('\n');
		}
		);

	if (proc.get_id() == 0)
	{
		ss.WriteString("compile error\n");
	}
	else
	{
		int exitStatus(proc.get_exit_status());
	}
	mss.WriteStdStream(std::cout);
	std::cout.flush();
}

