#pragma once

#include "interface/IADCGui.h"
#include "qx/MyString.h"
#include "qx/MyRedirect2.h"
#include "mem.h"
#include "files.h"
#include "ui_main.h"
#include "info_proj.h"

class MyStreamBase;
class ADCOutput;
class SSSRemoteCore;
class BinViewModel_t;
class Main_t;
class Project_t;
class DumpTarget_t;

//////////////////////////////////////////////////////////FilesViewModel_t

class FilesViewModel_t : public adcui::IFilesViewModel
{
protected:
	Core_t &mrCore;

	typedef mem::MemRef	myITEMID;

	struct elt_t : public AVLTreeNode<elt_t, int>
	{
		typedef AVLTreeNode<elt_t, int>	base_t;
		CFolderPtr	pFolder;
		myITEMID	uMyIndex;
		myITEMID	uParentIndex;
		myITEMID	uChildrenIndex;//not valid if iChildrenNum<1
		int			iChildrenNum;//0: no children, -1: not fetched
		adcui::FolderTypeEnum eType;
		elt_t(int key)
			: base_t(key),
			pFolder(nullptr), uMyIndex(0), uParentIndex(0), uChildrenIndex(0), iChildrenNum(-1), eType(adcui::FOLDERTYPE_UNK)
		{
		}
		elt_t(FolderPtr p, adcui::FolderTypeEnum e, myITEMID parent) 
			: AVLTreeNode<elt_t, int>(p->ID()),
			pFolder(p), uMyIndex(0), uParentIndex(parent), uChildrenIndex(0), iChildrenNum(-1), eType(e)
		{
			assert(0);
		}
		int compare(int o) const
		{
			if (_key() < o)
				return -1;
			if (_key() == o)
				return 0;
			return 1;
		}
	};
	//typedef AVLTreeNode<elt0_t, int>	elt_t;
	//std::vector<elt_t>	m_data;
#define POW2_SIZEOF_ELT  LOG2_UINT32((sizeof(elt_t) - 1)) + 1

	typedef mem::TPool<elt_t, 5, POW2_SIZEOF_ELT>	MyVec;

#undef POW2_SIZEOF_ELT

	MyVec	m_data;
	AVLTree<elt_t>	m_tree;
	myITEMID	muRootIndex;
public:
	FilesViewModel_t(Core_t &rCore)
		: mrCore(rCore),
		muRootIndex(0)
	{
	}

	inline elt_t &addEltRoot()
	{
		mem::HMEM<elt_t> h(m_data.New(0));
		elt_t &a(*h);
		a.pFolder = nullptr;
		a.eType = adcui::FOLDERTYPE_UNK;
		a.uParentIndex = 0;
		a.uMyIndex = h;
		//a.iChildrenNum = -1;//not fetched
		return a;
	}

	inline elt_t &eddElt(CFolderPtr pFolder, adcui::FolderTypeEnum eType, elt_t &aParent)
	{
		//size_t rootIndex(m_tree.saveRoot(m_data.data()));//preserve a root node in avl tree, this maybe screwed up below!
//		m_data.push_back(elt_t(pFolder, eType, uParentIndex));
		assert(pFolder);
		mem::HMEM<elt_t> h(m_data.New(pFolder->ID()));
		elt_t &a(*h);
		a.pFolder = pFolder;
		a.eType = eType;
		a.uParentIndex = aParent.uMyIndex;
		a.uMyIndex = h;
		if (aParent.iChildrenNum < 0)
		{
			aParent.iChildrenNum = 0;
			aParent.uChildrenIndex = h;
		}
		aParent.iChildrenNum++;
		//m_tree.recoverRoot(m_data.data(), rootIndex);
		if (eType <= adcui::FOLDERTYPE_FOLDER)
			m_tree.insert(&a);//&m_data.back());
		return a;
	}
	elt_t &at(ITEMID h) const
	{
		elt_t *p(m_data.get((myITEMID)h));
		return *p;
	}
	size_t size() const
	{
		return m_data.tellp();//size()
	}

	virtual int columnCount() const
	{
		return 1;
	}

	virtual void reset()
	{
		m_data.clear();
		m_tree.reset();
		if (!mrCore.main().hasProject())
			return;
		//m_data.resize(1);//NIL element
		elt_t &aRoot(addEltRoot());
		muRootIndex = aRoot.uMyIndex;

		Project_t &proj(mrCore.main().project());
		ProjectInfo_t PI(proj);

		const FoldersMap &m(proj.rootFolder().children());
		for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
		{
			CFolderRef rFolder(*i);
			if (!PI.IsPhantomFolder(rFolder))
			{
				const MyString &s(rFolder.name());
				adcui::FolderTypeEnum eType(Ext2FolderType(s));
				eddElt(&rFolder, eType, aRoot);
			}
		}
		//phantoms go after
		for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
		{
			CFolderRef rFolder(*i);
			if (PI.IsPhantomFolder(rFolder))
				eddElt(&rFolder, adcui::FOLDERTYPE_BINARY_PHANTOM, aRoot);
		}
	}
	virtual void fetch(ITEMID parent) const
	{
		if (!parent)
			parent = muRootIndex;
		if (parent)
		{
			elt_t &a(at(parent));
			const_cast<FilesViewModel_t *>(this)->assureChildren(a);
		}
	}
	virtual bool hasChildren(ITEMID parent, bool bFetched) const
	{
		if (!parent)
			parent = muRootIndex;
		if (!parent)
			return false;
		elt_t &a(at(parent));
		if (!(a.iChildrenNum < 0))//if it was fetched already - return it
			return (a.iChildrenNum > 0);
		if (!bFetched && a.pFolder)
		{
			FileFolder_t *p(a.pFolder->fileFolder());
			return p && !p->empty();
		}
		return false;
	}
	virtual unsigned childrenNum(ITEMID parent) const//children count
	{
		if (!parent)
			parent = muRootIndex;
		if (!isValid(parent))
			return 0;
		if (!parent)
			return 0;
		elt_t &a(at(parent));
//const_cast<FilesViewModel_t *>(this)->assureChildren(a);
		assert(!(a.iChildrenNum < 0));
		return unsigned(a.iChildrenNum);
	}

	virtual void data(ITEMID row, size_t, MyStreamBase &ss) const
	{
		assert(isValid(row));
		MyStreamUtil ssu(ss);
		ssu.WriteString(name(at(row)));
	}

	virtual void path(ITEMID row, MyStreamBase &ss) const
	{
		assert(isValid(row));
		Project_t &proj(mrCore.main().project());
		const elt_t &a(at(row));
		MyString s(proj.files().relPath(a.pFolder));
		s.append(FolderType2Ext(a.eType));
		MyStreamUtil ssu(ss);
		ssu.WriteString(s);
	}

	virtual adcui::FolderTypeEnum type(ITEMID row) const
	{
		assert(isValid(row));
		return at(row).eType;
	}

	virtual ITEMID idOfRoot() const
	{
		return muRootIndex;
	}
	virtual ITEMID idOfChild(ITEMID parent, unsigned childIndex) const
	{
		assert(isValid(parent));
		elt_t &a(at(parent ? parent : muRootIndex));
//const_cast<FilesViewModel_t *>(this)->assureChildren(a);
		assert(a.iChildrenNum > 0 && childIndex < unsigned(a.iChildrenNum));
		//assert(a.uChildrenIndex > 0);
		childIndex += (unsigned)MyVec::toIndex(a.uChildrenIndex);
		ITEMID child(MyVec::fromIndex(childIndex));
		assert(isValidNotRoot(child));
		return child;
	}
	virtual ITEMID idOfParent(ITEMID child) const
	{
		assert(isValidNotRoot(child));
		const elt_t &a(at(child));
		assert(isValid(a.uParentIndex));
		return a.uParentIndex == muRootIndex ? 0 : a.uParentIndex;
	}
	virtual unsigned indexOf(ITEMID item) const
	{
		assert(isValidNotRoot(item));
		const elt_t &a(at(item));
		size_t index(MyVec::toIndex((myITEMID)item));//1
		if (a.uParentIndex)
		{
			const elt_t &b(at(a.uParentIndex));
			index -= MyVec::toIndex(b.uChildrenIndex);//children start
		}
		else
			index -= 1;
		return (unsigned)index;//unsigned(&a - start);
	}
	virtual int uniqueOf(ITEMID item) const
	{
		assert(isValidNotRoot(item));
		const elt_t &a(at(item));
		return a._key();
	}
	virtual ITEMID IdFromUnique(int iUnique) const
	{
		AVLTree<elt_t>::const_iterator i(m_tree.find(iUnique));
		if (i == m_tree.end())
			return 0;//invalid
		return i->uMyIndex;
	}
	virtual void rename(ITEMID item, const char *name)
	{
		const elt_t &a(at(item));
		Project_t &proj(mrCore.main().project());
		MyString s(proj.files().relPath(a.pFolder));
		mrCore.PostCommand(MyStringf("namfile %s %s", s.c_str(), name), true);
	}
protected:
	virtual void populateChildren(elt_t &aParent)
	{
		assert(aParent.uChildrenIndex == 0);
		assert(aParent.iChildrenNum < 0);
		//assert(isValid(parent));
		CFolderPtr pFolder(aParent.pFolder);
		assert(pFolder);
		FileFolder_t *p(pFolder->fileFolder());
		if (p)
		{
			//adcui::IFilesViewModel::ITEMID uChildrenIndex(size());
			const FoldersMap &m(p->children());
			for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
				eddElt(VALUE(i), adcui::FOLDERTYPE_FOLDER, aParent);
		}
		if (aParent.iChildrenNum < 0)
			aParent.iChildrenNum = 0;//mark as no children
	}

	bool isValidNotRoot(ITEMID row) const
	{
		assert(row);
		return MyVec::toIndex((myITEMID)row) < size();
	}

	bool isValid(ITEMID row) const
	{
		if (!row)
			return true;//root
		return isValidNotRoot(row);
	}
	void assureChildren(elt_t &a)
	{
		if (a.iChildrenNum < 0)
			const_cast<FilesViewModel_t *>(this)->populateChildren(a);
	}

	virtual MyString name(const elt_t &elt) const
	{
		CFolderPtr p(elt.pFolder);
		MyString s(p->name());
		if (!p->fileModule())
		{
			FileFolder_t *pf(p->fileFolder());
			while (pf)
			{
				const FoldersMap &m(pf->children());
				if (m.size() != 1)
					break;
				p = VALUE(m.begin());
				if ((pf = p->fileFolder()) != nullptr)
					s.append(p->name());
			}
		}
		return s;
		//const MyString &s0(elt.pFolder->m.name());
	//	size_t n(s0.rfind(MODULE_SEP));
		//if (n == MyString::npos)
			//n = s0.length();
		//while (n > 0 && (s0[n - 1] == '\\' || s0[n - 1] == '/'))
			//n--;
		//return s0.substr(0, n);
	}
	CFolderPtr skipSingles(CFolderPtr p)
	{
		assert(!p->fileModule());
		FileFolder_t *pf(p->fileFolder());
		while (pf)
		{
			const FoldersMap &m(pf->children());
			if (m.size() != 1)
				break;
			CFolderPtr p2(VALUE(m.begin()));
			pf = p2->fileFolder();
			if (!pf)
				break;
			p = p2;
		}
		return p;
	}

	virtual void lockRead(bool bLock)
	{
		/*miLocked += */mrCore.main().lockProjectRead(bLock);//can be recursive
	}
};

