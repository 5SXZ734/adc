#include "db/type_obj.h"
#include "front_dc.h"

#include <assert.h>

#include "shared/defs.h"
#include "shared/misc.h"

#include "arglist.h"



const char *FDC_t::AUXREG_reg2str(unsigned sz, int ofs, int flags, char buf[80])
{
	MyString s("T");
	switch (sz)
	{
	case OPSZ_TWORD: s.append("T"); break;
	case OPSZ_QWORD: s.append("Q"); break;
	case OPSZ_DWORD: s.append("D"); break;
	case OPSZ_WORD: s.append("W"); break;
	case OPSZ_BYTE: s.append("B"); break;
	default: s.append("?"); break;
	}
	assert(ofs >= 0);
	s.append("@");
	s.append(Int2Str(ofs, I2S_HEXA));

	strcpy(buf, s.c_str());
	return buf;
}


/*static int8_t auxreg_id2offs(uint8_t sz, uint8_t id)
{
	assert(id > 0);
	id -= 1;
	return ((id&3)<<3) + (((id>>2)&3)!=0)*sz;
}

static bool auxreg_offs2id(uint8_t sz, int8_t offs, uint8_t &id)
{
	assert(false);
//	id = (offs>>3) + 1;
	return true;
}*/

const char *FDC_t::flagsToStr(SSID_t ssid, unsigned flags, char sep) const
{
	static std::string s;//BAD!
	const STORAGE_t &st(fe().STORAGE[ssid]);
	if (st.isFlag() && flags != 0)
	{
		s.clear();
		//s = st.name;
		//s += '(';
		const R2S_t &m(mRegLookup.rev());
		RegInfo_t r0 = { nullptr, (unsigned)ssid, 0, 0 };
		int count(0);
		for (R2S_t::const_iterator i(m.lower_bound(&r0)); i != m.end() && (*i)->opc == ssid && flags != 0; ++i)
		{
			const RegInfo_t &r(*(*i));
			unsigned f(BITMASK(r.opsz) << r.offs);
			if ((flags & f) == f)
			{
				if (count++ > 0)
					s += sep;//'|';
				s.append(r.name);
				flags &= ~f;//clear bits
			}
		}
		//s += ')';
	}
	return s.c_str();
}

bool FDC_t::flagsToArgList(SSID_t ssid, FlagMaskType flags, Arg1List_t &l) const
{
	const STORAGE_t &st(fe().STORAGE[ssid]);
	if (!st.isFlag())
		return false;

	if (flags != 0)
	{
		const R2S_t &m(mRegLookup.rev());
		RegInfo_t r0 = { nullptr, (unsigned)ssid, 0, 0 };
		for (R2S_t::const_iterator i(m.lower_bound(&r0)); i != m.end() && (*i)->opc == ssid && flags != 0; ++i)
		{
			const RegInfo_t &r(*(*i));
			FlagMaskType f(BITMASK(r.opsz) << r.offs);
			if ((flags & f) == f)
			{
				l.push_back(Arg1_t((OPC_t)r.opc, r.offs, (OpType_t)r.opsz));
				flags &= ~f;//clear bits
			}
		}
	}
	return true;
}

const char *FDC_t::regName(SSID_t ssid, int ofs, int sz, unsigned flags, char buf[80]) const
{
	switch (ssid)
	{
	case SSID_GLOBAL:
		assert(0);
	case SSID_LOCAL:
		assert(0);
	case SSID_AUXREG:
		return AUXREG_reg2str(sz, ofs, flags, buf);
	default:
		break;
	}
	
	const STORAGE_t &st(fe().STORAGE[ssid]);
	if (!st.isStacked())
		return mRegLookup.toRegName((OPC_t)ssid, ofs, sz);

	return mp->RegName(ssid, sz, ofs, flags, buf);
}


