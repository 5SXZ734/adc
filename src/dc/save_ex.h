#pragma once

#include "db/save.h"
#include "path.h"
#include "mem_ex.h"
#include "globs.h"


struct ProbeEx_t;
class secondary_ofstream;
class FileDef_t;
class FileTempl_t;
class ProjectEx_t;


//////////////////////////////////////////////////// (solid mode)
class FileSerializer_t : public StrucSerializer_t
{
	typedef StrucSerializer_t	Base_t;
	
public:
	FileSerializer_t(){}

	virtual MemoryMgrEx_t& memMgrEx() = 0;

	void saveFunction(std::ostream &, CGlobPtr);//funcDef
	TypePtr loadFunction(std::istream &, FuncDef_t &);

protected:

	void savePath(std::ostream &, HPATH);
	void loadPath(std::istream &, HPATH);

	void saveOp(std::ostream &, HOP);
	void loadOp(std::istream &, HOP, bool bForceOrder);

	void saveInstr(std::ostream &, const Ins_t &);
	void loadInstr(std::istream &, Ins_t &);

	virtual bool writeType(ObjId_t, std::ostream &, CTypePtr);
	virtual Type_t *loadType(ObjId_t, std::istream &, TypePtr);

	virtual INDEXTYPE opToIdx(OpPtr) const = 0;
	virtual HOP opFromIdx(INDEXTYPE) const = 0;

	virtual INDEXTYPE pathToIdx(PathPtr) const = 0;
	virtual PathPtr pathFromIdx(INDEXTYPE) const = 0;
};


//////////////////////////////////////////////////// (FileSerializerEx_t)
class FileSerializerEx_t : public FileSerializer_t//for dispersed mode
{
	typedef FileSerializer_t	Base_t;

protected:
	unique_vector<CTypePtr> mDeferredTypes;//for dispersed mode

public:
	FileSerializerEx_t()
		: mDeferredTypes(1)//1-biased
	{
	}
	SINDEXTYPE addDifferedRef(CTypePtr p){
		return (SINDEXTYPE)mDeferredTypes.add(p);
	}

	bool isDispersedMode() const { return true; }

	//size_t addUV(TypePtr p){ return muv.add(p); }
	unique_vector<CTypePtr> &deferredTypes(){ return mDeferredTypes; }
	const unique_vector<CTypePtr> &deferredTypes() const { return mDeferredTypes; }
	void setDeferredTypes(std::vector<CTypePtr> &v);
};


//////////////////////////////////////////////////// (FuncSerializerEx_t)
class FuncSerializer_t : public FileSerializer_t
{
	typedef FileSerializer_t	Base_t;

public:
	FuncSerializer_t()
	{
	}
};



//////////////////////////////////////////////////// (GlobalSerializerEx_t)
class GlobalSerializerEx_t : public GlobalSerializer_t
{
	typedef GlobalSerializer_t	Base_t;
//	bool	mbDisperseMode;
public:
	GlobalSerializerEx_t(Project_t &rProj, MyString path)
		: Base_t(rProj, path)//,
	//	mbDisperseMode(disperseMode)
	{
	}
	ProjectEx_t &GetProjectEx() const;

	enum SaveMode { SaveMode_None, SaveMode_Solid, SaveMode_Dispersed };
	void SaveEx(std::ostream &os, SaveMode);
	void LoadEx(std::istream &is, bool quick);

#if(FUNC_SAVE_ENABLED)
	bool LoadDispersed(MyString sFile, Folder_t &, Dc_t &);
#endif

protected:
	void	saveTypeClass(std::ostream &, CTypePtr);
	void	loadTypeClass(std::istream &, TypePtr);

	//void	saveTypeVFTable(std::ostream &, CTypePtr);
	//void	loadTypeVFTable(std::istream &, TypePtr);

	void	saveDC(std::ostream &, const Dc_t &);
	Dc_t *	loadDC(std::istream &);

	void	saveFuncDef(std::ostream &, CTypePtr);
	void	loadFuncDef(std::istream &, TypePtr);

	void	saveProxy(std::ostream &, CTypePtr);
	void	loadProxy(std::istream &, TypePtr);

	//void	savef(std::ostream &, const FileDef_t &, secondary_ofstream *);
	//void	loadf(std::istream &, FileDef_t &, Dc_t &);

	void	saveSolid(std::ostream &, const Folder_t &, const Dc_t &);
	void	loadSolid(std::istream &, Folder_t &, Dc_t &);

#if(FUNC_SAVE_ENABLED)
	bool	saveDispersed(std::ostream &os, CFolderRef, const Dc_t &, std::set<std::string> &usedFileNames);
	void	loadDispersed(std::istream &, FolderRef, Dc_t &);

	void saveDeferredTypeRefs(std::ostream &, const std::vector<CTypePtr> &);
	void loadDeferredTypeRefs(std::istream &, std::vector<CTypePtr> &);
#endif

	//void	save(std::ostream &, const Folder_t &, const Dc_t &);
	//void	load(std::istream &, Folder_t &, Dc_t &);

	void	saveGlob(std::ostream &, CGlobPtr);
	void	loadGlob(std::istream &is, GlobPtr);

	void	saveFileDef(std::ostream &, const FileDef_t &);
	void	loadFileDef(std::istream &, FileDef_t &);

	void	saveFileTempl(std::ostream &, const FileTempl_t &);
	void	loadFileTempl(std::istream &, FileTempl_t &);

	void	saveFileStubs(std::ostream &, const FileStubs_t &);
	void	loadFileStubs(std::istream &, FileStubs_t &);


protected:
	virtual bool writeType(ObjId_t, std::ostream &, CTypePtr);
	virtual Type_t *loadType(ObjId_t, std::istream &, TypePtr);

	virtual bool	saveFile(std::ostream &, const Folder_t &, FILEID_t);
	virtual bool	loadFile(std::istream &, Folder_t &, FILEID_t);

	virtual INDEXTYPE globIdx(CGlobPtr) { return 0; }
};



class GlobalRecovererEx_t : public GlobalRecoverer_t
{
	//Dc_t	*mpDC;
public:
	GlobalRecovererEx_t(Project_t &rProjRef)
		: GlobalRecoverer_t(rProjRef)//,
		//mpDC(nullptr)
	{
	}
	//void setDC(Dc_t *pDC){ mpDC = pDC; }

	void recoverDC(Dc_t &);
	void recoverFunc(std::vector<TypePtr> &, GlobPtr funcDef, TypePtr locals, ADDR dockVA);

//protected:
	//virtual void recoverField(FieldRef, NamesMgr_t *);

private:
	void recoverArgs(OpList_t &, HOP);
	void recoverCallRets(OpList_t &);
};


