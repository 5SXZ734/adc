#include "front_IA.h"

#include <assert.h>
#include <stdio.h>
#include "shared.h"
#include "shared/defs.h"
#include "shared/front.h"
#include "shared/misc.h"
#include "x86_IR.h"



/////////////////////////////////////////
// O P C _ C P U R E G

const RegInfo_t RegTable[] = {
	//8-bit
	{ "AL", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 0 },
	{ "AH", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 0 + 1 },
	{ "CL", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 1 },
	{ "CH", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 1 + 1 },
	{ "DL", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 2 },
	{ "DH", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 2 + 1 },
	{ "BL", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 3 },
	{ "BH", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 3 + 1 },
#if(X64_SUPPORT)
	{ "SPL", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 4 },
	{ "BPL", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 5 },
	{ "SIL", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 6 },
	{ "DIL", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 7 },
	{ "R8B", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 8 },
	{ "R9B", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 9 },
	{ "R10B", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 10 },
	{ "R11B", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 11 },
	{ "R12B", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 12 },
	{ "R13B", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 13 },
	{ "R14B", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 14 },
	{ "R15B", OPC_CPUREG, OPSZ_BYTE, GPR_SLOT_SIZE * 15 },
#endif

	//16-bit
	{ "AX", OPC_CPUREG, OPSZ_WORD, GPR_SLOT_SIZE * 0 },
	{ "CX", OPC_CPUREG, OPSZ_WORD, GPR_SLOT_SIZE * 1 },
	{ "DX", OPC_CPUREG, OPSZ_WORD, GPR_SLOT_SIZE * 2 },
	{ "BX", OPC_CPUREG, OPSZ_WORD, GPR_SLOT_SIZE * 3 },
	{ "SP", OPC_CPUREG, OPSZ_WORD, GPR_SLOT_SIZE * 4 },
	{ "BP", OPC_CPUREG, OPSZ_WORD, GPR_SLOT_SIZE * 5 },
	{ "SI", OPC_CPUREG, OPSZ_WORD, GPR_SLOT_SIZE * 6 },
	{ "DI", OPC_CPUREG, OPSZ_WORD, GPR_SLOT_SIZE * 7 },
#if(X64_SUPPORT)
	{ "R8W", OPC_CPUREG, OPSZ_WORD, GPR_SLOT_SIZE * 8 },
	{ "R9W", OPC_CPUREG, OPSZ_WORD, GPR_SLOT_SIZE * 9 },
	{ "R10W", OPC_CPUREG, OPSZ_WORD, GPR_SLOT_SIZE * 10 },
	{ "R11W", OPC_CPUREG, OPSZ_WORD, GPR_SLOT_SIZE * 11 },
	{ "R12W", OPC_CPUREG, OPSZ_WORD, GPR_SLOT_SIZE * 12 },
	{ "R13W", OPC_CPUREG, OPSZ_WORD, GPR_SLOT_SIZE * 13 },
	{ "R14W", OPC_CPUREG, OPSZ_WORD, GPR_SLOT_SIZE * 14 },
	{ "R15W", OPC_CPUREG, OPSZ_WORD, GPR_SLOT_SIZE * 15 },
#endif
	{ "IP", OPC_CPUREG, OPSZ_WORD, GPR_SLOT_SIZE * 16 },

	//32-bit
	{ "EAX", OPC_CPUREG, OPSZ_DWORD, GPR_SLOT_SIZE * 0 },
	{ "ECX", OPC_CPUREG, OPSZ_DWORD, GPR_SLOT_SIZE * 1 },
	{ "EDX", OPC_CPUREG, OPSZ_DWORD, GPR_SLOT_SIZE * 2 },
	{ "EBX", OPC_CPUREG, OPSZ_DWORD, GPR_SLOT_SIZE * 3 },
	{ "ESP", OPC_CPUREG, OPSZ_DWORD, GPR_SLOT_SIZE * 4 },
	{ "EBP", OPC_CPUREG, OPSZ_DWORD, GPR_SLOT_SIZE * 5 },
	{ "ESI", OPC_CPUREG, OPSZ_DWORD, GPR_SLOT_SIZE * 6 },
	{ "EDI", OPC_CPUREG, OPSZ_DWORD, GPR_SLOT_SIZE * 7 },
#if(X64_SUPPORT)
	{ "R8D", OPC_CPUREG, OPSZ_DWORD, GPR_SLOT_SIZE * 8 },
	{ "R9D", OPC_CPUREG, OPSZ_DWORD, GPR_SLOT_SIZE * 9 },
	{ "R10D", OPC_CPUREG, OPSZ_DWORD, GPR_SLOT_SIZE * 10 },
	{ "R11D", OPC_CPUREG, OPSZ_DWORD, GPR_SLOT_SIZE * 11 },
	{ "R12D", OPC_CPUREG, OPSZ_DWORD, GPR_SLOT_SIZE * 12 },
	{ "R13D", OPC_CPUREG, OPSZ_DWORD, GPR_SLOT_SIZE * 13 },
	{ "R14D", OPC_CPUREG, OPSZ_DWORD, GPR_SLOT_SIZE * 14 },
	{ "R15D", OPC_CPUREG, OPSZ_DWORD, GPR_SLOT_SIZE * 15 },
#endif
	{ "EIP", OPC_CPUREG, OPSZ_DWORD, GPR_SLOT_SIZE * 16 },

#if(X64_SUPPORT)
	//64-bit
	{ "RAX", OPC_CPUREG, OPSZ_QWORD, GPR_SLOT_SIZE * 0 },
	{ "RCX", OPC_CPUREG, OPSZ_QWORD, GPR_SLOT_SIZE * 1 },
	{ "RDX", OPC_CPUREG, OPSZ_QWORD, GPR_SLOT_SIZE * 2 },
	{ "RBX", OPC_CPUREG, OPSZ_QWORD, GPR_SLOT_SIZE * 3 },
	{ "RSP", OPC_CPUREG, OPSZ_QWORD, GPR_SLOT_SIZE * 4 },
	{ "RBP", OPC_CPUREG, OPSZ_QWORD, GPR_SLOT_SIZE * 5 },
	{ "RSI", OPC_CPUREG, OPSZ_QWORD, GPR_SLOT_SIZE * 6 },
	{ "RDI", OPC_CPUREG, OPSZ_QWORD, GPR_SLOT_SIZE * 7 },
	{ "R8", OPC_CPUREG, OPSZ_QWORD, GPR_SLOT_SIZE * 8 },
	{ "R9", OPC_CPUREG, OPSZ_QWORD, GPR_SLOT_SIZE * 9 },
	{ "R10", OPC_CPUREG, OPSZ_QWORD, GPR_SLOT_SIZE * 10 },
	{ "R11", OPC_CPUREG, OPSZ_QWORD, GPR_SLOT_SIZE * 11 },
	{ "R12", OPC_CPUREG, OPSZ_QWORD, GPR_SLOT_SIZE * 12 },
	{ "R13", OPC_CPUREG, OPSZ_QWORD, GPR_SLOT_SIZE * 13 },
	{ "R14", OPC_CPUREG, OPSZ_QWORD, GPR_SLOT_SIZE * 14 },
	{ "R15", OPC_CPUREG, OPSZ_QWORD, GPR_SLOT_SIZE * 15 },
	//RIP
	{ "RIP", OPC_CPUREG, OPSZ_QWORD, GPR_SLOT_SIZE * 16 },
#endif

	//mmx
	{ "MM0", OPC_MMXREG, MMXR_SIZE, MMXR_SLOT_SIZE * 0 },
	{ "MM1", OPC_MMXREG, MMXR_SIZE, MMXR_SLOT_SIZE * 1 },
	{ "MM2", OPC_MMXREG, MMXR_SIZE, MMXR_SLOT_SIZE * 2 },
	{ "MM3", OPC_MMXREG, MMXR_SIZE, MMXR_SLOT_SIZE * 3 },
	{ "MM4", OPC_MMXREG, MMXR_SIZE, MMXR_SLOT_SIZE * 4 },
	{ "MM5", OPC_MMXREG, MMXR_SIZE, MMXR_SLOT_SIZE * 5 },
	{ "MM6", OPC_MMXREG, MMXR_SIZE, MMXR_SLOT_SIZE * 6 },
	{ "MM7", OPC_MMXREG, MMXR_SIZE, MMXR_SLOT_SIZE * 7 },

	//XMM
	{ "XMM0", OPC_SIMD, XMMR_SIZE, XMMR_SLOT_SIZE * 0 },
	{ "XMM1", OPC_SIMD, XMMR_SIZE, XMMR_SLOT_SIZE * 1 },
	{ "XMM2", OPC_SIMD, XMMR_SIZE, XMMR_SLOT_SIZE * 2 },
	{ "XMM3", OPC_SIMD, XMMR_SIZE, XMMR_SLOT_SIZE * 3 },
	{ "XMM4", OPC_SIMD, XMMR_SIZE, XMMR_SLOT_SIZE * 4 },
	{ "XMM5", OPC_SIMD, XMMR_SIZE, XMMR_SLOT_SIZE * 5 },
	{ "XMM6", OPC_SIMD, XMMR_SIZE, XMMR_SLOT_SIZE * 6 },
	{ "XMM7", OPC_SIMD, XMMR_SIZE, XMMR_SLOT_SIZE * 7 },
	{ "XMM8", OPC_SIMD, XMMR_SIZE, XMMR_SLOT_SIZE * 8 },
	{ "XMM9", OPC_SIMD, XMMR_SIZE, XMMR_SLOT_SIZE * 9 },
	{ "XMM10", OPC_SIMD, XMMR_SIZE, XMMR_SLOT_SIZE * 10 },
	{ "XMM11", OPC_SIMD, XMMR_SIZE, XMMR_SLOT_SIZE * 11 },
	{ "XMM12", OPC_SIMD, XMMR_SIZE, XMMR_SLOT_SIZE * 12 },
	{ "XMM13", OPC_SIMD, XMMR_SIZE, XMMR_SLOT_SIZE * 13 },
	{ "XMM14", OPC_SIMD, XMMR_SIZE, XMMR_SLOT_SIZE * 14 },
	{ "XMM15", OPC_SIMD, XMMR_SIZE, XMMR_SLOT_SIZE * 15 },

	//YMM
	{ "YMM0", OPC_SIMD, YMMR_SIZE, YMMR_SLOT_SIZE * 0 },
	{ "YMM1", OPC_SIMD, YMMR_SIZE, YMMR_SLOT_SIZE * 1 },
	{ "YMM2", OPC_SIMD, YMMR_SIZE, YMMR_SLOT_SIZE * 2 },
	{ "YMM3", OPC_SIMD, YMMR_SIZE, YMMR_SLOT_SIZE * 3 },
	{ "YMM4", OPC_SIMD, YMMR_SIZE, YMMR_SLOT_SIZE * 4 },
	{ "YMM5", OPC_SIMD, YMMR_SIZE, YMMR_SLOT_SIZE * 5 },
	{ "YMM6", OPC_SIMD, YMMR_SIZE, YMMR_SLOT_SIZE * 6 },
	{ "YMM7", OPC_SIMD, YMMR_SIZE, YMMR_SLOT_SIZE * 7 },
	/*{ "YMM8", OPC_SIMD, YMMR_SIZE, YMMR_SLOT_SIZE * 8 },
	{ "YMM9", OPC_SIMD, YMMR_SIZE, YMMR_SLOT_SIZE * 9 },
	{ "YMM10", OPC_SIMD, YMMR_SIZE, YMMR_SLOT_SIZE * 10 },
	{ "YMM11", OPC_SIMD, YMMR_SIZE, YMMR_SLOT_SIZE * 11 },
	{ "YMM12", OPC_SIMD, YMMR_SIZE, YMMR_SLOT_SIZE * 12 },
	{ "YMM13", OPC_SIMD, YMMR_SIZE, YMMR_SLOT_SIZE * 13 },
	{ "YMM14", OPC_SIMD, YMMR_SIZE, YMMR_SLOT_SIZE * 14 },
	{ "YMM15", OPC_SIMD, YMMR_SIZE, YMMR_SLOT_SIZE * 15 },*///avoid byte overflow!

	//seg
	{ "CS", OPC_SEGREG, SR_SLOT_SIZE, SR_SLOT_SIZE * 0 },
	{ "DS", OPC_SEGREG, SR_SLOT_SIZE, SR_SLOT_SIZE * 1 },
	{ "ES", OPC_SEGREG, SR_SLOT_SIZE, SR_SLOT_SIZE * 2 },
	{ "SS", OPC_SEGREG, SR_SLOT_SIZE, SR_SLOT_SIZE * 3 },
	{ "FS", OPC_SEGREG, SR_SLOT_SIZE, SR_SLOT_SIZE * 4 },
	{ "GS", OPC_SEGREG, SR_SLOT_SIZE, SR_SLOT_SIZE * 5 },

	//FPU
	{ "ST0", OPC_FPUREG, FR_SIZE, FR_SLOT_SIZE * 0 },
	{ "ST1", OPC_FPUREG, FR_SIZE, FR_SLOT_SIZE * 1 },
	{ "ST2", OPC_FPUREG, FR_SIZE, FR_SLOT_SIZE * 2 },
	{ "ST3", OPC_FPUREG, FR_SIZE, FR_SLOT_SIZE * 3 },
	{ "ST4", OPC_FPUREG, FR_SIZE, FR_SLOT_SIZE * 4 },
	{ "ST5", OPC_FPUREG, FR_SIZE, FR_SLOT_SIZE * 5 },
	{ "ST6", OPC_FPUREG, FR_SIZE, FR_SLOT_SIZE * 6 },
	{ "ST7", OPC_FPUREG, FR_SIZE, FR_SLOT_SIZE * 7 },

	//CPUSW flags
	{ "CF", OPC_CPUSW, 1, 0 },		// 0x0001	Carry flag
	{ "PF", OPC_CPUSW, 1, 2 },		// 0x0004	Parity flag
	{ "AF", OPC_CPUSW, 1, 4 },		// 0x0010	Adjust flag
	{ "ZF", OPC_CPUSW, 1, 6 },		// 0x0040	Zero flag
	{ "SF", OPC_CPUSW, 1, 7 },		// 0x0080	Sign flag
	{ "TF", OPC_CPUSW, 1, 8 },		// 0x0100	Trap flag
	{ "IF", OPC_CPUSW, 1, 9 },		// 0x0200	Interrupt enable flag
	{ "DF", OPC_CPUSW, 1, 10 },		// 0x0400	Direction flag
	{ "OF", OPC_CPUSW, 1, 11 },		// 0x0800	Overflow flag
	{ "IOPL", OPC_CPUSW, 2, 12 },	// 0x3000	I/O privilege level (286+ only), always 1 on 8086 and 186
	{ "NT", OPC_CPUSW, 1, 14 },		// 0x4000	Nested task flag (286+ only), always 1 on 8086 and 186
#if(X64_SUPPORT)
	{ "RF", OPC_CPUSW, 1, 16 },		//  0x0001 0000	Resume flag(386 + only)	
	{ "VM", OPC_CPUSW, 1, 17 },		//	0x0002 0000	Virtual 8086 mode flag(386 + only)	
	{ "AC", OPC_CPUSW, 1, 18 },		//	0x0004 0000	Alignment check(486SX + only)	
	{ "VIF", OPC_CPUSW, 1, 19 },	//	0x0008 0000	Virtual interrupt flag(Pentium + )	
	{ "VIP", OPC_CPUSW, 1, 20 },	//	0x0010 0000	Virtual interrupt pending(Pentium + )	
	{ "ID", OPC_CPUSW, 1, 21 },		//	0x0020 0000	Able to use CPUID instruction(Pentium + )	
#endif

	//FPUSW flags
	{ "IE", OPC_FPUSW, 1, 0 },		// 0x0001	INVALID OPERATION
	{ "DE", OPC_FPUSW, 1, 1 },		// 0x0002	DENORMALIZED OPERAND
	{ "ZE", OPC_FPUSW, 1, 2 },		// 0x0004	ZERO DIVIDE
	{ "OE", OPC_FPUSW, 1, 3 },		// 0x0008	OVERFLOW
	{ "UE", OPC_FPUSW, 1, 4 },		// 0x0010	UNDERFLOW
	{ "PE", OPC_FPUSW, 1, 5 },		// 0x0020	PRECISION
	{ "SF", OPC_FPUSW, 1, 6 },		// 0x0040	STACK FAULT
	{ "ES", OPC_FPUSW, 1, 7 },		// 0x0080	ERROR SUMMARY STATUS
	{ "C0", OPC_FPUSW, 1, 8 },		// 0x0100	CONDITION CODE(8)
	{ "C1", OPC_FPUSW, 1, 9 },		// 0x0200	CONDITION CODE(9)
	{ "C2", OPC_FPUSW, 1, 10 },		// 0x0400	CONDITION CODE(10)
	{ "TOP", OPC_FPUSW, 3, 11 },	// 0x3800	TOP OF STACK POINTER
	{ "C3", OPC_FPUSW, 1, 14 },		// 0x4000	ONDITION CODE(14)
	{ "B", OPC_FPUSW, 1, 15 },		// 0x8000	FPU BUSY

	//FPUCW flags
	{ "IM", OPC_FPUCW, 1, 0 },	// 0x0001	INVALID OPERATION
	{ "DM", OPC_FPUCW, 1, 1 },	// 0x0002	DENORMALIZED OPERAND
	{ "ZM", OPC_FPUCW, 1, 2 },	// 0x0004	ZERO DIVIDE
	{ "OM", OPC_FPUCW, 1, 3 },	// 0x0008	OVERFLOW
	{ "UM", OPC_FPUCW, 1, 4 },	// 0x0010	UNDERFLOW
	{ "PM", OPC_FPUCW, 1, 5 },	// 0x0020	PRECISION
	{ "PC", OPC_FPUCW, 2, 8 },	// 0x0300	PRECISION CONTROL
	{ "RC", OPC_FPUCW, 2, 10 },	// 0x0C00	ROUNDING CONTROL

	//K registers
	{ "K0", OPC_KREG, KR_SLOT_SIZE, KR_SLOT_SIZE * 0 },
	{ "K1", OPC_KREG, KR_SLOT_SIZE, KR_SLOT_SIZE * 1 },
	{ "K2", OPC_KREG, KR_SLOT_SIZE, KR_SLOT_SIZE * 2 },
	{ "K3", OPC_KREG, KR_SLOT_SIZE, KR_SLOT_SIZE * 3 },
	{ "K4", OPC_KREG, KR_SLOT_SIZE, KR_SLOT_SIZE * 4 },
	{ "K5", OPC_KREG, KR_SLOT_SIZE, KR_SLOT_SIZE * 5 },
	{ "K6", OPC_KREG, KR_SLOT_SIZE, KR_SLOT_SIZE * 6 },
	{ "K7", OPC_KREG, KR_SLOT_SIZE, KR_SLOT_SIZE * 7 },

	{ nullptr }
};

const char *ToRegName(OPC_t opc, unsigned ofs, unsigned sz)
{
	static RegInfoLookup_t _lookup(RegTable);
	return _lookup.toRegName(opc, (uint8_t)ofs, (uint8_t)sz);
}




//////////////////////////////////////////////////////
// S S I D _ F P U R E G

const char *IAFrontDC_t::FPUREG_reg2str(int sz, int ofs, int flags, char buf[80])
{
	/*static char *r64[8]	= {"F0","F1","F2","F3","F4","F5","F6","F7"};
		char *str = 0;
		if (ofs >= 0)
		if (ofs < 64)
		{
		assert(!(ofs & 7));
		str = r64[ofs >> 3];
		}*/

	int ofsAbs(abs(ofs));
	int d(ofsAbs / FR_SLOT_SIZE);
	int r(ofsAbs % FR_SLOT_SIZE);

	if (ofs >= 0)
		strcpy(buf, "F");//args
	else
		strcpy(buf, "G");//vars

	if (flags & 1)
	{
		//strlwr(g_buf);
		char *p(buf);
		for (; *p; ++p) *p = (char)tolower(*p);
	}

	sprintf(buf + strlen(buf), "%d", d);
	if (r)
	{
		strcat(buf, ".");
		sprintf(buf + strlen(buf), "%d", r);
	}

	if (sz != FR_SIZE)
	{
		strcat(buf, ":");
		sprintf(buf + strlen(buf), "%d", sz);
	}

	return buf;
}




///////////////////////// IAFrontDC_t

IAFrontDC_t::IAFrontDC_t()
{
	m.addStorage(SSID_GLOBAL, "GLOBAL", SS_NONE);
	m.addStorage(SSID_LOCAL, "LOCAL", SS_ARGUMENT | SS_STACKED);
	m.addStorage(SSID_AUXREG, "AUXREG", SS_DISCRETE);
}

const RegInfo_t *IAFrontDC_t::RegInfoTable() const
{
	return RegTable;
}

const char *IAFrontDC_t::RegName(SSID_t ssid, int sz, int ofs, int flags, char buf[80]) const
{
	switch (ssid)
	{
	case SSID_FPUREG:
		return FPUREG_reg2str(sz, ofs, flags, buf);
	default:
		break;
	}
	return nullptr;
}



///////////////////////////////////////////////////////////////// IA32FrontDC_t

namespace x86_32 { extern templ_t * g_list; }

IA32FrontDC_t::IA32FrontDC_t()
{
	FE_t &fe(m);

//	fe.flags = FE_X86_32;
	fe.name = "IA X86 (32 bit)";

	fe.addStorage(SSID_CPUREG, "CPUREG", SS_DISCRETE|SS_ARGUMENT|SS_RETURNED|SS_SPOILT);
	fe.addStorage(SSID_FPUREG, "FPUREG", SS_DISCRETE | SS_ARGUMENT | SS_RETURNED | SS_STACKED);
	fe.addStorage(SSID_CPUSW, "CSW", SS_FLAGS|SS_ARGUMENT|SS_RETURNED|SS_NOMLOC);
	fe.addStorage(SSID_FPUSW, "FSW", SS_FLAGS|SS_NOMLOC);
	fe.addStorage(SSID_FPUCW, "FPUCW", SS_FLAGS|SS_NOMLOC);
	fe.addStorage(SSID_SEGREG, "SEGREG", SS_DISCRETE | SS_NOMLOC);

#ifndef _DEMO
	fe.ITS = x86_32::g_list;
#endif

	static r_t stack_ptr[] = {
		{SSID_CPUREG, OFS(R_ESP), SIZ(R_ESP)},
		{SSID_CPUREG, OFS(R_EBP), SIZ(R_EBP)}//?
	};
	fe.stack_ptr = &stack_ptr[0];
	fe.stackb_ptr = &stack_ptr[1];

//#define R2MASK(a)	reg_t(OFS(a), SIZ(a)).REG2MASK()

	static r_t spoilt_regs_check[] = {
		{OPC_CPUREG, OFS(R_EAX), SIZ(R_EAX)},
		{OPC_CPUREG, OFS(R_ECX), SIZ(R_ECX)},
		{OPC_CPUREG, OFS(R_EDX), SIZ(R_EDX)},
		{OPC_CPUREG, OFS(R_EBX), SIZ(R_EBX)},
		//{OPC_CPUREG, OFS(R_ESP), SIZ(R_ESP)},
		{OPC_CPUREG, OFS(R_EBP), SIZ(R_EBP)},
		{OPC_CPUREG, OFS(R_ESI), SIZ(R_ESI)},
		{OPC_CPUREG, OFS(R_EDI), SIZ(R_EDI)},
		{0,0,0} };
	fe.spoilt_regs_check = spoilt_regs_check;

	static r_t spoilt_regs_def[] = {
		{OPC_CPUREG, OFS(R_EAX), SIZ(R_EAX)},
		{OPC_CPUREG, OFS(R_ECX), SIZ(R_ECX)},
		{OPC_CPUREG, OFS(R_EDX), SIZ(R_EDX)},
		//{OPC_CPUREG, OFS(R_ESP), SIZ(R_ESP)},
		{0,0,0} };
	fe.spoilt_regs_def = spoilt_regs_def;//ebx,ebp,esi,edi
		//R2MASK(R_EBX) | R2MASK(R_EBP) | R2MASK(R_ESI) | R2MASK(R_EDI);
	fe.spoilt_flags_def = 0;//CPUSW_MASK;

	fe.ptr_near = OPSZ_DWORD;
	fe.ptr_far = -1;//not allowed

	fe.stackaddrsz = OPSZ_DWORD;

	static const FE_t::stub_t fdefs[] = {
	{ "stos", "0 0 0 0 (edi,ecx) {ecx,edi}" },//280
	{ "movs", "0 0 0 0 (edi,esi,ecx) {ecx,esi!,edi!}" },//2780,87
	{ "cmps", "0 0 0 0 (edi,esi) {ecx,esi!,edi!}" },//780,87
//	{ "scas", "0 0 0 0 (edi,esi,ecx) {ecx,edi}"},//2780
	//{ "atan2", "0 0 2 1 () {}" },
	{ nullptr, nullptr }
	};

	fe.fdefs = fdefs;
}

const char *IA32FrontDC_t::Name() const
{
	return FENAME_PFX FRONT_NAME;
}

const char **IA32FrontDC_t::Symbols() const
{
	static const char *arr[] = { "_X86", "_WIN32", nullptr };
	return arr;
}


SSID_t IA32FrontDC_t::ArgSSID(unsigned index, unsigned fflags) const
{
	if (fflags & F_ThisCall)
	{
		if (index < 1)
			return SSID_CPUREG;//ECX
	}
	else if (fflags & F_FastCall)
	{
		if (index < 2)
			return SSID_CPUREG;//ECX and EDX
	}
	return SSID_LOCAL;
}

SSID_t IA32FrontDC_t::RetSSID(unsigned index, OpType_t retType) const
{
	if (index == 0)
	{
		if (retType == OPTYP_REAL80)
			return SSID_FPUREG;
		uint8_t opsz(retType & OPSZ_MASK);
		if (opsz == OPSZ_DWORD)
			return (SSID_t)RCL(R_EAX);
		if (opsz == OPSZ_WORD)
			return (SSID_t)RCL(R_AX);
		if (opsz == OPSZ_BYTE)
			return (SSID_t)RCL(R_AL);
	}
	return SSID_NULL;
}

unsigned IA32FrontDC_t::RetOff(unsigned index, OpType_t retType) const
{
	if (index == 0)
	{
		if (retType != OPTYP_REAL64)
		{
			if ((retType & OPSZ_MASK) == OPSZ_DWORD)
				return OFS(R_EAX);
		}
	}
	return 0;
}

unsigned IA32FrontDC_t::ArgOff(unsigned index, unsigned fflags) const
{
	int i(index);
	if (fflags & F_ThisCall)
	{
		if (index < 1)
			return OFS(R_ECX);
		i -= 1;
	}
	else if (fflags & F_FastCall)
	{
		if (index == 0)
			return OFS(R_ECX);
		if (index == 1)
			return OFS(R_EDX);
		i -= 2;
	}
	int ws(m.ptr_size);
	int off(ws + i * ws);//skip ret addr
	return off;
}

unsigned IA32FrontDC_t::ArgSize(unsigned index, unsigned u) const
{
	if ((u & 0xF) == 0)
		u = m.ptr_size;
	return u;
}



/////////////////////////////////////////////////////IA64FrontDC_t
namespace x86_64 { extern templ_t * g_list; }
IA64FrontDC_t::IA64FrontDC_t()
{
	FE_t &fe(m);

	//fe.flags = FE_X86_64;
	fe.name = "IA X86 (64 bit)";

	fe.addStorage(SSID_CPUREG, "CPUREG", SS_DISCRETE|SS_ARGUMENT|SS_RETURNED|SS_SPOILT);
	fe.addStorage(SSID_FPUREG, "FPUREG", SS_DISCRETE | SS_ARGUMENT | SS_RETURNED | SS_STACKED);
	fe.addStorage(SSID_CPUSW, "CSW", SS_FLAGS|SS_ARGUMENT|SS_RETURNED|SS_NOMLOC);
	fe.addStorage(SSID_FPUSW, "FSW", SS_FLAGS|SS_NOMLOC);
	fe.addStorage(SSID_FPUCW, "FPUCW", SS_FLAGS|SS_NOMLOC);
	fe.addStorage(SSID_SEGREG, "SEGREG", SS_DISCRETE | SS_NOMLOC);

#ifndef _DEMO
	fe.ITS = x86_64::g_list;
#endif

	static r_t stack_ptr[] = {
		{SSID_CPUREG, OFS(R_RSP), SIZ(R_RSP)},
		{SSID_CPUREG, OFS(R_RBP), SIZ(R_RBP)}//?
	};
	fe.stack_ptr = &stack_ptr[0];
	fe.stackb_ptr = &stack_ptr[1];

	static r_t spoilt_regs_check[] = {
		{OPC_CPUREG, OFS(R_RAX), SIZ(R_RAX)},
		{OPC_CPUREG, OFS(R_RCX), SIZ(R_RCX)},
		{OPC_CPUREG, OFS(R_RDX), SIZ(R_RDX)},
		{OPC_CPUREG, OFS(R_RBX), SIZ(R_RBX)},
		//{OPC_CPUREG, OFS(R_RSP), SIZ(R_RSP)},
		{OPC_CPUREG, OFS(R_RBP), SIZ(R_RBP)},
		{OPC_CPUREG, OFS(R_RSI), SIZ(R_RSI)},
		{OPC_CPUREG, OFS(R_RDI), SIZ(R_RDI)},
		{0,0,0} };
	fe.spoilt_regs_check = spoilt_regs_check;

	static r_t spoilt_regs_def[] = {
		{OPC_CPUREG, OFS(R_RAX), SIZ(R_RAX)},
		{OPC_CPUREG, OFS(R_RCX), SIZ(R_RCX)},
		{OPC_CPUREG, OFS(R_RDX), SIZ(R_RDX)},
		{0,0,0} };
	fe.spoilt_regs_def = spoilt_regs_def;
	fe.spoilt_flags_def = 0;//CPUSW_MASK;

	fe.ptr_near = OPSZ_QWORD;
	fe.ptr_far = -1;//not allowed

	fe.stackaddrsz = OPSZ_QWORD;

static const FE_t::stub_t fdefs[] = {
	//{ "stos", "0 0 0 0 (edi,ecx) {ecx,edi}" },//280
	//{ "movs", "0 0 0 0 (edi,esi,ecx) {ecx,esi!,edi!}" },//2780,87
	//{ "cmps", "0 0 0 0 (edi,esi) {ecx,esi!,edi!}" },//780,87
	{ nullptr, nullptr }
	};

	fe.fdefs = fdefs;
}

const char *IA64FrontDC_t::Name() const
{
	return FENAME_PFX FRONT_NAME;
}

const char **IA64FrontDC_t::Symbols() const
{
	static const char *arr[] = { "_X64", "_WIN64", nullptr };
	return arr;
}


SSID_t IA64FrontDC_t::ArgSSID(unsigned index, unsigned fflags) const
{
	if (index < 4)
		return SSID_CPUREG;//ECX
	return SSID_LOCAL;
}

unsigned IA64FrontDC_t::ArgOff(unsigned index, unsigned fflags) const
{
	unsigned i(index);
	if (index == 0)
		return OFS(R_RCX);
	if (index == 1)
		return OFS(R_RDX);
	if (index == 1)
		return OFS(R_R8);
	if (index == 1)
		return OFS(R_R9);
	i -= 4;
	unsigned ws(m.ptr_size);
	unsigned off(ws + i * ws);//skip ret addr
	return off;
}

unsigned IA64FrontDC_t::ArgSize(unsigned index, unsigned u) const
{
	if ((u & 0xF) == 0)
		u = m.ptr_size;
	return u;
}

SSID_t IA64FrontDC_t::RetSSID(unsigned index, OpType_t retType) const
{
	if (index == 0)
	{
		if (retType == OPTYP_REAL80)
			return SSID_FPUREG;
		uint8_t opsz(retType & OPSZ_MASK);
		if (opsz == OPSZ_QWORD)
			return (SSID_t)RCL(R_RAX);
		if (opsz == OPSZ_DWORD)
			return (SSID_t)RCL(R_EAX);
		if (opsz == OPSZ_WORD)
			return (SSID_t)RCL(R_AX);
		if (opsz == OPSZ_BYTE)
			return (SSID_t)RCL(R_AL);
	}
	return SSID_NULL;
}

unsigned IA64FrontDC_t::RetOff(unsigned index, OpType_t retType) const
{
	if (index == 0)
	{
		if (retType != OPTYP_REAL64)
		{
			if ((retType & OPSZ_MASK) == OPSZ_QWORD)
				return OFS(R_RAX);
		}
	}
	return 0;
}



