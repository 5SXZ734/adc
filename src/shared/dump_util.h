#pragma once

#include <ostream>
#include <string>
#include <strstream>

#include "interface/IADCGui.h"

struct VALUE_t;

//////////////////////////////////////////
class MyStrStream : public std::strstream
{
	bool	mbColors;
	int		esc_count;//escaped symbols so far
public:
	MyStrStream(bool bColors = false)
		: mbColors(bColors),
		esc_count(0)
	{
	}
	void flush(std::string &s)
	{
		int l((int)pcount());
		s.append(str(), l);//str() fails to return a complete string with trailing zeros!
		freeze(false);
		seekp(0);//reset
		esc_count = 0;
	}
	void color(int iColor)
	{
		if (mbColors)
			(*this) << (char)MAKECOLORID(iColor);
	}
	void write(const std::string &s)
	{
		std::strstream::write(s.data(), s.length());
	}
};


//////////////////////////////////// DumpUtil_t
class DumpUtil_t
{
protected:
	MyStrStream&	mos;

public:
	DumpUtil_t(MyStrStream& os)
		: mos(os)
	{
	}

	void	set_color(int col);

	void	out_hex(const void *p, int type);
	void	out_imm(const void *p, uint8_t type, bool _signed, bool drawsign, unsigned base);
	void	out_bitset0(const unsigned char *, uint8_t byteOffs, uint8_t width);
	void	out_bitset(const void *p, unsigned bytesNum);
	void	out_string(const std::string &s){ mos << s; }
	void	out_bytes(const uint8_t *pbytes, unsigned len);

	enum OutFormat {
		OUT_NULL,
		OUT_HEX,
		OUT_DECIMAL,
		OUT_BINARY,
		OUT_OCTAL,
		OUT_ASCII
	};

	void	out_value(const VALUE_t &, std::ostream &, int color = -1, OutFormat = OUT_NULL);
};




