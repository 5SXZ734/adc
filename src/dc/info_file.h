#pragma once

#include "info_dc.h"

class FileInfo_s : public DcInfo_s
{
public:
	static MemoryMgr_t* memMgrOfBody(CGlobPtr);
	//static void SetDcOwnerSegPtr(TypePtr, TypePtr);
	static GlobPtr IsLocalsTop(TypePtr);//union
	static GlobPtr ContextFuncDef(CFieldPtr);
};

//////////////////////////////////////////////////FileInfo_t
class FileInfo_t : public DcInfo_t, public FileInfo_s
{
protected:
	FileDef_t &mrFileDef;

public:
	FileInfo_t(const FileInfo_t &r)
		: DcInfo_t(r),
		mrFileDef(r.mrFileDef)
	{
	}
	FileInfo_t(const FileInfo_t &, MemoryMgr_t &);
	FileInfo_t(const DcInfo_t &, FileDef_t &);
	FileInfo_t(const DcInfo_t &, FileDef_t &, MemoryMgr_t &);
	//FileInfo_t(const DcInfo_t &, FileDef_t &);

	//file  info
	FileDef_t &FileDef() const { return mrFileDef; }
	MemoryMgrEx_t &memMgrEx() const {
		return reinterpret_cast<MemoryMgrEx_t &>(mrMemMgr);
	}
	HPATH NewPath() const;
	HOP NewOp() const;
	HOP NewTmpOp() const;
	HOP NewPrimeOp() const;
	HOP NewPrimeTmpOp() const;
	Ins_t* NewRootInfo() const;
	XOpLink_t *NewXOpLink(HOP) const;
	XOpLink_t *NewXOpLink2(HOP) const;//doubled
	void Delete(HPATH) const;
	void Delete(HOP) const;
	void DeleteRootInfo(HOP) const;
	bool TakeType(TypePtr) const;
	FieldPtr RecallGlob(GlobPtr) const;
	FieldPtr RecallGlobIt(GlobMapIt) const;
	TypePtr TakeTypeFromFile(TypePtr) const;
	//bool UndoFuncdef(FieldPtr, TypePtr pClass);
	//bool UndoStatic(FieldPtr);
	void ClearFile(bool bClosing) const;
	void UnloadFuncdefs() const;
	void LoadFuncdefs() const;
	bool DeleteFunc(TypePtr) const;
	void DeleteFuncDef(GlobPtr) const;
	enum E_KIND { E_KIND_STRUC, E_KIND_CLASS, E_KIND_ENUM/*, E_KIND_UNION*/ };
	TypePtr MakeStruct(MyString name, TypePtr ownerClass, int iNesting, int iForceName = 0, E_KIND bClass = E_KIND_STRUC) const;//filemode: 0:tail, -1: head, 1:before lst
	bool AcquireProxyType(TypePtr) const;
#if(!NO_TYPE_PROXIES)
	TypePtr MakeProxyOf(const char *, TypePtr, TypePtr proxy);
#endif
	FuncDef_t*	typeFuncDefIntrinsic(TypePtr) const;
	bool AddImpField(FieldPtr) const;
	bool OfThisModule(TypePtr) const;
	
	void	CheckInclusion() const;
	FolderPtr OwnerFolder() const { return mrFileDef.ownerFolder(); }
	Folder_t &OwnerFolderRef() const { return *mrFileDef.ownerFolder(); }

	void setSyncOp(ProbeExIn_t &) const;
	
	size_t findIntrinsic(const char *name) const;

	////////////////////////////////////files
	//void addObjectToFile(Obj_t *, bool bHead = true);

	int		CheckString(CFieldPtr, bool bUser = false) const;
	bool	Overlap(FieldPtr, FieldPtr) const;

	MemoryMgr_t *memMgrOf(CFieldPtr) const;
	MemoryMgr_t *memMgrOf(CTypePtr) const;
	//NamesMgr_t *namespaceOf(FieldPtr pField, MemoryMgr_t **) const;
	
	bool	DumpToFile(SrcDumpFile &, adcui::UDispFlags dispflags) const;
	bool	DumpToStream(std::ostream &, adcui::UDispFlags dispflags) const;
	void	ProgressInfo(const char *str, OpPtr pOp) const;
	
	//command handlers
	FieldPtr	CreateExportedField(const MyStringEx&, MyString demangled, bool bIsFunc) const;
	//TypePtr	RelocateExTypeInfo(const char *pcNameFull, TypePtr, TypePtr proxy);
	bool CanRelocateType(TypePtr) const;
	TypePtr RelocateType(TypePtr, const FullName_t&, bool bCreateProxy = true) const;
	TypePtr CreateProxyTo(TypePtr, const FullName_t&) const;

	void	AddTypeObj(TypePtr, unsigned = FileDef_t::AT_TAIL) const;
	//void	AddIntrObj(GlobPtr);
	MyString DumpFieldDeclaration(CFieldPtr) const;
	TypePtr AddTypedefEx(TypePtr, const char *, TypePtr owner = nullptr);//put in file
	FolderPtr CheckTypeInstantiation(const Field_t &) const;
	bool DeleteFieldGlobal(Locus_t &, TypePtr context) const;

	int FromFuncProfileEx(GlobPtr, const FuncProfile_t&) const;
	TypePtr	MakeNonStaticMember(GlobPtr, FieldPtr) const;

	void RelocateFuncInnards(GlobPtr, FileDef_t&);

	void SetupLocus(ProbeExIn_t&);//from probe
};




