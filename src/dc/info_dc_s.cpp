#include "info_dc.h"

#include "prefix.h"

#include <fstream>
#include "qx/MyFileMgr.h"

#include "clean_ex.h"
#include "type_funcdef.h"
#include "info_class.h"
#include "db/symbol_map.h"


bool DcInfo_s::CheckOwnership(TypePtr p)//check if p is declared in module to which it belongs
{
	FolderPtr pFolder(USERFOLDER(p));
	if (!pFolder)
		return true;//doesn't matter
	TypePtr iModule1(ModuleOf(p));
	TypePtr iModule2(ModuleOf(pFolder));
	return iModule1 == iModule2;
}

size_t DcInfo_s::s_CheckTemplatedPrefix(const MyString &s)
{
	if (s.isEmpty())
		return 0;
	size_t i(0);
	if (s[i] == '`')
	{
#if(0)
		int level(1);
		for (++i; i < s.length(); i++)//first char is already checked
		{
			if (s[i] == '\'')
			{
				if (--level == 0)
				{
					i++;
					break;
				}
			}
			else if (s[i] == '`')
				level++;
		}
#else
		i = s.length();
#endif
	}
	else
	{
//		if (bFunc && s.front() == '~')//destructor is OK
//			i++;
		if (i == s.length() || !(isalpha(s[i]) || s[i] == '_'))//first letter is alpha or underscore
			return 0;
		for (++i; i < s.length(); i++)//first char is already checked
		{
			if (!(isalnum(s[i]) || s[i] == '_'))
				break;
		}
	}
	return i;
}

void DcInfo_s::AssureMemMgr(FolderPtr pFolder)
{
	FileDef_t *pFileDef(pFolder->fileDef());
	assert(pFileDef);
	if (!pFileDef->ownsMemory())
	{
		pFileDef->AttachMemMgr();
		//pFileDef = FILEDEF(pFolder);
#ifdef _DEBUG
		pFileDef->memMgr().mName = pFolder->name();
#endif
	}
}

FileDef_t * DcInfo_s::SetFileDef(FolderPtr pFolder, FileDef_t *pFileDef, bool bMemMgr, bool bInclude)
{
	if (!pFileDef)
		pFileDef = new FileDef_t;
	pFileDef->setFolder(pFolder);
	assert(!FILEDEF(pFolder));
	pFolder->ClearPvt();
	pFolder->SetPvt(pFileDef);
	if (bMemMgr)
		AssureMemMgr(pFolder);
	if (bInclude)
	{
/*		if (!mrDC.isFolderOfKind(*pFolder, FTYP_PREFIX) && mrDC.folderPtr(FTYP_PREFIX))
			pFileDef->addIncludeList(mrDC.folderPtr(FTYP_PREFIX));
		pFileDef->addIncludeList(pFolder);//include itself
*/	}
	return pFileDef;
}

FileDef_t *DcInfo_s::AssureFileDef(FolderPtr pFolder, bool bFileDef)//returns trus if has just assigned
{
	if (pFolder->hasPvt())
	{
		if (pFolder->fileDef())
			return pFolder->fileDef();
		assert(pFolder->fileId() == FILEID_NULL);
		pFolder->ClearPvt();
	}
	return SetFileDef(pFolder, nullptr, bFileDef, bFileDef);
}

FolderPtr DcInfo_s::TakeTypeFromFile2(TypePtr pType)
{
	Folder_t *pFolder(USERFOLDER(pType));
	if (pFolder)
	{
		FileDef_t *pFileDef(FILEDEF(pFolder));
		if (pFileDef)//dc file?
		{
			if (pFileDef->takeType(pType) == nullptr)
				ASSERT0;
			SET_USERFOLDER(pType, nullptr);
			return pFolder;
		}
	}
	return nullptr;
}

FileDef_t *DcInfo_s::TakeTypeIfNotContainedIn(TypePtr pType, const FileDef_t *pTargetFileDef)
{
	FolderPtr pFolder(USERFOLDER(pType));
	if (pFolder)
	{
		FileDef_t *pFileDef(pFolder->fileDef());
		if (pFileDef)
		{
			if (pTargetFileDef && pFileDef != pTargetFileDef)
			{
				if (pFileDef->takeType(pType) == nullptr)
					ASSERT0;
				SET_USERFOLDER(pType, nullptr);
			}
			return pFileDef;
		}
	}
	return nullptr;
}


GlobPtr DcInfo_s::GlobObj(CFieldPtr p)
{
	assert(IsGlobal(p));
	return (GlobPtr)p->UserData<GlobObj_t>();
}

GlobPtr DcInfo_s::GlobFuncObj(CFieldPtr p)
{
	assert(IsGlobal(p));
	CGlobPtr iGlob(GlobObj(p)); 
	if (!iGlob)
		return nullptr;
	if (iGlob->func())
		return (GlobPtr)iGlob;
	return nullptr;
}

FieldExPtr DcInfo_s::AsFieldEx(CFieldPtr p)
{
	if (p->UserData<GlobObj_t>())
	{
		assert(IsGlobal(p));
		return (FieldExPtr)reinterpret_cast<const FieldEx_t*>(p);
	}
	return nullptr;
}

GlobPtr DcInfo_s::GlobObjNA(CFieldPtr pSelf)
{
	if (!IsGlobal(pSelf))
		return nullptr;
	return (GlobPtr)pSelf->UserData<GlobObj_t>();
}

/*FieldPtr DcInfo_s::FindCloneOf(CFieldPtr pMasterField, bool bFindStub)
{
	assert(pMasterField->isCloneMaster());
	const ConflictFieldMap& m(OwnerSeg(pMasterField->owner())->typeSeg()->conflictFields());
	ADDR va(pMasterField->_key());
	for (ClonedFieldMapCIt i(m.lower_bound(va)); i != m.end() && KEY(i) == va; ++i)
	{
		CFieldPtr pField(VALUE(i));
		GlobPtr g(GlobObj(pField));
		if (!g || !g->func() || g->func()->isStub() || !bFindStub)//pick up unless not a stub and no bFirstStub given
		{
			return (FieldPtr)pField;
		}
	}
	return nullptr;
}*/

bool DcInfo_s::IsLocalSSID(CFieldPtr pSelf)
{
	//return IsLo cal0();
	switch (FuncInfo_s::SSIDx(pSelf))
	{
	case SSID_CPUREG:
	case SSID_FPUREG:
	case SSID_AUXREG:
	case SSID_LOCAL:
		return true;
	}
	return false;
}

/*Dc_t *DcInfo_s::DcOf(CFieldPtr p)
{
	return DCREF(ModuleOf(USERFOLDER(p)));
}*/

// given an address of imported symbol, find a top level folder of a module, exporting the object
ExpFieldInfo_t DcInfo_s::VA2ExpFieldInfo(const Folder_t &rImpFolder, ADDR va)
{
	ExpFieldInfo_t exp;
	assert(TopFolder(rImpFolder));
	Dc_t *pDC(DcInfo_t::DcFromFolder(rImpFolder));
	if (pDC)
	{
		DcInfo_t DI(*pDC);
		I_Front *pIFront(pDC->frontEnd());
		assert(pIFront);
		SymbolInfo sym;
		DumpSymbol_t symDump(sym);
		if (pIFront->getImportInfo(va, &symDump))
			return ExpFieldInfo_t(0, 0);
	}
	return exp;
}

Dc_t *DcInfo_s::DcFromModule(TypePtr iModule)
{
	assert(iModule->typeModule());
	return DCREF(iModule);
}


static size_t skipbackUntil(const MyString &s, size_t i, char lc, char rc)
{
	int t(1);//level
	while (--i != -1)
	{
		char c(s[i]);
		if (c == lc)
		{
			if (--t == 0)
				break;
		}
		else if (c == rc)
		{
			t++;
		}
	}
	return i;
}

static bool checkOperatorBack(const MyString &s, size_t i)
{
	assert(i >= 0 && s[i] == '>');
	if (i > 0 && i-- != -1)
		if (s[i] == '>' //operator '>>'
			|| s[i] == '-')//operator '->'
				i--;

	for (; i != -1; i--)
	{
		if (!isspace(s[i]))
			break;
	}

	static const char s2[] = "rotarepo";//"operator" reversed
	for (size_t j(0); s2[j] != 0 && i != -1; i--, j++)
	{
		if (s[i] != s2[j])
			return false;
	}

	return true;
}

size_t DcInfo_s::ScopePos(const MyString &s, size_t from, int level)//find position of the last scope resolution operator (::)
{
	assert(!s.startsWith(":"));
	if (from == -1)
		from = s.length() - 1;
	//this one is tricky: "CORBA::Object_var::operator CORBA::Object_var::T_ptr"
	//                                      \_here!
	//int t(0);//template level
	for (size_t i(from); i != -1; i--)
	{
		char c(s[i]);

		if (c == '>')
		{
			if (!checkOperatorBack(s, i))
			{
				i = skipbackUntil(s, i, '<', c);
				if (i == MyString::npos)
					break;
			}
		}
		else if (c == '}')
		{
			i = skipbackUntil(s, i, '{', c);
			if (i == MyString::npos)
				break;
		}
		else if (c == ')')
		{
			i = skipbackUntil(s, i, '(', c);
			if (i == MyString::npos)
				break;
		}
		else if (c == '\'')//ms specific?
		{
			i = skipbackUntil(s, i, '`', c);
			if (i == MyString::npos)
				break;
		}
		else if (c == ':')
		{
			if (i-- == 0)
				break;//error
			c = s[i];
			if (c == ':')
			{
				static const std::string cs("operator");
				if (s.mid(unsigned(i + 2), (unsigned)cs.length()) != cs)//handle situation described at the top
				{
					size_t j(ScopePos(s, i - 1, level + 1));
					if (j != MyString::npos)
						i = j;
					else if (level > 0)
						i = MyString::npos;
				}
				return i;
			}
			//if (s.find("operator", int(i + 2)) != -1)
				//return i;
		}
	}
	return MyString::npos;
}


FolderPtr DcInfo_s::FolderOf(CGlobPtr p)
{
	return p->folder();
}

FolderPtr DcInfo_s::FolderOf(CTypePtr p)
{
	return USERFOLDER(p);
}

FileDef_t &DcInfo_s::FindFileDefOf(CGlobPtr iGlob)
{
	return *FILEDEF(FolderOf(iGlob));
}

FileDef_t *DcInfo_s::FindFileDefOfPtr(CGlobPtr iGlob)
{
	return FILEDEF(FolderOf(iGlob));
}

TypePtr DcInfo_s::OwnerProc(CGlobPtr pSelf)
{
	return OwnerProc(DockField(pSelf));
}

TypePtr DcInfo_s::OwnerProc(CFieldPtr pField)
{
	if (!pField)
		return nullptr;
	assert(IsGlobal(pField));
	pField = CloneLead(pField);
	if (!pField)
		return nullptr;
	return pField->isTypeProc();
}

FieldPtr DcInfo_s::DockField(CGlobPtr pSelf)
{
	FieldPtr pField(FieldEx_t::dockField(pSelf));
	if (!pField->hasUserData())//intrinsic
		return nullptr;
	return pField;
}

PNameRef DcInfo_s::DockName(CGlobPtr pSelf)
{
	FieldPtr pField(DockField(pSelf));
	if (!pField)
		return nullptr;
	return pField->name();
}

ADDR DcInfo_s::DockAddr(CGlobPtr pSelf)
{
	return DockField(pSelf)->_key();
}

TypePtr DcInfo_s::ModuleOfEx(CTypeBasePtr pSelf)
{
	CGlobPtr pGlob(pSelf->objGlob());
	if (pGlob)
	{
		FieldPtr pField(FieldEx_t::dockField(pGlob));
		if (pField)
			return ModuleOf(pField->owner());
		assert(0);
	}
	if (pSelf->typeModule())
		return pSelf->objTypeGlob();
	if (pSelf->typeStrucLoc())
		return ModuleOfEx(pSelf->owner());
	if (pSelf->typeEnum())
		return ModuleOf(pSelf->baseType());
	if (pSelf->ownerTypeMgr())
		return ModuleOf(pSelf->ownerTypeMgr()->owner());
	return ModuleOf((CTypePtr)pSelf);
}

bool DcInfo_s::IsGlobal(CFieldPtr pSelf)//locals aware
{
	if (FuncInfo_s::IsLocal(pSelf))
		return false;
	return pSelf->isGlobal();
}

bool DcInfo_s::IsMember(CFieldPtr p)
{
	return !IsGlobal(p) && !FuncInfo_s::IsLocal(p);
}//data

GlobPtr DcInfo_s::IsCFuncOrStub(CFieldPtr pSelf)
{
	if (!pSelf)// || !pSelf->type())
		return nullptr;
	GlobPtr pGlob(GlobObjNA(pSelf));
	if (!pGlob)
		return nullptr;
	if (pGlob->func())
		return  pGlob;
	return nullptr;
	//return GlobFuncObj(pSelf);
}

bool DcInfo_s::IsCFunc(CFieldPtr pSelf)
{
	GlobPtr ifDef(IsCFuncOrStub(pSelf));
	if (!ifDef)
		return false;
	if (ifDef->typeFuncDef()->isStub())
		return false;
	return true;
}

FolderPtr DcInfo_s::OwnerFolder(CTypePtr iType)
{
	FolderPtr pFolder(USERFOLDER(iType));
	if (!pFolder)
	{
		TypePtr iType2(iType->owner());
		while (iType2 && !(iType2->typeSeg() || iType2->typeNamespace()))//check if a top-level type not in any file
		{
			pFolder = USERFOLDER(iType2);
			if (pFolder)
				break;
			iType2 = iType2->owner();
		}
	}
	return pFolder;
}


bool DcInfo_s::IsStaticMember(CGlobPtr iGlob)
{
//CHECKID(pSelf,0x1430)
//STOP
	//assert(pSelf->userDataType() > FUDT_ LOCAL);
	if (iGlob->owner())
		if (!iGlob->owner()->typeNamespace())
			return true;
	return false;
}

bool DcInfo_s::IsStaticMemberFunction(CGlobPtr pSelf)
{
	//assert(pSelf->userDataType() > FUDT_ LOCAL);
	if (!pSelf->owner())
		return false;//not in member list
	if (!pSelf->func())
		return true;//static variable
	if (ProtoInfo_s::IsThisCallType(pSelf))
		return false;//non-static member
//?	if (iGlob->flags() & FDEF_VIRTUAL)
	//?	return false;//?
	return true;
}


TypePtr DcInfo_s::OwnerScope(CTypePtr pSelf)
{
//CHECKID(pSelf, 0x7728)
//STOP
	assert(!pSelf->objGlob());
	return pSelf->owner();
}

TypePtr DcInfo_s::OwnerScope(CGlobPtr pSelf)
{
	return pSelf->owner();
}

TypePtr DcInfo_s::OwnerScope(CFieldPtr pSelf)//for struc fields
{
	assert(!IsGlobal(pSelf) || !GlobObj(pSelf));
	assert(!FuncInfo_s::IsLocal(pSelf));
	return pSelf->owner();
}

void DcInfo_s::SetOwnerScope(FieldPtr pSelf, TypePtr iScope)
{
	assert(pSelf->hasUserData());
	//assert(pSelf->userDataType() == FUDT_GLOBAL);
	GlobPtr iGlob(GlobObj(pSelf));
	//assert(!pud->pOwnerScope);
	iGlob->setOwner(iScope);
}

bool DcInfo_s::HasMethods(CTypePtr p)
{
	if (p->typeClass())
	{
		if (!ClassInfo_t::MethodsOf(p).empty())
			return true;
		if (!ClassInfo_t::VTablesOf(p).empty())
			return true;
	}
	return false;
}

bool DcInfo_s::IsEmptyStruc(CTypePtr p)
{
	Struc_t &rStruc(*p->typeStruc());
	if (!rStruc.fields().empty())
		return false;
	if (rStruc.typeMgr())
		return false;
	return !HasMethods(p);
}


bool DcInfo_s::IsFieldInherited(CFieldPtr pField)
{
	return (pField->flags() & FLD_HIER_PUBLIC) != 0;//FLD_HIER__MASK) != 0;
}

void DcInfo_s::CollectVPtrsOffsets(CTypePtr pSelf, ADDR offCur, std::vector<ADDR> &out)
{
	const FieldMap &m(pSelf->typeStruc()->fields());
	for (FieldMapCIt i = m.begin(), E = m.end(); i != E; ++i)
	{
		CFieldPtr pField(VALUE(i));
		ADDR off(offCur + pField->offset());
		if (!IsFieldInherited(pField))
		{
			//should be a very first field
			if (pField->type() && pField->type()->typeVPtr())
			{
				out.push_back(off);
				//assert(++i == m.end());
				assert(pField->offset() == 0);
			}
			break;
		}
		assert(pField->type());
		CollectVPtrsOffsets(pField->type(), off, out);
	}
}


GlobPtr DcInfo_s::GetLocalOwner(CFieldPtr pSelf)
{
#if(NEW_LOCAL_VARS)
	CTypePtr pOwner(pSelf->owner());
	if (pOwner)
	{
		if (pOwner->typeFuncDef())
			return (GlobPtr)pOwner;
		if (pOwner->typeStrucLoc())
		{
			assert(pOwner->owner() && pOwner->owner()->typeFuncDef());
			return (GlobPtr)pOwner->owner();
		}
	}
	return nullptr;
#else
	TypePtr iType(mpOwner);
	if (!iType)
		return nullptr;//pseudolabel?
	assert(iType->typeStruc());
	FieldPtr pFieldP(iType->parentField());
	if (!pFieldP)
	{
		//no local vars? only args?
	}
	else
	{
		TypePtr iUnion(pFieldP->owner());
		if (iUnion->parentField())
			return nullptr;
		TypeUnion_t *pUnion(iUnion->typeUnion());
		if (!pUnion)
			return nullptr;
		const FieldMap &m(pUnion->fields());
		if (m.empty())
			return nullptr;
		FieldMapCIt i(m.begin());
		CFieldPtr pField(VALUE(i));
		iType = pField->type();
		if (!iType)
			return nullptr;
	}
	if (!iType->typeFuncDef())
		return nullptr;
	return iType;
#endif
}


void DcInfo_s::BeautifyName(MyString &s)
{
	if (s.startsWith("`") && s.endsWith("'"))
	{
		s.remove(0, 1);
		s.chop(1);
	}
	assert(!s.empty());
	for (size_t i(0); i < s.length(); i++)
		if (!(isalnum(s[i]) || s[i] == '_'))
			s[i] = '_';
	if (isdigit(s.front()))
		s.front() = '_';
}

MyString DcInfo_s::NameScopeChopped(MyString s0, MyString &s, bool bChop)//returns scope
{
	MyString scope;
	size_t n(ScopePos(s0));
	if (n != MyString::npos)
	{
		scope = s0.left(int(n));
		s0.remove(0, unsigned(n + 2));
	}
	if (bChop)
		ChopName(s0, s);
	else
		s = s0;
	return scope;
}

size_t DcInfo_s::s_ChopTemplatedPrefix(MyString &s)//chop[ off ugly stuff on the right of s
{
	size_t n(s_CheckTemplatedPrefix(s));//n will be a first ugly symbol
	if (n > 0 && n < s.length())
	{
		if (s[n] == DUB_SEPARATOR)
			return 0;
		s.truncate(unsigned(n));
	}
	return n;
}

/*void DcInfo_s::DetachFuncdef(FieldPtr pField)
{return;
	GlobPtr iGlob(GlobObj(pField));
	//if (iGlob)
		//pField->setUserDataType(FUDT_ GLOBAL);

}*/

MyString DcInfo_s::EnhancedName(const MyString& s0, MyString scope)
{
	MyString s(s0);
	bool bTilda(false);
	if (s.front() == '~')
	{
		s.remove(0, 1);
		bTilda = true;
	}

	std::replace(s.begin(), s.end(), ' ', '_');
	if (s_ChopTemplatedPrefix(s) > 0)//no empty
	{
		int iSpecial(0);//1:constructor, 2:destructor
		if (!scope.isEmpty())
		{
			MyString s2;
			NameScopeChopped(scope, s2, true);//extract a terminal scope
			s_ChopTemplatedPrefix(s2);
			if (s2 == s)
			{
				if (!bTilda)
				{
					s = "constructor";
					iSpecial = 1;//constructor
				}
				else
				{
					s = "destructor";
					iSpecial = 2;//destructor
				}
			}
		}
		if (bTilda && iSpecial == 0)
			return "";//?
		BeautifyName(s);
	}
	return s;
}


GlobPtr DcInfo_s::IsPtrToCFunc(CFieldPtr pSelf)
{
	if (!pSelf->type())
		return nullptr;
	TypePtr_t *pTypePtr(pSelf->type()->typePtr());
	if (!pTypePtr)
		return nullptr;
	TypePtr iTypeBase(pTypePtr->pointee());
	if (!iTypeBase)
		return nullptr;
	if (!iTypeBase->typeFuncDef())
		return nullptr;
	return (GlobPtr)iTypeBase;
}

TypePtr DcInfo_s::IsPtrToFuncType(CFieldPtr pSelf)
{
	if (!pSelf->type())
		return nullptr;
	TypePtr_t *pTypePtr(pSelf->type()->typePtr());
	if (!pTypePtr)
		return nullptr;
	TypePtr iTypeBase(pTypePtr->pointee());
	if (!iTypeBase)
		return nullptr;
	if (!iTypeBase->typeFunc())
		return nullptr;
	return iTypeBase;
}

GlobPtr DcInfo_s::FuncDefAttached(CFieldPtr pField)
{
	return GlobFuncObj(pField);
}


Dc_t *DcInfo_s::DcFromFolder(const Folder_t &rFolder)
{
	FolderPtr pTopFolder(TopFolder(rFolder));
	if (!pTopFolder->fileModule())
		return nullptr;
	return DCREF(pTopFolder->fileModule()->module());
}

Dc_t *DcInfo_s::DcFromType(TypePtr iType)
{
	FolderPtr pFolder(USERFOLDER(iType));
	return DcFromFolder(*pFolder);
}

FolderPtr DcInfo_s::FolderOfEx(CObjPtr pObj)
{
	CFieldPtr pField(pObj->objField());
	if (pField)
	{
		CTypePtr pModule(ProjectInfo_t::ModuleOf(pField));
		assert(pModule);
		//s1 = ProjectInfo_t::ModuleTitle(pModule);

		CGlobPtr pGlob(GlobObj(pField));
		assert(pGlob);

		return FolderOf(pGlob);
	}

	CTypePtr pType(pObj->objType());
	if (pType)
		return FolderOf(pType);

	return nullptr;
}



