#include <QLabel>
#include <QComboBox>
#include <QToolButton>
#include <QToolTip>
#include <QCompleter>
#include <QApplication>
#include <QtGui/QClipboard>
#include <QMenu>
#include <QtGui/QDragEnterEvent>

#include "SxCommandLine.h"


//////////////////////////////////////////////////////////////////////////
// SxCommandLineEdit

SxCommandLineEdit::SxCommandLineEdit(QWidget *parent, const char * /*pname*/)
:   QLineEdit(parent)//, pname)
{
    setMaxLength(20400);
    //setAcceptDrops(true);

	connect( this, SIGNAL( textChanged ( const QString & ) ), 
		SLOT( slotTextChanged( const QString & ) ) );
	connect( this, SIGNAL( selectionChanged() ), 
		SLOT( slotSelectionChanged() ) );

}

SxCommandLineEdit::~SxCommandLineEdit()
{
}

void SxCommandLineEdit::dragEnterEvent(QDragEnterEvent * /*e*/)
{
/*?	if (Q3UriDrag::canDecode(e))
        e->acceptAction();
    else
        e->ignore();*/
}

void SxCommandLineEdit::dragMoveEvent(QDragMoveEvent * /*e*/)
{
/*?    if(Q3UriDrag::canDecode(e))
        e->acceptAction();
    else
        e->ignore();*/
}

void SxCommandLineEdit::dropEvent(QDropEvent * e)
{
	qApp->sendEvent(window(), e);// qApp->mainWidget(), e );
}

/*void SxCommandLineEdit::emitAllSignals()
{
	slotTextChanged( QString() );
	slotSelectionChanged();
	//slotClipboardStateChanged();
	emit enablePaste( isPasteAvailable() );
    //slotUndoStateChanged();
}*/

void SxCommandLineEdit::slotTextChanged ( const QString & )
{
	emit enableUndo( isUndoAvailable() );
	emit enableRedo( isRedoAvailable() );
}

void SxCommandLineEdit::slotSelectionChanged()
{
    bool selection = hasSelectedText();
	emit enableCopy( selection );
	emit enableCut( selection );
	emit enableDelete( selection );
}

void SxCommandLineEdit::slotUndo() {
	QLineEdit::undo(); }
void SxCommandLineEdit::slotRedo() {
	QLineEdit::redo(); }
void SxCommandLineEdit::slotCopy() {
	QLineEdit::copy(); }
void SxCommandLineEdit::slotCut() {
	QLineEdit::cut(); }
void SxCommandLineEdit::slotPaste() {
	QLineEdit::paste(); }
void SxCommandLineEdit::slotDelete() {
	QLineEdit::del(); }
void SxCommandLineEdit::slotSelectAll() {
	QLineEdit::selectAll(); }
void SxCommandLineEdit::slotDeselect() {
	QLineEdit::deselect(); }

bool SxCommandLineEdit::isPasteAvailable() const
{
 /*?   QClipboard* cb = QApplication::clipboard();
    if ( cb->ownsClipboard() )
		return true;

    QString text;
    bool can_decode = Q3TextDrag::canDecode( cb->data( QClipboard::Clipboard ) );
    return (can_decode);*/return false;
}



////////////////////////////////////////////////////////////////
// SxCommandLine

SxCommandLine::SxCommandLine(QWidget *parent, const QString &, bool bIns)
: QWidget(parent),
mLblPix(this),
mLbl(this),
mpCBx(nullptr),
mpInputLed(nullptr)
{
	mLblPix.setPixmap(QPixmap(":input_line_16.png"));

	setPrompt(QString());

	mpInputLed = new SxCommandLineEdit(this, "InputLed");
	//connect(mpInputLed, SIGNAL(returnPressed()), SLOT(slotReturnPressed()));

	mpCBx = new QComboBox(this);//?true, pOwner, "Cbx");
	mpCBx->setEditable(true);
	mpCBx->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
	mpCBx->setLineEdit(mpInputLed);
	if (bIns)
		mpCBx->setInsertPolicy(QComboBox::InsertAtTop);
	else
		mpCBx->setInsertPolicy(QComboBox::NoInsert);
	mpCBx->setDuplicatesEnabled(false);
	mpCBx->setCompleter(new QCompleter(mpCBx->model(), mpCBx->lineEdit()));// setAutoCompletion(true);
	/*d->completer = new QCompleter(d->model, d->lineEdit);
	connect(d->completer, SIGNAL(activated(QModelIndex)), this, SLOT(_q_completerActivated(QModelIndex)));
	d->completer->setCaseSensitivity(d->autoCompletionCaseSensitivity);
	d->completer->setCompletionMode(QCompleter::InlineCompletion);
	d->completer->setCompletionColumn(d->modelColumn);
	d->lineEdit->setCompleter(d->completer);
	d->completer->setWidget(this);*/

	lineEdit()->setToolTip(tr("Type 'cmdlist' for a list of available commands"));
	connect( mpCBx, SIGNAL(activated(const QString&)), SLOT(slotInputCbxActivated(const QString&)) );
	connect(lineEdit(), SIGNAL(returnPressed()), SLOT(slotReturnPressed()));
	
	QToolButton *pCloseTBtn = new QToolButton(this);//, "CloseTBtn");
	pCloseTBtn->setText(tr("Close"));
	pCloseTBtn->setIcon(QIcon(":std_close_16.png"));
	pCloseTBtn->setAutoRaise(true);
	pCloseTBtn->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	pCloseTBtn->setToolTip(tr("Close"));
	connect(pCloseTBtn, SIGNAL(clicked()), SIGNAL(signalClose()));

	QHBoxLayout *hbox(new QHBoxLayout(this));
	setLayout(hbox);
	hbox->addWidget(&mLbl);
	hbox->addWidget(&mLblPix);
	hbox->addWidget(mpCBx);
	hbox->addWidget(pCloseTBtn);
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->setSpacing(0);
}

SxCommandLine::~SxCommandLine()
{
/*?	QLayoutIterator it = iterator();
	QLayoutItem *child;
	while ((child = it.current()) != 0)
	{
		it.takeCurrent();
		delete child->widget();
		delete child->layout();
		delete child;
	}*/
}

void SxCommandLine::setFont(const QFont &f)
{
	mpCBx->setFont(f);
}

void SxCommandLine::setFocus(bool bSelectAll)
{
	lineEdit()->setFocus();
	if (bSelectAll)
		lineEdit()->selectAll();
}

bool SxCommandLine::hasFocus() const
{
	return lineEdit()->hasFocus();
}

QLineEdit *SxCommandLine::lineEdit() const
{
	return mpCBx->lineEdit();
}

void SxCommandLine::slotReturnPressed()
{
	QString s = lineEdit()->text();
	if (!s.isEmpty())
	{
		emit signalCommand(s);
		lineEdit()->clear();
	}
}

void SxCommandLine::slotInputCbxActivated(const QString& str)
{
	mpInputLed->clear();
	mpInputLed->setText(str);
	mpInputLed->setFocus();
}

void SxCommandLine::updateOutputFont(const QFont &f)
{
	mpCBx->setFont(f);
}

void SxCommandLine::setCommandList(const QStringList &l)
{
	mpCBx->blockSignals(true);
	mpCBx->clear();
	for (QStringList::ConstIterator it(l.begin()); it != l.end(); it++)
		mpCBx->addItem(*it);
	//mpCBx->insertStringList(l);
	mpCBx->blockSignals(false);
	mpInputLed->clear();
	mpCBx->setEnabled(!l.isEmpty());
}

QString SxCommandLine::getText()
{
	return mpInputLed->text();
}

void SxCommandLine::setText(const QString &s)
{
	mpInputLed->setText(s);
}

void SxCommandLine::clear()
{
	mpInputLed->clear();
}

void SxCommandLine::slotRun()
{
	slotReturnPressed();
}

void SxCommandLine::setPrompt(QString prompt)
{
	if (prompt.isEmpty())
		prompt = tr("Command ");
	mLbl.setText(prompt);
}

/////////////////////////////////////////////////////////
// SeditexCommandWin

SeditexCommandWin::SeditexCommandWin(QWidget *parent, const char *name, bool bIns)
: QWidget(parent),//, name),
//mpClient(nullptr),
mpCommandLine(nullptr),
//mbCLDisabled(false),
mbIns(bIns)
{
	setObjectName(name);

	mpVBox = new QVBoxLayout(this);
	mpVBox->setContentsMargins(0, 0, 0, 0);
	mpVBox->setSpacing(0);

//	connect(mpTextEdit, SIGNAL(signalCreatePopupMenuRequested(QMenu *)),
//		SLOT(slotCreatePopupMenuRequested(QMenu *)));
	
}

void SeditexCommandWin::addWidget(QWidget *pWin)
{
	//mpClient = pWin;
	mpVBox->addWidget(pWin);
}

SeditexCommandWin::~SeditexCommandWin()
{
	//TurnCommandLine(false);
}

void SeditexCommandWin::slotToggleCommandLine()
{
	TurnCommandLine(!mpCommandLine->isVisible(), false);
}

bool SeditexCommandWin::TurnCommandLineOn()
{
	return TurnCommandLine(true);
	/*if (mpCommandLine && mpCommandLine->isHidden())
	{
		mpCommandLine->show();
		mpCommandLine->focus();
		return true;
	}
	return false;*/
}

bool SeditexCommandWin::TurnCommandLineOff(bool)
{
	return TurnCommandLine(false);
	/*if (mpCommandLine && mpCommandLine->isShown())
	{
		mpCommandLine->hide();
		return true;
	}
	return false;*/
}

void SeditexCommandWin::slotCloseCommandLine()
{
	TurnCommandLine(false);
}

void SeditexCommandWin::slotCmdListChanged(const QStringList &l)
{
	if (mpCommandLine)
		mpCommandLine->setCommandList(l);
}

void SeditexCommandWin::slotDisplayCommand(const QString &s)
{
	if (mpCommandLine)
	{
		mpCommandLine->setText(s);
		mpCommandLine->setFocus();
	}
}

void SeditexCommandWin::slotCommand()
{
	if (mpCommandLine)
		slotCommand(mpCommandLine->getText());
}

void SeditexCommandWin::slotCommand(const QString &s)
{
	if (s.isEmpty())
		return;

	emit signalCommand(mSenderId, s);

	//regain focus
	if (mpCommandLine)
	{
		//mpCommandLine->clear();
		mpCommandLine->setFocus();
	}
}

void SeditexCommandWin::focusInEvent(QFocusEvent *e)
{
	QWidget::focusInEvent(e);
	if (mpCommandLine)
		mpCommandLine->setFocus();
}

bool SeditexCommandWin::EnableCommandLine(bool bEnable, const QString &senderId)
{
	if (!bEnable)
	{
		if (mpCommandLine)
		{
			TurnCommandLine(false);
//			delete mpCommandLine;
			mpCommandLine->deleteLater();
			mSenderId = QString();
			mPrompt = QString();
		}
	}
	else if (!mpCommandLine)
	{
		mpCommandLine = new SxCommandLine(this, QString(objectName()), mbIns);
		mpVBox->addWidget(mpCommandLine);
		mpCommandLine->hide();
		mpCommandLine->setPrompt(mPrompt);
		//mpCommandLine->setFont(mpClient->font());
		connect(mpCommandLine, SIGNAL(signalCommand(const QString &)), SLOT(slotCommand(const QString &)));
		connect(mpCommandLine, SIGNAL(signalClose()), SLOT(slotCloseCommandLine()));

		mSenderId = senderId;
		QString aName(mSenderId.section(QChar('\t'), 0, 0));
		if (!aName.isEmpty())
			mPrompt = tr("%1 Command:").arg(aName);
	}
	//mbCLDisabled =!bEnable;
	return mpCommandLine != nullptr;// !mbCLDisabled;
}

bool SeditexCommandWin::TurnCommandLine(bool bOn, bool bUnconditionally)
{
	if (!bUnconditionally)
	{
		if (mpCommandLine->isVisible())
		{
			if (!mpCommandLine->hasFocus())
			{
				mpCommandLine->setFocus(true);
				return false;
			}
		}
	}

	//if (!mbCLDisabled)
	if (mpCommandLine)
	{
		if (bOn)
		{
			mpCommandLine->show();
			mpCommandLine->setFocus();
			emit signalCmdLineToggled(true);
			return true;
		}
		else
		{
			mpCommandLine->hide();
			emit signalCmdLineToggled(false);
			return true;
		}
	}
	return false;
}

void SeditexCommandWin::slotSetPrompt(QString s)
{
	mPrompt = s;
	if (mpCommandLine)
		mpCommandLine->setPrompt(s);
}

void SeditexCommandWin::OnPopupMenu(QMenu *popup)
{
	//if (!mbCLDisabled)
	if (mpCommandLine)
	{
		popup->addSeparator();

		QAction *pAction(new QAction(tr("&Toggle Command Line"), popup));
		connect(pAction, SIGNAL(triggered()), SLOT(slotToggleCommandLine()));
		popup->addAction(pAction);
		//popup->setAccel(CTRL + Key_T, id);
	}
}

void SeditexCommandWin::OnFontChange(const QFont &f)
{
	//mpClient->setFont(f);
	if (mpCommandLine)
		mpCommandLine->setFont(f);
}

