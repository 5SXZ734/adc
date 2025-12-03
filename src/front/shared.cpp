#include "shared.h"
#include "qx/MyStream.h"
#include "demanglers/demanglers.h"
#include "wintypes.h"

HTYPE toAscii(I_Module &mr, bool bSkipEmpty)//zero terminated
{
	DECLDATAPTR(const char, pc);
	unsigned l(0);
	for (; *pc; ++pc, ++l);
	if (l == 0 && bSkipEmpty)
		return nullptr;
	return mr.arrayOf(mr.type(TYPEID_CHAR), l + 1);
}

HTYPE toAsciiPre(I_Module &mr, bool bSkipEmpty)//length prepended
{
	DECLDATAPTR(const unsigned char, pc);
	unsigned l(*pc);
	if (l == 0 && bSkipEmpty)
		return nullptr;
	return mr.arrayOf(mr.type(TYPEID_CHAR), l + 1);
}

HTYPE toAsciiEx(I_Module &mr, bool bSkipEmpty, char term, size_t limit)//term terminated
{
	DECLDATAPTR(const char, pc);
	size_t l(0);
	for (; *pc != term; ++pc && l < limit, ++l);
	if (l == 0 && bSkipEmpty)
	{
		mr.error("sequence overlimit");
		return nullptr;
	}
	return mr.arrayOf(mr.type(TYPEID_CHAR), unsigned(l + 1));
}

HTYPE toUnicode(I_Module &mr)
{
	DECLDATAPTR(const char16_t, pc);
	//if (!pc)
	//return nullptr;
	unsigned l(0);
	if (*pc != 0xFFFF)//invalid symbol in UNICODE
		for (; *pc; ++pc, ++l);
	return mr.arrayOf(mr.type(OPTYP_CHAR16), l + 1);
}

HTYPE toNUnicode(I_Module &mr)
{
	DECLDATA(unsigned short, a);
	return mr.arrayOf(mr.type(OPTYP_CHAR16), a + (unsigned short)1);
}

#include "wintypes.h"

std::string guid_to_string(const adcwin::GUID &guid)
{
	char buf[128];
	sprintf(buf, "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
	return buf;
}




// symbol mangling /////////////////////////////////////

MangleSchemeEnum __checkMangleScheme(const char **pmangled)
{
	const char *mangled(*pmangled);

	static const char *pfx0("__Z");
	static const std::string pfx1("_GLOBAL__sub_I_");
	static const char *pfx2("__GLOBAL__sub_I_");
	static const char *pfx3("__imp_?");
	static const char *pfx4("?");
	static const char *pfx5("_Z");

	if (strncmp(mangled, pfx4, strlen(pfx4)) == 0)
		return MANGLE_MSVC;
	if (strncmp(mangled, pfx5, strlen(pfx5)) == 0)
		return MANGLE_GCC;
	if (strncmp(mangled, pfx0, strlen(pfx0)) == 0)
	{
		(*pmangled)++;
		return MANGLE_GCC;
	}
	if (strncmp(mangled, pfx1.c_str(), pfx1.length()) == 0)
	{
		(*pmangled) += pfx1.length();
		return MANGLE_GCC;
	}
	if (strncmp(mangled, pfx2, strlen(pfx2)) == 0)
	{
		(*pmangled) += strlen(pfx2);
		return MANGLE_GCC;
	}
	if (strncmp(mangled, pfx3, strlen(pfx3)) == 0)
	{
		(*pmangled) = mangled + strlen(pfx3) - 1;
		return MANGLE_MSVC;
	}
	return MANGLE_UNKNOWN;
}

I_Front::SymbolKind __demangleName_gcc(const char *mangled, MyStreamBase &);

static bool skipPrefix(const char* pre, char** pstr)
{
	size_t l(strlen(pre));
	if (strncmp(pre, *pstr, l) != 0)
		return false;
	*pstr += l;
	return true;
}

#if(0)
#include <fstream>
std::ofstream gfs("__test");
static void __comp(const char *mangled, const char *old)
{
	static int z = 0;
	z++;
CHECK(z == 0x276)
STOP
	char* buf0(llvm_microsoftDemangle(mangled));
	char* buf(buf0);
	if (buf0)
	{
		if (!skipPrefix("public: ", &buf))//no access specifiers
			if (!skipPrefix("protected: ", &buf))
				skipPrefix("private: ", &buf);
	}
	else
		buf = "<failed>";
	if (!old)
		old = "<failed>";
	bool match(true);
	for (const char* p(buf), *q(old); *p || *q; p++, q++)
	{
		while (*p && *p == ' ') p++;
		while (*q && *q == ' ') q++;
		if (*p != *q)
		{
			match = false;
			break;
		}
		if (!*p)
			break;
	}
	//if (strcmp(buf, old) != 0)
	if (!match)
		gfs << "{" << z << "}\n" << buf << "\n" << old << std::endl;
	if (buf0)
		free(buf0);
}
#endif

static char* simplified(char* str)
{
	if (!str[0])
		return str;
	const char* src = str;// str.cbegin();
	//const char* end = str.cend();
	//NakedStringType result = isConst || !str.isDetached() ?	StringType(str.size(), Qt::Uninitialized) : std::move(str);

	char* dst = str;// const_cast<Char*>(result.cbegin());
	char* ptr = dst;
	//bool unmodified = true;
	for (;;)
	{
		while (*src != 0 && isspace(*src))
			++src;
		while (*src != 0 && !isspace(*src))
			*ptr++ = *src++;
		if (*src == 0)
			break;
		//if (*src != ' ')
		//	unmodified = false;
		*ptr++ = ' ';
	}
	if (ptr != dst && ptr[-1] == ' ')
		--ptr;
	//*ptr = '\0';
	//int newlen = ptr - dst;
	//if (isConst && newlen == str.size() && unmodified)
	//	return str;// nothing happened, return the original
	//result.resize(newlen);
	return ptr;
}

I_Front::SymbolKind __demangleName(const char *mangled, MyStreamBase &ss)
{
	MangleSchemeEnum eMangle(__checkMangleScheme(&mangled));

	//struct ManglePfx { const char *pfx; MangleSchemeEnum e; unsigned shift; }
	//mangle = {{}, {}, {} }

	if (eMangle == MANGLE_MSVC)
	{
		MyStreamUtil ssu(ss);
		char *buf0(nullptr);
//CHECK(strcmp(mangled, "??_C@_02GFKOMOKH@?5?$CI?$AA@") == 0)
//STOP
		if (strncmp(mangled, "??_C@", 5) == 0)//strings?
		{
			ssu.WriteString("`string'");
			return I_Front::SYMK_STRING;
		}

		if (strncmp(mangled, "??_7", 4) == 0)//vtable?
		{
			STOP
		}
#if(1)
		buf0 = llvm_demangle_msvc(mangled);//try llvm first
#endif
		if (!buf0)
		{
			buf0 = undname(mangled);
			if (!buf0)
				return I_Front::SYMK_NULL;
		}

		I_Front::SymbolKind eKind(I_Front::SYMK_NULL);

		char* buf(buf0);
		//[thunk]: public: 
		bool bThunk(skipPrefix("[thunk]: ", &buf));
		if (bThunk)
			eKind = I_Front::SYMK_THUNK;

		if (!skipPrefix("public: ", &buf))//no access specifiers
			if (!skipPrefix("protected: ", &buf))
				skipPrefix("private: ", &buf);

		static const char *pfx1("class");
		static const std::string sfx1("`RTTI Type Descriptor\'");
		static const std::string sfx2("`RTTI Base Class Descriptor at");
		static const std::string sfx3("`RTTI Class Hierarchy Descriptor\'");
		static const std::string sfx4("`RTTI Base Class Array\'");
		static const std::string sfx5("`RTTI Complete Object Locator'");
		static const std::string sfx6("`vftable\'");
		static const std::string sfx7("`vftable\'{for `");
		static const std::string sfx8("`vbtable\'");
		static const std::string sfx9("`local vftable\'");
		static const std::string sfx10("`vector deleting dtor\'");

		if (strstr(buf, "local vftable"))
		{
			STOP
		}

		char *buf2 = strchr(buf, '`');
		if (buf2)
		{
			if (strcmp(buf2, sfx6.c_str()) == 0)
			{
				eKind = I_Front::SYMK_VFTABLE;
				ssu.WriteString(buf);
			}
			else if (strcmp(buf2, sfx9.c_str()) == 0)
			{
				eKind = I_Front::SYMK_LVFTABLE;
				ssu.WriteString(buf);
			}
			else if (strcmp(buf2, sfx8.c_str()) == 0)
			{
				eKind = I_Front::SYMK_VBTABLE;
				ssu.WriteString(buf);
			}
			else if (strncmp(buf2, sfx7.c_str(), sfx7.length()) == 0)
			{
				eKind = I_Front::SYMK_VFTABLE;
#if(0)
				for (char* p(buf2 + 1); *p; ++p)//discard bad symbols
					if (!isalnum(*p))
						*p = ' ';
				char *end(simplified(buf2 + 1));
				*end++ = '\'';
				*end = '\0';
#endif
				ssu.WriteString(buf);
			}
			else if (strcmp(buf2, sfx1.c_str()) == 0)
			{
				eKind = I_Front::SYMK_RTTI_TD;
				// make it a static class member:
				//    "class <type> `RTTI Type Descriptor'" => "const <type>::`RTTI Type Descriptor'"
				MyString s(buf);
				if (s.startsWith(pfx1))
					s.replace(0, (unsigned)strlen(pfx1), "const");
				s.chop((unsigned)sfx1.length() + 1);//w space
				s.append("::");
				s.append(sfx1);
				ssu.WriteString(s);
			}
			else if (strncmp(buf2, sfx2.c_str(), sfx2.length()) == 0)//"RTTI Base Class Descriptor at"
			{
				eKind = I_Front::SYMK_RTTI_BCD;
				// chop off extra stuff:
				//    "<type>::`RTTI Base Class Descriptor at (0, -1, 0, 64)'" => "<type>::`RTTI Base Class Descriptor'"
				MyString s(buf, buf2 - buf);
				s.append("`RTTI Base Class Descriptor'");
				ssu.WriteString(s);
			}
			else if (strcmp(buf2, sfx3.c_str()) == 0)//Class Hierarchy Descriptor
			{
				eKind = I_Front::SYMK_RTTI_CHD;
				ssu.WriteString(buf);
			}
			else if (strcmp(buf2, sfx4.c_str()) == 0)//Base Class Array
			{
				eKind = I_Front::SYMK_RTTI_BCA;
				ssu.WriteString(buf);
			}
			else if (strcmp(buf2, sfx5.c_str()) == 0)//Complete Object Locator
			{
				eKind = I_Front::SYMK_RTTI_COL;
				ssu.WriteString(buf);
			}
			else if (strncmp(buf2, sfx10.c_str(), sfx10.length()) == 0)//vector deleting dtor
			{
				MyString s(buf, buf2 - buf);
				s.append("`vector deleting destructor'");//replace
				s.append(buf2 + sfx10.length());
				ssu.WriteString(s);
			}
			else
			{
				ssu.WriteString(buf);
			}
		}
		else if (bThunk)
		{
			MyString s("`thunk to ");
			s.append(buf);
			s.append("'");
			ssu.WriteString(s);
		}
		else
		{
			ssu.WriteString(buf);
		}

		free_demangled(buf0);
		return eKind;
	}
	else if (eMangle == MANGLE_GCC)
	{
		return __demangleName_gcc(mangled, ss);
	}
	return I_Front::SYMK_NULL;
}

int CheckPortableExecutable(const I_DataSourceBase &aRaw, unsigned size)
{
	using namespace adcwin;
	DataFetch2_t<IMAGE_DOS_HEADER> idh(aRaw, 0);
	if (idh.e_magic == IMAGE_DOS_SIGNATURE
#ifdef IMAGE_SAFE_DOS_SIGNATURE
		|| idh.e_magic == IMAGE_SAFE_DOS_SIGNATURE
#endif
		)
	{
		if (idh.e_lfanew)
		{
			DataFetch_t<DWORD> eh(aRaw, idh.e_lfanew);
			if (eh == IMAGE_NT_SIGNATURE)
			{
				DataFetch_t<WORD> magic(aRaw, idh.e_lfanew + 4 + sizeof(IMAGE_FILE_HEADER));
				if (magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
					return 1;//PE32
				if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
					return 2;//PE64
			}
		}
		else
			return -1;//MSDOS EXE?
	}
	return 0;
}










