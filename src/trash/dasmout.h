
#ifndef __DASMOUTPUT_H__
#define __DASMOUTPUT_H__

#include <map>
#include <vector>
#include <string>
#include <strstream>
#include "dc/main.h"
#include "dc/struc.h"
#include "back/x86out.h"
#include "back/dump_bin.h"


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
class CACHEbin_t;
class DRAWobj_t;

#define Name_t	const char


#include <sstream>










class DRAW_t : public DRAWobj_t, public Outp_t
{
protected:
	virtual void reset();
	virtual int lineFeed(ROWID, int);
	//virtual void setRowID(ROWID){}
	virtual void draw(int clmn, const char * str);
	virtual void drawIndent(int d);
	virtual void drawCode(const ins_t&);
	virtual void drawData(IStream_t * pis, int typ, int &arr, int);
	virtual void drawStr( const char * str, int pos = 0 );
	virtual void NewRow(ADDR);

public:
	class Iterator
	{
	public:
		ADDR		addr;
		int			line;
		mapRow_it	rowIt;
	public:
		Iterator(DRAW_t &d)
			: drawObj(d)
			{
				addr = 0;
				line = 0;
				rowIt = drawObj.mColumnMgr.mRows.end();
			}
		void Seek(ADDR, unsigned int);
		void Forward();
		void Backward();
	private:
		DRAW_t	&drawObj;
	};

public:
	virtual Column_t& Column(COLID colID);
	//virtual int ScreenLine(){ return mscr_line; }
	virtual ADDR CurrentAddr(){ return 0; }
	virtual int BackgroundColor(){ return mbgnd; }
	virtual int ColumnColor(){ return mcol; }
	virtual void SetColumnColor(int c){ mcol = c; }
	void col_setTree( int colID );
	virtual int ColTree(){ return mcoltree; }
	virtual void incLevel();
	virtual void setTreeInfo( int );
	virtual void clearTreeInfo( int );
	virtual int Level(){ return mlevel; }

public:
	int		mlevel;
	Table_t	mColumnMgr;
	int		mcoltree;
	int		mColInfo[ CLMN_TOTAL ];

	//int		mscr_line;
	int		mcol;
	int		mbgnd;
//	int		mskip_col;

	//DUMP_t&	mDump;

	static char mbuf[256];
	std::ostrstream	mostr;

	DRAW_t() 
		: Outp_t(mostr),
		mostr(mbuf, sizeof(mbuf))
//		mDump(dump)
	{	
		for (int i = 0; i < CLMN_TOTAL; i++)
			mColInfo[i] = 0;

		mColInfo[CLMN_LEVEL] = mColumnMgr.newColumn("LEVEL");
		mColInfo[CLMN_ADDR] = mColumnMgr.newColumn("OFFSET", 8);
		mColInfo[CLMN_ADDR_V] = mColumnMgr.newColumn("VA", 9);
		mColInfo[CLMN_BYTES] = mColumnMgr.newColumn("BYTES", 16);
		mColInfo[CLMN_NAMES] = mColumnMgr.newColumn("NAMES", 16);
		mColInfo[CLMN_CODE] = mColumnMgr.newColumn("CODE", 36);
		mColInfo[CLMN_COMMENTS] = mColumnMgr.newColumn("COMMENTS", 100);

		mColumnMgr.col_setFlags( CLMN_ADDR, 1 );
		mColumnMgr.col_setFlags( CLMN_ADDR_V, 1 );
		mColumnMgr.col_setFlags( CLMN_BYTES, 2 );

		//col_setTree( CLMN_ADDR_V );//CLMN_CODE;
		//col_setTree( CLMN_NAMES );
		col_setTree( CLMN_ADDR_V );

		mbgnd = COLOR_DASM_BGND;
//		mDump.setPainter( this );
	}

	void reset( ADDR addr, int line );

	virtual void drawDataArrayOpen( Array_t * pArray, Name_t * pName );
	virtual void drawDataArrayClose( Array_t *, Name_t * ){}

	virtual void drawDataStrucOpen( Type_t * pStruc, Name_t * pName );
	virtual void drawDataStrucClose( Struc_t * pStruc, Name_t * pName );

	virtual void drawType( ADDR addr_r, Simple_t * pType, int num );

	virtual void update2( ADDR addr );
	virtual void Clear();
	virtual void drawCode( ins_t & );
	virtual void drawName( Field_t * f, Complex_t * pParent );//Type_t * pCode, int nameID, Namespace_t* NS );
	//virtual void drawName( int nameID, Namespace_t* NS );
	virtual void drawUnknown( ADDR addr );
	virtual void drawBytes(const void * pBytes, unsigned int len, bool bSpace);
	virtual void drawFuncBeg( int line );
	virtual void drawFuncEnd( int line );
	virtual void drawComment( const char * pName );
	virtual void drawUnk();
	virtual void print( const char * str );
	virtual void printc( int, char ){}
	static int decodeAddress(ADDR addr_v, char * name, int& loc_sz);
	
	void drawRowID(ROWID);
	void drawAddressV();

	void begin();
	void end();

	void Dump();
};

#define DRAW(a)	if(mpD)mpD->a;


#endif//__DASMOUTPUT_H__
