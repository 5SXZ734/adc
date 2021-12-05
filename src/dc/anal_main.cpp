#include "anal_main.h"
#include "db/ui_main.h"
#include "anal_data.h"
#include "anal_ptr.h"
#include "anal_local.h"
#include "anal_pcode.h"
#include "anal_init.h"
#include "db/type_code.h"
#include "clean_ex.h"
#include "cc.h"
#include "ui_main_ex.h"
#include "flow.h"

////////////////////////////////////////////////////////
class ReAnalyzeStage_t : public PrimaryAnalysis_t::Stage_t
{
	FlowState_t	mFlowState;
public:
	ReAnalyzeStage_t(PrimaryAnalysis_t &rBoss)
		: Stage_t(rBoss, "RNA")
	{
		mFlowState.addStack(TreeTerminalFirst(rBoss.FuncDef().Body()), PathPtr());
	}
	virtual ~ReAnalyzeStage_t()
	{
		if (ProtoInfo_t::IsFuncStatusAborted(FuncDefPtr()))
		{
			mrBoss.abort();//stop further processing
			return;
		}
		ProtoInfo_t::SetFuncStatus(FuncDefPtr(), FDEF_DC_PHASE2);
	}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		PathOpTracer_t tr;
		Reanalizer_t RA(FI, tr);
		FlowIterator flowIt(FI, mFlowState);
		try
		{
			for (; flowIt; ++flowIt)
			{
				RA.OutputDebugInfo(*flowIt);

				STAGE_STATUS_e e;
				while ((e = RA.CheckStatus(*flowIt, flowIt.from())) != STAGE_STATUS_e::CONTINUE)
				{
					flowIt.drop();//the paths up the stack could have been analysed
				}

				if (!*flowIt)
					break;

				e = RA.ReAnalyzePath(*flowIt, flowIt.from(), op());
				setOp(HOP());//reset if did not throw
				if (e == STAGE_STATUS_e::FAILED)
					return e;
				assert(e == STAGE_STATUS_e::CONTINUE);
			}

			//OutputDebugString("done.\n");
		}
		catch (const StubFault_t&)
		{
			HPATH hPath(*flowIt);//failed path
			hPath->setAnalized(false);
			flowIt.save(mFlowState);//preserve state to catch up later at interrupt point
			mrBoss.Project().analyzer()->setStopFlag(StopFlag::PAUSE);//request a pause
			//Decompiler_t* pAnal(dynamic_cast<Decompiler_t*>(proj().analyzer()));
			//addSubTask(mCurAddr, 0, 1, true);//a hint to remove last op
			setOp(RA.CurrentOp());
			setVA(RA.OpVA(RA.CurrentOp()));
			//setOpField();
			//pAnal->setCurrentFieldRef(stubFault.field());
			//setOpField(fieldRef(mpCurOp));
			MyStream ss;
			FI.WriteFuncProfileFromCall(RA.CurrentOp(), ss);// , pAnal);
			FI.guix().GuiOnFunctionCallStop(ss);//GUI will not respond until awaken up the stack
			return STAGE_STATUS_e::CONTINUE;
		}
		return STAGE_STATUS_e::DONE;
	}
};

////////////////////////////////////////////////////////////
class DataInboundStage_t : public PrimaryAnalysis_t::Stage_t
{
	bool	mbLocalsOnly;
public:
	DataInboundStage_t(PrimaryAnalysis_t &rBoss, bool b)
		: Stage_t(rBoss, "DIN"),
		mbLocalsOnly(b)
	{}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		PathOpTracer_t tr;
		AnlzXDeps_t an(FI, mbLocalsOnly, tr);
		an.setRedumpCache(&mrBoss.RedumpCache());
		//an.setRangedOnly(true);
		if (!mbLocalsOnly)
			return an.run();
		an.PathTouchXDeps();
		return STAGE_STATUS_e::DONE;
	}
};

///////////////////////////////////////////////////////// data structures recovery
class DerefPtrStage_t : public PrimaryAnalysis_t::Stage_t
{
public:
	DerefPtrStage_t(PrimaryAnalysis_t &rBoss) : Stage_t(rBoss, "DAT"){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		PathOpTracer_t tr;
		AnalyzePtrs_t an(FI, tr);
		STAGE_STATUS_e e(an.run());//mrBoss.CacheArgs());
		FuncDefPtr()->flags() |= FDEF_DATA_DONE;//this will prevent data types recovery at subsequent re-runs
		return e;
	}
};

////////////////////////////////////////////////////////
class SavedRegsStage_t : public PrimaryAnalysis_t::Stage_t
{
public:
	SavedRegsStage_t(PrimaryAnalysis_t &rBoss) : Stage_t(rBoss, "REG"){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		PathOpTracer_t tr;
		InitialTracer_t an(FI, tr);
		return an.CheckSpoiltRegs();
	}
};

//////////////////////////////////////////////////////////
class PostInboundStage_t : public PrimaryAnalysis_t::Stage_t
{
public:
	PostInboundStage_t(PrimaryAnalysis_t &rBoss) : Stage_t(rBoss, "IRE"){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		return FI.AnalizeIrreducibleExpressions();
	}
};

//////////////////////////////////////////////////////////
class HiddenStage_t : public PrimaryAnalysis_t::Stage_t
{
public:
	HiddenStage_t(PrimaryAnalysis_t &rBoss) : Stage_t(rBoss, "HID"){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		PathOpTracer_t tr;
		HiddenTracer_t BT(FI, tr);
		FuncCleaner_t FD(FI);
		STAGE_STATUS_e iRet(BT.AnalizeHidden(FD));
		//preserve existing arguments (possibly from symbols) for reuse down the pipeline
		/*FieldPtr pField;
		while ((pField = FD.TakeOrphanArg()) != nullptr)
			mrBoss.CacheLocalArg(pField);*/
		//assert(!FD.TakeOrphanArg());
		FD.Cleanup();
		FD.CleanupFinal();
		return iRet;
	}
};

////////////////////////////////////////////////////////
class ArgStage_t : public PrimaryAnalysis_t::Stage_t
{
public:
	ArgStage_t(PrimaryAnalysis_t &rBoss) : Stage_t(rBoss, "ARG"){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		return FI.AssureArgList();
	}
};

/////////////////////////////////////////////////////////
class CallStage_t : public PrimaryAnalysis_t::Stage_t
{
public:
	CallStage_t(PrimaryAnalysis_t &rBoss) : Stage_t(rBoss, "CAL"){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		return FI.CheckCallOuts();
	}
};

///////////////////////////////////////////////////////////
class ReturnStage_t : public PrimaryAnalysis_t::Stage_t
{
public:
	ReturnStage_t(PrimaryAnalysis_t &rBoss) : Stage_t(rBoss, "RET"){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		return FI.UpdateRetOps();
	}
};

/////////////////////////////////////////////////////
class UprootStage_t : public PrimaryAnalysis_t::Stage_t
{
	bool mbPreserveVars;
public:
	UprootStage_t(PrimaryAnalysis_t &rBoss, bool bPreserveVars) : Stage_t(rBoss, "UPR"), mbPreserveVars(bPreserveVars){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		return FI.TouchRoots(mbPreserveVars);
	}
};

///////////////////////////////////////////////////////
class LocalOpStage_t : public PrimaryAnalysis_t::Stage_t
{
public:
	LocalOpStage_t(PrimaryAnalysis_t &rBoss) : Stage_t(rBoss, "LOC"){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		PathOpTracer_t tr;
		LocalsTracer_t an(FI, tr);
		//an.setOpTracer(&mrBoss.opTracer());
		return an.MakeLocals();
	}
};

///////////////////////////////////////////////////////
class LocalAttachStage_t : public PrimaryAnalysis_t::Stage_t
{
public:
	LocalAttachStage_t(PrimaryAnalysis_t &rBoss) : Stage_t(rBoss, "VAR"){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		PathOpTracer_t tr;
		LocalsTracer_t an(FI, tr);
		an.setRedumpCache(&mrBoss.RedumpCache());
		//an.setCahchedArgs(&mrBoss.CacheArgs());
		return an.CheckLocals();
	}
};

///////////////////////////////////////////////////////
class SwitchStage_t : public PrimaryAnalysis_t::Stage_t
{
public:
	SwitchStage_t(PrimaryAnalysis_t &rBoss) : Stage_t(rBoss, "SWI"){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		return FI.TouchSwitches();
	}
};

///////////////////////////////////////////////////////
class LogicStage_t : public PrimaryAnalysis_t::Stage_t
{
public:
	LogicStage_t(PrimaryAnalysis_t &rBoss) : Stage_t(rBoss, "LOG"){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		return FI.TouchLogics();
	}
};

///////////////////////////////////////////////////////
class IfStage_t : public PrimaryAnalysis_t::Stage_t
{
public:
	IfStage_t(PrimaryAnalysis_t &rBoss) : Stage_t(rBoss, "IFS"){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		return FI.TouchIfs();
	}
};

///////////////////////////////////////////////////////
class WhileStage_t : public PrimaryAnalysis_t::Stage_t
{
public:
	WhileStage_t(PrimaryAnalysis_t &rBoss) : Stage_t(rBoss, "WHI"){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		return FI.TouchWhiles();
	}
};

///////////////////////////////////////////////////////
class ElseStage_t : public PrimaryAnalysis_t::Stage_t
{
public:
	ElseStage_t(PrimaryAnalysis_t &rBoss) : Stage_t(rBoss, "ELS"){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		return FI.TouchElses();
	}
};

/////////////////////////////////////////////////////// locals distribution
class LocalImplantStage_t : public PrimaryAnalysis_t::Stage_t
{
public:
	LocalImplantStage_t(PrimaryAnalysis_t &rBoss) : Stage_t(rBoss, "LOT"){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		PathOpTracer_t tr;
		LocalsTracer_t anLocals(FI, tr);
		//relocate locals to their appropriate paths

		anLocals.ImplantLocals();
		//?	ReCheckLocals();
		return STAGE_STATUS_e::DONE;
	}
};

///////////////////////////////////////////////////////
class FinalStage_t : public PrimaryAnalysis_t::Stage_t
{
	//bool mbPreserveVars;
public:
	FinalStage_t(PrimaryAnalysis_t &rBoss) : Stage_t(rBoss, "FIN"){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());// , FileDef());
		FI.SetFuncStatusFinished();
		//InitialTracer_t fi(func());
		//fi.UpdateCallers();
		//FI.SetFuncInvalid(&mrBoss.FuncDefRef(), false);
		//ReNumberize();
		mrBoss.FuncDef().m_dc = 0;
		return STAGE_STATUS_e::DONE;
	}
};

///////////////////////////////////////////////////////
class TestStage_t : public PrimaryAnalysis_t::Stage_t
{
public:
	TestStage_t(PrimaryAnalysis_t &rBoss) : Stage_t(rBoss, "TST"){}
	virtual STAGE_STATUS_e run()
	{
		FuncInfo_t FI(DcRef(), FuncDefRef());
		BlocksTracer_t bt(FI);
		bt.UndoPathTree();

		PathOpTracer_t pathOpTracer;
		bt.RedoPathTree(pathOpTracer);
		return STAGE_STATUS_e::DONE;
	}
};

////////////////////////////////////////
// GenerateStage_t


GenerateStage_t::GenerateStage_t(PrimaryAnalysis_t &rBoss)
	: Stage_t(rBoss, "GEN"),
	mpCurSeg(0),
	mFuncTracerData(nullptr, mPathMap)
{
	//mrBoss.AssureMemMgr(mrBoss.OwnerFolder());

	FuncInfo_t FI(DcRef(), FuncDefRef());

	CFieldPtr pField(dockField());
//	if (pField->isClone())
	//	pField = ProjectInfo_s::CloneLead(pField);

	//ROWID row(FI.VA2DA(iProc, pField->Offset()));
	ADDR va(pField->_key());
	m_stack.push_back(Elt_t(va, 0, 0));

	FI.createBodyPath();

	assert(FuncDef().Body()->ops().empty());

	const FieldMap& m(pField->owner()->typeStruc()->fields());
	for (FieldMapCIt it(m.lower_bound(pField->_key())); it != m.end() && KEY(it) == va; ++it)
	{
		pField = VALUE(it);
		FieldPtr pLabel(ProjectInfo_t::EntryField(pField->isTypeProc()));
		if (pLabel && pLabel->isTypeCode())
			mFuncTracerData.miTypeCode = pLabel->type();//save code type for incremental passes
		else if (pField->isTypeThunk())
			mFuncTracerData.miTypeCode = pField->isTypeThunk()->typeThunk()->baseType();
		if (mFuncTracerData.miTypeCode)
			break;
	}
}

GenerateStage_t::~GenerateStage_t()
{
	if (ProtoInfo_t::IsFuncStatusAborted(FuncDefPtr()))
	{
		mrBoss.abort();//stop further processing
		return;
	}
	ProtoInfo_t::SetFuncStatus(FuncDefPtr(), FDEF_DC_PHASE2);
}

void GenerateStage_t::addSubTask(ADDR addr, ADDR addrRef, int flags, bool bFront)
{
CHECK(addrRef == 0x50bec4)
STOP
	//FuncInfo_t FI(DcRef(), FuncDefRef(), FileDef());
	//TypePtr iProc(FI.Owner Proc());
	//ROWID r(addr ? FI.VA2DA(iProc, addr) : 0);
	//ROWID rRef(addrRef ? FI.VA2DA(iProc, addrRef) : 0);
	if (bFront)
		m_stack.push_front(Elt_t(addr, addrRef, flags));
	else
		m_stack.push_back(Elt_t(addr, addrRef, flags));
}

STAGE_STATUS_e GenerateStage_t::run()
{
	if (m_stack.empty())
		return STAGE_STATUS_e::DONE;

	Elt_t node(m_stack.front());
	m_stack.pop_front();

	/*		ToDoNode_t *pNode(dynamic_cast<ToDoNode_t *>(popCommand()));
	if (!pNode)
	{
	//?fprintf(stderr, "Command %s failed.\n", pNode->toStr().c_str());
	return 1;
	}

	ToDoNode_t &node(*pNode);*/
	//fprintf(stdout, "processing %s\n", node.toStr().c_str(), node.addrx.addr);

	Locus_t aLoc;
	
	int iRet(0);
	int modified = 0;
	//?	mpProject->lockWrite(true);
	//switch (node.id())
	{
		DcInfo_t DI(DcRef());
		//case TODO_DECOMPILE:

		if (DI.LocusFromVAEx(node.vaAt, aLoc))//node.arg(1)
		{
//CHECK(aLoc.addr() == 0x50bbe0)
//STOP
			Locus_t aLocRef;
			DI.LocusFromVAEx(node.vaRef, aLocRef);

			FuncInfo_t FI(DcRef(), FuncDefRef());
			PathOpTracer_t tr;
			PCodeTracer_t tracer(FI, tr, *this, mFuncTracerData);
			//tracer.setCahchedArgs(&mrBoss.CacheArgs());

			if (node.iRollBack > 0)//recover from a break with user interference 
				tracer.RemoveLastOps(node.iRollBack);
			else
				tracer.ResetCurrentPath();

			iRet = tracer.GeneratePseudoCode(aLoc, aLocRef.addr());
			if (iRet != 0)
				modified = 1;

			mFuncTracerData = tracer;
		}
		//aLoc.pStruc->type Proc()->ANALIZE();
		//break;
		//default:
		//break;
	}
	//delete pNode;
	//?	mpProject->lockWrite(false);

	/*
	//	mToDoList.clear();
	if ( mbFunc > 1 )
	fprintf(stdout, "%d functions have been processed.\n", funcs_num);
	else
	fprintf(stdout, "Done.\n");
	fflush(nullptr);*/

	if (iRet < 0)
		return STAGE_STATUS_e::FAILED;

	//return modified;
	return m_stack.empty() ? STAGE_STATUS_e::DONE : STAGE_STATUS_e::CONTINUE;
}



/////////////////////////////////// PrimaryAnalysis_t

PrimaryAnalysis_t::PrimaryAnalysis_t(const FuncInfo_t &r, RedumpCache_t &redumpCache)
	: DcInfo_t(r),
	mrRedumpCache(redumpCache),
	mrFuncRef(r.FuncDefRef()),
	mrFuncDef(r.FuncDef()),
	miRet(STAGE_STATUS_e::FAILED),
	mbRedecomp(!mrFuncDef.isStub())
{
CHECK(DockAddr() == 0x100299E)
STOP
	FieldPtr pDockField(DockField());
	FuncInfo_t FI(*this, FuncDefRef());
	unsigned iDecomp(FI.FuncStatus());
#if(0)
	if (!IsExported(pDockField))//???
	if (iDecomp == 0)//a new function? shouldn't it be done at func acqusition?
	{
		FuncProfile_t fp;
		//InitFuncProfile(fp);
		StubInfo_t SI(DcRef());
		StubBase_t stb(pDockField, 0);
		if (SI.CreateFuncProfile(stb, false, fp))//if possible - SHOULDN't IT BE DONE DURING ACQUISITION?
		{
			.FromFuncProfileEx(fp);
			//?pStub->setField(pDockField);
		}
	}
#endif

	FullName_t sFunc(GlobNameFull(FuncDefPtr(), DcInfo_t::E_PRETTY, CHOP_SYMB));
	gui().GuiOnShowProgressInfo(sFunc.join());

	//bool bThisCall(mrFuncDef.thisCallArg(DcInfo_t::OwnerScope(pDockField)) != nullptr);
	//bool bFastCall(mrFuncDef.isFastCall());
	//mCachedArgs.setThisCall(bThisCall);

	//adjust args storages according to a calling convention
	CallingConv_t CC(*this, FuncDefPtr());// bThisCall, bFastCall);

	FieldMap tmpArgs;
	FuncCCArgsCIt<> it(mrFuncDef, CC);
	while (it)
	{
		ri_t r(it.toR_t());
		//FieldPtr pField0((FieldPtr)it.field());
		++it;//advance
		FieldPtr pField(mrFuncDef.takeFrontArg());
		ADDR oldKey(pField->_key());
		//assert(pField == pField0);
		int order(ARG_ORDER_FROM_KEY(oldKey));
		if (order == LOCAL_ORDER_UNKNOWN)
		{
			if (r.ssid == SSID_LOCAL)
				order = LOCAL_ORDER_STACK;
			else
				order = LOCAL_ORDER_REG;
		}
		ADDR newKey(FuncInfo_s::setupKey(r.ssid, r.ofs, order));
		//if (newKey != oldKey)
			pField->overrideKey(newKey);//WARNING: this may disrupt the integrity of a balanced tree, but we don't care - it's disbanded with next statement
//?		CacheLocalArg(pField);
		//pField->setOwnerComplex(nullptr);//!
		if (!tmpArgs.insert_unique(pField).second)
			ASSERT0;
	}

	//set storage for return value(s) according to a current calling convention
	FieldMap tmpRets;
	FuncCCRetsCIt jt(mrFuncDef, CC);
	while (!jt.isAtEnd())
	{
		ri_t r(jt.toR_t());
		++jt;
		FieldPtr pField(mrFuncDef.takeFrontRet());

		ADDR oldKey(pField->_key());
		int order(ARG_ORDER_FROM_KEY(oldKey));
		assert(order == LOCAL_ORDER_RETVAL);
		ADDR newKey(FuncInfo_s::setupKey(r.ssid, r.ofs, order));
		pField->overrideKey(newKey);
		if (!tmpRets.insert_unique(pField).second)
			ASSERT0;
	}

	ProtoInfo_t TI(*this, FuncDefPtr());

	//re-add args
	while (!tmpArgs.empty())
	{
		FieldMapIt it(tmpArgs.begin());
		FieldPtr pField(tmpArgs.take(it));
		TI.AddArgField(pField);
	}
	//re-add rets
	while (!tmpRets.empty())
	{
		FieldMapIt it(tmpRets.begin());
		FieldPtr pField(tmpRets.take(it));
		TI.AddRetField(pField);
	}

	//these are to be determined
	mrFuncDef.setPStackPurge(0);
	mrFuncDef.setFStackPurge(0);
	FieldMap spoilt;
	mrFuncDef.takeSpoiltFields(spoilt);//(!) affects 'Suspicious input dependency...' warning

	DcInfo_t DI(FI, memMgrGlob());
	DI.ClearFieldMap(spoilt);

	FI.SetFuncStatus(FDEF_DC_PHASE1);//ready to go...
}

PrimaryAnalysis_t::~PrimaryAnalysis_t()
{
	DcInfo_t PI(*this, memMgrGlob());
	DcCleaner_t<> BC(PI);
/*	bool bIsStdCall(FuncInfo_t::IsFuncCleanArged(FuncDefPtr()));
	if (bIsStdCall)
	{
		//if a func of stdcall type, restore unused program stack params
		while (!mCachedArgs.empty())
		{
			FieldPtr pField(mCachedArgs.take(mCachedArgs.begin()));
			if (pField->SSIDx() == SSID_LOCAL)
			{
				FuncInfo_t FI(*this, FuncDefRef());
				pField->setOwnerComplex(nullptr);//SSIDx!
				if (FI.InsertLocalArg(pField))//re-insert
					continue;
			}
			BC.destroyField(pField, mrFuncDef.namesMgr());
		}
	}
	else//discard all unused former args - WHAT IF ANALYSIS FAILED?
	{
		BC.clearFields(mCachedArgs, mrFuncDef.namesMgr());
	}*/
	//gui().GuiOnShowProgressInfo("");

	BC.destroyUnrefedTypes();//not in destructor of a base class!
}

//0:failed, 1:completed, 2:continue
int PrimaryAnalysis_t::runOnce()
{
	assert(!empty());
	Stage_t &rStage(*mPipeline.front());
	for (;;)
	{
		miRet = rStage.run();//<0: not dirty
		if (miRet != STAGE_STATUS_e::FAILED)//it may or may not have been dirty
		{
			bool bAbort(false);
			if (miRet != STAGE_STATUS_e::CONTINUE)
			{
				//if (rStage.checkStopper())
				if (ProtoInfo_t::IsFuncStatusAborted(FuncDefPtr()))
					bAbort = true;
				assert(miRet == STAGE_STATUS_e::SKIPPED || miRet == STAGE_STATUS_e::DONE);
				pop();
				//CHECK(empty())
				//STOP
			}
#if(1)
			else if (Project().analyzer()->stopFlag() == StopFlag::RESET)//skip fractions of particular stage (for debugging only)
				continue;
#endif
			if (!bAbort && miRet != STAGE_STATUS_e::FAILED)//could have been set in stage's destructor
				return (empty() ? 1 : -1);//finish or continue with the next stage
		}
		break;
	}// while (!bBreak);
	clean();//cleanup and abort
	return 0;
}

int PrimaryAnalysis_t::run()
{
	for (;;)
	{
		if (empty())
			break;
		if (!runOnce())
			return 0;
		if (Project().analyzer()->stopFlag() != StopFlag::RESET)
			break;
	}
	return 1;
}

void PrimaryAnalysis_t::afterRun()
{
	redump(FuncDefPtr());// ctx, REDUMP_ALL);
	if (!empty())
	{
#if(1)///////////////////////////////////////////////!!!
		if (main().debugMode())
#else
		if (mPipeline.front()->name() != "GEN")//skip multiple passes
#endif
			Project().analyzer()->setStopFlag(StopFlag::PAUSE);
		if (main().debugMode())
			gui().GuiOnShowProgressInfo(currentStageName());
	}
	//else
		//gui().GuiOnShowProgressInfo("");
}

void PrimaryAnalysis_t::populate()
{
	assert(mPipeline.empty());
	gTraceObjLifetime = true;
//if (mbRedecomp) return;

	if (!mbRedecomp)
		push(new GenerateStage_t(*this));
	else
		push(new ReAnalyzeStage_t(*this));
//return;
//if (mbRedecomp) return;
	push(new DataInboundStage_t(*this, false));//trace inbound data dependencies
//return;
	push(new LocalOpStage_t(*this));//produce local ops (SSID_LOCAL)

	push(new DataInboundStage_t(*this, true));//again, after local ops created
	push(new PostInboundStage_t(*this));//before expressions manipulated
	if (!(FuncDefPtr()->flags() & FDEF_DATA_DONE))
		push(new DerefPtrStage_t(*this));//must be processed while all xrefs are intact
//return;
	push(new SavedRegsStage_t(*this));
//return;
	push(new HiddenStage_t(*this));//mark a lost code
	push(new ArgStage_t(*this));//create unused stack args
	push(new CallStage_t(*this));
	push(new ReturnStage_t(*this));
	push(new UprootStage_t(*this, true));//do not uproot the ops with attached fields to preserve types set at AnalizePtrs stage.
	push(new LocalAttachStage_t(*this));//generate local vars, attach to ops, must be called after uproot stage
	push(new UprootStage_t(*this, false));//run uproote again, once all variables are set
//return;
#if(1)
	push(new SwitchStage_t(*this));
	push(new LogicStage_t(*this));
	push(new IfStage_t(*this));
	push(new WhileStage_t(*this));
	push(new ElseStage_t(*this));
#endif
	push(new LocalImplantStage_t(*this));//for local vars, find a location to anchor
	push(new FinalStage_t(*this));
	//push(new TestStage_t(*this));
}

/////////////////////////////////////////////// Reanalizer_t

Reanalizer_t::Reanalizer_t(const FuncTracer_t& r)
	: FuncTracer_t(r),
	mCurOp(HOP())
{
}

Reanalizer_t::Reanalizer_t(const FuncInfo_t& fi, PathOpTracer_t& tr)
	: FuncTracer_t(fi, tr),
	mCurOp(HOP())
{
}

STAGE_STATUS_e Reanalizer_t::CheckStatus(HPATH hPath, HPATH hPathFrom)
{
	//if (!hPath)//may happen due to unrecognised instruction???
		//return STAGE_STATUS_e::SKIPPED;

/*#ifdef _DEBUG
	int pathNo(PathNo(hPath));
#endif
CHECK(pathNo == 123)
STOP*/

	if (!hPath || !hPath->isAnalized())
		return STAGE_STATUS_e::CONTINUE;

	HOP hOp(hPath->headOp());
	if (hOp)//exit path does not have ops
	{
		HOP hFromOp(hPathFrom ? hPathFrom->tailOp() : HOP());
		assert(PathOf(hOp) == hPath && IsAnalized(hOp));
		if (!CheckAlreadyAnalized(hOp, hFromOp))
			SetFuncStatus(FDEF_DC_ERROR);
	}

	return STAGE_STATUS_e::SKIPPED;
}

STAGE_STATUS_e Reanalizer_t::ReAnalyzePath(HPATH hPath, HPATH hPathFrom, HOP hStartOp)
{
	if (!hPath)//may happen due to unrecognised instruction
	{
		SetFuncStatus(FDEF_DC_ERROR);
		return STAGE_STATUS_e::FAILED;
	}

#ifdef _DEBUG
	int pathNo(PathNo(hPath));
#endif
CHECK(pathNo == 123)
STOP

	assert(!hStartOp || PathOf(hStartOp) == hPath);

	HOP hFromOp(hPathFrom ? hPathFrom->tailOp() : HOP());

	PathOpList_t::Iterator jOp(hPath->ops(), hStartOp ? hStartOp : hPath->headOp());

	if (!hStartOp)//if not re-analyzing
	{
		assert(!hPath->isAnalized());
		/*{
			if (jOp)
			{
				HOP hOp(jOp.data());
				assert(PathOf(hOp) == hPath && IsAnalized(hOp));
				if (!CheckAlreadyAnalized(hOp, hFromOp))
				{
					SetFuncStatus(FDEF_DC_ERROR);
					//ON_EXIT(mhOpPr);
					//return STAGE_STATUS_e::FAILED;
				}
			}
			//mFlowIt.drop();
			return STAGE_STATUS_e::SKIPPED;
		}*/

		hPath->setAnalized(true);

		if (!jOp)
		{
			//mFlowIt.drop();
			return STAGE_STATUS_e::CONTINUE;// SKIPPED;
		}
	}
	else if (hPath->headOp() != hStartOp)
	{
		hFromOp = PrevOp(hStartOp);
	}

	InitialTracer_t IT(*this);

	do {
		mCurOp = jOp.data();

		if (IT.CheckCall(mCurOp) != InitialTracer_t::CALL_ERROR)//may throw
		{
			STOP
		}

		if (IT.Analize(mCurOp, hFromOp) < 0)
		{
			return STAGE_STATUS_e::FAILED;//critical error
		}
		hFromOp = PRIME(mCurOp);
		++jOp;

	} while (jOp);

	mCurOp = HOP();
	return STAGE_STATUS_e::CONTINUE;
}

void Reanalizer_t::OutputDebugInfo(HPATH hPath)
{
	if (hPath)
	{
		int pathNo(PathNo(hPath));
//CHECK(pathNo == 66)
//STOP
		if (hPath->isAnalized())
		{
			//OutputDebugString(MyStringf("\tdrop=%d\n", pathNo).c_str());
			//fprintf(stdout, "\tdrop=%d\n", pathNo);
		}
		else
		{
			//OutputDebugString(MyStringf("path=%d\n", pathNo).c_str());
			//fprintf(stdout, "path=%d\n", pathNo);
		}
	}
}


//////////////////////////////////////////////////////////
// Decompiler_t

Decompiler_t::Decompiler_t(Dc_t &riDC, GlobPtr iGlob)
	: mrMain(riDC.project().mainx()),
	mrFile(*iGlob->folder()),
	miRet(0),
	m_prim(FuncInfo_t(riDC, *iGlob), mRedumpCache)
{
	m_timer.start();
	//FuncInfo_t::SetFuncStatus(0);//reset
	//m_prim.FuncDefRef().flags() &= ~TYP_FDEF_NCONV;//reset at beggining
//?	FuncInfo_t::SetFuncInvalid(m_prim.FuncDefPtr(), true);
	//FuncInfo_t::SetFuncStatus(FDEF_DC_PHASE1);
}

Decompiler_t::~Decompiler_t()
{
	//need to update a stub
	FuncInfo_t FI(m_prim, m_prim.FuncDefRef());
	if (m_prim.result() == STAGE_STATUS_e::DONE)
	{
		//FuncProfile_t fp(m_prim);
		//m_prim.GetFuncProfile(fp);
		//ostringstream ss;
		//m_prim.dump_trimmed(fp, ss);
#if(0)
		StubInfo_t SI(m_prim.DcRef());
		Stub_t &rStub(SI.InsertStub(m_prim.DockAddr(), REFMODE_DIRECT, ""));
		//rStub.setField2(m_prim.DockField());
		rStub.setModified(true);
#endif
	}
	else if (m_prim.result() == STAGE_STATUS_e::FAILED)
	{
#if(0)
		FI.Purge();//shouldn't it be done for bad functions at beginning of decomp?
#endif
	}

	if (!ProtoInfo_t::IsFuncStatusFinished(m_prim.FuncDefPtr()))
		if (!FI.IsStub())
			FI.SetFuncStatus(FDEF_DC_ERROR);

	FuncCleaner_t FD(FI);
	FD.PurgeCallOuts(mRedumpCache);
	mRedumpCache.clear();//everything else
	FD.PurgeDanglingVars();
	FD.PurgeDanglingArgs();
	FD.CleanupFinal();

	m_prim.redump(FI.FuncDefPtr());

	if (mrMain.options().bTimeStats)
		fprintf(stdout, "*** elapsed: %g sec\n", m_timer.elapsed() / 1000.0);

	/*if (mbAll)
	{
		Locus_t aLoc;
		ProbeEx_t* pCtx(new ProbeEx_t(aLoc));
		pCtx->setFolder(FI.OwnerFolder());
		pCtx->setScope(FI.FuncDefPtr());
		FI.main().postEvent(new adc::CEventCommand(pCtx, "dc -all"));
	}*/
}

int Decompiler_t::process()
{
#if(1)
	miRet = m_prim.runOnce();
#else
	miRet = m_prim.run();
#endif
	//if (miRet)
	//ProbeEx_t ctx;
	//ctx.setFolder(&mrFile);
	m_prim.afterRun();
	return 1;
}

bool Decompiler_t::writeToDoList(MyStreamBase &ss)
{
	return m_prim.dump(ss) > 0;
}

I_Context *Decompiler_t::makeContext() const
{
	Locus_t aLoc;
	getLocus(aLoc);

	ProbeEx_t *pCtx(new ProbeEx_t(aLoc));
	pCtx->setFolder(currentFile());
	//?pCtx->setScope(GlobPtr());
	return pCtx;
}

void Decompiler_t::getLocus(Locus_t &aLoc) const
{
	FuncInfo_t FI(m_prim, FuncRef());
	if (m_prim.currentOp())
		FI.LocusFromOp(m_prim.currentOp(), aLoc);
	else
		FI.LocusFromVA_2(FI.OwnerProc(), FI.DockAddr(), aLoc);
}


