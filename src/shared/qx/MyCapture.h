#pragma once

#include <stdio.h>
#include <string>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "MyStream.h"

class MyCapture
{
public:
	MyCapture(FILE **);
	~MyCapture();

	void Begin();
	bool Read(MyStreamBase &);
	bool End();
	void Close();

private:
	struct file_t
	{
		FILE *f;
		int	fd;
		file_t(FILE *_f, int _fd) : f(_f), fd(_fd) {}
	};

	enum PIPES { READ, WRITE };
	std::list<file_t> m_files;
	int m_pipe[2];
	bool m_init;
};

class MyCaptureThread : public std::thread
{
public:
	MyCaptureThread(FILE **, MyStreamBase &);
	virtual ~MyCaptureThread();

	void Start();
	void Stop();

protected:
	virtual void run();

	MyCaptureThread & operator = (const MyCaptureThread &){ return *this; }

private:
	MyStreamBase	&mss;
	MyCapture mCapture;
	bool mbStop;
	std::mutex	mMx;
	std::condition_variable mWC;
};

class MyCaptureStream : public MyStream
{
public:
	MyCaptureStream(int, const char *, unsigned = 0);
	virtual ~MyCaptureStream();
	bool Flush(MyStreamBase &, FILE * = nullptr, unsigned = 0, int = 0);
	void Restart(int);
	int WinId(){ return mWinId; }
	void Stop();
	void Pause();
	void Continue();
protected:
	virtual void OutputReady(int, bool) = 0;
protected:
	virtual unsigned Write(void *p, unsigned sz);
private:
	void startCapture();
	void stopCapture(bool bExit);

private:
	int mStreamId;
	int mWinId;
	std::string mName;
	MyCaptureThread *mpCapture;
	std::mutex	mMx;
	std::mutex	mWC;
	bool	mbDirty;
};


