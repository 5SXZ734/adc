#pragma once

#include "db/ui_names.h"
#include "db/ui_exports.h"

////////////////////////////////////////////
class NamesViewModelEx_t : public NamesViewModelBiased_t
{
	Dc_t &mrDC;
public:
	NamesViewModelEx_t(CoreEx_t &rCore, FolderPtr pNFolder)
		: NamesViewModelBiased_t(rCore, pNFolder),
		mrDC(*DCREF(ProjectInfo_t::ModuleOf(pNFolder)))
	{
	}
protected:
	virtual void writeFieldName(MyStreamUtil &ssu, CFieldPtr pField, bool bPretty) const
	{
		if (DcInfo_t::IsGlobal(pField) && bPretty)
		{
			DcInfo_t DI(mrDC);
			CGlobPtr iGlob(DcInfo_t::GlobObj(pField));
			MyString s;
			if (!DI.GetPrettyName(iGlob, s, CHOP_SYMB))
				s = "?";
			ssu.WriteString(s);
		}
		else
		{
			NamesViewModelBiased_t::writeFieldName(ssu, pField, bPretty);
		}
	}

	virtual void writeTypeName(MyStreamUtil &ssu, TypePtr pType, bool bPretty) const
	{
		assert(pType);
		DcInfo_t DI(mrDC);
		MyString s;
		if (bPretty)
		{
			if (!DI.GetPrettyName(pType, s, CHOP_SYMB))
				s = "?";
		}
		else
		{
			assert(pType->name());
			//s = pType->name()->c_str();
			ProjectInfo_t::ChopName(pType->name()->c_str(), s, CHOP_SYMB);
		}
		ssu.WriteString(s);
	}
	virtual TypePtr nameOwner(FieldPtr pField, bool bDuplicated) const
	{
		if (bDuplicated)
		{
			TypePtr pScope(DcInfo_s::OwnerScope(pField));
			if (pScope)
				return pScope;
		}
		return NamesViewModelBiased_t::nameOwner(pField, bDuplicated);
	}
	virtual unsigned indexOf(ITEMID item) const
	{
		ObjPtr pObj(toObjPtr(item));
		assert(pObj);
		PNameRef pn;
		NamesMgr_t *pns;
		FieldPtr pField(pObj->objField());
		assert(!pField);//only tpes can be the parents
		TypePtr pType(pObj->objTypeGlob());
		pns = ProjectInfo_t::OwnerNamesMgr(pType->owner(), nullptr);
		if (item & PTR_LSB)
		{
			DcInfo_t DI(mrDC);
			pn = DI.FindPrettyName(pType);
		}
		else
		{
			pn = pType->name();
		}
		return (unsigned)rankImpl(pns, pn);
	}
	
	/*virtual bool isPrettyName(CFieldPtr pField, PNameRef pn) const//(!) pn is in pField's scope!
	{
		//if (IsGlobal(pField))
		if (pField->userDataType() == FUDT_ GLOBAL)
		{
			CGlobPtr iGlob(DcInfo_t::GlobObj(pField));
			TypePtr pScope(iGlob->ownerScope1());
			if (pScope)
			{
				//if scoped - always a pretty name!
				//if (!pField->hasUgly Name())
					return true;
				//otherwise, pField's name and pn will be different (handled in base class) - glob may or may not have a pretty name - doesn't matter
				//iGlob->hasPrettyName()
			}
		}
		return NamesViewModelBiased_t::isPrettyName(pField, pn);
	}*/

	virtual bool goToDefinition(ITEMID item) const
	{
		ObjPtr pObj(toObjPtr(item));
		assert(pObj);
		FieldPtr pField(pObj->objField());
		FolderPtr pFolder(nullptr);
		if (pField)
		{
			//CGlobPtr iGlob(DcInfo_t::GlobObj(pField));
		}
		else
		{
			TypePtr pType(pObj->objTypeGlob());
		}

		if (pFolder)
		{
			bool bDefinition(false);

			MyString s(FilesMgr0_t::relPath(pFolder));
			assert(!s.empty());
			{
				s.append(bDefinition ? SOURCE_EXT : HEADER_EXT);
				//mrDC.project().main().postEvent(new adc::CEventCommand(MyStringf("show -@locus %s", checkQuotes(s).c_str())));
			}
		}
		return false;
	}
};



class ADCExportsViewModelEx : public ExportsViewModel_t
{
	Dc_t &mrDC;
public:
	ADCExportsViewModelEx(Core_t& rCore, FolderPtr pFolder)
		: ExportsViewModel_t(rCore, pFolder),
		mrDC(*DCREF(ProjectInfo_t::ModuleOf(pFolder)))
	{
	}
	virtual TypePtr nameOwner(FieldPtr pField, bool bDuplicated) const
	{
		if (bDuplicated)
		{
			TypePtr pScope(DcInfo_s::OwnerScope(pField));
			if (pScope)
				return pScope;
		}
		return ExportsViewModel_t::nameOwner(pField, bDuplicated);
	}
	virtual unsigned indexOf(ITEMID item) const
	{
		ObjPtr pObj(toObjPtr(item));
		assert(pObj);
		PNameRef pn;
		NamesMgr_t *pns;
		FieldPtr pField(pObj->objField());
		assert(!pField);//only tpes can be the parents
		TypePtr pType(pObj->objTypeGlob());
		pns = ProjectInfo_t::OwnerNamesMgr(pType->owner(), nullptr);
		if (item & PTR_LSB)
		{
			DcInfo_t DI(mrDC);
			pn = DI.FindPrettyName(pType);
		}
		else
		{
			pn = pType->name();
		}
		return (unsigned)rankImpl(pns, pn);
	}
};


