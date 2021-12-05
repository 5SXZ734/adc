#ifndef SMSPICEOUTPUTTEXTEDIT_H
#define SMSPICEOUTPUTTEXTEDIT_H

#include <QTextEdit>
#include <QtCore/QPair>

#include "SxFindTextDlg.h"
#include "SxCommandWin.h"

class QMenu;
class QSilPageSetup;
class QSplitter;
class SxFindTextDlg;
class SeditexTextEdit;

class SeditexTextEdit : public QTextEdit
{
Q_OBJECT
public:
	friend class SeditexPrinter;

	class FindTextObject : public SxFindTextDlg::ITarget
	{
		SeditexTextEdit* mpTarget;
	public:
		FindTextObject(SeditexTextEdit*);
		virtual bool findIt(
			const QString& expr,
			bool cs, //case sensitive
			bool wo, //whole words
			bool forward, //forward/reverse
			bool reset
		) const override;
		//virtual int type(){ return 0; }
	};

	class MyColorTags : public QMap<QString, QPair<QString, QString> >
	{
	public:
		MyColorTags();
		void toStringList(QStringList &);
		QStringList &tagList(){ return lAllTags; }
	private:
		void updateTagList();
		QStringList lAllTags;
	};

    SeditexTextEdit(QWidget *pparent, const char *pname);
    virtual ~SeditexTextEdit();

	static const char * GetColorTag(const char *, bool);
	static QStringList &GetFontColorTagList();
	static void WriteColorTags(QStringList &);

	void setClearable(bool b) { mbClearable = b; }
	void setAutoScroll(bool b) { mbAutoScroll = b; }
	void setSavable(bool b) { mbSaveToFileSupport = b; }
	void setChevrons(bool b){ mbCheckChevrons = b; }

    bool getTextHtml(QString &) const;

	void EmitSignals();
	virtual void setFont(const QFont &);
    virtual void append(const QString & rtext);

	void setLineRef(int n){ mLineRef = n; }
	int outputId(){ return mOutputId; }

	void setUseColors(bool b){ mbCheckChevrons = b; }

protected:
	virtual QSize sizeHint() const;
    QString & checkChevrons( QString & qsSource );

    virtual void contentsDragEnterEvent(QDragEnterEvent * pevent);
    virtual void contentsDragMoveEvent(QDragMoveEvent * pevent);
    virtual void contentsDropEvent(QDropEvent * pevent);
    virtual void dragEnterEvent(QDragEnterEvent * pevent);
    virtual void dragMoveEvent(QDragMoveEvent * pevent);
    virtual void dropEvent(QDropEvent * pevent);
    virtual void keyPressEvent(QKeyEvent * pevent);
    virtual void focusInEvent(QFocusEvent * pevent);
    virtual void focusOutEvent(QFocusEvent * pevent);

    //virtual QMenu *createPopupMenu(const QPoint & rpos);
	virtual void contextMenuEvent(QContextMenuEvent *);
    // just to silence some compiler warnings
    //?virtual QMenu *createPopupMenu(void) { return 0; }

    // NOT SUPPORTED
    virtual void paste();

	virtual void emitAllSignals();
	virtual void updateOutputFont(const QFont &);

private:
	static SxFindTextDlg  *getFindDlg(QWidget *);
	static void destroyFindDlg();

public slots:
	void slotCopy();
	void slotClear();
	void slotFlush();
	void slotSelectAll();
	void slotDeselect();
	void slotFindText();
	void slotSaveAs();
	void slotPrint(QSilPageSetup *);

	void slotSaveToFile(const QString &, bool);
	//void slotSetText(const QString&);
	void slotAppendText(const QString&);
	void slotUpdateContents(const QString&);

    void slotClosePage();
	void slotAppend(const QString &, int);
	void scrollToBottom();

protected slots:
	void slotSelectionChanged();

signals:
    void enableCopy( bool );
	void enableClear( bool );
	void enableFindText( bool );
	void enableSaveAs( bool );
	void enableFind(SxFindTextDlg::ITarget *);
	void signalSaveAs(QTextEdit *);
	void signalPrint(SeditexTextEdit *);
	void signalCreatePopupMenuRequested(QMenu *);
	void enableClose( bool );
	void signalClosePage();
	void signalCleared();

private:
    QString htmlOutput;
	QString	strPath;
	bool mbAutoScroll;
	bool mbClearable;
	bool mbSaveToFileSupport;
	bool mbCheckChevrons;
	int	mLineRef;
	int mOutputId;
	QString	msLeft;

	static MyColorTags gColorTags;
	static SxFindTextDlg* spFindTextDlg;
};




class SeditexInputOutputWin : public SeditexCommandWin
{
Q_OBJECT
public:
	SeditexInputOutputWin(QWidget *, const char *);
	virtual ~SeditexInputOutputWin();
	///void init(QWidget *);//bool bChevrons);

	SeditexTextEdit * textEdit(){ return mpTextEdit; }

	void readFile(QString);
	void readFile(int, const QString &);
	void append(const QString &);
	void setUseColors(bool b){ textEdit()->setUseColors(b); }
	void setSplitPane(bool b){ mbSplitPane = b; }

protected:
    virtual void emitAllSignals();
	virtual void updateOutputFont(const QFont &);
	virtual QSize sizeHint() const { return QSize(480, 320); }
	virtual void keyPressEvent(QKeyEvent *);

private:
	SeditexTextEdit *createTextEditPane(const char *);

public slots:
	void slotLogCommand(const QString &);
	void slotOutput(const QString &);
	void slotAppend(const QString &);
	void slotAppend(const QString &, int);
	void slotAppend(bool, const QString &);
	void slotCopy();
	void slotClear();
	void slotFlush();
	void slotSelectAll();
	void slotDeselect();
	void slotFindText();
	void slotSaveAs();
	void slotPrint(QSilPageSetup *);

protected slots:
	void slotCreatePopupMenuRequested(QMenu *);
	void slotUpdate();
	void slotCloseErrPane();

signals:
	void enableClose( bool );
	//void signalExecuteCommand(const QString &);

    void enableCopy( bool );
	void enableClear( bool );
	void enableFindText( bool );
	void enableSaveAs( bool );
	void enableFind(SxFindTextDlg::ITarget *);
	void signalSaveAs(QTextEdit *);
	void signalPrint(SeditexTextEdit *);

	//bogus ones to shut Qt debug messaging up
	void signalPageClosed();
	void signalClosePage();
	void signalFileSaved(QWidget *, const QString&);
	void signalHandlePageMenu(QWidget *, const QPoint &);

private:
	QSplitter *mpSplitter;
	SeditexTextEdit *mpTextEdit;
	SeditexTextEdit *mpTextEdit2;
	QString	mLastFile;
	bool mbSplitPane;
};

typedef  SeditexInputOutputWin	SeditexOutputWin;

#endif // !SMSPICEOUTPUTTEXTEDIT_H


