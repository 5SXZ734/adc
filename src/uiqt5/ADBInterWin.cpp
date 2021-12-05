#include "ADBInterWin.h"

#include <QLineEdit>
#include <QToolButton>
#include <QAction>
#include <QKeyEvent>
#include <QMenu>
#include <QLayout>

////////////////////////////////////////////////
// ADCInterDataWin::Toolbar

ADCInterWinToolbar::ADCInterWinToolbar(QWidget *parent)
	: QFrame(parent)
{
	setAutoFillBackground(true);
	setBackgroundRole(QPalette::Window);
	setFrameStyle(QFrame::Panel | QFrame::Raised);
	
	mpLineEdit = new QLineEdit(this);
	//mpLineEdit->setEditable(true);
	mpLineEdit->setSizePolicy(QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed));
	//mpLineEdit->setMinimumSize(QSize(120, 0));
	//mpGotoCbx->lineEdit()->installEventFilter(this);
//?	connect(mpLineEdit, SIGNAL(returnPressed()), SLOT(slotGo2Location()));
	mpLineEdit->setToolTip(tr("Apply"));
	connect(mpLineEdit, SIGNAL(textChanged(const QString &)), SLOT(slotTextChanged()));

	mpApplyAction = new QAction(QIcon(":go.png"), tr("Apply"), this);
	mpApplyAction->setEnabled(false);
	connect(mpApplyAction, SIGNAL(triggered()), SIGNAL(signalApply()));

	mpApplyTbtn = new QToolButton(this);
	mpApplyTbtn->setAutoRaise(true);
	mpApplyTbtn->setDefaultAction(mpApplyAction);

	//QAction *pDropAction = new QAction(QIcon(":dropdown_16.png"), tr("Menu"), this);

	mpDropdownTbtn = new QToolButton(this);
	mpDropdownTbtn->setText(tr("View Options"));
	mpDropdownTbtn->setToolTip(mpDropdownTbtn->text());
	mpDropdownTbtn->setIcon(QIcon(":dc_view_16.png"));//repair_16//download_16//set_down_double_16//arrow_down_16//settings_16//page_white_gear_16//dots_more_16//gears_16
	mpDropdownTbtn->setAutoRaise(true);//arrow_down_2_16
	mpDropdownTbtn->setPopupMode(QToolButton::InstantPopup);
//	mpDropdownTbtn->setDefaultAction(mpApplyAction);

	mpPopup = new QMenu(this);
	mpDropdownTbtn->setMenu(mpPopup);
	
//?	connect(mpApplyTbtn, SIGNAL(clicked()), SLOT(slotGo2Location()));

	//mpRefreshTbtn = new QToolButton(this);
	//mpRefreshTbtn->setAutoRaise(true);

	QHBoxLayout * hbox(new QHBoxLayout);
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->addWidget(mpDropdownTbtn);
	hbox->addWidget(mpLineEdit);
	hbox->addWidget(mpApplyTbtn);

	setLayout(hbox);
}

void ADCInterWinToolbar::slotTextChanged()
{
	updateState();
}

void ADCInterWinToolbar::setEnabled(bool b)
{
	mpDropdownTbtn->setEnabled(b);
	//mpPopup->setEnabled(b);
	mpLineEdit->setEnabled(b);
	if (mpApplyTbtn->defaultAction())
		mpApplyTbtn->defaultAction()->setEnabled(b);
	else
		mpApplyTbtn->setEnabled(b);
	if (!b)
		mpLineEdit->clear();
}

void ADCInterWinToolbar::updateState(QString s)
{
	if (s == mpLineEdit->text())
		return;
	mpLineEdit->setText(s);
	updateState();
}

void ADCInterWinToolbar::updateState()
{
	bool bMatch(mData.isEmpty() || mData.dispText() == mpLineEdit->text());
	QPalette palette(mpLineEdit->palette());
	palette.setColor(mpLineEdit->foregroundRole(), bMatch ? Qt::black : Qt::gray);
	mpLineEdit->setPalette(palette);
	mpApplyAction->setEnabled(!mpLineEdit->text().isEmpty() && (!bMatch || mData.isEmpty()));
}



///////////////////////////////////////////////////// ADCInterWin

ADCInterWin::ADCInterWin(QWidget * parent, const char *name)
	: QWidget(parent),
	mpToolbar(nullptr)
{
	setObjectName(name);
	//setFrameStyle(QFrame::Panel | QFrame::Raised);

	mpToolbar = new ADCInterWinToolbar(this);
	mpToolbar->setFont(font());
	mpToolbar->lineEdit()->installEventFilter(this);
	connect(mpToolbar, SIGNAL(signalApply()), SLOT(slotApplySubject()));
	//mpToolbar->mpApplyTbtn->setDefaultAction(mpView->mpGotoEntryAction);
	mpToolbar->setEnabled(false);

	QVBoxLayout * vbox(new QVBoxLayout(this));
	vbox->setSpacing(0);
	vbox->setContentsMargins(0, 0, 0, 0);
	//setLayout(vbox);

	vbox->addWidget(mpToolbar);
}

QString ADCInterWin::getToolbarText() const
{
	return mpToolbar->lineEdit()->text();
}

void ADCInterWin::setToolbarEnabled(bool bEnable)
{
	mpToolbar->setEnabled(bEnable);
}

void ADCInterWin::slotApplySubject()
{
	applySubject(getToolbarText());
}

bool ADCInterWin::eventFilter(QObject * pObject, QEvent * e)
{
	if (pObject == mpToolbar->lineEdit() && e->type() == QEvent::KeyPress)
	{
		int key = ((QKeyEvent *)e)->key();
		if (key == Qt::Key_Enter || key == Qt::Key_Return)
		{
			//mpToolbar->setPath("");
			//slotReset(mpToolbar->typePath());
			slotApplySubject();
			return true;	// prevent default button from receiving event
		}
	}

	return QWidget::eventFilter(pObject, e);
}



