#pragma once

#include "type_funcdef.h"
#include "anal_pcode.h"

struct RedumpCache_t;

class PrimaryAnalysis_t : public DcInfo_t	//WARNING: cannot be FuncInfo_t because of its life span and TLS stuff!
{
public:
	class Stage_t
	{
	protected:
		PrimaryAnalysis_t &mrBoss;
		MyString mName;
		ADDR mVA;
		HOP mpOp;
		FieldPtr mpField;
		bool	mbStopper;//indicate a last stage in pipeline
	public:
		Stage_t(PrimaryAnalysis_t &rBoss, MyString name = "")
			: mrBoss(rBoss),
			mName(name),
			mVA(0),
			mpOp(HOP()),
			mpField(0)//,
			//mbStopper(false)
		{
		}
		virtual ~Stage_t() {}
		virtual STAGE_STATUS_e run() { return STAGE_STATUS_e::FAILED; }
		virtual MyString statusStr() const { return name(); }
		const MyString &name() const { return mName; }
		void setVA(ADDR va) { mVA = va; }
		void setOp(HOP pOp) { mpOp = pOp; }
		void setOpField(FieldPtr p) { mpField = p; }
		HOP op() { return mpOp; }
		ADDR VA() { return mVA; }
		FieldPtr opField() { return mpField; }
		//void setStopper(){ mbStopper = true; }
		//bool checkStopper() const { return mbStopper; }
	protected:
		Main_t& mainx() const { return mrBoss.main(); }
		Dc_t &DcRef() const { return mrBoss.DcRef(); }
		FieldPtr dockField() const { return mrBoss.DockField(); }
		//TypePtr ownerProc() const { return mrBoss.Owner Proc(); }
		FuncDef_t& FuncDef() const { return mrBoss.FuncDef(); }
		GlobObj_t& FuncDefRef() const { return mrBoss.FuncDefRef(); }
		GlobPtr FuncDefPtr() const { return mrBoss.FuncDefPtr(); }
		FileDef_t &FileDef() const { return mrBoss.FileDef(); }
		Project_t& proj() { return mrBoss.Project(); }
		//TypePtr projRef() const { return mrBoss.ProjRef(); }
		//FuncInfo_t funcInfo() const { return mrBoss.funcInfo(); }
	};

private:
	GlobObj_t &mrFuncRef;//FuncDef_t
	FuncDef_t &mrFuncDef;
	std::list<Stage_t *> mPipeline;
	STAGE_STATUS_e	miRet;
	//FieldMap	mCachedArgs;//preserved old args
	//FieldMap	mCachedRets;//preserved old rets - don't have to do this.
	bool	mbRedecomp;
	RedumpCache_t &mrRedumpCache;

public:
	PrimaryAnalysis_t(const FuncInfo_t &, RedumpCache_t &);
	~PrimaryAnalysis_t();

	void populate();
	FileDef_t& FileDef() const { return *mrFuncRef.folder()->fileDef(); }
	FuncInfo_t funcInfo() const { return FuncInfo_t(DcRef(), FuncDefRef()/*, FileDef()*/); }
	FieldPtr DockField() const { return DcInfo_s::DockField(FuncDefPtr()); }
	ADDR DockAddr() const { return DcInfo_s::DockAddr(FuncDefPtr()); }
	TypePtr OwnerProc() const { return DcInfo_s::OwnerProc(&mrFuncRef); }
	FuncDef_t& FuncDef() const { return mrFuncDef; }
	//OpTracer_t &opTracer(){ return m_optr; }
	GlobObj_t& FuncDefRef() const { return mrFuncRef; }
	GlobPtr FuncDefPtr() const { return &mrFuncRef; }
	//FieldMap &CacheArgs(){ return mCachedArgs; }
	RedumpCache_t &RedumpCache(){ return mrRedumpCache; }
/*	void CacheLocalArg(FieldPtr p){ 
		if (!mCachedArgs.insert_unique(p).second)
			ASSERT0;
	}*/
	//void CacheLocalRet(FieldPtr p){ mCachedRets.insert_unique(p); }

	//0:failed, 1:completed, 2:continue
	int runOnce();
	int run();
	bool empty()
	{
		return mPipeline.empty();
	}
	void clean()
	{
		while (!empty())
			pop();
	}
	void pop()
	{
		delete mPipeline.front();
		mPipeline.pop_front();
	}
	void push(Stage_t *pStage)
	{
		mPipeline.push_back(pStage);
	}
	int dump(MyStreamBase &ss)
	{
		int iCount(0);
		MyStreamUtil ssh(ss);
		for (std::list<Stage_t *>::iterator i(mPipeline.begin()); i != mPipeline.end(); i++, iCount++)
			ssh.WriteString((*i)->statusStr());
		return iCount;
	}
	ADDR currentVA() const
	{
		if (mPipeline.empty())
			return 0;
		return mPipeline.front()->VA();
	}
	HOP currentOp() const
	{
		if (mPipeline.empty())
			return HOP();
		return mPipeline.front()->op();
	}
	FieldPtr currentOpField() const
	{
		if (mPipeline.empty())
			return nullptr;
		return mPipeline.front()->opField();
	}
	virtual void setCurrentFieldRef(FieldPtr p)
	{
		if (!mPipeline.empty())
			mPipeline.front()->setOpField(p);
	}
	const char *currentStageName() const
	{
		if (!mPipeline.empty())
			return mPipeline.front()->name().c_str();
		return "?";
	}
	STAGE_STATUS_e result() const { return miRet; }
	void abort() { miRet = STAGE_STATUS_e::FAILED; }

	size_t scheduled() const { return mPipeline.size(); }
	void afterRun();
};


class GenerateStage_t : public PrimaryAnalysis_t::Stage_t
{
	Seg_t			*mpCurSeg;
	PathMap			mPathMap;
	PCodeTraceData	mFuncTracerData;
	struct Elt_t
	{
		ADDR vaAt;
		ADDR vaRef;
		int iRollBack;//how many ops to rolback
		Elt_t() : vaAt(0), vaRef(0), iRollBack(0) {}
		Elt_t(ADDR at, ADDR ref, int rb) : vaAt(at), vaRef(ref), iRollBack(rb) {}
	};
	std::list<Elt_t> m_stack;

public:
	GenerateStage_t(PrimaryAnalysis_t &rBoss);
	virtual ~GenerateStage_t();
	void addSubTask(ADDR addr, ADDR addrRef, int flags = 0, bool bFront = false);
	virtual STAGE_STATUS_e run();
	virtual MyString statusStr() const 
	{
		MyString s(name());
		if (!m_stack.empty())
		{
			s.append(MyStringf(" %X", m_stack.back().vaAt));
			if (m_stack.back().vaRef)
				s.append(MyStringf(" (ref:%X)", m_stack.back().vaRef));
		}
		return s;
	}
	TypePtr typeCode() const { return mFuncTracerData.miTypeCode; }
};


struct RedumpCache_t
{
	//typedef std::pair<OpPtr, FieldPtr>		CallOutValue;
	//typedef std::map<OpPtr, OpPtr>	CallOutMap;//callout => perm callop

	//std::set<OpPtr>	trans;//transient op refs args,vars,callargs?
	std::map<OpPtr, FieldPtr>	perm;//permanent(+callouts) -> trasient op refs map
	std::map<OpPtr, OpPtr>	callout;//calloutop -> {permanent (callop) //            : transient (varop)}
	void clear()
	{
		//trans.clear();
		perm.clear();
		callout.clear();
	}
	FieldPtr findPermOp(OpPtr p) const//given a perm op, return a trans
	{
		std::map<OpPtr, FieldPtr>::const_iterator i(perm.find(p));
		if (i != perm.end())
			return i->second;
		return nullptr;
	}
	FieldPtr takeCallout(std::map<OpPtr, OpPtr>::iterator i)//returns varop
	{
		OpPtr hOutOp(i->first);
		std::map<OpPtr, FieldPtr>::iterator j(perm.find(hOutOp));
		if (j == perm.end())
			return nullptr;
		FieldPtr pField(j->second);
		callout.erase(i);
		perm.erase(j);
		return pField;
	}
	bool insertPermOp(OpPtr pOp, FieldPtr pLocal)//or callout
	{
		return perm.insert(std::make_pair(pOp, pLocal)).second;
	}
	void insertCallOut(OpPtr hOutOp, OpPtr hCallOp, FieldPtr second)
	{
		if (!callout.insert(std::make_pair(hOutOp, hCallOp)).second)
			ASSERT0;
		if (!insertPermOp(hOutOp, second))
			ASSERT0;
	}
};

///////////////////////////////////////////////////Reanalizer_t
class Reanalizer_t : public FuncTracer_t
{
	//HOP mFromOp;
	HOP mCurOp;

public:
	Reanalizer_t(const FuncTracer_t&);
	Reanalizer_t(const FuncInfo_t&, PathOpTracer_t&);

	STAGE_STATUS_e CheckStatus(HPATH, HPATH from);
	STAGE_STATUS_e ReAnalyzePath(HPATH, HPATH from, HOP start);//base analysis
	HOP CurrentOp() const { return mCurOp; }

	void OutputDebugInfo(HPATH);
};


class Decompiler_t : public IAnalyzer
{
	const Mainx_t &mrMain;
	Folder_t& mrFile;
	//FieldPtr mpField;
	int miRet;

	PrimaryAnalysis_t m_prim;
	QxTime m_timer;

	RedumpCache_t mRedumpCache;

public:
	Decompiler_t(Dc_t &iDC, GlobPtr funcDef);
	virtual ~Decompiler_t();

	void populate() {
		m_prim.populate();
	}

	RedumpCache_t &redumpCache(){ return mRedumpCache; }

	virtual int process();

	virtual size_t size() const//number of tasks
	{
		return m_prim.scheduled();
	}
	virtual void writeTask(size_t, MyStreamBase &ss)
	{
		MyStreamUtil ssu(ss);
		ssu.WriteString("later");
	}
	virtual void writeModule(size_t, MyStreamBase &ss)
	{
		MyStreamUtil ssu(ss);
	}
	virtual void writeDA(size_t, MyStreamBase &ss)
	{
		MyStreamUtil ssu(ss);
	}
	virtual bool writeToDoList(MyStreamBase &);
	virtual bool finished() { return m_prim.empty(); }
	virtual ADDR currentVA() const { return m_prim.currentVA(); }
	virtual FolderPtr currentFile() const { return &mrFile; }
	virtual FieldPtr currentOpField() const { return m_prim.currentOpField(); }
	virtual void setCurrentFieldRef(FieldPtr p) { m_prim.setCurrentFieldRef(p); }
	virtual void setContextFile(const char *s) {
		//assert(s == mrFile.m.Name());
	}
	virtual I_Context *makeContext() const;
	virtual void getLocus(Locus_t &) const;

	HOP currentOp() const { return m_prim.currentOp(); }
	FileDef_t& FileDef() const { return m_prim.FileDef(); }
	FuncDef_t& FuncDef() const { return m_prim.FuncDef(); }
	GlobObj_t& FuncRef() const { return m_prim.FuncDefRef(); }
	GlobPtr FuncDefPtr() const { return m_prim.FuncDefPtr(); }
	Dc_t &dc() const { return m_prim.dc(); }
	Dc_t &dcRef() const { return m_prim.DcRef(); }
};




