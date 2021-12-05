#pragma once

#include <QString>
#include <QFrame>

#include "interface/IADCGui.h"

class QMenu;
class QLineEdit;
class QToolButton;

////////////////////////////// ADCModelInfo
class ADCModelInfo
{
	QString	name;
	QString	relPath;//no module
	QString	module;
public:
	ADCModelInfo(){}
	ADCModelInfo(QString n, QString p)
		: name(n)
	{
		setPath(p);
	}
	QString dispText() const { return module + name; }
	QString dispTextStem() const { return module + stem(); }
	bool isEmpty() const { return module.isEmpty() && name.isEmpty(); }
	void setName(QString n){ name = n; }
	void setPath(QString p)
	{
		int i(p.indexOf(MODULE_SEP));
		module = i > 0 ? p.left(i + 1) : "";//include MODULE_SEP
		relPath = i > 0 ? p.mid(i + 1) : p;
	}
	QString stem() const {
		return relPath.left(relPath.lastIndexOf(QChar('.')));
	}
	void swapNamePath(){
		qSwap(name, relPath);
	}
};

////////////////////////////// ADCInterWinToolbar
class ADCInterWinToolbar : public QFrame
{
	Q_OBJECT
public:
	ADCInterWinToolbar(QWidget *parent);
	void setModelData(const ADCModelInfo &a){ mData = a; }
	void setModelData(QString subjName, QString filePath){
		mData.setName(subjName);
		mData.setPath(filePath);
	}
	void updateState(QString);
	void updateState();
	void setEnabled(bool);
	//QString globName() const { return mData.name; }
	QString dispText() const { return mData.dispText(); }
	QString dispTextStem() const { return mData.dispTextStem(); }
	QLineEdit* lineEdit() const { return mpLineEdit; }
	QMenu*	popup() const { return mpPopup; }
private slots:
	void slotTextChanged();
signals:
	void signalApply();
private:
	QMenu*			mpPopup;
	QLineEdit*		mpLineEdit;
	QAction*		mpApplyAction;
	QToolButton*	mpApplyTbtn;
	QToolButton*	mpDropdownTbtn;
	ADCModelInfo	mData;
};


class ADCInterWin : public QWidget
{
Q_OBJECT
public:
	ADCInterWin(QWidget * parent, const char *);
	virtual ~ADCInterWin(){}

protected:
	virtual bool eventFilter(QObject *, QEvent *);

	QString getToolbarText() const;
	void setToolbarData(const ADCModelInfo &data, bool bUpdate)
	{
		mpToolbar->setModelData(data);
		if (bUpdate)
			mpToolbar->updateState();//apply
	}
	void setToolbarData(QString subjName, QString filePath, bool bUpdate)
	{
		mpToolbar->setModelData(subjName, filePath);
		if (bUpdate)
			mpToolbar->updateState();//apply
	}
	void setToolbarEnabled(bool);
	void updateToolbarState(QString s){ 
		mpToolbar->updateState(s);
	}
	QString dispText() const { return mpToolbar->dispText(); }
	QString dispTextStem() const { return mpToolbar->dispTextStem(); }

	virtual void applySubject(QString){}

	QMenu* toolbarMenu() const { return mpToolbar->popup(); }

protected slots:
	void slotApplySubject();
private:
	ADCInterWinToolbar *mpToolbar;
};



