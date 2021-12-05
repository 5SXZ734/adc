#pragma once

#include <QFrame>
#include "ADCTextView.h"
#include "ADBInterWin.h"

class QLineEdit;
class QToolButton;
class ADCBinView;
class ADCModelDataMap;

struct ADCCurInfo;
namespace adcui {
	class IBinViewModel;
}


class ADCInterDataWin : public ADCInterWin
{
Q_OBJECT
public:
	ADCInterDataWin(QWidget * parent, const char *name);
	virtual ~ADCInterDataWin();

	void createView();
	ADCBinView * view() const { return mpView; }
	void setModel(/*ADCModelDataMap &, */adcui::IBinViewModel *);
	void takeModel();

protected:
	virtual void emitAllSignals(){}
	virtual void updateOutputFont(const QFont &);

	void applySubject();
	void applySubject(QString);

public slots:
	void slotReset(QString);
	void slotAboutToClose();
	//void slotUpdateDataFromLocus();
	void slotSyncPanesResponce2(int, bool);
	void slotResetFromLocus(bool);
	void slotApplySubject();
	void slotCurInfoChanged(const ADCCurInfo &);

private slots:
	void slotRequestModel(ADCBinView &, QString);

signals:
	void signalRequestModel(ADCInterDataWin &, QString);
	void signalRefreshBinaryDump(int);
	void signalLocusInfo(QString, int);
	void signalPostCommand(const QString &, bool);
	void signalModified();

	//bogus ones to shut Qt debug messaging up
	void signalPageClosed();
	void signalClosePage();
	void signalFileSaved(QWidget *, const QString&);
	void signalHandlePageMenu(QWidget *, const QPoint &);

protected:
	ADCBinView		*mpView;
	
	ADCModelDataMap	mModels;
	ADCModelData *mpModelData;
};



