#include <QLineEdit>
#include <QToolButton>
#include <QLayout>
#include <QAction>
#include <QKeyEvent>
#include <QMenu>
#include "ADBDataWin.h"
#include "ADCCodeWin.h"
#include "ADCBinWin.h"



////////////////////////////////////////////////
// ADCInterDataWin::Toolbar

/*ADCInterDataWin::Toolbar::Toolbar(QWidget *parent)
	: QFrame(parent)
{
	setAutoFillBackground(true);
	setBackgroundRole(QPalette::Window);
	setFrameStyle(QFrame::Panel | QFrame::Raised);

	mpLineEdit = new QLineEdit(this);
	//mpLineEdit->setEditable(true);
	mpLineEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
	//mpLineEdit->setMinimumSize(QSize(120, 0));
	//mpGotoCbx->lineEdit()->installEventFilter(this);
	//?	connect(mpLineEdit, SIGNAL(returnPressed()), SLOT(slotGo2Location()));
	mpLineEdit->setToolTip(tr("Apply"));

	mpApplyTbtn = new QToolButton(this);
	mpApplyTbtn->setAutoRaise(true);
	//?	connect(mpApplyTbtn, SIGNAL(clicked()), SLOT(slotGo2Location()));

	//mpRefreshTbtn = new QToolButton(this);
	//mpRefreshTbtn->setAutoRaise(true);

	QHBoxLayout * hbox(new QHBoxLayout);
	hbox->setSpacing(0);
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->addWidget(mpLineEdit);
	hbox->addWidget(mpApplyTbtn);

	setLayout(hbox);
}

void ADCInterDataWin::Toolbar::setEnabled(bool b)
{
	mpLineEdit->setEnabled(b);
	if (mpApplyTbtn->defaultAction())
		mpApplyTbtn->defaultAction()->setEnabled(b);
	else
		mpApplyTbtn->setEnabled(b);
}

void ADCInterDataWin::Toolbar::setTypePath(QString s)
{
	mpLineEdit->setText(s);
}

QString ADCInterDataWin::Toolbar::typePath() const
{
	return mpLineEdit->text();
}*/

////////////////////////////////////////////////
// ADCInterDataWin

ADCInterDataWin::ADCInterDataWin(QWidget * parent, const char *name)
	: ADCInterWin(parent, name),
	mpView(nullptr),
	mpModelData(nullptr)
{
	createView();

	//mpView->populateMenu(toolbarMenu());
	//setAutoFillBackground(true);
	//setBackgroundRole(QPalette::Light);
}

ADCInterDataWin::~ADCInterDataWin()
{
}

void ADCInterDataWin::createView()
{
	mpView = new ADCBinView(this);
	mpView->setExtraMargin(QSize(0, 0));
	mpView->setFrameStyle(QFrame::Panel | QFrame::Raised);
	mpView->setFont(font());
	mpView->show();

	setFocusProxy(mpView);

	connect(mpView, SIGNAL(signalRequestModel(ADCBinView &, QString)), SLOT(slotRequestModel(ADCBinView &, QString)));
	connect(mpView, SIGNAL(signalLocusInfo(QString, int)), SIGNAL(signalLocusInfo(QString, int)));
	connect(mpView, SIGNAL(signalRefreshBinaryDump(int)), SIGNAL(signalRefreshBinaryDump(int)));
	connect(mpView, SIGNAL(signalPostCommand(const QString &, bool)), SIGNAL(signalPostCommand(const QString &, bool)));
	connect(this, SIGNAL(signalModified()), mpView, SLOT(slotGlobalsModified()));

	QVBoxLayout *vbox(dynamic_cast<QVBoxLayout *>(layout()));
	vbox->addWidget(mpView);
}

void ADCInterDataWin::slotRequestModel(ADCBinView &, QString subjFullName)
{
	emit signalRequestModel(*this, subjFullName);
}

void ADCInterDataWin::takeModel()
{
	if (mpView)
		mpView->setModelData(nullptr);
}

void ADCInterDataWin::setModel(/*ADCModelDataMap &rm, */adcui::IBinViewModel *pIModel)
{
	if (!mModels.isEmpty())
	{
		if (mpView)
			mpView->setModelData(nullptr);
		mModels.begin().value()->Release();
	}
	Q_ASSERT(mModels.isEmpty());

	//takeModel();
	if (!pIModel)
	{
		setToolbarEnabled(false);
	}
	else
	{
		//createView();
		ADCDocPos *pCur(new ADCDocTablePos(pIModel));
		ADCModelData *pModelData(new ADCModelData(mModels, pIModel, nullptr, pCur));
		mModels.insert(QString(), pModelData);
		mpView->setModelData(pModelData);
		//pModelData->Release();
		setToolbarEnabled(true);

		toolbarMenu()->clear();
		mpView->populateMenu(toolbarMenu());
		toolbarMenu()->addSeparator();
		mpView->populateColumnsMenu(toolbarMenu(), nullptr);

		ADCStream ss;
		pIModel->scopeName(ss);//format: <module>!<typeFullName>

		ADCModelInfo data(QString(), ss.ReadString());
		data.swapNamePath();
		setToolbarData(data, true);//apply
	}
}

void ADCInterDataWin::updateOutputFont(const QFont &f)
{
	setFont(f);
	if (mpView)
		mpView->setFont(f);
//	if (mpToolbar)
	//	mpToolbar->setFont(f);
}

void ADCInterDataWin::applySubject()
{
	applySubject(getToolbarText());
}

void ADCInterDataWin::applySubject(QString s)
{
	emit signalRequestModel(*this, s);
	//updateContents();
	//if (hasModel())
		//return binModel();

	/*adcui::IBinViewModel *pIModel(mpView->reset2(s));
	if (pIModel)
	{
		ADCStream ss;
		pIModel->scopeName(ss);
		QString scope(ss.ReadString());
		QString path;//?
		setToolbarData(scope, path, true);//apply
	}*/
}

void ADCInterDataWin::slotApplySubject()
{
	applySubject();
	mpView->setFocus();
}

void ADCInterDataWin::slotReset(QString s)
{
	applySubject(s);
/*?	emit signalRequestModel(*this, s);
	if (mpToolbar)
		mpToolbar->setTypePath(s);*/
	//	mpView->setFocus();
}

void ADCInterDataWin::slotAboutToClose()
{
	setModel(nullptr);
	//takeModel();
/*?	mpToolbar->setTypePath("");
	mpToolbar->setEnabled(false);*/
}

/*void ADCInterDataWin::slotUpdateDataFromLocus()
{
	if (!isVisible())
		return;
	mpView->updateDataFromLocus(0);
}*/

void ADCInterDataWin::slotSyncPanesResponce2(int, bool bForce)
{
	if (!mpView->isVisible())
		return;
	if (mpView->hasFocus())//not from itself
		return;
	if (bForce)
		applySubject(getToolbarText());
	mpView->updateDataFromLocus(0);
}

void ADCInterDataWin::slotResetFromLocus(bool bForce)
{
	if (!bForce || !isVisible())
		return;
	emit signalRequestModel(*this, QString());
	//?	if (mpToolbar)
	//?	mpToolbar->setTypePath(s);
}

void ADCInterDataWin::slotCurInfoChanged(const ADCCurInfo &ci)
{
	if (!isVisible())
		return;
	ADCModelInfo data(ci.scope, ci.scopePath);
	if (!ci.obj.isEmpty())
		data = ADCModelInfo(ci.obj, ci.objPath);
	if (!data.dispText().isEmpty())
		updateToolbarState(data.dispText());
}


