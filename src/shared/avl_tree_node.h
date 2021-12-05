#pragma once

#include <assert.h>

//template <typename NODE, class COMP> class AVLTree;

template <typename SELF, typename KEY>
struct AVLTreeNode 
{
public:
	typedef KEY	AVLTreeKey;
	const AVLTreeKey key;

	SELF *children[2];
	SELF *parent;
	int height;

	//friend class AVLTree<SELF>;
public:
	/*AVLTreeNode()
	{
		key = AVLTreeKey();
		parent = 0;
		children[0] = children[1] = 0;
		height = 0;
	}*/
	AVLTreeNode(AVLTreeKey k)
		: key(k)
	{
		parent = 0;
		children[0] = children[1] = 0;
		height = 0;
	}
	void clear()
	{
		children[0] = children[1] = parent = 0;
		height = 0;
	}
	AVLTreeNode & operator=(const AVLTreeNode& o)
	{
		assert(key == o.key);
		children[0] = o.children[0];
		children[1] = o.children[1];
		parent = o.parent;
		height = o.height;
		return *this;
	}
	void overrideKey(const AVLTreeKey &a)//USE WITH CAUTION!
	{
		assert(!children[0] && !children[1] && !parent);
		const_cast<AVLTreeKey &>(key) = a;
	}
	const AVLTreeKey &_key() const { return key; }
	//SELF *_parent() const { return parent; }
};

