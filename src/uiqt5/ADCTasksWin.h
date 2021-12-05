#pragma once


#include <QtCore/QAbstractItemModel>
#include <QListView>
#include <QTreeWidget>

#include "interface/IADCGui.h"
#include "ADCUtils.h"

class ADCStream;

class ADCTasksViewModel : public QAbstractItemModel
{
	adcui::ITasksViewModel *mpIModel;
public:
	ADCTasksViewModel(QObject *parent, adcui::ITasksViewModel *pIModel)
		: QAbstractItemModel(parent),
		mpIModel(pIModel)
	{
		mpIModel->AddRef();
	}
	~ADCTasksViewModel()
	{
		mpIModel->Release();
	}
	virtual void reset()
	{
		beginResetModel();
		mpIModel->reset();
		endResetModel();//QAbstractItemModel::reset();
	}
	adcui::ITasksViewModel *model() const {
		return mpIModel; }
	QString module(const QModelIndex & index) const {
		ADCStream ss;
		mpIModel->module(index.row(), ss);
		return ss.ReadString();
	}
	QString viewPos(const QModelIndex & index) const {
		ADCStream ss;
		mpIModel->viewPos(index.row(), ss);
		return ss.ReadString();
	}
protected:
	virtual QModelIndex	index(int row, int column, const QModelIndex & parent) const { 
		if (!hasIndex(row, column, parent))
			return QModelIndex();
		return createIndex(row, column, (quintptr)nullptr);
		//return QModelIndex(); 
	}
	virtual QModelIndex	parent(const QModelIndex & ) const { 
		return QModelIndex();
	}
	virtual int	rowCount(const QModelIndex &) const { 
		return (int)mpIModel->count();
	}
	virtual int	columnCount(const QModelIndex &) const { 
		return 1;
	}
	virtual QVariant data(const QModelIndex & index, int role) const 
	{
		if (index.isValid())
		switch (role)
		{
		case Qt::DisplayRole:
		{
			ADCStream ss;
			mpIModel->data(index.row(), ss);
			QString s;
			ss.ReadString(s);
			return s;
		}
		case Qt::ForegroundRole:
		{
			break;
		}
		case Qt::DecorationRole:
		{
			break;
		}
		default:
			break;
		}
		return QVariant();
	}
	virtual Qt::ItemFlags flags(const QModelIndex &index) const
	{
		if (!index.isValid())
			return Qt::ItemFlags();

		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	QVariant headerData(int , Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
			return QString("blah");

		return QVariant();
	}
};

class ADCTasksView : public QListView
{
	typedef TModelLocker<ADCTasksView> ModelLocker;
public:
	typedef	adcui::ITasksViewModel	ModelType;
	ModelType *safeModel() const { 
		if (model())
			return static_cast<ADCTasksViewModel *>(model())->model(); 
		return nullptr;
	}
public:
	ADCTasksView(QWidget *parent)
		: QListView(parent)
	{
	}
protected:
	virtual void paintEvent(QPaintEvent *);
};

class ADCTasksWin : public QWidget
{
	Q_OBJECT
public:
	ADCTasksWin(QWidget * parent, const char *);
	virtual ~ADCTasksWin(){}

	void setModel(adcui::ITasksViewModel *);

protected:
	virtual void showEvent(QShowEvent *);
	virtual void emitAllSignals(){}
	virtual void updateOutputFont(const QFont &){}
	//virtual bool isClosable() { return false; }

public slots:
	//void slotPushFront(QString, QString);
	//void slotPushBack(QString, QString);
	//void slotPopFront();
	void slotUpdate();

protected slots:
	void slotItemClicked(const QModelIndex &);
	void slotItemActivated(const QModelIndex &);

signals:
	void signalRequestData(ADCStream &);
	void signalItemClicked(QString);
	void signalItemActivated(QString);

	//bogus ones to shut Qt debug messaging up
	void signalPageClosed();
	void signalClosePage();
	void signalFileSaved(QWidget *, const QString&);
	void signalHandlePageMenu(QWidget *, const QPoint &);

protected:
	QListView*	mpListView;
	QString	mBinaryName;
	bool	mbDirty;
};


