
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
#include "dasmout.h"

using namespace std;






/////////////////////////////////////////////////////////////////



void DRAW_t::Iterator::Seek(ADDR addr, unsigned int line)
{
	rowIt = drawObj.mColumnMgr.mRows.find(addr);
	if (rowIt != drawObj.mColumnMgr.mRows.end())
	{
	}
}

void DRAW_t::Iterator::Forward()
{
//	vecColumn_it itCol;
//	for (itCol = drawObj.mColumnMgr.mCols; itCol != drawObj.mColumnMgr.end(); itCol++)
//	{
//	}
}

void DRAW_t::Iterator::Backward()
{
}


//////////////////////////////////////////////////////////////////


void DRAW_t::incLevel()
{
	mColumnMgr.AppendText( CLMN_LEVEL, (char)0 );
//	mpArr[ CLMN_LEVEL ][mlevel] = 0;
	mlevel++;
//	mpArr[ CLMN_LEVEL ][mlevel] = 0;
}

void DRAW_t::setTreeInfo( int f )
{
/*	char * s = mColumnMgr.CellData( CLMN_LEVEL );
	char c = s[ mlevel-1 ];
	c |= f;
	s[ mlevel-1] = c;*/
}

void DRAW_t::clearTreeInfo( int f )
{
/*	char * s = mColumnMgr.col_str( CLMN_LEVEL );
	char c = s[ mlevel-1 ];
	c &= ~f;
	s[ mlevel-1] = c;*/
}


















////////////////////////////////////////////////////////////////

char DRAW_t::mbuf[256];

Column_t& DRAW_t::Column(COLID colID)
{ 
	return mColumnMgr.mCols[mColInfo[colID]];
}

void DRAW_t::reset( ADDR addr, int line )
{
//?	mDump.reset( addr, line );
}

void DRAW_t::col_setTree( int colID )
{
	//assert( colID < (int)mCols.size() );
	mcoltree = CLMN_NAMES;
}

void DRAW_t::drawRowID(ROWID)
{
/*?	PDATA pdata = NULL;

	ADDR addr;

	if ( mDump.D2R( mDump.maddr_pr, addr, pdata ) )
	{
		char buf[9];
		sprintf( buf, "%08X", addr);
		mColumnMgr.add_col( CLMN_ADDR, buf );
	}

	if ( do_bytes )
		drawBytes( pdata, mDump.milen, true );*/
}

void DRAW_t::drawAddressV()
{
/*?	ADDR addr;
	if ( !mDump.D2V( mDump.maddr_pr, addr ) )
		return;

	char buf[10];
	sprintf(buf, "%08X", addr);
	mColumnMgr.add_col( CLMN_ADDR_V, buf );*/
}

void DRAW_t::drawName( Field_t * f, Complex_t * pParent )//Type_t * pCode, int nameID, Namespace_t * NS )
{
	if ( f )
	{
		if (f->name())
		{
//?			const char * n = pParent->nameSpace()->get(f->nameID());
			const char * n = f->name();
			mColumnMgr.AppendText(CLMN_NAMES, n, 0);
		}
		else
		{
			const char * p = f->getName( pParent );//nameID, NS );
			mColumnMgr.AppendText(CLMN_NAMES, p);
		}

		mColumnMgr.col_color( CLMN_NAMES, COLOR_DASM_DATA_NAME);

		if (f->type())
		if (f->type()->typeCode())
			mColumnMgr.AppendText(CLMN_NAMES, ":");
	}
	else
	{
		mColumnMgr.AppendText(CLMN_NAMES, "unk");
	}
}

void DRAW_t::drawFuncBeg( int line )
{
	switch ( line )
	{
//	case 0:
//		col_color( CLMN_NAMES, COLOR_DASM_COMMENT );
//		add_col( CLMN_NAMES, "; --------------- S U B R O U T I N E ---------------------------------------" );
//		mskip_col = CLMN_NAMES;
//		break;
	case 0:
		{
		//const char * p = mpfunc->getName( *this, mpfunc->type()->nameSpace() );
		//add_col(CLMN_NAMES, p);
//?		col_color( CLMN_CODE, COLOR_DASM_FUNC_NAME );
//?		add_col( CLMN_CODE, "proc" );
		//mcol = COLOR_DASM_DIRECTIVE;
		}
		break;
	}
}

void DRAW_t::drawFuncEnd( int line )
{
	switch( line )
	{
	case 0:
		{
		//col_color( CLMN_NAMES, COLOR_DASM_FUNC_NAME );
		//const char * p = mpfunc->getName( *this, mpfunc->type()->nameSpace() );
		//add_col(CLMN_NAMES, p);
		mColumnMgr.col_color( CLMN_CODE, COLOR_DASM_DIRECTIVE );
		mColumnMgr.AppendText( CLMN_CODE, "endp" );
		}
		break;
	case 1:
		break;
	}

	do_bytes = 0;
}

void DRAW_t::Clear()
{
	mColumnMgr.mRows.clear();
//?	mDump.clean();
//?	mlevel = 0;
	//?for (int i = 0; i < CLMN_TOTAL; i++)
		//?mColumnMgr.mCols[i].clear();

//?	mcol = mbgnd = 0;
//	msel_pos = 0; msel_len = 0;
//	mskip_col = -1;
}

#define MAX_BYTES_LEN 16

void DRAW_t::drawBytes(const void * pBytes, unsigned int len, bool bSpace)
{
	if ( len == 0 )
		return;

//	char * p0 = mpArr[CLMN_BYTES];
//	char * p = p0;
	char s[3+1] = {'?','?',' ',0};
//	if ( bSpace )
//		*p++ = ' ';
	if ( len > MAX_BYTES_LEN )
		len = MAX_BYTES_LEN;

	for (int i = 0; i < (int)len; i++)
	{	
//		if ( p-p0 >= BUFSIZE_BYTES-4)
//			break;
		if (pBytes)
			sprintf( s, "%02X ", ((unsigned char *)pBytes)[i] );
		mColumnMgr.AppendText( CLMN_BYTES, s );
	}
}

void DRAW_t::drawStr( const char * pLine, int pos )
{
	int colID = CLMN_CODE;
	int level = 0;
	
//	char * p[CLMN_TOTAL];
//	for (int i = 0; i < CLMN_TOTAL; i++)
//		p[i] = mpArr[i];

	while (1)
	{
		char c = *pLine;
		if ( c == 0 )
			break;
		pLine++;
		switch (c)
		{
		case (char)0xF0:
			if ( *pLine < CLMN_TOTAL )
				colID = *pLine;
			pLine++;
			continue;
		case (char)0xF1:
			level = *pLine;
			pLine++;
			continue;
		}
	
		//*p[colID] = c;
		//p[colID]++;
		std::string s;
		s.push_back( c );
		mColumnMgr.AppendText( colID, s.c_str() );
	}

//	for (int i = 0; i < CLMN_TOTAL; i++)
//		if ( p[i])
//			*(p[i]) = 0;
}

void DRAW_t::NewRow(ADDR rowID)
{
	mColumnMgr.newRow(rowID);
}

void DRAW_t::drawData(IStream_t * pis, int typ, int &arr, int)
{
	int size0 = typ & 0xF;
	assert(size0 <= OPSZ_QWORD);

	int old_col = set_color(COLOR_DASM_CODE);

	mos << STR_DATASZ(size0);
	mos << " ";

	char buf[80];
	char * pbuf = buf;

	for (int i = 0; i < arr; i++)
	{
		if ( pis != NULL )
		{
			value_t v;
			v.clear();

			pis->read((char *)&v.i8, size0);

			if ( pbuf == buf ) 
			if ( i > 0 )
				mos << ", ";

			if ((size0 == OPSZ_BYTE) && isprint(v.ui8))
			{
				char c = (char)v.ui8;
				*pbuf++ = c;
			}
			else
			{
				if ( pbuf > buf ) 
				{
					int col_old = set_color(COLOR_DASM_CHAR);
					mos << "'";
					for ( int i = 0; i < pbuf-buf; i++ )
						mos << buf[i];
					mos << "'";
					set_color(col_old);
					pbuf = buf;
					mos << ", ";
				}

				if ( MAIN.littleEnd() )
					Swap( &v.ui8, size0 );

				out_imm(&v.ui8, size0, 0, 0, COLOR_DASM_NUMBER);
			}

			//drawBytes(&v, size0, i > 0);
		}
		else
		{
		}
	}

	if ( pbuf > buf ) 
	{
		int col_old = set_color(COLOR_DASM_CHAR);
		mos << "'";
		for ( int i = 0; i < pbuf-buf; i++ )
			mos << buf[i];
		mos << "'";
		set_color(col_old);
	}

	set_color(old_col);
	mos << std::ends;
}

void DRAW_t::drawType( ADDR addr_r, Simple_t * pType, int num )
{/*?
	IStrStream0_t mis( mDump.mpseg->findBlock(addr_r) );
	IStream_t& is = mis;

	begin();
	if ( is.seekg(addr_r ) )
		drawData( &is, pType->size(), num);
	else
		drawData( NULL, pType->size(), num);
	end();*/

	//col_color( CLMN_ADDR, COLOR_DASM_ADDR_DATA );
	//col_color( CLMN_BYTES, COLOR_DASM_BYTES );
}

void DRAW_t::drawUnknown( ADDR addr )
{
/*?	begin();

	unsigned int size = 0;
	unsigned char b = 0;

	ADDR addr_pr = addr;

	if (mDump.mpseg)
	{
		IStrStream0_t mis( mDump.mpseg->findBlock(addr) );
		IStream_t& is = mis;

		if ( is.seekg(addr) )
		{
			b = is.get();
			size = OPSZ_BYTE;
		}
	}

//	col_color( CLMN_BYTES, COLOR_DASM_BYTES_UNK );
	std::string s;
	if (size == 0)
	{
//		add_col( CLMN_BYTES, "??" );
		s = "?";
	}
	else
	{
//		int i = b;
//		s.sprintf( "%02X", i );
//		add_col( CLMN_BYTES, s.ascii() );
		
		s = Int2Str(b, I2S_HEXA|I2S_MODULO);
	}

	int old_col = set_color(COLOR_DASM_UNK);
	mos << "db";
	int indent = 5 - (int)s.length();
	assert(indent > 0);
	while (indent--) mos << " ";
	mos << s;

	set_color(COLOR_DASM_COMMENT);
	mos << " ;";

	if (isprint(b))
		mos << " " << (char)b;

	set_color(old_col);

	mColumnMgr.col_color( CLMN_ADDR, COLOR_DASM_UNK );
	mColumnMgr.col_color( CLMN_ADDR_V, COLOR_DASM_UNK );
	mColumnMgr.col_color( CLMN_BYTES, COLOR_DASM_UNK );

	end();*/
}

void DRAW_t::drawDataArrayOpen( Array_t * pArray, Name_t * pName )
{
	if ( pName )
	{
		const char * n = pName;//?NS->get(nameID);
		mColumnMgr.AppendText( CLMN_NAMES, n, 0 );
	}
	mColumnMgr.col_color( CLMN_NAMES, COLOR_DASM_DATA_NAME );
}

void DRAW_t::drawDataStrucOpen( Type_t * pType, Name_t * pName )
{
	if ( pName )
	{
		const char * n = pName;//?NS->get(nameID);
		mColumnMgr.AppendText( CLMN_NAMES, n, 0);
	}
	else
	{
		char buf[32];
		sprintf( buf, "[%d]", *(int*)&pName );
		mColumnMgr.AppendText( CLMN_NAMES, buf );
	}
	
	if ( pType )
	{
		std::string s;
		s.push_back( (char)0xFF );//color
		s.push_back( (char)COLOR_DASM_COMMENT );
		s += "{";
		if ( pType->name() )
			s += pType->name();
		else
		{
			if (pType->typeSeg())
				s += "seg";
			else if (pType->typeFunc())
				s += "func";
			else if (pType->typeStruc())
				s += "struc";
			else if ( pType->typeUnion() )
				s += "union";
		}
		s += "}";
		mColumnMgr.AppendText( CLMN_CODE, s.c_str() );
	}

	if ( pType->typeFunc() )
	{
		drawFuncBeg(0);
	}

	mColumnMgr.col_color( CLMN_NAMES, COLOR_DASM_DATA_NAME );
//	mskip_col = CLMN_NAMES;
}

void DRAW_t::drawDataStrucClose( Struc_t * pStruc, Name_t * pName )
{
	if ( pName )
	{
		mColumnMgr.col_color( CLMN_NAMES, COLOR_DASM_DATA_NAME );
		const char * n = pName;//?NS->get(nameID);
		mColumnMgr.AppendText( CLMN_NAMES, n, 0 );
	}
}

void DRAW_t::drawComment( const char * pComment )
{
	if ( pComment != NULL )
	{
		std::string s = "; ";
		s += pComment;
		mColumnMgr.AppendText( CLMN_COMMENTS, s.c_str() );
		mColumnMgr.col_color( CLMN_COMMENTS, COLOR_DASM_COMMENT );
	}
}

void DRAW_t::drawUnk()
{
	char buf[NAME_LEN_MAX];
	buf[0]=0;//!mpobj->getName( *this, buf, sizeof(NAME_LEN_MAX) );
	mColumnMgr.AppendText( CLMN_NAMES, buf );
	mcol = COLOR_DASM_UNK;
	mColumnMgr.col_color( CLMN_NAMES, COLOR_DASM_UNK_NAME );
	mColumnMgr.col_color( CLMN_BYTES, COLOR_DASM_ADDR_UNK );
}

void DRAW_t::drawCode( ins_t &ins )
{
	begin();
	out_code(ins);
	end();
}

void DRAW_t::begin()
{
	mos.seekp(std::ios::beg);
}

void DRAW_t::end()
{
	mos << std::ends;
	mColumnMgr.AppendText( CLMN_CODE, mbuf, mostr.pcount() );
}

void DRAW_t::print( const char * str )
{
	mColumnMgr.AppendText( CLMN_NAMES, str );
	do_bytes = 0;
}


int DRAW_t::decodeAddress(ADDR addr_v, char * name, int& loc_sz)
{
	loc_sz = 0;
	
	dC_t * pdC = MAIN.struc()->typedC();

	Seg_t * pSeg = NULL;
	if ( pdC )
		pSeg = pdC->findSegment( addr_v );

	if ( pSeg == NULL )
		return 0;

	Struc_t * s = pSeg;

	int offs = addr_v;;
	Field_t * f = s->field0( offs );
	if ( f != NULL )
	{
		if ( f->type()->typeStruc() )
		{
			int offs2 = f->offset();
			ADDR d = offs2;

			s = f->type()->typeStruc();
			f = s->fieldT( d );
			if ( f != NULL )
			{
				if ( d != f->offset() )
					f = NULL;
			}
		}

		int d = addr_v - f->offset();
		if ( d != 0 )
			if ( f->size() <= 0 || d >= f->size() )
				return 0;

		if ( f != NULL )
		{
			const char * p = f->getName( s );//f->nameID(), s->nameSpace() );
			if ( p != NULL )
			{
				strcpy( name, p );
				if ( d > 0 )
					strcat( name, fmt("+%x", d) );
			}
		}
		return 1;
	}

	return 0;
}

void DRAW_t::update2( ADDR addr )
{
/*?	do_bytes = 1;
	mlevel = 0;

	for ( int i = 0; i < (int)mColumnMgr.mCols.size(); i++ )
		mColumnMgr.mCols[i].clear();

//	mColumnMgr.col_color( CLMN_ADDR, COLOR_DASM_ADDR_UNK );
//	mColumnMgr.col_color( CLMN_BYTES, COLOR_DASM_ADDR_UNK );

	mDump.update2( addr );*/
}
















void DRAW_t::Dump()
{
	DUMPbin_t dumpObj(*this);
	Struc_t * pStruc = (Struc_t *)MAIN.Project();
	if (!pStruc)
		return;
	Field_t * pField = pStruc->Field(0);
	if (!pField)
		return;
	dumpObj.dump(pField, 0, -1);
}

void DRAW_t::reset()
{
	Clear();
}

int DRAW_t::lineFeed(ROWID, int)
{
	return 0;
}

void DRAW_t::draw(int clmn, const char * str)
{
	mColumnMgr.AppendText(clmn, str);
}

void DRAW_t::drawIndent(int d)
{
}

void DRAW_t::drawCode(const ins_t&)
{
}


