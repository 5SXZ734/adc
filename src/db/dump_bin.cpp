
#include "dump_bin.h"
#include <assert.h>
#include <algorithm>
#include "inttypes.h"//PRIx64

#include "shared/defs.h"
#include "shared/misc.h"
#include "shared/front.h"
#include "shared/data_source.h"
#include "shared/dump_util.h"
#include "type_seg.h"
#include "names.h"
#include "main.h"
#include "proj.h"
#include "type_proxy.h"
#include "type_code.h"
#include "field.h"
#include "anlzbin.h"
#include "clean.h"
#include "dump_strucvar.h"
#include "dump_visit.h"

#define NO_COLORED_LINES	0
#define SQUEEZE_STRUCVAR_GAPS	0
#define STRUCVAR_INSTANTIATION_LIMIT	1000//*100
#define CODE_LINES_MAX	1000//*100//there must be a limit, otherwise it gonna stuck on invalid input

/*static void drawAddr(char buf[32], ADDR64 base, ADDR va, bool bLarge)
{
	if (bLarge)
		sprintf(buf, "%016" PRIx64, base + (ADDR64)va);
	else
		sprintf(buf, "%08X", (ADDR)base + va);
}*/

class FrontPtr//assure set/reset front ptr in bin dumper if 
{
	BinDumper_t &mr;
	CTypePtr miSeg;
public:
	FrontPtr(BinDumper_t &r, CTypePtr pSeg)
		: mr(r), miSeg(pSeg && pSeg->typeSeg()->frontIndex() > 0 ? pSeg : nullptr)
	{
		if (miSeg)
			mr.PushFrontPtr(miSeg);
	}
	~FrontPtr()
	{
		if (miSeg)
			mr.PopFrontPtr();
	}
};

/*static ROWID __viewOffs(CTypePtr pSelf)
{
	if (pSelf->typeModule())
	{
		if (ProjectInfo_t::IsPhantomModule(pSelf))
			return 0;
	}
	ROWID ra(pSelf->typeSeg()->viewOffs());
	if (ra == ROWID_INVALID)
		return 0;
	return ra;
}

static ROWID __viewSize(CTypePtr pSelf)
{
	if (pSelf->typeModule())
	{
		if (ProjectInfo_t::IsPhantomModule(pSelf))
		{
			CTypePtr pSeg(ProjectInfo_t::FindFrontSeg(pSelf));
			if (pSeg)
				return ProjectInfo_t::Size(pSeg);
		}
	}
	ROWID ra(pSelf->typeSeg()->viewSize0());
	if (ra == ROWID_INVALID)
		return ProjectInfo_t::Size(pSelf);
	return ra;
}*/

//////////////////////////////////////////
class StrucDumpIter
{
protected:
	BinDumper_t &mrDump;
private:
	const TypeObj_t &mrStrucRef;
	const Struc_t &mrStruc;
protected:
	const FieldMap& m;
	FieldMapCIt mit;
public:
	StrucDumpIter(BinDumper_t &rDump, const TypeObj_t &rStrucRef)
		: mrDump(rDump),
		mrStrucRef(rStrucRef),
		mrStruc(*mrStrucRef.typeStruc()),
		m(mrStruc.fields()),
		mit(m.end())
	{
	}
	virtual ~StrucDumpIter()
	{
	//	mit = mrStruc.fields().end();
	}
	virtual StrucDumpIter &initialise()
	{
		ROWID rda(mrDump.targetRDA());
		mit = BeginIt(rda);
		return *this;
	}
	bool isUfield() const {
		if (mit == m.end())
			return false;
		FieldMapCIt itnx(mit);
		++itnx;
		if (itnx == m.end())
			return false;
		return KEY(itnx) == KEY(mit);
	}
	CTypePtr strucRef() const { return &mrStrucRef; }
	const Struc_t &struc() const { return mrStruc; }
	virtual operator bool() const
	{
		return mit != m.end();
	}
	virtual void operator=(const StrucDumpIter &o)
	{
		mit = o.mit;
	}
	bool checkValid() const
	{
		FieldMapCIt i(mit);
		bool bIsEos(ProjectInfo_s::IsEosField(VALUE(i)));
		i++;
		return (!bIsEos || mit == m.end());
	}
	virtual StrucDumpIter &operator++()
	{
#ifdef _DEBUG
		checkValid();
#endif
		ADDR lower(KEY(mit));
		do {
			++mit;
		}  while (mit != m.end() && KEY(mit) == lower);//skip U-fields
//		if (mit != m.end())
	//	if (IsEosField(VALUE(mit)))
		//	mit++;
		return *this;
	}
	CFieldPtr nextAddrField(ROWID &rda) const
	{
		FieldMapCIt it(mit);
		do {
			if (++it == m.end())
				return nullptr;
		} while (KEY(it) == KEY(mit));
		rda = KEY(it) - mrDump.mpFrames->m_addr;
		return VALUE(it);
	}
	ROWID offsAdj(ROWID offs) const { return offs; }
	virtual CFieldPtr field(ROWID &rda) const
	{
		if (mit != m.end())
		{
			//rda = offsAdj(KEY(mit) - mrStrucRef.base());
			rda = KEY(mit) - mrDump.mpFrames->m_addr;
			return VALUE(mit);
		}
		return nullptr;
	}
	virtual CFieldPtr fieldOrEos(ROWID &rda) const
	{
		if (mit != m.end())
		{
//			assert(!IsEosField(VALUE(mit)));
			//rda = offsAdj(KEY(mit) - mrStrucRef.base());
			rda = KEY(mit) - mrDump.mpFrames->m_addr;// mrStrucRef.base());
			return VALUE(mit);
		}

		if (m.empty())
			return nullptr;
		FieldMapCIt i(mit);
		i = m.Priorz(i);
		if (!ProjectInfo_s::IsEosField(VALUE(i)))
			return nullptr;
		//rda = offsAdj(KEY(i) - mrStrucRef.base());
		rda = KEY(i) - mrDump.mpFrames->m_addr;
		return VALUE(i);
	}
protected:
	ADDR toAddr(ROWID rda) const 
	{
		return (ADDR)(mrStrucRef.base() + rda);
	}
	FieldMapCIt BeginIt(ROWID rdaToTarget)
	{
		if (mrDump.mbFreezePageBreak)
			return m.begin();

		DUMPframe_t &oFrame(*mrDump.mpFrames);
		if (oFrame.mbCollapsed)
		{
			FieldMapCIt it(m.end());
			ADDR offs(OPSZ_BYTE);
			if (it != m.begin())
			{
				--it;
				offs = KEY(it) - oFrame.m_addr;
			}
			mrDump.advance(offs, offs);//what's about segs?
			return it;
		}

		ROWID tgtRDA(mrDump.targetRDA());

		ADDR targetAddr(oFrame.rda2va(rdaToTarget));// toAddr(rdaToTarget));
		ADDR targetOffs(targetAddr - oFrame.m_addr);

		//FieldMapIt itnx(m.SkipEos(m.UpperBound(targetAddr)));//get the next field above the target range
		FieldMapCIt itnx(m.upper_bound(targetAddr));//get the next field above the target range
		FieldMapCIt it(itnx);
		//if ((it = ProjectInfo_t::EosAwarePrior(m, it)) != m.end())
		if ((it = m.Priorz(it)) != m.end())
		{
			//CFieldPtr pField(VALUE(it));

			//the field at target range or right below
			ADDR addr(KEY(it));

			if (addr < oFrame.m_addr)
			{
				mrDump.advance((ADDR)tgtRDA, targetOffs);
				return itnx;
			}

			//get the first field in range
			//move to the first field in sequence of the same address (union check)
			it = m.lower_bound(addr);

			ADDR offs(addr - oFrame.m_addr);// mrStrucRef.base());

			ADDR addr_nx(itnx == m.end() ? (ADDR)-1 : KEY(itnx));
			ADDR offs_nx(addr_nx == ADDR(-1) ? mrDump.Range(oFrame) : addr_nx - oFrame.m_addr);
			unsigned range(offs_nx - offs);
			ROWID sz(0);
			if (range > 0)
			{
				FieldMapCIt it2(it);
				for (;;)
				{
					CFieldPtr pField(VALUE(it2));
					ROWID da(oFrame.mBaseDA + offs);
					sz = std::max(sz, mrDump.SizeOf(pField->type(), &da));
					if (it2 == m.begin())
						break;
					--it2;
					if (KEY(it2) != KEY(it))
						break;
				}

				if (sz == 0)
					sz = OPSZ_BYTE;
//				else if (IsFunc(pField->type()))
	//				sz = -1;//it is unclear if the code is pending
			}
			else
			{
				STOP
			}
			//ROWID vsz(iType ? iType->view Size() : 1);

/*			if (sz < 0)//need to calculate
			{
				//?assert(mrDump.mOffs == oFrame.mBaseDA);
				ROWID rowIDpr = mrDump.curDA();//of parent struc
				rowIDpr += offs;
				sz = mrDump.mrDumpTarget.cachedObjSize(rowIDpr);
			}*/

			if (sz == ROWID_INVALID)//need to calculate
			{
				if (addr < targetAddr)
				{
					CFieldPtr pField(VALUE(it));
					TypePtr iType(pField->type());
					if (iType->typeCode() || iType->typeThunk())
					{
						mrDump.advance(offs, offs);
						throw (E_MISALIGNED_CODE);
					}
					if (ProjectInfo_t::IsProc(iType))
					{
						//got a function of non-fixed size
						Struc_t &rFunc(*iType->typeStruc());
						FieldMap &m2(rFunc.fields());
						assert(!m2.empty());
						FieldMapRIt i(m2.rbegin());
						if (ProjectInfo_s::IsEosField(VALUE(i)))
							i++;
						if (targetAddr > KEY(i))
						{
							offs = KEY(i) - oFrame.m_addr;
							mrDump.advance(offs, offs);
							throw (E_MISALIGNED_CODE);
						}//otherwiaise have to enter the function
					}
					if (iType->typeStrucvar())
					{
						mrDump.advance(offs, offs);
						throw (E_MISALIGNED_CODE);
					}
					mrDump.advance(offs, offs);
					return it;
				}
				else if (addr == targetAddr)
				{
					mrDump.advance(offs, offs);
					return it;
				}
				else //if (sizepr < 0)
				{
					assert(0);//?
					throw (E_MISALIGNED_CODE);
				}
			}
			else
			{
				if (addr + sz > targetAddr)
				{
					mrDump.advance(offs, offs);
					return it;
				}
				if (mrDump.mrDumpTarget.isCompactMode())
					mrDump.advance(ADDR(offs + sz), ADDR(offs + sz));
				else
					mrDump.advance((ADDR)tgtRDA, targetOffs);
			}
		}
		else
		{
			//no fields above
			if (!mrDump.mrDumpTarget.isCompactMode())
				mrDump.advance((ADDR)tgtRDA, targetOffs);
		}
		return itnx;
	}
};


/*bool OutpADDR2Name::decode(ADDR addr, bool bFunc, std::string &name, int &sz) const
{
	Locus_t loc;
	FieldPtr f(FindFieldInSubsegs(ProjectInfo_t::OwnerSegList(mrSeg.typ eobj()), addr, loc));
	if (f)
	{
		if (bFunc && !IsFunc(f->type()) && IsFunc(f->OwnerStruc()))
			f = f->owner()->parentField();
		name = f->namexx();
		if (f->type() && f->type()->typeSimple())
			sz = f->OpSize();
		else
			sz = 1;
		return true;
	}
	return false;
}*/



DUMPframe_t::DUMPframe_t(BinDumper_t &r, const I_DataSourceBase &rData, CTypePtr iCont)
	: frame_t((TypePtr)iCont, 0),
	mr(r),
	mrData(rData),
	miSeg(nullptr),
	miExtentOpen(0),
	miExtent(0),
	mBaseDA(0),
	mBitsRDA(0),
	mLimitDA(ROWID(-1)),
	mpCodePending(nullptr),
	miLevel(0),
	mpDumpingUnion(nullptr),
	mNextChildRDA(0),
	muLineStatus(0),
	mIncomplete(0),
	mbCollapsed(0),
	mbDumpingSeg(0),
	mbTemp(0),
	mbProblemLine(0),
	mbLarge(0),
	mbMSB(0)
//	mRowSpan(0)
{
	mpUp = mr.mpFrames;
	mr.mpFrames = this;
	//init();
}

DUMPframe_t &DUMPframe_t::operator=(const DUMPframe_t &o)
{
	frame_t::operator=(o);
	assert(&mr == &o.mr);
	miSeg = o.miSeg;
	miExtentOpen = o.miExtentOpen;
	miExtent = o.miExtent;
	mpUp = o.mpUp;
	mBaseDA = o.mBaseDA;
	mBitsRDA = o.mBitsRDA;
	mLimitDA = o.mLimitDA;
	mpCodePending = o.mpCodePending;
	miLevel = o.miLevel;
	mpDumpingUnion = o.mpDumpingUnion;
	muLineStatus = o.muLineStatus;
	mNextChildRDA = o.mNextChildRDA;
	mIncomplete = o.mIncomplete;
	mbCollapsed = o.mbCollapsed;
	mbDumpingSeg = o.mbDumpingSeg;
	//mbTemp = o.mbTemp;//NOT THIS ONE!
	mbProblemLine = o.mbProblemLine;
	mbLarge = o.mbLarge;
	return *this;
}

void BinDumper_t::InitFrame(DUMPframe_t&o, const Section_t& aSect, CTypePtr pModule)
{
	assert(!o.mpUp);

	TypePtr pSeg(aSect.seg);
	Seg_t *pSegPvt(pSeg->typeSeg());
	assert(pSegPvt);

	o.m_addr = pSeg->base() + (ADDR)aSect.off;

	o.miModule = (TypePtr)pModule;

	o.miSeg = pSeg;
	o.mbLarge = pSegPvt->isLarge();
	o.mbDumpingSeg = 1;
	o.mbMSB = pSegPvt->isLittleEnd() ? 0 : 1;

	o.miExtentOpen = o.miExtent = (unsigned)aSect.size;// (unsigned)__viewSize(pSeg);
	if (o.miExtent == -1)
		o.miExtent = pSegPvt->size(pSeg);// Range(o);

	SetRange(o, o.miExtent);
	//if (o.miExtent == 0)
		//;
	o.mBaseDA = aSect.da;// __viewOffs(pSeg);
	/*o.m_offs = aSect.ra;// rawOffs(o.cont());
	if (aSect.ra != OFF_NULL)
		o.m_size = (aSect.size != -1 ? aSect.size : o.miExtent);// rawSize(o.cont());
	else
		o.m_size = 0;*/

	if (aSect.ra != OFF_NULL)
	{
		o.set(0, o.mrData.size());
		o.subspace(aSect.ra, aSect.size);
	}
	else
		o.set(OFF_NULL, 0);
}

void BinDumper_t::InitFrame(DUMPframe_t &o, unsigned range)
{
	Struc_t *pStruc(o.m_cont->typeStruc());
	if (pStruc)
		o.m_addr = o.m_cont->base();

	if (o.cont()->typeModule())
		o.miModule = o.cont();
	else if (o.mpUp)
		o.miModule = o.mpUp->miModule;

	assert(!o.cont()->typeSeg());

	assert(o.mpUp);
	{
		o.miSeg = o.mpUp->miSeg;
		if (o.cont()->typeBitset())
		{
			unsigned range2(o.cont()->sizeBytes());
			if (range2 == 0)
				range2 = range;
			else if (range2 == -1)
				range2 = range;
			SetRange(o, std::min(range2, Range(*o.mpUp)));
		}
		else
			SetRange(o, std::min(range, Range(*o.mpUp)));
		o.mIncomplete = o.mpUp->mIncomplete;
	}
	ROWID da(0);
	o.miExtentOpen = o.miExtent = (unsigned)Size(o.cont(), &da);
	if (o.miExtent == -1)
		o.miExtent = Range(o);
	else if (o.miExtent > Range(o))
		o.mIncomplete++;
	if (o.miExtent == 0)
		o.miExtent = OPSZ_BYTE;

	o.mBaseDA = mCurrentDA;

	o.mbLarge = o.mpUp->mbLarge;
	o.mbMSB = o.mpUp->mbMSB;
	o.m_offs = RawOffs(*o.mpUp);
	if (o.m_offs != OFF_NULL)
		o.m_offs += mCurrentDA - o.mpUp->mBaseDA;
	o.m_size = RawSize(*o.mpUp);
	if (o.m_size != 0)
		o.m_size -= mCurrentDA - o.mpUp->mBaseDA;
	o.mpDumpingUnion = o.mpUp->mpDumpingUnion;
	if (!o.mpDumpingUnion && o.cont()->typeUnion())
		o.mpDumpingUnion = &o;

	if (o.mpUp->mbCollapsed)
		o.mbCollapsed = true;
	o.miLevel = o.mpUp->level();
	if (!o.cont()->typeBitset())//bitsets do not add a level
		o.miLevel++;//GetLevel();
}

DUMPframe_t::~DUMPframe_t()
{
	if (mbTemp)
		return;
	if (mpUp)
	{
		mpUp->mpCodePending = mpCodePending;
		mpCodePending = nullptr;
		//mpUp->mRowSpan += mRowSpan;
	}
	else
	{
//?		assert(!mpCodePending);
		mr.mpTopFrame = nullptr;
	}
	mr.mpFrames = mpUp;
}

unsigned DUMPframe_t::advanceBits(unsigned d)
{
	unsigned startBit(mBitsRDA);
	unsigned endBit(startBit + d);

	unsigned span(endBit / CHAR_BIT - startBit / CHAR_BIT);
	mBitsRDA = endBit % CHAR_BIT;
	return span;
}






/////////////////////////////////////////////////

BinDumper_t::BinDumper_t(ProjectInfo_t &rProjInfo, DumpTarget_t& drawObj, const I_DataSourceBase &rData)
	: ProjectInfo_t(rProjInfo),
	mrDumpTarget(drawObj),
	mrData(rData),
	//mScopeBase(0),
	mnStart0(0),
	//mEndDA(0),
	mnLinesLeft(-1),
	mCurrentDA(0),
	mTargetDA(0),
	mpTopFrame(nullptr),
	mpFrames(nullptr),
	//mRowSpan(0),
//	mAddr(0),
	mbDumpFrom(ROWID(-1)),
	mbFreezePageBreak(0),
	mbFreezeFlush(0),
	//muDumpingStrucvar(0),
	mpStrucvarDumper(nullptr),
	mbDisableLineSkip(false),
	//mbFreezePageBreakU(0),
	mBreakStatus(E_NONE),
	//mOffs(0),
	miRowIDLenMax(0)
{
}

BinDumper_t::~BinDumper_t()
{
	BinaryCleaner_t<> BC2(ProjectInfo_t(*this, mMemMgrNS));
	BC2.destroyUserTypesNS(mTypesMgrNS);
	BC2.destroy(mTypesMgrNS);
}

void BinDumper_t::dump(const Section_t& aScope, ROWID start, int lines)
{
//static int here = 0;
//assert(here == 0);
//here++;
	assert(aScope.seg);
	
	mScope = aScope;
	mCurrentDA = aScope.da;//always start from scope base
	miRowIDLenMax = HexWidthMax(ViewSize(aScope.seg));
	if (miRowIDLenMax == 0)
		miRowIDLenMax = 1;

//start = 0xe652;
//lines = LINES2DUMP;

	//mOffs = 0;
//CHECK(!(start < end))
CHECK(start == 0x3561)
STOP
	assert(mScope.size != 0);//for strucs with no fields
	//ROWID end(mScope.da + mScope.size);

	mnStart0 = start;//may be misaligned!
	mTargetDA = mnStart0;
	//mRowSpan = 0;

#if(0)
fprintf(stdout, "dumping: %X\n", mnStart0);
fflush(stdout);
#endif

	mnLinesLeft = lines;
	mbDumpFrom = ROWID(-1);
	mbFreezePageBreak = 0;
	//mbFreezePageBreakU = 0;
	for (;;) 
	{
		//assert(mrDumpTarget.checkColDataVec());
		mrDumpTarget.coldata_clear();
		mrDumpTarget.coldata_expand();
		mBreakStatus = E_NONE;
		dumpHeader();
		try 
		{
			dumpModule(aScope.seg);
			//dumpType(iScope, ATTR_NULL, -1);
		}
		catch (E_BreakStatus n)
		{
			mrDumpTarget.coldata_clear();
			switch (n)
			{
			case E_MISALIGNED_DATA://data alignment fault
			case E_MISALIGNED_CODE://code alignment fault
			case E_MISALIGNED_ARRAY://array alignment fault
			case E_MISALIGNED_UNION://union alignment fault
			case E_MISALIGNED_GAP:
				assert(mTargetDA > mCurrentDA);
				mTargetDA =  mCurrentDA;
				mnStart0 = mTargetDA;
				mbDumpFrom = mTargetDA;
				mCurrentDA = mScope.da;
				//mOffs = 0;
				mbFreezePageBreak = 0;
				mbFreezeFlush = 0;
				//muDumpingStrucvar = 0;
				mbDisableLineSkip = false;
				continue;
			case E_RANGE_OVER:
				break;
			case E_PAGE_END:
				if (mCurrentDA < start)//this happens when falling into a region with var-type underlaping data above (need to determine it's size first)
				{
					mnStart0 = start;
					mTargetDA =  mnStart0;
					mbDumpFrom = ROWID(-1);
					mCurrentDA = mScope.da;
					assert(mbFreezePageBreak == 0 && mbFreezeFlush == 0 && !mbDisableLineSkip);
					continue;
				}
				//page end
				break;
			case E_FIELD_OVERLAP:
				//?assert(0);
				break;
			default:
#if(_INTERNAL)
				fprintf(STDERR, "[INTERNAL] alignment fault at ~%X\n", (ADDR)start);
				fflush(STDERR);
#endif
				break;
			}
		}
#if(1)
		catch (...)
		{
			STOP
		}
#endif
		break;
	}

#if(0)//dumper dubugging
	fprintf(stdout, "range dumped: ~%X ... ~%X (%d bytes)\n", (unsigned)mnStart0, (unsigned)mCurrentDA, (unsigned)(mCurrentDA - mnStart0));
	fflush(stdout);
#endif
//here--;
}

uint8_t BinDumper_t::lineColor(const DUMPframe_t *pFrame) const
{
#if(!NO_COLORED_LINES)
	if (pFrame->cont())
	{
		if (pFrame->cont()->typeStruc())
			return Project().dumpLineStatus(pFrame->cont());
	}
#endif
	return 0;
}

void BinDumper_t::drawChildHere(uint8_t iColor)
{
	if (mpFrames->mbCollapsed)
		return;

	int iLevel(GetLevel());
	mrDumpTarget.SetLevelInfo(iLevel, adcui::ITEM_HAS_PARENT | iColor);
	if (iLevel > 0)
		mrDumpTarget.SetLevelInfo(iLevel - 1, adcui::ITEM_HAS_CHILD_HERE | iColor);//dumpFuncEnd didn't have it
}

void BinDumper_t::dumpFuncEnd(CFieldPtr pField)
{
	/*if (!(pField->type() && pField->type()->typeFuncEnd()))
		return;
	assert(pField->type() && pField->type()->typeFuncEnd());*/

	if (targetRDA() <= 0 || (curDA() >= mbDumpFrom))//(mbDumpPriorLines > 0))
	{
		if (mpFrames->mpUp)
			drawChildHere(lineColor(mpFrames));
		mrDumpTarget.drawData(".endp", pField ? adcui::COLOR_DASM_ENDP : adcui::COLOR_DASM_ENDP2);//directive
	}

	//FeedLine();
}

void BinDumper_t::dumpStrucEnd(CFieldPtr /*pFieldC*/)
{
/*	FieldPtr pField((FieldPtr)pFieldC);
	if (!IsEosField(pField))
		return;*/
	if (targetRDA() <= 0 || (curDA() >= mbDumpFrom))//(mbDumpPriorLines > 0))
	{
		if (mpFrames->mpUp)
			drawChildHere(lineColor(mpFrames));
		mrDumpTarget.drawData(".ends", adcui::COLOR_DASM_CODE);//directive
	}

	//FeedLine();
}

/*bool BinDumper_t::checkStrucvarBeginsHere(FieldPtr pField) const//go down to find out if we need to dump a group of strucs at this DA
{
	unsigned range(-1);
	TypePtr iType(pField->type());
	while (iType)
	{
		if (iType->typeStrucvar())
		{
			ROWID sz(SizeOf(pField->type(), &bc));
			if (range < sz)
				sz = range;
			if (targetRDA() < sz)
				return true;
			return false;
		}
		if (iType->isShared())
			break;
		/ *if (iType->typeBitset())
		{
			ROWID sz(iType->typeBitset()->size Bytes(iType));
			if (targetRDA() < sz)
				return true;
			return false;
		}* /
		if (iType->typeUnion())
			break;
		Struc_t *pStruc(iType->typeStruc());
		if (!pStruc)
			break;
		ADDR iBase(pStruc->base(iType));
		FieldPtr pField2;
		if ((pField = Field(iType, iBase, &pField2, FieldIt_Exact)) == nullptr)
			break;
		iType = pField->type();
		if (pField2)
			range = pField2->_key() - pField->_key();
	}
	return false;
}*/

static TypePtr CheckProxyType(CFieldPtr pField, CTypePtr iTypeProxy)
{
	CTypePtr iType(pField->type());
	if (iTypeProxy)
		iType = iTypeProxy;
	return (TypePtr)iType;
}

void BinDumper_t::dumpField2(CFieldPtr pFieldC, CTypePtr iTypeProxy)
{
	CFieldPtr pField((FieldPtr)pFieldC);
	CTypePtr iOwner(pField->owner());
	CTypePtr iType(CheckProxyType(pField, iTypeProxy));

	dumpCodeEnd(pField);//finalize a pending code if any

	//assert(!iType || !iType->typeFuncEnd());

	if (!checkSkipLine(pField, mbDisableLineSkip))
	{
		if (iType)
		{
			/*if (iType->typeCode())
				FeedLine();//make a gap at every label
			else */
			if (iType->typeStruc())
			{
				if (IsProc(iType) || pField->attrib() == ATTR_NEWLINE)
					FeedLine(curDA());
			}
		}

		//if (!pField->isCloneMaster())
		{
			if (!iType || !iType->typeBitset())
			{
				uint8_t f(lineColor(mpFrames));
				/*const FieldMap& m(iOwner->typeStruc()->fields());
				FieldMapCIt it(pField);
				assert(it != m.end() && KEY(it) == pField->_key());
				for (int count(0);; count++)//assume U-fields
				{
					if (count > 0)
						FeedLine(curDA());*/
				if (mpFrames->mpUp)
					drawChildHere(f);
				drawFieldType(pField, iTypeProxy);
				drawFieldName(pField, iTypeProxy);
				drawFieldExtra(pField);
				/*if (++it == m.end() || KEY(it) != pField->_key())
					break;
			}*/
			}
		}
		/*else
		{
			//assert(mpFrames->mpUp);
			uint8_t f(lineColor(mpFrames));
			int iLevel(GetLevel());
			Seg_t* pSeg(mpFrames->cont()->typeSeg());
			assert(pSeg);
			const ConflictFieldMap& m(pSeg->conflictFields());
			assert(!m.empty());
			ClonedFieldMapCIt it(m.lower_bound(pField->_key()));
			assert(it != m.end() && KEY(it) == pField->_key());
			for (int count(0); ; count++)
			{
				if (count > 0)
					FeedLine(curDA());
				CFieldPtr pField2(VALUE(it));
				drawChildHere(f);
				mrDumpTarget.SetLevelInfo(iLevel, f);
				drawFieldType(pField2, pField->type());
				drawFieldName(pField2, pField->type());
				if (++it == m.end() || KEY(it) != pField->_key())
					break;
			}
		}*/
	}
}

void BinDumper_t::dumpFieldType(CFieldPtr pFieldC, unsigned range, CTypePtr iTypeProxy)
{
	CFieldPtr pField((FieldPtr)pFieldC);
	CTypePtr iOwner(pField->owner());
	CTypePtr iType(CheckProxyType(pField, iTypeProxy));

	if (!iType)
	{
		dumpUnk2(OPSZ_BYTE, false);
		return;
	}

	if (iType->typeCode() || iType->typeThunk())
	{
		if (!IsEntryLabel(pField))
			FeedLine(curDA());
		dumpCodeBegin(pField, range);
		dumpCode2();
		return;
	}

	AttrIdEnum attr(pField->attrib());
	if (!CHECK_ATTR_TYPE(attr, __ATTR_BITSET))//not set while dumping a regular bitset
		if (iOwner && iOwner->typeBitset())
		{
			assert(IsBitvar(iOwner));
			SET_ATTR_TYPE(attr, __ATTR_BITSET);
		}

	if (!mpFrames->mbCollapsed && pField->isCollapsed())
	{
		if (targetRDA() > 0)
			throw (E_MISALIGNED_DATA);
		mrDumpTarget.SetLevelInfo(GetLevel(), adcui::ITEM_CLOSED);
		ROWID da(curDA());
		ROWID sz(SizeOf(iType, &da));//pField->type()?
		//ROWID sz(pField->size Bytes());
		if (sz != -1)
		{
			FeedLine(curDA(), (unsigned)sz);
			return;
		}
		attr = AttrIdEnum(attr | ATTR_COLLAPSED);//this is not a user attribute
		//FeedLine(curDA());//finalize the collapsed line
		DUMPframe_t saved(*mpFrames);
		saved.mbTemp = true;//do not update parent frame in destructor
		ROWID startDA(mCurrentDA);
		mpFrames->mbCollapsed = true;
//		EnterNoBreakSection();
//		EnterNoFlushSection();

		dumpType(iType, attr, range);

		saved.mbProblemLine = mpFrames->mbProblemLine;
		unsigned rowSpan(unsigned(mCurrentDA - startDA));
		
		mCurrentDA = startDA;
		*mpFrames = saved;
		FeedLine(curDA(), rowSpan, 0, false, false);//finalize the collapsed line
		//mpFrames->mRowSpan = 0;
//		LeaveNoFlushSection();
//		LeaveNoBreakSection();

	}
	else
		dumpType(/*SkipModifier*/(iType), attr, range);
}

void BinDumper_t::dumpType(CTypePtr t, AttrIdEnum attr, unsigned range)
{
	switch (t->ObjType())
	{
	/*case OBJID_TYPE_MODULE:
		dumpSeg2(t, t->typeModule()->dataSourceRef());
		break;
	case OBJID_TYPE_SEG:
		dumpSeg2(t, DataSubSource_t(data(), t->typeSeg()->rawBlock()));
		break;*/
	case OBJID_TYPE_PROC:
		dumpFunc2(t, range);
		break;
/*	case OBJID_TYPE_UNION:
#if(!NEW_LOCAL_VARS)
	case OBJID_TYPE_UNIONLOC:
#endif
		dumpUnion2(t);
		break;*/
	case OBJID_TYPE_STRUCVAR:
		dumpStrucvar(t, range);
		break;
	case OBJID_TYPE_ENUM:
		dumpEnum(t, attr, range);
		break;
	/*case OBJID_TYPE_BIT:
		dumpBits(t, 1);
		break;*/
	case OBJID_TYPE_BITSET:
		dumpBitset(t, range);
		break;
	case OBJID_TYPE_STRUC:
	case OBJID_TYPE_CLASS:
		if (t->typeUnion())
		{
			dumpUnion2(t, range);
			break;
		}
	case OBJID_TYPE_STRUCLOC:
	case OBJID_TYPE_FUNCDEF:
	case OBJID_TYPE_NAMESPACE:
		dumpStruc2(t, range);
		break;
	case OBJID_TYPE_ARRAY:
		dumpArray2(t, attr, range);
		break;
	case OBJID_TYPE_ARRAY_INDEX:
		dumpArrayIndex(t, attr, range);
		break;
	case OBJID_TYPE_PTR:
	case OBJID_TYPE_REF:
	case OBJID_TYPE_RREF:
	case OBJID_TYPE_VPTR:
	case OBJID_TYPE_THISPTR:
	case OBJID_TYPE_SIMPLE:
		dumpSimple2(t, attr, range);
		break;
	case OBJID_TYPE_CONST:
	case OBJID_TYPE_TYPEDEF:
		dumpType(t->typeModifier()->incumbent(), attr, range);
		break;
	case OBJID_TYPE_PROXY:
		dumpType(t->typeProxy()->incumbent(), attr, range);
		break;
	case OBJID_TYPE_IMP:
	case OBJID_TYPE_EXP:
		dumpType(t->baseType(), attr, range);
		break;
	default:
		assert(0);
	}
}

void BinDumper_t::dumpModule(CTypePtr pModule)
{
	const Module_t& rModule(*pModule->typeModule());
	size_t n(rModule.sections().find(mTargetDA));
CHECK(n == -1)
STOP
	const Section_t& aSect((n != -1) ? rModule.sections().get(n) : mScope);

	mCurrentDA = aSect.da;

	//DataSubSource_t data(rModule.dataSourceRef(), aSect.ra, aSect.size);
	
	DUMPframe_t frame(*this, rModule.dataSourceRef(), aSect.seg);
	InitFrame(frame, aSect, pModule);
	mpTopFrame = &frame;
	
	FrontPtr frontPtr(*this, FindFrontSegUp(aSect.seg));

	StrucDumpIter it0(*this, *aSect.seg);
	it0.initialise();
	StrucDumpIter it1(it0);
	if (it1)
		++it1;

	if (curDA() == mTargetDA)
	if (curDA() == aSect.da && aSect.off == 0)//begining of a segment
	{
		drawColumn(adcui::IBinViewModel::CLMN_NAMES, aSect.seg->typeSeg()->title());
		FeedLine(curDA());
	}

	try
	{
		dumpStrucIter(it0, it1, Range(frame));
	}
	catch (E_BreakStatus e)
	{
		if (e == E_PARTIAL_PAGE_DONE || e == E_PAGE_END || e == E_RANGE_OVER)
		{
			if (curRDA() == frame.miExtent)
			{
				assert(!frame.mbCollapsed);
				ROWID rda;
				CFieldPtr pField(it0.field(rda));
				if (pField)
				{
					mrDumpTarget.coldata_clear();
					mrDumpTarget.coldata_expand();
					dumpStrucEnd0(frame, pField);
				}
				if (e == E_RANGE_OVER)
					return;
			}
		}
		throw e;
	}
	ROWID rda;
	CFieldPtr pField(it0.field(rda));
	if (pField && !frame.mIncomplete)
		if (!frame.mbCollapsed)
			dumpStrucEnd0(frame, pField);
}

TypePtr BinDumper_t::GetCurrentSeg() const
{
	return mpFrames->miSeg;
	/*for (DUMPframe_t *p(mpFrames); p; p = p->mpUp)
	{
		TypePtr iType(p->cont());
		if (iType && iType->typeSeg())
			return iType;
	}
	return nullptr;*/
}

ADDR BinDumper_t::curVA() const
{
	ADDR va(ADDR(-1));
	GetVirtualAddr(curDA(), va);
	return va;
}

const I_DataSourceBase &BinDumper_t::GetDataSource() const
{
	assert(mpFrames->miModule);
		return mpFrames->miModule->typeModule()->dataSourceRef();
	//return nullptr;
}

bool BinDumper_t::GetRawAddr(ROWID da, OFF_t &ra) const
{
	if (RawBlock(*mpFrames).empty())
		return false;

	assert(da >= mpFrames->mBaseDA);
	ROWID rda(da - mpFrames->mBaseDA);
	OFF_t off(mpFrames->lower() + rda);
	ra = GetDataSource().pos(off);//RawOffs(*mpFrames)
	assert(ra == data().pos(off));
	return true;
}

TypePtr BinDumper_t::GetVirtualAddr(ROWID da, ADDR& addr) const
{
	const DUMPframe_t& top(*mpTopFrame);
	ADDR off(ADDR(da - top.mBaseDA));//offset in current section
	off += top.offset();//offset in segment
	if ((ADDR)top.cont()->size() < off)
		return nullptr;
	addr = top.cont()->base() + off;
	return top.cont();
}

void BinDumper_t::advance(unsigned deltaDA, unsigned /*deltaRA*/)
{
	mCurrentDA += (ROWID)(signed)deltaDA;
}

unsigned BinDumper_t::advanceBits(unsigned d)
{
	unsigned rda(mpFrames->advanceBits(d));
	mCurrentDA += rda;
	return rda;
}

ROWID BinDumper_t::targetRDA() const
{
	ROWID da(mpFrames ? curDA() : mScope.da);
	if (da > mTargetDA)
		return 0;
	return mTargetDA - da;
}

int BinDumper_t::GetLevel()
{
	return mpFrames->level();
}

/*void DumpTarget_t::drawRowSpan(unsigned rowSpan)
{
	char buf[12];
	sprintf(buf, "%d", rowSpan);
	ColData_t &dat(coldata(adcui::IBinViewModel::CLMN_ROWSPAN));
	dat.append(buf);
}*/

void BinDumper_t::flushRow(bool bProblem)
{
	/*ROWID rowid(mrDumpTarget.coldata().da2);
	ROWID rowid0(mrDumpTarget.coldata().da);
	//ROWID rowid(rowid0);
	unsigned rowSpan(mrDumpTarget.coldata().span);
	assert(rowid0 != ROWID_END);
	ROWID row_end(rowid0 + rowSpan);
	assert(row_end != ROWID_END);*/
	unsigned forceRange(mpFrames->mpDumpingUnion ? mpFrames->mpDumpingUnion->miExtent : 0);

	if (mpFrames->mIncomplete > 0 || mpFrames->mbProblemLine)
		bProblem = true;
	mpFrames->mbProblemLine = false;//reset
	ROWID row_beg;
	ROWID row_end(mrDumpTarget.flushRow(&row_beg, bProblem, forceRange));
	//mrDumpTarget.expandColDataVec();

	if (mpFrames->mbCollapsed)
		return;

	// Check the end of the page.
//	if (mnLinesLeft > 0)
	if (row_beg >= mnStart0)
		mnLinesLeft--;

	//if (row_end >= mEndDA)
		//Break(E_RANGE_OVER);//scope end

	if (mpFrames->mbDumpingSeg)
			if (row_end >= mpFrames->mBaseDA + mpFrames->miExtent)
			{
				Break(E_RANGE_OVER);//scope end
				return;//no need to check other break conditions
			}

	if (IsProc(mpFrames->cont()))
		if (mpFrames->miExtent != -1)
			if (row_end >= mpFrames->mBaseDA + mpFrames->miExtent)
			{
				Break(E_RANGE_OVER);//scope end
				return;//no need to check other break conditions
			}

	assert(mbFreezePageBreak || row_end <= mrDumpTarget.upperLimit());
	if (row_end >= mrDumpTarget.upperLimit())//finish lines for the current row_end
		Break(E_PARTIAL_PAGE_DONE);//partial page done

	if (mnLinesLeft <= 0)
		if (row_end - row_beg > 0)//allow full row lines processed
			Break(E_PAGE_END);//full page done
}

void BinDumper_t::adjustLevelInfo()
{
	//store level info string
	int levelr = GetLevel() - 1;
	for (DUMPframe_t* pFrame(mpFrames); pFrame && pFrame->mpUp; pFrame = pFrame->mpUp)
	{
		if (levelr < 0)
			break;
		bool bBitset(pFrame->cont()->typeBitset() != nullptr);
		uint8_t n(mrDumpTarget.GetLevelInfo(levelr));
		if (!bBitset)
			if (!(n & (~adcui::ITEM_TREE_MASK)))
				n |= lineColor(pFrame);
		if (pFrame->mNextChildRDA > 0)
			n |= adcui::ITEM_HAS_NEXT_CHILD | adcui::ITEM_HAS_CHILD_HERE;
		//levelInfo += (char)('0' + n);
		mrDumpTarget.SetLevelInfo(levelr, n);
		if (!bBitset)
			levelr--;
	}
}

void BinDumper_t::dumpCellDA(ROWID da, unsigned rdaBits)
{
	mrDumpTarget.drawRowID(da, rdaBits, miRowIDLenMax);
}

void BinDumper_t::dumpCellRA(ROWID da)
{
	OFF_t ra(0);
	if (GetRawAddr(da, ra))
		mrDumpTarget.drawAddressR(ra);
}

void BinDumper_t::dumpCellVA(ROWID da)//int rowShift)
{
	ADDR va(0);// = -(int)mRowSpan;
	TypePtr iSeg(GetVirtualAddr(da, va));
	if (iSeg && iSeg->typeSeg()->subsegs().empty())//display VA only for terminal segs (there is only column for VA!))
	{
		TypePtr iSeg2(GetCurrentSeg());
		if (iSeg2)
		{
			CTypePtr pModule(mpFrames->miModule);
			//if (IsPhantomModule(pModule))
			if (iSeg2->typeSeg()->isPhantom())
				mrDumpTarget.drawOrdinal(iSeg2, va);// +rowShift);
			else
				mrDumpTarget.drawAddressV(iSeg2, va);// +rowShift);
		}
	}
}

void BinDumper_t::FeedLine(ROWID rowid0, unsigned rowSpan0, int rowShift, bool bBitsMode, bool bNoAdvance)
{
CHECK(rowid0 == 0x3550)
STOP
	unsigned rowSpan(rowSpan0);
	//ROWID rowid0(curDA());
	ROWID rowid(rowid0 + rowShift);//this one is real

	if (!bBitsMode)
	{
		if (targetRDA() > 0 && (rowid + rowSpan < mbDumpFrom))//(mbDumpPriorLines == 0))
		{
			if (!mbFreezePageBreak)
				if (!mbDisableLineSkip)
					return;
		}

		//CHECK(rowid == 0xD281)
		//STOP

		if (rowid < mnStart0 && (rowid + rowSpan < mbDumpFrom))//(mbDumpPriorLines == 0))
		{
			if (!mbFreezePageBreak)
				if (!mbDisableLineSkip)
					return;
		}
	}

	if (mrDumpTarget.coldata_empty())
		return;

	DumpTarget_t::MyColData &coldata(mrDumpTarget.coldata());

	if (mpFrames->mbCollapsed && !isDumpingStrucvar())
	{
		if (rowSpan > 0)
		{
			if (bBitsMode)
				rowSpan = advanceBits(rowSpan);//arg is in bits, returns bytes
			else
				advance(rowSpan, rowSpan);
			mbDisableLineSkip = false;
		}
		return;
	}

	unsigned rdaBits(bBitsMode ? mpFrames->mBitsRDA : 0);

	int baseOffset((int)curRDA());

	if (!mpFrames->mbCollapsed)
	{
		int swapShift(0);
		if (bBitsMode && mpFrames->mbMSB)
		{
			int bitsetSize((int)mpFrames->miExtent);
			assert(bitsetSize == mpFrames->cont()->sizeBytes());
			assert(baseOffset < bitsetSize);
			baseOffset = bitsetSize - baseOffset - 1;//now it is adjusted
			swapShift = baseOffset - (int)curRDA();
		}

		dumpCellDA(rowid + swapShift, rdaBits);
		adjustLevelInfo();
		dumpCellRA(rowid + swapShift);// curDA() + rowShift);
		dumpCellVA(rowid + swapShift);// curDA() + rowShift);
	}

	if (rowSpan > 0)
	{
		if (bBitsMode)
		{
			unsigned bytesOut(1);
			rowSpan = advanceBits(rowSpan);//arg is in bits, returns bytes
			if (rowSpan > 0)
			{
				bytesOut = rowSpan;
				if (mpFrames->mBitsRDA > 0)
					bytesOut++;//one more byte
				mbDisableLineSkip = false;
			}
			if (!RawBlock(*mpFrames).empty())
			{
				OFF_t off(mpFrames->lower() + baseOffset);
				if (mpFrames->mbMSB)
					off -= (bytesOut - 1);//always display bytes in order they appear
				mrDumpTarget.drawBytes(GetDataSource(), off, bytesOut);
			}

		}
		else
		{
			if (!bNoAdvance)
				advance(rowSpan, rowSpan);
			mbDisableLineSkip = false;
		}
	}

	if (!mpFrames->mbCollapsed)
	{
		coldata.da = rowid0;
		if (mpFrames->mpDumpingUnion)
			coldata.da = mpFrames->mpDumpingUnion->mBaseDA;
		coldata.da2 = rowid;
		coldata.span += rowSpan;
		coldata.bit = rdaBits;
	}

	bool bTooManyLines(false);
	if (!mrDumpTarget.isFullMode() && mrDumpTarget.coldata_size() >= STRUCVAR_INSTANTIATION_LIMIT && rowSpan > 0)//unions? bitsets?
	{
		if (!mpFrames->mbCollapsed)
		{
			ColData_t &aDat(mrDumpTarget.coldata(adcui::IBinViewModel::CLMN_COMMENTS));
			aDat.writestr("... too many lines!", safeColor(adcui::COLOR_ERROR));
		}
		bTooManyLines = true;
	}

	if (mbFreezeFlush == 0)
		if (!mbDisableLineSkip)
			flushRow();
	//else
//	if (!bNoColDataExtent)
	if (bTooManyLines)
		throw E_TOO_MANY_LINES;
	mrDumpTarget.coldata_expand();
}

class MyDataStreamBase : public I_DataStreamBase
{
	const BinDumper_t &mr;
public:
	MyDataStreamBase(const BinDumper_t &r)
		: mr(r)
	{
	}
protected:
	virtual POSITION setcp(POSITION){
		assert(0); return 0;
	}
	virtual POSITION cp() const {
		ADDR va(0);
		if (!mr.GetVirtualAddr(mr.curDA(), va))
			throw(-1);
		return (POSITION)va;
	}
	virtual OFF_t cpr() const {
		OFF_t ra(0);
		if (!mr.GetRawAddr(mr.curDA(), ra))
			throw(-1);
		return (OFF_t)ra;
	}
	virtual unsigned skip(int){
		assert(0); return 0;
	}
	virtual unsigned skipBits(int){
		assert(0); return 0;
	}
	virtual POSITION align(unsigned){
		assert(0); return 0;
	}
	virtual size_t dataAt(OFF_t off, OFF_t size, PDATA pDest) const {
		return module().dataAt(off, size, pDest);
	}
	virtual OFF_t size() const { 
		return mr.mpFrames->m_size; }
	virtual const I_AuxData *aux() const {
//???		assert(0);
		/*DataPtr pData(module().dataSourcePtr());
		if (!pData)
			return nullptr;
		return pData->pvt().aux();*/
		return nullptr;
	}
private:
	const Module_t &module() const {
		return *mr.mpFrames->miModule->typeModule();
	}
};

unsigned BinDumper_t::drawValueEx(const value_t &v, uint8_t optyp0, AttrIdEnum attr, MyStrStream &os) const
{
	//MyString sLoc;
	IOutpADDR2Name::out_t dot;
	if (OPTYP_IS_PTR(optyp0))
	{
//		int sz(0);
		TypePtr iSeg(GetCurrentSeg());
		Seg_t *pSeg(iSeg->typeSeg());
		if (pSeg->isLarge())
		{
			ADDR64 imageBase(pSeg->imageBase(iSeg));
			if (v.ui64 >= imageBase)//can be a wrong pointer
			{
				ADDR64 d(v.ui64 - imageBase);
				if (d < 0x100000000)
					decode(ADDR(d), false, dot);
			}
		}
		else
			decode(v.ui32, false, dot);
	}
	if (!dot.name.empty())
	{
		os << "offset " << dot.name;
		return 0;//colornum?
	}

	return drawValue(v, optyp0, attr, os);
}

void BinDumper_t::drawValueRef(const value_t &v, AttrIdEnum attr)
{
	if (MISC_ATTR_CHECK_RANGE(attr) && v.ui64 != 0)
	{
		MyDataStreamBase LI(*this);
		CTypePtr iFrontSeg(mFrontSegs.empty() ? nullptr : mFrontSegs.back());
		if (iFrontSeg)
		{
			FRONT_t *pFRONT(FrontAt(iFrontSeg->typeSeg()->frontIndex()));
			CTypePtr iSegThis(mpFrames->miSeg);
			AttrIdEnum attr2(attr);
			ADDR va(v.ui32);
			if (iSegThis->typeSeg()->isLarge() && attr == ATTR_VA)
			{
				ADDR64 imageBase(iSegThis->typeSeg()->imageBase(iSegThis));
				va = ADDR(v.ui64 - imageBase);
				attr2 = ATTR_RVA;
			}
			CTypePtr iModule(mpFrames->miModule);
			const Module_t &aModule(*iModule->typeModule());
			ModuleInfo_t MI(*this, *iModule);
			I_Front::AKindEnum iKind(pFRONT->device(&GetDataSource())->translateAddress(LI, aModule.moduleTag(), va, attr2));
			CTypePtr iSeg2(nullptr);
			if (iKind == I_Front::AKIND_RAW)
			{
				OFF_t ra(va);
				iSeg2 = MI.FindSegFromRA(ra, iModule);
				if (iSeg2)
					va = iSeg2->base() + ADDR(ra - MI.rawOffs(iSeg2));//ra->va
			}
			else if (iKind == I_Front::AKIND_VA)
				iSeg2 = MI.VA2Seg(iSegThis, va);

			if (iSeg2)
			{
				Locus_t aLoc;
				MI.FindFieldInSeg(iSeg2, va, aLoc);
				if (!aLoc.empty())
				{
					unsigned off;
					CFieldPtr pField0(aLoc.field0());
					CFieldPtr pField(aLoc.stripToSeg(off));
					//if (aLoc.stripToSeg() && aLoc.offs() == 0)
					if (pField || aLoc.offs() == 0)
					{
						MyStrStream os;
						if (pField && (!pField0 || (pField == pField0 || off == 0)) && pField->name())
						{
							if (aLoc.seg() != iSegThis)
								os << aLoc.seg()->typeSeg()->title() << ".";
							drawName2(pField, os);
							if (off > 0)
								os << MyStringf("+%X", off);
						}
						else if (off == 0 && aLoc.offs() == 0)
							os << aLoc.seg()->typeSeg()->title();
						MyString s;
						os.flush(s);
						if (!s.empty())
							mrDumpTarget.drawComment(s);
					}
				}
			}
		}
	}
}

static DumpUtil_t::OutFormat toOutFormat(AttrIdEnum attr)
{
	switch (attr)
	{
	default: break;
	case ATTR_HEX: return DumpUtil_t::OUT_HEX;
	case ATTR_DECIMAL: return DumpUtil_t::OUT_DECIMAL;
	case ATTR_BINARY: return DumpUtil_t::OUT_BINARY;
	case ATTR_OCTAL: return DumpUtil_t::OUT_OCTAL;
//	case ATTR_ ASCII: return DumpUtil_t::OUT_ASCII;
	}
	return DumpUtil_t::OUT_NULL;
}

static int toOutColor(AttrIdEnum attr, uint8_t typ)
{
	int iColor(-1);
	if (MISC_ATTR_CHECK_RANGE(attr))
	{
		iColor = MISC_ATTR_TO_COLOR(attr);
		assert(iColor <= adcui::COLOR_ATTRIB_END);
	}
	else if (OPTYP_IS_PTR(typ))
		iColor = adcui::COLOR_DASM_ADDRESS;
	else if (typ)
		iColor = adcui::COLOR_DASM_NUMBER;
	return iColor;
}

unsigned BinDumper_t::drawValue(const value_t &v, uint8_t optyp0, AttrIdEnum attr, MyStrStream &os) const
{
	int colornum(0);
	DumpUtil_t outp(os);
	outp.out_value(VALUE_t(optyp0, v), os, toOutColor(attr, optyp0), toOutFormat(attr));
	if (mrDumpTarget.isColorsEnabled())
		colornum += 2;//number of extra bytes written by coloring
	return colornum;
}

static bool checkEol(char ch, DataStream_t &is)
{
	if (ch == 0xA)
		return true;//both Win and Unix text lines terminate in 0xA
	if (ch != 0xD)
		return false;
	//check if we've got a Mac format (0xDs only). Gotta check if the next symbol is not '0xA', meaning it could be the Win one.
	if (is.peek<char>() == 0xA)//win
		return false;
	return true;
}

static IDStr_t DataSizeStr[] = {
	{OPSZ_BYTE,			"DB"}, 
	{OPSZ_WORD,			"DW"}, 
	{OPSZ_DWORD,		"DD"}, 
	{OPSZ_FWORD,		"DF"},
	{OPSZ_QWORD,		"DQ"}, 
	{OPSZ_TWORD,		"DT"},
	{OPSZ_NULL,			"?"}
};

static char * STR_DATASZ(int datasz)
{
	static char str[3];
	const char *p = DataSizeStr->GetStr(datasz);
	if (!p)
		return 0;
	strncpy(str, p, sizeof(str));
	return str;
}

static void enumOpen(MyStrStream &os)
{
	os.color(adcui::COLOR_DASM_UNK_NAME);
	os << "{";
}

static void enumClose(MyStrStream &os)
{
	os << "}";
	os.color(adcui::COLOR_POP);
}

unsigned BinDumper_t::dumpDataRow(DataStream_t* pis, uint8_t optyp0, AttrIdEnum attr, size_t &arrIndex, size_t &arr, bool bSwapBytes, CTypePtr iType)
{
	uint8_t optyp(optyp0);
	if (optyp == OPTYP_NULL)
		optyp = OPSZ_BYTE;

	size_t arr0(arr);
	int size0(optyp & OPSZ_MASK);
	assert(size0 <= OPSZ_QWORD);

	TypeEnum_t *pEnum(iType ? iType->typeEnum() : nullptr);
	bool bIsByte(size0 == OPSZ_BYTE);
	bool bIsWord(size0 == OPSZ_WORD);
	//bool bDoAscii((attr == ATTR_ ASCII || attr == ATTR_ASCII_TEXT) && bIsByte);
	bool bDoAscii(optyp == OPTYP_CHAR8);
	int bDoUnicode(optyp == OPTYP_CHAR16 ? 1 : 0);
	bool bText(attr == ATTR_ASCII_TEXT);
	int iColorStr(safeColor(adcui::COLOR_DASM_ASCII));
	if (bIsWord)
		/*if (attr == ATTR_UNICODE)
		{
			bDoUnicode = 1;
			//iColorStr = safeColor(adcui::COLOR_WSTRING);
		}
		else */if (attr == ATTR_NUNICODE)
		{
			bDoUnicode = 2;
			//iColorStr = safeColor(adcui::COLOR_WSTRING);
		}
	//int old_col = set_color(COLOR_DASM_CODE);

	ColDesc_t &aCol(mrDumpTarget.Column(adcui::IBinViewModel::CLMN_DATA));
	ColData_t &aDat(mrDumpTarget.coldata(adcui::IBinViewModel::CLMN_DATA));
	aDat.reserve(256);

	MyStrStream os(mrDumpTarget.isColorsEnabled());
#if(1)
	//	if (!(mrDumpTarget.Column(adcui::IBinViewModel::CLMN_TYPES).width > 0))
	{
		if (!mrDumpTarget.isValuesOnlyMode() && iType)
		{
			CTypePtr iType2(pEnum ? pEnum->baseType() : iType);
			assert(iType2);
			if (!iType2->nameless())
			{
				dumpNameRefScoped(iType2, os);
				os.flush(aDat);
				aDat.append(" ");
			}
		}

		aDat += STR_DATASZ(size0);
		aDat.append(" ");
	}
#else
	if (!iType)
		aDat += STR_DATASZ(size0);
	if (!aDat.empty())
		aDat.append(" ");
#endif

	std::string buf;
	int nbuf(0);
	int colornum(0);
	int colw(aCol.width);
	for (int count(0); arr > 0; count++, arrIndex++)
	{
		value_t v;

		if (pis)
		{
			v.clear();
			pis->read(size0, (PDATA)&v.i8);
			mrDumpTarget.drawBytes(&v, size0);
		}
		else if (arr > 1)
		{
			os << arr << " dup(?)";
			os.flush(aDat);
			arrIndex += arr;
			arr = 0;
			return (unsigned)arr0;
		}

		--arr;

		bool bIsChar(bIsByte && isprint(v.ui8));
		if (bIsChar && arr == 0)
		{
			char c((char)v.ui8);
			mrDumpTarget.drawComment(MyStringf("'%c'", c), true);//if empty
		}

		if (bDoAscii && bIsChar && !pEnum)
		{
			char c((char)v.ui8);
			buf += c;
			if (nbuf++ == 0 && count > 0)
				aDat.append(", ");
		}
		else
		{
			bool bUnicodeStr(bDoUnicode == 1 || (bDoUnicode == 2 && arrIndex > 0));//arrIndex
			bool bIsWChar(bIsWord && v.ui16 < 0x0100 && isprint(v.ui8));
			if (bUnicodeStr && iColorStr && v.ui16)//colors enabled, terminated NUL not included
			{
				buf.append((const char *)&v.ui16, sizeof(char16_t));
				if (nbuf++ == 0 && count > 0)
					aDat.append(", ");
			}
			else if (bUnicodeStr && bIsWChar)
			{
				//char16_t wc((char16_t)v.ui16);
				buf.append((const char *)&v.ui16, sizeof(char));
				if (nbuf++ == 0 && count > 0)
					aDat.append(", ");
			}
			else
			{
				if (nbuf == 0)
				{
					if (count > 0)
						aDat.append(", ");
				}
				else// if (nbuf > 0)
				{
					//flush ascii/unicode string
					aDat.writestr(buf, bDoUnicode != 0, iColorStr);
					nbuf = 0;
					buf.clear();
					aDat.append(", ");
				}

				if (pis)
				{
					if (bSwapBytes)
						swap_endian(&v.ui8, size0);

					FieldPtr pField(nullptr);
					if (pEnum)
						pField = Field(pEnum->enumRef(), v.ui32, 0, FieldIt_Exact);
					AttrIdEnum attr2(/*pField ? ATTR_NULL :*/ AttrIdEnum(attr & ATTR__MASK));
					//MyStrStream os(mrDumpTarget.isColorsEnabled());
					colornum += drawValueEx(v, optyp0, attr2, os);
					if (pField)
					{
						os << " ";
						enumOpen(os);
						dumpNameRefScoped(pField, os, pEnum->enumRef());//colornum?
						enumClose(os);
					}
					os.flush(aDat);

					if (arr0 == 1)
					{
						try {//?check: elf/Hopper
							drawValueRef(v, attr2);
						}
						catch (...)
						{
						}
					}
				}
				else
				{
					aDat.append("?");
				}
			}
		}

		if (!bText)
		{
			if ((int)aDat.length() - colornum >= colw)
				break;
		}
		else
			if (bIsByte && checkEol(v.i8, *pis))
				break;
	}

	if (nbuf > 0)
		aDat.writestr(buf, bDoUnicode != 0, iColorStr);

	return (unsigned)(arr0 - arr);
}

void BinDumper_t::dumpUnk2(unsigned num, bool bGap)
{
	if (num == 0)
		num = OPSZ_BYTE;

	if (bGap)
	{
		if (targetRDA() > 0)
		{
			assert(!isDumpingStrucvar());
			throw E_MISALIGNED_GAP;
		}

		drawRowExtra(mpFrames->cont(), curVA(), num);
		if (!mpFrames->mbCollapsed)
		{
			ColData_t &aDat(mrDumpTarget.coldata(adcui::IBinViewModel::CLMN_DATA));
			MyString s;
			if (num > 1)
				s = MyStringf("; gap %d bytes", num);
			else
				s = "; gap 1 byte";
			aDat.writestr(s, safeColor(adcui::COLOR_UNEXPLORED));
		}
		mbDisableLineSkip = true;
		FeedLine(curDA(), num);
		return;
	}

	int i(0);
	if (!isDumpingStrucvar())//cannot skip while dumping a strucvar
	{
		if (targetRDA() > 0)
		{
			assert((int)targetRDA() <= num);
			//i = (int)std::min((ROWID)num, mSta rtRow);
			i = (int)targetRDA();
			advance(i, i);
			//mSt artRow = 0;
		}

		ROWID rowID(curDA());
		if (rowID < mnStart0)
		{
			unsigned d(unsigned(mnStart0 - rowID));
			if (d > num)
				d = num;
#if(0)
			if (d >= num)
				d = num - 1;
#endif
			advance(d, d);
			i = d;
			//mrDumpTarget.setRowID(RowID()-1);
			//mrDumpTarget.setRowID(-1);
		}
	}
	dumpUnk0(num, i);
}

void BinDumper_t::dumpUnk0(unsigned num, unsigned start)
{
	if (mpFrames->mbCollapsed)
	{
		FeedLine(curDA(), num - start);
		return;
	}

#if(SQUEEZE_STRUCVAR_GAPS)
	if (isDumpingStrucvar())
	{
		dumpUnk2(num, true);
		return;
	}
#endif

	for (unsigned i(start); i < num; ++i)
	{
		size_t iarr(0), arr(1);
		unsigned bytes;
		if (!RawBlock(*mpFrames).empty())
		{
			DataSubSource_t data(GetDataSource(), RawBlock(*mpFrames));
			DataStream_t is(data);
			bytes = dumpDataRow(&is, OPSZ_NULL, ATTR_NULL, iarr, arr, false);
		}
		else
		{
			bytes = dumpDataRow(nullptr, OPSZ_NULL, ATTR_NULL, iarr, arr, false);
		}
		FeedLine(curDA(), bytes);
	}
}

bool BinDumper_t::decode(ADDR addr, bool /*bFunc*/, IOutpADDR2Name::out_t &dot) const
{
	ModuleInfo_t MI(mrProject, *mpFrames->miModule);

	FieldPtr f(MI.FindFieldInSubsegs(GetCurrentSeg(), addr));
	if (f)
	{
		//if (bFunc && !IsFunc(f->type()) && IsFunc(f->OwnerStruc()))
			//f = f->owner()->parentField();

		MyStrStream os;
		drawName2(f, os);
		os.flush(dot.name);

		if (f->type() && f->type()->typeSimple())
			dot.sz = f->OpSize();
		else
			dot.sz = 1;
		return true;
	}
	return false;
}

int BinDumper_t::safeColor(int iColor) const
{
	if (mrDumpTarget.isColorsEnabled())
		return iColor;
	return 0;
}

class RAII_NoBreakSection
{
	BinDumper_t &mr;
public:
	RAII_NoBreakSection(BinDumper_t &r)
		: mr(r)
	{
		mr.EnterNoBreakSection();
	}
	~RAII_NoBreakSection()
	{
		mr.LeaveNoBreakSection();
	}
};

class RAII_FrameIncomplete
{
	DUMPframe_t &mrFrame;
	bool bIncomplete;
public:
	RAII_FrameIncomplete(DUMPframe_t &r)
		: mrFrame(r),
		bIncomplete(false)
	{
	}
	~RAII_FrameIncomplete()
	{
		if (bIncomplete)
			mrFrame.mIncomplete--;
	}
	void setIncomplete()
	{
		mrFrame.mIncomplete++;
		bIncomplete = true;
	}
};

void BinDumper_t::dumpSimple2(uint8_t optyp, AttrIdEnum attr, size_t arrIndex, size_t arr0, unsigned range, CTypePtr iType)
{
	//ColDesc_t &aCol(mrDumpTarget.Column(adcui::IBinViewModel::CLMN_DATA));
	uint8_t size0(optyp & OPSZ_MASK);

	bool bSwapBytes = mpFrames->mbMSB != 0;

	TypePtr iSeg(GetCurrentSeg());
	assert(!iSeg || iSeg->typeSeg()->isLittleEnd() != bSwapBytes);

	Block_t aRaw(RawBlock(*mpFrames));
	if (!aRaw.empty())
	{
		size_t rem(0);
		size_t arr(arr0);
		RAII_FrameIncomplete RAII_FI(*mpFrames);
		if (size0 * arr > range)
		{
			arr = range / size0;
			rem = range - arr * size0;
			RAII_FI.setIncomplete();
			mpFrames->mIncomplete++;
		}

		if (mpFrames->mbCollapsed)
		{
			FeedLine(curDA(), (unsigned)(size0 * arr));
			return;
		}

		DataSubSource_t data(GetDataSource(), aRaw);
		DataStream_t is(data);
		{
			//RAII_NoBreakSection RAII_NB(*this);//entering no-break section (yet can be break by too-many-lines)
			try
			{
				EnterNoBreakSection();
				size_t n(arr);
				for (; n > 0;)
				{
					size_t total(dumpDataRow(&is, optyp, attr, arrIndex, n, bSwapBytes, iType));
					if (n == 0 && arr < arr0)
					{
						ColData_t& aDat(mrDumpTarget.coldata(adcui::IBinViewModel::CLMN_COMMENTS));
						aDat.writestr(MyStringf(" ... %d more!", int(arr0 - arr)).c_str(), safeColor(adcui::COLOR_ERROR));
					}
					FeedLine(curDA(), (unsigned)(size0 * total));
				}
				if (rem > 0)
				{
					assert(rem < size0);
					int x((int)rem);
					uint8_t size2(uint8_t(x & -x));//Highest Power of 2 That Evenly Divides a Number
					n = rem / size2;
					size_t total(dumpDataRow(&is, size2, attr, arrIndex, n, bSwapBytes, iType));
					(void)total;
					FeedLine(curDA(), (unsigned)(rem));
				}
				if (arr < arr0)
					throw E_RANGE_OVER;
				//throw (-1);//force it enterring a catch block
			}
			catch (E_BreakStatus e)
			{
				LeaveNoBreakSection();//may throw
				throw(e);
			}
			LeaveNoBreakSection();//may throw
		}
		return;
	}
	
	int row(0);
	size_t iarr(0), arr(arr0);
	if (!IsPowerOfTwo(size0))
	{
		arr *= size0;
		size0 = OPSZ_BYTE;
		iType = nullptr;
	}
	while (arr > 0)
	{
		size_t total(dumpDataRow(nullptr, size0, attr, iarr, arr, bSwapBytes, iType));
		FeedLine(curDA(), (unsigned)(size0 * total));
		row++;
	}
}

void BinDumper_t::dumpEnum(CTypePtr iSelf, AttrIdEnum attr, unsigned range)
{
	dumpSimple2(iSelf, attr/*SET_ATTR_TYPE(attr, __ATTR_ENUM)*/, range);
}

void BinDumper_t::dumpSimple2(CTypePtr iSelf, AttrIdEnum attr, unsigned range)
{
	if (CHECK_ATTR_TYPE(attr, __ATTR_BITSET))
	{
		CLEAR_ATTR_TYPE(attr, __ATTR_BITSET);
		dumpBits(iSelf, attr, 1);
		return;
	}

	Simple_t &rSimple(*iSelf->typeSimple());
	if (targetRDA() > 0)//must have skipped fields at lower offsets
		if (mbFreezePageBreak == 0 || rSimple.size(iSelf) > (int)targetRDA())
			throw (E_MISALIGNED_DATA);
	if (unsigned(iSelf->size()) > range)
	{
		mpFrames->mIncomplete++;
		dumpUnk2(range, mrDumpTarget.isCompactMode());
		throw E_FIELD_OVERLAP;
		//return;
	}
	uint8_t optyp(rSimple.optype());
	if (rSimple.typePtr())
		optyp = MAKETYP_PTR(rSimple.opsize());
	//CTypePtr iType(CHECK_ATTR_TYPE(attr, __ATTR_ENUM) ? iSelf : nullptr);
	dumpSimple2(optyp, attr, 0, 1, range, iSelf);//iType);
}

/*class FieldTmpArr_t : public FieldTmp_t//a wrapper to reset field's type if exception is thrown in client code
{
	int index;//array index
	MyString	buf;
	NameRef_t	name;
	TypePtr		iIndexRef;
public:
	FieldTmpArr_t(TypePtr pType, int i, TypePtr indexRef)
		 : FieldTmp_t(pType),
		 index(i),
		 iIndexRef(indexRef)
	{
		setName(&name);
		update();
	}
	~FieldTmpArr_t()
	{
		name.setPvt0(nullptr);
		setName(nullptr);
	}
	void increment(){ index++; update(); }
private:
	void update()
	{
		if (iIndexRef)
		{
			TypeEnum_t *pEnum(iIndexRef->typeEnum());
			FieldPtr pField(ProjectInfo_t::Field(pEnum->enumRef(), index, 0, FieldIt_Exact));
			if (pField)
			{
				buf.sprintf("[%d|%s]", index, pField->name()->c_str());
				//dumpNameRefScoped(pField, os);//colornum?
				name.setPvt0((char *)buf.c_str());
				return;
			}
		}
		buf.sprintf("[%d]", index);
		name.setPvt0((char *)buf.c_str());
	}
};*/

#define DUMP_GRANULARITY	1024//so much elements dumped at one pass (to avoid heavy loops)
#define WHOLE_ARRAYS 0

void BinDumper_t::dumpArray2(CTypePtr iSelf, AttrIdEnum attr, unsigned maxRange, CTypePtr iIndexRef)
{
	assert(iSelf->typeArray());
	Array_t &rArray(*iSelf->typeArray());
	if (CHECK_ATTR_TYPE(attr, __ATTR_BITSET))
	{
		CLEAR_ATTR_TYPE(attr, __ATTR_BITSET);
		dumpBits(rArray.baseType(), attr, (unsigned)rArray.total());
		return;
	}

	if (mpFrames->mbCollapsed)
	{
		FeedLine(curDA(), iSelf->size(), 0, false);
		return;
	}

	TypePtr pType(rArray.baseType());
	unsigned baseSize = pType->size();
	if (baseSize == 0)
		baseSize = OPSZ_BYTE;

	if (pType->typeSimple() && !iIndexRef)
	{
		bool bWholeArrays(isDumpingStrucvar() || WHOLE_ARRAYS);
		if (bWholeArrays)
		{
#if(WHOLE_ARRAYS)
			ROWID range(targetRDA());
			if (range > 0)
				throw (E_MISALIGNED_ARRAY);//array misalign
#endif
			uint8_t optyp(pType->typeSimple()->optype());
			size_t total(rArray.total());
			dumpSimple2(optyp, attr, 0, total, maxRange, pType);
			return;
		}
//		if (attr & ATTR_COLLAPSED)
		{
			attr = (AttrIdEnum)(attr & ATTR__MASK);//clear special attributes
			ROWID range(targetRDA());
			uint8_t optyp(pType->typeSimple()->optype());
			size_t total(rArray.total());
			if (total == 0)
				total = OPSZ_BYTE;
			unsigned dumpChunk(baseSize * DUMP_GRANULARITY);//bytes
			unsigned startRDA((unsigned)(range / dumpChunk) * dumpChunk);
			advance(startRDA, startRDA);
			if (targetRDA() > 0)
			if (mbFreezePageBreak == 0 || total*optyp > targetRDA())
				throw (E_MISALIGNED_ARRAY);//array misalign
			//assert(curDA() <= mTargetDA);
			size_t startIndex((size_t)range / baseSize);
			startIndex = (startIndex / DUMP_GRANULARITY) * DUMP_GRANULARITY;//adjusted
			size_t total0(total);
			total = std::min((size_t)DUMP_GRANULARITY, total0 - startIndex);
			dumpSimple2(optyp, attr, startIndex, total, maxRange, pType);
			if (startIndex + total < total0)
				Break(E_PARTIAL_PAGE_DONE);
			return;
		}
	}

	size_t left(rArray.total());
	unsigned range = std::min((unsigned)(left * baseSize), maxRange);

	DUMPframe_t frame(*this, data(), iSelf);
	InitFrame(frame, range);

	if (targetRDA() == 0)
		setLevelInfo(adcui::ITEM_HAS_NEXT_CHILD);

	FeedLine(curDA());

	int index = 0;
	if (!isDumpingStrucvar())
	{
		if (targetRDA() > 0)
		{
			//CHECK(!((int)targetRDA() < left * baseSize))
			//STOP
			unsigned tgt((unsigned)targetRDA());
			if (isDumpingStrucvar())//strucvars are dumpt whole, so target rda can fall behind
			{
				assert(0);//?
				if (!(tgt < (int)range))
					tgt = range;
			}
			else
			{
				//assert(tgt < range);
				if (!(tgt < (int)range))
					tgt = range;
			}

			div_t n(div((int)tgt, baseSize));
			unsigned delta(n.quot * baseSize);
			advance(delta, delta);
			left -= n.quot;
			index += n.quot;
			//mS tartRow = n.rem;
		}
	}
	else
	{
		STOP
	}

	//FieldTmpArr_t f(pType, index, iIndexRef);
	//f.setAttribute(attr);
	while (left--)
	{
		mpFrames->mNextChildRDA = (left > 0) ? curRDA() + baseSize : 0;
		unsigned range2(std::min(Range(*mpFrames), baseSize));
		if (range2 < baseSize)
		{
			mpFrames->mIncomplete++;
			dumpUnk2(range2, mrDumpTarget.isCompactMode());
			throw E_FIELD_OVERLAP;
		}

		TypePtr pType2(SkipModifier(pType));

		bool bSkipLine(targetRDA() > 0 && curDA() < mbDumpFrom);
		if (!bSkipLine)
		{
			//if (mpFrames->mpUp)
				drawChildHere(lineColor(mpFrames));
			
			//draw type
			MyStrStream os(mrDumpTarget.isColorsEnabled());
			if (!mrDumpTarget.isValuesOnlyMode())
				if (pType2->typeComplex() && !pType2->typeCode() && pType2->isShared())
				{
					dumpNameRefScoped(pType2, os);
					ColData_t &aDat(mrDumpTarget.coldata(adcui::IBinViewModel::CLMN_DATA));
					os.flush(aDat);
				}

			//draw name
			
			FieldPtr pField3(nullptr);
			if (iIndexRef)
			{
				TypeEnum_t *pEnum(iIndexRef->typeEnum());
				pField3 = Field(pEnum->enumRef(), index, 0, FieldIt_Exact);
			}
			if (pField3)
			{
				os << "[";

#if(0)
				os.color(adcui::COLOR_DASM_UNK);
				os << index << "|";//no coloring
				os.color(adcui::COLOR_POP);
#endif
				//dumpNameRefScoped(pField3, os);
				os << index;
#if(1)
				os << " ";
				enumOpen(os);
				dumpNameRefScoped(pField3, os);
				//os << index;
				enumClose(os);//no coloring
#endif
				os << "]";

			}
			else
			{
				os << "[" << index << "]";
//				FieldTmp_t tmp(pType2);
				//drawFieldName(&tmp, nullptr);
	//			drawFieldType(&tmp, nullptr);
			}
			

//?			drawNameSuffix(nullptr, pType2, os);

			os.flush(mrDumpTarget.coldata(adcui::IBinViewModel::CLMN_NAMES));
		}

		dumpType(pType2, attr, range2);//exception can be thrown from here
		
		//dump Field2(&f);

		//f.increment();
		index++;
	}
}

bool BinDumper_t::checkSkipLine(CFieldPtr pField, bool &/*bDisableLineSkip*/) const
{
	bool bSkipLine(true);

	if (bSkipLine && !IsEntryLabel(pField))
	{
		if (targetRDA() <= 0 || (curDA() >= mbDumpFrom))//(mbDumpPriorLines > 0))
			bSkipLine = false;
	}

	if (bSkipLine)
		if (isDumpingStrucvar())
			bSkipLine = false;

	/*if (bSkipLine)
		if (checkStrucvarBeginsHere(pField))
		{
			//?		mTargetDA = mCurrentDA;
			bSkipLine = false;
			bDisableLineSkip = true;
		}*/

	return bSkipLine;
}

void BinDumper_t::dumpArrayIndex(CTypePtr iSelf, AttrIdEnum attr, unsigned range)
{
	TypeArrayIndex_t &rSelf(*iSelf->typeArrayIndex());
	dumpArray2(rSelf.arrayRef(), attr, range, rSelf.indexRef());
}

void BinDumper_t::setLevelInfo(int n)
{
	if (mpFrames->mbCollapsed)
		return;
//	if (targetRDA() > 0)
	//	return;
	int iLevel = GetLevel()-1;
	if (iLevel < 0)
		return;
	mrDumpTarget.SetLevelInfo(iLevel, uint8_t(n));
}

/*Strucvar_t::FieldIt BinDumper_t::BeginIt(Strucvar_t *pStruc)
{
	return Strucvar_t::FieldIt(pStruc->fields());
}*/


ROWID BinDumper_t::SizeOfFunc(CTypePtr iType, ROWID da) const
{
	assert(IsProc(iType));

	Struc_t &rFunc(*iType->typeStruc());
	FieldMap &m(rFunc.fields());
	if (!m.empty())
	{
		FieldMapRIt i(m.rbegin());
		unsigned offs(KEY(i) - iType->base());
		if (IsEosField(VALUE(i)))
			return offs;
		da += offs;
		ROWID sz(mrDumpTarget.cachedObjSize(da));
		if (sz != -1)
			sz += offs;
		return sz;
	}
	return OPSZ_BYTE;
}


ROWID BinDumper_t::SizeOf(CTypePtr iType, ROWID *pda) const
{
	if (!iType)
		return 0;

	ROWID da(*pda);
	if (!(iType->typeCode() || iType->typeThunk() || iType->typeStrucvar()))
	{
		if (IsProc(iType))
		{
			//return -1;
			Struc_t &rFunc(*iType->typeStruc());
			FieldMap &m(rFunc.fields());
			if (!m.empty())
			{
				FieldMapRIt i(m.rbegin());
				unsigned offs(KEY(i) - iType->base());
				ROWID sz(ROWID_INVALID);
				FieldPtr pField(VALUE(i));
				if (IsEosField(pField))
				{
					//return offs;
					sz = offs;
					if (i != m.rend())//no entry label
					{
						++i;
						pField = VALUE(i);
						offs = KEY(i) - iType->base();
					}
					else
					{
						pField = nullptr;
						offs = 0;
					}
				}

		//		if (offs > 0)//there is some non-eof field
				{
					if (pField)
					{
						if (pField->isTypeCodeEx())
						{
							da += offs;
							ROWID sz2(mrDumpTarget.cachedObjSize(da));
							if (sz2 != ROWID_INVALID)
							{
								sz2 += offs;
								if (sz == ROWID_INVALID)
									return sz2;
								//if (sz2 <= sz)
								//	return sz;
							}
							else
								sz = ROWID_INVALID;
						}
						else
						{
							if (pField->type())
								return Size(pField->type(), pda);
							return OPSZ_BYTE;
						}
					}

				}

				return sz;
			}
		}
		else
			return Size(iType, pda);
	}
#if(NEW_VARSIZE_MAP)
	TypePtr iSeg(OwnerSeg(pField->owner()));
	assert(iSeg);
	return iSeg->typeSeg()->varObjSize(pField->_key());
#else
	return mrDumpTarget.cachedObjSize(da);	
#endif
}

void BinDumper_t::dumpFuncEnd0(DUMPframe_t &frame, CFieldPtr pField)
{
	if (frame.mbCollapsed)
		return;

	//FuncEnd_t m_eof;
	//FieldTmp_t	m_tmp(&m_eof);//end of func pseudo field
	if (pField && !IsEosField(pField))
		return;
	assert(!pField || IsEosField(pField));
	//pField = &m_tmp;
	//ROWID fsz(curRDA());
	//DUMPframe_t &frame(*mpFrames);
	//?		InitFrame(frame);
	//mrDumpTarget.invalidateRows(frame.mBaseDA + frame.mCurDA);
	frame.mNextChildRDA = 0;
#if(0)//this needed to prevent a hole in RAW column
	//frame.mRaw1 = frame;
	frame.advanceRaw((int)fsz);
#endif
	mCurrentDA--;
	//frame.mCurRDA--;
	dumpFuncEnd(pField);
	EnterNoBreakSection();
	frame.miExtent = unsigned(-1);//avoid triggering range_over exception
	FeedLine(curDA(), 0, 1);
	FeedLine(curDA(), 0, 1);//empty line after a function : doesn't work!
	//frame.mCurRDA++;
	mCurrentDA++;
	LeaveNoBreakSection();
}

void BinDumper_t::dumpStrucEnd0(DUMPframe_t &frame, CFieldPtr pField)
{
	assert(!frame.mbCollapsed);
	//ROWID fsz(curRDA());
	frame.mNextChildRDA = 0;
#if(0)
	if (!RawBlock(frame).empty())
	{
		//frame.mRaw1 = frame;
		frame.advanceRaw((int)fsz);
	}
#endif
	//frame.mCurRDA--;
	mCurrentDA--;
	dumpStrucEnd(pField);
	EnterNoBreakSection();
	frame.miExtent = unsigned(-1);
	FeedLine(curDA(), 0, 1);
	mCurrentDA++;
	//frame.mCurRDA++;
	LeaveNoBreakSection();
}

void BinDumper_t::dumpStrucvar(CTypePtr iSelf, unsigned range)
{
	if (!mpFrames || RawBlock(*mpFrames).empty())
		return;
	//there is no way to tell if strucvar is complete or (to avoid dangling tails), therefore we have to dump it as whole
	if (!mpStrucvarDumper)
	{
		StrucVarDumperEx_t dumper(*this, *iSelf, range);

		//try
		{
			try
			{
				StrucvarDumperIface_t iface(dumper);
				dumper.dump(iface);
			}
			catch (E_BreakStatus e)
			{
				switch (e)
				{
				case E_MISALIGNED_DATA:
				case E_MISALIGNED_CODE:
				case E_MISALIGNED_ARRAY:
				case E_MISALIGNED_UNION:
					//muDumpingStrucvar = 0;//avoid strucvar's size be cached by RAII object
					mrDumpTarget.coldata_clear();//avoid whatever lines are dumped to be added to rows cache
					dumper.end();
					throw e;//align and try again
				case E_FIELD_OVERLAP:
				case E_FIELD_BAD_TYPE:
				case E_NO_DATA:
					break;
				case E_TOO_MANY_LINES:
				case E_RANGE_OVER:
					break;
				default:
					assert(0);
					break;
				}
				//dumper.markIncomplete();
				//			if (e != E_FIELD_OVERLAP && e != E_FIELD_BAD_TYPE)
				//			throw e;//rethrow
			}
			catch (const StrucvarTracer_t::FieldOverlapException& e)
			{
				//dumper.markIncomplete();
				if (e.delta() > 0)
					dumpUnk2(e.delta(), mrDumpTarget.isCompactMode());
			}
			catch (const StrucvarTracer_t::FieldBadTypeException&)
			{
				//dumper.markIncomplete();
			}
			dumper.end();
		}
	}
	else
	{
		StrucVarDumper_t dumper(*this, *iSelf, range, mpStrucvarDumper->frames());
		StrucvarDumperIface_t iface(dumper);
		dumper.dump(iface);
	}

}

void BinDumper_t::dumpUnkBit0(unsigned num, unsigned start)
{
	unsigned i(start);
	while (i < num)
	{
		//size_t iarr(0) , arr(1);
		if (!RawBlock(*mpFrames).empty())
		{
			unsigned bytes(0);
			FeedLine(curDA(), bytes);
			i++;
			continue;
		}

		//unsigned bytes(dumpDataRow(nullptr, OPSZ_NULL, ATTR_NULL, iarr, arr, false));
		unsigned bytes(0);
		FeedLine(curDA(), bytes);
		i++;
	}
}

void BinDumper_t::dumpBits(CTypePtr pType, AttrIdEnum attr, unsigned num, CTypePtr iBitset)
{
	if (mpFrames->mbCollapsed)
	{
		FeedLine(curDA(), num, 0, true);
		return;
	}

	MyStrStream os(mrDumpTarget.isColorsEnabled());

	uint8_t optyp;
	//unsigned iSize;
	if (pType)
	{
		assert(pType->typeSimple());
		optyp = pType->typeSimple()->optype();
		CTypePtr iType2(pType->typeEnum() ? pType->baseType() : pType);
		assert(iType2);
		if (!iType2->nameless())
		{
			dumpNameRefScoped(iType2, os);
			os << " ";
		}
		//iSize = pType->size();
	}
	else
	{
		optyp = iBitset->sizeBytes();
		if (optyp == 0)//dumping strucvar?
			optyp = mpFrames->miExtent;
	}
	
	os << STR_DATASZ(OPSIZE(optyp));//os << "D?";//BIT

	unsigned inbyteOffset(mpFrames->mBitsRDA);
	unsigned bitOffset(unsigned(curRDA()) * CHAR_BIT + inbyteOffset);

	os.color(adcui::COLOR_DASM_COMMENT);
	os << ".";
	os.fill('0');
	os.width(2);
	os.flags(std::ios::hex|std::ios::uppercase);
	os << bitOffset;
	os.color(adcui::COLOR_POP);
	os << " ";

	const uint8_t sz_max(CHAR_BIT * sizeof(value_t));//uint64_t
	assert(inbyteOffset + num <= sz_max);//fixit later!
	//uint8_t sz(uint8_t(num / CHAR_BIT));
	//if (num % CHAR_BIT)
		//sz++;
	value_t v;
	unsigned char *pv(nullptr);
	FieldPtr pField(nullptr);
	if (!RawBlock(*mpFrames).empty())
	{
		pv = (unsigned char *)&v;
		//OFF_t off(RawOffs(*mpFrames));
		OFF_t  off(mpFrames->lower());
		GetDataSource().dataAt(off, OPSIZE(optyp), (PDATA)&v);//sz
		if (mpFrames->mbMSB)
			swap_endian(&v, OPSIZE(optyp));

		v.ui64 >>= bitOffset;
		uint64_t mask(~((~uint64_t(0)) << num));
		v.ui64 &= mask;

		if (OPTYP_IS_SINT(optyp) && (uint64_t(1) << (num - 1)) & v.ui64)
			v.ui64 |= ~mask;

		if (pType && pType->typeEnum())
			pField = Field(pType->baseType(), v.ui32, 0, FieldIt_Exact);
	}
	else
		pType = nullptr;

	DumpUtil_t outp(os);
	if (!pType || attr == ATTR_BINARY)
	{
		int iColor(pType ? adcui::COLOR_DASM_NUMBER : -1);
		if (iColor != -1)
			outp.set_color(iColor);
		outp.out_bitset0(pv, 0/*uint8_t(inbyteOffset)*/, uint8_t(num));//already shifted
		if (iColor != -1)
			outp.set_color(adcui::COLOR_POP);//pop back
	}
	else
		drawValue(v, optyp, attr, os);

	if (pField)
	{
		os << " ";
		enumOpen(os);
		dumpNameRefScoped(pField, os);//colornum?
		enumClose(os);
	}

	ColData_t &aDat(mrDumpTarget.coldata(adcui::IBinViewModel::CLMN_DATA));
	os.flush(aDat);

	FeedLine(curDA(), num, 0, true);
}

void BinDumper_t::dumpBitGap(unsigned gap, bool bFinal, bool bCompact)
{
	DUMPframe_t &frame(*mpFrames);
	if (bCompact)//mrDumpTarget.isCompactMode())
	{
		if (bFinal)
		{
			drawChildHere(frame.muLineStatus);
			mpFrames->mNextChildRDA = 0;
		}
		dumpBits(nullptr, ATTR_NULL, gap, frame.cont());
		return;
	}

	while (gap-- > 0)
	{
		if (gap == 0 && bFinal)
		{
			//mrDumpTarget.SetLevelInfo(iLevel - 1, adcui::ITEM_UPPER_BOUND | uLineStatus);
			drawChildHere(frame.muLineStatus);
			mpFrames->mNextChildRDA = 0;
		}
		dumpBits(nullptr, ATTR_NULL, 1, frame.cont());
	}
}

void BinDumper_t::dumpBitField(CFieldPtr pField, bool bNextChild, unsigned range)
{
	DUMPframe_t &frame(*mpFrames);

	//frame.mBitsRDA = pField->_key() % CHAR_BIT;
	ADDR radBits((ADDR)curRDA() * CHAR_BIT + frame.mBitsRDA);

	if (pField->offset() > radBits)
		dumpBitGap(pField->offset() - radBits, false, mrDumpTarget.isCompactMode());

	mpFrames->mNextChildRDA = bNextChild ? curRDA() + range : 0;
	if (mpFrames->mpUp)
		drawChildHere(frame.muLineStatus);

	drawFieldType(pField, pField->type());
	drawFieldName(pField, pField->type());
	unsigned attr(pField->attrib());
	SET_ATTR_TYPE(attr, __ATTR_BITSET);
	dumpType(pField->type(), AttrIdEnum(attr), range);
}

void BinDumper_t::dumpBitset(CTypePtr iSelf, unsigned range)
{
	const Bitset_t &rSelf(*iSelf->typeBitset());
	DUMPframe_t frame(*this, data(), iSelf);
	InitFrame(frame, range);
	frame.muLineStatus = lineColor(mpFrames->mpUp);//bitsets are never shared

	if (rSelf.hasFields())
		setLevelInfo(adcui::ITEM_HAS_NEXT_CHILD);

	if (!isDumpingStrucvar())
		if (targetRDA() > 0)
			throw (E_MISALIGNED_UNION);//union misalign

	if (iSelf->sizeBytes() > range)
	{
		assert(iSelf->sizeBytes() != -1);
		mpFrames->mIncomplete++;
		throw E_FIELD_OVERLAP;
	}

	EnterNoBreakSection();
//?	FeedLine(curDA());

	unsigned uSize(rSelf.size(iSelf));//bit size
	const FieldMap &l(rSelf.fields());
	mpFrames->mNextChildRDA = !l.empty() ? -1 : 0;
	FieldMapCIt it(rSelf.fields().begin());
	while (it != l.end())
	{
		CFieldPtr pField(VALUE(it));
		FieldMapCIt it0(it++);
		bool bLast(it == l.end());
		bool bNextChild(!bLast || pField->offset_hi() < uSize);
		dumpBitField(pField, bNextChild, range);//arg: next child
		STOP
	}
	//unsigned gap2(rSelf.size Bytes(iSelf) * CHAR_BIT - frame.mBitsRDA);

	ADDR rdaBits((ADDR)curRDA() * CHAR_BIT + frame.mBitsRDA);
	
	if (uSize > (int)rdaBits)
		dumpBitGap(uSize - rdaBits, true, mrDumpTarget.isCompactMode());

	LeaveNoBreakSection();
}

void BinDumper_t::dumpStruc2(CTypePtr iSelf, unsigned range)
{
	Struc_t &rSelf(*iSelf->typeStruc());
	DUMPframe_t frame(*this, data(), iSelf);
	InitFrame(frame, range);
	
	if (rSelf.hasFields() || rSelf.typeSeg() && rSelf.typeSeg()->traceLink())// && frame.mpUp)
	if (targetRDA() == 0 || isDumpingStrucvar())
		setLevelInfo(adcui::ITEM_HAS_NEXT_CHILD);

	FeedLine(curDA());

	StrucDumpIter it0(*this, *iSelf);
	it0.initialise();
	StrucDumpIter it1(it0);
	if (it1)
		++it1;
#if(0)
	dumpStrucIter(it0, it1, range);
#else
	try
	{
		dumpStrucIter(it0, it1, Range(frame));
	}
	catch (E_BreakStatus e)
	{
		if (e == E_PARTIAL_PAGE_DONE || e == E_PAGE_END || e == E_RANGE_OVER)
		{
			if (frame.miExtent == ROWID_INVALID)
			{
				assert(0);
				//fsz = FuncSizeLimited(iSelf);
				//if (curRDA() >= fsz)
					//fsz = curRDA();
			}
			if (curRDA() == frame.miExtent)
			{
				assert(!frame.mbCollapsed);
				ROWID rda;
				CFieldPtr pField(it0.field(rda));
				if (pField)
				{
					mrDumpTarget.coldata_clear();
					mrDumpTarget.coldata_expand();
					dumpStrucEnd0(frame, pField);
				}
				if (e == E_RANGE_OVER)
					return;
			}
		}
		throw e;
	}
	ROWID rda;
	CFieldPtr pField(it0.field(rda));
	if (pField && !frame.mIncomplete)
		if (!frame.mbCollapsed)
			dumpStrucEnd0(frame, pField);
#endif
}

void BinDumper_t::dumpStrucIter(StrucDumpIter &it0, StrucDumpIter &it1, unsigned range0)
{
	ROWID sizex(mpFrames->miExtent);

	ROWID maxRDA(curRDA() + range0);
	if (sizex < maxRDA)
		maxRDA = sizex;

	bool bIsFunc(IsProc(mpFrames->cont()));
	bool bIsSeg(mpFrames->cont()->typeSeg() != nullptr);

	//int z = 0;
	while (curRDA() < sizex)//maxRDA
	{
		//z++;
CHECK(maxRDA < curRDA())
STOP

		assert(maxRDA >= curRDA());
		unsigned range((unsigned)(maxRDA - curRDA()));
		if (range == 0)
		{
			mpFrames->mIncomplete++;
			//dumpUnk2(range, mrDumpTarget.isCompactMode());
			throw E_FIELD_OVERLAP;
			//?break;//Break(E_RANGE_OVER);
		}

		ROWID rda;
		CFieldPtr pField0(it0.fieldOrEos(rda));
CHECKID(pField0, 0x1a4)
STOP
//		if (pField0 && IsEosField(pField0)))
//			pField0 = nullptr;
		CFieldPtr pField(it0.field(rda));
	//	if (pField)
		//	mpFrames->mbNextChild = true;
//		if (!((pField && pField0) || (!pField && !pField0)))
//		{
//			STOP
//		}

		if (pField && !IsEosField(pField))
		{
//CHECK(addr == 0x4c08eb)
//STOP
			if (rda == ROWID_INVALID)
			{
				//drawChildHere(lineColor(mpFrames));
				//mrDumpTarget.drawData("endp", adcui::COLOR_DASM_CODE);//directive
				Break(E_RANGE_OVER);//scope end
			}

			mpFrames->mNextChildRDA = (bIsFunc || bIsSeg) ? -1 : 0;//always for functions

			if (rda > curRDA())//???
			{
				range = (unsigned)(rda - curRDA());
				mpFrames->mNextChildRDA = rda;
			}
			else
			{
				ROWID rda2;
				CFieldPtr pFieldNx(it1.fieldOrEos(rda2));
				if (pFieldNx)
				{
					//union check
					if (pFieldNx->_key() == pField->_key())// || it0.nextAddrField(rda2))
					{
						//CTypePtr pCont(mpFrames->cont());
						//pField = dumpUnion0(pCont, pCont->typeStruc()->fields(), FieldMapCIt(pField), range);
					}

					range = std::min(range, (unsigned)(rda2 - rda));
					mpFrames->mNextChildRDA = curRDA() + range;
				}
				else
				{
					Seg_t *pSeg(it1.struc().typeSeg());
					if (pSeg)
					{
						CTypePtr iSeg(it1.strucRef());
						ROWID ssz(iSeg->size());
						ROWID tsz(SegTraceSize(iSeg));
						if (ssz < tsz)
						{
							range = (unsigned)(maxRDA - curRDA());

							Struc_t *pTrace(pSeg->traceLink()->typeStruc());
							FieldMap &m(pTrace->fields());
							FieldMapIt itp(m.lower_bound((FieldKeyType)ssz));
					//FieldMapIt itp(ProjectInfo_t::SkipEos(m, m.lower_bound((FieldKeyType)ssz)));
							if (itp != m.end())
								mpFrames->mNextChildRDA = curRDA() + range;
						}
						else if (curRDA() < tsz)
						{
							ROWID maxRDA2 = tsz;
							range = (unsigned)(maxRDA2 - curRDA());
						}
					}
					//?range = std::min(sizex, (ROWID)maxSize0) - rda;
				}

				if (rda > maxRDA)
					throw E_FIELD_OVERLAP;
				/*if (pField->type())
				{
					CTypePtr iType(pField->type());
					Simple_t *pSimple(iType->typeSimple());
					ROWID rda2(rda + iType->size());
					if (pSimple && rda2 > maxRDA)
					{
						STOP
						/ *adjustLevelInfo();
						assert(rda2 - maxRDA == range);
						dumpUnk2(range, mrDumpTarget.isCompactMode());
						throw E_FIELD_OVERLAP;* /
					}
				}*/
	
				if (rda == curRDA())
				{
#if(0)
					try
					{
						dump Field2(pField);
					}
					catch (E_BreakStatus e)
					{
						//if (e == E_PARTIAL_PAGE_DONE || e == E_PAGE_END || e == E_RANGE_OVER)
						{
							it0 = it1;
							if (it1)
								++it1;
						}
						throw e;
					}
#else
					if (pField->owner())
					{
						bool bUfield(it0.isUfield());

						it0 = it1;
						if (it1)
							++it1;

						if (bUfield)
						{
							if (targetRDA() > 0)
								throw (E_MISALIGNED_UNION);//union misalign
							dumpUnionFields(pField, range);
						}
						else
						{
							dumpField2(pField);
							dumpFieldType(pField, range);
						}
					}
					else
					{
						dumpField2(pField);
						dumpFieldType(pField, range);
						it0 = it1;
						if (it1)
							++it1;
					}
#endif
					continue;
				}
				throw E_DEFAULT;//misalign?
			}
		}
		else
		{
			CTypePtr iSeg(it1.strucRef());
			Seg_t *pSeg(iSeg->typeSeg());
			if (pSeg)
			{
				ROWID ssz(iSeg->size());
				ROWID tsz(SegTraceSize(iSeg));

				if (ssz < tsz)
				{
					range = (unsigned)(maxRDA - curRDA());

					CTypePtr iTrace(pSeg->traceLink());
					Struc_t *pTrace(iTrace->typeStruc());
					FieldMap &m(pTrace->fields());
	//FieldMapIt itp(ProjectInfo_t::SkipEos(m, m.lower_bound((FieldKeyType)ssz)));
					FieldMapIt itp(m.lower_bound((FieldKeyType)ssz));
					if (itp != m.end())
						mpFrames->mNextChildRDA = curRDA() + range;
				}
				else if (curRDA() < tsz)
				{
					ROWID maxRDA2 = tsz;
					range = (unsigned)(maxRDA2 - curRDA());
				}

			}
		}

		if (mpFrames->mpCodePending)
		{
			dumpCodeBegin(nullptr, range);
			dumpCode2();//this will recreate a code printer with expanded range
		}
		else
		{
			if (mpFrames->miExtentOpen == -1)
				if (!pField && bIsFunc)
						Break(E_RANGE_OVER);

			if ((int)targetRDA() <= range)
			{
				if (bIsFunc || pField)
					mpFrames->mNextChildRDA = curRDA() + range;//always for functions
				bool bCompact(mrDumpTarget.isCompactMode());
#if(SQUEEZE_STRUCVAR_GAPS)
				if (!bCompact && isDumpingStrucvar())
					bCompact = true;
#endif
				dumpUnk2(range, bCompact);
			}
			else
				advance(range, range);
		}
	}
}

void BinDumper_t::dumpFunc2(CTypePtr iSelf, int range)
{
	//Struc_t &rFunc(*iSelf->typeStruc());

	DUMPframe_t frame(*this, data(), iSelf);
	InitFrame(frame, range);
	//if (rFunc.hasFields())//always
		setLevelInfo(adcui::ITEM_HAS_NEXT_CHILD);

	if (!frame.mbCollapsed)
		FeedLine(curDA());

	ROWID fsz(SizeOfFunc(iSelf, curDA()));
	(void)fsz;
	StrucDumpIter it0(*this, *iSelf);
	it0.initialise();
	StrucDumpIter it1(it0);
	if (it1)
		++it1;
	try
	{
		dumpStrucIter(it0, it1, range);
		if (mBreakStatus != E_RANGE_OVER)
			return;
	}
	catch (E_BreakStatus e)
	{
		if (e != E_RANGE_OVER)
			throw e;
	}

	mBreakStatus = E_NONE;//reset

	if (!frame.mbCollapsed)
	{
		mrDumpTarget.coldata_clear();
		mrDumpTarget.coldata_expand();
		ROWID rda;
		dumpFuncEnd0(frame, it0.field(rda));
	}
//	dumpCodeEnd(nullptr);
}


void BinDumper_t::dumpHeader()
{
/*?	if (mStar tRow > 0)
		ret urn;

	int i = 0;
	mapSEG_t& segs = mrMain.dc()->mSegs;
	for ( mapSEG_it it = segs.begin(); it != segs.end(); ++it ) 
	{
		ADDR offs = it->first;
		Seg_t * pSeg = it->second;
		const char * name = pSeg->name();
		if (!name && pSeg->parent())
			name = pSeg->parent()->name();

		mrDumpTarget.draw(CLMN_NAMES,
			fmt("Segment %d: %s, BASE=%08X, END=%08X, SIZE=%08X", 
				i++, name, pSeg->mBase, pSeg->mBase+pSeg->size()-1, pSeg->size())
				);
		FeedLine(curDA(), );
	}
	FeedLine(curDA(), );*/
}

void BinDumper_t::EnterNoBreakSection()
{
	if (++mbFreezePageBreak == 1)
	{}//mTargetDA = mCurrentDA;
}

void BinDumper_t::LeaveNoBreakSection()
{
	assert(mbFreezePageBreak > 0);
	if (--mbFreezePageBreak == 0)
		if (mBreakStatus != 0)
			throw mBreakStatus;
}

void BinDumper_t::Break(E_BreakStatus status)
{
	if (mbFreezePageBreak == 0)
		throw (status);
	else //if (!mBreakStatus)
		mBreakStatus = status;
}

void BinDumper_t::PushFrontPtr(CTypePtr iSeg)
{
	assert(iSeg->typeSeg()->frontIndex() > 0);
	mFrontSegs.push_back(iSeg);
}

void BinDumper_t::PopFrontPtr()
{
	assert(!mFrontSegs.empty());
	mFrontSegs.pop_back();
}


void BinDumper_t::dumpUnion2(CTypePtr iSelf, unsigned range)
{
	Struc_t& rUnion(*iSelf->typeStruc());

	DUMPframe_t frame(*this, data(), iSelf);
	InitFrame(frame);

	if (rUnion.hasFields() && mpFrames->mpUp)
		setLevelInfo(adcui::ITEM_HAS_NEXT_CHILD);

	if (targetRDA() > 0)
		throw (E_MISALIGNED_UNION);//union misalign

	FeedLine(curDA());

	dumpUnionFields(VALUE(rUnion.fields().begin()), iSelf->size());//adjust range
}

void BinDumper_t::dumpUnionFields(CFieldPtr pField, unsigned range)
{
	CTypePtr pCont(pField->owner());
	const FieldMap& fields(pCont->typeStruc()->fields());
	FieldMapCIt it(pField);

	dumpCodeEnd(pField);

	//mbDumpPriorLines++;
	EnterNoBreakSection();
	//mbFreezePageBreakU++;

	DUMPframe_t saved(*mpFrames);//reuse frame's state for all union fields
	saved.mbTemp = true;//do not update parent frame in destructor
	//ADDR offs_orig((ADDR)curRDA());
	
	ROWID cur_da(mCurrentDA);

	CodePrinter* pCodePrinter(nullptr);
	CFieldPtr pFieldCode(nullptr);
	unsigned shift(0);
	while (it != fields.end())
	{
		CFieldPtr pField(VALUE(it));
		++it;
		bool bLast(it == fields.end() || KEY(it) != pField->_key());
		mpFrames->mNextChildRDA = !bLast ? range : 0;
		dumpField2(pField);
		if (pField->type())
		{
			if (!bLast && (pField->type()->typeCode() || pField->type()->typeThunk() || pField->type()->typeProc()))
			{
				assert(!pFieldCode);
				pFieldCode = pField;
				FeedLine(curDA(), 0);
			}
			else
				dumpFieldType(pField, range);
			assert(!mpFrames->mpCodePending);
		}
		else if (bLast)
		{
			if (!pFieldCode)
				dumpFieldType(pField, range);
			else
			{
				dumpFieldType(pFieldCode, range);
				pCodePrinter = mpFrames->mpCodePending;//this must be preserved
			}
		}
		else
			FeedLine(curDA(), 0);

		//recover starting state
		*mpFrames = saved;
		//mRowSpan = 0;
		shift = std::max(shift, (unsigned)(mCurrentDA - cur_da));//this will get a union's size
		mCurrentDA = cur_da;
		if (bLast)
			break;
	}

	mpFrames->mpCodePending = pCodePrinter;

	//unsigned shift(iSelf->size());
	advance(shift, shift);
	//assert(mRowSpan == 0);
	 
	//mbDumpPriorLines--;
	LeaveNoBreakSection();
}



class CodePrinter : private DataSubSource_t
{
	CodeStream_t m_code;
	CTypePtr miTypeCode;
	ROWID mStartDA;
	int miLine;//line count
	CTypePtr miSegList;
	bool mbCollapsed;
public:
	CodePrinter(const I_DataSourceBase &rData, OFF_t ra, ROWID da, CFieldPtr pField, CTypePtr iOwnerSeg, unsigned range)
		//: DataSubSource_t(r.data(), ProjectInfo_t::SegOffset(pField), range),
		: DataSubSource_t(rData, ra, range),//RawOffs(*r.mpFrames)
		m_code(*this, iOwnerSeg->imageBase(), pField->_key(), pField->_key(), iOwnerSeg->typeSeg()->isLarge()),
		miTypeCode(pField->isTypeThunk() ? pField->isTypeThunk()->baseType() : pField->isTypeCode()),
		mStartDA(da),//r.curDA()
		miLine(0),
		miSegList(pField->parentSegList()),
		mbCollapsed(pField->isCollapsed())
	{
	}
	CodePrinter(const CodePrinter &o, unsigned addRange)
		: DataSubSource_t(o, addRange),//add range
		m_code(*this, o.imageBase(), o.base(), o.addr(), o.isLarge()),
		miTypeCode(o.miTypeCode),
		mStartDA(o.mStartDA),
		miLine(o.miLine),
		miSegList(o.miSegList),
		mbCollapsed(false)//!
	{
		//assert(!isFinalized());
	}
	bool isCollapsed() const { return mbCollapsed; }
	ADDR base() const { return m_code.base(); }
	ADDR addr() const { return m_code.addr(); }
	ADDR64 imageBase() const { return m_code.imageBase(); }
	bool isLarge() const { return m_code.isLarge(); }
	ROWID startDA() const { return mStartDA; }
	int line() const { return miLine; }
	void advance(){
		m_code.updateAddr();
		miLine++;
	}
	bool isAtEnd() const {
		return /*isFinalized() ||*/ m_code.isAtEnd();
	}
	CTypePtr superLink() const { return miSegList; }
	CTypePtr typeCode() const { return miTypeCode; }
	int print(IOutpADDR2Name &cb, ins_desc_t &desc)
	{
		int len(miTypeCode->typeCode()->print(m_code, m_code.imageBase(), m_code.addr(), &cb, desc));
		if (len > 0)
			return len;
		m_code.seek(OFF_NULL);
		return 0;
	}
	int skip(ins_desc_t &desc)
	{
		assert(m_code);
		int len(miTypeCode->typeCode()->skip(m_code, m_code.imageBase(), m_code.addr(), desc));
		if (len > 0)
			return len;
		m_code.seek(OFF_NULL);
		return 0;
	}
	//bool isFinalized() const { return m_desc.isFinalized(); }
};

void BinDumper_t::dumpCodeBegin(CFieldPtr pField, unsigned range)
{
	CodePrinter *pCodePrinter(mpFrames->mpCodePending);//old
	if (pCodePrinter)
	{
		//proceed with state of the old code printer but update a range
		mpFrames->mpCodePending = new CodePrinter(*pCodePrinter, range);
		delete pCodePrinter;//kill the old one
	}
	else
	{
		EnterNoBreakSection();
		CTypePtr iOwneSeg(ProjectInfo_t::OwnerSeg(pField->owner()));
		ROWID ra(RawOffs(*mpFrames));
		mpFrames->mpCodePending = new CodePrinter(data(), ra, curDA(), pField, iOwneSeg, range);
	}
	//return *mpFrames->mpCodePending;
}

void BinDumper_t::dumpCodeEnd(CFieldPtr pFieldAt)
{
	CodePrinter *pCodeIt(mpFrames->mpCodePending);
	if (!pCodeIt)
		return;

	if (pFieldAt)
	{
		if (!pFieldAt->isTypeCode())
			mpFrames->mbProblemLine = 1; 
		//else
			//return;//let it flow
	}

#if(NEW_VARSIZE_MAP)
	rSeg.setVarObjSize(startVA, unsigned(curDA() - startDA));
#else
	mrDumpTarget.setCachedObjSize(pCodeIt->startDA(), curDA() - pCodeIt->startDA());
#endif
	mpFrames->mpCodePending = nullptr;
	delete pCodeIt;
	LeaveNoBreakSection();
}



///////////////////////////////////////////////
class CodeDumpTarget_t : public ModuleInfo_t,
	public IOutpADDR2Name
{
	CTypePtr miSeg;//owner
	BinDumper_t &mrDump;
	CTypePtr	miTypeCode;
	bool	mbColors;
	bool	mbResolveRefs;
	VALUE_t		m_ripVA;//RIP-relative addressing
	MyString	mMnemoString;
	MyString	mOpsString;
	MyString	mBytesString;
	MyString	mCommentString;
public:
	CodeDumpTarget_t(Project_t &rProjRef, CTypePtr iSeg, CTypePtr iTypeCode, BinDumper_t &rDump) 
		: ModuleInfo_t(rProjRef, *rDump.mpFrames->miModule),
		miSeg(iSeg),
		mrDump(rDump),
		miTypeCode(iTypeCode),
		mbColors(mrDump.mrDumpTarget.isColorsEnabled()),
		mbResolveRefs(mrDump.mrDumpTarget.isResolveRefsMode())
	{
	}
	const VALUE_t &ripVA() const {
		return m_ripVA;
	}
	const MyString &mnemoString() const {
		return mMnemoString;
	}
	const MyString &opsString() const {
		return mOpsString;
	}
	const MyString &bytesString() const {
		return mBytesString;
	}
	const MyString& commentString() const {
		return mCommentString;
	}
	bool checkThunkAt(ADDR va, CodeDumpTarget_t &aDecoder) const
	{
		TypePtr iSeg(VA2Seg(miSeg, va));
		if (!iSeg)
			return false;

		{
			Locus_t aLoc;
			if (!FindFieldInSeg(iSeg, va, aLoc))
				return false;
			if (!aLoc.field0()->isTypeThunk())
				return false;
		}

		DataSourcePane2_t data(GetDataSource()->pvt(), iSeg);
		CodeIterator codeIt(data, miTypeCode, iSeg, va);
		//codeIt.unassemble();
		ins_desc_t desc;
		int len(miTypeCode->typeCode()->print(codeIt, codeIt.imageBase(), codeIt.addr(), &aDecoder, desc));
		(void)len;

		if (!desc.isUnconditionalJump())
			return false;

		TypePtr iSeg2(VA2Seg(miSeg, desc.vaRef));
		if (!iSeg2)
			return false;

		Locus_t aLoc;
		if (!FindFieldInSeg(iSeg2, desc.vaRef, aLoc))
			return false;

		return true;
	}

	bool _decode(ADDR va, out_t &dot)
	{
		//ModuleInfo_t MI(mrDump, *mrDump.mpFrames->miModule);
	//?	va -= (ADDR)miSeg->typeSeg()->imageBase();

		CTypePtr iSeg(VA2Seg(miSeg, va));
		if (!iSeg)
			return false;
		if (miSeg->typeSeg()->superLink() == iSeg)
			return false;

		Locus_t aLoc;
		CFieldPtr f(FindFieldInSeg(iSeg, va, aLoc));
		if (!f)
			return false;

#if(0)
		aLoc.wrapUp();//could be an entry label
		CFieldPtr f2(aLoc.as Proc());
		if (f2)
			f = f2;
		int offs(va - f->_key());
#else
		int offs(0);
		if (!f->isTypeCode() || IsEntryLabel(f))
			if ((f = aLoc.stripToSeg((unsigned &)offs)) == nullptr)
				return false;
#endif

		//if (bCall && f->type() && !IsFunc(f->type()) && IsFunc(f->OwnerStruc()))
		//f = f->owner()->parentField();

		MyStrStream os;
		mrDump.drawName2(f, os);
		if (offs > 0)
			os << "+" << offs;
		else if (offs < 0)
			os << "-" << -offs;

		if (mbResolveRefs)
			os.flush(dot.name);
		else
			os.flush(mCommentString);

		if (f->type() && f->type()->typeSimple())
			dot.sz = f->OpSize();
		else
			dot.sz = 1;
		return true;
	}

	/*bool check_thunk_at(ADDR va, ADDR &va_out, reg_t &sreg) const
	{
		return false;
		if (!miTypeCode)
			return false;
		//ModuleInfo_t MI(mrDump, *mrDump.mpFrames->miModule);
		TypePtr iSeg(VA2Seg(miSeg, va));
		if (!iSeg)
			return false;
		DataSourcePane2_t data(GetDataSource()->pvt(), iSeg);
		CodeIterator codeIt(data, miTypeCode, iSeg, va);
		codeIt.unassemble();

		const ins_desc_t &desc(codeIt.desc());
		const ins _t &ins(codeIt.ins());
		//if (!ins.is_jump() && !ins.is_goto())
		if (!desc.isUnconditionalJump())// || !desc.isIndirect())
			return false;

		//assert(ins.ops_num == 1);
		const opr_t& op = ins.ops[0];
		//va_out = op.lval.ui32;
		va_out = desc.va;
		sreg.clear();
		if (op.is_indirect())
			sreg = op.segreg();
		//desc.isIndirect();
		//sreg.assign(

		return true; 
	}*/

protected:
	virtual bool decode(ADDR va, bool /*bCheckThunk*/, out_t& dot) const
	{
		/*if (bCheckThunk)
		{
			ADDR va1((ADDR)va);
			reg_t sreg;
			if (check_thunk_at((ADDR)va, va1, sreg))
			{
				if (_decode(va1, dot))
				{
					dot.bThunk = true;
					dot.sreg = sreg;
					return true;
				}
			}
		}*/
		if (const_cast<CodeDumpTarget_t *>(this)->_decode(va, dot) != 0 && mbResolveRefs)//the name will be tried anyway
			return true;
		return false;
	}

	virtual void on_RIP_relative_addressing(const VALUE_t &v)
	{
		m_ripVA = v;
	}

	virtual void print_bytes(const uint8_t *bytes, size_t length)
	{
		mBytesString.assign((const char *)bytes, length);
	}

	virtual bool colorsEnabled() const
	{
		return mbColors;
	}

	virtual void print_code(const char * /*fmt*/, const uint8_t *data, size_t length)
	{
		std::string s((const char *)data, length);
		size_t n(s.find('\t'));//mnemo-ops separator
		if (n != std::string::npos)
		{
			mMnemoString = s.substr(0, n);
			mOpsString = s.substr(n + 1);
		}
		else
			mMnemoString = s;
	}
};


class TooManyLinesFault_t : public std::exception
{
public:
	TooManyLinesFault_t(){}
protected:
	virtual const char * what() const throw (){
		return "Too Many Lines Fault";
	}
};

class InvalidOpcodeFault_t : public std::exception
{
public:
	InvalidOpcodeFault_t(){}
protected:
	virtual const char * what() const throw (){
		return "Invalid Op Code";
	}
};

void BinDumper_t::dumpCode2()
{
//CHECK(pField->_key() == 0x20903ff2)
//CHECK(pField->_key() == 0xba)

	assert(mpFrames->mpCodePending);
	CodePrinter &codeIt(*mpFrames->mpCodePending);
CHECK(codeIt.base() == 0x73e70)
STOP

	CTypePtr iTypeCode(codeIt.typeCode());

	bool bCollapsed(mpFrames->mbCollapsed || codeIt.isCollapsed());

	try
	{
		bool bFlowBreak(false);
		int span_length(0);
		while (!codeIt.isAtEnd())
		{
			if (!mrDumpTarget.isFullMode() && !(codeIt.line() < CODE_LINES_MAX))
				throw TooManyLinesFault_t();

CHECK(codeIt.addr() == 0x5106e3)
STOP
			ins_desc_t aDesc;
			if (!bCollapsed)
			{
				CodeDumpTarget_t aDecoder(mrProject, codeIt.superLink(), iTypeCode, *this);

				int ins_length(codeIt.print(aDecoder, aDesc));
				if (ins_length == 0)
					throw InvalidOpcodeFault_t();

				MyString sMnemo(aDecoder.mnemoString());
				MyString sOps(aDecoder.opsString());

				if (mrDumpTarget.isResolveRefsMode())
				{
					//replace call target if thunk is detected
					if (aDesc.isCall() && aDesc.isDirect())
					{
						CodeDumpTarget_t aDecoder2(mrProject, codeIt.superLink(), iTypeCode, *this);
						if (aDecoder.checkThunkAt(aDesc.vaRef, aDecoder2))
						{
							mrDumpTarget.drawComment(sOps);//old op as a remark
							sOps = aDecoder2.opsString();
						}
					}
				}
				else if (!aDecoder.commentString().isEmpty())
				{
					mrDumpTarget.drawComment(aDecoder.commentString());
				}

				//draw code
				MyStrStream os(mrDumpTarget.isColorsEnabled());
				os.color(!sMnemo.isEmpty() ? adcui::COLOR_DASM_CODE : adcui::COLOR_DASM_UNK_NAME);

				os.write(sMnemo);
				if (!sOps.empty())
				{
					const int MAX_MNEMO_LEN = 7;
					int dcol(MAX_MNEMO_LEN - (int)sMnemo.length());
					if (dcol < 0)
						dcol = 1;
					while (dcol-- > 0)
						os << ' ';
					os.write(sOps);
				}
				os.color(adcui::COLOR_POP);

				MyString sCode;
				os.flush(sCode);

				mrDumpTarget.draw(adcui::IBinViewModel::CLMN_CODE, sCode);

				//draw bytes
				const MyString &bytes(aDecoder.bytesString());
				mrDumpTarget.drawBytes(bytes.data(), bytes.length());

				//draw rip-addressing info
				const VALUE_t &v(aDecoder.ripVA());
				if (v.typ)
				{
					MyString s;
					MyStrStream os2(mrDumpTarget.isColorsEnabled());
					drawValue(v, v.optyp, ATTR_OFFS, os2);
					os2.flush(s);
					mrDumpTarget.drawComment(s);
				}

				FeedLine(curDA(), ins_length);
			}
			else
			{
				int len(codeIt.skip(aDesc));//get length only
				if (len == 0)
					throw InvalidOpcodeFault_t();
				span_length += len;
			}
			
			codeIt.advance();
			if (aDesc.isFinalized())
			{
				bFlowBreak = true;
				break;
			}
		}

		if (bCollapsed)
			FeedLine(curDA(), span_length);

		if (mpFrames->mNextChildRDA == -1)//no more fields in (open-ranged) function
			Break(E_RANGE_OVER);//this should override any break status so far

		if (bFlowBreak)
			dumpCodeEnd(nullptr);//finalize code dump if a flow sweep ended naturally (by jmp,ret,etc.) or because of a bad instruction
	}
	catch (const DataAccessFault_t &)
	{
		if (!bCollapsed)
			mrDumpTarget.drawComment("data access fault");
		mpFrames->mbProblemLine = true;
		dumpCodeEnd(nullptr);
		dumpUnk0(1, 0);
	}
	catch (const TooManyLinesFault_t &)
	{
		if (!bCollapsed)
			mrDumpTarget.drawComment("too many lines");
		mpFrames->mbProblemLine = true;
		dumpCodeEnd(nullptr);
		dumpUnk0(1, 0);
	}
	catch (const InvalidOpcodeFault_t &)
	{
		if (!bCollapsed)
			mrDumpTarget.drawComment("invalid opcode");
		mpFrames->mbProblemLine = true;
		EnterNoBreakSection();
		dumpCodeEnd(nullptr);
		dumpUnk0(1, 0);
		if (mpFrames->mNextChildRDA == -1)//no more fields in (open-ranged) function
			Break(E_RANGE_OVER);//this should override any break status so far
		LeaveNoBreakSection();
	}
}





DumpInvariant_t::DumpInvariant_t(bool bColorsEnabled, bool bPackMode)
	: mTreeColumn(0),
	mbFullMode(false),
	mbCompactMode(bPackMode),
	mbColorsEnabled(bColorsEnabled),
	mbHideTypesMode(false),
	mbResolveRefsMode(true),
	mpTarget(nullptr)//,
	//mRefsNum(0)
{
	resize(adcui::IBinViewModel::CLMN_TOTAL);
}

DumpInvariant_t::~DumpInvariant_t()
{
	delete mpTarget;
}

void DumpInvariant_t::initColumns(CTypePtr iModule)
{
	using namespace adcui;
	resize(IBinViewModel::CLMN_TOTAL);
	//coldata().resize(cols().size());
	
	bool bStrucDump(iModule == nullptr);
	if (!bStrucDump)
	{
		initColumn(IBinViewModel::CLMN_TREEINFO, "$LEVEL$", -8);//set width to 0 to disable
		initColumn(IBinViewModel::CLMN_DA, "DA", -9, CLMNFLAGS_DISPSEL | CLMNFLAGS_HEADER);
		//initColumn(IBinViewModel::CLMN_ROWSPAN, "$ROWSPAN$", 0);
		initColumn(IBinViewModel::CLMN_FILE, "FILE", -9, CLMNFLAGS_DISPSEL | CLMNFLAGS_HEADER);
		if (ProjectInfo_t::IsPhantomModule(iModule))
			initColumn(IBinViewModel::CLMN_OFFS, "ORD", +9, CLMNFLAGS_DISPSEL | CLMNFLAGS_HEADER);
		else
			initColumn(IBinViewModel::CLMN_OFFS, "VA", +9, CLMNFLAGS_DISPSEL | CLMNFLAGS_HEADER);
		initColumn(IBinViewModel::CLMN_BYTES, "BYTES", -16, CLMNFLAGS_TRIM);
		//initColumn(IBinViewModel::CLMN_TYPES, "TYPES", -16);
		initColumn(IBinViewModel::CLMN_NAMES, "NAMES", 32);
		initColumn(IBinViewModel::CLMN_CODE, "CODE", 36);
		initColumn(IBinViewModel::CLMN_XREFS, "XREFS", -16);
		initColumn(IBinViewModel::CLMN_COMMENTS, "REMARKS", +32);
	}
	else
	{
		initColumn(IBinViewModel::CLMN_TREEINFO, "$LEVEL$", -8);//set width to 0 to disable
		initColumn(IBinViewModel::CLMN_OFFS, "OFFS", +5, CLMNFLAGS_DISPSEL | CLMNFLAGS_HEADER);
		initColumn(IBinViewModel::CLMN_NAMES, "NAMES", 24);
		initColumn(IBinViewModel::CLMN_CODE, "CODE", 24);
	}
}

void DumpInvariant_t::initColumn(COLID iCol, const char * name, int width, unsigned flags)
{
	ColDesc_t &col(at(iCol));
	col.width = width;
	col.name = name;
	col.flags = flags;
}


void BinDumper_t::drawName2(CFieldPtr pField, MyStrStream &os) const
{
	if (pField->owner() && pField->owner()->typeStrucvar())
	{
		MyString s(mrProject.fieldDisplayName(pField));
		/*if (pField->name())
			s = pField->name()->c_str();
		else
			s = "???";*///error after recovery?
		if (s.endsWith("#"))
		{
			s.chop(1);
			s.append(MyStringf("%X", curRDA()).c_str());
		}
		os << s;
		return;
	}

	if (pField->m_nFlags & FLD_TEMP)
	{
		os << mrProject.fieldDisplayName(pField);
		return;
	}
	
	if (pField->isTypeSeg())
	{
		if (pField->nameless())
			os << TypeName(pField->type());
		else
			os << pField->name()->c_str();
		return;
	}
	
	dumpNameRefScoped(pField, os);
}


void BinDumper_t::drawCommentDemangled(CFieldPtr pField) const
{
	//draw demangled name for imported? symbols
	if (!pField->nameless() && IsImpOrExp(pField) && !mFrontSegs.empty())
	{
		MyString s0(pField->name()->c_str()), s1;
#if(0)
		//if (pField->flags()  & FLD_SHAREDNAME)
		unsigned n(ChopName(s0, s1));
		if (n > 0)
		{
			TypePtr iNSOwner(mrProject.GetNameOwnerOf(pField->owner()));
			NamesMgr_t *pNS(iNSOwner->typeComplex()->namesMgr());
			Obj_t* pObj(pNS->findObj(s1));//pField->name()));
			FieldPtr pField2(pObj ? pObj->objField() : nullptr);
			//mrDumpTarget.drawComment(MyStringf("duplicate of %08X", pField2->_key()));
			
			MyStrStream os(mrDumpTarget.isColorsEnabled());
			MyString s("duplicate of ");

			if (pField2)
			{
				value_t v;
				v.ui32 = pField2->_key();
				drawValueEx(v, OPTYP_DWORD, ATTR_OFFS, os);
				os.flush(s);
			}
			else
				s.append("<unknown>");

			mrDumpTarget.drawComment(s);
		}
#endif

		MyStream ss;
		FRONT_t *pFE(FrontAt(mFrontSegs.back()->typeSeg()->frontIndex()));
		pFE->device(&GetDataSource())->demangleName(s0.c_str(), ss);
		MyString s;
		if (ss.ReadString(s) && s != s0)
			mrDumpTarget.drawComment(s);
	}
}

void BinDumper_t::drawColumn(COLID /*iCol*/, const char *str) const
{
	if (mpFrames->mbCollapsed)
		return;
	MyStrStream os(mrDumpTarget.isColorsEnabled());
	os << str;
	os.flush(mrDumpTarget.coldata(adcui::IBinViewModel::CLMN_NAMES));
}

void BinDumper_t::drawFieldName(CFieldPtr pField, CTypePtr iTypeProxy)
{
	if (mpFrames->mbCollapsed)
		return;

	if (!pField->name() && isDumpingStrucvar())
		return;

	CTypePtr iType(CheckProxyType(pField, iTypeProxy));

	MyStrStream os(mrDumpTarget.isColorsEnabled());
	drawName2(pField, os);
	drawCommentDemangled(pField);

	if (!pField->isCollapsed())
	{
		drawNameSuffix(pField->owner(), iType, os);
		os << std::ends;
	}

	os.flush(mrDumpTarget.coldata(adcui::IBinViewModel::CLMN_NAMES));
}

void BinDumper_t::drawFieldExtra(CFieldPtr pField)
{
	drawRowExtra(pField->owner(), pField->_key(), pField->size());
}

void BinDumper_t::drawRowExtra(CTypePtr pScope, ADDR va, ADDR sz)
{
	if (!mpFrames->mbCollapsed)
	{
		MyString s;
		for (COLID iCol(0); iCol < (COLID)mrDumpTarget.cols().size(); iCol++)
		{
			ColDesc_t& aCol(mrDumpTarget.Column(iCol));
			if (aCol.width < 0 || !(aCol.flags & adcui::CLMNFLAGS_HEADER))
				continue;
			if (mrProject.getCellStr(iCol, pScope, va, sz, s))
				mrDumpTarget.draw(iCol, s);
		}
	}
}

static void writeSubscript(MyStrStream &os, size_t n, CTypePtr iOwner)
{
	if (iOwner && !iOwner->typeStrucvar())//cannot change size of context dependent objects
	{
		if (iOwner->typeBitset())
		{
			os << ":";
			os.color(adcui::COLOR_ARRAY);
			os << n;//iType->typeArray()->total();
			os.color(adcui::COLOR_POP);
		}
		else
		{
			os << "[";
			os.color(adcui::COLOR_ARRAY);
			os << n;//iType->typeArray()->total();
			os.color(adcui::COLOR_POP);
			os << "]";
		}
	}
	else
	{
		os << "[";
		os << n;//iType->typeArray()->total();
		os << "]";
	}
}

void BinDumper_t::drawNameSuffix(CTypePtr iOwner, CTypePtr iType, MyStrStream &os) const
{
	if (!iType)
		return;

	iType = SkipModifier(iType);
	/*if (iType->typeFuncEnd())
		;
	else */if (iType->typeCode())
		os << ":";
	else if (iType->typeArray())// && iType->typeArray()->baseType()->typeSimple())
	{
		//get all way down to the array's base type
		Array_t *pArr;
		for (CTypePtr p(iType); (pArr = p->typeArray()) != nullptr; p = pArr->baseType())
		{
			writeSubscript(os, pArr->total(), iOwner);//tmp fields don't have owner!
			iType = pArr->baseType();
		}
	}
}

void BinDumper_t::drawTypeSuffix(CTypePtr iOwner, CTypePtr iType, MyStrStream &os) const
{
	if (!iType)
		return;
	iType = SkipModifier(iType);
	if (iType->typeArray())
		drawNameSuffix(iOwner, iType, os);
}

void BinDumper_t::dumpNameRefScoped(CObjPtr pObj, std::ostream &os, CTypePtr pScope0) const
{
//CHECKID(pObj, 0x18)
//STOP
	if (!pScope0)
		pScope0 = SkipBitset(mpFrames->cont());
	CTypePtr pScope(SkipBitset(mrProject.objectDisplayScope(pObj)));
	if (pScope && pScope != pScope0)
	{
		dumpNameRefScoped(pScope, os);
		os << "::";
	}
	mrDumpTarget.drawNameRef(pObj, os);
}

void BinDumper_t::drawBitsetLine(CFieldPtr pField, CTypePtr iType, CTypePtr iType0)
{
	if (RawBlock(*mpFrames).empty())
		return;
	bool bSwapBytes(mpFrames->mbMSB != 0);//MSB
	CTypePtr iSeg(GetCurrentSeg());
	assert(!iSeg || iSeg->typeSeg()->isLittleEnd() != bSwapBytes);
	Bitset_t *pBitset(iType->typeBitset());
	
	DataSubSource_t data(GetDataSource(), RawBlock(*mpFrames));
	DataStream_t is(data);

	size_t iarr(0), arr(1);
	unsigned bytesOut(iType->sizeBytes());
	unsigned bytes(dumpDataRow(&is, uint8_t(bytesOut), ATTR_NULL, iarr, arr, bSwapBytes, iType0));
	(void)bytes;
	if (pField->isCollapsed())
	{
		ColData_t &aDat(mrDumpTarget.coldata(adcui::IBinViewModel::CLMN_DATA));
		MyStrStream os(mrDumpTarget.isColorsEnabled());
		os << " ";
		enumOpen(os);
		const FieldMap &l(pBitset->fields());
		value_t u;
		GetDataSource().dataAt(RawOffs(*mpFrames), bytesOut, (PDATA)&u);
		if (bSwapBytes)
			swap_endian(&u.ui8, bytesOut);
		value_t v(u);
		assert(bytesOut <= sizeof(u));
		int count(0);
		for (FieldMapCIt i(l.begin()); i != l.end(); i++)
		{
			CFieldPtr pField2(VALUE(i));
			CTypePtr iType2(SkipArray(pField2->type()));
			unsigned offs(pField2->offset());
			unsigned size(BitSize(pField2->type()));
			uint64_t mask1((-1) << size);
			uint64_t mask2((~mask1) << offs);
			uint64_t mask3(u.ui64 & mask2);
			if (mask3)
			{
				v.ui64 &= ~mask2;
				if (count > 0)
					os << "|";
				dumpNameRefScoped(pField2, os, SkipBitset(iType));
				if (size > 1)
				{
					mask3 >>= offs;
					os << "=";
					FieldPtr pField3(nullptr);
					if (iType2 && iType2->typeEnum())
						pField3 = Field(iType2->baseType(), (ADDR)mask3, 0, FieldIt_Exact);
					if (pField3)
						dumpNameRefScoped(pField3, os);//colornum?
					else
						os << mask3;
				}
				count++;
			}
		}
		if (v.ui64)
		{
			if (count > 0)
				os << "|";
			drawValueEx(v, uint8_t(bytesOut), ATTR_NULL, os);
		}
		enumClose(os);
		os.flush(aDat);
	}
}

void BinDumper_t::drawFieldType(CFieldPtr pField, CTypePtr iTypeProxy)
{
	if (mpFrames->mbCollapsed)
		return;

	CTypePtr iType0(CheckProxyType(pField, iTypeProxy));
	if (!iType0)
		return;

	MyStrStream os(mrDumpTarget.isColorsEnabled());

	CTypePtr iType(SkipModifier(iType0));
	if (!iType->isShared() && !iType->objGlob())
	{
		if (iType->typeProc() && !pField->isCollapsed())
			return;
		if (iType->typeArray())
			return;//dumped with name
		if (iType->nameless() && !iType->typeUnion())
		{
			if (iType->ObjType() == OBJID_TYPE_STRUC)
			{
				const FieldMap& m(iType->typeStruc()->fields());
				if (!m.empty())
				{
					CFieldPtr pField2(VALUE(m.begin()));
					if (pField2 == VALUE(m.rbegin()))//the only child
					{
						if (pField2->offset() == 0 && pField2->type() && pField2->type()->typeBitset())
						{
							drawBitsetLine(pField, pField2->type(), iType0);
							return;
						}
					}
				}
			}
			return;
		}

		MyString s;
		if (iType->typeSeg())
			s = "; segment";
		else if (iType->typeUnion())
			s = "; union";
		else if (iType->typeProc())
			s = "; function";

		//MyString s(mrProject.typeDisplayName(iType));
		if (!s.empty())
		{
			adcui::Color_t eColor(adcui::COLOR_NULL);
			if (s.startsWith(";"))
				eColor = adcui::COLOR_UNEXPLORED;
			else
				eColor = DumpTarget_t::fieldColor(pField);
			if (eColor != adcui::COLOR_NULL)
				os.color(eColor);
			os << s;
			if (eColor != adcui::COLOR_NULL)
				os.color(adcui::COLOR_POP);
			ColData_t &aDat(mrDumpTarget.coldata(adcui::IBinViewModel::CLMN_DATA));
			os.flush(aDat);
			return;
		}
	}

	if (iType->ObjType() == OBJID_TYPE_STRUC)
	{
		const FieldMap &m(iType->typeStruc()->fields());
		if (!m.empty())
		{
			CFieldPtr pField2(VALUE(m.begin()));
			if (pField2 == VALUE(m.rbegin()))//the only child
			{
				if (pField2->offset() == 0 && pField2->type() && pField2->type()->typeBitset())
				{
					drawBitsetLine(pField, pField2->type(), iType0);
					return;
				}
			}
		}
	}

	if (pField->isCollapsed())
	{
		iType = SkipArray(iType);
		if (iType->typeSimple())
		{
			int size0(iType->size());
			if (!mrDumpTarget.isValuesOnlyMode())
				if (!iType->nameless())//enum?
				{
					dumpNameRefScoped(iType, os);
					os << " ";
				}
			os << STR_DATASZ(size0);
		}
		else if (iType->typeComplex())
		{
			if (!mrDumpTarget.isValuesOnlyMode())
				dumpNameRefScoped(iType, os);
		}
		drawTypeSuffix(pField->owner(), iType0/*pField->type()*/, os);
	}
	else
	{
		if (iType->typeComplex())//&& !iType->typeSeg())
		{
			if (!iType->typeCode() && (iType->isShared() || iType->objGlob()))
			{
				//MyStrStream os;
				if (iType->typeProc())
					os << TypeName(iType);//?
				else if (!mrDumpTarget.isValuesOnlyMode())
					dumpNameRefScoped(iType, os);

				//			for (std::list<size_t>::const_iterator i(lArr.begin()); i != lArr.end(); i++)
				//				os << "[" << *i << "]";
			}
		}
	}

	ColData_t &aDat(mrDumpTarget.coldata(adcui::IBinViewModel::CLMN_DATA));
	os.flush(aDat);
}







////////////////////////////////
// COUTbin_t

COUTbin_t::COUTbin_t(TypePtr iModule, /*ROWID scopeBase, ROWID scopeSize,*/ int /*w_row*/)
	: CACHEbin_t(iModule/*, scopeBase, scopeSize*/)//iScope->typeSeg()->view Offs())
{
	mpInvariant = new DumpInvariant_t(false, false);
	mpInvariant->initColumns(iModule);
	mpTarget = new DumpTarget_t(*mpInvariant);
	//mpTarget->setRowIdLen(w_row);// AddressWidth(Project_t::instance()));
}

COUTbin_t::~COUTbin_t()
{
	delete mpTarget;
	delete mpInvariant;
}

void COUTbin_t::outputRow(adcui::DUMPOS iPos, std::ostream &os, DumpVisitor_t &rDumper)
{
	//CHECK(rowID == 0x5BA7)
	//STOP
	std::string	outBuf;
	int iLevel(0);

	int i = 0;
	size_t x = 0, xpr = 0;
	
	for (COLID iCol(0); iCol < (COLID)target().cols().size(); iCol++)
	{
		ColDesc_t &aCol(target().Column(iCol));
		int colw(aCol.width);

		MyString sCellData(rDumper.CellDataItEx(iPos, iCol, nullptr, nullptr));
		if (!sCellData.empty())
		{
			/*if (sCellData.startsWith("Child#"))
			{
				STOP
			}*/
			int nCellDataLen((int)sCellData.length());
			if (adcui::IBinViewModel::CLMN_TREEINFO == i)
			{
				iLevel = nCellDataLen;
			}
			else
			{
				int len(std::min(nCellDataLen, colw));
				if (len > 0)
				{
					size_t xn = x;
					if (target().isTreeColumn(iCol))
						if (iLevel > 0)
						{
							int offs = iLevel * 4;
							xn += offs;
							len = std::min(nCellDataLen, colw - offs);
						}

					if (len > 0)
					{
						while (xn > xpr++)
							outBuf.append(" ");
						if (aCol.flags & (adcui::CLMNFLAGS_HEADER | adcui::CLMNFLAGS_TRIM))
							sCellData.truncate(len);
						//outBuf.append(sCellData.substr(0, len));
						if (!outBuf.empty() && !isspace(*outBuf.rbegin()))
							outBuf.append(" ");
						outBuf.append(sCellData);
						xpr = xn + len;
					}
				}
			}
		}
		if (colw > 0)
			x += colw;
		i++;
	}
	os << outBuf;
	os << std::endl;
}


