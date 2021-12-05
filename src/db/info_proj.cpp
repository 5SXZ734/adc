#include "qx/MyStringList.h"
#include "debug.h"
#include "info_proj.h"
#include "type_seg.h"
#include "type_proc.h"
#include "type_proxy.h"
#include "type_code.h"
#include "clean.h"
#include "main.h"
#include "ui_main.h"
#include "anlzbin.h"
#include "dump_visit.h"
#include "dump_bin.h"

ProjectInfo_t::ProjectInfo_t(const ProjectInfo_t &r)
: mrMain(r.mrMain),
mrProject(r.mrProject),
mrMemMgr(r.mrMemMgr)
{
}

/*ProjectInfo_t::ProjectInfo_t(Project_t &rProjRef)
: mrMain(rProjRef.mrMain),
mrProject(rProjRef),
mrMemMgr(mrProject.memMgr())//,
{
}*/

ProjectInfo_t::ProjectInfo_t(const Project_t &rProjRef)
: mrMain(rProjRef.mrMain),
mrProject(const_cast<Project_t &>(rProjRef)),
mrMemMgr(mrProject.memMgr())//,
{
}

/*ProjectInfo_t::ProjectInfo_t(Project_t &rProjRef, MemoryMgr_t &rMemMgr)
: mrMain(rProjRef.mrMain),
mrProject(rProjRef),
mrMemMgr(rMemMgr)//,
{
}*/

ProjectInfo_t::ProjectInfo_t(const ProjectInfo_t &rPI, MemoryMgr_t &rMemMgr)
: mrMain(rPI.Project().mrMain),
mrProject(rPI.Project()),
mrMemMgr(rMemMgr)//,
//mrFiles(rProject.files())
{
}

MemoryMgr_t& ProjectInfo_t::memMgrGlob() const 
{
	return mrProject.memMgr();
}

bool ProjectInfo_t::IsMemMgrGlob() const
{
	return &memMgr() == &memMgrGlob();
}

/*MemoryMgr_t& ProjectInfo_t::memMgrGlobNS() const 
{
#if(STRUCVARS_LITE)
	return mrProject.memMgrNS();
#else
	return mrProject.memMgr();
#endif
}*/

MyString ProjectInfo_t::autoname(CFieldPtr pSelf, CFieldPtr pImpField, const char *pfx) const
{
	//pSelf = CloneLead(pSelf);
	CTypePtr pType(pSelf->type());
	if (!pType)
	{
		const FieldMap& m(pSelf->owner()->typeStruc()->fields());
		FieldMapCIt it(m.lower_bound(pSelf->_key()));
		CFieldPtr pField(VALUE(it));
		if (pField->isTypeProc() || pField->isTypeThunk() || pField->isTypeCode())
			pType = pField->type();
	}

	if (!pfx)
		pfx = mrProject.autoPrefix(pSelf, pType);

	if (pImpField)
		pSelf = pImpField;
	
	ADDR va(pSelf->_key());
	TypePtr iSeg(OwnerSeg(pSelf->owner()));
	return MyStringf("%s_%s", pfx, VA2STR(iSeg, va).c_str());
}


void ProjectInfo_t::RegisterTypesMap(TypePtr iOwner, bool bRegister)
{
	if (bRegister)
	{
		std::set<TypePtr> &m(mrProject.typeMaps());
		if (!m.insert(iOwner).second)
			ASSERT0;
	}
	else
	{
		std::set<TypePtr> &m(mrProject.typeMaps());
		if (m.erase(iOwner) != 1)
			ASSERT0;
	}
}

void ProjectInfo_t::DeleteNamespace(TypePtr p)
{
	Complex_t *pc(p->typeComplex());
	if (pc->namesMgr())
		pc->deleteNamespace(memMgr());
	p->flags() &= ~TYP_HAS_NMAP;
}

TypePtr ProjectInfo_t::CreateModule(int unique) const
{
	Module_t *pvt(new Module_t);
	pvt->setUnique(unique);
	return memMgr().NewTypeRef(pvt);
}

int ProjectInfo_t::NextModuleUnique() const
{
	int u(-1);//find out the unique
	const FoldersMap& m(mrProject.rootFolder().children());
	for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
	{
		CFolderRef rFolder(*i);
		if (rFolder.hasPvt())
		{
			int u2(rFolder.fileModule()->module()->typeModule()->unique());
			u = std::max(u, u2);
		}
	}
	return u + 1;//next
}

std::ostream& ProjectInfo_t::PrintError() const { return mrMain.printError(); }
std::ostream& ProjectInfo_t::PrintWarning() const { return mrMain.printWarning(); }
std::ostream& ProjectInfo_t::PrintInfo() const { return mrMain.printInfo(); }

FolderPtr ProjectInfo_t::CreateModuleFolder(MyString sModule) const
{
	TypePtr iModule;
	FolderPtr pFolder(AddFile(sModule + MODULE_SEP));
	if (!pFolder->hasPvt())
	{
		iModule = CreateModule(NextModuleUnique());
		pFolder->SetPvt(new FileModule_t(iModule));
		iModule->typeModule()->setFolderPtr(pFolder);
		iModule->typeModule()->setTitle(sModule);
		ModuleInfo_t MI(*this, *iModule);
		MI.AssureNamesFile();
		MI.AssureTypeMgr();
	}
	else
	{
		assert(pFolder->fileModule());
		iModule = pFolder->fileModule()->module();
	}
	return pFolder;
}

FolderPtr ProjectInfo_t::LoadBinary(const MyPath &fi0) const
{
	MyPath fi(fi0);

	DataSource_t *pDataSource(new DataSource_t);

	if (!pDataSource->load(fi.Path()))
	{
		if (!fi.Ext().empty())
		{
			delete pDataSource;
			return nullptr;
		}

		fi.SetExt("exe");
		if (!pDataSource->load(fi.Path()))
		{
#ifdef WIN32
			fi.SetExt("dll");
#else
			fi.SetExt("so");
#endif
			if (!pDataSource->load(fi.Path()))
			{
				delete pDataSource;
				return nullptr;
			}
		}
	}

	FolderPtr pFolder(CreateModuleFolder(fi.Name()));
	TypePtr iModule(pFolder->fileModule()->module());
	
	DataPtr pData(memMgr().NewDataObj());
	pData->setPvt(pDataSource);

	ModuleInfo_t MI(*this, *iModule);
	MI.SetDataSource(pData);
	//name always comes from the path
	//MI.SetModuleTitle(fi.Name());
	pData->setName(ModuleTitle(iModule));

	if (ModuleTitle(iModule).endsWith(".exe"))//is executable?
		if (!mrProject.mpStartUpBinary)
			mrProject.mpStartUpBinary = pFolder;

	MI.UpdateViewGeometry2();

	return pFolder;
}

bool ProjectInfo_t::SetStrucSize(TypePtr iSelf, unsigned fixedSize, bool bForce) const
{
//	if (iSelf->typeSeg())
	//	return iSelf->typeSeg()->setSize(iSelf, fixedSize);

	Struc_t &rSelf(*iSelf->typeStruc());
	if (rSelf.typeBitset())
		return false;
	FieldMap &m(rSelf.fields());
	ADDR uBase(rSelf.base(iSelf));
	FieldKeyType uKey(uBase + fixedSize);

	int iSize(0);
	if (!m.empty())
	{
		FieldMapRIt rit(m.rbegin());
		FieldPtr pField(VALUE(rit));
		FieldMapRIt ritEos(rit);
		FieldPtr pFieldEos(nullptr);
		if (IsEosField(pField))
		{
			pFieldEos = pField;
			++rit;
			pField = (rit != m.rend() ? VALUE(rit) : nullptr);
		}

		if (pField)
		{
			unsigned offs(pField->_key() - iSelf->base());
			if (fixedSize <= offs)
				return false;//can resize below the last field
		}

		if (pFieldEos)
		{
			int offsEos(pFieldEos->_key() - uBase);
			if (fixedSize == offsEos)
				return false;//already

			//relocate eos field
			m.take(ritEos);
			pFieldEos->overrideKey(uKey);
			m.insert_unique(pFieldEos);
			return true;
		}

		if (!bForce && pField)
		{
			int fsz(pField->size());
			if (fsz == 0)
				fsz = OPSZ_BYTE;
			if (fsz > 0 && pField->_key() + fsz == fixedSize)
				return false;//no need to insert eos field
		}
		/*if (iFieldSize < 0)//code?
			iFieldSize = 0;
		int sizeMax(offs + iFieldSize);
		if (sz > sizeMax)
			mSize = sz;
		else
			mSize = -1;//determined by the last field
			*/
	}

	//create a new eos field
	if (!(uKey > 0))
		return false;

	FieldPtr pField(memMgr().NewField(uKey));
	if (!m.insert_unique(pField).second)
		ASSERT0;
	pField->setOwnerComplex(iSelf);
	SetEosField(pField);
	return true;
}

void ProjectInfo_t::SendProgress(const ProgressMonitor_t<size_t> &PM) const
{
	if (PM.percent() <= 100)
		gui().GuiOnShowProgressInfo(MyStringf("%s: <font color=darkBlue>%d%%</font>", PM.desc(), PM.percent()));
	else
		gui().GuiOnShowProgressInfo("");//hide
}

void ProjectInfo_t::SendProgress2(const ProgressMonitor_t<ROWID> &PM) const
{
	if (PM.percent() <= 100)
		gui().GuiOnShowProgressInfo(MyStringf("Listing: %d%%", PM.percent()));
}

const IGui_t &ProjectInfo_t::gui() const { return mrProject.gui(); }

int ProjectInfo_t::PrintListing(TypePtr iScope, unsigned uColFlags, bool bCompact)
{
	TypePtr iModule(ModuleOf(iScope));
	COUTbin_t cacheObj(iScope/*, iScope->typeSeg()->viewOffs(), ViewSize(iScope)*/, HexWidthMax(ViewSize(iScope)));
	if (iScope != iModule)
		cacheObj.scopeTo(iScope->typeSeg()->viewOffs(), ProjectInfo_t::ViewSize(iScope));
	//cacheObj.init();
/*?	if (!pScope)
		pScope = this;
	cacheObj.setScope(pScope);*/
	cacheObj.showColumn(adcui::IBinViewModel::CLMN_DA, true);
	cacheObj.showColumn(adcui::IBinViewModel::CLMN_FILE, true);
	cacheObj.showColumn(adcui::IBinViewModel::CLMN_BYTES, true);
	if (uColFlags != 0)//turn off unwanted columns
	{
		for (size_t i(0); i < cacheObj.colsNum(); i++)
		{
			int colw(cacheObj.columnWidth((COLID)i));
			if (!(uColFlags & (1 << i)))
				cacheObj.setColumnWidth((COLID)i, -colw);
		}
	}
	if (bCompact)
		cacheObj.setCompactMode(true);
	cacheObj.setFullMode(true);
	adcui::DUMPOS iPos0(cacheObj.NewPosition());
	ROWID iRow(0);//starting row
//	if (pField)
		//startRow = pField->offset();

	DumpVisitor_t bin(mrProject, cacheObj);

	ProgressMonitor_t<ROWID> PM(cacheObj.scopeLowerBound(), cacheObj.scopeUppperBound());

#if(1)//forward
	if (bin.SeekIt(iPos0, DA_t(iRow, 0, 0), nullptr))
	{
		adcui::ITER drawIt(bin.NewIterator1(iPos0));
		adcui::DUMPOS iPos(cacheObj.PosFromIter(drawIt));
		do {
			cacheObj.outputRow(iPos, std::cout, bin);//bad
			if (PM.update(cacheObj.RowIdIt(iPos)))
				SendProgress2(PM);
			iRow++;
		} while (bin.ForwardIt(drawIt));
	}
#else//backward
	cacheObj.SeekIt(drawIt, -1, 0);
	while (drawIt.BackwardIt(drawIt));
#endif

	//COUTbin_t coutObj(cacheObj);
	//coutObj.output(os);
	SendProgress2(PM);//force 100% to be shown

	return 1;
}


FolderPtr ProjectInfo_t::ModuleFromDA(ROWID da)
{
	const FoldersMap &m(mrProject.rootFolder().children());
	for (FoldersMap::const_iterator it(m.begin()); it != m.end(); it++)
	{
		CFolderRef rFolder(*it);
		if (!IsPhantomFolder(rFolder))
		{
			TypePtr iModule(ModuleOf(&rFolder));
			const Module_t &aBin(*iModule->typeModule());
			if (aBin.viewOffs() <= da && da < aBin.viewOffs() + ViewSize(iModule))
				return (FolderPtr)&rFolder;
		}
	}
	return nullptr;
}

Folder_t *ProjectInfo_t::ModuleFromPath(MyString sBin) const
{
	int n(sBin.find(':'));
	if (n > 0)
		sBin.truncate(n);
	const FoldersMap &m(mrProject.rootFolder().children());
	for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
	{
		CFolderRef rFolder(*i);
		//if (!IsPhantomFolder(*pFolder))
		{
			TypePtr iModule(ModuleOf(&rFolder));
			//const Module_t &aBin(*iModule->typeModule());
			if (ModuleTitle(iModule) == sBin)
				return (FolderPtr)&rFolder;
		}
	}
	return nullptr;
}

FolderPtr ProjectInfo_t::FindModuleFolder(MyString s, bool bCaseSensitive, CFolderPtr from) const
{
//#ifdef WIN32
//	s = s.lower();
//#endif
	if (!s.endsWith(MODULE_SEP))
		s.append(MODULE_SEP);
	const FoldersMap& m(mrProject.rootFolder().children());
	if (bCaseSensitive)
	{
		FoldersMap::const_iterator i(m.find(s));
		if (i != m.end())
		{
			CFolderPtr pFolder(&(*i));
			return (FolderPtr)pFolder;
		}
	}
	FoldersMap::const_iterator i(m.begin());
	if (from)
	{
		i = FoldersMap::const_iterator(from);
		++i;
	}
	for (; i != m.end(); i++)
	{
		CFolderPtr pFolder(&(*i));
		if (icasecmp(pFolder->name(), s))
			return (FolderPtr)pFolder;
	}
	return nullptr;
}

TypePtr ProjectInfo_t::FindModule(const MyString& s, bool bCaseSensitive, CFolderPtr from) const
{
	CFolderPtr pFolder(FindModuleFolder(s, bCaseSensitive, from));
	if (pFolder)
		return ModuleOf(pFolder);
	return nullptr;
}

//from user's input
FolderPtr ProjectInfo_t::FindModuleFolderEx(const MyString& s, bool bAllowMiscase) const
{
	FolderPtr pResult(nullptr);
	FolderPtr pCandidate(nullptr);
	for (;;)
	{
		pResult = FindModuleFolder(s, false, pResult);//case 
		if (!pResult)
			break;
		TypePtr pModule(pResult->fileModule()->module());
		if (!pModule)
			break;
		ModuleInfo_t MI(*this, *pModule);
		int iCase(MI.IsArchCaseSensitive());
		if (iCase != 1)
			break;//have guessed right
		if (s == MI.ModuleName())
			break;//exact match
		if (!pCandidate)
			pCandidate = pResult;
		//continue search from this point
	}
	if (!pResult && pCandidate && bAllowMiscase)
		pResult = pCandidate;
	return pResult;
}

TypePtr ProjectInfo_t::FindModuleEx(const MyString& s, bool bAllowMiscase) const
{
	FolderPtr pFolder(FindModuleFolderEx(s, bAllowMiscase));
	if (pFolder)
		return ModuleOf(pFolder);
	return nullptr;
}


FolderPtr ProjectInfo_t::FindModuleFolderByTag(int tag) const
{
	FoldersMap &m(mrProject.rootFolder().children());
	for (FoldersMap::iterator i(m.begin()); i != m.end(); ++i)
	{
		FolderRef rFolder(*i);
		if (ModuleOf(&rFolder)->typeModule()->moduleTag() == tag)
			return &rFolder;
	}
	return nullptr;
}

FolderPtr ProjectInfo_t::FindModuleFolderByUnique(int un) const
{
	FoldersMap &m(mrProject.rootFolder().children());
	for (FoldersMap::iterator i(m.begin()); i != m.end(); ++i)
	{
		FolderRef rFolder(*i);
		if (ModuleOf(&rFolder)->typeModule()->unique() == un)
			return &rFolder;
	}
	return nullptr;
}

Folder_t *ProjectInfo_t::FolderOfModule(TypePtr iModule) const
{
	assert(iModule);
	return iModule->typeModule()->folderPtr();
/*	const FoldersMap &m(mrProject.rootFolder().children());
	for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
	{
		FolderPtr pFolder(i.pvt());
		if (pFolder->miBinary == iModule)
			return pFolder;
	}
	return nullptr;*/
}

FolderPtr ProjectInfo_t::LocusFromDA(ROWID row, Locus_t &l)//go downward
{
	if (row == 0)
		return nullptr;
	
	Folder_t *pFolder(ModuleFromDA(row));//?
	if (!pFolder)
		return nullptr;

	TypePtr iModule(ModuleOf(pFolder));

	l.clear();

	DA_t da2(row, 0, 0);
	terminalFieldAtSeg(iModule, da2, l, iModule->typeModule()->rawBlock());
	if (l.empty())
		return nullptr;
	//l.setFolder(pFolder);
	return pFolder;
}

bool ProjectInfo_t::LocusFromVA(CTypePtr iSeg, ADDR va, Locus_t &l, bool bStripToSeg) const//go upward
{
	Seg_t *pSeg(iSeg->typeSeg());
	assert(pSeg && pSeg->subsegs().empty());

	DA_t da(pSeg->viewOffsAt(iSeg, va), 0, 0);
	terminalFieldAtSeg(iSeg, da, l, pSeg->rawBlock());
	if (l.empty())
		return false;
	if (bStripToSeg)
	{
		unsigned offs;
		l.stripToSeg(offs);
	}
	return !l.empty();
}

bool ProjectInfo_t::LocusFromVA_1(CTypePtr iSeg, ADDR va, Locus_t &l) const//go upward
{
	FieldPtr pField(Field(iSeg, va, nullptr, FieldIt_Exact));//exact
	while (iSeg)
	{
		Seg_t &rSeg(*iSeg->typeSeg());
		l.push_front(Frame_t(rSeg.rawBlock(), (TypePtr)iSeg, va, pField));
		if (!rSeg.superLink())
		{
			assert(iSeg->typeModule());//shuld be binary
			break;
		}
		if (rSeg.traceLink())
		{
			pField = rSeg.traceLink()->parentField();
			va = pField->_key();
		}
		else
		{
			pField = nullptr;
			va = rSeg.base(iSeg);
		}
		iSeg = rSeg.superLink();
	}
	return !l.empty();
}

bool ProjectInfo_t::LocusFromVA_2(CTypePtr iCont, ADDR va, Locus_t &aLoc) const//does not change a folder
{
	assert(aLoc.empty());

	FieldPtr pField(nullptr);
	while (iCont)
	{
		aLoc.push_front(Frame_t(Block_t(), (TypePtr)iCont, va, pField));

		const Seg_t *pSeg(iCont->typeSeg());
		if (pSeg && pSeg->superLink())
		{
			va = (ADDR)rawOffs(iCont);
			iCont = pSeg->superLink();
			pField = iCont->parentField();
			//va = pField ? pField->_key() : 0;
		}
		else
		{
			pField = iCont->parentField();
			if (!pField)
			{
				if (!pSeg)
					break;
				va = (ADDR)rawOffs(iCont);
				iCont = pSeg->superLink();
			}
			else
			{
				iCont = pField->OwnerComplex();
				va = pField->_key();
			}
		}
	}

	return true;
}

bool ProjectInfo_t::LocusFromVA(CFieldPtr pField, Locus_t &aLoc) const//does not change a folder
{
	assert(aLoc.empty() && pField);
	ADDR va(pField->_key());

	CTypePtr iCont(pField->owner());
	while (iCont)
	{
		aLoc.push_front(Frame_t(Block_t(), (TypePtr)iCont, va, (FieldPtr)pField));

		const Seg_t *pSeg(iCont->typeSeg());
		if (pSeg && pSeg->superLink())
		{
			va = (ADDR)rawOffs(iCont);
			iCont = pSeg->superLink();
			pField = iCont->parentField();
			//va = pField ? pField->_key() : 0;
		}
		else
		{
			pField = iCont->parentField();
			if (!pField)
			{
				if (!pSeg)
					break;
				va = (ADDR)rawOffs(iCont);
				iCont = pSeg->superLink();
			}
			else
			{
				iCont = pField->OwnerComplex();
				va = pField->_key();
			}
		}
	}

	return true;
}

BinaryAnalyzer_t *ProjectInfo_t::StartBinaryAnalizer(TypePtr iModule, unsigned affinity) const
{
	if (!mrProject.binAnalizer())
	{
		mrProject.pushAnalyzer(new BinaryAnalyzer_t(mrProject, iModule, affinity));
//?		gui().GuiOnAnalizerStarted();
	}
	return mrProject.binAnalizer();
}

BinaryAnalyzer_t* ProjectInfo_t::FindBinaryAnalizer() const
{
	return mrProject.findBinAnalizer();
}

MyString ProjectInfo_t::AutoName(CFieldPtr pSelf, CFieldPtr pImpField) const
{
	MyString s;

	//Project_t &rProj(Project());

//	if (rProj.Autoname(pSelf, s))
	//	return s;

	//?	if (IsDataOp())
	//?		return GetOwnerData()->nam ex(buf);

	ADDR offs(pSelf->_key());
	if (pSelf->isGlobal())
	{
		MyString s(autoname(pSelf, pImpField));
		if (s.empty())
		{
			if (offs != 0)
			{
				if (IsProc(pSelf->type()))
				{
					s = autoname(pSelf, pImpField);
				}
				else
				{
					assert(0);
/*?					int t = 0;
					if (pSelf->type())
						t = pSelf->type()->optype();*/
					//?s = mrDC.regName(SSID_GLOBAL, offs, t, 0);
					
				}
			}
			else
			{
				assert(0);//?		s = MyStringf("glob_%d@%d", nIndex, ID());
			}
		}

		assert(!s.empty());
		TypePtr iSeg(pSelf->owner());
		if (iSeg->typeSeg())
		{
			const FieldMap& m(iSeg->typeStruc()->fields());
			FieldMapCIt it(m.lower_bound(pSelf->_key()));
			if (VALUE(it) != pSelf)
			{
				for (int n(0); ; n++, it++)
				{
					if (VALUE(it) == pSelf)
					{
						std::ostringstream ss;
						ss << DUB_SEPARATOR << n + 1 << DUB_TERMINATOR;
						s.append(ss.str());
						//s.append(MyStringf("#%d", n + 1));
						break;
					}
					assert(it != m.end());
				}
			}
		}
		return s;
	}

	if (pSelf->owner()->typeBitset())
		offs = pSelf->owner()->parentField()->_key();
	s = MyStringf("m%s", Int2Str(offs, /*I2S_SIGNED |*/ I2S_HEXA).c_str());
	if (*s.rbegin() != 'h')
		s += 'h';
	if (pSelf->owner()->typeBitset())
		s.append(MyStringf("%db", pSelf->_key()));
	return s;
}

TypePtr ProjectInfo_t::FindFrontSegById(TypePtr iModule, int id)
{
	FRONT_t *pFRONT(FrontEnd(iModule));
	if (pFRONT && pFRONT->id() == id)
		return iModule;//later for segs
	return nullptr;
}

FRONT_t *ProjectInfo_t::FrontAt(size_t i) const
{
	return mrProject.frontFromIndex(i);
}

FRONT_t *ProjectInfo_t::FrontEnd(CTypePtr iSeg) const
{
	return FrontAt(iSeg->typeSeg()->frontIndex());
}

void ProjectInfo_t::ReleaseFrontEnd(size_t i) const
{
	mrProject.releaseFront(i);
}

unsigned ProjectInfo_t::NewFrontEnd(const char *name, int id)
{
	return (unsigned)mrProject.newFrontSlot(name, id);
}

/*void ProjectInfo_t::DumpFiles(const Folder_t &rFolder0, MyStreamUtil &ssu) const//from binary
{
	assert(rFolder0.fileFolder());
	FileTree_t::Iterator i((Folder_t *)&rFolder0);
	i++;//skip the root
	for (; i; i++)
	{
		const Folder_t &rFolder(*i.top());
		int level(i.level());
		mrProject.dumpFile(rFolder, level, ssu);
	}
	//mpRootFolder->dump(ss, level);
}

void ProjectInfo_t::DumpFiles(MyStreamBase &ss) const
{
	MyStreamUtil ssu(ss);
	//go through all the binaries
	//non-phantoms first
	const FoldersMap &m(mrProject.rootFolder().children());
	for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
	{
		FolderPtr pFolder(i.pvt());
		if (!IsPhantomFolder(*pFolder))
		{
			mrProject.dumpFile(*pFolder, 0, ssu);
			DumpFiles(*pFolder, ssu);
		}
	}
	//phantoms second
	for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
	{
		FolderPtr pFolder(i.pvt());
		if (IsPhantomFolder(*pFolder))
		{
			mrProject.dumpFile(*pFolder, 0, ssu);
			DumpFiles(*pFolder, ssu);
		}
	}
}*/

bool ProjectInfo_t::OfTheSameModule(CFolderPtr a, CFolderPtr b) const
{
	return TopFolder(*a) == TopFolder(*b);
}

MyString ProjectInfo_t::StrucNameless(CTypeBasePtr pSelf) const
{
	Typedef_t *pTypedef(pSelf->typeTypedef());
	if (pTypedef)
	{
		int id(pTypedef->ID());
		return MyStringf("%s_%d", pTypedef->printType(), id);
	}
	Struc_t *pStruc(pSelf->typeStruc());
	if (!pStruc)
		return "?";
	int id(pStruc->ID());
	if (mrMain.options().bNoAutoIds)
		id = 0;
	if (pStruc->typeFuncDef())
		return MyStringf("%s_%d", pStruc->printType(), id);
	if (pSelf->isVTable())
	{
		if (pSelf->isShared())
			return MyStringf("%s_%d", pStruc->printType(), id);
		return MyStringf("`%s'", pStruc->printType());
	}
	/*if (pStruc->typeRTTI_TD())
		return MyStringf("`%s'", pStruc->printType());
	if (pStruc->typeRTTI_BCD())
		return MyStringf("`%s'", pStruc->printType());
	if (pStruc->typeRTTI_CHD())
		return MyStringf("`%s'", pStruc->printType());
	if (pStruc->typeRTTI_BCA())
		return MyStringf("`%s'", pStruc->printType());
	if (pStruc->typeRTTI_COL())
		return MyStringf("`%s'", pStruc->printType());*/

	const char *pfx(pStruc->printType());
	assert(!pStruc->typeClass() || !pSelf->isEnum());
	if (pSelf->isEnum())
		pfx = "enum";
	else if (pSelf->typeUnion())
		pfx = "union";

#ifdef _DEBUG
	if (pStruc->ID() < 0)
		return MyStringf("%s__%d", pfx, -id);
#endif
	return MyStringf("%s_%d", pfx, id);
}

TypePtr ProjectInfo_t::SetType(Array_t &rSelf, TypePtr iTypeBase, bool bNoRef) const
{
	assert(iTypeBase && !rSelf.baseType());
	assert(!iTypeBase || !iTypeBase->typeFuncDef());
	if (iTypeBase != rSelf.baseType())
	{
		assert(!rSelf.baseType());
		//Release TypeRef(rSelf.baseType());
		rSelf.setBaseType(iTypeBase);
		if (!bNoRef)
			AcquireTypeRef(rSelf.baseType());
	}
	return rSelf.baseType();
}

TypePtr ProjectInfo_t::SetType(TypeEnum_t &rSelf, TypePtr iEnumRef, bool bNoRef) const
{
	assert(iEnumRef && !rSelf.enumRef());
	if (iEnumRef != rSelf.enumRef())
	{
		assert(!rSelf.enumRef());
		rSelf.setEnumRef(iEnumRef);
		if (!bNoRef)
			AcquireTypeRef(rSelf.enumRef());
	}
	return rSelf.enumRef();
}

TypePtr ProjectInfo_t::SetType(TypeArrayIndex_t &rSelf, TypePtr iArrayRef, TypePtr iIndexRef, bool bNoRef) const
{
	assert(iArrayRef && !rSelf.arrayRef());
	if (iArrayRef != rSelf.arrayRef())
	{
		assert(!rSelf.arrayRef());
		rSelf.setArrayRef(iArrayRef);
		if (!bNoRef)
			AcquireTypeRef(rSelf.arrayRef());
	}
	assert(iIndexRef && !rSelf.indexRef());
	if (iIndexRef != rSelf.indexRef())
	{
		assert(!rSelf.indexRef());
		rSelf.setIndexRef(iIndexRef);
		if (!bNoRef)
			AcquireTypeRef(rSelf.indexRef());
	}
	return rSelf.arrayRef();
}

TypePtr ProjectInfo_t::SetType(TypeConst_t &rSelf, TypePtr iTypeBase, bool bNoRef) const
{
	assert(iTypeBase && !rSelf.baseType());
	assert(!iTypeBase->typeFuncDef());
	if (iTypeBase != rSelf.baseType())
	{
		assert(!rSelf.baseType());
		rSelf.setBaseType(iTypeBase);
		if (!bNoRef)
			AcquireTypeRef(rSelf.baseType());
	}
	return rSelf.baseType();
}

TypePtr ProjectInfo_t::SetType(Typedef_t &rSelf, TypePtr pType) const
{
	//assert(pType && !rSelf.mpBaseType);//can be typedef of void
	if (pType && pType == rSelf.mpBaseType)
		return rSelf.mpBaseType;
	assert(!pType || !pType->typeFuncDef());
	assert(!rSelf.mpBaseType);
	if (pType)
	{
		rSelf.mpBaseType = pType;
		AcquireTypeRef(rSelf.mpBaseType);
	}
	return rSelf.mpBaseType;
}

TypePtr ProjectInfo_t::SetType(TypePair_t &rSelf, TypePtr left, TypePtr right, bool bNoRef) const
{
	assert(!rSelf.left() && !rSelf.right());
	if (left)
	{
		rSelf.setLeft(left);
		if (!bNoRef)
			AcquireTypeRef(rSelf.left());
	}
	if (right)
	{
		rSelf.setRight(right);
		if (!bNoRef)
			AcquireTypeRef(rSelf.right());
	}
	return nullptr;
}

TypePtr ProjectInfo_t::SetType(TypeFunc_t &rSelf, TypePtr ret, TypePtr args, unsigned flags, bool bNoRef) const
{
	assert(!rSelf.retVal() && !rSelf.args());
	if (ret)
	{
		rSelf.setRetVal(ret);
		if (!bNoRef)
			AcquireTypeRef(rSelf.retVal());
	}
	if (args)
	{
		rSelf.setArgs(args);
		if (!bNoRef)
			AcquireTypeRef(rSelf.args());
	}
	rSelf.setFlags(flags);
	return nullptr;
}


/*TypePtr ProjectInfo_t::setT ype(VarArray_t &rSelf, TypePtr iTypeBase) const
{
	if (!iTypeBase || iTypeBase != rSelf.baseType())
	{
		Release TypeRef(rSelf.baseType());
		rSelf.setBaseType0(iTypeBase);
		AcquireTypeRef(rSelf.baseType());
	}
	return rSelf.baseType();
}*/


NamesMgr_t &ProjectInfo_t::OwnerNamespaceEx(TypePtr iClass, TypePtr *ppOwner) const
{
	TypePtr pOwner;
	OwnerNamesMgr(iClass, &pOwner);
	//NamesMgr_t &rNS1(NamespaceInitialized(pOwner));
	//return rNS1;
	return *pOwner->typeComplex()->namesMgr();
}

MyString ProjectInfo_t::ObjName(CObjPtr pObj) const
{
	CFieldPtr pField(pObj->objField());
	if (pField)
		return FieldName(pField);
	TypePtr iType(pObj->objTypeGlob());
	assert(iType);
	return TypeName(iType);
}

bool ProjectInfo_t::TmpLabelName(CFieldPtr pSelf, MyString &s) const
{
	if (!pSelf->owner())//temp fields are expected to be named
	{
		if (pSelf->m_nFlags & FLD_TEMP)
			if (pSelf->type())
			{
				s = TypeName(pSelf->type());
				return true;
			}
	}
	return false;
}

bool ProjectInfo_t::LabelName(CFieldPtr pSelf, MyString &s) const
{
	if (!TmpLabelName(pSelf, s))//name is taken from type (funcend)
	{
		if (!pSelf->isTypeCode())
			return false;

		ADDR offs(pSelf->_key());
		//uint32_t offs = pSelf->offset();
		if (offs != BAD_ADDR && offs != 0)
		{
			TypePtr iSeg(OwnerSeg(pSelf->owner()));
			s = MyString("loc_") + VA2STR(iSeg, offs);
		}
		else
			s = MyString("$loc_?");
	}
	return true;
}

bool ProjectInfo_t::SetTypeEx(FieldPtr pField, TypePtr pType) const
{
	assert(pField);
	if (pField->type() != pType)
	{
		ClearType(pField);
		SetType(pField, pType);
	}
	return true;
}

bool ProjectInfo_t::DaFromAttrAtLocus(const Locus_t &l, AttrIdEnum attr, const value_t &v, DA2_t &da) const
{
	TypePtr iModule(l.module2());
	if (!iModule)
		return false;
	TypePtr iFrontSeg(l.frontSeg2());
	if (!iFrontSeg)
		return false;
	FRONT_t *pFront(FrontEnd(iFrontSeg));
	if (!pFront)
		return false;
	Seg_t *pSeg(l.seg()->typeSeg());
	ADDR64 imageBase(pSeg->imageBase(l.seg()));
	ADDR addr32;
	//ADDR64 eva;
	if (attr == ATTR_VA)//absolute
	{
		ADDR64 eva(v.ui64);
		if (imageBase > eva)
			return false;
		eva -= imageBase;
		if (eva > 0xFFFFFFFF)
			return false;
		addr32 = ADDR(eva);
		if (pSeg->isLarge())
			attr = ATTR_RVA;
	}
	else
	{
		addr32 = v.ui32;
/*		if (attr == ATTR_RVA)//base relative
			eva = addr32 + imageBase;
		else
			eva = addr32;*/
	}

	const Module_t &aModule(*iModule->typeModule());
	ModuleInfo_t MI(*this, *iModule);

	LocusInfo_t LI(l);
	I_Front::AKindEnum iKind(pFront->device(&aModule.dataSourceRef())->translateAddress(LI, aModule.moduleTag(), addr32, attr));
	if (iKind == I_Front::AKIND_NULL)
		return false;

	if (iKind == I_Front::AKIND_RAW)
	{
		da.row = 0;//addr32;
		if (!MI.R2D2(iModule, addr32, da.row))
			return false;
	}
	else if (iKind == I_Front::AKIND_VA)
	{
		TypePtr iSeg(MI.FindSegAt(iFrontSeg, addr32, pSeg->affinity()));
		if (!iSeg)
			return false;
		da.row = iSeg->typeSeg()->viewOffsAt(iSeg, addr32);
	}
	else if (iKind & I_Front::AKIND_OTHER)
	{
		int otherModuleTag(iKind & I_Front::AKIND_MODULE_MASK);
		FolderPtr pFolder(FindModuleFolderByTag(otherModuleTag));
		if (pFolder)
		{
			Locus_t l2;
			LocusInfo_t LI2(l2);//dummy
			ModuleInfo_t MI2(*this, *ModuleOf(pFolder));
			//the front should be the same!
			iKind = pFront->device(MI2.GetDataSource())->translateAddress(LI2, otherModuleTag, addr32, attr);//reference address ubknown?
			if (iKind != I_Front::AKIND_NULL)
			{
				da.pFolder = pFolder;
				da.row = addr32;
			}
		}
		return false;
	}
	return true;
}

Field_t	*ProjectInfo_t::findFieldByAddr(CTypePtr iSelf, ADDR a, unsigned objId)//ObjId_t objId)
{
	//Struc_t &rSelf(*iSelf->typeStruc());
	FieldPtr pField(Field(iSelf, a, nullptr, FieldIt_Exact));
	if (pField && pField->type() && pField->type()->ObjType() == objId)
		return pField;
	return nullptr;
}

Field_t	*ProjectInfo_t::findFieldByName(CTypePtr iSelf, const char *name)
{
	ADDR a;
	ObjId_t objId;
	if ((objId = Field_t::IsDefaultName(name, a)) != OBJID_NULL)
		return findFieldByAddr(iSelf, a, objId);

	Struc_t &rSelf(*iSelf->typeStruc());
	for (FieldMapIt it = rSelf.fields().begin(), E = rSelf.fields().end(); it != E; ++it)
	{
		FieldPtr pField(VALUE(it));
		if (!pField->nameless() && FieldName(pField) == name)
			return pField;
	}

	return nullptr;
}


TypePtr ProjectInfo_t::findContByName(CTypePtr iSelf, const char *name, ROWID &da)
{
	Struc_t &rSelf(*iSelf->typeStruc());

	ADDR a;
	ObjId_t objId;
	if ((objId = Field_t::IsDefaultName(name, a)) != OBJID_NULL)
	{
		FieldPtr pField(findFieldByAddr(iSelf, a, objId));
		if (!pField || !pField->type() || !pField->type()->typeComplex())
			return nullptr;
		da += pField->_key() - rSelf.base(iSelf);//structures just add up to the offs
		return pField->type();
	}

	//check subsegs first
	Seg_t *pSelf(iSelf->typeSeg());
	if (pSelf)
	{
		for (SubSegMapCIt i(pSelf->subsegs().begin()); i != pSelf->subsegs().end(); i++)
		{
			TypePtr iSeg2(IVALUE(i));
			Seg_t *pSeg2(iSeg2->typeSeg());
			if (pSeg2->title() == name)
			{
				da = pSeg2->viewOffs();//segments pass its view offs
				return iSeg2;
			}
		}
	}

	for (FieldMapIt it = rSelf.fields().begin(), E = rSelf.fields().end(); it != E; ++it)
	{
		FieldPtr pField(VALUE(it));
		if (!pField->nameless() && FieldName(pField) == name)
		{
			if (!pField->type() || !pField->type()->typeComplex())
				return nullptr;
			da += pField->_key() - rSelf.base(iSelf);//structures just add up to the offs
			return pField->type();
		}
	}

	return nullptr;
}

unsigned ProjectInfo_t::rangeFrom(TypePtr iSelf, ADDR a)
{
	Struc_t &rSelf(*iSelf->typeStruc());
	ADDR a2(iSelf->size());
	FieldMapIt itnx(rSelf.fields().end());
	FieldMapIt it(FieldIt(iSelf, a, &itnx));
	if (itnx != rSelf.fields().end())
		a2 = std::min(itnx->_key() - a, a2);
	return a2;
}

bool ProjectInfo_t::DaToLocus(DA_t da, Locus_t &aLoc, CTypeBasePtr iScope) const
{
	assert(iScope);
	if (iScope->typeSeg())
	{
		Block_t raw(iScope->typeSeg()->rawBlock());
		return terminalFieldAtSeg(iScope, da, aLoc, raw);
	}
	
	aLoc.add(iScope, (ADDR)da.row, nullptr);
	terminalFieldAt(aLoc);//iScope, da, l, Block_t());
	return true;
}

/*FieldPtr ProjectInfo_t::appendStrucvarField(TypePtr iSelf)
{
	Strucvar_t &rSelf(*iSelf->typeStrucvar());

	FieldMap &m(rSelf.fields());
	long offs(0);
	if (!m.empty())
		offs = VALUE(m.rbegin())->_key();

//	ADDR offs((long)rSelf.fields().Size());//actual offset is not important for strucvar's fields
//	if (offs == 0)
		offs++;//reserve slot 0 for bitfields

	FieldPtr pField(mrMemMgr.NewField(offs));
	//SetType(pField, pFieldType);

	rSelf.insertField(pField);
	pField->setOwnerComplex(iSelf);

	return pField;
}*/


FieldPtr  ProjectInfo_t::AddField(TypePtr iSelf, FieldPtr  pFieldNew, ADDR nOffset) const
{
CHECK(nOffset == 2624)
STOP

	Struc_t *pSelf(iSelf->typeStruc());

	assert(pFieldNew);
	assert(pSelf->ObjType() != OBJID_TYPE_STRUC || pFieldNew->objField());
	//assert(ObjType() != OBJID_TYPE_FUNCDEF || pFieldNew->IsArgOp());

//CHECK(CompNamex("Struc@sub_4015E0@13"))
//STOP

//?	pFieldNew->Unlink();
	if (pFieldNew->OwnerStruc())
	{
		Struc_t * pStruc = pFieldNew->OwnerStruc();
		pStruc->takeField(pFieldNew);
	}

	FieldMapIt itPr = pSelf->fields().end();
	for (FieldMapIt it = pSelf->fields().begin(), E = pSelf->fields().end(); it != E; ++it)
	{
		FieldPtr  pField = VALUE(it);
		if (pField->_key() > nOffset)
			break;
		itPr = it;
	}

//	if (pField)
//	if (pField->offset() == nOffset)
//	{
//		return pField;
//	}
//assert(0);
/*	if (itPr == mFields.end())
		mFields.push_front(pFieldNew);
	else
		mFields.insert(it, pFieldNew);*/
	InsertUniqueFieldIt(iSelf, nOffset, pFieldNew);

	assert(0);//?pFieldNew->mpOwner = this;
//	pFieldNew->setOffset( nOffset );

	if (!pSelf->typeSeg())
	{
		//pFieldNew->Is Label() )
		assert(0);//?pFieldNew->ObjUnlink();
	}

	return pFieldNew;
}


FieldPtr ProjectInfo_t::AddField(TypePtr iSelf, ADDR nOffset, int nMode, bool bThis) const
{
CHECK(nOffset == 2624)
STOP

	Struc_t *pSelf(iSelf->typeStruc());
	assert(pSelf->isValid());

	FieldMapIt itPr = pSelf->fields().end();
	FieldMapIt it = pSelf->fields().begin();
	if (it != pSelf->fields().end())
	{
		FieldMapIt E(pSelf->fields().end());
		while (it != E)
		{
			FieldPtr  pField = VALUE(it);
			if (pField->_key() >= nOffset)
				break;
			itPr = it;
			it++;
		}

		if (it != pSelf->fields().end())
		{
			FieldPtr  pField = VALUE(it);
			if (pField->_key() == nOffset)
			{
				if (nMode == 1 || nMode == 2)
				if (pField->IsComplex0())
				{
					FieldPtr pField2 = AddField(pField->type(), 0, nMode, bThis);
					if (nMode == 1)
						pField = pField2;
				}
				return pField;
			}
		}
	}

	FieldPtr  pFieldPr = nullptr;
	if (itPr != pSelf->fields().end())
		pFieldPr = VALUE(itPr);

	if (nMode == 1 || nMode == 2)
	if (pFieldPr)
	if (pFieldPr->_key() + pFieldPr->size() > nOffset)//overlapped with prev
	if (pFieldPr->IsComplex0())
		return AddField(pFieldPr->type(), nOffset - pFieldPr->_key(), nMode, bThis);
	else
		return 0;

	if (nOffset < 0)
	{
#if(1)
		return 0;
#else
		fprintf(stdout, "%s: Adding field with negative offset %d\n", iSelf->namexx().c_str(), nOffset);
#endif
	}
	else if (nOffset == 0)
		if (bThis)
			return 0;

//CHECK(nOffset == 0)
//CHECK(CompNamex("Struc@_sub_501CB0@This"))
//STOP

	FieldPtr  pField = mrMemMgr.NewField(nOffset);//NEW!!!
//assert(0);
/*?	mFields.insert(it, pField);*/
	InsertUniqueFieldIt(iSelf, nOffset, pField);

	assert(pField->_key() == nOffset);//pField->setOffset( nOffset );
	//pField->mpOwner = this;
	ON_OBJ_CREATED(pField);
/*
	//reapply this struct
	for (XRef_t *pXRef = m_pXRefs; pXRef; pXRef = pXRef->N ext())
	{
		if (pXRef->pField->IsField())
		if (!pXRef->pField->isPtr())
			pXRef->pField->ApplyStruct(this);
	}
*/
	return pField;
}

FieldMapIt ProjectInfo_t::InsertFieldIt(Frame_t &aTop, unsigned uSize) const
{
	TypePtr iSelf(aTop.cont());
	Struc_t &rSelf(*iSelf->typeStruc());
	FieldMap &rFields(rSelf.fields());

	if (aTop.field())
		return rFields.end();

	ADDR offs(aTop.addr());
	/*if (offs == -1)
	{
		PrintError() << "Field insertion rejected at offset (" << VA2STR(offs) << ") for " 
			<< TypeName(iSelf)
			<< std::endl;
		return rFields.end();
	}*/
	
	if (!(offs >= iSelf->base()))
	{
		PrintError() << "Below base (" << VA2STR(offs) << ") field insert rejected ("
			<< TypeName(iSelf) << ", base=" << VA2STR(iSelf->base()) << ")"
			<< std::endl;
		return rFields.end();
	}

//CHECK(offs==0x10065dc)
//STOP

	FieldMapIt it(rFields.lower_bound(offs));
	if (it != rFields.end() && KEY(it) == offs)
	{
		aTop.setField(VALUE(it));
		return rFields.end();//a field already exists
	}

	if (uSize != 0)
	{
		ADDR offsNx((it != rFields.end()) ? KEY(it) : iSelf->attic());
		assert(offsNx != -1);
		if (offsNx - offs < uSize)
			return rFields.end();
	}
	aTop.setField(mrMemMgr.NewField(offs));
	it = InsertUniqueFieldIt(iSelf, offs, aTop.field());
	assert(aTop.field()->_key() == offs);
	assert(VALUE(it));
	return it;
}

FieldMapIt ProjectInfo_t::InsertFieldIt(TypePtr iSelf, FieldPtr pField) const
{
	Struc_t &rSelf(*iSelf->typeStruc());
	assert(!rSelf.typeProxy());
	assert(iSelf->typeUnion() || rSelf.typeStrucLoc());
	assert(!pField->owner());
	FieldMapIt i(rSelf.insertFieldIt(pField, nullptr));
	if (i != rSelf.fields().end())
		pField->setOwnerComplex(iSelf);
	return i;
}

bool ProjectInfo_t::InsertField(Locus_t &aLoc, unsigned checkSize) const
{
	Frame_t &aTop(aLoc.back());
	Struc_t &rSelf(*aTop.cont()->typeStruc());
	FieldMapIt i(InsertFieldIt(aTop, checkSize));
	if (i == rSelf.fields().end())
		return false;
	return aTop.field() != nullptr;
}

/*FieldPtr ProjectInfo_t::AppendFieldOfType(TypePtr iSelf, TypePtr pType)
{
	Struc_t *pSelf(iSelf->typeStruc());
	int offs = iSelf->base();
	if (!pSelf->fields().empty())
	{
		FieldMapIt it = pSelf->fields().end();
		it = pSelf->fields().Prior(it);
		offs += KEY(it);
		FieldPtr  pFieldLast = VALUE(it);
		int sz = pFieldLast->size();
		if ( sz <= 0 )
			sz = 1;
		offs += sz;
	}
	return InsertFieldOfType(frame_t(iSelf, offs), pType);
}*/

FieldPtr ProjectInfo_t::AppendUField(Locus_t& aLoc) const
{
	TypePtr pCont(aLoc.struc());
	FieldPtr pField;
	if (pCont->typeUnion())
	{
		Struc_t& rSelf(*pCont->typeUnion());
		FieldMap& m(rSelf.fields());
		ADDR uKey(pCont->base());
		pField = mrMemMgr.NewField(uKey);
		if (m.insert(pField) == m.end())
			ASSERT0;
	}
	else
	{
		Struc_t& rSelf(*pCont->typeStruc());
		FieldMap& m(rSelf.fields());
		pField = mrMemMgr.NewField(aLoc.addr());
		if (m.insert(pField) == m.end())
			ASSERT0;
	}
	pField->setOwnerComplex(pCont);
	//ON_OBJ_CREATED(pField);
	aLoc.setField(pField);
	return pField;
}

FieldMapIt ProjectInfo_t::InsertUniqueFieldIt(TypePtr iSelf, ADDR va, FieldPtr pField) const
{
CHECKID(pField, 0x359)
STOP
	assert(!iSelf->typeProxy());
	Struc_t &rSelf(*iSelf->typeStruc());
	assert(!pField->owner());
	FieldPtr pEos(nullptr);
	FieldMapIt i(rSelf.insertUniqueFieldIt(va, pField, &pEos));
	if (pEos)
	{
		assert(pEos->nameless() && !pEos->type() && !pEos->hasUserData());
		memMgr().Delete(pEos);
	}
	if (i != rSelf.fields().end())
		pField->setOwnerComplex(iSelf);
	return i;
}

bool ProjectInfo_t::InsertFieldAt(TypePtr iStruc, FieldPtr pField, ADDR key) const
{
	assert(!iStruc->typeProxy());
	//assert(!FuncInfo_s::SSID(pField) || pField->_key() == FuncInfo_s::loc2key(pField));
	assert(!pField->owner());
	Struc_t &rStruc(*iStruc->typeStruc());
	FieldPtr pEos(nullptr);
	FieldMapIt it(rStruc.insertUniqueFieldIt(key, pField, &pEos));
	if (pEos)
	{
		//assert(!pField->isLocal());
		assert(!iStruc->typeUnion());
		assert(pEos->nameless() && !pEos->type() && !pEos->hasUserData());
		memMgr().Delete(pEos);
	}
	if (it == rStruc.fields().end())
		return false;
	pField->setOwnerComplex(iStruc);
	assert(pField->_key() == key);
	return true;
}

FieldPtr ProjectInfo_t::AppendBField(TypePtr iSelf) const
{
	Bitset_t &rSelf(*iSelf->typeBitset());

	FieldMap &m(rSelf.fields());
	ADDR offs(0);
	if (!m.empty())
		offs = m.rbegin()->_key() + 1;//actual offset is not important for bitvar's fields

	FieldPtr pField(mrMemMgr.NewField(offs));
	//SetType(pField, pFieldType);

	rSelf.insertField(pField);
	pField->setOwnerComplex(iSelf);
	return pField;
}

MyString ProjectInfo_t::FolderPathAbs(CFolderPtr pFolder) const
{
	return FolderPathRel(pFolder, nullptr);
}

MyString ProjectInfo_t::FolderPathRel(CFolderPtr pFolder, CFolderPtr from)  const
{
	return mrProject.files().relPath(pFolder, from);
}


MyString ProjectInfo_t::TypeName0(CTypePtr p, char chopSymb) const
{
	MyString s;
	ProjectInfo_t::ChopName(mrProject.Project_t::typeName(p), s, chopSymb);
	return s;
}

MyString ProjectInfo_t::TypeName(CTypePtr p, char chopSymb) const
{
	MyString s;
	ProjectInfo_t::ChopName(mrProject.typeName(p), s, chopSymb);
	return s;
}

MyString ProjectInfo_t::TypeDisplayName(CTypePtr p) const
{
	assert(p);
	MyString s(mrProject.typeName(p));
	if (p->typeEnum())
	{
		size_t n;
		if ((n = s.find('<')) != std::string::npos)
		{
			assert(0);
			s.resize(n);//trim enum's names
		}
	}
	return s;
}

//strip of name and type
void ProjectInfo_t::TakeField0(TypePtr iSelf, FieldPtr pField) const
{
	assert(pField && pField->owner() == iSelf);
	Struc_t &rSelf(*iSelf->typeStruc());
	FieldMap& m(rSelf.fields());

	/*if (pField->isClone())
	{
		FieldPtr pField0(CloneLead(pField));

		Seg_t& rSeg(*pField->owner()->typeSeg());
		ConflictFieldMap &m2(rSeg.conflictFields());
		for (ClonedFieldMapIt i(m2.lower_bound(pField->_key()));; ++i)
		{
			assert(i != m2.end() && KEY(i) == pField->_key());
			if (VALUE(i) == pField)
			{
				//demote the master field
				m2.take(i++);
				if (m2.size() == 1)
				{
					i = m2.begin();
					FieldPtr pLast(VALUE(i));
					m2.take(i);
					FieldMapIt j(m.find(pField0->_key()));
					assert(j != m.end());
					m.take(j);
					m.insert(pLast);
					pLast->flags() &= ~FLD_CLONED;
					pLast->setType0(pField0->type());
					pField0->setType0(nullptr);
					memMgr().Delete(pField0);
					STOP
				}
				break;
			}
		}
		pField->flags() &= ~FLD_CLONED;
		pField->setOwnerComplex(nullptr);

		if (!pField->nameless())
			ClearFieldName(pField, rSelf.namesMgr());

		if (pField->type())
		{
			BinaryCleaner_t<> PC(*this);
			PC.ClearType(pField);
		}
		return pField;
	}*/

	FieldMapIt it(pField);// m.find(pField->_key()));
	//if (it == m.end())
		//return false;

	if (!pField->nameless())
		ClearFieldName(pField, rSelf.namesMgr());

	if (pField->type())
	{
		BinaryCleaner_t<> PC(*this);//, iModule);
		PC.ClearType(pField);
	}

	rSelf.takeField0(it);
}

TypePtr ProjectInfo_t::VA2Locus(CTypePtr iContext, ADDR va, Locus_t &aLoc, bool bWrapUp) const
{
	if (!iContext || !iContext->typeSeg() || iContext->typeModule())
		return nullptr;

	CTypePtr iSeg1(iContext);
	if (!IsInside(iSeg1, va))
	{
		unsigned affinity(iSeg1->typeSeg()->affinity());
		TypePtr iSeg0(OwnerSegList(iSeg1));
		if (!iSeg0)
			return nullptr;
		iSeg1 = iSeg0->typeSeg()->findSubseg(va, affinity);
		if (!iSeg1)
			return nullptr;
	}

	const Seg_t &rSeg1(*iSeg1->typeSeg());
	//DA_t da(va, 0, 0);
	aLoc.push_back(Frame_t(rSeg1.rawBlock(), (TypePtr)iSeg1, va, nullptr));
	terminalFieldAt(aLoc);
	//if (!terminalFieldAt(iSeg1, da, aLoc, iSeg1->typeSeg()->rawBlock()))
		//aLoc.push_back(Frame_t(iSeg1, (ADDR)da.row, nullptr));//no field

	if (bWrapUp)
		aLoc.wrapUp();

	return (TypePtr)iSeg1;
}

int ProjectInfo_t::PrintTypesList(const TypesMgr_t &rSelf, std::ostream& os, int mode)
{
	int iLevel(0);
	for (TypePtr piStruc(rSelf.owner()); piStruc && piStruc->parentField(); piStruc = piStruc->parentField()->owner())
		iLevel++;

	const TypesMap_t &m(rSelf.aliases());
	for (TypesMapCIt it(m.begin()); it != m.end(); it++)
	{
		if (it == m.begin())
		{
			for (int i(iLevel); i > 0; i--)
				os << "\t";
			std::cout << "[" << TypeName(rSelf.owner()) << "]\n";
		}

		for (int i(iLevel); i > 0; i--)
			os << "\t";

		TypePtr pTypeCplx(it->pSelf);
		//		const char *pName(pType->name());
		//		if (pName)
		//			os << pName;
		//		else
		os << TypeName(pTypeCplx);
		os << "\n";
		if (mode == 1)
		{
			assert(0);
			/*?if (pComplex)
			{
				for (XRefObjList_t::Iterator i(pComplex->xrefs_()); i; i++)
				{
					Obj_t *pObj(i.data().pObj);
					TypePtr pType(0);
					if (pObj->objField())
						pType = pObj->objField()->type();
					else
					{
						TypePtr iType(pObj->objTy peRef());
						if (iType)
						{
							if (iType->typePtr())
								pType = iType->typePtr()->pointee();
							else if (iType->typeArray())
								pType = iType->typeArray()->baseType();
						}	
					}
					assert(pType);
					assert(pType == pTypeCplx);
					for (int i(iLevel + 1); i > 0; i--)
						os << "\t";
					os << "@" << pObj->namexx();
					os << "\n";
				}
			}*/
		}
	}
	return 1;
}

TypePtr ProjectInfo_t::SetType(TypeImp_t &rSelf, TypePtr pType, bool bNoRef) const
{
	assert(pType && !rSelf.baseType());
	if (pType)
	{
		assert(!pType->typeFuncDef());
		assert(!pType->typeProc());
		assert(!pType->typeSeg());
		if (pType == rSelf.baseType())
			return rSelf.baseType();
	}
	assert(!rSelf.baseType());
	rSelf.setBaseType(pType);
	if (!bNoRef)
		AcquireTypeRef(rSelf.baseType());
	return rSelf.baseType();
}

TypePtr ProjectInfo_t::SetType(TypeExp_t &rSelf, TypePtr pType, bool bNoRef) const
{
	assert(pType && !rSelf.baseType());
	if (pType)
	{
		assert(!pType->typeFuncDef());
		assert(!pType->typeProc());
		assert(!pType->typeSeg());
		if (pType == rSelf.baseType())
			return rSelf.baseType();
	}
	assert(!rSelf.baseType());
	rSelf.setBaseType(pType);
	if (!bNoRef)
		AcquireTypeRef(rSelf.baseType());
	return rSelf.baseType();
}

TypePtr ProjectInfo_t::SetType(TypePtr_t &rSelf, TypePtr pType, bool bNoRef) const
{
	assert(pType && !rSelf.pointee());
	if (pType)
	{
		assert(!pType->typeFuncDef());
		assert(!pType->typeProc());
		if (pType == rSelf.pointee())
			return rSelf.pointee();
	}

//CHECK(ID()==0xb1)
//STOP

	assert(!rSelf.pointee());
	//Release TypeRef(rSelf.mpBaseType);

	rSelf.setPointee0(pType);
	if (!bNoRef)
		AcquireTypeRef(rSelf.pointee());

	//	if ( !mpType )
	//		mpType = TYPEMGR.type( OPTYP_NULL );

	return rSelf.pointee();
}

TypePtr ProjectInfo_t::SetType(TypeProxy_t &rSelf, TypePtr pType) const
{
	assert(pType && !rSelf.incumbent());
	assert(!pType->typeFuncDef());
	//if (pType)
		if (pType == rSelf.incumbent())
			return rSelf.incumbent();

	assert(!rSelf.incumbent());
	//Release TypeRef(rSelf.mpBaseType);

	rSelf.setIncumbent(pType);
	AcquireTypeRef(rSelf.incumbent());

	return rSelf.incumbent();
}

TypePtr ProjectInfo_t::SetType(TypeThunk_t &rSelf, TypePtr pType) const
{
	assert(pType && !rSelf.baseType());
	assert(pType->typeCode());
	if (pType == rSelf.baseType())
		return rSelf.baseType();

	rSelf.setBaseType(pType);
	AcquireTypeRef(rSelf.baseType());

	return rSelf.baseType();
}

int ProjectInfo_t::DeleteStrucObj(TypePtr iSelf, ADDR d)
{
	Struc_t &rSelf(*iSelf->typeStruc());

	ADDR offs = d;
	FieldMapIt it_f = StrucFieldIt(iSelf, offs );
	if (it_f == rSelf.fields().end())
		return 0;

	FieldPtr  f = VALUE(it_f);
	ADDR fa = KEY(it_f);

	if (f->type())
	{
		if (f->type()->typeStruc())
		{
			d = offs - fa;
			if (DeleteStrucObj(f->type(), d + f->type()->base()))
				return 1;
		}
		Array_t * a(f->type()->typeArray());
		if (a)
		{
			d = offs - fa;
			TypePtr t(a->baseType());
			if (t->typeStruc())
			{
				int i = d / t->size();
				d = d - i * t->size();
				if (DeleteStrucObj(t, d + f->type()->base()))
					return 1;
			}
		}
	}

	if ( fa != offs )
		return 0;

	rSelf.takeField0(it_f);
	delete f;

	return 1;
}


int ProjectInfo_t::ClearTypeName(NamesMgr_t &rNS, TypePtr pSelf) const
{
//CHECKID(pSelf, 0x36)
//CHECK(pSelf->typeProxy())
//STOP
	if (!pSelf->nameless())
	//if (!pSelf->typeProxy())
	{
		RemoveNameRef(rNS, pSelf->name());
		pSelf->setName(nullptr);
	}
	return true;
}

MyString ProjectInfo_t::FieldName(CFieldPtr p, char chopSymb) const
{
//CHECK(p->_key() == 0x5B80EC)
//STOP
	MyString s;
	ProjectInfo_t::ChopName(mrProject.fieldDisplayName(p), s, chopSymb);
	return s;
}

MyString ProjectInfo_t::FieldName0(CFieldPtr p, char chopSymb) const
{
//CHECK(p->_key() == 0x5B80EC)
//STOP
	MyString s;
	ProjectInfo_t::ChopName(mrProject.Project_t::fieldDisplayName(p), s, chopSymb);
	return s;
}

int ProjectInfo_t::ClearFieldName(FieldPtr pSelf, NamesMgr_t* pNS) const
{
	if (pSelf->name())
	{
		if (!pNS)
			pNS = OwnerNamesMgr(pSelf, nullptr);
		if (!RemoveNameRef(*pNS, pSelf->name()))
			return false;
		pSelf->setName(nullptr);
		pSelf->clearUglyName();
		pSelf->setHardNamed(false);
	}
	return true;
}

TypePtr ProjectInfo_t::TakeTypeOf(FieldPtr pField) const
{
	TypePtr iType(pField->mpType);
	pField->setType0(nullptr);
	if (iType && !iType->isShared())
		iType->setParentField(pField);
	//iType->releas eRef();
	return iType;
}

bool ProjectInfo_t::IsTypeBit(TypePtr iType) const
{
	if (!iType->typeSimple())
		return false;
	FieldPtr pField(iType->parentField());
	if (!pField)
		return false;
	iType = pField->owner();
	if (iType->typeBitset())
		return true;
	if (!iType->typeArray())
		return false;
	pField = iType->parentField();
	if (!pField)
		return false;
	return (iType->typeBitset() != nullptr);
}

//void ClearType(TypePtr_t &) const;
//void ClearType(Array_t &) const;
//void ClearType(Typedef_t &) const;

bool ProjectInfo_t::CheckTypeInclusion(CTypePtr pType, CTypePtr pType2)//check if pType is contained in pType2 (directly or indirectly)
{
	assert(pType2 && pType2->typeStruc());
	assert(!pType->isEnum());
	assert(!pType->typeFuncDef());
	if (pType == pType2)
		return true;
	for (Struc_t::HierIterator i(pType2, true); i; i++)//step into shared!
	{
		CFieldRef r(*i);
		if (SkipArray(r.type()) == pType)
			return true;//a struc cannot contain itself (directly or indirectly)
	}
	return false;
}

void ProjectInfo_t::ClearType(FieldPtr pSelf) const
{
	if (!pSelf->type())
		return;
	BinaryCleaner_t<> PC(*this);
	PC.ClearType(pSelf);
}

TypePtr ProjectInfo_t::SetType(FieldPtr pSelf, TypePtr pType) const
{
CHECKID(pSelf, 0x00008fe8)
STOP
	//assert(!pSelf->isClone());
	if (pType == pSelf->mpType)
		return pSelf->mpType;

	assert(!pSelf->mpType);

	//avoid infinite recursion: check if a field's owner is not directly or indirectly contained in a new type
	if (pType)
	{
		CTypePtr pType2(SkipArray(pType));
		if (pType2->typeStruc())
		{
			if (!pSelf->owner()->typeFuncDef())
			if (CheckTypeInclusion(pSelf->owner(), pType2))
			{
				//PrintWarning() << "Setting a type makes a containing record recursive - prevented (" << TypeName(pType, CHOP_SYMB) << ", offset=" << VA2STR(pSelf->_key()) << ")" << std::endl;
				PrintWarning() << "Failed to apply a type producing recursion: " << TypeName(pType) << " (in " << TypeName(pSelf->owner()) << ")" << std::endl;
				return nullptr;
			}
		}
	}

	pSelf->mpType = pType;
	AcquireTypeRef(pSelf->mpType);

	if (pSelf->mpType && !pSelf->mpType->isShared())
		pSelf->mpType->setParentField(pSelf);

	return pSelf->mpType;
}

bool ProjectInfo_t::RemoveNameRef(NamesMgr_t &rNS, PNameRef iName) const
{
	if (!rNS.isInitialized())
		return false;
	assert(iName);
	//WARNING: iName may reside in different module
	PNameRef iName2(rNS.removen(iName));
	if (!iName2)
		return false;
	mrMemMgr.Delete(iName2);
	return true;
}


bool ProjectInfo_t::SetRootDirectory(const MyDirPath &dir)
{
	AssureRootFolder();
	mrProject.files().rootFolder()->setName(dir.Dir());
	return true;
}

OFF_t ProjectInfo_t::rawOffs(CTypePtr iSelf) const
{
	Seg_t &rSelf(*iSelf->typeSeg());
#if(0)
	if (rSelf.traceLink())
		return rSelf.traceLink()->parentField()->_key();
	//if (segRef())
		//return segRef()->parentField()->offset();
	if (rSelf.typeModule())
		return 0;
	FieldPtr pField(iSelf->parentField());
	if (pField)
		return pField->_key();
#endif
	return rSelf.rawBlock().m_offs;
}

OFF_t ProjectInfo_t::rawSize(CTypePtr iSelf) const
{
	Seg_t &rSelf(*iSelf->typeSeg());
	if (rSelf.traceLink())
		return rSelf.traceLink()->size();
//	if (segRef())
	//	return segRef()->size();
	if (rSelf.typeModule())
		return rSelf.typeModule()->rawBlock().m_size;
//	FieldPtr pField(iSelf->parentField());
	//if (pField)
		//return iSelf->size();
	return rSelf.rawBlock().m_size;
}

FolderPtr ProjectInfo_t::AddFileEx(Folder_t &rFolder, const char *name, int type)
{
	//Folder_t *pFile(memMgrGlob().NewFile());
	//pFile->data().SetNameType(name, type);
	//?rFolder.files().push_back(pFile);
#if(0)
	for (FileTree_t::ChildrenIterator i(&rFolder); i; i++)
	{
		Folder_t &folder(*i.data());
		Folder_t &file(folder);
		if (file.theName() == MyString(name))
			return nullptr;

		if (!FilesMgr0_t::IsFolder(folder))
		{
			if (file.theName() > MyString(name))
			{
				FolderPtr pFile(memMgrGlob().NewFile(name));
				//pFile->data().setName(name);
				TreeInsertChildBefore(&rFolder, pFile, &folder);
				return pFile;
			}
		}
	}

	Folder_t *pFile(memMgrGlob().NewFile(name));
	//pFile->data().setName(name);
	TreePushChildBack(&rFolder, pFile);
	return pFile;
#else
	FolderPtr pNew(memMgrGlob().NewFile(name));
	FileFolder_t *p(rFolder.fileFolder());
	assert(p);
	if (!p->children().insert(pNew).second)
	{
		memMgrGlob().Delete(pNew);
		return nullptr;
	}
	pNew->setParent(&rFolder);
	return pNew;
#endif
}


MyString ProjectInfo_t::FieldDisplayName0(FieldPtr pField) const
{
	if (!pField->isTemp() && pField->owner())
	{
		TypePtr iModule(ModuleOf(pField->owner()));
		if (iModule)
		{
			ModuleInfo_t MI(*(ProjectInfo_t *)this, *iModule);
			return MI.FieldDisplayName(pField);
		}
	}
	return FieldName(pField);
}

TypePtr ProjectInfo_t::FindAtticTypeByName(const MyString &s0) const//NO auto names!
{
	TypesMgr_t *ptm(mrProject.typeMgr());
	if (ptm)
	{
		//check stock types first
		OpType_t optyp;
		if (Str2OpTyp(s0, optyp))
			return ptm->getStockType(optyp);

		//no autonames in attic!
		TypePtr iScope(mrProject.self());
		NamesMgr_t *pNs(iScope->typeComplex()->namesMgr());
		if (pNs)
		{
//			NamespaceInitialized((TypePtr)iScope);

			MyStringList l(MyStringList::split("::", s0));
			assert(!l.empty());
			for (;;)
			{
				MyString s(l.front());
				l.pop_front();
				ObjPtr pObj(pNs->findObj(s0));
				if (!pObj)
					break;
				TypePtr iType(pObj->objTypeGlob());
				if (!iType)
					break;
				if (l.empty())
					return iType;
				iScope = iType;
			}
		}
	}
	return nullptr;
}

TypesMgr_t *ProjectInfo_t::superTypesMgr(TypesMgr_t *pTypeMgr) const
{
	TypePtr pOwner(pTypeMgr->owner());
	if (pOwner->typeSeg())
		pTypeMgr = findTypeMgr(pOwner->typeSeg()->superLink());
	else if (pOwner->parentField())
		pTypeMgr = findTypeMgr(pOwner->parentField()->owner());
	else if (pOwner->owner())
		pTypeMgr = findTypeMgr(pOwner->owner());
	else
		pTypeMgr = nullptr;
	return pTypeMgr;
}

FRONT_t *ProjectInfo_t::FrontOf(TypePtr iFrontSeg) const
{
	return FrontEnd(iFrontSeg);
}

I_Front *ProjectInfo_t::IFrontOf(TypePtr iFrontSeg) const
{
	if (iFrontSeg)
	{
		FRONT_t *pFE(FrontOf(iFrontSeg));
		if (pFE)
		{
			ModuleInfo_t MI(mrProject, *ModuleOf(iFrontSeg));
			return pFE->device(MI.GetDataSource());
		}
	}
	return nullptr;
}

int ProjectInfo_t::RecoilNoOutDir(const MyString &s) const
{
	gui().GuiOnNoOutputDirectory(s);
	fprintf(stdout, "Please specify an Output directory.\n");
	return 0;
}

MyString ProjectInfo_t::AtStr(Cmd_t& args, bool bRemove)
{
	//DECLARE_CONTEXT(ctx);

	//	if (!ctx.empty())
		//	return true;


		//extract affinity
	/*	unsigned uAffinity(0);

		Probe_t aLocRef;
		MyString sAtRef(args.FindOpt("-ref"));
		if (!sAtRef.empty())
			if (Str2DA(sAtRef, &da) && LocusFromDA(da, aLocRef))
			{
				if (aLocRef.seg())
					uAffinity = aLocRef.seg()->typeSeg()->affinity();
			}*/

			//loc = ctx;
	MyString sAt;
	if (bRemove)
		sAt = args.RemoveOptOrPfx("-@");
	else
		sAt = args.FindOptOrPfx("-@");

	return sAt;
}



	
//	return MI.LocusFromStr(sAt, pStartSeg, ctx.locus());


bool ProjectInfo_t::LocusFromStr(MyString sAt, Locus_t& aLoc, CTypePtr pStartSeg)
{
	CTypePtr pModule(aLoc.module2());
	if (!pModule)
		pModule = ModuleOf(pStartSeg);
	if (!pModule)
		return false;

	ModuleInfo_t MI(*this, *pModule);

	if (sAt.empty())
		return false;

	ROWID da;
	if (!(MI.Str2DA(sAt, &da, pStartSeg) && LocusFromDA(da, aLoc)))
	{
		PrintError() << "Invalid target address in command: " << sAt << std::endl;
		return false;
	}
	return true;
}

/*size_t ProjectInfo_t::BinaryIndexFromFileOffs(size_t addr) const
{
	size_t uBinaryIndex(-1);
	for (size_t i(0); i < mrProject.binaryInfo().size(); i++)
	{
		const BinaryInfo_t &aBin(mrProject.binaryInfo().at(i));
		if (aBin.mRaw.offs <= addr && addr < aBin.mRaw.offs + aBin.mRaw.size)
		{
			uBinaryIndex = i;
			break;
		}
	}
	return uBinaryIndex;
}*/

FieldPtr ProjectInfo_t::adjustPick(Locus_t &rSelf) const
{
	assert(!rSelf.empty());
	FieldPtr pField(rSelf.field0());
	/*???if (rSelf.back().pCont->isShared())//delete shared containers as a whole
	{
		rSelf.pop_back();
		pField = rSelf.field();
	}
	else */if (!pField)//exact pick at zero offset for in-place containers
	{
		ADDR addr(rSelf.addr());
		ADDR base(rSelf.back().cont()->base());
		rSelf.pop_back();
		if (!rSelf.empty() && addr - base == 0)
			pField = rSelf.field0();
	}
	else if (IsEntryLabel(pField))
	{
		//delete a function
		rSelf.pop_back();
		assert(!rSelf.empty());
		pField = rSelf.field0();
	}
	
	if (pField)
	{
		TypePtr iType(pField->type());
		if (iType && iType->typeBitset() && pField->_key() == 0)
		{
			rSelf.pop_back();
			pField = rSelf.field0();
		}
	}
	return pField;
}

Debugger_t * ProjectInfo_t::newDebugger() const
{
	ModuleInfo_t MI(mrProject, *ModuleOf(mrProject.mpStartUpBinary));
	MyString sPath(MI.ModulePath());

	Debugger_t *pDbg(new Debugger_t(*this, sPath));//mrProject.startUpBinaryPath());
	pDbg->setContext(new Probe_t);
	mrProject.OnNewDebugger();
	return pDbg;
}

bool ProjectInfo_t::startDebugger()
{
	if (!mrProject.debugger())
	{
		mrProject.setDebugger(newDebugger());
		//const Seg_t *pSeg(findCodeSeg());
		//mpDebugger->setRange(pSeg->base(), pSeg->size());
		mrProject.debugger()->debug();
		gui().GuiDebuggerStarted();
		return true;
	}
	return false;
}

bool ProjectInfo_t::destroyDebugger()
{
	if (mrProject.debugger())
	{
		delete mrProject.debugger();
		mrProject.setDebugger(nullptr);
		gui().GuiDebuggerStopped();
		//gui().GuiOnExprModified();//redraw view
		return true;
	}
	return false;
}

/*TypePtr ProjectInfo_t::NameOwnerOf(FieldPtr pField) const
{
	CFieldPtr pField(pField);
	while (pField)
	{
		TypePtr pCplx(pField->OwnerComplex());
		if (pCplx->typeFuncDef())
			return pCplx;
		if (pCplx->typeUnion())
			return IsLocalsTop(pCplx);
		pField = pCplx->parentField();
	}
	return nullptr;
}*/


/*bool ProjectInfo_t::SetName0(FieldPtr pField, PNameRef pNameRef)
{
	pNameRef->addRef();
	pField->setName(pNameRef);
	return true;
}

bool ProjectInfo_t::SetName0(TypePtr iType, PNameRef pNameRef)
{
	pNameRef->addRef();
	iType->setName(pNameRef);
	return true;
}*/


FolderPtr ProjectInfo_t::AddModule(const MyPath &aPath, bool bCaseSensitive, Probe_t &aLoc, MyString opts) const
{
	if (aPath.empty())
		return nullptr;

	//DECLARE_CONTEXT(ctx);
	//MyPath aPath(args[1], args.mRefPath);
	FolderPtr pFolder(FindModuleFolder(aPath.Name(), bCaseSensitive));
	if (pFolder && !IsPhantomFolder(*pFolder))
	{
		fprintf(STDERR, "Warning: Module already loaded: %s\n", aPath.Name().c_str());
		return pFolder;
	}

	pFolder = LoadBinary(aPath);
	if (!pFolder)
	{
		PrintError() << "Could not load module at path: " << aPath.Path() << std::endl;
		return nullptr;
	}

	TypePtr iModule(pFolder->fileModule()->module());

	Module_t &aBin(*iModule->typeModule());
	aLoc.clear_all();
	aLoc.locus().push_back(Frame_t(aBin.rawBlock(), iModule, 0, nullptr));
	//?aLoc.setFolder(pFolder);

	MyString s("preformat");
	if (!opts.isEmpty())
		s.append(" " + opts);
	s.append(" -n " + ModuleTitle(iModule));

	mrMain.postContextCommand(s.c_str(), &aLoc);//raw offs

	//aBin.assureN amespace();
	//TypesMgr_t *pTypesMgr(assureTypeMgr(iModule));
	//Stockracer_t tt(mrProject, *pTypesMgr);
	//tt.AddStockTypes();

	//UpdateViewGeometry2();
	//gui().GuiOnFileListChanged();
	mrProject.markDirty(DIRTY_FILES);
	return pFolder;
}

//chec if p is a nested type in q
bool ProjectInfo_t::CheckNested(TypePtr p, TypePtr q) const
{
	assert(p != q);
	while (p)
	{
		p = p->owner();
		if (p == q)
			return true;
	}
	return false;
}


void ProjectInfo_t::AssureRootFolder() const
{
	if (!mrProject.files().mpRootFolder)
	{
		FolderPtr pFolder(memMgrGlob().NewFile(""));
		pFolder->SetPvt(new FileFolder_t);
		mrProject.files().setRootFolder(pFolder);
	}
}

FolderPtr ProjectInfo_t::RenameFolder(Folder_t &rFolder, const MyString &newName) const
{
	FoldersMap &m(rFolder.Parent()->fileFolder()->children());
	FoldersMap::iterator it(&rFolder);
	FolderPtr pFolder(m.take(it));
	assert(pFolder && pFolder == &rFolder);
	pFolder->overrideKey(newName);
	if (!m.insert(pFolder).second)
		ASSERT0;
	return pFolder;
}

FolderPtr ProjectInfo_t::AssureSubItem(const MyString &name, bool caseSensitive, FolderPtr pParent) const//create if not exists
{
	FoldersMap &m(pParent->fileFolder()->children());
	if (!caseSensitive)
	{
		FoldersMap::iterator i(m.find_ncase(name));
		if (i != m.end())
			return &(*i);
	}
	else
	{
		FoldersMap::iterator i(m.find(name));
		if (i != m.end())
			return &(*i);
	}

	FolderPtr pNew(memMgrGlob().NewFile(name));
	if (!m.insert(pNew).second)
		ASSERT0;
	pNew->setParent(pParent);
	return pNew;
}

bool ProjectInfo_t::RenameSubItem(Folder_t &rSelf, const MyString &name, bool bFolder)
{
	Folder_t &rParent(*rSelf.Parent());

	if (!bFolder)
		if (name.endsWith("/"))
			bFolder = true;

	assert(!name.endsWith("/"));

	FoldersMap &m(rParent.fileFolder()->children());
	if (m.find(name) != m.end())
	//if (rSelf.name() == name)
		return false;//exsists

	FoldersMap::iterator i(m.find(rSelf.name()));
	if (i == m.end())
		return false;

	FolderPtr pOld(m.take(i));
	assert(pOld == &rSelf);
	if (!m.insert(pOld).second)
		ASSERT0;

/*	Folder_t *pAfter(nullptr);
	for (FileTree_t::ChildrenIterator i(&rParent); i; i++)
	{
		Folder_t *pFolder(i.data());
		if (pFolder == &rSelf)
			continue;
		Folder_t &file(pFolder);
		if (file.name() == name)
			return false;//already
		if (!mrProject.files().IsFolderLessThan(*pFolder, name, bFolder))
			break;
		pAfter = pFolder;
	}
	TreeTakeChild(&rParent, &rSelf);
	rSelf.data().setName(name);
	if (pAfter)
		TreeInsertChildAfter(&rParent, &rSelf, pAfter);
	else
		TreePushChildFront(&rParent, &rSelf);*/

	return true;
}


FolderPtr ProjectInfo_t::FindFileByName(const char *path)
{
	MyString s(path);
	size_t n1(s.rfind(MODULE_SEP));
	assert(n1 != MyString::npos && n1 > 0);
	int n2(s.findRev('.'));
	if (n2 >= 0 && n2 > (int)n1)
	{
		if (s.mid(n2) == SOURCE_EXT || s.mid(n2) == HEADER_EXT)
			s.truncate(n2);
	}
	return mrProject.files().FindFileByStem(s);
}

/*Folder_t * ProjectInfo_t::FindFileById(int fileId)
{
	for (FilesMgr0_t::FolderIterator i(mrProject.files()); i; i++)
	{
		FolderPtr pFolder(*i);
		if (pFolder->fileFolder())
			continue;
		if (pFolder->ID() == fileId)
			return pFolder;
	}
	return nullptr;
}*/

/*MyString ProjectInfo_t::FindFileNameById(int fileId)
{
	Folder_t *pFile(FindFileById(fileId));
	if (!pFile)
		return "";
	return pFile->m.name();
}*/

static MyString makeUniqueName(Folder_t &rParent, bool bFolder)
{
	MyString sNameDef(NAMEDEFAULT_DIR);
	if (!bFolder)
		sNameDef = NAMEDEFAULT_FILE;

	int num(0);
	const FoldersMap &m(rParent.fileFolder()->children());
	for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
	{
		CFolderRef f(*i);
		if (f.name().startsWith(sNameDef, false))
		{
			int n(atoi(f.name().mid((unsigned)sNameDef.length()).c_str()));
			if (n != 0)
				num = n;
		}
	}
	char buf[256];
	sprintf(buf, "%s%d", sNameDef.c_str(), ++num);
	return MyString(buf);
}


FolderPtr ProjectInfo_t::AddFile(MyString sPath) const//full qualified name must be provided
{
	AssureRootFolder();

CHECK(sPath.endsWith("src/nputf"))
STOP

	ZPath_t l(sPath);
	assert(l.front().endsWith(MODULE_SEP));
	//MyStringList l(MyStringList::split("/", sPath));
	if (!l.isAbsolute())
		return nullptr;
	
	int iCase(-1);
	MyString sModule(l.take_front());
	FolderPtr pFolder0(FindModuleFolderEx(sModule, true));
	if (pFolder0)
	{
		//extract case sensitivity info
		TypePtr pModule(pFolder0->fileModule()->module());
		if (pModule)
		{
			ModuleInfo_t MI(*this, *pModule);
			iCase = MI.IsArchCaseSensitive();
		}
	}
	else
	{
		const FilesMgr0_t& files(mrProject.files());
		pFolder0 = AssureSubItem(sModule, true, files.rootFolder());
	}

	//find binary index
//	l.pop_front();
	//size_t uBinaryIndex();
//	if (!BinaryIndexFromPath(sBin))
	//	return nullptr;

	FolderPtr pFolder(pFolder0);
	while (!l.empty())
	{
		MyString s(l.take_front());
		assert(!s.endsWith("!"));
		
		//FILEID_t uFolder(FILEID_NULL);

		bool bFolder(s.endsWith("/"));

		if (s.startsWith("?"))
		{
			s.erase(0, 1);
			s.prepend(makeUniqueName(*pFolder, bFolder));
		}

		pFolder = AssureSubItem(s, iCase, pFolder);
		if (bFolder && !pFolder->hasPvt())
			pFolder->SetPvt(new FileFolder_t);

		assert(pFolder);
	}

	//gui().GuiOnFileListChanged();
	mrProject.markDirty(DIRTY_FILES);
	return pFolder;
}


