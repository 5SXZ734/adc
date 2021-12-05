#pragma once

#include <assert.h>
#include <type_traits>
#include <list>
//#include "db/mem.h"



//template <typename, typename> class Tree_t;
template <typename> class Tree_t;

class MemoryMgr_t;


template <typename NodePtr>
bool TreeIsFirst(NodePtr pSelf)
{
	if (!pSelf->Parent())
	{
		assert(pSelf->IsRoot());
		return true;
	}
	return pSelf->Parent()->FirstChild() == pSelf;
}

template <typename NodePtr>
bool TreeIsBody(NodePtr pSelf)
{
	return (pSelf->IsRoot() && TreeIsFirst(pSelf));
}

template <typename NodePtr>
bool TreeIsLast(NodePtr pSelf)
{
	if (!pSelf->Parent())
	{
		assert(pSelf->IsRoot());
		return true;
	}
	return pSelf->Parent()->LastChild() == pSelf;
}

template <typename NodePtr>
NodePtr TreePrev(NodePtr pSelf)
{
	if (!pSelf->Parent())
	{
		assert(pSelf->IsRoot());
		return NodePtr();
	}
	if (pSelf->Parent()->FirstChild() == pSelf)
		return NodePtr();
	return pSelf->pPrev;
}

template <typename NodePtr>
NodePtr TreeNext(NodePtr pSelf)
{
	if (!pSelf->Parent())
	{
		assert(pSelf->IsRoot());
		return NodePtr();
	}
	if (pSelf->Parent()->LastChild() == pSelf)
		return NodePtr();
	return pSelf->pNext;
}

template <typename NodePtr>
NodePtr TreeTerminalFirst(NodePtr pSelf)
{
	NodePtr pPath(pSelf);
	for (;;)
	{
		if (pPath->IsTerminal())
			break;
		pPath = pPath->mChildren.Head();
	}
	assert(pPath);
	return pPath;
}

template <typename NodePtr>
NodePtr TreeTerminalLast(NodePtr pSelf)
{
	NodePtr pPath(pSelf);
	while (!pPath->IsTerminal())
	{
		pPath = pPath->GetChildLast();//get last
	}
	assert(pPath);
	return pPath;
}

template <typename NodePtr>
NodePtr TreeNextEx(NodePtr pSelf)
{
	NodePtr p(pSelf);
	while (TreeIsLast(p))
	{
		p = p->Parent();
		if (!p)
			break;
	}
	if (p)
	{
		p = TreeNext(p);
		if (p)
			while (!p->IsTerminal())
				p = p->mChildren.Head();
	}
	return p;
}

template <typename NodePtr>
NodePtr TreePrevEx(NodePtr pSelf)
{
	assert(pSelf->IsTerminal());
	NodePtr pPath(pSelf);
	while (TreeIsFirst(pPath))
	{
		pPath = pPath->Parent();
		if (!pPath)
			break;
	}
	if (pPath)
	{
		pPath = TreePrev(pPath);
		while (pPath && !pPath->IsTerminal())
		{
			//pPath = pPath->Childs();
			pPath = pPath->GetChildLast();//get last!!!
		}
	}
	return pPath;
}

template <typename NodePtr>
bool TreeIsFirstEx(NodePtr pSelf)
{
	assert(pSelf->IsTerminal());
	NodePtr pPath(pSelf);
	for (;;)
	{
		if (TreeIsFirst(pPath))
		{
			if (!pPath->Parent())//upper level reached
				return true;
			pPath = pPath->Parent();
		}
		else
			break;
	}
	return false;
}

template <typename NodePtr>
bool TreeIsLastEx(NodePtr pSelf)
{
	assert(pSelf->IsTerminal());
	NodePtr pPath(pSelf);
	for (;;)
	{
		if (TreeIsLast(pPath))
		{
			if (!pPath->Parent())//upper level reached
				return true;
			pPath = pPath->Parent();
		}
		else
			break;
	}
	return false;
}

template <typename NodePtr>
int TreeIndexEx(NodePtr pSelf)
{
	assert(pSelf->IsTerminal());//only leaves allowed - no branches
	int i(0);
	NodePtr p(pSelf);
	while (!TreeIsFirstEx(p))
	{
		p = TreePrevEx(p);
		i++;
	}
	return i;
}

template <typename NodePtr>
NodePtr TreeNextSibling(NodePtr pSelf)
{
	assert(pSelf->pParent);
	if (pSelf->pNext == pSelf->pParent->Children().Head())
		return NodePtr();
	return pSelf->pNext;
}

template <typename NodePtr>
NodePtr TreePrevSibling(NodePtr pSelf)
{
	assert(pSelf->pParent);
	if (pSelf == pSelf->pParent->Children().Head())
		return NodePtr();
	return pSelf->pPrev;
}

template <typename NodePtr>
void TreePushChildFront(NodePtr pSelf, NodePtr p)
{
	pSelf->mChildren.push_front(p);
	p->SetParent(pSelf);
}

template <typename NodePtr>
void TreePushChildBack(NodePtr pSelf, NodePtr p)
{
	pSelf->mChildren.push_back(p);
	p->SetParent(pSelf);
}

template <typename NodePtr>
bool TreeInsertChildBefore(NodePtr pSelf, NodePtr p, NodePtr pOther)
{
	if (!pSelf->mChildren.insert_before(p, pOther))
		return false;
	p->SetParent(pSelf);
	return true;
}

template <typename NodePtr>
bool TreeInsertChildAfter(NodePtr pSelf, NodePtr p, NodePtr pOther)
{
	if (!pSelf->mChildren.insert_after(p, pOther))
		return false;
	p->SetParent(pSelf);
	return true;
}

template <typename NodePtr>
void TreeTakeChild(NodePtr pSelf, NodePtr p)
{
	pSelf->UnlinkChild(p);
}

template <typename NodePtr>
NodePtr TreeCommonLowermost(NodePtr pSelf, NodePtr pPath0)
{
	NodePtr pBlock(pSelf);

	//search for common block for 2 paths
	NodePtr pPath = NodePtr();
	while (pBlock)
	{
		pPath = pPath0;
		while (pPath && (pPath->Parent() != pBlock->Parent()))//!Are Chained(pPath, pBlock))
			pPath = pPath->Parent();
		if (pPath)
			break;
		pBlock = pBlock->Parent();
	}

	if (pPath)
	{//list was found, now get upper path
		if (pSelf->CheckRelation(pPath, pBlock) == 1)
			pPath = pBlock;
	}

	return pPath;
}

template <typename NodePtr>
bool TreeIsLowerThan(NodePtr pSelf, NodePtr pBlock)//test all blocks
{
	if (pBlock == pSelf)
		return false;

	NodePtr pPath = pBlock;
	while (1)
	{
		if (pPath->HasChildren())
			pPath = pPath->FirstChild();//go down the tree
		else
		{
			while (1)
			{
				NodePtr pPathNx = TreeNext(pPath);
				if (pPathNx)//go up
				{
					pPath = pPathNx;
					break;
				}
				pPath = pPath->Parent();
				if (!pPath)
					return false;//not found - pBlock is lower
			}
		}

		if (pPath == pSelf)
			return true;//found this - is lower
	}

	assert(false);
	return false;
}

template <typename NodePtr>
bool TreeAddChild(NodePtr pSelf, NodePtr p, bool bHead = false)
{
	assert(p);
	if (p->IsLinked())
		p->Parent()->UnlinkChild(p);
	if (bHead)
		pSelf->mChildren.push_front(p);
	else
		pSelf->mChildren.push_back(p);
	p->SetParent(pSelf);
	return true;
}

template <typename NodePtr>
NodePtr TreeRoot(NodePtr pSelf)
{
	NodePtr p(pSelf);
	while (p->Parent())
		p = p->Parent();
	return p;
}

template <typename NodePtr>
bool TreeIsChildOf(NodePtr pSelf, NodePtr pPathTop)
{
	assert(pPathTop);
	NodePtr p(pSelf);
	while (p)
	{
		p = p->Parent();
		if (p == pPathTop)
			return true;
	}
	return false;
}




template <typename T>//, typename TPTR = void>
class TreeNode_t
{
public:
	//typedef Tree_t<T, TPTR>	Tree;
	typedef T SelfPtr;
	typedef Tree_t<SelfPtr>	Tree;
	//using SelfPtr = std::conditional_t<std::is_same<void, TPTR>::value, TreeNode_t*, TPTR>;

public:
	SelfPtr pPrev;
	SelfPtr	pNext;
	SelfPtr	pParent;
	Tree	mChildren;
	//T m;
public:
	TreeNode_t()
		: pPrev(SelfPtr()),
		pNext(SelfPtr()),
		pParent(SelfPtr())
	{
	}
	~TreeNode_t()
	{
		assert(mChildren.empty());
	}

	//T &data() { return m; }
	//const T &data() const { return m; }

	void Clear()
	{
		pPrev = pNext = SelfPtr();
		pParent = SelfPtr();
		mChildren.Clear();//?
	}

	bool IsLinked()
	{
		return pNext && pPrev && pParent;
	}

	int ChildIndex(SelfPtr p) const { return mChildren.indexOf(p); }

	bool IsRoot() const { return (pParent == SelfPtr()); }
	bool IsTerminal() const
	{
		return mChildren.IsEmpty();
		//return !IsBody() && mChildren.IsEmpty();
	}
	bool HasChildren() const { return !mChildren.empty(); }

	bool IsLastChild(SelfPtr p) const
	{
		return mChildren.Head() == p->pNext;
		//return LastChild() == p;
	}
	bool IsFirstChild(SelfPtr p) const { return FirstChild() == p; }

	void SetNext(SelfPtr p) { pNext = p; }
	void SetPrev(SelfPtr p) { pPrev = p; }
	void SetParent(SelfPtr p) { pParent = p; }
	void SetChildren(SelfPtr p) { mChildren = p; }

	SelfPtr Parent() const { return pParent; }
	Tree &Children(){ return mChildren; }
	const Tree &Children() const { return mChildren; }
	bool HasChildren() { return !mChildren.empty(); }
	//SelfPtr Childs() const { return mChildren.Head(); }//get first child
	SelfPtr GetChildLast() const { assert(!mChildren.empty()); return mChildren.back(); }
	SelfPtr GetChildFirst() const { return mChildren.front(); }
	int CheckChildrenCount(int i) const { return mChildren.check_count(i); }
	SelfPtr FirstChild() const { return mChildren.Head(); }
	SelfPtr LastChild() const { return mChildren.Tail(); }
	SelfPtr TakeChild(SelfPtr p)
	{
		if (!UnlinkChild(p))
		{
			assert(0);
		}
		return p;
	}
	SelfPtr TakeFirstChild()
	{
		SelfPtr p(mChildren.Head());
		if (!UnlinkChild(p))
		{
			assert(0);
		}
		return p;
	}

	SelfPtr GetChild(int i) const
	{
		return mChildren.at(i);
	}

	//-1:this is closier to head; 0:the same; 1-otherwise
	static int CheckRelation(SelfPtr p, SelfPtr pLink)
		//-2:failed to determine
		//-1:this is closier to head; 
		//0:the same; 
		//1-otherwise
	{
		if (pLink == p)
			return 0;

		if (p->Parent() != pLink->Parent())
			return -2;

		while (!TreeIsFirst(pLink))
		{
			pLink = TreePrev(pLink);
			if (pLink == p)
				return -1;
		}

		return 1;
	}

	/*bool IsLowerThan2(SelfPtr pBlock) const
	{
		if (pBlock == Th is())
			return false;

		SelfPtr pPath(Th is());
		while (1)//go up the tree
		{
			SelfPtr pPathPr = TreePrev(pPath);
			if (!pPathPr)
			{
				pPath = pPath->Parent();
				if (!pPath)
					return false;//higher
			}
			else
				pPath = pPathPr;

			if (pPath == pBlock)
				return true;//found - this is lower
		}

		return false;
	}*/


	SelfPtr GetOnLevelPath(SelfPtr pPath, int nCheck) const//-1:first,0:normal,1-last
	{
		assert(pPath);

		do {
			if (pPath->Parent() == Parent())
				return pPath;

			if (nCheck == -1)
			{
				if (!TreeIsFirst(pPath))
					return SelfPtr();
			}
			else if (nCheck == 1)
			{
				if (!TreeIsLast(pPath))
					return SelfPtr();
			}

			pPath = pPath->Parent();
		} while (pPath);

		return SelfPtr();
	}

	int UnlinkChild(SelfPtr p)
	{
		mChildren.Unlink(p);
		p->pParent = SelfPtr();
		return 1;
	}
};



///////////////////////////////////////////
template <typename T>//, typename TPTR = void>
class Tree_t
{
public:
	typedef T NodePtr;
	//typedef TreeNode_t<T, TPTR>	Node;
	//using NodePtr = std::conditional_t<std::is_same<void, TPTR>::value, Node*, TPTR>;
		//typedef	typename Node::SelfPtr	NodePtr;
	//typedef T NodeData;
public:
	NodePtr	mp;
public:
	Tree_t()
		: mp(NodePtr())
	{
	}
	/*Tree_t(NodePtr p)
		: mp(p)
	{
	}*/
	~Tree_t()
	{
	}
	void operator=(NodePtr p)
	{
		mp = p;
	}

	void AddRoot(NodePtr p)//no parent
	{
		assert(!mp);
		assert(!p->IsLinked());
		p->SetNext(p);
		p->SetPrev(p);
		mp = p;
	}

	int insert_before(NodePtr p, NodePtr pOther)
	{
		if (pOther == p)
			return 0;
		if (p->pNext == pOther)
			return 1;//already
		assert(!p->IsLinked());
		if (mp == pOther)
			mp = p;
		pOther->pPrev->pNext = p;
		p->pPrev = pOther->pPrev;
		pOther->pPrev = p;
		p->pNext = pOther;
		return 1;
	}

	int insert_after(NodePtr p, NodePtr pOther)
	{
		if (pOther == p)
			return 0;
		if (p->pPrev == pOther)
			return 1;//already
		assert(!p->IsLinked());
		pOther->pNext->pPrev = p;
		p->pNext = pOther->pNext;
		pOther->pNext = p;
		p->pPrev = pOther;
		return 1;
	}

	void push_front(NodePtr p)
	{
		assert(!p->IsLinked());
		if (!mp)
		{
			p->pNext = p->pPrev = p;
		}
		else
		{
			p->pNext = mp;
			p->pPrev = p->pNext->pPrev;
			p->pNext->pPrev = p;
			p->pPrev->pNext = p;
		}
		mp = p;
	}

	void push_back(NodePtr p)
	{
		assert(!p->IsLinked());
		if (!mp)
		{
			p->pNext = p->pPrev = p;
			mp = p;
		}
		else
		{
			p->pNext = mp;
			p->pPrev = p->pNext->pPrev;
			p->pNext->pPrev = p;
			p->pPrev->pNext = p;
		}
	}

	NodePtr at(int i) const
	{
		NodePtr p(mp);
		if (p)
		{
			do {
				if (i == 0)
					return p;
				i--;
				p = p->pNext;
			} while (p != mp);
		}
		return NodePtr(); // num is too large
	}

	int indexOf(NodePtr p0) const
	{
		if (empty())
			return -1;
		int i(0);
		for (NodePtr p(mp); p != p0; p = p->pNext)
			i++;
		return i;
	}

	int check_count(int i) const//<0 if less, ==0 if equal, >0 if greater
	{
		if (mp)
		{
			NodePtr p(mp);
			do {
				if (--i < 0)
					break;
				p = p->pNext;
			} while (p != mp);
		}
		return -i;
	}

	//Base &base() const { return *this; }
	/*?NodePtr TakeChild(NodePtr p)
	{
	UnlinkChild(p);
	return p;
	}*/

	int Unlink(NodePtr p)
	{
		if (p->pNext != p)
			p->pNext->pPrev = p->pPrev;
		if (p->pPrev != p)
			p->pPrev->pNext = p->pNext;
		if (mp == p)
		{
			if (p->pNext != p)
				mp = p->pNext;
			else
				mp = NodePtr();
		}
		p->pNext = p->pPrev = NodePtr();
		return 1;
	}

	NodePtr Head() const { return mp; }
	NodePtr Tail() const { return mp ? mp->pPrev : NodePtr(); }
	bool IsEmpty() const { return (mp == NodePtr()); }
	bool empty() const { return (mp == NodePtr()); }
	NodePtr front() const { return mp; }
	NodePtr back() const { return mp->pPrev; }

	//////////////// iterators

	template <typename TREE>//node
	class TIterator : public std::list<typename TREE::NodePtr>//this iterator must use only children/next relationships
	{
		typedef typename TREE::NodePtr	NodePtr;
		typedef std::list<NodePtr> Base;
		//Tree_t<U> &mr;
	public:
		TIterator(const TREE &tree)
			//: mr(r)
		{
			if (!tree.IsEmpty())
				this->push_back(tree.Head());
		}
		TIterator(const NodePtr root)
		{
			this->push_back(root);
		}
		operator bool() const { return !this->empty(); }
//?		const Node& operator*() const { return top(); }
		NodePtr operator*() { return top(); }
		//NodePtr top() { return this->back(); }
		const NodePtr top() const { return this->back(); }
		TIterator& operator ++()
		{
			NodePtr pBack(this->back());
			if (pBack->HasChildren())
			{
				this->push_back(pBack->FirstChild());
				return *this;
			}
			NodePtr pNext(next());
			if (pNext)
			{
				this->pop_back();
				this->push_back(pNext);
				return *this;
			}
			for (;;)
			{
				this->pop_back();
				if (this->empty())
					break;
				pNext = next();
				if (pNext)
				{
					this->pop_back();
					this->push_back(pNext);
					break;
				}
			}
			return *this;
		}
		TIterator& operator ++(int) { return operator++(); }
		int level() const { return (int)this->size() - 1; }
		NodePtr parent() const
		{
			typename Base::const_reverse_iterator ri(this->rbegin());
			assert(!this->empty());
			if (++ri != this->rend())
				return (*ri);
			return NodePtr();
		}
		NodePtr next() const
		{
			NodePtr p(parent());
			if (!p)
				return NodePtr();
			if (p->IsLastChild(this->back()))
				return NodePtr();
			return top()->pNext;
		}
	};
	typedef TIterator<Tree_t<T> > Iterator;

	/////////////////////////////////////////
	template <typename TREE>//node
	class TLeafIterator
	{
		typedef typename TREE::NodePtr	NodePtr;
		const TREE& mr;
		NodePtr	mpLeaf;
	public:
		TLeafIterator(const TREE &r)
			: mr(r),
			mpLeaf(r.empty() ? NodePtr() : TreeTerminalFirst(r.Head()))
		{
		}
		operator bool() const { return (mpLeaf != NodePtr()); }
		TLeafIterator& operator ++()
		{
			//mpLeaf = mpLeaf->NextEx();
			NodePtr p(mpLeaf);
			while (TreeIsLast(p))
			{
				p = (*p).Parent();
				if (!p)
					break;
				if (&(*p).Children() == &mr)
				{
					mpLeaf = NodePtr();
					return *this;
				}
			}
			if (p)
			{
				p = TreeNext(p);
				if (p)
				{
					for (;;)
					{
						if (p->IsTerminal())
							break;
						p = p->mChildren.Head();
					}
				}
			}
			mpLeaf = p;
			return *this;
		}
		TLeafIterator& operator ++(int) { return operator++(); }
		TLeafIterator& operator --() { mpLeaf = TreePrevEx(mpLeaf); return *this; }
		TLeafIterator& operator --(int) { return operator--(); }
		const NodePtr operator*() const { return mpLeaf; }
		NodePtr operator*() { return mpLeaf; }
		//const Node *data() const { return mp->Op(); }
		NodePtr data() { return mpLeaf; }
		//XRef_t<Node>* self() const { return mp; }
		void rebuild(NodePtr) {}
	};
	typedef TLeafIterator<Tree_t<T> > LeafIterator;

	///////////////////////////////////////////////
	template<typename TREE>//node
	class TChildrenIterator
	{
		typedef typename TREE::NodePtr NodePtr;
		const TREE	*mpList;
		NodePtr		mp;
	public:
		TChildrenIterator() : mpList(0), mp(NodePtr()) {}
		TChildrenIterator(const TREE &r) : mpList(&r), mp(r.Head()) {}
		TChildrenIterator(NodePtr p) : mpList(&p->Children()), mp(p->Children().Head()) {}
		void operator=(const TChildrenIterator &o) { mpList = o.mpList; mp = o.mp; }
		operator bool() const { return (mp != NodePtr()); }
		TChildrenIterator& operator ++() { mp = TreeNext(mp); return *this; }
		TChildrenIterator& operator ++(int) { return operator++(); }
		//const NodePtr operator*() const { return mp; }
		//NodePtr operator*() { return mp; }
		NodePtr data() const { return mp; }
		//NodePtr data() { return mp; }
		void clear() { mp = NodePtr(); }
		bool is_first() const { return mp == &mpList->front(); }
		const TREE& list() const { return *mpList; }
	};
	typedef TChildrenIterator<Tree_t<T> >	ChildrenIterator;


	//////////////////////////////////////////////////////////
	template <typename TREE>//node
	class THierIterator
	{
		typedef typename TREE::NodePtr	NodePtr;
		THierIterator *mpParent;
		TREE *mpTree;
		typename TREE::ChildrenIterator mi;
	public:
		THierIterator()
			: mpParent(0),
			mpTree(0)
		{
		}
		THierIterator(TREE &rTree)
			: mpParent(0),
			mpTree(&rTree),
			mi(rTree)
		{
		}
		/*THierIterator(const THierIterator &o)//copy constructor
		: mpParent(o.mpParent),
		mrTree(o.mrTree),
		mi(o.mi)
		{
		}*/
		THierIterator(bool, THierIterator &rUp)//bogus bool to distigwish from copy constructor
			: mpParent(&rUp),
			mpTree(&rUp.data()->Children()),
			mi(rUp.data()->Children())
		{
		}
		THierIterator &operator=(const THierIterator &o)
		{
			mpParent = o.mpParent;
			mpTree = o.mpTree;
			mi = o.mi;
			return *this;
		}
		NodePtr data() { return mi.data(); }
		NodePtr parentData()
		{
			if (!mpParent)
				return 0;
			return mpParent->data();
		}
		bool hasParent() { return mpParent != 0; }
		THierIterator &parent() { return *mpParent; }
		operator bool() const { return (mi != 0); }
		THierIterator& operator ++() { mi++; return *this; }
		THierIterator& operator ++(int) { return operator++(); }
		bool isFirst() { return mpParent->data()->IsFirstChild(data()); }
		bool isLast() { return mpParent->data()->IsLastChild(data()); }
		bool isTerminal() { return data()->IsTerminal(); }
		/*THierIterator terminalFirst()
		{
		//TreePathHierIterator i;
		//return data()->TreeTerminalFirst();
		assert(0);
		return i;
		}*/
	};

	typedef THierIterator<Tree_t<T> > HierIterator;
};






