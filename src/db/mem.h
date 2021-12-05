#pragma once

#include <map>
#include "config.h"

//#include "qx/MyPath.h"
#include "qx/MyString.h"
#include "shared/defs.h"
#include "shared/tree.h"

class Complex_t;
class TypeObj0_t;
class TypeObj_t;
class DataObj_t;
class Obj_t;
typedef Obj_t *	ObjPtr;
typedef const Obj_t *	CObjPtr;
typedef	TypeObj_t *	TypePtr;
typedef	TypeObj0_t *	TypeBasePtr;
typedef	const TypeObj_t *	CTypePtr;
typedef	const TypeObj0_t *	CTypeBasePtr;
class Typedef_t;
class Seg_t;
class TypeCode_t;
class Struc_t;
//class TypeUnion_t;
class Simple_t;
class Strucvar_t;
class FieldMap;
struct Cmd_t;
enum AttrIdEnum : unsigned int;
class Debugger_t;

/*enum GoToAttr_t {
	GOTO_UNDEFINED,
	GOTO_SHORT,
	GOTO_NEAR,
	GOTO_FAR
	};*/


/*union AuxReg_t
{
	uint8_t	t8;
	struct { uint8_t t8l, t8h; };
	uint16_t	t16;
	struct { uint16_t t16l, t16h; };
	uint32_t	t32;
	struct { uint32_t t32l, t32h; };
	uint64_t	t64;
};*/

//inline bool IS _PTR(void * p) { return (((*(uint32_t*)&p)>>8)!=0); }


////////////////////////

#define STDERR	stdout
#define BAD_ADDR	0

/*enum FTYP_t {
	FTYP_ASM,
	FTYP_DC,
	FTYP_OUT2
};*/

//#define	SHOW_ALL_BRACES 0


/*#ifdef WIN32
#undef assert
void __assert(void *, unsigned);
#ifdef _DEBUG
#define assert(exp) (void)( (exp) || (__assert(__FILE__, __LINE__), 0) )
#else
#define assert(exp) ;
#endif
#endif*/



//template <typename, typename> class TreeNode_t;
class Field_t;

typedef Field_t* FieldPtr;
typedef Field_t& FieldRef;
typedef const Field_t* CFieldPtr;
typedef const Field_t& CFieldRef;

class Folder_t;

class Obj_t;
//class Folder0_t;
//typedef TreeNode_t<Folder0_t>	Folder_t;
typedef Folder_t*	FolderPtr;
typedef const Folder_t*	CFolderPtr;
typedef Folder_t& FolderRef;
typedef const Folder_t& CFolderRef;

class TypeProc_t;
class TypeObj_t;
//typedef const TypeObj_t* CTypePtr;
//typedef const TypeObj_t& CTypeRef;
class NameRef_t;
typedef NameRef_t* NameRefPtr;
class NamesMgr_t;
class TypesMgr_t;

#include "qx/MyMemoryPool.h"

template <typename T>
class TPoolWrapper
{
public:
	typedef TMemoryPool<T>	MyPool;
	MyPool* mp;
public:
	TPoolWrapper(unsigned chunkSize = 0)
	{
		mp = new MyPool(chunkSize);
	}
	~TPoolWrapper()
	{
		delete mp;
	}
	MyPool &pool(){ return *mp; }
	T* New()
	{
#if(NEW_MEMMGR)
		T* p(mp->allocate());
		new(p) T();
		return p;
#else
		return new T();
#endif
	}
	
#if(NO_OBJ_ID)
	template <typename U>
	T* New1(U&& o)
	{
#if(NEW_MEMMGR)
		T* p(mp->allocate());
		new(p) T(std::move(o));
		return p;
#else
		return new T(std::move(o));
#endif
	}
#else
	template <typename U>
	T* New1(U&& o, int id)
	{
#if(NEW_MEMMGR)
		T* p(mp->allocate());
		new(p) T(std::move(o), id);
		return p;
#else
		return new T(std::move(o));
#endif
	}
#endif


	template <typename U>
	T* New(U arg)
	{
#if(NEW_MEMMGR)
		T* p(mp->allocate());
		new(p) T(arg);
		return p;
#else
		return new T(arg);
#endif
	}
	/*template <typename U, typename V>
	T* New(U arg1, V arg2)
	{
#if(NEW_MEMMGR)
		T* p(mp->allocate());
		new(p) T(arg1, arg2);
		return p;
#else
		return new T(arg);
#endif
	}*/
	void Delete(T *p)
	{
#if(NEW_MEMMGR)
		if (p)
		{
			p->~T();
#if(_DEBUG)
			//memset(((char *)p)+4, 0, sizeof(T) - 4);
			memset(p, 0xCC, sizeof(T));
#endif
			mp->deallocate(p);
		}
#else
		delete p;
#endif
	}
	void Reset()
	{
		mp->reset();
	}
};

class TypePtr_t;
class Array_t;
class Type_t;
class I_DataSource;

class MemoryMgr_t
{
public:
	TPoolWrapper<Folder_t> mFiles;
	TPoolWrapper<TypeObj_t> mTypeRefs;
	TPoolWrapper<NameRef_t> mNameRefs;
	TPoolWrapper<Field_t> mFields;
	//TPoolWrapper<TypesMgr_t> mTypeMaps;
	TPoolWrapper<DataObj_t> mDataObjs;

public:
#ifdef _DEBUG
	MyString mName;
#endif

public:
	MemoryMgr_t();
	virtual ~MemoryMgr_t();

	//template <typename T> T* New();
	//template <typename T> void Delete(T *);

	FieldPtr NewFieldNoId();
#if(!NO_OBJ_ID)
	FieldPtr NewFieldId(int);
	TypeObj_t* NewTypeRefId(int);//id
#endif

	FieldPtr NewField(Field_t&&);
	FieldPtr NewField(uint32_t);//key + auto id
	TypeObj_t* NewTypeRef();//auto id
	TypeObj_t* NewTypeRef(Type_t *);
	TypeObj_t* NewTypeRefNoId();
	TypeObj_t* NewTypeRefNoId(Type_t *);
	NameRef_t* NewNameRef(const char *);
	NameRef_t* NewNameRef();
	//TypesMgr_t*	NewTypesMap();
	DataObj_t*	NewDataObj();

	//global
	FolderPtr	NewFile(MyString);

	void Delete(FieldPtr);
	void Delete(TypeObj_t *);
	void Delete(NameRef_t *);
	//void Delete(TypesMgr_t *);
	void Delete(DataObj_t *);

	//global
	void Delete(FolderPtr);

	void print_stats(std::ostream &os);
};


