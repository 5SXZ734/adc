#include <limits>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QLineEdit>
#include <QTextStream>
#include <QClipboard>
#include <QGuiApplication>
#include "ADCTableView.h"
#include "colors.h"
#include "ADCUtils.h"


#define COL_WIDTH_MAX	256

static void draw3DRect(QPainter *p, QRectF r)
{
	p->setPen(Qt::white);
	p->drawLine(r.topLeft(), r.topRight());
	p->drawLine(r.topLeft(), r.bottomLeft());
	p->setPen(Qt::lightGray);
	p->drawLine(r.bottomLeft(), r.bottomRight());
	p->drawLine(r.topRight(), r.bottomRight());
}




///////////////////////////// ADCTableView

ADCTableView::ADCTableView(QWidget *parent, const char *name)
: ADCTextView(parent, name),
//mbShowHeader(true),
m_nSelCol(-1),
m_nResizing(-1),
mbDrawGrid(false),
mpLineEdit(nullptr)//,
//mCurrentRow(0)
{
	//mTopMargin = 1;
	//viewport()->setContentsMargins(0, lineHeight(), 0, 0);
	mTreePixmaps[0].load(":toggle-small.png");
	mTreePixmaps[1].load(":toggle-small-expand.png");
}

void ADCTableView::DrawTopMargin(QPainter *p, const QRect &rc0, bool bTopLeftCorner, const ADCTextRow &aRow)
{
	//QRect rc0(0, 0, p->device()->width(), p->device()->height());

	//int char_width = p->fontMetrics().width("0");

	QRect rc = rc0;

	p->fillRect(rc, QBrush(QColor(242, 242, 242)));// Qt::lightGray));

	if (!bTopLeftCorner)
		p->translate(QPoint(-m_nOffsetChar * charWidth(), 0));

	QPen pen(Qt::white);
	p->setPen(pen);

	for (uint col(0); col < aRow.cellsNum(); col++)
	{
		const ADCCell & aCell(aRow.cell(col));

		if (bTopLeftCorner)
		{
			if (!aCell.isHeader())
				continue;
		}
		else
		{
			if (aCell.isHeader())
				continue;
		}

		int w(aCell.width());
		if (w <= 0)
			continue;

		QString sData(aCell);

		rc.setRight(rc.left() + w * charWidth() - 1);

		draw3DRect(p, QRect(rc.left(), rc.top(), rc.width() - 1, rc.height() - 1));
		p->setPen(QPen(Qt::darkGray));
		QRect rc2(rc);
		rc2.setTop(rc2.top() - 2);
		p->drawText(rc2, Qt::TextSingleLine | Qt::AlignHCenter, sData);

		rc.setLeft(rc.right() + 1);
	}
}

void ADCTableView::drawTree(QPainter *p, QPointF &pt0, const QString &levelStr, int)//int drawIt )
{
	int level(levelStr.length());//pLevelStr[0]-'0';
	if (level <= 0)
		return;

	QPointF pt0_s = pt0;
	qreal w(charWidth());
	qreal h(lineHeight());

	qreal w2(w * 2.0);
	qreal w05(w * 0.5);
	qreal h05(h * 0.5);

	/*const static QColor g_colors[4] = {
		Qt::gray,//{0xE0, 0xE0, 0xE0},//default (light gray)
		QColor(224, 176, 128),//{128, 255, 128},//func decompiled (light green)
		Qt::red,//func being decompiled
		QColor(160, 192, 248)//{ 128, 128, 0 }//shared types
	};*/
	//QPen pens[2] = {QPen(g_colors[0]), QPen(g_colors[1], 2)};

	uint n(0);
	qreal y(pt0.y() + h05);
	for (int i(0); i < level; i++)
	{
		qreal x(pt0.x() + w05);

		uint f0(levelStr[i].cell() - '0');
		uint iColor(f0 >> 5);
		//Q_ASSERT(iColor < sizeof(g_colors)/sizeof(g_colors[0]));
		uint f(f0 & adcui::ITEM_TREE_MASK);
		if (f)
		{
			n |= f;
			if (iColor == 0)
				ColorFromId(p, adcui::COLOR_TREE);
				//p->setPen(QPen(QBrush(g_colors[iColor]), 1, Qt::DotLine));// ColorFromId(COLOR_TREE));
			else
				ColorFromId(p, adcui::Color_t(adcui::COLOR_TREE+iColor));
				//p->setPen(g_colors[iColor]);
				//p->setPen(QPen(QBrush(g_colors[iColor], Qt::Dense4Pattern), 2, Qt::DotLine));// ColorFromId(COLOR_TREE));

			drawTreeCode(p, x, y, f, nullptr);
		}
		pt0.setX(pt0.x() + w2);
	}

	if (n == 0)
	{
		pt0.setX(pt0_s.x());
	}
}

static void drawShadowLine(QPainter *p, qreal x, qreal y, qreal x2, qreal y2, QPen *penShadow)
{
	QPointF a(x, y);
	QPointF b(x2, y2);
	p->drawLine(a, b);
	if (penShadow)
	{
		p->save();
		p->setPen(*penShadow);
		QPointF c;
		if (x == x2)
			c = QPointF(1, 0);
		else
			c = QPointF(0, -1);
		//QPoint c(1, 1);
		a += c;
		b += c;
		p->drawLine(a, b);
		p->restore();
	}
}

void ADCTableView::drawTreeCode(QPainter *p, qreal x, qreal y, uint f, QPen *penShadow)
{
	qreal w(charWidth());
	qreal h(lineHeight());
	qreal w2(w * 2.0);
	qreal h05(h / 2.0);

#if(0)
	if (0 && mscr_line - 1 == 0 /*&& f == ITEM_HAS_SIBLINGS*/)
	{
		int x1 = x - w2;
		//int x2 = x+w2;
		int y1 = y - h05;
		int y2 = y + h05;
		p->drawLine(x1, y, x1, y2);
		//for ( int d = 1; d < 7; d++ )
		//{
		//	p->drawLine( x1-d, y1+d, x1-d, y2 );
		//	p->drawLine( x1+d, y1+d, x1+d, y2 );
		//}
		int dx = 3;

		p->drawLine(x1 - dx, y, x1, y1);
		p->drawLine(x1, y1, x1 + dx, y);
		p->drawLine(x1 + dx, y, x1 - dx, y);

		//QPointArray pa(2);
		//pa.setPoint( 0, x1-dx, y2 );
		//pa.setPoint( 1, x1, y1 );
		//pa.setPoint( 2, x1+dx, y2 );
		//p->drawLineSegments( pa );
	}
	else
#endif
	{
		if (f & adcui::ITEM_HAS_CHILD_HERE)//0x01
		{
			qreal y2 = y - h05;
			drawShadowLine(p, x, y - 1, x, y2, penShadow);//VERT beam up from origin
		}

		if (f & adcui::ITEM_HAS_PARENT)//0x02
		{
			qreal x2 = x - w2;
			drawShadowLine(p, x /*- 1*/, y, x2 + 1, y, penShadow);//HORZ beam to the left of origin
		}

		if (f & adcui::ITEM_HAS_NEXT_CHILD)//0x04
		{
			qreal y2 = y + h05;
			drawShadowLine(p, x, y + 1, x, y2, penShadow);//VERT beam down from origin
		}

		/*if (f & adcui::ITEM_UPPER_BOUND)
		{
		int x2 = x + w2 + w2;
		int y2 = y + h05;
		p->drawLine(x, y2, x2, y2);//HORZ beam to the left of origin
		}*/

		const int S = 4;//square half size minus 1/2
		if (f == (adcui::ITEM_HAS_PARENT | adcui::ITEM_HAS_NEXT_CHILD)
			|| (f == adcui::ITEM_HAS_NEXT_CHILD)
			|| (f & adcui::ITEM_CLOSED)//root item
			)
		{
#if(0)
			QRect rt(x - S, y - S, S * 2 + 1, S * 2 + 1);
			p->eraseRect(rt);
			p->drawRect(rt);
			//if ( !(f & adcui::ITEM_CLOSED) )
			//	p->drawLine( x, y, x, y+h05 );
			p->drawLine(rt.left() + 2, y, rt.right() - 2, y);
			if (f & adcui::ITEM_CLOSED)
				p->drawLine(x, rt.top() + 2, x, rt.bottom() - 2);
#else
			if (f & adcui::ITEM_CLOSED)
				p->drawPixmap(QPointF(x - S, y - S), mTreePixmaps[1]);
			else
				p->drawPixmap(QPointF(x - S, y - S), mTreePixmaps[0]);
#endif
		}
		/*else if (f == adcui::ITEM_HAS_NEXT_CHILD)
		{
		if (f & adcui::ITEM_CLOSED)
		p->drawPixmap(x - S, y - S, mTreePixmaps[1]);
		else
		p->drawPixmap(x - S, y - S, mTreePixmaps[0]);
		}*/
	}
}

void ADCTableView::DrawCaret(QPainter &P0, int x, int y)
{
	int x0(x);
	int iCol(currentRow().colAtPos(x0));//x0 will contain a column's start position
	if (iCol != -1 && currentRow().cell(iCol).isTreeCell())
	{
		Q_ASSERT(x >= x0);
		int cp(x - x0);//position in a column

		//int cp(curIt().x());
		int cp05(cp >> 1);
		int l(mTreeInfo.length());
		if (cp05 < l && !(cp & 1))
		{
			uint f0(mTreeInfo[cp05].cell() - '0');
			uint f(f0 & adcui::ITEM_TREE_MASK);
			if (f != 0)
			{
				QPen pen(Qt::black, 2);
				P0.setPen(pen);
				P0.setCompositionMode(QPainter::RasterOp_NotSourceXorDestination);

				qreal w(charWidth());
				qreal h(lineHeight());

				//int w2(w << 1);
				qreal w05(w / 2.0);
				qreal h05(h / 2.0);

				int xx(x + leftMargin() - m_nOffsetChar);
				int yy(y + topMargin());

				xx += adcui::IADCTableModel::colTextShift(x0);
				
				QPen penShadow(Qt::yellow, 4);
				drawTreeCode(&P0, xx * w + w05, yy * h + h05, f, nullptr);//&penShadow);
				return;
			}
		}
	}
	ADCTextView::DrawCaret(P0, x, y);
}

void ADCTableView::syncViewportMargins(bool)// bOverTop)
{
	int w(0);
	if (hasModel())
	{
		ADCTextRow aHeader(adcui::DUMPOS(0));
		getColumnInfo(aHeader, true);

		//viewport()->setContentsMargins(0, lineHeight(), 0, 0);
		for (uint col(0); col < aHeader.cellsNum(); col++)
		{
			const ADCCell &aCell(aHeader.cell(col));
			if (aCell.isHeader())
			{
				int wcol(aCell.width());
				if (wcol > 0)
					w += wcol;
			}
		}
	}
	//else
		//viewport()->setContentsMargins(0, 0, 0, 0);
	mMargin.setWidth(w);
	if (mMargin.height() != 0)
		mMargin.setHeight(1);// bOverTop ? 1 : 0);
	ADCTableViewBase::syncViewportMargins();
}

int ADCTableView::DrawMarginCell(QPainter &p, const QRect &rc2, const ADCCell &aCell)
{
	p.drawText(rc2, Qt::TextSingleLine | Qt::AlignHCenter, aCell);
	return aCell.length();
}

void ADCTableView::DrawLeftMargin(QPainter *pPainter, const ADCTextRow &aRow, int nLineCount)
{
	ADCColorPair saved1(m_clrBkgnd);
	ADCColorPair saved2(mMarginColor);
	if (aRow.lineColor() != adcui::COLOR_NULL)
		m_clrBkgnd = MapColorPair(aRow.lineColor());
	if (aRow.marginColor() != adcui::COLOR_NULL)
		mMarginColor = MapColorPair(aRow.marginColor());

	//adcui::IADCTableModel *pIModel(hasModel() ? tableModel() : nullptr);
	QFontMetricsF fm(pPainter->font());
	QRectF rc(0, 0, pPainter->device()->width(), pPainter->device()->height());
	pPainter->fillRect(rc, QBrush(mMarginColor.first));// Qt::lightGray));

	ADCTextRow	aHeaderRow(adcui::DUMPOS(0));
	bool bTopLabels(viewport()->contentsMargins().top() <= 0);
	if (bTopLabels)
		getColumnInfo(aHeaderRow, false);

	//for (int col(0); col < pIModel->colsNum(); col++)
	for (uint col(0); col < aRow.cellsNum(); col++)
	{
		const ADCCell &aCell(aRow.cell(col));
		if (!aCell.isHeader())
			continue;

		int wcol(aCell.width());
		if (wcol <= 0)
			continue;

		rc.setRight(rc.left() + wcol*charWidth() - 1);

		QString sData;
		const ADCCell &aCell2(nLineCount == 0 && bTopLabels ? aHeaderRow.cell(col) : aCell);

		//QString sData(pcData);

		int iDataLen(0);
//		if (!sData.isEmpty())
		{
			draw3DRect(pPainter, QRectF(rc.left(), rc.top(), rc.width(), rc.height()));

			//QPointF pt(rc.left(), rc.bottom() - fm.descent());

//			pPainter->save();
			pPainter->setPen(QPen(mMarginColor.second));
			QRectF rc2(rc);
			rc2.setTop(rc2.top());
			//rc2.translate(0, -(fm.descent() + 1));
			//p->drawText(rc2, Qt::TextSingleLine | Qt::AlignHCenter, sData);
			//iDataLen = DrawMarginCell(*pPainter, rc2, aRow.cell(col));
			iDataLen = drawCell(pPainter, rc2, col, aCell2, aRow.isCurrent());
//			pPainter->restore();
		}

		if (iDataLen > wcol)
		{
			if (!(mColAutoExpandInfo.size() > (int)col))
				mColAutoExpandInfo.resize(col + 1);
			mColAutoExpandInfo[col] = qMax(mColAutoExpandInfo[col], iDataLen - wcol);
		}

		//qreal dpi = charWidth();
		//qreal psz = pPainter->font().pointSizeF();
		rc.setLeft(rc.right() + 1);
	}

	adcui::PixmapEnum iPix(textModel()->pixmapIt(aRow.iter()));
	if (iPix != adcui::PIXMAP_NULL)
	{
		Q_ASSERT((iPix-1) < mPixmaps.size());
		pPainter->drawPixmap(rc.x(), rc.y(), mPixmaps[iPix-1]);
	}

	m_clrBkgnd = saved1;
	mMarginColor = saved2;
}

class MyGetRowInfo : public adcui::IADCTableRow
{
	ADCTextRow &aRow;
public:
	MyGetRowInfo(ADCTextRow &r)
		: aRow(r){
	}
protected:
	virtual void addCell(unsigned iCol, const char *str, bool bUtf8){ aRow.addCell(iCol, str, bUtf8); }
	virtual void addColor(unsigned iCol, adcui::Color_t eColor){ aRow.addColor(iCol, eColor); }
	virtual void addTree(unsigned iCol, const char *str){ aRow.addTree(iCol, str); }
	virtual void setLineColor(adcui::Color_t eColor){ aRow.setLineColor(eColor); }
	virtual void setMarginColor(adcui::Color_t eColor){ aRow.setMarginColor(eColor); }
	virtual void setCellFlags(unsigned iCol, unsigned flags){ aRow.setCellFlags(iCol, flags); }
	virtual void setCellWidth(unsigned iCol, int width){ aRow.setCellWidth(iCol, width);  }
	virtual void setUnderLineColor(adcui::Color_t eColor){ aRow.setUnderLineColor(eColor); }
};

void ADCTableView::getColumnInfo(ADCTextRow &aRow, bool bNoData)
{
	MyGetRowInfo aGetRow(aRow);
	tableModel()->getColumnInfo(aGetRow, bNoData);
}

void ADCTableView::getRowInfo(ADCTextRow &aRow)
{
	MyGetRowInfo aGetRow(aRow);
	tableModel()->getRowDataIt(aRow.iter(), aGetRow);
}

void ADCTableView::OnCurrentRow(ADCTextRow &aRow)
{
	currentRow().asTextRow() = aRow;
}

void ADCTableView::OnNewRow(ADCTextRow &aRow)
{
	int last(-1);
	for (uint col(0); col < aRow.cellsNum(); col++)
	{
		ADCCell &aCell(aRow.cell(col));
		if (!aCell.isHeader() && aCell.isShown())
		{
			if (aCell.isTreeCell())
			{
				if (last >= 0)
					aRow.cell(last).setTrimmed();//make sure a cell before the one with a tree info is always trimmed
				break;
			}
			last = col;
		}
	}

	ADCTextView::OnNewRow(aRow);
}

bool ADCTableView::DrawContentsLine(QPainter *p, const ADCTextRow &aRow, int nLineCount)
{
	ADCColorPair saved1(m_clrBkgnd);
	ADCColorPair saved2(mMarginColor);
	if (aRow.lineColor() != adcui::COLOR_NULL)
		m_clrBkgnd = MapColorPair(aRow.lineColor());
	if (aRow.marginColor() != adcui::COLOR_NULL)
		mMarginColor = MapColorPair(aRow.marginColor());

	const QRectF rc0(0, 0, p->device()->width(), p->device()->height());
	//QRect rm(rc0);
	//rm.setRight(leftMargin()*charWidth());
	QRectF rc(rc0);
	//rc.setLeft(rm.right() + 1);
	//mTreeInfo.clear();

	p->save();
	//p->setBackgroundMode(Qt::OpaqueMode);

	QFontMetricsF fm(p->font());
	qreal w(charWidth());
	qreal h(lineHeight());

	//m_clrBkgnd = Qt::yellow;

	Q_ASSERT(hasModel());
	adcui::IADCTableModel *pIModel(tableModel());

	p->fillRect(rc, m_clrBkgnd.first);

	if (!hasSelection())
	{
		//draw a gray rectangle around the current line (have to do it first so underscores are visible) 
		if (1 || !inplaceEdit())
		{
			if (mbHighlightCaret & aRow.isCurrent())
			{
				if (0 && hasFocus())
				{
					p->save();
					p->setPen(s_highCaretFg);
					QRectF rcc(rc0);
					p->drawRect(rcc.left(), rcc.top(), rcc.right(), rcc.bottom());
					p->restore();
				}
				else if (aRow.lineColor() == adcui::COLOR_NULL)
					p->fillRect(rc, s_highCaretBg);
			}
		}
	}

	if (aRow.underLineColor() != adcui::COLOR_NULL)
	{
		p->save();
		ColorFromId(p, aRow.underLineColor());
		p->drawLine(0, h - 1, rc.width(), h - 1);
		p->restore();
	}

	p->translate(QPointF(-m_nOffsetChar * w, 0));

	QRectF rsel(rc0);
	rsel.setRight(rsel.left());

	if (!pIModel->atEndIt(aRow.iter()))
	{
		qreal rc_left = rc.left();

		//prepare a vector of displayable columns
		QVector<QPair<uint, uint>> vCols;
		for (uint col(0); col < aRow.cellsNum(); col++)
		{
			const ADCCell &aCell(aRow.cell(col));
			//int colw(pIModel->columnWidth(col));
			int colw(aCell.width());
			if (colw > 0 && !aCell.isHeader())
				vCols.push_back(qMakePair(col, (uint)colw));
		}

		for (int icol(0); icol < vCols.size(); icol++)
		{
			uint col(vCols[icol].first);
			uint colw(vCols[icol].second);

			{
				rc.setRight(rc_left + colw * w);
				//QRect rc2(rc);
				int iShift(adcui::IADCTableModel::colTextShift(icol));
				if (iShift > 0)
					rc.setLeft(rc.left() + w * iShift);

//?				if (rc.right() > rc.left())
				{
					p->save();
					//p->translate(QPoint(-m_nOffsetChar * w, 0));

					//adcui::IADCTableModel *pIModel(hasModel() ? tableModel() : nullptr);

					//assert(colID < pIModel->colsNum());

					QRectF rc2(rc);//save for drawing grid
					//rc2.translate(0, -(fm.descent() + 1));

					const ADCCell &aCell(aRow.cell(col));
					//const char *pszTreeInfo(pIModel->cellTreeDataIt(col, aRow.iter()));
					//if (pszTreeInfo)
					if (aCell.isTreeCell())
					{
						QString s(aRow.cell(adcui::IBinViewModel::CLMN_TREEINFO));//QString(pszTreeInfo)
						if (aRow.isCurrent())
							mTreeInfo = s;

						QPointF pt0(rc.left(), rc.top());
						p->save();
						drawTree(p, pt0, s, nLineCount);
						p->restore();
						rc.setTopLeft(pt0);
					}

					if (!aCell.isEmpty())
						rsel.setRight(rc.left() + (aCell.length()) * w);
					drawCell(p, rc, col, aCell, aRow.isCurrent());

					//drawColumnCell(col, p, rc, aRow.cell(col), nLineCount);
					
					if (mbDrawGrid)
					{
						p->setPen(Qt::lightGray);
						p->drawLine(rc2.bottomLeft() - QPointF(w, 0), rc2.bottomRight());
						p->drawLine(rc2.topRight(), rc2.bottomRight());
					}

					/*if (0)
						if (aRow.isCurrent())
						{
							p->save();
							p->setCompositionMode(QPainter::RasterOp_SourceXorDestination);//XorROP
							rc2.setRight(rc2.right() - 1);
							p->eraseRect(rc2);
							p->restore();
						}*/
					p->restore();
				}

				rc_left += colw*w;

				//prevent overlap by next column data
				if (rc.right() > rc.left())//? || (mpIModel->colId(CLMN_TREEINFO) == i+1) )
					rc.setLeft(rc.right());
			}
		}
	}

	// draw a column risizing vertical bar
	if (m_nResizing != -1)//? && mGrabMouse)
	{
		const ADCCell &aCell(aRow.cell(m_nResizing));
		if (!aCell.isHeader())
		{
			int x(0);
			for (uint col(0); col < aRow.cellsNum(); col++)
			{
				const ADCCell &aCell2(aRow.cell(col));
				if (aCell2.isHeader() || !aCell2.isShown())
					continue;
				x += aCell2.width();
				if (col == m_nResizing)
					break;
			}
			x *= w;
			//p->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
			p->setPen(QPen(QBrush(Qt::lightGray, Qt::Dense4Pattern), 3));//, Qt::DotLine));
			//p->setPen(Qt::lightGray);
			//p->setPen(QColor(0xFF,0xFF,0xFF));
			p->drawLine(QPoint(x, rc0.top()), QPoint(x, rc0.bottom()));
		}
	}

	unsigned selFrom;
	unsigned selLen(checkSelection(aRow.iter(), selFrom));
	if (selLen > 0)
	{
		rsel.setLeft(selFrom * w);
		if (rsel.width() >= 0)
		{
			qreal selTo(selLen != -1 ? (selFrom + selLen) * w : std::numeric_limits<qreal>::max());
			rsel.setRight(qMin(rsel.right(), selTo));
			p->setBackground(QBrush(QColor(153, 201, 239)));//VS-blue-like selection (
			p->setCompositionMode(QPainter::RasterOp_SourceAndDestination);//RasterOp_NotSourceXorDestination);
			p->eraseRect(rsel);
		}
	}
	p->restore();

	m_clrBkgnd = saved1;
	mMarginColor = saved2;
	return true;
}

void ADCTableView::slotCopySelection()
{
	ADCModelData::rect_t sel;
	if (!modelData().getSelection(sel))
		return;
	QString s;
	QTextStream ts(&s);
	adcui::IADCTextModel *pIModel(textModel());
	adcui::AutoIter t(pIModel, sel.p1.y);
	do {
		adcui::DUMPOS tp(pIModel->posFromIter(t));
		if (pIModel->checkEqual(tp, sel.p2.y) > 0)
			break;
		ADCTextRow aRow(tp);
		getRowInfo(aRow);

		uint ncells(0);
		for (uint col(0); col < aRow.cellsNum(); col++)
			if (!aRow.cell(col).isEmpty())
				ncells = col + 1;
		int icol(0);
		for (uint col(0); col < ncells; col++)
		{
			const ADCCell& aCell(aRow.cell(col));
			int colw(aCell.width());
			if (colw > 0 && !aCell.isHeader())
			{
				int iShift(adcui::IADCTableModel::colTextShift(icol));
				if (iShift > 0)
					ts << " ";
				if (!aCell.isEmpty())
					ts << aCell;
				else
					ts << "\t";
				icol++;
			}
		}
		ts << "\n";
	} while (pIModel->forwardIt(t));
	QGuiApplication::clipboard()->setText(s);
}

#include <QApplication>
class ADCWaitCursor
{
public:
	ADCWaitCursor()
	{
		QApplication::setOverrideCursor(Qt::WaitCursor);
	}
	~ADCWaitCursor()
	{
		QApplication::restoreOverrideCursor();
	}
};

#define ISCSYM(c) (isalnum(c) || ((c) == '_'))

bool ADCTableView::findText(uint iCol, const QString& what, bool bcs, bool wo, bool rev, bool reset_cp)
{
	ADCWaitCursor waitCursor;
	Qt::CaseSensitivity cs(bcs ? Qt::CaseSensitive : Qt::CaseInsensitive);
	adcui::IADCTextModel* pIM(textModel());
	ADCDocPos& cp(curIt());
	adcui::AutoIter t(pIM, cp.line());
	if (reset_cp)//start from beginning/end of document?
		pIM->seekLineIt(pIM->posFromIter(t), rev ? contentsHeight() - 1 : 0);
	int iShift(adcui::IADCTableModel::colTextShift(iCol));
	int x(qMax(0, cp.x() - iShift));
	x += rev ? -1 : 1;//move it off current position
	do {
		adcui::DUMPOS tp(pIM->posFromIter(t));
		ADCTextRow aRow(tp);
		getRowInfo(aRow);
		const ADCCell& s(aRow.cell(iCol));
		if (rev)
			x = s.lastIndexOf(what, x, cs);
		else
			x = s.indexOf(what, x, cs);
		if (x >= 0)
		{
			bool bFound(true);
			if (wo)//whole word check
			{
				if (x > 0)//left bound
					if (ISCSYM(s[x - 1].cell()))
						bFound = false;
				if (bFound && x + what.length() < s.length())
					if (ISCSYM(s[x + what.length()].cell()))
						bFound = false;
			}
			if (bFound)
			{
				pIM->copyIt(cp.line(), tp);
					cp.setX(x + iShift + what.length());//set cursor to the end of selection
					startSelection();
					cp.setX(x + iShift);//set cursor at the begining of selection
					updateSelection();
					assureCursorVisible();
					//viewport()->setFocus();
					updateContents();
				updateCurPosInDoc();
				return true;
			}
		}
		x = rev ? -1 : 0;
	} while (rev ? pIM->backwardIt(t) : pIM->forwardIt(t));
	return false;
}

int ADCTableView::ColumnWidth(int colID)
{
	return tableModel()->columnWidth(colID);
}

void ADCTableView::SetColumnWidth(int colID, int w)
{
	tableModel()->setColumnWidth(colID, w);
}

void ADCTableView::mousePressEvent(QMouseEvent * e)
{
	if (mpViewport)
	{
		Q_ASSERT(hasModel());
		//adcui::IADCTableModel *pIModel(hasModel() ? tableModel() : nullptr);
		//if (!pIModel)
		//return;

		if (m_nResizing != -1)
		{
			mGrabMouse = 1;
			setFocus();

			ADCTextRow	aHeader(adcui::DUMPOS(0));
			getColumnInfo(aHeader, true);

			int x(0);
			for (uint i = 0; i < aHeader.cellsNum(); i++)
			{
				const ADCCell &aCell(aHeader.cell(i));
				if (!aCell.isShown())
					continue;
				x += aCell.width();
				if (x * charWidth() > e->x())
				{
					m_nSelCol = i;
					break;
				}
			}
			updateContents();
			return;
		}
	}

	ADCTableViewBase::mousePressEvent(e);
}

void ADCTableView::mouseMoveEvent(QMouseEvent * e)
{
	if (mpViewport)
	{
		Q_ASSERT(hasModel());
		//adcui::IADCTableModel *pIModel(hasModel() ? tableModel() : nullptr);
		//if (!pIModel)
		//{
		//ADCTableViewBase::mouseMoveEvent(e);
		//return;
		//}

		int nLine(e->y() / lineHeight());
		int nChar(e->x() / charWidth());

		ADCTextRow	aHeader(adcui::DUMPOS(0));
		getColumnInfo(aHeader, true);

		if (mGrabMouse)
		{
			if (m_nResizing != -1)
			{
				//assert(ColumnWidth(m_nResizing) > 0);

				if (aHeader.cell(m_nResizing).isHeader())//resize left margin
				{
					int w(0);
					for (int i(0); i < m_nResizing; i++)
					{
						const ADCCell &aCell(aHeader.cell(i));
						if (aCell.isHeader() && aCell.isShown())
							w += aCell.width();
					}
					SetColumnWidth(m_nResizing, qBound(1, nChar - w + 1, COL_WIDTH_MAX));
					syncViewportMargins(nLine < 0);
				}
				else//just change a column's widh 
				{
					int w(leftMargin() - m_nOffsetChar);
					for (int i(0); i < m_nResizing; i++)
					{
						const ADCCell &aCell(aHeader.cell(i));
						if (!aCell.isHeader() && aCell.isShown())
							w += aCell.width();
					}
					SetColumnWidth(m_nResizing, qBound(1, nChar - w + 1, COL_WIDTH_MAX));
				}

				updateContents();
				//ADCBinViewBase::mouseMoveEvent( e );
				return;

			}

			ADCTableViewBase::mouseMoveEvent(e);
			return;
		}
		else
		{
		//	syncViewportMargins(false);
			m_nResizing = -1;
			if (nChar < leftMargin())
			{
				int x(0);
				for (uint i(0); i < aHeader.cellsNum(); i++)
				{
					const ADCCell &aCell(aHeader.cell(i));
					if (aCell.isShown())
					{
						if (aCell.isHeader())
						{
							x += aCell.width() * charWidth();
							if (x - 3 < e->x() && e->x() < x + 3)
							{
								m_nResizing = i;
								break;
							}
						}
					}
				}
			}
			else
			{
				if (nLine < topMargin())//check if mouse is hovered over the header
				{
					int x((leftMargin() - m_nOffsetChar) * charWidth());
					for (uint i(0); i < aHeader.cellsNum(); i++)
					{
						const ADCCell &aCell(aHeader.cell(i));
						if (aCell.isShown())
						{
							if (!aCell.isHeader())
							{
								x += aCell.width() * charWidth();
								if (x - 3 < e->x() && e->x() < x + 3)
								{
									m_nResizing = i;
									break;
								}
							}
						}
					}
				//	syncViewportMargins(true);
				}
			}
		}

		if (m_nResizing != -1)
			setCursor(Qt::SizeHorCursor);
		else
			unsetCursor();
	}

	ADCTableViewBase::mouseMoveEvent(e);
}


void ADCTableView::mouseReleaseEvent(QMouseEvent *e)
{
	m_nSelCol = -1;
	m_nResizing = -1;

	//mGrabMouse = false;

	//updateContents();

	ADCTableViewBase::mouseReleaseEvent(e);
}

int ADCTableView::cellRectAtPos(const ADCTextRow0 &aRow, QPoint pt, QRect &r0)
{
	int w(charWidth());
	int h(lineHeight());

	QRect r;
	int icol(aRow.cellRectAtPos(pt, r));
	if (icol != -1)
	{
		r.translate(-m_nOffsetChar, 0);
		r.translate(leftMargin(), topMargin());
		r0 = QRect(r.x() * w, r.y() * h, r.width() * w, r.height() * h);
		//r0.translate(1, 1);
	}
	return icol;
}

void ADCTableView::ensureCellVisible(adcui::DUMPOS it, int col, QRect &r)
{
	QRect rcClient(mpViewport->contentsRect());
//	int w0(charWidth());
	int h0(lineHeight());

	QRect rcLine(rcClient);
	rcLine.setBottom(rcLine.top() + h0);

	bool bFound(false);

	adcui::IADCTableModel *pIModel(hasModel() ? tableModel() : nullptr);
	adcui::AutoIter it2(pIModel, topIt().line());
	//tableModel()->copyIt(drawIt, topIt().line());
	for (; rcLine.top() < rcClient.bottom(); rcLine.translate(0, h0))
	{
		if (pIModel->checkEqual(pIModel->posFromIter(it2), it) == 0)
		{
			bFound = true;
			break;
		}
		if (!pIModel->forwardIt(it2))
			break;
	}
	//pIModel->deleteIterator(drawIt);

	if (!bFound)
	{
		pIModel->copyIt(topIt().line(), it);
		setVScrollBar();//update vertical scroll bar
		rcLine = rcClient;
		rcLine.setBottom(rcLine.top() + h0);
	}

	ADCTextRow	aHeader(adcui::DUMPOS(0));
	getColumnInfo(aHeader, true);

	int iVisibleCol(0);
	int x(-m_nOffsetChar);
	for (uint icol(0); icol < aHeader.cellsNum(); icol++)
	{
		const ADCCell &aCell(aHeader.cell(icol));
		if (aCell.isHeader())
			continue;
		int wcol(aCell.width());
		if (wcol <= 0)
			continue;
		if (iVisibleCol++ == col)
		{
			if (x < 0)
			{
				m_nOffsetChar += x;//scroll left
				setHScrollBar();
				x = 0;
			}
			r.setLeft(x/* *w0*/);
			r.setRight(r.left() + wcol/* *w0*/);
			r.setTop((rcLine.y() - rcClient.top())/h0/* * h0*/);
			r.setBottom(r.top() + 1/*h0*/);
			return;
		}
		x += wcol;
	}
}


void ADCTableView::toggleColumn(int i)
{
	if (tableModel())
		SetColumnWidth(i, -ColumnWidth(i));
	syncViewportMargins();
	updateContents();
}

void ADCTableView::toggleColumn(int i, bool bOn)
{
	int w(abs(ColumnWidth(i)));
	if (bOn)
		SetColumnWidth(i, w);
	else
		SetColumnWidth(i, -w);
	syncViewportMargins();
	updateContents();
}

void ADCTableView::paintEvent(QPaintEvent *e)
{
	mColAutoExpandInfo.clear();
	ADCTableViewBase::paintEvent(e);
	checkAutoexpand();
}

bool ADCTableView::checkAutoexpand()
{
	if (mColAutoExpandInfo.isEmpty())
		return false;
	for (int i(0); i < mColAutoExpandInfo.size(); i++)
		SetColumnWidth(i, ColumnWidth(i) + mColAutoExpandInfo[i]);
	syncViewportMargins();
	updateContents();
	return true;
}

bool ADCTableView::eventFilter(QObject *pObject, QEvent *e)
{
	if (pObject == mpLineEdit)
	{
		if (e->type() == QEvent::FocusOut)
		{
			slotApply();
			killEditor();
			return true;
		}
		if (e->type() == QEvent::KeyPress)
		{
			QKeyEvent *e2((QKeyEvent *)e);
			if (e2->key() == Qt::Key_Tab)
			{
				slotApply();
				if (nextCell())
					return true;
			}
			else if (e2->key() == Qt::Key_Space)
			{
				slotApply();
				nextCell();
				return true;
			}
		}
	}
	// event not filtered out
	return ADCTableViewBase::eventFilter(pObject, e);
}

bool ADCTableView::nextCell()
{
	int caretY;
	if (!curIt().checkDistanceFrom(topIt(), pageHeight(), caretY))
		return false;//?

	QPoint caret(curIt().x()/* - m_nOffsetChar*/, caretY);

	QRect r;
	int icol(currentRow().cellRectAtPos(caret, r));
	if (icol > 0)
	{
		curIt().setX(r.right() + 1);
		icol = currentRow().cellRectAtPos(caret, r);
		if (icol)
		{
			slotEditCell();
			return true;
		}
	}

	return false;
}

void ADCTableView::keyPressEvent(QKeyEvent *e)
{
	if (mpLineEdit)
	{
		switch (e->key())
		{
		case Qt::Key_Return:
			slotApply();
			return;
		case Qt::Key_Escape:
			killEditor();
			return;
		default:
			break;
		}
	}
	ADCTableViewBase::keyPressEvent(e);
}

void ADCTableView::slotApply()
{
	if (!mpLineEdit)
		return;

	QString s(mpLineEdit->text());

	int caretY;
	if (!curIt().checkDistanceFrom(topIt(), pageHeight(), caretY))
		return;//?

	QPoint caret(curIt().x()/* - m_nOffsetChar*/, caretY);

	QRect r;
	int col(currentRow().cellRectAtPos(caret, r));
	if (col < 0)
		return;

	if (tableModel()->setCellDataIt(col, curIt().line(), s.toLatin1()))
	{
		killEditor();
		updateContents();
	}
}

/*bool ADCTableView::event(QEvent *e)
{
switch (e->type())
{
case QEvent::KeyPress:
{
QKeyEvent *e2((QKeyEvent *)e);
if (e2->key() == Qt::Key_Tab && mpLineEdit)
return false;
break;
}
default:
break;
}
return ADCTableViewBase::event(e);
}*/

void ADCTableView::mouseDoubleClickEvent(QMouseEvent *)
{
	slotEditCell();
}

class MyCellEdit : public QLineEdit
{
	int m_cellIndex;
public:
	MyCellEdit(QWidget *parent, int _cellIndex)
		: QLineEdit(parent),
		m_cellIndex(_cellIndex)
	{
	}
	int cellIndex() const { return m_cellIndex; }
};

ADCDocTablePos &ADCTableView::currentRow() const { return static_cast<ADCDocTablePos &>(curIt()); }

void ADCTableView::createEditor()
{
	int caretY;
	if (!curIt().checkDistanceFrom(topIt(), pageHeight(), caretY))
		return;//?

	QPoint caret(curIt().x()/* - m_nOffsetChar*/, caretY);

	QRect r;
	int col(cellRectAtPos(currentRow(), caret, r));
	if (col < 0)
		return;

	const char *cellStr(tableModel()->cellDataIt(col, curIt().line()));

	assert(!mpLineEdit);
	mpLineEdit = new MyCellEdit(viewport(), currentRow().cellIndexFromPos(caret));
	mpLineEdit->installEventFilter(this);
	mpLineEdit->setFont(font());// mFont);
	//make it fit nicely into a table's cell
	mpLineEdit->setGeometry(r.x(), r.y() - 1, r.width() + 1, r.height() + 1);
	//?	mpLineEdit->setFrameShape(QLineEdit::Box);
	//	mpLineEdit->setFrameShadow(QFrame::Sunken);
	//?	mpLineEdit->setLineWidth(1);
	//?	mpLineEdit->setMidLineWidth(0);
	if (cellStr)
		mpLineEdit->setText(cellStr);
}

void ADCTableView::placeEditor()
{
	if (!mpLineEdit)
		return;
	int caretY;
	if (!curIt().checkDistanceFrom(topIt(), pageHeight(), caretY))
		return;
	//QPoint caret(curIt().x() - m_nOffsetChar, caretY);
	QRect r;
	int col(currentRow().cellRectFromIndex(((MyCellEdit *)mpLineEdit)->cellIndex(), caretY, r));
	if (col < 0)
		return;
	r.moveLeft(m_nOffsetChar);
	r.translate(leftMargin(), topMargin());
	int w(charWidth());
	int h(lineHeight());
	QRect r2(r.x() * w, r.y() * h, r.width() * w, r.height() * h);
	r2.translate(-1, -1);
	mpLineEdit->setGeometry(r2);
}

void ADCTableView::killEditor()
{
	if (mpLineEdit)
	{
		mpLineEdit->deleteLater();
		mpLineEdit = nullptr;
		setFocus();
	}
}

void ADCTableView::slotEditCell()
{
	killEditor();
	createEditor();
	if (mpLineEdit)
	{
		mpLineEdit->show();
		mpLineEdit->setFocus();
		mpLineEdit->selectAll();
	}
}
