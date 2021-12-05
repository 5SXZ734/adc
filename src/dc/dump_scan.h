#pragma once

#include "globs.h"
#include "op.h"
#include "dump_func.h"

class IDumpScanner_t
{
public:
	int scan(const char*);
protected:
	virtual void OnLineEnd() {}
	virtual void OnTab() {}
	virtual void OnColor(int c) {}
	virtual void OnWString(const wchar_t*, uint16_t) {}
	//virtual void OnFont(int f){}
	virtual void OnFuncDecl(CGlobPtr) {}
	virtual void OnFuncDefinition(CGlobPtr) {}
	virtual void OnStubInfo(CGlobPtr) {}
	virtual void OnFieldDecl(CFieldPtr, bool = false) {}
	virtual void OnFieldDef(CFieldPtr) {}
	virtual void OnGlobDecl(CGlobPtr) {}
	virtual void OnGlobDef(CGlobPtr) {}
	//virtual void OnFieldInst(CFieldPtr){}
	virtual void OnFieldRef(CFieldPtr) {}
	virtual void OnConstRef(CFieldPtr) {}
	virtual void OnFieldGap(CFieldPtr) {}
	virtual void OnImpFieldRef(CFieldPtr) {}
	virtual void OnOp(OpPtr) {}
	virtual void OnLabelDecl(PathPtr) {}
	virtual void OnTypeRef(CTypePtr, bool) {}
	virtual void OnStrucDecl(CTypePtr, bool) {}
	virtual void OnStrucEnd(CTypePtr) {}
	virtual void OnImpGlobDecl(CGlobPtr) {}
	virtual void OnImpClsGlob(CGlobPtr) {}
	virtual void OnVFuncDecl(CGlobPtr, int) {}
	virtual void OnVTableDecl(CGlobPtr, int) {}
	virtual void OnImpVTableDecl(CGlobPtr, int) {}
	//virtual void OnExtClsField(const ExpFieldInfo_t &){}
	virtual void OnDefault(char c) {}
};

template <typename T_Dumper>
class DumpScanner_t : public IDumpScanner_t
{
//protected:
public:
	T_Dumper& mr;
public:
	DumpScanner_t(T_Dumper& r) : mr(r) {}
	virtual void OnLineEnd() { mr.dumpChar('\0'); }
	virtual void OnTab() { mr.dumpTab(); }
	virtual void OnWString(const wchar_t* wc, uint16_t l) { mr.dumpWStr(wc, l); }
	virtual void OnColor(int c) { mr.PushColor(c); }
	//virtual void OnFont(int f){ mr.PushFont(f); }
	virtual void OnDefault(char c)
	{
		if ((int)c < 0x20)
			mr.dumpStr("<ERROR>");
		else
			mr.dumpChar(c);
	}
};

template <typename T_Dumper>
class FileScanner_t : public DumpScanner_t<T_Dumper>
{
	typedef DumpScanner_t<T_Dumper> BASE;
protected:
	using BASE::mr;
public:
	FileScanner_t(T_Dumper& r)
		: BASE(r)
	{
	}
protected:
	virtual void OnFieldDecl(CFieldPtr p, bool bNameOnly = false) {
		assert(p->objField());
		mr.drawFieldDecl(p, bNameOnly);
	}
	virtual void OnFieldDef(CFieldPtr p) {
		mr.drawFieldDefinition(p, nullptr, nullptr, true);
	}
	virtual void OnGlobDecl(CGlobPtr g) {
		mr.drawGlobDecl(g);
	}
	virtual void OnGlobDef(CGlobPtr g) {
		mr.drawGlobDefinition(g, nullptr, nullptr, true);
	}
	virtual void OnFieldGap(CFieldPtr p) {
		mr.DumpFieldGap(p);
	}
	virtual void OnFieldRef(CFieldPtr p) {
		mr.dumpFieldRef(p);
	}
	virtual void OnConstRef(CFieldPtr p) {
		mr.dumpConstRef(p);
	}
	virtual void OnImpFieldRef(CFieldPtr p) {
		mr.drawImpFieldRef(p, nullptr);
	}
	virtual void OnTypeRef(CTypePtr p, bool) {
		mr.drawTypeName(p, false);
	}
	virtual void OnStrucDecl(CTypePtr p, bool bImporting) {
		mr.dumpStrucDecl(p, bImporting);
	}
	virtual void OnStrucEnd(CTypePtr p) {
		mr.dumpStrucEnd(p);
	}
	virtual void OnFuncDecl(CGlobPtr);//fwd
	virtual void OnVFuncDecl(CGlobPtr, int);
	virtual void OnVTableDecl(CGlobPtr g, int){
		mr.drawVTableDeclaration(g);
	}
	virtual void OnImpVTableDecl(CGlobPtr g, int){
		mr.drawImpVTableDeclaration(g);
	}
	virtual void OnFuncDefinition(CGlobPtr);//fwd
	virtual void OnImpGlobDecl(CGlobPtr g) {
		mr.drawImpGlobDecl(g, nullptr);
	}
	virtual void OnImpClsGlob(CGlobPtr g) {
		mr.drawImpClsGlobDecl(g, nullptr, nullptr);
	}
};

template <typename T_Dumper>
class FuncScanner_t : public DumpScanner_t<T_Dumper>
{
	typedef DumpScanner_t<T_Dumper> BASE;
protected:
	using BASE::mr;
public:
	FuncScanner_t(T_Dumper& r)
		: BASE(r)
	{
	}
public:
	//IDump
	virtual void OnFuncDecl(CGlobPtr g) {
		assert(g->objGlob());
		FuncDumpFlags_t flags(FUNCDUMP_ALL);
		mr.DumpFunctionDeclaration0(mr.FuncDefPtr(), FuncDumpFlags_t(flags & ~FUNCDUMP_OWNERCLASS), nullptr, g->ownerScope1());
	}
	virtual void OnVFuncDecl(CGlobPtr g, int) {
		assert(0);
		//OnFuncDecl(g);
	}
	virtual void OnVTableDecl(CGlobPtr g, int) {
		assert(0);
		//mr.drawVTableDeclaration(g)
	}
	virtual void OnFuncDefinition(CGlobPtr g) {
		assert(g->objGlob());
		mr.DumpDefinition();
	}
	virtual void OnStubInfo(CGlobPtr g) {
		assert(g->objGlob());
		mr.DumpFunctionClosingInfo(g);
	}
	virtual void OnFieldDecl(CFieldPtr p, bool) {
		assert(p->objField());
		//dumpFieldDecl(p, nullptr, nullptr);
		if (!mr.disp().dumpSym(adcui::SYM_FLDDECL, p))
			mr.drawFieldDefinition(p, nullptr, nullptr, false);
	}
	virtual void OnOp(OpPtr p) {
		mr.drawCodeLine0(p);
	}
	virtual void OnLabelDecl(PathPtr p) {
		mr.drawLabelDecl0(p);
	}
};


template <typename T_Dumper>
class ProtoScanner_t : public DumpScanner_t<T_Dumper>
{
	typedef DumpScanner_t<T_Dumper> BASE;
protected:
	using BASE::mr;
public:
	ProtoScanner_t(T_Dumper& r)
		: BASE(r)
	{
	}
public:
	//IDump
	virtual void OnFuncDecl(CGlobPtr g) {
		assert(g->objGlob());
		FuncDumpFlags_t flags(FUNCDUMP_ALL);
		CTypePtr pScope(g->ownerScope1());
		//if (!pScope)
			//pScope = mr.PrimeSeg();
		mr.DumpFunctionDeclaration0(mr.FuncDefPtr(), FuncDumpFlags_t(flags & ~FUNCDUMP_OWNERCLASS), nullptr, pScope, false);// !mr.disp().IsHeader());
	}
	virtual void OnVFuncDecl(CGlobPtr g, int) {
		assert(0);
		//OnFuncDecl(g);
	}
	virtual void OnVTableDecl(CGlobPtr g, int) {
		assert(0);
		//mr.drawVTableDeclaration(g);
	}
	virtual void OnFuncDefinition(CGlobPtr g) {
		assert(g->objGlob());
		//mr.DumpDefinition();
		FuncDumpFlags_t flags(FUNCDUMP_ALL);
		mr.DumpFunctionDeclaration0(mr.FuncDefPtr(), FuncDumpFlags_t(flags & ~FUNCDUMP_OWNERCLASS), nullptr, nullptr, true);// !mr.disp().IsHeader());// g->ownerScope1());
		//assert(0);
	}
};


template <typename T_Dumper>
void FileScanner_t<T_Dumper>::OnFuncDecl(CGlobPtr g)
{
	assert(g->typeFuncDef());
	ProtoInfo_t FI(mr.dc(), (GlobPtr)g);// , mr.FileDef());

	ProtoDumper_t FD(FI, mr.proot(), mr.indent(), mr.disp(), mr.ctx(), mr.ed());

	ProtoScanner_t<ProtoDumper_t> FS(FD);
	FS.OnFuncDecl(g);
}

template <typename T_Dumper>
void FileScanner_t<T_Dumper>::OnVFuncDecl(CGlobPtr g, int off)
{
	OnFuncDecl(g);
	assert(ClassInfo_t::IsMethodVirtual(g));
	if (mr.IsProbing() && off != -1)
	{
		ClassVTable_t* pVTbl(ClassInfo_t::OwnerVTable(g));
		if (pVTbl && pVTbl->self)
		{
			mr.probeSrc()->atPos() = pVTbl->address() + off;
		}
	}
}

template <typename T_Dumper>
void FileScanner_t<T_Dumper>::OnFuncDefinition(CGlobPtr g)
{
	if (mr.disp().isUnfoldMode())
	{
		mr.dumpFieldNameFull(g);
		mr.dumpStr("()");
		return;
	}

	//assert(pField->objField());

	assert(g->typeFuncDef());
	FuncInfo_t FI(mr.dc(), *(GlobPtr)g);// , mr.FileDef());
	FuncDumper_t FD(FI, mr.proot(), mr.indent(), mr.disp(), mr.ctx(), mr.ed());

	FuncScanner_t<FuncDumper_t> FS(FD);
	FS.OnFuncDefinition(g);
}

