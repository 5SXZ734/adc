#include "info_dc.h"
#include "prefix.h"
#include "db/type_proxy.h"
#include "db/type_code.h"
#include "db/symbol_map.h"
#include "db/info_types.h"
#include "db/types_mgr.h"
#include "clean_ex.h"
#include "ui_main_ex.h"
#include "sym_parse.h"
#include "ana_main.h"
#include "arglist.h"
#include "expr_simpl.h"
#include "info_class.h"
#include "expr_dump.h"
#include "cc.h"

///////////////////////////////////////////////////
// DcInfo_t

DcInfo_t::DcInfo_t(const DcInfo_t &r)
	: ModuleInfo_t(r),
	mrDC(r.mrDC),
	mrFrontDC(r.mrFrontDC),
	mrFE(r.mrFE)
{
}

DcInfo_t::DcInfo_t(const DcInfo_t &r, MemoryMgr_t &rMM)
	: ModuleInfo_t(r, rMM),
	mrDC(r.mrDC),
	mrFrontDC(r.mrFrontDC),
	mrFE(r.mrFE)
{
}

DcInfo_t::DcInfo_t(Dc_t &rDC)
	: ModuleInfo_t(rDC.project(), *rDC.module()),
	mrDC(rDC),
	mrFrontDC(*mrDC.frontDC()),
	mrFE(*mrFrontDC.GetFE())
{
	//int n = ScopePos("CORBA::Object_var::operator CORBA::Object_var::T_ptr");
}

DcInfo_t::DcInfo_t(const Dc_t &rDC)
	: ModuleInfo_t(rDC.project(), *rDC.module()),
	mrDC(const_cast<Dc_t &>(rDC)),
	mrFrontDC(*mrDC.frontDC()),
	mrFE(*mrFrontDC.GetFE())
{
}

DcInfo_t::DcInfo_t(Dc_t &rDC, MemoryMgr_t &rMemMgr)
	: ModuleInfo_t(ModuleInfo_t(rDC.project(), *rDC.module()), rMemMgr),
	mrDC(rDC),
	mrFrontDC(*mrDC.frontDC()),
	mrFE(*mrFrontDC.GetFE())
{
}

RegMaskType DcInfo_t::ToRegMask(const r_t *r_list) const
{
	RegMaskType mask(0);
	if (r_list)
	{
		for (const r_t* r = r_list; r->ssid; r++)
		{
			RegMaskType rmask(TOREGMASK(r->ofs, r->siz));
			mask |= rmask;
		}
	}
	return mask;
}

static bool toGPRs(const r_t *r_list, GPRs_t &v)
{
	RegMaskType mask(0);
	if (r_list)
	{
		for (const r_t* r = r_list; r->ssid; r++)
		{
			v.push_back(REG_t(r->ofs, r->siz));
		}
	}
	return mask;
}

//for 32-bit regs only
static RegMaskType toRegMask(const r_t *r_list, uint32_t savedregs)//check if regs by mask are valid regs???
{
	RegMaskType ret(0);
	if (r_list)
	for (const r_t *r = r_list; r->ssid; r++)
	{
		RegMaskType rmask(TOREGMASK(r->ofs, r->siz));
#if(X64_SUPPORT)
		uint32_t m(uint32_t(rmask >> (r->ofs >> 1)));//squeeze
#else
		uint32_t m(rmask);
#endif
		uint32_t u(savedregs & m);
		if (u)
		{
			if (u != m)
				return 0;//error
			ret |= rmask;
		}
	}

	return ret;
}


/*void DcInfo_t::InitFuncProfile(FuncProfile_t &r) const
{
	toGPRs(mrFE.spoilt_regs_def, r.spoiltRegs);
	r.spoiledFlags = mrFE.spoilt_flags_def;
}*/


bool DcInfo_t::ToFuncProfile(const char *pBuf, FuncProfile_t &rFP) const
{
	FuncProfile_t si;

	for (;;)
	{
		si = FuncProfile_t();//clear
		//InitFuncProfile(si);
		si.spoiltRegs.clear();//none (or default?)

		//uint32_t spoiledFlags(0);

		MyString inRegs, spoiledRegs;

		try {
			std::istringstream iss(pBuf);
			for (int n(0); !(iss >> std::ws).eof(); n++)
			{
				switch (n)
				{
				case 0:
					iss >> si.stackin;
					continue;
				case 1:
					iss >> si.pstackPurge;
					continue;
				case 2:
					iss >> si.fpuin;
					continue;
				case 3:
					iss >> si.fpudiff;
					continue;
				case 4:
				{
					if (iss.peek() == '(')//multiple regs
					{
						iss.ignore();
						//iss >> std::ws >> std::hex >> savedregs;
						while (!iss.eof())
						{
							char c;
							iss >> c;
							if (isspace(c))
								continue;
							if (c == ')')
								break;
							inRegs += c;
						}
					}
					else
						iss >> inRegs;// >> savedregs;//single reg
					continue;
				}
				case 5:
					if (iss.peek() == '{')//multiple regs
					{
						iss.ignore();
						//iss >> std::ws >> std::hex >> savedregs;
						while (!iss.eof())
						{
							char c;
							iss >> c;
							if (isspace(c))
								continue;
							if (c == '}')
								break;
							spoiledRegs += c;
						}
					}
					else
						iss >> spoiledRegs;// >> savedregs;//single reg
					continue;
				case 6:
					iss >> std::hex >> si._flags;
					continue;
				//case 7:
					//iss >> std::hex >> spoiledFlags;
					//continue;
				/*case 8:
					iss >> cpuout;
					continue;*/
				}
				fprintf(STDERR, "Warning: Extra entries in .PROTO definition ignored\n");
				break;
			}
		}
		catch (int)
		{
			//PrintError() << "Invalid stub definition" << std::endl;
			return false;
		}

		//si.spoiledFlags = spoiledFlags;

		if ((si.stackin < 0) || !(abs(si.pstackPurge) < SHRT_MAX))
			return false;
		if ((si.fpuin < 0) || (si.fpudiff < -8))
			return false;
		if ((si.fpuin > 8) || (si.fpudiff > 8))
			return false;
//?		if (!checkcpuregs(si))
	//?		return false;

		if (!inRegs.empty())
		{
			assert(si.cpuin.empty());
			si.cpuin.push_back(REG_t());//placeholder for 'thisptr'
			MyStringList l(MyStringList::split(",", inRegs));
			for (MyStringList::const_iterator i(l.begin()); i != l.end(); ++i)
			{
				bool bThisPtr(false);
				MyString s(*i);
				if (s.endsWith("^"))//retval!
				{
					bThisPtr = true;
					s.chop(1);
				}
				const RegInfo_t *pri(mrDC.fromRegName(s));
				if (!pri)
					throw(-2);
				REG_t r(pri->offs, pri->opsz);
				if (bThisPtr)
					si.cpuin.back() = r;//dups?
				else
					si.cpuin.push_back(r);
			}
		}

		if (!spoiledRegs.empty() && spoiledRegs != "*")//{*} - means a default set
		{
			//RegMaskType ff(0);
			MyStringList l(MyStringList::split(",", spoiledRegs));
			for (MyStringList::const_iterator i(l.begin()); i != l.end(); ++i)
			{
				MyString s(*i);
				bool bRetval(false);
				if (s.endsWith("!"))//retval!
				{
					bRetval = true;
					s.chop(1);
				}
				const RegInfo_t *pri(mrDC.fromRegName(s));
				if (!pri)
				{
					PrintError() << "Unrecognized register specification in .PROTO definition: `" << s << "`" << std::endl;
					return 0;//throw(-2);
				}
				REG_t r(pri->offs, pri->opsz);
				//RegMaskType f(RegMaskType(BITMASK(pri->opsz)) << pri->offs);//spoiled mask
				//ff |= f;
				if (bRetval)
					si.cpuout.push_back(r);
				si.spoiltRegs.push_back(r);
			}
		}
		
		//if (si.spoiltRegs.empty())
			//toGPRs(mrFE.spoilt_regs_def, si.spoiltRegs);

		//si.spoiledFlags &= CPUSW_MASK;
		break;
	}

	si.fpuin = si.fpuin * FTOP_STEP;
	si.fpudiff = si.fpudiff * FTOP_STEP;
	rFP = si;
	return true;
}

static uint32_t fromRegMask(const r_t *r_list, RegMaskType rmask)
{
	uint32_t ret(0);
	if (r_list)
	for (const r_t *r = r_list; r->ssid; r++)
	{
		RegMaskType rmask0(TOREGMASK(r->ofs, r->siz));
		RegMaskType rmask1(rmask0 & rmask);
		if (rmask1)
		{
			if (rmask1 != rmask0)
				return 0;//error
#if(X64_SUPPORT)
			uint32_t m(uint32_t(rmask1 >> (r->ofs >> 1)));//squeeze
#else
			uint32_t m(rmask);
#endif
			ret |= m;
		}
	}
	return ret;
}

/*MyString DcInfo_t::RegMaskToString(RegMaskType savedregs) const
{
	std::ostringstream ss;

#if(X64_SUPPORT)
	uint32_t mask(fromRegMask(mrFE.sav_regs, savedregs));
#else
	uint32_t mask(savedregs);
#endif
	if (mask != 0)
		ss << std::hex << std::setfill('0') << std::setw(sizeof(mask)* 2) << mask << std::dec;//RegMask/Type
	else
		ss << mask;
	return ss.str();
}*/

bool DcInfo_t::RegMaskToGPRs(RegMaskType rmask, GPRs_t &v) const
{
	for (const r_t *r(mrFE.spoilt_regs_check); r->ssid; r++)
	{
		RegMaskType rmask0(TOREGMASK(r->ofs, r->siz));
		RegMaskType rmask1(rmask0 & rmask);
		if (rmask1)
		{
			if (rmask1 != rmask0)
				return false;//error
			v.push_back(REG_t(r->ofs, (OpType_t)r->siz));
		}
	}
	return true;
}

bool DcInfo_t::RegMaskToArgList(RegMaskType rmask, Arg1List_t &aList) const
{
	for (const r_t *r(mrFE.spoilt_regs_check); r->ssid; r++)
	{
		RegMaskType rmask0(TOREGMASK(r->ofs, r->siz));
		RegMaskType rmask1(rmask0 & rmask);
		if (rmask1)
		{
			if (rmask1 != rmask0)
				return false;//error
			aList.push_back(Arg1_t((OPC_t)r->ssid, r->ofs, (OpType_t)r->siz));
		}
	}
	return true;
}

bool DcInfo_t::ArgListToRegMask(const Arg1List_t &l, RegMaskType &rmask0) const
{
	rmask0 = 0;
	for (Arg1List_t::const_iterator i(l.begin()); i != l.end(); ++i)
	{
		const Arg1_t &r(*i);
		if (r.opc() != OPC_CPUREG)
			return false;
		RegMaskType rmask(TOREGMASK(r.offs(), r.opsz()));
		rmask0 |= rmask;
	}
	return true;
}

bool DcInfo_t::RegStringToArgList(MyString s, Arg1List_t &l) const
{
	MyStringList ls(MyStringList::split(",", s));
	for (MyStringList::const_iterator i(ls.begin()); i != ls.end(); ++i)
	{
		const RegInfo_t *p(mrDC.fromRegName(*i));
		if (!p)
			return false;
		l.push_back(Arg1_t((OPC_t)p->opc, p->offs, (OpType_t)p->opsz));
	}
	return true;
}

bool DcInfo_t::RegStringToGPRs(MyString s, GPRs_t &v, OPC_t opc) const
{
	MyStringList ls(MyStringList::split(",", s));
	for (MyStringList::const_iterator i(ls.begin()); i != ls.end(); ++i)
	{
		const RegInfo_t *p(mrDC.fromRegName(*i));
		if (p && p->opc == opc)
			v.push_back(REG_t(p->offs, (OpType_t)p->opsz));
	}
	return true;
}

bool DcInfo_t::FlagMaskToArgList(FlagMaskType flags, Arg1List_t &l) const
{
	//FlagMaskType f(flags & CPUSW_MASK);
	return mrDC.flagsToArgList(SSID_CPUSW, flags, l);
}

bool DcInfo_t::ArgListToFlagsMask(const Arg1List_t &l, FlagMaskType &fmask) const
{
	fmask = 0;
	for (Arg1List_t::const_iterator i(l.begin()); i != l.end(); ++i)
	{
		const Arg1_t &r(*i);
		if (r.opc() != OPC_CPUSW)
			return false;
		fmask |= BITMASK(r.opsz()) << r.offs();
	}
	return true;
}

void DcInfo_t::DestroyField(FieldPtr pField) const
{
CHECKID(pField, 0x32f)
STOP
	//TypePtr iType(pField->type());
	//if (iType)
	{
		/*if (iType->type Func())
		{
		if (!Project().isClosing())
		DisconnectStub(pField->address());
		DeleteFunc(iType);
		assert(!HasMethods(iType));//?
		}
		else*/
		{
			DcInfo_t DI(mrDC, memMgrGlob());//delete from the global mem 
			DcCleaner_t<> DC(DI);
			DC.ClearType(pField);
			//SetType(pField, nullptr);
			DC.destroyUnrefedTypes();//?DC);
		}
	}
}

void DcInfo_t::InitDc()
{
	TypePtr iFrontSeg(mrDC.miFrontSeg);
	assert(iFrontSeg);
	//mrDC.miFrontSeg = iOwnerSeg;

	//	assureTypeMgr(&mrDcRef);
	//	mrDC.assureNa mespace();

	//mpOwnerSeg->assureNa mespace();
	//?	mrDC.setProject(ProjRef());
	//	projx().setDC(&mrDcRef);

	//	assert(!mrDC.mpCmdServer);//after setting up a seg oner and project
	//assert(0);
	//	mrDC.mpCmdServer = new FileInfoCmdServer_t(*this);

	//load stubs
	//createStubsMgr();
	//?	assert(mrDC.stubs().empty());
	//gui().GuiOnStubsModified();

	FileInfo_t FI(mrDC, *AddFileEx(nullptr, FTYP_PREFIX)->fileDef());

	//Assure SpecialFile(mrDC.binaryPtr(), FTYP_TYPES);
	//AssureS pecialFile(mrDC.binaryPtr(), FTYP_PROTOTYPES);

	//create typedefs
	//assert(mrDC.mTypedefs.empty());
	FI.AddTypeObj(AddTypedef(GetStockType(OPTYP_UINT8), "UNK_"));
	FI.AddTypeObj(AddTypedef(GetStockType(OPTYP_UINT8), "BYTE_"));
	FI.AddTypeObj(AddTypedef(GetStockType(OPTYP_UINT16), "WORD_"));
	FI.AddTypeObj(AddTypedef(GetStockType(OPTYP_UINT32), "DWORD_"));
	FI.AddTypeObj(AddTypedef(GetStockType(OPTYP_INT8), "INT8_"));
	FI.AddTypeObj(AddTypedef(GetStockType(OPTYP_INT16), "INT16_"));
	FI.AddTypeObj(AddTypedef(GetStockType(OPTYP_INT32), "INT32_"));
	FI.AddTypeObj(AddTypedef(GetStockType(OPTYP_INT32), "char16_t"));
	FI.AddTypeObj(AddTypedef(GetStockType(OPTYP_INT32), "char32_t"));

	//	for (std::list<TypePtr>::iterator i(mrDC.mTypedefs.begin()); i != mrDC.mTypedefs.end(); i++)
	//	pFilePrefix->AddType(*i);

	CreateIntrinsics();
	/*for (size_t i(0); i < mrDC.mIntr insics.size(); i++)
		FI.AddIntrObj(mrDC.mIntr insics[i]);*/

	//clear templated types mappings (if the module is added)
	/*	FolderPtr pFolderTempl(mrDC.folderPtr(FTYP_TEMPLATES));
		if (pFolderTempl)
		FILEDEF(pFolderTempl)->fileTempl()->clearMappings();*/
}
TypePtr DcInfo_t::GetStockTypeEx(OpType_t optyp, TypesMgr_t** pptm) const
{
	TypePtr iType(GetStockType(optyp));
	if (!iType)
	{
		assert((optyp & OPSZ_MASK) > 0);
		iType = GetStockType(OPTYP_BYTE);
		iType = ArrayOf(iType, (optyp & OPSZ_MASK));
	}
	return iType;
}

TypePtr DcInfo_t::GetStockType(OpType_t optyp, TypesMgr_t **pptm) const
{
	TypesMgr_t *ptm(mrProject.typeMgr());
	if (!ptm)
	{
		TypePtr iSeg;
#if(0)
		iSeg = PrimeSeg();
#else
		iSeg = mrDC.module();
#endif
		ptm = iSeg->typeSeg()->typeMgr();
	}
	if (pptm)
		*pptm = ptm;
	return ptm->getStockType(optyp);
}

/*bool DcInfo_t::RegainTemplatedType(TypePtr iSelf)
{
//so, typedef has been moved to this module, but it's target still is in another.
//move the target close to the typedef.

Typedef_t *pTypedef(iSelf->typeTypedef());
assert(pTypedef);

TypePtr iTypedefTarget(iSelf->absBaseType());
assert(iTypedefTarget->isUgly());

TypesMgr_t *pTypesMgr(iSelf->ownerTypeMgr());
TypesMgr_t *pTypesMgr0(iTypedefTarget->ownerTypeMgr());
assert(pTypesMgr != pTypesMgr0);

FolderPtr pFolderTempl(USERFOLDER(iTypedefTarget));
Dc_t *pDC0(DcFromFolder(*pFolderTempl));
assert(pDC0 != &DcRef());

assert(pFolderTempl == pDC0->folderPtr(FTYP_TEMPLATES));
//it doesn't contain fields or func members as assured above
FileDef_t &rFileDef0(*FILEDEF(pFolderTempl));
FileInfo_t fileInfo0(*pDC0, rFileDef0);
if (!fileInfo0.TakeType(iTypedefTarget))
ASSERT0;
rFileDef0.fileTempl()->clearMappings();//if any (or remove one?)
//and re-add it to the current module's 'templated' folder
AddTemplatedType(iTypedefTarget);
assert(iTypedefTarget->nameless());//but it can be named! fix it later
assert(iTypedefTarget->refsNum() == 2);//only typedef itself and the typesmgr
//relink from typesmgr

if (pTypesMgr0->releaseTypeRef(iTypedefTarget, true) != 0)//2 refs less
ASSERT0;
TypesTracer_t TT(*this, *pTypesMgr);
if (!TT.addTypeNew0(iTypedefTarget, nullptr))//1 ref added
ASSERT0;
iTypedefTarget->addRef();//1 more for typedef
return true;
}*/

TypePtr DcInfo_t::EnumOf(TypePtr iType, OpType_t base)
{
	assert(iType && iType->isEnum());
	TypePtr iPrimeSeg(PrimeSeg());
	TypesMgr_t *pTypesMgr(iType->ownerTypeMgr());
	assert(pTypesMgr);
	TypesTracer_t tt(*this, memMgrGlob(), *pTypesMgr);
	return tt.enumOf(iType, base);
}

TypePtr DcInfo_t::PtrOf(TypePtr iType)
{
	TypePtr iPrimeSeg(PrimeSeg());
	TypesMgr_t *pTypesMgr;
	if (!iType)
	{
		iType = GetStockType(OPTYP_NULL, &pTypesMgr);
		assert(iType && pTypesMgr == iType->ownerTypeMgr());
		iType = nullptr;
	}
	else
	{
		pTypesMgr = iType->ownerTypeMgr();
		if (!pTypesMgr)
		{
			TypesTracer_t TT(*this, memMgrGlob(), *iPrimeSeg->typeMgr());
			return TT.AcquireTypePtrOf(iType, nullptr, PtrSizeOf(iPrimeSeg));
		}
	}
	TypesTracer_t tt(*this, memMgrGlob(), *pTypesMgr);
	return tt.ptrOf(iType, PtrSizeOf(iPrimeSeg));
}

TypePtr DcInfo_t::RefOf(TypePtr iType)
{
	if (!iType)
		return nullptr;
	TypesMgr_t *pTypesMgr(iType->ownerTypeMgr());
	if (!pTypesMgr)
	{
		TypesTracer_t TT(*this, memMgrGlob(), *PrimeSeg()->typeMgr());
		return TT.refOf(iType, PtrSizeOf(PrimeSeg()));
	}

	TypesTracer_t tt(*this, memMgrGlob(), *pTypesMgr);
	return tt.refOf(iType, PtrSizeOf(iType));
}

TypePtr DcInfo_t::RvalRefOf(TypePtr iType)
{
	if (!iType)
		return nullptr;
	TypesMgr_t *pTypesMgr(iType->ownerTypeMgr());
	if (!pTypesMgr)
	{
		TypesTracer_t TT(*this, memMgrGlob(), *PrimeSeg()->typeMgr());
		//tt.addTypeNew0(iType, nullptr);
		return TT.rvalRefOf(iType, PtrSizeOf(PrimeSeg()));
	}

	TypesTracer_t tt(*this, memMgrGlob(), *pTypesMgr);
	return tt.rvalRefOf(iType, PtrSizeOf(iType));
}

/*TypePtr DcInfo_t::setTypeEx(TypeVFTable_t &rSelf, TypePtr pType) const
{
	assert(pType && !rSelf.baseType());
	if (pType)
	if (pType == rSelf.baseType())
		return rSelf.baseType();

	assert(!rSelf.baseType());
	rSelf.setBaseType(pType);
	AcquireTypeRef(rSelf.baseType());

	return rSelf.baseType();
}*/

bool DcInfo_t::MakeTypeVPtr(FieldPtr pField)
{
	assert(IsMember(pField));
	if (pField->type())
	{
		if (!pField->isTypeSimple() || pField->isTypeSimple()->typeSimple()->optype())//voids are OK
			return false;
		BinaryCleaner_t<> DC(*this);
		DC.ClearType(pField);
	}
	if (pField->offset() != 0)
		return false;

	TypesTracer_t TT(*this, memMgrGlob(), *mrModule.typeMgr());
	TypePtr pType(TT.AcquireTypeVPtr(PtrSizeOf(PrimeSeg())));

	SetType(pField, pType);
	return true;
}

TypePtr DcInfo_t::VFTableOf(TypePtr)
{
	TypePtr iType(memMgrGlob().NewTypeRef());
	Struc_t *pType(new Struc_t);
	iType->SetPvt(pType);
	iType->setTag(I_Front::SYMK_VFTABLE);
	//if (iBaseType)
		//setTypeEx(*pType, iBaseType);
	return iType;
}

TypePtr DcInfo_t::VBTableOf(TypePtr iBaseType)
{
	TypePtr iType(memMgrGlob().NewTypeRef());
	Struc_t *pType(new Struc_t);
	iType->SetPvt(pType);
	iType->setTag(I_Front::SYMK_VBTABLE);
	//if (iBaseType)
		//setTypeEx(*pType, iBaseType);
	return iType;
}

TypePtr DcInfo_t::LVFTableOf(TypePtr iBaseType)
{
	TypePtr iType(memMgrGlob().NewTypeRef());
	Struc_t *pType(new Struc_t);
	iType->setTag(I_Front::SYMK_LVFTABLE);
	iType->SetPvt(pType);
	//if (iBaseType)
		//setTypeEx(*pType, iBaseType);
	return iType;
}

TypePtr DcInfo_t::CVFTableOf(TypePtr iBaseType)
{
	TypePtr iType(memMgrGlob().NewTypeRef());
	Struc_t *pType(new Struc_t);
	iType->SetPvt(pType);
	iType->setTag(I_Front::SYMK_CVFTABLE);
	//if (iBaseType)
		//setTypeEx(*pType, iBaseType);
	return iType;
}

/*TypePtr DcInfo_t::RTTITypeDescriptorOf(TypePtr iBaseType)
{
	TypePtr iType(memMgrGlob().NewTypeRef());
	TypeRTTI_TD_t *pType(new TypeRTTI_TD_t);
	iType->SetPvt(pType);
	assert(!iBaseType);
	//setTypeEx(*pType, iBaseType);
	return iType;
}

TypePtr DcInfo_t::RTTIBaseClassDescriptorOf(TypePtr iBaseType)
{
	TypePtr iType(memMgrGlob().NewTypeRef());
	TypeRTTI_BCD_t *pType(new TypeRTTI_BCD_t);
	iType->SetPvt(pType);
	assert(!iBaseType);
	//setTypeEx(*pType, iBaseType);
	return iType;
}

TypePtr DcInfo_t::RTTIClassHierarchyDescriptorOf(TypePtr iBaseType)
{
	TypePtr iType(memMgrGlob().NewTypeRef());
	TypeRTTI_CHD_t *pType(new TypeRTTI_CHD_t);
	iType->SetPvt(pType);
	assert(!iBaseType);
	//setTypeEx(*pType, iBaseType);
	return iType;
}

TypePtr DcInfo_t::RTTIBaseClassArrayOf(TypePtr iBaseType)
{
	TypePtr iType(memMgrGlob().NewTypeRef());
	TypeRTTI_BCA_t *pType(new TypeRTTI_BCA_t);
	iType->SetPvt(pType);
	assert(!iBaseType);
	//setTypeEx(*pType, iBaseType);
	return iType;
}

TypePtr DcInfo_t::RTTICompleteObjLocatorOf(TypePtr iBaseType)
{
	TypePtr iType(memMgrGlob().NewTypeRef());
	TypeRTTI_COL_t *pType(new TypeRTTI_COL_t);
	iType->SetPvt(pType);
	assert(!iBaseType);
	//setTypeEx(*pType, iBaseType);
	return iType;
}*/

TypePtr DcInfo_t::ArrayOf(TypePtr iType, unsigned num) const
{
	assert(iType && num > 0);
	TypesMgr_t *pTypesMgr(iType->ownerTypeMgr());
	TypesTracer_t TT(*this, memMgrGlob(), *pTypesMgr);
	return TT.arrayOf(iType, num);
}

MyString DcInfo_t::VA2STR2(ADDR va) const
{
	return ProjectInfo_t::VA2STR(PrimeSeg(), va);
}

void DcInfo_t::redump(const ProbeEx_t &ctx, RedumpFlags f) const
{
	TypePtr iCont((f == REDUMP_ALL) ? nullptr : ctx.scope());
	guix().GuiInvalidateDisplays(ctx.folder(), iCont, f);
}

void DcInfo_t::RedumpStubs()
{
	guix().GuiInvalidateStubs(mrDC);
}

void DcInfo_t::redump(TypeBasePtr pCont) const
{
	RedumpFlags flags;
	FolderPtr pFolder;
	CGlobPtr pGlob(pCont->objGlob());
	if (pGlob)
	{
		pFolder = pGlob->folder();
		flags = REDUMP_SRC;
	}
	else//struclocs?!
	{
		CTypePtr pType(pCont->objType());
		pFolder = FolderOf(pType);
		flags = REDUMP_H;
	}
	guix().GuiInvalidateDisplays(pFolder, pCont, flags);
}

void DcInfo_t::redump(FolderPtr pFolder) const
{
	guix().GuiInvalidateDisplays(pFolder, nullptr, REDUMP_ALL);
}

FolderPtr DcInfo_t::AddFileEx(const char *pszName, FTYP_Enum eFileType) const
{
	FolderPtr pFolder;
	if (pszName)
	{
		assert(eFileType == FTYP_SOURCE);
		pFolder = AddFile(fixFileName(pszName, ModulePtr()));
	}
	else
		pFolder = AddSpecialFile(eFileType);
	if (pFolder)
		AssureFileDef(pFolder);
	return pFolder;
}

/*FileDef_t *DcInfo_t::TakeFieldIfNotContainedIn(userdataglobal_t &ud, const FileDef_t *pTargetFileDef)
{
	assert(ud.pSelf && pTargetFileDef);

	if (!ud.pSelf->hasUserData())
		return nullptr;

	FileDef_t *pFileDef(FILEDEF(USERFOLDER(ud.pSelf)));
	assert(pFileDef);

	if (pFileDef != pTargetFileDef)
	{
		GlobMapIt i(pFileDef->FindFieldIt(ud.pSelf));
		assert(i != pFileDef->fields().end());
		pFileDef->takeGlobIt(i, ud);
	}
	return pFileDef;
}*/

int DcInfo_t::AddTypeToFile(TypePtr pType, FolderPtr pFolder0, unsigned at) const
{
	FilesMgr0_t &rFiles(Files());
	FileDef_t* pFileDef0(pFolder0->fileDef());
	assert(pFileDef0);
	FileDef_t *pFileDef(TakeTypeIfNotContainedIn(pType, pFileDef0));
	if (pFileDef)
	{
		if (pFileDef != pFileDef0)
		{
			FileInfo_t FI(mrDC, *pFileDef0);
			FI.AddTypeObj(pType, at);
		}
		assert(USERFOLDER(pType));
		return -1;
	}

	FileInfo_t FI(mrDC, *pFileDef0);
	FI.AddTypeObj(pType, at);//append to file end
	//gui().GuiOnFileListChanged();
	return 1;
}
				
bool DcInfo_t::MoveGlobToFile(GlobPtr pGlob, FolderPtr pFolderDest) const
{
CHECKID(pGlob, 4570)
STOP
	assert(pFolderDest);
	FolderPtr pFolder(pGlob->folder());
	if (pFolder == pFolderDest)
		return false;//already in

	if (ModulePtr() != ModuleOf(pFolderDest))
		return false;

	FileDef_t* pFileDef(pFolder->fileDef());
	assert(pFileDef);

	GlobMapIt i(pFileDef->findGlobIt(pGlob));
	assert(i != pFileDef->globs().end());
	if (pFileDef->takeGlobIt(i) != pGlob)
		ASSERT0;

//CHECK(pField->address() == 0x5087E0)
//CHECKID(iGlob, 22871)
//STOP

	if (pGlob->func() && !ProtoInfo_t::IsStub(pGlob))
	{
		FileInfo_t GI(*this, *pFileDef);
		GI.RelocateFuncInnards(pGlob, *pFolderDest->fileDef());
	}

	if (!pFolderDest->fileDef()->insertGlob(pGlob))
		ASSERT0;
	return true;
}


FieldExPtr DcInfo_t::AddGlobToFile2(FieldPtr pField, FolderPtr pFolder0) const
{
	assert(pField/* && !pField->isCloneMaster()*/);
//CHECKID(pField, 5598)
CHECK(pField->_key() == 0x13eec0)
STOP
	FieldExPtr pFieldx(AsFieldEx(pField));
	//if the field already in some other file - remove it from there
	if (!pFieldx)
	{
		FileDef_t* pFileDef0(pFolder0->fileDef());
		assert(pFileDef0);

		FieldMap& m(OwnerMap(pField));//before moved
		if (pField->isExported())
			projx().exportPool().remove(pField);

		pFieldx = memMgrGlob2().NewFieldEx(std::move(*pField));//start with a var
		if (pFieldx->name())
		{
			assert(pFieldx->name()->obj() == pField);
			pFieldx->name()->setObj(pFieldx);
		}
		if (pFieldx->type() && !pFieldx->type()->isShared())
		{
			assert(pFieldx->type()->parentField() == pField);
			pFieldx->type()->setParentField(pFieldx);
		}
		m.rebind(pField, pFieldx);//adjust the map after a node substitution
		memMgrGlob().Delete(pField);

		if (pFieldx->isExported())
			projx().exportPool().add(pFieldx);

		pFileDef0->insertGlob(pFieldx);

		if (pFieldx->isTypeProc())
			AssureFuncDef(pFieldx->globPtr());
		return pFieldx;
	}

	MoveGlobToFile(GlobObj(pFieldx), pFolder0);
	return pFieldx;
}


bool DcInfo_t::MakeThisCallFromArg(FieldPtr pArg) const
{
	GlobPtr g(pArg->owner()->objGlob());
	FuncDef_t* pf(g->typeFuncDef());
	assert(pf);
	TypePtr pClass(g->owner());
	//assert(IsClassMember());
	if (!pClass)
		return false;
	if (pArg->isConstPtrToStruc(false) != pClass)
		if (pArg->isConstPtrToConstStruc(false) != pClass)
			return false;
	ADDR oldKey(pArg->_key());
	ADDR newKey(FuncInfo_s::setupKey(ARG_SSID_FROM_KEY(oldKey), ARG_OFFS_FROM_KEY(oldKey), LOCAL_ORDER_THIS));
	if (newKey != oldKey)
	{
		FuncArgsCIt cit(*pf);
		if (cit)
		{
			CFieldPtr pFirst(VALUE(cit));
			if (FuncInfo_s::order(pFirst) <= LOCAL_ORDER_THIS)//already?
				return false;
		}
		FuncArgsIt it(*pf, pArg);
		CFieldPtr pArg2(it.take());
		assert(pArg2 == pArg);
		pArg->overrideKey(newKey);
		if (!pf->argFields().insert_unique(pArg).second)
			ASSERT0;
	}
	return true;
}

bool DcInfo_t::MakeMemberThisCallFromArg(FieldPtr pArg) const//will adjust position of class declaration in file
{
	GlobPtr g(pArg->owner()->objGlob());
	assert(g->typeFuncDef());
	TypePtr iClass(pArg->isConstPtrToStruc(false));
	if (!iClass)
		iClass = pArg->isConstPtrToConstStruc(false);
	assert(iClass);
	assert(!iClass->typeProxy());
	bool bWasEmpty(IsEmptyStruc(iClass));
	TypePtr pClass(pArg->isConstPtrToStruc());
	if (pClass)
	{
		ClassInfo_t classInfo(*this, pClass);
		if (!classInfo.MakeClassMemberOf(g))
			return false;
	}
	if (bWasEmpty)
		g->folder()->fileDef()->checkForwardDeclaration(iClass);
	return MakeThisCallFromArg(pArg);
}

/*
//THIS MUST BE DONE IN ACCORD WITH CALLING CONVENTION!
bool bThisPtr(flags == AFA_THISPTR);
bool bRegister(flags == AFA_REGISTER || bThisPtr);//?

CHECKID((&mrFuncDefRef), 0x1c28)
STOP

int order(LOCAL_ORDER_STACK);// LOCAL_ORDER_UNKNOWN);
int off(mrDC.PtrSize());//stack: a room for a return ptr
SSID_t ssid(SSID_LOCAL);

if (bRegister)
{
ssid = SSID_CPUREG;
if (iType->isTypeSimple(OPTYP_REAL64))
ssid = SSID_FPUREG;
off = 0;
}

//if (!bRegister)//p-stack
if (mrDC.SS(ssid).isStacked())
{
//assert(!bThisPtr);
for (FuncArgsCRIt rit(mrFuncDef); rit; ++rit)
{
FieldPtr pLast(rit.field());
if (SSIDx(pLast) != ssid)
continue;
int siz(pLast->size());
if (siz <= 0)
siz = OPSZ_BYTE;
if (ssid == SSID_LOCAL)
off = (ALIGNED(pLast->address() + siz, mrDC.PtrSize()));//the stack args should be aligned to machine's word size?
else
off = pLast->address() + siz;
assert(off < LOCAL_ARG_RANGE);
break;
}
}
else
{
if (bThisPtr)
order = LOCAL_ORDER_THIS;
else if (bRegister)
order = LOCAL_ORDER_REG;
}

ADDR key(Field_t::setupKey(ssid, off, order));
*/

/*FieldPtr DcInfo_t::AssureThisPtrToFromType(GlobPtr g, TypePtr pThisType) const
{
	CTypePtr pClass(IsConstPtrToConstStruc(pThisType, true));
	if (!pClass)
		pClass = IsConstPtrToStruc(pThisType, true);

	if (!pClass)
		return nullptr;

	FieldPtr pThisArg(ProtoInfo_t::ThisPtrArg(g));
	if (!pThisArg)
	{
		ADDR key(Field_t::setupKey(SSID_NULL, 0, LOCAL_ORDER_THIS));
		pThisArg = memMgrGlob().NewField(0);
		if (!InsertField((GlobToTypePtr)g, pThisArg, key))
			return nullptr;
	}
	return;
}*/

FieldPtr DcInfo_t::AssureThisPtrTo(GlobPtr g, TypePtr iClass, bool bConst) const//may not yet be attached to a field
{
	if (!iClass)
		iClass = DcInfo_t::OwnerScope(g);

	FieldPtr pThisArg(ProtoInfo_t::ThisPtrArg(g));
	if (!pThisArg)
	{
		ADDR key(FuncInfo_s::setupKey(SSID_NULL, 0/*mrDC.PtrSize()*/, LOCAL_ORDER_THIS));
		pThisArg = memMgrGlob().NewField(0);
		if (!InsertFieldAt((GlobToTypePtr)g, pThisArg, key))
		{
			PrintError() << "Failed to apply `this` pointer in class method " << GlobNameFull(g, E_PRETTY, CHOP_SYMB) << std::endl;
			memMgrGlob().Delete(pThisArg);
			return nullptr;
		}
		if (bConst)
			SetConstPtrToConstStruc(pThisArg, iClass);
		else
			SetConstPtrToStruc(pThisArg, iClass);
	}
	else
	{
		CTypePtr pClass2(bConst ? pThisArg->isConstPtrToConstStruc() : pThisArg->isConstPtrToStruc());
		if (!pClass2)
		{
			if (bConst)
				SetConstPtrToConstStruc(pThisArg, iClass);
			else
				SetConstPtrToStruc(pThisArg, iClass);
		}
		else if (pClass2 != iClass)
			PrintError() << "Failed to apply `this` pointer in method with non-owner class " << GlobNameFull(g, E_PRETTY, CHOP_SYMB) << std::endl;
	}
	return pThisArg;
}

FolderPtr DcInfo_t::FindFileByStem(MyString s) const//by stem (dir+basename)
{
	if (s.startsWith("$"))
	{
		FolderPtr pVFile(nullptr);
		/*if (s == "$constants")
			pVFile = AssureSpecialFile(FTYP_CONST);
		else*/ if (s == "$stubs")
			pVFile = AssureSpecialFile(FTYP_STUBS);
		else if (s == "$exports")
			pVFile = AddFileEx(FPATH_FROM_EXPORTED);
		else if (s == "$imports")
			pVFile = AddFileEx(FPATH_FROM_IMPORTED);
		if (pVFile)
			AssureFileDef(pVFile, false);
		return pVFile;
	}

	MyString filePath(fixFileName(s, ModulePtr()));
	return Files().FindFileByStem(s);
}

bool DcInfo_t::IsConst(CFieldPtr pSelf) const
{
	if (!IsGlobal(pSelf))
		return false;
	CFieldPtr p(pSelf);
	while (p)
	{
		TypePtr iType(p->OwnerComplex());
		Seg_t* pSeg(iType->typeSeg());
		if (pSeg)
		{
			if (pSeg->isReadOnly())
			{
				assert(CheckDataAtVA(iType, pSelf->_key()));
				return true;
			}
			break;
		}
		p = iType->parentField();
	}
	assert(p);
	TypePtr iSeg(OwnerSeg(p->owner()));
	if (!IsPhantomModule(ModuleOf(p)))
		if (!CheckDataAtVA(iSeg, pSelf->_key()))
			return false;
	return pSelf->isConst();//still can be not const
}

bool DcInfo_t::IsThruConst(CFieldPtr pSelf) const
{
	if (!pSelf->isGlobal())
		return false;
	assert(ModuleOf(pSelf) == ModulePtr());//mine
	//GlobPtr pGlob(GlobObj(pSelf));
	if (GlobObj(pSelf))
		return false;
	if (!IsConst(pSelf))
		return false;
	//assert(pGlob->folder());
	return true;// pGlob->folder() == mrDC.folderPtr(FTYP_CONST);
}

GlobPtr DcInfo_t::AcquireFunction(FieldPtr pField, FolderPtr pFolder, FTYP_Enum ftyp) const
{
	if (!pFolder)
	{
		assert(ftyp > FTYP_SOURCE);
		pFolder = AssureSpecialFile(ftyp);
	}

	//assert(!pField->isCloneMaster());

	FieldExPtr pFieldx(AddGlobToFile2(pField, pFolder));
	GlobPtr pGlob(GlobObj(pFieldx));
	AssureFuncDef(pGlob);
	assert(pGlob->func());
	if (ProtoInfo_t::FuncStatus(pGlob) == 0)//never defined
	{
		//FuncInfo_t FI(*this, *pGlob);// , * pFolder->fileDef());
		//if (!IsExported(pFieldx))
		ProtoInfo_t TI(*this, pGlob);
		StubInfo_t SI(TI);
		StubBase_t stb(pFieldx, 0);
		//if (SI.CreateFuncProfile(stb, false, fp))
		const Stub_t* pStub(SI.FindStub(stb.atAddr()));
		if (pStub)
		{
			FuncProfile_t fp;
			SI.ToFuncProfile(pStub->value(), fp);
			TI.FromFuncProfile(fp);

			FileInfo_t GI(TI, *pFolder->fileDef());
			GI.FromFuncProfileEx(pGlob, fp);
			//?pStub->setField(pDockField);
		}
	}
	return pGlob;
}

TypePtr DcInfo_t::AddTypedef(TypePtr iTypeBase, const char *name, TypePtr iOwner)
{
	TypePtr iType(memMgr().NewTypeRef(new Typedef_t(nullptr)));
	Typedef_t *pType(iType->typeTypedef());
	SetType(*pType, iTypeBase);//type obj() must be valid before making this call

	/*#if(!NEW_TYPE_NAMES)
		TypesMgr_t *pTypeMgr(PrimeSeg()->typeMgr());
		#else
		TypesMgr_t *pTypeMgr(iTypeBase->ownerTypeMgr());
		#endif*/

	if (!iOwner)
		iOwner = PrimeSeg();

	TypesMgr_t *pTypeMgr(AssureTypeMgr(iOwner));

	TypesTracer_t TT(*this, *pTypeMgr);
	if (!TT.addTypeNew0(iType))
		ASSERT0;

	if (name)
	{
		//NamesMgr_t &rNS(NamespaceInitialized(iOwner));//PrimeSeg()));
		NamesMgr_t& rNS(*iOwner->typeComplex()->namesMgr());
		PNameRef pn(AddName(rNS, MyString(name), iType));
		if (!pn)
		{
			PrintError() << "Could not use typedef '" << name << "'. Name exists" << std::endl;
			DcCleaner_t<> DC(*this);
			TT.removeType(iType);
			//DC.Release TypeRef(iType);
			assert(iType->refsNum() == 0);
			//static NamesMgr_t rNS0;
			DC.destroyTypeRef(*iType, true, rNS);
			mrDC.memMgr().Delete(iType);
			return nullptr;
		}

		iType->setName(pn);
	}

	return iType;
}

GlobPtr DcInfo_t::AddIntrinsic(const char *name, const char *stub)
{
	FuncProfile_t fp;
	//InitFuncProfile(fp);
	if (!ToFuncProfile(stub, fp) || fp.isThisCall())//intrinsic cannot be a class member
	{
		PrintError() << "Invalid intrinsic's stub format: " << name << " (" << stub << ")" << std::endl;
		return nullptr;
	}

	//no need to add it to types manager
	//	TypesTracer_t tt(*typeMgr());
	MemoryMgr_t *pMemMgr(&memMgr());
	GlobPtr g(memMgrGlob2().NewIntrinsic());
	g->flags() |= FDEF_INTRINSIC;
	FuncDef_t *pfDef(g->typeFuncDef());
	AssureNamespace((GlobToTypePtr)g);//must have a namespace

	//Files().addObjectToFile2(tid);
	ProtoInfo_t TI(*this, g);
	TI.FromFuncProfile(fp);

	mrDC.mIntrinsics.push_back(g);
	//g->addRef();
	//NamesMgr_t &rNS(NamespaceInitialized(PrimeSeg()));
	NamesMgr_t& rNS(*PrimeSeg()->typeComplex()->namesMgr());
	g->typeFuncDef()->setName(AddName(rNS, name, g));// , memMgr()));
	//g->typeFuncDef()->setName(pNS->insertStockName(name));
	
	return g;
}

int DcInfo_t::CreateIntrinsics()
{
	int count(0);
#if(1)
	//assert(mrDC.mInt rinsics.size() == 1);
	//mrDC.mIntr insics.push_back(nullptr);//first element is NUL
	for (const FE_t::stub_t *p(mrFE.fdefs); p && p->key; p++)
	{
		if (AddIntrinsic(p->key, p->value))
			count++;
	}

#if(0)
	AddIntrinsic("movs", "0 0 0 0 2780 00ffffff 0 0ffff 0");
	AddIntrinsic("sqrt", "0 0 1 0 0 ffffffff 0 0ffff");
	AddIntrinsic("cos", "0 0 1 0 0 ffffffff 0 0ffff");
	AddIntrinsic("sin", "0 0 1 0 0 ffffffff 0 0ffff");
	AddIntrinsic("atan2", "0 0 2 1 0 ffffffff 0 0ffff");
#endif
#endif
	return count;
}


const STORAGE_t &DcInfo_t::Storage(SSID_t ssid) const
{
	return mrFE.STORAGE[ssid];
}

bool DcInfo_t::IsDefinitionFile(CFolderPtr pFolder) const
{
	if (mrDC.isFolderOfKind(*pFolder, FTYP_PREFIX))
		return false;
	//if (mrDC.isFolderOfKind(*pFolder, FPATH_FROM_IMPORTED))//header only files
	if (FolderPath(pFolder) == FPATH_FROM_IMPORTED)
		return false;
	Folder_t *pTopFolder(TopFolder(*pFolder));
//!	if (pDC->isFolderOfKind(rFolder, FPATH_FROM_EXPORTED) && IsPhantomFolder(*pTopFolder))//header only files
//!		return true;
	/*	if (pDC->isFolderOfKind(rFolder, FTYP_TEMPLATES))
			return true;*/
	assert(pFolder->fileDef());
	if (!pFolder->fileDef()->hasGlobs())
		return false;
	return true;
}

bool DcInfo_t::IsDeclarationFile(CFolderPtr pFolder) const
{
	/*if (mrDC.isFolderOfKind(*pFolder, FTYP_CONST))
		return false;*/
	return true;
}

/*FileTempl_t *DcInfo_t::CheckTemplatesMappings(bool bRebuild) const
{
FolderPtr pFolder1(mrDC.folderPtr(FTYP_TEMPLATES));
FileTempl_t *pFileTempl(FILEDEF(pFolder1)->fileTempl());
if (pFileTempl->hasMappings())
{
if (!bRebuild)
return pFileTempl;
pFileTempl->clearMappings();
}

FileTree_t::Iterator i(ModuleFolderPtr());
i++;//skip the root
for (; i; i++)
{
FolderPtr pFolder(i.top());
if (pFolder == pFolder1)
continue;
FileDef_t *pFileDef(FILEDEF(pFolder));
if (!pFileDef)
continue;
for (TypesListCIt i(pFileDef->types().begin()); i != pFileDef->types().end(); i++)
{
TypePtr iType(*i);
if (iType->typeProxy())
continue;
Typedef_t *pTypeDef(iType->typeTypedef());
if (pTypeDef && pTypeDef->baseType()->isUgly())
pFileTempl->addMapping(pTypeDef->baseType(), iType);
}
}
return pFileTempl;
}*/


/*bool DcInfo_t::FieldCompUglyName0(FieldPtr p, const char *pc) const
{
	assert(p->hasUg lyName());
	PNameRef pn(FindPrettyName(p));
	assert(pn);
	return strcmp(pn->c_str(), pc) == 0;
}*/

static MyString checkedStr(const char* pc, int n)//if n<0 - look for eos character
{
	if (n < 0)
		return pc;
	return MyString(pc, n);
}

//match a symbol (mangled/extracted) against existing global field to see if it is the same
bool DcInfo_t::FieldCompName0(CFieldPtr p, const MyString& mangled, const MyString& extracted, node_t* pn0) const
{
//CHECKID(p, 3012)
CHECK(extracted == "DrawState")
STOP

	MyString name0;//field's and scope
	FullName_t scope0;

	int nameLen;
	const char* pc(p->safeName(nameLen));//maybe mangled as well
	if (pc)
	{
		ChopName(checkedStr(pc, nameLen), name0);//this is an existing one - needs chopping
		assert(!name0.isEmpty());

		if (!mangled.isEmpty())
		{
			if (name0 == mangled)
				return true;
			//the global's field name could be 1) a mangled symbol, or extracted one, set previously by FE
			if (projx().cxxSymbols().findSymbol(name0))
				return false;//no match
		}
		else
		{
			assert(!extracted.isEmpty());
			node_t* pn(FromSymbol(name0));
			if (pn && pn->name == extracted)
				return true;
		}
	}

	CGlobPtr pGlob(GlobObj(p));
	TypePtr pScope0(pGlob ? OwnerScope(pGlob) : nullptr);

	if (pScope0)
	{
		if (pScope0->name())
		{
			scope0 = TypeNameFull(pScope0);
			if (pGlob)
			{
				PNameRef pn0(pGlob->func() ? pGlob->func()->name() : nullptr);
				if (pGlob->hasPrettyName())
				{
					PNameRef pn(FindPrettyName(pGlob));
					assert(pn);
					ChopName(pn->c_str(), name0);
				}
				//else if (ProtoInfo_t::FuncStatus(pGlob) == 0)
					//return p;//__noproto - good to go
			}
		}
	}
	else if (name0.isEmpty())
	{
		if (pc)
			name0 = pc;
	}

	if (!extracted.isEmpty())
	{
		MyString name;
		MyString scope(NameScopeChopped(extracted, name, false));
		//TypePtr pScope(scope.isEmpty() ? nullptr : FindTypeByName(scope));
//CHECK(name == "Unlock" && scope == "CMultiLock")
//STOP
		MyString name2(EnhancedName(name, scope));//enhanced name
		if (name2.isEmpty())
			return false;
		if (name0.isEmpty() || name0 == name2)
			if (scope0.empty() || scope0.join() == scope)
			{
				//if (!p->name() || mangled.isEmpty())
					return true;//the C++ symbol in global scope, extracted from demangled string (or user definition)
			}
	}
	return false;
}

//while recreating an object from symbols, if the one already exists at given location, we have to determine if we're dealing with the duplicate
FieldPtr DcInfo_t::MatchExistingField(FieldPtr pField0, const MyString& sMangled, const MyString& sExtracted, node_t *pn)
{
CHECK(pField0->_key() == 0x5f4918e0)
STOP
	/*if (pField0->isCloneMaster())
	{
		TypePtr pSeg(OwnerSeg(pField0->owner()));
		ADDR va(pField0->_key());
		const ConflictFieldMap& m(pSeg->typeSeg()->conflictFields());
		for (ClonedFieldMapCIt it(m.lower_bound(va)); it != m.end() && KEY(it) == va; ++it)
		{
			CFieldPtr pField(VALUE(it));
			if (FieldCompName0(pField, sMangled, sExtracted, pn))
				return (FieldPtr)pField;//a clone was matched
		}
	}
	else*/
	{

		TypePtr pSeg(OwnerSeg(pField0->owner()));
		ADDR va(pField0->_key());
		const FieldMap& m(pSeg->typeStruc()->fields());
		for (FieldMapCIt it(m.lower_bound(va)); it != m.end() && KEY(it) == va; ++it)
		{
			CFieldPtr pField(VALUE(it));
			if (FieldCompName0(pField, sMangled, sExtracted, pn))
				return (FieldPtr)pField;//was matched
		}
	}
	return nullptr;
}

TypePtr DcInfo_t::MakeProxyTypeTo(TypePtr pIncumbent) const
{
	assert(!pIncumbent->typeProxy());
	assert(pIncumbent->typeTypedef() || pIncumbent->typeStruc());
	TypePtr iModule2(ModuleOf(pIncumbent));//other
	assert(iModule2 != ModulePtr());//different modules

CHECKID(pIncumbent, 59707)
STOP

	TypesMgr_t& rTypesMgr(*PrimeSeg()->typeMgr());//PrimeSeg()
	TypePtr iProxy(rTypesMgr.findProxyOf(pIncumbent));
	if (iProxy)
		return iProxy;

	TypePtr iNewOwner(rTypesMgr.owner());

	TypesTracer_t TT(*this, rTypesMgr);
	TypeProxy_t* pTypeProxy(new TypeProxy_t());
	iProxy = TT.memMgr().NewTypeRef(pTypeProxy);
	SetType(*pTypeProxy, pIncumbent);
	ON_OBJ_CREATED(iProxy);

#ifdef _DEBUG
	//__catch_struc_id(iProxy);
#endif
	//iProxy->m_nFlags |= TYPEOBJ_ID_OVERIDE;

	//the original type must be removed
	TypePtr pIncumbent0(pIncumbent);//can be a typedef (to ugly)
//	assert(SkipModifier(pIncumbent) == pIncumbent);

	/*	if (iOwnerClass0)
		{
		//assert(pIncumbent->owner() == iOwnerClass0);
		//if (!pIncumbent->ownerTypeMgr()->removeNameAlias(pIncumbent))
		//ASSERT0;
		}
		else
		iOwnerClass0 = pIncumbent0->owner();*/

	if (!TT.addTypeNew0(iProxy))
		ASSERT0;

	return iProxy;
}

bool DcInfo_t::RenameProxyType(TypePtr pTypePxy, const MyString& s0) const
{
	assert(pTypePxy->typeProxy());
	TypePtr pIncumb(pTypePxy->typeProxy()->incumbent());

	if (pIncumb->isNested())//clone the name in current module (for top level incumbents only!)
		return false;

	MyString s(s0);
	if (s.isEmpty())
	{
		TypePtr iModule2(ModuleOf(pIncumb));
		DcInfo_t DC2(*DCREF(iModule2));
		PNameRef pn0(DC2.TypePureName(pIncumb));
		if (pn0 && (!pIncumb->hasUglyName() || pn0 != pIncumb->name()))//assure no ugly name assigned to a proxy
		{
			unsigned n(ChopName(pn0->c_str(), s));
		}
	}
	//assert(!s.empty());
	int forceMode(pIncumb->hasUglyName() ? 1 : 0);
	assert(IsMemMgrGlob());
	NamesMgr_t& rNS1(OwnerNamespaceEx(PrimeSeg()));// rTypesMgr.owner()));
	if (!ForceName(rNS1, s, pTypePxy, forceMode))
		ASSERT0;

	return true;
}

bool DcInfo_t::RenameField(FieldPtr pField, MyString sName) const
{
	assert(IsGlobal(pField));
CHECKID(pField, 0xb92)
STOP

	assert(!sName.isEmpty());

	if (!pField->name())
	{
		if (sName.empty())
			return true;
	}
	else
	{
		if (pField->name()->c_str() == sName)
			return true;

		if (pField->isExported())
			if (!projx().exportPool().remove(pField))
				ASSERT0;

		if (!ClearFieldName(pField))
			ASSERT0;
	}
	
	SetFieldName2(pField, sName, true);
	if (!pField->nameless())
	{
		pField->setHardNamed(true);
		if (pField->isExported())
			if (!projx().exportPool().add(pField))
				ASSERT0;
	}

	return true;
}

PNameRef DcInfo_t::TypePureName(CTypePtr iSelf) const
{
	if (!iSelf->typeProxy())
	{
		if (iSelf->hasUglyName())
		{
			PNameRef pn(FindPrettyName(iSelf));
			if (pn)
				return pn;
		}
	}
	return iSelf->name();
}

MyString DcInfo_t::TypePrettyName(CTypePtr pType) const
{
	return TypePrettyName(pType, DUB_SEPARATOR);//do not chop!
}

MyString DcInfo_t::TypePrettyName2(CTypePtr pType) const
{
	TypePtr iModule(ModuleInfo_t::ModuleOf(pType));
	if (!iModule)//is it non-sharing func ptr?
	{
		if (pType->typeFuncDef())
		{
			assert(DockField((CGlobPtr)pType));
			iModule = ModuleOfEx((CGlobPtr)pType);
		}
	}

	if (iModule)
	{
		TypePtr pProxy(nullptr);
		if (iModule != ModulePtr())//still may be in another module
		{
			pProxy = FindProxyOf(pType);//look for a proxy in current module
			if (pProxy)
				return TypePrettyName(pProxy, DUB_SEPARATOR);
			return StrucNameless(pType);
		}
	}

	return TypePrettyName(pType, DUB_SEPARATOR);//do not chop!
}

MyString DcInfo_t::TypePrettyName(CTypePtr pType, char chopSymb) const
{
	return TypeName(pType, chopSymb);
}

MyString DcInfo_t::GlobPrettyName0(CGlobPtr pGlob, char chopSymb) const
{
	//assert(IsGlobal(pField));
	//CGlobPtr pGlob(GlobObj(pField));
	assert(pGlob && pGlob->hasPrettyName());
	PNameRef pn(FindPrettyName(pGlob));
	assert(pn);
	MyString s;
	ProjectInfo_t::ChopName(pn->c_str(), s, chopSymb);
	return s;
}

/*void DcInfo_t::_TypeNameScopedUtil(TypePtr pSelf, TypePtr pScopeFrom, char chopSymb, MyString &s)
{
	if (pScopeFrom)
	{
		TypePtr pOwner(pSelf->ownerScope());
		if (pOwner && pOwner != pScopeFrom)
		{
			_TypeNameScopedUtil(pOwner, pScopeFrom, chopSymb, s);
			s.append("::");
		}
	}
	s.append(TypeName0(pSelf));
}

MyString DcInfo_t::TypeNameScoped(TypePtr pSelf, TypePtr pScope, char chopSymb)
{
	pScope = ProjectInfo_t::CommonScope(pScope, pSelf);
	MyString s;
	_TypeNameScopedUtil(pSelf, pScope, chopSymb, s);
	return s;
}*/

FullName_t DcInfo_t::TypeNameFull(CTypeBasePtr p, TypeNameFullMode ePrettyMode, char chopSymb) const//full name as is - no pretty names, nor suffixes
{
	assert(p && !p->objGlob());//for globs use GlobNameFull!

	if (p->typeProxy())
	{
		TypePtr pIncumb(p->typeProxy()->incumbent());
		TypePtr iModule(ModuleOf(pIncumb));
		assert(iModule && iModule != ModulePtr());
		DcInfo_t DI2(*DCREF(iModule));
		return DI2.TypeNameFull(pIncumb, ePrettyMode, chopSymb);
	}

	if (p->typeEnum())
		p = p->baseType();

	FullName_t s;
	switch (ePrettyMode)
	{
	default:
		mrProject.typeNameScoped(p->owner(), chopSymb, s);
		s.append(TypeName0((TypePtr)p));//no pretty names!
		break;
	case E_PRETTY_NAME://pretty name only
		mrProject.typeNameScoped(p->owner(), chopSymb, s);
		s.append(TypeName((TypePtr)p));//pretty name
		break;
	case E_PRETTY_SCOPE://pretty scope only
		_TypePrettyScope(p->owner(), chopSymb, s);
		s.append(TypeName0((TypePtr)p));//no pretty name)
		break;
	case E_PRETTY://everything is pretty
		_TypePrettyScope(p->owner(), chopSymb, s);
		s.append(TypeName((TypePtr)p, chopSymb));
		break;
	}
	return std::move(s);
}

bool DcInfo_t::GetPrettyName(CTypePtr pSelf, MyString &s, char chopSymb)
{
	PNameRef pn(nullptr);
	if (pSelf->hasPrettyName())
		pn = FindPrettyName(pSelf);
	if (!pn)
		pn = pSelf->name();
	if (!pn)
		return false;
	ChopName(pn->c_str(), s, chopSymb);
	return true;
}

bool DcInfo_t::GetPrettyName(CGlobPtr pSelf, MyString &s, char chopSymb)
{
	PNameRef pn(nullptr);
	if (pSelf->hasPrettyName())
		pn = FindPrettyName(pSelf);
	if (!pn)
		pn = DockName(pSelf);
	if (!pn)
		return false;
	ChopName(pn->c_str(), s, chopSymb);
	return true;
}

void DcInfo_t::_TypePrettyScope(CTypePtr pSelf, char chopSymb, FullName_t &v) const
{
	if (pSelf && !pSelf->typeSeg())
	{
		assert(pSelf->typeStruc() && (!pSelf->nameless() || pSelf->hasUglyName()));
		_TypePrettyScope(pSelf->owner(), chopSymb, v);
		v.append(TypeName(pSelf, chopSymb));//no pretty names!
		//v.append("::");
	}
}

FullName_t DcInfo_t::TypePrettyNameFull(CTypePtr p, char chopSymb) const
{
	assert(p);

	CTypePtr iModule(ModuleOf(p));
	if (iModule && iModule != ModulePtr())
	{
		DcInfo_t DI(*DCREF(iModule));
		return DI.TypePrettyNameFull(p, chopSymb);
	}

	FullName_t v;
	_TypePrettyScope(p->owner(), chopSymb, v);
	v.append(TypeName(p, chopSymb));//chop
	return v;
}

FullName_t DcInfo_t::GlobNameFull(CGlobPtr g, TypeNameFullMode prettyMode, char chopSymb) const
{
	CFieldPtr p(DockField(g));
	assert(p);

	MyString s;
	if (prettyMode == E_PRETTY)
		ChopName(mrProject.fieldName(p), s, chopSymb);//later
	else
		ChopName(mrProject.fieldName(p), s, chopSymb);

	FullName_t aName(s);

	TypePtr iOwner(OwnerScope(g));
	if (iOwner)
	{
		while (!iOwner->typeSeg())
		{
			//assert(iOwner->typeStruc() && (!iOwner->nameless() || iOwner->hasUg lyName()));

			MyString s2;
			ChopName(TypeName(iOwner, chopSymb), s2, chopSymb);
			aName.prepend(s2);
			iOwner = iOwner->owner();
		}
	}

	return std::move(aName);
}

/*MyString DcInfo_t::TypeNameFull0(TypePtr p) const
{
	if (p->typeStruc())
	{
		//make fully qualified name
		if (p->typeProxy())
		{
			assert(0);
			p = SkipProxy(p);
			TypePtr iOther(ModuleOf(p));
			DcInfo_t DI(*DCREF(iOther));
			return DI.TypeNameFull(p);
		}
	}

	return TypeNameFull(p);
}*/


FieldPtr DcInfo_t::AssureFieldAt(Locus_t& aLoc, const MyString &sMangled, node_t *pn)//name only
{
	TypePtr iSeg2(aLoc.seg());
	ADDR va(aLoc.addr());
	if (!aLoc.field0())
	{
		if (InsertField(aLoc) && !sMangled.empty())
		{
			if (SetFieldName2(aLoc.field0(), sMangled, true))
				aLoc.field0()->setHardNamed(true);
		}
		return aLoc.field0();
	}

	FieldPtr pField0(aLoc.field0());//lead
//CHECKID(pField0, 0x17006)
CHECK(pField0->_key() == 0x5f4a67d0)
STOP
	FieldPtr pField(MatchExistingField(pField0, sMangled, pn ? pn->name : "", pn));
	if (pField)//may be a clone
	{
		MyString oldName;
		GlobPtr pGlob(GlobObj(pField));
		if (!sMangled.isEmpty())
		{
			MyStringEx aName(ToCompoundName(pField->name()));
			if (!aName.empty(0))
			{
				oldName = aName[0];
				if (sMangled == oldName)
					return nullptr;
			}
			aName.set(0, sMangled);
			RenameField(pField, FromCompoundName(aName));
			if (!oldName.isEmpty() && pn)
			{
				assert(pGlob);
				ApplyPrettyName(pGlob, pn->name, pGlob->ownerScope(), 2);
			}
		}
/*		if (pGlob)
			if (ProtoInfo_t::FuncStatus(pGlob) != 0)//was deffined before
				return nullptr;*/
		aLoc.setField(pField);
		return pField;
	}
	
	//if (pField0->nameless() /*&& !pField0->isCloneMaster()*/ && !(GlobObj(pField0) && GlobObj(pField0)->hasPrettyName()))
	if (pField0->nameless() && !pField0->hasUglyName())
	{
		NamesMgr_t &rNS(OwnerNamespaceEx(iSeg2));
		AddName(rNS, sMangled, pField0);
		return pField0;
	}


	pField = AddSecondaryField(iSeg2, va, nullptr);
	if (!pField)
		return nullptr;
	if (!sMangled.isEmpty())
		if (SetFieldName2(pField, sMangled, true))
			pField->setHardNamed(true);
	aLoc.setField(pField);
	//the locus now holds a ref to a secondary field

	return aLoc.field0();
}

TypePtr DcInfo_t::FindProxyOf(CTypePtr p) const
{
	TypesMgr_t *ptm(PrimeSeg()->typeSeg()->typeMgr());
	return ptm->findProxyOf(p);
}

/*bool Project_t::ADDR2ROWID(ADDR addr, int level, ROWID &rowID)
{
if (level == 0)
{
rowID = addr;
return R2D(addr, rowID);
}
if (level == 1)
return V2D(addr, rowID, level);
return false;
}*/

FieldPtr DcInfo_t::FindGlobal(ADDR addr) const
{
	return FindGlobalAtVA(addr, FieldIt_Exact, true);
}

FieldPtr DcInfo_t::FindGlobalAtVA(ADDR addr, FieldIt_Mode mode, bool bDeep) const
{
	const Seg_t &rSeg0(*PrimeSeg()->typeSeg());
	TypePtr iSeg(rSeg0.findSubseg(addr, rSeg0.affinity()));
	if (iSeg)
		return __findFieldV(iSeg, addr, mode, bDeep);
	return nullptr;
}

ObjPtr DcInfo_t::FindObjByAutoNameEx(const MyString &s, CTypePtr iScope) const
{
	value_t v;
	int kind(mrProject.checkAutoPrefix(s, &v));
	if (kind == 0)
		return nullptr;
	//autoname detected!
#ifdef _DEBUG
	if (kind == 1 || kind == -1)//a type
#else
	if (kind == 1)//a type
#endif
	{
		int compId(v.i32);
#ifdef _DEBUG
		if (kind < 0)
			compId = -compId;
#endif
		const TypesMgr_t *ptm(iScope->typeStruc()->typeMgr());
		if (ptm)
			return ptm->findTypeByCompId(compId);
	}
	if (kind == 2 || kind == 4)//a field (glob)
	{
		CFieldPtr pField(FindGlobalAtVA(v.ui32, FieldIt_Exact, false));
		if (pField)
		{
			//if (!pField->isCloneMaster())
			{
				if (kind == 4)//a glob
					return (ObjPtr)GlobObj(pField);
				return (ObjPtr)pField;
			}
			assert(0);
		}
	}
	return nullptr;
}

void unChopName(MyString &s)
{
	for (MyString::reverse_iterator i(s.rbegin()); i != s.rend(); ++i)
	{
		char &chr(*i);
		if (chr == CHOP_SYMB)
		{
			chr = DUB_SEPARATOR;
			s += DUB_TERMINATOR;
			break;
		}
		if (!isdigit(chr))
			break;
	}
}

ObjPtr DcInfo_t::FindObjByScopedNameEx(const MyString &s0, CTypePtr iScopeFrom) const
{
	CTypePtr iScope(iScopeFrom);
	MyStringList l(MyStringList::split("::", s0));
	assert(!l.empty());
	for (;;)
	{
		MyString s(l.front());
		unChopName(s);
		l.pop_front();
		ObjPtr pObj;
		if ((pObj = FindObjByAutoNameEx(s, iScope)) == nullptr)
			pObj = FindObjByName(s, iScope);
		if (!pObj)
			break;
		if (l.empty())
			return pObj;
		iScope = pObj->objTypeGlob();
		if (!iScope)
			break;
	}
	return nullptr;
}

/*void DcInfo_t::RebuildImportInfoMap() const
{
	Module_t &aBin(mrDC.moduleRef());
	TypePtr iFrontSeg(PrimeSeg());
	//Seg_t &rFrontSeg(*iFrontSeg->typeSeg());

	MyString sModule;
	Folder_t *pExportingModule(nullptr);

	SymbolMap symb(GetDataSource()->pvt(), IFrontOf(iFrontSeg));
	symb.dumpImported();
	for (SymbolMap::Iterator symIt(symb); symIt; symIt++)//IMPORTS
	{
		MyString s(symIt.symbol());
		if (symIt.module() != sModule)
		{
			sModule = symIt.module();
			pExportingModule = FindModuleFolder(sModule);
		}
		if (!pExportingModule)
			continue;
		if (symIt.symbol().empty())//named only yet
		{
			STOP//continue;
		}
		Locus_t aLoc;
		ADDR va(symIt.va());
		FieldPtr pImpField(FindFieldInSubsegs(iFrontSeg, va, aLoc));
		if (pImpField)
		{
			assert(aLoc.field() == pImpField);
			unsigned off;
			if ((pImpField = aLoc.stripToSeg(off)) != nullptr && off == 0)
			{
				//assert(IsTypeImp(pImpField) && !pImpField->nameless());
				TypePtr pExportModule(ModuleOf(pExportingModule));
				mrDC.importMap().add(pExportModule, symIt.ordinal(), pImpField->address());
			}
		}
	}
}*/

FieldPtr DcInfo_t::FromExportedField(CFieldPtr pExtField) const
{
	assert(pExtField->isExported());

	TypePtr pSeg(PrimeSeg());
	NamesMgr_t* pnm(pSeg->typeSeg()->namesMgr());
	PNameRef pn(pExtField->name());
	
	assert(pn);
	const char* pc0(pn->c_str());
	ObjPtr pObj(pnm->findObjEx(pc0));
	if (pObj)
	{
		FieldPtr pField(pObj->objField());
		assert(pField);
		return pField;
	}

	return nullptr;
}


FieldPtr DcInfo_t::ToExportedField(CFieldPtr pImpField) const
{
	return projx().toExportedField(pImpField);
}

#if(0)
FieldPtr DcInfo_t::ToExportedField2(CTypePtr pExpModule, unsigned short uOrd, const char *pName) const
{
	//using a frontend of exporting module, retrieve an exported entry's va through name or ordinal
	Module_t &aBin2(*pExpModule->typeModule());
	const Dc_t *pDC2(DCREF(pExpModule));
	TypePtr iPrimeSeg2(pDC2->primeSeg());
	DcInfo_t DI2(*pDC2);
		
	FieldPtr pExpField(nullptr);
	if (!IsPhantomModule(pExpModule))
	{
		MyString sName;
		if (pName)
			ChopName(pName, sName);
		I_Front *pIFront2(IFrontOf(iPrimeSeg2));

		SymbolInfo symb;
		DumpSymbol_t symDump(symb);
		if (pIFront2->getExportInfo(sName.isEmpty() ? nullptr : sName, uOrd, &symDump))
		{
			Locus_t aLoc;
			FindFieldInSubsegs(iPrimeSeg2, symb.va, aLoc);
			if (aLoc.stripToUserField(sName.isEmpty() ? nullptr : sName))
			{
				if (aLoc.addr() == symb.va)
					pExpField = aLoc.field();
				/*unsigned offs;
				pExpField = stripToSeg(aLoc, offs);
				assert(offs == 0);*/
			}

			if (pExpField && !pExpField->isExported())
			{
				assert(pExpField->isCloneMaster());
				if (!sName.isEmpty())
				{
					pExpField = DI2.FindFieldByName(sName, DI2.PrimeSeg());
					//Seg_t &rPrimeSeg2(*iPrimeSeg2->typeSeg());
					//NamesMgr_t &rNS(*rPrimeSeg2.namesMgr());
					//PNameRef pNameRef(pName);
					//Obj_t *pObj(rNS.findObj(pNameRef));
					//pExpField = pObj->objField();
				}
				else
				{
					assert(0);
					//MyStringEx aName(symb);
					//pExpField = pDC2->exportMap().lookup(symb.uOrdinal);
				}
				assert(pExpField);
			}
		}
		else
		{
			STOP
		}
	}
	else
	{
CHECK(uOrd==0xe3)
STOP
		//always try the name first
		if (pName)//impport by name
		{
			MyString s;
			ProjectInfo_t::ChopName(pName, s);
			pExpField = DI2.FindFieldByName(s, DI2.PrimeSeg());
		}
		if (!pExpField && uOrd != ORDINAL_NULL)
		{
			ADDR va(ORDINAL_BIAS + uOrd);
			pExpField = Field(iPrimeSeg2, va, nullptr, FieldIt_Exact);
			if (!pExpField)
			{
				assert(va == ORDINAL_BIAS);
			}
			else if (pExpField->isCloneMaster())
			{
				assert(!pExpField->hasUserData());
				ConflictFieldMap &m2(iPrimeSeg2->typeSeg()->conflictFields());
				for (ClonedFieldMapCIt i(m2.lower_bound(va)); i != m2.end(); i++)
				{
					if (i->first != va)
						break;
					pExpField = i->second;
					if (pExpField->nameless())
						break;//prefer a first nameless exported field as long as the imported one nameless as well
				}

			}
		}
	}

	return pExpField;
}
#endif

void DcInfo_t::AddExportedFieldInfo(ExpFieldInfo_t exp) const
{assert(0);
#if(0)
	assert(IsExported(exp.pField) && !exp.pField->nameless());
	ExpFieldsMap &m(projx().mExpFieldsMap);
	ExpFieldsMapIt i(m.insert(std::make_pair(exp.pField->name(), exp)));
#endif
/*	if (!ret.second)
	{
		//export symbols may refer to the same shared location - and those should have already been registered
		ExpFieldInfo_t &exp2(ret.first->second);
		assert(exp2.pField == exp.pField);
		assert(!exp2.pFolder || exp2.pFolder == exp.pFolder);
		if (!exp2.pFolder)
			exp2.pFolder = exp.pFolder;
	}*/
}

ExpFieldInfo_t DcInfo_t::RemoveExportedFieldInfo(PNameRef pName) const
{assert(0);
#if(0)
	ExpFieldsMap &m(projx().mExpFieldsMap);
	ExpFieldsMapCIt i(m.find(pName));
	if (i != m.end())
		return i->second;
#endif
	/*for (ExpFieldsMapIt i(m.lower_bound(pName)); (i != m.end() && i->first == pName); i++)
	{
		ExpFieldInfo_t exp(i->second);
		if (exp.pField == pField)
		{
			m.erase(i);
			return exp;
		}
	}*/
	return ExpFieldInfo_t();
}

bool DcInfo_t::TmpLabelNameEx(CFieldPtr pSelf, MyString &s) const
{
	const PathPtr pPath(FuncInfo_t::LabelPath(pSelf));
	if (pPath)
	{
		if (pPath->Type() == BLK_EXIT)
			s = "$exit";
		else
		{
			//s = MyStringf("$loc_%d", FuncInfo_t::PathNo(pPath));
			s = "$loc_???";//LATER
		}
		return true;
	}
	return TmpLabelName(pSelf, s);
}

TypePtr DcInfo_t::OwnerSegEx(CTypePtr iType) const
{
	CGlobPtr pGlob(iType->objGlob());
	if (pGlob)
	{
		FieldPtr pField(FieldEx_t::dockField(pGlob));//can be taken
		return OwnerSeg(pField->owner());
	}
	CTypePtr iSeg(OwnerSeg(iType));
	if (!iSeg)
		iSeg = mrDC.primeSeg();
	return (TypePtr)iSeg;
}

#if(0)
bool DcInfo_t::LabelNameEx(CFieldPtr pSelf, MyString &s) const
{
	if (!pSelf->owner())//temp fields are expected to be named
		return TmpLabelNameEx(pSelf, s);//name is taken from type (funcend)

	TypePtr iFunc(pSelf->ownerProc());
	if (!iFunc)
		return false;

	CGlobPtr ifDef(GlobObj(iFunc->parentField()));
	if (!ifDef || ifDef->func())
		return false;

	if (!IsLocal(pSelf))//?
		return false;
	ASSERT0;

//	if (!FuncInfo_t::IsLab el(pSelf))
	//	return false;

/*?	const PathPtr pPath(FuncInfo_t::Labe lPath(pSelf));
	if (pPath)
	{
		const char * pfx = nullptr;
		if (pPath->Type() == BLK_EXIT)
			pfx = "$EXIT_";
		if (pPath->Type() == BLK_ENTER)
			pfx = "$ENTER_";
		if (pfx)
		{
			assert(0);
			/ *?Fu nc_t *pFunc = pPath->m.ownerProcPvt();
			strcpy(buf, pfx);
			char buf2[NAMELENMAX];
			pFunc->nam ex(buf2);
			buf2[NAMELENMAX - strlen(pfx) - 1] = 0;
			strcat(buf, buf2);
			return (int)strlen(buf);* /
		}
	}*/

	ADDR offs(pSelf->_key());
	if (offs != BAD_ADDR && offs != 0)
	{
		s = MyStringf("loc_%X", offs);
		return true;
	}

	/*const PathOpList_t &l(FuncInfo_t::fieldOps(pSelf));
	if (!l.empty())
	{
		assert(0);
		//FileDef_t &rFileDef(DcInfo_t::FindFileDefOf(*ifDef->typeFuncDef()));
		//FuncInfo_t FI(mrDC, *ifDef, rFileDef);
		s = MyStringf("$loc_%d", FuncInfo_t::No(PRIME(l.front())));
	}
	else*/
		s = MyString("$loc_?");
	return true;
}
#endif

bool DcInfo_t::GetDisplayName(CFieldPtr pSelf, MyString &s) const
{
	CGlobPtr iGlob(IsGlobal(pSelf) ? GlobObj(pSelf) : nullptr);
	if (!iGlob)
	{
		if (!pSelf->name())
			return false;
		s = pSelf->name()->c_str();
		return true;
	}
	PNameRef pn(nullptr);
	if (iGlob->func())
	{
		if (iGlob->hasUglyName())
			pn = FindPrettyName(iGlob);
		else
			pn = iGlob->name();
		//check the field itself
	}
	assert(!pn);
	{
		if (iGlob->hasPrettyName())
		{
			if ((pn = FindPrettyName(iGlob)) != nullptr)
			{
				s = pn->c_str();
				return true;
			}
		}
		else
		{
			if (pn = pSelf->name())
			{
				if (!pSelf->hasUglyName())//gcc mangled symbols are legal C names - should be marked based on difference with demangled names
				{
					if (!OwnerScope(iGlob))//the scoped objects expected to be aliased
					{
						int n;
						const char* pc(NameRef_t::skipLenPfx(pn->c_str(), n));
						if (IsLegalCName(pc, n))
						{
							if (n > 0)
								s.assign(pc, n);
							else
								s = pc;
							return true;
						}
						/*if (!pSelf->hasUglyName())
						{
							s = pn->c_str();
							return true;
						}*/
					}
				}
			}
		}
	}
	return false;
}

MyString DcInfo_t::FieldDisplayNameEx(CFieldPtr pSelf, CFieldPtr pImpField) const//IN CONTEXT OF pImpField!
{
	if (FuncInfo_s::IsLocal(pSelf))
	{
CHECKID(pSelf, 0x2a76)
STOP
		GlobPtr iTypeDef(GetLocalOwner(pSelf));
		FuncInfo_t FI(mrDC, *iTypeDef);
		return FI.LocalName(pSelf);
	}

	CGlobPtr pGlob(GlobObjNA(pSelf));
	MyString s;

	if (pImpField)
	{
		TypePtr iModule2(ModuleInfo_t::ModuleOf(pSelf->owner()));
		assert(iModule2 != ModulePtr() || pImpField->isTypeExp());
		DcInfo_t DI2(*DCREF(iModule2));

		if (DI2.GetDisplayName(pSelf, s))//autoname if no pretty name
		{
			//try to display an exported name but check if there is no conflict in current module
			if (OwnerScope(pGlob))//globals only
				return s;
			//const NamesMgr_t &rNS(NamespaceInitialized(PrimeSeg()));
			const NamesMgr_t& rNS(*PrimeSeg()->typeComplex()->namesMgr());
			ObjPtr pObj(rNS.findObj(s));
			if (!pObj)
				return s;//the name is taken
			s.clear();
		}
	}
	else
	{
		if (GetDisplayName(pSelf, s))
			return s;
	}

	//no pretty names down here

	if (pGlob)
	{
		if (pGlob->func())
		{
			if (!pGlob->nameless())
				return pGlob->name()->c_str();
		}

		if (!pSelf->hasUglyName())
		{
			if (!pSelf->nameless())
			{
				if (!OwnerScope(pGlob))
					return pSelf->name()->c_str();//class memebers are to have pretty names
			}

//?			if (LabelNameEx(pSelf, s))
	//?			return s;
		}
	}

	return AutoName(pSelf, pImpField);
}

MyString DcInfo_t::TypeDisplayNameEx(CTypePtr iType0)
{
	CTypePtr iType(iType0);
	if (iType->typeEnum())
		iType = iType->baseType();

	CTypePtr iModule(ModuleOf(iType));
	if (!iModule)//is it non-sharing func ptr?
	{
		if (iType->typeFuncDef())
			iModule = ModuleOfEx((CGlobPtr)iType);
		else
			ASSERT0;
	}
	if (iModule != ModulePtr())//still may be in another module
	{
		//bImported = true;
		CTypePtr iProxy(FindProxyOf(iType));//look for a proxy in current module
		if (iProxy)
		{
			if (!iType->isNested())
				return TypePrettyName(iProxy);//imported top-level types are named in current module
			DcInfo_t DI(*DCREF(iModule));
			return DI.TypePrettyName(iType);
		}
		return StrucNameless(iType);
	}
	return TypePrettyName(iType);
}

/*void DcInfo_t::RebuildTypesMap(ExpTypesMap &m)
{
	std::list<TypesMgr_t::OrderIterator> l;//stack
	l.push_back(TypesMgr_t::OrderIterator(*mrDC.primeSeg()->typeSeg()->typeMgr()));

	while (!l.empty())
	{
		TypesMgr_t::OrderIterator &i(l.back());
		if (!i)
		{
			l.pop_back();
			continue;
		}

		TypePtr iType(*i);
		if (iType->isExpor ting())
		{
			MyString s(TypeNameFull(iType));
			AddCachedType(s, iType);
			if (iType->typeStruc())
			{
				TypesMgr_t *ptm(iType->typeStruc()->typeMgr());
				if (ptm)
				{
					++i;
					l.push_back(TypesMgr_t::OrderIterator(*ptm));
					continue;
				}
			}
		}

		++i;
	}
}*/

int DcInfo_t::ExcludeFile(FileDef_t &rSelf, FolderPtr pFile)
{
	//check if included
	if (!rSelf.removeIncludeList(pFile))
		return false;
	pFile->m_nRefCount++;//?

	return ReleaseFile(pFile);
}

Decompiler_t *DcInfo_t::CreateDecompiler(GlobPtr iCFunc) const
{
	assert(iCFunc->folder());
	/*if (!FILEDEF(pFolder))//the binary has been converted into the source?
	{
		assert(0);
		AssureFileDef(pFolder);
		MyString s(Project().files().relPath(pFolder));
		gui().GuiOnFileRenamed(s, s + SOURCE_EXT);//file name changed
		gui().GuiOnFileListChanged();
	}*/

	if (projx().decompiler())
		return nullptr;

	FolderPtr pFolder(iCFunc->folder());
	AssureMemMgr(pFolder);

	Decompiler_t *pDecompiler(new Decompiler_t(mrDC, iCFunc));

	projx().pushAnalyzer(pDecompiler);
	projx().decompiler()->setCurrentFieldRef(DockField(iCFunc));
	return pDecompiler;
}

GlobPtr DcInfo_t::NewFuncDef()
{
	FuncDef_t *pf(new FuncDef_t());
	pf->setPStackPurge(RetAddrSize());
	//pf->mSpoiltRegs = toRegMask(mrFE.spoilt_regs_def);
	//pf->mSpoiltFlags = (uint16_t)mrFE.spoilt_flags_def;//no flags saved by default
	return memMgrGlob2().NewGlobObj(pf);
}

TypeObj_t* DcInfo_t::findOpType(OpType_t t) const
{
	if (t == OPTYP_PTR32)
	{
		TypesTracer_t tt(*this, memMgrGlob(), *PrimeSeg()->typeMgr());
		return tt.ptrOf(nullptr, PtrSizeOf(PrimeSeg()));
	}
	return GetStockType(t);
}

bool DcInfo_t::CheckThunkAt(CFieldPtr pField) const
{
	return pField->isTypeThunk();
/*	assert(IsGlobal(pField));
	if (!pField->isType Proc())
		return false;
	CFieldPtr pLabel(EntryField(pField->isType Proc()));
	if (!pLabel)
		return false;
	CTypePtr iSeg(OwnerSeg(pField->owner()));
	DataSourcePane2_t data(GetDataSource()->pvt(), iSeg);
	CodeIterator2 codeIt(data, pLabel->isTypeCode(), iSeg, pField->address(), mrFrontDC);
	for (;;)//while (codeIt.unassemble())
	{
		ADDR va(codeIt.updateAddr());

		PCode_t pcode;
		if (!codeIt.generate(pcode))
			break;

		const ins_desc_t &desc(codeIt.desc());
		if (desc.isControlTransfer())
			return desc.isUnconditionalJump();

		//check if there is an instruction using a stack pointer register
		for (PCode_t::const_iterator iti(pcode.begin()); iti != pcode.end(); ++iti)
		{
			const INS_t &rINS(*iti);
			for (lstOPND_cit ito(rINS.ops.begin()); ito != rINS.ops.end(); ++ito)
			{
				const OPND_t &op(*ito);
				if (op.ssid() == mrFE.stack_ptr->ssid)
					if (op.offs() == mrFE.stack_ptr->ofs)
						return false;
			}
		}
	}*/
	return false;
}

void DcInfo_t::AssureFuncDef(GlobPtr iGlob) const
{
CHECKID(iGlob, 0x4393)
STOP
	assert(iGlob);
	FuncDef_t *pf(iGlob->func());
	if (!pf)
	{
		pf = new FuncDef_t();
		iGlob->SetPvt(pf);
		pf->setPStackPurge(RetAddrSize());
		//pf->mSpoiltRegs = toRegMask(mrFE.spoilt_regs_def);
		//pf->mSpoiltFlags = (uint16_t)mrFE.spoilt_flags_def;//no flags saved by default
	}
	else
	{
		assert(iGlob->typeFuncDef());
	}
}

void DcInfo_t::AssureThunk(GlobPtr iGlob) const
{
//CHECKID(iGlob,5471)
//STOP
	assert(iGlob);
	/*TypeThunk_t *pf(iGlob->thunk());
	if (!pf)
	{
		pf = new TypeThunk_t();
		iGlob->SetPvt(pf);
	}
	else
	{
		assert(iGlob->isThunk());
	}*/
}

//look back
ADDR DcInfo_t::CheckUnderlap(FieldMapCIt it0) const
{
	CFieldPtr pField0(VALUE(it0));
	SSID_t ssid(FuncInfo_s::SSIDx(pField0));
	if (Storage(ssid).isDiscrete())
		return 0;

	TypePtr iOwner(pField0->owner());
	//const Struc_t &rStruc(*iOwner->typeStruc());
	const FieldMap &m(iOwner->typeStruc()->fields());
	ADDR base(iOwner->base());
	ADDR target(pField0->_key() - base);
	ADDR gap(target);
	FieldMapCIt it(it0);
	if (it != m.begin())
	{
		--it;
		CFieldPtr pField(VALUE(it));
		if (!Storage(FuncInfo_s::SSIDx(pField)).isDiscrete())
		{
			ADDR upper(pField->addressHi2() - base);
			if (upper < target)
				gap -= upper;
			else
				gap = 0;
		}
		else
			gap = 0;
	}
	return gap;
}

bool DcInfo_t::AssureTypeClass(TypePtr pSelf, bool bOrNamespace) const
{
CHECKID(pSelf, 0x74d7)
STOP
	if (bOrNamespace)
	{
		if (!pSelf->typeClass())//namespaces allowed - it s a static member
			if (AssureTypeNamespace(pSelf, false))
				return true;
	}
	
	if (pSelf->ObjType() == OBJID_TYPE_CLASS)
		return true;

	if (pSelf->ObjType() == OBJID_TYPE_STRUC)
	{
		Struc_t *pStruc(pSelf->typeStruc());
		TypeClass_t *pClass(new TypeClass_t(pStruc->ID()));//preserve id
		pClass->Struc_t::moveFrom(*pStruc);
		pSelf->ClearPvt();
		pSelf->SetPvt(pClass);
		return true;
	}
	if (pSelf->ObjType() == OBJID_TYPE_NAMESPACE)
	{
		if (bOrNamespace)
			return true;//already
		TypeNamespace_t *pNamespace(pSelf->typeNamespace());

		//all nested namespaces must be stepped down as well
		TypesMgr_t *pTypesMgr(pNamespace->typeMgr());
		if (pTypesMgr)
		{
			const TypesMap_t &m(pTypesMgr->aliases());
			for (TypesMapCIt i(m.begin()); i != m.end(); i++)
			{
				TypePtr iType2(i->pSelf);
				if (iType2->typeNamespace())
					AssureTypeClass(iType2);
			}
		}

		TypeClass_t *pClass(new TypeClass_t(pNamespace->ID()));//preserve id
		pClass->moveFrom(*pNamespace);
		pSelf->ClearPvt();
		pSelf->SetPvt(pClass);

		const ClassMemberList_t &l(pClass->methods());
		for (ClassMemberListCIt it(l.begin()); it != l.end(); ++it)
		{
			//FieldPtr pField(*it);
			GlobPtr pGlob(*it);
			if (pGlob->func())
			{
				//assert(!(pGlob->flags() & TYP_FDEF_ CONST));
//?				assert(!(pGlob->flags() & FDEF_VIRTUAL));
				if (!(pGlob->flags() & FDEF_STATIC))
				{
					//FuncInfo_t FI(dc(), *pGlob);
//CHECKID(pGlob, 0x1e2a6)
//STOP
					FieldPtr pArg(AssureThisPtrTo(pGlob, pSelf, false));
					(void)pArg;
					//if (pArg)
						//FuncInfo_t::MakeThisPtrArg(pArg);
				}
			}
		}
		return true;
	}
	return false;
}

bool DcInfo_t::AssureTypeStruc(TypePtr iType) const//revert from a class
{
	if (!iType->typeStruc())
		return false;//struc derivatives only considered
	if (iType->ObjType() == OBJID_TYPE_STRUC)
		return true;
	TypeClass_t *pClass(iType->typeClass());
	if (!pClass || HasMethods(iType))
		return false;
	Struc_t *pStruc(new Struc_t(pClass->ID()));
	pStruc->moveFrom(*iType->typeStruc());
	iType->ClearPvt();
	iType->SetPvt(pStruc);
	return false;
}

/*bool DcInfo_t::AddStaticMember(TypePtr iClass, FieldPtr pField)
{
	assert(iClass->typeStruc());
CHECKID(pField, 35261)
STOP
	if (IsStaticMemberFunction(pField))//already
	{
		assert(HasMember(iClass, pField));
		assert(OwnerScope(pField) == iClass);
	}
	else
	{
		if (!AddClassMember(iClass, pField))
			return false;
	}

	
	return true;
}*/

/*bool DcInfo_t::DelocateFieldName(FieldPtr pField)
{
	if (pField->nameless())
		return true;//already

	NamesMgr_t *pNS(OwnerNamesMgr(pField->owner(), nullptr));
	PNameRef pn(pField->name());
//CHECK(!strcmp(pn->c_str(), "?Load@CBufFile@@QAE_NPBDPAPAX@Z"))
CHECKID(pField, 0x3b9)
STOP
	if (!RegisterPrettyFieldName(pn, pField))
	{
		assert(0);
		//fprintf(STDERR, "Warning: Failed class member name at %s : %s\n", VA2STR(pField->address()).c_str(), pn->c_str());
		//return false;
	}
	else
	{
		pNS->removen(pn);
		pField->setName(nullptr);
		assert(0);
		//pField->setUgly Name();
	}
	return true;
}

bool DcInfo_t::DelocateTypeName(TypePtr iSelf)
{
	CHECKID(iSelf, 0xa5d8)
		STOP

	if (iSelf->nameless())
		return true;//already

	NamesMgr_t *pNS(OwnerNamesMgr(iSelf->owner(), nullptr));
	PNameRef pn(iSelf->name());
	//CHECK(!strcmp(pn->c_str(), "?Load@CBufFile@@QAE_NPBDPAPAX@Z"))
	//STOP
	if (!RegisterPrettyTypeName(pn, iSelf))
	{
		assert(0);
		//fprintf(STDERR, "Warning: Failed class member name at %s : %s\n", VA2STR(pField->address()).c_str(), pn->c_str());
		//return false;
	}
	else
	{
		pNS->removen(pn);
		iSelf->setName(nullptr);
	}
	return true;
}*/

GlobPtr DcInfo_t::NewFuncDef(FieldPtr pField, FolderPtr pFolder)
{
	assert(pFolder);
	assert(!GlobFuncObj(pField));
	FieldExPtr pFieldx(AddGlobToFile2(pField, pFolder));
	GlobPtr iGlob(GlobObj(pFieldx));
	AssureFuncDef(iGlob);
	return iGlob;
}

node_t* DcInfo_t::FromSymbol(MyString sMangled, bool bCreate) const
{
	if (sMangled.isEmpty())
		return nullptr;
	node_t* pn(projx().cxxSymbols().findSymbol(sMangled));
	if (!pn && bCreate)
	{
		MyString sDemangled;
		I_Front::SymbolKind eKind(demangleSymbol(mrDC.frontEnd(), sMangled, sDemangled));
		if (!sDemangled.isEmpty() && sDemangled != sMangled)
			pn = projx().cxxSymbols().addSymbol(sMangled, sDemangled);//add one to ProjectEx_t::cxxSymbols()
	}
	return pn;
}

MyString DcInfo_t::SymbolName(CFieldPtr pField, bool bCreate) const
{
	if (pField->name())
	{
		node_t* pn(FromSymbol(pField->name()->c_str(), bCreate));
		if (pn)
		{
			size_t n(ScopePos(pn->name));
			if (n != MyString::npos)
				return pn->name.substr(n + 2);
			return MyString(pn->name);
		}
	}
	return "";
}

int DcInfo_t::CollectExportedSymbols() const
{
	int count(0);
	//Module_t &aBin(*iModule->typeModule());
	TypePtr iFrontSeg(FindFrontSeg());
	if (iFrontSeg)
	{
		Seg_t &rFrontSeg(*iFrontSeg->typeSeg());
		I_Front *pIFront(IFrontOf(iFrontSeg));
		if (pIFront)
		{
			CxxSymbMap &cxx(projx().cxxSymbols());
			SymbolMap symb(GetDataSource()->pvt(), pIFront);
			symb.dumpExported();
			for (SymbolMap::Iterator symIt(symb); symIt; symIt++)//EXPORTS
			{
				//if (symIt.isModule())
				//continue;
				MyString sMangled(symIt.symbol());
				MyString sDemangled;
				I_Front::SymbolKind eKind(symIt.demangled(sDemangled));
				if (!sDemangled.isEmpty() && sDemangled != sMangled)
				{
					cxx.addSymbol(sMangled, sDemangled);
					count++;
				}
			}
		}
	}
	return count;
}

int DcInfo_t::CollectImportedSymbols() const
{
	int count(0);
	//Module_t &aBin(*iModule->typeModule());
	TypePtr iFrontSeg(FindFrontSeg());
	if (iFrontSeg)
	{
		Seg_t &rFrontSeg(*iFrontSeg->typeSeg());
		I_Front *pIFront(IFrontOf(iFrontSeg));
		if (pIFront)
		{
			CxxSymbMap &cxx(projx().cxxSymbols());
			SymbolMap symb(GetDataSource()->pvt(), pIFront);
			symb.dumpImported();
			for (SymbolMap::Iterator symIt(symb); symIt; symIt++)//IMPORTS
			{
				MyString sMangled(symIt.symbol());
				MyString sDemangled;
				I_Front::SymbolKind eKind(symIt.demangled(sDemangled));
				if (!sDemangled.isEmpty() && sDemangled != sMangled)
				{
					cxx.addSymbol(sMangled, sDemangled);
					count++;
				}
			}
		}
	}
	return count;
}

int DcInfo_t::CollectDebugSymbols() const
{
	int count(0);
	TypePtr iFrontSeg(FindFrontSeg());
	if (iFrontSeg)
	{
		Seg_t &rFrontSeg(*iFrontSeg->typeSeg());
		I_Front *pIFront(IFrontOf(iFrontSeg));
		if (pIFront)
		{
			CxxSymbMap &cxx(projx().cxxSymbols());
			SymbolMap symb(GetDataSource()->pvt(), pIFront);
			symb.dumpDebugInfo1(*this);
			for (SymbolMap::Iterator symIt(symb); symIt; symIt++)//DEBUGS
			{
				MyString sMangled(symIt.symbol());
				MyString sDemangled;
				I_Front::SymbolKind eKind(symIt.demangled(sDemangled));
				if (!sDemangled.isEmpty() && sDemangled != sMangled)
				{
					cxx.addSymbol(sMangled, sDemangled);
					count++;
				}
			}
		}
	}
	return count;
}


bool DcInfo_t::AddInheritance(TypePtr iSelf, FieldPtr pField0, int /*iKind*/)
{
CHECKID(iSelf, 0x1fc7)
STOP
	Struc_t &rSelf(*iSelf->typeStruc());
	//if (!rSelf.hasFields())
		//return false;
	for (FieldMapIt i = rSelf.fields().begin(), E = rSelf.fields().end(); i != E; ++i)
	{
		FieldPtr pField(VALUE(i));
		TypePtr iType(pField->type());
		if (!iType)
			break;
		//if (pField->flags() & FLD_HIER__MASK)
		if (pField->flags() & FLD_HIER_PUBLIC)
		{
			if (pField == pField0)
				return true;
			continue;
		}
		if (!iType->typeStruc())
			break;
		if (pField0 && pField != pField0)
			break;
		//pField->flags() &= ~FLD_HIER__MASK;
		//pField->flags() |= ((iKind & 3) << FLD_HIER__SHIFT);
		pField->flags() |= FLD_HIER_PUBLIC;
		return true;
	}

	return false;
}

bool DcInfo_t::RemoveInheritance(TypePtr iSelf)
{
	Struc_t &rSelf(*iSelf->typeStruc());
	FieldPtr pFieldLast(nullptr);
	for (FieldMapIt i = rSelf.fields().begin(), E = rSelf.fields().end(); i != E; ++i)
	{
		FieldPtr pField(VALUE(i));
		//TypePtr iType(pField->type());
		//if (pField->flags() & FLD_HIER__MASK)
		if (pField->flags() & FLD_HIER_PUBLIC)
			pFieldLast = pField;
		else
			break;
	}
	if (pFieldLast)
	{
		pFieldLast->flags() &= ~FLD_HIER_PUBLIC;//FLD_HIER__MASK;
		return true;
	}
	return false;
}

bool DcInfo_t::ToggleVFTablePointer(TypePtr iSelf)
{
	Struc_t &rSelf(*iSelf->typeStruc());
	if (!rSelf.fields().empty())
	{
	}

	return false;
}

bool DcInfo_t::AssureTypeNamespace(TypePtr iSelf, bool bVerbose) const
{
	if (iSelf->typeNamespace())
	{
		assert(iSelf->ObjType() == OBJID_TYPE_NAMESPACE);
		return true;//already
	}
	if (iSelf->ownerScope() && !iSelf->ownerScope()->typeNamespace())
	{
		if (bVerbose)
			PrintError() << "Could not convert to namespace. Type " << TypePrettyNameFull(iSelf, CHOP_SYMB)
				<< " is nested, owner scope: " << TypePrettyNameFull(iSelf->ownerScope(), CHOP_SYMB) << std::endl;
		return false;
	}
	Struc_t *pStruc(iSelf->typeStruc());
	if (!pStruc || pStruc->hasFields())//namespaces do not have data fields
	{
		if (bVerbose)
			PrintError() << "Could not convert to namespace. " << TypePrettyNameFull(iSelf, CHOP_SYMB) << " has data fields" << std::endl;
		return false;
	}

	//now make sure there is no references from types or fields; an only reference is in typesmgr
	//the problem is that some non static member functions could have 'this' pointer set to the iSelf. 
	// Have to undone this first and see if there are still other refs
	bool bFailure(false);
	std::list<FieldPtr>  l2;
	TypeClass_t *pClass(pStruc->typeClass());
	if (pClass)
	{
		BinaryCleaner_t<> BC(Project());
		ClassMemberList_t &l(pClass->methods());
		for (ClassMemberListIt it(l.begin()); it != l.end(); ++it)
		{
			GlobPtr pGlob(*it);
			assert(OwnerScope(pGlob) == iSelf);
			if (!IsStaticMemberFunction(pGlob))
			{
				if (pGlob->func())
				{
					FuncDef_t *pfDef(pGlob->typeFuncDef());
					FieldPtr pThis(ProtoInfo_t::ThisPtrArg(pGlob));
					assert(pThis);

					if (pfDef->isStub())//can delete a former this ptr field
					{
//?						pfDef->setThisPtr(nullptr);
						BC.ClearType(pThis);
						//l2.push_back(pThis);
						pClass->addMember(pGlob);//?why: pThis);
					}
					else
					{
						bFailure = true;
						break;
					}
				}
			}
		}
	}

	//check a reference count
	if (iSelf->refsNum() > 1)//only typesmgr keeps one!
	{
		if (bVerbose)
			PrintError() << "Could not convert to namespace. " << TypePrettyNameFull(iSelf, CHOP_SYMB) << " is referenced (" << iSelf->refsNum() - 1 << ")" << std::endl;
		bFailure = true;
	}

	if (!l2.empty())//?
	{
		assert(0);
		ProjectInfo_t PI(Project());//in global memmgr!
		while (!l2.empty())
		{
			FieldPtr pThis(l2.front());
			l2.pop_front();
			GlobPtr iFuncDef((GlobPtr)pThis->owner());
			//FuncDef_t *pfDef(iFuncDef->typeFuncDef());
			if (!bFailure)
			{
				//delete old 'this' ptrs
				PI.TakeField0((GlobToTypePtr)iFuncDef, pThis);
				memMgr().Delete(pThis);
			}
			else
			{
				//restore 'this' ptrs
				FuncInfo_t FI(dc(), *iFuncDef);
				FI.SetConstPtrToStruc(pThis, iSelf);
			}
		}
	}

	if (!bFailure)
	{
		if (iSelf->ObjType() == OBJID_TYPE_STRUC)
		{
			Struc_t *pStruc(iSelf->typeStruc());
			TypeNamespace_t *pNamespace(new TypeNamespace_t(pStruc->ID()));//preserve id
			assert(!pStruc->hasFields());
			pNamespace->Struc_t::moveFrom(*pStruc);
			iSelf->ClearPvt();
			iSelf->SetPvt(pNamespace);
			assert(!iSelf->isExporting());
			return true;
		}

		if (iSelf->ObjType() == OBJID_TYPE_CLASS)
		{
			TypeClass_t *pClass(iSelf->typeClass());
			TypeNamespace_t *pNamespace(new TypeNamespace_t(pClass->ID()));//preserve id
			pNamespace->moveFrom(*pClass);
			iSelf->ClearPvt();
			iSelf->SetPvt(pNamespace);
			assert(!iSelf->isExporting());
			return true;
		}
		assert(0);
	}
	return false;
}


typedef std::list<std::pair<size_t, size_t> >	removed_list;
typedef std::stack<size_t>	order_stack;
typedef	std::vector<int> remap_vector;
typedef std::list<size_t>	adj_list;
/*class adj_list : public std::list<int>
{
public:
	adj_list(){}
	void sortRamapped(const remap_vector &remap)
	{
	}
};*/



class Graph : public std::vector<adj_list>// an array containing adjacency lists
{
	// A recursive function used by topologicalSort 
	void topologicalSortUtil(size_t v, std::vector<bool> &visited, order_stack& Stack)
	{
		// Mark the current node as visited. 
		visited[v] = true;

		// Recur for all the vertices adjacent to this vertex 
		const adj_list &l(at(v));
		for (adj_list::const_iterator i(l.begin()); i != l.end(); ++i)
			if (!visited[*i])
				topologicalSortUtil(*i, visited, Stack);

		// Push current vertex to stack which stores result 
		Stack.push(v);
	}

	bool isCyclicUtil(size_t v, std::vector<bool> &visited, std::vector<bool> &recStack) const
	{
		assert(!visited[v]);

		// Mark the current node as visited and part of recursion stack 
		visited[v] = recStack[v] = true;

		// Recur for all the vertices adjacent to this vertex 
		const adj_list &l(at(v));
		for (adj_list::const_iterator i(l.begin()); i != l.end(); ++i)
		{
			if (!visited[*i] && isCyclicUtil(*i, visited, recStack))
				return true;
			if (recStack[*i])
				return true;
		}

		recStack[v] = false;  // remove the vertex from recursion stack 
		return false;
	}

	//back edges removal
	void removeCyclesUtil(size_t v, std::vector<bool> &visited, std::vector<bool> &recStack, removed_list &removed)
	{
		assert(!visited[v]);

CHECK(v == 29)
STOP
		// Mark the current node as visited and part of recursion stack 
		visited[v] = recStack[v] = true;

		// Recur for all the vertices adjacent to this vertex 
		adj_list &l(at(v));
		adj_list::iterator i(l.begin());
		while (i != l.end())
		{
			if (!visited[*i])
				removeCyclesUtil(*i, visited, recStack, removed);
			if (!recStack[*i])
			{
				++i;
			}
			else
			{
				removed.push_back(std::make_pair(v, *i));
				adj_list::iterator j(i++);
				l.erase(j);
			}
		}

		recStack[v] = false;  // remove the vertex from recursion stack 
	}

public:
	Graph(size_t V)
		: std::vector<adj_list>(V)
	{
	}

	void addEdge(size_t v, int w)
	{
		at(v).push_back(w); // Add w to v’s list. 
	}

	// prints a Topological Sort of the complete graph 
	// The function to do Topological Sort. It uses recursive topologicalSortUtil() 
	void topologicalSort(order_stack &Stack)
	{
		// Mark all the vertices as not visited 
		std::vector<bool> visited(size());
		// Call the recursive helper function to store Topological Sort starting from all vertices one by one 
		for (size_t i(0); i < size(); i++)
			if (!visited[i])
				topologicalSortUtil(i, visited, Stack);
	}

	// Returns true if the graph contains a cycle, else false. 
	bool isCyclic() const//variation of DFS()
	{
		// Mark all the vertices as not visited and not part of recursion stack 
		std::vector<bool> visited(size());//will be initialized
		std::vector<bool> recStack(size());
		// Call the recursive helper function to detect cycle in different DFS trees 
		for (size_t i(0); i < size(); i++)
			if (!visited[i] && isCyclicUtil(i, visited, recStack))
				return true;
		return false;
	}

	// Returns true if the graph contains a cycle, else false. 
	void removeCycles(removed_list &removed)//variation of DFS()
	{
		// Mark all the vertices as not visited and not part of recursion stack 
		std::vector<bool> visited(size());//will be initialized
		std::vector<bool> recStack(size());
		// Call the recursive helper function to detect cycle in different DFS trees 
		for (size_t i(0); i < size(); i++)
			if (!visited[i])
				removeCyclesUtil(i, visited, recStack, removed);
	}

	template <class _Fn>
	void DFSUtil(size_t v, std::vector<bool> &visited, _Fn _Func)
	{
		// Mark the current node as visited and print it 
		visited[v] = true;
		_Func(v);

		// Recur for all the vertices adjacent to this vertex 
		for (adj_list::iterator i(at(v).begin()); i != at(v).end(); ++i)
			if (!visited[*i])
				DFSUtil(*i, visited, _Func);
	}

	// DFS traversal of the vertices reachable from v. It uses recursive DFSUtil() 
	template <class _Fn>
	void DFS(_Fn _Func)
	{
		// Mark all the vertices as not visited 
		std::vector<bool> visited(size());
		// Call the recursive helper function to print DFS traversal 
		for (int i(0); i < size(); i++)
			if (!visited[i])
				DFSUtil(i, visited, _Func);
	}
};

void DcInfo_t::CalculateDependencies(FolderPtr pFolderTop)
{
	if (!pFolderTop)
		pFolderTop = ModuleFolder();

	// 1) we need a way to assign an index to a folder in consistent way (otherwise the ordering is going to change from session to session)
	std::map<MyString, CFolderPtr> a;
	for (FilesMgr0_t::FolderIterator i(pFolderTop); i; ++i)//only subtree
	{
		CFolderRef pFolder(*i);
		if (pFolder.fileDef())
			a.insert(std::make_pair(FilesMgr0_t::relPath(&pFolder), &pFolder));
	}
	unique_vector<CFolderPtr> folderVec;
	folderVec.reserve(a.size());
	for (std::map<MyString, CFolderPtr>::const_iterator i(a.begin()); i != a.end(); i++)
		folderVec.add(i->second);
	a.clear();//no longer needed

	// 2) build a graph of dependencies among the files
	Graph g(folderVec.size());
	for (size_t i(0); i < folderVec.size(); i++)
	{
		CFolderPtr pFolder(folderVec[i]);
		FileDef_t *pFileDef(pFolder->fileDef());
		//cout << i << ":\t" << FilesMgr0_t::relPath(pFolder) << endl;
		//create unique list of dependencies
		std::set<CFolderPtr> m;
		pFileDef->buildDependencies(m);
		if (!m.empty())
		{
			//add new dependencies to the graph
			int fromIndex(folderVec.indexOf(pFolder));
			for (std::set<CFolderPtr>::const_iterator j(m.begin()); j != m.end(); j++)
				g.addEdge(i, folderVec.indexOf(*j));
		}
	}

#if(0)
	for (size_t i(0); i < g.size(); i++)
	{
		cout << FilesMgr0_t::relPath(folderVec[i]) << endl;
		for (adj_list::const_iterator j(g[i].begin()); j != g[i].end(); j++)
			cout << "\t" << FilesMgr0_t::relPath(folderVec[*j]) << endl;
	}
#endif

	// 3) remove back-edges
	removed_list removed;
	g.removeCycles(removed);
	for (removed_list::const_iterator i(removed.begin()); i != removed.end(); i++)
		fprintf(STDERR, "Warning: Circular types dependency exists from %s to %s\n",
			FilesMgr0_t::relPath(folderVec[(*i).first]).c_str(),
			FilesMgr0_t::relPath(folderVec[(*i).second]).c_str());
	assert(!g.isCyclic());

	// 4) perform topological sort on the graph
	order_stack orderStack;//and remap vector (less dependent nodes come first)
	//orderStack.reserve(folderVec.size());
	g.topologicalSort(orderStack);
	assert(g.size() == orderStack.size());

	// 5) buid a indexes re-map vector (given an index in FolderVec, get a weighted postion in files dependencies list)
	remap_vector remapVec(folderVec.size());
	for (int i(0); !orderStack.empty(); i++)
	{
		remapVec[orderStack.top()] = i;
#if(0)
		cout << orderStack.top() << ":\t" << FilesMgr0_t::relPath(folderVec[orderStack.top()]) << endl;
#endif
		orderStack.pop();
	}
	assert(remapVec.size() == folderVec.size());

	// 6) sort graph's adjacency lists according to topologycal ordering (via remap vector)
	class RemapComparator
	{
		const remap_vector &remapVec;
	public:
		RemapComparator(const remap_vector &r)
			: remapVec(r){
		}
		bool operator ()(size_t a, size_t b) const {
			return remapVec[a] < remapVec[b];
		}
	};
	for (size_t i(0); i < g.size(); i++)
	{
		// Sorting List using Function Objects as comparator
		g[i].sort(RemapComparator(remapVec));
	}

#if(0)
	for (size_t i(0); i < g.size(); i++)
	{
		cout << FilesMgr0_t::relPath(folderVec[i]) << endl;
		for (adj_list::const_iterator j(g[i].begin()); j != g[i].end(); j++)
			cout << "\t" << FilesMgr0_t::relPath(folderVec[*j]) << endl;
	}
#endif

	/*class increment
	{
	private:
		int num;
	public:
		increment(int n) : num(n) {  }
		int operator () (int arr_num) const {
			return num + arr_num;
		}
	};

	int arr[] = {1, 2, 3, 4, 5}; 
    int n = sizeof(arr)/sizeof(arr[0]); 
    int to_add = 5; 
  
    std::transform(arr, arr+n, arr, increment(to_add));*/

	/*class dfs_functor
	{
		const unique_vector<FolderPtr> &folderVec;
	public:
		dfs_functor(const unique_vector<FolderPtr> &_folderVec)
			: folderVec(_folderVec)
		{
		}
		void operator () (int v) const {
			std::cout << FilesMgr0_t::relPath(folderVec[v]) << std::endl;
		}
	};*/

	class dfs_dummy_functor {//just for marking nodes
	public:
		void operator () (size_t) const {}
	};

	for (size_t v(0); v < g.size(); v++)
	{
		//g.DFS(dfs_functor(folderVec));
		CFolderPtr pFolder(folderVec[v]);

		int cnt = 0;
		std::vector<bool> visited(g.size());
		const adj_list &l(g[v]);
		for (adj_list::const_iterator i(l.begin()); i != l.end(); ++i)
		{
			if (!visited[*i])
			{
				pFolder->fileDef()->addIncludeList((FolderPtr)folderVec[*i]);
				g.DFSUtil(*i, visited, dfs_dummy_functor());//mark vertices
				cnt++;
			}
		}

		std::cout << FilesMgr0_t::relPath(folderVec[v]) << " (" << cnt << " )" << std::endl;
	}

}

/*void DcInfo_t::ValidateGlobalName(FieldPtr p)
{
	assert(IsGlobal(p));
	TypePtr pScope(OwnerScope(p));
	const NamesMgr_t *nm_0(OwnerNamesMgr(PrimeSeg(), nullptr));
	const NamesMgr_t *nm_1(pScope ? OwnerNamesMgr(pScope, nullptr) : nullptr);

	if (p->hasUg lyName())
	{
		if (p->hasPrettyName())
		{
			if (pScope)
			{
				//[0]
				assert(nm_0->contains(p->name()));
				assert(nm_1->contains(FindPrettyName(p)));
				//BIN: dysplays UGLY name
				//SRC: dysplays PRETTY name
			}
			else
			{
				//[1]
				assert(nm_0->contains(p->name()));
				assert(nm_0->contains(FindPrettyName(p)));
				//BIN: dysplays UGLY name
				//SRC: dysplays PRETTY name
			}
		}
		else//no pretty name
		{
			if (pScope)
			{
				//[2]
				assert(nm_0->contains(p->name()));
				//BIN: dysplays UGLY name
				//SRC: displays AUTO name
			}
			else
			{
				//[3]
				assert(nm_0->contains(p->name()));
				//BIN: dysplays UGLY name
				//SRC: displays AUTO name
			}
		}
	}
	else//NO UGLY NAME
	{
		if (p->hasPrettyName())
		{
			assert(!FindPrettyName(p));//PRETTY NAME is not kept in PrettyNmaesMap!
			if (pScope)
			{
				//[4]
				assert(nm_1->contains(p->name()));
				//BIN: displays <SCOPE>::<SCOPE>::..::<>
				//SRC: displays PRETTY name
			}
			else
			{
				//[5]
				assert(nm_0->contains(p->name()));
			}
		}
		else//no pretty name
		{
			if (pScope)
			{
				//[6]
				//BIN: display if p->name(), display p->name(), otherwise - AUTO name
				//SRC: display if p->name(), display p->name(), otherwise - AUTO name
			}
			else
			{
				//[7]
			}
		}
	}
}*/

TypePtr DcInfo_t::NameOwnerOf(FieldPtr p) const
{
	assert(IsGlobal(p));
	if (p->name())
	{
		CGlobPtr g(GlobObj(p));
		if (p->hasUglyName())
		{
			if (g && g->hasPrettyName())
			{
				if (OwnerScope(p))
					return PrimeSeg();
				return PrimeSeg();
			}
			else//no pretty name
			{
				if (OwnerScope(p))
					return PrimeSeg();
				return PrimeSeg();
			}
		}
		else//NO UGLY NAME
		{
			if (g && g->hasPrettyName())
			{
				assert(0);//IMPOSSIBLE: pretty names are to alias ugly ones
			}
			else//no pretty name
			{
				if (OwnerScope(p))
					return PrimeSeg();
				return PrimeSeg();
			}
		}
	}
	return nullptr;
}

TypePtr DcInfo_t::PrettyNameOwnerOf(FieldPtr p) const
{
	assert(IsGlobal(p));
	return nullptr;
}

TypePtr DcInfo_t::FuncTypeFromProfile(const FuncProfile_t &si) const
{
	TypesTracer_t TT(*this, memMgrGlob(), *PrimeSeg()->typeMgr());
	
	unsigned flags(0);

	if (si._flags & PPF_Variardic)
		flags |= TypeFunc_t::E_VARIARDIC;

	if (si.stackin > 0 && si.stackin == si.pstackPurge)
		flags |= TypeFunc_t::E_CLEANARG;

	Arg1List_t lArgs;
	GetTempArgs(si, lArgs);
	TypePtr iArgs(nullptr);
	for (Arg1List_t::reverse_iterator i(lArgs.rbegin()); i != lArgs.rend(); ++i)
	{
		const Arg1_t &a(*i);
		TypePtr iNew(GetStockType(a.optyp()));
		if (!iArgs)
			iArgs = iNew;
		else
			iArgs = TT.pairOf(iNew, iArgs);
	}

	Arg1List_t lRets;
	GetTempRets(si, lRets);
	TypePtr iRets(nullptr);
	for (Arg1List_t::reverse_iterator i(lRets.rbegin()); i != lRets.rend(); ++i)
	{
		const Arg1_t &a(*i);
		TypePtr iNew(GetStockType(a.optyp()));
		if (!iRets)
			iRets = iNew;
		else
			iRets = TT.pairOf(iNew, iRets);
	}

	return TT.funcTypeOf(iRets, iArgs, flags);
}


MyString DcInfo_t::LocalRegToString(CFieldPtr p) const
{
	assert(FuncInfo_s::IsLocalReg(p));
	SSID_t ssid(FuncInfo_s::SSIDx(p));
	int ofs(FuncInfo_s::address(p));
	int sz(p->size());
	return RegToString(ssid, REG_t(ofs, sz), true);
}

MyString DcInfo_t::LocalToString(int ofs, unsigned sz)
{
	MyString s;
	if (ofs == 0)
	{
		s = "@RETADDR";
	}
	else
	{
		if (ofs > 0)//args
			s = "A";
		else//vars
		{
			s = "V";
			ofs = -ofs;
		}
		if (sz != 0)
		{
			switch (sz)
			{
			case OPSZ_BYTE: s += 'B'; break;
			case OPSZ_WORD: s += 'W'; break;
			case OPSZ_DWORD: s += 'D'; break;
			case OPSZ_QWORD: s += 'Q'; break;
			default: s += '?'; break;
			}
		}
//?		if (flags & 1)
	//		s = s.lower();
		//if (sz > 0)
			s += '@';//at
		s.append(Int2Str(ofs, I2S_HEXA));
	}

	return s;
}

MyString DcInfo_t::RegToString(SSID_t ssid, REG_t reg, bool force) const
{
	if (ssid == SSID_CPUSW)
	{
		assert(0);
		//unsigned flags(unsigned(sz) << ofs);
		//return mrDC.flagsToStr((SSID_t)opc, flags);
	}
	char buf[80];
	const char *pc(mrDC.toRegName(ssid, reg.m_ofs, reg.m_siz, 0, buf));
	if (!pc && force)
	{
		ADDR key(FuncInfo_s::setupKey(ssid, reg.m_ofs, reg.m_siz));
		MyString s("R@");
		s.append(VA2STR(key));
		return s;
	}
	return MyString(pc);
}

MyString DcInfo_t::FlagsToStr(OPC_t opc, unsigned flags, bool b) const
{
	assert(flags);
	SSID_t ssid(SSID_t(opc & 0xF));
	const STORAGE_t &ss(mrDC.SS(ssid));

	if (b)
		return MyString().sprintf("%s.%s", ss.name, mrDC.flagsToStr(ssid, flags));

	MyString s(MyString().sprintf("%s{%s}", ss.name, mrDC.flagsToStr(ssid, flags, '.')));
	/*?if (ofs)
	{
		strcat(g_buf, ".");
		strcat(g_buf, Int2Str(ofs).c_str());
	}*/
	return s;
}

MyString DcInfo_t::ArgListToString(const Arg1List_t &l, const char *sep) const
{
	MyString s;
	for (Arg1List_t::const_iterator i(l.begin()); i != l.end(); ++i)
	{
		if (!s.isEmpty())
			s.append(sep);
		const Arg1_t &a(*i);
		s.append(RegToString(a.ssid(), REG_t(a.offs(), a.opsz()), true));
	}
	return s;
}

MyString DcInfo_t::GPRsToString(SSID_t opc, const GPRs_t &v, const char *sep) const
{
	MyString s;
	for (size_t i(0); i < v.size(); i++)
	{
		const REG_t &r(v[i]);
		if (!s.isEmpty())
			s.append(sep);
		s.append(RegToString(opc, r, true));
	}
	return s;
}

void DcInfo_t::WriteGlobInfo(CTypeBasePtr iType, MyStreamBase &ss) const
{
	MyString s1, s2;
	if (iType)
	{
		FullName_t v;
		CGlobPtr iGlob(iType->objGlob());
		CFolderPtr pFolder;
		if (iGlob)
		{
			v = GlobNameFull(iGlob, E_PRETTY, CHOP_SYMB);
			pFolder = FolderOf(iGlob);
		}
		else
		{
			v = TypeNameFull(iType, E_PRETTY, CHOP_SYMB);
			pFolder = FolderOf(iType->objType());
		}
		s1 = v.join();
		if (pFolder)
			s2 = mrProject.files().relPath(pFolder);
	}

	MyStreamUtil ssu(ss);
	ssu.WriteString(s1);
	ssu.WriteString(s2);
}

MyString DcInfo_t::extractVName(const MyString& s0) const//module!<faile|obj_name>
{
	MyString sObjName(s0);
	MyString sModule(ModuleName());
	if (ZPath_t::moduleOf(sObjName) != sModule)
		return "";
	sObjName.remove(0, unsigned(sModule.length() + 1));
	return sObjName;
}

GlobPtr DcInfo_t::CheckGlob(CObjPtr pObj)
{
	if (!pObj)
		return nullptr;
	CGlobPtr pGlob(pObj->objGlob());
	if (!pGlob)
	{
		CFieldPtr pField(pObj->objField());
		if (pField && IsGlobal(pField))
			pGlob = GlobObj(pField);
	}
	return (GlobPtr)pGlob;
}

TypePtr DcInfo_t::ReconcileTypes(TypePtr pType1, TypePtr pType2)
{
	if (!pType1)
		return pType2;//right can be null too
	if (!pType2)
		return 0;//left isn't null

	bool bComplex1(pType1->typeComplex() != 0);
	bool bComplex2(pType2->typeComplex() != 0);

	if (bComplex1 ^ bComplex2)
		return 0;//one simple, other - complex

	if (bComplex1)
	{
		if (pType1->typeComplex() == pType2->typeComplex())
			return pType1;
		return 0;
	}

	//?	if (mpStruc)
	//?	if (!T.mpStruc)
	//?	return false;//left is struc, right is not

	if (!(pType1->typeSimple() && pType2->typeSimple()))
		return 0;

	uint8_t t1 = pType1->typeSimple()->optype();
	uint8_t t2 = pType2->typeSimple()->optype();
	if (!AgreeTypes(t1, t2))
		return 0;

	if (pType1->typePtr())
	{
		TypesTracer_t TT(*this, memMgrGlob(), *mrProject.typeMgr());
		if (pType2->typePtr())
		{
			TypePtr pType(ReconcileTypes(pType1->typePtr()->pointee(), pType2->typePtr()->pointee()));
			if (!pType)
				return 0;
			return TT.ptrOf(pType);
		}
		TypePtr pType(ReconcileTypes(pType1->typePtr()->pointee(), nullptr));
		if (!pType)
			return 0;
		//TypesTracer_t TT(*this, mrProject.typeMgr());
		return TT.ptrOf(pType);
	}
	if (pType2->typePtr())
	{
		TypePtr pType(ReconcileTypes(nullptr, pType2->typePtr()->pointee()));
		if (!pType)
			return 0;
		TypesTracer_t TT(*this, memMgrGlob(), *mrProject.typeMgr());
		return TT.ptrOf(pType);
	}

	return GetStockType((OpType_t)t1);//?
}

bool DcInfo_t::LocusFromVAEx(ADDR va, Locus_t &aLoc) const
{
	TypePtr iPrimeSeg(PrimeSeg());
	const Seg_t &rSeg0(*iPrimeSeg->typeSeg());
	TypePtr iSeg(rSeg0.findSubseg(va, rSeg0.affinity()));
	if (!iSeg)
		return false;
	return ProjectInfo_t::LocusFromVA(iSeg, va, aLoc, false);
}

//when we apply a prototype at some location, check if there is a already an entry to pick up
FieldPtr DcInfo_t::AssureDockField(FieldPtr pField0, const MyString& sName)
{
	if (sName.empty())
		return nullptr;//don't bother

	MyString nul;
	//assert(IsGlobal(pField0) && !pField0->isClone());
	TypePtr iSeg(OwnerSeg(pField0->owner()));
	//if (pField0->isCloneMaster())
	{
		FieldPtr pField(MatchExistingField(pField0, nul, sName, nullptr));
		if (pField)
			return pField;
	}
	/*else
	{
		GlobPtr pGlob(GlobObj(pField0));
		if (!pGlob)
			return pField0;

		//check if a it's time to clone
		if (FieldCompName0(pField0, nul, sName, nullptr))
			return pField0;//already?
	}*/
	FieldPtr pField2(AddSecondaryField(iSeg, pField0->_key(), nullptr));

	if (SetFieldName2(pField2, sName, true))
		pField2->setHardNamed(true);

	return pField2;
}

MyString DcInfo_t::MakePrettyName(MyString s0, TypePtr pClass, int& forceMode)
{
	if (forceMode != 2)
		NameScopeChopped(s0, s0, false);
	else
		forceMode = 0;

	if (s0.empty())
		return MyString();

	MyString sScope;
	if (pClass)//?  - need this to identify constructors/destructors
		sScope = pClass->name()->c_str();

	MyString s(EnhancedName(s0, sScope));//no more scope
	if (s.isEmpty())
		return MyString();

	if (forceMode == 0)
		if (s.rfind("operator", 0) == 0)//startsWith//operator is reserved
			forceMode = 1;
	return s;
}

//forceMode: as in addnf, + 2 : force as is
bool DcInfo_t::ApplyPrettyName(GlobPtr iGlob, MyString s0, TypePtr pClass, int forceMode) const
{
//CHECKID(pField, 0x1580)
//STOP

	//GlobPtr iGlob(GlobObj(pField));
	assert(iGlob);
//CHECKID(iGlob, 0xa2d)
//STOP

	MyString s(MakePrettyName(s0, pClass, forceMode));

	PNameRef pn0(nullptr);
	if (iGlob->hasPrettyName())
		pn0 = FindPrettyName(iGlob);
	else if (!OwnerScope(iGlob))//?pField))
		pn0 = DockName(iGlob);

	if (pn0)
	{
		MyString s3;
		ChopName(pn0->c_str(), s3);
		if (s3 == s)
			return true;//already
	}

/*	if (bTilda && iSpecial == 0)//CHECKIT! (mfc42.dll)
		s.prepend("~");*/

	if (!pClass)
		pClass = PrimeSeg();
	//bool bIsGlobal(iClass == PrimeSeg());

	NamesMgr_t &rNS(OwnerNamespaceEx(pClass));
	assert(IsMemMgrGlob());

	/*if (pField->nameless())// && bIsGlobal)//pretty names are only as substitutes!
	{
		addnf(rNS, s, pField, forceMode);//ugly globals may also have pretty names
		assert(!pField->hasUg lyName());
	}
	else*/ if (!iGlob->hasPrettyName())
	{
		PNameRef pn(ForcePrettyName(rNS, s, iGlob, forceMode));//the alternative (pretty) name
		assert(pn);

		//ugly names for static vars (not funcs) are supposed to be inferred from mangled=>demangled names

		if (!RegisterPrettyFieldName(pn, iGlob))//ELF mangled symbols do have legitimate C names
			ASSERT0;
		}

	return true;
}

bool DcInfo_t::ApplyPrettyName(TypePtr pSelf, MyString s0, TypePtr iClass, int forceMode) const
{
CHECKID(pSelf, 29112)
STOP
	if (pSelf->hasUglyName())
	{
		//is there a pretty name already?
		PNameRef pn0(FindPrettyName(pSelf));
		if (pn0)
			return false;
	}

	if (forceMode != 2)
	{
		size_t n(ScopePos(s0));
		if (n != MyString::npos)
			s0 = s0.remove(0, unsigned(n + 2));
	}
	else
		forceMode = 0;

	if (s0.empty())
		return false;

	MyString s(s0);//no more scope

	if (s_ChopTemplatedPrefix(s) == 0)
		return false;//ugly from the very first symbol, no a pretty preffix to use

	if (s == s0)
		return false;

	if (!iClass)
		iClass = PrimeSeg();
	NamesMgr_t &rNS(OwnerNamespaceEx(iClass));
//	if (forceMode == 0)
	//	if (s.rfind("operator", 0) == 0)//startsWith//operator is reserved
		//	forceMode = 1;

	assert(!pSelf->nameless());
	assert(IsMemMgrGlob());
	
	PNameRef pn(ForcePrettyName(rNS, s, pSelf, 1));//the alternative (pretty) name, always should be suffixed to emphasize the fact it has been renamed
	if (!RegisterPrettyTypeName(pn, pSelf))
		ASSERT0;

	return true;
}

void DcInfo_t::SetPtrToStruc(FieldPtr pField, TypePtr iClass) const
{
	if (pField->isPtrToStruc() == iClass)
		return;//already
	TypesTracer_t tt(*this, memMgrGlob(), *PrimeSeg()->typeMgr());
	TypePtr iType(tt.ptrOf(iClass, PtrSizeOf(PrimeSeg())));
	assert(iType);
	if (pField->type() && pField->type() != iType)
	{
		DcCleaner_t<> DC((DcInfo_t &)*this);
		DC.ClearType(pField);
	}
	SetType(pField, iType);
}

void DcInfo_t::SetConstPtrToStruc(FieldPtr pField, TypePtr iClass) const
{
	if (pField->isConstPtrToStruc() == iClass)
		return;//already
	TypesTracer_t tt(*this, memMgrGlob(), *PrimeSeg()->typeMgr());
	TypePtr iType(tt.ptrOf(iClass, PtrSizeOf(PrimeSeg())));
	assert(iType);
	iType = tt.constOf(iType);
	if (pField->type() && pField->type() != iType)
	{
		DcCleaner_t<> DC((DcInfo_t &)*this);
		DC.ClearType(pField);
	}
	SetType(pField, iType);
}

void DcInfo_t::SetConstPtrToConstStruc(FieldPtr pField, TypePtr pClass) const
{
	if (pField->isConstPtrToConstStruc() == pClass)
		return;//already
	TypesTracer_t tt(*this, memMgrGlob(), *PrimeSeg()->typeMgr());
	TypePtr p(tt.constOf(pClass));
	assert(p);
	p = tt.ptrOf(p, PtrSizeOf(PrimeSeg()));
	assert(p);
	p = tt.constOf(p);
	assert(p);
	if (pField->type() && pField->type() != p)
	{
		DcCleaner_t<> DC((DcInfo_t &)*this);
		DC.ClearType(pField);
	}
	SetType(pField, p);
}

void DcInfo_t::DumpBlocks(CGlobPtr ifDef, MyStreamBase &ss) const
{
	FuncInfo_t FI(*this, *ifDef);

	MyStreamUtil ssu(ss);
	FI.WriteGlobInfo(ifDef, ss);

	FI.DumpBlocks(*ifDef->typeFuncDef(), ss);
}

void DcInfo_t::DumpExpr(MyStreamBase &ss, unsigned uiFlags, CGlobPtr pFunc, OpPtr pOp) const
{
	assert(pFunc->typeFuncDef());
	FuncInfo_t FI(mrDC, *pFunc);
	HOP hOp(pOp);
	if (hOp)
	{
		if (uiFlags == adcui::IADCExprViewModel::DUMPEXPR_PTRS)
		{
			if (mrDC.ReadPtrDump(FI.DockAddr(), FI.OpNo(hOp), ss))
				return;
			uiFlags = adcui::IADCExprViewModel::DUMPEXPR_UNFOLD;
		}

		adcui::UDispFlags uflags;
//?		if (safe->isUnfoldMode())
		if (uiFlags == adcui::IADCExprViewModel::DUMPEXPR_UNFOLD)
			uflags.setL(adcui::DUMP_UNFOLD);

		uflags.setL(adcui::DUMP_BLOCKS);

		ExprCacheEx_t aCache(FI.PtrSize());
		MyStreamUtil ssu(ss);

		EXPR_t expr(FI, hOp, uflags, aCache);
		Out_t* pOut(expr.DumpOp(hOp));

		TExprDump2view<EXPRSimpl_t> ES(FI, hOp, uflags, aCache, 0, ssu);
		ES.Simplify(pOut);

		/*HPATH hPath(FuncInfo_t::PathOf(hOp));
		if (hPath->Type() == BLK_JMPSWITCH)
		{
			SwitchQuery_t si;
			if (ES.SimplifySwitch(pOut, si, hPath))
				ES.SimplifySwitchEx(pOut);
		}*/
	}
}

int DcInfo_t::DumpExprPtr(MyStreamBase &ss) const
{
	MyStreamUtil ssh(ss);
	for (MyPtrDumps::const_iterator i(mrDC.mPtrDumps.begin()); i != mrDC.mPtrDumps.end(); i++)
	{
		ADDR funcAddr(i->first);
		Locus_t loc;
		FieldPtr pField(FindFieldInSubsegs(PrimeSeg(), funcAddr, loc));
		assert(pField);
		MyString funcName(GlobNameFull(GlobObj(pField), E_PRETTY, CHOP_SYMB).join());
		const MyPtrDump &r(i->second);
		for (MyPtrDump::const_iterator j(r.begin()); j != r.end(); j++)
		{
			int line(j->first);
			ssh.WriteStringf("%s\t%d", funcName.c_str(), line);
		}
	}
	return (int)mrDC.mPtrDumps.size();
}

bool DcInfo_t::GetTempArgs(const FuncProfile_t &fp, Arg1List_t &lArgList) const
{
	//cpu regs
	bool bThiscall(CpuRegs2ArgList(fp.cpuin, lArgList));

	//FPU params adjust
	int nFpuIn(fp.fpuin / FTOP_STEP);
	for (int i(0); i < nFpuIn; i++)
	{
		int id(nFpuIn - i);
		lArgList.push_back(Arg1_t(OPC_FPUREG, (id - 1) * FR_SLOT_SIZE, OPTYP_REAL80));
	}

	//stack params adjust
	uint32_t param = 0;

	int stackaddrsz = mrFE.stackaddrsz;
	int retaddrsize = mrFE.ptr_near;//RetAddrSize();

	uint32_t bytes_max = fp.stackin;
	assert(bytes_max % stackaddrsz == 0);
	uint32_t param_max = bytes_max / stackaddrsz;
	while (param < param_max)
	{
		lArgList.push_back(Arg1_t(OPC_LOCAL, param * stackaddrsz + retaddrsize, (OpType_t)stackaddrsz));
		param++;
	}
	return bThiscall;
}

bool DcInfo_t::CpuRegs2ArgList(const GPRs_t &v, Arg1List_t &l) const
{
	//cpu regs
	bool bThiscall(false);
	for (size_t i(0); i < v.size(); i++)
	{
		const REG_t &r(v[i]);
		if (!r.isValid())
		{
			assert(i == 0);
			continue;//no this ptr
		}
		else if (i == 0)
			bThiscall = true;//not applicable for input vectors other than args one
		l.push_back(Arg1_t(OPC_CPUREG, r.m_ofs, (OpType_t)r.m_siz));
	}
	return bThiscall;
}

void DcInfo_t::ArgList2CpuRegs(const Arg1List_t &l, bool bThisCall, GPRs_t &v) const
{
	if (!l.empty())
	{
		int n(0);
		if (!bThisCall)
			v.push_back(REG_t());
		for (Arg1List_t::const_iterator i(l.begin()); i != l.end(); ++i, n++)
		{
			const Arg1_t &r(*i);
			assert(r.opc() == OPC_CPUREG);
			v.push_back(REG_t(r.offs(), r.opsz()));
		}
	}
}

void DcInfo_t::GetTempRets(const FuncProfile_t &fp, Arg1List_t &lArgList) const
{
	//cpu regs
	CpuRegs2ArgList(fp.cpuout, lArgList);

	int fpuIn(fp.fpuin / FTOP_STEP);
	int fpuDiff(fp.fpudiff / FTOP_STEP);
	int z = 0 - fpuIn + fpuDiff;
	int fpuArgs(-z);
	assert(fpuArgs >= 0);
	//for (int i(z); i < 0; i++)
	for (int i(0); i < fpuArgs; i++)//fpu ret vals are relative to the exit point (retops must be shifted down by fpu top at exit location)
	{
		//int id(fpuIn + i);
		int id(i);
		lArgList.push_back(Arg1_t(OPC_FPUREG, id * FR_SLOT_SIZE, OPTYP_REAL80));
	}
}

unsigned DcInfo_s::checkOverlap(CFieldPtr pSelf)
{
	TypePtr iStruc(pSelf->owner());
	if (iStruc->typeUnion())
		return 0;
	const FieldMap &m(iStruc->typeStruc()->fields());
	FieldMapCIt i(m.find(pSelf->_key()));
	assert(i != m.end());
	if (++i == m.end())
		return 0;
	CFieldRef rNext(*i);
	if (FuncInfo_s::SSIDx(&rNext) != FuncInfo_s::SSIDx(pSelf))
		return 0;
	/*if (iStruc->typeBitset())
	{
		return 0;
	}
	else*/
	{
		ADDR a1(rNext._key());
		if (pSelf->_key() == a1)
			return 0;//union
		ADDR a2(pSelf->addressHi());
		
		if (a2 <= a1)
			return 0;
		return a2 - a1;
	}
}

bool DcInfo_t::CheckProblem(CFieldPtr pField, MyString &sReason, bool bIsDeclaration) const
{
	assert(pField);
	if (IsGlobal(pField))
	{
		CGlobPtr pGlob(GlobObj(pField));
		if (pGlob)
		{
			if (bIsDeclaration)
			{
				if (DockField(pGlob)->isTypeImp())
				{
					CFieldPtr pImpField(DockField(pGlob));
					if (!ToExportedField(pImpField))
					{
						sReason = MyStringf("Unresolved imported entry");
						return true;
					}
				}

				if (ClassInfo_t::IsMethodVirtual(pGlob))
				{
					if (!ProtoInfo_s::IsThisCallType(pGlob))
					{
						sReason = MyStringf("A virtual method expected to be non-static");
						return true;
					}
				}
				else
				{
					int vptr_off;
					if (ClassInfo_t::IsVTable(pGlob, vptr_off))
					{
						if (vptr_off < 0)
						{
							sReason = MyStringf("V-table has no associated `vptr`");
							return true;
						}
						//check if there is a vptr at given offset in class hierarchy
						TypePtr pClass(OwnerScope(pGlob));
						Locus_t aLoc;
						aLoc.push_back(Frame_t(pClass, 0, nullptr));
						terminalFieldAt(aLoc);
						CFieldPtr pVPtr(aLoc.field0());
						if (!pVPtr || !pVPtr->type() || !pVPtr->type()->typeVPtr())
						{
							sReason = MyStringf("No `vptr` at offset %d in class", vptr_off);
							return true;
						}
						//FieldPtr pVptr(Field(pClass, 0, nullptr, FieldIt_Exact));
					}
				}
			}
			else
			{
			}
		}
	}
	else if (FuncInfo_t::IsLocalVar(pField))
	{
	}
	else
	{
		unsigned uOverlap(checkOverlap(pField));
		if (uOverlap > 0)
		{
			sReason = MyStringf("Succeeding field overlap by value of %d", uOverlap);
			return true;
		}
	}
	return false;
}

/*static unsigned __toOrdinal(const MyString& s)
{
	//either name or ordinal must be specified for exported attribute
	if (s.empty())
		return ORDINAL_NULL;
	return atoi(s.c_str());
}*/

FieldPtr DcInfo_t::GetExportedField(const MyStringEx& aName) const
{
	const ProjectEx_t& proj(projx());
	if (aName[0].isEmpty())
		return proj.exportPool().findSuffix(FromCompoundName(aName));
#if(0)
	//both name and tag
	FieldPtr pField(proj.exportPool().find(FromCompoundName(aName)));
	if (!pField)
	{
		if (aName.size() > 1)//no tag
		{
			//ALWAYS TRY BY NAME FIRST (ORDINALS MAY BE INCONSISTENT!)
			pField = proj.exportPool().findPrefix(FromCompoundName(aName, 1));//no tag
			if (!pField)
				pField = proj.exportPool().findPrefix().findSuffix(FromCompoundName(aName, -1));//no name
		}
	}
	return pField;
#else
	return proj.exportPool().findPrefix(FromCompoundName(aName, 1));//ignore a tag
#endif
}

FieldPtr DcInfo_t::TakeBogusField(const MyStringEx& aName)
{
	TypePtr pFrontSeg(FindFrontSeg());
	assert(pFrontSeg);

	FieldPtr pField(GetExportedField(aName));

	if (pField)
	{
		Dc_t::BogusFieldMap& m(mrDC.bogusFields());
		pField = m.take(pField);//the field is not of this module?
	}

	if (!pField)
		return nullptr;
CHECKID(pField, 0xe5c8)
STOP

	assert(pField->isExported());
	if (!projx().exportPool().remove(pField))
		ASSERT0;

	if (!pField->owner())
		pField->setOwnerComplex(PrimeSeg());
	ClearFieldName(pField);
	pField->setOwnerComplex(nullptr);

	return pField;
}

FieldPtr DcInfo_t::ExtraditeUnresolvedExternal(const MyStringEx& aName, TypePtr pRecipientModule)
{
	assert(projx().unresolvedExternalsModule() == ModulePtr());
	TypePtr pFrontSeg(FindFrontSeg());
	assert(pFrontSeg);
	const MyString& sName(aName[0]);
	TypePtr pCont(mrProject.GetNameOwnerOf(pFrontSeg));

//	FieldPtr pField(FindFieldByName(sName, pCont));
//	if (!pField)
//		return nullptr;

	FieldPtr pField(GetExportedField(aName));
	if (!pField || ModuleOf(pField) != ModulePtr())
		return nullptr;

CHECKID(pField, 0x1191)
STOP

	GlobPtr pGlob(GlobObjNA(pField));
	assert(pGlob);

	FileInfo_t GI(mrDC, *pGlob->folder()->fileDef());
	if (GI.RecallGlob(pGlob))//get rid of glob
	{
		projx().exportPool().remove(pField);
		TakeField0(pField->owner(), pField);//extract,discard name and type
		return pField;
	}

/*	ModuleInfo_t MI(Project(), *pRecipientModule);
	GlobPtr pGlob(GlobObjNA(pField));
	assert(pGlob);
	TypePtr pScope(OwnerScope(pGlob));
	if (pScope)
	{
		if (!pScope->typeNamespace())
		{
			TypePtr pScopeTop(TypeTop(pScope));//namespace?
			FolderPtr pFolder(OwnerFolder(pScopeTop));
			FileInfo_t FI(mrDC, *pFolder->fileDef(), memMgrGlob());

			if (!FI.CanRelocateType(pScopeTop))
				return nullptr;
			FI.RelocateType(pScopeTop, MyString(), false);//no proxy type
			//all member fields will be relocated
			return pField;
		}

		ClassInfo_t CI(*this, pScope, memMgrGlob());
		CI.RemoveClassMember(pField, pGlob);

		FileDef_t* pf(pGlob->folder()->fileDef());
		pf->takeGlob(pGlob);
		STOP

		/ *TypeClass_t* pClass(pScope->typeClass());
		assert(pClass);
		//BinaryCleaner_t<> BC(Project());
		ClassMemberList_t& l(pClass->methods());

		if (!pScope->typeStruc()->takeField(pField))
		{
			STOP
		}* /
	}*/

	return pField;
}


void DcInfo_t::CleanupUnclaimedExports()
{
	TypePtr iFrontSeg(FindFrontSeg());
	if (!iFrontSeg)
		return;

	NamesMgr_t* pNS(iFrontSeg->typeStruc()->namesMgr());

	Dc_t::BogusFieldMap& m(mrDC.bogusFields());

	ADDR uOrd;
	FieldPtr pField;
	while ((pField = m.takeFirst(&uOrd)) != nullptr)
	{
		PrintError() << "Phony field (ordinal " << uOrd - ORDINAL_BIAS << ") not reclaimed: "
			<< (pField->nameless() ? "<NONAME>" : pField->name()->c_str()) << std::endl;

		pField->setOwnerComplex(iFrontSeg);//why?
		mrProject.deleteBogusField(pField, pNS);
	}

}

FieldPtr DcInfo_t::CheckThruConst(CFieldPtr pData, OFF_t &oPos) const
{
	assert(IsGlobal(pData));
	//if (IsLocal(pData))
		//return nullptr;
	if (pData->name())
		return nullptr;

	if (!IsConst(pData) || IsTypeImp(pData))
		return nullptr;

	if (!IsThruConst(pData))
		return nullptr;

	TypePtr pType(pData->type());
	if (!pType)
		return nullptr;

	if (!pType->typeSimple())
	{
		//return nullptr;
		if (pType->typePtr())
			return nullptr;//?

		//if (pType->typeComplex())
			//return nullptr;

		//Array_t* pPvtArray(pType->typeArray());
		//if (pPvtArray)
		{
			//if (!pPvtArray->baseType()->typeSimple())//strings?
				//return nullptr;
			//if (pPvtArray->baseType()->typePtr())
				//return nullptr;
			if (!pData->isStringOf(OPTYP_CHAR8))
				if (!pData->isStringOf(OPTYP_CHAR16))
					return nullptr;
		}
	}

	if (!GetRawOffset(pData, oPos))
		ASSERT0;

	return (FieldPtr)pData;
}







