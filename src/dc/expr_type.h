#pragma once

#include "qx/MyString.h"
#include "shared/defs.h"
#include "globs.h"
#include "proj_ex.h"
#include "db/type_alias.h"

class FuncDef_t;
class Field_t;

class TypeObj_t;
class Struc_t;
class ExprInfoBase_t;
typedef	TypeObj_t *	TypePtr;
class Out_t;

#define	TPTR	uint16_t
#define MAKEPTR(level, mask)	( (((uint16_t)mask&0xFF)<<8) | ((uint16_t)level&0xFF) )
#define PTRLVL(tptr)			( (tptr)&0xFF )
MyString CallID2Str(const Out_t &);
MyString GlobID2Str(CGlobPtr);

namespace expr
{
	class GTypeSimple;
	class GTypeArray;
	class GTypePtrBase;
	class GTypePtr;
	class GTypeRef;
	class GTypeComplex;
	class GTypeTypedef;
	class GTypePair;
	class GTypeFunc;
	class GTypeGlob;
	class GTypeCallPtr;
	class GType;

	class IOut
	{
	public:
		virtual MyString OpType2Str(OpType_t) = 0;
		virtual MyString Space() = 0;
		virtual MyString ObjName2Str(CTypePtr) = 0;
		virtual MyString CallDef2Str(const Out_t *) = 0;
		virtual MyString Glob2Str(CGlobPtr) = 0;
		virtual void DumpCallDef2Str(std::ostream &, const Out_t *) = 0;
		virtual void DumpGlob2Str(std::ostream &, CGlobPtr) = 0;

		virtual void dump(std::ostream &, OpType_t) = 0;
		//virtual void dump(std::ostream &, FieldPtr ) = 0;
		virtual void dumpTypeRef(std::ostream &, CTypePtr) = 0;
		virtual void dumpTypeRef(std::ostream &, const GType*) = 0;
	};

	
	typedef TMakeAlias<const GType *>	MakeAlias2;


	//////////////////////////////////////// GType (progenitor of all G-types)
	class GType
	{
		const TypeObj0_t *mpFast;
	public:
		GType()
			: mpFast(0)
		{
		}
		void setFast(const TypeObj0_t * p){ 
			mpFast = p;
			//assert(!mpFast || !mpFast->type Proc());
		}
		const TypeObj0_t * fast() const { return mpFast; }
		virtual GTypeSimple* typeSimple() const { return nullptr; }
		virtual GTypeArray* typeArray() const { return nullptr; }
		virtual GTypePtrBase* typePtrBase() const { return nullptr; }
		virtual GTypePtr* typePtr() const { return nullptr; }
		virtual GTypeRef* typeRef() const { return nullptr; }
		virtual GTypeComplex* typeComplex() const { return nullptr; }
		virtual GTypeTypedef* typeTypedef() const { return nullptr; }
		virtual GTypePair* typePair() const { return nullptr; }
		virtual GTypeFunc* typeFunc() const { return nullptr; }
		virtual GTypeGlob* typeGlob() const { return nullptr; }
		virtual GTypeCallPtr* typeCallPtr() const { return nullptr; }

		virtual TypePtr typeObj() const { return nullptr; }
		virtual GType* baseType() const { return nullptr; }
		virtual unsigned size() const = 0;
		virtual Struc_t *typeStruc() const { return nullptr; }
		virtual FuncDef_t *typeFuncDef() const { return nullptr; }
		//virtual TypeUnion_t *typeUnion() const { return nullptr; }
		virtual MyString name(MyString, IOut &) const = 0;
		virtual void aka(MakeAlias2 &) const = 0;
		virtual void dump(std::ostream &os, MyString s0, IOut &rOut) const
		{
			//MyString s(name(s0, rOut));
			//os << s;
			os << "FUCK";
		}
		void dump_void(std::ostream &os, MyString suffix, IOut &rOut) const
		{
			//output_optype(0, 0);//void
			//os << "void";//?
			rOut.dump(os, OPTYP_NULL);
			os << suffix;
			/*if (pField)
			{
				bTab ? dumpTab() : dumpSpace();
				dumpField _local(pField);
			}*/
			return;
		}
		virtual bool isPrivate() const { return false; }
	};


	//////////////////////////////////////// GTypeSimple
	class GTypeSimple : public GType
	{
		OpType_t	mTypeID;
	public:
		GTypeSimple(OpType_t t)
			: mTypeID(t)
		{
		}
		OpType_t optype() const { return mTypeID; }
		virtual GTypeSimple *typeSimple() const { return const_cast<GTypeSimple *>(this); }
		virtual unsigned size() const { return (mTypeID & OPSZ_MASK); }
		virtual MyString name(MyString s, IOut &o) const {
			if (!s.empty())
				s.prepend(o.Space());
			s.prepend(o.OpType2Str(mTypeID));
			return s;
		}
		virtual void aka(MakeAlias2 &s) const {
			s.forTypeSimple(mTypeID);
			//s.append(TypeID2Str(mTypeID));
		}
		virtual void dump(std::ostream &os, MyString suffix, IOut &rOut) const
		{
			//output_optype(mTypeID, 0);
			rOut.dump(os, mTypeID);
			os << suffix;
			/*if (pField)
			{
				bTab ? dumpTab() : dumpSpace();
				dumpFie ld_local(pField);
			}*/
		}
		MyString name() const { return ""; }
	};


	//////////////////////////////////////// GTypeArray
	class GTypeArray : public GType
	{
		GType *mpBaseType;
		unsigned mTotal;
	public:
		GTypeArray(const GType *p, unsigned n)
			: mpBaseType((GType*)p),
			mTotal(n)
		{
			assert(mpBaseType);//?&& n);
			assert(mpBaseType != this);
		}
		virtual GType *baseType() const { return mpBaseType; }
		virtual GTypeArray *typeArray() const { return const_cast<GTypeArray *>(this); }
		virtual unsigned size() const { return (mpBaseType->size() * mTotal); }
		unsigned total() const { return mTotal; }
		virtual MyString name(MyString s, IOut &o) const
		{
			if (!s.empty())
			{
				s.prepend("(");
				s.append(")");
			}
			s = mpBaseType->name(s, o);
			s.append(MyStringf("[%d]", mTotal));
			return s;
		}
		virtual void aka(MakeAlias2 &buf) const
		{
			buf.forTypeArray(mpBaseType, mTotal);
			//mpBaseType->aka(s);
			//s.append(MyStringf("[%d]", mTotal));
		}
		virtual void dump(std::ostream &os, MyString suffix, IOut &rOut) const
		{
			if (mpBaseType)
				mpBaseType->dump(os, "", rOut);
			else
				dump_void(os, suffix, rOut);
			os << suffix;
				/*if (pField)
				{
				bTab ? dumpTab() : dumpSpace();
				dumpFi eld_local(pField);
				}*/
			os << "[" << mTotal << "]";
		}
	};


	//////////////////////////////////////// GTypePtrBase
	class GTypePtrBase : public GTypeSimple
	{
	public:
		GTypePtrBase(OpType_t t)
			: GTypeSimple(t)
		{
		}
		virtual GTypePtrBase *typePtrBase() const { return const_cast<GTypePtrBase *>(this); }
	protected:
		virtual char symb() const { return '*'; }
	};



	//////////////////////////////////////// GTypePtr
	class GTypePtr : public GTypePtrBase
	{
		GType *mpBaseType;
	public:
		GTypePtr(OpType_t t, const GType *p)
			: GTypePtrBase(t),
			mpBaseType((GType*)p)
		{
			assert(!mpBaseType || (mpBaseType != this));
		}
		virtual GType *baseType() const { return mpBaseType; }
		GType *pointee() const { return baseType(); }
		int level() const
		{
			int l(1);
			const GTypePtr *p(this);
			while (p->pointee() && p->pointee()->typePtr())
			{
				p = p->pointee()->typePtr();
				l++;
			}
			return l;
		}
		virtual bool isPrivate() const
		{
			if (mpBaseType)
				return mpBaseType->isPrivate();
			return false;
		}
		virtual GTypePtr *typePtr() const { return const_cast<GTypePtr *>(this); }
		virtual MyString name(MyString s, IOut &o) const {
			s.insert(0, 1, symb());
			if (mpBaseType)
				s = mpBaseType->name(s, o);
			else
				s.prepend(o.OpType2Str(OPTYP_NULL));
			return s;
		}
		virtual void aka(MakeAlias2 &s) const {
			s.forTypePtr(mpBaseType, size());
		}
		virtual void dump(std::ostream &os, MyString suffix, IOut &rOut) const
		{
			if (suffix.empty())
				suffix.append(" ");
			suffix += symb();
			if (mpBaseType)
			{
				mpBaseType->dump(os, suffix, rOut);
				//os << suffix;
			}
			else
				dump_void(os, suffix, rOut);
		}
	};


	//////////////////////////////////////// GTypeCallRef
	class GTypeRef : public GTypePtr
	{
	public:
		GTypeRef(OpType_t t, GType* p)
			: GTypePtr(t, p)
		{
			assert(!p->typeRef());
		}
		virtual GTypeRef* typeRef() const { return const_cast<GTypeRef*>(this); }
		virtual void aka(MakeAlias2& s) const {
			s.forTypeRef(baseType(), size());
		}
	protected:
		virtual char symb() const { return '&'; }
	};


	//////////////////////////////////////// GTypeCallPtr
	class GTypeCallPtr : public GTypePtrBase
	{
		const Out_t	&mrOutCall;
	public:
		GTypeCallPtr(OpType_t t, const Out_t &r)
			: GTypePtrBase(t),
			mrOutCall(r)
		{
			//assert(mpOutCall && mpOutCall->isCallLike());
		}
		virtual MyString name(MyString s, IOut &o) const {
			if (!s.empty())
				s.prepend(o.Space());
			s.prepend(o.CallDef2Str(&mrOutCall));
			return s;
		}
		virtual void aka(MakeAlias2 &s) const {
			s.append(CallID2Str(mrOutCall));
		}
		virtual void dump(std::ostream &os, MyString, IOut &o) const
		{
			o.DumpCallDef2Str(os, &mrOutCall);
		}
		virtual GTypeCallPtr* typeCallPtr() const { return const_cast<GTypeCallPtr*>(this); }
		Out_t* exprTerm() const { return (Out_t *)&mrOutCall; }
		/*virtual unsigned size() const {
			assert(0);
			return 0;
		}*/
	};


	//////////////////////////////////////// GTypeComplex
	class GTypeComplex : public GType
	{
		TypePtr	mpCplx;
	public:
		GTypeComplex(CTypePtr p)
			: mpCplx((TypePtr)p)
		{
			assert(mpCplx);
		}
		virtual TypePtr typeObj() const { return mpCplx; }
		virtual GTypeComplex *typeComplex() const { return const_cast<GTypeComplex *>(this); }
		virtual Struc_t *typeStruc() const { return mpCplx->typeStruc(); }
		virtual FuncDef_t *typeFuncDef() const { return mpCplx->typeFuncDef(); }
		virtual Struc_t *typeUnion() const { return mpCplx->typeUnion(); }
		virtual unsigned size() const { return mpCplx->size(); }
		virtual MyString name(MyString s, IOut &o) const {
			if (!s.empty())
				s.prepend(o.Space());
			s.prepend(o.ObjName2Str(mpCplx));
			return s;
		}
		virtual void aka(MakeAlias2 &a) const {
			::MakeAlias b;
			mpCplx->aka(b);
			a.append(b);
		}
		virtual bool isPrivate() const
		{
			if (!mpCplx->isShared())
				return true;
			return false;
		}
		virtual void dump(std::ostream &os, MyString suffix, IOut &rOut) const
		{
			if (!mpCplx->isShared() && mpCplx->typeFuncDef())
			{
				os << "FUNCDEF";
				//root().dumpFunctionDeclaration(pTypeCplx->typeFuncDef(), pField, suffix);// 0);
			}
			else
			{
				rOut.dumpTypeRef(os, mpCplx);
				os << suffix;
				/*if (pField)
				{
					bTab ? dumpTab() : dumpSpace();
					dumpF ield_local(pField);
				}*/
			}
		}
	};


	//////////////////////////////////////// GTypeTypedef
	class GTypeTypedef : public GType
	{
		GType* mpBaseType;
		TypePtr	mpCplx;
	public:
		GTypeTypedef(CTypePtr p, GType* q)
			: mpCplx((TypePtr)p),
			mpBaseType(q)
		{
			assert(mpCplx);
		}
		virtual TypePtr typeObj() const { return mpCplx; }
		virtual GTypeTypedef *typeTypedef() const { return const_cast<GTypeTypedef *>(this); }
		virtual GType *baseType() const { return mpBaseType; }
		virtual unsigned size() const { return mpCplx->size(); }//?
		virtual MyString name(MyString s, IOut &o) const {
			if (!s.empty())
				s.prepend(o.Space());
			s.prepend(o.ObjName2Str(mpCplx));
			return s;
		}
		virtual void aka(MakeAlias2 &a) const {
			::MakeAlias b;
			mpCplx->aka(b);
			a.append(b);
		}
		virtual bool isPrivate() const
		{
			if (!mpCplx->isShared())
				return true;
			return false;
		}
		virtual void dump(std::ostream &os, MyString suffix, IOut &rOut) const
		{
			rOut.dumpTypeRef(os, mpCplx);
			os << suffix;
		}
	};


	//////////////////////////////////////// GTypeGlob
	class GTypeGlob : public GTypePtr
	{
		GlobPtr	mpGlob;
	public:
		GTypeGlob(OpType_t t, CGlobPtr g)
			: GTypePtr(t, nullptr),
			mpGlob((GlobPtr)g)
		{
		}
		GlobPtr globref() const { return mpGlob; }
		virtual GTypeGlob *typeGlob() const { return const_cast<GTypeGlob *>(this); }
		virtual MyString name(MyString s, IOut &o) const {
			if (!s.empty())
				s.prepend(o.Space());
			s.prepend(o.Glob2Str(mpGlob));
			return s;
		}
		virtual void aka(MakeAlias2 &s) const {
			s.append(GlobID2Str(mpGlob));
		}
		virtual void dump(std::ostream &os, MyString s0, IOut &o) const
		{
			o.DumpGlob2Str(os, mpGlob);
		}
	};


	//////////////////////////////////////// GTypePair
	class GTypePair : public GType
	{
		const GType	*mpLeft;
		const GType	*mpRight;
	public:
		GTypePair(const GType *l, const GType *r)
			: mpLeft(l),
			mpRight(r)
		{
			//assert(mpLeft && mpRight);
		}
		const GType *left() const { return mpLeft; }
		const GType *right() const { return mpRight; }
		virtual GTypePair *typePair() const { return const_cast<GTypePair *>(this); }
		virtual unsigned size() const { assert(0); return 0; }
		virtual MyString name(MyString s, IOut &o) const {
			left()->name(s, o);
			s.append(",");
			right()->name(s, o);
			return s;
		}
		virtual void aka(MakeAlias2 &a) const {
			a.forTypePair(mpLeft, mpRight);
		}
		virtual void dump(std::ostream &os, MyString suffix, IOut &rOut) const {
			os << "pairtype";
		}
	};


	//////////////////////////////////////// GTypeFunc
	class GTypeFunc : public GTypePair
	{
		unsigned mFlags;
	public:
		GTypeFunc(const GType *r, const GType *a, unsigned f)
			: GTypePair(r, a),
			mFlags(f)
		{
		}

		const GType *retVal() const { return left(); }
		const GType *args() const { return right(); }
		unsigned flags() const { return mFlags; }
		
		virtual GTypeFunc *typeFunc() const { return const_cast<GTypeFunc *>(this); }

		//virtual unsigned size() const { return mpCplx->size(); }
		virtual MyString name(MyString s, IOut &o) const {
			if (retVal())
				retVal()->name(s, o);
			else
				s.append("void");
			s.append("(");
			if (args())
				args()->name(s, o);
			s.append(")");
			return s;
		}
		virtual void aka(MakeAlias2 &a) const {
			a.forTypeFunc(left(), right(), mFlags);
		}
		virtual void dump(std::ostream &os, MyString suffix, IOut &rOut) const {
			os << "functype";//?
		}
	};

	
	///////////////////////////////////////////////////////////
	//////////////////////////////////////// GTypeMgr
	class CTypeMgr : private std::map<MyString, const GType *>
	{
		typedef std::map<MyString, const GType *> CBase;
		typedef CBase::iterator CBaseIt;
		typedef CBase::const_iterator CBaseCIt;
		OpType_t mPtrSize;
	public:
		CTypeMgr(OpType_t ptrSize)
			: mPtrSize(ptrSize)
		{
		}
		~CTypeMgr()
		{
			clear();
		}
		void clear()
		{
			while (!empty())
			{
				const GType *pType(begin()->second);
				erase(begin());
				delete pType;
			}
		}
		GType *find(const GType *) const;
		bool contains(const GType *) const;
		OpType_t ptrSize() const { return mPtrSize; }

		GType *fromGlob(CGlobPtr) const;
		GType *fromImpField(CFieldPtr, CFieldPtr expField, bool skipModifier = false) const;
		GType *fromTypeObj(CTypePtr) const;
		GType *fromCall(const Out_t *) const;
		//TypePtr toTypeRef(const GType *) const;
		GType *fromOpType(OpType_t) const;
		GType *ptrOf(const GType *p) const;
		GType *derefOf(const GType *p) const;//p must be a pointer
		GType *arrayOf(const GType *p, unsigned total) const;
	private:
		MyString alias2(const GType *) const;
	};

}//namespace expr



class TYP_t
{
	expr::GType *mpType;
public:

	TYP_t();
	TYP_t(expr::GType *);
	TYP_t(CTypePtr, const ExprInfoBase_t *);

	TYP_t& operator=(const TYP_t&);
	operator bool() const { return !isNull(); }
	bool operator==(const TYP_t&) const;
	bool operator!=(const TYP_t&x) const { return !operator==(x); }
	operator expr::GType* () const { return mpType; }

	TypePtr asTypeRef() const;
	GlobPtr asGlobRef() const;
	TYP_t	ptrBase() const;
	TYP_t	arrayBase() const;
	unsigned arraySize() const;
	TYP_t	incumbent() const;
	TYP_t	stripTypedef() const;
	TYP_t	stripModifier() const;

	void clear(){ mpType = nullptr; }
	uint8_t optyp0() const;
	uint8_t optyp() const { return isSimple() ? optyp0() : 0; }
	uint8_t opsiz() const { return optyp0() & OPSZ_MASK; }
	int	 size() const;
	expr::GType *type() const { return mpType; }

	bool	isNull() const;
	bool	isArray() const;
	bool	isPtr() const;
	bool	isRef() const;
	bool	isTypedef() const;
	bool	isModifier() const;
	bool	isComplex() const;
	bool	isSimple() const;
	bool	isPrivate() const;
	bool	isFunc() const;
	bool	isCallPtr() const;
	bool	isGlob() const;
	bool	isReal() const;
	bool	isReal80() const { return isSimple() && (optyp0() == OPTYP_REAL80); }
	bool	isInt() const;
	bool	isUInt() const { return isInt() && OPTYP_IS_UINT(optyp()); }//unsigned

	bool	isImplicitCastOf(const TYP_t&typeFrom) const { return typeFrom.canPromoteTo(*this); }
	bool	canPromoteTo(const TYP_t&, bool bMaybeUnk = true, bool bStrict = false) const;
	//int		CombineWith(TYP_t& T, int delta);
	void	dump(std::ostream &, expr::IOut &) const;
};




