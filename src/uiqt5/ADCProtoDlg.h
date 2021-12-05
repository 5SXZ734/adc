#pragma once

#ifdef NOCMAKE
#include "ADCProtoDlgBase.ui.h"
#else
#include "ui_ADCProtoDlgBase.h"
#endif
//#include "ADCUtils.h"
#include "ADCStream.h"


class ADCProtoEditDlg : public QDialog, public Ui::ADCProtoDlgBase
{
    Q_OBJECT

public:

	struct Data
	{
		QString	sObject;
		//QString	sEncoding;
		int	nPSalign;

		int	nPSin;
		int	nPStackPurge;
		int	nPStackPurgeRet;
		bool bPScleanArgs;
		bool bVariardic;
		bool bUsercall;
		bool bThiscall;
		int	nFPRin;
		int	nFPRout;
		QString	sGPRin;
		QString	sSpoiledRegs;
		//QString	sFlagsSav;
		QString	sGPRout;
		void write(ADCStream& ss) const
		{
			ss.WriteStringf("Object=%s", sObject.toLatin1().constData());
			//ss.WriteStringf("Encoding=%s", sEncoding.toLatin1().constData());
			ss.WriteStringf("PSalign=%d", nPSalign);

			ss.WriteStringf("PSin=%d", nPSin);
			ss.WriteStringf("PStackPurge=%d", nPStackPurge);
			ss.WriteStringf("PStackPurgeRet=%d", nPStackPurgeRet);
			ss.WriteStringf("PScleanArgs=%d", bPScleanArgs ? 1 : 0);
			ss.WriteStringf("Variardic=%d", bVariardic ? 1 : 0);
			ss.WriteStringf("Usercall=%d", bUsercall ? 1 : 0);
			ss.WriteStringf("Thiscall=%d", bThiscall ? 1 : 0);
			ss.WriteStringf("FPRin=%d", nFPRin);
			ss.WriteStringf("FPRout=%d", nFPRout);
			ss.WriteStringf("GPRin=%s", sGPRin.toLatin1().constData());
			ss.WriteStringf("SpoiledRegs=%s", sSpoiledRegs.toLatin1().constData());
			//ss.WriteStringf("FlagsSav=%s", sFlagsSav.toLatin1().constData());
			ss.WriteStringf("GPRout=%s", sGPRout.toLatin1().constData());
		}
	};

	struct DataParser : public Data
	{
		MY_QSCRIPT(DataParser);
		MY_QUSTRING(Object);
		//MY_QUSTRING(Encoding);
		MY_QINT(PSalign);
		MY_QINT(PSin);
		MY_QINT(PStackPurge);
		MY_QINT(PStackPurgeRet);
		MY_QBOOL(PScleanArgs);
		MY_QBOOL(Variardic);
		MY_QBOOL(Usercall);
		MY_QBOOL(Thiscall);
		MY_QINT(FPRin);
		MY_QINT(FPRout);
		MY_QUSTRING(GPRin);
		MY_QUSTRING(SpoiledRegs);
		//MY_QUSTRING(FlagsSav);
		MY_QUSTRING(GPRout);
		DataParser()
		{
			MY_QIMPL(Object);
			//MY_QIMPL(Encoding);
			MY_QIMPL(PSalign);
			MY_QIMPL(PSin);
			MY_QIMPL(PStackPurge);
			MY_QIMPL(PStackPurgeRet);
			MY_QIMPL(PScleanArgs);
			MY_QIMPL(Variardic);
			MY_QIMPL(Thiscall);
			MY_QIMPL(Usercall);
			MY_QIMPL(FPRin);
			MY_QIMPL(FPRout);
			MY_QIMPL(GPRin);
			MY_QIMPL(SpoiledRegs);
			//MY_QIMPL(FlagsSav);
			MY_QIMPL(GPRout);
		}
	};

public:
    ADCProtoEditDlg(QWidget* parent);
	virtual ~ADCProtoEditDlg();

	void setObjectInfo(QString);
	void setConfig(int stackaddrsize);
	void setData(const Data&);
	void getData(Data&);

public slots:
	void slotSetProtoData(const ADCProtoEditDlg::Data&);

private slots:
	void slotAccept();
	void slotHide();
	void slotApply();

	void slotStackInValueChanged(int);
	void slotStackPurgeValueChanged(int);
	void slotStdCallToggled(bool);
	void slotVariardicToggled(bool);

signals:
	void signalSetProtoData(const ADCProtoEditDlg::Data&, bool);

protected:
	virtual void changeEvent(QEvent *);
	virtual void showEvent(QShowEvent *);

private:

	int	mStackAddrSize;
	int mStackPurgeRet;
};


