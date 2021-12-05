#include "format.pdb.h"
#include "shared.h"
#include "PDB.h"
#include "cv/cvinfo.h"

//using namespace pdb;

namespace PDB20 {

	/////////////////////////////////////////////////////////
	////////////////////////////////////////////////////T_PDB (top)
	/////////////////////////////////////////////////////////

	class CDynamicType_PDB : public I_FormatterType
	{
		class T_PDB : public T_PDB_base
		{
			HTYPE	mpe;
		public:
			T_PDB(I_SuperModule& rMain) : T_PDB_base(rMain) {}
			void declBlock(unsigned oAt, size_t blockSize, int iStreamId, int iBlockId)
			{
				OFF_t oBlock(oAt * blockSize);
				mr.setcp((POSITION)oBlock);
				char buf[64];
				if (iStreamId < 0)
					sprintf(buf, "directory_block_%d", iBlockId);
				else
					sprintf(buf, "stream_%d_block_%d", iStreamId, iBlockId);
				mr.declField(buf, mr.arrayOf(mr.type(TYPEID_BYTE), (int)blockSize), ATTR_COLLAPSED);
			}

			void preformat()
			{
				mr.installFrontend(_PFX("FE_PDB"));
				PDB20_CreateStructures(mr);
				PDB20_DeclareDynamicTypes(mr);

				DECLDATAEX(PDB20::SuperBlock, aHeader);
				mr.declField("aHeader", mr.type("PDB_HEADER"));

				DECLDATAPTR(uint16_t, oStreamDirBlockIndex);

				uint32_t directorySize(aHeader.RootStream.StreamSize);
				int uStreamDirIndicesNum((int)ceil((double)directorySize / aHeader.PageSize));
				mr.declField("aStreamDirIndices", mr.arrayOf(mr.type(TYPEID_USHORT), uStreamDirIndicesNum), (AttrIdEnum)ATTR_PDB20_BLK);

				//the stream directory itself may also be scattered, so we need a consistent way to access it's contents
				HPATCHMAP hStreamDir(mr.newPatchMap());
				for (int i(0); directorySize > 0; i++)
				{
					uint32_t blockSize(std::min(directorySize, aHeader.PageSize));
					directorySize -= blockSize;
					mr.addPatch(hStreamDir, *oStreamDirBlockIndex * aHeader.PageSize, blockSize);
					declBlock(*oStreamDirBlockIndex, aHeader.PageSize, -1, i);
					++oStreamDirBlockIndex;
				}
				I_DataSourceBase* pDirDS(mr.stitchDerivativeModule(hStreamDir, "$directory", _PFX("PDB20_DIR"), false, 1));
			}
		};
	protected:
		virtual const char* name() const { return "MS_PDB20"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_PDB a(r);
			a.preformat();
		}
	};
	DECLARE_FORMATTER(CDynamicType_PDB, MS_PDB20);



	/////////////////////////////////////////T_PDB20_DIR
	class CFormatter_DIR : public I_FormatterType
	{
		//pre-processed PDB stream directory
		class T_Impl : public T_PDB_base
		{
			HTYPE	mpe;
			PDB20::SuperBlock	mHeader;
		public:
			T_Impl(I_SuperModule& rMain) : T_PDB_base(rMain), mpe(nullptr)
			{
				const I_DataSourceBase* pIHost(rMain.host());
				pIHost->dataAt(0, sizeof(PDB20::SuperBlock), (PDATA)&mHeader);
			}
			void preformat()
			{
				mr.reuseFrontend(0);

				if (mr.NewScope(mr.declField("aStreamDirectory")))
				{
					SAFE_SCOPE_HERE(mr);
					mr.installNamespace();
					DECLDATA(uint16_t, uNumStreams);
					mr.declField("NumStreams", mr.type(TYPEID_UINT16), ATTR_DECIMAL);
					mr.skip(sizeof(uint16_t));
					DECLDATAPTREX(PDB_STREAM, pStreamSizes);
					mr.declField("StreamSizes", mr.arrayOf(mr.type("PDB_STREAM"), uNumStreams));

					if (mr.NewScope(mr.declField("aStreamBlocks")))
					{
						SAFE_SCOPE_HERE(mr);
						int iWidth(PrintWidth(uNumStreams.get()));
						char fmt[32];
						sprintf(fmt, "stream_%%0%dd", iWidth);

						for (size_t i(0); i < uNumStreams; i++, ++pStreamSizes)
						{
							const PDB_STREAM &uStreamDesc(*pStreamSizes);
							unsigned uStreamSize(uStreamDesc.StreamSize);
							if ((int)uStreamSize <= 0)
								continue;
							int n((int)ceil((double)uStreamSize / mHeader.PageSize));
							assert(n > 0);
							DECLDATAPTR(uint16_t, pStreamBlocks);
							char buf[32];
							sprintf(buf, fmt, i);
							mr.declField(buf, mr.arrayOf(mr.type(TYPEID_UINT16), n), ATTR_COLLAPSED);
#if(0)
							if (i > 15) break;
#endif
							{
								//if (n > 1)//Fork PDB DBI Stream
								{
									HPATCHMAP hDBI(mr.newPatchMap());
									while (uStreamSize > 0)
									{
										uint32_t blockSize(std::min(uStreamSize, mHeader.PageSize));
										uStreamSize -= blockSize;
										mr.addPatch(hDBI, *pStreamBlocks * mHeader.PageSize, blockSize);
										++pStreamBlocks;
									}
									char buf[32];
									sprintf(buf, fmt, (int)i);
									switch (i)
									{
									//case 0:
										/*	strcat(buf, "$directory_old");
											mr.stitchDerivativeModule(hDBI, buf, nullptr, true, STREAM_INDEX_TO_MODULE_ID(i));
											break;*/
										case 1:
											strcat(buf, "$PDB");
											mr.stitchDerivativeModule(hDBI, buf, _PFX("PDB20_PDB"), false, STREAM_INDEX_TO_MODULE_ID(i));
											break;
										case 2:
											strcat(buf, "$TPI");
											mr.stitchDerivativeModule(hDBI, buf, _PFX("PDB20_TPI"), true, STREAM_INDEX_TO_MODULE_ID(i));
											break;
										case 3:
											strcat(buf, "$DBI");
											mr.stitchDerivativeModule(hDBI, buf, _PFX("PDB20_DBI"), false, STREAM_INDEX_TO_MODULE_ID(i));
											break;
										/*case 4:
											strcat(buf, "$IPI");
											mr.stitchDerivativeModule(hDBI, buf, _PFX("PDB20_TPI"), true, STREAM_INDEX_TO_MODULE_ID(i));
											break;*/
									default:
										mr.stitchDerivativeModule(hDBI, buf, nullptr, true, STREAM_INDEX_TO_MODULE_ID(i));
										break;
									}
								}
							}
						}
					}
				}
			}
		};
	public:
		virtual const char* name() const { return "PDB20_DIR"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl impl(r);
			impl.preformat();
		}
	};
	DECLARE_FORMATTER(CFormatter_DIR, PDB20_DIR);



	////////////////////////////////////////////T_PDB_PDB(#1)
	class CFormatter_PDB : public I_FormatterType
	{
		//pre-processed PDB PDB stream (index 1)
		class T_Impl : public T_PDB_base
		{
			HTYPE	mpe;
		public:
			T_Impl(I_SuperModule& rMain) : T_PDB_base(rMain), mpe(nullptr) {}
			void preformat()
			{
				createStructures2();

				DECLDATAEX(NameIndexHeader, aHeader);
				mr.declField("aHeader", mr.type("NameIndexHeader"));
				POSITION nameStart(mr.cp());
				POSITION end(mr.cp() + aHeader.NamesOffset);
				if (mr.cp() != end)
					mr.setcp(end);
				DECLDATA(uint32_t, numOk);
				mr.declField("numOk", mr.type(TYPEID_UINT32));//0
				DECLDATA(uint32_t, count);
				mr.declField("count", mr.type(TYPEID_UINT32));//1
				DECLDATA(uint32_t, skip);
				mr.declField("skip", mr.type(TYPEID_UINT32));//2
				if ((count / 32) > skip)
					return;//Invalid name index
				//int n(((count / 32) + 1));// * sizeof(uint32_t));
				int n(EXTENDED(skip.get(), sizeof(uint32_t)));
				DECLDATAPTR(uint32_t, okBits);
				mr.declField("okBits", mr.arrayOf(mr.type(TYPEID_DWORD), n), ATTR_BINARY);
				POSITION deletedOffset(mr.cp());
				DECLDATA(uint32_t, deletedskip);
				mr.declField("deletedskip", mr.type(TYPEID_UINT32));//0
				//?DECLDATAPTR(uint32_t, deletedBits);
				if (deletedskip != 0)
				{
					n = EXTENDED(deletedskip.get(), sizeof(uint32_t));
					mr.declField("deletedBits", mr.arrayOf(mr.type(TYPEID_DWORD), n), ATTR_BINARY);
					//deletedOffset += (deletedskip + 1) * sizeof(uint32_t);
					//mr.error("PDB#1: deleted names bitset is not empty, unsupported");
					//return;
				}
				//mr.setcp(deletedOffset + (deletedskip + 1) * sizeof(uint32_t));
				struct StringVal
				{
					uint32_t id;
					uint32_t stream;
				};
				DECLDATAPTREX(StringVal, val);
				for (uint32_t i = 0; i < count; ++i)
				{
					if (!(okBits[i / 32] & (1 << (i % 32))))
						continue;

					mr.declField(nullptr, mr.type("StringVal"));
					if (mr.EnterScope(nameStart + val.id))
					{
						SAFE_SCOPE_HERE(mr);
						DataStream_t pStr(mr, mr.cpr());
						if (pStr.strCmp("/names") == 0)
						{
							//char buf1[32];
							//sprintf(buf1, "stream_#%d", (int)val.stream);
							//I_DataSourceBase *pIS(mr.module(buf1));
							//if (pIS)
							mr.preformatModule(STREAM_INDEX_TO_MODULE_ID(val.stream), _PFX("PDB_NAMES"), true);
						}
						mr.declField(nullptr, toAscii(mr));
					}
					++val;
				}
			}
		private:
			void createStructures2()
			{
				if (mr.NewScope("NameIndexHeader"))
				{
					mr.declField("Version", mr.enumOf(mr.type("PdbStreamVersion"), TYPEID_UINT32));
					mr.declField("TimeDateStamp", mr.type(TYPEID_UINT32));
					mr.declField("Age", mr.type(TYPEID_UINT32));
					mr.declField("NamesOffset", mr.type(TYPEID_UINT32));
					mr.Leave();
				}
				if (mr.NewScope("StringVal"))
				{
					mr.declField("NameOffset", mr.type(TYPEID_UINT32));
					mr.declField("Stream", mr.type(TYPEID_UINT32));
					mr.Leave();
				};
			}
		};
	protected:
		virtual const char* name() const { return "PDB20_PDB"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl impl(r);
			impl.preformat();
		}
	};
	DECLARE_FORMATTER(CFormatter_PDB, PDB20_PDB);




	
	////////////////////////////////////////////T_PDB_TPI(#2)
	class CFormatter_TPI : public I_FormatterType
	{
		//pre-processed PDB PDB stream (index 1)
		class T_Impl : public T_PDB_base
		{
			HTYPE	mpe;
		public:
			T_Impl(I_SuperModule& rMain) : T_PDB_base(rMain), mpe(nullptr) {}
			void preformat()
			{
				mr.reuseFrontend(0);//resuse the one of primary module _PFX("FE_PDB"));

				DECLDATAEX(pdb::TypeInfoHeader, tih);
				mr.declField("aHeader", mr.type("TypeInfoHeader"));
				POSITION end(tih.headerSize);
				mr.setcp(end);
				if (mr.NewScope(mr.declField("aTypeInfo")))
				{
					SAFE_SCOPE_HERE(mr);

					for (uint32_t i(tih.min); i < tih.max; ++i)
					{
//CHECK(i == 0x100e)
//STOP
						mr.setcp(end);

						DECLDATAEX(pdb::TypeRecord, tr);
						if (tr.leaf == 0)
						{
							end += sizeof(pdb::TypeRecord);
							mr.declField(nullptr, mr.type("TypeRecord"));
						}
						else
						{
							end += sizeof(uint16_t);//skip a length field of TypeRecord
							end += tr.length;
							mr.declField(nullptr, mr.type(_PFX("CodeView_LEAF")), ATTR_NEWLINE);
						}
					}
				}
			}
		};

	protected:
		virtual const char* name() const { return "PDB20_TPI"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl impl(r);
			impl.preformat();
		}
	};
	DECLARE_FORMATTER(CFormatter_TPI, PDB20_TPI);






	////////////////////////////////////////////T_PDB_DBI(#3)
	class CFormatter_DBI : public I_FormatterType
	{
		//pre-processed PDB DBI stream (index 3)
		class T_Impl : public T_PDB_base
		{
			HTYPE	mpe;
		public:
			T_Impl(I_SuperModule& rMain) : T_PDB_base(rMain), mpe(nullptr) {}
			void preformat()
			{
				mr.reuseFrontend(0);

				DECLDATAEX(pdb::DbiStreamHeader, aDbiSH);
				mr.declField("aDbiStreamHeader", mr.type("DbiStreamHeader"));

				//char buf1[32];
#if(0)
				//sprintf(buf1, "stream_#%d", (int)aDbiSH.GlobalStreamIndex);
				mr.preformatModule(STREAM_INDEX_TO_MODULE_ID(aDbiSH.GlobalStreamIndex), _PFX("PDB20_GSI"), true);

				//sprintf(buf1, "stream_#%d", (int)aDbiSH.PublicStreamIndex);
				mr.preformatModule(STREAM_INDEX_TO_MODULE_ID(aDbiSH.PublicStreamIndex), _PFX("PDB_PSI"), true);
#endif
				//sprintf(buf1, "stream_#%d", (int)aDbiSH.SymRecordStream);
				mr.preformatModule(STREAM_INDEX_TO_MODULE_ID(aDbiSH.SymRecordStream), _PFX("PDB20_SYMREC"), true);

				if (aDbiSH.ModInfoSize > 0)
				{
					if (mr.NewScope(mr.declField("aModInfoSubstream", ATTR_COLLAPSED)))
					{
						SAFE_SCOPE_HERE(mr);
						POSITION end((POSITION)mr.cpr() + aDbiSH.ModInfoSize);
						//for (int k(0); k < 5; k++)
						while (mr.cpr() < end)
						{
							OFF_t z = mr.cpr();
							mr.align(ALIGN_DWORD);
							OFF_t oModInfo(mr.cpr());
							DECLDATAEX(pdb::ModInfo, aModInfo);
							mr.declField(nullptr, mr.type(_PFX("ModInfo")), ATTR_COLLAPSED);

							I_DataSourceBase* pIS(mr.module(STREAM_INDEX_TO_MODULE_ID(aModInfo.ModuleSymStream)));
							if (pIS)
							{
								pIS->setAuxData((PDATA)&oModInfo, sizeof(oModInfo));
								mr.preformatModule(STREAM_INDEX_TO_MODULE_ID(aModInfo.ModuleSymStream), _PFX("PDB20_MOD"), true);
							}
						}
					}
				}

				if (aDbiSH.SectionContributionSize > 0)
				{
					if (mr.NewScope(mr.declField("aSectionContributionSubstream", ATTR_COLLAPSED)))
					{
						DECLDATA(uint32_t, aSectionContrSubstreamVersion);
						mr.declField("aSectionContrSubstreamVersion", mr.enumOf(mr.type("SectionContrSubstreamVersion"), TYPEID_UINT32));
						int nSectionContrSubstreamNum(aDbiSH.SectionContributionSize / sizeof(pdb::SectionContribEntry));
						const char* pcTypeName("SectionContribEntry");
						if (aSectionContrSubstreamVersion == pdb::SectionContrSubstreamVersion::V2)
						{
							nSectionContrSubstreamNum = aDbiSH.SectionContributionSize / sizeof(pdb::SectionContribEntry2);
							pcTypeName = "SectionContribEntry2";
						}
						mr.declField("aSectionsArray", mr.arrayOf(mr.type(pcTypeName), nSectionContrSubstreamNum), ATTR_COLLAPSED);
						mr.Leave();
					}
				}

				if (aDbiSH.SectionMapSize > 0)
				{
					if (mr.NewScope(mr.declField("aSectionMapSubstream", ATTR_COLLAPSED)))
					{
						DECLDATAEX(pdb::SectionMapHeader, aSMH);
						mr.declField("aSectionMapHeader", mr.type("SectionMapHeader"));
						int n(aDbiSH.SectionMapSize / sizeof(pdb::SectionMapEntry));
						mr.declField("aSegDescriptors", mr.arrayOf(mr.type("SectionMapEntry"), n), ATTR_COLLAPSED);
						mr.Leave();
					}
				}

				if (aDbiSH.SourceInfoSize > 0)
				{
					if (mr.NewScope(mr.declField("aFileInfoSubstream", ATTR_COLLAPSED)))
					{
						POSITION beg(mr.cp());
						DECLDATA(uint16_t, aNumModules);
						mr.declField("NumModules", mr.type(TYPEID_UINT16), ATTR_DECIMAL);
						DECLDATA(uint16_t, aNumSourceFiles);
						mr.declField("NumSourceFiles", mr.type(TYPEID_UINT16), ATTR_DECIMAL);
						if (aNumModules > 0)
						{
							mr.declField("ModIndices", mr.arrayOf(mr.type(TYPEID_UINT16), aNumModules), ATTR_COLLAPSED);
							mr.declField("ModFileCounts", mr.arrayOf(mr.type(TYPEID_UINT16), aNumModules), ATTR_COLLAPSED);
						}
						if (aNumSourceFiles > 0)
						{
							mr.declField("FileNameOffsets", mr.arrayOf(mr.type(TYPEID_UINT32), aNumSourceFiles), ATTR_COLLAPSED);
							POSITION end(beg + aDbiSH.SourceInfoSize);
							while (mr.cp() < end)
								mr.declField(nullptr, toAsciiPre(mr));//! length(byte)-prepended (255 max)
						}
						mr.Leave();
					}
				}

				if (aDbiSH.TypeServerSize > 0)
					mr.declField("TypeServerSubstream", mr.arrayOf(mr.type(TYPEID_BYTE), aDbiSH.TypeServerSize), ATTR_COLLAPSED);

				if (aDbiSH.ECSubstreamSize > 0)
				{//all are unknown
					if (mr.NewScope(mr.declField("aECSubstream", ATTR_COLLAPSED)))
					{
						POSITION beg0(mr.cp());
						mr.declField("Unknown1", mr.type(TYPEID_UINT32));
						mr.declField("Unknown2", mr.type(TYPEID_UINT32));
						DECLDATA(uint32_t, uSize);
						mr.declField("StringPoolSize", mr.type(TYPEID_UINT32));
						POSITION beg(mr.cp());
						POSITION end(beg + uSize);
						while (mr.cp() < end)
							mr.declField(nullptr, toAscii(mr));
						POSITION cur(mr.cp());
						int iTheRest(aDbiSH.ECSubstreamSize - (cur - beg0));
						if (iTheRest > 0)
							mr.declField("Unknown3", mr.arrayOf(mr.type(TYPEID_BYTE), iTheRest));
						mr.Leave();
					}
				}

				if (aDbiSH.OptionalDbgHeaderSize > 0)
				{
					DECLDATAPTR(uint16_t, pDebugHeader);
					if (aDbiSH.OptionalDbgHeaderSize == sizeof(pdb::DBIDebugHeader))
						mr.declField("aOptionalDbgHeader", mr.type("DBIDebugHeader"));
					else
					{
						uint16_t n(uint16_t(aDbiSH.OptionalDbgHeaderSize / sizeof(uint16_t)));
						//mr.declField("aOptionalDbgHeader", mr.arrayOf(mr.type(TYPEID_UINT16), n));

						mr.declField("aOptionalDbgHeader", mr.arrayOfIndex(
							mr.arrayOf(mr.type(TYPEID_UINT16), n),
							mr.enumOf(mr.type("DBGTYPE"), TYPEID_BYTE)
						));
					}

					if (pdb::dbgtypeSectionHdr < aDbiSH.OptionalDbgHeaderSize / sizeof(uint16_t))
					{
						//DECLDATAEX(pdb::DBIDebugHeader, debugHeader);
						//uint16_t sectionHdr(debugHeader.sectionHdr);
						uint16_t sectionHdr(pDebugHeader[pdb::dbgtypeSectionHdr]);
						if (sectionHdr != 0xFFFF)
						{
							//char buf1[32];
							//sprintf(buf1, "stream_#%d", (int)sectionHdr);
							//I_DataSourceBase *pIS(mr.module(buf1));
							//if (pIS)
							mr.preformatModule(STREAM_INDEX_TO_MODULE_ID(sectionHdr), _PFX("PDB_ISH"), true);
						}
					}
				}

				//?mr.Leave();
			}
		};
	protected:
		virtual const char* name() const { return "PDB20_DBI"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl impl(r);
			impl.preformat();
		}
	};
	DECLARE_FORMATTER(CFormatter_DBI, PDB20_DBI);



	
	////////////////////////////////////////////CFormatter_SYMREC(#?)
	class CFormatter_SYMREC : public I_FormatterType
	{
		//pre-processed PDB NAMES
		class T_Impl : public T_PDB_base
		{
			HTYPE	mpe;
		public:
			T_Impl(I_SuperModule& rMain) : T_PDB_base(rMain), mpe(nullptr) {}
			void preformat(I_SuperModule&, unsigned long dataSize)
			{
				mr.reuseFrontend(0);
				POSITION end(mr.cp());
				for (;;)
				{
					POSITION end(mr.align(ALIGN_DWORD));
					if (end >= dataSize)
						break;
					//DECLDATAPTR(uint16_t, pRec);
					//end += (POSITION)pRec.size() + pRec[0];
					mr.declField(nullptr, mr.type(_PFX("CodeView_SYMBOL")), ATTR_NEWLINE);
					//mr.setcp(end);
				}
			}
		};
	public:
		virtual const char* name() const { return "PDB20_SYMREC"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl a(r);
			a.preformat(r, nSize);
		}
	};
	DECLARE_FORMATTER(CFormatter_SYMREC, PDB20_SYMREC);


#if(0)
	static void declHash(I_Module& mr)
	{
		DECLDATAEX(pdb::GSIHashHdr, header);

		mr.declField("aHeader", mr.type("GSIHashHdr"));

		unsigned n(header.cbHr / sizeof(pdb::HRFile));
		if (n > 0)
			mr.declField("aBuffer", mr.arrayOf(mr.type("HRFile"), n), ATTR_COLLAPSED);

		unsigned m(header.cbBuckets);
		if (m > 0)
		{
			const int iphrHash = 4096;
			const int iphrHash_b = iphrHash / CHAR_BIT;
			if (m > iphrHash_b)
			{
				mr.declField("aBucketsBitvec", mr.arrayOf(mr.type(TYPEID_BYTE), iphrHash_b), AttrIdEnum(ATTR_BINARY | ATTR_COLLAPSED));
				m -= iphrHash_b;
			}
			m /= sizeof(int32_t);
			if (m > 0)
				mr.declField("aComprBuckets", mr.arrayOf(mr.type(TYPEID_UINT32), m), ATTR_COLLAPSED);//, (AttrIdEnum)ATTR_GSI_OFF_MH);//incorrect!
		}
	}


	/////////////////////////////////////////////T_PDB_GSI
	class CFormatter_GSI : public I_FormatterType
	{
		class T_Impl : public T_PDB_base
		{
			HTYPE	mpe;
		public:
			T_Impl(I_SuperModule& rMain) : T_PDB_base(rMain), mpe(nullptr) {}
			void preformat(I_SuperModule&, unsigned long dataSize)
			{
				mr.reuseFrontend(0);
				if (mpe = mr.NewScope(mr.declField()))
				{
					mr.installNamespace();
					mr.installTypesMgr();
					declHash(mr);
					mr.Leave();
				}
			}
		};
	public:
		virtual const char* name() const { return "PDB20_GSI"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl impl(r);
			impl.preformat(r, nSize);
		}
	};

	DECLARE_FORMATTER(CFormatter_GSI, PDB20_GSI);




	/////////////////////////////////////////////T_PDB_PSI
	class CFormatter_PSI : public I_FormatterType
	{
		class T_Impl : public T_PDB_base
		{
			HTYPE	mpe;
		public:
			T_Impl(I_SuperModule& rMain) : T_PDB_base(rMain), mpe(nullptr) {}
			void preformat(I_SuperModule&, unsigned long dataSize)
			{
				mr.reuseFrontend(0);
				if (mpe = mr.NewScope(mr.declField()))
				{
					mr.installNamespace();
					mr.installTypesMgr();

					DECLDATAEX(pdb::PSGSIHDR, header);
					mr.declField("aPSGSIHDR", mr.type("PSGSIHDR"));
					if (mr.NewScope(mr.declField("aHash")))
					{
						declHash(mr);
						mr.Leave();
					}

					if (header.cbAddrMap > 0)
						mr.declField("aAddressMap", mr.arrayOf(mr.type(TYPEID_UINT), header.cbAddrMap / sizeof(uint32_t)), ATTR_COLLAPSED);

					if (header.nThunks > 0 && header.cbSizeOfThunk > 0)
					{
						int m(header.cbSizeOfThunk);
						if ((header.nThunks % m) != 0)
							fprintf(stdout, "Attention: Trouble with aThunksMap of PSI stream!\n");
						int n(header.nThunks / m);
						mr.declField("aThunksMap", mr.arrayOf(mr.arrayOf(mr.type(TYPEID_CHAR), m), n), ATTR_COLLAPSED);
					}

					mr.Leave();
				}
			}
		};
	public:
		virtual const char* name() const { return "PDB20_PSI"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl impl(r);
			impl.preformat(r, nSize);
		}
	};

	DECLARE_FORMATTER(CFormatter_PSI, PDB20_PSI);
#endif




	///////////////////////////////////////////////////// T_PDB_MOD
	class CFormatter_MOD : public I_FormatterType
	{
		//pre-processed PDB MOD streams (many)
		class T_Impl : public T_PDB_base
		{
			HTYPE	mpe;
		public:
			T_Impl(I_SuperModule& rMain) : T_PDB_base(rMain), mpe(nullptr) {}
			void preformat()
			{
				mr.reuseFrontend(0);
				if (mpe = mr.NewScope(mr.declField()))
				{
					SAFE_SCOPE_HERE(mr);
					mr.installNamespace();
					mr.installTypesMgr();

					I_DataSourceBase* pDBI(mr.module(STREAM_INDEX_TO_MODULE_ID(pdb::PDB_STREAM_DBI)));
					if (!pDBI)
						return;

					const I_AuxData* pIAux(mr.aux());
					if (pIAux)
					{
						//AuxData_t<1> aux(mr.aux());
						//OFF_t *paux(aux.data<OFF_t>(0));
						OFF_t& oModInfo(*(OFF_t*)pIAux->data());//in DBI stream

						DataFetch2_t<pdb::ModInfo> aModInfo(*pDBI, oModInfo);

						POSITION end(mr.cp() + aModInfo.SymByteSize);

						mr.declField("Signature", mr.enumOf(mr.type("CV_SIGNATURE"), TYPEID_UINT32));
						//mr.declField("Symbols", mr.arrayOf(mr.type(TYPEID_UINT8), aModInfo.SymByteSize - 4), ATTR_COLLAPSED);

						if (mr.NewScope(mr.declField("Symbols")))
						{
							SAFE_SCOPE_HERE(mr);
							POSITION beg(mr.cp());
							while (beg < end)
							{
								DECLDATAPTR(uint16_t, pSymb);
								uint16_t reclen(pSymb[0]);
								if (reclen == 0)
									break;
								beg += sizeof(uint16_t) + reclen;
								mr.declField(nullptr, mr.type(_PFX("CodeView_SYMBOL")), ATTR_NEWLINE);
								mr.setcp(beg);
								beg = mr.align(ALIGN_DWORD);
								//assert(mr.cp() == beg);
							}
						}

						mr.setcp(end);
						if (aModInfo.C11ByteSize > 0)
						{
							end += aModInfo.C11ByteSize;
							mr.declField("C11LineInfo", mr.arrayOf(mr.type(TYPEID_UINT8), aModInfo.C11ByteSize), ATTR_COLLAPSED);
						}


						mr.setcp(end);
						if (aModInfo.C13ByteSize > 0)
						{
							end += aModInfo.C13ByteSize;
#if(0)
							mr.declField("C13LineInfo", mr.arrayOf(mr.type(TYPEID_UINT8), aModInfo.C13ByteSize), ATTR_COLLAPSED);
#else
							if (mr.NewScope(mr.declField("C13LineInfo")))
							{
								SAFE_SCOPE_HERE(mr);
								POSITION beg(mr.cp());
								//POSITION end(beg + aModInfo.C13ByteSize);
								while (beg < end)
								{
									DECLDATAEX(pdb::CV_SubsectionHeader, header);
									beg += sizeof(pdb::CV_SubsectionHeader);
									if (header.sig == 0 || (header.sig & pdb::DEBUG_S_IGNORE))
										continue;
									beg += header.size;
									switch (header.sig)
									{
									case pdb::DEBUG_S_FILECHKSMS:
										mr.declField(nullptr, mr.type(_PFX("CodeView_D_FILECHKSMS")));
										break;
									case pdb::DEBUG_S_LINES:
										mr.declField(nullptr, mr.type(_PFX("CodeView_D_LINES")));
										break;
									default:
										mr.declField(nullptr, mr.type(_PFX("CodeViewSubsection__TBD")));
										break;
									}
									/*switch (pSymb[1])
									{
									case S_OBJNAME:
									mr.declField(nullptr, mr.type(_PFX("CodeView_S_OBJNAME")));
									break;
									}*/
									mr.setcp(beg);
									beg = mr.align(ALIGN_DWORD);
								}
							}
#endif
						}

						mr.setcp(end);
						DECLDATA(uint32_t, aGlobalRefsSize);
						mr.declField("GlobalRefsSize", mr.type(TYPEID_UINT32));
						if (aGlobalRefsSize > 0)
							mr.declField("GlobalRefs", mr.arrayOf(mr.type(TYPEID_UINT8), aGlobalRefsSize), ATTR_COLLAPSED);

						mr.clearAuxData();//no longer needed
					}
				}
			}
		};
	public:
		virtual const char* name() const { return "PDB20_MOD"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl impl(r);
			impl.preformat();
		}
	};

	DECLARE_FORMATTER(CFormatter_MOD, PDB20_MOD);



}//namespace PDB20


/////////////////////////////////////////
void PDB20_RegisterFormatters(I_ModuleEx& rMain)
{
	rMain.RegisterFormatterType(_PFX("MS_PDB20"));//primary
	rMain.RegisterFormatterType(_PFX("PDB20_DIR"));
	rMain.RegisterFormatterType(_PFX("PDB20_PDB"));
	rMain.RegisterFormatterType(_PFX("PDB20_TPI"));
	rMain.RegisterFormatterType(_PFX("PDB20_DBI"));
	rMain.RegisterFormatterType(_PFX("PDB20_MOD"));

	//rMain.RegisterFormatterType(_PFX("PDB20_GSI"));
	//rMain.RegisterFormatterType(_PFX("PDB20_PSI"));
	rMain.RegisterFormatterType(_PFX("PDB20_SYMREC"));
}


