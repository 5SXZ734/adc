#pragma once

#include <QDialog>
#include <QFontDialog>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class ADBResManager;
class QComboBox;

class ADCConfigFontPage : public QFontDialog
{
	Q_OBJECT
public:
	ADCConfigFontPage(QWidget* parent = 0);
public slots:
	void update(const ADBResManager&);
	void apply(ADBResManager&);
};

struct ADCConfigAnalysisData
{
	int callDepth;
	ADCConfigAnalysisData()
		: callDepth(1)
	{}
};

class ADCConfigAnalysisPage : public QWidget
{
	Q_OBJECT
public:
	ADCConfigAnalysisPage(QWidget* parent = 0);
public slots:
	void update(const ADBResManager&);
	void apply(ADBResManager&);
private:
	QComboBox* serverCombo;
};

class QueryPage : public QWidget
{
	Q_OBJECT
public:
	QueryPage(QWidget* parent = 0);
public slots:
	void update(const ADBResManager&);
	void apply(ADBResManager&);
};

class UpdatePage : public QWidget
{
	Q_OBJECT
public:
	UpdatePage(QWidget* parent = 0);
public slots:
	void update(const ADBResManager&);
	void apply(ADBResManager&);
};

//********************************** (ADCConfigDlg)
class ADCConfigDlg : public QDialog
{
	Q_OBJECT
public:
	ADCConfigDlg(ADBResManager&, QWidget*);
	~ADCConfigDlg() {}

public slots:
	void changePage(QListWidgetItem* current, QListWidgetItem* previous);

protected slots:
	void accept();
	void apply();
	void update();

signals:
	void signalApply(ADBResManager&);		//pages => resmgr
	void signalPostApply();	//to the caller
	void signalUpdate(const ADBResManager&);		//resmgr => pages

private:
	void createIcons();
	void addWidgetAndConnect(QWidget*);

private:
	ADBResManager& mrResMgr;
	QListWidget* contentsWidget;
	QStackedWidget* pagesWidget;

	ADCConfigAnalysisData mData;
};



