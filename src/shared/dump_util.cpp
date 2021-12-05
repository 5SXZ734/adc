
#include <assert.h>
#include <bitset>
#include <iomanip>

#include "defs.h"
#include "misc.h"
#include "dump_util.h"

#ifndef WIN32
char* strlwr(char* s)
{
    char* tmp = s;

    for (;*tmp;++tmp) {
        *tmp = tolower((unsigned char) *tmp);
    }

    return s;
}
char* strupr(char* s)
{
    char* tmp = s;

    for (;*tmp;++tmp) {
        *tmp = toupper((unsigned char) *tmp);
    }

    return s;
}
#endif


void DumpUtil_t::set_color(int col)
{
	if (col)
		mos.color(col);
}

void DumpUtil_t::out_bitset0(const unsigned char *p, uint8_t byteOffs, uint8_t width0)
{
	uint8_t width(width0);
	assert(width > 0);
	assert(byteOffs < CHAR_BIT);

	if (p)
	{
		std::string s;
		while (width-- > 0)
		{
			unsigned char byte(*(char *)p);
			if (((byte >> byteOffs) & 1))
				s.insert(0, "1");//mos << "1";
			else
				s.insert(0, "0");//mos << "0";
			if (++byteOffs == CHAR_BIT)
			{
				p++;
				byteOffs = 0;
			}
		}
		mos << s;
		if (width0 > 1)
			mos << "b";

	}
	else
	{
		if (width0 > 1)
		{
			mos << (unsigned)width << " dup(?)";
		}
		else
		{
			std::string s;
			while (width-- > 0)
			{
				s.insert(0, "?");
				if (++byteOffs == CHAR_BIT)
					byteOffs = 0;
			}
			mos << s;
		}
	}
}



void DumpUtil_t::out_bitset(const void *p, unsigned bytesNum)
{
	const char* beg = reinterpret_cast<const char*>(p);
	const char* end = beg + bytesNum;
	while (beg < end)
	{
		mos << std::bitset<CHAR_BIT>(*(--end)) << "b";
		if (end > beg)
			mos << ' ';
	}
}

void DumpUtil_t::out_hex(const void *p, int type)
{
	uint8_t sz(type & OPSZ_MASK);

	std::ios::fmtflags f(std::ios::uppercase);
	mos.flags(std::ios::hex | f);
	mos << std::setfill ('0');

	if (sz == OPSZ_BYTE)
		mos << std::setw(2) << *(uint8_t*)p;
	else if (sz == OPSZ_WORD)
		mos << std::setw(4) << *(uint16_t*)p;
	else if (sz == OPSZ_DWORD)
		mos << std::setw(8) << *(uint32_t*)p;
	else if (sz == OPSZ_QWORD)
		mos << std::setw(16) << *(int64_t*)p;
	else
		ASSERT0;

	mos << "h";
}

void DumpUtil_t::out_imm(const void *p, uint8_t optyp, bool _signed, bool bPlus, unsigned base)
{
	uint8_t opsz(optyp & OPSZ_MASK);

	if (OPTYP_IS_REAL(optyp))
	{
		if (opsz == OPSZ_DWORD)
			mos << *(const float *)p;
		else if (opsz == OPSZ_QWORD)
			mos << *(const double *)p;
		else
			ASSERT0;
		return;
	}

	assert(base != 2);

	std::ios::fmtflags f(std::ios::uppercase);
	if (base == 16)
		mos.flags(std::ios::hex | f);
	else if (base == 10)
		mos.flags(std::ios::dec | f);
	else if (base == 8)
		mos.flags(std::ios::oct | f);
	//else if (base == 2)
		//mos.flags(std::ios::bin | f);

	uint64_t v;

	bool minus = false;
	if (_signed)
	{
		int64_t i;
		if (opsz == OPSZ_BYTE)
			i = *(int8_t*)p;
		else if (opsz == OPSZ_WORD)
			i = *(int16_t*)p;
		else if (opsz == OPSZ_DWORD)
			i = *(int32_t*)p;
		else if (opsz == OPSZ_QWORD)
			i = *(int64_t*)p;
		else
		{
			assert(0);
		}
		if (i < 0)
		{
			minus = true;
			v = -i;
		}
		else
			v = i;
	}
	else
	{
		if (opsz == OPSZ_BYTE)
			v = *(uint8_t*)p;
		else if (opsz == OPSZ_WORD)
			v = *(uint16_t*)p;
		else if (opsz == OPSZ_DWORD)
			v = *(uint32_t*)p;
		else if (opsz == OPSZ_QWORD)
		{
			char buf[40];
			uint64_t v(*(uint64_t *)p);
			if (!i64toStr(v, buf, 16))
				mos << "?";
			else
				strupr(buf);
			uint64_t v2(v);

			if (base == 16)
			{
				uint8_t d;
				for (;;)
				{
					d = (uint8_t)(v2 & 0xF);
					v2 = v2 >> 4;
					if (v2 == 0)
						break;
				}
				if (d > 9)
					mos << "0";
				mos << buf;
				if (v > 9)
					mos << "h";
			}
			else
				mos << buf;
			return;
		}
		else
		{
			mos << "!" << (unsigned)opsz << "!";
			//assert(false);
			return;
		}
	}

	uint64_t v2(v);

	if (minus)
		mos << "-";
	else if (bPlus)
		mos << "+";

	uint8_t d;
	if (base == 16)//chek if leading digit is an alpha
	{
		for (;;)
		{
			d = (uint8_t)(v2 & 0xF);
			v2 = v2 >> 4;
			if (v2 == 0)
				break;
		}
		if (d > 9)
			mos << "0";
		mos << v;
		if (v > 9)
			mos << "h";
	}
	else
		mos << v;
	return;
}

void DumpUtil_t::out_bytes(const uint8_t *pbytes, unsigned len)
{
	mos.flags(std::ios::hex|std::ios::uppercase);
	mos.fill('0');
	for (unsigned i(0); i < len; i++, pbytes++)
	{
		mos.width(2);
		mos << (unsigned int)*pbytes;//ins.bytes[i];   // print out byte
	}
}

void DumpUtil_t::out_value(const VALUE_t &v, std::ostream &os, int iColor, OutFormat fmt)
{
	uint8_t optyp(v.typ);
	if (optyp == OPTYP_NULL)
		optyp = OPSZ_BYTE;
	uint8_t size0(OPSIZE(optyp));
	int colornum(0);
	if (iColor != -1)
		set_color(iColor);

	if (fmt == OUT_BINARY)
	{
		out_bitset(&v.ui8, size0);
	}
	else if (fmt == OUT_OCTAL)
	{
		out_imm(&v.ui8, size0, false, false, 8);
	}
	else if (fmt == OUT_DECIMAL)
	{
		bool bSigned(false);
		if (OPTYP_IS_SINT(v.typ))
		{
			switch (size0)
			{
			case OPSZ_BYTE: bSigned = (v.i8 < 0); break;
			case OPSZ_WORD: bSigned = (v.i16 < 0); break;
			case OPSZ_DWORD: bSigned = (v.i32 < 0); break;
			case OPSZ_QWORD: bSigned = (v.i64 < 0); break;
			default: break;
			}
		}
		out_imm(&v.ui8, size0, bSigned, false, 10);
	}
	else if (fmt == OUT_HEX)
	{
		out_hex(&v.ui8, size0);
	}
	else if (fmt == OUT_ASCII && size0 == OPSZ_BYTE)
	{
		std::string s(MyStringf("'%c'", v.i8));
		out_string(s);
	}
	else
	{
		out_imm(&v.ui8, optyp, OPTYP_IS_SINT(v.typ), false, 16);
	}

	if (iColor != -1)
		set_color(adcui::COLOR_POP);//pop back
}




