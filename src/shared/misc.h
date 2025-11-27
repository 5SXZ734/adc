
#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <assert.h>
//#include "interface/IADCGui.h"
#include "qx/MyString.h"
#include "defs.h"
#if defined(_WIN32)
#   define strcasecmp _stricmp
#endif


struct value_t
{
	union {
		int8_t	i8;
		uint8_t	ui8;

		int16_t	i16;
		uint16_t	ui16;

		int32_t	i32;
		uint32_t	ui32;
		
		int64_t	i64;
		
		union {
			uint64_t	ui64;
			struct 
			{
				uint32_t	ui64l;
				uint32_t	ui64h;
			};
		};
	
		float	r32;
		double	r64;
		////////////
		uint8_t	buf[sizeof(ui64)];
	};

	value_t(){ clear(); }
	value_t(uint64_t ui){ ui64 = ui; }
	value_t(void *pv, uint8_t sz){ assign(pv, sz, false); }
	value_t(const value_t &o){ ui64 = o.ui64; }
	void assign(void *pv, uint8_t sz, bool bSigned)
	{
		if (bSigned)
		{
			if (!pv)
				i64 = 0;
			else if (sz == OPSZ_BYTE)
				i64 = *(int8_t *)pv;
			else if (sz == OPSZ_WORD)
				i64 = *(int16_t *)pv;
			else if (sz == OPSZ_DWORD)
				i64 = *(int32_t *)pv;
			else if (sz == OPSZ_QWORD)
				i64 = *(int64_t *)pv;
			else
				ASSERT0;
		}
		else
		{
			ui64 = 0;
			assert(sz <= sizeof(ui64));
			/*if (sz > sizeof(ui64))
			{
				abort();
			}*/
			if (pv)
				memcpy(&ui64, pv, sz);
		}
	}

	const char	*str(int typ, int flags);
	void	clear(){ r64 = 0; }
};

struct VALUE_t : public value_t 
{
	union {
		uint8_t	typ;
		struct {
			uint8_t	opsiz:4;
			uint8_t	optyp:4;
		};
	};

	VALUE_t(){
		typ = OPTYP_NULL; }
	VALUE_t(int32_t l){
		typ = OPTYP_NULL;
		i64 = l; }
	VALUE_t(uint8_t t, value_t v){
		typ = t;
		ui64 = v.ui64; }

	const char * toString();
	void	invert();
	void	clear(){ value_t::clear(); typ = OPTYP_NULL; }
	void	estimateSize();

	VALUE_t& operator = ( const VALUE_t& v ) 
		{ typ = v.typ; i64 = v.i64; return *this; }
	VALUE_t& operator = ( int32_t l )
		{ i64 = l; return *this; }

	friend VALUE_t operator * ( const VALUE_t& u, const VALUE_t& v )
		{ return VALUE_t ( u.i32 * v.i32 ); }
	friend VALUE_t operator / ( const VALUE_t& u, const VALUE_t& v )
		{ return VALUE_t ( u.i32 / v.i32 ); }
	friend VALUE_t operator % ( const VALUE_t& u, const VALUE_t& v )
		{ return VALUE_t ( u.i32 % v.i32 ); }
	friend VALUE_t operator + ( const VALUE_t& u, const VALUE_t& v )
		{ return VALUE_t ( u.i32 + v.i32 ); }
	friend VALUE_t operator - ( const VALUE_t& u, const VALUE_t& v )
		{ return VALUE_t ( u.i32 - v.i32 ); }
	friend VALUE_t operator << ( const VALUE_t& u, const VALUE_t& v )
		{ return VALUE_t ( u.i32 << v.i32 ); }
	friend VALUE_t operator >> ( const VALUE_t& u, const VALUE_t& v )
		{ return VALUE_t ( u.i32 >> v.i32 ); }
	friend VALUE_t operator & ( const VALUE_t& u, const VALUE_t& v )
		{ return VALUE_t ( u.i32 & v.i32 ); }
	friend VALUE_t operator | ( const VALUE_t& u, const VALUE_t& v )
		{ return VALUE_t ( u.i32 | v.i32 ); }
	friend VALUE_t operator ^ ( const VALUE_t& u, const VALUE_t& v )
		{ return VALUE_t ( u.i32 ^ v.i32 ); }

	int operator == (int32_t l) { return i32 == l; }


	bool	isReal() {
		return OPTYP_IS_REAL(typ);
	}
	bool	isUnsignedInt() {
		return OPTYP_IS_UINT(typ);
	}
	bool	isSignedInt() {
		return OPTYP_IS_SINT(typ);
	}
	bool	isInt() {
		return !OPTYP_IS_PTR(typ) && OPTYP_IS_INT(typ);
	}
};

int i64toStr(uint64_t i64, char *buf, int radix);

//////////////////
// m i s c . h

struct IDStr_t 
{
	uint32_t	id;
	union {
		const char	*str;
		int32_t	val;
	};

	IDStr_t * find(uint32_t _id, uint32_t mask = -1)
	{
		IDStr_t * p;
		for (p = this; p->id != 0; p++)
			if ((p->id & mask) == _id)
				break;
		return p;
	}

	const char * GetStr(uint32_t _id, uint32_t mask = -1)
	{
		IDStr_t * p;
		for (p = this; p->id != 0; p++)
			if ((p->id & mask) == _id)
				break;
		return p->str;
	}

	int GetID(const char *_str, uint32_t &_id)
	{
		for (IDStr_t * p = this; p->id != 0; p++)
		{
#ifdef WIN32
			if (stricmp(p->str, _str) == 0)
#else
			if (strcasecmp(p->str, _str) == 0)
#endif
			{
				_id = p->id;
				return 1;
			}
		}

		_id = 0;
		return 0;
	}
};

__inline int SearchTable(char *pStr, char const *pTable[], uint32_t &iInd)
{
	char const **p = &pTable[0];
	while (*p)
	{
		if (!strcmp(*p, pStr))
		{
			iInd = (uint32_t)(p-pTable);
			return 1;
		}
		p++;
	}	

	return 0;
}

struct TypStr_t {
	int		typ;
	const char	*str;
	const char	*str2;

	const TypStr_t *get(int id) const
	{
		for (const TypStr_t *p(this); p->typ != -1; p++)
			if (p->typ == id)
				return p;
		return nullptr;
	}
};

struct TypStr0_t {
	int		tid;
	char	stdstr[32];
	char	altstr[32];

	TypStr0_t *get(int id)
	{
		TypStr0_t *p = this;
		while (p->tid != -1)
		{
			if (p->tid == id)
				return p;
			p++;
		}
		return p;
	}
};

//extern TypStr0_t TypeDefs[];

template <class T> void XChg( T &a, T &b )
	{ T c = a; a = b; b = c; }

//функция подсчёта количества двоичных едениц в переменной
template <typename T>
unsigned CountBits(T value)//T must be unsigned
{
	unsigned u;
	for (u = 0; value; u++)
		value &= (value - 1);
	return u;
}

int NOD(int N, int M);

void strshift_right(char *str, int pos);//right
void strshift_left(char *str, int pos);
__inline bool isempty(const char *s){
	return !s || s[0] == 0; }
bool checkoverlap(int offs1, int sz1, int offs2, int sz2);

void EmergencyClose();
const char *OpTyp2Str(uint8_t typ, int nMode = 0, int *pnMode = nullptr);
bool Str2OpTyp(const char *, OpType_t &);

//flags for Int2Str
#define	I2S_HEXC	0x00000001	//C/C++
#define I2S_HEXA	0x00000002	//Asm
#define I2S_HEX		0x00000003	//just hex
#define	I2S_SIGNED	0x00000010	//signed
MyString Int2Str(uint64_t i64, uint32_t flags = 0);
bool StrHex2Int(const char *, value_t &);
bool Str2Int(const char *, value_t &);//checks hexes as well
int STR_IMMIDIATE(int64_t i64, char *buf);

__inline const char* unk(){
	return "?"; }

const char *MakeName(
	const char *p1,//mid priority
	const char *p2,//lo priority
	const char *p3);//hi priority

#define	MakeLong(a, b)		((uint32_t)(((uint16_t)(a)) | ((uint32_t)((uint16_t)(b))) << 16))


template <typename T>
T swap_endian(T u)
{
	//static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

	union
	{
		T u;
		unsigned char u8[sizeof(T)];
	} source, dest;

	source.u = u;

	for (size_t k = 0; k < sizeof(T); k++)
		dest.u8[k] = source.u8[sizeof(T) - k - 1];

	return dest.u;
}

inline bool swap_endian(void* v, int size)
{
	switch (size)
	{
	case OPSZ_BYTE:
		return true;
	case OPSZ_WORD:
		*(uint16_t*)v = swap_endian(*(uint16_t*)v);
		return true;
	case OPSZ_DWORD:
		*(uint32_t*)v = swap_endian(*(uint32_t*)v);
		return true;
	case OPSZ_QWORD:
		*(uint64_t*)v = swap_endian(*(uint64_t*)v);
		return true;
	default:
		break;
	}
	return false;
}


#define BITMASK(nbits)	~((~0) << (nbits))

struct reg_t
{
	uint8_t	m_ofs;
	union {
		uint8_t	m_sz : 4;
		uint8_t	m_typ;
	};
	
	reg_t() : m_ofs(0), m_typ(0){}
	reg_t(uint8_t ofs, uint8_t sz)
		: m_ofs(ofs), m_typ(sz)
	{
	}
	void clear()
	{ 
		m_typ = 0; 
		m_ofs = 0; 
	}
	reg_t &operator=(const reg_t &r)
	{
		m_typ = r.m_typ; 
		m_ofs = r.m_ofs; 
		return *this;
	}
	char const *STR_FPUREG_ASM() const;
	RegMaskType REG2MASK() const
	{
		assert(m_ofs + m_sz <= sizeof(RegMaskType)*CHAR_BIT);
		RegMaskType m(BITMASK(m_sz));
		m <<= m_ofs;
		return m;
	}
	bool operator==(const reg_t &r) const
	{
		return (m_sz == r.m_sz && m_ofs == r.m_ofs);
	}
	int	size() const { return m_sz; }
//	uint8_t Id(uint8_t c){ return m_id; }
	int offset() const { return m_ofs; }
	void assign(uint8_t ofs, uint8_t sz){
		m_typ = sz; m_ofs = ofs;
	}

	bool empty() const { return !m_typ && !m_ofs; }
	//char cpureg2char();
	//bool char2cpureg(char c);
	uint8_t toId() const
	{
		if (m_sz == 0)
			return 0;
		return (m_ofs / m_sz) + 1;
	}
};

struct REG_t
{
	int m_ofs;
	int m_siz;

public:
	REG_t()
		: m_ofs(0),
		m_siz(0)
	{
	}
	REG_t(int ofs, int siz)
		: m_ofs(ofs),
		m_siz(siz)
	{
	}
	REG_t(const REG_t& o)
		: m_ofs(o.m_ofs),
		m_siz(o.m_siz)
	{
	}

	bool fromMask(uint32_t mask, int ofs)//offset @0
	{
		for (m_ofs = ofs; (mask && !(mask & 1)); m_ofs++)
			mask >>= 1;
		for (m_siz = 0; (mask && (mask & 1)); m_siz++)
			mask >>= 1;
		return mask == 0;
	}

	bool fromMask(RegMaskType mask)
	{
		for (m_ofs = 0; (mask && !(mask & 1)); m_ofs++)
			mask >>= 1;
		for (m_siz = 0; (mask && (mask & 1)); m_siz++)
			mask >>= 1;
		return mask == 0;
	}

	reg_t to_reg() const
	{
		return reg_t(uint8_t(m_ofs), uint8_t(m_siz));
	}
	bool isValid() const { return m_siz > 0; }
	bool isNull() const { return !isValid(); }

	void Setup(int siz, int ofs) { m_siz = siz; m_ofs = ofs; }
	void clear() { m_siz = 0; m_ofs = 0; }

	void MergeWith(const REG_t& r) {
		
		if (!isValid())
		{
			*this = r;
		}
		else
		{
			assert(r.isValid());
			*this = united(r);
/*			int d = r.m_ofs - m_ofs;
			if (d > 0)
				m_siz = std::max(m_siz, d + r.m_siz);
			else if (d < 0)
			{
				m_siz = std::max(r.m_siz, -d + m_siz);
				m_ofs = r.m_ofs;
			}
			else
				m_siz = std::max(m_siz, r.m_siz);*/
		}
	}
	uint32_t Mask() {
		if (m_ofs + m_siz >= sizeof(uint32_t))
			return 0;
		uint32_t m = (~0) << m_siz;
		m = ~m << m_ofs;
		return m;
	}

	uint32_t CheckReg(uint8_t opc, uint8_t& sz, uint8_t& id);
	bool MASK2REG(uint32_t mask, uint8_t opc, uint8_t& typ, uint8_t& id);

	REG_t& operator=(const REG_t &o){
		m_ofs = o.m_ofs;
		m_siz = o.m_siz;
		return *this;
	}
	bool operator==(const REG_t &o) const {
		return m_ofs == o.m_ofs && m_siz == o.m_siz;
	}
	bool operator!=(const REG_t & o) const {
		return !(operator==(o));
	}
	int lower() const { return m_ofs; }
	int upper() const { return m_ofs + m_siz; }

	bool adjacent(const REG_t& o) const
	{
		int l(std::max(lower(), o.lower()));
		int u(std::min(upper(), o.upper()));
		return u == l;
	}

	bool intersects(const REG_t& o) const
	{
		int l(std::max(lower(), o.lower()));
		int u(std::min(upper(), o.upper()));
		return u > l;
	}
	REG_t intersected(const REG_t &o) const {
		int l(std::max(lower(), o.lower()));
		int u(std::min(upper(), o.upper()));
		if (u < l)
			return REG_t();//nul
		return REG_t(l, u - l);
	}
	REG_t united(const REG_t& o) const {
		int l(std::min(lower(), o.lower()));
		int u(std::max(upper(), o.upper()));
		return REG_t(l, u - l);
	}

	REG_t difference(const REG_t& r, REG_t& q) const {//self - r (may split) - non commutative
		if (lower() < r.lower())
		{
			if (r.upper() >= upper())
				q.clear();
			else
				q = REG_t(r.upper(), upper() - r.upper());
			return REG_t(lower(), r.lower() - lower());
		}
		if (r.upper() == upper())
			q.clear();
		return REG_t(r.upper(), upper() - r.upper());
	}
};

struct Reg_t : public reg_t
{
	//?	XOpList_t	m_xdeps;
	//?	void CopyFrom(Reg_t &r);//this = r
	//?	void Clear() { reg_t::Clear(); m_xdeps = 0; }
	friend void Exchange(Reg_t& r1, Reg_t& r2)
	{
		Reg_t t;
		t.clear(); t = r1; r1 = r2; r2 = t;
	}
};

class MemTrace_t : private std::set<int>
{
	static MemTrace_t *	m_list;
	uint32_t				m_refs;
	MemTrace_t*			m_next;
	MemTrace_t* const	m_base;
	const char* const	m_name;
	const int			m_size;
public:
	MemTrace_t(const char * const n, int s, MemTrace_t * base)
		: m_refs(0),
		m_name(n),
		m_size(s),
		m_base(base),
		m_next(nullptr)
	{
		m_next = m_list;
		m_list = this;
	}
	~MemTrace_t()
	{
		MemTrace_t * pr(nullptr);
		MemTrace_t * p(m_list);
		while (p != this)
		{
			pr = p;
			p = p->m_next;
		}
		if (pr)
			pr->m_next = m_next;
	}
	void add(int id)
	{
		if (m_base)
			m_base->release(id);
		m_refs++;
		if (id != 0)
			if (!insert(id).second)
				ASSERT0;
	}
	void release(int id)
	{
		assert(m_refs > 0);
		m_refs--;
		if (id != 0)
		{
			if (find(id) == end())
				ASSERT0;
			erase(id);
		}
		if (m_base)
			m_base->add(id);
	}
	uint32_t num() const { return m_refs; }
	const char * name() const { return m_name; }
	int size() const { return m_size; }
	static void print_summary(std::ostream &);
	static void print_status(std::ostream &, const std::string &);
	static void clear();
};

#define MEMTRACE_DECL(this_class) \
	static MemTrace_t t##this_class; \
	static void MT_add(int); \
	static void MT_release(int);

#if(MEMTRACE_ENABLED)
#define	MEMTRACE_ADD		MT_add(0)
#define	MEMTRACE_RELEASE	MT_release(0)
#define	MEMTRACE_ADD2(id)		MT_add(id)
#define	MEMTRACE_RELEASE2(id)	MT_release(id)
#else
#define	MEMTRACE_ADD
#define	MEMTRACE_RELEASE
#define	MEMTRACE_ADD2(id)
#define	MEMTRACE_RELEASE2(id)
#endif

#define MEMTRACE0_IMPL(this_class) \
	MemTrace_t this_class::t##this_class(#this_class, sizeof(this_class), nullptr); \
	void this_class::MT_add(int id){ t##this_class.add(id); } \
	void this_class::MT_release(int id){ t##this_class.release(id); }

#define MEMTRACE_IMPL(this_class,base_class) \
	MemTrace_t this_class::t##this_class(#this_class, sizeof(this_class), &base_class::t##base_class); \
	void this_class::MT_add(int id){ t##this_class.add(id); } \
	void this_class::MT_release(int id){ t##this_class.release(id); }

int tab2spaces(int pos);

class DataStream_t;

class IOutpADDR2Name
{
public:
	struct out_t
	{
		std::string name;
		int sz;
		bool bThunk;
		reg_t sreg;//if indirect
		out_t() : sz(0), bThunk(false){}
	};
	virtual bool decode(ADDR, bool, out_t &) const = 0;
	//virtual int print(DataStream_t &, ADDR64 base, ADDR va, std::ostream &, bool) const { return 0; }
	virtual void on_RIP_relative_addressing(const VALUE_t &) = 0;
	virtual void print_bytes(const uint8_t *, size_t) = 0;
	virtual bool colorsEnabled() const = 0;
	virtual void print_code(const char *fmt, const uint8_t *data, size_t) = 0;
};

// The main function that checks if two given strings match. The first
// string may contain wildcard characters
bool MatchWildcard(const char *first, const char * second);


template <typename T>
class SlotVector : public std::vector<T *>//the vecor with reusable slots
{
public:
	SlotVector()
	{
		this->push_back(nullptr);//nil element
	}
	~SlotVector()
	{
		for (size_t i(0); i < this->size(); i++)
		{
			delete this->at(i);
			this->at(i) = nullptr;
		}
	}
	void reset()
	{
		for (size_t i(0); i < this->size(); i++)
			this->at(i) = nullptr;
	}
	size_t newSlot(T *pT)
	{
		size_t i;
		for (i = 1; i < this->size(); i++)
			if (!this->at(i))
				break;
		assert(pT);
		if (i < this->size())
		{
			this->at(i) = pT;
			return i;
		}
		this->push_back(pT);
		return this->size() - 1;
	}
	T *deleteSlot(size_t i)
	{
		T *p(this->at(i));
		if (p)
			this->at(i) = nullptr;
		return p;
	}
	T *getSlot(size_t i) const
	{
		if (i < this->size())
			return this->at(i);
		return nullptr;
	}
	int squeezedIndex(T *p0) const
	{
		int j(0);
		for (size_t i(1); i < this->size(); i++)
		{
			T *p(this->at(i));
			if (!p)
				continue;
			if (p == p0)
				return (int)j;
			j++;
		}
		return -j;
	}
};

template <typename T, typename DUMPOS, typename ITER>
class CoolIter : public SlotVector<T>//vector of positiions
{
	struct DataIt
	{
		DUMPOS iRef;//position from which the iterator originated
		DUMPOS iSelf;//position of iterator itself in position vector
		DataIt(DUMPOS _ref, DUMPOS _self) : iRef(_ref), iSelf(_self) {}
	};
public:
	SlotVector<DataIt>	mIts;//vector of current iterators (iterators and positions are different entities)
public:
	CoolIter() {}
	~CoolIter() { this->reset(); }//?
	T *fromIter(ITER it) const {
		return this->at(mIts[it]->iSelf);
	}
	T *fromPos(DUMPOS iPos) const {
		return this->at(iPos);
	}
	DUMPOS newPos(T *pData) {
		return (DUMPOS)this->newSlot(pData); }
	T *deletePos(DUMPOS iPos)
	{
		return this->deleteSlot(iPos);
	}
	DUMPOS posFromIter(ITER it) const
	{
		return DUMPOS(mIts[it]->iSelf);
	}
	ITER newIter(DUMPOS iPos, T *pData)
	{
		DUMPOS iSelf(newPos(pData));
		*this->at(iSelf) = *this->at(iPos);
		return ITER(short(mIts.newSlot(new DataIt(iPos, iSelf))));
	}
	T * deleteIter(ITER it, bool bUpdatePos)
	{
		DataIt &r(*mIts[it]);
		if (bUpdatePos)
			*this->at(r.iRef) = *this->at(r.iSelf);
		T *pData(this->deleteSlot(r.iSelf));
		delete mIts.deleteSlot(it);
		return pData;
	}
};

#ifndef WIN32
__inline char *strrev(char *str)
{
	char *p1, *p2;

	if (!str || !*str)
		return str;
	for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
	{
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}
	return str;
}

#define iscsymf(_c)   (isalpha(_c) || ((_c) == '_'))
#define iscsym(_c)    (isalnum(_c) || ((_c) == '_'))
#endif



template <typename T>
union SmartUnion_t
{
	size_t iNextDead;
	T	data;
	SmartUnion_t() : data(0){}
	SmartUnion_t(T t) : data(t){}
};

template <typename T>
class smart_vector : protected std::vector<SmartUnion_t<T> >
{
	typedef SmartUnion_t<T> Elt_t;
	typedef std::vector<Elt_t>	Base_t;
	size_t	m_deadList;
public:
	smart_vector() : Base_t(1), m_deadList(0){}
	T &operator[](size_t i){ return Base_t::at(i).data; }
	const T &operator[](size_t i) const { return Base_t::at(i).data; }
	void remove(size_t i)
	{
		assert(i > 0 && i < this->size());
		Elt_t &e(Base_t::at(i));
		delete e.data;
		e.iNextDead = m_deadList;
		m_deadList = i;
	}
	size_t add(T t)
	{
		if (m_deadList == 0)
		{
			this->push_back(SmartUnion_t<T>(t));
			return this->size() - 1;
		}
		size_t i(m_deadList);
		Elt_t &e(Base_t::at(i));
		m_deadList = e.iNextDead;
		e.data = t;
		return i;
	}
	bool isValid(size_t i)
	{
		if (!(i > 0 && i < this->size()))
			return false;
		for (int iDead(m_deadList); iDead > 0;)
		{
			if (i == iDead)
				return false;
			Elt_t &e(Base_t::at(iDead));
			iDead = e.iNextDead;
		}
		return true;
	}
	void getDeadList(std::vector<int> &v)
	{
		for (int iDead(m_deadList); iDead > 0;)
		{
			v.push_back(iDead);
			Elt_t &e(Base_t::at(iDead));
			iDead = e.iNextDead;
		}
	}
};

template <typename T>
class SmartVectorMapper_t : protected std::vector<int>
{
	typedef std::vector<int> Base_t;
	smart_vector<T>	&mVector;
public:
	SmartVectorMapper_t(smart_vector<T> &v)
		: mVector(v)
	{
		mVector.getDeadList(*this);
		std::sort(begin(), end());
	}
	T &operator[](int i)
	{
		return mVector[map(i)];
	}
	int size()
	{
		return mVector.size() - Base_t::size();
	}
	int map(int i)
	{
		iterator it(std::lower_bound(begin(), end(), i));
		int nDead(it - begin());//get number of dead elements preceeding i
		return (i - nDead);
	}
};

#define TABMASK	0x3//3

#include <iomanip>

/*inline std::ostream &TAB(std::ostream &os)
{
	os << '\t';
	return os;
}*/

///////////////////////////

struct _Tabs
{
	int num;
	char ch;
};

inline _Tabs TABS(int __n)
{
	_Tabs t;
	t.num = __n;
	t.ch = '\t';
	return t;
}

inline _Tabs SPACES(int __n)
{
	_Tabs t;
	t.num = __n;
	t.ch = ' ';
	return t;
}

template<typename _CharT, typename _Traits>
inline std::basic_ostream<_CharT, _Traits>& operator<<(std::basic_ostream<_CharT, _Traits>& __os, _Tabs __f)
{
	if (__f.num > 0)
		while (__f.num--)
			__os << __f.ch;
	return __os;
}



//display address
struct DA_t
{
	ROWID		row;
	unsigned short	line;
	unsigned short	bit;
	int	adjust;
	DA_t() : row(0), line(0), bit(0), adjust(0){}
	DA_t(ROWID r, unsigned short l, unsigned short b, int adj = 0) : row(r), line(l), bit(b), adjust(adj){}
	bool operator==(const DA_t &o) const { return row + adjust == o.row + o.adjust && line == o.line /*&& bit == o.bit*/; }
	bool operator<(const DA_t& o) const {
		if (row + adjust < o.row + o.adjust)
			return true;
		if (row + adjust > o.row + o.adjust)
			return false;
		if (line < o.line)
			return true;
		//return bit < o.bit;
		return false;
	}
	void clear(){ row = 0; line = 0; bit = 0; adjust = 0; }
	ROWID rowj() const
	{
		if (adjust < 0)
			return row - (-adjust);
		return row + adjust;
	}
};

class UserData_t
{
	void *	mpUserData;
public:
	UserData_t() : mpUserData(nullptr) {}
	~UserData_t() { assert(!mpUserData); }
	UserData_t & operator=(const UserData_t& o){
		mpUserData = o.mpUserData;
		return *this;
	}
	template <typename T> T *UserData() { return reinterpret_cast<T *>(mpUserData); }
	template <typename T> const T *UserData() const { return reinterpret_cast<T *>(mpUserData); }
	template <typename T> T *SetUserData(T *pv) { mpUserData = pv; return UserData<T>(); }
	bool hasUserData() const { return mpUserData != 0; }

	template <typename T> T *PUserData() { return reinterpret_cast<T *>(&mpUserData); }
	template <typename T> const T *PUserData() const { return reinterpret_cast<const T *>(&mpUserData); }
	template <typename T> T *PSetUserData(T pv) { mpUserData = *(void *)&pv; return PUserData<T>(); }
};

#define __LOG2A(s) ((s &0xffffffff00000000ULL) ? (32 +__LOG2B(s >>32)): (__LOG2B(s)))
#define __LOG2B(s) ((s &0xffff0000)         ? (16 +__LOG2C(s >>16)): (__LOG2C(s)))
#define __LOG2C(s) ((s &0xff00)             ? (8  +__LOG2D(s >>8)) : (__LOG2D(s)))
#define __LOG2D(s) ((s &0xf0)               ? (4  +__LOG2E(s >>4)) : (__LOG2E(s)))
#define __LOG2E(s) ((s &0xc)                ? (2  +__LOG2F(s >>2)) : (__LOG2F(s)))
#define __LOG2F(s) ((s &0x2)                ? (1)                  : (0))

#define LOG2_UINT64 __LOG2A
#define LOG2_UINT32 __LOG2B
#define LOG2_UINT16 __LOG2C
#define LOG2_UINT8  __LOG2D

std::string string_format(const std::string &fmt, ...);//DOESN'T WORK SOMETIMES!
std::string& stringf(std::string& stgt, const char *sformat, ...);
std::string& stringfappend(std::string& stgt, const char *sformat, ...);

template <typename T>
bool IsPowerOfTwo(T x) { return !(x == 0) && !(x & (x - 1)); }
unsigned char LowestBitPosition(unsigned);

class RefCounter_t
{
	int mRefs;
public:
	RefCounter_t() : mRefs(0){}
	~RefCounter_t()
	{
		assert(mRefs == 0);
	}
	int addRef()
	{
		mRefs++;
		return mRefs;
	}
	int releaseRef()
	{
		assert(mRefs > 0);
		mRefs--;
		return mRefs;
	}
	int refsNum() const
	{
		return mRefs;
	}
};


template <typename T>
class SetVal_t
{
	T	o;
	T*	po;
public:
	SetVal_t() : o(T()), po(0){}
	void operator=(const T &_o){
		o = _o;
		po = &o;
	}
	bool is_set() const { return po != 0; }
	const T &get() const { return o; }
};

namespace my {

	////////////////////////////////////////////////////////tree_t
	template <typename T>
	struct tree_node : public T
	{
		tree_node *next;
		tree_node *children;
		tree_node() : next(0), children(0){}
	};

	template <typename T>
	struct tree_t
	{
	public:
		typedef tree_node<T> tree_node_t;

		tree_node_t* mp_root;
	public:
		tree_t()
			: mp_root(new tree_node_t)
		{
		}
		~tree_t()
		{
			destroy(root());
		}
		void reset(tree_node_t* p)
		{
			destroy(root());
			mp_root = p;
			if (!mp_root)
				mp_root = new tree_node_t;
		}
		bool kill_root()
		{
			if (!mp_root)
				return false;
			assert(!mp_root->next);//never
			//check if there is the only sub-root item
			if (!mp_root->children || mp_root->children->next)
				return false;
			tree_node_t *p(mp_root->children);
			mp_root->children = nullptr;//avoid a recursive destroy
			reset(p);
			return true;
		}
		void destroy(tree_node_t *p)
		{
			while (p)
			{
				tree_node_t *pn = p->next;
				destroy(p->children);
				delete p;
				p = pn;
			}
		}
		void print(std::ostream &os, tree_node_t *p, int level = 0)
		{
			for (; p; p = p->next)
			{
				for (int i(0); i < level; i++)
					os << "\t";
				os << p->name << std::endl;
				print(os, p->children, level + 1);
			}
		}
		tree_node_t *root() const
		{
			return mp_root;
		}
		bool empty() const
		{
			assert(!mp_root->next);
			return (mp_root->children == nullptr);
		}
	};

	template <typename T>
	class tree_builder : public std::vector<tree_node<T> *>//keeps last item on each level
	{
		typedef std::vector<tree_node<T>*> base_t;
		using base_t::size;
		using base_t::empty;
		using base_t::back;
		using base_t::pop_back;
		using base_t::push_back;

		int m_lines;
		tree_t<T>	&mr_tree;

	public:
		typedef tree_node<T> tree_node_t;

	public:
		tree_builder(tree_t<T> &r)
			: m_lines(0),
			mr_tree(r)
		{
			assert(mr_tree.root() && !mr_tree.root()->children);
			push_back(mr_tree.root());//a root
		}

		void add(size_t l, tree_node_t *node)
		{
			tree_node_t *lviAfter(nullptr);

			assert(size() > 0);
			l += 2;//fix for a root

			if (l > size())
			{
				assert(l - size() == 1);
			}
			else if (l == size())
			{
				if (!empty())
				{
					lviAfter = back();
					pop_back();
				}
			}
			else
			{
				while (l <= size())
				{
					lviAfter = back();
					pop_back();
				}
			}

			if (lviAfter)
				lviAfter->next = node;
			else
				back()->children = node;

			push_back(node);
			m_lines++;
		}
	};

}//namespace my

unsigned long upper_power_of_two(unsigned long v);

template <typename T>
unsigned TProgress(T cur, T range)
{
	double p((double)cur);
	p /= range;
	return (unsigned)(p * 100);
}

template <typename T>
class ProgressMonitor_t
{
	T	m_from;
	T	m_to;
	unsigned m_percent;
	const char *mpDesc;
public:
	ProgressMonitor_t(const char *pDesc)
		: m_from(0),
		m_to(0),
		m_percent(-1),
		mpDesc(pDesc)
	{
	}
	ProgressMonitor_t(T from, T to)
		: m_from(from),
		m_to(to),
		m_percent(-1)
	{
		if (m_to < m_from)
			m_to = m_from;
	}
	const char *desc() const { return mpDesc; }
	void setDesc(const char *desc){ mpDesc = desc; }
	bool update(T cur)
	{
		if (cur < m_from)
			cur = m_from;
		if (cur > m_to)
			cur = m_to;
		unsigned old(m_percent);
		m_percent = unsigned((double(cur - m_from) / (m_to - m_from)) * 100);
		return m_percent != old;
	}
	void reset()
	{
		m_percent = -1;
	}
	void reset(T from, T to)
	{
		m_from = from;
		m_to = to;
		m_percent = -1;
		if (m_to < m_from)
			m_to = m_from;
	}
	unsigned percent() const
	{
		return m_percent;
	}
};

template <typename T>
unsigned HexWidthMax(T a)
{
	unsigned n(0);
	for (T b(a); b != 0; b >>= 4)
		n++;
	return n;
}

template <typename T>
std::string NumberToString(T Number)
{
	std::ostringstream ss;
	ss << Number;
	return ss.str();
}

template <typename T>
std::string HexNumberToString(T Number)
{
	std::ostringstream ss;
	ss << std::hex;
	ss << std::setfill('0');
	ss << Number;
	return ss.str();
}

template <typename T, int align>
T PADDING(T offset){
	return (align - (offset & (align - 1))) & (align - 1);
	//return (-offset & (align - 1));
}

template <typename T>
T ALIGNED(T offset, int align){
	return (offset + T(align - 1)) & ~T(align - 1);
	//return (offset + (align - 1)) & ((T)-align);
}


#define DIR_SEP_WIN	'\\'
#define DIR_SEP_UNIX	'/'
#ifdef WIN32
#define DIR_SEP	DIR_SEP_WIN
#else
#define DIR_SEP	DIR_SEP_UNIX
#endif

template <typename T>
struct MyTreeVectorElt
{
	T	*pNode;
	size_t		uParentIndex;
	size_t		uChildrenIndex;//not valid if iChildrenNum<1
	int			iChildrenNum;//0: no children, -1: not fetched
	MyTreeVectorElt(T *node, size_t parent)
		: pNode(node), uParentIndex(parent), uChildrenIndex(0), iChildrenNum(-1)
	{
	}
};

template <typename T>
class MyTreeVector : public std::vector<MyTreeVectorElt<T> >
{
	typedef std::vector<MyTreeVectorElt<T> > base_t;
	using base_t::push_back;
	using base_t::back;
	using base_t::at;
	using base_t::size;
	using base_t::empty;
	typedef MyTreeVectorElt<T>	elt_t;
public:
	MyTreeVector(){}

	inline elt_t &addEltRoot(T *root)
	{
		push_back(elt_t(root, 0));
		return back();
	}

	inline elt_t &eddElt(T *node, size_t parent)
	{
		push_back(elt_t(node, parent));
		elt_t &a(back());
		elt_t &aParent(at(parent));
		if (aParent.iChildrenNum < 0)
		{
			aParent.iChildrenNum = 0;
			aParent.uChildrenIndex = size() - 1;
		}
		aParent.iChildrenNum++;
		return a;
	}

	bool hasChildren(size_t parent, bool bFetched) const
	{
		if (!empty())
		{
			const elt_t &a(at(parent));
			if (!(a.iChildrenNum < 0))//if it was fetched already - return it
				return (a.iChildrenNum > 0);
			if (!bFetched)
				return (a.pNode->children != nullptr);
		}
		return false;
	}
	unsigned childrenNum(size_t parent) const//children count
	{
		if (empty())
			return 0;
		const elt_t &a(at(parent));
		if (a.iChildrenNum < 0)
			return 0;
		return unsigned(a.iChildrenNum);
	}

	T *data(size_t row) const
	{
		assert(isValid(row));
		const elt_t &a(at(row));
		return a.pNode;
	}

	size_t idOfChild(size_t parent, unsigned childIndex) const
	{
		assert(isValid(parent));
		const elt_t &a(at(parent));
		assert(a.iChildrenNum > 0 && childIndex < unsigned(a.iChildrenNum));
		assert(a.uChildrenIndex > 0);
		return a.uChildrenIndex + childIndex;
	}
	size_t idOfParent(size_t child) const
	{
		assert(isValidNotRoot(child));
		const elt_t &a(at(child));
		assert(isValid(a.uParentIndex));
		return a.uParentIndex;
	}
	unsigned indexOf(size_t item) const
	{
		assert(isValidNotRoot(item));
		const elt_t &a(at(item));
		if (a.uParentIndex)
		{
			const elt_t &b(at(a.uParentIndex));
			assert(item >= b.uChildrenIndex);
			return unsigned(item - b.uChildrenIndex);//where children start
		}
		return (unsigned)item - 1;//a root: subtract a nul element
	}

	bool isValidNotRoot(size_t row) const
	{
		assert(row);
		return row < size();
	}
	bool isValid(size_t row) const
	{
		if (!row)
			return true;//a root
		return isValidNotRoot(row);
	}
	inline elt_t& at_(const size_t i){ return at(i); }
	inline const elt_t& at_(const size_t i) const { return at(i); }
	inline bool isEmpty() const { return empty(); }
};

template <typename T>
bool IsPowerOf2(T x) {
	return x > 0 && !(x & (x-1));
}

template <typename T>
class unique_list : public std::list<T>
{
	std::set<T>	m;
public:
	unique_list(){}
	bool add(T p)
	{
		//check if allredy included
		std::pair<typename std::set<T>::iterator, bool> ret(m.insert(p));
		if (!ret.second)
			return false;
		this->push_back(p);
		return true;
	}
	bool remove(T p)
	{
		//check if allredy included
		typename std::set<T>::iterator i(m.find(p));
		if (i == m.end())
			return false;
		for (typename std::list<T>::iterator j(this->begin()); j != this->end(); j++)
		{
			if (*j == p)
			{
				this->erase(j);
				return true;
			}
		}
		assert(0);
		return true;
	}
};

template <typename T>
class unique_vector : protected std::vector<T>
{
	typedef	std::vector<T>	base_t;
	using base_t::resize;
	using base_t::at;
	typedef std::map<T, size_t> mapped;
private:
	mapped	m;//from T to index
public:
	unique_vector(){}
	unique_vector(size_t n) : std::vector<T>(n){}//biased
	const std::vector<T> &base() const { return *this; }
	void reserve(size_t n){
		base_t::reserve(n);
	}
	void reset(size_t n){
		clear();
		resize(n);
	}
	void clear(){
		base_t::clear();
		m.clear();
	}
	size_t size() const {
		return base_t::size();
	}
	size_t add(T t){
		std::pair<typename mapped::iterator, bool> res;
		res = m.insert(std::make_pair(t, size()));
		if (!res.second)
			return res.first->second;
		base_t::push_back(t);
		return size() - 1;
	}
	const T &operator[](size_t i) const { return at(i); }
	const T operator[](size_t i){ return at(i); }
	int indexOf(T t) const {
		typename mapped::const_iterator i(m.find(t));
		if (i == m.end())
			return -1;
		return (int)i->second;
	}
	//void push(T t){ append(t); }
};

template <typename T>
size_t PADDING(size_t offset)
{
	return (sizeof(T) - (offset % sizeof(T))) % sizeof(T);
}

template <typename T>
size_t ALIGNED(size_t offset)
{
	return offset + PADDING<T>(offset);
}

template <typename T>
T EXTENDED(T offset, int align)
{
	return ALIGNED(offset, align) / align;
}

/*template <typename T>
unsigned ALLIGNED(size_t Address)
{
size_t Offset = sizeof(T)-(Address % sizeof(T));
return Address + Offset;
}*/


#ifdef WIN32
#define STRICMP	stricmp
#else
#define STRICMP	strcasecmp
#endif

inline bool icasecmp(const std::string& l, const std::string& r)
{
    return l.size() == r.size()
        && equal(l.cbegin(), l.cend(), r.cbegin(),
            [](std::string::value_type l1, std::string::value_type r1)
                { return toupper(l1) == toupper(r1); });
}

inline bool icasecmp(const std::wstring& l, const std::wstring& r)
{
    return l.size() == r.size()
        && equal(l.cbegin(), l.cend(), r.cbegin(),
            [](std::wstring::value_type l1, std::wstring::value_type r1)
                { return towupper(l1) == towupper(r1); });
}

template <typename T>//T:unsigned int
int PrintWidth(T i)
{
	int w(0);
	do {
		w++;
		i /= 10;
	} while (i > 0);
	return w;
}

void PrintHex(unsigned, unsigned width, char buf[32]);

#include "qx/MyStringList.h"

class MyStringEx : MyStringList
{
public:
	MyStringEx() {}
	MyStringEx(const MyString& s)
		: MyStringList(split("\n", s, true)){
	}
	MyStringEx(const char* pc)
		: MyStringList(split("\n", pc, true)){
	}
	MyStringEx& operator=(const MyString& s)
	{
		std::list<MyString>& self(*this);
		self = MyStringList(split("\n", s, true));
		return *this;
	}
	operator const MyString& () const {
		if (!MyStringList::empty())
			return front();
		static const MyString dummy;
		return dummy;
	}
	const MyString& at(size_t i) const {
		for (const_iterator it(begin()); it != end(); ++it)
			if (i-- == 0)
				return *it;
		static const MyString dummy;
		return dummy;
	}
	const MyString& operator[](size_t i) const {
		return at(i);
	}
	void set(size_t i, const MyString& s){
		if (!(i < size()))
			resize(i + 1);
		for (iterator it(begin()); it != end(); ++it)
			if (i-- == 0)
			{
				*it = s;
				break;
			}
	}
	void push(const MyString& s)
	{
		push_back(s);
	}
	operator bool() const { return !MyStringList::empty(); }
	const char *c_str(const char *p = nullptr) const {
		if (!MyStringList::empty())
			return front().c_str();
		return p;
	}
	bool empty(size_t i) const {
		return at(i).isEmpty();
	}
	void clear(){
		MyStringList::clear();
	}
	size_t size() const {
		return MyStringList::size();
	}
};

////////////////////////////////// LEB128

template <typename T, typename TS>
size_t decode_ULEB128(T& result, TS& is)
{
	size_t count(0);
	result = 0;
	uint8_t byte;
	for (int shift(0); shift + 7 < sizeof(T) * CHAR_BIT; shift += 7)
	{
		byte = is.get();
		result |= (T)(byte & 0x7f) << shift;
		++count;
		if (!(byte & 0x80))
			return count;
	}
	return 0;
}

template <typename T, typename TS>
size_t encode_ULEB128(const T& value, TS& os)
{
	size_t count(0);
	do {
		uint8_t byte = value & 0x7f;
		value >>= 7;
		if (value != 0) // more bytes to come
			byte |= 0x80;//set high order bit of byte
		os.put(byte);//emit byte
		count++;
	} while (value != 0);
	return count;
}

/// Utility function to encode a ULEB128 value to a buffer. Returns
/// the length in bytes of the encoded value.
template <typename T>
inline size_t encode_ULEB128(T Value, uint8_t* p, size_t p_max = 0)//p_max == 0: don't care
{
	uint8_t* orig_p = p;
	size_t Count = 0;
	do {
		uint8_t Byte = Value & 0x7f;
		Value >>= 7;
		if (Value != 0)
			Byte |= 0x80; // Mark this byte to show that more bytes will follow.
		*p++ = Byte;
		if (++Count == p_max)
			break;
	} while (Value != 0);
	return Count;
}

template <typename T, typename TS>
size_t decode_SLEB128(T& result, TS& is)
{
	int count(0);
	result = 0;
	int shift = 0;
	int size = sizeof(T) * CHAR_BIT;
	uint8_t byte;
	do {
		byte = is.get();
		count++;
		result |= (T)(byte & 0x7f) << shift;
		shift += 7;
	} while (byte & 0x80);

	// sign bit of byte is second high order bit (0x40)
	if ((shift < size) && (byte & 0x40))
		// sign extend
		result |= (~0 << shift);
	return count;
}

 template <typename T, typename TS>
 size_t encode_SLEB128(const T& value, TS &os)
 {
	 bool more;
	 size_t count(0);
	 do {
		 uint8_t byte = value & 0x7f;
		 value >>= 7;
		 more = !((((value == 0) && ((byte & 0x40) == 0)) || ((value == -1) && ((byte & 0x40) != 0))));
		 if (more)
			 byte |= 0x80; // Mark this byte to show that more bytes will follow.
		 os.put(byte);
		 count++;
	 } while (more);
	 return count;
 }

template <typename TS>
size_t skip_LEB128(TS& is)
{
	size_t sz(1);
	char ch;
	for (; sz < 16 && (ch = is.get()); ++sz)
		if (!(ch & 0x80))
			break;
	return sz;
}

inline size_t skip_LEB128(const uint8_t* p)
{
	size_t sz(1);
	char ch;
	for (; sz < 16 && (ch = *p); ++sz, ++p)
		if (!(ch & 0x80))
			break;
	return sz;
}


/* a=target variable, b=bit number to act upon 0-n */
#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))        // '!!' to make sure this returns 0 or 1

/* x=target variable, y=mask */
#define BITMASK_SET(x,y) ((x) |= (y))
#define BITMASK_CLEAR(x,y) ((x) &= (~(y)))
#define BITMASK_FLIP(x,y) ((x) ^= (y))
#define BITMASK_CHECK_ALL(x,y) (!(~(x) & (y)))
#define BITMASK_CHECK_ANY(x,y) ((x) & (y))

//Changing the nth bit to x
#define CHANGE_NTH_BIT0(a, n, x)	a ^= (-x ^ a) & (1UL << n)
#define CHANGE_NTH_BIT(a, n, x)	a = (a & ~(1LL << n)) | (x << n);

template <typename T>
T ChangeBit(T a, unsigned mask, bool x)
{
	return a ^ (-(int)x ^ a) & mask;
	//return (a & ~(1UL << n)) | (x << n);
}

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);

bool fileExists(const std::string& filename);




