#include "type_seg.h"
#include <fstream>
#include "inttypes.h"//PRIx64
#include "prefix.h"

#include "shared/defs.h"
#include "shared/link.h"
#include "shared/misc.h"
#include "shared/data_source.h"
#include "info_proj.h"
#include "obj.h"
#include "field.h"
#include "names.h"
#include "type_module.h"
#include "main.h"


///////////////////////////////////////////

Seg_t::Seg_t()//TypePtr pRange)
	:
//#if(USE_SEG_RANGES)
	//mpRange(pRange),
//#else
	mBase64(0),
//#endif
	mFlags(0),
	//mbLittleEnd(false),
	//mpSegRef(nullptr),
	//mpInferior(nullptr),
	mpTraceLink(nullptr),
	mAddress(0),
	//mValidRange(0),
	miFront(0),
	mEntryPoint(0),
	//mpSegRange(nullptr),
	mpSuperLink(nullptr),
	mViewOffs((ROWID)-1),
	mViewSize0((ROWID)-1)//,
	//mSize(-1)
{
}

Seg_t::~Seg_t()
{
	assert(mSubSegs.empty());
	assert(!mpSuperLink);
	//assert(!mpInferior);
	assert(!mpTraceLink);
	//assert(!mpSegRange);
	assert(!mpTypesMgr);
	assert(!miFront);
	//clear();
}

FRONT_t::FRONT_t(const char *name, int id)
: m_id(id),
m_name(name),
mpIFront(nullptr)
{
}

FRONT_t::~FRONT_t()
{
	releaseDevice();
	//assert(!mpIFront);
}

I_Front *FRONT_t::device(DataPtr pData)
{
	if (!mpIFront)
		mpIFront = MAIN.getFrontend(m_name.c_str(), pData ? &pData->pvt() : nullptr);
	return mpIFront;
}

I_Front *FRONT_t::device(const I_DataSourceBase *p)
{
	if (!mpIFront)
		mpIFront = MAIN.getFrontend(m_name.c_str(), p);
	return mpIFront;
}

void FRONT_t::releaseDevice()
{
	if (mpIFront)
	{
		mpIFront->release();
		mpIFront = nullptr;
		MAIN.releaseFrontend(name().c_str());
	}
}

/*void Seg_t::setFrontEnd(const char *frontName, int id)
{
	if (!frontName || !frontName[0])
	{
		clearFrontEnd();
		return;
	}
	if (!mpFront)
		mpFront = new FRONT_t(frontName, id);
	assert(mpFront->name() == frontName);//in case it wasn't
}

void Seg_t::clearFrontEnd()
{
	if (mpFront)
	{
		mpFront->Release();
		mpFront = nullptr;
	}
}

void Seg_t::releaseDevice()
{
	if (miFront)
		mpFront->releaseDevice();
}*/

/*int Seg_t::addEntryPointV(ADDR addr)
{
	TypePtr pSeg = findSubseg(addr);
	if (pSeg == nullptr)
		return 0;
	pSeg->typeSeg()->setEntryPoint(addr);
	return 1;
}*/

FieldMapIt Seg_t::NextNL(FieldMapIt it)
{
assert(0);/*	FieldPtr  pField = *it;
	assert(!pField->IsLa bel() || !((Label_t *)pField)->mpPath);

	if (pField->Is Func())
	{
		Label_t * pLabelExit = ((TypeProc_t *)pField)->getTailLabel();
		if (pLabelExit)
			pField = pLabelExit;
	}
	FieldPtr  pMLocNx = (FieldPtr )pField->Ne xt();
	return pMLocNx;*/
	return ++it;
}

int Seg_t::CountNL()//count mlocs but labels
{
	int i = 0;
	for (FieldMapIt it = mFields.begin(), E = mFields.end(); it != E; it = NextNL(it))
	{
		i++;
	}

	return i;
}

/*int Seg_t::MoveToSeg(FieldPtr  pField)
{
	if (Type() == SEG_UNK)
		return 0;

	if (pField->ownerProcPvt())
		return 0;

	Struc_t * pSeg0 = pField->GetOwn erSeg()->typeSeg();
	if (pSeg0 == this)
		return -1;

	if (!AddObj(pField, pField->offset()))
		return 0;

	AdjustSegPos(pField);
	return 1;
}*/

int Seg_t::AdjustSegPos(FieldPtr  pField)
{
assert(0);/*	while (!pField->IsFirst())
	{
		FieldPtr  pMLocPr = (FieldPtr )pField->Pr ev();
		if (pMLocPr->offset() <= pField->offset())
			break;
		pField->Shift(-1);
	}
	while (!pField->IsLast())
	{
		FieldPtr  pMLocNx = (FieldPtr )pField->N ext();
		if (pMLocNx->offset() > pField->offset())
			break;
		pField->Shift(1);
	}*/

	return 1;
}



void Seg_t::namex(MyString &s) const
{
	s = mTitle;
	//if (s.empty())
		//s = ".seg";
}

Seg_t *	Seg_t::Split(uint32_t addr)
{assert(0);/*
	if (m_pChilds)
		return 0;
	if (!__isMineAddr(addr))
		return 0;
	if (Get Base() == addr)
		return 0;

	Seg_t * pSeg = new Seg_t;
	pSeg->LinkAfter(this);
	pSeg->m_pParent = m_pParent;
	pSeg->m_nFlags = m_nFlags;
	pSeg->setBase(addr);
	pSeg->SetSize(size() - (addr - Get Base()));

	SetSize(size() - pSeg->size());

	for (FieldPtr  pLoc = m_pFields;//Locs;
	pLoc;
	pLoc = (FieldPtr )pLoc->Ne xt())
	{
		uint32_t addr2 = pLoc->offset();
		if (addr2 != BAD_ADDR)
		if (addr2 >= addr)
			break;
	}

	while (pLoc)
	{
		FieldPtr  pLocNx = (FieldPtr )pLoc->N ext();
		pLoc->LinkTail((Link_t **)&pSeg->m_pFields);//Locs);
		pLoc = pLocNx;
	}

	for (Block_t * pBlock = m_pBlocks;
	pBlock;
	pBlock = (Block_t *)pBlock->Ne xt())
	{
		uint32_t addr2 = pBlock->GetAddress();
		assert(addr2 != BAD_ADDR);
		if (addr2 >= addr)
			break;
		uint32_t size2 = pBlock->GetSize();
		if (addr2 + size2 > addr)
			pBlock->Split(addr);
	}

	while (pBlock)
	{
		Block_t * pBlockNx = (Block_t *)pBlock->Ne xt();
		pBlock->LinkTail((Link_t **)&pSeg->m_pBlocks);
		pBlock->mOffs -= pSeg->Get Base() - Get Base();
		pBlock = pBlockNx;
	}

	return pSeg;*/return 0;
}














////////////////////////////////////////////////////////


/*int Seg_t::undefineObj( ADDR d )
{

	for ( mapSEG_it its = mSegs.begin(); its != mSegs.end(); ++its ) 
	{
		Seg_t * pSeg = its->second;
		int ret = pSeg->undefineObj( d );
		if ( ret >= 0 )
			return ret;
	}

	ADDR addr;
	if ( !offs2addr( d, addr ) )
		return -1;
	if ( !contains( addr ) )
		return -1;

	mapGLOB_it it = mMLOCs.find( addr );
	if ( it == mMLOCs.end() )
		return 0;

	GLOB_t * pObj = it->second;
	return pObj->convertObjType( nullptr );
return 0;
}*/

/*int Seg_t::deleteObj( ADDR addr )
{

	for ( mapSEG_it its = mSegs.begin(); its != mSegs.end(); ++its ) 
	{
		Seg_t * pSeg = its->second;
		int ret = pSeg->deleteObj( d );
		if ( ret >= 0 )
			return ret;
	}

	ADDR addr;
	if ( !offs2addr( d, addr ) )
		return -1;
	if ( !contains( addr ) )
		return -1;

	mapGLOB_it it = mMLOCs.find( addr );
	if ( it == mMLOCs.end() )
		return 0;

	GLOB_t * pObj = it->second;
	mMLOCs.erase( it );
	delete pObj;
	return 1;
}*/



/*int Seg_t::setSize(TypePtr iSelf, int sz)
{
	if (sz >= 0)
	{
		if (!mFields.empty())
		{
			FieldMapRIt rit(mFields.rbegin());
			FieldPtr pField(VALUE(rit));
			int iFieldSize(pField->size());
			if (iFieldSize < 0)//code?
				iFieldSize = 0;
			int sizeMax(pField->offset() - iSelf->base() + iFieldSize);
			if (sz > sizeMax)
				mSize = sz;
			else
				mSize = -1;//determined by the last field
		}
		else
			mSize = sz;
	}
	return mSize;
}*/

int Seg_t::size(CTypePtr pSelf) const
{
	//boundless strucs are limited by offset+size of it's last field,
	//	segs must be bound explicitly (by eos), otherwise they are expand infinitely
	//	modules (modules) are limited by it's raw data

	int sz(size0(pSelf));
	if (sz == -1)//?
	{
		TypePtr pTrace(traceLink());
		if (pTrace)
			return pTrace->size();
		sz = Struc_t::size(pSelf);

	}
	return sz;
}

int Seg_t::size0(CTypePtr iSelf) const
{
	FieldPtr pField(iSelf->parentField());
	if (!pField)
		return -1;//limitless?

	int sz(0);
	TypePtr pRangeSet(pField->owner());
	assert(pRangeSet->typeSeg());
	const FieldMap& m(pRangeSet->typeSeg()->fields());
	RangeCIt it(m.find(pField->_key()));

	assert(it != m.end());
	RangeCIt itnx(it);
	itnx++;
	if (itnx != m.end())
	{
		CFieldPtr pFieldNx(VALUE(itnx));
		int sz2(KEY(itnx) - KEY(it));
		if (!pFieldNx->type())//explicit eos?
			sz = sz2;
		else
		{
			assert(pFieldNx->isTypeSeg());
			if (subsegs().empty())//terminal seg
				sz = sz2;
			else if (sz2 > sz)//non-terminal seg
				sz = sz2;
		}
	}

	if (sz == 0)
		sz = -1;//limitless?

	return sz;
}

ADDR Seg_t::segSize(CTypePtr pSelf) const
{
	if (!subsegs().empty())
		return 0;

	int sz(0);
	TypePtr iTrace(traceLink());
	if (iTrace)
		sz = iTrace->size();

	FieldPtr pField(pSelf->parentField());
	if (pField)
	{
		TypePtr pRangeSet(pField->owner());
		assert(pRangeSet->typeSeg());
		const FieldMap& m(pRangeSet->typeSeg()->fields());
		RangeCIt it(m.find(pField->_key()));

		assert(it != m.end());
		RangeCIt itnx(it);
		itnx++;
		if (itnx != m.end())
		{
			CFieldPtr pFieldNx(VALUE(itnx));
			int sz2(KEY(itnx) - KEY(it));
			if (!pFieldNx->type())//explicit eos?
				sz = sz2;
			else
			{
				assert(pFieldNx->isTypeSeg());
				if (subsegs().empty())//terminal seg
					sz = sz2;
				else if (sz2 > sz)//non-terminal seg
					sz = sz2;
			}
		}
	}

	if (sz == 0)
		sz = -1;//limitless?

	return sz;
}

/*Seg_t * Seg_t::addSegment( const char * name, ADDR base, int size, int flags )
{
	Seg_t * pSEG = new Seg_t( name, this );
	pSEG->m Base = base;
	pSEG->mSize = size;
	pSEG->mFlags = flags;
	//strncpy( pSEG->mName, name, NAME_LEN_MAX );

	mSegs.insert( std::pair<ADDR, Seg_t *>( base, pSEG ) );

	OUTPUT.printf("Segment created: %s, BASE=%08X, END=%08X, SIZE=%08X", 
		name, base, base+size-1, size );
	return pSEG;
}*/

//subsegs must go in order in regard to its traces, except when there is no trace.
bool Seg_t::registerSubseg0(TypePtr iSeg0)
{
	Seg_t &rSeg0(*iSeg0->typeSeg());
	for (SubSegMapCIt i(subsegs().begin()); i != subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));
		Seg_t &rSeg(*iSeg->typeSeg());
		ADDR ra(rSeg.addressP());
		ADDR ra0(rSeg0.addressP());
		if (ra0 < ra)
		{
			subsegs().insert(i, iSeg0);
			return true;
		}
	}

	subsegs().push_back(iSeg0);
	return true;


//	return ret.second;
}

bool Seg_t::removeSubseg(TypePtr iSubSeg)
{
	for (SubSegMapIt i(subsegs().begin()); i != subsegs().end(); i++)
	{
		if (*i == iSubSeg)
		{
			subsegs().erase(i);
			return true;
		}
	}
	return false;
}

TypePtr Seg_t::findSubseg(ADDR addr, unsigned uAffinity) const
{
	for (SubSegMapCIt i(subsegs().begin()); i != subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));
		if (iSeg->typeSeg()->affinity() == uAffinity)
			if (iSeg->base() <= addr)
				if (addr < iSeg->base() + iSeg->size())
					return iSeg;
	}
	return nullptr;
}

TypePtr Seg_t::findSubseg64(ADDR64 addr) const
{
	for (SubSegMapCIt i(subsegs().begin()); i != subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));
		const Seg_t &rSeg(*iSeg->typeSeg());
		if (rSeg.affinity() != affinity())
			continue;
		ADDR64 tvaBase(iSeg->imageBase() + iSeg->base());
		if (tvaBase <= addr && addr < tvaBase + iSeg->size())
			return iSeg;
	}
	return nullptr;
}

FieldPtr Seg_t::FindFieldV(ADDR addr0)
{
	assert(addr0 != -1);

	for (FieldMapIt it = fields().begin(), E = fields().end(); it != E; it++)
	{
		FieldPtr pField(VALUE(it));
		TypePtr pType = pField->type();
		if (!pType)
			continue;
		Seg_t * pSeg(pType->typeSeg());
		if (!pSeg)
			continue;
		FieldPtr pLoc(ProjectInfo_t::__findFieldV(pType, addr0, ProjectInfo_t::FieldIt_Exact, true));
		if (pLoc)
			return pLoc;
	}

	return 0;
}

bool ProjectInfo_t::terminalFieldAtSeg(CTypeBasePtr iSelf, DA_t &da, Locus_t &l, Block_t rb) const
{
	Seg_t &rSelf(*iSelf->typeSeg());

	DA_t daj(da);
	//check if 'da' falls into a segment
	for (SubSegMapCIt i(rSelf.subsegs().begin()); i != rSelf.subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));
		Seg_t &rSeg(*iSeg->typeSeg());

//CHECK(!pSeg0)
//STOP
		FieldPtr pField(nullptr);

		TypePtr pTrace(rSeg.traceLink());
		if (pTrace)
			pField = pTrace->parentField();

		ROWID vo(rSeg.viewOffs());
		if (vo > da.row)
			break;

		DA_t da2(da);
		if (terminalFieldAtSeg(iSeg, da2, l, rSeg.rawBlock()))
		{
			l.push_front(Frame_t(rb, (TypePtr)iSelf, iSelf->base() + (ADDR)rawOffs(iSeg), pField));
			return true;
		}

		daj.row -= SegOverSize0(iSeg);
	}

CHECKID(iSelf, 0x1f)
STOP

	ROWID lower(rSelf.viewOffs());
	ROWID upper(lower + rSelf.viewSize0());// iSelf->size());
	if (daj.row < upper)
	{
		ADDR va(iSelf->base() + ADDR(daj.row - lower));
		l.push_front(Frame_t(rb, (TypePtr)iSelf, va, nullptr));
		terminalFieldAt(l, daj.bit);
		//DA_t da2(va, 0, daj.bit, daj.adjust);
		//if (!terminalFieldAt(iSelf, da2, l, rb))
			//l.push_front(Frame_t(rb, iSelf, (ADDR)da2.row, nullptr));
		return true;
	}

	ROWID upperp(lower + SegTraceSize(iSelf));
	if (daj.row < upperp)
	{
		TypePtr iSuper(rSelf.traceLink());
		Struc_t &rSuper(*iSuper->typeStruc());
		ADDR va(iSuper->base() + ADDR(daj.row - lower));
		//DA_t da2(va, 0, 0);
		Block_t rb2(rb);
		rb2.m_size = SegTraceSize(iSelf);
		l.push_front(Frame_t(rb2, rSelf.traceLink(), va, nullptr));
		terminalFieldAt(l, daj.bit);
		//if (!terminalFieldAt(iSuper, da2, l, rb2))//rSuper.rawBlock()
			//l.push_front(Frame_t(rb2, rSelf.traceLink(), va, nullptr));//rSuper.rawBlock()
		return true;
	}
	return false;
}

/*ROWID Seg_t::view Size()//not considering size in parent
{
	if (mViewSize == (ROWID)-1)
	{
		mViewSize = size();
		if (mpInferior)
		{
			ROWID vsz(mpInferior->view Size());
			mViewSize = std::max(mViewSize, vsz);
		}
		else
		{
			for (SubSegMapCIt i(subsegs().begin()); i != subsegs().end(); i++)
			{
				Seg_t *pSeg(VALUE(i));
				ROWID vsz(pSeg->view Size());
				ROWID szp(pSeg->SegTraceSize());
				if (vsz > szp)
					mViewSize += (vsz - szp);
			}
		}
	}
	return mViewSize;
}*/

/*void ProjectInfo_t::UpdateViewGeometry()
{
	ROWID da(0);
	for (FileTree_t::ChildrenIterator it(mrProject.files().rootFolder()); it; it++)
	{
		Folder_t &rFolder(*it.data());
		if (!IsPhantomFolder(rFolder))
		{
			TypePtr iModule(ModuleOf(rFolder));
			da = updateViewGeometry2(iModule, da);
			da += SegOverSize0(iModule);
		}
	}
}*/

ROWID Seg_t::viewOffsAt(CTypePtr iSelf, ADDR va) const 
{
	assert(mViewOffs != (ROWID)-1 && va >= base(iSelf));
	ADDR offs(va - base(iSelf));
	ADDR_RANGE extra(0);
	for (SubSegMapCIt i(subsegs().begin()); i != subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));
		const Seg_t &rSeg(*iSeg->typeSeg());
		if (rSeg.addressP() >= va)
			break;
		offs += (ADDR)ProjectInfo_t::SegOverSize0(iSeg);
	}

	return mViewOffs + offs;
}

/*void Seg_t::recoverRawPtr(PDATA pData)
{
	for (SubSegMapCIt i(subsegs().begin()); i != subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));
		iSeg->typeSeg()->recoverRawPtr(pData);
	}
	if (mRaw.size)
		mRaw.ptr = pData + mRaw.offs;
}*/

/*ADDR Seg_t::base() const
{
	if (type obj()->parentField())
		return type obj()->parentField()->_key();
	return 0;
}*/


/*Seg_t *Seg_t::entryPointV(ADDR &a) const
{
	for (SubSegMapCIt i(sub segs().begin()); i != sub segs().end(); i++)
	{
		Seg_t *pSeg(VALUE(i));
		if (pSeg->mEntryPoint != 0)
		{
			a = pSeg->mEntryPoint;
			return pSeg;
		}
	}
	return nullptr;
}*/

ADDR ProjectInfo_t::baseOf(TypePtr p) const
{
	return p->base();
}

TypePtr ProjectInfo_t::findEntryPoint(TypePtr iSeg, ADDR &va) const
{
	const Seg_t &rSelf(*iSeg->typeSeg());
	if (rSelf.mEntryPoint != 0)
	{
		va = rSelf.mEntryPoint;
		return iSeg;
	}

	for (SubSegMapCIt i(rSelf.subsegs().begin()); i != rSelf.subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));//look only virtual addresses
		TypePtr iSeg2(findEntryPoint(iSeg, va));
		if (iSeg2)
			return iSeg2;
	}

	return nullptr;
}

TypePtr ProjectInfo_t::findCodeSeg(CTypePtr iSeg) const
{
	const Seg_t &rSeg(*iSeg->typeSeg());//look only for real segments (not seg traces)
	for (SubSegMapCIt i(rSeg.subsegs().begin()); i != rSeg.subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));
		TypePtr iSeg2(findCodeSeg(iSeg));
		if (iSeg2)
			return iSeg2;
	}

	if (rSeg.uflags() & I_SuperModule::ISEG_CODE)
		return (TypePtr)iSeg;

	return nullptr;
}

bool ProjectInfo_t::IsTerminalSeg(TypePtr iSeg) const
{
	Seg_t &rSeg(*iSeg->typeSeg());
	if (rSeg.subsegs().empty())
		if (!iSeg->parentField())//not a seg trace
			return true;
	return false;
}

ADDR Seg_t::addressP2(TypePtr iSelf) const
{
	if (traceLink())
	{
		FieldPtr pField(traceLink()->parentField());
		return pField->_key();
	}
	assert(addressP() - iSelf->base() == rawBlock().m_offs);
	return addressP();//rSeg.rawBlock().offs;
}


/*FieldMapIt Seg_t::field_it(ADDR &addr, FieldMapIt *ppNext)
{
	return field_itx(addr, ppNext);
}*/

/*#if(1)
FieldMapIt Seg_t::field_itx(ADDR &addr, FieldMapIt *ppNext, int * pnOversize)
{
	for (SubSegMapCIt i(sub segs().begin()); i != sub segs().end(); i++)
	{
		Seg_t *pSeg0(VALUE(i));
		ADDR segOffs(KEY(i));
		if (addr < segOffs)
			break;

		Seg_t *pSeg(pSeg0);
		if (pSeg->infer Link())
			pSeg = pSeg->infer Link()->typeSeg();

		ADDR_RANGE sz = (ADDR_RANGE)pSeg0->view Size();

		if (addr - segOffs < sz)
		{
			addr -= segOffs;
			addr += pSeg->base();
			return Struc_t::field_it(segOffs, ppNext);
		}

		ADDR_RANGE szP = pSeg->rawSize();

		int d = sz - szP;
		if (d > 0)
		{
			addr -= d;
			if (pnOversize)
				*pnOversize += d;
		}
	}

	ADDR addr_v = / *base()+* /addr;
	FieldMapIt itf = Struc_t::field_it( addr_v, ppNext );
	if (itf != fields().end())
	{
		ADDR offs = itf->first;
		FieldPtr  p = itf->second;
#if(0)//?
		if (p->type() && p->type()->typeSeg())
		{
			itf = fields().end();
//			if (ppNext)
//				*ppNext = it;
		}
		else
#endif
		{
			int sz = p->size();
			if (sz <= 0)
				sz = 1;

			if (addr < offs + sz)
			{
			}
			else
			{
				itf = fields().end();
//				if (ppNext)
//					*ppNext = it;
			}
		}
	}

	return itf;
}


#else
FieldMapIt Seg_t::field_it( ADDR &addr, FieldMapIt *ppNext )
{
	int size0 = 0;

	mapSEG_it it;
	for ( it = mSegs.begin(); it != mSegs.end(); ++it ) 
	{
		Seg_t * pSeg = it->second;
		ADDR offs = it->first;

		if ( offs < (ADDR)size0 )
		{
			int d = size0-offs;
			addr -= d;
			size0 -= d;
		}

		if ( (int)addr < offs )
			break;

		int size = pSeg->size();
		if ( (int)addr < offs+size )
		{
			//addr = offs;
			return Struc_t::field_it( offs, ppNext );
		}

		if ( offs > (ADDR)size0 )
		{
			size0 = offs;
		}

		size0 += size;
	}

	ADDR addr_v = *//*base()+*//*addr;
	return Struc_t::field_it( addr_v, ppNext );
}
#endif*/


void ProjectInfo_t::SegJustCreated(TypePtr iSelf) const
{
	Seg_t &rSelf(*iSelf->typeSeg());
	//TypePtr iSeg(type obj());
	//if (bEcho)
	{
		const Seg_t *pSeg(&rSelf);
		/*MyString s;
		if (!pSeg->nameless())
			s = pSeg->name()->c_str();
		if (s.empty())
		{
			FieldPtr pField(pSeg->typ eobj()->parentField());
			//if (pSeg->segRef())
			if (pField)
			{
				//s = pSeg->segRef()->parentField()->namexx();
				s = pField->namexx();
			}
		}
		if (s.empty() && pSeg->traceLink())
		{
			Seg_t *pSeg0(pSeg->traceLink()->typeSeg());
			FieldPtr pField(pSeg0->type obj()->parentField());
			if (pField)
				s = pField->namexx();
		}
		if (name)
			s = name;
		if (s.empty())
			s = "?";*/

		MyString s(iSelf->typeSeg()->title());
		if (s.empty())
			s = "<noname>";

		MyString sOrphan;
		if (!pSeg->ownerRangeSet(iSelf))
			sOrphan = " (orphan)";

		ADDR bs(iSelf->base());
		unsigned sz(iSelf->size());
		unsigned ro((unsigned)rawOffs(iSelf));
		unsigned rs((unsigned)rawSize(iSelf));
#if(0)
		fprintf(stdout, "Segment created: %s, RANGE=[%08X, %08X), SIZE=%08X(%08X)\n",
			s.c_str(), pSeg->base(), pSeg->base() + pSeg->size(), pSeg->size(), pSeg->sizep());
#else
		fprintf(stdout, "Segment%s created: %s, BASE=%s(.%08X), END=%s(.%08X), SIZE=%08X(%08X)\n",
			sOrphan.c_str(), s.c_str(),
			VA2STR(iSelf, bs, -1).c_str(), ro,
			VA2STR(iSelf, sz == 0 ? bs : bs + sz - 1, -1).c_str(), rs == 0 ? ro : ro + rs - 1,
			sz, rs);
#endif
	}
}

static std::ifstream::pos_type filesize(const char* filename)
{
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	return in.tellg();
}

/*bool Block_t::loadFromFile(const std::string &sPath)
{
	assert(!ptr);

	std::ifstream in(sPath.c_str(), std::ifstream::ate | std::ifstream::binary);
	if (!in.is_open())
		return false;

	std::ifstream::pos_type lSize(in.tellg());

	in.seekg(std::ifstream::beg);

	ptr = new char[(int)lSize];

	in.read((char *)ptr, lSize);

	if (in)//C++11?
	{
		//std::cout << "all characters read successfully.";
	}
	else
	{
		//std::cout << "error: only " << is.gcount() << " could be read";
		delete [] ptr;
		ptr = nullptr;

	}
	in.close();

	size = (unsigned)lSize;
	return true;
}*/

/*bool Block_t::uploadFromFile(const std::string &sPath, Block_t &other)
{
	std::ifstream in(sPath.c_str(), std::ifstream::ate | std::ifstream::binary);
	if (!in.is_open())
		return false;

	std::ifstream::pos_type lSize(in.tellg());

	in.seekg(std::ifstream::beg);

	if (!ptr)
	{
		size = (unsigned)lSize;
		ptr = new char[size];
		other = *this;
		in.read((char *)ptr, size);
	}
	else
	{
		assert(0);
		Block_t old(*this);
		size = unsigned(old.size + lSize);
		ptr = new char[size];
		std::copy((char *)old.ptr, (char *)(old.ptr+old.size), (char *)ptr);
		other.offs = old.size;
		other.size = (unsigned)lSize;
		other.ptr = ptr + old.size;
		in.read((char *)other.ptr, other.size);
	}

	if (in)//C++11?
	{
		//std::cout << "all characters read successfully.";
	}
	else
	{
		assert(0);
		//std::cout << "error: only " << is.gcount() << " could be read";
		delete[] ptr;
		ptr = nullptr;

	}
	in.close();

	return true;
}
*/

#if(NEW_VARSIZE_MAP)
void Seg_t::setVarObjSize(ADDR rowID, unsigned sz)
{
	std::pair<std::map<ADDR, unsigned>::iterator, bool> it;
	it = mCodeSzMap.insert(std::make_pair(rowID, (unsigned)sz));
//?	assert(it.second);
}

unsigned Seg_t::varObjSize(ADDR va) const
{
	std::map<ADDR, unsigned>::const_iterator it;
	it = mCodeSzMap.find(va);
	if (it == mCodeSzMap.end())
		return -1;
	return it->second;
}
#endif


//////////////////////Seg_t


TypePtr Seg_t::ownerRangeSet(CTypePtr iSelf)
{
	FieldPtr pField(iSelf->parentField());
	if (pField)
	{
		TypePtr pRangeSeg(pField->owner());
		if (pRangeSeg && pRangeSeg->typeSeg())
			return pRangeSeg;
	}
	return nullptr;
}

ADDR Seg_t::base(CTypeBasePtr pSelf0) const
{
	CTypePtr pSelf((CTypePtr)pSelf0);
	FieldPtr pField(pSelf->parentField());
	if (!pField)
		return 0;
	return pField->_key();
}

ADDR64 Seg_t::imageBase(CTypePtr iSelf) const 
{
	TypePtr iRangeSeg(ownerRangeSet(iSelf));
	if (!iRangeSeg)
		return base64();
	//if (!superLink())
		//return 0;
	//return superLink()->typeSeg()->rangeSet(mpRange->iOwner)->imageBase;
	return iRangeSeg->imageBase();
}

char * Seg_t::printVA(CTypePtr pSelf, ADDR va, char buf[32], unsigned minw)
{
	const Seg_t* pSeg(pSelf->typeSeg());
	assert(pSeg);
	if (pSeg->isLarge())
	{
		ADDR64 va64(pSelf->imageBase() + va);
		unsigned w(HexWidthMax(va64));
		if (minw > 0 && w < minw)
			w = minw;
		char fmt[8];
		sprintf(fmt, "%%0%d" PRIX64, w);
		sprintf(buf, fmt, va64);
	}
	else
	{
		assert(pSelf->imageBase() == 0);
		sprintf(buf, "%08X", va);// + (ADDR)iSeg->imageBase());
	}
	return buf;
}

void* Seg_t::ADDR2PV(CTypePtr pSelf, ADDR va)
{
	const Seg_t* pSeg(pSelf->typeSeg());
	assert(pSeg);
	if (pSeg->isLarge())
	{
		if (sizeof(void*) == 4)
			ASSERT0;
		ADDR64 va64(pSelf->imageBase() + va);
		return reinterpret_cast<void*>(va64);
	}
	if (sizeof(void*) == 8)
		ASSERT0;
	union { ADDR a; void* pv; } u;
	u.a = va;
	return u.pv;
}

ADDR Seg_t::PV2ADDR(CTypePtr pSelf, void* pv)
{
	const Seg_t* pSeg(pSelf->typeSeg());
	assert(pSeg);
	if (pSeg->isLarge())
	{
		if (sizeof(void*) == 4)
			ASSERT0;
		union { ADDR64 a64; ADDR a; void* pv; } u;
		u.pv = pv;
		u.a64 -= pSelf->imageBase();
		assert(u.a64 < 0xFFFFFFFF);
		return u.a;
	}
	if (sizeof(void*) == 8)
		ASSERT0;
	union { ADDR a; void* pv; } u;
	u.pv = pv;
	return u.a;
}
