#pragma once

#include <vector>

#include "ui_main.h"
#include "types.h"
#include "type_proxy.h"

class Core_t;
class MyStreamBase;

////////////////////////////////////////////////////////TypesViewModel_t - module agnostic
class TypesViewModel_t : public adcui::ITypesViewModel
{
	struct less_than_key
	{
		Project_t& m_proj;

		less_than_key(Project_t& proj)
			: m_proj(proj)
		{
		}

		int compare0(TypePtr p1, TypePtr p2)
		{
			if (p1 == p2)
				return 0;
			if (p1->nameless())
			{
				if (p2->nameless())
				{
					if (p1->typeSimple())
					{
						uint8_t t1(p1->typeSimple()->optype());
						if (p2->typeSimple())
						{
							uint8_t t2(p2->typeSimple()->optype());
							if (t1 < t2)
								return -1;//p1 < p2
							if (t1 == t2)
								return 0;
							return 1;//p1 > p2
						}
						return -1;//p1 < p2
					}
					else if (p2->typeSimple())
					{
						return 1;//p1 > p2
					}
					if (p1->typeProxy())
						p1 = p1->typeProxy()->incumbent();
					if (p2->typeProxy())
						p2 = p2->typeProxy()->incumbent();
					int i1(p1->typeComplex()->ID());
					int i2(p2->typeComplex()->ID());
					if (i1 < i2)
						return -1;
					if (i1 == i2)
						return 0;
					return 1;
				}
				return -1;//p1 < p2
			}
			else if (p2->nameless())
				return 1;//p1 > p2
			int res(strcmp(p1->name()->c_str(), p2->name()->c_str()));
			return res;
		}

		int compare(TypePtr p1, TypePtr p2)
		{
			int res;
			TypePtr q1(scope(p1));
			TypePtr q2(scope(p2));
			if (q1)
			{
				if (q2)
				{
					if ((res = compare(q1, q2)) != 0)
						return res;
				}
				else
				{
					if ((res = compare(q1, p2)) != 0)
						return res;
					return 1;//p2 is less
				}
			}
			else if (q2)
			{
				if ((res = compare(p1, q2)) != 0)
					return res;
				return -1;//p1 is less
			}

			return compare0(p1, p2);
		}

		inline bool operator() (TypePtr p1, TypePtr p2)
		{

			//CHECK(p1->checkId(0x1f) && p2->checkId(0xb2))
			//STOP
			if (compare(p1, p2) < 0)
			{
#if(0)
				if (p1->typeComplex() && p2->typeComplex())
				{
					MyString s1(fullName(p1));
					MyString s2(fullName(p2));
					assert(s1 < s2);
				}
#endif
				return true;
			}
			return false;
		}

		static TypePtr scope(TypePtr p)
		{
			TypePtr p0(p->owner());
			if (!p0)
				return nullptr;
			if (p0->typeSeg())
				return nullptr;
			return p0;
		}

		MyString fullName(TypePtr p)
		{
			ProjectInfo_t PI(m_proj);
			MyString s(PI.TypeName(p));
			while (p->owner() && !p->owner()->typeSeg())
			{
				s.prepend(PI.TypeName(p->owner()) + "::");
				p = p->owner();
			}
			return s;
		};

	};


protected:
	Core_t &mrCore;
	std::vector<TypePtr>	m_data;
public:
	TypesViewModel_t(Core_t &rCore)
		: mrCore(rCore)
	{
	}
	virtual ~TypesViewModel_t()
	{
	}

	void sort()
	{
		std::sort(m_data.begin(), m_data.end(), less_than_key(mrCore.project()));
	}

	/*void find(CTypePtr p)
	{
		std::vector<TypePtr>::const_iterator it(std::lower_bound(m_data.begin(), m_data.end(), p, less_than_key(mrCore.project())));
	}*/

protected:
	virtual int columnCount() const
	{
		return 2;
	}

	virtual void reset()
	{
		m_data.clear();
	}

	virtual void fetch(ITEMID) const
	{
	}

	virtual bool hasChildren(ITEMID parent, bool bFetched) const
	{
		if (parent == 0)
			return !m_data.empty();
		return false;
	}

	virtual unsigned childrenNum(ITEMID parent) const
	{
		if (parent == 0)
			return (unsigned)m_data.size();
		return 0;
	}

	virtual void data(size_t row, size_t column, MyStreamBase &ss) const
	{
		//if (!mrCore.hasProject())
		//return;
		if (!(row < m_data.size()))
			return;
		assert(mrCore.hasProject());
		ProjectInfo_t PI(mrCore.project());
		TypePtr p(m_data.at(row));
		MyString s(PI.TypeName(p, CHOP_SYMB));
		while (p->owner() && !p->owner()->typeSeg())
		{
			s.prepend(PI.TypeName(p->owner(), CHOP_SYMB) + "::");
			p = p->owner();
		}
		MyStreamUtil ssu(ss);
		ssu.WriteString(s);
	}

	virtual void path(ITEMID, MyStreamBase &) const 
	{
	}

	virtual ITEMID idOfChild(ITEMID parent, unsigned childIndex) const
	{
		if (childIndex < m_data.size())
			return (ITEMID)m_data.at(childIndex);
		return 0;
	}

	virtual ITEMID idOfParent(ITEMID child) const
	{
		return 0;
	}

	virtual unsigned indexOf(ITEMID item) const
	{
		return 0;
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

	// ITypesViewModel

	virtual unsigned flags(size_t index) const
	{
		assert(mrCore.hasProject());
		unsigned flags(adcui::ITypesViewModel::E_PRIMITIVE);
		TypePtr iType(m_data.at(index));
		if (iType->typeStrucvar())
			flags = adcui::ITypesViewModel::E_CONTEXT_DEPENDENT;
		else if (iType->typeCode())
			flags = adcui::ITypesViewModel::E_CODE;
		else if (iType->typeStruc())
		{
			if (iType->flags() & TYP_ENUM)
				flags = adcui::ITypesViewModel::E_ENUM;
			else
				flags = adcui::ITypesViewModel::E_COMPOUND;
		}
		else if (iType->typeTypedef())
			flags = adcui::ITypesViewModel::E_TYPEDEF;

		if (iType->hasUserData())
			flags |= adcui::ITypesViewModel::E_USERDATA;
		if (iType->typeProxy())
			flags |= adcui::ITypesViewModel::E_FWD;
		if (iType->owner()->typeProject())
			flags |= adcui::ITypesViewModel::E_ATTIC;
		return flags;
	}

protected:
	void pushTypes(TypesMgr_t &rTypeMgr)
	{
		for (TypesMapCIt it2(rTypeMgr.aliases().begin()); it2 != rTypeMgr.aliases().end(); it2++)
		{
			TypePtr pType(it2->pSelf);
			//if (!pType->typeArray() && !pType->typeArrayIndex() && !pType->typePtr() && !pType->typeEnum() && !pType->typeConst())
			if (pType->typeComplex())
			{
				m_data.push_back(pType);
			}
		}
	}
};







/////////////////////////////////////////////// TypesViewModel2_t
/*class TypesViewModel2_t : public TypesViewModel_t
{
public:
	TypesViewModel2_t(Core_t &rCore)
		: TypesViewModel_t(rCore)
	{
	}
	virtual const char *moduleName(){ return "?"; }
	virtual void reset()
	{
		TypesViewModel_t::reset();
		if (!mrCore.hasProject())
			return;

		Probe_t *pCtx(mrCore.project().getContext());
		if (pCtx)
		{

			ProjectInfo_t PI(mrCore.project());
			//MyStreamUtil ssh(ss);

			//std::set<std::string> aCheck;

			for (TypesMgr_t *pTypeMgr(pCtx->typesMap()); pTypeMgr; pTypeMgr = PI.superTypesMgr(pTypeMgr))
			{

				//MyString s;//("[" + (pOwner->typeModule() ? MyString("BINARY") : PI.TypeName(pOwner)) + "]");
				//ssh.WriteString(s);

				pushTypes(*pTypeMgr);
			}

			//ssh.WriteString("[EXTERNAL]");
			//mrMain.writeExternalTypes(ss);
			//m_data.push_back(pType);
			mrCore.project().releaseContext(pCtx);
		}
	}
};*/




/////////////////////////////////////////////// TypesViewModel3_t - module aware model
class TypesViewModel3_t : public TypesViewModel_t
{
protected:
	TypePtr miBinary;
public:
	TypesViewModel3_t(Core_t &rCore, TypePtr iModule)
		: TypesViewModel_t(rCore),
		miBinary(iModule)
	{
	}
	virtual const char *moduleName() const
	{
		return ProjectInfo_s::ModuleTitle(miBinary).c_str();
	}
	virtual void reset()
	{
		TypesViewModel_t::reset();
		if (!mrCore.hasProject())
			return;

		const std::set<TypePtr> &m(mrCore.project().typeMaps());
		for (std::set<TypePtr>::const_iterator i(m.begin()); i != m.end(); i++)
			pushTypes(*(*i)->typeMgr());

		ModuleInfo_t MI(mrCore.project(), *miBinary);
		FolderPtr pFolderTypes(MI.FolderOfKind(FTYP_TYPES));
		if (pFolderTypes)
		{
			FileTypes_t *rFile(pFolderTypes->fileTypes());

			const std::set<TypePtr> &m(rFile->typeMaps());
			for (std::set<TypePtr>::const_iterator i(m.begin()); i != m.end(); i++)
				pushTypes(*(*i)->typeMgr());
		}

		sort();
	}
	virtual bool apply(const char *typeStr)
	{
		adc::CEventCommand* p(new adc::CEventCommand(MyStringf("makeobj -f -t %s", typeStr), false));
		p->setContextZ(mrCore.getContext())->Release();
		mrCore.main().postEvent(p);
		return true;
	}
};

