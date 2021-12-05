#include "reg.h"
#include "prefix.h"

#include "shared/defs.h"
#include "shared/link.h"
#include "shared/misc.h"
#include "shared/front.h"
#include "db/mem.h"
#include "db/obj.h"



/*void Reg_t::CopyFrom(Reg_t &r)
{ 
	reg_t::CopyFrom(r);

	assert(m_xdeps.empty());
	if (1)
	{
		m_xdeps = r.m_xdeps; 
		r.m_xdeps = 0;
	}
	else
	if (!r.m_xdeps.empty())
	{
		r.m_xdeps.relocate_to(m_xdeps);
		//XOpLink_t *pXRef = r.m_xdeps.unlink(r.m_xdeps.head(), true);
		//m_xdeps.link_tail(pXRef);

		assert(r.m_xdeps.empty());
	}
}*/

uint32_t REG_t::CheckReg(uint8_t cl, uint8_t &sz, uint8_t &id)
{
	uint32_t mask = Mask();
	if (!mask)
		return 0;

	if (!MASK2REG(mask, cl, sz, id))
		return 0;

	return mask;
}

bool REG_t::MASK2REG(uint32_t mask, uint8_t cl, uint8_t &sz, uint8_t &id)
{
	if (!mask)
		return false;

	sz = 0;
	id = 0;

	uint32_t m;
	if (cl == OPC_CPUREG)
	{
		id = 1;
		sz = 4;
		m = BITMASK(sz);
		bool hi = false;
		bool b = false;
		while (mask)
		{
			if (mask & m)
			{
				if (mask == m)
				{
					if (sz == 1)
					{
						if (id > 4)
							return false;//sp,bp,si,di?
					}
					if (hi)
					{
						if (sz != 1)
							return false;
						id += 4;
					}
					return true;
				}
				if (mask & ~m)
					return false;//???
				sz >>= 1;
				m = BITMASK(sz);
				b = true;//don't update id
				continue;
			}

			mask >>= sz;
			if (!b) 
				id++;
			else
			{
				if (hi)
					return false;
				hi = true;
			}
		}
	}
	else if (cl == OPC_AUXREG)
	{
		id = 1;
		sz = 8;
		m = BITMASK(sz);
		bool hi = false;
		bool b = false;
		while (mask)
		{
			if (mask & m)
			{
				if (mask == m)
				{
					if (hi)
					{
						assert(sz != 8);
						id += 4;
					}
					return true;
				}
				if (mask & ~m)
					return false;//???
				sz >>= 1;
				m = BITMASK(sz);
				b = true;//don't update id
				continue;
			}

			mask >>= sz;
			if (!b) 
				id++;
			else
			{
				if (hi)
					return false;
				hi = true;
			}
		}
	}
	else if ((cl&0xF) == OPC_CPUSW)
		return false;
	else if (cl == OPC_FPUSW)
		return false;
	else if (cl == OPC_FPUREG)
		return false;

	sz = 0;
	while (mask & 1)
	{
		sz++;
		mask >>= 1;
	}

	if (mask)//?
		return false;
	return true;
}

/*
int Reg_t::UNMAKECPUREG(uint8_t &regid, uint8_t &regsz)
{
	int i = 0;
	do {
		if (R & 0x0000000FL)
			break;
		R >>= 4;
	} while (++i < 8);

	if (i > 7)
		return 0;

	regid = i+1;
	if ((R&0xF) == 0xF)
		regsz = OPSZ_DWORD;
	else if ((R&3) == 3)
		regsz = OPSZ_WORD;
	else if ((R&3) != 0)
	{
		if ((R&3) == 2)
			regid += 4;
		regsz = OPSZ_BYTE;
	}
	else 
		return 0;

	return 1;
}*/
/*
OpPtr Reg_t::Get TopOp()
{
	OpPtr pOp = 0;

	XRef_t *pXDeps = m_pXDeps;
	while (1) 
	{
		if (pXDeps)
		{
			if (pXDeps->CheckCount(1) != 0)
				break;
			pOp = pXDeps->pOp;
		}
		else if (!pOp)
			break;
		
		if (!pOp->IsCodeOp())
		{
			if (!pOp->IsCallOutOp())
				break;

			assert(pOp->XIn()->CheckCount(1) == 0);
			pXDeps = pOp->XIn();
		}
		else
		{
			if (pOp->IsPri meOp())
			{
				if ( pOp->isRoot()
					|| (pOp->Action() != ACTN_MOV) )
					break;
				pOp = pOp->GetA rgs();
			}
			else
				pXDeps = pOp->XIn();
		}
	}

	return pOp;
}
*/
/////////////////////////////////////////////////

char const *reg_t::STR_FPUREG_ASM() const
{
	static char buf[6];

	int id = (offset() >> 3) + 1;
	assert(id <= 8);
	strcpy(buf, "ST(?)");
	if (id > 1)
		buf[3] = '0'+(id-1);
	else
		buf[2] = 0;

//?	_STR(buf);
	return buf;
}



