#pragma once

#include <map>
#include "interface/IADCGui.h"
#include "shared/defs.h"
#include "shared/misc.h"
#include "mem.h"

class DumpTarget_t;

struct T_RowData
{
	long	pos;
	unsigned	span;
	unsigned short	adjusted;//ROWID + this = actual DA
	unsigned short	bit:8;//0-255
	unsigned short	problem:1;
	unsigned short	separator:1;
	T_RowData()
		: pos(0), span(0), adjusted(0), bit(0), problem(0), separator(0){}
	T_RowData(long p, unsigned s, unsigned short _adjusted, unsigned short _bit, bool b)
		: pos(p), span(s), adjusted(_adjusted), bit(_bit), problem(b), separator(0){}
};

typedef std::multimap<ROWID, T_RowData>	RowMap;
typedef RowMap::const_iterator	RowMapCIt;
typedef RowMap::iterator	RowMapIt;
typedef int		COLID;
typedef const unsigned char dumpstr_t;

////////////////////////////////////////////// DumpIterator
class DumpIterator
{
	const DumpTarget_t	&mr;
public:
	RowMapCIt	mRowIt;
	short		mLine;
	short		mBit;
public:
	friend class DumpTarget_t;	
	DumpIterator(const DumpTarget_t &);
	DumpIterator(const DumpIterator &);
	~DumpIterator();

	static ROWID ROWIDX(const RowMapCIt &i){ return i->first + i->second.adjusted; }
	static bool IsPosterior(const RowMapCIt &i){ return i->second.adjusted != 0; }
	bool IsPost() const { return mRowIt->second.adjusted != 0; }
	unsigned span() const { return mRowIt->second.span; }

	DA_t Reset();//set at beginning
	DA_t SetAtEnd();
	ROWID RowID() const;
	ROWID RowID2() const;
	//ROWID RowIDx() const;
	short Line() const {
		return mLine;
	}
	short Bit() const {
		return mBit;
	}
	short Bit2() const;
	int Adjusted() const;
	bool hasProblem() const;
	const char *CellData(COLID) const;
	unsigned row_span() const;
	bool seek(ROWID, unsigned int, ROWID);
	RowMapCIt lower_bound(ROWID, ROWID) const;
	RowMapCIt upper_bound(ROWID, ROWID) const;
	bool isAtEnd() const;
	bool isAtBegin() const;
	DA_t toDA() const {
		return DA_t(RowID(), Line(), Bit());
	}
};

#define ROWID_END	(ROWID)-1

class DumpPosition
{
public:
	DA_t	mda;
public:
	DumpPosition(){}
	DumpPosition(ROWID r) : mda(r, 0, 0){}
	DumpPosition(const DumpPosition &o) : mda(o.mda){}
	virtual ~DumpPosition(){}//!
	DumpPosition &operator=(const DumpPosition &o)
	{
		mda = o.mda;
		return *this;
	}
	DA_t &da(){ return mda; }
	const DA_t &da() const { return mda; }
	ROWID rowID() const { return mda.row;  }
	unsigned short line() const { return mda.line; }
	unsigned short bit() const { return mda.bit; }
};

class DumpPositionIt : public DumpPosition
{
public:
	DumpIterator	mIt;
public:
	DumpPositionIt(const DumpTarget_t &r) : mIt(r){}
	DumpPositionIt(const DumpPositionIt &o) : DumpPosition(o), mIt(o.mIt){}
	const char *CellData(COLID iCol) const { return mIt.CellData(iCol); }
	DA_t da2() const { return DA_t(mIt.RowID2(), mIt.Line(), mIt.Bit2()); }
	int	adjusted() const { return mIt.Adjusted(); }
	short bit2() const { return mIt.Bit(); }
	bool hasProblem() const { return mIt.hasProblem(); }
};



