#pragma once

#include "files_ex.h"
#include "ui_main_ex.h"

/////////////////////////////////////////////// NamesViewModelBiased_t
class TemplViewModel0_t : public adcui::ITemplViewModel
{
protected:
	CoreEx_t& mrCore;
	FolderPtr mpFolder;
	Dc_t& mrDC;
public:
#ifdef _DEBUG
	enum { COL_ID, COL_SUBST, COL_SCOPE, COL_INCUMB, COL__TOTAL };
#else
	enum { COL_SUBST, COL_SCOPE, COL_INCUMB, COL__TOTAL };
#endif

	TemplViewModel0_t(CoreEx_t& rCore, FolderPtr pFolder)
		: mrCore(rCore),
		mpFolder(pFolder),
		mrDC(*DCREF(ProjectInfo_s::ModuleOf(mpFolder)))
	{
		assert(mpFolder->fileTempl());
	}
};

class TemplViewModel_t : public TemplViewModel0_t
{
public:
	TemplViewModel_t(CoreEx_t &rCore, FolderPtr pFolder)
		: TemplViewModel0_t(rCore, pFolder)
	{
	}

	virtual size_t count() const 
	{
		return templMap().size();
	}
	
	virtual unsigned flags(size_t index) const
	{
		assert(mrCore.hasProject());
		return 0;
	}

	virtual const char *moduleName()
	{
		return ProjectInfo_s::ModuleTitle(ProjectInfo_s::ModuleOf(mpFolder)).c_str();
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
			return !templMap().empty();
		return false;
	}

	virtual unsigned childrenNum(ITEMID parent) const
	{
		if (parent == 0)
			return (unsigned)templMap().size();
		return 0;
	}

	virtual int columnCount() const
	{
		return COL__TOTAL;
	}

	virtual void data(ITEMID item, size_t col, MyStreamBase &ss) const 
	{
		MyStreamUtil ssu(ss);
		if (item == 0)
		{
#ifdef _DEBUG
			if (col == COL_ID)
				ssu.WriteString("Id");
			else
#endif
			if (col == COL_SUBST)
				ssu.WriteString("Substitute");
			else if (col == COL_SCOPE)
				ssu.WriteString("Scope");
			else if (col == COL_INCUMB)
				ssu.WriteString("Incumbent");
			else
				ssu.WriteString("?");
			return;
		}
		DcInfo_t DI(mrDC);
		CObjPtr pObj((CObjPtr)item);
		CTypePtr pType(pObj->objType());
		CGlobPtr pGlob(pObj->objGlob());
#ifdef _DEBUG
		if (col == COL_ID)
		{
			ssu.WriteStringf("%X", pObj->ID());
		}
		else
#endif
		if (col == COL_SUBST)//substitute 
		{
			if (pType)
				ssu.WriteString(DI.TypePrettyName(pType, CHOP_SYMB));
			else
				ssu.WriteString(DI.GlobPrettyName0(pGlob, CHOP_SYMB));
		}
		else if (col == COL_SCOPE)//scope
		{
			TypePtr pScope(nullptr);
			if (pType)
			{
				if (pType->typeFuncDef())
					pScope = DcInfo_t::OwnerScope(DcInfo_t::DockField((CGlobPtr)pType));
				else if (pType->isNested())
					pScope = pType->owner();
			}
			else
			{
				pScope = pGlob->ownerScope();
			}
			if (pScope)
				ssu.WriteString(DI.TypePrettyNameFull(pScope, CHOP_SYMB).join());
		}
		else if (col == COL_INCUMB)//original
		{
			if (pType)
				ssu.WriteString(DI.TypeName0(pType));//CHOP_SYMB
			else
				ssu.WriteString(DI.SymbolName(DcInfo_s::DockField(pGlob)));
		}
	}

	virtual TypeEnum type(ITEMID item, size_t column) const
	{
		if (column == COL_SUBST)
		{
			CTypeBasePtr pObj((CTypeBasePtr)item);
			CTypePtr pType(pObj->objType());
			CGlobPtr pGlob(pObj->objGlob());
			if (pType)
			{
				assert(!pType->typeFuncDef());
			}
			else
			{
				//assert(pGlob->userDataType() == FUDT_ GLOBAL);
				{
					//CGlobPtr iGlob(DcInfo_t::GlobObj(pField));
					if (pGlob->func())
						return adcui::ITemplViewModel::E_FUNCTION;
					return adcui::ITemplViewModel::E_GLOBAL;
				}
			}
			return adcui::ITemplViewModel::E_TYPE;
		}
		return adcui::ITemplViewModel::E_UNK;
	}

	virtual void path(ITEMID item, MyStreamBase &ss) const
	{
		MyStreamUtil ssu(ss);
		if (item == 0)//module name request
			ssu.WriteString(ProjectInfo_s::ModuleTitle(mpFolder));
	}

	virtual ITEMID idOfChild(ITEMID parent, unsigned childIndex) const
	{
		assert(parent == 0);
		CObjPtr p(templMap().at(childIndex)->first);
		if (!p)
			return 0;
		return (ITEMID)p;
	}

	virtual ITEMID idOfParent(ITEMID child) const
	{
		CObjPtr pn((CObjPtr)child);
		assert(pn);
		return (ITEMID)0;
	}

	virtual unsigned indexOf(ITEMID item) const
	{
		TypeBasePtr p((TypeBasePtr)item);
		assert(p);
		return (unsigned)templMap().rank(p);
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
	const FileTempl_t &fileTempl() const {
		return *mpFolder->fileTempl();
	}

	const PrettyObjMap &templMap() const
	{
		return fileTempl().map();
	}
};

#if(0)
/////////////////////////////////////////////// ADCNamesViewModel2 (fields)
class ADCTemplViewModel2 : public TemplViewModel0_t
{
public:
	ADCTemplViewModel2(CoreEx_t &rCore, FolderPtr pFolder)
		: TemplViewModel0_t(rCore, pFolder)
	{
	}

	virtual size_t count() const 
	{
		return templFields().size();
	}
	
	virtual unsigned flags(size_t index) const
	{
		assert(mrCore.hasProject());
		return 0;
	}

	virtual const char *moduleName()
	{
		return ProjectInfo_s::ModuleTitle(ProjectInfo_s::ModuleOf(mpFolder)).c_str();
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
			return !templFields().empty();
		return false;
	}

	virtual unsigned childrenNum(ITEMID parent) const
	{
		if (parent == 0)
			return (unsigned)templFields().size();
		return 0;
	}

	virtual int columnCount() const
	{
		return COL__TOTAL;
	}

	virtual void data(ITEMID item, size_t col, MyStreamBase &ss) const 
	{
		MyStreamUtil ssu(ss);
		if (item == 0)
		{
#ifdef _DEBUG
			if (col == COL_ID)
				ssu.WriteString("Id");
			else
#endif
			if (col == COL_SUBST)
				ssu.WriteString("Substitute");
			else if (col == COL_SCOPE)
				ssu.WriteString("Scope");
			else if (col == COL_INCUMB)
				ssu.WriteString("Incumbent");
			else
				ssu.WriteString("?");
			return;
		}
		//const Project_t &proj(mrCore.project());
		DcInfo_t DI(mrDC);
		CFieldPtr pField((CFieldPtr)item);
#ifdef _DEBUG
		if (col == COL_ID)
		{
			ssu.WriteStringf("%X", pField->ID());
		}
		else
#endif
		if (col == COL_SUBST)//modified name
		{
			ssu.WriteString(DI.FieldName(pField, CHOP_SYMB));
		}
		else if (col == COL_SCOPE)
		{
			TypePtr iScope(DI.OwnerScope(pField));
			if (iScope)
				ssu.WriteString(DI.TypePrettyNameFull(iScope, CHOP_SYMB));
		}
		else if (col == COL_INCUMB)
		{
			ssu.WriteString(DI.SymbolName(pField));
/*			if (pField->userDataType() == FUDT_ GLOBAL)
			{
				//retrive the name from ProjectEx_t::mCxxImports (CxxSymbMap)
				if (!pField->nameless())
				{
					ssu.WriteString(DI.SymbolName(pField));
				}
			}
			else if (pField->userDataType() == FUDT_FUNC)
			{
				TypePtr iFuncDef(GlobFuncObj(pField));
				if (iFuncDef && !iFuncDef->nameless())
					ssu.WriteString(DI.TypeName0(iFuncDef));
			}*/
			//ssu.WriteString("<ugly name>");//pn->c_str()
		}
	}

	virtual TypeEnum type(ITEMID item, size_t column) const
	{
		if (column == COL_SUBST)
		{
			CFieldPtr pField((CFieldPtr)item);
			if (DcInfo_s::IsGlobal(pField))
			{
				CGlobPtr iGlob(DcInfo_t::GlobObj(pField));
				if (iGlob->func())
					return adcui::ITemplViewModel::E_FUNCTION;
				return adcui::ITemplViewModel::E_GLOBAL;
			}
			return adcui::ITemplViewModel::E_TYPE;
		}
		return adcui::ITemplViewModel::E_UNK;
	}

	virtual void path(ITEMID item, MyStreamBase &ss) const
	{
		MyStreamUtil ssu(ss);
		if (item == 0)//module name request
			ssu.WriteString(ProjectInfo_s::ModuleTitle(mpFolder));
	}

	virtual ITEMID idOfChild(ITEMID parent, unsigned childIndex) const
	{
		assert(parent == 0);
		CFieldPtr pf(templFields().at(childIndex)->first);
		if (!pf)
			return 0;
		return (ITEMID)pf;
	}

	virtual ITEMID idOfParent(ITEMID child) const
	{
		CFieldPtr p((CFieldPtr)child);
		assert(p);
		return (ITEMID)0;
	}

	virtual unsigned indexOf(ITEMID item) const
	{
		FieldPtr pf((FieldPtr)item);
		assert(pf);
		return (unsigned)templFields().rank(pf);
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
	const FileTempl_t &fileTempl() const {
		return *mpFolder->fileTempl();
	}

	const PrettyFieldsMap &templFields() const
	{
		return fileTempl().fields();
	}

};

#endif