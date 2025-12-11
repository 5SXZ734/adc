#include <iostream>
#include <assert.h>
#include <stdarg.h>

#include <QtCore/QEvent>
#include <QtCore/QSignalMapper>
#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QPixmap>
#include <QtGui/QCursor>
#include <QtGui/QColor>
#include <QtGui/QKeyEvent>
//#include <QtGui/QEvent>
#include <QApplication>
#include <QLayout>
#include <QScrollBar>
#include <QLineEdit>
#include <QToolTip>
#include <QToolButton>
#include <QComboBox>
#include <QMenu>
#include <QLabel>
#include <QAction>

#include "ADCBinWin.h"
//#include "ADCTypesWin.h"
#include "ADCUtils.h"
#include "colors.h"

using namespace adcui;

QWidget* ADCBinView::gpActiveWin = nullptr;

bool ADCBinView::checkActiveWin(QWidget *p)
{
	if (gpActiveWin == p)
		return false;
	gpActiveWin = p;
#if(0)
	static int g(0);
	fprintf(stderr, "Active window changed (%d)\n", ++g);
	fflush(stderr);
#endif
	return true;
}

ADCBinView::ADCBinView(QWidget *parent)
: ADCBinViewBase(parent, "ADCBINVIEW"),
	//?mpIModel(nullptr),
	mModelHint(0),
	//m_bSyncMode(false),
	mpModelData(nullptr),
	muContextId(0)
{
	connect(this, SIGNAL(signalRefresh()), SLOT(slotRefresh()));

	//have to intercept 
	/*QAction *pFakeAction = new QAction(tr("Nothing"), this);
	pFakeAction->setShortcut(QKeySequence(tr("Shift+D")));
	pFakeAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(pFakeAction, SIGNAL(triggered()), SLOT(slotNothing()));
	addAction(pFakeAction);*/

	
	mpEditNameAction = newAction(tr("Edit Name"), [this]() { editName(); }, Qt::Key_N);
	mpEnterTypeAction = newAction(tr("Edit Type"), SLOT(slotEnterType()),   Qt::Key_T);
	//mpBuildTypeAction = newAction(tr("Build Type ..."), SLOT(slotBuildType()), tr("Ctrl+T"), QIcon(":types.png"));
	mpMakeAsciiAction = newAction(tr("Make ASCII"), SLOT(slotMakeAscii()), Qt::Key_A);
	mpMakeTextAction = newAction(tr("Make Text"), SLOT(slotMakeText()), Qt::Key_Y);
	mpMakeCodeAction = newAction(tr("Make Code"), SLOT(slotMakeCode()), Qt::Key_C);
	mpMakeThunkAction = newAction(tr("Make Thunk"), SLOT(slotMakeThunk()), Qt::Key_H);
	mpMakeIntAction = newAction(tr("Make Integer"), SLOT(slotMakeData()), Qt::Key_D);
	mpToggleSignAction = newAction(tr("Toggle Sign"), SLOT(slotToggleSign()), Qt::Key_S);
	mpMakeRealAction = newAction(tr("Make Real"), SLOT(slotMakeReal()), Qt::Key_G);
	mpMakeBitAction = newAction(tr("Make Bit"), SLOT(slotMakeBit()), Qt::Key_B);
	mpSetFunctionEndAction = newAction(tr("Set Function End"), SLOT(slotSetFunctionEnd()), Qt::Key_E);
	mpMakeFunctionAction = newAction(tr("Make Function"), SLOT(slotMakeFunction()), Qt::Key_F);
	mpMakeOffsetAction = newAction(tr("Make Offset"), SLOT(slotOffset()), Qt::Key_O);
	mpMakeArrayAction = newAction(tr("Make Array"), SLOT(slotMakeArray()), Qt::Key_R);
	mpMakeUnicodeAction = newAction(tr("Make UNICODE"), SLOT(slotMakeUnicode()), Qt::Key_U);
	mpMakeUntypedAction = newAction(tr("Make Untyped"), SLOT(slotMakeUntyped()), Qt::Key_X);
	mpMakeCloneAction = newAction(tr("Make a Clone"), SLOT(slotMakeClone()), Qt::Key_Z);
	mpToggleCollapsedAction = newAction(tr("Collapse/Expand"), SLOT(slotToggleCollapsed()), Qt::Key_QuoteLeft);
	mpMakeGapAction = newAction(tr("Make Gap"), SLOT(slotMakeGap()), Qt::Key_M);
	mpInstantiateTypeAction = newAction(tr("Instantiate Type"), SLOT(slotInstantiateType()), tr("CTRL+SHIFT+I"));
	mpGoToAction = newAction(tr("Go To Location"), SIGNAL(signalGoToLocation()), tr("Ctrl+G"), QIcon(":go_page.png"));

	mpToggleExportedAction = newAction(tr("Toggle Exported"), SLOT(slotToggleExported()));
	mpToggleImportedAction = newAction(tr("Toggle Imported"), SLOT(slotToggleImported()));

	mpNewScopeStrucAction = newAction(tr("Make Sub-Scope"), SLOT(slotNewScopeStruc()));
	//mpNewScopeUnionAction = newAction(tr("Make Union"), SLOT(slotNewScopeUnion()));
	//mpNewScopeBitsetAction = newAction(tr("Make Bitset"), SLOT(slotNewScopeBitset()));
	mpNewScopeSegAction = newAction(tr("Make Segment"), SLOT(slotNewScopeSeg()));


	mpDeleteAction = newAction(tr("Delete"), SLOT(slotDelete()), Qt::Key_Delete, QIcon(":delete_16.png"));
	mpGoToSelAction = newAction(tr("Go To Selected"), SLOT(slotGoToSelected()), Qt::Key_Return);
	mpGoBackAction = newAction(tr("Go Back"), SLOT(slotGoBack()), Qt::Key_Escape, QIcon(":go_back.png"));

	mpPrintObjInfoAction = newAction(tr("[DEBUG] Print Object Info"), SLOT(slotPrintObjInfo()));

	mpSyncAction = newAction(tr("Synchronize"), SLOT(slotForceSynchronize()), Qt::Key_Space);

	enableContextActions(true);

	/*mpDecompileFuncAction = new QAction(QIcon(":wrench.16.png"), tr("&Decompile Function"), this);
	mpDecompileFuncAction->setShortcut(Qt::Key_F5);
	mpDecompileFuncAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(mpDecompileFuncAction, SIGNAL(triggered()), SLOT(slotDecompileFunction()));*/

	//addAction(mpDecompileFuncAction);
	//mpJumpTargetAction = new QAction(tr("Jump Target"), this);
	//connect(mpJumpTargetAction, SIGNAL(triggered()), SLOT(slotJumpTarget()));

	mpSaveListingAction = new QAction(QIcon(":save_listing.png"), tr("Save Listing..."), this);
	connect(mpSaveListingAction, SIGNAL(triggered()), SLOT(slotSaveListing()));

	//mpMakeArrayAction = new QAction(tr("Make Array..."), this);
	//connect(mpMakeArrayAction, SIGNAL(triggered()), SLOT(slotMakeArray()));

	mpToggleCompactViewAction = new QAction(QIcon(":compact.png"), tr("Compact Mode"), this);
	mpToggleCompactViewAction->setCheckable(true);
	connect(mpToggleCompactViewAction, SIGNAL(toggled(bool)), SLOT(slotSetCompact(bool)));

	mpToggleValuesOnlyAction = new QAction(QIcon(":gem_doc.png"), tr("Hide Types"), this);
	mpToggleValuesOnlyAction->setCheckable(true);
	connect(mpToggleValuesOnlyAction, SIGNAL(toggled(bool)), SLOT(slotHideTypes(bool)));

//	mpGoBackAction = new QAction(QIcon(":go_back.png"), tr("Go Back"), this);
	//connect(mpGoBackAction, SIGNAL(triggered()), SLOT(slotGoBack()));

	mpResolveRefsAction = new QAction(QIcon(":resolve.png"), tr("Resolve References"), this);
	mpResolveRefsAction->setCheckable(true);
	connect(mpResolveRefsAction, SIGNAL(toggled(bool)), SLOT(slotResolveRefs(bool)));

	mpGotoEntryAction = new QAction(QIcon(":go.png"), tr("Go to Address"), this);
	//connect(mpGotoEntryAction, SIGNAL(clicked()), SLOT(slotGo2Location()));

	mpRefreshAction = new QAction(QIcon(":refresh.png"), tr("Refresh"), this);
	connect(mpRefreshAction, SIGNAL(triggered()), SLOT(slotRefresh()));

//?	setInputMethodEnabled(true);

	mMarginExtra = QSize(2, 0);

	m_clrBkgnd = MapColorPair(COLOR_DASM_BGND);
	mbHighlightCaret = true;

	mPixmaps.push_back(QPixmap(":dbg_next.png"));
	mPixmaps.push_back(QPixmap(":dbg_bp.png"));
	mPixmaps.push_back(QPixmap(":dbg_bp_next.png"));
}

ADCBinView::~ADCBinView()
{
	//setModel(nullptr);
	//delete mpPainter;
}

void ADCBinView::createContents()
{
	deleteContents();
	ADCBinViewBase::createContents();
	connect(mpHorzScrollBar, SIGNAL(actionTriggered(int)), SLOT(slotHSBarAction(int)));
	connect(mpVertScrollBar, SIGNAL(actionTriggered(int)), SLOT(slotVSBarAction(int)));
}

template<typename Func>
	QAction* ADCBinView::newAction(const QString& name,
					   Func&& handler,
					   const QKeySequence& key,
					   const QIcon& icon,
					   QWidget* parent)
	{
		if (!parent)
			parent = this;

		QAction* act = new QAction(icon, name, this);
		mActions.append(act);

		act->setShortcut(key);
		act->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		act->setShortcutVisibleInContextMenu(true);
		QObject::connect(act, &QAction::triggered, parent,
						 [fn = std::forward<Func>(handler)](bool /*checked*/) mutable {
							 fn();   // your lambda: no bool param needed
						 });
		return act;
	}


QAction *ADCBinView::newAction(QString name, const char *slot, const QKeySequence &key, const QIcon &icon, QWidget *parent)
{
	if (!parent)
		parent = this;
	QAction *pAction(new QAction(icon, name, this));
	mActions.append(pAction);
	pAction->setShortcut(key);
	pAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
#if QT_VERSION >= 0x050A00//05.10.00
	pAction->setShortcutVisibleInContextMenu(true);
#endif
	connect(pAction, SIGNAL(triggered()), parent, slot);
	return pAction;
}

QAction *ADCBinView::newActionCheck(QString name, const char *slot, const QKeySequence &key, const QIcon &icon, QWidget *parent)
{
	if (!parent)
		parent = this;
	QAction *pAction(new QAction(icon, name, this));
	mActions.append(pAction);
	pAction->setCheckable(true);
	pAction->setShortcut(key);
	pAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
#if QT_VERSION >= 0x050A00//05.10.00
	pAction->setShortcutVisibleInContextMenu(true);
#endif
	connect(pAction, SIGNAL(toggled(bool)), parent, slot);
	return pAction;
}

void ADCBinView::enableContextActions(bool bEnable)
{
	ADCBinViewBase::enableContextActions(bEnable);
	for (QList<QAction *>::Iterator i(mActions.begin()); i != mActions.end(); i++)
	{
		if (bEnable)
			addAction(*i);
		else
			removeAction(*i);
	}
}

void ADCBinView::slotNothing()
{
}

QAction *ADCBinView::addContextAction(QAction *pAction)
{
	/*QAction *pAction(new QAction(pix, s, this));
	if (!shortcut.isEmpty())
	{
		pAction->setShortcut(QKeySequence(shortcut));
		pAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	}*/
	mExtraActions.push_back(pAction);
	addAction(pAction);
	return pAction;
}

void ADCBinView::slotDisassemblyViewToggled(bool bOn)
{
	if (!bOn)
	{
		emit signalToggleView(this);
	}
}

void ADCBinView::updateDataFromLocus(int /*dumpId*/)
{
	bool bRefresh(false);
	{
		ModelLocker lock(this);
		if (lock.pmodel() && lock.model().reloadRawData())
			bRefresh = true;
	}
	if (bRefresh)
	//if (hasModel() && binModel()->reloadRawData())
	{
		//emit signalRefreshBinaryDump(dumpId);
		updateContents();
	}
}

void ADCBinView::populateMenu(QMenu *pMenu) const
{
	adcui::IBinViewModel* pIModel(hasModel() ? binModel() : nullptr);
	if (pIModel)
	{
		pMenu->addAction(mpToggleCompactViewAction);
		pMenu->addAction(mpToggleValuesOnlyAction);
		pMenu->addAction(mpResolveRefsAction);

		mpToggleCompactViewAction->setChecked(pIModel->isCompactMode());
		mpToggleValuesOnlyAction->setChecked(pIModel->isValuesOnlyMode());
		mpResolveRefsAction->setChecked(pIModel->isResolveRefsMode());
	}
}

adcui::IBinViewModel *ADCBinView::reset2(QString s)
{
	emit signalRequestModel(*this, s);
	updateContents();
	if (hasModel())
		return binModel();
	return nullptr;
}

ADCModelData *ADCBinView::setModelData(ADCModelData *pModelData)
{
	if (mpModelData == pModelData)
		return mpModelData;

	//ADCModelData *old(mpModelData);
	//QString moduleName(mpModelData ? binModel()->moduleName() : QString());
#if(0)
	if (mpModelData)
		mpModelData->Release();//weak!
#endif
	mpModelData = pModelData;
	if (mpModelData)
	{
//weak!		mpModelData->AddRef();
		createContents();

		syncViewportMargins();
	}
	else
	{
		//emit signalReleaseModel(moduleName + MODULE_SEP);
		deleteContents();
	}

	updateActions();

	emit signalModelChanged();

	slotReset();
	return mpModelData;
}

void ADCBinView::updateActions()
{
	adcui::IBinViewModel* pIModel(hasModel() ? binModel() : nullptr);

	mpToggleCompactViewAction->setChecked(pIModel ? pIModel->isCompactMode() : false);
	mpToggleValuesOnlyAction->setChecked(pIModel ? pIModel->isValuesOnlyMode() : false);
	mpResolveRefsAction->setChecked(pIModel ? pIModel->isResolveRefsMode() : false);
}

void ADCBinView::slotClose(QString)
{
	setModelData(nullptr);
	/*adcui::IBinViewModel *pBinModel(binModel());
	QString moduleName(mpModelData ? pBinModel->moduleName() : QString());
	if (!moduleName.isEmpty())
		emit signalReleaseModel(moduleName + MODULE_SEP);*/
}

void ADCBinView::slotReset()
{
	ModelLocker lock(this);
	m_nOffsetChar = 0;

	OnContentsChanged();

	//SetTopAddrAbs(0);

	setHScrollBar();
	setVScrollBar();

//	updateContents();
}

void ADCBinView::showEvent(QShowEvent * e)
{
	syncViewportMargins();
	ADCBinViewBase::showEvent(e);
}

void ADCBinView::focusInEvent(QFocusEvent *e)
{
	ADCBinViewBase::focusInEvent(e);
	if (checkActiveWin(this))
	{
		ModelLocker lock(this);
		updateCurPosInDoc();
	}
}

void ADCBinView::focusOutEvent(QFocusEvent *e)
{
	ADCBinViewBase::focusOutEvent(e);
	muContextId = 0;
}

void ADCBinView::SetTopAddrAbs(int line)
{
	if (hasModel())
		binModel()->seekLineIt(topIt().line(), line);
}

void ADCBinView::slotContextIdChanged(unsigned u)
{
	if (!hasFocus())
		return;
	muContextId = u;
}

/*
void CdAsmView::CalcLineCharDim()
{
	CDC *pdc = GetDC();
	CFont *pOldFont = pdc->GetCurrentFont();
	if (GetFont())
		pdc->SelectObject(GetFont());
	CSize szCharExt = pdc->GetTextExtent(_T("X"));
	m_nLineHeight = szCharExt.cy+1;
	if (m_nLineHeight < 1)
		m_nLineHeight = 1;
	m_nCharWidth = szCharExt.cx - 1;
	pdc->SelectObject(pOldFont);
	ReleaseDC(pdc);
}
*/

#if(SELWORD)
static int checkSelWord( const char * s, int pos, QPair<int, int> &pt )
{
	if ( pos < 0 )
		return 0;

	int start = 0;
	int len = 0;
	while (1)
	{
		char c = *s++;
		if ( c == 0 )
		{
			if ( pos > 0 )
				return 0;
			break;
		}

		if (isalnum(c) || ((c) == '_') || ((c) == '@') || ((c) == '?'))
		{
			len++;
		}
		else
		{
			if ( pos == 0 )
				break;
			start += len;
			start++;
			len = 0;
		}

		if ( pos > 0 )
			pos--;
	}

	if ( len > 0 )
	{
		pt.first = start;
		pt.second = len;
		return 1;
	}

	return 0;
}
#endif

static const char *strtag(const char *s)
{
	while (*s && (unsigned char)*s < 0x80)
		s++;
	return *s ? s : nullptr;
}

template<typename T> int toUcs4_helper(const unsigned short *uc, int length, T *out)
{
	int i = 0;
	for (; i < length; ++i) {
		uint u = uc[i];
		if (QChar::isHighSurrogate(u) && i < length-1) {
			ushort low = uc[i+1];
			if (QChar::isLowSurrogate(low)) {
				++i;
				u = QChar::surrogateToUcs4(u, low);
			}
		}
		*out = T(u);
		++out;
	}
	return i;
}

static int toWCharArray(const QString &s, wchar_t *array)
{
	if (sizeof(wchar_t) == sizeof(QChar)) {
		memcpy(array, s.utf16(), sizeof(wchar_t)*s.length());
		return s.length();
	} else {
		return toUcs4_helper<wchar_t>(s.utf16(), s.length(), array);
	}
}

/*static QStdWString toStdWString(const QString &s)
{
	QStdWString str;
	str.resize(s.length());
	if (!s.length())
		return str;
	str.resize(toWCharArray(s, &(*str.begin())));
	return str;
}*/

/*
int ADCBinView::CheckWordUnderCaret(DRAW_t& D)
{
	if ( !hasFocus() )
		return 0;
	if ( D.mscr_line != m_ptCaret.y() )
		return 0;

	m_sSelWord = "";

	int caretX = m_ptCaret.x() + m_nOffsetChar;

	//get column where cursor is placed
	int left = 0;
	int right = left;
	for (int col = 0; col < CLMN_TOTAL; col++)
	{
		right += ColumnWidth(col);
		if (caretX < right)
			break;
		left = right;
	}

	if (col == CLMN_TOTAL)
		return 0;

	if (!D.col_str(col))
		return 0;

	//cursor position in column
	D.msel_pos = caretX - left;
	if (col != CLMN_FILE)
		D.msel_pos--;

	if ( col == CLMN_NAMES )
		D.msel_pos -= D.mlevel << 1;

	if (D.msel_pos < 0)
		return 0;

	if ((int)strlen(D.col_str(col)) < D.msel_pos)
		return 0;

	//scan for start of word
	int c = D.col_str(col)[D.msel_pos];
	if (__iscsym(c))
	while (D.msel_pos > 0)
	{
		c = D.col_str(col)[D.msel_pos - 1];
		if (!__iscsym(c))
			break;
		D.msel_pos--;
	}

	//get length of word
	D.msel_len = 0;
	while (__iscsym(D.col_str(col)[D.msel_pos + D.msel_len]))
		D.msel_len++;
	if (D.msel_len == 0)
		return 0;

	m_sSelWord = QString( D.col_str(col) ).mid( D.msel_pos, D.msel_len );
//	memcpy(m_sSelWord.GetBuffer(D.msel_len + 1), &D.mpArr[col][D.msel_pos], D.msel_len);
//	m_sSelWord.ReleaseBuffer(D.msel_len + 1);
//	m_sSelWord.SetAt(D.msel_len, 0);
	
	D.msel_pos += left;
	if (col != CLMN_FILE)
		D.msel_pos += 1;
	if ( col == CLMN_NAMES )
		D.msel_pos += D.mlevel << 1;
	return 1;
}*/



/////////////////////////////////////////////////////////////////////////////
// CdAsmView drawing

static int g_count(0);

void ADCBinView::paintEvent(QPaintEvent *e)
{
#if(SELWORD)
	m_sSelWord = "";
#endif
//	fprintf(stdout, "ADCBinView::paintEvent(%d)\n", g_count++);fflush(stdout);

	ModelLocker lock(this);
	ADCBinViewBase::paintEvent(e);
}

void ADCBinView::keyPressEvent(QKeyEvent *e)
{
	ModelLocker lock(this);
	ADCBinViewBase::keyPressEvent(e);
}

void ADCBinView::emitSynchronize(bool bForce)
{
	emit signalSynchronize(curLine() + topMargin(), bForce);
}

void ADCBinView::mousePressEvent(QMouseEvent *e)
{
	ModelLocker lock(this);
	ADCBinViewBase::mousePressEvent(e);
}

void ADCBinView::mouseReleaseEvent(QMouseEvent *e)
{
	if (!hasModel())
		return;

	bool bRefresh(false);
	{
		int iCol(columnResizing());
		if (iCol < 0)
			emitSynchronize(false);
		else if (iCol == adcui::IBinViewModel::CLMN_CODE)
			bRefresh = true;
		{
		ModelLocker lock(this);
		ADCBinViewBase::mouseReleaseEvent(e);
		}
	}
	//must not be locked
	if (bRefresh)
		emit signalRefreshBinaryDump(-1);
}

void ADCBinView::mouseMoveEvent(QMouseEvent *e)
{
	ModelLocker lock(this);
	ADCBinViewBase::mouseMoveEvent(e);
}

void ADCBinView::wheelEvent(QWheelEvent *e)
{
	ModelLocker lock(this);
	ADCBinViewBase::wheelEvent(e);
}

void ADCBinView::closeEvent(QCloseEvent *e)
{
	setModelData(nullptr);
	//ADCBinViewBase::closeEvent(e);
	e->ignore();
}

/*
LRESULT CdAsmView::OnSelectObj(WPARAM wParam, LPARAM lParam)
{
	if (!IsWindowVisible())
		return 0;

	FieldPtr  pLoc = (FieldPtr )wParam;
	DWORD dwAddr = pLoc->offset();
	GoToAddress(dwAddr);
	return 1;
}*/

void ADCBinView::slotProjectModified()
{
	//if (mpIModel)
		//mpIModel->invalidate(false);
	updateContents();
}

void ADCBinView::slotRefresh0()
{
	//if (mpIModel)
		//mpIModel->invalidate(true);
	updateContents();
}

void ADCBinView::slotRefresh()
{
	emit signalRefreshBinaryDump(-1);
	updateContents();
}

void ADCBinView::slotGlobalsModified()
{
	//if (mpIModel)
		//mpIModel->invalidate(false);
#if(0)
	if (hasFocus())
	{
		ModelLocker lock(this);

#if(0)
		updateCurPosInDoc();
#else
		int linesFromTop(curLine());
		if (linesFromTop >= 0)
		{
			topIt().copyFrom(&curIt());
			curIt().copyFrom(&topIt());//validate cur!
			//Q_ASSERT(line >= 0);
			/*		if (line < 0)
						line = 0;//mRefPos;
						int linesFromTop(line - topMargin());*/
			if (0 < linesFromTop && linesFromTop < pageHeight())
			{
				modelData().scrollUp(linesFromTop);
			}
		}
		updateCurPosInDoc();
		setVScrollBar();
#endif

	}
#endif
	updateContents();
}

void ADCBinView::slotLocusAdjusted()
{
	if (!hasFocus())
		return;

	ModelLocker lock(this);

	if (lock.pmodel() && lock.model().seekPosIt("$sync", topIt().line()))
	{
		curIt().copyFrom(&topIt());
		int line(curLine());
		if (line < 0)
			line = 0;//mRefPos;
		int linesFromTop(line);// -topMargin());
		if (0 < linesFromTop && linesFromTop < pageHeight())
		{
			modelData().scrollUp(linesFromTop);
			/*AutoIter t(lock.pmodel(), topIt().line(), true);
			while (linesFromTop-- > 0)
				--t;*/
		}
		setVScrollBar();
	}
	updateContents();
}

void ADCBinView::slotNewScopeStruc()
{
	emit signalPostCommand("mkscope -s");
}

/*void ADCBinView::slotNewScopeUnion()
{
	emit signalPostCommand("mkscope -u");
}

void ADCBinView::slotNewScopeBitset()
{
	emit signalPostCommand("mkscope -b");
}*/

void ADCBinView::slotNewScopeSeg()
{
	emit signalPostCommand("mkseg");
}

void ADCBinView::slotDelete() 
{
	emit signalPostCommand("del");
}

void ADCBinView::slotMakeUntyped() 
{
	emit signalPostCommand("makeunk");
}

void ADCBinView::slotMakeClone() 
{
	emit signalPostCommand("make_clone");
}

void ADCBinView::slotToggleCollapsed() 
{
	emit signalPostCommand("togl");
}

void ADCBinView::slotMakeCode() 
{
	emit signalPostCommand("makecode");
}

void ADCBinView::slotMakeThunk() 
{
	emit signalPostCommand("makethunk");
}

void ADCBinView::slotMakeData() 
{
	emit signalPostCommand("makeint -f");
}

void ADCBinView::slotToggleSign()
{
	emit signalPostCommand("make_signed -f");
}

void ADCBinView::slotMakeReal() 
{
	emit signalPostCommand("makereal -f");
}

void ADCBinView::slotMakeBit() 
{
	emit signalPostCommand("makebit -f");
}

void ADCBinView::slotMakeAscii() 
{
	emit signalPostCommand("makeint -f -a");
}

void ADCBinView::slotMakeText() 
{
	emit signalPostCommand("makeint -f -a -t");
}

void ADCBinView::slotMakeUnicode() 
{
	emit signalPostCommand("makeint -f -u");
}

void ADCBinView::slotMakeGap()
{
	emit signalPostCommand("mkgap -f");
}

void ADCBinView::slotInstantiateType()
{
	emit signalPostCommand("instantiate");
}

void ADCBinView::slotToggleExported()
{
	emit signalPostCommand("toggle_exported");
}

void ADCBinView::slotToggleImported()
{
	emit signalPostCommand("toggle_imported");
}

void ADCBinView::slotMakeFunction() 
{
	//emit signalPostCommand("makefunc -f -@%VA%");
	emit signalPostCommand("makefunc -f");
}

void ADCBinView::slotMakeArray()
{
	//ADCArrayDlg dlg(this);
	//dlg.exec();
	//mpIModel->postCommandIt(curIt().line(), "makearr");
	emit signalPostCommand("makearr");
}

void ADCBinView::slotSetFunctionEnd() 
{
	emit signalPostCommand("funcend");
}

bool ADCBinView::event(QEvent* pEvent)
{
	switch (pEvent->type())
	{
	case QEvent::MouseButtonDblClick:
	{
		return ADCBinViewBase::event(pEvent);
	}
	case QEvent::ToolTip:
	{
		QHelpEvent* helpEvent = static_cast<QHelpEvent*>(pEvent);
		QPoint p(helpEvent->pos());

		std::string s;
		if (hasModel())
		{
			ModelLocker lock(this);
			ADCDocPos tipIt(binModel());
			if (setIter(tipIt, p.x(), p.y()))
			{
				MyStream ss;
				binModel()->tipInfoIt(tipIt.line(), tipIt.x(), ss);
				ss.ReadString(s);
			}
		}

		if (!s.empty())
		{
			QToolTip::showText(helpEvent->globalPos(), QString::fromStdString(s));
		}
		else
		{
			QToolTip::hideText();
			pEvent->ignore();
		}

		return true;
	}
	case QEvent::Polish:
	{
		bool bRet(ADCBinViewBase::event(pEvent));
		syncViewportMargins();
		return bRet;
	}
	default:
		break;
	}
	return ADCBinViewBase::event(pEvent);
}

void ADCBinView::mouseDoubleClickEvent(QMouseEvent *e)
{
	if (!mpEdit)
	{
		slotGoToSelected();
		return;
	}

	ModelLocker lock(this);
	ADCTextView::mouseDoubleClickEvent(e);
}

void ADCBinView::slotProperties() 
{
/*	ADDR dwOffset;
	if (!GetSelAddr(dwOffset))
		return;

	FieldPtr pLoc = G DC.GetGlobal2(dwOffset);

	CLabelDlg dlg;
	dlg.m_dwOffset = dwOffset;
	if (pLoc && !pLoc->nameless())
	{
		pLoc->GetName(dlg.m_sName.GetBuffer(NAMELENMAX));
		dlg.m_sName.ReleaseBuffer();
	}

	if (dlg.DoModal() != IDOK)
	{
		SetFocus();
		updateCaret();
		return;
	}

////////////////////////////////////
	if (!pLoc)
	{
		Seg_t *pSeg = G DC.GetOw nerSeg(dlg.m_dwOffset);
		if (!pSeg)
			return;//fix later

		pLoc = new Field_t;
		pLoc->SetOffset(dlg.m_dwOffset);
		if (!GetDocument()->EXECUTE(CMD_ADD, pLoc, pSeg))
		{
			delete pLoc;
			return;
		}
	}
	else
	{
		GetDocument()->EXECUTE(CMD_OFFSET, pLoc, dlg.m_dwOffset);
	}

	GetDocument()->EXECUTE(CMD_NAME, pLoc, dlg.m_sName);

	SetFocus();
	updateCaret();
//!	MAIN.UpdateAllViews(this, UPDATE_GLOBALS|UPDATE_DUMP);*/
	updateContents();
}

void ADCBinView::slotArray() 
{
/*	if (!m_pLoc)
		return;
	if (!m_pLoc->IsD ata())
		return;
	FieldPtr pData = (FieldPtr )m_pLoc;

	CArrayDlg dlg;
	dlg.SetupFrom(pData);
	if (dlg.DoModal() != IDOK)
		return;

	TYPE _t T;
	pData->GetType(T);
	T.m_nArray = dlg.m_dwArraySize;
	GetDocument()->SetType(pData, T);

	updateContents();*/
}


#define PAGE_WIDTH_MAX	80

void ADCBinView::slotHSBarAction(int a)
{
	ModelLocker lock(this);
	switch (a)
	{
	case QAbstractSlider::SliderSingleStepAdd: onCharRight(); break;
	case QAbstractSlider::SliderSingleStepSub: onCharLeft(); break;
	case QAbstractSlider::SliderPageStepAdd: onPageRight(); break;
	case QAbstractSlider::SliderPageStepSub: onPageLeft(); break;
	default:
		break;
	}
}

void ADCBinView::slotVSBarAction(int a)
{
	ModelLocker lock(this);
	switch (a)
	{
	case QAbstractSlider::SliderSingleStepAdd: onNextLine(); break;
	case QAbstractSlider::SliderSingleStepSub: onPrevLine(); break;
	case QAbstractSlider::SliderPageStepAdd: onNextPage(); break;
	case QAbstractSlider::SliderPageStepSub: onPrevPage(); break;
	case QAbstractSlider::SliderToMinimum:
		SetTopAddrAbs(0);
		break;
	case QAbstractSlider::SliderToMaximum:
		SetTopAddrAbs(binModel()->linesNum() - 1);
		break;
	case QAbstractSlider::SliderMove:
	{
		int nPos(qBound(mpVertScrollBar->minimum(), mpVertScrollBar->sliderPosition(), mpVertScrollBar->maximum()));
		SetTopAddrAbs(nPos);
		break;
	}
	default:
		return;
	}
	updateContents();
	//updateCurPosInDoc();
}

int ADCBinView::LineUp(int count)
{
	Q_ASSERT(hasModel());
	return modelData().linesUp(count);
}

int ADCBinView::LineDown(int count)
{
	Q_ASSERT(hasModel());
	return modelData().lineDown(count);
}


int ADCBinView::scrollUpDown(int delta)
{
	int total(0);

	adcui::IBinViewModel *pIModel(hasModel() ? binModel() : nullptr);
	if (pIModel)
	{
		while (delta < 0)
		{
			total -= LineUp(1);
			delta++;
		}

		if (delta > 0)
			total += LineDown(delta);


		int pos(pIModel->lineFromIt(topIt().line()));
		if (pos != mpVertScrollBar->value())
		{
			mpVertScrollBar->blockSignals(true);
			mpVertScrollBar->setValue(pos);
			mpVertScrollBar->blockSignals(false);
		}
	}

	return total;
}

void ADCBinView::editName()
{
	ModelLocker lock(this);
	startInplaceEdit();
}

bool ADCBinView::startInplaceEdit()
{
	if (!ADCBinViewBase::startInplaceEdit())
		return false;
	enableContextActions(false);
	return true;
}

bool ADCBinView::stopInplaceEdit()
{
	if (!ADCBinViewBase::stopInplaceEdit())
		return false;
	enableContextActions(true);
	return true;
}

class MyCustomEvent : public QEvent
{
public:
	MyCustomEvent() : QEvent(MyCustomEvent::type())
	{}

	virtual ~MyCustomEvent()
	{}

	static QEvent::Type type()
	{
		if (customEventType == QEvent::None)
		{
			int generatedType = QEvent::registerEventType();
			customEventType = static_cast<QEvent::Type>(generatedType);
		}
		return customEventType;
	}

//private:
	static QEvent::Type customEventType;
};

QEvent::Type MyCustomEvent::customEventType = QEvent::None;

void ADCBinView::customEvent(QEvent *e)
{
	if (e->type() == MyCustomEvent::customEventType)
	{
		ADCBinViewBase::updateCurPosInDoc();
		//	if (!hasFocus())
		//	return;

		adcui::IBinViewModel *pIModel(hasModel() ? binModel() : nullptr);
		if (!pIModel)
			return;

		pIModel->setBitPosition(curIt().line());

		{
			ModelLocker lock(this);
			ADCStream ss;
			int x;
			if (!pIModel->locusInfo(ss, x))
				return;
			QString s(ss.ReadString());
			emit signalLocusInfo(s, x);
		}
	
	}
}

void ADCBinView::updateCurPosInDoc()
{
	qApp->postEvent(this, new MyCustomEvent());
	return;
}

/*void ADCBinView::slotCancel()
{
	stopInplaceEdit();
}*/

void ADCBinView::slotName()
{
	slotEditCell();
/*	if (m_sSelWord.isEmpty())
		return;

	assert(!m_rcSel.isEmpty());

//	int w0 = charWidth();
//	int h0 = lineHeight();

//	if (ColumnWidth(CLMN_NAMES) <= 0)
	//	return;

	QRect rCol;
	if (cellRectAtPos(m_ptCaret, rCol) < 0)
		return;

	QRect rcSel(m_rcSel);
	rcSel.setRight(rCol.right());
	rcSel.translate(leftMargin()*charWidth(), topMargin()*lineHeight());

	assert(!mpLineEdit);
	mpLineEdit = new QLineEdit(this);
	mpLineEdit->installEventFilter(this);
	mpLineEdit->setFont(font());// mFont);
	//mpLineEdit->setGeometry(x, y, w, h);
	mpLineEdit->setGeometry(rcSel);
//?	mpLineEdit->setFrameShape(QLineEdit::Box);
//	mpLineEdit->setFrameShadow(QFrame::Sunken);
//?	mpLineEdit->setLineWidth(1);
//?	mpLineEdit->setMidLineWidth(0);
	mpLineEdit->setText(m_sSelWord);
	mpLineEdit->show();
	mpLineEdit->set Focus();
	mpLineEdit->selectAll();*/

}

void ADCBinView::slotOffset()
{
	binModel()->postCommandIt(curIt().line(), "makeoff");
}

/*void ADCBinView::createEditor()
{
	if (!m_sSelWord.isEmpty())
	{
		assert(!m_rcSel.isEmpty());
		ADCBinViewBase::createEditor();
		if (mpLineEdit)
		{
			QRect r(mpLineEdit->geometry());
			r.setLeft(leftMargin()*charWidth() + m_rcSel.left());
			mpLineEdit->setGeometry(r);
			mpLineEdit->setText(m_sSelWord);
		}
	}
}*/

void ADCBinView::slotDecompile() 
{
/*	assert(m_pLocSel && m_pLocSel->Is Func());
	MAIN.decompileFunc((TypeProc_t *)m_pLocSel);*/
}

void ADCBinView::slotHideTypes(bool bOn)
{
	adcui::IBinViewModel *pIModel(hasModel() ? binModel() : nullptr);
	if (pIModel)
	{
		pIModel->setValuesOnlyMode(bOn);
		//if (!pIModel->setValuesOnlyMode(true))
			//pIModel->setValuesOnlyMode(false);
		slotGlobalsModified();//dump changed
	}
}

void ADCBinView::slotResolveRefs(bool bOn)
{
	adcui::IBinViewModel *pIModel(hasModel() ? binModel() : nullptr);
	if (pIModel)
	{
		pIModel->setResolveRefsMode(bOn);
		slotGlobalsModified();//dump changed
	}
}

void ADCBinView::slotSetCompact(bool bOn)
{
	if (hasModel())
	{
		ModelLocker lock(this);
		assureCursorVisible();
	}

	adcui::IBinViewModel *pIModel(hasModel() ? binModel() : nullptr);
	if (pIModel)
	{
		pIModel->setCompactMode(bOn);
		//if (!pIModel->setCompactMode(true))
			//pIModel->setCompactMode(false);
	}

	slotGlobalsModified();//dump changed
#if(0)
	ModelLocker lock(this);

	int linesFromTop(m_caretY);

		topIt().copyFrom(&curIt());
		curIt().copyFrom(&topIt());//validate cur!
		//Q_ASSERT(line >= 0);
/*		if (line < 0)
			line = 0;//mRefPos;
		int linesFromTop(line - topMargin());*/
		if (0 < linesFromTop && linesFromTop < pageHeight())
		{
			AutoIter t(lock.pmodel(), topIt().line(), true);
			while (linesFromTop-- > 0)
				--t;//pIModel->backwardIt(t);
		}
		setVScrollBar();

	updateContents();
#endif
}

/*void ADCBinView::slotTogglePackView(bool bOn)
{

}*/

void ADCBinView::slotGoBack() 
{
	ModelLocker lock(this);
	/*if (m_Jumps.isEmpty())
	{
		QApplication::beep();
		return;
	}

	MyJumpInfo &jump(m_Jumps.front());*/

	if (!lock.pmodel())
		return;

	int x;
	int jumpsLeft(lock.model().popJumpIt(topIt().line(), curIt().line(), &x));// jump));
	if (jumpsLeft < 0)
	{
		QApplication::beep();
		return;
	}

	m_caretY = -1;
	setCaret(x, curLine());

	//QPoint caret;
	//if (jump.Read(&caret, sizeof(caret)) == sizeof(caret))
	//	setCaret(caret.x(), caret.y());
	updateContents();

	//m_Jumps.pop_front();

	if (!jumpsLeft)//the last one?
		emit signalJumpInfoStatusChanged(false);

	setVScrollBar();
}

void ADCBinView::updateViewFromLocus()
{
	{//RAII-block
		ModelLocker lock(this);
		if (!binModel()->seekPosIt("$sync", topIt().line()))
			return;
		curIt().copyFrom(&topIt());
		setCaret(-1, 0);
		setVScrollBar();
		updateCurPosInDoc();
	}
	updateContents();
	//setFocus();
}

void ADCBinView::slotFocusTaskTopAtLine(QString task, int linesFromTop)
{
	ModelLocker lock(this);
	if (!lock.pmodel() || !lock.model().seekPosIt(task.toLatin1(), curIt().line()))
		return;

	showCursorAtLine(linesFromTop);
	//setVScrollBar();

	setFocus();
	//fprintf(stdout, "Got focus(2)\n");fflush(stdout);

	updateCurPosInDoc();
	emitSynchronize(false);

	updateContents();
}

void ADCBinView::slotGoToLocation(QString str, int linesFromTop)
{
	adcui::IBinViewModel *pIModel(hasModel() ? binModel() : nullptr);
	if (!pIModel)
		return;

	int jumpsNum(pIModel->pushJumpIt(topIt().line(), curIt().line(), curIt().x()));// jump);

	{//RAII-block
		ModelLocker lock(this);
		if (!pIModel->seekPosIt(str.toLatin1(), topIt().line()))
		{
			pIModel->popJumpIt(adcui::DUMPOS(), adcui::DUMPOS());
			return;
		}

		if (jumpsNum == 1)//was previously empty?
			emit signalJumpInfoStatusChanged(true);

		setCaret(-1, 0);

		modelData().scrollUp(linesFromTop);

		setVScrollBar();
	}
	
	setFocus();//lock is activated here!
	//fprintf(stdout, "Got focus(3)\n");fflush(stdout);
	updateContents();
}

void ADCBinView::slotCurOpChanged()
{
	if (hasModel())
	{
		binModel()->updateSelection();
		updateContents();
	}
}

void ADCBinView::slotDebuggerBreak()
{
	ModelLocker lock(this);
	if (lock.pmodel() && lock.model().seekPosIt("$dbg", curIt().line()))
	{
		//assureCursorVisible();
		centerCursorIfNotVisible();
		curIt().shiftHome();
		updateCurPosInDoc();
	}
	updateContents();
}

void ADCBinView::slotSyncPanesResponce3(int iPosRef, bool)// bSyncMode)
{
	//if (bSyncMode)
	if (!viewport() || viewport()->hasFocus())
		return;//sync initiator

	QString posStr("$sync");
	if (QApplication::keyboardModifiers() & Qt::ControlModifier)
		posStr = "$syncobj";

	ModelLocker lock(this);
	if (lock.pmodel() && lock.model().seekPosIt(posStr.toLatin1().constData(), topIt().line()))
	{
		curIt().copyFrom(&topIt());

		int iPos(0);//global position of the widget
		for (QWidget *p(this); p != nullptr; p = p->parentWidget())
			iPos += p->pos().y();
		if (iPos < iPosRef)
			iPos = iPosRef - iPos;
		else
			iPos -= iPosRef;
		int linesFromTop(iPos / lineHeight() - topMargin());

		if (0 < linesFromTop && linesFromTop < pageHeight())
		{
			modelData().scrollUp(linesFromTop);
		}
		setVScrollBar();
		//mpIModel->invalidate(false);
	}
	updateContents();
}

void ADCBinView::slotGoToSelected()
{
	ModelLocker lock(this);
	if (!lock.pmodel())
		return;

	int jumpsNum(lock.model().pushJumpIt(topIt().line(), curIt().line(), curIt().x()));
	int linesFromTop(curLine());

	//QString s(m_sSelWord.simplified());
	if (!lock.model().seekPosIt("$probe", topIt().line(), curIt().line()))
	{
		lock.model().popJumpIt(adcui::DUMPOS(), adcui::DUMPOS());
		editName();
		return;
	}

	curIt().copyFrom(&topIt());
	showCursorAtLine(linesFromTop);

	if (lock.model().checkJumpTopIt(topIt().line(), curIt().line()) == 1)
	{
		//view did not change, start name editor instead
		lock.model().popJumpIt(adcui::DUMPOS(0), adcui::DUMPOS(0));
		editName();
		return;
	}

	updateCurPosInDoc();

	if (jumpsNum == 1)//was empty?
		emit signalJumpInfoStatusChanged(true);

	//setVScrollBar();
}

void ADCBinView::slotCopySelection()
{
	ModelLocker lock(this);
	ADCBinViewBase::slotCopySelection();
}

void ADCBinView::contextMenuEvent(QContextMenuEvent *e)
{
	if (inplaceEdit() || hasSelection())
	{
		ADCTextView::contextMenuEvent(e);
		return;
	}

	adcui::IBinViewModel *pIModel(hasModel() ? binModel() : nullptr);
	if (!pIModel)
		return;

	QMenu contextMenu;

	QMenu moreMenu(tr("&More..."));
	{
		moreMenu.addAction(mpToggleExportedAction);
		moreMenu.addAction(mpToggleImportedAction);
		moreMenu.addSeparator();
		moreMenu.addAction(mpInstantiateTypeAction);
		moreMenu.addSeparator();
		moreMenu.addAction(mpNewScopeStrucAction);
		//moreMenu.addAction(mpNewScopeUnionAction);
		//moreMenu.addAction(mpNewScopeBitsetAction);
		//moreMenu.addAction(mpNewScopeSegAction);
	}


	//QMenu* subColumns = new QMenu(tr("&Columns"), contextMenu);
	//populateColumnsMenu(subColumns, contextMenu);

	QMenu subScopeView(tr("&Scope to"));
	{
		ModelLocker lock(this);
		QSignalMapper signalMapper(&subScopeView);
		connect(&signalMapper, SIGNAL(mapped(QString)), SLOT(slotScopeView(QString)));
		ADCStream ss;
		pIModel->listObjHierarchyAtIt(curIt().line(), ss);
		QString s;
		while (ss.ReadString(s))
		{
			QAction* a(new QAction(&subScopeView));
			a->setCheckable(true);
			if (s.startsWith("-->"))
			{
				s.remove(0, 3);
				a->setChecked(true);
			}
			QStringList l(s.split("::"));
			a->setText(l.back());
			subScopeView.addAction(a);
			connect(a, SIGNAL(triggered()), &signalMapper, SLOT(map()));
			signalMapper.setMapping(a, s);
		}
	}

	//unsigned uContextId(adcui::CONTEXTID_NULL);
	//emit signalContextIdInq(uContextId);
	//?	if (muContextId == adcui::CXTID_BINARY)
	{
		contextMenu.addAction(mpEditNameAction);
		contextMenu.addAction(mpEnterTypeAction);
		contextMenu.addSeparator()->setText(tr("Action"));
		//contextMenu.addAction(mpBuildTypeAction);
		contextMenu.addAction(mpMakeCodeAction);
		contextMenu.addAction(mpMakeIntAction);
		contextMenu.addAction(mpMakeRealAction);
		contextMenu.addAction(mpMakeBitAction);
		contextMenu.addAction(mpMakeAsciiAction);
		contextMenu.addAction(mpMakeTextAction);
		contextMenu.addAction(mpMakeUnicodeAction);
		contextMenu.addAction(mpMakeFunctionAction);
		contextMenu.addAction(mpSetFunctionEndAction);
		contextMenu.addAction(mpMakeThunkAction);
		contextMenu.addAction(mpMakeUntypedAction);
		contextMenu.addAction(mpMakeCloneAction);
		contextMenu.addAction(mpMakeArrayAction);
		contextMenu.addAction(mpMakeGapAction);
		contextMenu.addAction(mpToggleCollapsedAction);
		contextMenu.addMenu(&moreMenu);
		contextMenu.addSeparator();
		contextMenu.addAction(mpDeleteAction);
		contextMenu.addSeparator();
	}

	//contextMenu.addAction(mpToggleCompactViewAction);
	//contextMenu.addAction(mpToggleValuesOnlyAction);
	//contextMenu.addAction(mpResolveRefsAction);
	//contextMenu.addMenu(subColumns);
	contextMenu.addMenu(&subScopeView);
	//contextMenu.addAction(mpSaveListingAction);

	#ifdef _DEBUG
	contextMenu.addSeparator();
	contextMenu.addAction(mpPrintObjInfoAction);
	#endif

	//?contextMenu.addSeparator();
	//?contextMenu.addAction(mpDecompileFuncAction);

	if (!mExtraActions.empty())
	{
		contextMenu.addSeparator();
		for (QList<QAction*>::iterator i(mExtraActions.begin()); i != mExtraActions.end(); i++)
			contextMenu.addAction(*i);
	}

	/*adcui::Color_t entityId(pIModel->jumpTarget());
	if (entityId == adcui::COLOR_DASM_RVA)
	{
	contextMenu.addSeparator();
	mpJumpTargetAction->setText(tr("Jump RVA Target"));
	contextMenu.addAction(mpJumpTargetAction);
	}*/

	contextMenu.exec(QCursor::pos());
}

void ADCBinView::populateColumnsMenu(QMenu * subColumns, QWidget *parent)
{
	adcui::IBinViewModel* pIModel(hasModel() ? binModel() : nullptr);
	if (!pIModel)
		return;

	QSignalMapper* signalMapper = new QSignalMapper(parent);// contextMenu);
	connect(signalMapper, SIGNAL(mapped(int)), SLOT(slotMap(int)));
	for (int i(0); i < pIModel->colsNum(); i++)// mpIModel->colsNum(); i++)
	{
		if (pIModel->columnWidth(i) != 0)
		{
			QAction* a(new QAction(parent));
			a->setCheckable(true);
			a->setText(pIModel->colName(i));
			subColumns->addAction(a);
			a->setChecked(pIModel->columnWidth(i) > 0);
			connect(a, SIGNAL(triggered()), signalMapper, SLOT(map()));
			signalMapper->setMapping(a, i);
		}
	}
}

/*void ADCBinView::slotJumpTarget()
{
	slotGoToSelected();
}*/

void ADCBinView::slotPrintObjInfo()
{
	emit signalPostCommand("objinfo", true);
}

void ADCBinView::slotForceSynchronize()
{
	emitSynchronize(true);
}

void ADCBinView::slotScopeView(QString s)
{
	binModel()->scopeTo(s.toLatin1());
	updateContents();
}

void ADCBinView::slotMap(int index)
{
	toggleColumn(index);
}

void ADCBinView::slotSaveListing()
{
	adcui::IBinViewModel *pIModel(hasModel() ? binModel() : nullptr);
	uint cols(0);
	for (int i(0); i < pIModel->colsNum(); i++)
		if (pIModel->columnWidth(i) > 0)//visible
			cols |= (1 << i);

	ADCStream ss;
	pIModel->scopeName(ss);
	QString scope(ss.ReadString());

	QString s("listing");
	if (cols != 0)
		s.append(QString(" -cols %1").arg(cols));
	if (pIModel->isCompactMode())
		s.append(" -c");
	if (!scope.isEmpty())
		s.append(QString(" %1").arg(scope));

	emit signalSaveListing(s);
}

void ADCBinView::startInplaceTypeEdit()
{
	emit signalOpenTypeDlg();

#if(0)
	enableContextActions(false);
	//QStringList wordList;
	//wordList << "alpha" << "omega" << "omicron" << "zeta";

	ADCTypeCompleter *typeEdit = new ADCTypeCompleter(this);
	emit signalRequestModel(*typeEdit, QString());//binModel()->moduleName());
	typeEdit->installEventFilter(this);
	typeEdit->setFont(font());
	typeEdit->resize(QSize(160, lineHeight() + 6));
	int w(charWidth());
	int h(lineHeight());
	QPoint cp((leftMargin() + curIt().x()) * w + (w/2), (topMargin() + m_caretY) * h + (h/2));
	typeEdit->move(cp);
	typeEdit->show();
	typeEdit->setFocus();

	typeEdit->complete();
#endif
}

void ADCBinView::stopInplaceTypeEdit()
{
	enableContextActions(true);
	setFocus();
	updateContents();
}

bool ADCBinView::eventFilter(QObject *pObject, QEvent *e)
{
/* 	if (dynamic_cast<ADCTypeCompleter *>(pObject))
	{
		if (e->type() == QEvent::KeyPress)
		{
			int key(((QKeyEvent *)e)->key());
			if (key == Qt::Key_Up || key == Qt::Key_Down)
				return true;
			if (key == Qt::Key_Escape)
			{
				pObject->deleteLater();
				stopInplaceTypeEdit();
				return true;
			}
			if (key == Qt::Key_Enter || key == Qt::Key_Return)
			{
				pObject->deleteLater();
				stopInplaceTypeEdit();
				return true;
			}
		}
		else if (e->type() == QEvent::FocusOut)
		{
			pObject->deleteLater();
			stopInplaceTypeEdit();
			return true;
		}
	}*/

	return ADCBinViewBase::eventFilter(pObject, e);
}

void ADCBinView::slotEnterType()
{
	startInplaceTypeEdit();
}

void ADCBinView::slotBuildType()
{
	/*QString sObj;
	QString sArray;
	QStringList l;
	//int iArray(0);
	{
		ModelLocker lock(this);

		ADCStream ss;
		//	ROWID rowid = mpIModel->rowIdIt(mSelIt);
		//	emit signalCallCommand(fmt("typeslist %#X", rowid), ss);

		if (!binModel()->listTypesAtIt(curIt().line(), ss))
			return;

		ss.ReadString(sObj, "\n");//object's name
		ss.ReadString(sArray, "\n");
		iArray = sArray.toInt();

		QString s;
		while (ss.ReadString(s, "\n"))
		{
			//while (s.endsWith(","))
			//s.truncate(s.length()-1);
			l.append(s);
		}
	}*/

	//ADCTypesDlg dlg(this);

	QString sModule(binModel()->moduleName());

	//QString s(sModule + MODULE_SEP + TYPES_EXT + "/" + sType);

	//QComboBox *pcbx(new QComboBox(this));
	//pcbx->move(mapFromGlobal(QCursor::pos()));
	//pcbx->show();

	emit signalPickModuleType(sModule);

//	emit signalRequestModel(dlg.view(), QString());//from context
	//dlg.view().populate();

/*	dlg.setField(sObj);
	dlg.setContents(l);
	dlg.setArray(iArray);
	if (dlg.exec() == QDialog::Accepted)
	{
		QString tname(dlg.getSelected());
		int n(dlg.array());
		if (n > 0)
			tname = QString("%1[%2]").arg(tname).arg(n);
		emit signalPostCommand(QString("makeobj -t %1").arg(tname));
	}*/
}

void ADCBinView::slotDecompileFunction()
{
	binModel()->postCommandIt(curIt().line(), "decompile");
}

void ADCBinView::slotRedraw()
{
	updateContents();
}

void ADCBinView::slotAnalysisStarted()
{
}

void ADCBinView::slotSetFont(const QFont &f)
{
	setFont(f);
}

/*void ADCBinView::slotToggleSyncMode(bool bOn)
{
//?	if (bOn != m_bSyncMode)
	{
//?		m_bSyncMode = bOn;
		adcui::IBinViewModel *pIModel(hasModel() ? binModel() : nullptr);
		if (pIModel && pIModel->setSyncMode(bOn))
		{
			if (bOn && hasFocus())//isActiveWindow())
			{
				ModelLocker lock(this);
				emitSynchronize(false);
			}
			//slotSyncPanesResponce(-1);//?
			updateContents();
		}
	}
}*/













/////////////////////////////////////////////////////////////////

ADCBinWinToolbar::ADCBinWinToolbar(QWidget *parent, ADCBinView* pView)
	: ADCBinWinToolbarBase(parent)
{
	//setAutoFillBackground(true);
	//setBackgroundRole(QPalette::Window);
//	setFrameStyle(QFrame::Panel | QFrame::Raised);

	//mpPackViewTbtn = new QToolButton(this);
	//mpPackViewTbtn->setAutoRaise(true);

	//ADCBinWin* pWin(dynamic_cast<ADCBinWin*>(parent));
	//ADCBinView* pView(pWin->view());

	QMenu* pPopup1 = new QMenu(this);
	pPopup1->addAction(pView->mpToggleCompactViewAction);
	pPopup1->addAction(pView->mpToggleValuesOnlyAction);
	pPopup1->addAction(pView->mpResolveRefsAction);
	pPopup1->addSeparator();
	pPopup1->addAction(pView->mpSaveListingAction);

	QMenu* pPopup2 = new QMenu(this);
	pView->populateColumnsMenu(pPopup2, this);

	//mpResolveRefsTbtn = new QToolButton(this);
	//mpResolveRefsTbtn->setAutoRaise(true);

	mpGoBackTbtn = addButton(pView->mpGoBackAction);

	mpGotoCbx = addComboBox();
	mpGotoCbx->setEditable(true);
	mpGotoCbx->setSizePolicy(QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed));
	mpGotoCbx->setMinimumSize(QSize(180, 0));
	mpGotoCbx->setDuplicatesEnabled(false);
	mpGotoCbx->setInsertPolicy(QComboBox::InsertAtTop);
	//mpGotoCbx->lineEdit()->installEventFilter(this);
	connect(mpGotoCbx->lineEdit(), SIGNAL(returnPressed()), SLOT(slotGo2Location()));
	mpGotoCbx->setToolTip(tr("Go to location"));
	//mpGotoCbx->setFrameStyle(QFrame::Panel | QFrame::Raised);

	mpGotoEntryTbtn = addButton(pView->mpGotoEntryAction);
	connect(mpGotoEntryTbtn, SIGNAL(clicked()), SLOT(slotGo2Location()));

	connect(this, SIGNAL(signalGoToLocation(QString, int)), pView, SLOT(slotGoToLocation(QString, int)));

	addSeparator();

	mpColumnsTbtn = addButton(QIcon(":columns.png"), tr("Columns"));
	mpColumnsTbtn->setPopupMode(QToolButton::InstantPopup);
	mpColumnsTbtn->setMenu(pPopup2);

	addSeparator();

	mpOptionsTbtn = addButton(QIcon(":dc_view_16.png"), tr("View Options"));
	mpOptionsTbtn->setPopupMode(QToolButton::InstantPopup);
	mpOptionsTbtn->setMenu(pPopup1);

	addStretch();
}

void ADCBinWinToolbar::setFont(const QFont& f)
{
	ADCBinWinToolbarBase::setFont(f);
	mpGotoCbx->setFont(f);
	mpGotoCbx->lineEdit()->setFont(f);
	//?updateGeometry();
}

void ADCBinWinToolbar::slotGo2Location()
{
	QString s(mpGotoCbx->currentText().simplified());
	emit signalGoToLocation(s, 0);
	if (!s.isEmpty())
		mpGotoCbx->clearEditText();
}

bool ADCBinWinToolbar::eventFilter( QObject * pObject, QEvent * e )
{
	if (e->type() == QEvent::KeyPress)
	{
		if (pObject == mpGotoCbx->lineEdit())
		{
			int key = ((QKeyEvent *)e)->key();
			if (key == Qt::Key_Enter || key == Qt::Key_Return)
			{
				slotGo2Location();
				return true;	// prevent default button from receiving event
			}
		}
	}

	return ADCBinWinToolbarBase::eventFilter(pObject, e);
}

void ADCBinWinToolbar::enableGoBackButton(bool bNonEmpty)
{
	mpGoBackTbtn->setEnabled(bNonEmpty);
}

void ADCBinWinToolbar::focusLocationBox()
{
	mpGotoCbx->setFocus();
}

void ADCBinWinToolbar::enableControls(bool bEnable)
{
	mpColumnsTbtn->setEnabled(bEnable);
	mpOptionsTbtn->setEnabled(bEnable);
	mpGotoEntryTbtn->setEnabled(bEnable);
	mpGoBackTbtn->setEnabled(bEnable);
	//mpGotoLed->setEnabled(bEnable);
	mpGotoCbx->setEnabled(bEnable);
}

//////////////////////////////////////////////////

/*ADCBinFuncDefBar::ADCBinFuncDefBar(QWidget *parent)
: QFrame(parent)
{
	QLabel *pLabel(new QLabel(this));
	pLabel->setText(tr("Define a function:"));

	QLineEdit *pLineEdit(new QLineEdit(this));

	//QSpacerItem * pSpacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);

	QHBoxLayout * hbox = new QHBoxLayout;
	hbox->addWidget(pLabel);
	hbox->addWidget(pLineEdit);
	//hbox->addItem(pSpacer);
	//hbox->addWidget(pRefreshTbtn);
	hbox->setContentsMargins(0, 0, 0, 0);

	setLayout(hbox);
}*/








//////////////////////////////////////////////////

ADCBinWin::ADCBinWin(QWidget * parent, const char *name)
: ADCBinWinBase(parent),
mpView(nullptr),
mpToolbar(nullptr),
mpModelData(nullptr)
{
	setObjectName(name);

	//setAutoFillBackground(false);
	//setBackgroundRole(QPalette::Button);
	//setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	//mpView->setFrameShadow(QFrame::Sunken);
	//setLineWidth(10);
	//setMidLineWidth(10);

	slotJumpStackStatusChanged(false);
}

ADCBinWin::~ADCBinWin()
{
}

void ADCBinWin::createView(ADCModelData *pModelData)
{
	mpView = new ADCBinView(this);
	mpView->setModelData(pModelData);

	//mpView->show();
	connect(this, SIGNAL(signalGoToLocation(QString, int)), mpView, SLOT(slotGoToLocation(QString, int)));
	connect(this, SIGNAL(signalAnalysisStarted()), mpView, SLOT(slotAnalysisStarted()));
	//connect(this, SIGNAL(signalSyncPanesResponce2(int, bool)), mpView, SLOT(slotSyncPanesResponce(int, bool)));

	//mpView->setAutoFillBackground(true);
	//mpView->setBackgroundRole(QPalette::Window);
	//mpView->setFrameStyle(QFrame::Panel | QFrame::Sunken);

	setFocusProxy(mpView);
	connect(mpView, SIGNAL(signalGoToLocation()), SLOT(slotShowLocationBox()));
	//connect( this, SIGNAL(sigGo2EntryPoint()), mpView, SLOT(slotGo2EntryPoint()) );
	connect(mpView, SIGNAL(signalSynchronize(int, bool)), SIGNAL(signalSyncRequestAtLine(int, bool)));
	connect(mpView, SIGNAL(signalLocusInfo(QString, int)), SIGNAL(signalLocusInfo(QString, int)));
	connect(mpView, SIGNAL(signalRefreshBinaryDump(int)), SIGNAL(signalRefreshBinaryDump(int)));
	connect(mpView, SIGNAL(signalPostCommand(const QString &, bool)), SIGNAL(signalPostCommand(const QString &, bool)));
	//connect(mpView, SIGNAL(signalCallCommand(const QString &, ADCStream &)), SIGNAL(signalCallCommand(const QString &, ADCStream &)));
	connect(mpView, SIGNAL(signalJumpInfoStatusChanged(bool)), SLOT(slotJumpStackStatusChanged(bool)));
	connect(mpView, SIGNAL(signalSaveListing(QString)), SIGNAL(signalSaveListing(QString)));
	//connect(mpView, SIGNAL(signalContextIdInq(uint &)), SIGNAL(signalContextIdInq(uint &)));
	//connect(mpView, SIGNAL(signalRequestModel(ADCTypesView &, QString)), SIGNAL(signalRequestModel(ADCTypesView &, QString)));
	connect(mpView, SIGNAL(signalPickModuleType(QString)), SIGNAL(signalPickModuleType(QString)));
	connect(mpView, SIGNAL(signalRequestModel(ADCTypeCompleter &, QString)), SIGNAL(signalRequestModel(ADCTypeCompleter &, QString)));
	connect(mpView, SIGNAL(signalReleaseModel(QString)), SIGNAL(signalReleaseModel(QString)));
	connect(mpView, SIGNAL(signalOpenTypeDlg()), SIGNAL(signalOpenTypeDlg()));
	connect(this, SIGNAL(signalGlobalsModified()), mpView, SLOT(slotGlobalsModified()));
	connect(this, SIGNAL(signalLocusAdjusted()), mpView, SLOT(slotLocusAdjusted()));

	mpToolbar = new ADCBinWinToolbar(this, mpView);
	connect(mpToolbar, SIGNAL(signalGoToLocation(QString, int)), SIGNAL(signalGoToLocation(QString, int)));


	QVBoxLayout * vbox(new QVBoxLayout);
	//vbox->addLayout(hbox);
	vbox->addWidget(mpToolbar);
	vbox->addWidget(mpView);
	//vbox->addWidget(mpFuncDefBar);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->setSpacing(0);

	setLayout(vbox);
}

static void clearLayout(QLayout* layout, bool deleteWidgets = true)
{
	while (QLayoutItem* item = layout->takeAt(0))
	{
		if (deleteWidgets)
		{
			if (QWidget* widget = item->widget())
				widget->deleteLater();
		}
		if (QLayout* childLayout = item->layout())
			clearLayout(childLayout, deleteWidgets);
		delete item;
	}
}

ADCModelData *ADCBinWin::setModel(ADCModelDataMap &rm, adcui::IBinViewModel *pIModel)
{
	Q_ASSERT(pIModel);
	ADCDocPos *pCur(new ADCDocTablePos(pIModel));
	mpModelData = new ADCModelData(rm, pIModel, nullptr, pCur);//add ref
	createView(mpModelData);
	return mpModelData;
}

void ADCBinWin::setModelData(ADCModelData *pModelData)
{
	mpModelData = pModelData;
	mpModelData->AddRef();
	createView(mpModelData);
}

void ADCBinWin::takeModel()
{
	Q_ASSERT(mpModelData);
	mpModelData->Release();
	mpModelData = nullptr;
	Q_ASSERT(mpView);
	mpView->setModelData(nullptr);
	clearLayout(layout(), true);
	delete layout();
	delete mpView;
	mpView = nullptr;
	mpToolbar = nullptr;
}

void ADCBinWin::locatePosition(QString s)
{
	emit signalGoToLocation(s, 10);
}

void ADCBinWin::closeEvent(QCloseEvent *e)
{
	takeModel();
	ADCBinWinBase::closeEvent(e);
}

/*void ADCBinWin::slotProjectNew()
{
	emit signalRequestModel(*this, QString());
	if (mpView)
		mpView->slotReset();
}

void ADCBinWin::slotReset()
{
	emit signalRequestModel(*this, QString());
	mpView->slotReset();
}*/

/*void ADCBinWin::slotSync PanesResponce(int, bool)
{
}*/

void ADCBinWin::updateOutputFont(const QFont &f)
{
	if (mpView)
		mpView->setFont(f);
	if (mpToolbar)
		mpToolbar->setFont(f);
}

void ADCBinWin::slotAboutToClose()
{
	//takeModel();
}

void ADCBinWin::slotUpdateViewFromLocus()
{
	if (!isVisible())
		return;
	mpView->updateViewFromLocus();
}

void ADCBinWin::slotJumpStackStatusChanged(bool bNonEmpty)
{
	if (mpToolbar)
		mpToolbar->enableGoBackButton(bNonEmpty);
}

void ADCBinWin::slotShowLocationBox()
{
	if (mpToolbar)
		mpToolbar->focusLocationBox();
}

/*void ADCBinWin::slotHighlightLocationAt(QString sAtAddr, QString sRefAddr, QString sFunc, int)
{
	emit signalGoToLocation(sRefAddr, 20);
	emit signalHighlightLocationAt(sRefAddr, sFunc);
}*/



////////////////////
/*#include <qpushbutton.h>

ADCArrayDlg::ADCArrayDlg(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle(tr("Make Array"));
	QGridLayout* pBaseLayout = new QGridLayout(this);

	QHBoxLayout *pHBox = new QHBoxLayout;
	QSpacerItem *pSpacer1 = new QSpacerItem(60, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	pHBox->addItem(pSpacer1);

	QPushButton *pOKPBtn = new QPushButton(tr("OK"), this);
	pHBox->addWidget(pOKPBtn);

	QPushButton *pCancelPBtn = new QPushButton(tr("Cancel"), this);
	pHBox->addWidget(pCancelPBtn);

	QSpacerItem *pSpacer2 = new QSpacerItem( 60, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	pHBox->addItem(pSpacer2);

	pBaseLayout->addLayout(pHBox, 1, 0);
}*/






