#ifndef __SEDITEXCOMMANDLINE_H__
#define __SEDITEXCOMMANDLINE_H__

#include <QLayout>
#include <QLineEdit>
#include <QLabel>

class SxCommandLineEdit;
class QComboBox;
class QLabel;
class QMenu;

class SxCommandLineEdit : public QLineEdit//, public DocumentObjectBase
{
	Q_OBJECT
public:
    SxCommandLineEdit(QWidget * pparent, const char * pname);
    virtual ~SxCommandLineEdit();

protected:
    virtual void dragEnterEvent(QDragEnterEvent * pevent);
    virtual void dragMoveEvent(QDragMoveEvent * pevent);
    virtual void dropEvent(QDropEvent * pevent);
	bool isPasteAvailable () const;

public slots:
    //void emitAllSignals();
	void slotTextChanged( const QString & );
	void slotSelectionChanged();

protected slots:
    void slotUndo();
    void slotRedo();
	void slotCopy();
	void slotCut();
	void slotPaste();
	void slotDelete();
	void slotSelectAll();
	void slotDeselect();

signals:
    void enableUndo( bool );
    void enableRedo( bool );
	void enableCopy( bool );
	void enableCut( bool );
	void enablePaste( bool );
	void enableDelete( bool );
};

class SxCommandLine : public QWidget
{
Q_OBJECT
public:
	SxCommandLine(QWidget *, const QString &, bool);
	virtual ~SxCommandLine();
	void updateOutputFont(const QFont &);
	void setCommandList(const QStringList &);
	void setFont(const QFont &);
	void setFocus(bool bSelectAll = false);
	bool hasFocus() const;
	QString getText();
	void setText(const QString &);
	void clear();
	void setPrompt(QString);
private:
	QLineEdit *lineEdit() const;
protected slots:
	void slotReturnPressed();
	void slotInputCbxActivated(const QString &);
	void slotRun();
signals:
	void signalCommand(const QString &);
	void signalClose();
protected:
	QLabel					mLblPix;
	QLabel					mLbl;
    QComboBox				*mpCBx;
    SxCommandLineEdit		*mpInputLed;
};

class SeditexCommandWin : public QWidget
{
Q_OBJECT
public:
	SeditexCommandWin(QWidget *, const char *, bool);
	virtual ~SeditexCommandWin();

	bool TurnCommandLineOn();
	bool TurnCommandLineOff(bool = false);
	bool EnableCommandLine(bool, const QString & = QString());
	bool TurnCommandLine(bool, bool bUnconditionally = true);
	bool IsCommandLineOn(){ return (mpCommandLine != nullptr); }

protected:
	virtual void focusInEvent(QFocusEvent * e);
	//void setClientWidget(QWidget *);
	void OnPopupMenu(QMenu *);
	void OnFontChange(const QFont &);

	void addWidget(QWidget *);

public slots:
	void slotSetPrompt(QString);
	void slotToggleCommandLine();
	void slotDisplayCommand(const QString &);
	void slotCmdListChanged(const QStringList&);

protected slots:
	void slotCloseCommandLine();
    void slotCommand();
    void slotCommand(const QString &);

signals:
	void signalCommand(const QString &, const QString &);
	void signalCommand(const QString &);
	void signalCmdLineToggled(bool);

private:
	//QWidget	*mpClient;
	QVBoxLayout *mpVBox;
	SxCommandLine *mpCommandLine;
	//bool	mbCLDisabled;
	QString	mSenderId;
	QString	mPrompt;
	int	mOutputId;
	bool	mbIns;
};

#endif//__SEDITEXCOMMANDLINE_H__
