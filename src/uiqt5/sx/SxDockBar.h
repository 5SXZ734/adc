#ifndef __SXDOCKBAR_H_INCLUDED__
#define __SXDOCKBAR_H_INCLUDED__

#include <QTabWidget>
#include <QMenu>

#include "SxTabWidget.h"

class QTabWidget;
class QToolButton;

// -------------------------------------------------------------------------
class SxTabWidgetEx : public SxTabWidget
{
Q_OBJECT
public:
	SxTabWidgetEx(QWidget *parent, const char *name = nullptr);
	int showTab(QWidget *, int);
	int hideTab(QWidget *);
	bool contains(QWidget *);

protected:
	virtual void emitAllSignals(){}
	virtual void updateOutputFont(const QFont &){}

protected slots:
	void slotClosePage();
	void slotTabChanged(int);

signals:
	void signalClosePage();
	void signalPageOpened();
	void signalPageClosed();

private:
	QToolButton *mpCloseTabBtn;
};

#endif//__SXDOCKBAR_H_INCLUDED__

