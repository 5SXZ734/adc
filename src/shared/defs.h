
#ifndef __DEFS_H__
#define __DEFS_H__

#include "config.h"
#include <stdint.h>

#pragma warning(disable:4996)

typedef uint64_t			ADDR64;
typedef uint32_t			ADDR;
typedef uint32_t			ADDR_RANGE;
typedef uint64_t			ROWID;

typedef const char *		PDATA;

#define	ROWID_INVALID		((ROWID)-1)
#define BAD_ADDR	0
#define NAME_LEN_MAX	64

typedef uint64_t			OFF_t;	//position/size in file
//typedef unsigned __int64		OFF_DIFF_t;
#define	OFF_NULL	OFF_t(-1)

enum SSID_t//storage space id
{//be sure this sequence corresponds to STORAGE_t::SS element's definitions!
	SSID_NULL,				//0
	SSID_IMM = SSID_NULL,	//0
	SSID_GLOBAL = SSID_IMM,	//0
	SSID__1,		//1
	SSID_AUXREG,	//2
	SSID_CPUREG,	//3
SSID_FTOPREG = SSID_CPUREG,
	SSID_FPUREG,	//4
	SSID_SEGREG,	//5
	SSID_CPUSW,		//6
	SSID_FPUSW,		//7
	SSID_FPUCW,		//8
	SSID_DBGREG,	//9
	SSID_CTRLREG,	//a
	SSID_TESTREG,	//b
	SSID_MMXREG,	//c
	SSID_KREG,		//d
	SSID_SIMD,		//e
	SSID_LOCAL,		//f			last to comply with arg allocation order
	SSID_TOTAL,		//0x10!

	SSID_THISPTR	= 0x8000,
};

enum OPC_t 
{
	OPC_NULL	= SSID_NULL,	//0:???
	OPC_IMM		= SSID_IMM,		//0:scalar or (?)address
	OPC_GLOBAL	= SSID_GLOBAL,	//0
	OPC__1		= SSID__1,	//1:
	OPC_AUXREG	= SSID_AUXREG,	//2:
	OPC_CPUREG	= SSID_CPUREG,	//3:
OPC_FTOPREG	= SSID_FTOPREG,	//3:
	OPC_FPUREG	= SSID_FPUREG,	//4:
	OPC_SEGREG	= SSID_SEGREG,	//5: CPU segment register
	OPC_CPUSW	= SSID_CPUSW,	//6:CPU status word
	OPC_FPUSW	= SSID_FPUSW,	//7:FPU status word
	OPC_FPUCW	= SSID_FPUCW,	//8:FPU control word
	OPC_DBGREG	= SSID_DBGREG,	//9
	OPC_CTRLREG	= SSID_CTRLREG,	//a
	OPC_TESTREG	= SSID_TESTREG,	//b
	OPC_MMXREG	= SSID_MMXREG,	//c	(!) OPC_FPUREG
	OPC_KREG	= SSID_KREG,		//d AVX512 opmask registers (16 bit wide?)
	OPC_SIMD	= SSID_SIMD,	//d	=> XMM/YMM/ZMM - SIZE IS A POW2!
	OPC_LOCAL	= SSID_LOCAL,	//f

	OPC_CS = 0x10,
	OPC_DS = 0x20,
	OPC_ES = 0x30,
	OPC_SS = 0x40,
	OPC_FS = 0x50,
	OPC_GS = 0x60,

	OPC_INDIRECT = 0x70,	//memory reference (combined with OPC_IMM/OPC_CPUREG/OPC_AUXREG)
	OPC_ADDRESS = 0x80,
	OPC_FLAGS = 0x80,

	OPC__LAST = 0xFF
};

#define FOPC_CPUSW	OPC_FLAGS|OPC_CPUSW
#define IS_ADDR(a)	(((a) & OPC_ADDRESS) && (((a) & 0xF) != OPC_CPUSW))

enum OpType_t
{
	//xxxxxxxx
	//    ^^^^:(0-3)size
	//^^^^----:(4-7)type
	OPTYP_NULL		= 0,
	OPTYP_VOID		= OPTYP_NULL,	//for fields with uncertain types

	OPSZ_NULL		= OPTYP_NULL,
	OPSZ_BIT		= 1,
	OPSZ_BYTE		= 1,
	OPSZ_WORD		= 2,
	OPSZ_DWORD		= 4,
	OPSZ_FWORD		= 6,
	OPSZ_QWORD		= 8,
	//OPSZ_DQWORD		= 0x10,
	OPSZ_TWORD		= 0xA,
	OPSZ_MASK		= 0xF,

	//OPTYP_BIT		= 0x10,//available

	OPTYP_REAL0		= 0x20,	//0x20
	OPTYP_REAL32	= OPTYP_REAL0|OPSZ_DWORD,	//0x24
	OPTYP_REAL64	= OPTYP_REAL0|OPSZ_QWORD,	//0x28
	OPTYP_REAL80	= OPTYP_REAL0|OPSZ_TWORD,	//0x2A

	OPTYP_BCD0		= 0x30,	//0x30
	OPTYP_BCD80		= OPTYP_BCD0|OPSZ_TWORD,	//0x3A	//BCD80

	OPTYP_INTEGER	= 0x40,	//some integer - either signed or unsigned - dunno..
	OPTYP_BYTE		= OPTYP_INTEGER|OPSZ_BYTE,
	OPTYP_WORD		= OPTYP_INTEGER|OPSZ_WORD,
	OPTYP_DWORD		= OPTYP_INTEGER|OPSZ_DWORD,
	OPTYP_QWORD		= OPTYP_INTEGER|OPSZ_QWORD,

	OPTYP_BOOL		= 0x50|OPSZ_BYTE,
	OPTYP_CHAR0		= 0x60,
	OPTYP_CHAR8		= OPTYP_CHAR0|OPSZ_BYTE,
	OPTYP_CHAR16	= OPTYP_CHAR0|OPSZ_WORD,
	OPTYP_CHAR32	= OPTYP_CHAR0|OPSZ_DWORD,

	OPTYP_UINT0		= 0x70,	//unsigned
	OPTYP_UINT8		= OPTYP_UINT0|OPSZ_BYTE,		//0x51
	OPTYP_UINT16	= OPTYP_UINT0|OPSZ_WORD,		//0x52
	OPTYP_UINT32	= OPTYP_UINT0|OPSZ_DWORD,	//0x54
	OPTYP_UINT64	= OPTYP_UINT0|OPSZ_QWORD,	//0x08

	OPTYP_INT0		= 0x80, //signed
	OPTYP_INT8		= OPTYP_INT0|OPSZ_BYTE,	//0x61
	OPTYP_INT16		= OPTYP_INT0|OPSZ_WORD,	//0x62
	OPTYP_INT32		= OPTYP_INT0|OPSZ_DWORD,	//0x64
	OPTYP_INT64		= OPTYP_INT0|OPSZ_QWORD,	//0x68

	OPTYP_NOT_INTEGER = 0x90,

	//some ptr
	OPTYP_PTR		= 0xE0,
	OPTYP_PTR_FAR	= 0xF0,

	//ptrs: offset only
	OPTYP_PTR16		= OPTYP_PTR|OPSZ_WORD,
	OPTYP_PTR32		= OPTYP_PTR|OPSZ_DWORD,
	OPTYP_PTR64		= OPTYP_PTR|OPSZ_QWORD,
	//ptrs: seg:offset
	OPTYP_PTR16_FAR	= OPTYP_PTR_FAR|OPSZ_DWORD,
	OPTYP_PTR32_FAR	= OPTYP_PTR_FAR|OPSZ_FWORD,
	OPTYP_PTR64_FAR	= OPTYP_PTR_FAR|OPSZ_TWORD,

	OPTYP_MASK		= 0xF0
};

#define OPSIZE(a)			((a) & OPSZ_MASK)
#define OPTYPE(a)			((a) & OPTYP_MASK)
#define MAKETYP(t, s)		OpType_t(OPTYPE(t) | OPSIZE(s))
#define MAKETYP_PTR(s)		MAKETYP(OPTYP_PTR, s)
#define MAKETYP_UINT(s)		MAKETYP(OPTYP_UINT0, OPSIZE(s))
#define MAKETYP_SINT(s)		MAKETYP(OPTYP_INT0, OPSIZE(s))

#define OPTYP_IS_REAL(a)	(((a) & OPTYP_MASK) == OPTYP_REAL0)
#define OPTYP_IS_PTR(a)		(OPTYPE(a) == OPTYP_PTR)
#define OPTYP_IS_INT(a)		(OPTYP_INTEGER <= OPTYPE(a) && OPTYPE(a) < OPTYP_NOT_INTEGER)//any
#define OPTYP_IS_SINT(a)		(OPTYPE(a) == OPTYP_INT0)//signed
#define OPTYP_IS_UINT(a)		(OPTYPE(a) == OPTYP_UINT0)//unsigned

#define NAMELENMAX	32

enum Dist_e {
	DIST_NULL,
	DIST_FAR,
	DIST_NEAR,
	DIST_SHORT,
};

enum OPCONST_e {
	OPID_0 = 1,
	OPID_1,
//	OPID_CF0,
//	OPID_CF1,
	OPID_0R64,
	OPID_1R64,
	OPID_L2T,			//log2(10)
	OPID_L2E,			//log2(e)
	OPID_PI,			//pi
	OPID_LG2,			//log10(2)
	OPID_LN2,			//loge(2)
};


enum Seg_e {
	SEG_NULL,
	SEG_CODE,
	SEG_DATA,		//uninitialized datas
	SEG_STACK,
	SEG_UNK,
	SEG_RET,
	/////////
	SEG_CONST	= SEG_DATA | 0x10,
};

#ifdef _DEBUG
#define CHECK(arg)  if(arg) 
#define CHECKID(obj,id)  if(obj && obj->checkId(id)) 
#define STOP { int a = 0; (void)a; }
#define STOP2(arg) { arg; }
#define STOPx { throw (0); }
#else
#define CHECK(arg)
#define CHECKID(obj,id)
#define STOP
#define STOP2(arg)
#define STOPx
#endif

#define ASSERT0	{assert(0); abort();}

#undef TRACE1
#define TRACE1
#undef TRACE2
#define TRACE2


#define REGID32(sz, ofs)	((((uint32_t)((uint16_t)sz)) << 16) + (uint16_t)ofs)

#if(X64_SUPPORT)
typedef uint64_t	RegMaskType;
typedef uint64_t	FlagMaskType;
#else
typedef uint32_t	RegMaskType;
typedef uint32_t	FlagMaskType;
#endif

inline RegMaskType TOREGMASK(uint8_t ofs, uint8_t siz)
{
	return ((~((~RegMaskType(0)) << siz)) << ofs);
}

struct r_t
{
	uint8_t	ssid;
	uint8_t	ofs;
	union {
		uint8_t	typ;
		uint8_t	siz;//:4;
	};
	
	r_t(){}
	r_t(uint8_t _ssid, uint8_t _ofs, uint8_t _typ)
		: ssid(_ssid),
		ofs(_ofs),
		typ(_typ)
	{
	}
	bool operator==(const r_t &r) const {
		return (ssid == r.ssid && ofs == r.ofs && siz == r.siz);
	}
};

struct ri_t
{
	SSID_t	ssid;
	int		ofs;
	union {
		OpType_t	typ;
		uint8_t	siz:4;
	};
	ri_t(){}
	ri_t(uint8_t _ssid, int _ofs, uint8_t _typ)
		: ssid((SSID_t)_ssid),
		ofs(_ofs),
		typ((OpType_t)_typ)
	{
	}
	bool operator==(const ri_t &o) const {
		return (ssid == o.ssid && ofs == o.ofs && typ == o.typ);
	}
	bool operator!=(const ri_t& o) const {
		return !operator==(o);
	}
};

#define SSID_MASK	0xf

#endif//__DEFS_H__


