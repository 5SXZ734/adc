#include <conio.h>
#include <iostream>
#include <fstream>
#include <set>

#include "qx/MyArgs.h"
#include "qx/MyPath.h"
#include "qx/MyStringList.h"
//#include "qx/MyFileMgr.h"

using namespace std;

class MyMain
{
	bool bSafe;
	int iLastLine;
	ifstream ifs;
	ofstream ofs;

public: 
	MyMain(bool _bSafe)
		: bSafe(_bSafe),
		iLastLine(1)
	{
	}

	bool process(ifstream &patch_fs)
	{
		while (!patch_fs.eof())
		{
			MyString s;
			std::getline(patch_fs, s);

			if (s.isEmpty())
				continue;

			 if (s.startsWith("Only in"))
				 continue;
			
			//cout << s << endl;

			if (s.startsWith("diff "))
			{
				flush();

				MyStringList l(MyStringList::split(" ", s));
				MyString sRight(l.back());
				l.pop_back();
				MyString sLeft(l.back());

				MyPath fLeft(sLeft);
				ifs.open(fLeft.Path().c_str());
				if (!ifs.is_open())
					return false;

				MyPath fRight(sRight);
				
				if (bSafe)
				{
					MyString sExt(fRight.Ext() + ".new");
					fRight.SetExt(sExt.c_str());
				}

				ofs.open(fRight.Path().c_str());
				if (!ofs.is_open())
					return false;

				cout << fRight.Path() << endl;

				std::getline(patch_fs, s);
			}

			if (!patch(patch_fs, ifs, ofs, s))
				return false;
		}

		flush();//last file
		return true;
	}

	bool patch(ifstream &patch_fs, ifstream &ifs, ofstream &ofs, MyString s)
	{
		MyStringList l(MyStringList::split(" ", s));
		if (l.size() != 2)
			return false;
		s = l.front();
		int iMode(0);
		if (s.front() == 'd')
			iMode = -1;//delete
		else if (s.front() == 'a')
			iMode = 1;//add
		if (iMode == 0)
			return false;

		int iLine(atoi(s.c_str() + 1));
		int iNum(atoi(l.back().c_str()));

		if (iMode > 0)
			iLine++;

		MyString s2;
		while (iLastLine < iLine && !ifs.eof())
		{
			std::getline(ifs, s2);
			ofs << s2 << endl;
			iLastLine++;
		}

		if (iMode < 0)//delete : skip lines
		{
			for (int i(0); i < iNum; i++)
			{
				std::getline(ifs, s2);
				iLastLine++;
			}
			return true;
		}

		for (int i(0); i < iNum; i++)
		{
			std::getline(patch_fs, s2);
			ofs << s2 << endl;
			//iLastLine++;
		}

		return true;
	}

	void flush()
	{
		if (ifs.is_open())
		{
			//flush the rest of the file
			MyString s2;
			while (!ifs.eof())
			{
				std::getline(ifs, s2);
				ofs << s2;
				if (ifs.eof())
					break;
				ofs << endl;
				iLastLine++;
			}

			iLastLine = 1;
			ifs.close();
		}
		if (ofs.is_open())
			ofs.close();
	}
};


int main(int argc, char* argv[])
{
	MyArgs2 args(argc, argv);

	bool bSafe(false);
	if (args.RemoveOpt("-s"))
		bSafe = true;

	if (args.size() < 2)
	{
		cerr << "No patch file" << endl;
		return -1;
	}

	MyPath f(args[1]);
	ifstream ifs(f.Path().c_str());
	if (!ifs.is_open())
	{
		cerr << "Could not open file: " << f.Path() << endl;
		return -2;
	}

	//cout << f.Path() << endl;

	MyMain m(bSafe);
	if (!m.process(ifs))
	{
		cerr << "Got error during the patching" << endl;
		return -3;
	}
	
#ifdef _DEBUG
	_getch();
#endif
	return 0;
}


