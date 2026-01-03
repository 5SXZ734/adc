#pragma once

#include "db/anlzbin.h"
#include "db/symbol_map.h"
#include "info_dc.h"
#include "info_file.h"
#include "stubs.h"
#include "front_impl_ex.h"

class PostAnalysis_t;
class I_DebugInfoDumper;

class StageBase_t
{
protected:
	PostAnalysis_t &mr;
	unsigned m_id;
	int m_progress;
public:
	StageBase_t(PostAnalysis_t &r, int id)
		: mr(r), m_id(id), m_progress(-1){}
	virtual ~StageBase_t(){}
	virtual bool run() = 0;
	unsigned id() const { return m_id; }
	int progress() const { return m_progress; }
};

///////////////////////////////////////////////////// (Debug) Types
/*class StageTypes_t : public StageBase_t,
	public IFront_Base<I_ModuleCB>
{
	PostAnalysis_t &mr;
	TypesMap symb;
	ProgressMonitor_t<size_t> PM;
	I_DebugInfoDumper *mpDbgInfoDumper;
	Dc_t &mrDC;
	FolderPtr mpFolder;
public:
	StageTypes_t(PostAnalysis_t &r);
	~StageTypes_t();
protected:
	void init(){}
	void setPprogress(unsigned);
	//in: StageBase_t
	virtual bool run();
	//void run0(){}
	//in: I_DataSourceBase
	virtual size_t dataAt(OFF_t rawOffs, OFF_t size, PDATA) const { assert(0); return 0; }
	virtual OFF_t size() const { assert(0); return 0; }
	//in: I_Module
	virtual void selectFile(const char *path);
	virtual HTYPE NewScope(const char *pTypeName, SCOPE_enum eScope, const char *pFieldName, AttrIdEnum attr);
	virtual HTYPE declTypedef(const char *, HTYPE);
};*/

///////////////////////////////////////////////////////////// exports

class StageExports_t : public StageBase_t
{
	TypePtr iFrontSeg;
	FolderPtr pExpFolder;
	SymbolMap symb;
	ProgressMonitor_t<size_t> PM;
	SymbolMap::Iterator symIt;
public:
	StageExports_t(PostAnalysis_t &);
	~StageExports_t();
protected:
	void init();
	virtual bool run();
	void run0();
};


///////////////////////////////////////////////////////////// imports
class StageImports_t : public StageBase_t
{
	FileDef_t& mrFileImported;
	//MyString sModule;
	TypePtr mpExportingModule;
	SymbolMap symb;
	SymbolMap::Iterator symIt;
	ProgressMonitor_t<size_t> PM;
public:
	StageImports_t(PostAnalysis_t&);
	~StageImports_t();
protected:
	void init();
	virtual bool run();
	void run0();
};


///////////////////////////////////////////////////// StageStubs_t
class StageStubs_t : public StageBase_t
{
public:
	StageStubs_t(PostAnalysis_t&);
protected:
	virtual void init(){}
	virtual bool run();
};



///////////////////////////////////////////////////// StageResources_t
class StageResources_t : public StageBase_t
{
public:
	StageResources_t(PostAnalysis_t&);
protected:
	virtual void init(){}
	virtual bool run();
};


///////////////////////////////////////////////////// StageSignatures_t
class StageSignatures_t : public StageBase_t
{
public:
	StageSignatures_t(PostAnalysis_t&);
protected:
	virtual void init(){}
	virtual bool run();
};








///////////////////////////////////////////////////// Debug info 2
class StageDebugInfo_t : public StageBase_t
{
	I_DebugInfoDumper *mpDbgInfoDumper;
	ProgressMonitor_t<size_t> PM;
	MyString mHint;
	const StubPool_t& mrSP;
	FrontImplSymDump_t mfi;
public:
	StageDebugInfo_t(PostAnalysis_t &, MyString);
	~StageDebugInfo_t();
protected:
	friend class IFront_SymDump;
	virtual bool run() override;
	void resetProgress(const char *, unsigned);
	FrontImplSymDump_t& fi() { return mfi; }
};


class IFront_SymDump : public IFront_Src<I_ModuleCB>
{
	StageDebugInfo_t& mr;
public:
	IFront_SymDump(StageDebugInfo_t& r)
		: IFront_Src<I_ModuleCB>(r.fi()),
		mr(r)
	{
	}
protected:
	//in: I_ModuleCB
	virtual void dump(ADDR va, OFF_t oSymbolName, unsigned uNameMax, unsigned uFlags) override { mr.fi().dumpImpl(va, oSymbolName, uNameMax, uFlags, mr.mrSP); }
	virtual void dump(ADDR va, const char *pSymbolName, unsigned uFlags) override { mr.fi().dumpImpl(va, pSymbolName, uFlags, mr.mrSP); }
	virtual void dumpSrc(const char *) override {}
	virtual void resetProgress(const char *pc, unsigned u) override { mr.resetProgress(pc, u); }

};


enum { STAGE_TYPEINFO, STAGE_EXPORTS, STAGE_IMPORTS, STAGE_DBGINFO, STAGES_TOTAL };

///////////////////////////////////////////////////PostAnalysis_t
class PostAnalysis_t : public IAnalyzer
{
	Dc_t &mrDC;
	std::vector<int>	mProgress;
	std::list<StageBase_t*>	mStages;
	static std::vector<double> gStats;
	StubPool_t	mStubPool;
public:
	////////////////////////////////////////////////////////////////////////
	PostAnalysis_t(ProjectEx_t &rProjRef, unsigned segAffinity, Dc_t &rDC);
	virtual ~PostAnalysis_t();
	Dc_t &dc(){ return mrDC; }
	void addStage(StageBase_t*);
	const StubPool_t &stubsPool() const { return mStubPool; }
	MemoryMgr_t& memMgrGlob() const { return mrDC.memMgr(); }
	DataPtr dataSource() const { return mrDC.moduleRef().dataSourcePtr0(); }
	FileDef_t& addFileEx() {
		DcInfo_t PI(mrDC);
		return *PI.AddFileEx(FPATH_FROM_IMPORTED)->fileDef();
	}
	Project_t& project() const {
		return mrDC.project();
	}
	
	Dc_t* AssureDC(TypePtr);

	void RebuildTypesMap();
	//void RebuildTypesMap(const DcInfo_t &);
	void RebuildTypesMap2(const DcInfo_t &);

protected:
	// IAnalyzer
	virtual size_t size() const//number of tasks
	{
		return 0;
	}
	virtual void writeTask(size_t i, MyStreamBase &ss)
	{
	}
	virtual void writeModule(size_t, MyStreamBase &ss)
	{
	}
	virtual void writeDA(size_t i, MyStreamBase &ss)
	{
	}
	virtual bool writeToDoList(MyStreamBase &ss)
	{
		return true;
	}
	virtual bool finished();
	virtual ADDR currentVA() const { return 0; }
	virtual Folder_t *currentFile() const { return 0; }
	virtual FieldPtr currentOpField() const { return 0; }
	virtual void setContextFile(const char *s) {  }
	virtual I_Context *makeContext() const { return nullptr; }
	virtual void getLocus(Locus_t &) const {}
	virtual void setCurrentFieldRef(FieldPtr ){}

protected:
	virtual int process();
	bool popStage();
public:
	MyPath protoPath(MyString = MyString()) const;
	StubSet& loadSignatures();
	StubSet& loadSignatures(MyString module);
};


