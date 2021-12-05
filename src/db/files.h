#pragma once

#include <set>
#include "shared/avl_tree2.h"
#include "qx/MyPath.h"
#include "qx/MyString.h"
#include "qx/MyStream.h"
#include "qx/MyMemoryPool.h"
#include "shared/tree.h"
#include "shared/misc.h"
#include "shared/sbtree2.h"
#include "mem.h"
//#include "obj.h"
#include "shared/obj_id.h"
#include "interface/IADCGui.h"

/*class MyFileName : public MyString
{
	MyString &base(){ return *this; }
public:
	MyFileName(){}
	MyFileName(const MyString &s) : MyString(s){}
	bool IsFolder() const { return endsWith(FOLDER_SEP); }
	bool IsModule() const { return endsWith(MODULE_SEP); }
	const MyString &asMyString() const { return *this; }
	bool operator<(const MyFileName &o)
	{
		if (IsFolder())
		{
			if (!o.IsFolder())
				return true;//foders go before files
		}
		else
		{
			if (o.IsFolder())
				return false;
			//both files (or binaries)
		}
		return (base() < o);
	}

};*/


class ZPath_t : public std::list<MyString>
{
public:
	ZPath_t(const MyString &str)
	{
		init(str);
	}
	void init(const MyString &str)
	{
		MyString sep(MODULE_SEP "\\/");

		int j = 0;
		int i = str.findAnyOf(sep, j);
		int k(0);//to deal with paths like this: ../../blah/blah
		sep = "\\/";

		while (i != -1)
		{
			if (i > j && i <= (int)str.length())
			{
				MyString s(str.substr(j, i - j));
				if (s != ".")
				{
					if (s == "..")
					{
						if (!empty())
							pop_back();
						else
							k++;
					}
					else
						add(s);
					if (!empty())
					{
						if (str.substr(i, 1) == MODULE_SEP)
							back().append(MODULE_SEP);
						else
							back().append(FOLDER_SEP);
					}
				}
			}
			j = (int)(i + 1);
			i = str.findAnyOf(sep, j);
		}

		int l((int)str.length() - 1);
		if (str.substr(j, l - j + 1).length() > 0)
			add(str.substr(j, l - j + 1));
		//else
			//push_back("*");//a directory
		while (k--)
			push_front("__" FOLDER_SEP);
	}
	MyString toString() const //const char *sep = "/") const
	{
		MyString s;
		for (const_iterator i(cbegin()); i != cend(); i++)
		{
			/*const_iterator j(i);
			j++;//next
			if (i == cbegin() && (*i).endsWith(MODULE_SEP))
				s.append(*i);
			else if (j != cend())
				s.append((*i) + sep);
			else if ((*i) != "*")*/
				s.append((*i));
		}
		return s;
	}
	MyString root() const
	{
		assert(isAbsolute());
		return front();
	}
	bool isAbsolute() const
	{
		return !empty() && front().endsWith(MODULE_SEP);
	}
	bool isFile() const
	{
		return !empty() && !(back().endsWith(MODULE_SEP) || back().endsWith(FOLDER_SEP));
	}
	MyString take_front()
	{
		MyString s(front());
		pop_front();
		return s;
	}
	void removeExt()//remove extention
	{
		int n(back().find('.'));
		if (n >= 0)
		{
			back().truncate(n);
			if (back().empty())
				back().assign("?");
		}
	}
	static MyString moduleOf(const MyString& s0)
	{
		int n(s0.find(MODULE_SEP));
		if (n > 0)
			return s0.left(n);
		return MyString();
	}
private:
	void add(const MyString &s)
	{
		push_back(s);
	}
};


#define FIL_TYPE		0xF000	//mask for file types

enum FTYP_Enum 
{
	FTYP_FOLDER = -1,
	//FTYP_HEADER,	//
	FTYP_SOURCE = 0,	//regular source file (implementation + declaration)
	FTYP_STUBS,		//stubs go here
	FTYP_PREFIX,
	FTYP__CONST,
	FTYP_TEMPLATES,
	FTYP_RESOURCES,
	FTYP_TYPES,
	FTYP_PROTOTYPES,
	FTYP_NAMES,
	FTYP_EXPORTS,
	FTYP_IMPORTS,
	FTYP__TOTAL
};

#define FPATH_FROM_IMPORTED	"from_imports"
#define	FPATH_FROM_EXPORTED	"from_exports"
#define FPATH_FROM_DBG	"from_symbols"
#define FPATH_FROM_MAP "from_map"

//#define ADC_DIR			".tmp/"
#define ADC_DIR			"./"

inline const char *FTYP2name(FTYP_Enum e)
{
	switch (e)
	{
	case FTYP_PREFIX: return "prefix";
	case FTYP_STUBS: return "stubs";
	//case FTYP_CONST: return "constants";
	case FTYP_TEMPLATES:
	case FTYP_TYPES:
	case FTYP_NAMES:
	case FTYP_EXPORTS:
	case FTYP_IMPORTS:
	case FTYP_RESOURCES:
	case FTYP_PROTOTYPES:
		return "";
	default:
		break;
	}
	assert(0);
	return 0;
}

inline MyString FolderType2Ext(FTYP_Enum e)
{
	switch (e)
	{
	case FTYP_RESOURCES: return RESOURCE_EXT;
	case FTYP_TYPES: return TYPES_EXT;
	case FTYP_NAMES: return NAMES_EXT;
	case FTYP_EXPORTS: return EXPORTS_EXT;
	case FTYP_IMPORTS: return IMPORTS_EXT;
	case FTYP_TEMPLATES: return TEMPLATES_EXT;
	case FTYP_PROTOTYPES: return STUBS_EXT;
	default: break;
	}
	assert(0);
	return 0;
}

class FileDef_t;
class FileTypes_t;
class FileNames_t;
class FileModule_t;
class FileFolder_t;
class FileStubs_t;
class FileRes_t;
class FileTempl_t;
class FileExports_t;
class FileImports_t;

class File_t
{
public:
	File_t()
	{
	}
	virtual ~File_t(){}
	virtual bool isEmpty() const { return false; }
	virtual FILEID_t fileId() const { return FILEID_NULL; }
	virtual FileDef_t *fileDef() const { return nullptr; }
	virtual FileTypes_t *fileTypes() const { return nullptr; }
	virtual FileNames_t *fileNames() const { return nullptr; }
	virtual FileRes_t *fileRes() const { return nullptr; }
	virtual FileStubs_t *fileStubs() const { return nullptr; }
	virtual FileModule_t *fileModule() const { return nullptr; }
	virtual FileFolder_t *fileFolder() const { return nullptr; }
	virtual FileTempl_t *fileTempl() const { return nullptr; }
	virtual FileExports_t *fileExports() const { return nullptr; }
	virtual FileImports_t *fileImports() const { return nullptr; }
};

////////////////////////////////////////FileResources_t
class FileRes_t : public File_t
{
public:
	struct res_info
	{
		MyString name;
		ADDR va;
		adcui::FolderTypeEnum	eType;
		res_info() : va(0), eType(adcui::FOLDERTYPE_UNK){}
		//res_info(MyString s, ADDR a) : name(s), va(a){}
	};

	typedef my::tree_t<res_info>	tree_t;
	typedef my::tree_builder<res_info>	tree_builder_t;
	typedef tree_t::tree_node_t	tree_node_t;

private:
	tree_t	m_tree;

public:
	FileRes_t()
	{
	}
	virtual ~FileRes_t()
	{
	}
	void reset()
	{
		m_tree.reset(nullptr);
	}
	tree_t &tree(){ return m_tree; }

protected:
	virtual FILEID_t fileId() const { return FILEID_RESOURCES; }
	virtual FileRes_t *fileRes() const { return const_cast<FileRes_t *>(this); }
	virtual bool isEmpty() const { return m_tree.empty(); }
};

////////////////////////////////////////FileTypes_t
class FileTypes_t : public File_t
{
	std::set<TypePtr>	mTypesMapOwners;//all (modules) typesmap owners must be registered here

public:
	FileTypes_t(){}
	virtual ~FileTypes_t()
	{
		assert(mTypesMapOwners.empty());
	}
	std::set<TypePtr> &typeMaps(){ return mTypesMapOwners; }
	const std::set<TypePtr> &typeMaps() const { return mTypesMapOwners; }
protected:
	virtual FILEID_t fileId() const { return FILEID_TYPES; }
	virtual FileTypes_t *fileTypes() const { return const_cast<FileTypes_t *>(this); }
	virtual bool isEmpty() const { return mTypesMapOwners.empty(); }
};


////////////////////////////////////////FileTypes_t
class FileNames_t : public File_t
{
	NamesMgr_t *mpNS;
public:
	FileNames_t(NamesMgr_t *p) : mpNS(p){}
	virtual ~FileNames_t()
	{
	}
	void setNamesMap(NamesMgr_t *p){ mpNS = p; }
	NamesMgr_t *namesMgr() const { return mpNS; }
protected:
	virtual FILEID_t fileId() const { return FILEID_NAMES; }
	virtual FileNames_t *fileNames() const { return const_cast<FileNames_t *>(this); }
	virtual bool isEmpty() const { return true; }
};


typedef sbtree::bimap<unsigned short, Field_t*>::impl_s	ExportMap_t;

////////////////////////////////////////FileExports_t
class FileExports_t : public FileNames_t
{
	int mCount;//if this falls t 0 - the file should be removed
public:
	FileExports_t(NamesMgr_t *p) : FileNames_t(p), mCount(0){}
	virtual ~FileExports_t(){}
	bool count(bool b) {
		if (b)
		{
			++mCount;
			return true;
		}
		assert(mCount > 0);
		--mCount;
		return mCount > 0;
	}
protected:
	virtual FILEID_t fileId() const { return FILEID_EXPORTS; }
	virtual FileExports_t *fileExports() const { return const_cast<FileExports_t *>(this); }
	virtual bool isEmpty() const { return mCount == 0; }
};

////////////////////////////////////////FileImports_t
class FileImports_t : public FileExports_t
{
public:
	FileImports_t(NamesMgr_t *p) : FileExports_t(p){}
	virtual ~FileImports_t(){}
protected:
	virtual FILEID_t fileId() const { return FILEID_IMPORTS; }
	virtual FileImports_t *fileImports() const { return const_cast<FileImports_t *>(this); }
};



class MyFolderKey : public MyString
{
public:
	MyFolderKey()
	{
	}
	MyFolderKey(const char *pc)
		: MyString(pc)
	{
	}
	MyFolderKey(const MyString &s)
		: MyString(s)
	{
	}
	//bool operator<(const MyFolderKey &o) const
	//{
		//return *this < o;
	//}
};

class Folder_t : public AVLTreeNode<Folder_t, MyFolderKey>
{
	typedef AVLTreeNode<Folder_t, MyFolderKey>	Base;
public:
	uint32_t	m_nFlags;

#if(!NO_FILE_ID)
	int		mID;
	int		ID() const { return mID; }
	static int	sID;
	static void resetUniqueId(){ sID = 0; }
#endif

	int		m_nRefCount;

public:
	FolderPtr	mpOwner;
	//MyFileName	mName;
	MyString	mDesc;
	//TypePtr		miBinary;

	File_t		*mp;

public:
	Folder_t();
	Folder_t(const MyFolderKey &);
	~Folder_t();

	FILEID_t fileId() const { return mp->fileId(); }

	File_t		*file() const { return mp; }
	FileDef_t *fileDef() const { return mp->fileDef(); }
	FileTypes_t *fileTypes() const { return mp->fileTypes(); }
	FileNames_t *fileNames() const { return mp->fileNames(); }
	FileExports_t *fileExports() const { return mp->fileExports(); }
	FileImports_t *fileImports() const { return mp->fileImports(); }
	FileStubs_t *fileStubs() const { return mp->fileStubs(); }
	FileModule_t *fileModule() const { return mp->fileModule(); }
	FileFolder_t *fileFolder() const { return mp->fileFolder(); }
	FileRes_t *fileRes() const { return mp->fileRes(); }
	FileTempl_t *fileTempl() const { return mp->fileTempl(); }

	int compare(const MyFolderKey &o) const
	{
		const MyFolderKey &a(_key());
		if (a.back() == '/')
		{
			if (o.back() != '/')
				return -1;
		}
		else if (o.back() == '/')
			return 1;
		//return strcmp(a.c_str(), o.c_str());

		const char* pa(a.c_str());
		const char* pb(o.c_str());

#ifdef WIN32
		int res(stricmp(pa, pb));
#else
		int res(strcasecmp(pa, pb));
#endif
		if (res == 0)
			res = strcmp(pa, pb);

		return res;
	}

	int compare_ncase(const MyFolderKey &o) const
	{
		const MyFolderKey &a(_key());
		if (a.back() == '/')
		{
			if (o.back() != '/')
				return -1;
		}
		else if (o.back() == '/')
			return 1;

		const char* pa(a.c_str());
		const char* pb(o.c_str());

#ifdef WIN32
		return stricmp(pa, pb);
#else
		return strcasecmp(pa, pb);
#endif
	}


	void SetPvt0(File_t *p){ mp = p; }

	template <typename T>
	T *SetPvt()
	{
		assert(!mp);
		mp = new T();
		return mp;
	}
	template <typename T>
	T *SetPvt(T *p)
	{
//CHECKID(this,0x20)
//STOP
		assert(!mp);
		mp = p;
		return p;
	}
	void ClearPvt()
	{
		delete mp;
		mp = nullptr;
	}
	bool hasPvt() const { return mp != nullptr; }
	File_t &pvt() const { return *mp; }

	FolderPtr Parent() const { return mpOwner; }
	void setParent(FolderPtr p){ mpOwner = p; }

	const MyFolderKey &name() const { return _key(); }
	int	setName(const MyFolderKey &);
	//void	getName(MyFolderKey &) const;
	//MyString	theName() const { MyFolderKey s; getName(s); return s; }

	//TypePtr		module() const { return miBinary; }
	//void		setModule(TypePtr p){ miBinary = p; }

	int		setDesc(const char *szDesc);
	int		getDesc(char *szDesc) const;
	bool	isNamedByUser() const;

/*	int		Type() const { 
			return (m_nFlags & 0xF000) >> 12; }
	void	SetType(int t){
			m_nFlags &= ~0xF000; 
			m_nFlags |= (t & 0xF) << 12; }*/
};

//typedef sbtree_multimap<FolderPtr, NameElt_t, NamesMapCompare>	FoldersMap;
typedef AVLTree<Folder_t>	FoldersMap;

///////////////////////////////////////////FileFolder_t
class FileFolder_t : public File_t
{
	FoldersMap	mChildren;
public:
	FileFolder_t()
	{
	}
	FoldersMap &children(){ return mChildren; }
	const FoldersMap &children() const { return mChildren; }
	bool empty() const { return mChildren.empty(); }
protected:
	virtual FILEID_t fileId() const { return FILEID_FOLDER; }
	virtual FileFolder_t *fileFolder() const { return const_cast<FileFolder_t *>(this); }
};

////////////////////////////////////////FileModule_t
class FileModule_t : public FileFolder_t
{
	TypePtr mpModule;
	std::vector<FolderPtr>	m_folders;//keep track of special files
	friend class GlobalSerializer_t;
public:
	FileModule_t(TypePtr pModule)
		: mpModule(pModule),
		m_folders(FTYP__TOTAL)
	{
	}
	std::vector<FolderPtr> &specials(){ return m_folders; }
	const std::vector<FolderPtr> &specials() const { return m_folders; }

	FolderPtr	special(size_t n) const { return m_folders[n]; }
	void setSpecial(size_t i, FolderPtr p){ m_folders[i] = p; }

	TypePtr module() const { return mpModule; }

protected:
	virtual FILEID_t fileId() const { return FILEID_MODULE; }
	virtual FileModule_t *fileModule() const { return const_cast<FileModule_t *>(this); }
private:
	FileModule_t()//for serialization
		: mpModule(nullptr),
		m_folders(FTYP__TOTAL)
	{
	}
	void setModule0(TypePtr p){ mpModule = p; }
};


#define	NAMEDEFAULT_DIR	"folder"
#define	NAMEDEFAULT_FILE "file"

//typedef	TreeNode_t<Folder0_t>	Folder_t;
//typedef	Tree_t<Folder0_t>	FileTree_t;

typedef Folder_t *	FolderPtr;
typedef const Folder_t *	CFolderPtr;

/////////////////////////////////////////////////////FilesMgr0_t
class FilesMgr0_t
{
public:
	//FileTree_t mRootFolder;
	FolderPtr		mpRootFolder;

public:
	FilesMgr0_t()
	: mpRootFolder(nullptr)
	{
	}
	~FilesMgr0_t()
	{
		assert(!mpRootFolder);
	}

	static void fixPath(MyString &);
	static MyString realPath(CFolderPtr, bool bWithModule = false);
	static MyString relPath(CFolderPtr, CFolderPtr from = nullptr);
	static MyString path(CFolderPtr);
	bool empty() const;
	FolderPtr rootFolder() const { return mpRootFolder; }
	void setRootFolder(FolderPtr p){ mpRootFolder = p; }
	MyString GetRootDirectory();

	FolderPtr FindFileByPath(MyString, adcui::FolderTypeEnum &iKind);//with extension
	FolderPtr FindFileByStem(MyString);
	//bool	IsMineFile(Fil e_t * pFile);

	static bool IsFolder(const Folder_t &);
	static bool IsModule(const Folder_t &);
	//bool IsFolderLessThan(Folder_t &, const MyFileName &, bool);

	//////////////////////////////

	struct folders_map_elt
	{
		const FoldersMap &mr;
		FoldersMap::const_iterator mi;
		folders_map_elt(CFolderPtr p)
			: mr(p->fileFolder()->children()),
			mi(mr.begin())
		{
		}
	};
	class FolderIterator : public std::list<folders_map_elt>
	{
	public:
		FolderIterator(const FilesMgr0_t &r)
		{
			push_back(folders_map_elt(r.rootFolder()));
		}
		FolderIterator(CFolderPtr p)
		{
			push_back(folders_map_elt(p));
		}
		operator bool() const { return !empty(); }
		FolderIterator& operator ++()
		{
			CFolderRef p(*back().mi);
			if (p.fileFolder())
			{
				if (!p.fileFolder()->empty())
				{
					push_back(folders_map_elt(&p));
					return *this;
				}
			}
			while (++back().mi == back().mr.end())
			{
				pop_back();
				if (empty())
					break;
			}
			return *this;
		}
		FolderIterator& operator ++(int) { return operator++(); }
		CFolderRef operator*(){ return *(back().mi); }
	};
};


MyString FolderType2Ext(adcui::FolderTypeEnum);
adcui::FolderTypeEnum Ext2FolderType(const MyString &);


