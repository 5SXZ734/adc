#ifndef __DUMP_OLD_H__
#define __DUMP_OLD_H__

#include <map>
#include <vector>
#include <string>
#include <strstream>
#include "dc/main.h"
#include "dc/struc.h"
#include "back/x86out.h"


struct ins_t;
class Type_t;
class Struc_t;
class Union_t;
class Array_t;
class Simple_t;
class DisplayCache_t;
class DRAW_t;
class Code_t;
class FuncEnd_t;
class Complex_t;
class Block_t;
class DUMPbin_t;
class DRAWbin_t;
class DRAWobj_t;

class DUMP_t
{
public:
	ADDR	maddr_pr;
	int		mline;

	unsigned long	milen;

	Seg_t	* mpseg;
	Field_t	* mpfunc;
	Field_t	* mpobj;
	Field_t	* mpobj_nx;

	//int		mRawOffs;
	//int		mRawSize;

	IStream_t * mis;//std::istrstream mis;
	//DisplayCache_t *	mdcache;
	DRAW_t * mpD;

public:

	DUMP_t();

	void reset( ADDR addr, int line );
	//void update();
	void update2( ADDR addr );
	void clean();
	void setPainter( DRAW_t * p )
	{
		mpD = p;
	}

	int dumpDocBegin( int &line );
	int dumpDocEnd( int &line );
	int dumpMain( ADDR addr, int line );
	void lineDown();
	void lineUp();
	int	CalcAddr();
	bool dump();

	bool closed( ADDR, int );

	Block_t * D2R( dC_t * pSelf, ADDR addr, int &myAddr, int &blkOffs );
	bool D2V( dC_t * pSelf, ADDR addr, ADDR &v );
	bool D2R( ADDR d, ADDR &addr_r, PDATA &pData );
	bool D2V( ADDR d, ADDR &addr_v );

	//virtual bool V2D( ADDR addr_v, ADDR &d );
	//virtual bool R2D( ADDR addr_v, ADDR &d );
	bool R2D( ADDR addr_r, ADDR &d );
	bool V2D( ADDR addr_v, ADDR &d );

	bool __D(Seg_t * pSelf, dC_t * pdC, ADDR_RANGE &d);
	ADDR D(Seg_t * pSelf);
	bool ADDR2D( Seg_t * pSelf, ADDR addr, ADDR &addr_d );

/*	int dumpCacheObj( ADDR addr, int &line );
	int dumpDynObj( ADDR addr, int &line );
	DisplayCache_t &dcache();*/
	int dump_TypeEx(Obj_t * pType, ADDR addr_r, int &line, int offs,
		Name_t * pName, Type_t * pParent = NULL, Obj_t * pNext = NULL );

	int dump_Type(Type_t * pSelf, ADDR addr, int &line, int offs, 
		Name_t * pName, Type_t * pParent = NULL, Obj_t * pNext = NULL );

	int dump_Simple(Simple_t * pSelf, ADDR addr_r, int &line, int offs, 
		Name_t * pName, Type_t * pParent = NULL, Obj_t * pNext = NULL );

	int dump_Array( Array_t * pSelf, ADDR addr_r, int &line, int offs, 
		Name_t * pName, Type_t * pParent = NULL, Obj_t * pNext = NULL );

	int dump_Code(Code_t * pSelf, ADDR addr, int &line, int offs, 
		Name_t * pName, Type_t * pParent = NULL, Obj_t * pNext = NULL );

	int dump_Struc(Struc_t * pSelf, ADDR addr, int &line, int offs, 
		Name_t * pName, Type_t * pParent = NULL, Obj_t * pNext = NULL );

	int dump_Union(Union_t * pSelf, ADDR addr_r, int &line, int offs, 
		Name_t * pName, Type_t * pParent = NULL, Obj_t * pNext = NULL );

	int dump_Seg(Seg_t * pSelf, ADDR addr, int &line, int offs, 
		Name_t * pName, Type_t * pParent, Obj_t * pNext );

	int dump_Func(Func_t * pSelf, ADDR addr, int &line, int offs, 
		Name_t * pName, Type_t * pParent = NULL, Obj_t * pNext = NULL);

	int dump_FuncEnd(FuncEnd_t * pSelf, ADDR addr_r, int &line, int offs, 
		Name_t * pName, Type_t * pParent = NULL, Obj_t * pNext = NULL );

	int dump_Unk(Obj_t * pSelf, ADDR addr_r, int &line, int offs, 
		Name_t * pName, Type_t * pParent = NULL, Obj_t * pNext = NULL);

protected:
	int draw_Array_closed( Array_t * pSelf, ADDR addr_r, int &line, int offs,
		Name_t * pName, Type_t * pParent, Obj_t * pNext);
};

#endif//__DUMP_OLD_H__

