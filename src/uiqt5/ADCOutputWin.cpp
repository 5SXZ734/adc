
#include <stdarg.h>

#include <QTextEdit>
#include <QLayout>
#include <QLineEdit>
#include <QLabel>

#include "ADCOutputWin.h"

ADC_OutputWin::ADC_OutputWin(QWidget * parent, const char *name)
	: QWidget(parent)//, name)
{
	setObjectName(name);

	mpTextEdit = new QTextEdit(this);
	mpTextEdit->setReadOnly(true);
	mpTextEdit->setWordWrapMode(QTextOption::NoWrap);
	mpTextEdit->setLineWrapMode(QTextEdit::NoWrap);

	mpLineEdit = new QLineEdit(this);

	mpLabel = new QLabel(this);
    mpLabel->setText( tr("Command :") );

	QHBoxLayout * h = new QHBoxLayout;// (0, 0, 6);
	h->addWidget(mpLabel);
	h->addWidget(mpLineEdit);

	QVBoxLayout * v = new QVBoxLayout;// (this, 0, 0);
	v->addLayout(h);
	v->addWidget(mpTextEdit);

	setLayout(v);
}


void ADC_OutputWin::slotAppend(const QString &s, int)
{
	mpTextEdit->append(s);
}

void ADC_OutputWin::updateOutputFont(const QFont &f)
{
	mpTextEdit->setFont(f);
	mpLineEdit->setFont(f);
	//mpLabel->setFont(f);
}


