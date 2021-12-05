#include <QTabWidget>
#include <QToolButton>
#include <QHeaderView>
#include <QTreeWidget>

#include "ADCUtils.h"
#include "ADCTabsWin.h"

///////////////////////////////////////////////////////////////////// ADCTabsWin
ADCTabsWin::ADCTabsWin(QWidget *parent, const char *name, Qt::Orientation ori)//, ADCWinToolbar *pToolbar)
//: ADCTabsWinBase(ori, parent)
: ADCTabsWinBase(parent)
//mpToolbar(pToolbar)
{
	(void)ori;
	setObjectName(name);
	setFrameShape(NoFrame);
	//setFrameShadow(Sunken);
	
	//mpToolbar = new ADCWinToolbar(this);

	//QTabWidget *pTest= new QTabWidget(this);
	//pTest->setStyleSheet("QTabWidget::pane { border: 0; }");
	mpTabWidget = new QTabWidget(this);
//mpTabWidget->hide();
#if(1)
	mpTabWidget->setTabPosition(QTabWidget::South);
#endif
	//mpTabWidget->setTabsClosable(true);
	mpTabWidget->setUsesScrollButtons(true);
#if(1)
	mpTabWidget->setTabShape(QTabWidget::Triangular);
#endif
	mpTabWidget->setMovable(true);
	//mpTabWidget->setTabBarAutoHide(true);

	// Close Tab Button
#ifdef OLD_CLOSE_BTN
	QToolButton *pCloseTabBtn(new QToolButton(mpTabWidget));
	//pCloseTabBtn->hide();
	pCloseTabBtn->setIcon(QIcon(":std_close_16.png"));
	pCloseTabBtn->setAutoRaise(true);
	pCloseTabBtn->setMaximumSize(16, 16);
	connect(pCloseTabBtn, SIGNAL(released()), SLOT(slotCloseCurrentView()));
	mpTabWidget->setCornerWidget(pCloseTabBtn, Qt::BottomRightCorner);
#else
	mpTabWidget->setTabsClosable(true);
#endif

	connect(mpTabWidget, SIGNAL(tabCloseRequested(int)), SLOT(slotCloseTab(int)));
	connect(mpTabWidget, SIGNAL(currentChanged(int)), SLOT(slotCurrentChanged(int)));

	QVBoxLayout * vbox(new QVBoxLayout(this));
	vbox->setSpacing(0);
	vbox->setContentsMargins(0, 0, 0, 0);
	//if (mpToolbar)
	//vbox->addWidget(mpToolbar);
	vbox->addWidget(mpTabWidget);
	//vbox->addWidget(pTest);

	setStyleSheet("QTabWidget::pane { border: none; }");
	//setStyleSheet("QTabWidget::pane{ border: 2px solid #C2C7CB; }");
	setStyleSheet("QTabWidget::pane{ border: 1px solid white; }");

	//setStyleSheet("QTabWidget::pane{ border - top: 1px solid #C2C7CB; }");
}

void ADCTabsWin::setToolbar(ADCWinToolbar *pToolbar)
{
	mpToolbar = pToolbar;
	QVBoxLayout *l(static_cast<QVBoxLayout *>(layout()));
	l->insertWidget(0, pToolbar);
}

void ADCTabsWin::slotCurrentChanged(int)
{
	onCurrentChanged();
}

void ADCTabsWin::closeEvent(QCloseEvent *e)
{
	//depopulate();
	ADCTabsWinBase::closeEvent(e);
}

void ADCTabsWin::depopulate()
{
	while (mpTabWidget->count() > 0)
	{
		QWidget *pView(mpTabWidget->widget(0));
		mpTabWidget->removeTab(0);
		delete pView;
	}
}

void ADCTabsWin::slotCloseTab(int i)
{
	QWidget *pView(mpTabWidget->widget(i));
	mpTabWidget->removeTab(i);
	delete pView;

	if (mpTabWidget->count() == 0)
		emit signalClosePage();
}

void ADCTabsWin::slotCloseCurrentView()
{
	slotCloseTab(mpTabWidget->currentIndex());
}

void ADCTabsWin::slotOpenView(QString path)
{
	QString module(path);

	int n(module.indexOf(MODULE_SEP));
	if (n == -1)
		return;
	module.truncate(n);

	int i(0);
	for (; i < mpTabWidget->count(); i++)
	{
		if (mpTabWidget->tabText(i) == module)
		{
			mpTabWidget->setCurrentIndex(i);
			return;
		}
	}

	QWidget *pView(createView(path));

	i = mpTabWidget->addTab(pView, QIcon(toIconStr(module)), module);
	mpTabWidget->setCurrentIndex(i);
}

void ADCTabsWin::updateOutputFont(const QFont &f)
{
	setFont(f);
	for (int i(0); i < mpTabWidget->count(); i++)
	{
		QWidget* w(mpTabWidget->widget(i));
		w->setFont(f);
		QTreeView* p(dynamic_cast<QTreeView*>(w));
		if (p)
			p->header()->setFont(f);
	}

	//	if (mpView)
	//	mpView->setFont(f);
	//if (mpBinView)
	//mpBinView->setFont(f);
}


