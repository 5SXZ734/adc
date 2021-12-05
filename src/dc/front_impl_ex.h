#pragma once

#include "db/front_impl.h"

class StubPool_t;

class FrontImplSymDump_t : public FrontImplBase_t//for symbol dumping
{
protected:
	Dc_t &mrDC;
	FolderPtr mpFolder;
	//std::map<uint64_t, TypePtr>	mIdMap;
public:
	FrontImplSymDump_t(Dc_t&, FolderPtr);// , CFieldPtr = nullptr);
	void PushScope(TypePtr, ADDR);
protected:
	template<typename> friend class IFront_Src;
	friend class IFront_SymDump;
	//in: I_DataSourceBase
	size_t dataAt(OFF_t rawOffs, OFF_t size, PDATA pDest) const { return mrModule.dataAt(rawOffs, size, pDest); }
	OFF_t size() const { return mrModule.rawBlock().m_size; }
	//in: I_Module
	void selectFile(const char *path, const char* folder);
	TypePtr NewScope(const char *pTypeName, SCOPE_enum, AttrScopeEnum);
	TypePtr NewScope(FieldPtr, SCOPE_enum, AttrScopeEnum);
	TypePtr declTypedef(const char *, TypePtr);
	TypePtr type(HNAME name);//no throw
	FieldPtr	declField(HNAME name, TypePtr type, AttrIdEnum attr, ADDR at, I_Module&);
	FieldPtr	declUField(HNAME name, TypePtr type, AttrIdEnum);
	FieldPtr	declUField(HNAME name, SCOPE_enum eScope, AttrIdEnum);

	void dumpImpl(ADDR va, OFF_t oSymbolName, unsigned uNameMax, unsigned uFlags, const StubPool_t&);
	void dumpImpl(ADDR va, const char *pSymbolName, unsigned uFlags, const StubPool_t&);

private:
	TypePtr newScope(const char* pTypeName, SCOPE_enum eScope, AttrScopeEnum);
	FieldPtr declFuncField(MyString filedName, TypePtr, AttrIdEnum attr, ADDR at);
};



template <typename T>
class IFront_Src : public IFront_Base<T>
{
	FrontImplSymDump_t& mr;
public:
	IFront_Src(FrontImplSymDump_t& r)
		: IFront_Base<T>(r),
		mr(r)
	{
	}
protected:
	//in: I_DataSourceBase
	virtual size_t dataAt(OFF_t rawOffs, OFF_t size, PDATA pDest) const override { return mr.dataAt(rawOffs, size, pDest); }
	virtual OFF_t size() const override { return mr.size(); }
	//in: I_Module
	virtual void selectFile(const char* path, const char* folder) override { return mr.selectFile(path, folder); }
	virtual HTYPE NewScope(const char *pTypeName, SCOPE_enum e, AttrScopeEnum a) override { return mr.NewScope(pTypeName, e, a); }
	virtual HTYPE declTypedef(const char *pc, HTYPE t) override { return mr.declTypedef(pc, (TypePtr)t.pvt); }
	virtual HTYPE type(HNAME name) override { return mr.type(name); }
	virtual HFIELD	declField(HNAME name, HTYPE type, AttrIdEnum attr, ADDR at) override { return mr.declField(name, (TypePtr)type.pvt, attr, at, *this); }
	virtual HFIELD	declField(HNAME name, AttrIdEnum attr, ADDR at) override { return mr.declField(name, nullptr, attr, at, *this); }
	virtual HFIELD	declUField(HNAME name, HTYPE type, AttrIdEnum attr) override { return mr.declUField(name, (TypePtr)type.pvt, attr); }
	virtual HFIELD	declUField(HNAME name, AttrIdEnum attr) override { return mr.declUField(name, nullptr, attr); }

	virtual I_Front* frontend() const { assert(0); return nullptr; }
};



