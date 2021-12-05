#include "ADCCodeWin.h"
#include <QLineEdit>
#include <QToolButton>
#include <QAction>
#include <QKeyEvent>
#include <QMenu>
#include "ADCSourceWin.h"
#include "ADCUtils.h"


//////////////////////////////////////////////////// ADCInterCodeWin

ADCInterCodeWin::ADCInterCodeWin(QWidget * parent, const char *name)
	: ADCInterWin(parent, name),
	mpView(nullptr),
	mpModelData(nullptr)
{
	create();
}

void ADCInterCodeWin::create()
{
	//mpView = new QFrame(this);
	mpView = new ADCSourceView(this);
	mpView->setFont(font());
	//mpView->setModel(nullptr);
	//mpView->setFrameShape(QFrame::StyledPanel);
	mpView->setFrameShape(QFrame::Panel);
	mpView->setFrameShadow(QFrame::Raised);
	connect(mpView, SIGNAL(signalRequestModel(ADCSourceView &, QString)), SLOT(slotRequestModel(ADCSourceView &, QString)));
	connect(mpView, SIGNAL(signalCaretPosChanged(int, int, const ADCCurInfo &)), SIGNAL(signalCaretPosChanged(int, int, const ADCCurInfo &)));
	connect(mpView, SIGNAL(signalContextIdInq(uint &)), SIGNAL(signalContextIdInq(uint &)));
	connect(mpView, SIGNAL(signalPostCommand(const QString &, bool)), SIGNAL(signalPostCommand(const QString &, bool)));
	connect(mpView, SIGNAL(signalSaveListing1(QString, QString)), SLOT(slotSaveListing(QString, QString)));
	connect(mpView, SIGNAL(signalSynchronize(int, bool)), SIGNAL(signalSyncPanesRequest3(int, bool)));
	connect(mpView, SIGNAL(signalQuickPrototype()), SIGNAL(signalQuickPrototype()));
	connect(this, SIGNAL(signalSyncPanesResponce4(int, bool)), mpView, SLOT(slotSyncPanesResponce3(int, bool)));
	connect(this, SIGNAL(signalContextIdChanged(unsigned)), mpView, SLOT(slotContextIdChanged(unsigned)));

	mpView->populateToolbarMenuUnfold(toolbarMenu());

	QVBoxLayout *vbox(dynamic_cast<QVBoxLayout *>(layout()));
	vbox->addWidget(mpView);
}

void ADCInterCodeWin::slotRequestModel(ADCSourceView &, QString subjFullName)
{
	emit signalRequestModel(*this, subjFullName);
}

void ADCInterCodeWin::applySubject()
{
	applySubject(getToolbarText());
	mpView->setFocus();
}

void ADCInterCodeWin::applySubject(QString s)
{
	adcui::ISrcViewModel *pIModel(mpView->reset1(s));
	if (pIModel)
		setToolbarData(pIModel->subjectName(), pIModel->filePath(), true);//and update
}

void ADCInterCodeWin::slotSyncPanesResponce2(int iLine, bool bForce)
{
	if (mpView->isVisible())
	{
		if (!mpView->hasFocus())//not from itself
		{
			if (bForce)
				applySubject(getToolbarText());
			emit signalSyncPanesResponce4(iLine, bForce);
		}
	}
}

void ADCInterCodeWin::setModel(adcui::ISrcViewModel *pIModel)
{
	if (!mModels.isEmpty())
		mModels.begin().value()->Release();
	Q_ASSERT(mModels.isEmpty());
	if (!pIModel)
	{
		mpView->setModelData(nullptr);
		setToolbarEnabled(false);
		//mpToolbar->deleteLater();
		//mpView->deleteLater();
		//mpView = nullptr;
		//mpToolbar = nullptr;
		//create();//re-create
		//mpView->deleteContents();
	}
	else
	{
		ADCDocPos *pCur(new ADCDocTablePos(pIModel));
		ADCModelData *pModelData(new ADCModelData(mModels, pIModel, nullptr, pCur));
		mModels.insert(QString(), pModelData);
		mpView->setModelData(pModelData);
		//setModelDataAt(0, pModelData);
		//pModelData->Release();//1 ref remains
		setToolbarEnabled(true);
		setToolbarData(pIModel->subjectName(), pIModel->filePath(), true);//and update/apply
#if(0)
		static int z(0);
		fprintf(stdout, "Inter Code view refreshed (%d)\n", ++z);
		fflush(stdout);
#endif
	}
}

void ADCInterCodeWin::updateOutputFont(const QFont &f)
{
	setFont(f);
	if (mpView)
		mpView->setFont(f);
/*	if (mpToolbar)
		mpToolbar->setFont(f);*/
}

void ADCInterCodeWin::showEvent(QShowEvent *e)
{
	ADCInterWin::showEvent(e);
}

void ADCInterCodeWin::changeEvent(QEvent *e)
{
	ADCInterWin::changeEvent(e);
}

void ADCInterCodeWin::slotReset()
{
	//mpView->reset1(QString());
	setModel(nullptr);
}

void ADCInterCodeWin::slotCurInfoChanged(const ADCCurInfo &ci)
{
	if (!isVisible())
		return;
	//if (mpToolbar->globName() == sName)
		//if (mpToolbar->globFullPath() == sPath)
			//return;
	//mpToolbar->setModelData(sName, sPath);
	ADCModelInfo data(ci.scope, ci.scopePath);
	if (!ci.obj.isEmpty())
		data = ADCModelInfo(ci.obj, ci.objPath);
	if (!data.dispText().isEmpty())
		updateToolbarState(data.dispText());
	//if (!sName.isEmpty() && !sPath.isEmpty())
		//mpView->reset(sName, sPath);
}

void ADCInterCodeWin::slotSrcDumpInvalidated(QString filePath)
{
	ADCModelInfo data(QString(), filePath);

	QString stem1(data.dispTextStem());
	QString stem2(dispTextStem());
	if (stem1 == stem2)
		mpView->slotRedump();
	//mpView->updateContents();//it could have been this pane that triggered the re-dump, so it must be given a chance to re-validate itself.
}

void ADCInterCodeWin::slotSaveListing(QString path, QString options)
{
	emit signalSaveListing3(path, options);
}







