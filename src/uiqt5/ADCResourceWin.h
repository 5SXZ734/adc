#pragma once

#include <QSplitter>
#include <QTreeView>
#include "sx/SxDocument.h"
#include "ADCUtils.h"
#include "ADCTabsWin.h"//tab win

//class QListView;
class ADCStream;


///////////////////////////////////////////////////ADCResViewModel
class ADCResViewModel : public ADCTreeViewModel
{
Q_OBJECT
public:
	ADCResViewModel(QObject *parent, adcui::IResViewModel *pIModel)
		: ADCTreeViewModel(parent, pIModel)
	{
	}

	adcui::IResViewModel &resModel() const {
		return static_cast<adcui::IResViewModel &>(*mpIModel);
	}
	
	QString viewPos(const QModelIndex & index) const {
		ADCStream ss;
		quint32 itemId(index.internalId());
		resModel().viewPos(itemId, ss);
		return ss.ReadString();
	}

protected:
	virtual int	rowCount(const QModelIndex &parent) const
	{
		return ADCTreeViewModel::rowCount(parent);
	}
	virtual int	columnCount(const QModelIndex &) const 
	{ 
		return mpIModel->columnCount();
	}
	virtual Qt::ItemFlags flags(const QModelIndex &index) const
	{
		if (!index.isValid())
			return Qt::ItemFlags();
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;// | Qt::ItemIsEditable;
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
			if (index.column() > 0)
				break;
			quint32 itemId(index.internalId());
			ADCStream ss;
			mpIModel->data(itemId, 0, ss);
			return ss.ReadString();
		}
		case Qt::DecorationRole:
		{
			quint32 itemId(index.internalId());
			unsigned type(resModel().type(itemId));
			return QIcon(toIconStr(type));
		}
		default:
			break;
		}
		return QVariant();
	}
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		{
			if (section == 0)
				return tr("Resource");
			return tr("VA");
		}
		return QVariant();
	}
};



class ADCResView : public QTreeView
{
Q_OBJECT
//friend class ADCResWin;
public:
	ADCResView(QWidget *);
	~ADCResView();
	void setModel(adcui::IResViewModel *);
//signals:
	//void signalRefresh();
//private:
	//QAction*	mpRefreshAction;
};


class ADCWinToolbar;
////////////////////////////

class ADCResWin : public ADCTabsWin
{
	Q_OBJECT
public:
	ADCResWin(QWidget * parent, const char *);
	virtual ~ADCResWin(){}

	void setModel(adcui::IResViewModel *);

protected:
	void populate();
	void populateToolbar(){}

	//virtual void showEvent(QShowEvent *);
	virtual QWidget* createView(QString);
	virtual void onCurrentChanged(){}

private:
	//QString path(QTreeWidgetItem *) const;
	ADCResView* currentView() const
	{
		return dynamic_cast<ADCResView *>(mpTabWidget->currentWidget());
	}

public slots:
	void slotReset();
	void slotItemDoubleClicked(const QModelIndex &);
	void slotCurrentItemChanged(const QModelIndex &);
	//void slotItemClicked(int);
	void slotAboutToClose();
	void slotProjectNew();
	void slotCollapseChildren();
	void slotExpandChildren();

signals:
	void signalRequestModel(ADCResView &, QString);
	void signalRequestModel(ADCResWin &);
	void signalDumpResources(ADCStream &);
	void signalPostCommand(const QString &, bool);
	void signalShowCanvas();

	void signalItemDoubleClicked(QString);
	void signalItemClicked(QString);

protected:
	//ADCWinToolbar *mpToolbar;
	//ADCResourceView*	mpListView;
	QTreeView*	mpListView;
};


class ADCCanvasWin : public QWidget
{
	Q_OBJECT
public:
	ADCCanvasWin(QWidget * parent, const char *);
	virtual ~ADCCanvasWin(){}

protected:
	virtual void paintEvent(QPaintEvent *);

public slots:
	void slotUpdate();
signals:
	void signalUpdatePixmap(QPixmap &);

	//bogus ones to shut Qt debug messaging up
	void signalPageClosed();
	void signalClosePage();
	void signalFileSaved(QWidget *, const QString&);
	void signalHandlePageMenu(QWidget *, const QPoint &);
	void signalItemDoubleClicked(QString);
	void signalItemClicked(QString);

protected:
	virtual void emitAllSignals(){}
	virtual void updateOutputFont(const QFont &){}
private:
	QPixmap		mPixmap;
};
