#pragma once

#include "tiny_process/process.hpp"
#include <thread>
#include "qx/MyString.h"
#include "qx/MyStream.h"

class Compiler_t : public std::thread
{
	MyStream	mss;
public:
	Compiler_t();

	void compile(MyString sCommand, MyString sWorkDir);
	MyStreamBase &ss() { return mss; }
	bool isRunning() const { return get_id() != std::thread::id(); }

protected:
	void run(MyString workDir, MyString command);
};
