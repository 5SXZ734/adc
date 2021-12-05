// fixeols.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <conio.h>
#include <iostream>
#include <fstream>
#include <set>

#include "qx/MyArgs.h"
#include "qx/MyDir.h"
#include "qx/MyStream.h"

using namespace std;

class MyMain
{
	MyPath mRefDir;
	set<MyString> mExtSet;
public: 
	MyMain(const MyPath &refDir)
		: mRefDir(refDir)
	{
	}
	void addExtension(const MyString &s)
	{
		mExtSet.insert(s);
	}
	const MyPath refDir(){ return mRefDir; }
	void process(const MyPath &f)
	{
		if (mExtSet.find(f.Ext()) == mExtSet.end())
			return;
		cout << f.RelPath(mRefDir);
		int count(0);
		ifstream ifs(f.Path().c_str());
		MyStream ss;
		while (!ifs.eof())
		{
			MyString s;
			std::getline(ifs, s);
			const char *pc = s.c_str();
			while (!s.empty() && s.at(s.length()-1) == 0xD)
			{
				s.resize(s.length()-1);
				count++;
			}
			ss.WriteString(s);
		}
		ifs.close();
		ss.Rewind(0);
		ofstream ofs(f.Path().c_str());
		if (ofs.is_open())
		{
			MyString s;
			for (int i(0); ss.ReadString(s); i++)
			{
				if (i > 0)
					ofs << endl;
				const char *pc(s.c_str());
				ofs << s;// << endl;
			}
			ofs.close();
		}
		if (count > 0)
			cout << MyString(" (") << count << MyString(" )");
		cout << endl;
	}
};

class DirIter : public MyDirIterator
{
	MyMain &mrMain;
public:
	DirIter(const MyPath &f, MyMain &rMain)//const MyPath &refDir = MyPath())
		: MyDirIterator(f),
		mrMain(rMain)
	{
	}
	void start()
	{
		readDirEntries(MyDirIterator::Dirs|MyDirIterator::Files|MyDirIterator::NoSymLinks);
	}
protected:
	virtual void OnEntry(const MyPath &f, int flags)
	{
		MyString a(f.Name());
		if (flags & Dirs)
		{
			if (a != "." && a != "..")
			{
				MyDirPath g(f.Path());
				DirIter it(g, mrMain);
				it.start();
			}
		}
		else
		{
			mrMain.process(f);
		}
	}

};

int main(int argc, char* argv[])
{
	MyString s;

	MyArgs2 args(argc, argv);
	if (args.size() > 1)
		s = MyDirPath(args[1]).Path();

	if (s.empty() && !My::__GetCwd(s))
		return -1;

	MyPath f(s);
	cout << f.Dir() << endl;

	MyMain m(f);
	m.addExtension("h");
	m.addExtension("c");
	m.addExtension("cpp");
	m.addExtension("hpp");
	m.addExtension("H");
	m.addExtension("C");
	m.addExtension("hxx");
	m.addExtension("cxx");
	
	DirIter it(f, m);
	it.start();

	_getch();
	return 0;
}


