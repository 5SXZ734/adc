#pragma once

#include "shared/front.h"

class Arg1List_t;

class FDC_t//frontDC holder
{
	I_FrontDC	*mp;
	RegInfoLookup_t mRegLookup;
public:
	FDC_t()
		: mp(nullptr)
	{
		set(nullptr);
	}
	void set(I_FrontDC *p)
	{
		if (mp)
		{
			mRegLookup.clear();
			mp->Release();
		}
		mp = p;
		if (mp)
		{
			mp->AddRef();
			mRegLookup.create(mp->RegInfoTable());
		}
	}
	I_FrontDC *get() const {
		return mp;
	}
	const FE_t &fe() const {
		return *mp->GetFE();
	}
	const RegInfo_t *fromReg(OPC_t opc, unsigned ofs) const {
		return mRegLookup.fromReg(opc, ofs);
	}
	const RegInfo_t *fromReg(OPC_t opc, unsigned ofs, unsigned opsz) const {
		return mRegLookup.fromReg(opc, ofs, opsz);
	}
	bool exists(OPC_t opc, unsigned ofs, unsigned opsz) const {
		return mRegLookup.fromReg(opc, ofs, opsz) != nullptr;
	}
	const RegInfo_t *fromRegName(const char *s, OPC_t opc = OPC_NULL) const {
		return mRegLookup.fromRegName(s, opc);
	}
	bool contains(const char *s, OPC_t opc = OPC_NULL) const {
		return mRegLookup.fromRegName(s, opc) != nullptr;
	}
	const char *regName(SSID_t, int ofs, int sz, unsigned flags, char buf[80]) const;
	const char *flagsToStr(SSID_t, unsigned flags, char sep = '|') const;
	bool flagsToArgList(SSID_t, FlagMaskType flags, Arg1List_t &) const;

private:
	static const char* AUXREG_reg2str(unsigned sz, int ofs, int flags, char buf[80]);
};








