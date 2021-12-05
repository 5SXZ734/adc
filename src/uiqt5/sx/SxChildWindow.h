#ifndef __SEDITEXCHILDWIN_H__
#define __SEDITEXCHILDWIN_H__

#include <QMdiSubWindow>

class SeditexChildWin: public QMdiSubWindow
{
Q_OBJECT
public:
    SeditexChildWin( QWidget* parent );
    virtual ~SeditexChildWin();

	virtual QWidget * getWidget(){
		return widget(); }

	void setWidget( QWidget * pWidget );

protected:
	virtual bool eventFilter(QObject * o, QEvent * e);
	virtual void closeEvent(QCloseEvent * e);
	virtual void mouseReleaseEvent( QMouseEvent * e );
	virtual void contextMenuEvent ( QContextMenuEvent * e );
	virtual void resizeEvent( QResizeEvent * e )
	{
		QMdiSubWindow::resizeEvent(e);
	}

	virtual QSize sizeHint() const { return QSize(640, 480); }

private:
	int canClose();

signals:
	void signalHandlePageMenu(QWidget *, const QPoint&);
};


#endif//__SEDITEXCHILDWIN_H__
