#pragma once

#ifdef WIN32
	#include <windows.h>
#else
	#include <pthread.h>
#endif

#include <QtCore/QEvent>

#include "qx/IGui.h"

// custom events
typedef enum
{
    GUI_EVENT_EXIT         = QEvent::User,
    GUI_BLOCKING_EVENT,
		GUI__EVENT_LAST
} SmspiceCustomEvent;

class SxGUI : public My::IGui
{
public:
	SxGUI();
	virtual ~SxGUI();

protected:
	virtual int SendEvent(int, void *);
	virtual int PostEvent(int, My::IGuiMsgBase *);

	virtual void PostBlockingEvent(void *) = 0;
	virtual bool isClosingDown() = 0;

	void ClearBlockingEvent(void *);

private:
	bool postBlockingEvent(int, void *, int&);

	// thread safety
	static bool isMainThread();

private:
#ifdef WIN32
	static DWORD	msMainThreadId;
#else
	static pthread_t msMainThreadId;
#endif

	static bool msbRegisteredMainThread;
};
