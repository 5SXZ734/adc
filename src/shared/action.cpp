
#include <assert.h>
#include <ctype.h>
#include <map>
#include "qx/MyString.h"
#include "defs.h"
#include "misc.h"
#include "action.h"

class CPriorMap : private std::map<Action_t, unsigned>
{
public:
	CPriorMap()
	{
		struct IDVal_t
		{
			Action_t id;
			int32_t	val;
		};
		static const IDVal_t ActnPrior[] =
		{
			{ ACTN_INCPOST, 0 }, { ACTN_DECPOST, 0 },
			/*{ ACTN_RET, 0 },*/ { ACTN_CALL, 0 }, { ACTN_ARRAY, 0 }, { ACTN_POSTCALL, 0 }, //{ ACTN_INTRINSIC, 0 },
			{ ACTN_PTR, 0 }, { ACTN_DOT, 0 },
			{ ACTN_OVERFLOW, 0 }, { ACTN_SIGN, 0 }, { ACTN_PARITY, 0 },

			{ ACTN_INCPRE, 10 }, { ACTN_DECPRE, 10 },
			{ ACTN_LOGNOT, 10 }, { ACTN_NOT, 10 }, { ACTN_CHS, 10 },
			{ ACTN_OFFS, 10 }, { ACTN_ADDR, 10 }, { ACTN_INDIR, 10 },
			//{ACTN_SIZEOF,10},
			//{ACTN_NEW,10},{ACTN_DELETE,10},
			{ ACTN_TYPE, 10 }, { ACTN_SHIFT, 10 },

			{ ACTN_DOTEX, 20 }, { ACTN_PTREX, 20 },

			{ ACTN_MUL, 30 }, { ACTN_DIV, 30 }, { ACTN_MOD, 30 },

			{ ACTN_ADD, 40 }, { ACTN_SUB, 40 },
			//		{ACTN_CMP,40},

			{ ACTN_SHL, 50 }, { ACTN_SHR, 50 },

			{ ACTN__LESS, 60 }, { ACTN__LESSOREQUAL, 60 }, { ACTN__GREATER, 60 }, { ACTN__GREATEROREQUAL, 60 },

			{ ACTN__EQUAL, 70 }, { ACTN__NOTEQUAL, 70 },

			/*	{ACTN_JMPIF+IFCOND_B,60}, {ACTN_JMPIF+IFCOND_L,60},
			{ACTN_JMPIF+IFCOND_BE,60}, {ACTN_JMPIF+IFCOND_LE,60},
			{ACTN_JMPIF+IFCOND_A,60}, {ACTN_JMPIF+IFCOND_G,60},
			{ACTN_JMPIF+IFCOND_AE,60}, {ACTN_JMPIF+IFCOND_GE,60},
			{ACTN_SETIF+IFCOND_B,60}, {ACTN_SETIF+IFCOND_L,60},
			{ACTN_SETIF+IFCOND_BE,60}, {ACTN_SETIF+IFCOND_LE,60},
			{ACTN_SETIF+IFCOND_A,60}, {ACTN_SETIF+IFCOND_G,60},
			{ACTN_SETIF+IFCOND_AE,60}, {ACTN_SETIF+IFCOND_GE,60},

			{ACTN_JMPIF+IFCOND_E,70}, {ACTN_JMPIF+IFCOND_NE,70},
			{ACTN_SETIF+IFCOND_E,70}, {ACTN_SETIF+IFCOND_NE,70},*/

			{ ACTN_AND, 80 },
			//		{ACTN_TEST,80},

			{ ACTN_XOR, 90 },

			{ ACTN_OR, 100 },

			{ ACTN_LOGAND, 110 },

			{ ACTN_LOGOR, 120 },

			{ ACTN_QUERY, 130 }, { ACTN_ELSE, 130 },

			{ ACTN_MOV, 140 },
			{ ACTN_MOVMUL, 140 }, { ACTN_MOVDIV, 140 }, { ACTN_MOVMOD, 140 },
			{ ACTN_MOVAND, 140 }, { ACTN_MOVXOR, 140 }, { ACTN_MOVOR, 140 },
			{ ACTN_MOVSHL, 140 }, { ACTN_MOVSHR, 140 },
			{ ACTN_MOVADD, 140 }, { ACTN_MOVSUB, 140 },
			{ ACTN_CHECK, 140 },

			{ ACTN_GET, 150 },

			{ ACTN_COMMA, 160 }, { ACTN_COMMA2, 160 }, { ACTN_COMMA3, 160 }, { ACTN_ARG, 160 }, { ACTN_VAR, 160 }, { ACTN_SEMI, 160 }, { ACTN_SEMI2, 160 }, { ACTN_ASSCHK, 160 },

			{ ACTN_FRACT, 170 }, { ACTN_UNITE, 170 },

			{ ACTN_GOTO, 180 }, { ACTN_GOTOIF, 180 }, { ACTN_RET, 180 },

			{ ACTN_NULL, -1 }
		};

		for (const IDVal_t *p(ActnPrior); p->id != ACTN_NULL; p++)
			insert(std::make_pair((Action_t)p->id, p->val));
	}
	unsigned get(Action_t a) const
	{
		const_iterator i(find(a));
		if (i != end())
			return i->second;
		assert(0);
		return (unsigned)-1;
	}
};

int ComparePrior(Action_t a1, Action_t a2, int32_t &result)
{
	static const CPriorMap m;

	int32_t prior1(m.get(a1));
	int32_t prior2(m.get(a2));

	if (prior1 < prior2)
		result = 1;
	else if (prior1 > prior2)
		result = -1;
	else
		result = 0;

	return 1;
}

class CActionMap : private std::map<Action_t, const char *>
{
public:
	CActionMap()
	{
		struct ActnStr_t
		{
			Action_t	action;
			const char		*str;
			//const char		*strex;

			/*const ActnStr_t *get(Action_t a) const
			{
			for (const ActnStr_t *p(this); p->action != ACTN_NULL; p++)
			{
			if (p->action == a)
			return p;
			}

			return nullptr;
			}*/
		};
		
		static const ActnStr_t ActnStr[] = {
			{ ACTN_GOTO, "GOTO" },
			{ ACTN_GOTOIF, "GOTOIF" },
			{ ACTN_RET, "RET" },
			{ ACTN_NOT, "NOT\t~" },
			{ ACTN_CHS, "CHS\t-" },
			{ ACTN_VAR, "VAR\tVAR " },
			{ ACTN_ARG, "ARG\tARG " },

			{ ACTN_OVERFLOW, "OVERFLOW" },
			{ ACTN_NOVERFLOW, "NOVERFLOW" },
			{ ACTN_BELOW, "BELOW" },
			{ ACTN_NBELOW, "NBELOW" },
			{ ACTN_ZERO, "ZERO" },
			{ ACTN_NZERO, "NZERO" },
			{ ACTN_NABOVE, "NABOVE" },
			{ ACTN_ABOVE, "ABOVE" },
			{ ACTN_SIGN, "SIGN" },
			{ ACTN_NSIGN, "SIGN" },
			{ ACTN_PARITY, "PARITY" },
			{ ACTN_NPARITY, "NPARITY" },
			{ ACTN_LESS, "LESS" },
			{ ACTN_NLESS, "NLESS" },
			{ ACTN_NGREATER, "NGREATER" },
			{ ACTN_GREATER, "GREATER" },

			{ ACTN_MOV, "MOV\t = " },
			{ ACTN_CHECK, "CHECK\t ~~ " },//ACTN_TEST
			{ ACTN_ADD, "ADD\t + " },
			{ ACTN_SUB, "SUB\t - " },
			{ ACTN_MUL, "MUL\t * " },
			{ ACTN_DIV, "DIV\t / " },
			{ ACTN_MOD, "MOD\t % " },
			{ ACTN_AND, "AND\t & " },
			{ ACTN_OR, "OR\t | " },
			{ ACTN_XOR, "XOR\t ^ " },
			{ ACTN_SHL, "SHL\t << " },
			{ ACTN_SHR, "SHR\t >> " },

			{ ACTN_MOVMUL, "MOVMUL\t *= " },
			{ ACTN_MOVDIV, "MOVDIV\t /= " },
			{ ACTN_MOVMOD, "MOVMOD\t %= " },
			{ ACTN_MOVAND, "MOVAND\t &= " },
			{ ACTN_MOVXOR, "MOVXOR\t ^= " },
			{ ACTN_MOVOR, "MOVOR\t |= " },
			{ ACTN_MOVSHL, "MOVSHL\t <<= " },
			{ ACTN_MOVSHR, "MOVSHR\t >>= " },
			{ ACTN_MOVADD, "MOVADD\t += " },
			{ ACTN_MOVSUB, "MOVSUB\t -= " },
			{ ACTN_GET, "GET\t := " },

			{ ACTN_INCPOST, "INCPOST\t++" },
			{ ACTN_DECPOST, "DECPOST\t--" },
			{ ACTN_INCPRE, "INCPOST\t++" },
			{ ACTN_DECPRE, "DECPOST\t--" },

			{ ACTN_INDIR, "INDIR\t*" },//(*)
			{ ACTN_OFFS, "OFFS\t&" },
			{ ACTN_ADDR, "ADDR\t@" },
			{ ACTN_PTR, "PTR\t->" },
			{ ACTN_PTREX, "PTREX\t->*" },
			{ ACTN_TYPE, "TYPE\t" },//(type)
			{ ACTN_SHIFT, "SHIFT", },//(offset)
			{ ACTN_DOT, "POINT\t." },
			{ ACTN_DOTEX, "POINTEX\t.*" },
			{ ACTN_ARRAY, "ARRAY\t[]" },//[]
			{ ACTN_SEMI, "SEMI\t; " },
			{ ACTN_SEMI2, "SEMI2\t; " },
			{ ACTN_COMMA, "COMMA\t, " },
			{ ACTN_COMMA2, "COMMA2\t(,) " },
			{ ACTN_COMMA3, "COMMA3\t, " },
			{ ACTN_ASSCHK, "ASSCHK\t`" },

			{ ACTN_LOGAND, "LOGAN\t && " },
			{ ACTN_LOGOR, "LOGOR\t || " },
			{ ACTN_LOGNOT, "LOGNOT\t!" },
			{ ACTN_QUERY, "QUERY\t ? " },
			{ ACTN_ELSE, "ELSE\t : " },
			{ ACTN_FRACT, "FRACT\t (:) " },

			//{ ACTN_INTRINSIC, "INTRINSIC" },//()
			{ ACTN_CALL, "CALL" },//()
			{ ACTN_SQRT, "sqrt" },
			//{ ACTN_MOVS, "movs" },
			{ ACTN_SIN, "sin" },
			{ ACTN_COS, "cos" },
			{ ACTN_ABS, "fabs" },
			{ ACTN_2XM1, "f2xm1" },
			{ ACTN_SCALE, "fscale" },
			{ ACTN_TAN, "tan" },
			{ ACTN_ATAN2, "atan2" },
			{ ACTN_RNDINT, "round" },
			{ ACTN_SIGNEXT, "SIGNEXTEND" },
			{ ACTN_ZEROEXT, "ZEROEXTEND" },
			{ ACTN_ROL, "rol" },
			{ ACTN_ROR, "ror" },
			{ ACTN_IN, "in" },
			{ ACTN_OUT, "out" },
			{ ACTN_BSWAP, "bswap" },
			{ ACTN_SCASBWNE, "scasbwne" },
			{ ACTN_UNITE, ":" },
			{ ACTN_POSTCALL, "POSTCALL" },

			{ ACTN_ASSIGNCS, "ASSIGNCS" },
			{ ACTN_ASSIGNDS, "ASSIGNDS" },

			{ ACTN_DASSIGN, "DASSIGN" },
			{ ACTN_DCOMMAND, "DCOMMAND" },
			{ ACTN_DCLOSE, "DCLOSE" },
			{ ACTN_DERROR, "DERROR" },
			{ ACTN_DONERROR, "DONERROR" },
			{ ACTN_DRESULT, "DRESULT" },
			{ ACTN_DSEEK, "DSEEK" },
			{ ACTN_DWRITE, "DWRITE" },
			{ ACTN_MADDS, "MADDS" },
			{ ACTN_MBOUNDS, "MBOUNDS" },
			//	{ACTN_MDADA1,		"MDADA1"					},
			{ ACTN_MDADA4, "MDADA4" },
			{ ACTN_MITOA, "MITOA" },
			{ ACTN_MLENGTH, "MLENGTH" },
			{ ACTN_MODULE, "MODULE" },
			{ ACTN_MON, "MON" },
			{ ACTN_MSYNC, "MSYNC" },
			//	{ACTN_MTCWX,		"MTCWX"						},
			{ ACTN_MZVARWRITE, "MZVARWRITE" },
			{ ACTN_MZVARUSE, "MZVARUSE" },

			{ ACTN_LOHALF, "lo" },
			{ ACTN_HIHALF, "hi" },

			{ ACTN__LESS, "#LESS\t < " },
			{ ACTN__GREATEROREQUAL, "#GREATEOREQUAL\t >= " },
			{ ACTN__EQUAL, "#EQUAL\t == " },
			{ ACTN__NOTEQUAL, "#NOTEQUAL\t != " },
			{ ACTN__LESSOREQUAL, "#LESSOREQUAL\t <= " },
			{ ACTN__GREATER, "#GREATER\t > " },

			{ ACTN_SWITCH, "SWITCH" },
			{ ACTN_NULL, "???" }
		};
		for (const ActnStr_t *p(ActnStr); p->action != ACTN_NULL; p++)
			insert(std::make_pair((Action_t)p->action, p->str));
	}
	const char *get(Action_t a) const//display name
	{
		const_iterator i(find(a));
		if (i != end())
		{
			const char *chTab(strchr(i->second, '\t'));
			if (chTab)
				return ++chTab;
			return i->second;
		}
		assert(0);
		return "";
	}
	std::string getx(Action_t a) const//internal name
	{
		const_iterator i(find(a));
		if (i != end())
		{
			const char *chTab(strchr(i->second, '\t'));
			if (chTab)
				return std::string(i->second, chTab - i->second);
			return i->second;
		}
		assert(0);
		return "";
	}
};

static CActionMap gActionMap;

std::string ACTN2STR(Action_t Action)//for logging expression tree
{
	switch (Action)
	{
	case ACTN_INDIR: return "(*)";
	case ACTN_CALL:
	//case ACTN_INTRINSIC:
		return "()";
	case ACTN_ARRAY: return "[]";
	case ACTN_TYPE: return "(type)";
	}

	MyString s(gActionMap.getx(Action));
	return s.simplifyWhiteSpace();
}

const char *ACTN2STR2(Action_t Action)
{
	return gActionMap.get(Action);
}
