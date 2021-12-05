#include "MyCapture.h"
#include "MyStream.h"

#include <fcntl.h>
//#ifndef WIN32
#include <unistd.h>
//#else
//#include <io.h>
//#endif

using namespace std;

MyCapture::MyCapture(FILE** files)
	: m_init(false)
{
	m_pipe[READ] = 0;
	m_pipe[WRITE] = 0;
#ifdef WIN32
	if (_pipe(m_pipe, 64 * 1024, O_BINARY) != -1)
#else
	if (pipe(m_pipe) != -1)
#endif
	{
		int success(0);
		for (int i(0); files[i] != nullptr; i++)
		{
			FILE* f(files[i]);
			file_t file(f, dup(fileno(f)));
			if (file.fd != -1)
			{
				success++;
				m_files.push_back(file);
			}
		}

		m_init = (success > 0);
	}
}

MyCapture::~MyCapture()
{
	for (list<file_t>::iterator it(m_files.begin()); it != m_files.end(); it++)
	{
		file_t& file(*it);
		if (file.fd > 0)
			close(file.fd);
	}

	if (m_pipe[READ] > 0)
		close(m_pipe[READ]);
	//	if (m_pipe[WRITE] > 0)
	//		close(m_pipe[WRITE]);
}

void MyCapture::Begin()
{
	if (!m_init)
		return;

	for (list<file_t>::iterator it(m_files.begin()); it != m_files.end(); it++)
	{
		file_t& file(*it);
		fflush(file.f);
		dup2(m_pipe[WRITE], fileno(file.f));
		//		if (m_pipe[WRITE] > 0)
		//			close(m_pipe[WRITE]);
				//m_pipe[WRITE] = file.fd;
	}

	if (m_pipe[WRITE] > 0)
		close(m_pipe[WRITE]);
}

bool MyCapture::Read(MyStreamBase& ss)
{
	char buf[1024];

	int bytesRead = 0;
	//	if (!eof(m_pipe[READ]))
	do {
		if (bytesRead > 0)
			ss.Write(buf, bytesRead);

		bytesRead = read(m_pipe[READ], buf, sizeof(buf));//&(*buf.begin()), bufSize);

	} while (bytesRead == sizeof(buf) && buf[sizeof(buf) - 1] != 0);

#if(0)
	while (bytesRead == sizeof(buf))
	{
		ss.Write(buf, bytesRead);
		bytesRead = 0;
		if (!eof(m_pipe[READ]))
		{
			bytesRead = read(m_pipe[READ], buf, sizeof(buf));//&(*buf.begin()), bufSize);
		}
	}
#endif

	if (bytesRead > 0)
	{
		ss.Write(buf, bytesRead);
	}
	return true;
}

bool MyCapture::End()
{
	if (!m_init)
		return false;

	/*?	for (list<file_t>::iterator it(m_files.begin()); it != m_files.end(); it++)
		{
			file_t &file(*it);
			fflush(file.f);
			dup2(file.fd, fileno(file.f));
		}*/

		//..Read?

	return true;
}

void MyCapture::Close()
{
	for (list<file_t>::iterator it(m_files.begin()); it != m_files.end(); it++)
	{
		file_t& file(*it);
		char eof = 0;
		size_t bytes(::fwrite(&eof, sizeof(eof), 1, file.f));
		fflush(file.f);
		(void)bytes;
	}
}




/////////////////////////////////
#include <assert.h>

MyCaptureThread::MyCaptureThread(FILE** files, MyStreamBase& ss)
#ifdef WIN32
	://QxThread(),
#else
	://QxThread(64*1024),
#endif // WIN32
	mbStop(false),
	mCapture(files),
	mss(ss)
{
}

MyCaptureThread::~MyCaptureThread()
{
	//assert(!running());
	assert(get_id() == std::thread::id());
}

void MyCaptureThread::run(void)
{
	{
		std::lock_guard<std::mutex> lock(mMx);
		mCapture.Begin();
		mWC.notify_one();
	}

	do {
		mCapture.Read(mss);
	} while (!mbStop);

	mCapture.End();
}

void MyCaptureThread::Start()
{
	std::unique_lock<std::mutex> lock(mMx);
	//start();
	std::thread& self(*this);
	self = std::thread(&MyCaptureThread::run, std::ref(*this));
	mWC.wait(lock);
}

void MyCaptureThread::Stop()
{
	mbStop = true;

	mCapture.Close();

	//if (!wait(3000)) terminate();
	join();
}



///////////////////////////////////

MyCaptureStream::MyCaptureStream(int streamId, const char* name, unsigned chunkSize)
	: MyStream(chunkSize),
	mStreamId(streamId),
	mWinId(0),
	mName(name),
	mpCapture(nullptr),
	mbDirty(false)
{
	startCapture();
}

MyCaptureStream::~MyCaptureStream()
{
}

void MyCaptureStream::Stop()
{
	stopCapture(true);
}

void MyCaptureStream::startCapture()
{
	assert(!mpCapture);

	FILE* files[] = { stdout, stderr, nullptr };
	FILE** pfiles = files;
	if (mStreamId == 1)
		files[1] = nullptr;
	else if (mStreamId == 2)
		pfiles = &files[1];

	mpCapture = new MyCaptureThread(pfiles, *this);
	mpCapture->Start();
}

void MyCaptureStream::stopCapture(bool bExit)
{
	if (mpCapture)
	{
		if (!bExit)
			mbDirty = true;
		mpCapture->Stop();
		delete mpCapture;
		mpCapture = nullptr;
	}

	if (!bExit)
	{
		if (mbDirty)
			OutputReady(mStreamId, true);
	}
	else
	{
		fclose(stderr);
		fclose(stdout);
	}
}

unsigned MyCaptureStream::Write(void* p, unsigned sz)
{
	std::lock_guard<std::mutex> lock(mMx);
	unsigned bytes(MyStream::Write(p, sz));
	if (bytes > 0)
	{
		if (!mbDirty)
		{
			mbDirty = true;
			OutputReady(mStreamId, false);
		}
	}
	return bytes;
}

bool MyCaptureStream::Flush(MyStreamBase& ss, FILE* pf, unsigned portion, int reset)
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
		ssh.WriteStringf("[%s] %d", mName.c_str(), mWinId);
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

void MyCaptureStream::Restart(int winId)
{
	stopCapture(false);
	{
		std::lock_guard<std::mutex> lock(mMx);
		MyStream::Reset(true);
		mWinId = winId;
	}
	startCapture();
}

void MyCaptureStream::Pause()
{
	stopCapture(false);
	{
		std::lock_guard<std::mutex> lock(mMx);
		MyStream::Reset(true);
	}
}

void MyCaptureStream::Continue()
{
	startCapture();
}