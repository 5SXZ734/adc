#pragma once

#include "db/types.h"
#include "shared/link.h"

class FuncDef_t;

enum {
	ARG_NULL,
	ARG_RETURNED	= 0x0001,
//	ARG_THISPTR		= 0x0002,
};

class Arg0_t
{
	OPC_t	m_opc;
	int		m_offs;
public:
	Arg0_t()
		: m_opc(OPC_NULL),
		m_offs(0)
	{
	}
	Arg0_t(OPC_t ssid, int off)
		: m_opc(ssid),
		m_offs(off)
	{
	}
	OPC_t opc() const { return m_opc; }
	SSID_t ssid() const { return SSID_t(m_opc & SSID_MASK); }
	int offs() const { return m_offs; }
};

class Arg1_t : public Arg0_t
{
	OpType_t	m_optyp;
public:
	Arg1_t()
		: m_optyp(OPTYP_NULL)
	{
	}
	Arg1_t(OPC_t opc, int off, OpType_t typ)
		: Arg0_t(opc, off),
		m_optyp(typ)
	{
	}
	void setOptyp(OpType_t t){ m_optyp = t; }
	OpType_t optyp() const { return m_optyp; }
	unsigned opsz() const { return (m_optyp & OPSZ_MASK); }
};

class Arg1List_t : public std::list<Arg1_t>
{
public:
	Arg1List_t(){}
};

class Arg2_t : public Arg0_t
{
	unsigned	m_siz;
public:
	Arg2_t()
		: m_siz(0)
	{
	}
	Arg2_t(OPC_t opc, int off, unsigned siz)
		: Arg0_t(opc, off),
		m_siz(siz)
	{
	}
	unsigned size() const { return m_siz; }
};

