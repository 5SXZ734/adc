#pragma once

struct ins_t;

#include <string>
#include "shared/defs.h"
//#include "shared/misc.h"
#include "shared/dump_util.h"

//char * STR_OPSIZE(int opsz);
//char * STR_DATASZ(int opsz);

class IOutpADDR2Name;


class Outp_t : public DumpUtil_t
{
public:
	bool		do_size;
	//bool		do_bytes;
	const IOutpADDR2Name *do_names;
public:

	Outp_t(MyStrStream& os)
		: DumpUtil_t(os),
		do_size(true),
		do_names(nullptr)
	{
	}

	bool	enable_names(const IOutpADDR2Name *p);
	
	void	out_cpureg(int o, int sz);
	void	out_segreg(int o, int sz);
	void	out_simdreg(int o, int sz);
	void	out_op(const ins_t &, int op_no);

	void	out_prefix(const ins_t&);
	void	out_code(const ins_t&);
	void	out_addr(const ins_t&);
	void	output(const ins_t&);
	bool	out_dist(ADDR from, ADDR to);

	//std::string addr2obj(ADDR a);
};

void x86_Print(const ins_t &, IOutpADDR2Name *);



