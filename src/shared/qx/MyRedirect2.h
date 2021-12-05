#pragma once

#include <list>

#include "IUnk.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include "MyStream.h"

class MyStreamBase;

namespace My
{

	class XSink : public IUnk
	{
	public:
		XSink();
		virtual ~XSink();
		virtual bool Redirect(int);
		//virtual bool Restore(int fd);
		virtual void Pause(){}
	};

	class XFileSink : public XSink
	{
		FILE *mf;
	public:
		XFileSink(const char *fname, const char *mode);
		XFileSink(FILE *from);
		virtual ~XFileSink();
	protected:
		virtual bool Redirect(int fd);
	};

	class XPipe : public IUnk
	{
		int m_pipe[2];
	public:
		enum PIPES { READ, WRITE };
		XPipe(int kB);
		virtual ~XPipe();
		bool redirectWrite(int fd, int &);
		bool Read(MyStreamBase &ss);
		void unblock();
	};

	class XPipeReader : public std::thread
	{
		MyStreamBase &m_ss;
		std::mutex mMx;
		std::condition_variable mWC;
		int	mkB;
		int mfd;
		int	mhWrite;
	public:
		XPipeReader(MyStreamBase &ss, int = 64);
		~XPipeReader();
		void Start(int);
		void Stop();
		void Pause();
	protected:
		void run(void);
	};

	class XPipeSink : public XSink
	{
		XPipeReader &mrReader;
	public:
		XPipeSink(XPipeReader &);
		virtual ~XPipeSink();
		XPipeSink &operator = (const XPipeSink &){ return *this; }
	protected:
		virtual bool Redirect(int);
		virtual void Pause(){ mrReader.Pause(); }
	};

	class XRedirect;
	class XBuddySink : public XSink
	{
		XRedirect &mSelf;
		XRedirect &mBuddy;
	public:
		XBuddySink(XRedirect &, XRedirect &);
		virtual ~XBuddySink();
		XBuddySink &operator = (const XBuddySink &){ return *this; }
		bool sync();
	protected:
		virtual bool Redirect(int);
	};

	class XRedirect
	{
		friend class XBuddySink;
	public:
		XRedirect(FILE *, bool recover = false);
		~XRedirect();
		bool toFile(const char *fname, bool append = false);
		bool toPipe(XPipeReader &);
		bool toBuddy(XRedirect &);
		bool Restore(int);
		int desc();
	private:
		bool toNul();
		bool Push(XSink *);
		bool Pop();
		bool disengage();
		bool setBuddy(XRedirect *);
		XSink *sink();
		FILE *file(){ return mf0; }
		bool sync();
	private:
		FILE *mf0;
		int	mfdOld;
		std::list<XSink *>	mSinks;
		XRedirect *mpBuddy;
		XBuddySink *mpBuddySink;
	};

	class XCaptureStream : public MyStream
	{
		std::string mHeader;
		XPipeReader *mpPipeReader;
		std::mutex	mMx;
		std::mutex	mWC;
		bool	mbDirty;
	public:
		XCaptureStream(const char *, int, unsigned = 0);
		virtual ~XCaptureStream();
		bool Flush(MyStreamBase &, FILE * = nullptr, unsigned = 0, int = 0);
		void Start(int fd){ mpPipeReader->Start(fd); }
		void Redirect(const char *);
		//int WinId(){ return mWinId; }
		const std::string &header(){ return mHeader; }
		XPipeReader &pipeReader(){ return *mpPipeReader; }
	protected:
		virtual void OutputReady(bool) = 0;
	protected:
		virtual unsigned Write(void *p, unsigned sz);
	};


}//namespace My


