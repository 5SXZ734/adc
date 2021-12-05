#pragma once

#include <list>
#include "inttypes.h"//PRIx64

#include "qx/QxTime.h"
#include "shared/defs.h"
#include "shared/circvec.h"
#include "type_seg.h"
#include "command.h"
#include "anlz.h"
#include "info_module.h"
#include "shared/data_source.h"


class DataSourcePane2_t : public DataSubSource_t
{
public:
	DataSourcePane2_t(const I_DataSourceBase &target, CTypePtr iSeg)
		: DataSubSource_t(target, iSeg->typeSeg()->rawBlock())
	{
	}
	DataSourcePane2_t(const I_DataSourceBase &target, OFF_t lower, OFF_t extent)
		: DataSubSource_t(target, lower, extent)
	{
	}
	DataSourcePane2_t(const DataSourcePane2_t &o, OFF_t addExtent)
		: DataSubSource_t(o, addExtent)
	{
	}
};

class CodeStream_t : public DataStream_t
{
protected:
	ADDR64	m_imageBase;
	ADDR	m_base;//segment base
	bool	mbLarge;//64bit?
	ADDR	m_addr;//current address
public:
	CodeStream_t(const I_DataSourceBase &, CTypePtr seg, ADDR);
	CodeStream_t(const I_DataSourceBase &rData, ADDR64 imageBase, ADDR base, ADDR va, bool isLarge);
	/*CodeStream_t(const CodeStream_t &o)
		: DataStream_t(o),
		m_imageBase(o.m_imageBase),
		m_base(o.m_base),
		mbLarge(o.mbLarge),
		m_addr(o.m_addr)
	{
	}*/
	operator bool() const { return !isAtEnd(); }
	ADDR base() const { return m_base; }
	ADDR addr() const { return m_addr; }
	ADDR64 imageBase() const { return m_imageBase; }
	bool isLarge() const { return mbLarge; }
	ADDR updateAddr(){
		OFF_t off(tell());
		if (off != OFF_NULL)
			m_addr = m_base + (ADDR)off;
		else
			m_addr = (ADDR)-1;
		return m_addr;
	}
};

class CodeIterator : public CodeStream_t
{
protected:
	CTypePtr miTypeCode;
	ins_desc_t	m_desc;

public:
	CodeIterator(const I_DataSourceBase &, CTypePtr, CTypePtr seg, ADDR);
	ADDR nextAddr() const { return m_addr + m_desc.length; }
	const ins_desc_t &desc() const { return m_desc; }
	CTypePtr typeCode() const {
		return miTypeCode;
	}
	bool unassemble();
};

class CodeIterator1 : public CodeIterator
{
public:
	CodeIterator1(const I_DataSourceBase &, CFieldPtr);
};

class ProjModifier_t : public ModuleInfo_t
{
public:
	ProjModifier_t(const ModuleInfo_t&);
	ProjModifier_t(const ModuleInfo_t&, MemoryMgr_t&);
	ProjModifier_t(Project_t&, TypeObj_t& mod);
	FieldPtr MakeData(Locus_t &, TypePtr, AttrIdEnum, bool bForce = false);
	FieldPtr MakeReal(Locus_t &, bool bForce = false);
	FieldPtr makeBit(Locus_t &, TypePtr, AttrIdEnum, bool bForce = false);
	int ExpandFunc(const Locus_t &, ADDR addr_end);
	bool SetFunctionEnd(Locus_t &);
	bool ResizeStruc(TypePtr iType, unsigned newSize, const Frame_t *pOuter, bool bForce);
	int RelocateFields(TypePtr iSrc, FieldMapIt lower, FieldMapIt upper, TypePtr pStrucDst, ADDR, ADDR) const;
	int relocateFields(TypePtr from, FieldMapIt, TypePtr to, ADDR aBegin, ADDR anEnd);
	FieldPtr makeObjOfType(Locus_t &, TypePtr, const MyString &, bool bForce);
	int SplitFunction(Locus_t &, bool bWeak);
	int makeFunc2(Locus_t &, bool, int depth = -1);
	//int makeThunk(Locus_t &, bool);
	int makeUndefined(Locus_t &);
	FieldPtr makeOffset(Locus_t &, bool bForce);
	FieldPtr MakeArray(Locus_t &, int);
	FieldPtr ToggleExported(Locus_t &);
	FieldPtr ToggleImported(Locus_t &);
	int MakeGap(Probe_t &, bool);
	bool getBoundsOnAddress(TypePtr, ADDR addr, ADDR bounds[2]);
	bool makeScope(Locus_t &, ObjId_t);
	void UpdateLocus(Locus_t &);
	TypePtr TypeInContext(MyString, CTypePtr);

	int ForwardCommand(const char* command, CFieldPtr);
	int ForwardCommand(const char* command, DataStream_t&, TypePtr type, TypePtr seg);//recursive
	bool TriggerCodeSweep(TypePtr seg, ADDR, const char * = "");
	enum { SweepCode_Call = 1, SweepCode_NamePpg = 2 };
	ADDR SweepCode(CFieldPtr, int depth, unsigned flags = 0);
	ADDR SweepThunk(CFieldPtr, int depth, unsigned flags = 0);
	int SweepProc(CFieldPtr pField, int depth = 0, unsigned flags = 0);
	int CheckThunk(CFieldPtr, ADDR&) const;//-1:indirect,0:not a thunk;1:direct
	RESULT_e MakeThunk(Locus_t &, bool bUserAction = false, bool bPropagateName = false);
	RESULT_e MakeCode(Locus_t &, bool bForce);
	RESULT_e MakeProcedure(Locus_t &, bool bUserAction = false, unsigned type = 0);//0:func;1:thunk;2:vthunk;3:nvthunk
	FieldPtr MakeOffset(Locus_t &, bool bForce);
	FieldPtr MakeArray(TypePtr, ADDR, int);
	bool MakeArray(TypePtr, FieldMapCIt, int);
	FieldPtr MakeBit(TypePtr pType, AttrIdEnum attr, bool bForce, Locus_t &);
	TypePtr NextIntAt(const Locus_t &, bool bSigned);
	TypePtr NextRealAt(const Locus_t &);
	FieldPtr MakeData(TypePtr, AttrIdEnum, bool bForce, Locus_t &);
	FieldPtr MakeReal(TypePtr, ADDR, bool bForce, Locus_t &);
	TypePtr CarouselType(const Locus_t&);
	bool MakeSigned(Locus_t &, bool bForce);
	FieldPtr MakeString(AttrIdEnum, Locus_t&);
	int MakeUnk(Locus_t &);
	int MakeClone(Locus_t &);
	TypePtr MakeArrayOfTypeAttr(const Locus_t &, TypePtr);
};





/////////////////////////////////////////////BinaryAnalyzer_t
class BinaryAnalyzer_t : public IAnalyzer
{
	Project_t& mrProject;
	TypePtr		mpModule;
	int			mbFunc;
	int funcs_num;
	MyString	mCurFileName;
	ADDR	mCurVA;
	unsigned	mSegAffinity;

//public:
	//QxTime	mAtNotifier;

public:

	class Command
	{
	//protected:
		//int mId;
	public:
		Command(){}
		//Command(int id) : mId(id){}
		virtual ~Command(){}
		virtual std::string toStr(){ return std::string(); }
		//int id(){ return mId; }
	};

	enum MakeTypeEnum { E_NULL, E_MAKEDATA, E_MAKECODE, E_MAKEFUNC, E_SWEEPFUNC, E_MAKEFUNCTHUNK };

	struct Task_t
	{
		TypePtr		seg;
		ADDR		va;
		MyString	cmd;
		Task_t() : seg(nullptr), va(0){}
		Task_t(TypePtr p, ADDR v, MyString s) : seg(p), va(v), cmd(s){}
		bool extractVA(ADDR &_va)
		{
			MyString& s(cmd);
			unsigned n(s.find("-@"));
			if (n == -1)
				return false;
			unsigned n2(n + 2);
			while (n2 < s.length() && isspace(s[n2]))
				n2++;
			unsigned n3(n2);
			while (n3 < s.length() && !isspace(s[n3]))
				n3++;
			std::istringstream ss(s.substr(n2, n3 - n2));
			ADDR64 va64;
			ss >> std::hex >> va64;
			_va = ADDR(va64 - seg->imageBase());
			while (n > 0 && isspace(s[n - 1]))
				--n;
			s.remove(n, n3 - n);
			return true;
		}
		bool getVA(ADDR& _va) const
		{
			const MyString& s(cmd);
			unsigned n(s.find("-@"));
			if (n == -1)
				return false;
			while (n < s.length() && isspace(s[n]))
				n++;
			unsigned n2(n);
			while (n2 < s.length() && !isspace(s[n2]))
				n2++;
			std::istringstream ss(s.substr(n, n2 - n));
			ADDR64 va64;
			ss >> std::hex >> va64;
			_va = ADDR(va64 - seg->imageBase());
			return true;
		}
	};

	class TaskList_t : public CircularVector_t<Task_t>
	{
		std::set<ADDR>	mSet, mSetF;		//we need to keep track of which addresses are already submitted
													//otherwise it is gonna be slow
													//keep 2 maps, so it would be possible override the code request with the func one
													//CAUTION: the solution wil be needed for mixing of modules
	public:
		TaskList_t()
		{
		}
		~TaskList_t()
		{
			cleanup();
		}
		bool pushTaskBack(TypePtr p, ADDR va, MyString s)
		{
CHECK(va == 0x422a20)
STOP
			if (s.startsWith("makefunc"))
			{
				if (!mSetF.insert(va).second)
					return false;
				//mSet.erase(va);//override any code/data requests for a given va
			}
			else
			{
				if (mSetF.find(va) != mSetF.end())//already request for a function
					return false;
				if (!mSet.insert(va).second)
					return false;
			}
			push_back(Task_t(p, va, s));
			return true;
		}
		//void pushTaskFront(TypePtr s, MakeTypeEnum e, ADDR v, ADDR v2, int d)
		//{
			//push_front(Task_t(s, e, v, v2, d));
		//}
		Task_t popTaskFront()
		{
			assert(!empty());
			Task_t a(front());
			front() = Task_t();//clear
//CHECK(a.va == 0x422a20)
//STOP
			pop_front();
			if (a.cmd.startsWith("makefunc"))
			{
				if (mSetF.erase(a.va) != 1)
					ASSERT0;
			}
			else
			{
				if (mSet.erase(a.va) != 1)
					ASSERT0;
			}
			return a;
		}
		/*void cleanup()
		{
			assert(0);
			while (!empty())
			{//cleanup queue
				Command *pCmd(popCommand());
				delete pCmd;
			}
		}*/
	};
	
	TaskList_t	mToDoList;

public:
	BinaryAnalyzer_t(Project_t &rProjRef, TypePtr iModule, unsigned uSegAffinity);
	virtual ~BinaryAnalyzer_t();
	MemoryMgr_t &memMgr() const;

	//IAnalyzer
	virtual int process();
	
	bool pushTask(TypePtr, ADDR, const char*);
	void setFuncFlag(int bFunc){ mbFunc = bFunc; }
	void setCurrentVA(ADDR a){ mCurVA = a; }

protected:
	// IAnalyzer
	virtual size_t size() const//number of tasks
	{
		return mToDoList.size();
	}
	virtual void writeTask(size_t i, MyStreamBase &ss)
	{
		MyStreamUtil ssu(ss);
		MyString s(mToDoList[i].cmd);
		ssu.WriteString(s);
	}
	virtual void writeModule(size_t, MyStreamBase &ss)
	{
		MyStreamUtil ssu(ss);
		ssu.WriteString(mpModule->typeModule()->folderPtr()->name());
	}
	virtual void writeDA(size_t i, MyStreamBase &ss)
	{
		MyStreamUtil ssu(ss);
		ADDR va(mToDoList[i].va);
		CTypePtr pSeg(checkVA(va, mToDoList[i].seg));
		ROWID da(ProjectInfo_s::VA2DA(pSeg, va));
		char buf[32];
		sprintf(buf, "~%" PRIX64, da);
		ssu.WriteString(buf);
	}
	virtual bool writeToDoList(MyStreamBase &ss)
	{
		MyStreamUtil ssu(ss);
		ssu.WriteString(mpModule->typeModule()->folderPtr()->name());
		for (TaskList_t::iterator i(mToDoList.begin()); i != mToDoList.end(); i++)
			ssu.WriteString((*i).cmd);
		return true;
	}
	virtual bool finished(){ return mToDoList.empty(); }
	virtual ADDR currentVA() const { return mCurVA; }
	virtual Folder_t *currentFile() const { return 0; }
	virtual FieldPtr currentOpField() const { return 0; }
	virtual void setContextFile(const char *s) { mCurFileName = s; }
	virtual I_Context *makeContext() const { return nullptr; }
	virtual void getLocus(Locus_t &) const {}
	virtual void setCurrentFieldRef(FieldPtr ){}

protected:
	static void COMMAND_continue(BinaryAnalyzer_t *, int, char *[]);
	static void COMMAND_pause(BinaryAnalyzer_t *, int, char *[]);
	static void COMMAND_abort(BinaryAnalyzer_t *, int, char *[]);

private:
	const Frame_t &frameToDelete(const Locus_t &);
	int isArrayTo(TypePtr, OpType_t) const;
	TypePtr checkVA(ADDR va, TypePtr seg);
};

