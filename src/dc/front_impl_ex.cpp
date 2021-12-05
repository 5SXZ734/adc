#include "front_impl_ex.h"

#include "db/symbol_map.h"
#include "info_proto.h"
#include "stubs.h"
#include "make_type.h"
#include "info_class.h"
#include "sym_parse.h"



FrontImplSymDump_t::FrontImplSymDump_t(Dc_t &rDC, FolderPtr pFolder)//, CFieldPtr pField)
	: FrontImplBase_t(rDC.project(), rDC.module()),
	mrDC(rDC),
	mpFolder(pFolder)
{
	pushScope(rDC.module(), 0, false);
	pushScope(rDC.primeSeg(), 0, false);
	/*if (pField)
	{
		assert(DcInfo_s::IsGlobal(pField));
		pushScope(OwnerSeg(pField->owner()), pField->_key(), false);
	}*/
}

void FrontImplSymDump_t::PushScope(TypePtr pCont, ADDR va)
{
	pushScope(pCont, va, false);
}

void FrontImplSymDump_t::selectFile(const char *path, const char* folder)
{
	MyString sPath(path);
	if (!sPath.isEmpty())
	{
		//fprintf(stdout, "%s\n", ss1.str().c_str());
		//MyString s(fixFileName(sPath, ModulePtr()));

		ZPath_t z(sPath);
		if (z.isFile())
			z.removeExt();
		assert(!z.isAbsolute());
		MyString sFolder(folder);
		if (!sFolder.isEmpty())
			z = ZPath_t(sFolder + "/" + z.toString());
		AssurePathAbsolute(z, ModulePtr());
CHECK(z.back() == "omniORB")
STOP
		MyString s(z.toString());

		FilesMgr0_t::fixPath(s);
		mpFolder = AddFile(s);
		DcInfo_t DI(mrDC);
		DI.AssureFileDef(mpFolder);
	}
	else
		mpFolder = nullptr;
}

TypePtr FrontImplSymDump_t::type(HNAME name)//no throw here
{
	TypesMgr_t *pTypesMgr;
	TypePtr p(nullptr);

	if (strncmp(name, "::", 2) == 0)//in global scope
	{
		name += 2;
		p = FindTypeByName(name, mrDC.primeSeg());
		//p = findType(mrDC.primeSeg(), name, &pTypesMgr);
	}
	else if (!isScopeEmpty())//?
	{
		assert(strchr(name, ':') == nullptr);
		p = findType(scope().cont(), name, &pTypesMgr);
	}
	return p;
}

TypePtr FrontImplSymDump_t::declTypedef(const char *pTypeName, TypePtr iTypeBase)
{
	DcInfo_t DI(mrDC);

	FolderPtr pFolder(mpFolder);
	if (!pFolder)
		pFolder = DI.AddFileEx(FPATH_FROM_DBG);

	FileInfo_t FI(mrDC, *pFolder->fileDef(), memMgrGlob());
	StubPool_t stubs;
	TypeFomNode_t TN(FI, stubs, false);

	if (strncmp(pTypeName, "::", 2) == 0)//in global scope
	{
		pTypeName += 2;
		std::list<TypePtr> newTypes;
		FullName_t aName(DcInfo_s::SplitScopedName(pTypeName));
		return TN.ProcessTypedef(aName, iTypeBase);
	}

	TypePtr pOwner(DI.PrimeSeg());
	if (!isScopeEmpty())
		pOwner = scope().cont();

	return FI.AddTypedefEx(iTypeBase, pTypeName, pOwner);

/*	if (mpFolder)
	{
		FolderPtr pFolder2(USERFOLDER(iType));
		if (pFolder2 != mpFolder)
		{
			if (ModuleOf(iType) != ModulePtr())
			{
				FileInfo_t &FI2(FI);//mrDC, *mpFolder->fileDef(), memMgrGlob());
				TypePtr iTop0(TypeTop(iType));
				//assert(iTop0 == iClass0);//later
				if (FI2.CanRelocateType(iTop0))
					FI2.RelocateType(iTop0, pTypeName);
				else
				{
					fprintf(STDERR, "Warning: Failed to accommodate %s at %s\n", pTypeName, mpFolder->name().c_str());
				}
			}
			else
				DI.AddTypeToFile(iType, mpFolder->fileDef());
		}
	}
	return iType;*/
}


TypePtr FrontImplSymDump_t::NewScope(const char* pTypeName, SCOPE_enum eScope, AttrScopeEnum eAttr)
{
	return newScope(pTypeName, eScope, eAttr);
}

TypePtr FrontImplSymDump_t::newScope(const char *pTypeName, SCOPE_enum eScope, AttrScopeEnum eAttr)
{
	DcInfo_t DI(mrDC);
	FolderPtr pFolder(mpFolder);
	if (!pFolder)
		pFolder = DI.AddFileEx(FPATH_FROM_DBG);

	FileInfo_t FI(mrDC, *pFolder->fileDef(), memMgrGlob());
	StubPool_t stubs;
	TypeFomNode_t TN(FI, stubs, false);

	//if name is prepended with double-colon, this is a fully quallified name
	MyString sFullName(pTypeName);
	bool bGlobalScope(sFullName.startsWith("::"));//in global scope?
	if (bGlobalScope)
		sFullName.erase(0, 2);
	
	FullName_t aName(DcInfo_s::SplitScopedName(sFullName));
	FullName_t aOwnerClass;

	if (eScope == SCOPE_FUNC)
	{
		Locus_t aLoc;
		aLoc.add(scope().cont(), scope().m_addr, nullptr);

		//TypePtr iProc(scope().cont());
		//if (!iProc->typeProc())
		if (!aLoc.field0() || !aLoc.field0()->isTypeProc())
		{
			PrintWarning() << "No procedure at " << aLoc.toString() << " to bind a function" << std::endl;
			return nullptr;
		}

		TypePtr iOwnerClass(nullptr);
		if (bGlobalScope)
		{
			if (aName.size() > 1)//scoped
			{
				aOwnerClass = aName.scope();
				iOwnerClass = TN.ProcessClass(aOwnerClass, 1, false, false);
				if (!iOwnerClass)
					return nullptr;//?
			}
		}
		else
		{
			iOwnerClass = scope(1).cont();//examine a context where the procedure is declared, it may be a class
			if (iOwnerClass->typeSeg())
				iOwnerClass = nullptr;
		}

		FieldPtr pField(nullptr);
		FieldPtr pField0(aLoc.field0());//can be a clone master
		GlobPtr pGlob(DcInfo_s::GlobObj(pField0));
		if (pGlob && (!pGlob->func() || ProtoInfo_t::FuncStatus(pGlob) == 0))
			pField = pField0;//data and code do not co-exist
//CHECKID(pField0, 0x5bdb)
//CHECK(pField0->address()==0x61fe0)
//STOP
		if (!pField)
		{
			pField = DI.AssureDockField(pField0, sFullName);//may map into a clone
			if (!pField)
				pField = pField0;
		}

		pGlob = DcInfo_t::GlobObj(pField);
		if (!pGlob)
		{
			pGlob = DI.NewFuncDef(pField, pFolder);
			pField = DcInfo_s::DockField(pGlob);//refresh!
		}
		else
		{
			if (!pGlob->func())
				DI.AssureFuncDef(pGlob);
			else if (ProtoInfo_t::FuncStatus(pGlob) != 0)//NOT __noproto, prootype has already been defined
			{
				FullName_t aName2(DI.GlobNameFull(pGlob, DcInfo_t::E_PRETTY_NULL));//no dub suffixes
				//check for identity (and report)
				return nullptr;
			}
		}

		if (iOwnerClass)
		{
CHECKID(iOwnerClass, 0x1d61)
STOP
			//unsigned fattr(static_cast<unsigned>(attr2));
			//unsigned fmask(static_cast<unsigned>(AttrScopeEnum::ATTRP__MASK));
			bool bVirtual(eAttr/*(fattr & fmask)*/ == AttrScopeEnum::ATTRP_VIRTUAL);
			bool bMaybeNSpace(!bVirtual);
			if (DI.AssureTypeClass(iOwnerClass, bMaybeNSpace))
			{
				ClassInfo_t classInfo(DI, iOwnerClass);
				if (bVirtual)
					classInfo.AddClassVirtualMember(pGlob);
				else
					classInfo.AddClassMember(pGlob);
			}
		}
		if (!sFullName.isEmpty())// && !pGlob->hasPrettyName())
		{
			if (!iOwnerClass)//global
			{
				assert(DcInfo_t::IsGlobal(pField));
				MyString s(DI.EnhancedName(aName.back(), aOwnerClass.join()));
				if (!s.isEmpty())
				{
					MyStringEx aNamex(ToCompoundName(pField->name()));
					if (aNamex.empty(0))
					{
						aNamex.set(0, s);
						DI.RenameField(pField, FromCompoundName(aNamex));
					}
				}
			}
			assert(pGlob);
			DI.ApplyPrettyName(pGlob, sFullName, iOwnerClass);
		}

		if (pGlob)
		{
			pushScope((GlobToTypePtr)pGlob, 0, false);
			//if (CHECK_ATTR_TYPE(attr, __ATTR_PROC))
			if (eAttr != AttrScopeEnum::null)
			{
				//?assert((attr & FDEF__FMOD_MASK) == 0);
				if (eAttr == AttrScopeEnum::ATTRP_STATIC)
					pGlob->flags() |= FDEF_STATIC;
				if (eAttr == AttrScopeEnum::ATTRP_VIRTUAL)
				{
					STOP
				}
				//pGlob->flags() |= FDEF_VIRTUAL;
			}
			ProtoInfo_t::SetFuncStatus(pGlob, FDEF_DEFINED);
		}
		return (TypePtr)pGlob;
	}

	if (bGlobalScope)//in global scope
	{
		FileInfo_t::E_KIND eClass;
		switch (eScope)
		{
		case SCOPE_CLASS:
			eClass = FileInfo_t::E_KIND_CLASS; break;
		/*case SCOPE_UNION:
			eClass = FileInfo_t::E_KIND_UNION; break;*/
		default:
			eClass = FileInfo_t::E_KIND_STRUC; break;
		}
		TypePtr iType(TN.ProcessCompound(aName, 0, false, eClass));//may be external?
		if (iType)
		{
//CHECKID(iType, 0x17529)
//STOP
			if (eScope == SCOPE_ENUM)
				iType->flags() |= TYP_ENUM;
			else if (eScope == SCOPE_NAMESPACE)
				DI.AssureTypeNamespace(iType);

			pushScope(iType, 0, false);
			if (pFolder)
			{
				FolderPtr pFolder2(USERFOLDER(iType));
				if (pFolder2 != pFolder)
				{
					if (ModuleOf(iType) != ModulePtr())
					{
						TypePtr iTop0(TypeTop(iType));
						//assert(iTop0 == iClass0);//later
						if (FI.CanRelocateType(iTop0))
							FI.RelocateType(iTop0, aName);
						else
							PrintWarning() << "Failed to relocate " << aName << " at " << pFolder->name() << std::endl;
					}
					else //if (!iType->isNested() || iType->owner()->typeNamespace())
						if (!USERFOLDER(iType) 
							//|| mrDC.isFolderOfKind(*USERFOLDER(iType), FPATH_FROM_DBG))
							|| FolderPath(USERFOLDER(iType)) == FPATH_FROM_DBG)//was this type previously added to the default file?
							DI.AddTypeToFile(iType, pFolder);
				}
			}
		}
		//if (typeId != 0)
			//mIdMap.insert(std::make_pair(typeId, iType));
		return iType;
	}

	MyString sName(sFullName);

	/*MyString sFieldName(pFieldName);
	bool bGlobalField(sFieldName.startsWith("::"));//in global scope?
	if (bGlobalField)
		sFieldName.erase(0, 2);*/

	bool bClass(false);
	if (eScope == SCOPE_CLASS)
	{
		bClass = true;
		eScope = SCOPE_STRUC;
	}

	TypePtr iType(NewScopeImpl(sName, eScope));
	if (iType)
	{
		if (bClass)
			DI.AssureTypeClass(iType);

		if (eScope == SCOPE_VFTABLE)
		{
			abort();
#if(0)
			//Locus_t aLoc;
			//aLoc.add(scope().cont(), scope().m_addr, nullptr);

			FullName_t aName(DcInfo_s::SplitScopedName(sFieldName));
			FullName_t aOwnerClass;

			if (aName.size() > 1)//scoped
			{
				aOwnerClass = aName.scope();
				TypePtr iClass(TN.ProcessClass(aOwnerClass, 1, false, false));
				if (iClass && DI.AssureTypeClass(iClass))
				{
					ClassInfo_t classInfo(DI, SkipModifier(iClass));
					//if (classInfo.AddClassMember(mpField))//AddStaticMember
					//if (node->hasName())
						//ApplyPrettyName(mpField, sFullName, classInfo.ClassPtr(), 0);
				}
			}
#endif
		}
		else
		{
			if (iType->hasUglyName())
				DI.ApplyPrettyName(iType, sName, iType->owner());
			if (iType->isShared())
			{
				DI.AddTypeToFile(iType, pFolder);
				//if (iType->isShared())
				{
					MyString s(DI.TypeNameFull(iType).join());
					if (!mrDC.project().findExpTypeMap(s, ModulePtr(), true))
						mrDC.project().addExpTypesMap(s, iType);
				}
			}
		}
	}

	return iType;
}

FieldPtr FrontImplSymDump_t::declFuncField(MyString sName, TypePtr iType, AttrIdEnum attr, ADDR at)
{
	DcInfo_t DI(mrDC);
	TypePtr iScope(scope().cont());

	FolderPtr pFolder(mpFolder);
	if (!pFolder)
		pFolder = DI.AddFileEx(FPATH_FROM_DBG);

	GlobPtr pGlob(iScope->objGlob());
	if (iType && iType == iScope)//a scopefield?
		ASSERT0;// return declFieldImpl(sName, iType, attr);

	ProtoInfo_t FI(mrDC, pGlob);
	ProtoInfo_t::AppendFuncArgFlags flags(ProtoInfo_t::AFA_NULL);
	if (CHECK_ATTR_TYPE(attr, __ATTR_FUNC))
	{
		if (attr == ATTRF_RETVAL)
			return FI.AppendFuncRet(iType);
		if (attr == ATTRF_REGISTER)
			flags = ProtoInfo_t::AFA_REGISTER;
		else if (attr == ATTRF_THISPTR)
		{
			TypePtr pClass(IsConstPtrToStruc(iType, true));
			if (pClass)
			{
				TypePtr pClass1(DcInfo_t::OwnerScope(pGlob));
				if (pClass1)
				{
					if (pClass == pClass1)
					{
						if (FI.AssureTypeClass(pClass))
						{
							if (at == -1)
								flags = ProtoInfo_t::AFA_THISPTR;
						}
						else
							printError() << "Failed to convert namespace ("
							<< hyperLinked(FI.TypePrettyNameFull(pClass, CHOP_SYMB)) << ") to class"
							<< std::endl;
					}
					else
						printError() << "A type used for 'this' ptr ("
						<< hyperLinked(FI.TypePrettyNameFull(pClass, CHOP_SYMB)) << ") is not a scope of the method: "
						<< hyperLinked(DI.GlobNameFull(pGlob, DcInfo_t::E_PRETTY, CHOP_SYMB))
						<< std::endl;
				}
				else
					printError() << "'this' ptr for non-member method rejected" << std::endl;
			}
			else
				printError() << "Invalid type for 'this' ptr" << std::endl;
		}
		attr = ATTR_NULL;
	}
	FieldPtr pArg(nullptr);
	if (at == -1)
	{
		//if (flags == ProtoInfo_t::AFA_THISPTR)
			//pArg = FI.AssureThisPtrTo(pGlob, nullptr, false);
		//else
		pArg = FI.AppendFuncArg(iType, flags);
		if (!pArg)
			return nullptr;
		if (flags == ProtoInfo_t::AFA_THISPTR)
			FI.MakeThisCallFromArg(pArg);

		scope().m_addr = pArg->_key();
		if (!sName.isEmpty())
		{
			FI.AssureNamespace((GlobToTypePtr)FI.FuncDefPtr());
			assert(!pArg->isExported());
			FI.SetFieldName2(pArg, sName, false);
		}
	}
	else
	{
		//pArg = InsertFieldOfType(scp, iType);
		if (scope().cont()->typeBitset())
			popScope();
		if (at != -1)
			setcpImpl(at);
		pArg = declFieldImpl(sName, iType, attr);
		//pArg = (FieldPtr)IFront_Base::declField(sName, type, attr, at);
		if (!pArg)
			return nullptr;
		if (flags == ProtoInfo_t::AFA_THISPTR)
		{
			TypePtr pClass(pArg->isConstPtrToStruc());
			if (pClass)
			{
				ClassInfo_t classInfo(DI, pClass);
				if (classInfo.MakeClassMemberOf(pGlob))
					FI.MakeThisCallFromArg(pArg);
			}
		}
	}
	updateFrameSize(pArg->type());
	return pArg;
}

FieldPtr FrontImplSymDump_t::declUField(HNAME filedName, TypePtr iType, AttrIdEnum attr)
{
	TypePtr iScope(scope().cont());

	MyString sName(filedName);
	bool bGlobalScope(sName.startsWith("::"));//in global scope?
	if (bGlobalScope)
		sName.erase(0, 2);

	if (iType && iType == iScope)//a scopefield?
		return declFieldImpl(sName, iType, attr);

	if (CHECK_ATTR_TYPE(attr, __ATTR_FUNC) && !iScope->typeFuncDef())
	{
		printError(signalError("invalid attribute for non-function scope"));
		return nullptr;
	}

	if (iScope->typeFuncDef())
	{
		printError(signalError("invalid scope for U-field"));
		return nullptr;
	}

	return declUnionField(sName, iType, attr);
}

FieldPtr FrontImplSymDump_t::declUField(HNAME filedName, SCOPE_enum eScope, AttrIdEnum attr)
{
	assert(0);
	return 0;
}

FieldPtr FrontImplSymDump_t::declField(HNAME fieldName, TypePtr iType, AttrIdEnum attr, ADDR at, I_Module& iface)
{
	TypePtr iScope(scope().cont());

	MyString sName(fieldName);
	bool bGlobalScope(sName.startsWith("::"));//in global scope?
	if (bGlobalScope)
		sName.erase(0, 2);

	if (iType && iType == iScope)//a scopefield?
		return declFieldImpl(sName, iType, attr);

	if (CHECK_ATTR_TYPE(attr, __ATTR_FUNC) && !iScope->typeFuncDef())
	{
		printError(signalError("invalid attribute for non-function scope"));
		return nullptr;
	}

	if (iScope->typeFuncDef())
	{
		return declFuncField(sName, iType, attr, at);
	}

	FieldPtr pField;
	if (iScope->typeUnion())
	{
		return nullptr;
		//assert(0);
		//pField = declUnionField(sName, iType, attr);
	}
	//else
	{
		closeBitset();
		if (at != -1)
			setcpImpl(at);
		pField = declFieldImpl(sName, iType, attr);
	}
	if (!pField)
		return nullptr;

	updateFrameSize(pField->type());

	if (!iScope->typeSeg())//global var?
		return pField;
	//if (pField->isCloneMaster())
		//return pField;
	if (pField->isTypeProc())
		return pField;

	DcInfo_t DI(mrDC);
	FolderPtr pFolder(mpFolder);
	if (!pFolder)
		pFolder = DI.AddFileEx(FPATH_FROM_DBG);
	FieldExPtr pFieldx(DI.AddGlobToFile2(pField, pFolder));

	FullName_t aFullName(DcInfo_s::SplitScopedName(sName));

	FileInfo_t FI(DI, *pFolder->fileDef(), memMgrGlob());

	//check class membership
	if (aFullName.size() > 1)//has a scope
	{
		StubPool_t stubs;
		TypeFomNode_t TN(FI, stubs, false);

		TypePtr iClass(TN.ProcessClass(aFullName.scope(), 0, false, false));
		if (!iClass)
			return nullptr;

		TypePtr pClass(SkipModifier(iClass));

		if (pClass->typeClass()//namespaces allowed - it s a static member
			|| TN.AssureTypeNamespace(iClass, false)
			|| TN.AssureTypeClass(iClass))
		{
			ClassInfo_t classInfo(TN, SkipModifier(iClass));
			if (classInfo.AddClassMember(pFieldx->globPtr()))
				TN.ApplyPrettyName(pFieldx->globPtr(), sName, classInfo.ClassPtr(), 0);
		}
	}

	if (pFieldx->name())
	{
		node_t* pn(DI.FromSymbol(pFieldx->name()->c_str()));
		if (pn)
			DI.CheckUglyName(pFieldx, pn->name);
	}

	return pFieldx;
}


void FrontImplSymDump_t::dumpImpl(ADDR va, OFF_t oSymbolName, unsigned uNameMax, unsigned uFlags, const StubPool_t& stb)
{
	if (oSymbolName == OFF_NULL)
	{
		dumpImpl(va, nullptr, uFlags, stb);
	}
	else
	{
		DataStream_t aPtr(GetDataSource()->pvt(), oSymbolName);
		std::ostringstream ss;
		aPtr.fetchString(ss, uNameMax);
		dumpImpl(va, ss.str().c_str(), uFlags, stb);
	}
}

void FrontImplSymDump_t::dumpImpl(ADDR va, const char *pMangled, unsigned uFlags, const StubPool_t& stb)
{
	DcInfo_t DI(mrDC);
	if (!mpFolder)
		mpFolder = DI.AddFileEx(FPATH_FROM_DBG);
	FileInfo_t dbgFile(mrDC, *mpFolder->fileDef(), memMgrGlob());

CHECK(va == 0x5f4a67d0)
STOP

	MyString sMangled(pMangled);
	MyString sDemangled;
	I_Front::SymbolKind eKind(demangleSymbol(mrDC.frontEnd(), sMangled, sDemangled));

	node_t *pn(nullptr);
	if (!sDemangled.isEmpty() && sDemangled != sMangled)
		pn = DI.projx().cxxSymbols().addSymbol(sMangled, sDemangled);//add one to ProjectEx_t::cxxSymbols()
	//fprintf(stdout, "SYMBOL @%08X %s\n", va, sMangled.c_str());

	if (1 && eKind > I_Front::SYMK_SPECIAL)
	{
		if (pn && pn->type == NODE_NULL)//data
		{
			FrontImplSymDump_t fimp(mrDC, mpFolder);//should I use a CP?
			TypePtr pSeg(DI.FindSegAt(DI.PrimeSeg(), va, DI.PrimeSeg()->typeSeg()->affinity()));
			if (pSeg)
			{
				fimp.PushScope(pSeg, va);
				IFront_Src<I_Module> iface(fimp);
				I_Front* pIFront(DI.IFrontOf(DI.PrimeSeg()));
				if (pIFront->processSpecial(iface, eKind, pn->name.c_str()))//use a parsed name to avoid modifiers
				{
					if (pn->isConst())
					{
						Locus_t aLoc;
						aLoc.add(pSeg, va, nullptr);
						aLoc.field0()->flags() |= FLD_CONST;
					}
					return;
				}
			}
		}
	}


//CHECK(sMangled.startsWith("__ZN2nc4core2ir14MemoryLocationC1Eixx.part.4"))
//STOP

	TypeFomNode_t conv(dbgFile, stb,  true);

	try
	{
		conv.ProcessDebugSymbolEntry(va, pn, eKind, sMangled, uFlags == 1);//func?
	}
	catch (int err)
	{
		PrintError() << "Failed to process debug symbol in module '" << ModuleTitle(ModulePtr()) << "' (code=" << err << "):"
			<< std::endl << "\t" << sDemangled << std::endl;
	}

}

