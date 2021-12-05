#pragma once

#include "files.h"
#include "ui_main.h"
#include "ui_names.h"

/////////////////////////////////////////////// ExportsViewModel_t

class ExportsViewModel_t : public NamesViewModel_t
{
public:
	ExportsViewModel_t(Core_t& rCore, FolderPtr pFolder)
		: NamesViewModel_t(rCore, pFolder)
	{
	}
protected:
	virtual int columnCount() const
	{
		return (int)adcui::ExportViewColumns::TOTAL;
	}
	virtual void writeHeaderData(size_t col, MyStreamUtil& ssu) const
	{
		switch (adcui::ExportViewColumns(col))
		{
		case adcui::ExportViewColumns::ORD:
			ssu.WriteString("Ordinal/Tag");
			break;
		case adcui::ExportViewColumns::VA:
			ssu.WriteString("VA");
			break;
		case adcui::ExportViewColumns::NAME:
			ssu.WriteString("Name");
			break;
		}
	}
	virtual void writeRowData(ITEMID item, size_t column, MyStreamUtil& ssu) const
	{
		ObjPtr pObj(toObjPtr(item));
		FieldPtr pField(pObj->objField());
		if (pField)
		{
			switch (adcui::ExportViewColumns(column))
			{
			case adcui::ExportViewColumns::VA:
				writeVA(pField, ssu);
				break;
			case adcui::ExportViewColumns::ORD://modified name
				if (pField->isExported())
					writeTag(pField, ssu);
				break;
			case adcui::ExportViewColumns::NAME:
				if (pField->isExported())
				{
					if (item & PTR_LSB)
						ssu.WriteString("<alias>");
					else
						writeName(pField, ssu);
				}
			default:
				break;
			}
		}
	}
	virtual bool goToDefinition(ITEMID item) const
	{
		ObjPtr pObj(toObjPtr(item));
		assert(pObj);
		FieldPtr pField(pObj->objField());
		return false;
	}
protected:
	void writeVA(FieldPtr pField, MyStreamUtil& ssu) const
	{
		ssu.WriteStringf("%08X", pField->_key());
	}
	void writeTag(FieldPtr pField, MyStreamUtil& ssu) const
	{
		int n;
		const char* pc(pField->name()->tag(n));
		if (pc)
			ssu.WriteString(MyString(pc, n));
	}
	void writeName(FieldPtr pField, MyStreamUtil& ssu) const
	{
		int n;
		const char* pc(pField->name()->name(n));
		assert(n >= 0);//always prefixed by length
		if (pc)
			ssu.WriteString(MyString(pc, n));
		else
			ssu.WriteString("<noname>");
	}
};




/////////////////////////////////////////////// ImportsViewModel_t

class ImportsViewModel_t : public ExportsViewModel_t
{
public:
	ImportsViewModel_t(Core_t& rCore, FolderPtr pFolder)
		: ExportsViewModel_t(rCore, pFolder)
	{
	}
protected:
	virtual int columnCount() const
	{
		return (int)adcui::ImportViewColumns::TOTAL;
	}
	virtual void writeHeaderData(size_t col, MyStreamUtil& ssu) const
	{
		switch (adcui::ImportViewColumns(col))
		{
		case adcui::ImportViewColumns::VA:
			ssu.WriteString("VA");
			break;
		case adcui::ImportViewColumns::MODULE:
			ssu.WriteString("Module");
			break;
		case adcui::ImportViewColumns::ORD:
			ssu.WriteString("Ordinal/Tag");
			break;
		case adcui::ImportViewColumns::NAME:
			ssu.WriteString("Name");
		default:
			break;
		}
	}
	virtual void writeRowData(ITEMID item, size_t column, MyStreamUtil& ssu) const
	{
		ObjPtr pObj(toObjPtr(item));
		FieldPtr pField(pObj->objField());
		if (!pField)
			return;

		switch (adcui::ImportViewColumns(column))
		{
		case adcui::ImportViewColumns::VA:
			writeVA(pField, ssu);
			break;
		case adcui::ImportViewColumns::MODULE:
			if (pField->isTypeImp())
				writeModule(pField, ssu);
			break;
		case adcui::ImportViewColumns::ORD://modified name
			if (pField->isTypeImp())
				writeTag(pField, ssu);
			break;
		case adcui::ImportViewColumns::NAME:
			if (pField->isTypeImp())
				writeName(pField, ssu);
		default:
			break;
		}
	}
private:
	void writeModule(FieldPtr pField, MyStreamUtil& ssu) const
	{
		int mid(pField->name()->mid());
		if (mid >= 0)
		{
			ProjectInfo_t PI(project());
			FolderPtr pFolder(PI.FindModuleFolderByUnique(mid));
			CTypePtr pModule(ProjectInfo_s::ModuleOf(pFolder));
			ssu.WriteString(ProjectInfo_s::ModuleTitle(pModule));
		}
	}
};



#if(0)
class ADCExportsViewModel : public adcui::IExportsViewModel
{
	Core_t &mrCore;
	FolderPtr mpFolder;

	enum { COL_ORD, COL_VA, COL_NAME, COL_TOTAL };
public:
	ADCExportsViewModel(Core_t &rCore, FolderPtr pFolder)
		: mrCore(rCore),
		mpFolder(pFolder)
	{
		assert(mpFolder->fileExports());
	}

	virtual ~ADCExportsViewModel()
	{
	}

	virtual size_t count() const 
	{
		return expMap().size();
	}
	
	virtual unsigned flags(size_t index) const
	{
		assert(mrCore.hasProject());
		return 0;
	}

	virtual const char *moduleName()
	{
		return ProjectInfo_t::ModuleTitle(ProjectInfo_t::ModuleOf(mpFolder)).c_str();
	}

protected:
	virtual void reset()
	{
	}

	virtual void fetch(ITEMID) const
	{
	}

	virtual bool hasChildren(ITEMID parent, bool bFetched) const
	{
		if (parent == 0)
			return !expMap().empty();
		return false;
	}

	virtual unsigned childrenNum(ITEMID parent) const
	{
		if (parent == 0)
			return (unsigned)expMap().size();
		return 0;
	}

	virtual int columnCount() const
	{
		return COL_TOTAL;
	}

	virtual void data(ITEMID item, size_t col, MyStreamBase &ss) const 
	{
		MyStreamUtil ssu(ss);
		if (item == 0)
		{
			if (col == COL_ORD)
				ssu.WriteString("Ordinal");
			else  if (col == COL_VA)
				ssu.WriteString("VA");
			else if (col == COL_NAME)
				ssu.WriteString("Name");
			else
				ssu.WriteString("?");
			return;
		}

		const ExportMap_t::inner_node& node(*(const ExportMap_t::inner_node*)item);
		if (col == COL_ORD)//modified name
		{
			ssu.WriteString(NumberToString(node.key));
		}
		else if (col == COL_VA)
		{
			ssu.WriteStringf("%08X", node.value.p->_key());
		}
		else if (col == COL_NAME)
		{
			if (node.value.p->name())
				ssu.WriteString(node.value.p->name()->c_str());
		}
	}

	virtual void path(ITEMID item, MyStreamBase &ss) const
	{
		MyStreamUtil ssu(ss);
		if (item == 0)//module name request
			ssu.WriteString(ProjectInfo_t::ModuleTitle(mpFolder));
	}

	virtual ITEMID idOfChild(ITEMID parent, unsigned childIndex) const
	{
		assert(parent == 0);
		const ExportMap_t::inner_node& pf(expMap().at(childIndex));
		//if (!pf)
			//return 0;
		return (ITEMID)&pf;
	}

	virtual ITEMID idOfParent(ITEMID child) const
	{
		CFieldPtr p((CFieldPtr)child);
		assert(p);
		return (ITEMID)0;
	}

	virtual unsigned indexOf(ITEMID item) const
	{
		const ExportMap_t::inner_node* pf((const ExportMap_t::inner_node*)item);
		assert(pf);
		return (unsigned)expMap().rank(pf->key);
	}

	virtual int uniqueOf(ITEMID) const
	{
		return 0;
	}

	virtual ITEMID IdFromUnique(int) const
	{
		return 0;
	}

	virtual void rename(ITEMID, const char *)
	{
	}

private:
	const FileExports_t &fileExp() const {
		return *mpFolder->fileExports();
	}

	const ExportMap_t &expMap() const
	{
		return fileExp().expMap();
	}

};
#endif


