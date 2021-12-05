#pragma once

#include <algorithm>//max
#include <string.h>//strcmp

//////////////////////////////////////////////////////////AVL_Tree

struct const_char_ptr
{
	const char *mpc;
	const_char_ptr() : mpc(0){}
	const_char_ptr(const char *pc) : mpc(pc){}
	const_char_ptr(const const_char_ptr &o) : mpc(o.mpc){}
	const_char_ptr &operator=(const const_char_ptr &o){
		mpc = o.mpc;
		return *this;
	}
	operator bool() const { return mpc != 0; }
	bool operator<(const const_char_ptr &o) const {
		return strcmp(mpc, o.mpc) < 0;
	}
	bool operator==(const const_char_ptr &o) const {
		return strcmp(mpc, o.mpc) == 0;
	}
	bool operator>(const const_char_ptr &o) const {
		return strcmp(mpc, o.mpc) > 0;
	}
};

// An AVL tree node
template <typename KEY, typename VALUE>
struct AVL_Tree_Node
{
	typedef	KEY		Key;
	typedef	VALUE	Value;

	AVL_Tree_Node() : key(KEY()), parent(0), left(0), right(0), height(0), value(VALUE()){}
	AVL_Tree_Node(KEY k, VALUE v) : key(k), parent(0), left(0), right(0), height(0), value(v){}
	KEY key;
	struct AVL_Tree_Node *parent;
	struct AVL_Tree_Node *left;
	struct AVL_Tree_Node *right;
	int height;
	VALUE	value;
};

template <typename NODE>
class AVL_Tree
{
	typedef	NODE	Node;
	typedef	typename Node::Key	KEY;
	typedef typename Node::Value	VALUE;

	Node *mpRoot;

public:
	AVL_Tree()
		: mpRoot(nullptr)
	{
	}

	~AVL_Tree()
	{
		assert(empty());
		//clear();
	}

	typedef std::pair<KEY, VALUE>	KeyValPair;
	typedef void (*TraverseCalback)(const KEY &, VALUE &, void *user);
	typedef std::pair<TraverseCalback, void *>	TraverseInfo;

	//iterators
public:
	class iterator
	{
		Node	*node;
	public:
		iterator() : node(nullptr){}
		iterator(Node *p) : node(p){}
		iterator &operator=(const iterator &o){
			node = o.node;
			return *this;
		}
		Node *pvt(){ return node; }
		const Node *pvt() const { return node; }
		iterator &operator++(){
			node = AVL_Tree::inOrderSuccessor(node);
			return *this;
		}
		iterator &operator--(){
			node = AVL_Tree::inOrderPredecessor(node);
			return *this;
		}
		iterator& operator ++(int){ return operator++(); }
		bool operator==(const iterator &o) const {
			return node == o.node;
		}
		bool operator!=(const iterator &o) const { return !operator==(o); }
	};

	class const_iterator
	{
		Node	*node;
	public:
		const_iterator() : node(nullptr){}
		const_iterator(Node *p) : node(p){}
		const_iterator(const iterator &o) : node((Node *)o.pvt()){}
		Node *pvt(){ return node; }
		const Node *pvt() const { return node; }
		const_iterator &operator++(){
			assert(0);
			return *this;
		}
		const_iterator &operator--(){
			assert(0);
			return *this;
		}
		const_iterator& operator ++(int){ return operator++(); }
		bool operator==(const const_iterator &o) const {
			return node == o.node;
		}
		bool operator!=(const const_iterator &o) const { return !operator==(o); }
	};

	class reverse_iterator
	{
		Node	*node;
	public:
		reverse_iterator() : node(nullptr){}
		reverse_iterator(Node *p) : node(p){}
		Node *pvt(){ return node; }
		const Node *pvt() const { return node; }
		reverse_iterator &operator++(){
			return *this;
		}
		reverse_iterator& operator ++(int){ return operator++(); }
		bool operator==(const reverse_iterator &o) const {
			return node == o.node;
		}
		bool operator!=(const reverse_iterator &o) const { return !operator==(o); }
	};

	iterator begin() const {
		return iterator(minValueNode(mpRoot));
	}
	iterator end() const {
		return iterator();
	}

	reverse_iterator rbegin() const {
		return reverse_iterator(maxValueNode(mpRoot));
	}
	reverse_iterator rend() const {
		return reverse_iterator();
	}

	size_t size() const {
		assert(0);
		return 0;
	}

	iterator lower_bound(KEY) const {
		assert(0);
		return iterator();
	}
	iterator upper_bound(KEY) const {
		assert(0);
		return iterator();
	}

public:
	bool empty() const
	{
		return mpRoot == nullptr;
	}

	struct insert_info
	{
		Node *	newNode;
		Node *	oldNode;
	};
	
	std::pair<iterator, bool> insert(Node *newNode)
	{
		assert(newNode && !isLinked(newNode));
		insert_info data;
		data.newNode = newNode;
		data.oldNode = 0;
		mpRoot = insert(mpRoot, data);
		if (data.oldNode)
			return std::make_pair(iterator(data.oldNode), false);
		return std::make_pair(iterator(newNode), true);
	}

	struct delete_info
	{
		KEY key;
		Node *	node;
	};

	/*void erase(const KEY &key)
	{
		delete_info data;
		data.key = key;
		data.node = 0;
		mpRoot = deleteNode(mpRoot, data);
	}

	void erase(iterator i)
	{
		delete_info data;
		data.key = i.pvt()->key;
		data.node = i.pvt();
		mpRoot = deleteNode(mpRoot, data);
	}*/

	Node *take(const KEY &key)
	{
		delete_info data;
		data.key = key;
		data.node = 0;
		mpRoot = deleteNode(mpRoot, data);
		return data.node;
	}

	Node *take(iterator i)
	{
		delete_info data;
		data.key = i.pvt()->key;
		data.node = 0;// i.pvt();
		mpRoot = deleteNode(mpRoot, data);
		return data.node;
	}

	iterator find(const KEY &key) const
	{
		return iterator(find(mpRoot, key));
	}

	void clear()
	{
		assert(0);
		clear(mpRoot);
		mpRoot = nullptr;
	}

private:
	void traverse(const TraverseInfo &ti) const
	{
		traverse(mpRoot, ti);
	}

	void traverse_from(const KEY &key, const TraverseInfo &ti) const
	{
		KEY key2(key);
		traverse_from(mpRoot, key2, ti);
	}
	
	bool isLinked(Node *node) const
	{
		return (node->parent || node->left || node->right);
	}

	void clear(Node *leaf)
	{
		if (leaf != nullptr)
		{
			clear(leaf->left);
			clear(leaf->right);
			free(leaf);
		}
	}

	// A utility function to get height of the tree
	int height(Node *N) const
	{
		if (N == nullptr)
			return 0;
		return N->height;
	}

	/* Helper function that allocates a new node with the given key and
		nullptr left and right pointers. */
	/*Node* newNode(const KeyValPair &kv) const
	{
		Node* node = (Node*)malloc(sizeof(Node));
		node->key = kv.first;
		node->left = nullptr;
		node->right = nullptr;
		node->height = 1;  // new node is initially added at leaf
		node->value = kv.second;
		return(node);
	}*/

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
			while (current->left != nullptr)
				current = current->left;
		}
		return current;
	}

	static Node * maxValueNode(Node* node)
	{
		Node* current = node;
		if (current)
		{
			// loop down to find the rightmost leaf
			while (current->right != nullptr)
				current = current->right;
		}
		return current;
	}

	static Node * inOrderSuccessor(Node *n)
	{
		if (n->right != 0)
			return minValueNode(n->right);

		Node *p = n->parent;
		while (p != 0 && n == p->right)
		{
			n = p;
			p = p->parent;
		}
		return p;
	}

	static Node* inOrderPredecessor(Node* n)
	{
		if (n->left)
			return maxValueNode(n->left);

		Node* p = n->parent;
		while (p != 0 && n == p->left)
		{
			n = p;
			p = p->parent;
		}
		return p;
	}

    /* T1, T2 and T3 are subtrees of the tree rooted with y(on left side) or x(on right side)
         y                           x
        / \     Right Rotation      / \
       x   T3      –––––––>       T1   y
      / \          <-------           / \
    T1  T2     Left Rotation         T2  T3
   
      Keys in both of the above trees follow the following order keys(T1) < key(x) < keys(T2) < key(y) < keys(T3)
      So BST property is not violated anywhere.
   */

	// A utility function to right rotate subtree rooted with y
	// See the diagram given above.
	Node *rightRotate(Node *y) const
	{
		Node *x = y->left;
		Node *T2 = x->right;

		// Perform rotation
		x->right = y;
		x->parent = y->parent;
		y->left = T2;
		y->parent = x;
		T2->parent = y;

		// Update heights
		y->height = std::max(height(y->left), height(y->right)) + 1;
		x->height = std::max(height(x->left), height(x->right)) + 1;

		// Return new root
		return x;
	}

	// A utility function to left rotate subtree rooted with x
	// See the diagram given above.
	Node *leftRotate(Node *x) const
	{
		Node *y = x->right;
		Node *T2 = y->left;

		// Perform rotation
		y->left = x;
		y->parent = x->parent;
		x->right = T2;
		x->parent = y;
		T2->parent = x;

		//  Update heights
		x->height = std::max(height(x->left), height(x->right)) + 1;
		y->height = std::max(height(y->left), height(y->right)) + 1;

		// Return new root
		return y;
	}

	// Get Balance factor of node N
	int getBalance(Node *N) const
	{
		if (N == nullptr)
			return 0;
		return height(N->left) - height(N->right);
	}

	//Node* insert(Node* node, const KeyValPair &kv)
	Node* insert(Node* node, insert_info &data)
	{
		/* 1.  Perform the normal BST rotation */
		if (node == nullptr)
			return data.newNode;

		KEY key(data.newNode->key);

		if (key < node->key)
		{
			node->left = insert(node->left, data);
			node->left->parent = node;
		}
		else if (key > node->key)
		{
			node->right = insert(node->right, data);
			node->right->parent = node;
		}
		else // Equal keys not allowed
		{
			data.oldNode = node;
			return node;
		}

		/* 2. Update height of this ancestor node */
		node->height = 1 + std::max(height(node->left), height(node->right));

		/* 3. Get the balance factor of this ancestor
			  node to check whether this node became
			  unbalanced */
		int balance = getBalance(node);

		// If this node becomes unbalanced, then there are 4 cases

		// Left Left Case
		if (balance > 1 && key < node->left->key)
			return rightRotate(node);

		// Right Right Case
		if (balance < -1 && key > node->right->key)
			return leftRotate(node);

		// Left Right Case
		if (balance > 1 && key > node->left->key)
		{
			node->left = leftRotate(node->left);
			return rightRotate(node);
		}

		// Right Left Case
		if (balance < -1 && key < node->right->key)
		{
			node->right = rightRotate(node->right);
			return leftRotate(node);
		}

		/* return the (unchanged) node pointer */
		return node;
	}

	// Recursive function to delete a node with given key
	// from subtree with given root. It returns root of
	// the modified subtree.
	Node* deleteNode(Node* root, delete_info &data)
	{
		// STEP 1: PERFORM STANDARD BST DELETE

		if (root == nullptr)
			return root;

		// If the key to be deleted is smaller than the
		// root's key, then it lies in left subtree
		if (data.key < root->key)
		{
			root->left = deleteNode(root->left, data);
			if (root->left)
				root->left->parent = root;
		}

		// If the key to be deleted is greater than the
		// root's key, then it lies in right subtree
		else if (data.key > root->key)
		{
			root->right = deleteNode(root->right, data);
			if (root->right)
				root->right->parent = root;
		}

		// if key is same as root's key, then This is the node to be deleted
		else
		{
			assert(!data.node);
			data.node = root;

			Node *parent = root->parent;
			Node *right = root->right;
			Node *left = root->left;
			root->parent = root->right = root->left = nullptr;//disconnect

			// node with only one child or no child
			if (left == nullptr)
			{
				if (parent)
				{
					if (root == parent->left)
						parent->left = right;
					else
						parent->right = right;
				}
				if (right == nullptr)
					return nullptr;//No child case
				// One child case
				right->parent = parent;
				root = right;
			}
			else if (right == nullptr)
			{
				if (parent)
				{
					if (root == parent->left)
						parent->left = left;
					else
						parent->right = left;
				}
				if (left == nullptr)
					return nullptr;//No child case
				// One child case
				left->parent = parent;
				root = left;
			}
			else
			{
				// node with two children: Get the inorder
				// successor (smallest in the right subtree)
				Node* temp = minValueNode(root->right);

				// Copy the inorder successor's data to this node
				root->key = temp->key;

				// Delete the inorder successor
				data.key = temp->key;
				root->right = deleteNode(root->right, data);
			}
		}

		// If the tree had only one node then return
		assert(root != nullptr);
		//if (root == nullptr)
			//return root;

		// STEP 2: UPDATE HEIGHT OF THE CURRENT NODE
		root->height = 1 + std::max(height(root->left), height(root->right));

		// STEP 3: GET THE BALANCE FACTOR OF THIS NODE (to
		// check whether this node became unbalanced)
		int balance = getBalance(root);

		// If this node becomes unbalanced, then there are 4 cases

		// Left Left Case
		if (balance > 1 && getBalance(root->left) >= 0)
			return rightRotate(root);

		// Left Right Case
		if (balance > 1 && getBalance(root->left) < 0)
		{
			root->left = leftRotate(root->left);
			root->left->parent = root;
			return rightRotate(root);
		}

		// Right Right Case
		if (balance < -1 && getBalance(root->right) <= 0)
			return leftRotate(root);

		// Right Left Case
		if (balance < -1 && getBalance(root->right) > 0)
		{
			root->right = rightRotate(root->right);
			root->right->parent = root;
			return leftRotate(root);
		}

		return root;
	}

	Node *find(Node *root, const KEY &key) const
	{
		if (root)
		{
			if (key == root->key)
				return root;
			if (key < root->key)
				return find(root->left, key);
			return find(root->right, key);
		}
		return nullptr;
	}

	// A utility function to print preorder traversal of the tree.
	// The function also prints height of every node
	void traverse(Node *root, const TraverseInfo &ti) const
	{
		if (root != nullptr)
		{
			traverse(root->left, ti);
			(*ti.first)(root->key, root->value, ti.second);
			traverse(root->right, ti);
		}
	}

	bool traverse_from(Node *root, KEY &key, const TraverseInfo &ti) const
	{
		if (root != nullptr)
		{
			if (key)
			{
				if (!(key < root->key))
				{
					if (key > root->key)
						return false;
					key = KEY();
					(*ti.first)(root->key, root->value, ti.second);
					traverse(root->right, ti);
					return true;
				}
			}
			if (key)
			{
				if (key < root->key)
				{
					if (!traverse_from(root->left, key, ti))
						return false;
					(*ti.first)(root->key, root->value, ti.second);
					traverse(root->right, ti);
					return true;
				}
				if (!traverse_from(root->right, key, ti))
					return false;
				return true;
			}

			traverse(root->left, ti);
			(*ti.first)(root->key, root->value, ti.second);
			traverse(root->right, ti);
		}
		return true;
	}
};
