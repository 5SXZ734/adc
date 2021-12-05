#include "locus.h"
#include "info_proj.h"

/////////////////////////////////////////////////////
// Frame_t

Frame_t::Frame_t(const Block_t& r, TypePtr c, ADDR a, FieldPtr f)
	: frame_t(r, c, a), _extra(0), pField(f)
{
}

Frame_t::Frame_t(const Block_t& r, TypePtr c, ADDR a, unsigned u, FieldPtr f)
	: frame_t(r, c, a), _extra(u), pField(f)
{
}

ADDR Frame_t::setField2(CFieldPtr p)
{
	assert(p->owner() == m_cont);
	setField(p);
	ADDR leftOver(m_addr + _extra);
	if (leftOver > pField->_key())
		leftOver -= pField->_key();
	else
		leftOver = 0;
	_extra = 0;
	m_addr = pField->_key();
	return leftOver;
}

Struc_t* Frame_t::struc() const
{
	return m_cont->typeStruc();
}

OFF_t Frame_t::rawoff() const
{
	if (empty())
		return OFF_NULL;
	return m_offs + ((m_addr + _extra) - m_cont->base());
}




////////////////////////////////////////////////////// Locus_t

TypePtr Locus_t::struc() const
{
	return empty() ? nullptr : back().cont()->objTypeGlob();
}

TypePtr Locus_t::proc() const
{
	if (empty())
		return nullptr;
	CTypeBasePtr iFunc(back().cont());
	if (!iFunc->typeProc())
		return nullptr;
	return iFunc->objType();
}

TypePtr Locus_t::module() const
{
	/*if (empty())
		return nullptr;
	TypePtr pCont(front().cont()->objTypeGlob());
	if (!pCont->typeModule())
		return nullptr;
	return pCont;*/
	return module2();
}

TypePtr Locus_t::module2() const
{
	if (empty())
		return nullptr;
	TypePtr pCont(front().cont()->objTypeGlob());
	if (!pCont->typeModule())
	{
//		if (!pCont->typeSeg())
//			return nullptr;
		return ProjectInfo_t::ModuleOf(pCont);
	}
	return pCont;
}

TypePtr Probe_t::moduleFromLocus() const
{
//	if (mpFolder)
//		return ProjectInfo_t::ModuleOf(mpFolder);
	return mLocus.module2();
}

/*Folder_t* Probe_t::binaryFolder() const
{
	if (mpFolder)
		return ProjectInfo_t::TopFolder(*mpFolder);
	return nullptr;
}*/

void Probe_t::update()//a field has been added
{
	/*	Frame_t &fr(back());
		if (fr.pField)
		FieldMap &m(fr.pCont->typeStruc()->fields());
		FieldMapIt i(m.find(fr.addr));
		if (i == m.end())
			return;
		VALUE(i)*/
}

ROWID Probe_t::daFromFieldDecl() const
{
	if (!mpFieldDecl)
		return ROWID_INVALID;
	return ProjectInfo_s::VA2DA(mpFieldDecl->owner(), mpFieldDecl->_key());
}

void Locus_t::wrapUp()
{
	assert(!empty());
	TypePtr iSeg(front().cont()->objTypeGlob());
	//assert(iSeg->typeSeg());
	if (!iSeg->typeSeg())
		return;
	TypePtr iSeg0;
	while ((iSeg0 = iSeg->typeSeg()->superLink()) != nullptr)
	{
		const Seg_t& rSeg(*iSeg->typeSeg());
		const Seg_t& rSeg0(*iSeg0->typeSeg());
		FieldPtr pField(nullptr);
		if (rSeg.traceLink())
			pField = rSeg.traceLink()->parentField();
		push_front(Frame_t(iSeg0, rSeg.addressP(), pField));
		iSeg = iSeg0;
	}
}

void Locus_t::add(CTypeBasePtr pCont, ADDR a, FieldPtr p)
{
	if (!p)
	{
		FieldMap& m(pCont->typeStruc()->fields());
		FieldMapIt it(m.find(a));
		if (it != m.end())
			p = VALUE(it);
		add0(pCont, a, p);
	}
	else
		add0(pCont, p->_key(), p);
}

void Locus_t::add0(CTypeBasePtr pCont, ADDR a, FieldPtr p)
{
	push_back(Frame_t((TypePtr)pCont, a, p));
}

FieldPtr Locus_t::asProc() const//exact pick
{
	for (const_reverse_iterator i(rbegin()); i != rend(); i++)
	{
		const Frame_t& f(*i);
		if (!f.field())
			break;
		if (f.field()->type() && f.field()->type()->typeProc())
			return f.field();
		if (f.cont()->base() != f.addrx())
			break;
	}
	return nullptr;
}

TypePtr Locus_t::funcOwner() const
{
	for (const_reverse_iterator i(rbegin()); i != rend(); i++)
	{
		const Frame_t& f(*i);
		assert(f.cont());
		if (f.cont()->typeProc())
			return f.cont();
	}
	return nullptr;
}

ROWID Locus_t::da() const
{
	ROWID a(0);
	for (const_reverse_iterator i(rbegin()); i != rend(); i++)
	{
		const Frame_t& f(*i);
		a += f.addrx() - f.cont()->base();// + extra????
		Seg_t* pSeg(f.cont()->typeSeg());
		if (pSeg)
		{
			ROWID ra(pSeg->viewOffs());
			if (ra == (ROWID)-1)
				ra = 0;
			a += ra;
			return a;
		}
	}
	return ROWID_INVALID;
}

ADDR Locus_t::va() const
{
	ADDR a(0);
	for (const_reverse_iterator i(rbegin()); i != rend(); i++)
	{
		const Frame_t& f(*i);
		a += f.addrx();
		if (f.cont()->typeSeg())
			break;
		a -= f.cont()->base();
	}
	return a;
}

OFF_t Locus_t::ra() const
{
	OFF_t a(0);
	for (const_reverse_iterator i(rbegin()); i != rend(); i++)
	{
		const Frame_t& f(*i);
		a += f.addrx() - f.cont()->base();
		Seg_t* pSeg(f.cont()->typeSeg());
		if (pSeg)
		{
			if (!(a < pSeg->rawBlock().m_size))
				throw (-1);
			a += pSeg->rawBlock().m_offs;
			break;
		}
	}
	return a;
}

TypePtr Locus_t::seg() const
{
	for (const_reverse_iterator i(rbegin()); i != rend(); i++)
	{
		const Frame_t& f(*i);
		if (f.cont()->typeSeg())
			return f.cont();
	}
	assert(!empty());
	return ProjectInfo_t::OwnerSeg(front().cont());
	//return nullptr;
}

TypePtr Locus_t::frontSeg() const
{
	for (const_reverse_iterator i(rbegin()); i != rend(); i++)
	{
		const Frame_t& f(*i);
		if (f.cont()->typeSeg())
		{
			TypePtr iSeg(f.cont());
			do {
				if (iSeg->typeSeg()->frontIndex())
					return iSeg;
				iSeg = iSeg->typeSeg()->superLink();
			} while (iSeg);
		}
	}
	return nullptr;
}

TypePtr Locus_t::frontSeg2() const
{
	const Locus_t& l(*this);
	for (Locus_t::const_reverse_iterator rit(l.rbegin()); rit != l.rend(); rit++)
	{
		const Frame_t& f(*rit);
		Seg_t* pSeg(f.cont()->typeSeg());
		if (pSeg)
		{
			if (pSeg->frontIndex() != 0)
			{
				//??? assert(frontFromIndex(pSeg->frontIndex()));
				return f.cont();
			}
		}
	}
	return nullptr;
}

MyString Locus_t::toString() const
{
	MyString s;
	for (const_iterator i(begin()); i != end(); i++)
	{
		const Frame_t& f(*i);
		if (!s.empty())
			s.append(" '");
		s.append(MyStringf("%X", f.addrx()));
	}
	return s;
}

MyString Locus_t::toStringSeg(const Project_t& proj) const
{
	MyString s, s0;
	ADDR va(0);
	unsigned bit(-1);
	for (const_reverse_iterator i(rbegin()); i != rend(); i++)
	{
		const Frame_t& f(*i);
		if (!f.cont()->typeBitset())
		{
			//ADDR base(0);
			//if (!f.pCont->isShared() && f.pCont->parentField())
				//base = f.pCont->parentField()->_key();
			va += f.addrx() - f.cont()->base();//base;
		}
		else
		{
			assert(bit == -1);
			//get parent of the parent to check if collapsed
			const_reverse_iterator j(++i);
			assert(j != rend());
			j++;
			if (j == rend())//not in-module scope
				bit = f.addrx();
			else
			{
				const Frame_t& f2(*j);
				if (!f2.field()->isCollapsed())
					bit = f.addrx();
			}
		}
		Seg_t* pSeg(f.cont()->typeSeg());
		if (pSeg)
		{
			if (pSeg->subsegs().empty())//virtual address only for terminal segs
				va += f.cont()->base();
			if (!s.empty())
				s.prepend(" '");
			if (s0.empty())
			{
				char buf[32];
				s.prepend(pSeg->printVA(f.cont(), va, buf));
			}
			else
			{
				s.prepend(s0);
				s0.clear();
			}
			s0 = pSeg->title();
			va = 0;
		}
	}
	if (s.empty() && !empty())
	{
		TypePtr iStruc(front().cont());
		s.prepend(MyStringf(" '%X", va));
		if (!iStruc->nameless())
		{
			MyString s2;
			ProjectInfo_t::ChopName(iStruc->name()->c_str(), s2);
			s.prepend(s2);
		}
		else
		{
			ProjectInfo_t PI(proj);
			s.prepend(PI.StrucNameless(iStruc));
		}
	}
	if (bit != -1)
		s.append(MyStringf(".%02X", bit));
	return s;
}

ADDR Locus_t::range(bool bRelaxed) const//returns available range starting from current; ignore eos fields (if relaxed)
{
	ADDR a0(-1);
	ADDR iOffs(0);
	for (const_reverse_iterator i(rbegin()); i != rend(); i++)
	{
		const Frame_t& f(*i);
		//a += f.addrx() - f.pCont->base();// + extra????
		TypePtr iCont(f.cont());
		Struc_t* pStruc(iCont->typeStruc());
		if (pStruc)
		{
			ADDR iBase(pStruc->base(iCont));
			ADDR iOffs0(f.addrx() - iBase);
			iOffs += iOffs0;

			int sz(pStruc->size(iCont));
			if (sz >= 0)
			{
				ADDR upper(iBase + sz);
				Seg_t* pSeg(pStruc->typeSeg());
				if (pSeg)
				{
					ADDR upper2(iBase + (ADDR)ProjectInfo_t::SegTraceSize(iCont));

					ADDR lo(std::min(upper, upper2));
					ADDR hi(std::max(upper, upper2));
					ADDR addr_limit(hi);
					if ((iBase + iOffs) < lo)
						addr_limit = lo;
					ADDR a((addr_limit - iBase) - iOffs);
					if (a < a0)
						a0 = a;
				}
				else
				{
					/*?					ADDR d(ADDR(sz) - iOffs);
										if (d < a0)
											if (pStruc->size0() > 0)
												a0 = d;*/
				}
			}
			FieldMapIt it(pStruc->fields().upper_bound(iOffs + iBase));
			if (it != pStruc->fields().end())
			{
				FieldPtr pField(VALUE(it));
				if (!ProjectInfo_s::IsEosField(pField) || !bRelaxed)
				{
					ADDR iOffs2(pField->_key() - iBase);
					ADDR a(iOffs2 - iOffs);
					if (a < a0)
						a0 = a;
					break;//next field always limits the range
				}
			}
		}
	}
	return a0;
}

bool Locus_t::reduce()
{
	const Frame_t& f0(back());
	assert(!empty());
	assert(!f0.cont()->typeSeg());
	unsigned o(f0.addrx() - f0.cont()->base());
	pop_back();
	if (o > 0)
	{
		Frame_t& f(back());
		f.setAddr(f.addr() + o);
		f.setField(nullptr);
	}
	return true;
}

Frame_t* Locus_t::upframe(unsigned upCount) const
{
	for (const_reverse_iterator i(rbegin()); i != rend(); i++)
	{
		const Frame_t& f(*i);
		if (upCount-- == 0)
			return (Frame_t*)&f;
	}
	return nullptr;
}

bool Locus_t::stripToField(FieldPtr pRef)
{
	assert(pRef);
	for (reverse_iterator i(rbegin()); i != rend(); i++)
	{
		Frame_t& aFrame(*i);
		FieldPtr p0(aFrame.field());//a leading field in current frame (assume a prev field has lower addr)
		/*if (p0 == pRef)
		{
			while (back().field() != pRef)
				pop_back();
			return true;
		}
		else */
		if (p0)//may null for a first frame only
		{
			//should account for u-fields
			TypePtr s(aFrame.cont());
			FieldMap& m(s->typeStruc()->fields());
			for (FieldMapIt it(p0); it != m.end() && KEY(it) == p0->_key(); ++it)
			{
				FieldPtr p(VALUE(it));
				if (p == pRef)
				{
					while (back().field() != p0)
						pop_back();
					if (back().field() != pRef)
						back().setField(pRef);//set a picked u-field
					return true;
				}
			}
		}
	}
	return false;
}

//make sure the back is a seg container
FieldPtr Locus_t::stripToSeg(unsigned& offs)
{
	if (empty())
		return nullptr;
	FieldPtr pField(field0());

	offs = 0;
	while (&front() != &back())//quit if only 1 element is left
	{
		const Frame_t& a(back());
		if (a.cont()->typeSeg())
			break;
		ADDR va(addr());
		ADDR base(a.cont()->base());
		offs += (va - base) + a.extra();
		pop_back();
		pField = field0();
	}
	const Frame_t& a(back());
	offs += a.extra();
	if (offs == 0)
		return pField;
	if (pField)
	{
		int sz(pField->size());
		if (sz <= 0 || !(offs < (unsigned)sz))
			pField = nullptr;
	}
	return pField;
}

bool Locus_t::stripToSeg()
{
	while (!empty())
	{
		const Frame_t& f(back());
		if (f.cont()->typeSeg())
			break;
		unsigned off(f.addrx() - f.cont()->base());
		pop_back();
		if (!empty())
			back().m_addr += off;
	}
	return !empty();
}

bool Locus_t::stripToProc()
{
	/*	while (!empty())
		{
			const Frame_t &f(back());
			if (f.cont()->typeSeg())
				break;
			unsigned off(f.addrx() - f.cont()->base());
			pop_back();
			if (!empty())
				back().m_addr += off;
		}*/
	return !empty();
}

bool Locus_t::stripToNamedField()
{
	while (!empty())
	{
		FieldPtr pField(back().field());
		if (pField && pField->name())
			return true;
		if (back().cont()->typeSeg())
			return false;
		unsigned off(back().m_addr - back().cont()->base());
		pop_back();
		if (!empty())
			back().m_addr += off;
	}
	return false;
}

bool Locus_t::stripToUserField(const char* resolveName)
{
	while (!empty())
	{
		FieldPtr pField(back().field());
		if (pField)
		{
			if (pField->hasUserData())
				return true;
			/*?if (pField->isCloneMaster() && resolveName)
			{
				const ConflictFieldMap& m(back().cont()->typeSeg()->conflictFields());
				for (ClonedFieldMapCIt j(m.lower_bound(pField->_key())); j != m.end(); ++j)
				{
					CFieldPtr pClone(VALUE(j));
					if (pClone->_key() != pField->_key())
						break;
					if (pClone->hasUserData() && pClone->name() && strcmp(resolveName, pClone->name()->c_str()) == 0)
					{
						back().setField(pClone);
						return true;
					}
				}
			}*/
		}
		pop_back();
	}
	return false;
}
