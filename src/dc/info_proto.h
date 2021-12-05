#pragma once

#include "info_dc.h"

class FuncInfo_t;

class ProtoInfo_s : public DcInfo_s
{
public:
	//static void	SetFuncInvalid(GlobPtr);//some error
	static bool SetFuncStatus(GlobPtr, unsigned status);
	static unsigned FuncStatus(CGlobPtr);
	static bool IsFuncStatusFinished(CGlobPtr);//Ok
	static bool	IsFuncStatusAborted(CGlobPtr);
	static bool	IsFuncStatusProcessing(CGlobPtr);
	static bool IsFuncDecompiled(CGlobPtr g) { return (g->flags() & FDEF_STATUS_MASK) != 0; }//was ever?
	static bool IsStub(CGlobPtr);
	static bool IsStubUndefined(CGlobPtr);

	static bool	IsFuncVarArged(CGlobPtr);
	static void SetFuncVarArged(GlobPtr, bool);
	static bool	IsFuncCleanArged(CGlobPtr);
	static void SetFuncCleanArged(GlobPtr, bool);
	static void SetFuncUserCall(GlobPtr, bool);
	static bool	IsFuncUserCall(CGlobPtr);
	static bool IsThisCallType(CGlobPtr);
	static FieldPtr	ThisPtrArg(CGlobPtr);

	static bool	IsThisPtrArg(CFieldPtr);
	static bool MakeThisPtrArg(FieldPtr arg);
	static bool IsOpOverlappedByField(const ri_t& source, const ri_t& target);
	static bool IsSpoiltReg(CFieldPtr);
	static bool IsRetVal(CFieldPtr);
	static bool IsLocalArgAny(CFieldPtr);//including spoilts and rets
};

class ProtoInfo_t : public DcInfo_t, public ProtoInfo_s
{
	GlobObj_t& mrFuncDefRef;
	FuncDef_t& mrFuncDef;

public:
	ProtoInfo_t(const DcInfo_t&, CGlobPtr);
	ProtoInfo_t(const FuncInfo_t&);
	//ProtoInfo_t() {}

	uint16_t		StackIn() const;
	int			FpuIn() const;

	GlobPtr	FuncDefPtr() const { return &mrFuncDefRef; }
	GlobObj_t& FuncDefRef() const { return mrFuncDefRef; }
	FuncDef_t& FuncDef() const { return mrFuncDef; }

	TypePtr OwnerClass() const { return OwnerScope(FuncDefPtr()); }
	bool IsFuncCleanArged() const { return ProtoInfo_s::IsFuncCleanArged(FuncDefPtr()); }
	bool IsFuncVarArged() const { return ProtoInfo_s::IsFuncVarArged(FuncDefPtr()); }
	bool UnmakeThisCall() const;
	bool IsThisCallType() const;
	bool IsClassMember() const;
	bool IsConstClassMember() const;//IsThisCallType applied as well
	FieldPtr FirstArg() const;
	FieldPtr DockField() const;

	void FromFuncProfile(const FuncProfile_t& fp) const;
	FieldPtr AddRetField(FieldPtr) const;
	FieldPtr AddArgField(FieldPtr) const;
	void AddToSpoiltList(SSID_t, REG_t, FuncArgsMap* = nullptr) const;
	int CreateArgList(const Arg1List_t&, bool bThiscall) const;
	int CreateRetList(const Arg1List_t&) const;
	int CreateSpoiltList(const GPRs_t&) const;

	FieldPtr AddRetField(OpType_t optype, uint8_t opc, int offset) const;

	void GetFuncProfile(FuncProfile_t&) const;
	void GetArgProfile(ProtoProfile_t&) const;

	bool InsertLocalArg(FieldPtr);

	enum AppendFuncArgFlags { AFA_NULL, AFA_THISPTR, AFA_REGISTER };
	FieldPtr AppendFuncArg(TypePtr, AppendFuncArgFlags);
	FieldPtr AppendFuncRet(TypePtr);

	void ClearLocalType(FieldPtr) const;
	FieldPtr FindArgFromOp(const ri_t&) const;

	int PStackPurge() const;
	bool IsIntrinsic() const;

	FieldPtr RepositionArg(FieldPtr, const REG_t&) const;//or ret/spoilt
	void ApplyDefaultSpoiltRegs() const;
	void ApplyCalleeSpoiltRegs(CGlobPtr) const;

	MyString LocalName(CFieldPtr pSelf) const;
	bool LocalAutoName(CFieldPtr, MyString&) const;
};

