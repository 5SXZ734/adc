#pragma once

#include <list>
#include <set>
#include <fstream>
#include "qx/MyFileMgr.h"
#include "shared/misc.h"
#include "shared/sbtree.h"
#include "db/files.h"
#include "db/names.h"
#include "dc/globs.h"
#include "mem_ex.h"
#include "qx/MyFileMgr.h"

typedef std::list<TypePtr> TypesList;
typedef TypesList::iterator TypesListIt;
typedef TypesList::const_iterator TypesListCIt;
typedef TypesList::reverse_iterator TypesListRIt;




//typedef std::multimap<ADDR, GlobPtr> FieldsList;
typedef sbtree::config1_s<GlobObj_t>		GlobMapConfig;//there can be cloned fields
typedef sbtree::size_balanced_tree<GlobMapConfig>	GlobMap;
typedef GlobMap::iterator GlobMapIt;
typedef GlobMap::const_iterator GlobMapCIt;

typedef unique_list<FolderPtr>	IncludeList_t;
typedef IncludeList_t::iterator	IncludeListIt;
typedef IncludeList_t::const_iterator	IncludeListCIt;

//typedef std::list<GlobPtr>	IntrList;
//typedef IntrList::iterator IntrListIt;
//typedef IntrList::const_iterator IntrListCIt;


class FileDef_t : public File_t
{
	FolderPtr		mpOwnerFolder;
	IncludeList_t	mIncludes;	//list of headers this file includes
	TypesList		mTypes;
	//IntrList		mIntr insics;
	GlobMap			mGlobs;
	MemoryMgrEx_t *mpMemMgr;
	bool mbDirty;//needs saving?
	//differred load support
	std::vector<CTypePtr>	mDeferredTypes;//for dispersed save&load. Non-empty if the file is off (alement at 0 index must always be nullptr).
	MyString				mDispersedName;

//public:
//	static MemoryMgr_t gFakeMemMgr;
public:
	FileDef_t();
	~FileDef_t();

	void setFolder(FolderPtr p){ mpOwnerFolder = p; }

	void AttachMemMgr();
	FolderPtr ownerFolder() const { return mpOwnerFolder; }

	const TypesList &types() const { return mTypes; }
	
	std::vector<CTypePtr> &deferredTypes() { return mDeferredTypes; }
	const std::vector<CTypePtr> &deferredTypes() const { return mDeferredTypes; }
	void setDeferredTypes(const std::vector<CTypePtr> &v);
	void releaseDeferredTypes();
	const MyString &dispersedName() const { return mDispersedName; }
	void setDispersedName(MyString s){ mDispersedName = s; }
	bool isOffloaded() const { return mDeferredTypes.size() > 0; }

	//TypesList &types(){ return mTypes; }
	enum {AT_TAIL, AT_HEAD, AT_BEFORE_LAST};
	void	addTypeObj(TypePtr, unsigned = AT_TAIL);
	//void	addIntrObj(GlobPtr);
	//bool		FindType(TypePtr) const;
	TypePtr takeType(TypePtr);

	const GlobMap &globs() const { return mGlobs; }
	GlobMap &globs(){ return mGlobs; }
	bool	hasGlobs() const { return !mGlobs.empty(); }
	//?bool	insertField(FieldPtr);
	bool	insertGlob(GlobPtr);
	bool	insertGlob(FieldExPtr);
	GlobMapIt	findGlobIt(CGlobPtr) const;
	GlobMapIt	findGlobSlowIt(CGlobPtr) const;
	GlobMapIt	firstGlobIt() const;
	bool		findGlob(CGlobPtr);
	GlobPtr takeGlobIt(GlobMapIt);
	GlobPtr takeGlob(GlobPtr);
	GlobPtr takeGlobFront();
	GlobPtr		firstGlob() const;
	bool		hasNonStubs() const;

	bool ownsMemory() const { 
		return mpMemMgr != nullptr;
	}
	MemoryMgr_t &memMgr() const {
		return *mpMemMgr;
	}
	MemoryMgrEx_t &memMgrEx() const {
		return reinterpret_cast<MemoryMgrEx_t&>(memMgr());
	}
	bool IsDirty() const {
		return mbDirty;
	}
	void SetDirty(bool bDirty){
		mbDirty = bDirty;
	}

	const IncludeList_t &includes() const { return mIncludes; }
	IncludeList_t &includes() { return mIncludes; }

	size_t CountObjs() const {
		return mTypes.size() + mGlobs.size();
	}

	bool IsEmpty() const {
		return mTypes.empty() && mGlobs.empty(); }

	//FieldPtr FindMLoc(const char *);
	bool	addIncludeList(FolderPtr);
	bool	removeIncludeList(FolderPtr);
	void	CheckInclusion();
	void	checkForwardDeclaration(TypePtr);
	void	buildDependencies(std::set<CFolderPtr> &);
protected:
	virtual FILEID_t fileId() const { return FILEID_FILE; }
	virtual FileDef_t *fileDef() const { return const_cast<FileDef_t *>(this); }
public:
	virtual bool isEmpty() const;
private:
	TypesListIt	findTypeIt(TypePtr);
	TypesListIt findInsertTopIt(TypePtr);
	TypesListIt findInserBottomIt(TypePtr);
	void printTypes(std::ostream &);
};


typedef	sbtree_multimap<TypeBasePtr, PNameRef>	PrettyObjMap;//globs and types

class FileTempl_t : public File_t
{
	PrettyObjMap m;
public:
	FileTempl_t()
	{
	}
	bool addPrettyName(TypeBasePtr p, PNameRef pn){
		return m.insert_unique(std::make_pair(p, pn)).second;
	}
	PNameRef findPrettyName(CTypeBasePtr f) const {
		PrettyObjMap::const_iterator i(m.find((TypeBasePtr)f));
		if (i != m.end())
			return i->second;
		return nullptr;
	}
	PNameRef takePrettyName(CTypeBasePtr p){
//CHECKID(g, 0x9d90)
//STOP
		PrettyObjMap::iterator i(m.find((TypeBasePtr)p));
		if (i == m.end())
			return nullptr;
		PNameRef pn(i->second);
		m.erase(i);
		return pn;
	}
	PrettyObjMap		&map(){ return m; }
	const PrettyObjMap	&map() const { return m; }
protected:
	virtual FILEID_t fileId() const { return FILEID_TEMPL; }
	virtual FileTempl_t *fileTempl() const { return const_cast<FileTempl_t *>(this); }
};


///////////////////////////////////////////////////////////////////

inline void SET_USERFOLDER(TypePtr pType, FolderPtr pFolder)
{
	pType->SetUserData<Folder_t>(pFolder);
}

inline FolderPtr USERFOLDER(CTypePtr p){
	return (FolderPtr)p->UserData<Folder_t>();
}

//dump healper
class SrcDumpFile : protected std::ofstream
{
	MyPath mPath;
public:
	SrcDumpFile(){}
	SrcDumpFile(const MyPath &path)
		: mPath(path)
	{
	}
	bool open()
	{
		if (!is_open())
		{
			MyFile file(mPath);
			if (!file.EnsureDirExists())//may fail
				return false;
			//PrintError() << "Can't cd to " << path.Dir() << std::endl;
			std::ofstream::open(file.Path().c_str());
		}
		return is_open();
	}
	MyString path() const { return mPath.Path(); }
	void setPath(const MyPath &path)
	{
		if (path != mPath)
		{
			//if (is_open())
				close();
			mPath = path;
		}
	}
	std::ostream &os(){ return *this; }
	bool isSet() const { return !mPath.empty(); }
	bool isOpen() const { return is_open(); }
};


