// rrbtree.cpp
// This particular source file, rrbtree.cpp, is hereby released to the public domain.
//
// Author: Willow Schlanger <wschlanger@gmail.com>
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// This contains code based on the public domain tutorial from here:
// http://eternallyconfuzzled.com/tuts/datastructures/jsw_tut_rbtree.aspx

#include <stddef.h>
#include <string.h>

// "nodelist" is a circular linked list template class.
// Unlike the Standard C++ list<> class, previously existing classes can be made into linked list nodes.
// This offers greater flexibility than STL provides.

namespace wstd
{

template <class ValueType>
class RrbTree;

template <class ValueType>
class RrbNode
{
friend class RrbTree<ValueType>;
private:
    RrbNode *parent;
    RrbNode *link[2];
    size_t span;				// weight + getSpan(0) + getSpan(1)
    size_t weight;				// must be at least 1
    size_t order;				// 1 + getOrder(0) + getOrder(1)
    signed char red;
    ValueType value;

    size_t getSpan(int child) const
    {
        if(link[child] == NULL)
            return 0;
        return link[child]->span;
    }
    
    size_t getOrder(int child) const
    {
    	if(link[child] == NULL)
    		return 0;
    	return link[child]->order;
    }

    void updateSpan()
    {
        span = weight + getSpan(0) + getSpan(1);
        order = 1 + getOrder(0) + getOrder(1);
    }

    RrbNode *min()
    {
        RrbNode *x = this;
        while(x->link[0] != NULL)
            x = x->link[0];
        return x;
    }

    RrbNode *max()
    {
        RrbNode *x = this;
        while(x->link[1] != NULL)
            x = x->link[1];
        return x;
    }

public:
    RrbNode()
    {
        parent = NULL;
        link[0] = link[1] = NULL;
        span = weight = 1;
        red = 1;				// new nodes are red
        order = 1;
    }
    
    RrbNode *next()
    {
    	RrbNode *node = this;
    	if(node->link[1] != NULL)
    		return node->link[1]->min();
    	RrbNode *y = node->parent;
    	while(y != NULL && node == y->link[1])
    	{
    		node = y;
    		y = y->parent;
    	}
    	return y;
    }
    
    const RrbNode *next() const
    {
    	const RrbNode *node = (const RrbNode *)(this);
    	if(node->link[1] != NULL)
    		return node->link[1]->min();
    	RrbNode *y = node->parent;
    	while(y != NULL && node == y->link[1])
    	{
    		node = y;
    		y = y->parent;
    	}
    	return y;
    }
    
    RrbNode *prev()
    {
    	RrbNode *node = this;
    	if(node->link[0] != NULL)
    		return node->link[0]->max();
    	RrbNode *y = node->parent;
    	while(y != NULL && node == y->link[0])
    	{
    		node = y;
    		y = y->parent;
    	}
    	return y;
    }
    
    const RrbNode *prev() const
    {
    	const RrbNode *node = (const RrbNode *)(this);
    	if(node->link[0] != NULL)
    		return node->link[0]->max();
    	RrbNode *y = node->parent;
    	while(y != NULL && node == y->link[0])
    	{
    		node = y;
    		y = y->parent;
    	}
    	return y;
    }
    
    size_t getWeight() const
    {
    	return weight;
    }
    
    bool setWeight(size_t value)
    {
    	if(value == 0)
    		return false;
    	if(value == weight)
    		return true;
    	
    	// Check that we won't overflow.
    	if(value > weight)
    	{
    		RrbNode *node = this;
    		while(node->parent != NULL)
    			node = node->parent;
    		// node is now the root node.
    		size_t delta = value - weight;
    		size_t tmp = node->span + delta;
    		if(tmp < node->span || tmp < delta)
    			return false;	// we'd overflow!
    	}
    	
    	weight = value;
    	updateSpan();
    	for(RrbNode *node = this; ;)
    	{
    		node = node->parent;
    		if(node == NULL)
    			break;
    		node->updateSpan();
    	}
    	
    	return true;
    }
    
	ValueType &operator*()
	{
		return value;
	}
    
	const ValueType &operator*() const
	{
		return value;
	}
	
	ValueType *operator->()
	{
		return &value;
	}
	
	const ValueType *operator->() const
	{
		return &value;
	}

    size_t getWeightedPosition() const
    {
    	const RrbNode *node = (const RrbNode *)(this);
        size_t span = 0;

        if(node->parent == NULL)
            return getSpan(0);

        span += node->getSpan(0);       // BUG FIX 11-30-2010

        while(node->parent != NULL)
        {
            int dir = (node->parent->link[1] == node);

            if(dir != 0)
            {
                span += node->parent->getSpan(0) + node->parent->weight;
            }

            node = node->parent;
        }

        return span;
    }

    size_t getOrder() const
    {
    	const RrbNode *node = (const RrbNode *)(this);
        size_t span = 0;

        if(node->parent == NULL)
            return getOrder(0);

        span += node->getOrder(0);       // BUG FIX 11-30-2010

        while(node->parent != NULL)
        {
            int dir = (node->parent->link[1] == node);

            if(dir != 0)
            {
                span += node->parent->getOrder(0) + 1;
            }

            node = node->parent;
        }

        return span;
    }
};

template <class ValueType>
class RrbTree
{
public:
    RrbNode<ValueType> *root;

    RrbTree()
    {
        root = NULL;
    }
    
    size_t size() const
    {
    	if(root == NULL)
    		return 0;
    	return root->order;
    }
    
    size_t weightedSize() const
    {
    	if(root == NULL)
    		return 0;
    	return root->span;
    }

    // Note: path should be an array of at least 128 RrbNode pointers, if not NULL.
    RrbNode<ValueType> *findBySpan(size_t span, RrbNode<ValueType> **path = NULL, size_t *depthOut = NULL)
    {
        RrbNode<ValueType> *node = root;
        size_t depth = 0;
        if(depthOut != NULL)
            *depthOut = 0;
        for(;;)
        {
            if(node == NULL)
                return NULL;
            if(span >= node->span)
                return NULL;
            if(path != NULL)
                path[depth] = node;
            ++depth;
            size_t leftSpan = node->getSpan(0);
            if(span > leftSpan)
            {
                if(node->link[1] == NULL)
                    return NULL;
                span -= (node->span - node->link[1]->span);
                node = node->link[1];
            }
            else if(span < leftSpan)
                node = node->link[0];
            else
                break;
        }
        if(depthOut != NULL)
            *depthOut = depth;
        return node;
    }

    bool removeBySpan(size_t span)
    {
        RrbNode<ValueType> *path[128];
        size_t depth;
        RrbNode<ValueType> *node = findBySpan(span, path, &depth);
        if(node == NULL)
            return false;		// not found

        bool done = false;
        root = remove_r(root, done, node, path, depth, 0, false);
        if(root != NULL)
        {
            root->red = 0;		// make root black
            root->updateSpan();
            root->parent = NULL;
        }

        return true;
    }

    // Note: path should be an array of at least 128 RrbNode pointers, if not NULL.
    RrbNode<ValueType> *findByOrder(size_t order, RrbNode<ValueType> **path = NULL, size_t *depthOut = NULL)
    {
    	size_t &span = order;
        RrbNode<ValueType> *node = root;
        size_t depth = 0;
        if(depthOut != NULL)
            *depthOut = 0;
        for(;;)
        {
            if(node == NULL)
                return NULL;
            if(span >= node->span)
                return NULL;
            if(path != NULL)
                path[depth] = node;
            ++depth;
            size_t leftSpan = node->getOrder(0);
            if(span > leftSpan)
            {
                if(node->link[1] == NULL)
                    return NULL;
                span -= (node->order - node->link[1]->order);
                node = node->link[1];
            }
            else if(span < leftSpan)
                node = node->link[0];
            else
                break;
        }
        if(depthOut != NULL)
            *depthOut = depth;
        return node;
    }

    bool removeByOrder(size_t order)
    {
    	size_t &span = order;
        RrbNode<ValueType> *path[128];
        size_t depth;
        RrbNode<ValueType> *node = findByOrder(span, path, &depth);
        if(node == NULL)
            return false;		// not found

        bool done = false;
        root = remove_r(root, done, node, path, depth, 0, false);
        if(root != NULL)
        {
            root->red = 0;		// make root black
            root->updateSpan();
            root->parent = NULL;
        }

        return true;
    }

    ~RrbTree()
    {
        clear();
    }

    void clear()
    {
        destroyAll(root);
        root = NULL;
    }
    
    RrbNode<ValueType> *insertBySpan(size_t pos, ValueType value, size_t weight = 1)
    {
        size_t s = weightedSize();
        size_t x = s + weight;
        if(weight == 0 || x < s || x < weight)
            return NULL;	// weight was either 0 or so big we overflowed

        RrbNode<ValueType> *node = new RrbNode<ValueType>();
        node->value = value;
        node->span = node->weight = weight;

        bool ok = doSpanInsert(pos, node);
        if(!ok)
        {
            delete node;
            return NULL;
        }

        return node;
    }
    
    RrbNode<ValueType> *insertByOrder(size_t pos, ValueType value, size_t weight = 1)
    {
        size_t s = size();
        size_t x = s + weight;
        if(weight == 0 || x < s || x < weight)
            return NULL;	// weight was either 0 or so big we overflowed

        RrbNode<ValueType> *node = new RrbNode<ValueType>();
        node->value = value;
        node->span = node->weight = weight;

        bool ok = doOrderInsert(pos, node);
        if(!ok)
        {
            delete node;
            return NULL;
        }

        return node;
    }

    // Calls a Functor on every node until true is returned.
    // Returns false if we just operated on ALL nodes and never
    // obtained true.
    template <class Functor>
    bool scan(Functor &func)
    {
        return doScan(func, root);
    }

private:

    template <class Functor>
    bool doScan(Functor &func, RrbNode<ValueType> *node)
    {
        if(node == NULL)
            return false;
        if(doScan(func, node->link[0]))
            return true;

        if(func(*node, *this))
            return true;

        if(doScan(func, node->link[1]))
            return true;
        return false;
    }

    // Before calling insert(), user must allocate an RrbNode object
    // to pass in to this function. The default weight of that object
    // is 1, user may change it if necessary. The 'user' pointer in
    // the RrbNode object passed in can be left NULL, or it can be
    // changed too.
    //
    // Note: 'span' is the insertion offset. Use 0 if the tree is
    // empty. We will find the object that has the given span, then
    // we will insert the new object BEFORE that object. You can
    // append to the end of the tree by specifying a span equal to
    // getSpan() [i.e. root->span]. To prepend, insert with span = 0.
    //
    // Warning: undefined behavior if root->span overflows!
    bool doSpanInsert(size_t span, RrbNode<ValueType> *src)
    {
        initForInsert(src);
        if(root == NULL)
        {
            if(span != 0)
                return false;	// failed to insert
            root = src;
            root->red = 0;		// make root black
            root->parent = NULL;	// redundant

            return true;
        }
        size_t totalSpan = weightedSize();
        if(totalSpan == 0)
            return false;		// internal error

        RrbNode<ValueType> *insPt;
        int child;
        RrbNode<ValueType> *path[128];
        size_t depth = 0;

        if(span >= totalSpan)
        {
            if(span > totalSpan)
            {
                return false;
            }

            insPt = root;
            for(;;)
            {
                path[depth] = insPt;
                ++depth;
                if(insPt->link[1] == NULL)
                    break;
                insPt = insPt->link[1];
            }

            child = 1;			// use right child, is NULL
        }
        else
        {
            insPt = findBySpan(span, path, &depth);
            if(insPt == NULL)
            {
                return false;
            }
            child = 0;

            if(insPt->link[0] != NULL)
            {
                insPt = insPt->link[0];
                for(;;)
                {
                    path[depth] = insPt;
                    ++depth;
                    if(insPt->link[1] == NULL)
                        break;
                    insPt = insPt->link[1];
                }
                child = 1;
            }
        }
        
        return doInsert(insPt, child, src, path, depth);
    }

    bool doOrderInsert(size_t order, RrbNode<ValueType> *src)
    {
    	size_t &span = order;
        initForInsert(src);
        if(root == NULL)
        {
            if(span != 0)
                return false;	// failed to insert
            root = src;
            root->red = 0;		// make root black
            root->parent = NULL;	// redundant
            return true;
        }
        size_t totalSpan = size();
        if(totalSpan == 0)
            return false;		// internal error

        RrbNode<ValueType> *insPt;
        int child;
        RrbNode<ValueType> *path[128];
        size_t depth = 0;

        if(span >= totalSpan)
        {
            if(span > totalSpan)
                return false;

            insPt = root;
            for(;;)
            {
                path[depth] = insPt;
                ++depth;
                if(insPt->link[1] == NULL)
                    break;
                insPt = insPt->link[1];
            }

            child = 1;			// use right child, is NULL
        }
        else
        {
            insPt = findByOrder(span, path, &depth);
            if(insPt == NULL)
                return false;
            child = 0;

            if(insPt->link[0] != NULL)
            {
                insPt = insPt->link[0];
                for(;;)
                {
                    path[depth] = insPt;
                    ++depth;
                    if(insPt->link[1] == NULL)
                        break;
                    insPt = insPt->link[1];
                }
                child = 1;
            }
        }

        return doInsert(insPt, child, src, path, depth);
    }

    void initForInsert(RrbNode<ValueType> *src)
    {
        src->link[0] = src->link[1] = NULL;
        src->span = src->weight;
        src->order = 1;
        src->red = 1;		// new nodes are red
        src->parent = NULL;
    }

    bool doInsert(RrbNode<ValueType> *node, int child, RrbNode<ValueType> *src, RrbNode<ValueType> **path, size_t depth)
    {
        root = insert_r(root, node, src, 0, path, depth, child);
        root->parent = NULL;    // Left this out last time, BUG FIX!
        root->red = 0;		// make root black
        return true;	// success
    }

    RrbNode<ValueType> *insert_r(RrbNode<ValueType> *rootT, RrbNode<ValueType> *insPt, RrbNode<ValueType> *src, size_t height, RrbNode<ValueType> **path, size_t depth, int child)
    {
        if(rootT == NULL)
            rootT = src;
        else
        {
            int dir;
            if(rootT == insPt)
                dir = child;
            else
            {
                dir = (path[height + 1] == rootT->link[1]) ? 1 : 0;
            }

            // Update child node.
            rootT->link[dir] = insert_r(rootT->link[dir], insPt, src, height + 1, path, depth, child);
            rootT->link[dir]->parent = rootT;

            // Rebalance.
            if(is_red(rootT->link[dir]))
            {
                if(is_red(rootT->link[!dir]))
                {
                    rootT->red = 1;
                    rootT->link[0]->red = 0;
                    rootT->link[1]->red = 0;
                }
                else
                {
                    if(is_red(rootT->link[dir]->link[dir]))
                        rootT = rot_single(rootT, !dir);
                    else if(is_red(rootT->link[dir]->link[!dir]))
                        rootT = rot_double(rootT, !dir);
                }
            }

            rootT->updateSpan();
        }
        return rootT;
    }

    bool is_red(RrbNode<ValueType> *node)
    {
        if(node == NULL)
            return false;		// NULL nodes are black
        return node->red != 0;
    }

    RrbNode<ValueType> *rot_single(RrbNode<ValueType> *rootT, int dir)
    {
        RrbNode<ValueType> *save = rootT->link[!dir];

        rootT->link[!dir] = save->link[dir];
        save->link[dir] = rootT;

        if(rootT->link[!dir] != NULL)
            rootT->link[!dir]->parent = rootT;
        if(save->link[dir] != NULL)
            save->link[dir]->parent = save;

        rootT->red = 1;
        save->red = 0;

        rootT->updateSpan();
        save->updateSpan();

        save->parent = rootT->parent;
        rootT->parent = save;

        return save;
    }

    RrbNode<ValueType> *rot_double(RrbNode<ValueType> *rootT, int dir)
    {
        rootT->link[!dir] = rot_single(rootT->link[!dir], !dir);
        rootT->link[!dir]->updateSpan();
        rootT->updateSpan();

        RrbNode<ValueType> *tmp = rot_single(rootT, dir);
        tmp->updateSpan();
        return tmp;
    }

    void destroyAll(RrbNode<ValueType> *node)
    {
        if(node == NULL)
            return;
        destroyAll(node->link[0]);
        destroyAll(node->link[1]);
        delete node;
    }

    RrbNode<ValueType> *remove_r(RrbNode<ValueType> *rootT, bool &done, RrbNode<ValueType> *target, RrbNode<ValueType> **path, size_t depth, size_t height, bool override)
    {
        if(rootT == NULL)
            done = true;
        else
        {
            int dir;

            if(rootT == target)
            {
                if(rootT->link[0] == NULL || rootT->link[1] == NULL)
                {
                    RrbNode<ValueType> *save = rootT->link[rootT->link[0] == NULL];

                    if(is_red(rootT))
                        done = true;
                    else if(is_red(save))
                    {
                        save->red = 0;
                        done = true;
                    }

                    delete rootT;
                    return save;
                }
                else
                {
                    RrbNode<ValueType> *p = rootT->link[0];
                    int q = 0;
                    while(p->link[1] != NULL)
                    {
                        p = p->link[1];
                        q = 1;
                    }

                    RrbNode<ValueType> *tmp[2];

                    signed char redT = rootT->red;
                    rootT->red = p->red;
                    p->red = redT;

                    tmp[0] = p->link[0];
                    tmp[1] = p->link[1];

                    target = rootT;
                    p->parent->link[q] = rootT;
                    rootT = p;
                    p->link[0] = target->link[0];
                    p->link[1] = target->link[1];
                    if(p->link[0] != NULL)
                        p->link[0]->parent = p;
                    if(p->link[1] != NULL)
                        p->link[1]->parent = p;
                    rootT->updateSpan();

                    target->link[0] = tmp[0];
                    target->link[1] = tmp[1];
                    if(target->link[0] != NULL)
                        target->link[0]->parent = target;
                    if(target->link[1] != NULL)
                        target->link[1]->parent = target;

                    tmp[0] = p->parent;
                    p->parent = target->parent;
                    target->parent = tmp[0];

                    dir = 0;
                    rootT->link[dir] = remove_r(rootT->link[dir], done, target, path, depth, height + 1, true);
                    rootT->updateSpan();
                    if(rootT->link[dir] != NULL)
                        rootT->link[dir]->parent = rootT;

                    if(!done)
                    {
                        rootT = remove_balance(rootT, dir, done);
                        rootT->updateSpan();
                    }

                    return rootT;
                }
            }

            if(override)
                dir = 1;
            else
                dir = (path[height + 1] == rootT->link[1]) ? 1 : 0;
            rootT->link[dir] = remove_r(rootT->link[dir], done, target, path, depth, height + 1, override);
            rootT->updateSpan();
            if(rootT->link[dir] != NULL)
                rootT->link[dir]->parent = rootT;

            if(!done)
            {
                rootT = remove_balance(rootT, dir, done);
                rootT->updateSpan();
            }
        }
        return rootT;
    }

    // Remember to update parent pointers.
    RrbNode<ValueType> *remove_balance(RrbNode<ValueType> *rootT, int dir, bool &done)
    {
        RrbNode<ValueType> *p = rootT;
        RrbNode<ValueType> *s = rootT->link[!dir];

        if(is_red(s))
        {
            rootT = rot_single(rootT, dir);
            s = p->link[!dir];
        }

        if(s != NULL)
        {
            if(!is_red(s->link[0]) && !is_red(s->link[1]))
            {
                if(is_red(p))
                    done = true;
                p->red = 0;
                s->red = 1;
            }
            else
            {
                signed char save = p->red;
                bool new_root = (rootT == p);

                if(is_red(s->link[!dir]))
                    p = rot_single(p, dir);
                else
                    p = rot_double(p, dir);

                p->red = save;
                p->link[0]->red = 0;
                p->link[1]->red = 0;

                if(new_root)
                    rootT = p;
                else
                {
                    rootT->link[dir] = p;
                    p->parent = rootT;
                }

                done = true;
            }
        }

        return rootT;
    }
};

}	// namespace wstd

//---

using namespace wstd;

#include <iostream>
#include <string>

class OutputWriter
{
	public:
	bool operator()(RrbNode<const char *> &node, RrbTree<const char *> &tree)
	{
		std::cout << *node;
		return false;		// keep going
	}
};

int main(int argc, const char **argv)
{
	RrbTree<const char *> t;
	
	t.insertBySpan(0, "alpha", 5);
	t.insertBySpan(5, "beta", 4);
	t.insertBySpan(5+4, "gamma", 5);
	t.insertBySpan(5 + 4, "delta!!", 7);

	t.insertByOrder(0, "123", 5);
	t.insertByOrder(1, "789", 4);
	t.insertByOrder(1, "456", 5);
	
	OutputWriter writer;
	t.scan(writer);
	std::cout << std::endl;
	
	RrbNode<const char *> *tmp = t.findByOrder(6);
	while(tmp != NULL)
	{
		std::cout << **tmp;
		tmp = tmp->prev();
	}
	std::cout << std::endl;

	std::cout << "\nOutput is:" << std::endl;	
	for(size_t i = 0; i < t.size(); ++i)
	{
		std::cout << (*t.findByOrder(i)).getOrder() << ") " << **t.findByOrder(i) << " [weightedPos=" << (*t.findByOrder(i)).getWeightedPosition() << "]" << std::endl;
	}

	std::cout << "\nDeleting 456..." << std::endl;	
	t.removeByOrder(1);
	std::cout << "Output is:" << std::endl;	
	for(size_t i = 0; i < t.size(); ++i)
	{
		std::cout << (*t.findByOrder(i)).getOrder() << ") " << **t.findByOrder(i) << " [weightedPos=" << (*t.findByOrder(i)).getWeightedPosition() << "]" << std::endl;
	}

	std::cout << "\nDeleting \"delta!!\"..." << std::endl;	
	t.removeBySpan(18);
	std::cout << "Output is:" << std::endl;	
	for(size_t i = 0; i < t.size(); ++i)
	{
		std::cout << (*t.findByOrder(i)).getOrder() << ") " << **t.findByOrder(i) << " [weightedPos=" << (*t.findByOrder(i)).getWeightedPosition() << "]" << std::endl;
	}

	return 0;
}
