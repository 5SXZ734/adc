#pragma once

#include "ADBInterWin.h"
#include "ADCTextView.h"

struct ADCCurInfo;
class ADCSourceView;

//class ADCSourceView;
namespace adcui { class ISrcViewModel; }

////////////////////////////// ADCInterCodeWin

class ADCInterCodeWin : public ADCInterWin
{
Q_OBJECT
public:
	ADCInterCodeWin(QWidget * parent, const char *);
	virtual ~ADCInterCodeWin(){}
	void setModel(adcui::ISrcViewModel *);

protected:
	void create();
	virtual void emitAllSignals(){}
	virtual void updateOutputFont(const QFont &);
	//virtual bool isClosable() { return false; }
	virtual void showEvent(QShowEvent *);
	virtual void changeEvent(QEvent *);

	virtual void applySubject(QString);

private:
	void applySubject();

protected slots:
	void slotReset();
	void slotCurInfoChanged(const ADCCurInfo &);
	void slotRequestModel(ADCSourceView &, QString);
	void slotSrcDumpInvalidated(QString);
	void slotSyncPanesResponce2(int, bool);
	void slotSaveListing(QString, QString);

signals:
	void signalRequestModel(ADCInterCodeWin &, QString);
	void signalSyncPanesResponce4(int, bool);
	void signalCaretPosChanged(int, int, const ADCCurInfo &);
	void signalContextIdChanged(unsigned);
	void signalContextIdInq(uint &);
	void signalPostCommand(const QString&, bool);
	void signalSaveListing3(QString, QString);
	void signalSyncPanesRequest3(int, bool);
	void signalQuickPrototype();

	//bogus ones to shut Qt debug messaging up
	void signalPageClosed();
	void signalClosePage();
	void signalFileSaved(QWidget *, const QString&);
	void signalHandlePageMenu(QWidget *, const QPoint &);

protected:
	ADCSourceView *	mpView;

	ADCModelDataMap	mModels;
	ADCModelData *mpModelData;
};



