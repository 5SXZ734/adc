#include <QSpinBox>
#include <QShortcut>
#include "ADCProtoDlg.h"
#include "ADCStream.h"


///////////////////////////////////////////////////ADCProtoEditDlg
ADCProtoEditDlg::ADCProtoEditDlg(QWidget* parent)
	: QDialog(parent),
	mStackAddrSize(0),//default
	mStackPurgeRet(0)
{
	setupUi(this);
	setModal(false);
	setWindowTitle(tr("Quick Prototype"));
	setWindowIcon(QIcon(":type_edit4.png"));

	QPalette palette(mpleObject->palette());
	palette.setColor(mpleObject->backgroundRole(), Qt::lightGray);
	//palette.setColor(mpleObject->foregroundRole(), Qt::white);
	//mpleObject->setAutoFillBackground(true);
	mpleObject->setPalette(palette);

	//mpsbStackIn->setReadOnly(true);
	mpsbStackIn->setKeyboardTracking(false);
	mpsbStackPurge->setKeyboardTracking(false);
	mpsbStackPurge->setRange(SHRT_MIN, SHRT_MAX);

	connect(mpsbStackIn, SIGNAL(valueChanged(int)), SLOT(slotStackInValueChanged(int)));
	connect(mpsbStackPurge, SIGNAL(valueChanged(int)), SLOT(slotStackPurgeValueChanged(int)));
	connect(mpcbStdCall, SIGNAL(toggled(bool)), SLOT(slotStdCallToggled(bool)));
	connect(mpcbVariardic, SIGNAL(toggled(bool)), SLOT(slotVariardicToggled(bool)));

	mpsbFpuIn->setRange(0, 8);
	mpsbFpuPurge->setRange(-8, 8);

	connect(mppbAccept, SIGNAL(clicked()), SLOT(slotAccept()));
	connect(mppbHide, SIGNAL(clicked()), SLOT(slotHide()));
	connect(mppbApply, SIGNAL(clicked()), SLOT(slotApply()));

	new QShortcut(QKeySequence(Qt::Key_F4), this, SLOT(slotAccept()));

	//layout()->setSizeConstraint(QLayout::SetFixedSize);
	//updateGeometry();
}

ADCProtoEditDlg::~ADCProtoEditDlg()
{
}

void ADCProtoEditDlg::slotStackInValueChanged(int v)
{
	if (mStackAddrSize > 0)
	{
		mpsbStackIn->setValue((v / mStackAddrSize) * mStackAddrSize);
		if (mpcbStdCall->isChecked())
			mpsbStackPurge->setValue(mpsbStackIn->value());
	}
}

void ADCProtoEditDlg::slotStackPurgeValueChanged(int v)
{
	if (mStackAddrSize > 0)
		mpsbStackPurge->setValue((v / mStackAddrSize) * mStackAddrSize);
}

void ADCProtoEditDlg::slotAccept()
{
	hide();

	ADCProtoEditDlg::Data aData;
	getData(aData);

	emit signalSetProtoData(aData, true);//continue analysis
}

void ADCProtoEditDlg::slotHide()
{
	hide();
}

void ADCProtoEditDlg::slotApply()
{
}

void ADCProtoEditDlg::setObjectInfo(QString s)
{
	mpleObject->setText(s);
}

void ADCProtoEditDlg::setConfig(int stackaddrsize)
{
	mStackAddrSize = stackaddrsize;
	mpsbStackIn->setSingleStep(stackaddrsize);
	mpsbStackPurge->setSingleStep(stackaddrsize);
}

void ADCProtoEditDlg::setData(const Data &a)
{
	{
		mpsbStackIn->blockSignals(true);
		mpsbStackIn->setValue(a.nPSin);
		mpsbStackIn->blockSignals(false);
	}
	mpsbStackPurge->setValue(a.nPStackPurge);
	{
		mpcbStdCall->blockSignals(true);
		mpcbStdCall->setChecked(a.bPScleanArgs);
		mpcbStdCall->blockSignals(false);
	}
	mpsbFpuIn->setValue(a.nFPRin);
	mpsbFpuPurge->setValue(a.nFPRout);
	mpleArgRegs->setText(a.sGPRin);
	mpleSpoiltRegs->setText(a.sSpoiledRegs);
	mpcbVariardic->setChecked(a.bVariardic);
	mpcbUsercall->setChecked(a.bUsercall);
	mpcbThisCall->setChecked(a.bThiscall);
	//mpleEFlagsSaved->setText(a.sFlagsSav);
	mpleRetRegs->setText(a.sGPRout);
}

void ADCProtoEditDlg::getData(Data&a)
{
	a.nPSalign = mStackAddrSize;

	a.nPSin = mpsbStackIn->value();
	a.nPStackPurge = mpsbStackPurge->value();//mpcbStdCall->isChecked() ? a.nPSin : 0;
	a.nPStackPurgeRet = mStackPurgeRet;
	a.nFPRin = mpsbFpuIn->value();
	a.nFPRout = mpsbFpuPurge->value();
	a.sGPRin = mpleArgRegs->text();
	a.sSpoiledRegs = mpleSpoiltRegs->text();
	a.bPScleanArgs = mpcbStdCall->isChecked();
	a.bVariardic = mpcbVariardic->isChecked();
	a.bUsercall = mpcbUsercall->isChecked();
	a.bThiscall = mpcbThisCall->isChecked();
	//a.sFlagsSav = mpleEFlagsSaved->text();
	a.sGPRout = mpleRetRegs->text();
}


void ADCProtoEditDlg::changeEvent(QEvent *e)
{
	//if (e->type() == QEvent::ActionChanged)
	/*{
		if (isActiveWindow())
			setWindowOpacity(1.0);
		else
			setWindowOpacity(0.65);
	}*/
    
    QDialog::changeEvent(e);
}

void ADCProtoEditDlg::showEvent(QShowEvent *)
{
	mpsbStackIn->setFocus();
	//QDialog::showEvent(e);//we want it at old position
}

void ADCProtoEditDlg::slotSetProtoData(const Data &aData)
{
	setObjectInfo(aData.sObject);

	setConfig(aData.nPSalign);
	mStackPurgeRet = aData.nPStackPurgeRet;

	setData(aData);
}

void ADCProtoEditDlg::slotStdCallToggled(bool bOn)
{
	mpsbStackPurge->setReadOnly(bOn);
	if (bOn)
		mpsbStackPurge->setValue(mpsbStackIn->value());
	else
		mpsbStackPurge->setValue(0);
}

void ADCProtoEditDlg::slotVariardicToggled(bool bOn)
{
	if (bOn)
		mpcbStdCall->setChecked(false);
}







