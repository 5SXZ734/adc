#ifndef SMSPICECOMMLISTBOX_H
#define SMSPICECOMMLISTBOX_H

#include <QtCore/QMap>
#include <QListWidget>
#include <QLineEdit>

#include "SxCommandLine.h"

class QWidget;
class QLabel;
class QComboBox;
class QToolButton;
class QFont;
class QMenu;

class SeditexCommListBox : public QListWidget
{
Q_OBJECT
public:
    SeditexCommListBox(QWidget *pparent, const char *pname);
    virtual ~SeditexCommListBox();
	void addCommand(const QString &);

protected:
    virtual void dragEnterEvent(QDragEnterEvent * pevent);
    virtual void dragMoveEvent(QDragMoveEvent * pevent);
    virtual void dropEvent(QDropEvent * pevent);
    virtual void keyPressEvent(QKeyEvent * pevent);

protected slots:
	void slotContextMenuRequested(QListWidgetItem * item, const QPoint & pos);
signals:
	void signalCreatePopupMenuRequested(QMenu *);
};


////////////////////////////////////////////////


class SeditexCommandHistoryWin : public SeditexCommandWin
{
Q_OBJECT
public:
	SeditexCommandHistoryWin(QWidget * parent, const char *);
	virtual ~SeditexCommandHistoryWin();
protected:
	virtual void emitAllSignals(){}
	virtual void updateOutputFont(const QFont &);
public slots:
	void slotClear();
protected slots:
	void slotCreatePopupMenuRequested(QMenu *);
	void slotHighlighted(const QString &);
	void slotLogCommand(const QString &);

private:
	SeditexCommListBox *mpListBox;

};

#endif // !SMSPICECOMMLISTBOX_H

