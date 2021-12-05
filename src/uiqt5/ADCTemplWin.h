#pragma once

#include <QtCore/QAbstractItemModel>
#include <QListView>
#include <QTreeView>
#include "interface/IADCGui.h"
#include "ADCUtils.h"
#include "ADCTabsWin.h"


class ADCTemplViewModel : public ADCTreeViewModel//QAbstractItemModel
{
public:
	ADCTemplViewModel(QObject *parent, adcui::ITemplViewModel *pIModel)
		: ADCTreeViewModel(parent, pIModel)
	{
		mpIModel->AddRef();
	}
	~ADCTemplViewModel()
	{
		mpIModel->Release();
	}

	adcui::ITemplViewModel &templModel() const { return static_cast<adcui::ITemplViewModel &>(*mpIModel); }

	QString module() const
	{
		ADCStream ss;
		templModel().path(0, ss);
		return ss.ReadString();
	}
	virtual void reset()
	{
		beginResetModel();
		mpIModel->reset();
		endResetModel();//QAbstractItemModel::reset();
	}
protected:
	virtual int	columnCount(const QModelIndex &) const { 
		return templModel().columnCount();
	}
	QVariant headerData(int column, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		{
			ADCStream ss;
			mpIModel->data(0, column, ss);
			return ss.ReadString();

			/*if (column == 0)
				return QString("Substitute");
			if (column == 1)
				return QString("Scope");
			return QString("Incumbent");*/
		}
		return QVariant();
	}
	virtual QVariant data(const QModelIndex & index, int role) const 
	{
		if (index.isValid())
		{
			switch (role)
			{
			case Qt::EditRole:
			case Qt::DisplayRole:
			case Qt::ToolTipRole:
			{
				ADCStream ss;
				quintptr itemId(index.internalId());
				mpIModel->data(itemId, index.column(), ss);
				return ss.ReadString();
			}
			case Qt::ForegroundRole:
			{
				//unsigned flags(templModel().flags(index.row()));
				break;
			}
			case Qt::DecorationRole:
			{
				quintptr itemId(index.internalId());
				unsigned type(templModel().type(itemId, index.column()));
				return QIcon(TypeToIconStr(type));
			}
			//case Qt::FontRole:
			//return QFont(...);
			default:
				break;
			}
		}
		return QVariant();
	}
	virtual Qt::ItemFlags flags(const QModelIndex &index) const
	{
		if (!index.isValid())
			return Qt::ItemFlags();

		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	virtual bool canFetchMore(const QModelIndex &) const
	{
		return true;
	}
private:
	static const char *TypeToIconStr(uint u)
	{
		switch (u)
		{
		case adcui::ITemplViewModel::E_TYPE:
			return ":type_4.png";
		case adcui::ITemplViewModel::E_GLOBAL:
			return ":cube_green.png";
		case adcui::ITemplViewModel::E_FUNCTION:
			return ":function.png";
		default:
			break;
		}
		return nullptr;
	}
};



/////////////////////////////////////////////////////////
class ADCTemplView : public QTreeView
{
Q_OBJECT
friend class ADCNamesWin;
public:
	ADCTemplView(QWidget *parent, QString path);
	~ADCTemplView();
	void populate(ADCStream &);
	void populate();
	void setContextId(unsigned);
	void setFilter(unsigned);
	void setModel(adcui::ITemplViewModel *);
	ADCTemplViewModel *templModel() const { return static_cast<ADCTemplViewModel *>(model()); }
protected:
	virtual void keyPressEvent(QKeyEvent *) ;
	//virtual void currentChanged(const QModelIndex &, const QModelIndex &);
public slots:
	void slotCustomContextMenuRequested(const QPoint &);
	void slotToggleTypesFields();
signals:
	void signalRefresh();
	void signalCurrentChanged(const QModelIndex &, const QModelIndex &);
	void signalRequestModel(ADCTemplView &, QString, bool);

private:
	QAction*	mpRefreshAction;
	bool mbMode;//types or fields(1)
	QString		msPath;
};



/////////////////////////////////////////////////////////
class ADCTemplWin : public ADCTabsWin
{
Q_OBJECT
public:
	ADCTemplWin(QWidget *parent, const char *name);
	~ADCTemplWin();

	void setModel(adcui::ITemplViewModel *);

	void populate();
	void populateToolbar();

protected:
	virtual void showEvent(QShowEvent *);

	virtual QWidget* createView(QString);
	//virtual void onCurrentChanged();

//private:
	ADCTemplView* currentView();
//	uint filterValue() const;

public slots:
	void slotProjectNew();
	void slotUpdate();
	void slotContextIdChanged(unsigned);
	void slotAboutToClose();

//private slots:
//	void slotFilterType(int);
//	void slotTypeActivated(const QModelIndex &);
//	void slotDoubleClicked(const QModelIndex &);

signals:
	void signalRequestModel(ADCTemplWin &, bool);
	void signalRequestModel(ADCTemplView &, QString, bool);
	//void signalDumpTypes(ADCStream &);

	void signalItemDoubleClicked(QString);
	void signalItemClicked(QString);
	//void signalContextIdChanged(unsigned);

private:
	bool mbDirty;
};

