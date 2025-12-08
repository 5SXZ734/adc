#pragma once

#include <set>
#include "shared/misc.h"
#include "db/mem.h"
#include "db/types.h"
#include "info_func.h"
#include "expr_type.h"
#include "arglist.h"

struct SwitchQuery_t;
enum ESimplifyOutcome : unsigned;


enum OUT_e
{
	OUT_NULL, OUT_OP = 1, OUT_FIELD, OUT_IMM, OUT_PATH, OUT_ARG, OUT_ARG_THIS
};


class Out_t
{
public:
	friend class EXPR_t;

	static int sID;

#ifdef _DEBUG
	int			mId;
#endif
	Action_t	mAction;
	Out_t* mpU;
	Out_t* mpL;
	Out_t* mpR;
	HOP			mpDockOp;
	HOP			mpOp;
	CFieldPtr	mpField;//always local -or- for locals to track a field mapping
	CFieldPtr	mpExpField;//may be external (for imported symbols)
	HPATH		mpPath;
	OUT_e		mKind;
	TYP_t		mTyp;
	OPC_t		mSsid;//for OUT_OP only
	int			mOffs;
	int			mDisp;
	int			m_mult;
	int			m_eflags;	//for OPC_CPUCW etc.
	int			m_seg : 16;		//for ptrs
	value_t		m_value;
	XOpList_t	m_xin;
	XOpList_t	m_xout;
	bool		m_bExtraIn : 1;
	bool		m_bExtraOut : 1;
	bool		m_bNoReduce : 1;
	bool		m_bPostCall : 1;

public:
	Out_t(ExprInfoBase_t* pExpr);
	Out_t& operator =(const Out_t& o)
	{
		cloneFrom(o);
		return *this;
	}
	void cloneFrom(const Out_t&);

	bool isFieldKind() const { return mKind == OUT_FIELD; }
	bool isArgThisKind() const { return mKind == OUT_ARG_THIS; }
	bool isArgKind() const { return mKind == OUT_ARG || isArgThisKind(); }
	bool isOpFieldKind() const { return isOpKind() || isFieldKind(); }
	bool isExtendedBy(const Out_t*) const;

	HOP dockOp() const { return mpDockOp; }
	void setFieldRef(FieldPtr p) { mpField = p; }
	FieldPtr fieldRef() const { return (FieldPtr)mpField; }

	void setField(FieldPtr p) { mpField = p; }
	FieldPtr field() const { return (FieldPtr)mpField; }
	FieldPtr expField() const { return (FieldPtr)mpExpField; }
	FieldPtr fieldEx() const { return mpExpField ? (FieldPtr)mpExpField : (FieldPtr)mpField; }

	SSID_t SSId() const { return SSID_t(mSsid & SSID_MASK); }
	OPC_t opc() const { return mSsid; }
	int	opoff() const { return mOffs; }
	
	const XOpList_t& xin() const { return m_xin; }
	void setXIn(const XOpList_t& a) { m_xin = a; }
	const XOpList_t& xout() const { return m_xout; }
	void setXOut(const XOpList_t& a) { m_xout = a; }

	int	size() const { return mTyp.size(); }
	int depth();
	VALUE_t Value() const;
	TYP_t typ() const { return mTyp; }
	TYP_t typx() const;
	void setTyp(const TYP_t& t) { mTyp = t; }
	//void setTyp(const Out_t* p) { mTyp = p->typ(); }
	void clearTyp() { mTyp.clear(); }

public:
	bool AddChild(Out_t*);

	int CheckEqualedD(Out_t* pOut), CheckEqualedU(Out_t* pOut);
	Out_t* CheckIndirParent();
	bool	CheckIndirReady();
	bool	CheckIndirReady2();

	bool isLeft() const {
		return mpU && mpU->mpL == this;
	}
	bool isRight() const {
		return mpU && mpU->mpR == this;
	}
	bool isImmidiate() const {
		return isNumKind() && (!mTyp.isReal());
	}
	int isImmidiateEx() const;
	bool isNumKind() const { return (mKind == OUT_IMM); }
	bool isNumZero() const { return (mKind == OUT_IMM) && (m_value.i64 == 0); }
	bool isNumUnit() const { return (mKind == OUT_IMM) && (m_value.i64 == 1); }
	bool isOpIndirect() {
		return isOpKind() && (opc() & OPC_INDIRECT);
	}
	
	bool is(OUT_e a) const { return a == mKind; }
	bool is(Action_t a) const { return a == mAction; }

	int setOffset(int);
	bool isIntrinsicRef() const;
	
	bool isTerm() const { return is(ACTN_NULL); }//terminal/operand (not an operation)
	bool isTop() const { return is(ACTN_NULL); }//?
	bool isActionAddOrSub() const { return is(ACTN_ADD) || is(ACTN_SUB); }
	bool isActionSepar() const { return is(ACTN_SEMI) || is(ACTN_COMMA3); }//statement separator - semicolon or comma
	bool isMovExAction() const { return ISMOVEX(mAction); }
	bool isComparAction() const { return ISCMPRSN(mAction); }
	bool isComparAction0() const { return ISJMPIF(mAction); }

	bool isOpKind() const { return mKind == OUT_OP; }
	bool isOpOf(SSID_t ssid) const { return mKind == OUT_OP && (mSsid & SSID_MASK) == ssid; }
	bool isCallLike() const;
	bool isBadRet() const;
	bool isEqualTo(const Out_t*) const;
	bool isEqualToTerm(const Out_t*) const;
	bool isRoot() const { return (!mpU /*&& !mAction && !pOp*/); }

	void flipChilds();
	Out_t* sibling() const;
	//int CheckTypeCast(FieldPtr pField);
//int CombineTypes(TYP_t TypeL, TYP_t TypeR, TYP_t &Type);

	int ExtractMultiplier(int&, bool bDetach = false);
	int __extractMultiplier(int&, bool bDetach = false);


	class Iterator : public std::vector<Out_t*>
	{
	public:
		Iterator(const Out_t* pRoot)
		{
			assert(pRoot);
			push_back((Out_t*)pRoot);
		}
		operator bool() const { return !empty(); }
		Iterator& operator++()
		{
			Out_t* pCur(back());
			pop_back();
			if (pCur->mpR)
				push_back(pCur->mpR);
			if (pCur->mpL)
				push_back(pCur->mpL);
			return *this;
		}
		Iterator& operator++(int) { return operator++(); }
		int level() const
		{
			int l(-1);
			for (Out_t* p(back()); p; p = p->mpU)
				l++;
			return l;
		}
		Out_t& operator*() { return *back(); }
		Out_t* top() { return back(); }
	};
};



template <typename T_Dumper>
class ProtoImpl4Expr_t//for expr_term (Out_t)
{
	//std::ostream& mos;
	T_Dumper& mrDumper;

public:
	typedef expr::GType* TYPEPTR;
	typedef Out_t* FIELDPTR;
	typedef expr::GType* FUNCPTR;
	typedef expr::GType* RETPTR;

	typedef TYPEPTR		type_ptr;
	typedef FIELDPTR	field_ptr;
	typedef		ProtoImpl4Expr_t	ARGDUMPER;
	struct ARGINFO {
		TYPEPTR pType;
		FIELDPTR pField;
		std::string sReg;
		ARGINFO(TYPEPTR t, FIELDPTR f) : pType(t), pField(f) {}
	};

public:
	ProtoImpl4Expr_t(T_Dumper& rDumper)
		: mrDumper(rDumper)
	{
	}
	ProtoImpl4Expr_t(const ProtoImpl4Expr_t& o)
		: mrDumper(o.mrDumper)
	{
	}
	bool IsDefinition() const {
		return false;
	}
	void dumpStr(const char* s, size_t n = 0) {
		mrDumper.dumpStr(s, n);
	}
	void	dumpWStr(const wchar_t* s, uint16_t n = 0){
		mrDumper.dumpWStr(s, n);
	}
	void dumpReserved(const char* s) {
		mrDumper.dumpReserved(s);
	}
	void dumpTab() {
		mrDumper.dumpTab();
	}
	void dumpArrayIndex(unsigned i) {
		dumpStr(NumberToString(i).c_str());
	}
	type_ptr scope() const {
		return nullptr;
	}
	type_ptr baseType(type_ptr pn) const {
		if (pn->typePtr() || pn->typeRef())
			return pn->baseType();
		/*switch (pn->type)
		{
		case NODE_TYPEDEF:
		case NODE_PTR:
		case NODE_REF:
		case NODE_RVAL_REF:
		case NODE_PTR_TO_MEMBER:
		case NODE_ARRAY:
			return pn->left;
		}*/
		return nullptr;
	}
	bool isSharedType(type_ptr p) const {
		/*if (p->type == NODE_FUNC)
			return false;*/
		return true;
	}
	bool isTypeOkToDump(type_ptr) const {
		return true;
	}
	void dumpTypeRef(type_ptr p) {
		/*if (p->isConst())
			mos << "const ";
		mos << p->name;*/
		if (p->typeCallPtr())
		{
			MyString s("(*");
			const Out_t& r(*p->typeCallPtr()->exprTerm());
			s += ")";
			dumpStr(s);
		}
		else
		{
			CTypePtr iType(p->typeObj());
			if (iType)
				mrDumper.dumpTypeRef(iType);
			else
				dumpStr("?");
		}
	}
	void dumpBasicType(type_ptr p) {
		if (p)
		{
			int n;
			const char* pc(OpTyp2Str(p->typeSimple()->optype(), 0, &n));
			mrDumper.dumpPreprocessor(pc);
			//int color((n == 1) ? adcui::COLOR_KEYWORD : adcui::COLOR_PREPROCESSOR);
			/*if (p->isConst())
				mos << "const ";
			assert(!p->name.empty());
			mos << p->name;*/
		}
		else
			dumpReserved("void");
	}
	void drawFieldName(field_ptr p) {
		/*if (p->type == NODE_NULL)
			mos << p->name;*/
	}
	bool isPointerType(type_ptr p) const {
		return p->typePtr() != nullptr;
		/*switch (p->type)
		{
		case NODE_PTR:
		case NODE_REF:
		case NODE_RVAL_REF:
		case NODE_PTR_TO_MEMBER:
			return true;
		}
		return false;*/
	}
	bool isArrayType(type_ptr p) const { return p->typeArray() != nullptr; }
	bool isFunctionType(type_ptr p) const {
		if (p->typeFunc())
			return true;
		return false;// p->mTyp.isFunc();
	}
	bool isSpecialPointerType(type_ptr p) const { return false; }
	bool isReferenceType(type_ptr p) const { return p->typeRef() != nullptr; }
	bool isRvalReferenceType(type_ptr p) const { return false; }
	bool isExcludedType(type_ptr p) const { return false; }
	bool isBasicType(type_ptr p) const { return p->typeSimple() != nullptr; }
	bool isCompoundType(type_ptr p) const { return false; }//?
	bool isEnumerationType(type_ptr p) const { return false; }//?
	bool isConstantType(type_ptr p) const { return false; }
	bool isTypedefType(type_ptr p) const { return false; }
	unsigned arrayIndex(type_ptr p) const {
		return p->typeArray()->total();
	}

	bool isArgVisible(field_ptr p) {
		return false;// (p->type == NODE_NULL);
	}
	type_ptr fieldScope(field_ptr) const { return nullptr; }
	bool useTabSeparator(field_ptr) { return false; }
	bool isRegister(const ARGINFO& a) const { return false; }// (a.pField->attr& NodeAttr_REGISTER) != 0;}
	bool isBitfield(field_ptr) const { return false; }

	bool getFunctionArguments(FUNCPTR pType, std::vector<ARGINFO>& v) const
	{
		if (!pType->typeCallPtr())//?
			return false;
		assert(pType->typeCallPtr());//?
		Out_t* p(pType->typeCallPtr()->exprTerm());
		Out_t* pOutArg(p->mpR);
		for (int i(0); pOutArg != nullptr; i++)
		{
			if (pOutArg->is(ACTN_COMMA))
			{
				Out_t* q(pOutArg->mpL);
				v.push_back(ARGINFO(q->typx().type(), nullptr));
				pOutArg = pOutArg->mpR;
			}
			else
			{
				v.push_back(ARGINFO(pOutArg->typx().type(), nullptr));
				pOutArg = nullptr;
			}
		}
		return false;
	}

	void getFunctionReturnValues(FUNCPTR pType, std::vector<ARGINFO>& v) const
	{
		if (!pType->typeCallPtr())//?
			return;
		assert(pType->typeCallPtr());//?
		Out_t* p(pType->typeCallPtr()->exprTerm());
		if (p->mpU->is(ACTN_MOV) || p->mpU->isMovExAction())
		{
			assert(p->isRight());
			Out_t* q(p->sibling());
			v.push_back(ARGINFO(q->typx().type(), nullptr));
		}
		else if (p->mpU->is(ACTN_TYPE))
		{
			Out_t* q(p->mpU);
			v.push_back(ARGINFO(q->typx().type(), nullptr));
		}
#if(0)
		else
			v.push_back(ARGINFO(p->typ(), nullptr));
#endif
	}

	type_ptr fieldType(field_ptr p) const {
		return p->mTyp.type();
	}//?
	//type_ptr argumentType(field_ptr p) const { return (p->type == NODE_NULL) ? p->left : p; }
	//type_ptr returnType(ret_ptr p) const { return p; }

	//bool isFunctionIntrinsic(type_ptr p) const { return false; }
	bool isFunctionStdCall(type_ptr p) const {
		return false;//?
	}
	bool isFunctionUserCall(type_ptr p) const {
		return false;
	}
	//void dumpAttribute(ProtoAttrEnum){}
	//bool dumpAttribute(type_ptr, FuncStatusEnum){ return false; }
	bool dumpAttributes(type_ptr pSelf)
	{
		if (isFunctionStdCall(pSelf))//__stdcall is default for non-static class members 
		{
			dumpStr(" ");
			dumpReserved("__stdcall");
			return true;
		}
		return false;
	}

	//FuncStatusEnum functionStatus(type_ptr p) const { return FUNCSTAT_OK; }
	type_ptr ownerScope(type_ptr p) const { return nullptr; }
};



