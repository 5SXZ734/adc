#pragma once

#include "shared/defs.h"
#include "shared/misc.h"
#include "shared/data_source.h"

#include "interface/IADCFront.h"

const OpType_t TYPEID_VOID = OPTYP_NULL;

const OpType_t TYPEID_BYTE = OPSZ_BYTE;
const OpType_t TYPEID_WORD = OPSZ_WORD;
const OpType_t TYPEID_DWORD = OPSZ_DWORD;
const OpType_t TYPEID_QWORD = OPSZ_QWORD;

const OpType_t TYPEID_FLOAT = OPTYP_REAL32;
const OpType_t TYPEID_DOUBLE = OPTYP_REAL64;

const OpType_t TYPEID_BOOL = OPTYP_BOOL;

const OpType_t TYPEID_CHAR = OPTYP_CHAR8;
const OpType_t TYPEID_CHAR8 = OPTYP_CHAR8;
const OpType_t TYPEID_INT8 = OPTYP_INT8;
const OpType_t TYPEID_UINT8 = OPTYP_UINT8;

const OpType_t TYPEID_CHAR16 = OPTYP_CHAR16;
const OpType_t TYPEID_SHORT = OPTYP_INT16;
const OpType_t TYPEID_USHORT = OPTYP_UINT16;
const OpType_t TYPEID_INT16 = OPTYP_INT16;
const OpType_t TYPEID_UINT16 = OPTYP_UINT16;

const OpType_t TYPEID_CHAR32 = OPTYP_CHAR32;
const OpType_t TYPEID_INT = OPTYP_INT32;
const OpType_t TYPEID_UINT = OPTYP_UINT32;
const OpType_t TYPEID_LONG = OPTYP_INT32;
const OpType_t TYPEID_ULONG = OPTYP_UINT32;
const OpType_t TYPEID_INT32	= OPTYP_INT32;
const OpType_t TYPEID_UINT32 = OPTYP_UINT32;

const OpType_t TYPEID_LONGLONG = OPTYP_INT64;
const OpType_t TYPEID_ULONGLONG = OPTYP_UINT64;
const OpType_t TYPEID_INT64	= OPTYP_INT64;
const OpType_t TYPEID_UINT64 = OPTYP_UINT64;


////////////////meta data support

class AuxDataBase_t//for read-only
{
protected:
	char* buf;
	bool	autodelete;
	struct meta_item_t
	{
		unsigned size;
		unsigned offs;
	};
	struct meta_header_t
	{
		unsigned n;
		unsigned size;//whole size
		meta_item_t items[1];//and more...
		static size_t calc_size_for(unsigned z) { return offsetof(meta_header_t, items) + sizeof(meta_item_t) * z; }
	};
public:
	AuxDataBase_t()
		: buf(nullptr), autodelete(true){
	}
	AuxDataBase_t(const I_AuxData *p)
		: buf(p ? (char *)p->data() : nullptr), autodelete(false){
	}
	~AuxDataBase_t(){
		if (autodelete)
			delete [] buf;//if wasn't taken
	}
	template <typename T>
	T *data(const char* name) const {//find by name
		meta_header_t *h(header());
		if (h)
		{
			for (unsigned id(0); id < h->n; ++id)
			{
				const meta_item_t &a(h->items[id]);
				if (a.size > 0)
				{
					const char* pbuf(buf + a.offs);
					unsigned nameLen(pbuf[0]);
					if (nameLen > 0)//non-empty name
						if (strcmp(++pbuf, name) == 0)
							return (T*)(pbuf + nameLen + 1);
				}
			}
		}
		return nullptr;
	}
	PDATA take(){
		PDATA p((PDATA)buf);
		//delete [] buf;
		buf = nullptr;
		return p;
	}
	unsigned size() const {
		meta_header_t *h(header());
		if (!h)
			return 0;
		return h->size;
	}
	PDATA data() const {
		return (PDATA)buf;
	}
protected:
	meta_header_t *header() const
	{
		return (meta_header_t *)buf;
	}
};

template <unsigned N>//if empty - will be initiated with U type's size (aka a header)
class AuxData_t : public AuxDataBase_t
{
public:
	AuxData_t(){
	}
	AuxData_t(const I_AuxData *p)
		: AuxDataBase_t(p){
	}
	unsigned reserve(const char* blockName, unsigned blockSize, size_t id, bool bClear = false)
	{
		assert(autodelete);
		if (!buf)
		{
			//allocate header
			unsigned sz(unsigned(meta_header_t::calc_size_for(N)));
			buf = new char[sz];
			memset(buf, 0, sz);
			meta_header_t *h(header());
			h->n = N;
			h->size = sz;
		}
		
		meta_header_t *h(header());
		unsigned oldSize(h->size);

		//update a header (append a data block)
		meta_item_t &a(h->items[id]);
		assert(a.size == 0);

		//the very first item in block - a length-prefixed, zero terminated name string
		unsigned nameLen(blockName ? unsigned(strnlen(blockName, 0xFF)) : 0);//1 byte max
		blockSize += 1;//name length 1 byte
		if (nameLen > 0)
			blockSize += nameLen + 1;//zero terminated
		
		a.size = blockSize;
		a.offs = oldSize;
		h->size = oldSize + blockSize;
		char *newbuf = new char[h->size];
		memcpy(newbuf, buf, oldSize);
		if (bClear)
			memset(newbuf + oldSize, 0, blockSize);
		
		newbuf[a.offs] = nameLen;
		if (nameLen > 0)
		{
			strncpy(&newbuf[a.offs + 1], blockName, nameLen);
			newbuf[a.offs + 1 + nameLen] = 0;
		}

		delete[] buf;
		buf = newbuf;
		return oldSize;
	}
	template <typename T>
	T *data(unsigned id) const {
		meta_header_t *h(header());
		if (h)
		{
			const meta_item_t &a(h->items[id]);
			if (a.size > 0)
			{
				char* pbuf(buf + a.offs);
				unsigned nameLen(pbuf[0]);
				++pbuf;
				if (nameLen > 0)
					pbuf += nameLen + 1;//skip zero terminated string
				return (T*)(pbuf);
			}
		}
		return nullptr;
	}
	template <typename T>
	const char* name(unsigned id) const {
		meta_header_t *h(header());
		if (h)
		{
			const meta_item_t &a(h->items[id]);
			if (a.size > 0)
			{
				const char* pbuf(buf + a.offs);
				if (pbuf[0] > 0)//non-empty name
					++pbuf;
				return pbuf;
			}
		}
		return nullptr;
	}
};

template <unsigned N>
class __SafeAuxData : public AuxData_t<N>
{
	typedef AuxData_t<N> BASE;
	I_Module	&mr;
public:
	__SafeAuxData(I_Module &r)
		: mr(r)
	{
	}
	~__SafeAuxData()
	{
		mr.setAuxData(AuxDataBase_t::data(), BASE::size());//warning: size becomes 0 after take()
	}
};

HTYPE toAsciiPre(I_Module &, bool bSkipEmpty = false);//length-prepended
HTYPE toAscii(I_Module &, bool bSkipEmpty = false);//zero-terminated
HTYPE toAsciiEx(I_Module& mr, bool bSkipEmpty, char term, size_t limit = -1);//term terminated
HTYPE toUnicode(I_Module &);
HTYPE toNUnicode(I_Module &);


//print GUID as string
namespace adcwin { struct GUID; }
std::string guid_to_string(const adcwin::GUID &);

class MyStreamBase;
enum MangleSchemeEnum { MANGLE_UNKNOWN, MANGLE_MSVC, MANGLE_GCC };
I_Front::SymbolKind __demangleName(const char *mangled, MyStreamBase &ss);
MangleSchemeEnum __checkMangleScheme(const char **pmangled);

void DeclareCapstoneTypes(I_ModuleEx &);
void DeclareUDis86Types(I_ModuleEx &);

int CheckPortableExecutable(const I_DataSourceBase &aRaw, unsigned size);





