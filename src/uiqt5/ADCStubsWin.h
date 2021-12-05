#pragma once

#include <QFrame>
#include "ADCTableView.h"
#include "ADCTabsWin.h"

typedef ADCTableView	ADCStubsViewBase;

class ADCStubsView : public ADCStubsViewBase
{
Q_OBJECT
	class ModelLocker
	{
		//ADCStubsView *mpView;
		adcui::IStubsViewModel *mpModel;
	public:
		ModelLocker(ADCStubsView *pView)
			: mpModel(pView->stubsModel())
		{
			if (mpModel)
			{
				mpModel->AddRef();
				mpModel->lockRead(true);
			}
		}
		~ModelLocker()
		{
			if (mpModel)
			{
				mpModel->lockRead(false);
				mpModel->Release();
			}
		}
		adcui::IStubsViewModel* model() const { return mpModel; }
	};
public:
	ADCStubsView(QWidget *);
	virtual ~ADCStubsView();
	friend class ADCStubsWin;
	//void setModel(ADCModelData *);//adcui::IStubsViewModel *pIModel);
	void reset();
	void reload();
	void editStub();
	void highlightStub(QString);
	void onAnalysisStarted();
	void onSelFuncChanged();
	void setModel(ADCModelDataMap &, adcui::IStubsViewModel *);
protected:
	virtual void closeEvent(QCloseEvent *);
	virtual void paintEvent(QPaintEvent *);
	virtual void mouseDoubleClickEvent(QMouseEvent *);
	virtual void showEvent(QShowEvent *e);
	virtual int DrawMarginCell(QPainter &, const QRect &rc2, const ADCCell &);
	virtual bool checkAutoexpand();
	virtual void OnContentsChanged();
	virtual void OnNewRow(ADCTextRow &);
private:
	virtual bool hasModel() const { return mpModelData != nullptr; }
	adcui::IStubsViewModel *stubsModel() const { return static_cast<adcui::IStubsViewModel *>(modelData().pIModel); }
	virtual ADCModelData &modelData() const { return *mpModelData; }
signals:
	void signalGoToLocation(QString);
private:
	//adcui::IStubsViewModel *mpIModel;
	ADCModelData *mpModelData;
};

//typedef QFrame	ADCStubsWinBase;
class ADCStubsWin : public ADCTabsWin//ADCStubsWinBase
{
Q_OBJECT
public:
	ADCStubsWin(QWidget *parent, const char *name);
	virtual ~ADCStubsWin();
	void setModel(ADCModelDataMap &, adcui::IStubsViewModel *pIModel);
public slots:
	void slotProjectNew();
	void slotReset();
	void slotUpdate();
	void slotEditStub();
	//void slotHighlightLocationAt(QString, QString, QString, int);
	void slotGoToTaskTop();
	void slotAnalysisStarted();
	void slotSelFuncChanged();
	void slotAboutToClose();
	void slotDcNew();
private slots:
	void slotSaveAll();
	void slotReload();
	//void slotProjectClosed();
protected:
	virtual void showEvent(QShowEvent *);
	void createView();
	void destroyView();
	virtual QWidget* createView(QString);

private:
	ADCStubsView* currentView();

signals:
	void signalGoToLocation(QString);
	void signalRequestModel(ADCStubsWin &);
	void signalRequestModel(ADCStubsView &, QString);
	//void signalReload();

	//bogus ones to shut Qt debug messaging up
	/*void signalPageClosed();
	void signalClosePage();
	void signalFileSaved(QWidget *, const QString&);
	void signalHandlePageMenu(QWidget *, const QPoint &);*/

protected:
	virtual void emitAllSignals(){}
	virtual void updateOutputFont(const QFont &);
private:
	ADCStubsView *mpView;
	//ADCModelData *mpModelData;
};
