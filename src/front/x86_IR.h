
#ifndef _INSTR_H_INCLUDED_
#define _INSTR_H_INCLUDED_

#include "shared/action.h"
#include "shared/misc.h"
#include "shared/front.h"

#define INS_BYTES_MAX	16
#define INS_MNEMO_MAX	16
#define INS_OPS_MAX 4	//maximum of explicit & implicit ops in code instruction

struct ins_t;
class TypeCode_t;
struct OPND_t;
struct INS_t;
struct FE_t;
class IPCode_t;


struct itf_entry_t {
	unsigned ops[INS_OPS_MAX];
	unsigned flags;
};

struct ito_t
{
	Action_t	action;
	int			cls;
	int		type;
	int		id;
	int		disp;
	int		dispmul;
	int		flags;
	int		stackdiff;
	const char	*name;

	OPC_t	opc() const { return (OPC_t)(cls & 0xF); }
	int		indirect() const { return (cls & 0x10) ? 1 : 0; }
	int		is_address() const { return (cls & 0x20) ? 1 : 0; }
	bool	is_null() const { return !(cls || type); }
};

struct templ_t
{
public:
	const char* name;
	const itf_entry_t* m_forms;
	const ito_t* m_it;
	uint8_t	m_pfx;
	templ_t* next;

public:
	templ_t(const char* n, const itf_entry_t* f, const ito_t* o, int p, templ_t* nx)//int (*f)[INS_OPS_MAX]
		: name(n),
		m_forms(f),
		m_it(o),
		m_pfx(uint8_t(p)),
		next(nx)
	{
	}
};


struct TDef_t
{
	unsigned opc:8;
	unsigned opsz:4;
	unsigned opsz_h:4;
	unsigned optyp2:8;
	unsigned opid:8;
	bool isAddr() const { return (opc & OPC_ADDRESS) && ((opc & 0xF) != OPC_CPUSW); }
};


struct opr_t
{
	//uint8_t	bOffs;
	uint8_t	opc;
	/*union {
		reg_t	r;	//register
		struct {	//memory
		};

	};*/
	//reg_t	r;
	uint8_t	optype;
	uint8_t	dptr;	//dereferenced ptr
	reg_t	opseg;
	reg_t	rbase;
	reg_t	rindex;
	uint8_t	scale;//0,1,2,3	(^2)
	value_t lval;
	ADDR	vaRip;

	void	Clear();
	//void	checkname();
	//////////////////
	int		MatchOp(TDef_t, const FE_t&) const;
	//bool	ShiftPtrLevel(int d);
	bool	is_indirect() const { return (opc & OPC_INDIRECT) != 0; }
	bool	is_immidiate() const { return (opc & ~OPC_ADDRESS) == OPC_IMM; }
	reg_t	segreg() const;
	uint8_t	opsize() const { return optype & OPSZ_MASK; }
	bool	is_ptr() const { return OPTYP_IS_PTR(optype); }
	bool	isAddr() const { return (opc & OPC_ADDRESS) && ((opc & 0xF) != OPC_CPUSW); }
};

struct ins_t : public ins_desc_t
{
	bool	bIsLarge_;			//32 or 64 bit
	//uint8_t	u_opsize;			// just what it says ...

	reg_t	m_opseg;			//segment override prefix

	uint8_t	prefix;			//repeat prefix
	char	mnemo[INS_MNEMO_MAX];
	opr_t	ops[INS_OPS_MAX];
	uint8_t	ops_num;

	uint8_t	bytes[INS_BYTES_MAX];

	ins_t()
	{
		reset();
	}

	uint8_t ptrSize() const { return bIsLarge_ ? OPSZ_QWORD : OPSZ_DWORD; }

	void reset();
	/*signed char *	buff(int i) {
		return &(&ops[ops_num].lval.i8)[i];
	}*/

	void inc_ops_num()
	{
		ops_num++;
	}

	void setdisp2(void *p, uint8_t sz)
	{
		switch (sz)
		{
		default: break;
		case OPSZ_BYTE: ops[ops_num].lval.i64 = *(int8_t *)p; break;
		case OPSZ_WORD: ops[ops_num].lval.i64 = *(int16_t *)p; break;
		case OPSZ_DWORD: ops[ops_num].lval.i64 = *(int32_t *)p; break;
		case OPSZ_QWORD: ops[ops_num].lval.i64 = *(int64_t *)p; break;
		}
	}
	
	bool is_invalid() const { return (mnemo[0] == 0); }

	int checkRIPaddressing(VALUE_t& v, size_t from = 0) const
	{
		for (size_t i(from); i < ops_num; i++)
		{
			if (ops[i].vaRip != 0)
			{
				v.ui64 = imageBase + ops[i].vaRip;
				v.optyp = rip_target(ops[i].vaRip);
				return int(i);
			}
		}
		return -1;
	}
private:
	uint8_t rip_target(ADDR vaRip) const
	{
		if (bIsLarge_)
		{
			ADDR64 ui64 = imageBase + (ADDR64)vaRip;
			if (ui64 <= imageBase)
				return 0;
			ADDR64 vaDif(ui64 - imageBase);
			if (vaDif > 0xFFFFFFFF)
				return 0;
			return OPTYP_PTR64;
		}
		ADDR ui32 = (ADDR)imageBase + vaRip;
		return OPTYP_PTR32;
	}
};


enum {
	OPND__NULL,	//not used
	OPND_1,		//first operand (at index 0)
	OPND_2,		//first operand (at index 1)
	OPND_3,		//...
	OPND_4,
	OPND_5,
};

#define IT_FRM(arg)		static itf_entry_t itf_##arg[] = {
#define IT_FRME			{{0}}};
#define IT_BEG(arg)		IT_FRME static ito_t ito_##arg[] = {
#define IT_END(arg)		}; static templ2_t it_##arg(#arg, &itf_##arg[0], ito_##arg);

#define IT_FRM2(arg,n)	static itf_entry_t itf_##arg##n[] = {
#define IT_BEG2(arg,n)	IT_FRME static ito_t ito_##arg##n[] = {
#define IT_END2(arg,n)	}; static templ2_t it_##arg##n(#arg, &itf_##arg##n[0], ito_##arg##n);

#define IT_BEG3(arg)	static ito_t ito_##arg[] = {
#define IT_END3(arg,p)	}; static templ2_t it_##arg(#arg, &itf_##p[0], ito_##arg);

#define IT_END6(a,b,c)	static templ2_t it_##a(#a, &itf_##b[0], ito_##c);
#define IT_END7(a,n,p)	}; static templ2_t it_##a##n(#a, &itf_##a##n[0], ito_##a##n, p);


//                              action------class---type-imm-?-?---fstack-pstack-?
#define ACTN4(a, f, s)			{a,			0,		-1, 0, 0, 0,    f, s, nullptr},	//action with fstack/pstack change value
#define ACTN1(a)				{a,			0,		-1, 0, 0, 0,    0, 0, nullptr},	//just an action
#define OPND3(c, t, i)			{ACTN_NULL, c,		t,	i, 0, 0,    0, 0, nullptr},
#define OPND4(c, t, i, n)		{ACTN_NULL, c,		t,	i, 0, 0,    0, 0, n},
#define OPND1(i)				{ACTN_NULL, -1,		0,	i, 0, 0,    0, 0, nullptr},	//instruction operand reference (at index `i)
#define OPND0					{ACTN_NULL, 0,		0,	0, 0, 0,    0, 0, nullptr},	//instruction operand reference (at index 0)
#define OPND_VA					{ACTN_NULL, -2,		0,	0, 0, 0,    0, 0, nullptr},	//operand picks a current VA
#define OPND_NVA				{ACTN_NULL, -3,		0,	0, 0, 0,    0, 0, nullptr},	//operand picks the next VA


#define SIZEOF(a)	(-1-a)






bool PCodeCreate(const ins_t &, const FE_t &, IPCode_t &);


#endif//_INSTR_H_INCLUDED_
