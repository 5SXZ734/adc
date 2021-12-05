#include "dump_cache.h"
#include "info_proj.h"

//////////////////////////
// CACHEbin_t

CACHEbin_t::CACHEbin_t(CTypeBasePtr iRoot/*, ROWID scopeBase, ROWID scopeSize*/)
	: mpScopeRoot(iRoot),
	//mScopeDA(scopeBase()),
	//mScopeSize(scopeRange()),
	mScope(scopeBase(), (size_t)scopeRange(), 0, 0, (TypePtr)mpScopeRoot),
	miRowIDLenMax(HexWidthMax(ProjectInfo_t::ViewSize(mpScopeRoot)))
{
//	if (mScope.size == -1)//strucvar?
//		mScope.size = 0;
	assert(mpScopeRoot->typeModule() || mpScopeRoot->isShared());
	//enable_names(decodeAddress);
}

CACHEbin_t::~CACHEbin_t()
{
	for (size_t i(0); i < mPosVec.size(); i++)
		delete mPosVec[i];
	//?releaseTarget();// delete mpTarget;
}

ROWID CACHEbin_t::scopeBase() const
{
	return ProjectInfo_t::ViewOffset(mpScopeRoot);
}

ROWID CACHEbin_t::scopeRange() const
{
	return ProjectInfo_t::ViewSize(mpScopeRoot);
}

bool CACHEbin_t::scopeTo(ROWID scopeDA, ROWID scopeSize)
{
	if (scopeDA < scopeBase() || scopeDA + scopeSize >= scopeBase() + scopeRange())//overflow possible!
		return false;
	mScope.da = scopeBase() + scopeDA;
	mScope.size = (size_t)scopeSize;
	return true;
}

ROWID CACHEbin_t::rowsNum() const
{
	if (ptarget())
		return mScope.size;
	return 0;
}

adcui::DUMPOS CACHEbin_t::NewPosition()
{
	return mPosVec.newPos(new DumpPosition(mScope.da));
	/*size_t i;
	for (i = 0; i < mPos.size(); i++)
		if (!mPos[i])
			break;

	DumpPosition * itCell(new DumpPosition(*this));
	if (i < mPos.size())
	{
		mPos[i] = itCell;
		return i;
	}

	mPos.push_back(itCell);
	return mPos.size() - 1;*/
}

void CACHEbin_t::DeletePosition(adcui::DUMPOS iPos)
{
	delete mPosVec.deletePos(iPos);
}

adcui::DUMPOS CACHEbin_t::PosFromIter(adcui::ITER it) const
{
	return mPosVec.posFromIter(it);
}

void CACHEbin_t::DeleteIterator(adcui::ITER it, bool bUpdatePos)
{
	delete mPosVec.deleteIter(it, bUpdatePos);
}

#if(0)//?
void CACHEbin_t::Invalidate()
{
	//mIts.clear();//check for no inactive iteractives

	for (size_t i(0); i < mPos.size(); i++)
	{
		DumpPosition *itCell(mPos[i]);
		if (itCell)
			itCell->clear(mpSc ope);
	}

	//Complex_t *pScope(target().sco pe());
	//setScope(pScope);
}
#endif

bool CACHEbin_t::IsAtEndIt(adcui::DUMPOS iPos)
{
	DumpPosition &aPos(*mPosVec[iPos]);//->IsAtEnd();
	return !(aPos.rowID() < scopeUppperBound());
}

/*ROWID CACHEbin_t::adjustedRowId(ROWID r0)
{
	if (r0 != ROWID_END)
	{
		if (mpSc ope)
		{
			ROWID r(mpSc ope->VA2 DA(0));
			if (r0 >= r)
			{
				r = r0 - r;
				if (r < mpSco pe->view Size())
					return r;
			}
		}
	}
	return ROWID_END;
}*/

ROWID CACHEbin_t::RowIdIt(adcui::DUMPOS itID)
{
	DumpPosition *itCell(mPosVec[itID]);
	return itCell->rowID();
}

void CACHEbin_t::SetBitPos(adcui::DUMPOS itPos, unsigned uBit)
{
	DumpPosition *itCell(mPosVec[itPos]);
	itCell->da().bit = uBit;
}

int CACHEbin_t::LineIt(adcui::DUMPOS itID)
{
	DumpPosition *itCell(mPosVec[itID]);
	return itCell->line();
}

DA_t CACHEbin_t::toDA(adcui::DUMPOS aPos) const
{
	DumpPosition *p(mPosVec[aPos]);
	return DA_t(p->rowID(), p->line(), p->bit());
}

const char *CACHEbin_t::CellDataIt(adcui::DUMPOS iPos, COLID iCol)
{
	DumpPositionIt &r((DumpPositionIt &)*mPosVec[iPos]);
	return r.CellData(iCol);
}

const char *CACHEbin_t::CellTreeDataIt(adcui::DUMPOS it, COLID iCol)
{
	if (!target().isTreeColumn(iCol))
		return nullptr;
	const DumpPositionIt &r(posIt(it));
	return r.CellData(adcui::IBinViewModel::CLMN_TREEINFO);
}

const char *CACHEbin_t::CellTreeDataIt(adcui::DUMPOS it)
{
	const DumpPositionIt &r(posIt(it));
	return r.CellData(adcui::IBinViewModel::CLMN_TREEINFO);
}

bool CACHEbin_t::IsTreeColumn(COLID iCol)
{
	return target().isTreeColumn(iCol);
}


int CACHEbin_t::pageWidth()
{
	int pageWidth = 0;
	for (size_t i(0); i < target().cols().size(); i++)
	{
		ColDesc_t &aCol(target().Column(COLID(i)));
		if (aCol.width > 0)
			pageWidth += aCol.width;
	}
	return pageWidth;
}

void CACHEbin_t::showColumn(COLID iCol, bool bShow)
{
	ColDesc_t &aCol(target().Column(iCol));
	int w(abs(aCol.width));
	setColumnWidth(iCol, bShow ? (w) : (-w));
}

bool CACHEbin_t::setColumnWidth(COLID iCol, int w)//returns true if switched from hidden to shown or otherwise
{
	ColDesc_t &aCol(target().Column(iCol));
	int w0(aCol.width);
	aCol.width = w;
	// If the column was just shown/hidden - need to update a dump buffer
	// Detect if two integers have opposite signs
	// The XOR of x and y will have the sign bit as 1 iff they have opposite sign.In other words, 
	//     XOR of x and y will be negative number number iff x and y have opposite signs
	if ((w ^ w0) < 0)
	{
		target().resetTreeColumn();
		return true;
	}
	//return w0;
	return false;
}



