#include "x86_IR.h"

#include <assert.h>
#include "shared/defs.h"
#include "shared/misc.h"
#include "shared/front.h"
#include "shared/action.h"
#include "front_IA.h"


void opr_t::Clear()
{
	memset(this, 0, sizeof(opr_t)); 
}

#define OPID_ANYREG	((uint8_t)-1)

int opr_t::MatchOp(TDef_t tdef, const FE_t& fe) const
{
	if (opc == OPC_IMM)
	{
		if (tdef.opc == OPC_IMM)
		{
			{//scalar
				//if ( estimateSize(i32) <= OPSZ(tdef) )
				if (rbase.m_ofs == 0)
				{
					if (opsize()/*rbase.m_sz*/ == (uint8_t)tdef.opsz)
					{
		//				opsize/*rbase.m_typ*/ &= ~0x0F;
			//			opsize/*rbase.m_typ*/ |= tdef.opsz & 0x0F;
						return 1;
					}
				}
				else
				{
					if (tdef.opid == rbase.m_ofs)
						return 1;
				}
				return 0;
			}
		}
		else if (tdef.isAddr())
		{
			/*?if (bRel)
				return 1;*/
		}
	}
	else if (opc & OPC_INDIRECT)//IsIndirect())
	{
		if (tdef.opc & OPC_INDIRECT)
		{
			if (tdef.opsz)
			{
				//if (rbase.m_typ & 0x0F)
				if (opsize())
					return (tdef.opsz == opsize()/*rbase.m_sz*/);
				return 1;
			}

			//rbase.m_typ &= ~0x0F;//LEA?
//?			optype = 0;
			return 1;
		}

		if (tdef.opc == OPC_IMM)
		{
			/*if (bOffs)//G_Parser.m_pCurInstr->action == ACTN_OFFS)//immidiate address
			{
				assert(0);
				//?assert(strlen(name));//m_p MLoc);
				assert(rbase.empty());//?
				assert(rindex.empty());//?
				if (tdef.opsz == OPSZ_DWORD)//?
					return 1;
			}*/
		}
		else if (opc == OPC_GLOBAL)
		{
			if (tdef.opc == OPC_IMM)
			{
				if (rbase.m_sz)
				{
					if (rbase.m_sz == tdef.opsz)
						return 1;
				}
				else
					return 1;
			}
		}
	}
	else if (opc == OPC_FPUREG)
	{
		if (tdef.opc == OPC_FPUREG)
		{
			if (tdef.opid == OPID_ANYREG)
				return 1;

			if (tdef.opid == rbase.m_ofs)
				return 1;

			return 0;
		}
	}
	else if (opc == OPC_SEGREG)
	{
		if (tdef.opc == OPC_SEGREG)
		{
			if (tdef.opid == OPID_ANYREG)
				return 1;

			if (tdef.opid == rbase.m_ofs)
				return 1;
		}
	}
	else if (opc == OPC_CPUREG)
	{
		if (tdef.opc == OPC_CPUREG)
		{
			if (tdef.opsz == rbase.m_sz)
			{
				if (tdef.opid == OPID_ANYREG)
					return 1;//any register

				return (tdef.opid == rbase.m_ofs);//some defined register
			}
		}
		/*		else if (OPC(tdef) == OPC_INDIRECT)
				{//try it as indirect ModR/M addressing mode (with Mod == 11)
				if (OPSZ(tdef) == (optyp & 0x0F))
				{
				assert(!OPID(tdef));
				return 1;
				}
				}*/
	}
	else if (opc == OPC_GLOBAL)
	{
		if (tdef.opc == OPC_INDIRECT)
		{
			if (!rbase.m_sz || rbase.m_sz == tdef.opsz)
				return 1;
		}
	}
	else if (isAddr())//opc & OPC_ADDRESS)
	{
		if (tdef.opc == OPC_IMM)
		{
			if (tdef.opsz == (unsigned)fe.ptr_size)
				return 1;
		}
	}

	if (opc == tdef.opc)
	{
		if (!rbase.m_sz || rbase.m_sz == tdef.opsz)
		{
			return 1;
		}
	}

	return 0;
}



/*bool opr_t::ShiftPtrLevel(int d)
{
	while (d != 0)
	{
		if (d < 0)//dereference
		{
			if (!OPTYP_IS_PTR(rbase.m_typ))
				return false;//error
			uint8_t p = ((rbase.m_typ >> 4) & 0x7) + 1;
			uint8_t s = rbase.m_sz;
			p--;
			if (p == 0)
			{
				rbase.m_typ = dptr;
				dptr = 0;
			}
			else
				rbase.m_typ = OPTYP_PTR | (((p - 1) & 0x7) << 4) | s;
			d++;
		}
		else//take address
		{
			assert(false);//fix later
			d--;
		}
	}

	return true;
}*/

static SREG_t toSREG(OPC_t opc)
{
	switch (opc & OPC_INDIRECT)
	{
	case OPC_CS: return R_CS;
	case OPC_DS: return R_DS;
	case OPC_ES: return R_ES;
	case OPC_SS: return R_SS;
	case OPC_FS: return R_FS;
	case OPC_GS: return R_GS;
	}
	return SREG_NULL;
}

reg_t opr_t::segreg() const
{
	reg_t r(opseg);
	if (r.empty())
	{
		SREG_t sr(toSREG((OPC_t)opc));
		if (sr == SREG_NULL)
			r.assign(OFS(R_DS), SIZ(R_DS));//DS is the default, except with BP indexing then SS is used
	}
	return r;
}

void ins_t::reset()
{
	memset(this, 0, sizeof(ins_t)); 
}




struct comparator { // simple comparison function
	bool operator()(const char *a, const char *b) const
	{ 
		return (strcmp(a, b) < 0);
	}
};

class Fast_t : private std::multimap<const char *, const templ_t *, comparator>
{
public:
	Fast_t(const FE_t& fe)
	{
		for (const templ_t *p(fe.ITS); p; p = p->next)
			insert(std::make_pair(p->name, p));
	}
	const templ_t *first(const char *mnemo) const
	{
		const_iterator i(find(mnemo));
		if (i != end())
			return i->second;
		return nullptr;
	}
};

static const templ_t * FindTemplate(const FE_t& fe, const char * mnemo, int pfx, const templ_t * pout)
{
	if (!pout)
	{
#if(0)//???
		static Fast_t fast(fe);
		pout = fast.first(mnemo);
#else
		pout = fe.ITS;
		assert(pout);
#endif
	}
	else
		pout = pout->next;

	while (pout)
	{
		int n(strcmp(mnemo, pout->name));
		if (n == 0)
			if (pout->m_pfx == pfx)
				return pout;

		pout = pout->next;
	}

	return nullptr;
}

static bool MatchForm(const ins_t& ins, const itf_entry_t *forms, const FE_t& fe)
{
	int i;
	for (i = 0; i < ins.ops_num; i++)
	{
		const opr_t &op(ins.ops[i]);
		const TDef_t &tdef((const TDef_t &)forms->ops[i]);
		if (!op.MatchOp(tdef, fe))
			break;
	}

	if (i == ins.ops_num)
		if (forms->ops[i] == 0)
			return true;//all were matched!

	return false;
}

static void adjustFromTemplate(uint32_t tdef, opr_t &self)//extract extra info for op from template
{
	if (!self.optype)
	{
		uint8_t t(OPTYP(tdef) & 0x0F);
		if (t)
		{
			assert(0);//self.opsize |= t;
		}
	}
	if (!(self.optype & 0xF0))//type
	{
		uint8_t t(OPTYP(tdef) & 0xF0);
		if (t)
			self.optype |= t;
	}

	/*uint8_t tx = OPTYPX(tdef);
	if (tx)
	{
		assert(0);
		assert(self.is_ptr());
		self.dptr = tx;
	}*/
}

static int MatchTemplate(const ins_t& ins, const templ_t * pout, const FE_t& fe/*, I_ Builder * pBld*/)
{
	if (ins.ops_num == 0)
	{//no ops or all ops are implicit
		if (pout->m_forms[0].ops[0] == 0)//is it the end of the table?
			return 0;
		return -1;
	}

	/*for (int i = 0; i < ins.ops_num; i++)
	{
		opr_t& op = ((opr_t&)ins.ops[i]);
		Adjust5(pout, op);
	}*/

	//find instruction form
	const itf_entry_t *pform = pout->m_forms;
	for (int iForm(0); pform->ops[0] != 0; iForm++, pform++)//while not end of table
	{
		if (MatchForm(ins, pform, fe))
		{
//			for (int i(0); i < ins.ops_num; i++)
	//			adjustFromTemplate((*pform)[i], ((ins_t&)ins).ops[i]);

			return iForm;//all were matched!
		}
	}

	return -1;//not found
}

static const templ_t * findTemplate(const FE_t& fe, const ins_t &ins, int &iForm)
{
	char token[NAMELENMAX];
	strncpy(token, ins.mnemo, NAMELENMAX);
	//_strupr(token);
	char *p(token);
	for (; *p; ++p) *p = (char)std::toupper(*p);

	const templ_t* pout(nullptr);

	while ((pout = FindTemplate(fe, token, ins.prefix, pout)) != nullptr)
	{
		iForm = MatchTemplate(ins, pout, fe);
		if (iForm >= 0)
			return pout;
	}

	return nullptr;
}

/*int8_t Id2Offset_OPC_CPUREG( uint8_t sz, uint8_t id)
{
	int s = sz&OPSZ_MASK;

	assert(id > 0);
	id -= 1;

	if (sz == OPSZ_BYTE)
		return ((id&3) << 2) + (((id>>2)&3)!=0);

	return (id&OPSZ_MASK) << 2;
}*/


static int AdjustIndirect(int opi, const ins_t &ins, const FE_t& fe, IPCode_t &pcode)
{
	opr_t& o = (opr_t&)ins.ops[opi];

	if (!o.is_indirect() || (o.opc & 0xF))
		return 0;

/*	bool bStackVar = false;
	if (GetRBase() == OFS(R_ESP))
		bStackVar = true;
	else if (GetRIndex() == OFS(R_ESP))
	{
		XChg(rbase, rindex);
		bStackVar = true;
	}
	else 
	{
		if (rbase == OFS(R_EBP))
			bStackVar = true;
		else if (rindex == OFS(R_EBP))
		{
			XChg(rbase, rindex);
			bStackVar = true;
		}
	}*/

	int N[5] = {0,0,0,0,0};//M,Rb,Ri,S,D

	reg_t __opseg;
	if (o.is_indirect())
		__opseg = !ins.m_opseg.empty() ? ins.m_opseg : reg_t(OFS(R_DS), OPSZ_WORD);

	uint8_t __opc(0);
	/*if (strlen(o.name) > 0)
	{
		assert(0);
		N[0] = 1;
		__opc = OPC_ADDRESS|OPC_GLOBAL;
	}*/

	if (!o.rbase.empty())
	{
		//assert(o.opc == OPC_CPUREG);
		if (o.is_indirect())
			if (o.rbase.m_ofs == fe.stack_ptr->ofs || o.rbase.m_ofs == fe.stackb_ptr->ofs)//esp or ebp base use ss
				__opseg = !ins.m_opseg.empty() ? ins.m_opseg : reg_t(OFS(R_SS), OPSZ_WORD);

/*		if ( GetRIndex() == GetRBase() )
		{
			assert(false);
//			scale++;
//			rbase = 0;
		}*/

		if (N[0])
		{
			assert(o.rindex.empty());
			XChg(o.rbase, o.rindex);
		}
		else
		{
			N[1] = 1;
			__opc = OPC_CPUREG;
		}
	}

	if (!o.rindex.empty())
	{
		N[2] = 1;
		if (!N[0])
			__opc = OPC_CPUREG;

		if (o.scale > 1)
		{
			N[3] = 1;
		}
	}
	else
	{
		assert(!o.scale);
	}

	if (o.lval.i32)
	{
		N[4] = 1;
	}

	int _count = 0;
	for (int i = 0; i < 4; i++)
		if (N[i]) _count++;
//	assert(_count);

	if (_count < 2)
	{
		o.opc = (__opseg.toId() << 4) | __opc;
		//o.opc |= __opc;
//		if (bStackVar)
//		{
		//	o.rbase.m_ofs = o.rbase.m_ofs;//o.r.CopyFrom(o.rbase);
//			optyp = addrsz;
		//	o.rbase.Clear();
//		}
		assert(o.rindex.empty());
		return 0;//ok:no need to make desection list
	}

	bool bLast(false);
	HOPND pOpT(nullptr);
	HINS pINS(nullptr);

	while (_count)
	{
		if (!pOpT)
		{
			pINS = pcode.newins(ACTN_MOV, INS_pvt_t(), true);//push_front
			pOpT = pcode.newop(pINS, OPC_AUXREG, fe.ptr_size, OFS(R_T));
		}

		HOPND pOP;
		if (!bLast || (_count < 2))
		{
			if (N[1])//Rb
			{
				pOP = pcode.newop(pINS, OPC_CPUREG, ins.addrSize, o.rbase.m_ofs);
				o.rbase.clear();
				N[1] = 0;

/*				if (bStackVar && N[4])
				{
					pOp->SetScalar(i32);
					N[4] = 0;
					i32 = 0;
				}*/
			}
			else if (N[2])//Ri
			{
				pOP = pcode.newop(pINS, OPC_CPUREG, ins.addrSize, o.rindex.m_ofs);
				o.rindex.clear();
				N[2] = 0;
				if (N[3])
					pcode.setAction(pINS, ACTN_MUL);
			}
			else if (N[3])//S
			{
				pOP = pcode.newop(pINS, OPC_IMM, MAKETYP_SINT(fe.ptr_size), 0);
				pcode.setScalar(pOP, o.scale);
				o.scale = 0;
				N[3] = 0;
			}
			else if (N[4])//D
			{
				pOP = pcode.newop(pINS, OPC_IMM, MAKETYP_SINT(fe.ptr_size), 0);
				pcode.setScalar(pOP, o.lval.i32);
				o.lval.i32 = 0;
				N[4] = 0;
			}
			else
				ASSERT0;

			_count--;
		}
		else
		{
			pOP = pcode.newop(pINS, OPC_AUXREG, fe.ptr_size, OFS(R_T));
		}

		if (bLast)
		{
			if (pcode.getAction(pINS) == ACTN_MOV)
				pcode.setAction(pINS, ACTN_ADD);
			if (pOpT == pcode.lastOp(pINS))
				pcode.setOpType(pOpT, MAKETYP_PTR(fe.ptr_size));
		}

		bLast = !bLast;
		if (!bLast)
			pOpT = 0;
	}

	o.opc &= ~0xf;
	o.opc |= OPC_AUXREG;
	o.rbase.m_ofs = OFS(R_T);
	return 1;
}

static HOPND CreateOp(const ins_t &ins, const FE_t&, int opi, HINS m_ri, IPCode_t &pcode)
{
	const opr_t& o = ins.ops[opi];

	HOPND pOP;

//?	assert(o.rbase.empty() && o.rindex.empty() && !o.scale);

	if (o.opc & OPC_INDIRECT)// || o.opc == OPC_GLOBAL)
	{
		if (o.opseg.empty())
		{
	//?		((opr_t &)o).opseg.assign(OFS(R_DS), OPSZ_WORD);
		}

		if (o.opc & 0xF)
		{
			if (o.vaRip)
			{
				pOP = pcode.newop(m_ri, o.opc & 0xF0, o.optype, 0);//convert to [addr]
				pcode.setScalar(pOP, o.vaRip);
			}
			else
			{
				pOP = pcode.newop(m_ri, o.opc, o.optype, o.rbase.m_ofs);
				pcode.setScalar(pOP, o.lval.i32);
			}
		}
		else
		{
			pOP = pcode.newop(m_ri, o.opc, o.optype, 0);
			pcode.setScalar(pOP, o.lval.i32);
		}

		assert(o.rindex.empty());
		//?pOP->SetOpSeg(o.opseg.m_ofs);
		pcode.setDPtr(pOP, o.dptr);
	}
	else if (o.opc == (OPC_ADDRESS|OPC_GLOBAL))
	{
		if (ins.isControlTransfer() && o.is_immidiate())
			return pcode.newopp(m_ri, OPC_ADDRESS|OPC_GLOBAL, ins.addrSize, o.lval.ui64);

		pOP = pcode.newop(m_ri, o.opc, o.rbase.m_typ, o.rbase.m_ofs);
		pcode.setDPtr(pOP, o.dptr);
		pcode.setScalar(pOP, o.lval.i32);
	}
	else if (o.opc != OPC_IMM)
	{
		uint8_t offs = o.rbase.m_ofs;
		uint8_t optyp(o.rbase.m_sz);
		optyp |= o.optype & 0xF0;
		if (o.vaRip)
		{
			pOP = pcode.newop(m_ri, OPC_ADDRESS|OPC_GLOBAL, optyp, 0);
			pcode.setDPtr(pOP, o.dptr);
			pcode.setScalar(pOP, o.vaRip);
		}
		else
		{
			pOP = pcode.newop(m_ri, o.opc, optyp, offs);
			pcode.setDPtr(pOP, o.dptr);
			pcode.setScalar(pOP, o.lval.i32);
		}
	}
	else
	{
		if (ins.isControlTransfer() && o.is_immidiate())
			return pcode.newopp(m_ri, OPC_ADDRESS|OPC_GLOBAL, ins.addrSize, o.lval.ui64);

		pOP = pcode.newop(m_ri, o.opc, o.optype, 0);
		pcode.setDPtr(pOP, o.dptr);
		pcode.setScalar(pOP, o.lval.i32);
	}

	return pOP;
}

static int makeop(const templ_t &templ, int iForm, const ins_t &ins, const FE_t& fe, int &n, HINS m_ri, IPCode_t &pcode)//terminal op
{
	assert(m_ri);

	const ito_t &it = templ.m_it[n];

	if (!it.is_null())
	{
		int imm(0);

		HOPND pOP;
		if (it.cls == -1)//operand ref
		{
			assert(it.id > 0 && it.id <= INS_OPS_MAX);
//			assert(m_pi);

			int i = it.id-1;
			AdjustIndirect(i, ins, fe, pcode);

			opr_t& op = (opr_t&)ins.ops[i];

			adjustFromTemplate(templ.m_forms[iForm].ops[i], op);

			if (templ.m_it[n-1].action == ACTN_OFFS)
			{
				assert(op.opc & OPC_INDIRECT);

				op.opc &= 0xF;
				op.rbase.m_typ = ins.addrSize;
				if (op.vaRip == 0)
				{
					imm = op.lval.i32;
					op.lval.i32 = 0;
				}
				else
				{
					STOP
				}
			}
			else if (templ.m_it[n-1].action == ACTN_INDIR)
			{
				assert(op.rbase.m_sz != 0);
				assert(OPTYP_IS_PTR(op.rbase.m_typ));

				if (op.isAddr())// & OPC_ADDRESS)
				{
					op.opc &= 0xF;
					op.rbase.m_typ = op.dptr;
					op.dptr = 0;
				}
				else
				{
					ASSERT0;
					/*?assert(!(op.opc & OPC_INDIRECT));
					assert(OPTYP_IS_PTR(op.rbase.m_typ));
					op.opc |= OPC_INDIRECT;
					op.ShiftPtrLevel(-1);*/
				}
			}

			pOP = CreateOp(ins, fe, i, m_ri, pcode);

			if (imm)
			{
				uint8_t t(OPSIZE(pcode.opType(pOP)));
				pOP = pcode.newop(m_ri, OPC_IMM, MAKETYP_SINT(t), 0);
				pcode.setScalar(pOP, imm);
				pcode.setAction(m_ri, ACTN_ADD);
			}
		}
		else if (it.cls == -2)//current VA
		{
			pOP = pcode.newop(m_ri, OPC_IMM, MAKETYP_UINT(ins.addrSize), 0);
			pcode.setScalar(pOP, ins.vaCur);
		}
		else if (it.cls == -3)//next VA
		{
			pOP = pcode.newop(m_ri, OPC_IMM, MAKETYP_UINT(ins.addrSize), 0);
			pcode.setScalar(pOP, ins.vaCur + ins.length);
		}
		else
		{
			int c, t, o(0), d(0);
			
			c = it.cls;
			if (c == OPC_IMM)
				d = it.id;

			int it_type(it.type);
			if (it_type < 0)
			{
				int id = -it_type - 1;
				if (id <= INS_OPS_MAX)//SIZEOF
				{
					it_type = ins.ops[id-1].opsize();//rbase.m_sz;
				}
				else
				{
					id -= INS_OPS_MAX;
					if (id <= INS_OPS_MAX)//TYPEOF
					{
						it_type = ins.ops[id-1].optype;//rbase.m_typ;
					}
					else
					{
						id -= INS_OPS_MAX;
						assert(id == 1);
						assert(OPC(it.id) == OPC_IMM);
						it_type = OPTYP(it.id);
						o = OPID(it.id);
						//assert(it_type == OPTYP_REAL64);
						d = 0;//special constant - value determined by offset (o)
					}
				}
			}

			const STORAGE_t& ss = fe.storage(SSID_t(c & 0xF));

			t = it_type;
			if (c == OPC_IMM)
			{
				//d = it.id;
			}
			else
			{
				if (ss.isDiscrete())
					o = it.id;
				else
					o = it.id;//flags
			}

			if (it.disp > 0)
			{
				d = it.disp * it.dispmul;
			}
			else if (it.disp < 0)
			{
				int opnum = -it.disp;
				const opr_t& op = ins.ops[opnum-1];
				int disp = op.rbase.m_ofs-1;
				d = disp * it.dispmul;
			}
			
			//assert(c != OPC_CPUREG || o == 0);
			pOP = pcode.newop(m_ri, c, t, o);
			pcode.setScalar(pOP, d);
			
			if (!isempty(it.name))
			{
				pcode.setOpName(pOP, it.name);
				/*if (templ.m_it[n - 1].action == ACTN_CALL)
				{
					STOP
					//?pOP->SetOpSeg(OPSEG_CODE);
				}*/
			}
		}
	}

	n++;
	return 1;
}

static int makeins(const templ_t &templ, int iForm, const ins_t &ins, const FE_t& fe, int &iTerm, HINS m_ri, IPCode_t &pcode)//instruction
{
	const ito_t &aTerm(templ.m_it[iTerm]);

	if (!aTerm.action)//op
	{
		if (pcode.insCount() == 0 || aTerm.is_null())//!aTerm.cls)//nop!//cls can be OPC_IMM=0
		{
			iTerm++;
			return 1;
		}
		return makeop(templ, iForm, ins, fe, iTerm, m_ri, pcode);
	}

	iTerm++;

	if (aTerm.action == ACTN_COMMA)
	{
		makeins(templ, iForm, ins, fe, iTerm, m_ri, pcode);//left
		m_ri = 0;
		makeins(templ, iForm, ins, fe, iTerm, m_ri, pcode);//right
		return 1;
	}
	
	if (aTerm.action == ACTN_COMMA0)
	{
		makeins(templ, iForm, ins, fe, iTerm, m_ri, pcode);//left
		makeins(templ, iForm, ins, fe, iTerm, m_ri, pcode);//right
		return 1;
	}

	if (!m_ri)
	{
		INS_pvt_t aPvt;
		aPvt.stackdiff = (short)((aTerm.stackdiff >> 16) & 0xFFFF);
		aPvt.cpuflags = (aTerm.flags >> 16) & 0xFFFF;
		aPvt.cpuflags_tested = 0;
		aPvt.fpuflags = aTerm.flags & 0xFFFF;
		aPvt.fpudiff = (short)(aTerm.stackdiff & 0xFFFF);

		m_ri = pcode.newins(ACTN_NULL, aPvt, false);//push_back

		if (ISINTRINSIC(aTerm.action))
		{
			pcode.newop(m_ri);
		}
	}
/*	else
	{
		if (m_ri->Action() != ACTN_MOV)
			if (m_ri->Action() != ACTN_CHECK)
				if (m_ri->Action() != ACTN_GOTOIF)
					if (m_ri->Action() != ACTN_CALL)
						assert(false);
	}*/

	Action_t actn2(ACTN_NULL);
	if (aTerm.action != ACTN_OFFS)
		if (aTerm.action != ACTN_INDIR)
		{
			Action_t act(pcode.getAction(m_ri));
			if (act == ACTN_NULL || act == ACTN_MOV || act == ACTN_CHECK || act == ACTN_GOTOIF)
				pcode.setAction(m_ri, aTerm.action);
			else
				actn2 = aTerm.action;
		}

	makeins(templ, iForm, ins, fe, iTerm, m_ri, pcode);
	if (ISBINARY(aTerm.action) || aTerm.action == ACTN_RET)
		makeins(templ, iForm, ins, fe, iTerm, m_ri, pcode);//right

	if (actn2 == ACTN_SIGNEXT)
	{
		HOPND op2(pcode.lastOp(m_ri));//back
		if (pcode.opClass(op2) == OPC_IMM)
		{
			HOPND op1(pcode.firstOp(m_ri));//front
			uint8_t opsz1(OPSIZE(pcode.opType(op1)));
			uint8_t opsz2(OPSIZE(pcode.opType(op2)));
			if (opsz1 > opsz2)
			{
				value_t v;
				v.i32 = pcode.getScalar(op2);
				if (opsz2 == OPSZ_BYTE)
					pcode.setScalar(op2, v.i8);
				else if (opsz2 == OPSZ_WORD)
					pcode.setScalar(op2, v.i16);
				else if (opsz2 == OPSZ_DWORD)
					pcode.setScalar(op2, v.i32);
				else
					ASSERT0;
				pcode.setOpType(op2, MAKETYP_UINT(opsz1));
			}
		}
	}

	return 1;
}

bool PCodeCreate(const ins_t &ins, const FE_t &fe, IPCode_t &pcode)
{
	int iForm;
	const templ_t *templ(findTemplate(fe, ins, iForm));
	if (!templ)
		return false;

	int iTerm(0);
	makeins(*templ, iForm, ins, fe, iTerm, nullptr, pcode);
	return true;
}



