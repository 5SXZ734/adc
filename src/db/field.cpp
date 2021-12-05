#include "field.h"
#include "prefix.h"

#include "qx/MyString.h"
#include "shared/defs.h"
#include "shared/link.h"
#include "shared/misc.h"
#include "shared/action.h"
#include "mem.h"
#include "obj.h"
#include "type_seg.h"
#include "type_struc.h"
#include "type_code.h"
#include "types.h"
#include "info_proj.h"



Field_t::Field_t()//load-only version
	: Obj_t(OBJID_FIELD),
	TreeLinkInfo(0),
	mpType(nullptr),
	mpName(nullptr),
	mpOwner(nullptr)
{
}

/*Field_t::Field_t(FieldKeyType k)
	: Obj_t(OBJID_FIELD),
	TreeLinkInfo(k),
	mpType(nullptr),
	mpName(nullptr),
	mpOwner(nullptr)
{
//CHECKID(this,135)
//STOP
}*/

#if(!NO_OBJ_ID)
Field_t::Field_t(/*FieldKeyType k,*/ int id)
	: Obj_t(OBJID_FIELD, id),
	TreeLinkInfo(0),//k),
	mpType(nullptr),
	mpName(nullptr),
	mpOwner(nullptr)
{
}
#endif

/*Field_t::Field_t(const Field_t &o)
	: Obj_t(o),//avoid objid increment
	TreeLinkInfo(o._key()),
	mpType(nullptr),
	mpName(nullptr),
	mpOwner(nullptr)
{
}*/

#if(!NO_OBJ_ID)
Field_t::Field_t(Field_t&& o, int id) : Obj_t(std::move(o), id)
#else
Field_t::Field_t(Field_t&& o) : Obj_t(std::move(o))
#endif
	,TreeLinkInfo(std::move(o))
	,mpType(o.mpType)
	,mpName(o.mpName)
	,mpOwner(o.mpOwner)//weak
{
	//rebind(this, &o);
	o.mpType = nullptr;
	o.mpName = nullptr;
	o.mpOwner = nullptr;
}

Field_t::~Field_t()
{
	assert(nameless());
	//assert(xrefs().empty());
#if(!NO_EXTRA_USER)
	assert(!m_user2.hasUserData());
#endif
	assert(!type());
	assert(!hasUserData());
}

void Field_t::CopyFrom(CFieldRef o, MemoryMgr_t &rMemMgr)
{
	assert(0);/*
	m_nFlags = o.m_nFlags;

	setTy pe(o.type());
	mpName = nullptr;//?
	assert(!o.mpName);

	m_opc = o.m_opc;
	mOffset = o.mOffset;

	setOwnerComplex(nullptr);//?

	assert(!mpPath_ && !o.mpPath_);
	assert(m_xrefs.empty() && o.m_xrefs.empty());*/
}

bool Field_t::destruct(MemoryMgr_t &rMemMgr)
{
	assert(nameless());//name must be cleared at this point by some parent
	if (hasUserData())//must get rid of path in the first place
		return false;
	assert(!mpType);
	assert(!hasUserData());
	return true;
}

uint8_t Field_t::OpSize() const
{
	return (mpType) ? mpType->size() : 0; //(m_opsz); }
}

OpType_t Field_t::OpTypeZ() const
{
	TypePtr iType(ProjectInfo_t::SkipModifier(mpType));
	if (iType)
	{
		Array_t *pArray(iType->typeArray());
		if (pArray)
			 iType = pArray->absBaseType();
		Simple_t *pSimple(iType->typeSimple());
		if (pSimple)
			return (OpType_t)pSimple->optype();
	}
	return OPTYP_NULL;
}

/*void Field_t::SetOpType(OpType_t tp)
{
	//(OpType_t)((int)tp&(~OPTYP _PTR))
	/ *TypePtr pType(nullptr);//G DC.typeMgr()->type(tp)
	if (tp & OPTYP _PTR)
	{
		//assert(pType->isShared());
		TypesTracer_t tt(*findTypeMgr(pType));
		pType = tt.ptrOf(nullptr);
	}
	else* /
	TypePtr pType(GetStockType(tp));
	assert(pType);
	assert(pType->isShared());
	setT ype(pType); // m_optyp = tp; }
}*/

/*size_t Field_t::Array()
{
	if (type()->typeArray())
		return type()->typeArray()->total();
	return 1; 
}*/

TypePtr Field_t::typex() const
{
	assert(!mpType->typeProxy());
	if (mpType->typeImp())
		return mpType->baseType();
	return /*ProjectInfo_t::SkipProxy*/(mpType);
}

/*int Field_t::CheckXRefStatus()
{
	for (XOpList_t::Iterator i(xr efs()); i; i++)
	{
		Op_t op(i.data());
		if (!op->IsCodeOp() || !op->isHidden() )//valid!!!
			return 1;
	}

	m_nFlags |= LBL_UNREFD;

	return 0;
}*/

bool Field_t::isComplex()
{
	return mpType && (mpType->typeComplex() != nullptr);
}

int Field_t::OvercastType(uint8_t t)
{assert(0);/*?
CHECK(offset() == 0x1fdd)
STOP
	int sz = t&0xf;
	if (t <= OpSize())
		return 0;
	FieldPtr p = this;
	do {
		FieldPtr pNx = (FieldPtr )p->Nex t();
		int d = p->offset()-offset();
		if (d + p->OpSize() > sz)
			break;
		if (!(p->OpType() & 0xF0))
			if (d == 0)
				p->SetOpType((OpType_t)t);
			else
				delete p;
		p = pNx;
	} while (p);
*/
	return 1;
}

bool Field_t::IsEqualTo(FieldPtr p)
{
	//if (FuncInfo_s::SSIDx(this) != FuncInfo_s::SSIDx(p))
		//return false;
	if (_key() != p->_key())
		return false;
	if (OpSize() != p->OpSize())
		return false;

	return true;
}

void Field_t::Clear()
{
	m_nFlags = 0;

	//m_opc = OPC_NULL;

	assert(0);/*
	setT ype(nullptr);*/
}

/*
Struc_t * Field_t::CreateStruct()
{//generate struct definition from data declaration
	Struc_t *pStruc = AssureComplex(0);

	FieldPtr pStructField = 0;

	for (
		FieldPtr pField = m_pOp;
		pField;
		pField = (FieldPtr )pField->Nex t())
	{
		if (pStructField)//check for array
		{
			if ( (pField->m_optyp == pStructField->m_optyp)
				&& (pField->m_pStruc == pStructField->m_pStruc) )
			{
				assert(pField->offset() > pStructField->offset());
				int sz = pStructField->OpSize();
				int d = pField->offset() - (pStructField->offset() + pStructField->m_array*sz);
				if ( (d < 0) || (d % sz) )
					pStructField = 0;
				else
				{
					pStructField->m_array += d / sz;//add number inside gap
					pStructField->m_array += pField->m_array;//add number with itself
				}
			}
			else
				pStructField = 0;
		}

		if (!pStructField)
		{
			pStructField = pStruc->AddF ield(m_nOffset);
			if (!pStructField)
				return 0;
			
			pStructField->m_optyp = pField->m_optyp;
			pStructField->m_pStruc = pField->m_pStruc;
			pStructField->m_array = pField->m_array;
		}	
	}

	//pStruc->AddDataRef(this);

	return pStruc;
}
*/

FieldPtr  Field_t::GetField(int nOffset)
{
	TypePtr pTypeEl = type();
	if (pTypeEl->typeArray())
	{
		pTypeEl = pTypeEl->typeArray()->baseType();
		assert(!pTypeEl->typeArray());
		nOffset %= pTypeEl->size();
	}

	assert(pTypeEl->typeStruc());

	return pTypeEl->typeStruc()->GetFieldEx(nOffset, 1);
}

/*ostream& FIELD(ostream& stream, int iOffs)
{
	stream << Int2Str(iOffs, I2S_SIGNED|I2S_HEXA);
	return stream;
}

OMANIP(int) FIELD(int iOffs)
{
	return OMANIP(int) (FIELD, iOffs);
}*/

bool Field_t::IsComplexOrArray()
{
	return (type() && (type()->typeComplex() || type()->typeArray()));
}

bool Field_t::IsComplex0()
{
	return (type() && type()->typeComplex());
}

/*int Field_t::CheckXRefs()
{
	if (!IsComplexOrArray())
		return 0;
	if (!IsL ocal())
		return 0;

	for (XFieldList_t::Iterator i(xr efs()); i; i++)
	{
		OpPtr op(i.data().op());
		if (op->IsCodeOp())
		{
			OpPtr pOp = PrimeOp(op);
			if (pOp->IsPri meOp())
				if (pOp->isHidden())
				{
					FuncInfo_t f0(G DC, *i.data().pFunc->funcdef());
					InitialTracer_t an(f0);//*pOp->Get Owner Proc());
					an.SetShown(pOp);
				}
		}
	}

	return 1;
}*/

Struc_t	* Field_t::OwnerStruc() const
{
	assert(mpOwner && mpOwner->typeStruc());
	return mpOwner->typeStruc();
}

TypePtr Field_t::OwnerComplex() const 
{
	assert(mpOwner && mpOwner->typeComplex());
	return (TypePtr)mpOwner;
}

void Field_t::setOwnerComplex(TypePtr pComplex)
{
CHECKID(this,0x354ef)
STOP
	assert(!pComplex || !pComplex->typeProxy());
	mpOwner = pComplex; 
}


/*PathPtr  Field_t::GetOwnerPath() const
{
	Struc_t * pStruc = OwnerStruc();
	if ( pStruc )
	{
		StrucLoc_t * pStrucLoc = pStruc->typeStrucLoc();
		if ( pStrucLoc )
			return pStrucLoc->OwnerPath();
	}
	return nullptr;
}*/

/*bool Field_t::isArg() const
{
	return (owner()->typeFuncDef() && order() <= LOCAL_ORDER_ARG_UPPER);
}

bool Field_t::isRet() const
{
	return (owner()->typeFuncDef() && order() >= LOCAL_ORDER_RET_LOWER);
}*/

int Field_t::size() const
{
	if (type())
	{
		if (owner() && owner()->typeBitset())//!!CHECK OWNER!! (notepad ex01)
		{
			if (type()->typeArray())
				return type()->typeArray()->total();
			return 1;
		}
		return type()->size();
	}
	return 0;
}

int	Field_t::sizeBytes() const
{
	if (!type())
		return 0;
	return type()->sizeBytes();
}

/*int Field_t::siz ep() const
{
	int sz = 0;
	if (type())
		sz = type()->siz ep();
//?	if (sz <= 0)
//?		sz = 1;
	return sz;
}*/

/*int Field_t::fixedSize()
{
/ *	Type_t * pType = type();
	if (pType)
		return pType->fixedSize();* /
	return size();
}*/

/*int Field_t::oversize()
{
	ADDR_RANGE sz = size();
	ADDR_RANGE szP = type() ? type()->siz ep() : 0;

	int d = sz - szP;
	if (d > 0)
		return d;

	return 0;
}*/

/*FieldPtr Field_t::GetOwnerFile()
{
	Complex_t * pComplex = OwnerComplex();
	if (pComplex->typeSeg())
		return Obj_t::GetOwnerFile();

	return pComplex->GetOwnerFile();
}*/

TypePtr Field_t::isPtrToStruc(bool bSkipModifier) const
{
	return ProjectInfo_t::IsPtrToStruc(mpType, bSkipModifier);
}

TypePtr Field_t::isPtrToConstStruc(bool bSkipModifier) const
{
	return ProjectInfo_t::IsPtrToConstStruc(mpType, bSkipModifier);
}

TypePtr Field_t::isConstPtrToStruc(bool bSkipModifier) const
{
	return ProjectInfo_t::IsConstPtrToStruc(mpType, bSkipModifier);
}

TypePtr Field_t::isConstPtrToConstStruc(bool bSkipModifier) const
{
	return ProjectInfo_t::IsConstPtrToConstStruc(mpType, bSkipModifier);
}

/*TypePtr Field_t::isPtrToFunc() const
{
	TypePtr iStruc(isPtrToStruc());
	if (!iStruc)
		return nullptr;
	if (!iStruc->type Proc())
		return nullptr;
	return iStruc;
}*/

#if(0)
//checks if this is a real required object;
//if not, enlarge it till that
FieldPtr  Field_t::AssureObjType(ObjId_t objid)
{
	assert(objField());
	if (ObjType() == objid)
		return this;

	assert(0);

	FieldPtr pMLoc = 0;
	switch (objid)
	{
//?	case OBJ_ DATA:
//?		if (Obj Type() != OBJID_UNK)
//?			OUTPUT.fatalerror("Incompatible object conversion");//Can't convert labels and functions into datas
//?		pMLoc = new Fi eld_t;
//?		pMLoc->m_pOwnerStruc = m_pOwnerStruc;
//?		break;
	case OBJID_NULL:
		return 0;
	case OBJID_FIELD:
//		if (ObjT ype() != OBJID_UNK)
//?			OUTPUT.fatalerror("Incompatible object conversion");//Can't convert datas and functions into labels
//		pMLoc = new Field_t();
	//	pMLoc->setOwnerComplex(mpOwner);
		break;
	case OBJID_TYPE_PROC:
//?		if (ObjT ype() == OBJ_ DATA)
//?			OUTPUT.fatalerror("Incompatible object conversion");//Can't convert datas into functions
		if (ObjType() == OBJID_FIELD)
		{
			FieldPtr pLabel = this;
			if (pLabel->ownerProcPvt())//it seems that there is a need to split function!
			{
				pMLoc = pLabel->ConvertToFunc(0);
				return pMLoc;
			}
		}
		else if (ObjType() == OBJID_FIELD)
		{
			if (type()->type Proc())//m_pStruc->Is Func() )
			{
				return this;
			}
		}
		assert(0);
		//?pMLoc = new TypeProc_t;
		break;
	default:
		assert(false);
	}

assert(0);//?	assert(IsLinked());
assert(0);//?	pMLoc->LinkAfter(this);
	/*?pMLoc->setOffset( offset() );

	char buf[NAMELENMAX];
	buf[0] = 0;
	if (!nameless())
		na mex(buf);

	RedirectXDeps(pMLoc);
	ObjReplace(pMLoc);
	//Delete();
	delete this;

	if ( buf[0] != '\0' )
		pMLoc->Set Name(buf, 0);*/

	return pMLoc;
}
#endif


ADDR Field_t::checkGap() const
{
	//get a gap's offset
	TypePtr iStruc(owner());
	//if (isClone());
	const FieldMap &m(iStruc->typeStruc()->fields());
	FieldMapCIt i(m.find(_key()));
	assert(i != m.end());
	ADDR o(0);
	i = m.Priorz(i);
	if (i != m.end())
		o = i->_key() + i->size();
	return o;
}

/*

int Field_t::OffsetHi()//for unnamed unions && strucs
{
	int nSize = 0;
	int nOffsLo;

	FieldPtr pField = this;
	while (1)
	{
		nOffsLo = pField->offset();
		nSize = pField->Size();

		FieldPtr pFieldPr = (FieldPtr )pField->P rev();
		if (!pFieldPr)
			break;
		int d = pField->offset() - pFieldPr->offset();
		assert(d > 0);

		if (pFieldPr->Size() > d)
		{
			nOffsLo = pFieldPr->offset();
			while (pFieldPr)
			break;
		}

		pField = pFieldPr;
	}

	int nOffsHi = pField->offset() + pField->Size();

	FieldPtr pFieldNx = pField;
	while (1)
	{
		pFieldNx = (FieldPtr )pFieldNx->N ext();
		if (!pFieldNx)
			break;
		if (pFieldNx->offset() > nOffsHi)
			break;
		int offs_hi = pFieldNx->offset() + pFieldNx->Size();
		if (offs_hi > nOffsHi)
			nOffsHi = offs_hi;
	}

	return nOffsHi;
}


int Field_t::CheckOverwrite(FieldPtr pField)
{
	FieldPtr p = this;
	while (p)
	{
		if (p->offset() > pField->offset())
			assert(false);
		if (p->offset() + p->Size() > pField->offset())
			return 1;
	}

	return 0;
}*/

int Field_t::CheckOverwrite(ADDR nOffset)
{
	assert(_key() != 0);
	if (_key() > nOffset)
		return 0;
	int nDelta = _key()+size() - nOffset;
	if (nDelta > 0)
		return nDelta;
	return 0;
}

bool Field_t::isCombined() const
{
	return false;
}

TypePtr Field_t::parentSeg()
{
	Obj_t *pObj(owner());
	while (pObj)
	{
		TypePtr iType(pObj->objTypeGlob());
		if (iType)
		{
			if (iType->typeSeg())
				return iType;
			pObj = iType->parentField();
		}
		else
		{
			FieldPtr pField(pObj->objField());
			assert(pField);
			pObj = pField->owner();
		}
	}
	return nullptr;
}

TypePtr Field_t::parentSegList() const
{
	CFieldPtr pSelf(this);
	while (pSelf)
	{
		TypePtr pCplx(pSelf->OwnerComplex());
		if (pCplx->typeSeg())
			return pCplx;
		pSelf = pCplx->parentField();
	}
	return nullptr;
}

/*TypeProc_t * Field_t::ownerProcPvt() const
{
	if (mpOwner)
	{
		if (mpOwner->typeProc())
			return mpOwner->typeProc();
	}
	return nullptr;
}*/

TypePtr Field_t::ownerProc() const
{
	if (mpOwner)
		if (mpOwner->typeProc())
			return (TypePtr)mpOwner;
	return nullptr;
}

FieldPtr Field_t::parentField()
{
	TypePtr pCplx(OwnerComplex());
	if (!pCplx)
		return nullptr;
	return pCplx->parentField();
}

bool IsLegalCName(const char *p, unsigned n)
{
	if (p)
	{
		if (isalpha(*p) || *p == '_' || *p == '~')//check the first symbol //check for destructor (FIX LATER)
		{
			for (++p; --n >= 0; ++p)
			{
				if (n == 0 || !(*p) || *p == DUB_SEPARATOR)//fix for dub names
					return true;//at least a single legal symbol
				if (!(isalnum(*p) || *p == '_'))
					break;
			}
		}
	}
	return false;
}

void Field_t::setName(PNameRef p)
{
CHECKID(this, 0xa8d)
STOP
	setName0(p);
	if (mpName)
	{
		int n;
		const char* pc(NameRef_t::skipLenPfx(mpName->c_str(), n));
		if (!IsLegalCName(pc, n))
			setUglyName();
	}
	else
		clearUglyName();
}
	

/*
bool Field_t::IsStatic()
{
	if (!IsDa ta())
		return false;
	if (!Get OwnerPath())
		return false;
	return (ppL ist == (Link_t **)&GetOwnerF unc()->Statics());
}*/

bool Field_t::isGlobal() const
{
	TypePtr iOwner(owner());
	while (iOwner)
	{
		if (iOwner->typeProc())
			return true;
		if (iOwner->typeSeg())
			return true;
		FieldPtr pField(iOwner->parentField());
		if (!pField)
			break;
		iOwner = pField->owner();
	}
	return false;//some struc's member
}


/*?int Field_t::ExpandStruc()
{assert(0);
	if (IsLast())
		return 0;
	if (!IsComplex0())
		return 0;
	if (type()->typeArray())//m_nArray > 0)
		return 0;

	FieldPtr pFieldNx = (FieldPtr )Ne xt();
	int nOffs = pFieldNx->offset() - offset();

	pFieldNx->Unlink();
	type()->typeStruc()->AddF ield(pFieldNx, nOffs);
	return 1;
}*/

// (!) Do not need maintain a reference list. Instead, go through hierarchy of 
// contained complex objects and check if no one contain the given structure

//check if pStruc is somehow owner of this field
//STRUC CAN'T CONTAIN ITSELF OR THOSE ONES THAT CONTAIN THIS STRUC!!!
/*int Field_t::CheckOwner(TypePtr pStruc)
{
	if (mpOwner == pStruc)
		return 1;
	
static int level = 0;
	if (level > 100)
		return 1;//antilock
	level++;

	//go through reference fields
	for (XRefObjList_t::Iterator i(OwnerComplex()->xr efs_()); i; i++)
	{
		Obj_t *pObj(i.data().pObj);
		if (pObj == this)
			continue;
		FieldPtr pField(pObj->objField());
		if (pField)
		{
			assert(pField->type() == mpOwner);
			if (!pField->OpType())//complex
				if (pField->CheckOwner(pStruc))
				{
					level--;
					return 1;
				}
		}
	}

level--;

	return 0;
}*/

#if(0)
int Field_t::ApplyArray(int32_t nSize)
{
	TYPE _t T(this);
	T.m_nArray = nSize;
	return ApplyType(T);

/*	FieldPtr pData = 0;
	if (!IsField())
		if (IsDataOp())
			pData = GetOwnerData();
		else if (!IsArgOp())
			assert(false);
		else
			return 1;//???

	if (nSize < 0)
		return 0;//don't change

	if (Size() < 0)
		return 0;

	if ((uint32_t)nSize == m_array)
		return -1;//already

	m_array = nSize;

	if (pData)
	{
		pData->KillOverlapped();
		pData->CheckXRefs();
	}

	return 1;*/
}
#endif

#if(0)
int Field_t::SetStruct(TypePtr pStruc)
{
	assert(pStruc);
	//assert(objField() || IsDataOp());
//CHECK(offset() == 4)
//STOP

	if (!pStruc)
	{
		uint8_t t = *(uint8_t *)&pStruc;
assert(0);//?		if (!AgreeTypes(m_optyp, t))
			return 0;
		return 1;
	}

	//?assert(G DC.iTypepStruc->ObjT ype() == OBJID_TYPE_STRUC);

	if (type())//?m_pStruc)
	{
//CHECK(pStruc->CompName("Struc@sub_4015E0@13"))
//STOP

		assert(isComplex());
		assert(pStruc);
		if (type() != pStruc)
			pStruc->typeStruc()->MergeWith(type());
		else
			return 1;//already
	}

//	assert(!pStruc->IsIncapsulatedWith(m_pOwner, 0));
//	if (!pStruc)
//		return 1;

	int nMaxSz = pStruc->typeStruc()->size();
//	assert(m_iOffset >= 0);

	if (type() && type() != pStruc)//?m_pStruc && (m_pStruc != pStruc))
		return 0;

	if (OpType())
	{assert(0);/*?
		FieldPtr pField = new Fi eld_t();
		pField->LinkAfter(this);
		pField->m_pOwnerStruc = m_pOwnerStruc;
		pField->setOffset( offset() );
		pField->SetOpType(OpType());*/
assert(0);//?		m_optyp = 0;
	}
	
	SetStruc0(pStruc);
	return 1;
}
#endif

#if(0)
int Field_t::SetStruct(Struc_t * pStruc)
{
	if ( !pStruc->typeStruc() )
		return 0;

	Type_t * pType = type();
	if ( pType )
	{
		if ( pType->typeArray() )
			pType = pType->typeArray()->type();
		assert( !pType->typeArray() );
	}

	TYPE _t T(this);

	if ( pType->typeStruc() )//already???
	{
		if (pType->typeStruc() != pStruc)
			pStruc->MergeWith( pType->typeStruc() );
		return 1;
	}
	
	if (T.m_nPtr)
		T.m_nPtr = 0;


/*	if (IsL ocal())//only locals
	{
		int retsz = ownerProcPvt()->m_pfDef->RetAddrSize();
		int nMaxSz = pStruc->Size();
		if (m_nOffset < 0)//var!
		{
			if (-m_nOffset < nMaxSz)
				return 0;//overlaps @return...
		}
		else if ((m_nOffset >= 0) && (m_nOffset < retsz))//return address?
		{
			return 0;
		}
		else if (m_nOffset >= retsz)//arg!
		{
			if (nMaxSz > 4)
				return 0;//structures pass to func as pointers to them!
		}

//		FieldPtr pData = this;
//		OpPtr pField = pData->m_pFields;
//		assert(pField->m_nOffset == 0);
	}*/

	SetStruc0(pStruc);
	SetOpType(OPTYP_NULL);

	return 1;
}

int Field_t::ApplyStruct(TypePtr pStruc)
{
	if (!pStruc)
		return 1;

	//?if (IS _PTR(pStruc))
	{
		if (pStruc->typeStruc()->Ob jType() != OBJID_TYPE_STRUC)
			return 0;

		if (CheckOwner(pStruc))
			return 0;
	}

	if (!SetStruct(pStruc))
		return 0;

	//?if (!IS _PTR(pStruc))
		return 1;

	KillOverlapped();
	return 1;
}
#endif

/*int Field_t::ApplyStruct(Struc_t *pStruc)
{
	assert(0);
//CHECK(CompName("dbl_22041DF0"))
//STOP
	if (!SetStruct(pStruc))
		return 0;

	if (!IsLo cal())
		return 0;//only for locals

	KillOverlapped();
	CheckXRefs();
	return 1;
}*/

#if(0)
int Field_t::ApplyType(mTyp E_t& T)
{
	FieldPtr pData = 0;
/*!	if (!IsField())
		if (IsDataOp())
			pData = GetOwnerData();
		else
			if (IsArgOp())
				pData = GetOwnerData();
			else if (!IsExitOp())
				return 0;*/

	int nSizePr = size();

/*	if (typ == 0xFF)
	{
		typ = 0;
		pStruc = (Struc_t *)0xFF;
	}*/

	if (T.mpStruc || T.mnPtr)
	{
		assert(!T.OpType() || T.isPtr());

/*		if ((int)pStruc == 0xFF)
		{
			AssureComplex(0);//create new struc
		}
		else*/
		if (T.mpStruc != type())
		{
			if (T.FuncDef())
				if (!T.OpType())
					return 0;

			setT ype(nullptr);//m_pStruc = 0;

			if (!T.OpType())//complex
			{
				if (objField())
				{
					if (!ApplyStruct(T.mpStruc))
						return 0;
				}
				else if (pData)
				{
					if (pData->IsLo alEx())
						return 0;
					if (!pData->ApplyStruct(T.mpStruc))
						return 0;
				}
				else 
					return 0;
			}
			else
			{
				SetOpType((OpType_t)T.OpType());
				if (T.Struc())
					SetStruc0(T.mpStruc);
				else
					setT ype(G DC.typeMgr()->getStockType(*(OpType_t *)&T.mnPtr));//m_nPtr = (int)T.m_pStruc;
			}
		}
	}
	else
	{
		setT ype( nullptr );
	}

//	if (typ || pStruc)
	setT ype(G DC.typeMgr()->getStockType((OpType_t)T.OpType()));//otherwise don't change


	if (T.m_nArray >= 0)
	{
		TypesTracer_t tt(*G DC.typeMgr());
		setTy pe(tt.arrayOf(type(), T.m_nArray) );
	}


	int nSize = size();
	if (pData)
	{
		if (nSizePr != nSize)
			pData->OnSizeChange(nSizePr);
		pData->KillOverlapped();
		pData->CheckXRefs();
	}

	return 1;
}
#endif

/*int Field_t::SetOffset(int nOffs)
{
	if (IsLoc al0())
	{
		if (IsCombined())
		{
			if (offset() != nOffs)
				return 0;
			return 1;
		}

		m_opc = StorageId();
		setOffset(nOffs);
		return 1;
	}

	return Field_t::SetOffset(nOffs);
}*/

/*int Field_t::SetOffset(int nOffset)
{
	if (offset() == offset)
		return 1;

	setOffset( nOffset );

	if (!IsLinked())
		return 1;

	while (!IsFirst())
	{
		FieldPtr pFieldPr = (FieldPtr )Pr ev();
		if (pFieldPr->offset() <= offset())
			break;
		Shift(-1);
	}
	while (!IsLast())
	{
		FieldPtr pFieldNx = (FieldPtr )Nex t();
		if (pFieldNx->offset() > offset())
			break;
		Shift(1);
	}
	return 1;
}*/

/*?Struc_t * Field_t::AssureComplex(Struc_t *pStruc)
{
	if (IsDataOp())
		return m_pOwnerData->AssureComplex(pStruc);

	if (IsComplex0())
		return m_pStruc->typeStruc();

	if (!pStruc)
		pStruc = G DC.Make Struct((OpPtr)0);

	if (m_optyp)
	{
		FieldPtr pField = pStruc->AddF ield(0, 2);
		pField->m_optyp = m_optyp;
		
		if (m_pStruc)
		{
			pField->SetStruc0(m_pStruc);
			m_pStruc->ReleaseOpRef(this);
		}
		m_optyp = 0;
	}
	
	ApplyStruct(pStruc);
	return pStruc;return nullptr;
}*/

/*ADDR Field_t::addressEx() const
{
	ADDR va(_key());
	TypeBasePtr pOwner(mpOwner);
	while (!pOwner->isShared())
	{
		FieldPtr pField(pOwner->parentField());
		if (!pField)
			break;
		va += pField->offset();
		pOwner = pField->owner();
	}
	return va;
}*/

ADDR Field_t::offset() const
{
	ADDR ownerBase(owner()->base());
	return _key() - ownerBase;
}

void Field_t::OnSizeIncrease()
{
/*?	if (IsDataOp())
		GetOwnerData()->OnSizeIncrease();
	else*/
		OwnerStruc()->OnSizeIncrease();
}

#if(0)
int Field_t::OnSizeIncrease()
{assert(0);


	if (IsL ocal())
	{
		return 0;
	}

	//global!
	if (offset() == 0)
		return 1;//not fixed

//	for (
	Struc_t *pSeg = GetOw nerSeg()->typeSeg();//G DC.m_pSeg_s; 
//	pSeg; 
//	pSeg = (Seg_t*)pSeg->Nex t())
	if (pSeg)//FIXME!
	{
		for (FieldMapIt it = pSeg->fields().begin(); it != pSeg->fields().begin(); it++)
		{
			FieldPtr  pMLoc = *it;
			if (!pMLoc->IsD ata())
				continue;

			Data _t *pData = (Data _t *)pMLoc;
			if (pData->offset() != 0)
			if (pData != this)
			if ( CheckOverwrite(pData->offset()) )
			{
assert(0);/*?				pData->LinkAfter(this);
				while (!pData->IsLast())
				{
					FieldPtr pDataNx = (FieldPtr )pData->Ne xt();
					if (pDataNx->offset() == 0)
						break;
					if (pDataNx->offset() > pData->offset())
						break;
					pData->Shift(1);
				}*/
			}
		}
	}
	return 1;
}
#endif

/*TypeProc_t *Field_t::IsExitLabel()
{
	if (IsLa bel())
	{
		FieldPtr pLabel = this;
		PathPtr pPath = pLabel->Path();
		if (pPath && pPath->Type() == BLK_EXIT)
		{
			TypeProc_t *pFunc = pPath->m.ownerProcPvt();
			return pFunc;
		}
	}

	return 0;
}*/

/*int Field_t::AdjustFilePos()
{
	if (ownerProcPvt())
		return 0;

	if (offset() == 0)
		return 1;//not fixed

	Struc_t *pSeg(GetOwnerSeg()->typeSeg()); 

	for (FieldMapIt it = pSeg->fields().begin(); it != pSeg->fields().end(); it++)
	{
		FieldPtr pMLoc = VALUE(it);
		if (pMLoc->IsData())
		{
			FieldPtr pData = (FieldPtr )pMLoc;
			if (pData->offset() != -1)
			if (pData != this)
			if (pData->CheckOverwrite(offset()))
			{
assert(0);/ *?				LinkAfter(pData);
				while (!IsLast())
				{
					FieldPtr pDataNx = (FieldPtr )Ne xt();
					if (pDataNx->offset() == -1)
						break;
					if (pDataNx->offset() > offset())
						break;
					Shift(1);
				}* /
				return 1;
			}
		}
	}

	return 0;
}*/

/*bool Field_t::IsCodeLoc()
{ 
	if (IsExitLabel())
		return false;
	return IsLabelEx(); 
}*/

static const std::pair<MyString, ObjId_t> g_pfx[] = {
	std::make_pair("sub_", OBJID_TYPE_PROC),
	std::make_pair("loc_", OBJID_TYPE_CODE),
	std::make_pair("byte_", OBJID_FIELD),
	std::make_pair("word_", OBJID_FIELD),
	std::make_pair("dword_", OBJID_FIELD),
	std::make_pair("qword_", OBJID_FIELD),
	std::make_pair("flt_", OBJID_FIELD),
	std::make_pair("dbl_", OBJID_FIELD),
	std::make_pair("asc_", OBJID_FIELD),
	std::make_pair("stru_", OBJID_TYPE_STRUC)
};

const char * Field_t::GetDefaultName(int type, uint32_t addr)
{
	assert(false);
	return 0;
}

ObjId_t	Field_t::IsDefaultName(const char *name, ADDR& addr)
{
	MyString s(name);
	for (int i(0); i < sizeof(g_pfx) / sizeof(std::pair<std::string, ObjId_t>); i++)
	{
		const std::pair<std::string, ObjId_t> &p(g_pfx[i]);
		if (s.startsWith(p.first))
		{
			value_t n;
			if (!StrHex2Int(s.mid((int)p.first.length()), n))
				return OBJID_NULL;
			addr = n.ui32;
			return p.second;
		}
	}

	return OBJID_NULL;
}

/*void Field_t::DUMP( std::ostream& os )
{
	os << "fld" << std::endl;
}*/

#if(0)
int Field_t::CountGotoXRefs()
{//how many xrefs to this label from statments like "goto thisLabel;"
	if (m_nFlags & LBL_UNREFD)
		return 0;

	if (g_ pDI && g_ pDI->isUnfoldMode())
		return 1;//always actual

	return Ops()->__checkOpenedGOTOs(true);

	int nCount = 0;
	for (XRef_t *pXRef = m_pXRefs; pXRef; pXRef = pXRef->N ext())
	{
		OpPtr pOp = pXRef->pOp;
		if ( !pOp->IsDataOp() )
		{
			assert(pOp->IsPr imeOp());
			if (pOp->IsGoto())
				nCount++;
		}
	}

	return nCount;
}
#endif

FieldPtr  Field_t::ConvertToFunc(bool bFar)
{assert(0);/*
	assert(IsL abel());

	if (IsExitLabel())
		return 0;

	TypeProc_t *pFuncOwner = ownerProcPvt();
	PathPtr pCallerBody = pFuncOwner->tailPath();

	TypeProc_t *pFunc = new TypeProc_t; 
	pFunc->m_pName = m_pName;
	m_pName = 0;
	if (bFar)
		pFunc->m_pfDef->m_nFlags |= FDEF_FAR;
	pFunc->LinkAfter(pFuncOwner);
	pFunc->ObjLinkBefore(pFuncOwner);
	pFunc->Assur eFuncDef();
	pFunc->createBodyPath();
	pFunc->CreateTailPath();

	//transfer paths
	PathPtr pPath = Path();
	PathPtr pPathPr = pPath->PrevEx();
	assert(pPathPr->Type() == BLK_JMP || pPathPr->m.Type() == BLK_JMPSWITCH);
#if(0)
	//divide list in two
	if (!pPath->IsFirst())
	{
		PathPtr pPathFirst = pPath->First();
		PathPtr pPathLast = pPath->Last();
		pPathFirst->pPrev = pPathPr;
		pPathPr->pNext = pPathFirst;
		pPathLast->pNext = pPath;
		pPath->pPrev = pPathLast;
	}

	while (1) 
	{
		PathPtr pPathP = pPath->Parent();
		if (!pPath->IsFirst())
			BlockIn(pPath);
		if (pPathP->IsBody())
			break;
		pPath = pPathP;
	}

	pPath->Redirect((Link_t **)&pFunc->Body()->m_pChilds);
#endif

	while (pPath)
	{
		if (pPath->Type() == BLK_EXIT)
			break;
		assert(pPath->Parent()->IsBody());
		PathPtr pPathNx = pPath->NextEx();

		pFunc->Ad dPath(pPath);
		Label_t *pLabel = pPath->Label();
		if (pLabel)
		{
			pFunc->fu ncdef()->AddP ath();
			SetPathLabel(pFunc, pLabel);
			pLabel->Obj Unlink();
			pFunc->AddField( pLabel );
		}
		if (pPath->IsRetPath())
			pPath->ChangeGotoPath(pFunc->tailPath());
		pPath = pPathNx;
	}

	Unlink();
	RedirectXDeps(pFunc);
	Delete();

	assert(!pFuncOwner->IsDecompiled());
	assert(pFuncOwner->m_pfDef);
	G DC.arrFuncDefs.Add(pFuncOwner->m_pfDef);

	GenerateStage_t * pBuilder = nullptr;//!G DC.m_pBuilder;
	if (pBuilder && pBuilder->m_pCurFunc == pFuncOwner)
		pBuilder->m_pCurFunc = pFunc;
	else
	{
		assert(pFunc->m_pfDef);
		G DC.arrFuncDefs.Add(pFunc->m_pfDef);
	}

	char buf1[NAMELENMAX];	
	pFuncOwner->na mex(buf1);
	char buf2[NAMELENMAX];
	pFunc->na mex(buf2);
	OUTPUT.printf("Function %s was splitted in two at label %s\n", buf1, buf2);

	return pFunc;*/return 0;
}

int Field_t::OnSizeChange(int nSizePr)
{assert(0);
/*
	int nSize = size();
	if (nSize == nSizePr)
		return 1;

	if (!IsLo cal0())
	{
		if (nSize > nSizePr)
			return OnSizeIncrease();
		return 1;
	}

	TypeProc_t *pFunc = ownerProcPvt();

	//regenerate out-of-range locals
	if (nSize > 0)
	{
		XRef_t *pXRef = m_pXRefs;
		while (pXRef)
		{
			XRef_t *pXRefNx = pXRef->Ne xt();
			OpPtr pOp = pXRef->Op();
			if (!IsOverlap(pOp->SSID(), pOp->Offset0() + pOp->CalcDispl(), pOp->OpSize()))
			{
				pOp->DetachMLoc();
				pFunc->SetChangeInfo(FUI_LOCALS);
			}
			pXRef = pXRefNx;
		}
	}

	if (pFunc->fun cdef()->IsDecompiled())//initial analisis is over
	{
		pFunc->SetChangeInfo(FUI_XDEPSINS|FUI_XOUTS|FUI_ROOTS);
	}
*/
	return 1;
}



#if(0)
/*int Field_t::KillSelf()
{
	FieldPtr pDest = 0;
	if (IsLo cal0())
	{
/ *		if (!IsFirst())
		{
			FieldPtr pDataPr = (FieldPtr )Pr ev();
			if (pDataPr->offset() == offset())
				pDest = pDataPr;
		}

		if (!pDest)
		if (!IsLast())
		{
			FieldPtr pDataNx = (FieldPtr )Ne xt();
			if (pDataNx->offset() == offset())
				pDest = pDataNx;
		}
		
		if (!pDest)
		{//try higher
			PathPtr pPath = Get OwnerPath();
			while (1)
			{
				pPath = pPath->Parent();
				if (!pPath)
					break;
				pDest = pPath->FindLocal(offset());
				if (pDest)
					break;
			}
		}

		if (!pDest)
		{
			//try rdirect to sibling
			if (!IsFirst())
				pDest = (FieldPtr )P rev();
			else if (!IsLast())
				pDest = (FieldPtr )N ext();
		} * /

		if (IsCombined())
			SeparateFromCodeOp();

		while (m_pXRefs)
			ReleaseXRef(m_pXRefs->pOp);

		GetOwnerFunc()->ownerProcPvt(FUI_LOCALS);
	}
	else
	{
		if (offset() != 0)
		if (!IsFirst())
		{
			pDest = (FieldPtr )Pr ev();
			if (pDest->offset() == 0)
				pDest = 0;
		}

		if (m_pXRefs)
		{
			if (!pDest)
				return 0;
			RedirectXDeps(pDest);
		}
	}

	assert(!m_pXRefs);
	delete this;
	return 1;
}*/
#endif


#define LSB1(x)	((x) & ~((x)-1))//Position of least significant bit that is set


bool Field_t::isStringOf(OpType_t optyp) const
{
	CTypePtr p(isTypeArray());
	if (!p)
		return false;
	const Array_t* pp(p->typeArray());
	return pp->baseType()->isTypeSimple(optyp);
	//return attrib() == ATTR_ ASCII || attrib() == ATTR_ASCII_TEXT;
}

void Field_t::setAttribute(AttrIdEnum a)
{
	assert(a < ATTR__MASK + 1);
	m_nFlags &= ~FLD_ATTRIB_MASK;
	m_nFlags |= ((a << FLD_ATTRIB_OFFS) & FLD_ATTRIB_MASK);
}

void Field_t::toggleAttribute(AttrIdEnum a)
{
	assert(a < 0x10);
	if (attrib() == a)
		setAttribute(ATTR_NULL);
	else
		setAttribute(a);
}

/*int Field_t::SetInverted(bool bSet)
{
	bool bOldState = (m_nFlags & FLD_INVERTED) != 0;
	m_nFlags &= ~FLD_INVERTED;

	if (!bSet)
	{
		if (!bOldState)
			return -1;//already
		return 1;
	}

	if (isStringOf())
		return 0;
	if (!(m_nFlags & FLD_CONST))
		return 0;

	m_nFlags |= FLD_INVERTED;
	if (bOldState)
		return -1;//already
	return 1;
}*/

/*bool TypesTracer_t::SetFieldType(FieldPtr pSelf, uint8_t optyp, TypePtr iStruc, int nPtr, int nArray)
{
	TypePtr pType(0);
	if (optyp != 0)
	{
		if (iStruc)
		{//ptr to struc!
			assert(OPTYP_IS_PTR((optyp));
			int l = PTRL EVEL(optyp);
			pType = iStruc;
			assert(pType->isShared());
			while (l--)
				pType = ptrOf(pType);
		}
		else if (nPtr)
		{//ptr to ordinal
			assert(OPTYP_IS_PTR((optyp));
			int l = PTRL EVEL(optyp);
			pType = GetStockType((OpType_t)nPtr);
			while (l--)
				pType = ptrOf(pType);
		}
		else
		{//just ordinal
				if (optyp & OPTYP _PTR)//ptr to void?
			//if (isPtr())//ptr to void?
			{
				int iPtrLevel(PTRL EVEL(optyp));
				assert(iPtrLevel > 0);

				//if (pField->type() && pField->type()->typePtr())
				//return;

				while (iPtrLevel--)
					pType = ptrOf(pType);

				assert(pType);

				/ *pType = GetStockType((OpType_t)(m_optyp&(~OPTYP _PTR)));
				TypesMgr_t *pTypesMgr(findTypeMgr(pType));

				while (iPtrLevel--)
				pType = pTypesMgr->ptrOf(pType);* /
			}
			else
			{
				pType = GetStockType((OpType_t)optyp);
				assert(pType);
				//pType = GetStockType((OpType_t)m_optyp);
			}
		}
	}
	else
	{
		if (iStruc && iStruc->typeStruc())
		{
			pType = iStruc;
		}
		else if (nPtr)
		{
			assert(0);
		}
	}

	if (nArray)
	{
		if (pType)
			pType = arrayOf(pType, nArray);
	}

	pSelf->set Type(pType);
	return true;
}*/

int Field_t::ExpandStruc()
{assert(0);/*?
	if (!isLocal())
		return 0;

	if (IsLast())
		return 0;*/
/*
	if ( !type() )
		return 0;

	if ( type()->typeArray() )
		return 0;

	if ( !type()->typeStruc() )
		return 0;

	Data _t *pLocalNx = 0;assert(0);//?(FieldPtr )N ext();
	FieldPtr pField = pLocalNx;
	if (!pField)
		return 0;

	int nOffset = pLocalNx->offset() - offset();

	type()->typeStruc()->AddField(pField, nOffset);
	if (!pLocalNx->nameless())
	{
		char buf[NAMELENMAX];
		pLocalNx->nam ex(buf);
		pField->Set Name(buf);
	}

	pLocalNx->RedirectXDeps2(this);
	delete pLocalNx;
*/
	return 1;
}


int Field_t::CombineWithCodeOp()
{assert(0);
/*	if (!IsLo cal0())
		return 0;

	if (m_pOp_->m_array)
		return 0;

	PathPtr  pPath0 = GetRes idePath();//lowerest path

	OpPtr pOp = 0;
	for (XRef_t * pXRef = m_pXRefs;
	pXRef;
	pXRef = pXRef->Ne xt())
	{
		PathPtr  pPath = pXRef->pOp->Path();
		do {
			if (pPath == pPath0)
				break;
			if (!pPath->IsFirst())
				break;
			pPath = pPath->Parent();
		} while (pPath);
			
		if (pPath == pPath0)
		{
			pOp = pXRef->pOp;
			break;
		}
	}

	if (!pOp)
		return 0;

	if (m_pOp_ == pOp)
		return -1;//allready

	if (pOp->IsIndirectB() || pOp->IsAddr())
		return 0;
	assert(pOp->SSID() == m_pOp_->SSID());
	if (pOp->Offset0() != m_pOp_->Offset0())
		return 0;
	if (pOp->Size() != m_pOp_->Size())
		return 0;
//	if (pOp->IsCallOutOp())
//		if (PrimeOp(pOp)->XOut()->CheckCount(1) != 0)
//			return 0;
	if (pOp->CheckFlagsUsage())
		return 0;

	*(uint8_t *)&m_nOffset = pOp->m_optyp;//save optyp
	pOp->m_optyp = m_pOp_->m_optyp;
	if (IS_ PTR(m_pOp_->m_pStruc))
	{
		Struc_t * pStruc = m_pOp_->m_pStruc;
		pStruc->ReleaseOpRef(m_pOp_);
		pStruc->Add OpRef(pOp);
	}
	else
	{
		assert(!pOp->m_nPtr);
		pOp->m_nPtr = m_pOp_->m_nPtr;
	}

	delete m_pOp_;
	m_pOp_ = (FieldPtr )pOp;*/
	return 1;
}

int Field_t::SeparateFromCodeOp()
{
/*!	if (!IsLo cal0())
		return 0;

	if (!IsCombined())
		return 0;

	FieldPtr pOp = this;

	//?AssureOp(pOp->m_optyp);
	SetOpType(pOp->m_optyp);
	pOp->CopyTo(this);
	m_p MLoc = 0;

	int offs = offset();
	uint8_t offs8 = *(uint8_t *)&offs;
	pOp->SetOpType(offs8);//restore old optyp
	if (!pOp->OpType())
		pOp->SetOpType(Field_t::OpType());

	setOffset( 0 );
	if (pOp->IsLo cal())
		setOffset( pOp->Offset0() );

	if (IS_ PTR(pOp->m_pStruc))
	{
		Type_t * pStruc = pOp->m_pStruc;
		pStruc->ReleaseOpRef(pOp);
		pStruc->Add OpRef(this);
	}
	else
	{
		m_nPtr = pOp->m_nPtr;
	}

	if (pOp->OpC() == OPC_FPUREG)
		mOffs += pOp->FpuIn();
*/
	return 1;
}


bool Field_t::is_strucvar() const
{
	if (mpOwner && mpOwner->typeStrucvar())
		return true;
	return false;
}

void Field_t::setAttributeFromId(AttrIdEnum attr)
{
	if (attr & __ATTR_TYPE_MASK)
		return;
	if (attr & ATTR_COLLAPSED)
		m_nFlags |= FLD_COLLAPSED;

	setAttribute(AttrIdEnum(attr & ATTR__MASK));
}

int Field_t::convertObjType(TypePtr pType)
{
	if ( pType == nullptr )
	{
		mpType = pType;
//		decompiled = 0;
	}
	else if ( mpType == nullptr )
	{
		mpType = pType;
	}
	else
	{
	}

	return 1;
}

TypePtr Field_t::isTypeSeg() const {
	return mpType && mpType->typeSeg() ? mpType : nullptr; }

TypePtr Field_t::isTypeProc() const {
	return (mpType && mpType->typeProc()) ? mpType : nullptr; }

/*TypePtr Field_t::isTypeVFTable() const {
	return (mpType && mpType->typeVFTable()) ? mpType : nullptr; }*/

TypePtr Field_t::isTypeStruc() const {
	return (mpType && mpType->typeStruc()) ? mpType : nullptr; }

TypePtr Field_t::isTypePtr() const {
	return (mpType && mpType->typePtr()) ? mpType : nullptr; }

TypePtr Field_t::isTypeImp() const {
	return (mpType && mpType->typeImp()) ? mpType : nullptr; }

TypePtr Field_t::isTypeExp() const {
	return (mpType && mpType->typeExp()) ? mpType : nullptr; }

TypePtr Field_t::isTypeCode() const {
	return (mpType && mpType->typeCode()) ? mpType : nullptr; }

TypePtr Field_t::isTypeThunk() const {
	return (mpType && mpType->typeThunk()) ? mpType : nullptr; }

TypePtr	Field_t::isTypeCodeEx() const {
	if (isTypeCode())
		return mpType;
	if (isTypeThunk())
		return mpType->typeThunk()->baseType();
	return nullptr;
}

TypePtr Field_t::isTypeArray() const {
	return mpType && mpType->typeArray() ? mpType : nullptr;
}

TypePtr Field_t::isTypeBitset() const {
	return mpType && mpType->typeBitset() ? mpType : nullptr;
}

bool Field_t::isData() const { 
	return mpType && !mpType->typeCode() && !mpType->typeStrucvar(); }

TypePtr Field_t::isTypeSimple() const {
	return (mpType && mpType->typeSimple()) ? mpType : nullptr; }

bool Field_t::isTypeSimple(OpType_t t) const {
	if (!type())
		return false;
	return type()->isTypeSimple(t);
}





