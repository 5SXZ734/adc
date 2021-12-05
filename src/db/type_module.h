#pragma once

#include "type_seg.h"
#include "proj.h"
#include "mem.h"
#include "info_proj.h"

/*struct RAW_t
{
	PDATA ptr;
	unsigned size;
	PDATA meta;
	RAW_t() : ptr(0), size(0), meta(0){}
	RAW_t(PDATA p, unsigned z, PDATA m) : ptr(p), size(z), meta(m){}
};*/

struct Section_t
{
	ROWID	da;
	size_t	size;
	OFF_t	ra;
	size_t	off;//in parent
	TypePtr seg;
	Section_t()
		: da(0), size(0), ra(0), off(0), seg(nullptr){}
	Section_t(ROWID	_da, size_t _size, OFF_t _ra, size_t _off, TypePtr _seg)
		: da(_da), size(_size), ra(_ra), off(_off), seg(_seg){}
	bool operator<(const Section_t& o) const { return da < o.da; }
	//bool operator<(ROWID o_da) const { return da < o_da; }//didn't work for upper_bound
};

class SectionMap_t : protected std::vector<Section_t>
{
	typedef std::vector<Section_t> base;
public:
	SectionMap_t() {}
	void reset() { clear(); }
	const Section_t &add(ROWID da, size_t sz, OFF_t ra, ADDR va, TypePtr seg)
	{
		push_back(Section_t(da, sz, ra, va, seg));
		return back();
	}
	size_t find(ROWID da) const {
		Section_t a(da, 0, 0, 0, nullptr);
		const_iterator i(std::upper_bound(begin(), end(), a));
		if (i != begin())
		{
			--i;
			const Section_t& b(*i);
			if (da - b.da < b.size)
				return std::distance(begin(), i);
		}
		return (size_t)-1;
	}
	const Section_t& get(size_t n) const { return at(n); }
	size_t size() const { return base::size(); }
};

//////////////////////////////
class Module_t : public Seg_t
{
	typedef	Seg_t	Base_t;
public:
	int			mModuleTag;
	int			mUnique;//used in exported map
	FolderPtr	mpModuleFolder;
	DataPtr		mpDataSource;
	MyString	mSubTitle;
	MyString	mDelayedFormat;
	RangeSetMap	mRangeMgr;//for managing subsegs
	SectionMap_t mSections;

public:
	Module_t();
	virtual ~Module_t();
	
	void setSubTitle(const MyString &s);
	const MyString &subTitle() const { return mSubTitle; }
	//const MyString &superTitle() const { return Seg_t::title(); }//for derivative modules
	void setModuleTag(int n){ mModuleTag = n; }
	int moduleTag() const { return mModuleTag; }
	void setUnique(int n) { mUnique = n; }
	int unique() const { return mUnique; }
	FolderPtr	folderPtr() const { return mpModuleFolder; }
	DataPtr		dataPtr() const { return mpDataSource; }
	void setFolderPtr(FolderPtr p){
		mpModuleFolder = p;
		//assert(mpModuleFolder->fileModule()->module() == this;
	}
	void setDelayedFormat(MyString s){ mDelayedFormat = s; }
	MyString delayedFormat() const { return mDelayedFormat;  }

	std::vector<FolderPtr> &specials(){ return mpModuleFolder->fileModule()->specials(); }
	const std::vector<FolderPtr> &specials() const { return mpModuleFolder->fileModule()->specials(); }

	FolderPtr	special(size_t n) const { return specials()[n]; }
	void setSpecial(size_t i, FolderPtr p){
		specials()[i] = p;
	}

	virtual int size(CTypePtr pSelf) const {
		if (mRaw.m_size > 0)
			return (int)mRaw.m_size;
		return Seg_t::size(pSelf);
	}
	virtual int ObjType() const { return OBJID_TYPE_MODULE; }
	virtual	Module_t	*typeModule() const { return const_cast<Module_t *>(this); }
	virtual const char *printType() const { return "module"; }
	/*RAW_t asRAW() const {
		return RAW_t(mRaw.ptr, mRaw.size, mpAuxData.ptr);
	}*/

	bool isRangeMgrEmpty() const;
	RangeSetMap	&rangeMgr(){ return mRangeMgr; }
	const RangeSetMap	&rangeMgr() const { return mRangeMgr; }
	bool addSubRange(TypePtr);

	const SectionMap_t& sections() const { return mSections; }
	SectionMap_t& sections(){ return mSections; }

	void setDataSource(DataPtr p){
		mpDataSource = p;
	}
	DataPtr dataSourcePtr0() const {
		return mpDataSource;
	}
	bool hasDataSource() const {
		return mpDataSource != nullptr;
	}
	virtual const I_DataSourceBase &dataSourceRef() const {
		if (!mpDataSource)
		{
			static DataSourceNull_t dataNull;
			return dataNull;
		}
		return mpDataSource->pvt();
	}

	size_t dataAt(OFF_t rawOffs, OFF_t size, PDATA pDest) const {
		const Block_t &r(rawBlock());
		if (r.empty() || size == 0)
			throw (-1);
		if (!(rawOffs < r.m_size))
			throw (-2);
		return dataSourceRef().dataAt(rawOffs, size, pDest);
	}

	//bool addDataChunk(OFF_t, size_t, TypePtr)
	static MyString makeSubtitle(const char *module, const char *submodule);
	MyString rangeSetName(TypePtr ) const;
};


class LocusInfo_t : public I_DataStreamBase
{
	const Locus_t &mr;
public:
	LocusInfo_t(const Locus_t &r)
		: mr(r)
	{
	}
protected:
	virtual POSITION setcp(POSITION){
		assert(0); return 0;
	}
	virtual POSITION cp() const {
		return (POSITION)mr.va();
	}
	virtual OFF_t cpr() const {
		return (OFF_t)mr.ra();
	}
	virtual unsigned skip(int){
		assert(0); return 0;
	}
	virtual unsigned skipBits(int){
		assert(0); return 0;
	}
	virtual POSITION align(unsigned){
		assert(0); return 0;
	}
	virtual size_t dataAt(OFF_t rawOffs, OFF_t size, PDATA pDest) const {
		return module().dataAt(rawOffs, size, pDest);
	}
	virtual OFF_t size() const { 
		return mr.back().m_size; }
	virtual const I_AuxData *aux() const
	{
		DataPtr pData(module().dataSourcePtr0());
		if (!pData)
			return nullptr;
		return pData->pvt().aux();
	}
private:
	const Module_t &module() const {
		return *mr.module2()->typeModule();
	}
};



