#pragma once

#include <QSplitter>

class QTabWidget;
class ADCWinToolbar;

typedef QFrame	ADCTabsWinBase;
//typedef QSplitter	ADCTabsWinBase;

/////////////////////////////////////////////////////////
class ADCTabsWin : public ADCTabsWinBase
{
Q_OBJECT
public:
	ADCTabsWin(QWidget *parent, const char *name, Qt::Orientation = Qt::Horizontal);//, ADCWinToolbar *pToolbar);
	//void init();

protected:
	virtual void closeEvent(QCloseEvent *);
	virtual QWidget* createView(QString) = 0;

protected:
	virtual void onCurrentChanged(){}
	virtual void emitAllSignals(){}
	virtual void updateOutputFont(const QFont &);
	void setToolbar(ADCWinToolbar *);

public slots:
	void depopulate();

private slots:
	void slotCloseTab(int);
	void slotCloseCurrentView();
	void slotOpenView(QString);
	void slotCurrentChanged(int);

signals:
	//bogus ones to shut Qt debug messaging up
	void signalPageClosed();
	void signalClosePage();
	void signalFileSaved(QWidget *, const QString&);
	void signalHandlePageMenu(QWidget *, const QPoint &);

protected:
	ADCWinToolbar*	mpToolbar;
	QTabWidget*		mpTabWidget;
};

