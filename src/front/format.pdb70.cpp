#include "format.pdb.h"
#include "shared.h"
#include "PDB.h"
#include "cv/cvinfo.h"

using namespace pdb;


#define BYTE	unsigned char
#define WORD	unsigned short
#define DWORD	unsigned long
#define IMAGE_SIZEOF_SHORT_NAME              8

struct IMAGE_SECTION_HEADER {
	BYTE    Name[IMAGE_SIZEOF_SHORT_NAME];
	union {
		DWORD   PhysicalAddress;
		DWORD   VirtualSize;
	} Misc;
	DWORD   VirtualAddress;
	DWORD   SizeOfRawData;
	DWORD   PointerToRawData;
	DWORD   PointerToRelocations;
	DWORD   PointerToLinenumbers;
	WORD    NumberOfRelocations;
	WORD    NumberOfLinenumbers;
	DWORD   Characteristics;
};

namespace PDB70 {

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
			void declBlock(ulittle32_t oAt, size_t blockSize, int iStreamId, int iBlockId)
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
				//			if (mpe = mr.NewScope(mr.declField()))
				{
					//			SAFE_SCOPE_HERE(mr);
					PDB70_CreateStructures(mr);
					PDB70_DeclareDynamicTypes(mr);

					DECLDATAEX(SuperBlock, aSuperBlock);
					mr.declField("aSuperBlock", mr.type("SuperBlock"));

					//stream dir's blocks list
					size_t oStreamDirBlocks(aSuperBlock.BlockMapAddr * aSuperBlock.BlockSize);
					mr.setcp((POSITION)oStreamDirBlocks);

					DECLDATAPTR(ulittle32_t, oStreamDirBlockIndex);

					uint32_t directorySize(aSuperBlock.NumDirectoryBytes);
					int uStreamDirIndicesNum((int)ceil((double)directorySize / aSuperBlock.BlockSize));
					mr.declField("aStreamDirIndices", mr.arrayOf(mr.type(TYPEID_ULITTLE32), uStreamDirIndicesNum), (AttrIdEnum)ATTR_PDB_BLK);

					//the stream directory itself may also be scattered, so we need a consistent way to access it's contents
					HPATCHMAP hStreamDir(mr.newPatchMap());
					for (int i(0); directorySize > 0; i++)
					{
						uint32_t blockSize(std::min(directorySize, aSuperBlock.BlockSize));
						directorySize -= blockSize;
						mr.addPatch(hStreamDir, *oStreamDirBlockIndex * aSuperBlock.BlockSize, blockSize);
						declBlock(*oStreamDirBlockIndex, aSuperBlock.BlockSize, -1, i);
						++oStreamDirBlockIndex;
					}

					I_DataSourceBase* pDirDS(mr.stitchDerivativeModule(hStreamDir, "$directory", _PFX("PDB_DIR"), false, 1));
					if (pDirDS)
					{
						OFF_t o(0);
						DataFetch_t<ulittle32_t> uNumStreams(*pDirDS, o);
						o += uNumStreams.size();

						DataFetchPtr_t<ulittle32_t> pStreamSizes(*pDirDS, o);
						o += pStreamSizes.size() * uNumStreams;

						DataFetchPtr_t<ulittle32_t> pStreamBlocks(*pDirDS, o);

						for (size_t i(0); i < uNumStreams; i++, ++pStreamSizes)
						{
							uint32_t uStreamSize(*pStreamSizes);
							if ((int)uStreamSize <= 0)
								continue;
							int n((int)ceil((double)uStreamSize / aSuperBlock.BlockSize));
							assert(n > 0);
							for (size_t j(0); j < (size_t)n; j++, ++pStreamBlocks)
								declBlock(*pStreamBlocks, aSuperBlock.BlockSize, int(i), int(j));
						}
					}
					//I_DataSourceBase *pIStreamDir(mr.createDataLeech(hStreamDir, true));
					//size_t oStreamDir(pIStreamDir->pos(0));
					//pIStreamDir->Release();
				}
			}
		};
	protected:
		virtual const char* name() const { return "MS_PDB70"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_PDB a(r);
			a.preformat();
		}
	};
	DECLARE_FORMATTER(CDynamicType_PDB, MS_PDB70);


	/////////////////////////////////////////T_PDB_DIR
	class CFormatter_DIR : public I_FormatterType
	{
		//pre-processed PDB stream directory
		class T_Impl : public T_PDB_base
		{
			HTYPE	mpe;
			SuperBlock	mSuperBlock;
		public:
			T_Impl(I_SuperModule& rMain) : T_PDB_base(rMain), mpe(nullptr)
			{
				const I_DataSourceBase* pIHost(rMain.host());
				pIHost->dataAt(0, sizeof(SuperBlock), (PDATA)&mSuperBlock);
			}
			void preformat()
			{
				mr.reuseFrontend(0);
				//if (mpe = mr.NewScope(mr.declField()))
				{
					//SAFE_SCOPE_HERE(mr);
					//mr.installNamespace();
					//mr.installTypesMgr();
	//				PDB_CreateStructures(mr);
					if (mr.NewScope(mr.declField("aStreamDirectory")))
					{
						SAFE_SCOPE_HERE(mr);
						mr.installNamespace();
						DECLDATA(ulittle32_t, uNumStreams);
						mr.declField("NumStreams", mr.type(TYPEID_ULITTLE32), ATTR_DECIMAL);
						DECLDATAPTR(ulittle32_t, pStreamSizes);
						mr.declField("StreamSizes", mr.arrayOf(mr.type(TYPEID_ULITTLE32), uNumStreams));
						if (mr.NewScope(mr.declField("aStreamBlocks")))
						{
							int iWidth(PrintWidth(uNumStreams.get()));
							char fmt[32];
							sprintf(fmt, "stream_%%0%dd", iWidth);

							SAFE_SCOPE_HERE(mr);
							for (size_t i(0); i < uNumStreams; i++, ++pStreamSizes)
							{
								uint32_t uStreamSize(*pStreamSizes);
								if ((int)uStreamSize <= 0)
									continue;
								int n((int)ceil((double)uStreamSize / mSuperBlock.BlockSize));
								assert(n > 0);
								DECLDATAPTR(ulittle32_t, pStreamBlocks);
								char buf[32];
								sprintf(buf, fmt, i);
								mr.declField(buf, mr.arrayOf(mr.type(TYPEID_ULITTLE32), n), AttrIdEnum(ATTR_PDB_BLOCK_ID | ATTR_COLLAPSED));
#if(0)
								if (i > 15) break;
#endif
								{
									//if (n > 1)//Fork PDB DBI Stream
									{
										HPATCHMAP hDBI(mr.newPatchMap());
										while (uStreamSize > 0)
										{
											uint32_t blockSize(std::min(uStreamSize, mSuperBlock.BlockSize));
											uStreamSize -= blockSize;
											mr.addPatch(hDBI, *pStreamBlocks * mSuperBlock.BlockSize, blockSize);
											++pStreamBlocks;
										}
										char buf[32];
										sprintf(buf, fmt, (int)i);
										switch (i)
										{
										case 0:
											strcat(buf, "$directory_old");
											mr.stitchDerivativeModule(hDBI, buf, _PFX("PDB_DIR_OLD"), true, STREAM_INDEX_TO_MODULE_ID(i));
											break;
										case 1:
											strcat(buf, "$PDB");
											mr.stitchDerivativeModule(hDBI, buf, _PFX("PDB_PDB"), false, STREAM_INDEX_TO_MODULE_ID(i));
											break;
										case 2:
											strcat(buf, "$TPI");
											mr.stitchDerivativeModule(hDBI, buf, _PFX("PDB_TPI"), true, STREAM_INDEX_TO_MODULE_ID(i));
											break;
										case 3:
											strcat(buf, "$DBI");
											mr.stitchDerivativeModule(hDBI, buf, _PFX("PDB_DBI"), false, STREAM_INDEX_TO_MODULE_ID(i));
											break;
										case 4:
											strcat(buf, "$IPI");
											mr.stitchDerivativeModule(hDBI, buf, _PFX("PDB_TPI"), true, STREAM_INDEX_TO_MODULE_ID(i));
											break;
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
			}
		};
	public:
		virtual const char* name() const { return "PDB_DIR"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl impl(r);
			impl.preformat();
		}
	};
	DECLARE_FORMATTER(CFormatter_DIR, PDB_DIR);



	////////////////////////////////////////////T_PDB_DIR_OLD(#0) - see PDB2.0
	class CFormatter_DIR_OLD : public I_FormatterType
	{
		//pre-processed PDB DIR OLD stream (index 0)
		class T_Impl : public T_PDB_base
		{
			HTYPE	mpe;
			SuperBlock	mSuperBlock;
		public:
			T_Impl(I_SuperModule& rMain) : T_PDB_base(rMain), mpe(nullptr)
			{
				const I_DataSourceBase* pIHost(rMain.host());
				pIHost->dataAt(0, sizeof(SuperBlock), (PDATA)&mSuperBlock);
			}
			void preformat()
			{
				createStructures2();

				mr.reuseFrontend(0);

				if (mr.NewScope(mr.declField("aStreamDirectory")))
				{
					SAFE_SCOPE_HERE(mr);
					mr.installNamespace();
					DECLDATA(uint16_t, uNumStreams);
					mr.declField("NumStreams", mr.type(TYPEID_UINT16), ATTR_DECIMAL);
					mr.skip(sizeof(uint16_t));
					DECLDATAPTREX(PDB20::PDB_STREAM, pStreamSizes);
					mr.declField("StreamSizes", mr.arrayOf(mr.type("PDB_STREAM"), uNumStreams));

					if (mr.NewScope(mr.declField("aStreamBlocks")))
					{
						int iWidth(PrintWidth(uNumStreams.get()));
						char fmt[32];
						sprintf(fmt, "stream_%%0%dd", iWidth);

						SAFE_SCOPE_HERE(mr);
						for (size_t i(0); i < uNumStreams; i++, ++pStreamSizes)
						{
							const PDB20::PDB_STREAM &uStreamDesc(*pStreamSizes);
							unsigned uStreamSize(uStreamDesc.StreamSize);
							if ((int)uStreamSize <= 0)
								continue;
							int n((int)ceil((double)uStreamSize / mSuperBlock.BlockSize));
							assert(n > 0);
							//DECLDATAPTR(uint16_t, pStreamBlocks);
							char buf[32];
							sprintf(buf, fmt, i);
							mr.declField(buf, mr.arrayOf(mr.type(TYPEID_UINT32), n), ATTR_COLLAPSED);
						}
					}
				}
			}
		private:
			void createStructures2()
			{
				if (mr.NewScope("PDB_STREAM"))
				{
					mr.declField("StreamSize", mr.type(TYPEID_UINT));
					mr.declField("StreamPages", mr.type(TYPEID_UINT));
					mr.Leave();
				}
			}
		};
	protected:
		virtual const char* name() const { return "PDB_DIR_OLD"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl impl(r);
			impl.preformat();
		}
	};
	DECLARE_FORMATTER(CFormatter_DIR_OLD, PDB_DIR_OLD);




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
				//if (mpe = mr.NewScope(mr.declField()))
				{
					//SAFE_SCOPE_HERE(mr);
					//mr.installNamespace();
					//mr.installTypesMgr();

	//				PDB_CreateStructures(mr);
					createStructures2();

					DECLDATAEX(NameIndexHeader, aHeader);
					mr.declField("aHeader", mr.type("NameIndexHeader"));
					//mr.skip(aHeader.NamesOffset);
					POSITION nameStart(mr.cp());
					POSITION end(mr.cp() + aHeader.NamesOffset);
					//while (mr.cp() < end)
					//mr.declField(nullptr, toAscii(mr));
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

					/*?if (deletedskip != 0)
					{
						for (uint32_t i = 0; i < count; ++i)
						{
							if (!(deletedBits[i / 32] & (1 << (i % 32))))
								continue;
							mr.declField(nullptr, mr.type("StringVal"));
							if (mr.EnterScope(nameStart + val.id))
							{
								SAFE_SCOPE_HERE(mr);
								DECLDATAPTR(char, pStr);
								mr.declField(nullptr, toAscii(mr));
							}
							val++;
						}
					}*/
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
					mr.declField("GUID", mr.arrayOf(mr.type(TYPEID_UINT8), 16));
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
		virtual const char* name() const { return "PDB_PDB"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl impl(r);
			impl.preformat();
		}
	};
	DECLARE_FORMATTER(CFormatter_PDB, PDB_PDB);







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
				//PDB_CreateStructures(mr);
				//PDB_DeclareDynamicTypes(mr);
				DECLDATAEX(TypeInfoHeader, tih);
				mr.declField("aHeader", mr.type("TypeInfoHeader"));
				POSITION end(tih.headerSize);
				mr.setcp(end);
				if (mr.NewScope(mr.declField("aTypeInfo")))
				{
					SAFE_SCOPE_HERE(mr);

					for (uint32_t i(tih.min); i < tih.max; ++i)
					{
						CHECK(i == 0x100e)
							STOP
							mr.setcp(end);

						//check padding 
						/*DECLDATA(uint8_t, pad);
						if (pad > LF_PAD0)
						{
							uint8_t n(pad - LF_PAD0);
							mr.skip(n);
							//end += mr.align(ALIGN_DWORD);
						}*/

						DECLDATAEX(TypeRecord, tr);
						if (tr.leaf == 0)
						{
							end += sizeof(TypeRecord);
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
		virtual const char* name() const { return "PDB_TPI"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl impl(r);
			impl.preformat();
		}
	};
	DECLARE_FORMATTER(CFormatter_TPI, PDB_TPI);





	/*enum PDB_DBI_AUX_e
	{
		PDB_DBI_AUX_MODINFO,//0: module info's arrays indices offsets
		PDB_DBI__AUX_TOTAL
		//..
	};*/

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
				//if (mpe = mr.NewScope(mr.declField()))
				{
					//mr.installNamespace();
					//mr.installTypesMgr();

	//				PDB_CreateStructures(mr);
	//				PDB_DeclareDynamicTypes(mr);

					DECLDATAEX(DbiStreamHeader, aDbiSH);
					mr.declField("aDbiStreamHeader", mr.type("DbiStreamHeader"));

					//char buf1[32];

					//sprintf(buf1, "stream_#%d", (int)aDbiSH.GlobalStreamIndex);
					mr.preformatModule(STREAM_INDEX_TO_MODULE_ID(aDbiSH.GlobalStreamIndex), _PFX("PDB_GSI"), true);

					//sprintf(buf1, "stream_#%d", (int)aDbiSH.PublicStreamIndex);
					mr.preformatModule(STREAM_INDEX_TO_MODULE_ID(aDbiSH.PublicStreamIndex), _PFX("PDB_PSI"), true);

					//sprintf(buf1, "stream_#%d", (int)aDbiSH.SymRecordStream);
					mr.preformatModule(STREAM_INDEX_TO_MODULE_ID(aDbiSH.SymRecordStream), _PFX("PDB_SYMREC"), true);

					if (aDbiSH.ModInfoSize > 0)
					{
						if (mr.NewScope(mr.declField("aModInfoSubstream", ATTR_COLLAPSED)))
						{
							POSITION end((POSITION)mr.cpr() + aDbiSH.ModInfoSize);
							//for (int k(0); k < 5; k++)
							while (mr.cpr() < end)
							{
								OFF_t z = mr.cpr();
								mr.align(ALIGN_DWORD);
								OFF_t oModInfo(mr.cpr());
								DECLDATAEX(ModInfo, aModInfo);
								mr.declField(nullptr/*"aModInfo#"*/, mr.type(_PFX("ModInfo")), ATTR_COLLAPSED);
								//sprintf(buf1, "stream_#%d", (int)aModInfo.ModuleSymStream);
								I_DataSourceBase* pIS(mr.module(STREAM_INDEX_TO_MODULE_ID(aModInfo.ModuleSymStream)));
								if (pIS)
								{
									/*AuxData_t<1> aux;
									aux.reserve(1 * sizeof(OFF_t), 0);
									OFF_t *paux(aux.data<OFF_t>(0));
									*paux = oModInfo;*/
									pIS->setAuxData((PDATA)&oModInfo, sizeof(oModInfo));
									mr.preformatModule(STREAM_INDEX_TO_MODULE_ID(aModInfo.ModuleSymStream), _PFX("PDB_MOD"), true);
								}
							}
							//mr.declField(nullptr, mr.type(TYPEID_BYTE));
							mr.Leave();
						}
					}
					if (aDbiSH.SectionContributionSize > 0)
					{
						if (mr.NewScope(mr.declField("aSectionContributionSubstream", ATTR_COLLAPSED)))
						{
							DECLDATA(uint32_t, aSectionContrSubstreamVersion);
							mr.declField("aSectionContrSubstreamVersion", mr.enumOf(mr.type("SectionContrSubstreamVersion"), TYPEID_UINT32));
							int nSectionContrSubstreamNum(aDbiSH.SectionContributionSize / sizeof(SectionContribEntry));
							const char* pcTypeName("SectionContribEntry");
							if (aSectionContrSubstreamVersion == SectionContrSubstreamVersion::V2)
							{
								nSectionContrSubstreamNum = aDbiSH.SectionContributionSize / sizeof(SectionContribEntry2);
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
							DECLDATAEX(SectionMapHeader, aSMH);
							mr.declField("aSectionMapHeader", mr.type("SectionMapHeader"));
							int n(aDbiSH.SectionMapSize / sizeof(SectionMapEntry));
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
								/*DECLDATAPTR(uint16_t, pModFileCount);
								uint32_t uSourceFilesTotal(0);//total number of source file contributions to modules (may be equal to aNumSourceFiles?)
								for (uint16_t i(0); i < aNumModules; i++, ++pModFileCount)
								uSourceFilesTotal += *pModFileCount;*/
								mr.declField("ModFileCounts", mr.arrayOf(mr.type(TYPEID_UINT16), aNumModules), ATTR_COLLAPSED);
							}
							if (aNumSourceFiles > 0)
							{
								mr.declField("FileNameOffsets", mr.arrayOf(mr.type(TYPEID_UINT32), aNumSourceFiles), ATTR_COLLAPSED);
								POSITION end(beg + aDbiSH.SourceInfoSize);
								while (mr.cp() < end)
									mr.declField(nullptr, toAscii(mr));
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
						DECLDATAEX(DBIDebugHeader, debugHeader);
						if (aDbiSH.OptionalDbgHeaderSize == sizeof(DBIDebugHeader))
							mr.declField("aOptionalDbgHeader", mr.type("DBIDebugHeader"));
						else
							mr.declField("aOptionalDbgHeader", mr.arrayOf(mr.type(TYPEID_UINT16), aDbiSH.OptionalDbgHeaderSize / sizeof(uint16_t)));

						if (debugHeader.sectionHdr != 0xFFFF)
						{
							//char buf1[32];
							//sprintf(buf1, "stream_#%d", (int)debugHeader.sectionHdr);
							//I_DataSourceBase *pIS(mr.module(buf1));
							//if (pIS)
							mr.preformatModule(STREAM_INDEX_TO_MODULE_ID(debugHeader.sectionHdr), _PFX("PDB_ISH"), true);
						}
					}

					//mr.Leave();
				}
			}
		};
	protected:
		virtual const char* name() const { return "PDB_DBI"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl impl(r);
			impl.preformat();
		}
	};
	DECLARE_FORMATTER(CFormatter_DBI, PDB_DBI);






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
				//if (mpe = mr.NewScope(mr.declField()))
				{
					//SAFE_SCOPE_HERE(mr);
					//mr.installNamespace();
					//mr.installTypesMgr();
	//				PDB_CreateStructures(mr);
	//				PDB_CreateStructures_MOD(mr);
	//				PDB_DeclareDynamicTypes(mr);
					POSITION end(mr.cp());
					while (end < dataSize)
					{
						DECLDATAPTR(uint16_t, pRec);
						end += (POSITION)pRec.size() + pRec[0];
						mr.declField(nullptr, mr.type(_PFX("CodeView_SYMBOL")), ATTR_NEWLINE);
						//end = mr.align(ALIGN_DWORD);
						mr.setcp(end);
					}
				}
			}
		};
	public:
		virtual const char* name() const { return "PDB_SYMREC"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl a(r);
			a.preformat(r, nSize);
		}
	};
	DECLARE_FORMATTER(CFormatter_SYMREC, PDB_SYMREC);






	////////////////////////////////////////////T_PDB_NAMES(#?)
	class CFormatter_NAMES : public I_FormatterType
	{
		//pre-processed PDB NAMES
		class T_Impl : public T_PDB_base
		{
			HTYPE	mpe;
		public:
			T_Impl(I_SuperModule& rMain) : T_PDB_base(rMain), mpe(nullptr) {}
			void preformat(I_SuperModule&, unsigned long)
			{
				//if (mpe = mr.NewScope(mr.declField()))
				{
					//SAFE_SCOPE_HERE(mr);
					//mr.installNamespace();
					//mr.installTypesMgr();
					//createStructures();
					mr.reuseFrontend(0);
					createStructures2();
					DECLDATAEX(NameStreamHeader, aHeader);
					mr.declField("aHeader", mr.type("NameStreamHeader"));
					POSITION beg(mr.cp());
					mr.skip(aHeader.offset);
					DECLDATAPTR(uint32_t, p);
					uint32_t size(p++);
					mr.declField("StringPoolSize", mr.type(TYPEID_UINT32));
					if (size > 0)
					{
						mr.declField("StringPoolIds", mr.arrayOf(mr.type(TYPEID_UINT32), size), (AttrIdEnum)ATTR_OFFS_NAMES_HDR);//ATTR_HEX);
						for (uint32_t i(0); i < size; ++i, ++p)
						{
							//if (i == 0xd6)break;
							//CHECK(i == 0xd4)
							//STOP
							uint32_t id(*p);
							if (id != 0)
								if (mr.EnterScope(beg + id))
								{
									SAFE_SCOPE_HERE(mr);
									DataStream_t pc(mr, mr.cpr());
									std::ostringstream s;
									pc.fetchString(s);
									mr.declField(nullptr, toAscii(mr));
								}
						}
					}
				}
			}
		private:
			void createStructures2()
			{
				if (mr.NewScope("NameStreamHeader"))
				{
					mr.declField("Signature", mr.enumOf(mr.type("CV_SIGNATURE"), TYPEID_UINT32));
					mr.declField("Version", mr.type(TYPEID_INT32));
					mr.declField("Offset", mr.type(TYPEID_INT32), (AttrIdEnum)ATTR_OFFS_NAMES_HDR);
					mr.Leave();
				};
			}
		};
	public:
		virtual const char* name() const { return "PDB_NAMES"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl a(r);
			a.preformat(r, nSize);
		}
	};
	DECLARE_FORMATTER(CFormatter_NAMES, PDB_NAMES);





	/////////////////////////////////////////////T_PDB_ISH
	class CFormatter_ISH : public I_FormatterType
	{
		//pre-processed PDB IMAGE_SECTION_HEADERS
		class T_Impl : public T_PDB_base
		{
			HTYPE	mpe;
		public:
			T_Impl(I_SuperModule& rMain) : T_PDB_base(rMain), mpe(nullptr) {}
			void preformat(I_SuperModule&, unsigned long dataSize)
			{
				if (mpe = mr.NewScope(mr.declField()))
				{
					mr.installNamespace();
					mr.installTypesMgr();
					createStructures();
					int n(dataSize / sizeof(IMAGE_SECTION_HEADER));
					mr.declField(nullptr, mr.arrayOf(mr.type("IMAGE_SECTION_HEADER"), n));

					mr.Leave();
				}
			}
		private:
			HTYPE type(OpType_t i) { return mr.type(i); }
			HTYPE arrayOf(HTYPE t, unsigned n) { return mr.arrayOf(t, n); }
			HTYPE type(HNAME n) { return mr.type(n); }
			void createStructures()
			{
				if (mr.NewScope("IMAGE_SECTION_HEADER"))
				{
					mr.installTypesMgr();
					if (mr.NewScope("IMAGE_SCN", SCOPE_STRUC))
					{
						mr.skipBits(5);//<0-4>
						mr.declBField("CNT_CODE", type(TYPEID_DWORD));//<5>
						mr.declBField("CNT_INITIALIZED_DATA", type(TYPEID_DWORD));//<6>
						mr.declBField("CNT_UNINITIALIZED_DATA", type(TYPEID_DWORD));//<7>
						mr.skipBits(1);//<8>
						mr.declBField("LNK_INFO", type(TYPEID_DWORD));//<9>
						mr.skipBits(1);//<10>
						mr.declBField("LNK_REMOVE", type(TYPEID_DWORD));//<11>
						mr.declBField("LNK_COMDAT", type(TYPEID_DWORD));//<12>
						mr.skipBits(2);//<13,14>
						mr.declBField("MEM_FARDATA", type(TYPEID_DWORD));//<15>
						mr.skipBits(1);//<16>
						mr.declBField("MEM_PURGEABLE", type(TYPEID_DWORD));//<17>
						mr.declBField("MEM_LOCKED", type(TYPEID_DWORD));//<18>
						mr.declBField("MEM_PRELOAD", type(TYPEID_DWORD));//<19>
						mr.declBField("ALIGN", arrayOf(type(TYPEID_DWORD), 4));//<20-23>
						mr.declBField("LNK_NRELOC_OVFL", type(TYPEID_DWORD));//<24>
						mr.declBField("MEM_DISCARDABLE", type(TYPEID_DWORD));//<25>
						mr.declBField("MEM_NOT_CACHED", type(TYPEID_DWORD));//<26>
						mr.declBField("MEM_NOT_PAGED", type(TYPEID_DWORD));//<27>
						mr.declBField("MEM_SHARED", type(TYPEID_DWORD));//<28>
						mr.declBField("MEM_EXECUTE", type(TYPEID_DWORD));//<29>
						mr.declBField("MEM_READ", type(TYPEID_DWORD));//<30>
						mr.declBField("MEM_WRITE", type(TYPEID_DWORD));//<31>
						mr.Leave();
					}
					mr.declField("Name", arrayOf(type(TYPEID_CHAR), 8));//IMAGE_SIZEOF_SHORT_NAME
					if (mr.NewScope(mr.declField("Misc")))//nullptr, SCOPE_ UNION)))
					{
						mr.installNamespace();
						mr.declUField("PhysicalAddress", type(TYPEID_DWORD));
						mr.declUField("VirtualSize", type(TYPEID_DWORD));
						mr.Leave();
					}
					//mr.declField("Misc", pUnion);
					mr.declField("VirtualAddress", type(TYPEID_DWORD), ATTR_RVA);
					mr.declField("SizeOfRawData", type(TYPEID_DWORD));
					mr.declField("PointerToRawData", type(TYPEID_DWORD), (AttrIdEnum)ATTR_PDB_FILEPTR);
					mr.declField("PointerToRelocations", type(TYPEID_DWORD));
					mr.declField("PointerToLinenumbers", type(TYPEID_DWORD));
					mr.declField("NumberOfRelocations", type(TYPEID_WORD));
					mr.declField("NumberOfLinenumbers", type(TYPEID_WORD));
					mr.declField("Characteristics", type("IMAGE_SCN"), ATTR_COLLAPSED);//TYPEID_DWORD
					mr.Leave();
				}
			}
		};
	public:
		virtual const char* name() const { return "PDB_ISH"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl impl(r);
			impl.preformat(r, nSize);
		}
	};

	DECLARE_FORMATTER(CFormatter_ISH, PDB_ISH);





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
					//				PDB_CreateStructures(mr);
					//				PDB_CreateStructures_MOD(mr);
					//				PDB_DeclareDynamicTypes(mr);

					I_DataSourceBase* pDBI(mr.module(STREAM_INDEX_TO_MODULE_ID(pdb::PDB_STREAM_DBI)));
					if (!pDBI)
						return;

					const I_AuxData* pIAux(mr.aux());
					if (pIAux)
					{
						//AuxData_t<1> aux(mr.aux());
						//OFF_t *paux(aux.data<OFF_t>(0));
						OFF_t& oModInfo(*(OFF_t*)pIAux->data());//in DBI stream

						DataFetch2_t<ModInfo> aModInfo(*pDBI, oModInfo);

						POSITION end(mr.cp() + aModInfo.SymByteSize);

						mr.declField("Signature", mr.type(TYPEID_UINT32));
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
									DECLDATAEX(CV_SubsectionHeader, header);
									beg += sizeof(CV_SubsectionHeader);
									if (header.sig == 0 || (header.sig & DEBUG_S_IGNORE))
										continue;
									beg += header.size;
									switch (header.sig)
									{
									case DEBUG_S_FILECHKSMS:
										mr.declField(nullptr, mr.type(_PFX("CodeView_D_FILECHKSMS")));
										break;
									case DEBUG_S_LINES:
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
		virtual const char* name() const { return "PDB_MOD"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl impl(r);
			impl.preformat();
		}
	};

	DECLARE_FORMATTER(CFormatter_MOD, PDB_MOD);


	static void declHash(I_Module& mr)
	{
		DECLDATAEX(GSIHashHdr, header);

		mr.declField("aHeader", mr.type("GSIHashHdr"));

		unsigned n(header.cbHr / sizeof(HRFile));
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
		virtual const char* name() const { return "PDB_GSI"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl impl(r);
			impl.preformat(r, nSize);
		}
	};

	DECLARE_FORMATTER(CFormatter_GSI, PDB_GSI);





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

					DECLDATAEX(PSGSIHDR, header);
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
		virtual const char* name() const { return "PDB_PSI"; }
		virtual void createz(I_SuperModule& r, unsigned long nSize)
		{
			T_Impl impl(r);
			impl.preformat(r, nSize);
		}
	};

	DECLARE_FORMATTER(CFormatter_PSI, PDB_PSI);



}//namespace PDB70



///////////////////////////////// FE_PDB_t
class FE_PDB_t : public I_Front
{
	pdb::MyPdbReader mPdb;
public:
	FE_PDB_t(const I_DataSourceBase *p)
		: mPdb(*p, CheckPdbSignature(*p, -1))
	{
	}
protected:
	virtual void release()
	{
		delete this;
	}
	virtual AKindEnum translateAddress(const I_DataStreamBase &mr, int moduleId, ADDR &addr, AttrIdEnum attr)
	{
		ADDR atAddr(mr.cp());
		switch (attr)
		{
		case ATTR_OFFS:
			return AKIND_VA;
		case ATTR_PDB_TYPEOFF:
			//return AKIND_NULL;
			if (moduleId != STREAM_INDEX_TO_MODULE_ID(pdb::PDB_STREAM_TPI))
				return OTHER_MODULE(pdb::PDB_STREAM_TPI);
			addr = (ADDR)mPdb.TypeOffsetFromIndex(addr);
			return AKIND_RAW;
		case ATTR_PDB_BLK:
			addr = addr * mPdb.BlockSize();
			return AKIND_RAW;
		case ATTR_PDB20_BLK:
			addr = addr * mPdb.BlockSize();
			return AKIND_RAW;
		case ATTR_OFFS_NAMES_HDR:
			addr += sizeof(pdb::NameStreamHeader);
			return AKIND_RAW;
		case ATTR_PDB_NAME:
			if (MODULE_ID_TO_STREAM_INDEX(moduleId) != mPdb.NamesStreamIndex())
				return OTHER_MODULE(mPdb.NamesStreamIndex());
			addr = (ADDR)mPdb.NameOffsetFromIndex(addr);
			return AKIND_RAW;
		case ATTR_STREAM_INDEX:
			if (MODULE_ID_TO_STREAM_INDEX(moduleId) != addr)
				return OTHER_MODULE(addr);
			addr = 0;
			return AKIND_RAW;
		case ATTR_MODULE_INDEX:
			if (addr > 0)
			{
				int iStream(mPdb.ModuleIndexToStream((int)addr));
				if (MODULE_ID_TO_STREAM_INDEX(moduleId) != iStream)
					return OTHER_MODULE(iStream);
			}
			addr = 0;
			return AKIND_RAW;
		case ATTR_SYM_OFF_P1:
			if (addr == 0)
				return AKIND_NULL;
			if (MODULE_ID_TO_STREAM_INDEX(moduleId) != mPdb.DebugInfoHeader().SymRecordStream)
				return OTHER_MODULE(mPdb.DebugInfoHeader().SymRecordStream);
			addr -= 1;//1-based
			return AKIND_RAW;
		case ATTR_GSI_OFF_MH:
			addr += sizeof(GSIHashHdr);
			return AKIND_RAW;
		case ATTR_MOD_FILECHKSMS:
		{
			OFF_t oStart(mPdb.FileChecksumOffset(MODULE_ID_TO_STREAM_INDEX(moduleId)));
			if (oStart == 0)
				return AKIND_NULL;
			addr += (ADDR)oStart;
			return AKIND_RAW;
		}
		default:
			break;
		}
		return AKIND_NULL;
	}
};

I_Front *CreateFE_PDB(const I_DataSourceBase *aRaw)
{
	return new FE_PDB_t(aRaw);
}


/////////////////////////////////////////
void PDB_RegisterFormatters(I_ModuleEx &rMain)
{
	rMain.RegisterFormatterType(_PFX("MS_PDB70"));//primary
	rMain.RegisterFormatterType(_PFX("PDB_DIR"));
	rMain.RegisterFormatterType(_PFX("PDB_DIR_OLD"));
	rMain.RegisterFormatterType(_PFX("PDB_PDB"));
	rMain.RegisterFormatterType(_PFX("PDB_TPI"));
	rMain.RegisterFormatterType(_PFX("PDB_DBI"));
	rMain.RegisterFormatterType(_PFX("PDB_ISH"));
	rMain.RegisterFormatterType(_PFX("PDB_SYMREC"));
	rMain.RegisterFormatterType(_PFX("PDB_NAMES"));
	rMain.RegisterFormatterType(_PFX("PDB_MOD"));
	rMain.RegisterFormatterType(_PFX("PDB_GSI"));
	rMain.RegisterFormatterType(_PFX("PDB_PSI"));
}


