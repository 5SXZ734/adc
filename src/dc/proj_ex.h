#pragma once

#include "db/proj.h"
#include "db/mem.h"
#include "files_ex.h"
//#include "main_ex.h"

class node_t;
struct ProbeEx_t;
template <typename T> class BinaryCleaner_t;
class Mainx_t;
class IGuiEx_t;
class Decompiler_t;

enum DirtyFlagsExEnum
{
	DIRTY_CUR_OP = DIRTY__TOTAL,
	DIRTY_SEL_OBJ,
	DIRTY_CUR_FUNC,
	DIRTY_CUR_FOLDER
};

/*struct PNameRefCompare {
	bool operator()(PNameRef const a, PNameRef const b) const
	{
		return strcmp(a->c_str(), b->c_str()) < 0;
	}
};*/

struct ExpFieldInfo_t
{
	FieldPtr pField;
	Folder_t *pFolder;
	ExpFieldInfo_t() : pField(0), pFolder(0){}
	ExpFieldInfo_t(FieldPtr a, Folder_t *b) : pField(a), pFolder(b){}
	bool empty() const { return !pField; }
};

//typedef std::multimap<PNameRef, ExpFieldInfo_t, PNameRefCompare>	ExpFieldsMap;
//typedef ExpFieldsMap::iterator	ExpFieldsMapIt;
//typedef ExpFieldsMap::const_iterator	ExpFieldsMapCIt;

///////////////////////////////////////////////////////ExpTypesMap

class ExpTypeInfo_t
{
	TypePtr pTypeRef;
public:
	ExpTypeInfo_t() : pTypeRef(0){}
	ExpTypeInfo_t(TypePtr a) : pTypeRef(a){}
	void set(TypeObj_t *a){ pTypeRef = a; }
	TypePtr typeObj() const { return pTypeRef; }
	FolderPtr folder() const { 
		return USERFOLDER(pTypeRef);
	}
	bool empty() const {return !pTypeRef; }
};

class ExpTypesMap : public std::multimap<MyString, TypePtr>//exported types map consists of fully qualified names (should keep references to the types?)
{
public:
	ExpTypesMap(){}
	void addType(const MyString &, TypePtr);
	TypePtr findType(const MyString &, TypePtr module, bool bOwn);
};
//typedef std::multimap<PNameRef, TypePtr, PNameRefCompare>	ExpTypesMap;
typedef ExpTypesMap::iterator	ExpTypesMapIt;
typedef ExpTypesMap::const_iterator	ExpTypesMapCIt;

class CxxSymbMap : public  std::map<std::string, node_t *>
{
public:
	CxxSymbMap(){}
	~CxxSymbMap();
	iterator addSymbolIt(const char *, const char *);
	node_t *addSymbol(const char *, const char *);
	node_t *findSymbol(const char *) const;
};
typedef CxxSymbMap::iterator	CxxSymbMapIt;
typedef CxxSymbMap::const_iterator	CxxSymbMapCIt;



/////////////////////////////////////////////////////ProjectEx_t
class ProjectEx_t : public Project_t
{
	//ExpFieldsMap	mExpFieldsMap;//for fast lookup, given an imported field entry, get the exported one from another module, whith a folder it is contained in
	ExpTypesMap		mExpTypesMap;//fast lookup of exported types by name(!), returning a typeref and a folder it is contained in

	ExportPool_t mExportPool;
	//ExportPool_t mExportPoolEx;//for importing by ordinal

	CxxSymbMap	mCxxImports;//a sort of a cache, mapping mangled C++ names to the parsed-out expressions (fields or types)
		//Let it reside on project level to allow symbol sharing between modules during preliminary analyses.
		//It is not preserved between sessions, but can be re-built whenever a new module is added.

	bool	mDisperseMode;

	//IAnalyzer* mpAnalyzerEx;//post analyser or decompiler

	TypePtr mpUnExtModule;

	std::vector<ObjPtr> mCutList;//weak

public:
	ProjectEx_t(Main_t &, MemoryMgr_t &);
	virtual ~ProjectEx_t();

	const Mainx_t &mainx() const { return reinterpret_cast<const Mainx_t &>(mrMain); }
	const IGuiEx_t& guix() const;
	/*void setAnalyzerEx(IAnalyzer* p) {
		mpAnalyzerEx = p;
	}
	virtual IAnalyzer* analyzer() const {
		if (mpAnalyzerEx)
			return mpAnalyzerEx;
		return Project_t::analyzer();
	}
	virtual void killAnalizer() {
		if (mpAnalyzerEx)
		{
			delete mpAnalyzerEx;
			mpAnalyzerEx = nullptr;
			return;
		}
		Project_t::killAnalizer();
	}*/

	Decompiler_t* decompiler() const;

	CxxSymbMap &cxxSymbols(){ return mCxxImports; }
	const CxxSymbMap &cxxSymbols() const { return mCxxImports; }

	ExportPool_t& exportPool() { return mExportPool; }
	const ExportPool_t& exportPool() const { return mExportPool; }

	bool disperseMode() const { return mDisperseMode; }
	void setDisperseMode(bool b) {bool	mDisperseMode = b; }

	//ProbeEx_t *getContextEx();

	//ExpTypesMap	&expTypesMap() { return mExpTypesMap; }
	//const ExpTypesMap	&expTypesMap() const { return mExpTypesMap; }
	void addExpTypesMap(const MyString &s, TypePtr p) {
CHECK(s == "CDialog")
STOP
		mExpTypesMap.addType(s, p);
	}
	TypePtr findExpTypeMap(const MyString &s, TypePtr pModule, bool bOwn){
		return mExpTypesMap.findType(s, pModule, bOwn);
	}

	//const std::string &demangledName(const std::string &) const;
	//MyString scopeFromMangled(PNameRef) const;
	//MyString nameFromMangled(PNameRef, bool bTrimScope, unsigned &n) const;

	virtual void dispatchDirty() const;
	//bool removeExportedTypeInfo(const MyString &, TypePtr);
	//bool removeExportedTypeInfo(const MyString &, TypePtr);
	//ExpTypesMapIt getExportedTypeInfoIt(const MyString &, TypePtr module);
	//ExpTypesMapIt getExportedTypeInfoIt(const MyString &, TypePtr module);
	void CollectSymbols();
	ExpTypesMapIt getExportedTypeInfoIt2(const MyString &);
	ExpTypesMapIt getExportedTypeInfoIt(TypePtr) ;
	//TypePtr getExportedTypeInfo(const MyString &, TypePtr module);
	ObjPtr findObject(MyString, bool bPretty, CTypePtr iModule = nullptr) const;
	void setUnresolvedExternalsModule(TypePtr p);
	TypePtr unresolvedExternalsModule() const { return mpUnExtModule; }
	FieldPtr toExportedField(CFieldPtr pImpField) const;

	const std::vector<ObjPtr>& cutList() const { return mCutList; }
	bool addCutList(CObjPtr);
	ObjPtr takeCutListItem(size_t);

//	void rebuildTypesMap();
	//void rebuildTypesMap();

protected:
	virtual bool ExecuteCommand(Cmd_t &, int *);
	virtual void OnNewDebugger();
	virtual void OnDebuggerBreak(Debugger_t &);
	virtual Probe_t *NewLocus(const Locus_t &);
	//virtual TypePtr currrentVA(ADDR &) const;//seg
	virtual bool OnReleaseTypeRef(TypePtr);
	virtual TypePtr GetNameOwnerOf(TypePtr) const;
	virtual TypePtr GetNameOwnerOf(FieldPtr) const;
	virtual void OnShowTaskTop();
	virtual uint8_t dumpLineStatus(CTypePtr) const;
	virtual void OnDataEdited(TypePtr);
	virtual void OnDestroyProject(bool);
	virtual void OnDestroyModule(TypePtr, bool bClosing);
	virtual bool OnInstallFrontEnd(TypePtr, MyString);
	virtual void OnBogusFields(TypePtr);
	virtual TypePtr OnNewPortal(TypePtr);//, FrontImpl_t &);
	virtual FieldPtr OnMakeExported(const frame_t&, TypePtr, const MyStringEx&, TypePtr module);
	virtual void OnUnmakeExported(FieldPtr);
	virtual void OnDeclField(FieldPtr, AttrIdEnum, const MyStringEx&);
	virtual TypePtr OnModuleRef(TypePtr) const;
	virtual Struc_t *OnNewNamespace();
	virtual void OnMakeFunction(CFieldPtr);
	virtual void OnFinalizeFrontEnd(TypePtr);

	virtual unsigned fileType(const Folder_t &) const;
	virtual void dumpFile(const Folder_t &rFolder, int level, MyStreamUtil &) const;
	virtual TypePtr acquireType(const char *, CTypePtr owner);
	virtual void OnRecoverStruc(std::vector<TypePtr> &);
	virtual void OnRecoverField(FieldPtr, TypePtr module, TypePtr scope);
	virtual bool isAutoname(const MyString &, TypePtr iModule) const;
	//virtual PNameRef displayName(FieldPtr, TypePtr iModule) const;
	virtual MyString tipName(CFieldPtr, CTypePtr iModule) const;
	virtual MyString tipName(CTypePtr, CTypePtr iModule) const;
	virtual FieldPtr findGlobal(const char *, TypePtr iModule) const;
	virtual ObjPtr findObject(const char *, TypePtr module);
	virtual MyString typeName(CTypeBasePtr) const;
	virtual void typeNameScoped(CTypeBasePtr, char chopSymb, FullName_t &) const;
	virtual MyString fieldName(CFieldPtr) const;
	virtual MyString fieldDisplayName(CFieldPtr) const;
	virtual MyString typeDisplayName(CTypePtr) const;
	virtual bool deleteField(Locus_t&);
	virtual bool deleteBogusField(FieldPtr, NamesMgr_t *);

	virtual bool getCellStr(int col, CTypePtr, ADDR, unsigned, MyString &) const;
	virtual TypePtr objectDisplayScope(CObjPtr) const;
	virtual void PrintObjInfo(std::ostream &, ObjPtr, TypePtr module = nullptr);
	virtual const char *autoPrefix(CFieldPtr, CTypePtr) const;
	virtual void printMemInfo(std::ostream &) const;
};




#include "db/proj_cmd.h"



class ProjExCmdServer_t : public ProjCmdServer_t
{
public:
	ProjExCmdServer_t(CMDServerCommandMap& rcm, const ProjectInfo_t& rpi)
		: ProjCmdServer_t(rcm, rpi)
	{
	}
protected:
	friend class ProjectEx_t;
	virtual bool ExecuteCommand(Cmd_t&);
private:
	friend class ProjExCmdMap_t;
	ProjectEx_t& projx() const { return (ProjectEx_t&)mrProject; }

#define MYCOMMAND(name) \
	static int COMMAND_##name(ProjExCmdServer_t *, Cmd_t &); \
	int OnCommand_##name(Cmd_t &);
	MYCOMMAND(postformat);
	MYCOMMAND(mkstub);
	MYCOMMAND(dumpfunc);
	MYCOMMAND(decompile);
	MYCOMMAND(newfile);
	MYCOMMAND(delfile);
	MYCOMMAND(newsrc);
	MYCOMMAND(select_source_file);
	MYCOMMAND(symdump);
	MYCOMMAND(exptypdump);
	MYCOMMAND(gensrc);
	MYCOMMAND(setoutdir);
	MYCOMMAND(calcdp);
	MYCOMMAND(load_stubs);
#undef MYCOMMAND
};

class ProjExCmdMap_t : public ProjCmdMap_t
{
public:
	ProjExCmdMap_t()
	{
#define MYCOMMAND_REGISTER(arg)	RegisterCommand(#arg, (CMDServerHandlerPtr)ProjExCmdServer_t::COMMAND_##arg);
#define MYCOMMAND_REGISTER2(arg,alias)	RegisterCommand(#arg, (CMDServerHandlerPtr)ProjExCmdServer_t::COMMAND_##arg); \
	RegisterCommand(#alias, (CMDServerHandlerPtr)ProjExCmdServer_t::COMMAND_##arg);
		MYCOMMAND_REGISTER(postformat);
		MYCOMMAND_REGISTER(mkstub);
		MYCOMMAND_REGISTER(dumpfunc);
		MYCOMMAND_REGISTER(decompile);
		MYCOMMAND_REGISTER(newfile);
		MYCOMMAND_REGISTER(delfile);
		MYCOMMAND_REGISTER(newsrc);
		MYCOMMAND_REGISTER2(select_source_file, selsrc);
		MYCOMMAND_REGISTER(symdump);
		MYCOMMAND_REGISTER(exptypdump);
		MYCOMMAND_REGISTER(gensrc);
		MYCOMMAND_REGISTER(setoutdir);
		MYCOMMAND_REGISTER(calcdp);
		MYCOMMAND_REGISTER2(load_stubs, ldstb);
#undef MYCOMMAND_REGISTER
#undef MYCOMMAND_REGISTER2
	}
};













