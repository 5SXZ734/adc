#pragma once

#include "shared/front.h"
#include "command.h"
#include "type_seg.h"
#include "anlz.h"
#include "files.h"

class Main_t;
class IAnalyzer;
class Debugger_t;
class ProjCmdServer_t;
class FrontImpl_t;
class node_t;
template <typename T> class BinaryCleaner_t;
class IGui_t;
class ModuleInfo_t;
class DcInfo_t;
class BinaryAnalyzer_t;

#define CHOP_SYMB		'#'		//replacement char for dubbed names
#define	DUB_SEPARATOR	'\t'	//separator for dubbed names
#define	DUB_TERMINATOR	'\r'	//terminator for dubbed names

enum DirtyFlagsEnum
{
	DIRTY_FILES,
	DIRTY_GLOBALS,
	DIRTY_TYPES,
	DIRTY_NAMES,
	DIRTY_LOCALITY,
	DIRTY_LOCUS,
	DIRTY_TASKLIST,
	DIRTY_CURVA,
	DIRTY_LOCUS_ADJUSTED,//whenever a command needs to reposition the curIt
	//...
	DIRTY__TOTAL
};

class reserved_pfx_map : public std::map<std::string, unsigned>//second: 0:fields,1:types; bit[0]={0:suffix_is_hex,1:suffic_is_dec}
{
public:
	reserved_pfx_map();
	int is_autoname(const char *ps, value_t *pvalue = nullptr) const;//1:type, -1:type with negative id (debug), 2: field
	void add(const std::string &s, int nType = 0){
		insert(std::make_pair(s, nType));
	}
};





struct ExportPoolCmp {
    bool operator()(CFieldPtr a, CFieldPtr b) const {
        return strcmp(a->name()->c_str(), b->name()->c_str()) < 0;
    }
};

class ExportPool_t : std::set<FieldPtr, ExportPoolCmp>
{
	typedef std::set<FieldPtr, ExportPoolCmp> base;
	struct ExportPoolTagCmp {
		bool operator()(CFieldPtr a, CFieldPtr b) const {

			const char* pa(a->name()->tag());
			const char* pb(b->name()->tag());
//CHECK(!strcmp(pa, "#179|6") && !strcmp(pb, "#178|7"))
//STOP
			int ret(strcmp(pa, pb));
			return ret < 0;
		}
	};

	std::set<FieldPtr, ExportPoolTagCmp> mTagMap;
	typedef std::set<FieldPtr, ExportPoolTagCmp> TagMap;
	bool mbTagMapInitialized;
public:
	ExportPool_t() : mbTagMapInitialized(false){}
	/*FieldPtr get(CFieldPtr p) const
	{
		const_iterator i(find((FieldPtr)p));
		if (i == end())
			return nullptr;
		return *i;
	}*/
	bool add(CFieldPtr p){
		if (p)
		{
			if (!insert((FieldPtr)p).second)
				return false;
			if (mbTagMapInitialized)
				addTagMap((FieldPtr)p);
			//checkValid();
			return true;
		}
		return false;
	}
	bool remove(CFieldPtr p){
		if (p)
		{
			iterator i(base::find((FieldPtr)p));
			if (i != end())
			{
				erase(i);
				if (mbTagMapInitialized)
				{
					TagMap::iterator j(mTagMap.find((FieldPtr)p));
					if (j != mTagMap.end())
						mTagMap.erase(j);
				}
				//checkValid();
				return true;
			}
		}
		return false;
	}
	void clear(){
		base::clear();
		mTagMap.clear();
		mbTagMapInitialized = false;
	}
	FieldPtr find(const char *pc, bool /*bIgnoreTag*/) const {
		assert(pc);
		Field_t t;
		NameRef_t n((NamePtr)pc);
		//n.setPvt0((char*)pc);
		t.setName0(&n);
		const_iterator i(base::find(&t));
		FieldPtr p(i == end() ? nullptr : *i);
		t.setName0(nullptr);
		n.overrideKey(nullptr);// clearPvt();
		return p;
	}
	FieldPtr findPrefix(const char *pc, int iModuleTag = -1) const {//NOTE: pc must be stripped from suffix!
		assert(pc);
		Field_t t;
		NameRef_t n((NamePtr)pc);
		//n.setPvt0((char*)pc);
		t.setName0(&n);
		// ex: findPrefix(b) :
		//   1) a#   2) a#   3) a#
		//    ->b#      b       b
		//      c#    ->b#    ->c#
		//WARNING: for cases 2) and 3) the find() version must be called first
		const_iterator i(base::find(&t));//try exact match first
		if (i == end())
		{
			i = upper_bound(&t);//always greater
			if (i != end())
			{
				//find out if the string differ only by tags
				PNameRef pn((*i)->name());
				if (!pn->checkName(pc))
					i = end();//not prefixed
				else if (iModuleTag >= 0 && iModuleTag != pn->mid())
					i = end();
			}
		}
		FieldPtr p(i == end() ? nullptr : *i);
		//n.clearPvt();
		t.setName0(nullptr);
		n.overrideKey(nullptr);
		return p;
	}
	FieldPtr findSuffix(const char *pc/*, int iModuleTag = -1*/) const {
		assert(pc);
		const_cast<ExportPool_t*>(this)->initTagMap();
		Field_t t;
		NameRef_t n((NamePtr)pc);
		t.setName0(&n);
		TagMap::const_iterator i(mTagMap.find(&t));
		FieldPtr p;
		if (i != mTagMap.end())
		{
			p = *i;
			//if (iModuleTag >= 0 && p->name()->mid() != iModuleTag)
				//p = nullptr;
		}
		else
			p = nullptr;
		//n.clearPvt();
		t.setName0(nullptr);
		n.overrideKey(nullptr);
		return p;
	}
	void checkValid() const
	{
		const_iterator i(begin());
		if (i != end())
		{
			const_iterator j(i);
			for (++j; j != end(); i = j, ++j)
			{
				const char* a((*i)->name()->c_str());
				const char* b((*j)->name()->c_str());
				if (strcmp(a, b) > 0)
					ASSERT0;
			}
		}
		{
			TagMap::const_iterator ii(mTagMap.begin());
			if (ii != mTagMap.end())
			{
				TagMap::const_iterator j(ii);
				for (++j; j != mTagMap.end(); ii = j, ++j)
				{
					const char* a((*ii)->name()->tag());
//CHECK(!strcmp(a, "#18|7"))
//STOP
					const char* b((*j)->name()->tag());
					if (strcmp(a, b) > 0)
						ASSERT0;
				}
			}
		}
	}
private:
	void initTagMap()
	{
		if (!mbTagMapInitialized)
		{
			for (const_iterator i(begin()); i != end(); ++i)
				addTagMap(*i);
			mbTagMapInitialized = true;
		}
	}
	bool addTagMap(FieldPtr p)
	{
		const char* pc(p->name()->tag());
		if (!pc)
			return false;
		if (!mTagMap.insert((FieldPtr)p).second)
			return false;
		assert(mTagMap.find((FieldPtr)p) != mTagMap.end());
		return true;
	}
};



class FullName_t : public std::list<MyString>
{
	typedef std::list<MyString> Base;
public:
	FullName_t(){}
	FullName_t(const FullName_t& o) : Base(o){//COPY constructor
	}
	FullName_t(FullName_t&& o) : Base(std::move(o)){//MOVE constructor
	}
	explicit FullName_t(const MyString& s){
		push_back(s);
	}
	FullName_t& operator=(const FullName_t& o){//COPY assignment
		Base::operator=(o);
		return *this;
	}
	FullName_t& operator=(FullName_t&& o){//MOVE assignment
		Base::operator=(std::move(o));
		return *this;
	}
	MyString join(const MyString& sep = "::") const {
		MyString s;
		for (const_iterator i(begin()); i != end(); ++i)
		{
			if (i != begin())
				s.append(sep);
			s.append(*i);
		}
		return std::move(s);
	}
	void append(const MyString& s){
		push_back(s);
	}
	void append(MyString&& s){
		push_back(s);
	}
	void prepend(const MyString& s){
		insert(begin(), s);
	}
	friend std::ostream& operator<<(std::ostream&, const FullName_t&);
	/*static FullName_t split(const MyString& str, const MyString& sep = "::")
	{
		FullName_t lst;
		int j = 0;
		int i = str.find(sep, j);
		while (i != -1)
		{
			if (i > j && i <= (int)str.length())
				lst.push_back(str.substr(j, i - j));
			j = (int)(i + sep.length());
			i = str.find(sep, sep.length() > 0 ? j : j + 1);
		}
		int l((int)str.length() - 1);
		if (str.substr(j, l - j + 1).length() > 0)
			lst.push_back(str.substr(j, l - j + 1));
		return std::move(lst);
	}*/
	bool startsWith(const FullName_t& pfx)
	{
		const_iterator i(begin());
		for (const_iterator j(pfx.begin()); j != pfx.end(); ++i, ++j)
			if (i == end() || *i != *j)
				return false;
		return true;
	}
	void trimLeft(size_t n)
	{
		iterator to(begin());
		std::advance(to, n);
		erase(begin(), to);
	}
	FullName_t scope() const {
		FullName_t a;
		if (!empty())
			std::copy(begin(), std::prev(end()), std::back_inserter(a));//all but the last
		return std::move(a);
	}
};

inline std::ostream& operator<<(std::ostream& os, const FullName_t& self)
{
	os << self.join();
	return os;
}



class Project_t : public Seg_t
{
protected:
	friend class ProjectInfo_t;
	//friend class ProjectInfoEx_t;
	typedef	Seg_t	Base_t;

	Main_t	&mrMain;
	MemoryMgr_t &mrMemMgr;
	const IGui_t &mrGui;

	unsigned	muDirty;//DirtyFlagsEnum

	//ProjCmdServer_t	*mpCmdServer;
	//unsigned		mStrucId;

	MyString		msPath;

	FilesMgr0_t	mFiles;

	//std::vector<BinaryInfo_t>	mBinInfo;
	Folder_t	*mpStartUpBinary;

	std::list<IAnalyzer*>	mAnlz;
	//BinaryAnalyzer_t*	mpAnlz;

//	SmartVector_t<Type_t *>	mTypes;

	static Project_t *spSelf;


	Debugger_t	*mpDebugger;
	
	bool		mbIsClosing;
	TypePtr		mpSelf;

	SlotVector<FRONT_t>	mFEs;

	std::set<TypePtr>	mTypesMapOwners;//all (attic) typesmap owners must be registered here
	reserved_pfx_map	mAutoPfxMap;

public:
	Project_t(Main_t &, MemoryMgr_t &);
	virtual ~Project_t();

	Main_t &main() const { return mrMain; }
	const IGui_t &gui() const { return mrGui; }
	SlotVector<FRONT_t>	&frontVec(){ return mFEs; }
	const SlotVector<FRONT_t>	&frontVec() const { return mFEs; }

	FRONT_t *frontFromIndex(size_t i) const {
		return mFEs.getSlot(i);
	}
	void releaseFront(size_t i){
		FRONT_t *pfe(mFEs.getSlot(i));
		if (pfe)
		{
			if (pfe->RefsNum() == 1)
				mFEs.deleteSlot(i)->Release();
			else
				pfe->Release();
		}
	}
	size_t newFrontSlot(const char *name, int id)
	{
		return mFEs.newSlot(new FRONT_t(name, id));
	}

	std::set<TypePtr> &typeMaps(){ return mTypesMapOwners; }
	const std::set<TypePtr> &typeMaps() const { return mTypesMapOwners; }

	void setSelfObj(TypePtr p){ mpSelf = p; }
	TypePtr self() const { return mpSelf; }
	unsigned	isDirty() const { return muDirty; }
	void markDirty(unsigned/*DirtyFlagsEnum*/ e){
		muDirty |= (1 << e);
//CHECK(checkDirty(DIRTY_GLOBALS))
//STOP
	}
	bool checkDirty(unsigned/*DirtyFlagsEnum*/ e) const { return (muDirty & (1 << e)) != 0; }
	void clearDirty(){ muDirty = 0; }
	virtual void dispatchDirty() const;
	TypePtr getStockType(OpType_t) const;

	virtual Project_t *typeProject() const { return const_cast<Project_t *>(this); }
	virtual int ObjType() const { return OBJID_TYPE_PROJECT; }

	static Project_t *instance(){ return spSelf; }
	virtual bool ExecuteCommand(Cmd_t &, int *);

	MemoryMgr_t &memMgr() const { return mrMemMgr; }
	//MemoryMgr_t &memMgrNS();
	//TypesMgr_t *typesMgrNS();
	//void deleteTypesMgrNS(){ delete mpTypesMgrNS;  mpTypesMgrNS = nullptr; }
	//void deleteMemMgrNS(){ delete mpMemMgrNS; mpMemMgrNS = nullptr; }

	void setClosing(){
		assert(!mbIsClosing);
		mbIsClosing = true;
	}
	bool isClosing() const {
		return mbIsClosing;
	}

	virtual bool Autoname(CFieldPtr , MyString &) const;
	//int objOverrideNext() { return ++mStrucId; }
	Folder_t *startupFolder() const { return mpStartUpBinary; }
	void setStartupFolder(Folder_t *p){ mpStartUpBinary = p; }

	virtual Probe_t *NewLocus(const Locus_t &);

	const FilesMgr0_t &files() const { return mFiles; }
	FilesMgr0_t &files() { return mFiles; }
	const FileFolder_t &rootFolder() const { return *files().mpRootFolder->fileFolder(); }
	FileFolder_t &rootFolder(){ return *files().mpRootFolder->fileFolder(); }
	MyString outputDir() const {
		return files().mpRootFolder->_key();
	}
	void setOutputDir(MyFolderKey s) {
		files().mpRootFolder->overrideKey(s);
	}

	void setPath(const MyString &s);
	const MyString &path(){ return msPath; }
	
/*	virtual void setLocus(Probe_t *);
	Probe_t *setContext(Probe_t *pctx){ 
		return mctx.set(pctx); }
	Probe_t *getContext() const {
		Probe_t *p(mctx.ptr());
		if (p)
			p->AddRef();
		return p;
	}
	void releaseContext(I_Context *pCtx) const {
		assert(pCtx);
		pCtx->Release();
	}*/

	//size_t addBinarySlot(DataSource_t *);
	//I_DataSource *binarySlotAt(size_t) const;

	virtual TypePtr GetNameOwnerOf(TypePtr) const;
	virtual TypePtr GetNameOwnerOf(FieldPtr) const;
	//Obj_t *GetContextObj();
	virtual void OnShowTaskTop() {}
	virtual void OnDestroyProject(bool bClosing);
	virtual void OnDestroyModule(TypePtr, bool bClosing);
	virtual void OnDataEdited(TypePtr){ markDirty(DIRTY_GLOBALS); }
	virtual bool OnInstallFrontEnd(TypePtr, MyString){ return false; }
	virtual void OnBogusFields(TypePtr) {}
	virtual void OnNewDebugger(){}
	//virtual FieldPtr OnFieldInsertFailed2(TypePtr, ADDR, FieldPtr );
	virtual TypePtr OnNewPortal(TypePtr);//, FrontImpl_t &);//seg
	virtual FieldPtr OnMakeExported(const frame_t&, TypePtr, const MyStringEx&, TypePtr module);
	virtual void OnUnmakeExported(FieldPtr) {}
	virtual void OnDeclField(FieldPtr, AttrIdEnum, const MyStringEx&){}
	virtual TypePtr OnModuleRef(TypePtr) const { return nullptr; }
	virtual Struc_t *OnNewNamespace(){ return new Struc_t(); }
	virtual void OnMakeFunction(CFieldPtr){}
	virtual void OnFinalizeFrontEnd(TypePtr) {}

	virtual unsigned fileType(const Folder_t &) const { return 0; }
	virtual void dumpFile(const Folder_t &, int, MyStreamUtil &) const { assert(0); }//later
	virtual TypePtr acquireType(const char *, CTypePtr){ return nullptr; }
	virtual void OnRecoverStruc(std::vector<TypePtr> &){}
	virtual void OnRecoverField(FieldPtr, TypePtr module, TypePtr scope);
	virtual bool isAutoname(const MyString &, TypePtr iModule) const;
	//virtual PNameRef displayName(FieldPtr, TypePtr iModule) const;
	virtual MyString tipName(CFieldPtr, CTypePtr iModule) const;
	virtual MyString tipName(CTypePtr, CTypePtr iModule) const;
	virtual FieldPtr findGlobal(const char *, TypePtr) const { return nullptr; }
	virtual ObjPtr findObject(const char *, TypePtr module);
	virtual MyString typeName(CTypeBasePtr) const;//terminal types only
	virtual void typeNameScoped(CTypeBasePtr, char chopSymb, FullName_t &) const;
	virtual void PrintObjInfo(std::ostream &, ObjPtr, TypePtr = nullptr){}
	virtual TypePtr objectDisplayScope(CObjPtr) const;
	virtual bool deleteField(Locus_t&);
	virtual bool deleteBogusField(FieldPtr, NamesMgr_t *);

	virtual MyString fieldName(CFieldPtr) const;
	virtual MyString fieldDisplayName(CFieldPtr) const;//for binary dump
	virtual MyString typeDisplayName(CTypePtr) const;//for bin dump
	virtual bool getCellStr(int, CTypePtr, ADDR, unsigned, MyString &) const { return false; }//for binary dump
	virtual const char *autoPrefix(CFieldPtr, CTypePtr) const;
	int checkAutoPrefix(const char *, value_t * = nullptr) const;//1:type, -1:type with negative id (debug), 2: field

	void setDebugger(Debugger_t *p){ mpDebugger = p; }
	Debugger_t *debugger() const { return mpDebugger; }

	IAnalyzer* analyzer() const {
		if (mAnlz.empty())
			return nullptr;
		return mAnlz.back();
	}

	void pushAnalyzer(IAnalyzer *p){
		assert(p);
		mAnlz.push_back(p);
	}

	void popAnalizer() {
		if (!mAnlz.empty())
		{
			IAnalyzer* p(mAnlz.back());
			mAnlz.pop_back();
			delete p;
		}
	}

	BinaryAnalyzer_t* binAnalizer() const;//assume a top one
	BinaryAnalyzer_t* findBinAnalizer() const;

	int execAnalyzer();
	bool checkAnalyzerStatus();

	//void PrintTypesList(Struc_t * pStruc, ADDR a);

	//MyString expandCommand(MyString, I_Context*);

	virtual void OnDebuggerBreak(Debugger_t &) {}
	virtual bool OnReleaseTypeRef(TypePtr) { return false; }
	//virtual TypePtr currrentVA(ADDR &) const;
	virtual uint8_t dumpLineStatus(CTypePtr) const;//struc
	virtual void printMemInfo(std::ostream &) const;

protected:
	virtual const char *printType() const { return "project"; }
};

//#define PROJ	(*Project_t::instance())






#define DECLARE_CONTEXT(ctx) \
	Probe_t *pctx(dynamic_cast<Probe_t *>(args.context())); \
	if (!pctx) { \
		fprintf(STDERR, "%s: No appropriate context\n", args[0].c_str()); \
		return 0; } \
	Probe_t &ctx(*pctx)



