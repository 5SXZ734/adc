#pragma once

#include <vector>
#include <algorithm>
#include <type_traits>
#include "MyStream.h"

#include <stdint.h>



/////////////////////////////////////////////////////////////////////SkewVector
typedef std::pair<size_t, int>	SkewPair;

class SkewVector : protected  std::vector<SkewPair>
{
	typedef std::vector<SkewPair> Base;
	struct SkewComparator {
		bool operator()(const SkewPair& a, const SkewPair& b) const {
			return a.first < b.first;
		}
	};
public:
	SkewVector(){}
	void preprocess(size_t iSize)
	{
		std::sort(begin(), end(), SkewComparator());

		//transform the dead indices vector into shift lookup table
		size_t x(0);//current index in transformed vector
		size_t y(0);//downshift amount
		for (; y < size(); y++)
		{
			int iDeadPrNum(0);
			int iLastDeadSeq(0);
			if (x > 0)
			{
				iLastDeadSeq = (int)at(x - 1).first + 1;
				iDeadPrNum = (int)at(x - 1).second;
			}

			if (at(y).first - iLastDeadSeq > y - iDeadPrNum)
			{
				at(x).first = at(y).first - 1;
				at(x).second = (int)y;
				x++;
			}
		}
		resize(x);
		//size_t iSize(mr.tellp());

		int iDeadPrNum(0);
		int iLastDeadStart(0);
		if (x > 0)
		{
			iLastDeadStart = (int)at(x - 1).first + 1;
			iDeadPrNum = (int)at(x - 1).second;
		}

		if (iSize - iLastDeadStart > y - iDeadPrNum)
			//if (x < iSize)//if the last indice is not dead, skew vector must conclude with it
			push_back(std::make_pair(iSize - 1, (int)y));
	}
	size_t transform(size_t i) const
	{
		SkewPair a(i, 0);
		assert(a.first >= 0);
		const_iterator lower = std::lower_bound(begin(), end(), a, SkewComparator());
		assert(lower != end());
		a.first -= (*lower).second;
		assert(a.first >= 0);
		return a.first + 1;//make it 1-based(!)
	}
	void clear(){ Base::clear(); }
	size_t size() const { return Base::size(); }
	const SkewPair &at(size_t i) const { return Base::at(i); }
	void add(size_t n){ push_back(std::make_pair(n, int(size()))); }
	bool empty() const { return Base::empty(); }
	size_t count() const//alive only
	{
		if (empty())
			return 0;
		const SkewPair &r(back());
		return r.first - r.second + 1;
	}
private:
	SkewPair &at(size_t i){ return Base::at(i); }
};


/////////////////////////////////////////////////////////////////////THelper
class skew_iterator
{
	const SkewVector &mr;
	size_t m_index;
	int m_skew_index;
public:
	skew_iterator(const SkewVector &r)
		: mr(r),
		m_index(0),
		m_skew_index(-1)
	{
		m_index += advance_skew_index();
	}
	size_t index() const { return m_index; }
	operator bool() const
	{
		return ((size_t)m_skew_index < mr.size());
	}
	int advance_skew_index()
	{
		int y_pr(0);
		assert(m_skew_index < (int)mr.size());
		if (!(m_skew_index < 0))
			y_pr = mr.at(m_skew_index).second;
		m_skew_index++;
		if (!((size_t)m_skew_index < mr.size()))
			return 0;//overflow
		int y(mr.at(m_skew_index).second);
		assert(y >= y_pr);
		int n_skip(y - y_pr);
		return n_skip;
	}
	skew_iterator& operator++()
	{
		assert((size_t)m_skew_index < mr.size());
		if (!(m_index++ < mr.at(m_skew_index).first))
		{
			int n_skip(advance_skew_index());
			m_index += n_skip;
		}
		return *this;
	}
};



/////////////////////////////////////////////////////////////////////TMemoryPool

template <typename T>
union TMemoryPoolEntry
{
	char	m[sizeof(T)];//data
	TMemoryPoolEntry<T>* pDeadNext;
	T* data(){ return (T*)&m; }
};

template <typename T>
class TMemoryPool : public My::TOStream<TMemoryPoolEntry<T> >
{
	typedef TMemoryPoolEntry<T> EltType;
	typedef My::TOStream<EltType> Base;
public:
	typedef T*	DataPtr;
	typedef const T* CDataPtr;
private:
	EltType* pDeadList;
public:
	TMemoryPool(unsigned chunkSize = 0)
		: Base(chunkSize),
		pDeadList(0)
	{
	}
	void reset(bool bClear = false)
	{
		this->TPut_Reset(bClear);
	}
	Base &base() { return *this; }
	T* allocate()
	{
		if (!pDeadList)
			return this->push()->data();
		EltType* pElt(pDeadList);
		pDeadList = pDeadList->pDeadNext;
		return pElt->data();
	}
	void deallocate(T *p)
	{
		assert(this->contains(p));
		EltType* tmp(pDeadList);
		pDeadList = (EltType*)p;
		pDeadList->pDeadNext = tmp;
	}

	int count_dead()
	{
		int i(0);
		for (EltType *p(pDeadList); p; p = p->pDeadNext)
			i++;
		return i;
	}

	void print_stats(std::ostream &os)
	{
		Base::print_stats(os);
		os << "dead = " << count_dead() << std::endl;
	}

	typedef char * BytePtr;


	/////////////////////////////////////////////////////////////////////THelper
	template <typename U>
	class THelper : public std::vector<std::pair<BytePtr, size_t> >
	{
		typedef typename TMemoryPool::Chunk	Chunk;
	public:
		typedef TMemoryPool<U>	MyPool;
		typedef typename MyPool::DataPtr DataPtr;
		typedef typename MyPool::CDataPtr CDataPtr;
	private:
		MyPool &mr;
		SkewVector m_skew;
	public:
		THelper(MyPool &r, unsigned flags = 0)//1:loading,2-no skew
			: mr(r)
		{
			reset(flags);
		}
		void reset(unsigned flags = 0)
		{
			clear();

			//create an eltement's index lookup table based on chunk ptr
			size_t index(0);
			Chunk *pChunk = mr.Head();
			if (pChunk)
			{
				do {
					this->push_back(std::make_pair((BytePtr)pChunk->Data(), index));
					pChunk = pChunk->Next();
					index++;
				} while (pChunk != mr.Head());
			}
			//if (!bLoading)
			if (!(flags & 1))
				std::sort(begin(), end());

			m_skew.clear();
			if (!(flags & 2))
			{
				// prepare an ordered list of dead indices
				for (EltType *p(mr.pDeadList); p; p = p->pDeadNext)
				{
					size_t n(toIndex(p->data()));
					assert(!(n < 0));
					m_skew.add(n);
				}
				m_skew.preprocess(mr.tellp());
			}
		}

		MyPool &pool() const { return mr; }

		const SkewVector &skew() const { return m_skew; }

		size_t count() const//alive only
		{
			return m_skew.count();
		}
		bool isEmpty() const
		{
			return count() == 0;
		}
		struct ChunkPtrComparator {
			/*bool operator()(const std::pair<BytePtr, size_t>& a, const std::pair<BytePtr, size_t>& b) const {
				return a.first < b.first;
			}*/
			bool operator()(BytePtr a, const std::pair<BytePtr, size_t>& b) const {
				return a < b.first;
			}
		};

		size_t toIndex(CDataPtr pT) const
		{
			//std::pair<BytePtr, size_t> a((BytePtr)pT, 0);
			const_iterator upper = std::upper_bound(begin(), end(), (BytePtr)pT, ChunkPtrComparator());
			if (upper == begin())
				return -1;//not in range
			--upper;
			std::ptrdiff_t offs((char *)pT - (char *)(*upper).first);
			std::ptrdiff_t offs_max(mr.chunkSize()*sizeof(EltType));
			if (offs >= offs_max)
				return -1;//not in range
			assert(!(offs % sizeof(EltType)));//at elt boundary
			size_t i(offs / sizeof(EltType));
			return (*upper).second*mr.chunkSize() + i;
		}

		DataPtr fromSqueezedIndex(size_t i) const//for load only!
		{
			//?assert(m_skew.empty());
			if (!i)
				return 0;
			i--;//revert it to 0-based(!)
			size_t index(i / mr.chunkSize());
			size_t offset(i % mr.chunkSize());
			assert(index < count());
			Chunk *pChunk((Chunk *)(this->at(index).first - Chunk::dataOffset()));//chunks must be in list order!
			EltType &rElt(pChunk->at(offset));
			return rElt.data();
		}

		size_t toSqueezedIndex(CDataPtr pT) const
		{
			if (!pT)
				return 0;
			return m_skew.transform(toIndex(pT));
		}

		template <typename V>
		class TIterator : protected skew_iterator
		{
			My::TOStreamReader<TMemoryPoolEntry<V> >	m_get;
		public:
			TIterator(THelper<V> &r)
				: skew_iterator(r.skew()),
				m_get(r.pool().base())
			{
				skip(index());
			}
			operator bool() const
			{
				return skew_iterator::operator bool();
			}
			void skip(size_t n_skip)
			{
				if (m_get.SkipGet((unsigned)n_skip) != n_skip)
				{
					assert(0);
				}
			}

			TIterator& operator ++()
			{
				size_t i(index());
				skew_iterator::operator++();
				skip(index() - i);
				return *this;
			}
			TIterator& operator ++(int){ return operator++(); }
			const V& operator*() const { return *m_get.data()->data(); }//?
			V& operator*() { return *m_get.data()->data(); }
			V *data(){ return m_get.data()->data(); }
		};
		typedef TIterator<U> Iterator;
	};

	typedef THelper<T>	Helper;
	typedef typename THelper<T>::Iterator EltIterator;
	typedef typename THelper<const T>::Iterator EltConstIterator;
	
};







/////////////////////////////////

namespace mem
{

typedef unsigned int MemRef;

template <typename T>
class HMEM
{
	MemRef	m_h;
	T *m_ptr;
public:
	HMEM() : m_h(0), m_ptr(0) {}
	HMEM(MemRef h, T *ptr) : m_h(h), m_ptr(ptr) {
		assert(m_h != 1);
	}
	HMEM(const HMEM &o)
		: m_h(o.m_h),
		m_ptr(o.m_ptr)
	{
	}
	void operator=(const HMEM &o)
	{
		m_h = o.m_h; 
		m_ptr = o.m_ptr;
	}
	//HMEM(T *p) : m_h(p), m_ptr(p){}
	T* operator->() const { return m_ptr; }
	T &operator*() const { return *m_ptr; }
	operator MemRef(){ return m_h; }
	const MemRef & memRef() const { return m_h; }
	operator void *(){ return (void *)m_h;  }
	operator bool() const { return m_ptr != nullptr; }
	//operator const T*() const { return m_ptr; }
	//operator T*() { return m_ptr; }
	bool operator==(const HMEM &o) const { return m_ptr == o.m_ptr; }
	bool operator!=(const HMEM &o) const { return !operator==(o); }
	bool operator<(const HMEM &o) const { return m_ptr < o.m_ptr; }
};


/*union MemRef
{
	unsigned int pos;
	struct {
		unsigned short chunk;
		unsigned short entry;
	};
	MemRef() : pos(0){}
	MemRef(unsigned int i) : pos(i){}
	MemRef(unsigned short c, unsigned short e): chunk(c), entry(e){}
	bool operator==(const MemRef&o) const { return pos == o.pos; }
};*/

static const unsigned ENTRY_GRADE_DEFAULT = 6;//64
static const unsigned CHUNK_GRADE_DEFAULT = 6;//64

template <typename T, int TEntrySize = ENTRY_GRADE_DEFAULT> union TEntry;

template <typename T, int TEntrySize = ENTRY_GRADE_DEFAULT>
struct TDeadPtr
{
	typedef TEntry<T, TEntrySize> EltType;
	EltType* rawPtr;
	MemRef hRef;
	//TDeadPtr() : rawPtr(0){}	//C2620
	TDeadPtr next() const
	{
		if (!rawPtr)
			return TDeadPtr();
		return rawPtr->iNextDead;
	}
	operator bool() const { return rawPtr != 0; }
};

template <typename T, int TEntrySize>
union TEntry
{
	char	m[1 << TEntrySize];//data
	TDeadPtr<T, TEntrySize> iNextDead;

	T* data(){ return (T*)&m; }
};

template <typename T, int TChunkGrade = CHUNK_GRADE_DEFAULT, int TEntryGrade = ENTRY_GRADE_DEFAULT>
class TChunk
{
public:
	typedef TEntry<T, TEntryGrade> EltType;
	EltType	mData[1 << TChunkGrade];
public:
	TChunk() {}
	EltType &operator[](size_t i) { return *(EltType *)(mData->m + (i << TEntryGrade)); }//mData[i];
	const EltType &operator[](size_t i) const { return *(EltType *)(mData->m + (i << TEntryGrade)); }//mData[i];
};

template <typename T, int TChunkSize = CHUNK_GRADE_DEFAULT, int TEntrySize = ENTRY_GRADE_DEFAULT>
class TPool : std::vector<TChunk<T, TChunkSize, TEntrySize>*>
{
	typedef TPool<T, TChunkSize, TEntrySize>	PoolType;
	typedef T	DataType;
	typedef TEntry<T, TEntrySize> EltType;
	typedef TChunk<T, TChunkSize, TEntrySize>	ChunkType;
	typedef std::vector<ChunkType *> Base;
	typedef TDeadPtr<T,TEntrySize> DeadType;
private:
	using Base::back;
	using Base::push_back;
	using Base::resize;
	using Base::size;
	using Base::at;
public:
	typedef MemRef DataPtr;
	typedef const MemRef CDataPtr;
	static const unsigned CHUNKGRADE = TChunkSize;
	static const unsigned ENTRYGRADE = TEntrySize;
	static const unsigned CHUNKMASK = ~(((unsigned)-1) << CHUNKGRADE);//used to extract reminder of power of 2 division
	static const unsigned ENTRYMASK = ~(((unsigned)-1) << ENTRYGRADE);

	static unsigned CHUNKID(MemRef pos) { return ((unsigned)(pos >> CHUNKGRADE)); }
	static unsigned ENTRYID(MemRef pos) { return ((unsigned)(pos & CHUNKMASK)); }
	static MemRef MEMREF(unsigned chunk, unsigned entry) { return MemRef((chunk << CHUNKGRADE) | entry); }

private:
	DeadType iDeadList;
	size_t	nTotal;//number of elements in the last chunk
public:
	TPool()//unsigned chunkSize = 0)
		: nTotal(1 << TChunkSize)//to trigger first alloc
	{
		//const size_t zz = EltType::zz;
		iDeadList.rawPtr = 0;
		push_back(nullptr);//NIL
	}

	~TPool()
	{
		for (size_t i(1); i < size(); i++)
			delete at(i);
	}

	void clear()
	{
		for (size_t i(1); i < size(); i++)//preseve to reuse?
			delete at(i);
		Base::clear();
		iDeadList.rawPtr = 0;
		push_back(nullptr);//NIL
		nTotal = 1 << TChunkSize;//to trigger first alloc
	}

	T* get(MemRef h) const 
	{
		ChunkType* pChunk = (*this)[CHUNKID(h)];
		EltType &rEntry = (*pChunk)[ENTRYID(h)];
		return rEntry.data();
	}

	static const unsigned chunkSize(){ return 1 << TChunkSize; }
	static const unsigned entrySize(){ return 1 << TEntrySize; }
	size_t tellp() const//returns nummber of elements
	{
		return (long(size()) - 2)*size_t(chunkSize()) + nTotal;
	}

	HMEM<DataType> New()
	{
		MemRef hRef;
		EltType *p(allocate(hRef));
		new(p) T();
		return HMEM<DataType>(hRef, p->data());
	}


	template <typename U>
	HMEM<DataType> New(U arg)
	{
		MemRef hRef;
		EltType *p(allocate(hRef));
		new(p) T(arg);
		return HMEM<DataType>(hRef, p->data());
	}

	void Delete(const MemRef &hRef)
	{
		if (hRef)
		{
			EltType *pElt(toElt(hRef));
			pElt->data()->~T();
#if(1)
			memset(pElt->data(), 0xC, sizeof(EltType));
#endif
			deallocate(hRef);
		}
	}

	T *allocate()
	{
		MemRef hRef;
		EltType *pElt(allocate(hRef));
		return pElt->data();
	}

	TPool &pool(){ return *this; }
	void print_stats(std::ostream &os){}

	static size_t toIndex(DataPtr pT)
	{
		if (!pT)
			return 0;
		size_t i(CHUNKID(pT) - 1);
		//i *= mr.chunkSize();
		i <<= CHUNKGRADE;
		return i + ENTRYID(pT);
	}

	static DataPtr fromIndex(size_t i)
	{
		size_t chunk(i / chunkSize());
		//unsigned chunk(unsigned(i) >> CHUNKGRADE);
		size_t entry(i % chunkSize());
		//unsigned entry(unsigned(i) & CHUNKMASK);
		return MEMREF(unsigned(chunk) + 1, unsigned(entry));
	}

private:
	EltType *allocate(MemRef &hRef)
	{
		if (!iDeadList)
		{
			if (!(nTotal < chunkSize()))
			{
				push_back(new ChunkType());
				nTotal = 0;
			}
			++nTotal;
			hRef = MEMREF((unsigned short)size() - 1, (unsigned short)nTotal - 1);
			ChunkType &rChunk(*back());
			return &(rChunk[nTotal - 1]);
		}
		DeadType e(iDeadList);
		iDeadList = iDeadList.next();
		hRef = e.hRef;
		return e.rawPtr;
	}

	void deallocate(const MemRef &hRef)
	{
		DeadType tmp(iDeadList);
		EltType *pElt(toElt(hRef));
		//iDeadList = pElt;
		iDeadList.rawPtr = pElt;
		iDeadList.hRef = hRef;
		pElt->iNextDead = tmp;
	}

	EltType *toElt(MemRef hRef) const
	{
		ChunkType &rChunk(*at(CHUNKID(hRef)));
		return &rChunk[ENTRYID(hRef)];
	}

public:
	template <typename U>
	class THelper
	{
		typedef	PoolType	MyPool;
		typedef	typename MyPool::DataPtr	DataPtr;
		typedef	typename MyPool::CDataPtr	CDataPtr;
		typedef	typename MyPool::DeadType	DeadType;
		typedef THelper<U>	HelperType;
		MyPool &mr;
		SkewVector m_skew;
	public:
		THelper(MyPool &r, unsigned flags = 0)//1:loading,2-no skew
			: mr(r)
		{
			reset(flags);
		}
		size_t count() const//alive only
		{
			return m_skew.count();
		}
		bool isEmpty() const
		{
			return count() == 0;
		}
		MyPool &pool(){ return mr; }
		const SkewVector &skew() const { return m_skew; }
		void reset(unsigned flags)
		{
			m_skew.clear();
			if (!(flags & 2))
			{
				// prepare an ordered list of dead indices
				for (DeadType p(mr.iDeadList); p; p = p.next())
				{
					size_t n(toIndex(p.hRef));
					assert(!(n < 0));
					m_skew.add(n);
				}
				m_skew.preprocess(mr.tellp());
			}
		}
		size_t toSqueezedIndex(CDataPtr h) const
		{
			if (!h)
				return 0;
			return m_skew.transform(toIndex(h));
		}
		HMEM<DataType> fromSqueezedIndex(size_t i) const
		{
			if (!i)
				return HMEM<DataType>();
			i--;//revert it to 0-based(!)

			//size_t chunk(i / mr.chunkSize());
			unsigned chunk(unsigned(i) >> CHUNKGRADE);

			//size_t entry(i % mr.chunkSize());
			unsigned entry(unsigned(i) & CHUNKMASK);

			MemRef h(MEMREF(chunk + 1, entry));
			return HMEM<DataType>(h, mr.get(h));
		}
	public://iterator
		template <typename V>
		class TIterator : protected skew_iterator
		{
			PoolType	&m_pool;
			DataPtr		m_cur;
		public:
			TIterator(THelper<V> &r)
				: skew_iterator(r.skew()),
				m_pool(r.pool()),
				m_cur(MEMREF(1, 0))//first
			{
				skip(index());
			}
			operator bool() const
			{
				return skew_iterator::operator bool();
			}
			void skip(size_t n_skip)
			{
				if (n_skip == 0)
					return;
				unsigned chunk(CHUNKID(m_cur) - 1);
				unsigned entry(ENTRYID(m_cur));
				entry += (unsigned)n_skip;
				while (!(entry < m_pool.chunkSize()))
				{
					chunk++;
					entry -= m_pool.chunkSize();
				}
				m_cur = MEMREF(chunk + 1, entry);
			}

			TIterator& operator ++()
			{
				size_t i(index());
				skew_iterator::operator++();
				skip(index() - i);
				return *this;
			}
			TIterator& operator ++(int){ return operator++(); }
			const V& operator*() const { return *m_pool.get(m_cur); }
			V *operator*() { return *m_pool.get(m_cur); }
			HMEM<V> data() { return HMEM<V>(m_cur, m_pool.get(m_cur)); }
		};
		typedef TIterator<U> Iterator;
	};
	typedef THelper<T>	Helper;
	typedef typename THelper<T>::Iterator EltIterator;
	typedef typename THelper<const T>::Iterator EltConstIterator;

public:
	template <typename U>
	class TSurrogate : public std::vector<std::vector<U> >
	{
	public:
		TSurrogate(){}
		U &cell(MemRef h)
		{
			unsigned chunk(CHUNKID(h));
			unsigned entry(ENTRYID(h));
			if (!(chunk < size()))
				resize(chunk + 1);
			std::vector<U> &a(at(chunk));
			if (a.empty())
				a.resize(chunkSize());
			assert(entry < chunkSize());
			return a[entry];
		}
	};

};

}//namespace mem


