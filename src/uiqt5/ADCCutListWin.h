#pragma once

#include <QWidget>

class QTreeWidget;
class ADCStream;

class ADCCutListWin : public QWidget
{
	Q_OBJECT
public:
	ADCCutListWin(QWidget * parent, const char *);
	virtual ~ADCCutListWin(){}

protected:
	virtual void emitAllSignals(){}
	virtual void updateOutputFont(const QFont&);
	virtual bool isClosable() { return false; }

public slots:
	void slotUpdate();
	void slotClean();

protected slots:
	void slotItemClicked(const QModelIndex &);
	void slotItemActivated(const QModelIndex &);
	void slotCustomContextMenuRequested(const QPoint&);
	void slotRemoveSelected();

signals:
	void signalItemClicked(QString);
	void signalItemActivated(QString);
	void signalDumpCutList(ADCStream&);
	void signalUncut(int);

	//bogus ones to shut Qt debug messaging up
	void signalPageClosed();
	void signalClosePage();
	void signalFileSaved(QWidget *, const QString&);
	void signalHandlePageMenu(QWidget *, const QPoint &);

protected:
	QTreeWidget*	mpView;
};


