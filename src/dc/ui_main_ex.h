#pragma once

#include "shared/circvec.h"
#include "db/ui_main.h"
#include "db/ui_files.h"
#include "dump.h"
#include "stubs.h"
#include "main_ex.h"

class ADCStubsViewModelOld;
class GuiEx_t;

#pragma warning(disable:4250)

/////////////////////////////////////////////////////////StubsArray_t

#if(!SBTREE_STUBS)
class StubsArrayElt : public std::vector<StubIt>
{
	struct compi_t {
		bool operator()(StubIt i, ADDR a) const { return (i->first < a); }
	};

	Folder_t*	pFolder;

public:
	StubsArrayElt() : pFolder(0){}
	StubsArrayElt(Folder_t *p) : pFolder(p){}
	size_t redump(StubMgr_t &);
	Stub_t *fromIndex(size_t idx, Folder_t **ppFolder) const
	{
		if (idx < size())
		{
			if (ppFolder)
				*ppFolder = pFolder;
			return &at(idx)->second;
		}
		return nullptr;
	}
	int toIndex(ADDR addr) const
	{
		std::vector<StubIt>::const_iterator it(std::lower_bound(begin(), end(), addr, compi_t()));
		if (it != end())
			if ((*it)->first == addr)
				return int(std::distance(begin(), it));
		return -1;
	}
};

class StubsArray_t : protected std::map<size_t, StubsArrayElt>
{
	//Main_t &mrMain;
	typedef std::map<size_t, StubsArrayElt>	Base_t;

	struct comp_t {
		bool operator()(StubIt i, StubIt j){ return (i->first < j->first); }
	};

public:
	StubsArray_t(){}

	void clear() { Base_t::clear(); }

	int toIndex(Folder_t *pFolder, ADDR addr);
	Stub_t *fromIndex(size_t idx, Folder_t **ppFolder) const;
	size_t size() const;
	bool empty() const;
	void redump(Main_t &);
	void redump(Main_t &, Dc_t *, size_t &);
};
#endif



class IGuiEx_t : public virtual IGui_t
{
public:
	virtual void GuiOnDcNew() const {}
	virtual void GuiOnStubsModified() const {}
	virtual void GuiOnExprModified() const {}
	//virtual void GuiOnSyncPanes(int line) const {}
	virtual void GuiInvalidateDisplays(FolderPtr folder, TypeBasePtr cont, RedumpFlags) const {}
	virtual void GuiInvalidateStubs(Dc_t &rDC) const {}
	virtual void GuiOnDecompileFunction(MyString relPath, MyString pos) const {}
	virtual bool GuiOnNoSourceContext(const char *cmd) const { return false; }
	virtual void GuiOnFunctionCallStop(MyStreamBase &) const {}
	virtual void OnCutListChanged() const {}
};

//HOW NAVIGATION IN SRC WORKS:
//1) Initiate navigation request: In GUI, in source view, click on a target object and from context menu choose 'Go to Declaration/Definition';
//2) In core, the model tries to locate the target in its own dump. If it succeeds, it adjusts the current iterator and the GUI refreshes the view. Done.
//		If it fails (the target is located in another file or in it's counterpart - H/CPP), 
//3)

struct SrcJumpData_t
{
	ObjPtr obj;
	OpPtr op;
	TypeBasePtr scope;
	FolderPtr folder;
	bool isHeader;
	TypePtr module;
	int linesFromTop;
	adcui::LocusIdEnum pickId;
	SrcJumpData_t()
		: obj(nullptr),
		op(OpPtr()),
		scope(nullptr),
		folder(nullptr),
		isHeader(false),
		module(nullptr),
		linesFromTop(0),
		pickId(adcui::LocusId_NULL)
	{
	}
	SrcJumpData_t(ObjPtr _obj, TypeBasePtr _scope, FolderPtr _folder, bool _isHeader, TypePtr _module, int _linesFromTop)
		: obj(_obj),
		op(OpPtr()),
		scope(_scope),
		folder(_folder),
		isHeader(_isHeader),
		module(_module),
		linesFromTop(_linesFromTop),
		pickId(adcui::LocusId_NULL)
	{
	}
};

class CoreEx_t : public virtual Core_t,
	public adcui::IADCCore
{
	SlotVector<DisplayUI_t>	mDisplays;

	//CircularVector_t<SrcJumpData_t> mJumpStackBack;
	std::list<SrcJumpData_t> mJumpStackBack;
	std::list<SrcJumpData_t> mJumpStackForward;
	
public:
	CoreEx_t(Mainx_t&, ADCOutputSink &rGui, bool bEnableOutputCapture);
	virtual ~CoreEx_t();
	ProjectEx_t &projx() const { return reinterpret_cast<ProjectEx_t &>(project()); }
	size_t openDisplay(CFolderPtr pFile, adcui::UDispFlags, OpType_t ptrSize);
	void closeDisplay(size_t);
	DisplayUI_t *displayAt(size_t);
	int invalidateDisplays(FolderPtr file, TypeBasePtr cont, RedumpFlags);
	void pushJumpBack(const SrcJumpData_t& a){ mJumpStackBack.push_front(a); }
	void pushJumpForward(const SrcJumpData_t& a){ mJumpStackForward.push_front(a); }
	bool popJumpBack(SrcJumpData_t&);
	bool popJumpForward(SrcJumpData_t&);
	//void recoilJumpRequest();
	void clearJumpBack() { mJumpStackBack.clear(); }
	void clearJumpForward() { mJumpStackForward.clear(); }
	virtual Probe_t* setLocus(Probe_t *);
	ProbeEx_t *getContextEx();
	ObjPtr objAtLocus(const Dc_t*);
	virtual int checkSelection(CTypePtr, const DA_t&);
	void WriteLocusPath(MyStreamBase &, bool bSkipSegs) const;
protected:
	//adcui::IADCCore
	virtual adcui::IBinViewModel *NewBinViewModel(const char *);
	virtual adcui::ISrcViewModel *NewSrcViewModel(const char *, int, const char*);
	virtual adcui::ITypesViewModel *NewTypesViewModel(const char *);
	virtual adcui::INamesViewModel *NewNamesViewModel(const char *);
	virtual adcui::ITemplViewModel *NewTemplViewModel(const char *, bool);
	virtual adcui::IResViewModel *NewResViewModel(const char *);
	virtual adcui::IStubsViewModel *NewStubsViewModel();
	virtual adcui::IStubsViewModel *NewStubsViewModel(const char *);
	virtual adcui::IFilesViewModel *NewFilesViewModel();
	virtual adcui::IADCExprViewModel *NewExprViewModel();
	virtual BinViewModel_t *NewBinViewModel(CObjPtr iScope, bool bTypeEdit);
	virtual void DumpBlocks(const char *, MyStreamBase &);
	//virtual void DumpExpr(MyStreamBase &, adcui::IADCExprViewModel::Flags);
	//virtual int GetPtrExprList(MyStreamBase &);
	//virtual int DumpPtrExpr(int, MyStreamBase &);
//	virtual long GetProjectFiles(MyStreamBase &);
	virtual int GetFileIdByName(const char *);
	virtual bool GetFileNameById(int, MyStreamBase &);
	virtual bool GetProtoInfo(MyStreamBase &);
	virtual bool SetProtoInfo(MyStreamBase &);
	virtual long GetLocusInfo(unsigned, MyStreamBase &ss);
	virtual bool JumpSourceBack(const char *);
	virtual bool JumpSourceForward(const char *);
	virtual void DumpCutList(MyStreamBase &);
private:
	FolderPtr findFolder(MyString sStem, bool &bHeader);
	adcui::ITemplViewModel *newTemplViewModel(FolderPtr);
//	adcui::ITemplViewModel *newTemplViewModel2(FolderPtr);
	adcui::INamesViewModel *newNamesViewModel2(FolderPtr folderNames);
	virtual adcui::IExportsViewModel* newExportsViewModel(FolderPtr) override;
protected:
	virtual TypePtr OnObjectPicked(ObjPtr, TypePtr module);
};


/////////////////////////////////////////////////////////GuiEx_t

class GuiEx_t : public IGuiEx_t,
	public Gui_t
{
#if(!SBTREE_STUBS)
	StubsArray_t	mStubsArray;
#endif
	friend class Mainx_t;
public:
	GuiEx_t(My::IGui *pIGui, Mainx_t &rMain)
		: Gui_t(pIGui, rMain)
	{
	}
	virtual ~GuiEx_t()
	{
	}
	CoreEx_t *corex() const { return dynamic_cast<CoreEx_t *>(mpCore); }
	//void	RebuildStubsArray();
	//size_t	StubsArraySize();
	//int		StubsArrayToIndex(ADDR);
	//Stub_t*	StubsArrayfromIndex(size_t);
	//dc commands (should go to corex)
	virtual void GuiOnDcNew() const { GuiNotify(adcui::UIMSG_DC_NEW); }
	virtual void GuiOnStubsModified() const { GuiNotify(adcui::UIMSG_STUBS_LIST_MODIFIED); }
	virtual void GuiOnExprModified() const { GuiNotify(adcui::UIMSG_EXPR_MODIFIED); }
	//virtual void GuiOnSyncPanes(int line) const;
	virtual void GuiInvalidateDisplays(FolderPtr folder, TypeBasePtr cont, RedumpFlags) const;
	virtual void GuiInvalidateStubs(Dc_t &rDC) const;
	virtual void GuiOnDecompileFunction(MyString relPath, MyString pos) const;
	virtual bool GuiOnNoSourceContext(const char *cmd) const;
	virtual void GuiOnFunctionCallStop(MyStreamBase &ss) const { GuiNotify(adcui::UIMSG_UNDEFINED_PROTO, ss); }
	virtual void OnCutListChanged() const { GuiNotify(adcui::UIMSG_CUTLIST_UPDATED); }

public:
	//ProjectEx_t &projx() const { return reinterpret_cast<ProjectEx_t &>(project()); }
};

class ContextSafeEx_t
{
	CoreEx_t &rCore;
	ProbeEx_t *pCtx;
public:
	ContextSafeEx_t(CoreEx_t& r)
		: rCore(r),
		pCtx(rCore.getContextEx())
	{
	}
	ContextSafeEx_t::~ContextSafeEx_t()
	{
		if (pCtx)
			rCore.releaseContext(pCtx);
	}
	operator bool() const { return pCtx != nullptr; }
	bool empty() const { return !pCtx && pCtx->locus().empty(); }
	ProbeEx_t *get() const { return pCtx; }
	const ProbeEx_t *operator->() const { return pCtx; }
};



GuiEx_t *CreateADCGuiBroker(My::IGui *pIGui, Mainx_t &rMain,/*const char *appName, const char *appVersion,*/ bool bEnableOutputCapture);
