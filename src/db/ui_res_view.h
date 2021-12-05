#pragma once

#include "inttypes.h"//PRIx64
#include "interface/IADCGui.h"
#include "qx/MyString.h"
#include "qx/MyRedirect2.h"
#include "mem.h"
#include "files.h"
#include "ui_main.h"
#include "shared/misc.h"
#include "info_module.h"

class MyStreamBase;
class ADCOutput;
class SSSRemoteCore;
class BinViewModel_t;
class Main_t;
class Project_t;
class DumpTarget_t;


////////////////////////////////////////////ResViewModel_t

class ResViewModel_t : public adcui::IResViewModel
{
protected:
	Core_t &mrCore;
	TypePtr miModule;

	typedef FileRes_t::tree_node_t	res_node_t;
	typedef MyTreeVectorElt<res_node_t>	elt_t;

	MyTreeVector<res_node_t>	m_data;

public:
	ResViewModel_t(Core_t &rCore, TypePtr iModule)
		: mrCore(rCore),
		miModule(iModule)
	{
	}

	virtual int columnCount() const
	{
		return 1;
	}
	
	virtual void reset()
	{
		m_data.clear();

		ReadLocker lock(mrCore.main());
		if (mrCore.hasProject() && miModule)
		{
			ModuleInfo_t MI(mrCore.project(), *miModule);
			FolderPtr pFolderRes(MI.FolderOfKind(FTYP_RESOURCES));
			if (pFolderRes)
			{
				m_data.addEltRoot(pFolderRes->fileRes()->tree().root());//NIL
			}
		}
	}

	virtual void fetch(ITEMID parent) const
	{
		if (m_data.isEmpty())
			return;
		const elt_t &a(const_cast<elt_t &>(m_data.at_(parent)));
		if (a.iChildrenNum < 0)
			const_cast<ResViewModel_t *>(this)->populateChildren(parent);
	}
	virtual bool hasChildren(ITEMID parent, bool bFetched) const
	{
		return m_data.hasChildren(parent, bFetched);
	}
	virtual unsigned childrenNum(ITEMID parent) const//children count
	{
		return m_data.childrenNum(parent);
	}

	virtual void data(ITEMID row, size_t, MyStreamBase &ss) const
	{
		res_node_t *pNode(m_data.data(row));
		MyStreamUtil ssu(ss);
		ssu.WriteString(pNode->name);
	}

	static MyString sep(adcui::FolderTypeEnum eType)
	{
		switch (eType)
		{
		case adcui::FOLDERTYPE_BINARY_EXE:
		case adcui::FOLDERTYPE_BINARY_DLL:
		case adcui::FOLDERTYPE_BINARY_PHANTOM:
			return MODULE_SEP;
		case adcui::FOLDERTYPE_FOLDER:
			return "/";
		default: break;
		}
		return "";
	}

	virtual void path(ITEMID item, MyStreamBase &ss) const
	{
		MyString s;
		for (;;)
		{
			const elt_t &a(m_data.at_(item));
			if (!s.empty())
			{
				s.prepend(sep(a.pNode->eType));
				s.prepend(a.pNode->name);
			}
			else
				s.prepend(MyStringf("@%08X", a.pNode->va));
			item = a.uParentIndex;
			if (!item)
				break;
		}
		s.prepend(MODULE_SEP);
		s.prepend(ProjectInfo_t::ModuleTitle(miModule));
		MyStreamUtil ssu(ss);
		ssu.WriteString(s);
	}

	virtual ITEMID idOfChild(ITEMID parent, unsigned childIndex) const
	{
		return m_data.idOfChild(parent, childIndex);
	}
	virtual ITEMID idOfParent(ITEMID child) const
	{
		return m_data.idOfParent(child);
	}
	virtual unsigned indexOf(ITEMID item) const
	{
		return m_data.indexOf(item);
	}
	virtual int uniqueOf(ITEMID item) const
	{
		assert(0);
		return 0;
	}
	virtual ITEMID IdFromUnique(int iUnique) const
	{
		assert(0);
		return 0;
	}
	virtual adcui::FolderTypeEnum type(ITEMID item) const
	{
		assert(m_data.isValidNotRoot(item));
		res_node_t *pNode(m_data.data(item));
		return pNode->eType;
	}
	virtual void rename(ITEMID, const char *){}
	virtual void viewPos(ITEMID item, MyStreamBase &ss) const
	{
		const elt_t &a(m_data.at_(item));
		TypePtr iSeg0(ProjectInfo_t::FindFrontSegIn(miModule));

		ADDR va(a.pNode->va);
		TypePtr iSeg(iSeg0->typeSeg()->findSubseg(va, iSeg0->typeSeg()->affinity()));
		if (iSeg)
		{
			MyStreamUtil ssu(ss);
			ROWID da(iSeg->typeSeg()->viewOffsAt(iSeg, va));
			char buf[32];
			sprintf(buf, "~%" PRIX64, da);
			ssu.WriteString(buf);
		}
	}
protected:
	void populateChildren(ITEMID item)
	{
		res_node_t *pNode(m_data.data(item));
		for (res_node_t *node(pNode->children); node; node = node->next)
			m_data.eddElt(node, item);
	}
};
