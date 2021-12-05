
#pragma once

#include <algorithm>
#include <stdlib.h>

#include "avl_tree_node.h"

// AVL Tree (balanced binary search tree)

template <class NODE, class COMP = std::less<typename NODE::AVLTreeKey>>
class AVLTree : public COMP
{
	typedef COMP	key_compare;
	typedef	NODE	Node;
	typedef typename NODE::AVLTreeKey	AVLTreeKey;
	enum AVLTreeNodeSide
	{
		NODE_LEFT = 0,
		NODE_RIGHT = 1
	};


	Node *m_root_node;
	int m_num_nodes;

	//iterators
public:

	////////////////////////////////////////////////////////iterator
	class iterator
	{
		Node	*node;
		friend class AVLTree;
		friend class const_iterator;
		friend class reverse_iterator;
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef Node& reference;
		typedef Node* pointer;
		typedef const Node& const_reference;
		typedef const Node* const_pointer;
	public:
		//iterator(const AVLTree &r) : mr(r), node(nullptr){}
		//iterator(const iterator &o) : mr(o.mr), node(o.node){}
		//iterator(const AVLTree &r, Node *p) : mr(r), node(p){}
		explicit iterator(Node *in_node)
			: node(in_node)
		{
		}
		iterator(iterator const &) = default;
		iterator &operator=(const iterator &o){
			node = o.node;
			return *this;
		}
		//Node *pvt(){ return node; }
		//const Node *pvt() const { return node; }
		const_reference operator *() const
		{
			return *node;
		}
		reference operator *()
		{
			return *node;
		}
		const_pointer operator->() const
		{
			return node;
		}
		pointer operator->()
		{
			return node;
		}

		iterator operator++(){//prefix
			node = AVLTree::inOrderSuccessor(node);
			return *this;
		}
		iterator operator ++(int){//postfix
			iterator t(*this);
			operator++();
			return t;
		}
		bool operator==(const iterator &o) const {
			return node == o.node;
		}
		bool operator!=(const iterator &o) const { return !operator==(o); }
	//private:
		iterator operator--(){//prefix
			if (!node)//at the end?
			{
				assert(0);
				//node = mr.maxValueNode();
			}
			else
				node = AVLTree::inOrderPredecessor(node);
			return *this;
		}
		iterator operator --(int){//postfix
			iterator t(*this);
			operator--();
			return t;
		}
	};

	//iterator begin() const {
//		return iterator(const_cast<AVLTree &>(*this), minValueNode(m_root_node));
	//}
	iterator begin()
	{
		return iterator(minValueNode(m_root_node));
	}
	iterator end() const {
		return iterator(nullptr);
	}

	///////////////////////////////////////////////////////const_iterator
	class const_iterator
	{
		//const AVLTree	&mr;
		Node	*node;
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef const Node&	const_reference;
		//typedef typename Node *pointer;
		typedef const Node* const_pointer;
	public:
		explicit const_iterator(const Node *in_node) : node(const_cast<Node*>(in_node))
		{
		}
		const_iterator(iterator const &other) : node(other.node)
		{
		}
		const_iterator(const_iterator const &) = default;
		//const_iterator(const AVLTree &r) : mr(r), node(nullptr){}
		//const_iterator(const iterator &o) : mr(o.mr), node((Node *)o.pvt()){}
		//const_iterator(const AVLTree &r, Node *p) : mr(r), node(p){}
		//Node *pvt(){ return node; }
		//const Node *pvt() const { return node; }
		const_reference operator *() const
		{
			return *node;
		}
		const_pointer operator->() const
		{
			return node;
		}

		const_iterator &operator++(){//prefix
			node = AVLTree::inOrderSuccessor(node);
			return *this;
		}
		const_iterator operator ++(int){//postfix
			const_iterator t(*this);
			operator++();
			return t;
		}
		const_iterator &operator=(const const_iterator &o){
			node = o.node;
			return *this;
		}
		bool operator==(const const_iterator &o) const {
			return node == o.node;
		}
		bool operator!=(const const_iterator &o) const { return !operator==(o); }
	//private:
		const_iterator &operator--(){//prefix
			if (!node)//at the end?
			{
				assert(0);
				//node = mr.maxValueNode();
			}
			else
				node = AVLTree::inOrderPredecessor(node);
			return *this;
		}
		const_iterator operator --(int){//postfix
			const_iterator t(*this);
			operator--();
			return t;
		}
	};

	const_iterator begin() const
	{
		return const_iterator(minValueNode(m_root_node));
	}

	///////////////////////////////////////////////////reverse_iterator
	class reverse_iterator
	{
		friend class AVLTree;
		Node	*node;
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef Node& reference;
		typedef Node* pointer;
		typedef const Node& const_reference;
		typedef const Node* const_pointer;
	public:
		explicit reverse_iterator(Node *in_node) : node(in_node)
		{
		}
		explicit reverse_iterator(iterator const &other) : node(other.node)
		{
			//++*this;
		}
		reverse_iterator(reverse_iterator const &) = default;
		//reverse_iterator(const AVLTree &r) : mr(r), node(nullptr){}
		//reverse_iterator(const AVLTree &r, Node *p) : mr(r), node(p){}
		//reverse_iterator(iterator i) : mr(i.mr), node(i.pvt()){}
		//Node *pvt(){ return node; }
		//const Node *pvt() const { return node; }
		reference operator *() const
		{
			return *node;
		}
		pointer operator->() const
		{
			return node;
		}
		bool operator==(const reverse_iterator &o) const {
			return node == o.node;
		}
		bool operator!=(const reverse_iterator &o) const { return !operator==(o); }
	//private:
		reverse_iterator &operator++(){//prefix
			node = AVLTree::inOrderPredecessor(node);
			return *this;
		}
		reverse_iterator operator ++(int){//postfix
			reverse_iterator t(*this);
			operator++();
			return t;
		}
	};

	/*reverse_iterator rbegin() {
		return reverse_iterator(*this, maxValueNode(m_root_node));
	}*/
	reverse_iterator rbegin()
	{
		return reverse_iterator(maxValueNode());
	}
	reverse_iterator rend() {
		return reverse_iterator(nullptr);
	}


	///////////////////////////////////////////////////reverse_iterator
	class const_reverse_iterator
	{
		Node	*node;
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef Node& reference;
		typedef Node* pointer;
		typedef const Node& const_reference;
		typedef const Node* const_pointer;
	public:
		const_reverse_iterator() : node(nullptr){}
		const_reverse_iterator(Node *p) : node(p){}
		//Node *pvt(){ return node; }
		//const Node *pvt() const { return node; }
		const_reference operator *() const
		{
			return *node;
		}
		const_pointer operator->() const
		{
			return node;
		}
		const_reverse_iterator &operator++(){//prefix
			node = AVLTree::inOrderPredecessor(node);
			return *this;
		}
		const_reverse_iterator operator ++(int){//postfix
			const_reverse_iterator t(*this);
			operator++();
			return t;
		}
		const_reverse_iterator operator --(int){//postfix
			const_reverse_iterator t(*this);
			operator++();
			return t;
		}
		bool operator==(const const_reverse_iterator &o) const {
			return node == o.node;
		}
		bool operator!=(const const_reverse_iterator &o) const { return !operator==(o); }
	};

	const_reverse_iterator rbegin() const {
		return const_reverse_iterator(maxValueNode(m_root_node));
	}
	const_reverse_iterator rend() const {
		return const_reverse_iterator();
	}

	size_t size() const {
		return m_num_nodes;
	}

	iterator lower_bound(const AVLTreeKey &key) const
	{
		// find leftmost node not less than _Keyval
		Node *p = m_root_node;
		Node *_Wherenode = nullptr;// minValueNode(m_root_node);	// end() if search fails

		while (p != nullptr)
		{
			int iComp(p->compare(key));
			//if (p->key < key)
			if (iComp < 0)
				p = p->children[NODE_RIGHT];	// descend right subtree
			else
			{	// p not less than _Keyval, remember it
				_Wherenode = p;
				p = p->children[NODE_LEFT];	// descend left subtree
			}
		}

		return iterator(_Wherenode);	// return best remembered candidate
	}

	iterator upper_bound(const AVLTreeKey &key) const
	{
		// find leftmost node greater than _Keyval
		Node *p = m_root_node;
		Node *_Wherenode = nullptr;	// end() if search fails

		while (p != nullptr)
		{
			int iComp(p->compare(key));
			//if (key < p->key)
			if (iComp > 0)
			{	// p greater than _Keyval, remember it
				_Wherenode = p;
				p = p->children[NODE_LEFT];	// descend left subtree
			}
			else
				p = p->children[NODE_RIGHT];	// descend right subtree
		}

		return iterator(_Wherenode);	// return best remembered candidate
	}

	static bool is_linked(Node *node)
	{
		return (node->parent || node->children[NODE_LEFT] || node->children[NODE_RIGHT]);
	}

	struct insert_info
	{
		Node *	newNode;
		Node *	oldNode;
		insert_info(Node *n) : newNode(n), oldNode(nullptr){}
	};
	
	struct delete_info
	{
		const AVLTreeKey &key;
		Node *	node;
		delete_info(const AVLTreeKey &k) : key(k), node(nullptr){}
	};

public:
	AVLTree()
		: m_root_node(nullptr),
		m_num_nodes(0)
	{
	}

	key_compare& get_comparator_() {
		return *this;
	}

	key_compare const& get_comparator_() const {
		return *this;
	}

	bool empty() const
	{
		return (m_root_node == nullptr);
	}

	void reset()
	{
		m_root_node = nullptr;
		m_num_nodes = 0;
	}

	void moveFrom(AVLTree &o)
	{
		m_root_node = o.m_root_node;
		o.m_root_node = 0;
		m_num_nodes = o.m_num_nodes;
		o.m_num_nodes = 0;
	}

	/*Node *take(const AVLTreeKey &key)
	{
		delete_info data(key);
		avl_tree_remove(data);
		if (data.node)
			data.node->clear();
		return data.node;
	}*/

	Node *take(iterator &i)
	{
		delete_info data(i->key);
		++i;//std::advance(i, 1);
		avl_tree_remove(data);
		if (data.node)
			data.node->clear();
		return data.node;
	}

	Node *take(reverse_iterator &i)
	{
		delete_info data(i->key);
		++i;//std::advance(i, 1);
		avl_tree_remove(data);
		if (data.node)
			data.node->clear();
		return data.node;
	}

	iterator find(const AVLTreeKey &key) const
	{
		return iterator(avl_tree_lookup(key));
	}

	//get first node with case ignored
	iterator find_ncase(const AVLTreeKey &key) const
	{
		return iterator(avl_tree_lookup_ncase(key));
	}

	//Node *root() const { return m_root_node; }
	/*size_t saveRoot(Node *base_node) const
	{
		if (!m_root_node)
			return -1;
		return (m_root_node - base_node);
	}
	void recoverRoot(Node *base_node, size_t rootIndex)
	{
		if (rootIndex != -1)
			m_root_node = base_node + rootIndex;
	}*/

	std::pair<iterator, bool> insert(Node *newNode)
	{
		assert(newNode && !is_linked(newNode));
		insert_info data(newNode);
		/*m_root_node = */avl_tree_insert(m_root_node, data);
		if (data.oldNode)
			return std::make_pair(iterator(data.oldNode), false);
		return std::make_pair(iterator(newNode), true);
	}

	std::pair<iterator, bool> insert_unique(Node *newNode)
	{
		return insert(newNode);
	}

	iterator last() const//stl map doesn't have it
	{
		return iterator(maxValueNode());
	}

	const_iterator clast() const//stl map doesn't have it
	{
		return const_iterator(maxValueNode());
	}

private:
	Node * minValueNode() const
	{
		return minValueNode(m_root_node);
	}

	Node * maxValueNode() const
	{
		return maxValueNode(m_root_node);
	}

	/* Given a non-empty binary search tree, return the
	   node with minimum key value found in that tree.
	   Note that the entire tree does not need to be
	   searched. */
	static Node * minValueNode(Node* node)
	{
		Node* current = node;
		if (current)
		{
			// loop down to find the leftmost leaf
			while (current->children[NODE_LEFT] != nullptr)
				current = current->children[NODE_LEFT];
		}
		return current;
	}

	static  Node * maxValueNode(Node* node)
	{
		Node* current = node;
		if (current)
		{
			// loop down to find the rightmost leaf
			while (current->children[NODE_RIGHT] != nullptr)
				current = current->children[NODE_RIGHT];
		}
		return current;
	}

	static  Node * inOrderSuccessor(Node *n)
	{
		if (n->children[NODE_RIGHT] != 0)
			return minValueNode(n->children[NODE_RIGHT]);
		  
		Node *p = n->parent;
		while (p != 0 && n == p->children[NODE_RIGHT])
		{
			n = p;
			p = p->parent;
		}
		return p;
	}

	static  Node* inOrderPredecessor(Node* n)
	{
		if (n->children[NODE_LEFT])
			return maxValueNode(n->children[NODE_LEFT]);

		Node* p = n->parent;
		while (p != 0 && n == p->children[NODE_LEFT])
		{
			n = p;
			p = p->parent;
		}
		return p;
	}

	void avl_tree_free_subtree(Node *node)
	{
		if (node == nullptr)
			return;

		avl_tree_free_subtree(node->children[NODE_LEFT]);
		avl_tree_free_subtree(node->children[NODE_RIGHT]);

		free(node);
	}

	void avl_tree_free()
	{
		// Destroy all nodes

		avl_tree_free_subtree(m_root_node);

		// Free back the main tree data structure

		free(m_root_node);
	}

	static int avl_tree_subtree_height(Node *node)
	{
		if (node == nullptr)
			return 0;
		return node->height;
	}

	/* Update the "height" variable of a node, from the heights of its
	* children.  This does not update the height variable of any parent
	* nodes. */

	static void avl_tree_update_height(Node *node)
	{
		Node *left_subtree = node->children[NODE_LEFT];
		Node *right_subtree = node->children[NODE_RIGHT];
		int left_height = avl_tree_subtree_height(left_subtree);
		int right_height = avl_tree_subtree_height(right_subtree);

		if (left_height > right_height)
			node->height = left_height + 1;
		else
			node->height = right_height + 1;
	}

	// Find what side a node is relative to its parent 

	static  AVLTreeNodeSide avl_tree_node_parent_side(Node *node)
	{
		if (node->parent->children[NODE_LEFT] == node)
			return NODE_LEFT;
		return NODE_RIGHT;
	}

	// Replace node1 with node2 at its parent.
	void avl_tree_node_replace(Node *node1, Node *node2)
	{
		int side;

		// Set the node's parent pointer.

		if (node2 != nullptr)
			node2->parent = node1->parent;

		// The root node? 

		if (node1->parent == nullptr)
		{
			m_root_node = node2;
		}
		else
		{
			side = avl_tree_node_parent_side(node1);
			node1->parent->children[side] = node2;

			avl_tree_update_height(node1->parent);
		}
	}

	/* Rotate a section of the tree.  'node' is the node at the top
	* of the section to be rotated.  'direction' is the direction in
	* which to rotate the tree: left or right, as shown in the following
	* diagram:
	*
	* Left rotation:              Right rotation:
	*
	*      B                             D
	*     / \                           / \
	*    A   D                         B   E
	*       / \                       / \
	*      C   E                     A   C
	
	* is rotated to:              is rotated to:
	*
	*        D                           B
	*       / \                         / \
	*      B   E                       A   D
	*     / \                             / \
	*    A   C                           C   E
	*/

	Node *avl_tree_rotate(Node *node, AVLTreeNodeSide direction)
	{
		Node *new_root;

		/* The child of this node will take its place:
		   for a left rotation, it is the right child, and vice versa. */

		new_root = node->children[1 - direction];

		/* Make new_root the root, update parent pointers. */

		avl_tree_node_replace(node, new_root);

		/* Rearrange pointers */

		node->children[1 - direction] = new_root->children[direction];
		new_root->children[direction] = node;

		/* Update parent references */

		node->parent = new_root;

		if (node->children[1 - direction] != nullptr) {
			node->children[1 - direction]->parent = node;
		}

		/* Update heights of the affected nodes */

		avl_tree_update_height(new_root);
		avl_tree_update_height(node);

		return new_root;
	}

	/* Balance a particular tree node.
	 *
	 * Returns the root node of the new subtree which is replacing the
	 * old one. */

	Node *avl_tree_node_balance(Node *node)
	{
		Node *child;

		Node *left_subtree = node->children[NODE_LEFT];
		Node *right_subtree = node->children[NODE_RIGHT];

		/* Check the heights of the child trees.  If there is an unbalance
		 * (difference between left and right > 2), then rotate nodes
		 * around to fix it */

		int diff = avl_tree_subtree_height(right_subtree) - avl_tree_subtree_height(left_subtree);

		if (diff >= 2) 
		{

			/* Biased toward the right side too much. */

			child = right_subtree;

			if (avl_tree_subtree_height(child->children[NODE_RIGHT])
				< avl_tree_subtree_height(child->children[NODE_LEFT])) {

				/* If the right child is biased toward the left
				 * side, it must be rotated right first (double
				 * rotation) */

				avl_tree_rotate(right_subtree, NODE_RIGHT);
			}

			/* Perform a left rotation.  After this, the right child will
			 * take the place of this node.  Update the node pointer. */

			node = avl_tree_rotate(node, NODE_LEFT);

		}
		else if (diff <= -2) {

			/* Biased toward the left side too much. */

			child = node->children[NODE_LEFT];

			if (avl_tree_subtree_height(child->children[NODE_LEFT])
				< avl_tree_subtree_height(child->children[NODE_RIGHT])) {

				/* If the left child is biased toward the right
				 * side, it must be rotated right left (double
				 * rotation) */

				avl_tree_rotate(left_subtree, NODE_LEFT);
			}

			/* Perform a right rotation.  After this, the left child will
			 * take the place of this node.  Update the node pointer. */

			node = avl_tree_rotate(node, NODE_RIGHT);
		}

		/* Update the height of this node */

		avl_tree_update_height(node);

		return node;
	}

	/* Walk up the tree from the given node, performing any needed rotations */

	void avl_tree_balance_to_root(Node *node)
	{
		Node *rover;

		rover = node;

		while (rover != nullptr) {

			/* Balance this node if necessary */

			rover = avl_tree_node_balance(rover);

			/* Go to this node's parent */

			rover = rover->parent;
		}
	}

	Node *avl_tree_insert(Node* node, insert_info &invariant)//AVLTree *tree, AVLTreeKey key, AVLTreeValue value)
	{
		//const AVLTreeKey &key(invariant.newNode->key);

		// Walk down the tree until we reach a nullptr pointer

		Node **rover = &m_root_node;
		Node *previous_node = nullptr;

		while (*rover != nullptr)
		{
			previous_node = *rover;
			//if (tree->compare_func(key, (*rover)->key) < 0)
			int iComp = invariant.newNode->compare((*rover)->key);
			//int iComp = key.compare((*rover)->key);
			//if (key < (*rover)->key)
			if (iComp < 0)
			{
				rover = &((*rover)->children[NODE_LEFT]);
			}
			//else if (key > (*rover)->key)
			else if (iComp > 0)
			{
				rover = &((*rover)->children[NODE_RIGHT]);
			}
			else
			{
				invariant.oldNode = (*rover);
				return nullptr;
			}
		}

		// Create a new node.  Use the last node visited as the parent link.
		Node *new_node = invariant.newNode;
		//Node *new_node = (Node *)malloc(sizeof(Node));

		if (new_node == nullptr)
			return nullptr;

		new_node->children[NODE_LEFT] = nullptr;
		new_node->children[NODE_RIGHT] = nullptr;
		new_node->parent = previous_node;
		//new_node->key = key;
		//new_node->value = value;
		new_node->height = 1;

		// Insert at the nullptr pointer that was reached

		*rover = new_node;

		// Rebalance the tree, starting from the previous node.

		avl_tree_balance_to_root(previous_node);

		// Keep track of the number of entries

		++m_num_nodes;

		return new_node;
	}

	/* Find the nearest node to the given node, to replace it.
	* The node returned is unlinked from the tree.
	* Returns nullptr if the node has no children. */

	Node *avl_tree_node_get_replacement(Node *node)
	{
		Node *left_subtree = node->children[NODE_LEFT];
		Node *right_subtree = node->children[NODE_RIGHT];

		/* No children? */

		if (left_subtree == nullptr && right_subtree == nullptr)
			return nullptr;

		/* Pick a node from whichever subtree is taller.  This helps to
		 * keep the tree balanced. */

		int left_height = avl_tree_subtree_height(left_subtree);
		int right_height = avl_tree_subtree_height(right_subtree);

		int side;
		if (left_height < right_height)
			side = NODE_RIGHT;
		else
			side = NODE_LEFT;

		/* Search down the tree, back towards the center. */

		Node *result = node->children[side];

		while (result->children[1 - side] != nullptr)
			result = result->children[1 - side];

		/* Unlink the result node, and hook in its remaining child
		 * (if it has one) to replace it. */

		Node *child = result->children[side];
		avl_tree_node_replace(result, child);

		/* Update the subtree height for the result node's old parent. */

		avl_tree_update_height(result->parent);

		return result;
	}

	/* Remove a node from a tree */

	void avl_tree_remove_node(delete_info &invariant)
	{
		Node *node(invariant.node);

		Node *balance_startpoint;

		/* The node to be removed must be swapped with an "adjacent"
		 * node, ie. one which has the closest key to this one. Find
		 * a node to swap with. */

		Node *swap_node = avl_tree_node_get_replacement(node);

		if (swap_node == nullptr)
		{

			// This is a leaf node and has no children, therefore  it can be immediately removed.

			// Unlink this node from its parent.

			avl_tree_node_replace(node, nullptr);

			// Start rebalancing from the parent of the original node

			balance_startpoint = node->parent;

		}
		else 
		{
			/* We will start rebalancing from the old parent of the
			 * swap node.  Sometimes, the old parent is the node we
			 * are removing, in which case we must start rebalancing
			 * from the swap node. */

			if (swap_node->parent == node)
				balance_startpoint = swap_node;
			else
				balance_startpoint = swap_node->parent;

			// Copy references in the node into the swap node

			for (int i = 0; i < 2; ++i)
			{
				swap_node->children[i] = node->children[i];

				if (swap_node->children[i] != nullptr)
					swap_node->children[i]->parent = swap_node;
			}

			swap_node->height = node->height;

			// Link the parent's reference to this node

			avl_tree_node_replace(node, swap_node);
		}

		// Destroy the node

//!		free(node);

		// Keep track of the number of nodes

		--m_num_nodes;

		// Rebalance the tree

		avl_tree_balance_to_root(balance_startpoint);
	}

	// Remove a node by key

	int avl_tree_remove(delete_info &invariant)//AVLTreeKey key)
	{
		// Find the node to remove

		Node *node = avl_tree_lookup_node(invariant.key);

		if (node == nullptr)
			return 0;// Not found in tree

		// Remove the node

		invariant.node = node;
		avl_tree_remove_node(invariant);

		return 1;
	}

	Node *avl_tree_lookup_node(const AVLTreeKey &key) const
	{
		// Search down the tree and attempt to find the node which
		// has the specified key

		Node *node = m_root_node;

		while (node != nullptr)
		{
			int iComp = node->compare(key);
			//if (key == node->key)
			if (iComp == 0)
			{
				// Keys are equal: return this node
				return node;
			}

			//if (key < node->key)
			//if (node->key > key)
			if (iComp > 0)
				node = node->children[NODE_LEFT];
			else
				node = node->children[NODE_RIGHT];
		}

		// Not found

		return nullptr;
	}

	Node *avl_tree_lookup_node_ncase(const AVLTreeKey &key) const
	{
		// Search down the tree and attempt to find the node which
		// has the specified key

		Node *node = m_root_node;

		while (node != nullptr)
		{
			int iComp = node->compare_ncase(key);
			//if (key == node->key)
			if (iComp == 0)
			{
				// Keys are equal: return this node
				return node;
			}

			//if (key < node->key)
			//if (node->key > key)
			if (iComp > 0)
				node = node->children[NODE_LEFT];
			else
				node = node->children[NODE_RIGHT];
		}

		// Not found

		return nullptr;
	}

	// Find the node
	Node *avl_tree_lookup(const AVLTreeKey &key) const
	{
		return avl_tree_lookup_node(key);
	}

	Node *avl_tree_lookup_ncase(const AVLTreeKey &key) const
	{
		return avl_tree_lookup_node_ncase(key);
	}

	Node *avl_tree_root_node() const
	{
		return m_root_node;
	}

	static const AVLTreeKey &avl_tree_node_key(Node *node)
	{
		return node->key;
	}

	/*AVLTreeValue avl_tree_node_value(Node *node)
	{
		return node->value;
	}*/

	static Node *avl_tree_node_child(Node *node, AVLTreeNodeSide side)
	{
		if (side == NODE_LEFT || side == NODE_RIGHT)
			return node->children[side];
		return nullptr;
	}

	static Node *avl_tree_node_parent(Node *node)
	{
		return node->parent;
	}
	
	int avl_tree_num_entries() const
	{
		return m_num_nodes;
	}

	/*static void avl_tree_to_array_add_subtree(Node *subtree, AVLTreeValue *array, int *index)
	{
		if (subtree == nullptr)
			return;

		// Add left subtree first

		avl_tree_to_array_add_subtree(subtree->children[NODE_LEFT], array, index);

		// Add this node

		array[*index] = subtree->key;
		++*index;

		// Finally add right subtree

		avl_tree_to_array_add_subtree(subtree->children[NODE_RIGHT], array, index);
	}

	AVLTreeValue *avl_tree_to_array(AVLTree *tree)
	{
		// Allocate the array

		AVLTreeValue *array = malloc(sizeof(AVLTreeValue)* tree->m_num_nodes);

		if (array == nullptr)
			return nullptr;

		int index = 0;

		// Add all keys

		avl_tree_to_array_add_subtree(tree->m_root_node, array, &index);

		return array;
	}*/
};





