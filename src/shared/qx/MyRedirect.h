#pragma once

#include <stdio.h>
#include <string>

class MyReopen
{
public:
	MyReopen(FILE *);
	bool Reopen(const std::string &, const char *);
	void Close();
	void Cleanup();
protected:
	FILE *mf;
	FILE *mf2;
	std::string ms2;
};

class MyRedirect
{
public:
	MyRedirect(FILE *);
	~MyRedirect();
	int Redirect(int);
	void Flush();
	int fd(){ return mfd; }
	FILE *f(){ return mf; }
private:
	FILE *mf;
	int mfd;
};

class TRedirect
{
	FILE *mpf;
	int mFileDesc;
	std::string mLastFile;

public:
	TRedirect(FILE *);
	~TRedirect();
	bool toFile(const char *fileName);
	bool toTerminal();
	void setLastFile(const char *s){ mLastFile = s?s:""; }
};



