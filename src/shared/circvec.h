#pragma once

#include <assert.h>
#include <cstddef>
#include <utility>

template <typename T>
class CircularVector_t
{
	T*	mpBuf;
	unsigned	mCapacityPow;
	size_t	mHead;
	size_t	mTail;

	//std::list<T>	m;
public:
	CircularVector_t()//later!unsigned _capacityPow = 0)
		: mpBuf(nullptr),
		mCapacityPow(0),
		mHead(0),
		mTail(0)//mark as full (and empty) - will trigger expand
	{
		//if (_capacityPow > 0)
		{
			//assert(mCapacityPow < 32);//4G
//			mpBuf = new T[capacity()];
//			mTail = capacity();
		}
	}
	~CircularVector_t()
	{
		delete [] mpBuf;
	}
	void cleanup()
	{
		delete [] mpBuf;
		mpBuf = nullptr;
		mCapacityPow = 0;
		mHead = mTail = 0;
	}
	size_t capacity() const
	{
		if (mpBuf)
			return size_t(1) << mCapacityPow;
		return 0;
	}
	//push
	void push_back(T t) {
		if (mTail == mHead)
			expand();
		if (mTail == capacity())
			mTail = 0;//from being empty
		mpBuf[mTail++] = t;
		if (mTail == capacity())
			mTail = 0;
//m.push_back(t);
	}
	void push_front(T t) {
		if (mHead == mTail)
			expand();
		if (mHead == 0)
			mHead = capacity();
		mpBuf[--mHead] = t;
//m.push_front(t);
	}
	//pop
	void pop_back() {
//assert(m.back() == back());
		if (mTail == 0)
			mTail = capacity();
		--mTail;
		if (mTail == mHead)
		{
			mHead = 0;//mark empty
			mTail = capacity();
		}
//m.pop_back();
	}
	void pop_front(){
//assert(m.front() == front());
//#ifdef _DEBUG
//		mpBuf[mHead] = 0;//for debug
//#endif
		mHead++;
		if (mHead == capacity())
			mHead = 0;
		if (mTail == mHead)
		{
			mHead = 0;//mark empty
			mTail = capacity();
		}
//m.pop_front();
	}
	//back/front
	T& back0(){
		if (mTail == 0)
			return mpBuf[capacity() - 1];
		return mpBuf[mTail - 1];
	}
	T& back(){
//		assert(m.back() == back0());
		return back0();
	}

	T& front0() {
		assert(mHead != capacity());
		return mpBuf[mHead];
	}
	T& front() {
		//assert(m.front() == front0());
		return front0();
	}
	size_t size0() const {
		if (empty())
			return 0;
		if (mTail > mHead)
			return mTail - mHead;
		//if (mTail < mHead)
			return (capacity() - mHead) + mTail;
		//return cpacity();
	}
	size_t size() const {
		//assert(m.size() == size0());
		return size0();
	}
	bool empty0() const
	{
		if (mTail < capacity())
			return false;
		assert(mHead == 0);
		return true;
	}
	bool empty() const
	{
		//assert(m.empty() == empty0());
		return empty0();
	}
	T& operator[](size_t index)
	{
		return at(index);
	}
	const T& operator[](size_t index) const
	{
		return at(index);
	}
	///////////////////////////////////iterator_t
	//template <typename T>
	class iterator_t
	{
		CircularVector_t&	mr;
		size_t	mi;
	public:
		iterator_t(CircularVector_t &r, size_t i)
			: mr(r),
			mi(i)
		{
		}
		iterator_t(const iterator_t &o)
			: mr(o.mr),
			mi(o.mi)
		{
		}
		//compare
		bool operator==(const iterator_t &o) const {
			return mi == o.mi;	}
		bool operator!=(const iterator_t &o) const {
			return !operator==(o); }
		//assign
		void operator=(const iterator_t &o){
			mi = o.mi; }
		operator bool() const {
			return mi != mr.mTail; }
		//deref
		const T& operator*() const {
			assert(mi != mr.capacity());
			return mr.mpBuf[mi]; }
		T& operator*() {
			assert(mi != mr.capacity());
			return mr.mpBuf[mi]; }
		//pre
		iterator_t& operator++() {
			if (++mi == mr.capacity())
				mi = 0;
			return *this;
		}
		iterator_t& operator--() {
			if (mi == 0)
				mi = mr.capacity();
			--mi;
			return *this;
		}
		//post
		iterator_t& operator ++(int){ return operator++(); }
		iterator_t& operator --(int){ return operator--(); }
	};
	typedef	iterator_t	iterator;
	iterator begin()
	{
		return iterator(*this, mHead);
	}
	iterator end()
	{
		return iterator(*this, mTail);
	}
private:
	void expand() {
		assert(mTail == mHead);
		if (mpBuf)
		{
			unsigned newCapacity(mCapacityPow + 1);
			T* p(new T[size_t(size_t(1) << newCapacity)]);
			for (size_t i(0); i < mTail; i++)
				p[i] = std::move(mpBuf[i]);
				//memcpy(p, mpBuf, mTail * sizeof(T));
			size_t headChunk(capacity() - mHead);
			size_t newHead((size_t(1) << newCapacity) - headChunk);
			for (size_t i(0); i < headChunk; i++)
				p[newHead + i] = std::move(mpBuf[mHead + i]);
				//memcpy(p + newHead, mpBuf + mHead, headChunk * sizeof(T));
			mCapacityPow = newCapacity;
			mHead = newHead;
			delete [] mpBuf;
			mpBuf = p;
		}
		else
		{
			size_t newCapacity(capacity());
			if (newCapacity == 0)
				newCapacity = 1;
			mpBuf = new T[newCapacity];
			mTail = newCapacity;
			assert(mHead == 0);
		}
	}
	T& at(size_t index)
	{
		if (mTail > mHead)
			return mpBuf[mHead + index];
		size_t leadChunk(capacity() - mHead);
		if (index < leadChunk)
			return mpBuf[mHead + index];
		return mpBuf[index - leadChunk];
	}
};

