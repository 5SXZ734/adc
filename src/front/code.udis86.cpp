#include <assert.h>

#include "shared/defs.h"
#include "shared/front.h"
#include "shared/misc.h"
#include "x86_IR.h"
#include "interface/IADCMain.h"
#include "shared/data_source.h"
#include "front_IA.h"
#include "x86_dump.h"
#include <inttypes.h>
extern "C" {
#ifndef UD_NO_STDINT_DEFINE
#define UD_NO_STDINT_DEFINE
#endif
#include "udis86.h"
}
/////////////////////////////////////////////UDis86Code32_t
class UDis86Code32_t : public I_Code
{
protected:
	ud_t * ud_obj;

public:
	UDis86Code32_t()
	{
		ud_obj = new ud_t;
		ud_init(ud_obj);
		ud_set_mode(ud_obj, 32);
		ud_set_syntax(ud_obj, UD_SYN_INTEL);
	}

	virtual ~UDis86Code32_t()
	{
		delete ud_obj;
	}

	static int hook(ud_t *ud_obj)
	{
		DataStream_t *pis((DataStream_t *)ud_get_user_opaque_data(ud_obj));
		return (unsigned char)pis->read<unsigned char>();
	}

	virtual int Unassemble(DataStream_t &is, ADDR64 base, ADDR va, ins_desc_t &desc)
	{
		ins_t ins;
		int len(Unassemble(is, base, va, ins));
		desc = ins;
		return len;
	}

	int Unassemble(DataStream_t &is, ADDR64 base, ADDR va, ins_t &ins)
	{
		ins.reset();
		ud_set_pc(ud_obj, base + va);
		ud_set_input_hook(ud_obj, hook);
		ud_set_user_opaque_data(ud_obj, &is);
		int n(ud_disassemble(ud_obj));
		if (n == 0)
			return 0;
#ifdef _DEBUG
		const char* pc_hex = ud_insn_hex(ud_obj);
		const char* pc_asm = ud_insn_asm(ud_obj);
#endif
		//enum ud_mnemonic_code e = ud_insn_mnemonic(ud_obj);
		strncpy(ins.mnemo, ud_lookup_mnemonic(ud_obj->mnemonic), sizeof(ins.mnemo));

		//if (strcmp(ins.mnemo, "ret") == 0)
			//strcpy(ins.mnemo, "retn");
#if(0)
		else if (strcmp(ins.mnemo, "jz") == 0)
			strcpy(ins.mnemo, "je");
		else if (strcmp(ins.mnemo, "jnz") == 0)
			strcpy(ins.mnemo, "jne");
#endif
		ins.bIsLarge_ = ud_obj->dis_mode == 64;
		ins.length = n;
		ins.imageBase = base;
		ins.vaCur = va;
		ins.opSize = toSize(ud_obj->opr_mode);
		ins.addrSize = toSize(ud_obj->adr_mode);
		assert(n <= INS_BYTES_MAX);
		memcpy(ins.bytes, ud_insn_ptr(ud_obj), n);
		if (ud_obj->pfx_rep || ud_obj->pfx_repe)
			//ins.prefix = PFX_REP;
		//if (ud_obj->pfx_repe)
			ins.prefix = PFX_REPE;
		if (ud_obj->pfx_repne)
			ins.prefix = PFX_REPNE;

CHECK(va == 0x73e47)
STOP

		if (strstr(ins.mnemo, "jmp"))
			ins.setControlTransfer(ins_desc_t::CT_JUMP);
		else if (strstr(ins.mnemo, "call"))
			ins.setControlTransfer(ins_desc_t::CT_CALL);


		uint8_t opsize(0);//yet to be determined?
		for (int i(0); i < 3; i++)
		{
			if (ud_obj->operand[i].type == UD_NONE)
				break;
			const ud_operand_t* ud_op = &ud_obj->operand[i];
			opr_t &rop(ins.ops[i]);//dest
			if (ud_op->type == UD_OP_REG)
			{
				rop.opc = toReg(ud_op->base, rop.rbase);
				rop.optype = rop.rbase.m_sz;
				if (!opsize)
					opsize = rop.rbase.m_sz;
			}
			else if (ud_op->type == UD_OP_IMM)
			{
				rop.opc = OPC_IMM;
				//size is not specified?
				bool bExpand(false);
				uint8_t sz(ins.opSize);
				if (ud_op->size)
				{
					sz = toSize((uint8_t)ud_op->size);
					if (opsize)
					{
						if (sz < opsize)
						{
							//sign extend
							bExpand = true;
						}
						rop.optype = opsize;
					}
					else 
					{
						rop.optype = sz;
						if (sz < ins.opSize)
						{
							if (ud_obj->mnemonic != UD_Iret)
							{
								bExpand = true;
								rop.optype = ins.opSize;
							}
						}
					}
				}
				else if (opsize)//take from other op
				{
					if (sz < opsize)
					{
						//sign extend
						bExpand = true;
					}
					rop.optype = opsize;
				}
				else
					rop.optype = sz;
				//check if need for size-extent
				rop.lval.assign((void *)&ud_op->lval, sz, bExpand);

				if (ins.isControlTransfer())
				{
					assert(i == 0);
					ins.setRefVA(ins.ops[0].lval.ui32);
					rop.opc = OPC_GLOBAL|OPC_ADDRESS;
					rop.opseg.assign(OFS(R_CS), OPSZ_WORD);
				}
			}
			else if (ud_op->type == UD_OP_CONST)
			{
				rop.opc = OPC_IMM;
				assert(opsize);
				if (ud_op->lval.ubyte == 0)
				{
					rop.lval.ui8 = 0;
					rop.optype = opsize;
					//rop.rbase.m_ofs = OPID_0;
				}
				else if (ud_op->lval.ubyte == 1)
				{
					rop.lval.ui8 = 1;
					rop.optype = opsize;
					//rop.rbase.m_ofs = OPID_1;
				}
				else
					ASSERT0;
			}
			else if (ud_op->type == UD_OP_MEM)
			{
				rop.opc = OPC_INDIRECT;
				if (ud_obj->pfx_seg)
					 toReg((ud_type)ud_obj->pfx_seg, rop.opseg);
				if (ud_op->base != UD_NONE)
					toReg(ud_op->base, rop.rbase);
				if (ud_op->index != UD_NONE)
					toReg(ud_op->index, rop.rindex);
				ins.setdisp2((void *)&ud_op->lval, toSize(ud_op->offset));
				rop.scale = ud_op->scale;
//?rop.rbase.m_sz = toSize(ud_op->size);//?
				uint8_t _opsz((ud_op->size != 0 ) ? toSize((uint8_t)ud_op->size) : ins.opSize);
				if (!opsize)
					opsize = _opsz;
				bool bIndir(strcmp(ins.mnemo, "lea") != 0);
				if (ud_op->base == UD_R_RIP)
				{
					rop.vaRip = va + ins.length + rop.lval.i32;//provide hint for RIP-addressing
					ins.setRefVA(rop.vaRip, bIndir);
				}
				else
					ins.setRefVA(rop.lval.i32, bIndir);
				rop.optype = ins.isControlTransfer() ? MAKETYP_PTR(_opsz) : _opsz;
				ins.setOpSize(rop.optype);
			}
			else if (ud_op->type == UD_OP_JIMM)
			{
				opsize = toSize((uint8_t)ud_op->size);
				ADDR disp(va + ins.length);
				switch (opsize)
				{
				default:
					assert(0);
					//rop.bRel = true;
					//ins.setdisp2((void *)&ud_op->lval, opsize);
					break;
				case OPSZ_DWORD:
					disp += ud_op->lval.sdword;
					break;
				case OPSZ_WORD:
					disp += ud_op->lval.sword;
					break;
				case OPSZ_BYTE:
					//ins.setdist(DIST_SHORT);
					disp += ud_op->lval.sbyte;
					break;
				//case OPSZ_DWORD: ins.setdist(DIST_NEAR); break;
				//case OPSZ_FWORD: ins.setdist(DIST_FAR); break;
				}
				rop.lval.ui64 = base + disp;
				rop.optype = ins.addrSize;//opsize;
				if (!ins.isControlTransfer())
					ins.setControlTransfer(ins_desc_t::CT_JUMP);
				ins.setRefVA(disp);
				rop.opc = OPC_GLOBAL|OPC_ADDRESS;
				rop.opseg.assign(OFS(R_CS), OPSZ_WORD);
			}
			else
			{
				break;
			}
			ins.inc_ops_num();
		}


		ins.setFlowBreak(checkFlowBreak(ins.mnemo));

		return n;
	}

	/*bool checkRIPaddressing(ADDR64 imageBase, ADDR va, VALUE_t &v) const
	{
		for (int i(0); i < 3; i++)
		{
			const ud_operand_t &ud_op(ud_obj->operand[i]);
			if (ud_op.type == UD_OP_MEM && ud_op.base == UD_R_RIP)
			{
				v.ui64 = imageBase + va + ud_insn_len(ud_obj) + (ADDR)ud_op.lval.udword;

				if (ud_obj->dis_mode == 64)//large
				{
					
					/ *if (v.ui64 <= imageBase)
						return false;
					ADDR64 vaDif(v.ui64 - imageBase);
					if (vaDif > 0xFFFFFFFF)
						return false;* /
					v.typ = OPTYP_PTR64;
					return true;
				}
				//v.ui32 = (ADDR)imageBase + rip_va;
				v.typ = OPTYP_PTR32;
				return true;
			}
		}
		return false;
	}*/

	static uint8_t toSize(uint8_t sz)
	{
		if (sz == 128)
			return 16;
		if (sz == 80)
			return 10;
		if (sz == 64)
			return 8;
		if (sz == 32)
			return 4;
		if (sz == 16)
			return 2;
		if (sz == 8)
			return 1;
		assert(sz == 0);
		return 0;
	}
	static OPC_t toReg(ud_type t, reg_t &r)
	{
#define CASE_R(arg)	case UD_R_##arg: r = reg_t(OFS(R_##arg), SIZ(R_##arg)); return (OPC_t)RCL(R_##arg);
		switch (t)
		{
		default:
			break;
			//8 bit GPRs
			CASE_R(AL);
			CASE_R(CL);
			CASE_R(DL);
			CASE_R(BL);
			CASE_R(AH);
			CASE_R(CH);
			CASE_R(DH);
			CASE_R(BH);
			// 16 bit GPRs
			CASE_R(AX);
			CASE_R(CX);
			CASE_R(DX);
			CASE_R(BX);
			CASE_R(SP);
			CASE_R(BP);
			CASE_R(SI);
			CASE_R(DI);
			// 32 bit GPRs
			CASE_R(EAX);
			CASE_R(ECX);
			CASE_R(EDX);
			CASE_R(EBX);
			CASE_R(ESP);
			CASE_R(EBP);
			CASE_R(ESI);
			CASE_R(EDI);
			// segment registers
			CASE_R(ES);
			CASE_R(CS);
			CASE_R(SS);
			CASE_R(DS);
			CASE_R(FS);
			CASE_R(GS);
			// x87 registers
			CASE_R(ST0);
			CASE_R(ST1);
			CASE_R(ST2);
			CASE_R(ST3);
			CASE_R(ST4);
			CASE_R(ST5);
			CASE_R(ST6);
			CASE_R(ST7);

#if(X64_SUPPORT)
			//8 bit GPRs
			CASE_R(SPL);
			CASE_R(BPL);
			CASE_R(SIL);
			CASE_R(DIL);
			CASE_R(R8B);
			CASE_R(R9B);
			CASE_R(R10B);
			CASE_R(R11B);
			CASE_R(R12B);
			CASE_R(R13B);
			CASE_R(R14B);
			CASE_R(R15B);
			//16-bit GPRs
			CASE_R(R8W);
			CASE_R(R9W);
			CASE_R(R10W);
			CASE_R(R11W);
			CASE_R(R12W);
			CASE_R(R13W);
			CASE_R(R14W);
			CASE_R(R15W);
			// 32 bit GPRs
			CASE_R(R8D);
			CASE_R(R9D);
			CASE_R(R10D);
			CASE_R(R11D);
			CASE_R(R12D);
			CASE_R(R13D);
			CASE_R(R14D);
			CASE_R(R15D);
			// 64 bit GPRs
			CASE_R(RAX);
			CASE_R(RCX);
			CASE_R(RDX);
			CASE_R(RBX);
			CASE_R(RSP);
			CASE_R(RBP);
			CASE_R(RSI);
			CASE_R(RDI)
			CASE_R(R8);
			CASE_R(R9);
			CASE_R(R10);
			CASE_R(R11);
			CASE_R(R12);
			CASE_R(R13);
			CASE_R(R14);
			CASE_R(R15);
			//rip
			CASE_R(RIP);
#endif

			// mmx registers
			CASE_R(MM0);
			CASE_R(MM1);
			CASE_R(MM2);
			CASE_R(MM3);
			CASE_R(MM4);
			CASE_R(MM5);
			CASE_R(MM6);
			CASE_R(MM7);

			// extended multimedia registers
			CASE_R(XMM0);
			CASE_R(XMM1);
			CASE_R(XMM2);
			CASE_R(XMM3);
			CASE_R(XMM4);
			CASE_R(XMM5);
			CASE_R(XMM6);
			CASE_R(XMM7);
			CASE_R(XMM8);
			CASE_R(XMM9);
			CASE_R(XMM10);
			CASE_R(XMM11);
			CASE_R(XMM12);
			CASE_R(XMM13);
			CASE_R(XMM14);
			CASE_R(XMM15);

			// control registers
			CASE_R(CR0);
			CASE_R(CR1);
			CASE_R(CR2);
			CASE_R(CR3);
			CASE_R(CR4);
			CASE_R(CR5);
			CASE_R(CR6);
			CASE_R(CR7);
			CASE_R(CR8);
			CASE_R(CR9);
			CASE_R(CR10);
			CASE_R(CR11);
			CASE_R(CR12);
			CASE_R(CR13);
			CASE_R(CR14);
			CASE_R(CR15);

			// debug registers
			CASE_R(DR0);
			CASE_R(DR1);
			CASE_R(DR2);
			CASE_R(DR3);
			CASE_R(DR4);
			CASE_R(DR5);
			CASE_R(DR6);
			CASE_R(DR7);
		}
#undef CASE_R
		assert(0);
		return OPC_NULL;
	}
	virtual int Print(DataStream_t &is, ADDR64 base, ADDR va, IOutpADDR2Name *cb, ins_desc_t &desc)
	{
		ins_t ins;
		int len(Unassemble(is, base, va, ins));
		if (len > 0)
		{
			desc = ins;
			x86_Print(ins, cb);
			
			VALUE_t v;
			if (ins.checkRIPaddressing(v))//base, va, v))
				cb->on_RIP_relative_addressing(v);
		}
		return len;
	}
	virtual int Skip(DataStream_t &is, ADDR64 base, ADDR va, ins_desc_t &desc)
	{
		ud_set_pc(ud_obj, base + va);
		ud_set_input_hook(ud_obj, hook);
		ud_set_user_opaque_data(ud_obj, &is);
		int n(ud_disassemble(ud_obj));
		desc.flowBreak = checkFlowBreak(ud_insn_asm(ud_obj));
		return n;
	}
	static int checkFlowBreak(const char *mnemo)
	{
		if (strstr(mnemo, "jmp"))
			return 1;
		if (strstr(mnemo, "ret"))
			return 2;
		return 0;
	}
	virtual int Generate(DataStream_t &is, ADDR64 base, ADDR va, ins_desc_t &desc, IPCode_t &pcode, const I_FrontDC &rFrontDC)
	{
		ins_t ins;
		int len(Unassemble(is, base, va, ins));
		if (len > 0)
		{
			if (!PCodeCreate(ins, *rFrontDC.GetFE(), pcode))
				return 0;
			desc = ins;
		}
		return len;
	}
};

DECLARE_CODE_TYPE(UDis86Code32_t, UDIS86_32);


/////////////////////////////////////////////UDis86Code32_t
class UDis86Code64_t : public UDis86Code32_t
{
public:
	UDis86Code64_t()
	{
		ud_set_mode(ud_obj, 64);
	}
};

DECLARE_CODE_TYPE(UDis86Code64_t, UDIS86_64);

/////////////////////////////////////////////UDis86Code16_t
class UDis86Code16_t : public UDis86Code32_t
{
public:
	UDis86Code16_t()
	{
		ud_set_mode(ud_obj, 16);
	}
};

DECLARE_CODE_TYPE(UDis86Code16_t, UDIS86_16);


void DeclareUDis86Types(I_ModuleEx &mr)
{
	mr.DeclareCodeType(_PFX("UDIS86_16"));
	mr.DeclareCodeType(_PFX("UDIS86_32"));
	mr.DeclareCodeType(_PFX("UDIS86_64"));
}

