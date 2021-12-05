#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "IGui.h"

namespace My {
	class Sync : public std::condition_variable
	{
		typedef std::condition_variable	SyncBase;
	public:
		Sync()
		{
		}
		/*void lock()
		{
			mMutex.lock();
		}
		void unlock()
		{
			mMutex.unlock();
		}
		void wait()
		{
			//std::unique_lock<std::mutex> lock(mMutex);
			SyncBase::wait(lock, [] { return false; });//infinite wait until notified
		}
		void wait(unsigned long ms)
		{
			//std::unique_lock<std::mutex> lock(mMutex);
			SyncBase::wait_for(lock, std::chrono::milliseconds(ms));
		}*/
		void wakeOne()
		{
			SyncBase::notify_one();
			mMutex.unlock();
		}
		void wakeAll()
		{
			SyncBase::notify_all();
			mMutex.unlock();
		}
		std::mutex mMutex;
	};




	class GuiThread : public std::thread,
		public My::IGui
	{
	public:
		GuiThread(int argc, char** argv)
			: //std::thread(),
			m_argc(argc),
			m_argv(argv),
			mpIGui(0)
		{
		}

		virtual ~GuiThread()
		{
			Stop();
		}

		bool Start(IUnk* pICore)
		{
			std::unique_lock<std::mutex> lock(mWC.mMutex);
			std::thread& self(*this);
			self = std::thread(&GuiThread::run, std::ref(*this), pICore);
			mWC.wait(lock);
			bool bRet = (mpIGui != 0);
			return bRet;
		}

		void Stop()
		{
			if (get_id() != std::thread::id())//running?
			{
				if (mpIGui)
					mpIGui->PostEvent(me[EV_QUIT], 0);
				join();//wait();
			}
		}

		enum { EV_READY, EV_RUN, EV_QUIT };

	protected:
		void run(IUnk* pICore)
		{
			me[EV_READY] = EventId(EV_READY);
			me[EV_RUN] = EventId(EV_RUN);
			me[EV_QUIT] = EventId(EV_QUIT);

			mpIGui = NewApplication(pICore);
			mWC.mMutex.lock();
			if (mpIGui)
			{
				mpIGui->PostEvent(me[EV_READY], new GuiMsgReady(mWC));
				mpIGui->CallEvent(me[EV_RUN], nullptr);
				IGui* pIGui = mpIGui;
				mpIGui = nullptr;
				pIGui->Release();
				Unload();
			}
			else
			{
				mWC.wakeOne();
			}
		}

		virtual int	SendEvent(int eID, void* pData)
		{
			if (mpIGui)
				return mpIGui->SendEvent(eID, pData);
			return 1;
		}

		virtual int PostEvent(int eID, My::IGuiMsgBase* pData)
		{
			if (mpIGui)
				return mpIGui->PostEvent(eID, pData);
			return 1;
		}

		virtual int	CallEvent(int eID, void* pData)
		{
			if (mpIGui)
				return mpIGui->CallEvent(eID, pData);
			return 1;
		}

		virtual My::IGui* NewApplication(IUnk*) = 0;
		virtual void Unload() = 0;
		virtual int EventId(int n) { return n; }

		class GuiMsgReady : public My::IGuiMsgBase
		{
		public:
			GuiMsgReady(My::Sync& wc)
				: mWC(wc)
			{
			}
			virtual ~GuiMsgReady()
			{
				mWC.wakeOne();
			}
			virtual void* data() { return 0; }
		protected:
			My::Sync& mWC;
		};

	protected:
		int		m_argc;
		char** m_argv;
		My::IGui* mpIGui;
		My::Sync	mWC;
		int		me[3];
	};

}//namespace My




