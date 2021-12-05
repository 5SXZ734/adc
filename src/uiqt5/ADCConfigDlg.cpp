#include "ADCConfigDlg.h"
#include "ADCUserPrefs.h"
#include "ADCApp.h"

#include <QtWidgets>


///////////////////////////////////////////////////////////////////////// (font)

ADCConfigFontPage::ADCConfigFontPage(QWidget* parent)
	: QFontDialog(parent)
{
	setOption(NoButtons, true);
	setOption(DontUseNativeDialog, true);
	setOption(MonospacedFonts, true);
	setSizeGripEnabled(false);
}

void ADCConfigFontPage::update(const ADBResManager& rm)
{
	setCurrentFont(rm.getOutputFont());	
}

void ADCConfigFontPage::apply(ADBResManager& rm)
{
	rm.setOutputFont(currentFont());
}




/////////////////////////////////////////////////// (analysis)

ADCConfigAnalysisPage::ADCConfigAnalysisPage(QWidget* parent)
	: QWidget(parent)
{
	QGroupBox* binaryGroup = new QGroupBox(tr("Binary Analysis"));

	QLabel* serverLabel = new QLabel(tr("Procedure call depth:"));
	serverCombo = new QComboBox;
	serverCombo->addItem(tr("Unlimited"));
	serverCombo->addItem(tr("Minimal"));
	serverCombo->addItem(tr("Custom"));

//	QSpinBox* callDepth = new QSpinBox;
//	callDepth->setRange(1, 10);

	QHBoxLayout* serverLayout = new QHBoxLayout;
	serverLayout->addWidget(serverLabel);
//	serverLayout->addWidget(callDepth);
	serverLayout->addWidget(serverCombo);

	QVBoxLayout* configLayout = new QVBoxLayout;
	configLayout->addLayout(serverLayout);
	binaryGroup->setLayout(configLayout);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(binaryGroup);
	mainLayout->addStretch(1);
	setLayout(mainLayout);
}

void ADCConfigAnalysisPage::update(const ADBResManager& rm)
{
	if (rm.getCallDepth() < 0)
		serverCombo->setCurrentText(tr("Unlimited"));
	else if (rm.getCallDepth() == 0)
		serverCombo->setCurrentText(tr("Minimal"));
	else
		serverCombo->setCurrentText(tr("Custom"));
}


void ADCConfigAnalysisPage::apply(ADBResManager& rm)
{
	if (serverCombo->currentText() == "Unlimited")
		rm.setCallDepth(-1);
	else if (serverCombo->currentText() == "Minimal")
		rm.setCallDepth(0);
	else
		rm.setCallDepth(INT_MAX);
}



/////////////////////////////////////////////////// 

UpdatePage::UpdatePage(QWidget* parent)
	: QWidget(parent)
{
	QGroupBox* updateGroup = new QGroupBox(tr("Package selection"));
	QCheckBox* systemCheckBox = new QCheckBox(tr("Update system"));
	QCheckBox* appsCheckBox = new QCheckBox(tr("Update applications"));
	QCheckBox* docsCheckBox = new QCheckBox(tr("Update documentation"));

	QGroupBox* packageGroup = new QGroupBox(tr("Existing packages"));

	QListWidget* packageList = new QListWidget;
	QListWidgetItem* qtItem = new QListWidgetItem(packageList);
	qtItem->setText(tr("Qt"));
	QListWidgetItem* qsaItem = new QListWidgetItem(packageList);
	qsaItem->setText(tr("QSA"));
	QListWidgetItem* teamBuilderItem = new QListWidgetItem(packageList);
	teamBuilderItem->setText(tr("Teambuilder"));

	QPushButton* startUpdateButton = new QPushButton(tr("Start update"));

	QVBoxLayout* updateLayout = new QVBoxLayout;
	updateLayout->addWidget(systemCheckBox);
	updateLayout->addWidget(appsCheckBox);
	updateLayout->addWidget(docsCheckBox);
	updateGroup->setLayout(updateLayout);

	QVBoxLayout* packageLayout = new QVBoxLayout;
	packageLayout->addWidget(packageList);
	packageGroup->setLayout(packageLayout);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(updateGroup);
	mainLayout->addWidget(packageGroup);
	mainLayout->addSpacing(12);
	mainLayout->addWidget(startUpdateButton);
	mainLayout->addStretch(1);
	setLayout(mainLayout);
}

void UpdatePage::update(const ADBResManager&)
{
}

void UpdatePage::apply(ADBResManager&)
{
}

// //////////////////////////////////////////////////////////////

QueryPage::QueryPage(QWidget* parent)
	: QWidget(parent)
{
	QGroupBox* packagesGroup = new QGroupBox(tr("Look for packages"));

	QLabel* nameLabel = new QLabel(tr("Name:"));
	QLineEdit* nameEdit = new QLineEdit;

	QLabel* dateLabel = new QLabel(tr("Released after:"));
	QDateTimeEdit* dateEdit = new QDateTimeEdit(QDate::currentDate());

	QCheckBox* releasesCheckBox = new QCheckBox(tr("Releases"));
	QCheckBox* upgradesCheckBox = new QCheckBox(tr("Upgrades"));

	QSpinBox* hitsSpinBox = new QSpinBox;
	hitsSpinBox->setPrefix(tr("Return up to "));
	hitsSpinBox->setSuffix(tr(" results"));
	hitsSpinBox->setSpecialValueText(tr("Return only the first result"));
	hitsSpinBox->setMinimum(1);
	hitsSpinBox->setMaximum(100);
	hitsSpinBox->setSingleStep(10);

	QPushButton* startQueryButton = new QPushButton(tr("Start query"));

	QGridLayout* packagesLayout = new QGridLayout;
	packagesLayout->addWidget(nameLabel, 0, 0);
	packagesLayout->addWidget(nameEdit, 0, 1);
	packagesLayout->addWidget(dateLabel, 1, 0);
	packagesLayout->addWidget(dateEdit, 1, 1);
	packagesLayout->addWidget(releasesCheckBox, 2, 0);
	packagesLayout->addWidget(upgradesCheckBox, 3, 0);
	packagesLayout->addWidget(hitsSpinBox, 4, 0, 1, 2);
	packagesGroup->setLayout(packagesLayout);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(packagesGroup);
	mainLayout->addSpacing(12);
	mainLayout->addWidget(startQueryButton);
	mainLayout->addStretch(1);
	setLayout(mainLayout);
}

void QueryPage::update(const ADBResManager&)
{
}

void QueryPage::apply(ADBResManager&)
{
}



//****************************************************** (ADCConfigDlg)

ADCConfigDlg::ADCConfigDlg(ADBResManager& rm, QWidget* parent)
	: QDialog(parent),
	mrResMgr(rm)
{
	setObjectName(QStringLiteral("ADCConfigDlg"));
	setModal(false);
	setWindowIcon(QIcon(":options2_24.png"));
	setWindowTitle(tr("Config Dialog"));

	contentsWidget = new QListWidget;
	contentsWidget->setViewMode(QListView::ListMode);//IconMode
//	contentsWidget->setIconSize(QSize(96, 84));
//	contentsWidget->setMovement(QListView::Static);
	//contentsWidget->setMaximumWidth(128);
	contentsWidget->setSpacing(6);

	pagesWidget = new QStackedWidget;
	addWidgetAndConnect(new ADCConfigFontPage(this));
	addWidgetAndConnect(new ADCConfigAnalysisPage(this));
	addWidgetAndConnect(new UpdatePage(this));
	addWidgetAndConnect(new QueryPage(this));

	QPushButton* okButton = new QPushButton(tr("OK"));
	QPushButton* closeButton = new QPushButton(tr("Close"));
	QPushButton* applyButton = new QPushButton(tr("Apply"));

	okButton->setDefault(true);

	createIcons();
	contentsWidget->setCurrentRow(0);

	connect(okButton, SIGNAL(clicked()), SLOT(accept()));
	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(applyButton, SIGNAL(clicked()), SLOT(apply()));

	QSplitter* splitter = new QSplitter(Qt::Horizontal);
	splitter->setChildrenCollapsible(false);
	splitter->addWidget(contentsWidget);
	splitter->addWidget(pagesWidget);

	QHBoxLayout* horizontalLayout = new QHBoxLayout;
	horizontalLayout->addWidget(splitter);
	//horizontalLayout->addWidget(contentsWidget);
	//horizontalLayout->addWidget(pagesWidget, 1);

	QHBoxLayout* buttonsLayout = new QHBoxLayout;
	buttonsLayout->addStretch(1);
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(closeButton);
	buttonsLayout->addWidget(applyButton);

	QFrame* line2 = new QFrame;
	line2->setFrameShape(QFrame::HLine);
	line2->setFrameShadow(QFrame::Sunken);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->addLayout(horizontalLayout);
	//mainLayout->addStretch(1);
	//mainLayout->addSpacing(12);
	mainLayout->addWidget(line2);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);

	update();
}

void ADCConfigDlg::addWidgetAndConnect(QWidget* widget)
{
	pagesWidget->addWidget(widget);
	connect(this, SIGNAL(signalUpdate(const ADBResManager&)), widget, SLOT(update(const ADBResManager&)));
	connect(this, SIGNAL(signalApply(ADBResManager&)), widget, SLOT(apply(ADBResManager&)));

}

void ADCConfigDlg::createIcons()
{
	QListWidgetItem* fontButton = new QListWidgetItem(contentsWidget);
	fontButton->setIcon(QIcon(":fonts_64.png"));
	fontButton->setText(tr("Font"));
//	fontButton->setTextAlignment(Qt::AlignHCenter);
	fontButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	QListWidgetItem* configButton = new QListWidgetItem(contentsWidget);
	configButton->setIcon(QIcon(":analysis_32.png"));
	configButton->setText(tr("Analysis"));
//	configButton->setTextAlignment(Qt::AlignHCenter);
	configButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	QListWidgetItem* updateButton = new QListWidgetItem(contentsWidget);
	updateButton->setIcon(QIcon(":/images/update.png"));
	updateButton->setText(tr("Update"));
//	updateButton->setTextAlignment(Qt::AlignHCenter);
	updateButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	QListWidgetItem* queryButton = new QListWidgetItem(contentsWidget);
	queryButton->setIcon(QIcon(":/images/query.png"));
	queryButton->setText(tr("Query"));
//	queryButton->setTextAlignment(Qt::AlignHCenter);
	queryButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	connect(contentsWidget, &QListWidget::currentItemChanged, this, &ADCConfigDlg::changePage);
}

/*void ADCConfigDlg::setData(const ADCConfigDlg::Data& aData)
{
}

void ADCConfigDlg::getData(ADCConfigDlg::Data& aData)
{
}*/


void ADCConfigDlg::changePage(QListWidgetItem* current, QListWidgetItem* previous)
{
	if (!current)
		current = previous;

	pagesWidget->setCurrentIndex(contentsWidget->row(current));
}

void ADCConfigDlg::accept()
{
	apply();
	close();
}

void ADCConfigDlg::apply()
{
	emit signalApply(mrResMgr);//pages => resmgr 
	emit signalPostApply();//notify a client
}

void ADCConfigDlg::update()
{
	emit signalUpdate(mrResMgr);
}






