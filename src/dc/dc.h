#pragma once

#include <map>
#include <list>
#include <set>

#include "db/mem.h"
#include "db/type_seg.h"
#include "db/type_module.h"
#include "probe_ex.h"
#include "xref.h"
#include "files_ex.h"
#include "proj_ex.h"
#include "arglist.h"
#include "front_dc.h"

class Seg_t;
class I_Front;
struct FE_t;
struct STORAGE_t;
class Struc_t;
struct Msg_t;
class Output_t;
class TypesMgr_t;
class MyStreamBase;
class FilesMgr_t;
class FileDef_t;
class StubMgr_t;
class ProjectEx_t;
class Decompiler_t;
class FileInfoCmdServer_t;
class node_t;
class TypeFomNode_t;
struct FuncProfile_t;
struct ProtoProfile_t;
class CxxSymbMap;
struct StubBase_t;

typedef std::vector<REG_t>	GPRs_t;

enum {
		DC__SRC		= 0x0000FF00L,	//source data attached flag
	DC_DC_			= 0x10000000L,	//asm raw files creation mode
	DC_GUI			= 0x20000000L,	//gui mode
	DC_NOECHO		= 0x40000000L,	//no input stream echoed to console
	DC_DIRECT		= 0x80000000L,	//direct output: no dumping
};

#if(0)
typedef std::map<OpPtr, FieldPtr >	mapOp2Fld_t;
typedef mapOp2Fld_t::iterator		mapOp2Fld_it;
typedef mapOp2Fld_t::const_iterator		mapOp2Fld_itc;
#endif

typedef std::multimap<int, MyStreamPos>	MyPtrDump;
struct MyPtrDumps : public std::map<ADDR, MyPtrDump>
{
	My::Stream0 mss;
	MyPtrDumps() : mss(0x1000) {}//4K
	~MyPtrDumps()
	{
		//for (size_t i(0); i < size(); i++)
		//delete at(i);
	}
	void add(ADDR funcAddr, int line, MyStreamBase &ss)
	{
		std::pair<iterator, bool> ret(insert(make_pair(funcAddr, MyPtrDump())));
		MyPtrDump &a(ret.first->second);
		a.insert(std::make_pair(line, mss.tellp()));
		MyStreamUtil ssu(mss);
		ssu.WriteStream(ss);
	}
	bool read(ADDR funcAddr, int line, MyStreamBase &ss)
	{
		//get a function
		iterator it(find(funcAddr));
		if (it == end())
			return false;

		MyPtrDump &r(it->second);

		//get a line entry (can be >1)
		std::pair <MyPtrDump::iterator, MyPtrDump::iterator> ret;
		ret = r.equal_range(line);

		MyStreamUtil ssh(ss);
		for (MyPtrDump::const_iterator j(ret.first); j != ret.second; j++)
		{
			const MyStreamPos &p(j->second);
			mss.seekg(p);
			ssh.WriteString(mss);
		}
		return true;
	}
};


typedef std::map<MyString, std::pair<ADDR, FolderPtr> >	ImpLookupMap;


////////////////////////////////////////////////////////// Dc_t
class Dc_t
{
	typedef	Type_t	Base_t;
public:
	ProjectEx_t&	mrProject;
	FolderPtr	mpModuleFolder;
	TypePtr		miFrontSeg;

	I_Front		*mpIFront;//nullptr for phantom segs
	FDC_t		mFDC;////nullptr for phantom segs

	std::vector<GlobPtr>	mIntrinsics;

	MyPtrDumps	mPtrDumps;

	//////////////////////////////import
	struct ImpEntry_t//given a this module's VA, find an exported field (and info) in another module
	{
		TypePtr pModule;//external module exporting a symbol
		unsigned short ordinal;
		ImpEntry_t(TypePtr p, unsigned short o)
			: pModule(p), ordinal(o)
		{
		}
	};

	class ImportMap : private std::map<ADDR, ImpEntry_t>//impVA => ordinal
	{
		typedef std::map<unsigned short, ADDR>	Ord2ImpVAMap;//ordinal => import VA
		typedef std::map<TypePtr, Ord2ImpVAMap>	ModImpMap;//ordinal=>impVA
		ModImpMap	mReverse;//module:ordinal pair => impVA
	public:
		ImportMap() {}
		const ImpEntry_t* toOrdinal(ADDR va) const
		{
			const_iterator it(find(va));
			if (it == end())
				return nullptr;
			return &it->second;
		}
		ADDR fromOrdinal(unsigned short ord, TypePtr pModule) const
		{
			ModImpMap::const_iterator i(mReverse.find(pModule));
			if (i == mReverse.end())
				return 0;
			const Ord2ImpVAMap& m(i->second);
			Ord2ImpVAMap::const_iterator j(m.find(ord));
			if (j == m.end())
				return 0;
			return j->second;
		}
		bool add(TypePtr pExportingModule, unsigned short ordinal, ADDR impVA)
		{
			insert(std::make_pair(impVA, ImpEntry_t(pExportingModule, ordinal)));
			std::pair<ModImpMap::iterator, bool> ret(mReverse.insert(std::make_pair(pExportingModule, Ord2ImpVAMap())));
			Ord2ImpVAMap& m(ret.first->second);
			m.insert(std::make_pair(ordinal, impVA));
			return true;
		}
		bool isEmpty() const { return empty(); }
	};

	//ImportMap	m_imp;

	//////////////////////////static
	/*struct StaticInfo
	{
		CFieldRef	rField;
		TypePtr		iClass;
		StaticInfo(CFieldRef a, TypePtr b)
			: rField(a), iClass(b)
		{
		}
	};

	struct StaticMapCompare {
		bool operator()(const StaticInfo &left, const StaticInfo &right) const {
			return left.rField.address() < right.rField.address();
		}
	};*/

//	typedef std::multiset<StaticInfo, StaticMapCompare>		StaticMap;
//	typedef	StaticMap::iterator			StaticMapIt;
//	typedef StaticMap::const_iterator	StaticMapCIt;

//	StaticMap	m_stat;//static members of classes

	//std::map<MyString, TypePtr>	mTypeCache;

	//unresolved externals
	class BogusFieldMap : public std::multimap<ADDR, FieldPtr>
	{
	public:
		BogusFieldMap()
		{
		}
		void add(ADDR uOrd, CFieldPtr p)
		{
			insert(std::make_pair(uOrd, (FieldPtr)p));
		}
		FieldPtr take(FieldPtr p)
		{
			if (p)
				for (iterator i(begin()); i != end(); i++)
					if (i->second == p)
					{
						erase(i);
						return p;
					}
			return nullptr;
		}
		FieldPtr take(ADDR uOrd)
		{
			iterator i(find(uOrd));
			if (i == end())
				return nullptr;//some other?
			FieldPtr p(i->second);
			erase(i);
			return p;
		}
		FieldPtr takeFirst(ADDR* puOrd)
		{
			if (empty())
				return nullptr;
			iterator i(begin());
			if (puOrd)
				*puOrd = i->first;
			FieldPtr p(i->second);
			erase(i);
			return p;
		}
	};

	BogusFieldMap		mBogusFields;//temporary fields, during phantom segment adoption
	
public:
	Dc_t(ProjectEx_t &, TypePtr iFrontSeg, TypePtr module);
	~Dc_t();

	TypePtr module() const { return mpModuleFolder->fileModule()->module(); }
	Module_t &moduleRef(){ return *module()->typeModule(); }
	const Module_t &moduleRef() const { return *module()->typeModule(); }
	const MyString &moduleName() const { return moduleRef().title(); }
	
	FolderPtr moduleFolder() const { return mpModuleFolder; }

	FileModule_t &moduleFile(){ return *moduleFolder()->fileModule(); }
	const FileModule_t &moduleFile() const { return *moduleFolder()->fileModule(); }

	//ImportMap& importMap() { return m_imp; }
	//ExportMap_t& exportMap();
	//const ExportMap_t& exportMap() const;

	const Dc_t &self() const { 
		return *this;
	}
	FRONT_t &front();
	const FRONT_t &front() const;
	I_Front *frontEnd() const {
		return mpIFront;
	}
	const FDC_t& fdc() const { return mFDC; }
	I_FrontDC *frontDC() const {
		return mFDC.get(); }
	const FE_t &fe() const {
		return mFDC.fe();
	}
	MyString frontName() const {
		return front().name();
	}
	I_Front *setFrontEnd(I_Front *p){
		mpIFront = p;
		return mpIFront;
	}
	void setFrondDC(I_FrontDC *p){
		mFDC.set(p);
	}
	void initFrontend();
	const RegInfo_t *fromRegName(const char *s, OPC_t opc = OPC_NULL) const {
		return mFDC.fromRegName(s, opc);
	}
	bool hasReg(const char *s) const {
		return mFDC.contains(s);
	}
	const char *toRegName(SSID_t ssid, int ofs, int sz, unsigned f, char buf[80]) const {
		return mFDC.regName(ssid, ofs, sz, f, buf);
	}
	const char *flagsToStr(SSID_t ssid, unsigned flags, char sep = '|') const {
		return mFDC.flagsToStr(ssid, flags, sep);
	}
	bool flagsToArgList(SSID_t ssid, FlagMaskType flags, Arg1List_t &l) const	{
		return mFDC.flagsToArgList(ssid, flags, l);
	}
	REG_t fromReg(OPC_t opc, unsigned ofs) const {
		const RegInfo_t *p(mFDC.fromReg(opc, ofs));
		if (p)
			return REG_t(p->offs, p->opsz);
		return REG_t();
	}
	bool isRegValid(OPC_t opc, unsigned ofs, unsigned opsz) const {
		return mFDC.exists(opc, ofs, opsz);
	}
	bool isRegValid(SSID_t ssid, const REG_t &r) const {
		return mFDC.exists((OPC_t)ssid, r.m_ofs, r.m_siz);
	}

//	FileInfoCmdServer_t	*cmdServer() const { return mpCmdServer; }
	MemoryMgr_t &memMgr() const { return mrProject.memMgr(); }//global
	
	ProjectEx_t &project() const { return mrProject; }
	void cleanExportMap();

	bool isFolderOfKind(const Folder_t &r, FTYP_Enum e) const { return &r == moduleFile().special(e); }
	FolderPtr	folderPtr(FTYP_Enum eFile) const { return moduleFile().special(eFile); }

	adcui::FolderTypeEnum IsViewFile(const Folder_t &);

	void NewPtrDump(ADDR funcAddr, int line, MyStreamBase &);
	bool ReadPtrDump(ADDR, int, MyStreamBase &);
	
	GlobPtr getIntrinsic(size_t) const;

	TypePtr primeSeg() const { return miFrontSeg; }

	FolderPtr stubsFolderPtr() const { return moduleFile().special(FTYP_PROTOTYPES); }
	StubMgr_t &stubs();//{ return mStubsMgr; }//stubsFolderPtr()->m.fileStubs()->stubs(); }
	const StubMgr_t &stubs() const;// const { return mStubsMgr; }//stubsFolderPtr()->m.fileStubs()->stubs(); }
	StubMgr_t *stubsPtr() const;// {  return !mStubsMgr.empty();
		//FolderPtr p(stubsFolderPtr());
		//return (p && !p->m.fileStubs()->stubs().empty());

	//ProbeEx_t *getContext() const;
	//const ProbeEx_t &context() const;
	//ProbeEx_t &context();
	//void setContext(ProbeEx_t *);

	const STORAGE_t&	SS(SSID_t) const;
	int			PtrSize() const;
	int			PtrSizeEx() const;
	int			stackAddrSize() const;

	//void addCachedType(const MyString &fullName, TypePtr self);
	//TypePtr removeCachedType(const char *fullName, TypePtr self);
	//TypePtr findCachedType(const MyString &fullName, TypePtr module);
	void saveStubs() const;

	void addBogusField(ADDR va, FieldPtr p){
		mBogusFields.add(va, p);
	}
	BogusFieldMap	&bogusFields(){ return mBogusFields; }
	const BogusFieldMap	&bogusFields() const { return mBogusFields; }
};



#define DCREF(pModule) pModule->UserData<Dc_t>()
//Dc_t *DCREF(TypePtr iModule) { return (Dc_t *)iModule->typeModule()->mpDC; }


typedef XRef_t<FieldPtr> XFieldLink_t;//XRefObj_t;
typedef XRefList_t<FieldPtr> XFieldList_t;//XRefObjList_t;


typedef	std::list<GlobPtr>	ClassMemberList_t;//WARNING: cannot be a map (some distinct entries may have the same address)
//typedef	XFieldList_t	ClassMemberList_t;
typedef	ClassMemberList_t::iterator	ClassMemberListIt;
typedef	ClassMemberList_t::const_iterator	ClassMemberListCIt;

typedef std::multimap<unsigned, GlobPtr> ClassVirtMembers_t;

struct ClassVTable_t
{
	GlobPtr self;
	ClassVirtMembers_t entries;
	ClassVTable_t() : self(nullptr) {}
	ClassVTable_t(GlobPtr p) : self(p) {}
	ADDR address() const { return self ? self->_key() : ADDR(-1); }
};

typedef std::multimap<unsigned, ClassVTable_t>	ClassVTables_t;

#define	UNK_VTABLE_OFF	-1//INT_MIN
#define	UNASSOC_VTABLE_OFF	-2
#define UNK_VENTRY_OFF	-1

#define	NESTED_SCOPES	1

/////////////////////////////////////////////////////////TypeClass_t
class TypeClass_t : public Struc_t
{
	ClassMemberList_t	mMethods;
	ClassVTables_t		mVTables;
public:
	TypeClass_t(){}
	TypeClass_t(int id) : Struc_t(id){}//override id
	ClassMemberList_t& methods() { return mMethods; }
	const ClassMemberList_t	&methods() const { return mMethods; }
	ClassVTables_t& vtables() { return mVTables; }
	const ClassVTables_t& vtables() const { return mVTables; }
	bool hasMethods() const { return !mMethods.empty(); }
	void moveFrom(TypeClass_t &);
	bool addMember(GlobPtr);
	ClassVTable_t &addVTable(GlobPtr, int voffs);
	bool addVirtMember(GlobPtr, int off, ClassVTable_t &);
protected:
	virtual int ObjType() const { return OBJID_TYPE_CLASS; }
	virtual TypeClass_t * typeClass() const { return const_cast<TypeClass_t *>(this); }
	virtual const char *printType() const { return "class"; }
};

/////////////////////////////////////////////////////////TypeNameSpace_t
class TypeNamespace_t : public TypeClass_t
{
public:
	TypeNamespace_t(){}
	TypeNamespace_t(int id) : TypeClass_t(id){}//override id
protected:
	virtual int ObjType() const { return OBJID_TYPE_NAMESPACE; }
	virtual TypeNamespace_t * typeNamespace() const { return const_cast<TypeNamespace_t *>(this); }
	virtual const char *printType() const { return "namespace"; }
	virtual bool maybeUnion() const override { return false; }
};

#if(0)
/////////////////////////////////////////////////////////TypeVFTable_t
class TypeVFTable_t : public Struc_t
{
	typedef	Struc_t	Base_t;
	//TypePtr mpOwnerClass;
	TypePtr mpBaseClass;//may be nullptr

public:
	TypeVFTable_t() : /*mpOwnerClass(nullptr),*/ mpBaseClass(nullptr) {}
	virtual ~TypeVFTable_t(){}

	/*void setOwnerClass(TypePtr p){ mpOwnerClass = p; }
	TypePtr ownerClass() const {
		return mpOwnerClass;
	}*/

	virtual void setBaseType(TypePtr p){ mpBaseClass = p; }
	virtual TypePtr baseType() const {
		return mpBaseClass;
	}

	virtual TypeVFTable_t * typeVFTable() const override { return const_cast<TypeVFTable_t *>(this); }
	virtual int ObjType() const { return OBJID_TYPE_VFTABLE; }
	virtual const char *printType() const { return "vftable"; }
};

/////////////////////////////////////////////////////////TypeVBTable_t (base class vtable)
class TypeVBTable_t : public TypeVFTable_t
{
public:
	TypeVBTable_t(){}
	virtual ~TypeVBTable_t(){}

	virtual TypeVBTable_t * typeVBTable() const override  { return const_cast<TypeVBTable_t *>(this); }
	virtual int ObjType() const { return OBJID_TYPE_VBTABLE; }
	virtual const char *printType() const { return "vbtable"; }
};

/////////////////////////////////////////////////////////TypeLVFTable_t (local vftable)
class TypeLVFTable_t : public TypeVFTable_t
{
public:
	TypeLVFTable_t(){}
	virtual ~TypeLVFTable_t(){}

	virtual TypeLVFTable_t * typeLVFTable() const override { return const_cast<TypeLVFTable_t *>(this); }
	virtual int ObjType() const { return OBJID_TYPE_LVFTABLE; }
	virtual const char *printType() const { return "lvftable"; }
};

/////////////////////////////////////////////////////////TypeCVFTable_t (construction vftable)
class TypeCVFTable_t : public TypeVFTable_t
{
public:
	TypeCVFTable_t(){}
	virtual ~TypeCVFTable_t(){}

	virtual TypeCVFTable_t * typeCVFTable() const override { return const_cast<TypeCVFTable_t *>(this); }
	virtual int ObjType() const { return OBJID_TYPE_CVFTABLE; }
	virtual const char *printType() const { return "cvftable"; }
};
#endif


