#pragma once

#include "config.h"
#include "shared/front.h"

//CPUREGS/////////////////////////////////////

#define SR_SLOT_SIZE	2

enum SREG_t {
	SREG_NULL = 0,
	R_CS	= REGID(OPC_SEGREG, SR_SLOT_SIZE*0,	2,	1), 
	R_DS	= REGID(OPC_SEGREG, SR_SLOT_SIZE*1,	2,	2),
	R_ES	= REGID(OPC_SEGREG, SR_SLOT_SIZE*2,	2,	3),
	R_SS	= REGID(OPC_SEGREG, SR_SLOT_SIZE*3,	2,	4),
	R_FS	= REGID(OPC_SEGREG, SR_SLOT_SIZE*4,	2,	5),
	R_GS	= REGID(OPC_SEGREG, SR_SLOT_SIZE*5,	2,	6)
};

#if(X64_SUPPORT)
#define GPR_SLOT_POW2	3
#else
#define GPR_SLOT_POW2	2
#endif
#define GPR_SLOT_SIZE	(1 << GPR_SLOT_POW2)

// 32BIT GPR MASK LAYOUT
//=======================
//      ---DI       ---SI
//--------EDI --------ESI
//31 30 29 28 27 26 25 24
/////////////////////////
//      ---BP       ---SP
//--------EBP --------ESP
//23 22 21 20 19 18 17 16
/////////////////////////
//      BH BL       AD DL
//      ---BX       ---DX
//--------EBX --------EDX
//15 14 13 12 11 10 09 08
/////////////////////////
//      CH CL       AH AL
//      ---CX       ---AX
//--------ECX --------EAX
//07 06 05 04 03 02 01 00


// 64BIT GPR MASK LAYOUT
//===============================================
//                   R15B                    R14B
//                  -R15W                   -R14W
//            -- -- -R15D             -- -- -R14D
//-- -- -- -- -- -- --R15 -- -- -- -- -- -- --R14
//7F 7E 7D 7C 7B 7A 79 78 77 76 75 74 73 72 71 70
/////////////////////////////////////////////////
//                   R13B                    R12B
//                  -R13W                   -R12W
//            -- -- -R13D             -- -- -R12D
//-- -- -- -- -- -- --R13 -- -- -- -- -- -- --R12
//6F 6E 6D 6C 6B 6A 69 68 67 66 65 64 63 62 61 60
/////////////////////////////////////////////////
//                   R11B                    R10B
//                  -R11W                   -R10W
//            -- -- -R11D             -- -- -R10D
//-- -- -- -- -- -- --R11 -- -- -- -- -- -- --R10
//5F 5E 5D 5C 5B 5A 59 58 57 56 55 54 53 52 51 50
/////////////////////////////////////////////////
//                    R8B                     R8B
//                  --R9W                   --R8W
//            -- -- --R9D             -- -- --R8D
//-- -- -- -- -- -- -- R9 -- -- -- -- -- -- -- R8
//4F 4E 4D 4C 4B 4A 49 48 47 46 45 44 43 42 41 40
//===============================================
//                    DIL                     SIL
//                  -- DI                   -- SI
//            -- -- --EDI             -- -- --ESI
//-- -- -- -- -- -- --RDI -- -- -- -- -- -- --RSI
//3F 3E 3D 3C 3B 3A 39 38 37 36 35 34 33 32 31 30
/////////////////////////////////////////////////
//                    BPL                     SPL
//                  ---BP                   ---SP
//            --------EBP             --------ESP
//--------------------RBP --------------------RSP
//2F 2E 2D 2C 2B 2A 29 28 27 26 25 24 23 22 21 20
/////////////////////////////////////////////////
//                  BH BL                   AD DL
//                  -- BX                   -- DX
//            -- -- --EBX             -- -- --EDX
//-- -- -- -- -- -- --RBX -- -- -- -- -- -- --RDX
//1F 1E 1D 1C 1B 1A 19 18 17 16 15 14 13 12 11 10
/////////////////////////////////////////////////
//                  CH CL                   AH AL
//                  -- CX                   -- AX
//            -- -- --ECX             -- -- --EAX
//-- -- -- -- -- -- --RCX -- -- -- -- -- -- --RAX
//0F 0E 0D 0C 0B 0A 09 08 07 06 05 04 03 02 01 00

enum {
	R_AL	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*0,	1,	1),//00000001
	R_AH	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*0+1,	1,	5),//00000002
	R_CL	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*1,	1,	2),//00000010
	R_CH	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*1+1,	1,	6),//00000020
	R_DL	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*2,	1,	3),//00000100
	R_DH	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*2+1,	1,	7),//00000200
	R_BL	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*3,	1,	4),//00001000
	R_BH	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*3+1,	1,	8),//00002000
#if(X64_SUPPORT)
	R_SPL	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*4,	1,	9),
	R_BPL	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*5,	1,	10),
	R_SIL	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*6,	1,	11),
	R_DIL	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*7,	1,	12),
	R_R8B	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*8,	1,	13),
	R_R9B	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*9,	1,	14),
	R_R10B	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*10,	1,	15),
	R_R11B	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*11,	1,	16),
	R_R12B	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*12,	1,	17),
	R_R13B	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*13,	1,	18),
	R_R14B	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*14,	1,	19),
	R_R15B	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*15,	1,	20),
#endif
	R8_LAST = -1
};

enum {
	R_AX	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*0,	2,	1),//00000003
	R_CX	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*1,	2,	2),//00000030
	R_DX	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*2,	2,	3),//00000300
	R_BX	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*3,	2,	4),//00003000
	R_SP	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*4,	2,	5),//00030000
	R_BP	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*5,	2,	6),//00300000
	R_SI	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*6,	2,	7),//03000000
	R_DI	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*7,	2,	8),//30000000
#if(X64_SUPPORT)
	R_R8W	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*8,	2,	9),
	R_R9W	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*9,	2,	10),
	R_R10W	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*10,	2,	11),
	R_R11W	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*11,	2,	12),
	R_R12W	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*12,	2,	13),
	R_R13W	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*13,	2,	14),
	R_R14W	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*14,	2,	15),
	R_R15W	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*15,	2,	16),
#endif
	R16_LAST = -1
};

enum {
	R_EAX	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*0,	4,	1),//0000000F
	R_ECX	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*1,	4,	2),//000000F0
	R_EDX	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*2,	4,	3),//00000F00
	R_EBX	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*3,	4,	4),//0000F000
	R_ESP	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*4,	4,	5),//000F0000
	R_EBP	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*5,	4,	6),//00F00000
	R_ESI	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*6,	4,	7),//0F000000
	R_EDI	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*7,	4,	8),//F0000000
#if(X64_SUPPORT)
	R_R8D	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*8,	4,	9),
	R_R9D	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*9,	4,	10),
	R_R10D	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*10,	4,	11),
	R_R11D	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*11,	4,	12),
	R_R12D	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*12,	4,	13),
	R_R13D	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*13,	4,	14),
	R_R14D	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*14,	4,	15),
	R_R15D	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*15,	4,	16),
#endif
	R32_LAST = -1
};

#if(X64_SUPPORT)
enum {
	R_RAX	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*0,	8,	1),
	R_RCX	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*1,	8,	2),
	R_RDX	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*2,	8,	3),
	R_RBX	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*3,	8,	4),
	R_RSP	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*4,	8,	5),
	R_RBP	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*5,	8,	6),
	R_RSI	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*6,	8,	7),
	R_RDI	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*7,	8,	8),
	R_R8	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*8,	8,	9),
	R_R9	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*9,	8,	10),
	R_R10	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*10,	8,	11),
	R_R11	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*11,	8,	12),
	R_R12	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*12,	8,	13),
	R_R13	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*13,	8,	14),
	R_R14	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*14,	8,	15),
	R_R15	= REGID(OPC_CPUREG, GPR_SLOT_SIZE*15,	8,	16),

	R_RIP		= REGID(OPC_CPUREG, GPR_SLOT_SIZE*16,	8,	17),
};
#endif

//some fake extra
/*enum {
#if(X64_SUPPORT)
	R_FPUTOP = REGID(OPC_CPUREG, GPR_SLOT_SIZE * 17, 0, 20)
#else
	R_FPUTOP = REGID(OPC_CPUREG, GPR_SLOT_SIZE * 8, 0, 20)
#endif
};*/

#define FR_SLOT_SIZE	0x10
#define FR_SIZE	0xA
#define FTOP_STEP		1
//#define FPU_ARG_SIZE(arg) ((arg)*OPSZ_QWORD)

enum {
	R_ST0 = REGID(OPC_FPUREG, FR_SLOT_SIZE*0, FR_SIZE,	1),
	R_ST1 = REGID(OPC_FPUREG, FR_SLOT_SIZE*1, FR_SIZE,	2),
	R_ST2 = REGID(OPC_FPUREG, FR_SLOT_SIZE*2, FR_SIZE,	3),
	R_ST3 = REGID(OPC_FPUREG, FR_SLOT_SIZE*3, FR_SIZE,	4),
	R_ST4 = REGID(OPC_FPUREG, FR_SLOT_SIZE*4, FR_SIZE,	5),
	R_ST5 = REGID(OPC_FPUREG, FR_SLOT_SIZE*5, FR_SIZE,	6),
	R_ST6 = REGID(OPC_FPUREG, FR_SLOT_SIZE*6, FR_SIZE,	7),
	R_ST7 = REGID(OPC_FPUREG, FR_SLOT_SIZE*7, FR_SIZE,	8)
};

#define MMXR_SLOT_SIZE	FR_SLOT_SIZE	//64bits, aliases for FPU ST0-ST7
#define MMXR_SIZE	FR_SIZE

// mmx registers
enum {
	R_MM0 = REGID(SSID_MMXREG, MMXR_SLOT_SIZE * 0, MMXR_SIZE, 1),
	R_MM1 = REGID(SSID_MMXREG, MMXR_SLOT_SIZE * 1, MMXR_SIZE, 2),
	R_MM2 = REGID(SSID_MMXREG, MMXR_SLOT_SIZE * 2, MMXR_SIZE, 3),
	R_MM3 = REGID(SSID_MMXREG, MMXR_SLOT_SIZE * 3, MMXR_SIZE, 4),
	R_MM4 = REGID(SSID_MMXREG, MMXR_SLOT_SIZE * 4, MMXR_SIZE, 5),
	R_MM5 = REGID(SSID_MMXREG, MMXR_SLOT_SIZE * 5, MMXR_SIZE, 6),
	R_MM6 = REGID(SSID_MMXREG, MMXR_SLOT_SIZE * 6, MMXR_SIZE, 7),
	R_MM7 = REGID(SSID_MMXREG, MMXR_SLOT_SIZE * 7, MMXR_SIZE, 8)
};

#define SIMD_SLOT_SIZE	32//LATER

#define XMMR_SLOT_SIZE	SIMD_SLOT_SIZE
#define XMMR_SIZE	16	//128 bits

#define YMMR_SLOT_SIZE	SIMD_SLOT_SIZE
#define YMMR_SIZE	32	//256 bits

//#define ZMMR_SLOT_SIZE	64	//512 bits
//#define ZMMR_SIZE	64

enum {
	R_XMM0 = REGID(SSID_SIMD, XMMR_SLOT_SIZE*0, XMMR_SIZE,	1),
	R_XMM1 = REGID(SSID_SIMD, XMMR_SLOT_SIZE*1, XMMR_SIZE,	2),
	R_XMM2 = REGID(SSID_SIMD, XMMR_SLOT_SIZE*2, XMMR_SIZE,	3),
	R_XMM3 = REGID(SSID_SIMD, XMMR_SLOT_SIZE*3, XMMR_SIZE,	4),
	R_XMM4 = REGID(SSID_SIMD, XMMR_SLOT_SIZE*4, XMMR_SIZE,	5),
	R_XMM5 = REGID(SSID_SIMD, XMMR_SLOT_SIZE*5, XMMR_SIZE,	6),
	R_XMM6 = REGID(SSID_SIMD, XMMR_SLOT_SIZE*6, XMMR_SIZE,	7),
	R_XMM7 = REGID(SSID_SIMD, XMMR_SLOT_SIZE*7, XMMR_SIZE,	8),
	R_XMM8 = REGID(SSID_SIMD, XMMR_SLOT_SIZE*8, XMMR_SIZE,	9),
	R_XMM9 = REGID(SSID_SIMD, XMMR_SLOT_SIZE*9, XMMR_SIZE,	10),
	R_XMM10 = REGID(SSID_SIMD, XMMR_SLOT_SIZE*10, XMMR_SIZE,	11),
	R_XMM11 = REGID(SSID_SIMD, XMMR_SLOT_SIZE*11, XMMR_SIZE,	12),
	R_XMM12 = REGID(SSID_SIMD, XMMR_SLOT_SIZE*12, XMMR_SIZE,	13),
	R_XMM13 = REGID(SSID_SIMD, XMMR_SLOT_SIZE*13, XMMR_SIZE,	14),
	R_XMM14 = REGID(SSID_SIMD, XMMR_SLOT_SIZE*14, XMMR_SIZE,	15),
	R_XMM15 = REGID(SSID_SIMD, XMMR_SLOT_SIZE*15, XMMR_SIZE,	16),
};

enum {
	R_YMM0 = REGID(SSID_SIMD, YMMR_SLOT_SIZE * 0, YMMR_SIZE, 1),
	R_YMM1 = REGID(SSID_SIMD, YMMR_SLOT_SIZE * 1, YMMR_SIZE, 2),
	R_YMM2 = REGID(SSID_SIMD, YMMR_SLOT_SIZE * 2, YMMR_SIZE, 3),
	R_YMM3 = REGID(SSID_SIMD, YMMR_SLOT_SIZE * 3, YMMR_SIZE, 4),
	R_YMM4 = REGID(SSID_SIMD, YMMR_SLOT_SIZE * 4, YMMR_SIZE, 5),
	R_YMM5 = REGID(SSID_SIMD, YMMR_SLOT_SIZE * 5, YMMR_SIZE, 6),
	R_YMM6 = REGID(SSID_SIMD, YMMR_SLOT_SIZE * 6, YMMR_SIZE, 7),
	R_YMM7 = REGID(SSID_SIMD, YMMR_SLOT_SIZE * 7, YMMR_SIZE, 8),
	R_YMM8 = REGID(SSID_SIMD, YMMR_SLOT_SIZE * 8, YMMR_SIZE, 9),
	// WARNING! BYTE OVERSIZED!
	R_YMM9 = REGID(SSID_SIMD, YMMR_SLOT_SIZE * 9, YMMR_SIZE, 10),
	R_YMM10 = REGID(SSID_SIMD, YMMR_SLOT_SIZE * 10, YMMR_SIZE, 11),
	R_YMM11 = REGID(SSID_SIMD, YMMR_SLOT_SIZE * 11, YMMR_SIZE, 12),
	R_YMM12 = REGID(SSID_SIMD, YMMR_SLOT_SIZE * 12, YMMR_SIZE, 13),
	R_YMM13 = REGID(SSID_SIMD, YMMR_SLOT_SIZE * 13, YMMR_SIZE, 14),
	R_YMM14 = REGID(SSID_SIMD, YMMR_SLOT_SIZE * 14, YMMR_SIZE, 15),
	R_YMM15 = REGID(SSID_SIMD, YMMR_SLOT_SIZE * 15, YMMR_SIZE, 16),//LATER!
};

// control registers
#define CR_SLOT_SIZE	4	//32bit
#define CR_SIZE	4
enum {
	R_CR0 = REGID(SSID_CTRLREG, CR_SLOT_SIZE * 0, CR_SIZE, 1),
	R_CR1 = REGID(SSID_CTRLREG, CR_SLOT_SIZE * 1, CR_SIZE, 2),
	R_CR2 = REGID(SSID_CTRLREG, CR_SLOT_SIZE * 2, CR_SIZE, 3),
	R_CR3 = REGID(SSID_CTRLREG, CR_SLOT_SIZE * 3, CR_SIZE, 4),
	R_CR4 = REGID(SSID_CTRLREG, CR_SLOT_SIZE * 4, CR_SIZE, 5),
	R_CR5 = REGID(SSID_CTRLREG, CR_SLOT_SIZE * 5, CR_SIZE, 6),
	R_CR6 = REGID(SSID_CTRLREG, CR_SLOT_SIZE * 6, CR_SIZE, 7),
	R_CR7 = REGID(SSID_CTRLREG, CR_SLOT_SIZE * 7, CR_SIZE, 8),

	R_CR8 = REGID(SSID_CTRLREG, CR_SLOT_SIZE * 8, CR_SIZE, 9),
	R_CR9 = REGID(SSID_CTRLREG, CR_SLOT_SIZE * 9, CR_SIZE, 10),
	R_CR10 = REGID(SSID_CTRLREG, CR_SLOT_SIZE * 10, CR_SIZE, 11),
	R_CR11 = REGID(SSID_CTRLREG, CR_SLOT_SIZE * 11, CR_SIZE, 12),
	R_CR12 = REGID(SSID_CTRLREG, CR_SLOT_SIZE * 12, CR_SIZE, 13),
	R_CR13 = REGID(SSID_CTRLREG, CR_SLOT_SIZE * 13, CR_SIZE, 14),
	R_CR14 = REGID(SSID_CTRLREG, CR_SLOT_SIZE * 14, CR_SIZE, 15),
	R_CR15 = REGID(SSID_CTRLREG, CR_SLOT_SIZE * 15, CR_SIZE, 16)
};

// debug registers
#define DR_SLOT_SIZE	4	//32bit
#define DR_SIZE	4
enum {
	R_DR0 = REGID(SSID_DBGREG, DR_SLOT_SIZE * 0, DR_SIZE, 1),
	R_DR1 = REGID(SSID_DBGREG, DR_SLOT_SIZE * 1, DR_SIZE, 2),
	R_DR2 = REGID(SSID_DBGREG, DR_SLOT_SIZE * 2, DR_SIZE, 3),
	R_DR3 = REGID(SSID_DBGREG, DR_SLOT_SIZE * 3, DR_SIZE, 4),
	R_DR4 = REGID(SSID_DBGREG, DR_SLOT_SIZE * 4, DR_SIZE, 5),
	R_DR5 = REGID(SSID_DBGREG, DR_SLOT_SIZE * 5, DR_SIZE, 6),
	R_DR6 = REGID(SSID_DBGREG, DR_SLOT_SIZE * 6, DR_SIZE, 7),
	R_DR7 = REGID(SSID_DBGREG, DR_SLOT_SIZE * 7, DR_SIZE, 8)
};

// test registers
#define TR_SLOT_SIZE	4	//32bit
#define TR_SIZE	4

enum {
	//R_TR0 = REGID(SSID_TESTREG, TR_SLOT_SIZE * 0, TR_SIZE, 1),
	//R_TR1 = REGID(SSID_TESTREG, TR_SLOT_SIZE * 1, TR_SIZE, 2),
	//R_TR2 = REGID(SSID_TESTREG, TR_SLOT_SIZE * 2, TR_SIZE, 3),
	R_TR3 = REGID(SSID_TESTREG, TR_SLOT_SIZE * 3, TR_SIZE, 4),
	R_TR4 = REGID(SSID_TESTREG, TR_SLOT_SIZE * 4, TR_SIZE, 5),
	R_TR5 = REGID(SSID_TESTREG, TR_SLOT_SIZE * 5, TR_SIZE, 6),
	R_TR6 = REGID(SSID_TESTREG, TR_SLOT_SIZE * 6, TR_SIZE, 7),
	R_TR7 = REGID(SSID_TESTREG, TR_SLOT_SIZE * 7, TR_SIZE, 8)
};



//EFLAGS////////////////////////////////////////

enum {//offsets
	CSW_CF	= 0,
	CSW_PF	= 2,
	CSW_AF	= 4,
	CSW_ZF	= 6,
	CSW_SF	= 7,
	CSW_TF	= 8,
	CSW_IF	= 9,
	CSW_DF	= 10,
	CSW_OF	= 11,
	CSW_IOPL= 12,
	CSW_NT	= 14
};

//masks
enum CPUFlags_t {
	F_CF = 0x0001,//(1 << 0)//carry flag
	F_PF = 0x0004,//(1 << 2)//parity flag
	F_AF = 0x0010,//(1 << 4)//auxiliary carry flag
	F_ZF = 0x0040,//(1 << 6)//zero flag
	F_SF = 0x0080,//(1 << 7)//sign flag
	F_IF = 0x0200,//(1 << 9)//interrupt flag
	F_DF = 0x0400,//(1 << 10)//direction flag
	F_OF = 0x0800,//(1 << 11)//overflow flag
	};

#define CPUSW_MASK		(F_CF|F_PF|F_AF|F_ZF|F_SF|F_IF|F_DF|F_OF)

//ids
enum CSWID_t {
	CSWID_CF = 1, CSWID_PF, CSWID_AF, CSWID_ZF, CSWID_SF, CSWID_IF, CSWID_DF, CSWID_OF };

//encodins
enum {
	R_CF   = REGID(SSID_CPUSW, 0, 1, 1),
	R_PF   = REGID(SSID_CPUSW, 2, 1, 2),
	R_AF   = REGID(SSID_CPUSW, 4, 1, 3),
	R_ZF   = REGID(SSID_CPUSW, 6, 1, 4),
	R_SF   = REGID(SSID_CPUSW, 7, 1, 5),
	R_TF   = REGID(SSID_CPUSW, 8, 1, 6),
	R_IF   = REGID(SSID_CPUSW, 9, 1, 7),
	R_DF   = REGID(SSID_CPUSW, 10, 1, 8),
	R_OF   = REGID(SSID_CPUSW, 11, 1, 9),
	R_IOPL = REGID(SSID_CPUSW, 12, 2, 10),
	R_NT   = REGID(SSID_CPUSW, 14, 1, 11),
#if(X64_SUPPORT)
	R_RF   = REGID(SSID_CPUSW, 16, 1, 12),
	R_VM   = REGID(SSID_CPUSW, 17, 1, 13),
	R_AC   = REGID(SSID_CPUSW, 18, 1, 14),
	R_VIF  = REGID(SSID_CPUSW, 19, 1, 15),
	R_VIP  = REGID(SSID_CPUSW, 20, 1, 16),
	R_ID   = REGID(SSID_CPUSW, 21, 1, 17)
#endif
};




//FPU status word //////////////////////////////////

//masks
enum FPUSW_t {
	FPUSW_IE	= 0x0001,	//	INVALID OPERATION
	FPUSW_DE	= 0x0002,	//	DENORMALIZED OPERAND
	FPUSW_ZE	= 0x0004,	//	ZERO DIVIDE
	FPUSW_OE	= 0x0008,	//	OVERFLOW
	FPUSW_UE	= 0x0010,	//	UNDERFLOW
	FPUSW_PE	= 0x0020,	//	PRECISION
	FPUSW_SF	= 0x0040,	//STACK FAULT
	FPUSW_ES	= 0x0080,	//ERROR SUMMARY STATUS
	FPUSW_C0	= 0x0100,	//		CONDITION CODE(8)
	FPUSW_C1	= 0x0200,	//		CONDITION CODE(9)
	FPUSW_C2	= 0x0400,	//		CONDITION CODE(10)
	FPUSW_TOP	= 0x3800,	//TOP OF STACK POINTER
	FPUSW_C3	= 0x4000,	//		CONDITION CODE(14)
	FPUSW_B		= 0x8000,	//FPU BUSY
	};

#define FPUSW_MASK		(FPUSW_C0|FPUSW_C1|FPUSW_C2|FPUSW_C3)

// encodings
enum {
	R_FPUSW_IE	= REGID(SSID_FPUSW, 0, 1, 1),	//	INVALID OPERATION
	R_FPUSW_DE	= REGID(SSID_FPUSW, 1, 1, 2),	//	DENORMALIZED OPERAND
	R_FPUSW_ZE	= REGID(SSID_FPUSW, 2, 1, 3),	//	ZERO DIVIDE
	R_FPUSW_OE	= REGID(SSID_FPUSW, 3, 1, 4),	//	OVERFLOW
	R_FPUSW_UE	= REGID(SSID_FPUSW, 4, 1, 5),	//	UNDERFLOW
	R_FPUSW_PE	= REGID(SSID_FPUSW, 5, 1, 6),	//	PRECISION
	R_FPUSW_SF	= REGID(SSID_FPUSW, 6, 1, 7),	//STACK FAULT
	R_FPUSW_ES	= REGID(SSID_FPUSW, 7, 1, 8),	//ERROR SUMMARY STATUS
	R_FPUSW_C0	= REGID(SSID_FPUSW, 8, 1, 9),	//		CONDITION CODE(8)
	R_FPUSW_C1	= REGID(SSID_FPUSW, 9, 1, 10),	//		CONDITION CODE(9)
	R_FPUSW_C2	= REGID(SSID_FPUSW, 10, 1, 11),	//		CONDITION CODE(10)
	R_FPUSW_TOP	= REGID(SSID_FPUSW, 11, 3, 12),	//TOP OF STACK POINTER
	R_FPUSW_C3	= REGID(SSID_FPUSW, 14, 1, 13),	//		CONDITION CODE(14)
	R_FPUSW_B	= REGID(SSID_FPUSW, 15, 1, 14),	//FPU BUSY
};

//FPU control word

//masks
enum FPUCW_t {
	FPUCW_IM	= 0x0001,	//	INVALID OPERATION
	FPUCW_DM	= 0x0002,	//	DENORMALIZED OPERAND
	FPUCW_ZM	= 0x0004,	//	ZERO DIVIDE
	FPUCW_OM	= 0x0008,	//	OVERFLOW
	FPUCW_UM	= 0x0010,	//	UNDERFLOW
	FPUCW_PM	= 0x0020,	//	PRECISION
		FPUCW_0x00C0,
	FPUCW_PC	= 0x0300,	//PRECISION CONTROL
	FPUCW_RC	= 0x0C00,	//ROUNDING CONTROL
		FPUCW_0xF0C0,
};

enum {
	R_FPUCW_IM	= REGID(SSID_FPUCW, 0, 1, 1),	//	INVALID OPERATION
	R_FPUCW_DM	= REGID(SSID_FPUCW, 1, 1, 2),	//	DENORMALIZED OPERAND
	R_FPUCW_ZM	= REGID(SSID_FPUCW, 2, 1, 3),	//	ZERO DIVIDE
	R_FPUCW_OM	= REGID(SSID_FPUCW, 3, 1, 4),	//	OVERFLOW
	R_FPUCW_UM	= REGID(SSID_FPUCW, 4, 1, 5),	//	UNDERFLOW
	R_FPUCW_PM	= REGID(SSID_FPUCW, 5, 1, 6),	//	PRECISION
	R_FPUCW_PC	= REGID(SSID_FPUCW, 8, 2, 7),	//PRECISION CONTROL
	R_FPUCW_RC	= REGID(SSID_FPUCW, 10, 2, 8),	//ROUNDING CONTROL
};


// AVX512 opmask registers
#define KR_SLOT_SIZE	4	//16bit
#define KR_SIZE	2
enum {
	R_K0 = REGID(SSID_KREG, KR_SLOT_SIZE * 0, KR_SIZE, 1),
	R_K1 = REGID(SSID_KREG, KR_SLOT_SIZE * 1, KR_SIZE, 2),
	R_K2 = REGID(SSID_KREG, KR_SLOT_SIZE * 2, KR_SIZE, 3),
	R_K3 = REGID(SSID_KREG, KR_SLOT_SIZE * 3, KR_SIZE, 4),
	R_K4 = REGID(SSID_KREG, KR_SLOT_SIZE * 4, KR_SIZE, 5),
	R_K5 = REGID(SSID_KREG, KR_SLOT_SIZE * 5, KR_SIZE, 6),
	R_K6 = REGID(SSID_KREG, KR_SLOT_SIZE * 6, KR_SIZE, 7),
	R_K7 = REGID(SSID_KREG, KR_SLOT_SIZE * 7, KR_SIZE, 8)
};


class IAFrontDC_t : public I_FrontDC
{
protected:
	FE_t	m;
public:
	IAFrontDC_t();
	virtual const FE_t * GetFE() const override { return &m; }
	virtual const RegInfo_t *RegInfoTable() const override;
	virtual const char *RegName(SSID_t opc, int sz, int ofs, int flags, char buf[80]) const override;
private:
	static const char* FPUREG_reg2str(int sz, int ofs, int flags, char buf[80]);
};


class IA32FrontDC_t : public IAFrontDC_t
{
public:
	IA32FrontDC_t();
	virtual const char *Name() const  override;
	virtual const char **Symbols() const  override;

	virtual SSID_t ArgSSID(unsigned index, unsigned fflags) const override;
	virtual SSID_t RetSSID(unsigned index, OpType_t retType) const override;
	virtual unsigned RetOff(unsigned index, OpType_t retType) const override;
	virtual unsigned ArgOff(unsigned index, unsigned fflags) const override;
	virtual unsigned ArgSize(unsigned index, unsigned u) const override;
};

class IA64FrontDC_t : public IAFrontDC_t
{
public:
	IA64FrontDC_t();
	virtual const char *Name() const;
	virtual const char **Symbols() const;

	virtual SSID_t ArgSSID(unsigned index, unsigned fflags) const override;
	virtual SSID_t RetSSID(unsigned index, OpType_t retType) const override;
	virtual unsigned RetOff(unsigned index, OpType_t retType) const override;
	virtual unsigned ArgOff(unsigned index, unsigned fflags) const override;
	virtual unsigned ArgSize(unsigned index, unsigned u) const override;
};

const char *ToRegName(OPC_t opc, unsigned ofs, unsigned sz);




