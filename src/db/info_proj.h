#pragma once

#include "qx/MyPath.h"
#include "mem.h"
#include "types.h"
#include "field.h"
#include "data.h"
#include "proj.h"

class TypesMgr_t;
class Project_t;

//class VarArray_t;
struct Block_t;
class Locus_t;
class Probe_t;
class BinaryAnalyzer_t;
class MyFileName;
class FRONT_t;
class I_Front;
class Frame_t;
class Main_t;
class IGui_t;
class ZPath_t;
class FilesMgr0_t;

typedef	NameRef_t*	PNameRef;
//#define STRUC_AS_MULTIMAP

enum RESULT_e { RESULT_ALREADY = -1, RESULT_FAILED, RESULT_OK };

struct DA2_t : public DA_t
{
	FolderPtr pFolder;//top
	DA2_t() : pFolder(nullptr){}
};

struct FieldMapItEx
{
	TypePtr		owner;
	FieldMapIt	it;
	FieldMapItEx(TypePtr o, FieldMapIt i) : owner(o), it(i){}
};

struct BinaryContext_t
{
	PDATA	pData;
	ADDR	va;//address in segment (for alignments)
	unsigned	range;
	ROWID		da;
	BinaryContext_t() : pData(nullptr), va(0), range(0), da(0){}
	BinaryContext_t(PDATA p, ADDR v, unsigned r, ROWID d) : pData(p), va(v), range(r), da(d){}
	void advance(unsigned delta)
	{
		if (pData)
			pData += delta;
		va += delta;
		if (range >= delta)
			range -= delta;
		else
			range = 0;
		da += delta;
	}
};

class ProjectInfo_s
{
public:
	static bool gTraceObjLifetime;
public:
	static TypePtr OwnerSeg0(CTypePtr);//for non-shared types only
	static TypePtr OwnerSeg(CTypePtr);//types can be shared
	static TypePtr FrontSeg(CTypePtr);
	static unsigned SegOffset(CFieldPtr);
	static TypePtr ModuleOf(CTypeBasePtr);//trace to the root
	static TypePtr ModuleOf(CFolderPtr);//as a folder root 
	static TypePtr ModuleOf(CFieldPtr);//go up to the root
	static FolderPtr TopFolder(const Folder_t&);//as a folder root 
	static bool IsTopFolder(const Folder_t&);
	static bool IsPhantomFolder(const Folder_t&);
	//static NamesMgr_t& NamespaceInitialized(TypePtr);
	static NamesMgr_t* OwnerNamesMgr(CFieldPtr, TypePtr* ppOwner);
	static NamesMgr_t* OwnerNamesMgr(CTypePtr, TypePtr* ppOwner);//no initialization
	static MyString VA2STR(CTypePtr, ADDR rva, int w = 0);
	static MyString VA2STR(ADDR va, bool bLarge, ADDR64 imageBase, int w = 0);
	static MyString VA2STR(CFieldPtr);
	static MyString VA2STR(ADDR);
	static bool IsInside(CTypePtr iSelf, ADDR va);
	static bool CheckDataAtVA(CTypePtr seg, ADDR va);
	static FieldPtr	__findFieldByNameInSegs(CTypePtr seg, const char*);//search in all segments, starting from 'seg'
	static FieldPtr	FindFieldByNameInSegs2(CTypePtr seg, const char*);//search in all segments, starting from 'seg', chopping of the duplicate mark
	static FieldPtr	FindFieldByNameInSegs3(CTypePtr comp, const char*);//the same as FindFieldByNameInSegs2, but considers auto-names
	static FieldPtr	FindFieldByName(const char*, CTypePtr cont);//search in scope of 'comp' only
	static FieldPtr	FindFieldByAddrInSegs(CTypePtr seg, ADDR);//search in all segments, starting from 'seg'
	static TypesMgr_t* findTypeMgr(CTypePtr);
	static TypePtr FindFrontSegIn(CTypeBasePtr);
	static TypePtr FindFrontSegUp(CTypePtr);
	enum FieldIt_Mode { FieldIt_Prev, FieldIt_Overlap, FieldIt_Exact };
	static FieldPtr __findFieldV(CTypePtr, ADDR addr0, FieldIt_Mode, bool bDeep);
	static FieldMapIt StrucFieldIt(CTypePtr, ADDR addr, FieldMapIt* ppNext = nullptr);
	static FieldMapIt FieldIt(CTypePtr, ADDR addr, FieldMapIt* ppNext = nullptr, FieldIt_Mode mode = FieldIt_Prev);
	static FieldPtr  Field(CTypePtr, ADDR addr, FieldPtr* ppNext = nullptr, FieldIt_Mode mode = FieldIt_Prev);
	//0: field may not overlap address
	//1: field may not sit at address but must overlap it
	//2: field must sit at address
	static TypePtr IsBitvar(CTypePtr);//returns a strucvar
	static ROWID Size(CTypeBasePtr, ROWID* = nullptr);
	static ROWID Size(CFieldPtr, ROWID * = nullptr);
	static ROWID ViewSize(CTypeBasePtr, ROWID* = nullptr);
	static ROWID ViewOffset(CTypeBasePtr);
	static ROWID SegOverSize0(CTypePtr);
	static ROWID SegOverSize(CTypePtr);
	static size_t SegTraceSize(CTypeBasePtr);//size in parent
	static void AcquireTypeRef(TypePtr);
	static unsigned BitSize(TypePtr);
	static TypesMgr_t* NewTypesMgr(TypePtr);
	static void DeleteTypesMgr(TypePtr);
	static unsigned ChopName(const MyString&, MyString&, char chopSymb = 0);
	static TypePtr OwnerSegList(CTypePtr);//superlink
	static TypePtr OwnerSegRange(CTypePtr);
	static bool IsRangeSeg(CTypePtr);
	static TypePtr SkipProxy(CTypePtr);
	//static bool IsUglyType(TypePtr);
	//static TypePtr IsUglyLocum(TypePtr);
	//static TypePtr SkipUglyLocum(TypePtr);//ugly typedef as well
	static bool terminalFieldAt0(Locus_t&, unsigned bitOffs = 0);//TypePtr , DA_t &, Locus_t &, Block_t) const;
	static void terminalFieldAt(Locus_t&, unsigned bitOffs = 0);
	static TypePtr TypeTop(TypePtr);
	static bool RegisterSubseg(TypePtr, ADDR, TypePtr);//seg
	static ROWID VA2DA(CTypePtr, ADDR);
	static FieldPtr EntryField(CTypePtr);//struc, at 0 offset	//EntryLabel
	static bool IsEntryLabel(CFieldPtr);
	static bool IsPhantomModule(CTypeBasePtr);//binary
	static TypePtr SkipArray(CTypePtr);
	static bool IsImpOrExp(CFieldPtr);
	static bool IsTypeImp(CFieldPtr);
	static bool IsExported(CFieldPtr);
	static TypePtr SkipModifier(CTypePtr);
	static MyString	fixFileName(MyString, CTypePtr iModule);
	static void	AssurePathAbsolute(ZPath_t&, CTypePtr iModule);
	static FieldPtr CloneLead(CFieldPtr);//given a cloned one, returns a prime
	static bool CheckStrucEnd(FieldMapIt, FieldMap&);
	static bool CheckStrucEnd(FieldMapCIt, const FieldMap&);
	static FieldMapIt EosAwarePrior(FieldMap&, FieldMapIt);
	static FieldMapIt EosAwareNext(FieldMap&, FieldMapIt);
	static FieldMapCIt EosAwareNext(const FieldMap&, FieldMapCIt);
	static unsigned FuncSizeLimited(TypePtr);
	static const MyString& ModuleTitle(CTypePtr);
	static const MyString& ModuleTitle(CFolderPtr);//module
	static NamesMgr_t* AssureNamespace0(TypePtr);
	static TypePtr CommonScope(CTypePtr, CTypePtr);
	static bool IsProc(CTypeBasePtr);
	static TypePtr IsConstPtrToConstStruc(CTypePtr, bool bSkipModifier);
	static TypePtr IsConstPtrToStruc(CTypePtr, bool bSkipModifier);
	static TypePtr IsPtrToConstStruc(CTypePtr p, bool bSkipModifier);
	static TypePtr IsPtrToStruc(CTypePtr, bool bSkipModifier);
	static TypePtr SkipBitset(CTypePtr);
	static FieldPtr	FindFieldByName(MyString, const Locus_t &);
	static bool IsEosField(CFieldPtr);
	static bool IsEosField(CFieldPtr, const FieldMap&);
	static void SetEosField(FieldPtr);
	static FieldPtr EosField(CTypePtr);
	static FieldPtr UFieldPrev(CFieldPtr);
	static FieldPtr UFieldNext(CFieldPtr);
};

class ProjectInfo_t : public ProjectInfo_s
{
protected:
	//TypeObj_t &mrProjRef;
	Main_t &mrMain;
	Project_t &mrProject;
	MemoryMgr_t &mrMemMgr;
public:
	ProjectInfo_t(const ProjectInfo_t &);
	//ProjectInfo_t(Project_t &);
	ProjectInfo_t(const Project_t &);
	//ProjectInfo_t(Project_t &, MemoryMgr_t &);
	ProjectInfo_t(const ProjectInfo_t &, MemoryMgr_t &);
	MemoryMgr_t& memMgr() const { return mrMemMgr; }
	MemoryMgr_t& memMgrGlob() const;
	bool IsMemMgrGlob() const;
	TypePtr CreateModule(int unique) const;
	Main_t &main() const { return mrMain; }
	std::ostream& PrintError() const;
	std::ostream& PrintWarning() const;
	std::ostream& PrintInfo() const;

	bool OfTheSameModule(CFolderPtr, CFolderPtr) const;
	Project_t &Project() const { return mrProject; }
	Project_t *ProjPtr() const { return &mrProject; }
	I_Front *IFrontOf(TypePtr iFrontSeg) const;
	FRONT_t *FrontOf(TypePtr iFrontSeg) const;
	OpType_t PtrSizeOf(TypesMgr_t &) const;
	OpType_t PtrSizeOf(TypePtr) const;
	FilesMgr0_t &Files() const;

	//names mapping
	void DeleteNamespace(TypePtr);
	bool RemoveNameRef(NamesMgr_t &, PNameRef) const;
	int ClearFieldName(FieldPtr, NamesMgr_t * = nullptr) const;
	int ClearTypeName(NamesMgr_t &, TypePtr) const;
	NamesMgr_t &OwnerNamespaceEx(TypePtr, TypePtr *ppOwner = nullptr) const;//with initialiaztion
	//TypePtr NameOwnerOf(FieldPtr) const;

	TypesMgr_t *superTypesMgr(TypesMgr_t *) const;
	TypePtr VA2Locus(CTypePtr iContext, ADDR va, Locus_t &, bool bWrapUp = true) const;
	
	//find by name
	FieldPtr	findFieldByName(CTypePtr, const char *);
	TypePtr		findContByName(CTypePtr, const char *, ROWID &);
	FieldPtr	findFieldByAddr(CTypePtr, ADDR, unsigned);//find by address
	TypePtr findCodeSeg(CTypePtr) const;
	FolderPtr FindModuleFolder(MyString, bool bCaseSensitive, CFolderPtr from = nullptr) const;
	FolderPtr FindModuleFolderEx(const MyString&, bool bAllowMiscase) const;
	FolderPtr FindModuleFolderByTag(int) const;
	FolderPtr FindModuleFolderByUnique(int) const;
	TypePtr FindModule(const MyString&, bool bCaseSensitive, CFolderPtr from = nullptr) const;
	TypePtr FindModuleEx(const MyString&, bool bAllowMiscase) const;
	TypePtr findEntryPoint(TypePtr, ADDR &) const;
	Folder_t* FindFileByName(const char *);
	TypePtr FindAtticTypeByName(const MyString &) const;//scoping is OK
	TypePtr FindFrontSegById(TypePtr iModule, int frontId);
	int NextModuleUnique() const;

	bool IsTerminalSeg(TypePtr) const;
	bool IsTypeBit(TypePtr) const;
	static bool CheckTypeInclusion(CTypePtr pType1, CTypePtr pType2);
	TypePtr SetType(FieldPtr, TypePtr) const;
	TypePtr SetType(TypeImp_t &, TypePtr, bool bNoRef = false) const;
	TypePtr SetType(TypeExp_t &, TypePtr, bool bNoRef = false) const;
	TypePtr SetType(TypePtr_t &, TypePtr, bool bNoRef = false) const;
	TypePtr SetType(Array_t &, TypePtr, bool bNoRef = false) const;
	TypePtr SetType(Typedef_t &, TypePtr) const;
	TypePtr SetType(TypeProxy_t &, TypePtr) const;
	TypePtr SetType(TypeThunk_t &, TypePtr) const;
	TypePtr SetType(TypeEnum_t &, TypePtr, bool bNoRef = false) const;
	TypePtr SetType(TypeArrayIndex_t &, TypePtr, TypePtr, bool bNoRef = false) const;
	TypePtr SetType(TypeConst_t &, TypePtr, bool bNoRef = false) const;
	TypePtr SetType(TypePair_t &, TypePtr left, TypePtr right, bool bNoRef = false) const;
	TypePtr SetType(TypeFunc_t &, TypePtr ret, TypePtr args, unsigned flags, bool bNoRef = false) const;
	bool SetTypeEx(FieldPtr, TypePtr) const;
	void ClearType(FieldPtr) const;

	TypePtr TakeTypeOf(FieldPtr) const;//do not release reference
	void RegisterTypesMap(TypePtr, bool);
	BinaryAnalyzer_t *StartBinaryAnalizer(TypePtr, unsigned affinity) const;
	BinaryAnalyzer_t* FindBinaryAnalizer() const;

	////////////////////////////////////// Names
	MyString ObjName(CObjPtr) const;
	MyString FieldName0(CFieldPtr, char chopSymb = '\0') const;//non-virtual
	MyString FieldName(CFieldPtr, char chopSymb = '\0') const;
	bool LabelName(CFieldPtr, MyString &) const;
	bool TmpLabelName(CFieldPtr, MyString &) const;
	MyString AutoName(CFieldPtr, CFieldPtr pImpField) const;
	MyString autoname(CFieldPtr, CFieldPtr, const char *pfx = nullptr) const;
	MyString FieldDisplayName0(FieldPtr) const;

	MyString TypeName0(CTypePtr, char chopSymb = 0) const;//non-virtual
	MyString TypeName(CTypePtr, char chopSymb = 0) const;
	MyString TypeDisplayName(CTypePtr) const;//trim enum's name
	MyString StrucNameless(CTypeBasePtr) const;//force autoname

	int PrintTypesList(const TypesMgr_t &, std::ostream&, int);

	//int makeData0(const Probe_t &, TypePtr, AttrIdEnum, bool);
	
	bool terminalFieldAtSeg(CTypeBasePtr, DA_t &, Locus_t &, Block_t) const;
	FolderPtr LocusFromDA(ROWID, Locus_t &);
	bool LocusFromVA(CTypePtr, ADDR, Locus_t &, bool bStripToSeg = true) const;//DOWN
	bool LocusFromVA_1(CTypePtr, ADDR, Locus_t &) const;//UP
	bool LocusFromVA_2(CTypePtr, ADDR, Locus_t &) const;//UP
	bool LocusFromVA(CFieldPtr, Locus_t &) const;//UP
	bool LocusFromStr(MyString, Locus_t&, CTypePtr startSeg = nullptr);
	MyString AtStr(Cmd_t&, bool bRemove = false);
	FieldPtr adjustPick(Locus_t &) const;

	FolderPtr ModuleFromDA(ROWID);
	FolderPtr ModuleFromPath(MyString) const;
	FolderPtr FolderOfModule(TypePtr) const;

	int PrintListing(TypePtr, unsigned uColFlags = 0, bool bCompact = false);
	Debugger_t * newDebugger() const;
	bool startDebugger();
	bool destroyDebugger();
	OFF_t rawOffs(CTypePtr) const;//seg
	OFF_t rawSize(CTypePtr) const;//seg
	//void UpdateViewGeometry();
	void SegJustCreated(TypePtr) const;
	ADDR baseOf(TypePtr) const;
	bool SetRootDirectory(const MyDirPath &);
	Folder_t*	AddFileEx(Folder_t &, const char *name, int type);
	
	int		DeleteStrucObj(TypePtr, ADDR addr);
	unsigned rangeFrom(TypePtr, ADDR);

	//size_t BinaryIndexFromFileOffs(size_t) const;
	FolderPtr AssureSubItem(const MyString &, bool cc, FolderPtr parent) const;//create if not exists
	bool RenameSubItem(Folder_t &, const MyString &, bool bFolder);
	FolderPtr RenameFolder(Folder_t &, const MyString &) const;
	void AssureRootFolder() const;
	FolderPtr AddFile(MyString) const;
	FolderPtr LoadBinary(const MyPath &) const;
	int RecoilNoOutDir(const MyString &) const;
	FolderPtr CreateModuleFolder(MyString) const;

	bool SetStrucSize(TypePtr, unsigned, bool bForce = false) const;
	bool SetFuncSize(TypePtr, unsigned) const;

	//StrucModifier_t
	bool InsertFieldAt(TypePtr, FieldPtr, ADDR key) const;
	bool InsertField(Locus_t &, unsigned checkSize = 0) const;
	FieldMapIt InsertFieldIt(Frame_t &, unsigned size = 0) const;
	FieldMapIt InsertUniqueFieldIt(TypePtr, ADDR, FieldPtr) const;
	FieldMapIt InsertFieldIt(TypePtr, FieldPtr) const;
	FieldPtr AddField(TypePtr, FieldPtr , ADDR nOffset) const;//ready
	FieldPtr AddField(TypePtr, ADDR nOffset, int nMode = 0, bool bThis = false) const;
	FieldPtr AppendUField(Locus_t&) const;
	//FieldPtr appendStrucvarField(TypePtr);
	FieldPtr AppendBField(TypePtr) const;

	bool CheckRelocEligability(FieldPtr pField, TypePtr iDst) const;
	FieldPtr TakeEosField(TypePtr, unsigned &) const;

	FRONT_t *FrontAt(size_t) const;
	FRONT_t *FrontEnd(CTypePtr) const;
	void ReleaseFrontEnd(size_t) const;
	unsigned NewFrontEnd(const char *name, int id);
	bool DeleteFolder(FolderPtr) const;
	void TakeField0(TypePtr iSelf, FieldPtr) const;//in struct
	bool CheckNested(TypePtr, TypePtr) const;
	FolderPtr AddModule(const MyPath &, bool cc, Probe_t &, MyString opts) const;
	bool DaToLocus(DA_t, Locus_t &, CTypeBasePtr module/* = 0*/) const;
	bool DaFromAttrAtLocus(const Locus_t &, AttrIdEnum, const value_t &, DA2_t &) const;
	void SendProgress(const ProgressMonitor_t<size_t> &) const;
	void SendProgress2(const ProgressMonitor_t<ROWID> &) const;
	const IGui_t &gui() const;
	MyString FolderPathAbs(CFolderPtr) const;//module is included
	MyString FolderPathRel(CFolderPtr, CFolderPtr from) const;//relative to 'from'
};

#define TRACE_OBJ_LIFETIME 0

#if(TRACE_OBJ_LIFETIME)
#define ON_OBJ_CREATED(pObj)	if (ProjectInfo_t::gTraceObjLifetime) PrintObjInfo(std::cout, pObj);
#else
#define ON_OBJ_CREATED(pObj)
#endif

typedef uint32_t	ProblemInfo_t;


////////////////////////////////////////////////////frame2_t
struct frame2_t : public frame_t
{
	frame2_t(TypePtr _p, ADDR _o = 0, bool b = false) : frame_t(_p, _o), uExtent(0), bAdvance(b){}
	//frame2_t() : bAdvance(false){}
	unsigned	uExtent;
	bool	bAdvance;//on Leave()
};





