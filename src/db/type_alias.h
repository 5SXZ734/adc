#pragma once

#include "type_obj.h"

#define TYPEID_CODE	"@CODE"
#define TYPEID_PROC	"@PROC"//length<32
#define TYPEID_IMP	"@IMP"
#define TYPEID_EXP	"@EXP"
//#define TYPEID_IMPTR64	"@IMPTR64"
#define TYPEID_THISPTR32	"@THISPTR32"
#define TYPEID_THISPTR64	"@THISPTR64"
#define TYPEID_VPTR32	"@VPTR32"
#define TYPEID_VPTR64	"@VPTR64"
//	simp:	$<id>
//	comp:	!<id>
//	ptr:	<base>*<size>
//	imp:	<base>@IMP
//	vptr:	@VPTR32|@VPTR64
//	ref:	<base>&<size>
//	rref:	<base>&&<size>
//	enum:	<base><simp>#
//	arr:	<base>[total]
//	arr_i:	<base>[total<simp><comp>]	//?
//	const:	<base>@
//	thunk:	<base>?
//	tdef:	^<id>
//	pair:	<left>,<right>
//	func:	<base>(<args>)|<flags>

#define	ARRAY_BYTES_MASK	0x80000000

template <typename T>
class TMakeAlias : public std::string
{
public:
	TMakeAlias() {}
	TMakeAlias(T p)
	{
		p->aka(*this);
	}

	void forOpTyp(OpType_t typeID)
	{
		char s[4] = { "$xx" };
		s[1] = digit2char(typeID >> 4);
		s[2] = digit2char(typeID);
		append(s);
	}

	void forTypeVoid()
	{
		forOpTyp(OPTYP_NULL);
	}

	void forTypePtr(T pBaseType, unsigned ptrSize)
	{
		if (pBaseType)
		{
			//assert(mrTypesMgr.contains(pBaseType));
			pBaseType->aka(*this);
		}
		else
			forTypeVoid();
		append("*");
		appendHex(ptrSize);
	}

	void forTypeImp(T pBaseType)
	{
		pBaseType->aka(*this);
		append(TYPEID_IMP);
	}

	void forTypeExp(T pBaseType)
	{
		pBaseType->aka(*this);
		append(TYPEID_EXP);
	}

	void forTypeVPtr(OpType_t t)
	{
		if (t == OPSZ_DWORD)
			append(TYPEID_VPTR32);
		else if (t == OPSZ_QWORD)
			append(TYPEID_VPTR64);
		else
			ASSERT0;
	}

	void forTypeThisPtr(T pBaseType, unsigned ptrSize)
	{
		forTypePtr(pBaseType, ptrSize);
	}

	void forTypeRef(T pBaseType, unsigned ptrSize)
	{
		assert(pBaseType);
		//assert(mrTypesMgr.contains(pBaseType));
		pBaseType->aka(*this);
		append("&");
		appendHex(ptrSize);
	}

	void forTypeRRef(T pBaseType, unsigned ptrSize)
	{
		assert(pBaseType);
		//assert(mrTypesMgr.contains(pBaseType));
		pBaseType->aka(*this);
		append("&&");
		appendHex(ptrSize);
	}

	void forTypeEnum(OpType_t typ, T iType)
	{
		iType->aka(*this);
		forOpTyp(typ);
		append("#");
	}

	void forTypeArray(T baseType, unsigned total)
	{
		baseType->aka(*this);
		append("[");
		if (total & ARRAY_BYTES_MASK)
		{
			append("\'");
			appendHex(total & ~ARRAY_BYTES_MASK);
		}
		else
			appendHex(total);
		append("]");
	}

	void forTypeArrayIndex(T pArrayRef, T pIndexRef)
	{
		const Array_t& rArray(*pArrayRef->typeArray());
		rArray.baseType()->aka(*this);

		append("[");
		int arrayNum((int)rArray.total());
		appendHex(arrayNum);
		int comp_id(pIndexRef->typeEnum()->enumRef()->typeComplex()->ID());
		forTypeComplex(comp_id);
		forOpTyp((OpType_t)pIndexRef->typeSimple()->optype());
		append("]");
	}

	void forTypeSimple(OpType_t typ)
	{
		forOpTyp(typ);
	}

	void forTypeComplex(int id)
	{
		//assert(buf[0] == 0);//not in array's index
		append("!");
		appendHex(id, true);
	}

	/*void MakeAlias::forTypeStrucvar(int id)
	{
	sprintf(buf, "!%08X", id);
	}*/

	void forTypedef(T pBaseType, int id)
	{
		append("^");
		appendHex(id, true);
		/*	if (pBaseType)
				pBaseType->aka(*this);
				else
				forTypeVoid();
				append("^");
				appendHex(id);*/
	}

	void forTypeProc()
	{
		assert(empty());
		append(TYPEID_PROC);
	}

	void forTypeCode(int id)
	{
		assert(empty());
		append(TYPEID_CODE);
		appendHex(id);
		//strcat(buf, TYPEID_CODE);
	}

	void forTypeThunk(T pCode)
	{
		forTypeCode(pCode->typeCode()->ID());
		append("?");
	}

	void forTypeProxy(T pIncumb)
	{
#if(1)
		pIncumb->type()->aka(*this);
#else
		pIncumb->aka(*this);
#endif
	}

	void forTypeConst(T pBaseType)
	{
		assert(pBaseType);
		pBaseType->aka(*this);
		append("@");
	}

	void forTypePair(T left, T right)
	{
		left->aka(*this);
		append(",");
		right->aka(*this);
	}

	void forTypeFunc(T ret, T args, unsigned f)
	{
		if (!ret)
			forOpTyp(OPTYP_NULL);
		else
			ret->aka(*this);
		append("(");
		if (args)
			args->aka(*this);
		append(")");
		if (f != 0)
		{
			append("|");
			appendHex(f);
		}
	}
private:
	void appendHex(unsigned v, bool w = false)
	{
		char buf[9];
		int i(sizeof(buf));
		buf[--i] = 0;
		for (; v != 0; v >>= 4)
		{
			/*int n(v & 0xF);
			if (n >= 10)
				buf[--i] = (n - 10) + 'A';
			else
				buf[--i] = n + '0';*/
			buf[--i] = digit2char(v & 0xF);
		}
		if (w)
			while (i > 0)
				buf[--i] = '0';//padding
		append(buf + i);
	}
	static char digit2char(uint8_t i)
	{
		i &= 0x0F;
		if (i < 0xA)
			return '0' + i;
		return 'A' + i - 0xA;
	}
};




