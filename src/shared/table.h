#pragma once

#include <string>
#include <vector>
#include <map>
#include <list>
#include <assert.h>
#include "defs.h"

class MyColumn
{
public:
	std::string	name;
	int		width;
	unsigned flags;
	unsigned color;
public:
	MyColumn() : width(0), flags(0), color(0){}
};

class MyColumnVec : public std::vector<MyColumn>
{
public:
	MyColumnVec(){}
	MyColumn &setup(size_t iCol, const char *name, int width, unsigned flags = 0)
	{
		if (!(iCol < size()))
			resize(iCol + 1);
		MyColumn &col(at(iCol));
		col.name = name;
		col.width = width;
		col.flags = flags;
		return col;
	}
};


typedef std::list<int>			listWrap_t;
typedef listWrap_t::iterator	listWrap_it;

class Cell_t : public std::string
{
	listWrap_t	mWrap;
public:
	Cell_t(){}
};

typedef std::map<long, Cell_t>	mapCell_t;
typedef mapCell_t::iterator		mapCell_it;

class Row_t : public mapCell_t
{
	int flags;
public:
	Row_t()
	{
		flags = 0; 
	}
};

typedef std::map<long, MyColumn>	mapColumn_t;
typedef mapColumn_t::iterator		mapColumn_it;

typedef std::map<long, Row_t>	mapRow_t;
typedef mapRow_t::iterator		mapRow_it;


class Table_t
{
public:
	mapColumn_t	mCols;
	mapRow_t	mRows;

public:
	Table_t(){}

	int newColumn(const char * name, int width)
	{
		int colID = (int)mCols.size() + 1;

		std::pair<mapColumn_it, bool> it;
		it = mCols.insert(std::make_pair(colID, MyColumn()));
		if (it.second != false)
		{
			MyColumn &col = it.first->second;
			if (name != nullptr)
				col.name = name;
			col.width = width;
		}
		return colID;
	}

	void newRow(ADDR rowID)
	{
		std::pair<mapRow_it, bool> it;
		it = mRows.insert(std::pair<ADDR, Row_t>(rowID, Row_t()));
		if (it.second != false)
		{
			Row_t &row = it.first->second;
		}
	}

	Row_t &lastRow()
	{
		mapRow_it it = mRows.end();
		if (it == mRows.begin())
		{
			newRow(0);
			it = mRows.end();
		}
		it--;
		return it->second;
	}

	void clear_col(int colID)
	{
		assert(colID < (int)mCols.size());
		MyColumn &col = mCols[colID];
		//?col.clear();
	}

	void AppendText(int colID, const char *s, int sz)
	{
		if (s == nullptr)
			return;

		assert(colID < (int)mCols.size());
		//	Column_t &col = mCols[colID];
		//	col.append( s, sz );
		Row_t &row = lastRow();
		mapCell_it it = row.find(colID);
		if (it == row.end())
		{
			std::pair<mapCell_it, bool> itf;
			itf = row.insert(std::pair<int, Cell_t>(colID, Cell_t()));
			if (!itf.second)
				return;
			it = itf.first;
		}
		Cell_t &cell = it->second;

		if (sz > 0)
		{
			while (sz-- > 0)
				cell += *s++;
			return;
		}
		while (*s)
		{
			cell += *s++;
		}
	}

	void AppendText(int colID, const char c)
	{
		int sz = (int)mCols.size();
		assert(colID < sz);
		MyColumn &col = mCols[colID];
		//?col.push_back(c);
	}

	const char * CellData(int x, ADDR y)
	{
		//assert( x < (int)mCols.size() );
		//Column_t &col = mCols[x];

		mapRow_it it = mRows.find(y);
		if (it == mRows.end())
			return "<nul>";
		Row_t &row = it->second;//?col.data;
		mapCell_it itCell = row.find(x);
		if (itCell == row.end())
			return "<nul>";
		Cell_t &cell = itCell->second;
		return cell.c_str();
	}

	void col_color(int colID, int c)
	{
		assert(colID < (int)mCols.size());
		MyColumn &col = mCols[colID];
		//?col.set_color(c);
	}

	int col_get_color(int colID)
	{
		assert(colID < (int)mCols.size());
		MyColumn &col = mCols[colID];
		return col.color;
	}

	void col_setFlags(int colID, int f)
	{
		assert(colID < (int)mCols.size());
		MyColumn &col = mCols[colID];
		col.flags = f;
	}

	int col_flags(int colID)
	{
		assert(colID < (int)mCols.size());
		MyColumn &col = mCols[colID];
		return col.flags;
	}

	const char * col_name(int colID)
	{
		assert(colID < (int)mCols.size());
		MyColumn &col = mCols[colID];
		return col.name.c_str();
	}

};

