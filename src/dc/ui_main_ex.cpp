#include "ui_main_ex.h"
#include "ui_expr_view.h"

#include "db/ui_bin_view.h"
#include "db/ui_res_view.h"
#include "ui_src_view.h"
#include "ui_stubs.h"
#include "ui_files_ex.h"
#include "ui_names_ex.h"
#include "ui_templ.h"


#include "anal_init.h"


//RefPtr_t<ProbeEx_t> SrcViewModel_t::mcxRef;

#if(!SBTREE_STUBS)
////////////////////////////////////////////////////////// StubsArray_t

int StubsArray_t::toIndex(Folder_t *pFolder, ADDR addr)
{
	int idx(-1);
	for (iterator i(begin()); i != end(); i++)
	{
		const StubsArrayElt &a(i->second);
		if (a.folder() == pFolder)
		{
			idx = a.toIndex(addr);
			if (idx != -1)
				idx += (int)i->first;
			break;
		}
	}
	return idx;
}

Stub_t *StubsArray_t::fromIndex(size_t idx, Folder_t **ppFolder) const
{
	const_iterator i(upper_bound(idx));
	if (i != cbegin())
	{
		i--;
		const StubsArrayElt &a(i->second);
		return a.fromIndex(idx - i->first, ppFolder);
	}
	return nullptr;
}

size_t StubsArray_t::size() const
{
	if (empty())
		return 0;
	const_reverse_iterator i(rbegin());
	const StubsArrayElt &a(i->second);
	return i->first + a.size();
}

bool StubsArray_t::empty() const
{
	return Base_t::empty();
}

size_t StubsArrayElt::redump(StubMgr_t &stubs)
{
	size_t n(stubs.size());
	assert(n > 0);
	
	resize(n);

	size_t i(0);
	for (StubIt it(stubs.begin()); it != stubs.end(); it++, i++)
		at(i) = it;

	return n;
}

void StubsArray_t::redump(Main_t &rMain)
{
	clear();
	if (!rMain.hasProject())
		return;
	ProjectEx_t &proj(reinterpret_cast<ProjectEx_t &>(rMain.project()));
	ProjectInfoEx_t PX(proj);
	size_t baseOffs(0);
	for (FileTree_t::ChildrenIterator it(proj.files().rootFolder()); it; it++)
	{
		Folder_t &rFolder(*it.data());
		//if (!PX.IsPhantomFolder(rFolder))
			//continue;
		Dc_t *pDC(PX.DcFromFolder(rFolder));
		if (!pDC || !pDC->hasStubs())
			continue;

		std::pair<Base_t::iterator, bool> ret(insert(std::make_pair(baseOffs, StubsArrayElt(&pDC->binaryFolder()))));
		assert(ret.second);
		StubsArrayElt &v(ret.first->second);

		baseOffs += v.redump(pDC->stubs());
	}
}

#endif




////////////////////////////////////////////////////////// BinViewModel2_t

class BinViewModel2_t : public BinViewModel_t
{
public:
	BinViewModel2_t(Core_t &rCore, Main_t &rMain, CTypeBasePtr iScope, bool bStrucDump)
		: BinViewModel_t(rCore, rMain, DcInfo_t::ModuleOfEx(iScope), iScope, bStrucDump)
		//mpFolder(module() ? module()->typeModule()->folderPtr() : nullptr)
	{
	}

protected:

	virtual void OnSetCurPos(const Probe_t &probe)
	{
		mrCore.setLocus(new ProbeEx_t(probe))->Release();

#if(0)
		MyString s("setlocus");
		//adc::CEventCommand *cmd(new adc::CEventCommand(s, false));
		//mrMain.postEvent(cmd);
		adc::CEventCommand cmd(s, false);
		cmd.m_autodelete = false;
		mrMain.callEvent(&cmd);
#endif
		return;

	}
	virtual void scopeName(MyStreamBase& ss)
	{
		CTypePtr pCont(realScope());
		if (!pCont)
			return;

		FolderPtr pFolder(DcInfo_s::FolderOf(pCont));
		if (!pFolder)
		{
			BinViewModel_t::scopeName(ss);
			return;
		}

		DcInfo_t DI(*DCREF(ProjectInfo_s::ModuleOf(pFolder)));

		FullName_t aName;
		
		if (pCont->objGlob())
			aName = DI.GlobNameFull(pCont->objGlob(), DcInfo_t::E_PRETTY, CHOP_SYMB);
		else
			aName = DI.TypeNameFull(pCont, DcInfo_t::E_PRETTY, CHOP_SYMB);
		/*proj().typeNameScoped(pCont->owner(), CHOP_SYMB, aName);
		if (!pCont->typeModule())
		{
			ProjectInfo_t PI(proj());
			//aName.append(proj().typeName(pCont));
			aName.append(PI.TypeName(pCont, CHOP_SYMB));
		}*/

		MyString s(moduleName());
		assert(!s.empty());
		s += MODULE_SEP;
		s += aName.join();

		MyStreamUtil ssh(ss);
		ssh.WriteString(s);
	}
};

BinViewModel_t *CoreEx_t::NewBinViewModel(CObjPtr pObj, bool bTypeEdit)
{
	if (pObj)
	{
		CTypeBasePtr pScope(pObj->objType());
		if (!pScope)
		{
			CFieldPtr pField(pObj->objField());
			if (!pField)
			{
				CGlobPtr pGlob(pObj->objGlob());
				if (pGlob)
				{
					if (pGlob->func())
						pScope = pGlob;
					else
						pField = DcInfo_s::DockField(pGlob);
				}
			}

			if (pField)
			{
				assert(!pScope);
				if (pField->isTypeImp())
				{
					pField = projx().toExportedField(pField);
					if (!pField)
						return nullptr;
				}
				pScope = DcInfo_t::CheckGlob(pField);
				if (!pScope || !pScope->hasPvt())
					pScope = pField->type();
			}
		}
		
		if (pScope)
		{
			if (!pScope->typeComplex())
				return nullptr;
			if (pScope->typeCode())
				return nullptr;
			//if (pScope->typeStrucvar())
				//return nullptr;
			resetBinaryDump(nullptr);//binary dump at index 1 is reserved for type editor
			return new BinViewModel2_t(*this, mrMain, pScope, bTypeEdit);
		}
	}
	return nullptr;
}


///////////////////////////////////////////////////////////////GuiEx_t

CoreEx_t::CoreEx_t(Mainx_t& rMain, ADCOutputSink &rGui, bool bEnableOutputCapture)
	: Core_t(rMain, rGui, bEnableOutputCapture)
{
}

CoreEx_t::~CoreEx_t()
{
}

ProbeEx_t *CoreEx_t::getContextEx()
{
	ProbeEx_t *pCtx2(nullptr);
	Probe_t *pCtx(getContext());
	if (pCtx)
	{
		pCtx2 = dynamic_cast<ProbeEx_t *>(pCtx);
		if (!pCtx2)
			releaseContext(pCtx);
	}
	return pCtx2;
}

Probe_t* CoreEx_t::setLocus(Probe_t *loc)
{
	//Obj_t *pObj(loc->obj());//can be deleted !

	ProbeEx_t &ctx((ProbeEx_t &)*loc);//new
	ProbeEx_t *pOld(getContextEx());//old

	//bool bDcChanged(ctx.dcRef() != pOld->dcRef());
	//bool bFolderChanged(pOld ? pOld->folder() != loc->folder() : true);
	bool bCurFolderChanged(pOld ? ctx.folder() != pOld->folder() : true);
	bool bCurFuncChanged(pOld ? ctx.scope() != pOld->scope() : true);
	bool bCurOpChanged(pOld ? (ctx.opLine() != pOld->opLine() || ctx.curVAs() != pOld->curVAs()) : true);
	bool bObjChanged(pOld ? ctx.obj() != pOld->obj() : true);


//		mcx.bUnfold = cx.bUnfold;
//		mcx.bIsFuncDecl = cx.bIsFuncDecl;
//		mcx.pFieldDecl = cx.pFieldDecl;
	
	Core_t::setLocus(loc);//aLoc, pObj);

	if (pOld)
		pOld->Release();
	//pOld may not be no longer valid

	if (bCurFolderChanged)
		project().markDirty(DIRTY_CUR_FOLDER);

	if (bCurFuncChanged)
		project().markDirty(DIRTY_CUR_FUNC);

	if (bCurOpChanged)
	{
		//mcx.mCurVAs.reset();
		if (ctx.opLine() && ctx.curVAs().empty() && ctx.fileDef())
		{
			Dc_t* pDC(ctx.dcRef());
			if (pDC)
			{
				FuncInfo_t fi(*pDC, *ctx.scopeFunc());
				fi.VAListFromOp(ctx.curVAs(), ctx.opLine());
			}
		}
		project().markDirty(DIRTY_CUR_OP);//gui().GuiOnCurOpChanged();
	}

	if (bObjChanged)
	{
		if (ctx.obj() && ctx.obj()->objField())
			project().markDirty(DIRTY_SEL_OBJ);//gui().GuiOnSelObjChanged();
	}
	return mctx.ptr();
}

adcui::IBinViewModel *CoreEx_t::NewBinViewModel(const char *fileName)
{
	if (mrMain.hasProject())
	{
		ReadLocker lock(mrMain);
		bool bHeader(false);
		Folder_t *pFolder(nullptr);
		if (fileName)
			pFolder = findFolder(fileName, bHeader);
		if (pFolder)
		{
			//ProjectInfoEx_t PJ(project());
			TypePtr iModule(ProjectInfo_t::ModuleOf(pFolder));
			if (iModule)
			{
				//FolderPtr pFolderTop(ProjectInfo_t::TopFolder(*pFolder));
				ModuleInfo_t MI2(project(), *iModule);
				MI2.FireDelayedFormat();
				//assert(iModule->typeModule()->folderPtr() == pFolderTop);
				BinViewModel2_t *pModel(new BinViewModel2_t(*this, mrMain, iModule, false));//, *pFolder));
				return pModel;
			}
		}
	}
	return nullptr;
}

adcui::IStubsViewModel *CoreEx_t::NewStubsViewModel()
{
#if(!SBTREE_STUBS)
	if (mrMain.hasProject())
	{
		ProjectInfoEx_t PJ(project());
		for (FileTree_t::ChildrenIterator i(projx().files().rootFolder()); i; i++)
		{
			const Folder_t &rFolder(*i.data());
			if (PJ.IsPhantomFolder(rFolder))
				continue;
			Dc_t *pDC(PJ.DcFromFolder(rFolder));
			if (!pDC)
				continue;
			ADCStubsViewModelOld *pIModel(new ADCStubsViewModelOld(mrMain, mStubsArray));
			return pIModel;
		}
	}
#endif
	return nullptr;
}

adcui::IStubsViewModel *CoreEx_t::NewStubsViewModel(const char *filePath)
{
	if (!filePath)
		return NewStubsViewModel();

	ReadLocker lock(mrMain);

	//ProjectInfo_t PI(project());

	adcui::IStubsViewModel *pIModel(nullptr);
	bool bHeader(false);
	CFolderPtr pFile(findFolder(filePath, bHeader));
	if (pFile)
	{
		TypePtr iModule(ProjectInfo_t::ModuleOf(pFile));
		Dc_t *pDC(DCREF(iModule));
		pIModel = new ADCStubsViewModel2(*this, *pDC);
	}
	return pIModel;
}

ObjPtr CoreEx_t::objAtLocus(const Dc_t* pDC)
{
	ObjPtr pObj(nullptr);
	ContextSafeEx_t safe(*this);
	if (safe && safe->dcRef() == pDC)
		pObj = safe->scopeFunc();
	if (!pObj)
		return nullptr;
	return pObj;
}


adcui::IADCExprViewModel *CoreEx_t::NewExprViewModel()
{
	return new ExprViewModel_t(*this);
}

adcui::ISrcViewModel *CoreEx_t::NewSrcViewModel(const char *filePath, int flags, const char* subj)
{
	ReadLocker lock(mrMain);

	SrcViewModel_t *pIModel(nullptr);
	bool bHeader(false);
	MyString subjFullName(subj);
	CGlobPtr pGlob(nullptr);

	CFolderPtr pFolder(nullptr);
	if (!filePath)
	{
		assert(hasProject());
		{//RAII-block
			ContextSafeEx_t safe(*this);
			if (!safe.empty())
			{
				FileDef_t *pFileDef(safe.get()->fileDef());
				if (pFileDef)
				{
					pFolder = pFileDef->ownerFolder();
					//safe.get()->cont
				}
			}
		}
		if (!pFolder)//check context folder first
			return nullptr;

		Dc_t* pDC0(DCREF(ProjectInfo_s::ModuleOf(pFolder)));
		if (pDC0)//in order to check the arch
		{
			if (!subjFullName.empty())
			{
				MyString moduleName(ZPath_t::moduleOf(subjFullName));
				if (moduleName.empty())
					return nullptr;

				DcInfo_t DI0(*pDC0);
				int iCase(DI0.IsArchCaseSensitive());
				CTypePtr pModule(DI0.FindModule(moduleName, iCase));
				if (!pModule)
					return nullptr;

				subjFullName.remove(0, unsigned(moduleName.length() + 1));

				DcInfo_t DI(*DCREF(pModule));
				ObjPtr pObj(DI.FindObjByScopedNameEx(subjFullName, DI.PrimeSeg()));
				if (!pObj)
					return nullptr;

				pGlob = DI.CheckGlob(pObj);
				if (!pGlob)
					return nullptr;

				pFolder = pGlob->folder();
			}
		}
	}
	else
	{
		pFolder = findFolder(filePath, bHeader);
	}

	if (pFolder && pFolder->fileDef())
	{
		CTypePtr iModule(ProjectInfo_t::ModuleOf(pFolder));
		flags |= adcui::DUMP_LNUMS;

		if (bHeader)
			flags |= adcui::DUMP_HEADER;

		//ProjectInfoEx_t PJ(project());
		const Dc_t *pDC(DCREF(iModule));
		assert(pDC);

		pIModel = new SrcViewModel_t(*this, *pDC, *pFolder, flags);
		if (!bHeader)
		{
			FileDef_t &rFileDef(*pFolder->fileDef());
			if (rFileDef.isOffloaded())
			{
				FileInfo_t FI(*pDC, rFileDef);
				FI.LoadFuncdefs();
			}
		}
	}


	if (pIModel)
	{
		if (pGlob)
			pIModel->setSubject(pGlob);
		else if (!subjFullName.isEmpty())
			pIModel->setSubject(subjFullName);
	}


	return pIModel;
}

#include "db/ui_types.h"

class ADCTypesViewModel4 : public TypesViewModel3_t
{
public:
	ADCTypesViewModel4(CoreEx_t &rCore, TypePtr iModule)
		: TypesViewModel3_t(rCore, iModule)
	{
	}
protected:
	virtual void data(size_t row, size_t column, MyStreamBase &ss) const
	{
		if (column == 0)
		{
			TypesViewModel3_t::data(row, column, ss);
			return;
		}
		
		if (!(row < m_data.size()))
			return;
		assert(mrCore.hasProject());
		TypePtr p(m_data.at(row));
		FolderPtr pFolder(USERFOLDER(p));
		if (!pFolder)
			return;
		MyStreamUtil ssu(ss);
		ssu.WriteString(mrCore.project().files().relPath(pFolder, miBinary->typeModule()->folderPtr()));
	}
};

adcui::ITypesViewModel *CoreEx_t::NewTypesViewModel(const char *pcFilePath)
{
	MyString filePath;
	if (pcFilePath)
		filePath = pcFilePath;
	if (filePath.empty())
		return Core_t::NewTypesViewModel(nullptr);

	ReadLocker lock(mrMain);
	bool bHeader(false);
	CFolderPtr pFolder(findFolder(filePath, bHeader));
	if (pFolder)
	{
		ProjectInfo_t PI(project());
		//return newTypesViewModel(PI.ModuleOf(pFolder));
		return new ADCTypesViewModel4(*this, PI.ModuleOf(pFolder));
	}
	return nullptr;
}

adcui::INamesViewModel *CoreEx_t::newNamesViewModel2(FolderPtr pNamesFolder)
{
	return new NamesViewModelEx_t(*this, pNamesFolder);
}

adcui::INamesViewModel *CoreEx_t::NewNamesViewModel(const char *pcFilePath)
{
	MyString filePath;
	if (pcFilePath)
		filePath = pcFilePath;
	if (filePath.empty())
		return Core_t::NewNamesViewModel(nullptr);

	ReadLocker lock(mrMain);
	bool bHeader(false);
	CFolderPtr pModuleFolder(findFolder(filePath, bHeader));
	if (pModuleFolder)
	{
		CTypePtr iModule(ProjectInfo_t::ModuleOf(pModuleFolder));
		ModuleInfo_t MI(project(), *iModule);
		FolderPtr pFolderNames(MI.FolderOfKind(FTYP_NAMES));
		if (pFolderNames)
		{
			if (DCREF(ProjectInfo_t::ModuleOf(pFolderNames)))
			{
				//return new NamesViewModel_t(*this, pFolderNames);
				return new NamesViewModelEx_t(*this, pFolderNames);
			}
			return newNamesViewModel(pFolderNames);
		}
	}
	return nullptr;
}

adcui::IExportsViewModel* CoreEx_t::newExportsViewModel(FolderPtr pFolder)
{
	return new ExportsViewModel_t(*this, pFolder);
}

adcui::ITemplViewModel *CoreEx_t::newTemplViewModel(FolderPtr pFolder)
{
	return new TemplViewModel_t(*this, pFolder);
}

/*adcui::ITemplViewModel *CoreEx_t::newTemplViewModel2(FolderPtr pFolder)
{
	return new ADCTemplViewModel2(*this, pFolder);
}*/

adcui::ITemplViewModel *CoreEx_t::NewTemplViewModel(const char *pcFilePath, bool bFields)
{
	MyString filePath;
	if (pcFilePath)
		filePath = pcFilePath;
	assert(!filePath.empty());

	ReadLocker lock(mrMain);
	bool bHeader(false);
	FolderPtr pModuleFolder(findFolder(filePath, bHeader));
	if (pModuleFolder)
	{
		TypePtr iModule(ProjectInfo_t::ModuleOf(pModuleFolder));
		ModuleInfo_t MI(project(), *iModule);
		FolderPtr pFolder(MI.FolderOfKind(FTYP_TEMPLATES));
		if (pFolder)
		{
//			if (bFields)
	//			return newTemplViewModel2(pFolder);
			return newTemplViewModel(pFolder);//types
		}
	}
	return nullptr;
}

adcui::IResViewModel *CoreEx_t::NewResViewModel(const char *filePath)
{
	if (!filePath)
		return Core_t::NewResViewModel(nullptr);
	ReadLocker lock(mrMain);
	ProjectInfo_t PI(project());
	adcui::IResViewModel *pIModel(nullptr);
	bool bHeader(false);
	CFolderPtr pFolder(findFolder(filePath, bHeader));
	if (pFolder)
	{
		pIModel = new ResViewModel_t(*this, PI.ModuleOf(pFolder));
		pIModel->reset();
	}
	return pIModel;
}


size_t CoreEx_t::openDisplay(CFolderPtr pFile, adcui::UDispFlags flags, OpType_t ptrSize)
{
	DisplayUI_t *pDisp(new DisplayUI_t(pFile, flags, true, ptrSize));//in draft mode
	return mDisplays.newSlot(pDisp);
}

void CoreEx_t::closeDisplay(size_t dispId)//, bool bDeleteHit)
{
	if (!dispId)
		return;
	Display_t *pDisp(mDisplays.deleteSlot(dispId));
	delete pDisp;
}

DisplayUI_t *CoreEx_t::displayAt(size_t dispId)
{
	if (!dispId)
		return nullptr;
	assert((size_t)dispId < mDisplays.size());
	return mDisplays.at(dispId);
}

void GuiEx_t::GuiOnDecompileFunction(MyString relPath, MyString func) const
{
	ADCPostData *pData(new ADCPostData());
	MyStream &ss(pData->obj());
	ss.WriteString(relPath);
	ss.WriteString(func);
	igui()->PostEvent(adcui::UIMSG_DECOMPILE_FUNCTION, pData);
}

bool GuiEx_t::GuiOnNoSourceContext(const char *cmd) const
{
	ADCPostData *pData(new ADCPostData());
	MyStream &ss(pData->obj());
	ss.WriteString(cmd);
	igui()->PostEvent(adcui::UIMSG_DECOMP_RECOIL, pData);
	return true;
}

int CoreEx_t::invalidateDisplays(FolderPtr pFile, TypeBasePtr iCont, RedumpFlags uType)
{
	int count(0);
	for (size_t i(0); i < mDisplays.size(); i++)
	{
		DisplayUI_t *pDisp(mDisplays[i]);
		if (pDisp)
		{
			if (!pFile || pDisp->file() == pFile)
			{
				RedumpFlags u(pDisp->IsHeader() ? REDUMP_H : REDUMP_SRC);
				if (u & uType)
					if (pDisp->InvalidateDump(iCont))
						count++;
			}
		}
	}
	return count;
}

bool CoreEx_t::popJumpBack(SrcJumpData_t& data)
{
	if (mJumpStackBack.empty())
		return false;
	data = mJumpStackBack.front();
	mJumpStackBack.pop_front();
	return true;
}

bool CoreEx_t::popJumpForward(SrcJumpData_t& data)
{
	if (mJumpStackForward.empty())
		return false;
	data = mJumpStackForward.front();
	mJumpStackForward.pop_front();
	return true;
}

/*void CoreEx_t::recoilJumpRequest()
{
	projx().guix().GuiNotify(adcui::UIMSG_JUMP_RECOIL);
}*/

void GuiEx_t::GuiInvalidateDisplays(FolderPtr pFile, TypeBasePtr iCont, RedumpFlags uType) const
{
	if (!core()->main().hasProject())
		return;

	//Project_t pProject(mrMain.project());

	MyString sExt(".*");
	if (uType == REDUMP_SRC)
		sExt = SOURCE_EXT;
	else if (uType == REDUMP_H)
		sExt = HEADER_EXT;

	//unsigned uType(3);//all kinds of file
	//if (sExt == HEADER_EXT)
		//uType &= ~2;//not a source
	//else if (sExt == SOURCE_EXT)
		//uType &= ~1;//not a header
	int count(corex()->invalidateDisplays(pFile, iCont, uType));
	if (count > 0)
	{
		MyString s("*");//all
		if (pFile)
			s = core()->project().files().relPath(pFile);
		GuiOnFileModified(s + sExt);
	}
	else
		GuiOnExprModified();//redraw may yet required
}

void GuiEx_t::GuiInvalidateStubs(Dc_t &) const
{
#if(!SBTREE_STUBS)
	if (mStubsArray.empty())
		return;
	mStubsArray.clear();
	GuiOnStubsModified();
#endif
}

int CoreEx_t::GetFileIdByName(const char *path)
{
	MyString sPath(path);
	if (!sPath.empty())
	{
		sPath = ProjectInfo_t::fixFileName(path, nullptr);
		//sPath.replace('\\', '/');

		if (sPath == " $")//special
		{
			assert(0);
		}
		ReadLocker lock(mrMain);
		if (mrMain.hasProject())
		{
			ProjectInfo_t PI(project());
			Folder_t* pFile(PI.FindFileByName(sPath));
			if (pFile)
				return pFile->ID();
		}
	}
	return -1;
}

FolderPtr CoreEx_t::findFolder(MyString sPath, bool &bHeader)
{
	//ProjectInfoEx_t PI(project());
	if (!hasProject())
		return nullptr;
	adcui::FolderTypeEnum iKind;
	CFolderPtr pFolder(projx().files().FindFileByPath(sPath, iKind));
	if (!pFolder)
		return nullptr;
	bHeader = (iKind == adcui::FOLDERTYPE_FILE_H);
	return (FolderPtr)pFolder;
}

bool CoreEx_t::GetFileNameById(int fileId, MyStreamBase &ss)
{
	assert(0);
	return false;
}

bool CoreEx_t::GetProtoInfo(MyStreamBase &ss)
{
	ReadLocker lock(mrMain);

	if (!hasProject())
		return false;

	Decompiler_t* pAnal(dynamic_cast<Decompiler_t*>(project().analyzer()));
	if (pAnal)
	{
		FuncInfo_t FI(pAnal->dc(), *pAnal->FuncDefPtr());
		return FI.WriteFuncProfileFromCall(pAnal->currentOp(), ss);// , pAnal);//from anal
	}

	//try locus
	ContextSafeEx_t safe(*this);
	if (safe.empty())
		return false;
	GlobPtr iScope(safe->scopeFunc());
	if (!iScope)
		return false;

	MyStreamUtil ssu(ss);

	FuncProfile_t fp;

	ADDR va(safe->locus().va());
	FolderPtr pFolder(safe->folder());
	MyString sFolder(project().files().relPath(pFolder) + SOURCE_EXT);
	FileDef_t& rFileDef(*pFolder->fileDef());

	ProtoInfo_t PRO(*safe->dcRef(), iScope);

	FieldPtr pField(nullptr);
	if (safe->locality().scoping == adcui::LocusId_FUNC_HEADER)
	{
		pField = DcInfo_s::DockField(iScope);
		assert(pField);
		PRO.GetFuncProfile(fp);
	}
	else
	{
		FuncInfo_t FI(PRO, *iScope);

		HOP hOp(safe->opLine());
		if (!hOp || !FuncInfo_t::IsCall(hOp))
			return false;

		if (!FuncInfo_t::IsCallIntrinsic(hOp))
		{
			PathOpTracer_t tr;
			//FuncInfo_t FI(PRO, *iScope);
			InitialTracer_t IT(FI, tr);
			pField = IT.GetCalleeFieldEx(hOp);
			if (pField && pField->isGlobal())
			{
				if (pField->isTypeImp())
				{
					FieldPtr pExpField(PRO.ToExportedField(pField));
					CGlobPtr iCallee(DcInfo_s::GlobObj(pExpField));
					Dc_t* pDC2(DcInfo_t::DcFromFolder(*iCallee->folder()));
					
					ProtoInfo_t FI2(*pDC2, iCallee);
					FI2.GetFuncProfile(fp);
				}
				else
				{
					CGlobPtr iCallee(DcInfo_s::GlobObj(pField));
					ProtoInfo_t TI3(PRO, iCallee);
					TI3.GetFuncProfile(fp);//GetArgProfileFromCall
				}
			}
			else
			{
				StubBase_t stb(nullptr, va);
				MyString sObject(stb.atAsString(FI.ImageBase()));

				FI.GetFuncProfileFromCall(fp, hOp);

				ssu.WriteString(sFolder);
				ssu.WriteStringf("Object=@%s", sObject.c_str());
				PRO.WriteFuncProfile(fp, ss);
				return true;
			}
		}
		else
		{
			GlobPtr iCallee(PRO.dc().getIntrinsic(hOp->m_disp));
			if (!iCallee)
				return false;
			ProtoInfo_t PRO2(PRO, iCallee);
			PRO2.GetFuncProfile(fp);
			ssu.WriteString(sFolder);
			ssu.WriteStringf("Object=%s", iCallee->name()->c_str());
			PRO.WriteFuncProfile(fp, ss);
			return true;
		}
	}

	StubBase_t stb(pField, va);
	MyString sObject(stb.atAsString(PRO.ImageBase()));

	ssu.WriteString(sFolder);
	ssu.WriteStringf("Object=@%s", sObject.c_str());
	PRO.WriteFuncProfile(fp, ss);
	return true;
}

void DcInfo_t::WriteFuncProfile(MyStreamBase &ss, CFolderPtr pFolder, const StubBase_t &stb)
{
	MyString sFolder(Project().files().relPath(pFolder) + SOURCE_EXT);

	MyStreamUtil ssu(ss);
	FuncProfile_t fp;

	StubInfo_t SI(*this);
		
	Stub_t* pStub(SI.FindStub(stb.atAddr()));
	if (pStub)
	{
		MyString s(SI.ValueFromStub(*pStub));
		ToFuncProfile(s, fp);
	}
	else
		fp.pstackPurgeRet = RetAddrSize();

	ssu.WriteString(sFolder);
	ssu.WriteStringf("Object=@%s", stb.atAsString(ImageBase()).c_str());
	WriteFuncProfile(fp, ss);
}

void DcInfo_t::WriteFuncProfile(const FuncProfile_t &fp, MyStreamBase &ss)
{
	MyStreamUtil ssu(ss);

	ssu.WriteStringf("PSalign=%d", mrDC.stackAddrSize());

	ssu.WriteStringf("PSin=%d", fp.stackin);
	ssu.WriteStringf("PStackPurge=%d", fp.pstackPurge);
	ssu.WriteStringf("PStackPurgeRet=%d", fp.pstackPurgeRet);
	ssu.WriteStringf("PScleanArgs=%d", (fp._flags & PPF_StackPurge) ? 1 : 0);
	ssu.WriteStringf("Variardic=%d", (fp._flags & PPF_Variardic) ? 1 : 0);
	ssu.WriteStringf("Usercall=%d", (fp._flags & PPF_Usercall) ? 1 : 0);
	ssu.WriteStringf("Thiscall=%d", fp.isThisCall() ? 1 : 0);

	ssu.WriteStringf("FPRin=%d", fp.fpuin);
	ssu.WriteStringf("FPRout=%d", fp.fpudiff);

	Arg1List_t lRegsIn;
	CpuRegs2ArgList(fp.cpuin, lRegsIn);
	ssu.WriteStringf("GPRin=%s", ArgListToString(lRegsIn, ",").c_str());//fp.cpuin.toString());

	ssu.WriteStringf("SpoiledRegs=%s", GPRsToString(SSID_CPUREG, fp.spoiltRegs, ",").c_str());//RegMaskToArgList(savedregs, ","));

	//Arg1List_t lFlagsSav;
	//FlagMaskToArgList(fp.spoiledFlags, lFlagsSav);
	//ssu.WriteStringf("FlagsSav=%s", ArgListToString(lFlagsSav, ",").c_str());

	Arg1List_t lRegsOut;
	CpuRegs2ArgList(fp.cpuout, lRegsOut);
	ssu.WriteStringf("GPRout=%s", ArgListToString(lRegsOut, ",").c_str());//fp.cpuout.toString());
}

struct proto_data_t
{
	MyString sObject;
	//MyString sEncoding;
	int	nPSalign;
	int	nPSin;
	int	nPStackPurge;
	int	nPStackPurgeRet;
	bool bPScleanArgs;
	bool bVariardic;
	bool bUsercall;
	bool bThiscall;
	int	nFPRin;
	int	nFPRout;
	MyString sGPRin;
	MyString sSpoiledRegs;
	MyString sFlagsSav;
	MyString sGPRout;
};

struct proto_data_parser_t : public proto_data_t
{
	MY_SCRIPT(proto_data_parser_t);
	MY_STRING(Object);
	//MY_STRING(Encoding);
	MY_INT(PSalign);

	MY_INT(PSin);
	MY_INT(PStackPurge);
	MY_INT(PStackPurgeRet);
	MY_BOOL(PScleanArgs);
	MY_BOOL(Variardic);
	MY_BOOL(Usercall);
	MY_BOOL(Thiscall);
	MY_INT(FPRin);
	MY_INT(FPRout);
	MY_STRING(GPRin);
	MY_STRING(SpoiledRegs);
	//MY_STRING(FlagsSav);
	MY_STRING(GPRout);
	proto_data_parser_t()
	{
		MY_IMPL(Object);
		//MY_IMPL(Encoding);
		MY_IMPL(PSalign);
		MY_IMPL(PSin);
		MY_IMPL(PStackPurge);
		MY_IMPL(PStackPurgeRet);
		MY_IMPL(PScleanArgs);
		MY_IMPL(Variardic);
		MY_IMPL(Usercall);
		MY_IMPL(Thiscall);
		MY_IMPL(FPRin);
		MY_IMPL(FPRout);
		MY_IMPL(GPRin);
		MY_IMPL(SpoiledRegs);
		//MY_IMPL(FlagsSav);
		MY_IMPL(GPRout);
	}
};

static void ToFuncProfile(const proto_data_t& aData, FuncProfile_t &fp, const StubInfo_t &DI)
{
	fp.stackin = aData.nPSin;
	fp.pstackPurge = aData.nPStackPurge;
	fp.pstackPurgeRet = aData.nPStackPurgeRet;
	if (aData.bPScleanArgs)
	{
		fp._flags |= PPF_StackPurge;
		fp.pstackPurge = fp.stackin;
	}
	if (aData.bVariardic)
		fp._flags |= PPF_Variardic;
	if (aData.bUsercall)
		fp._flags |= PPF_Usercall;

	fp.fpuin = aData.nFPRin * FTOP_STEP;
	fp.fpudiff = aData.nFPRout * FTOP_STEP;

	Arg1List_t lGPRin;
	DI.RegStringToArgList(aData.sGPRin, lGPRin);
	DI.ArgList2CpuRegs(lGPRin, aData.bThiscall, fp.cpuin);

	DI.RegStringToGPRs(aData.sSpoiledRegs, fp.spoiltRegs, OPC_CPUREG);

	Arg1List_t lFlagsSav;
	DI.RegStringToArgList(aData.sFlagsSav, lFlagsSav);
	FlagMaskType spoiledFlags;
	DI.ArgListToFlagsMask(lFlagsSav, spoiledFlags);
	//fp.spoiledFlags = spoiledFlags;

	Arg1List_t lGPRout;
	DI.RegStringToArgList(aData.sGPRout, lGPRout);
	DI.ArgList2CpuRegs(lGPRout, true, fp.cpuout);//bypass 'this' slot
}

bool CoreEx_t::SetProtoInfo(MyStreamBase &iss)//will trigger re-analysis!
{
	MyStreamUtil ssu(iss);

	ReadLocker lock(mrMain);

	ADDR va;
	FolderPtr pFolder;
	FieldPtr pField;
	CTypePtr iModule0;
	Locus_t aLoc;
	CTypeBasePtr pScope(nullptr);
	//bool bFromstub(true);
	//bool bPurge(false);
	bool bCallee(false);//change the callee's prototype, update the caller

	Decompiler_t *pAnal(dynamic_cast<Decompiler_t *>(project().analyzer()));
	if (!pAnal)
	{
		//try locus
		if (!hasProject())
			return false;

		ContextSafeEx_t safe(*this);
		if (safe.empty())
			return false;
		pScope = safe->scopeFunc();
		if (!pScope)
			return false;

		pFolder = safe->folder();
		if (!pFolder)
			return false;

		iModule0 = DcInfo_t::ModuleOf(pFolder);
		aLoc = safe->locus();

		DcInfo_t DI(*safe->dcRef());
		FuncInfo_t FI(DI, *pScope->objGlob());// , * pFolder->fileDef());
		
		if (safe->locality().scoping == adcui::LocusId_FUNC_HEADER)
		{
			pField = DcInfo_s::DockField(pScope->objGlob());
			assert(pField);
			//if (FI.IsStub())
				//bPurge = true;//prevent stub decompilation
		}
		else
		{
			HOP hOp(safe->opLine());
			if (!hOp || !FuncInfo_t::IsCall(hOp))
				return false;
			PathOpTracer_t tr;
			InitialTracer_t IT(FI, tr);
			pField = IT.GetCalleeFieldEx(hOp);
			bCallee = true;
			/*if (pField)
			{
				if (pField->isGlobal())
				{
					GlobPtr iCallee(DcInfo_t::GlobObj(pField));
					if (iCallee)//can be func or impptr
					{
						//if (iCallee->func())
						{
							bCallee = true;
							//if (iCallee->typeFuncDef()->isStub())
								//bPurge = true;
						}
						//else
							//pField = nullptr;
					}
					else
						pField = nullptr;
				}
				else
					pField = nullptr;
			}*/
			//if (!bCallee)
				//bFromstub = false;//just reanalyze the contents, the callsites will be adjusted appropriately
		}

		va = safe->locus().va();
	}
	else
	{
		pScope = pAnal->FuncDefPtr();
		pFolder = pAnal->currentFile();
		if (!pFolder)
			return false;
		pField = pAnal->currentOpField();
		//if (!pField)
			//return false;
		iModule0 = DcInfo_t::ModuleOf(pFolder);
		va = pAnal->currentVA();
CHECK(va == 0x54ACB0)
STOP
		pAnal->getLocus(aLoc);
		bCallee = true;
	}

	static proto_data_parser_t aData;
	aData.parse(ssu);

	////////////////////////////////////////////////

	StubBase_t stb(pField, va);

	DcInfo_t DI0(*DCREF(iModule0));//cur

	CTypePtr iModule(iModule0);
	//if (pField && pField->isTypeImp())
	{
		//bCallee = true;
		/*FieldPtr pExpField(DI0.ToExportedField(pField));
		iModule = ModuleInfo_t::ModuleOf(pExpField);
		//override with exported field
		stb = StubBase_t(pExpField, -1);*/
	}

	const Dc_t *pDC(DCREF(iModule));
	if (!pDC)
		return false;

	StubInfo_t SI(*pDC);

	FuncProfile_t fp;
	ToFuncProfile(aData, fp, SI);

	std::ostringstream ss;
	SI.dump_trimmed(fp, ss);//DI?

	MyString sValue(ss.str().c_str());

	ProbeEx_t *pCtx(new ProbeEx_t(aLoc));
	pCtx->setFolder(pFolder);
	pCtx->setScope(pScope);

	//update stub
	//DI0.PostMakeStub(stb.atAsString(), sValue, iModule, pCtx);//pAnal->makeContext());

	MyString sCmd(MyStringf("mkstub -apply @%s %s", stb.atAsString(DI0.ImageBase()).c_str(), sValue.c_str()));
	mrMain.postEvent(new adc::CEventCommand(pCtx, sCmd));

/*
	MyString sCmd("dc");
	//pCtx->AddRef();//reuse
	if (bFromstub)
		sCmd.append(" -fromstub");//force update from a stub!
	if (bPurge)
		sCmd.append(" -purge");
	if (bCallee)
		sCmd.append(MyStringf(" -@%X", stb.atAddr()));

	mrMain.postEvent(new adc::CEventCommand(pCtx, sCmd));*/

	if (bCallee)
	{
		//the above was for a callee, now update the caller
		// Warning: mkstub should not trigger the decompilation, otherwise this one will be skipped!
		mrMain.postEvent(new adc::CEventCommand(pCtx, "dc"));
	}

	pCtx->Release();
	return true;
}

TypePtr CoreEx_t::OnObjectPicked(ObjPtr pObj, TypePtr pModule)
{
	if (!pObj)
		return nullptr;
	FieldPtr pField(pObj->objField());
	if (pField && DcInfo_t::IsGlobal(pField))
	{
		if (pField->isTypeImp())
		{
			DcInfo_t DI(*DCREF(pModule));
			FieldPtr pExpField(DI.ToExportedField(pField));
			if (pExpField)
				pField = pExpField;
		}
		//if (pField->userDataType() == FUDT_ GLOBAL)
		{
			CGlobPtr pGlob(DcInfo_t::GlobObjNA(pField));
			if (pGlob && pGlob->func())
				return (GlobToTypePtr)pGlob;
		}
	}
	return Core_t::OnObjectPicked(pObj, pModule);
}

void CoreEx_t::DumpBlocks(const char *fullName, MyStreamBase &ss)
{
	ReadLocker lock(mrMain);

	CGlobPtr pGlob(nullptr);
	MyString sFullName(fullName);
	if (sFullName.empty())
	{
		if (hasProject())
		{
			ContextSafeEx_t safe(*this);
			if (!safe.empty())
			{
				FileDef_t* pFileDef(safe.get()->fileDef());
				if (pFileDef)
					pGlob = safe->scopeFunc();
				if (pGlob)
				{
					FileInfo_t FI(*safe.get()->dcRef(), *pFileDef);
					FI.DumpBlocks(pGlob, ss);
				}
			}
		}
	}
	else
	{
		ZPath_t zpath(fullName);
		if (!zpath.isAbsolute())
			return;
		ProjectInfo_t PI(project());
		CTypePtr iModule(PI.FindModuleEx(zpath.front(), true));
		if (!iModule)
			return;
		zpath.pop_front();
		if (zpath.empty())
			return;
		const Dc_t *pDC(DCREF(iModule));
		if (!pDC)
			return;

		DcInfo_t DI(*pDC);
		MyString sObjName(DI.extractVName(sFullName));
		if (sObjName.empty())
			return;
		ObjPtr pObj(DI.FindObjByScopedNameEx(sObjName, DI.PrimeSeg()));
		if (!pObj)
			pObj = objAtLocus(pDC);
		pGlob = DI.CheckGlob(pObj);
		if (!pGlob || !pGlob->func())
			return;
		FileInfo_t FI(DI, *pGlob->folder()->fileDef());
		FI.DumpBlocks(pGlob, ss);
	}
}

int CoreEx_t::checkSelection(CTypePtr iModule, const DA_t &da)
{
	ContextSafeEx_t safe(*this);
	if (safe)
	{
		if (!safe->curVAs().empty())
		{
			ModuleInfo_t PJ(project(), *iModule);
			Locus_t l;
			if (PJ.DaToLocus(da, l, iModule))
				return safe->curVAs().check(l.back().addr());
		}
	}
	return 0;
}

void CoreEx_t::DumpCutList(MyStreamBase& ss)
{
	ReadLocker lock(mrMain);
	MyStreamUtil ssu(ss);
	auto v(projx().cutList());
	for (auto pObj : v)
	{
		MyString s1, s2;
		CFieldPtr pField(pObj->objField());
		if (pField)
		{
			CTypePtr pModule(ProjectInfo_t::ModuleOf(pField));
			assert(pModule);
			//s1 = ProjectInfo_t::ModuleTitle(pModule);

			CGlobPtr pGlob(DcInfo_t::GlobObj(pField));
			assert(pGlob);

			CFolderPtr pFolder(DcInfo_t::FolderOf(pGlob));

			s1 = project().files().path(pFolder);// , miBinary->typeModule()->folderPtr())

			const Dc_t* pDC(DCREF(pModule));
			DcInfo_t DI(*pDC);
			s2 = DI.GlobNameFull(pGlob, DcInfo_t::E_PRETTY, CHOP_SYMB).join();
		}
		else
		{
			CTypePtr pType(pObj->objType());
			if (pType)
			{
			}
		}
		ssu.WriteString(s1.isEmpty() ? "?" : s1);
		ssu.WriteString(s2.isEmpty() ? "?" : s2);
	}
}

/*void CoreEx_t::DumpExpr(MyStreamBase &ss, adcui::IADCExprViewModel::Flags flags)
{
	ReadLocker lock(mrMain);
	if (hasProject())
	{
		ContextSafeEx_t safe(projx());
		if (safe)
		{
			FileDef_t *pFileDef(safe->fileDef());
			if (pFileDef)
			{
				FileInfo_t FI(*safe->dcRef(), *pFileDef);
				FI.DumpExpr(ss, flags);
				projx().releaseContext(pCtx);
			}
		}
	}
}*/

/*int CoreEx_t::GetPtrExprList(MyStreamBase &ss)
{
	ReadLocker lock(mrMain);
	int ret(0);
	if (hasProject())
	{
		ContextSafeEx_t safe(projx());
		if (!safe.empty())
		{
			FileDef_t *pFileDef(safe->fileDef());
			if (pFileDef)
			{
				FileInfo_t FI(*safe->dcRef(), *pFileDef);
				ret = FI.GetPtrExprList(ss);
			}
		}
	}
	return ret;
}*/

/*int CoreEx_t::DumpPtrExpr(int i, MyStreamBase &ss)
{
	ReadLocker lock(mrMain);
	//if (!dc())
	return 0;
	//return (dc()->ReadPtrDump(i, ss) != 0)?1:0;
}*/

/*long GuiEx_t::GetProjectFiles(MyStreamBase &ss)
{
	ReadLocker lock(mrMain);

	if (!hasProject())
		return 0;

	ProjectInfo_t PI(project());
	PI.DumpFiles(ss);

	return 1;
}*/

/*void GuiEx_t::RebuildStubsArray()
{
	InvalidateStubsArray();
	mv.resize(Base_t::size());
	size_t i(0);
	for (StubIt it(begin()); it != end(); it++, i++)
		mv[i] = it;
}*/

/*size_t GuiEx_t::StubsArraySize()
{
	if (mv.empty())
		RebuildStubsArray();
	return mv.size();
}*/

/*int GuiEx_t::StubsArrayToIndex(ADDR addr)
{
	if (mv.empty())
		RebuildStubsArray();
	return mv.toIndex(nullptr, addr); return 0;
	if (!mv.empty())
	{
		std::vector<StubIt>::iterator it(std::lower_bound(mv.begin(), mv.end(), addr, compi_t()));
		if (it != mv.end())
			if ((*it)->first == addr)
				i = (int)std::distance(mv.begin(), it);
	}
//	return i;
}*/

/*Stub_t *GuiEx_t::StubsArrayFromIndex(size_t idx)
{
	if (mv.empty())
		RebuildStubsArray();
	return mv.fromIndex(idx, nullptr);
}*/

adcui::IFilesViewModel *CoreEx_t::NewFilesViewModel()
{
	return new FilesViewModelEx_t(*this);
}

GuiEx_t *CreateADCGuiBroker(My::IGui *pIGui, Mainx_t &rMain, bool bAllowOutputCapture)
{
	GuiEx_t *pGui(new GuiEx_t(pIGui, rMain));
	pGui->setCore(new CoreEx_t(rMain, *pGui, bAllowOutputCapture));
	return pGui;
}

void CoreEx_t::WriteLocusPath(MyStreamBase &ss, bool bSkipSegs) const
{
	MyStreamUtil ssu(ss);
	Probe_t *pCtx(getContext());
	if (pCtx)
	{
		ProjectInfo_t PI(project());
		MyStreamUtil ssu(ss);
		for (auto i(pCtx->locus().begin()); i != pCtx->locus().end(); )
		{
			CTypePtr iType((*i).cont());
			++i;
			MyString s(PI.TypeName(iType));
			if (iType->typeModule())
				s += MODULE_SEP;
			else
			{
				if (iType->typeSeg() && bSkipSegs)
					continue;
				if (i != pCtx->locus().end())
					s.append(FOLDER_SEP);
			}
			ssu.WriteString(s);
		}
		releaseContext(pCtx);
	}
}

long CoreEx_t::GetLocusInfo(unsigned flags, MyStreamBase &ss)
{
	if (flags != 0)
	{
		ReadLocker lock(mrMain);
		if (!mrMain.hasProject())
			return 0;

#if(0)
		WriteLocusPath(ss, (flags & 2) != 0);//skip segments
#else
		{//RTTI-block
		ContextSafeEx_t safe(*this);
		if (safe && safe->scopeFunc())
		{
			const Dc_t *pDC(safe->dcRef());
			if (pDC)//?
			{
				DcInfo_t DI(*pDC);
				DI.WriteGlobInfo(safe->scopeFunc(), ss);
				return 1;
			}
		}
		}
#endif
		return 1;
	}
	return Core_t::GetLocusInfo(0, ss);
}

bool CoreEx_t::JumpSourceBack(const char *kwd)
{
	if (mJumpStackBack.empty())
		return false;
	const SrcJumpData_t &data(mJumpStackBack.front());
	assert(data.folder && data.folder->fileDef());
	MyString sFolder(FilesMgr0_t::relPath(data.folder));
	assert(!sFolder.empty());
	sFolder.append(data.isHeader ? HEADER_EXT : SOURCE_EXT);
	MyString sHint(kwd);
	if (data.linesFromTop >= 0)
		sHint.append(MyStringf("(%d)", data.linesFromTop));
	projx().guix().GuiShowFile(sFolder, sHint, false);
	return true;
}

bool CoreEx_t::JumpSourceForward(const char *kwd)
{
	if (mJumpStackForward.empty())
		return false;
	const SrcJumpData_t &data(mJumpStackForward.front());
	assert(data.folder && data.folder->fileDef());
	MyString sFolder(FilesMgr0_t::relPath(data.folder));
	assert(!sFolder.empty());
	sFolder.append(data.isHeader ? HEADER_EXT : SOURCE_EXT);
	MyString sHint(kwd);
	if (data.linesFromTop >= 0)
		sHint.append(MyStringf("(%d)", data.linesFromTop));
	projx().guix().GuiShowFile(sFolder, sHint, false);
	return true;
}


