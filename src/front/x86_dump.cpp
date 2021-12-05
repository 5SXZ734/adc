#include "x86_dump.h"

#include <assert.h>
#include <bitset>
#include <locale>
#include "shared/defs.h"
//#include "shared/misc.h"
#include "shared/front.h"
#include "front/front_IA.h"
#include "x86_IR.h"


/*static IDStr_t OpSizeStr[] = {
	{OPSZ_BYTE,		"BYTE"}, 
	{OPSZ_WORD,		"WORD"}, 
	{OPSZ_DWORD,	"DWORD"}, 
	{OPSZ_FWORD,	"FWORD"},
	{OPSZ_QWORD,	"QWORD"}, 
	{OPSZ_TWORD,	"TWORD"},
	{OPSZ_NULL,		"?"}
};*/

/*char * STR_OPSIZE(int opsz)
{
	static char str[6];
	strncpy(str, OpSizeStr->GetStr(opsz), sizeof(str));
	return str;
}*/

bool Outp_t::enable_names(const IOutpADDR2Name *p)
{
	//bool do_names_old = do_names != nullptr;
	do_names = p;
	return do_names != nullptr;
}

//
//  Lowercases string
//
std::string &str_tolower(std::string& s)
{
	for (char& c : s) c = (char)std::tolower(c);
	//std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)std::tolower(c); });
    return s;
}

//
// Uppercases string
//
std::string &str_toupper(std::string& s)
{
	for (char& c : s) c = (char)std::toupper(c);
	//std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)std::toupper(c); });
    return s;
}

void Outp_t::out_cpureg(int ofs, int sz)
{
	std::string s(ToRegName(OPC_CPUREG, ofs, sz));
	mos << str_tolower(s);
/*	if (sz == 4)
		mos << 'e';
	if (sz == 1)
		mos << "acdbacdb"[r] << "llllhhhh"[r];
	else
		mos << "acdbsbsd"[r] << "xxxxppii"[r];*/
}

void Outp_t::out_simdreg(int ofs, int sz)
{
	assert(sz < CHAR_BIT * sizeof(int));
	int realSz(1 << sz);
	const char *pc(ToRegName(OPC_SIMD, ofs, realSz));
	if (!pc)
		pc = "?";
	std::string s(pc);
	mos << str_tolower(s);
}

void Outp_t::out_segreg(int ofs, int sz)
{
	std::string s(ToRegName(OPC_SEGREG, ofs, sz));
	mos << str_tolower(s);
}

/*std::string Outp_t::addr2obj(ADDR a)
{
	//int loc_sz;
	//std::string s;
	IOutpADDR2Name::out_t dot;
	if (do_names)
		do_names->decode(a, false, dot);
	return dot.name;
}*/

/////////////////////////////////////////////// o p _ t

bool Outp_t::out_dist(ADDR from, ADDR to)
{
	ADDR d;
	if (to > from)
		d = to - from;
	else
		d = from - to;
	if (d < 0x80)
	{
		mos << "short";
		return true;
	}
	return false;
}

void Outp_t::out_op(const ins_t& ins, int i)
{
CHECK(ins.vaCur == 0x100225f)
STOP

	const opr_t& op(ins.ops[i]);

	IOutpADDR2Name::out_t dot;
	if (op.is_indirect())//indir
	{
		bool bRbase(!op.rbase.empty());
		bool bRindex(!op.rindex.empty());
		bool bLoc(false);
		if (do_names)
			if (ins.addrSize == ins.ptrSize())
			{
				ADDR va(op.vaRip != 0 ? op.vaRip : op.lval.ui32);
				bLoc = do_names->decode(va, false, dot) != 0;
				if (bLoc && op.vaRip != 0)
					bRbase = false;
			}

		if (do_size)
		{
			if (op.opsize() != dot.sz)
			{
				switch (op.opsize())
				{
				/*case OPSZ_DQWORD:
					mos << "dqword ptr "; break;*/
				case OPSZ_QWORD:
					mos << "qword ptr "; break;
				case OPSZ_DWORD:
					mos << "dword ptr "; break;
				case OPSZ_WORD:
					mos << "word ptr "; break;
				case OPSZ_BYTE:
					mos << "byte ptr "; break;
				default:
					mos << "? ptr ";
				}
			}
		}

		reg_t r(op.opseg);
		if (r.empty())
			if (ins.isCall() || ins.isJump())
				r = op.segreg();

		if (!r.empty())
		{
			out_segreg(r.m_ofs, r.m_sz);
			mos << ":";
		}

		if (bLoc)
			mos << dot.name;

		bool regaddr(bRbase || bRindex);
		bool brackets(regaddr || !bLoc);

		if (brackets)
			mos << "[";

		if (bRbase)
			out_cpureg(op.rbase.m_ofs, ins.addrSize);

		if (bRindex)
		{
			if (bRbase)
				mos << "+";
			out_cpureg(op.rindex.m_ofs, ins.addrSize);
		}

		if (op.scale)
		{
			mos << "*" << (int)op.scale;
		}

		if (!bLoc)
		{
			set_color(regaddr ? adcui::COLOR_DASM_NUMBER : adcui::COLOR_DASM_ADDRESS);
			if (ins.addrSize == OPSZ_WORD)
			{
				if (op.lval.ui16 != 0 || (!bRbase && !bRindex))
					out_imm(&op.lval.ui16, ins.addrSize,
					true, (bRbase || bRindex),
					16);
			}
			else if (ins.addrSize == OPSZ_DWORD)
			{
				if (op.lval.ui32 != 0 || (!bRbase && !bRindex))
					out_imm(&op.lval.ui32, ins.addrSize, true, (bRbase || bRindex),	16);
			}
			else if (ins.addrSize == OPSZ_QWORD)
			{
				if (op.lval.ui64 != 0 || (!bRbase && !bRindex))
					out_imm(&op.lval.ui64, ins.addrSize, true, (bRbase || bRindex), 16);
			}
			else
			{
				mos << "?";
			}
			set_color(adcui::COLOR_POP);
		}

		if (brackets)
			mos << "]";

		return;
	}

	if (op.is_immidiate())
	{
		bool bLoc(false);
		if (do_names && op.lval.i64 > 0)
		{
			ADDR64 va0(op.lval.ui64);
			if (va0 >= ins.imageBase)
			{
				ADDR64 va(va0 - ins.imageBase);
				if (va <= UINT_MAX)
					bLoc = do_names->decode(ADDR(va), ins.isCall(), dot);
			}
		}

		if (ins.isJump())
		{
			if (out_dist(ins.vaCur + ins.length, op.lval.ui32))
				mos << " ";
		}
		else if (bLoc && !ins.isCall())
			mos << "offset" << " ";

		if (bLoc)
		{
			if (dot.bThunk)
			{
				mos << "->";
				if (!dot.sreg.empty())
				{
					out_segreg(dot.sreg.m_ofs, dot.sreg.m_sz);
					mos << ":";
				}
			}
			mos << dot.name;
		}
		else
		{
			set_color(ins.isControlTransfer() ? adcui::COLOR_DASM_ADDRESS : adcui::COLOR_DASM_NUMBER);
			out_imm(&op.lval.ui8, op.optype, false, false, 16);
			set_color(adcui::COLOR_POP);
		}
		return;
	}
	
	if (op.opc == OPC_CPUREG)
	{
		out_cpureg(op.rbase.m_ofs, op.rbase.m_sz);
	}
	else if (op.opc == OPC_FPUREG)
	{
		mos << "st";
		if (op.rbase.m_ofs > 0)
			mos << "(" << (op.rbase.m_ofs>>3) << ")";//fpu_ofs2id
	}
	else if (op.opc == OPC_SEGREG)
	{
		out_segreg(op.rbase.m_ofs, op.rbase.m_sz);
		//mos << "cdesfg"[op.opid-1] << "s";
		//assert(0);
		//?mos << "ecsdfg"[op.r.m_ofs] << "s";
	}
	else if (op.opc == OPC_TESTREG)
	{
		mos << "T" << op.rbase.m_ofs;
	}
	else if (op.opc == OPC_DBGREG)
	{
		mos << "D" << op.rbase.m_ofs;
	}
	else if (op.opc == OPC_CTRLREG)
	{
		mos << "C" << op.rbase.m_ofs;
	}
	else if (op.opc == OPC_KREG)
	{
		const char *pc(ToRegName(OPC_KREG, op.rbase.m_ofs, op.rbase.m_sz));
		std::string s(pc ? pc : "?");
		mos << str_tolower(s);
	}
	else if (op.opc == OPC_SIMD)
	{
		out_simdreg(op.rbase.m_ofs, op.rbase.m_typ);
	}
	else
		mos << "?";
}

void Outp_t::out_prefix(const ins_t& ins)
{
	if (ins.prefix == 0)
		return;
	if (ins.prefix == PFX_REPE)
		mos << "repe ";
	else if (ins.prefix == PFX_REPNE)
		mos << "repne ";
	else
		mos << "pfx_?";
}

void Outp_t::out_code(const ins_t& ins)
{
//CHECK(std::string(ins.mnemo) == "call")
CHECK(ins.vaCur == 0x100642f)
STOP

	if (ins.is_invalid())
		return;

	out_prefix(ins);

	mos << ins.mnemo;

	if (ins.ops_num == 0)
		return;

	do_size = true;
	int j;
	for (j = 0; j < ins.ops_num; j++)
	{
		uint8_t opc = ins.ops[j].opc;
		switch (opc)
		{
		case OPC_CTRLREG:
		case OPC_DBGREG:
		case OPC_FPUREG:
		case OPC_CPUREG:
		case OPC_SEGREG:
		case OPC_TESTREG:
		case OPC_MMXREG:
		case OPC_SIMD:
			do_size = false;
		}
	}

	mos << "\t";//mnemo - opcodes separator



CHECK(ins.vaCur == 0x169cd19)
STOP

	for (j = 0; j < ins.ops_num; j++)
	{
		if (j > 0)
			mos << ",";
		mos << " ";
		out_op(ins, j);
	}
}

void Outp_t::out_addr(const ins_t& ins)
{
	mos.fill('0');
	mos.width(8);
	mos.flags(std::ios::hex|std::ios::uppercase);
	mos << ins.vaCur;
}

void Outp_t::output(const ins_t& ins)
{
	out_addr(ins);
	mos << " ";

	out_bytes(&ins.bytes[0], ins.length);
	int col = 2 * ins.length; 			// output column

	int dcol = 15 - col;
	if (dcol < 0)
		dcol = 1;

	while (dcol--)
		mos << " ";

	out_code(ins);
}


void x86_Print(const ins_t &ins, IOutpADDR2Name *cb)
{
	MyStrStream os(cb->colorsEnabled());

	Outp_t outp(os);
	outp.enable_names(cb);
	outp.out_code(ins);
	os << std::ends;

	std::string s;
	os.flush(s);
	cb->print_code(nullptr, (const uint8_t *)s.data(), s.size());

	cb->print_bytes(ins.bytes, ins.length);
}








