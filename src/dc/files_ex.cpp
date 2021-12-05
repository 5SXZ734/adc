#include "files_ex.h"
#include "prefix.h"
#include <fstream>
#include "qx/MyStringList.h"
#include "qx/MyFileMgr.h"
#include "shared/defs.h"
#include "shared/link.h"
#include "shared/misc.h"
#include "db/mem.h"
#include "db/obj.h"
#include "db/field.h"
#include "db/proj.h"
#include "db/type_struc.h"
#include "db/info_types.h"
#include "db/main.h"
#include "db/clean.h"
#include "db/ui_main.h"
#include "info_dc.h"
#include "mem_ex.h"
#include "proj_ex.h"
#include "type_funcdef.h"
#include "sym_parse.h"
#include "make_type.h"
#include "dump_file.h"
#include "cc.h"


FileDef_t::FileDef_t()
	: mpOwnerFolder(nullptr),
	mpMemMgr(nullptr),
	mbDirty(false)
{
}

FileDef_t::~FileDef_t()
{
	//if (mpMemMgr)
		delete mpMemMgr;
	assert(mTypes.empty() && mIncludes.empty() && mGlobs.empty());
}

void FileDef_t::AttachMemMgr()
{
	assert(!mpMemMgr);
	mpMemMgr = new MemoryMgrEx_t();
}

void FileDef_t::releaseDeferredTypes()
{
	for (size_t i(1); i < mDeferredTypes.size(); i++)
	{
		CTypePtr p(mDeferredTypes[i]);
CHECKID(p, 0xe35)
STOP
		if (!const_cast<TypePtr>(p)->releaseRef() && p->isShared())//non-stub funcdefs with no parent unionloc have zero refs count
			ASSERT0;
	}
	mDeferredTypes.clear();
}

void FileDef_t::setDeferredTypes(const std::vector<CTypePtr> &v)
{
	assert(mDeferredTypes.empty());
	mDeferredTypes.reserve(v.size());
	mDeferredTypes.resize(1);
	assert(!v.empty() && !v[0]);//1-biased
	for (size_t i(1); i < v.size(); i++)
	{
		CTypePtr p(v[i]);
		mDeferredTypes.push_back(p);
		const_cast<TypePtr>(p)->addRef();
	}
}

//returns a most appropriate place to insert a new type at (considering the scoping)
TypesListIt FileDef_t::findInsertTopIt(TypePtr pSelf)
{
	if (!pSelf->isNested())
	{
		for (TypesListIt i(mTypes.begin()); i != mTypes.end(); i++)
		{
			TypePtr p(*i);
			assert(p != pSelf);
			if (!p->isNested())
				return i;
		}
		return mTypes.end();//unscoped go after the scoped ones
	}
	
	TypesListIt k(mTypes.end());
	TypePtr pScope0(nullptr);
	for (TypesListIt i(mTypes.begin()); i != mTypes.end(); i++)//few passes are required
	{
		TypePtr p(*i);
		assert(p != pSelf);
		//if (!p->isNested())
			//return i;
		if (p->ownerScope() == pSelf->ownerScope())
			return i;
		TypePtr pScope(ProjectInfo_t::CommonScope(pSelf, p));
		if (pScope)
		{
			if (k == mTypes.end())
			{
				k = i;
				pScope0 = pScope;
				continue;
			}
			if (pScope0 != pScope && pScope0->isScopeOf(pScope))//the new scope is deeper
			{
				 k = i;
				 pScope0 = pScope;
			}
		}
	}
	return k;	
}

TypesListIt FileDef_t::findInserBottomIt(TypePtr pSelf)
{
	if (!pSelf || !pSelf->owner())
		return mTypes.end();

	if (!pSelf->isNested())
		return mTypes.end();//append from bottom
	
	TypesListIt k(mTypes.end());//deepest preffix (scope) found so far
	TypePtr pScope0(nullptr);

	TypesListIt j(mTypes.end());
	while (j != mTypes.begin())
	{
		TypesListIt i(j--);
		TypePtr p(*j);
		assert(p != pSelf);
		if (!p->isNested())
			continue;//skip unscoped
		if (p->ownerScope() == pSelf->ownerScope())
			return i;
		TypePtr pScope(ProjectInfo_t::CommonScope(pSelf, p));
		if (pScope)
		{
			if (k == mTypes.end())
			{
				k = i;
				pScope0 = pScope;
				continue;
			}
			
			if (pScope != pScope0 && pScope0->isScopeOf(pScope))//check if the new scope is deeper (don't fall all the way up the same level)
			{
				k = i;
				pScope0 = pScope;
			}
		}
	}

	return k;	
}

void FileDef_t::printTypes(std::ostream &os)
{
	TypePtr pModule(ModuleInfo_t::ModuleOf(ownerFolder()));
	DcInfo_t DI(*DCREF(pModule));
	for (TypesListIt i(mTypes.begin()); i != mTypes.end(); i++)//few passes are required
	{
		TypePtr p(*i);
		if (p->typeProxy())
			os << "proxy\n";
		else
			os << DI.TypePrettyNameFull(p, CHOP_SYMB) << std::endl;
	}
}

#define DEBUG_TYPE_INS 0

void FileDef_t::addTypeObj(TypePtr p, unsigned at)
{
CHECKID(p, 62819)
STOP

	if (at == AT_TAIL)
	{
		TypesListIt it(findInserBottomIt(p));
		mTypes.insert(it, p);//push_back(p);
	}
	else if (at == AT_HEAD)
	{
		TypesListIt it(findInsertTopIt(p));
		mTypes.insert(it, p);//push_front(p);
	}
	else
	{
		TypePtr pObjLast(nullptr);
		if (!mTypes.empty())
		{
			pObjLast = mTypes.back();
			mTypes.pop_back();
		}
		mTypes.push_back(p);
		if (pObjLast)
			mTypes.push_back(pObjLast);
	}

	assert(!USERFOLDER(p));
	SET_USERFOLDER(p, mpOwnerFolder);

#if(DEBUG_TYPE_INS)
	static int z = 0;
	std::cout << "====(" << z++ << ")===============\n";
	printTypes(std::cout);
#endif
//	assert(ModuleInfo_t::CheckOwnership(p));
}


/*void FileDef_t::addIntrObj(GlobPtr p)
{
//CHECKID(p, 62819)
//STOP

	mIntri nsics.push_back(p);

	assert(!USERFOLDER(p));
	SET_USERFOLDER(p, mpOwnerFolder);

}*/

void FileDef_t::checkForwardDeclaration(TypePtr iSelf)//try to keep the struc in fwd declaration section (top of file) if it is empty, at the bottom otherwise
{
	assert(iSelf);
	Struc_t *pSelf(iSelf->typeStruc());
	if (!pSelf)
		return;

	bool bEmpty(DcInfo_t::IsEmptyStruc(iSelf));

	//the begining of the types section are empty ones

	TypesListCIt i(mTypes.end());//iSelf pos to be located
	TypesListCIt j(mTypes.begin());//fwd decl section upper bound
	for (; j != mTypes.end(); j++)
	{
		TypePtr p(*j);
		if (!p || !p->typeStruc() || !DcInfo_t::IsEmptyStruc(p))
			break;
		if (p == iSelf)
		{
			if (bEmpty)
				return;//needs no relocation
			i = j;//mark as found
		}
	}

	if (i == mTypes.end())
	{
		if (!bEmpty)
			return;//not in fwd decl section, assume it is somewhere below

		//find the struc in the rest of the file
		for (; i != mTypes.end(); i++)
			if (*i == iSelf)
				break;
	}

	//assert(bEmpty);
	assert(i != mTypes.end());

	//take it at old position
	mTypes.erase(i);

	//re-insert at the end of fwd decl section
	mTypes.insert(j, iSelf);
}

TypePtr DcInfo_t::FindStruc(FileDef_t &rSelf, const FullName_t &sName)
{
	for (TypesListCIt i(rSelf.types().begin()); i != rSelf.types().end(); i++)
	{
		TypePtr p(*i);

//CHECKID(p, 60666)
//STOP

		FullName_t s(TypeNameFull(p));//this is slow!
		if (s == sName)
			return p;
	}
	return nullptr;
}

bool FileDef_t::isEmpty() const
{
	return mTypes.empty() && mGlobs.empty() && mIncludes.empty();
}

void FileDef_t::CheckInclusion()
{
	assert(0);
/*	Obj_t *pObj = ObjNx(); 
	while (pObj != this)
	{
		Obj_t *pObjNx = pObj->ObjNx();
		if (pObj->IsStruc())
		{
			Struc_t *pStruc = pObj->objT ype()->typeStruc();
			CheckStrucInclusion(pStruc);//pStruc may be relinked here
		}
		pObj = pObjNx;
	}*/
}

bool FileDef_t::addIncludeList(FolderPtr pFile)
{
	assert(pFile);
	//if (pFile->m.User Data<FileDef_t>() == this)
		//return 0;

	//check if allredy included
	if (!mIncludes.add(pFile))
		return false;

	pFile->m_nRefCount++;
	return true;
}

bool FileDef_t::removeIncludeList(Folder_t *pFile)
{
	assert(pFile);
	//if (pFile->m.Use rData<FileDef_t>() == this)
		//return 0;

	//check if allredy included
	if (!mIncludes.remove(pFile))
		return false;

	pFile->m_nRefCount--;
	return true;
}

/*bool FileDef_t::FindType(TypePtr pType) const
{
	for (TypesListCIt i(types().begin()); i != types().end(); i++)
		if (*i == pType)
			return true;
	return false;
}*/


TypesListIt FileDef_t::findTypeIt(TypePtr pType)
{
	for (TypesListIt i(mTypes.begin()); i != mTypes.end(); i++)
		if (*i == pType)
			return i;
	return mTypes.end();
}

GlobMapIt FileDef_t::findGlobIt(CGlobPtr p) const
{
	ADDR va(DcInfo_s::DockAddr(p));
	for (GlobMapCIt i(mGlobs.lower_bound(va)); i != mGlobs.end() && i->_key() == va; i++)
		if (&(*i) == p)
			return GlobMapIt((GlobPtr)p);
	return const_cast<GlobMap&>(mGlobs).end();
}

GlobMapIt FileDef_t::findGlobSlowIt(CGlobPtr p) const
{
	for (GlobMapCIt i(mGlobs.begin()); i != mGlobs.end(); i++)
		if (&(*i) == p)
			return GlobMapIt((GlobPtr)p);
	return const_cast<GlobMap&>(mGlobs).end();
}

GlobMapIt FileDef_t::firstGlobIt() const
{
	return const_cast<GlobMap&>(mGlobs).begin();
}

bool FileDef_t::findGlob(CGlobPtr p)
{
	if (findGlobIt(p) != mGlobs.end())
		return true;
	return false;
}

void FileDef_t::buildDependencies(std::set<CFolderPtr> &m)
{
	TypePtr pModule(ProjectInfo_t::ModuleOf(mpOwnerFolder));
	assert(pModule);
	FileInfo_t FI(*DCREF(pModule), *this);

	for (TypesListIt i(mTypes.begin()); i != mTypes.end(); i++)
	{
		TypePtr pType(*i);
		if (pType->typeStruc())
		{
			for (Struc_t::HierIterator i(pType); i; i++)
			{
				CFolderPtr pFolder2(FI.CheckTypeInstantiation(*i));
				if (pFolder2)
					m.insert(pFolder2);
			}
		}
	}
	for (GlobMapIt i(mGlobs.begin()); i != mGlobs.end(); i++)
	{
		GlobPtr iGlob(&(*i));
		if (iGlob->func())
		{
			//go thru func's arguments looking for compound instantiations
			for (Struc_t::HierIterator i((GlobToTypePtr)iGlob); i; i++)
			{
				FolderPtr pFolder2(FI.CheckTypeInstantiation(*i));
				if (pFolder2)
					m.insert(pFolder2);
			}
			//the ret(s) must be considered as well
			for (FuncRetsCIt i(*iGlob->func()->typeFuncDef()); !i.isAtEnd(); ++i)
			{
				FolderPtr pFolder2(FI.CheckTypeInstantiation(*VALUE(i)));
				if (pFolder2)
					m.insert(pFolder2);
			}
		}
		else//a variable
		{
			FolderPtr pFolder2(FI.CheckTypeInstantiation(*DcInfo_s::DockField(iGlob)));
			if (pFolder2)
				m.insert(pFolder2);
		}
	}
}

bool FileDef_t::insertGlob(GlobPtr iGlob)
{
	FieldPtr pField(FieldEx_t::dockField(iGlob));
	ADDR at(pField->_key());
CHECKID(pField, 4307)
STOP
	iGlob->overrideKey(at);
	GlobMapIt i(mGlobs.insert(iGlob));
	if (i == mGlobs.end())
		return false;
	iGlob->setFolder(mpOwnerFolder);
//	iGlob->setDockField(pField);
	pField->SetUserData<GlobObj_t>(iGlob);
	//pField->setUserDataType(FUDT_GLOBAL);
	return true;
}

bool FileDef_t::insertGlob(FieldExPtr pFieldx)
{
	ADDR at(pFieldx->_key());
CHECKID(pFieldx, 4307)
STOP
	GlobPtr g(pFieldx->globPtr());
	g->overrideKey(at);//iGlob->dockField()->address());
	GlobMapIt i(mGlobs.insert(g));
	if (i == mGlobs.end())
		return false;
	g->setFolder(mpOwnerFolder);
//	g->setDockField(pFieldx);
	//if (iGlob->func())
		//iGlob->func()->setFuncField(pField);
	pFieldx->SetUserData<GlobObj_t>(g);
	//pFieldx->setUserDataType(FUDT_GLOBAL);
	return true;
}

TypePtr FileDef_t::takeType(TypePtr p)
{
	TypesListIt i;
	if (!p)
	{
		i = mTypes.begin();
		p = *i;
	}
	else
		i = findTypeIt(p);
	if (i == mTypes.end())
		return nullptr;
	mTypes.erase(i);
	//SET_USERFOLDER(p, nullptr);
	return p;
}

/*bool FileDef_t::insertField(FieldPtr pField)
{
	return insertField(userdataglobal_t(pField));
}*/

/*FieldPtr FileDef_t::firstField() const
{
	return (FieldPtr)mGlobs.begin()->dockField();
}*/

GlobPtr FileDef_t::firstGlob() const
{
	return (GlobPtr)&(*mGlobs.begin());
}

bool FileDef_t::hasNonStubs() const
{
	for (GlobMapCIt i(mGlobs.begin()); i != mGlobs.end(); i++)
	{
		CGlobPtr iGlob(&(*i));
		if (iGlob->func() && !iGlob->func()->isStub())
			return true;
	}
	return false;
}

GlobPtr FileDef_t::takeGlobFront()
{
	GlobMapIt i(mGlobs.begin());

//CHECKID(pField, 0x32f)
//STOP
	return takeGlobIt(i);
}

GlobPtr FileDef_t::takeGlobIt(GlobMapIt i)
{
	FieldPtr pField(DcInfo_s::DockField(&(*i)));
	GlobPtr iGlob(DcInfo_t::GlobObj(pField));
	assert(iGlob && iGlob == &(*i));
CHECKID(pField, 0x328)
STOP
	pField->SetUserData<GlobObj_t>(nullptr);
	mGlobs.take(i);
	iGlob->setFolder(nullptr);
	return iGlob;
}

GlobPtr FileDef_t::takeGlob(GlobPtr p)
{
	GlobMapIt it(findGlobIt(p));
	if (it == mGlobs.end())
		return nullptr;
	return takeGlobIt(it);
}

////////////////////////////////////////////////////////////////

/*void FilesMgr_t::ExcludeFile(Folder_t &rFolder)
{
	for (FileTree_t::LeafIterator it(mRootFolder); it; it++)
	{
		Folder_t &rFolder2(*it.data());
		FileDef_t &rFileDef2(*rFolder2.m.User Data<FileDef_t>());
		rFileDef2.removeIncludeList(&rFolder);
	}
}*/


// ->	headers...
//		<externals>
//		<stubs>
//		sources...



Folder_t *DcInfo_t::AddSpecialFile(FTYP_Enum e) const
{
	if (mrDC.moduleRef().special(e))
		return mrDC.moduleRef().special(e);

	MyString sPath(fixFileName(ADC_DIR, mrDC.module()));
	sPath.append(FTYP2name(e));

	FolderPtr pFolder(ProjectInfo_t::AddFile(sPath));
	mrDC.moduleRef().setSpecial(e, pFolder);
	return pFolder;
}

FolderPtr DcInfo_t::TemplatesFolder() const
{
	return mrDC.moduleRef().special(FTYP_TEMPLATES);
}

TypePtr DcInfo_s::ModuleRefEx(CTypePtr iSelf)
{
	TypePtr iModule(ModuleOf(iSelf));
	if (!iModule)
	{
		if (iSelf->typeFuncDef())
			iModule = ModuleOfEx((CGlobPtr)iSelf);
	}
	return iModule;
}

bool DcInfo_t::RegisterPrettyTypeName(PNameRef pn, TypePtr t) const
{
//CHECKID(t, 0xa4c7)
//STOP
	assert(t->hasUglyName());
	TypePtr iModuleP(ModuleRefEx(t));
	assert(iModuleP == ModulePtr());
	FolderPtr pFolder(AssureSpecialFile(FTYP_TEMPLATES));
	if (!pFolder->hasPvt())
		pFolder->SetPvt(new FileTempl_t());
	assert(!t->typeFuncDef());
	if (!pFolder->fileTempl()->addPrettyName(t, pn))
		return false;
	t->flags() |= TYP_PRETTY_NAME;
	return true;
}

bool DcInfo_t::RegisterPrettyFieldName(PNameRef pn, GlobPtr g) const
{
	//FieldPtr f(g->dockField());
	assert(!g->hasPrettyName());
//CHECKID(f, 0x1182a)
//STOP
	FolderPtr pFolder(AssureSpecialFile(FTYP_TEMPLATES));
	if (!pFolder->hasPvt())
		pFolder->SetPvt(new FileTempl_t());
	if (!pFolder->fileTempl()->addPrettyName(g, pn))
		return false;
	//assert(f->name());
	g->setPrettyName();
	return true;
}

PNameRef DcInfo_t::FindPrettyName(CGlobPtr g) const
{
	assert(g->hasPrettyName());
	FolderPtr pFolder(TemplatesFolder());
	assert(pFolder);
	PNameRef pn(pFolder->fileTempl()->findPrettyName(g));
//?	assert(pn);
	return pn;
}

PNameRef DcInfo_t::FindPrettyName(CTypePtr p) const
{
	FolderPtr pFolder(TemplatesFolder());
	if (!pFolder)
		return nullptr;
	return pFolder->fileTempl()->findPrettyName(p);
}

PNameRef DcInfo_t::TakePrettyName(TypePtr t) const
{
//CHECKID(p, 0x2311)
//STOP
	assert(t->hasUglyName());
	FolderPtr pFolder(TemplatesFolder());
	if (!pFolder)
		return nullptr;
	PNameRef pn(pFolder->fileTempl()->takePrettyName(t));
	if (pn)
		t->flags() &= ~TYP_PRETTY_NAME;
	return pn;
}

PNameRef DcInfo_t::TakePrettyName(GlobPtr g) const
{
	FolderPtr pFolder(TemplatesFolder());
	if (!pFolder)
		return nullptr;
	//FieldPtr f((FieldPtr)g->dockField());
	PNameRef pn(pFolder->fileTempl()->takePrettyName(g));
	if (pn)
		g->clearPrettyName();
	return pn;
}

FieldPtr DcInfo_t::FindTemplatedField(const char *n) const
{
	FolderPtr pFolder(TemplatesFolder());
	if (!pFolder)
		return nullptr;
	return FindFieldByName(n, PrimeSeg());
}

/*TypePtr DcInfo_t::FindTemplatedType(const char *n) const
{
	FolderPtr pFolder(TemplatesFolder());
	if (!pFolder)
		return nullptr;
	return pFolder->fileTempl()->types().field(n);
}*/

bool ProjectInfo_t::DeleteFolder(FolderPtr pFolder) const
{
	FoldersMap &m(pFolder->Parent()->fileFolder()->children());
	FoldersMap::iterator j(m.find(pFolder->name()));
	if (j == m.end())
		return false;
	m.take(j);
	memMgrGlob().Delete(pFolder);
	return true;
}

int DcInfo_t::DeleteFile(FolderPtr pFolder)
{
	assert(pFolder);

	FileDef_t *pFile(FILEDEF(pFolder));
	if (pFile && !pFile->IsEmpty())
		return 0;//not empty

	for (FilesMgr0_t::FolderIterator i(mrProject.files()); i; i++)
	{
		CFolderPtr p(&(*i));
		if (p != pFolder)
		{
			FileDef_t *pFile2(p->fileDef());
			if (pFile2 && ExcludeFile(*pFile2, pFolder) == 2)//remove inclusion in other files
				return 1;
		}
	}

/*?	Probe_t *pCtx(mrProject.getContext());
	if (pCtx)
	{
		if (pCtx->folder() == pFolder)
			pCtx->setFolder(nullptr);
		mrProject.releaseContext(pCtx);
	}*/

	if (!DeleteFolder(pFolder))
		return 0;

	mrProject.markDirty(DIRTY_FILES);
	//gui().GuiOnFileListChanged();

	return 1;
}

int DcInfo_t::DeleteFileByName(const MyString &s)
{
	assert(!s.empty());

	FolderPtr pFile(mrProject.files().FindFileByStem(s));
	if (!pFile)
		return 0;
	return DeleteFile(pFile);
}

/*int FilesMgr_t::SelectFileByName(const MyString &s)
{
	assert(!s.empty());

	Folder_t *pFile(FindFileByStem(s));
	if (!pFile)
		return 0;
	SelectFile(pFile);
	return 1;
}*/

int DcInfo_t::ReleaseFile(Folder_t * pFile)
{
	pFile->m_nRefCount--;
	if (pFile->m_nRefCount == 0)
	{
		for (int i(0); i < FTYP__TOTAL; i++)
			if (pFile == mrDC.moduleRef().special(i))
			{
				mrDC.moduleRef().setSpecial(i, nullptr);
				break;
			}

		if (FILEDEF(pFile)->IsEmpty())
		{
			mrProject.markDirty(DIRTY_FILES);
			//gui().GuiOnFileListChanged();
			delete pFile;
			return 2;
		}
	}

	return 1;
}

TypePtr	DcInfo_t::FindStrucFast(MyString fullName)
{
	MyStringList l;
	size_t n;
	while ((n = ScopePos(fullName)) != MyString::npos)//from the back
	{
		MyString s(fullName.substr(n+2));
		l.push_front(s);
		fullName.truncate((unsigned)n);
	}
	l.push_front(fullName);

	TypePtr p(PrimeSeg());
	for (MyStringList::const_iterator i(l.begin()); i != l.end(); i++)
	{
		if (!p->typeComplex())
			return nullptr;
		NamesMgr_t *pNS(p->typeComplex()->namesMgr());
		if (!pNS)
			return nullptr;
		const MyString &s(*i);
		Obj_t *pObj(pNS->findObj(s));
		if (!pObj)
			return nullptr;
		TypePtr p2(pObj->objTypeGlob());
		if (!p2)
			return nullptr;
		assert(p2->owner() == p);
		p = p2;
	}

/*	if (IsUglyType(p))
	{
		FolderPtr iFolderTempl(mrDC.folderPtr(FTYP_TEMPLATES));//ugly types reside only in this file
		if (!iFolderTempl)
			return nullptr;
		FileTempl_t *pFileTempl(CheckTemplatesMappings(false));
		p = pFileTempl->backType(p);
	}*/

	return p;
}

/*void FilesMgr_t::MoveToDefaultHeader(Obj_t * pObj)
{
	assert(0);
	if (!m_pDefHeader)
		m_pDefHeader = AddFileEx(FTYP_HEADER);
	m_pDefHeader->AddTail(pObj);
}*/

adcui::FolderTypeEnum Dc_t::IsViewFile(const Folder_t &r)
{
	if (isFolderOfKind(r, FTYP_RESOURCES))
		return adcui::FOLDERTYPE_FILE_RC;
	if (isFolderOfKind(r, FTYP_TYPES))
		return adcui::FOLDERTYPE_FILE_T;
	if (isFolderOfKind(r, FTYP_NAMES))
		return adcui::FOLDERTYPE_FILE_N;
	if (isFolderOfKind(r, FTYP_EXPORTS))
		return adcui::FOLDERTYPE_FILE_E;
	if (isFolderOfKind(r, FTYP_IMPORTS))
		return adcui::FOLDERTYPE_FILE_I;
	if (isFolderOfKind(r, FTYP_TEMPLATES))
		return adcui::FOLDERTYPE_FILE_TT;
	if (isFolderOfKind(r, FTYP_PROTOTYPES))
		return adcui::FOLDERTYPE_FILE_STUB;
	return adcui::FOLDERTYPE_UNK;
}

unsigned ProjectEx_t::fileType(const Folder_t &rFolder) const
{
	//ProjectInfoEx_t PJ(const_cast<ProjectEx_t &>(*this));
	const FilesMgr0_t &rFiles(files());
	Dc_t *pDC(DcInfo_t::DcFromFolder(rFolder));
	DcInfo_t DI(*pDC);

	const MyString &s(rFolder.name());
	unsigned folderType(adcui::FOLDERTYPE_UNK);
	if (!rFiles.IsFolder(rFolder))//a file?
	{
		folderType = pDC->IsViewFile(rFolder);
		if (folderType == adcui::FOLDERTYPE_UNK)
		{
			FileDef_t *pFileDef(rFolder.fileDef());
			if (pFileDef/* && !pFileDef->objects().empty()*/)
			{
				if (DI.IsDefinitionFile(&rFolder))//header only files
					folderType = adcui::FOLDERTYPE_FILE_CPP;
				else
					folderType = adcui::FOLDERTYPE_FILE_H;
			}
		}
	}
	else if (!rFolder.Parent())
	{
		if (ProjectInfo_t::IsPhantomFolder(rFolder))
			folderType = adcui::FOLDERTYPE_BINARY_PHANTOM;
		else
			folderType = Ext2FolderType(s);
	}
	else
		folderType = adcui::FOLDERTYPE_FOLDER;

	return folderType;
}

TypePtr ProjectEx_t::acquireType(const char *sTypeName, CTypePtr iOwner)//from user's input
{
	TypePtr iModule(ProjectInfo_t::ModuleOf(iOwner));
	if (!iModule)
		return nullptr;

	ModuleInfo_t MI(*this, *iModule);

	if (MyString(sTypeName).startsWith(FENAME_PFX))
	{
		TypesMgr_t *pTypesMgr;
		return MI.findType(iOwner, sTypeName, &pTypesMgr);
	}

	TypePtr iType(nullptr);
	Dc_t *pDC(DCREF(iModule));
	if (pDC)
	{
		DcInfo_t DI(*pDC);
		node_t *node(SymParse(sTypeName));
		Frame_t ctx((TypePtr)iOwner, 0, nullptr);//?
		TypeFomNode0_t conv(DI, ctx);
		iType = conv.process(node);
		delete node;
	}
	if (!iType)
	{
		TypesMgr_t *pTypeMgr(MI.findTypeMgr(iOwner));
		if (pTypeMgr)
		{
			TypesTracer_t TT(MI, *pTypeMgr);
			iType = TT.findTypeByName(sTypeName);
		}
	}
	else if (iType->isEnum())
	{
		TypesMgr_t *pTypeMgr(MI.findTypeMgr(iOwner));
		if (pTypeMgr)
		{
			TypesTracer_t TT(MI, *pTypeMgr);
			iType = TT.enumOf(iType, OPTYP_UINT32);
		}
	}
	return iType;
}

void ProjectEx_t::dumpFile(const Folder_t &rFolder, int level, MyStreamUtil &ssu) const
{
	const Folder_t &file(rFolder);
	MyString s, s2;
	int folderType(0);

	const FilesMgr0_t &rFiles(files());
	Dc_t *pDC(DcInfo_t::DcFromFolder(rFolder));
	DcInfo_t DI(*pDC);

	for (int i(0); i < level; i++)
		s.append("\t");
	s.append(file.name());
	if (!rFiles.IsFolder(rFolder))//a file?
	{
		folderType = pDC->IsViewFile(rFolder);
		if (folderType == adcui::FOLDERTYPE_UNK)
		{
			FileDef_t *pFileDef(rFolder.fileDef());
			if (pFileDef/* && !pFileDef->objects().empty()*/)
			{
				if (DI.IsDefinitionFile(&rFolder))//header only files
				{
					s2 = s;
					s2.append(MyStringf("%s\t%d", SOURCE_EXT, adcui::FOLDERTYPE_FILE_CPP));
				}
				s.append(HEADER_EXT);
				folderType = adcui::FOLDERTYPE_FILE_H;
			}
		}
	}
	else if (level == 0)
	{
		if (ProjectInfo_t::IsPhantomFolder(rFolder))
			folderType = adcui::FOLDERTYPE_BINARY_PHANTOM;
		else
			folderType = Ext2FolderType(s);
	}
	else
		folderType = adcui::FOLDERTYPE_FOLDER;

	if (!s2.empty())
		ssu.WriteString(s2);//src

	if (folderType)
		s.append(MyStringf("\t%d", folderType));
	ssu.WriteString(s);
}

/*Fi le_t*	FilesMemoryMgr_t::NewFile()
{
#if(NEW_MEMMGR)
	F ile_t* p(rFiles.allocate());
	new(p)File _t();
	return p;
#else
	return new Fil e_t;
#endif
}

void FilesMemoryMgr_t::Delete(Fi le_t *p)
{
#if(NEW_MEMMGR)
	if (p)
	{
		p->~Fi le_t();
		rFiles.deallocate(p);
	}
#else
	delete p;
#endif
}

Folder_t* FilesMemoryMgr_t::NewFolder()
{
#if(NEW_MEMMGR)
	Folder_t* p(rFolders.allocate());
	new(p)Folder_t();
	return p;
#else
	return new Folder_t;
#endif
}

void FilesMemoryMgr_t::Delete(Folder_t *p)
{
#if(NEW_MEMMGR)
	if (p)
	{
		p->~Folder_t();
		rFolders.deallocate(p);
	}
#else
	delete p;
#endif
}
*/








