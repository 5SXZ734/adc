
#include "shared/defs.h"
#include "shared/link.h"
#include "shared/heap.h"
#include "shared/misc.h"
#include "shared/front.h"
#include "shared/instr.h"
#include "dc/globals.h"
#include "dc/obj.h"
#include "dc/types.h"
#include "dc/op.h"
#include "dc/path.h"
#include "dc/field.h"
#include "dc/func.h"
#include "dc/dc.h"
#include "dc/file.h"
#include "out.h"
#include "display.h"
#include "back.h"
#include "asm.h"
#include "interface/IADCGUI.h"

#if(0)

static IDStr_t OpSizeStr[] = {
	{OPSZ_BYTE,		"BYTE"}, 
	{OPSZ_WORD,		"WORD"}, 
	{OPSZ_DWORD,	"DWORD"}, 
	{OPSZ_FWORD,	"FWORD"},
	{OPSZ_QWORD,	"QWORD"}, 
	{OPSZ_TWORD,	"TWORD"},
	{OPSZ_NULL,		"?"}
};

static IDStr_t DataSizeStr[] = {
	{OPSZ_BYTE,			"DB"}, 
	{OPSZ_WORD,			"DW"}, 
	{OPSZ_DWORD,		"DD"}, 
	{OPSZ_FWORD,		"DF"},
	{OPSZ_QWORD,		"DQ"}, 
	{OPSZ_TWORD,		"DT"},
	{OPSZ_NULL,			"?"}
};

static char * STR_OPSIZE(int opsz)
{
	static char str[6];
	strncpy(str, OpSizeStr->GetStr(opsz), sizeof(str));
	return str;
}

static char * STR_DATASZ(int datasz)
{
	static char str[3];
	const char *p = DataSizeStr->GetStr(datasz);
	if (!p)
		return 0;
	strncpy(str, p, sizeof(str));
	return str;
}


//////////////////////////////////////////

struct Asm_t 
{
	const char *mnemo;
	Op_t *ops0;
	Op_t *ops;

	int nLines;
	int	action;
	int fpudiff;
	int stackdiff;
	int stackin;

static Heap_t arr;	//char s_ops[3][sizeof(Op_t)];
					//char t_ops[3][sizeof(Op_t)];

	Asm_t(Op_t *);
	Op_t	*Add0(Op_t *);
	Op_t	*add(Op_t *);
	Op_t	*add_indir(Op_t *pBase, Op_t *pIndex, Op_t *pScale, Op_t *pDisp);
	Op_t	&opnd(int i) {
		assert(i >= 0 && i < 3);
		return *(Op_t *)ops0->Get(i);
	}
	Op_t	&opin(int i) {
		return opnd(++i);
	}
	int DumpAsm();
	int dumpasm_MOV();
	int dumpasm_ADD();
	int dumpasm_SUB();

	void PushColor(int){}
	void PopColor(){}

	int		CheckSizeAmbiguity(Op_t *pOp);
	void	OutputASM(std::ostream &, Op_t *);
	int		OutputIndirectAsm(std::ostream &, Op_t *);
	int		OutputAddrAsm(std::ostream& os, Op_t *pOp);
};

///////////////////////////////////////////////////////////////////
Heap_t Asm_t::arr(10, sizeof(Op_t));

Asm_t::Asm_t(Op_t *pOp)
{
	assert(pOp->m_pRI);
	assert(pOp->IsAsmFirst());

	arr.Reset();

	mnemo = 0;
	ops0 = ops = 0;

	nLines = 0;//number of lines in pack
	Op_t *p = pOp;
	do {
		nLines++;
		p = p->Next();
	} while (p && !p->IsAsmFirst());
	assert(nLines);

//	Op_t &op0 = pOp->OPIn(0);
//	Op_t &op1 = pOp->OPIn(1);
	action = pOp->Action();
	fpudiff = pOp->FpuDiff();
	stackdiff = pOp->StackDiff() - pOp->GetOwnerFunc()->RetAddrSize();
	stackin = pOp->StackIn();

	if (pOp->IsRet())
		action = ACTN_RET;

	if (nLines == 1)
	{
		Add0(pOp);
		if (!pOp->IsCall() && !pOp->IsGoto())
		{
			for (Op_t *p = pOp->GetArgs();
			p;
			p = p->Next())
				Add0(p);
		}
	}
}

int Asm_t::CheckSizeAmbiguity(Op_t *pOp)
{
	assert(pOp->IsIndirect() || pOp->IsAddr());

	if (!pOp->OpSize())
		return 0;

	if (pOp->fieldRef())
		return 0;

	if (ISCALL(action)
		|| (action == ACTN_GOTO))
		return 0;

	Op_t *pOpRoot = &opnd(0);
	if (pOpRoot->OpC() == OPC_CPUREG)
		return (pOpRoot->OpSize() != pOp->OpSize());

	for (Op_t *pIn = &opin(0); 
	pIn;
	pIn = pIn->Next())
	{
		if (pIn->OpC() == OPC_CPUREG)
			return (pIn->OpSize() != pOp->OpSize());
	}

	return 1;
}

Op_t *Asm_t::Add0(Op_t *pOp)
{
	assert(pOp);
	Op_t *p = (Op_t *)arr.AddNew(true);
	p->CopyOf(pOp);
	p->Link_t::Clear();
	p->LinkTail(&ops0);

	if (p->IsLocal())//revert it back
	{
		if (p->OpC() == OPC_ADDRESS+OPC_LOCAL)
			p->m_opc = OPC_CPUREG;
		else if (p->OpC() == OPC_LOCAL)
			p->m_opc = OPC_INDIRECT+OPC_CPUREG;
		else
			assert(false);
		p->m_disp = p->m_disp0;
		p->m_disp0 = 0;
		p->setFieldRef(NULL);
	}

	p->m_pRI = 0;
	p->m_pRoot = 0;
	p->m_pXIn = 0;
	p->m_pXOut = 0;

	return p;
}

Op_t *Asm_t::add(Op_t *pOp)
{
	Op_t *p = (Op_t *)arr.AddNew(true);
	if (pOp)
	{
		p->CopyOf(pOp);
		p->Link_t::Clear();
	}
	p->LinkTail(&ops);
	return p;
}


char *ifcondstr1(UInt32 cond, bool jset, bool lwrcase)
{
static char buf[3+3];
static const char *tbl[0x10] = {
	"O",
	"NO",					
	"B",/*"C"//"NAE"*/		
	"NB",/*"AE"//"NC"*/		
	"Z",/*"E"*/				
	"NZ",/*"NE"*/			
	"NA",/*"BE"*/			
	"A",/*"NBE"*/			
	"S",					
	"NS",					
	"P",/*"PE"*/			
	"NP",/*"PO"*/			
	"L",/*"NGE"*/			
	"NL",/*"GE"*/			
	"NG",/*"LE"*/			
	"G"/*"NLE"*/
	};

	assert(cond < 0x10);
	sprintf(buf, "%s%s", (jset)?("set"):("j"), tbl[cond]);
	if (lwrcase)
	{
		//_strlwr(buf);
		char *p(buf);
		for (; *p; ++p) *p = tolower(*p);
	}
	return buf;
}

Op_t *Asm_t::add_indir(Op_t *pBase, Op_t *pIndex, Op_t *pScale, Op_t *pDisp)
{
	Op_t *pOp = add(0);
	Op_t &Op = *pOp;

	Op.m_opc = OPC_INDIRECT;

	if (pBase)
	{
		assert(pBase->OpC() == OPC_CPUREG);
		Op.m_offs = pBase->OpOffs();
		Op.m_pRoot = pBase->m_pRoot;
	}

	if (pDisp)
	{
		assert(pDisp->IsScalar());
		Op.m_disp = pDisp->m_disp;
		if (!Op.m_pRoot)
			Op.m_pRoot = pDisp->m_pRoot;
	}

	return &Op;
}

int Asm_t::dumpasm_MOV()
{
	if (opin(0).OpC() == OPC_FPUSW)
	{
		mnemo = "fnstsw";
		add(&opnd(0));
	}
	else if (opnd(0).OpC() == OPC_FPUREG)
	{
		if (opin(0).OpC() == OPC_FPUREG)
		{
			if (fpudiff == FPUD(1))
			{
				mnemo = "fstp";
				add(&opnd(0));
			}
			else if (fpudiff == FPUD(0))
			{
				mnemo = "fst";
				add(&opnd(0));
			}
			else if (fpudiff == FPUD(-1))
			{
				mnemo = "fld";
				add(&opin(0));
			}
		}
		else
		{
			assert(opnd(0).OpOffs() == 1);
			if (opin(0).IsInt())
				mnemo = "fild";
			else
				mnemo = "fld";
			add(&opin(0));
		}
	}
	else if (opin(0).OpC() == OPC_FPUREG)
	{
		assert(opin(0).OpOffs() == 1);
		if (opnd(0).IsInt())
		{
			if (fpudiff == FPUD(0))
				mnemo = "fist";
			else if (fpudiff == FPUD(1))
				mnemo = "fistp";
		}
		else
		{
			if (fpudiff == FPUD(0))
				mnemo = "fst";
			else if (fpudiff == FPUD(1))
				mnemo = "fstp";
		}
		add(&opnd(0));
	}
	else if (opnd(0).IsStackPtrB() && (opnd(0).m_disp == 0))
	{
		mnemo = "push";
		add(&opin(0));
	}
	else if (opin(0).IsStackPtrB() && (opin(0).m_disp == 0))
	{
		mnemo = "pop";
		add(&opnd(0));
	}
//	else if ((opnd(0).OpC() == OPC_CPUREG) && opin(0).IsAddr() && opin(0).IsLocal())
//	{
//		mnemo = "lea";
//		add(&opnd(0));
//		add(&opin(0));
//	}
	else
	{
		mnemo = "mov";
		add(&opnd(0));
		add(&opin(0));
		if (opnd(0).OpSize() == OPSZ_DWORD)
		{
			if (opin(0).OpSize() < OPSZ_DWORD)
			{
				if (opnd(0).IsSignedInt())
					mnemo = "movsx";
				else
					mnemo = "movsz";
			}
		}
	}

	return 1;
}

int Asm_t::dumpasm_ADD()
{
	if (opnd(0).OpC() == OPC_FPUREG)
	{
		assert(opnd(0).EqualTo(&opin(0)));
		if (opin(1).IsReal())
		{
			if (fpudiff == FPUD(0))
				mnemo = "fadd";
			else
				mnemo = "faddp";
		}
		else
			mnemo = "fiadd";

		if (opin(1).OpC() == OPC_FPUREG)
		{
			if ((opin(0).OpOffs() != 2)//st(1)
				|| (opin(1).OpOffs() != 1))//st(0)
			{
				add(&opin(0));
				add(&opin(1));
			}
		}
		else
			add(&opin(1));
	}
	else
	{
		if (opnd(0).EqualTo(&opin(0)))
		{
			if (opin(1).IsScalar() && (opin(1).OpDisp() == 1))
			{
				mnemo = "inc";
				add(&opin(0));
			}
			else
			{
				mnemo = "add";
				add(&opin(0));
				add(&opin(1));
			}
		}
		else
		{
			if (opin(0).OpC() != OPC_CPUREG)
				return 0;

			mnemo = "lea";
			add(&opnd(0));
			add_indir(&opin(0), 0, 0, &opin(1));
		}
	}

	return 1;
}

int Asm_t::dumpasm_SUB()
{
	if (opnd(0).OpC() == OPC_FPUSW)
	{
		assert(opin(0).OpC() == OPC_FPUREG);
		assert(opin(0).OpOffs() == 1);
		if (fpudiff == FPUD(0))
			mnemo = "fcom";
		else if (fpudiff == FPUD(1))
			mnemo = "fcomp";
		else if (fpudiff == FPUD(2))
			mnemo = "fcompp";
		if ((opin(1).OpC() != OPC_FPUREG) || (opin(1).OpOffs() != 2))
			add(&opin(1));
		return 1;
	}
	
	if (opnd(0).OpC() == OPC_CPUSW)
	{
		mnemo = "cmp";
		add(&opin(0));
		add(&opin(1));
		return 1;
	}
	
	if (opnd(0).OpC() == OPC_FPUREG)
	{
		if (opin(1).IsScalar())
		assert(0);//?if (opin(1).r64 == 0)
		if (opnd(0).IsReal())
		{
			mnemo = "fchs";
			return 1;
		}

		if (opnd(0).EqualTo(&opin(0)))
		{
			if (opin(1).IsReal())
			{
				if (fpudiff == FPUD(0))
					mnemo = "fsub";
				else
					mnemo = "fsubp";
			}
			else
				mnemo = "fisub";

			if (opin(1).OpC() == OPC_FPUREG)
			{
				if ((opin(0).OpOffs() != 2)//st(1)
					|| (opin(1).OpOffs() != 1))//st(0)
				{
					add(&opin(0));
					add(&opin(1));
				}
			}
			else
				add(&opin(1));
			return 1;
		}

		assert(opnd(0).EqualTo(&opin(1)));
		if (opin(0).IsReal())
		{
			if (fpudiff == FPUD(0))
				mnemo = "fsubr";
			else
				mnemo = "fsubrp";
		}
		else
			mnemo = "fisubr";

		if (opin(0).OpC() == OPC_FPUREG)
		{
			if ((opin(0).OpOffs() != 1)//st(1)
				|| (opin(1).OpOffs() != 2))//st(0)
			{
				add(&opin(1));
				add(&opin(0));
			}
		}
		else
			add(&opin(0));
		return 1;
	}

	if (opin(1).IsScalar())
	{
		if (opin(1).OpDisp() == 0)
		{
			mnemo = "neg";
			add(&opin(1));
			return 1;
		}
		
		if (opin(1).OpDisp() == 1)
		{
			mnemo = "dec";
			add(&opin(0));
			return 1;
		}
	}

	mnemo = "sub";
	add(&opin(0));
	add(&opin(1));
	return 1;
}

int Asm_t::DumpAsm()
{
//CHECK(No() == 14)
//STOP

	if (!ops0)
		return 0;

	//instruction
	if (nLines != 1)
		return 0;

	if (action == ACTN_RET)
	{
		mnemo = "ret";
		add(&opnd(0));
		return 1;
	}

	if (action == ACTN_GOTO)
	{
		mnemo = "jmp";
		add(&opnd(0));
		return 1;
	}

	if (ISCALL(action))
	{
		switch (action)
		{
		case ACTN_CALL:
			mnemo = "call";;
			add(&opnd(0));
			return 1;
		case ACTN_SIN:
			mnemo = "fsin";
			return 1;
		case ACTN_COS:
			mnemo = "fcos";
			return 1;
		case ACTN_SQRT:
			mnemo = "fsqrt";
			return 1;
		case ACTN_TAN:
			mnemo = "tan";
			return 1;
		case ACTN_ATAN2:
			mnemo = "fpatan";
			return 1;
		case ACTN_2XM1:
			mnemo = "f2xm1";
			return 1;
		case ACTN_SCALE:
			mnemo = "fscale";
			return 1;
		case ACTN_RNDINT:
			mnemo = "frndint";
			return 1;
		default:
			return 0;
			//assert(false);
		}
		return 1;
	}

	int nInOps = ops0->Count() - 1;

	if (nInOps == 1)
	{
		if (ISJMPIF(action))//IsCondJump())
		{
			mnemo = ifcondstr1(action-ACTN_JMPIF, false, true);
			add(&opnd(0));
		}
		else if (ISSETIF(action))
		{
			mnemo = ifcondstr1(action-ACTN_SETIF, true, true);
			add(&opnd(0));
		}
		else if (action == ACTN_MOV)
		{
			return dumpasm_MOV();
		}
		else if (action == ACTN_AND)
		{
			assert(opnd(0).OpC() == OPC_CPUSW);
			mnemo = "test";
			add(&opin(0));
			add(&opin(0));
		}
		else
			return 0;
		return 1;
	}
	else if (nInOps == 2)
	{
		switch (action)
		{
		case ACTN_ADD:
			return dumpasm_ADD();
		case ACTN_SUB:
			return dumpasm_SUB();
		case ACTN_AND:
			if (opnd(0).OpC() == OPC_CPUSW)
				mnemo = "test";
			else
				mnemo = "and";
			add(&opin(0));
			add(&opin(1));
			break;
		case ACTN_OR:
			mnemo = "or";
			add(&opin(0));
			add(&opin(1));
			break;
		case ACTN_MUL:
			if (opnd(0).OpC() == OPC_FPUREG)
			{
				assert(opnd(0).EqualTo(&opin(0)));
				if (opin(1).IsReal())
				{
					if (fpudiff == FPUD(0))
						mnemo = "fmul";
					else
						mnemo = "fmulp";
				}
				else
					mnemo = "fimul";

				if (opin(1).OpC() == OPC_FPUREG)
				{
					if ((opin(0).OpOffs() != 2)//st(1)
						|| (opin(1).OpOffs() != 1))//st(0)
					{
						add(&opin(0));
						add(&opin(1));
					}
				}
				else
					add(&opin(1));
			}
			else
			{
				mnemo = "mul";
				add(&opin(0));
				add(&opin(1));
			}
			break;
		case ACTN_DIV:
			if (opnd(0).OpC() == OPC_FPUREG)
			{
				if (opnd(0).EqualTo(&opin(0)))
				{
					if (opin(1).IsReal())
					{
						if (fpudiff == FPUD(0))
							mnemo = "fdiv";
						else
							mnemo = "fdivp";
					}
					else
						mnemo = "fidiv";

					if (opin(1).OpC() == OPC_FPUREG)
					{
						if ((opin(0).OpOffs() != 2)//st(1)
							|| (opin(1).OpOffs() != 1))//st(0)
						{
							add(&opin(0));
							add(&opin(1));
						}
					}
					else
						add(&opin(1));
				}
				else
				{
					assert(opnd(0).EqualTo(&opin(1)));
					if (opin(0).IsReal())
					{
						if (fpudiff == FPUD(0))
							mnemo = "fdivr";
						else
							mnemo = "fdivrp";
					}
					else
						mnemo = "fidivr";

					if (opin(0).OpC() == OPC_FPUREG)
					{
						if ((opin(0).OpOffs() != 1)//st(1)
							|| (opin(1).OpOffs() != 2))//st(0)
						{
							add(&opin(1));
							add(&opin(0));
						}
					}
					else
						add(&opin(0));
				}
			}
			else
			{
				mnemo = "div";
				add(&opin(0));
				add(&opin(1));
			}
			break;
		case ACTN_SHL:
			mnemo = "shl";
			add(&opin(0));
			add(&opin(1));
			break;
		default:
			return 0;
		}
	}//(nInOps==2)

	return 1;
}

int Asm_t::OutputAddrAsm(std::ostream& os, Op_t *pOp)
{
//	char buf[NAMELENMAX];
	assert(pOp->IsAddr());

//	if (!pOp->IsLocal())
	{
//		if (!m_pRI)
		if (!ISCALL(action))
		if (action != ACTN_GOTO)
		if (!ISJMPIF(action))
		{
			PushColor(adcui::COLOR_ASM_RESERVED);
			os << "offset";
			PopColor();
			os << " ";
		}

		if (pOp->fieldRef())
		{
			const char * pbuf = pOp->fieldRef()->namexx();
			PushColor(adcui::COLOR_ASM_NAME);
			os << pbuf;
			PopColor();
		}
	}
	
	return 1;
}

int Asm_t::OutputIndirectAsm(std::ostream& os, Op_t *pOp)
{
	char buf[NAMELENMAX];
	assert(pOp->IsIndirect());

	if (CheckSizeAmbiguity(pOp))
	{
		PushColor(adcui::COLOR_ASM_RESERVED);
		os << /*_STR*/( STR_OPSIZE(pOp->OpSize()) );
		os << " ptr ";
		PopColor();
	}

//	if (0)
//	if (opseg != OPSEG_UNDEFINED)
//		os << STR_SEGREG(opseg) << ":";

	if (pOp->fieldRef() && !pOp->IsLocal())
	{
		pOp->fieldRef()->namex(buf);
		PushColor(adcui::COLOR_ASM_NAME);
		os << buf;
		PopColor();
	}

	if (!(pOp->OpC() & OPC_INDIRECT))
		return 1;

//	bool bLocNames = true;
	Reg_t rb;
	rb.Setup(GDC.PtrSize(), pOp->OpOffs());//Id(OpC() & 0xF));
	Int64 LDisp = pOp->m_disp;

	if ( (rb.Size()) || (LDisp) )
	{
		os << "[";

		if (rb.Size())
			os << GDC.SS(SSID_CPUREG).reg2str(rb.m_sz, rb.Offset(), 0);

/*		if (IsLocal())
		{
			if (m_pData)
			{
				LDisp -= Offset0();
				LDisp -= GDC.FE.ptr_near;//?
				os << "+";
				PushColor(COLOR_ASM_LOCAL);
				os << m_pData->Name();
				PopColor();
			}
		}*/
		
		if (LDisp != 0)
		{
			if (LDisp > 0)
				os << "+";
			PushColor(adcui::COLOR_ASM_NUMBER);
			os << Int2Str(LDisp,I2S_HEXA|I2S_MODULO);
			PopColor();
		}

		os << "]";
	}

	return 1;
}

void Asm_t::OutputASM(std::ostream &os, Op_t *pOp)
{
	if (action == ACTN_RET)//IsRet())
	{
		int d = stackdiff;// - _RETADDRSIZE;
		if (d)
			os << d;
		return;
	}

	if (pOp->IsIndirect())
	{
		OutputIndirectAsm(os, pOp);
	}
	else if (pOp->IsAddr())
	{
		OutputAddrAsm(os, pOp);
	}
	else
	switch (pOp->OpC())
	{
	case OPC_IMM:
		assert(!pOp->fieldRef());//scalar

		PushColor(adcui::COLOR_ASM_NUMBER);
		if (pOp->OpType() == OPTYP_FLOAT)
		{
			int i = pOp->OpDisp();
			os << *((float *)&i);// << "f";
		}
		else if (pOp->OpType() == OPTYP_DOUBLE)
			assert(0);//?os << *((double *)&pOp->i64);
		else
		{
			UInt32 flags = I2S_HEXA;
			if (pOp->IsSignedInt())
				flags |= I2S_MODULO;

			os << Int2Str( pOp->OpDisp(), flags );
		}
		PopColor();
		break;

	case OPC_CPUREG:
		os << GDC.SS(SSID_CPUREG).reg2str(pOp->OpSize(), pOp->OpOffs(), 0);
		break;

	case OPC_FPUREG:
		{
		reg_t r;
		r.Setup(pOp->OpSize(), pOp->OpOffs());
		os << r.STR_FPUREG_ASM();
		}
		break;

	default:
		os << unk();
		//assert(false);
	}
}
/*
int Out_t::Simplify7_2(Out_t *pOut)
{
	assert(Action != ACTN_COMMA);
	assert(pOut->Action == ACTN_MOV);

	return 1;
}

int Out_t::Simplify7_1(Out_t *pOut)
{
	if (Action != ACTN_COMMA)
		return Simplify7_2(pOut);

	if (!pOutL->Simplify7_1(pOut))
		return 0;
	if (!pOutR->Simplify7_1(pOut))
		return 0;
	return 1;
}

int Out_t::Simplify7_0()
{
	if (Action != ACTN_COMMA)
		return 0;

	//((a,b),(c,d)) => ((a,b),c),d)
	if (pOutL->Action == ACTN_COMMA)
	{assert(false);
		Out_t *pOut = pOutL;
		pOut->DetachParent();//this
		pOut = pOut->pOutR->InsertParent(ACTN_COMMA);
		pOut->Add(pOutR);
		return 1;
	}

	return pOutR->Simplify7_1(pOutL);
}

*/

void DisplayAsm_t::OutputComment( const char * pc, int b )
{
	PushColor(adcui::COLOR_COMMENT);
	mos << comment();
	mos << pc;//asm comment - till end of line
}

void DisplayAsm_t::OutputLogoStr(const char * pc)
{
	PushColor(adcui::COLOR_COMMENT);
	mos << comment();
	mos << pc;//asm comment - till end of line
}

void DisplayAsm_t::OutputAsmLocal(Field_t * pData)
{
	assert(pData->IsLocal());
	PushColor(adcui::COLOR_ASM_LOCAL);
	mos << pData->namexx();
	PopColor();
	OutputTab(2);
	int offs = pData->Offset();
	if (offs < 0)
		offs += pData->GetOwnerFunc()->RetAddrSize();//?
	mos << "= " << Int2Str(offs, I2S_HEXA|I2S_MODULO);
}

void DisplayAsm_t::AlignOutput(int endpos)
{
	if (TestOpt(adcui::DUMP_TABS))
	{
		OutputTab();
	}
	else
	{
		long curpos;
		do {
			mos << ' ';
			curpos = (long)mos.tellp();
		} while (curpos < endpos);
	}
}

void DisplayAsm_t::OutputData(Field_t * pData)
{
	PushColor(adcui::COLOR_ASM_NAME);
	mos << pData->namexx();
	PopColor();
	AlignOutput(33);
	int sz = pData->size();
	char *sz_str = _STR(STR_DATASZ(sz));
	if (sz_str)
		mos << sz_str << " ?";
	else
		mos << "db " << Int2Str(sz, I2S_HEXA) << " dup(?)";
}

void DisplayAsm_t::OutputOperand0(Op_t * pOp0)
{
	assert(pOp0->IsAsmFirst());

CHECK(pOp0->No() == 102)
STOP

	Asm_t a(pOp0);
	if (!a.DumpAsm() || !a.mnemo)
	{
		EXPR_t expr(0);
		Out_t *pOutRoot = expr.AddRoot();
		Out_t *pOutU = pOutRoot;

		Op_t *pOp = pOp0;
		do {
			if (!pOutU->IsRoot())
				pOutU = pOutU->InsertParent(ACTN_COMMA);
			pOutU = expr.DumpExpression(pOp, pOutU);

			pOp = pOp->Next();
		} while (pOp && !pOp->IsAsmFirst());

		mos << FONT(adcui::FONT_ITALIC);
		OutExpr(pOutRoot);
		mos << FONT(0);
		return;
	}

	long mnemopos = (long)mos.tellp();
	PushColor(adcui::COLOR_ASM_MNEMO);
	mos << (a.mnemo);
	PopColor();

	AlignOutput(mnemopos+12);

	for (Op_t *pOp = a.ops;
	pOp;
	pOp = pOp->Next())
	{
		if (!pOp->IsFirst())
			mos << ", ";
		a.OutputASM(mos, pOp);
	}
}


void DisplayAsm_t::OutputLabelDecl(Field_t * pSelf)
{
	drawFieldRef(pSelf);
}

void DisplayAsm_t::OutputFuncBeg(Func_t * pFunc)
{
	mos << pFunc->namexx();
	OutputTab(2);
	PushColor(adcui::COLOR_ASM_RESERVED);
	mos << "proc near";
	PopColor();
}

void DisplayAsm_t::OutputFuncEnd(Func_t * pFunc)
{
//	Func_t * pFunc = pFuncDef->OwnerFunc();
	mos << pFunc->namexx();
	OutputTab(2);
	PushColor(adcui::COLOR_ASM_RESERVED);
	mos << "endp";
	PopColor();
}

void DisplayAsm_t::drawCodeLineT( Op_t * pOp )
{
	OutputPrefix(pOp);

	OutputTab(1);
	OutputOperand0(pOp);
}

int DisplayAsm_t::GetIndent( Op_t * pOp )
{
	return 1;
}

void Dumper_t::OutputInclude(File_t * pFile)
{
	char buf[64];
	pFile->namex(buf);
	OutputReserved("include");
	mos << " " << buf;
}

void DisplayAsm_t::Func_Dump(Func_t * pSelf)
{
	if (TestOpt(adcui::DUMP_ASMRAW))
		if (Func_DumpRaw(pSelf))
			return;

static const char str[] = " --------------- S U B R O U T I N E --------------------------------------- ";
	OutputComment( str, 0 );
	NewLine();

	OutputFuncBeg( pSelf );

	//FIXME!!!
//	DumpLocals();

	if (!pSelf->funcdef()->Body())
	{
		OutputBodyStub();
	}
	else
	{
		Path_t *pPathPr = 0;
		for (Path_t *pPath = pSelf->funcdef()->Body()->GetTerminalFirst();
		pPath;
		pPath = pPath->NextEx())
		{
			if (pPathPr)
			if (pPathPr->Type() == BLK_JMP)
				NewLine();
			if (pPath->Label() && pPath->Type() != BLK_EXIT)
			{
				dumpField_label2( pPath->Label() );
				NewLine();
			}

			Op_t *pOp = pPath->GetFirstOp();
			while (pOp)
				pOp = Op_DumpAsm(pOp);

			pPathPr = pPath;
		}
	}

	OutputFuncEnd( pSelf );
}


Op_t * Dumper_t::Op_DumpAsm(Op_t * pSelf)
{
	Op_t *pOp = pSelf;
	do {

		if (pOp->IsAsmFirst())
		{
			drawCodeLine(pOp);
			NewLine();
		}

		pOp = pOp->Next();
	} while (pOp && !pOp->IsAsmFirst());

	return pOp;
}


#endif