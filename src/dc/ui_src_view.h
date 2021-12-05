#pragma once

#include <algorithm>

#include "shared/table.h"
#include "shared/misc.h"
#include "dump_scan.h"
#include "dump.h"

class Decompiler_t;
class CoreEx_t;
struct SrcJumpData_t;

class MyDumpVisitor : public IDumpScanner_t,
	public SyncObj_t
{
	SetVal_t<ADDR>	mOffs;
	bool		mbVerbose;
	bool		mbOffsSigned;//true if mOffs is signed
public:
	MyDumpVisitor(bool bVerbose = false)
		: mbVerbose(bVerbose),
		mbOffsSigned(false)
	{
	}
	const SetVal_t<ADDR> &offs() const { return mOffs; }
	bool isOffsSigned() const { return mbOffsSigned; }

protected:
	virtual void OnOp(OpPtr p){ mpOp = p; }
	virtual void OnFieldDecl(CFieldPtr p, bool = false){ 
		setField(p); 
		if (isOffsOk(p))
		{
			if (FuncInfo_s::IsLocal(p))
			{
				mOffs = FuncInfo_s::address(p);
				if (FuncInfo_s::isLocalVar(p))
					mbOffsSigned = true;
			}
			else
				mOffs = p->_key();
		}
	}
	virtual void OnFieldDef(CFieldPtr p){ 
		return OnFieldDecl(p);
	}
	virtual void OnGlobDecl(CGlobPtr g){
		setGlob(g);
	}
	virtual void OnGlobDef(CGlobPtr g){
		setGlob(g);
	}

	/*virtual void OnFieldInst(CFieldPtr p){ 
		return OnFieldDecl(p);
	}*/
	virtual void OnFieldGap(CFieldPtr p){
		if (isOffsOk(p))
		{
			mOffs = p->checkGap();
			if (p && FuncInfo_s::isLocalVar(p))
				mbOffsSigned = true;
		}
	}
	virtual void OnLabelDecl(PathPtr p){
		mpPath = p;
	}
	virtual void OnFieldRef(CFieldPtr p){ setField(p); }
	virtual void OnConstRef(CFieldPtr p){ setField(p); }
	virtual void OnImpFieldRef(CFieldPtr p){ setField(p); }
	virtual void OnImpGlobDecl(CGlobPtr p){ setGlob(p); }
	virtual void OnImpClsGlob(CGlobPtr p) {
//CHECKID(p, 5121)
//STOP
		setGlob(p);
	}
	virtual void OnTypeRef(CTypePtr p, bool){
		if (!mpType)//multiple typerefs on a line in class definition (with inheritance)
			mpType = (TypePtr)p;
	}
	virtual void OnStrucDecl(CTypePtr p, bool)
	{
		mpType = (TypePtr)p;
	}
	virtual void OnStrucEnd(CTypePtr p){
		mOffs = p->size();
	}
	virtual void OnFuncDecl(CGlobPtr g){
		//if (mbVerbose)
			//fprintf(stdout, "found: %d\n", p->ID());
		//mpType = (GlobToTypePtr)g;
		setGlob(g);
	}
	virtual void OnVFuncDecl(CGlobPtr g, int off){
		OnFuncDecl(g);
		if (off >= 0)
			mOffs = off;
	}
	virtual void OnVTableDecl(CGlobPtr g, int off){
		setGlob(g);
		if (off >= 0)
			mOffs = off;
	}
	virtual void OnImpVTableDecl(CGlobPtr g, int off){
		setGlob(g);
		if (off >= 0)
			mOffs = off;
	}
	virtual void OnFuncDefinition(CGlobPtr g)
	{
		//if (mbVerbose)
			//fprintf(stdout, "found: %d\n", p->ID());
		//mpType = (GlobToTypePtr)g;
		setGlob(g);
	}
private:
	bool isOffsOk(CFieldPtr p) const {
		return (p && !p->isTypeCode() && p->owner() && (p->owner()->isShared() || p->owner()->typeStrucLoc()));
	}
};

class SrcViewModel_t : public adcui::ISrcViewModel
{
	CoreEx_t &mrCore;
	const Dc_t &mrDcRef;
	//Main_t &mrMain;
	const Folder_t &mrFile;
	//bool mbHeader;
	size_t	mDispId;
	size_t	mModelId;
//?	MyString	mData;
	MyColumnVec	mcols;
	struct Data
	{
		int	i;
		Data() : i(0){}
	};

	typedef adcui::DUMPOS DUMPOS;
	typedef adcui::ITER ITER;

	CoolIter<Data, DUMPOS, ITER>	mPos;

	//static RefPtr_t<ProbeEx_t> mcxRef;
	//bool mbSyncMode;
	
	TypeBasePtr mpSubject;

	MyString	mFilePath;//cached
	MyString	mSubjectName;//cached

	/*class ReadLocker
	{
		SrcViewModel_t *mpModel;
	public:
		ReadLocker(SrcViewModel_t *pModel) : mpModel(pModel){ mpModel->lockRead(true); }
		~ReadLocker(){ mpModel->lockRead(false); }
	};

	class WriteLocker
	{
		SrcViewModel_t *mpModel;
	public:
		WriteLocker(SrcViewModel_t *pModel) : mpModel(pModel){ mpModel->lockWrite(true); }
		~WriteLocker(){ mpModel->lockWrite(false); }
	};*/

public:
	SrcViewModel_t(CoreEx_t &rCore, const Dc_t &rDcRef, const Folder_t &rFile, adcui::UDispFlags flags);
	virtual ~SrcViewModel_t();

	FileDef_t &FileDef() const;
	FolderPtr folder() const {
		return (FolderPtr)&mrFile;
	}

	virtual void tipInfoIt(adcui::DUMPOS it, int x, MyStreamBase &ss);
	virtual int lineFromItEx(adcui::DUMPOS it, MyStreamBase &ss);
	virtual void lockRead(bool bLock);
	virtual void lockWrite(bool bLock);
	void deleteDisplay();
	DisplayUI_t *newDisplay(CFolderPtr pFile, adcui::UDispFlags);
	DisplayUI_t *display() const;
	bool InvalidateDump();
protected:
	inline int &iter(adcui::ITER it)
	{
		return mPos.fromIter(it)->i;
	}
	inline int &dumpos(adcui::DUMPOS iPos) const
	{
		return mPos.fromPos(iPos)->i;
	}
	virtual adcui::DUMPOS newPosition();
	virtual void deletePosition(adcui::DUMPOS iPos);
	virtual adcui::DUMPOS posFromIter(ITER it);
	virtual ITER newIterator(adcui::DUMPOS iPos);
	virtual void deleteIterator(ITER it, bool bUpdatePos);
	virtual void copyIt(adcui::DUMPOS itTo, adcui::DUMPOS itFrom);
	virtual void seekLineIt(adcui::DUMPOS it, int line);
	virtual int lineFromIt(adcui::DUMPOS it);
	virtual bool backwardIt(adcui::ITER it);
	virtual bool forwardIt(adcui::ITER it);
	virtual void invalidate(bool b);
	const char *getDumpPosAtIt(adcui::DUMPOS it, DumpContext_t *pctx) const;
	//bool validateIt(adcui::DUMPOS it);
	virtual const char *dataIt(adcui::DUMPOS it, bool bPlain = false);//not used
	virtual int checkEqual(adcui::DUMPOS it1, adcui::DUMPOS it2);
	virtual int linesNum(){ return display()->linesTotal(); }
	virtual int charsNum(){ return 256; }
	virtual bool atEndIt(adcui::DUMPOS it);

	//IADCTextModel

	virtual const char *cellDataIt(COLID col, adcui::DUMPOS it, bool bPlain = false);//not used
	virtual void getRowDataIt(adcui::DUMPOS it, adcui::IADCTableRow &aTab);
	virtual void getColumnInfo(adcui::IADCTableRow &aRow, bool bNoData);

	virtual bool setCellDataIt(COLID col, adcui::DUMPOS, const char *) { return false; }
	virtual const char *cellTreeDataIt(COLID col, adcui::DUMPOS){ return nullptr; }
	virtual int columnWidth(COLID col){ return mcols[col].width; }
	virtual void setColumnWidth(COLID col, int w){ mcols[col].width = w; }
	virtual const char *colName(COLID col){ return mcols[col].name.c_str(); }
	virtual unsigned colFlags(COLID col) const { return mcols[col].flags; }
	virtual void setColFlags(COLID col, unsigned f){ mcols[col].flags = f; }
	virtual int colColor(COLID col){ return mcols[col].color; }
	virtual int colsNum(){ return (int)mcols.size(); }
	virtual int colColorIt(COLID, adcui::DUMPOS){ return 0; }

	virtual void setCurPosIt(adcui::DUMPOS it, int x);
	virtual bool seekPosIt(const char *pc, adcui::DUMPOS it, adcui::DUMPOS);
	//virtual bool setSyncMode(bool bOn);
	virtual adcui::IADCTextEdit *startEditIt(adcui::DUMPOS it, int /*x*/);

	//ISrcViewModel

	virtual adcui::UDispFlags mode() const { return display()->flags(); }

	SyncObj_t getClosestObj(adcui::DUMPOS it0);
	bool visit(const char *pc, MyDumpVisitor &a, GlobPtr iFuncDef) const;
	bool visit(adcui::DUMPOS it, MyDumpVisitor &a, bool bCont = false) const;

	virtual int setMode(adcui::UDispFlags flags, bool bSet, adcui::DUMPOS itSeek);
	virtual int Redump(adcui::DUMPOS itSeek, bool bResetIterators);
	virtual bool IsRedumpPending();
	virtual int fileId() const;
	virtual const char *filePath() const { return mFilePath.c_str(); }
	virtual bool getModuleName(MyStreamBase &) const;
	virtual bool IsHeader() const {
		return mode().testL(adcui::DUMP_HEADER); }

public:
	bool seekOpRef(OpPtr, adcui::DUMPOS);
	bool seekFieldRef(CFieldPtr, adcui::DUMPOS);
	bool seekTypeRef(CTypePtr, adcui::DUMPOS);
	bool seekGlobRef(CGlobPtr, adcui::DUMPOS);
	void setSubject(CGlobPtr);
	//virtual bool synchronizeIt(adcui::DUMPOS it, adcui::ISrcViewModel *pIOther, adcui::DUMPOS otherIt);
	//ADDR getVA(adcui::DUMPOS it) const;
	//virtual bool getAddress(adcui::DUMPOS it, MyStreamBase &ss);
	virtual void gotFocus(bool bGot);
	virtual bool checkTaskTop(adcui::DUMPOS it);
	virtual adcui::PixmapEnum pixmapIt(adcui::DUMPOS it);
	bool initializeJumpTarget(SrcJumpData_t &target, const SrcJumpData_t &back);
	virtual bool initiateJump(bool bToDefinition, int linesFromTop);
	virtual bool jump(adcui::DUMPOS, bool fwd, bool flip);
	virtual void printDumpInfo();
	virtual bool setSubject(const char *);
	virtual const char *subjectName() const { return mSubjectName.c_str(); }

private:
	bool jump(const SrcJumpData_t&, adcui::DUMPOS);

private:
	Project_t &project() const;
	ProjectEx_t &projectx() const;
	const Dc_t &dc() const { return mrDcRef; }
	Decompiler_t *decompiler() const;
	Main_t &main() const;
};

