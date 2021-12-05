#include "debug.h"
#include "ui_main.h"
#include "proj.h"
#include "types_mgr.h"
#include "anlzbin.h"
#include "main.h"


////////////////////////////////////////////////////////////////
// Debugger_t

Debugger_t::Debugger_t(const ProjectInfo_t &pi, const MyString &path)
	: ProjectInfo_t(pi),
	mPath(path),
	mCurrent(0),
	mResumeMode(Resume_Continue),
	miTextSeg(nullptr),
	mLastBP(mBPs.end()),
	mbSuspended(true)
{
	miTextSeg = findCodeSeg(ModuleCur());
}

Debugger_t::~Debugger_t()
{
	//if (running())
	if (get_id() != std::thread::id())
		//QxThread::wait();
		join();
}


#ifdef WIN32
#include <windows.h>
#include <psapi.h>
#include <tchar.h>

static MyString GetFileNameFromHandle(HANDLE hFile)
{
	BOOL bSuccess = FALSE;
	TCHAR pszFilename[MAX_PATH + 1];
	HANDLE hFileMap;

	MyString strFilename;

	// Get the file size.
	DWORD dwFileSizeHi = 0;
	DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi);

	if (dwFileSizeLo == 0 && dwFileSizeHi == 0)
	{
		return FALSE;
	}

	// Create a file mapping object.
	hFileMap = CreateFileMapping(hFile,
		nullptr,
		PAGE_READONLY,
		0,
		1,
		nullptr);

	if (hFileMap)
	{
		// Create a file mapping to get the file name.
		void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

		if (pMem)
		{
			if (GetMappedFileName(GetCurrentProcess(),
				pMem,
				pszFilename,
				MAX_PATH))
			{

#define BUFSIZE 512

				// Translate path with device name to drive letters.
				TCHAR szTemp[BUFSIZE];
				szTemp[0] = '\0';

				if (GetLogicalDriveStrings(BUFSIZE - 1, szTemp))
				{
					TCHAR szName[MAX_PATH];
					TCHAR szDrive[3] = TEXT(" :");
					BOOL bFound = FALSE;
					TCHAR* p = szTemp;

					do
					{
						// Copy the drive letter to the template string
						*szDrive = *p;

						// Look up each device name
						if (QueryDosDevice(szDrive, szName, MAX_PATH))
						{
							size_t uNameLen = _tcslen(szName);

							if (uNameLen < MAX_PATH)
							{
								bFound = _tcsnicmp(pszFilename, szName,
									uNameLen) == 0;

								if (bFound)
								{
									//strFilename.Format(L"%s%s", szDrive, pszFilename + uNameLen);
									strFilename.append(szDrive);
									strFilename.append(pszFilename + uNameLen);
								}
							}
						}

						// Go to the next nullptr character.
						while (*p++);
					} while (!bFound && *p); // end of string
				}
			}
			bSuccess = TRUE;
			UnmapViewOfFile(pMem);
		}

		CloseHandle(hFileMap);
	}

	return(strFilename);
}

bool Debugger_t::isInRange(ADDR a) const
{
	if (!miTextSeg)
		return false;
	ADDR textBase(miTextSeg->base());
	unsigned textSize(miTextSeg->size());
	//mpDebugger->setRange(pSeg->base(), pSeg->size());
	return textBase <= a && a < textBase + textSize;
}

/*void Debugger_t::setRange(ADDR base, unsigned size)
{
	mTextBase = base;
	mTextSize = size;
}*/

void Debugger_t::debug()
{
	//QxThread::start();
	std::thread& self(*this);
	self = std::thread(&Debugger_t::run, std::ref(*this));
}

void Debugger_t::run()
{
	mbSuspended = false;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	BOOL success(CreateProcessA(mPath.Path().c_str(), nullptr, nullptr, nullptr, false, DEBUG_ONLY_THIS_PROCESS, nullptr, nullptr, &si, &pi));
	if (!success)
		return;

	DEBUG_EVENT debug_event = { 0 };


	MyString strEventMessage;

	DWORD dwContinueStatus = DBG_CONTINUE;
	BOOL m_bBreakpointOnceHit(FALSE);

	bool bStopDebugging(false);
	for (;;)
	{
		if (!WaitForDebugEvent(&debug_event, INFINITE))
			break;

		switch (debug_event.dwDebugEventCode)
		{
		case CREATE_PROCESS_DEBUG_EVENT:
		{
			//LPVOID pStartAddress = (LPVOID)debug_event.u.CreateProcessInfo.lpStartAddress;
			aCreateProcessInfo = debug_event.u.CreateProcessInfo;
			// Do something with pStartAddress to set BREAKPOINT.
			strEventMessage.sprintf("Loaded '%s", GetFileNameFromHandle(debug_event.u.CreateProcessInfo.hFile).c_str());
		}
		break;

		case CREATE_THREAD_DEBUG_EVENT:
			strEventMessage.sprintf("Thread 0x%X (Id: %d) created at: 0x%X",
				debug_event.u.CreateThread.hThread,
				debug_event.dwThreadId,
				debug_event.u.CreateThread.lpStartAddress); // Thread 0xc (Id: 7920) created at: 0x77b15e58
			break;

		case EXIT_THREAD_DEBUG_EVENT:
			strEventMessage.sprintf("The thread %d exited with code: %d",
				debug_event.dwThreadId,
				debug_event.u.ExitThread.dwExitCode);	// The thread 2760 exited with code: 0
			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			fprintf(stdout, "Program exited (code=0x%X)", debug_event.u.ExitProcess.dwExitCode);
			fflush(stdout);
			bStopDebugging = true;
			break;

		case LOAD_DLL_DEBUG_EVENT:
			{
			MyString sDllPath(GetFileNameFromHandle(debug_event.u.LoadDll.hFile));
			mDllNameMap.insert(std::make_pair(debug_event.u.LoadDll.lpBaseOfDll, sDllPath));
			strEventMessage.sprintf("Loaded '%s' (%X)",
				sDllPath.c_str(),
				debug_event.u.LoadDll.lpBaseOfDll);
			}
			break;

		case UNLOAD_DLL_DEBUG_EVENT:
			strEventMessage.sprintf("Unloaded %s", mDllNameMap[debug_event.u.UnloadDll.lpBaseOfDll].c_str());
			break;

		case OUTPUT_DEBUG_STRING_EVENT:
			{
			OUTPUT_DEBUG_STRING_INFO & DebugString = debug_event.u.DebugString;

			WCHAR *msg = new WCHAR[DebugString.nDebugStringLength];
			ZeroMemory(msg, DebugString.nDebugStringLength);

			ReadProcessMemory(pi.hProcess, DebugString.lpDebugStringData, msg, DebugString.nDebugStringLength, nullptr);

			if (DebugString.fUnicode)
				strEventMessage = "msg";// msg;
			else
				strEventMessage = (LPSTR)msg;

			delete[]msg;
			}
			break;

		case EXCEPTION_DEBUG_EVENT:
			{
			EXCEPTION_DEBUG_INFO& exception = debug_event.u.Exception;
			switch (exception.ExceptionRecord.ExceptionCode)
			{
			//case EXCEPTION_BREAKPOINT:
			case STATUS_BREAKPOINT:
				if (m_bBreakpointOnceHit) // Would be set to false, before debugging starts
				{
					// Handle the actual breakpoint event
					strEventMessage = "Break point hit";

					CONTEXT aContext;
					aContext.ContextFlags = CONTEXT_ALL;
					GetThreadContext(aCreateProcessInfo.hThread, &aContext);
					PVOID pvAddress(exception.ExceptionRecord.ExceptionAddress);
#ifdef _X86_
					aContext.Eip--; // Move back one byte
					assert((DWORD)pvAddress == aContext.Eip);
#endif
#ifdef _AMD64_
					aContext.Rip--; // Move back one byte
					assert((DWORD64)pvAddress == aContext.Rip);
#endif

					OnBreak(pvAddress);

					if (mLastBP != mBPs.end())
					{
						// a user breakpoint - have to re-insert it after the current instruction executed
						aContext.EFlags |= 0x100;// Set trap flag, which raises "single-step" exception
					}
					else if (mResumeMode == Resume_StepIn)
					{
						aContext.EFlags |= 0x100;// Set trap flag, which raises "single-step" exception
					}
					if (mResumeMode == Resume_StepOver)
					{
						TypePtr iCode(miTextSeg->typeSeg()->typeMgr()->getTypeCode());
						TypePtr iModule(ModuleOf(miTextSeg));
						ModuleInfo_t MI(*this, *iModule);
						DataSourcePane2_t data(MI.GetDataSource()->pvt(), miTextSeg);
						CodeIterator codeIt(data, iCode, miTextSeg, mCurrent);
						codeIt.unassemble();
						const ins_desc_t &desc(codeIt.desc());
						if (desc.isCall())
						{
							PVOID pv(Seg_t::ADDR2PV(miTextSeg, codeIt.nextAddr()));
							insertBreakpoint(pv, BP_AUXILIARY);
						}
						else
						{
							aContext.EFlags |= 0x100;
						}
					}
					//always update context as eip has been rolled back
					SetThreadContext(aCreateProcessInfo.hThread, &aContext);
				}
				else
				{
					// This is first breakpoint event sent by kernel, just ignore it.
					// Optionally display to the user that first BP was ignored.
					m_bBreakpointOnceHit = true;

					//Set initial sturtup breakpoint
					insertBreakpoint(aCreateProcessInfo.lpStartAddress, BP_AUXILIARY);
				}
				break;

			case STATUS_SINGLE_STEP:
				if (mLastBP != mBPs.end())
				{
					//re-insert last user breakpoint
					writeDebuggeeByte(mLastBP->first, 0xCC);
					mLastBP = mBPs.end();//and clear it

					//revert auxiliary bp
					BpMapIt it(mBPs.find(exception.ExceptionRecord.ExceptionAddress));
					if (it != mBPs.end())
					{

					}
					if (mResumeMode == Resume_Continue)
						break;
				}
				//if (mResumeMode == Resume_StepIn)
				{
					CONTEXT aContext;
					aContext.ContextFlags = CONTEXT_ALL;
					GetThreadContext(aCreateProcessInfo.hThread, &aContext);
					PVOID pvAddress(exception.ExceptionRecord.ExceptionAddress);

					bool bUpdateContext(false);
					PVOID pv(exception.ExceptionRecord.ExceptionAddress);
					ADDR va(Seg_t::PV2ADDR(miTextSeg, pv));
					if (isInRange(va))
					{
						//if (mResumeMode == Resume_StepIn)
						{
							OnBreak(pvAddress);
						}

						if (mResumeMode == Resume_StepIn)
						{
							aContext.EFlags |= 0x100; // Set trap flag, which raises "single-step" exception
							bUpdateContext = true;
						}
						else if (mResumeMode == Resume_StepOver)
						{
							TypePtr iCode(miTextSeg->typeSeg()->typeMgr()->getTypeCode());
							TypePtr iModule(ModuleOf(miTextSeg));
							ModuleInfo_t MI(*this, *iModule);
							DataSourcePane2_t data(MI.GetDataSource()->pvt(), miTextSeg);
							CodeIterator codeIt(data, iCode, miTextSeg, mCurrent);
							codeIt.unassemble();
							const ins_desc_t &desc(codeIt.desc());
							if (desc.isCall())//a call
							{
								PVOID pv(Seg_t::ADDR2PV(miTextSeg, codeIt.nextAddr()));
								insertBreakpoint(pv, BP_AUXILIARY);
							}
							else
							{
								aContext.EFlags |= 0x100;
								bUpdateContext = true;
							}
						}
					}
					else
					{
						if (mResumeMode == Resume_StepIn)
						{
							TypePtr iCode(miTextSeg->typeSeg()->typeMgr()->getTypeCode());
							TypePtr iModule(ModuleOf(miTextSeg));
							ModuleInfo_t MI(*this, *iModule);
							DataSourcePane2_t data(MI.GetDataSource()->pvt(), miTextSeg);
							CodeIterator codeIt(data, iCode, miTextSeg, mCurrent);
							codeIt.unassemble();
							const ins_desc_t &desc(codeIt.desc());
							if (desc.isCall())
							{
								PVOID pv(Seg_t::ADDR2PV(miTextSeg, codeIt.nextAddr()));
								insertBreakpoint(pv, BP_AUXILIARY);
							}
							else//TODO: treat as step out(!)
							{
								aContext.EFlags |= 0x100;
								bUpdateContext = true;
							}
						}
						else
						{
							aContext.EFlags |= 0x100;
							bUpdateContext = true;
						}
					}
					if (bUpdateContext)
						SetThreadContext(aCreateProcessInfo.hThread, &aContext);
				}
				break;

			default:
				if (exception.dwFirstChance == 1)
				{
					PVOID pv(exception.ExceptionRecord.ExceptionAddress);
					ADDR va(Seg_t::PV2ADDR(miTextSeg, pv));
					fprintf(stdout, "First chance exception at %X, exception-code: 0x%08X",
						(DWORD)va,
						exception.ExceptionRecord.ExceptionCode);

					CONTEXT aContext;
					aContext.ContextFlags = CONTEXT_ALL;
					GetThreadContext(aCreateProcessInfo.hThread, &aContext);
					PVOID pvAddress(exception.ExceptionRecord.ExceptionAddress);
					OnBreak(pvAddress);
					aContext.EFlags |= 0x100;
					SetThreadContext(aCreateProcessInfo.hThread, &aContext);
				}
				else
				{
					bStopDebugging = true;
				}

				// There are cases where OS ignores the dwContinueStatus, 
				// and executes the process in its own way.
				// For first chance exceptions, this parameter is not-important
				// but still we are saying that we have NOT handled this event.

				// Changing this to DBG_CONTINUE (for 1st chance exception also), 
				// may cause same debugging event to occur continously.
				// In short, this debugger does not handle debug exception events
				// efficiently, and let's keep it simple for a while!
				dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
			}
			break;
			}

		default:
			strEventMessage.clear();
			break;
		}

		if (!strEventMessage.empty())
		{
			//fprintf(stdout, "%s\n", strEventMessage.c_str());
			//fflush(stdout);
		}

		if (bStopDebugging)
			break;

		ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, dwContinueStatus);
		dwContinueStatus = DBG_CONTINUE;//Reset
	}

	mrMain.postEvent(new SxCustomEvent((SxEventEnum)adc::EVENT_DEBUG_TERMINATED, nullptr));
}

void Debugger_t::OnBreak(PVOID pvAddr)
{
	std::unique_lock<std::mutex> lock(mMx);//?

	//setup debugger context
	mResumeMode = Resume_Continue;
	mCurrent = Seg_t::PV2ADDR(miTextSeg, pvAddr);//?
	mcx.ptr()->clear();

	mrProject.OnDebuggerBreak(*this);

	mbSuspended = true;
	gui().GuiDebuggerBreak();
	mWC.wait(lock);
	mbSuspended = false;

	//check for current breakpoint and revert it
	mLastBP = mBPs.find(pvAddr);
	if (mLastBP != mBPs.end())
	{
		if (mLastBP->second.flags & BP_AUXILIARY)//one shot breakpoint must be removed
		{
			restoreBreakpoint(pvAddr, true);//remove
			assert(mLastBP == mBPs.end());
		}
		else
		{
			restoreBreakpoint(pvAddr, false);//user bp must be re-activated right after the current instruction executed
			mLastBP = mBPs.end();
		}
	}
	else
	{
		//bp is not inserted by debugger - do nothing
	}
}

int Debugger_t::processEvent(const SxCustomEvent *)
{
	return 0;
}

void Debugger_t::resume(EResumeMode resumeMode)
{
	std::unique_lock<std::mutex> lock(mMx);//?
	mResumeMode = resumeMode;
	mWC.notify_one();
}

bool Debugger_t::insertBreakpoint(PVOID atAddr, unsigned flags)
{
	assert(!(flags & BP_AUXILIARY) || (mLastBP == mBPs.end()));

	//Set initial sturtup breakpoint
	BYTE cInstruction;
	SIZE_T dwReadBytes;

	// Read the first instruction    
	if (!ReadProcessMemory(aCreateProcessInfo.hProcess, atAddr, &cInstruction, 1, &dwReadBytes))
		return false;

	std::pair<std::map<PVOID, BP_t>::iterator, bool> ret;
	ret = mBPs.insert(std::make_pair(atAddr, BP_t(cInstruction, flags)));
	if (!ret.second)
		return false;//exist!

	// Replace it with Breakpoint
	if (!writeDebuggeeByte(atAddr, 0xCC))
	{
		mBPs.erase(ret.first);
		return false;
	}
	
	if (flags & BP_AUXILIARY)
		mLastBP = ret.first;

	return true;
}

bool Debugger_t::writeDebuggeeByte(PVOID atAddr, BYTE cInstruction)
{
	SIZE_T dwReadBytes;
	if (!WriteProcessMemory(aCreateProcessInfo.hProcess, atAddr, &cInstruction, 1, &dwReadBytes))
		return false;
	FlushInstructionCache(aCreateProcessInfo.hProcess, atAddr, 1);
	return true;
}

bool Debugger_t::restoreBreakpoint(PVOID atAddr, bool bRemove)
{
	std::map<PVOID, BP_t>::const_iterator it(mBPs.find(atAddr));
	if (it == mBPs.end())
		return false;

	const BP_t &bp(it->second);

	if (!writeDebuggeeByte(atAddr, bp.original))
		return false;

	if (bRemove)
	{
		// Map has the important property that inserting a new element into a map does not invalidate iterators that point to existing elements.
		// Erasing an element from a map also does not invalidate any iterators, except, of course, for iterators that actually point to the element that is being erased.

		if (mLastBP == it)
			mLastBP = mBPs.end();
		mBPs.erase(it);
	}
	return true;
}

bool Debugger_t::toggleBP(PVOID atAddr)
{
	std::map<PVOID, BP_t>::const_iterator it(mBPs.find(atAddr));
	if (it == mBPs.end())
	{
		insertBreakpoint(atAddr, BP_USER);
	}
	else
		restoreBreakpoint(atAddr, true);
	return true;
}

bool Debugger_t::hasUserBreakpointAt(PVOID atAddr) const
{
	std::map<PVOID, BP_t>::const_iterator it(mBPs.find(atAddr));
	return (it != mBPs.end() && (it->second.flags & BP_USER));
}

TypePtr Debugger_t::segOwner() const { return miTextSeg->typeSeg()->superLink(); }

TypePtr Debugger_t::ModuleCur() const
{
	TypePtr iType(nullptr);
	assert(0);
/*	Probe_t *pCtx(mrProject.getContext());
	if (pCtx)
	{
		iType = pCtx->moduleFromLocus();
		mrProject.releaseContext(pCtx);
	}*/
	return iType;
}

#else//!WIN32

void Debugger_t::debug() {}
void Debugger_t::resume(EResumeMode) {}
bool Debugger_t::toggleBP(PVOID) { return false; }
bool Debugger_t::hasUserBreakpointAt(PVOID atAddr) const { return false; }
int Debugger_t::processEvent(const SxCustomEvent*) { return 0; }
void Debugger_t::run() {}
TypePtr Debugger_t::segOwner() const { return miTextSeg->typeSeg()->superLink(); }

#endif


