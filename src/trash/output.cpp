#include <strstream>

#include "shared/defs.h"
#include "shared/link.h"
#include "shared/heap.h"
#include "shared/misc.h"
#include "shared/front.h"
#include "dc/globals.h"
#include "dc/obj.h"
#include "dc/data.h"
#include "dc/func.h"
#include "dc/dc.h"
#include "dc/struc.h"
#include "dc/path.h"
#include "dc/op.h"
#include "dc/debug.h"
#include "dc/file.h"
#include "dc/arglist.h"
#include "out.h"
#include "dump.h"
#include "back.h"


/*#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif*/


const bool G_bStyleIndir = 0;
static DisplayUnfold_t * g_pDI = NULL;


std::ostream &TAB(std::ostream &os)
{
	//if (1)
	if (!g_pDI || g_pDI->TestOpt(DUMP_TABS))
	{
		os << '\t';
	}
	else
	{
		do {
			os  << " ";
		} while (os.tellp() & TABMASK);
	}

	return os;
}

std::ostream& TABS(std::ostream& os, int nNum)
{
	if (nNum > 0)
		while (nNum--)
			os << TAB;
	return os;
}

OMANIP_int TABS(int nNum)
{
	return OMANIP_int(TABS, nNum);
}

void DisplayUnfold_t::OutputColor(std::ostream &os, int nColorId)
{
	if (!TestOpt(DUMP_COLORS))
		return;

	int comment = m_nComment;
	if (nColorId == COLOR_COMMENT)
		comment++;
	else if (nColorId == 0) 
	{
		if (comment > 0)
			comment--;
	}
	else if (m_nComment)
		comment++;

	if (m_nComment == 0 || comment == 0)
	if (TestOpt(DUMP_ASM|DUMP_ASMRAW) != (DUMP_ASM|DUMP_ASMRAW))
	{
		if (!m_hi.IsLocked())
			if (os.tellp() <= m_hi.pos)
				m_hi.pos += 2;
		os.put((char)0xff);
		os.put((char)nColorId);
	}

	m_nComment = comment;
}

std::ostream & COLOR(std::ostream &os, int nColorId)
{
	if (g_pDI)
		g_pDI->OutputColor( os, nColorId );
	return os;
}

OMANIP_int COLOR(int nColorId)
{
	return OMANIP_int(COLOR, nColorId);
}

void DisplayUnfold_t::OutputFont(std::ostream &os, int nFontId)
{
	if (!TestOpt(DUMP_FONTS))
		return;

	if (TestOpt(DUMP_ASM|DUMP_ASMRAW) == (DUMP_ASM|DUMP_ASMRAW))
		return;

	if (!m_hi.IsLocked())
		if (os.tellp() <= m_hi.pos)
			m_hi.pos += 2;

	os.put((char)0xfe);
	os.put((char)nFontId);
}

std::ostream &FONT(std::ostream &os, int nFontId)
{
	if (g_pDI)
		g_pDI->OutputFont(os, nFontId);

	return os;
}

OMANIP_int FONT(int nFontId)
{
	return OMANIP_int(FONT, nFontId);
}

Op_t * DisplayUnfold_t::GetTopOp(Op_t * pSelf)
{
	Op_t *pOp = pSelf;
	while (1)
	{
		if (!pOp->IsCodeOp())
		{
			if (pOp->IsXOut())
				pOp = pOp->Root();
			return pOp;
		}
		else if (pOp->m_pRI)
		{
			if (IsOpRootEx(pOp))
				return pOp;
			if (!pOp->XOut())
				return pOp;
			if (pOp->XOut()->CheckCount(1) > 0)
				return 0;//!!?
			pOp = pOp->XOut()->Op();
		}
		else
			pOp = pOp->Root();
	}
	return pOp;
}



////////////////////////////
// O p _ t

void DisplayInfo_t::OutputExpression4(std::ostream &os, Op_t * pOp0)
{
	Op_t * pOp = pOp0;
	if (pOp->IsCall() && pOp->XOut()->CheckCount(1) == 0)
		pOp = pOp0->XOut()->Op();

	if (pOp && pOp->IsDataOp())
	{
		TYPE_t T;
		pOp->GetType(T);
		OutputTYPE0(os, &T, 1);
	}

	OutputExpression3(os, pOp0);
}

void DisplayInfo_t::OutputSimple(std::ostream &os, Op_t * pSelf)
{
	/*?	if (pSelf->IsEntry())
	{
		if (pSelf->IsPtr() && pSelf->IsThisPtr())// && !pSelf->IsRootEx())
		{
			os << _RESERVED("this");
			return;
		}
	}*/

	DisplayUnfold_t::OutputSimple( os, pSelf );
}

void DisplayInfo_t::OutputOpenBrace( std::ostream& os, Path_t * pPath )
{
	if (pPath)
	{
		int nIndent = 0;
		if (!pPath->IsRoot())
			nIndent = GetPathIndent(pPath);

		bool bDoWhile = CheckDoWhile(pPath) != 0;
//			if (bDoWhile)
//				nIndent--;

		os << TABS(nIndent);

		if (bDoWhile)
			os << _RESERVED("do") << " ";
		else if (pPath->m_nType == BLK_LOOPENDLESS)
			os << _RESERVED("while") << " (1) ";
	}

	os << "{";

	if (0)
	if ( pPath )
		os << "//{" << pPath->Name(false) << "}";
}

void DisplayInfo_t::OutputCloseBrace( std::ostream& os, Path_t * pPath )
{
	if (pPath)
	{
		if (!pPath->IsRoot())
			os << TABS(GetPathIndent(pPath));
	}
	
	os << "}";

	if (pPath)
	if (pPath->IsRoot())
	{
		//output some debug info for function block
		Func_t *pf = pPath->GetOwnerFunc();
		assert(pf);
		OutputClosingInfo(os, pf);
	}
}

int DisplayInfo_t::GetIndent(Op_t * pSelf)
{
	assert(pSelf->Path());

	Path_t * pPath = pSelf->Path();
	int nIndent = GetPathIndent(pPath);

	if (pPath->m_nType == BLK_JMPIF)
	if (pPath->GetLastOp() == pSelf)
	{
		Path_t *pPathTop = pPath->GetLogicsTop(1);//check first
		if (pPathTop)
		if (CheckDoWhile(pPathTop->Parent()))//m_nType == BLK_LOOPDOWHILE)
		if (pPathTop->IsLast())
			nIndent--;
	}

	return nIndent;
}

bool DisplayUnfold_t::CheckTopOp(Op_t * pSelf, Op_t *pOpTop)
{
CHECK(pSelf->No() == 46)
STOP

	Op_t *pOp = pSelf;
	while (1)
	{
		if (pOp == pOpTop)
			return true;

		if (!pSelf->IsCodeOp())
		{
			if (!pOp->IsXOut())
				break;
		}
		
		if (pOp->m_pRI)
		{
			if (IsOpRootEx(pOp))
			{
				//check if pOp & pOpTop are on the same logics branch
				Path_t *pPathTop = pOpTop->Path();
				if (pOp->Path() == pPathTop)
					break;
				pPathTop = pPathTop->GetLogicsTop(1);
				if (pPathTop)
					if (pPathTop->m_nType & BLK_LOGIC)
						if (pOpTop->IsLast())
							if (pOp->Path()->IsChildOf(pPathTop))
								return true;
				break;
			}
//			if (!pOp->XOut())
//				break;
			if (pOp->XOut()->CheckCount(1) != 0)
			{
				for (XRef_t *pXOut = pOp->XOut(); pXOut; pXOut = pXOut->Next())
					if (CheckTopOp(pXOut->Op(), pOpTop))
						return true;
				break;
			}
		}

		if (pOp->m_pRI)
		{
			if (!pOp->XOut())
				break;
			pOp = pOp->XOut()->Op();

		}
		else if (pOp->IsXIn())
		{
			pOp = pOp->Root();
		}
		else
		{
//			if (!pOp->IsXOut())
//			{
//				pOp = pOp->Root();
//				continue;
//			}	
			if (!pOp->XOut())
				break;
			pOp = pOp->XOut()->Op();
		}
	}

	return false;
}

bool DisplayUnfold_t::IsOpRootEx(Op_t * pSelf)
{
	if (!pSelf->m_pRI)
		return false;
	
	if ( IsOutputUnfold() )
		return true;

	if ( pSelf->IsRoot() )
		return true;

	if ( !pSelf->XOut() )
		return true;

	return false;
}

Op_t * DisplayInfo_t::GetThisPtr( FuncDef_t * pSelf )
{
	return pSelf->m_pThisPtr;
}

int DisplayInfo_t::CheckLabelDead(Field_t * pSelf)
{
	for (XRef_t *pXRef = pSelf->m_pXRefs; pXRef; pXRef = pXRef->Next())
	{
		Op_t *pOp = pXRef->Op();
		if (pOp->IsCodeOp())
		if ( pOp->IsHidden() )//valid!!!
			continue;

		if ( pOp->IsDataOp() )
		{
			IOp_t * pIOp = (IOp_t *)pOp;

			//Field_t *pField = (Field_t *)pOp;
			if (IsDataVisible(pIOp->GetOwnerData())
				&& !IsDataDead(pIOp->GetOwnerData()))
				return 0;

			Op_t * pOpSwitch = pIOp->GetSwitchOp();
			Path_t *pBlockSwitch = pOpSwitch->Path()->Parent();
			if (pBlockSwitch->Type() != BLK_SWITCH)
				return 0;

			if (!pIOp->IsCaseOf(pBlockSwitch))
				if (!pIOp->IsDefaultOf(pBlockSwitch))
					return 0;//visible

			continue;//FIXME
		}

		Path_t *pPath = pOp->Path();
		if (!pPath)
			return 0;

		if (CheckGOTOStatus5(pPath) == JUMP_GOTO)
			return 0;//actual - not dead!
	}

	return 1;//not visible
}

int DisplayInfo_t::IsLabelVisible(Field_t * pSelf)
{
	if ( pSelf->Path()->Type() == BLK_ENTER )
		return 0;
	if ( pSelf->Path()->Type() == BLK_EXIT )
		return 0;

	return DisplayUnfold_t::IsLabelVisible( pSelf );
}

void DisplayInfo_t::OutputOpName(std::ostream &os, Op_t * pSelf)
{
	static char buf[NAMELENMAX+1];
	const char *str = "";

	if (pSelf->objField())
	{
		Field_t *pField = pSelf->objField();
		pField->namex(buf);
		os << buf;
		return;
	}

	int flags = 0;
	switch (pSelf->OpC())
	{
	case OPC_CPUSW:
		flags = pSelf->CPUFlags(); goto $L0;
	case OPC_FPUSW:
		flags = pSelf->FPUFlags(); goto $L0;
	case OPC_FPUCW:
$L0:
		str = GDC.SS(pSelf->StorageId()).reg2str(0, 0, flags);
		break;

	case OPC_CPUREG:
/*?		if (this)
		if (!IsOutputUnfold())
		if (pSelf->IsEntry())
		if (pSelf->IsPtr())
		if (pSelf->IsThisPtr())
		{
			os << _RESERVED("this");
			return;
		}
*/
	case OPC_FPUREG:
	case OPC_AUXREG:
//		if (pSelf->m_pData)
//			str = pSelf->m_pData->Name();
//		else
			str = GDC.SS(pSelf->StorageId()).reg2str(pSelf->OpSize(), pSelf->Offset0(), 0);
		break;

	default:
		if (pSelf->IsIndirect() || pSelf->IsAddr())
		{
			buf[0] = 0;
			if (pSelf->IsAddr())
				strcat(buf, "&");

			if (pSelf->fieldRef())
			{
				pSelf->fieldRef()->namex(buf+strlen(buf));
//				strcat(buf, str);
			}
			else
			{
				if (pSelf->IsLocal())//OpC() == OPC_LOCAL)
				{
					str = GDC.SS(SSID_LOCAL).reg2str(pSelf->OpSize(), pSelf->Offset0(), 0);
					strcat(buf, str);
				}
				else
				{
					OutputIndirect(os, pSelf);
				}
			}

			buf[NAMELENMAX] = 0;
			os << buf;
			return;
		}
	}//switch

	if (isempty(str))
		str =  "???";
	strncpy(buf, str, NAMELENMAX);
	buf[NAMELENMAX-1] = 0;
	
	os << buf;
}

bool DisplayInfo_t::CheckTypeOutput(Out_t * pSelf)
{
	if (!TestOpt(DUMP_TYPES))
		return false;

	if (pSelf->Action == ACTN_TYPE)
		return false;

	if (pSelf->OutT == OUT_OP)
	{
		if (pSelf->SSID() == SSID_CPUSW || pSelf->SSID() == SSID_FPUSW)
			return false;
	}

	switch (pSelf->Action)
	{
	case ACTN_MOV:
	case ACTN_COMMA:
	case ACTN_CHECK:
		return false;
	}
	if (ISCMPRSN(pSelf->Action))
		return false;//bool
	if (ISLOGIC(pSelf->Action) || pSelf->Action == ACTN_LOGNOT)
		return false;//bool
	if (!pSelf->pOutU)
		return false;
//	if (pSelf->pOutU->Action == ACTN_TYPE)
//		return false;
	if (pSelf->pOutU->Action == ACTN_OFFS)
		return false;
	if (pSelf->pOutU->Action == ACTN_CALL && pSelf->IsLeft())
		return false;//pfunc
	if (pSelf->pOutU->Action == ACTN_ARRAY && pSelf->IsLeft())
		return false;//ptr
	if (pSelf->IsRight())
	{
		if (pSelf->pOutU->Action == ACTN_PTR || pSelf->pOutU->Action == ACTN_POINT)
			return false;//field
	}
	if (pSelf->Action == ACTN_CALL && pSelf->pOutU->Action != ACTN_MOV)
		return false;
	return true;
}

void DisplayInfo_t::OutputOpType(std::ostream &os, Op_t * pSelf)
{
	if (!pSelf->IsAddr())
	{
		if ( TestOpt(DUMP_TYPES) )
		{
			//if ( !IsCodeOp() /*|| !Root()->IsHidden()*/ )
			{
				os << COLOR(COLOR_TYPES);
				os << "{";
				output_optype(os, pSelf->OpType(), 1);
				os << "}";
				os << COLOR(0);
			}
		}
	}
}

bool DisplayInfo_t::IsConjumpVisible(Path_t * pSelf)
{
	return pSelf->IsConjumpVisible0();
}

int DisplayUnfold_t::IsRootVisible(Op_t * pSelf)//0-if not
{
	assert(pSelf->m_pRI);
	assert(pSelf->Path());

CHECK(pSelf->No() == 58)
STOP

//	if (!Path())
//		return 1;

		if (pSelf->IsDeadEx())
		{
//			if (IsDead())
//			{
//				if (IsOutputDead())
//					return 1;
//				return 0;
//			}

			if (IsOutputDead())
				return 1;
			return 0;
		}
		
		if (!IsOpRootEx(pSelf))
			return 0;
/*?	{
		if (pSelf->IsDeadEx())
			return 0;

		if (!pSelf->IsRoot())
			return 0;
	}*/

	if ( pSelf->IsRet() )
	{
		if ( !IsOutputDead())
		if ( pSelf->IsLastEx() && !pSelf->GetArgs() )
			return 0;
	}
	else if ( pSelf->IsGoto() )
	{
		if ( !IsOutputDead())
		{
			Path_t *pPath = pSelf->Path();
			if (pPath && pPath->CheckGOTOStatus() == 0)
				return 0;
//			if ( CheckGOTOStatus() == 0 )
//				return 0;
		}
	}
	else if ( pSelf->IsCondJump() )
	{
//?		if ( this )
		{
			if (!IsConjumpVisible(pSelf->Path()))
				return 0;
		}
//?		else 
//?		if (!pSelf->Path()->IsConjumpVisible0())
//?			return 0;
	}
	else
	{
//		if (Action() == ACTN_MOV)
//		{
//			if (pSelf->IsHidden())
//				return 0;
//			if (pSelf->OPIn(0).IsStackPtr())
//				return 0;
//		}
//
//		if (!CheckOverwrite())
//			return 0;
		if (pSelf->IsHidden())
		{
			if (!IsOutputDead())
				return 0;
		}
	}

	return 1;
}

void DisplayInfo_t::OutputPseudolabel0(std::ostream &os, Op_t * pSelf)
{
	assert(pSelf->m_pRI);

//	Field_t *pLabel = 0;
	Op_t *pOp = pSelf;
	while (!IsRootVisible(pOp))
	{
//		if (pOp->Label())
//			pLabel = pOp->Label();
		pOp = pOp->NextEx();
	}
/*
	if (!pLabel)
	{
		//find last real label
		Op_t *pOp2 = pOp;
		while (pOp2)
		{
			if (pOp2->Label())
			{
				pLabel = pOp2->Label();
				break;
			}
			pOp2 = pOp2->PrevEx();
			if (!pOp2)//first
				pLabel = pOp->GetOwnerFunc();
		}

		//?pLabel = pOp2->Path()->LabelPrev();
	}
*/
//	if (!pLabel || !pLabel->Name())
	{
		static char buf[20];
		os << "$loc_" << itoa(pOp->No(), buf, 10);
	}
//	else
//	{
//		os << pLabel->Name();
//	}
}




UInt32 _A(UInt32 action)
{
	if (ISJMPIF(action))
		action = ACTN_JMPIF;
	else if (ISCALL(action))
		action = ACTN_CALL;

	return action;
}

Op_t * DisplayInfo_t::GetLabelOp( Op_t * pSelf )
{
	assert(pSelf->m_pRI);

	Op_t *pOp = pSelf;
	while (pOp)
	{
		if (pOp->Label())
			break;

		pOp = pOp->PrevEx();

		if (IsRootVisible(pOp))
		{
			if (!pOp->IsSplit())
			if (!pOp->IsHidden())
			{
				pOp = 0;
				break;
			}
		}
	}

	if (pOp)
		if (!pOp->Label())
			return 0;

	return pOp;
}

int Op_t::CheckBreak()
{
	if (Path())
		if (Path()->CheckGOTOStatus() == JUMP_BREAK)
			return 1;

	return 0;
}

int DisplayInfo_t::CheckDoWhile(Path_t * pSelf)
{
	if (pSelf->m_nType != BLK_LOOPDOWHILE)
		return 0;

	Path_t * pParent = pSelf->m_pParent;
//	if (!pParent)
//		return 1;

	if (pParent->m_nType == BLK_IFWHILE) 
		return 0;

	if (pParent->m_nType == BLK_NULL)
	{
		if ( !pParent->IsLast())
			return 1;

		pParent = pParent->m_pParent;
		if ( !pParent )
			return 1;

		if (pParent->m_nType == BLK_IFWHILE)
			return 0;
	}

	return 1;
}


//////////////////////////
// O U T P U T


#if(0)
int Op_t::__checkOpenedGOTOs(bool bGotoOnly)
{
	assert(m_pRI);

	Op_t *pOp = this;
	while (1)
	{
		if (pOp->IsFirstEx())//first achieved
		{
			break;
		}

		if (pOp->Label())
		{
			for (XRef_t *pXRef = pOp->Label()->m_pXRefs; pXRef; pXRef = pXRef->Next())
			{
				bool bSwitch = false;
				Op_t *pOp = pXRef->pOp;
				if ( pOp->IsDataOp() )
				{
					bSwitch = true;
					pOp = ((Field_t *)pOp)->GetSwitchOp();
				}

//				if ( !pOp->opnd_BLOCK )
				{
					if (!pOp->CheckBreak())
					{
						if ( pOp->IsCondJump() )
						{
							if (!bGotoOnly)
							if (pOp->GetArgs()->CheckCount(1) <= 0)
								return 1;//actual
						}
						else
						{
							assert(pOp->IsGoto());
							if (bSwitch)
								return 1;
//							if (pOp->GetGotoDestOp()->Label() == this)
								return 1;
						}
					}
				}


/*				Op_t *__pOp = pXRef->pOp;
				if (__pOp->IsData())
					__pOp = __pOp->GetSwitchOp();
				else
				{
					if (__pOp->IsGoto())
						return 1;
					assert(ISJMPIF(__pOp->Action()));
					if (!__pOp->opnd_BLOCK)
						return 1;
					continue;
				}

				if (__pOp->__checkOpenedGOTOs())
					return 1;
*/			}
		}

		pOp = pOp->PrevEx();
		if (!pOp->IsHidden())
			if (pOp->IsRoot())
				if (!pOp->IsSplit())
					break;

		if ( (pOp->IsGoto())
			|| (pOp->IsRet()) )
			break;//logic block end achived
	}

	return 0;
}
#endif

void DisplayInfo_t::OutputGoto(std::ostream& os, Op_t * pSelf)
{
	if (pSelf->IsRet())
//	if (!IsObjVisible(pSelf->labelRef()))
	{
		OutputRet(os, pSelf);
		return;
	}

	if (pSelf->IsDataOp())
	{
		os << _RESERVED("goto") << " ";
		OutputLabelRef(os, pSelf->labelRef());
		os << ";";
		return;
	}

CHECK(pSelf->No() == 229)
STOP

	if (pSelf->IsHidden())
	{
		os << _RESERVED("goto") << " ";
		OutputLabelRef(os, pSelf->labelRef());
		os << ";";
		return;
	}

	if (pSelf->IsGoto())
	{
		if (pSelf->Path() && !IsOutputUnfold())
		{
			if (pSelf->Path()->m_nType == BLK_JMPSWITCH)
			{
				if (/*?showBlocks() &&*/ pSelf->Path()->Parent()->m_nType == BLK_SWITCH)
				{
					os << _RESERVED("switch") << " (";
					OutputSwitch(os, pSelf);

					os << ")";
				}
				else
				{
					os << _RESERVED("goto") << " ";
					OutputExpression3(os, pSelf);
					os << ";";
				}
				return;
			}
		}
	}
	else
		assert(pSelf->IsCondJump());

	if (!pSelf->IsAddr())//like "goto eax" : probably switch jump
	{
		os << _RESERVED("goto") << " ";
		OutputExpression3(os, pSelf);
		os << ";";
		return;
	}

	if ( pSelf->IsRet() )
	{
		os << _RESERVED("goto") << " ";
		OutputLabelRef(os, pSelf->labelRef());
	}
	else if (pSelf->CheckBreak())
	{
		os << _RESERVED("break");
	}
	else
	{
		Op_t *pOpDest = pSelf->labelRef()->Ops();
		if (!pOpDest)
		{
			os << "?";
			return;
		}
		if ( !IsOutputUnfold() /*&& IsGoto()*/ )
		{
			pOpDest = pSelf->GetGotoDestOp();
			if (pOpDest->IsRet())
			{
				os << FONT(FONT_ITALICS);//not real return
				OutputRet(os, pOpDest);
				return;
			}
		}

		os << _RESERVED("goto") << " ";
		Op_t * pOpGoto = GetLabelOp(pOpDest);
		if (!pOpGoto)
		{
			OutputPseudolabel0(os, pOpDest);
		}
		else
		{
			Field_t *pLabel = pOpGoto->Label();
			assert(pLabel);
			//os << pLabel->Name();
			OutputLabelRef(os, pLabel);
		}
	}

	os << ";";
}








void DisplayInfo_t::OutputCase(std::ostream& os, IOp_t * pIOp )
{
	assert(pIOp->IsDataOp());

	int nValue = pIOp->offset() / pIOp->OpSize();
	Op_t * pOp2;

	if (pIOp->fieldRef())//IsPtr())
	{
		pOp2 = pIOp->GetSwitchOp();
	}
	else
	{
		assert(pIOp->GetOwnerData()->m_pXRefs->CheckCount(1) == 0);
		pOp2 = pIOp->GetOwnerData()->m_pXRefs->Op();
		pOp2 = pOp2->GetSwitchOp2();
	}

	Path_t *pPathJmpSwitch = pOp2->Path();
	assert(pPathJmpSwitch->m_nType == BLK_JMPSWITCH);
	SwitchInfo_t si;
	pPathJmpSwitch->GetSwitchInfo(si);

	assert(si.pJumpTable);
	if (si.pIndexTable)
	{
		assert(!si.nIndexOffset1);
		nValue -= si.nIndexOffset2;
	}
	else
	{
		nValue -= si.nIndexOffset1;
	}

	int nIndent = GetPathIndent(pOp2->Path());
	os << TABS(nIndent);
	os << _RESERVED("case") << " ";
	os << Int2Str(nValue, I2S_HEXC);
	os << ":";
}

void DisplayInfo_t::OutputDefault(std::ostream& os, Path_t * pPath)
{
	int nIndent = GetPathIndent(pPath);
	os << TABS(nIndent);
	os << _RESERVED("default:");
}


//////////////////
// F u n c _ t




/*?const char *Func_t::Declaration(UInt32 flags)//no coloring
{
	static char buf[1024];
	std::ostrstream outs(buf, sizeof(buf));

	outs.seekp(0);
	OutputDeclaration(outs, funcdef(), flags);
	outs << '\0';
	return buf;
}*/

///////////////////////////
// P a t h _ t 




int DisplayInfo_t::CheckGOTOStatus5(Path_t * pSelf)
{
	return pSelf->CheckGOTOStatus();
}

void DisplayInfo_t::OutputPathCondjump(std::ostream &os, Path_t * pSelf)
{
	assert(pSelf->m_nType == BLK_JMPIF);

	Path_t *pPath = pSelf;
	Path_t *pPathN = pPath->GetLogicsTop(1);//check first
	if (pPathN)
		pPath = pPathN;

	int res = 0;
	Path_t *pParent = pPath->Parent();
	if (pParent)
	{
		if (pParent->m_nType == BLK_IFWHILE)
		{
			os << _RESERVED("while") << " (";
			OutputPathCondition3(os, pPath);
			os << ")";
			OutputPathGoto(os, pPath);
			return;
		}
		
		if (pPath->IsLast() && pParent->m_nType == BLK_LOOPDOWHILE)//pParent->CheckDoWhile())
		{
			os << "} " << _RESERVED("while") << " (";
			OutputPathCondition3(os, pPath);
			os << ");";
			return;
		}

		if (pParent->CheckElseIf())
		{
			os << _RESERVED("else") << " ";
		}
	}
		
	os << _RESERVED("if") << " (";
	OutputPathCondition3(os, pPath);
	os << ")";
	OutputPathGoto(os, pPath);
	return;
}

void DisplayInfo_t::OutputDecl0(std::ostream& os, Func_t * pFunc)
{
	os << COLOR(COLOR_COMMENT) << "//";

	char buf[NAMELENMAX];
	pFunc->parent()->namex(buf);
	char * p = buf;
	while (*p)
	{
		os << " " << *p;
		p++;
	}
}


int DisplayInfo_t::GetPathIndent(Path_t * pSelf)
{
//	assert(IsTerminal());
	int nLevel = 0;

	Path_t *pPath = pSelf;
	while (1)//pPath->Parent())
	{
		if (pPath->IsRoot())
		{
			nLevel++;
			break;
		}

		Path_t *pParent = pPath->Parent();
		if (pParent->m_nType == BLK_IF)
		{
			if (!pPath->IsFirst())
			{
				if (pPath->m_nType != BLK_SWITCH)
					nLevel++;
//				if (pPath->m_pParent->Parent())
//				if (pPath->m_pParent->Parent()->m_nType == BLK_IFELSE)
//					nLevel--;
			}
		}
		else if (pParent->m_nType == BLK_IFELSE)
		{
			if (!pPath->IsFirst())
				if (!pPath->CheckElseIf())
					nLevel++;
		}
		else if (pParent->m_nType == BLK_CASE)
		{
			nLevel++;
		}
		else if (pParent->m_nType == BLK_DEFAULT)
		{
			nLevel++;
		}
		else if (pParent->m_nType == BLK_LOOPENDLESS)
		{
			nLevel++;
		}
		else if (pParent->m_nType == BLK_LOOPDOWHILE)
		{
			if (CheckDoWhile(pParent))
				nLevel++;
		}
		else if (pParent->m_nType == BLK_IFWHILE)
		{
			if (!pPath->IsFirst())
				nLevel++;
		}

		pPath = pParent;
	}

	return nLevel;
}

void DisplayInfo_t::OutputPath(std::ostream& os, Path_t * pPath )
{
		assert(!pPath->m_pChilds);

		os << TABS(GetPathIndent(pPath));

		if (pPath->m_nType == BLK_JMPIF)
		{
			OutputPathCondjump(os, pPath);
		}
		else if (pPath->m_nType == BLK_JMPSWITCH 
		//[		&& pPath->Parent() 
			&& pPath->Parent()->m_nType == BLK_SWITCH)
		{
			os << _RESERVED("switch") << " (";
			os << pPath->GetFirstOp()->No();
			os << ")";
		}
		else
		{
			os << "[";//"{";
			Op_t *pOpFirst = pPath->GetFirstOp();
			if (pOpFirst)
				os << pOpFirst->No();
			else
				os << pPath->IndexEx();
//			if (pPath->m_nType == BLK_RET)
//			{
//				os << "/" << _RESERVED("return");
//			}
//			else 
			if (pPath->m_nType == BLK_JMP)
			{
				if (pPath->IsRetPath())
					os << "/" << _RESERVED("return");
				else
					OutputPathGoto(os, pPath);
			}
			os << "]";//"}";
		}
}

void DisplayInfo_t::OutputBreak( std::ostream& os, Path_t * pPath )
{
	assert(pPath->m_nType == BLK_SWITCH);
	int nIndents = GetPathIndent(pPath->GetChildFirst());
	os << TABS(nIndents);
	os << TAB;
	os << _RESERVED("break") << ";";
}










/*void DisplayInfo_t::OutputLocal(std::ostream& os, Data_t * pData)
{
		int nIndent = 1;
		if (IsOutputAsm())
			nIndent = 0;

		bool bDead = false;
		if (pData->IsDead())//m_nFlags & DAT_HIDDEN)
			if (!IsOutputUnfold())
				bDead = true;

		if (bDead)
			nIndent = 0;

		if (nIndent)
			os << TABS(nIndent);

		if (bDead)
			os << COLOR(COLOR_COMMENT) << "//? ";

		pData->Out(os);

		if (!pData->m_pXRefs)
			os << COLOR(COLOR_COMMENT) << "//UNREFERENCED!";
		}
}*/



void DisplayInfo_t::OutputArg( std::ostream& os, Op_t * pOp )
{
	if (pOp->IsEntry())
	{
//		Data_t *pData = pOp->m_pData;
//		if (pData)
//			pData->OutputLocal(os);
//		else
		{
			bool bDead = false;
			if (pOp->IsThisPtr())
				bDead = true;
			if (bDead)
				os << COLOR(COLOR_COMMENT) << "//";

			os << TAB;
			OutputArg2(os, pOp);
			if (!pOp->IsLast())
				os << ",";
			else
				os << ")";
		}
	}
	else if (pOp->IsExit())
	{
		TYPE_t T;
		pOp->GetType(T);
		OutputTYPE0(os, &T);
		if (!pOp->IsLast())
			os << ",";
	}
	else
	{
		assert(false);
	}
}










void DisplayInfo_t::OutputPseudolabel( std::ostream &os, Op_t * pOp )
{
	OutputPseudolabel0(os, pOp);
	os << ":";
}

void DisplayInfo_t::OutputFuncDef(std::ostream& os, FuncDef_t * pfDef )
{
	os << _RESERVED("typedef") << " ";
	OutputFuncDef0(os, pfDef, pfDef->Name(), 0);
	os << ";";
}

void DisplayInfo_t::OutputUnionBeg( std::ostream& os, Field_t * pField )
{
	os << TAB;

	int nLevel = pField->GetIndentLevel();
	os << TABS(nLevel-1);

	os << _RESERVED("union") << " {";
}

void DisplayInfo_t::OutputUnionEnd( std::ostream& os, Field_t * pField )
{
	os << TAB;

	int nLevel = pField->GetIndentLevel();
	os << TABS(nLevel-1);

	os << "};";
	os << COLOR(COLOR_COMMENT) << "//union" ;
}

void DisplayInfo_t::OutputStrucBeg2( std::ostream& os, Field_t * pField )
{
	os << TAB;

	int nLevel = pField->GetIndentLevel();
	if (pField->IsUnderUnion())
		nLevel--;
	nLevel--;
	os << TABS(nLevel);

	os << _RESERVED("struct") << " {";
}

void DisplayInfo_t::OutputStrucEnd2( std::ostream& os, Field_t * pField )
{
	os << TAB;
	
	int nLevel = pField->GetIndentLevel();
	if (pField->IsUnderUnion())
		nLevel--;
	nLevel--;
	os << TABS(nLevel);

	os << "};";
	os << COLOR(COLOR_COMMENT) << "//struct" ;
}

void DisplayInfo_t::OutputStrucMethod( std::ostream& os, Func_t * pFunc )
{
	os << TAB;
	OutputDeclaration(os, pFunc->funcdef(), 2|4|8|0x30);//don't draw owner struc
	os << ";";
}

void DisplayInfo_t::OutputElse( std::ostream& os, Path_t * pPath )
{
	os << TABS(GetPathIndent(pPath)) << _RESERVED("else");
}

void DisplayInfo_t::OutputDo( std::ostream& os, Path_t * pPath )
{
	os << TABS(GetPathIndent(pPath)) << _RESERVED("do");
}

void DisplayInfo_t::OutputStrX( std::ostream& os, const char * pc )
{
	const char *p = pc;
	while (*p) //get string length
	{
		if ((*p == 0xD) || (*p == 0xA))
			break;
		p++;
	}
	os.write(pc, (int)(p-pc));
}

void DisplayInfo_t::OutputStructOpRefs( std::ostream& os, Struc_t * pStruc )
{
	os << COLOR(COLOR_COMMENT) << "//Op:";
	strxdeps(os, pStruc->m_pXRefs);
}


void DisplayUnfold_t::OutputInclude(std::ostream& os, File_t * pFile)
{
	char buf[64];
	pFile->GetNameExt(buf);
	os << _RESERVED("#include") << " " << _STRING(buf);
}

void DisplayUnfold_t::OutputHeaderIfndef( std::ostream& os, File_t * pFile )
{
	os << _RESERVED("#ifndef") << " ";
	File_OutIDStr(pFile, os);
}

void DisplayUnfold_t::OutputHeaderDefine( std::ostream& os, File_t * pFile )
{
	os << _RESERVED("#define") << " ";
	File_OutIDStr(pFile, os);
}

void DisplayUnfold_t::OutputHeaderEndif( std::ostream& os, File_t * pFile )
{
	os << _RESERVED("#endif");
	os << COLOR(COLOR_COMMENT);
	os << "//";
	File_OutIDStr(pFile, os);
	os << COLOR(0);
}



char Op_t::GetFPUStatusChar()
{
	UInt8 _fpu = FpuIn();
	if (_fpu < 8)//ok
		return '0'+(8-_fpu);
	else if (_fpu == 8)
		return '-';//empty
	else if (_fpu < 0x7F)
		return 'u';//underflow
	else 
		return 'o';//overflow
}

/*
Op_t *Op_t::__filterOp()
{
	if (!IsCodeOp())
		return 0;

	if (m_pRI)
	{
		if (Action() != ACTN_MOV)
			return 0;
	}
	else
	{
		if (XIn()->CheckCount(1) != 0)
			return this;
	}
	
	Op_t *pOp = __filterOp()
}*/


Op_t *Op_t::__FilterDumpOp(XRef_t *pXIns, Path_t *pPath_, VALUE_t &v)
{
	if (pXIns->CheckCount(1) == 0)
	{
		Op_t *pOp = pXIns->Op();
		//if (pOp->IsEntry())//?
		//	return 0;
		return pOp;
	}

	Path_t *pPath = 0;
	Op_t *pDumpOp = 0;
	for (XRef_t *pXIn = pXIns; pXIn; pXIn = pXIn->Next())
	{
		Op_t *pOp = pXIn->Op();
		if (pOp->IsCode())
			if (!pPath)
				pPath = pOp->Path();
		if ( pOp->CheckFractalAssign(pPath) == 1)
			continue;

		if (pDumpOp)
			return 0;//alredy?
		pDumpOp = pOp;
	}
	return pDumpOp;
}

//test visibility of leaf op while outputting
int DisplayInfo_t::IsOpVisible(Op_t * pSelf)
{
//	if (IsCodeOp())
//	{
//		if (Root()->IsHidden())
//			return 0;
//	}

	if (pSelf->IsEntry())
	{
//?		if (!(pSelf->m_nFlags & OPND_NCONF))
		if (!TestOpt(DUMP_LOGICONLY))
		{
			if (!pSelf->IsThisPtr())
				return 1;
			if (IsOutputDead())
				return 1;
		}
	}
	else 
	{
		if (pSelf->m_pRI)
		{
			if (IsRootVisible(pSelf))
				return 1;
		}
		else if (pSelf->IsOnTop())
			return 1;
	}

	return 0;
}

/*
int Op_t::Select(bool bSel)
{
	if (!IsCode())
		return 0;

	if (bSel)
	{
		if (m_nType & OPND_SEL)
			return 0;
	}
	else
	{
		if (!(m_nType & OPND_SEL))
			return 0;
	}

	int nCount = 0;
	if (bSel)
	{
		if (!(m_nType & OPND_SEL))
		{
			m_nType |= OPND_SEL;
			nCount++;
		}
	}
	else
	{
		if (m_nType & OPND_SEL)
		{
			m_nType &= ~OPND_SEL;
			nCount++;
		}
	}

	for (XRef_t *pXIn = XIn(); pXIn; pXIn = pXIn->Next())
	{
		Op_t *pOp = pXIn->pOp;
		if (bSel)
		{
			if (!pOp->IsRootEx())
				nCount += pOp->Select(bSel);
		}
		else if (pOp->m_nFlags & OPND_SEL)
			nCount += pOp->Select(bSel);
	}

	if (m_pRI)
	for (Op_t *pOp = GetArgs(); pOp; pOp = pOp->Next())
	{
		if (bSel)
		{
//			if (!pOp->IsRootEx())
				nCount += pOp->Select(bSel);
		}
		else if (pOp->m_nFlags & OPND_SEL)
			nCount += pOp->Select(bSel);
	}

	return nCount;
}*/


void DisplayInfo_t::OutputSwitch(std::ostream &os, Op_t * pSelf)
{
	EXPR_t expr(m_dwFlags);

	Out_t *pOut = expr.DumpSwitch(pSelf);
	OutExpr(os, pOut);

#ifdef _DEBUG
	LogExp(pOut, pSelf->Root()->No());
#endif
}

int DisplayInfo_t::CheckProblem(Op_t * pSelf)
{
	if (!pSelf->m_pRI)
		return 0;
	if (!pSelf->IsAnalized())
		return 0;
	if (IsObjDead(pSelf))
		return 0;
	//...
	return 0;
}



////////////////////////////////////////


char * DisplayUnfold_t::GetDeclStr(Arg_t * pSelf)
{
	if (!pSelf->m_ssid)
		return "";

	static char str[256];
	std::ostrstream os(str, sizeof(str));
	
	os.seekp(0);
	OutputTYPE0(os, &pSelf->m_type, 1);
	os << pSelf->GetName();
	if (pSelf->m_type.m_nArray)
		os << "[" << pSelf->m_type.m_nArray << "]";
	os << '\0';
	return str;
}






///////////////////////////////////////////////

void DisplayInfo_t::OutputFuncDefScript(std::ostream &os, FuncDef_t * pSelf)
{
	StubInfo_t si;
	pSelf->GetStubInfo(si);

	bool B[7];
	B[6] = (pSelf->m_nFlags & FDEF_ARGSEX) != 0;
	B[5] = (pSelf->m_nSavedRegs != GFE.m.sav_regs_def)
		|| B[6];
	B[4] = (si.IsEmptyCPUIn()) || B[5];
	B[3] = (pSelf->FpuOut() != 0) || B[4];
	B[2] = (si.fpuin != 0) || B[3];
	B[1] = (pSelf->StackOut() != 0) || B[2];
	B[0] = (si.stackin != 0) || B[1];

	os << si.stackin;
	if ( B[1] )
		os << " " << pSelf->StackOut();
	if ( B[2] )
		os << " " << si.fpuin;
	if ( B[3] )
		os << " " << (int)pSelf->FpuOut();
	if ( B[4] )
		os << " " << std::hex << si.CPUInStr() << std::dec;
	if ( B[5] )
		os << " " << std::hex << pSelf->m_nSavedRegs << std::dec;
	if ( B[6] )
		os << " " << std::hex << "1" << std::dec;
}






void DisplayInfo_t::OutputFuncDef0(std::ostream &os, FuncDef_t * pSelf, const char *name, int ptr_level)
{
	OutputFuncDefRets(os, pSelf);

	bool bPtr = ptr_level > 0;

	if (bPtr)
		os << "(";

	OutputCallType(os, pSelf);

/*!	Op_t * pOpThis = pSelf->GetThisPtr();
	if (pOpThis && pOpThis->m_pStruc)
	{
		os << pOpThis->m_pStruc->Name();
		os << "::";
	}*/

/*	int ptr_level;
	if (pField)
		ptr_level = pField->PtrLevel();
	else
		ptr_level = 1;

	char *name = unk();
	if (pField)
		name = pField->Name();*/

//	assert(ptr_level);
	if (bPtr)
	while (ptr_level--)
		os << "*";

	os << name;

	if (bPtr)
		os << ")";

	//arguments
	os << "(";
	OutputFuncDefArgs(os, pSelf);
	os << ")";
}


bool DisplayUnfold_t::IsObjDead(Obj_t * pObj)
{
	return false;
/*?	switch (ObjType())
	{
	case OBJ_ DATA:
		return ((Data_t *)pObj)->IsDead();
	case OBJ_FUNC:
		return 0;//always alive
	case OBJ_LABEL:
		return ((Field_t *)pObj)->IsDead() != 0;
	}

	return 0;*/
}


int Path_t::GetSwitchInfo(SwitchInfo_t &si)
{
	EXPR_t expr(0, EXPR_NOEXTEND);
	assert(m_nType == BLK_JMPSWITCH);
	Op_t *pOpSwitch = GetLastOp();
	Out_t *pOut = expr.DumpExpression2(pOpSwitch);
	expr.GetSwitchInfo(pOut->pOutR, si);
	return 1;
}





