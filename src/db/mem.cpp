#include "mem.h"
#include "prefix.h"

#include "shared/data_source.h"
#include "names.h"
#include "field.h"
#include "types.h"
#include "files.h"
#include "type_module.h"

#define CH_SIZE	16

MemoryMgr_t::MemoryMgr_t()
	: mFiles(CH_SIZE),
	mTypeRefs(CH_SIZE * 32),
	mNameRefs(CH_SIZE * 32),
	mFields(CH_SIZE * 64)
{
}

MemoryMgr_t::~MemoryMgr_t()
{
}


void MemoryMgr_t::print_stats(std::ostream &os)
{
	os << "[files]\n";
	mFiles.pool().print_stats(os);
	os << "[typerefs]\n";
	mTypeRefs.pool().print_stats(os);
	os << "[namerefs]\n";
	mNameRefs.pool().print_stats(os);
	os << "[fields]\n";
	mFields.pool().print_stats(os);
}



FolderPtr MemoryMgr_t::NewFile(MyString s)
{
	return mFiles.New(s);
}

void MemoryMgr_t::Delete(FolderPtr p)
{
	assert(p);
	mFiles.Delete(p);
}

/////////////////////////Field_t 

FieldPtr MemoryMgr_t::NewFieldNoId()
{
	return mFields.New();
}

FieldPtr MemoryMgr_t::NewField(Field_t&& o)
{
#if(NO_OBJ_ID)
	return mFields.New1(o);
#else
	return mFields.New1(o, IdCheck_t::idNext());
#endif
}

#if(!NO_OBJ_ID)
FieldPtr MemoryMgr_t::NewFieldId(int id)
{
	return mFields.New(id);
}
#endif

FieldPtr MemoryMgr_t::NewField(FieldKeyType key)
{
#if(NO_OBJ_ID)
	FieldPtr p(mFields.New());
#else
	FieldPtr p(mFields.New(IdCheck_t::idNext()));
#endif
	p->overrideKey(key);
	return p;
}

void MemoryMgr_t::Delete(FieldPtr p)
{
	if (!p)
		return;
	p->destruct(*this);
	mFields.Delete(p);
}

//////////////////////////TypeObj_t

#if(!NO_OBJ_ID)
TypeObj_t* MemoryMgr_t::NewTypeRefId(int id)
{
	return mTypeRefs.New(id);
}
#endif

TypeObj_t* MemoryMgr_t::NewTypeRef()
{
#if(!NO_OBJ_ID)
	return mTypeRefs.New(IdCheck_t::idNext());
#else
	return mTypeRefs.New();
#endif
}

TypeObj_t* MemoryMgr_t::NewTypeRef(Type_t *t)
{
	TypeObj_t *p(NewTypeRef());
	p->SetPvt(t);
	return p;
}

TypeObj_t* MemoryMgr_t::NewTypeRefNoId()
{
	return mTypeRefs.New();
}

TypeObj_t* MemoryMgr_t::NewTypeRefNoId(Type_t *t)
{
	TypeObj_t *p(NewTypeRefNoId());
	p->SetPvt(t);
	return p;
}

void MemoryMgr_t::Delete(TypeObj_t *p)
{
//CHECK(&p->pvt() && p->typeStrucvar())
//STOP
	assert(p);
	assert(!p->objGlob());
	p->destruct(*this);
	mTypeRefs.Delete(p);
}

//////////////////////////NameRef_t
NameRef_t* MemoryMgr_t::NewNameRef(const char *pc)
{
	NameRef_t *p(mNameRefs.New(pc));
	//p->setPvt(s);
	//p->setObj(pObj);
	return p;
}

NameRef_t* MemoryMgr_t::NewNameRef()
{
	return mNameRefs.New();
}

void MemoryMgr_t::Delete(NameRef_t *p)
{
	if (p)
		mNameRefs.Delete(p);
}

/////////////////////////////////

/*TypesMgr_t*	MemoryMgr_t::NewTypesMap()
{
	TypesMgr_t *p(new TypesMgr_t);//mTypeMaps.New());
	return p;
}

void MemoryMgr_t::Delete(TypesMgr_t *p)
{
	if (p)
		delete p;//mTypeMaps.Delete(p);
}*/

/////////////////////////////////

DataObj_t*	MemoryMgr_t::NewDataObj()
{
	DataObj_t *p(mDataObjs.New());
	return p;
}

void MemoryMgr_t::Delete(DataObj_t *p)
{
	if (p)
		mDataObjs.Delete(p);
}
