#include "ADBFilesWin.h"

#include <QLayout>
#include <QMenu>
#include <QHeaderView>

#include "interface/IADCGui.h"
#include "ADCUtils.h"

//////////////////////////////////////////////////////////////// ADCFolderItem

/*class ADCFolderItem : public QTreeWidgetItem
{
public:
	adcui::FolderTypeEnum m_type;
public:
	static adcui::FolderTypeEnum folderType(QString s0)
	{
		QString s(s0.section('\t', 1, 1));
		if (s.isEmpty())
			return adcui::FOLDERTYPE_UNK;
		return (adcui::FolderTypeEnum)s.toUInt();
	}
	static QString folderName(QString s)
	{
		s = s.section('\t', 0, 0);
		const QByteArray cs(MODULE_SEP);
		if (s.endsWith(cs))
			s.truncate(s.length() - cs.length());
		else if (s.endsWith("\\") || s.endsWith("/"))
			s.truncate(s.length() - 1);
		Q_ASSERT(!(s.endsWith("\\") || s.endsWith("/")));
		return s;
	}
	ADCFolderItem(QTreeWidget *view, QTreeWidgetItem *after, QString s)
		: QTreeWidgetItem(view, after),
		m_type(folderType(s))
	{
		setText(0, folderName(s));
		setIcon(0, QIcon(toIconStr(m_type)));
		if (m_type == adcui::FOLDERTYPE_BINARY_PHANTOM)
			setForeground(0, QBrush(Qt::darkGray));
	}
	ADCFolderItem(QTreeWidgetItem *view, QTreeWidgetItem *after, QString s)
		: QTreeWidgetItem(view, after),
		m_type(folderType(s))
	{
		setText(0, folderName(s));
		if (IsFolder())
			setIcon(0, QIcon(":std_folder_opened_16.png"));
		else
			setIcon(0, QIcon(toIconStr(m_type)));
	}
	bool IsRoot() const { return !parent(); }
	bool IsFolder() const { return m_type == adcui::FOLDERTYPE_FOLDER; }
	bool IsPhantom() const { return m_type == adcui::FOLDERTYPE_BINARY_PHANTOM; }
	bool IsFile() const { return !IsRoot() && !IsFolder(); }
	QString path() const
	{
		QString sPath;
		const ADCFolderItem *lvi(this);
		while (lvi)
		{
			QString s(lvi->text(0));
			if (lvi->IsRoot())
				s.append(MODULE_SEP);
			else if (lvi->IsFolder())
				s.append("/");
			sPath.prepend(s);
			lvi = (const ADCFolderItem *)lvi->parent();
		}
CHECK_QSTRING(sPath, z);
		return sPath;
	}
};*/



//////////////////////////////////////////////////////////////// ADCProjectWin

ADCProjectWin::ADCProjectWin(QWidget * parent, const char *name)
	: QWidget(parent)//,
	//mpCurItem(nullptr)
{
	setObjectName(name);

	mpListView = new ADCFilesView(this);
	mpListView->setItemDelegate(new ADCFilesViewDelegate(this));

	mpListView->setRootIsDecorated(true);
	//mpListView->setSelectionMode(Q3ListView::Extended);
	mpListView->setAllColumnsShowFocus(true);
	mpListView->setExpandsOnDoubleClick(false);
	mpListView->setContextMenuPolicy(Qt::CustomContextMenu);

	//mpListView->setFocusPolicy(QListView::NoFocus);
	//mpListView->setSelectionMode(QListView::NoSelection);
	//mpListView->setSorting(-1);
//	mpListView->headerItem()->setText(0, tr("Files"));
//	mpListView->setColumnCount(1);

	mpListView->setHeaderHidden(true);
	mpListView->header()->setStretchLastSection(false);
#ifdef QT4
	mpListView->header()->setResizeMode(QHeaderView::ResizeToContents);
#else
	mpListView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif

	connect(mpListView, SIGNAL(doubleClicked(const QModelIndex &)), SLOT(slotItemDoubleClicked(const QModelIndex &)));
	connect(mpListView, SIGNAL(activated(const QModelIndex &)), SLOT(slotItemDoubleClicked(const QModelIndex &)));
	//connect(mpListView, SIGNAL(expanded(const QModelIndex &)), SLOT(slotItemExpanded(const QModelIndex &)));
	//connect(mpListView, SIGNAL(collapsed(const QModelIndex &)), SLOT(slotItemCollapsed(const QModelIndex &)));
	connect(mpListView, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(slotCustomContextMenuRequested(const QPoint &)));

	connect(this, SIGNAL(signalNewFile()), SLOT(slotNewFile()));
	connect(this, SIGNAL(signalNewFolder()), SLOT(slotNewFolder()));
	connect(this, SIGNAL(signalDeleteItem()), SLOT(slotDeleteItem()));
	connect(this, SIGNAL(signalCompile()), SLOT(slotCompile()));

	QVBoxLayout * vbox(new QVBoxLayout(this));
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addWidget(mpListView);
}

/*void ADCProjectWin::init()
{
	emit signalRequestModel(*this);
}*/

void ADCProjectWin::slotCustomContextMenuRequested(const QPoint &pos)
{
    QModelIndex index(mpListView->indexAt(pos));
    if (!index.isValid())
		return;

	const ADCFilesViewModel *model(static_cast<const ADCFilesViewModel *>(index.model()));

	QMenu menu;

	QString s(model->itemPath(index));
	if (!model->isModuleItem(index))
	{
		menu.addAction(QIcon(":edit_name.png"), tr("Edit Name"), this, SLOT(slotEditName()));
		menu.addSeparator();
	}
	if (model->isModuleItem(index) || model->isFolderItem(index))//a folder
	{
		menu.addAction(QIcon(":file_add.png"), tr("Add File"), this, SIGNAL(signalNewFile()));
		menu.addAction(QIcon(":folder_add.png"), tr("Add Folder"), this, SIGNAL(signalNewFolder()));
		menu.addSeparator();
	}
	else
	{
		menu.addAction(QIcon(":compile.png"), tr("Compile"), this, SIGNAL(signalCompile()));
		menu.addAction(QIcon(":.png"), tr("Calculate Dependencies"), this, SLOT(slotCalculateDependencies()));
		menu.addSeparator();
	}
	menu.addAction(QIcon(":delete_16.png"), tr("Delete"), this, SIGNAL(signalDeleteItem()));
	//if (model->isModuleItem(index))
	{
//?		menu.addSeparator();
//?		menu.addAction(tr("Set Output Direcory..."), this, SIGNAL(signalSetTargetDirectory()));

		menu.addSeparator();
		menu.addAction(QIcon(":save_listing.png"), tr("Save Listing(s)..."), this, SLOT(slotGenerateSources()));
		menu.addAction(QIcon(":.png"), tr("Dump Exports..."), this, SLOT(slotDumpExports()));
		
	//}
	//if (model->isModuleItem(index))
	//{
		menu.addSeparator();
		menu.addAction(QIcon(":application_plus.png"), tr("Load Binary"), this, SIGNAL(signalLoadBinary()));
		menu.addAction(QIcon(":application_minus.png"), tr("Unload Binary"), this, SIGNAL(signalUnloadBinary()));
	}

#if(0)
	menu.addAction(tr("test"), this, SLOT(slostTest()));
#endif
	menu.exec(mpListView->viewport()->mapToGlobal(pos));
}

void ADCProjectWin::slotGenerateSources()
{
	QModelIndex lvi(mpListView->currentIndex());
	if (lvi.isValid())
	{
		const ADCFilesViewModel *model(static_cast<const ADCFilesViewModel *>(lvi.model()));
		//if (model->isModuleItem(lvi))
		{
			QString s(model->itemPath(lvi));
			emit signalGenerateSources(s);
		}
	}
}

void ADCProjectWin::slotDumpExports()
{
	QModelIndex lvi(mpListView->currentIndex());
	if (lvi.isValid())
	{
		const ADCFilesViewModel *model(static_cast<const ADCFilesViewModel *>(lvi.model()));
		if (model->isModuleItem(lvi))
		{
			QString s(model->itemPath(lvi));
			if (s.endsWith(MODULE_SEP))
			{
				s.chop(1);
				emit signalDumpExports(s);
			}
		}
	}
}

void ADCProjectWin::slotCalculateDependencies()
{
	QModelIndex lvi(mpListView->currentIndex());
	if (lvi.isValid())
	{
		const ADCFilesViewModel *model(static_cast<const ADCFilesViewModel *>(lvi.model()));
		//if (model->isModuleItem(lvi))
		{
			QString s(model->itemPath(lvi));
			emit signalCalculateDependencies(s);
		}
	}

}

void ADCProjectWin::slotNewFile()
{
	QModelIndex lvi(mpListView->currentIndex());
	if (lvi.isValid())
	{
		const ADCFilesViewModel *model(static_cast<const ADCFilesViewModel *>(lvi.model()));
		if (!model->isFileItem(lvi))
		{
			QString s(model->itemPath(lvi));
			s.append("?");
			s.append(" -r");//request a recoil for name editing
			emit signalNewItem(s);
		}
	}
}

void ADCProjectWin::slotNewFolder()
{
	QModelIndex lvi(mpListView->currentIndex());
	if (lvi.isValid())
	{
		const ADCFilesViewModel *model(static_cast<const ADCFilesViewModel *>(lvi.model()));
		if (!model->isFileItem(lvi))
		{
			QString s(model->itemPath(lvi));
			s.append("?/");
			s.append(" -r");//request a recoil for name editing
			emit signalNewItem(s);
		}
	}
}

void ADCProjectWin::slotDeleteItem()
{
	QModelIndex lvi(mpListView->currentIndex());
	if (lvi.isValid())
	{
		const ADCFilesViewModel *model(static_cast<const ADCFilesViewModel *>(lvi.model()));
		QString s(model->itemPath(lvi));
		if (!s.isEmpty())
			emit signalDeleteItem(s);
	}
}

void ADCProjectWin::slotCompile()
{
	QModelIndex lvi(mpListView->currentIndex());
	if (lvi.isValid())
	{
		const ADCFilesViewModel *model(static_cast<const ADCFilesViewModel *>(lvi.model()));

		QString s(model->itemPath(lvi));
		if (!s.isEmpty())
			emit signalCompileItem(s);
	}
}

void ADCProjectWin::slotEditName()
{
	QModelIndex lvi(mpListView->currentIndex());
	if (lvi.isValid())
	{
#ifdef _DEBUG
		const ADCFilesViewModel *model(static_cast<const ADCFilesViewModel *>(lvi.model()));
		Q_ASSERT(!model->isModuleItem(lvi));
#endif
		mpListView->edit(lvi);
	}
}


void ADCProjectWin::slotUpdate()
{
	ADCFilesViewModel *model(static_cast<ADCFilesViewModel *>(mpListView->model()));
	if (!model)
		return;

	QList<int> l;
	for (ADCFilesViewModel::ItemIterator i(*model); i; i++)
	{
		if (mpListView->isExpanded(*i))
			l.push_back(i.childUnique());
	}

	model->invalidate();

	for (QList<int>::ConstIterator i(l.begin()); i != l.end(); i++)
	{
		QModelIndex index(model->idFromUnique(*i));
		if (index.isValid())
		{
			mpListView->setExpanded(index, true);
			model->fetchMore(index);//this will enable a children sub-section in model's display vector!
		}
	}

	update();
	//update();
	/*ADCStream ss;
	emit signalFileListRequest(ss);
	slotFileListChanged(ss);*/
}

void ADCProjectWin::slostTest()
{
	if (ADCFilesViewModel *model = static_cast<ADCFilesViewModel *>(mpListView->model()))
	{
		model->print(std::cout);
		std::cout.flush();
	}
}

/*void ADCProjectWin::slotFileListChanged(ADCStream &ss)
{
	mpListView->blockSignals(true);
	mpListView->clear();
	mpCurItem = nullptr;

	QList<QTreeWidgetItem *> stack;//keeps last item on each level

	bool bIsFantom(false);
	QString s;
	while (ss.ReadString(s))
	{
		//std::string z(s.toStdString());

		int l(1);
		for (; s.startsWith("\t"); l++)
			s.remove(0, 1);

		QTreeWidgetItem *lviAfter(nullptr);

		if (l > stack.size())
		{
			Q_ASSERT(l - stack.size() == 1);
		}
		else if (l == stack.size())
		{
			if (!stack.isEmpty())
			{
				lviAfter = stack.back();
				stack.pop_back();
			}
		}
		else 
		{
			while (l <= stack.size())
			{
				lviAfter = stack.back();
				stack.pop_back();
			}
		}

		ADCFolderItem *lvi;
		if (stack.isEmpty())
		{
			lvi = new ADCFolderItem(mpListView, lviAfter, s);
			bIsFantom = lvi->IsPhantom();
			if (!bIsFantom)
				lvi->setExpanded(true);
		}
		else
		{
			lvi = new ADCFolderItem(stack.back(), lviAfter, s);
			if (lvi->IsFolder() && !bIsFantom)
				lvi->setExpanded(true);
		}

		stack.push_back(lvi);
	}

	mpListView->blockSignals(false);*
}*/

#define SEL_FONT 1
#define SEL_BRUSH 0
void ADCProjectWin::slotCurFileChanged(QString s0)
{
/*	QString s(QDir::toNativeSeparators(s0));
CHECK_QSTRING(s, z1);
	//QStringList l(s.split('\n'));
	if (mpCurItem)
	{
#if(SEL_FONT)
		QFont f(mpCurItem->font(0));
		f.setWeight(QFont::Normal);
		mpCurItem->setFont(0, f);
#endif
#if(SEL_BRUSH)
		QBrush b(Qt::black);
		mpCurItem->setForeground(0, b);
#endif
	}

	for (QTreeWidgetItemIterator i(mpListView); *i; i++)
	{
		QString s2(QDir::toNativeSeparators(path(*i)));
CHECK_QSTRING(s2, z2);
		if (s2 == s)
		{
#if(SEL_FONT)
			QFont f((*i)->font(0));
			f.setWeight(QFont::Bold);
			(*i)->setFont(0, f);
#endif
#if(SEL_BRUSH)
			QBrush b(Qt::red);
			(*i)->setForeground(0, b);
#endif
			mpCurItem = *i;
			break;
		}
	}*/
}

void ADCProjectWin::slotShowCurrentFile()
{
/*	if (mpCurItem)
	{
		emit signalShowSourceWin(path(mpCurItem), QString());
	}*/
}

void ADCProjectWin::slotItemDoubleClicked(const QModelIndex &index)
{
	if (index.isValid())
	{
		const ADCFilesViewModel *model(static_cast<const ADCFilesViewModel *>(index.model()));
		if (!model->isFolderItem(index))
			//emit signalShowSourceWin(model->itemPath(index), QString());
			emit signalActivateItem(model->itemPath(index));
	}
}

void ADCProjectWin::updateOutputFont(const QFont &f)
{
	mpListView->setFont(f);
}

void ADCProjectWin::setModel(adcui::IFilesViewModel *pIModel)
{
	ADCFilesViewModel *model(new ADCFilesViewModel(this, pIModel));
	mpListView->setModel(model);
	//mpListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);//Qt::ScrollBarAsNeeded);
	//mpListView->header()->setStretchLastSection(true);
}

void ADCProjectWin::slotAboutToClose()
{
	QAbstractItemModel *pModel(mpListView->model());
	mpListView->setModel(nullptr);
	delete pModel;
}

void ADCProjectWin::slotProjectNew()
{
	Q_ASSERT(!mpListView->model());
	emit signalRequestModel(*this);
}

QModelIndex ADCProjectWin::pathToIndex(QString s)
{
	QModelIndex ret;

	QAbstractItemModel *model0(mpListView->model());
	if (ADCFilesViewModel *model = static_cast<ADCFilesViewModel *>(model0))
	{
		s.replace('\\', '/');
		QStringList l(s.split('/', Qt::SkipEmptyParts));
		if (!l.isEmpty())
		{
			QString s(l.front());
			s.truncate(s.indexOf(':'));
			QModelIndex parent;
			do {
				QModelIndex parent0(parent);
				model0->fetchMore(parent);
				//if (model0->hasChildren(parent))
				for (int i(0); i < model0->rowCount(parent); i++)
				{
					QModelIndex item(model0->index(i, 0, parent));
					QString s2(model0->data(item, Qt::DisplayRole).toString());
					if (s2 == s)
					{
						parent = item;
						break;
					}
				}
				if (parent == parent0)
					break;//not fond on current level
				l.pop_front();
				if (l.isEmpty())
					return parent;
				s = l.front();
			} while (parent != QModelIndex());
		}
	}
	return ret;
}

void ADCProjectWin::slotNewFolderRecoil(QString s)
{
	QModelIndex index(pathToIndex(s));
	if (index.isValid())
	{
		mpListView->setCurrentIndex(index);
		mpListView->edit(index);
	}
}

void ADCProjectWin::slotFolderRenamed(QString s)
{
	QModelIndex index(pathToIndex(s));
	if (index.isValid())
	{
		mpListView->setCurrentIndex(index);
	}
}
