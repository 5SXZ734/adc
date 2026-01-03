#include "ana_pcode.h"
#include "prefix.h"
#include "shared/defs.h"
#include "shared/misc.h"
#include "shared/front.h"
#include "shared/action.h"
#include "shared/data_source.h"
#include "front/front_IA.h"

#include "db/mem.h"
#include "db/field.h"
#include "db/type_struc.h"
#include "db/type_code.h"
#include "db/type_seg.h"
#include "db/proj.h"
#include "db/script.h"
#include "db/command.h"
#include "db/main.h"

#include "info_dc.h"
#include "files_ex.h"
#include "clean_ex.h"
#include "path.h"
#include "op.h"
#include "ana_branch.h"
#include "ana_switch.h"
#include "ana_init.h"
#include "ana_main.h"
#include "ui_main_ex.h"

//////////////////////////////////////
// PCodeTracer_t

//this will crash VS2015 compiler:
//PCodeTracer_t::PCodeTracer_t(const FuncInfo_t &r GenerateStage_t& rAnalizer)

PCodeTracer_t::PCodeTracer_t(const FuncTracer_t &r, GenerateStage_t& rStage, PCodeTraceData &rData)
	: FuncTracer_t(r),
	PCodeTraceData(rData),//OwnerSeg(r.OwnerFuncRef()), OwnerFuncRef()->parentField()->Offset()),
	mrStage(rStage)//,
	//mpCachedArgs(nullptr)
{
	if (!miSeg)
		miSeg = r.OwnerSeg();
	//if (!mBaseAddr)
		//mBaseAddr = OwnerFuncRef()->parentField()->Offset();

	miTypeCode = rStage.typeCode();//save code type for incremental passes
}

PCodeTracer_t::PCodeTracer_t(const FuncInfo_t &fi, PathOpTracer_t &tr, GenerateStage_t& rStage, PCodeTraceData &rData)
	: FuncTracer_t(fi, tr),
	PCodeTraceData(rData),
	mrStage(rStage)//
	//mpCachedArgs(nullptr)
{
	if (!miSeg)
		miSeg = fi.OwnerSeg();

}

PCodeTracer_t::~PCodeTracer_t()
{
	uint16_t f(FUI_ALL);
	f &= ~FUI_BASE;
	SetChangeInfo(f);
}

ADDR PCodeTracer_t::LowerBound() const
{
	if (mBaseAddr == 0)//guess a function never starts at 0 offset?
		const_cast<PCodeTracer_t *>(this)->mBaseAddr = DockAddr();
	return mBaseAddr;
}

ADDR PCodeTracer_t::UpperBound() const
{
CHECK(mBaseAddr == 0x20623820)
STOP
	if (mEndAddr < mBaseAddr)
	{
		CFieldPtr pField0(DockField());
		ADDR lower(pField0->_key());
		CTypePtr pType(nullptr);
		ADDR upper(lower);
		
		const FieldMap& m(pField0->owner()->typeStruc()->fields());
		for (FieldMapCIt it(m.lower_bound(lower)); it != m.end() && KEY(it) == lower; ++it)
		{
			CFieldPtr pField(VALUE(it));
			if (pField->isTypeProc())
			{
				assert(!m.empty());
				//pField = EntryField(pField->type());
				const FieldMap& m2(pField->type()->typeStruc()->fields());
				assert(!m2.empty());
				pField = VALUE(m2.rbegin());//last field
				upper = pField->_key();
				pType = pField->type();
				break;
			}
			if (pField->isTypeThunk())
			{
				pType = pField->isTypeThunk()->baseType();
				break;
			}
			if (pField->isTypeCode())
			{
				pType = pField->isTypeCode();
				break;
			}
		}

		//if (pField->isClone())
		//	pField = CloneLead(pField);

//CHECKID(pProc, 0xb71)
//STOP
		assert(pType);
		if (pType->typeCode())
		{
			DataSourcePane2_t data(GetDataSource()->pvt(), miSeg);
			CodeIterator codeIt(data, pType, miSeg, upper);
			while (codeIt.unassemble())
			{
				STOP//until path break
			}
			upper = codeIt.nextAddr();
		}
		const_cast<PCodeTracer_t*>(this)->mEndAddr = upper;
	}
	return mEndAddr;
}

bool PCodeTracer_t::CheckCodeBounds(ADDR va) const//true if inside
{
	return LowerBound() <= va && va < UpperBound();
}

bool PCodeTracer_t::RemoveLastOps(int iNum)
{
	if (!mpCurOp || !CurrentPath())
		return false;

	FuncCleaner_t FC(*this);
	for (int i(0); i < iNum; i++)
	{
		HOP hOp(mpCurOp);
		assert(hOp);
		mpCurOp = PrevPrime(hOp);
		FC.DestroyPrimeOp(hOp);
	}

	FC.Cleanup();
	FC.CleanupFinal();

		/*	DcInfo_t DI(mrDC, memMgrGlob());
			DcCleaner_t<> DC(DI);
			//all unreffed types are owned by global memmgr - must be disposed with dc cleaner
			while (FC.hasUnrefedTypes())
				DC.DestroyTypeRef(FC.popUnrefedType());
			DC.destroyUnrefedTypes();*/

	mpCurOp = HOP();
	return true;
}

bool PCodeTracer_t::IsSeen(HPATH pPath)
{
	return !PathOps(pPath).empty();
}

bool PCodeTracer_t::IsProcessed(HPATH pPath)
{
	if (PathOps(pPath).empty())
		return false;
	OpPtr pOp(GetFirstOp(pPath));
	if (pOp && !CheckAlreadyAnalized(pOp, mpLastOp))
	{
		SetFuncStatus(FDEF_DC_ERROR);
		//mrStage.setStopper();
		//fprintf(stderr, "%d\n", pOp->No());//?
	}
	return true;//already built
}

HOP PCodeTracer_t::GetRefOp(ADDR va) const
{
	if (va < LowerBound())
		return HOP();
	PathMapIt it(mPathMap.upper_bound(va));
	if (it == mPathMap.begin())
		return HOP();//above the scope (or the map is empty)
	HPATH pPath((--it)->second);
	if (!pPath)
		return HOP();//fall in a gap?
	if (PathIsEmpty(pPath))
		return HOP();

	HOP pOp(GetLastOp(pPath));
//#ifdef _DEBUG
#if(0)
	int off(va - LowerBound());
	HOP pOp2(PRIME(PathOps(pPath).Head()));
	for (; pOp2; pOp2 = NextPrime(pOp2))
	{
		if (No(pOp2) >= off)
		{
			do {
				if (No(pOp2) > off)
					break;
			//check micro-ops
				if (pOp2 == pOp)
					break;
				pOp2 = NextPrime(pOp2);
			} while (pOp2);
			assert(pOp2 == pOp);
			break;
		}
	}
/*	FieldPtr pLabel(Field(OwnerFuncRef(), va, nullptr, 0));
	if (!pLabel)
		return HOP();

	pPath = La belPath(pLabel));
	if (PathOps(pPath).empty())
		return HOP();
	HOP pOp;
	for (pOp = PRIME(PathOps(pPath).Head()); DockField()->address() + No(pOp) < va; pOp = NextPrimeEx(pOp))
	{
		assert(pOp);
	}*/
	
#endif
	return pOp;
}

CodeIterator2::CodeIterator2(const I_DataSourceBase &rData, CTypePtr iTypeCode, CTypePtr iSeg, ADDR va, const I_FrontDC &rFrontDC)
	: CodeIterator(rData, iTypeCode, iSeg, va),
	mrFrontDC(rFrontDC)
{
}

bool CodeIterator2::generate(PCode_t &pcode)
{
	if (operator bool())
	{
		//updateAddr();
		if (miTypeCode->typeCode()->generate(*this, m_imageBase, m_addr, m_desc, pcode, mrFrontDC) > 0)
		{
			if (m_desc.isFinalized())
				seek(OFF_NULL);
			return true;
		}
		MAIN.printError() << "Unsupported instruction at " << ProjectInfo_t::VA2STR(m_addr, mbLarge, m_imageBase) << std::endl;
		pcode.newop(pcode.newins(ACTN_INVALID));//make it be displayed anyway
		pcode.setInvalid();
		seek(OFF_NULL);
	}
	return false;
}
//ADDR gVA;
int PCodeTracer_t::GeneratePseudoCode(const Locus_t &aLoc, ADDR vaRef, int iThunkLevel)
{
	ADDR start_addr(aLoc.addr());
	TypePtr pSeg(aLoc.seg());
//CHECK(start_addr == 0x50beb4)
CHECK(vaRef == 0x50bec4)
STOP

	if (!miTypeCode)
		return 0;

	if (vaRef != -1)
	{
		mpLastOp = GetRefOp(vaRef);
		if (!mpLastOp)
			return 0;
	}

//	fprintf(stdout, " *** %X\n", start_addr);

	ADDR mCurAddr = start_addr;

	DataSourcePane2_t data(GetDataSource()->pvt(), pSeg);
	CodeIterator2 codeIt(data, miTypeCode, pSeg, mCurAddr, mrFrontDC);

	for (;;)//sweep thru instruction flow until a break (jmp or ret)
	{
		mCurAddr = codeIt.updateAddr();
		//gVA = mCurAddr;
CHECK(mCurAddr + ImageBase() == 0x005a8332)
STOP
		PCode_t pcode(pSeg->imageBase());
		if (!codeIt.generate(pcode))
		{
			if (!pcode.isInvalid())
				break;//natural
			//create invalid instruction before break
		}

		ADDR nextAddr(codeIt.nextAddr());

		if (!CurrentPath())
		{
			PathMapIt j(AssurePathBeginsAt(mCurAddr, mCurAddr));
			if (IsProcessed(j->second))
				return 0;
			SetCurrent(j);
			CurrentPath()->setAnalized(true);
		}
		else if (iThunkLevel == 0)
		{
			PathMapIt j(GetPathAt(mCurAddr));
			if (j != mPathMap.end())
			{
				if (j->second != CurrentPath())//may be true after user's interference at path's first op
				{
					if (IsProcessed(j->second))
						return 0;
					SetCurrent(j);
					CurrentPath()->setAnalized(true);
				}
			}
		}

		HOP pLastOpEx(mpLastOp);//prior the current instruction
		int nCount(0);
		mpCurOp = HOP();
		try
		{
			for (PCode_t::const_iterator it(pcode.begin()); it != pcode.end(); it++, nCount++)
			{
				if (!SetOps(*it, mCurAddr, 0))//-(iThunkLevel * RetAddrSize())))
					return -1;

				SetRoot(mpCurOp, 1);//reset some flags
				Numberize(mpCurOp, mCurAddr);

				if (ActionOf(mpCurOp) == ACTN_INVALID)
					break;

				bool bIsGoto(IsGoto(mpCurOp));//direct
				if (IsRetOp(mpCurOp))
				{
					assert(iThunkLevel == 0);
					//AddPathRef(CreateTailPath(), mpCurOp);
				}
				else if ((bIsGoto && IsAddr(mpCurOp)) || IsCondJump(mpCurOp))
				{
					if (iThunkLevel > 0)//jump is expected
						return 2;
					if (CheckCodeBounds(mpCurOp->m_disp))
					{
						PathMapIt jj(AssurePathBeginsAt(mpCurOp->m_disp, mCurAddr));
						HPATH pPath(jj->second);
						AddPathRef(pPath, mpCurOp);
					}
					else
						PrintWarning() << "Jumping out of function bounds @" << VA2STR(mCurAddr) << std::endl;
				}

				InitialTracer_t an(*this);
				//an.setCahchedArgs(mpCachedArgs);
				//an.Preanalize(mpCurOp, mCurAddr);
				
				for (;;)
				{
					InitialTracer_t::CheckCallResult eRes(an.CheckCall(mpCurOp));
					if (eRes == InitialTracer_t::CALL_ERROR)
					{
						if (an.IsCallDirect(mpCurOp) && mpCurOp->m_disp == nextAddr)
						{
							TakeOp(CurrentPath(), mpCurOp);
							DeleteRootInfo(mpCurOp);
							Delete(mpCurOp);
							mpCurOp = HOP();
						}
						break;
					}
					if (eRes != InitialTracer_t::CALL_THUNK)
						break;
#if(1)
					FieldPtr pThunkField(IsCallToThunkAt(mpCurOp));
					assert(pThunkField);
					//redirect analyzer to thunk's target
					Locus_t aLoc2;
					LocusFromVA(pThunkField, aLoc2);
					HPATH hPath(CurrentPath());
					HOP pOpCall(TakeOp(hPath, mpCurOp));
					assert(pOpCall->args().empty());
					mpCurOp = HOP();
					if (GeneratePseudoCode(aLoc2, -1, iThunkLevel + 1) != 2)
						ASSERT0;
					if (ActionOf(mpCurOp) == ACTN_INVALID)
						pcode.setInvalid();
					else
					{
						mpCurOp->setAction(ActionOf(pOpCall));
						mpCurOp->addPStackDiff(pOpCall->pstackDiff());
						SetOpVA(mpCurOp, OpVA(pOpCall));//retain address
					}
					
					//FuncCleaner_t FD(*this);
					//FD.DestroyRhs(pOpCall);
					DeleteRootInfo(pOpCall);
					Delete(pOpCall);
					//go to re-check
#else
					break;
#endif
				}

				if (!mpCurOp)
					continue;//was deleted above

CHECK(mCurAddr == 0x8051d6d)
STOP
				if (iThunkLevel == 0 || !bIsGoto)//if about to exit a thunk - this will be done in a caller
				{
					an.CheckSpecial(mpCurOp);//check for special ops combinations
					an.CheckShift(mpCurOp);
					an.CheckReadOnly(mpCurOp);
					int res(an.Analize(mpCurOp, mpLastOp));
					if (res < 0)
						return -1;//critical error

					mpLastOp = mpCurOp;
				}

				if (iThunkLevel == 0)
				{
					//mPathMapIt->second.second = mCodeIt.nextAddr();
					BlockTyp_t blkType(FinalizePath(CurrentPath()));
					if (blkType != BLK_NULL)
					{
						//if (blkType == BLK_JMPIF)	//WARNING: Always mark the end of the path even if the label never referenced (will be discarded anyway!)
						mPathMap.insert(std::make_pair(codeIt.nextAddr(), HPATH()));
						ResetCurrentPath();
					}
				}
			}
		}
		catch (const StubFault_t &stubFault)
		{
			//if (iError == -7)
			{
				mrProject.analyzer()->setStopFlag(StopFlag::PAUSE);//request a pause
				mrStage.addSubTask(mCurAddr, 0, nCount + 1, true);//a hint to remove last op
				mrStage.setVA(mCurAddr);
				mrStage.setOp(mpCurOp);
				mpLastOp = pLastOpEx;//recover a prior op (in case of multi-op instruction)
				mrProject.analyzer()->setCurrentFieldRef(stubFault.field());
				//mrStage.setOpField(fieldRef(mpCurOp));
				MyStream ss;
				WriteFuncProfileFromCall(mpCurOp, ss);// mrProject.analyzer());
				guix().GuiOnFunctionCallStop(ss);//GUI will not respond until awaken up the stack
				return 1;
			}
			//assert(0);
		}

		if (pcode.isInvalid())
			break;

		CheckSwitchStatement(mCurAddr);

		const ins_desc_t &desc(codeIt.desc());
		if (desc.isDirect())//no indirect calls?
		{
			if (desc.isJump())//jump - conditional or not
			{
//CHECK(mCurAddr == 0x511f6a)
//STOP
				if (desc.isDirect())
				{
					if (CheckCodeBounds(desc.vaRef))
						mrStage.addSubTask(desc.vaRef, mCurAddr);
				}
			}
			else //if ( mbFunc > 1 )//call
			{
				//pushCommand(new ToDoNode_t(TODO_MAKEFUNC, ADDRX(pSeg, taddr), ADDRX(), nullptr, false));//, ADDRX(pSelf, addr2));
			}
		}
	}

	if (iThunkLevel > 0)
		return 2;

	ResetCurrentPath();
	mpLastOp = HOP();
	return 1;
}

void PCodeTracer_t::CheckSwitchStatement(ADDR curAddr)
{
	if (!mpCurOp)
		return;

//CHECK(curAddr == 0x50bec4)
//STOP
	if (PathType(PathOf(mpCurOp)) != BLK_JMPSWITCH)
		return;

	std::vector<ADDR> o;
	FieldPtr pJumpTable(GetJumpTableEntries(PathOf(mpCurOp), o));
	if (!pJumpTable)
		return;

	ADDR vba(pJumpTable->_key());

	for (size_t i(0); i < o.size(); i++)
	{
		ADDR addrTo(o[i]);
		CHECK(addrTo == 0x50becb)
			STOP
			HOP pIOp(NewPrimeOp());
		ADDR va(vba + (int)i * mrDC.PtrSize());
		SetOpVA(pIOp, va);
		pIOp->Setup4(OPC_ADDRESS | OPC_GLOBAL, OPTYP_PTR32, 0, addrTo);

		SwitchTracer_t anSwitch(*this);//?
		HPATH pDataPath(anSwitch.AssureSwitchTablePath(PathOf(mpCurOp)));//, pJumpTable));
		anSwitch.addIOp(pDataPath, pIOp);

		PathMapIt jj(AssurePathBeginsAt(addrTo, curAddr));
		if (jj != mPathMap.end())
		{
			HPATH hPath(jj->second);
			AddPathRef(hPath, pIOp);
			//FieldPtr pLabel(PathLabel(hPath));
			//if (pLabel && pLabel->type() && pLabel->type()->typeCode())
			{
				ADDR addrFrom(curAddr);
				mrStage.addSubTask(addrTo, addrFrom);
			}
			//else
			//	fprintf(STDERR, "Warning: Invalid label reference in jump table at %s<%d>, index = %d\n", OwnerFuncName().c_str(), 0, i);//?	No(mpCurPath)
		}
	}
}

FieldPtr PCodeTracer_t::GetJumpTableEntries(HPATH pSwitchPath, std::vector<ADDR> &o)
{
	FieldPtr pJumpTable = GetJumpTable(pSwitchPath);
	if (!pJumpTable)
	{
		PrintError() << "Unrecognized jump table at " << OwnerFuncName() << "<" << PathNo(pSwitchPath) << ">" << std::endl;
		return nullptr;
	}

	if (pJumpTable->type())
	{
		Array_t *pTypeArray(pJumpTable->type()->typeArray());
		if (pTypeArray)
		{
			TypePtr iTypePtr(pTypeArray->baseType());
			TypePtr_t *pTypePtr(iTypePtr->typePtr());
			if (pTypePtr)
			{
				ADDR va(pJumpTable->_key());
				if (CheckDataAtVA(miSeg, va))
				{
					const Block_t &block(miSeg->typeSeg()->rawBlock());
					DataSubSource_t data(GetDataSource()->pvt(), block);
					DataStream_t is(data);
					//IStream_t is(GetDataSource()->pvt(), block.mOffs, block.m_size);
					if (is.seek(va - miSeg->base()))
					{
						value_t v;
						int sz(iTypePtr->size());
						o.resize(pTypeArray->total());
						for (size_t i(0); i < pTypeArray->total(); i++)
						{
							if (is.read(sz, (char *)&v.i8) != sz)
							{
								fprintf(STDERR, "Warning: Corrupt jump table at %s<%d>\n", OwnerFuncName().c_str(), PathNo(pSwitchPath));
								o.resize(i);
								break;
							}
							const ADDR &a(*(ADDR *)&v.i8);
							o[i] = a;
						}
						return pJumpTable;
					}
				}
			}
			else
			{
				PrintError() << "Invalid base type for jump table at " << OwnerFuncName() << "<" << PathNo(CurrentPath()) << ">" << std::endl;
			}
		}
	}

	return nullptr;
}

int PCodeTracer_t::SetupFrom(Ins_t &rSelf, const INS_t &r)
{
	rSelf.mAction = (Action_t)r.action;
	rSelf.setPStackDiff(r.pvt.stackdiff);
	rSelf.mEFlagsTested = r.pvt.cpuflags_tested;
	rSelf.mEFlagsModified = r.pvt.cpuflags;
	rSelf.setFStackDiff(r.pvt.fpudiff * FTOP_STEP);
	rSelf.mFFlagsAffected = r.pvt.fpuflags;
	return 1;
}

HPATH PCodeTracer_t::SplitPath(HPATH pSelf, ADDR addr)
{
#ifdef _DEBUG
	int pathno(PathNo(pSelf));
#endif
	HPATH pPathNew(NewPath());
	SetPathType(pPathNew, PathType(pSelf));
	SetPathType(pSelf, BLK_NULL);
	HPATH pParent(pSelf->Parent());
	TreeInsertChildAfter(pParent, pPathNew, pSelf);
	pPathNew->setAnalized(pSelf->isAnalized());

#if(0)
	PathOpList_t &lTo(pPathNew->m.opsRef());
	HOP pOp;
	for (pOp = PRIME(pSelf->m.headOp()); pOp && LowerBound() + No(pOp) < addr; pOp = NextPrime(pOp));
	while (pOp)
	{
		HOP pOp2(pOp);
		pOp = NextPrime(pOp);
		pSelf->takeOp(pOp2);
		LinkOpTail(pPathNew, pOp2);
	}
#else
	PathOpList_t::Iterator i(pSelf->opsRef());
	while (i && LowerBound() + OpNo(i.data()) < addr)
		++i;
	while (i)
	{
		HOP pOp(i.data());
		++i;
		pSelf->takeOp(pOp);
		LinkOpTail(pPathNew, pOp);
	}
#endif

	return pPathNew;
}

PathMapIt PCodeTracer_t::AssurePathBeginsAt(ADDR va, ADDR vaCur)
{
//CHECK(vaCur==0x50d871)
//STOP
CHECK(va==0x10053bd)
STOP

	HPATH pSplitPath = HPATH();
	PathMapIt it(mPathMap.upper_bound(va));
	if (it != mPathMap.begin())
	{
		//there should be a path before the given address
		PathMapIt jt(it);
		HPATH pPath((--jt)->second);//a path va falls into
		if (!pPath)//fall into a gap?
		{
			if (jt->first == va)
				mPathMap.erase(jt);//a path break, indicated by nil value, have to convert it into a real entry
		}
		else
		{
			if (jt->first == va)
				return jt;//a path was already registered at given address
			if (!PathIsEmpty(pPath))
			{
				if (pPath == CurrentPath())
				{//a ref is into an already processed part of a path (split required)
					if (va <= vaCur)//a ref is below a current position (processing a current path)
						pSplitPath = pPath;
					//else a ref below a current position (a new path can be safely inserted)
				}
				else
				{
					pSplitPath = pPath;//a ref is into an already processed part of a path (split required)
				}
			}
		}
	}
	//else a reference to above function's scope (or empty)
	
	HPATH pPath;
	if (pSplitPath)
	{
		pPath = SplitPath(pSplitPath, va);
		assert(pPath);
	}
	else
	{
		PathPtr pPathAfter = PathPtr();
		if (it != mPathMap.end())
			pPathAfter = it->second;
		pPath = InsertPathBefore(va, pPathAfter);
	}

	std::pair<PathMapIt, bool> res;
	res = mPathMap.insert(std::make_pair(va, pPath));
	assert(res.second);

	if (pSplitPath && pSplitPath == CurrentPath())
		SetCurrent(res.first);

	assert(pPath);
	return res.first;
}

HPATH PCodeTracer_t::CheckPathChangeAt(HPATH pPath, ADDR va)
{
	PathMap &m(mPathMap);
	return HPATH();
}

HPATH PCodeTracer_t::InsertPathAfter(HPATH pPathAfter)
{
	assert(pPathAfter);
	HPATH pPath(NewPath());
	TreeInsertChildAfter(HPATH(mrFuncDef.Body()), pPath, pPathAfter);
	//pPath->SetParent(Body());
	return pPath;
}

int PCodeTracer_t::SetupOpFrom(HOP hSelf, const OPND_t &r, ADDR vaCur, int stackPtrShift)
{
	//if (!p)
		//return 0;

	Op_t &rSelf(*hSelf);

//	if (r.opc() == OPC_CPUREG)
//		assert(!(r.opid() & 0xF0));//no complex regs
	if (r.ssid() == OPC_CPUSW)
		if (r.opid())
			if (IsRhsOp(hSelf))
			{
				InsRefPrime(hSelf).mEFlagsTested = r.opid();
				((OPND_t &)r).m_opid = 0;//???
			}

	rSelf.m_opc = (r.opc() & 0xFF);
	rSelf.mOffs = (r.offs() & 0xFF);
	rSelf.m_optyp = (r.OpType() & 0xFF);
	assert(r.m_dptr == 0);//?m_nPtr = r.m_dptr;
	//rSelf.SetOpSeg((r.opc() & OPC_INDIRECT) >> 4);// r.opseg);
	rSelf.m_disp = r.disp;

	if (r.ssid() == mrFE.stack_ptr->ssid)
		if (r.offs() == mrFE.stack_ptr->ofs)
			rSelf.m_disp += stackPtrShift;
			

	if (ActionOf(hSelf) == ACTN_RET)
	{//opc:1,optyp:0x84//it was
		assert(IsPrimeOp(hSelf));

//		rSelf.m_opc = OPC_ADDRESS|OPC_GLOBAL;
//		rSelf.SetOpType(OPTYP_NULL);//OPTYP _PTR|G DC.PtrSize());
//		rSelf.ins().addPStackDiff(rSelf.m_disp);
		assert(rSelf.m_disp == 0);

//		rSelf.setAction(ACTN_GOTO);
//		rSelf.SetOpSeg(OPSEG_CODE);
		rSelf.SetOpSeg(OPSEG_STACK);

		AddPathRef(CreateTailPath(), hSelf);
		return 1;
	}

	if (rSelf.SSID() == SSID_GLOBAL && !rSelf.IsScalar())
	{
//		rSelf.mOffs = 0;
//		rSelf.m_disp = r.offs();
		if (!r.name.empty())
		{
CHECKID(hSelf, 0x38b2)
STOP
			size_t n(findIntrinsic(r.name.c_str()));
			assert(n != -1);
			//rSelf.setAction(ACTN_INTRINSIC);
			rSelf.m_disp = (int32_t)(n + 1);//1-biased
			rSelf.ins().opnd_INTRINSIC = true;
		}
		else
		{

			if (IsGoto(hSelf) || IsCondJump(hSelf))
			{
/*				PathMapIt jj(AssurePathBeginsAt(rSelf.m_disp));
				HPATH pPath(jj->second);
				//if (!pField->typeCode())
				//return 0;//?
				//HPATH h(Lab elPath(pField));
				AddPathRef(pPath, hSelf);*/
			}
			else if (rSelf.m_disp != 0)
			{
				FieldPtr pField(FindGlobalAtVA(rSelf.m_disp, FieldIt_Prev, false));//shallow
				
				//if (!pField)
					//pField = Field(OwnerFuncRef(), rSelf.m_disp, nullptr, FieldIt_Prev);

				if (pField)
				{
					if (IsEntryLabel(pField))
						pField = pField->owner()->parentField();//this is a func

					//a global var reference - if there is a field, make sure it is in src file
					GlobPtr pGlob(GlobObj(pField));
					if (!pGlob && !pField->isTypeCodeEx())//no codes no thunks
					{
						assert(!pField->ownerProc());

						bool bAddFolder(true);
						Folder_t *pFolder(nullptr);
						if (IsTypeImp(pField))
						{
							pFolder = AddFileEx(FPATH_FROM_IMPORTED);
							//?Folder_t *pImpFolder(ImpToExpFieldInfo(OwnerSeg(OwnerFuncRef()), pField->address()));
						}
						else if (pField->type() && !IsProc(pField->type()))
						{
							OFF_t off;
							if (CheckThruConst(pField, off))
								bAddFolder = false;
							/*CTypePtr pSeg(DcInfo_t::OwnerSeg(pField->owner()));
							if (pSeg->typeSeg()->isReadOnly())
								pFolder = AddFileEx(nullptr, FTYP_CONST);*/
						}

						if (bAddFolder)
						{
							if (!pFolder)
								pFolder = AddFileEx(nullptr, FTYP_STUBS);
							FieldExPtr pFieldx(AddGlobToFile2(pField, pFolder));
							mrFileDef.addIncludeList(pFolder);
						}
					}
				}
				else
					PrintWarning() << "Addressing outside of segment range @VA=" << VA2STR(vaCur) << " " << std::hex << "(" << rSelf.m_disp << ")" << std::endl;
			}
		}
	}
	/*else if (0)
	{
		assert(r.disp != -1);
		FieldPtr pMLoc = 0;
		if (rSelf.m_opc & OPC_A DDRESS)
		{
			assert(rSelf.m_opc == (OPC_A DDRESS|OPC_ GLOBAL));

			if (rSelf.IsCall())
				pMLoc = mrDC.Register Ref(r.disp, OBJID_TYPE_PROC);
			else if (rSelf.IsGoto() || rSelf.IsCondJump())
				pMLoc = mrDC.Register Ref(r.disp, OBJID_FIELD);
			else
				pMLoc = mrDC.Register Ref(r.disp, OBJID_UNK);
		}
		else
		{
			pMLoc = mrDC.Register Ref(r.disp, OBJID_FIELD);
		}
		
		assert(pMLoc);
		AddXRef(pMLoc, &rSelf);
	}*/

	if ((rSelf.OpC() & OPC_INDIRECT) == OPC_INDIRECT)
	{
		if (rSelf.SSID() == SSID_CPUREG)
		{
			if (IsStackPtrB(hSelf) || (rSelf.OpOffsU() == mrFE.stackb_ptr->ofs))//what about aux?
			{
				//rSelf.m_opc &= 0xF;
				//rSelf.m_opc |= (OPSEG_STACK << 4);
				rSelf.SetOpSeg(OPSEG_STACK);
			}
			else//DS by default
			{
				rSelf.SetOpSeg(OPSEG_DATA);
			}
		}
#ifdef OLD_FIELDREF
#if (0)
		if (rSelf.OpSeg() == OPSEG_NULL0 || rSelf.OpSeg() == OPSEG_DATA || rSelf.OpSeg() == OPSEG_CODE)//assume - data seg
		{
			if (!fieldRef(&rSelf))
			{
				Locus_t loc;
				FieldPtr pField(PrimeSeg()->FindFieldInSubsegs(rSelf.m_disp, loc));//check global
				if (pField)
				{
					AddOp Ref(pField, &rSelf);
				}
			}
			else
			{
				//?
			}
		}
#endif
#endif
	}
	else if (rSelf.IsScalar())
	{
		if (rSelf.OpOffs() == OPID_0)
		{
			rSelf.m_disp = 0;
			rSelf.SetOpId(0);
		}
		else if (rSelf.OpOffs() == OPID_1)
		{
/*			if (rSelf.m_optyp == OPTYP_REAL32)
				r32 = 1.0f;
			else if (rSelf.m_optyp == OPTYP_REAL64)
				r64 = 1.0;
			else*/
				rSelf.m_disp = 1;
			rSelf.SetOpId(0);
		}
	}

	if (rSelf.OpC() == OPC_FPUREG )
	{
		Ins_t &rRI = InsRefPrime(hSelf);
		rSelf.m_opc = OPC_INDIRECT | SSID_FPUSW;
		if (rRI.FStackDiff() < 0 )//load
		{
			rSelf.m_disp = rSelf.mOffs / FR_SLOT_SIZE;
			if (IsPrimeOp(hSelf) )
				rSelf.m_disp += rRI.FStackDiff();
		}
		else
			rSelf.m_disp = rRI.mFStackIn + rSelf.mOffs / FR_SLOT_SIZE;
		rSelf.mOffs = OFS(R_FPUSW_TOP);
	}

	return 1;
}

PathMapIt PCodeTracer_t::GetPathAt(ADDR addr) const
{
	return mPathMap.find(addr);
}

HPATH PCodeTracer_t::InsertPathBefore(ADDR addr, PathPtr pPathNext)
{
	PathPtr pPath(NewPath());
	if (pPathNext)
		TreeInsertChildBefore(mrFuncDef.Body(), pPath, pPathNext);
	else
	{
		PathPtr pPathTail(mrPathTree.pathTail());
		if (pPathTail)
			TreeInsertChildBefore(mrFuncDef.Body(), pPath, pPathTail);//insert right before tail
		else
			TreePushChildBack(mrFuncDef.Body(), pPath);//just append
	}
	
	return pPath;
}

int PCodeTracer_t::SetOps(const INS_t &anIns, ADDR vaCur, int stackPtrShift)
{
	//bool bNewPath = false;

//CHECK(mCurOffs == 219)
CHECK(vaCur == 0x512055)
STOP

	assert(CurrentPath());

	mpCurOp = NewPrimeOp();

	LinkOpTail(CurrentPath(), mpCurOp);

	if (!SetupFrom(mpCurOp->ins(), anIns))
		return 0;

	lstOPND_cit it_op(anIns.ops.begin());
	if (it_op == anIns.ops.end())
		return 0;

	// primary op
	if (!SetupOpFrom(mpCurOp, anIns.ops.front(), vaCur, stackPtrShift))
		return 0;

//CHECK(mCurAddr == 0x50bec4)
//STOP

	if (anIns.action == ACTN_INVALID)
		return -1;

	if (anIns.action == ACTN_RET)
	{
		if (++it_op != anIns.ops.end())//one more?
		{
			const OPND_t& r(*it_op);
			mpCurOp->ins().addPStackDiff(r.disp);
		}
		mpCurOp->setAction(ACTN_GOTO);
	}
	else
	{
		// extra ops
		for (++it_op; it_op != anIns.ops.end(); ++it_op)
		{
			const OPND_t& r(*it_op);

			HOP pOpIn(NewOp());
			AddOpArg(mpCurOp, pOpIn);

			/*?			if (anIns.fpudiff < 0)//load
			if (pArg->opc() == OPC_FPUREG)
			pArg->mOffs -= anIns.fpudiff * OPSZ_QWORD;
			*/
			if (!SetupOpFrom(pOpIn, r, vaCur, stackPtrShift))
				return 0;
		}
	}

	return 1;
}

BlockTyp_t PCodeTracer_t::FinalizePath(HPATH pPath)
{
	BlockTyp_t blk(CheckPathBreak(mpCurOp));
	if (blk != BLK_NULL)
	{
		SetPathType(pPath, blk);
		BranchTracer_t an(*this);
		an.AdjustGoto(pPath);
		an.AdjustGoto2(pPath);
	}
	return blk;
}











/*TypePtr GenerateStage_t::RegisterFunc(const char * pszName, bool bFar)
{
	//	Fil e_t *pFile = 0;

	assert(!getCurFunc());//func being processed already opened?!!
	//	pFile = m_pCurFile;

	TypeProc_t * pFunc = nullptr;
	FieldPtr  f = G DC.FindM Loc0(pszName);
	TypePtr tidFunc;
	if (!f)
	{
		//FieldPtr  pField = new Fiel d_t();//func fld
		//GetCurSeg()->AddObj(pField, BAD_ADDR);
		assert(0);
		TypePtr tidSeg(0);//GetCurSeg());
		StrucModifier_t an;
		FieldPtr  pField = an.AppendFieldOfType(tidSeg, pszName, nullptr);
		//pField->Set Name(pszName);
		mrFilesMgr.AddOb jToContextFile(pField);

		pFunc = new TypeProc_t();
		tidFunc = G_Me mMgr.NewTypeRef(pFunc);
		tidFunc->setParentField(pField);
		//?pFunc->SetBase(pField->address());
		pFunc->Assu reFuncDef();
		if (bFar)
			pFunc->f uncdef()->flags() |= FDEF_FAR;

		pField->setT ype(tidFunc);
		//		pFunc->AddO bjRef(pField);
		Func Info_t an2(*pFunc);
		f = an2.EnsureHeadLabel();
	}
	else
	{
		f = f->AssureObjType(OBJID_FIELD);
		pFunc = f->type()->type Proc();//m_pStruc;//Func;

		//check for func with such a name DECOMP status
		if ((pFunc->func def()->flags() & FDEF_STATUS_MASK) > FDEF_DEFINED)
		{
			//			Display_t::PutGoToInfo(DID_FUNCBEG, pFunc);
			fprintf(stderr, "Fatal Error: The function <%s> has been processed already\n", pFunc->ty peobj()->na mexx().c_str());
		}
		//hope no more funcs with requested name
		mrFilesMgr.AddObjTo ContextFile(pFunc->typ eobj());
	}

	return tidFunc;
}*/

/*FieldPtr  GenerateStage_t::RegisterGlobal(const char * pszName, ObjId_t objID)
{
	assert(pszName);
	assert(objID == OBJID_FIELD);

	//CHECK(strcmp(pszName, "loc_50BF4F") == 0)
	//STOP

	assert(0);
	TypePtr tid(0);//?(GetCurSeg());
	TypePtr pOwnerStruc = tid;
	if (!pOwnerStruc)
		fprintf(stderr, "Fatal Error: No segment specified\n");

	if (getCurFunc())
		pOwnerStruc = getCurFunc();

	FieldPtr  pField = G DC.FindM Loc0(pszName);

	if (!pField)
	{
		//?pField = Obj_t::Crea teObj(objID)->objField();
		//?pOwnerStruc->AddF ield(pField, GetCurAddr());
		//pField->Se tName(pszName);
		StrucModifier_t an;
		pField = an.AppendFieldOfType(pOwnerStruc, pszName, nullptr);
	}
	else
	{
		pField = pField->AssureObjType(objID);
		//pOwnerStruc->AddF ield(pField, GetCurAddr());
		StrucModifier_t an;
		an.appendField(pOwnerStruc, pField);
	}

	return pField;
}*/
















