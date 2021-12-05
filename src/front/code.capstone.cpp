#include <assert.h>

#include "shared/defs.h"
#include "shared/front.h"
#include "shared/misc.h"
#include "interface/IADCMain.h"
#include "interface/IADCFront.h"
#include "shared/data_source.h"
#include "x86_IR.h"
#include "front_IA.h"
#include "x86_dump.h"
#include <inttypes.h>

#include "capstone/capstone.h"

class CS_t : public I_Code
{
protected:
	csh m_handle;
	cs_insn *m_insn;
	cs_arch m_arch;
	cs_mode m_mode;
public:
	CS_t(cs_arch arch, cs_mode mode)
		: m_handle(0),
		m_insn(nullptr),
		m_arch(arch),
		m_mode(mode)
	{
	}
	virtual ~CS_t()
	{
		destroy();
	}
protected:
	bool create(bool bDetail)
	{
		if (m_handle)
			return true;//already
		if (!cs_open(m_arch, m_mode, &m_handle) == CS_ERR_OK)
			return false;
		if (bDetail)
		{
			// turn ON detail feature - MUST BE SET BEFORE THE NEXT STATEMENT!
			cs_option(m_handle, CS_OPT_DETAIL, CS_OPT_ON);
		}
		// allocate memory cache for 1 instruction, to be used by cs_disasm_iter later.
		m_insn = cs_malloc(m_handle);
		return true;
	}
	void destroy()
	{
		if (m_insn)
		{
			cs_free(m_insn, 1);
			m_insn = nullptr;
		}
		if (m_handle)
			cs_close(&m_handle);
	}
	int disassemble1(DataStream_t &is, uint64_t address, bool bDetail)
	{
		create(bDetail);//assure created

		uint8_t code_buf[INS_BYTES_MAX];//16 max instruction size(?)
		OFF_t oBeg(is.tell());//save stream position
		size_t code_size(0);
		//read max possible (for x86 instruction) number of bytes but avoid data access fault
		for (; code_size < sizeof(code_buf) && !is.isAtEnd(); code_size++)
			code_buf[code_size] = is.read<uint8_t>();
			//size_t code_size(is.read(sizeof(code_buf), (PDATA)code_buf));
		const uint8_t *code(code_buf);
		if (!cs_disasm_iter(m_handle, &code, &code_size, &address, m_insn))
			return 0;

		int insn_size(m_insn->size);
		assert(insn_size <= INS_BYTES_MAX);
		is.seek(oBeg + insn_size);//advance stream position to instruction length only

		return insn_size;
	}
};

class CS_X86_t : public CS_t
{
public:
	CS_X86_t(cs_arch arch, cs_mode mode)
		: CS_t(arch, mode)
	{
	}
protected:
	virtual int Unassemble(DataStream_t &is, ADDR64 base, ADDR va, ins_desc_t &desc)
	{
		ins_t ins;
		int len(Unassemble(is, base, va, ins));
		desc = ins;
		return len;
	}

	int Unassemble(DataStream_t &is, ADDR64 base, ADDR va, ins_t &ins)
	{
CHECK(va == 0x2066724c)
STOP
		int insn_size(disassemble1(is, base + va, true));
		if (insn_size == 0)
			return 0;
	
		static mnemo_alias_map m;
		const char *insn_name(cs_insn_name(m_handle, m_insn->id));
		assert(strlen(insn_name) < INS_MNEMO_MAX);

		ins.reset();
		strncpy(ins.mnemo, m.remapped(insn_name), sizeof(ins.mnemo));
		//strncpy(ins.mnemo, m_insn->mnemonic, sizeof(ins.mnemo));

		//if (strcmp(ins.mnemo, "ret") == 0)
			//strcpy(ins.mnemo, "retn");

		//uint8_t groups_count = m_insn->detail->groups_count;
		uint8_t *bytes = m_insn->bytes;

		const cs_x86 &detail(m_insn->detail->x86);

		ins.length = (uint8_t)insn_size;
		ins.imageBase = base;
		ins.vaCur = va;
		ins.opSize = toOpSize();
		ins.addrSize = detail.addr_size;
		ins.bIsLarge_ = base != 0;
		
		memcpy(ins.bytes, m_insn->bytes, insn_size);
		if (detail.prefix[0] == X86_PREFIX_REP)
			ins.prefix = PFX_REPE;//?PFX_REP
		else if (detail.prefix[0] == X86_PREFIX_REPE)
			ins.prefix = PFX_REPE;
		else if (detail.prefix[0] == X86_PREFIX_REPNE)
			ins.prefix = PFX_REPNE;

		implicit_insn_ops_map m2;//check implicit ops
		if (m2.check(insn_name))
			return insn_size;

//CHECK(strcmp(m_insn->mnemonic, "jne") == 0)
CHECK(va == 0x605cad)
STOP

		for (int j(0); j < m_insn->detail->groups_count; j++)
		{
			if (m_insn->detail->groups[j] == CS_GRP_JUMP)
				ins.setControlTransfer(ins_desc_t::CT_JUMP);
			else if (m_insn->detail->groups[j] == CS_GRP_CALL)
				ins.setControlTransfer(ins_desc_t::CT_CALL);
		}

		REG_t r;
		uint8_t opsize(0);
		for (int i(0); i < detail.op_count; i++)
		{
			const cs_x86_op &op(detail.operands[i]);
			opr_t &rop(ins.ops[i]);//dest

			if (op.type == X86_OP_REG)
			{
				rop.opc = toReg(op.reg, r);
				if (rop.opc == SSID_SIMD)
				{
					assert(IsPowerOf2(r.m_siz));
					r.m_siz = LowestBitPosition(r.m_siz);
				}
				rop.rbase = r.to_reg();
				rop.optype = rop.rbase.m_sz;
				if (!opsize)
					opsize = r.m_siz;
			}
			else if (op.type == X86_OP_IMM)
			{
				rop.opc = OPC_IMM;
				//size is not specified?
				bool bExpand(false);
				uint8_t sz(ins.opSize);
				if (op.size)
				{
					sz = op.size;
					if (m_insn->id == X86_INS_RET)
					{
						if (sz != OPSZ_WORD)
							sz = OPSZ_WORD;//wrong op size reported!
					}
					/*if (opsize)
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
							if (m_insn->id != X86_INS_RET)
							{
								bExpand = true;
								rop.optype = ins.opSize;
							}
						}
						if (m_insn->id == X86_INS_RET)
						{
							if (rop.optype != OPSZ_WORD)
								rop.optype = OPSZ_WORD;
						}
					}*/
					rop.optype = sz;
				}
				else if (opsize)//take from other op
				{
					/*if (sz < opsize)
					{
						//sign extend
						bExpand = true;
					}*/
					rop.optype = opsize;
				}
				else
					rop.optype = sz;
				//check if need for size-extent
				rop.lval.assign((void *)&op.imm, sz, bExpand);

				if (ins.isControlTransfer())
				{
					if (i == 0)//ljmp!
					{
						ins.setRefVA(ADDR(ins.ops[0].lval.ui32 - base));
						rop.opc = OPC_GLOBAL|OPC_ADDRESS;
						rop.opseg.assign(OFS(R_CS), OPSZ_WORD);
					}
				}

				/*if (grp_info == (1 << CS_GRP_JUMP) + (1 << CS_GRP_BRANCH_RELATIVE))
				{
					rop.bRel = true;
					switch (detail.encoding.imm_size)
					{
					default: break;
					case OPSZ_BYTE: ins.setdist(DIST_SHORT); break;
						//case OPSZ_DWORD: ins.setdist(DIST_NEAR); break;
						//case OPSZ_FWORD: ins.setdist(DIST_FAR); break;
					}
					ins.b_xctrl = 1;
				}*/
			}
			else if (op.type == X86_OP_MEM)
			{
				const x86_op_mem &op_mem(op.mem);
				rop.opc = OPC_INDIRECT;
				if (detail.prefix[1] != 0)
					 toSegReg((x86_prefix)detail.prefix[1], rop.opseg);
				if (op_mem.base != X86_REG_INVALID)
				{
					toReg(op_mem.base, r);
					rop.rbase = r.to_reg();
				}
				if (op_mem.index != X86_REG_INVALID)
				{
					toReg(op_mem.index, r);
					rop.rindex = r.to_reg();
				}
				ins.setdisp2((void *)&op_mem.disp, detail.addr_size);
				rop.scale = op_mem.scale > 1 ? op_mem.scale : 0;
				//?rop.rbase.m_sz = toSize(ud_op->size);//?
				uint8_t _opsz((op.size != 0) ? op.size : ins.opSize);
				if (opsize == 0)
					opsize = _opsz;
				bool bIndir(strcmp(ins.mnemo, "lea") != 0);
				if (op_mem.base == X86_REG_RIP)
				{
					rop.vaRip = va + ins.length + rop.lval.i32;//provide hint for RIP-addressing
					ins.setRefVA(rop.vaRip, bIndir);
				}
				else
					ins.setRefVA(rop.lval.i32, bIndir);
				rop.optype = ins.isControlTransfer() ? MAKETYP_PTR(_opsz) : _opsz;
				ins.setOpSize(rop.optype);
				/*if (rop.opseg.empty() && m_insn->id != X86_INS_LEA)
				{
					if (op.mem.base == X86_REG_ESP || op.mem.base == X86_REG_EBP)
						rop.opseg.assign(OFS(R_SS), OPSZ_WORD);
					else
						rop.opseg.assign(OFS(R_DS), OPSZ_WORD);
				}*/
			}
			else
			{
				break;//?
			}
			ins.inc_ops_num();
		}

		ins.setFlowBreak(checkFlowBreak(ins.mnemo));
		return insn_size;
	}

	/*bool checkRIPaddressing(ADDR va) const
	{
		const cs_x86 &detail(m_insn->detail->x86);
		for (int i(0); i < detail.op_count; i++)
		{
			const cs_x86_op &op(detail.operands[i]);
			if (op.type == X86_OP_MEM)
			{
				const x86_op_mem &op_mem(op.mem);
				if (op_mem.base == X86_REG_RIP)
				{
					ADDR rip_va = va + m_insn->size + (ADDR)op_mem.disp;
					return true;//no more
				}
			}
		}
		return false;
	}*/

	static int checkFlowBreak(const char *mnemo)
	{
		if (strstr(mnemo, "jmp"))
			return 1;
		if (strstr(mnemo, "ret"))
			return 2;
		return 0;
	}
private:
	uint8_t toOpSize() const
	{
		if (m_mode == CS_MODE_16)
			return 2;
		if (m_mode == CS_MODE_32)
			return 4;
		if (m_mode == CS_MODE_64)
			return 8;
		assert(0);
		return 0;
	}
	static OPC_t toSegReg(x86_prefix t, reg_t &r)
	{
	#define CASE_SR(arg)	case X86_PREFIX_##arg: r = reg_t(OFS(R_##arg), SIZ(R_##arg)); return (OPC_t)RCL(R_##arg);
		switch (t)
		{
		default:
			break;
		CASE_SR(CS);
		CASE_SR(SS);
		CASE_SR(DS);
		CASE_SR(ES);
		CASE_SR(FS);
		CASE_SR(GS);
		}
		assert(0);
		return OPC_NULL;
	}

	static OPC_t toReg(x86_reg t, REG_t &r)
	{
#define CASE_R(arg)	case X86_REG_##arg: r = REG_t(OFS(R_##arg), SIZ(R_##arg)); return (OPC_t)RCL(R_##arg);
		switch (t)
		{
		default:
			break;
		//8 bit GPRs
		CASE_R(AL); CASE_R(CL); CASE_R(DL); CASE_R(BL); CASE_R(AH); CASE_R(CH); CASE_R(DH); CASE_R(BH);
		// 16 bit GPRs
		CASE_R(AX); CASE_R(CX); CASE_R(DX); CASE_R(BX); CASE_R(SP); CASE_R(BP); CASE_R(SI); CASE_R(DI);
		// 32 bit GPRs
		CASE_R(EAX); CASE_R(ECX); CASE_R(EDX); CASE_R(EBX); CASE_R(ESP); CASE_R(EBP); CASE_R(ESI); CASE_R(EDI);
		// segment registers
		CASE_R(ES); CASE_R(CS); CASE_R(SS); CASE_R(DS); CASE_R(FS); CASE_R(GS);
		// x87 registers
		CASE_R(ST0); CASE_R(ST1); CASE_R(ST2); CASE_R(ST3); CASE_R(ST4); CASE_R(ST5); CASE_R(ST6); CASE_R(ST7);

#if(X64_SUPPORT)
		//8 bit GPRs
		CASE_R(SPL); CASE_R(BPL); CASE_R(SIL); CASE_R(DIL); 
		CASE_R(R8B); CASE_R(R9B); CASE_R(R10B); CASE_R(R11B); CASE_R(R12B); CASE_R(R13B); CASE_R(R14B); CASE_R(R15B);
		//16-bit GPRs
		CASE_R(R8W); CASE_R(R9W); CASE_R(R10W); CASE_R(R11W); CASE_R(R12W); CASE_R(R13W); CASE_R(R14W); CASE_R(R15W);
		// 32 bit GPRs
		CASE_R(R8D); CASE_R(R9D); CASE_R(R10D); CASE_R(R11D); CASE_R(R12D); CASE_R(R13D); CASE_R(R14D); CASE_R(R15D);
		// 64 bit GPRs
		CASE_R(RAX); CASE_R(RCX); CASE_R(RDX); CASE_R(RBX); CASE_R(RSP); CASE_R(RBP); CASE_R(RSI); CASE_R(RDI)
		CASE_R(R8); CASE_R(R9); CASE_R(R10); CASE_R(R11); CASE_R(R12); CASE_R(R13); CASE_R(R14); CASE_R(R15);
		//rip
		CASE_R(RIP);
#endif
		// mmx registers
		CASE_R(MM0); CASE_R(MM1); CASE_R(MM2); CASE_R(MM3); CASE_R(MM4); CASE_R(MM5); CASE_R(MM6); CASE_R(MM7);

		// extended multimedia registers
		CASE_R(XMM0); CASE_R(XMM1); CASE_R(XMM2); CASE_R(XMM3); CASE_R(XMM4); CASE_R(XMM5); CASE_R(XMM6); CASE_R(XMM7);
		CASE_R(XMM8); CASE_R(XMM9); CASE_R(XMM10); CASE_R(XMM11); CASE_R(XMM12); CASE_R(XMM13); CASE_R(XMM14); CASE_R(XMM15);

		// control registers
		CASE_R(CR0); CASE_R(CR1); CASE_R(CR2);  CASE_R(CR3); CASE_R(CR4); CASE_R(CR5); CASE_R(CR6); CASE_R(CR7); 
		CASE_R(CR8); CASE_R(CR9); CASE_R(CR10); CASE_R(CR11); CASE_R(CR12); CASE_R(CR13); CASE_R(CR14); CASE_R(CR15);

		// debug registers
		CASE_R(DR0); CASE_R(DR1); CASE_R(DR2); CASE_R(DR3); CASE_R(DR4); CASE_R(DR5); CASE_R(DR6); CASE_R(DR7);

		CASE_R(YMM0); CASE_R(YMM1); CASE_R(YMM2); CASE_R(YMM3); CASE_R(YMM4); CASE_R(YMM5); CASE_R(YMM6); CASE_R(YMM7);
		CASE_R(YMM8); CASE_R(YMM9); CASE_R(YMM10); CASE_R(YMM11); CASE_R(YMM12); CASE_R(YMM13); CASE_R(YMM14); CASE_R(YMM15);

		//AVX bitmask registers
		CASE_R(K0); CASE_R(K1); CASE_R(K2); CASE_R(K3); CASE_R(K4);	CASE_R(K5); CASE_R(K6); CASE_R(K7);

		}
#undef CASE_R
		assert(0);
		return OPC_NULL;
	}
	class mnemo_alias_map : public std::map<std::string, std::string>
	{
	public:
		mnemo_alias_map()
		{
			(*this)["je"] = "jz";
			(*this)["jne"] = "jnz";
			(*this)["sete"] = "setz";
			(*this)["setne"] = "setnz";
		}
		const char *remapped(const char *s) const
		{
			const_iterator i(find(s));
			if (i != end())
				return i->second.c_str();
			return s;
		}
	};

	class implicit_insn_ops_map : public std::set<std::string>
	{
	public:
		implicit_insn_ops_map()
		{
			insert("stosb");
			insert("stosw");
			insert("stosd");
			insert("stosq");
			insert("movsb");
			insert("movsw");
			insert("movsd");
			insert("movsq");
			insert("scasb");
			insert("scasw");
			insert("scasd");
			insert("scasq");
		}
		bool check(const char* s) const
		{
			const_iterator i(find(s));
			return (i != end());
		}
	};
	virtual int Print(DataStream_t &is, ADDR64 base, ADDR va, IOutpADDR2Name *cb, ins_desc_t &desc)
	{
		ins_t ins;
		if (Unassemble(is, base, va, ins) > 0)
		{
			desc = ins;
			x86_Print(ins, cb);

			VALUE_t v;
			if (ins.checkRIPaddressing(v) >= 0)
			{
				//v.optyp = ins.rip_target(ins.bIsLarge, v);
				cb->on_RIP_relative_addressing(v);
			}
			return ins.length;
		}
		return 0;
	}
	virtual int Skip(DataStream_t &is, ADDR64 base, ADDR va, ins_desc_t &desc)
	{
		int insn_size(disassemble1(is, base + va, true));
		if (insn_size > 0)
		{
			const char *insn_name(cs_insn_name(m_handle, m_insn->id));
			desc.flowBreak = checkFlowBreak(insn_name);
		}
		return insn_size;
	}
	virtual int Generate(DataStream_t &is, ADDR64 base, ADDR va, ins_desc_t &desc, IPCode_t &pcode, const I_FrontDC &rFrontDC)
	{
CHECK(va==0x000e8ca5)
STOP
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

////////////////////////////// CS_X86_32_t
class CS_X86_32_t : public CS_X86_t
{
public:
	CS_X86_32_t()
		: CS_X86_t(CS_ARCH_X86, CS_MODE_32)
	{
	}
};

DECLARE_CODE_TYPE(CS_X86_32_t, CAPSTONE_X86_32);

////////////////////////////// CS_X86_64_t
class CS_X86_64_t : public CS_X86_t
{
public:
	CS_X86_64_t()
		: CS_X86_t(CS_ARCH_X86, CS_MODE_64)
	{
	}
};

DECLARE_CODE_TYPE(CS_X86_64_t, CAPSTONE_X86_64);

////////////////////////////// CS_X86_16_t
class CS_X86_16_t : public CS_X86_t
{
public:
	CS_X86_16_t()
		: CS_X86_t(CS_ARCH_X86, CS_MODE_16)
	{
	}
};

DECLARE_CODE_TYPE(CS_X86_16_t, CAPSTONE_X86_16);


////////////////////////////// CS_ARM_t
class CS_ARM_t : public CS_t
{
public:
	CS_ARM_t()
		: CS_t(CS_ARCH_ARM, CS_MODE_ARM)
	{
	}
protected:
	virtual int Unassemble(DataStream_t &is, ADDR64 base, ADDR va, ins_desc_t &desc)
	{
		ins_t ins;
		int len(Unassemble(is, base, va, ins));
		desc = ins;
		return len;
	}
	int Unassemble(DataStream_t &is, ADDR64 base, ADDR va, ins_t &ins)
	{
		int insn_size(disassemble1(is, base + va, true));
		if (insn_size == 0)
			return 0;

		ins.reset();
		strncpy(ins.mnemo, m_insn->mnemonic, sizeof(ins.mnemo));

		const cs_arm &detail(m_insn->detail->arm);

		ins.length = (uint8_t)insn_size;
		ins.imageBase = base;
		ins.vaCur = va;
		//ins.u_opsize = toOpSize();
		//ins.addrSize = detail.addr_size;
		
		memcpy(ins.bytes, m_insn->bytes, insn_size);

CHECK(va == 0x107810)
STOP

		int xctrl;
		ins.setFlowBreak(checkFlowBreak(xctrl));
		ins.setControlTransfer(xctrl);

		for (int i(0); i < std::min((uint8_t)INS_OPS_MAX, detail.op_count); i++)//op count may be greater
		{
			const cs_arm_op &op(detail.operands[i]);
			opr_t& rop(ins.ops[i]);
			switch (op.type)
			{
			default:
				break;
			case ARM_OP_REG:
				rop.optype = OPSZ_DWORD;
				rop.opc = OPC_CPUREG;
				rop.rbase.assign(op.type, OPSZ_DWORD);
				//toReg(op.reg);
				break;
			case ARM_OP_IMM:
				rop.optype = OPSZ_DWORD;
				rop.lval.assign((void *)&op.imm, OPSZ_DWORD, false);
				if (ins.isControlTransfer())
					ins.setRefVA(op.imm);
				break;
			case ARM_OP_MEM:
				if (op.mem.base == ARM_REG_PC)
				{
					rop.vaRip = va + ins.length + op.mem.disp + 4;//provide hint for RIP-addressing
					ins.setRefVA(rop.vaRip, true);
				}
				else
					ins.setRefVA(op.mem.disp, true);
				ins.setOpSize(OPSZ_DWORD);
				break;
			case ARM_OP_FP:
				break;
			}
			ins.inc_ops_num();
		}

		return insn_size;
	}
	int checkFlowBreak(int &xctrl)
	{
		int brk(0);
		xctrl = ins_desc_t::CT_NONE;
		for (int j(0); j < m_insn->detail->groups_count; j++)
		{
			if (m_insn->detail->groups[j] == CS_GRP_JUMP)
			{
				const cs_arm &arm(m_insn->detail->arm);
				if (xctrl != ins_desc_t::CT_CALL)//not a call!
				{
					xctrl = ins_desc_t::CT_JUMP;
					if (arm.cc == ARM_CC_INVALID)//conditional?
						brk = ins_desc_t::FB_JUMP;
				}
			}
			else if (m_insn->detail->groups[j] == CS_GRP_CALL)
			{
				xctrl = ins_desc_t::CT_CALL;
				brk = ins_desc_t::FB_NONE;
			}
			else if (m_insn->detail->groups[j] == CS_GRP_RET)
			{
				brk = ins_desc_t::FB_RET;
			}
		}
		return brk;
	}
	const char *toReg(int id)
	{
#define CASE_R(a)	case ARM_REG_##a: return #a;
		switch (id)
		{
		default:
			break;
		CASE_R(APSR);
		CASE_R(APSR_NZCV);
		CASE_R(CPSR);
		CASE_R(FPEXC);
		CASE_R(FPINST);
		CASE_R(FPSCR);
		CASE_R(FPSCR_NZCV);
		CASE_R(FPSID);
		CASE_R(ITSTATE);
		CASE_R(LR);
		CASE_R(PC);
		CASE_R(SP);
		CASE_R(SPSR);
		CASE_R(D0);
		CASE_R(D1);
		CASE_R(D2);
		CASE_R(D3);
		CASE_R(D4);
		CASE_R(D5);
		CASE_R(D6);
		CASE_R(D7);
		CASE_R(D8);
		CASE_R(D9);
		CASE_R(D10);
		CASE_R(D11);
		CASE_R(D12);
		CASE_R(D13);
		CASE_R(D14);
		CASE_R(D15);
		CASE_R(D16);
		CASE_R(D17);
		CASE_R(D18);
		CASE_R(D19);
		CASE_R(D20);
		CASE_R(D21);
		CASE_R(D22);
		CASE_R(D23);
		CASE_R(D24);
		CASE_R(D25);
		CASE_R(D26);
		CASE_R(D27);
		CASE_R(D28);
		CASE_R(D29);
		CASE_R(D30);
		CASE_R(D31);
		CASE_R(FPINST2);
		CASE_R(MVFR0);
		CASE_R(MVFR1);
		CASE_R(MVFR2);
		CASE_R(Q0);
		CASE_R(Q1);
		CASE_R(Q2);
		CASE_R(Q3);
		CASE_R(Q4);
		CASE_R(Q5);
		CASE_R(Q6);
		CASE_R(Q7);
		CASE_R(Q8);
		CASE_R(Q9);
		CASE_R(Q10);
		CASE_R(Q11);
		CASE_R(Q12);
		CASE_R(Q13);
		CASE_R(Q14);
		CASE_R(Q15);
		CASE_R(R0);
		CASE_R(R1);
		CASE_R(R2);
		CASE_R(R3);
		CASE_R(R4);
		CASE_R(R5);
		CASE_R(R6);
		CASE_R(R7);
		CASE_R(R8);
		CASE_R(R9);
		CASE_R(R10);
		CASE_R(R11);
		CASE_R(R12);
		CASE_R(S0);
		CASE_R(S1);
		CASE_R(S2);
		CASE_R(S3);
		CASE_R(S4);
		CASE_R(S5);
		CASE_R(S6);
		CASE_R(S7);
		CASE_R(S8);
		CASE_R(S9);
		CASE_R(S10);
		CASE_R(S11);
		CASE_R(S12);
		CASE_R(S13);
		CASE_R(S14);
		CASE_R(S15);
		CASE_R(S16);
		CASE_R(S17);
		CASE_R(S18);
		CASE_R(S19);
		CASE_R(S20);
		CASE_R(S21);
		CASE_R(S22);
		CASE_R(S23);
		CASE_R(S24);
		CASE_R(S25);
		CASE_R(S26);
		CASE_R(S27);
		CASE_R(S28);
		CASE_R(S29);
		CASE_R(S30);
		CASE_R(S31);
		}
		return "?";
	}
	void filterOpString(const char *pc, MyStrStream &ss, int iColor)
	{
		const char *p(pc);
		while (*p)
		{
			if (p[0] == '#' && p[1] == '0' && p[2] == 'x')
			{
				ss << *p, p++;
				ss.color(iColor);
				while (isalnum(*p))
					ss << *p, p++;
				ss.color(adcui::COLOR_POP);
			}
			ss << *p, p++;
		}
	}
	bool checkRIPaddressing(ADDR va, ADDR &rip_va) const
	{
		const cs_arm &detail(m_insn->detail->arm);
		for (int i(0); i < std::min((uint8_t)INS_OPS_MAX, detail.op_count); i++)//op count may be greater
		{
			const cs_arm_op &op(detail.operands[i]);
			if (op.type == ARM_OP_MEM)
			{
				if (op.mem.base == ARM_REG_PC)
				{
					rip_va = va + m_insn->size + op.mem.disp + 4;
					return true;
				}
			}
		}
		return false;
	}
	virtual int Print(DataStream_t &is, ADDR64 base, ADDR va, IOutpADDR2Name *cb, ins_desc_t &desc)
	{
CHECK(base + va == 0x63f28c10)
STOP
		int insn_size(disassemble1(is, base + va, true));
		if (insn_size > 0)
		{
			int xctrl;
			desc.flowBreak = checkFlowBreak(xctrl);

			MyStrStream ss(cb->colorsEnabled());
			ss << m_insn->mnemonic;
			if (m_insn->op_str)
			{
				ss << '\t';
				filterOpString(m_insn->op_str, ss, xctrl ? adcui::COLOR_DASM_ADDRESS : adcui::COLOR_DASM_NUMBER);
			}
			std::string s;
			ss.flush(s);
			cb->print_code(nullptr, (const uint8_t *)s.c_str(), s.length());
			cb->print_bytes(m_insn->bytes, insn_size);

			ADDR rip_va;
			VALUE_t v;
			if (checkRIPaddressing(va, rip_va))
			{
				
				v.ui32 = rip_va;
				v.optyp = OPSZ_DWORD;
				cb->on_RIP_relative_addressing(v);
			}

			//ARM_Print
		}
		return insn_size;
	}
	virtual int Skip(DataStream_t &, ADDR64 base, ADDR va, ins_desc_t &desc)
	{
		return 0;
	}
	virtual int Generate(DataStream_t &, ADDR64 base, ADDR va, ins_desc_t &, IPCode_t &, const I_FrontDC &)
	{
		return 0;
	}
};

DECLARE_CODE_TYPE(CS_ARM_t, CAPSTONE_ARM);

////////////////////////////// CS_ARM_t
class CS_AARCH64_t : public CS_t
{
public:
	CS_AARCH64_t()
		: CS_t(CS_ARCH_ARM64, CS_MODE_ARM)
	{
	}
protected:
	virtual int Unassemble(DataStream_t &is, ADDR64 base, ADDR va, ins_desc_t &desc)
	{
		ins_t ins;
		int len(Unassemble(is, base, va, ins));
		desc = ins;
		return len;
	}
	int Unassemble(DataStream_t &is, ADDR64 base, ADDR va, ins_t &ins)
	{
		int insn_size(disassemble1(is, base + va, true));
		if (insn_size == 0)
			return 0;
		const cs_arm64 &detail(m_insn->detail->arm64);
		return -1;
	}
	virtual int Print(DataStream_t &is, ADDR64 base, ADDR va, IOutpADDR2Name *cb, ins_desc_t &desc)
	{
		return 0;//?ARM_Print(is, base, va, cb);
	}
	virtual int Skip(DataStream_t &, ADDR64 base, ADDR va, ins_desc_t &desc)
	{
		return 0;
	}
	virtual int Generate(DataStream_t &, ADDR64 base, ADDR va, ins_desc_t &, IPCode_t &, const I_FrontDC &)
	{
		return 0;
	}
};

DECLARE_CODE_TYPE(CS_AARCH64_t, CAPSTONE_AARCH64);


void DeclareCapstoneTypes(I_ModuleEx &mr)
{
	mr.DeclareCodeType(_PFX("CAPSTONE_X86_16"));
	mr.DeclareCodeType(_PFX("CAPSTONE_X86_32"));
	mr.DeclareCodeType(_PFX("CAPSTONE_X86_64"));

	mr.DeclareCodeType(_PFX("CAPSTONE_ARM"));
	mr.DeclareCodeType(_PFX("CAPSTONE_AARCH64"));
}




