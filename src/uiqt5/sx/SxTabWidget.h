#ifndef __SEDITEXTABWIDGET_H__
#define __SEDITEXTABWIDGET_H__

#include <QTabWidget>
#include <QTabBar>

class QMenu;

class SxTabBar : public QTabBar
{
Q_OBJECT
public:
	SxTabBar(QWidget *parent = 0, const char * name = 0);
	//virtual QTab *selectTab(const QPoint &) const;

protected:
	virtual void mousePressEvent(QMouseEvent *);
};

class SxTabWidget : public QTabWidget
{
Q_OBJECT
public:
	SxTabWidget( QWidget *parent = 0, const char * name = nullptr );

	void handleRightButton(const QPoint& pt);

signals:
	void signalHandlePageMenu(QWidget *, const QPoint&);
};

#endif//__SEDITEXTABWIDGET_H__
