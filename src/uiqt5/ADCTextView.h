#pragma once

#include <QFrame>
#include <QDialog>
#include <QtGui/QPen>
#include <QtCore/QVector>
#include <QtCore/QMap>

#include "ADCCell.h"
#include "ADCLineEdit.h"
#include "colors.h"

class QScrollBar;
class QLineEdit;
class QToolButton;
class QPushButton;
class QLabel;
class QSpinBox;

typedef	QFrame	ADCTextViewBase;

class ADCModelData;
class ADCModelDataMap : public QMap<QString, ADCModelData *>
{
public:
	ADCModelDataMap(){}
	ADCModelData *take(ADCModelData *p)
	{
		for (iterator i(begin()); i != end(); i++)
		{
			if (i.value() == p)
			{
				erase(i);
				return p;
			}
		}
		return nullptr;
	}
};

class ADCModelData : public My::IUnk
{
	ADCModelDataMap	&mrOwner;
public:
	adcui::IADCTextModel *pIModel;
	ADCDocLine		*pTopPos;
	ADCDocPos		*pCurPos;//ADCDocTablePos
	ADCDocPos		*pSelBeg;
	ADCDocPos		*pSelEnd;
public:
	//ADCModelData() : pIModel(0){}
	ADCModelData(ADCModelDataMap &r, adcui::IADCTextModel *p, ADCDocLine *top = 0, ADCDocPos *cur = 0)
		: mrOwner(r),
		pIModel(p),
		pTopPos(top ? top : new ADCDocLine(p)),
		pCurPos(cur ? cur : new ADCDocPos(p)),
		pSelBeg(nullptr),
		pSelEnd(nullptr)
	{
		pIModel->AddRef();
	}
	virtual ~ADCModelData()//can be overriden
	{
		delete pTopPos;
		delete pCurPos;
		pIModel->Release();
		mrOwner.take(this);//no longer in a list
	}
	int linesUp(int count)
	{
		int total(0);
		adcui::AutoIter t(pIModel, pTopPos->iter(), true);//update position on exit
		while (count--)
		{
			if (!pIModel->backwardIt(t))
				break;
			total++;
		}
		return total;
	}
	int scrollUp(int lines)
	{
		assert(lines >= 0);
		adcui::AutoIter t(pIModel, pTopPos->iter(), true);
		while (lines-- > 0)
			--t;// pIModel->backwardIt(t);
		return lines;
	}
	int lineDown(int count)
	{
		int total(0);
		adcui::AutoIter t(pIModel, pTopPos->iter(), true);
		while (count--)
		{
			if (!pIModel->forwardIt(t))
			{
				pIModel->backwardIt(t);//make it valid
				break;
			}
			total++;
		}
		return total;
	}
	bool isOnScreen(int lines)
	{
		adcui::AutoIter t(pIModel, pTopPos->iter());
		for (int i(0); i < lines; i++)
		{
			if (pIModel->checkEqual(pCurPos->iter(), pIModel->posFromIter(t)) == 0)
				return true;//visible
			if (!pIModel->forwardIt(t))
				break;
		}
		return false;
	}
	bool hasSelection() const {
		return (pSelBeg != nullptr && pSelEnd != nullptr);
	}
	void startSelection()
	{
		killSelection();
		pSelBeg = new ADCDocPos(pIModel, pCurPos->line(), pCurPos->x());
		pSelEnd = new ADCDocPos(pIModel, pCurPos->line(), pCurPos->x());
	}
	bool updateSelection()
	{
		if (!hasSelection())
			return false;
		pSelEnd->set(*pCurPos, 0, pCurPos->x());
		return true;
	}
	bool killSelection()
	{
		if (!hasSelection())
			return false;
		delete pSelBeg;
		pSelBeg = nullptr;
		delete pSelEnd;
		pSelEnd = nullptr;
		return true;
	}
	struct pt_t
	{
		int x;
		adcui::DUMPOS y;
		pt_t() : x(0), y(0){}
		pt_t(int _x, adcui::DUMPOS _y) : x(_x), y(_y){}
	};
	struct rect_t {
		pt_t p1;
		pt_t p2;
	};
	bool getSelection(rect_t& o) const//normalized
	{
		if (!hasSelection())
			return false;
		o.p1 = pt_t(pSelBeg->x(), pSelBeg->iter());
		o.p2 = pt_t(pSelEnd->x(), pSelEnd->iter());
		int d(pIModel->checkEqual(o.p1.y, o.p2.y));//is beg>end?
		if (d == 0)//beg==end
		{
			o.p2.y = o.p1.y;
			int d(o.p2.x - o.p1.x);
			if (d < 0)
				qSwap(o.p1, o.p2);
		}
		else if (d > 0)//beg>end
			qSwap(o.p1, o.p2);
		return true;
	}
	unsigned checkSelection(adcui::DUMPOS pos, unsigned &from) const
	{
		rect_t sel;
		if (!getSelection(sel))
			return 0;
		if (sel.p1.y == sel.p2.y)//a single line
		{
			if (pIModel->checkEqual(pos, sel.p1.y) != 0)
				return 0;
			from = sel.p1.x;
			return sel.p2.x - sel.p1.x;
		}
		int d(pIModel->checkEqual(pos, sel.p1.y));
		if (d < 0)
			return 0;//above beg
		if (d == 0)
		{
			from = sel.p1.x;
			return -1;
		}
		d = pIModel->checkEqual(sel.p2.y, pos);
		if (d < 0)
			return 0;//below end
		from = 0;
		if (d > 0)
			return -1;//whole line
		return sel.p2.x;
	}
};

class ADCTextView : public ADCTextViewBase
{
	Q_OBJECT
public:
	ADCTextView(QWidget * parent, const char *name);
	virtual ~ADCTextView();
	void setShowCaret(bool b);
	int setCaret(int x, int y);//screen coords
	//const ADCDocPos &caret(){ return *mpCurIt; }
	void updateContents(bool bReset = false);
	void setFont(const QFont &);
	int curLine();

	virtual bool hasModel() const { return false; }
	virtual ADCModelData &modelData() const = 0;
	adcui::IADCTextModel *textModel() const { return modelData().pIModel; }
	ADCDocLine &topIt() const { return *modelData().pTopPos; }
	ADCDocPos &curIt() const { return *modelData().pCurPos; }
	bool hasSelection() const { return modelData().hasSelection(); }
	void startSelection() { modelData().startSelection(); }
	void updateSelection() { modelData().updateSelection(); }
	void killSelection() { modelData().killSelection(); }
	unsigned checkSelection(adcui::DUMPOS pos, unsigned &from) const { return modelData().checkSelection(pos, from); }
	static bool checkSelectionKey();

protected:
	virtual bool event(QEvent *e);
	virtual void showEvent(QShowEvent *);
	virtual bool eventFilter(QObject *, QEvent *);
	virtual void resizeEvent(QResizeEvent *);
	virtual void timerEvent(QTimerEvent *);
	virtual void wheelEvent(QWheelEvent *);
	virtual void paintEvent(QPaintEvent *);
	virtual void focusInEvent(QFocusEvent *);
	virtual void focusOutEvent(QFocusEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void mouseDoubleClickEvent(QMouseEvent *);
	virtual void keyPressEvent(QKeyEvent *e);
	virtual void contextMenuEvent(QContextMenuEvent *);

	qreal		lineHeight();
	qreal		charWidth();
	int		pageHeight();
	int		pageWidth();
	//	void	CalcLineCharDim();
	int		contentsHeight();
	int		contentsWidth();
	virtual void	OnContentsChanged();
	void zoomIn(int range = 1);
	void zoomOut(int range = 1);
public:
	int		leftMargin();
	int		topMargin();
protected:
	virtual void	createContents();
	virtual void	deleteContents();

	void	recalcScrollBars();
	void	setHScrollBar();
	void	setVScrollBar();
	bool	setIter(ADCDocPos &, int, int);

	void	resetCaret();
	virtual void updateCurPosInDoc();//whenever current position in model changed
	QWidget *viewport(){ return mpViewport; }

	virtual void DrawCaret(QPainter &, int x, int y);
	virtual bool DrawContentsLine(QPainter *p, const ADCTextRow &, int nLineCount);
	virtual void DrawLeftMargin(QPainter *, const ADCTextRow &, int){}
	virtual void DrawTopMargin(QPainter *, const QRect &, bool, const ADCTextRow &){}
	uint drawCell(QPainter *, QRectF &rc, int col, const ADCCell &, bool bRowIsCurrent);
	virtual void ColorFromId(QPainter *, adcui::Color_t, bool bNoBgnd = false, bool bHasSel = false);
	virtual void getColumnInfo(ADCTextRow &, bool bNoData) = 0;
	virtual void getRowInfo(ADCTextRow &) = 0;
	virtual void OnNewRow(ADCTextRow &){}
	virtual void OnCurrentRow(ADCTextRow &){}

	//virtual bool drawLine(QPainter *p, const QRect &rc, int drawIt, int line) = 0;

	virtual int scrollUpDown(int delta);
	virtual int scrollLeftRight(int delta);

	virtual void onNextLine();
	virtual void onPrevLine();
	virtual void onNextPage();
	virtual void onPrevPage();

	virtual void onCharRight();
	virtual void onCharLeft();
	virtual void onPageRight();
	virtual void onPageLeft();

	void OnKeyPageUp();
	void OnKeyPageDown();

	void OnKeyLeft();
	void OnKeyRight();
	void OnKeyUp();
	void OnKeyDown();
	void OnKeyHome();
	void OnKeyEnd();

	ADCLineEdit *inplaceEdit(){ return mpEdit; }

	virtual bool startInplaceEdit();
	virtual bool stopInplaceEdit();
	int assureCursorVisible();
	int centerCursorIfNotVisible();
	int showCursorAtLine(int);
	virtual void syncViewportMargins(bool = false);
	void showScrollBars(bool bShow);
	virtual void enableContextActions(bool bEnable);

protected:
	bool startInplaceEdit(adcui::IADCTextEdit *, ADCDocPos *);
	void restartCaretTimer();
	void updateEditCaret();//document coords
	void assureLineEditVisible(QPoint * = nullptr);
	int fromScreenLine(ADCDocLine &, int);//set iter at given line
	void setUpdateFlags(unsigned);

//public slots:
	//void slotRefPosChanged(int);

protected slots:
	virtual void slotHSliderValueChanged(int nPos);
	virtual void slotVSliderValueChanged(int nPos);
	virtual void slotCopySelection();
	virtual void slotCutSelection();

private slots:
	void slotLineEditDataChanged();
	void slotLineEditCurrentChanged();
	void slotLineEditSelectionChanged();

signals:
	void signalCurLineChanged(int);
	void signalRefresh();

protected:
	//ADCDocLine	*mpTopIt;
	//ADCDocPos	*mpCurIt;

	QFrame *mpViewport;
	QScrollBar *mpVertScrollBar;
	QScrollBar *mpHorzScrollBar;
	QToolButton *mpCornerWidget;
	QAction	*mpRefreshAction;

	//QFont	mFont;
	//scroll bars ranges
	int		m_nWidth;
	int		m_nHeight;

	//char dimention
	qreal		m_nLineHeight;
	qreal		m_nCharWidth;

	//screen dimention
	int		m_nPageLines;
	int		m_nPageWidth;

	QSize	mMargin;
	QSize	mMarginExtra;//extra left margin space (in chars)

	bool	mbHighlightCaret;

	bool	mbCaretVisible;
	bool	mbShowCaret;
	//QPoint	mViewOffset;

	enum { UPDATE_CONTENTS, UPDATE_CURSOR };
	unsigned	mUpdateFlags;
	int	mTimerId;
	//int	mRefPos;//reference line position on screen - for panes synchronization
	int m_caretY;

	static const QColor s_highCaretFg;
	static const QColor s_highCaretBg;

protected:
	int		mGrabMouse;
	//horizontal screen offset
	int		m_nOffsetChar;

	ADCColorPair	m_clrBkgnd;
	ADCColorPair	mMarginColor;

	//int	mTopMargin;
	//int	mLeftMargin;
	ADCLineEdit	*mpEdit;
	QMenu *mpLineEditMenu;
	QAction* mpCopyAction;
};

////////////////////////////////// ADCGotoDlg
class ADCGotoDlg : public QDialog
{
    Q_OBJECT
public:
    ADCGotoDlg(ADCTextView *, int nMaxLine = 1);
    void setMaxLines(const int nMaxLine);
	int lineNumber() const;

signals:
    void lineNbrChanged(int);

public slots:
    void slotOk();

private:
    QPushButton *mpOkBtn;
    QPushButton *mpCancelBtn;
    QLabel *mpLabel;
    QSpinBox *mpSpinBox;
};

#define TEST_COLORS 0






