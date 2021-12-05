#pragma once

#include <QSplitter>
#include <QTreeView>
#include "interface/IADCGui.h"
#include "sx/SxDocument.h"
#include "ADCUtils.h"

//class QListView;
class ADCStream;



class ADCExprViewModel : public ADCTreeViewModel
{
public:
	ADCExprViewModel(QObject *parent, adcui::IADCExprViewModel *pIModel)
		: ADCTreeViewModel(parent, pIModel)
	{
		mpIModel->AddRef();
	}
	~ADCExprViewModel()
	{
		mpIModel->Release();
	}

	adcui::IADCExprViewModel &exprModel() const { return static_cast<adcui::IADCExprViewModel &>(*mpIModel); }

	virtual void resetEx(adcui::IADCExprViewModel::Flags flags)
	{
		beginResetModel();
		exprModel().resetEx(flags);
		endResetModel();//QAbstractItemModel::reset();
	}
public:
	virtual int	columnCount(const QModelIndex &) const 
	{ 
		return mpIModel->columnCount();
	}
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		{
			switch (section)
			{
			case 0: return tr("Expression/Term");
			case 1: return tr("#");
			case 2: return tr("Id");
			case 3: return tr("Type");
			case 4: return tr("Offset");//Address
			default: break;
			}
			//pListView->header()->moveSection(0, 2);
		}
		return QVariant();
	}
	virtual Qt::ItemFlags flags(const QModelIndex &index) const
	{
		if (!index.isValid())
			return Qt::ItemFlags();
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	virtual QVariant data(const QModelIndex &index, int role) const
	{
		if (index.isValid())
		switch (role)
		{
		case Qt::ForegroundRole:
		{
			break;
		}
		case Qt::EditRole:
		case Qt::DisplayRole:
		{
			quint32 itemId(index.internalId());
			ADCStream ss;
			mpIModel->data(itemId, index.column(), ss);
			return ss.ReadString();
		}
		/*case Qt::DecorationRole:
		{
			quint32 itemId(index.internalId());
			unsigned type(namesModel().type(itemId));
			return QIcon(nameTypeToIconStr(type));
		}*/
		default:
			break;
		}
		return QVariant();
	}
	virtual bool canFetchMore(const QModelIndex &) const
	{
		return true;
	}
};


class ADCExprView : public QTreeView
{
public:
	ADCExprView(QWidget *parent)
		: QTreeView(parent)
	{
	}
	void setModel(adcui::IADCExprViewModel *pIModel);
	void update();
};


/////////////////////////////////////
class ADCExprWin : public QSplitter
{
	Q_OBJECT
public:
	ADCExprWin(QWidget * parent, const char *);
	virtual ~ADCExprWin(){}
	void setModel(adcui::IADCExprViewModel *);

protected:
	virtual void emitAllSignals(){}
	virtual void updateOutputFont(const QFont &);
	//virtual bool isClosable() { return false; }

public slots:
	void slotProjectNew();
	void slotAboutToClose();
	void slotCurExprChanged();
	void slotCurInfoChanged(const ADCCurInfo &);
	//void slotSyncResponce(int, bool);
	void slotSyncResponce(bool);

signals:
	void signalRequestModel(ADCExprWin &);

	//void signalDumpExpr(ADCStream &, bool);
	//void signalDumpPtrExprList(ADCStream &);

	//bogus ones to shut Qt debug messaging up
	void signalPageClosed();
	void signalClosePage();
	void signalFileSaved(QWidget *, const QString&);
	void signalHandlePageMenu(QWidget *, const QPoint &);
	//void signalSyncModeInquiry(bool &);

protected:
	ADCExprView *	mpListView;
//	QTreeView *	mpPtrExprView;
};