#include <math.h>
#ifdef WIN32
#include <windows.h>//for debugging
#endif
#include <assert.h>
#include <QtCore/QEvent>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QCursor>
#include <QtGui/QWheelEvent>
#include <QtGui/QPainterPath>
#include <QScrollBar>
#include <QToolButton>
#include <QGridLayout>
#include <QLineEdit>
#include <QAction>
#include <QMenu>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QApplication>
#include <QStyleOptionGraphicsItem>

#include "ADCTextView.h"
#include "colors.h"

#define BLINKING_RATE	250
#define NO_CARET_BLINK	0

const QColor ADCTextView::s_highCaretFg = QColor(224, 224, 224);
const QColor ADCTextView::s_highCaretBg = QColor(230, 230, 230);
//const QColor ADCTextView::s_highCaretBg = QColor(192, 245, 250);

#if(TEST_COLORS)
static int gTestColor(0);//Qt::GlobalColor
static int gTestCompMode(0);//QPainter::CompositionMode
void test_colors()
{
#if(0)
	if (++gTestCompMode > QPainter::RasterOp_SourceAndNotDestination)
	{
		gTestCompMode = 0;
		if (++gTestColor > Qt::transparent)
			gTestColor = 0;
	}
#else
	static const int a[][2] = { { 1,15 },{ 4,19 },{ 7,12 },{ 7,14 },{ 7,21 },{ 9,15 },{ 14,19 },{ 16,19 },{ 18,19 } };
	static int i(-1);
	if (++i >= sizeof(a) / sizeof(int[2]))
		i = 0;
	gTestColor = a[i][0];
	gTestCompMode = a[i][1];
#endif
	fprintf(stdout, "color=%d, compmode=%d\n", gTestColor, gTestCompMode);
	fflush(stdout);
}
#endif

ADCTextView::ADCTextView(QWidget *parent, const char *name)
: ADCTextViewBase(parent),//, Qt::WNoAutoErase),
mpViewport(nullptr),
mpHorzScrollBar(nullptr),
mpVertScrollBar(nullptr),
mpCornerWidget(nullptr),
mpRefreshAction(nullptr),
m_nLineHeight(0),
m_nCharWidth(0),
m_nPageLines(-1),
m_nPageWidth(-1),
m_nWidth(-1),
m_nHeight(-1),
m_nOffsetChar(0),
//mMargin(0, 0),
mMarginExtra(0,0),
mbHighlightCaret(false),
//m_ptCaret(0, 0),
mbCaretVisible(false),
mbShowCaret(true),
//mpTopIt(nullptr),
//mpCurIt(nullptr),
mGrabMouse(0),
m_clrBkgnd(Qt::white, Qt::black),
mMarginColor(QColor(242, 242, 242), Qt::darkGray),
mUpdateFlags(1 << UPDATE_CONTENTS),
mTimerId(0),
//mRefPos(0),
m_caretY(-1),
mpEdit(nullptr),
//mTopMargin(0),
//mLeftMargin(0)
mpLineEditMenu(nullptr)
{
	setObjectName(name);

	setFocusPolicy(Qt::StrongFocus);//WheelFocus);ClickFocus
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


	//mFont = QFont("Courier New", 8);
	//mpPainter = new QPainter;
	//mpLineCache = new QPixmap;

	mpCopyAction = new QAction(QIcon(":copy_24.png"), tr("Copy"), this);
	mpCopyAction->setShortcut(tr("Ctrl+C"));
	mpCopyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(mpCopyAction, SIGNAL(triggered()), SLOT(slotCopySelection()));

	addAction(mpCopyAction);
	//createContents();

	//restartCaretTimer();// timer for blinking text caret
}

ADCTextView::~ADCTextView()
{
}

void ADCTextView::deleteContents()
{
	delete mpViewport;
	mpViewport = nullptr;
	delete mpHorzScrollBar;
	mpHorzScrollBar = nullptr;
	delete mpVertScrollBar;
	mpVertScrollBar = nullptr;
	delete mpCornerWidget;
	mpCornerWidget = nullptr;
	delete mpRefreshAction;
	mpRefreshAction = nullptr;

	delete layout();
}

void ADCTextView::createContents()
{
	setBackgroundRole(QPalette::NoRole);
	setAutoFillBackground(false);

	Q_ASSERT(!mpViewport);
	mpViewport = new QFrame(this);
	setFrameStyle(QFrame::Panel | QFrame::Sunken);
	mpViewport->setBackgroundRole(QPalette::NoRole);// Base);
	mpViewport->setAttribute(Qt::WA_OpaquePaintEvent, true);
	mpViewport->setAutoFillBackground(false);
	mpViewport->installEventFilter(this);
	mpViewport->setFocusProxy(this);
	mpViewport->setMouseTracking(true);

	Q_ASSERT(!mpHorzScrollBar);
	mpHorzScrollBar = new QScrollBar(Qt::Horizontal, this);
	mpHorzScrollBar->setCursor(Qt::ArrowCursor);
	mpHorzScrollBar->setMinimum(0);
	mpHorzScrollBar->setMaximum(80);
	mpHorzScrollBar->setSingleStep(1);//, 2);
	mpHorzScrollBar->setTracking(true);
	connect(mpHorzScrollBar, SIGNAL(valueChanged(int)), SLOT(slotHSliderValueChanged(int)));

	Q_ASSERT(!mpVertScrollBar);
	mpVertScrollBar = new QScrollBar(Qt::Vertical, this);
	mpVertScrollBar->setCursor(Qt::ArrowCursor);
	mpVertScrollBar->setMinimum(0);
	mpVertScrollBar->setMaximum(300);
	mpVertScrollBar->setSingleStep(1);
	mpVertScrollBar->setTracking(true);
	connect(mpVertScrollBar, SIGNAL(valueChanged(int)), SLOT(slotVSliderValueChanged(int)));

	Q_ASSERT(!mpRefreshAction);
	mpRefreshAction = new QAction(QIcon(":refresh.png"), tr("Refresh View"), this);
	connect(mpRefreshAction, SIGNAL(triggered()), SIGNAL(signalRefresh()));

	Q_ASSERT(!mpCornerWidget);
	mpCornerWidget = new QToolButton(this);
	mpCornerWidget->setBackgroundRole(QPalette::Window);
	mpCornerWidget->setAutoFillBackground(true);
	mpCornerWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	mpCornerWidget->setDefaultAction(mpRefreshAction);
	mpCornerWidget->setAutoRaise(true);

	Q_ASSERT(!layout());

	QGridLayout* pGrid = new QGridLayout;
	pGrid->setSpacing(0);
	pGrid->setContentsMargins(0, 0, 0, 0);

	//setLineWidth(0);
	//setMidLineWidth(1);
	//pGrid->setMargin(0);
	//pGrid->setColumnMinimumWidth(1, 1);
	//pGrid->setRowMinimumHeight(1, 1);
	//pGrid->setColumnStretch(1, 0);
	//pGrid->setRowStretch(1, 0);
	//pGrid->setHorizontalSpacing(0);
	//pGrid->setVerticalSpacing(0);

	pGrid->addWidget(mpViewport, 0, 0);
	pGrid->addWidget(mpHorzScrollBar, 1, 0);
	pGrid->addWidget(mpVertScrollBar, 0, 1);
	pGrid->addWidget(mpCornerWidget, 1, 1);

	setLayout(pGrid);

	qApp->processEvents();//required because of Polish event, being posted, resets geometry
}

void ADCTextView::showScrollBars(bool bShow)
{
	mpHorzScrollBar->setVisible(bShow);
	mpVertScrollBar->setVisible(bShow);
	mpCornerWidget->setVisible(bShow);
}

void ADCTextView::enableContextActions(bool bEnable)
{
	if (bEnable)
		addAction(mpCopyAction);
	else
		removeAction(mpCopyAction);
}

bool ADCTextView::startInplaceEdit(adcui::IADCTextEdit *pI, ADCDocPos *pt)
{
	if (pI && !mpEdit)
	{
		mpEdit = new ADCLineEdit(this, pI, pt);
		connect(mpEdit, SIGNAL(signalCurrentChanged()), SLOT(slotLineEditCurrentChanged()));
		connect(mpEdit, SIGNAL(signalSelectionChanged()), SLOT(slotLineEditSelectionChanged()));
		connect(mpEdit, SIGNAL(signalDataChanged()), SLOT(slotLineEditDataChanged()));
		updateEditCaret();
		//updateContents();
		return true;
	}
	return false;
}

bool ADCTextView::startInplaceEdit()
{
	adcui::IADCTextModel *pIModel(hasModel() ? textModel() : nullptr);
	adcui::IADCTextEdit *pIEdit(pIModel->startEditIt(curIt().line(), curIt().x()));
	if (!pIEdit)
		return false;

	ADCDocPos *pt(new ADCDocPos(pIModel, curIt().line(), pIEdit->startPos()));
	if (!startInplaceEdit(pIEdit, pt))
	{
		pIEdit->Release();
		delete pt;
		return false;
	}
	return true;
}

bool ADCTextView::stopInplaceEdit()
{
	if (1 && mpEdit)
	{
		if (mpLineEditMenu)
			return false;
		delete mpEdit;
		mpEdit = nullptr;
		updateContents();
		return true;
	}
	return false;
}

bool ADCTextView::eventFilter(QObject *o, QEvent *e)
{
	if (o == mpViewport)
	{
		switch (e->type())
		{
		case QEvent::Resize:
		case QEvent::Paint:
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
		case QEvent::MouseButtonDblClick:
		case QEvent::TouchBegin:
		case QEvent::TouchUpdate:
		case QEvent::TouchEnd:
		case QEvent::MouseMove:
		case QEvent::ContextMenu:
		case QEvent::Wheel:
			return ADCTextViewBase::event(e);
		default:// let the viewport widget handle the event
			break;
		}
	}
	return false;
}

bool ADCTextView::event(QEvent *e)
{
	/*static int z(0);
	QString qs(QString::asprintf("[%d] event(%d)\n", ++z, (int)e->type()));
	OutputDebugString((LPCWSTR)qs.utf16());*/
#if(0)
	//if (e->type() != QEvent::Paint)
		return ADCTextViewBase::event(e);
	//ADCTextViewBase::paintEvent((QPaintEvent*)e);
	//return true;
#else
	switch (e->type()) {
	case QEvent::Paint:
		ADCTextViewBase::paintEvent((QPaintEvent*)e);
		break;
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonDblClick:
	case QEvent::MouseMove:
	case QEvent::Wheel:
	//case QEvent::Drop:
	//case QEvent::DragEnter:
	//case QEvent::DragMove:
	//case QEvent::DragLeave:
		return false;
	default:
		return ADCTextViewBase::event(e);
	}
	return true;
#endif
}

void ADCTextView::showEvent(QShowEvent *e)
{
	//OutputDebugString(L"showEvent()\n");
	ADCTextViewBase::showEvent(e);
}

qreal ADCTextView::lineHeight()
{
	if (m_nLineHeight == 0)
	{
		QFontMetricsF fm(font());
		m_nLineHeight = fm.lineSpacing();
	}
	return m_nLineHeight;
}

qreal ADCTextView::charWidth()
{
	if (m_nCharWidth == 0)
	{
		QFontMetricsF fm(font());
		m_nCharWidth = fm.horizontalAdvance('W');
	}
	return m_nCharWidth;
}

int ADCTextView::pageHeight()
{
	if (m_nPageLines == -1)
	{
		m_nPageLines = height() / lineHeight() - 1;
		m_nPageLines -= topMargin();
		if (m_nPageLines <= 0)
			m_nPageLines = 1;

//		mpVertScrollBar->setSteps(1, m_nPageLines);
	}
	return m_nPageLines;
}

int ADCTextView::pageWidth()
{
	if (m_nPageWidth == -1)
	{
		m_nPageWidth = width() / charWidth();
		m_nPageWidth -= leftMargin();
		if (m_nPageWidth == 0)
			m_nPageWidth = 1;

//		mpHorzScrollBar->setSteps(1, m_nPageWidth);
	}
	return m_nPageWidth;
}

void ADCTextView::recalcScrollBars()
{
	if (mpHorzScrollBar)
	{
		mpHorzScrollBar->blockSignals(true);
		mpHorzScrollBar->setMaximum(qMax(0, contentsWidth() - 1));
		mpHorzScrollBar->setPageStep(pageWidth());
		mpHorzScrollBar->blockSignals(false);
	}

	if (mpVertScrollBar)
	{
		mpVertScrollBar->blockSignals(true);
		mpVertScrollBar->setMaximum(qMax(0, contentsHeight() - 1));
		mpVertScrollBar->setPageStep(pageHeight());
		mpVertScrollBar->blockSignals(false);
	}
}

int ADCTextView::contentsWidth()
{
	if (hasModel())
	{
		if (m_nWidth == -1)
			m_nWidth = textModel()->charsNum();// GetContentsWidth();// 256;
		return m_nWidth;
	}
	return 0;
}

int ADCTextView::contentsHeight()
{
	if (hasModel())
	{
		if (m_nHeight == -1)
			m_nHeight = textModel()->linesNum();// GetContentsHeight();// mpIModel->linesTotal();
		return m_nHeight;
	}
	return 0;
}

void ADCTextView::resizeEvent(QResizeEvent * e)
{
	//m_nPageWidth = -1;
	//m_nHeight = -1;

	m_nPageLines = -1;
	m_nPageWidth = -1;
	//pageHeight();
	//pageWidth();
	recalcScrollBars();

	ADCTextViewBase::resizeEvent(e);
}

void ADCTextView::restartCaretTimer()
{
#if(!NO_CARET_BLINK)
	if (mTimerId != 0)
		killTimer(mTimerId);
	if (mpViewport)
		mTimerId = startTimer(BLINKING_RATE);
#endif
}

void ADCTextView::focusInEvent(QFocusEvent *e)
{
#if(0)
	static int z = 0;
	OutputDebugString(QString("focusInEvent(%1)\n").arg(z++).toLatin1().constData());
#endif
	mUpdateFlags = 0;
	restartCaretTimer();
	ADCTextViewBase::focusInEvent(e);
	//updateCurPosInDoc();
}

void ADCTextView::focusOutEvent(QFocusEvent *e)
{
	//OutputDebugString(L"focusOutEvent()\n");
	stopInplaceEdit();
	ADCTextViewBase::focusOutEvent(e);
	mbCaretVisible = false;
	if (mTimerId != 0)
	{
		killTimer(mTimerId);
		mTimerId = 0;
	}
	updateContents(true);//reset update flags
}

bool ADCTextView::setIter(ADCDocPos &rIter, int x, int y)
{
	int w(charWidth());
	int h(lineHeight());

	QPoint p(x / w - leftMargin(), y / h - topMargin());

/*	if (p.x() < 0)
		p.setX(0);
	if (p.y() < 0)
		p.setY(0);*/

	if (p.x() < 0)//over left margin?
		//p.setX(rIter.x());
		//return false;
		p.setX(0);

	p.setX(p.x() + m_nOffsetChar);

	return rIter.set(topIt(), p.y(), p.x());// pti;
}

void ADCTextView::mousePressEvent(QMouseEvent *e)
{
	//fprintf(stdout, "mousePressEvent()\n");fflush(0);
//?	if (!mpCurIt)
	//	return;
//?	setFocus();
	if (mpViewport)
	{
		if (mpEdit && mpLineEditMenu)
		{
			//updateCurPosInDoc();
			return;
		}
		qreal w(charWidth());
		qreal h(lineHeight());

#if QT_VERSION_MAJOR >= 6
		QPointF pos = e->position();   // floating-point
		QPoint p0(
			int(pos.x() / w - leftMargin()),
			int(pos.y() / h - topMargin())
		);
#else
		QPoint p0(
			e->x() / w - leftMargin(),
			e->y() / h - topMargin()
		);
#endif

		QPoint p(p0);
		if (p.x() < 0)//over left margin?
			p.setX(0);
		p.setX(p.x() + m_nOffsetChar);
		if (!setCaret(p.x(), p.y()))
			return;

		if (e->button() == Qt::RightButton)
			return;

		mGrabMouse = 1;
		if (mpEdit && mpEdit->OnClick(curIt().line(), curIt().x(), e->button() == Qt::RightButton))// pti.x() + m_nOffsetChar))
			mGrabMouse = 2;
		else
		{
			stopInplaceEdit();
			//updateCurPosInDoc();
		}

		if (p0.x() >= 0 && p0.y() >= 0)//click inside client area (not on margin)
		{
			if (checkSelectionKey())
				updateSelection();
			else
				killSelection();
		}
	}

	//resetCaret();
	//updateContents();
	//setFocus();
}

void ADCTextView::mouseReleaseEvent(QMouseEvent *)
{
	//fprintf(stdout, "mouseReleaseEvent()\n"); fflush(0);
	if (mpEdit && mGrabMouse == 2)
	{
		QPoint pt;
		assureLineEditVisible(&pt);
		setCaret(pt.x() + mpEdit->curPos(), pt.y());
	}
	mGrabMouse = 0;
	updateContents();
}

void ADCTextView::mouseDoubleClickEvent(QMouseEvent *e)
{
#if QT_VERSION_MAJOR >= 6
	QPoint a = e->position().toPoint();
#else
	QPoint a(e->x(), e->y());
#endif

	QPoint b(leftMargin()*charWidth(), topMargin()*lineHeight());
	QPoint p(a - b);

	if (p.x() < 0)
		p.setX(0);
	if (p.y() < 0)
		p.setY(0);

	QPoint pti(p.x() / charWidth(), p.y() / lineHeight());
	curIt().set(topIt(), pti.y(), pti.x() + m_nOffsetChar);// pti;
	//fprintf(stdout, "mouseDoubleClickEvent(%d,%d)\n", pti.x(), pti.y()); fflush(stdout);

	updateCurPosInDoc();

	if (mpEdit && mpEdit->OnDoubleClick(curIt().line(), pti.x() + m_nOffsetChar))
	{
	}
	else
	{
		stopInplaceEdit();
		updateCurPosInDoc();
	}

	resetCaret();
	//updateContents();
	setFocus();
}

bool ADCTextView::checkSelectionKey()
{
	return (QApplication::keyboardModifiers() & Qt::ShiftModifier) != 0;
}

void ADCTextView::mouseMoveEvent(QMouseEvent *e)
{
	//fprintf(stdout, "mouseMoveEvent()\n");
#if QT_VERSION_MAJOR >= 6
	QPointF pos = e->position();  // floating-point, Qt 6+
	QPoint p0(
		int(pos.x() / charWidth() - leftMargin()),
		int(pos.y() / lineHeight() - topMargin())
	);
#else
	QPoint p0(
		e->x() / charWidth() - leftMargin(),
		e->y() / lineHeight() - topMargin()
	);
#endif


	if (mGrabMouse == 2)
	{
		if (mpEdit)
			if (!mpEdit->OnMove(m_nOffsetChar + p0.x()))
			{
			}
	}

	if (mGrabMouse)
	{
#if(0)
		if (e->button() == Qt::NoButton)
		{
			mGrabMouse = 0;//prevent wierd behaviour on bp stop in core thread
			return;
		}
#endif

		if (mGrabMouse == 1)
			if (!hasSelection())
				if (checkSelectionKey())
					startSelection();

		if (p0.y() < 0)
		{
			scrollUpDown(-1);
			setCaret(m_nOffsetChar + p0.x(), 0);
		}
		else if (p0.y() >= pageHeight() - 1)
		{
			scrollUpDown(+1);
			setCaret(m_nOffsetChar + p0.x(), pageHeight() - 1);
		}
		else if (p0.x() < 0)
		{
			scrollLeftRight(-1);
			setCaret(m_nOffsetChar, p0.y());
		}
		else if (p0.x() > pageWidth() - 1)
		{
			scrollLeftRight(+1);
			setCaret(m_nOffsetChar + (pageWidth() - 1), p0.y());
		}
		else
		{
			setCaret(m_nOffsetChar + p0.x(), p0.y());
		}

		if (mGrabMouse == 1)
			if (p0.x() >= 0 && p0.y() >= 0)//not on margin
				updateSelection();

		//updateCurPosInDoc();
		updateContents();
	}

	//ADCTextViewBase::mouseMoveEvent(e);
}

int ADCTextView::leftMargin()
{
	int w(0);
	if (mMargin.width() >= 0)
		w += mMargin.width();
	w += mMarginExtra.width();
	return w;
}

int ADCTextView::topMargin()
{
	int h(0);
	if (mMargin.height() >= 0)
		h += mMargin.height();
	h += mMarginExtra.height();
	return h;
}

void ADCTextView::updateEditCaret()
{
	if (mpEdit)
	{
		int x(mpEdit->startPosX() + mpEdit->curPos());
		//setCaret(x - m_nOffsetChar, -1);
		if (curIt().setX(x))
			resetCaret();

/*		while ((m_ptCaret.x() + m_nOffsetChar) > x)
			OnKeyLeft();
		while ((m_ptCaret.x() + m_nOffsetChar) < x)
			OnKeyRight();*/
	}
	updateContents();
}

int ADCTextView::setCaret(int x, int y)
{
	/*QPoint old(m_ptCaret);
	if (x >= 0)
		m_ptCaret.setX(x);
	if (y >= 0)
		m_ptCaret.setY(y);
	if (m_ptCaret != old)*/
	if (curIt().set(topIt(), y, x))
	{
		updateCurPosInDoc();
		resetCaret();
		//updateContents();
		return 1;
	}
	return 0;
}

void ADCTextView::setHScrollBar()
{
	if (!mpHorzScrollBar)
		return;
	int nPos = mpHorzScrollBar->value();
	int pos = (int)(((double)m_nOffsetChar / 80) * 80);
	if (pos != nPos)
	{
		mpHorzScrollBar->blockSignals(true);
		mpHorzScrollBar->setValue(pos);
		mpHorzScrollBar->blockSignals(false);
	}

	updateContents();
}

//updates v-scrollbar's position in responce to the change in model
void ADCTextView::setVScrollBar()
{
	if (!mpVertScrollBar || !hasModel())
		return;
	int nPos = mpVertScrollBar->value();
	int pos(textModel()->lineFromIt(topIt().line()));

	if (pos != nPos)
	{
		mpVertScrollBar->blockSignals(true);
		mpVertScrollBar->setValue(pos);
		mpVertScrollBar->blockSignals(false);
		updateContents();
	}
}

void ADCTextView::assureLineEditVisible(QPoint *ptAt)
{
	assert(mpEdit);

	if (mpEdit->origin().base() < topIt())
	{
		topIt() = mpEdit->origin().base();
		if (ptAt)
			*ptAt = QPoint(mpEdit->origin().x(), 0);
		updateContents();
		return;
	}

	ADCDocLine it(topIt());
	for (int y(0); y < pageHeight(); y++, ++it)
	{
		if (it == mpEdit->origin().base())
		{
			if (ptAt)
				*ptAt = QPoint(mpEdit->origin().x(), y);
			updateContents();
			return;
		}
	}

	topIt() = mpEdit->origin().base();
	for (int j(pageHeight()); j > 0; j--)
		--(topIt());
	if (ptAt)
		*ptAt = QPoint(mpEdit->origin().x(), pageHeight());
	updateContents();
}

void ADCTextView::OnKeyLeft()
{
	if (!(curIt().x() > m_nOffsetChar))
		scrollLeftRight(-1);
	//setCaret(curIt().x() - 1, m_caretY);
	if (curIt().shiftX(-1))
	{
		resetCaret();
		updateCurPosInDoc();
		updateContents();
	}
}

void ADCTextView::OnKeyRight()
{
	if (!(curIt().x() < m_nOffsetChar + pageWidth()))
		scrollLeftRight(+1);
	//setCaret(curIt().x() + 1, m_caretY);
	if (curIt().shiftX(+1))
	{
		resetCaret();
		updateCurPosInDoc();
		updateContents();
	}
}

void ADCTextView::OnKeyHome()
{
	if (curIt().shiftHome())
	{
		//curIt().setX(0);
		m_nOffsetChar = 0;
		resetCaret();
		updateCurPosInDoc();
	}
}

void ADCTextView::OnKeyEnd()
{
	if (curIt().shiftEnd())
	{
		//?m_ptCaret.setX(pageWidth() - 1);
		resetCaret();
		updateCurPosInDoc();
	}
}

int ADCTextView::assureCursorVisible()
{
	int y;
	int maxY(pageHeight());
	if (!curIt().checkDistanceFrom(topIt(), maxY, y))
	{
		int dy(0);
		if (!(curIt() < topIt()))
			dy = -(pageHeight() - 1);
		topIt() = curIt();
		if (dy)
			y = topIt().shiftY(dy);
		setVScrollBar();
	}
	return y;
}

int ADCTextView::centerCursorIfNotVisible()
{
	//ADCDocLine t(topIt());
	int lines(pageHeight());
	if (lines > 0)
	{
		Q_ASSERT(hasModel());
		if (modelData().isOnScreen(lines))
			return 1;//visible
	}
	int ret(showCursorAtLine(lines / 2));
	//setVScrollBar();
	return ret;
}

int ADCTextView::showCursorAtLine(int linesFromTop)
{
	topIt().copyFrom(&curIt());

	if (linesFromTop >= 0)
	{
		Q_ASSERT(hasModel());
		linesFromTop = modelData().scrollUp(linesFromTop);
	}
	setVScrollBar();
	return linesFromTop;
}

void ADCTextView::OnKeyUp()
{
	int y(assureCursorVisible());
	if (!(y > 0))
		onPrevLine();
	curIt().shiftY(-1);
	resetCaret();
	updateCurPosInDoc();
	updateContents();
}

void ADCTextView::OnKeyDown()
{
	int y(assureCursorVisible());
	if (!(y < pageHeight() - 1))
		onNextLine();
	curIt().shiftY(+1);
	resetCaret();
	updateCurPosInDoc();
	updateContents();
}

void ADCTextView::OnKeyPageUp()
{
	assureCursorVisible();
	onPrevPage();
	curIt().shiftY(-pageHeight());
	updateCurPosInDoc();
	updateContents();
}

void ADCTextView::OnKeyPageDown()
{
	assureCursorVisible();
	onNextPage();
	curIt().shiftY(+pageHeight());
	updateCurPosInDoc();
	updateContents();
}

void ADCTextView::keyPressEvent(QKeyEvent *e)
{
	ADCLineEdit *pIEdit(inplaceEdit());
	if (pIEdit)
	{
		if (e->modifiers() & Qt::ControlModifier)
		{
			ADCTextViewBase::keyPressEvent(e);
			return;
		}

		bool bShiftModifier((e->modifiers() & Qt::ShiftModifier) != 0);
		//bool bCtrlModifier((e->modifiers() & Qt::ControlModifier) != 0);
		switch (e->key())
		{
		case Qt::Key_Shift:
			return;
		case Qt::Key_Enter:
		case Qt::Key_Return:
			pIEdit->OnApply();
			//fall through ...
		case Qt::Key_Escape:
			//stopInplaceEdit();
			//return;
			break;
		case Qt::Key_Backspace:
			pIEdit->OnBackspace();
			return;
		case Qt::Key_Delete:
			pIEdit->OnDelete();
			return;
		case Qt::Key_Left:
			pIEdit->OnLeft(bShiftModifier);
			return;
		case Qt::Key_Right:
			pIEdit->OnRight(bShiftModifier);
			return;
		case Qt::Key_Home:
			pIEdit->OnHome(bShiftModifier);
			return;
		case Qt::Key_End:
			pIEdit->OnEnd(bShiftModifier);
			return;
		default:
			if (Qt::Key_A <= e->key() && e->key() <= Qt::Key_Z)
			{
				int n((e->key() - Qt::Key_A));
				if (e->modifiers() & Qt::ShiftModifier)
					pIEdit->OnChar('A' + n);
				else
					pIEdit->OnChar('a' + n);
				return;
			}
			else if (Qt::Key_0 <= e->key() && e->key() <= Qt::Key_9)
			{
				int n((e->key() - Qt::Key_0));
				pIEdit->OnChar('0' + n);
				return;
			}
			else if (e->key() == Qt::Key_Underscore)
			{
				pIEdit->OnChar(e->key());
				return;
			}
			if (e->key() == Qt::Key_Space)
				return;//prevent the abort, this key can be easily pressed by mistake
			if (e->key() == Qt::Key_Control)
				return;//shortcut actions started with CTRL
		}

		stopInplaceEdit();
	}

	switch (e->key())
	{
	case Qt::Key_PageUp: OnKeyPageUp();	break;
	case Qt::Key_PageDown: OnKeyPageDown(); break;

	case Qt::Key_Up:OnKeyUp(); break;
	case Qt::Key_Down: OnKeyDown(); break;
	case Qt::Key_Left: OnKeyLeft();	break;
	case Qt::Key_Right: OnKeyRight(); break;
	case Qt::Key_Home: OnKeyHome();	break;
	case Qt::Key_End: OnKeyEnd();	break;

	default:
		ADCTextViewBase::keyPressEvent(e);
		break;
	}
}

void ADCTextView::slotLineEditDataChanged()
{
	updateContents();
}

void ADCTextView::slotLineEditCurrentChanged()
{
	updateEditCaret();
}

void ADCTextView::slotLineEditSelectionChanged()
{
	updateContents();
}

void ADCTextView::contextMenuEvent(QContextMenuEvent *)
{
	if (mpEdit)
	{
		mpLineEditMenu = new QMenu(this);
		mpEdit->PopulateEditMenu(*mpLineEditMenu);
		//prevent a stop of inplace edit due to focus out event
		mpLineEditMenu->exec(QCursor::pos());
		delete mpLineEditMenu;
		mpLineEditMenu = nullptr;
		updateEditCaret();
		return;
	}
	if (hasSelection())
	{
		QMenu menu(new QMenu(this));
		menu.addAction(mpCopyAction);
		menu.exec(QCursor::pos());
	}
}

void ADCTextView::slotCopySelection()
{
}

void ADCTextView::slotCutSelection()
{
}

void ADCTextView::updateContents(bool bReset)
{
	if (bReset)
		setUpdateFlags(0);
	else
		setUpdateFlags(mUpdateFlags | (1 << UPDATE_CONTENTS) | (1 << UPDATE_CURSOR));
	if (mpViewport)
		mpViewport->update();
}

void ADCTextView::setFont(const QFont &f)
{
	ADCTextViewBase::setFont(f);
	m_nLineHeight = 0;
	m_nCharWidth = 0;
	m_nPageLines = -1;
	m_nPageWidth = -1;
	if (viewport())
		viewport()->ensurePolished();
	syncViewportMargins();
}

void ADCTextView::syncViewportMargins(bool)// bOverTop)
{
	QMargins margins;// (viewport()->contentsMargins());
	if (hasModel())
	{
		margins.setLeft((mMargin.width() + mMarginExtra.width())*charWidth());
		margins.setTop((mMargin.height() + mMarginExtra.height())*lineHeight());
	}
	if (viewport())
		viewport()->setContentsMargins(margins);
}

void ADCTextView::OnContentsChanged()//int w, int h)
{
	m_nWidth = -1;
	m_nHeight = -1;
	m_nPageLines = -1;
	m_nPageWidth = -1;
	recalcScrollBars();
	//pageHeight();
	//pageWidth();

	//mpHorzScrollBar->setMaximum(w);
	//mpVertScrollBar->setMaximum(h);
	updateContents();
}

void ADCTextView::resetCaret()
{
	mbCaretVisible = true;
	restartCaretTimer();
	setUpdateFlags(mUpdateFlags | (1 << UPDATE_CURSOR));
	updateContents();
}

/*void ADCTextView::updateCaret(QPainter * pPainter)
{
#ifndef ZZZ
	if (!hasFocus())
		return;

	if (m_ptCaret.x() < 0 || m_ptCaret.x() > pageWidth())
		m_ptCaret.setX(0);
	if (m_ptCaret.y() < 0 || m_ptCaret.y() > pageHeight())
		m_ptCaret.setY(1);

	QPoint pt;
	pt.setX(lineWidth() + (m_ptCaret.x() + mViewOffset.x()) * charWidth());
	pt.setY(lineWidth() + (m_ptCaret.y() + mViewOffset.y()) * lineHeight());
	//	SetCaretPos(pt);

	int line_height = lineHeight();

	// Draw caret Xored
//?	pPainter->setRasterOp(Qt::NotXorROP);

	QPen pen(Qt::black, 2);
	pPainter->setPen(pen);
//?	pPainter->setBackgroundColor(Qt::white);
	pPainter->drawLine(pt.x(), pt.y(), pt.x(), pt.y() + line_height);

	//Restore painter
//?	pPainter->setRasterOp(Qt::NotXorROP);
#endif
}*/

int ADCTextView::scrollLeftRight(int delta)
{
	int val(mpHorzScrollBar->value());
	val += delta;
	if (val < 0)
		val = 0;
	mpHorzScrollBar->setValue(val);
	m_nOffsetChar = mpHorzScrollBar->value();
	return mpHorzScrollBar->value() - val;
}

void ADCTextView::onCharRight()
{
	scrollLeftRight(+1);
	updateContents();
/*	if (m_nOffsetChar <  PAGE_WIDTH_MAX - 1)
		m_nOffsetChar += 1;
	//	updateCurPosInDoc();
	setHScrollBar();
	updateContents();*/
}

void ADCTextView::onCharLeft()
{
	scrollLeftRight(-1);
	updateContents();
	/*if (m_nOffsetChar > 0)
		m_nOffsetChar -= 1;
	setHScrollBar();
	updateContents();*/
}

void ADCTextView::onPageRight()
{
	int delta(+pageWidth());
	if (scrollLeftRight(delta) != delta)
		setCaret(pageWidth() - 1, -1);
	updateContents();
//	onCharRight();//?
}

void ADCTextView::onPageLeft()
{
	int delta(-pageWidth());
	if (scrollLeftRight(delta) != delta)
		setCaret(pageWidth() - 1, -1);
	updateContents();
//	onCharLeft();
}

int ADCTextView::scrollUpDown(int delta)
{
	int val = mpVertScrollBar->value();
	mpVertScrollBar->setValue(val + delta);
	return mpVertScrollBar->value() - val;
}

void ADCTextView::onNextLine()
{
	scrollUpDown(+1);
	//updateCurPosInDoc();
}

void ADCTextView::onPrevLine()
{
	scrollUpDown(-1);
	//updateCurPosInDoc();
}

void ADCTextView::onPrevPage()
{
	int delta = -pageHeight();
	scrollUpDown(delta);
}

void ADCTextView::onNextPage()
{
	int delta = +pageHeight();
	scrollUpDown(delta);
}

void ADCTextView::slotHSliderValueChanged(int nPos)
{
	m_nOffsetChar = nPos;
	if (m_nOffsetChar < 0)
		m_nOffsetChar = 0;
	else if (m_nOffsetChar >= contentsWidth())
		m_nOffsetChar = contentsWidth() - 1;
	//?updateCurPosInDoc();
	updateContents();
}

void ADCTextView::slotVSliderValueChanged(int nPos)
{
	adcui::IADCTextModel *pIModel(hasModel() ? textModel() : nullptr);
	if (pIModel)
	{
		if (nPos < 0)
			pIModel->seekLineIt(topIt().line(), 0);
		else if (nPos >= contentsHeight())
			pIModel->seekLineIt(topIt().line(), contentsHeight() - 1);
		else
			pIModel->seekLineIt(topIt().line(), nPos);
		//?updateCurPosInDoc();
	}
	updateContents();
}

#define WHEEL_DELTA 120

void ADCTextView::wheelEvent(QWheelEvent* e)
{
    // For Qt6: use angleDelta().y(), for Qt5: use delta()
//#if QT_VERSION_MAJOR >= 6
    const int delta = e->angleDelta().y();
//#else
  //  const int delta = e->delta();
//#endif

    if (e->modifiers() == Qt::ControlModifier)
    {
        if (delta < 0)
            zoomOut();
        else if (delta > 0)
            zoomIn();
        return;
    }

    int numSteps = delta / WHEEL_DELTA;
    numSteps *= 10;

    if (numSteps > 0) // up
    {
        while (numSteps--)
            onPrevLine();
    }
    else if (numSteps < 0)
    {
        while (numSteps++)
            onNextLine();
    }

    m_caretY = -1;
    //? emit signalCurLineChanged(curLine() + topMargin());

    // updateCurPosInDoc();
    updateContents();
    curLine();
}

void ADCTextView::zoomIn(int range)
{
	QFont f = font();
	const int newSize = f.pointSize() + range;
	if (newSize <= 0)
		return;
	f.setPointSize(newSize);
	setFont(f);
}

void ADCTextView::zoomOut(int range)
{
	zoomIn(-range);
}

void ADCTextView::setShowCaret(bool b)
{
	mbShowCaret = b;
	if (mbShowCaret)
	{
		restartCaretTimer();
	}
	else if (mTimerId != 0)
	{
		killTimer(mTimerId);
		mTimerId = 0;
	}
	updateContents();
}

void ADCTextView::timerEvent(QTimerEvent*)
{
	if (!hasFocus())
		return;

	/*mpPainter->begin(this);

	updateCaret(mpPainter);

	mpPainter->end();*/
	mbCaretVisible = !mbCaretVisible;

	//mUpdateFlags |= (1 << UPDATE_CONTENTS)|(1 << UPDATE_CURSOR);
	setUpdateFlags(mUpdateFlags | (1 << UPDATE_CURSOR));
	if (mpViewport)
		mpViewport->update();
}

void ADCTextView::ColorFromId(QPainter *p, adcui::Color_t e, bool bNoBgnd, bool bHasSel)
{
	::ColorFromId(p, e, bNoBgnd, bHasSel);
}

static void check_trim(QString &s, int len)
{
	int p, pp, ppp, f;
	p = pp = ppp = f = 0;
	if (len < 1)
		len = 1;
	while (p < (int)s.length() && len > 0)
	{
		if (s[p] == (char)0xFF)//skip 2 symbols
		{
			p += 2;
			continue;
		}
		if (f == 0)
			f = p;
		ppp = pp;
		pp = p;
		p++;
		len--;
	}

	if (p == s.length())
		return;
	if (ppp && ppp != f)
		s[ppp] = '.';
	if (pp && pp != f)
		s[pp] = '.';
	s.resize(p);
}

struct ADCPainterState
{
	QColor	colorFgnd;
	QColor	colorBgnd;
	Qt::BGMode	modeBgnd;
	bool	fontBold;
	bool	fontItalic;
	bool	fontUnderline;
	ADCPainterState(const QPainter &P)
	{
		colorFgnd = P.pen().color();
		colorBgnd = P.background().color();
		modeBgnd = P.backgroundMode();
		fontBold = P.font().bold();
		fontItalic = P.font().italic();
		fontUnderline = P.font().underline();
	}
	void recover(QPainter &P)
	{
		if (colorFgnd != P.pen().color())
			P.setPen(colorFgnd);
		if (colorBgnd != P.background().color())
			P.setBackground(QBrush(colorBgnd));
		if (modeBgnd != P.backgroundMode())
			P.setBackgroundMode(modeBgnd);
		uint u(0);
		if (fontBold != P.font().bold())
			u |= 1;
		if (fontItalic != P.font().italic())
			u |= 2;
		if (fontUnderline != P.font().underline())
			u |= 4;
		if (u != 0)
		{
			QFont f(P.font());
			f.setBold(fontBold);
			f.setItalic(fontItalic);
			f.setUnderline(fontUnderline);
			P.setFont(f);
		}
	}
};

static char nibble2hex(char c)
{
	static const char *tab("0123456789ABCDEF");
	return tab[c & 0xF];
}


static QPixmap generateWavyPixmap(qreal maxRadius, const QPen& pen)
{
	const qreal radiusBase = qMax(qreal(1), maxRadius);

	QPixmap pixmap;

	const qreal halfPeriod = qMax(qreal(2), qreal(radiusBase * 1.61803399)); // the golden ratio
	const int width = ceil(100 / (2 * halfPeriod)) * (2 * halfPeriod);
	const qreal radius = floor(radiusBase * 2) / 2.;

	QPainterPath path;

	qreal xs = 0;
	qreal ys = radius;

	while (xs < width) {
		xs += halfPeriod;
		ys = -ys;
		path.quadTo(xs - halfPeriod / 2, ys, xs, 0);
	}

	pixmap = QPixmap(width, radius * 2);
	pixmap.fill(Qt::transparent);
	{
		QPen wavePen = pen;
		wavePen.setCapStyle(Qt::SquareCap);

		// This is to protect against making the line too fat, as happens on OS X
		// due to it having a rather thick width for the regular underline.
		const qreal maxPenWidth = .8 * radius;
		if (wavePen.widthF() > maxPenWidth)
			wavePen.setWidthF(maxPenWidth);

		QPainter imgPainter(&pixmap);
		imgPainter.setPen(wavePen);
		imgPainter.setRenderHint(QPainter::Antialiasing);
		imgPainter.translate(0, radius);
		imgPainter.drawPath(path);
	}

	return pixmap;
}

static QPixmap generateWavyPixmap(const QPen &pen)
{
	qreal penWidth(pen.widthF());
	qreal underlineOffset(2);
	qreal maxHeight = 4;// fdescent - qreal(1);
	// Adapt wave to underlineOffset or pen width, whatever is larger, to make it work on all platforms
	return generateWavyPixmap(qMin(qMax(underlineOffset, penWidth), maxHeight / qreal(2.)), pen);
}

static void drawSquiggle(QPainter *painter, QPointF pos, int width, const QPixmap &wave)
{
	qreal maxHeight = 4;// fdescent - qreal(1);

	//static const QPixmap wave = generateWavyPixmap(2, pen);
	int descent = floor(maxHeight);

	//QFontMetricsF fm(painter->font());// Metrics());
	painter->save();
	painter->translate(0, pos.y() - 1);
//	painter->setBrushOrigin(painter->brushOrigin().x(), 0);
	painter->fillRect(pos.x(), 0, width, qMin(wave.height(), descent), wave);
	painter->restore();
}

uint ADCTextView::drawCell(QPainter *p, QRectF &rc, int /*col*/, const ADCCell &aCell, bool bRowIsCurrent)//, int fgnd_color)
{
#if(SELWORD)
	bool bSelWord = false;
	//if (mscr_line - 1 == m_ptCaret.y())
	if (isCurrent())
		bSelWord = true;
#endif

	qreal w(charWidth());
	//int h = lineHeight();
	//int y_base = rc.top() + h - 4;
	//p->drawTextItem(0, 0, QTextItem());

	//QFontMetrics fm(p->fontMetrics());
	QFontMetricsF fm(p->font());
//	QRect rc(rc0);
//	rc.translate(0, -(fm.descent() + 1));

	QList<ADCPainterState>	aStack;
	aStack.push_back(ADCPainterState(*p));

	int iSquiggleFrom(-1);
	adcui::Color_t eSquiggleColor(adcui::COLOR_NULL);

	uint count(0);
	for (uint i(0); i < aCell.codesNum(); i++)
	{
		const ADCCell::Code &aCode(aCell.code(i));
		if (aCode.ops[0] == ADCCell::OPCODE_DATA)
		{
#ifdef _DEBUG
//			std::string z(aCell.mid(aCode.ops[1], aCode.ops[2]).toStdString());
#endif
			QString s(aCell.mid(aCode.ops[1], aCode.ops[2]));

			if (aCell.isHeader())
				p->drawText(rc, Qt::TextSingleLine | Qt::AlignHCenter, s);
			else
			{
				if (aCell.isTrimmed())
					check_trim(s, rc.width() / w);
				p->drawText(rc.left(), rc.bottom() - (fm.descent() - 0), s);
				//??assert(fm.horizontalAdvance(s) == s.length() * w);
			}

			count += s.length();
			rc.setLeft(rc.left() + s.length() * w);

			if (iSquiggleFrom >= 0)
			{
				int iSquiggleTo(rc.left());
				//QPen old(p->pen());
				//p->save();
				qreal y(rc.bottom() - 2);
				if (eSquiggleColor == adcui::COLOR_SQUIGGLE_RED)
				{
					static QPixmap wave(generateWavyPixmap(QPen(Qt::red)));
					drawSquiggle(p, QPointF(iSquiggleFrom, y), iSquiggleTo - iSquiggleFrom, wave);
				}
				else if (eSquiggleColor == adcui::COLOR_SQUIGGLE_GREEN)
				{
					static QPixmap wave(generateWavyPixmap(QPen(Qt::darkGreen)));
					drawSquiggle(p, QPointF(iSquiggleFrom, y), iSquiggleTo - iSquiggleFrom, wave);
				}
				//p->setPen(old);
				//p->restore();
				iSquiggleFrom = -1;
			}
		}
		else if (aCode.ops[0] == ADCCell::OPCODE_COLOR)
		{
			adcui::Color_t eColor((adcui::Color_t)aCode.ops[1]);
			if (eColor != adcui::COLOR_POP)
			{
				bool bSquiggle(eColor == adcui::COLOR_SQUIGGLE_RED || eColor == adcui::COLOR_SQUIGGLE_GREEN);

				if (!bSquiggle)
					ColorFromId(p, eColor, bRowIsCurrent, hasSelection());

				aStack.push_back(ADCPainterState(*p));

				if (bSquiggle)
				{
					eSquiggleColor = eColor;
					iSquiggleFrom = rc.left();
				}
			}
			else //if (aCode.ops[1] == adcui::COLOR_POP)
			{
				//ColorFromId(p, adcui::COLOR_NULL);//restore original attributes
				//colorStack.back().recover(*p);
				aStack.pop_back();
				Q_ASSERT(!aStack.empty());
				//if (!aStack.empty())//?assert?
					aStack.back().recover(*p);
				//ColorFromId(p, colorStack.back());
			}
		}
		else if (aCode.ops[0] == ADCCell::OPCODE_TREE)
		{
			QString s(aCell.mid(aCode.ops[1], aCode.ops[2]));
#ifdef _DEBUG
//			std::string z(s.toStdString());
#endif
			for (int i(0); i < s.length(); i++)
				s[i] = nibble2hex(s[i].cell() - '0');//make it printable

			if (aCell.isHeader())
				p->drawText(rc, Qt::TextSingleLine | Qt::AlignHCenter, s);
			else
			{
				if (aCell.isTrimmed())
					check_trim(s, rc.width() / w);
				p->drawText(rc.left(), rc.bottom() - (fm.descent() + 1), s);
			}

			count += s.length();
			rc.setLeft(rc.left() + s.length() * w);
		}
	}

	return count;

	/*
#if(SELWORD)
			QPair<int, int> pt;
			bool bInvert(false);
			if (bSelWord)
			{
				int pos(curIt().x() - rc.left() / w);
				if (checkSelWord(s.toUtf8(), pos, pt))
				{
					m_sSelWord = s.mid(pt.first, pt.second);
					bInvert = true;
				}
			}
#endif
			p->drawText(rc.left(), y_base, s);
#if(SELWORD)
			if (bInvert)
			{
				QRect rcs = rc;
				rcs.setLeft(rcs.left() + pt.first * w);
				rcs.setRight(rcs.left() + pt.second * w);
				p->save();
				p->setCompositionMode(QPainter::RasterOp_SourceXorDestination);//XorROP
				p->eraseRect(rcs);
				p->restore();
				m_rcSel = rcs;
				m_rcSel.translate(-m_nOffsetChar*w, mscr_line*h);
			}
#endif
	}*/
}

bool ADCTextView::DrawContentsLine(QPainter *p, const ADCTextRow &aRow, int)
{
	ADCColorPair saved1(m_clrBkgnd);
	ADCColorPair saved2(mMarginColor);
	if (aRow.lineColor() != adcui::COLOR_NULL)
		m_clrBkgnd = MapColorPair(aRow.lineColor());
	if (aRow.marginColor() != adcui::COLOR_NULL)
		mMarginColor = MapColorPair(aRow.marginColor());

	QRect rc0(0, 0, p->device()->width(), p->device()->height());
	//QRect rm(rc0);
	//rm.setRight(leftMargin());
	QRectF rc(rc0);
	//rc.setLeft(rm.right() + 1);

	p->save();
	//p->setBackgroundMode(Qt::OpaqueMode);

	int w = charWidth();
	//int h = lineHeight();

	p->fillRect(rc, m_clrBkgnd.first);

	//draw gray rectangle around the current line (have to do it first so underscores are visible) 
	if (mbHighlightCaret)
	{
		if (aRow.isCurrent())//pIModel->checkEqual(curIt().line(), aRow.iter()) == 0)
			if (hasFocus())
			{
				p->setPen(s_highCaretFg);
				p->drawRect(rc0.left(), rc0.top(), rc0.right(), rc0.bottom() + 1);
			}
	}

	p->translate(QPoint(-m_nOffsetChar * w, 0));

	adcui::IADCTextModel *pIModel(hasModel() ? textModel() : nullptr);
	if (!pIModel->atEndIt(aRow.iter()))
	{
		assert(0);
		ADCCell cell;//pIModel->dataIt(drawIt)
		drawCell(p, rc, 0, cell, aRow.isCurrent());// , 0);
	}

	p->restore();

	m_clrBkgnd = saved1;
	mMarginColor = saved2;
	return true;
	//return drawLine(p, rc, drawIt, line);
}

void paintTest(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	int _width = 50;
	int _height = 80;
	/*
	 The drop shadow effect will be created by drawing a filled, rounded corner rectangle with a gradient fill.
	 Then on top of this will be drawn  filled, rounded corner rectangle, filled with a solid color, and offset such that the gradient filled
	 box is only visible below for a few pixels on two edges.

	 The total box size is _width by _height. So the top box will start at (0,0) and go to (_width-shadowThickness, _height-shadowThickness),
	 while the under box will be offset, and start at (shadowThickness+0, shadowThickness+0) and go to  (_width, _height).
	 */

	int shadowThickness = 3;

	QLinearGradient gradient;
	gradient.setStart(0, 0);
	gradient.setFinalStop(_width, 0);
	QColor grey1(150, 150, 150, 125);// starting color of the gradient - can play with the starting color and ,point since its not visible anyway

	// grey2 is ending color of the gradient - this is what will show up as the shadow. the last parameter is the alpha blend, its set
	// to 125 allowing a mix of th color and and the background, making more realistic shadow effect.
	QColor grey2(225, 225, 225, 125);

	gradient.setColorAt((qreal)0, grey1);
	gradient.setColorAt((qreal)1, grey2);

	QBrush brush(gradient);

	painter->setBrush(brush);

	// for the desired effect, no border will be drawn, and because a brush was set, the drawRoundRect will fill the box with the gradient brush.
	QPen _outterborderPen(Qt::black);
	_outterborderPen.setStyle(Qt::NoPen);
	painter->setPen(_outterborderPen);

	QPointF topLeft(shadowThickness + 0, shadowThickness + 0);
	QPointF bottomRight(_width, _height);

	QRectF rect(topLeft, bottomRight);

	painter->drawRoundedRect(rect, 25, 25); // corner radius of 25 pixels

	// draw the top box, the visible one
	QBrush brush2(QColor(255, 250, 200, 255), Qt::SolidPattern);

	painter->setBrush(brush2);

	QPointF topLeft2(0, 0);
	QPointF bottomRight2(_width - shadowThickness, _height - shadowThickness);

	QRectF rect2(topLeft2, bottomRight2);

	painter->drawRoundedRect(rect2, 25, 25);
}

void ADCTextView::setUpdateFlags(unsigned f)
{
#if(0)
	if (f != mUpdateFlags)
	{
		static int z = 0;
		QString qs(QString::asprintf("[%d] setUpdateFlags(%d)\n", z++, f));
		OutputDebugString((LPCWSTR)qs.utf16());
	}
#endif
	mUpdateFlags = f;
}

void ADCTextView::paintEvent(QPaintEvent *e)
{
#if(0)
	{
	static int z(0);
	QString qs(QString::asprintf("[%d] paintEvent(%d)\n", ++z, mUpdateFlags));
	OutputDebugString((LPCWSTR)qs.utf16());
	}
#endif
	//mFont = font();
	if (!mpViewport)
	{
		ADCTextViewBase::paintEvent(e);
		return;
	}

	QRect rcClient(mpViewport->contentsRect());
	QPainter P0(viewport());
	//P0.begin(mpViewport);

	adcui::IADCTextModel *pIModel(hasModel() ? textModel() : nullptr);
	if (!pIModel)//? || !mpTopIt)
	{
		//P0.fillRect(rcClient, QBrush(Qt::white));//lightGray));
		ADCTextViewBase::paintEvent(e);
		//P0.end();
		return;
	}

	int caretY(-1);

	if (!mUpdateFlags || (mUpdateFlags & (1 << UPDATE_CONTENTS)))
	{
#if(0)
	{
		static int z = 0;
		QString qs(QString::asprintf("paintEvent(%d)\n", z++));
		//std::wstring ws(qs.toStdWString());
		OutputDebugString((LPCSTR)qs.toUtf8());
	}
#endif

		qreal line_height = lineHeight();
		QMargins margins(viewport()->contentsMargins());

		//draw top margin
		if (margins.top() > 0)
		{
			ADCTextRow	aHeaderRow(adcui::DUMPOS(0));
			getColumnInfo(aHeaderRow, false);

			//draw top-left corner cell(s)
			if (margins.left() > 0)
			{
				QRect rcLine(0, 0, margins.left(), margins.top());
				QRect rcCacheLine(0, 0, rcLine.width(), rcLine.height());
				QImage pm(rcCacheLine.width(), rcCacheLine.height(), QImage::Format_RGB32);
				{
					QPainter painter(&pm);
					painter.setFont(font());
					DrawTopMargin(&painter, rcCacheLine, true, aHeaderRow);
				}

				P0.drawImage(rcLine.topLeft(), pm);
			}

			//draw top margin (not including a top-left corner cell)
			{
				QRect rcLine(rcClient.left(), 0, rcClient.right(), margins.top());
				QRect rcCacheLine(0, 0, rcLine.width(), rcLine.height());
				QImage pm(rcCacheLine.width(), rcCacheLine.height(), QImage::Format_RGB32);
				{
					QPainter painter(&pm);
					painter.setFont(font());
					DrawTopMargin(&painter, rcCacheLine, false, aHeaderRow);
				}

				P0.drawImage(rcLine.topLeft(), pm);
			}
		}

		
		QRectF rcLeftMargin(0, rcClient.top(), rcClient.left(), rcClient.bottom());//left margin
		QImage pm0(rcLeftMargin.width(), line_height, QImage::Format_RGB32);
		QPainter painter0(&pm0);
		painter0.setFont(font());
		QRectF rcLine0(rcLeftMargin);
		rcLine0.setBottom(rcLine0.top() + line_height);


		QImage pm(rcClient.width(), line_height, QImage::Format_RGB32);// main area
		QPainter painter(&pm);
		painter.setFont(font());
		QRect rcLine(rcClient);
		rcLine.setBottom(rcLine.top() + line_height);


		//if (!pm.isNull())
		{


			bool bAtEnd(false);
			int lineNo(0);
			for (adcui::AutoIter t(pIModel, topIt().line());
				rcLine.top() < rcClient.bottom();
				rcLine0.translate(0, line_height),
				rcLine.translate(0, line_height),
				lineNo++)
			{
				//mTab.reset();
				ADCTextRow aRow(pIModel->posFromIter(t));
				getRowInfo(aRow);
//fprintf(stdout, "DA=%s\n", aRow.cell(1).toLatin1().constData());fflush(stdout);
				aRow.setCurrent(pIModel->checkEqual(curIt().line(), aRow.iter()) == 0);
				if (aRow.isCurrent())
				{
					caretY = lineNo;
					OnCurrentRow(aRow);
				}

				OnNewRow(aRow);

				DrawLeftMargin(&painter0, aRow, lineNo);
				P0.drawImage(rcLine0.topLeft(), pm0);

				if (bAtEnd)//pIModel->atEndIt(t))
				{
					P0.drawImage(rcLine.topLeft(), pm);
				}
				else
				{
					DrawContentsLine(&painter, aRow, lineNo);
					//pt.drawPixmap(rcLine.topLeft(), pm);
					P0.drawImage(rcLine.topLeft(), pm);

					//draw line edit's selection
					if (mpEdit && mpEdit->origin().compareEq(aRow.iter()))
					{
						int x1(mpEdit->selBegPos());
						int x2(mpEdit->selEndPos());
						if (x1 != x2)
						{
							if (x2 < x1)
								qSwap(x1, x2);
							int wSel(x2 - x1);
							QRect r1(leftMargin() + mpEdit->startPosX() + x1 - m_nOffsetChar, topMargin() + lineNo, wSel, 1);
							QRect rcs(r1.x()*charWidth(), r1.y()*line_height, wSel*charWidth(), line_height - 2);//why -2 ?
							QRect rci(rcLine.intersected(rcs));
							if (!rci.isEmpty())
							{
								P0.save();
								//P.setBackgroundMode(Qt::OpaqueMode);
								P0.setBackgroundMode(Qt::TransparentMode);
#if(!TEST_COLORS)
								P0.setBackground(QBrush(Qt::darkGreen));
								P0.setCompositionMode(QPainter::CompositionMode_ColorBurn);
#else
								P0.setBackground(QBrush((Qt::GlobalColor)gTestColor));
								P0.setCompositionMode((QPainter::CompositionMode)gTestCompMode);
#endif
								P0.eraseRect(rci);
								P0.restore();
							}
						}
					}

					if (!pIModel->forwardIt(t))
					{
						bAtEnd = true;
						QRect rc(0, 0, painter.device()->width(), painter.device()->height());
						painter.fillRect(rc, m_clrBkgnd.first);
						//clean the bottom of the document...
					}
				}
			}

			

		}
	}

	if (mUpdateFlags & (1 << UPDATE_CURSOR))
	{
		if (mbShowCaret && mbCaretVisible)
		{
			//int caretY;
			//if (caretY >= 0)
			if (curIt().checkDistanceFrom(topIt(), pageHeight(), caretY))
				if (curIt().x() >= m_nOffsetChar)
				{
					//QPoint pt((curIt().x() + leftMargin() - m_nOffsetChar)*charWidth(),
						//(caretY + topMargin())*lineHeight());
					//	SetCaretPos(pt);

					DrawCaret(P0, curIt().x(), caretY);
				}
		}
	}

	//paintTest(&P0, 0, 0);

	mUpdateFlags = 0;
	//P0.end();
}

void ADCTextView::DrawCaret(QPainter &P0, int x0, int y0)
{
	qreal w(charWidth());
	qreal h(lineHeight());

	qreal x((x0 + leftMargin() - m_nOffsetChar) * w);
	qreal y((y0 + topMargin()) * h);

	// Draw caret Xored
	//?	pPainter->setRasterOp(Qt::NotXorROP);

	QPen pen(Qt::black, 2);
	P0.setPen(pen);

	P0.setCompositionMode(QPainter::RasterOp_NotSourceXorDestination);// CompositionMode_Xor);

	//?	pPainter->setBackgroundColor(Qt::white);
	P0.drawLine(x, y, x, y + h);

	//Restore painter
	//?	pPainter->setRasterOp(Qt::NotXorROP);

}

int ADCTextView::curLine()
{
	if (m_caretY < 0)//? && mpCurIt)
		curIt().checkDistanceFrom(topIt(), pageHeight(), m_caretY);
	return m_caretY;
}

void ADCTextView::updateCurPosInDoc()
{
	if (!hasModel())
		return;
	//fromScreenLine(curIt(), m_ptCaret.y());
	//curIt() = mCaret;
	int n;
	if (mpEdit && !mGrabMouse
		&& !mpEdit->isInside(curIt().line(), curIt().x(), n))
		stopInplaceEdit();
	if (!mpEdit)
		textModel()->setCurPosIt(curIt().line(), curIt().x());

	m_caretY = -1;
	//emit signalCurLineChanged(curLine() + topMargin());
	curLine();
}

int ADCTextView::fromScreenLine(ADCDocLine &a, int y)
{
	if (y < 0 || y > pageHeight())
		return 0;
	a = topIt();
	while (y--)
		++a;
	return 1;
}

/*void ADCTextView::slotRefPosChanged(int pos)
{
	mRefPos = pos;
}*/




ADCGotoDlg::ADCGotoDlg(ADCTextView* pParent, int nMax)
	: QDialog(pParent)//->topLevelWidget())
{
	setModal(false);

	setWindowTitle(tr("Go To Line"));
	setObjectName(tr("QSilGotoDlg"));
	
	//row 1
	mpLabel = new QLabel(this);
	mpLabel->setText(tr("&Line number (1 - %1):").arg(nMax));
	mpLabel->setWordWrap(false);

	//row2
	mpSpinBox = new QSpinBox(this);
	mpSpinBox->setMinimum(1);
	mpSpinBox->setMaximum(nMax);

	mpLabel->setBuddy(mpSpinBox);

	//row 3
	QSpacerItem *spacer1 = new QSpacerItem(21, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	mpOkBtn = new QPushButton(this);
	mpOkBtn->setText(tr("OK"));
	mpOkBtn->setDefault(true);
	connect(mpOkBtn, SIGNAL(clicked()), SLOT(slotOk()));

	mpCancelBtn = new QPushButton(this);
	mpCancelBtn->setText(tr("Cancel"));
	connect(mpCancelBtn, SIGNAL(clicked()), SLOT(reject()));

	QHBoxLayout *hbox1(new QHBoxLayout());
	hbox1->setSpacing(6);
	hbox1->addItem(spacer1);
	hbox1->addWidget(mpOkBtn);
	hbox1->addWidget(mpCancelBtn);

	//base layout
	QVBoxLayout *vbox(new QVBoxLayout(this));
	vbox->addWidget(mpLabel);
	vbox->addWidget(mpSpinBox);
	vbox->addLayout(hbox1);

	QWidget::setTabOrder(mpSpinBox, mpOkBtn);
	QWidget::setTabOrder(mpOkBtn, mpCancelBtn);

	resize(280, 100);
	setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	mpSpinBox->setFocus();
}

void ADCGotoDlg::setMaxLines(const int nMaxLine)
{
	mpSpinBox->setMaximum(nMaxLine);
}

int ADCGotoDlg::lineNumber() const
{
	return mpSpinBox->value();
}

void ADCGotoDlg::slotOk()
{
	emit lineNbrChanged(mpSpinBox->value());
	emit accept();
}



