#pragma once

#include "info_proj.h"
#include "type_module.h"

class ModuleInfo_s : public ProjectInfo_s
{
public:
	static FieldPtr FindFieldInSubsegs(CTypePtr seg, ADDR, Locus_t&);
	static FieldPtr FindFieldInSubsegs(CTypePtr seg, ADDR, unsigned* segOff = nullptr);
	static FieldPtr GetFieldFromValue(const VALUE_t&, CTypePtr seg);
	static bool GetVAfromValue(const VALUE_t&, CTypePtr seg, ADDR&);
};

class ModuleInfo_t : public ProjectInfo_t, public ModuleInfo_s
{
protected:
	TypeObj_t &mrModuleRef;
	Module_t &mrModule;
public:
	ModuleInfo_t(const ModuleInfo_t &o)
		: ProjectInfo_t(o),
		mrModuleRef(o.mrModuleRef),
		mrModule(o.mrModule)
	{
	}

	ModuleInfo_t(const ProjectInfo_t &rPI, const TypeObj_t &rBinaryRef)
		: ProjectInfo_t(rPI),
		mrModuleRef((TypeObj_t &)rBinaryRef),
		mrModule(*rBinaryRef.typeModule())
	{
	}

	ModuleInfo_t(const Project_t &rTypeRef, const TypeObj_t &rBinaryRef)
		: ProjectInfo_t(rTypeRef, rTypeRef.memMgr()),
		mrModuleRef((TypeObj_t &)rBinaryRef),
		mrModule(*rBinaryRef.typeModule())
	{
	}

	ModuleInfo_t(const ModuleInfo_t &rMI, MemoryMgr_t &rMemMgr)
		: ProjectInfo_t(rMI.Project(), rMemMgr),
		mrModuleRef(rMI.mrModuleRef),
		mrModule(*mrModuleRef.typeModule())
	{
	}

	const Module_t& ModuleRef() const { return mrModule; }
	TypePtr ModulePtr() const { return &mrModuleRef; }
	MyString ModulePath() const;
	const MyString &ModuleName() const { return ModuleTitle(ModulePtr()); }
	void SetModuleTitle(const MyString &);
	void FireDelayedFormat();
	TypePtr FindFrontSeg() const;
	int IsArchCaseSensitive() const;

	FolderPtr FolderOfKind(FTYP_Enum) const;//FTYP_Enum
	void RegisterTypesMapEx(TypePtr, bool) const;
	FolderPtr AssureSpecialFile(FTYP_Enum) const;
	FolderPtr AssureFolderOfKind(FTYP_Enum) const;
	FolderPtr AssureNamesFile() const;
	FolderPtr AssureExportsFile() const;
	FolderPtr AssureImportsFile() const;
	NamesMgr_t *AssureNamespace(TypePtr = nullptr) const;
	TypesMgr_t *AssureTypeMgr(TypePtr = nullptr);//struc
	TypePtr GetStockType(OpType_t, TypesMgr_t ** = nullptr) const;

	TypePtr MakeSegment(TypePtr iSelf, ADDR addrP, unsigned sizeP, const char *name, unsigned uFlags);
	TypePtr AddSubRange(TypePtr iSegRange, ADDR addrV, ADDR_RANGE sizeV, TypePtr iSeg);
	bool AddSubRange0(TypePtr iRangeSet, ADDR base, unsigned size, TypePtr iSeg);

	PNameRef AddName(NamesMgr_t &, const char *, ObjPtr, bool bShare = false) const;
	PNameRef AddPrettyName(NamesMgr_t &, const char *, TypeBasePtr) const;
	PNameRef ForceName(NamesMgr_t &, const char *, ObjPtr, int forceMode = 0) const;//forceMode: 0:no force; 1:append special symbol; 2:append a number
	PNameRef ForcePrettyName(NamesMgr_t &, const char *, TypeBasePtr, int forceMode = 0) const;
	bool SetTypeName(NamesMgr_t &, TypePtr, const char *pName) const;
	bool SetFieldName(NamesMgr_t &, FieldPtr, const char *pName, bool bForce = false) const;
	bool SetFieldName(FieldPtr, const char *name, bool bForce = false) const;
	bool SetTypeName(TypePtr, const char *name, bool bForce = false) const;
	bool SetFieldName2(FieldPtr, const MyStringEx&, bool bForce = false) const;
	bool SetHardName2(FieldPtr, MyString) const;
	bool CheckUglyName(FieldPtr, MyString demangled) const;
	//provides possibility to override the assigned name in binary dumping
	MyString FieldDisplayName(FieldPtr) const;
	//MyString TypeDisplayName(CTypePtr) const;
	
	bool DeleteField(Locus_t &) const;//struc
	bool DeleteField(TypePtr, ADDR) const;//struc

	//FieldPtr insertStrucvarField(ADDR, TypePtr, Name_t *, TypePtr);
	PNameRef addNamedObj(TypePtr, Obj_t *, const char *pName, NamesMgr_t ** = nullptr, int iForceNAme = 0) const;

	bool DumpSegResources(MyStreamBase &) const;
	FieldPtr AddSecondaryField(TypePtr, ADDR, FieldPtr) const;
	bool CheckResources();
	//FieldPtr AddStrucvarField(TypePtr iScope, const char *name, TypePtr iType, AttrIdEnum attr);
	size_t FetchStringAt(OFF_t, MyString &) const;
	bool IsNull(OFF_t) const;
	bool GetRawOffset(CFieldPtr, OFF_t &) const;
	bool VA2FP(TypePtr, ADDR a, OFF_t &) const;
	//void UpdateViewGeometry() const;
	void UpdateViewGeometry2() const;
	//ROWID updateViewGeometry(TypePtr, ROWID) const;
	ROWID updateViewGeometry2(TypePtr, ROWID, OFF_t, size_t) const;
	void SetDataSource(DataPtr) const;
	PDATA GetAuxData(OFF_t &size) const;
	DataPtr GetDataSource() const;
	const I_DataSourceBase &GetDataSourceRef() const;
	DataPtr GetDataHost() const;
	MyString FieldNameDemagled(CFieldPtr) const;
	//bool AddRangeSeg(TypePtr iRangeSet, ADDR base, unsigned size, TypePtr iSeg);
	TypePtr NewRangeSet(ADDR64 base64, const char *name);
	void DumpSegments(CTypePtr iSeg, std::ostream &, int);
	void DumpSections(std::ostream &);

	TypePtr findType(CTypePtr, const char* name, TypesMgr_t **ppTypesMgr = nullptr) const;
	ObjPtr FindObjByAutoName(const MyString &, CTypePtr scope) const;//if not an autoname - return nullptr
	ObjPtr FindObjByName(const MyString &, CTypePtr scope) const;//no auto names
	FieldPtr FindGlobalByName(const char *) const;
	TypePtr FindTypeByName(const MyString &) const;//scoping is OK
	TypePtr FindTypeByName(const MyString &, CTypePtr scope) const;//scoping is OK
	FieldPtr FindFieldInSeg(CTypePtr, ADDR, Locus_t &) const;//terminal
	static TypePtr	FindSegAt(CTypePtr iSelf, ADDR, unsigned affinity = 0);
	TypePtr FindSegFromRA(OFF_t, CTypePtr from);
	TypePtr FindEntryPoint(ADDR &) const;

	TypePtr VA2Seg(CTypePtr, ADDR) const;//seg
	bool Str2DA(MyString s0, ROWID *pr, CTypePtr startSeg = nullptr);
	bool R2D(CTypePtr, ADDR addr, ROWID &rowID);
	TypePtr R2D2(CTypePtr, ADDR addr, ROWID &rowID);
	TypePtr SegTraceOffsetToVA(CTypePtr trace, unsigned offs, ADDR &va);
	bool IsReadOnlyVA(CTypePtr, ADDR) const;
	MyString FolderPath(CFolderPtr) const;//module not included
	MyString FormatterAtLocus(Probe_t&);
	MyStringEx Adjusted(const MyStringEx&) const;
	//bool LocusFromStr(MyString, CTypePtr pStartSeg, Locus_t&);
};

///////////////////////////////////////////////////// ScopeStack_t
class ScopeStack_t : public std::list<frame2_t>
{
	ModuleInfo_t &mrPJ;
public:
	ScopeStack_t(ModuleInfo_t &r) : mrPJ(r){}
	frame_t &push_scope(TypePtr, ADDR, bool);//going one level down always in a current scope
	frame_t &jump_scope(TypePtr, ADDR, bool);//jump to address in some other scope in the hierarchy
	ADDR pop_scope(bool bForceNoAdvance = false);
	bool isTopShared() const;
	bool isTopStrucvar() const;
	bool isTopSeg() const {
		if (!back().contBase())
			return false;
		return back().cont()->typeSeg() != nullptr;
	}
};

class StockTracer_t : public ProjectInfo_t
{
	TypesMgr_t &mrTypesMgr;
public:
	StockTracer_t(Project_t &r, TypesMgr_t &rtm)
		: ProjectInfo_t(r),
		mrTypesMgr(rtm)
	{
	}
	void AddStockTypes();
private:
	TypePtr addStockType(OpType_t);
};


