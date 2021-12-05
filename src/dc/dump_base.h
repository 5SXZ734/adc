#pragma once

#include "dump.h"

class Obj_t;
class Out_t;
class FileDumper_t;
class FuncDumper_t;
class FileDef_t;
//enum ProtoAttrEnum;
enum FuncStatusEnum : int;

enum FuncDumpFlags_t
{
	FUNCDUMP_RETLIST = 1,		//returned argument list
	FUNCDUMP_CALLCONV = 2,		//calling convention
	FUNCDUMP_OWNERCLASS = 4,	//owner class
	FUNCDUMP_MODIFIER = 8,		//like 'const'
	FUNCDUMP_ARGS_OPEN = 0x10,
	FUNCDUMP_ARGS_CLOSE = 0x20,
	FUNCDUMP_ARGS = 0x30,		//braces only
	FUNCDUMP_ARGTYPES = 0x40,
	FUNCDUMP_ARGNAMES = 0x80,
	FUNCDUMP_ARGLIST = FUNCDUMP_ARGTYPES | FUNCDUMP_ARGNAMES,
	FUNCDUMP_ALL = 0xFF,
	FUNCDUMP_UGLY_NAME = 0x100,

	FUNCDUMP_UNFOLD_FLAGS = FUNCDUMP_ARGS,
	FUNCDUMP_DECL_FLAGS = FUNCDUMP_CALLCONV | FUNCDUMP_RETLIST | FUNCDUMP_ARGLIST,
	FUNCDUMP_IMPL_FLAGS = FUNCDUMP_ALL
};

typedef PathTree_t::HierIterator	TreePathHierIterator;

template <typename T>
class DumperBase_t : public T
{
protected:
	using T::GlobObj;
	using T::PrimeSeg;
public:
	using T::dc;
protected:
	int m_indent;
	FileDumper_t* m_pRoot;
	Display_t& m_disp;
	std::ostream& mos;
	//ProbeEx_t* mpProbeSrc;
	ProbeEx_t* mpCtx;
	const MyLineEditBase* mpEd;
public:
	template <typename... Args>
	DumperBase_t(FileDumper_t* root, int indent, Display_t& r, ProbeEx_t* pCtx, const MyLineEditBase* ped, Args&&... args)
		: T(std::forward<Args>(args)...),
		m_indent(indent),
		m_pRoot(root),
		m_disp(r),
		mos(r.os()),
		//mpProbeSrc(pProbe),
		mpCtx(pCtx),
		mpEd(ped)
	{
	}

	Display_t& disp() { return m_disp; }
	bool draftMode() const { return m_disp.draftMode(); }
	int IncreaseIndent() { return ++m_indent; }
	int DecreaseIndent() { if (m_indent > 0) return --m_indent; return 0; }
	const char* comment() const { return "//"; }

	void	dumpUnk();
	void	dumpColorTerm(const char*, adcui::Color_t, bool bForce = false);
	void	dumpSemi();
	void	dumpTab(int n = 1);
	void	OutputTabT(int n = 1);
	void	OutputWs(int n = 1, const char* ch = " ");
	bool	PushColor(int, bool force = false);//return false if colors are disabled
	//void	PushFont(int);
	void	PopColor(bool force = false) { PushColor(adcui::COLOR_POP, force); }
	//void	PopFont(){ PushFont(0); }
	void	NewLine(int indent = 1);//>0:use default, 0:ignore, <0:make a shift left
	void	dumpSpace(int = 1);
	void	dumpSep(bool bTab = false);
	void	dumpComma(bool bSpace = false);
	void	dumpStrUntilEol(const char*);
	void	dumpStr(const char*, size_t = 0);
	void	dumpWStr(const wchar_t*, uint16_t = 0);
	void	dumpChar(char);
	void	dumpTypeRef(CTypePtr);
	void	dumpTypeNameFull(CTypePtr);
	void	drawTypeName(CTypeBasePtr, bool bUgly);
	void	dumpStrucDecl(CTypePtr, bool bImporting);
	void	dumpStrucEnd(CTypePtr);
	void	dumpFieldNameFull(CGlobPtr);
	void	dumpFieldNameScoped(CFieldPtr, CTypePtr scope);
	void	dumpTypeNameScoped(CTypePtr, CTypePtr scope);
	void	dumpTypeNameScoped0(CTypePtr, CTypePtr pScopeFrom);
	void	drawFieldName(CFieldPtr, adcui::Color_t);
	void	drawConstRef(CFieldPtr);
	void	drawFieldName(CFieldPtr, CFieldPtr pImpField, bool bUgly, adcui::Color_t = adcui::COLOR_NULL, adcui::Color_t eFont = adcui::COLOR_NULL);//globals
	void	drawVTableDeclaration(CGlobPtr);
	void	drawImpVTableDeclaration(CGlobPtr);
	void	dumpComment(const char* pc, bool bCxxStyle);
	void	dumpReserved(const char*);
	void	dumpPreprocessor(const char*);
	//void	dumpProtoAttribute(ProtoAttrEnum);
	bool	dumpProtoAttribute(CTypePtr, FuncStatusEnum);
	void	dumpString(CTypePtr, DataStream_t&);
	void	output_optype(uint8_t typ, int nMode);
	void	output_optype(const char*, bool bColor);
	void	DumpOpenBrace();
	void	DumpCloseBrace();
//	void	DumpVTableFor(CFieldPtr);
	adcui::Color_t GlobalFont(CFieldPtr) const;
	adcui::Color_t GlobalColor(CFieldPtr, CFieldPtr imp = nullptr) const;
	adcui::Color_t GlobalColor2(CGlobPtr, CGlobPtr imp = nullptr) const;
	bool	fetchString(CTypePtr, DataStream_t&, MyString &);

public:
	FileDumper_t* proot() const { return m_pRoot; }
	FileDumper_t& root() const { return *m_pRoot; }
	int indent() const { return m_indent; }
	//DumperBase_t* parentDumper() const { return m_pParent; }
	bool openScope(CTypeBasePtr);
	const ProbeExIn_t* IsProbing() const;
	void pickField(CFieldPtr);

	void	dumpFieldRef(CFieldPtr);//reference
	void	dumpConstRef(CFieldPtr);//reference
	void	drawImpFieldRef(CFieldPtr, CFieldPtr pExpField);//reference
	void	dumpVTableDecl(CGlobPtr, int, bool bImporting);
	void	dumpFieldDecl(CFieldPtr, CFieldPtr impField, CTypePtr scope);//declaration
	void	drawFieldDef(CFieldPtr, CFieldPtr impField, CTypePtr scope);//definition
	void	drawImpClsGlobDecl(CGlobPtr, CGlobPtr imp, CTypePtr scope);//imported class method field
	//void	drawExtClsFieldDecl(const ExpFieldInfo_t &);//external(!) class method field (when current module doesn't have it imported)
	void	DumpStrucDef(CTypePtr);
	void	Type_Dump(CTypePtr);
	void	DumpFieldGap(CFieldPtr);
	void	drawFieldDefinition0(CFieldPtr);
	void	drawFieldDefinition(CFieldPtr, CFieldPtr, CTypePtr scope, bool bDefinition);
	void	drawGlobDefinition(CGlobPtr, CGlobPtr, CTypePtr scope, bool bDefinition);//generic with type + name
	void	DumpGlobInitialization(CGlobPtr);
	void	DumpFunctionDeclaration(CGlobPtr, FuncDumpFlags_t, CGlobPtr imp, CTypePtr scope, bool bIsDefinition = false);
	void	DumpFunctionDeclaration0(CGlobPtr, FuncDumpFlags_t, CGlobPtr imp, CTypePtr scope, bool bIsDefinition = false);
	void	DumpVirtualMethodDeclaration(CGlobPtr, int off, FuncDumpFlags_t, CGlobPtr imp, CTypePtr scope, bool bIsDefinition = false);
	void	DumpFunctionClosingInfo(CGlobPtr);
	void	DumpFunctionStubInfo(CGlobPtr);
	void	DumpFunctionStub(CGlobPtr);

	bool	IsGlobalVisible(CFieldPtr) const;
	void	OutputDeclspecExpImp(bool bImport);
	void	dumpSimpleInst(CTypePtr, DataStream_t&, AttrIdEnum);
	enum	LASTSYM_enum { LASTSYM_NULL, LASTSYM_VAL, LASTSYM_PUN };
	//LASTSYM_enum		dumpRawData1(CTypePtr, CTypePtr context, AttrIdEnum, CTypePtr startSeg, int level);
	LASTSYM_enum	dumpRawData0(CTypePtr, DataStream_t&, CTypePtr context, AttrIdEnum, CTypePtr startSeg, int level);
	LASTSYM_enum	dumpRawData(CTypePtr, DataStream_t&, CTypePtr context, AttrIdEnum, CTypePtr startSeg, int level);
	LASTSYM_enum	dumpRawDataStruc(CTypePtr, DataStream_t&, CTypePtr context, AttrIdEnum, CTypePtr startSeg, int level);
	void			dumpRawDataVTable(const ClassVTable_t&, DataStream_t&, CTypePtr startSeg, bool bImporting);
	void	DumpVirtualEntry(CGlobPtr, int vmeth_off, bool bImporting);

	//bool	DumpExtSym(const ExpFieldInfo_t &);
	//bool	DumpSym(adcui::SYM_e, CFieldPtr);
	//bool	DumpSym(adcui::SYM_e, CTypePtr);
	void DumpXRefs(const XOpList_t &, bool bExtra = false){}

	CObjPtr curObj() const { 
		//return mpProbeSrc ? mpProbeSrc->obj() : nullptr;
		return mpCtx ? mpCtx->obj() : nullptr;
	}
	OpPtr curOpLine() const {
		if (mpCtx)//mpProbeSrc)
		{
			//if (mpProbeSrc->modelId() == m_disp.modelId())
			return mpCtx->opLine();//return mpProbeSrc->opLine();
		}
		return OpPtr();
	}

	ProbeEx_t* ctx() const { return mpCtx; }
	ProbeExIn_t* probeSrc() const {
		return dynamic_cast<ProbeExIn_t*>(mpCtx);
	}
	const MyLineEditBase* ed() const { return mpEd; }

protected:
	const FileInfo_t& fileInfo() const { return *reinterpret_cast<const FileInfo_t*>(this); }
	const DcInfo_t& dcInfo() const { return *this; }

public:
	long currentPosInLine(bool bTextShift = true) const {
		long pos((long)this->mos.tellp());
		if (pos < 0)//if this stream is never written, tellp return -1 (wierd?)
			pos = 0;
		pos -= m_disp.controlCharsOnLine();//so far
		if (bTextShift)
			pos += adcui::IADCTableModel::colTextShift(0);
		return pos;
	}
};


