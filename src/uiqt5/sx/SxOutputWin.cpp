#include <QtCore/QTextStream>
#include <QtCore/QRegExp>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QFont>
#include <QApplication>
#include <QTextEdit>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QLayout>
#include <QLabel>
#include <QComboBox>
#include <QToolButton>
#include <QToolTip>
#include <QSplitter>
#include <QScrollBar>

#include "SxOutputWin.h"
#include "SxCommandWin.h"
#include "SxCommandLine.h"

SxFindTextDlg* SeditexTextEdit::spFindTextDlg = nullptr;

SeditexTextEdit::MyColorTags::MyColorTags()
{
	insert("USUAL", QPair<QString, QString>("<font color=#000000>", "</font>"));
	insert("WARNING", QPair<QString, QString>("<font color=#9600F6>", "</font>"));
	insert("ERROR", QPair<QString, QString>("<font color=#E91428>", "</font>"));
	insert("NOTE", QPair<QString, QString>("<font color=#1A0FFF>", "</font>"));
	insert("ERR_WARNING", QPair<QString, QString>("<i><font color=#9600F6>", "</font></i>"));
	insert("ERR_FATAL", QPair<QString, QString>("<u><b><font color=#F60000>", "</font></b></u>"));
	insert("ERR_PANIC", QPair<QString, QString>("<font color=#E21E1E>", "</font>"));
	insert("ERR_INFO", QPair<QString, QString>("<font color=#14A228>", "</font>"));
	insert("ERR_ERROR", QPair<QString, QString>("<u><b><font color=#ED4625>", "</font></b></u>"));
	insert("ERR_INTERNAL", QPair<QString, QString>("<font color=#CA2A0A>", "</font>"));
	insert("ERR_EXTERNAL", QPair<QString, QString>("<font color=#CA2A0A>", "</font>"));
	insert("ERR_SYNTAX", QPair<QString, QString>("<font color=#FF7A0F>", "</font>"));
	insert("ERR_MEMORY", QPair<QString, QString>("<font color=#FF3C0F>", "</font>"));

	insert("darkgrey", QPair<QString, QString>("<font color=darkgrey>", "</font>"));
	insert("red", QPair<QString, QString>("<font color=red>", "</font>"));
	insert("darkred", QPair<QString, QString>("<font color=darkred>", "</font>"));
	insert("blue", QPair<QString, QString>("<font color=blue>", "</font>"));
	insert("darkblue", QPair<QString, QString>("<font color=darkblue>", "</font>"));
	insert("darkgreen", QPair<QString, QString>("<font color=darkgreen>", "</font>"));

	insert("bold", QPair<QString, QString>("<b>", "</b>"));
	insert("underline", QPair<QString, QString>("<u>", "</u>"));
	insert("italic", QPair<QString, QString>("<i>", "</i>"));

	updateTagList();
}

void SeditexTextEdit::MyColorTags::updateTagList()
{
	QMap<QString, int> m;
	for (Iterator it(begin()); it != end(); it++)
	{
		QPair<QString, QString>& tags(*it);
		m.insert(tags.first, 0);
		m.insert(tags.second, 1);
	}
	for (QMap<QString, int>::Iterator itm(m.begin()); itm != m.end(); itm++)
		lAllTags.append(itm.key());
}

void SeditexTextEdit::MyColorTags::toStringList(QStringList& l)
{
	for (MyColorTags::Iterator it(begin()); it != end(); it++)
	{
		QString id(it.key());
		QString tag1(it.value().first);
		QString tag2(it.value().second);
		l.append(QString("%1\t%2\t%3").arg(id).arg(tag1).arg(tag2));
	}
}

SeditexTextEdit::MyColorTags SeditexTextEdit::gColorTags;

const char* SeditexTextEdit::GetColorTag(const char* id, bool cls)
{
	MyColorTags::Iterator it(gColorTags.find(QString(id)));
	if (it != gColorTags.end())
	{
		if (!cls)
			return it.value().first.toLatin1();
		return it.value().second.toLatin1();
	}
	return "";
}

QStringList& SeditexTextEdit::GetFontColorTagList()
{
	return gColorTags.tagList();
}

void SeditexTextEdit::WriteColorTags(QStringList& l)
{
	gColorTags.toStringList(l);
}

static int gDocId = 0;


SeditexTextEdit::FindTextObject::FindTextObject(SeditexTextEdit* parent)
	: SxFindTextDlg::ITarget(parent),
	mpTarget(parent)
{
}

bool SeditexTextEdit::FindTextObject::findIt(const QString& text, bool cs, bool wo, bool rev, bool reset) const
{
	QTextDocument::FindFlags options = 0;
	if (cs)
		options |= QTextDocument::FindCaseSensitively;
	if (wo)
		options |= QTextDocument::FindWholeWords;
	if (rev)
		options |= QTextDocument::FindBackward;
	if (reset)
		options |= QTextDocument::FindWholeWords;

	return mpTarget->find(text, options);
}

SeditexTextEdit::SeditexTextEdit(QWidget* pparent, const char* /*pname*/)
	: QTextEdit(pparent),
	mbAutoScroll(false),
	mbClearable(false),
	mbSaveToFileSupport(false),
	mbCheckChevrons(false),
	mLineRef(-1),
	mOutputId(gDocId++)
{
	// use optimized mode
	// - different modes have different user interactions
	setReadOnly(true);
	setLineWrapMode(QTextEdit::NoWrap);
	setAcceptRichText(false);

	/*setTextFormat(Qt::RichText);*/
	//setGeometry( 0, 0, 600, 400 );

	// set user interaction
	// - do NOT set read only otherwise the cursor is never visible
//?	setFocusPolicy(QTextEdit::StrongFocus);
//?	setUndoRedoEnabled(false);
//?	setWordWrap(NoWrap);//QTextEdit::FixedColumnWidth);
//?	setWrapColumnOrWidth(110);
	//setAcceptDrops(true);
//?	setVScrollBarMode(AlwaysOn);

	connect(this, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));
}

SeditexTextEdit::~SeditexTextEdit()
{
	//	if ( mppExtPtr )
	//		*mppExtPtr = nullptr;
}

QSize SeditexTextEdit::sizeHint() const
{
	return QSize(600, 400);
}

void SeditexTextEdit::contentsDragEnterEvent(QDragEnterEvent* pevent)
{
	dragEnterEvent(pevent);
}

void SeditexTextEdit::contentsDragMoveEvent(QDragMoveEvent* pevent)
{
	dragMoveEvent(pevent);
}

void SeditexTextEdit::contentsDropEvent(QDropEvent* pevent)
{
	dropEvent(pevent);
}

void SeditexTextEdit::dragEnterEvent(QDragEnterEvent* /*pevent*/)
{
	/*?	if(Q3UriDrag::canDecode(pevent))
			pevent->acceptAction();
		else
			pevent->ignore();*/
}

void SeditexTextEdit::dragMoveEvent(QDragMoveEvent* /*pevent*/)
{
	/*?	if(Q3UriDrag::canDecode(pevent))
			pevent->acceptAction();
		else
			pevent->ignore();*/
}

void SeditexTextEdit::dropEvent(QDropEvent* pevent)
{
	qApp->sendEvent(window(), pevent);
}

void SeditexTextEdit::keyPressEvent(QKeyEvent* pevent)
{
	// there are the only allowable key implementations (all others are ignored)
	// - this is necessary so the control displays it's cursor but does not allow edits
	// - some key implementations are NOT safe

	int nkey = pevent->key();
	Qt::KeyboardModifiers nkey_modifier = pevent->modifiers();

	// navigation keys
	// - combination with ShiftButton (e.g. Key_Up, etc.) results in crash
	if ((nkey == Qt::Key_Up
		|| nkey == Qt::Key_Down
		|| nkey == Qt::Key_Left
		|| nkey == Qt::Key_Right

		|| nkey == Qt::Key_Home
		|| nkey == Qt::Key_End

		|| nkey == Qt::Key_PageUp
		|| nkey == Qt::Key_PageDown)

		&& nkey_modifier ^ Qt::ShiftModifier)
	{
		QTextEdit::keyPressEvent(pevent);
	}

	// other
	// - select all
	else if ((nkey == Qt::Key_A) && (nkey_modifier & Qt::ControlModifier))
		QTextEdit::keyPressEvent(pevent);

	// - copy
	else if ((nkey == Qt::Key_C) && (nkey_modifier & Qt::ControlModifier))
		QTextEdit::keyPressEvent(pevent);

	// - copy key on Sun keyboards
	else if (nkey == Qt::Key_F16)
		QTextEdit::keyPressEvent(pevent);

	else if ((nkey == Qt::Key_F) && (nkey_modifier & Qt::ControlModifier))
		slotFindText();

	/*else if ((nkey == Qt::Key_Z) && (nkey_modifier & Qt::ControlModifier))
	{
		append("test1");
		append("test2");
		append("test3");
		QTextEdit::keyPressEvent(pevent);
	}*/

	// - ignore key (combination)
	else
		pevent->ignore();
}

void SeditexTextEdit::paste()
{
	// NOT SUPPORTED
}

void SeditexTextEdit::focusInEvent(QFocusEvent* pevent)
{
	QTextEdit::focusInEvent(pevent);

	if (pevent->gotFocus())
	{
		// user has selected control, so stop autoscrolling
		mbAutoScroll = false;
	}
}

void SeditexTextEdit::focusOutEvent(QFocusEvent* pevent)
{
	// determine whether to re-enable autoscroll
	if (pevent->lostFocus())
	{
		QScrollBar* pscroll_bar = verticalScrollBar();
		mbAutoScroll = !textCursor().hasSelection()
			&& pscroll_bar->value() == pscroll_bar->maximum();
	}
}

// Override the default context menu with our own custom version so that
// we don't have a seemingly impossible to disable (cleanly) paste menu item
void SeditexTextEdit::contextMenuEvent(QContextMenuEvent* event)
//QMenu *SeditexTextEdit::createPopupMenu(const QPoint & pos)
{
#if(0)
	QMenu* popup = QTextEdit::createPopupMenu(pos);
#else
	//Q_UNUSED(pos)
	const bool isEmptyDocument = document()->isEmpty();//? (length() == 0);

	QMenu* popup = new QMenu(this);//, "seditex_output_popup" );

	QAction* pCopyAction(new QAction(tr("&Copy"), popup));
	pCopyAction->setShortcut(Qt::CTRL + Qt::Key_C);
	pCopyAction->setEnabled(textCursor().hasSelection());
	connect(pCopyAction, SIGNAL(triggered()), SLOT(slotCopy()));

	popup->addAction(pCopyAction);

	QAction* pClearAction(new QAction(tr("Clear"), popup));
	pClearAction->setEnabled(!isEmptyDocument);
	connect(pClearAction, SIGNAL(triggered()), SLOT(slotClear()));

	popup->addAction(pClearAction);

	if (mbSaveToFileSupport)
	{
		QAction* pSaveAsAction(new QAction(tr("Save to File..."), popup));
		pSaveAsAction->setEnabled(!isEmptyDocument);
		connect(pSaveAsAction, SIGNAL(triggered()), SLOT(slotSaveAs()));
	}

	popup->addSeparator();

	QAction* pSelectAllAction(new QAction(tr("Select All"), popup));
	pSelectAllAction->setShortcut(Qt::CTRL + Qt::Key_A);
	pSelectAllAction->setEnabled(!isEmptyDocument);
	connect(pSelectAllAction, SIGNAL(triggered()), SLOT(slotSelectAll()));

	popup->addAction(pSelectAllAction);

	emit signalCreatePopupMenuRequested(popup);
#endif

	popup->exec(event->globalPos());
	delete popup;
	//return popup;
}

void SeditexTextEdit::append(const QString& rs)
{
	if (!rs.isEmpty())
	{
		if (0)
			if (mbCheckChevrons)
			{
				// QTextEdit removes chevron (i.e. '<', '>') pairs as they are considered
				// rich text tags, so replace them with textual representation (i.e. '&lt;'
				// and '&gt;') which will displayed as chevrons
				if (rs.contains('<') && rs.contains('>'))
				{
					QString s(rs);
					checkChevrons(s);
					QTextEdit::append(s);
					return;
				}
			}

		QTextEdit::append(rs);
	}
}

void SeditexTextEdit::slotCopy()
{
	QTextEdit::copy();
}

void SeditexTextEdit::slotClear()
{
	QTextEdit::clear();

	emit signalCleared();

	//    originalCoreOutput="";
	// reset flag to auto-scroll
	mbAutoScroll = true;
}

void SeditexTextEdit::slotFlush()
{
	//?	repaintContents(/*erase background*/ true);
}

void SeditexTextEdit::slotSelectAll()
{
	QTextEdit::selectAll();
}

void SeditexTextEdit::slotDeselect()
{
	//?	QTextEdit::selectAll(false );
	QTextCursor cursor = textCursor();
	cursor.clearSelection();
	setTextCursor(cursor);
}

SxFindTextDlg* SeditexTextEdit::getFindDlg(QWidget* pOwner)
{
	if (!spFindTextDlg)
		spFindTextDlg = new SxFindTextDlg(pOwner);
	return spFindTextDlg;
}

void SeditexTextEdit::destroyFindDlg()
{
	if (spFindTextDlg)
	{
		delete spFindTextDlg;
		spFindTextDlg = nullptr;
	}
}

void SeditexTextEdit::slotFindText()
{
	SxFindTextDlg* pDlg = getFindDlg(this);
	if (pDlg)
	{
		pDlg->setTed(new FindTextObject(this));
		pDlg->setFont(font());
		const QPoint pc(mapToGlobal(rect().center()));
		pDlg->move(pc.x() - pDlg->width() / 2, pc.y() - pDlg->height() / 2);
		pDlg->show();
	}
}

void SeditexTextEdit::slotUpdateContents(const QString& s)
{
	//?	scrollToBottom();
	mbAutoScroll = false;
	append(s);
}

void SeditexTextEdit::setFont(const QFont& rfont)
{
	QTextEdit::setFont(rfont);

	// This is needed to force the scrollbars to update whenever the font
	// is changed. Has the side effect of re-enabling the auto scrolling, but
	// since the font doesn't get changed very often we can live with that.
//?	scrollToBottom();
	mbAutoScroll = true;
}

bool SeditexTextEdit::getTextHtml(QString& htmlout) const
{
	htmlout = document()->toPlainText();
	htmlout.replace("\n", "<br>");
	htmlout.replace("  ", "&nbsp;&nbsp;");
	htmlout.replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
	return 1;
}

void SeditexTextEdit::slotSelectionChanged()
{
	bool selection = textCursor().hasSelection();
	emit enableCopy(selection);
}

void SeditexTextEdit::EmitSignals()
{
	emit enableSaveAs(true);
	slotSelectionChanged();
	if (mbClearable)
		emit enableClear(true);
	emit enableFindText(true);
//?	emit enableFind(this);
}

void SeditexTextEdit::slotPrint(QSilPageSetup*)
{
	//nothing know about printer object - ask the app to print myself
	emit signalPrint(this);
}

/*
*  The function selects combinations look like a tag (bordered by "<" and ">")
*  And checks if these combination are valid formatting tags
*  The valid formatting tags are:
*  <font..>, </font>, <b>, </b>, <i>, </i>, <u>, </u>
*  Otherwise, if the tag is not valid the function replace ">" with "&gt;"
*  and ""
*/
QString& SeditexTextEdit::checkChevrons(QString& qsSource)
{
	QRegExp rxCommonTagExp("</?[^(>|<)]+>", Qt::CaseInsensitive, QRegExp::RegExp);//FALSE, FALSE );
	QRegExp rxValid("</?(font[^>]*|b|i|u)>", Qt::CaseInsensitive, QRegExp::RegExp);//FALSE, FALSE );
	int outerPosition = 0;
	int innerPosition = 0;
	int outerLength = 0;
	//int innerLength = 0;
	int oldPosition = 0;
	QString leftString, midString, rightString;
	int leftBorder = 0, rightBorder = 0;
	int oldLength = 0, newLength = 0;

	/* Set lazy flavor */
	rxCommonTagExp.setMinimal(true);
	/* Look for a tag - either valid or invalid tag */
	while ((outerPosition = rxCommonTagExp.indexIn(qsSource, outerPosition, QRegExp::CaretWontMatch)) != -1)
	{
		/* We have found a structure looks like a tag */
		/* Checking if the tag is valid (<font..>, </font>, <b>, </b>, <i>, </i>, <u>, </u>) or not */
		outerLength = rxCommonTagExp.matchedLength();
		if (-1 != outerLength)
		{
			innerPosition = 0;
			/* Copy the found substring */
			QString qsInner = qsSource.mid(outerPosition, outerLength);
			innerPosition = rxValid.indexIn(qsInner, innerPosition, QRegExp::CaretWontMatch);
			if (innerPosition == -1)
			{
				/* The tag is not valid. Replacing chevrons */
				/* Replacing > chevron */
				/* qsSource.replace( outerPosition + outerLength - 1, 1, "&gt;" ); */
				/* Replacing < chevron */
				/* qsSource.replace( outerPosition, 1, "&lt;" ); */
				leftBorder = oldPosition;
				rightBorder = outerPosition + outerLength;
			}
			else
			{
				/* We have found a valid tag */
				leftBorder = oldPosition;
				rightBorder = outerPosition;
			}
			if (leftBorder < rightBorder)
			{
				/* Split the source string into 3 parts */
				/* Center part - we need to replace all chevrons here */
				midString = qsSource.mid(leftBorder, rightBorder - leftBorder);
				/* Check if the middle part contains any chevron */
				if (midString.contains('<') || midString.contains('>'))
				{
					/* Left part - the constant part */
					leftString = qsSource.left(leftBorder);
					/* Right part - the constant part */
					if ((int)qsSource.length() < rightBorder)
					{
						rightString = "";
					}
					else
					{
						rightString = qsSource.right(qsSource.length() - rightBorder);
					}
					/* Save the old length */
					oldLength = midString.length();
					/* Replacing > chevron */
					midString.replace(QChar('>'), "&gt;");
					/* Replacing < chevron */
					midString.replace(QChar('<'), "&lt;");
					qsSource = leftString + midString + rightString;
					/* Get the new length */
					newLength = midString.length();
					/* Update the old starting point */
					outerPosition += newLength - oldLength;
				}
			}
		}
		/* Setting the new starting point */
		outerPosition += outerLength;
		oldPosition = outerPosition;
	}
	if (0 == oldPosition)
	{
		/* We haven't found any structure looks like a tag */
		/* Replacing > chevron */
		qsSource.replace(QChar('>'), "&gt;");
		/* Replacing < chevron */
		qsSource.replace(QChar('<'), "&lt;");
	}
	else
	{
		/* Left part - the constant part */
		leftString = qsSource.left(oldPosition);
		if ((int)qsSource.length() < oldPosition)
		{
			rightString = "";
		}
		else
		{
			rightString = qsSource.right(qsSource.length() - oldPosition);
		}
		/* We try to replace all chevrons in the rest of the string */
		/* Replacing > chevron */
		rightString.replace(QChar('>'), "&gt;");
		/* Replacing < chevron */
		rightString.replace(QChar('<'), "&lt;");
		qsSource = leftString + rightString;
	}

	return qsSource;
}

void SeditexTextEdit::slotAppendText(const QString& str)
{
	// do not change currently selected text
/*	int ncursor_para_from;
	int ncursor_index_from;
	int ncursor_para_to;
	int ncursor_index_to;

	bool bSelected = hasSelectedText();
	if (bSelected)
		getSelection(&ncursor_para_from, &ncursor_index_from, &ncursor_para_to, &ncursor_index_to);

	blockSignals(true);

	// disable internal repaints to reduce annoying flickering
	setUpdatesEnabled(false);*/

	append(str);

	// reapply exising selection
/*	if (bSelected)
		setSelection(ncursor_para_from, ncursor_index_from, ncursor_para_to, ncursor_index_to);

	setUpdatesEnabled(true);
	updateContents();
	scrollToBottom();

	blockSignals(false);*/
}

void SeditexTextEdit::slotSaveToFile(const QString& rfile, bool bappend)
{
	// write output to file
	QFile file(rfile);
	if (!file.open(QIODevice::Text | QIODevice::WriteOnly | (bappend ? QIODevice::Append : QIODevice::Truncate)))
	{
		QMessageBox::warning(window(), tr("Can't open file"), rfile);
		return;
	}

	QTextStream os(&file);
	bool bLogText = !(file.fileName().right(4) == "html");

	QString s;
	if (bLogText)
	{
		static QStringList lst(GetFontColorTagList());

		/*?int l(0);/ *? paragraphs());
		for (int i(0); i < l; i++)
		{
			QString s(text(i));
			for (QStringList::Iterator it(lst.begin()); it != lst.end(); it++)
				s.remove(*it);

			os << s;
			os << "\n";
		}*/

		//getText(s, lst);
	}
	else
	{
		getTextHtml(s);
		os << s;
	}

	file.close();
}


void SeditexTextEdit::slotSaveAs()
{
	emit signalSaveAs(this);
}


void SeditexTextEdit::updateOutputFont(const QFont& f)
{
	setFont(f);
}

void SeditexTextEdit::emitAllSignals()
{
	EmitSignals();
	//?if ( mbClosable )
	emit enableClose(true);
}

void SeditexTextEdit::slotClosePage()
{
	emit signalClosePage();
}

void SeditexTextEdit::slotAppend(const QString& s0, int id)
{
	if (id != mOutputId)
		return;

	append(s0);
	scrollToBottom();
	return;

	QString s(msLeft);
	s += s0;
	int n(s.indexOf(QChar('\n'), -1));
	if (n > 0)
	{
		msLeft = s.mid(n + 1);
		s.truncate(n);
	}
	else
		msLeft.truncate(0);

	//mbAutoScroll = false;
	append(s);
	if (mLineRef > 0)
	{
		/*?		int parNum = paragraphs();
				if (mLineRef-1 < parNum)
				{
					int para = 0;
					int index = 0;
					if (find(QString("%1 :").arg(mLineRef), false, true, TRUE, &para, &index))
					{
						QString s(text(para));
						s.truncate(index);
						s.simplifyWhiteSpace();
						if (s.isEmpty())
							mLineRef = -1;
					}
				}
				else
					scrollToBottom();*/
	}
}

void SeditexTextEdit::scrollToBottom()
{
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}


/////////////////////////////////////////////////////

SeditexInputOutputWin::SeditexInputOutputWin(QWidget* parent, const char* name)
	: SeditexCommandWin(parent, name, true),
	mpTextEdit2(nullptr),
	mbSplitPane(false)
{
	mpSplitter = new QSplitter(Qt::Vertical, this);
	addWidget(mpSplitter);
	EnableCommandLine(true);

	//setClientWidget(mpSplitter);
	mpTextEdit = createTextEditPane("TE1");
}

SeditexTextEdit* SeditexInputOutputWin::createTextEditPane(const char* name)
{
	SeditexTextEdit* e(new SeditexTextEdit(mpSplitter, name));
	e->setClearable(true);
	e->setAutoScroll(true);
	e->setChevrons(false);//bChevrons);

	connect(e, SIGNAL(enableCopy(bool)), SIGNAL(enableCopy(bool)));
	connect(e, SIGNAL(enableClear(bool)), SIGNAL(enableClear(bool)));
	connect(e, SIGNAL(enableFindText(bool)), SIGNAL(enableFindText(bool)));
	connect(e, SIGNAL(enableSaveAs(bool)), SIGNAL(enableSaveAs(bool)));
	connect(e, SIGNAL(enableFind(SxFindTextDlg::ITarget*)), SIGNAL(enableFind(SxFindTextDlg::ITarget*)));
	connect(e, SIGNAL(signalSaveAs(QTextEdit*)), SIGNAL(signalSaveAs(QTextEdit*)));
	connect(e, SIGNAL(signalPrint(SeditexTextEdit*)), SIGNAL(signalPrint(SeditexTextEdit*)));
	connect(e, SIGNAL(signalCreatePopupMenuRequested(QMenu*)),
		SLOT(slotCreatePopupMenuRequested(QMenu*)));

	return e;
}

SeditexInputOutputWin::~SeditexInputOutputWin()
{
	TurnCommandLine(false);
}

void SeditexInputOutputWin::slotCreatePopupMenuRequested(QMenu* popup)
{
	OnPopupMenu(popup);
	if (!mLastFile.isEmpty())
	{
		popup->addSeparator();

		QAction* pAction(new QAction(QIcon("std_apply_16.png"), tr("&Update"), this));
		connect(pAction, SIGNAL(activated()), SLOT(slotUpdate()));
		pAction->setShortcut(Qt::Key_F5);
		popup->addAction(pAction);
	}
}

void SeditexInputOutputWin::slotCopy() { mpTextEdit->slotCopy(); }
void SeditexInputOutputWin::slotClear() { mpTextEdit->slotClear(); }
void SeditexInputOutputWin::slotFlush() { mpTextEdit->slotFlush(); }
void SeditexInputOutputWin::slotSelectAll() { mpTextEdit->slotSelectAll(); }
void SeditexInputOutputWin::slotDeselect() { mpTextEdit->slotDeselect(); }
void SeditexInputOutputWin::slotFindText() { mpTextEdit->slotFindText(); }
void SeditexInputOutputWin::slotSaveAs() { mpTextEdit->slotSaveAs(); }
void SeditexInputOutputWin::slotPrint(QSilPageSetup* pPageSetup) { mpTextEdit->slotPrint(pPageSetup); }

void SeditexInputOutputWin::slotAppend(const QString& s, int id)
{
	mpTextEdit->slotAppend(s, id);
}

void SeditexInputOutputWin::slotAppend(bool bErr, const QString& s)
{
	if (!bErr || !mbSplitPane)
	{
		mpTextEdit->slotAppend(s, mpTextEdit->outputId());
		return;
	}

	if (!mpTextEdit2)
	{
		mpTextEdit2 = createTextEditPane("TE2");
		connect(mpTextEdit2, SIGNAL(signalCleared()), SLOT(slotCloseErrPane()));
		mpTextEdit2->resize(mpTextEdit2->size().width(), 100);
		mpTextEdit2->show();
	}
	mpTextEdit2->slotAppend(s, mpTextEdit2->outputId());
}

void SeditexInputOutputWin::slotCloseErrPane()
{
	if (mpTextEdit2)
	{
		mpTextEdit2->deleteLater();
		mpTextEdit2 = nullptr;
	}
}

/*void SeditexInputOutputWin::slotAppend(const QString &s)
{
	QTextEdit *pTEd = mpTextEdit;

	int par = pTEd->paragraphs();
	int pos = 0;
	if (par > 0)
	{
		par--;
		pos = pTEd->paragraphLength(par);
		if (pos > 0)
			pos--;
		pTEd->insertAt(s, par, pos);
	}
	else
		pTEd->append(s);

	pTEd->scrollToBottom();
}*/

void SeditexInputOutputWin::emitAllSignals()
{
	mpTextEdit->EmitSignals();
}

void SeditexInputOutputWin::updateOutputFont(const QFont& f)
{
	mpTextEdit->setFont(f);
	if (mpTextEdit2)
		mpTextEdit2->setFont(f);
	OnFontChange(f);
}

void SeditexInputOutputWin::readFile(QString path)
{
	mLastFile = "";
	mpTextEdit->clear();
	mpTextEdit->setReadOnly(true);//setTextFormat(Qt::LogText);
	mpTextEdit->setWordWrapMode(QTextOption::NoWrap);
	mpTextEdit->setAutoFormatting(QTextEdit::AutoNone);

	QFile file(path);
	if (file.open(QIODevice::ReadOnly))
	{
		QTextStream stream(&file);
#if(0)
		while (!stream.atEnd())
		{
			mpTextEdit->append(stream.readLine());
		}
#else
		mpTextEdit->setText(stream.readAll());
#endif
		file.close();
		mLastFile = path;
	}
}

void SeditexInputOutputWin::readFile(int, const QString& a)
{
	mpTextEdit->clear();
	mpTextEdit->setReadOnly(true);//setTextFormat(Qt::LogText);
	mpTextEdit->setWordWrapMode(QTextOption::NoWrap);
	mpTextEdit->setAutoFormatting(QTextEdit::AutoNone);

	mpTextEdit->setText(a);
}

void SeditexInputOutputWin::slotUpdate()
{
	readFile(mLastFile);
}

void SeditexInputOutputWin::keyPressEvent(QKeyEvent* e)
{
	switch (e->key())
	{
	case Qt::Key_F5:
		slotUpdate();
	default:
		return;
	}

	return QWidget::keyPressEvent(e);
}

void SeditexInputOutputWin::append(const QString& s)
{
	mpTextEdit->append(s);
}

void SeditexInputOutputWin::slotOutput(const QString& s)
{
	mpTextEdit->append(s);
}

void SeditexInputOutputWin::slotAppend(const QString& s)
{
	QTextEdit* pTEd(mpTextEdit);
	pTEd->append(s);

	/*	int par(pTEd->paragraphs());
		if (par > 0)
		{
			int pos(pTEd->paragraphLength(par-1));
			if (pos == 1 && pTEd->text(par-1) == "\n")
			{
				pTEd->removeParagraph(par-1);
				pTEd->append(s);
			}
			else
	//        if (pos > 0)
	//          pos--;
				pTEd->insertAt(s, par-1, pos);
		}
		else
			pTEd->append(s);

		pTEd->scrollToBottom();*/
}

void SeditexInputOutputWin::slotLogCommand(const QString&)
{
}

#include "SxDocument.h"

static DocumentObject* __createSmspiceInputOutputWin(SxMainWindow* pMainWin, QWidget* /*parent*/, QString& path, QString& extra)
{
	DocumentWin<SeditexOutputWin>* pWin = new DocumentWin<SeditexOutputWin>(pMainWin, extra.toLatin1());
	//pWin->init(false);
	pWin->setPermanent(true);
	pWin->setStrID(path, extra);
	pWin->mstrIcon = "std_console_16.png";
	return pWin;
}

static DocumentCreator gOutputWinDocCreator(
	"SMSPICEINPUTOUTPUTWIN", __createSmspiceInputOutputWin);

static DocumentObject* __createSSSOutputWin(SxMainWindow* pMainWin, QWidget* /*parent*/, QString& path, QString& extra)
{
	DocumentWin<SeditexOutputWin>* pWin = new DocumentWin<SeditexOutputWin>(pMainWin, extra.toLatin1());
	//pWin->init(false);
	pWin->setStrID(path, extra);
	pWin->mstrIcon = "ss_document_16.png";
	return pWin;
}

static DocumentCreator gSSSOutputWinDocCreator(
	"SSSOUTPUTWIN", __createSSSOutputWin);

