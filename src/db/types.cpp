#include "types.h"
#include "prefix.h"

#include "qx/MyStringList.h"
#include "type_module.h"
#include "main.h"
#include "type_proxy.h"
#include "types_mgr.h"

using namespace std;

///////////////////////////////////////////////////

static char digit2char(uint8_t i)
{
	i &= 0x0F;
	if (i < 0xA)
		return '0' + i;
	return 'A' + i - 0xA;
}

MyString TypeID2Str(OpType_t typeID)
{
	MyString s("$xx");
	s.at(1) = digit2char(typeID >> 4);
	s.at(2) = digit2char(typeID);
	return s;
}

/////////////////////////////////////////

Type_t::Type_t()
{
}

Type_t::~Type_t()
{
//	assert(mRefs == 0);
}

Type_t::Type_t(const Type_t &)
{
}

Type_t &Type_t::operator=(const Type_t &)
{
	return *this;
}

/*ObjFlagsType &Type_t::flags(){ return typ eobj()->m_nFlags; }
const ObjFlagsType &Type_t::flags() const { return typ eobj()->m_nFlags; }*/


void Type_t::aka(MakeAlias &buf) const
{
	buf.forTypeVoid();
}

/*ADDR Type_t::base()
{
	FieldPtr pField(parentField());
	if (!pField)
		return 0;
	return pField->_key();
}*/

///////////////////////////////////////// Bit_t

/*void Bit_t::aka(MakeAlias &) const
{
	strcpy(buf, "!");
}

void Bit_t::namex(MyString &s) const
{
	s = "!BIT";
}*/

///////////////////////////////////////// Simple_t

Simple_t::Simple_t(OpType_t typeID)
{
	mTypeID = typeID;
	assert(!OPTYP_IS_PTR(typeID));
}

void Simple_t::aka(MakeAlias &buf) const
{
	buf.forTypeSimple(mTypeID);
}

void Simple_t::namex(MyString &s) const
{
	s.append(OpTyp2Str(mTypeID, 0));
}


/////////////////////////////////////////// Complex_t

COMP_ID Complex_t::sIdSeed = 0;
#ifdef _DEBUG
COMP_ID Complex_t::sIdSeedNeg = 0;
#endif

Complex_t::Complex_t()
	: mpNamespace(nullptr),
	mID(sIdSeed++)
{
}

Complex_t::Complex_t(COMP_ID id)
	: mpNamespace(nullptr),
	mID(id)
{
}

Complex_t::~Complex_t()
{
	assert(nameless());
	assert(!mpNamespace);
}

Complex_t::Complex_t(const Complex_t& o)
{
}

Complex_t& Complex_t::operator=(const Complex_t& o)
{
	this->Type_t::operator=(o);
	return *this;
}

PNameRef Complex_t::name() const
{
	if (mpNamespace)
		return mpNamespace->name();
	return nullptr;
}

void Complex_t::moveFrom(Complex_t& o)
{
#if(NO_OBJ_ID)
	mID = o.mID;//important! type aliases are based on this
#endif
	mpNamespace = o.mpNamespace;
	o.mpNamespace = nullptr;
}

void Complex_t::deleteNamespace(MemoryMgr_t& rm)
{
	mpNamespace->clear(rm);
	delete mpNamespace;
	mpNamespace = nullptr;
}

NamesMgr_t* Complex_t::assureNamespace()
{
	if (!mpNamespace)
		mpNamespace = new NamesMgr_t;
	return mpNamespace;
}

void Complex_t::setName(PNameRef n)
{
	/*	if (!n)
		{
			delete mpNam espace;
			mpNam espace = nullptr;
			return;
		}

		if (!mpNa mespace)
		{
			mpNa mespace = new Name space_t(this, n);
			return;
		}*/
		/*if (mpNam espace->mpName && strcmp(mpNam espace->mpName->c_str(), "AFX_MSGMAP") == 0)
		{
			STOP
		}*/

	assert(mpNamespace);
	mpNamespace->setName(n);
}

void Complex_t::setNameRef(PNameRef p)
{
	assert(mpNamespace);
	mpNamespace->setNameRef(p);
}

int Complex_t::isValid() const
{
	/*?	for (XRef_t *pXRef = m_pXRefs; pXRef; pXRef = pXRef->N ext())
		{
			FieldPtr  pField = pXRef->pObj->objField();
			assert(0);
			//?if (pField)
			//?if (pField->type() != this)
			{
				assert(false);
				return 0;
			}
		}*/

	return 1;
}

void Complex_t::aka(MakeAlias &buf) const
{
	buf.forTypeComplex(mID);
}

///////////////////////////////////////// TypePtr_t


TypePtr_t::~TypePtr_t()
{
	assert(!mpBaseType);
}

void TypePtr_t::namex(MyString &s) const
{
	if (mpBaseType)
		mpBaseType->namex(s);
	else
		s = TypeID2Str(OPTYP_NULL);
	s.append("*");
}

int TypePtr_t::level()
{
	int l = 0;
	TypePtr_t * pPtr = this;
	do {
		l++;
		Type_t * pType = pPtr->pointee()->type();
		pPtr = pType->typePtr();
	} while (pPtr);
	return l;
}

void TypePtr_t::aka(MakeAlias &buf) const
{
	buf.forTypePtr(mpBaseType, size(nullptr));
}

///////////////////////////////////////// TypeImp_t

int TypeImp_t::size(CTypePtr) const
{
	return mpBaseType->size(); 
}

void TypeImp_t::namex(MyString& s) const
{
	s.append("__imp__");
}

Simple_t *TypeImp_t::typeSimple() const { return mpBaseType->typeSimple(); }
Struc_t* TypeImp_t::typeStruc() const { return mpBaseType->typeStruc(); }
Array_t* TypeImp_t::typeArray() const { return mpBaseType->typeArray(); }
TypePtr_t* TypeImp_t::typePtr() const { return mpBaseType->typePtr(); }

void TypeImp_t::aka(MakeAlias &buf) const { buf.forTypeImp(mpBaseType); }


///////////////////////////////////////// TypeExp_t

int TypeExp_t::size(CTypePtr) const
{
	return mpBaseType->size(); 
}

void TypeExp_t::namex(MyString& s) const
{
	s.append("__exp__");
}

Simple_t *TypeExp_t::typeSimple() const { return mpBaseType->typeSimple(); }
Struc_t* TypeExp_t::typeStruc() const { return mpBaseType->typeStruc(); }
Array_t* TypeExp_t::typeArray() const { return mpBaseType->typeArray(); }
TypePtr_t* TypeExp_t::typePtr() const { return mpBaseType->typePtr(); }

void TypeExp_t::aka(MakeAlias &buf) const { buf.forTypeExp(mpBaseType); }


////////////////////////////////////////////////////// TypeThisPtr_t

void TypeThisPtr_t::aka(MakeAlias& buf) const
{
	buf.forTypePtr(mpBaseType, size(nullptr));
}

///////////////////////////////////////////////////////////////////// TypeVPtr_t

void TypeVPtr_t::aka(MakeAlias& buf) const {
	buf.forTypeVPtr(typeID());
}

///////////////////////////////////////// TypeRef_t

void TypeRef_t::namex(MyString &s) const
{
	assert(mpBaseType);
	mpBaseType->namex(s);
	s.append("&");
}

void TypeRef_t::aka(MakeAlias &buf) const
{
	buf.forTypeRef(mpBaseType, size(nullptr));
}

///////////////////////////////////////// TypeRRef_t

void TypeRRef_t::namex(MyString &s) const
{
	assert(mpBaseType);
	mpBaseType->namex(s);
	s.append("&&");
}

void TypeRRef_t::aka(MakeAlias &buf) const
{
	buf.forTypeRRef(mpBaseType, size(nullptr));
}


///////////////////////////////////////// TypeArrayIndex_t

TypePtr TypeArrayIndex_t::setArrayRef(TypePtr p)
{
	assert(!p || (p->typeArray() && !mpArrayRef));
	mpArrayRef = p;
	return mpArrayRef;
}

TypePtr TypeArrayIndex_t::setIndexRef(TypePtr p)
{
	assert(!p || (p->typeEnum() && !mpIndexRef));
	mpIndexRef = p;
	return mpIndexRef;
}

int TypeArrayIndex_t::size(CTypePtr) const 
{
	return mpArrayRef->size();
}

void TypeArrayIndex_t::namex(MyString &s) const
{
	const Array_t &rArray(*mpArrayRef->typeArray());
	const TypeEnum_t &rEnum(*mpIndexRef->typeEnum());
	rArray.baseType()->namex(s);
	s.append(MyStringf("[%d|", rArray.total()));
	mpIndexRef->namex(s);
	s.append("]");
}

void TypeArrayIndex_t::aka(MakeAlias &buf) const
{
	buf.forTypeArrayIndex(mpArrayRef, mpIndexRef);
}

///////////////////////////////////////// TypeConst_t

TypeConst_t::TypeConst_t()
	: mpBaseType(nullptr)
{
}

TypeConst_t::TypeConst_t(TypePtr pType)
	: mpBaseType(pType)
{
}

TypeConst_t::~TypeConst_t()
{
	assert(!mpBaseType);
}

void TypeConst_t::aka(MakeAlias &buf) const
{
	buf.forTypeConst(mpBaseType);
}

void TypeConst_t::namex(MyString &s) const
{
	s.append("const ");
	mpBaseType->namex(s);
}

int TypeConst_t::size(CTypePtr) const
{
	return mpBaseType->size();
}

///////////////////////////////////////// TypePair_t

void TypePair_t::namex(MyString &s) const {

	left()->namex(s);
	s.append(",");
	right()->namex(s);
}

void TypePair_t::aka(MakeAlias &buf) const
{
	buf.forTypePair(mpLeft, mpRight);
}

///////////////////////////////////////// TypeFunc_t

void TypeFunc_t::namex(MyString &s) const {
	if (retVal())
		retVal()->namex(s);
	else
		s.append("void");
	s.append("(");
	if (args())
		args()->namex(s);
	s.append(")");
}

uint8_t TypeFunc_t::retValOpType() const
{
	if (!retVal() || !retVal()->typeSimple())
		return 0;
	return retVal()->typeSimple()->optype();
}

void TypeFunc_t::aka(MakeAlias &buf) const
{ 
	buf.forTypeFunc(left(), right(), mFlags);
}


///////////////////////////////////////// TypeEnum_t

/*PNameRef TypeEnum_t::name() const
{
	if (mpBaseType)
		return mpBaseType->name();
	return nullptr;
}*/

void TypeEnum_t::namex(MyString &s) const
{
	mpBaseType->namex(s);
	s.append("<");
	Simple_t::namex(s);
	s.append(">");
}

void TypeEnum_t::setBaseType(TypePtr p)
{
	assert(!p || !p->typeEnum());
	mpBaseType = p;
}

void TypeEnum_t::aka(MakeAlias& buf) const
{
	buf.forTypeEnum(typeID(), mpBaseType);
}

///////////////////////////////////////// Array_t

Array_t::Array_t(TypePtr pType, int total)
	: mpBaseType(nullptr)
{
	mTotal = total;
	assert(!pType);
	//setBaseType(pType);
}

Array_t::~Array_t()
{
	assert(!mpBaseType);
	//setBaseType(0);
}

int Array_t::size(CTypePtr) const
{
	int sz0 = mpBaseType->size();
	if (sz0 > 0)
	{
		if (mTotal > 0)
			return sz0 * total();
		return sz0;
	}
	return -1;
}

unsigned Array_t::total() const
{
	if (mTotal & ARRAY_BYTES_MASK)
	{
		int sz(mpBaseType->size());
		if (sz <= 0)
			sz = OPSZ_BYTE;
		return (mTotal & ~ARRAY_BYTES_MASK) / sz;//size in bytes
	}
	return mTotal;//elements num
}

TypePtr Array_t::absBaseType() const
{
	const Array_t * pTypeArray = this;
	for (;;)
	{
		TypePtr iType = pTypeArray->baseType();
		if (!iType)
			break;
		pTypeArray = iType->typeArray();
		if (!pTypeArray)
			return iType;
	}
	return nullptr;
}

void Array_t::namex(MyString &s) const
{
	MyString t;
	mpBaseType->namex(t);
	s.append(MyStringf("%s[%d]", t.c_str(), total()));
}

void Array_t::aka(MakeAlias& buf) const
{
	buf.forTypeArray(mpBaseType, mTotal);
}

///////////////////////////////////////////////
// VarArray_t

/*void VarArray_t::makeAlias(TypePtr baseType, char buf[NAMELENMAX])
{
	baseType->aka(buf);
	sprintf(buf+strlen(buf), "{}");//buffer overflow?
}

void VarArray_t::aka(MakeAlias &buf) const
{
	makeAlias(mpBaseType, buf);
}

template <typename T> size_t varr_size(void *pv)
{
	T *p0(reinterpret_cast<T *>(pv));
	T *p(p0);
	while (*p) p++;
	return ((p - p0) + 1) * sizeof(T);
}

size_t VarArray_t::size(PContext p) const
{
	/ *if (mKind == VARARRAY_ZTERM)
	{
		assert(mpBaseType->typeSimple());
		uint8_t opsz(mpBaseType->typeSimple()->size());
		switch (opsz)
		{
		case OPSZ_BYTE: return varr_size<char>(p);
		case OPSZ_WORD: return varr_size<short>(p);
		case OPSZ_DWORD: return varr_size<int>(p);
		}
	}* /
	assert(0);
	return 0;
}

template <typename T> size_t varr_total(void *pv)
{
	T *p(reinterpret_cast<T *>(pv));
	size_t n(0);
	while (p[n]) n++;
	return n + 1;
}

size_t VarArray_t::total(PContext p) const
{
	/ *if (mKind == VARARRAY_ZTERM)
	{
		assert(mpBaseType->typeSimple());
		uint8_t opsz(mpBaseType->typeSimple()->size());
		switch (opsz)
		{
		case OPSZ_BYTE: return varr_total<char>(p);
		case OPSZ_WORD: return varr_total<short>(p);
		case OPSZ_DWORD: return varr_total<int>(p);
		}
	}* /
	assert(0);
	return 0;
}*/


///////////////////////////////////////////////
// Typedef

COMP_ID Typedef_t::gID = 0;

Typedef_t::Typedef_t()
	: mpBaseType(0),
	mpName(0),
	mID(gID++)
{
}

Typedef_t::Typedef_t(TypePtr _iType)
	: mpBaseType(_iType), mpName(nullptr), mID(gID++)
{
}

Typedef_t::~Typedef_t()
{
	assert(!mpBaseType);
	//setBaseType(nullptr);
}

void Typedef_t::aka(MakeAlias &buf) const
{
	buf.forTypedef(mpBaseType, mID);
}

void Typedef_t::namex(MyString &s) const
{
	if (mpName)
		s.append(mpName->c_str());
	else
		s.append(MyStringf("typedef_%d", mID));
}

int Typedef_t::size(CTypePtr) const
{
	if (mpBaseType)
		return mpBaseType->size();
	return 0;
}

/*uint8_t Typedef_t::optype() const
{
	return mpBaseType->optype();
}*/

OpType_t ProjectInfo_t::PtrSizeOf(TypesMgr_t &rTypesMgr) const
{
	TypePtr iSeg(ProjectInfo_t::OwnerSeg(rTypesMgr.owner()));
	if (iSeg->typeSeg()->isLarge())
		return OPSZ_QWORD;
	return OPSZ_DWORD;
}

OpType_t ProjectInfo_t::PtrSizeOf(TypePtr iType) const
{
	TypePtr iSeg(ProjectInfo_t::OwnerSeg(iType));
	if (iSeg->typeSeg()->isLarge())
		return OPSZ_QWORD;
	return OPSZ_DWORD;
}

FilesMgr0_t &ProjectInfo_t::Files() const
{ 
	return mrProject.files();
}



