#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "MyFileMgr.h"
#include "MyRedirect2.h"
#include "unistd.h"

#ifdef WIN32
    #define DEVNULL "nul"
#else
    #define DEVNULL "/dev/null"
#endif

#undef fopen
#define fopen	MyFileMgr::Instance()->FileOpen

namespace My
{

	//////////////////////////////////
	// XSink

	XSink::XSink()
	{
	}

	XSink::~XSink()
	{
	}

	bool XSink::Redirect(int)
	{
		return true;
	}

	//////////////////////////////////
	// XFileSink

	XFileSink::XFileSink(const char *fname, const char *mode)
		: mf(nullptr)
	{
		assert(fname);
		mf = fopen(fname, mode);
		if (!mf)
		{
			throw -1;
		}
	}

	XFileSink::XFileSink(FILE *from)
		: mf(nullptr)
	{
		mf = freopen(DEVNULL, "w", from);
		if (!mf)
		{
			throw -1;
		}
	}

	XFileSink::~XFileSink()
	{
		if (mf)
			fclose(mf);
	}

	bool XFileSink::Redirect(int fd2)
	{
		if (!mf || (fd2 == -1))
			return false;
		int fd1(fileno(mf));
		if (-1 == dup2(fd1, fd2))
			return false;
		return true;
	}

	//////////////////////////////////
	// XPipe

	XPipe::XPipe(int kB)
	{
		m_pipe[READ] = -1;
		m_pipe[WRITE] = -1;
#ifdef WIN32
		if (_pipe(m_pipe, kB*1024, O_BINARY) == -1)
#else
		if(pipe(m_pipe) == -1)
#endif
		{
			throw -1;
		}
	}

	bool XPipe::redirectWrite(int fd, int &hWrite)
	{
		if (m_pipe[WRITE] == -1)
			return false;
		if (dup2(m_pipe[WRITE], fd) == -1)
			return false;
		hWrite = dup(m_pipe[WRITE]);
		close(m_pipe[WRITE]);
		m_pipe[WRITE] = -1;
		return true;
	}

	bool XPipe::Read(MyStreamBase &ss)
	{
		bool bRet(true);
		char buf[1024];
		int bytesRead(::read(m_pipe[READ], buf, sizeof(buf)));//&(*buf.begin()), bufSize);
		if (bytesRead > 0)
		{
			if (!buf[bytesRead-1])
			{
				bytesRead--;
				bRet = false;
			}
			if (bytesRead > 0)
				ss.Write(buf, bytesRead);
		}
		if (!bytesRead)
			bRet = false;
		return bRet;
	}

	void XPipe::unblock()
	{
		//?assert(0);
		char eof = 0;
		//size_t bytes(::fwrite(&eof, sizeof(eof), 1, file.f));
		//fflush(file.f);
//		size_t bytes(::write(m_pipe[WRITE], &eof, 1));
//		(void)bytes;
	}

	XPipe::~XPipe()
	{
		if (m_pipe[WRITE] != -1)
			close(m_pipe[WRITE]);
		if (m_pipe[READ] != -1)
			close(m_pipe[READ]);
	}

	//////////////////////////////////
	// XPipeReader

	XPipeReader::XPipeReader(MyStreamBase &ss, int kB)
#ifdef WIN32
		://QxThread(),
#else
		://QxThread(64*1024),
#endif // WIN32
		m_ss(ss),
		mkB(kB),
		mfd(-1),
		mhWrite(-1)
	{
	}

	XPipeReader::~XPipeReader()
	{
		//assert(!running());
		assert(get_id() == std::thread::id());//not running
	}

	void XPipeReader::Start(int fd)
	{
		if (get_id() != std::thread::id())//is running?
			join();

		std::unique_lock<std::mutex> lock(mMx);
		mfd = fd;
		//start();
		std::thread& self(*this);
		self = std::thread(&XPipeReader::run, std::ref(*this));
		mWC.wait(lock);
	}

	void XPipeReader::Stop()
	{
		char eof = 0;
		//size_t bytes(::fwrite(&eof, sizeof(eof), 1, file.f));
		//fflush(file.f);
		if (mhWrite >= 0)
		{
			size_t bytes(::write(mhWrite, &eof, 1));
			(void)bytes;
		}

		//mpPipe->unblock();
		//if (running())
		if (get_id() != std::thread::id())//is running?
		{
			//wait();
			join();
//			if (!wait(3000)) 
//				terminate();
		}
		mfd = -1;
	}

	void XPipeReader::Pause()
	{
		if (mhWrite != -1)
		{
			::close(mhWrite);
			mhWrite = -1;
		}
	}

	void XPipeReader::run()
	{
		mMx.lock();

		XPipe *pPipe(new XPipe(mkB));
		if (pPipe->redirectWrite(mfd, mhWrite))
		{
			mWC.notify_one();
			mMx.unlock();

			while (pPipe->Read(m_ss))
			{
			}
		}
		else
		{
			mWC.notify_one();
			mMx.unlock();
		}

		if (mhWrite != -1)
		{
			::close(mhWrite);
			mhWrite = -1;
		}
		delete pPipe;
	}

	//////////////////////////////////
	// XPipeSink

	XPipeSink::XPipeSink(XPipeReader &r)
		: mrReader(r)
	{
	}
	XPipeSink::~XPipeSink()
	{
		mrReader.Stop();
	}

	bool XPipeSink::Redirect(int fd)
	{
		mrReader.Start(fd);
		return true;
	}

	//////////////////////////////////
	// XBuddySink

	XBuddySink::XBuddySink(XRedirect &self, XRedirect &buddy)
		: mSelf(self),
		mBuddy(buddy)
	{
		if (!buddy.setBuddy(&self))
		{
			throw -1;
		}
	}

	XBuddySink::~XBuddySink()
	{
		mBuddy.setBuddy(nullptr);
	}

	bool XBuddySink::sync()
	{
		fflush(mSelf.file());
		return Redirect(mSelf.desc());
	}

	bool XBuddySink::Redirect(int fd2)
	{
		int fd1(mBuddy.desc());
		if (-1 == dup2(fd1, fd2))
			return false;
		return true;
	}

	//////////////////////////////////
	// XRedirect

	XRedirect::XRedirect(FILE *f0, bool recover)
		: mf0(f0),
		mfdOld(-1),
		mpBuddy(nullptr),
		mpBuddySink(nullptr)
	{
		assert(f0);
		int fd(-1);
		if (recover)
			fd = fileno(mf0);
		if (fd != -1)
		{
			mfdOld = dup(fd);
		}
		/*else
		{
			toNul();
		}*/
	}

	XRedirect::~XRedirect()
	{
		while (Pop());
		if (mpBuddy)
		{
			mpBuddy->disengage();
		}
		if (mfdOld != -1)
			close(mfdOld);
		clearerr(mf0);
		//setbuf(mf0, nullptr);
	}

	bool XRedirect::disengage()
	{
		if (!mpBuddySink)
			return false;

		if (mpBuddySink == sink())
			return Pop();

		std::list<XSink *>::iterator it(mSinks.begin());
		for (; it != mSinks.end(); it++)
		{
			if (*it == mpBuddySink)
				break;
		}
		
		if (it != mSinks.end())
			mSinks.erase(it);

		delete mpBuddySink;
		mpBuddySink = nullptr;
		return true;
	}

	bool XRedirect::setBuddy(XRedirect *pBuddy)
	{
		if (pBuddy && mpBuddy)
			return false;
		mpBuddy = pBuddy;
		return true;
	}

	bool XRedirect::Push(XSink *pSink)
	{
		if (!pSink)
			return false;
		fflush(mf0);
		if (mSinks.empty())
		{
			toNul();
		}
		else
		{
			XSink *pSink2(mSinks.front());
			pSink2->Pause();
		}
		mSinks.push_front(pSink);
		if (pSink->Redirect(desc()))
		{
			if (mpBuddy)
				mpBuddy->sync();
		}
		return true;
	}

	bool XRedirect::Pop()
	{
		if (mSinks.empty())
			return false;
		XSink *pOldSink(sink());
		mSinks.pop_front();
		int fd(fileno(mf0));
		fflush(mf0);
		int ret;
		if (sink())
		{
			sink()->Redirect(fd);
		}
		else
		{
			if (mfdOld != -1)
				ret = dup2(mfdOld, desc());
			else
				toNul();
		}
		if (pOldSink == mpBuddySink)
			mpBuddySink = nullptr;
		if (mpBuddySink && mpBuddySink == sink())
			mpBuddySink->sync();
		if (mpBuddy)
			mpBuddy->sync();
		delete pOldSink;
		return true;
	}

	XSink *XRedirect::sink()
	{
		if (mSinks.empty())
			return nullptr;
		return mSinks.front();
	}

	bool XRedirect::sync()
	{
		if (!mSinks.empty())
		{
			XSink *pSink(mSinks.front());
			if (pSink == mpBuddySink)//is on top
			{
				return mpBuddySink->sync();
			}
		}
		return false;
	}

	int XRedirect::desc()
	{
		int fd(fileno(mf0));
		if (fd == -1)
		{
			if (mf0 == stdout)
				fd = 1;
			else if (mf0 == stderr)
				fd = 2;
		}
		return fd;
	}
	
	bool XRedirect::toNul()
	{
		FILE *pf(freopen(DEVNULL, "w", mf0));
		return (pf != nullptr);
	}

	bool XRedirect::toFile(const char *fname, bool append)
	{
		try
		{
			const char *mode(append?"a":"w");
			XSink *pSink(new XFileSink(fname, mode));
			Push(pSink);
		}
		catch (...)
		{
			return false;
		}
		return true;
	}

	bool XRedirect::toPipe(XPipeReader &r)
	{
		try
		{
			XSink *pSink(new XPipeSink(r));
			Push(pSink);
		}
		catch (...)
		{
			return false;
		}
		return true;
	}

	bool XRedirect::toBuddy(XRedirect &buddy)
	{
		if (mpBuddySink)
			return false;
		try
		{
			mpBuddySink = new XBuddySink(*this, buddy);
			Push(mpBuddySink);
		}
		catch (...)
		{
			return false;
		}
		return true;
	}

	bool XRedirect::Restore(int levels)
	{
		unsigned l(levels);
		while (l-- > 0)
		{
			if (!Pop())
				break;
		}
		return ((levels < 0) || (l == 0));
	}

	//////////////////////////////////
	// XCaptureStream

	XCaptureStream::XCaptureStream(const char *header, int kB, unsigned chunkSize)
		: MyStream(chunkSize),
		mHeader(header?header:""),
		mpPipeReader(nullptr),
		mbDirty(false)
	{
		mpPipeReader = new XPipeReader(*this, kB);
	}
	
	XCaptureStream::~XCaptureStream()
	{
		delete mpPipeReader;
	}

	unsigned XCaptureStream::Write(void *p, unsigned sz)
	{
		std::lock_guard<std::mutex> lock(mMx);
		unsigned bytes(MyStream::Write(p, sz));
		if (bytes > 0)
		{
			if (!mbDirty)
			{
				mbDirty = true;
				OutputReady(false);
			}
		}
		return bytes;
	}

	bool XCaptureStream::Flush(MyStreamBase &ss, FILE *pf, unsigned portion, int reset)
	{
		std::lock_guard<std::mutex> lock(mMx);
		if (mbDirty)
		{
			if (pf)
			{
				WriteToFile(pf);
				rewind(0);
			}
			MyStreamUtil ssh(ss);
			if (!mHeader.empty())
				ssh.WriteString(mHeader.c_str());// "[%s] %d", mName.c_str(), mWinId);
			ssh.WriteStream(*this, portion, reset);
			if (empty())
			{
				mbDirty = false;
				Reset();//true);
			}
			return true;
		}
		return false;
	}

	void XCaptureStream::Redirect(const char *header)
	{
		mbDirty = true;
		OutputReady(true);
		{
			std::lock_guard<std::mutex> lock(mMx);
			Reset(true);
			if (header)
				mHeader = header;
		}
	}


}//namespace My
