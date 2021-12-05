#pragma once

#include <vector>
#include <map>
#include <string>
#include <strstream>
#include <list>
#include "qx/IUnk.h"
#include "dump_iter.h"

class DumpTarget_t;
class MyStrStream;
class I_DataSourceBase;

class ColDesc_t
{
public:
	std::string name;
	int width;
	unsigned flags;
	unsigned color;
	//std::string	data;
	//long	pos;
public:
	friend class Table_t;
	ColDesc_t()
		: width(0), flags(0), color(0)//, pos(0)
	{
	}
};


class ColData_t : public std::string
{
public:
	ColData_t(){}
	void writestr(const std::string &s, int iColor)
	{
		if (iColor)
			append(1, (char)MAKECOLORID(iColor));
		append(s);
		if (iColor)
			append(1, (char)MAKECOLORID(adcui::COLOR_POP));
	}
	void writestr(const std::string &s, bool bUnicode, int iColor)
	{
		if (iColor)
		{
			if (bUnicode)
			{
				append("L");
				append(1, (char)MAKECOLORID(iColor));
				append("'");
				append(1, (char)MAKECOLORID(adcui::COLOR_WSTRING));
				size_t l(s.length() / sizeof(char16_t));
				assert(l < 0x10000);//? may be greater
				append((const char *)&l, sizeof(char16_t));
				append(s);
				append(1, (char)MAKECOLORID(adcui::COLOR_POP));//this may not needed
				append("'");
				append(1, (char)MAKECOLORID(adcui::COLOR_POP));
			}
			else
			{
				append(1, (char)MAKECOLORID(iColor));
				append("'");
				append(s);
				append("'");
				append(1, (char)MAKECOLORID(adcui::COLOR_POP));
			}
		}
		else
		{
			if (bUnicode)
				append("L");
			append("'");
			append(s);
			append("'");
		}
	}
};


typedef std::vector<ColDesc_t>	MyColVec;



class DumpInvariant_t : public MyColVec
{
	COLID	mTreeColumn;//index of column with a tree
	bool	mbFullMode;
	bool	mbCompactMode;
	bool	mbHideTypesMode;
	bool	mbResolveRefsMode;
	bool	mbColorsEnabled;
public:
	DumpTarget_t*		mpTarget;//shared among binary views
	//int		mRefsNum;
public:
	DumpInvariant_t(bool bColorsEnabled, bool bPackMode);
	~DumpInvariant_t();
	void initColumns(CTypePtr mod);
	COLID treeColumn() const { return mTreeColumn; }
	void setTreeColumn(COLID i){ mTreeColumn = i; }
	bool setFullMode(bool bOn){
		if (mbFullMode == bOn) 
			return false;
		mbFullMode = bOn;
		return true;
	}
	bool setCompactMode(bool bOn){
		if (mbCompactMode == bOn) 
			return false;
		mbCompactMode = bOn;
		return true;
	}
	bool setValuesOnlyMode(bool bOn){
		if (mbHideTypesMode == bOn)
			return false;
		mbHideTypesMode = bOn;
		return true;
	}
	bool setResolveRefsMode(bool bOn) {
		if (mbResolveRefsMode == bOn)
			return false;
		mbResolveRefsMode = bOn;
		return true;
	}
	bool isFullMode() const { return mbFullMode; }
	bool isCompactMode() const { return mbCompactMode; }
	bool isColorsEnabled() const { return mbColorsEnabled; }
	bool isValuesOnlyMode() const { return mbHideTypesMode; }
	bool isResolveRefsMode() const { return mbResolveRefsMode; }
private:
	void initColumn(COLID colid, const char * name, int width, unsigned flags = 0);
};



class DumpTarget_t : public My::IUnk
{
public:

	class MyColData : public std::vector<ColData_t>
	{
	public:
		ROWID		da;//fake
		ROWID		da2;//real
		unsigned	span;
		unsigned	bit;
	public:
		MyColData(size_t n) : std::vector<ColData_t>(n), da(0), da2(0), span(0), bit(0){}
		long flush(std::ostream &);
	};
private:
	std::ostrstream	*mpOStream;
	DumpInvariant_t	&m_cols;
	std::list<MyColData>	m_data;
	RowMap	mRows;
#if(!NEW_VARSIZE_MAP)
	std::map<ROWID, unsigned>	mCodeSzMap;//cached code chunks sizes 
#endif
	const char	*mpInBuf;
	int			mnInBufSize;
	ROWID		mUpperLimit;
	//smart_vector<DumpIterator *>	mIters;//registered iterators
	std::vector<CObjPtr>	mObjRefs;//named objects
	RowMapIt	mLastRow;

public:
	DumpTarget_t(DumpInvariant_t &);
	~DumpTarget_t();
	const ColDesc_t& Column(COLID iCol) const { return m_cols[iCol]; }
	ColDesc_t& Column(COLID iCol){ return m_cols[iCol]; }
	bool isTreeColumn(COLID iCol);
	void resetTreeColumn();
	ROWID upperLimit() const { return mUpperLimit; }
	const MyColVec& cols() const { return m_cols; }
	MyColVec& cols(){ return m_cols; }
	void coldata_clear(){
		m_data.clear();
	}
	bool coldata_empty() const { return m_data.empty(); }
	size_t coldata_size() const { return m_data.size(); }
	size_t coldata_expand();
	bool coldata_pruneEmptyTail();
	void coldata_trimDanglingTail(unsigned);//in level info
	const MyColData& coldata() const { return m_data.back(); }
	MyColData& coldata(){ return m_data.back(); }
	const ColData_t& coldata(size_t i) const { return m_data.back().at(i); }
	ColData_t& coldata(size_t i){ return m_data.back().at(i); }
	CObjPtr objFromId(unsigned) const;
	bool setFullMode(bool bOn){//no lines limit
		return m_cols.setFullMode(bOn);
	}
	bool isFullMode() const { return m_cols.isFullMode(); }
	bool setCompactMode(bool bOn){
		return m_cols.setCompactMode(bOn);
	}
	bool isCompactMode() const { return m_cols.isCompactMode(); }
	bool setValuesOnlyMode(bool bOn){
		return m_cols.setValuesOnlyMode(bOn);
	}
	bool isValuesOnlyMode() const { return m_cols.isValuesOnlyMode(); }
	bool setResolveRefsMode(bool bOn) {
		return m_cols.setResolveRefsMode(bOn);
	}
	bool isResolveRefsMode() const { return m_cols.isResolveRefsMode(); }
	bool isColorsEnabled() const { return m_cols.isColorsEnabled(); }
public:
	ROWID flushRow(ROWID *, bool, unsigned);
	void discardLastRow();
	void invalidateRows(ROWID);
	void draw(COLID clmn, const std::string &);
	//void drawIndent(int d);
	//void drawLevelInfo(const char * levelInfoStr);
	void SetLevelInfo(int level, uint8_t f);
	uint8_t GetLevelInfo(int level) const;
	void drawRowID(ROWID, unsigned, unsigned w);
	void drawOrdinal(TypePtr, ADDR);
	void drawAddressV(TypePtr, ADDR);
	void drawAddressR(OFF_t);
	void drawCode(MyStrStream &);
	void drawData(const MyString &, int);
	void drawBytes(const void * pBytes, size_t len);
	void drawBytes(const I_DataSourceBase &, OFF_t oBytes, size_t len);
	void drawComment(const MyString &, bool bIfEmptyOnly = false);
	static void drawRowDA(ROWID rowID, unsigned uBit, unsigned w, char buf[32]);
	//void drawRowSpan(unsigned);
#if(!NEW_VARSIZE_MAP)
	int cachedObjSize(ROWID rowID) const;
	void setCachedObjSize(ROWID, ROWID);
#endif
	//void setRowIdLen(int l){ miRowIDLenMax = l; }
	bool begin(ROWID);
	void end();
	//void reset();
	const char *buffer() const { return mpInBuf; }
	int size() const { return mnInBufSize; }
	bool AddRow(const MyColData &, long pos, bool problem);
	const RowMap &rows() const { return mRows; }
	//Complex_t *sco pe() const { return mpSc ope; }
	void drawNameRef(CObjPtr, std::ostream &);
	void appendRowSpan(unsigned);
	static adcui::Color_t fieldColor(CFieldPtr);

protected:
	std::ostrstream	&os(){ return *mpOStream; }
private:
	void flushData(std::strstream &, std::string &);
};




