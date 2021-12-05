#pragma once

#include "db/types.h"
#include "db/field.h"

typedef unsigned long GlobKey;

#include "shared/sbtree2.h"
typedef sbtree::node_s<GlobObj_t, GlobKey>			GlobNodeBase;


///////////////////////////////////////////
class GlobObj_t : public TypeObj0_t,
	public GlobNodeBase
{
public:
	GlobObj_t()
		: TypeObj0_t(OBJID_GLOBOBJ),
		GlobNodeBase(0)
	{
	}
#if(!NO_OBJ_ID)
	GlobObj_t(int id)
		: TypeObj0_t(OBJID_GLOBOBJ, id),
		GlobNodeBase(0)
	{
	}
#endif
	/*GlobObj_t(const GlobObj_t& o)
		: TypeObj0_t(o)
	{
	}*/
	GlobObj_t(GlobObj_t&& o)
		: TypeObj0_t(std::move(o))
	{
	}
	/*GlobObj_t& operator=(GlobObj_t&& o)
	{
	}*/
	~GlobObj_t()
	{
		assert(!hasPrettyName());
	}
	FolderPtr folder() const {
		return const_cast<FolderPtr>(UserData<Folder_t>());//mpFolder;
	}
	void setFolder(FolderPtr p) {
		SetUserData<>(p);//mpFolder = p;
	}
	GlobKey _key() const { return key; }
	FuncDef_t* func() const {
		return &pvt() ? pvt().typeFuncDef() : nullptr;
	}

	TypePtr ownerScope1() const {
		return owner();
	}
	void setOwnerScope(TypePtr p) {
		setOwner(p);
	}
	void setPrettyName() { m_nFlags |= GLB_PRETTY_NAME; }
	void clearPrettyName() { m_nFlags &= ~GLB_PRETTY_NAME; }
	bool hasPrettyName() const { return (m_nFlags & GLB_PRETTY_NAME) != 0; }//an entry should exist in pretty names map
	unsigned ID2() const { return pvt().ID(); }
	bool isIntrinsic() const { return (m_nFlags & FDEF_INTRINSIC) != 0; }
};

typedef GlobObj_t* GlobPtr;
typedef const GlobObj_t* CGlobPtr;

typedef TypePtr GlobToTypePtr;
typedef CTypePtr CGlobToTypePtr;


class FieldEx_t : public Field_t
{
	GlobObj_t mGlob;
public:
	FieldEx_t() {}

//	FieldEx_t(const FieldEx_t& o) : Field_t(o), mGlob(o.mGlob){}
	//FieldEx_t(FieldEx_t&& o) : Field_t(std::move(o)), mGlob(std::move(o.mGlob)){}
#if(NO_OBJ_ID)
	FieldEx_t(Field_t&& o) : Field_t(std::move(o)) {}
#else
	FieldEx_t(int id) : Field_t(id){}	
	FieldEx_t(Field_t&& o, int id)
		: Field_t(std::move(o), id),
		mGlob(id)
	{
	}
#endif

	FieldEx_t& operator=(Field_t&& o)
	{
		Field_t::operator=(o);
		return *this;
	}

	GlobObj_t& glob() { return mGlob; }
	const GlobObj_t& glob() const { return mGlob; }
	GlobPtr globPtr() const { return (GlobPtr)&mGlob; }


	static FieldEx_t* dockField(const GlobObj_t* g) { 
		return (FieldEx_t*)(((char*)g)-(size_t)(&((FieldEx_t*)0)->mGlob));
	}
	FieldEx_t* dockField() const { return dockField(&mGlob); }
};




