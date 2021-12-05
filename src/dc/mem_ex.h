#pragma once

#include "shared/link.h"
#include "db/mem.h"
#include "xref.h"
#include "op.h"
#include "path.h"
#include "globs.h"



struct BlockInfo_t;
class GlobObj_t;

typedef XRef_t<FieldPtr> XFieldLink_t;

#define POW2_ENTRIES_PER_CHUNK	6//64
#define POW2_SIZEOF_PATH  LOG2_UINT32((sizeof(Path_t) - 1)) + 1
typedef mem::TPool<Path_t, POW2_ENTRIES_PER_CHUNK, POW2_SIZEOF_PATH>	MemPathPool;
#define POW2_SIZEOF_OP  LOG2_UINT32((sizeof(Op_t) - 1)) + 1
typedef mem::TPool<Op_t, POW2_ENTRIES_PER_CHUNK, POW2_SIZEOF_OP>	MemOpPool;


namespace tls {
	template <typename T>
	struct TlsMemPool
	{
		T* ptr;
		int refs;
	};
	void SetPoolPtr(MemOpPool*);
	void SetPoolPtr(MemPathPool*);
	TlsMemPool<MemOpPool> SwapPoolPtr(MemOpPool*, int);
	TlsMemPool<MemPathPool> SwapPoolPtr(MemPathPool*, int);
#if(NEW_PATH_PTR || NEW_OP_PTR)
	template <typename T>
	class MemPoolSwapper
	{
		TlsMemPool<T> old;
	public:
		MemPoolSwapper(T* p)
			: old(SwapPoolPtr(p, 0))
		{
		}
		~MemPoolSwapper()
		{
			SwapPoolPtr(old.ptr, old.refs);
		}
	};
#else
	template <typename T>
	class MemPoolSwapper
	{
	public:
		MemPoolSwapper(T*)
		{
		}
		~MemPoolSwapper()
		{
		}
	};
#endif
}


//////////////////////////////////////////////MemoryMgrEx_t
class MemoryMgrEx_t : public MemoryMgr_t
{
public:
#if(NEW_PATH_PTR)
	typedef MemPathPool	PathPool_t;
	typedef PathPool_t::Helper	PathHelper;
	typedef PathPool_t::EltIterator	PathEltIterator;
#else
	typedef TPoolWrapper<Path_t>	PathPool_t;
	typedef PathPool_t::MyPool::Helper	PathHelper;
	typedef PathPool_t::MyPool::EltIterator	PathEltIterator;
#endif

#if(NEW_OP_PTR)
	typedef MemOpPool	OpPool_t;
	typedef OpPool_t::Helper	OpHelper;
	typedef OpPool_t::EltIterator	OpEltIterator;
#else
	typedef TPoolWrapper<Op_t>	OpPool_t;
	typedef OpPool_t::MyPool::Helper	OpHelper;
	typedef OpPool_t::MyPool::EltIterator	OpEltIterator;
#endif
	
	PathPool_t mPaths;
	OpPool_t mOps;

public:
	MemoryMgrEx_t();
	~MemoryMgrEx_t();

	HPATH NewPath();
	HOP NewOp();
#if(!NO_OBJ_ID)
	HOP NewOpId(int);
#endif
	HOP NewIdOp();
	XOpLink_t *NewXOpLink();
	XOpLink_t *NewXOpLink2();
	XFieldLink_t *NewXFieldLink(FieldPtr);
	Ins_t* NewRootInfo();

	void Delete(PathPtr);
	void Delete(OpPtr);
	void Delete(XOpLink_t *);
	void Delete2(XOpLink_t *);
	void Delete(XFieldLink_t *);
	void Delete(Ins_t *);

	void Delete(BlockInfo_t *) {}

	void print_stats(std::ostream &os);

};


typedef FieldEx_t* FieldExPtr;
typedef const FieldEx_t* CFieldExPtr;

class MemoryMgr2_t : public MemoryMgr_t
{
public:
	TPoolWrapper<FieldEx_t> mFields2;
public:
	MemoryMgr2_t()
		: mFields2(16 * 64)//CH_SIZE
	{
	}

	static GlobObj_t* NewGlobObj();
	static GlobObj_t* NewGlobObj(Type_t*);

	void Delete1(GlobObj_t*);

	GlobObj_t* NewIntrinsic();

	FieldExPtr NewFieldEx(Field_t&&);//move
	FieldExPtr NewFieldExNoId();
#if(!NO_OBJ_ID)
	FieldExPtr NewFieldExId(int);
#endif

	FieldExPtr NewFieldEx(uint32_t);//key + auto id

	void Delete(FieldExPtr);
};




void PrintPoolRequstsInfo();


