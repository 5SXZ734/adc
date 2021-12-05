
#pragma once


/*#ifdef FRONTEND_EXPORTS
#ifdef WIN32
#define FRONTEND_API __declspec(dllexport)
#else
#define FRONTEND_API 
#endif
#else
#define FRONTEND_API //__declspec(dllimport)
#endif*/

#include <map>
#include <set>
#include <string>
#include <iostream>

#include "interface/IADCMain.h"
#include "interface/IADCFront.h"

struct templ_t;

enum OpSeg_t : unsigned
{
	OPSEG_NULL0,
	OPSEG_CODE,
	OPSEG_DATA,
	OPSEG_AUX,
	OPSEG_STACK,
};

enum SS_t 
{
	SS_NONE = 0,
	SS_TYPE_MASK		= 0x00000003L, //flags or register, memory otherwise
		SS_MEMORY		= 0x00000000L,
		SS_DISCRETE		= 0x00000001L, //like for registers, contiguous otherwise
		SS_FLAGS		= 0x00000003L, 
		//SS_MEMORY		= 0x00000003L,
	SS_ARGUMENT		= 0x00000004L,	//can be func's argument?
	SS_RETURNED		= 0x00000008L,	//can be returned by func?
	SS_STACKED		= 0x00000020L,	//can take negative and positive offsets (for args and vars)
	SS_NOMLOC		= 0x00000040L,	//don't attach local mlocs to ops
	SS_SPOILT		= 0x00000080L	//check for being spoilt
};

struct STORAGE_t
{
	const char *name;
	uint32_t		flags;

public:
	STORAGE_t(): name(nullptr), flags(0){}
	STORAGE_t(const char *n, uint32_t f): name(n), flags(f){}
	void operator=(const STORAGE_t &o){ name = o.name; flags = o.flags; }
	bool empty() const { return name == nullptr && flags == 0; }
	bool isDiscrete() const { return (flags & SS_TYPE_MASK) != SS_MEMORY; }//register or flag
	bool isContiguous() const { return !isDiscrete(); }
	bool isRegister() const { return (flags & SS_TYPE_MASK) == SS_DISCRETE; }
	bool isFlag() const { return (flags & SS_TYPE_MASK) == SS_FLAGS; }
	bool isStacked() const { return (flags & SS_STACKED) != 0; }
};

#define STORAGE_MAX	16

struct FE_t
{
	//uint32_t			flags;
	const char *	name;
	STORAGE_t STORAGE[STORAGE_MAX];	//starge spaces
	const templ_t	*ITS;		//instruction templates lists

	struct stub_t
	{
		const char *key;
		const char *value;
	};
	
	const stub_t *	fdefs;		//special func defs

	union {
		int			ptr_near;	//size of near pointer
		int			ptr_size;	
	};
	int				ptr_far;	//size of far pointer
	int				stackaddrsz;

	const r_t		*stack_ptr;	//stack ptr reg (sp, esp)
	const r_t		*stackb_ptr;//stack frame ptr reg (bp, ebp)
	const r_t		*spoilt_regs_check;

	const r_t		*spoilt_regs_def;//default saved regs mask
	FlagMaskType	spoilt_flags_def;//default saved cpu flags mask

	FE_t()
	{
		memset(this, 0, sizeof(FE_t));
	}

	inline const STORAGE_t &storage(SSID_t ssid) const 
	{
		assert(ssid < STORAGE_MAX);
		return STORAGE[ssid];
	}

	int addStorage(SSID_t ssid, const char *_name, uint32_t flags)
	{
		assert(ssid >= SSID_NULL && ssid < STORAGE_MAX);
		assert(STORAGE[ssid].empty());
		STORAGE[ssid] = STORAGE_t(_name, flags);
		return 1;
	}
};


#define REGID(cls, ofs, siz, id)	((ofs&0xFF)|((siz&0xFF)<<8)|((id&0xFF)<<16)|((cls&0xFF)<<24))
//#define RRR(ofs, siz, id)	((ofs&0xFF)|((siz&0xFF)<<8)|((id&0xFF)<<16))
#define OFS(arg)			((uint8_t)((arg>>0)&0xFF))//16
#define SIZ(arg)			((uint8_t)((arg>>8)&0xFF))
#define RID(arg)			((uint8_t)((arg>>0)&0xFF))
#define RCL(arg)			((uint8_t)((arg>>24)&0xFF))

#define OPC(arg)		(uint8_t)	( (arg&0x000000FF)>>0 )		//class
#define OPSZ(arg)		(uint8_t)	( (arg&0x00000F00)>>8 )		//size
#define OPTYP(arg)		(uint8_t)	( (arg&0x0000FF00)>>8 )		//type
//#define OPTYPX(arg)		(uint8_t)	( (arg&0x00FF0000)>>16 )	//if type=ptr then this=*type
#define OPID(arg)		(uint8_t)	( (arg&0xFF000000)>>24 )	//id

#define MAKEOFS(arg)	(uint32_t)( (arg&0xFF)<<0 )
#define MAKEOPC(arg)	(uint32_t)( (arg&0xFF)<<0 )		//class
#define MAKEOPSZ(arg)	(uint32_t)( (arg&0x0F)<<8 )		//size
#define MAKEOPTYP(arg)	(uint32_t)( (arg&0xFF)<<8 )		//type
//#define MAKEOPTYPX(arg)	(int)(uint32_t)( (arg&0xFF)<<16 )		//type-ex
#define MAKEOPID(arg)	(uint32_t)( (arg&0xFF)<<24 )	//id


#define M_(arg)			((arg&0xFFFF)<<16)	//eflags modified
#define C_(arg)			((arg&0xFFFF)<<0)	//FPU status word flags affected
#define __SP(arg)		(((arg*PTRSIZE)&0xFFFF)<<16)	//stack pointer changed implicit
#define _FTOP(arg)		((arg&0xFFFF)<<0)	//fputop changed
#define _INT			(MAKEOPTYP(OPTYP_INTEGER))
#define S_				(MAKEOPTYP(OPTYP_INT0))
#define U_				(MAKEOPTYP(OPTYP_UINT0))

//AUXREGS///////////////////////////////////////////////////
/*
enum TEMP8_t {
	T8 = 1, U8, V8, W8, 
		T16L = 1, U16L, V16L, W16L, T16H, U16H, V16H, W16H};
enum TEMP16_t {
	T16 = 1, U16, V16, W16, 
		T32L = 1, U32L, V32L, W32L, T32H, U32H, V32H, W32H};
enum TEMP32_t {
	T32 = 1, U32, V32, W32, 
		T64L = 1, U64L, V64L, W64L, T64H, U64H, V64H, W64H};
enum TEMP64_t {
	T64 = 1, U64, V64, W64};
*/


enum {
	R_T		= REGID(OPC_AUXREG, 0,	0,	1),
	R_T8	= REGID(OPC_AUXREG, 0,	1,	1),//00000001
	R_T16	= REGID(OPC_AUXREG, 0,	2,	1),//00000003
	R_T32	= REGID(OPC_AUXREG, 0,	4,	1),//0000000F
	R_T64	= REGID(OPC_AUXREG, 0,	8,	1),//000000FF
	R_T80	= REGID(OPC_AUXREG, 0,	10,	1),//000000FF
		R_U		= REGID(OPC_AUXREG, 0x10,	0,	2),
		R_U8	= REGID(OPC_AUXREG, 0x10,	1,	2),//00000100
		R_U16	= REGID(OPC_AUXREG, 0x10,	2,	2),//00000300
		R_U32	= REGID(OPC_AUXREG, 0x10,	4,	2),//00000F00
		R_U64	= REGID(OPC_AUXREG, 0x10,	8,	2),//0000FF00
		R_U80	= REGID(OPC_AUXREG, 0x10,	10,	2),//0000FF00
			R_V		= REGID(OPC_AUXREG, 0x20,	0,	3),
			R_V8	= REGID(OPC_AUXREG, 0x20,	1,	3),//00010000
			R_V16	= REGID(OPC_AUXREG, 0x20,	2,	3),//00030000
			R_V32	= REGID(OPC_AUXREG, 0x20,	4,	3),//000F0000
			R_V64	= REGID(OPC_AUXREG, 0x20,	8,	3),//00FF0000
				R_W		= REGID(OPC_AUXREG, 0x30,	0,	4),
				R_W8	= REGID(OPC_AUXREG, 0x30,	1,	4),//01000000
				R_W16	= REGID(OPC_AUXREG, 0x30,	2,	4),//03000000
				R_W32	= REGID(OPC_AUXREG, 0x30,	4,	4),//0F000000
				R_W64	= REGID(OPC_AUXREG, 0x30,	8,	4),//FF000000
//-----------------------------------------------
	R_T16L	= REGID(OPC_AUXREG, 0,	1,	1),//00000001
	R_T16H	= REGID(OPC_AUXREG, 1,	1,	5),//00000002
	R_T32L	= REGID(OPC_AUXREG, 0,	2,	1),//00000003
	R_T32H	= REGID(OPC_AUXREG, 2,	2,	5),//0000000C
	R_T64L	= REGID(OPC_AUXREG, 0,	4,	1),//0000000F
	R_T64H	= REGID(OPC_AUXREG, 4,	4,	5),//000000F0

	R_U16L	= REGID(OPC_AUXREG, 0x10,	1,	2),//00000100
	R_U16H	= REGID(OPC_AUXREG, 0x11,	1,	6),//00000200
	R_U32L	= REGID(OPC_AUXREG, 0x10,	2,	2),//00000300
	R_U32H	= REGID(OPC_AUXREG, 0x12,	2,	6),//00000C00
	R_U64L	= REGID(OPC_AUXREG, 0x10,	4,	2),//00000F00
	R_U64H	= REGID(OPC_AUXREG, 0x14,	4,	6),//0000F000

	R_V16L	= REGID(OPC_AUXREG, 0x20,	1,	3),//00010000
	R_V16H	= REGID(OPC_AUXREG, 0x21,	1,	7),//00020000
	R_V32L	= REGID(OPC_AUXREG, 0x20,	2,	3),//00030000
	R_V32H	= REGID(OPC_AUXREG, 0x22,	2,	7),//000C0000
	R_V64L	= REGID(OPC_AUXREG, 0x20,	4,	3),//000F0000
	R_V64H	= REGID(OPC_AUXREG, 0x24,	4,	7),//00F00000

	R_W16L	= REGID(OPC_AUXREG, 0x30,	1,	4),//01000000
	R_W16H	= REGID(OPC_AUXREG, 0x31,	1,	8),//02000000
	R_W32L	= REGID(OPC_AUXREG, 0x30,	2,	4),//03000000
	R_W32H	= REGID(OPC_AUXREG, 0x32,	2,	8),//0C000000
	R_W64L	= REGID(OPC_AUXREG, 0x30,	4,	4),//0F000000
	R_W64H	= REGID(OPC_AUXREG, 0x34,	4,	8),//F0000000
};

enum Pfx_e {
	PFX_NULL,
	PFX_REP,
	PFX_REPE,		//repeat while equal
	PFX_REPNE,		//repeat while not equal
};






