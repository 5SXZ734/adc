#pragma once

//calling conventions support

#include "type_funcdef.h"
#include "info_func.h"
#include "front/front_IA.h"

/////////////////////////////////////////////////////////// x86 (Win23) Calling Convetion
//	On x86 plaftorms, all arguments are widened to 32 bits when they are passed.
//		Return values are also widened to 32 bits and returned in the EAX register,
//		except for 8-byte structures, which are returned in the EDX:EAX register pair.
//		Larger structures are returned in the EAX register as pointers to hidden return structures. Parameters are pushed onto the stack from right to left.
//		Structures that are not PODs will not be returned in registers.
//
//	__cdecl:
//		Stack cleanup: Caller
//		Parameter passing: Pushes parameters on the stack, in reverse order (right to left)
//
//	__stdcall:
//		Stack cleanup: Callee
//		Parameter passing: Pushes parameters on the stack, in reverse order (right to left)
//
//	__fastcall:
//		Specifies that arguments to functions are to be passed in registers, when possible. 
//		This calling convention only applies to the x86 architecture.
//		The first two DWORD or smaller arguments that are found in the argument list from left to right are passed in ECX and EDX registers;
//		all other arguments are passed on the stack from right to left.
//		Called function pops the arguments from the stack.
//
// __thiscall:
//		Stack cleanup: The callee cleans the stack, which is impossible for vararg functions.
//		Parameter passing: Arguments are pushed on the stack from right to left, with the this pointer being passed via register ECX, 
//		and not on the stack, on the x86 architecture.
//		vararg member functions: use the __cdecl calling convention. 
//			All function arguments are pushed on the stack, with the this pointer placed on the stack last

class CallingConv_t//for IA-32
{
	bool mbThisCall;
	bool mbFastCall;
	unsigned mPtrSize;
	const I_FrontDC& mrFDC;
public:
	CallingConv_t(const DcInfo_t&, CGlobPtr);
	CallingConv_t(const DcInfo_t&, CTypePtr);
	//CallingConv_t(const DcInfo_t &, bool bThisCall, bool bFastCall);
	SSID_t argSsid(int) const;
	int argOff(int) const;
	unsigned argSize2(int, unsigned) const;
	unsigned retAddrSize() const {
		return mPtrSize;
	}
	unsigned stackAddrSize() const {
		return mPtrSize;//?
	}
	SSID_t retSsid(int, OpType_t) const;
	unsigned retOff(int, OpType_t) const;
	//given argument/retval's type, determine how would it affect a FPU stack
	int8_t fpuDiff(uint8_t optyp) const;
	unsigned fflags() const;
};

//////////////////////////////////// FuncArgsIt
class FuncArgsIt : public FieldMapIt
{
	FieldMap &mrf;
public:
	FuncArgsIt(FuncDef_t &rf)
		: FieldMapIt(rf.argFields().begin()),
		mrf(rf.argFields())
	{
	}
	FuncArgsIt(FuncDef_t &rf, FieldPtr p)
		: FieldMapIt(p),
		mrf(rf.argFields())
	{
	}
	operator bool() const {
		return !isAtEnd();
	}
	FieldPtr take() {
		return mrf.take(*this);
	}
	FieldPtr field(){
		return operator->();
	}
	CFieldPtr field() const {
		return operator->();
	}
protected:
	void reset(){
		FieldMapIt& r(*this);
		r = mrf.begin();
	}
	bool isAtEnd() const {
		return *this == mrf.end() || (FuncInfo_s::order(field()) > LOCAL_ORDER_ARG_UPPER);
	}
};

/////////////////////////////////////// FuncSpoiltIt
class FuncSpoiltIt : public FieldMapIt
{
	FieldMap &mrf;
public:
	FuncSpoiltIt(FuncDef_t &rf, SSID_t _ssid = SSID_NULL)
		: FieldMapIt(rf.retFields().lower_bound(LOCAL_MAKE_KEY(LOCAL_ORDER_SPOILT, _ssid, 0))),
		mrf(rf.retFields())
	{
	}
	operator bool() const {
		return !isAtEnd();
	}
	FieldPtr field(){
		return operator->();
	}
	CFieldPtr field() const {
		return operator->();
	}
	bool isAtEnd() const {
		return *this == mrf.end() || (FuncInfo_s::order(field()) != LOCAL_ORDER_SPOILT);
	}
	bool isAtMapEnd() const {
		return *this == mrf.end();
	}
	FieldPtr take() {
		return mrf.take(*this);
	}
};

//////////////////////////////////// FuncArgsCIt
class FuncArgsCIt : public FieldMapCIt
{
	const FieldMap &mrf;
public:
	FuncArgsCIt(const FuncDef_t &rf)
		: FieldMapCIt(rf.argFields().begin()),
		mrf(rf.argFields())
	{
	}
	operator bool() const {
		return !isAtEnd();
	}
	FieldPtr field() const {
		return (FieldPtr)operator->();
	}
protected:
	void reset(){
		FieldMapCIt& r(*this);
		r = mrf.begin();
	}
	bool isAtEnd() const {
		return *this == mrf.end() || !FuncDef_t::isLocalArg(field());
	}
};

///////////////////////////////////// FuncArgsCRIt - reverse
class FuncArgsCRIt : public FieldMapCRIt
{
	const FieldMap &mrf;
public:
	FuncArgsCRIt(const FuncDef_t &rf)
		: FieldMapCRIt(rf.argFields().rbegin()),
		mrf(rf.argFields())
	{
		while (*this != mrf.rend())//skip rets
		{
			if (FuncDef_t::isLocalArg(field()))
				break;
			operator++();
		}
	}
	operator bool() const {
		return !isAtEnd();
	}
	FieldPtr field() const {
		return (FieldPtr)operator->();
	}
protected:
	void reset(){
		FieldMapCRIt& r(*this);
		r = mrf.rbegin();
	}
	bool isAtEnd() const {
		return *this == mrf.rend() || !FuncDef_t::isLocalArg(field());
	}
};

///////////////////////////////////// FuncCCArgsCIt
// funcdef argument iterator
template <typename T = FuncArgsCIt>
class FuncCCArgsCIt : public T
{
	const CallingConv_t &mrcc;
	int	mIndex;
	unsigned mStackInTotal;
	unsigned mFpuInTotal;
public:
	using T::field;
	using T::isAtEnd;
public:
	FuncCCArgsCIt(const FuncDef_t &rf, const CallingConv_t &rcc)
		: T(rf),
		mrcc(rcc),
		mIndex(0),
		mStackInTotal(0),
		mFpuInTotal(0)
	{
		updateStates();
	}
	void reset()
	{
		T& r(*this);
		FuncArgsCIt::reset();
		mIndex = 0;
		mStackInTotal = 0;
		mFpuInTotal = 0;
		updateStates();
	}
	FuncCCArgsCIt &operator++(){
		T::operator++();
		++mIndex;
		updateStates();
		return *this;
	}
	SSID_t ssid(bool bCheck = false) const {
		CFieldPtr p(field());
		SSID_t _ssid(FuncInfo_s::SSIDx(p));
		if (bCheck)
			_ssid = SSID_NULL;
		if (_ssid != SSID_NULL)
			return _ssid;
		if (FuncInfo_s::order(p) == LOCAL_ORDER_REG)
			if (p->isTypeSimple(OPTYP_REAL64))
				return SSID_FPUREG;
		return mrcc.argSsid(mIndex);
	}
	int offset(bool bCheck = false) const {
		CFieldPtr p(field());
		SSID_t _ssid(FuncInfo_s::SSIDx(p));
		if (bCheck)
			_ssid = SSID_NULL;
		if (_ssid != SSID_NULL)//reg
			return FuncInfo_s::address(p);
		if (FuncInfo_s::order(p) == LOCAL_ORDER_REG)
			if (p->isTypeSimple(OPTYP_REAL64))
				return (mFpuInTotal - 1) * FR_SLOT_SIZE;//opsize();//fpuin total has been advanced
		if (mrcc.argSsid(mIndex) != SSID_LOCAL)
			return mrcc.argOff(mIndex);
		return mrcc.retAddrSize() + mStackInTotal - stackSizeAdjusted(opsize());
	}
	unsigned size() const {
		return field()->size();
	}
	unsigned size2() const {
//?		SSID_t e(field()->SSID());
//?		if (e == SSID_LOCAL || e == SSID_FPUREG)
//?			return field()->OpTypeZ();//?
		if (ssid() == SSID_FPUREG)
			return OPSIZE(OPTYP_REAL80);
		unsigned sz(mrcc.argSize2(mIndex, field()->size()));
		if (ssid() == SSID_LOCAL)
			sz = stackSizeAdjusted(sz);
		return sz;
	}
	uint8_t optype() const {
		CTypePtr p(ProjectInfo_t::SkipModifier(field()->type()));
		if (p)
		{
			if (ssid() == SSID_FPUREG)
				return OPTYP_REAL80;
			if (p->typeSimple())
				return p->typeSimple()->optype();
			if (p->size() <= OPSZ_QWORD)//allow small structs as params
				return (uint8_t)p->size();
		}
		return 0;
	}
	unsigned stackInTotal() const {
		return mStackInTotal;
	}
	unsigned fpuInTotal() const {
		return mFpuInTotal;
	}
	ri_t toR_t(bool bCheck = false) const {
		ri_t r;
		r.ssid = ssid(bCheck);
		r.ofs = offset(bCheck);
		r.typ = (OpType_t)optype();
		return r;
	}
private:
	void updateStates()
	{
		if (isAtEnd())
			return;
		if (ssid() == SSID_LOCAL)
		{
			unsigned u(opsize());
			mStackInTotal += stackSizeAdjusted(u);
		}
		else if (ssid() == SSID_FPUREG)
		{
			unsigned u(opsize());
			mFpuInTotal += 1;//u;
		}
	}
	unsigned opsize() const {
		unsigned u(optype() & OPSZ_MASK);
		if (u == 0)
			u = mrcc.stackAddrSize();//?
		return u;
	}
	unsigned stackSizeAdjusted(unsigned sz) const
	{
		unsigned u(mrcc.stackAddrSize());
		if (sz % u != 0)
			sz = (sz / u) * u + u;
		return sz;
	}
};



//func type argument iterator
class FuncTypeArgsCIt
{
	const TypeFunc_t &mrf;
	CTypePtr mpArg;//current
protected:
	int mIndex;
public:
	FuncTypeArgsCIt(CTypePtr f)
		: mrf(*f->typeFunc()),
		mpArg(mrf.args()),
		mIndex(0)
	{
	}
	void reset(){
		mpArg = mrf.args();
		mIndex = 0;
	}
	operator bool() const {
		return mpArg != nullptr;
	}
	FuncTypeArgsCIt &operator++()
	{
		mpArg = mpArg->typePair() ? mpArg->typePair()->right() : nullptr;
		++mIndex;
		return *this;
	}
	CTypePtr operator*() const {
		return mpArg->typePair() ? mpArg->typePair()->left() : mpArg;
	}

	unsigned size() const {
		return operator*()->size();
	}
	uint8_t optype() const {
		assert(operator*()->typeSimple());
		return operator*()->typeSimple()->optype();
	}
};

//func type argument iterator (with calling convention)
class FuncTypeArgsCIt2 : public FuncTypeArgsCIt
{
	const CallingConv_t& mrcc;

public:
	FuncTypeArgsCIt2(CTypePtr f, const CallingConv_t& rcc)
		: FuncTypeArgsCIt(f),
		mrcc(rcc)
	{
	}
	SSID_t ssid() const {
		return mrcc.argSsid(mIndex);
	}
	int offset() const {
		return mrcc.argOff(mIndex);
	}
};

//typedef FieldMap ArgsMap;
/*: public FieldMap
{
	//bool mbThisCall;
	//bool mbFastCall;
public:
	ArgsMap()
	//: mbThisCall(false),
	//mbFastCall(false)
	{}
	//void setThisCall(bool b){ mbThisCall = b; }
	//void setFastCall(bool b){ mbFastCall = b; }
	//bool isThisCall() const { return mbThisCall; }
	//bool isFastCall() const { return mbFastCall; }
};
*/


class FuncRetsIt : public FieldMapRIt
{
	FieldMap &mr;
public:
	FuncRetsIt(FuncDef_t &rf)
		: FieldMapRIt(rf.retFields().rbegin()),
		mr(rf.retFields())
	{
	}
	operator bool() const {
		return *this != mr.rend();//&& FuncDef_t::isRetVal(field());
	}
	FieldPtr take() {
		return mr.take(*this);
	}
	FieldPtr field(){
		return operator->();
	}
	FieldPtr field() const {
		return (FieldPtr)operator->();
	}
	bool isAtEnd() const {
		return *this == mr.rend() || !FuncDef_t::isRetVal(field());
	}
};

class FuncRetsCIt : public FieldMapCRIt
{
	const FieldMap &mr;
public:
	FuncRetsCIt(const FuncDef_t &rf)
		: FieldMapCRIt(rf.retFields().rbegin()),
		mr(rf.retFields())
	{
	}
	operator bool() const {
		return *this != mr.rend();//&& FuncDef_t::isRetVal(field());
	}
	FieldPtr field() const {
		return (FieldPtr)operator->();
	}
	bool isAtEnd() const {
		return *this == mr.rend() || !FuncDef_t::isRetVal(field());
	}
};

//rets iter with calling convention
class FuncCCRetsCIt : public FuncRetsCIt
{
	const CallingConv_t& mrcc;
	int mIndex;
public:
	FuncCCRetsCIt(const FuncDef_t& rf, const CallingConv_t& cc)
		: FuncRetsCIt(rf),
		mrcc(cc),
		mIndex(0)
	{
	}
	FuncCCRetsCIt& operator++() {
		FuncRetsCIt::operator++();
		++mIndex;
		//updateStates();
		return *this;
	}
	ri_t toR_t(bool bCheck = false) const
	{
		ri_t r;
		r.ssid = ssid(bCheck);
		r.ofs = offset(bCheck);
		r.typ = optyp();
		return r;
	}
	SSID_t ssid(bool bCheck = false) const {
		CFieldPtr p(field());
		SSID_t _ssid(FuncInfo_s::SSIDx(p));
		if (bCheck)
			_ssid = SSID_NULL;
		if (_ssid == SSID_NULL)
		{
			if (p->isTypeSimple(OPTYP_REAL64))
				_ssid = SSID_FPUREG;
			else
				_ssid = mrcc.retSsid(mIndex, optyp());
		}
		return _ssid;
	}
	int offset(bool bCheck = false) const {
		CFieldPtr p(field());
		SSID_t _ssid(FuncInfo_s::SSIDx(p));
		if (bCheck)
			_ssid = SSID_NULL;
		if (_ssid == SSID_NULL)
			return mrcc.retOff(mIndex, optyp());
		return FuncInfo_s::address(p);
	}
	OpType_t optyp() const {
		CFieldPtr p(field());
		return p->OpTypeZ();
	}
};



