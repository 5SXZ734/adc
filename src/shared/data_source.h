#pragma once

#include <fstream>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <exception>
#include "qx/IUnk.h"
#include "misc.h"
#include "mmap.h"

////////////////////////////////////////////// I_AuxData
class I_AuxData : public My::IUnk
{
public:
	virtual PDATA data() const = 0;
	virtual OFF_t size() const = 0;
	bool isNull(OFF_t off) const { return !(off < size()); }
};


/////////////////////////////////// DataAccessFault_t
class DataAccessFault_t : public std::exception
{
	int		m_code;
	OFF_t	m_pos;
	OFF_t	m_size;
public:
	DataAccessFault_t()
		: m_code(0),
		m_pos(0),
		m_size(0)
	{
	}
	DataAccessFault_t(int code, OFF_t pos, OFF_t size)
		: m_code(code),
		m_pos(pos),
		m_size(size)
	{
	}
	int code() const { return m_code; }
	OFF_t pos() const { return m_pos; }
	OFF_t size() const { return m_size; }
	virtual const char * what() const throw (){
		return "Data Access Fault";
	}
};


////////////////////////////////////////////// I_DataSourceBase
class I_DataSourceBase
{
public:
	virtual void release(){}
	virtual size_t dataAt(OFF_t off, OFF_t siz, PDATA) const = 0;
	virtual OFF_t size() const = 0;
	virtual OFF_t pos(OFF_t o) const { return o; }
	virtual const I_DataSourceBase *host() const { return nullptr; }
	virtual const I_AuxData *aux() const { return nullptr; }
	virtual void setAuxData(PDATA, size_t){}
	virtual void clearAuxData(){}
	bool isNull(OFF_t off) const { return !(off < size()); }
	virtual I_DataSourceBase *openFile(const char *, bool /*bLocal*/ = false) const { return nullptr; }//must be released
	virtual int moduleId() const { return 0; }
	virtual const char *modulePath() const { return nullptr; }
};


////////////////////////////////////////////// Block_t
struct Block_t
{
	OFF_t	m_offs;
	OFF_t	m_size;
	Block_t() : m_offs(0), m_size(0){}
	Block_t(OFF_t a, OFF_t s)	: m_offs(a), m_size(s){}
	Block_t(const Block_t &o)	: m_offs(o.m_offs), m_size(o.m_size){}
	void operator=(const Block_t &o){
		m_offs = o.m_offs;
		m_size = o.m_size;
	}
	bool operator==(const Block_t &o) const {
		return m_offs == o.m_offs && m_size == o.m_size;
	}
	Block_t &subspace(OFF_t doffs, OFF_t dsize)
	{
		if ((int)doffs < m_size)
		{
			m_offs += doffs;
			m_size -= doffs;
			m_size = std::min(m_size, dsize);
		}
		else
		{
			m_offs = 0;
			m_size = 0; 
		}
		return *this;
	}
	bool empty() const { return m_size == 0; }
	//OFF_t rawoffs() const { return m_offs; }
	//OFF_t rawsize() const { return m_size; }
	void set(OFF_t _offs, OFF_t _size)
	{
		m_offs = _offs;
		m_size = _size;
	}
	OFF_t lower() const { return m_offs; }
	OFF_t upper() const { return m_offs + m_size; }
	OFF_t size() const { return m_size; }
};


/*class DataSourcePane_t : public I_DataSourceBase
{
	const I_DataSourceBase &m_target;
	OFF_t	m_lower;
	OFF_t	m_upper;
public:
	DataSourcePane_t(const I_DataSourceBase &target, OFF_t lower, OFF_t upper, int z)
		: m_target(target),
		m_lower(lower),
		m_upper(upper)
	{
		assert(m_lower < m_target.size());
		assert(m_upper <= m_target.size());
	}
	DataSourcePane_t(const I_DataSourceBase &target, const Block_t &block)
		: m_target(target),
		m_lower(block.lower()),
		m_upper(block.upper())
	{
		assert(m_lower < m_target.size());
		assert(m_upper <= m_target.size());
	}
protected:
	virtual size_t dataAt(OFF_t off, OFF_t siz, PDATA pData) const
	{
		assert(off < size());
		return m_target.dataAt(m_lower + off, siz, pData);
	}
	virtual OFF_t size() const
	{
		return m_upper - m_lower;
	}
	virtual OFF_t pos(OFF_t o) const
	{
		return m_target.pos(0) + o;
	}
};*/


////////////////////////////////////////////// DataSubSource_t
class DataSubSource_t : public I_DataSourceBase
{
	const I_DataSourceBase &m_host;
	OFF_t	m_lower;
	OFF_t	m_upper;
public:
	DataSubSource_t(const I_DataSourceBase &r)
		: m_host(r),
		m_lower(0),
		m_upper(r.size())
	{
	}
	DataSubSource_t(const I_DataSourceBase &r, OFF_t lower, OFF_t extent)//may throw
		: m_host(r),
		m_lower(lower),
		m_upper(std::min(m_lower + extent, r.size()))
	{
		assert(m_lower <= m_host.size());
		assert(m_upper <= m_host.size());
	}
	DataSubSource_t(const I_DataSourceBase &host, const Block_t &block)
		: m_host(host),
		m_lower(block.lower()),
		m_upper(block.upper())
	{
		assert(m_lower <= m_host.size() || block.empty());
CHECK(!(m_upper <= m_host.size() || block.empty()))
STOP
		assert(m_upper <= m_host.size() || block.empty());
	}
	DataSubSource_t(const DataSubSource_t &o)
		: m_host(o.m_host),
		m_lower(o.m_lower),
		m_upper(o.m_upper)
	{
	}
	DataSubSource_t(const DataSubSource_t &o, OFF_t extent)//for range expansion
		: m_host(o.m_host),
		m_lower(o.m_lower),
		m_upper(o.m_upper + extent)
	{
	}
	void operator=(const DataSubSource_t &o)
	{
		assert(&m_host == &o.m_host);
		m_lower = o.m_lower;
		m_upper = o.m_upper;
	}
	virtual size_t dataAt(OFF_t off, OFF_t siz, PDATA pData) const
	{
		if (off >= size())
			throw DataAccessFault_t(-11, pos(off), siz);
		if (off + siz > size())
			throw DataAccessFault_t(-12, pos(off), siz);
		return m_host.dataAt(m_lower + off, siz, pData);
	}
	virtual OFF_t size() const
	{
		return m_upper - m_lower;
	}
	virtual OFF_t pos(OFF_t off) const
	{
		return m_host.pos(0) + off;
	}
};



////////////////////////////////////////////// DataStream_t
class DataStream_t
{
protected:
	const I_DataSourceBase &mr;
	OFF_t m_cur;//absolute position
public:
	DataStream_t(const I_DataSourceBase &r)
		: mr(r),
		m_cur(0)
	{
	}
	DataStream_t(const I_DataSourceBase &r, OFF_t o)
		: mr(r),
		m_cur(o)
	{
	}
	DataStream_t(const DataStream_t &o)
		: mr(o.mr),
		m_cur(o.m_cur)
	{
	}
	/*DataStream_t(const I_Module &r)
		: mr(r),
		m_cur(r.cpr())
	{
	}*/
	operator bool() const
	{
		return mr.size() > 0;
	}
	void operator=(const DataStream_t &o)
	{
		assert(&mr == &o.mr);
		m_cur = o.m_cur;
	}
	size_t read(size_t bytes, PDATA p)
	{
		size_t read(mr.dataAt(m_cur, bytes, p));
		m_cur += read;
		return read;
	}
	template <typename T>
	size_t read(T &t)
	{
		size_t bytes(mr.dataAt(m_cur, sizeof(T), (PDATA)&t));
		m_cur += bytes;
		return bytes;
	}
	template <typename T>
	T read()
	{
		T t;
		m_cur += mr.dataAt(m_cur, sizeof(T), (PDATA)&t);
		return t;
	}
	template <typename T>
	T peek(){
		T t;
		mr.dataAt(m_cur, sizeof(T), (PDATA)&t);
		return t;
	}
	int get()//1 character
	{
		char t;
		m_cur += mr.dataAt(m_cur, sizeof(t), (PDATA)&t);
		return t;
	}
	size_t forward(size_t d){
		size_t i(0);
		for (; i < d; i++)
		{
			char ch(read<char>());
			(void)ch;
		}
		return i;
	}
	template <typename T>
	size_t forward(){
		return forward(sizeof(T));
	}
	size_t backward(unsigned d){
		if ((OFF_t)d > m_cur)
			d = (unsigned)m_cur;
		m_cur -= d;
		return d;
	}
	int move(int d){
		if (d > 0)
			return int(forward(d));
		if (d < 0)
			return -int(backward(-d));
		return 0;
	}
	template <typename T>
	size_t skip(){
		return forward(sizeof(T));
	}
	size_t skip(size_t d){
		return forward(d);
	}
	bool seek(OFF_t o) {
		m_cur = o;
		return isValid(o);
	}
	OFF_t tell() const {
		return m_cur;
	}
	OFF_t current() const {
		return m_cur;
	}
	bool isAtEnd() const {
		return !isValid(m_cur);
	}

	//////////////////////////////////////// LEB128

	/*template <typename T>
	size_t decode_ULEB128(T &result)
	{
		size_t count(decode_ULEB128(result, *this));
		assert(count != 0);
		return count;
	}*/
	template <typename T>
	T read_ULEB128()
	{
		T t;
		::decode_ULEB128(t, *this);
		return t;
	}
	/*template <typename T>
	size_t decode_SLEB128(T &result)
	{
		int count(get_SLEB128(result, *this));
		assert(count != 0);
		return count;
	}*/
	template <typename T>
	T read_SLEB128()
	{
		T t;
		::decode_SLEB128(t, *this);
		return t;
	}

	///////////////////////////////// strings
	template <typename T>
	size_t skipStringZ()//0-terminated
	{
		size_t n(1);//eos
		while (read<T>() != 0)
			n++;
		return n * sizeof(T);
	}
	template <typename T>
	size_t skipLPfxString()//length-prefixed
	{
		T len(read<T>());
		return sizeof(T) + skip(len);
	}
	template <typename T>
	size_t fetchString(std::basic_ostream<T> &s)//zero-terminated
	{
		size_t i(0);
		T ch;
		for (; (ch = read<T>()) != 0; ++i)
			s << ch;
		return i * sizeof(T);//number of bytes read?
	}
	template <typename T>
	size_t fetchString(std::basic_ostream<T> &s, size_t n)
	{
		if (n == 0)
			return fetchString(s);
		size_t i(0);
		T ch;
		for (; i < n && (ch = read<T>()) != 0; ++i)
			s << ch;
		return i * sizeof(T);
	}
	template <typename T>
	size_t fetchString(std::basic_ostream<T> &s, size_t imax, char eos)//zero-terminated
	{
		size_t i(0);
		T ch;
		for (; (ch = read<T>()) != eos && i < imax; ++i)
			s << ch;
		return i * sizeof(T);//number of bytes read?
	}
	template <typename T>
	size_t fetchLPfxString(std::basic_ostream<T> &s)//length-prefixed
	{
		T len(read<T>());
		int i(1);
		for (; i <= len; ++i)
			s << read<T>();
		return i * sizeof(T);
	}
	/*template <typename T>
	int strCmp(const T *p2, size_t n = (size_t)-1) const
	{
		for (size_t i(0); i < n; i++)
		{
			unsigned T c1 = read<T>();
			unsigned T c2 = *p2;
			if (c1 != c2)
				return c1 < c2 ? -1 : 1;
			if (c1 == 0)
				break;
			++p1;
			++p2;
		}
		return 0;
	}*/
	int strCmp(const char* p2, size_t n = (size_t)-1) const
	{
		DataStream_t p1(*this);
		for (size_t i(0); i < n; i++)
		{
			unsigned char c1 = p1.read<unsigned char>();
			unsigned char c2 = *p2;
			if (c1 != c2)
				return c1 < c2 ? -1 : 1;
			if (c1 == 0)
				break;
			//++p1;
			++p2;
		}
		return 0;
	}
	int strCmp(const char16_t* p2, size_t n = (size_t)-1) const
	{
		DataStream_t p1(*this);
		for (size_t i(0); i < n; i++)
		{
			char16_t c1 = p1.read<char16_t>();
			char16_t c2 = *p2;
			if (c1 != c2)
				return c1 < c2 ? -1 : 1;
			if (c1 == 0)
				break;
			//++p1;
			++p2;
		}
		return 0;
	}

	////////////////////////////////////// align
	template <typename T>
	OFF_t align(){
		m_cur = ALIGNED(m_cur, sizeof(T));
		return m_cur;
	}
	OFF_t align(unsigned u){
		m_cur = ALIGNED(m_cur, u);
		return m_cur;
	}
protected:
	bool isValid(OFF_t o) const {
		return (o < mr.size());
	}
};


//////////////////////////////////////// AuxDataSource_t
class AuxDataSource_t : public I_AuxData
{
	PDATA	mpData;
	size_t	mSize;
public:
	AuxDataSource_t(PDATA p, size_t sz, bool bCopy)
		: mpData(p),
		mSize(sz)
	{
		assert(p);
		if (bCopy)
		{
			mpData = new char[sz];
			memcpy((void *)mpData, p, sz);
		}
	}
	~AuxDataSource_t()
	{
		delete[] mpData;
	}
	virtual PDATA data() const { return mpData; }
	virtual OFF_t size() const { return mSize; }
};



/////////////////////////////////////////////////////////////////////
class PatchList_t : public std::list<std::pair<OFF_t, size_t> >//position : size
{
	typedef std::map<OFF_t, size_t>	M_t;
	M_t	m;//a map to check consistency of the patches - NO OVERLAPS!
public:
	bool append(OFF_t oInParent, size_t uChunkSize)
	{
		if (!checkRange(oInParent, uChunkSize))
			return false;
		push_back(std::make_pair(oInParent, uChunkSize));
		return true;
	}
private:
	bool checkRange(OFF_t oInParent, size_t uChunkSize)
	{
		M_t::iterator j(m.lower_bound(oInParent));
		if (j != m.end())
		{
			if (j->first ==	 oInParent)
				return false;//the patch already exists
			assert(j->first > oInParent);
			if (oInParent + uChunkSize > j->first)
				return false;//the patch is overlapping the next one 
		}
		if (j != m.begin())
		{
			j--;
			assert(j->first < oInParent);
			if (j->first + j->second > oInParent)
				return false;//the previous patch overlaps this one
		}
		m.insert(std::make_pair(oInParent, uChunkSize));//should not fail
		return true;
	}
};


class data_leech_t
{
public:
	struct S_t
	{
		OFF_t	upper_self;//chunk's upper margin in own data range
		OFF_t	lower_host;//chunk's lower margin in parent data range
		S_t() : upper_self(0), lower_host(0){}
		S_t(OFF_t a, OFF_t b) : upper_self(a), lower_host(b){}
	};
	typedef std::vector<S_t>	V_t;
private:
	struct S_comp_t
	{
		bool operator() (OFF_t a, const S_t & b) const
		{
			return a < b.upper_self;
		}
	};
	V_t	mChunks;
public:
	data_leech_t(){}
	data_leech_t(const PatchList_t &pm)
	{
		assert(!pm.empty());
		//build a chunks vector
		mChunks.reserve(pm.size());
		size_t curSize(0);
		for (PatchList_t::const_iterator i(pm.begin()); i != pm.end(); i++)
		{
			assert(i->second != 0);
			curSize += i->second;
			mChunks.push_back(S_t(curSize, i->first));
		}
	}
	V_t &chunks(){ return mChunks; }
	const V_t &chunks() const { return mChunks; }
	OFF_t size() const
	{
		if (mChunks.empty())
			return 0;
		return mChunks.back().upper_self;
	}
	OFF_t pos(OFF_t offs) const
	{
		V_t::const_iterator it(std::upper_bound(mChunks.begin(), mChunks.end(), offs, S_comp_t()));
		size_t i(std::distance(mChunks.begin(), it));
		OFF_t lower_self(i == 0 ? 0 : mChunks[i - 1].upper_self);
		OFF_t chunk_offs(offs - lower_self);
		if (!(i < mChunks.size()))
			throw DataAccessFault_t(5, offs, -1);
		return mChunks[i].lower_host + chunk_offs;
	}
	size_t dataAt(OFF_t offs, OFF_t size, PDATA pDest, const I_DataSourceBase &rDataHost) const
	{
		V_t::const_iterator it(std::upper_bound(mChunks.begin(), mChunks.end(), offs, S_comp_t()));
		//assert(it != mChunks.begin());
		//it--;
		size_t count(0);
		for (size_t i(std::distance(mChunks.begin(), it)); i < mChunks.size(); i++)
		{
			OFF_t lower_self(i == 0 ? 0 : mChunks[i - 1].upper_self);
			OFF_t chunk_size(mChunks[i].upper_self - lower_self);
			OFF_t chunk_offs(offs - lower_self);
			size_t bytes(std::min(size_t(chunk_size - chunk_offs), size_t(size)));
			size_t bytes2(rDataHost.dataAt(mChunks[i].lower_host + chunk_offs, bytes, pDest));
			count += bytes2;
			offs += bytes2;
			if (bytes2 != bytes)
				break;
			if ((size -= bytes2) == 0)
				break;
			pDest += bytes2;
		}
		return count;
	}

};

class I_DataSourceLeech : public I_DataSourceBase,
	private data_leech_t
{
	const I_DataSourceBase &mrHost;
public:
	I_DataSourceLeech(const PatchList_t &pm, const I_DataSourceBase &rHost)
		: data_leech_t(pm),
		mrHost(rHost)
	{
	}
	virtual size_t dataAt(OFF_t rawOffs, OFF_t size, PDATA pData) const {
		return data_leech_t::dataAt(rawOffs, size, pData, mrHost);
	}
	virtual OFF_t size() const {
		return data_leech_t::size();
	}
	virtual OFF_t pos(OFF_t o) const {
		return data_leech_t::pos(o);
	}
	virtual const I_DataSourceBase *host() const {
		return &mrHost;
	}
};

/////////////////////////////////////////////////////////////////// DataSourceNull_t
class DataSourceNull_t : public I_DataSourceBase//dummy data source
{
public:
	DataSourceNull_t(){}
protected:
	virtual size_t dataAt(OFF_t /*off*/, OFF_t /*siz*/, PDATA) const { return 0; }
	virtual OFF_t size() const { return 0; }
};



/////////////////////////////////////////////////////////////// DataFetch_t simple types
template <typename T>
class DataFetch_t
{
protected:
	T	mt;
public:
	DataFetch_t(const I_DataSourceBase &r, OFF_t _cpr)//may throw
	{
		if (r.dataAt(_cpr, sizeof(T), (PDATA)&mt) != sizeof(T))
			throw(-333);
	}
	/*DataFetch_t(const DataFetch_t &o)
		: mt(o.mt)
	{
	}*/
	operator const T&() const {
		return mt; }
	const T& get() const {
		return mt;
	}
	//bool operator==(const T &o) const { return mt == o; }
	size_t size() const { return sizeof(mt); }
};

#if(0)

// simple types
template <typename T>
class DataFetchPtr_t : protected DataStream_t
{
protected:
	T	mt;
public:
	DataFetchPtr_t(const I_DataSourceBase &r, OFF_t _cpr)
		: DataStream_t(r, _cpr),
		mt(0)
	{
		if (isValid())
			read(mt);
			//if (fetch(_cpr) != sizeof(T))
				//throw(-334);
	}
	DataFetchPtr_t(const DataFetchPtr_t &o)
		: DataStream_t(o),
		mt(o.mt)
	{
	}
	DataFetchPtr_t &operator=(OFF_t t)
	{
		seek(t);
		if (isValid())
			read(mt);
			//if (fetch(tell()) != sizeof(T))
				//throw(-335);
		return *this;
	}
	const T &operator*() const {
		if (!isValid())
			throw(-336);
		return get(); }//indirection
	operator bool() const { return isValid(); }
	DataFetchPtr_t &operator++()
	{
		//mRawOffs += sizeof(T);
		if (isValid())
			read(mt);
			//if (fetch(mRawOffs) != sizeof(T))
				//mRawOffs = OFF_NULL;//make it invalid - do not throw(!)//throw(-336);
		return *this;
	}
	DataFetchPtr_t operator++(int){//postfix
		DataFetchPtr_t t(*this);
		operator++();
		return t;
	}
	T operator[] (const int index) const
	{
		assert(index >= 0);
		T t;
		DataStream_t ds(*this);
		ds.move(index * sizeof(T));//can be backward (if negative)
		ds.read(t);
		//if (const_cast<DataFetchPtr_t *>(this)->fetch(mRawOffs + index * sizeof(T), &t) != sizeof(T))
			//throw(-337);
		return t;
	}
	OFF_t cpr() const { return tell();  }
	/*const T& operator[] (const unsigned int index)
	{
		fetch(mRawOffs + index * sizeof(T));
		return get();
	}*/
/*	size_t skipString()
	{
		size_t n(1);//eos
		while (get())
		{
			this->operator++();
			n++;
		}
		this->operator++();//skip eos
		return n;
	}
	size_t fetchString(std::basic_ostream<T> &s, size_t n)
	{
		if (n == 0)
			return fetchString(s);
		DataFetchPtr_t &p(*this);
		size_t i(0);
		for (; i < n; i++, ++p)
			s << *p;
		return i;
	}
	size_t fetchString(std::basic_ostream<T> &s)
	{
		DataFetchPtr_t &p(*this);
		size_t i(0);
		for (; *p; ++p, i++)
			s << *p;
		++p;//eos
		return i;
	}
	int strCmp(const char *p2, size_t n = (size_t)-1) const
	{
		DataFetchPtr_t p1(*this);
		for (size_t i(0); i < n; i++)
		{
			unsigned char c1 = *p1;
			unsigned char c2 = *p2;
			if (c1 != c2)
				return c1 < c2 ? -1 : 1;
			if (c1 == 0)
				break;
			++p1;
			++p2;
		}
		return 0;
	}
	int strCmp(const wchar_t *p2, size_t n = (size_t)-1) const
	{
		DataFetchPtr_t p1(*this);
		for (size_t i(0); i < n; i++)
		{
			wchar_t c1 = *p1;
			wchar_t c2 = *p2;
			if (c1 != c2)
				return c1 < c2 ? -1 : 1;
			if (c1 == 0)
				break;
			++p1;
			++p2;
		}
		return 0;
	}*/
	bool isValid() const {
		return !isAtEnd();
		//return isValid(mRawOffs);
	}
	size_t size() const { return sizeof(T); }
	OFF_t current() const { return tell();  }

	operator const T& () const {
		return mt;
	}
	const T& get() const {
		return mt;
	}

	using DataStream_t::fetchString;
	using DataStream_t::skipString;
	using DataStream_t::strCmp;
protected:
	/*size_t fetch(OFF_t _cpr) {
		return mr.dataAt(_cpr, sizeof(T), (PDATA)&mt);
	}
	size_t fetch(OFF_t _cpr, T* pt) {
		return mr.dataAt(_cpr, sizeof(T), (PDATA)pt);
	}
	bool isValid(OFF_t _cpr) const {
		return (_cpr < mr.size());
	}*/
};

#else

// simple types
/*template <typename T>
class DataFetch_t
{
	T	mt;
protected:
	const I_DataSourceBase& mr;
public:
	DataFetch_t(const I_DataSourceBase& r)
		: mr(r),
		mt(0)
	{
	}
	DataFetch_t(const I_DataSourceBase& r, OFF_t _cpr)//may throw
		: mr(r)
	{
		if (fetch(_cpr) != sizeof(T))
			throw(-333);
	}
	DataFetch_t(const DataFetch_t& o)
		: mt(o.mt),
		mr(o.mr)
	{
	}
	operator const T& () const {
		return mt;
	}
	const T& get() const {
		return mt;
	}
	//bool operator==(const T &o) const { return mt == o; }
	size_t size() const { return sizeof(mt); }
protected:
	size_t fetch(OFF_t _cpr) {
		return mr.dataAt(_cpr, sizeof(T), (PDATA)& mt);
	}
	size_t fetch(OFF_t _cpr, T* pt) {
		return mr.dataAt(_cpr, sizeof(T), (PDATA)pt);
	}
	bool isValid(OFF_t _cpr) const {
		return (_cpr < mr.size());
	}
};*/

///////////////////////////////////////////////////// DataFetchPtr_t : simple types
template <typename T>
class DataFetchPtr_t
{
protected:
	T	mt;
	const I_DataSourceBase& mr;
	OFF_t mRawOffs;
public:
	DataFetchPtr_t(const I_DataSourceBase& r, OFF_t _cpr)
		: mt(0),
		mr(r),
		mRawOffs(_cpr)
	{
		if (isValid())
			if (fetch(_cpr) != sizeof(T))
				throw(-334);
	}
	DataFetchPtr_t(const DataFetchPtr_t& o)
		: mt(o.mt),
		mr(o.mr),
		mRawOffs(o.mRawOffs)
	{
	}
	DataFetchPtr_t& operator=(OFF_t t)
	{
		mRawOffs = t;
		if (isValid())
			if (fetch(mRawOffs) != sizeof(T))
				throw(-335);
		return *this;
	}
	const T& operator*() const {
		if (!isValid())
			throw(-336);
		return get();
	}//indirection
	operator bool() const { return isValid(); }
	DataFetchPtr_t& operator++()
	{
		mRawOffs += sizeof(T);
		if (isValid())
			if (fetch(mRawOffs) != sizeof(T))
				mRawOffs = OFF_NULL;//make it invalid - do not throw(!)//throw(-336);
		return *this;
	}
	DataFetchPtr_t operator++(int) {//postfix
		DataFetchPtr_t t(*this);
		operator++();
		return t;
	}
	T operator[] (const int index) const
	{
		T t;
		if (const_cast<DataFetchPtr_t*>(this)->fetch(mRawOffs + index * sizeof(T), &t) != sizeof(T))
			throw(-337);
		return t;
	}
	OFF_t cpr() const { return mRawOffs; }

	bool isValid() const {
		return isValid(mRawOffs);
	}
	size_t size() const { return sizeof(T); }
	OFF_t current() const { return mRawOffs; }

	operator const T& () const {
		return mt;
	}
	const T& get() const {
		return mt;
	}
protected:
	size_t fetch(OFF_t _cpr) {
		return mr.dataAt(_cpr, sizeof(T), (PDATA)& mt);
	}
	size_t fetch(OFF_t _cpr, T* pt) {
		return mr.dataAt(_cpr, sizeof(T), (PDATA)pt);
	}
	bool isValid(OFF_t _cpr) const {
		return (_cpr < mr.size());
	}
};


#endif


////////////////////////////////////////////////// DataFetch2_t : compound types
template <class T>
class DataFetch2_t : public T
{
protected:
	const I_DataSourceBase &mr;
public:
	DataFetch2_t(const I_DataSourceBase &r)
		: mr(r)
	{
	}
	DataFetch2_t(const I_DataSourceBase &r, OFF_t _cpr)
		: mr(r)
	{
		if (fetch(_cpr) != sizeof(T))//signal if out of range
			throw(-343);
	}
	const T& get() const {
		return *this;
	}
protected:
	size_t fetch(OFF_t _cpr){
		return mr.dataAt(_cpr, sizeof(T), (PDATA)this);
	}
	size_t fetch(OFF_t _cpr, T *pt) {
		return mr.dataAt(_cpr, sizeof(T), (PDATA)pt);
	}
	bool isValid(OFF_t _cpr) const {
		return (_cpr < mr.size());
	}
};

/////////////////////////////////////////////////////////// DataFetchPtr2_t : compound types
template <typename T>
class DataFetchPtr2_t : public DataFetch2_t<T>
{
	typedef DataFetch2_t<T>	Base_t;
	using Base_t::fetch;
protected:
	OFF_t mRawOffs;
public:
	DataFetchPtr2_t(const I_DataSourceBase &r, OFF_t _cpr)
		: DataFetch2_t<T>(r),
		mRawOffs(_cpr)
	{
		if (isValid())//do not signal if out of range!
			if (fetch(_cpr) != sizeof(T))
				throw(-344);
	}
	DataFetchPtr2_t(const DataFetchPtr2_t &o)
		: DataFetch2_t<T>(o),
		mRawOffs(o.mRawOffs)
	{
	}
	DataFetchPtr2_t &operator=(OFF_t t)
	{
		mRawOffs = t;
		if (isValid())
			if (fetch(mRawOffs) != sizeof(T))
				throw(-345);
		return *this;
	}
	operator bool() const { return isValid(); }
	DataFetchPtr2_t &operator++()
	{
		mRawOffs += sizeof(T);
		if (isValid())
			if (fetch(mRawOffs) != sizeof(T))
				mRawOffs = OFF_NULL;//make it invalid - do not throw(!)//throw(-346);
		return *this;
	}
	DataFetchPtr2_t operator ++(int){
		DataFetchPtr2_t t(*this);
		operator++();
		return t;
	}
	const T *operator->() const {
		return this;
	}
	T operator[] (const int index) const
	{
		T t;
		OFF_t o(mRawOffs);
		if (index < 0)
			o -= (-index) * sizeof(T);
		else
			o += index * sizeof(T);
		if (const_cast<DataFetchPtr2_t *>(this)->fetch(o, &t) != sizeof(T))
			throw(-347);
		return t;
	}
	const T &operator*() const {
		assert(isValid());
		return Base_t::get(); }//indirection
	bool isValid() const {
		return Base_t::isValid(mRawOffs);
	}
	size_t size() const { return sizeof(T); }
	OFF_t lowerBound() const { return mRawOffs;  }
	OFF_t upperBound() const { return mRawOffs + sizeof(T);  }
};






