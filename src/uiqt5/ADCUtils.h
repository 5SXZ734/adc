#pragma once

//#include "qx/IUnk.h"
//#include "qx/MyStream.h"

#include <QAbstractItemModel>
#include <QLayout>
#include <QComboBox>

#include "interface/IADCGui.h"

class QString;
class QSpinBox;

#include "ADCStream.h"

/////////////////////////////////////////////////// ADCTreeViewModel
class ADCTreeViewModel : public QAbstractItemModel
{
protected:
	adcui::ITreeViewModel *mpIModel;
public:
	ADCTreeViewModel(QObject *parent, adcui::ITreeViewModel *pIModel)
		: QAbstractItemModel(parent),
		mpIModel(pIModel)
	{
		mpIModel->AddRef();
	}
	~ADCTreeViewModel()
	{
		mpIModel->Release();
	}
	/*virtual void reset()
	{
		mpIModel->reset();
		//QAbstractItemModel::reset();
	}*/

	void invalidate()
	{
		beginResetModel();
		mpIModel->reset();
		endResetModel();//QAbstractItemModel::reset();
		//emit layoutChanged();
	}

	QModelIndex idFromUnique(int iUnique) const
	{
		quintptr itemId((quintptr)mpIModel->IdFromUnique(iUnique));
		if (itemId == 0)
			return QModelIndex();
		return createIndex(mpIModel->indexOf(itemId), 0, itemId);
	}
	QString itemPath(const QModelIndex &index) const
	{
		ADCStream ss;
		mpIModel->path(index.internalId(), ss);
		return ss.ReadString();
	}
public:

	struct ItemIteratorData
	{
		quintptr parentId;
		int childrenNum;//number of childs
		quintptr childId;
		int childIndex;//child index
		ItemIteratorData(quintptr pid, int n, quintptr cid)
			: parentId(pid), childrenNum(n), childId(cid), childIndex(0)
		{
		}
		/*ItemIteratorData& operator=(const ItemIteratorData& o)
		{
			parentId = o.parentId;
			childrenNum = o.childrenNum;
			childId = o.childId;
			childIndex = o.childIndex;
			return *this;
		}*/
	};

	class ItemIterator : public QList<ItemIteratorData>
	{
		const ADCTreeViewModel &mrModel;
		friend class ADCFilesViewModel;
	public:
		ItemIterator(ADCTreeViewModel &r)
			: mrModel(r)
		{
			int n(mrModel.mpIModel->childrenNum(0));
			if (n > 0)
			{
				quintptr q((quintptr)mrModel.mpIModel->idOfChild(0, 0));
				push_back(ItemIteratorData(0, n, q));
			}
		}
		operator bool() const { return !empty(); }
		QModelIndex operator*()
		{
			const ItemIteratorData &a(back());
			return mrModel.createIndex(a.childIndex, 0, a.childId);
		}
		quintptr childUnique() const { return mrModel.mpIModel->uniqueOf(back().childId); }
		quintptr parentUnique() const { return mrModel.mpIModel->uniqueOf(back().parentId); }
		ItemIterator &operator++()
		{
			ItemIteratorData &a(back());
			if (mrModel.mpIModel->hasChildren(a.childId, true))
			{
				int n(mrModel.mpIModel->childrenNum(a.childId));
				quintptr q((quintptr)mrModel.mpIModel->idOfChild(a.childId, 0));
				push_back(ItemIteratorData(a.childId, n, q));
				return *this;
			}
			if (++a.childIndex < a.childrenNum)
			{
				a.childId++;
				return *this;
			}
			for (;;)
			{
				pop_back();
				if (empty())
					break;
				ItemIteratorData &b(back());
				if (++b.childIndex < b.childrenNum)
				{
					b.childId++;
					break;
				}
			}
			return *this;
		}
		ItemIterator &operator++(int){ return operator++(); }
		size_t level() const { return size() - 1; }
	};

	void print(std::ostream &os)
	{
		for (ADCTreeViewModel::ItemIterator i(*this); i; i++)
		{
			for (size_t j(0); j < i.level(); j++)
				os << "\t";
			os << data(*i, Qt::DisplayRole).toString().toStdString() << std::endl;
		}
	}
	virtual QModelIndex index(int row, int column, const QModelIndex &parent) const
	{
		if (hasIndex(row, column, parent))
		{
			quintptr parentId(parent.isValid() ? parent.internalId() : 0);//mpIModel->idOfRoot());
			quintptr childId((quintptr)mpIModel->idOfChild(parentId, row));
			if (childId)
				return createIndex(row, column, childId);
		}
		return QModelIndex();
	}
	virtual QModelIndex	parent(const QModelIndex &index) const 
	{
		if (index.isValid())
		{
			quintptr childId(index.internalId());
			if (childId)
			{
				quintptr parentId((quintptr)mpIModel->idOfParent(childId));
				if (parentId)
					return createIndex(mpIModel->indexOf(parentId), 0, parentId);
			}
		}
		return QModelIndex();
	}
	virtual int	rowCount(const QModelIndex &parent) const
	{ 
		if (parent.column() > 0)
			return 0;
		quintptr parentId(parent.isValid() ? parent.internalId() : 0);
		return (int)mpIModel->childrenNum(parentId);
	}

	virtual bool hasChildren(const QModelIndex & parent) const
	{
		if (parent.column() > 0)
			return false;
		quintptr parentId(parent.isValid() ? parent.internalId() : 0);
		return (int)mpIModel->hasChildren(parentId, false);
	}
	virtual void fetchMore(const QModelIndex & parent)
	{
		quintptr parentId(parent.isValid() ? parent.internalId() : 0);
		mpIModel->fetch(parentId);
	}
	virtual bool canFetchMore(const QModelIndex &) const
	{
		return true;
	}
};

const char *toIconStr(QString);
const char *toIconStr(uint);
const char *toFolderIconStr(bool bOpened);


template <typename TVIEW>
class TModelLocker
{
	typename TVIEW::ModelType *mpModel;
public:
	TModelLocker(TVIEW *pView)
		: mpModel(pView->safeModel())
	{
		if (mpModel)
			mpModel->lockRead(true);
	}
	~TModelLocker()
	{
		if (mpModel)
			mpModel->lockRead(false);
	}
	typename TVIEW::ModelType *pmodel() const { return mpModel; }
	typename TVIEW::ModelType &model() const { return *mpModel; }
};

struct ADCCurInfo
{
	QString scope;
	QString scopePath;
	QString obj;
	QString objPath;
};




#include <QObject>
#include <QAbstractItemView>
#include <QHelpEvent>
#include <QToolTip>

class QToolTipper : public QObject
{
	Q_OBJECT
public:
	explicit QToolTipper(QObject* parent = nullptr)
		: QObject(parent)
	{
	}

protected:
	bool eventFilter(QObject* obj, QEvent* event)
	{
		if (event->type() == QEvent::ToolTip)
		{
			QAbstractItemView* view = qobject_cast<QAbstractItemView*>(obj->parent());
			if (!view)
			{
				return false;
			}

			QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
			QPoint pos = helpEvent->pos();
			QModelIndex index = view->indexAt(pos);
			if (!index.isValid())
				return false;

			QString itemText = view->model()->data(index).toString();
			QString itemTooltip = view->model()->data(index, Qt::ToolTipRole).toString();

			QFontMetrics fm(view->font());
			int itemTextWidth = fm.horizontalAdvance(itemText);//width
			QRect rect = view->visualRect(index);
			int rectWidth = rect.width();

			if ((itemTextWidth > rectWidth) && !itemTooltip.isEmpty())
			{
				QToolTip::showText(helpEvent->globalPos(), itemTooltip, view, rect);
			}
			else
			{
				QToolTip::hideText();
			}
			return true;
		}
		return false;
	}
};


class QToolButton;

class ADCWinToolbar : public QFrame
{
	QHBoxLayout		m_hbox;
public:
	ADCWinToolbar(QWidget *);
	QToolButton *addButton(QAction *);
	QToolButton *addButton(const QIcon &, const QString &);
	void addStretch();
	void addSeparator();
	QComboBox *addComboBox();
	QSpinBox* addSpinBox();
protected:
	virtual void cleanup();
};


