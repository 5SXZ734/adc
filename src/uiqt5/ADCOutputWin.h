#ifndef __OUTPUTWIN_H__
#define __OUTPUTWIN_H__

#include <QWidget>
#include "sx/SxDocument.h"

class QTextEdit;
class QLineEdit;
class QLabel;

class ADC_OutputWin : public QWidget
{
Q_OBJECT
public:
	ADC_OutputWin(QWidget * parent, const char *name);
	virtual ~ADC_OutputWin(){}

protected:
	virtual void emitAllSignals(){}
	virtual void updateOutputFont(const QFont &);
	//virtual bool isClosable() { return false; }

public slots:
	void slotAppend(const QString &, int);

signals:
	void signalCloseDocument();

protected:
	QTextEdit *	mpTextEdit;
	QLineEdit *	mpLineEdit;
	QLabel *	mpLabel;
};

#endif//__OUTPUTWIN_H__
