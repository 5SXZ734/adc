#ifndef __SEDITEXFINDTEXTDLG_H__
#define __SEDITEXFINDTEXTDLG_H__

#include <QtCore/QString>
#include <QDialog>

class QLabel;
class QComboBox;
class QCheckBox;
class QPushButton;

class SxFindTextDlg : public QDialog
{
	Q_OBJECT

public:
    SxFindTextDlg(QWidget * pparent);
    virtual ~SxFindTextDlg()
    {
        delete mpTed4Search;
    }

	class ITarget : public QObject
	{
	public:
        ITarget(QObject* parent) : QObject(parent) {}
		virtual bool findIt(const QString& expr,
			bool cs, //case sensitive
			bool wo, //whole words
			bool reverse, //forward/reverse
            bool reset//start from begin/end of document
		) const = 0;
		//virtual int type() = 0;
	};

    void setTed(ITarget* value) 
	{
      //  delete mpTed4Search;
		mpTed4Search = value; 
		miCountSearch = 0;
	}

	ITarget * getTed() { return mpTed4Search; }
    void activate();
    void setFont(QFont);

protected:
	virtual void windowActivationChange(bool oldActive);
    virtual void showEvent(QShowEvent *);
    virtual void closeEvent(QCloseEvent *);
    virtual void reject(){
        //savePosition();
        QDialog::reject();
    }

    virtual void done(int r)
    {
        savePosition();
        QDialog::done(r);
    }

private:

    void activateFindCombo();
    void savePosition() {
        mGeometry = geometry();
    }
    void restorePosition() {
        if (!mGeometry.isEmpty())
            setGeometry(mGeometry);
    }

protected slots:
    void slotFindNext();
    //void slotFindNext2();
    void slotCancel();
    void slotReverseSearchStateChanged( int );
    void slotWholeWordChbxStateChanged( int );
    void slotCaseSensitiveChbxStateChanged( int );
private slots:
    void findTextchanged(const QString&);

private:
	//static SxFindTextDlg* mpThis;

    QLabel* mpFindLabel;
    QComboBox* mpFindTextCbx;
    QCheckBox* mpMatchWordCheck;
    QCheckBox* mpRegExpCheck;
    QCheckBox* mpCaseSensitiveCheck;
    QCheckBox* mpReverseCheck;
    QPushButton* mpFindNextBtn;
    QPushButton* mpCancelBtn;

    ITarget *  mpTed4Search;

    bool   mbReverseSearch;    
    bool   mbAdd2ComboBox;
    int    miCountSearch;
    QRect  mGeometry;
};



#endif//__SEDITEXFINDTEXTDLG_H__
