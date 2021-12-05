#pragma once

#include "type_module.h"
#include "types.h"

/*class NamesMgr_t;
class FieldMap;
class Dc_t;
class TypeCode_t;
class TypeUnion_t;
class Simple_t;
class Seg_t;
//class VarArray_t;
class Strucvar_t;
class MemoryMgrEx_t;

*/

template <typename T = ProjectInfo_t>
class BinaryCleaner_t : public T//ModuleInfo_t
{
protected:
	using T::memMgr;
	using T::mrProject;
protected:
	bool mbClosing;
	std::list<TypePtr>	mUnrefedTypes;
	//std::list<TypesMgr_t *>	mUnrefedTypemaps;
public:
	//BinaryCleaner_t(Project_t &);
	template <typename... Args>
	BinaryCleaner_t(Args&&... args)
		: T(std::forward<Args>(args)...),
		mbClosing(false)
	{
	}
	//BinaryCleaner_t(const T &, MemoryMgr_t &);
	//BinaryCleaner_t(const ModuleInfo_t &);

	//BinaryCleaner_t(ModuleInfo_t &);
	//BinaryCleaner_t(ProjectInfo_t &, TypePtr iModule);
	~BinaryCleaner_t();

	void setClosing(bool b){ mbClosing = b; }
	bool isClosing() const { return mbClosing; }

	void destroyUnrefedTypes();//?BinaryCleaner_t &);
	void clearUnrefedTypes(){
		mUnrefedTypes.clear();
	}
	void pushUnrefedType(TypePtr p){
CHECKID(p, 0x27ac)
STOP
		mUnrefedTypes.push_back(p);
	}
	TypePtr popUnrefedType(){
		TypePtr p(mUnrefedTypes.front());
		mUnrefedTypes.pop_front();
		return p;
	}
	bool hasUnrefedTypes() const { 
		return !mUnrefedTypes.empty(); }

	//cleanup
	void destroyUserTypes(TypesMgr_t &, NamesMgr_t&);
	void destroyUserTypesNS(TypesMgr_t &);
	void destroy(TypesMgr_t &);
	void destroyTypePtr(TypePtr);
	void destroyTypeImpPtr(TypePtr);
	void destroyTypedef(TypePtr);
	void destroySimple(Simple_t &) {}
	void destroyArray(TypePtr);
	///
	virtual void destroy(FieldRef);
	virtual void destroyTypeRef(TypeObj_t &, bool, NamesMgr_t&);
	virtual void destroyFileRef(Folder_t &, bool);
	virtual void destroyModule(TypePtr);
	//void destroyExtraFields(TypePtr, NamesMgr_t&);
	void destroySeg(TypePtr, NamesMgr_t&);
	virtual void destroyProc(TypePtr, NamesMgr_t&);
	void destroyCode(TypePtr, NamesMgr_t&);
	void destroyTypeStruc(TypePtr, NamesMgr_t&);
	void destroyUnion(TypePtr, NamesMgr_t&);
	void destroyEnum(TypePtr);
	void destroyArrayIndex(TypePtr);
	void destroyTypeImp(TypePtr);
	void destroyTypeExp(TypePtr);
	void destroyTypeConst(TypePtr);
	void destroyTypeThunk(TypePtr);
	void destroyTypePair(TypePtr);
	void destroyTypeFunc(TypePtr);
	void destroy(Type_t &) {}
	void destroyCompound(TypePtr, NamesMgr_t&);
	void destroyStrucvar(TypePtr, NamesMgr_t&);
	//void destroyBit(TypePtr) {}
	void destroyBitset(TypePtr, NamesMgr_t&);

	void destroyDataSource(DataSource_t &);
	void destroyDataLeech(DataLeech_t &);
	void destroyDataObj(DataObj_t &);
	void destroyBinaryData(Module_t &);
	void ReleaseDataRef(TypePtr);//mod

	void ClearType(FieldPtr);

	bool ReleaseTypeRef0(TypePtr, bool bUnlink);
	bool ReleaseTypeRef(TypePtr);
	void DestroyTypeRef(TypePtr);
	
	bool DestroyFolders();
	void DestroyFolder(FolderPtr);

	bool UnregisterSubseg(TypePtr seg);
	bool DeleteRangeSeg(TypePtr superSeg, TypePtr iSeg);

	void destroyStockTypes();
	void destroyProject();
	//void destroyTypesNS();

	//void clearFields(FieldMap &);
	void clearFields(FieldMap &, NamesMgr_t *);
	void destroyField(FieldPtr , NamesMgr_t *);

protected:
	void destroyField(TypePtr, FieldPtr );
	//virtual void destroyConflictField(FieldPtr , NamesMgr_t *);
};


template <typename T = ModuleInfo_t>
class ModuleCleaner_t : public BinaryCleaner_t<T>
{
public:
	ModuleCleaner_t(const ModuleInfo_t &);
	//ModuleCleaner_t(const ProjectInfo_t &, TypePtr);
	void destroyModule();
};
