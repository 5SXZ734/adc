
#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdarg.h>

#include "qx/MyString.h"

#include "defs.h"
#include "misc.h"

/////////////////////////////////

/*static const TypStr_t TypStrZ[] = {
	{OPTYP_NULL,		"?",	"?"		},	//0
	{OPTYP_REAL0,		"Real",	"r"		},	//0x20
	{OPTYP_BCD0,			"BCD",	"b"		},	//0x30
	{OPTYP_INTEGER,		"?Int",	"?i"	},	//0x10
	{OPTYP_UINT0,	"UInt",	"ui"	},	//0x80
	{OPTYP_INT0,		"Int",	"i"		},	//0x80
	{OPTYP_PTR,			"Ptr",	"p"		},	//0x40
	{-1,				"???",	"???",	},
};

static const TypStr_t SizStr[] = {//type size
	{OPSZ_NULL,		"?",		"?"},
	{OPSZ_BYTE,			"Byte",		"DB"},	
	{OPSZ_WORD,			"Word",		"DW"},	
	{OPSZ_DWORD,		"DWord",	"DD"},	
	{OPSZ_FWORD,		"FWord",	"DF"},	
	{OPSZ_QWORD,		"QWord",	"DQ"},
	{OPSZ_TWORD,		"TWord",	"DT"},
	{-1,				"???",		"???"},
};

static const TypStr0_t TypeDefs[] = {
	{OPTYP_NULL,	"void",				{0}},
	{OPTYP_INT8,	{"char"},				{0}},
	{OPTYP_UINT8,	{"unsigned char"},		{0}},
	{OPTYP_INT16,	{"short"},				{0}},
	{OPTYP_UINT16,	{"unsigned short"},		{0}},
	{OPTYP_INT32,	{"int"},				{0}},
	{OPTYP_UINT32,	{"unsigned"},			{0}},
	{OPTYP_INT64,	{"__int64"},			{0}},
	{OPTYP_UINT64,	{"unsigned __int64"},	{0}},
	{OPTYP_REAL32,	{"float"},				{0}},
	{OPTYP_REAL64,	{"double"},				{0}},
	{OPSZ_BYTE,		{0},					{"BYTE"}},
	{OPSZ_WORD,		{0},					{"WORD"}},
	{OPSZ_DWORD,	{0},					{"DWORD"}},
	{OPSZ_QWORD,	{0},					{"QWORD"}},
	{-1}
};*/

#include <map>

class OpType2Str_t : public std::vector<MyString>
{
	std::map<MyString, OpType_t> m_rev;
public:
	OpType2Str_t()
	{
		set(OPTYP_NULL, "~UNK_");
		set(OPSZ_BYTE, "~BYTE_");
		set(OPSZ_WORD, "~WORD_");
		set(OPSZ_DWORD, "~DWORD_");
		//OPSZ_FWORD
		set(OPSZ_QWORD, "~QWORD_");
		set(OPSZ_TWORD, "~TWORD_");
		
//		set(OPTYP_BIT, "~BIT");

		//reals
		set(OPTYP_REAL0, "~REAL0_");	//unknown size
		set(OPTYP_REAL32, "float");
		set(OPTYP_REAL64, "double");
		set(OPTYP_REAL80, "~REAL80_");
		//OPTYP_BCD0
		//OPTYP_BCD80

		//integers (may be signed or unsigned) 
		set(OPTYP_INTEGER, "~INT0_");	// unknown size
		set(OPTYP_BYTE, "~INT8_");
		set(OPTYP_WORD, "~INT16_");
		set(OPTYP_DWORD, "~INT32_");
		set(OPTYP_QWORD, "~INT64_");

		//unsigned integers
		set(OPTYP_UINT0, "~UNSIGNED0_");	//unknown size
		set(OPTYP_UINT8, "unsigned char");
		set(OPTYP_BOOL, "bool");
		set(OPTYP_UINT16, "unsigned short");
		set(OPTYP_UINT32, "unsigned int");
		set(OPTYP_UINT32, "unsigned long");//?
		set(OPTYP_UINT64, "unsigned __int64");

		//signed integers
		set(OPTYP_INT0, "~SIGNED0_");	//unknown size
		set(OPTYP_INT8, "int8_t");
		set(OPTYP_INT16, "short");
		set(OPTYP_INT32, "int");
		set(OPTYP_INT32, "long");
		set(OPTYP_INT64, "__int64");

		set(OPTYP_CHAR8, "char");
		set(OPTYP_CHAR16, "char16_t");
		set(OPTYP_CHAR32, "char32_t");
		set(OPTYP_CHAR16, "wchar_t");

		set(OPTYP_PTR, "~PVOID_");
		//OPTYP_PTR16
		//OPTYP_PTR1616
		//set(OPTYP_PTR, "PTR32");
		//OPTYP_PTR32_FAR
	}
	const MyString& toString(OpType_t t) const
	{
		size_t i((size_t)t);
		assert(i < size());
#if(0)
		assert(!at(i).empty());
#else
		if (at(i).empty())
		{
			static const MyString s("?");
			return s;
		}
#endif
		return at(i);
	}
	bool fromCString(const char *str, OpType_t &optyp)
	{
		optyp = OPTYP_NULL;
		if (strcmp(str, "void") == 0)
			return true;
		std::map<MyString, OpType_t>::iterator i(m_rev.find(str));
		if (i == m_rev.end())
			return false;
		optyp = i->second;
		return true;
	}

private:
	void set(OpType_t t, const char *str)
	{
		size_t i((size_t)t);
		if (size() < i + 1)
			resize(i + 1);
		if (at(i).empty())//in case of aliases
			at(i) = str;
		m_rev.insert(std::make_pair(MyString(str), (OpType_t)i));
	}
};

static OpType2Str_t g_OpType2Str;

#if(0)
static char *GetDefTypeStr(uint8_t typ, int b)
{
	uint8_t t = typ&0xF0;
	uint8_t s = typ&0x0F;

	const TypStr_t *pTyp = TypStrZ->get(t);

	static char str[10];
	strcpy(str, (b)?(pTyp->str2):(pTyp->str));

	if (s)
	{
		char *buf(str+strlen(str));
//		if (!t)
//			strcat(str, SizStr->get(s)->str);
//		else
			sprintf(buf, "%d", (int)((typ&0x0F)<<3));
	}
	else if (pTyp->typ != OPTYP_NULL)
	{
		strcpy(str+strlen(str), unk());
	}

	return str;
}
#endif

bool Str2OpTyp(const char *str, OpType_t &out)
{
	OpType_t opt;
	if (!g_OpType2Str.fromCString(str, opt))
		return false;
	out = opt;
	return true;
}

const char *OpTyp2Str(uint8_t typ, int nMode, int *pnMode)//0-full,1-short
{
#if(1)
	const char *str(g_OpType2Str.toString((OpType_t)typ));
	if (!nMode)
		if (str[0] == '~')
			str++;

	if (pnMode)//mode used
		*pnMode = islower(str[0])?1:0;
	return str;
#else
	TypStr0_t *p0 = 0;

	char *pstr = 0;
	int n = nMode & 0xF;//type scheme
	bool bShort = (nMode & 0x80000000) != 0;

	switch (n)
	{
	case 1:
		p0 = TypeDefs->get(typ);
		if (!isempty(p0->stdstr))
		{
			pstr = p0->stdstr;
			break;
		}
	case 2:
		n = 2;
		if (!p0)
			p0 = TypeDefs->get(typ);
		if (!isempty(p0->altstr))
		{
			pstr = p0->altstr;
			break;
		}
	case 0:
		n = 0;
		pstr = GetDefTypeStr(typ, bShort);
		break;
	}

	if (pnMode)//mode used
		*pnMode = n;

	assert(pstr);
	return pstr;
#endif
}

void strshift_right(char *str, int pos)//right
{
	assert(pos > 0);
	for (int i = (int)strlen(str); i >= 0; i--)
		str[i+pos] = str[i];
}

void strshift_left(char *str, int pos)
{
	int n = (int)strlen(str)+1-pos;
	assert(pos > 0);
	for (int i = 0; i < n; i++)
		str[i] = str[i+pos];
}

MyString Int2Str(uint64_t i64, uint32_t flags)
{
	char buf[32];
	char *p = buf;

	int radix = (flags&I2S_HEX)?(16):(10);

	bool bSign = false;
	uint64_t absval;
	if (flags & I2S_SIGNED)
	{
		if ((int64_t)i64 < 0)
		{
			bSign = true;
			i64 = -(int64_t)i64;
		}

		if (radix == 16)
		{
			if (!sprintf(p, "%X", (int32_t)i64))
				return unk();
		}
		else
		{
			if (!sprintf(p, "%d", (int32_t)i64))
				return unk();
		}
		absval = abs((int32_t)i64);
	}
	else
	{
		if (!i64toStr(i64, p, radix))
		//if (!_ultoa((uint32_t)i64, p, radix))
			return unk();
		absval = (uint64_t)i64;
	}

	//_strupr(buf);
	{
		char *p(buf);
		for (; *p; ++p) *p = toupper(*p);
	}
	
	if ((flags & I2S_HEX) == I2S_HEXC)
	{
		if (absval > 9)
		{
			strshift_right(buf, 2);
			buf[0] = '0';
			buf[1] = 'x';
		}
	}
	else if ((flags & I2S_HEX) == I2S_HEXA)
	//else if (flags & I2S_HEX)
	{
		if (!isdigit(buf[0]))
		{
			strshift_right(buf, 1);
			buf[0] = '0';
		}
		if (absval > 9)
			//if ((flags & I2S_HEX) == I2S_HEX)
				strcat(buf, "h");
	}

	if (bSign)
	{
		strshift_right(buf, 1);
		buf[0] = '-';
	}

	return buf;
}

int STR_IMMIDIATE(int64_t i64, char *buf)
{
	assert(0);
	return 0;//? (_i64toa(i64, buf, 10) != 0);
}

int AgreeTypes(uint8_t& typ1, uint8_t& typ2)
{
	uint8_t sz1 = OPSIZE(typ1);
	uint8_t sz2 = OPSIZE(typ2);
	if (sz1 && !sz2)
		typ2 |= sz1;
	else if (sz2 && !sz1)
		typ1 |= sz2;
	else if (sz1 != sz2)
		return 0;

	uint8_t _typ1 = OPTYPE(typ1);
	uint8_t _typ2 = OPTYPE(typ2);
	if (_typ1 && !_typ2)
		typ2 |= _typ1;
	else if (_typ2 && !_typ1)
		typ1 |= _typ2;
	else
	{
		if (_typ1 == _typ2)
			return 1;

		if (OPTYP_IS_PTR(_typ1))
		{
			if (!OPTYP_IS_PTR(_typ2))
			{
				//if (!OPTYP_IS_INT(_typ2)))
					return 0;
			}
			if (OPTYP_IS_REAL(_typ2))
				return 0;
			/*if (!OPTYP_IS_PTR(_typ2))
			{
				typ2 &= ~OPTYP_MASK;
				typ2 |= _typ1;
			}*/
			return 1;
		}
		else if (OPTYP_IS_PTR(_typ2))
		{
			if (!OPTYP_IS_PTR(_typ1))
			{
				//if (!OPTYP_IS_INT(_typ1)))
					return 0;
			}
			if (OPTYP_IS_REAL(_typ1))
				return 0;
			/*if (!OPTYP_IS_PTR(_typ1))
			{
				typ1 &= ~OPTYP_MASK;
				typ1 |= _typ2;
			}*/
			return 1;
		}

		if (!OPTYP_IS_INT(_typ1) || !OPTYP_IS_INT(_typ2))
			return 0;

		//both are integers!

#define IS_SURE_INT(a)	(a != OPTYP_INTEGER)//confirmed integer
//#define _80(a)	(a&0x80)
//#define _30(a)	(a&0x30)

		if (IS_SURE_INT(_typ1) && !IS_SURE_INT(_typ2))
			typ2 = typ1;
		else if (IS_SURE_INT(_typ2) && !IS_SURE_INT(_typ1))
			typ1 = typ2;
		else if (IS_SURE_INT(_typ1) && IS_SURE_INT(_typ2))
		{
/*			if (_80(_typ1))
			{
				if (!_80(_typ2))
					return 0;

				if (!_30(_typ1))
				{
					if (_30(_typ2))
						typ1 |= _30(typ2);
				}
				else
				{
					if (!_30(_typ2))
						typ2 |= _30(typ1);
				}
			}
			else //if (_30(_typ1))
			{
				if (_30(_typ2) != _30(_typ1))*/
			//abort();
					return 0;
			//}
		}
	}
	return 1;
}

const char *MakeName(
	const char *p1,//mid priority
	const char *p2,//lo priority
	const char *p3)//hi priority
{
	const char Ch = '_';//parts divider
	int len1 = (p1)?((int)strlen(p1)+1):(0);
	int len2 = (p2)?((int)strlen(p2)+1):(0);
	int len3 = (p3)?((int)strlen(p3)+1):(0);

	if (len3 > NAMELENMAX)
	{
		len3 = NAMELENMAX;
		len1 = len2 = 0;
	}
	else 
	{	
		int l = NAMELENMAX-len3;
		if (len1 > l)
		{
			len1 = l;
			len2 = 0;
		}
		else
		{
			int l = NAMELENMAX-(len3+len1);
			if (len2 > l)
			{
				len2 = l;
			}
		}
	}

	assert(len1+len2+len3 <= NAMELENMAX);

	static char buf[NAMELENMAX+1];
	if (len1 > 0)
		strncpy(buf, p1, len1);
	buf[len1-1] = Ch;
	if (len2 > 0)
		strncpy(buf+len1, p2, len2);
	buf[len1+len2-1] = Ch;
	if (len3 > 0)
		strncpy(buf+len1+len2, p3, len3);
	buf[len1+len2+len3-1] = 0;

	return buf;
}

//вычисление наибольшего общего делителя (НОД)
int NOD(int N, int M)
{
	do {
		int L = M;
		M = N-M*(N/M);
		N = L;
	} while (M != 0);

	return N;
}

bool StrHex2Int(const char *s, value_t &v)
{
	char buf[(sizeof(v.i64) << 1) + 2];//enough for 64-bit value

	assert(s);
	
	int sign = 1;
	//trim left & check sign
	while (*s)
	{
		if (!isspace(*s))
		{	
			if (*s == '-')
				sign *= -1;
			else if (*s != '+')
				break;
		}
		s++;
	}

	//check C-like representation
	if (strlen(s) > 2)
	{
		if (s[0] == '0' && s[1] == 'x')
			s += 2;
	}

	//cut off left zeroes
	while (*s && *s == '0')
		s++;

	v.clear();
	if (!*s)
		return true;

	//copy to  buffer
	int i;
	for (i = 0; i < sizeof(buf); i++)
	{
		buf[i] = *s;
		if (*s == 0)
			break;
		if (isspace(*s))
			return false;//some extra staff?!
		s++;
	}

	if (buf[0] == 0)//not empty
		return false;

	for (i = 0; buf[i]; i++)
	{
		int w;
		char c = buf[i];
		if (c >= '0' && c <= '9')
			w = c-'0';
		else if (c >= 'a' && c <= 'f')
			w = 10+(c-'a');
		else if (c >= 'A' && c <= 'F')
			w = 10+(c-'A');
		else if (c == 'h')
			break;
		else
			return false;
		v.i64 <<= 4;
		v.i64 += w;
	}

	v.i64 *= sign;
	return true;
}

int i64toStr(uint64_t i64, char *buf, int radix)
{
	assert(radix == 10 || radix == 16);

	int i = 0;
	if (i64)
	{
		while (i64)
		{
			int64_t rem = i64 % radix;
			i64 /= radix;
			char c = (char)rem;
			if (c < 10)
				c += '0';
			else
			{
				c -= 10;
				c += 'a';
			}
			buf[i++] = c;
		}
	}
	else
		buf[i++] = '0';

	buf[i] = 0;
	strrev(buf);
	return 1;
}

bool Str2Int(const char *ps, value_t &v)
{
	MyString s(ps);
	if (s.startsWith("0x", false) || s.endsWith("h", false))
		return StrHex2Int(ps, v);
	char *end;
	v.i32 = strtol(ps, &end, 10);
	if (*end != 0)
		return false;
	return true;
}

const char *value_t::str(int typ, int flags)
{
	static char buf[20];
	char *p = buf;

	int radix = (flags&I2S_HEX)?(16):(10);

	bool bSign = false;
	uint64_t absval;
	if ((typ & OPSZ_MASK) == OPSZ_QWORD)
	{	
		if (flags & I2S_SIGNED)
		{
			if (i64 < 0)
			{
				i64 = -i64;
				bSign = true;
			}

			if (!i64toStr(i64, p, radix))
				return unk();
			
			absval = i64;
		}
		else
		{
			if (!i64toStr(i64, p, radix))
				return unk();
			absval = (uint64_t)i64;
		}
	}
	else
	{
		if (flags & I2S_SIGNED)
		{
			if (i32 < 0)
			{
				i32 = -i32;
				bSign = true;
			}

			if (radix == 16)
			{
				if (!sprintf(p, "%X", i32))
					return unk();
			}
			else
			{
				if (!sprintf(p, "%d", i32))
					return unk();
			}
			absval = abs(i32);
		}
		else
		{
			if (radix == 16)
			{
				if (!sprintf(p, "%X", (uint32_t)i64))
					return unk();
			}
			else
			{
				if (!sprintf(p, "%u", (uint32_t)i64))
					return unk();
			}
			absval = ui32;
		}
	}

	//_strupr(buf);
	{
		char *p(buf);
		for (; *p; ++p) *p = toupper(*p);
	}
	
	if ((flags & I2S_HEX) == I2S_HEXC)
	{
		if (absval > 9)
		{
			strshift_right(buf, 2);
			buf[0] = '0';
			buf[1] = 'x';
		}
	}
	else if ((flags & I2S_HEX) == I2S_HEXA)
	//else if (flags & I2S_HEX)
	{
		if (!isdigit(buf[0]))
		{
			strshift_right(buf, 1);
			buf[0] = '0';
		}
		if (absval > 9)
			//if ((flags & I2S_HEX) == I2S_HEX)
				strcat(buf, "h");
	}

	if (bSign)
	{
		strshift_right(buf, 1);
		buf[0] = '-';
	}

	return buf;
}

static bool checkexp(char *buf)
{
	char *p = buf;
	while (*p)
	{
		if (*p == 'e')
		{
			p++;
			if (*p == '+')
				strshift_left(p, 1);
			else 
			{
				assert(*p == '-');
				p++;
			}
			while (*p == '0')//cut left zeroes off
				strshift_left(p, 1);
			return true;
		}
		p++;
	}

	return false;
}

static void cutzeroes(char *buf)
{
	char *p = buf;
	while (*p)
		p++;//find end of buf
	while (1)
	{
		p--;
		if (*p != '0')
			break;
		*p = 0;
	}
	if (*p == '.')
	{
		p++;
		*p = '0';
	}
}

static bool checkpoint(const char *buf)
{
	const char *p = buf;
	while (*p)
	{
		if (*p == '.')
			return true;
		p++;
	}
	return false;
}

const char * VALUE_t::toString()
{
	static char buf[30];

	if (OPSIZE(typ) > OPSZ_QWORD)
	{
		return "?";
	}

	if (typ == OPTYP_REAL32)
	{
		//os << r32;// << "f";
		sprintf(buf, "%g", r32);
		if (!checkpoint(buf))
			strcat(buf, ".0");
		strcat(buf, "f");
		return buf;
	}
	
	if (typ == OPTYP_REAL64)
	{
#if(0)
		sprintf(buf, "%.6f", r64);
		cutzeroes(buf);
		return buf;
#else
		sprintf(buf, "%g", r64);
		if (checkexp(buf))
			return buf;

		if (!checkpoint(buf))
			strcat(buf, ".0");
		return buf;
#endif
	}

	if (typ == OPTYP_CHAR8)
	{
		if (isprint(i8))
		{
			sprintf(buf, "'%c'", i8);
			return buf;
		}
	}
	{
		uint32_t flags = 0;
		if (!OPTYP_IS_UINT(typ))
			flags |= I2S_SIGNED;
		flags |= I2S_HEXC;

		value_t v;
		//v.clear();
		memcpy(&v.i32, &i32, typ & OPSZ_MASK);
		strcpy(buf, v.str(typ, flags));
//		if (opsz == OPSZ_QWORD)
//			strcat(buf, Int2Str( i64, flags ));
//		else
//			strcat(buf, Int2Str( i32, flags ));
		return buf;
	}

	switch (opsiz)
	{
	case 0:
		sprintf(buf, "%d", i32);
		return buf;

	case OPSZ_BYTE:
		if (isprint(i8))
			sprintf(buf, "'%c'", i8);
		else
		{
			int i = i8;
			sprintf(buf, "%d", i);
		}
		return buf;

	case OPSZ_WORD:	
		if (typ == OPTYP_UINT16)
			sprintf(buf, "%d", (unsigned int)ui16);
		else//?if (typ == OPTYP_INT16)
			sprintf(buf, "%d", (int)ui16);
		return buf;

	case OPSZ_DWORD: 
		if (typ == OPTYP_REAL32)
		{
			sprintf(buf, "%f", r32);
			if (!checkpoint(buf))
				strcat(buf, ".0");
			strcat(buf, "f");
		}
		else if (typ == OPTYP_UINT32)
			sprintf(buf, "%ud", ui32);
		else //?if (typ == OPTYP_INT32)
			sprintf(buf, "%d", i32);
		return buf;

	case OPSZ_QWORD:
		if (typ == OPTYP_REAL64)
		{
			sprintf(buf, "%g", r64);
			return buf;
		}
//		else if (typ == OPTYP_INT64)
//			os << i32; 
//		else if (typ == OPTYP_UINT64)
//			os << ui32;
	}

	return unk();
}

void VALUE_t::invert()
{
	if (typ == OPTYP_REAL32)
		r32 = 1/r32;
	else if (typ == OPTYP_REAL64)
		r64 = 1/r64;
	else
		assert(false);
}

void VALUE_t::estimateSize()
{
	if (i64 & 0xFFFFFFFF00000000)
		opsiz = 8;
	else if (i32 & 0xFFFF0000)
		opsiz = 4;
	else if (i16 & 0xFF00)
		opsiz = 2;
	else opsiz = 1;
}

bool checkoverlap(int offs1, int sz1, int offs2, int sz2)
{
	int dOffs = offs2 - offs1;//delta offset
	if (dOffs > 0)
	{
		if (sz1 > dOffs)
			return true;
	}
	else if (dOffs < 0) 
	{
		if (sz2 > -dOffs)
			return true;
	}
	else //if (dOffs == 0)
		return true;

	return false;
}




//////////////////////////

MemTrace_t * MemTrace_t::m_list = nullptr;


void MemTrace_t::print_summary(std::ostream &ofs)
{
	char buf[1024];

	ofs << "MEMORY USAGE INFO\n-----------------\n";

	uint32_t nTotal(0);
	for (MemTrace_t * p(m_list); p; p = p->m_next)
	{
		nTotal += p->size();
		sprintf(buf, "%s: %d (%d)\n", p->name(), p->num(), p->size());
		ofs << buf;
	}
	
	sprintf(buf, "-----------------\nTotal memory allocated: %d bytes\n", nTotal);
	ofs << buf;
}

void MemTrace_t::print_status(std::ostream &ofs, const std::string &header)
{
	char buf[1024];

	int count(0);
	for (MemTrace_t * p(m_list); p; p = p->m_next)
	{
		int num = p->num();
		if (num != 0)
		{
			if (count == 0 && !header.empty())
			{
				ofs << header << std::endl;
				for (size_t i(0); i < header.length(); i++)
					ofs << '-';
				ofs << std::endl;
			}

			sprintf(buf, "\t%d %s(s) not deleted\n", num, p->name());
			ofs << buf;
			if (!p->empty())
			{
				for (MemTrace_t::const_iterator i(p->begin()); i != p->end(); i++)
				{
					if (i != p->begin())
						 ofs << ",";
					ofs << *i;
				}
				ofs << std::endl;
			}
			count++;
		}
	}
}

void MemTrace_t::clear()
{
	for (MemTrace_t * p(m_list); p; p = p->m_next)
	{
		p->m_refs = 0;
		p->std::set<int>::clear();
	}
}


int tab2spaces(int pos)
{
	int t = 0;
	do {
		t++;
	} while ((pos + t) & TABMASK);
	return t;
}

bool MatchWildcard(const char *first, const char * second)
{
	// If we reach at the end of both strings, we are done
	if (*first == '\0' && *second == '\0')
		return true;

	// Make sure that the characters after '*' are present in second string.
	// This function assumes that the first string will not contain two
	// consecutive '*' 
	if (*first == '*' && *(first + 1) != '\0' && *second == '\0')
		return false;

	// If the first string contains '?', or current characters of both 
	// strings match
	if (*first == '?' || *first == *second)
		return MatchWildcard(first + 1, second + 1);

	// If there is *, then there are two possibilities
	// a) We consider current character of second string
	// b) We ignore current character of second string.
	if (*first == '*')
		return MatchWildcard(first + 1, second) || MatchWildcard(first, second + 1);
	return false;
}



/*std::string string_format(const std::string &fmt, ...)
{
	int size = ((int)fmt.size()) * 2 + 50;
    std::string str;
    va_list ap;
    while (1) {
        str.resize(size);
        va_start(ap, fmt);
		int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
        va_end(ap);
		if (n > -1 && n < size)
		{
			str.resize(n);
			return str;
		}
        if (n > -1)
            size = n + 1;
        else
            size *= 2;
    }
}*/

#include <sstream>

//=============================================================================
void DoFormatting(std::string& sF, const char* sformat, va_list marker)
{
	char *s, ch = 0;
	int n, i = 0, m;
	long l;
	double d;
	std::string sf = sformat;
	std::ostringstream ss;

	m = (int)sf.length();
	while (i<m)
	{
		ch = sf.at(i);
		if (ch == '%')
		{
			i++;
			if (i<m)
			{
				ch = sf.at(i);
				switch (ch)
				{
					case 's': { s = va_arg(marker, char*);  ss << s;         } break;
					case 'c': { n = va_arg(marker, int);    ss << (char)n;   } break;
					case 'd': { n = va_arg(marker, int);    ss << (int)n;    } break;
					case 'l': { l = va_arg(marker, long);   ss << (long)l;   } break;
					case 'f': { d = va_arg(marker, double); ss << (float)d;  } break;
					case 'e': { d = va_arg(marker, double); ss << (double)d; } break;
					case 'X':
					case 'x':
					{
								if (++i<m)
								{
									ss << std::hex << std::setiosflags(std::ios_base::showbase);
									if (ch == 'X') ss << std::setiosflags(std::ios_base::uppercase);
									char ch2 = sf.at(i);
									if (ch2 == 'c') { n = va_arg(marker, int);  ss << std::hex << (char)n; }
									else if (ch2 == 'd') { n = va_arg(marker, int); ss << std::hex << (int)n; }
									else if (ch2 == 'l') { l = va_arg(marker, long);    ss << std::hex << (long)l; }
									else ss << '%' << ch << ch2;
									ss << std::resetiosflags(std::ios_base::showbase | std::ios_base::uppercase) << std::dec;
								}
					} break;
					case '%': { ss << '%'; } break;
					default:
					{
							   ss << "%" << ch;
							   //i = m; //get out of loop
					}
				}
			}
		}
		else ss << ch;
		i++;
	}
	va_end(marker);
	sF = ss.str();
}

//=============================================================================
std::string& stringf(std::string& stgt, const char *sformat, ...)
{
	va_list marker;
	va_start(marker, sformat);
	DoFormatting(stgt, sformat, marker);
	return stgt;
}

//=============================================================================
std::string& stringfappend(std::string& stgt, const char *sformat, ...)
{
	std::string sF = "";
	va_list marker;
	va_start(marker, sformat);
	DoFormatting(sF, sformat, marker);
	stgt += sF;
	return stgt;
}



//Round up to next higher power of 2 (return x if it's already a power of 2).
unsigned long upper_power_of_two(unsigned long v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;

}

unsigned char LowestBitPosition(unsigned v)
{
	static const unsigned char MultiplyDeBruijnBitPosition2[32] = {
		  0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
			31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
	};
	return MultiplyDeBruijnBitPosition2[(unsigned)(v * 0x077CB531U) >> 27];
}

void PrintHex(unsigned n, unsigned w, char buf[32])
{
	char fmt[6];
	if (w > 0)
		sprintf(fmt, "%%0%dX", w);
	else
		strcpy(fmt, "%08X");
	//?assert(w <= 8);
	sprintf(buf, fmt, n);
}

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to)
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

#include <sys/stat.h>

bool fileExists(const std::string& filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1)
    {
        return true;
    }
    return false;
}


