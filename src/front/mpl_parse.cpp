
#include "stdafx.h"
#include <stdlib.h>
#include <assert.h>
#include <list>
#include <algorithm>

#include "../dc/globals.h"
#include "../dc/instr.h"
#include "../dc/misc.h"
#include "parsex.h"
#include "misc.h"
#include "parse.h"

//using namespace std;


int ParseMPL_t::ParseOperandOut(const char **str, ins_t &ins)
{
	VALUE_t v;

	opr_t& pi = ins.ops[ins.ops_num];
	pi.Clear();

	if (__isNext(str, ":"))
	{
		if (!__isSymbol(str, pi.name))
			return 0;

		if (__isNext(str, "+"))
		{
			if (!__isNumber(str, v, 2))//hex without C-style prefix
				return 0;
			pi.i32 = v.i32;
		}

		pi.opc = OPC_ADDRESS|OPC_GLOBAL;
//		pi.optyp = OPTYP_PTR32;
		return 1;
	}

	if (__isNext(str, "@"))
	{
		if (!__isSymbol(str, pi.name))
			return 0;

		pi.opc = OPC_GLOBAL;
		pi.optyp = OPTYP_PTR32;
		return 1;
	}

	if (!__isNumber(str, v, 2))//hex without C-style prefix
		return 0;

	pi.opc = OPC_IMM;
	pi.i32 += v.i32;//immidiate/scalar displacement
	if (!pi.optyp)
		pi.optyp = v.typ&0xF;

	return 1;
}


int ParseMPL_t::ParseData(const char **str, char *token)
{
	static const char *StrDataID[] = {
		"BCONST", "WCONST", "LCONST", 0};

	const char *str0 = *str;

	int index = __isNextEx(str, StrDataID);
	if (index == -1)
	{
		char token[32];
		if (!__isSymbol(str, token))
			return 0;
		if (strncmp(token, "BLANK", 5) != 0)
			return 0;

		GDC.SetCurSeg(SEG_DATA);

		int sz = atoi(&token[5]);
		assert(sz);
		if (GetCurName())
		{
			GDC.OpenData(GetCurName());
//			TYPE_t T;
			if (sz == OPSZ_BYTE || sz == OPSZ_WORD || sz == OPSZ_DWORD || sz == OPSZ_QWORD)
//				T.Setup(sz);
				//GDC.getCurData()->SetType(sz, 0);
				GDC.AddData(sz, 0, 0);
			else
//				T.Setup(OPSZ_BYTE, 0, 0, sz);
				//GDC.getCurData()->SetType(OPSZ_BYTE, sz);
				GDC.AddData(OPSZ_BYTE, sz, 0);
			
			SetCurName(0);
			GDC.SetCurAddr(GDC.GetCurAddr() + sz);
		}
		return 1;
	}

	//constants
	GDC.SetCurSeg(SEG_CONST);

	int sz = 1 << index;
	if (GetCurName())
	{
		GDC.OpenData(GetCurName());
		SetCurName(0);
		
//		CurData()->set_size(sz);
//		CurData()->set_array(0);
	}
	
	VALUE_t v;
	if (!__isNumber(str, v, 1))
		getMsgMan()->ERRORMSG("!!!");

//	GDC.getCurData()->AppendIField(sz, &v);
	GDC.AddData(sz, 0, &v);
	return 1;
}

void ParseMPL_t::ParseLine(const char **str)//comment allready stripped
{
	if (endofline(str))
		return;

	char token[80];
	token[0] = 0;
	char const *str0 = *str;

	if ( __isNext(str, ":") )
	{
		if (!__isSymbol(str, token))
			getMsgMan()->ERRORMSG("Label expected");
		SetCurName(token);
		return;
	}

	if (ParseData(str, token))
		return;

	ins_t ins;
	out2_t * pout = __isInstruction2(&str0, ins);
	if (pout)
	{

//CHECK(m_nInstrNo == 225)
//STOP

		*str = str0;
		if (!GDC.getCurFunc())
		{
			GDC.SetCurSeg(SEG_CODE);
			GDC.OpenFunc(GetCurName(), false);//near or far?
			SetCurName(0);
		}

		GDC.CreateInstr(ins, pout);
		return;
	}

	if (!endofline(str))
		getMsgMan()->ERRORMSG("Bad staff: %s", str);
}


out2_t * ParseMPL_t::__isInstruction2(const char **str, ins_t& ins)
{assert(false);/*
	const char *_str = *str;

	assert(Code.m_out == 0);
	
	char token[32];
	if (ExtractToken(&_str, token))//mnemonic assumed
	{
		_strupr(token);

		Code.m_ins->ops_num = 0;
		
		while (Code.m_out = GDC.FindTemplate(Code.m_out, token, 0))
		{
			//parse out all operands
			while (!endofline(&_str))
			{
//				if ( Code.opsnum && (!__isNext(&_str, ",")) )//not first
//					break;//where is a comma?

				if (!ParseOperandOut(&_str, *Code.m_ins))
					break;
				
//				Adjust1(Code.m_pi[Code.opsnum], pout);
				Code.m_ins->ops_num++;
			}

			if (Code.m_ins->ops_num == 0)
			{//no ops or all ops are implicit
				if (!Code.CheckCell(0))
					return 1;
				continue;//not empty forms table - continue
			}

			//find instruction form
			for (int j = 0; Code.CheckCell(j); j++)
			{
				if (Code.MatchForm(j))
				{
					Code.Adjust2(j);
					return 1;//all were matched!
				}
			}
		}
	}
*/
	return 0;//undefined instruction
}

/*
int ParseMPL_t::Adjust1(opr_t &pi, out2_t *pout)
{
	if (pout->m_it[0].action == ACTN_GOTO)
	{
		pi.opc = OPC_ADDRESS|OPC_GLOBAL;
		pi.optyp = OPTYP_PTR32;//|GDC.PtrSize();
	}
	return 1;
}*/

