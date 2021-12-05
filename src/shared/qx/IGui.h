#ifndef __IGUI_H__
#define __IGUI_H__

#include "IUnk.h"

class MyStreamBase;

namespace My
{
	class IGuiMsgBase : public IUnk
	{
	public:
		virtual void *data() = 0;
	};

	class IGui : public IUnk
	{
	public:
		virtual int	SendEvent(int e, void *p){ return CallEvent(e, p); }
		virtual int	PostEvent(int e, IGuiMsgBase *){ return CallEvent(e, 0); }
		virtual int CallEvent(int e, void *p){ return ProcessEvent(e, p); }

	protected:
		virtual int	ProcessEvent(int, void *){ return 1; }
	};

	template <class T>
	class IGuiMsg : public IGuiMsgBase
	{
	public:
		IGuiMsg(){}
		IGuiMsg(T t) : m_t(t){}
		virtual ~IGuiMsg(){}
		virtual void *data(){ return &m_t; }
		T &obj(){ return m_t; }
	protected:
		T	m_t;
	};

	class IWClient : public IUnk
	{
	public:
		virtual void PostRequest(long, MyStreamBase &) = 0;
		virtual long SendRequest(long, MyStreamBase &) = 0;
		virtual long CallRequest(long, MyStreamBase &) = 0;
	};

	class IWServer : public IWClient
	{
	public:
		virtual void Disconnected(long) = 0;
		virtual void ParentDisconnected() = 0;
	};

}//namespace My

#endif//__IGUI_H__
