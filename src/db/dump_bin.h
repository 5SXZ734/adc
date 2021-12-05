#pragma once

#include <map>
#include <vector>
#include <string>
#include <strstream>
//#include <sstream>
#include "shared/misc.h"
#include "interface/IADCGui.h"
#include "type_struc.h"
#include "types_mgr.h"
#include "probe.h"
#include "type_module.h"
#include "dump_target.h"

class Type_t;
class Struc_t;
//class TypeUnion_t;
class Array_t;
class Simple_t;
class DRAW_t;
class TypeCode_t;
class Complex_t;
class BinDumper_t;
class DumpTarget_t;
class Main_t;
class StrucDumpIter;
class FRONT_t;
class MyStrStream;
class CodePrinter;
class StrucVarDumper_t;


class DUMPframe_t : public frame_t
{
public:
	DUMPframe_t(BinDumper_t &r, const I_DataSourceBase &rData, CTypePtr iCont);
	~DUMPframe_t();
	DUMPframe_t &operator=(const DUMPframe_t &);
	unsigned advanceBits(unsigned);//returns rda of bytes

	int level() const { return miLevel; }
	ADDR va(ROWID da) const {
		return ADDR(da - mBaseDA) + m_addr;
	}
	ADDR rda2va(ROWID rda) const {
		return ADDR(rda) + m_addr;
	}
	ADDR offset() const {
		return m_addr - cont()->base();
	}

	ROWID limitDA() const { return mLimitDA; }
	void setLimitDA(ROWID d) { mLimitDA = d; }

	BinDumper_t		&mr;
	const I_DataSourceBase &mrData;
	TypePtr			miModule;
	TypePtr			miSeg;
	
	unsigned		miExtentOpen;//-1 for CDTs and functions with opened range
	unsigned		miExtent;
	ROWID			mLimitDA;
	DUMPframe_t		*mpUp;
	ROWID			mBaseDA;
	unsigned		mBitsRDA;		//if dumping a bitfield

	CodePrinter*	mpCodePending;

	int				miLevel;
	int				mIncomplete;
	DUMPframe_t		*mpDumpingUnion;
	ROWID			mNextChildRDA;
	uint8_t			muLineStatus;
	unsigned		mbCollapsed:1;
	unsigned		mbDumpingSeg:1;
	unsigned		mbTemp:1;
	unsigned		mbProblemLine:1;//re-setted at row flush
	unsigned		mbLarge:1;//64-bit context?
	unsigned		mbMSB : 1;	//big endian?
	//unsigned		mRowSpan;//for collapsed fields
};

enum E_BreakStatus
{
	E_NONE,
	E_MISALIGNED_DATA,
	E_MISALIGNED_CODE,
	E_MISALIGNED_ARRAY,
	E_MISALIGNED_UNION,
	E_MISALIGNED_GAP,
	E_PARTIAL_PAGE_DONE,
	E_PAGE_END,
	E_RANGE_OVER,
	E_SEG_OVER,
	E_FIELD_OVERLAP,
	E_FIELD_BAD_TYPE,
	E_NO_DATA,
	E_TOO_MANY_LINES,
	E_DEFAULT
};

class BinDumper_t : public ProjectInfo_t
{
	friend class StrucDumpIter;
//	friend class FuncDumpIter;
	friend class SegDumpIter;
	friend class DUMPframe_t;
	friend class StrucVarDumper_t;
	friend class StrucVarDumperEx_t;
	friend class CodePrinter;

public://!
	DumpTarget_t	&mrDumpTarget;
	const I_DataSourceBase &mrData;
	Section_t		mScope;
	ROWID			mnStart0;		//start address
	ROWID			mbDumpFrom;
	//ROWID			mEndDA;
	ROWID			mTargetDA;
	ROWID			mCurrentDA;
	//ROWID			mOffs;
	DUMPframe_t		*mpTopFrame;
	DUMPframe_t		*mpFrames;
	int				mnLinesLeft;	//lines left, initially - lines total
	//int				mRowSpan;
	int				mbFreezePageBreak;
	unsigned		mbFreezeFlush;
	//unsigned		muDumpingStrucvar;
	StrucVarDumper_t* mpStrucvarDumper;
	bool			mbDisableLineSkip;
	//int				mbFreezePageBreakU;
	E_BreakStatus	mBreakStatus;
	std::list<CTypePtr>	mFrontSegs;
	unsigned		miRowIDLenMax;
	MemoryMgr_t		mMemMgrNS;//for non-serializable entities (such as strucvar's contents)
	TypesMgr_t		mTypesMgrNS;

public:
	BinDumper_t(ProjectInfo_t &, DumpTarget_t &, const I_DataSourceBase &);
	~BinDumper_t();

	void dump(const Section_t&, ROWID start, int lines);

private:
	friend class MyDataStreamBase;
	friend class CodeDumpTarget_t;

	void InitFrame(DUMPframe_t &, unsigned range = -1);
	void InitFrame(DUMPframe_t &, const Section_t&, CTypePtr module);
	void FeedLine(ROWID, unsigned rowSpan = 0, int rowShift = 0, bool bBits = false, bool bNoColDataExtent = false);
	void adjustLevelInfo();
	void dumpCellVA(ROWID da);
	void dumpCellRA(ROWID rda);
	void dumpCellDA(ROWID, unsigned);
	const I_DataSourceBase &GetDataSource() const;
	bool GetRawAddr(ROWID da, OFF_t &) const;
	TypePtr GetVirtualAddr(ROWID da, ADDR &) const;
	void setLevelInfo(int n);
	TypePtr GetCurrentSeg() const;
	ROWID curRDA() const {
		return mCurrentDA - mpFrames->mBaseDA;
	}
	ROWID curDA() const {
		return mCurrentDA;
	}
	Block_t RawBlock(const DUMPframe_t& f) const {
		return Block_t(RawOffs(f), RawSize(f));
	}
	OFF_t RawOffs(const DUMPframe_t& f) const {
		if (f.m_offs == OFF_NULL)
			return OFF_NULL;
		ROWID rda(curRDA());
		return rda + f.m_offs;
	}
	OFF_t RawSize(const DUMPframe_t& f) const {
		if (f.m_offs == OFF_NULL)
			return 0;
		ROWID rda(curRDA());
		return f.m_size - rda;
	}
	ADDR Range(const DUMPframe_t& f) const {
		//return f.mRange;
		ROWID daLim(f.limitDA());
		assert(daLim != -1);
		ROWID da(curDA());
		if (daLim > da)
			return ADDR(daLim - da);
		return 0;
	}
	void SetRange(DUMPframe_t& f, ADDR d) {
		//f.mRange = d;
		ROWID da(curDA());
		f.setLimitDA(da + d);
	}

	ADDR curVA() const;
	ROWID targetRDA() const;
	bool checkSkipLine(CFieldPtr, bool &bDisableLineSkip) const;
	MemoryMgr_t		&memMgrNS(){ return mMemMgrNS; }
	TypesMgr_t		&typesMgrNS(){ return mTypesMgrNS; }
	TypePtr module() const
	{
		if (mpFrames)
			return mpFrames->miModule;
		return nullptr;
	}
	const I_DataSourceBase &data() const {
		if (!mpFrames)
			return mrData;
		return mpFrames->mrData;
	}
	
	bool isDumpingStrucvar() const {
		return mpStrucvarDumper != nullptr;
	}
	void advance(unsigned va, unsigned ra);
	void dumpType(CTypePtr, AttrIdEnum, unsigned range);
	void dumpModule(CTypePtr);
	void dumpField2(CFieldPtr, CTypePtr iTypeProxy = nullptr);
	void dumpFieldType(CFieldPtr, unsigned range, CTypePtr iTypeProxy = nullptr);
	void dumpStrucEnd(CFieldPtr);
	void dumpFuncEnd(CFieldPtr);
	void dumpStrucEnd0(DUMPframe_t &, CFieldPtr);
	void dumpFuncEnd0(DUMPframe_t &, CFieldPtr);
	void dumpSimple2(CTypePtr, AttrIdEnum, unsigned);
	void dumpSimple2(uint8_t typ, AttrIdEnum , size_t index, size_t arr, unsigned range, CTypePtr = nullptr);
	void dumpEnum(CTypePtr, AttrIdEnum, unsigned);
	//void dumpSeg2(CTypePtr, const I_DataSourceBase &);
	//void dumpOverSeg(CTypePtr);
	//void dumpSegRef2(TypeSegate_t *, unsigned);
	void dumpFunc2(CTypePtr, int);
	void dumpArray2(CTypePtr, AttrIdEnum, unsigned, CTypePtr iIndexRef = nullptr);
	void dumpArrayIndex(CTypePtr, AttrIdEnum, unsigned);
	void dumpStruc2(CTypePtr, unsigned);
	void dumpStrucvar(CTypePtr, unsigned);
	void dumpUnkBit0(unsigned num, unsigned start);
	void dumpUnion2(CTypePtr, unsigned);
	void dumpUnionFields(CFieldPtr, unsigned);
	void dumpCode2();
	void dumpCodeBegin(CFieldPtr, unsigned);
	void dumpCodeEnd(CFieldPtr);
	void dumpUnk0(unsigned num, unsigned start);//no skip
	void dumpUnk2(unsigned num, bool bGap);
	void dumpHeader();
	void dumpStrucIter(StrucDumpIter &, StrucDumpIter &, unsigned);
	unsigned dumpDataRow(DataStream_t *, uint8_t optyp, AttrIdEnum, size_t &arrIndex, size_t &arr, bool bSwapBytes, CTypePtr = nullptr);
	unsigned drawValue(const value_t &, uint8_t optyp, AttrIdEnum attr, MyStrStream &) const;
	unsigned drawValueEx(const value_t &, uint8_t optyp, AttrIdEnum attr, MyStrStream &) const;//resolve names
	void drawValueRef(const value_t &, AttrIdEnum attr);
	void drawChildHere(uint8_t iColor);
	void flushRow(bool problem = false);

	int GetLevel();
	uint8_t lineColor(const DUMPframe_t *) const;
	bool decode(ADDR addr, bool bFunc, IOutpADDR2Name::out_t &) const;
	int safeColor(int iColor) const;

	void drawFieldName(CFieldPtr, CTypePtr);
	void drawFieldType(CFieldPtr, CTypePtr);
	void dumpNameRefScoped(CObjPtr, std::ostream &, CTypePtr scope = nullptr) const;
	void drawFieldExtra(CFieldPtr);
	void drawRowExtra(CTypePtr, ADDR, ADDR);
	void drawColumn(COLID, const char*) const;

	//bits
	void dumpBits(CTypePtr, AttrIdEnum, unsigned, CTypePtr = nullptr);
	void dumpBitset(CTypePtr, unsigned);
	void drawBitsetLine(CFieldPtr, CTypePtr, CTypePtr);
	unsigned advanceBits(unsigned rdaBits);//returns bytes advanced
	void dumpBitField(CFieldPtr, bool bNextChild, unsigned range);
	void dumpBitGap(unsigned, bool bFinal, bool bCompact);

	/*class RAII_Strucvar_t
	{
		BinDumper_t &mr;
		ROWID startDA;
		ADDR startVA;
	public:
		RAII_Strucvar_t(BinDumper_t &r)
			: mr(r)
		{
			startDA = mr.startStrucvar();
			startVA = mr.curVA();
		}
		~RAII_Strucvar_t()
		{
			mr.endStrucvar(startDA, startVA);
		}
	};*/
public:
	void drawName2(CFieldPtr, MyStrStream &) const;
	void drawCommentDemangled(CFieldPtr) const;
	void drawNameSuffix(CTypePtr, CTypePtr, MyStrStream &) const;
	void drawTypeSuffix(CTypePtr, CTypePtr, MyStrStream &) const;
	//Strucvar_t::FieldIt BeginIt(Strucvar_t *);
	TypePtr	findSegmentD(Seg_t *, ROWID daddr, int &extra);
	void EnterNoBreakSection();
	void LeaveNoBreakSection();
	void Break(E_BreakStatus);
	void PushFrontPtr(CTypePtr);
	void PopFrontPtr();
	//bool checkStrucvarBeginsHere(CFieldPtr ) const;
	ROWID SizeOf(CTypePtr, ROWID * = nullptr) const;
	ROWID SizeOfFunc(CTypePtr, ROWID) const;
};


#define R_POSTERIOR(i)	((i)->second.adjusted != 0)
#define R_SPAN(i)		(i)->second.span
#define R_ROW(i)		(i)->first


#include "dump_cache.h"

class DumpVisitor_t;

//console-bound output
class COUTbin_t : public CACHEbin_t
{
	DumpInvariant_t*	mpInvariant;
	DumpTarget_t*		mpTarget;
public:
	COUTbin_t(TypePtr iModule, /*ROWID, ROWID, */int row_width);
	~COUTbin_t();
	//void init(){ aquireTarget();}
	void outputRow(adcui::DUMPOS, std::ostream &, DumpVisitor_t &);
protected:
	virtual DumpTarget_t *ptarget() const { return mpTarget; }
	//virtual DumpTarget_t *aquireTarget();
	//virtual void releaseTarget();
};








