#pragma once

#include "tiny_process/process.hpp"
#include <thread>
#include <condition_variable>
#include "qx/MyApp.h"
#include "qx/MyPath.h"
#include "shared/defs.h"
#include "command.h"
#include "info_proj.h"

class Project_t;
class TypeProc_t;
class Seg_t;

enum EResumeMode
{
	Resume_Continue, // F5
	Resume_Stop,     // Shift+F5
	Resume_StepIn,   // F11
	Resume_StepOver, // F10
	Resume_StepOut
};

enum { BP_USER = 1, BP_AUXILIARY = 2 };

#ifndef WIN32
typedef unsigned char BYTE;
typedef void *	PVOID;
#endif

struct BP_t
{
	BYTE	original;
	unsigned flags;
	BP_t(BYTE _original, unsigned _flags)
		: original(_original),
		flags(_flags)
	{
	}
};

class Debugger_t : public std::thread,
	public My::EventLoop,
	//public QxProcess,
	public ProjectInfo_t
{
	MyPath mPath;
	std::map<void *, MyString> mDllNameMap;
	std::mutex mMx;
	std::condition_variable	mWC;
	ADDR mCurrent;
	enum EResumeMode	mResumeMode;
	TypePtr miTextSeg;
	//ADDR mTextBase;
	//unsigned mTextSize;
	RefPtr_t<Probe_t> mcx;
	typedef std::map<PVOID, BP_t>	BpMap;
	typedef BpMap::iterator BpMapIt;
	BpMap	mBPs;
	BpMapIt	mLastBP;
#ifdef WIN32
	CREATE_PROCESS_DEBUG_INFO aCreateProcessInfo;
#endif
	bool mbSuspended;
public:
	Debugger_t(const ProjectInfo_t &, const MyString &);
	~Debugger_t();
	//void setRange(ADDR, unsigned);
	void debug();
	void resume(EResumeMode);
	bool toggleBP(PVOID);
	bool hasUserBreakpointAt(PVOID) const;
	ADDR current() const { return mCurrent; }
	void setContext(Probe_t *p){ mcx.set(p); }
	const Probe_t &context() const { return *dynamic_cast<const Probe_t *>(mcx.ptr()); }
	Probe_t &context(){ return *dynamic_cast<Probe_t *>(mcx.ptr()); }
	bool suspended() const { return mbSuspended; }
	TypePtr segOwner() const;
	TypePtr ModuleCur() const;//from context
#ifdef WIN32
protected:
	void OnBreak(PVOID);
private:
	bool isInRange(ADDR a) const;
	bool insertBreakpoint(PVOID, unsigned);
	bool restoreBreakpoint(PVOID, bool);
	bool writeDebuggeeByte(PVOID, BYTE);
#endif

protected://QxThread
	virtual void run();
protected://SxApp
	virtual int processEvent(const SxCustomEvent *);
};

#ifdef WIN32
// a way to set a data change breakpoint on a memory range
// https://dzone.com/articles/memory-access-breakpoint-large
struct protect_mem_t {
	protect_mem_t(void* addr, size_t size) : addr(addr), size(size), is_protected(FALSE) {
		protect();
	}
	~protect_mem_t() { release(); }
	BOOL protect() {
		if (!is_protected) {
			// To catch only read access you should change PAGE_NOACCESS to PAGE_READONLY
			is_protected = VirtualProtect(addr, size, PAGE_NOACCESS, &old_protect);
		}
		return is_protected;
	}
	BOOL release() {
		if (is_protected)
			is_protected = !VirtualProtect(addr, size, old_protect, &old_protect);
		return !is_protected;
	}

protected:
	void*   addr;
	size_t  size;
	BOOL    is_protected;
	DWORD   old_protect;
};
#endif

