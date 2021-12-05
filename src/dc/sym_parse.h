#pragma once

#include <assert.h>
#include <string>
#include <vector>
#include <iostream>
#include "dump_proto.h"

#define NO_BS	1

enum NodeTypeEnum
{
	//root ids
	NODE_NULL,//data?
	NODE_FUNC,
	NODE_TYPEDEF,
	NODE_STRUCT,
	//non-root ids?
	NODE_TYPE,
	NODE_PTR,//*
	NODE_REF,//&
	NODE_RVAL_REF,//&&
	NODE_PTR_TO_MEMBER,
	NODE_ARRAY,
	NODE_NUMBER,
	NODE_COMMA,
	NODE_SEMI,
	NODE_CONST,
	NODE_VOLATILE,
	NODE_VALIST,
#if(NO_BS)
		//special
		NODE__EXTRA,
		NODE_THUNK,
		NODE_VTHUNK,//virtual thunk
		NODE_NVTHUNK,//non-virtual thunk
		NODE_STRING,
#endif
	NODE_LAST
};

enum NodeAttrEnum
{
	NodeAttr_NULL,
	NodeAttr__SV_MASK		= 0x00000003,
		NodeAttr_STATIC		= 0x00000001,
		NodeAttr_VIRTUAL	= 0x00000002,
	NodeAttr__REGISTER_MASK = 0x00000004,
		NodeAttr_REGISTER	= 0x00000004,
	NodeAttr__CV_MASK		= 0x000000F0,
		NodeAttr_CONST		= 0x00000010,
		NodeAttr_VOLATILE	= 0x00000020,
	//calling conventions
	NodeAttr__CC_MASK		= 0x00000F00,
		NodeAttr_CDECL		= 0x00000100,
		NodeAttr_STDCALL	= 0x00000200,
		NodeAttr_THISCALL	= 0x00000300,
		NodeAttr_FASTCALL	= 0x00000400,
	NodeAttr__TT_MASK		= 0x0000F000,
		NodeAttr_SIGNED		= 0x00001000,
		NodeAttr_UNSIGNED	= 0x00002000,
		NodeAttr_ENUM		= 0x00003000,
		NodeAttr_STRUCT		= 0x00004000,
		NodeAttr_CLASS		= 0x00005000,
		NodeAttr_UNION		= 0x00006000,
	NodeAttr__X64_MASK		= 0x000F0000,
		NodeAttr_PTR64		= 0x00010000,
	NodeAttr__REFQ_MASK		= 0x00F00000,//ref-qualifiers (C++11)
		NodeAttr_L_REF		= 0x00100000,
		NodeAttr_R_REF		= 0x00200000,
	NodeAttr__BasicType		= 0x01000000,
	NodeAttr_LAST
};

class node_t
{
public:
	NodeTypeEnum type;
	NodeAttrEnum attr;
	node_t *parent;
	std::string name;
	node_t *left;
	node_t *right;
public:
	node_t(NodeTypeEnum t);
	~node_t();
	void setType(NodeTypeEnum);
	void setName(NodeTypeEnum);
	void setName(const std::string &s){ name = s; }
	void setAttr(NodeAttrEnum u){ attr = (NodeAttrEnum)(attr | u); }
	bool isScopeOpen() const;
	bool hasName() const { return !name.empty(); }
	bool isTerminal() const { return !left && !right; }
	std::string toString() const;
	bool isClass() const { return (attr & NodeAttr__TT_MASK) == NodeAttr_CLASS; }
	bool isStruct() const { return (attr & NodeAttr__TT_MASK) == NodeAttr_STRUCT; }
	bool isUnion() const { return (attr & NodeAttr__TT_MASK) == NodeAttr_UNION; }
	bool isCompound() const { return isClass() || isStruct() || isUnion(); }
	bool isUnsigned() const { return (attr & NodeAttr__TT_MASK) == NodeAttr_UNSIGNED; }
	bool isEnum() const { return (attr & NodeAttr__TT_MASK) == NodeAttr_ENUM; }
	bool isBasicType() const { return (attr & NodeAttr__BasicType) != 0; }
	bool isConst() const { return (attr & NodeAttr__CV_MASK) == NodeAttr_CONST; }

	bool isCDecl() const { return (attr & NodeAttr__CC_MASK) == NodeAttr_CDECL; }
	bool isStdCall() const { return (attr & NodeAttr__CC_MASK) == NodeAttr_STDCALL; }
	bool isThisCall() const { return (attr & NodeAttr__CC_MASK) == NodeAttr_THISCALL; }
	bool isFastCall() const { return (attr & NodeAttr__CC_MASK) == NodeAttr_FASTCALL; }

	bool isRegister() const { return (attr & NodeAttr__REGISTER_MASK) == NodeAttr_REGISTER; }
};

node_t *new_node(NodeTypeEnum n = NODE_NULL);
void delete_node(node_t *);
node_t *SymParse(const char *);
void SymParsePrint(node_t *, std::ostream &);
void SymParsePrintFlat(node_t *, std::ostream &);
void SymParsePrintProto(node_t *, std::ostream &);//new
#ifdef _DEBUG
void SymParseTest();
#endif

std::string FormatTemplatedType(const std::string &, int, const std::string &scope);


class ProtoImpl4Sym_t//for symbols (node_t)
{
	std::ostream &mos;
public:
	typedef node_t*	TYPEPTR;
	typedef node_t*	FIELDPTR;
	//typedef	node_t*	ARGPTR;
	typedef	node_t*	RETPTR;
	typedef	node_t* FUNCPTR;

	typedef TYPEPTR		type_ptr;
	typedef FIELDPTR	field_ptr;
	typedef		ProtoImpl4Sym_t	ARGDUMPER;
	struct ARGINFO {
		TYPEPTR pType;
		FIELDPTR pField;
		std::string sReg;
		ARGINFO(TYPEPTR t, FIELDPTR f) : pType(t), pField(f) {}
	};

public:
	ProtoImpl4Sym_t(std::ostream &os)
		: mos(os)
	{
	}
	ProtoImpl4Sym_t(const ProtoImpl4Sym_t&o)
		: mos(o.mos)
	{
	}
	bool IsDefinition() const {
		return false;
	}
	void dumpStr(const char *s, size_t n = 0){
		if (n == 0)
			mos << s;
		else
			mos.write(s, n);
	}
	void	dumpWStr(const wchar_t*, uint16_t){
		assert(0);
	}
	void dumpReserved(const char *s){
		dumpStr(s);
	}
	void dumpTab(){
		mos << "\t";
	}
	void dumpArrayIndex(unsigned i){
		mos << i;
	}
	type_ptr scope() const {
		return nullptr;
	}
	type_ptr baseType(type_ptr pn) const {
		switch (pn->type)
		{
		case NODE_TYPEDEF:
		case NODE_PTR:
		case NODE_REF:
		case NODE_RVAL_REF:
		case NODE_PTR_TO_MEMBER:
		case NODE_ARRAY:
			return pn->left;
		}
		return nullptr;
	}
	bool isSharedType(type_ptr p) const {
		if (p->type == NODE_FUNC)
			return false;
		return true;
	}
	bool isTypeOkToDump(type_ptr) const {
		return true;
	}
	void dumpTypeRef(type_ptr p){
		if (p->isConst())
			mos << "const ";
		mos << p->name;
	}
	void dumpBasicType(type_ptr p){
		if (p)
		{
			if (p->isConst())
				mos << "const ";
			assert(!p->name.empty());
			mos << p->name;
		}
		else
			mos << "void";
	}
	void drawFieldName(field_ptr p){
		if (p->type == NODE_NULL)
			mos << p->name;
	}
	bool isPointerType(type_ptr p) const {
		switch (p->type)
		{
		case NODE_PTR:
		case NODE_REF:
		case NODE_RVAL_REF:
		case NODE_PTR_TO_MEMBER:
			return true;
		}
		return false;
	}
	bool isArrayType(type_ptr p) const { return p->type == NODE_ARRAY; }
	bool isFunctionType(type_ptr p) const { return p->type == NODE_FUNC; }
	bool isSpecialPointerType(type_ptr p) const { return false; }
	bool isReferenceType(type_ptr p) const { return p->type == NODE_REF; }
	bool isRvalReferenceType(type_ptr p) const { return p->type == NODE_RVAL_REF; }
	bool isExcludedType(type_ptr p) const { return false; }
	bool isBasicType(type_ptr p) const { return p->isBasicType(); }
	bool isCompoundType(type_ptr p) const { return false; }//?
	bool isEnumerationType(type_ptr p) const { return false; }//?
	bool isConstantType(type_ptr p) const { return false; }
	bool isTypedefType(type_ptr p) const { return false; }
	unsigned arrayIndex(type_ptr p) const {
		return atoi(p->name.c_str());
	}

	bool isArgVisible(field_ptr p){
		return (p->type == NODE_NULL);
	}
	type_ptr fieldScope(field_ptr) const { return nullptr; }
	bool useTabSeparator(field_ptr){ return false; }
	bool isRegister(const ARGINFO &a) const { return (a.pField->attr & NodeAttr_REGISTER) != 0; }
	bool isBitfield(field_ptr) const { return false; }

	bool getFunctionArguments(node_t *p, std::vector<ARGINFO> &args)
	{
		if (p->right)
		{
			for (node_t *pa(p->right);; pa = pa->right)
			{
				if (pa->type != NODE_COMMA)
				{
					if (pa->name == "...")
						return true;//more args may follow (varargs)
					args.push_back(ARGINFO(pa, pa));
					break;
				}
				args.push_back(ARGINFO(pa->left, pa->left));
			}
		}
		return false;
	}

	void getFunctionReturnValues(node_t *p, std::vector<ARGINFO> &rets)
	{
		assert(p->type == NODE_FUNC);
		if (p->left)
		{
			rets.push_back(ARGINFO(p->left, p->left));
		}
	}
	
	type_ptr fieldType(field_ptr p) const {
		return p->left;
	}//?
	//type_ptr argumentType(field_ptr p) const { return (p->type == NODE_NULL) ? p->left : p; }
	//type_ptr returnType(ret_ptr p) const { return p; }

	//bool isFunctionIntrinsic(type_ptr p) const { return false; }
	bool isFunctionStdCall(type_ptr p) const {
		return p->isStdCall(); 
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


