#pragma once

#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <assert.h>

namespace sbtree
{

	template <class self_t, typename key_t>//why key_t?
	struct node_s
	{
		typedef self_t self_type;
		typedef self_type value_type;
		//typedef typename self_type::key_type key_type;
		typedef key_t key_type;
		key_type key;
		self_type *parent;
		self_type *left;
		self_type *right;
		size_t size;
		node_s(key_type _key)
			: key(_key),
			parent(nullptr),
			left(nullptr),
			right(nullptr),
			size(0)
		{
		}
		node_s(node_s&& o)
			: key(o.key),
			parent(o.parent),
			left(o.left),
			right(o.right),
			size(o.size)
		{
			o.clear();
		}
		node_s()
			: node_s(key_type())
		{
		}
		node_s& operator=(const node_s& o)
		{
			key = o.key;
			parent = o.parent;
			left = o.left;
			right = o.right;
			size = o.size;
			return *this;
		}
		node_s& operator=(node_s&& o)
		{
			key = o.key;
			parent = o.parent;
			left = o.left;
			right = o.right;
			size = o.size;
			o.clear();
			return *this;
		}
		void clear()
		{
			key = key_type();
			parent = nullptr;
			left = nullptr;
			right = nullptr;
			size = 0;
		}
		void overrideKey(const key_type &a)//USE WITH CAUTION!
		{
			assert(!left && !right && !parent);
			const_cast<key_type &>(key) = a;
		}
		void reset()
		{
			parent = left = right = nullptr;
			size = 0;
		}
		inline bool isEmpty() const { return size == 0;}
		inline size_t getSize() const { return size; }
		inline void setSize(size_t n){ size = n; }
	};

	template<class node_t, class comparator_t>
	struct head_s : public comparator_t
	{
		template<class any_key_compare>
		head_s(any_key_compare &&comp)
			: comparator_t(std::forward<any_key_compare>(comp)),
			root(nullptr)
		{
		}
		head_s& operator=(head_s&& other){
			if (this != &other)
			{
				std::swap(root, other.root);
			}
			return *this;
		}
		node_t *root;
	};


	template<class sbtree_config_t>
	class size_balanced_tree : public sbtree_config_t
	{
		template <typename T, typename TD> friend class deallocator_t;
	private:
		using sbtree_config_t::is_nil_;
		using sbtree_config_t::nil_;
		using sbtree_config_t::get_comparator_;
		using sbtree_config_t::get_most_left_;
		using sbtree_config_t::get_most_right_;
		using sbtree_config_t::get_root_;
		using sbtree_config_t::get_key_;
		using sbtree_config_t::get_parent_;
		using sbtree_config_t::get_left_;
		using sbtree_config_t::get_right_;
		using sbtree_config_t::get_size_;
		using sbtree_config_t::set_root_;
		using sbtree_config_t::set_parent_;
		using sbtree_config_t::set_size_;
		using sbtree_config_t::set_left_;
		using sbtree_config_t::set_right_;
		using sbtree_config_t::set_most_left_;
		using sbtree_config_t::set_most_right_;

	public:
		typedef typename sbtree_config_t::node_type	node_t;
		typedef typename sbtree_config_t::key_compare	key_compare;

		typedef typename node_t::key_type key_type;
		typedef typename node_t::value_type value_type;

		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;
		typedef node_t &reference;
		typedef node_t const &const_reference;
		typedef node_t *pointer;
		typedef node_t const *const_pointer;

	public:
		class iterator
		{
		public:
			typedef std::random_access_iterator_tag iterator_category;
			typedef typename size_balanced_tree::value_type value_type;
			typedef typename size_balanced_tree::difference_type difference_type;
			typedef typename size_balanced_tree::reference reference;
			typedef typename size_balanced_tree::pointer pointer;
		public:
			explicit iterator(node_t *in_node) : node(in_node)
			{
			}
			iterator(iterator const &) = default;
			iterator &operator += (difference_type diff)
			{
				node = size_balanced_tree::sbt_advance_(node, diff);
				return *this;
			}
			iterator &operator -= (difference_type diff)
			{
				node = size_balanced_tree::sbt_advance_(node, -diff);
				return *this;
			}
			iterator operator + (difference_type diff) const
			{
				return iterator(size_balanced_tree::sbt_advance_(node, diff));
			}
			iterator operator - (difference_type diff) const
			{
				return iterator(size_balanced_tree::sbt_advance_(node, -diff));
			}
			difference_type operator - (iterator const &other) const
			{
				return static_cast<difference_type>(size_balanced_tree::sbt_rank_(node)) - static_cast<difference_type>(size_balanced_tree::sbt_rank_(other.node));
			}
			iterator &operator++()
			{
				node = size_balanced_tree::bst_move_<true>(node);
				return *this;
			}
			iterator &operator--()
			{
				node = size_balanced_tree::bst_move_<false>(node);
				return *this;
			}
			iterator operator++(int)
			{
				iterator save(*this);
				++*this;
				return save;
			}
			iterator operator--(int)
			{
				iterator save(*this);
				--*this;
				return save;
			}
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
			reference operator[](difference_type index) const
			{
				return *(*this + index);
			}
			bool operator > (iterator const &other) const
			{
				return *this - other > 0;
			}
			bool operator < (iterator const &other) const
			{
				return *this - other < 0;
			}
			bool operator >= (iterator const &other) const
			{
				return *this - other >= 0;
			}
			bool operator <= (iterator const &other) const
			{
				return *this - other <= 0;
			}
			bool operator == (iterator const &other) const
			{
				return node == other.node;
			}
			bool operator != (iterator const &other) const
			{
				return node != other.node;
			}
		private:
			friend class size_balanced_tree;
			node_t *node;
		};
		class const_iterator
		{
		public:
			typedef std::random_access_iterator_tag iterator_category;
			typedef typename size_balanced_tree::value_type value_type;
			typedef typename size_balanced_tree::difference_type difference_type;
			typedef typename size_balanced_tree::reference reference;
			typedef typename size_balanced_tree::const_reference const_reference;
			typedef typename size_balanced_tree::pointer pointer;
			typedef typename size_balanced_tree::const_pointer const_pointer;
		public:
			explicit const_iterator(const node_t *in_node) : node((node_t* )in_node)
			{
			}
			const_iterator(iterator const &other) : node(other.node)
			{
			}
			const_iterator(const_iterator const &) = default;
			const_iterator &operator += (difference_type diff)
			{
				node = size_balanced_tree::sbt_advance_(node, diff);
				return *this;
			}
			const_iterator &operator -= (difference_type diff)
			{
				node = size_balanced_tree::sbt_advance_(node, -diff);
				return *this;
			}
			const_iterator operator + (difference_type diff) const
			{
				return const_iterator(size_balanced_tree::sbt_advance_(node, diff));
			}
			const_iterator operator - (difference_type diff) const
			{
				return const_iterator(size_balanced_tree::sbt_advance_(node, -diff));
			}
			difference_type operator - (const_iterator const &other) const
			{
				return static_cast<difference_type>(size_balanced_tree::sbt_rank_(node)) - static_cast<difference_type>(size_balanced_tree::sbt_rank_(other.node));
			}
			const_iterator &operator++()
			{
				node = size_balanced_tree::bst_move_<true>(node);
				return *this;
			}
			const_iterator &operator--()
			{
				node = size_balanced_tree::bst_move_<false>(node);
				return *this;
			}
			const_iterator operator++(int)
			{
				const_iterator save(*this);
				++*this;
				return save;
			}
			const_iterator operator--(int)
			{
				const_iterator save(*this);
				--*this;
				return save;
			}
			const_reference operator *() const
			{
				return *node;
			}
			const_pointer operator->() const
			{
				return node;
			}
			const_reference operator[](difference_type index) const
			{
				return *(*this + index);
			}
			bool operator > (const_iterator const &other) const
			{
				return *this - other > 0;
			}
			bool operator < (const_iterator const &other) const
			{
				return *this - other < 0;
			}
			bool operator >= (const_iterator const &other) const
			{
				return *this - other >= 0;
			}
			bool operator <= (const_iterator const &other) const
			{
				return *this - other <= 0;
			}
			bool operator == (const_iterator const &other) const
			{
				return node == other.node;
			}
			bool operator != (const_iterator const &other) const
			{
				return node != other.node;
			}
		private:
			friend class size_balanced_tree;
			node_t *node;
		};
		class reverse_iterator
		{
		public:
			typedef std::random_access_iterator_tag iterator_category;
			typedef typename size_balanced_tree::value_type value_type;
			typedef typename size_balanced_tree::difference_type difference_type;
			typedef typename size_balanced_tree::reference reference;
			typedef typename size_balanced_tree::pointer pointer;
		public:
			explicit reverse_iterator(node_t *in_node) : node(in_node)
			{
			}
			explicit reverse_iterator(iterator const &other) : node(other.node)
			{
				++*this;
			}
			reverse_iterator(reverse_iterator const &) = default;
			reverse_iterator &operator += (difference_type diff)
			{
				node = size_balanced_tree::sbt_advance_(node, -diff);
				return *this;
			}
			reverse_iterator &operator -= (difference_type diff)
			{
				node = size_balanced_tree::sbt_advance_(node, diff);
				return *this;
			}
			reverse_iterator operator + (difference_type diff) const
			{
				return reverse_iterator(size_balanced_tree::sbt_advance_(node, -diff));
			}
			reverse_iterator operator - (difference_type diff) const
			{
				return reverse_iterator(size_balanced_tree::sbt_advance_(node, diff));
			}
			difference_type operator - (reverse_iterator const &other) const
			{
				return static_cast<difference_type>(size_balanced_tree::sbt_rank_(other.node)) - static_cast<difference_type>(size_balanced_tree::sbt_rank_(node));
			}
			reverse_iterator &operator++()
			{
				node = size_balanced_tree::bst_move_<false>(node);
				return *this;
			}
			reverse_iterator &operator--()
			{
				node = size_balanced_tree::bst_move_<true>(node);
				return *this;
			}
			reverse_iterator operator++(int)
			{
				reverse_iterator save(*this);
				++*this;
				return save;
			}
			reverse_iterator operator--(int)
			{
				reverse_iterator save(*this);
				--*this;
				return save;
			}
			reference operator *() const
			{
				return *node;
			}
			pointer operator->() const
			{
				return node;
			}
			reference & operator[](difference_type index) const
			{
				return *(*this + index);
			}
			bool operator > (reverse_iterator const &other) const
			{
				return *this - other > 0;
			}
			bool operator < (reverse_iterator const &other) const
			{
				return *this - other < 0;
			}
			bool operator >= (reverse_iterator const &other) const
			{
				return *this - other >= 0;
			}
			bool operator <= (reverse_iterator const &other) const
			{
				return *this - other <= 0;
			}
			bool operator == (reverse_iterator const &other) const
			{
				return node == other.node;
			}
			bool operator != (reverse_iterator const &other) const
			{
				return node != other.node;
			}
			iterator base() const
			{
				return ++iterator(node);
			}
		private:
			friend class size_balanced_tree;
			node_t *node;
		};
		class const_reverse_iterator
		{
		public:
			typedef std::random_access_iterator_tag iterator_category;
			typedef typename size_balanced_tree::value_type value_type;
			typedef typename size_balanced_tree::difference_type difference_type;
			typedef typename size_balanced_tree::reference reference;
			typedef typename size_balanced_tree::const_reference const_reference;
			typedef typename size_balanced_tree::pointer pointer;
			typedef typename size_balanced_tree::const_pointer const_pointer;
		public:
			explicit const_reverse_iterator(node_t *in_node) : node(in_node)
			{
			}
			explicit const_reverse_iterator(const_iterator const &other) : node(other.node)
			{
				++*this;
			}
			const_reverse_iterator(reverse_iterator const &other) : node(other.node)
			{
			}
			const_reverse_iterator(const_reverse_iterator const &) = default;
			const_reverse_iterator &operator += (difference_type diff)
			{
				node = size_balanced_tree::sbt_advance_(node, -diff);
				return *this;
			}
			const_reverse_iterator &operator -= (difference_type diff)
			{
				node = size_balanced_tree::sbt_advance_(node, diff);
				return *this;
			}
			const_reverse_iterator operator + (difference_type diff) const
			{
				return const_reverse_iterator(size_balanced_tree::sbt_advance_(node, -diff));
			}
			const_reverse_iterator operator - (difference_type diff) const
			{
				return const_reverse_iterator(size_balanced_tree::sbt_advance_(node, diff));
			}
			difference_type operator - (const_reverse_iterator const &other) const
			{
				return static_cast<difference_type>(size_balanced_tree::sbt_rank_(other.node)) - static_cast<difference_type>(size_balanced_tree::sbt_rank_(node));
			}
			const_reverse_iterator &operator++()
			{
				node = size_balanced_tree::bst_move_<false>(node);
				return *this;
			}
			const_reverse_iterator &operator--()
			{
				node = size_balanced_tree::bst_move_<true>(node);
				return *this;
			}
			const_reverse_iterator operator++(int)
			{
				const_reverse_iterator save(*this);
				++*this;
				return save;
			}
			const_reverse_iterator operator--(int)
			{
				const_reverse_iterator save(*this);
				--*this;
				return save;
			}
			const_reference operator *() const
			{
				return *node;
			}
			const_pointer operator->() const
			{
				return node;
			}
			const_reference operator[](difference_type index) const
			{
				return *(*this + index);
			}
			bool operator > (const_reverse_iterator const &other) const
			{
				return *this - other > 0;
			}
			bool operator < (const_reverse_iterator const &other) const
			{
				return *this - other < 0;
			}
			bool operator >= (const_reverse_iterator const &other) const
			{
				return *this - other >= 0;
			}
			bool operator <= (const_reverse_iterator const &other) const
			{
				return *this - other <= 0;
			}
			bool operator == (const_reverse_iterator const &other) const
			{
				return node == other.node;
			}
			bool operator != (const_reverse_iterator const &other) const
			{
				return node != other.node;
			}
			const_iterator base() const
			{
				return ++iterator(node);
			}
		private:
			friend class size_balanced_tree;
			node_t *node;
		};

	protected:
		//full

		size_balanced_tree(key_compare const &comp)
			: sbtree_config_t(comp)
		{
		}

	public:
		//empty
		size_balanced_tree()
			//: size_balanced_tree(key_compare())
		{
		}

		//range
		/*template <class iterator_t>
		size_balanced_tree(iterator_t begin, iterator_t end, key_compare const &comp = key_compare())
		: size_balanced_tree(comp)
		{
		insert(begin, end);
		}
		//range
		template <class iterator_t>
		size_balanced_tree(iterator_t begin, iterator_t end)
		: size_balanced_tree(key_compare())
		{
		insert(begin, end);
		}*/
		//copy
		/*size_balanced_tree(size_balanced_tree const &other)
			: size_balanced_tree(other.get_comparator_())
			{
			sbt_copy_<std::false_type>(nullptr, other.get_root_());
			}*/
		//move
		size_balanced_tree(size_balanced_tree &&other)
			: size_balanced_tree(key_compare())
		{
			std::swap(this->head_.root, other.head_.root);
		}
		//destructor
		~size_balanced_tree()
		{
		}

		size_balanced_tree& operator=(size_balanced_tree&& other){
			if (this != &other)
			{
				this->head_ = std::move(other.head_);
			}
			return *this;
		}

		void swap(size_balanced_tree &other)
		{
			std::swap(this->head_, other.head_);
		}

		typedef std::pair<iterator, iterator> pair_ii_t;
		typedef std::pair<const_iterator, const_iterator> pair_cici_t;

		iterator insert(node_t *node)
		{
			return iterator(sbt_insert_<false>(node));
		}

		std::pair<iterator, bool> insert_unique(node_t *node)
		{
			iterator lb = lower_bound(node->key);
			if (lb != end() && !(get_comparator_()(node->key, lb->key)))
			{
				// key already exists update lb->second if you care to
				return std::make_pair(lb, false);
			}
			// the key does not exist in the map add it to the map
			return std::make_pair(iterator(sbt_insert_hint_(lb.node, node)), true);
		}

		//with hint
		/*template<class in_value_t>
		typename std::enable_if<std::is_convertible<in_value_t, value_type>::value, iterator>::type insert(const_iterator hint, in_value_t &&value)
		{
		return iterator(sbt_insert_hint_(hint.node, sbt_create_node_(std::forward<in_value_t>(value))));
		}*/
		//range
		template<class iterator_t> void insert(iterator_t begin, iterator_t end)
		{
			for (; begin != end; ++begin)
			{
				emplace_hint(cend(), *begin);
			}
		}
		//initializer list
		/*void insert(std::initializer_list<value_type> il)
		{
		insert(il.begin(), il.end());
		}*/

		//single element
		template<class ...args_t> iterator emplace(args_t &&...args)
		{
			//check_max_size_();
			return iterator(sbt_insert_<false>(sbt_create_node_(std::forward<args_t>(args)...)));
		}
		//with hint
		template<class ...args_t> iterator emplace_hint(const_iterator hint, args_t &&...args)
		{
			//check_max_size_();
			return iterator(sbt_insert_hint_(hint.node, sbt_create_node_(std::forward<args_t>(args)...)));
		}

		void rebind(node_t* from, node_t* to)//quick node replacement (adjustment in tree after a node move)
		{
			//assert(find(from));

			/*to->key = from->key;
			to->parent = from->parent;
			to->left = from->left;
			to->right = from->right;
			to->size = from->size;*/

			if (to->parent != nil_())
			{
				if (to->parent->left == from)
					to->parent->left = to;
				else if (to->parent->right == from)
					to->parent->right = to;
			}
			else
			{
				//for the nil node - the left is a most_left, the right is the modet_right and the parent - the top node
				if (to->parent->parent == from)
					to->parent->parent = to;
			}

			if (to->left->parent == from)
				to->left->parent = to;//otherwise it is nil (a ptr to the root!)
			if (to->right->parent == from)
				to->right->parent = to;

			if (get_most_left_() == from)
				set_most_left_(to);
			if (get_most_right_() == from)
				set_most_right_(to);
		}

		iterator find(key_type const &key)
		{
			node_t *where = bst_lower_bound_(key);
			return (is_nil_(where) || get_comparator_()(key, get_key_(where))) ? iterator(nil_()) : iterator(where);
		}
		const_iterator find(key_type const &key) const
		{
			node_t *where = bst_lower_bound_(key);
			return (is_nil_(where) || get_comparator_()(key, get_key_(where))) ? const_iterator(nil_()) : const_iterator(where);
		}

		node_t *take(iterator &i)
		{
			node_t *p = i.node;
			//const_iterator pos = std::next(where);
			std::advance(i, 1);
			sbt_erase_<false>(p);
			//where = iterator(pos.node);
			p->parent = p->left = p->right = nullptr;
			return p;
		}

		node_t *take(reverse_iterator &i)
		{
			node_t *p = i.node;
			std::advance(i, 1);
			sbt_erase_<false>(p);
			//where = iterator(pos.node);
			p->parent = p->left = p->right = nullptr;
			return p;
		}

		size_type erase(key_type const &key)
		{
			size_type erase_count = 0;
			node_t *where = bst_lower_bound_(key);
			while (!is_nil_(where) && !get_comparator_()(key, get_key_(where)))
			{
				node_t *next = bst_move_<true>(where);
				erase(iterator(where));
				where = next;
				++erase_count;
			}
			return erase_count;
		}
		iterator erase(const_iterator erase_begin, const_iterator erase_end)
		{
			if (erase_begin == cbegin() && erase_end == cend())
			{
				clear();
				return begin();
			}
			else
			{
				while (erase_begin != erase_end)
				{
					erase(erase_begin++);
				}
				return iterator(erase_begin.node);
			}
		}

		size_type count(key_type const &key) const
		{
			pair_cici_t range = equal_range(key);
			return std::distance(range.first, range.second);
		}
		size_type count(key_type const &min, key_type const &max) const
		{
			if (get_comparator_()(max, min))
			{
				return 0;
			}
			return sbt_rank_(bst_upper_bound_(max)) - sbt_rank_(bst_lower_bound_(min));
		}

		pair_ii_t range(key_type const &min, key_type const &max)
		{
			if (get_comparator_()(max, min))
			{
				return pair_ii_t(end(), end());
			}
			return pair_ii_t(iterator(bst_lower_bound_(min)), iterator(bst_upper_bound_(max)));
		}
		pair_cici_t range(key_type const &min, key_type const &max) const
		{
			if (get_comparator_()(max, min))
			{
				return pair_cici_t(cend(), cend());
			}
			return pair_cici_t(const_iterator(bst_lower_bound_(min)), const_iterator(bst_upper_bound_(max)));
		}

		//reverse index when index < 0
		pair_ii_t slice(difference_type slice_begin = 0, difference_type slice_end = 0)
		{
			difference_type s_size = size();
			if (slice_begin < 0)
			{
				slice_begin = std::max<difference_type>(s_size + slice_begin, 0);
			}
			if (slice_end <= 0)
			{
				slice_end = s_size + slice_end;
			}
			if (slice_begin > slice_end || slice_begin >= s_size)
			{
				return pair_ii_t(end(), end());
			}
			return pair_ii_t(at(slice_begin), at(slice_end));
		}
		//reverse index when index < 0
		pair_cici_t slice(difference_type slice_begin = 0, difference_type slice_end = 0) const
		{
			difference_type s_size = size();
			if (slice_begin < 0)
			{
				slice_begin = std::max<difference_type>(s_size + slice_begin, 0);
			}
			if (slice_end <= 0)
			{
				slice_end = s_size + slice_end;
			}
			if (slice_begin > slice_end || slice_begin >= s_size)
			{
				return pair_cici_t(cend(), cend());
			}
			return pair_cici_t(at(slice_begin), at(slice_end));
		}

		iterator lower_bound(key_type const &key)
		{
			return iterator(bst_lower_bound_(key));
		}
		const_iterator lower_bound(key_type const &key) const
		{
			return const_iterator(bst_lower_bound_(key));
		}
		iterator upper_bound(key_type const &key)
		{
			return iterator(bst_upper_bound_(key));
		}
		const_iterator upper_bound(key_type const &key) const
		{
			return const_iterator(bst_upper_bound_(key));
		}

		pair_ii_t equal_range(key_type const &key)
		{
			node_t *lower, *upper;
			std::tie(lower, upper) = bst_equal_range_(key);
			return pair_ii_t(iterator(lower), iterator(upper));
		}
		pair_cici_t equal_range(key_type const &key) const
		{
			node_t *lower, *upper;
			std::tie(lower, upper) = bst_equal_range_(key);
			return pair_cici_t(const_iterator(lower), const_iterator(upper));
		}

		iterator begin()
		{
			return iterator(get_most_left_());
		}
		iterator end()
		{
			return iterator(nil_());
		}
		const_iterator begin() const
		{
			return const_iterator(get_most_left_());
		}
		const_iterator end() const
		{
			return const_iterator(nil_());
		}
		const_iterator cbegin() const
		{
			return const_iterator(get_most_left_());
		}
		const_iterator cend() const
		{
			return const_iterator(nil_());
		}
		reverse_iterator rbegin()
		{
			return reverse_iterator(get_most_right_());
		}
		reverse_iterator rend()
		{
			return reverse_iterator(nil_());
		}
		const_reverse_iterator rbegin() const
		{
			return const_reverse_iterator(get_most_right_());
		}
		const_reverse_iterator rend() const
		{
			return const_reverse_iterator(nil_());
		}
		const_reverse_iterator crbegin() const
		{
			return const_reverse_iterator(get_most_right_());
		}
		const_reverse_iterator crend() const
		{
			return const_reverse_iterator(nil_());
		}

		node_t &front()
		{
			return *get_most_left_();
		}
		node_t &back()
		{
			return *get_most_right_();
		}

		const node_t &front() const
		{
			return *get_most_left_();
		}
		const node_t &back() const
		{
			return *get_most_right_();
		}

		bool empty() const
		{
			return is_nil_(get_root_());
		}
		void clear()
		{
			sbt_clear_(get_root_());
			set_root_(nil_());
		}
		size_type size() const
		{
			return get_size_(get_root_());
		}
		/*size_type max_size() const
		{
		return node_allocator_t(get_node_allocator_()).max_size();
		}*/

		//if(index >= size) return end
		iterator at(size_type index)
		{
			return iterator(sbt_at_(index));
		}
		//if(index >= size) return end
		const_iterator at(size_type index) const
		{
			return const_iterator(sbt_at_(index));
		}

		//rank(begin) == 0, key rank
		size_type rank(key_type const &key) const
		{
			return bst_lower_rank_(key);
		}
		//rank(begin) == 0, rank of iterator
		static size_type rank(const_iterator where)
		{
			return sbt_rank_(where.node);
		}

		//rank(begin) == 0, key rank current best
		size_type lower_rank(key_type const &key) const
		{
			return bst_lower_rank_(key);
		}
		//rank(begin) == 0, key rank when insert
		size_type upper_rank(key_type const &key) const
		{
			return bst_upper_rank_(key);
		}

		iterator last() const//stl map doesn't have it
		{
			return iterator(rbegin().node);
		}

		const_iterator clast() const//stl map doesn't have it
		{
			return const_iterator(rbegin().node);
		}

	protected:
		void sbt_refresh_size_(node_t *node)
		{
			set_size_(node, get_size_(get_left_(node)) + get_size_(get_right_(node)) + 1);
		}

		template<bool is_left>
		/*static*/ void set_child_(node_t *node, node_t *child)
		{
			if (is_left)
			{
				set_left_(node, child);
			}
			else
			{
				set_right_(node, child);
			}
		}

		template<bool is_left>
		static node_t *get_child_(node_t *node)
		{
			if (is_left)
			{
				return get_left_(node);
			}
			else
			{
				return get_right_(node);
			}
		}

		void bst_init_node_(node_t *parent, node_t *node)
		{
			set_parent_(node, parent);
			set_left_(node, nil_());
			set_right_(node, nil_());
			set_size_(node, 1);
		}

		template<bool is_next>
		static node_t *bst_move_(node_t *node)
		{
			if (!is_nil_(node))
			{
				if (!is_nil_(get_child_<!is_next>(node)))
				{
					node = get_child_<!is_next>(node);
					while (!is_nil_(get_child_<is_next>(node)))
					{
						node = get_child_<is_next>(node);
					}
				}
				else
				{
					node_t *parent;
					while (!is_nil_(parent = get_parent_(node)) && node == get_child_<!is_next>(parent))
					{
						node = parent;
					}
					node = parent;
				}
			}
			else
			{
				return get_child_<is_next>(node);
			}
			return node;
		}

		template<bool is_min>
		static node_t *bst_most_(node_t *node)
		{
			while (!is_nil_(get_child_<is_min>(node)))
			{
				node = get_child_<is_min>(node);
			}
			return node;
		}

		node_t *bst_lower_bound_(key_type const &key) const
		{
			node_t *node = get_root_(), *where = nil_();
			while (!is_nil_(node))
			{
				if (get_comparator_()(get_key_(node), key))
				{
					node = get_right_(node);
				}
				else
				{
					where = node;
					node = get_left_(node);
				}
			}
			return where;
		}

		node_t *bst_upper_bound_(key_type const &key) const
		{
			node_t *node = get_root_(), *where = nil_();
			while (!is_nil_(node))
			{
				if (get_comparator_()(key, get_key_(node)))
				{
					where = node;
					node = get_left_(node);
				}
				else
				{
					node = get_right_(node);
				}
			}
			return where;
		}

		std::pair<node_t *, node_t *> bst_equal_range_(key_type const &key) const
		{
			node_t *node = get_root_();
			node_t *lower = nil_();
			node_t *upper = nil_();
			while (!is_nil_(node))
			{
				if (get_comparator_()(get_key_(node), key))
				{
					node = get_right_(node);
				}
				else
				{
					if (is_nil_(upper) && get_comparator_()(key, get_key_(node)))
					{
						upper = node;
					}
					lower = node;
					node = get_left_(node);
				}
			}
			node = is_nil_(upper) ? get_root_() : get_left_(upper);
			while (!is_nil_(node))
			{
				if (get_comparator_()(key, get_key_(node)))
				{
					upper = node;
					node = get_left_(node);
				}
				else
				{
					node = get_right_(node);
				}
			}
			return std::make_pair(lower, upper);
		}

		node_t *sbt_at_(size_type index)
		{
			node_t *node = get_root_();
			if (index >= get_size_(node))
			{
				return nil_();
			}
			size_type rank = get_size_(get_left_(node));
			while (index != rank)
			{
				if (index < rank)
				{
					node = get_left_(node);
				}
				else
				{
					index -= rank + 1;
					node = get_right_(node);
				}
				rank = get_size_(get_left_(node));
			}
			return node;
		}

		node_t *sbt_at_(size_type index) const
		{
			node_t *node = get_root_();
			if (index >= get_size_(node))
			{
				return nil_();
			}
			size_type rank = get_size_(get_left_(node));
			while (index != rank)
			{
				if (index < rank)
				{
					node = get_left_(node);
				}
				else
				{
					index -= rank + 1;
					node = get_right_(node);
				}
				rank = get_size_(get_left_(node));
			}
			return node;
		}

		static node_t *sbt_advance_(node_t *node, difference_type step)
		{
			if (is_nil_(node))
			{
				if (step == 0)
				{
					return node;
				}
				else if (step > 0)
				{
					--step;
					node = get_left_(node);
				}
				else
				{
					++step;
					node = get_right_(node);
				}
				if (is_nil_(node))
				{
					return node;
				}
			}
			size_type u_step;
			while (step != 0)
			{
				if (step > 0)
				{
					u_step = step;
					if (get_size_(get_right_(node)) >= u_step)
					{
						step -= get_size_(get_left_(get_right_(node))) + 1;
						node = get_right_(node);
						continue;
					}
				}
				else
				{
					u_step = -step;
					if (get_size_(get_left_(node)) >= u_step)
					{
						step += get_size_(get_right_(get_left_(node))) + 1;
						node = get_left_(node);
						continue;
					}
				}
				if (is_nil_(get_parent_(node)))
				{
					return get_parent_(node);
				}
				else
				{
					if (get_right_(get_parent_(node)) == node)
					{
						step += get_size_(get_left_(node)) + 1;
						node = get_parent_(node);
					}
					else
					{
						step -= get_size_(get_right_(node)) + 1;
						node = get_parent_(node);
					}
				}
			}
			return node;
		}

		static size_type sbt_rank_(node_t *node)
		{
			if (is_nil_(node))
			{
				return get_size_(get_parent_(node));
			}
			size_type rank = get_size_(get_left_(node));
			node_t *parent = get_parent_(node);
			while (!is_nil_(parent))
			{
				if (node == get_right_(parent))
				{
					rank += get_size_(get_left_(parent)) + 1;
				}
				node = parent;
				parent = get_parent_(node);
			}
			return rank;
		}

		size_type bst_lower_rank_(key_type const &key) const
		{
			node_t *node = get_root_();
			size_type rank = 0;
			while (!is_nil_(node))
			{
				if (get_comparator_()(get_key_(node), key))
				{
					rank += get_size_(get_left_(node)) + 1;
					node = get_right_(node);
				}
				else
				{
					node = get_left_(node);
				}
			}
			return rank;
		}

		size_type bst_upper_rank_(key_type const &key) const
		{
			node_t *node = get_root_();
			size_type rank = 0;
			while (!is_nil_(node))
			{
				if (get_comparator_()(key, get_key_(node)))
				{
					node = get_left_(node);
				}
				else
				{
					rank += get_size_(get_left_(node)) + 1;
					node = get_right_(node);
				}
			}
			return rank;
		}

		template<bool is_left> node_t *sbt_rotate_(node_t *node)
		{
			node_t *child = get_child_<!is_left>(node), *parent = get_parent_(node);
			set_child_<!is_left>(node, get_child_<is_left>(child));
			if (!is_nil_(get_child_<is_left>(child)))
			{
				set_parent_(get_child_<is_left>(child), node);
			}
			set_parent_(child, parent);
			if (node == get_root_())
			{
				set_root_(child);
			}
			else if (node == get_child_<is_left>(parent))
			{
				set_child_<is_left>(parent, child);
			}
			else
			{
				set_child_<!is_left>(parent, child);
			}
			set_child_<is_left>(child, node);
			set_parent_(node, child);
			set_size_(child, get_size_(node));
			sbt_refresh_size_(node);
			return child;
		}

		template<bool is_left>
		node_t *sbt_maintain_(node_t *node)
		{
			if (is_nil_(get_child_<is_left>(node)))
			{
				return node;
			}
			if (get_size_(get_child_<is_left>(get_child_<is_left>(node))) > get_size_(get_child_<!is_left>(node)))
			{
				node = sbt_rotate_<!is_left>(node);
			}
			else
			{
				if (get_size_(get_child_<!is_left>(get_child_<is_left>(node))) > get_size_(get_child_<!is_left>(node)))
				{
					sbt_rotate_<is_left>(get_child_<is_left>(node));
					node = sbt_rotate_<!is_left>(node);
				}
				else
				{
					return node;
				};
			};
			if (!is_nil_(get_child_<true>(node)))
			{
				sbt_maintain_<true>(get_child_<true>(node));
			}
			if (!is_nil_(get_child_<false>(node)))
			{
				sbt_maintain_<false>(get_child_<false>(node));
			}
			node = sbt_maintain_<true>(node);
			node = sbt_maintain_<false>(node);
			return node;
		}

		void check_max_size_()
		{
			if (size() >= max_size() - 1)
			{
				throw std::length_error("sbtree too long");
			}
		}

		template<bool is_leftish>
		node_t *sbt_insert_(node_t *key)
		{
			if (is_nil_(get_root_()))
			{
				bst_init_node_(nil_(), key);
				set_root_(key);
				set_most_left_(key);
				set_most_right_(key);
				return key;
			}
			node_t *node = get_root_(), *where = nil_();
			bool is_left = true;
			while (!is_nil_(node))
			{
				set_size_(node, get_size_(node) + 1);
				where = node;
				if (is_leftish)
				{
					is_left = !get_comparator_()(get_key_(node), get_key_(key));
				}
				else
				{
					is_left = get_comparator_()(get_key_(key), get_key_(node));
				}
				if (is_left)
				{
					node = get_left_(node);
				}
				else
				{
					node = get_right_(node);
				}
			}
			sbt_insert_at_<false>(is_left, where, key);
			return key;
		}

		node_t *sbt_insert_hint_(node_t *where, node_t *key)
		{
			bool is_leftish = false;
			node_t *other;
			if (is_nil_(get_root_()))
			{
				bst_init_node_(nil_(), key);
				set_root_(key);
				set_most_left_(key);
				set_most_right_(key);
				return key;
			}
			else if (where == get_most_left_())
			{
				if (!get_comparator_()(get_key_(where), get_key_(key)))
				{
					sbt_insert_at_<true>(true, where, key);
					return key;
				}
				is_leftish = true;
			}
			else if (where == nil_())
			{
				if (!get_comparator_()(get_key_(key), get_key_(get_most_right_())))
				{
					sbt_insert_at_<true>(false, get_most_right_(), key);
					return key;
				}
			}
			else if (!get_comparator_()(get_key_(where), get_key_(key)) && !get_comparator_()(get_key_(key), get_key_(other = bst_move_<false>(where))))
			{
				if (is_nil_(get_right_(other)))
				{
					sbt_insert_at_<true>(false, other, key);
				}
				else
				{
					sbt_insert_at_<true>(true, where, key);
				}
				return key;
			}
			else if (!get_comparator_()(get_key_(key), get_key_(where)) && ((other = bst_move_<true>(where)) == nil_() || !get_comparator_()(get_key_(other), get_key_(key))))
			{
				if (is_nil_(get_right_(where)))
				{
					sbt_insert_at_<true>(false, where, key);
				}
				else
				{
					sbt_insert_at_<true>(true, other, key);
				}
				return key;
			}
			else
			{
				is_leftish = true;
			}
			if (is_leftish)
			{
				sbt_insert_<true>(key);
			}
			else
			{
				sbt_insert_<false>(key);
			}
			return key;
		}

		template<bool is_hint>
		void sbt_insert_at_(bool is_left, node_t *where, node_t *node)
		{
			if (is_hint)
			{
				node_t *parent = where;
				do
				{
					set_size_(parent, get_size_(parent) + 1);
				} while (!is_nil_(parent = get_parent_(parent)));
			}
			bst_init_node_(where, node);
			if (is_left)
			{
				set_left_(where, node);
				if (where == get_most_left_())
				{
					set_most_left_(node);
				}
			}
			else
			{
				set_right_(where, node);
				if (where == get_most_right_())
				{
					set_most_right_(node);
				}
			}
			sbt_insert_maintain_(where, node);
		}

		void sbt_insert_maintain_(node_t *where, node_t *node)
		{
			while (!is_nil_(where))
			{
				if (node == get_left_(where))
				{
					where = sbt_maintain_<true>(where);
				}
				else
				{
					where = sbt_maintain_<false>(where);
				}
				node = where;
				where = get_parent_(where);
			}
		}

		template<bool is_clear>
		void sbt_erase_(node_t *node)
		{
			node_t *erase_node = node;
			node_t *fix_node;
			node_t *fix_node_parent;
			bool is_left;
			if (!is_clear)
			{
				fix_node = node;
				while (!is_nil_((fix_node = get_parent_(fix_node))))
				{
					set_size_(fix_node, get_size_(fix_node) - 1);
				}
			}

			if (is_nil_(get_left_(node)))
			{
				fix_node = get_right_(node);
				is_left = true;
			}
			else if (is_nil_(get_right_(node)))
			{
				fix_node = get_left_(node);
				is_left = false;
			}
			else
			{
				if (get_size_(get_left_(node)) > get_size_(get_right_(node)))
				{
					node = sbt_erase_at_<is_clear, true>(node);
					if (!is_clear)
					{
						sbt_erase_maintain_(node, true);
					}
				}
				else
				{
					node = sbt_erase_at_<is_clear, false>(node);
					if (!is_clear)
					{
						sbt_erase_maintain_(node, false);
					}
				}
				return;
			}
			fix_node_parent = get_parent_(erase_node);
			if (!is_nil_(fix_node))
			{
				set_parent_(fix_node, fix_node_parent);
			}
			if (get_root_() == erase_node)
			{
				set_root_(fix_node);
			}
			else if (get_left_(fix_node_parent) == erase_node)
			{
				set_left_(fix_node_parent, fix_node);
				if (!is_clear)
				{
					sbt_erase_maintain_(fix_node_parent, true);
				}
			}
			else
			{
				set_right_(fix_node_parent, fix_node);
				if (!is_clear)
				{
					sbt_erase_maintain_(fix_node_parent, false);
				}
			}
			if (get_most_left_() == erase_node)
			{
				set_most_left_(is_nil_(fix_node) ? fix_node_parent : bst_most_<true>(fix_node));
			}
			if (get_most_right_() == erase_node)
			{
				set_most_right_(is_nil_(fix_node) ? fix_node_parent : bst_most_<false>(fix_node));
			}
		}

		template<bool is_clear, bool is_left>
		node_t *sbt_erase_at_(node_t *node)
		{
			node_t *erase_node = node;
			node_t *fix_node;
			node_t *fix_node_parent;
			node = bst_move_<!is_left>(node);
			fix_node = get_child_<is_left>(node);
			if (!is_clear)
			{
				fix_node_parent = node;
				while ((fix_node_parent = get_parent_(fix_node_parent)) != erase_node)
				{
					set_size_(fix_node_parent, get_size_(fix_node_parent) - 1);
				}
			}
			set_parent_(get_child_<!is_left>(erase_node), node);
			set_child_<!is_left>(node, get_child_<!is_left>(erase_node));
			if (node == get_child_<is_left>(erase_node))
			{
				fix_node_parent = node;
			}
			else
			{
				fix_node_parent = get_parent_(node);
				if (!is_nil_(fix_node))
				{
					set_parent_(fix_node, fix_node_parent);
				}
				set_child_<!is_left>(fix_node_parent, fix_node);
				set_child_<is_left>(node, get_child_<is_left>(erase_node));
				set_parent_(get_child_<is_left>(erase_node), node);
			}
			if (get_root_() == erase_node)
			{
				set_root_(node);
			}
			else if (get_child_<!is_left>(get_parent_(erase_node)) == erase_node)
			{
				set_child_<!is_left>(get_parent_(erase_node), node);
			}
			else
			{
				set_child_<is_left>(get_parent_(erase_node), node);
			}
			set_parent_(node, get_parent_(erase_node));
			if (!is_clear)
			{
				sbt_refresh_size_(node);
			}
			return fix_node_parent;
		}

		void sbt_erase_maintain_(node_t *where, bool is_left)
		{
			if (is_left)
			{
				where = sbt_maintain_<false>(where);
			}
			else
			{
				where = sbt_maintain_<true>(where);
			}
			node_t *node = where;
			where = get_parent_(where);
			while (!is_nil_(where))
			{
				if (node == get_left_(where))
				{
					where = sbt_maintain_<false>(where);
				}
				else
				{
					where = sbt_maintain_<true>(where);
				}
				node = where;
				where = get_parent_(where);
			}
		}

		void sbt_clear_(node_t *node)
		{
			if (!is_nil_(node))
			{
				sbt_clear_uncheck_(node);
				sbt_destroy_node_(node);
			}
		}

		void sbt_clear_uncheck_(node_t *node)
		{
			if (!is_nil_(get_left_(node)))
			{
				sbt_clear_uncheck_(get_left_(node));
				sbt_destroy_node_(get_left_(node));
			}
			if (!is_nil_(get_right_(node)))
			{
				sbt_clear_uncheck_(get_right_(node));
				sbt_destroy_node_(get_right_(node));
			}
		}

		void sbt_destroy_node_(node_t* node)
		{
			delete node;
		}

		template<class is_move>
		void sbt_copy_(size_balanced_tree *memory, node_t *other)
		{
			if (!is_nil_(other))
			{
				set_root_(sbt_copy_uncheck_<is_move>(memory, nil_(), other));
				set_most_left_(bst_most_<true>(get_root_()));
				set_most_right_(bst_most_<false>(get_root_()));
			}
		}

		template<class is_move>
		node_t *sbt_copy_uncheck_(size_balanced_tree *memory, node_t *node, node_t *other)
		{
			node_t *new_node = sbt_copy_node_(memory, other, is_move());
			set_parent_(new_node, node);
			set_left_(new_node, nil_());
			set_right_(new_node, nil_());
			set_size_(new_node, get_size_(other));
			try
			{
				if (!is_nil_(get_left_(other)))
				{
					set_left_(new_node, sbt_copy_uncheck_<is_move>(memory, new_node, get_left_(other)));
				}
				if (!is_nil_(get_right_(other)))
				{
					set_right_(new_node, sbt_copy_uncheck_<is_move>(memory, new_node, get_right_(other)));
				}
			}
			catch (...)
			{
				sbt_clear_(new_node);
				throw;
			}
			return new_node;
		}
	};

	template <typename T, typename TD>
	class deallocator_t
	{
		typedef typename T::node_t	node_t;
		T& tree;
		TD& dealloc;
	public:
		deallocator_t(T& t, TD& td) : tree(t), dealloc(td) {}

		void clear()
		{
			sbt_clear_(tree.get_root_());
			tree.set_root_(tree.nil_());
			//sbt_clear_uncheck_(tree.nil_());
			//tree.reset();
		}

	private:
		void sbt_clear_(node_t* node)
		{
			if (!tree.is_nil_(node))
			{
				sbt_clear_uncheck_(node);
				dealloc.destroy_node(node);
			}
		}

		void sbt_clear_uncheck_(node_t* node)
		{
			if (!tree.is_nil_(tree.get_left_(node)))
			{
				sbt_clear_uncheck_(tree.get_left_(node));
				dealloc.destroy_node(tree.get_left_(node));
			}
			if (!tree.is_nil_(tree.get_right_(node)))
			{
				sbt_clear_uncheck_(tree.get_right_(node));
				dealloc.destroy_node(tree.get_right_(node));
			}
		}
	};


	////////////////////////////////////////////////////////////////////+value
	template <typename key_t, typename value_t>
	struct node_ex_s : public sbtree::node_s<node_ex_s<key_t, value_t>, key_t>
	{
		typedef sbtree::node_s<node_ex_s<key_t, value_t>, key_t>	base_type;
		typedef key_t key_type;
		value_t	value;
		node_ex_s()
			: value(value_t())
		{
		}
		node_ex_s(key_type _key, value_t _value)
			: base_type(_key),
			value(_value)
		{
		}
	};

	////////////////////////////////////////////////////////////////////
	//with a root node
	template<class node_t, class comparator_t = std::less<typename node_t::key_type>>
	struct config1_s
	{
	public:
		typedef node_t	node_type;
		typedef comparator_t	key_compare;
		typedef typename node_t::key_type key_type;

	private:
		typedef head_s<node_t, comparator_t> head_t;
		head_t head_;

	protected:
		template<class any_key_compare>
		config1_s(any_key_compare &&comp)
			: head_(comp)
		{
			head_.root = new node_t;
			reset();
		}

	public:
		config1_s()
			: config1_s(key_compare())
		{
		}

		~config1_s()
		{
			delete head_.root;
		}

		void moveFrom(config1_s &o)
		{
			assert(is_nil_(get_root_()));//empty
			//std::swap(head_, o.head_);
			node_t *tmp = head_.root;
			head_.root = o.head_.root;
			o.head_.root = tmp;
		}

		void reset()
		{
			set_size_(nil_(), 0);
			set_root_(nil_());
			set_most_left_(nil_());
			set_most_right_(nil_());
		}

	protected:
		bool empty()
		{
			return false;
		}

		key_compare &get_comparator_(){
			return head_;
		}

		key_compare const &get_comparator_() const {
			return head_;
		}

		node_t *nil_() const {
			return head_.root;
		}

		node_t *get_root_() const
		{
			return get_parent_(nil_());
		}

		void set_root_(node_t *root)
		{
			set_parent_(nil_(), root);
		}

		node_t *get_most_left_() const
		{
			return get_left_(nil_());
		}

		void set_most_left_(node_t *left)
		{
			set_left_(nil_(), left);
		}

		node_t *get_most_right_() const
		{
			return get_right_(nil_());
		}

		void set_most_right_(node_t *right)
		{
			set_right_(nil_(), right);
		}

		static key_type const &get_key_(node_t *node)
		{
			return node->key;
		}

		static bool is_nil_(const node_t *node)
		{
			return node->isEmpty();
		}

		static node_t *get_parent_(const node_t *node)
		{
			return node->parent;
		}

		static void set_parent_(node_t *node, node_t *parent)
		{
			node->parent = parent;
		}

		static node_t *get_left_(const node_t *node)
		{
			return node->left;
		}

		static void set_left_(node_t *node, node_t *left)
		{
			node->left = left;
		}

		static node_t *get_right_(const node_t *node)
		{
			return node->right;
		}

		static void set_right_(node_t *node, node_t *right)
		{
			node->right = right;
		}

		static size_t get_size_(const node_t *node)
		{
			return node->getSize();
		}

		static void set_size_(node_t *node, size_t size)
		{
			node->setSize(size);
		}
	};


	////////////////////////////////////////////////////////////////////
	//without a root node
#if(0)//not functional?
	template<class node_t, class comparator_t = std::less<typename node_t::key_type>>
	class config2_s
	{
	public:
		typedef node_t	node_type;
		typedef comparator_t	key_compare;
		typedef typename node_t::key_type key_type;

	private:
		typedef head_s<node_t, comparator_t> head_t;
		head_t head_;

	protected:
		template<class any_key_compare>
		config2_s(any_key_compare &&comp)
			: head_(comp)
		{
		}

	public:
		config2_s()
			: config2_s(key_compare())
		{
		}

	protected:
		key_compare &get_comparator_(){
			return head_;
		}

		key_compare const &get_comparator_() const {
			return head_;
		}

		node_t *nil_() const {
			return nullptr;
		}

		node_t *get_root_() const
		{
			return head_.root;
		}

		void set_root_(node_t *root)//s
		{
			set_parent_(nil_(), root);
		}

		node_t *get_most_left_() const
		{
			return minValueNode(head_.root);
		}

		void set_most_left_(node_t *left)//s
		{
			set_left_(nil_(), left);
		}

		node_t *get_most_right_() const
		{
			return maxValueNode(head_.root);
		}

		void set_most_right_(node_t *right)//s
		{
			set_right_(nil_(), right);
		}

		static key_type const &get_key_(node_t *node)//s
		{
			return node->key;
		}

		static bool is_nil_(const node_t *node)
		{
			return node == nullptr;
		}

		static node_t *get_parent_(const node_t *node)
		{
			if (node)
				return node->parent;
			return nullptr;
		}

		/*static*/ void set_parent_(node_t *node, node_t *parent)
		{
			if (node == nullptr)
				head_.root = parent;
			else
				node->parent = parent;
		}

		static node_t *get_left_(const node_t *node)//s
		{
			return node->left;
		}

		/*static*/ void set_left_(node_t *node, node_t *left)
		{
			if (node == nullptr)
				head_.root = left;
			else
				node->left = left;
		}

		static node_t *get_right_(const node_t *node)//s
		{
			return node->right;
		}

		/*static*/ void set_right_(node_t *node, node_t *right)
		{
			if (node == nullptr)
				head_.root = right;
			else
				node->right = right;
		}

		static size_t get_size_(const node_t *node)
		{
			if (node)
				return node->size;
			return 0;
		}

		static void set_size_(node_t *node, size_t size)//s
		{
			node->size = size;
		}

	private:
		static node_t * minValueNode(const node_t* node)
		{
			node_t* current = node;
			if (current)
			{
				// loop down to find the leftmost leaf
				while (current->left != nullptr)
					current = current->left;
			}
			return current;
		}

		static node_t * maxValueNode(const node_t* node)
		{
			node_t* current = node;
			if (current)
			{
				// loop down to find the rightmost leaf
				while (current->right != nullptr)
					current = current->right;
			}
			return current;
		}
	};
#endif



	///////////////////////////////////////////////// sbtree bimap
	template <typename K, typename V>
	struct bimap
	{
		//inner node
		struct inode_s : public sbtree::node_s<inode_s, K>
		{
			typedef sbtree::node_s<inode_s, K> base;
			struct value_s
			{
				V p;
				value_s() : p(nullptr) {}
				value_s(V _p) : p(_p) {}
			};
			value_s value;
			inode_s() {}
			inode_s(K _key, V _val) : base(_key), value(_val){}
		};

		//outer node
		struct binode_s : public sbtree::node_s<binode_s, inode_s>
		{
			typedef sbtree::node_s<binode_s, inode_s> base;

			binode_s() {}
			binode_s(K _key, V _val)
				: base(inode_s(_key, _val))
			{
			}
		};

		struct compare_s {
			bool operator()(const inode_s& a, const inode_s& b) const
			{
				return a.value.p < b.value.p;
			}
		};

		typedef sbtree::config1_s<inode_s> config_t;

		typedef sbtree::config1_s<binode_s, compare_s> biconfig_t;

		class impl_s : protected sbtree::size_balanced_tree<config_t>//ordinal=>field_ptr
		{
			typedef sbtree::size_balanced_tree<config_t> base_t;
			template <typename T, typename TD> friend class sbtree::deallocator_t;
			typedef sbtree::size_balanced_tree<biconfig_t> reverse_map_t;
			typedef typename reverse_map_t::iteartor reverse_map_iterator;
			typedef typename reverse_map_t::const_iteartor reverse_map_const_iterator;
			reverse_map_t mReverse;//field_ptr=>ordinal
		public:
			typedef inode_s	inner_node;
		public:
			impl_s() {}
			~impl_s() {
				clear();
			}
			bool add(K key, V val)
			{
				binode_s* pn(new binode_s(key, val));
				std::pair<reverse_map_iterator, bool> ret(mReverse.insert_unique(pn));
				if (ret.second)
				{
					if (insert_unique(&pn->key).second)
						return true;
					binode_s* pn2(mReverse.take(ret.first));
					assert(pn2 == pn);
					delete pn;
				}
				return false;
			}
			V lookup(const K key) const
			{
				typename base_t::const_iterator i(find(key));
				if (i != base_t::end())
					return i->value.p;
				return nullptr;
			}
			K lookup(const V& val) const
			{
				inode_s key(0, val);//key is irrelevant
				reverse_map_const_iterator i(mReverse.find(key));
				if (i != mReverse.end())
				{
					const binode_s& n(*i);
					return n.key.key;
				}
				return -1;
			}
			bool empty() const { return base_t::empty(); }
			size_t size() const { return base_t::size(); }
			const inner_node& at(size_t index) const { return *base_t::at(index); }
			size_t rank(const K key) const { return base_t::rank(key); }
			void clear()
			{
				sbtree::deallocator_t<reverse_map_t, impl_s> dealloc(mReverse, *this);
				dealloc.clear();
				base_t::reset();//clear a direct map (no deallocations)
			}
			void destroy_node(binode_s* node)
			{
				delete node;
			}
		};
	};//struct bimap


}//namespace sbtree


