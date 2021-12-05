#ifndef __ANLZ_H__
#define __ANLZ_H__

//#include <list>
//#include "qx/IUnk.h"
#include "mem.h"

class MyStreamBase;
class I_Context;
class Locus_t;

enum class STAGE_STATUS_e { SKIPPED = -1, FAILED, DONE, CONTINUE };

enum class StopFlag { ABORT = -1, RESET = 0, PAUSE = 1 };

class IAnalyzer// : public My::IUnk
{
	StopFlag	m_eStopFlag;
public:
	IAnalyzer()
		: m_eStopFlag(StopFlag::RESET)
	{
	}
	virtual ~IAnalyzer()
	{
	}
	void setStopFlag(StopFlag i){
		m_eStopFlag = i;
	}
	StopFlag stopFlag() const {
		return m_eStopFlag;
	}


	virtual size_t size() const = 0;//number of tasks
	virtual void writeTask(size_t, MyStreamBase &) = 0;
	virtual void writeModule(size_t, MyStreamBase &) = 0;
	virtual void writeDA(size_t, MyStreamBase &) = 0;

	virtual bool writeToDoList(MyStreamBase &) = 0;
	virtual int process() = 0;
	virtual bool finished() = 0;
	virtual ADDR currentVA() const = 0;
	virtual FolderPtr currentFile() const = 0;
	virtual FieldPtr currentOpField() const = 0;
	virtual void setCurrentFieldRef(FieldPtr) = 0;

	virtual void setContextFile(const char *) = 0;
	virtual I_Context *makeContext() const = 0;
	virtual void getLocus(Locus_t &) const = 0;
};



#endif//__ANLZ_H__
