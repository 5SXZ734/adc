#pragma once

#include "MyStreamBase.h"

#include <string>
#include <iostream>
#include <assert.h>
#include <memory.h>//memcpy
#include <cstddef>//offsetof
#include <stdio.h>//FILE
#include <stdlib.h>//atoi,atof

namespace My
{

#ifdef read
#undef read
#endif
#ifdef write
#undef write
#endif

#define CHUNKSIZE	64

	template <typename T>
	class TChunk
	{
	public:
		TChunk	*mpNext;
		T	mData[1];
	public:
		TChunk() : mpNext(0){ mpNext = this; }
		~TChunk()
		{
		}
		T *Data() { return mData; }
		T &at(size_t i){ return mData[i]; }
		TChunk *Next() const { return mpNext; }
		static TChunk *newChunk(int chunkSize)
		{
			//int z = offsetof(TChunk, mData);
			return (TChunk *)new char[offsetof(TChunk, mData) + chunkSize*sizeof(T)];
		}
		static long dataOffset(){ return offsetof(TChunk, mData); }
	};


	template <typename T>
	class TChunkList
	{
		typedef TChunk<T>	Chunk;
		Chunk *mpTail;
	public:
		TChunkList()
			: mpTail(0)
		{
		}
		int count() const
		{
			int i(0);
			Chunk *p(tail());
			if (p)
				do {
					++i;
					p = p->Next();
				} while (p != tail());
				return i;
		}
		void init(Chunk *pChunk)
		{
			//assert(!mpTail);
			mpTail = pChunk;
		}
		Chunk *link(Chunk *pChunk)
		{
			if (mpTail)
			{
				pChunk->mpNext = mpTail->mpNext;
				mpTail->mpNext = pChunk;
			}
			else
				pChunk->mpNext = pChunk;
			mpTail = pChunk;
			return mpTail;
		}
		Chunk *tail() const{ return mpTail; }
		Chunk *head() const
		{
			if (mpTail)
				return mpTail->mpNext;
			return 0;
		}
		bool empty() const { return !mpTail; }
		Chunk *take()//from head
		{
			if (mpTail)
			{
				Chunk *p(mpTail->mpNext);
				if (p != mpTail)
					mpTail->mpNext = p->mpNext;
				else
					mpTail = 0;
				p->mpNext = 0;
				return p;
			}
			return 0;
		}
		void replicate(TChunkList &o, int chunkSize) const
		{
			Chunk *pChunkHead(head());
			Chunk *pChunk(pChunkHead);
			Chunk *pChunkNew(0);
			if (pChunk)
			{
				do {
					pChunkNew = o.link(Chunk::newChunk(chunkSize));
					memcpy(&pChunkNew->mData, &pChunk->mData, chunkSize*sizeof(T));
					pChunk = pChunk->mpNext;
				} while (pChunk != pChunkHead);
			}
		}
	};




	template <typename T>
	struct TPtr
	{
		typedef TChunk<T>	Chunk;
		typedef TChunkList<T> ChunkList;

		ChunkList	mChunks;
		unsigned	mOffset;
		TPtr() : mOffset(0){}
		TPtr(const TPtr<T> &o)
			: mChunks(o.mChunks),
			mOffset(o.mOffset)
		{
		}
		TPtr<T> &operator=(const TPtr<T> &o)
		{
			mChunks = o.mChunks;
			mOffset = o.mOffset;
			return *this;
		}
		void Init(Chunk *p){ mChunks.init(p); mOffset = 0; }
		unsigned Pos() const { return mOffset; }
		Chunk *GetChunk() const { return mChunks.tail(); }
		Chunk *Tail() const { return mChunks.tail(); }
		Chunk *Head() const { return mChunks.head(); }
		bool IsNull() const { return mChunks.empty(); }
		bool IsEmpty() const
		{
			if (!mChunks.empty())
			{
				if (mChunks.tail() != mChunks.head())
					return false;
				if (mOffset != 0)
					return false;
			}
			return true;
		}
		void Clear(){ mChunks.init(0); mOffset = 0; }
		unsigned Write(T *src, unsigned len, unsigned chunkSize)
		{
			assert(!mChunks.empty());
			assert(mOffset < chunkSize);
			unsigned sz(chunkSize - mOffset);
			if (len < sz)
				sz = len;
			T *dst = &mChunks.tail()->mData[mOffset];
			memcpy(dst, src, sz*sizeof(T));
			mOffset += sz;
			return sz;
		}

		T* Push(unsigned chunkSize)
		{
			assert(!mChunks.empty());
			assert(mOffset < chunkSize);
			T *dst = &mChunks.tail()->mData[mOffset++];
			return dst;
		}
	};




	template <typename T>
	class TOStream : public TPtr<T>
	{
		typedef TPtr<T>	Base;
	public:
		typedef TChunk<T>	Chunk;
		typedef TChunkList<T> ChunkList;
		typedef TPtr<T>	StreamPos;

		ChunkList	mDeadChunks;
		unsigned	mChunkSize;

	protected:
		using Base::mOffset;
		using Base::mChunks;

		Chunk *Expand()
		{
			Chunk *pChunk = takeDeadChunk();
			if (!pChunk)
			{
				pChunk = Chunk::newChunk(chunkSize());
			}
			linkChunk(pChunk);
			mOffset = 0;
			return mChunks.tail();
		}

		void TPut_Reset(bool bClear)
		{
			while (Chunk *pChunk = takeChunk())
			{
				if (bClear)
					delete[](char *)pChunk;
				else
					linkDeadChunk(pChunk);
			}
			mOffset = 0;
		}

		Chunk *takeChunk(){ return mChunks.take(); }
		Chunk *takeDeadChunk(){ return mDeadChunks.take(); }
		void linkChunk(Chunk *pChunk){ mChunks.link(pChunk); }
		void linkDeadChunk(Chunk *pChunk){ mDeadChunks.link(pChunk); }

	public:
		explicit TOStream(unsigned chunkSize = 0)
			: mChunkSize(chunkSize ? chunkSize : CHUNKSIZE)
		{
		}

		TOStream(T *p, unsigned sz, unsigned chunkSize)
			: mChunkSize(chunkSize ? chunkSize : CHUNKSIZE)
		{
			write(p, sz);
		}

		TOStream(const TOStream &ss)
			: TPtr<T>(ss),
			mChunkSize(ss.mChunkSize)
		{
			ss.mChunks.replicate(mChunks, mChunkSize);
			ss.mDeadChunks.replicate(mDeadChunks, mChunkSize);
		}

		~TOStream()
		{
			Chunk *p;
			while ((p = takeDeadChunk()) != 0)
				delete p;
			while ((p = takeChunk()) != 0)
				delete [] (char *)p;
		}

		const StreamPos &pos() const { return reinterpret_cast<const StreamPos &>(*this); }
		//StreamPos seek(const StreamPos &o){ StreamPos t(mPut); mPut = o; return t; }

		unsigned chunkSize() const { return mChunkSize; }
		unsigned chunkSize0() const { return Chunk::dataOffset() + mChunkSize * sizeof(T); }

		unsigned write(T *p, unsigned len)
		{
			if (!p || !len)
				return 0;

			len *= sizeof(T);

			unsigned bytes = 0;
			for (;;)
			{
				if (this->IsNull() || !(mOffset < chunkSize()))
				{
					Chunk *pChunk = Expand();
					if (!pChunk)
						break;
				}

				unsigned sz(this->Write(p, len, chunkSize()));
				if (!sz)
					break;

				bytes += sz;
				if (sz >= len)
					break;

				len -= sz;
				p = ((char *)p) + sz;
			}

			return bytes;
		}

		int tellp() const
		{
			int p = this->Pos();
			for (Chunk *pChunk(this->Head()); pChunk != this->Tail(); pChunk = pChunk->Next())
				p += chunkSize();
			return p;
		}

		int chunks_count() const { return mChunks.count(); }

		T *push()
		{
			if (this->IsNull() || !(mOffset < chunkSize()))
			{
				Chunk *pChunk = Expand();
				if (!pChunk)
					return 0;
			}
			return this->Push(chunkSize());
		}

		bool contains(void *p)
		{
			Chunk *pChunk(this->Head());
			if (pChunk)
				do {
					if (pChunk->Data() <= p && p < (pChunk->Data() + mChunkSize * sizeof(T)))
						return true;
					pChunk = pChunk->Next();
				} while (pChunk != this->Head());
				return false;
		}

		void print_stats(std::ostream &os)
		{
			os << "chunk_size = " << chunkSize() << "(" << chunkSize0() << ")" << std::endl;
			unsigned p(tellp());
			unsigned n(p / chunkSize());
			unsigned o(p % chunkSize());
			unsigned n2(o > 0 ? n + 1 : n);
			os << "chunks_num = " << n2 << std::endl;
			os << "capacity = " << n2 * chunkSize() << std::endl;
			os << "current = " << n << "[" << o << "]" << std::endl;
		}
	};









	template <typename T>
	class TOStreamReader : public TPtr<T>
	{
		typedef TPtr<T> Base;
		typedef TChunk<T>	Chunk;
		typedef TChunkList<T> ChunkList;
		const TOStream<T> &mr;
	public:
		typedef Base	StreamPos;
		using Base::mChunks;
		using Base::mOffset;

		TOStreamReader(const TOStream<T> &r)
			: mr(r)
		{
		}

		TOStreamReader(const TOStream<T> &r, const TPtr<T> &rPos)
			: TPtr<T>(rPos),
			mr(r)
		{
		}

		TOStreamReader(const TOStreamReader<T> &o) : Base(o), mr(o.mr){ assert(0); }
		TOStreamReader<T> &operator=(const TOStreamReader<T> &o){ assert(0); return *this; }
		Base seek(const Base &o){ Base t(*this); Base::operator=(o); return t; }

		T *data()
		{
			if (this->IsNull())
				this->Init(mr.Head());
			else if (IsGetOverflow())
			{
				Chunk *pChunk(NextGetChunk());
				if (!pChunk)
					return 0;

				this->Init(pChunk);
			}

			if (mChunks.empty())
				return 0;

			return &mChunks.tail()->mData[mOffset];
		}

		T *data(unsigned &blockSize)
		{
			if (this->IsNull())
				this->Init(mr.Head());
			else if (IsGetOverflow())
			{
				Chunk *pChunk(NextGetChunk());
				if (!pChunk)
					return 0;

				this->Init(pChunk);
			}

			if (mChunks.empty())
				return 0;

			unsigned upperLimit = mr.chunkSize();

			if (mChunks.tail() == mr.Tail())
				if (mr.Pos() < upperLimit)
					upperLimit = mr.Pos();

			blockSize = upperLimit - mOffset;
			return &mChunks.tail()->mData[mOffset];
		}

		Chunk *NextGetChunk() const
		{
			if (!mChunks.empty() && mChunks.tail() != mr.Tail())
				return mChunks.tail()->Next();
			return 0;
		}

		bool IsGetOverflow() const
		{
			return (mChunks.empty() || !(mOffset < mr.chunkSize()));
		}

		char peek() const
		{
			if (!mChunks.empty())
			{
				if (mChunks.tail() == mr.GetChunk())
				{
					unsigned limit = mr.chunkSize();
					if (mr.Pos() < limit)
					{
						limit = mr.Pos();
						if (mOffset < limit)
							return mChunks.tail()->mData[mOffset];
					}
				}
				else
				{
					if (mOffset < mr.chunkSize())
						return mChunks.tail()->mData[mOffset];
					if (mChunks.tail()->mpNext)
						return mChunks.tail()->mpNext->mData[0];
				}
			}
			return 0;
		}

		unsigned read(T *pDst, unsigned bufSize)
		{
			if (!pDst || !bufSize)
				return 0;

			if (this->IsNull())
			{
				this->Init(mr.Head());
				if (this->IsNull())
					return 0;
			}

			unsigned left = bufSize;
			for (;;)
			{
				if (IsGetOverflow())
				{
					Chunk *pChunk(NextGetChunk());
					if (!pChunk)
						break;
					this->Init(pChunk);
				}

				if (mChunks.empty())
					break;

				unsigned sz(TGet_read(pDst, left));
				if (!sz)
					break;

				left -= sz;
				if (left == 0)
					break;

				pDst = ((char*)pDst) + sz;
			}

			return (bufSize - left);
		}

		unsigned write(T *pDst, unsigned bufSize)
		{
			if (!pDst || !bufSize)
				return 0;

			if (this->IsNull())
			{
				this->Init(mr.Head());
				if (this->IsNull())
					return 0;
			}

			unsigned left = bufSize;
			for (;;)
			{
				if (IsGetOverflow())
				{
					Chunk *pChunk(NextGetChunk());
					if (!pChunk)
						break;
					this->Init(pChunk);
				}

				if (mChunks.empty())
					break;

				unsigned sz(this->Write(pDst, left, mr.chunkSize()));
				if (!sz)
					break;

				left -= sz;
				if (left == 0)
					break;

				pDst = ((char*)pDst) + sz;
			}

			return (bufSize - left);
		}

		unsigned TGet_read(void *pDst, unsigned len)
		{
			assert(!mChunks.empty());
			assert(mOffset < mr.chunkSize());
			unsigned upperLimit = mr.chunkSize();
			if (mChunks.tail() == mr.GetChunk())
				if (mr.Pos() < upperLimit)
					upperLimit = mr.Pos();

			if (mOffset < upperLimit)
			{
				unsigned len2 = upperLimit - mOffset;
				if (len2 < len)
					len = len2;
				void *src = &mChunks.tail()->mData[mOffset];
				memcpy(pDst, src, len);
				mOffset += len;
				return len;
			}
			return 0;
		}

		bool empty() const
		{
			if (!mr.IsNull())
			{
				if (this->IsNull())
				{
					if (mr.Pos() > 0)
						return false;
				}
				else
				{
					if (this->GetChunk() != mr.GetChunk())
						return false;
					if (this->Pos() != mr.Pos())
					{
						assert(this->Pos() < mr.Pos());
						return false;
					}
				}
			}
			return true;
		}

		unsigned rewind(unsigned p)
		{
			Chunk *pChunk = mr.Head();
			if (!pChunk)
				return (unsigned)-1;

			unsigned chunks = 0;
			while (pChunk != mr.Tail())
			{
				unsigned sz(mr.chunkSize());
				if (p < sz)
					break;
				p -= sz;
				chunks += sz;
				pChunk = pChunk->Next();
			}

			this->Init(pChunk);
			if (pChunk == mr.Tail())
				if (p > mr.Pos())
					p = mr.Pos();

			mOffset = p;
			return chunks + p;
		}

		int tellg() const
		{
			if (this->IsNull())
				return 0;
			int p = this->Pos();
			for (Chunk *pChunk = mr.Head(); pChunk != this->GetChunk(); pChunk = pChunk->Next())
				p += mr.chunkSize();
			return p;
		}

		unsigned SkipGet(unsigned sz0)
		{
			if (!sz0)
				return 0;

			if (this->IsNull())
			{
				this->Init(mr.Head());
				if (this->IsNull())
					return 0;
			}

			unsigned left = sz0;
			for (;;)
			{
				if (IsGetOverflow())
				{
					Chunk *pChunk(NextGetChunk());
					if (!pChunk)
						break;
					this->Init(pChunk);
				}

				if (mChunks.empty())
					break;

				unsigned bytes = SkipGetChunk(left);
				if (!bytes)
					break;

				left -= bytes;
				if (left == 0)
					break;
			}

			return sz0 - left;
		}

		unsigned SkipGetChunk(unsigned len)
		{
			assert(!mChunks.empty());
			assert(mOffset < mr.chunkSize());
			unsigned limit = mr.chunkSize();
			if (mChunks.tail() == mr.GetChunk())
				if (mr.Pos() < limit)
					limit = mr.Pos();
			if (limit > mOffset)
			{
				unsigned len2 = limit - mOffset;
				if (len2 < len)
					len = len2;
				mOffset += len;
				return len;
			}
			return 0;
		}
	};




	/////////////////////////////////////////////////////
	// TIOStream

	template <typename T>
	class TIOStream : public TOStream<T>
	{
		typedef TOStream<T>	Base;
	protected:
		typedef TChunk<T>	Chunk;
		typedef TChunkList<T> ChunkList;

		TOStreamReader<T>	mGet;
	public:
		typedef typename Base::StreamPos	StreamPos;
		T *data(unsigned &blockSize){ return mGet.data(blockSize); }

		const StreamPos &posg() const { return mGet; }
		StreamPos seekg(const StreamPos &o){ mGet.seek(o); return mGet; }

		unsigned skip(unsigned len, int iReset)
		{
			unsigned bytes(mGet.SkipGet(len));
			if (iReset != 0)
			{
				if (mGet.IsGetOverflow())
				{
					Chunk *pChunk(mGet.NextGetChunk());
					if (!pChunk)
						return bytes;

					mGet.Init(pChunk);
				}

				if (!mGet.mChunks.empty())
				{
					while (this->Head() != mGet.mChunks.tail())
					{
						if (iReset < 0)
						{
							delete[](char *)this->takeChunk();
						}
						else
						{
							this->linkDeadChunk(this->takeChunk());
						}
					}
				}
			}
			return bytes;
		}

		unsigned read(T *pDst, unsigned bufSize){ return mGet.read(pDst, bufSize); }

		char peek() const { return mGet.peek(); }

		bool empty(){ return mGet.empty(); }

		void reset(bool bClear)
		{
			mGet.Clear();
			this->TPut_Reset(bClear);
		}

		unsigned rewind(unsigned p){ return mGet.rewind(p); }

		int tellg() const { return mGet.tellg(); }

	protected:
		explicit TIOStream(unsigned chunkSize = 0)
			: TOStream<T>(chunkSize),
			mGet(base())
		{
		}

		TIOStream(const TIOStream &ss)
			: TOStream<T>(ss),
			mGet(ss.mGet)
		{
		}

		TIOStream(T *p, unsigned sz, unsigned chunkSize = 0)
			: TOStream<T>(p, sz, chunkSize),
			mGet(base())
		{
		}

		TOStream<T> &base(){ return *this; }

	};


	/////////////////////////////////////////////////////////////////////
	// ByteRWStream

	class ByteStreamReader : public TOStreamReader<char>
	{
	public:
		ByteStreamReader(const TOStream<char> &r)
			: TOStreamReader<char>(r)
		{
		}
		ByteStreamReader(const TOStream<char> &r, const StreamPos &rPos)
			: TOStreamReader<char>(r, rPos)
		{
		}
		template <typename T>
		unsigned writeg(T t)
		{
			return write((char *)&t, sizeof(t));
		}
	};




	/////////////////////////////////////////////////////////////////////
	// ByteRWStream

	class ByteRWStream : public TIOStream<char>
	{
		typedef TIOStream<char> Base;
	public:
		ByteRWStream(const ByteRWStream &ss)
			: Base(ss)
		{
		}
		explicit ByteRWStream(unsigned chunkSize = 0)
			: Base(chunkSize)
		{
		}
		ByteRWStream(void *p, unsigned sz, unsigned chunkSize = 0)
			: Base((char *)p, sz, chunkSize)
		{
		}
		typedef Base::StreamPos	StreamPos;

		unsigned read_delim(void *pDst, unsigned bufSize, const char *pDelims, bool *pbDelimMet)
		{
			if (!pDst || !bufSize)
				return 0;

			if (mGet.IsNull())
			{
				mGet.Init(Head());
				if (mGet.IsNull())
					return 0;
			}

			unsigned left = bufSize;
			for (;;)
			{
				if (mGet.IsGetOverflow())
				{
					Chunk *pChunk(mGet.NextGetChunk());
					if (!pChunk)
						break;
					mGet.Init(pChunk);
				}

				if (mGet.mChunks.empty())
					break;

				unsigned sz(TGet_read_delim(pDst, left, pDelims, pbDelimMet));
				if (*pbDelimMet)
				{
					left -= sz;
					break;
				}

				if (!sz)
					break;

				left -= sz;
				if (left == 0)
					break;

				pDst = ((char*)pDst) + sz;
			}

			return (bufSize - left);
		}

		template <typename T>
		unsigned writex(T t)
		{
			return write((char *)&t, sizeof(T));
		}

		template <typename T>
		T readx()
		{
			T t;
			if (read((char *)&t, sizeof(t)) != sizeof(t))
				throw(-1);
			return t;
		}

	private:
		bool scan_delimiter(char c, const char *pDelims)
		{
			for (const char *pDelim = pDelims;; ++pDelim)
			{
				char delim = *pDelim;
				if (delim == '\xFF')
					break;//force skip the EOL
				if (c == delim)
					return true;

				if (!delim)
					break;
			}
			return false;
		}

		unsigned TGet_read_delim(void *pDst, unsigned len, const char *pDelims, bool *pbDelimMet)
		{
			assert(!mGet.mChunks.empty());
			assert(mGet.mOffset < chunkSize());
			unsigned upperLimit(mGet.mOffset + len);
			if (upperLimit > chunkSize())
				upperLimit = chunkSize();
			//if (len < upperLimit)
			//	upperLimit = len;
			if (mGet.mChunks.tail() == GetChunk())
				if (Pos() < upperLimit)
					upperLimit = Pos();

			unsigned i = 0;
			if (mGet.mOffset < upperLimit)
			{
				const char *pSrc = &mGet.mChunks.tail()->mData[mGet.mOffset];

				while (mGet.mOffset < upperLimit)
				{
					++mGet.mOffset;
					char c = pSrc[i];
					if (scan_delimiter(c, pDelims))
					{
						*pbDelimMet = true;
						return i;
					}
					((char *)pDst)[i++] = c;
				}
			}

			*pbDelimMet = false;
			return i;//?len;
		}
	};


	typedef MyStreamBase	StreamBase;

	/////////////////////////////////////////////////////////////////////
	// MyStream

	class Stream0 : public StreamBase, protected ByteRWStream//actual storage
	{
	public:
		Stream0(const Stream0 &ss)
			: StreamBase(ss),
			ByteRWStream(ss)
		{
		}
		explicit Stream0(unsigned chunkSize = 0)
			: ByteRWStream(chunkSize)
		{
		}
		Stream0(void *p, unsigned sz, unsigned chunkSize = 0)
			: ByteRWStream(p, sz, chunkSize)
		{
		}
	public:
		friend class StreamReader;
		typedef ByteRWStream::StreamPos	StreamPos;
		unsigned Seekg(unsigned p){ return rewind(p); }
		unsigned ReadDelim(void *p, unsigned len, const char *pDelims, bool *pbDelimMet){
			return read_delim((char *)p, len, pDelims, pbDelimMet);
		}

		const StreamPos &tellp() const { return pos(); }
		//StreamPos seekp(const StreamPos &o){ return ByteRWStream::seek(o); }
		const StreamPos &tellg() const { return posg(); }
		StreamPos seekg(const StreamPos &o){ return ByteRWStream::seekg(o); }

		// StreamBase
		virtual unsigned Read(void *p, unsigned sz){ return ByteRWStream::read((char *)p, sz); }
		virtual unsigned Write(void *p, unsigned sz){ return ByteRWStream::write((char *)p, sz); }
		virtual void Reset(bool bClean = false){ return reset(bClean); }
		virtual char Peek() const { return peek(); }
		virtual void *Data(unsigned &sz){ return data(sz); }
		virtual unsigned Skip(unsigned sz, int iReset = 0){ return skip(sz, iReset); }
		virtual unsigned Tellg() const { return ByteRWStream::tellg(); }
		virtual unsigned Tellp() const { return ByteRWStream::tellp(); }
		virtual unsigned Rewind(unsigned p){ return rewind(p); }
	};

	class StreamUtil
	{
		StreamBase &mss;
		char mcEol;
	public:
		StreamUtil(StreamBase &r)
			: mss(r),
			mcEol(0)
		{
		}
		/*StreamUtil(const StreamUtil &o)
			: mr(o.mr),
			mcEol(o.eol())
			{
			}*/
		StreamBase &ss() const { return mss; }
		char SetEndOfLineChar(char c){ char tmp = mcEol; mcEol = c; return tmp; }
		int eol() const { return mcEol; }

		unsigned ReadInt(int *, int = 0);
		unsigned ReadDouble(double *, int = 0);
		unsigned ReadBool(bool *, int = 0);
		unsigned ReadChar(char *, int = 0);
		char ReadChar();

		unsigned WriteStream(StreamBase &, unsigned = (unsigned)-1, int = 0);
		unsigned WriteString(StreamBase &);
		unsigned WriteString(const char *);
		unsigned WriteStringf(const char *, ...);
		//no eos versions
		unsigned WriteString0(StreamBase &);
		unsigned WriteString0(const char *);
		unsigned WriteStringf0(const char *, ...);
		unsigned WriteInt(int);
		unsigned WriteDouble(double);
		unsigned WriteChar(char);
		unsigned WriteBool(bool);
		unsigned WriteInt(const int *, int);
		unsigned WriteDouble(const double *, int);
		unsigned WriteBool(const bool *, int);

		//STL helper
		unsigned ReadString(std::string &, const char * delims = nullptr, bool bAppendDelim = false);
		unsigned WriteString(const std::string &);
		unsigned WriteString0(const std::string &);
		unsigned ReadStdStream(std::istream &, unsigned = (unsigned)-1);
		unsigned WriteStdStream(std::ostream &, unsigned = (unsigned)-1);
		unsigned WriteToFile(FILE *);

		unsigned SkipBytes(unsigned sz, int iReset = 0){ return mss.Skip(sz, iReset); }

	private:
		StreamUtil(const StreamUtil &o) : mss(o.mss){ assert(0); }
		StreamUtil &operator=(const StreamUtil &){ assert(0); return *this; }
	};




	class Stream : public Stream0, public StreamUtil
	{
	public:
		explicit Stream(unsigned chunkSize = 0)
			: Stream0(chunkSize),
			StreamUtil(reinterpret_cast<StreamBase &>(*this))
		{
		}
		Stream(void *p, unsigned sz, unsigned chunkSize = 0)
			: Stream0(p, sz, chunkSize),
			StreamUtil(reinterpret_cast<StreamBase &>(*this))
		{
		}
	};

	class StreamReader : public ByteStreamReader
	{
	public:
		StreamReader(const Stream0 &r)
			: ByteStreamReader(r)
		{
		}
		StreamReader(const Stream0 &r, const StreamPos &rPos)
			: ByteStreamReader(r, rPos)
		{
		}
	};

}//namespace My

typedef My::Stream					MyStream;
typedef My::StreamUtil				MyStreamUtil;
typedef	My::Stream0::StreamPos		MyStreamPos;
typedef My::StreamReader			MyStreamReader;


//////////////////////////////////////////////////////////////////////////////////////////////
//MY_SCRIPT

typedef MyStreamUtil	MyScriptArgType;
#define MY_SCRIPT_DECL(a)	bool MyScript##a(MyScriptArgType &ss)
#define MY_STRING(a)	MY_SCRIPT_DECL(a){ return readString(ss, s##a); }
#define MY_INT(a)		MY_SCRIPT_DECL(a){ return readInt(ss, n##a); }
#define MY_BOOL(a)		MY_SCRIPT_DECL(a){ return readBool(ss, b##a); }
#define MY_REAL(a)		MY_SCRIPT_DECL(a){ return readReal(ss, d##a); }

#define MY1_STRING(a)	std::string s##a; MY_SCRIPT_DECL(a){ return readString(ss, s##a); }
#define MY1_INT(a)		int n##a; MY_SCRIPT_DECL(a){ return readInt(ss, n##a); }
#define MY1_BOOL(a)		bool b##a; MY_SCRIPT_DECL(a){ return readBool(ss, b##a); }
#define MY1_REAL(a)		double d##a; MY_SCRIPT_DECL(a){ return readReal(ss, d##a); }

#define MY2_STRING(a)	MY_SCRIPT_DECL(a){ return readString(ss, o.s##a); }
#define MY2_INT(a)		MY_SCRIPT_DECL(a){ return readInt(ss, o.n##a); }
#define MY2_BOOL(a)		MY_SCRIPT_DECL(a){ return readBool(ss, o.b##a); }
#define MY2_REAL(a)		MY_SCRIPT_DECL(a){ return readReal(ss, o.d##a); }

#define MY_IMPL(a)	add(std::string(#a), &MyClass::MyScript##a);
#define MY_SCRIPT(a)	private: \
	typedef a MyClass; \
	typedef bool (MyClass::*MyScript)(MyScriptArgType &); \
	typedef std::map<std::string, MyScript> MyScriptMap; \
	typedef MyScriptMap::iterator MyScriptMapIt; \
	MyScriptMap m; \
	void add(std::string key, MyScript val){ \
		m.insert(std::pair<std::string, MyScript>(key, val)); } \
	bool readString(MyScriptArgType &ss, std::string &s){ \
		return (ss.ReadString(s, "\n") > 0); } \
	bool readInt(MyScriptArgType &ss, int &n){ \
		std::string s; if (!readString(ss, s)) return false; \
		n = atoi(s.c_str()); return true; } \
	bool readBool(MyScriptArgType &ss, bool &b){ \
		std::string s; if (!readString(ss, s)) return false; \
		b = (atoi(s.c_str()) != 0); return true; } \
	bool readReal(MyScriptArgType &ss, double &d){ \
		std::string s; if (!readString(ss, s)) return false; \
		d = atof(s.c_str()); return true; } \
	public: \
	bool exec(MyScriptArgType &ss){ return parse(ss); } \
	bool parse(MyScriptArgType &ss){ \
		std::string key; \
		while (ss.ReadString(key, "=")) { \
			MyScriptMapIt it = m.find(key); \
			if (it != m.end()){ \
				MyScript pf = it->second; \
				if (!(this->*pf)(ss)) return false; \
			} else if (key == "List") { \
				int n = 0; if (!readInt(ss, n)) return false; \
				while (n-- > 0) { std::string s; if (!readString(ss, s)) return false; } \
			} else if (key == "Data") { \
				int n = 0; if (!readInt(ss, n)) return false; ss.SkipBytes(n); \
			} \
		} \
		ss.SkipBytes((unsigned)-1); \
		return true; \
	}


