#pragma once

#include <iostream>
#include <assert.h>
#include "shared/link.h"
#include "shared/defs.h"
#include "shared/obj_id.h"
//#include "names.h"

class Field_t;
class TypeProc_t;
class Struc_t;
class Seg_t;
class TypeObj_t;
class NamesMgr_t;
class TypesMgr_t;
class GlobObj_t;
class TypeObj_t;

typedef uint32_t	ObjFlagsType;

#define OBJ_ID_ZERO	(int)0

class IdCheck_t
{
	int		mID;
public:
	int		ID() const { return mID; }
	//void	SetID(int id){ mID = id; }
	bool	checkId(int id) const {
		return (id == mID);
	}
	IdCheck_t & operator=(const IdCheck_t& o){
		mID = o.mID;
		return *this;
	}
	static int	sID;
	static void resetUniqueId(){ sID = 0; }
	static int idNext(){
		return ++sID; }

	IdCheck_t() : mID(0){}
	IdCheck_t(int);
	IdCheck_t(const IdCheck_t &o)
		: mID(o.mID)
	{
	}
	IdCheck_t(IdCheck_t&& o) { mID = o.mID; }
	~IdCheck_t();
	void setId(){ mID = idNext(); }
};

struct ObjFlags_t
{
	ObjFlagsType	m_nFlags;
	ObjFlags_t() : m_nFlags(0){}
	ObjFlags_t(ObjFlagsType f) : m_nFlags(f){}
	ObjFlags_t(const ObjFlags_t &o)
		: m_nFlags(o.m_nFlags)
	{
	}
	ObjFlags_t(ObjFlags_t&& o) { m_nFlags = o.m_nFlags; }
	unsigned objId() const { return m_nFlags & OBJ_TYPE; }
	ObjFlagsType &flags(){ return m_nFlags; }
	const ObjFlagsType &flags() const { return m_nFlags; }
	ObjFlags_t & operator=(const ObjFlags_t& o){
		m_nFlags = o.m_nFlags;
		return *this;
	}
};

#if(!NO_OBJ_ID)
class Obj_t : public IdCheck_t,
#else
class Obj_t : 
#endif
	public ObjFlags_t
{

#ifdef _DEBUG
	void on_new_obj();
	void on_kill_obj();
#endif

public:

	Obj_t(ObjFlagsType f) : ObjFlags_t(f)
	{
		assert(f <= OBJID_GLOBOBJ);
	}

#if(!NO_OBJ_ID)
	Obj_t(ObjFlagsType f, int id)
		: IdCheck_t(id),
		ObjFlags_t(f)
	{
		assert(f <= OBJID_GLOBOBJ);
#ifdef _DEBUG
		on_new_obj();
#endif
	}
#endif

/*	Obj_t(const Obj_t& o) :
#if(!NO_OBJ_ID)
		IdCheck_t(o),
#endif
		ObjFlags_t(o)
	{
#ifdef _DEBUG
		on_new_obj();
#endif
	}*/


#if(!NO_OBJ_ID)
	Obj_t(Obj_t&& o, int id) : IdCheck_t(id),
#else
	Obj_t(Obj_t&& o) :
#endif
		ObjFlags_t(std::move(o))
	{
#ifdef _DEBUG
		on_new_obj();
#endif
	}


	~Obj_t()
	{
#ifdef _DEBUG
		on_kill_obj();
#endif
	}

	Obj_t& operator=(const Obj_t& o) { assert(0); return *this; }

	//virtual int Obj Type() const { assert(0); return OBJID_NULL; }
	const Obj_t &asObj() const { return *this; }
	Obj_t &asObj(){ return const_cast<Obj_t &>(*this); }
	//ObjFlagsType flags() const { return m_nFlags; }

	TypeObj_t *objType() const {
		if (objId() == OBJID_TYPEOBJ)
			return (TypeObj_t *)this;
		return nullptr;
	}

	TypeObj_t *objTypeGlob() const {
		if (objId() == OBJID_TYPEOBJ)
			return (TypeObj_t *)this;
		if (objId() == OBJID_GLOBOBJ)
			return (TypeObj_t *)this;//derivative?
		return nullptr;
	}
	GlobObj_t *objGlob() const {
		if (objId() == OBJID_GLOBOBJ)
			return (GlobObj_t *)this;//derivative!
		return nullptr;
	}
	Field_t *objField() const {
		return objId() == OBJID_FIELD ? (Field_t *)this : nullptr;
	}

	//bool	CompName(const char * pName);//compare with named objects only
	//bool	CompNamex(const char * pName);//compare with autonames as well
	bool	ValidName(const char * pName);//check validity of name

	
	///////////////////////////////

	//int		MoveUp();
	//int		MoveDown();
		
};


void print_obj_stats();








