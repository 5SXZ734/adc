#pragma once

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QRect>

#include "interface/IADCGui.h"

//basic contents drawing element (either fro margin or main area)
class ADCCell : public QString
{
public:
	enum
	{
		OPCODE_NULL,
		OPCODE_DATA,//draw string fragment, op[1]:starting index, op[2]:size
		OPCODE_COLOR,//update fgnd/bgnd color/font, op[1]:Color_t
		OPCODE_TREE//the same as data, but indicates a special hier encoded string (for tree branches)
	};
	struct Code
	{
		unsigned ops[3];
		Code(){
			ops[0] = ops[1] = ops[2] = 0;
		}
		Code(unsigned a, unsigned b, unsigned c = 0){
			ops[0] = a; ops[1] = b; ops[2] = c;
		}
	};
private:
	QVector<Code>	mCodes;
	uint			mFlags;
	int				mWidth;
public:
	ADCCell() : mFlags(0), mWidth(0){}
	void addDataInfo(const QString &s)
	{
		mCodes.push_back(Code(OPCODE_DATA, length(), s.length()));
		append(s);
	}
	void addColorInfo(adcui::Color_t eColor)
	{
		mCodes.push_back(Code(OPCODE_COLOR, eColor));
	}
	void addTreeInfo(const QString &s)
	{
		mCodes.push_back(Code(OPCODE_TREE, length(), s.length()));
		append(s);
	}
	uint codesNum() const { return mCodes.size(); }
	const Code &code(uint i) const { return mCodes[i]; }
	void setFlags(unsigned u){ mFlags = u; }
	void setWidth(unsigned w){ mWidth = w; }
	int width() const { return mWidth; }
	bool isHeader() const { return (mFlags & adcui::CLMNFLAGS_HEADER) != 0; }
	//void setHeader(){ mFlags |= adcui::CLMNFLAGS_HEADER; }
	//void setTreeCell(){ mFlags |= adcui::CLMNFLAGS_TREE; }
	bool isTreeCell() const { return (mFlags & adcui::CLMNFLAGS_TREE) != 0; }
	void setTrimmed(){ mFlags |= adcui::CLMNFLAGS_TRIM; }
	bool isTrimmed() const { return (mFlags & adcui::CLMNFLAGS_TRIM) != 0; }
	bool isShown() const { return mWidth > 0; }
};

///////////////////////////////////////////ADCTextRow
class ADCTextRow0 : private QVector<ADCCell>
{
	typedef QVector<ADCCell>	Row;
public:
	ADCTextRow0() 
	{
	}
	void reset(){ clear(); }
	uint cellsNum() const
	{
		return size();
	}
	const ADCCell &cell(uint col) const
	{
		static const ADCCell gCell;
		if (!(col < (unsigned)size()))
			return gCell;
		return at(col);
	}
	ADCCell &cell(uint col)
	{
		//Q_ASSERT(col < (unsigned)size());
		static ADCCell gCell;
		if (!(col < (unsigned)size()))
			return gCell;
		return (*this)[col];
	}
	void addCell(unsigned iCol, const char *str, bool bUtf8)
	{
		if (!str || !str[0])
			return;
		if (!(iCol < (unsigned)size()))
			resize(iCol + 1);
		ADCCell &aCell((*this)[iCol]);
		if (bUtf8)
			aCell.addDataInfo(QString::fromUtf8(str));
		else
			aCell.addDataInfo(QString::fromLatin1(str));//ascii
	}

	void addColor(unsigned iCol, adcui::Color_t e)
	{
		Row &row(*this);
		if (!(iCol < (unsigned)row.size()))
			row.resize(iCol + 1);
		ADCCell &aCell(row[iCol]);
		aCell.addColorInfo(e);
	}
	virtual void addTree(unsigned iCol, const char *str)//tree info string
	{
		if (!(iCol < (unsigned)size()))
			resize(iCol + 1);
		ADCCell &aCell((*this)[iCol]);
		aCell.addTreeInfo(QString::fromLatin1(str));//ascii
	}
	void setCellFlags(unsigned iCol, unsigned flags)
	{
		if (!(iCol < (unsigned)size()))
			resize(iCol + 1);
		ADCCell &aCell((*this)[iCol]);
		aCell.setFlags(flags);
	}
	void setCellWidth(unsigned iCol, int width)
	{
		if (!(iCol < (unsigned)size()))
			resize(iCol + 1);
		ADCCell &aCell((*this)[iCol]);
		aCell.setWidth(width);
	}

	int cellIndexFromPos(QPoint pt)//warning: it returns an index of visible column in client area
	{
		int cellIndex(0);
		int x(0);
		for (uint icol(0); icol < cellsNum(); icol++)
		{
			const ADCCell &aCell(cell(icol));
			if (aCell.isHeader())
				continue;
			int wcol(aCell.width());
			if (wcol <= 0)
				continue;
			if (x <= pt.x() && pt.x() < x + wcol)
				return cellIndex;
			x += wcol;
			cellIndex++;
		}
		return -1;
	}

	int cellRectFromIndex(int col, int rowScreen, QRect &r) const
	{
		int cellIndex(0);
		int x(0);
		for (uint icol(0); icol < cellsNum(); icol++)
		{
			const ADCCell &aCell(cell(icol));
			if (aCell.isHeader())
				continue;
			int wcol(aCell.width());
			if (wcol <= 0)
				continue;
			if (cellIndex == col)
			{
				r.setLeft(x);
				r.setTop(rowScreen);
				r.setWidth(wcol);
				r.setHeight(1);
				return icol;
			}
			x += wcol;
			cellIndex++;
		}
		return -1;
	}

	int cellRectAtPos(QPoint pt, QRect &r) const
	{
		int x(0);
		for (uint icol(0); icol < cellsNum(); icol++)
		{
			const ADCCell &aCell(cell(icol));
			if (aCell.isHeader())
				continue;
			int wcol(aCell.width());
			if (wcol > 0)
			{
				if (x <= pt.x() && pt.x() < x + wcol)
				{
					r.setLeft(x);
					r.setTop(pt.y());
					r.setWidth(wcol);
					r.setHeight(1);
					return icol;
				}
				x += wcol;
			}
		}

		return -1;
	}

	int colAtPos(int &x0)
	{
		int x(0);//-m_nOffsetChar);
		for (uint icol(0); icol < cellsNum(); icol++)
		{
			const ADCCell &aCell(cell(icol));
			if (aCell.isHeader())
				continue;
			int wcol(aCell.width());
			if (wcol <= 0)
				continue;
			if (x <= x0 && x0 < x + wcol)
			{
				x0 = x;
				return icol;
			}
			x += wcol;
		}
		return -1;
	}
};



class ADCTextRow : public ADCTextRow0
{
	adcui::DUMPOS m_it;
	adcui::Color_t	m_margin;//default color scheme for margin
	adcui::Color_t	m_contents;//default color scheme for client area
	adcui::Color_t	m_underLine;
	bool	mbCurrent;
public:
	ADCTextRow(adcui::DUMPOS it) 
		: m_it(it),
		m_margin(adcui::COLOR_NULL), 
		m_contents(adcui::COLOR_NULL),
		m_underLine(adcui::COLOR_NULL),
		mbCurrent(false)
	{
	}
	void setIter(adcui::DUMPOS it){ m_it = it; }
	adcui::DUMPOS iter() const { return m_it; }
	adcui::DUMPOS line() const { return m_it; }
	adcui::Color_t marginColor() const { return m_margin; }
	adcui::Color_t lineColor() const { return m_contents; } 
	bool isCurrent() const { return mbCurrent; }
	void setMarginColor(adcui::Color_t e){ m_margin = e; }
	void setLineColor(adcui::Color_t e){ m_contents = e; }
	void setCurrent(bool b){ mbCurrent = b; }
	void setUnderLineColor(adcui::Color_t flags){ m_underLine = flags; }
	adcui::Color_t underLineColor() const { return m_underLine; }
};
