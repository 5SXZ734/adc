#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <assert.h>

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

enum SxEventEnum
{
	SXEVENT_NULL,
	SXEVENT_QUIT,
	SXEVENT_BLOCKED,
	SXEVENT_USER = 0x1000
};

struct SxCustomEvent
{
	SxCustomEvent()
		: m_id(SXEVENT_NULL),
		m_data(nullptr),
		m_autodelete(true)
	{
	}

	SxCustomEvent(SxEventEnum id, void* data)
		: m_id(id),
		m_data(data),
		m_autodelete(true)
	{
	}
	virtual ~SxCustomEvent()
	{
	}

	virtual void* data() const { return m_data; }

	SxEventEnum m_id;
	void* m_data;
	bool	m_autodelete;
};

namespace My {

	class SxMsgBlockerEventData;

	class EventLoopBase
	{
	public:
		EventLoopBase();
		virtual ~EventLoopBase();

	protected:
		bool blockEvent(const SxCustomEvent*, int&);
		void closeBlockedEvent(SxMsgBlockerEventData*);
		void closeBlockedEvent(SxMsgBlockerEventData*, int);

		virtual void postBlockedEvent(SxMsgBlockerEventData*) = 0;
		virtual int	processBlockedEvent(const void*) = 0;

		// thread safety
		/*static*/ bool isMyThread();

		// just to shut the compiler up
		EventLoopBase(const EventLoopBase&) {}
		EventLoopBase& operator = (const EventLoopBase&) { return *this; }

	private:
#ifdef WIN32
		DWORD     msMainThreadId;
#else
		pthread_t msMainThreadId;
#endif

		//static bool msbRegisteredMainThread;
	};

	template <class T>
	class SxCustomEventT : public SxCustomEvent
	{
	public:
		SxCustomEventT(SxEventEnum id, T t)
			: SxCustomEvent(id, &m_t)
		{
		}

		virtual ~SxCustomEventT()
		{
		}

		virtual void* data() const { return &m_t; }
	protected:
		T	m_t;
	};

	class EventLoop : public EventLoopBase
	{
		class EventQueue : public std::queue<const SxCustomEvent*>
		{
		public:
			EventQueue() {}
			~EventQueue() {
				assert(empty());
			}
			void _Clear()
			{
				while (!empty())
				{
					const SxCustomEvent* e(front());
					pop();
					delete e;
				}
			}
			bool _IsEmpty() const {
				return empty();
			}
			void _Push(const SxCustomEvent* e) {
				push(e);
			}
			const SxCustomEvent* _Pop() {
				if (empty())
					return nullptr;
				const SxCustomEvent* e(front());
				pop();
				return e;
			}
		};

	private:
		int m_argc;
		char** m_argv;
		EventQueue mEventQueue;
		std::mutex mMutex;
		std::condition_variable	mWaitCondition;
		bool mbClosingDown;
		int	mRetCode;
		EventQueue	mFailedEvents;
		int	miTimeout;
	protected:
		bool mbStop;

	public:
		EventLoop();
		EventLoop(int argc, char** argv);
		virtual ~EventLoop();

		void setTimeout(int n) { miTimeout = n; }
		void repostFailedEvents();
		void postEvent0(const SxCustomEvent* e){ mEventQueue._Push(e); }
		void postEvent(const SxCustomEvent*);
		int sendEvent(const SxCustomEvent*);
		int callEvent(const SxCustomEvent*);
		void stop(int = 0);

		int argc() { return m_argc; }
		char** argv() { return m_argv; }

		// termination
		void setClosingDown() { mbClosingDown = true; }
		bool closingDown() { return mbClosingDown; }
		int returnCode() { return mRetCode; }
		void setReturnCode(int n) { mRetCode = n; }
		void clearEventQueue();
		bool hasPendingEvents();
		void reset();
		void clear();

		virtual int exec();

		virtual bool canQuit() { return true; }

	protected:
		void exit(int = 0);
		virtual int processEvent(const SxCustomEvent*) = 0;

		// EventLoopBase
		virtual void postBlockedEvent(SxMsgBlockerEventData*);
		virtual int	processBlockedEvent(const void*);

		virtual void OnBusy() {}
		virtual void OutOfEvents() {}
		virtual void OnIdle() {}
		virtual void OnTimeout() {}
		virtual void OnQuit() { clear();  mbStop = true; }

		// just to shut the compiler up
		explicit EventLoop(const EventLoop&) : EventLoopBase() {}
		EventLoop& operator = (const EventLoop&) { return *this; }
	};


}//namespace My

