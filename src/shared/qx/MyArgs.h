#pragma once

#include <vector>
#include "MyString.h"

class MyArgs
{
public:
	int argc;
	char **argv;
	MyArgs(char *);
	~MyArgs();
	int Find(const char *);
	bool Insert(unsigned, char*);
};

class MyArgs2 : public std::vector<MyString>
{
public:
	MyArgs2(){}
	MyArgs2(const char *);
	MyArgs2(int, char **);
	~MyArgs2(){}
	void Init(const char *);
	void Add(const MyString &);
	MyString AsString(size_t from = 0) const;
	int Find(const char *) const;
	MyString FindOpt(const char *) const;
	MyString FindOptPfx(const char *) const;
	MyString FindOptOrPfx(const char *) const;
	MyString RemoveOptOrPfx(const char *);
	bool RemoveOpt(const char *);
	bool RemoveOptEx(const char *, MyString &);//option with argument
	bool RemoveAt(int i);
	bool InsertAt(int i, const char *);
private:
	size_t findOpt(const char *) const;
	size_t findOptPfx(const char *) const;

};

