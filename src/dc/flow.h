#pragma once

#include "info_func.h"

struct FlowState_t
{
	struct Elt_t
	{
		PathPtr	path;
		PathPtr	from;
		Elt_t(HPATH p, HPATH from) : path(p), from(from) {}
	};
	std::list<Elt_t>	m_stack;
	std::vector<PathPtr> m_backtrace;
	std::vector<std::pair<size_t, int> >	m_forks;
	void addStack(PathPtr path, PathPtr from) {
		m_stack.push_back(Elt_t(path, from));
	}
};



class FlowIterator : public FuncInfo_t
{
protected:
	struct Elt_t
	{
#ifdef _DEBUG
		int		fromNo;
		int		pathNo;
#endif
		HPATH	path;
		HPATH	from;
		Elt_t(HPATH p, HPATH from)
			: 
#ifdef _DEBUG
			fromNo(-1),
			pathNo(-1),
#endif
			path(p),
			from(from)
		{
		}
	};
	std::list<Elt_t>	m_stack;
public:
	FlowIterator(const FuncInfo_t&);
	FlowIterator(const FuncInfo_t&, const FlowState_t &);
	FlowIterator(const FuncInfo_t &, HPATH pStartPath);

	HPATH operator*() const {
		if (m_stack.empty())
			return HPATH();
		return m_stack.back().path;
	}
	operator bool() const { return !m_stack.empty(); }
	FlowIterator& operator ++();
	//FlowIterator& operator ++(int){ return operator++(); }

	HPATH from() const {
		if (m_stack.empty())
			return HPATH();
		return m_stack.back().from;
	}
	void addStack(HPATH, HPATH from);
	//HOP fromOp() const;

	void drop();//go back to the fork
	void save(FlowState_t&) const;
	void load(const FlowState_t&);
	
};


//with backtracing
class FlowIteratorEx : public FlowIterator
{
	//backtrace element
#ifdef _DEBUG
#if(NEW_PATH_PTR)
	struct BT_t : public HPATH
	{
		int no;
		BT_t() : no(-1) {}
		BT_t(const HPATH& o, int n) : HPATH(o), no(n) {}
	};
#else
	typedef HPATH BT_t;
#endif
#else
	typedef HPATH BT_t;
#endif

	std::vector<BT_t> m_backtrace;
	std::vector<std::pair<size_t, int> >	m_forks;//indeces of m_backtrace[] where branching occured

public:
	FlowIteratorEx(const FuncInfo_t&);
	//FlowIteratorEx(const FuncInfo_t&, const FlowState_t &);
	FlowIteratorEx(const FuncInfo_t &, HPATH from);

	FlowIteratorEx& operator ++();

	std::vector<BT_t> &backtrace(){ return m_backtrace; }
	void addBacktrace(HPATH);

	void drop();//go back to the fork
	void save(FlowState_t&) const;
	void load(const FlowState_t&);

};








