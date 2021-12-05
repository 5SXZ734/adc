#pragma once

#include <QtCore/QVariant>
#include <QtCore/QStringList>
#include <QtCore/QAbstractItemModel>
#include <QSplitter>
#include <QDialog>
#include <QDockWidget>
#include <QCompleter>
#include <QLineEdit>
#include <QTreeView>
#include <QListView>
#include <QStandardItemModel>

#include "interface/IADCGui.h"

#include <string>
#include "ADCUtils.h"
#include "ADCTabsWin.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QPushButton;
class QListView;
class QListViewItem;
class QLabel;
class QSpinBox;
class ADCWinToolbar;
class ADCStream;
class ADCBinView;
class ADCTypesView;
class ADCTypesToolbar;
class QLineEdit;
class ADCModelDataMap;

typedef QDialog	ADCTypesDlgBase;


class ADCTypesCompleter : public QCompleter
{
	Q_OBJECT
		Q_PROPERTY(QString separator READ separator WRITE setSeparator)

public:
	explicit ADCTypesCompleter(QObject* parent = 0)
		: QCompleter(parent)
	{
	}
	explicit ADCTypesCompleter(QAbstractItemModel* model, QObject* parent = 0)
		: QCompleter(model, parent)
	{
	}
	QString separator() const { return sep; }
public slots:
	void setSeparator(const QString& separator) { sep = separator; }
protected:
	QStringList splitPath(const QString& path) const
	{
		if (sep.isNull())
			return QCompleter::splitPath(path);
		return path.split(sep);
	}
	QString pathFromIndex(const QModelIndex& index) const
	{
		if (sep.isNull())
			return QCompleter::pathFromIndex(index);
		// navigate up and accumulate data
		QStringList dataList;
		for (QModelIndex i = index; i.isValid(); i = i.parent())
			dataList.prepend(model()->data(i, completionRole()).toString());
		return dataList.join(sep);
	}
private:
    QString sep;
};



class ADCTypesDlg : public ADCTypesDlgBase
{
Q_OBJECT
public:
    ADCTypesDlg(QWidget* parent, const char *);
    ~ADCTypesDlg();

	void setModel(adcui::ITypesViewModel*);
	void setField(QString);
	void setContents(const QStringList&);
	void setArray(int);
	QString getSelected();
	int array() const;
	ADCTypesView &view() { return *mpListView; }
	QString typeString() const;

protected:
	virtual void showEvent(QShowEvent *);
	virtual void closeEvent(QCloseEvent *);
	virtual bool eventFilter(QObject *, QEvent *);

public:
	//virtual void emitAllSignals(){}
	void updateOutputFont(const QFont &);

private:
	void createContents(QWidget *);
	void enableButtons(bool);
	void updateTypeString();

public slots:
	void depopulate();
	void slotLocusChanged(QString, int);
	void slotUpdateContextTypes();

protected slots:
    void slotOK();
    void slotHide();
	void slotApply();
	void slotHighlight(const QModelIndex&);
	void slotFilterType(int);
//	void slotCurrentChanged(QTreeWidgetItem *, QTreeWidgetItem *);
//	void slotDoubleClicked(QTreeWidgetItem *, int);
	void slotItemActivated(const QModelIndex &);
	void slotItemSelected(const QModelIndex &, const QModelIndex &);
	void slotApplyArray();
	void slotApplyPointer();
	void slotApplyConst();
	void slotApplyUndo();

signals:
	//void signalRequestModel(ADCTypesView &, QString);
	void signalRequestModel(ADCTypesDlg&, QString);

	//bogus ones to shut Qt debug messaging up
	void signalPageClosed();
	void signalClosePage();
	void signalFileSaved(QWidget *, const QString&);
	void signalHandlePageMenu(QWidget *, const QPoint &);

protected:
	QLineEdit *mpLineEdit;
	ADCTypesToolbar *mpToolbar;
	//QLabel *mpContextLbl;
	//QLabel *mpNameLbl;
    //QAction		*mpApplyAction;
	QPushButton	*mpApplyPBtn;
	//QPushButton	*mpOkPBtn;
    QPushButton	*mpHidePBtn;

    ADCTypesView *mpListView;

	QSpinBox	*mpArrayNumSBx;
	QAction *mpApplyArrayAction;
	QAction	*mpApplyPtrAction;
	QAction	*mpApplyConstAction;
	QAction *mpApplyUndoAction;
	int	miArray;
	QStringList	mTypeChain;
	//QRect mGeometry;

	ADCTypesCompleter* mpCompleter;
};

///////////////////////////////////////////

class ADCRecentTypesModel : public QStandardItemModel
{
	QAbstractItemModel* mpSaved;
public:
	ADCRecentTypesModel(QAbstractItemModel* pSaved)
		: mpSaved(pSaved)
	{
	}
	QAbstractItemModel* saved() const { return mpSaved; }
};

class ADCTypesViewModel : public ADCTreeViewModel//QAbstractItemModel
{
	friend class ADCTypesViewProxyModel;
	//adcui::ITypesViewModel *mpIModel;
	uint	mContextId;
	uint	mFilterId;
	uint	mColumnNum;
	ADCRecentTypesModel mRecentTypesModel;

public:
	ADCTypesViewModel(QObject *parent, adcui::ITypesViewModel *pIModel)
		: ADCTreeViewModel(parent, pIModel),//QAbstractItemModel(parent),
		//mpIModel(pIModel),
		mContextId(0),
		mFilterId(0),
		mColumnNum(0),//from core
		mRecentTypesModel(this)
	{
		mpIModel->AddRef();
	}
	~ADCTypesViewModel()
	{
		mpIModel->Release();
	}

	adcui::ITypesViewModel &typesModel() const { return static_cast<adcui::ITypesViewModel &>(*mpIModel); }

	QString moduleName() const
	{
		return typesModel().moduleName();
	}
	virtual void reset()
	{
		beginResetModel();
		mpIModel->reset();
		endResetModel();//QAbstractItemModel::reset();
	}
	bool setContextId(unsigned u){
		u = u & 0xF;// adcui::CXTID_LOCALITY_MASK;
		bool a(mContextId >= adcui::CXTID_SOURCE);
		bool b(u >= adcui::CXTID_SOURCE);
		mContextId = u;
		return a != b;
	}
	bool setFilter(unsigned u)
	{
		unsigned t(mFilterId);
		mFilterId = u;
		return mFilterId != t;
	}
	bool isFltered() const { return mFilterId != 0; }
	bool isRowVisible(QModelIndex index) const
	{
		if (mFilterId == 0)
			return true;
		unsigned flags(typesModel().flags(index.row()));
		unsigned type(flags & adcui::ITypesViewModel::E_TYPE_MASK);
		return ((mFilterId & (1 << type)) != 0);
	}
	bool applyType(QString typeStr, QModelIndex current)
	{
		if (typeStr.isEmpty())
			return false;
		QList<QStandardItem*> l(mRecentTypesModel.findItems(typeStr));
		if (l.isEmpty())
		{
			QString s0;
			if (current.isValid())
				s0 = current.data(Qt::DisplayRole).toString();
			QIcon icon;
			if (typeStr == s0)
				icon = current.data(Qt::DecorationRole).value<QIcon>();

			QStandardItem* p(new QStandardItem(icon, typeStr));
			p->setEditable(false);
			mRecentTypesModel.invisibleRootItem()->appendRow(p);
			mRecentTypesModel.sort(0);
		}

		return typesModel().apply(typeStr.toLatin1());
	}
	QAbstractItemModel* recentTypesModel() const { return (QAbstractItemModel*)&mRecentTypesModel; }
	void setColumnCount(uint n) { mColumnNum = n; }
protected:
	/*virtual QModelIndex	index(int row, int column, const QModelIndex & parent) const { 
		if (!hasIndex(row, column, parent))
			return QModelIndex();
		return createIndex(row, column, nullptr);
		//return QModelIndex(); 
	}
	virtual QModelIndex	parent(const QModelIndex & ) const { 
		return QModelIndex();
	}
	virtual int	rowCount(const QModelIndex &) const { 
		return (int)typesModel().childrenNum();
	}*/
	virtual int	columnCount(const QModelIndex &) const { 
		uint n(mpIModel->columnCount());
		if (mColumnNum > 0 && mColumnNum <= n)
			return mColumnNum;
		return n;
	}
	virtual QVariant data(const QModelIndex & index, int role) const 
	{
		if (!index.isValid())
			return QVariant();

		unsigned flags(typesModel().flags(index.row()));
		if (index.column() == 1)
		{
			if (role == Qt::DisplayRole || role == Qt::ToolTipRole)
			{
				if (flags & adcui::ITypesViewModel::E_ATTIC)
					return ATTIC_NAME;
				ADCStream ss;
				mpIModel->data(index.row(), 1, ss);
				QString s;
				ss.ReadString(s);
				return s;
			}
			return QVariant();
		}

		unsigned type(flags & adcui::ITypesViewModel::E_TYPE_MASK);

		switch (role)
		{
		case Qt::EditRole:
		case Qt::DisplayRole:
		case Qt::ToolTipRole:
		{
			ADCStream ss;
			mpIModel->data(index.row(), 0, ss);
			QString s;
			ss.ReadString(s);
			return s;
		}
		case Qt::ForegroundRole:
		{
			//			if (mContextId > adcui::CXTID_BINARY)
			if (type != adcui::ITypesViewModel::E_PRIMITIVE)
				//if (!(flags & adcui::ITypesViewModel::E_USERDATA))
				if (flags & adcui::ITypesViewModel::E_FWD)
					return QBrush(Qt::gray);
			break;
		}
		case Qt::DecorationRole:
		{
			switch (type)
			{
			case adcui::ITypesViewModel::E_PRIMITIVE:
				return QIcon(":type_3.png");
			case adcui::ITypesViewModel::E_COMPOUND:
				if (isUnavailable(flags))
					return QIcon(":type_4b.png");
				return QIcon(":type_4.png");
			case adcui::ITypesViewModel::E_ENUM:
				return QIcon(":enum.png");
			case adcui::ITypesViewModel::E_FUNC:
				return QIcon(":function.png");
			case adcui::ITypesViewModel::E_TYPEDEF:
				return QIcon(":type_0.png");
			case adcui::ITypesViewModel::E_CODE:
				return QIcon(":type_code.png");
			case adcui::ITypesViewModel::E_CONTEXT_DEPENDENT:
				if (isUnavailable(flags))
					return QIcon(":type_9b.png");
				return QIcon(":type_9.png");
			default:
				break;
			}
			break;
		}
		//case Qt::FontRole:
		//return QFont(...);
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

	QVariant headerData(int column, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		{
			if (column == 0)
				return QString("Name");
			return QString("Location");
		}
		return QVariant();
	}
	virtual bool canFetchMore(const QModelIndex &) const
	{
		return true;
	}
private:
	bool isUnavailable(unsigned flags) const
	{
		return ((mContextId > adcui::CXTID_BINARY)
			&& !(flags & adcui::ITypesViewModel::E_USERDATA));
	}
};

class ADCTypesView : public QTreeView
{
Q_OBJECT
friend class ADCTypesWin;
public:
	ADCTypesView(QWidget *parent);
	~ADCTypesView();
	void populate(ADCStream &);
	void populate();
	void setContextId(unsigned);
	void setFilter(unsigned);
	virtual void setModel(adcui::ITypesViewModel *);
	ADCTypesViewModel *typesModel() const { return static_cast<ADCTypesViewModel *>(model()); }
protected:
	virtual void keyPressEvent (QKeyEvent *) ;
	virtual void currentChanged(const QModelIndex &, const QModelIndex &);
signals:
	void signalRefresh();
	void signalCurrentChanged(const QModelIndex &, const QModelIndex &);
private:
	QAction*	mpRefreshAction;
};


class ADCTypesView2 : public ADCTypesView//for type dlg
{
public:
	ADCTypesView2(QWidget *parent)
		: ADCTypesView(parent)
	{
	}
protected:
	virtual void setModel(adcui::ITypesViewModel *);
};



/////////////////////////////////////////////////////////
class ADCTypesToolbar : public ADCWinToolbar
{
Q_OBJECT
public:
	ADCTypesToolbar(QWidget *parent);
	uint filterValue() const;
private slots:
	void slotFilterChanged(int);
signals:
	void signalFilterChanged(int);
private:
	typedef QAction *	PQAction;
	static const size_t E_FAVORITE = adcui::ITypesViewModel::E__TYPES_TOTAL;
	static const size_t E_TOTAL = adcui::ITypesViewModel::E__TYPES_TOTAL + 1;//+ favorite
	PQAction	m_typesFilter[E_TOTAL];

};



/////////////////////////////////////////////////////////
class ADCTypesWin : public ADCTabsWin
{
Q_OBJECT
public:
	ADCTypesWin(QWidget *parent, const char *name);
	~ADCTypesWin();
	//void init();

	void setModel(adcui::ITypesViewModel *);
	void setBinModel(ADCModelDataMap &, adcui::IBinViewModel *);

	void populate();
	void populateToolbar();

protected:
	virtual void showEvent(QShowEvent *);

	virtual QWidget* createView(QString);
	virtual void onCurrentChanged();

private:
	ADCTypesView* currentView();
	uint filterValue() const;

public slots:
	void slotProjectNew();
	void slotUpdate();
	void slotContextIdChanged(unsigned);
	void slotAboutToClose();

private slots:
	void slotFilterType(int);
	void slotTypeActivated(const QModelIndex &);
	void slotDoubleClicked(const QModelIndex &);

signals:
	void signalRequestModel(ADCTypesWin &);
	void signalRequestModel(ADCTypesView &, QString);
	//void signalRequestBinModel(ADCTypesWin &, QString);
	//void signalDumpTypes(ADCStream &);

	void signalItemDoubleClicked(QString);
	void signalItemClicked(QString);
	//void signalContextIdChanged(unsigned);

private:
	ADCTypesView*	mpView;
	ADCBinView*		mpBinView;

	bool mbDirty;
};



/////////////////////////////////////////////////////ADCTypeCompleter
class ADCTypeCompleter : public QLineEdit
{
	QCompleter m_completer;
public:
	ADCTypeCompleter(QWidget *parent)
		: QLineEdit(parent),
		m_completer(this)
	{
		m_completer.setCaseSensitivity(Qt::CaseSensitive);
		m_completer.setCompletionMode(QCompleter::UnfilteredPopupCompletion);
		//m_completer.setCompletionMode(QCompleter::InlineCompletion);
		setCompleter(&m_completer);
	}
	void complete()
	{
		m_completer.complete();
	}
	void setModel(adcui::ITypesViewModel *pIModel)
	{
		ADCTypesViewModel *p(new ADCTypesViewModel(this, pIModel));
		m_completer.setModel(p);
		p->reset();
	}
};



