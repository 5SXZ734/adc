#include <stdlib.h>
#include <chrono>

#include "QxSemaphore.h"
#include "MyApp.h"

namespace My {

	static const unsigned THREAD_TIMEOUT = 1000;


	/*#ifdef WIN32
	DWORD     EventLoopBase::msMainThreadId;
	#else
	pthread_t EventLoopBase::msMainThreadId;
	#endif

	bool EventLoopBase::msbRegisteredMainThread = false;*/


	//////////////////////////////////////////////////////////
	// SxMsgBlockerEventData

	class SxMsgBlockerEventData
	{
	public:
		SxMsgBlockerEventData(const SxCustomEvent* pMsg, QxSemaphore* psem, int& ret_code)
			: mpvData(pMsg),
			mpSem(psem),
			mpnReturnCode(&ret_code)
		{
		}

		virtual ~SxMsgBlockerEventData()
		{
			if (mpvData->m_autodelete)
				delete mpvData;
		}

		const void* getData() { return mpvData; }
		bool isBlocking() { return mpSem != nullptr; }

		void setReturnCode(const int nrc)
		{
			if (!mpSem)
				return;

			*mpnReturnCode = nrc;

			// release semaphore to indicate processing has been completed
			if (!mpSem->available())
			{
				(*mpSem)--;
			}
		}

	private:
		const SxCustomEvent* mpvData;
		QxSemaphore* mpSem;
		int* mpnReturnCode;
	};


	//////////////////////////////////////////////////////////////
	// EventLoopBase

	EventLoopBase::EventLoopBase()
	{
		//if (!msbRegisteredMainThread)
		{
#ifdef WIN32
			msMainThreadId = GetCurrentThreadId();
#else
			msMainThreadId = pthread_self();
#endif
			// msbRegisteredMainThread = true;
		}
	}

	EventLoopBase::~EventLoopBase()
	{
	}

	bool EventLoopBase::isMyThread()
	{
		//   if (!msbRegisteredMainThread)
		//     return false;

#ifdef WIN32
		if (GetCurrentThreadId() == msMainThreadId)
#else
		if (pthread_self() == msMainThreadId)
#endif
			return true;
		return false;
	}

	bool EventLoopBase::blockEvent(const SxCustomEvent* pEvent, int& nrc)
	{
		if (isMyThread())
			return false;

		QxSemaphore* pSema = new QxSemaphore(1);
		(*pSema)++;

		SxMsgBlockerEventData* pev_data = new SxMsgBlockerEventData(pEvent, pSema, nrc);
		postBlockedEvent(pev_data);

		(*pSema)++;

		delete pev_data;
		delete pSema;
		return true;
	}

	void EventLoopBase::closeBlockedEvent(SxMsgBlockerEventData* pev_data)
	{
		int nrc = processBlockedEvent(pev_data->getData());

		// CANNOT set non-blocking event return code as variable probably no longer valid
		if (pev_data->isBlocking())
			pev_data->setReturnCode(nrc);
		else
			delete pev_data;
	}

	void EventLoopBase::closeBlockedEvent(SxMsgBlockerEventData* pev_data, int nrc)
	{
		if (pev_data->isBlocking())
			pev_data->setReturnCode(nrc);
		else
			delete pev_data;
	}


	/////////////////////////////////////////////////////////////////
	// EventLoop

	EventLoop::EventLoop()
		: mbStop(false),
		m_argc(0),
		m_argv(nullptr),
		mbClosingDown(false),
		mRetCode(0),
		miTimeout(THREAD_TIMEOUT)
	{
	}

	EventLoop::EventLoop(int argc, char** argv)
		: mbStop(false),
		m_argc(argc),
		m_argv(argv),
		mbClosingDown(false),
		mRetCode(0),
		miTimeout(THREAD_TIMEOUT)
	{
	}

	EventLoop::~EventLoop()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mEventQueue._Clear();
	}

	void EventLoop::repostFailedEvents()
	{
		while (!mFailedEvents.empty())
		{
			const SxCustomEvent* pEvent = mFailedEvents._Pop();
			postEvent(pEvent);
		}
	}

	void EventLoop::postEvent(const SxCustomEvent* e)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mEventQueue._Push(e);
		mWaitCondition.notify_one();
	}

	int EventLoop::sendEvent(const SxCustomEvent* e)
	{
		int ret = 1;
		if (blockEvent(e, ret))
			return ret;

		return processEvent(e);
	}

	int EventLoop::callEvent(const SxCustomEvent* e)
	{
		int ret = processEvent(e);
		if (e->m_autodelete)
			delete e;
		return ret;
	}

	void EventLoop::postBlockedEvent(SxMsgBlockerEventData* pev_data)
	{
		postEvent(new SxCustomEvent(SXEVENT_BLOCKED, pev_data));
	}

	int	EventLoop::processBlockedEvent(const void* e)
	{
		return processEvent((const SxCustomEvent*)e);
	}

	int EventLoop::exec()
	{
		bool bBusy = false;
		bool bIdle = true;
		while (!mbStop)
		{
			const SxCustomEvent* e;
			{
				std::lock_guard<std::mutex> lock(mMutex);
				e = mEventQueue._Pop();
			}
			if (e)
			{
				bBusy = true;
				if (bIdle)
				{
					OnBusy();
					bIdle = false;
				}

				switch (e->m_id)
				{
				case SXEVENT_NULL:
					break;
				case SXEVENT_BLOCKED:
				{
					SxMsgBlockerEventData* pev_data = (SxMsgBlockerEventData*)e->m_data;
					closeBlockedEvent(pev_data);
				}
				break;
				case SXEVENT_QUIT:
					if (canQuit())
						OnQuit();
					break;
				default:
					processEvent(e);
					break;
				}
				if (e->m_autodelete)
				{
					delete e;
				}
#if(0)
				else
					mFailedEvents.Push(e);
#endif
			}
			else
			{
				if (bBusy)
				{
					OnIdle();//new event(s) can be pushed
					bBusy = false;
					continue;
				}

				bIdle = true;

				{
					std::unique_lock<std::mutex> Lock(mMutex);
					if (mWaitCondition.wait_for(Lock, std::chrono::milliseconds(miTimeout)) != std::cv_status::timeout)
						continue;
				}

				OnTimeout();
			}
		}

		return mRetCode;
	}

	void EventLoop::exit(int retCode)
	{
		mRetCode = retCode;
		mbStop = true;
	}

	void EventLoop::stop(int retCode)
	{
		mRetCode = retCode;
		mbStop = true;
		mWaitCondition.notify_one();
	}

	void EventLoop::clearEventQueue()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mEventQueue._Clear();
	}

	bool EventLoop::hasPendingEvents()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return !mEventQueue._IsEmpty();
	}

	void EventLoop::reset()
	{
		mbStop = false;
		mRetCode = 0;
	}

	void EventLoop::clear()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		for (;;)
		{
			const SxCustomEvent* e(mEventQueue._Pop());
			if (!e)
				break;
			if (e->m_id == SXEVENT_BLOCKED)
			{
				SxMsgBlockerEventData* pev_data = (SxMsgBlockerEventData*)e->m_data;
				closeBlockedEvent(pev_data, 0);
			}
			if (e->m_autodelete)
			{
				delete e;
			}
		}
	}

}//namespace My

