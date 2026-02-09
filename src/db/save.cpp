#include "save.h"
#include "prefix.h"
#include <fstream>

#include "shared/defs.h"
#include "shared/link.h"
#include "shared/data_source.h"
//#include "shared/front.h"
#include "mem.h"
#include "obj.h"
#include "type_proc.h"
#include "field.h"
#include "type_seg.h"
#include "type_strucvar.h"
#include "proj.h"
#include "type_code.h"
#include "types_mgr.h"
#include "info_module.h"
#include "names.h"
#include "main.h"
#include "files.h"


#if(SR_DEBUG)
std::streamoff BasicSerializer_t::_CP = -1;
#endif




int WriteStr(std::ostream &os, char *pName)
{
	int len;
	len = (pName)?((int)strlen(pName)):(0);
	assert(len < 0x100);
	uint8_t len2 = len;
	os.write((char *)&len2, sizeof(len2));
	if (len2 > 0)
		os.write(pName, len2);
	return len2;
}

int WriteStrBig(std::ostream &os, char *str)
{
	int len;
	len = (str)?((int)strlen(str)):(0);
	assert(len < 0x10000);
	uint16_t len2 = len;
	os.write((char *)&len2, sizeof(len2));
	if (len2 > 0)
		os.write(str, len2);
	return len2;
}

////////////////////////////////// Obj_t (local)

void StrucSerializer_t::save(std::ostream &os, const Obj_t &rSelf)
{
	write(os, rSelf.m_nFlags);
}

void StrucSerializer_t::load(std::istream &is, Obj_t &rSelf)
{
	read(is, rSelf.m_nFlags);
}

/////////////// Field_t


void StrucSerializer_t::saveField(std::ostream &os, CFieldPtr pSelf)
{
//CHECKID(pSelf, 0x1806)
CHECK(_CP == 29458)
STOP
	save(os, pSelf->asObj());
	writeIdx(os, nameToIdx(pSelf->name()));
	writeIdx(os, fieldTypeToIdx(pSelf->type()));
	//do not save the offset (taken care by parent)
}

void StrucSerializer_t::loadField(std::istream &is, FieldPtr pSelf)
{
//CHECKID(pSelf, 0x2fea)
CHECK(_CP == 0x29458)
STOP
	load(is, pSelf->asObj());
	pSelf->setName0(nameFromIdx(readIdx(is)));
	TypePtr pType(fieldTypeFromIdx(readIdx(is)));
	if (pType)
	{
		pSelf->setType0(pType);
		pType->addRef();
	}
}

void StrucSerializer_t::saveFields(std::ostream &os, const FieldMap &l)
{
	unsigned fieldsNum((unsigned)l.size());
	write(os, fieldsNum);
	for (FieldMapCIt I = l.begin(), E = l.end(); I != E; ++I)
	{
		CFieldPtr pField(VALUE(I));
CHECK(_CP == 93517)
STOP
		writeIdx(os, fieldToIdx(pField));
		write(os, KEY(I));//offset
	}
}

FieldPtr StrucSerializer_t::loadField(std::istream &is, FieldMap &m, int kind)
{
CHECK(_CP == 93517)
STOP

	FieldPtr pField(fieldFromIdx(readIdx(is)));
CHECKID(pField, 0x1ac8)
STOP
//	assert(!pField->hasUserData());//!LabelP ath(pField));//fundefs may have been loaded before
	if (!pField)
	{
		throw (-SR_INVALID_FIELD_ID);
	}
	FieldKeyType uKey;
	read(is, uKey);

	pField->overrideKey(uKey);

	if (mbLocal)
		;//assert(0);//?pField->key2loc(uKey);
	else if (kind > 1)//strucvar
		;//assert(pField->_key() == ADDR_STRUCVAR);//pField->setOffset(-1);
	else
	{
		assert(pField->_key() == uKey);//pField->setOffset(uKey);
	}

	m.insert(pField);
	return pField;
}


/////////////// NameRef_t


StrucSerializer_t::StrucSerializer_t()//MemmoryAccessor_t &rMemAcc, MemmoryAccessor_t &rMemAccG)
	: //MemmoryAccessor_t(rMemAcc),
	//mrMemAccG(rMemAccG),
	mbLocal(false)
{
}

void StrucSerializer_t::saveNameRef(std::ostream &os, CPNameRef pSelf)
{
	write(os, MyString(pSelf->c_str()));
}

void StrucSerializer_t::loadNameRef(std::istream &is, PNameRef pSelf)
{
	MyString s;
	read(is, s);
	pSelf->overrideKey_(s.c_str());
}


/////////////////////////////
// F u n c _ t

void GlobalSerializer_t::saveFunc(std::ostream &os, CTypePtr iSelf)
{
	const TypeProc_t &rSelf(*iSelf->typeProc());
	assert(rSelf.ObjType() == OBJID_TYPE_PROC);
	saveStruc(os, iSelf);
}

void GlobalSerializer_t::loadFunc(std::istream &is, TypePtr iSelf)
{
	const TypeProc_t &rSelf(*iSelf->typeProc());
	assert(rSelf.ObjType() == OBJID_TYPE_PROC);
	loadStruc(is, iSelf);
}

///////////////////////////////////////////////////////// FileModule_t

void GlobalSerializer_t::saveFileModule(std::ostream &os, const Folder_t &rSelf)
{
	saveFileFolder(os, rSelf);
	const FileModule_t &rPvt(*rSelf.fileModule());
	writeIdx(os, typeToIdx(rPvt.module()));
	write(os, FTYP__TOTAL);
	for (int i(1); i < FTYP__TOTAL; i++)
		writeIdx(os, folderToIdx(rPvt.special(i)));
}

void GlobalSerializer_t::loadFileModule(std::istream &is, Folder_t &rSelf)
{
	loadFileFolder(is, rSelf);
	FileModule_t &rPvt(*rSelf.fileModule());
	rPvt.setModule0(typeFromIdx(readIdx(is)));
	FTYP_Enum eFTyp;
	read(is, eFTyp);
	if (eFTyp != FTYP__TOTAL)
		throw(SR_FTYP_MISMATCH);
	for (int i(1); i < FTYP__TOTAL; i++)
		rPvt.setSpecial(i, folderFromIdx(readIdx(is)));
}


///////////////////////////////////////////////////////// FileFolder_t

void GlobalSerializer_t::saveFileFolder(std::ostream &os, const Folder_t &rSelf)
{
	const FoldersMap &m(rSelf.fileFolder()->children());
	unsigned chNum((unsigned)m.size());
	write(os, chNum);
	for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
	{
		CFolderPtr pFolder(&(*i));
		writeIdx(os, folderToIdx(pFolder));
		write(os, static_cast<MyString>(pFolder->_key()));
	}
}

void GlobalSerializer_t::loadFileFolder(std::istream &is, Folder_t &rSelf)
{
	FoldersMap &m(rSelf.fileFolder()->children());
	unsigned chNum;
	read(is, chNum);
	while (chNum--)
	{
		FolderPtr pFolder(folderFromIdx(readIdx(is)));
		if (!pFolder)
			throw (-SR_INVALID_FOLDER_ID);

		MyString sKey;
		read(is, sKey);
		pFolder->overrideKey(sKey);

		m.insert(pFolder);

		pFolder->setParent(&rSelf);
	}
}


///////////////////////////////////////////////////////// FileTypes_t

void GlobalSerializer_t::saveFileTypes(std::ostream &os, const FileTypes_t &rSelf)
{
}

void GlobalSerializer_t::loadFileTypes(std::istream &is, FileTypes_t &)
{
}


///////////////////////////////////////////////////////// FileNames_t

void GlobalSerializer_t::saveFileNames(std::ostream &, const FileNames_t &)
{
}

void GlobalSerializer_t::loadFileNames(std::istream &, FileNames_t &)
{
}

///////////////////////////////////////////////////////// FileExports_t

void GlobalSerializer_t::saveFileExports(std::ostream&, const FileExports_t&)
{
}

void GlobalSerializer_t::loadFileExports(std::istream&, FileExports_t&)
{
}

///////////////////////////////////////////////////////// FileImports_t

void GlobalSerializer_t::saveFileImports(std::ostream&, const FileImports_t&)
{
}

void GlobalSerializer_t::loadFileImports(std::istream&, FileImports_t&)
{
}

///////////////////////////////////////////////////////// FileRes_t

void GlobalSerializer_t::saveFileRes(std::ostream &os, const FileRes_t &rSelf)
{
}

void GlobalSerializer_t::loadFileRes(std::istream &is, FileRes_t &)
{
}

///////////////////////////////////////////////////////// FilesSerializer_t

/*FilesSerializer_t::FilesSerializer_t(FilesMgr_t &rFilesMgr, GlobalSerializer_t &rGlobal)
	: mrFilesMgr(rFilesMgr),
	mrMemAccG(rGlobal),
	mrA(rGlobal.memMgr().mFiles.pool())
{
}*/

bool GlobalSerializer_t::saveFile(std::ostream &os, const Folder_t &rFile, FILEID_t u)
{
	switch (u)
	{
	case FILEID_NULL:
		break;
	case FILEID_MODULE:
		saveFileModule(os, rFile);
		break;
	case FILEID_FOLDER:
		saveFileFolder(os, rFile);
		break;
	case FILEID_RESOURCES:
		saveFileRes(os, *rFile.fileRes());
		break;
	case FILEID_TYPES:
		saveFileTypes(os, *rFile.fileTypes());
		break;
	case FILEID_NAMES:
		saveFileNames(os, *rFile.fileNames());
		break;
	case FILEID_EXPORTS:
		saveFileExports(os, *rFile.fileExports());
		break;
	case FILEID_IMPORTS:
		saveFileImports(os, *rFile.fileImports());
		break;

	default:
		return false;
	}
	return true;
}

bool GlobalSerializer_t::loadFile(std::istream &is, Folder_t &rFolder, FILEID_t u)
{
	switch (u)
	{
	case FILEID_NULL:
		rFolder.SetPvt(new File_t);
		break;
	case FILEID_MODULE:
		rFolder.SetPvt(new FileModule_t);
		loadFileModule(is, rFolder);
		break;
	case FILEID_FOLDER:
		rFolder.SetPvt(new FileFolder_t);
		loadFileFolder(is, rFolder);
		break;
	case FILEID_RESOURCES://?
		rFolder.SetPvt(new FileRes_t);
		loadFileRes(is, *rFolder.fileRes());
		break;
	case FILEID_TYPES:
		rFolder.SetPvt(new FileTypes_t);
		loadFileTypes(is, *rFolder.fileTypes());
		break;
	case FILEID_NAMES:
		rFolder.SetPvt(new FileNames_t(nullptr));
		loadFileNames(is, *rFolder.fileNames());
		break;
	case FILEID_EXPORTS:
		rFolder.SetPvt(new FileExports_t(nullptr));
		loadFileExports(is, *rFolder.fileExports());
		break;
	case FILEID_IMPORTS:
		rFolder.SetPvt(new FileImports_t(nullptr));
		loadFileImports(is, *rFolder.fileImports());
		break;
	default:
		return false;
	}	
	return true;
}

///////////////////////////////////////////////////////// Folder_t

void GlobalSerializer_t::save(std::ostream &os, const Folder_t &rSelf)
{
	unsigned char u(rSelf.fileId());
	write(os, u);

	//writeIdx(os, folderToIdx(rSelf.pNext));
	//writeIdx(os, folderToIdx(rSelf.FirstChild()));
	write(os, rSelf.m_nFlags);
//	writeIdx(os, typeToIdx(rSelf.module()));
	//write(os, rSelf.name());
	write(os, rSelf.mDesc);

	if (!saveFile(os, rSelf, FILEID_t(u)))
		throw (-SR_INVALID_FILE_OBJECT);// "invalid type object";
}

void GlobalSerializer_t::load(std::istream &is, Folder_t &rSelf)
{
	unsigned char u;
	read(is, u);

	//rSelf.pNext = folderFromIdx(readIdx(is));
	//rSelf.SetChildren(folderFromIdx(readIdx(is)));
	read(is, rSelf.m_nFlags);
//	rSelf.setModule(typeFromIdx(readIdx(is)));

	//MyString s;
	//read(is, s);
	//rSelf.setName(s);
	read(is, rSelf.mDesc);

	if (!loadFile(is, rSelf, FILEID_t(u)))
		throw (-SR_INVALID_FILE_OBJECT);
}





//////////////////////////////////////////////////// Type_t


void StrucSerializer_t::writeType(std::ostream &os, CTypePtr pType)
{
	assert(pType);
	unsigned char u(&pType->pvt() ? pType->ObjType() : OBJID_NULL);
	write(os, u);
	assert(u != OBJID_NULL);
	if (!writeType(ObjId_t(u), os, pType))
		throw (-SR_INVALID_TYPE_OBJECT);// "invalid type object";
}

bool StrucSerializer_t::writeType(ObjId_t u, std::ostream &os, CTypePtr pType)
{
	switch (u)
	{
	case OBJID_TYPE_STRUC:
		saveStruc(os, pType);
		break;
	case OBJID_TYPE_STRUCVAR:
		saveStrucvar(os, pType);
		break;
	case OBJID_TYPE_ENUM:
		saveTypeEnum(os, pType);
		break;
	case OBJID_TYPE_CONST:
		saveTypeConst(os, pType);
		break;
	case OBJID_TYPE_PAIR:
		saveTypePair(os, pType);
		break;
	case OBJID_TYPE_FUNC:
		saveTypeFunc(os, pType);
		break;
	case OBJID_TYPE_ARRAY_INDEX:
		saveTypeArrayIndex(os, pType);
		break;
	/*case OBJID_TYPE_BIT:
		saveBit(os, pType);
		break;*/
	case OBJID_TYPE_BITSET:
		saveBitfield(os, pType);
		break;
/*	case OBJID_TYPE_UNION:
		saveUnion(os, pType);
		break;*/
	case OBJID_TYPE_PTR:
	case OBJID_TYPE_THISPTR:
		saveTypePtr(os, pType);
		break;
	case OBJID_TYPE_REF:
	case OBJID_TYPE_RREF:
		saveTypeRef(os, pType);
		break;
	case OBJID_TYPE_IMP:
		saveTypeImpPtr(os, pType);
		break;
	case OBJID_TYPE_EXP:
		saveTypeExpPtr(os, pType);
		break;
	case OBJID_TYPE_VPTR:
		saveTypeVPtr(os, pType);
		break;
	case OBJID_TYPE_ARRAY:
		saveArray(os, pType);
		break;
	case OBJID_TYPE_TYPEDEF:
		saveTypedef(os, pType);
		break;
	case OBJID_TYPE_SIMPLE:
		saveSimple(os, pType);
		break;
	default:
		return false;
	}

	//GlobalSerializer_t &rS(dynamic_cast<GlobalSerializer_t &>(mrMemAccG));
	//if (!rS.writeType(ObjId_t(pType->ObjType()), os, pType))
		//throw (-SR_INVALID_TYPE_OBJECT);// "invalid type object";
	return true;
}

void StrucSerializer_t::loadType(std::istream &is, TypePtr pType)
{
CHECKID(pType, 7377)
STOP
	unsigned char u;
	read(is, u);

	if (!loadType(ObjId_t(u), is, pType))
		throw (-SR_INVALID_TYPE_OBJECT);
}

Type_t *StrucSerializer_t::loadType(ObjId_t objId, std::istream &is, TypePtr pTypeRef)
{
	Type_t *p(nullptr);
	switch (objId)
	{
	case OBJID_TYPE_STRUC:
		p = new Struc_t;
		pTypeRef->SetPvt(p);
		loadStruc(is, pTypeRef);
		break;
	case OBJID_TYPE_STRUCVAR:
		p = new Strucvar_t;
		pTypeRef->SetPvt(p);
		loadStrucvar(is, pTypeRef);
		break;
	case OBJID_TYPE_ENUM:
		p = new TypeEnum_t;
		pTypeRef->SetPvt(p);
		loadTypeEnum(is, pTypeRef);
		break;
	case OBJID_TYPE_CONST:
		p = new TypeConst_t;
		pTypeRef->SetPvt(p);
		loadTypeConst(is, pTypeRef);
		break;
	case OBJID_TYPE_PAIR:
		p = new TypePair_t;
		pTypeRef->SetPvt(p);
		loadTypePair(is, pTypeRef);
		break;
	case OBJID_TYPE_FUNC:
		p = new TypeFunc_t;
		pTypeRef->SetPvt(p);
		loadTypeFunc(is, pTypeRef);
		break;
	case OBJID_TYPE_ARRAY_INDEX:
		p = new TypeArrayIndex_t;
		pTypeRef->SetPvt(p);
		loadTypeArrayIndex(is, pTypeRef);
		break;
	/*case OBJID_TYPE_BIT:
		p = new Bit_t;
		pTypeRef->SetPvt(p);
		loadBit(is, pTypeRef);
		break;*/
	case OBJID_TYPE_BITSET:
		p = new Bitset_t;
		pTypeRef->SetPvt(p);
		loadBitfield(is, pTypeRef);
		break;
/*	case OBJID_TYPE_UNION:
		p = new TypeUnion_t;
		pTypeRef->SetPvt(p);
		loadUnion(is, pTypeRef);
		break;*/
	case OBJID_TYPE_PTR:
		p = new TypePtr_t;
		pTypeRef->SetPvt(p);
		loadTypePtr(is, pTypeRef);
		break;
	case OBJID_TYPE_THISPTR:
		p = new TypeThisPtr_t;
		pTypeRef->SetPvt(p);
		loadTypePtr(is, pTypeRef);
		break;
	case OBJID_TYPE_REF:
		p = new TypeRef_t;
		pTypeRef->SetPvt(p);
		loadTypeRef(is, pTypeRef);
		break;
	case OBJID_TYPE_RREF:
		p = new TypeRRef_t;
		pTypeRef->SetPvt(p);
		loadTypeRef(is, pTypeRef);
		break;
	case OBJID_TYPE_IMP:
		p = new TypeImp_t;
		pTypeRef->SetPvt(p);
		loadTypeImpPtr(is, pTypeRef);
		break;
	case OBJID_TYPE_EXP:
		p = new TypeExp_t;
		pTypeRef->SetPvt(p);
		loadTypeExpPtr(is, pTypeRef);
		break;
	case OBJID_TYPE_VPTR:
		p = new TypeVPtr_t;
		pTypeRef->SetPvt(p);
		loadTypeVPtr(is, pTypeRef);
		break;
	case OBJID_TYPE_ARRAY:
		p = new Array_t;
		pTypeRef->SetPvt(p);
		loadArray(is, pTypeRef);
		break;
	case OBJID_TYPE_TYPEDEF:
		p = new Typedef_t;
		pTypeRef->SetPvt(p);
		loadTypedef(is, pTypeRef);
		break;
	case OBJID_TYPE_SIMPLE:
		p = new Simple_t;
		pTypeRef->SetPvt(p);
		loadSimple(is, pTypeRef);
		break;
	default:
		break;
	}

	/*if (!p)
	{
		GlobalSerializer_t &rS(dynamic_cast<GlobalSerializer_t &>(mrMemAccG));
		p = rS.loadType(ObjId_t(u), is, pTypeRef);
	}*/

	return p;
}


bool GlobalSerializer_t::writeType(ObjId_t objId, std::ostream &os, CTypePtr pType)
{
	assert(objId == pType->ObjType());
	switch (objId)
	{
	case OBJID_TYPE_PROJECT:
		saveProject(os, pType);
		break;
	case OBJID_TYPE_MODULE:
		saveModule(os, pType);
		break;
	case OBJID_TYPE_SEG:
		saveSeg(os, pType);
		break;
	case OBJID_TYPE_PROC:
		saveFunc(os, pType);
		break;
	case OBJID_TYPE_CODE:
		saveCode(os, pType);
		break;
	case OBJID_TYPE_THUNK:
		saveThunk(os, pType);
		break;
	default:
		return Base_t::writeType(objId, os, pType);
	}
	return true;
}

Type_t *GlobalSerializer_t::loadType(ObjId_t objId, std::istream &is, TypePtr pType)
{
	Type_t *p(nullptr);
	switch (objId)
	{
	case OBJID_TYPE_PROJECT:
		p = &mrProject;
		assert(!mrProject.self());
		mrProject.setSelfObj(pType);
		pType->SetPvt(p);
		loadProject(is, pType);
		break;
	case OBJID_TYPE_MODULE:
		p = new Module_t();
		pType->SetPvt(p);
		loadModule(is, pType);
		break;
	case OBJID_TYPE_SEG:
		p = new Seg_t;
		pType->SetPvt(p);
		loadSeg(is, pType);
		break;
	case OBJID_TYPE_PROC:
		p = new TypeProc_t;
		pType->SetPvt(p);
		loadFunc(is, pType);
		break;
	case OBJID_TYPE_CODE:
		p = new TypeCode_t;
		pType->SetPvt(p);
		loadCode(is, pType);
		break;
	case OBJID_TYPE_THUNK:
		p = new TypeThunk_t();
		pType->SetPvt(p);
		loadThunk(is, pType);
		break;
	default:
		p = Base_t::loadType(objId, is, pType);
	}
	return p;
}

/////////////////////////////////////////////////////////////// Type_t

/*void GlobalSerializer_t::Type_Save(std::ostream &os, TypePtr pSelf)
{
	StrucSerializer_t::save(os, pSelf->asObj());
}


void GlobalSerializer_t::Type_Load(std::istream &is, TypePtr pSelf)
{
	StrucSerializer_t::load(is, pSelf->asObj());
}*/

/////////////////////////////////////////////////////////////// TypeCode_t

void GlobalSerializer_t::saveCode(std::ostream &os, CTypePtr iSelf)
{
	const TypeCode_t &rSelf(*iSelf->typeCode());
	assert(rSelf.ObjType() == OBJID_TYPE_CODE);
	StrucSerializer_t::saveComplex(os, iSelf);
//	writeIdx(os, typeToIdx(rSelf.frontSeg()));
}


void GlobalSerializer_t::loadCode(std::istream &is, TypePtr iSelf)
{
	const TypeCode_t &rSelf(*iSelf->typeCode());
	assert(rSelf.ObjType() == OBJID_TYPE_CODE);
	StrucSerializer_t::loadComplex(is, iSelf);
	//rSelf.setFrontSeg(typeFromIdx(readIdx(is)));
}


///////////////////////////////////////////////////////TypeThunk_t

void GlobalSerializer_t::saveThunk(std::ostream &os, CTypePtr iSelf)
{
	const TypeThunk_t &rSelf(*iSelf->typeThunk());
	assert(rSelf.ObjType() == OBJID_TYPE_THUNK);
	writeIdx(os, typeToIdx(rSelf.baseType()));
}

void GlobalSerializer_t::loadThunk(std::istream &is, TypePtr iSelf)
{
	TypeThunk_t &rSelf(*iSelf->typeThunk());
	assert(rSelf.ObjType() == OBJID_TYPE_THUNK);
	TypePtr pCode(typeFromIdx(readIdx(is)));
	rSelf.setBaseType(pCode);
	if (pCode)
		pCode->addRef();
}

///////////////////////////////////////////////////////////////// Simple_t

void StrucSerializer_t::saveSimple(std::ostream &os, CTypePtr iSelf)
{
	const Simple_t &rSelf(*iSelf->typeSimple());
	//Type_Save(os, pSelf->typ eobj());
	write(os, rSelf.typeID());
}

void StrucSerializer_t::loadSimple(std::istream &is, TypePtr iSelf)
{
	Simple_t &rSelf(*iSelf->typeSimple());
	//Type_Load(is, rSelf.type obj());
	OpType_t t;
	read(is, t);
	rSelf.setTypeID(t);
}

///////////////////////////////////////////////////////////////// TypePtr_t

void StrucSerializer_t::saveTypePtr(std::ostream &os, CTypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_PTR || iSelf->ObjType() == OBJID_TYPE_THISPTR);
	const TypePtr_t &rSelf(*iSelf->typePtr());
	saveSimple(os, iSelf);
	writeIdx(os, typeToIdx(rSelf.pointee()));
}

void StrucSerializer_t::loadTypePtr(std::istream &is, TypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_PTR || iSelf->ObjType() == OBJID_TYPE_THISPTR);
	TypePtr_t &rSelf(*iSelf->typePtr());
	loadSimple(is, iSelf);
	TypePtr pPointee(typeFromIdx(readIdx(is)));
	rSelf.setPointee0(pPointee);
	if (pPointee)
		pPointee->addRef();
}

///////////////////////////////////////////////////////////////// TypeRef_t

void StrucSerializer_t::saveTypeRef(std::ostream &os, CTypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_REF || iSelf->ObjType() == OBJID_TYPE_RREF);
	const TypeRef_t &rSelf(*iSelf->typeRef());
	saveSimple(os, iSelf);
	writeIdx(os, typeToIdx(rSelf.pointee()));
}

void StrucSerializer_t::loadTypeRef(std::istream &is, TypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_REF || iSelf->ObjType() == OBJID_TYPE_RREF);
	TypeRef_t &rSelf(*iSelf->typeRef());
	loadSimple(is, iSelf);
	TypePtr pPointee(typeFromIdx(readIdx(is)));
	rSelf.setPointee0(pPointee);
	if (!pPointee)
		throw(-SR_INVALID_TYPE_REF);
	pPointee->addRef();
}

///////////////////////////////////////////////////////////////// TypePtrImp_t

void StrucSerializer_t::saveTypeImpPtr(std::ostream &os, CTypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_IMP);
	const TypeImp_t &rSelf(*iSelf->typeImp());
	writeIdx(os, typeToIdx(rSelf.baseType()));
}

void StrucSerializer_t::loadTypeImpPtr(std::istream &is, TypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_IMP);
	TypeImp_t &rSelf(*iSelf->typeImp());
	TypePtr pBaseType(typeFromIdx(readIdx(is)));
	if (!pBaseType)
		throw(-SR_INVALID_TYPE_REF);
	rSelf.setBaseType(pBaseType);
	pBaseType->addRef();
}

///////////////////////////////////////////////////////////////// TypePtrExp_t

void StrucSerializer_t::saveTypeExpPtr(std::ostream &os, CTypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_EXP);
	const TypeExp_t &rSelf(*iSelf->typeExp());
	writeIdx(os, typeToIdx(rSelf.baseType()));
}

void StrucSerializer_t::loadTypeExpPtr(std::istream &is, TypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_EXP);
	TypeExp_t &rSelf(*iSelf->typeExp());
	TypePtr pBaseType(typeFromIdx(readIdx(is)));
	if (!pBaseType)
		throw(-SR_INVALID_TYPE_REF);
	rSelf.setBaseType(pBaseType);
	pBaseType->addRef();
}

///////////////////////////////////////////////////////////////// TypeVImp_t

void StrucSerializer_t::saveTypeVPtr(std::ostream &os, CTypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_VPTR);
	//const TypeVPtr_t &rSelf(*iSelf->typeVPtr());
	saveSimple(os, iSelf);
}

void StrucSerializer_t::loadTypeVPtr(std::istream &is, TypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_VPTR);
	//TypeVPtr_t &rSelf(*iSelf->typeVPtr());
	loadSimple(is, iSelf);
}


///////////////////////////////////////////////////////////////// TypeConst_t

void StrucSerializer_t::saveTypeConst(std::ostream &os, CTypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_CONST);
	const TypeConst_t &rSelf(*iSelf->typeConst());
	writeIdx(os, typeToIdx(rSelf.baseType()));
}

void StrucSerializer_t::loadTypeConst(std::istream &is, TypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_CONST);
	TypeConst_t &rSelf(*iSelf->typeConst());
	TypePtr pBaseType(typeFromIdx(readIdx(is)));
	if (!pBaseType)
		throw(-SR_INVALID_TYPE_REF);
	rSelf.setBaseType0(pBaseType);
	pBaseType->addRef();
}

///////////////////////////////////////////////////////////////// TypePair_t

void StrucSerializer_t::saveTypePair(std::ostream &os, CTypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_PAIR);
	const TypePair_t &rSelf(*iSelf->typePair());
	writeIdx(os, typeToIdx(rSelf.left()));
	writeIdx(os, typeToIdx(rSelf.right()));
}

void StrucSerializer_t::loadTypePair(std::istream &is, TypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_PAIR);
	TypePair_t &rSelf(*iSelf->typePair());
	TypePtr iLeft(typeFromIdx(readIdx(is)));
	if (!iLeft)
		throw(-SR_INVALID_TYPE_REF);
	rSelf.setLeft(iLeft);
	iLeft->addRef();
	TypePtr iRight(typeFromIdx(readIdx(is)));
	if (!iRight)
		throw(-SR_INVALID_TYPE_REF);
	rSelf.setRight(iRight);
	iRight->addRef();
}



///////////////////////////////////////////////////////////////// TypeFunc_t

void StrucSerializer_t::saveTypeFunc(std::ostream &os, CTypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_FUNC);
	const TypeFunc_t &rSelf(*iSelf->typeFunc());
	writeIdx(os, typeToIdx(rSelf.left()));
	writeIdx(os, typeToIdx(rSelf.right()));
	write(os, rSelf.flags());
}

void StrucSerializer_t::loadTypeFunc(std::istream &is, TypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_FUNC);
	TypeFunc_t &rSelf(*iSelf->typeFunc());
	TypePtr iRetVal(typeFromIdx(readIdx(is)));
	if (iRetVal)
	{
		rSelf.setRetVal(iRetVal);
		iRetVal->addRef();
	}
	TypePtr iArgs(typeFromIdx(readIdx(is)));
	if (iArgs)
	{
		rSelf.setRight(iArgs);
		iArgs->addRef();
	}
	unsigned flags;
	read(is, flags);
	rSelf.setFlags(flags);
}


///////////////////////////////////////////////////////////////// TypeEnum

void StrucSerializer_t::saveTypeEnum(std::ostream &os, CTypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_ENUM);
	const TypeEnum_t &rSelf(*iSelf->typeEnum());
	saveSimple(os, iSelf);
	writeIdx(os, typeToIdx(rSelf.enumRef()));
	assert(!rSelf.enumRef()->typeEnum());
}

void StrucSerializer_t::loadTypeEnum(std::istream &is, TypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_ENUM);
	TypeEnum_t &rSelf(*iSelf->typeEnum());
	loadSimple(is, iSelf);
	TypePtr pEnumRef(typeFromIdx(readIdx(is)));
	assert(!pEnumRef->typeEnum());
	rSelf.setEnumRef(pEnumRef);
	if (pEnumRef)
		pEnumRef->addRef();
}


///////////////////////////////////////////////////////////////// TypeArrayIndex_t

void StrucSerializer_t::saveTypeArrayIndex(std::ostream &os, CTypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_ARRAY_INDEX);
	const TypeArrayIndex_t &rSelf(*iSelf->typeArrayIndex());
	writeIdx(os, typeToIdx(rSelf.arrayRef()));
	writeIdx(os, typeToIdx(rSelf.indexRef()));
}

void StrucSerializer_t::loadTypeArrayIndex(std::istream &is, TypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_ARRAY_INDEX);
	TypeArrayIndex_t &rSelf(*iSelf->typeArrayIndex());
	rSelf.setArrayRef(typeFromIdx(readIdx(is)))->addRef();
	rSelf.setIndexRef(typeFromIdx(readIdx(is)))->addRef();
}

//////////////////////////////////////////////////////////////// Array_t

void StrucSerializer_t::saveArray(std::ostream &os, CTypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_ARRAY);
	const Array_t &rSelf(*iSelf->typeArray());
	writeIdx(os, typeToIdx(rSelf.baseType()));
	write(os, rSelf.total0());
}

void StrucSerializer_t::loadArray(std::istream &is, TypePtr iSelf)
{
	assert(iSelf->ObjType() == OBJID_TYPE_ARRAY);
	Array_t &rSelf(*iSelf->typeArray());
//CHECKID(iSelf, 0x1635)
//STOP
	TypePtr pBaseType(typeFromIdx(readIdx(is)));
	assert(pBaseType);
	rSelf.setBaseType(pBaseType);
	pBaseType->addRef();
	unsigned total;
	read(is, total);
	rSelf.setTotal0(total);
}

//////////////////////////////////////////////////////////////// Typedef_t

void StrucSerializer_t::saveTypedef(std::ostream &os, CTypePtr iSelf)
{
	const Typedef_t &rSelf(*iSelf->typeTypedef());
	assert(rSelf.ObjType() == OBJID_TYPE_TYPEDEF);
	writeIdx(os, nameToIdx(rSelf.name()));
	writeIdx(os, typeToIdx(rSelf.baseType()));
}

void StrucSerializer_t::loadTypedef(std::istream &is, TypePtr iSelf)
{
	Typedef_t &rSelf(*iSelf->typeTypedef());
	assert(rSelf.ObjType() == OBJID_TYPE_TYPEDEF);
	rSelf.setNameRef(nameFromIdx(readIdx(is)));
	TypePtr pBaseType(typeFromIdx(readIdx(is)));
	if (pBaseType)
	{
		rSelf.setBaseType0(pBaseType);
		pBaseType->addRef();
	}
}

/////////////////////////////////////////////////////////////// NamesMgr_t

void StrucSerializer_t::save(std::ostream &os, const NamesMgr_t &rSelf)
{
	return;
}

void StrucSerializer_t::load(std::istream &is, NamesMgr_t &rSelf)
{
	return;//rebuild the namespace later
}


/////////////////////////////////////////////////////////////// Complex_t

void StrucSerializer_t::saveComplex(std::ostream &os, CTypePtr iSelf)
{
	const Complex_t &rSelf(*iSelf->typeComplex());
	//Type_Save(os, pSelf->typ eobj());
	bool bNSfollows0(rSelf.namesMgr() != nullptr);
	bool bNSfollows((iSelf->flags() & TYP_HAS_NMAP) != 0);
	assert(bNSfollows0 == bNSfollows);

	//write(os, bNSfollows);
	if (bNSfollows)
	{
		NamesMgr_t *pNs(rSelf.namesMgr());
		writeIdx(os, nameToIdx(pNs->name()));
		save(os, *pNs);
	}
}


void StrucSerializer_t::loadComplex(std::istream &is, TypePtr iSelf)
{
	Complex_t &rSelf(*iSelf->typeComplex());
	//?Type_Load(is, &rSelf.type obj());
	bool bNSfollows((iSelf->flags() & TYP_HAS_NMAP) != 0);
	//read(is, bNSfollows);
	if (bNSfollows)
	{
		NamesMgr_t *pNs(rSelf.assureNamespace());
		pNs->setNameRef(nameFromIdx(readIdx(is)));
		load(is, *pNs);
	}
}

#define NEW_TM_SAVE	1

/////////////////////////////////////////////////////////// TypesMgr_t

void StrucSerializer_t::saveTypeMap(std::ostream &os, const TypesMgr_t &rSelf)
{
	//writeIdx(os, typeToIdx(rSelf.owner()));

	for (TypesMgr_t::OrderIterator i(rSelf); i; i++)
	{
		TypePtr pType(*i);
//CHECKID(pType, 11379)
//STOP
		writeIdx(os, typeToIdx(pType));
		assert(pType->owner() == rSelf.owner());
	}

	writeEos(os);

	writeIdx(os, typeToIdx(rSelf.getTypeCode()));
}

void StrucSerializer_t::loadTypeMap(std::istream &is, TypesMgr_t &rSelf)
{
	assert(rSelf.owner());
	//TypePtr iOwner(typeFromIdx(readIdx(is)));
	//rSelf.setOwner(iOwner);
	//iOwner->typeStruc()->setTypesMgr0(&rSelf);

	TypeInfo_t *pti(nullptr);
	for (;;)
	{
		TypePtr iType(typeFromIdx(readIdx(is)));
		if (!iType)
			break;
		pti = rSelf.newNameAlias(iType, pti);//CAN't do here: all types must be loaded!
		iType->setOwner(rSelf.owner());
	}

	rSelf.setTypeCode(typeFromIdx(readIdx(is)));
}

/////////////////////////////////////////////////////////////// Struc_t

void StrucSerializer_t::saveStruc(std::ostream &os, CTypePtr iSelf)
{
//CHECKID(iSelf, -14)
//STOP
	saveComplex(os, iSelf);

	const Struc_t &rSelf(*iSelf->typeStruc());
	if (iSelf->flags() & TYP_HAS_TMAP)
		saveTypeMap(os, *rSelf.typeMgr());
	
	saveFields(os, rSelf.fields());
}

void StrucSerializer_t::loadStruc(std::istream &is, TypePtr iSelf)
{
	loadComplex(is, iSelf);

	Struc_t &rSelf(*iSelf->typeStruc());

	if (iSelf->flags() & TYP_HAS_TMAP)
	{
		assert(!rSelf.typeMgr());
		//ProjectInfo_t::assureTypeMgr(iSelf);
		ProjectInfo_t::NewTypesMgr(iSelf);//register it with binary later
		loadTypeMap(is, *rSelf.typeMgr());
	}

//	int size;
//	read(is, size);
//	rSelf.setSize0(size);

	int kind;
	/*if (rSelf.typeUnion())
		kind = 1;
	else*/ if (rSelf.typeStrucvar())
		kind = 2;
	else
		kind = 0;

	unsigned fieldsNum;
	read(is, fieldsNum);
	while (fieldsNum--)
	{
//CHECK(fieldsNum == 6)
//STOP
		FieldPtr pField(loadField(is, rSelf.fields(), kind));
		pField->setOwnerComplex(iSelf);
	}
}


///////////////////////////////////////////////////////////// Strucvar_t

void StrucSerializer_t::saveStrucvar(std::ostream &os, CTypePtr iSelf)
{
	saveComplex(os, iSelf);
}

void StrucSerializer_t::loadStrucvar(std::istream &is, TypePtr iSelf)
{
	loadComplex(is, iSelf);
}

///////////////////////////////////////////////////////////// Bit_t

/*void StrucSerializer_t::saveBit(std::ostream &os, TypePtr iSelf)
{
	const Bit_t &rSelf(*iSelf->typeBit());
	write(os, OPTYP_BIT);
}

void StrucSerializer_t::loadBit(std::istream &is, TypePtr iSelf)
{
	const Bit_t &rSelf(*iSelf->typeBit());
	OpType_t t;
	read(is, t);
}*/

///////////////////////////////////////////////////////////// Bitfield_t

void StrucSerializer_t::saveBitfield(std::ostream &os, CTypePtr iSelf)
{
	const Bitset_t &rSelf(*iSelf->typeBitset());
	assert(rSelf.ObjType() == OBJID_TYPE_BITSET);
	saveStruc(os, iSelf);
}

void StrucSerializer_t::loadBitfield(std::istream &is, TypePtr iSelf)
{
	const Bitset_t &rSelf(*iSelf->typeBitset());
	loadStruc(is, iSelf);
}


///////////////////////////////////////////////////////////// TypeUnion_t

/*void StrucSerializer_t::saveUnion(std::ostream &os, CTypePtr iSelf)
{
	//assert(rSelf.typeUnion());
	const TypeUnion_t &rSelf(*iSelf->typeUnion());
	saveStruc(os, iSelf);
}

void StrucSerializer_t::loadUnion(std::istream &is, TypePtr iSelf)
{
	const TypeUnion_t &rSelf(*iSelf->typeUnion());
	loadStruc(is, iSelf);
}*/



/////////////////////////////////////////////////////////// Raw_t

void GlobalSerializer_t::saveRaw(std::ostream &os, const Block_t &rSelf)
{
	write(os, rSelf.m_offs);
	write(os, rSelf.m_size);
}

void GlobalSerializer_t::loadRaw(std::istream &is, Block_t &rSelf)
{
	read(is, rSelf.m_offs);
	read(is, rSelf.m_size);
}


/////////////////////////////////////////////////////////// GlobalSerializer_t::

void GlobalSerializer_t::saveRangeMgr(std::ostream &os, const Module_t &rSelf)
{
	const RangeSetMap &l(rSelf.rangeMgr());
	for (RangeSetMapCIt i(l.begin()); i != l.end(); i++)
	{
		TypePtr iSegRange(*i);
		if (iSegRange)//do not save empty ones (it is possible they could exist)
			writeIdx(os, typeToIdx(iSegRange));
	}
	writeEos(os);//end of sets of ranges
}

void GlobalSerializer_t::loadRangeMgr(std::istream &is, Module_t &rSeg)
{
	for (;;)
	{
		TypePtr iSegRange(typeFromIdx(readIdx(is)));
		if (!iSegRange)
			break;
		rSeg.addSubRange(iSegRange);
		//iSegRange->addRef();
	}
}

/////////////////////////////////////////////////////////// Seg_t

void GlobalSerializer_t::saveSeg(std::ostream &os, CTypePtr iSelf)
{
	saveStruc(os, iSelf);

	const Seg_t &rSelf(*iSelf->typeSeg());

//OS_CHECK(z1);
	write(os, rSelf.title());
//	write(os, rSelf.base(iSelf));
	write(os, rSelf.base64());
	saveRaw(os, rSelf.rawBlock());
	write(os, rSelf.uflags());
	//write(os, rSelf.entryPoint());

	writeIdx(os, typeToIdx(rSelf.traceLink()));
	write(os, rSelf.addressP());

	for (SubSegMapCIt i(rSelf.subsegs().begin()); i != rSelf.subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));
		writeIdx(os, typeToIdx(iSeg));
//?		ADDR va(KEY(i));
//?		write(os, va);
	}
	writeEos(os);

	//conflicting fields
	//const ConflictFieldMap &m2(rSelf.conflictFields());
	unsigned extraFieldsNum(0);// (unsigned)m2.size());
	write(os, extraFieldsNum);
	/*for (ClonedFieldMapCIt it(m2.begin()); it != m2.end(); it++)
	{
		CFieldPtr pField(VALUE(it));
		writeIdx(os, fieldToIdx(pField));
		write(os, KEY(it));//offset
	}*/

	//unresolved fields
#if(0)
	const BogusFieldMap &m3(rSelf.bogusFields());
	unsigned num3((unsigned)m3.size());
	write(os, num3);
	for (BogusFieldMap::const_iterator it(m3.begin()); it != m3.end(); it++)
	{
		FieldPtr pField(it->second);
		writeIdx(os, fieldToIdx(pField));
		write(os, it->first);//offset
	}
#endif

	const SlotVector<FRONT_t> &fe(mrProject.frontVec());
	int fIdx(fe.squeezedIndex(mrProject.frontFromIndex(rSelf.frontIndex())));
	write(os, fIdx);
	//write(os, rSelf.frontName());
}

void GlobalSerializer_t::loadSeg(std::istream &is, TypePtr iSelf)
{
	loadStruc(is, iSelf);

	Seg_t &rSelf(*iSelf->typeSeg());

//IS_CHECK(z1);
	MyString title;
	read(is, title);
	rSelf.setTitle(title);

	uint64_t base64;
	read(is, base64);
	rSelf.setBase64(base64);

	loadRaw(is, rSelf.rawBlock());

	uint32_t uflags;
	read(is, uflags);
	rSelf.setUFlags(uflags);

	//ADDR entryPoint;
	//read(is, entryPoint);
	//rSelf.setEntryPoint(entryPoint);

	TypePtr traceLink(typeFromIdx(readIdx(is)));
	if (traceLink)
		rSelf.setTraceLink0(traceLink);//the superior may not be loaded at this point

	ADDR address;
	read(is, address);
	rSelf.setAddressP(address);

	//subsegs
	for (;;)
	{
		TypePtr iSeg(typeFromIdx(readIdx(is)));
		if (!iSeg)
			break;
		rSelf.subsegs().push_back(iSeg);
		iSeg->addRef();
	}

	//conflicting fields
	//ConflictFieldMap &m2(rSelf.conflictFields());
	unsigned num2;
	read(is, num2);
	/*while (num2--)
	{
		FieldPtr pField(fieldFromIdx(readIdx(is)));
		if (!pField)
			throw (-SR_INVALID_FIELD_ID);

		FieldKeyType uKey;
		read(is, uKey);
		pField->overrideKey(uKey);

		//m2.insert(std::make_pair(uKey, pField));
		m2.insert(pField);
		assert(pField->_key() == uKey);//?		pField->setOffset(uKey);
		pField->setOwnerComplex(iSelf);
	}*/

#if(0)
	//unresolved fields
	BogusFieldMap &m3(rSelf.bogusFields());
	unsigned num3;
	read(is, num3);
	while (num3--)
	{
		FieldPtr pField(fieldFromIdx(is));
		if (!pField)
			throw (-SR_INVALID_FIELD_ID);

		FieldKeyType uKey;
		read(is, uKey);
		pField->overrideKey(uKey);

		m3.insert(std::make_pair(uKey, pField));
		assert(pField->_key() == uKey);//?		pField->setOffset(uKey);
		pField->setOwnerComplex(iSelf);
	}
#endif

	int fIdx;
	read(is, fIdx);
	if (fIdx >= 0)
	{
		rSelf.setFrontIndex(fIdx + 1);
		const SlotVector<FRONT_t> &fe(mrProject.frontVec());
		FRONT_t *pF(mrProject.frontFromIndex(rSelf.frontIndex()));
		if (pF)
			pF->AddRef();
	}
	
	//MyString sFront;
	//read(is, sFront);
	//rSelf.setFrontEnd(sFront, 0);//+1 ref
}

/////////////// TypeObj_t

void StrucSerializer_t::saveTypeObj(std::ostream &os, CTypePtr iSelf)
{
//CHECK(rSelf.ID() < 0)
CHECKID(iSelf, 0x844)
STOP
	//const TypeObj_t &rSelf(*iSelf);
	save(os, iSelf->asObj());
	writeType(os, iSelf);
}

void StrucSerializer_t::loadTypeObj(std::istream &is, TypePtr iSelf)
{
CHECKID(iSelf, 0x36f0)
STOP
	load(is, iSelf->asObj());
	loadType(is, iSelf);
}


///////////////////////////////////////////////////////////// Project_t

void GlobalSerializer_t::saveProject(std::ostream &os, CTypePtr iSelf)
{
	const Project_t &rSelf(*iSelf->typeProject());
	saveSeg(os, iSelf);

	const SlotVector<FRONT_t> &fe(rSelf.frontVec());
	int fsz(fe.squeezedIndex(nullptr));
	write(os, -fsz);//get the size
	for (size_t i(1); i < fe.size(); i++)
	{
		FRONT_t *pfe(fe.getSlot(i));
		if (pfe)
		{
			write(os, pfe->id());
			write(os, pfe->name());
		}
	}

	//saveTypeMap(os, *rSelf.typeMgr());
	//saveSeg(os, iSelf);
}

void GlobalSerializer_t::loadProject(std::istream &is, TypePtr iSelf)
{
	Project_t &rSelf(*iSelf->typeProject());
	loadSeg(is, iSelf);

	SlotVector<FRONT_t> &fe(rSelf.frontVec());
	unsigned fsz;
	read(is, fsz);
	for (unsigned i(0); i < fsz; i++)
	{
		int id;
		read(is, id);
		MyString s;
		read(is, s);
		FRONT_t *pF(new FRONT_t(s, id));
		if (fe.newSlot(pF) != i + 1)
			ASSERT0;
	}

	//loadTypeMap(is, *rSelf.typeMgr());
	//loadSeg(is, iSelf);
}



///////////////////////////////////////////////////////////// Module_t

void GlobalSerializer_t::saveModule(std::ostream &os, CTypePtr iSelf)
{
CHECKID(iSelf, 1584)
STOP
	const Module_t &rSelf(*iSelf->typeModule());

	saveSeg(os, iSelf);

	write(os, rSelf.unique());
	write(os, rSelf.moduleTag());
	writeIdx(os, dataToIdx(rSelf.dataSourcePtr0()));
	writeIdx(os, folderToIdx(rSelf.folderPtr()));
	write(os, rSelf.subTitle());
	write(os, rSelf.delayedFormat());

	saveRangeMgr(os, rSelf);
}

void GlobalSerializer_t::loadModule(std::istream &is, TypePtr iSelf)
{
CHECKID(iSelf, 2552)
STOP

	Module_t &rSelf(*iSelf->typeModule());

	loadSeg(is, iSelf);

	int unique;
	read(is, unique);
	rSelf.setUnique(unique);

	int tag;
	read(is, tag);
	rSelf.setModuleTag(tag);

	DataPtr pData(dataFromIdx(readIdx(is)));
	if (pData)
	{
		rSelf.setDataSource(pData);
		pData->addRef();
	}

	rSelf.setFolderPtr(folderFromIdx(readIdx(is)));

	MyString s;
	read(is, s);
	rSelf.setSubTitle(s);
	read(is, s);
	rSelf.setDelayedFormat(s);
	//FilesSerializer_t a(rSelf.files(), *this);
	//a.load(is);
	//rSelf.files().destruct(mrMemMgr);
	loadRangeMgr(is, rSelf);
}



//////////////////////////////////////////////////// DataObj_t

void GlobalSerializer_t::save(std::ostream &os, const DataObj_t &rSelf)
{
	unsigned char u(rSelf.pvt().dataId());
	write(os, u);
	write(os, rSelf.name());
	saveDataObj(os, rSelf, DATAID_t(u));
}

void GlobalSerializer_t::load(std::istream &is, DataObj_t &rSelf)
{
	unsigned char u;
	read(is, u);
	MyString s;
	read(is, s);
	rSelf.setName(s);
	loadDataObj(is, rSelf, DATAID_t(u));
}

void GlobalSerializer_t::saveDataObj(std::ostream &os, const DataObj_t &rSelf, DATAID_t u)
{
	switch (u)
	{
	case DATAID_SOURCE:
		saveDataSource(os, *rSelf.pvt().dataSource());
		break;
	case DATAID_LEECH:
		saveDataLeech(os, *rSelf.pvt().dataLeech());
		break;
	default:
		throw (-SR_INVALID_DATA_OBJECT);
	}
}

void GlobalSerializer_t::loadDataObj(std::istream &is, DataObj_t &rSelf, DATAID_t u)
{
	switch (u)
	{
	case DATAID_SOURCE:
	{
		DataSource_t *p(new DataSource_t);
		rSelf.setPvt(p);
		loadDataSource(is, *p);
		break;
	}
	case DATAID_LEECH:
	{
		DataLeech_t *p(new DataLeech_t);
		rSelf.setPvt(p);
		loadDataLeech(is, *p);
		break;
	}
	default:
		throw (-SR_INVALID_DATA_OBJECT);
	}
}

///////////////////////////////////////////////////// DataSource_t

void GlobalSerializer_t::saveDataSource(std::ostream &os, const DataSource_t &rSelf)
{
	//write(os, rSelf.Name);
	
	write(os, rSelf.path());
	//writeIdx(os, typeToIdx(rSelf.miPrimeSeg));

/*	if (!rSelf.path())//write raw binary as a whole (compressed?)
	{
	}*/

	saveAuxData(os, rSelf);
}

void GlobalSerializer_t::loadDataSource(std::istream &is, DataSource_t &rSelf)
{
	MyString sPath;
	read(is, sPath);
	//rSelf.setPath(sPath);

//?	read(is, rSelf.mPath);

	if (!sPath.isEmpty())
	{
		if (!rSelf.load(sPath))
			throw (-SR_FAILED_LOAD_BINARY);
		//MI.SetDataSource(pDataSource);
	}

/*?	unsigned u;
	is.read((char *)&u, sizeof(u));

	if (rSelf.rawBlock().rawsize() != u)
		throw (-SR_BINARY_SIZE_UNMATCHED);*/

	loadAuxData(is, rSelf);
}


void GlobalSerializer_t::saveAuxData(std::ostream &os, const I_DataSource &rSelf)
{
	Block_t raw;
	PDATA ptr(nullptr);
	const I_AuxData *paux(rSelf.aux());
	if (paux)
	{
		raw.m_size = paux->size();
		ptr = paux->data();
	}

	saveRaw(os, raw);
	if (raw.m_size > 0)
		write(os, ptr, (size_t)raw.m_size);//Ccheck size?
}

void GlobalSerializer_t::loadAuxData(std::istream &is, I_DataSource &rSelf)
{
	Block_t raw;
	loadRaw(is, raw);
	if (raw.m_size > 0)
	{
		PDATA ptr(new char[(size_t)raw.m_size]);
		read(is, ptr, (size_t)raw.m_size);
		rSelf.setAuxData(ptr, (size_t)raw.m_size);
		delete[] ptr;
	}
}



//////////////////////////////////////////////////// DataLeech_t

void GlobalSerializer_t::saveDataLeech(std::ostream &os, const DataLeech_t &rSelf)
{
	writeIdx(os, dataToIdx(rSelf.dataHost()));
	const data_leech_t::V_t &v(rSelf.chunks());
	unsigned uChunks((unsigned)v.size());
	write(os, uChunks);
	for (size_t i(0); i < uChunks; i++)
	{
		std::pair<unsigned, unsigned> u((unsigned)v[i].upper_self, (unsigned)v[i].lower_host);
		write(os, u);
	}
	saveAuxData(os, rSelf);
}

void GlobalSerializer_t::loadDataLeech(std::istream &is, DataLeech_t &rSelf)
{
	DataPtr pDataSource(dataFromIdx(readIdx(is)));
	if (!pDataSource)
		throw (-SR_FAILED_LOAD_BINARY);
	rSelf.setDataHost(pDataSource);
	pDataSource->addRef();
	unsigned n;
	read(is, n);
	data_leech_t::V_t &v(rSelf.chunks());
	v.resize(n);
	for (unsigned i(0); i < n; i++)
	{
		std::pair<unsigned, unsigned> u;
		read(is, u);
		v[i].upper_self = u.first;
		v[i].lower_host = u.second;
	}
	loadAuxData(is, rSelf);
}


//////////////////////////////////////////////////// 

void GlobalSerializer_t::Save(std::ostream &os)
{
	//mPath = rPath;

	os.write(g_Magic, sizeof(g_Magic));
	os.write((char *)&g_Version, sizeof(g_Version));

	save(os);

	const FilesMgr0_t &rFiles(mrProject.files());
	writeIdx(os, folderToIdx(rFiles.rootFolder()));
	writeIdx(os, folderToIdx(mrProject.startupFolder()));

		// ptrs to binaries in folder
/*	for (FileTree_t::ChildrenIterator it(mrProject.files().rootFolder()); it; it++)
	{
		Folder_t &rFolder(*it.data());
		writeIdx(os, typeToIdx(rFolder.m.miBinary));
	}*/
	//writeEos(os);

/*?	unsigned u(rSelf.rawBlock().rawsize());
	//if (!mbImbedRawData)
		//u = 0;
	os.write((char *)&u, sizeof(u));
#if(0)
	if (u > 0)
		os.write((char *)rSelf.rawBlock().data(), rSelf.rawBlock().size());
#endif*/

	//mrProject.setPath(mPath);
}

void GlobalSerializer_t::Load(std::istream &is)
{
	//mPath = rPath;

	char magic[4];
	is.read(magic, sizeof(magic));
	if (strncmp(magic, g_Magic, sizeof(magic)) != 0)
	{
		throw (-SR_INVALID_PROJECT);// "Not a valid ADC project";
	}

	uint32_t version;
	is.read((char *)&version, sizeof(version));
	if (version != g_Version)
	{
		throw (-SR_WRONG_VERSION);// "Wrong version";
	}

	/*	Project_t *pSelf(dynamic_cast<Project_t *>(readObj(is)));
	if (!pSelf)
	throw "Wrong project objid";*/

	ProjectInfo_t PJ(mrProject);

	load(is);

	FilesMgr0_t &rFiles(mrProject.files());
	rFiles.setRootFolder(folderFromIdx(readIdx(is)));
	mrProject.setStartupFolder(folderFromIdx(readIdx(is)));

		//ptrs to binaries
	/*for (FileTree_t::ChildrenIterator it(rFilesMgr.rootFolder()); it; it++)
	{
		Folder_t &rFolder(*it.data());
		rFolder.m.miBinary = typeFromIdx(readIdx(is));
	}*/

}

/*void GlobalRecoverer_t::recoverFiles(FilesMgr0_t &rFilesMgr)
{
	//recover
	for (FileTree_t::Iterator i(rFilesMgr.mRootFolder); i; i++)
	{
		Folder_t *r(*i);
		r->SetParent(i.parent());
		Folder_t *p(i.top());
		r->pNext->SetPrev(p);
		//STOP
	}
}*/

void GlobalRecoverer_t::recover()
{
	const FoldersMap &m(mrProject.rootFolder().children());
	//recoverFiles(rFiles);

	assert(mrProject.typeMgr());
	std::vector<TypePtr> v;
	v.push_back(mrProject.self());
	recoverTypesMgr(v, *mrProject.typeMgr());
	RegisterTypesMap(mrProject.self(), true);

	for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
	{
		CFolderRef rFolder(*i);
		//if (!IsPhantomFolder(rFolder))
		{
			TypePtr iModule(rFolder.fileModule()->module());
			Module_t &aModule(*iModule->typeModule());
			std::vector<TypePtr> v;
			v.push_back(iModule);
			recoverSeg(v, aModule.namesMgr() ? iModule : nullptr);

			//recover rangesegs
			const RangeSetMap &l(aModule.rangeMgr());
			for (RangeSetMapCIt i(l.begin()); i != l.end(); i++)
			{
				TypePtr iSegRange(*i);
				Seg_t &rSegRange(*iSegRange->typeSeg());
				FieldMap &m(rSegRange.fields());
				for (FieldMapIt i = m.begin(), E = m.end(); i != E; ++i)
				{
					FieldPtr pField(VALUE(i));
					TypePtr iType(pField->type());
					if (iType)
					{
						assert(!iType->parentField());
						iType->setParentField(pField);
					}
				}
			}

			v.pop_back();
			//aBin.recoverRawPtr(aBin.rawBlock().ptr);
			ModuleInfo_t MI(*this, *iModule);
			MI.UpdateViewGeometry2();
		}
	}
}

Project_t &GlobalSerializer_t::GetProject() const
{
#if(0)
	TMemoryPool<TypeObj_t>::EltIterator i((TMemoryPool<TypeObj_t>::THelper<TypeObj_t> &)mrT);
	TypePtr iProj(&(*i));
	return iProj;
#else
	return mrProject;
#endif
}

void GlobalRecoverer_t::recoverTypesMgr(std::vector<TypePtr> &v, TypesMgr_t &rSelf)
{
	TypePtr iNSOwner(rSelf.owner());
	NamesMgr_t *pNs(iNSOwner->typeComplex()->namesMgr());
	if (!pNs)
	{
		throw (-SR_RECOVERY_FAIL);// assert(0);
		pNs = &OwnerNamespaceEx(rSelf.owner());
	}

	assert(rSelf.empty());//not yet populated with akas
	//const TypesMap_t &m(rSelf.aliases());
	//for (TypesMapCIt i(m.begin()); i != m.end(); i++)
	for (TypesMgr_t::OrderIterator i(rSelf); i; i++)
	{
		TypePtr iType(*i);
CHECKID(iType, 7377)
STOP

		rSelf.insertNameAlias(const_cast<TypeInfo_t *>(i.typeInfo()));
		//TypePtr iType(i.pvt()->pSelf);

		if (iType->typeProxy())
		{
			if (iType->name())
			{
				PNameRef pn(iType->name());
//CHECK(MyString(pNameRef->c_str()) == "std")
//STOP
				if (!pNs->insertName(pn))
					ASSERT0;
				pn->setObj(iType);
				assert(pn->obj() == iType);
#if(SHARED_NAMES)
				pNameRef->addRef();
#endif
			}
		}
		else if (rSelf.isStockType(iType))
		{
			Simple_t *pType(iType->typeSimple());
			uint8_t t(pType->optype());
			const char *pstr(OpTyp2Str(t));
			PNameRef pn(new NameRef_t(pstr));
			iType->setName0(pn);
			if (!pNs->insertName(pn))
				ASSERT0;
			//assert(pn->obj() == iType);
		}
		else if (!iType->typeImp())
		{
			PNameRef pn(iType->name());
			if (pn)
			{
				if (!pNs->insertName(pn))
					ASSERT0;
				pn->setObj(iType);
			}
			if (iType->typeStruc())
			{
				assert(!iType->typeSeg());
				v.push_back(iType);
				recoverStruc(v, iType);
				v.pop_back();
			}
		}
	}
}

// for each global in (prime) seg (and sub-segs)
//-----------------------
// 1) global has NO name
//		1a) the name is ugly (FLD_UGLY_NAME set
//			1aa) alternative name exists in PrettyFieldsMap
//			1ab) NO alternative name in PrettyFieldsMap.
// 2) global has name (in prime seg's n-map)
//		2a) the name is ugly (FLD_UGLY_NAME set, it cannot be displayed in DC context)
//			2aa) alternative name exists in PrettyFieldsMap
//				2aaa) global is scoped (static member of a class). the alt name is owned by n-map of the class.
//				2aab) global is UN-scoped (just a global). the alt name is owned by n-map of the prime seg.
//			2ab) NO alternative name in PrettyFieldsMap.
//				2aba) global is scoped; auto name is displayed;
//				2abb) global is UN-scoped;  auto name is displayed;
//		2b) the name is NOT ugly (FLD_UGLY_NAME cleared), in prime seg's n-map
//			2ba) global CANNOT be scoped(!)


void Project_t::OnRecoverField(FieldPtr pSelf, TypePtr pModule, TypePtr pScope)
{
	TypePtr pScope0 = GetNameOwnerOf(pSelf);//MAKE IT FASTER!
	assert(pScope && pScope == pScope0);
	//if (!pScope)
		//pScope = pScope0;

	NamesMgr_t *pNS(pScope->typeComplex()->namesMgr());
	assert(pSelf->name()->obj() == pSelf);
	if (!pNS->insertName(pSelf->name()))
		ASSERT0;
}

void GlobalRecoverer_t::recoverField(std::vector<TypePtr> &v, FieldRef rSelf, TypePtr pScope)
{
CHECKID((&rSelf), 0x149cd)
STOP

	if (rSelf.name())
	{
//CHECK(strcmp(rSelf.name()->c_str(), "ZZZ") == 0)
//STOP
		mrProject.OnRecoverField(&rSelf, v.front(), pScope);
	}

	TypePtr iType(rSelf.type());
	if (iType)
	{

		if (!iType->owner())
		{
CHECKID(iType, 0xb83)
STOP
			assert(!iType->parentField());
			iType->setParentField(&rSelf);//recover parent field for non-shared typerefs
		}

		if (!iType->isShared())//warning: funcdefs may refer to not yet loaded types!
		{
			Complex_t *pTypeCplx(iType->typeComplex());//so this can crash
			if (pTypeCplx)
			{
				//		if (pTypeCplx->namesMgr())
				//		pTypeCplx->namesMgr()->initialize();
				//iType->typeComplex()->recoverNamespace();

				//non shared strucs can also mange the type lists
				Struc_t *pStruc(pTypeCplx->typeStruc());
				if (pStruc && pStruc->typeMgr())
				{
					TypePtr iModule(v.front());
					assert(iModule->typeModule());
					ModuleInfo_t MI(*this, *iModule);
					recoverTypesMgr(v, *pStruc->typeMgr());
					MI.RegisterTypesMapEx(iType, true);
				}
			}
		}
	}

	//mrProject.OnRecoverField(&rSel, v.front(), );
//	assert(!iType->typeSeg());
}

void GlobalRecoverer_t::recoverStruc(std::vector<TypePtr> &v, TypePtr pScope0)
{
	TypePtr iStruc(v.back());
//CHECKID(iStruc, 0x3774)
//CHECK(iStruc->typeUnionLoc())
//STOP

	Struc_t &rStruc(*iStruc->typeStruc());
	assert(!iStruc->parentField());//shared!

//	if (rStruc.namesMgr())
//		rStruc.namesMgr()->initialize();

	if (rStruc.typeMgr())
	{
		recoverTypesMgr(v, *rStruc.typeMgr());
		if (v.front()->typeModule())
		{
			ModuleInfo_t MI(*this, *v.front());
			MI.RegisterTypesMapEx(iStruc, true);
		}
		else
		{
			assert(v.front()->typeProject());
			RegisterTypesMap(iStruc, true);
		}
	}

	for (Struc_t::HierIterator i(iStruc); i; i++)
	{
		FieldRef rField(*i);
//CHECK(rField._key() == 0x5B8694)
CHECKID((&rField), 0xedd7)
STOP
		TypePtr pScope(i.NamespaceStruc());
		if (!pScope)
			pScope = pScope0;
		recoverField(v, rField, pScope);//pScope);
	}

	//recoverNamespace();//parent fields must be initialized at this point?
	mrProject.OnRecoverStruc(v);
}

void GlobalRecoverer_t::recoverSeg(std::vector<TypePtr> &v, TypePtr pScope0)
{
	TypePtr iSeg(v.back());
	assert(iSeg->typeSeg());
	Seg_t &rSeg(*iSeg->typeSeg());
	TypePtr pScope(rSeg.namesMgr() ? iSeg : pScope0);

	recoverStruc(v, pScope);

	for (SubSegMapCIt i(rSeg.subsegs().begin()); i != rSeg.subsegs().end(); i++)
	{
		TypePtr iSeg3(IVALUE(i));
		iSeg3->typeSeg()->setSuperLink(iSeg);
		v.push_back(iSeg3);
		recoverSeg(v, pScope);
		v.pop_back();
	}

	//extra (cloned) fields
	/*for (ClonedFieldMapIt j(rSeg.conflictFields().begin()); j != rSeg.conflictFields().end(); j++)
	{
		FieldRef rField(*VALUE(j));
		recoverField(v, rField, pScope);
	}*/
}
