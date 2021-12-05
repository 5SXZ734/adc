/////////////////////////////////////////////////////
// NEW SIMPLIFY

	void SimplifyNew(Out_t *) const;
	int PreSimplifyNew(Out_t *, int) const;
	int PrimarySimplifyNew(Out_t *, int) const;
	int SecondarySimplifyNew(Out_t *, int) const;
	int PostSimplifyNew(Out_t *, int) const;

int EXPRSimpl_t::PreSimplifyNew(Out_t *pSelf, int i) const
{
	__dump(i++, pSelf);
	for (;;)
	{
		bool b(false);
		for (Out_t::Iterator it(pSelf); it; ++it)
		{
			Out_t* pCur(it.top());
			if (SimplifyGet(pCur, 0))
			//	|| SimplifyAssign(pCur))
			{
				__dump(i++, pSelf);
				b = true;
				break;
			}
		}
		if (!b)
			break;
	}
	return i;
}

int EXPRSimpl_t::PrimarySimplifyNew(Out_t *pSelf, int i) const
{
	for (;;)
	{
CHECK(checkDump(i, 10))
STOP
		bool b(false);
		for (Out_t::Iterator it(pSelf); it; ++it)
		{
			Out_t *pCur(it.top());
			if (SimplifyType1(pCur)
				//---------DispatchSimplify_2
				|| SimplifyGet(pCur, 1)
				|| SimplifyAssign(pCur)
				|| SimplifyFract(pCur)
				|| SimplifySign(pCur)
				|| SimplifyAdd(pCur)
				|| SimplifySub(pCur)
				|| SimplifyMultiply(pCur)
				|| SimplifyDivide(pCur)
				|| SimplifyShiftLeft(pCur)
				|| SimplifyShiftRight(pCur)
				|| SimplifyBitwiseAnd(pCur)
//				|| Simplify1_1(pCur)
				|| SimplifyHalf1(pCur)//post
				|| SimplifyAndOr(pCur)
				|| SimplifyCheck(pSelf)
				|| SimplifyHalf2(pCur)//post
				|| SimplifyCommonMult(pCur)
				//-----------
				|| SimplifyType2(pCur)
				//-----------DispatchSimplify_5
				|| SimplifyType(pCur)
				|| AttachGlob2(pCur)
				|| SimplifyIndir2(pCur)
				|| SimplifyOffs(pCur)
				|| SimplifyPtr2(pCur)
				|| SimplifyArrays(pCur)//post
				|| SimplifyIndir(pCur)//post
//?				|| SimplifyAssign2(pCur)//post
				)
			{
				__dump(i++, pSelf);
				b = true;
				break;
			}
		}
		if (!b)
			break;
	}

	return i;
}

int EXPRSimpl_t::SecondarySimplifyNew(Out_t *pSelf, int k) const
{
	for (;;)
	{
		bool b(false);
		for (Out_t::Iterator it(pSelf); it; ++it)
		{
			Out_t *pCur(it.top());
			if (SimplifyRawCondition(pCur)
				|| Simplify60(pCur)//zeroextend,signextend
				|| SimplifyPtr2(pCur)
				|| SimplifyIndir2(pCur)
				|| SimplifyOffs(pCur)
				|| Simplify2(pCur)//compound assignments, ++, --
//?				|| Simplify4_1(pCur)//types
				|| Simplify4(pCur)//this
				|| Simplify13(pCur)
				|| Simplify7(pCur)//mul&div => shl&shr
				|| SimplifyEqu(pCur)
				|| Simplify10(pCur)//signs
				|| SimplifyCompar(pCur)//comparisons
				)
			{
				__dump(k++, pSelf);
				b = true;
				break;
			}
		}
		if (!b)
			break;
	}
	return k;
}

int EXPRSimpl_t::PostSimplifyNew(Out_t *pSelf, int k) const
{
	int m(k);
	for (;; m++)
	{
		if (m > k)
			__dump(m, pSelf);
		if (DispatchSimplify_Post(pSelf))
			continue;
		break;
	}
	return m;
}


void EXPRSimpl_t::SimplifyNew(Out_t * pSelf) const
{
	int i(PreSimplifyNew(pSelf, 0));
	i = PrimarySimplifyNew(pSelf, i);
	i = SecondarySimplifyNew(pSelf, i);
	PostSimplifyNew(pSelf, i);
}






















/////////////////////////////////////////////
#if(0)
class SegDumpIter : public StrucDumpIter
{
	const TypeObj_t &mrSegRef;
	const Seg_t &mrSeg;
	SubSegMapCIt	msit;
	enum E_status { E_UNDERFLOW, E_INSIDE, E_INPARENT, E_OVERFLOW };
	FieldTmp_t	m_tmp;//for segments without a trace
public:
	SegDumpIter(BinDumper_t &rDump, const TypeObj_t &rSegRef)
		: StrucDumpIter(rDump, rSegRef),
		mrSegRef(rSegRef),
		mrSeg(*rSegRef.typeSeg()),
		msit(mrSeg.subsegs().end()),
		m_tmp(NULL)
	{
	}
	virtual ~SegDumpIter()
	{
	}
	//SegDumpIter &operator=(const SegDumpIter &){ return *this; }//shut up!
	SegDumpIter &initialise()
	{
		//check if the current daddr gets inside of a segment
		E_status sitStatus;
		checkSubseg(sitStatus);

		if (sitStatus == E_INSIDE || sitStatus == E_INPARENT)
		{
			assert(msit != mrSeg.subsegs().end());
			signalSubseg();
			CTypePtr iSeg2(IVALUE(msit));
			const Seg_t &rSeg2(*iSeg2->typeSeg());
			//ADDR segOffs(mrDump.rawOffs(iSeg2));
			ADDR segOffs(rSeg2.addressP() - mrSegRef.base());
			const FieldMap &m(mrSeg.fields());
			mit = m.SkipEos(m.lower_bound(rSeg2.addressP()));//segOffs));
			ROWID d(__viewOffs(iSeg2) - mrDump.curDA());
			mrDump.advance((unsigned)d, segOffs);
		}
		/*else if (sitStatus == E_INPARENT)
		{
			assert(msit != mrSeg.subsegs().end());
			//signalSubseg();
			CTypePtr iSeg(IVALUE(msit));
			msit++;
			ADDR offs(mrDump.rawOffs(iSeg));
			FieldMap &m(mrSeg.fields());
			mit = ProjectInfo_t::SkipEos(m, m.lower_bound(offs));
			ROWID d(iSeg->typeSeg()->view Offs());
			d -= mrDump.curDA();
			mrDump.advance((unsigned)d, offs);
		}*/
		else
		{
			ROWID rda(mrDump.targetRDA());
			mit = BeginIt(rda);
			if (mit != mrSeg.fields().end())
			{
				CFieldPtr pField(VALUE(mit));
				if (msit != mrSeg.subsegs().end())
				{
					Seg_t *pSubseg(IVALUE(msit)->typeSeg());
					TypePtr iSubsegTrace(pSubseg->traceLink());
					if (iSubsegTrace)
					{
						if (iSubsegTrace == pField->type())
							signalSubseg();
					}
					else
					{
						ADDR a1(pSubseg->addressP());//subseg's attachment point in parent
						ROWID a2(pField->address());//field's address in parent
						if (a1 <= a2)
							signalSubseg();
					}
				}
			}
			else
			{
				if (msit != mrSeg.subsegs().end())
					signalSubseg();
			}
		}
		return *this;
	}
	virtual operator bool() const
	{
		if (StrucDumpIter::operator bool())
			return true;
		return msit != mrSeg.subsegs().end();
	}
	virtual void operator=(const StrucDumpIter &_o)
	{
		const SegDumpIter &o(static_cast<const SegDumpIter &>(_o));
		//?assert(0);//not called in dumpStrucIter()! but somehow works???
		StrucDumpIter::operator=(o);
		msit = o.msit;
		m_tmp = o.m_tmp;
	}
	void operator=(const SegDumpIter &o)
	{
		assert(&mrSeg == &o.mrSeg);
		StrucDumpIter::operator=(o);
		msit = o.msit;
		m_tmp = o.m_tmp;
	}
	SubSegMapCIt nextOrphanSubseg() const
	{
		if (msit != mrSeg.subsegs().end())
		{
			TypePtr iSeg(IVALUE(msit));
			if (!iSeg->typeSeg()->traceLink())
				return msit;
		}
		return mrSeg.subsegs().end();
	}
	bool isTracelessSubsegPending() const 
	{
		return nextOrphanSubseg() != mrSeg.subsegs().end();
	}
	bool signalSubseg()
	{
		m_tmp.setType0(NULL);
		if (msit == mrSeg.subsegs().end())
			return false;
		m_tmp.setType0(IVALUE(msit));
		return true;
	}
	bool isSignaled() const {
		return m_tmp.type() != NULL;
	}
	virtual StrucDumpIter &operator++()
	{
		// We have to advance two iterators simultaneously. One is a regular struc's fields iterator, 
		// the second is of a subseg list, optionally referencing the first. Two are correlated in order, 
		// but the second may contain missing links to the first.
		// This must be specially handled. In general, we have to keep apearance of iterating a single entinty.

		if (m_tmp.type())
		{
			m_tmp.setType0(NULL);//unsignal it
			TypePtr iSegTrace(IVALUE(msit)->typeSeg()->traceLink());
			if (iSegTrace)
			{
				if (iSegTrace == mit->type())
					StrucDumpIter::operator++();//advance mit as well
			}
			//else mit should have been already advanced
			++msit;//advanced here
			if (msit != mrSeg.subsegs().end())
			{
				//now decide if we should signal a new seg
				Seg_t *pSubseg(IVALUE(msit)->typeSeg());
				ADDR a1(pSubseg->addressP());//subseg's address in parent

				iSegTrace = pSubseg->traceLink();
				/*if (iSegTrace)
				{
					if (mit != mrSeg.fields().end())
					{
						Field_t *pField(VALUE(mit));
						if (pField->type() == iSegTrace)
							signalSubseg();
						//else proceed with a regular field
					}
					else//no more regular fields - just 1 or more traceless subsegs
						signalSubseg();
				}
				else*///got a traceless segment
				{
					if (mit != mrSeg.fields().end())
					{
						CFieldPtr pField(VALUE(mit));
						ADDR a2(pField->address());//field's address in parent
						if (iSegTrace)
						{
							if (pField->type() == iSegTrace)
								signalSubseg();
						}
						else
						{
							if (a1 <= a2)//for the subseg, there should be an attachment point in parent
								signalSubseg();//subsegs go before the regular fields with the same attachment position
							/*else if (a1 == a2)
							{
							if (!iSegTrace || )
							signalSubseg();
							}*/
							//else a regular field comes first
						}
					}
					else
						signalSubseg();//no more regular fields
				}
			}
			//else proceed with the next regular field
		}
		else
		{
			if (msit != mrSeg.subsegs().end())
			{
				const Seg_t *pSubseg(IVALUE(msit)->typeSeg());
				CTypePtr iSegTrace(pSubseg->traceLink());
				ADDR a1(pSubseg->addressP());//subseg's address in parent
				if (mit != mrSeg.fields().end())
				{
					CFieldPtr pField(VALUE(mit));
					ADDR a2(pField->address());//field's offset in parent
					if (a1 <= a2)//for the subseg, there should be an attachment point in parent
						signalSubseg();//subsegs go before the regular fields with the same attachment position
					else
					{
						StrucDumpIter::operator++();
						if (mit != mrSeg.fields().end())
						{
							CFieldPtr pField(VALUE(mit));
							ADDR a2(pField->address());
							if (iSegTrace)
							{
								if (pField->type() == iSegTrace)
									signalSubseg();
							}
							else
							{
								if (a1 <= a2)
									signalSubseg();
							}
						}
						else
							signalSubseg();
					}
				}
				else
					signalSubseg();
			}
			else
				StrucDumpIter::operator++();
		}
		return *this;

#if(0)
		//m_tmp.setType0(NULL);
		if (mit != mrSeg.fields().end())
		{
			if (!isTracelessSubsegPending())
			{
				TypePtr iType1(VALUE(mit)->type());
				StrucDumpIter::operator++();
				if (msit != mrSeg.subsegs().end())
				{
					//advance msit if it was equiv to mit
					if (IVALUE(msit)->typeSeg()->traceLink() == iType1)
					{
						++msit;
						if (isTracelessSubsegPending())
						{
							//we got orphan seg - signal it
							signalSubseg();
							return *this;
						}
					}
				}

				if (msit != mrSeg.subsegs().end())
				{
					if (mit == mrSeg.fields().end())
					{
						assert(isTracelessSubsegPending());
						signalSubseg();
					}
					else if (IVALUE(msit)->typeSeg()->traceLink() == VALUE(mit)->type())//are we in parent seg?
						signalSubseg();
				}
				return *this;
			}
			else
			{
				StrucDumpIter::operator++();
				if (mit != mrSeg.fields().end())
				{
					if (KEY(mit) < IVALUE(msit)->typeSeg()->view Offs())
						return *this;
					signalSubseg();
					return *this;
				}
			}
		}
		//else there are still can be orphan segs

		if (!m_tmp.type())
		{
			signalSubseg();
			return *this;
		}
		m_tmp.setType0(NULL);
		
		++msit;//must not be at end
		if (isTracelessSubsegPending())
			signalSubseg();
		else if (msit != mrSeg.subsegs().end())
		{
			//there must be a valid mit!
			if (IVALUE(msit)->typeSeg()->traceLink() == VALUE(mit)->type())
				signalSubseg();
		}

		return *this;
#endif
	}
	virtual CFieldPtr field(ROWID &rda) const 
	{
		if (m_tmp.type())
		{
			TypePtr iSeg(IVALUE(msit));
//			assert(!iSeg->typeSeg()->traceLink());
			rda = __viewOffs(iSeg) - mrDump.mpFrames->mBaseDA;
			return &m_tmp;
		}
		return StrucDumpIter::field(rda);
	}

	virtual CFieldPtr fieldOrEos(ROWID &rda) const
	{
		if (m_tmp.type())
		{
			TypePtr iSeg(IVALUE(msit));
//			assert(!iSeg->typeSeg()->traceLink());
			rda = __viewOffs(iSeg) - mrDump.mpFrames->mBaseDA;
			return &m_tmp;
		}
		return StrucDumpIter::fieldOrEos(rda);
	}

protected:
	E_status hit(ROWID offs, TypePtr iSeg, ROWID da) const//qualify current subseg in regard to current position
	{
		Seg_t &rSeg(*iSeg->typeSeg());
		if (offs > da)
			return E_OVERFLOW;
		ROWID vsz(__viewSize(iSeg));
		if (offs + vsz > da)
			return E_INSIDE;
		ROWID sz(rSeg.size(iSeg));
		ROWID iOver2(vsz - sz);
		ROWID szp(mrDump.SegTraceSize(iSeg));
		if (offs + szp + iOver2 > da)
			return E_INPARENT;
		return E_UNDERFLOW;
	}

	void checkSubseg(E_status &sitStatus)
	{
		msit = mrSeg.subsegs().begin();
		sitStatus = E_UNDERFLOW;
		m_tmp.setType0(NULL);
		ROWID tgtDA(mrDump.targetRDA() + mrDump.curDA());
		for (; msit != mrSeg.subsegs().end(); msit++)
		{
			TypePtr iSeg(IVALUE(msit));
			Seg_t &rSeg(*iSeg->typeSeg());
			ROWID segDA(__viewOffs(iSeg));
			sitStatus = hit(segDA, iSeg, tgtDA);
//			if (iHit == E_INPARENT)//parent hit - next gonna be overflow
//				continue;
			if (sitStatus == E_OVERFLOW)//overflow
				break;
			if (sitStatus == E_INSIDE)//va hit
				break;
			if (sitStatus == E_UNDERFLOW)
			{
CHECKID(iSeg, 0xe89)
STOP
				mrDump.mpFrames->mExtra += (ADDR)mrDump.SegOverSize0(iSeg);
				assert((int)mrDump.mpFrames->mExtra >= 0);
			}
			else if (sitStatus == E_INPARENT)
			{
				ROWID vsz(__viewSize(iSeg));
				ROWID sz(iSeg->size());
				ROWID z(vsz - sz);
				ROWID d(mrDump.SegOverSize(iSeg));
				//mrDump.mpFrames->mExtra = (ADDR)d;
				break;
			}
		}
	}
};
#endif


void BinDumper_t::dumpSeg2(CTypePtr iSelf, const I_DataSourceBase &rData)
{
	DUMPframe_t frame(*this, rData, iSelf);
	InitFrame(frame);
	FrontPtr frontPtr(*this, iSelf);
	
	const Seg_t &rSelf(*iSelf->typeSeg());
	if (rSelf.hasFields() || rSelf.traceLink())
		if (targetRDA() == 0)
			setLevelInfo(adcui::ITEM_HAS_NEXT_CHILD);

	FeedLine(curDA());
	frame.mNextChildRDA = -1;

	SegDumpIter it0(*this, *iSelf);
	it0.initialise();
	SegDumpIter it1(*this, *iSelf);
	it1 = it0;
	if (it1)
		++it1;

	//assert(frame.mRangeDA == max_sz);

	unsigned range(frame.mRangeDA);

#if(0)
	dumpStrucIter(it0, it1, unsigned(range));
	dumpOverSeg(iSelf);
#else
	try
	{
		dumpStrucIter(it0, it1, unsigned(range));
		//if (mBreakStatus != E_RANGE_OVER)
			//return;
	}
	catch (E_BreakStatus e)
	{
		if (e != E_RANGE_OVER)
			throw e;
	}
	mBreakStatus = E_NONE;//reset

	try
	{
		mrDumpTarget.coldata_clear();
		mrDumpTarget.coldata_expand();

		dumpOverSeg(iSelf);
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
		dumpStrucEnd0(frame, it0.field(rda));
	}
#endif
}

void BinDumper_t::dumpOverSeg(CTypePtr iSelf)
{
	Seg_t &rSelf(*iSelf->typeSeg());
	DUMPframe_t &frame(*mpFrames);

	ROWID tsz(SegTraceSize(iSelf));
	ROWID sz(iSelf->size());
	ROWID vsz(__viewSize(iSelf));

	ROWID min_sz(std::min(tsz, sz));//break line
	ROWID max_sz(std::max(tsz, sz));
	ROWID dsz(max_sz - min_sz);
	ROWID esz(vsz - max_sz);

	//dump leftovers in super segment
	int iOver(int(tsz - sz));
	if (iOver > 0 && sz != -1)
	{
		CTypePtr iTrace(rSelf.traceLink());

		mTargetDA = frame.mBaseDA + frame.mCurRDA;
		mCurrentDA = frame.mBaseDA;

		//assert(0);//?broken
		
		//Struc_t &rTrace(*iTrace->typeStruc());
		frame.setCont(iTrace);
		frame.miExtent += iOver;

		StrucDumpIter it2(*this, *iTrace);
		it2.initialise();

		if (mrDumpTarget.isCompactMode() && (curRDA() < min_sz))
			advance(ADDR(min_sz + frame.mExtra), ADDR(min_sz));

		StrucDumpIter it3(*this, *iTrace);
		it3 = it2;
		if (it3)
			++it3;
		dumpStrucIter(it2, it3, unsigned(frame.miExtent - curRDA()));
	}

	int iOver2(int(SegOverSize(iSelf)));
	frame.mExtra += iOver2;
	assert((int)frame.mExtra >= 0);
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
}


ROWID ModuleInfo_t::updateViewGeometry(TypePtr iSelf, ROWID da) const
{
	Seg_t &rSelf(*iSelf->typeSeg());
	ADDR ssz(iSelf->segSize());
	if (ssz == 0)
	{
		CTypePtr pSub(*rSelf.subsegs().begin());
		ssz = pSub->typeSeg()->addressP() - iSelf->base();
	}

	rSelf.setView(da, ssz);//if aggregate, this will give the range to the next segment
	ADDR base0(iSelf->base());
//	ROWID szp(SegTraceSize(iSelf));
//	if (szp > rSelf.mViewSize)
//		rSelf.mViewSize = szp;
	//bool bCheckValidRange(true);
	//rSelf.mValidRange = 0;

	ROWID iOver(0);
	ROWID iOver2(0);

	for (SubSegMapCIt i(rSelf.subsegs().begin()); i != rSelf.subsegs().end(); i++)
	{
		TypePtr iSeg(IVALUE(i));
		const Seg_t &rSeg(*iSeg->typeSeg());
		ADDR base(iSeg->base());
		ROWID da2(rSelf.viewOffs());
		ADDR addrP;
		if (rSeg.traceLink())
		{
			FieldPtr pField(rSeg.traceLink()->parentField());
			addrP = pField->address();
		}
		else
		{
			assert(rSeg.addressP() - base0 == rSeg.rawBlock().m_offs);
			addrP = rSeg.addressP();//rSeg.rawBlock().offs;
		}

		unsigned offsP(addrP - base0);//offset in parent

		/*if (bCheckValidRange)
		{
			if (base0 + offsP == base)
			{}//rSelf.mValidRange = base0 + offsP;
			else
				bCheckValidRange = false;
		}*/

		da2 += offsP + iOver;
		if (da2 > da)
			da = da2;

		ROWID vz2(updateViewGeometry(iSeg, da) - da);
		da += vz2;
		ROWID sz2(iSeg->size());
		ROWID pz2(SegTraceSize(iSeg));
		iOver2 += vz2 - sz2;
		int d(int(sz2 - pz2));
		if (d > 0)
			iOver += d;
		else if (d < 0)
			da += -d;
	}
	rSelf.setView(rSelf.viewOffs(), rSelf.viewSize0() + iOver + iOver2);
	return rSelf.viewOffs() + rSelf.viewSize0();
}

void ModuleInfo_t::UpdateViewGeometry() const
{
	updateViewGeometry(ModulePtr(), 0);
}

