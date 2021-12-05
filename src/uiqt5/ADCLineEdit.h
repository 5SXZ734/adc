#pragma once

#include <QtCore/qobject.h>

//#include "qx/MyStream.h"
#include "qx/MyString.h"

//#include "interface/IADCGui.h"
#include "ADCCell.h"

class ADCDocLine : public ADCTextRow0
{
protected:
	adcui::IADCTextModel *pIModel;
	adcui::DUMPOS	m_it;
public:
	ADCDocLine() : pIModel(nullptr){}//, yIt(0){}
	ADCDocLine(const ADCDocLine &o);
	ADCDocLine(adcui::IADCTextModel *_pIModel, adcui::DUMPOS copyFromIt = adcui::DUMPOS());
	~ADCDocLine();
	void setIter(adcui::DUMPOS it){ m_it = it; }
	adcui::DUMPOS iter() const { return m_it; }
	adcui::DUMPOS line() const { return m_it; }
	ADCTextRow0 &asTextRow(){ return *this; }
	bool sety(const ADCDocLine &o, int dy);//set line relative to other iterator
	void operator=(const ADCDocLine &o);
	//adcui::DUMPOS line() const { return yIt; }
	bool compareEq(adcui::DUMPOS pos) const;
	void copyFrom(ADCDocLine *p);
	bool operator==(const ADCDocLine &o) const;
	bool operator<(const ADCDocLine &o) const;
	bool operator>(const ADCDocLine &o) const;
	void operator++();
	void operator--();
	bool checkDistanceFrom(const ADCDocLine &o, int maxy, int &result);
	int shiftY(int dy);
	virtual bool shiftHome(){ return 0; }
	virtual bool shiftEnd(){ return 0; }
};


///////////////////////////////////////////////////ADCDocPos
class ADCDocPos : public ADCDocLine
{
protected:
	int mx;
public:
	ADCDocPos() : mx(0){}
	ADCDocPos(adcui::IADCTextModel *_pIModel, adcui::DUMPOS _lineIt = adcui::DUMPOS(), int x = 0);
	bool set(const ADCDocLine &o, int deltaY, int x);
	virtual bool setX(int x, bool bShift = false);
	bool shiftX(int deltaX, int minX = 0, int maxX = 1024);
	virtual bool shiftHome();
	int x() const { return mx; }
	const ADCDocLine &base() const { return *this; }
};



///////////////////////////////////////////////////ADCDocTablePos
class ADCDocTablePos : public ADCDocPos
{
public:
	ADCDocTablePos(){}
	ADCDocTablePos(adcui::IADCTextModel *_pIModel, adcui::DUMPOS _lineIt = adcui::DUMPOS(), int x = 0);
	/*adcui::IADCTableModel *tableModel() const {
		return dynamic_cast<adcui::IADCTableModel *>(pIModel);
	}*/
	virtual bool setX(int x, bool bShift);
	virtual bool shiftHome();
	virtual bool shiftEnd();
	adcui::IADCTableModel::COLID firstNonHeaderColumn();
};



class QAction;
class QMenu;

/////////////////////////////////////////////////////////ADCLineEdit
class ADCLineEdit : public QObject
{
Q_OBJECT
public:
	ADCLineEdit(QWidget *parent, adcui::IADCTextEdit *p, ADCDocPos *pAtPos);
	~ADCLineEdit();
	const ADCDocPos &origin() const { return *mpOrigin; }
	int startPosX() const { return mpOrigin->x(); }
	int curPos() const { return mCurPos; }
	int selBegPos() const { return mSelBegPos; }
	int selEndPos() const { return mSelEndPos; }
	void OnApply();
	int OnChar(char c);
	int OnBackspace();
	int OnDelete();
	int OnLeft(bool);
	int OnRight(bool);
	int OnHome(bool);
	int OnEnd(bool);
	bool isInside(adcui::DUMPOS y, int x, int &n) const;
	int OnClick(adcui::DUMPOS y, int x, bool bPreserveSel);
	int OnDoubleClick(adcui::DUMPOS y, int x);
	int OnMove(int x);
	void PopulateEditMenu(QMenu &);
private:
	int setCur(int);
	int setSel(int, int);
	int setData(QString);
	int deleteSel();
	int clearSel();
	bool hasSel() const;

public slots:
	void slotCut();
	void slotCopy();
	void slotPaste();
	void slotSelectAll();

signals:
	void signalSelectionChanged();
	void signalCurrentChanged();
	void signalDataChanged();

private:
	adcui::IADCTextEdit	*mpIEdit;
	QString	mData;
	ADCDocPos *mpOrigin;
	int mCurPos;
	int mSelBegPos;
	int mSelEndPos;

	QAction	*mpCopyAction;
	QAction	*mpCutAction;
	QAction	*mpPasteAction;
	QAction *mpSelectAllAction;
};

