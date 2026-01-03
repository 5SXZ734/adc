#pragma once

#include "qx/MyStringList.h"
#include "db/anlz.h"
#include "shared/table.h"
#include "db/proj.h"
#include "dump_file.h"
#include "stubs.h"
#include "type_funcdef.h"
#include "ana_main.h"
#include "info_dc.h"

class StubsViewModel_t : public adcui::IStubsViewModel
{
protected:
	CoreEx_t& mrCore;
	Main_t& mrMain;

	struct Data
	{
		size_t i;
		Data() : i(0){}
	};

	typedef adcui::DUMPOS DUMPOS;
	typedef adcui::ITER ITER;

	CoolIter<Data, DUMPOS, ITER>	mPos;
	MyColumnVec	mcols;
	MyString ms;

	/*class ReadLocker
	{
		ADCStubsViewModelOld *mpModel;
	public:
		ReadLocker(ADCStubsViewModelOld *pModel) : mpModel(pModel){ mpModel->lockRead(true); }
		~ReadLocker(){ mpModel->lockRead(false); }
	};*/

public:

	StubsViewModel_t(CoreEx_t &rCore)
		: mrCore(rCore),
		mrMain(rCore.main())
	{
		using namespace adcui;
		//mcols.setup(COLID_MODULE, "MODULE", 10, CLMNFLAGS_HEADER);
		mcols.setup(COLID_ADDRESS, "ADDRESS", 10, CLMNFLAGS_HEADER);
		mcols.setup(COLID_NAME, "NAME", 16, CLMNFLAGS_HEADER);
		mcols.setup(COLID_STACK_IN, "STACK ARGS", 11);
		mcols.setup(COLID_STACK_OUT, "STACK DIF", 11);
		mcols.setup(COLID_FPU_IN, "FPU ARGS", 9);
		mcols.setup(COLID_FPU_OUT, "FPU DIF", 9);
		mcols.setup(COLID_REG_ARGS, "REG ARGS", 9);
		mcols.setup(COLID_REG_RETS, "RETS/SPOILT", 28);
		mcols.setup(COLID_FLAGS, "FLAGS", 10);
		//mcols.setup(COLID_SAVED_FLAGS, "SAVED FLAGS", 13);
		//mcols.setup(COLID_RET_REGS, "RET REGS", 9);
		mcols.setup(COLID_PROBLEMS, "", -8);//problems
	}

	virtual ~StubsViewModel_t()
	{
	}

	void clear()
	{
		reset(0);
	}

	unsigned headerColsNum() const
	{
		unsigned n(0);
		for (COLID i(0); i < mcols.size(); i++)
			if (colFlags(i) & adcui::CLMNFLAGS_HEADER)
				n++;
		return n;
	}

	void reset(StubMgr_t *pStubsMgr)
	{
		assert(0);
		//mpStubsMgr = pStubsMgr;
	}

	/*bool seekAddrIt(ITER it, ADDR addr)
	{
		if (!mpStubsMgr)
			return nullptr;
		int line(mpStubsMgr->toIndex(addr));
		if (line < 0)
			return false;
		/ *StubIt its(mpStubsMgr->FindStubIt(addr));
		if (its == mpStubsMgr->end())
			return false;
		size_t line(its - mpStubsMgr->begin());* /
		seekLineIt(it, (int)line);
		return true;
	}*/

	ADDR64 imageBase() const
	{
		return 0;
	}

	MyString atAsStringIt(DUMPOS it)
	{
		//if (mpStubsMgr)
		{
			const Stub_t *pa(fromIndex(dumpos(it), nullptr));
			if (pa)
				return pa->atAsString(imageBase());
			/*int i(iter(it));
			if (i < (int)mpStubsMgr->size())
			{
				Stub_t &a(mpStubsMgr->at(i));
				return a.atAsString();
			}*/
		}
		return "";
	}

protected://IADCTextModel
	inline size_t &iter(ITER it)
	{
		return mPos.fromIter(it)->i;
	}
	inline size_t &dumpos(DUMPOS iPos)
	{
		return mPos.fromPos(iPos)->i;
	}

	virtual DUMPOS newPosition()
	{
		return mPos.newPos(new Data);
	}

	virtual void deletePosition(DUMPOS iPos)
	{
		delete mPos.deletePos(iPos);
	}

	virtual DUMPOS posFromIter(ITER it)
	{
		return mPos.posFromIter(it);
	}

	virtual ITER newIterator(DUMPOS iPos)
	{
		return mPos.newIter(iPos, new Data());
	}

	virtual void deleteIterator(ITER it, bool bUpdatePos)
	{
		delete mPos.deleteIter(it, bUpdatePos);
	}

	virtual void copyIt(DUMPOS itTo, DUMPOS itFrom)
	{
		dumpos(itTo) = dumpos(itFrom);
	}

	virtual bool backwardIt(ITER it)
	{
		if (iter(it) > 0)
		{
			iter(it)--;
			return true;
		}
		return false;
	}
	virtual bool forwardIt(ITER it)
	{
		if ((int)iter(it) < linesNum())
		{
			iter(it)++;
			return true;
		}
		return false;
	}
	virtual void seekLineIt(DUMPOS itTo, int line){ dumpos(itTo) = line; }
	virtual int lineFromIt(DUMPOS it){ return (int)dumpos(it); }
	virtual int checkEqual(DUMPOS it1, DUMPOS it2){
		size_t a(dumpos(it1));
		size_t b(dumpos(it2));
		if (a < b)
			return -1;
		if (a > b)
			return 1;
		return 0;
	}
	virtual void lockRead(bool bLock){ mrMain.lockProjectRead(bLock); }
	virtual void lockWrite(bool bLock){ mrMain.lockProjectWrite(bLock); }
	virtual int charsNum(){ return 128; }
	virtual bool atEndIt(DUMPOS it){ return !((unsigned)dumpos(it) < (unsigned)linesNum()); }

	virtual bool seekPosIt(const char *s, DUMPOS it, DUMPOS itRef)
	{
		//ProjectInfoEx_t PX(project());
		//if (mpStubsMgr)
		{
			ADDR va(0);
			Folder_t *pFolder(nullptr);
			ProbeEx_t *pCtx(mrCore.getContextEx());
			if (pCtx)
			{
				pFolder = pCtx->folder();
				mrCore.releaseContext(pCtx);
			}
			if (pFolder)
				pFolder = ProjectInfo_t::TopFolder(*pFolder);
			if (MyString(s) == "$task")
			{
				Decompiler_t *pIAnlz(decompiler());
				if (pIAnlz)
				{
					FieldPtr pField(pIAnlz->currentOpField());
					if (pField)
					{
						if (pField->isExported())
						{
							FieldPtr pExpField(nullptr);//?(PX.ToExportedField(pField));
							/*? if (pExpField)
							{
								va = pExpField->_key();
								pFolder = ProjectInfo_t::TopFolder(*USERFOLDER(pExpField));
							}*/
						}
						else
							va = (ADDR)pIAnlz->currentOpField()->_key();
					}
					else if (pIAnlz->currentOp())
						va = pIAnlz->currentVA();
				}
			}
			else if (MyString(s) == "@")
			{
				Obj_t *pObj(nullptr);
				ContextSafeEx_t safex(mrCore);
				if (safex)
				{
					pObj = pCtx->obj();
					if (pObj)
					{
						FieldPtr pField(pObj->objField());
						if (pField && DcInfo_t::IsGlobal(pField))
						{
							if (pField->isExported())
							{
								FieldPtr pExpField(nullptr);//?(PX.ToExportedField(pField));
								/*? if (pExpField)
								{
									va = pExpField->_key();
									pFolder = ProjectInfo_t::TopFolder(*USERFOLDER(pExpField));
								}*/
							}
							else
								va = pField->_key();
						}
					}
				}
				else
				{
					ContextSafe_t<> safe(mrCore);
					if (safe)
						va = pCtx->locus().va();
				}
			}
			else
			{
				value_t n;
				if (StrHex2Int(s, n))
					va = (ADDR)n.ui32;
			}
			if (va)
			{
				int i(toIndex(pFolder, va));
				if (i >= 0)
				{
					dumpos(it) = (int)i;
					return true;
				}
				/*StubIt sit(mpStubsMgr->FindStubIt(va));
				if (sit != mpStubsMgr->end())
				{
					size_t i(sit - mpStubsMgr->begin());
					iter(it) = (int)i;
					return true;
				}*/
			}
		}
		return false;
	}

protected:
	bool checkFuncBody(DcInfo_t &DI, const Stub_t &a, std::ostringstream &ss)
	{
		/*if (a.field() && DI.IsCFunc(a.field()))
		{
			ss << (char)adcui::SYM_COLOR;
			ss << (char)adcui::COLOR_USER_FUNCTION;
			return true;
		}*/
		return false;
	}
	//IADCTableModel
	virtual const char *cellDataIt(COLID col, DUMPOS it, bool bPlain = false)//, cell_info_t *)
	{
#if(1)
		Folder_t *pFolder(nullptr);
		const Stub_t *pStub(fromIndex(dumpos(it), &pFolder));
		if (!pStub || !pFolder)
			return nullptr;

		Dc_t *pDC(DcInfo_t::DcFromFolder(*pFolder));
		if (!pDC)
			return nullptr;

		StubInfo_t dcInfo(*pDC);//?
		const Stub_t &a(*pStub);
		switch (col)
		{
		default:
		{
			col -= headerColsNum();

			MyString s(dcInfo.ValueFromStub(a));
			ValueVec v(s);
			if (col < v.size())
			{
				ms = v[col];
				return ms.c_str();
			}
			break;
		}
		/*case COLID_MODULE:
		{
			 ms = dcInfo.ModuleName();
			 return ms.c_str();
		}*/
		case COLID_NAME:
		{
			std::ostringstream ss;
			/*if (checkFuncBody(dcInfo, a, ss))
				ss << dcInfo.FieldName(a.field());
			else*/
				ss << dcInfo.name(a);
			ms = ss.str();
			return ms.c_str();
		}
		case COLID_ADDRESS:
		{
			if (!dcInfo.IsPhantomFolder(*pFolder))
			{
				std::ostringstream ss;
				checkFuncBody(dcInfo, a, ss);
				ss << a.atAsString(dcInfo.ImageBase());
				ms = ss.str();
			}
			else
				ms.clear();
			return ms.c_str();
		}
		case COLID_PROBLEMS:
		{
			/*?if (a.field() && !a.value().empty())
			{
				ms = "Stub's definition does not match function's profile";
				return ms.c_str();
			}*/
			break;
		}
		}
		return nullptr;
#else
		if (!(iter(it) < linesNum()))
			return nullptr;
		static std::string s;
		s = std::string(fmt("%d:%d", iter(it), col));
		return s.c_str();
#endif
	}

	virtual void getRowDataIt(DUMPOS it, adcui::IADCTableRow &aTab)
	{
		Folder_t *pFolder(nullptr);
		const Stub_t *pStub(fromIndex(dumpos(it), &pFolder));
		if (!pStub || !pFolder)
			return;
		Dc_t *pDC(DcInfo_t::DcFromFolder(*pFolder));
		if (!pDC)
			return;

		for (int i(0); i < COLID__TOTAL; i++)
		{
			aTab.setCellWidth(i, columnWidth(i));
			aTab.setCellFlags(i, colFlags(i));
		}

		StubInfo_t SI(*pDC);//?
		const Stub_t &a(*pStub);
		bool bBody(false);
		MyString sValue(SI.ValueFromStub(a));
		ValueVec v(sValue);
		//set the cells
		if (!SI.IsPhantomFolder(*pFolder))
		{
			if (bBody)
				aTab.addColor(COLID_ADDRESS, adcui::COLOR_USER_FUNCTION);
			aTab.addCell(COLID_ADDRESS, a.atAsString(SI.ImageBase()));
		}

		if (bBody)
			aTab.addColor(COLID_NAME, adcui::COLOR_USER_FUNCTION);
		aTab.addCell(COLID_NAME, /*bBody ? SI.FieldName(a.field()) :*/ SI.name(a));

		for (size_t i(0); i < v.size(); i++)
			aTab.addCell(unsigned(COLID_STACK_IN + i), v[i]);

		/*?if (a.field() && !a.value().empty())
		{
			aTab.addCell(COLID_PROBLEMS, "Stub's definition does not match function's profile");
			aTab.setMarginColor(adcui::COLOR_MARGIN_ERROR);
		}*/

		if (checkTaskTop(it))
			aTab.setLineColor(adcui::COLOR_TASK_TOP);
	}

	virtual void getColumnInfo(adcui::IADCTableRow &aRow, bool bNoData)
	{
		for (int col(0); col < colsNum(); col++)
		{
			int w(columnWidth(col));
			if (w > 0)
			{
				if (!bNoData)
					aRow.addCell(col, colName(col));
				aRow.setCellFlags(col, colFlags(col));
				aRow.setCellWidth(col, w);
			}
		}
	}

	virtual bool setCellDataIt(COLID col, DUMPOS it, const char *cellStr)
	{
		size_t row(dumpos(it));
		FolderPtr pFolder(nullptr);
		const Stub_t *pStub(fromIndex(row, &pFolder));
		if (!pStub || !pFolder)
			return false;

		Dc_t *pDC(DcInfo_t::DcFromFolder(*pFolder));
		if (!pDC)
			return false;

		StubInfo_t SI(*pDC);
		ValueVec vv(SI.ValueFromStub(*pStub));
		vv.setAt(col - headerColsNum(), cellStr);//2 header columns

		ProbeEx_t *pCtx(new ProbeEx_t);
		pCtx->locus().add(SI.ModulePtr(), 0, nullptr);
		SI.PostMakeStub(pStub->atAsString(SI.ImageBase()), vv.toString(), SI.ModulePtr(), pCtx);
		pCtx->Release();
		return true;
	}
	virtual const char *cellTreeDataIt(COLID col, DUMPOS it){ return 0; }
	virtual int columnWidth(COLID col){ return mcols[col].width; }
	virtual void setColumnWidth(COLID col, int w){ mcols[col].width = w; }
	virtual const char *colName(COLID col){ return mcols[col].name.c_str(); }
	virtual unsigned colFlags(COLID col) const { return mcols[col].flags; }
	virtual void setColFlags(COLID col, unsigned f){ mcols[col].flags = f; }
	virtual int colColor(COLID col){ return mcols[col].color; }
	virtual int colsNum(){ return (int)mcols.size(); }
	virtual int colColorIt(COLID, DUMPOS){ return 0; }

	virtual bool checkTaskTop(DUMPOS it)
	{
		if (!hasProject())
			return false;
		Decompiler_t *pIAnlz(decompiler());
		if (!pIAnlz)
			return false;
		FieldPtr pField(pIAnlz->currentOpField());
		OpPtr pOp(pIAnlz->currentOp());
		if (!pField && !pOp)
			return false;
		const Stub_t *pStub(stubAtIt(it));
		if (!pStub)
			return false;
		if (pField)
		{
			if (pField->_key() != pStub->atAddr())
				return false;
		}
		else if (pOp)
		{
			if (pIAnlz->currentVA() != pStub->atAddr())
				return false;
		}
		return true;
	}

protected:
	virtual const Stub_t *fromIndex(size_t i, FolderPtr *ppFolder) const = 0;
	virtual void invalidate(bool) = 0;
	virtual int toIndex(FolderPtr, ADDR) = 0;
	virtual int linesNum() = 0;
private:
	const Stub_t* stubAtIt(DUMPOS it)
	{
		size_t i(dumpos(it));

		Folder_t *pFolder(nullptr);
		const Stub_t *pStub(fromIndex(i, &pFolder));
		if (!pStub || !pFolder)
			return nullptr;

		//StubMgr_t &x(mrStubsMgr);
		//return x.fromIndex(dumpos(it));
		return pStub;
	}
	bool hasProject() const { return mrMain.hasProject(); }
	ProjectEx_t &project() const { return reinterpret_cast<ProjectEx_t &>(mrMain.project()); }
	//Dc_t &dc() const { return mrDcRef; }
	//TypePtr dcRef() const { return &mrDcRef; }
	Decompiler_t *decompiler() const { return dynamic_cast<Decompiler_t *>(project().analyzer()); }
};

#if(!SBTREE_STUBS)
class ADCStubsViewModelOld : public StubsViewModel_t
{
	StubsArray_t	&mrArr;
public:
	ADCStubsViewModelOld(Main_t &rMain, StubsArray_t &rArr)
		: StubsViewModel_t(rMain), 
		mrArr(rArr)
	{
	}

protected:
	Stub_t *fromIndex(size_t i, FolderPtr *ppFolder)
	{
		return mrArr.fromIndex(i, ppFolder);
	}

	virtual int toIndex(FolderPtr p, ADDR a)
	{
		return toIndex(p, a);
	}
	virtual void invalidate(bool)
	{
		mrArr.redump(mrMain);
	}
	virtual int linesNum()
	{
		return (int)mrArr.size();
	}
	virtual int Redump(DUMPOS)
	{
		mrArr.redump(mrMain);
		return 1;
	}
};
#endif

class ADCStubsViewModel2 : public StubsViewModel_t
{
	const Dc_t	&mrDc;
	const StubMgr_t	&mrStubs;
public:
	ADCStubsViewModel2(CoreEx_t &rCore, const Dc_t &rDc)
		: StubsViewModel_t(rCore),
		mrDc(rDc),
		mrStubs(rDc.stubsFolderPtr()->fileStubs()->stubs())//rDc.stubs()
	{
	}
protected:
	virtual void invalidate(bool) override
	{
		//mvStubs.clear();
		//if (mrDC.hasStubs())
			//mvStubs.redump(mrDC.stubs());
	}
	virtual const Stub_t *fromIndex(size_t i, FolderPtr *ppFolder) const override
	{
		if (i < mrStubs.size())
		{
			if (ppFolder)
				*ppFolder = mrDc.moduleFolder();
			return mrStubs.at(i);
		}
		return nullptr;
	}
	virtual int toIndex(FolderPtr, ADDR a) override
	{
		return mrStubs.rank(a);
	}
	virtual int linesNum() override
	{
		return (int)mrStubs.size();
	}
	virtual int Redump(DUMPOS) override { return 0; }
	virtual bool reload() override
	{
		Locus_t aLoc;
		aLoc.add(mrDc.module(), 0, nullptr);
		ProbeEx_t* pCtx(new ProbeEx_t(aLoc));
		mrDc.project().main().postEvent(new adc::CEventCommand(pCtx, "load_stubs"));
		pCtx->Release();
		return true;// mrStubs.reload();
	}
};