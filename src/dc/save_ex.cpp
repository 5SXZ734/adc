#include "save_ex.h"
#include "prefix.h"
#include <fstream>

#include "qx/MyFileMgr.h"
#include "shared/data_source.h"
#include "stubs.h"
#include "info_dc.h"
#include "type_funcdef.h"
#include "db/type_proxy.h"
#include "info_func.h"
#include "proj_ex.h"
#include "savex_impl.h"

//using namespace std;

//////////////////////////////////
// FileSerializer_t

#if(FUNC_SAVE_ENABLED)
void FileSerializerEx_t::setDeferredTypes(std::vector<CTypePtr> &v)
{
	mDeferredTypes.clear();
	mDeferredTypes.reserve(v.size());//v is expected to be 1-biased
	for (size_t i(0); i < v.size(); i++)
		mDeferredTypes.add(v[i]);
}
#endif



/////////////////////Ins_t

void FileSerializer_t::saveInstr(std::ostream &os, const Ins_t &rSelf)
{
#if(FUNC_SAVE_ENABLED)
	//writeIdx(os, pathToIdx(rSelf.mpPath));
	write(os, rSelf.VA());
	write(os, rSelf.mAction);
	write(os, rSelf.mFlags);
	write(os, rSelf.mPStackIn);
	write(os, rSelf.mPStackDiff);
	write(os, rSelf.mFStackIn);
	write(os, rSelf.mFStackDiff);
	write(os, rSelf.mEFlagsTested);
	write(os, rSelf.mEFlagsModified);
	write(os, rSelf.mFFlagsAffected);
	writeIdx(os, opToIdx(rSelf.mArgs.Head()));
#endif
}


void FileSerializer_t::loadInstr(std::istream &is, Ins_t &rSelf)
{
#if(FUNC_SAVE_ENABLED)
	//rSelf.mpPath = pathFromIdx(readIdx(is));
	ADDR va;
	read(is, va);
	rSelf.setVA(va);
	read(is, rSelf.mAction);
	read(is, rSelf.mFlags);
	read(is, rSelf.mPStackIn);
	read(is, rSelf.mPStackDiff);
	read(is, rSelf.mFStackIn);
	read(is, rSelf.mFStackDiff);
	read(is, rSelf.mEFlagsTested);
	read(is, rSelf.mEFlagsModified);
	read(is, rSelf.mFFlagsAffected);
	rSelf.mArgs = opFromIdx(readIdx(is));
#endif
}


////////////////////////////////////////////////////////Op_t

static ObjFlagsType setupOpFlags(HOP op)
{
	ObjFlagsType flags(op->flags());
	assert((flags >> 16) == 0);//upper half must be clean
	if (op->m_pRI->mpPrimeOp == op)
	{
		flags |= OPND_EX_RI;
//#if(NEW_OP)
	//	if (op->ins().mpPath->headOp() == &op)
		//	RI = 2;
//#endif
		if (op->m_pRI->mpPathRef)
			flags |= OPND_EX_LABELREF;
	}
	if (op->m_disp)
		flags |= OPND_EX_DISP;
	if (op->m_disp0)
		flags |= OPND_EX_DISP0;
	if (!op->m_xin.empty())
		flags |= OPND_EX_XIN;
	if (op->localRef())
		flags |= OPND_EX_LOCALREF;
	return flags;
}


/*struct siRI_t
{
	uint16_t	RI_FLAGS : 1;		//0
	uint16_t	ACTION : 1;
	uint16_t	FPUIN : 1;
	uint16_t	FPUDIFF : 1;
	uint16_t	STACKIN : 1;		//4
	uint16_t	STACKDIFF : 1;
	uint16_t	EFLAGS_T : 1;
	uint16_t	EFLAGS_M : 1;
	uint16_t	FPUFLAGS : 1;		//8
	uint16_t	ARGS : 2;
	uint16_t	LABELREF : 1;
	uint16_t	__13 : 4;		//12
};*/

void FileSerializer_t::saveOp(std::ostream &os, HOP hSelf)
{
	const Op_t &rSelf(*hSelf);
#if(FUNC_SAVE_ENABLED)
//OS_CHECK(z);
//CHECK(rSelf.mpFieldRef)
CHECKID((&rSelf), 0x1a23)
STOP
	ObjFlagsType flags(setupOpFlags(hSelf));

	//Base_t::save(os, rSelf.asObj());
	write(os, flags);

	writeIdx(os, opToIdx(rSelf.pNext));
/*#if(!NEW_OP)
	writeIdx(os, opToIdx(rSelf.m_pRoot));
#else
	assert(0);//recover required
#endif*/
	if (flags & OPND_EX_RI)
	{
		const Ins_t &ins(rSelf.ins());
		saveInstr(os, ins);
//		if (flags.RI == 2)
	//		writeIdx(os, pathToIdx(ins.mpPath));
//?		if (flags & OPND_EX_LABELREF)
//?			writeIdx(os, pathToIdx(ins.mpPathRef));
			//writeIdx(os, fieldRefToIdx(ins.mpLabelRef));
	}
	write(os, rSelf.m_opc);
	write(os, rSelf.mOffs);
	write(os, rSelf.m_optyp);
	//write(os, rSelf.m_seg);
	if (flags & OPND_EX_DISP)
		write(os, rSelf.m_disp);
	if (flags & OPND_EX_DISP0)
		write(os, rSelf.m_disp0);
	if (flags & OPND_EX_XIN)
	{
		for (XOpList_t::Iterator i(rSelf.m_xin); i; i++)
			writeIdx(os, opToIdx(i.data()));
		writeEos(os);
	}

	if (flags & OPND_EX_LOCALREF)
		writeIdx(os, fieldRefToIdx(rSelf.localRef()));
#endif
}

void FileSerializer_t::loadOp(std::istream &is, HOP hSelf, bool bForceOrder)
{
	Op_t &rSelf(*hSelf);
#if(FUNC_SAVE_ENABLED)
//IS_CHECK(z);
//CHECK(z==0x152bf)
CHECKID(hSelf, 0x1a23)
STOP

	//Base_t::load(is, rSelf.asObj());
	ObjFlagsType flags;
	read(is, flags);
	rSelf.m_nFlags = flags & OPND_EX_RESERVED;

	rSelf.pNext = opFromIdx(readIdx(is));
/*#if(!NEW_OP)
	rSelf.m_pRoot = opFromIdx(readIdx(is));
#endif*/
	if (flags & OPND_EX_RI)
	{
		rSelf.setInsPtr(memMgrEx().NewRootInfo());
		loadInstr(is, *rSelf.m_pRI);
/*		if (flags.RI == 2)
		{
			PathPtr pPath(pathFromIdx(readIdx(is)));
			pPath->m.mOps = OpPtr(hSelf);
		}*/
		rSelf.ins().mpPrimeOp = hSelf;
#ifdef _DEBUG
		rSelf.__isPrime = true;
#endif
		if (flags & OPND_EX_LABELREF)
		{
			//rSelf.setPathRef(fieldRefFromIdx(readIdx(is)));
//?			rSelf.setPathRef(pathFromIdx(readIdx(is)));
/*?			if (rSelf.pathRef())
			{
				XOpLink_t *pXRef(reinterpret_cast<MemoryMgrEx_t &>(mrMemMgr).NewXOpLink(hSelf));
				rSelf.pathRef()->m.pushRefBack(pXRef);
			}*/
		}
	}
	read(is, rSelf.m_opc);
	read(is, rSelf.mOffs);
	read(is, rSelf.m_optyp);
	//read(is, rSelf.m_seg);
	if (flags & OPND_EX_DISP)
		read(is, rSelf.m_disp);
	if (flags & OPND_EX_DISP0)
		read(is, rSelf.m_disp0);
	if (flags & OPND_EX_XIN)
	{
		for (;;)
		{
			HOP hOp(opFromIdx(readIdx(is)));
			if (!hOp)
				break;
CHECKID(hOp, 0x15ff)
STOP
			XOpList_t& xin(rSelf.m_xin);
			XOpLink_t *pLink(memMgrEx().NewXOpLink2());
			pLink->setData(hOp);
			if (!bForceOrder || !xin || xin.tail()->data() < hOp)
			{
				assert(!xin || xin.tail()->data() < hOp);//xlinks must be sorted!
				xin.push_back(pLink);
			}
			else
				xin.insert(pLink, xin.lower_bound(hOp));

			XOpList_t& xout(hOp->m_xout);
			XOpLink_t *pLink2(&pLink[1]);
			pLink2->setData(hSelf);
			if (!bForceOrder || !xout || xout.tail()->data() < hSelf)
			{
				assert(!xout || xout.tail()->data() < hSelf);//xlinks must be sorted!
				xout.push_back(pLink2);
			}
			else
				xout.insert(pLink2, xout.lower_bound(hOp));
		}
	}

	if (flags & OPND_EX_LOCALREF)
	{
		rSelf.setLocalRef0(fieldRefFromIdx(readIdx(is)));
		if (rSelf.localRef())
		{
			XOpLink_t *pXRef(memMgrEx().NewXOpLink());
			pXRef->setData(hSelf);
			FuncInfo_t::PushLocalRefBack(rSelf.localRef(), pXRef);
		}
	}
#endif
}



//////////////////////////////////////////////////////// Path_t

void FileSerializer_t::savePath(std::ostream &os, HPATH hSelf)
{
	const Path_t &rSelf(*hSelf);
#if(FUNC_SAVE_ENABLED)
//CHECK(rSelf.Type() == BLK_ENTER)
#if(PATH_ID)
CHECK(rSelf.m.mID == 0xe0)
STOP
#endif
	writeIdx(os, pathToIdx(rSelf.pNext));
	writeIdx(os, pathToIdx(rSelf.FirstChild()));

	uint32_t flags(rSelf.flags());
	/*if (rSelf.anchor() != 0)
		flags |= PATH_EX_HAS_ANCHOR;*/
	if (rSelf.hasOps())
		flags |= PATH_EX_HAS_OPS;
	if (rSelf.hasRefs())
		flags |= PATH_EX_HAS_REFS;

	write(os, flags);

	/*if (flags & PATH_EX_HAS_ANCHOR)
		write(os, rSelf.anchor());*/
	
	if (flags & PATH_EX_HAS_OPS)
		writeIdx(os, opToIdx(rSelf.ops().Head()));

	if (flags & PATH_EX_HAS_REFS)
	{
		for (XOpList_t::Iterator i(rSelf.inflow()); i; i++)
			writeIdx(os, opToIdx(i.data()));
		writeEos(os);//sequence end
	}

#endif
}

void FileSerializer_t::loadPath(std::istream &is, HPATH hSelf)
{
	Path_t &rSelf(*hSelf);
#if(FUNC_SAVE_ENABLED)
#if(PATH_ID)
CHECK(rSelf.mID == 0xe0)
STOP
#endif
	rSelf.SetNext(pathFromIdx(readIdx(is)));
	rSelf.SetChildren(pathFromIdx(readIdx(is)));

	uint32_t flags;
	read(is, flags);
	rSelf.setFlags(flags & PATH_EX_RESERVED);

	/*if (flags & PATH_EX_HAS_ANCHOR)
	{
		ADDR anchor;
		read(is, anchor);
		rSelf.setAnchor(anchor);
	}*/

	if (flags & PATH_EX_HAS_OPS)
		rSelf.setOps(opFromIdx(readIdx(is)));

	if (flags & PATH_EX_HAS_REFS)
	{
		for (;;)
		{
			HOP hOp(opFromIdx(readIdx(is)));
			if (hOp == HOP())
				break;
			XOpLink_t *pXRef(memMgrEx().NewXOpLink());
			pXRef->setData(hOp);
			rSelf.pushRefBack(pXRef);
		}
	}

	//if (rSelf.LabelAt())
		//FuncInfo_t::SetLabelPath(rSelf.LabelAt(), PathPtr(hSelf));

//CHECK(rSelf.Type() == BLK_ENTER)
//STOP
#endif
}

/////////////////////////////////////////////////

bool FileSerializer_t::writeType(ObjId_t objId, std::ostream &os, CTypePtr pType)
{
	assert(objId == pType->ObjType());
	switch (pType->ObjType())
	{
	case OBJID_TYPE_STRUCLOC:
		saveStruc(os, pType);
		break;
#if(!NEW_LOCAL_VARS)
	case OBJID_TYPE_UNIONLOC:
		saveUnion(os, pType);
		break;
#endif
	default:
		return Base_t::writeType(objId, os, pType);
	}
	return true;
}

Type_t *FileSerializer_t::loadType(ObjId_t objId, std::istream &is, TypePtr pTypeObj)
{
	Type_t *pType(nullptr);
	switch (objId)
	{
	case OBJID_TYPE_STRUCLOC:
		pType = new TypeStrucLoc_t;
		pTypeObj->SetPvt(pType);
		loadStruc(is, pTypeObj);
		break;
#if(!NEW_LOCAL_VARS)
	case OBJID_TYPE_UNIONLOC:
		pType = new TypeUnionLoc_t;
		pTypeObj->SetPvt(pType);
		loadUnion(is, pTypeObj);
		break;
#endif
	default:
		pType = Base_t::loadType(objId, is, pTypeObj);
	}
	return pType;
}


///////////////////////////////////////////////////////////////// FuncDef_t

void FileSerializer_t::saveFunction(std::ostream &os, CGlobPtr iSelf)
{
	const FuncDef_t &rFuncDef(*iSelf->typeFuncDef());

	//get a locals top
#if(NEW_LOCAL_VARS)
	TypePtr iLocals(rFuncDef.locals());
#else
	TypePtr iLocals(nullptr);
	FieldPtr pField(iSelf->parentField());
	if (pField)
	{
		iLocals = pField->OwnerComplex();
		assert(iLocals->typeUnion() && !iLocals->parentField());//locals top
	}
#endif
	writeIdx(os, typeToIdx(iLocals));

#if(FUNC_SAVE_ENABLED)
	writeIdx(os, pathToIdx(rFuncDef.pathTree().body()));
	writeIdx(os, opToIdx(rFuncDef.mCallRets.Head()));
#endif
}

TypePtr FileSerializer_t::loadFunction(std::istream &is, FuncDef_t &rFuncDef)
{
	TypePtr iLocals(typeFromIdx(readIdx(is)));
#if(NEW_LOCAL_VARS)
	rFuncDef.setLocals(iLocals);
#endif

#if(FUNC_SAVE_ENABLED)
	rFuncDef.pathTree().setBody(pathFromIdx(readIdx(is)));
	rFuncDef.mCallRets = opFromIdx(readIdx(is));
#endif

	return iLocals;
}

void GlobalSerializerEx_t::saveFuncDef(std::ostream &os, CTypePtr iSelf)
{
	const FuncDef_t &rSelf(*iSelf->typeFuncDef());
	assert(rSelf.ObjType() == OBJID_TYPE_FUNCDEF);
CHECK(_CP == 0x26921)
STOP
	StrucSerializer_t::saveStruc(os, iSelf);

//	saveFields(os, rSelf.retFields());

	//writeIdx(os, typeToIdx(rSelf.OwnerFuncRef()));
//	write(os, fieldToIdx(rSelf.thisPtr()));
	//write(os, rSelf.mSpoiltRegs);
	write(os, rSelf.m_stackOut);
	write(os, rSelf.mSpoiltFlags);
	write(os, rSelf.m_nFPUOut);
}

void GlobalSerializerEx_t::loadFuncDef(std::istream &is, TypePtr iSelf)
{
	FuncDef_t &rSelf(*iSelf->typeFuncDef());
CHECK(_CP == 0x26921)
STOP
	assert(rSelf.ObjType() == OBJID_TYPE_FUNCDEF);
	bool bLocal(setLocalFlag(true));//must be called before loadStruc! this means - there is no global field refs(!)

	loadStruc(is, iSelf);

/*	unsigned retsNum;
	read(is, retsNum);
	while (retsNum--)
	{
		//CHECK(fieldsNum == 6)
		//STOP
		FieldPtr pField(loadField(is, rSelf.retFields()));
		pField->setOwnerComplex(iSelf);
	}*/

	setLocalFlag(bLocal);//restore

//	rSelf.setThisPtr(fieldFromIdx(is));
	//read(is, rSelf.mSpoiltRegs);
	read(is, rSelf.m_stackOut);
	read(is, rSelf.mSpoiltFlags);
	read(is, rSelf.m_nFPUOut);

	//if (!(rSelf.flags() & TYP_FDEF_TEMP))
	//	rSelf.setFuncField(fieldFromIdx(is));
}

///////////////////////////////////////////////////////TypeProxy_t

void GlobalSerializerEx_t::saveProxy(std::ostream &os, CTypePtr iSelf)
{
	const TypeProxy_t &rSelf(*iSelf->typeProxy());
	assert(((Type_t &)rSelf).ObjType() == OBJID_TYPE_PROXY);
	writeIdx(os, nameToIdx(rSelf.name()));
	writeIdx(os, typeToIdx(rSelf.incumbent()));
}

void GlobalSerializerEx_t::loadProxy(std::istream &is, TypePtr iSelf)
{
	TypeProxy_t &rSelf(*iSelf->typeProxy());
	assert(((Type_t &)rSelf).ObjType() == OBJID_TYPE_PROXY);
	rSelf.setNameRef(nameFromIdx(readIdx(is)));
	TypePtr pIncumbent(typeFromIdx(readIdx(is)));
	rSelf.setIncumbent0(pIncumbent);
	assert(pIncumbent);
	pIncumbent->addRef();
//CHECKID(pIncumbent, 0x3cef)
//STOP
}

///////////////////////////////////////////////////////TypeObj_t

bool GlobalSerializerEx_t::writeType(ObjId_t objId, std::ostream &os, CTypePtr pType)
{
	assert(objId == pType->ObjType());
	switch (pType->ObjType())
	{
	/*case OBJID_TYPE_RTTI_TD:
	case OBJID_TYPE_RTTI_BCD:
	case OBJID_TYPE_RTTI_CHD:
	case OBJID_TYPE_RTTI_BCA:
		saveStruc(os, pType);
		break;*/
	case OBJID_TYPE_CLASS:
	case OBJID_TYPE_NAMESPACE:
		saveStruc(os, pType);
		break;
	//case OBJID_TYPE_DC:
	//	saveDc(os, pType);
	//	break;
	case OBJID_TYPE_FUNCDEF:
		saveFuncDef(os, pType);
		break;
	case OBJID_TYPE_PROXY:
		saveProxy(os, pType);
		break;
/*	case OBJID_TYPE_VFTABLE:
	case OBJID_TYPE_VBTABLE:
	case OBJID_TYPE_LVFTABLE:
	case OBJID_TYPE_CVFTABLE:
		saveTypeVFTable(os, pType);
		break;*/
	default:
		return Base_t::writeType(objId, os, pType);
	}
	return true;
}

Type_t *GlobalSerializerEx_t::loadType(ObjId_t objId, std::istream &is, TypePtr pTypeObj)
{
	Type_t *pType(nullptr);
	switch (objId)
	{
	/*case OBJID_TYPE_RTTI_TD:
		pType = new TypeRTTI_TD_t;
		pTypeObj->SetPvt(pType);
		loadStruc(is, pTypeObj);
		break;
	case OBJID_TYPE_RTTI_BCD:
		pType = new TypeRTTI_BCD_t;
		pTypeObj->SetPvt(pType);
		loadStruc(is, pTypeObj);
		break;
	case OBJID_TYPE_RTTI_CHD:
		pType = new TypeRTTI_CHD_t;
		pTypeObj->SetPvt(pType);
		loadStruc(is, pTypeObj);
		break;
	case OBJID_TYPE_RTTI_BCA:
		pType = new TypeRTTI_BCA_t;
		pTypeObj->SetPvt(pType);
		loadStruc(is, pTypeObj);
		break;*/
	case OBJID_TYPE_CLASS:
		pType = new TypeClass_t;
		pTypeObj->SetPvt(pType);
		loadStruc(is, pTypeObj);//as a struc
		break;
	case OBJID_TYPE_NAMESPACE:
		pType = new TypeNamespace_t;
		pTypeObj->SetPvt(pType);
		loadStruc(is, pTypeObj);
		break;
	/*?case OBJID_TYPE_MODULE:
		//pType = new ProjectEx_t(mrMemMgr);
		pType = new Module_t();
		pTypeObj->SetPvt(pType);
		loadModule(is, pTypeObj);
		break;*/
	case OBJID_TYPE_FUNCDEF:
		pType = new FuncDef_t();
		pTypeObj->SetPvt(pType);
		loadFuncDef(is, pTypeObj);
		break;
	case OBJID_TYPE_PROXY:
		pType = new TypeProxy_t();
		pTypeObj->SetPvt(pType);
		loadProxy(is, pTypeObj);
//?		assert(pTypeObj->m_nFlags & TYPEOBJ_ID_OVERIDE);
//?		pTypeObj->replaceId(-mrProject.objOverrideNext());
		return pType;
	/*case OBJID_TYPE_VFTABLE:
		pType = new TypeVFTable_t;
		pTypeObj->SetPvt(pType);
		loadTypeVFTable(is, pTypeObj);
		break;
	case OBJID_TYPE_VBTABLE:
		pType = new TypeVBTable_t;
		pTypeObj->SetPvt(pType);
		loadTypeVFTable(is, pTypeObj);//reused
		break;
	case OBJID_TYPE_LVFTABLE:
		pType = new TypeLVFTable_t;
		pTypeObj->SetPvt(pType);
		loadTypeVFTable(is, pTypeObj);//reused
		break;
	case OBJID_TYPE_CVFTABLE:
		pType = new TypeCVFTable_t;
		pTypeObj->SetPvt(pType);
		loadTypeVFTable(is, pTypeObj);//reused
		break;*/
	default:
		pType = Base_t::loadType(objId, is, pTypeObj);
	}
/*	if (pTypeObj->m_nFlags & TYPEOBJ_ID_OVERIDE)
	{
		assert(pTypeObj->typeComplex() || pTypeObj->typeProxy());
		pTypeObj->replaceId(-mrProject.objOverrideNext());
	}*/
	return pType;
}

//int saved = 0;
//int loaded = 0;

//////////////////////////////////////////////////// GlobObj_t

void GlobalSerializerEx_t::saveGlob(std::ostream &os, CGlobPtr iGlob)
{
	StrucSerializer_t::save(os, iGlob->asObj());
	unsigned char u(iGlob->hasPvt() ? iGlob->ObjType() : OBJID_NULL);
	write(os, u);
	if (u != OBJID_NULL)
		if (!writeType(ObjId_t(u), os, (GlobToTypePtr)iGlob))
			throw (-SR_INVALID_GLOB_OBJECT);
}

void GlobalSerializerEx_t::loadGlob(std::istream &is, GlobPtr iGlob)
{
	StrucSerializer_t::load(is, iGlob->asObj());
	unsigned char u;
	read(is, u);
	if (u != OBJID_NULL)
		if (!loadType(ObjId_t(u), is, (GlobToTypePtr)iGlob))
			throw (-SR_INVALID_GLOB_OBJECT);
}

//////////////////////////////////////////////////// FileDef_t

void GlobalSerializerEx_t::saveFileDef(std::ostream &os, const FileDef_t &rSelf)
{
	//save included file refs
	for (IncludeListCIt i(rSelf.includes().begin()); i != rSelf.includes().end(); i++)
		writeIdx(os, folderToIdx(*i));
	writeEos(os);

	//save types section
	const TypesList &l(rSelf.types());
	for (TypesListCIt i(l.begin()); i != l.end(); i++)
	{
		CTypePtr pType(*i);
		if (pType->typeClass())
		{
			writeIdx(os, typeToIdx(pType) | INDEXTYPE_HIBIT);//mark a class with high bit
			saveTypeClass(os, pType);
		}
		else
			writeIdx(os, typeToIdx(pType));
	}
	writeEos(os);//sequence end

	//save fields section
	int i(0);
	const GlobMap &l2(rSelf.globs());
	for (GlobMapCIt it(l2.begin()); it != l2.end(); ++it, i++)
	{
CHECK(i == 282)
STOP
		const CGlobPtr iGlob(&(*it));
		writeIdx(os, fieldToIdx(DcInfo_s::DockField(iGlob)));
		saveGlob(os, iGlob);
	}
	writeEos(os);//sequence end
}

void GlobalSerializerEx_t::loadFileDef(std::istream &is, FileDef_t &rSelf)
{
	//load includes
	Folder_t *pFolder2;
	while ((pFolder2 = folderFromIdx(readIdx(is))) != nullptr)
		rSelf.addIncludeList(pFolder2);

	//load types
	//TypesList &l(pFileDef->types());
	
	INDEXTYPE idx;
	while (idx = readIdx(is))
	{
		TypePtr pType;
		if (idx & INDEXTYPE_HIBIT)
		{
			pType = typeFromIdx(idx & ~INDEXTYPE_HIBIT);
			loadTypeClass(is, pType);
		}
		else
			pType = typeFromIdx(idx);
		rSelf.addTypeObj(pType);
		//l.push_back(pTypeRef);
		//recover imported/exported maps in project;
/*		if (!pTypeRef->typeProxy() && pTypeRef->isExpo rting())
		{
			assert(!(pTypeRef->isUgly()));
			//?					dcInfo.AddExportedTypeInfo(pTypeRef, MyString(pTypeRef->name()->c_str()));//?
		}*/
	}

	//load fields
	//GlobMap &m(pFileDef->fields());
	for (int i(0);; i++)
	{
CHECK(i == 282)
STOP
		FieldPtr pField(fieldFromIdx(readIdx(is)));
		if (!pField)
			break;
		/*if (ud.parentField()->userDataType() == FUDT_FUNC)
		{
			ud.setFunc(typeFromIdx(readIdx(is)));
			assert(ud.func());
		}*/
		FieldExPtr pFieldx(reinterpret_cast<FieldExPtr>(pField));
		GlobPtr iGlob(pFieldx->globPtr());
		loadGlob(is, iGlob);
		rSelf.insertGlob(pFieldx);
	}

	rSelf.AttachMemMgr();
}

////////////////////////////////////////////////////FileTempl_t

void GlobalSerializerEx_t::saveFileTempl(std::ostream &os, const FileTempl_t &rSelf)
{
	const PrettyObjMap& m(rSelf.map());

#if(1)//optional
	//re-arange elements, sorted by object's index (?) to enforce order consistency
	typedef std::pair<INDEXTYPE, INDEXTYPE> t;
	std::vector<t> v;
	v.reserve(m.size());
	struct less_than_key
	{
		inline bool operator() (const t& a, const t& b)
		{
			return a.first < b.first;
		}
	};
	size_t i(0);
	for (PrettyObjMap::const_iterator it(m.begin()); it != m.end(); ++it, i++)
		v.push_back(std::make_pair(objToIdx(it->first), nameToIdx(it->second)));
	std::sort(v.begin(), v.end(), less_than_key());
	assert(v.size() == i);
	for (i = 0; i < v.size(); i++)
	{
		writeIdx(os, v[i].first);
		writeIdx(os, v[i].second);
	}
#else
	for (PrettyObjMap::const_iterator it(m.begin()); it != m.end(); ++it)
	{
		writeIdx(os, objToIdx(it->first));
		writeIdx(os, nameToIdx(it->second));
	}
#endif
	writeEos(os);
	writeEos(os);
}

void GlobalSerializerEx_t::loadFileTempl(std::istream &is, FileTempl_t &rSelf)
{
	for (;;)
	{
		TypeBasePtr pObj(objFromIdx(readIdx(is)));
		PNameRef pn(nameFromIdx(readIdx(is)));
		if (!pn)
		{
			assert(!pObj);
			break;
		}
		rSelf.addPrettyName(pObj, pn);
	}
}

bool GlobalSerializerEx_t::saveFile(std::ostream &os, const Folder_t &rFolder, FILEID_t u)
{
	switch (u)
	{
	case FILEID_FILE:
		saveFileDef(os, *rFolder.fileDef());
		break;
	case FILEID_TEMPL:
		saveFileTempl(os, *rFolder.fileTempl());
		break;
	case FILEID_STUBS:
		saveFileStubs(os, *rFolder.fileStubs());
		break;
	default:
		return GlobalSerializer_t::saveFile(os, rFolder, u);
	}
	return true;
}

bool GlobalSerializerEx_t::loadFile(std::istream &is, Folder_t &rFolder, FILEID_t u)
{
	switch (u)
	{
	case FILEID_FILE:
	{
		FileDef_t *pFile = new FileDef_t;
		rFolder.SetPvt(pFile);
		pFile->setFolder(&rFolder);
		loadFileDef(is, *pFile);
#ifdef _DEBUG
		pFile->memMgr().mName = rFolder.name();
#endif
		break;
	}
	case FILEID_TEMPL:
	{
		FileTempl_t *pFile(new FileTempl_t);
		rFolder.SetPvt(pFile);
		//pFile->setFolder(&rFolder);
		loadFileTempl(is, *pFile);
		break;
	}
	case FILEID_STUBS:
	{
		FileStubs_t *pFile(new FileStubs_t);
		rFolder.SetPvt(pFile);
		loadFileStubs(is, *pFile);
		break;
	}
	default:
		return GlobalSerializer_t::loadFile(is, rFolder, u);
	}
	return true;
}

////////////////////////////////////////////////////////////// Dc_t

void GlobalSerializerEx_t::saveDC(std::ostream &os, const Dc_t &rSelf)
{
	writeIdx(os, typeToIdx(rSelf.module()));

	//write(os, rSelf.frontName());

	writeIdx(os, typeToIdx(rSelf.primeSeg()));

	//write intrinsics
	unsigned intrNum((unsigned)rSelf.mIntrinsics.size());
	write(os, intrNum);
	for (unsigned i(0); i < intrNum; i++)
	{
		CGlobPtr pGlob(rSelf.mIntrinsics[i]);
		write(os, globIdx(pGlob));
		saveTypeObj(os, (GlobToTypePtr)pGlob);
	}

	//saveStubs(os, rSelf.stubs());
	//writeIdx(os, folderToIdx(rSelf.stubsFolderPtr()));

//?	rSelf.muDirty = DIRTY_NULL;
}

Dc_t *GlobalSerializerEx_t::loadDC(std::istream &is)
{
	TypePtr pModule(typeFromIdx(readIdx(is)));
	if (!pModule)
		return nullptr;//no more

	TypePtr iFrontSeg(typeFromIdx(readIdx(is)));

	ProjectEx_t &projx(GetProjectEx());

	Dc_t *pDC(new Dc_t(projx, iFrontSeg, pModule));
	Dc_t &rSelf(*pDC);

	//intrinsics
	unsigned intrNum;
	read(is, intrNum);
	if (intrNum > 0)
	{
		rSelf.mIntrinsics.resize(intrNum);

		for (unsigned i(0); i < intrNum; i++)
		{
			FieldPtr pField(fieldFromIdx(readIdx(is)));
			FieldExPtr pFieldx(reinterpret_cast<FieldExPtr>(pField));
			GlobPtr pGlob(pFieldx->globPtr());
			loadTypeObj(is, (GlobToTypePtr)pGlob);
			if (!pGlob)
				break;
			rSelf.mIntrinsics[i] = pGlob;
			//pGlob->addRef();
		}
	}

//	loadStubs(is, rSelf.stubs());

//	FolderPtr pStubsFolder(rSelf.stubsFolderPtr());//should be safe - this comes from module's folder which should be loaded
	//FolderPtr pStubsFolder(folderFromIdx(readIdx(is)));
	//assert(!pStubsFolder || rSelf.hasStubs());
	//if (pStubsFolder)
		//pStubsFolder->fileStubs()->setStubsPtr(&rSelf.stubs());

//	if (mpICtx)
	//	mpICtx->pDcRef = &rSelf;

	return pDC;
}

//////////////////////////////////////////////////// Struc_t (overriden)

void GlobalSerializerEx_t::saveTypeClass(std::ostream &os, CTypePtr iSelf)
{
	/*if (!bPost)
	{
		GlobalSerializer_t::saveStruc(os, iSelf);
		return;
	}*/

	TypeClass_t &rSelf(*iSelf->typeClass());
	
	const ClassMemberList_t &l(rSelf.methods());
	for (ClassMemberListCIt i(l.begin()); i != l.end(); ++i)
	{
		CFieldPtr pField(DcInfo_s::DockField(*i));//funcs or static fields
		writeIdx(os, fieldToIdx(pField));
	}
	const ADDR eos(0);
	write(os, eos);//end of non-virtual methods

	const ClassVTables_t& vtables(rSelf.vtables());
	for (ClassVTables_t::const_iterator i(vtables.begin()); i != vtables.end(); ++i)
	{
		const ClassVTable_t& vtable(i->second);
		INDEXTYPE idx(vtable.self ? fieldToIdx(DcInfo_s::DockField(vtable.self)) : (INDEXTYPE)-1);//a trick for unknown v-table
		writeIdx(os, idx);
		int off(i->first);//vptr offset in class
		write(os, off);
		for (ClassVirtMembers_t::const_iterator j(vtable.entries.begin()); j != vtable.entries.end(); ++j)
		{
			CGlobPtr pGlob(j->second);
			writeIdx(os, fieldToIdx(DcInfo_s::DockField(pGlob)));
			off = j->first;//offset in v-table
			write(os, off);
		}
		write(os, eos);//end of v-methods
	}
	write(os, eos);//end of vtables
}


void GlobalSerializerEx_t::loadTypeClass(std::istream &is, TypePtr iSelf)
{
	if (!iSelf)
		throw (-SR_INVALID_TYPE_REF);

	/*if (!bPost)
	{
		GlobalSerializer_t::loadStruc(is, iSelf);
		return;
	}*/

	assert(!iSelf->hasUserData());

	TypeClass_t &rSelf(*iSelf->typeClass());

	//MemoryMgrEx_t &memMgr(reinterpret_cast<MemoryMgrEx_t &>(mrMemMgr));
	
	//non-virtual methods list
	ClassMemberList_t &l(rSelf.methods());
	FieldPtr pField;
	while ((pField = fieldFromIdx(readIdx(is))) != nullptr)
	{
		FieldExPtr pFieldx(reinterpret_cast<FieldExPtr>(pField));
		rSelf.addMember(pFieldx->globPtr());
	}

	//v-tables with v-methods
	for (;;)
	{
		INDEXTYPE idx(readIdx(is));
		if (idx == 0)
			break;
		FieldPtr pField(idx == -1 ? nullptr : fieldFromIdx(idx));
		//FieldExPtr pFieldx(reinterpret_cast<FieldExPtr>(pField));
		GlobPtr pGlob(pField ? reinterpret_cast<FieldExPtr>(pField)->globPtr() : nullptr);
		int off;
		read(is, off);//vptr offset in class for this v-table
		ClassVTable_t &vtable(rSelf.addVTable(pGlob, off));
		while ((pField = fieldFromIdx(readIdx(is))) != nullptr)
		{
			//FieldExPtr pFieldx(reinterpret_cast<FieldExPtr>(pField));
			pGlob = pField ? reinterpret_cast<FieldExPtr>(pField)->globPtr() : nullptr;
			read(is, off);
			vtable.entries.insert(std::make_pair(off, pGlob));
		}
	}
}


////////////////////////////////////////////////////

/*void GlobalSerializerEx_t::saveTypeVFTable(std::ostream &os, CTypePtr iSelf)
{
	GlobalSerializer_t::saveStruc(os, iSelf);

	TypeVFTable_t &rSelf(*iSelf->typeVFTable());
//	writeIdx(os, typeToIdx(rSelf.ownerClass()));
	writeIdx(os, typeToIdx(rSelf.baseType()));
}

void GlobalSerializerEx_t::loadTypeVFTable(std::istream & is, TypePtr iSelf)
{
	GlobalSerializer_t::loadStruc(is, iSelf);

	TypeVFTable_t &rSelf(*iSelf->typeVFTable());
//	rSelf.setOwnerClass(typeFromIdx(readIdx(is)));
	rSelf.setBaseType(typeFromIdx(readIdx(is)));
	if (rSelf.baseType())
		rSelf.baseType()->addRef();
}*/



//////////////////////////////////////////////////// StubMgr_t

void GlobalSerializerEx_t::saveFileStubs(std::ostream &os, const FileStubs_t &rSelf)
{
	const StubMgr_t &m(rSelf.stubs());
	for (StubCIt it(m.begin()); it != m.end(); it++)
	{
		const Stub_t &aStub(it->second);
		write(os, aStub.atAddr());
//CHECK(aStub.atAddr() == 0x1001094)
//STOP
		uint8_t flags(aStub.refMode() & REFMODE__MASK);
		//if (aStub.field())
			//flags |= 0x10;
		if (!aStub.value().empty())
			flags |= 0x20;
		write(os, flags);
		//if (flags & 0x10)
			//writeIdx(os, fieldToIdx(aStub.field()));
		if (flags & 0x20)
			write(os, aStub.value());
	}
	ADDR eos(0);
	write(os, eos);
}

void GlobalSerializerEx_t::loadFileStubs(std::istream &is, FileStubs_t &rSelf)
{
	StubMgr_t &m(rSelf.stubs());
	for (;;)
	{
		ADDR atAddr;
		read(is, atAddr);
		if (!atAddr)//?
			break;
//CHECK(atAddr == 0x1001094)
//STOP
		uint8_t flags;
		read(is, flags);
		
		//FieldPtr pField(nullptr);
		//if (flags & 0x10)
			//pField = fieldFromIdx(is);
		MyString sValue;
		if (flags & 0x20)
			read(is, sValue);
		Stub_t &rStub(StubInfo_t::InsertStub0(m, atAddr, REFMODE_t(flags & REFMODE__MASK), sValue));
		//if (pField)
			//rStub.setField2(pField);
	}
}

ProjectEx_t &GlobalSerializerEx_t::GetProjectEx() const
{
	return reinterpret_cast<ProjectEx_t &>(GetProject());
}

/////////////////////////////////////////////////////////////////

/*class DcFolderIterator : public FoldersMap::const_iterator//with DCs only
{
	typedef FoldersMap::const_iterator	Base;
public:
	DcFolderIterator(FilesMgr0_t &rFilesMgr)
		: Base(rFilesMgr.rootFolder()->fileFolder()->children())
	{
		while (operator bool() && !check())
			operator++();
	}
	Base& operator ++() {
		do {
			Base::operator++();
		} while (operator bool() && !check());
		return *this;
	}
	Base& operator ++(int) { return operator++(); }
	Folder_t &folder(){ return *pvt(); }
	const Folder_t &folder() const { return *pvt(); }
	TypePtr binaryPtr() const { return folder().binary(); }
	const Module_t &module() const { return *modulePtr()->typeModule(); }
	Dc_t &dc(){ return *dcPtr(); }
	Dc_t *dcPtr() const { return DCREF(binaryPtr()); }
private:
	bool check() const
	{
		const Folder_t &rFolder(folder());
		if (!rFolder.miBinary)
			return false;
		if (!dcPtr())
			return false;
		return true;
	}
};*/

#if(FUNC_SAVE_ENABLED)
void GlobalSerializerEx_t::saveDeferredTypeRefs(std::ostream &os, const std::vector<CTypePtr> &v)
{
	//write remapping of types
	unsigned uvSize((unsigned)v.size());
	write(os, uvSize - 1);//1-biased
	for (size_t i(1); i < uvSize; i++)
		writeIdx(os, typeToIdx(v[i]));
	//writeEos(os);
}

void GlobalSerializerEx_t::loadDeferredTypeRefs(std::istream &is, std::vector<CTypePtr> &v)
{
	unsigned uvSize;
	read(is, uvSize);
	v.reserve(uvSize + 1);
	v.resize(1);//1-biased
	for (unsigned i(0); i < uvSize; i++)
	{
		TypePtr p(typeFromIdx(readIdx(is)));
		v.push_back(p);
		p->addRef();
	}
}

//////////////////////////////////////////////////

static MyPath makeDeferredPath(MyString primaryPath, const Folder_t &rFolder, std::set<std::string> &used)
{
	MyPath f(primaryPath);
	MyString sx(rFolder.fileDef()->dispersedName());
	if (sx.isEmpty())
	{
		MyString s(f.Basename());
		s.append(".");
		s.append(rFolder.name());
		MyString sfx;
		int iSfx(0);
		while (used.find(s + sfx) != used.end())//search for empty slot
			sfx = MyStringf(".%d", ++iSfx);
		s.append(sfx);
		f.SetBasename(s);
		f.SetExt(SECONDARY_EXT);

		assert(f.Basename() == s);
		if (!used.insert(s).second)
			ASSERT0;//empty slot expected
	}
	else
	{
		assert(used.find(sx) != used.end());
		f.SetName(sx);
	}
	return f;
}

class secondary_ofstream : public std::ofstream
{
	MyFile mFile;
public:
	secondary_ofstream(const MyPath &r)
		: mFile(r)
	{
	}
	bool assure_open()
	{
		if (!is_open())
		{
			if (!mFile.EnsureDirExists())
				return false;
			open(mFile.Path(), std::ios::binary);
			if (!is_open())
				return false;
		}
		return true;
	}
};

bool GlobalSerializerEx_t::saveDispersed(std::ostream &os, CFolderRef rFolder, const Dc_t &rDC, std::set<std::string> &used)
{
	//write(os, sPath);
	FileDef_t &rFileDef(*rFolder.fileDef());

	MemmoryAccessorExx_t<FileSerializerEx_t, GlobalSerializerEx_t> FS(SR_Saving, reinterpret_cast<MemoryMgrEx_t &>(rFileDef.memMgr()), reinterpret_cast<MemmoryAccessor_t<GlobalSerializerEx_t>&>(*this));
	//FileSerializerEx_t FS(memAcc, *this);
	MyString sPath(rDC.project().files().relPath(&rFolder));

	if (FS.isEmpty())//if modified!
		return false;

	MyPath fpath(makeDeferredPath(mPath, rFolder, used));

	secondary_ofstream ofs2(fpath);
	if (!ofs2.assure_open())
	{
		fprintf(STDERR, "Warning: Could not open secondary file: %s\n", fpath.Path().c_str());
		return false;
	}

	writeIdx(os, folderToIdx((FolderPtr)&rFolder));
	write(os, fpath.Name());//external (secondary) file's name
	rFileDef.setDispersedName(fpath.Name());

	write(ofs2, sPath);//for user's convinience print an actual path at the begining of a secondary file

	//!	FS.savef(ofs2, rFileDef);

	//unique_vector<TypePtr> uv;

	//save arrays
	FS.save(ofs2);

	//save reloadable funcdefs
	const GlobMap &l(rFileDef.globs());
	for (GlobMapCIt i(l.begin()); i != l.end(); i++)
	{
		CFieldPtr pField(DcInfo_s::DockField(&(*i)));
		CGlobPtr ifDef(DcInfo_s::GlobFuncObj(pField));
		if (ifDef && !ifDef->typeFuncDef()->isStub())
		{
			//FS.addDifferedRef(ifDef);//register a connection node
			//writeIdx(os, typeToIdx(ifDef));//a marker
//?			writeIdx(ofs2, FS.writeFieldType(ifDef));
			writeIdx(ofs2, FS.fieldRefToIdx(pField));//!
			FS.saveFunction(ofs2, ifDef);
		}
	}
	writeEos(ofs2);

	saveDeferredTypeRefs(os, FS.deferredTypes().base());
	rFileDef.setDeferredTypes(FS.deferredTypes().base());

	return true;
}

void GlobalSerializerEx_t::loadDispersed(std::istream &ifs2, Folder_t &rFolder, Dc_t &rDC)
{
	FileDef_t &rFileDef(*rFolder.fileDef());

	MyString s;
	read(ifs2, s);

	MyString s0(rDC.project().files().relPath(&rFolder));
	if (s != s0)
	{
		//throw (SR_FILES_MISMATCH);
		MAIN.printError() << "Deferred file signature mismatch: " << s << ", expected: " << s0 << std::endl;
		return;
	}

	MemoryMgrEx_t &memMgr(rFileDef.ownsMemory() ? rFileDef.memMgrEx() : reinterpret_cast<MemoryMgrEx_t&>(GetProject().memMgr()));//?
	MemmoryAccessorExx_t<FileSerializerEx_t, GlobalSerializerEx_t> FS(SR_Loading, reinterpret_cast<MemoryMgrEx_t &>(memMgr), reinterpret_cast<MemmoryAccessor_t<GlobalSerializerEx_t>&>(*this));
	//FileSerializerEx_t FS(memAcc, *this);

	FS.setDeferredTypes(rFileDef.deferredTypes());

	FS.load(ifs2);

	//a marker
	//while ((ifDef = typeFromIdx(readIdx(ifs2))) != nullptr)
	FieldPtr pField;
//?	while ((ifDef = FS.fieldTypeFromIdx(readIdx(ifs2)) != nullptr)
	while ((pField = FS.fieldRefFromIdx(readIdx(ifs2))) != nullptr)//!
	{
		GlobPtr ifDef(DcInfo_s::GlobObj(pField));
//CHECKID(ifDef, 0x10ee)
//STOP
		FuncDef_t &rfDef(*ifDef->typeFuncDef());
		FuncInfo_t FI(rDC, *ifDef);// , rFileDef);//DO NOT REMOVE! provides func context!
		TypePtr iLocals(FS.loadFunction(ifs2, rfDef));
		std::vector<TypePtr> v;
		v.push_back((GlobToTypePtr)ifDef);
		GlobalRecovererEx_t aRec(GetProject());
		aRec.recoverFunc(v, ifDef, iLocals, DcInfo_s::DockAddr(ifDef));
		v.pop_back();
	}
}

#endif//FUNC_SAVE_ENABLED

//////////////////////////////////////////////////////////

void GlobalSerializerEx_t::saveSolid(std::ostream &os, const Folder_t &rFolder, const Dc_t &)
{
	FileDef_t *pFileDef(rFolder.fileDef());
#if(FUNC_SAVE_ENABLED)
	MemmoryAccessorEx_t<FileSerializer_t, GlobalSerializerEx_t> FS(SR_Saving, pFileDef->memMgrEx(), reinterpret_cast<MemmoryAccessor_t<GlobalSerializerEx_t>&>(*this));
//	FileSerializer_t FS;// (memAcc, *this);

	writeIdx(os, folderToIdx(&rFolder));
	write(os, MyString(""));//empty secondary file's name - means a file shares storage with a primary one

	//!	FS.savef(os, *pFileDef);

	//save arrays
	FS.save(os);

	//save reloadable funcdefs
	const GlobMap &l(pFileDef->globs());
	for (GlobMapCIt i(l.begin()); i != l.end(); i++)
	{
		CFieldPtr pField(DcInfo_s::DockField(&(*i)));
		CGlobPtr ifDef(DcInfo_s::GlobFuncObj(pField));
		if (ifDef && !ifDef->typeFuncDef()->isStub())
		{
			writeIdx(os, fieldToIdx(pField));//a marker
			FS.saveFunction(os, ifDef);
		}
	}
	writeEos(os);
#endif
}

void GlobalSerializerEx_t::loadSolid(std::istream &is, Folder_t &rFolder, Dc_t &rDC)
{
	FileDef_t &rFileDef(*rFolder.fileDef());
#if(FUNC_SAVE_ENABLED)
	MemoryMgrEx_t &mm(rFileDef.ownsMemory() ? rFileDef.memMgrEx() : reinterpret_cast<MemoryMgrEx_t &>(GetProject().memMgr()));//?
	MemmoryAccessorEx_t<FileSerializer_t, GlobalSerializerEx_t> FS(SR_Loading, mm, reinterpret_cast<MemmoryAccessor_t<GlobalSerializerEx_t>&>(*this)); 
//	FileSerializer_t FS;// (memAcc, *this);

	FS.load(is);
	//load reloadable funcdefs

	FieldPtr pDockField;//a marker
	while ((pDockField = fieldFromIdx(readIdx(is))) != nullptr)
	{
		GlobPtr ifDef(DcInfo_s::GlobObj(pDockField));
//CHECKID(ifDef, 0x10ee)
//STOP
		FuncDef_t &rfDef(*ifDef->typeFuncDef());
		FuncInfo_t FI(rDC, *ifDef);// , rFileDef);//provides func context!
		TypePtr iLocals(FS.loadFunction(is, rfDef));
		std::vector<TypePtr> v;
		v.push_back((GlobToTypePtr)ifDef);
		GlobalRecovererEx_t aRec(GetProject());
		aRec.recoverFunc(v, ifDef, iLocals, DcInfo_s::DockAddr(ifDef));
		v.pop_back();
	}
#endif
}

////////////////////////////////////////////////////////////////////////////

static void scanUsedNames(const FoldersMap &m, std::set<std::string> &used)
{
	// gather all dispersed names (new files may have been added which haven't been saved yet)
	for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
	{
		CFolderRef pFolder0(*i);
		if (pFolder0.fileModule())
		{
			for (FilesMgr0_t::FolderIterator j(&pFolder0); j; j++)
			{
				CFolderRef pFolder(*j);
				FileDef_t *pFileDef(pFolder.fileDef());
				if (pFileDef && pFileDef->ownsMemory())
				{
					MyString s(pFileDef->dispersedName());
					if (!s.isEmpty())
						if (!used.insert(s).second)
							ASSERT0;
				}
			}
		}
	}
}

void GlobalSerializerEx_t::SaveEx(std::ostream &os, SaveMode mode)
{
	Base_t::Save(os);

	const ProjectEx_t &rProj(GetProjectEx());

	writeIdx(os, typeToIdx(rProj.unresolvedExternalsModule()));
	
	const FoldersMap &m(mrProject.rootFolder().children());

	std::set<std::string> used;//to keep track of already used file names (files may have identical names in different folders)
	MyString oldPath(mrProject.path());
	if (!oldPath.isEmpty())//was project saved before?
	{
		scanUsedNames(m, used);
		if (mode == SaveMode_None)
			mode = used.empty() ? SaveMode_Solid : SaveMode_Dispersed;
	}

	for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
	{
		CFolderRef pFolder0(*i);
		if (!pFolder0.fileModule())
			continue;
		Dc_t *pDC(DCREF(pFolder0.fileModule()->module()));
		if (!pDC)//go over modules with dcs only
			continue;

		Dc_t &rDC(*pDC);
		saveDC(os, rDC);

		for (FilesMgr0_t::FolderIterator j(&pFolder0); j; j++)
		{
			CFolderRef pFolder(*j);
			FileDef_t *pFileDef(pFolder.fileDef());
			if (!pFileDef || !pFileDef->ownsMemory())
				continue;

			if (mode == SaveMode_Dispersed)
			{
#if(FUNC_SAVE_ENABLED)
				if (saveDispersed(os, pFolder, rDC, used))
				{
					FileInfo_t FI(rDC, *pFileDef);
					FI.UnloadFuncdefs();
					continue;
				}
#endif
				//save into primary
			}

			pFileDef->setDispersedName(MyString());//clear
			saveSolid(os, pFolder, rDC);

		}
		writeEos(os);//no more files
	}

	writeEos(os);//no more dcs

	if (mode == SaveMode_Solid && !used.empty() /*&& !oldPath.isEmpty()*/ && mPath == oldPath)
	{
		//transition from dispersed to solid mode - all secondary files must be deleted (if path has not changed)
		while (!used.empty())
		{
			MyPath f(*used.begin(), MyPath(mPath));
			MyFile ff(f);
			if (ff.Unlink() != 0)
				fprintf(STDERR, "Warning: Could not unlink secondary file: %s\n", f.Path().c_str());
			used.erase(used.begin());
		}
	}
}

void GlobalSerializerEx_t::LoadEx(std::istream &is, bool diferred)
{
	GlobalSerializer_t::Load(is);

	ProjectEx_t &rProj(GetProjectEx());

	rProj.setUnresolvedExternalsModule(typeFromIdx(readIdx(is)));

	GlobalRecovererEx_t aRec(rProj);
	aRec.recover();

	FilesMgr0_t &rFilesMgr(rProj.files());
	Dc_t *pDC;
	while (pDC = loadDC(is))
	{
		aRec.recoverDC(*pDC);
		FolderPtr pFolder;
		while (pFolder = folderFromIdx(readIdx(is)))
		{
			MyString sFile;
			read(is, sFile);
			if (sFile.isEmpty())
				loadSolid(is, *pFolder, *pDC);
			else
			{
#if(FUNC_SAVE_ENABLED)
				FileDef_t &rFileDef(*pFolder->fileDef());
				loadDeferredTypeRefs(is, rFileDef.deferredTypes());//ref counters are incremented here
				rFileDef.setDispersedName(sFile);

				if (!diferred)//postpone loading secondary files until a user wants it
				{
					LoadDispersed(sFile, *pFolder, *pDC);
				}
#endif
			}
		}
	}

//	aRec.recoverCxxSymbols();
}

/////////////////////////////////////////////////////////////////// dispersed

#if(FUNC_SAVE_ENABLED)
bool GlobalSerializerEx_t::LoadDispersed(MyString sFile, Folder_t &rFolder, Dc_t &rDC)
{
	MyPath fPath(sFile, MyPath(mPath));
	std::ifstream ifs2(fPath.Path(), std::ios::binary);
	if (!ifs2.is_open())
	{
		MAIN.printError() << "Could not open deferred file: " << sFile << std::endl;
		return false;
	}
	loadDispersed(ifs2, rFolder, rDC);
	FileDef_t &rFileDef(*rFolder.fileDef());
	rFileDef.releaseDeferredTypes();
	return true;
}
#endif

//////////////////////////////////////////////////////////// recover

void GlobalRecovererEx_t::recoverArgs(OpList_t &l, HOP pOp0)
{
	OpPtr pOpPr = OpPtr();
	for (OpList_t::Iterator j(l); j; j++)
	{
		OpPtr pOp(j.data());
		//assert(!pOp->IsPrimeOp());
		pOp->setInsPtr(pOp0->insPtr());
		pOp->pPrev = pOpPr;
		pOpPr = pOp;
	}
	if (pOpPr)//it's a circular list
	{
		OpList_t::Iterator j(l);
		OpPtr pOp(j.data());
		pOp->pPrev = pOpPr;
	}
}

void GlobalRecovererEx_t::recoverCallRets(OpList_t &l)
{
	OpPtr pOpPr = OpPtr();
	for (OpList_t::Iterator j(l); j; j++)
	{
		OpPtr pOp(j.data());
		//assert(!pOp->IsPrimeOp());
		assert(pOp->m_xin.check_count(1) == 0);
		OpPtr pOp0(pOp->XIn()->data());
		assert(pOp0->ins().mpPrimeOp == pOp0);
		pOp->setInsPtr(pOp0->insPtr());
		pOp->pPrev = pOpPr;
		pOpPr = pOp;
	}
	if (pOpPr)//it's a circular list
	{
		OpList_t::Iterator j(l);
		OpPtr pOp(j.data());
		pOp->pPrev = pOpPr;
	}
}

void ProjectEx_t::OnRecoverStruc(std::vector<TypePtr> &v)
{
	TypePtr pClass(v.back());
	TypeClass_t *pClassPvt(pClass->typeClass());
	if (!pClassPvt)
		return;

	//NamesMgr_t *pNS(pClassPvt->namesMgr());

	ClassMemberList_t &l(pClassPvt->methods());
	for (ClassMemberListCIt i(l.begin()); i != l.end(); ++i)
	{
		GlobPtr pGlob(*i);
//CHECKID(pField, 0x975)
//STOP
		//setup a scope
		pGlob->setOwner(pClass);//scope
	}

	const ClassVTables_t& vtables(pClassPvt->vtables());
	for (ClassVTables_t::const_iterator i(vtables.begin()); i != vtables.end(); ++i)
	{
		const ClassVTable_t& vtable(i->second);
		if (vtable.self)
			vtable.self->setOwner(pClass);

		for (ClassVirtMembers_t::const_iterator j(vtable.entries.begin()); j != vtable.entries.end(); ++j)
		{
			GlobPtr pGlob(j->second);
			pGlob->setOwner(pClass);
		}
	}
}

void ProjectEx_t::OnRecoverField(FieldPtr pField, TypePtr pModule, TypePtr pScope0)
{
//CHECKID(pField, 0x42ab)
//STOP
	GlobPtr pGlob(DcInfo_t::GlobObjNA(pField));
	if (pGlob)
	{
		TypePtr pScope(DcInfo_t::OwnerScope(pGlob));
		if (pScope)//global class members
		{
			if (pField->name() && !pField->hasUglyName())
			{
				assert(pField->name()->obj() == pField);
				NamesMgr_t* pNS(pScope->typeComplex()->namesMgr());
				if (!pNS->insertName(pField->name()))
					ASSERT0;
				return;
			}
		}
	}
	if (pField->isExported())
	{
		exportPool().add(pField);
	}

	/*if (pField->hasUserData())
	{
		if (pField->userDataType() > FUDT_ LOCAL)
		{
			GlobPtr iGlob(DcInfo_t::GlobObj(pField));
			if (iGlob->func())
			{
				assert(!iGlob->typeFuncDef() || iGlob->nameless());//no longer!
				if (iGlob->typeFuncDef() && !iGlob->nameless())
				{
					assert(0);
					NamesMgr_t *pNS(nullptr);
					TypePtr iScope(DcInfo_t::OwnerScope(pField));
					if (iScope)
						pNS = iScope->typeComplex()->namesMgr();
					else
					{
						//TypePtr pModule(v.front());
						assert(pModule->typeModule());
						TypePtr iNSOwner(GetNameOwnerOf(pField));
						pNS = iNSOwner->typeComplex()->namesMgr();
					}
					if (!pNS->insertName(iGlob->name(), iGlob))//, nullptr))
						ASSERT0;
				}
			}
		}
	}*/
	return Project_t::OnRecoverField(pField, pModule, pScope0);
}

void GlobalRecovererEx_t::recoverFunc(std::vector<TypePtr> &v, GlobPtr ifDef, TypePtr iLocals, ADDR dockAddr)
{
	FuncDef_t &rFuncDef(*ifDef->typeFuncDef());
	for (PathTree_t::Iterator i(rFuncDef.pathTree().tree()); i; ++i)
	{
		PathPtr h(*i);
		Path_t &r(*h);
		r.SetParent(i.parent());
		PathPtr p(i.top());
		r.pNext->SetPrev(p);
		Path_t::OpEltPtr pEltPr = Path_t::OpEltPtr();
		for (PathOpList_t::Iterator j(r.ops()); j; ++j)
		{
			Path_t::OpEltPtr pElt(j.data());
			HOP hOp(PRIME(pElt));
			Ins_t& ins(hOp->ins());
//CHECKID(pOp, 0x1e73)
//STOP

#ifdef _DEBUG
			ins.setOff(ins.VA() - dockAddr);// ifDef->dockAddress());
#endif
			//assert(pOp->IsPrimeOp());
			//pOp->setInsPtr(pOp);

			recoverArgs(ins.mArgs, hOp);
			/*for (OpList_t::Iterator k(pOp->argsIt()); k; ++k++)
			{
				OpPtr pOp2(k.data());
				pOp2->setInsPtr(pOp);
			}*/
			ins.mpPath = h;
			pElt->pPrev = pEltPr;
			pEltPr = pElt;
		}
		if (pEltPr)//it's a circular list
		{
			PathOpList_t::Iterator j(r.ops());
			Path_t::OpEltPtr pElt(j.data());
			pElt->pPrev = pEltPr;
		}
		for (XOpList_t::Iterator k(r.inflow()); k; ++k)
			k.data()->setPathRef(h);
	}

	recoverCallRets(rFuncDef.mCallRets);
	/*for (OpList_t::Iterator k(rFuncDef.mCallRets); k; k++)
	{
		OpPtr pOp(k.data());
		assert(pOp->m_xin.check_count(1) == 0);
		OpPtr pOp0(pOp->XIn()->Op());
		assert(pOp0->IsPrimeOp());
		pOp->setInsPtr(pOp0);
	}*/

	if (iLocals)
	{
#if(NEW_LOCAL_VARS)
		iLocals->setOwner((GlobToTypePtr)ifDef);
#endif
		v.push_back(iLocals);
		recoverStruc(v, (GlobToTypePtr)ifDef);//union
		v.pop_back();
	}
}

void GlobalRecovererEx_t::recoverDC(Dc_t &rSelf)
{
	TypePtr pNSOwner;
	NamesMgr_t *pNS(OwnerNamesMgr(rSelf.primeSeg(), &pNSOwner));
	//typedefs
/*	for (list<TypePtr>::const_iterator i(rSelf.mTypedefs.begin()); i != rSelf.mTypedefs.end(); i++)
	{
		TypePtr iType(*i);
		assert(!iType->nameless());
		pNS->insertName(iType->name(), iType);
	}*/

	//intrinsics
	for (size_t i(0); i < rSelf.mIntrinsics.size(); i++)
	{
		GlobPtr iGlob(rSelf.mIntrinsics[i]);
		PNameRef pn(iGlob->name());
		assert(pn);
		pNS->insertName(pn);
		pn->setObj(iGlob);
		assert(pn->obj() == iGlob);
	}

	FolderPtr pFolderTempl(rSelf.folderPtr(FTYP_TEMPLATES));
	if (pFolderTempl)
	{
		const PrettyObjMap &m(pFolderTempl->fileTempl()->map());
		for (PrettyObjMap::const_iterator i(m.begin()); i != m.end(); ++i)
		{
			TypePtr pScope(nullptr);
			TypeBasePtr pObj(i->first);
			GlobPtr pGlob(pObj->objGlob());
			if (pGlob)
			{
				//GlobPtr pGlob(DcInfo_t::GlobObjNA(pField));
				assert(pGlob);//assert?
				pScope = pGlob->ownerScope1();
				if (!pScope)
					pScope = rSelf.primeSeg();
			}
			else
			{
				TypePtr pType(pObj->objType());
				pScope = DcInfo_t::OwnerScope(pType);
				if (!pScope)
					pScope = rSelf.primeSeg();
			}
			assert(pScope);//?
			{
				NamesMgr_t* pNS(pScope->typeComplex()->namesMgr());
				assert(pNS);
				PNameRef pn(i->second);
				if (!pNS->insertName(pn))
					ASSERT0;
				pn->setObj(pObj);
				assert(pn->obj() == pObj);
			}
		}
	}
}

/*void GlobalRecovererEx_t::recoverField(Field_t &rField, NamesMgr_t *pNS)
{
	GlobalRecoverer_t::recoverField(rField, pNS);
	if (rField.isStaticMember())
	{
		STOP
	}
}*/
