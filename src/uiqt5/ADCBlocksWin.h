#pragma once

#include <QtGui/QMouseEvent>
#include <QTreeWidget>
#include "sx/SxDocument.h"
#include "ADCCodeWin.h"

//class QListView;
class ADCStream;
struct ADCCurInfo;

class ADCBlocksWin : public ADCInterWin
{
	Q_OBJECT
public:
	ADCBlocksWin(QWidget * parent, const char *);
	virtual ~ADCBlocksWin(){}

protected:
	virtual void emitAllSignals(){}
	virtual void updateOutputFont(const QFont &);
	//virtual bool isClosable() { return false; }
	virtual void showEvent(QShowEvent *);

	virtual void applySubject(QString);

private:
	void populate(ADCStream &);
	void applySubject();

public slots:
	void slotReset();
	void slotCurInfoChanged(const ADCCurInfo &);
	void slotSyncPanesResponce2(int iLine, bool bForce);
private slots:
	void slotCurrentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *);
	void slotCollapseChildren();
	void slotExpandChildren();
	void slotCustomContextMenuRequested(const QPoint&);

signals:
	void signalDumpBlocks(QString, ADCStream &);

	//bogus ones to shut Qt debug messaging up
	void signalPageClosed();
	void signalClosePage();
	void signalFileSaved(QWidget *, const QString&);
	void signalHandlePageMenu(QWidget *, const QPoint &);

protected:
	QTreeWidget *	mpView;
};



