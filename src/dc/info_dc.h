#pragma once

#include "dc.h"
#include "db/info_module.h"

class Stub_t;
class FuncArgsMap;
class TypesTracer_t;


class DcInfo_s : public ModuleInfo_s
{
public:
	static Dc_t *DcFromFolder(const Folder_t &);
	static Dc_t *DcFromType(TypePtr);
	static Dc_t *DcFromModule(TypePtr);
	static FileDef_t *FILEDEF(const Folder_t *pFolder){ return pFolder->hasPvt() ? pFolder->pvt().fileDef() : nullptr; }
	static FileDef_t *FILEDEF(const Folder_t &rFolder){ return rFolder.hasPvt() ? rFolder.pvt().fileDef() : nullptr; }
	//static Dc_t *DcOf(CFieldPtr);
	static TypePtr OwnerProc(CGlobPtr);//funcdef
	static TypePtr OwnerProc(CFieldPtr);
	static FieldPtr DockField(CGlobPtr);//funcdef
	static PNameRef DockName(CGlobPtr);
	static ADDR DockAddr(CGlobPtr);

	static TypePtr ModuleOfEx(CTypeBasePtr);//for globs and args
	static TypePtr OwnerScope(CGlobPtr);
	static TypePtr OwnerScope(CFieldPtr);
	static TypePtr OwnerScope(CTypePtr);
	static void SetOwnerScope(FieldPtr, TypePtr);
	static bool IsStaticMemberFunction(CGlobPtr);
	//static size_t CheckTemplatedPrefix(const MyString &s);//, bool bFunc);
	static bool HasMethods(CTypePtr p);
	static bool IsEmptyStruc(CTypePtr p);
	static GlobPtr FuncDefAttached(CFieldPtr);
	static FileDef_t *AssureFileDef(FolderPtr, bool bMemMgr = true);
	static void AssureMemMgr(FolderPtr);
	static FileDef_t *SetFileDef(FolderPtr, FileDef_t *, bool bMemMgr, bool bInclude);
	static FolderPtr FolderOf(CGlobPtr);
	static FolderPtr FolderOf(CTypePtr);
	static FileDef_t &FindFileDefOf(CGlobPtr);
	static FileDef_t *FindFileDefOfPtr(CGlobPtr);
	static FileDef_t *TakeTypeIfNotContainedIn(TypePtr, const FileDef_t *);
	static FolderPtr TakeTypeFromFile2(TypePtr);
	//static FileDef_t *TakeFieldIfNotContainedIn(userdataglobal_t &, const FileDef_t *);
	static ExpFieldInfo_t	VA2ExpFieldInfo(const Folder_t &, ADDR);
	static GlobPtr GlobObj(CFieldPtr);//assert global
	static GlobPtr GlobFuncObj(CFieldPtr);
	static FieldExPtr AsFieldEx(CFieldPtr);//assert global
	static GlobPtr GlobObjNA(CFieldPtr);//no assert
	//static FieldPtr FindCloneOf(CFieldPtr, bool bFuncStub);//pick a clone field
	static TypePtr ModuleRefEx(CTypePtr);
	static bool CheckOwnership(TypePtr);
	static GlobPtr IsPtrToCFunc(CFieldPtr);
	static TypePtr IsPtrToFuncType(CFieldPtr);
	static GlobPtr IsCFuncOrStub(CFieldPtr);//may be stub
	static bool IsCFunc(CFieldPtr);//not a stub
	static FolderPtr OwnerFolder(CTypePtr);
	static bool IsLocalSSID(CFieldPtr);
	static bool IsGlobal(CFieldPtr);//locals aware
	static bool IsMember(CFieldPtr p);
	static size_t ScopePos(const MyString &, size_t from = MyString::npos, int level = 0);
	static FullName_t SplitScopedName(const MyString&);
	static MyString	NameScopeChopped(MyString, MyString &, bool bChop);
	static MyString EnhancedName(const MyString&, MyString scope);//make appropriate for SRC context
	static void BeautifyName(MyString&);
	//static void	DetachFuncdef(FieldPtr);
	static void CollectVPtrsOffsets(CTypePtr, ADDR, std::vector<ADDR> &);
	static bool IsFieldInherited(CFieldPtr);
	static bool IsStaticMember(CGlobPtr);
	static GlobPtr GetLocalOwner(CFieldPtr);
	//static bool WriteFuncProfile(MyStreamBase &, IAnalyzer *);//from anal
	static size_t s_ChopTemplatedPrefix(MyString&);
	static size_t s_CheckTemplatedPrefix(const MyString&);
	static unsigned	checkOverlap(CFieldPtr);
	static FolderPtr FolderOfEx(CObjPtr);
	static FieldMap& OwnerMap(CFieldPtr);
};

/////////////////////////////////////////////////////DcInfo_t
class DcInfo_t : public ModuleInfo_t, public DcInfo_s
{
protected:
	Dc_t &mrDC;
	const I_FrontDC &mrFrontDC;
	const FE_t &mrFE;

public:
	DcInfo_t(const DcInfo_t &);
	DcInfo_t(const DcInfo_t &, MemoryMgr_t &);
	DcInfo_t(Dc_t &);
	DcInfo_t(const Dc_t &);
	DcInfo_t(Dc_t &, MemoryMgr_t &);

	const I_FrontDC& FrontDC() const { return mrFrontDC; }
	const FE_t& FE() const { return mrFE; }

	//const Mainx_t &mainx() const { return reinterpret_cast<const Mainx_t &>(mrMain); }
	ProjectEx_t &projx() const { return reinterpret_cast<ProjectEx_t &>(mrProject); }
	const IGuiEx_t &guix() const { return projx().guix(); }

	void InitDc();
	Dc_t &dc() const { return mrDC; }
	Dc_t &DcRef() const { return mrDC; }
	Dc_t *DcPtr() const { return &mrDC; }
	FolderPtr ModuleFolder() const { return mrDC.moduleFolder(); }
	ADDR64 ImageBase() const
	{
		CTypePtr pSeg(PrimeSeg());
		return pSeg->typeSeg()->imageBase(pSeg);
	}

	MemoryMgrEx_t &memMgrGlobEx() const { return reinterpret_cast<MemoryMgrEx_t &>(dc().memMgr()); }
	MemoryMgr2_t &memMgrGlob2() const { return reinterpret_cast<MemoryMgr2_t &>(dc().memMgr()); }
	OpType_t PtrSize() const;

	//void createStubsMgr();
	bool	ToFuncProfile(const char *, FuncProfile_t &) const;
	TypePtr FuncTypeFromProfile(const FuncProfile_t &) const;
	//void InitFuncProfile(FuncProfile_t &) const;

	//MyString RegMaskToString(RegMaskType) const;
	bool RegMaskToGPRs(RegMaskType, GPRs_t &) const;
	bool RegMaskToArgList(RegMaskType, Arg1List_t &) const;
	bool ArgListToRegMask(const Arg1List_t &, RegMaskType &) const;

	bool FlagMaskToArgList(FlagMaskType, Arg1List_t &) const;
	bool ArgListToFlagsMask(const Arg1List_t &, FlagMaskType &) const;

	bool RegStringToArgList(MyString, Arg1List_t &) const;//comma-separated
	bool RegStringToGPRs(MyString, GPRs_t &, OPC_t) const;//comma-separated

	bool CpuRegs2ArgList(const GPRs_t &, Arg1List_t &) const;
	void ArgList2CpuRegs(const Arg1List_t &, bool bThisCall, GPRs_t &) const;
	MyString ArgListToString(const Arg1List_t &, const char *sep) const;
	MyString GPRsToString(SSID_t, const GPRs_t &, const char *sep) const;

	bool GetTempArgs(const FuncProfile_t &, Arg1List_t &) const;
	void GetTempRets(const FuncProfile_t &, Arg1List_t &) const;

	MyString	LocalRegToString(CFieldPtr) const;
	static MyString	LocalToString(int ofs, unsigned sz);
	MyString	RegToString(SSID_t, REG_t, bool force = false) const;
	MyString	FlagsToStr(OPC_t, unsigned flags, bool) const;
	RegMaskType ToRegMask(const r_t *) const;

	//TypePtr FindCachedType(const MyString &fullName, bool bOwn);
	//void RebuildTypesMap(ExpTypesMap &);
	FieldPtr FindGlobal(ADDR) const;//exact,top
	FieldPtr FindGlobalAtVA(ADDR, FieldIt_Mode, bool bDeep) const;

	TypePtr FindStruc(FileDef_t &, const FullName_t &);
	TypePtr	FindStrucFast(MyString);
	ObjPtr FindObjByAutoNameEx(const MyString &, CTypePtr iScope) const;
	ObjPtr FindObjByScopedNameEx(const MyString &, CTypePtr scopeFrom) const;

	MyString FieldDisplayNameEx(CFieldPtr, CFieldPtr pImpField = nullptr) const;//will check for locals, DOES NOT CHOP!
	MyString TypeDisplayNameEx(CTypePtr);
	bool LabelNameEx(CFieldPtr, MyString &) const;
	bool TmpLabelNameEx(CFieldPtr, MyString &s) const;
	//MyString FieldInbredName(FieldPtr) const;
	TypePtr OwnerSegEx(CTypePtr) const;

	//import/export
	FieldPtr ToExportedField(CFieldPtr) const;
	//FieldPtr ToExportedField2(CTypePtr mod, unsigned short ord, const char *nam) const;
	//void RebuildImportInfoMap() const;
//	FileTempl_t *CheckTemplatesMappings(bool bForce) const;//from binary

	//intrinsics/typedefs
	GlobPtr AddIntrinsic(const char *name, const char *stub);
	int CreateIntrinsics();
	TypePtr AddTypedef(TypePtr, const char *, TypePtr owner = nullptr);
	void AssureFuncDef(GlobPtr) const;
	void AssureThunk(GlobPtr) const;
	GlobPtr NewFuncDef();
	bool CheckThunkAt(CFieldPtr) const;

	FieldPtr FromExportedField(CFieldPtr pExtField) const;//returns my field
	static MyString MakePrettyName(MyString, TypePtr, int& forceMode);
	bool	ApplyPrettyName(GlobPtr, MyString, TypePtr scope, int forceMode = 0) const;
	bool	ApplyPrettyName(TypePtr, MyString, TypePtr scope, int forceMode = 0) const;
	//MyString TypeNameFull0(TypePtr) const;//ugly names first!
	enum TypeNameFullMode {	E_PRETTY_NULL, E_PRETTY_NAME = 1, E_PRETTY_SCOPE = 2, E_PRETTY = 3 };
	FullName_t TypeNameFull(CTypeBasePtr, TypeNameFullMode prettyMode = E_PRETTY_NULL, char chopSymb = '\0') const;//chopMode: 0:no chopping(as is); 1:chop; 2:preserve stubs with '#'
	FullName_t TypePrettyNameFull(CTypePtr, char chopSymb) const;
		void _TypePrettyScope(CTypePtr p, char chopSymb, FullName_t &) const;

	//ugly name => assigned name => autoname; no name chopping;
	MyString TypePrettyName(CTypePtr) const;
	MyString TypePrettyName2(CTypePtr) const;//check module
	MyString TypePrettyName(CTypePtr, char chopSymb/* = '\0'*/) const;
	MyString GlobPrettyName0(CGlobPtr, char chopSymb/* = '\0'*/) const;
	bool FieldCompName0(CFieldPtr, const MyString& mangled, const MyString& extracted, node_t*) const;//this way is faster
	FieldPtr MatchExistingField(FieldPtr, const MyString& mangled, const MyString& extracted, node_t*);
	FieldPtr AssureDockField(FieldPtr, const MyString&);

	FullName_t GlobNameFull(CGlobPtr, TypeNameFullMode prettyMode, char chopSymb = '\0') const;
	
//	TypePtr setTypeEx(TypeVFTable_t &, TypePtr) const;

	bool AssureTypeClass(TypePtr, bool bOrNamespace = false) const;
	bool AssureTypeNamespace(TypePtr, bool verbose = true) const;
	bool AssureTypeStruc(TypePtr) const;//revert from a class
	
	//bool DelocateFieldName(FieldPtr);//re-locate the name to UglyFieldNames map
	//bool DelocateTypeName(TypePtr);

	//files
	FolderPtr AddFileEx(const char *pszName, FTYP_Enum fileType = FTYP_SOURCE) const;
	bool IsDefinitionFile(CFolderPtr) const;//no cxx counterpart
	bool IsDeclarationFile(CFolderPtr) const;
	//bool UpdateExpFieldInfo(FieldPtr ) const;
	FolderPtr AddSpecialFile(FTYP_Enum) const;
	int	 AddTypeToFile(TypePtr, FolderPtr, unsigned = FileDef_t::AT_TAIL) const;
	//GlobPtr	AddGlobToFile0(FieldPtr, FolderPtr) const;
	FieldExPtr AddGlobToFile2(FieldPtr, FolderPtr) const;
	GlobPtr AcquireFunction(FieldPtr, FolderPtr, FTYP_Enum = FTYP_SOURCE) const;
	void DestroyField(FieldPtr) const;
	bool MoveGlobToFile(GlobPtr, FolderPtr) const;
	FolderPtr FindFileByStem(MyString) const;

	FolderPtr TemplatesFolder() const;
	bool RegisterPrettyTypeName(PNameRef, TypePtr) const;
	bool RegisterPrettyFieldName(PNameRef, GlobPtr) const;
	PNameRef FindPrettyName(CGlobPtr) const;
	bool GetDisplayName(CFieldPtr, MyString &) const;//check cfunc
	PNameRef FindPrettyName(CTypePtr) const;
	bool GetPrettyName(CTypePtr, MyString &, char choSymb);
	bool GetPrettyName(CGlobPtr, MyString &, char choSymb);
	PNameRef TypePureName(CTypePtr) const;//pretty name first, then - a regular one
	FieldPtr FindTemplatedField(const char *) const;
	//TypePtr FindTemplatedType(const char *) const;
	PNameRef TakePrettyName(TypePtr) const;
	PNameRef TakePrettyName(GlobPtr) const;
	//bool RegainTemplatedType(TypePtr);//from one module to another
	TypePtr MakeProxyTypeTo(TypePtr) const;
	bool RenameProxyType(TypePtr, const MyString&) const;
	bool RenameField(FieldPtr, MyString) const;
	
#undef DeleteFile
	int		DeleteFile(FolderPtr);
	int		ReleaseFile(Folder_t *);
	int		ExcludeFile(FileDef_t &, FolderPtr);
	int		DeleteFileByName(const MyString &);
	void	redump(const ProbeEx_t &, RedumpFlags) const;
	void	redump(TypeBasePtr) const;
	void	redump(FolderPtr = nullptr) const;//everything

	//types
	TypePtr GetStockType(OpType_t, TypesMgr_t ** = nullptr) const;
	TypePtr GetStockTypeEx(OpType_t, TypesMgr_t ** = nullptr) const;//array of bytes if invalid optype
	TypePtr EnumOf(TypePtr, OpType_t);
	TypePtr PtrOf(TypePtr);
	TypePtr RefOf(TypePtr);
	TypePtr RvalRefOf(TypePtr);
	TypePtr ArrayOf(TypePtr, unsigned) const;
	TypePtr VFTableOf(TypePtr = nullptr);
	TypePtr VBTableOf(TypePtr = nullptr);
	TypePtr LVFTableOf(TypePtr = nullptr);
	TypePtr CVFTableOf(TypePtr = nullptr);
	TypePtr PrimeSeg() const { return mrDC.primeSeg(); }
	bool IsConst(CFieldPtr) const;
	bool IsThruConst(CFieldPtr) const;
	bool IsStackPtr(CFieldPtr) const;

	TypeObj_t* findOpType(OpType_t) const;

	void ClearFieldMap(FieldMap&) const;
	bool MakeThisCallFromArg(FieldPtr pArg) const;
	
	TypePtr	UnmakeMemberMethod(GlobPtr) const;
	bool MakeMemberThisCallFromArg(FieldPtr pArg) const;//make a non-static function member
		
	FieldPtr FindFieldFromCPUinChar(GlobPtr, const REG_t&) const;
	FieldPtr AssureThisPtrTo(GlobPtr, TypePtr, bool bConst) const;

	//misc
	Decompiler_t *CreateDecompiler(GlobPtr) const;
	MyString VA2STR2(ADDR) const;

	void RedumpStubs();
	//int ProcessExportedSymbolsEx();
	//int ProcessImportedSymbolsEx();

	void AddExportedFieldInfo(ExpFieldInfo_t) const;
	ExpFieldInfo_t RemoveExportedFieldInfo(PNameRef) const;
	bool AddInheritance(TypePtr, FieldPtr = nullptr, int = 1);//always public
	bool RemoveInheritance(TypePtr);
	bool ToggleVFTablePointer(TypePtr);
	FieldPtr AssureFieldAt(Locus_t &, const MyString &mangled, node_t *);
	MyString SymbolName(CFieldPtr, bool bCreate = true) const;
	node_t* FromSymbol(MyString, bool bCreate = true) const;
	int CollectExportedSymbols() const;
	int CollectImportedSymbols() const;
	int CollectDebugSymbols() const;
	TypePtr FindProxyOf(CTypePtr) const;
	GlobPtr NewFuncDef(FieldPtr, FolderPtr);
	MyString GlobPrettyName(CGlobPtr);
	
	void CalculateDependencies(FolderPtr);
	ADDR	CheckUnderlap(FieldMapCIt) const;
	//ADDR	CheckOverlap(FieldMapCIt) const;
	//void ValidateGlobalName(FieldPtr);
	TypePtr NameOwnerOf(FieldPtr) const;
	TypePtr PrettyNameOwnerOf(FieldPtr) const;
	void	SetPtrToStruc(FieldPtr, TypePtr iClass) const;
	void	SetConstPtrToStruc(FieldPtr, TypePtr iClass) const;
	void	SetConstPtrToConstStruc(FieldPtr, TypePtr iClass) const;

	int		RetAddrSize() const;
	void WriteGlobInfo(CTypeBasePtr, MyStreamBase &) const;
	static GlobPtr CheckGlob(CObjPtr);
	MyString extractVName(const MyString&) const;
	TypePtr ReconcileTypes(TypePtr pType1, TypePtr pType2);
	void WriteFuncProfile(MyStreamBase &, CFolderPtr, const StubBase_t &);
	void WriteFuncProfile(const FuncProfile_t &, MyStreamBase &);
	bool LocusFromVAEx(ADDR, Locus_t &) const;
	void PostMakeStub(MyString key, MyString value, CTypePtr module, I_Context *);
	const STORAGE_t&	Storage(SSID_t) const;

	void DumpBlocks(CGlobPtr, MyStreamBase &) const;
	void DumpExpr(MyStreamBase &, unsigned, CGlobPtr, OpPtr) const;
	int	DumpExprPtr(MyStreamBase &) const;
	bool CheckProblem(CFieldPtr, MyString&, bool bIsDeclaration) const;
	bool MakeTypeVPtr(FieldPtr);
	bool ToggleThisPtr(FieldPtr);
	FieldPtr	GetExportedField(const MyStringEx&) const;
	FieldPtr	TakeBogusField(const MyStringEx&);
	FieldPtr	ExtraditeUnresolvedExternal(const MyStringEx&, TypePtr);
	void CleanupUnclaimedExports();
	FieldPtr CheckThruConst(CFieldPtr, OFF_t &) const;
};





