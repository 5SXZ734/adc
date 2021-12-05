#pragma once

#include "info_dc.h"
#include "info_file.h"
#include "db/command.h"

class StubPool_t;

class TypeFomNode0_t : public DcInfo_t//this one does not produces new types
{
	Frame_t	m_ctx;
public:
	TypeFomNode0_t(DcInfo_t &r, const Frame_t &ctx) 
		: DcInfo_t(r),
		m_ctx(ctx)
	{
	}
	TypePtr process(node_t *);
protected:
	TypePtr OnType(const char *name);
};

class TypeFomNode_t : public FileInfo_t
{
	//FieldPtr	mpField;
	const StubPool_t	&mrStubsPool;
	std::list<TypePtr>	m_newTypes;//list of created strucs during process()
	FieldPtr	mpThisPtr;
	//ADDR		m_va;//imported symbol
	bool		mbExporting;
	OFF_t		mRawOff;
	//ExpTypesMap	&mTypeLookup;
public:
	TypeFomNode_t(FileInfo_t &, const StubPool_t &, bool bExporting);//, ExpTypesMap &);
	void setRawOffset(OFF_t o){ mRawOff = o; }
	FieldPtr thisPtr() const { return mpThisPtr; }
	void resetThisPtr(){ mpThisPtr = nullptr; }
	//TypePtr processClass(MyString, int nesting);
	//TypePtr processCompound(MyString, int iNesting);
	std::list<TypePtr>	&newTypes() { return m_newTypes; }
	const std::list<TypePtr> &newTypes() const { return m_newTypes; }
	TypePtr process(node_t *, I_Front::SymbolKind, GlobPtr = nullptr);
	TypePtr process(I_Front::SymbolKind, GlobPtr = nullptr);
	TypePtr ProcessCompound(FullName_t, int iNesting, bool bExporting, E_KIND eClass = E_KIND_STRUC);
	TypePtr ProcessClass(FullName_t, int iNesting, bool bExporting, bool bIsStruc);
	TypePtr ProcessTypedef(FullName_t, TypePtr base);
	//TypePtr	FindStrucInFiles(const MyString &);//slow
	void	ProcessExportedEntry(GlobPtr, node_t *, I_Front::SymbolKind, const MyStringEx &, bool bIsFunc);
	bool	ProcessEntry(GlobPtr, node_t *, I_Front::SymbolKind);
	bool	ProcessExportedEntry(ADDR, node_t *, I_Front::SymbolKind, const MyStringEx&, bool bIsFunc);//export
	bool	ProcessMapEntry(ADDR, node_t *, I_Front::SymbolKind, const MyString &);//map
	bool	ProcessDebugSymbolEntry(ADDR, node_t *, I_Front::SymbolKind, const MyString &, bool bIsFunc);

	void ProcessNewTypes(FileInfo_t &imp);

protected:
	GlobPtr OnFunc(node_t *, GlobPtr);
	TypePtr OnFuncType(node_t *);
	TypePtr OnFuncTypeArgs(TypesTracer_t &, node_t *, unsigned &flags);
	TypePtr OnStruc(node_t *);
	TypePtr OnData(node_t *, I_Front::SymbolKind, GlobPtr);
	TypePtr OnType(const char *){ return nullptr; }
	TypePtr OnVTable(I_Front::SymbolKind, GlobPtr);
};
