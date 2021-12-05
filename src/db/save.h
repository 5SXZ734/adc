#pragma once

#include <iostream>
#include "qx/MyString.h"
#include "qx/MyMemoryPool.h"
#include "shared/defs.h"
#include "field.h"
#include "info_proj.h"

#define SR_DEBUG	(__DEBUGGING && 1)
#if(SR_DEBUG)
#define SR_OCHECK	this->_CP = os.tellp()
#define SR_ICHECK	this->_CP = is.tellg()
#else
#define SR_OCHECK
#define SR_ICHECK
#endif


const char g_Magic[4] = {'R','E','E','Q'};
const uint32_t g_Version = 7;

#define INDEXTYPE		uint32_t
#define SINDEXTYPE		int32_t
#define INDEXTYPE_HIBIT	INDEXTYPE(1 << (sizeof(uint32_t)*CHAR_BIT-1))
#define CHUNKINDEXTYPE	uint32_t

class GlobalSerializer_t;
class MyFile;
class StubMgr_t;
class FieldMap;
class Simple_t;
//class TypeUnion_t;
class Dc_t;
class TypeCode_t;
class Strucvar_t;
class FilesMgr0_t;
class File_t;
enum FILEID_t;
class FileModule_t;
class FileStubs_t;
class FileTypes_t;
class MemoryMgr_t;
class DataSource_t;
class DataLeech_t;
class FileNames_t;
class FileRes_t;
struct Block_t;

enum SR_Errors
{
	SR_0,
	SR_1,
	SR_INVALID_TYPE_OBJECT,
	SR_INVALID_FILE_OBJECT,
	SR_INVALID_DATA_OBJECT,
	SR_INVALID_PROJECT,
	SR_WRONG_VERSION,
	SR_WRITE_ERROR,
	SR_READ_ERROR,
	SR_STRING_WRITE_ERROR,
	SR_STRING_READ_ERROR,
	SR_RAW_WRITE_ERROR,
	SR_RAW_READ_ERROR,
	SR_BINARY_SIZE_UNMATCHED,
	SR_INVALID_FIELD_ID,
	SR_INVALID_FOLDER_ID,
	SR_INVALID_UNION_FIELD_ID,
	SR_INVALID_OBJREF,
	SR_INVALID_FRONTDLL,
	SR_FAILED_LOAD_BINARY,
	SR_MKDIR_FAILED,
	SR_OPEN_FILE_FAILED,
	SR_FILES_MISMATCH,
	SR_FTYP_MISMATCH,
	SR_INVALID_TYPE_REF,
	SR_INVALID_GLOB_OBJECT,
	SR_RECOVERY_FAIL,
	SR_MALFORMAT,
	SR__TOTAL
};

class BasicSerializer_t
{
#if(SR_DEBUG)
protected:
	static std::streamoff _CP;
#endif
public:
	template <typename T>
	void write(std::ostream &os, T u) const
	{
//CHECK(_CP >= 0x15e82)
//STOP
		os.write((char *)&u, sizeof(u));
		if (os.fail())
			throw (-SR_WRITE_ERROR);
		SR_OCHECK;
	}

	template <typename T>
	void read(std::istream &is, T &u) const
	{
		is.read((char *)&u, sizeof(u));
		if (is.fail())
			throw (-SR_READ_ERROR);
		SR_ICHECK;
	}

	void write(std::ostream &os, const std::string &s) const
	{
		short l((short)s.length());
		os.write((char *)&l, sizeof(l));
		if (l > 0)
			os.write((char *)s.c_str(), l);
		if (os.fail())
			throw (-SR_STRING_WRITE_ERROR);
		SR_OCHECK;
	}

	void read(std::istream &is, std::string &s) const
	{
		short l;
		is.read((char *)&l, sizeof(l));
		if (l > 0)
		{
			s.resize(l);
			is.read((char *)&(*s.begin()), l);
		}
		if (is.fail())
			throw (-SR_STRING_READ_ERROR);
		SR_ICHECK;
	}

	void write(std::ostream &os, const MyString &s) const
	{
		write(os, reinterpret_cast<const std::string &>(s));
		SR_OCHECK;
	}

	void read(std::istream &is, MyString &s) const
	{
		read(is, reinterpret_cast<std::string &>(s));
		SR_ICHECK;
	}

	void write(std::ostream &os, PDATA p, size_t sz) const
	{
		os.write((char *)p, sz);
		if (os.fail())
			throw (-SR_RAW_WRITE_ERROR);
		SR_OCHECK;
	}

	void read(std::istream &is, PDATA p, size_t sz) const
	{
		is.read((char *)p, sz);
		if (is.fail())
			throw (-SR_RAW_READ_ERROR);
		SR_ICHECK;
	}

	void writeEos(std::ostream &os) const
	{
		static INDEXTYPE eos(0);//end of sequence
		write(os, eos);
		SR_OCHECK;
	}

	void writeIdx(std::ostream &os, CHUNKINDEXTYPE n) const
	{
		write(os, n);
	}

	CHUNKINDEXTYPE readIdx(std::istream& is) const
	{
		CHUNKINDEXTYPE n;
		read(is, n);
		return n;
	}
};






class StrucSerializer_t : public BasicSerializer_t
{
public:
	bool	mbLocal;

public:
	StrucSerializer_t();

	bool setLocalFlag(bool b){ bool bLocal(mbLocal);  mbLocal = b; return bLocal; }

	void saveField(std::ostream &, CFieldPtr);
	void loadField(std::istream &, FieldPtr);

	void save(std::ostream &, const Obj_t &);
	void load(std::istream &, Obj_t &);

	void saveNameRef(std::ostream &, CPNameRef);
	void loadNameRef(std::istream &, PNameRef);

	void saveComplex(std::ostream &, CTypePtr);
	void loadComplex(std::istream &, TypePtr);

	void save(std::ostream &, const NamesMgr_t &);
	void load(std::istream &, NamesMgr_t &);

	void saveTypeObj(std::ostream &, CTypePtr);
	void loadTypeObj(std::istream &, TypePtr);

	void	saveTypeMap(std::ostream &, const TypesMgr_t &);
	void	loadTypeMap(std::istream &, TypesMgr_t &);

	void writeType(std::ostream &, CTypePtr);
	void loadType(std::istream &, TypePtr);

	virtual INDEXTYPE fieldTypeToIdx(CTypePtr) const = 0;
	virtual TypePtr fieldTypeFromIdx(INDEXTYPE) const = 0;

	virtual bool writeType(ObjId_t, std::ostream &, CTypePtr);
	virtual Type_t *loadType(ObjId_t, std::istream &, TypePtr);

	void	saveArray(std::ostream &, CTypePtr);
	void	loadArray(std::istream &, TypePtr);

	void	saveTypePtr(std::ostream &, CTypePtr);
	void	loadTypePtr(std::istream &, TypePtr);

	void	saveTypeRef(std::ostream &, CTypePtr);
	void	loadTypeRef(std::istream &, TypePtr);

	void	saveTypeImpPtr(std::ostream &, CTypePtr);
	void	loadTypeImpPtr(std::istream &, TypePtr);

	void	saveTypeExpPtr(std::ostream &, CTypePtr);
	void	loadTypeExpPtr(std::istream &, TypePtr);

	void	saveTypeVPtr(std::ostream &, CTypePtr);
	void	loadTypeVPtr(std::istream &, TypePtr);

	void	saveTypeEnum(std::ostream &, CTypePtr);
	void	loadTypeEnum(std::istream &, TypePtr);

	void	saveTypeConst(std::ostream &, CTypePtr);
	void	loadTypeConst(std::istream &, TypePtr);

	void	saveTypePair(std::ostream &, CTypePtr);
	void	loadTypePair(std::istream &, TypePtr);

	void	saveTypeFunc(std::ostream &, CTypePtr);
	void	loadTypeFunc(std::istream &, TypePtr);

	void	saveTypeArrayIndex(std::ostream &, CTypePtr);
	void	loadTypeArrayIndex(std::istream &, TypePtr);

	void	saveTypedef(std::ostream &, CTypePtr);
	void	loadTypedef(std::istream &, TypePtr);

	void	saveSimple(std::ostream &, CTypePtr);
	void	loadSimple(std::istream &, TypePtr);

	void	saveStruc(std::ostream &, CTypePtr);
	void	loadStruc(std::istream &, TypePtr);

	void	saveStrucvar(std::ostream &, CTypePtr);
	void	loadStrucvar(std::istream &, TypePtr);

	//void	saveBit(std::ostream &, TypePtr);
	//void	loadBit(std::istream &, TypePtr);

	void	saveBitfield(std::ostream &, CTypePtr);
	void	loadBitfield(std::istream &, TypePtr);

	//void	saveUnion(std::ostream &, CTypePtr);
	//void	loadUnion(std::istream &, TypePtr);

public:
	void saveFields(std::ostream &, const FieldMap &);
	FieldPtr loadField(std::istream &, FieldMap &, int kind = 0);//kind = 0:struc, 1:union, 2:strucvar

	virtual INDEXTYPE fieldToIdx(CFieldPtr) const = 0;
	virtual FieldPtr fieldFromIdx(INDEXTYPE) const = 0;

	virtual INDEXTYPE nameToIdx(CPNameRef) const = 0;
	virtual PNameRef nameFromIdx(INDEXTYPE) const = 0;

	virtual INDEXTYPE typeToIdx(CTypePtr) const = 0;
	virtual TypePtr typeFromIdx(INDEXTYPE) const = 0;

	virtual INDEXTYPE fieldRefToIdx(CFieldPtr) const = 0;
	virtual FieldPtr fieldRefFromIdx(INDEXTYPE) const = 0;

};


class GlobalSerializer_t : public StrucSerializer_t
{
	typedef StrucSerializer_t	Base_t;

public:
	Project_t	&mrProject;
	bool		mbImbedRawData;
	MyString	mPath;

public:
	GlobalSerializer_t(Project_t &rProj, MyString path)
		: mrProject(rProj),
		mbImbedRawData(false),
		mPath(path)
	{
	}
	//void setPath(MyString s){ mPath = s; }

	virtual MemoryMgr_t& memMgr() { assert(0); static MemoryMgr_t t; return t; }
	virtual const MemoryMgr_t& memMgr() const { assert(0); static MemoryMgr_t t; return t; }

	virtual void saveProject(std::ostream &, CTypePtr);
	virtual void loadProject(std::istream &, TypePtr);

	virtual void saveModule(std::ostream &, CTypePtr);
	virtual void loadModule(std::istream &, TypePtr);

	void Save(std::ostream &os);
	void Load(std::istream &is);

	//void save(std::ostream&, CFieldRef);
	//void load(std::istream&, FieldRef);

	Project_t &GetProject() const;

	virtual void save(std::ostream&) = 0;
	virtual void load(std::istream&) = 0;

protected:
	//datas
	void	save(std::ostream &, const DataObj_t &);
	void	load(std::istream &, DataObj_t &);

	void	saveDataObj(std::ostream &, const DataObj_t &, DATAID_t);
	void	loadDataObj(std::istream &, DataObj_t &, DATAID_t);

	void saveDataSource(std::ostream &, const DataSource_t &);
	void loadDataSource(std::istream &, DataSource_t &);

	void saveDataLeech(std::ostream &, const DataLeech_t &);
	void loadDataLeech(std::istream &, DataLeech_t &);

	void saveAuxData(std::ostream &, const I_DataSource &);
	void loadAuxData(std::istream &, I_DataSource &);


	//files
	void	save(std::ostream &, const Folder_t &);
	void	load(std::istream &, Folder_t &);

	virtual bool	saveFile(std::ostream &, const Folder_t &, FILEID_t);
	virtual bool	loadFile(std::istream &, Folder_t &, FILEID_t);

	void	saveFileModule(std::ostream &, const Folder_t &);
	void	loadFileModule(std::istream &, Folder_t &);

	void	saveFileFolder(std::ostream &, const Folder_t &);
	void	loadFileFolder(std::istream &, Folder_t &);

	void	saveFileTypes(std::ostream &, const FileTypes_t &);
	void	loadFileTypes(std::istream &, FileTypes_t &);

	void	saveFileNames(std::ostream &, const FileNames_t &);
	void	loadFileNames(std::istream &, FileNames_t &);

	void	saveFileExports(std::ostream &, const FileExports_t &);
	void	loadFileExports(std::istream &, FileExports_t &);

	void	saveFileImports(std::ostream &, const FileImports_t &);
	void	loadFileImports(std::istream &, FileImports_t &);

	void	saveFileRes(std::ostream &, const FileRes_t &);
	void	loadFileRes(std::istream &, FileRes_t &);

	void	saveCode(std::ostream &, CTypePtr);
	void	loadCode(std::istream &, TypePtr);

	void	saveThunk(std::ostream &, CTypePtr);
	void	loadThunk(std::istream &, TypePtr);

	void	saveFunc(std::ostream &, CTypePtr);
	void	loadFunc(std::istream &, TypePtr);

	void	saveRaw(std::ostream &, const Block_t &);
	void	loadRaw(std::istream &, Block_t &);

	void	saveSeg(std::ostream &, CTypePtr);
	void	loadSeg(std::istream &, TypePtr);

	void	saveRangeMgr(std::ostream &, const Module_t &);
	void	loadRangeMgr(std::istream &, Module_t &);

protected:
	virtual bool writeType(ObjId_t, std::ostream &, CTypePtr);
	virtual Type_t *loadType(ObjId_t, std::istream &, TypePtr);

	virtual INDEXTYPE folderToIdx(CFolderPtr) const = 0;
	virtual FolderPtr folderFromIdx(INDEXTYPE) const = 0;

	virtual INDEXTYPE dataToIdx(CDataPtr) const = 0;
	virtual DataPtr dataFromIdx(INDEXTYPE) const = 0;

	virtual INDEXTYPE objToIdx(CObjPtr) const { assert(0); return 0; }
	virtual TypeBasePtr objFromIdx(INDEXTYPE) const { assert(0); return nullptr; }

};


//////////////////////////////////////////////////////ProjectRecoverer_t
class GlobalRecoverer_t : public ProjectInfo_t
{
public:
	GlobalRecoverer_t(Project_t &rProjRef)
		: ProjectInfo_t(rProjRef)
	{
	}
	void recover();//project
	void recoverTypesMgr(std::vector<TypePtr> &, TypesMgr_t &);
	void recoverSeg(std::vector<TypePtr> &, TypePtr scope);
	void recoverStruc(std::vector<TypePtr> &, TypePtr scope);
	/*virtual*/ void recoverField(std::vector<TypePtr> &, FieldRef, TypePtr scope);
	//void recoverFiles(FilesMgr0_t &);
};


#define	SR_Saving	0
#define	SR_Loading	1



