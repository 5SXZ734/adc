#pragma once

#include <vector>
#include "qx/MyString.h"
#include "ui_main_ex.h"
#include "interface/IADCGui.h"
//#include "qx/MyString.h"

class ExprTreeItem
{
public:
	MyString	s;
	MyString	sN;
	MyString	sId;
	MyString	sTyp;
	MyString	sAddr;
	ExprTreeItem	*parent;
	ExprTreeItem	*children[2];
public:
	ExprTreeItem()
	{
		parent = nullptr;
		children[0] = nullptr;
		children[1] = nullptr;
	}
	~ExprTreeItem()
	{
		delete children[0];
		delete children[1];
	}
	ExprTreeItem *addChild(ExprTreeItem *lveAfter)
	{
		ExprTreeItem *lvi(new ExprTreeItem);
		lvi->parent = this;
		assert(!children[1]);
		if (!children[0])
			children[0] = lvi;
		else
			children[1] = lvi;
		return lvi;
	}
	bool hasChildren() const
	{
		return children[0] || children[1];
	}
	unsigned childrenNum() const
	{
		if (children[0])
		{
			if (children[1])
				return 2;
			return 1;
		}
		assert(!children[1]);
		return 0;
	}
	int indexOf(ExprTreeItem *child) const
	{
		if (children[0] == child)
			return 0;
		if (children[1] == child)
			return 1;
		return -1;
	}
};

class ExprTreeHistory : public std::vector<ExprTreeItem *>
{

public:
	ExprTreeHistory(){}
	~ExprTreeHistory()
	{
		clear();
	}
	ExprTreeItem *addRoot(ExprTreeItem *lviAfter)
	{
		assert(!lviAfter || back() == lviAfter);
		push_back(new ExprTreeItem);
		return back();
	}
	void clear()
	{
		for (size_t i(0); i < size(); i++)
			delete at(i);
		std::vector<ExprTreeItem *>::clear();
	}
	int indexOf(ExprTreeItem *child) const
	{
		for (size_t i(0); i < size(); i++)
			if (at(i) == child)
				return (int)i;
		return -1;
	}
};

class ExprViewModel_t : public adcui::IADCExprViewModel
{
	CoreEx_t &mrCore;

	//MyTreeVector<ExprTreeItem>	m_data;
	ExprTreeHistory		m_roots;

public:
	ExprViewModel_t(CoreEx_t &rCore)
		: mrCore(rCore)
	{
	}
	virtual ~ExprViewModel_t()
	{
	}
	virtual int columnCount() const
	{
		return 5;
	}

	virtual void reset(){}
	virtual void resetEx(Flags flags)
	{
		m_roots.clear();

		MyStream ss;
		//mrCore.DumpExpr(ss, DUMPEXPR_NULL);//adcui::Flags flags)
		//ExprTreeItem eti;

		ReadLocker lock(mrCore.main());
		if (mrCore.main().hasProject())
		{
			ContextSafeEx_t safe(mrCore);
			if (safe)
			{
				//FileDef_t *pFileDef(safe->fileDef());
				//if (pFileDef)
				{
					flags = safe->isUnfoldMode() ? DUMPEXPR_PTRS : DUMPEXPR_NULL;
					CGlobPtr pGlob(safe->scopeFunc());
					if (pGlob)
					{
						DcInfo_t dcInfo(*safe->dcRef());
						dcInfo.DumpExpr(ss, flags, pGlob, safe->opLine());
						//FI.DumpExprPtr(ss);
					}
				}
			}
		}

#if(1)

		//ExprTreeItem *lviRoot(new ExprTreeItem);

		std::list<ExprTreeItem *> stack;//keeps last item on each level

		MyString s;
		while (ss.ReadString(s, "\n"))
		{
			if (s.isEmpty())
				continue;

			MyStringList o(MyStringList::split("\t", s, true));

			MyString sN, sId, sTyp, sAddr;
			if (!o.empty())
			{
				sN = o.front();
				o.pop_front();
				if (!o.empty())
				{
					sId = o.front();
					o.pop_front();
				}
			}

			size_t l(1);
			for (; !o.empty() && o.front().isEmpty(); l++)
				o.pop_front();
			if (o.empty())//invalid input?
				break;
			else
			{
				s = o.front();
				o.pop_front();
			}
			if (!o.empty())
			{
				sTyp = o.front();
				o.pop_front();
				if (!o.empty())
				{
					sAddr = o.front();
					o.pop_front();
				}
			}

			ExprTreeItem *lviAfter(nullptr);

			if (l > stack.size())
			{
				assert(l - stack.size() == 1);
			}
			else if (l == stack.size())
			{
				if (!stack.empty())
				{
					lviAfter = stack.back();
					stack.pop_back();
				}
			}
			else
			{
				while (l <= stack.size())
				{
					lviAfter = stack.back();
					stack.pop_back();
				}
			}

			//MyString q;
			//if (lviAfter && lviAfter->childCount())
				//q = lviAfter->child(0)->text(0);

			ExprTreeItem *lvi;
			if (stack.empty())
				lvi = m_roots.addRoot(lviAfter);
			else
				lvi = stack.back()->addChild(lviAfter);

			lvi->s = s;
			lvi->sN = sN;
			lvi->sId = sId;
			lvi->sTyp = sTyp;
			lvi->sAddr = sAddr;
			//lvi->setExpanded(l > 1);

			stack.push_back(lvi);
		}

		//delete lviRoot;

#endif

	}
	virtual void fetch(ITEMID) const {}
	virtual bool hasChildren(ITEMID item, bool bFetched) const
	{
		ExprTreeItem *p((ExprTreeItem *)item);
		if (!p)
			return !m_roots.empty();
		return p->hasChildren();
	}
	virtual unsigned childrenNum(ITEMID parent) const
	{
		ExprTreeItem *p((ExprTreeItem *)parent);
		if (!p)
			return (unsigned)m_roots.size();
		return p->childrenNum();
	}
	virtual void data(ITEMID item, size_t column, MyStreamBase &ss) const
	{
		ExprTreeItem *p((ExprTreeItem *)item);
		MyStreamUtil ssu(ss);
		switch (column)
		{
		case 0:	ssu.WriteString(p->s); break;
		case 1: ssu.WriteString(p->sN); break;
		case 2: ssu.WriteString(p->sId); break;
		case 3: ssu.WriteString(p->sTyp); break;
		case 4: ssu.WriteString(p->sAddr); break;
		default: break;
		}
	}
	virtual void path(ITEMID, MyStreamBase &) const {}
	virtual ITEMID idOfChild(ITEMID parent, unsigned childIndex) const
	{
		ExprTreeItem *p((ExprTreeItem *)parent);
		if (!p)
		{
			assert(childIndex < m_roots.size());
			return (ITEMID)m_roots[childIndex];
		}
		assert(childIndex < 2 && p->children[childIndex]);
		return (ITEMID)p->children[childIndex];
	}
	virtual ITEMID idOfParent(ITEMID child) const
	{
		ExprTreeItem *p((ExprTreeItem *)child);
		return (ITEMID)p->parent;
	}
	virtual unsigned indexOf(ITEMID item) const
	{
		ExprTreeItem *p((ExprTreeItem *)item);
		if (p->parent)
			return p->parent->indexOf(p);
		return m_roots.indexOf(p);
	}
	virtual int uniqueOf(ITEMID) const { return 0; }
	virtual ITEMID IdFromUnique(int) const { return 0; }
	virtual void rename(ITEMID, const char *) {}
};



