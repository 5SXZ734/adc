#pragma once

#include "dump.h"
#include "dump_base.h"
#include "type_funcdef.h"
#include "dump_proto.h"


//////////////////////////////////////////////////
class FileDumper_t : public DumperBase_t<FileInfo_t>//,
	 //public FileInfo_t
{
	bool	mbImporting;
	//FileTempl_t	*mpFileTempl;
//public:
	//FileDef_t &mrFileDef;
public:
	FileDumper_t(const FileInfo_t &, Display_t &, ProbeEx_t*, const MyLineEditBase*, bool bImporting = false);
	//FileDumper_t(const FileInfo_t &, const DumperBase_t<FileInfo_t> &);//for imported declarations
	//FileDumper_t(const DumperBase_t &, Dc_t &, FileDef_t &);//for imported declarations
	//virtual void OnExtClsField(const ExpFieldInfo_t &a){ /*assert(p->objField());*/ drawExtClsFieldDecl(a); }
	//virtual void OnStubInfo(CGlobPtr);
	//virtual const FileInfo_t &fileInfo() const { return *this; }
	//virtual const DcInfo_t &dcInfo() const { return *this; }

	void	write(std::ostream &);
	MyString ExpandLine(int, const char * line0, int &len0, const DumpContext_t &);
	MyString ExpandLinePlain(int, const char * line0, int &len0, const DumpContext_t &);
	bool	IsHeader() const { return m_disp.IsHeader(); }
	bool	IsImporting() const { return mbImporting; }
	void	DumpChosen(CTypeBasePtr);
	void	DumpTypeDecl(CTypePtr, CTypePtr &ns);
	void	DumpGlobDecl(CGlobPtr, CTypePtr &ns);
	bool	DumpGlobDef(CGlobPtr, bool bFuncBefore);
	void	DumpFile();
	void	DumpContents(CFolderPtr);
	int		DumpIncludes(CFolderPtr);
	void	DumpPrologueInfo(CFolderPtr);
	void	DumpHeaderGuardOpen(CFolderPtr);
	void	DumpHeaderGuard(CFolderPtr);
	void	DumpHeaderGuardClose(CFolderPtr);
	void	DumpInclude(CFolderPtr);
	void	File_OutIDStr(CFolderPtr, bool);
	void	OutputFileBeg(CFolderPtr);
	void	OutputFileEnd(CFolderPtr);
	void	DumpPrefixContents(CFolderPtr);
	void	OutputDescBeg(const char * pc);
	void	OutputDescRest(const char * pc);
	void	OutputLogoStr(const char * pc);
	bool	drawImpGlobDecl(CGlobPtr, CTypePtr scope);//imported field
	void	drawFieldDecl(CFieldPtr pField, bool bNameOnly);
	void	drawGlobDecl(CGlobPtr pField);
	static int globStatus(CGlobPtr);//0:hidden,1:variable,2:function
	void	dumpGlobal(CGlobPtr, CGlobPtr imp, CTypePtr scope);
	void	drawGlobal(CGlobPtr, CGlobPtr imp, CTypePtr scope);
	void	DumpFunc(CGlobPtr);
	void	DumpFuncDef(CGlobPtr);
	void	DumpThunk(CGlobPtr);

	CTypePtr	DumpNamespaceSwitch(CTypePtr, CTypePtr);
	CTypePtr	dumpNamespaceOpen(CTypePtr, CTypePtr);
	void	dumpNamespaceOpen0(CTypePtr);
	void	dumpNamespaceClose0(CTypePtr);

	//1-whitespace after non-pointer types
	//2-tab instead of space
	//4-two whitespaces
	//void DumpTypedef(const char *source, const char *target);
	void Probe(ProbeExIn_t&);// size_t modelId, long X, long Y);
};

/////////////////////////////////////////
class StrucDumper_t : public DumperBase_t<DcInfo_t>
{
	const TypeObj_t&	mrTypeRef;//this may be a proxy
	const Struc_t&	mrStruc;
//	Folder_t*	mpExtFolder;//incase of dumping through a proxy, this going be different from a current folder
public:
	StrucDumper_t(const DcInfo_t &o, FileDumper_t *root, int indent, Display_t &disp, ProbeEx_t* pCtx, const MyLineEditBase* ped, const TypeObj_t &rStrucRef)
		: DumperBase_t<DcInfo_t>(root, indent, disp, pCtx, ped, o),
		mrTypeRef(rStrucRef),
		mrStruc(* SkipProxy(&mrTypeRef)->typeStruc())//,
		//mpExtFolder(nullptr)
	{
	}
	void	CheckEos();
	void	Dump(bool bImporting);
	void	DumpNestedTypes(bool bImporting, bool& bPublicAccess, bool &bStrucEnd);
	void	DumpStrucFields(FieldMapCIt, bool bImporting, bool bPublicAccess, bool &bStrucEnd);
	int		DumpClassMethods(bool bImporting, bool &bStrucEnd);
	void	DumpClassVTables(bool bImporting, int& nMethods, bool &bStrucEnd);
	void	DumpRaw(bool bImporting);//no data stream
	void	DumpRaw(DataStream_t &, bool bImporting);
	void	DumpDeclaration(bool bImporting);
	void	OutputFieldGap(FieldMapCIt);
	void	OutputFieldGap0(CFieldPtr next, ADDR gap);
	void	DumpField(FuncInfo_t *, FieldMapCIt, FieldMapCIt);
	//void	DumpClassMethodDecl(CTypePtr iFuncDef, CFieldPtr pImpField);
	//void	DumpClassFieldDecl(CFieldPtr pImpField);
//	void setImporting(FolderPtr p){ mpExtFolder = p; }
//	FolderPtr isImporting() const { return mpExtFolder; }
};


#define TABSIZE	4

////////////////////////////////////////////////////////
// ArrSizeDumper_t

template <typename T_Dumper>
class ArrSizeDumper_t : public Range_t
{
	T_Dumper& m_dumper;
	ProbeEx_t* mpCtx;
	const MyLineEditBase* mpEd;
	uint32_t miTotal;
	adcui::Color_t m_iColor;
	adcui::Color_t m_iFont;
public:
	MyString sName;

public:
	ArrSizeDumper_t(T_Dumper& dumper, uint32_t i)
		: m_dumper(dumper),
		mpCtx(m_dumper.ctx()),
		mpEd(m_dumper.ed()),
		miTotal(i),
		m_iColor(adcui::COLOR_NULL),
		m_iFont(adcui::COLOR_NULL)
	{
		if (mpCtx && m_dumper.disp().curLine() == mpCtx->line())
			setBegin(m_dumper.currentPosInLine());
	}
	void setColor(adcui::Color_t c) { m_iColor = c; }
	void setFont(adcui::Color_t f) { m_iFont = f; }
	bool checkCur()
	{
		if (mpCtx && (m_dumper.disp().curLine() == mpCtx->line()) && begin() == mpCtx->rangeObj().begin())
		{
			if (mpEd)
			{
				m_iColor = adcui::COLOR_CUR_EDIT;
				sName = mpEd->editName();
			}
			else
				m_iColor = adcui::COLOR_CUR;
			return true;
		}
		return false;
	}
	~ArrSizeDumper_t()
	{
		if (m_iColor)
			m_dumper.PushColor(m_iColor);
		if (m_iFont)
			m_dumper.PushColor(m_iFont);
		m_dumper.dumpStr(sName);
		if (m_iFont)
			m_dumper.PopColor();
		if (m_iColor)
			m_dumper.PopColor();

		if (begin() >= 0)
		{
			setEnd(m_dumper.currentPosInLine());
			if (probe() && inside2(probe()->x()))
			{
				probe()->pickRange(begin(), unsigned(size()));
				//probe()->pickX(begin());
				probe()->pickEntityId(adcui::COLOR_ARRAY);
				probe()->pickValue(value_t(miTotal));
			}
		}
	}
private:
	ProbeExIn_t* probe() const { return dynamic_cast<ProbeExIn_t*>(mpCtx); }
};


#include "dump_proto.h"

/*class Dumper0_t
{
public:
	bool		mbIsDef;		//definition
public:
	Dumper0_t() : mbIsDef(false){
	}
	void dumpStr(const char *s){
		std::cout << s;
	}
	void dumpReserved(const char *s){
		dumpStr(s);
	}
	void dumpTab(){
		std::cout << "\t";
	}
	void dumpArrayIndex(unsigned i){
		std::cout << i;
	}
	void dumpBasicType(CTypePtr);
	void drawFieldName(CFieldPtr){}
	bool isArgVisible(CFieldPtr){ return true; }
	TypePtr scope() const { return nullptr; }
	void dumpTypeRef(CTypePtr){}
	TypePtr fieldScope(CFieldPtr) const { return nullptr; }
	bool isTypeOkToDump(CTypePtr) const { return true; }
	bool useTabSeparator(CFieldPtr);
	bool isBitfield(CFieldPtr);

	TypePtr baseType(CTypePtr) const;
	bool isSharedType(CTypePtr) const;
	bool isArrayType(CTypePtr) const;
	bool isFunctionType(CTypePtr) const;
	bool isPointerType(CTypePtr) const;
	bool isSpecialPointerType(CTypePtr) const;
	bool isReferenceType(CTypePtr) const;
	bool isRvalReferenceType(CTypePtr) const;
	bool isExcludedType(CTypePtr) const;
	bool isBasicType(CTypePtr) const;
	bool isCompoundType(CTypePtr) const;
	bool isEnumerationType(CTypePtr) const;
	bool isConstantType(CTypePtr) const;
	bool isTypedefType(CTypePtr) const;
	bool arrayIndex(CTypePtr) const;
};*/


template <typename T_Dumper>
class ProtoImpl_t
{
public:
	T_Dumper	&mrDumper;
	bool		mbIsDef;		//definition
	bool		mbReplaceExtDoubles;	//true if to replace the extended double types (REAL80)
protected:
	typedef		CTypePtr	TYPEPTR;
	typedef		CFieldPtr	FIELDPTR;
	typedef		CTypePtr	FUNCPTR;
	typedef		CTypePtr	RETPTR;
	typedef		ProtoImpl_t	ARGDUMPER;
	struct ARGINFO {
		TYPEPTR pType;
		FIELDPTR pField;
		MyString sReg;
		ARGINFO(TYPEPTR t, FIELDPTR f) : pType(t), pField(f) {}
		ARGINFO(TYPEPTR t, FIELDPTR f, MyString s) : pType(t), pField(f), sReg(s){}
	};

public:
	ProtoImpl_t(T_Dumper&r)
		: mrDumper(r),
		mbIsDef(false),
		mbReplaceExtDoubles(false)
	{
	}
	/*ProtoImpl_t(const ProtoImpl_t &o)
		: mrDumper(o.mrDumper),
		mbIsDef(o.mbIsDef)
	{
	}*/
	void setReplaceExtDoubles(bool b) {
		mbReplaceExtDoubles = b;
	}
	bool IsDefinition() const {
		return mbIsDef != 0;
	}
	void dumpStr(const char *s, size_t n = 0){
		mrDumper.dumpStr(s, n);
	}
	void	dumpWStr(const wchar_t* s, uint16_t n = 0){
		mrDumper.dumpWStr(s, n);
	}
	void dumpReserved(const char *s){
		mrDumper.dumpReserved(s);
	}
	/*void dumpAttribute(ProtoAttrEnum at){
		mrDumper.dumpProtoAttribute(at);
	}*/
	/*bool dumpAttribute(CTypePtr iSelf, FuncStatusEnum eStatus){
		return mrDumper.dumpProtoAttribute(iSelf, eStatus);
	}*/
	bool dumpAttributes(CTypePtr iSelf) {
		bool bRet(false);
		FuncStatusEnum eStatus(functionStatus(iSelf));
		if (mrDumper.dumpProtoAttribute(iSelf, eStatus))
			bRet = true;
		//if (!IsDefinition())
		{
			if (isFunctionUserCall(iSelf))
			{
				mrDumper.dumpSpace();
				dumpReserved("__usercall");
				bRet = true;
			}
			else if (!IsDefinition() && isFunctionStdCall(iSelf))//__stdcall is default for non-static class members 
			{
				mrDumper.dumpSpace();
				dumpReserved("__stdcall");
				bRet = true;
			}
		}		
		return bRet;
	}
	void dumpTab(){
		mrDumper.dumpTab();
	}
	bool useTabSeparator(CFieldPtr pSelf){
		if (pSelf && (DcInfo_t::IsGlobal(pSelf) || FuncInfo_s::IsLocal(pSelf)))
			return false;//no tabbing for func declarations/definitions & for func args at all
		return true;
	}
	virtual void drawFieldName(CFieldPtr pField){
		adcui::Color_t iColor(adcui::COLOR_NULL);
		/*if (FuncInfo_t::IsLocalArg(pField))
		{
			if (pField->nameless())
			{
				FuncDef_t* pfDef(pField->owner()->typeFuncDef());
				if (pfDef->isStub() || !mbIsDef)
					iColor = adcui::COLOR_UNEXPLORED;
			}
			mrDumper.drawFieldName(pField, iColor);
			const XOpList_t& refs(FuncInfo_t::LocalRefs(pField));
			if (!refs.empty())
				mrDumper.DumpXRefs(refs);
		}
		else*/
			mrDumper.drawFieldName(pField, iColor);
	}
	void dumpBasicType(CTypePtr pSelf)
	{
		if (!pSelf)
		{
			mrDumper.output_optype(0, 0);//void
			return;
		}
		if (mrDumper.disp().testOpt1(adcui::DUMP_NOREAL80) && pSelf->typeSimple() && pSelf->typeSimple()->optype() == OPTYP_REAL80)
		{
			mrDumper.output_optype(OPTYP_REAL64, 0);//void
			return;
		}
		MyString s;
		pSelf->namex(s);
		mrDumper.output_optype(s, true);
		return;
		//assert(pSelf->typeSimple());
		//mrDumper.output_optype(pSelf->typeSimple()->optype(), 0);
	}
	void dumpTypeRef(CTypePtr pSelf)
	{
		if (pSelf->typeFuncDef())
		{
			if (pSelf->objGlob()->isIntrinsic())
			{
				dumpStr(pSelf->name()->c_str());
				return;
			}
			if (!pSelf->isShared())
				dumpStr("?");
		}
		if (!pSelf->isShared())
		{
			mrDumper.dumpReserved(pSelf->printType());
		}
		else
			mrDumper.dumpTypeRef(pSelf);
	}
	void dumpArrayIndex(unsigned i)
	{//RAII-block
		ArrSizeDumper_t<T_Dumper> aRange(mrDumper, i);
		aRange.sName = MyStringf("%d", i);
		if (!aRange.checkCur())
			aRange.setColor(adcui::COLOR_ARRAY);
	}

	//TypePtr scope() const { return mpScope; }
	TypePtr fieldScope(CFieldPtr pSelf) const
	{
		if (DcInfo_t::IsGlobal(pSelf))
			return DcInfo_t::OwnerScope(DcInfo_s::GlobObj(pSelf));
		return nullptr;
	}
	bool isTypeOkToDump(CTypePtr pSelf) const
	{
		if (!pSelf)
			return false;
		if (pSelf->typeComplex() && !pSelf->hasUserData())//not in file?
			if (!pSelf->typeFuncDef())
				if (!pSelf->isVTable())
					return false;
		return true;
	}
	bool isRegister(const ARGINFO &a) const { return a.pField->SSIDx() != SSID_NULL; }
	bool isBitfield(CFieldPtr p) const { return p && p->owner()->typeBitset(); }
	TypePtr baseType(CTypePtr p) const { return p->baseType(); }
	bool isSharedType(CTypePtr p) const { return p->isShared(); }
	bool isArrayType(CTypePtr p) const { return p->typeArray() != nullptr; }
	bool isFunctionType(CTypePtr p) const { return p->typeFunc() != nullptr; }
	bool isPointerType(CTypePtr p) const { return p->typePtr() != nullptr; }
	bool isSpecialPointerType(CTypePtr p) const { return p->typeImp() || p->typeVPtr(); }
	bool isReferenceType(CTypePtr p) const { return p->typeRef() != nullptr; }
	bool isRvalReferenceType(CTypePtr p) const { return p->typeRRef() != nullptr; }
	bool isExcludedType(CTypePtr p) const { return p->isVTable(); }
	bool isBasicType(CTypePtr p) const { return p->typeSimple() != nullptr; }
	bool isCompoundType(CTypePtr p) const { return p->typeComplex() != nullptr; }
	bool isEnumerationType(CTypePtr p) const { return p->typeEnum() != nullptr; }
	bool isConstantType(CTypePtr p) const { return p->typeConst() != nullptr; }
	bool isTypedefType(CTypePtr p) const { return p->typeTypedef() != nullptr; }
	unsigned arrayIndex(CTypePtr p) const { return p->typeArray()->total(); }

	//TypePtr argumentType(CFieldPtr p) const { return fieldType(p); }
	//TypePtr returnType(CFieldPtr p) const { return fieldType(p); }

	TypePtr ownerScope(CTypePtr p) const {
		TypePtr pScope(nullptr);
		if (!p->typeSeg())
		{
			if (p->owner())
				pScope = p->ownerScope();
			else if (p->objGlob())
				pScope = DcInfo_t::OwnerScope(p->objGlob());
			else
				pScope = DcInfo_t::OwnerScope(p);
			//if (!pScope)
				//pScope = mrDumper.PrimeSeg();
			if (pScope && pScope->typeSeg())
				pScope = nullptr;
		}
		return pScope;
	}

	virtual bool getFunctionArguments(CTypeBasePtr iSelf, std::vector<ARGINFO> &args) const
	{
		assert(iSelf->typeFunc());
		const TypeFunc_t &r(*iSelf->typeFunc());
		if (r.args())
		{
			for (CTypePtr iArg(r.args());; iArg = iArg->typePair()->right())
			{
				if (!iArg->typePair())
				{
					args.push_back(ARGINFO(iArg, nullptr));
					break;
				}
				args.push_back(ARGINFO(iArg->typePair()->left(), nullptr));
			}
		}
		return (r.flags() & TypeFunc_t::E_VARIARDIC) != 0;
	}

	virtual void getFunctionReturnValues(CTypePtr iSelf, std::vector<ARGINFO> &rets)
	{
		assert(iSelf->typeFunc());
		const TypeFunc_t &r(*iSelf->typeFunc());
		if (r.retVal())
			rets.push_back(ARGINFO(r.retVal(), nullptr));
	}

	//virtual bool isArgVisible(CFieldPtr pArg) const { return false; }

/*	virtual TypePtr fieldType(CFieldPtr p) const {
		CGlobPtr pfDef(DcInfo_t::IsCFuncOrStub(p));
		if (pfDef)
			return (TypePtr)pfDef;
		return p->type();
	}*/

	//virtual bool isFunctionIntrinsic(CTypePtr iSelf) const { /*assert(iSelf->typeFunc());*/	return false; }//?p->isIntrinsic();

	virtual bool isFunctionStdCall(CTypePtr iSelf) const {
		assert(iSelf->typeFunc());
		return iSelf->typeFunc()->isStdCall();
	}
	virtual bool isFunctionUserCall(CTypePtr iSelf) const {
		assert(iSelf->typeFunc());
		return iSelf->typeFunc()->isUserCall();
	}
	virtual FuncStatusEnum functionStatus(CTypePtr p) const { return FUNCSTAT_OK; }
};

/////////////////////////////////////////////////////// ProtoImpl2_t
template <typename T_Dumper>
class ProtoImpl2_t : public ProtoImpl_t<T_Dumper>//for globs dumping
{
	typedef ProtoImpl_t<T_Dumper> BASE;
protected:
	using BASE::mrDumper;
public:
	bool		mbDockAddr;
	bool		mbUglyName;
	CFieldPtr	mpField;
	CFieldPtr	mpImpField;
	//int			miColor;//color of the field
public:
	ProtoImpl2_t(T_Dumper &r)
		: BASE(r),
		mbDockAddr(false),
		mbUglyName(false),
		mpField(nullptr),
		mpImpField(nullptr)//,
		//miColor(nullptr)
	{
	}

protected:
	virtual void drawFieldName(CFieldPtr pSelf)
	{
		if (!DcInfo_t::IsGlobal(pSelf))
		{
			BASE::drawFieldName(pSelf);
		}
		else
		{
	//		if (miColor)
	//			mrDumper.PushColor(miColor);
			mrDumper.drawFieldName(pSelf, mpImpField, mbUglyName, mrDumper.GlobalColor(pSelf, mpImpField), mrDumper.GlobalFont(pSelf));
			if (mpField && mbDockAddr && !ProjectInfo_t::IsPhantomModule(mrDumper.ModulePtr()))
				if (!mpField->nameless())//don't need this with autonames
				{
					MyString sVA(ProjectInfo_s::VA2STR(mpField));
					mrDumper.dumpComment(sVA, false);
				}

	//		if (miColor)
	//			mrDumper.PopColor();
		}
	}


	//virtual bool isFunctionIntrinsic(CTypePtr p) const { return p->isIntrinsic(); }

	virtual bool isFunctionStdCall(CTypePtr p) const {
		if (p->typeFuncDef())
			return !ProtoInfo_s::IsThisCallType(p->objGlob()) && ProtoInfo_s::IsFuncCleanArged(p->objGlob());
		return BASE::isFunctionStdCall(p);
	}

	virtual bool isFunctionUserCall(CTypePtr p) const {
		if (p->typeFuncDef())
			return ProtoInfo_s::IsFuncUserCall(p->objGlob());
		return BASE::isFunctionUserCall(p);
	}

	virtual FuncStatusEnum functionStatus(CTypePtr p) const {
		const FuncDef_t *pf(p->typeFuncDef());
		if (pf)
		{
			if (p->objGlob()->isIntrinsic())
				return FUNCSTAT_INTRINSIC;
			if (ProtoInfo_s::IsFuncStatusProcessing(p->objGlob()))
				return FUNCSTAT_PROCESSING;
			if (ProtoInfo_s::IsFuncStatusAborted(p->objGlob()))
				return FUNCSTAT_INCOMPLETE;//invalid
			if (pf->isStub())
			{
				if (ProtoInfo_s::FuncStatus(p->objGlob()) == 0)
					return FUNCSTAT_UNDEFINED;
				return FUNCSTAT_STUB;
			}
			return FUNCSTAT_OK;
		}
		return BASE::functionStatus(p);
	}
};

