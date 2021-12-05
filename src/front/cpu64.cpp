#include <iostream>
#include <assert.h>
//#include <conio.h>

#include "shared/defs.h"
#include "shared/misc.h"
#include "shared/front.h"
#include "shared/action.h"
#include "x86_IR.h"
#include "x86.h"


#pragma warning(disable:4838)//conversion from 'type_1' to 'type_2' requires a narrowing conversion

namespace x86_64 {

	#define PTRSIZE 8	//IA64

	templ_t* g_list = nullptr;

	struct templ2_t : public templ_t
	{
		templ2_t(const char* n, const itf_entry_t* f, const ito_t* o, int p = 0)//int (*f)[INS_OPS_MAX]
			: templ_t(n, f, o, p, g_list)
		{
			g_list = this;

			templ_t** prev = &g_list;
			while (next)
			{
				int result = strcmp(name, next->name);
				if (result < 0)
					break;//add in tail of peers
				if (result == 0)
					break;//add in head of peers 

				*prev = next;
				next = (*prev)->next;
				(*prev)->next = this;
				prev = &((*prev)->next);
			}
		}

		/*void cout_list()
		{
			templ_t *p = g_list;
			while (p)
			{
				std::cout << p->name << std::endl;
				p = p->next;
			}

			getch();
		}*/
	};



// A D D	
IT_FRM(ADD)//"Add"
//	{_INT|R8_AL,	_INT|OP_I8},	//04 ib
//	{_INT|R16_AX,	_INT|OP_I16},	//05 iw
//	{_INT|R32_EAX,	_INT|OP_I32},	//05 id
//	{_INT|R64_RAX,	_INT|OP_I32},	//REX.W + 05 id

	{_INT|OP_R8,	_INT|OP_I8},	{_INT|OP_M8,	_INT|OP_I8},	//80 /0 ib
	{_INT|OP_R16,	_INT|OP_I16},	{_INT|OP_M16,	_INT|OP_I16},	//81 /0 iw
	{_INT|OP_R32,	_INT|OP_I32},	{_INT|OP_M32,	_INT|OP_I32},	//81 /0 id
	{_INT|OP_R64,	_INT|OP_I32},	{_INT|OP_M64,	_INT|OP_I32},	//REX.W + 81 /0 id
	{_INT|OP_R64,	_INT|OP_I64},	{_INT|OP_M64,	_INT|OP_I64},	//n\a

	{_INT|OP_R16,	S_|OP_I8},	{_INT|OP_M16,	S_|OP_I8},	//83 /0 ib
	{_INT|OP_R32,	S_|OP_I8},	{_INT|OP_M32,	S_|OP_I8},	//83 /0 ib
	{_INT|OP_R64,	S_|OP_I8},	{_INT|OP_M64,	S_|OP_I8},	//REX.W + 83 /0 ib

	{_INT|OP_R8,	_INT|OP_R8},	{_INT|OP_M8,	_INT|OP_R8},	//00 /r		|REX + 00 /r
	{_INT|OP_R16,	_INT|OP_R16},	{_INT|OP_M16,	_INT|OP_R16},	//01 /r
	{_INT|OP_R32,	_INT|OP_R32},	{_INT|OP_M32,	_INT|OP_R32},	//01 /r
	{_INT|OP_R64,	_INT|OP_R64},	{_INT|OP_M64,	_INT|OP_R64},	//REX.W + 01 /r

	{_INT|OP_R8,	_INT|OP_M8},	//02 /r		|REX + 02 /r
	{_INT|OP_R16,	_INT|OP_M16},	//03 /r
	{_INT|OP_R32,	_INT|OP_M32},	//03 /r
	{_INT|OP_R64,	_INT|OP_M64},	//REX.W + 03 /r

IT_BEG(ADD)
//op1=op1+op2
	ACTN4(ACTN_MOV, M_(F_CF|F_PF|F_AF|F_ZF|F_SF|F_OF), 0)
		OPND1(OPND_1)
		ACTN1(ACTN_ADD)
			OPND1(OPND_1)
			OPND1(OPND_2)
IT_END(ADD)


// A N D	
IT_FRM(AND)//"Logical AND",
//	{R8_AL,		U_|OP_I8},		//24 ib
//	{R16_AX,	U_|OP_I16},		//25 iw
//	{R32_EAX,	U_|OP_I32},		//25 id
//	{R32_RAX,	U_|OP_I32},		//REX.W + 25 id

	{OP_R8,		U_|OP_I8},		{OP_M8,		U_|OP_I8},	//80 /4 ib	| REX + 80 /4 ib
	{OP_R16,	U_|OP_I16},		{OP_M16,	U_|OP_I16},	//81 /4 iw
	{OP_R32,	U_|OP_I32},		{OP_M32,	U_|OP_I32},	//81 /4 id
	{OP_R64,	U_|OP_I32},		{OP_M64,	U_|OP_I32},	//REX.W + 81 /4 id
	{OP_R64,	U_|OP_I64},		{OP_M64,	U_|OP_I64},	// n\a

	{OP_R16,	S_|OP_I8},		{OP_M16,	S_|OP_I8},	//83 /4 ib
	{OP_R32,	S_|OP_I8},		{OP_M32,	S_|OP_I8},	//83 /4 ib
	{OP_R64,	S_|OP_I8},		{OP_M64,	S_|OP_I8},	//REX.W + 83 /4 ib

	{OP_R8,		OP_R8},		{OP_M8,		OP_R8},		//20 /r
	{OP_R16,	OP_R16},	{OP_M16,	OP_R16},	//21 /r
	{OP_R32,	OP_R32},	{OP_M32,	OP_R32},	//21 /r
	{OP_R64,	OP_R64},	{OP_M64,	OP_R64},	//REX.W + 21 /r

	{OP_R8,		OP_M8},		//22 /r
	{OP_R16,	OP_M16},	//23 /r
	{OP_R32,	OP_M32},	//23 /r
	{OP_R64,	OP_M64},	//REX.W + 23 /r
IT_BEG(AND)
#if(FULLEXPAND)
//(op1,csw(cf,pf,sf))=op1&op2, csw(cf,of)= 0
	ACTN1(ACTN_COMMA)
		ACTN4(ACTN_MOV, M_(F_PF|F_ZF|F_SF), 0)
			OPND1(OPND_1)
			ACTN1(ACTN_AND)
				OPND1(OPND_1)
				OPND1(OPND_2)
		ACTN4(ACTN_MOV, M_(F_CF|F_OF), 0)
			OPND3(FOPC_CPUSW, OPSZ_WORD, 0)
			OPND3(OPC_IMM, OPSZ_WORD, 0)
#else
//(op1,csw(cf,pf,sf,of,cf))=op1&op2
	ACTN4(ACTN_MOV, M_(F_PF|F_ZF|F_SF|F_CF|F_OF), 0)
		OPND1(OPND_1)
		ACTN1(ACTN_AND)
			OPND1(OPND_1)
			OPND1(OPND_2)
#endif
IT_END(AND)


// C A L L
IT_FRM(CALL)//"Call Procedure",
	{OP_REL16},			//E8 cw
	{OP_REL32},			//E8 cd
	{OP_R16},	{OP_M16},	//FF /2
	{OP_R32},	{OP_M32},	//FF /2
	{OP_R64},	{OP_M64},	//FF /2
//	{OP_PTR1632},	//?
//	{OP_M1632},		//?
//	{OP_M1664},		//REX.W + FF /3
IT_BEG(CALL)
#if(0)
	ACTN4(ACTN_CALL, 0, __SP(-1))
		OPND1(OPND_1)
		OPND0//ACTN1(ACTN_NULL)
#else
//[--esp]:4=$NVA, call(op1)
	ACTN1(ACTN_COMMA)
		ACTN4(ACTN_MOV, 0, __SP(-1))
			OPND3(OPC_SS|OPC_CPUREG, OPSZ_QWORD, OFS(R_ESP))
			OPND_NVA		//next address
		ACTN1(ACTN_CALL)
			OPND1(OPND_1)
			OPND0
#endif
IT_END(CALL)


//////////////////////
#include "jumps32.inl"

// J M P
IT_FRM(JMP)//"Jump Unconditional"
	{OP_REL8},		//EB cb
	{OP_REL16},		//E9 cw
	{OP_REL32},		//E9 cd
//	{OP_R16},		{OP_M16},	//FF /4
	{OP_R32},		{OP_M32},	//FF /4
	{OP_R64},		{OP_M64},	//FF /4
//	{OP_PTR1616},	//EA cd
	{OP_PTR1632},	//EA cp
//	{OP_M1616},		//FF /5
	{OP_M1632},		//FF /5
	{OP_M1664},		//REX.W + FF /5
IT_BEG(JMP)
//goto op1
	ACTN1(ACTN_GOTO)
		OPND1(OPND_1)
		OPND0
IT_END(JMP)

//////////////////////

// L E A
IT_FRM(LEA)//"Load Effective Address"
	{OP_R16,	MAKEOPC(OPC_INDIRECT)},	//8D /r
	{OP_R32,	MAKEOPC(OPC_INDIRECT)},	//8D /r
	{OP_R64,	MAKEOPC(OPC_INDIRECT)},	//REX.W + 8D /r
IT_BEG(LEA)
//op1 = &op2
	ACTN1(ACTN_MOV)
		OPND1(OPND_1)
		ACTN1(ACTN_OFFS)
			OPND1(OPND_2)
IT_END(LEA)



// M O V
IT_FRM(MOV)//"Move Data"
	{OP_R8,		OP_R8},		{OP_M8,		OP_R8},		//88 /r
	{OP_R16,	OP_R16},	{OP_M16,	OP_R16},	//89 /r
	{OP_R32,	OP_R32},	{OP_M32,	OP_R32},	//89 /r
	{OP_R64,	OP_R64},	{OP_M64,	OP_R64},	//REX.W + 89 /r
				
	{OP_R8,		OP_M8},		//8A /r
	{OP_R16,	OP_M16},	//8B /r
	{OP_R32,	OP_M32},	//8B /r
	{OP_R64,	OP_M64},	//REX.W + 8B /r

	{OP_R16,	OP_SREG},	{OP_M16,	OP_SREG},	//8C /r
	{OP_SREG,	OP_R16},	{OP_SREG,	OP_M16},	//8E /r

//	{R8_AL,		OP_MOFFS8},		//A0
 //	{R16_EX,	OP_MOFFS16},	//A1
//	{R32_EAX,	OP_MOFFS32},	//A1
//	{R64_RAX,	OP_MOFFS64},	//REX.W + A1

//	{OP_MOFFS8,	R8_AL},			//A2
//	{OP_MOFFS16,R32_EX},		//A3
//	{OP_MOFFS32,R32_EAX},		//A3
//	{OP_MOFFS64,R32_RAX},		//REX.W + A3		

	{OP_R8,		OP_I8},			//B0+ rb
	{OP_R16,	OP_I16},		//B8+ rw
	{OP_R32,	OP_I32},		//B8+ rd
	{OP_R64,	OP_I64},		//REX.W + B8+ rd io

	{OP_M8,		OP_I8},			//C6 /0
	{OP_M16,	OP_I16},		//C7 /0
	{OP_M32,	OP_I32},		//C7 /0
	{OP_M64,	S_|OP_I32},		//REX.W + C7 /0 id
	{OP_M64,	S_|OP_I64},		//n\a
	
//	{OP_CR,		OP_R32},		
//	{OP_R32,	OP_CR},
//	{OP_DR,		OP_R32},
//	{OP_R32,	OP_DR},
IT_BEG(MOV)
//op1 = op2
	ACTN1(ACTN_MOV)
		OPND1(OPND_1)
		OPND1(OPND_2)
IT_END(MOV)

// M O V (2)
IT_FRM2(MOV,2)//"Move Data"
	{OP_R64,	OP_SREG},	{OP_M64,	OP_SREG},	//REX.W + 8C /r
IT_BEG2(MOV,2)
//op1 = ZeroExtend(op2)
	ACTN1(ACTN_MOV)
		OPND1(OPND_1)
		ACTN1(ACTN_ZEROEXT)
			OPND1(OPND_2)
IT_END2(MOV,2)

// M O V (3)
IT_FRM2(MOV,3)//"Move Data"
	{OP_SREG,	OP_R64},	{OP_SREG,	OP_M64},	//REX.W + 8E /r
IT_BEG2(MOV,3)
//op1 = LoHalf(LoHalf(op2))
	ACTN1(ACTN_MOV)
		OPND1(OPND_1)
		ACTN1(ACTN_LOHALF)
			ACTN1(ACTN_LOHALF)
				OPND1(OPND_2)
IT_END2(MOV,3)

// M O V (4)
/*IT_FRM2(MOV,4)//"Move Data"
	{OP_M64,	OP_I32},		//REX.W + C7 /0 id
	{OP_M64,	OP_I64},		//n\a          - sign extneded???
IT_BEG2(MOV,4)
//op1 = SignExtend(op2)
	ACTN1(ACTN_MOV)
		OPND1(OPND_1)
		ACTN1(ACTN_SIGNEXT)
			OPND1(OPND_2)
IT_END2(MOV,4)*/

//..

// M O V S X
IT_FRM(MOVSX)//"Move with Sign-Extend"
	{U_|OP_R16,	U_|OP_R8},		{U_|OP_R16,	U_|OP_M8},		//0F BE /r
	{U_|OP_R32,	U_|OP_R8},		{U_|OP_R32,	U_|OP_M8},		//0F BE /r
	{U_|OP_R64,	U_|OP_R8},		{U_|OP_R64,	U_|OP_M8},		//REX + 0F BE /r
	{U_|OP_R32,	U_|OP_R16},		{U_|OP_R32,	U_|OP_M16},		//0F BF /r
	{U_|OP_R64,	U_|OP_R16},		{U_|OP_R64,	U_|OP_M16},		//REX.W + 0F BF /r
IT_BEG(MOVSX)
//op1 = SignExtend(op2)
	ACTN1(ACTN_MOV)
		OPND1(OPND_1)
		ACTN1(ACTN_SIGNEXT)
			OPND1(OPND_2)
IT_END(MOVSX)

// M O V S X D
IT_FRM(MOVSXD)//Move doubleword to quadword with signextension"
	{U_|OP_R64,	U_|OP_R32},		{U_|OP_R64,	U_|OP_M32},		//REX.W** + 63 /r
IT_BEG(MOVSXD)
//op1 = SignExtend(op2)
	ACTN1(ACTN_MOV)
		OPND1(OPND_1)
		ACTN1(ACTN_SIGNEXT)
			OPND1(OPND_2)
IT_END(MOVSXD)

// M O V Z X
IT_FRM(MOVZX)//"Move with Zero-Extend"
	{U_|OP_R16,	U_|OP_R8},		{U_|OP_R16,	U_|OP_M8},		//0F B6 /r
	{U_|OP_R32,	U_|OP_R8},		{U_|OP_R32,	U_|OP_M8},		//0F B6 /r
	{U_|OP_R64,	U_|OP_R8},		{U_|OP_R64,	U_|OP_M8},		//REX.W + 0F B6 /r
	{U_|OP_R32,	U_|OP_R16},		{U_|OP_R32,	U_|OP_M16},		//0F B7 /r
	{U_|OP_R64,	U_|OP_R16},		{U_|OP_R64,	U_|OP_M16},		//REX.W + 0F B7 /r
IT_BEG(MOVZX)
//op1 = ZeroExtend(op2)
	ACTN1(ACTN_MOV)
		OPND1(OPND_1)
		ACTN1(ACTN_ZEROEXT)
			OPND1(OPND_2)
IT_END(MOVZX)


// N O P
IT_FRM(NOP)//No Operation	// 90
IT_BEG(NOP)
	OPND0//ACTN1(ACTN_NULL)
IT_END(NOP)


// N O P (2)
IT_FRM2(NOP,2)	//No Operation (multi-byte)
	{OP_R16},		{OP_M16},
	{OP_R32},		{OP_M32},
IT_BEG2(NOP,2)
	OPND0//ACTN1(ACTN_NULL)
IT_END2(NOP,2)


// P O P (1)		
IT_FRM(POP)//"Pop Operand from the Stack",
	{OP_M16},	{OP_M32},	{OP_M64},	//8F /0
	{OP_R16},	{OP_R32},	{OP_R64},	//58+ r(b/w/d)
IT_BEG(POP)
#if(_SPLIT)
//op1=[esp++]:4
	ACTN1(ACTN_COMMA)
		ACTN1(ACTN_MOV)
			OPND1(OPND_1)
			OPND3(OPC_SS|OPC_CPUREG, OPSZ_DWORD, OFS(R_ESP))
		ACTN1(ACTN_MOV)
			OPND3(OPC_CPUREG, OPSZ_DWORD, OFS(R_ESP))
			ACTN1(ACTN_ADD)
				OPND3(OPC_CPUREG, OPSZ_DWORD, OFS(R_ESP))
				OPND3(OPC_IMM, OPSZ_DWORD, 4)//stack_addr_size
#else
//op1=[esp++]:4
	ACTN4(ACTN_MOV, 0, __SP(1))//?!!
		OPND1(OPND_1)
		OPND3(OPC_SS | OPC_CPUREG, SIZEOF(OPND_1), OFS(R_ESP))
#endif//_SPLIT
IT_END(POP)


// P U S H (1)
IT_FRM(PUSH)//"Push Operand onto the Stack"
	{OP_R16},		{OP_M16},	//FF /6
	{OP_R32},		{OP_M32},	//FF /6
	{OP_R64},		{OP_M64},	//FF /6

//?	{OP_I8},		//6A ib	//!CPU pushes 4 bytes anycase!!!
	{OP_I16},		//68 iw
	{OP_I32},		//68 id
IT_BEG(PUSH)
#if(_SPLIT)
//[--esp]:4=op1
	ACTN1(ACTN_COMMA)
		ACTN1(ACTN_MOV)
			OPND3(OPC_CPUREG, OPSZ_DWORD, OFS(R_ESP))
			ACTN1(ACTN_SUB)
				OPND3(OPC_CPUREG, OPSZ_DWORD, OFS(R_ESP))
				OPND3(OPC_IMM, OPSZ_DWORD, PTRSIZE)//stack_addr_size
		ACTN1(ACTN_MOV)
			OPND3(OPC_SS|OPC_CPUREG, OPSZ_DWORD, OFS(R_ESP))
			OPND1(OPND_1)
#else
//[--esp]:4=op1
	ACTN4(ACTN_MOV, 0, __SP(-1))
		OPND3(OPC_SS|OPC_CPUREG, SIZEOF(OPND_1), OFS(R_RSP))
		OPND1(OPND_1)
#endif//_SPLIT
IT_END(PUSH)

//..

// R E T (1)
IT_FRM(RET)	//"Return from Procedure"
IT_BEG(RET)
// @goto [rsp+=4]
	ACTN4(ACTN_RET, 0, __SP(1))
		OPND3(OPC_SS | OPC_CPUREG, SIZ(R_RSP), OFS(R_RSP))
		OPND0
IT_END(RET)

// S U B
IT_FRM(SUB)//"Integer Subtraction"
//	{_INT|R8_AL,	_INT|OP_I8},	//2C ib
//	{_INT|R16_EAX,	_INT|OP_I16},	//2D iw
//	{_INT|R32_EAX,	_INT|OP_I32},	//2D id
//	{_INT|R32_RAX,	_INT|OP_I32},	//REX.W + 2D id

	{_INT|OP_R8,	_INT|OP_I8},	{_INT|OP_M8,	_INT|OP_I8},	//80 /5 ib
	{_INT|OP_R16,	_INT|OP_I16},	{_INT|OP_M16,	_INT|OP_I16},	//81 /5 iw
	{_INT|OP_R32,	_INT|OP_I32},	{_INT|OP_M32,	_INT|OP_I32},	//81 /5 id
	{_INT|OP_R64,	_INT|OP_I32},	{_INT|OP_M64,	_INT|OP_I32},	//REX.W + 81 /5 id
	{_INT|OP_R64,	_INT|OP_I64},	{_INT|OP_M64,	_INT|OP_I64},	//n/a

	{_INT|OP_R16,	S_|OP_I8},		{_INT|OP_M16,	S_|OP_I8},		//83 /5 ib
	{_INT|OP_R32,	S_|OP_I8},		{_INT|OP_M32,	S_|OP_I8},		//83 /5 ib
	{_INT|OP_R64,	S_|OP_I8},		{_INT|OP_M64,	S_|OP_I8},		//REX.W + 83 /5 ib

	{_INT|OP_R8,	_INT|OP_R8},	{_INT|OP_M8,	_INT|OP_R8},	//28 /r		|REX + 28 /r
	{_INT|OP_R16,	_INT|OP_R16},	{_INT|OP_M16,	_INT|OP_R16},	//29 /r
	{_INT|OP_R32,	_INT|OP_R32},	{_INT|OP_M32,	_INT|OP_R32},	//29 /r
	{_INT|OP_R64,	_INT|OP_R64},	{_INT|OP_M64,	_INT|OP_R64},	//REX.W + 29 /r

	{_INT|OP_R8,	_INT|OP_M8},	//2A /r		|REX + 2A /r
	{_INT|OP_R16,	_INT|OP_M16},	//2B /r
	{_INT|OP_R32,	_INT|OP_M32},	//2B /r
	{_INT|OP_R64,	_INT|OP_M64},	//REX.W + 2B /r
IT_BEG(SUB)
//op1=op1-op2
	ACTN4(ACTN_MOV, M_(F_CF|F_PF|F_AF|F_ZF|F_SF|F_OF), 0)
		OPND1(OPND_1)
		ACTN1(ACTN_SUB)
			OPND1(OPND_1)
			OPND1(OPND_2)
IT_END(SUB)

// T E S T
IT_FRM(TEST)//"Logical Compare"
//	{R8_AL,		U_|OP_I8},		//A8 ib
//	{R16_EX,	U_|OP_I16},		//A9 id
//	{R32_EAX,	U_|OP_I32},		//A9 id
//	{R32_RAX,	U_|OP_I32},		//REX.W + A9 id

	{OP_R8,		U_|OP_I8},		{OP_M8,		U_|OP_I8},	//F6 /0 ib		|REX + F6 /0 ib
	{OP_R16,	U_|OP_I16},		{OP_M16,	U_|OP_I16},	//F7 /0 iw
	{OP_R32,	U_|OP_I32},		{OP_M32,	U_|OP_I32},	//F7 /0 id
	{OP_R64,	U_|OP_I32},		{OP_M64,	U_|OP_I32},	//REX.W + F7 /0 id

	{OP_R8,		OP_R8},		{OP_M8,		OP_R8},		//84 /r		|REX + 84 /r
	{OP_R16,	OP_R16},	{OP_M16,	OP_R16},	//85 /r
	{OP_R32,	OP_R32},	{OP_M32,	OP_R32},	//85 /r
	{OP_R64,	OP_R64},	{OP_M64,	OP_R64},	//REX.W + 85 /r
IT_BEG(TEST)
#if(FULLEXPAND)
//csw(cf,pf,sf)~~op1&op2, csw(cf,of)= 0
	ACTN1(ACTN_COMMA)
		ACTN4(ACTN_CHECK, M_(F_PF|F_ZF|F_SF), 0)
			OPND3(FOPC_CPUSW, OPSZ_WORD, 0)
			ACTN1(ACTN_AND)
				OPND1(OPND_1)
				OPND1(OPND_2)
		ACTN4(ACTN_MOV, M_(F_CF|F_OF), 0)
			OPND3(FOPC_CPUSW, OPSZ_WORD, 0)
			OPND3(OPC_IMM, OPSZ_WORD, 0)
#else
//csw(cf,pf,sf,cf,of)~~op1&op2
	ACTN4(ACTN_CHECK, M_(F_PF|F_ZF|F_SF|F_CF|F_OF), 0)
		OPND3(FOPC_CPUSW, OPSZ_WORD, 0)
		ACTN1(ACTN_AND)
			OPND1(OPND_1)
			OPND1(OPND_2)
#endif
IT_END(TEST)

// X O R
IT_FRM(XOR)//"Logical Exclusive OR"
//	{_INT|R8_AL,	_INT|OP_I8},			//34 ib
//	{_INT|R16_AX,	_INT|OP_I16},			//35 iw
//	{_INT|R32_AX,	_INT|OP_I32},			//35 iw
//	{_INT|R64_RAX,	_INT|OP_I32},			//REX.W + 35 id		(sign-extended).

	{_INT|OP_R8,	_INT|OP_I8},	{_INT|OP_M8,	_INT|OP_I8},		//80 /6 ib
//	{_INT|OP_R8,	_INT|OP_I8},	{_INT|OP_M8,	_INT|OP_I8},		//REX + 80 /6 ib
	{_INT|OP_R16,	_INT|OP_I16},	{_INT|OP_M16,	_INT|OP_I16},		//81 /6 iw
	{_INT|OP_R32,	_INT|OP_I32},	{_INT|OP_M32,	_INT|OP_I32},		//81 /6 iw
	
	{_INT|OP_R64,	S_|OP_I32},		{_INT|OP_M64,	S_|OP_I32},		//REX.W + 81 /6 id

	{_INT|OP_R16,	S_|OP_I8},		{_INT|OP_M16,	S_|OP_I8},		//83 /6 ib
	{_INT|OP_R32,	S_|OP_I8},		{_INT|OP_M32,	S_|OP_I8},		//83 /6 ib
	{_INT|OP_R64,	S_|OP_I8},		{_INT|OP_M64,	S_|OP_I8},		//REX.W + 83 /6 ib

	{_INT|OP_R8,	_INT|OP_R8},	{_INT|OP_M8,	_INT|OP_R8},	//30 /r			|REX + 30 /r
	{_INT|OP_R16,	_INT|OP_R16},	{_INT|OP_M16,	_INT|OP_R16},	//31 /r
	{_INT|OP_R32,	_INT|OP_R32},	{_INT|OP_M32,	_INT|OP_R32},	//31 /r
	{_INT|OP_R64,	_INT|OP_R64},	{_INT|OP_M64,	_INT|OP_R64},	//REX.W + 31 /r

	{_INT|OP_R8,	_INT|OP_M8},	//32 /r			|REX + 32 /r
	{_INT|OP_R16,	_INT|OP_M16},	//33 /r
	{_INT|OP_R32,	_INT|OP_M32},	//33 /r
	{_INT|OP_R64,	_INT|OP_M64},	////REX.W + 33 /r

IT_BEG(XOR)
//op1=op2^op3
	ACTN4(ACTN_MOV, M_(F_SF|F_ZF|F_PF), 0)
		OPND1(OPND_1)
		ACTN1(ACTN_XOR)
			OPND1(OPND_1)
			OPND1(OPND_2)
IT_END(XOR)

}//namespace x86_64

