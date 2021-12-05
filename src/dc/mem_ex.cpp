#include "mem_ex.h"
#include "path.h"
#include "type_funcdef.h"

#ifndef thread_local
# if __STDC_VERSION__ >= 201112 && !defined __STDC_NO_THREADS__
#  define thread_local _Thread_local
# elif defined _WIN32 && (defined _MSC_VER || defined __ICL || defined __DMC__ || defined __BORLANDC__)
#  define thread_local __declspec(thread) 
/* note that ICC (linux) and Clang are covered by __GNUC__ */
# elif defined __GNUC__ || defined __SUNPRO_C || defined __xlC__
#  define thread_local __thread
# else
#  error "Cannot define thread_local"
# endif
#endif

#define CHECK_POOL_REQ 0

#if(NEW_OP_PTR)
namespace tls {

	thread_local TlsMemPool<MemOpPool> opPool = { nullptr, 0 };
#if(CHECK_POOL_REQ)
	thread_local int opPoolGetCount = 0;
#endif

	void SetPoolPtr(MemOpPool *pool)
	{
		assert(!opPool.ptr || !pool || opPool.ptr == pool);
		if (pool)
		{
			if (opPool.refs++ == 0)
			{
				assert(!opPool.ptr);
				opPool.ptr = pool;
			}
		}
		else
		{
			assert(opPool.refs > 0);
			if (--opPool.refs == 0)
				opPool.ptr = nullptr;
		}
	}

	TlsMemPool<MemOpPool> SwapPoolPtr(MemOpPool* pool, int refs)
	{
		TlsMemPool<MemOpPool> old(opPool);
		opPool.ptr = pool;
		opPool.refs = refs;
		return old;
	}

}//namespace tls

Op_t *OpPtr::toOp(mem::MemRef h)
{
#if(CHECK_POOL_REQ)
	tls::opPoolGetCount++;
#endif
	if (!tls::opPool.ptr)
		return 0;
	return tls::opPool.ptr->get(h);
}
#endif


#if(NEW_PATH_PTR)
namespace tls {
	thread_local TlsMemPool<MemPathPool> pathPool = { nullptr, 0 };
#if(CHECK_POOL_REQ)
	thread_local int pathPoolGetCount = 0;
#endif

	void SetPoolPtr(MemPathPool *pool)
	{
		assert(!pathPool.ptr || !pool || pathPool.ptr == pool);
		if (pool)
		{
			if (pathPool.refs++ == 0)
			{
				assert(!pathPool.ptr);
				pathPool.ptr = pool;
			}
		}
		else
		{
			assert(pathPool.refs > 0);
			if (--pathPool.refs == 0)
				pathPool.ptr = nullptr;
		}
	}

	TlsMemPool<MemPathPool> SwapPoolPtr(MemPathPool* pool, int refs)
	{
		TlsMemPool<MemPathPool> old(pathPool);
		pathPool.ptr = pool;
		pathPool.refs = refs;
		return old;
	}
}

Path_t *PathPtr::toPath(mem::MemRef h)
{
#if(CHECK_POOL_REQ)
	tls::pathPoolGetCount++;
#endif
	return tls::pathPool.ptr->get(h);
}
#endif


void PrintPoolRequstsInfo()
{
#if(CHECK_POOL_REQ)
	fprintf(stdout, " +++ PATH pool fetch count: %d\n", tls::pathPoolGetCount);
	fprintf(stdout, " +++ OP pool fetch count: %d\n", tls::opPoolGetCount);
	fflush(stdout);
#endif
}

//////////////////////////////////////

MemoryMgrEx_t::MemoryMgrEx_t()
{
}

MemoryMgrEx_t::~MemoryMgrEx_t()
{
}

HPATH MemoryMgrEx_t::NewPath()
{
	return mPaths.New();
}

void MemoryMgrEx_t::Delete(PathPtr p)
{
	assert(p);
	mPaths.Delete(p);
}

/////////

HOP MemoryMgrEx_t::NewOp()
{
	return mOps.New();
}

#if(!NO_OBJ_ID)
HOP MemoryMgrEx_t::NewOpId(int id)
{
	return mOps.New(id);
}
#endif

HOP MemoryMgrEx_t::NewIdOp()
{
#if(NO_OBJ_ID)
	return mOps.New();
#else
	return mOps.New(IdCheck_t::idNext());
#endif
}

void MemoryMgrEx_t::Delete(OpPtr p)
{
	assert(p);
	mOps.Delete(p);
}

Ins_t* MemoryMgrEx_t::NewRootInfo()
{
	return new Ins_t();
}

void MemoryMgrEx_t::Delete(Ins_t *p)
{
	if (!p)
		return;
	p->destruct(*this);
	delete p;
}

// XOpLink_t

XOpLink_t *MemoryMgrEx_t::NewXOpLink()
{
	return new XOpLink_t;
}

XOpLink_t *MemoryMgrEx_t::NewXOpLink2()
{
	return new XOpLink_t[2];
}

void MemoryMgrEx_t::Delete(XOpLink_t *p)
{
	if (!p)
		return;
	/*p->destruct(*this);*/
	//mXRefOps.Delete(p);
	delete p;
}

void MemoryMgrEx_t::Delete2(XOpLink_t *p)
{
	if (!p)
		return;
	/*p->destruct(*this);*/
	//mXRefOps.Delete(p);
	delete [] p;
}

XFieldLink_t *MemoryMgrEx_t::NewXFieldLink(FieldPtr p)
{
	//assert(0);
	return new XFieldLink_t(p);
}

void MemoryMgrEx_t::Delete(XFieldLink_t *p)
{
	//assert(0);
	delete p;
}

void MemoryMgrEx_t::print_stats(std::ostream &os)
{
	os << "[paths]\n";
	mPaths.pool().print_stats(os);
	os << "[ops]\n";
	mOps.pool().print_stats(os);
	//os << "[xrefs]\n";
	//mXRefOps.pool().print_stats(os);
	//os << "[rootinfos]\n";
	//mRootInfos.pool().print_stats(os);
	MemoryMgr_t::print_stats(os);
}








GlobObj_t* MemoryMgr2_t::NewGlobObj(Type_t* t)
{
	GlobObj_t* p(NewGlobObj());
	p->SetPvt(t);
	return p;
}


GlobObj_t* MemoryMgr2_t::NewGlobObj()
{
#if(!NO_OBJ_ID)
	return new GlobObj_t(IdCheck_t::idNext());
#else
	return new GlobObj_t;
#endif
}

void MemoryMgr2_t::Delete1(GlobObj_t* g)
{
	assert(g);

	FieldExPtr p(FieldEx_t::dockField(g));
	//FieldPtr p(g->dockField());
	if (p)
	{
		//assert(p == px);
		FieldPtr p2(NewField(std::move(*p)));
		Delete(p);
	}
	else//intrinsic?
	{
		assert(0);
		abort();
		//Delete(px);
		//g->destruct(*this);
//		g->ClearPvt();
//		delete g;
	}
}





/////////////////////////Field_t 

FieldExPtr MemoryMgr2_t::NewFieldEx(Field_t&& o)
{
#if(NO_OBJ_ID)
	return mFields2.New1(o);	
#else
	return mFields2.New1(o, o.ID());// IdCheck_t::idNext());
#endif
}

FieldExPtr MemoryMgr2_t::NewFieldExNoId()
{
	return mFields2.New();
}

#if(!NO_OBJ_ID)
FieldExPtr MemoryMgr2_t::NewFieldExId(int id)
{
	return mFields2.New(id);
}
#endif

FieldExPtr MemoryMgr2_t::NewFieldEx(FieldKeyType key)
{
#if(NO_OBJ_ID)
	FieldExPtr p(mFields2.New());
#else
	FieldExPtr p(mFields2.New(IdCheck_t::idNext()));
#endif
	p->overrideKey(key);
	return p;
}

GlobPtr MemoryMgr2_t::NewIntrinsic()
{
	FieldExPtr p(mFields2.New());
	p->glob().SetPvt(new FuncDef_t());
	return p->globPtr();
}

void MemoryMgr2_t::Delete(FieldExPtr p)
{
	if (!p)
		return;
	p->destruct(*this);
	mFields2.Delete(p);
}

