#include "anlzbin.h"
#include "prefix.h"
#include <algorithm>
#include "shared/front.h"
#include "shared/data_source.h"
#include "field.h"
#include "type_proc.h"
#include "proj.h"
#include "type_code.h"
#include "types_mgr.h"
#include "info_types.h"
#include "main.h"
#include "names.h"
#include "ui_main.h"
#include "clean.h"

#define NO_FUNCS	0

/////////////////////////CodeIterator0

CodeStream_t::CodeStream_t(const I_DataSourceBase &rData, CTypePtr pSeg, ADDR va)
	: DataStream_t(rData),
	//m_data(pData->pvt(), pSeg),
	//mpSeg(iSeg),
	m_imageBase(pSeg->imageBase()),
	m_base(pSeg->base()),
	mbLarge(pSeg->typeSeg()->isLarge()),
	m_addr(va)
{
	assert(m_addr >= m_base);
	seek(m_addr - m_base);
//	if (operator bool())
	//	operator++();
}

CodeStream_t::CodeStream_t(const I_DataSourceBase &rData, ADDR64 imageBase, ADDR base, ADDR va, bool isLarge)
	: DataStream_t(rData),
	m_imageBase(imageBase),
	m_base(base),
	mbLarge(isLarge),
	m_addr(va)
{
	assert(m_addr >= m_base);
	seek(m_addr - m_base);
//	if (operator bool())
	//	operator++();
}


/////////////////////////CodeIterator

CodeIterator::CodeIterator(const I_DataSourceBase &rData, CTypePtr piCode, CTypePtr pSeg, ADDR va)
	: CodeStream_t(rData, pSeg, va),
	miTypeCode(piCode)
{
}

bool CodeIterator::unassemble()
{
	if (operator bool())
	{
		updateAddr();
		if (miTypeCode && miTypeCode->typeCode()->unassemble(*this, m_imageBase, m_addr, m_desc) > 0)
		{
			if (m_desc.isFinalized())
				seek(OFF_NULL);
			return true;
		}
		seek(OFF_NULL);
	}
	return false;
}

/////////////////////////CodeIterator1

CodeIterator1::CodeIterator1(const I_DataSourceBase &rData, CFieldPtr pField)
	: CodeIterator(rData, pField->isTypeCodeEx(), ProjectInfo_s::OwnerSeg(pField->owner()), pField->_key())
{
}



///////////////////////////////////////////////////////// (BinaryAnalyzer_t)

BinaryAnalyzer_t::BinaryAnalyzer_t(Project_t &rProjRef, TypePtr pModule, unsigned segAffinity)
	: mrProject(rProjRef),
	mpModule(pModule),
	mbFunc(0),
	funcs_num(0),
	mCurVA(0),
	mSegAffinity(segAffinity)
{
}

BinaryAnalyzer_t::~BinaryAnalyzer_t()
{
	if (!mToDoList.empty())
	{
		mToDoList.cleanup();
		/*do {
			Command *pTask(mToDoList.front());
			mToDoList.pop_front();
			delete pTask;
		} while (!mToDoList.empty());*/
		mrProject.markDirty(DIRTY_TASKLIST);
	}

	if ( mbFunc > 1 )
		fprintf(stdout, "%d functions have been processed.\n", funcs_num);
//	else
	//	fprintf(stdout, "Done.\n");
	//fflush(nullptr);

//?	gui().GuiOnAnalizerStopped();

//?	mrMain.setAnalizer(nullptr);
	//?SetStatus(0);
}

MemoryMgr_t &BinaryAnalyzer_t::memMgr() const
{
	return mrProject.memMgr();
}

int BinaryAnalyzer_t::isArrayTo(TypePtr iType, OpType_t optyp) const
{
	ModuleInfo_t MI(mrProject, *mpModule);
	if (iType && iType->typeArray() && iType->typeArray()->baseType() == MI.GetStockType(optyp))
		return true;
	return false;
}

TypePtr BinaryAnalyzer_t::checkVA(ADDR va, TypePtr pSeg)
{
	ModuleInfo_t MI(mrProject, *mpModule);
	TypePtr pSeg2(MI.FindSegAt(pSeg, va));
	if (!pSeg2)
	{
		pSeg2 = pSeg->typeSeg()->ownerRangeSet(pSeg);
		if (!pSeg2)
			return pSeg;
		pSeg2 = MI.FindSegAt(pSeg2, va);
	}
	return pSeg2;
}


void BinaryAnalyzer_t::COMMAND_continue(BinaryAnalyzer_t *, int, char *[])
{
}

void BinaryAnalyzer_t::COMMAND_pause(BinaryAnalyzer_t *, int, char *[])
{
}

void BinaryAnalyzer_t::COMMAND_abort(BinaryAnalyzer_t *, int, char *[])
{
}

const Frame_t &BinaryAnalyzer_t::frameToDelete(const Locus_t &rLoc)
{
	assert(!rLoc.empty());
	for (std::list<Frame_t>::const_reverse_iterator i(rLoc.rbegin()); i != rLoc.rend(); i++)
	{
		const Frame_t &f(*i);
		if (!f.cont()->isShared())
			return f;
	}
	return rLoc.back();
}

bool BinaryAnalyzer_t::pushTask(TypePtr pContext, ADDR addr, const char* str)
{
//CHECK(va == 0xF9cb5)
//STOP
	if (!mToDoList.pushTaskBack(pContext, addr, str))
		return false;
	//fprintf(stdout, "*%X: %s\n", addr, str);
	mrProject.markDirty(DIRTY_TASKLIST);
	return true;
}

int BinaryAnalyzer_t::process()
{
	assert(!mToDoList.empty());
	Task_t node(mToDoList.popTaskFront());
	/*if (!pNode)
	{
		fprintf(STDERR, "Internal Error: nullptr command called.\n");
		return 1;
	}*/

	mrProject.markDirty(DIRTY_TASKLIST);

	Locus_t aLoc;

	FolderPtr pModuleFolder(nullptr);

//CHECK(sAt=="38070")
//CHECK(node.va == 0x1006420)
//STOP
	//got a VA, look in itself or sibling segments
	ProjectInfo_t PI(mrProject);
	ADDR va;
	MyString orig(node.cmd);
	if (!node.extractVA(va))//will modify node.cmd
	{
		PI.PrintError() << "Malformed command: " << orig << std::endl;
		return 0;
	}
	TypePtr iSeg1(PI.VA2Locus(checkVA(va, node.seg), va, aLoc));
	if (!iSeg1)
	{
		ROWID da(va);
		pModuleFolder = PI.LocusFromDA(da, aLoc);
		if (!pModuleFolder)
		{
			PI.PrintError() << "Invalid target address in command: " << orig << std::endl;
			return 0;
		}
	}

	adc::CEventCommand e(node.cmd);//the 'at' key has been used above

	e.setContextZ(new Probe_t(aLoc))->Release();//the context of no used here
	assert(e.contextZ()->RefsNum() == 1);
	e.m_autodelete = false;
	e.m_bEcho = false;
	//int modified = mrMain.callEvent(&e);
	return MAIN.processEvent2(&e);
}






//************************************************************* (ProjModifier_t)


ProjModifier_t::ProjModifier_t(const ModuleInfo_t& o)
	: ModuleInfo_t(o)
{
}

ProjModifier_t::ProjModifier_t(const ModuleInfo_t& o, MemoryMgr_t& memMgr)
	: ModuleInfo_t(o, memMgr)
{
}

ProjModifier_t::ProjModifier_t(Project_t& rProject, TypeObj_t& rBinary)
	: ModuleInfo_t(rProject, rBinary)
{
}

int ProjModifier_t::ForwardCommand(const char *cmd, DataStream_t &aRaw, TypePtr pType, TypePtr pSeg)
{
	int count(0);
	if (!pType)
	{
		aRaw.skip(OPSZ_BYTE);
	}
	else if (pType->typeArray())
	{
		Array_t* pArrayPvt(pType->typeArray());
		for (unsigned n(0); n < pArrayPvt->total(); n++)
			count += ForwardCommand(cmd, aRaw, pArrayPvt->baseType(), pSeg);
	}
	else if (pType->typeStruc())
	{
		const FieldMap& m(pType->typeStruc()->fields());
		OFF_t oBeg(aRaw.current());
		for (FieldMapCIt i(m.begin()); i != m.end(); ++i)
		{
			CFieldPtr pField(VALUE(i));
			assert(oBeg + pField->offset() >= aRaw.current());
			OFF_t oGap(oBeg + pField->offset() - aRaw.current());
			if (oGap > 0)
				aRaw.skip((size_t)oGap);
			count += ForwardCommand(cmd, aRaw, pField->type(), pSeg);
		}
	}
	else if (pType->typePtr())
	{
		VALUE_t v;
		v.typ = (uint8_t)aRaw.read(pType->size(), (PDATA)&v);
		ADDR va;
		if (GetVAfromValue(v, pSeg, va))
		{
			Locus_t aLoc;
			LocusFromVA(pSeg, va, aLoc, false);
			//MyString s(MyStringf("%s -@%X", cmd, va));
			//mrMain.postEvent(new adc::CEventCommand(s, false));
			if (MakeCode(aLoc, false) == RESULT_OK)
			{
				SweepCode(aLoc.field0(), -1);
				count++;
			}
		}
	}
	else
	{
		aRaw.skip(pType->size());
	}
	return count;
}

int ProjModifier_t::ForwardCommand(const char *cmd, CFieldPtr pField)//thru indirect referencing
{
	TypePtr pSeg(OwnerSeg(pField->owner()));
	if (!CheckDataAtVA(pSeg, pField->_key()))
		return 0;
	DataSubSource_t data(GetDataSource()->pvt(), pSeg->typeSeg()->rawBlock());
	DataStream_t aRaw(data, pField->offset());
	return ForwardCommand(cmd, aRaw, pField->type(), pSeg);
}

int ProjModifier_t::MakeGap(Probe_t &ctx, bool)
{
	TypePtr iStruc(ctx.locus().struc());
	if (!iStruc)
		return 0;

	Frame_t &fr(ctx.locus().back());
	Struc_t &rStruc(*iStruc->typeStruc());
	bool bEmpty(!rStruc.hasFields());
	if (bEmpty || (fr.field() && VALUE(rStruc.fields().rbegin()) == fr.field()))
	{
		int iSize(iStruc->size());

		if (!ResizeStruc(fr.cont(), iSize + 1, ctx.locus().upframe(1), true))
			return 0;
		if (bEmpty && ctx.pickedFieldDecl())
		{
			ctx.resetFieldDecl();//this will shift curpos 1 line down
		}
		else if (fr.field())
		{
			fr.setAddr(fr.addr() + fr.field()->size());
			fr.setField(nullptr);
		}
		mrProject.markDirty(DIRTY_LOCUS_ADJUSTED);
		mrProject.markDirty(DIRTY_GLOBALS);
		mrProject.OnDataEdited(ModulePtr());
		return 1;
	}
	return 0;
}

RESULT_e ProjModifier_t::MakeCode(Locus_t& aLoc, bool bForce)
{
CHECK(aLoc.va() == 0x1006420)
STOP
	TypePtr iSelf(aLoc.struc());
	if (!iSelf)
		return RESULT_FAILED;

	Struc_t& rSelf = (*iSelf->typeStruc());
	ADDR addr0(aLoc.addr());

	FieldMap& m(rSelf.fields());

	if (aLoc.field0())
	{
		FieldMapIt it0(m.lower_bound(aLoc.field0()->_key()));
		if (aLoc.field0() && VALUE(it0) != aLoc.field0())
			aLoc.setField(VALUE(it0));
	}

	TypePtr iSeg(iSelf);
	if (!iSeg->typeSeg())
		iSeg = OwnerSeg0(iSeg);
	if (!iSeg)
		return RESULT_FAILED;

	Seg_t& rSeg(*iSeg->typeSeg());

	TypePtr iSegList(OwnerSegList(iSeg));
	if (!iSegList)
		return RESULT_FAILED;

	TypesMgr_t* pTypeMgr = findTypeMgr(iSelf);
	if (!pTypeMgr)
		return RESULT_FAILED;

//CHECK(addr0 == 0x1006495)
CHECK(rSeg.addr64(addr0, iSeg) == 0x63f0bee8)
STOP

	FieldMapIt it(FieldIt(iSelf, addr0, nullptr, FieldIt_Overlap));

	if (!CheckStrucEnd(it, m))//? && it->first == addr0 )
	{
		ADDR fa = KEY(it);
		FieldPtr  f = VALUE(it);

		if (f->type())
		{
			if (f->_key() < addr0)
			{/*
				TypeProc_t* pFunc(f->type()->type Proc());
				if (pFunc)
				{
					//adding code to function
					assert(0);
					//it = pFunc->insertf_it(addr0, nullptr, pTypeMgr->getTypeCode());
					//if (it == pFunc->fields().end())
					//	return 0;
		//?			it = pFunc->fields().end();
			//?		pSelf = pFunc;
				//?	iSelf = f->type();
				}*/
			}
			else
			{
				if (f->type()->typeCode())
				{
					if (!bForce)
						return RESULT_ALREADY;
				}
				else
				{
					f->convertObjType(pTypeMgr->getTypeCode());
					if (!f->isTypeCode())
					{
						if (!f->isTypeThunk())
							return RESULT_FAILED;//could not convert
						if (!bForce)
							return RESULT_FAILED;
						BinaryCleaner_t<> BC(*this);
						BC.ClearType(f);
					}
				}
			}
		}
	}

	if (CheckStrucEnd(it, m))
	{
		it = InsertFieldIt(aLoc.back());// frame_t(iSelf, addr0));
		if (CheckStrucEnd(it, m))
			return RESULT_FAILED;

		if (!SetTypeEx(aLoc.field0(), pTypeMgr->getTypeCode()))
			return RESULT_FAILED;
	}

	FieldPtr f(VALUE(it));

	if (!f->type())
		SetType(f, pTypeMgr->getTypeCode());

	if (!f->type() || !f->isTypeCode())
	{
		mrProject.markDirty(DIRTY_GLOBALS);//a field might have been inserted
		return RESULT_FAILED;
	}

	aLoc.setField(f);
	mrProject.markDirty(DIRTY_GLOBALS);
	return RESULT_OK;
}

ADDR ProjModifier_t::SweepCode(CFieldPtr pField, int depth, unsigned flags)
{
	if (!pField->isTypeCode())
		if (!pField->isTypeThunk())
			return pField->_key();
	CTypePtr pSelf(pField->owner());
	CTypePtr pSeg(OwnerSeg0(pSelf));
	assert(pSeg);
	const Seg_t& rSeg(*pSeg->typeSeg());
	const Struc_t& rSelf(*pSelf->typeStruc());

	ADDR addr_start(pField->_key());
CHECK(addr_start == 0x0014c920)
STOP
	ADDR end_addr(addr_start);
	BinaryAnalyzer_t *pAnalizer(StartBinaryAnalizer(ModulePtr(), rSeg.affinity()));
	if (!pAnalizer)
		return end_addr;

	const FieldMap& m(rSelf.fields());
	FieldMapCIt it(pField);

	ADDR addr_end(-1);
	FieldMapCIt it_nx(EosAwareNext(m, it));

	if (!CheckStrucEnd(it_nx, m))
		addr_end = KEY(it_nx);

	pAnalizer->setCurrentVA(pField->_key());
	mrProject.markDirty(DIRTY_CURVA);
#if(0)
fprintf(stdout, "VA=%08X\n", addr0);
#endif
//Sleep(0);
	/*if (pAnalizer->mAtNotifier.elapsed() > 250)
	{
		gui().GuiOnShowAnalizerInfo(MyStringf("VA: %X", addr0));
		pAnalizer->mAtNotifier.restart();
	}*/

//CHECK(addr0 == 0x2089f386)
//STOP


	ADDR64 vaBase(rSeg.imageBase(pSeg));
	ADDR64 vaStart(addr_start + vaBase);
CHECK(vaStart == 0x605c7a)
STOP
	ADDR nva(0);
	int pushedCount(0);
	//ADDR addr = addr0;
	DataSourcePane2_t data(GetDataSource()->pvt(), pSeg);
	CodeIterator1 codeIt(data, pField);
	int iThunk(0);//-1:not a thunk; 0:not tested; 1:simple thunk; 2+:complex thunk
	while (codeIt.unassemble())
	{
		nva = codeIt.nextAddr();
		if (nva > addr_end)
			break;//next label reached

		ADDR addrRef(codeIt.addr());
		end_addr = nva;// codeIt.nextAddr();

//CHECK(addrRef == 0x2089f385)//6
CHECK(codeIt.addr() == 0x24feb)
STOP
//if (addrRef > 0x10064A3)
//fprintf(stderr, "%X\n", addrRef); fflush(stderr);


		const ins_desc_t &desc(codeIt.desc());
		ADDR64 ibase(desc.imageBase);

		//check for possible thunk
		if (iThunk >= 0)
		{
			if (desc.controlTransfer)
			{
				if (desc.isUnconditionalJump())
					iThunk++;
				else
					iThunk = -1;//not a thunk
			}
			else
				iThunk++;
		}
	
		if (desc.controlTransfer != 0 && !desc.indirectAccess)//ins.is_direct())
		{
			ADDR taddr(desc.vaRef);
CHECK(taddr == 0x1d9880)
STOP
CHECK(taddr == 0x1d98bd)
STOP
			if (desc.controlTransfer == 1)//any jmp
			{
				//check if there is already a code (or something but not a func) at the target address
				Locus_t aLoc2;//target location
				TypePtr iSeg2;
				if ((iSeg2 = VA2Locus(pSeg, taddr, aLoc2, false)) != nullptr)
					if (aLoc2.field0() && aLoc2.field0()->type())
						if (aLoc2.field0()->isTypeCode() || !aLoc2.field0()->isTypeProc())
							iSeg2 = nullptr;

				if (iSeg2)
				{
					//use block starting address as a ref, because of size-relaxed funcs
					std::ostringstream ss;
					/*if (pField->isTypeThunk() && (flags & SweepCode_Call))
					{
						ss << "makefunc -trythunk -defer";//make func but try a thunk first; defer makefunc until post-anal (not sure if this should be a func)
					}
					else*/
						ss << "makecode";
					ss << " -@" << std::hex << ibase + taddr << " -ref " << ibase + addr_start << std::dec << " -d " << depth;
					if (pAnalizer->pushTask(iSeg2, taddr, ss.str().c_str()))
						pushedCount++;

					if (pField->isTypeThunk() && (flags & SweepCode_Call))//this should be deferred to avoid false positives
					{
						//make func but try a thunk first
						std::ostringstream ss;
						ss << "makefunc -weak -trythunk -@" << std::hex << ibase + taddr << std::dec << " -d " << depth;//weak: do not split an existing func
						if (flags & SweepCode_NamePpg)
							ss << " -nppg";
						BinaryAnalyzer_t* pAnalizer0(FindBinaryAnalizer());
						if (pAnalizer0->pushTask(iSeg2, taddr, ss.str().c_str()))//defer makefunc until post-anal (not sure if this should be a func)
							pushedCount++;
					}
				}
			}
			else //if ( mbFunc > 1 )//call
			{
				if (taddr != nva)//special case to PIC support
				if (depth != 0)
				{
					Locus_t aLoc2;//target location
					TypePtr iSeg2;
					if ((iSeg2 = VA2Locus(pSeg, taddr, aLoc2, false)) != nullptr)
						if (aLoc2.asProc())
							iSeg2 = nullptr;

					if (iSeg2)
					{
						std::ostringstream ss;
						ss << "makefunc -trythunk -@" << std::hex << ibase + taddr << std::dec;//make func but try a thunk first
						if (depth > 0)
							ss << " -d << " << depth - 1;
						if (pAnalizer->pushTask(iSeg2, taddr, ss.str().c_str()))
							pushedCount++;
					}
				}
			}
		}
		else//!ins.is_direct()
		{
			if (desc.indirectAccess)
			{
				ADDR va(desc.vaRef);
#if(0)//jump tables?
				if (!ins.ops[j].rbase.empty())
					continue;//cannot map to global var
#endif
//CHECK(va==0x1005abf8)
CHECK(va==0x00205bfa)
STOP
				//check if there is already a code (or something but not a func) at the target address
				Locus_t aLoc2;//target location
				TypePtr iSeg2;
				if ((iSeg2 = VA2Locus(pSeg, va, aLoc2, false)) != nullptr)
					if (aLoc2.field0())
					{
						//if (!aLoc2.field()->name() && (flags & SweepCode_NamePpg));//steal a name from thunk
						if (aLoc2.field0()->type())//already typed
							iSeg2 = nullptr;
					}
								
				if (iSeg2)// && IsTerminalSeg(iSeg2))
				{
					int typeId(desc.opSize & OPSZ_MASK);//((int)ins.ops[j].opsize);
					if (typeId > OPSZ_MASK)
						typeId = OPSZ_BYTE;//???

					std::ostringstream ss;
					ss << "makedata -@" << std::hex << ibase + va << std::dec << " " << typeId;
#ifdef _DEBUG
//					ss << std::hex << " -ref " << desc.vaCur64() << std::dec;
#endif
					if (pAnalizer->pushTask(iSeg2, va, ss.str().c_str()))
						pushedCount++;
				}
			}
		}
	}
	
#if(1)
	if (pushedCount >= 1000)
		fprintf(stdout, "PUSHED: %d at %08X\n", pushedCount, addr_start);
#endif

	return end_addr;
}

bool ProjModifier_t::TriggerCodeSweep(TypePtr pSeg, ADDR va, const char* optStr)
{
	const Seg_t& rSeg(*pSeg->typeSeg());
	BinaryAnalyzer_t *pAnalizer(StartBinaryAnalizer(ModulePtr(), rSeg.affinity()));

	ADDR64 va64(rSeg.addr64(va, pSeg));
CHECK(va == 0x1d98bd)
STOP
	
	std::ostringstream ss;
	ss << "makefunc -sweep -@" << std::hex << va64 << std::dec;

	if (optStr[0] != 0 && !isspace(optStr[0]))
		ss << " " << optStr;

	//int depth(mrMain.mrSturtupInfo.bFastDisasm ? 1 : -1);
	int depth(mrMain.options().nCallDepth);
	if (depth > 0)
		ss << " -d %d" << depth - 1;

	return pAnalizer->pushTask(pSeg, va, ss.str().c_str());
	//MI.SweepProc(aLoc.field(), depth);
}

ADDR ProjModifier_t::SweepThunk(CFieldPtr pField, int depth, unsigned flags)
{
	return SweepCode(pField, depth, flags | SweepCode_Call);
}

int ProjModifier_t::CheckThunk(CFieldPtr pField, ADDR& tva) const
{
	if (!pField->isTypeCode())
		return 0;
	CTypePtr pSelf(pField->owner());
	CTypePtr pSeg(OwnerSeg0(pSelf));
	assert(pSeg);

	DataSourcePane2_t data(GetDataSource()->pvt(), pSeg);
	CodeIterator1 codeIt(data, pField);
	while (codeIt.unassemble())
	{
		const ins_desc_t &desc(codeIt.desc());
		if (desc.controlTransfer)
		{
			if (desc.isUnconditionalJump())
			{
				tva = desc.vaRef;
				return 1;
			}
			break;
		}
	}

	return 0;
}

int ProjModifier_t::SplitFunction(Locus_t &aLoc, bool bWeak)
{
	FieldPtr pField0(aLoc.field0());
	TypePtr iSelf(aLoc.struc());
	Struc_t * pSelf = iSelf->typeStruc();
	ADDR addr_beg = aLoc.addr();

//CHECK(addr_beg == 0x401350)
//STOP

	if (pSelf->typeSeg())
		return 0;

	//bool bUserAction(mToDoList.empty());//?
	if (!pSelf->typeProc())
		return 0;//?
	if (bWeak)
		return -1;//just a test

	FieldPtr pParent(iSelf->parentField());
	TypePtr iSeg(pParent->OwnerComplex());
	Seg_t &rSeg(*iSeg->typeSeg());
	ADDR addr_p(pParent->_key());
	if (addr_p == addr_beg)
		return 0;

	/*if (bUserAction)
	{
		PrintError() << "Nested functiones are not allowed" << std::endl;
		return 0;//user is supposed to interactively split the function
	}*/
	//split the function here at address
	ADDR addrSplit(addr_beg);
	ADDR a(addr_beg);
	FieldMap &m(pSelf->fields());
	FieldMapIt it(m.lower_bound(a));
	if (it != m.end())
		if (it != m.begin())//can it be a first?
			--it;//the field must have been found at this address or the next one
	if (it != m.end())
	{
		FieldPtr f(VALUE(it));
		if (!f->type())
			return 0;//?
		if (!f->type()->typeCode())
			return 0;//?
		//adjust split address to include entire code chunk
		addrSplit = KEY(it);
		DataSourcePane2_t data(GetDataSource()->pvt(), iSeg);
		CodeIterator1 codeIt(data, f);
		while (codeIt.unassemble())
			addrSplit = codeIt.nextAddr();
		if (addrSplit > addr_beg)
			return -1;//can't split a function in the middle of the code flow
						//adjust func's size
		it++;		
		if (!RelocateFields(iSelf, it, m.end(), iSeg, iSelf->base(), iSelf->base()))
			return 0;
#if(0)
		SetFuncSize(iSelf, addrSplit - addr_p);
		//relocate tailing fields from function to the parent segment

		StrucModifier_t an(*this);
		while (it != m.end())
		{
			FieldPtr f2(VALUE(it));
			ADDR fa(KEY(it));
			FieldMapIt it2(it++);
			if (pSelf->takeField0(it2) != f2)
				ASSERT0;
			f2->setOwnerComplex(nullptr);
			an.insertField(iSeg, fa, f2);
		}
#endif
	}
#if(0)
	else//no tailing fields - just trim the function, nothing else
		SetFuncSize(iSelf, addrSplit - addr_p);
#endif

	//adjust locus
	aLoc.pop_back();
	aLoc.setField(pField0);
	return 1;
}

static bool CanBeAbsorbedByFunc(FieldPtr f)
{
	TypePtr iType(f->type());
	if (iType)
	{
		if (!iType->typeSimple())
			if (!iType->typeCode())
				return false;
	}
	if (f->isExported() || f->isTypeImp() /*|| f->isCloneMaster()*/)
		return false;//do not step over exported entries as well!
	if (f->hasUserData())
		return false;
	return true;
}

static FieldMapIt FindUpperLabel(FieldMap &m, FieldMapIt i, ADDR end)
{
	while (i != m.end())
	{
		FieldPtr f(VALUE(i));
		if (ProjectInfo_s::IsEosField(f))
			break;
		ADDR a(KEY(i));
		if (a >= end)
			break;
		if (!CanBeAbsorbedByFunc(f))
			break;
		++i;
	}
	return i;
}

int ProjModifier_t::makeFunc2(Locus_t &aLoc0, bool bUserAction, int depth)//, ADDR addr_end)
{
	Locus_t aLoc(aLoc0);

	RESULT_e eRes(MakeCode(aLoc, true));
	if (eRes == RESULT_FAILED)
		return 0;
	if (eRes == RESULT_OK)
		TriggerCodeSweep(aLoc.seg(), aLoc.va());
	if (MakeProcedure(aLoc, bUserAction, 0) == RESULT_OK)
		return 1;
	return 0;
}

RESULT_e ProjModifier_t::MakeThunk(Locus_t& aLoc, bool bUserAction, bool bPropagateName)
{
	FieldPtr pField(aLoc.field0());
CHECKID(pField, 0x8dd)
STOP
	if (pField->isTypeThunk())
		return RESULT_ALREADY;
	ADDR tva;
	int iThunk(CheckThunk(pField, tva));
	if (iThunk == 0)//not a thunk
		return RESULT_FAILED;
	if (pField->isTypeCode())
		if (pField->ownerProc())
			return RESULT_FAILED;
	assert(pField->isTypeCode());
	CTypePtr pSeg(OwnerSeg0(pField->owner()));
	if (!pSeg)
		return RESULT_FAILED;
	TypesMgr_t* pTypesMgr(findTypeMgr(pSeg));
	if (!pTypesMgr)
		return RESULT_FAILED;
	TypePtr pCode(pTypesMgr->getTypeCode());
	if (!pCode)
		return RESULT_FAILED;
	TypesTracer_t TT(*this, *pTypesMgr);
	TypePtr pThunk(TT.thunkOf(pCode));
	if (!pThunk)
		return RESULT_FAILED;
	//BinaryCleaner_t<> BC(*this);
	//BC.ClearType(pField);
	SetTypeEx(pField, pThunk);
	mrProject.markDirty(DIRTY_GLOBALS);

	if (tva != 0 && bPropagateName && pField->name())
	{
		Locus_t aLoc2;//target location
		TypePtr pSeg2(VA2Locus(pSeg, tva, aLoc2, true));
		if (pSeg2 && aLoc2.field0() && aLoc2.field0()->nameless())
		{
			if (!pField->isExported())
			{
				PNameRef pn(pField->name());
				//MyString sName(pn->c_str());
				NamesMgr_t* pnm(OwnerNamesMgr(pSeg, nullptr));
				NamesMapIt it(pn);
				if (it != pnm->end())//?
				{
					pField->setName0(nullptr);
					//it->setObj(aLoc2.field0());
					aLoc2.field0()->setName0(pn);
				}
				//if (BC.ClearFieldName(pField, pnm))
					//AddName(*pnm, aLoc2.field();
				STOP
			}
		}
	}
	return RESULT_OK;
}

RESULT_e ProjModifier_t::MakeProcedure(Locus_t& aLoc, bool bUserAction, unsigned funcType)
{
	ADDR addr_beg(aLoc.addr());
CHECK(addr_beg == 0x1d9880)
STOP

	TypePtr iSelf(aLoc.struc());
	if (!iSelf->typeSeg())
	{
		if (bUserAction)
		{
			if (iSelf->typeProc())
				PrintError() << "Already a function at VA=" << VA2STR(iSelf->parentField()) << std::endl;
			else
				PrintError() << "Can't make a function - no appropriate context" << std::endl;
		}
		return RESULT_FAILED;
	}

	Seg_t& rSeg(*iSelf->typeSeg());
	FieldMap& m(rSeg.fields());

	FieldMapIt it = StrucFieldIt(iSelf, addr_beg);
	if (it != m.end())
		if (KEY(it) == addr_beg && it->isTypeProc())
			return RESULT_ALREADY;

	//take opening label
	FieldPtr pField(aLoc.field0());

	{
		FieldMapIt it0(m.lower_bound(pField->_key()));
		if (VALUE(it0) != pField)
			ASSERT0;
	}

	//relocate the type
	TypePtr iType0(pField->type());
	assert(!iType0 || iType0->isShared());//code?
	pField->setType0(nullptr);
	if (iType0->typeThunk())
	{
		TypePtr iTypeCode(iType0->typeThunk()->baseType());//off with the thunk
		iType0->releaseRef();
		iType0 = iTypeCode;
		iType0->addRef();//we are keeping it
	}

	TypePtr pProc(memMgr().NewTypeRef(new TypeProc_t));
	pProc->setParentField(pField);//before SetFuncSize!

	pField->setType0(pProc);
	pProc->addRef();

	//re-insert entry label
//	an.insertField(pProc, pField->_key(), pField);
	aLoc.add(pProc, pField->_key(), nullptr);

	if (InsertField(aLoc))
		if (iType0 && SetTypeEx(aLoc.field0(), iType0))
			iType0->releaseRef();//we just took it

	aLoc.pop_back();//re-adjust locus - no entry labels
	//migrate an "exported" attribute
//	if (IsExported(pField))
//		pField1->set AttributeFromId(ATTR_EXPORTED);
	mrProject.OnMakeFunction(pField);
	mrProject.markDirty(DIRTY_GLOBALS);
	return RESULT_OK;
}

int ProjModifier_t::SweepProc(CFieldPtr pField, int depth, unsigned flags)
{
	FieldPtr pLabel;
	TypePtr pProc(pField->isTypeProc());
	if (!pProc)
	{
		if (IsEntryLabel(pField))
		{
			pProc = pField->owner();
			assert(pProc->typeProc());
			pLabel = (FieldPtr)pField;
			pField = pProc->parentField();
		}
		else
			pLabel = (FieldPtr)pField;//no proc
	}
	else
		pLabel = EntryField(pProc);

	TypePtr pSeg(OwnerSeg0(pLabel->owner()));
	Seg_t& rSeg(*pSeg->typeSeg());

	ADDR addr_end(SweepCode(pLabel, depth, flags));
	if (pProc)
	{
		FieldMapIt it_nx((FieldPtr)pField);
		++it_nx;
		FieldMapIt it_up(FindUpperLabel(rSeg.fields(), it_nx, addr_end));
		RelocateFields(pSeg, it_nx, it_up, pProc, pProc->base(), pProc->base());
		mrProject.markDirty(DIRTY_GLOBALS);
	}
	return 1;
}

int ProjModifier_t::ExpandFunc(const Locus_t & ax, ADDR addr_end)
{//return 0;
	TypePtr iSeg(ax.struc());
	assert(iSeg);
	if (!iSeg->typeSeg())
	{
		if (!iSeg->typeProc())
			return 0;
		iSeg = OwnerSeg(iSeg);
		if (!iSeg)
			return 0;
	}

	Seg_t *pSeg(iSeg->typeSeg());
	ADDR addr(ax.addr());

	FieldMap &m(pSeg->fields());

	FieldMapIt it = StrucFieldIt(iSeg, addr);
	if (it == m.end())
		return 0;

	FieldPtr pField(VALUE(it));
	if (!pField->type())
		return 0;

	TypePtr iFunc(pField->type());
	TypeProc_t *pFunc(iFunc->typeProc());
	if (!pFunc)
		return 0;

	ADDR addr_beg(iFunc->parentField()->_key());

CHECK(addr_beg == 0x50BA70)
STOP

	/*int fsize = addr_end - addr_beg;
	if (fsize <= 0 )
		return 0;

	if (pFunc->size() > fsize)
		return 0;
	
	assert(pFunc->base() == addr_beg);*/

	//relocate following fields
	++it;
	int fsize(relocateFields(iSeg, it, iFunc, addr_beg, addr_end));
	if (!fsize)
		return 0;

	/*while (it != pSeg->fields().end())
	{
		ADDR fa = it->first;
		FieldPtr  f = it->second;
		if (f->type() && f->type()->type Proc())
			break;//do not step over a function

		FieldMapIt it_nx = it;
		it_nx++;

		if (fa >= addr_beg)
		{
			int offs = fa - addr_beg;
			if (offs >= fsize)
				break;

			pSeg->fields().erase(it);
			f->setOwnerComplex(nullptr);
			StrucModifier_t an;
			an.insertField(pFunc->typ eobj(), fa, f, iFunc);
		}
		it = it_nx;
	}*/

#if(0)
	SetFuncSize(iFunc, fsize);
#endif

	mrProject.markDirty(DIRTY_GLOBALS);
	return 1;
}

int ProjModifier_t::relocateFields(TypePtr iStrucSrc, FieldMapIt it, TypePtr pStrucDst, ADDR addr_beg, ADDR addr_end)
{
#if(0)
	int fsize(pStrucDst->size());
#else
	int fsize = addr_end - addr_beg;
	if (fsize <= 0)
		return 0;

	if (pStrucDst->size() > fsize)
		return 0;
#endif

	//assert(pFunc->base() == addr_beg);
	Struc_t *pStrucSrc(iStrucSrc->typeStruc());

	while (it != pStrucSrc->fields().end())
	{
		ADDR fa = KEY(it);
CHECK(fa == 0x10061e3)
STOP
		if (fa >= addr_end)
			break;
		FieldPtr f(VALUE(it));
		if (!CanBeAbsorbedByFunc(f))
		{
			fsize = fa - addr_beg;
			break;//do not step over a function - pFunc is bad!
		}

		FieldMapIt it_nx = it;
		it_nx++;

		if (fa >= addr_beg)
		{
			int offs = fa - addr_beg;
			if (offs >= fsize)
				break;

			//pSeg->fields().erase(it);
			pStrucSrc->takeField0(it);

			f->setOwnerComplex(nullptr);
			InsertFieldAt(pStrucDst, f, fa);
		}
		it = it_nx;
	}

	return fsize;
}

bool ProjModifier_t::getBoundsOnAddress(TypePtr iSelf, ADDR addr, ADDR bounds[2])
{
	Struc_t &rSelf(*iSelf->typeStruc());//may not be a proc
	if (addr < iSelf->base())
		return false;
/*	if (iSelf->size() < 0)
		return false;//?
	if (addr >= iSelf->base() + iSelf->size())
		return false;*/

	bounds[0] = addr;//lower
	bounds[1] = bounds[0] + 1;//upper

	FieldMap &m(rSelf.fields());
	FieldMapCIt it(m.upper_bound(addr));//next field greater than addr
	if (it == m.begin())
		return true;//no fields before addr

	if (it == m.end() && m.empty())
		return true;

	--it;
	it = m.lower_bound(KEY(it));//can be a u-field

	CFieldPtr pField(VALUE(it));
	if (!pField->isTypeCode())
	{
		TypePtr iProc(pField->isTypeProc());
		if (iProc)//are we right after the function with pending code? (the block doesn't terminates with control break)
		{
			FieldMap &m2(iProc->typeProc()->fields());
			if (m2.empty())
				return true;//no pending code after a func
			FieldMapRIt j(m2.rbegin());
			pField = VALUE(j);
			if (addr < KEY(j))
				return getBoundsOnAddress(iProc, addr, bounds);//did I get inside of a function? (shrink)
			if (IsEosField(pField))
				++j;//skip the eos to get to the actual last field
			if (j == m2.rend())
				return true;
			pField = VALUE(j);//EosAwarePrior?
		}
	}
	if (pField->isTypeCode())
	{
		TypePtr iSeg(OwnerSeg(pField->owner()));
		DataSourcePane2_t data(GetDataSource()->pvt(), iSeg);
		CodeIterator1 codeIt(data, pField);
		while (codeIt.unassemble())
		{
			if (codeIt.addr() <= addr && addr < codeIt.nextAddr())
			{
				bounds[0] = codeIt.addr();
				bounds[1] = codeIt.nextAddr();
				break;
			}
		}
	}
	else if (pField->_key() <= addr && addr < pField->_key() + pField->size())
	{
		bounds[0] = pField->_key();
		bounds[1] = bounds[0] + pField->size();
	}
	return true;
}

bool ProjModifier_t::SetFunctionEnd(Locus_t & ax)
{
	bool bForce(false);
	if (ax.empty())
		return false;

	const Frame_t &scope(ax.back());
	TypePtr iFunc(scope.cont()->objTypeGlob());
	if (!iFunc->typeStruc())
		return false;//array?

	Struc_t &rFunc(*iFunc->typeStruc());

	//adjust a picked location to fall exactly on instruction's starting address
	ADDR bounds[2];
	if (!getBoundsOnAddress(iFunc, scope.addrx(), bounds))//prevent the addr getting within instruction bounds
		return false;

	if (iFunc->typeProc())//inside?
	{
		if (ResizeStruc(iFunc, bounds[1] - iFunc->base(), ax.upframe(1), bForce))
			return true;
		//PrintError() << Not inside the function" << std::endl;
		return false;
	}

	FieldMapIt it(rFunc.fields().lower_bound(bounds[1]));//next field >= addr upper bound

	//outside
	Seg_t *pSeg(ax.back().cont()->typeSeg());
	if (!pSeg)
	{
		//shrink a struc anyway
		if (ResizeStruc(iFunc, bounds[1] - iFunc->base(), ax.upframe(1), bForce))
			return true;
		PrintError() << "Not inside the function" << std::endl;
		return false;
	}

	FieldMap &m(pSeg->fields());

	//trace back a function
	if (it != m.end())
	{
		assert((ADDR)KEY(it) > scope.addrx());
	}

	while (it != m.begin())//no function above?
	{
		--it;
		iFunc = it->type();
		if (iFunc)
		{
			if (iFunc->typeProc())
			{
				Frame_t a(ax.back().cont(), KEY(it), VALUE(it));
				return ResizeStruc(iFunc, bounds[1] - iFunc->base(), &a, bForce);// ax.upframe(0));//expand
			}
			if (iFunc->typeThunk())
				break;
			if (iFunc->typeComplex() && !iFunc->typeCode())
				break;//?
		}
	}
	PrintError() << "Not a function up the address range" << std::endl;
	return false;
}

TypePtr ProjectInfo_s::SkipArray(CTypePtr p)
{
	if (p)
		while (p->typeArray())
			p = p->typeArray()->baseType();
	return (TypePtr)p;
}

bool ProjectInfo_t::CheckRelocEligability(FieldPtr pField, TypePtr iDst) const
{
	assert(!IsEosField(pField));
	if (pField->isExported() || pField->isTypeImp() /*|| pField->isCloneMaster()*/)
		return false;//do not step over exported entries as well!
	assert(!iDst->typeStrucvar());
	TypePtr iType0(pField->type());
	if (!iType0)
		return true;
	if (iType0 == iDst)
		return false;
	if (iType0->typeSeg() || iType0->typeProc())
		return false;
	if (iType0->typeCode())
		return (iDst->typeProc() || iDst->typeSeg());
	iType0 = SkipArray(iType0);
	if (iType0->typeSimple())
		return true;//ptrs,enums
	if (CheckTypeInclusion(iDst, iType0))
		return false;
	/*assert(iType0->typeStruc());
	for (Struc_t::HierIterator i(iType0, true); i; i++)//step into shared!
	{
		CFieldRef r(*i);
		if (SkipArray(r.type()) == iDst)
			return false;//a struc cannot contain itself (directly or indirectly)
	}*/
	return true;
}

int ProjModifier_t::RelocateFields(TypePtr iSrc, FieldMapIt lower, FieldMapIt upper, TypePtr iDst, ADDR base, ADDR base2) const//upper is not included!
{
	//base - subtracted from address of src to get offset
	//base2 - added to offset to get address of dst

	Struc_t &rSrc(*iSrc->typeStruc());
	Struc_t &rDst(*iDst->typeStruc());

	FieldMap &m(rSrc.fields());
	FieldMap &m2(rDst.fields());

	NamesMgr_t *pNS1(rSrc.namesMgr());
	NamesMgr_t *pNS2(pNS1 ? OwnerNamesMgr(iDst, nullptr) : nullptr);
	bool bRelocNames(pNS1 && pNS1 != pNS2);
	assert(!bRelocNames || pNS2);

	//ADDR base(iSrc->base());
	//ADDR base2(iDst->base());

	//first, if there is an eos field, strip it to prevent interference
	unsigned oldSize;
	FieldPtr pEos(!iDst->typeSeg() ? TakeEosField(iDst, oldSize) : nullptr);//not for segs
	int count(0);
	FieldMapIt i(lower);
	while (i != upper)
	{
		FieldMapIt j(i++);
		int o(KEY(j) - base);//got offset
		FieldPtr f(VALUE(j));
CHECK(f->_key() == 0x10061e3)
STOP
		if (!IsEosField(f))
			if (!CheckRelocEligability(f, iDst))
				break;//stop here
		count++;//the eos is counted as well
		//relocate a name to destination namespace
		if (bRelocNames && !f->nameless())
		{
			MyString s(f->name()->c_str());
			ClearFieldName(f, pNS1);
			//SetObjName(*pNS2, f, s);
			if (!AddName(*pNS2, s, f, f->isTypeImp()))
				PrintWarning() << "A name already exists in current context: ''" << s << "'" << std::endl;
		}
		bool bIsEos(IsEosField(f));
		m.take(j);
		f->setOwnerComplex(nullptr);
		if (bIsEos)
		{
			assert(i == m.end());
			memMgr().Delete(f);//will be re-created if need be
			break;
		}
		InsertFieldAt(iDst, f, base2 + o);
	}
	// check if old size must be re-established
	if (pEos)
	{
		int newSize(iDst->size());
		if (newSize > 0 && newSize < (int)oldSize)
			InsertFieldAt(iDst, pEos, base2 + oldSize);
		else
			memMgr().Delete(pEos);
	}
	return count;
}

FieldPtr ProjectInfo_t::TakeEosField(TypePtr iSelf, unsigned &sz) const
{
	FieldMap &m(iSelf->typeStruc()->fields());
	if (m.empty())
		return nullptr;
	FieldMapRIt i(m.rbegin());
	FieldPtr pField(VALUE(i));
	if (!IsEosField(pField))
		return nullptr;
	//remember the size
	sz = pField->_key() - iSelf->base();
	m.take(i);
	//memMgr().Delete(pField);
	return pField;
}

bool ProjModifier_t::ResizeStruc(TypePtr iSelf, unsigned newSize, const Frame_t *pOuter, bool bForce)
{
	bool bRet(false);
	if (pOuter)//if no fields to relocate, just change the size (it may fail)
	{
		Struc_t &rSelf(*iSelf->typeStruc());
		//ADDR addr(iStruc->base() + newSize);
		unsigned oldSize(iSelf->size());
		if (oldSize == -1)//open?
		{
			if (rSelf.fields().last() != rSelf.fields().end())
				oldSize = KEY(rSelf.fields().last()) - iSelf->base() + OPSZ_BYTE;
		}

		if (newSize == oldSize)
			if (!bForce)
				return bRet;//already

		if (newSize > oldSize)//expand (outer --> inner)
		{
			//relocate fields from outer container into this struc, shared are OK?

			TypePtr iSrc(pOuter->cont());
			Struc_t &rSrc(*iSrc->typeStruc());//outer container
			ADDR addrLo(pOuter->addr());//address the self sits at
			ADDR addrHi(addrLo + newSize);//upper bound for relocation
			FieldMapIt itLower(rSrc.fields().upper_bound(addrLo));//next field > addr
			FieldMapIt itUpper(rSrc.fields().lower_bound(addrHi));//will not be moved
			if (RelocateFields(iSrc, itLower, itUpper, iSelf, pOuter->addr(), iSelf->base()))//dst is inner
				bRet = true;
		}
		else if (newSize < oldSize)
		{
			// newSize < oldSize (inner --> outer)

			TypePtr iDst(pOuter->cont());
			ADDR addrLo(iSelf->base() + newSize);
			FieldMapIt itLower(rSelf.fields().lower_bound(addrLo));
			FieldMapIt itUpper(rSelf.fields().end());
			if (RelocateFields(iSelf, itLower, itUpper, iDst, iSelf->base(), pOuter->addr()))//dst - is outer
				bRet = true;
		}
	}
	if (SetStrucSize(iSelf, newSize, bForce))
		bRet = true;

	return bRet;
}

bool ProjectInfo_t::SetFuncSize(TypePtr iSelf, unsigned fixedSize) const
{
	return false;//SetStrucSize(iSelf, fixedSize);
}

FieldPtr ProjModifier_t::MakeData(Locus_t &aLoc, TypePtr pType, AttrIdEnum attr, bool bForce)
{
	if (aLoc.empty())
		return nullptr;//?
	return MakeData(pType, attr, bForce, aLoc);
}

FieldPtr ProjModifier_t::MakeReal(Locus_t &loc, bool bForce)
{
	if (loc.empty())
		return nullptr;
	return MakeReal(loc.struc(), loc.addr(), bForce, loc);
}

TypePtr ProjModifier_t::MakeArrayOfTypeAttr(const Locus_t &aLoc, TypePtr pType)
{
	const I_DataSource &aRaw(GetDataSource()->pvt());

	OFF_t oPtr(aLoc.back().rawoff());
	if (aRaw.isNull(oPtr))
		return pType;

	/*const Struc_t &rSelf(*aLoc.struc());

	FieldPtr f(VALUE(it));
	FieldMapIt itnx(it);
	itnx++;
	unsigned range(iSelf->size() - (f->_key() - iSelf->base()));
	if (itnx != rSelf.fields().end())
		range = std::min(VALUE(itnx)->_key() - f->_key(), range);*/

	unsigned range(aLoc.range(false));

	TypesMgr_t* pTypeMgr(findTypeMgr(aLoc.back().cont()));
	if (!pTypeMgr)
		pTypeMgr = AssureTypeMgr();
	TypesTracer_t TT(*this, *pTypeMgr);
	return TT.stringOf(pType, oPtr, range);
}



FieldPtr ProjModifier_t::MakeReal(TypePtr iSelf, ADDR addr, bool bForce, Locus_t &loc)
{
	Struc_t &rSelf(*iSelf->typeStruc());

	TypePtr pType(NextRealAt(loc));
	if (!pType)
		return nullptr;

	FieldPtr f(loc.field0());
	if (!f || !f->type())
	{
		FieldMapIt it(rSelf.fields().end());
		if (!f)
		{
			it = InsertFieldIt(loc.back());// frame_t(iSelf, addr));
			if (it == rSelf.fields().end())
				return nullptr;
			f = VALUE(it);
		}
		else
			it = FieldIt(iSelf, f->_key(), nullptr, FieldIt_Overlap);
		SetType(f, pType);
	}
	else
	{
		FieldMapIt itf(FieldIt(iSelf, addr, nullptr, FieldIt_Overlap));

		Array_t *pArray(f->type()->typeArray());
		if (pArray)
			pType = MakeArrayOfTypeAttr(loc, pType);//, loc.back().rawoff());

		if (bForce)
		{
			BinaryCleaner_t<> PC(*this);
			PC.ClearType(f);
			SetType(f, pType);
		}
	}

	mrProject.OnDataEdited(ModulePtr());
	return f;
}

TypePtr ProjModifier_t::CarouselType(const Locus_t&)
{
	return nullptr;
}

FieldPtr ProjModifier_t::MakeData(TypePtr pType, AttrIdEnum attr, bool bForce, Locus_t& aLoc)
{
	if (aLoc.struc()->typeStrucvar())
	{
		if (aLoc.addr() == 0)
			return nullptr;
		aLoc.reduce();
	}

	Frame_t& aTop(aLoc.back());

	//CHECK(aTop.addr() == 0x50bfb8)
	//STOP

	Struc_t& rSelf(*aTop.struc());

	if (!pType)
	{
		if (!attr)
		{
			pType = NextIntAt(aLoc, false);
			if (!pType)
				return nullptr;
		}
		else if (attr == ATTR_ASCII_TEXT)
			pType = GetStockType(OPTYP_CHAR8);
		else if (/*attr == ATTR_ UNICODE || */attr == ATTR_NUNICODE)
			pType = GetStockType(OPTYP_CHAR16);
	}

//CHECK(aTop.addr() == 0x1008bf1)
//STOP
	
	if (!aTop.field())
	{
		FieldMapIt it(InsertFieldIt(aTop));
		if (it == rSelf.fields().end())
			return nullptr;
//		aTop.setField(VALUE(it));
	}
	//else
		//it = FieldIt(aTop.cont(), aTop.field()->_key(), nullptr, FieldIt_Overlap);

	if (aTop.field()->type())
	{
		if (!bForce)
			return nullptr;

		if (aTop.field()->type() == pType)
			return aTop.field();

		BinaryCleaner_t<> BC(*this);
		BC.ClearType(aTop.field());
		SetType(aTop.field(), pType);
	}
	else
	{
		if (attr)
		{
			//unsigned uRange2(aLoc.range(false));
			pType = MakeArrayOfTypeAttr(aLoc, pType);//, aTop.rawoff());
			if (pType->typeArray())
				aTop.field()->setAttribute(attr);
		}

		SetType(aTop.field(), pType);

		//can we delete eos?
		if (bForce)
		{
			FieldMapIt itnx(aTop.field());
			if (++itnx != rSelf.fields().end())
			{
				FieldPtr f2(VALUE(itnx));
				if (IsEosField(f2) && aTop.field()->addressHi() == f2->_key())
				{
					rSelf.fields().take(itnx);
					memMgr().Delete(f2);
				}
			}
		}
	}

	mrProject.OnDataEdited(ModulePtr());
	return aTop.field();
}

bool ProjModifier_t::MakeSigned(Locus_t& aLoc, bool bForce)
{
#if(0)
	Frame_t& aTop(aLoc.back());
	if (!aTop.field())
		return false;
	TypePtr pType(aTop.field()->type());
	if (!pType)
		return false;
	if (pType->typeArray())
		pType = pType->typeArray()->absBaseType();
	if (!pType->typeSimple())
		return false;

	OpType_t optyp(OpType_t(pType->typeSimple()->optype()));
	if (OPTYPE(optyp) == OPTYP_UINT0)
		optyp = MAKETYP_SINT(optyp);
	else if (OPTYPE(optyp) == OPTYP_INT0)
		optyp = MAKETYP_UINT(optyp);
	else if (OPTYPE(optyp) == OPTYP_NULL || OPTYPE(optyp) == OPTYP_INTEGER)//some
		optyp = MAKETYP_SINT(optyp);
	else
		return false;

	pType = GetStockType(optyp);
	BinaryCleaner_t<> BC(*this);
	BC.ClearType(aTop.field());
	SetType(aTop.field(), pType);
	mrProject.OnDataEdited(ModulePtr());
#else
	if (aLoc.struc()->typeStrucvar())
	{
		if (aLoc.addr() == 0)
			return nullptr;
		aLoc.reduce();
	}

	Frame_t& aTop(aLoc.back());

	Struc_t& rSelf(*aTop.struc());

	TypePtr pType;// (aTop.field() ? aTop.field()->type() : nullptr);
	//if (!pType)
	{
		pType = NextIntAt(aLoc, true);
		if (!pType)
			return nullptr;
	}

//CHECK(aTop.addr() == 0x1008bf1)
//STOP
	
	if (!aTop.field())
	{
		FieldMapIt it(InsertFieldIt(aTop));
		if (it == rSelf.fields().end())
			return nullptr;
		aTop.setField(VALUE(it));
	}
	//else
		//it = FieldIt(aTop.cont(), aTop.field()->_key(), nullptr, FieldIt_Overlap);

	if (aTop.field()->type())
	{
		if (!bForce)
			return nullptr;

		if (aTop.field()->type() == pType)
			return aTop.field();

		BinaryCleaner_t<> BC(*this);
		BC.ClearType(aTop.field());
		SetType(aTop.field(), pType);
	}
	else
	{
		SetType(aTop.field(), pType);

		//can we delete eos?
		if (bForce)
		{
			FieldMapIt itnx(aTop.field());
			if (++itnx != rSelf.fields().end())
			{
				FieldPtr f2(VALUE(itnx));
				if (IsEosField(f2) && aTop.field()->addressHi() == f2->_key())
				{
					rSelf.fields().take(itnx);
					memMgr().Delete(f2);
				}
			}
		}
	}

	mrProject.OnDataEdited(ModulePtr());
	return aTop.field();
#endif
	return true;
}

FieldPtr ProjModifier_t::MakeString(AttrIdEnum attr, Locus_t& aLoc)
{
//	if (!bForce)
	//	return nullptr;

	Frame_t& aTop(aLoc.back());
	TypePtr pType(aTop.field()->type());
	if (!pType)
		return nullptr;

	FieldMapIt itf(FieldIt(aTop.cont(), aTop.addrx(), nullptr, FieldIt_Overlap));

	//toggle between unicodez to cunicode
	/*if (attr == ATTR_ UNICODE)
	{
		if (aTop.field()->attrib() == ATTR_ UNICODE)
			attr = ATTR_NUNICODE;
	}
	else if (attr == ATTR_NUNICODE)
		if (aTop.field()->attrib() == ATTR_NUNICODE)
			attr = ATTR_ UNICODE;*/

	if (attr)//must preserve attribute otherwise
		aTop.field()->setAttribute(ATTR_NULL);

	Array_t* pArray(aTop.field()->type()->typeArray());
	if (pArray)
	{
		/*if ((attr == ATTR_ ASCII || attr == ATTR_ASCII_TEXT) && pArray->baseType()->size() == OPSZ_BYTE)
		{
			aTop.field()->toggleAttribute(attr);
			return aTop.field();
		}*/
		/*if (pArray->baseType()->size() == OPSZ_WORD)
		{
			if (attr == ATTR_UNICODE)
			{
				aTop.field()->toggleAttribute(attr);
				return aTop.field();
			}
			if (attr == ATTR_NUNICODE)
			{
				//adjust length
				if (aTop.rawoff() != OFF_NULL)
				{
					unsigned short n;
					const I_DataSource& aRaw(GetDataSource()->pvt());
					if (aRaw.dataAt(aTop.m_offs, sizeof(unsigned short), (PDATA)&n) && (n < pArray->total()))
					{
						if (MakeArray(aTop.cont(), itf, n + 1))
						{
							aTop.field()->toggleAttribute(attr);
							mrProject.OnDataEdited(ModulePtr());
							return aTop.field();
						}
					}
				}
			}
			else
			{
				pType = MakeArrayOfTypeAttr(aLoc, pType);//, aTop.rawoff());
				if (pType->typeArray())
					aTop.field()->setAttribute(attr);
			}
		}
		else
		{
			pType = MakeArrayOfTypeAttr(aLoc, pType);
			if (pType->typeArray())
				aTop.field()->setAttribute(attr);
		}*/
	}
	else// if (attr)
	{
		pType = MakeArrayOfTypeAttr(aLoc, pType);
		if (pType->typeArray())
			aTop.field()->setAttribute(attr);
	}

	if (aTop.field()->type() != pType)
	{
		BinaryCleaner_t<> BC(*this);
		BC.ClearType(aTop.field());
		SetType(aTop.field(), pType);
		mrProject.OnDataEdited(ModulePtr());
	}
	return aTop.field();
}

FieldPtr ProjModifier_t::makeBit(Locus_t &loc, TypePtr pType, AttrIdEnum attr, bool bForce)
{
	if (loc.empty())
		return nullptr;
	return MakeBit(pType, attr, bForce, loc);
}

FieldPtr ProjModifier_t::MakeBit(TypePtr pType, AttrIdEnum attr, bool bForce, Locus_t &aLoc)
{
	TypePtr iSelf(aLoc.struc());
	//ADDR addr(aLoc.addr());
	Struc_t &rStruc(*iSelf->typeStruc());
	Bitset_t *pBitset(rStruc.typeBitset());

	if (!pType)
	{
		if (!pBitset)
		{
			if (!InsertField(aLoc))
				return nullptr;

			//FieldPtr pField(aLoc.field0());
			pBitset = new Bitset_t();
			TypePtr iBitset(memMgr().NewTypeRef(pBitset));
			if (!SetTypeEx(aLoc.field0(), iBitset))
			{
				memMgr().Delete(iBitset);
				return nullptr;
			}

			FieldPtr pField(AppendBField(iBitset));// GetStockType(OPSZ_BYTE));//pos 0
			terminalFieldAt(aLoc);//update locus
			aLoc.setField(pField);
//			mrProject.OnDataEdited(ModulePtr());
//			return pField;
		}
		/*OpType_t t(OpType_t(pBitset->size Bytes(iStruc)));
		assert(t);
		pType = GetStockType(t);
		if (!pType)
			return nullptr;*/
	}

	assert(!pType || pType->typeSimple());
	return MakeData(pType, attr, bForce, aLoc);
}

FieldPtr ProjModifier_t::makeOffset(Locus_t &loc, bool bForce)
{
	TypePtr iModule(loc.module());
	FieldPtr pField(MakeOffset(loc, bForce));
	if (!pField)
		return nullptr;

	//unbound
	for (Locus_t::const_reverse_iterator i(loc.rbegin()); i != loc.rend(); i++)
	{
		const Frame_t &f(*i);
		Struc_t *pStruc(f.struc());
		if (!pStruc || pStruc->typeSeg())
			break;
		FieldMap &m(pStruc->fields());
		FieldMapIt it(m.upper_bound(f.addr()));
		FieldPtr pField2(VALUE(it));
		if (pField2 == pField)//for lowest frame only
			it++;
		if (it == m.end())
			continue;
		if (IsEosField(pField2))
		{
			m.take(it);
			memMgr().Delete(pField2);
		}
	}

	mrProject.OnDataEdited(ModulePtr());
	return pField;
}

TypePtr ProjModifier_t::NextIntAt(const Locus_t& aLoc, bool bSigned)
{
	unsigned uRange(aLoc.range(false));
	const Frame_t &aTop(aLoc.back());

	TypePtr iSelf(aTop.cont());
	Struc_t &rSelf(*iSelf->typeStruc());
	FieldPtr pField(aTop.field());

	Bitset_t *pBitset(rSelf.typeBitset());
	if (pBitset)
	{
		if (rSelf.hasOnlyChild() && pField)
		{
		}
		else
		{
			OpType_t t(OpType_t(iSelf->sizeBytes()));
			assert(t);
			return GetStockType(t);
		}
	}

/*?	FieldPtr fnx(nullptr);
	FieldPtr f(Field(iSelf, aTop.addr, &fnx, 1));

	if (fnx && fnx->_key() < addr_limit)
		addr_limit = fnx->_key();

	ADDR max_size = addr_limit - aTop.addr;*/
	//assert(max_size >= 0);

	static const OpType_t types[] = {
		OPSZ_BYTE, OPSZ_WORD, OPSZ_DWORD, OPSZ_QWORD };

	const int tabsz(sizeof(types) / sizeof(OpType_t));

	OpType_t tID(types[tabsz - 1]);//get the last one
	
	uint8_t t(bSigned ? OPTYP_INT0 : OPTYP_UINT0);

	TypePtr iType(pField ? pField->type() : nullptr);
	if (iType && iType->typeSimple())
	{
		tID = iType->typeSimple()->typeID();
		uint8_t t0(OPTYPE(tID));
		uint8_t s0(OPSIZE(tID));
		if (t0 != t)
			return GetStockType(MAKETYP(t, s0));
	}

	int i(-1);
	for (i = 0; i < tabsz; i++)
	{
		if (types[i] == OPSIZE(tID))
			break;
	}

	int i0(i);
	for (;;)//i < tabsz)
	{
		i++;
		if (i >= tabsz)
			i = 0;
		if (i == i0)
			break;

		iType = GetStockType(MAKETYP(t, OPSIZE(types[i])));
		if ((unsigned)iType->size() > uRange)
			continue;

		return iType;
	}

	return nullptr;//error
}

TypePtr ProjModifier_t::NextRealAt(const Locus_t &l)
{
	const Frame_t &aTop(l.back());
	TypePtr iSelf(aTop.cont());
	Struc_t &rSelf(*iSelf->typeStruc());
	//TypesMgr_t *pTypesMgr(mrModule.typeMgr());
	FieldPtr pField(l.field0());
	ADDR max_size = l.range(false);

	if (rSelf.typeBitset())
		return nullptr;//invalid context

	static const OpType_t types[] = {
		OPTYP_REAL32, OPTYP_REAL64 };//, OPTYP_REAL80 };

	const int tabsz(sizeof(types) / sizeof(OpType_t));

	OpType_t tID(types[tabsz - 1]);//get the last one

	uint8_t t(OPTYP_REAL0);

	TypePtr iType(pField ? pField->type() : nullptr);
	if (iType && iType->typeSimple())
	{
		tID = iType->typeSimple()->typeID();
		uint8_t t0(OPTYPE(tID));
		uint8_t s0(OPSIZE(tID));
		if (t0 != t)
			return GetStockType(MAKETYP(t, s0));
	}

	int i(-1);
	for (i = 0; i < tabsz; i++)
		if (types[i] == tID)
			break;

	int i0(i);
	for (;;)//i < tabsz)
	{
		i++;
		if (i >= tabsz)
			i = 0;
		if (i == i0)
			break;

		iType = GetStockType(types[i]);
		if ((ADDR)iType->size() > max_size)
			continue;

		return iType;
	}

	return nullptr;//error
}

FieldPtr ProjModifier_t::MakeOffset(Locus_t &loc, bool bForce)
{
	Struc_t *pSelf(loc.struc()->typeStruc());

	OpType_t iPtrSize(loc.seg()->typeSeg()->ptrSize());

	FieldPtr f(loc.field0());
	ADDR max_size(loc.range(true));//ignore eos fields

		//can we delete eos?
		if (bForce)
		{
/*			FieldMapIt 
			FieldMapIt itnx(it);
			if (++itnx != pSelf->fields().end())
			{
				FieldPtr f2(VALUE(itnx));
				if (IsEosField(f2) && f->_key() + f->size() == f2->_key())
				{
					pSelf->fields().take(itnx);
					memMgr().Delete(f2);
				}
			}*/
		}


	if (max_size < ADDR(iPtrSize & OPSZ_MASK))
		return nullptr;

	TypesTracer_t tt(*this, *mrModule.typeMgr());
	if (f && f->type())
	{
		TypePtr pType(f->type());
		//get all way down to the array's base type
		std::list<unsigned> lArr;//save dimentions to restore them later
		Array_t *pArr;
		for (TypePtr p(pType); (pArr = p->typeArray()) != nullptr; p = pArr->baseType())
		{
			lArr.push_back(pArr->total());
			pType = pArr->baseType();
		}

		assert(pType);
		if (!pType->typePtr())
		{
			if (!pType->typeSimple() || (pType->size() != iPtrSize))//size of ptr?
				return nullptr;

			pType = tt.ptrOf(nullptr, iPtrSize);
			while (!lArr.empty())
			{
				pType = tt.arrayOf(pType, lArr.back());
				lArr.pop_back();
			}

			BinaryCleaner_t<> PC(*this);
			PC.ClearType(f);
			SetType(f, pType);
		}
	}
	else
	{
		if (InsertField(loc))
			SetTypeEx(loc.field0(), tt.ptrOf(nullptr, iPtrSize));
		f = loc.field0();
	}

	mrProject.markDirty(DIRTY_GLOBALS);
	//loc.signalModified();
	return f;
}

FieldPtr ProjModifier_t::MakeArray(Locus_t &loc, int num)
{
	return MakeArray(loc.struc(), loc.addr(), num);
}

FieldPtr ProjModifier_t::MakeArray(TypePtr iSelf, ADDR addr, int num)
{
	FieldMapIt it(FieldIt(iSelf, addr, nullptr, FieldIt_Overlap));
	if (!MakeArray(iSelf, it, num))
		return nullptr;
	mrProject.OnDataEdited(ModulePtr());
	return VALUE(it);
}

bool ProjModifier_t::MakeArray(TypePtr iSelf, FieldMapCIt itf, int num)
{
	Struc_t &rSelf(*iSelf->typeStruc());
	CFieldPtr pField(VALUE(itf));
	if (itf == rSelf.fields().end())
		return false;

	TypePtr iType(pField->type());
	if (!iType)
		return false;

	if (iType->typeArray())
		iType = iType->typeArray()->baseType();

	FieldMapCIt itf2(itf);
	itf2++;

	/*if (num < 0)
	{
		Array_t *pArray(pField->type()->typeArray());
		if (pArray)
			num = (int)pArray->total();
		else
			num = 0;
		num++;//increase by 1
	}*/

	ADDR addr_limit(iSelf->base() + iSelf->size());
	if (itf2 != rSelf.fields().end() && !IsEosField(VALUE(itf2)))
	{
		ADDR addr2(itf2->_key());
		assert(addr2 < addr_limit);
		addr_limit = addr2;
	}

	int maxSize(addr_limit - pField->_key());
	assert(maxSize >= 0);

	int eSize;
	if (iSelf->typeBitset())
		eSize = BitSize(iType);
	else
		eSize = iType->size();
	if (!(eSize > 0))
	{
		PrintError() << "Cannot make array of context dependent object (" << TypeName(iType) << ")" << std::endl;
		return false;
	}

	unsigned num2(maxSize / eSize);
	if (num < 0)
		num = num2;

#if(1)
#if(1)//0 for all
	if (rSelf.typeBitset())
#endif
	if (num2 < (unsigned)num)
	{
		PrintWarning() << "Cannot accommodate requested array size (" << num << "). Reduced to " << num2 << std::endl;
		num = num2;
	}
#endif

	TypesTracer_t TT(*this, *findTypeMgr(pField->type()));
	TypePtr iTypeArray(TT.arrayOf(iType, num));
	iTypeArray->addRef();//TT.arrayOf does not increment ref if the type exist (ClearType can destroy it)
	BinaryCleaner_t<> PC(*this);
	PC.ClearType(const_cast<FieldPtr>(pField));
	if (!SetType(const_cast<FieldPtr>(pField), iTypeArray))
		ASSERT0;
	iTypeArray->releaseRef();
	mrProject.markDirty(DIRTY_GLOBALS);
	return true;
}

int ProjModifier_t::makeUndefined(Locus_t &loc)
{
	return MakeUnk(loc);
}

int ProjModifier_t::MakeUnk(Locus_t &aLoc)
{
	TypePtr iSelf(aLoc.struc());
	ADDR va(aLoc.addr());
	Struc_t &rStruc(*iSelf->typeStruc());

	//FieldPtr pField(Field(iSelf, va, nullptr, FieldIt_Overlap));
	if (!InsertField(aLoc))
	{
		FieldPtr pField(aLoc.field0());
		if (pField->type())
		{
			BinaryCleaner_t<> PC(*this);
			PC.ClearType(pField);
			SetType(pField, nullptr);
		}
		else
		{
			TypesTracer_t TT(*this, *findTypeMgr(pField->type()));
			TypePtr pType(TT.GetStockType(OPTYP_NULL));
			SetType(pField, pType);
		}
	}

	mrProject.OnDataEdited(ModulePtr());
	return 1;
}

int ProjModifier_t::MakeClone(Locus_t& aLoc)
{
	if (!aLoc.field0())
	{
		PrintError() << "No field at locus to clone" << std::endl;
		return 0;
	}

	TypePtr pCont(aLoc.struc());
	if (pCont->typeBitset())
	{
		PrintError() << "Bitset fields cannot be cloned" << std::endl;
		return 0;
	}

	TypePtr pType(aLoc.field0()->type());

	FieldPtr pField(AppendUField(aLoc));
	assert(pField);

	if (pType)
		SetType(pField, pType);

	aLoc.setField(pField);

	mrProject.markDirty(DIRTY_LOCUS_ADJUSTED);
	mrProject.OnDataEdited(ModulePtr());
	return 1;
}


FieldPtr ProjModifier_t::ToggleExported(Locus_t& aLoc)
{
	FieldPtr pField(aLoc.field0());
	if (pField && pField->isExported())
	{
		//just turn 'exported' off and be done
		mrProject.OnUnmakeExported(pField);
		pField->setExported(false);
		mrProject.markDirty(DIRTY_GLOBALS);
		return pField;
	}

	return nullptr;
}

FieldPtr ProjModifier_t::ToggleImported(Locus_t& aLoc)
{
	return nullptr;
}

void ProjModifier_t::UpdateLocus(Locus_t &aLoc)
{
	assert(0);
	//TypePtr iCont(aLoc.struc());
	//DA_t da((ROWID)aLoc.addr(), 0, 0);
	terminalFieldAt(aLoc);//iCont, da, aLoc, Block_t());
}

bool ProjModifier_t::makeScope(Locus_t &aLoc, ObjId_t objId)
{
	if (aLoc.empty())
		return false;

	//Frame_t &aTop(aLoc.back());

	TypePtr iType(nullptr);
	if (objId == OBJID_TYPE_STRUC)
	{
		Struc_t *pStruc(new Struc_t());
		iType = memMgr().NewTypeRef(pStruc);
	}
	else if (objId == OBJID_TYPE__UNION)
	{
		Struc_t *pUnion(new Struc_t());
		iType = memMgr().NewTypeRef(pUnion);
	}
	else if (objId == OBJID_TYPE_BITSET)
	{
		Bitset_t *pBitset(new Bitset_t());
		iType = memMgr().NewTypeRef(pBitset);
	}
	if (!iType)
		return false;

	if (aLoc.struc()->typeUnion())
		AppendUField(aLoc);
	else
		InsertField(aLoc);
	if (!aLoc.field0() || !SetTypeEx(aLoc.field0(), iType))
	{
		memMgr().Delete(iType);
		return nullptr;
	}

	//now add a default field (so a new scope would not appear empty)
	if (iType->typeUnion())
	{
		FieldPtr pField2(AppendUField(aLoc));
	}
	else// if (iType->typeBitset())
	{
		//UpdateLocus(aLoc);
		if (InsertField(aLoc))
			SetTypeEx(aLoc.field0(), iType);
	}

	mrProject.markDirty(DIRTY_GLOBALS);
	return true;
}

TypePtr ProjModifier_t::TypeInContext(MyString sTypeName, CTypePtr pContext)
{
	assert(!sTypeName.empty());
	TypePtr pType(nullptr);
	if (!pContext)
		pContext = ModulePtr();
	return mrProject.acquireType(sTypeName, pContext);
}

FieldPtr ProjModifier_t::makeObjOfType(Locus_t &aLoc, TypePtr pType, const MyString &aName, bool bForce)
{
	FieldPtr pField(MakeData(aLoc, pType, ATTR_NULL, bForce));
	if (pField)
	{
		if (!aName.empty())
			SetFieldName(pField, aName);
		mrProject.markDirty(DIRTY_GLOBALS);
	}

	return pField;
}











