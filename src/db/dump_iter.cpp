#include "dump_iter.h"
#include "dump_target.h"

/////////////////////////////////
// DumpIterator

DumpIterator::DumpIterator(const DumpTarget_t &r)
	: mr(r),
	mRowIt(r.rows().begin()),
	mLine(0),
	mBit(0)
{
}

DumpIterator::DumpIterator(const DumpIterator &o)
	: mr(o.mr),
	mRowIt(o.mRowIt),
	mLine(o.mLine),
	mBit(o.mBit)
{
}

DumpIterator::~DumpIterator()
{
}

DA_t DumpIterator::Reset()
{
	mRowIt = mr.rows().begin();
	mLine = 0;
	mBit = 0;
	return toDA();
}

DA_t DumpIterator::SetAtEnd()
{
	mRowIt = mr.rows().end();
	mLine = 0;
	mBit = 0;
	return toDA();
}

unsigned DumpIterator::row_span() const
{
	RowMapCIt it2(mRowIt);
	++it2;
	if (it2 != mr.rows().end())
	{
		if (IsPosterior(it2))//row span will be indicated by the last posterior line
			return 0;
		if (it2->first == mRowIt->first)
			return 0;
	}
	RowMapCIt it3(mRowIt);
	for (; it3->second.span == 0; --it3)
	{
		assert(it3 != mr.rows().begin());
		assert(IsPosterior(it3) && it3->second.span == 0);
	}
	return it3->second.span;
}

bool DumpIterator::seek(ROWID rowID, unsigned int line, ROWID scopeBase)
{
	Reset();//enable multimap to find the first key in a sequence

	if (mr.rows().empty())
		return false;

	mRowIt = lower_bound(rowID, scopeBase);

	if (mRowIt != mr.rows().end())
	{
		if (RowID() == rowID)
		{
			RowMapCIt it = mRowIt;
			while (line--)
			{
				it++;
				if (it == mr.rows().end())
					break;
				rowID = it->first;
				if (rowID != RowID())
					break;
				mRowIt++;
				mLine++;
			}
			return true;
		}

		assert(RowID() > rowID);
		if (mRowIt == mr.rows().begin())
		{
			mRowIt = mr.rows().end();
			return false;
		}
	}

	--mRowIt;

	assert(RowID() < rowID);
	/*if (rowID >= RowID() + row_span())
	{
		//over the span - return not found
		mRowIt = mr.rows().end();
		return false;
	}*/

	unsigned span(mRowIt->second.span);

	//calculate the last line for overlapped rowid
	RowMapCIt it(mRowIt);
	while (it != mr.rows().begin())
	{
		--it;
		if (it->first != mRowIt->first)//RowID()
			break;
		span = std::max(span, it->second.span);
		mLine++;
	}

	if (rowID >= mRowIt->first + span)
	{
		//over the span - return not found
		mRowIt = mr.rows().end();
		mLine = 0;
		return false;
	}

	return true;
}

RowMapCIt DumpIterator::upper_bound(ROWID rowID, ROWID scope_base) const
{
	RowMapCIt it = mr.rows().upper_bound(rowID);
	for (;;) 
	{
		bool b = (it == mr.rows().end());
		if (b)
			break;
		if (it->first != rowID)
			break;
		it++;
	}
	return it;
}

RowMapCIt DumpIterator::lower_bound(ROWID rowID, ROWID scope_base) const
{
	RowMapCIt it = mr.rows().lower_bound(rowID + scope_base);
	if (it == mr.rows().end())
		return it;
	while (it != mr.rows().begin() && it->first == rowID)
		--it;
	if (it->first < rowID)
		it++;
	return it;
}

bool DumpIterator::isAtEnd() const 
{ 
	return mRowIt == mr.rows().end();
}

bool DumpIterator::isAtBegin() const 
{ 
	return mRowIt == mr.rows().begin();
}

/*int DumpIterator::row_ span()
{
	const char *pc(CellData(adcui::IBinViewModel::CLMN_ROWSPAN));
	if (!pc)
		return 0;
	int len = atoi(pc);
	return len;
}*/

ROWID DumpIterator::RowID() const
{
	if (mr.rows().empty())
		return 0;
	if (mRowIt == mr.rows().end())
		return ROWID_END;
	return mRowIt->first; 
}

short DumpIterator::Bit2() const
{
	if (mRowIt == mr.rows().end())
		return 0;
	return mRowIt->second.bit;
}

int DumpIterator::Adjusted() const
{
	if (mRowIt == mr.rows().end())
		return 0;
	return mRowIt->second.adjusted;
}

ROWID DumpIterator::RowID2() const
{
	if (mr.rows().empty())
		return 0;
	if (mRowIt == mr.rows().end())
		return ROWID_END;
	return mRowIt->first + mRowIt->second.adjusted;
}

bool DumpIterator::hasProblem() const
{
	if (mRowIt == mr.rows().end())
		return false;
	return (mRowIt->second.problem != 0);
}
/*ROWID DumpIterator::RowIDx() const
{
	if (mr.rows().empty())
		return -1;
	if (mRowIt == mr.rows().end())
	{
//		if (mRowIt == drawObj.mRows.begin())
	//		return 0;
		RowMapCIt itr(mRowIt);
		itr--;
		return itr->first+1;
	}
	return mRowIt->first;
}*/

const char * DumpIterator::CellData(COLID iCellIndex) const
{
	if (mRowIt == mr.rows().end())
		return nullptr;

	ROWID rowID = mRowIt->first;
	long rowDirPos = mRowIt->second.pos;

	const char * pInBuf(mr.buffer());
	if (!pInBuf)
		return nullptr;

	long * pDirPos = (long *)&pInBuf[rowDirPos];
	//int iCellIndex = mr.mcolids[colID];
	long lCellPos = pDirPos[iCellIndex];
	const char * pCellData = &pInBuf[lCellPos];
	if (pInBuf <= pCellData && pCellData < pInBuf + mr.size())
	return pCellData;
	return "?";
}

