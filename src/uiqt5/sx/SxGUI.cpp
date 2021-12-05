#include <QtCore/QSemaphore>
#include <QApplication>
#include "SxGUI.h"

#ifdef WIN32
DWORD     SxGUI::msMainThreadId;
#else
pthread_t SxGUI::msMainThreadId;
#endif

bool SxGUI::msbRegisteredMainThread = false;

#if(0)
#define TRACE(a)
#define TRACE1(a, b)
#else
#define TRACE(a) OutputDebugString(a)
#define TRACE1(a, b) \
	{char buf[64]; sprintf(buf, a, b); OutputDebugString(buf);}
#endif


//////////////////////////////////////////////////////////

class SmspiceGuiData
{
public:
	SmspiceGuiData(int eid, void* pvdata)
		: meId(eid),
		mpvdata(pvdata)
	{
	}

	virtual ~SmspiceGuiData()
	{
	}

	int id() { return meId; }
	virtual void* data() { return mpvdata; }
	virtual bool setReturnCode(int) { return false; }

protected:
	int		meId;
	void* mpvdata;
};

class SmspiceGuiDataEx : public SmspiceGuiData
{
public:
	SmspiceGuiDataEx(int eid, My::IGuiMsgBase* pIData)
		: SmspiceGuiData(eid, pIData)
	{
	}

	virtual ~SmspiceGuiDataEx()
	{
		My::IGuiMsgBase* pIData = (My::IGuiMsgBase*)mpvdata;
		if (pIData)
			pIData->Release();
	}

	virtual void* data()
	{
		My::IGuiMsgBase* pIData = (My::IGuiMsgBase*)mpvdata;
		if (pIData)
			return pIData->data();
		return SmspiceGuiData::data();
	}
};

#if(0)
class SmspiceGuiDataBlocking : public SmspiceGuiData
{
public:
	SmspiceGuiDataBlocking(int eid, void* pvdata, int& ret_code)
		: SmspiceGuiData(eid, pvdata),
		mpnReturnCode(&ret_code)
	{
		mpSem = new QSemaphore(1);
		mpSem->acquire();
	}

	virtual ~SmspiceGuiDataBlocking()
	{
		mpSem->acquire();
		delete mpSem;
	}

	virtual bool setReturnCode(int nrc)
	{
		qApp->processEvents();

		if (!mpSem)
			return false;

		*mpnReturnCode = nrc;

		// release semaphore to indicate processing has been completed
		if (!mpSem->available())
		{
			mpSem->release();
		}

		return true;
	}

private:
	QSemaphore* mpSem;
	int* mpnReturnCode;
};
#else
class SmspiceGuiDataBlocking : public SmspiceGuiData
{
public:
	SmspiceGuiDataBlocking(int eid, void* pvdata, int& ret_code)
		: SmspiceGuiData(eid, pvdata),
		mSem(1),
		mpnReturnCode(&ret_code)
	{
		mSem.acquire();
	}

	virtual ~SmspiceGuiDataBlocking()
	{
		mSem.acquire();
	}

	virtual bool setReturnCode(int nrc)
	{
		qApp->processEvents();

		*mpnReturnCode = nrc;

		// release semaphore to indicate processing has been completed
		mSem.release();
		return true;
	}

private:
	QSemaphore	mSem;
	int* mpnReturnCode;
};
#endif


//////////////////////////////////////////////////////////

SxGUI::SxGUI()
//: mbClosingDown(false)
{
	if (!msbRegisteredMainThread)
	{
#ifdef WIN32
		msMainThreadId = GetCurrentThreadId();
#else
		msMainThreadId = pthread_self();
#endif
		msbRegisteredMainThread = true;
	}
}

SxGUI::~SxGUI()
{
}

bool SxGUI::isMainThread()
{
	if (!msbRegisteredMainThread)
		return false;

#ifdef WIN32
	if (GetCurrentThreadId() == msMainThreadId)
#else
	if (pthread_self() == msMainThreadId)
#endif
		return true;
	return false;
}

bool SxGUI::postBlockingEvent(int eid, void* pvdata, int& nrc)
{
	if (isMainThread())
		return false;

	// post event data to GUI thread for processing
	SmspiceGuiDataBlocking* pev_data = new SmspiceGuiDataBlocking(eid, pvdata, nrc);
	PostBlockingEvent(pev_data);

	// blocking event is deallocated once unblocked to ensure synchronization
	// - this will block until semaphore has been unflagged (by GUI thread)
	delete pev_data;
	return true;
}

void SxGUI::ClearBlockingEvent(void* pGuiEventData)
{
	SmspiceGuiData* pev_data = (SmspiceGuiData*)pGuiEventData;
	int nrc = ProcessEvent(pev_data->id(), pev_data->data());

	// CANNOT set non-blocking event return code as variable probably no longer valid
	if (!pev_data->setReturnCode(nrc))
	{
		delete pev_data;
	}
}

int SxGUI::SendEvent(int eid, void* pvdata)
{
	if (isClosingDown())
		return 0;

	//if (eid != 0x1014){fprintf(stdout, "SENDEVENT:%d\n");fflush(stdout);}

	int nrc = 1;
	if (postBlockingEvent(eid, pvdata, nrc))
		return nrc;

	nrc = ProcessEvent(eid, pvdata);
	return nrc;
}

int SxGUI::PostEvent(int eid, My::IGuiMsgBase* pIData)
{
	if (isClosingDown())
		return 0;

	//if (eid != 0x1014){fprintf(stdout, "POSTEVENT:%d\n", eid);fflush(stdout);}

	SmspiceGuiData* pev_data = new SmspiceGuiDataEx(eid, pIData);
	PostBlockingEvent(pev_data);
	return 1;
}



