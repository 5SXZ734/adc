#pragma once

#include <type_traits>
#include <sstream>
#include <stdint.h>
//#include "format.elf.h"
#include "DWARF.h"

enum DWARF_AttrIdEnum
{
	ATTR_DWARF_INFO_OFFS = ATTR___MISC_BEGIN,	//A 4-byte unsigned offset into the .debug_info section.
	ATTR_DWARF_ABBREV_OFFS,			//A 4-byte unsigned offset into the .debug_abbrev section.
	ATTR_DWARF_ABBREV_CODE,
	ATTR_DWARF_STR_OFFS,				//A 4-byte unsigned offset into the .debug_str section
	ATTR_DWARF_LINE_OFFS,				//A 4-byte unsigned offset into the .debug_line section
	ATTR___DWARF_END
};

namespace dwarf
{
	struct die2abbrev_t 
	{
		struct elt_t 
		{
			POSITION die;
			POSITION abbrev;
			elt_t(POSITION a, POSITION b)
				: die(a), abbrev(b)
			{
			}
			//bool operator<(const POSITION _die){ return die < _die;	}
		};
		unsigned n;
		die2abbrev_t::elt_t	a[1];
		const elt_t *find(POSITION what) const;
	};

	inline bool operator <(die2abbrev_t::elt_t const& ms, POSITION const i){ return ms.die < i; }
	inline const die2abbrev_t::elt_t *die2abbrev_t::find(POSITION what) const
	{
		const elt_t *e = std::lower_bound(a, a + n, what);
		if (e != &a[n] && e->die == what)
			return e;
		return nullptr;
	}

}//namespace dwarf


void DWARF_CreateStructures(I_SuperModule&);
void DWARF_Declare_ProgramStatement(I_Module&);




