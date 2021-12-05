#include "flow.h"
#include "type_funcdef.h"


/////////////////////////////////////////////////////////////
// FlowIterator

FlowIterator::FlowIterator(const FuncInfo_t &r)
	: FuncInfo_t(r)
{
	addStack(TreeTerminalFirst(mrFuncDef.Body()), HPATH());
}

FlowIterator::FlowIterator(const FuncInfo_t& r, const FlowState_t& state)
	: FuncInfo_t(r)
{
	load(state);
}

FlowIterator::FlowIterator(const FuncInfo_t &r, HPATH pStartPath)
	: FuncInfo_t(r)
{
	assert(pStartPath->IsTerminal());
	//assert(mrFunc.IsMinePath(pStartPath));
	addStack(pStartPath, HPATH());
}

void FlowIterator::addStack(HPATH path, HPATH from)
{
	m_stack.push_back(Elt_t(path, from));
#ifdef _DEBUG
	m_stack.back().pathNo = PathNo(path);
	if (from)
		m_stack.back().fromNo = PathNo(from);
#endif
}

FlowIterator& FlowIterator::operator ++()
{
	if (m_stack.empty())
		return *this;

	HPATH pPath(operator*());//a back of the stack

#if _DEBUG
	int pathNo(PathNo(pPath));
#endif

	assert(!m_stack.empty());
	m_stack.pop_back();
	//if (!m_stack.empty())
	{
		if (PathType(pPath) == BLK_JMP)
		{
			addStack(GetGotoPath(pPath), pPath);
//			addBacktrace(pPath);
		}
		else if (PathType(pPath) == BLK_JMPSWITCH)//switch
		{
//			if (m_backtrace.empty() || m_backtrace.back() != pPath)
			{
				HPATH pJumpTablePath(GetJumpTablePath(pPath));
				if (!pJumpTablePath)//?
					return *this;
				int iCount(0);
				assert(!PathOps(pJumpTablePath).empty());
				for (PathOpList_t::Iterator i(PathOps(pJumpTablePath)); i; i++)
				{
					HPATH pPathNx(PathRef(PRIME(i.data())));
					//if (pPathNx != pPath)
					{
						addStack(pPathNx, pPath);
						iCount++;
					}
				}
//				addBacktrace(pPath);
//				m_forks.push_back(std::make_pair(m_backtrace.size(), iCount - 1));
			}
		}
		else if (PathType(pPath) == BLK_JMPIF)
		{
			//if (m_stack.empty() || m_stack.back() != hPathNx)
//			if (m_backtrace.empty() || m_backtrace.back() != pPath)
			{
				HPATH hPathNo(TreeNextEx(pPath));
				HPATH hPathYes(GetGotoPath(pPath));
				if (hPathNo)
					addStack(hPathNo, pPath);
				if (hPathYes)// && hPathYes != pPath)
					addStack(hPathYes, pPath);
//				addBacktrace(pPath);
//				if (hPathNo && hPathYes)
//					m_forks.push_back(std::make_pair(m_backtrace.size(), 1));
			}
		}
		else if (PathType(pPath) == BLK_EXIT)
		{
			STOP
		}
		else
		{
			addStack(TreeNextEx(pPath), pPath);
//			addBacktrace(pPath);
		}

		return *this;
	}

	return *this;
}

void FlowIterator::drop()//go back to the fork
{
	assert(!m_stack.empty());
	m_stack.pop_back();
}

/*HPATH FlowIterator::from() const
{
	if (!m_backtrace.empty())
		return m_backtrace.back();
	return HPATH();
}

HOP FlowIterator::fromOp() const
{
	if (!m_backtrace.empty())
		return PRIME(m_backtrace.back()->ops().back());
	return HOP();
}*/

void FlowIterator::save(FlowState_t& out) const
{
	out.m_stack.clear();
	out.m_backtrace.clear();
	out.m_forks.clear();
	for (auto a : m_stack)
		out.addStack(a.path, a.from);
}

void FlowIterator::load(const FlowState_t& in)
{
	m_stack.clear();
	for (auto a : in.m_stack)
		addStack(a.path, a.from);
}






/////////////////////////////////////////////////////////////
// FlowIteratorEx

FlowIteratorEx::FlowIteratorEx(const FuncInfo_t &r)
	: FlowIterator(r)
{
}

/*FlowIteratorEx::FlowIteratorEx(const FuncInfo_t& r, const FlowState_t& state)
	: FlowIteratorEx(r)
{
	load(state);
}*/

FlowIteratorEx::FlowIteratorEx(const FuncInfo_t &r, HPATH pStartPath)
	: FlowIterator(r, pStartPath)
{
}

void FlowIteratorEx::addBacktrace(HPATH h)
{
#ifdef _DEBUG
#if(NEW_PATH_PTR)
	m_backtrace.push_back(BT_t(h, PathNo(h)));
#else
	m_backtrace.push_back(h);
#endif
#else
	m_backtrace.push_back(h);
#endif
}

FlowIteratorEx& FlowIteratorEx::operator ++()
{
	if (m_stack.empty())
		return *this;

	HPATH pPath(operator*());//a back of the stack

#if _DEBUG
	int pathNo(PathNo(pPath));
#endif

	assert(!m_stack.empty());
	m_stack.pop_back();
	{
		if (PathType(pPath) == BLK_JMP)
		{
			addStack(GetGotoPath(pPath), pPath);
			addBacktrace(pPath);
		}
		else if (PathType(pPath) == BLK_JMPSWITCH)//switch
		{
			if (m_backtrace.empty() || m_backtrace.back() != pPath)
			{
				HPATH pJumpTablePath(GetJumpTablePath(pPath));
				if (!pJumpTablePath)//?
					return *this;
				int iCount(0);
				assert(!PathOps(pJumpTablePath).empty());
				for (PathOpList_t::Iterator i(PathOps(pJumpTablePath)); i; i++)
				{
					HPATH pPathNx(PathRef(PRIME(i.data())));
					addStack(pPathNx, pPath);
					iCount++;
				}
				addBacktrace(pPath);
				m_forks.push_back(std::make_pair(m_backtrace.size(), iCount - 1));
			}
		}
		else if (PathType(pPath) == BLK_JMPIF)
		{
			if (m_backtrace.empty() || m_backtrace.back() != pPath)
			{
				HPATH hPathNo(TreeNextEx(pPath));
				HPATH hPathYes(GetGotoPath(pPath));
				if (hPathNo)
					addStack(hPathNo, pPath);
				if (hPathYes)
					addStack(hPathYes, pPath);
				addBacktrace(pPath);
				if (hPathNo && hPathYes)
					m_forks.push_back(std::make_pair(m_backtrace.size(), 1));
			}
		}
		else if (PathType(pPath) == BLK_EXIT)
		{
		}
		else
		{
			addStack(TreeNextEx(pPath), pPath);
			addBacktrace(pPath);
		}

		return *this;
	}

	return *this;
}

void FlowIteratorEx::drop()//go back to the fork
{
	FlowIterator::drop();

	if (!m_forks.empty())
	{
		size_t n(m_forks.back().first);
		assert(m_backtrace.size() >= n);
		assert(m_forks.back().second > 0);
		m_forks.back().second--;
		m_backtrace.resize(n);
		if (m_forks.back().second == 0)
			m_forks.pop_back();
		if (m_forks.empty() && m_stack.empty())
			m_backtrace.clear();//up to the starting poin - no more forks
		//if (m_backtrace.back() != hPath)//we have already seen it
		if (!m_backtrace.empty())
			addStack(m_backtrace.back(), HPATH());//?
	}
}

void FlowIteratorEx::save(FlowState_t& out) const
{
	FlowIterator::save(out);

	out.m_backtrace.reserve(m_backtrace.size());
	for (size_t i(0); i < m_backtrace.size(); i++)
		out.m_backtrace.push_back(m_backtrace[i]);
	out.m_forks.reserve(m_forks.size());
	for (size_t i(0); i < m_forks.size(); i++)
		out.m_forks.push_back(m_forks[i]);
}

void FlowIteratorEx::load(const FlowState_t& in)
{
	FlowIterator::load(in);

	m_backtrace.clear();
	m_backtrace.reserve(in.m_backtrace.size());
	for (size_t i(0); i < in.m_backtrace.size(); i++)
		addBacktrace(HPATH(in.m_backtrace[i]));

	m_forks.clear();
	m_forks.reserve(in.m_forks.size());
	for (size_t i(0); i < in.m_forks.size(); i++)
		m_forks.push_back(in.m_forks[i]);
}