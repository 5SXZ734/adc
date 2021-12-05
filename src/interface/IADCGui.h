#ifndef __IADCGUI_H__
#define __IADCGUI_H__

#include <cstddef>
#include <stdint.h>
#include "qx/IGui.h"

class MyStreamBase;

#define PRIMARY_EXT		"adc"
#define SECONDARY_EXT	"dc"
#define SCRIPT_EXT		"s"
#define RESOURCE_EXT	".RESOURCES"//".<rc>"
#define TYPES_EXT		".TYPES"//".<t>"
#define NAMES_EXT		".NAMES"//".<n>"
#define STUBS_EXT		".PROTOTYPES"
#define TEMPLATES_EXT	".ALIASES"//".<tt>"
#define EXPORTS_EXT		".EXPORTS"//".<ex>"
#define IMPORTS_EXT		".IMPORTS"//".<imp>"
#define SOURCE_C_EXT	".c"
#define SOURCE_EXT		".cpp"	//implementation file extention
#define HEADER_EXT		".h"	//header file extention
#define UNFOLD_EXT		".l"
#define EXPORT_FILE_EXT	".export"

#define FOLDER_SEP		"/"
#define MODULE_SEP		"!"//":"

#define	ATTIC_NAME		":attic" MODULE_SEP

namespace adcui
{
	enum LogColorEnum//for output log
	{
		COLORTAG_OFF,
		COLORTAG_RED,
		COLORTAG_ORANGE,
		COLORTAG_DARKRED,
		COLORTAG_DARKBLUE,
		COLORTAG_DARKGREEN,
		COLORTAG_HYPERLINK,
		COLORTAG_HYPERLINK_OFF
	};

	enum Color_t//careful! must not exceed 127
	{
		COLOR_NULL,//restore original color back //COLOR_DEFAULT
		COLOR_POP,		//pop back previous color
		COLOR_NAME,
		COLOR_ARRAY,
		COLOR_ERROR,
		////////////////////these must be consecutive and match ids in TreeInfo_t
		COLOR_TREE,
		COLOR_TREE_FUNC_DECOMPILED,
		COLOR_TREE_FUNC_BEING_DECOMPILED,
		COLOR_TREE_SHARED_TYPE,
		COLOR_TREE_STRUCVAR_TYPE,
		//disassembly
		COLOR_DASM_BGND,
		COLOR_DASM_BGND_EX,		//decompiled code background
		COLOR_DASM_ADDR_CODE,
		COLOR_DASM_ADDR_DATA,
		COLOR_DASM_ADDR_FUNC,
		COLOR_DASM_ADDR_UNK,
		COLOR_DASM_CODE,
		COLOR_DASM_ENDP,
		COLOR_DASM_ENDP2,
		COLOR_DASM_NUMBER,
		//PE specific
		//COLOR_ATTR__MISC,
		COLOR_ATTRIB_BEGIN,
		//COLOR_DASM_RVA = COLOR_ATTR__MISC,//offset from image base
		//COLOR_DASM_OFFS,//offset from current segment
		//COLOR_DASM_RELOC,//relocation type and offset from block
		COLOR_ATTRIB_END = COLOR_ATTRIB_BEGIN + 0x10,
		//end of PE specific
		COLOR_DASM_ASCII,
		COLOR_WSTRING,
		COLOR_DASM_UNK,
		COLOR_DASM_DATA_NAME,
		COLOR_DASM_CODE_NAME,
		COLOR_DASM_UNK_NAME,
		COLOR_DASM_DIRECTIVE,
		COLOR_DASM_ADDRESS,
		COLOR_DASM_BYTES,
		COLOR_UNKNOWN,
		COLOR_DASM_COMMENT,
		COLOR_DASM_TYPE,
		COLOR_MARGIN_ERROR,
		/////////////
		COLOR_DASM_PROBE,
		COLOR_DASM_SEL,
		COLOR_DASM_SEL_AUX,
		//source
		COLOR_KEYWORD,
		COLOR_KEYWORD_EX,
		COLOR_PREPROCESSOR,
		COLOR_USER_FUNCTION,
		COLOR_IMPORT_REF,
		COLOR_EXPORT_REF,
		COLOR_EXPORTED,
		COLOR_COMMENT,
		COLOR_UNEXPLORED,
		COLOR_UNNAMED,
		COLOR_UNANALIZED,
		COLOR_DEAD = COLOR_UNANALIZED,
		COLOR_TYPES,
		COLOR_XOUTS,
		COLOR_XINS,
		COLOR_XDEPS,
		COLOR_CPUREGS,
		COLOR_FPUREGS,
		COLOR_STRING,
		COLOR_DUP_SUFFIX,
		//common
		COLOR_CUR,
		COLOR_CUR_EXPR,
		COLOR_CUR_EDIT,
		COLOR_SEL,
		COLOR_SELAUX,
		COLOR_TASK_TOP,
		COLOR_CUR_STUB,
		COLOR_CALL_BREAK,
		COLOR_FLOW_BREAK,
		COLOR_FLOW_SPLIT,
		COLOR_SQUIGGLE_RED,
		COLOR_SQUIGGLE_GREEN,
		COLOR_TAG_INTRINSIC,
		COLOR_TAG_STUB,
		COLOR_TAG_NOPROTO,
		COLOR_TAG_PROCESSING,
		COLOR_TAG_ERROR,

/*		COLOR__TERM__BEGIN,
		COLOR_TERM_DGREY = COLOR__TERM__BEGIN,
		COLOR_TERM_GREY,
		COLOR_TERM_LGREY,
		COLOR_TAG_ERROR,
		COLOR_TERM_GREEN,
		COLOR_TERM_BLUE,
		COLOR_TERM_CYAN,
		COLOR_TERM_MAGENTA,
		COLOR_TERM_YELLOW,
		COLOR_TERM_DRED,
		COLOR_TERM_DGREEN,
		COLOR_TERM_DBLUE,
		COLOR_TERM_DCYAN,
		COLOR_TERM_DMAGENTA,
		COLOR_TERM_DYELLOW,
		COLOR__TERM__END,//!
		COLOR_TERM_ORANGE = COLOR__TERM__END,*/

		COLOR_FONT_BOLD,
		COLOR_FONT_ITALIC,
		COLOR_FONT_UNDERLINE,
		//..
		COLOR__TOTAL
	};

#define MAKECOLORID(arg)	(0x100-unsigned(arg))
#define FROMCOLORID(arg)	((adcui::Color_t)(MAKECOLORID(arg)&0xFF))//revert

	enum SYM_e
	{
		SYM_NULL,
		SYM_COLOR,
		SYM_FONT,
		SYM_WSTRING,	//length-prefixed
	
		SYM_TAB			= 0x09,	//'\t'
		SYM_LINEFEED	= 0x0A,	//'\n'
		SYM_RET			= 0x10,	//'\n'
		SYM_SPACE		= 0x20,	//' '

		__SYM__TAGS		= 0x80,
		SYM_FUNCDECL	= __SYM__TAGS,
		SYM_FUNCDEF,
		SYM_OPREF,
		SYM_PATHREF,	//labels
		SYM_FLDDECL,		//declaration including type and attributes
		SYM_FLDDECL0,		//declaration, name only
		SYM_FLDDEF,		//definition
		SYM_GLBDECL,
		SYM_GLBDEF,
		//SYM_FLDINST,//?
		SYM_FLDREF,
		SYM_CONSTREF,
		SYM_GAP,
		SYM_STRUCEND,	//end of structure
//		SYM_LABELDECL,
		SYM_TYPEREF,
		SYM_TYPEDEF,	//ugly names
		SYM_STUBINFO,	//0x0b
		SYM_IMPFLDREF,	//imported field reference
		SYM_IMPGLB,		//imported field declaration
		SYM_IMPCLSGLB,	//imported class method field
		SYM_IMPTYPEDEF,	//declaration/definition of imported type (struc)
		SYM_IMPVTBLDECL,
		//SYM_IMPTYPEREF,	//imported type (via proxy)
//		SYM_EXTCLSFLD,	//external class method field
		SYM_VTBLDECL,
		SYM_VFUNCDECL,
		__SYM__LAST
	};

	enum TreeInfo_t
	{
		ITEM_HAS_CHILD_HERE	= 0x01,
		ITEM_HAS_PARENT		= 0x02,
		ITEM_HAS_NEXT_CHILD	= 0x04,
		ITEM_CLOSED			= 0x08,
		//ITEM_UPPER_BOUND	= 0x10,
		//ITEM_TREE_MASK		= 0x1F,//5 bit
		ITEM_TREE_MASK		= 0x0F,//4 bits
		///////tree color encoding (see Color_t)
		ITEM_FUNC_DECOMPILED	= 0x20,
		ITEM_FUNC_DECOMPILING	= 0x40,
		ITEM_CONT_SHARED		= 0x60,
		ITEM_CONT_STRUCVAR		= 0x80,
		ITEM__1					= 0xA0,//3 more available? WARNING: elements are '0'-biased!
		ITEM__2					= 0xC0,
		ITEM__3					= 0xE0
	};

	enum CellFlagsEnum
	{
		CLMNFLAGS_HEADER	= 0x01,//,	//display contents on left header
		CLMNFLAGS_TREE		= 0x02,
		CLMNFLAGS_TRIM		= 0x04,
		CLMNFLAGS_DISPSEL	= 0x08,	//display selected row
		CLMNFLAGS_WRAP		= 0x10	//wrap column's data down to below rows
	};

	enum DUMP_e : unsigned
	{
		DUMP_NULL		= 0,
		DUMP_LNUMS		= 0x00000001L,//line numbers
		DUMP_STACKTOP	= 0x00000002L,//esp-tracking stack top
		DUMP_FPUTOP		= 0x00000004L,//FPU stack top
		DUMP_PATHS		= 0x00000008L,
		DUMP_PFX		= DUMP_STACKTOP | DUMP_FPUTOP | DUMP_PATHS,
		DUMP_XREFS		= 0x00000010L,//for labels
		DUMP_INDATA		= 0x00000020L,//upon whom it depends (inflow)
		DUMP_OUTDATA	= 0x00000040L,//who depends upon it (outflow)
		DUMP_TABS		= 0x00000080L,
		DUMP_CASTS		= 0x00000100L,
		DUMP_ALL_ARGS	= 0x00000200L,//show(1)/supress(0) arguments in functoion declarations
		DUMP_COLORS		= 0x00000400L,
		DUMP_FONTS		= 0x00000800L,
		DUMP_NOREAL80	= 0x00001000L,
		DUMP_SUBEXPR	= 0x00002000L,//display hidden l-values, representing subexpressions (call args, typically)
		DUMP_PCALLS     = 0x00004000L,//do not expand call casts - just print 'pcall'
		//..1 more

		//these affects number of lines
		DUMP_REDUMP_MASK	= 0xFFFF0000L,

		DUMP_DEADCODE	= 0x00010000L,//show dead code
		DUMP_DEADLABELS	= 0x00020000L,//show dead labels
		DUMP_NOLOGO		= 0x00040000L,
		DUMP_FUNCEX		= 0x00080000L,//put a function's opening brace on the next line
		DUMP_NOSTRUCS	= 0x00100000L,//?
		DUMP_NOTYPES	= 0x00200000L,//?
		DUMP_BLOCKS		= 0x00400000L,//no blocks (structuring)
		DUMP_STRUCLOCS  = 0x00800000L,
		DUMP_LOGICONLY	= 0x01000000L,
		DUMP_NOVAROPS	= 0x02000000L,
		DUMP_NOSTUBS	= 0x04000000L,
		//..1 more

		//should never be toggled
		DUMP_HEADER		= 0x10000000L,
		//..2 more
		DUMP_UNFOLD		= 0x80000000L,//unfold
	};

	enum DUMP2_e
	{
		aa
	};

	struct UDispFlags {
		typedef uint32_t HALF;
		typedef uint64_t FULL;
		union {
			FULL	u;
			struct {
				HALF l;
				HALF h;
			};
		};
		UDispFlags(const UDispFlags &o){ u = o.u; }
		UDispFlags(){ l = 0; h = 0; }
		UDispFlags(HALF _l){ l = _l; h = 0; }
		UDispFlags(HALF _l, HALF _h){ l = _l; h = _h; }
		bool test(UDispFlags f) const { return (u & f.u) != 0; }
		bool testL(HALF f) const { return (l & f) != 0; }
		bool testH(HALF f) const { return (h & f) != 0; }
		void clear(UDispFlags f){ u &= ~f.u; }
		void clearL(HALF f){ l &= ~f; }
		void clearH(HALF f){ h &= ~f; }
		void set(UDispFlags f){ u |= f.u; }
		void setL(HALF f){ l |= f; }
		void setH(HALF f){ h |= f; }
		bool operator==(const UDispFlags &o) const { return u == o.u; }
		bool operator!=(const UDispFlags &o) const { return u != o.u; }
		UDispFlags operator&(const UDispFlags &o) const { 
			UDispFlags t(*this);
			t.u &= o.u;
			return t;
		}
		UDispFlags operator|(const UDispFlags &o) const { 
			UDispFlags t(*this);
			t.u |= o.u;
			return t;
		}
	};

	//margin pixmaps
	enum PixmapEnum
	{
		PIXMAP_NULL,
		PIXMAP_DBG_NEXT,	//actual cur pos in debugger
		PIXMAP_DBG_BP,		//breakpoint
		PIXMAP_DBG_BP_NEXT,	//dbg cur pos over breakpoint
		PIXMAP_DBG_NEXT_HAZY,
		PIXMAP_DBG_BP_NEXT_HAZY,
		PIXMAP__TOTAL
	};

	enum FolderTypeEnum
	{
		FOLDERTYPE_UNK,
		FOLDERTYPE_BINARY_EXE,
		FOLDERTYPE_BINARY_DLL,
		FOLDERTYPE_BINARY_PHANTOM,
		FOLDERTYPE_FOLDER,
		FOLDERTYPE_FILE_H,
		FOLDERTYPE_FILE_CPP,
		FOLDERTYPE_FILE_C,
		FOLDERTYPE_FILE_RC,
		FOLDERTYPE_FILE_T,//types
		FOLDERTYPE_FILE_N,//names
		FOLDERTYPE_FILE_E,//exports
		FOLDERTYPE_FILE_I,//imports
		FOLDERTYPE_FILE_STUB,
		FOLDERTYPE_FILE_TT,//templates
		FOLDERTYPE__TOTAL
	};

	enum CTX_Locality
	{
		CXTID_BINARY = 1,
		CXTID_SOURCE = 2,
		CXTID_SOURCE_H = 3
	};

	/*enum CXT_Scoping
	{
		CXTID_STRUC = 1,
		CXTID_FUNC = 2,
	};*/

	enum LocusIdEnum
	{
		LocusId_NULL,

		LocusId_STRUC_HEADER = 1,
		LocusId_STRUC_DATA = 2,
		LocusId_STRUC_METHOD = 3,
		LocusId_STRUC_STATIC = 4,//v-tables, rtti?
		LocusId_STRUC_BODY = 8,//anywhere else
		LocusId_STRUC__MASK = 0xF,

		LocusId_FUNC_HEADER = 9,//!
		LocusId_FUNC_OP = 10,
		LocusId_FUNC_LOCAL = 11,
		LocusId_FUNC_LABEL = 12,
		LocusId_FUNC_BODY = 16,//anywhere else
		LocusId_FUNC_MASK = 0xF0
	};

	/*enum CXT_e
	{
		CXTID_LOCALITY_MASK = 0x000F,
		CXTID_SCOPING_MASK  = 0x0FF0,
		CXTID_OBJECT_MASK	= 0xF000	//was an object's name clicked
	};*/


	class IADCTextEdit : public My::IUnk
	{
	public:
		virtual int startPos() const = 0;
		virtual void readData(MyStreamBase &) = 0;
		virtual void writeData(MyStreamBase &) = 0;
		virtual int apply() = 0;
	};

#ifdef _DEBUG//type-safety check
	template <typename T>
	class T_DUMPOS
	{
		T	p;
	public:
		T_DUMPOS() : p(0){}
		T_DUMPOS(const T_DUMPOS &o) : p(o.p){}
		explicit T_DUMPOS(size_t l) : p(T(l)){}
		T_DUMPOS &operator=(const T_DUMPOS &o){ p = o.p; return *this; }
		bool isNull() const { return (p <= 0); }
		operator int32_t() const { return p; }
		//void operator++() { ++p; }
		operator bool() const { return !isNull(); }
		operator size_t() const { return p; }
		bool operator < (size_t n) const { return size_t(p) < n; }
		bool operator > (size_t n) const { return size_t(p) > n; }
		bool operator == (size_t n) const { return size_t(p) == n; }
	};
	typedef T_DUMPOS<int16_t> ITER;
	typedef T_DUMPOS<int32_t> DUMPOS;
#else
	//a persistent entity to represent a position in a document
	typedef int DUMPOS;
	//a transient entity, existing within a function's scope,
	//	always originates from DUMPOS, 
	//	can update its originator at end of life;
	typedef short ITER;
	
#endif

	class IADCTextModel;
	class AutoIter
	{
		IADCTextModel *mp;
		ITER it;
		bool bUpdate;
	public:
		AutoIter(IADCTextModel *p, DUMPOS pos, bool _bUpdate = false);
		~AutoIter();
		operator ITER() const { return it; }
		AutoIter &operator++();
		AutoIter &operator--();
	};

	class IADCTextModel : public My::IUnk
	{
	public:
		virtual DUMPOS newPosition() = 0;
		virtual void deletePosition(DUMPOS) = 0;
		virtual DUMPOS posFromIter(ITER) = 0;

		virtual ITER newIterator(DUMPOS) = 0;
		virtual void deleteIterator(ITER, bool bUpdatePos = false) = 0;
		virtual void copyIt(DUMPOS to, DUMPOS from) = 0;
		virtual bool backwardIt(ITER) = 0;
		virtual bool forwardIt(ITER) = 0;
		virtual void seekLineIt(DUMPOS to, int line) = 0;
		virtual int lineFromIt(DUMPOS) = 0;
		virtual const char *dataIt(DUMPOS, bool bPlain = false) = 0;
		virtual int checkEqual(DUMPOS, DUMPOS) = 0;
		virtual void invalidate(bool) = 0;
		//virtual void lockRead(bool) = 0;
		virtual int linesNum() = 0;
		virtual int charsNum() = 0;
		virtual bool atEndIt(DUMPOS) = 0;
		virtual bool seekPosIt(const char *, DUMPOS, DUMPOS itRef = DUMPOS(-1)) = 0;
		virtual IADCTextEdit *startEditIt(DUMPOS, int /*x*/){ return 0; }
		virtual void setCurPosIt(DUMPOS, int /*x*/){}
		virtual PixmapEnum pixmapIt(DUMPOS) { return PIXMAP_NULL; }
	};

	inline AutoIter::AutoIter(IADCTextModel *p, DUMPOS pos, bool _bUpdate)
		: mp(p),
		it(mp->newIterator(pos)),
		bUpdate(_bUpdate)
	{
	}

	inline AutoIter::~AutoIter()
	{
		if (it)
			mp->deleteIterator(it, bUpdate);
	}

	inline AutoIter &AutoIter::operator++()
	{
		mp->forwardIt(it);
		return *this;
	}

	inline AutoIter &AutoIter::operator--()
	{
		mp->backwardIt(it);
		return *this;
	}

	class IADCTableRow
	{
	public:
		virtual void addCell(unsigned col, const char *, bool bUtf8 = true) = 0;
		virtual void addColor(unsigned col, Color_t) = 0;
		virtual void addTree(unsigned col, const char *) = 0;
		enum { STATUS_UNDERLINE = 0x01 };
		virtual void setLineColor(Color_t) = 0;
		virtual void setMarginColor(Color_t) = 0;
		virtual void setUnderLineColor(Color_t) = 0;
		virtual void setCellFlags(unsigned col, unsigned flags) = 0;//CellFlagsEnum
		virtual void setCellWidth(unsigned col, int width) = 0;
	};

	class IADCTableModel : public IADCTextModel
	{
	public:
		typedef unsigned	COLID;
		static const int iTextShift = 1;//offset at which text starts in the contents columns
		static const bool bNo1TextShift = false;//if true, do not shift the 1st visible column
		static int colTextShift(COLID iCol){
			if (iCol > 0 || !bNo1TextShift)
				return iTextShift;
			return 0;
		}
		//ITextViewModel
		virtual const char *dataIt(DUMPOS pos, bool bPlain = false){ return cellDataIt(0, pos, bPlain); }
		//IADCTableModel
		virtual const char *cellDataIt(COLID, DUMPOS, bool bPlain = false) = 0;
		virtual void getColumnInfo(IADCTableRow &, bool bNoData) = 0;
		virtual void getRowDataIt(DUMPOS, IADCTableRow &) = 0;
		virtual bool setCellDataIt(COLID, DUMPOS, const char *) = 0;
		virtual const char *cellTreeDataIt(COLID, DUMPOS) = 0;
		virtual int columnWidth(COLID) = 0;
		virtual void setColumnWidth(COLID, int w) = 0;
		virtual const char *colName(COLID) = 0;
		virtual void setColFlags(COLID, unsigned) = 0;
		virtual unsigned colFlags(COLID) const = 0;
		virtual int colColor(COLID) = 0;
		virtual int colsNum() = 0;
		virtual int colColorIt(COLID, DUMPOS) = 0;
	public:
		void showColumn(int col, bool bShow)
		{
			int w(columnWidth(col));
			if (w < 0)
				w = -w;
			if (!bShow)
				w = -w;
			setColumnWidth(col, w);
		}
	};

	class IBinViewModel : public IADCTableModel
	{
	public:
		enum {
			CLMN_TREEINFO,//tree info must go before the column it's used by
			CLMN_DA,
			//CLMN_ROWSPAN,
			CLMN_FILE,
			CLMN_OFFS,//in segment
			CLMN_BYTES,
			//CLMN_TYPES,
			CLMN_NAMES,
			CLMN_CODE,
			CLMN_DATA = CLMN_CODE,
			CLMN_XREFS,
			CLMN_COMMENTS,
			//----------
			CLMN_TOTAL
		};
		//virtual ROWID rowIdIt(int it) = 0;
		virtual int lineIt(DUMPOS it) = 0;
		virtual bool listObjHierarchyAtIt(DUMPOS it, MyStreamBase &) = 0;
		virtual void scopeTo(const char *) = 0;
		virtual void scopeName(MyStreamBase &) = 0;
		virtual int pushJumpIt(DUMPOS iTop, DUMPOS iCur, int) = 0;
		virtual int popJumpIt(DUMPOS iTop, DUMPOS iCur, int *x = 0) = 0;
		virtual int checkJumpTopIt(DUMPOS iTop, DUMPOS iCur) = 0;
		//virtual bool restorePositionIt(ITER it, MyStreamBase &) = 0;
		//virtual bool listTypesAtIt(DUMPOS it, MyStreamBase &) = 0;
		virtual void postCommandIt(DUMPOS it, const char *) = 0;
		//struct cell_info_t { int fgnd_color; int bgnd_color; };
		//virtual bool setSyncMode(bool) = 0;
		virtual bool checkTaskTop(DUMPOS) = 0;
		virtual int checkSelIt(DUMPOS it) = 0;
		virtual void updateSelection() = 0;
		virtual void lockRead(bool) = 0;
		virtual bool locusInfo(MyStreamBase &, int &) = 0;
		virtual Color_t jumpTarget() = 0;
		virtual void setBitPosition(DUMPOS) = 0;
		virtual bool setCompactMode(bool) = 0;
		virtual bool isCompactMode() const = 0;
		virtual bool setValuesOnlyMode(bool) = 0;
		virtual bool isValuesOnlyMode() const = 0;
		virtual bool setResolveRefsMode(bool) = 0;
		virtual bool isResolveRefsMode() const = 0;
		virtual const char *moduleName() const = 0;
		virtual void tipInfoIt(DUMPOS, int, MyStreamBase &) = 0;
		virtual bool reloadRawData() = 0;
	};

	class IStubsViewModel : public IADCTableModel
	{
	public:
		enum {
			//COLID_MODULE,
			COLID_ADDRESS,
			COLID_NAME,
			COLID_STACK_IN,
			COLID_STACK_OUT,
			COLID_FPU_IN,
			COLID_FPU_OUT,
			COLID_REG_ARGS,
			COLID_REG_RETS,
			COLID_FLAGS,
			//COLID_SAVED_FLAGS,
			//COLID_RET_REGS,
			COLID_PROBLEMS,
			COLID__TOTAL
		};
		virtual bool checkTaskTop(DUMPOS) = 0;
		virtual void lockRead(bool) = 0;
		virtual int Redump(DUMPOS) = 0;
		virtual bool reload() = 0;
	};

	class ISrcViewModel : public IADCTableModel
	{
	public:
		enum {
			CLMN_LNUMS,
			CLMN_STACKTOP,
			CLMN_FPUTOP,
			CLMN_PATHS,
			CLMN_CODE,
			CLMN_EXTRA,
			CLMN_TOTAL
		};
		virtual UDispFlags mode() const = 0;
		virtual int setMode(UDispFlags, bool, DUMPOS = DUMPOS()) = 0;
		virtual int Redump(DUMPOS, bool bResetIterators) = 0;
		virtual bool IsRedumpPending() = 0;
		virtual int fileId() const = 0;
		virtual const char *filePath() const = 0;
		virtual bool getModuleName(MyStreamBase &) const = 0;
		//virtual bool getAddress(DUMPOS, MyStreamBase &) = 0;
		//virtual bool setSyncMode(bool) = 0;
		virtual bool checkTaskTop(DUMPOS) = 0;
		virtual bool IsHeader() const = 0;
		virtual void lockRead(bool) = 0;
		virtual void tipInfoIt(DUMPOS, int, MyStreamBase &) = 0;
		virtual int lineFromItEx(DUMPOS, MyStreamBase &) = 0;
		//enum JumpHint { JumpHint_LocusDecl, JumpHint_LocusDef, JumpHint_JumpTop };
		virtual bool initiateJump(bool bToDefinition, int linesFromTop) = 0;
		virtual bool jump(DUMPOS, bool fwd, bool flip) = 0;
		virtual void printDumpInfo() = 0;
		virtual bool setSubject(const char *) = 0;
		virtual const char *subjectName() const = 0;
	};

	class ITasksViewModel : public My::IUnk
	{
	public:
		virtual void reset() = 0;
		virtual size_t count() const = 0; 
		virtual void data(size_t, MyStreamBase &) const = 0;
		virtual void module(size_t, MyStreamBase &) const = 0;
		virtual void viewPos(size_t, MyStreamBase &) const = 0;
		virtual void lockRead(bool) = 0;
	};

	class ITreeViewModel : public My::IUnk
	{
	public:
		typedef size_t	ITEMID;
	public:
		virtual void reset() = 0;
		virtual void fetch(ITEMID) const = 0; 
		virtual bool hasChildren(ITEMID, bool bFetched) const = 0;
		virtual unsigned childrenNum(ITEMID parent) const = 0; 
		virtual void data(ITEMID, size_t column, MyStreamBase &) const = 0; 
		virtual void path(ITEMID, MyStreamBase &) const = 0; 
		//virtual ITEMID idOfRoot() const = 0;
		virtual ITEMID idOfChild(ITEMID parent, unsigned childIndex) const = 0;
		virtual ITEMID idOfParent(ITEMID child) const = 0;
		virtual unsigned indexOf(ITEMID) const = 0;
		virtual int uniqueOf(ITEMID) const = 0;
		virtual ITEMID IdFromUnique(int) const = 0;
		virtual void rename(ITEMID, const char *) = 0;
		virtual int columnCount() const  = 0;
	};

	class ITypesViewModel : public ITreeViewModel
	{
	public:
		virtual void reset() = 0;
		//virtual size_t count() const = 0; 
		//virtual void data(size_t row, size_t column, MyStreamBase &) const = 0;
		enum E_Flags { E_PRIMITIVE, E_COMPOUND, E_ENUM, E_FUNC, E_CLASS, E_TYPEDEF, E_CODE, E_CONTEXT_DEPENDENT, E__TYPES_TOTAL, E_TYPE_MASK = 0xFF, E_USERDATA = 0x100, E_FWD = 0x200, E_ATTIC = 0x400 };
		virtual unsigned flags(size_t) const = 0;
		virtual const char *moduleName() const = 0;
		virtual bool apply(const char *) = 0;
	};

	/////////////////////////////////////////////
	class IFilesViewModel : public ITreeViewModel
	{
	public:
		virtual FolderTypeEnum type(ITEMID) const = 0;
		virtual void lockRead(bool) = 0;
	};

	//////////////////////////////////////////////
	class INamesViewModel : public ITreeViewModel
	{
	public:
		enum NameTypeEnum { E_NONE, E_TYPE, E_FIELD, E_GLOBAL, E_FUNCTION, E_IMPORTED, E__KIND_TOTAL, E_KIND_MASK = 0xF, E_EXPORTED = 0x10, E_ALIAS = 0x100 };
		virtual NameTypeEnum type(ITEMID, size_t column) const = 0;
		virtual bool apply(const char *) = 0;
		virtual bool goToDefinition(ITEMID) const = 0;
	};


	enum class ExportViewColumns { VA, ORD, NAME, TOTAL };
	enum class ImportViewColumns { VA, MODULE, ORD, NAME, TOTAL };

	typedef INamesViewModel IExportsViewModel;
	typedef INamesViewModel IImportsViewModel;

	/*class IExportsViewModel : public ITreeViewModel
	{
	public:
		//enum NameTypeEnum { E_NONE, E_TYPE, E_GLOBAL, E_FIELD, E_FUNCTION };
		//virtual NameTypeEnum type(ITEMID, size_t column) const = 0;
		//virtual bool apply(const char *) = 0;
		//virtual bool goToDefinition(ITEMID) const = 0;
	};*/

	class ITemplViewModel : public ITreeViewModel
	{
	public:
		enum TypeEnum { E_UNK, E_GLOBAL, E_TYPE, E_FUNCTION };
		virtual TypeEnum type(ITEMID, size_t column) const = 0;
	};

	///////////////////////////////////////////
	class IResViewModel : public IFilesViewModel
	{
	public:
		virtual void viewPos(ITEMID, MyStreamBase &) const = 0;
		virtual void lockRead(bool bLock){ 
			(void)bLock;
		}
	};

	class IDataSource : public My::IUnk
	{
	public:
		virtual void *data() const = 0;
		virtual size_t size() const = 0;
	};

	class IADCExprViewModel : public ITreeViewModel
	{
	public:
		IADCExprViewModel()
		{
		}
		enum Flags { DUMPEXPR_NULL, DUMPEXPR_PTRS = 1, DUMPEXPR_UNFOLD = 2 };
		virtual void resetEx(Flags flags) = 0;

	};


	////////////////////////////////////////
	class IADBCore : public virtual My::IUnk
	{
	public:
		virtual const char *GetCompanyName() const = 0;
		virtual const char *GetProductName() const = 0;
		virtual const char *GetProductCodeName() const = 0;
		virtual const char *GetProductVersion() const = 0;

		virtual long EnableOutputCapture() = 0;
		virtual long FlushOutput(MyStreamBase &) = 0;
		virtual long Shutdown() = 0;
		virtual void ClearLocus() = 0;
		virtual long PostCommand(const char *, bool) = 0;
		virtual long CallCommand(const char *, MyStreamBase &, bool) = 0;
		virtual long SetDebugMode(bool) = 0;
		virtual long Start() = 0;
		virtual long Pause() = 0;
		virtual long Stop() = 0;
		virtual bool GetToDoList(MyStreamBase &) = 0;
		virtual long GetProjectInfo(MyStreamBase &) = 0;
		virtual long GetLocusInfo(unsigned flags, MyStreamBase &) = 0;//Name<string>,Path<string>
		virtual void RefreshBinaryDump(size_t, bool) = 0;
		virtual void DumpResources(MyStreamBase &) = 0;
		//virtual void DumpTypes(MyStreamBase &) = 0;
		virtual unsigned ContextId() const = 0;
		virtual IBinViewModel *NewBinViewModel(const char*) = 0;
		virtual ITypesViewModel *NewTypesViewModel(const char*) = 0;
		virtual INamesViewModel *NewNamesViewModel(const char*) = 0;
		virtual IExportsViewModel *NewExportsViewModel(const char *) = 0;
		virtual IImportsViewModel *NewImportsViewModel(const char *) = 0;
		virtual IFilesViewModel *NewFilesViewModel() = 0;
		virtual IResViewModel *NewResViewModel(const char*) = 0;
		virtual IBinViewModel *NewScopeViewModel(const char *) = 0;
		virtual ITasksViewModel *NewTasksViewModel() = 0;
		enum ResType_e { UNKNOWN, CUR, BMP, ICO };
		virtual IDataSource *GetResourceData(ResType_e &) = 0;
		virtual bool SetCP(const char *) = 0;
		virtual bool ApplySettings(MyStreamBase&) = 0;
	};

	class IADCCore : public virtual IADBCore
	{
	public:
		//dc
		//virtual IBinViewModel *NewBinViewModel(const char *) = 0;
		virtual ISrcViewModel *NewSrcViewModel(const char *, int, const char*) = 0;
		//virtual ITypesViewModel *NewTypesViewModel(const char *) = 0;
		//virtual INamesViewModel *NewNamesViewModel(const char *) = 0;
		virtual ITemplViewModel *NewTemplViewModel(const char *, bool) = 0;
		//virtual IResViewModel *NewResViewModel(const char *) = 0;
		virtual IStubsViewModel *NewStubsViewModel() = 0;
		virtual IStubsViewModel *NewStubsViewModel(const char *) = 0;
		virtual IADCExprViewModel *NewExprViewModel() = 0;
		virtual void DumpBlocks(const char *, MyStreamBase &) = 0;
		//virtual void DumpExpr(MyStreamBase &, IADCExprViewModel::Flags flags) = 0;
		//virtual int GetPtrExprList(MyStreamBase &) = 0;
		//virtual int DumpPtrExpr(int, MyStreamBase &) = 0;
		//virtual long GetProjectFiles(MyStreamBase &) = 0;
		virtual int GetFileIdByName(const char *) = 0;
		virtual bool GetFileNameById(int, MyStreamBase &) = 0;
		virtual bool GetProtoInfo(MyStreamBase &) = 0;
		virtual bool SetProtoInfo(MyStreamBase &) = 0;
		virtual bool JumpSourceBack(const char *keyword) = 0;
		virtual bool JumpSourceForward(const char *keyword) = 0;
		virtual void DumpCutList(MyStreamBase &) = 0;
	};

	enum MsgIdEnum
	{
		MSGID_NULL		= 0x1000,
		MSGID_SHOW,
		MSGID_READY,
		MSGID_RUN,
		MSGID_QUIT,
		MSGID_OUTPUT_COLOR,
		MSGID_OUTPUT_READY,

		UIMSG_PROCESS_EVENTS,
		UIMSG_FILE_LIST_CHANGED,
		UIMSG_DISPLAY_OPENED,
		UIMSG_FILE_PRIOR_REMOVED,
		UIMSG_FILE_RENAMED,
		UIMSG_SRC_DUMP_INVALIDATED,		//redump contents
		//UIMSG_FILE_MODIFIED,
		UIMSG_EXPR_MODIFIED,		//just redraw, do not redump
		UIMSG_APPEND_TEXT,
		UIMSG_COMMAND_SUCCEEDED,
		UIMSG_GLOBAL_ADDED,
		UIMSG_PROJECT_NEW,
		UIMSG_DC_NEW,
		UIMSG_DECOMPILE_FUNCTION,
		UIMSG_PROJECT_ABOUT_TO_CLOSE,
		UIMSG_PROJECT_CLOSED,
		//UIMSG_PROJECT_CHANGED,
		UIMSG_PROJECT_SAVED,
		UIMSG_PROJECT_OPENED,
		UIMSG_GLOBALS_MODIFIED,
		UIMSG_STUBS_LIST_MODIFIED,
		//UIMSG_ANLZ_STATUS_CHANGED,
		UIMSG_TODOLIST_CHANGED,
		UIMSG_TODOLIST_PUSHED_BACK,
		UIMSG_TODOLIST_PUSHED_FRONT,
		UIMSG_TODOLIST_POPPED_FRONT,
		UIMSG_ANALIZER_STARTED,
		UIMSG_ANALIZER_PAUSED,
		UIMSG_ANALIZER_RESUMED,
		UIMSG_ANALIZER_STOPPED,
		UIMSG_ANALIZER_INFO,
		UIMSG_PROGRESS_INFO,
		UIMSG_PREANALIZED,
		UIMSG_GOTO_LOCATION,
		UIMSG_EDIT_STUB,
		UIMSG_SHOW_VIEW,
		UIMSG_CURFOLDER_CHANGED,
		UIMSG_CURFUNC_CHANGED,
		UIMSG_CUROP_CHANGED,
		UIMSG_CURFUNC_MODIFIED,
		UIMSG_CURSTRUC_MODIFIED,
		UIMSG_SELOBJ_CHANGED,
		//UIMSG_PING_BACK,
		UIMSG_NAME_CHANGED,
		UIMSG_LOCUS_CHANGED,
		UIMSG_LOCUS_ADJUSTED,
		UIMSG_TYPESMAP_CHANGED,
		UIMSG_LOCALITY_CHANGED,
		UIMSG_DEBUGGER_STARTED,
		UIMSG_DEBUGGER_STOPPED,
		UIMSG_DEBUGGER_BREAK,
		UIMSG_DEBUGGER_RESUMED,
		UIMSG_UNDEFINED_PROTO,
		UIMSG_CUTLIST_UPDATED,
		//recoils
		UIMSG_SAVE_RECOIL,
		UIMSG_COMMAND_RECOIL,
		UIMSG_NEW_FOLDER_RECOIL,
		UIMSG_ACQUIRE_RECOIL,
		UIMSG_DECOMP_RECOIL,
		//UIMSG_JUMP_RECOIL,

		//...
		UIMSG_LAST
	};
}//namespace adcui

My::IGui *CreateADBGui(int &argc, char ** argv, My::IUnk *pICore);
My::IGui *CreateADCGui(int &argc, char ** argv, My::IUnk *pICore);

#endif//__IADCGUI_H__



