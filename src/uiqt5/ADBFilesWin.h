#ifndef __PROJECTWIN_INCLUDED__
#define __PROJECTWIN_INCLUDED__

#include "sx/SxDocument.h"

#include "ADCUtils.h"

#include <QStyledItemDelegate>
#include <QTreeView>
#include <QtCore/QEvent>


///////////////////////////////////////////////////ADCFilesViewModel
class ADCFilesViewModel : public ADCTreeViewModel
{
Q_OBJECT
public:
	ADCFilesViewModel(QObject *parent, adcui::IFilesViewModel *pIModel)
		: ADCTreeViewModel(parent, pIModel)
	{
	}

	adcui::IFilesViewModel &filesModel() const { return static_cast<adcui::IFilesViewModel &>(*mpIModel); }
	adcui::IFilesViewModel *model() const {
		return &filesModel(); }


	bool isModuleItem(const QModelIndex &index) const
	{
		switch (filesModel().type(index.internalId()))
		{
		case adcui::FOLDERTYPE_BINARY_EXE:
		case adcui::FOLDERTYPE_BINARY_DLL:
		case adcui::FOLDERTYPE_BINARY_PHANTOM:
			return true;
		default: break;
		}
		return false;
	}
	bool isPhantomItem(const QModelIndex &index) const
	{
		quint32 itemId(index.internalId());
		unsigned type(filesModel().type(itemId));
		return (type ==  adcui::FOLDERTYPE_BINARY_PHANTOM);
	}
	bool isFolderItem(const QModelIndex &index) const
	{
		quint32 itemId(index.internalId());
		unsigned type(filesModel().type(itemId));
		return (type == adcui::FOLDERTYPE_FOLDER);
	}
	bool isFileItem(const QModelIndex &index) const
	{
		switch (filesModel().type(index.internalId()))
		{
		case adcui::FOLDERTYPE_FILE_H:
		case adcui::FOLDERTYPE_FILE_CPP:
		case adcui::FOLDERTYPE_FILE_C:
		case adcui::FOLDERTYPE_FILE_RC:
		case adcui::FOLDERTYPE_FILE_T:
		case adcui::FOLDERTYPE_FILE_N:
		case adcui::FOLDERTYPE_FILE_E:
		case adcui::FOLDERTYPE_FILE_I:
		case adcui::FOLDERTYPE_FILE_TT:
		case adcui::FOLDERTYPE_FILE_STUB:
			return true;
		default:
			break;
		}
		return false;
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
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	}
	virtual QVariant data(const QModelIndex &index, int role) const
	{
		if (index.isValid())
		switch (role)
		{
		//case Qt::SizeHintRole:
			//return QVariant(QSize(500, 500));
		case Qt::ForegroundRole:
		{
#if(1)//draw phantoms in gray?
			if (isPhantomItem(index))
				return QBrush(Qt::gray);
#endif
			break;
		}
		case Qt::EditRole:
		case Qt::DisplayRole:
		{
			quint32 itemId(index.internalId());
			ADCStream ss;
			mpIModel->data(itemId, 0, ss);//index.column());
			return ss.ReadString();
		}
		case Qt::DecorationRole:
		{
			quint32 itemId(index.internalId());
			unsigned type(filesModel().type(itemId));
			return QIcon(toIconStr(type));
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
			quint32 itemId(index.internalId());
			mpIModel->rename(itemId, value.toString().toLatin1());
			//stringList.replace(index.row(), value.toString());
			//emit dataChanged(index, index);
			return true;
		}
		return false;
	}
	virtual QVariant headerData(int /*section*/, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
			return tr("Files");
		return QVariant();
	}
};


////////////////////////////////////////////
class ADCFilesViewDelegate : public QStyledItemDelegate
{
public:
    ADCFilesViewDelegate(QObject *parent = 0)
		: QStyledItemDelegate(parent)
	{
	}
protected:
	virtual void initStyleOption(QStyleOptionViewItem * option, const QModelIndex & index) const
	{
		QStyledItemDelegate::initStyleOption(option, index);
#ifdef QT4
		if (QStyleOptionViewItemV4 *v4 = qstyleoption_cast<QStyleOptionViewItemV4 *>(option))
#else
		if (QStyleOptionViewItem * v4 = qstyleoption_cast<QStyleOptionViewItem*>(option))
#endif
		{
			const ADCFilesViewModel *model(static_cast<const ADCFilesViewModel *>(index.model()));
			if ((option->state & QStyle::State_Open) && model->isFolderItem(index))
				v4->icon = QIcon(toFolderIconStr(true));
		}
	}
	/*virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
	{
		return QStyledItemDelegate::createEditor(parent, option, index);
	}*/

	virtual bool editorEvent(QEvent * event, QAbstractItemModel * /*model*/, const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/)
	{
		//prevent double click from starting editing
		if (event->type() == QEvent::MouseButtonDblClick)
			return true;//handled
		return false;
	}
};


class ADCFilesView : public QTreeView
{
	typedef TModelLocker<ADCFilesView> ModelLocker;
	friend class TModelLocker<ADCFilesView>;
	typedef	adcui::IFilesViewModel	ModelType;
	ModelType *safeModel() const { 
		if (model())
			return static_cast<ADCFilesViewModel *>(model())->model(); 
		return nullptr;
	}
public:
	ADCFilesView(QWidget *parent)
		: QTreeView(parent)
	{
	}
protected:
	virtual void paintEvent(QPaintEvent *e)
	{
		ModelLocker lock(this);
		QTreeView::paintEvent(e);
	}
};

////////////////////////////////////
class ADCProjectWin : public QWidget
{
	Q_OBJECT
public:
	ADCProjectWin(QWidget * parent, const char *);
	virtual ~ADCProjectWin(){}
	//void init();

	void setModel(adcui::IFilesViewModel *);

protected:
	virtual void emitAllSignals(){}
	virtual void updateOutputFont(const QFont &);
	//virtual bool isClosable() { return false; }

private:
	QModelIndex pathToIndex(QString);

public slots:
	void slotProjectNew();
	void slotUpdate();
	//void slotFileListChanged(ADCStream &);
	void slotCurFileChanged(QString);
	void slotShowCurrentFile();
	void slotAboutToClose();
	void slotFolderRenamed(QString);

protected slots:
	void slotItemDoubleClicked(const QModelIndex &);
	//void slotItemExpanded(const QModelIndex &);
	//void slotItemCollapsed(const QModelIndex &);
	void slotCustomContextMenuRequested(const QPoint &);
	void slotNewFile();
	void slotNewFolder();
	void slotDeleteItem();
	void slotCompile();
	void slotEditName();
	void slostTest();
	void slotNewFolderRecoil(QString);
	void slotGenerateSources();
	void slotDumpExports();
	void slotCalculateDependencies();

signals:
	void signalActivateItem(QString);
	void signalRequestModel(ADCProjectWin &);
	void signalCloseDocument();
	void signalShowSourceWin(QString, QString);
	void signalNewFile();
	void signalNewFolder();
	void signalDeleteItem();
	void signalCompile();
	void signalGenerateSources(QString);
	void signalDumpExports(QString);
	void signalCalculateDependencies(QString);
	void signalNewItem(QString);
	void signalDeleteItem(QString);
	void signalCompileItem(QString);
	void signalSetTargetDirectory();
	//void signalFileListRequest(ADCStream &);
	void signalLoadBinary();
	void signalUnloadBinary();

	//bogus ones to shut Qt debug messaging up
	void signalPageClosed();
	void signalClosePage();
	void signalFileSaved(QWidget *, const QString&);
	void signalHandlePageMenu(QWidget *, const QPoint &);

protected:
	ADCFilesView*	mpListView;
};

#endif//__PROJECTWIN_INCLUDED__
