#include "qx/MyArgs.h"
#include "qx/MyFileMgr.h"

#undef MyFile

int main(int argc, char* argv[])
{
	MyArgs2 args(argc, argv);
	if (args.size() < 2)
		return -1;

	MyPath f = MyDirPath(args[1]);
	MyFile a(f);

	if (!a.EnsureDirExists())
		return -2;

	return 0;
}


