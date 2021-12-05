
#include "cc.h"
#include "front/front_IA.h"
#include "info_func.h"

//////////////////////////////////// CallingConv_t
CallingConv_t::CallingConv_t(const DcInfo_t& di, CGlobPtr g)
	: mPtrSize(di.PtrSize()),
	mbThisCall(ProtoInfo_s::IsThisCallType(g)),
	mbFastCall(g->typeFuncDef()->isFastCall()),
	mrFDC(di.FrontDC())
{
}

CallingConv_t::CallingConv_t(const DcInfo_t& di, CTypePtr p)//bool bThisCall, bool bFastCall)
	: mPtrSize(di.PtrSize()),
	mbThisCall(p->typeFunc()->isThisCall()),
	mbFastCall(p->typeFunc()->isFastCall()),
	mrFDC(di.FrontDC())
{
}

unsigned CallingConv_t::fflags() const
{
	unsigned f(0);
	if (mbThisCall)
		f |= I_FrontDC::F_ThisCall;
	if (mbFastCall)
		f |= I_FrontDC::F_FastCall;
	return f;
}

SSID_t CallingConv_t::argSsid(int index) const
{
	return mrFDC.ArgSSID(index, fflags());
}

SSID_t CallingConv_t::retSsid(int index, OpType_t retType) const
{
	return mrFDC.RetSSID(index, retType);
}

unsigned CallingConv_t::retOff(int index, OpType_t retType) const
{
	return mrFDC.RetOff(index, retType);
}

int CallingConv_t::argOff(int index) const
{
	return mrFDC.ArgOff(index, fflags());
}

unsigned CallingConv_t::argSize2(int index, unsigned u) const
{
	return mrFDC.ArgSize(index, u);
}

//given argument/retval's type, determine how would it affect a FPU stack
int8_t CallingConv_t::fpuDiff(uint8_t optyp) const
{
	if (OPTYP_IS_REAL(optyp))
		return (-1) * FTOP_STEP;//decrease by one FPU slot
	return 0;
}


