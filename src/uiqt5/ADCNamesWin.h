#pragma once

#include <QtCore/QVariant>
#include <QtCore/QAbstractItemModel>
#include <QtCore/QStringList>
#include <QSplitter>
#include <QDockWidget>
#include <QCompleter>
#include <QLineEdit>
#include <QTreeView>

//#include <string>

#include "interface/IADCGui.h"

#include "ADCUtils.h"
#include "ADCTabsWin.h"
#include "colors.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QPushButton;
class QListView;
class QListViewItem;
class QLabel;
class QSpinBox;
class QLineEdit;
class ADCWinToolbar;
class ADCStream;
class ADCNamesView;
class QSignalMapper;



/////////////////////////////////////////////////////////ADCNamesToolbar
class ADCNamesToolbar : public ADCWinToolbar
{
Q_OBJECT
public:
	ADCNamesToolbar(QWidget *parent);
	uint filterValue() const;

signals:
	void signalFilterChanged(int);
private:
	void add(int, const QIcon&, QString, QSignalMapper*);
public:
	enum { E_TYPES, E_FIELDS, E_FUNCTIONS, E_IMPORTS, E_EXPORTS, E__FILTER_MAX };
private:
	typedef QAction *	PQAction;
	PQAction	m_filters[E__FILTER_MAX];
};



///////////////////////////////////////////////////////// ADCNamesViewModel
class ADCNamesViewModel : public ADCTreeViewModel
{
	friend class ADCNamesViewProxyModel;
	uint	mFilterId;
public:
	ADCNamesViewModel(QObject *parent, adcui::INamesViewModel *pIModel)
		: ADCTreeViewModel(parent, pIModel),
		mFilterId(0)
	{
		mpIModel->AddRef();
	}
	~ADCNamesViewModel()
	{
		mpIModel->Release();
	}

	adcui::INamesViewModel &namesModel() const { return static_cast<adcui::INamesViewModel &>(*mpIModel); }

	QString moduleName() const
	{
		return "";//mpIModel->moduleName();
	}
	virtual void reset()
	{
		beginResetModel();
		mpIModel->reset();
		endResetModel();//QAbstractItemModel::reset();
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
		if (!isFltered())
			return true;
		quintptr itemId(index.internalId());
		uint filter(0);
		uint u(namesModel().type(itemId, index.column()));
#define _N(a)	adcui::INamesViewModel::a
#define _F(a)	(1 << ADCNamesToolbar::a)
		uint eKind(u & _N(E_KIND_MASK));
		switch (eKind)
		{
		case _N(E_TYPE): filter |= _F(E_TYPES); break;
		case _N(E_IMPORTED):
		case _N(E_FIELD):
		case _N(E_GLOBAL): filter |= _F(E_FIELDS); break;
		case _N(E_FUNCTION): filter |= _F(E_FUNCTIONS); break;
		default:
			break;
		}
		uint ie_mask((_F(E_IMPORTS) | _F(E_EXPORTS)));
		if (mFilterId & ie_mask)//IE filter is ON
		{
			uint filter_ie(0);
			if (mFilterId & _F(E_IMPORTS))
				if (eKind == _N(E_IMPORTED))
					filter_ie |= _F(E_IMPORTS);
			if (mFilterId & _F(E_EXPORTS))
				if (u & _N(E_EXPORTED))
					filter_ie |= _F(E_EXPORTS);
			if ((mFilterId & filter_ie) == 0)
				return false;
			if ((mFilterId & ~ie_mask) == 0)
				return true;//no other filters selected
		}
#undef _N
#undef _T
		return ((mFilterId & filter) != 0);
	}
	/*bool applyType(const QModelIndex & index, const char *typeStr)
	{
		return mpIModel->apply(index.row(), typeStr);
	}*/
	void goToDefinition(const QModelIndex &index)
	{
		if (!index.isValid())
			return;
		quintptr itemId(index.internalId());
		namesModel().goToDefinition(itemId);
	}

public:
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
				quintptr itemId(index.internalId());
				unsigned type(namesModel().type(itemId, index.column()));
				return QBrush(nameKindToColor(type));
			}
			break;
		case Qt::EditRole:
		case Qt::DisplayRole:
		case Qt::ToolTipRole:
		{
			quintptr itemId(index.internalId());
			ADCStream ss;
			mpIModel->data(itemId, index.column(), ss);//index.column());
			return ss.ReadString();
		}
		case Qt::DecorationRole:
		{
			quintptr itemId(index.internalId());
			unsigned type(namesModel().type(itemId, index.column()));
			QString pix(nameKindToIconStr(type));
			if (!pix.isEmpty())
				return QIcon(pix);
		}
		default:
			break;
		}
		return QVariant();
	}
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role)
	{
		if (index.isValid() && role == Qt::EditRole)
		{
			quintptr itemId(index.internalId());
			mpIModel->rename(itemId, value.toString().toLatin1());
			//stringList.replace(index.row(), value.toString());
			//emit dataChanged(index, index);
			return true;
		}
		return false;
	}
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		{
			ADCStream ss;
			mpIModel->data(0, section, ss);
			return ss.ReadString();

			//return tr("Names");
		}
		return QVariant();
	}
	virtual bool canFetchMore(const QModelIndex &) const
	{
		return true;
	}

private:
	friend class ADCNamesToolbar;
	static QString nameKindToIconStr(uint u)
	{
		switch (u & adcui::INamesViewModel::E_KIND_MASK)
		{
		case adcui::INamesViewModel::E_TYPE:
			return ":objects_4.png";//":cube_red.png";
		case adcui::INamesViewModel::E_FIELD:
			return ":cube_blue.png";
		case adcui::INamesViewModel::E_GLOBAL:
		case adcui::INamesViewModel::E_IMPORTED:
			return ":cube_green.png";
		case adcui::INamesViewModel::E_FUNCTION:
			return ":function.png";
		default:
			break;
		}
		return "";
	}
	static QColor nameKindToColor(uint u)
	{
		switch (u & adcui::INamesViewModel::E_KIND_MASK)
		{
		case adcui::INamesViewModel::E_IMPORTED:
			return ColorFromId(adcui::COLOR_IMPORT_REF);
		case adcui::INamesViewModel::E_GLOBAL:
		case adcui::INamesViewModel::E_FUNCTION:
			if (u & adcui::INamesViewModel::E_EXPORTED)
				return ColorFromId(adcui::COLOR_EXPORTED);
			break;
		default:
			break;
		}
		return QColor();
	}
};


/////////////////////////////////////////////////////////
class ADCNamesView : public QTreeView
{
Q_OBJECT
friend class ADCNamesWin;
public:
	ADCNamesView(QWidget *parent);
	~ADCNamesView();
	void populate(ADCStream &);
	void populate();
	void setContextId(unsigned);
	void setFilter(unsigned);
	void setModel(adcui::INamesViewModel *);
	ADCNamesViewModel *namesModel() const { return static_cast<ADCNamesViewModel *>(model()); }
protected:
	virtual void keyPressEvent (QKeyEvent *) ;
	virtual void currentChanged(const QModelIndex &, const QModelIndex &);
protected slots:
	void slotCustomContextMenuRequested(const QPoint &);
	void slotGoToDeclaration();
	void slotRefresh();
signals:
	void signalRefresh();
	void signalCurrentChanged(const QModelIndex &, const QModelIndex &);
private:
	QAction*	mpRefreshAction;
};




/////////////////////////////////////////////////////////
class ADCNamesWin : public ADCTabsWin
{
Q_OBJECT
public:
	ADCNamesWin(QWidget *parent, const char *name);
	~ADCNamesWin();
	//void init();

	void setModel(adcui::INamesViewModel *);

	void populate();
	void populateToolbar();

protected:
	virtual void showEvent(QShowEvent *);

	virtual QWidget* createView(QString);
	virtual void onCurrentChanged();
	virtual void updateOutputFont(const QFont&);

private:
	ADCNamesView* currentView();
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
	void signalRequestModel(ADCNamesWin &);
	void signalRequestModel(ADCNamesView &, QString);
	//void signalDumpTypes(ADCStream &);

	void signalItemDoubleClicked(QString);
	void signalItemClicked(QString);
	//void signalContextIdChanged(unsigned);

private:
	ADCNamesView*	mpView;

	bool mbDirty;
};







class ADCExportsViewModel : public ADCTreeViewModel
{
public:
	ADCExportsViewModel(QObject *parent, adcui::IExportsViewModel *pIModel)
		: ADCTreeViewModel(parent, pIModel)
	{
		mpIModel->AddRef();
	}
	~ADCExportsViewModel()
	{
		mpIModel->Release();
	}

	adcui::IExportsViewModel &exportsModel() const { return static_cast<adcui::IExportsViewModel &>(*mpIModel); }

	QString moduleName() const
	{
		return "";//mpIModel->moduleName();
	}
	virtual void reset()
	{
		beginResetModel();
		mpIModel->reset();
		endResetModel();//QAbstractItemModel::reset();
	}
	virtual bool isRowVisible(QModelIndex index) const
	{
		quintptr itemId(index.internalId());
		uint u(exportsModel().type(itemId, index.column()));
		if (u & adcui::INamesViewModel::E_EXPORTED)
			if (!(u & adcui::INamesViewModel::E_ALIAS))
				return true;
		return false;
	}
	void goToDefinition(const QModelIndex &index)
	{
		if (!index.isValid())
			return;
		quintptr itemId(index.internalId());
		exportsModel().goToDefinition(itemId);
	}
public:
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
			if (index.column() == int(adcui::ExportViewColumns::NAME))
				return QBrush(ColorFromId(adcui::COLOR_EXPORTED));
			break;
		case Qt::EditRole:
		case Qt::DisplayRole:
		{
			quintptr itemId(index.internalId());
			ADCStream ss;
			mpIModel->data(itemId, index.column(), ss);//index.column());
			return ss.ReadString();
		}
		/*case Qt::DecorationRole:
		{
			quintptr itemId(index.internalId());
			unsigned type(namesModel().type(itemId, index.column()));
			const char *pix(nameTypeToIconStr(type));
			if (pix)
				return QIcon(pix);
		}*/
		default:
			break;
		}
		return QVariant();
	}
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role)
	{
		if (index.isValid() && role == Qt::EditRole)
		{
			quintptr itemId(index.internalId());
			mpIModel->rename(itemId, value.toString().toLatin1());
			return true;
		}
		return false;
	}
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		{
			ADCStream ss;
			mpIModel->data(0, section, ss);
			return ss.ReadString();
		}
		return QVariant();
	}
	virtual bool canFetchMore(const QModelIndex &) const
	{
		return true;
	}
};


class ADCImportsViewModel : public ADCExportsViewModel
{
public:
	ADCImportsViewModel(QObject* parent, adcui::IExportsViewModel* pIModel)
		: ADCExportsViewModel(parent, pIModel)
	{
	}
protected:
	virtual bool isRowVisible(QModelIndex index) const
	{
		quintptr itemId(index.internalId());
		uint u(exportsModel().type(itemId, index.column()));
		if ((u & adcui::INamesViewModel::E_KIND_MASK) == adcui::INamesViewModel::E_IMPORTED)
			return true;
		return false;
	}
	virtual QVariant data(const QModelIndex& index, int role) const
	{
		if (index.isValid())
			if (role == Qt::ForegroundRole)
			{
				if (index.column() == int(adcui::ImportViewColumns::NAME))
					return QBrush(ColorFromId(adcui::COLOR_IMPORT_REF));
				return QVariant();
			}
		return ADCExportsViewModel::data(index, role);
	}
};


/////////////////////////////////////////////////////////
class ADCExportsView : public QTreeView
{
Q_OBJECT
friend class ADCExportsWin;
public:
	ADCExportsView(QWidget *parent);
	~ADCExportsView();
	void populate();
	void setModel(adcui::IExportsViewModel *);
	ADCExportsViewModel *exportsModel() const { return static_cast<ADCExportsViewModel *>(model()); }
protected:
	virtual QAbstractItemModel* newModel(adcui::IExportsViewModel*);
private slots:
	void slotCustomContextMenuRequested(const QPoint&);
	void slotGoToDeclaration();
signals:
	void signalRefresh();
};

class ADCImportsView : public ADCExportsView
{
public:
	ADCImportsView(QWidget* parent)
		: ADCExportsView(parent)
	{
	}
	virtual QAbstractItemModel* newModel(adcui::IExportsViewModel*) override;
};


class ADCExportsWin : public ADCTabsWin
{
	Q_OBJECT
public:
	ADCExportsWin(QWidget* parent, const char* name);
	~ADCExportsWin() {}
	void setImportView() { mbImp = true; }

	void setModel(adcui::IExportsViewModel*);

	//void populate();
	//void populateToolbar();
	ADCExportsView* currentView() const;

protected:
	virtual void showEvent(QShowEvent *);

private slots:
	void slotDoubleClicked(const QModelIndex&);
	void slotUpdate();

signals:
	void signalRequestModel(ADCExportsWin &);
	void signalRequestModel(ADCExportsView &, QString);
	void signalItemDoubleClicked(QString);

protected:
	virtual QWidget* createView(QString);

private:
	ADCExportsView*	mpView;
	bool mbImp;
	bool mbDirty;
};


typedef ADCExportsWin	ADCImportsWin;



