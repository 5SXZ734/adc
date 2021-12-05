#pragma once

template <typename T>
class QxSingleton
{
	static T *m_instance;
public:
	QxSingleton(){ m_instance = static_cast <T*> (this); }
	~QxSingleton(){}
	static void newInstance()
	{
		assert(!m_instance);
		m_instance = new T();
	}
	static T *instance()
	{
		if (!m_instance)
			m_instance = new T();
		return m_instance;
	}
	static void deleteInstance()
	{
		delete m_instance;
		m_instance = 0;
	}
	QxSingleton(QxSingleton const&){}
	QxSingleton& operator=(QxSingleton const&){}
};

template <typename T> T* QxSingleton<T>::m_instance = 0;

