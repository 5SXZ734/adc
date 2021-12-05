#include <assert.h>

#include "shared/defs.h"
#include "shared/misc.h"
#include "shared/front.h"
#include "shared/instr.h"
#include "x86out.h"
#include "front/x86/x86dasm.h"
#include "dc/type.h"
#include "dc/func.h"
#include "dc/segs.h"
#include "dc/names.h"
#include "dc/field.h"
#include "dc/block.h"
#include "dc/dc.h"
#include "dc/main.h"
#include "dump_old.h"
//#include "dasmout.h"

DUMP_t::DUMP_t()
	: mis(NULL),// (const char *)NULL ),
	//mdcache(NULL)
	mpD( NULL )
{
}

void DUMP_t::reset( ADDR addr, int line )
{
	clean();
	maddr_pr = addr;
	mline = 0;

	/*if (line == 0)
	{
		dump();
		return;
	}*/

	while ( mline < line )
	{
		dump();
	}
}

void DUMP_t::lineDown()
{
	ADDR addr = maddr_pr;//?+milen;//?maddr0;
	int line = mline;
	bool bCheck = false;
	while ( 1 )
	{
		if ( dumpMain( addr, line ) )
		{
			if ( bCheck )
				return;
			line++;
			//?maddr0 = addr;
			mline = line;
			bCheck = true;
			continue;
		}

		if ( addr >= MAIN.getSize()-1 )
			break;

		mline = 0;
		maddr_pr += milen;//?maddr0 = maddr_pr + milen;
		milen = 0;
		break;
	}
}

void DUMP_t::lineUp()
{
	ADDR addr0 = maddr_pr;//?+milen;//?maddr0;
	int line0 = mline;

	ADDR addr = addr0;
	int line = line0;

	bool bCheck = false;

	if ( line > 0 )
		line--;
	else
		addr--;	
	
	while ( 1 )
	{
		if ( addr >= addr0 && line >= line0 )
			break;

		if ( dumpMain( addr, line ) )
		{
//?			maddr0 = maddr_pr;
			mline = line;

			addr = maddr_pr;//maddr0;
			line++;
			continue;
		}

		if ( addr >= MAIN.getSize()-1 )
			break;

		line = 0;
		addr = maddr_pr + milen;
		milen = 0;
	}
}

int DUMP_t::CalcAddr()
{
	int line = 0;
	while (1)
	{
		if ( dumpMain( maddr_pr/*maddr0*/, line ) )
		{
			line++;
			continue;
		}

		if ( maddr_pr/*?maddr0*/ >= MAIN.getSize()-1 )
		{
			//bDrawLine = false;
			break;
		}

		//?maddr0 += milen;
		maddr_pr += milen;
		line = 0;
		break;
	}
	return 1;
}

bool DUMP_t::dump()
{
	if (!MAIN.struc())
	{
		mline++;
		return false;
	}

	while (1)
	{
		/*if ( maddr0 == MAIN.getSize()-1 )
		{
			bDrawLine = false;
			break;
		}*/

		if ( dumpMain( maddr_pr, mline ) )
		{
			mline++;
			break;
		}

		if ( maddr_pr >= MAIN.getSize()-1 )
		{
			return false;
		}

		maddr_pr += milen;
		milen = 0;
		mline = 0;
	}

	DUMP_t& D = *this;
	DRAW( drawRowID(0) );
	return true;
}

void DUMP_t::update2( ADDR addr )
{
	maddr_pr = addr;
	ADDR addr_r = maddr_pr;
	mpseg = NULL;
	mpfunc = NULL;
	mpobj = NULL;
	Struc_t * pStruc = MAIN.seg();
	if (pStruc)
		mpobj = pStruc->GetFieldEx(0);//global();
	return;	
}

void DUMP_t::clean()
{
	maddr_pr/*? = maddr0*/ = 0;
	mline = 0; milen = 0;
	mpseg = NULL;
	mpfunc = NULL;
	mpobj = mpobj_nx = NULL;
//	mRawOffs = 0;
//	mRawSize = 0;
}

#define CHECKDUMP(t,f)	if (pType->t()) \
	return f(pType->t(), addr, line, offs, pName, pParent, pNext);


int DUMP_t::dump_TypeEx( Obj_t * pSelf, ADDR addr, int &line, int offs,  
						  Name_t * pName, Type_t * pParent, Obj_t * pNext )
{
	Type_t * pType = NULL;

	if (pSelf)
	{
		pType = pSelf->objType();

		Custom_T * pDynType = pType->typeCustom();
		if ( pDynType != NULL )
		{
assert(0);
//?			return dump_TypeEx( pDynType->custom( addr ), addr, line, offs,
//?				pName, pParent, pNext );
		}
	}

//	if ( offs > pType->size() )
//		return 0;
	DUMP_t& D = *this;

	DRAW( incLevel() );
	if ( pNext != NULL )
		DRAW( setTreeInfo(0/*ITEM_HAS_SIBLINGS*/) );

	if (!pType)
	{
/*		if ( line == 0 )
		{
			if ( pParent != NULL )
				DRAW( setTreeInfo( ITEM_HAS_PARENT ) );
			DRAW( drawName( nameID, NS ) );
		}*/
		return dump_Unk(pSelf, addr, line, offs, pName, pParent, pNext);
	}

	CHECKDUMP(typedC,		dump_Seg);		//Struc_t
	CHECKDUMP(typeSeg,		dump_Seg);		//Struc_t
	CHECKDUMP(typeFunc,		dump_Func);		//Struc_t
	CHECKDUMP(typeStruc,	dump_Struc);	//Complex_t
	CHECKDUMP(typeUnion,	dump_Union);	//Complex_t
	CHECKDUMP(typePtr,		dump_Simple);	//Simple_t
	CHECKDUMP(typeArray,	dump_Array);	//Type_t
	CHECKDUMP(typeSimple,	dump_Simple);	//Type_t
	CHECKDUMP(typeComplex,	dump_Type);		//Type_t
	CHECKDUMP(typeFuncEnd,	dump_FuncEnd);	//Type_t
	CHECKDUMP(typeCode,		dump_Code);		//Type_t
	CHECKDUMP(typeCustom,	dump_Type);	//Type_t

	return dump_Type(pType, addr, line, offs, pName, pParent, pNext);
}

int DUMP_t::dump_Type( Type_t * pSelf, ADDR addr, int &line, int offs, 
				  Name_t * pName, Type_t * pParent, Obj_t * pNext )
{
	if (line == 0)
	{
		//if (offs == 0)
		{
			if ( pParent != NULL )
				DRAW( setTreeInfo( ITEM_HAS_PARENT ) );
			DRAW( drawName( mpobj, pParent?pParent->typeComplex():NULL ) );
			DRAW( drawUnknown( addr+offs ) );
			milen = OPSZ_BYTE;
			return 1;
		}
	}

	line -= 1;
	return 0;
}

int DUMP_t::draw_Array_closed( Array_t * pSelf, ADDR addr_r, int &line, int offs,
				Name_t * pName, Type_t * pParent, Obj_t * pNext )
{
	Type_t * pType = pSelf->baseType();
	Simple_t * pBaseType = pType->typeSimple();

	if (1)
	if ( offs == 0 )
	{
		switch( line )
		{
		case 0:
			if ( pParent != NULL )
				DRAW( setTreeInfo( ITEM_HAS_PARENT ) );
//			if ( hasChildren() )
				DRAW( setTreeInfo( 0/*ITEM_HAS_CHILDREN|ITEM_CLOSED*/ ) );
			DRAW( drawDataArrayOpen( pSelf, pName ) );
			break;
		default:
			line -= 1;
			return 0;
		}
	}

	if ( line != 0 )
	{
//		line -= 1;
		return 0;
	}

	int num = 1;
	switch ( pBaseType->size() ) {
	case OPSZ_BYTE: num = 12; break;
	case OPSZ_WORD: num = 8; break;
	case OPSZ_DWORD: num = 5; break;	
	case OPSZ_QWORD: num = 3; break;
	}

	int num_row = num * pBaseType->size();
	int rows = offs / num_row;
	int offs0 = rows * num_row;

	int num_max = (pSelf->size() - offs0 ) / pBaseType->size();
	if ( num_max < num )
		num = num_max;

	//maddr_pr = addr_r + offs0;

	DRAW( drawType( addr_r + offs0, pBaseType, num ) );
	int delta = offs0 - offs;
	if ( offs != 0 )
		maddr_pr += delta;
	milen = pBaseType->size() * num;
	return 1;
}

int DUMP_t::dump_Array( Array_t * pSelf, ADDR addr_r, int &line, int offs,
				Name_t * pName, Type_t * pParent, Obj_t * pNext )
{
	Type_t * pType = pSelf->baseType();
	Simple_t * pBaseType = pType->typeSimple();

	bool bClosed = pBaseType != NULL;//closed( addr_r, mlevel );
	if ( bClosed )
		return draw_Array_closed( pSelf, addr_r, line, offs, pName, pParent, pNext );

	if (1)
	if ( offs == 0 )
	{
		switch( line )
		{
		case 0:
			if ( pParent != NULL )
				DRAW( setTreeInfo( 0/*ITEM_HAS_PARENT*/ ) );
//			if ( hasChildren() )
				DRAW( setTreeInfo( 0/*ITEM_HAS_CHILDREN*/ ) );
			//if ( closed( addr_r, mlevel ) )
			if ( bClosed )
				DRAW( setTreeInfo( ITEM_CLOSED ) );
			DRAW( drawDataArrayOpen( pSelf, pName ) );
			return 1;
		default:
			line -= 1;
			break;
		}
	}

	int sz0 = pType->size();
/*		if ( sz0 < 0 )
	{
		ADDR addr = addr_r;
		for ( int i = 0; i < pArray->total(); i++ )
		{
			const char * pBuf = MAIN.getRawData( addr );
			if ( pBuf == NULL )
				return 0;

			Type_t * pTypeRT = pType->typeRT( (void *)pBuf );
			if ( pTypeRT == NULL )
				return NULL;

			int sz2 = pTypeRT->size();
			int offs2 = addr - addr_r;
			if ( offs2 <= offs && offs2 + sz2 > offs )
			{
				int ret = dump_TypeEx( 
					pTypeRT,
					addr, 
					line,
					offs - offs2,
					NULL,
					pArray,
					(i < pArray->total()-1)?pTypeRT:NULL
				);

				if ( ret != 0 )
					return 1;

				return 0;
			}
			addr += sz2;
		}

		return 0;
	}*/

	int num = offs / sz0;
	int offs0 = num * sz0;

	int ret = dump_TypeEx( 
		pType,
		addr_r + offs0, 
		line, 
		offs - offs0,// - pField->getOffset(), 
		fmt("[%d]", num),
		pSelf,
		(num < pSelf->total()-1)?pType:NULL
		);

	if ( ret != 0 )
		return 1;

	return 0;
}

int DUMP_t::dump_Union( Union_t * pSelf, ADDR addr_r, int &line, int offs, 
				Name_t * pName, Type_t * pParent, Obj_t * pNext )
{
	if ( offs != 0 )
		maddr_pr -= offs;

	if ( line == 0 )
	{
		if ( pParent != NULL )
			DRAW( setTreeInfo( 0/*ITEM_HAS_PARENT*/ ) );
//		if ( hasChildren() )
			DRAW( setTreeInfo( 0/*ITEM_HAS_CHILDREN*/ ) );
		DRAW( drawDataStrucOpen( pSelf, pName ) );
		return 1;
	}

	line--;
	Field_t * f_nx = NULL;
	Field_t * f = pSelf->getFieldByIndex( line, &f_nx );
	if ( f == NULL )
	{
		int fields_num = pSelf->fieldsCount();
/*		if ( line == fields_num )
		{
			milen = size();
			print( "UNION_END" );
			return 1;
		}*/
		line -= fields_num;
		return 0;
	}

	mpobj = f;
	mpobj_nx = f_nx;

	line -= line;
	int ret = dump_TypeEx( 
			f->type(),
			addr_r, 
			line, 
			0,
			f->name(),
			pSelf,
			f_nx?f_nx:NULL
			);

	return ret;
}

int DUMP_t::dump_Simple( Simple_t * pSelf, ADDR addr_r, int &line, int offs,
				Name_t * pName, Type_t * pParent, Obj_t * pNext )
{
	if ( line == 0 )
	{
		if ( pParent != NULL )
			DRAW( setTreeInfo( ITEM_HAS_PARENT ) );
		DRAW( drawName( mpobj, pParent?pParent->typeComplex():NULL ) );
		int num = 1;
		DRAW( drawType( addr_r, pSelf, num ) );
		if ( offs != 0 )
			maddr_pr -= offs;
		milen = pSelf->size() * num;
		return 1;
	}
	else
	{
		line -= 1;
	}

	return 0;
}

int DUMP_t::dump_Struc( Struc_t * pSelf, ADDR addr_r, int &line, int offs, 
				  Name_t * pName, Type_t * pParent, Obj_t * pNext )
{
	if (1)
	if ( offs == 0 )
	{
		switch( line )
		{
		case 0:
			if ( pParent != NULL )
				DRAW( setTreeInfo( 0/*ITEM_HAS_PARENT*/ ) );
			if ( pSelf->hasChildren() )
				DRAW( setTreeInfo( 0/*ITEM_HAS_CHILDREN*/ ) );
			DRAW( drawDataStrucOpen( pSelf, pName ) );
			return 1;
		default:
			line -= 1;
			break;
		}
	}

	ADDR offs2 = offs + pSelf->base();
	mapFIELD_it it_nx = NULL;
	mapFIELD_it it = pSelf->field_it( offs2, &it_nx );

	Field_t * f = NULL;
	ADDR fa = offs2;//-1;
	if ( it != pSelf->fields().end() )
	{
		fa = it->first;
		f = it->second;
	}

	Field_t * f_nx = NULL;
	ADDR fa_nx = -1;
	if ( it_nx != pSelf->fields().end() )
	{
		fa_nx = it_nx->first;
		f_nx = it_nx->second;
	}

	if (f)
	{
		int f_sz = f->size();
		if ( f_sz >= 0 )
		{
			if ( f_sz == 0 )
				f_sz = OPSZ_BYTE;
			if ( fa + f_sz <= offs2 )
				f = NULL;
		}
	}

	mpobj = f;
	mpobj_nx = f_nx;

	int offs_f = fa;//offs2;
	int offs_d = offs_f - pSelf->base();

	Type_t * pType = f?f->type():NULL;
	Name_t * f_name = f?f->name():NULL;

	int ret = dump_TypeEx( 
		pType,
		addr_r + offs_d, 
		line,
		offs2-offs_f,
		f_name,
		pSelf,
		f_nx?f_nx:NULL);

	if ( ret != 0 )
		return ret;

	if (pSelf->typeFunc())
	if (offs + milen == pSelf->size())
	{
		if (1)
		switch( line )
		{
		case 0:
		//case 1:
			/*if (0)
			if ( f_nx != NULL && f_nx->type() != NULL )
			{
				FuncEnd_t * t = f_nx->type()->typeFuncEnd();
				if ( t != NULL )
				{
					//return dump_TypeEx( 
					return dump_ TypeEx( t,
						addr_r,
						line,
						offs2-addr_r,
						//f_nx->type(),
						NULL,
						pSelf,
						NULL);
				}
			}*/
			//D.drawDataStrucClose( this, nameID, NS );
			DRAW( print("") );
			return 1;
		default:
			line -= 1;
			break;
		}
	}

	return 0;
}

int DUMP_t::dump_Seg( Seg_t * pSelf, ADDR addr_r, int &line, int offs, 
				Name_t * pName, Type_t * pParent, Obj_t * pNext )
{
	mpseg = pSelf;
	int ret = dump_Struc( pSelf, pSelf->base(), line, offs, pName, pParent, pNext );

	if (ret != 0)
	{
		if (!pSelf->typedC())//GetOwnerSeg())
		{
			DUMP_t& D = *this;

			char buf[10];
			sprintf(buf, "%08X", pSelf->base() + offs);
			DRAW(mColumnMgr.AppendText(CLMN_ADDR_V, buf));
		}
	}

	return ret;
}

int DUMP_t::dump_Code( Code_t * pSelf, ADDR addr0, int &line, int offs, 
				 Name_t * pName, Type_t * pParent, Obj_t * pNext )
{
	ADDR addr = addr0+offs;

	assert( addr >= mpobj->offset() );

	if ( offs == 0 )
	{
		switch ( line )
		{
		case 0:
			if ( pParent != NULL )
				DRAW( setTreeInfo( ITEM_HAS_PARENT ) );
			DRAW( drawName( mpobj, pParent?pParent->typeComplex():NULL ) );
			return 1;
		default:
			line -= 1;
			break;
		}
	}

	if ( line == 0 )
	{
		ADDR addr_pr;
		{
			ADDR addri = addr0;
			ADDR addr = addri;

			IStrStream0_t mis( mpseg->findBlock(addr) );
			IStream_t& is = mis;

			ADDR addr_nx = -1;

			Field_t * pObjNx = mpobj_nx;
			if ( pObjNx )
			{
				int d = pObjNx->offset()-mpobj->offset();
				addr_nx = addri + d;
			}
			else if ( mpfunc )
			{
				Struc_t * st = mpfunc->type()->typeStruc();
				int d = st->base() + st->size() - mpobj->offset();
				addr_nx = addri + d;
			}

			while (1)
			{
				ins_t ins;
				if ( !is.seekg( addr ) )
					break;
				milen = pSelf->unassemble(is, addr, ins);
				if ( milen == 0 )
					break;

				if (addr + milen > addr_nx)
					break;

				if (addr <= addri+offs && addr + milen > addri+offs)
				{
					DRAW(drawCode( ins ));
					return addr;
				}
				
				addr += milen;

				if (ins.b_break)
					break;
			}

			int line = 0;
			addri += offs;
			dump_Unk(NULL, addri, line, 0, NULL, pParent, pNext);
			addr_pr = addri;
		}

		int delta = addr_pr - addr;
		if ( delta != 0 )
			maddr_pr += delta;
		return 1;
	}
	
	line -= 1;
	return 0;
}

int DUMP_t::dump_Func( Func_t * pSelf, ADDR addr, int &line, int offs, 
				 Name_t * pName, Type_t * pParent, Obj_t * pNext )
{
	mpfunc = mpobj;
	return dump_Struc( pSelf, addr, line, offs, pName, pParent, pNext );
}

int DUMP_t::dump_FuncEnd( FuncEnd_t * pSelf, ADDR addr_r, int &line, int offs, 
		Name_t * pName, Type_t * pParent, Obj_t * pNext )
{
	switch ( line )
	{
	case 0:
		if ( pParent != NULL )
			DRAW( setTreeInfo( 0/*ITEM_HAS_PARENT*/ ) );
		DRAW( clearTreeInfo( 0/*ITEM_HAS_SIBLINGS*/ ) );
		DRAW( drawFuncEnd( line ) );
		return 1;
	case 1:
		DRAW( clearTreeInfo( 0/*ITEM_HAS_SIBLINGS*/ ) );
		DRAW( drawFuncEnd( line ) );
		return 1;
	default:
		line -= 2;
		break;
	}

	return 0;
}

int DUMP_t::dump_Unk(Obj_t * pSelf, ADDR addr, int &line, int offs, 
		Name_t * pName, Type_t * pParent, Obj_t * pNext)
{
	if ( line == 0 )
	{
		DUMP_t& D = *this;

		/*if (offs == 0)
		{
			if ( pParent != NULL )
				DRAW( setTreeInfo( ITEM_HAS_PARENT ) );
			DRAW( drawName( mpobj, pParent?pParent->typeComplex():NULL ) );
			DRAW( drawUnknown( addr+offs ) );
		}*/
		DRAW( drawUnknown(addr+offs) );
		milen = OPSZ_BYTE;
		return 1;
	}
	else
	{
		line -= 1;
	}
	return 0;
}

int DUMP_t::dumpDocBegin( int &line )
{
	DUMP_t& D = *this;
	switch( line )
	{
	case 0:
		DRAW( print("DOC_BEGIN,LINE:0") );
		return 1;
	case 1:
		DRAW( print("DOC_BEGIN,LINE:1") );
		return 1;
	default:
		line -= 2;
	}
	return 0;
}

int DUMP_t::dumpDocEnd( int &line )
{
	DUMP_t& D = *this;
	switch( line )
	{
	case 0:
		DRAW( print("DOC_END,LINE:0") );
		return 1;
	case 1:
		DRAW( print("DOC_END,LINE:1") );
		return 1;
	}
	return 0;
}

int DUMP_t::dumpMain( ADDR addr, int line )
{
	DUMP_t& D = *this;
	DRAW( update2( addr ) );

	update2( addr );

/*	if ( addr == 0 )
	{
		if ( drawDocBegin( line ) )
			return 1;
	}*/

	if ( mpobj )
	{
		int offs = addr - mpobj->offset();
		addr = mpobj->offset();
		Type_t * pType = mpobj->type();
		return dump_TypeEx( pType, addr, line, offs, NULL );
	}

	int ret = dump_Unk(NULL, addr, line, 0, NULL);
	if ( ret != 0 )
		return ret;

/*	if ( addr_r >= MAIN.getSize()-1 )
	{
		if ( drawDocEnd( line ) )
			return 1;
	}*/

	return 0;
}

#if(1)
Block_t * DUMP_t::D2R( dC_t * pSelf, ADDR _addrD, int &myAddr, int &blkOffs )
{
	ADDR addr = _addrD;

	for (mapSEG_it it = pSelf->mSegs.begin(); it != pSelf->mSegs.end(); ++it)
	{
		Seg_t * pSeg = it->second;

		ADDR segOffs = pSeg->Offset();
		if (addr < segOffs)
			break;

		ADDR_RANGE sz = pSeg->sizex();
		ADDR_RANGE szP = pSeg->sizep();

		if (addr - segOffs < sz)
		{
			myAddr = addr;
			return pSeg->findBlock0(pSeg->base() + addr - segOffs, blkOffs);
		}

		int d = sz - szP;
		if (d > 0)
			addr -= d;
	}

	myAddr = addr;
	return pSelf->findBlock0(addr, blkOffs);
}
#else
Block_t * DUMP_t::D2R( Seg_t * pSelf, ADDR _addrD, int &myAddr, int &blkOffs )
{
	int addrD = _addrD;

	int size0 = 0;

	mapSEG_it it;
	for ( it = pSelf->mSegs.begin(); it != pSelf->mSegs.end(); ++it ) 
	{
		Seg_t * pSeg = it->second;
		int offs = it->first;

		int d = size0-offs;
		if ( d > 0 )
		{
			addrD -= d;
			size0 -= d;
		}

		if ( addrD < offs )
		{
			myAddr = addrD;
			blkOffs = myAddr;
			return pSelf->findBlockOffs(myAddr);
		}

		int size = pSeg->size();
		d = addrD - (offs + size);
		if (d < 0)
		{
			myAddr = addrD;
			return pSeg->findBlock0(pSeg->base() + myAddr - offs, blkOffs);
/*			int rsize = pSeg->RawDataOffset() + pSeg->RawDataSize();
			if ( addrD < offs + rsize )
			{
				myAddr = addrD;
				return pSeg->findBlock(myAddr);
			}
			return NULL;*/
		}

		if ( offs > size0 )
		{
			size0 = offs;
		}

		size0 += size;
	}

	if ( addrD >= (int)pSelf->mSize )
		return NULL;

	myAddr = addrD;
	blkOffs = myAddr;
	return pSelf->findBlockOffs(myAddr);
}
#endif

bool DUMP_t::D2R( ADDR addrD, ADDR &r, PDATA &pData )
{
//	Block_t * pBlock = dc()->findBlock(addrD);
//	if (!pBlock)
//		return false;

	Struc_t * pStruc =  MAIN.struc();
	if (!pStruc)
		return false;

	dC_t * pdC = pStruc->typedC();
	if (pdC)
	{
		int addr;
		int offs;
		Block_t * pBlock = D2R(pdC, addrD, addr, offs);
		if (pBlock)
		{
			r = addr;
			pData = pBlock->data(offs);
			return true;
			//r = pBlock->address() + offs;
		}
		return false;
	}

	r = addrD; 
	return 1;
}

bool DUMP_t::D2V( dC_t * pSelf, ADDR addr, ADDR &v )
{
	int size0 = 0;

	mapSEG_it it;
	for ( it = pSelf->mSegs.begin(); it != pSelf->mSegs.end(); ++it ) 
	{
		Seg_t * pSeg = it->second;
		int offs = it->first;

		if ( offs < size0 )
		{
			int d = size0-offs;
			addr -= d;
			size0 -= d;
		}

		if ( (int)addr < offs )
		{
			v = addr;
			return false;
		}

		int size = pSeg->size();
		if ( (int)addr < offs+size )
		{
			v = pSeg->mBase + (addr-offs);
			return true;
		}

		if ( offs > size0 )
		{
			size0 = offs;
		}

		size0 += size;
	}

	return false;
}

bool DUMP_t::D2V( ADDR addrD, ADDR &v )
{
	dC_t * pdC = MAIN.struc()->typedC();
	if (pdC)
	{
		return D2V( pdC, addrD, v );
	}

	v = addrD;
	return 1;
}

/*bool Seg_t::V2D( ADDR addr_v, ADDR &addr_d )
{
	int delta = 0;

	mapSEG_it it;
	for ( it = mSegs.begin(); it != mSegs.end(); ++it ) 
	{
		Seg_t * pSeg = it->second;
		int d = addr_v - pSeg->mBase;
		if ( d < 0 )
			return false;

		if ( d < (int)pSeg->mSize )
		{
			addr_d = pSeg->RawDataOffset() + d + delta;
			return true;
		}

		int size0 = pSeg->size();
		d = size0 - pSeg->RawDataSize();
		if ( d > 0 )
			delta += d;
	}

	return false;
}*/

/*bool Seg_t::R2D( ADDR addr_r, ADDR &addr_d )
{
	int delta = 0;

	mapSEG_it it;
	for ( it = mSegs.begin(); it != mSegs.end(); ++it ) 
	{
		Seg_t * pSeg = it->second;
		int roffs = pSeg->RawDataOffset();
		int rsize = pSeg->RawDataSize();

		int d = addr_r - roffs;
		if ( d < 0 )
			return false;

		if ( d < rsize )
		{
			addr_d = roffs + d + delta;
			return true;
		}

		int size0 = pSeg->size();
		d = size0 - rsize;
		if ( d > 0 )
			delta += d;
	}

	return false;
}*/

bool DUMP_t::R2D( ADDR addr_r, ADDR &d )
{
	if (!GDC.contains(addr_r))
		return false;

	return ADDR2D(&GDC, addr_r, d);
	//return struc()->R2D( addr_r, d );
//	if ( !mpRangeMapper )
//		return false;
//	return mpRangeMapper->R2D( addr_r, d ) != 0;
}

bool DUMP_t::V2D( ADDR addr_v, ADDR &d )
{
	Seg_t * pSeg = GDC.findSegment(addr_v);
	if (!pSeg)
		return false;

	return ADDR2D(pSeg, addr_v, d);
	//?return struc()->V2D( addr_v, d );
/*	if ( !mpRangeMapper )
		return false;
	return mpRangeMapper->V2D( addr_v, d ) != 0;*/
}

bool DUMP_t::__D(Seg_t * pSelf, dC_t * pdC, ADDR_RANGE& d)
{
	if (pdC == pSelf)
		return true;

	mapSEG_it it = pdC->mSegs.begin();
	if (it != pdC->mSegs.end())
	{
		d += it->first;

		do {
			Seg_t * pSeg = it->second;
			if (pSeg == pSelf)
				return true;
			//if (__D(pSelf, pSeg, d))
			//	return true;

			ADDR offs =  pSeg->Offset();
			ADDR_RANGE sz = pSeg->size();

			++it;

			Seg_t * pSegNx = NULL;
			if (it != pdC->mSegs.end())
				pSegNx = it->second;

			ADDR offsNx = (pSegNx)?(pSegNx->Offset()):(pdC->size());

			d += __max(sz, offsNx-offs);

		} while (it != pdC->mSegs.end());
	}

	return false;
}

ADDR DUMP_t::D(Seg_t * pSelf)
{
	ADDR d = 0;
	if (!__D(pSelf, &GDC, d))
	{
		assert(0);
	}

//	Field_t * pField = parent()->objField();
//	d += pField->offset();

	return d;

	/*for ( mapSEG_it it = GDC.mSegs.begin(); it != GDC.mSegs.end(); ++it ) 
	{
		Seg_t * pSeg = it->second;
		if (pSeg == this)
			return d;

		int d = addr_v - pSeg->mBase;
		if ( d < 0 )
			return false;

		if ( d < (int)pSeg->mSize )
		{
			addr_d = pSeg->RawDataOffset() + d + delta;
			return true;
		}

		int size0 = pSeg->size();
		d = size0 - pSeg->RawDataSize();
		if ( d > 0 )
			delta += d;
	}*/
}



bool DUMP_t::ADDR2D( Seg_t * pSelf, ADDR addr, ADDR &addr_d )
{
	if (!pSelf->contains(addr))
		return false;

	ADDR d = D(pSelf);
	addr_d = d + (addr - pSelf->base());
	return true;
}


bool DUMP_t::closed( ADDR addr, int f )
{
	return false;
}


