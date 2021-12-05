#include "print.h"
#include "prefix.h"

#include "proj.h"
#include "type_strucvar.h"
#include "types_mgr.h"
#include "type_code.h"
#include "field.h"
#include "names.h"
#include "shared/misc.h"

#define PRINT_FIELD_ID		0
#define PRINT_TYPE_ID		0
#define PRINT_TYPE_REFS		0
#define PRINT_NAME_REFS		0
#define PRINT_TYPE_ALIASES	0

using namespace std;

void HierPrinter::print(ostream &os, const Obj_t &rSelf, int l) const
{
	assert(0);//? os << TABS(l) << "{" << rSelf.mID << "}" << ObjN ame(&rSelf);
}

MyString HierPrinter::typeNameEx(TypePtr pSelf, char chopSymb) const
{
	Array_t *pArray(pSelf->typeArray());
	if (pArray)
	{
		MyString s(typeNameEx(pArray->baseType(), chopSymb));
		return MyStringf("%s[%d]", s.c_str(), pArray->total());
	}

	TypePtr_t *pPtr(pSelf->typePtr());
	if (pPtr)
	{
		MyString s;
		if (pPtr->typeImp())
		{
			pPtr->namex(s);
			return s;
		}
		if (pPtr->typeRef())
			s = "&";
		else if (pPtr->typeRRef())
			s = "&&";
		else
			s = "*";
		if (pPtr->pointee())
			return typeNameEx(pPtr->pointee(), chopSymb) + s;
		return TypeID2Str(OPTYP_NULL) + s;
	}

	TypeConst_t* pConst(pSelf->typeConst());
	if (pConst)
	{
		MyString s(typeNameEx(pConst->baseType(), chopSymb));
		return "const " + s;
	}

	MyString s;
	TypeCode_t *pCode(pSelf->typeCode());
	if (pCode)
	{
		pCode->namex(s);
		return s;
	}

	Strucvar_t *pStrucvar(pSelf->typeStrucvar());
	if (pStrucvar)
	{
		pStrucvar->namex(s);
		return s;
	}

	ProjectInfo_t::ChopName(mrProject.typeName(pSelf), s, chopSymb);
	return s;
}

void HierPrinter::printType(ostream &os, TypePtr pSelf, int l) const
{
	os << TABS(l);
#if(PRINT_TYPE_ID)
	os << "<" << pSelf->ID() << ">";
#endif
	MyString s(typeNameEx(pSelf, CHOP_SYMB));
//CHECK(s == "struc_3448")
//STOP
	if (pSelf->typeTypedef())
		s = MyStringf("typedef(%s)", s.c_str());
	os << s;
	if (pSelf->typeProxy())
	{
//CHECK(TypeName(pSelf->typeProxy()->incumbent()).startsWith("_Lockit"))
//STOP
		os << "{proxy}";
	}
}

HierPrinter::HierPrinter(ProjectInfo_t &r)
	: ProjectInfo_t(r)
{
}

void HierPrinter::printContentsSeg(ostream &os, TypePtr iSelf, int l) const
{
	const Seg_t &rSelf(*iSelf->typeSeg());

	if (rSelf.traceLink())
		os << TABS(l) << "{ trace: " << TypeName(rSelf.traceLink()) << " }" << endl;

	printContentsStruc(os, iSelf, l);

	/*if (!rSelf.conflictFields().empty())
	{
		os << TABS(l) << "extra [" << endl;
		l++;
		for (ClonedFieldMapCIt it(rSelf.conflictFields().begin()); it != rSelf.conflictFields().end(); it++)
		{
			ADDR addr(KEY(it));
			CFieldPtr pField(VALUE(it));
			print(os, *pField, l);
		}
		--l;
		os << TABS(l) << "]" << endl;
	}*/

	for (SubSegMapCIt i(rSelf.subsegs().begin()); i != rSelf.subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));
		MyString sName(TypeName(iSeg));
		os << TABS(l) << "subseg " << sName << " [" << endl;
		l++;
		printContentsSeg(os, iSeg, l);
		--l;
		os << TABS(l) << "] " << sName << endl;
	}
}

void HierPrinter::printSeg(ostream &os, TypePtr iSelf, int l) const
{
	const Seg_t &rSelf(*iSelf->typeSeg());
	if (FrontEnd(iSelf))
	{
		os << TABS(l) << "frontend [" << endl;
		std::string sFront(FrontEnd(iSelf)->name());
		//MyPath fFront(mrMain.frontEnd2Path(rSelf.frontEnd()->name()));
		//if (!fFront.IsNull())
			//sFront = fFront.Name();
		os << TABS(l + 1) << sFront << endl;
		os << TABS(l) << "]" << endl;
	}

	std::string name;
	if (iSelf->isShared() || !iSelf->parentField())
	{
		if (!rSelf.nameless())
			name += "->";
		name += TypeName(iSelf);
	}
	else
	{
		name = ";";
		if (rSelf.traceLink())
			name += FieldName(rSelf.traceLink()->parentField());
		else
			name += FieldName(iSelf->parentField());
	}
	os << TABS(l);
#if(PRINT_TYPE_ID)
	os << "<" << iSelf->ID() << ">";
#endif
	if (name.empty())
		name = "<noname>";
	os << rSelf.printType() << " " << name << endl;
	os << TABS(l) << "{" << endl;
	l++;
	printContentsSeg(os, iSelf, l);
	--l;
	os << TABS(l) << "}";
}

void HierPrinter::printStruc(ostream &os, TypePtr iSelf, int l) const
{
	const Struc_t &rSelf(*iSelf->typeStruc());
	std::string name;

	if (iSelf->isShared() || !iSelf->parentField())
	{
//?		if (!rSelf.nameless())
	//?		name += "->";
		name += TypeName(iSelf, CHOP_SYMB);
	}
	else
	{
		name = ";";
		name += FieldName(iSelf->parentField(), CHOP_SYMB);
	}
	os << TABS(l);
#if(PRINT_TYPE_ID)
	os << "<" << iSelf->ID() << ">";
#endif
	os << rSelf.printType() << " " << name << endl;
	os << TABS(l) << "{" << endl;
	l++;
	printContentsStruc(os, iSelf, l);
	--l;
	os << TABS(l) << "}";
}

void HierPrinter::printContentsStruc(ostream &os, TypePtr iSelf, int l) const
{
	const Struc_t &rSelf(*iSelf->typeStruc());
	if (rSelf.namesMgr())
	{
		os << TABS(l) << "namespace [" << endl;
		print(os, *rSelf.namesMgr(), l + 1);
		os << TABS(l) << "]" << endl;
	}
	if (rSelf.typeMgr())
	{
		os << TABS(l) << "typesmgr [" << endl;
		print(os, *rSelf.typeMgr(), l + 1);
		os << TABS(l) << "]" << endl;
	}
	for (FieldMapCIt I = rSelf.fields().begin(), E = rSelf.fields().end(); I != E; I++)
	{
		ADDR addr(KEY(I));
		CFieldRef rField(*I);
		print(os, rField, l);
	}
}

void HierPrinter::printUnion(ostream &os, TypePtr iSelf, int l) const
{
	const Struc_t &rSelf(*iSelf->typeStruc());
	string name;
	if (iSelf->isShared() || !rSelf.nameless())
		name = TypeName(iSelf, CHOP_SYMB);
	//	else
	//	name = Name(parentField());

	os << TABS(l);
#if(PRINT_TYPE_ID)
	os << "<" << iSelf->ID() << ">";
#endif
	os << "union";// rSelf.printType();
	if (!name.empty())
		os << " " << name;
	os << endl;
	os << TABS(l) << "{" << endl;
	l++;
	printContentsUnion(os, iSelf, l);
	--l;
	os << TABS(l) << "}";
}

void HierPrinter::printContentsUnion(std::ostream &os, TypePtr iSelf, int l) const
{
	const Struc_t &rSelf(*iSelf->typeStruc());
	if (rSelf.namesMgr())
	{
		os << TABS(l) << "namespace [" << endl;
		print(os, *rSelf.namesMgr(), l + 1);
		os << TABS(l) << "]" << endl;
	}
	for (FieldMapCIt I = rSelf.fields().begin(), E = rSelf.fields().end(); I != E; I++)
	{
		CFieldRef rField(*I);
		print(os, rField, l);
	}
}

void HierPrinter::print(ostream &os, const TypesMgr_t &rSelf, int l) const
{
	//sort it first
	std::map<MyString, std::pair<TypePtr, MyString> > m;
	for (TypesMapCIt it(rSelf.aliases().begin()); it != rSelf.aliases().end(); it++)
	{
		TypePtr iType(it->pSelf);
//CHECKID(iType, 0x71b)
//STOP
		MyString s(typeNameEx(iType, CHOP_SYMB));
		if (!m.insert(std::make_pair(s, std::make_pair(iType, it->_key()))).second)
		{
			if (iType->typeTypedef())
				s = MyStringf("typedef(%s)", s.c_str());
//CHECK(s == "VOID*")
//STOP
			MyString sfx;
			for (int i(1); !m.insert(std::make_pair(s + sfx, std::make_pair(iType, it->_key()))).second; i++)
				sfx = MyStringf(" ~%d!", i);
		}
	}

	//for (TypesMapCIt it(rSelf.aliases().begin()); it != rSelf.aliases().end(); it++)
	for (std::map<MyString, std::pair<TypePtr, MyString> >::const_iterator it(m.begin()); it != m.end(); it++)
	{
		TypePtr iType(it->second.first);
		printTypeRef(os, iType, l);
		//os << TABS(l) << pType->na mexx() << endl;
#if(PRINT_TYPE_REFS)
		os << " {" << rSelf.typeRefsNum(iType) << "}";
#endif
#if(PRINT_TYPE_ALIASES)
		os << " `" << it->second.second << "'";
#endif
		os << endl;
	}
}

void HierPrinter::print(ostream &os, const NamesMgr_t &rSelf, int l) const
{
	if (rSelf.isInitialized())
	{
		for (NamesMapCIt it(rSelf.namesMap().begin()); it != rSelf.namesMap().end(); it++)
		{
			CNamePtr ps(it->c_str());
			MyString s;
			ChopName(ps, s, CHOP_SYMB);
			os << TABS(l) << s;
#if(PRINT_NAME_REFS)
			os << " {" << ps->refsNum() << "}";
#endif
			os <<  endl;
		}
	}
}


void HierPrinter::printTypeRef(ostream &os, TypePtr iSelf, int l) const
{
	const TypeObj_t &rSelf(*iSelf);
	switch (rSelf.pvt().ObjType())
	{
	case OBJID_TYPE_MODULE:
	case OBJID_TYPE_SEG:
		printSeg(os, iSelf, l);
		return;
	case OBJID_TYPE_STRUC:
	case OBJID_TYPE_CLASS:
		if (iSelf->typeStruc()->isUnion(iSelf))
		{
			printUnion(os, iSelf, l);
			return;
		}
	case OBJID_TYPE_PROC:
	case OBJID_TYPE_STRUCLOC:
	case OBJID_TYPE_NAMESPACE:
		printStruc(os, iSelf, l);
		return;
/*	case OBJID_TYPE_UNION:
#if(!NEW_LOCAL_VARS)
	case OBJID_TYPE_UNIONLOC:
#endif
		printUnion(os, iSelf, l);
		return;*/
	default:
		printType(os, iSelf, l);
	}
}

void HierPrinter::print(ostream &os, CFieldRef rSelf, int l) const
{
	MyString name;
	/*if (rSelf.isCloneMaster())
		name += "<clone-master>";*/
	if (!rSelf.nameless())
		name += rSelf.name()->c_str();
	else if (!rSelf.type() || !rSelf.type()->typeUnion() || rSelf.type()->isShared())
		name += FieldName(&rSelf, CHOP_SYMB);

	if (rSelf.type())
	{
		if (!rSelf.type()->isShared())
			printTypeRef(os, rSelf.type(), l);
		else
			os << TABS(l) << typeNameEx(rSelf.type(), CHOP_SYMB);
	}
	else if (IsEosField(&rSelf))
		os << TABS(l) << "<eos> ";
	else
		os << TABS(l) << "<notype> ";
#if(PRINT_FIELD_ID)
	os << "<" << rSelf.ID() << ">";
#endif
	if (!name.empty())
		os << " " << name;
	os << endl;
}

void HierPrinter::print(ostream &os)
{
	printSeg(os, mrProject.self(), 0);
	os << std::endl;

	const FoldersMap &m(mrProject.rootFolder().children());
	for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
	{
		CFolderRef rFolder(*i);
		//if (!IsPhantomFolder(rFolder))
			printSeg(os, rFolder.fileModule()->module(), 0);
			os << std::endl;
	}
}
