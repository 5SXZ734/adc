#pragma once 

#include "db/clean.h"
#include "info_dc.h"
#include "path.h"
#include "info_func.h"

struct RedumpCache_t;

template <typename T = DcInfo_t>
class DcCleaner_t : public BinaryCleaner_t<T>
{
	typedef BinaryCleaner_t<T> BASE;

protected:
	using BASE::PrimeSeg;
	using BASE::memMgr;
	
public:
	template <typename... Args>
	DcCleaner_t(Args&&... args) 
		: BinaryCleaner_t<T>(std::forward<Args>(args)...)
	{
	}

	~DcCleaner_t()
	{
	}

	/*DcCleaner_t(BinaryCleaner_t<DcInfo_t> &r) 
		: BinaryCleaner_t<T>(r)
	{
		setClosing(r.isClosing());
	}

	DcCleaner_t(BinaryCleaner_t<ProjectInfo_t> &r, const DcInfo_t &rDI) 
		: BinaryCleaner_t<T>(rDI)//,
		//mrDI(rDI)
	{
		setClosing(r.isClosing());
	}*/


	//cleanup
	virtual void destroy(FieldRef) override;
	virtual void destroyTypeRef(TypeObj_t &, bool, NamesMgr_t&) override;
	virtual void destroyFileRef(Folder_t &, bool) override;
	void destroyGlob(GlobPtr, bool);
	void destroyIntrinsics();
	void destoyTypeClass(TypePtr);
	//void destoyTypeVFTable(TypePtr);
	void destroyFuncDef(GlobPtr);
	void destroyThunk(GlobPtr);
	void destroyTypeProxy(TypePtr);
	void DestroyGlobObj(GlobPtr);
	virtual void destroyProc(TypePtr, NamesMgr_t&) override;
	virtual void destroyModule(TypePtr) override;
//	void destroyExportedTypesInfo();
	void destroyFolders();
	void DestroyPrettyName(GlobPtr, PNameRef);
	void DestroyPrettyName(TypePtr, PNameRef);
protected:
	void disconnectLocal(FieldPtr, MemoryMgrEx_t &);
	//void disconnectLabel(HPATH, MemoryMgrEx_t&);
	//virtual void destroyConflictField(FieldPtr , NamesMgr_t *);
	Dc_t &dc(){ return BASE::mrDC; }
	bool isClosing() const { return BASE::mbClosing; }
};


class FuncCleaner_t : public DcCleaner_t<FuncInfo_t>
{
	typedef DcCleaner_t<FuncInfo_t> BASE;

	std::list<HOP> mlOrphanOps;
	std::list<HOP> mlOrphanVarOps;
	std::list<HOP> mlOrphanCallOuts;
	std::list<HOP> mlOrphanEntryOps;
	std::list<FieldPtr> mlOrphanArgs;
	std::list<FieldPtr> mlOrphanVars;
	std::list<TypePtr> mlUnrefedArgTypes;//from global ns/mm
public:
	FuncCleaner_t(const FuncInfo_t &);
	~FuncCleaner_t();
	//void UntypeFields(FieldMap &);
	//void ClearOp(HOP, bool bPreserverRI = false);
	void ClearPath(HPATH);
	void DestroyPath(HPATH);
	//void ClearLocals(HPATH);
	void ClearPathRefs(HPATH) const;
	void DisconnectCallOutOp(CHOP);
	void DisconnectOp(CHOP, bool bPreserveCodeFlow);
	void ClearOpList(OpList_t &);
	void DestroyRhs(HOP);
	void DestroyOp(HOP);
	void DestroyPrimeOp(HOP);

	void AdjournPermanentOp(CHOP, RedumpCache_t &);
	void AdjournTransientOp(HOP, RedumpCache_t &);
	void AdjournRhs(HOP, RedumpCache_t &);
	void AdjournRhsOp(HOP, RedumpCache_t &);
	void AdjournCallOutOp(HOP, RedumpCache_t &);
	void AdjournCallOuts(HOP, RedumpCache_t &);

	bool RemoveUnrefedVarOp(HOP);
	bool DeleteUnrefedLocalVar(FieldPtr);//, MemoryMgr_t *);
	bool DeleteUnrefedLocalArg(FieldPtr);
	void ClearArgType(FieldPtr);
	void DetachLocalRef(HOP);
	void DetachLabelRef(HOP);
	HOP TakeOrphanOp();
	FieldPtr TakeOrphanArg();
	FieldPtr TakeOrphanVar();
	FieldPtr TakeLocal(FieldPtr);
	//void destroy(HOP);
	void destroy(Path_t &) {}
	void DestroyBody();
	void DetachArgsRefs();
	void DestroyStrucLoc(NamesMgr_t&);
	void CheckStrucLocsEmpty();

	void Cleanup();
	void CleanupFinal();
	void PurgeCallOuts(RedumpCache_t &);
	void Purge(RedumpCache_t &);//in case of failed processing
	void PurgeFinal(RedumpCache_t&);//after a purge
	void PurgeDanglingVars();
	void PurgeDanglingArgs();

private:
	void destroyUnrefedArgTypes();
	void pushUnrefedArgType(TypePtr p){
//CHECKID(p, 0x574b)
//STOP
		mlUnrefedArgTypes.push_back(p);
	}
	TypePtr popUnrefedArgType(){
		TypePtr p(mlUnrefedArgTypes.front());
		mlUnrefedArgTypes.pop_front();
		return p;
	}
	bool hasUnrefedArgTypes() const { 
		return !mlUnrefedArgTypes.empty(); }
	void cleanupOrphanFields();
	void cleanupOrphanArgs();
	void cleanupOrphanVars();
	MemoryMgrEx_t &memMgrEx() const {
		return reinterpret_cast<MemoryMgrEx_t &>(funcInfo().memMgr());
	}
	FuncInfo_t &funcInfo(){ return *const_cast<FuncCleaner_t *>(this); }
	const FuncInfo_t &funcInfo() const { return *this; }
};

