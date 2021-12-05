#pragma once

#include <list>

//enum ProtoAttrEnum { PROTO_ATTR_CDECL, PROTO_ATTR_STDCALL, PROTO_ATTR_UNDEFINED, PROTO_ATTR_STUB, PROTO_ATTR_INTRINSIC, PROTO_ATTR_PROCESSING, PROTO_ATTR_INCOMPLETE };
enum FuncStatusEnum : int { FUNCSTAT_INCOMPLETE = -2, FUNCSTAT_PROCESSING = -1, FUNCSTAT_OK = 0, FUNCSTAT_UNDEFINED = 1, FUNCSTAT_STUB = 2, FUNCSTAT_INTRINSIC = 3 };

template <typename TDUMPER>
class TProtoDumper : public TDUMPER
{
	typedef		TDUMPER	BASE;

protected:
	using TYPEPTR = typename BASE::TYPEPTR;
	using FIELDPTR = typename BASE::FIELDPTR;
	using FUNCPTR = typename BASE::FUNCPTR;
	using RETPTR = typename BASE::RETPTR;
	using ARGDUMPER = typename BASE::ARGDUMPER;
	using ARGINFO = typename BASE::ARGINFO;

	using BASE::dumpTab;
	using BASE::dumpStr;
	using BASE::dumpReserved;
	using BASE::dumpBasicType;

private:
	struct type_level_node_t
	{
		TYPEPTR p;
		bool braces;
		type_level_node_t()
			: p(nullptr),
			braces(false)
		{
		}
		type_level_node_t(TYPEPTR _p)
			: p(_p),
			braces(false)
		{
		}
	};
public:

	std::list<type_level_node_t>	mStack;
	bool		mbTabSep;	//kind of separator between base type and field
	int			miLastSymb;//whatever dumped last : 0:nothing, 1:identifer, 2:punct(*,&,etc)

public:
	/*TProtoDumper()
		: mbTabSep(0),
		miLastSymb(0)
	{
	}*/
	TProtoDumper(const TDUMPER &r)
		: TDUMPER(r),
		mbTabSep(0),
		miLastSymb(0)
	{
	}

	TProtoDumper(const TProtoDumper &r)
		: TDUMPER(r),
		mbTabSep(0),
		miLastSymb(0)
	{
	}

	void dumpFieldDeclaration(FIELDPTR pSelf, TYPEPTR pType, TYPEPTR pScope)
	{
		dumpType(pType, pScope);//BASE::fieldType(pSelf)
		dumpFieldSeparator(pSelf);
		dumpFieldNameScopped(pSelf, pScope);
		unrollStack(pSelf, pScope);
	}

	void dumpFunctionDeclaration(FIELDPTR pSelf, TYPEPTR pType, TYPEPTR pScope)
	{
		dumpFuncDeclaration0(pType, pScope);//BASE::fieldType(pSelf)
		dumpFieldSeparator(pSelf);
		dumpFieldNameScopped(pSelf, pScope);
		unrollStack(pSelf, pScope);
	}

	void dumpTypeDeclaration(TYPEPTR pSelf, TYPEPTR pScope)//any type (including functions)
	{
		dumpType(pSelf, pScope);
		unrollStack(pSelf, pScope);
	}

	void dumpFunctionDeclaration(TYPEPTR pSelf, TYPEPTR pScope)
	{
		dumpType(pSelf, pScope);
		BASE::dumpTypeRef(pSelf);
		unrollStack(pSelf, pScope);
	}

	void dumpTypedefDeclaration(TYPEPTR pSelf, TYPEPTR pScope)
	{
//CHECKID(pSelf, 0x12348)
//STOP
		BASE::dumpReserved("typedef");
		dumpSpace();
		dumpType(BASE::baseType(pSelf), pScope);
		dumpTypedefSeparator();
		BASE::dumpTypeRef(pSelf);
		unrollStack(pScope);
	}

	void dumpFuncdefDeclaration(TYPEPTR pSelf, TYPEPTR pScope)//no func field
	{
		//assert(BASE::isFunctionType(pSelf));
		//assert(BASE::isFunctionIntrinsic(pSelf));
		/*{
			dumpReserved("typedef");
			dumpSpace();
		}*/

		dumpFuncDeclaration0(pSelf, pScope);//BASE::scope());

		BASE::dumpTypeRef(pSelf);

		unrollStack(nullptr, pScope);
	}

//private:

	void dumpFieldNameScopped(FIELDPTR pSelf, TYPEPTR pScope)
	{
		TYPEPTR pFieldScope(BASE::fieldScope(pSelf));
		if (pFieldScope && pScope == pFieldScope)
			pFieldScope = nullptr;//reset
		if (pFieldScope)
		{
			dumpTypeNameScoped(pFieldScope, pScope);//BASE::scope());
			dumpStr("::");
		}
		BASE::drawFieldName(pSelf);
	}

	void dumpFieldSeparator(FIELDPTR pSelf)
	{
		if (miLastSymb == 1 || miLastSymb == 2)
		{
			if (BASE::useTabSeparator(pSelf))
				dumpTab();
			else
				dumpSpace();
		}
	}

	void dumpTypedefSeparator()
	{
		if (miLastSymb == 1 || miLastSymb == 2)
			dumpTab();
	}

	void unrollStack(FIELDPTR pField, TYPEPTR pScope)
	{
		bool bBitfield(pField && BASE::isBitfield(pField));
		if (mStack.empty())
		{
			if (bBitfield)
			{
				dumpStr(":");
				BASE::dumpArrayIndex(1);
			}
		}
		else
		{
			for (; !mStack.empty(); mStack.pop_back())
			{
				const type_level_node_t& b(mStack.back());
				if (b.braces)
					dumpStr(")");
				if (BASE::isArrayType(b.p))
					dumpArraySubscripts(b.p, bBitfield);
				else
				{
					//assert(BASE::isFunctionType(b.p));
					dumpFuncArguments(b.p, pScope);
				}
			}
		}
	}

	void unrollStack(TYPEPTR pScope)//for typedefs
	{
		for (; !mStack.empty(); mStack.pop_back())
		{
			const type_level_node_t &b(mStack.back());
			if (b.braces)
				dumpStr(")");
			if (BASE::isArrayType(b.p))
				dumpArraySubscripts(b.p, false);
			else
			{
				assert(BASE::isFunctionType(b.p));
				dumpFuncArguments(b.p, pScope);
			}
		}
	}

	void dumpSpace()
	{
		dumpStr(" ");
	}

	void dumpComma(bool bSpace)
	{
		dumpStr(",");
		if (bSpace)
			dumpSpace();
	}

	bool checkBraces(TYPEPTR pSelf)
	{
		assert(BASE::isPointerType(pSelf));
		//for (; pSelf && pSelf->typePtr(); pSelf = pSelf->baseType());
		TYPEPTR iPointee(BASE::baseType(pSelf));
		if (iPointee)
			if (BASE::isArrayType(iPointee) || BASE::isFunctionType(iPointee))
				if (!mStack.empty() && mStack.back().p == iPointee)
				{
					mStack.back().braces = true;
					return true;//array has a greater (right-hand) binding
				}
		return false;
	}

	void dumpArraySubscripts(TYPEPTR pSelf, bool bBitfield)
	{
		for (; BASE::isArrayType(pSelf); pSelf = BASE::baseType(pSelf))
		{
			if (!bBitfield)
				dumpStr("[");
			else
				dumpStr(":");
			BASE::dumpArrayIndex(BASE::arrayIndex(pSelf));
			if (!bBitfield)
				dumpStr("]");
		}
	}

	void dumpFuncArguments(FUNCPTR pSelf, TYPEPTR pScope)
	{
		//assert(BASE::isFunctionType(pSelf));
		std::vector<ARGINFO> args;
		bool isMoreArgs(BASE::getFunctionArguments(pSelf, args));
		dumpStr("(");
		size_t iCount(0);
		for (; iCount < args.size(); iCount++)
		{
			if (iCount > 0)
				dumpComma(true);
			TProtoDumper<ARGDUMPER> proto2(*this);
			proto2.dumpFuncArgument(args[iCount], pScope);// BASE::ownerScope(pSelf));//dump arguments in their function's scope
		}
		if (isMoreArgs)
		{
			if (iCount > 0)//?
				dumpComma(true);
			dumpStr("...");
		}
		dumpStr(")");
	}

	void dumpFuncArgument(const ARGINFO& arg, TYPEPTR pScope)
	{
		//if (BASE::isRegister(arg))
		{
			if (!arg.sReg.empty())
			{
				dumpReserved("register");
				dumpStr("<");
				dumpStr(arg.sReg.c_str());
				dumpStr(">");
				dumpSpace();
			}
		}
		//TYPEPTR pType(BASE::argumentType(pArg));
		dumpType(arg.pType, pScope);//BASE::scope());
		//if (isArgVisible(pArg))
		if (arg.pField)
		{
			dumpFieldSeparator(arg.pField);
			dumpFieldNameScopped(arg.pField, pScope);
		}
		unrollStack(nullptr, pScope);
	}

	void dumpFuncDeclaration0(RETPTR pSelf, TYPEPTR pScope)//int ptr_level)
	{
		std::vector<ARGINFO>	rets;
		BASE::getFunctionReturnValues(pSelf, rets);

		size_t iRet(-1);
		bool bExtraRets(false);
		for (size_t i(0); i < rets.size(); i++)
		{
			if (iRet != -1)
			{
				bExtraRets = true;
				break;
			}
			iRet = i;
		}

		if (iRet == -1)
		{
			dumpReserved("void");
			miLastSymb = 1;
		}
		else
		{
			const ARGINFO& ret(rets[iRet]);
			if (!ret.sReg.empty())
			{
				dumpReserved("register");
				dumpStr("<");
				dumpStr(ret.sReg.c_str());
				dumpStr(">");
				dumpSpace();
			}
			dumpType(ret.pType, pScope);//with space separator
			if (bExtraRets)
				dumpStr("...");//?
		}

#if(0)
		//if (BASE::IsDefinition())
		{
			if (BASE::dumpAttribute(pSelf, BASE::functionStatus(pSelf)))
			/*if (iStatus != FUNCSTAT_OK)
			{
				dumpSpace();
				if (iStatus == FUNCSTAT_PROCESSING)
					dumpAttribute(PROTO_ATTR_PROCESSING);
				else if (iStatus == FUNCSTAT_INCOMPLETE)
					dumpAttribute(PROTO_ATTR_INCOMPLETE);
				else if (iStatus == FUNCSTAT_UNDEFINED)
					dumpAttribute(PROTO_ATTR_UNDEFINED);
				else if (iStatus == FUNCSTAT_STUB)
					dumpAttribute(PROTO_ATTR_STUB);
				else if (iStatus == FUNCSTAT_INTRINSIC)
					dumpAttribute(PROTO_ATTR_INTRINSIC);*/
				miLastSymb = 1;
			//}
		}
		//else
		if (!BASE::IsDefinition())
		{
			if (BASE::isFunctionStdCall(pSelf))//__stdcall is default for non-static class members 
			{
				dumpSpace();
				dumpReserved("__stdcall");
				miLastSymb = 1;
			}
		}
#else
		if (BASE::dumpAttributes(pSelf))
			miLastSymb = 1;
#endif

		if (miLastSymb != 3)
			dumpSpace();
		miLastSymb = 0;
		mStack.push_back(pSelf);//new level

	}

	void dumpTypeNameScoped2(TYPEPTR pSelf, TYPEPTR pScopeFrom)
	{
		if (pSelf != pScopeFrom)
		{
			TYPEPTR pOwner(BASE::ownerScope(pSelf));
			if (pOwner && pOwner != pScopeFrom)
			{
				dumpTypeNameScoped2(pOwner, pScopeFrom);
				dumpStr("::");
			}
		}
		BASE::dumpTypeRef(pSelf);
	}

	void dumpTypeNameScoped(TYPEPTR pSelf, TYPEPTR pScope)
	{
		//TYPEPTR pScope(BASE::scope());
//		if (!pScope || pSelf == pScope)
//			return BASE::dumpTypeRef(pSelf);
		dumpTypeNameScoped2(pSelf, CommonScope(pScope, pSelf));
	}
	TYPEPTR CommonScope(TYPEPTR p0, TYPEPTR q0)
	{
		for (TYPEPTR p(p0); p; p = BASE::ownerScope(p))
		{
			for (TYPEPTR q(q0); q; q = BASE::ownerScope(q))
			{
				if (p == q)
				{
					return p;
				}
			}
		}
		return nullptr;
	}

	void dumpType(TYPEPTR pSelf, TYPEPTR pScope)
	{
		if (pSelf && !BASE::isTypeOkToDump(pSelf))
			pSelf = nullptr;

		if (!pSelf)
		{
			BASE::dumpBasicType(nullptr);
			miLastSymb = 1;
		}
		else if (BASE::isPointerType(pSelf))
		{
			if (BASE::isSpecialPointerType(pSelf))
			{
				BASE::dumpBasicType(pSelf);
				miLastSymb = 1;
			}
			else
			{
				dumpType(BASE::baseType(pSelf), pScope);
				//if (BASE::isImportPointerType(pSelf))
				//return;
				if (checkBraces(pSelf))
				{
					if (miLastSymb == 1)
						dumpSpace();
					dumpStr("(");
					miLastSymb = 3;//a sign not to put a space before identifier
				}
				else
					miLastSymb = 2;
				//no need to specify indirection on imported fields
				//?if (!field() || !root().IsTypeImp(field()) || field()->type() != pSelf)
				{
					if (BASE::isRvalReferenceType(pSelf))
						dumpStr("&&");
					else if (BASE::isReferenceType(pSelf))
						dumpStr("&");
					else
						dumpStr("*");
				}
			}
		}
		else if (BASE::isExcludedType(pSelf))
		{
			BASE::dumpTypeRef(pSelf);
			dumpSpace();
			dumpStr("{}");
			miLastSymb = 1;
		}
		else if (!BASE::baseType(pSelf))//terminal
		{
			if (BASE::isFunctionType(pSelf))//? && !BASE::isSharedType(pSelf))// || (pField && pField->IsTypeImp())))
			{
				dumpFuncDeclaration0(pSelf, pScope);
			}
			else
			{
				if (BASE::isBasicType(pSelf))
					BASE::dumpBasicType(pSelf);
				else
					dumpTypeNameScoped(pSelf, pScope);

				miLastSymb = 1;
			}
		}
		else if (BASE::isEnumerationType(pSelf))
		{
			dumpTypeNameScoped(BASE::baseType(pSelf), pScope);
			miLastSymb = 1;
		}
		else if (BASE::isTypedefType(pSelf))
		{
			dumpTypeNameScoped(pSelf, pScope);
			miLastSymb = 1;
		}
		else
		{
			//TypeConst_t *pTypeConst(pSelf->typeConst());
			if (BASE::isConstantType(pSelf))
			{
				dumpType(BASE::baseType(pSelf), pScope);
				if (miLastSymb != 0)//symb
					dumpSpace();
				dumpReserved("const");
				miLastSymb = 1;
			}
			else if (BASE::isArrayType(pSelf))
			{
				dumpType(BASE::baseType(pSelf), pScope);
				if (mStack.empty() || mStack.back().p != BASE::baseType(pSelf))//subarray?
					mStack.push_back(pSelf);//new level
				else
					mStack.back() = pSelf;
			}
			else//thunk?
			{
				BASE::dumpBasicType(nullptr);
				miLastSymb = 1;
			}
		}
	}
};

