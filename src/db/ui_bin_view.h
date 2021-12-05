#pragma once

#include "main.h"
#include "dump_bin.h"
#include "ui_main.h"


class FakeScope_t;

class BinViewModel_t : public adcui::IBinViewModel
{
protected:
	class MyCACHEbin_t : public CACHEbin_t
	{
		Core_t &mrCore;
		CTypePtr	miModule;
	public:
		MyCACHEbin_t(CTypeBasePtr iScope, Core_t &rCore, CTypePtr iModule)
			: CACHEbin_t(iScope),
			mrCore(rCore),
			miModule(iModule)
		{
			mrCore.aquireDumpTarget(miModule);
		}
		~MyCACHEbin_t()
		{
			mrCore.releaseDumpTarget(miModule);
		}
		virtual DumpTarget_t *ptarget() const
		{
			DumpInvariant_t *pInv(mrCore.dumpInvariant(miModule));
			return pInv->mpTarget;
		}
	};
protected:
	Core_t&	mrCore;
	Main_t&		mrMain;
	CTypePtr		miModule;
	//FolderPtr		mpFolder;
	FakeScope_t*	mpFakeModule;
	CTypeBasePtr	mpScopeRoot;
	CTypePtr		mpScope;
	MyCACHEbin_t	mDrawObj;
	//RefPtr_t<Probe_t>	mcxRef;
	static const int SB_MAX = 32767;
	int miLocked;
	MyString ms;//temporary buffer

/*private:

	class ReadLocker
	{
		BinViewModel_t *mpModel;
	public:
		ReadLocker(BinViewModel_t *pModel) : mpModel(pModel){ mpModel->lockRead(true); }
		~ReadLocker(){ mpModel->lockRead(false); }
	};

	class WriteLocker
	{
		BinViewModel_t *mpModel;
	public:
		WriteLocker(BinViewModel_t *pModel) : mpModel(pModel){ mpModel->lockWrite(true); }
		~WriteLocker(){ mpModel->lockWrite(false); }
	};*/

public:
	BinViewModel_t(Core_t &rCore, Main_t &rMain, CTypePtr module, CTypeBasePtr ccope, bool bStrucDump);
	virtual ~BinViewModel_t();

	CTypePtr module() const{
		return miModule;
	}

	CTypeBasePtr scope() const {
		return mpScopeRoot;
	}

	/*TypeObj_t &binaryRef() const {
		return *miModule;
	}*/

	virtual const char *moduleName() const;

	//void reset(Project_t *pProject)
	//bool seekAddrIt(adcui::ITER it, ADDR addr);
	//int infoIt(){ return mInfoIt; }
	//void clear(){ reset(nullptr); }

protected:

	virtual adcui::DUMPOS newPosition();
	virtual void deletePosition(adcui::DUMPOS iPos);
	virtual adcui::DUMPOS posFromIter(adcui::ITER it);
	virtual adcui::ITER newIterator(adcui::DUMPOS iPos);
	virtual void deleteIterator(adcui::ITER it, bool bUpdatePos);
	virtual void lockRead(bool bLock);
	void lockWrite(bool bLock);
	virtual void seekLineIt(adcui::DUMPOS it, int line);
	virtual int lineFromIt(adcui::DUMPOS it);
	virtual void copyIt(adcui::DUMPOS itTo, adcui::DUMPOS itFrom);
	virtual void setBitPosition(adcui::DUMPOS itPos);
	virtual bool backwardIt(adcui::ITER it);
	virtual bool forwardIt(adcui::ITER it);
	DA_t toDA(adcui::DUMPOS it);
	virtual int checkEqual(adcui::DUMPOS it1, adcui::DUMPOS it2);//returns like strcmp
	virtual int linesNum(){
		return SB_MAX;
	}
	virtual int charsNum(){ 
		return 256;
	}
	virtual bool atEndIt(adcui::DUMPOS it);
	virtual int lineIt(adcui::DUMPOS it);
	//virtual int levelIt(adcui::DUMPOS it);
	virtual const char * cellDataIt(COLID iCol, adcui::DUMPOS iPos, bool bPlain = false);
	virtual void getRowDataIt(adcui::DUMPOS iPos, adcui::IADCTableRow &aTab);
	virtual void getColumnInfo(adcui::IADCTableRow &aRow, bool bNoData);
	virtual int checkSelIt(adcui::DUMPOS it);
	virtual int colColorIt(COLID col, adcui::DUMPOS it){
		return adcui::COLOR_NULL;
	}
	virtual bool setCellDataIt(COLID col, adcui::DUMPOS it, const char *){
		return false; 
	}
	virtual const char *cellTreeDataIt(COLID col, adcui::DUMPOS it);
	virtual void invalidate(bool);
	virtual int columnWidth(COLID colID);
	virtual void setColumnWidth(COLID colID, int w);
	//virtual int colId(COLID col){ return mDrawObj.mcolids[col]; }
	virtual const char *colName(COLID col){ return mDrawObj.columnName(col); }
	virtual unsigned colFlags(COLID col) const { return mDrawObj.columnFlags(col); }
	virtual void setColFlags(COLID col, unsigned f){ mDrawObj.setColumnFlags(col, f); }
	virtual int colColor(COLID col){ return mDrawObj.columnColor(col); }
	virtual int colsNum(){ return (int)mDrawObj.colsNum(); }

	virtual void setCurPosIt(adcui::DUMPOS iPos, int x);
	virtual void OnSetCurPos(const Probe_t&){}
	virtual bool locusInfo(MyStreamBase &ss, int &x);
	virtual void updateSelection();
	virtual bool seekPosIt(const char *pc, adcui::DUMPOS it, adcui::DUMPOS itRef);
	void skipEmptyLines(DumpVisitor_t &binDumper, adcui::DUMPOS it);
	virtual adcui::IADCTextEdit *startEditIt(adcui::DUMPOS, int /*x*/);
	//virtual bool setSyncMode(bool bOn);
	ROWID viewExtent() const;
	//Complex_t *scopeComplex();
	virtual bool listObjHierarchyAtIt(adcui::DUMPOS it, MyStreamBase &ss);
	virtual void scopeTo(const char *name);
	virtual void scopeName(MyStreamBase &ss);
	
	struct Jump_t
	{
		DA_t rTop;
		DA_t rCur;
		int xCur;
		Jump_t(DA_t top, DA_t cur, int x)
			: rTop(top),
			rCur(cur),
			xCur(x)
		{}
		bool operator==(const Jump_t& x) const
		{
			return rTop == x.rTop && rCur == x.rCur && xCur == x.xCur;
		}
	};

	std::vector<Jump_t> mJumps;

	virtual int pushJumpIt(adcui::DUMPOS iTop, adcui::DUMPOS iCur, int x);
	virtual int popJumpIt(adcui::DUMPOS iTop, adcui::DUMPOS iCur, int *x);
	virtual int checkJumpTopIt(adcui::DUMPOS iTop, adcui::DUMPOS iCur);//to avoid duplications
	//virtual bool listTypesAtIt(adcui::DUMPOS it, MyStreamBase &ss);
	virtual void postCommandIt(adcui::DUMPOS it, const char *cmd);
	virtual bool checkTaskTop(adcui::DUMPOS it);
	virtual adcui::PixmapEnum pixmapIt(adcui::DUMPOS it);
	virtual adcui::Color_t jumpTarget();
	virtual bool setCompactMode(bool bOn);
	virtual bool isCompactMode() const;
	virtual bool setValuesOnlyMode(bool);
	virtual bool isValuesOnlyMode() const;
	virtual bool setResolveRefsMode(bool);
	virtual bool isResolveRefsMode() const;

	virtual void tipInfoIt(adcui::DUMPOS itPos, int x, MyStreamBase &ss);
	virtual bool reloadRawData();

protected:
	//TypePtr binaryRef() const { return mrMain.project().binaryRef(); }
	Project_t &proj() const { return mrMain.project(); }
	TypePtr realScope() const;
};
