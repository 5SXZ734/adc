#include <QtCore/QVariant>
#include <QtGui/QKeyEvent>
#include <QApplication>
#include <QComboBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QLayout>
#include <QToolTip>
#include <QWhatsThis>
#include "SxFindTextDlg.h"

//SxFindTextDlg *SxFindTextDlg::mpThis = nullptr;

SxFindTextDlg::SxFindTextDlg(QWidget* parent)
	: QDialog(parent),
	mpTed4Search(nullptr),
	mbReverseSearch(false),
	mbAdd2ComboBox(false),
	miCountSearch(0)
{
	setWindowTitle(tr("Find Text"));

	mpFindLabel = new QLabel(tr("F&ind:"), this);

	mpFindTextCbx = new QComboBox(this);
	mpFindTextCbx->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	mpFindTextCbx->setEditable(true);
	mpFindTextCbx->setInsertPolicy(QComboBox::InsertAtTop);
	mpFindTextCbx->setCompleter(nullptr);// setAutoCompletion(false);
	mpFindTextCbx->setDuplicatesEnabled(false);
	connect(mpFindTextCbx, SIGNAL(editTextChanged(const QString&)), SLOT(findTextchanged(const QString&)));
	//onnect( mpFindNextBtn, SIGNAL( clicked() ), this, SLOT( slotFindNext2() ) ); 

	mpMatchWordCheck = new QCheckBox(this);
	mpMatchWordCheck->setText(tr("Match &whole word"));
	mpMatchWordCheck->setShortcut(QKeySequence(tr("Alt+W")));
	connect(mpMatchWordCheck, SIGNAL(stateChanged(int)), this, SLOT(slotWholeWordChbxStateChanged(int)));

	mpRegExpCheck = new QCheckBox(this);
	mpRegExpCheck->setText(tr("Regular &expression"));
	mpRegExpCheck->setShortcut(QKeySequence(tr("Alt+E")));

	mpCaseSensitiveCheck = new QCheckBox(this);
	mpCaseSensitiveCheck->setText(tr("&Case sensitive"));
	mpCaseSensitiveCheck->setShortcut(QKeySequence(tr("Alt+C")));
	connect(mpCaseSensitiveCheck, SIGNAL(stateChanged(int)), this, SLOT(slotCaseSensitiveChbxStateChanged(int)));

	mpReverseCheck = new QCheckBox(this);
	mpReverseCheck->setText(tr("Reverse &search"));
	mpReverseCheck->setShortcut(QKeySequence(tr("Alt+S")));
	connect(mpReverseCheck, SIGNAL(stateChanged(int)), this, SLOT(slotReverseSearchStateChanged(int)));

	mpFindNextBtn = new QPushButton(this);
	mpFindNextBtn->setText(tr("&Find Next"));
	mpFindNextBtn->setShortcut(QKeySequence(tr("Alt+F")));
	mpFindNextBtn->setAutoDefault(true);
	mpFindNextBtn->setDefault(true);
	connect(mpFindNextBtn, SIGNAL(clicked()), this, SLOT(slotFindNext()));
	mpFindNextBtn->setEnabled(false);

	mpCancelBtn = new QPushButton(this);
	mpCancelBtn->setText(tr("Cancel"));
	connect(mpCancelBtn, SIGNAL(clicked()), SLOT(reject()));

	QHBoxLayout* pHBox1(new QHBoxLayout);
	pHBox1->addWidget(mpFindLabel);
	pHBox1->addWidget(mpFindTextCbx);

	QHBoxLayout* pHBox2(new QHBoxLayout);
	pHBox2->addStretch();
	pHBox2->addWidget(mpFindNextBtn);
	pHBox2->addWidget(mpCancelBtn);

	QGridLayout* pGrid(new QGridLayout);
	pGrid->addWidget(mpCaseSensitiveCheck, 0, 0);
	pGrid->addWidget(mpMatchWordCheck, 1, 0);
	pGrid->addWidget(mpRegExpCheck, 0, 1);
	pGrid->addWidget(mpReverseCheck, 1, 1);

	QVBoxLayout* baseLayout(new QVBoxLayout(this));
	baseLayout->addLayout(pHBox1);
	baseLayout->addLayout(pGrid);
	baseLayout->addLayout(pHBox2);

	// tab order
	setTabOrder(mpFindTextCbx, mpCaseSensitiveCheck);
	setTabOrder(mpCaseSensitiveCheck, mpMatchWordCheck);
	setTabOrder(mpMatchWordCheck, mpRegExpCheck);
	setTabOrder(mpRegExpCheck, mpReverseCheck);
	setTabOrder(mpReverseCheck, mpFindNextBtn);
	setTabOrder(mpFindNextBtn, mpCancelBtn);

	// buddies
	mpFindLabel->setBuddy(mpFindTextCbx);

	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
	setFixedHeight(sizeHint().height());

	resize(QSize(400, 180).expandedTo(minimumSizeHint()));
}

void SxFindTextDlg::slotFindNext()
{
	if (!mpFindNextBtn->isEnabled())
		return;

	if (!mpTed4Search)
		return;

	/*if (mpTed4Search->type() == 1)
	{
		slotFindNext2();
		return;
	}*/

	bool bresult = false;
	if (mbAdd2ComboBox)
	{
		mpFindTextCbx->insertItem(0, mpFindTextCbx->currentText());
		mbAdd2ComboBox = false;
		miCountSearch = 0;
	}

	QString text = mpFindTextCbx->currentText();
	bool case_sensitive = mpCaseSensitiveCheck->isChecked();
	bool match_whole_word = mpMatchWordCheck->isChecked();
	bool reverse = mpReverseCheck->isChecked();
	//bool reg_exp = mpRegExpCheck->isChecked();

	bresult = mpTed4Search->findIt(text, case_sensitive, match_whole_word, reverse, false);//from cp
	if (bresult)
	{
		miCountSearch++;
		return;
	}

	if (miCountSearch == 0)
	{
		QMessageBox::information(this, tr("Find Text"), tr("Search string not found"),
			QMessageBox::Ok | QMessageBox::Default, 0, 0);
		return;
	}

	int btn = QMessageBox::No;

	if (!mbReverseSearch)
	{
		btn = QMessageBox::question(this, tr("Find Text"), tr("End of document reached, continue from beginning?"),
			QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape, 0);
	}
	else
	{
		btn = QMessageBox::question(this, tr("Find Text"), tr("Beginning of document reached, continue from end?"),
			QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape, 0);
	}
	if (btn == QMessageBox::Yes)
		bresult = getTed()->findIt(text, case_sensitive, match_whole_word, reverse, true);//reset cp
}

/*void SxFindTextDlg::slotFindNext2()
{
	//Ensure that combo gets populated with find text
//	activateFindCombo();

	// Don't allow find text box to be empty
	bool reverse = mpReverseCheck->isChecked();
	bool case_sensitive = mpCaseSensitiveCheck->isChecked();
	bool match_whole_word = mpMatchWordCheck->isChecked();
	//bool reg_exp = mpRegExpCheck->isChecked();
	QString text = mpFindTextCbx->currentText();

	if (text.isEmpty())
	{
		QMessageBox::information(this,
			tr("Find Text"),
			tr("Please enter text to search for"),
			QMessageBox::Ok | QMessageBox::Default, 0, 0);

		mpFindTextCbx->setFocus();
		return;
	}

	bool ret = getTed()->findIt(text, case_sensitive, match_whole_word, reverse);

	if (!ret)
	{
		QMessageBox::information(this,
			tr("Find Text"),
			tr("Search string not found"),
			QMessageBox::Ok | QMessageBox::Default, 0, 0);
	}
}*/

void SxFindTextDlg::slotCancel()
{
	close();
}

void SxFindTextDlg::slotReverseSearchStateChanged(int /*value*/)
{
	mbReverseSearch = !mbReverseSearch;
}

void SxFindTextDlg::windowActivationChange(bool oldActive)
{
	if (isActiveWindow())
		setWindowOpacity(1.0);
	else if (oldActive)
		setWindowOpacity(0.65);

	//?SxFindTextDlgBase::windowActivationChange(oldActive);
}

void SxFindTextDlg::slotWholeWordChbxStateChanged(int)
{
	miCountSearch = 0;
}

void SxFindTextDlg::slotCaseSensitiveChbxStateChanged(int)
{
	miCountSearch = 0;
}

void SxFindTextDlg::findTextchanged(const QString& text)
{
	mpFindNextBtn->setEnabled(!text.isEmpty());
	mbAdd2ComboBox = true;
}

void SxFindTextDlg::showEvent(QShowEvent* e)
{
	QDialog::showEvent(e);
	restorePosition();
	activate();
}

void SxFindTextDlg::activate()
{
	QApplication::setActiveWindow(mpFindTextCbx);
	//    QString selection = getTed()->selectedText();
	//    mpFindTextCbx->setEditText(selection);
	mpFindTextCbx->lineEdit()->selectAll();
	//    mpFindNextBtn->setEnabled(!selection.isEmpty());
	mpFindTextCbx->lineEdit()->setFocus();
}

void SxFindTextDlg::setFont(QFont f)
{
	mpFindTextCbx->setFont(f);
	mpFindTextCbx->lineEdit()->setFont(f);
}

void SxFindTextDlg::closeEvent(QCloseEvent *e)
{
	//savePosition();
	QDialog::closeEvent(e);
}

void SxFindTextDlg::activateFindCombo()
{
	//Insert text into combo - simulate "Enter" being pressed
	mpFindNextBtn->blockSignals(true);
	QKeyEvent key_event(QEvent::KeyPress, Qt::Key_Enter, Qt::KeyboardModifiers());//?, Qt::NoButton);
	qApp->sendEvent(mpFindTextCbx->lineEdit(), &key_event);
	mpFindNextBtn->blockSignals(false);
}









