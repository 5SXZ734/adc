#pragma once


#include "ADCTextView.h"

typedef ADCTextView	ADCTableViewBase;

/*class ADCTableRow : 
	public ADCTextRow
{
public:
	ADCTableRow(){}
protected:
	virtual void addCell(unsigned iCol, const char *str)
	{
		ADCTextRow::addCell(iCol, str);
	}

	virtual void addColor(unsigned iCol, adcui::Color_t eColor)
	{
		ADCTextRow::addColor(iCol, eColor);
	}
};*/

class ADCTableView : public ADCTableViewBase
{
	Q_OBJECT
public:
	ADCTableView(QWidget * parent, const char *name);
	void setDrawGrid(bool b){ mbDrawGrid = b; }

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual bool eventFilter(QObject *, QEvent *);
	virtual void keyPressEvent(QKeyEvent *);
	virtual void mouseDoubleClickEvent(QMouseEvent *);
	//virtual bool event(QEvent *);

	adcui::IADCTableModel *tableModel() const { return static_cast<adcui::IADCTableModel *>(modelData().pIModel); }

	virtual void DrawCaret(QPainter &, int x, int y);
	virtual bool DrawContentsLine(QPainter *, const ADCTextRow &, int mscr_line);
	virtual void DrawLeftMargin(QPainter *, const ADCTextRow &, int nLineCount);
	virtual void DrawTopMargin(QPainter *, const QRect &, bool, const ADCTextRow &);
	virtual int DrawMarginCell(QPainter &, const QRect &rc2, const ADCCell &);
	virtual bool checkAutoexpand();
	virtual void getColumnInfo(ADCTextRow &, bool bNoData);
	virtual void getRowInfo(ADCTextRow &);
	virtual void OnNewRow(ADCTextRow &);
	virtual void OnCurrentRow(ADCTextRow &);

	//virtual bool drawLine(QPainter * p, const QRect &rc, int drawIt, int mscr_line);
	//void drawColumnCell(int col_id, QPainter *p, QRect &rc0, const ADCCell &, int mscr_line);
	void drawTree(QPainter *p, QPointF &pt0, const QString &levelStr, int mscr_line);
	void drawTreeCode(QPainter *p, qreal x, qreal y, uint code, QPen *penShadow);

	//int colAtPos(const ADCTextRow &aHeader, int &);
	void ensureCellVisible(adcui::DUMPOS, int col, QRect &r);
	void toggleColumn(int);
	void toggleColumn(int, bool);
	virtual void syncViewportMargins(bool = false);

	virtual void createEditor();
	virtual void killEditor();
	int columnResizing() const { return m_nResizing; }
	void placeEditor();
	//ADCTextRow &currentRow(){ return mCurrentRow; }
	ADCDocTablePos &currentRow() const;
	bool findText(uint column, const QString& what, bool cs, bool wo, bool rev, bool reset_cp);

private:
	int ColumnWidth(int colID);
	void SetColumnWidth(int colID, int w);
	bool nextCell();
	//int cellRectFromIndex(const ADCTextRow &, int col, int row, QRect &);
	//int cellIndexFromPos(const ADCTextRow &, QPoint);
	int cellRectAtPos(const ADCTextRow0 &, QPoint, QRect &);

protected slots:
	void slotEditCell();
	void slotApply();
	virtual void slotCopySelection();

protected:
	//bool	mbShowHeader;
	int		m_nResizing;
	int		m_nSelCol;
	bool	mbDrawGrid;
	QLineEdit	*mpLineEdit;
	QVector<int>	mColAutoExpandInfo;
	QVector<QPixmap>	mPixmaps;
	QPixmap	mTreePixmaps[2];
	QString	mTreeInfo;//temp
	//ADCTextRow mCurrentRow;
};
