#pragma once

#include <assert.h>

#include "shared/tree.h"
#include "op.h"
//#include "type_funcdef.h"
#include "shared/misc.h"


enum BlockTyp_t 
{
	BLK_NULL		= 0,
	BLK_JMP			= 0x0000001,	//relative <goto>
	BLK_JMPSWITCH	= 0x0000002,	//indirect <goto>
	BLK_JMPIF		= 0x0000003,	//relative <if-goto>
	BLK_CALL		= 0x0000004,
	BLK_BODY		= 0x000000A,
	BLK_EXIT		= 0x000000B,	//caller's path, after this func's call
	BLK_ENTER		= 0x000000C,	//caller's path, before this func's call
	BLK_DATA		= 0x000000F,	//contains data ops
	BLK_TERM_MASK	= 0x000000F,

		BLK_HASCHILDS	= 0x0010000,	
		//BLK_FIXED		= 0x0,	//with fixed number of childs (~2)
	BLK_IF			= BLK_HASCHILDS|0x0000010,
	BLK_IFELSE		= BLK_HASCHILDS|0x0000020,
	BLK_IFWHILE		= BLK_HASCHILDS|0x0000030,
	BLK_IFFOR		= BLK_HASCHILDS|0x0000040,
	BLK_CASE		= BLK_HASCHILDS|0x0000050,	
	BLK_DEFAULT		= BLK_HASCHILDS|0x0000060,
	BLK_SWITCH		= BLK_HASCHILDS|0x0000070,//inner block of switch
		BLK_LOGIC		= 0x0020000,
	BLK_AND			= BLK_LOGIC|BLK_HASCHILDS|0x0000100,
	BLK_OR			= BLK_LOGIC|BLK_HASCHILDS|0x0000200,
		BLK_LOOP		= 0x0040000,
	BLK_SWITCH_0	= BLK_LOOP|BLK_HASCHILDS|0x0001000,//outer block of switch (with default section)
	BLK_LOOPDOWHILE	= BLK_LOOP|BLK_HASCHILDS|0x0002000,
	BLK_LOOPENDLESS	= BLK_LOOP|BLK_HASCHILDS|0x0004000,
	//BLK_LOOPFOR		= BLK_LOOP|BLK_HASCHILDS|0x0008000,	//not realized
};

enum JumpStatus_t
{
	JUMP_NULL,
	JUMP_GOTO,
	JUMP_BREAK,
	JUMP_CONTINUE,
	JUMP_SWITCH,
	JUMP_RET,
};

class PathTreeEx_t;
class MemoryMgr_t;


typedef	PathTree_t::LeafIterator	TreePathLeafIterator;

union path_trace_cell_t
{
	uint8_t	flags;//trace info
	struct {
		uint8_t	_traced : 3;
		//uint8_t	_bTraced1:1;
		//uint8_t	_bTraced2:1;
		uint8_t	_xoutseen : 1;
		//uint8_t	m_b5:2;
	};
};

#define PATH_ID	0

class Path_t : public TreeNode_t<PathPtr>
{
public:
#if(PATH_ID)
	unsigned		mID;
#endif
private:

	OpList_t	mOps0;

	union {
		uint32_t		m_flags;
		BlockTyp_t	m_type:24;
	};

	ADDR			_mVA;
	XOpList_t		mInflow;//list of ops referencing a label

public:
#if(!NEW_PATH_TRACER)
	path_trace_cell_t	m_traceInfo;
#endif

public:

	Path_t();//BlockTyp_t = BLK_NULL);
	~Path_t();

	typedef OpPtr	OpEltPtr;
	typedef OpList_t	MyOpList;
	typedef MyOpList::Iterator	OpIterator;

	void reset() {
		m_flags &= BLK_TERM_MASK;
	}

	const XOpList_t &inflow() const { return mInflow; }

	MyOpList &opsRef(){ return mOps0; }
	const MyOpList &ops() const { return mOps0; }
	HOP headOp() const { 
		return mOps0.front();
	}
	HOP tailOp() const { 
		return !mOps0.empty() ? HOP(mOps0.back()) : HOP();
	}
	bool hasOps() const {
		return !mOps0.empty();
	}
	HOP takeOp(HOP p){
		return mOps0.take(p);
	}
	HOP takeOpFront(){
		return takeOp(headOp());
	}
	void pushOpFront(HOP p){
		assert(p);
		mOps0.push_front(p);
	}
	void pushOpBack(HOP p){
		assert(p);
		mOps0.push_back(p);
	}
	void insertOpAfter(HOP p, HOP pAfter){
		mOps0.LinkAfter(p, pAfter);
	}
	void insertOpBefore(HOP p, HOP pBefore){
		mOps0.LinkBefore(p, pBefore);
	}
	void setOps(OpPtr p){
		mOps0 = p;
	}

	void pushRefBack(XOpLink_t *pXRef){
		mInflow.push_back(pXRef);
	}
	XOpLink_t *takeRef(XOpList_t::Iterator i){
		assert(!mInflow.empty());
		return mInflow.take(i);
	}

	XOpLink_t *takeRefFront()
	{
		assert(!mInflow.empty());
		return mInflow.take_front();
	}

	bool hasRefs() const {
		return !mInflow.empty();
	}

	//void destruct(MemoryMgr_t &);

	uint32_t flags() const { return m_flags; }
	void setFlags(uint32_t n){ m_flags = n; }

	//NamesMgr_t *owner namesMgr();

	//bool hasLocals() const { return (mpLocals2 != nullptr); }
	//const FieldMap &locals() const;
	//FieldPtr 	locals();
	BlockTyp_t Type() const { return BlockTyp_t(m_flags & PATH_TYPE_MASK); }
	void	SetType(BlockTyp_t n){ 
		m_flags &= ~PATH_TYPE_MASK;
		m_flags |= (n & PATH_TYPE_MASK);
	}
	bool	isAnalized() const { return (m_flags & PATH_ANALIZED) != 0; }
	void	setAnalized(bool b){
		if (b)
			m_flags |= PATH_ANALIZED;
		else
			m_flags &= ~PATH_ANALIZED;
	}
	void	invert(){ m_flags ^= PATH_INVERTED; }
	bool	isInverted() const { return (m_flags & PATH_INVERTED )!= 0; }

	ADDR	anchor() const {// return mVA; }
		if (!hasOps())
			return 0;
		HOP hOp(headOp());
		return hOp->VA();
	}

	/*void	setAnchor(ADDR va) {
		mVA = va;
	}*/
	bool	IsIfWhileOrFor() const {
		return (Type() == BLK_IFWHILE || Type() == BLK_IFFOR); }
};

typedef Path_t::MyOpList PathOpList_t;

class Out_t;

struct SwitchQuery_t
{
	HPATH		pJumpTablePath;
	FieldPtr	pJumpTable;
	int			nIndexOffset1;	//offset of jump table's index 
	HPATH		pIndexTablePath;
	FieldPtr	pIndexTable;
	int		nIndexOffset2;	//offset of index table's index
	HPATH	pPathDefault;	//default (if) block
	Out_t	*pOutWrongTable;
	Out_t	*pOutWrongOffset;
	SwitchQuery_t()
		: pJumpTablePath(HPATH()),
		pJumpTable(nullptr),
		pIndexTablePath(HPATH()),
		pIndexTable(nullptr),
		nIndexOffset1(0),
		nIndexOffset2(0),
		pPathDefault(HPATH()),
		pOutWrongTable(nullptr),
		pOutWrongOffset(nullptr)
	{
	}
};

struct BlockInfo_t;

class PathTreeEx_t//wrapper
{
public:
	PathTree_t m;
public:
	PathTreeEx_t()
	{
	}
	~PathTreeEx_t()
	{
		assert(m.empty());
	}

	const PathTree_t &tree() const { return m; }
	PathTree_t &tree(){ return m; }

	PathPtr body() const { return m.Head(); }// mpPath;
	void setBody(PathPtr p){ m = p; }
	PathPtr pathHead() const;
	PathPtr pathTail() const;

	//void __NumberizeOps(HPATH , int &nId);
	int isPathDegenerate(CHPATH) const;
	//HOP __GetOp(HPATH , int &nId);
	//int __CountOps(HPATH , int &locex);
	//int	ChangeGotoPath(HPATH , HPATH pPathNew);
};



struct BlockInfo_t
{
	HPATH 	pPath;
	BlockTyp_t	nType;
};







