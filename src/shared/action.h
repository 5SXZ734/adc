#ifndef __ACTION_H__
#define __ACTION_H__

#include <string>

enum IfCond_t
{
	IFCOND_O,								//+0//(OF=1):
	IFCOND_NO,								//+1//(OF=0):
	IFCOND_B,IFCOND_NAE = 2,IFCOND_C = 2,	//+2//(CF=1):{<}
	IFCOND_AE,IFCOND_NB = 3,IFCOND_NC = 3,	//+3//(CF=0):{>=}
	IFCOND_E,IFCOND_Z = 4,					//+4//(ZF=1):{==}
	IFCOND_NE,IFCOND_NZ = 5,				//+5//(ZF=0):{!=}
	IFCOND_BE,IFCOND_NA = 6,				//+6//[(CF=1)||(ZF=1)]:{<=}
	IFCOND_A,IFCOND_NBE = 7,				//+7//[(CF=0)&&(ZF=0)]:{>}
	IFCOND_S,								//+8//(SF=1):
	IFCOND_NS,								//+9//(SF=0):
	IFCOND_P,IFCOND_PE = 10,				//+A//(PF=1):
	IFCOND_NP,IFCOND_PO = 11,				//+B//(PF=0):
	IFCOND_L,IFCOND_NGE = 12,				//+C//(SF<>OF):{<}
	IFCOND_GE,IFCOND_NL = 13,				//+D//(SF=OF):{>=}
	IFCOND_LE,IFCOND_NG = 14,				//+E//((ZF=1) || (SF<>OF)):{<=}
	IFCOND_G,IFCOND_NLE = 15				//+F//((ZF=0) && (SF=OF)):{>}
	};

#define _ACTN2(arg)		((arg&0xFFFF)<<16)	//extra action

enum Action_t {
	ACTN_NULL = 0,

	//unary(pre)
	ACTN_UNARYPRE = ACTN_NULL,			//0
		ACTN__1,
		ACTN_RET,							//2		return from procedure
		ACTN_NOT,							//3		Bitwise NOT (~O)
		ACTN_CHS,							//4		Unary minus (-O)
		ACTN_VAR,							//5 local variable declaration
		ACTN_ARG,							//6 argument
		ACTN_OVERFLOW = 0x20,		//Overflow
		ACTN_NOVERFLOW,				//No Overflow
		ACTN_BELOW,					//Below			(<)
		ACTN_NBELOW,				//Not Below		(>=)
		ACTN_ZERO,					//Zero			(==)
		ACTN_NZERO,					//Not Zero		(!=)
		ACTN_NABOVE,				//Not Above		(<=)
		ACTN_ABOVE,					//Above			(>)
		ACTN_SIGN,					//Sign
		ACTN_NSIGN,					//No Sign
		ACTN_PARITY,				//Parity
		ACTN_NPARITY,				//No Parity
		ACTN_LESS,					//Less			(<)
		ACTN_NLESS,					//Not Less		(>=)
		ACTN_NGREATER,				//Not Greater	(<=)
		ACTN_GREATER,				//Greater		(>)
		ACTN_JMPIF = 0x20,					//20..2f	conditional jumps
		ACTN_SETIF = 0x30,					//30..3f	conditional sets
	ACTN_UNARYPRE_END = 0x40,

	ACTN_UNARYPOST = ACTN_UNARYPRE_END,

	//binary
	ACTN_BINARY = 0x40,					//40
		ACTN_MOV,							//41	Assignment (L = R)
		ACTN_CHECK/*ACTN_TEST*/,			//42	(~~)
		ACTN_ADD,							//43	Add (L + R)
		ACTN_SUB,							//44	Subtract (L - R)
		ACTN_MUL,							//45	Multiply (L * R)
		ACTN_DIV,							//46	Divide (L / R)
		ACTN_MOD,							//47	Reminder (L % R)
		ACTN_AND,							//48	Bitwise AND (L & R)	
		ACTN_OR,							//49	Bitwise OR (L | R)
		ACTN_XOR,							//4a	Biywise exlusive OR (L ^ R)
		ACTN_SHL,							//4b	Shift left (L << R)
		ACTN_SHR,							//4c	Shift Right (L >> R)
		ACTN_GOTOIF,						//goto(a)if(b);
		ACTN_UNITE,							//@(a,b)
		ACTN_POSTCALL,						//@POSTCALL(cond, expression(s))		Special action to handle post-increment conditions
		//ACTN_INTRINSIC,						// intrinsic call
		ACTN_GOTO	= 0x7E,						//1		unconditional jumps
		ACTN_CALL	= 0x7F,//?should it be unary?	//Function call (user-defined)
	ACTN_BINARY_END = 0x80,

	ACTN_INTRINSICS = ACTN_BINARY_END,		//		-//-          (special)
		ACTN_SQRT = ACTN_INTRINSICS,	
		//ACTN_MOVS,
		ACTN_SIN,
		ACTN_COS,
		ACTN_ABS,
		ACTN_2XM1,
		ACTN_SCALE,
		ACTN_RNDINT,
		ACTN_ATAN2,
		ACTN_TAN,
		ACTN_PREM,							//rem(x/y)
		ACTN_PREM1,							//rem(x/y)
		ACTN_YL2X,							//y*log2x
		ACTN_YL2XP1,						//y*log2(x +1)
		//...
		ACTN_SIGNEXT,						//b = SignExtend(a)
		ACTN_ZEROEXT,						//b = ZeroExtend(a)
	//	ACTN_CBW,							//int16_t cbw(char);	//sign extend byte->word
	//	ACTN_CWD,							//int32_t cwd(int16_t);	//sign extend word->dword
	//	ACTN_CDQ,							//int64_t cdq(int32_t);	//sign extend dword->qword
		ACTN_ROL,
		ACTN_ROR,
		ACTN_IN,
		ACTN_OUT,
		ACTN_BSWAP,
		//...
		ACTN_SCASBWNE,						//scan string while not equal (repne scasb)

	//MPL/////////////////////
		ACTN_ASSIGNCS,
		ACTN_ASSIGNDS,
		ACTN_DASSIGN,
		ACTN_DCOMMAND,
		ACTN_DCLOSE,
		ACTN_DERROR,
		ACTN_DONERROR,
		ACTN_DRESULT,
		ACTN_DSEEK,
		ACTN_DWRITE,
		ACTN_MADDS,
		ACTN_MITOA,
		ACTN_MLENGTH,
		ACTN_MODULE,
		ACTN_MON,
		ACTN_MSYNC,
	//	ACTN_MTCWX,
		ACTN_MBOUNDS,
		ACTN_MDADA4,
		ACTN_MZVARWRITE,
		ACTN_MZVARUSE,

	ACTN_INTRINSICS_END = 0x100,

	///////////////////////////////////////////////////////
	//these never show in op codes (expressions only)

	//unary(pre)
	ACTN_UNARYPRE2 = ACTN_INTRINSICS_END,	//100
	ACTN_INDIR,							//101	Indirection (*a)
	ACTN_OFFS,							//102	Relative address (&a)
	ACTN_TYPE,							//103	Type-cast {(type)a}
	ACTN_SHIFT,							//104
	ACTN_LOGNOT,						//105	Logical NOT (!a)
	ACTN_INCPRE,						//106	Pre-increment (++a)
	ACTN_DECPRE,						//107	Pre-decrement (--a)
	ACTN_SWITCH,						//108
	ACTN_LOHALF,			
	ACTN_HIHALF,
	ACTN_ADDR,							//full address (seg+offs)
	ACTN_UNARYPRE2_END = 0x120,

	//unary(post)
	ACTN_UNARYPOST2 = ACTN_UNARYPRE2_END,	//120
		ACTN_INCPOST,						//121	Post-increment (a++)
		ACTN_DECPOST,						//122	Post-decrement (a--)
	ACTN_UNARYPOST2_END = 0x140,

	//binary
	ACTN_BINARY2 = ACTN_UNARYPOST2_END,
	ACTN_ARRAY,							//141	Array[]
	ACTN_LOGAND,						//142	Logical AND (L && R)
	ACTN_LOGOR,							//143	Logical OR (L || R)
	ACTN_PTR,							//144	Pointer to structure member (L->R)
	ACTN_DOT,							//145	Structure or union member (L.R)
	ACTN_DOTEX,							//146	Pointer to member (objects) (.*)
	ACTN_PTREX,							//147	Pointer to member (pointers) (->*)
	ACTN_GET,							//148	(:=)
	ACTN_FRACT,							//149
	ACTN_ELSE,							//14a	conditional (:)
	ACTN_SEMI,							//14b	Statement separator (;)
	ACTN_SEMI2,							//14c	Used in 'for' loops
	ACTN_QUERY,							//14d	conditional (?)
	ACTN_COMMA,							//14e	Comma (L, R), function argument separator
	ACTN_COMMA0,						//14f	?
	ACTN_COMMA2,						//150	used for ptr tracing
	ACTN_COMMA3,						//151	inlined (inrowed) statement separator
	ACTN_ASSCHK,						//152	combined assignment and check

	ACTN_CMPRSN = 0x160,			
	ACTN__LESS = ACTN_CMPRSN+2,			//162	(<)
	ACTN__GREATEROREQUAL,				//163	(>=)
	ACTN__EQUAL,							//164	(==)
	ACTN__NOTEQUAL,						//165	(!=)
	ACTN__LESSOREQUAL,					//166	(<=)
	ACTN__GREATER,						//167	(>)

	__ACTN_MOVEX_BEG = 0x170,
	ACTN_MOVMUL = __ACTN_MOVEX_BEG,		//170	(*=)
	ACTN_MOVDIV,						//171	(/=)
	ACTN_MOVMOD,						//172	(%=)
	ACTN_MOVAND,						//173	(&=)
	ACTN_MOVXOR,						//174	(^=)
	ACTN_MOVOR,							//175	(|=)
	ACTN_MOVSHL,						//176	(<<=)
	ACTN_MOVSHR,						//177	(>>=)
	ACTN_MOVADD,						//178	(+=)
	ACTN_MOVSUB,						//179	(-=)
	__ACTN_MOVEX_END,

	__ACTN_LAST = __ACTN_MOVEX_END,
	ACTN_INVALID = -1
};

#define ISSETIF(arg)	((arg >= ACTN_SETIF) && (arg < ACTN_SETIF+0x10))
#define ISJMPIF(arg)	((arg >= ACTN_JMPIF) && (arg < ACTN_JMPIF+0x10))
#define ISCMPRSN0(arg)	ISJMPIF(arg)
#define ISCMPRSN(arg)	(arg >= ACTN__LESS && arg <= ACTN__GREATER)
#define ISLOGIC(arg)	((arg == ACTN_LOGAND) || (arg == ACTN_LOGOR))
#define ISBOOL(arg)		ISJMPIF(arg)
#define ISCALL(arg)			((arg) == ACTN_CALL)
#define ISINTRINSIC(arg)	((ACTN_INTRINSICS <= arg) && (arg < ACTN_INTRINSICS_END))
#define ISMOVEX(arg)		((arg) >= __ACTN_MOVEX_BEG) && ((arg) < __ACTN_MOVEX_END)

#define ISUNARYPRE(arg)		((ACTN_UNARYPRE <= arg) && (arg < ACTN_UNARYPRE_END) || (ACTN_UNARYPRE2 <= arg) && (arg < ACTN_UNARYPRE2_END))
#define ISUNARYPOST(arg)	((ACTN_UNARYPOST2 <= arg) && (arg < ACTN_UNARYPOST2_END))
#define ISBINARY(arg)		(((ACTN_BINARY <= arg) && (arg < ACTN_BINARY_END)) || (arg >= ACTN_BINARY2))


std::string ACTN2STR(Action_t);
const char *ACTN2STR2(Action_t);
Action_t IFCOND2ACTN(Action_t);

//uint16_t CPUFID2MASK(uint8_t id);
int ComparePrior(Action_t action1, Action_t action2, int32_t &result);

#endif//__ACTION_H__
