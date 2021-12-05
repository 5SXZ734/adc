
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

//#ifdef WIN32
//#include <io.h>
//#else
#include "unistd.h"
//#endif
#include "MyRedirect.h"
#include "MyFileMgr.h"

// Under windows there is no stdout and stderr when using 
// /subsystem:window linker option.
// We must then open files to redirect stdout and stderr 


MyReopen::MyReopen(FILE *f)
: mf(f),
mf2(nullptr)
{
	assert(mf);
}

bool MyReopen::Reopen(const std::string &s, const char *mode)
{
	ms2 = s;
	if (!mode)
		mode = "w";

	mf2 = freopen(ms2.c_str(), mode, mf);
	if (!mf2)
	{
		return false;
	}
	return true;
}

void MyReopen::Cleanup()
{
	if (mf2)
	{
		fflush(mf2);
		fclose(mf2);
		mf2 = nullptr;
		if (!ms2.empty())
			unlink(ms2.c_str());
		ms2.resize(0);
	}
}

void MyReopen::Close()
{
	close(fileno(mf));
}



/////////////////////////////////////////////////////

MyRedirect::MyRedirect(FILE *f)
: mf(f),
mfd(-1)
{
}

MyRedirect::~MyRedirect()
{
	if (mfd != -1)
		::close(mfd);
}

int MyRedirect::Redirect(int Kb)
{
	int fdpipe[2];

	// open a pipe
#ifdef WIN32
	if(_pipe(fdpipe, Kb*1024, O_BINARY) == -1)
#else
	(void)Kb;
	if(pipe(fdpipe) == -1)
#endif
	{
		//perror("Unable to open pipe for stderr");
		return -1;
	}

	// force stderr to refer to the write end of our pipe
	if ( dup2(fdpipe[1], fileno(mf)) == -1 )
	{
		//perror("Dup2 failed for stderr pipe");
		return -2;
	}

	// close original write end of pipe
	close(fdpipe[1]);

	mfd = fdpipe[0];
	return 0;
}

// closing stdout (actually the rite end of the pipe) causes the
// blocking read() call in the thread to return
void MyRedirect::Flush()
{
	int eof = 0;
	::fwrite(&eof, sizeof(eof), 1, mf);
	::fflush(mf);
}


////////////////////////////////////////////////
// TRedirect

TRedirect::TRedirect(FILE *pf)
: mpf(pf),
mFileDesc(-1)
{
}

TRedirect::~TRedirect()
{
	toTerminal();
}

bool TRedirect::toFile(const char *fileName)
{
	if (mFileDesc == -1)
	{
		bool bAppend = false;
		if (!fileName)
		{
			fileName = mLastFile.c_str();
			bAppend = true;
		}

		if (strlen(fileName))
		{
			//Save position of current standard output
//?			fgetpos(mpf, &mTermPos);
			mFileDesc = dup(fileno(mpf));
			MyFileMgr::Instance()->FileReopen(fileName, bAppend?"a":"w", mpf);
			mLastFile = fileName;
			return true;
		}
	}
	return false;
}

bool TRedirect::toTerminal()
{
	if (mFileDesc != -1)
	{
		//Flush mpf so any buffered messages are delivered
		fflush(mpf);
		//Close file and restore standard output to mpf - which should be the terminal
		dup2(mFileDesc, fileno(mpf));
		close(mFileDesc);
		mFileDesc = -1;
		clearerr(mpf);
		//fsetpos(mpf, &mTermPos);
		return true;
	}
	return false;
}