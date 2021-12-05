#pragma once

#include "PDB.h"
#include "shared/front.h"
#include <stdint.h>


namespace PDB {

	/////////////////////////////////////////////////////////// CTypeFactory
	// reconstruct a type (at given position) based on data from TPI stream
	class CTypeFactory
	{
	protected:
		const pdb::MyPdbReader& mrPdbReader;
		const I_DataSourceBase& mrTpiStrm;
		std::map<OFF_t, HTYPE>	mTypes;//cached TPI type refs (but no enum targets)
		std::map<OFF_t, HTYPE>	mTypesEnum;//enum targets
		std::map<std::string, OFF_t> mTypeNames;
	public:
		CTypeFactory(const pdb::MyPdbReader& reader);
		void process(I_Module&, OFF_t);
		OFF_t typeOffsetFromIndex(int i) const {
			return mrPdbReader.TypeOffsetFromIndex(i);
		}
		void processFunction(OFF_t, I_Module&, const std::string& name);
		void processVariable(pdb::CV_typ_t, I_Module&, const std::string& name);
	protected:
		bool fetchSymbolName(unsigned index, std::string&);
		HTYPE processStruct(OFF_t oLeaf, I_Module&);
		HTYPE fetchType(unsigned index, I_Module& rICb);
		HTYPE fetchType1(OFF_t, I_Module& rICb);
		HTYPE fetchBasicType(unsigned short index, I_Module& rICb);
		HTYPE processEnum(OFF_t, I_Module&, bool bRef);
		HTYPE processUnion(OFF_t, I_Module&);
		HTYPE processPtr(OFF_t, I_Module&);
		HTYPE processProcedureType(OFF_t, I_Module&);
		HTYPE processMethodType(OFF_t, I_Module&);
		HTYPE processArgslistType(OFF_t, I_Module&, HTYPE);
		int fetchOffset(DataStream_t& o);
		void processProcedure(OFF_t, I_Module&, const std::string& name);
		void processMethod(OFF_t, I_Module&, const std::string& name);
		void processArgslist(OFF_t, I_Module&);
		bool skipNumeric(unsigned short value, DataStream_t&, value_t& v);
		void processFields(OFF_t oFields, I_Module&, bool bUnion);
		void processEnumFields(OFF_t oFields, int count, I_Module&);
		size_t fetchGlobalName(DataStream_t&, std::string&, OFF_t oLeafId = 0);
		//protected:
			//virtual HTYPE NewScope(const char *typeName, SCOPE_enum, const char *fieldName, AttrIdEnum, I_Module &, OFF_t);
		void registerType(OFF_t, HTYPE, const char*, bool bEnum = false);
	};

	/////////////////////////////////////////////////////////// CIpiTypeGenerator
	// reconstruct all user types from PDB (iteratively) using IPI stream as a driver
	class CIpiTypeGenerator : public CTypeFactory
	{
		pdb::MyPdbReader::ITypeInfoIterator mit;
	public:
		CIpiTypeGenerator(const pdb::MyPdbReader& reader);
		bool process(I_Module& rICb);//, unsigned &progress);
		unsigned progress() const { return mit.progress(); }
		//protected:
			//virtual HTYPE NewScope(const char *typeName, SCOPE_enum, const char *fieldName, AttrIdEnum, I_Module &, OFF_t);
	};

	///////////////////////////////////////////////////////////////// CCodeViewSymbolWalker
	class CCodeViewSymbolWalker : protected DataStream_t
	{
		OFF_t moUpper;
		pdb::TypeRecord aRec;

	public:
		CCodeViewSymbolWalker(const I_DataSourceBase& data, OFF_t bytes, bool bSig = true)
			: DataStream_t(data),
			moUpper(bytes)
		{
			if (bytes == OFF_NULL)
				moUpper = data.size();
			if (bSig)
			{
				DataFetch_t<uint32_t> aSig(data, 0);//check a signature
				if (aSig >= CV_SIGNATURE_RESERVED)
					invalidate();
				forward((unsigned)aSig.size());
			}
		}
		CCodeViewSymbolWalker(const I_DataSourceBase& data, OFF_t start, OFF_t upper)
			: DataStream_t(data, start),
			moUpper(upper)
		{
		}
		void operator=(OFF_t off) {
			assert(off < moUpper);
			seek(off);
		}
		operator bool() const {
			return tell() < moUpper;
		}
		void operator++() {
			uint16_t reclen;
			read<uint16_t>(reclen);
			forward(reclen);
		}
		pdb::SYM_ENUM_e symbol() const {
			DataStream_t aStrm(*this);
			aStrm.forward(sizeof(uint16_t));//TypeRecord.length
			return (pdb::SYM_ENUM_e)aStrm.read<uint16_t>();//TypeRecord.leaf
		}
		OFF_t upper() const {
			return moUpper;
		}
		OFF_t current() const {
			return tell();
		}
		void setCurrent(OFF_t o){
			seek(o);
		}
		const DataStream_t &dataStream() const { return *this; }
		unsigned progress() const {
			return TProgress(current(), upper());
		}
	private:
		void invalidate()
		{
			moUpper = 0;
		}
	};

	//////////////////////////////////////////////////////////////////////
	class CModuleInfoWalker : public pdb::MyPdbReader::ModInfoIterator
	{
		typedef pdb::MyPdbReader::ModInfoIterator Base;
		CCodeViewSymbolWalker *mpSymIt;
		std::string mModuleName;
	public:
		CModuleInfoWalker(const pdb::MyPdbReader& reader)//, unsigned _streamId = 0)
			: pdb::MyPdbReader::ModInfoIterator(reader),
			mpSymIt(nullptr)
		{
			if (nextValidModule())
			{
				/*if (_streamId != 0)
					while (_streamId != streamId())
					{
						Base::operator++();
						if (!nextValidModule())
							return;
					}*/
				fetchModuleName();
				mpSymIt = new CCodeViewSymbolWalker(data(), SymByteSize, true);
			}
		}
		~CModuleInfoWalker()
		{
			delete mpSymIt;
		}
		bool nextModule()
		{
			delete mpSymIt;
			mpSymIt = nullptr;
			Base::operator++();//advance module
			if (!nextValidModule())
				return false;
			fetchModuleName();
			mpSymIt = new CCodeViewSymbolWalker(data(), SymByteSize, true);
			return true;
		}
		void operator++()
		{
			++(*mpSymIt);//advance symbol
			if (!(*mpSymIt))
				nextModule();
		}
		pdb::SYM_ENUM_e symbol() const {
			return mpSymIt->symbol();
		}
		const DataStream_t &dataStream() const {
			return mpSymIt->dataStream();
		}
		OFF_t current() const {
			return mpSymIt->current();
		}
		void setCurrent(OFF_t o){
			mpSymIt->setCurrent(o);
		}
		const std::string &moduleName() const {
			return mModuleName;
		}
		size_t sourceName(uint16_t seg, uint32_t off, std::ostream &os) const {
			return mPdbReader.NameFromAddressInModule(ModuleSymStream, seg, off, os);
		}
		/*size_t sourceName(uint16_t seg, uint32_t off, std::ostream &os) const
		{
			const pdb::MyPdbReader::StreamInfo_t &a(mPdbReader.StreamInfo(ModuleSymStream));
			OFF_t oFileChksms(mPdbReader.FileChecksumOffset(ModuleSymStream));
			if (oFileChksms == 0)
				return 0;
			pdb::MyPdbReader::StreamInfo_t::AddrRangeMapCIt i(a.findRange(seg, off));
			if (i == a.pRanges->end())
				return 0;
			OFF_t oName(mPdbReader.NameOffsetFromIndex(oFileChksms + i->fileid));
			DataStream_t p(mPdbReader.NamesStream(), oName);
			return p.fetchString(os);
		}*/
	private:
		bool nextValidModule()
		{
			CModuleInfoWalker &j(*this);
			while (j && !j.isValid())
				Base::operator++();//advance module
			return isValid();
		}
		void fetchModuleName()
		{
			std::ostringstream ss;
			Base::moduleName(ss);
			mModuleName = ss.str();
		}
	};


	//////////////////////////////////////////////////////// CSymbolProcessor
	template <typename T_PE>
	class CSymbolProcessor
	{
	protected:
		typedef pdb::MyPdbReader::ModInfoIterator CModInfoIterator;

		const T_PE& mrpe;
		I_DataSourceBase* mpIPdb;
		pdb::MyPdbReader* mpPdbReader;
		CTypeFactory* mpTypeFactory;
		CIpiTypeGenerator* mpIpiTypeGenerator;//weak
		CModuleInfoWalker* mpModuleSymbolProcessor;//in-module symbols
		CCodeViewSymbolWalker* mpPublicSymbolProcessor;//when the module symbols stripped, process symrec stream
		unsigned mPhase;//0:types,1:symbols
	public:
		CSymbolProcessor(const T_PE& rpe);
		virtual ~CSymbolProcessor();
		void createProcessors(bool bNoTypeInfo, bool bNoPubSyms);
		int createA(const std::string&);
		void fetchOwnerFile(I_Module& rICb, uint16_t seg, uint32_t off);
		int fetchGlobalName(DataStream_t& aStrm, std::string& s);
		virtual bool processNextType(I_ModuleCB& rICb, unsigned&);
		virtual bool processNext(I_ModuleCB& rICb, unsigned& progress);//module symbols
		void processPublicSymbol(I_ModuleCB& rICb, pdb::SYM_ENUM_e symb, const DataStream_t& aSrc);
		bool processSymbol(I_ModuleCB& rICb, pdb::SYM_ENUM_e e, const DataStream_t& aSrc, OFF_t& oNext);//return true if oNext is set (for skip support)
		void printModuleSymbols();
		void dumpLines();
	private:
		bool printMsg(bool bMatched, int ver);
	};



	template <typename T_PE>
	class CSymbolProcessorOld : public CSymbolProcessor<T_PE>
	{
		CCodeViewSymbolWalker* mpPublicProcessor;//public symbols

	public:
		CSymbolProcessorOld(const T_PE& rpe);
		~CSymbolProcessorOld();
		//bool processNextType(I_ModuleCB& rICb, unsigned&);// progress)
		virtual bool processNext(I_ModuleCB& rICb);//, unsigned&)// progress)//global symbols
		void initPdb0();
	};

}//namespace PDB






