#pragma once

#include "shared/data_source.h"

class DataSource_t;
class DataLeech_t;
class DataObj_t;
typedef	DataObj_t*	DataPtr;
typedef	const DataObj_t*	CDataPtr;

//////////////////////////////////////////// I_DataSource
// common interface for DataSource_t and DataLeech_t
class I_DataSource : public I_DataSourceBase
{
	AuxDataSource_t		*mpAuxData;
public:
	I_DataSource();
	virtual ~I_DataSource();
	//virtual const char *path() const { return nullptr; }
	//virtual void setPath(const char *){}
	virtual DataSource_t *dataSource() const { return nullptr; }
	virtual DataLeech_t* dataLeech() const { return nullptr; }
	virtual DATAID_t dataId() const { return DATAID_NULL; }
	virtual DataPtr dataHost() const { return nullptr; }

	//I_DataSourceBase
	virtual void release(){ delete this; }
	virtual const I_AuxData *aux() const { return mpAuxData; }
	virtual void setAuxData(PDATA p, size_t size);
	virtual void clearAuxData();
	virtual I_DataSourceBase *openFile(const char *path, bool bLocal) const;

};

class DataObj_t : public RefCounter_t
{
	I_DataSource *mp;
	MyString mTitle;//like that for binary

public:
	DataObj_t()
		: mp(nullptr)
	{
	}
	~DataObj_t()
	{
		delete mp;
	}
	void setPvt(I_DataSource *p){ mp = p; }
	I_DataSource &pvt() const { return *mp; }
	DataPtr dataHost() const { return mp->dataHost(); }
	OFF_t size() const { return mp->size(); }
	OFF_t pos(OFF_t o) const { return mp->pos(o); }
	void setName(const MyString &s){ mTitle = s; }
	const MyString &name() const { return mTitle; }
	DataSource_t *dataSource() const { return mp->dataSource(); }
	DataLeech_t* dataLeech() const { return mp->dataLeech(); }
};

#if(0)///////////////////////////////////DataSource_t (old)
class DataSource_t : public I_DataSource
{
	PDATA	mpData;
	size_t	mSize;
	MyString mPath;
public:
	DataSource_t()
		: mpData(nullptr),
		mSize(0)
	{
	}
	virtual ~DataSource_t()
	{
		delete [] mpData;
	}
	const std::string &path() const { return mPath; }
	bool load(const char *filePath)
	{
		std::ifstream in(filePath, std::ifstream::ate | std::ifstream::binary);
		if (!in.is_open())
			return false;
		std::ifstream::pos_type lSize(in.tellg());
		in.seekg(std::ifstream::beg);
		mSize = (size_t)lSize;
		mpData = new char[mSize];
		in.read((char *)mpData, mSize);
		in.close();
		mPath = filePath;
		return true;
	}
//protected:
	virtual DATAID_t dataId() const { return DATAID_SOURCE; }
	virtual DataSource_t *dataSource() const { return const_cast<DataSource_t *>(this); }
	virtual size_t dataAt(OFF_t rawOffs, OFF_t size, PDATA pDest) const 
	{
		if (!mpData || size == 0)
			return 0;
		ADDR lo = ADDR(rawOffs);
		if (!(rawOffs < mSize))
			return 0;
		ADDR hi = ADDR(lo + size);
		if (hi > mSize)
			size -= (hi - mSize);
		memcpy((void*)pDest, mpData + lo, size);
		return size;
	}
	virtual OFF_t size() const
	{
		return mSize;
	}
};
#else

/////////////////////////////////// DataSource_t (new)
class DataSource_t : public I_DataSource
{
	MemoryMapped	mFile;
public:
	DataSource_t()
	{
	}
	virtual ~DataSource_t()
	{
		mFile.close();
	}
	const std::string &path() const { return mFile.filename(); }
	bool load(const char *filePath)
	{
		return mFile.open(filePath);
	}
//protected:
	virtual DATAID_t dataId() const { return DATAID_SOURCE; }
	virtual DataSource_t *dataSource() const { return const_cast<DataSource_t *>(this); }
	virtual size_t dataAt(OFF_t uOffs, OFF_t uSize, PDATA pDest) const //throw (DataAccessFault_t)
	{
		if (!mFile.getData() || uSize == 0)
			throw DataAccessFault_t(-1, uOffs, uSize);
		OFF_t fileSize((OFF_t)mFile.size());
		if (!(uOffs < fileSize))
			throw DataAccessFault_t(-2, uOffs, uSize);
		ADDR lo((ADDR)uOffs);
		ADDR hi((ADDR)(lo + uSize));
		if (hi > fileSize)
			uSize -= size_t(hi - fileSize);
		memcpy((void*)pDest, mFile.getData() + lo, size_t(uSize));
		return size_t(uSize);
	}
	virtual OFF_t size() const
	{
		return (OFF_t)mFile.size();
	}
	virtual const char *modulePath() const { return path().c_str(); }
};
#endif


///////////////////////////////////////DataLeech_t
class DataLeech_t : public I_DataSource,
	private data_leech_t
{
	DataPtr		mpDataHost;
public:
	DataLeech_t()
		: mpDataHost(nullptr)
	{
	}
	DataLeech_t(const PatchList_t &pm, DataPtr pdh)
		: data_leech_t(pm),
		mpDataHost(pdh)
	{
		assert(pdh);
		mpDataHost->addRef();
	}
	virtual ~DataLeech_t()
	{
		clearAuxData();
		assert(!mpDataHost);
		//mpDataHost->releaseRef();
	}
	V_t &chunks(){ return data_leech_t::chunks(); }
	const V_t &chunks() const { return data_leech_t::chunks(); }
	virtual DataPtr dataHost() const { return mpDataHost; }
	void setDataHost(DataPtr p){ mpDataHost = p; }
	
protected:
	virtual DATAID_t dataId() const { return DATAID_LEECH; }
	virtual DataLeech_t *dataLeech() const { return const_cast<DataLeech_t *>(this); }
	virtual size_t dataAt(OFF_t offs, OFF_t size, PDATA pDest) const {
		return data_leech_t::dataAt(offs, size, pDest, mpDataHost->pvt());
	}
	virtual OFF_t size() const {
		return data_leech_t::size();
	}
	virtual OFF_t pos(OFF_t offs) const {
		return data_leech_t::pos(offs);
	}
	virtual const I_DataSourceBase *host() const {
		return &mpDataHost->pvt();
	}
};


//////////////////////////////////////////////frame_t
struct frame_t : public Block_t
{
	TypeBasePtr m_cont;
	ADDR	m_addr;
	frame_t(TypeBasePtr p, ADDR a = 0)
		: m_cont(p),
		m_addr(a)
	{
	}
	frame_t(const Block_t &r, TypeBasePtr p, ADDR a)
		: Block_t(r),
		m_cont(p),
		m_addr(a)
	{
	}
	frame_t(const frame_t &o)
		: Block_t(o),
		m_cont(o.m_cont),
		m_addr(o.m_addr)
	{
	}
	TypePtr cont() const { return m_cont->objTypeGlob(); }
	TypeBasePtr contBase() const { return m_cont; }
	void setAddr(ADDR a){ m_addr = a; }
	ADDR addr() const { return m_addr; }
	unsigned offs() const { return m_addr - m_cont->base(); }
	frame_t &operator=(const frame_t &o){
		Block_t::operator=(o);
		m_cont = o.m_cont;
		m_addr = o.m_addr;
		return *this;
	}
	bool operator==(const frame_t &o) const {
		return Block_t::operator==(o) && m_cont == o.m_cont && m_addr == o.m_addr;
	}
};












