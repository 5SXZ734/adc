#pragma once

#ifdef _DEBUG
#ifdef WIN32
#define WIN32_DEBUG
#endif
#define __DEBUGGING 1
#define __RELEASING 0
#else
#define __DEBUGGING 0
#define __RELEASING 1
#endif

//memory
#define NEW_MEMMGR	1

#define DEBUG_SAVE	(__DEBUGGING || 0)

#define DUMP_OP_FIELD_ATTACH 0

#define TRACK_PTRS2VIEW (0 && __DEBUGGING)	//slowdown ~20%

#define NO_FILE_PROLOGUE __DEBUGGING
#define _INTERNAL 0

#define NO_OBJ_ID __RELEASING
#define	NO_FILE_ID	0

#define OLD_LOCALS 1

//op tracers
#define NEW_OP_PTR 1					//affects SAVE/RESTORE
#define NEW_OP_TRACER	NEW_OP_PTR

#define NEW_PATH_PTR 1					//affects SAVE/RESTORE
#define NEW_PATH_TRACER	NEW_PATH_PTR
#define	FUNC_SAVE_ENABLED	(NEW_OP_PTR && NEW_PATH_PTR)

#define X64_SUPPORT	1

#define MEMTRACE_ENABLED	0

#define NEW_VARSIZE_MAP	0

//#define	COMPACT_VIEW	//ui_main.cpp

#define NEW_TYPE_NAMES	1

#define NEW_PROXY_TYPE	1

#define SHARED_NAMES	0

#define SHARED_STOCK_TYPES	1

#define RECOVER_NAMESPACES 1
#define IMPLICIT_STATICS 0		//treat symbol as static if demangled info does not specify 'thiscall/const/virtual' and no 'static'

#define STRUCVARS_LITE	1		//do not save strucvar's contents

#define DEBUG_NS 1

#define NO_TYPE_PROXIES	1

#define NEW_LOCAL_VARS	1		//no struclocs

#define STDERR	stdout


#ifdef _DEBUG
#define CHECK(arg)  if(arg) 
#define STOP { int a = 0; (void)a; }
#define STOP2(arg) { arg; }
#define STOPx { throw (0); }
#else
#define CHECK(arg)
#define STOP
#define STOP2(arg)
#define STOPx
#endif

#define ASSERT0	{assert(0); abort();}

#undef TRACE1
#define TRACE1
#undef TRACE2
#define TRACE2
