
#include <QtGui/QClipboard>
#include <QAction>
#include <QMenu>
#include <QApplication>

#include "ADCLineEdit.h"
#include "ADCCell.h"
#include "ADCStream.h"


/////////////////////////////////////////////////////////////////////ADCDocLine

ADCDocLine::ADCDocLine(const ADCDocLine &o)
	: pIModel(o.pIModel),
	m_it(0)
{
	pIModel->AddRef();
	setIter(pIModel->newPosition());
	pIModel->copyIt(iter(), o.line());
}

ADCDocLine::ADCDocLine(adcui::IADCTextModel *_pIModel, adcui::DUMPOS copyFromIt)
	: pIModel(_pIModel),
	m_it(0)
{
	if (pIModel)
	{
		setIter(pIModel->newPosition());
		if (copyFromIt)
			pIModel->copyIt(iter(), copyFromIt);
	}
}

ADCDocLine::~ADCDocLine()
{
	if (pIModel)
		pIModel->deletePosition(iter());
	//pIModel->Release();
}

bool ADCDocLine::sety(const ADCDocLine &o, int dy)//set line relative to other iterator
{
	//if (dy == 0)
	//return false;
	if (pIModel->checkEqual(line(), o.line()) == 0)//equal?
	{
		if (!dy)
			return false;
	}
	else
		pIModel->copyIt(iter(), o.line());
	adcui::AutoIter j(pIModel, iter(), true);
	if (dy > 0)
	{
		for (int i(0); i < dy; i++)
			if (!pIModel->forwardIt(j))
				break;
	}
	if (dy < 0)
	{
		for (int i(0); i > dy; i--)
			if (!pIModel->backwardIt(j))
				break;
	}
	return true;
}

void ADCDocLine::operator=(const ADCDocLine &o)
{
	assert(pIModel == o.pIModel);
	pIModel->copyIt(iter(), o.line());
}


bool ADCDocLine::compareEq(adcui::DUMPOS pos) const
{
	return (pIModel->checkEqual(iter(), pos) == 0);
}

void ADCDocLine::copyFrom(ADCDocLine *p)
{
	pIModel->copyIt(iter(), p->line());
}

bool ADCDocLine::operator==(const ADCDocLine &o) const
{
	return (pIModel->checkEqual(iter(), o.line()) == 0);
}

bool ADCDocLine::operator<(const ADCDocLine &o) const
{
	return (pIModel->checkEqual(iter(), o.line()) < 0);
}

bool ADCDocLine::operator>(const ADCDocLine &o) const
{
	return (pIModel->checkEqual(iter(), o.line()) > 0);
}

void ADCDocLine::operator++()
{
	adcui::AutoIter t(pIModel, iter(), true);
	pIModel->forwardIt(t);
}

void ADCDocLine::operator--()
{
	adcui::AutoIter t(pIModel, iter(), true);
	pIModel->backwardIt(t);
}

bool ADCDocLine::checkDistanceFrom(const ADCDocLine &o, int maxy, int &result)
{
	assert(maxy != 0);
	//ADCDocLine t(o);
	adcui::AutoIter t(pIModel, o.line());
	if (maxy > 0)
	{
		for (int i(0); i < maxy; i++)
		{
			if (pIModel->checkEqual(iter(), pIModel->posFromIter(t)) == 0)
			{
				result = i;
				return true;
			}
			if (!pIModel->forwardIt(t))
				break;
		}
	}
	else if (maxy < 0)
	{
		for (int i(0); i > maxy; i--)
		{
			if (pIModel->checkEqual(iter(), pIModel->posFromIter(t)) == 0)
			{
				result = i;
				return true;
			}
			if (!pIModel->backwardIt(t))
				break;
		}
	}
	return false;
}

int ADCDocLine::shiftY(int dy)
{
	int i(0);
	if (dy != 0)
	{
		adcui::AutoIter t(pIModel, line(), true);
		if (dy > 0)
		{
			for (; i < dy; i++)
			{
				if (!pIModel->forwardIt(t))
					break;
			}
		}
		else// if (dy < 0)
		{
			for (; i > dy; i--)
			{
				if (!pIModel->backwardIt(t))
					break;
			}
		}
	}
	return i;
}



/////////////////////////////////////////////////////////////////////ADCDocPos
ADCDocPos::ADCDocPos(adcui::IADCTextModel *_pIModel, adcui::DUMPOS _lineIt, int x)
	: ADCDocLine(_pIModel, _lineIt),// hint),
	mx(x)
{
}

bool ADCDocPos::set(const ADCDocLine &o, int deltaY, int x)
{
	bool bRet(ADCDocLine::sety(o, deltaY));
	if (setX(x, false))
		bRet = true;
	return bRet;
}

bool ADCDocPos::setX(int x, bool)
{
	if (mx != x)
	{
		mx = x;
		return true;
	}
	return false;
}

bool ADCDocPos::shiftX(int deltaX, int minX, int maxX)
{
	int d(0);
	if (deltaX == 0)
		return false;//d;
	int ix(x() + deltaX);
	if (ix < minX)
	{
		d = deltaX - (minX - ix);
		ix = 0;
	}
	else if (ix >= maxX)
	{
		d = deltaX - (ix - maxX);
		ix = maxX - 1;
	}
	return setX(ix, true);
}

bool ADCDocPos::shiftHome()
{
	return setX(0);
}



/////////////////////////////////////////////////////////////////////ADCDocTablePos
ADCDocTablePos::ADCDocTablePos(adcui::IADCTextModel *_pIModel, adcui::DUMPOS _lineIt, int x)
	: ADCDocPos(_pIModel, _lineIt, x)//, hint)
{
//	assert(tableModel());
}

bool ADCDocTablePos::setX(int _x, bool bShift)
{
	int x0(x());
	if (!ADCDocPos::setX(_x))
		return false;

	if (!(x() & 1))
		return true;//check odd positions only

	const QString  &sti(cell(adcui::IBinViewModel::CLMN_TREEINFO));
	if (sti.isEmpty())
		return true;

	int x1(0), j(0);//visible
	for (uint i(0); i < cellsNum(); i++)
	{
		const ADCCell &aCell(cell(i));
		if (!aCell.isShown() || aCell.isHeader())
			continue;
		x1 += adcui::IADCTableModel::colTextShift(j);
		if (aCell.isTreeCell())
		{
			int x2(x1 + sti.length() * 2);
			if (x1 <= _x && _x < x2)
			{
				if (_x & 1)
				{
					if (bShift && _x == x0 + 1)//going right (only if shifting)
						return ADCDocPos::setX(_x + 1);
					return ADCDocPos::setX(_x - 1);
				}
			}
			break;
		}
		x1 += aCell.width();
		j++;
	}
	return true;
}

bool ADCDocTablePos::shiftHome()
{
	int visible(0);
	unsigned iStart(0);
	for (uint iCol(0); iCol < cellsNum(); iCol++)
	{
		const ADCCell &aCell(cell(iCol));
		if (aCell.isHeader())
			continue;
		int w(aCell.width());
		if (w <= 0)
			continue;
		if (!aCell.isEmpty())
		{
			iStart += adcui::IADCTableModel::colTextShift(visible);
			
			if (aCell.isTreeCell())
				iStart += cell(adcui::IBinViewModel::CLMN_TREEINFO).length() * 2;

			for (int i(0); i < aCell.length() && aCell[i].isSpace(); i++, iStart++);//scan for 1st no-space symbol

			int ix(x());
			if ((unsigned)ix > iStart)
				ix = iStart;
			else
				ix = 0;
			if (ADCDocPos::setX(ix))
				return true;
			break;
		}
		iStart += w;
		visible++;
	}
	return false;
}

bool ADCDocTablePos::shiftEnd()
{
	const ADCTextRow0 &aRow(*this);

	int iColLast(-1);
	int iColFirstVisible(-1);
	unsigned iStart(0), x(0);
	for (uint iCol(0); iCol < aRow.cellsNum(); iCol++)
	{
		const ADCCell &aCell(aRow.cell(iCol));
		if (aCell.isHeader())
			continue;
		int w(aCell.width());
		if (w <= 0)
			continue;
		if (!aCell.isEmpty())
		{
			iColLast = iCol;
			iStart = x;
		}
		x += w;
		if (iColFirstVisible < 0)
			iColFirstVisible = iCol;
	}
	if (iColLast == -1)
		return false;

	const ADCCell &aLastCell(aRow.cell(iColLast));

	unsigned iEnd(iStart);
	if (iColFirstVisible != iColLast || !adcui::IADCTableModel::bNo1TextShift)
		iEnd += adcui::IADCTableModel::iTextShift;

	if (aLastCell.isTreeCell())
		iEnd += aRow.cell(adcui::IBinViewModel::CLMN_TREEINFO).length() * 2;

	iEnd += aLastCell.length();

	return ADCDocPos::setX(iEnd);
}

adcui::IADCTableModel::COLID ADCDocTablePos::firstNonHeaderColumn()
{
	const ADCTextRow0 &aRow(*this);
	for (uint iCol(0); iCol < aRow.cellsNum(); iCol++)
	{
		const ADCCell &aCell(aRow.cell(iCol));
		if (aCell.isHeader())
			continue;
		int w(aCell.width());
		if (w > 0)
			return iCol;
	}
	return 0;
}




////////////////////////////////////////////////////////////////////////////////////
ADCLineEdit::ADCLineEdit(QWidget *parent, adcui::IADCTextEdit *p, ADCDocPos *pAtPos)
	: QObject(parent),
	mpIEdit(p),
	mpOrigin(pAtPos),
	mCurPos(0),
	mSelBegPos(0),
	mSelEndPos(0)
{
	MyStream ss;
	mpIEdit->readData(ss);
	MyString s;
	ss.ReadString(s);
	mData = s;
	mCurPos = mData.length();
	mSelEndPos = mCurPos;

	mpCutAction = new QAction(tr("Cut"), this);
	mpCutAction->setShortcut(Qt::CTRL | Qt::Key_X);
	mpCutAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(mpCutAction, SIGNAL(triggered()), SLOT(slotCut()));
	parent->addAction(mpCutAction);

	mpCopyAction = new QAction(tr("Copy"), this);
	mpCopyAction->setShortcut(tr("Ctrl+C"));
	mpCopyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(mpCopyAction, SIGNAL(triggered()), SLOT(slotCopy()));
	parent->addAction(mpCopyAction);

	mpPasteAction = new QAction(tr("Paste"), this);
	mpPasteAction->setShortcut(Qt::CTRL | Qt::Key_V);
	mpPasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(mpPasteAction, SIGNAL(triggered()), SLOT(slotPaste()));
	parent->addAction(mpPasteAction);

	mpSelectAllAction = new QAction(tr("Select All"), this);
	mpSelectAllAction->setShortcut(Qt::CTRL | Qt::Key_A);
	mpSelectAllAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(mpSelectAllAction, SIGNAL(triggered()), SLOT(slotSelectAll()));
	parent->addAction(mpSelectAllAction);
}

ADCLineEdit::~ADCLineEdit()
{
	/*parent->removeAction(mpCutAction);
	parent->removeAction(mpCopyAction);
	parent->removeAction(mpPasteAction);
	parent->removeAction(mpSelectAllAction);*/
	//delete mpCutAction;
	//delete mpCopyAction;
	//delete mpPasteAction;
	//delete mpSelectAllAction;
	delete mpOrigin;
	mpIEdit->Release();
}

void ADCLineEdit::PopulateEditMenu(QMenu &r)
{
	r.addAction(mpCutAction);
	r.addAction(mpCopyAction);
	r.addAction(mpPasteAction);
	r.addSeparator();
	r.addAction(mpSelectAllAction);
}

void ADCLineEdit::slotCut()
{
	if (hasSel())
	{
		QClipboard *clipboard = QApplication::clipboard();
		clipboard->setText(mData.mid(mSelBegPos, mSelEndPos - mSelBegPos));
		deleteSel();
	}
}

void ADCLineEdit::slotCopy()
{
	if (hasSel())
	{
		QClipboard *clipboard = QApplication::clipboard();
		//QString originalText = clipboard->text();
		clipboard->setText(mData.mid(mSelBegPos, mSelEndPos - mSelBegPos));
	}
}

void ADCLineEdit::slotPaste()
{
	QClipboard *clipboard = QApplication::clipboard();
	QString originalText = clipboard->text();
	deleteSel();
	QString data(mData);
	data.insert(mCurPos, originalText);
	setData(data);
	setCur(mCurPos + originalText.length());
}

void ADCLineEdit::slotSelectAll()
{
	setCur(mData.length());
	setSel(0, mCurPos);
}

void ADCLineEdit::OnApply()
{
	mpIEdit->apply();
}

int ADCLineEdit::OnChar(char c)
{
	deleteSel();
	QString data(mData);
	data.insert(mCurPos, c);
	setData(data);
	setCur(mCurPos + 1);
	return 1;
}

int ADCLineEdit::OnBackspace()
{
	if (deleteSel())
		return 1;
	if (mCurPos > 0)
	{
		QString data(mData);
		data.remove(mCurPos - 1, 1);
		setData(data);
		setCur(mCurPos - 1);
		return 1;
	}
	return 0;
}

int ADCLineEdit::OnDelete()
{
	if (deleteSel())
		return 1;
	if (mCurPos < mData.length())
	{
		QString data(mData);
		data.remove(mCurPos, 1);
		setData(data);
		return 1;
	}
	return 0;
}

int ADCLineEdit::OnLeft(bool bShiftModifier)
{
	if (bShiftModifier)
	{
		if (mCurPos > 0)
		{
			int beg(mSelBegPos);
			int end(mSelEndPos);
			if (!hasSel())
			{
				end = mCurPos;
				setCur(mCurPos - 1);
				beg = mCurPos;
			}
			else
			{
				setCur(mCurPos - 1);
				if (mCurPos >= beg)
					end = mCurPos;
				else
					beg = mCurPos;
			}
			setSel(beg, end);
		}
	}
	else
	{
		clearSel();
		if (mCurPos > 0)
			setCur(mCurPos - 1);
	}
	return 0;
}

int ADCLineEdit::OnRight(bool bShiftModifier)
{
	if (bShiftModifier)
	{
		if (mCurPos < mData.length())
		{
			int beg(mSelBegPos);
			int end(mSelEndPos);
			if (!hasSel())
			{
				beg = mCurPos;
				setCur(mCurPos + 1);
				end = mCurPos;
			}
			else
			{
				setCur(mCurPos + 1);
				if (mCurPos <= end)
					beg = mCurPos;
				else
					end = mCurPos;
			}
			setSel(beg, end);
		}
	}
	else
	{
		clearSel();
		if (mCurPos < mData.length())
			setCur(mCurPos + 1);
	}
	return 0;
}

int ADCLineEdit::OnHome(bool bShiftModifier)
{
	if (bShiftModifier)
	{
		if (mCurPos > 0)
		{
			int end(mCurPos);
			setCur(0);
			setSel(mCurPos, end);
		}
	}
	else
	{
		clearSel();
		if (mCurPos > 0)
			setCur(0);
	}
	return 0;
}

int ADCLineEdit::OnEnd(bool bShiftModifier)
{
	if (bShiftModifier)
	{
		if (mCurPos < mData.length())
		{
			int beg(mCurPos);
			setCur(mData.length());
			setSel(beg, mCurPos);
		}
	}
	else
	{
		clearSel();
		if (mCurPos < mData.length())
			setCur(mData.length());
	}
	return 0;
}

bool ADCLineEdit::isInside(adcui::DUMPOS y, int x, int &n) const
{
	if (!origin().compareEq(y))
		return false;
	n = x - mpOrigin->x();
	if (n < 0)
		return false;
	if (n > mData.length())
		return false;
	return true;
}

int ADCLineEdit::OnClick(adcui::DUMPOS y, int x, bool bPreserveSel)
{
	int n;
	if (!isInside(y, x, n))
		return 0;
	if (bPreserveSel)
		return 1;
	setCur(n);
	setSel(n, n);
	return 1;
}

int ADCLineEdit::OnDoubleClick(adcui::DUMPOS y, int x)
{
	int n;
	if (!isInside(y, x, n))
		return 0;
	setCur(n);
	setSel(0, mData.length());
	return 1;
}

int ADCLineEdit::OnMove(int x)
{
	int n(x - mpOrigin->x());
	if (n < 0)
		n = 0;
	if (n > mData.length())
		n = mData.length();
	if (n == mCurPos && n == mSelEndPos)
		return 0;
	setCur(n);
	setSel(mSelBegPos, n);
	return 1;
}

int ADCLineEdit::setData(QString s)
{
	if (mData == s)
		return 0;
	mData = s;
	ADCStream ss;
	ss.WriteString(mData);
	mpIEdit->writeData(ss);
	emit signalDataChanged();
	return 1;
}

int ADCLineEdit::setCur(int n)
{
	if (mCurPos == n)
		return 0;
	mCurPos = n;
	emit signalCurrentChanged();
	return 1;
}

int ADCLineEdit::setSel(int beg, int end)
{
	if (mSelBegPos == beg && mSelEndPos == end)
		return 0;
	mSelBegPos = beg;
	mSelEndPos = end;
	emit signalSelectionChanged();
	return 1;
}

int ADCLineEdit::deleteSel()
{
	if (hasSel())
	{
		QString data(mData);
		data.remove(mSelBegPos, mSelEndPos - mSelBegPos);
		setData(data);
		setCur(mSelBegPos);
		return clearSel();
	}
	return 0;
}

int ADCLineEdit::clearSel()
{
	if (hasSel())
		return setSel(0, 0);
	return 0;
}

bool ADCLineEdit::hasSel() const
{
	return (mSelBegPos != mSelEndPos);
}

