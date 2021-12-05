#pragma once

#include <iostream>
#include <strstream>
#include <algorithm>

#include "db/probe.h"
#include "db/types.h"
#include "globs.h"
#include "files_ex.h"
#include "info_dc.h"
#include "expr_cache.h"

class Out_t;
class FuncDef_t;
class Struc_t;
struct Dump_t;
class Obj_t;
class Display_t;
class FileDumper_t;

#define NO_SUBDUMPS	0

class DisplayStream : public std::strstream
{
	//std::strstream	*mpos;
	const char* mpBuf;
	long mSize;
	MyString m_name;
public:
	DisplayStream()
		: mpBuf(nullptr),
		mSize(0)
	{
		//mpos = new std::strstream();
	}
	~DisplayStream()
	{
		//delete mpBuf;
		freeze(false);
		//mpos->clear();
		//delete mpos;
	}
	void clear()
	{
		freeze(false);
		std::strstream& os(*this);
		os = std::strstream();
		mpBuf = nullptr;
	}
	long finalize()
	{
		mSize = (long)pcount();
		mpBuf = str();
		return mSize;
	}
	bool invalidate()
	{
		if (!mpBuf)
			return false;
		//delete mpBuf;
		freeze(false);
		mpBuf = nullptr;
		return true;
	}
	const char* buf() const { return mpBuf; }
	long size() const { return mSize; }
	std::strstream& os() { return *this; }

	MyString& name() { return m_name; }
	void setName(MyString s){ m_name = s; }
};




struct Dump_t
{
	Dump_t() : m_pos(0) {}
	Dump_t(int p) : m_pos(p) {}
	int		m_pos;
};

struct DumpContext_t
{
	CTypeBasePtr mpCplx;
	CFieldPtr mpDockField;
	DumpContext_t() : mpCplx(nullptr), mpDockField(nullptr) {}
	void clear() { mpCplx = nullptr, mpDockField = nullptr; }
	GlobPtr asFunc2() const {
		return (mpCplx && mpCplx->typeFuncDef()) ? (GlobPtr)mpCplx : nullptr;
	}
};

//////////////////////////////////////////////////////DumpChunk_t
class DumpChunk_t : public DisplayStream
{
	std::vector<Dump_t>	mDumps;
public:
	DumpChunk_t()
	{
	}
	~DumpChunk_t()
	{
	}
	//unsigned endsAt() const { return m_line + lines(); }
	//bool operator<(const DumpChunk_t& b) const { return m_line < b.m_line; }
	//bool operator<(DumpChunk_t *b) const { return m_line < b->m_line; }
	void add() {
		int pos = (int)pcount();
		mDumps.push_back(pos);
	}
	void clear() {
		DisplayStream::clear();
		mDumps.clear();
	}
	int pos(int l) const {
		return mDumps.at(l).m_pos;
	}
	const char* dataAt(unsigned line) const
	{
		if (!(line < lines()))
			return nullptr;
		//assert(line < lines());
		if (!buf())
			return nullptr;
		long aPos = pos(line);
		if (aPos < 0)
			return nullptr;
		return buf() + aPos;
	}
	unsigned lines() const {
		return (unsigned)mDumps.size();
	}
};


//////////////////////////////////////////////// dump_elt_t
struct dump_elt_t
{
	unsigned line_rel;//in parent
	unsigned line_abs;//absolute
	unsigned parent_index;
	unsigned indent;
	DumpChunk_t* pChunk;
	CTypeBasePtr pCplx;
	dump_elt_t()
		: line_rel(0), line_abs(0), parent_index(-1), indent(0), pChunk(nullptr), pCplx(nullptr)
	{
	}
	dump_elt_t(unsigned l_rel, unsigned l_abs, unsigned _indent, DumpChunk_t* _pChunk, CTypeBasePtr _pType)
		: line_rel(l_rel), line_abs(l_abs), parent_index(-1), indent(_indent), pChunk(_pChunk), pCplx(_pType)
	{
	}
	bool operator<(const dump_elt_t& o) const {
		return line_abs < o.line_abs;
	}
	size_t lines() const { return pChunk->lines(); }
	TypePtr func() const { return pCplx->typeFuncDef() ? (TypePtr)pCplx : nullptr; }
	TypePtr struc() const { return pCplx->typeStruc() ? (TypePtr)pCplx : nullptr; }
};


//////////////////////////////////////////////////// DumpInfo_t
class DumpInfo_t : protected std::vector<dump_elt_t>
{
	size_t	miDirtyChunk;
public:
	DumpInfo_t(bool bRoot)
		: miDirtyChunk(-1)
	{
		if (bRoot)
			pushRootItem();
	}

	~DumpInfo_t()
	{
		for (size_t i(0); i < size(); i++)
			delete at(i).pChunk;
	}

	void clear()
	{
		size_t j(size());
		for (size_t i = 0; i < j; i++)
			delete at(i).pChunk;
		std::vector<dump_elt_t>::clear();
		pushRootItem();
	}

	void pushRootItem()
	{
		assert(empty());
		push_back(dump_elt_t(0, 0, 0, new DumpChunk_t, nullptr));
	}

	template <typename _Fn>
	bool redumpDirtyChunk(DumpInfo_t* pold, _Fn& cb)//old dump info gets invalid after this operation
	{
		DumpInfo_t& old(*pold);
		reserve(old.size());
		assert(empty());

		//move unaffected chunks from a section above
		size_t iOld;//index into old chunks array
		for (iOld = 0; iOld < old.miDirtyChunk; iOld++)
		{
			push_back(old[iOld]);
			old[iOld].pChunk = nullptr;
		}

		//retrive a dirty container
		size_t iDirty(iOld);
		dump_elt_t& aOld(old[iDirty]);
		DumpChunk_t* pDirtyChunk(aOld.pChunk);
		unsigned lines_old(pDirtyChunk->lines());
		CTypeBasePtr pScope(aOld.pCplx);
		unsigned l_rel(aOld.line_rel);
		unsigned l_abs(aOld.line_abs);
		unsigned indent(aOld.indent);
		miDirtyChunk = aOld.parent_index;//must be set to parent

		//destroy the dirty chunk (and children)
		delete aOld.pChunk;
		aOld.pChunk = nullptr;
		for (++iOld; iOld < old.size(); iOld++)//start with the next item
		{
			if (!old.isChildOf(iOld, iDirty))
				break;
			delete old[iOld].pChunk;
			old[iOld].pChunk = nullptr;
		}

		//perform a redump here
		DumpChunk_t* pNewChunk(contBegin(pScope, l_rel, l_abs, indent));
		cb(pScope);
		contEnd(pNewChunk, true);

		//proceeed with the rest of unaffected chunks below a modification point
		//by the way, adjust absolute position within a root chunk with delta of lines
		int delta(int(pNewChunk->lines()) - int(lines_old));
		for (; iOld < old.size(); iOld++)
		{
			push_back(old[iOld]);
			back().line_abs += delta;
			old[iOld].pChunk = nullptr;
		}

		delete pold;
		return true;
	}

	bool isChildOf(size_t index, size_t parent) const {
		while (index != 0)
		{
			const dump_elt_t& a((*this)[index]);
			if (a.parent_index == parent)
				return true;
			index = a.parent_index;
		}
		return false;
	}

	void print(std::ostream& os) const//diagnostics
	{
		os << "[index] = {line_rel,line_abs,parent,lines,name}\n";
		for (size_t i(0); i < size(); i++)
		{
			const dump_elt_t& a(at(i));
			os << "[" << i << "] = {" 
				<< a.line_rel << ","
				<< a.line_abs << ","
				<< a.parent_index << ","
				<< a.lines() << ","
				<< a.pChunk->name()
				<< "}\n";
		}
		os.flush();
	}

	bool isEmpty() const {
		return empty();
	}
	DumpChunk_t& rootChunk() const {
		return *(*this)[0].pChunk;
	}

	std::strstream& os() { return rootChunk().os(); }

	bool	dirty() const {
		if (isEmpty())
			return true;
		return !rootChunk().buf() || (miDirtyChunk != -1);
	}

	DumpChunk_t* dirtyChunk() const
	{
		if (!(miDirtyChunk < size()))
			return nullptr;
		return at(miDirtyChunk).pChunk;
	}

	unsigned dirtyIndent() const {
		return at(miDirtyChunk).indent;
	}
	
	DumpChunk_t* contBegin(CTypeBasePtr pType, int indent)
	{
		unsigned l_rel((int)current().lines());//relative position in parent
		unsigned l_abs(linesTotal(0));
		if (miDirtyChunk != -1)
		{
			const dump_elt_t& a(at(miDirtyChunk));
			l_rel = a.pChunk->lines();
			//assert(l_rel == l_rel_);
			l_abs = linesTotal(a.pCplx);//at(a.parent_index)
			//assert(l_abs == l_abs_);
		}
		return contBegin(pType, l_rel, l_abs, indent);
	}

	DumpChunk_t* contBegin(CTypeBasePtr pType, unsigned l_rel, unsigned l_abs, int indent)
	{
		assert(pType || l_rel == 0);
		//if (dirtyChunk() != nullptr)
		//return false;//no nested chunks
		if (pType)
		{
			DumpChunk_t* pChunk(new DumpChunk_t);
			push_back(dump_elt_t(l_rel, l_abs, indent, pChunk, pType));
		}
		else
		{
			assert(size() == 1 && at(0).pChunk);
		}

		dump_elt_t& a(back());
		a.parent_index = (unsigned)miDirtyChunk;
		miDirtyChunk = size() - 1;
		return a.pChunk;
	}

	void contEnd(DumpChunk_t* pChunk, bool bFinal)
	{
		assert(pChunk);
		pChunk->os() << '\0';
		pChunk->finalize();

		miDirtyChunk = (*this)[miDirtyChunk].parent_index;//go back to parent
		if (bFinal)
			miDirtyChunk = -1;
	}

	const char* dataAtLine(unsigned line, DumpContext_t* pctx) const
	{
		dump_elt_t z(0, line, 0, nullptr, nullptr);
		std::vector<dump_elt_t>::const_iterator i(std::upper_bound(begin(), end(), z));
		if (i != begin())
		{
			i--;
			const dump_elt_t& c(*i);

			//size_t lo(c.line_abs);
			//size_t hi(lo + c.lines());

			//assert(lo <= line);

			size_t j(std::distance(cbegin(), i));

			size_t delta(0);
			//size_t ch_lines(0);//combined size of children
			for (;;)
			{
				const dump_elt_t& a(at(j));

				if (line < a.line_abs + a.lines() + delta)
				{
					DumpChunk_t* pChunk(at(j).pChunk);
					if (pctx)
					{
						pctx->mpCplx = at(j).pCplx;
						pctx->mpDockField = (pctx->mpCplx && pctx->mpCplx->typeFuncDef()) ? DcInfo_t::DockField((CGlobPtr)pctx->mpCplx) : nullptr;
					}
					return pChunk->dataAt(unsigned(line - a.line_abs - delta));
				}

				//ch_lines += a.lines();
				if (a.parent_index == -1)
					break;

				j = (size_t)a.parent_index;
				const dump_elt_t& b(at(j));

				size_t ch_sum((a.line_abs + a.lines()) - b.line_abs - a.line_rel);//sum of children
				delta += ch_sum;
				//lo = b.line_rel;
				//hi = lo + b.lines() + ch_lines;
			}

		}

		return rootChunk().dataAt(line);
	}

	bool invalidate(TypeBasePtr iCont)
	{
		miDirtyChunk = -1;
		if (iCont)//only a specified container needs to be updated
		{
			//find a chunk to re-dump when viewed
			for (miDirtyChunk = 0; miDirtyChunk < (int)size(); miDirtyChunk++)
			{
				const dump_elt_t& a(at(miDirtyChunk));
				if (a.pCplx == iCont)
					return true;
			}
			miDirtyChunk = -1;
			return false;
		}
		if (isEmpty())
			return false;
		return rootChunk().invalidate();
	}

	DumpChunk_t* pcurrent() const
	{
		size_t i(miDirtyChunk);
		while (i != -1)
		{
			const dump_elt_t& a(at(i));
			if (a.pChunk)
				return a.pChunk;
			i = a.parent_index;
		}
		return nullptr;
	}
	DumpChunk_t& current()
	{
		DumpChunk_t* pChunk(pcurrent());
		if (pChunk)
			return *pChunk;
		return rootChunk();
	}
	const DumpChunk_t& current() const
	{
		DumpChunk_t* pChunk(pcurrent());
		if (pChunk)
			return *pChunk;
		return rootChunk();
	}

	unsigned linesTotal(CTypeBasePtr pStartCont) const
	{
		if (isEmpty())
			return 0;
		
		size_t i_from(size() - 1);
		const dump_elt_t& a(at(i_from));//back()
		size_t lines(a.line_abs + a.lines());
		if (a.pCplx != pStartCont)
		{
			//for each parent (up to pStartCont), add up the extent beyond its last child
			unsigned child_lower(a.line_rel);
			size_t i(a.parent_index);
			while (i != -1)
			{
				const dump_elt_t& b(at(i));
				lines += (b.lines() - child_lower);
				if (b.pCplx == pStartCont)
					break;
				i = b.parent_index;
				child_lower = b.line_rel;
			}
		}

		return (unsigned)lines;
	}
};



//////////////////////////////////////////SyncObj_t
class SyncObj_t
{
protected:
	OpPtr mpOp;
	PathPtr mpPath;
	FieldPtr mpField;
	TypePtr mpType;
	GlobPtr mpGlob;
	TypeBasePtr mpContainer;
public:
	SyncObj_t()
		: mpOp(OpPtr()), mpPath(PathPtr()), mpField(nullptr), mpType(nullptr), mpGlob(nullptr), mpContainer(nullptr)
	{
	}
	SyncObj_t(const SyncObj_t& o)
		: mpOp(o.mpOp),
		mpPath(o.mpPath),
		mpField(o.mpField),
		mpType(o.mpType),
		mpGlob(o.mpGlob),
		mpContainer(o.mpContainer)
	{
	}
	void operator=(const SyncObj_t& o)
	{
		mpOp = o.mpOp;
		mpPath = o.mpPath;
		mpField = o.mpField;
		mpType = o.mpType;
		mpGlob = o.mpGlob;
		mpContainer = o.mpContainer;
	}
	operator bool() const
	{
		return (mpOp || mpPath || mpField || mpType);
	}
	OpPtr op() const { return mpOp; }
	void setOp(OpPtr p) { mpOp = p; }
	PathPtr path() const { return mpPath; }
	void setPath(PathPtr p) { mpPath = p; }
	void setField(CFieldPtr p) { mpField = (FieldPtr)p; }
	FieldPtr field() const { return mpField; }
	TypePtr type() const { return mpType; }
	void setGlob(CGlobPtr g) {
		mpGlob = (GlobPtr)g;
		setField(DcInfo_s::DockField(mpGlob));
	}
	GlobPtr glob() const { return mpGlob; }
	const ObjFlags_t* obj1() const
	{
		if (mpOp)
		{
			Op_t& rOp(*mpOp);
			return &rOp;
		}
		const ObjFlags_t* pObj;
		//if (!pObj)
		pObj = field();
		if (!pObj)
			pObj = type();
		return pObj;
	}
	bool checkObj() const
	{
		if (mpOp || field() || type())
			return true;
		return false;
	}
	TypeBasePtr cont() const { return (TypeBasePtr)mpContainer; }
	void setContainer(CTypeBasePtr p) { mpContainer = (TypeBasePtr)p; }
	void clear()
	{
		mpOp = OpPtr();
		mpPath = PathPtr();
		mpField = nullptr;
		mpType = nullptr;
		mpGlob = nullptr;
		mpContainer = nullptr;
	}
};


class Display_t
{
	adcui::UDispFlags	mFlags;
	//bool	mbHeader;
	int		miCurLine;
	bool	mbDraftMode;
	int		m_nComment;
#ifdef _DEBUG
	static bool gbDraftTest;
#endif
	static bool gbNoColorMode;
public:
	Display_t(adcui::UDispFlags flags, bool bDraftMode)
		: mFlags(flags),
		//mbHeader(bHeader),
		miCurLine(-1),
		mbDraftMode(bDraftMode),
		m_nComment(0)
	{
	}
public:
	virtual const std::ostream& os() const = 0;
	virtual std::ostream& os() = 0;
	virtual void newLine() = 0;

	virtual int controlCharsOnLine() const { return 0; }
	virtual void addExtraChars(int) {}
	virtual int currentPosInLine() /*const*/ { return -1/*never*/; };//to wrap text around
	virtual Out_t* cachedExpression(HOP) { return nullptr; }
	virtual void addCachedExpression(HOP, GlobPtr fdef, Out_t*, const std::set<ADDR>&) {}
	virtual void addProblem(HOP, ProblemInfo_t) {}
	virtual void pushColor(int colID) {}
	virtual void pushWString(const wchar_t*, uint16_t) {}
	virtual bool openScope(CTypeBasePtr, int indent) { return false; }
	virtual void closeScope(bool) {}
	virtual HOP syncOpLine() const { return HOP(); }
	virtual void clearg() {}
	virtual std::string asString() { return "?"; }
	virtual ExprCache_t& exprCache() = 0;
	virtual bool dumpSymOp(OpPtr) { return false; }
	virtual bool dumpSymPath(PathPtr) { return false; }
	virtual bool dumpSym(adcui::SYM_e, CFieldPtr) { return false; }
	virtual bool dumpSym(adcui::SYM_e, CTypePtr) { return false; }
	virtual bool dumpSym(adcui::SYM_e, CGlobPtr) { return false; }
	virtual bool dumpSym(adcui::SYM_e, CGlobPtr, int) { return false; }
	//virtual bool dumpExtSym(const ExpFieldInfo_t &){ return false; }
	virtual void setScopeName(MyString){}
public:
	bool draftMode() const { return mbDraftMode; }
	bool IsHeader() const { return mFlags.testL(adcui::DUMP_HEADER); }
	void setCurLine(int y) { miCurLine = y; }
	int curLine() const { return miCurLine; }

	void setFlags0(adcui::UDispFlags f) {
		mFlags = f;
	}
	void setFlags(adcui::UDispFlags f) {
		mFlags = f;
	}
	adcui::UDispFlags flags() const {
		return mFlags;
	}
	/*uint32_t flags1() const {
		return mFlags.l;
	}
	uint32_t flags2() const {
		return mFlags.h;
	}*/
	uint32_t testOpt1(uint32_t f) const {
		return mFlags.l & f;
	}
	uint32_t testOpt2(uint32_t f) const {
		return mFlags.h & f;
	}
	int IsOutputDead() const {
		int f(0);
		if (testOpt1(adcui::DUMP_DEADCODE))
		{
			f |= 1;
			if (testOpt1(adcui::DUMP_DEADLABELS))
				f |= 2;
		}
		return f;
	}
	bool showBlocks() const {
		return testOpt1(adcui::DUMP_BLOCKS) != 0;
	}
	bool isUnfoldMode() const {
		return testOpt1(adcui::DUMP_UNFOLD) != 0;
	}
	int commentLevel() const { return m_nComment; }
	void setCommentLevel(int n) { m_nComment = n; }
#ifdef _DEBUG
	static bool IsDraftTest() { return gbDraftTest; }
	static void EnableDraftTest(bool b) { gbDraftTest = b; }
#endif
	static bool IsNoColorMode() { return gbNoColorMode;	}
};



///////////////////////////////////////////DisplayStd_t (regular std streams/files)
class DisplayStd_t : public Display_t
{
	std::ostream& mos;
	ExprCache_t& mrExprCache;//no need to cache ops
public:
	DisplayStd_t(bool bHeader, bool bDraftMode, std::ostream& os, ExprCache_t& rExprCache)
		: Display_t(bHeader, bDraftMode),
		mos(os),
		mrExprCache(rExprCache)
	{
	}
protected:
	virtual const std::ostream& os() const { return mos; }
	virtual std::ostream& os() { return mos; }
	virtual void newLine() { mos << "\n"; }
	virtual ExprCache_t& exprCache() { return mrExprCache; }
	virtual void pushWString(const wchar_t* ws, uint16_t ulen)
	{
		std::string decodeWString0(std::istream &, uint16_t);
		std::string s((const char*)ws, ulen * sizeof(wchar_t));
		std::stringstream ss(s);
		mos << decodeWString0(ss, ulen);
	}
};


///////////////////////////////////////////DisplayUI_t (GUI)
class DisplayUI_t : public Display_t
{
	CFolderPtr	mpFile;
	DumpInfo_t* mpdi;
	HOP			mpSyncOpLine;
	SyncObj_t	mSeekObj;
	int			miSeekObjLine;
	int			mCtrlNum;//counts number of control characters written since beggining of a line
	long		mLineBegPos;//position of a current line in dump stream

	ExprCacheEx_t	mrExprCache;

public:
	DisplayUI_t(CFolderPtr, adcui::UDispFlags, bool bDraftMode, OpType_t ptrSize);
	~DisplayUI_t();

	virtual const std::ostream& os() const {
		return mpdi->current();
	}
	virtual std::ostream& os() {
		return mpdi->current();
	}

	template <typename _Fn>
	bool redumpDirtyChunk(_Fn& cb)
	{
		DumpInfo_t* pold(mpdi);
		mpdi = new DumpInfo_t(false);
		mpdi->redumpDirtyChunk(pold, cb);
		//pold should be deleted at this point
		return true;
	}

	virtual void pushColor(int colID);
	virtual void pushWString(const wchar_t*, uint16_t);
	virtual void newLine();

	virtual bool openScope(CTypeBasePtr pType, int indent)
	{
		return mpdi->contBegin(pType, indent) != nullptr;
	}

	void closeScope(bool bFinalize)
	{
		mpdi->contEnd(mpdi->dirtyChunk(), bFinalize);
	}

	virtual bool dumpSymOp(OpPtr);
	virtual bool dumpSymPath(PathPtr);
	virtual bool dumpSym(adcui::SYM_e, CFieldPtr);
	virtual bool dumpSym(adcui::SYM_e, CTypePtr);
	virtual bool dumpSym(adcui::SYM_e, CGlobPtr);
	virtual bool dumpSym(adcui::SYM_e, CGlobPtr, int);
	//virtual bool dumpExtSym(const ExpFieldInfo_t &);

	virtual int controlCharsOnLine() const {
		return mCtrlNum;
	}
	virtual void addExtraChars(int n) {
		mCtrlNum += n;
	}
	virtual int currentPosInLine() /*const*/ {
		return ((long)os().tellp() - mLineBegPos) - controlCharsOnLine();
	}

	virtual Out_t* cachedExpression(HOP pSelf) {
		return mrExprCache.findOpExpr(pSelf);
	}
	virtual void addCachedExpression(HOP pSelf, GlobPtr pCFunc, Out_t* pOut, const std::set<ADDR>& addresses) {
		mrExprCache.addOpExpr(pSelf, pCFunc, pOut, addresses);
	}
	virtual void addProblem(HOP pOp, ProblemInfo_t pi) {
		mrExprCache.addProblem(pOp, pi);
	}

protected:
	const DumpChunk_t& dos() const {
		return mpdi->current();
	}
	DumpChunk_t& dos() {
		return mpdi->current();
	}
public:
	//DumpChunk_t* dirtyChunk() const {
		//return mpdi->dirtyChunk();
	//}
	bool hasDirtyChunk() const {
		return mpdi->dirtyChunk() != nullptr;
	}
	unsigned dirtyIndent() const {
		assert(hasDirtyChunk());
		return mpdi->dirtyIndent();
	}
	bool	IsRedumpPending() const {
		return mpdi->dirty();
	}
	bool	InvalidateDump(TypeBasePtr iCont) {
		if (!mpdi->invalidate(iCont))
			return false;
		//mrExprCache.reset();
		return true;
	}
	long finalize() {
		mSeekObj.clear();
		return dos().finalize();
	}

	virtual std::string asString() {
		return std::string(dos().buf(), dos().size());
	}
	std::string finalizeAsString() {
		finalize();
		return asString();
	}

	virtual void clearg() {
		mpdi->clear();
	}

	virtual ExprCache_t& exprCache() { return mrExprCache; }
	virtual HOP syncOpLine() const {
		return mpSyncOpLine;
	}
	void setSyncOpLine(HOP pOp) {
		mpSyncOpLine = pOp;
	}
	SyncObj_t& setSeekObj(const SyncObj_t& a)
	{
		mSeekObj = a;
		return mSeekObj;
	}
	void checkSeekOp(HOP pOp)
	{
		if (mSeekObj.op() == pOp)
		{
			miSeekObjLine = (int)mpdi->linesTotal(nullptr) - 1;
			mSeekObj.clear();
		}
	}

	int seekObjLine() const { return miSeekObjLine; }
	void clearSeekLine() { miSeekObjLine = -1; }

	const Folder_t* file() const { return mpFile; }
	//int		FindDumpInfo(Dump_t *pDumpSrc) const;
	//int		CheckHitObj(Obj_t *pObj) const;//0:no_hit,1:direct_hit,2:indirect_hit
	//bool	GetPosAtLine(int, long &, TypePtr * = nullptr) const;
	const char* GetDataAtLine(int, DumpContext_t* = nullptr) const;
	ExprCacheEx_t& exprCacheEx() { return mrExprCache; }

	int		linesInChunk() const { return (int)dos().lines(); }
	int		linesTotal() const { return (int)mpdi->linesTotal(nullptr); }

	void	ClearX(bool bAll);

	void	AddDump() { dos().add(); }

	ProblemInfo_t	problemWith(CHOP) const;
	int checkAddress(CHOP, ADDR) const;
	void print(std::ostream& os) {
		mpdi->print(os);
	}
	virtual void setScopeName(MyString s){
		if (hasDirtyChunk())
			mpdi->dirtyChunk()->setName(s);
	}
};



///////////////////////////////////////////DisplayLine_t (for line expansin)
class DisplayLine_t : public Display_t
{
	DisplayStream m_dos;
	ExprCache_t& mrExprCache;
	HOP			mpSyncOpLine;
public:
	DisplayLine_t(ExprCache_t& rExprCache)
		: Display_t(false, false),
		mrExprCache(rExprCache),
		mpSyncOpLine(HOP())
	{
	}
	virtual HOP syncOpLine() const { return mpSyncOpLine; }
	void setSyncOpLine(HOP pOp) { mpSyncOpLine = pOp; }
	std::string finalizeAsString() {
		//mSeekObj.clear();
		m_dos.finalize();
		return asString();
	}
	//protected:
	virtual void clearg() {
		m_dos.clear();
	}
	virtual const std::ostream& os() const { return m_dos; }
	virtual std::ostream& os() { return m_dos; }
	virtual void newLine() { assert(0); }
	virtual ExprCache_t& exprCache() { return mrExprCache; }
	virtual std::string asString() {
		//if (m_dos.buf())
			return std::string(m_dos.buf(), m_dos.size());
		//return "";
	}
	/*virtual void pushColor(int colID)
	{
		char c = (char)adcui::SYM_COLOR;
		char d = (char)colID;
		os() << c << d;
		addExtraChars(2);
	}*/
};



///////////////////////////////////////////DisplayLineUI_t (for line expansin in GUI)
class DisplayLineUI0_t : public DisplayLine_t
{
	int			mCtrlNum;//counts number of control characters written since beggining of a line
public:
	DisplayLineUI0_t(DisplayUI_t& disp0)//ExprCacheEx_t &rExprCache)
		: DisplayLine_t(disp0.exprCacheEx()),
		mCtrlNum(0)
	{
		adcui::UDispFlags f(disp0.flags());
		//if (bPlain)
//			f &= ~(adcui::DUMP_FONTS | adcui::DUMP_COLORS | adcui::DUMP_TABS);
		setFlags(f);
		//?	SetCurObj(curObj());
		setSyncOpLine(disp0.syncOpLine());

		//mpProbeSrc->resetDelta();
		assert(disp0.controlCharsOnLine() == 0);//fix later
	}
protected:
	virtual Out_t* cachedExpression(HOP pSelf) {
		return exprCacheEx().findOpExpr(pSelf);
	}
	virtual void addCachedExpression(HOP pSelf, GlobPtr pCFunc, Out_t* pOut, const std::set<ADDR>& addresses) {
		exprCacheEx().addOpExpr(pSelf, pCFunc, pOut, addresses);
	}
	virtual void addProblem(HOP pOp, ProblemInfo_t pi) {
		exprCacheEx().addProblem(pOp, pi);
	}
	virtual int controlCharsOnLine() const {
		return mCtrlNum;
	}
	virtual void addExtraChars(int n) {
		mCtrlNum += n;
	}
	virtual int currentPosInLine() /*const*/ {
		assert(0);
		return 0;
	}
	virtual void pushWString(const wchar_t* ws, uint16_t ulen){
		os().write((char*)ws, ulen * sizeof(wchar_t));
		mCtrlNum += ulen;
	}
private:
	ExprCacheEx_t& exprCacheEx() { return reinterpret_cast<ExprCacheEx_t&>(exprCache()); }
};



///////////////////////////////////////////DisplayLineUI_t (for line expansin in GUI with COLORS)
class DisplayLineUI_t : public DisplayLineUI0_t
{
public:
	DisplayLineUI_t(DisplayUI_t& disp0)
		: DisplayLineUI0_t(disp0)
	{
	}
protected:
	virtual void pushColor(int colID)
	{
		char c = (char)adcui::SYM_COLOR;
		char d = (char)colID;
		os() << c << d;
		addExtraChars(2);
	}
	virtual void pushWString(const wchar_t* ws, uint16_t ulen)
	{
		char c = (char)adcui::SYM_WSTRING;
		os() << c;
		addExtraChars(1);

		os().write((const char*)&ulen, sizeof(ulen));
		addExtraChars(sizeof(ulen));
		os().write((const char*)ws, ulen * sizeof(wchar_t));
		addExtraChars(ulen);
	}
};



///////////////////////////////////////////DisplayField_t (for line expansin)
class DisplayField_t : public Display_t
{
	DisplayStream m_dos;
public:
	DisplayField_t(bool bHeader)
		: Display_t(bHeader, false)
	{
	}
	std::string finalizeAsString() {
		//mSeekObj.clear();
		m_dos.finalize();
		return asString();
	}
protected:
	virtual const std::ostream& os() const { return m_dos; }
	virtual std::ostream& os() { return m_dos; }
	virtual void newLine() { assert(0); }
	virtual ExprCache_t& exprCache() {
		assert(0);
		static ExprCache_t dummy(OPTYP_NULL);
		return dummy;
	}
	virtual std::string asString() {
		return std::string(m_dos.buf(), m_dos.size());
	}
};


void dispflags_usage(std::ostream& os);
unsigned dispflag_check(const char* opt);
char* dispflags2str(unsigned f, const char* sep);



