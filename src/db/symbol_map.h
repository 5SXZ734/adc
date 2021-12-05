#pragma once

#include <sstream>
#include "qx/MyString.h"
#include "qx/MyStream.h"

#include "shared/data_source.h"
#include "shared/front.h"
#include "front_impl.h"
#include "type_module.h"


struct SymbolInfo
{
	ADDR mva;
	I_Front::SymbolKind meKind;
	OFF_t	moSymbolName;
	MyStringEx	msSymbolName;
	unsigned	muNameMax;//0:if ascii-string
};


I_Front::SymbolKind demangleSymbol(I_Front *, const MyString &, MyString& );



class DumpSymbolBase_t : public I_Front::I_DumpSymbol
{
protected:
	const I_DataSource &mRaw;
	ADDR mva;
	I_Front::SymbolKind meKind;
	OFF_t moSymbolName;
	MyStringEx msSymbolName;
	OFF_t moModuleName;
	std::string msModule;
public:
	DumpSymbolBase_t(const I_DataSource &r)
		: mRaw(r)
	{
		reset();
	}
protected:
	virtual void setVA(ADDR va) override { mva = va; }
	virtual void setKind(I_Front::SymbolKind e) override { meKind = e; }
	virtual void setName(const char* s) override {
		msSymbolName = MyString(s);
	}
	virtual void setName(OFF_t o) override
	{
		moSymbolName = o;
		if (!mRaw.isNull(o))
		{
			DataStream_t aRaw(mRaw, o);
			std::ostringstream ss;
			if (aRaw.fetchString(ss))
				msSymbolName = MyStringEx(ss.str());
		}
	}
	virtual void flush() override { reset(); }
protected:
	virtual void reset()
	{
		mva = 0;
		meKind = I_Front::SYMK_NULL;
		moSymbolName = OFF_NULL;
		msSymbolName.clear();
		moModuleName = OFF_NULL;
		msModule.clear();
	}
};

class DumpSymbol_t : public I_Front::I_DumpSymbol
{
	SymbolInfo& msi;
public:
	DumpSymbol_t(SymbolInfo& si) : msi(si){}
protected:
	virtual void setVA(ADDR va) override { msi.mva = va; }
	virtual void setKind(I_Front::SymbolKind e) override { msi.meKind = e; }
	virtual void setName(const char* s) override {
		msi.msSymbolName = MyStringEx(s);
	}
	virtual void setName(OFF_t o) override { msi.moSymbolName = o; msi.muNameMax = 0; }
	virtual void flush() override {}
};


//////////////////////////////////////////////////// SymbolMap
class SymbolMap : public std::vector<SymbolInfo>
{
	const I_DataSource &mRaw;
	I_Front *mpIFront;
	std::set<MyString> mSrc;
public:
	SymbolMap(I_DataSource &, I_Front *pIFront);
	const I_DataSource &dataSource() const { return mRaw; }
	I_Front::SymbolKind demangled(const MyString &in, MyString& out) const
	{
		return demangleSymbol(mpIFront, in, out);
	}
	void addSrc(const char *path){
		mSrc.insert(path);
	}
	const std::set<MyString> &src() const { return mSrc; }

	void dumpExported();
	void dumpImported();
	void dumpDebugInfo1(const ModuleInfo_t &);

	class Iterator
	{
		const SymbolMap &m_map;
		size_t m_index;
	public:
		Iterator(const SymbolMap &m)
			: m_map(m),
			m_index(0)
		{
		}
		size_t index() const { return m_index; }
		Iterator& operator ++(){
			m_index++;
			return *this;
		}
		MyStringEx symbol() const {
			const SymbolInfo &r(m_map.at(m_index));
			if (r.msSymbolName)
			{
				//if (r.uNameMax == 0)
					return r.msSymbolName;
				//return MyString(r._pSymbolName, r.uNameMax);
			}
			std::ostringstream ss;
			if (r.moSymbolName != OFF_NULL)
			{
				DataStream_t aPtr(m_map.dataSource(), r.moSymbolName);
				aPtr.fetchString(ss, r.muNameMax);
			}
			return MyStringEx(ss.str());
		}
		MyString symbolTag() const {
			return symbol()[1];
			/*const SymbolInfo& r(m_map.at(m_index));
			if (!r.sSymbolTag.empty())
				return r.sSymbolTag;
			std::ostringstream ss;
			if (r.oSymbolTag != OFF_NULL)
			{
				DataStream_t aPtr(m_map.dataSource(), r.oSymbolTag);
				aPtr.fetchString(ss);
			}
			return ss.str();*/
		}
		I_Front::SymbolKind demangled(MyString& out) const {
			return m_map.demangled(symbol(), out);
		}
		Iterator& operator ++(int) {
			return operator++();
		}
		operator bool() const {
			return m_index < m_map.size();
		}
		ADDR va() const {
			return m_map.at(m_index).mva;
		}
		bool isFunc() const {
			return m_map.at(m_index).meKind == I_Front::SYMK_FUNC;
		}
	};

private:
	class SymbolFetch_t : public DumpSymbolBase_t
	{
		SymbolMap &mSymbols;
	public:
		SymbolFetch_t(SymbolMap &m)
			: DumpSymbolBase_t(m.dataSource()),
			mSymbols(m)
		{
		}
	protected:
		virtual void flush()
		{
			SymbolInfo a;
			a.moSymbolName = moSymbolName;
			a.msSymbolName = msSymbolName;
			a.muNameMax = 0;
			a.mva = mva;
			a.meKind = meKind;
			mSymbols.push_back(a);
			DumpSymbolBase_t::flush();
		}
	};

};



class DumpDebugInfoCallback : public FrontImplBase_t//public I_ModuleCB
{
	SymbolMap &symIt;
public:
	DumpDebugInfoCallback(Project_t &rProj, TypePtr iModule, SymbolMap &);
protected:
	friend class FrontIfaceDbg_t;
	/*void selectFile(const char *path)
	{
		MyString s(fixFile Name(MyString(path), binary()));
		mpFolder = AddFile(s);
		//AssureFileDef(mpFolder);
	}*/

	void dump(ADDR va, OFF_t oSymbolName, unsigned uNameMax, unsigned uFlags);
	void dump(ADDR va, const char *pSymbolName, unsigned uFlags);
	void dumpSrc(const char *path);
	void resetProgress(const char *, unsigned);

	//class I_DataSourceBase
	size_t dataAt(OFF_t off, OFF_t siz, PDATA) const { assert(0); return 0; }
	OFF_t size() const { assert(0); return 0; }
};


class FrontIfaceDbg_t : public IFront_Base<I_ModuleCB>
{
	DumpDebugInfoCallback& mr;
public:
	FrontIfaceDbg_t(DumpDebugInfoCallback& r)
		: IFront_Base<I_ModuleCB>(r),
		mr(r)
	{
	}
protected:
	//virtual void selectFile(const char *path){}

	virtual void dump(ADDR va, OFF_t oSymbolName, unsigned uNameMax, unsigned uFlags) override { mr.dump(va, oSymbolName, uNameMax, uFlags); }
	virtual void dump(ADDR va, const char *pSymbolName, unsigned uFlags) override { mr.dump(va, pSymbolName, uFlags); }
	virtual void dumpSrc(const char *path) override { mr.dumpSrc(path); }
	virtual void resetProgress(const char *pc, unsigned u) override { mr.resetProgress(pc, u); };

	//class I_DataSourceBase
	virtual size_t dataAt(OFF_t off, OFF_t siz, PDATA) const override { assert(0); return 0; }
	virtual OFF_t size() const override { assert(0); return 0; }
	virtual I_Front* frontend() const { assert(0); return nullptr; }
};