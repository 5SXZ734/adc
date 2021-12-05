#include "types.h"
#include "dump_strucvar.h"

#include "dump_bin.h"
#include "type_strucvar.h"


////////////////////////////////////////////////
// StrucVarDumper_t

StrucVarDumper_t::StrucVarDumper_t(BinDumper_t &rDumper, const TypeObj_t &r, unsigned scopeRange, std::list<DUMPframe_t *>& frames)
	: StrucvarTracer0_t(ModuleInfo_t(rDumper, *rDumper.module()), (TypeObj_t &)r, scopeRange),
	mrDumper(rDumper),
	m_frames(frames)
	//mbIncomplete(0)
{
	//?		rDumper.InitFrame(m_frame);
}

StrucVarDumper_t::~StrucVarDumper_t()
{
	//assert(m_frames.empty());
	//clearFrames();
	//m_scope.pop_back();
	//?assert(m_scope.empty());//misaligned data may throw
}

void StrucVarDumper_t::clearFrames()
{
	while (!m_frames.empty())
	{
		DUMPframe_t *pf(m_frames.back());
		m_frames.pop_back();
		delete pf;
	}

}

TypesMgr_t &StrucVarDumper_t::typesMgrNS(TypePtr p) const
{
	return mrDumper.typesMgrNS();
}

MemoryMgr_t &StrucVarDumper_t::memMgrNS() const
{
	return mrDumper.memMgrNS();
}

void StrucVarDumper_t::dump(I_Module& iface)
{
	I_DynamicType *pIDevice(mrSelf.typeStrucvar()->device());
	if (!pIDevice)
		return;
	if (iface.NewScope(pIDevice->name()))
	{
		//SAFE_SCOPE_HERE(*this);
		//try
		{
			//mbIncomplete = true;
			//top().mIncomplete++;
			pIDevice->createz(iface, -1);//!
			//top().mIncomplete--;//will remain marked incomplete if an exception thrown
			//mbIncomplete = false;
		}
		//catch(...)
		{
			STOP//no exception past this point (some may be thrown at data access)
		}
		Leave();
	}
}

/*void StrucVarDumper_t::markIncomplete()
{
//	for (std::list<DUMPframe_t*>::iterator i(m_frames.begin()); i != m_frames.end(); i++)
	//	(*i)->mIncomplete++;

		//bool bComplete(pFrame->mIncomplete == 0);
	//if (bComplete)
		//mrDumper.mrDumpTarget.coldata_trimDanglingTail(mrDumper.GetLevel());
}*/

void StrucVarDumper_t::OnField(FieldPtr pField, unsigned range, TypePtr iType)
{
	TypePtr iScope(scopeContainer());
	if (iType && iType == iScope)
	{
		//m_frames.pop_back();
		//return new Scope(nullptr, eScope, name, eAttr, 0, AttrScopeEnum::null) != HFIELD();
	}

	mrDumper.dumpField2(pField, iType);
	mrDumper.dumpFieldType(pField, (unsigned)range, iType);
	top().mNextChildRDA = -1;//maximum
}

void StrucVarDumper_t::OnAlign(ADDR newOffs)
{
	const DUMPframe_t &frame(top());
	ADDR oldOffs((ADDR)mrDumper.curRDA());
	unsigned d(newOffs - oldOffs);
	unsigned uRange(scopeRange());
	if (d > uRange)
		throw FieldOverlapException(uRange);
	if (frame.cont()->typeBitset())
		mrDumper.dumpBitGap(d, false, mrDumper.mrDumpTarget.isCompactMode());
	else
		mrDumper.dumpUnk0(d, 0);
}

OFF_t StrucVarDumper_t::cpr() const
{
	const DUMPframe_t &f(top());
	//assert(!f.rawBlock().empty());
	return mrDumper.RawOffs(f);
}

bool StrucVarDumper_t::isLarge() const {
	return top().mbLarge != 0;
}

size_t StrucVarDumper_t::dataAt(OFF_t off, OFF_t siz, PDATA pDest) const
{
	/*const Block_t &r(mrModule.rawBlock());
	if (r.empty() || size == 0)
		return 0;
	ADDR lo = ADDR(rawOffs);
	assert(rawOffs < r.size);
	ADDR hi = ADDR(lo + size);
	if (hi > r.size)
		size -= (hi - r.size);
	memcpy((void*)pDest, r.ptr + lo, size);
	return size;*/
	assert(GetDataSource());
	return GetDataSource()->pvt().dataAt(off, siz, pDest);
}

OFF_t StrucVarDumper_t::size() const
{
	const Block_t &r(mrModule.rawBlock());
	return r.m_size;
}

void StrucVarDumper_t::leave1()
{
	DUMPframe_t *pFrame(m_frames.back());
	if (pFrame->mIncomplete == 0)
	{
		unsigned u(pFrame->mBitsRDA);//pBitset->upperBound());
		if (u > 0)
		{
			assert(pFrame->cont()->typeBitset());
			//assert((mrDumper.Range(*pFrame) % CHAR_BIT) == 0);
			unsigned v(scopeRange());//upper_power_of_two(u));
			if (v > 0)
			{
				bool bCompact(mrDumper.mrDumpTarget.isCompactMode());
				if (!bCompact)
#if(SQUEEZE_STRUCVAR_GAPS)
					if (mrDumper.isDumpingStrucvar())
#endif
						bCompact = true;
				mrDumper.dumpBitGap(v, false, bCompact);
			}
		}
	}
	m_frames.pop_back();
	delete pFrame;
}

void StrucVarDumper_t::discardLastRow()
{
	mrDumper.mrDumpTarget.discardLastRow();//current (empty)
	mrDumper.mrDumpTarget.discardLastRow();//previous
	mrDumper.mrDumpTarget.coldata_expand();
}

void StrucVarDumper_t::discardLastFrame()
{
	discardLastRow();
	DUMPframe_t *pFrame(m_frames.back());
	m_frames.pop_back();
	delete pFrame;
}

void StrucVarDumper_t::Leave()
{
	DUMPframe_t *pFrame(m_frames.back());
	if (pFrame->cont()->typeBitset())
	{
		leave1();
		Leave();
		return;
	}

	//bool bComplete(pFrame->mIncomplete == 0);
	//if (bComplete)
		mrDumper.mrDumpTarget.coldata_trimDanglingTail(mrDumper.GetLevel());
	
	m_frames.pop_back();
	delete pFrame;

	//if (m_frames.empty())
		//mbIncomplete = bComplete;
}

TypePtr StrucVarDumper_t::newScope(FieldPtr pField0, SCOPE_enum e, const char*, unsigned range0, AttrScopeEnum)
{
	assert(pField0->isTemp());
	unsigned range(mRange);
	Bitset_t *pBitset(nullptr);
	if (m_frames.empty())
	{
		//TypePtr iCont(m_frames.empty() ? &mrSelf : top().cont());
		m_frames.push_back(new DUMPframe_t(mrDumper, mrDumper.data(), &mrSelf));
	}
	else
	{
		TypePtr iOwner(top().cont());
		if (iOwner->typeBitset())
		{
			leave1();
			iOwner = top().cont();
			assert(!iOwner->typeBitset());
		}

		discardLastRow();
		mrDumper.advance(-1, -1);

		//reuse last field's settings
		//mLastField.setOwnerComplex(iOwner);
		assert(mLastField.owner() == iOwner);
		//mLastField.setKey((FieldKeyType)mrDumper.curRDA());
		//assert(mLastField._key() == (FieldKeyType)mrDumper.curRDA());

		FieldPtr pField(nullptr);
		Strucvar_t *pStrucvar(iOwner->typeStrucvar());
		if (pStrucvar)
		{
			if (SCOPEX_enum(e) == SCOPEX_BITSET)
			{
				pField = pStrucvar->bitfield(iOwner);
			}
			else
				pField = &mLastField;
		}
		else
			ASSERT0;
		assert(pField);
		TypePtr iType(pField->type());
		if (iType)
			pBitset = iType->typeBitset();

		if (!pBitset)
		{
			mrDumper.drawChildHere(mrDumper.lineColor(mrDumper.mpFrames));
			mrDumper.drawFieldType(pField, iType);
			mrDumper.drawFieldName(pField, iType);
		}

		range = (unsigned)mrDumper.Range(top());
		//			if (mrDumper.curRDA() < range)
		//			range -= (unsigned)mrDumper.curRDA();
		if (range == 0)//do not enter
			throw E_FIELD_OVERLAP;

		if (!iType)
		{
			assert(iOwner->typeStrucvar());
			iType = iOwner;
		}
		m_frames.push_back(new DUMPframe_t(mrDumper, mrDumper.data(), iType));
	}

	if (range0 != 0)
		range = range0;

	mrDumper.InitFrame(top(), range);

	mrDumper.setLevelInfo(adcui::ITEM_HAS_NEXT_CHILD);
	if (!pBitset)
		mrDumper.FeedLine(mrDumper.curDA());
	top().mNextChildRDA = -1;//maximum
	//mbIncomplete++;
	return top().cont();
}

TypePtr StrucVarDumper_t::newScope(const char *, SCOPE_enum e, const char *pFieldName, unsigned range0, AttrScopeEnum)
{
	unsigned range(mRange);
	Bitset_t *pBitset(nullptr);
	if (m_frames.empty())
	{
		//TypePtr iCont(m_frames.empty() ? &mrSelf : top().cont());
		m_frames.push_back(new DUMPframe_t(mrDumper, mrDumper.data(), &mrSelf));
	}
	else
	{
		TypePtr iOwner(top().cont());
		if (iOwner->typeBitset())
		{
			leave1();
			iOwner = top().cont();
			assert(!iOwner->typeBitset());
		}

//		assert(0);
		mLastField.setName(pFieldName);
		mLastField.setOwnerComplex(iOwner);
//?		mLastField.setKey((FieldKeyType)mrDumper.curRDA());
		mLastField.setType0(nullptr);
		//mLastField.set AttributeFromId(attr);

		FieldPtr pField(nullptr);
		Strucvar_t *pStrucvar(iOwner->typeStrucvar());
		if (pStrucvar)
		{
			if (SCOPEX_enum(e) == SCOPEX_BITSET)
			{
				pField = pStrucvar->bitfield(iOwner);
				/*if (pStrucvar->hasFields())
					pField = VALUE(pStrucvar->fields().begin());
				if (!pField || pField->_key() > 0)
				{
					Bitset_t *pBitset(new Bitset_t());
					TypePtr iType(memMgrGlobNS().NewTypeRef(pBitset));
					ModuleInfo_t MI2(*this, memMgrGlobNS());
					pField = insertStrucvarField(0, iOwner, nullptr, iType);//at position '0'
				}*/
			}
			else
				pField = &mLastField;
				//pField = pStrucvar->findFieldByName(pFieldName);
		}
		else
			pField = findFieldByName(iOwner, pFieldName);
		assert(pField);
		TypePtr iType(pField->type());
		if (iType)
			pBitset = iType->typeBitset();

		if (!pBitset)
		{
			mrDumper.drawChildHere(mrDumper.lineColor(mrDumper.mpFrames));
			mrDumper.drawFieldType(pField, iType);
			mrDumper.drawFieldName(pField, iType);
		}

		range = (unsigned)mrDumper.Range(top());
		//			if (mrDumper.curRDA() < range)
		//			range -= (unsigned)mrDumper.curRDA();
		if (range == 0)//do not enter
			throw E_FIELD_OVERLAP;

		if (!iType)
		{
			assert(iOwner->typeStrucvar());
			iType = iOwner;
		}
		m_frames.push_back(new DUMPframe_t(mrDumper, mrDumper.data(), iType));
	}

	if (range0 != 0)
		range = range0;

	mrDumper.InitFrame(top(), range);

	mrDumper.setLevelInfo(adcui::ITEM_HAS_NEXT_CHILD);
	if (!pBitset)
		mrDumper.FeedLine(/*mrDumper.curDA(), */mrDumper.curDA());
	top().mNextChildRDA = -1;//maximum
	//mbIncomplete++;
	return top().cont();
}

TypePtr StrucVarDumper_t::scopeContainer() const
{
	return top().m_cont->objTypeGlob();
}

TypePtr StrucVarDumper_t::scopeSeg() const
{
	const DUMPframe_t& top(*mrDumper.mpTopFrame);
	return top.miSeg;
}

/*HTYPE StrucVarDumper_t::NewS cope(const char *sTypeName, SCOPE_enum e, const char *pFieldName, AttrIdEnum attr)
{
	return new Scope(sTypeName, e, pFieldName, attr, 0);
}*/

ADDR StrucVarDumper_t::scopeOffs() const
{
	const DUMPframe_t &aTop(top());
	if (aTop.cont()->typeBitset())
		return (ADDR)mrDumper.curRDA() * CHAR_BIT + aTop.mBitsRDA;
	return (ADDR)mrDumper.curRDA();
}

unsigned StrucVarDumper_t::scopeRange() const
{
	const DUMPframe_t &aTop(top());
	if (aTop.cont()->typeBitset())
		return (unsigned)mrDumper.Range(aTop) * CHAR_BIT - aTop.mBitsRDA;
	return (unsigned)mrDumper.Range(aTop);
}

POSITION StrucVarDumper_t::cp() const
{
	POSITION pos(0);
	const DUMPframe_t& top(*mrDumper.mpTopFrame);
	ADDR off(ADDR(mrDumper.curDA() - top.mBaseDA));//offset in current section
	off += top.offset();//offset in segment
	return (POSITION)top.cont()->base() + off;
	return pos;
}


void StrucVarDumper_t::setScopeRange(unsigned range)
{
	DUMPframe_t &aTop(top());
	mrDumper.SetRange(aTop, range);
	aTop.miExtent = unsigned(mrDumper.curRDA() + mrDumper.Range(aTop));
	//raw datas?
}





StrucVarDumperEx_t::StrucVarDumperEx_t(BinDumper_t& dumper, const TypeObj_t& self, unsigned scopeRange)
	: StrucVarDumper_t(dumper, self, scopeRange, m_frames),
	mStartDA(start()),
	mStartVA(dumper.curVA())
{
	assert(!mrDumper.mpStrucvarDumper);
	mrDumper.mpStrucvarDumper = this;
}

StrucVarDumperEx_t::~StrucVarDumperEx_t()
{
	clearFrames();//should be cleared if no exception
	assert(mrDumper.mpStrucvarDumper == this);
	mrDumper.mpStrucvarDumper = nullptr;
}

ROWID StrucVarDumperEx_t::start()
{
	mrDumper.EnterNoBreakSection();
	EnterNoFlushSection();
	//if (mrDumper.muDumpingStrucvar++ == 0)
	return mrDumper.curDA();
}


void StrucVarDumperEx_t::end()
{
	/*if (!m_frames.empty())
	{
		clearFrames()
		//mbIncomplete = true;
	}*/

	//if (--mrDumper.muDumpingStrucvar == 0)
	mrDumper.mrDumpTarget.setCachedObjSize(mStartDA, mrDumper.curDA() - mStartDA);

	LeaveNoFlushSection();
	mrDumper.LeaveNoBreakSection();
}

void StrucVarDumperEx_t::EnterNoFlushSection()
{
	mrDumper.mbFreezeFlush++;
}

void StrucVarDumperEx_t::LeaveNoFlushSection()
{
	--mrDumper.mbFreezeFlush;
	assert(mrDumper.mbFreezeFlush == 0);
	{
		bool bProblem(!m_frames.empty());
		DumpTarget_t& t(mrDumper.mrDumpTarget);
		if (!mrDumper.mpFrames->mbCollapsed)
		{
			t.coldata_pruneEmptyTail();
			while (!t.coldata_empty())
				mrDumper.flushRow(bProblem);
			//t.clearColDataVec();
			t.coldata_expand();
		}
		else
		{
			//while (t.coldata_pruneEmptyTail());
			while (t.coldata_size() > 1)
				t.discardLastRow();
			if (mrDumper.mpFrames->mIncomplete)
				mrDumper.mpFrames->mpUp->mbProblemLine = 1;
		}
	}
}

