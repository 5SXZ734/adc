#pragma once

#include "dump_iter.h"
#include "dump_target.h"
#include "type_module.h"//Section_t


/*struct column_info_t
{
	std::string name;
	int width;
	unsigned flags;
	unsigned color;
	column_info_t() : width(0), flags(0), color(0){}
};*/


class CACHEbin_t
{
public:
	CoolIter<DumpPosition, adcui::DUMPOS, adcui::ITER>	mPosVec;

private:
	//std::vector<column_info_t>	mcolsw;//column widths
	CTypeBasePtr mpScopeRoot;
	//ROWID	mScopeDA;
	//ROWID	mScopeSize;
	Section_t	mScope;//default
	unsigned	miRowIDLenMax;

public:
	CACHEbin_t(CTypeBasePtr/*, ROWID, ROWID*/);
	virtual ~CACHEbin_t();
	bool scopeTo(ROWID, ROWID);
	CTypeBasePtr scopeRoot() const { return mpScopeRoot; }
	//ROWID scopeUppperBound() const { return mScopeBase + mpScope->viewSize(); }
	ROWID scopeLowerBound() const { return mScope.da; }
	ROWID scopeUppperBound() const { return mScope.da + (mScope.size == 0 ? 1 : mScope.size); }
	unsigned	scopeAddressRangeWidth() const {
		return (mScope.size == 0) ? 1 : miRowIDLenMax; }

	virtual DumpTarget_t *ptarget() const = 0;
	DumpTarget_t &target(){ return *ptarget(); }
	const DumpTarget_t &target() const {
		const DumpTarget_t *p(ptarget());
		assert(p);
		return *p; }
	//virtual DumpTarget_t *aquireTarget() = 0;
	//virtual void releaseTarget() = 0;
	ROWID scopeBase() const;
	ROWID scopeRange() const;
	const Section_t& scope() { return mScope; }

	DumpPositionIt &posIt(adcui::DUMPOS iPos) const
	{
		assert((size_t)iPos < mPosVec.size());
		DumpPosition *p(mPosVec[iPos]);
		assert(dynamic_cast<DumpPositionIt *>(p));
		return (DumpPositionIt &)*p;
	}

	DumpPositionIt &Iter2PosIt(adcui::ITER it) const
	{
		adcui::DUMPOS iPos(PosFromIter(it));
		return posIt(iPos);
	}

	ROWID rowsNum() const;

	bool setFullMode(bool bOn){ return target().setFullMode(bOn); }
	bool setCompactMode(bool bOn){ return target().setCompactMode(bOn); }
	bool isCompactMode() const { return target().isCompactMode(); }
	bool setValuesOnlyMode(bool bOn){ return target().setValuesOnlyMode(bOn); }
	bool isValuesOnlyMode() const { return target().isValuesOnlyMode(); }
	bool setResolveRefsMode(bool bOn) { return target().setResolveRefsMode(bOn); }
	bool isResolveRefsMode() const { return target().isResolveRefsMode(); }


	int pageWidth();
	void showColumn(int, bool);
	int columnWidth(COLID iCol) const { return target().Column(iCol).width; }
	bool setColumnWidth(COLID iCol, int w);
	const char *columnName(COLID iCol) const { return target().Column(iCol).name.c_str(); }
	unsigned columnFlags(COLID iCol) const { return target().Column(iCol).flags; }
	void setColumnFlags(COLID iCol, unsigned f)
	{
		target().Column(iCol).flags = f;
		//if (ptarget())
			//target().Column(iCol).SetFlags(f);
	}
	unsigned columnColor(COLID iCol) const { return target().Column(iCol).color; }
	size_t colsNum() const { return target().cols().size(); }

	//void Invalidate();
	adcui::DUMPOS PosFromIter(adcui::ITER) const;
	adcui::DUMPOS NewPosition();
	void DeletePosition(adcui::DUMPOS);


	void DeleteIterator(adcui::ITER, bool bUpdatePos = false);

	//void ResetIt(adcui::DUMPOS);
	bool IsAtEndIt(adcui::DUMPOS);
	ROWID RowIdIt(adcui::DUMPOS);
	int LineIt(adcui::DUMPOS);
	void SetBitPos(adcui::DUMPOS, unsigned);
	DA_t toDA(adcui::DUMPOS) const;
	const char * CellDataIt(adcui::DUMPOS, COLID);
	const char * CellTreeDataIt(adcui::DUMPOS, COLID);
	const char *CellTreeDataIt(adcui::DUMPOS);
	bool IsTreeColumn(COLID);
	FieldPtr FieldFromIt(adcui::ITER) const;
};



