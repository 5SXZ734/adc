#pragma once

#include "ADBMainWin.h"
#include "ADCProtoDlg.h"

class ADCSourceView;
class ADCSourceWin;
class ADCStubsWin;
class ADCSourceWin0;
class ADCStubsView;
class ADCTemplWin;
class ADCTemplView;
class ADCExprWin;
class ADCInterCodeWin;


class ADCMainWin : public ADBMainWin
{
	Q_OBJECT

protected:
	enum PAGEX_e
	{
		PAGEX_PROTOTYPES = PAGE_TOTAL,
		PAGEX_BLOCKS,
		PAGEX_EXPR,
		PAGEX_TEMPLATES,
		PAGEX_INTER,
		PAGEX_CUTLIST,
		//..
		PAGEX_TOTAL
	};

public:
	ADCMainWin(ADBResManager&, adcui::IADCCore&);
	adcui::IADCCore& ADC() { return mrADC; }

	static int DEFAULT_WINDOCKCONFIG()
	{
		return ADBMainWin::DEFAULT_WINDOCKCONFIG() | (1 << PAGEX_BLOCKS) | (1 << PAGEX_EXPR) | (1 << PAGEX_INTER);
	}

	ADCSourceWin *CreateSourceWin(ADCSourceWin0 *);
	DocumentObject* CreateSourceWin(QString, QString);
	DocumentObject* CreateStubsView(QString, QString);

protected:
	virtual int HandleEvent(int, void*) override;
	virtual void OnSrcDumpInvalidated(QString);
	virtual void OnDecompileFunction(QString, QString);
	//bool OnNoSourceContextRecoil(const char *);
	virtual void OnDebuggerStopped();
	virtual void OnProjectStatus(bool bActive);
	virtual void OnShowDocumentAtHint(DocumentObject*, QString);
	virtual void OnFileRenamed(QString, QString);
	virtual int closePage(QWidget*) override;

	void updateSplitPanesUI();
	ADCSourceWin *getCurrentSourceWin();

private:
	void CreateTemplatesWin();
	void CreateStubsWin();
	void CreateBlocksWin();
	void CreateExprWin();
	void CreateProtoEditDlg();
	void CreateInterWin();
	void CreateCutListWin();

private slots:
	void slotRequestModel(ADCSourceWin &, QString, int, int);
	void slotRequestModel(ADCInterCodeWin &, QString);
	void slotRequestModel(ADCStubsWin &);
	void slotRequestModel(ADCStubsView &, QString);
	void slotRequestModel(ADCExprWin &);
	void slotRequestModel(ADCTemplWin &, QString, bool);
	void slotRequestModel(ADCTemplView &, QString, bool);

	void slotShowTemplatesView();
	void slotShowStubsView();
	void slotShowBlocksView();
	void slotShowExprView();
	void slotDumpCutList(ADCStream&);
	void slotUncutItem(int);
	void slotSetProtoData(const ADCProtoEditDlg::Data &, bool);
	void slotShowCutsView();
	void slotShowInterView();
	void showProtoDlg(ADCStream &);
	void slotShowProtoDlg();
	void slotDumpBlocks(QString, ADCStream &);
	//void slotDumpExpr(ADCStream &, bool);
	//void slotDumpPtrExprList(ADCStream &);
	void slotJumpSourceBack();
	void slotJumpSourceBack(bool);
	void slotJumpSourceForward();
	void slotNoSourceContextRecoil(QString);
	void slotSyncSourcePanesRequest(int, bool);
	void slotCaretPosChanged(int, int, const ADCCurInfo &);
	void slotCompile();
	void slotCompileFile(QString);
	void slotSyncModeInquiry(bool &);
	void slotSyncModeToggled(bool);
	void slotToggleLeftPane();
	void slotToggleRightPane();

	virtual void showSourceWin(QString fileName, QString atHint, bool bAskToOpen);
	//virtual void slotShowSourceWin(QString, QString);
	virtual void slotActivateFile(QString);
	virtual void slotTabChanged(QWidget *);
	virtual void slotTabChanged(DocumentObject *);
	virtual void slotBinaryPanePicked(int nLine, bool bForce);

signals:
	void signalDcNew();
	void signalStubsModified();
	void signalExprInvalidated();
	void signalEditStub();
	void signalCurOpChanged();
	void signalOpenStubsView(QString);
	void signalSourceDumpPicked(bool);
	void signalSetProtoData(const ADCProtoEditDlg::Data &);
	void signalSyncModeToggled(bool);
	void signalSrcDumpInvalidated(QString);
	void signalCutListUpdated();

protected:
	adcui::IADCCore& mrADC;

	//Build
	QMenu	*mpBuildMenu;
	QAction *mpCompileAction;

	QAction*	mpGoBackAction;
	QAction*	mpGoForwardAction;

	QAction*	mpShowStubsAction;
	QAction*	mpShowBlocksAction;
	QAction*	mpShowExprAction;
	QAction*	mpShowCutsAction;
	QAction*	mpShowInterCodeViewAction;

	QAction*	mpSyncModeAction;
	QAction*	mpSplitPaneLeftAction;
	QAction*	mpSplitPaneRightAction;

	ADCProtoEditDlg* mpProtoDlg;
};

