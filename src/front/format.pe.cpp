#include "format.pe.h"

#include <assert.h>
#include <iostream>
#include <fstream>

#include "qx/MyPath.h"
#include "qx/MyStream.h"
#include "qx/MyString.h"
#include "qx/MyStringList.h"
#include "qx/QxTime.h"

#include "shared.h"
#include "shared/front.h"
#include "shared/data_source.h"
#include "front_IA.h"
#include "decode.PDB.h"
#include "decode.COFF.h"
#include "decode.pe.h"


#define USE_CAPSTONE 1

#if(USE_CAPSTONE)
#define TYPECODE_X86_16	_PFX("CAPSTONE_X86_16")
#define TYPECODE_X86_32	_PFX("CAPSTONE_X86_32")
#define TYPECODE_X86_64	_PFX("CAPSTONE_X86_64")
#else
#define TYPECODE_X86_16	_PFX("UDIS86_16")
#define TYPECODE_X86_32	_PFX("UDIS86_32")
#define TYPECODE_X86_64	_PFX("UDIS86_64")
#endif


void DWARF_CreateStructures(I_SuperModule &);

std::vector<double> _timers(5, 0.0);

using namespace adcwin;

#define EXIT_PORTAL	EXIT_SCOPE

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
template <class T_PE>
class PE_Formatter_t : public T_PE,
	public PE_Strucs_t
{
	HTYPE	mpe;
	HTYPE	mhRangeSet;
public:
	PE_Formatter_t(I_SuperModule &rMain)
		: T_PE(rMain),
		PE_Strucs_t(rMain),
		mpe(nullptr)
	{
	}

	HTYPE top() const { return mpe; }
	using typename T_PE::MyDebugInfo;
	using typename T_PE::ExportEntryIterator;
	using typename T_PE::ResourceIterator;
	using typename T_PE::ImportEntryIterator;
	using typename T_PE::IMAGE_OPTIONAL_HEADER;

	using T_PE::ImageDataDirectoryPtr;
	using T_PE::RVA2VA;
	using T_PE::FP2RVA;
	using T_PE::RvaToFO;
	using T_PE::dataSource;
	using T_PE::RvaToSection;
	using T_PE::ImageDOSHeader;
	using T_PE::ImageFileHeader;
	using T_PE::FindSection;
	using T_PE::IBASE;

protected:

	/////////////////////////////////////////////////////COM
	void processLoadCOMTable()
	{
		const IMAGE_DATA_DIRECTORY* pidd(ImageDataDirectoryPtr(IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR));
		if (pidd && pidd->Size > 0)
		{
			createStructures_dotNET();
			declareDynamicTypes_dotNET();

			if (EnterSegment(pidd->VirtualAddress))
			{
				SAFE_SCOPE_HERE(mr);
				DECLDATAEX(IMAGE_COR20_HEADER, aHeader);
				mr.declField(nullptr, type("IMAGE_COR20_HEADER"));
				if (EnterSegment(aHeader.MetaData.VirtualAddress))
				{
					SAFE_SCOPE_HERE(mr);
					mr.declField(nullptr, type(_PFX("COR_MetaDataRoot")));
					DECLDATA2(WORD, uStreams, -(int)sizeof(WORD));
					if (mr.NewScope(mr.declField("StreamHeaders")))
					{
						SAFE_SCOPE_HERE(mr);
						for (unsigned u(0); u < uStreams; u++)
						{
							mr.align(ALIGN_DWORD);
							DECLDATAEX(ADCPE::COR_StreamHeader, aStreamHdr);
							DataStream_t aStreamHdrName(mr, mr.cpr() + offsetof(ADCPE::COR_StreamHeader, Name));
							if (aStreamHdrName.strCmp("#Strings") == 0)
							{
								if (EnterSegment(aHeader.MetaData.VirtualAddress + aStreamHdr.Offset))
								{
									SAFE_SCOPE_HERE(mr);
									POSITION beg(mr.cp());
									do {
										mr.declField(nullptr, toAscii(mr));
									} while ((mr.cp() - beg) < aStreamHdr.Size);
								}
							}
							else if (aStreamHdrName.strCmp("#~") == 0)
							{
								if (EnterSegment(aHeader.MetaData.VirtualAddress + aStreamHdr.Offset))
								{
									SAFE_SCOPE_HERE(mr);
									DECLDATAEX(COR_TablesRoot, aTableRoot);
									DECLDATAPTR2(DWORD, aTableRootRows, offsetof(COR_TablesRoot, Rows));
									mr.declField(nullptr, mr.type(_PFX("COR_TablesRoot")));
									int j(0);
									for (unsigned i(0); i < sizeof(QWORD) * 8; i++)
									{
										//if (i > 3)
										//break;
										if (aTableRoot.Valid & (QWORD(1) << i))
											CORTable_declField(mr, i, aTableRootRows[j++]);
									}
								}
							}
							else if (aStreamHdrName.strCmp("#Blob") == 0)
							{
								if (EnterSegment(aHeader.MetaData.VirtualAddress + aStreamHdr.Offset))
								{
									SAFE_SCOPE_HERE(mr);
									POSITION beg(mr.cp());
									do {
										mr.declField(nullptr, mr.type(_PFX("COR_Blob")));
									} while ((mr.cp() - beg) < aStreamHdr.Size);
								}
							}
							else if (aStreamHdrName.strCmp("#US") == 0)
							{
								if (EnterSegment(aHeader.MetaData.VirtualAddress + aStreamHdr.Offset))
								{
									SAFE_SCOPE_HERE(mr);
									POSITION beg(mr.cp());
									do {
										mr.declField(nullptr, mr.type(_PFX("COR_UserString")));
									} while ((mr.cp() - beg) < aStreamHdr.Size);
								}
							}
							mr.declField(nullptr, mr.type(_PFX("COR_StreamHeader")));
						}
					}
				}
			}
		}
	}

	void declareDynamicTypes()
	{
		DeclareUDis86Types(mr);
		DeclareCapstoneTypes(mr);
	}

public:
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	HFIELD createAccel()
	{
		//get the number of elements in a list
		int n(0);
		for (DECLDATAPTREX(ACCELTABLEENTRY, p); !(p->fFlags & 0x0080); p++)
			n++;
		return mr.declField(nullptr, arrayOf(type("ACCELTABLEENTRY"), n + 1));
	}

	HFIELD createResourceItem(unsigned resType)
	{
		HFIELD pf;
		switch (resType)
		{
		case RT_DIALOG:
			return mr.instField(nullptr, mr.type(_PFX("RES_DIALOG")));
		case RT_MENU:
			return mr.instField(nullptr, mr.type(_PFX("RES_MENU")));
		case RT_STRING:
			return mr.declField(nullptr, mr.type(_PFX("RES_STRINGS")));
		case RT_ACCELERATOR:
			return createAccel();
		case RT_VERSION:
			pf = mr.declField("VsVersionInfo", mr.type(_PFX("VS_VERSIONINFO")));
			//mr.declField("test", type(TYPEID_DWORD));
			return pf;
		case RT_ICON:
			return mr.instField(nullptr, mr.type(_PFX("RES_ICON")));
		case RT_BITMAP:
			return mr.instField(nullptr, mr.type(_PFX("RES_BITMAP")));
			//return mr.instField(nullptr, mr.type("BITMAPINFOHEADER"));
		case RT_GROUP_ICON:
			STOP
			break;
		default:
			break;
		}
		return nullptr;
	}

	bool EnterSegment(DWORD rva, bool bTryScope = false)
	{
		assert(mhRangeSet);
		DWORD va(RVA2VA(rva));
		if (!mr.EnterSegment(mhRangeSet, va))
		{
			//if (bTryScope)
			//if (mr.EnterScope(rva))
			//return true;
			fprintf(stderr, "Error: Could not enter segment at %X\n", va);
			return false;
			//throw (-1);
			//assert(0);//?
		}
		return true;
	}

	/////////////////////////////////////////////////////Export Table
	void processExportTable(AuxData_t<PE__META_TOTAL> &meta)
	{
		int iStatus(this->CheckDataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT));
		if (iStatus < 0)
		{
			const IMAGE_DATA_DIRECTORY &ied(this->ImageDataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT));
			fprintf(stderr, "Warning: Invalid Export directory\n");
			if (EnterSegment(ied.VirtualAddress))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField("ImageExportDirectory", arrayOf(type(TYPEID_BYTE), ied.Size));
			}
		}
		else if (iStatus > 0)
		{
			const IMAGE_DATA_DIRECTORY &ied(this->ImageDataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT));
			if (EnterSegment(ied.VirtualAddress))
			{
				SAFE_SCOPE_HERE(mr);
				DECLDATAEX(IMAGE_EXPORT_DIRECTORY, ied);
				if (ied.Base == 0)
					fprintf(stderr, "Warning: Export ordinal base is zero\n");

				PWORD pmap(nullptr);
				DataFetchPtr_t<WORD> pNameOrdinals(dataSource(), RvaToFO(ied.AddressOfNameOrdinals));
				if (pNameOrdinals)
				{
					//check if there is a need for an export meta data
					bool bMeta(ied.NumberOfFunctions != ied.NumberOfNames);//unconditionally needed if diff
					if (!bMeta)
					{
						for (unsigned i(0); i < ied.NumberOfNames; i++)
						{
							unsigned n(pNameOrdinals[i]);
							if (n != i)
							{
								bMeta = true;
								break;
							}
						}
					}

					if (bMeta)
					{
						//build an export meta data
						meta.reserve("NAM2ORD", ied.NumberOfFunctions * sizeof(WORD), PE_META_EXPORT, true);
						pmap = meta.data<WORD>(PE_META_EXPORT);
						for (unsigned i(0); i < ied.NumberOfNames; i++)
						{
							unsigned n(pNameOrdinals[i]);
							//if (n == 0)
							//continue;
							//n -= ied.Base;
							WORD u(0);
							if (!(n < ied.NumberOfFunctions))
								fprintf(stderr, "Warning: Exported name index %d is out of ordinal's range %d\n", i, n + ied.Base);
							else
								u = i + ied.Base;
							pmap[n] = u;//map the index
						}
					}
				}

				mr.declField("ImageExportDirectory", type("IMAGE_EXPORT_DIRECTORY"));
				if (EnterSegment(ied.Name))
				{
					SAFE_SCOPE_HERE(mr);
					mr.declField(nullptr, toAscii(mr));
				}
				if (ied.NumberOfFunctions > 0)
				{
					if (!(ied.NumberOfNames <= ied.NumberOfFunctions))
						fprintf(stderr, "Warning: Number of exported entries are less than number of names\n");//faulty format?

					if (EnterSegment(ied.AddressOfFunctions))
					{
						SAFE_SCOPE_HERE(mr);
						mr.declField("ExportedFunctions", arrayOf(type(TYPEID_DWORD), ied.NumberOfFunctions), ATTR_RVA);
						for (ExportEntryIterator i(*this); i; i++)//this is the first time use
						{
							std::ostringstream sName;
							DWORD rvaName(i.nameRVA(pmap));
							if (rvaName != 0)//i.isNamed()
							{
								if (EnterSegment(rvaName))//i.nameRVA()
								{
									SAFE_SCOPE_HERE(mr);
									DataStream_t hName(mr, mr.cpr());
									hName.fetchString(sName);
									mr.declField(nullptr, toAscii(mr));
									//static int z = 0;
									//printf("%d: %s\n", z++, sName.c_str());
								}
							}
							DWORD rva(i.funcRVA());
							if (rva != 0 && EnterSegment(rva))//some entries can be 0
							{
								SAFE_SCOPE_HERE(mr);

								const IMAGE_SECTION_HEADER *pish(RvaToSection(rva));
								bool bFunc(pish && (pish->Characteristics & IMAGE_SCN_CNT_CODE) && !m64bit);

								sName << '\n' << i.ordinal();
								sName << '\n';//will expand to a this module

								if (bFunc)
									mr.declCField(sName.str().c_str(), I_Module::CODE_TYPE_PROC, ATTR_EXPORTED);
								else
									mr.declField(sName.str().c_str(), nullptr, ATTR_EXPORTED);
							}
						}
					}
				}
				if (ied.NumberOfNames > 0)
				{
					if (EnterSegment(ied.AddressOfNames))
					{
						SAFE_SCOPE_HERE(mr);
						mr.declField("ExportedNames", arrayOf(type(TYPEID_DWORD), ied.NumberOfNames), ATTR_RVA);
					}
					if (EnterSegment(ied.AddressOfNameOrdinals))
					{
						SAFE_SCOPE_HERE(mr);
						mr.declField("ExportedOrdinals", arrayOf(type(TYPEID_WORD), ied.NumberOfNames));
					}
				}
			}
		}
	}

	/////////////////////////////////////////////////////Import Table
	void processImportTable()
	{
		const IMAGE_DATA_DIRECTORY &idd1(this->ImageDataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT));
		if (idd1.VirtualAddress != 0)//idd1.Size > 0)//size can be 0
		{
			if (EnterSegment(idd1.VirtualAddress))
			{
				SAFE_SCOPE_HERE(mr);
				for (ImportEntryIterator i(*this); i; i++)
				{
					if (i.thunkIndex() == 0)
					{
						DECLDATAEX(IMAGE_IMPORT_DESCRIPTOR, iid);
						mr.declField(nullptr, type("IMAGE_IMPORT_DESCRIPTOR"));

						if (EnterSegment(iid.Name))
						{
							mr.declField(nullptr, toAscii(mr));
							mr.Leave();
						}

						if (iid.OriginalFirstThunk && EnterSegment(iid.OriginalFirstThunk))
						{
							mr.declField(nullptr, arrayOf(pointerType(), i.thunksCount() + 1), ATTR_RVA_OR_ORDINAL);
							mr.Leave();
						}
					}

					if (EnterSegment(i.functionRVA()))
					{
						SAFE_SCOPE_HERE(mr);

						AttrIdEnum attr(ATTR_NULL);//ATTR_IMPORTED);
						if (i.firstThunkIsRVA())
							attr = AttrIdEnum(attr | ATTR_RVA);//IAT may be array of RVAs to names/ordinals

						std::ostringstream ss;
						if (i.functionName(ss) != 0)//!byOrdinal
						{
							if (EnterSegment(i.functionNameDescRVA()))//imp_by_name
							{
								SAFE_SCOPE_HERE(mr);
								mr.declField(nullptr, type(_PFX("IMAGE_IMPORT_BY_NAME")));
							}
						}
						ss << '\n' << i.ordinal();
						ss << '\n';
						i.moduleName(ss);
						mr.declField(ss.str().c_str(), impPointerType(), attr);
					}
					//break;
				}
				mr.declField(nullptr, type("IMAGE_IMPORT_DESCRIPTOR"));//the nullptr last one
			}
		}
	}

	///////////////////////////////////////////////////////Exceptions
	void processExceptionsTable()
	{
		const IMAGE_DATA_DIRECTORY* pidd(this->ImageDataDirectoryPtr(IMAGE_DIRECTORY_ENTRY_EXCEPTION));
		if (pidd && pidd->Size > 0)
		{
			if (m64bit)
			{
				if (EnterSegment(pidd->VirtualAddress))
				{
					DECLDATAPTREX(RUNTIME_FUNCTION, ptr);
					assert((pidd->Size % sizeof(RUNTIME_FUNCTION)) == 0);
					int iNum(pidd->Size / sizeof(RUNTIME_FUNCTION));
					for (int i(0); i < iNum; i++)
					{
						if (EnterSegment(ptr->UnwindData))
						{
							mr.declField(nullptr, type(_PFX("UNWIND_INFO")));
							mr.Leave();
						}
						ptr++;
					}
					mr.declField("ExceptionTable", arrayOf(type("RUNTIME_FUNCTION"), iNum));
					mr.Leave();
				}
			}
		}
	}

	///////////////////////////////////////////////////////// Security
	void processSecurityTable()
	{
		//Points to a list of WIN_CERTIFICATE structures, defined in WinTrust.H. 
		//Not mapped into memory as part of the image. Therefore, the VirtualAddress field is a file offset, rather than an RVA

		const IMAGE_DATA_DIRECTORY* pidd(ImageDataDirectoryPtr(IMAGE_DIRECTORY_ENTRY_SECURITY));
		if (pidd && pidd->VirtualAddress > 0 && pidd->Size > 0)
		{
			if (EnterSegment(pidd->VirtualAddress))
			{
				SAFE_SCOPE_HERE(mr);
#if(1)
				mr.declField("ImageWinCertificate", type(_PFX("WIN_CERTIFICATE")));
#else
				if (mr.NewScope(mr.declField("ImageWinCertificate")))
				{
					SAFE_SCOPE_HERE(mr);
					mr.declField(nullptr, type(_PFX("WIN_CERTIFICATE")));
					mr.setSize(pidd->Size);
				}
#endif
			}
		}
	}

	///////////////////////////////////////////////////////// TLS
	void processTLSTable()
	{
		const IMAGE_DATA_DIRECTORY* pidd(ImageDataDirectoryPtr(IMAGE_DIRECTORY_ENTRY_TLS));
		if (pidd && pidd->VirtualAddress > 0)
		{
			ENTER_PORTAL(mr, pidd->VirtualAddress)
			{
				if (m64bit)
				{
					DECLDATAEX(IMAGE_TLS_DIRECTORY64, u);
					mr.declField("ImageTLSDirectory", type("IMAGE_TLS_DIRECTORY64"));
					//if (mr.EnterScope(u.AddressOfCallBacks))
					{
						//mr.Leave();
					}
				}
				else
				{
					DECLDATAEX(IMAGE_TLS_DIRECTORY32, u);
					mr.declField("ImageTLSDirectory", type("IMAGE_TLS_DIRECTORY32"));
					ENTER_SCOPE(mr, u.AddressOfCallBacks)
					{
						int n(0);
						for (DECLDATAPTR(DWORD, p); p.isValid() && *p; ++p)//range check?
							n++;
						mr.declField("ImageTLSCallbacks", mr.arrayOf(pointerType(), n + 1));
					}
					EXIT_SCOPE(mr);
				}
			}
			EXIT_SCOPE(mr);
		}
	}

	//////////////////////////////////////////////////////Relocations
	void processRelocationsTable()
	{
		const IMAGE_DATA_DIRECTORY* pidd(this->ImageDataDirectoryPtr(IMAGE_DIRECTORY_ENTRY_BASERELOC));
		if (pidd && pidd->Size > 0)
		{
			if (mr.NewScope("FixupBlock"))
			{
				mr.declField("PageRVA", type(TYPEID_DWORD), ATTR_RVA);
				mr.declField("BlockSize", type(TYPEID_DWORD));
				mr.Leave();
			}
			if (EnterSegment(pidd->VirtualAddress))
			{
				int left(pidd->Size);
				while (left > 0)
				{
					DECLDATAEX(FixupBlock, a);
					mr.declField(nullptr, type("FixupBlock"));
					int l((a.BlockSize - sizeof(FixupBlock)) / sizeof(WORD));
					mr.declField(nullptr, arrayOf(type(TYPEID_WORD), l), (AttrIdEnum)ATTR_RELOC);
					left -= a.BlockSize;
				}
				mr.Leave();
			}
		}
	}


	////////////////////////////////////////////////////Bound Import
	void processBoundImportTable()
	{
		const IMAGE_DATA_DIRECTORY* pidd(this->ImageDataDirectoryPtr(IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT));
		if (pidd && pidd->Size > 0)
		{
			DWORD n(pidd->Size / sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR));//max
			if (n > 0 && EnterSegment(pidd->VirtualAddress))
			{
				DECLDATAPTREX(IMAGE_BOUND_IMPORT_DESCRIPTOR, q);
				for (DWORD i(0); i < n; i++, q++)
				{
					if (/*q->TimeDateStamp == 0 && */q->OffsetModuleName == 0/* && q->NumberOfModuleForwarderRefs == 0*/)//end of table?
					{
						n = i + 1;
						break;
					}
					if (mr.EnterScope(RVA2VA(pidd->VirtualAddress + q->OffsetModuleName)))
					{
						mr.declField(nullptr, toAscii(mr));
						mr.Leave();
					}
				}

				/*DECLDATAPTREX(IMAGE_BOUND_IMPORT_DESCRIPTOR, p);
				for (DWORD i(0); i < n; i++, p++)
				{
					if (p->NumberOfModuleForwarderRefs)
					{
						assert(0);
						mr.declField(nullptr, arrayOf(type("IMAGE_BOUND_FORWARDER_REF"), p->NumberOfModuleForwarderRefs));
					}

				}*/
				mr.declField(nullptr, arrayOf(type("IMAGE_BOUND_IMPORT_DESCRIPTOR"), n));
				mr.Leave();
			}
		}
	}


	/////////////////////////////////////////////////////////////Debug
	void processDebugTable()
	{
		const IMAGE_DATA_DIRECTORY* pidd(this->ImageDataDirectoryPtr(IMAGE_DIRECTORY_ENTRY_DEBUG));
		if (pidd && pidd->Size > 0)
		{
			//IMAGE_DEBUG_DIRECTORY data;
			//memset(&data, 0, sizeof(data));
			DWORD SizeOfData(0);
			if (EnterSegment(pidd->VirtualAddress))
			{
				SAFE_SCOPE_HERE(mr);
				DECLDATAEX(IMAGE_DEBUG_DIRECTORY, r);
				mr.declField(nullptr, type("IMAGE_DEBUG_DIRECTORY"));
				
				DWORD rva(r.AddressOfRawData);// ? RVA2VA(r.AddressOfRawData) : 0);
				DWORD fp(r.PointerToRawData);
				/*if (rva == 0 && r.PointerToRawData)
				{
					if (mr.EnterSegment(mr.traceOf(mpe), r.PointerToRawData))
					{
						mr.Leave();
					}
					//if (!FP2RVA(r.PointerToRawData, rva))
					//	rva = 0;
				}*/
				if ((rva && EnterSegment(rva)) || (fp && mr.EnterSegment(mr.traceOf(mpe), r.PointerToRawData)))
				{
					SAFE_SCOPE_HERE(mr);
					if (r.Type == IMAGE_DEBUG_TYPE_CODEVIEW)
					{
						DECLDATA(DWORD, dwSig);
						if (strncmp((const char *)&dwSig, "RSDS", 4) == 0)
						{
							DECLDATAEX(CV_INFO_PDB70, aPdbHdr);
							mr.declField(nullptr, type(_PFX("CV_INFO_PDB70")));
						}
						else if (strncmp((const char*)&dwSig, "NB10", 4) == 0)
						{
							DECLDATAEX(CV_INFO_PDB20, aPdbHdr);
							mr.declField(nullptr, type(_PFX("CV_INFO_PDB20")));
						}
					}
					/*else if (r.Type == IMAGE_DEBUG_TYPE_MISC)
					{
						if (data.SizeOfData)
							mr.declField(nullptr, arrayOf(type(TYPEID_BYTE), data.SizeOfData));//IMAGE_DEBUG_MISC
					}*/
				}
			}
		}
	}

	////////////////////////////////////////////////////////Resource
	void processResourceTable()
	{
		const IMAGE_DATA_DIRECTORY* pidd(ImageDataDirectoryPtr(IMAGE_DIRECTORY_ENTRY_RESOURCE));
		if (pidd)
		{
			//static int z = 0;
			ResourceIterator i(*this);
			for (; i; i++)
			{
				if (i.isFirst())
				{
					//if (EnterSegment(i.rvaDir()))
					ENTER_PORTAL(mr, i.rvaDir())
					{
						DECLDATAEX(IMAGE_RESOURCE_DIRECTORY, IRD);
						if (!mr.declField(nullptr, type("IMAGE_RESOURCE_DIRECTORY")))
							throw (-1);//break the sequence

						//					mr.error("test error");
#if(0)
						const char* pcEntry(i.depth() == 1 ? "IMAGE_RESOURCE_DIRECTORY_ROOT" : "IMAGE_RESOURCE_DIRECTORY_ENTRY");
						//const char *pcEntry("IMAGE_RESOURCE_DIRECTORY_ENTRY");
						if (IRD.NumberOfNamedEntries > 0)
							mr.declField(nullptr, arrayOf(type(pcEntry), IRD.NumberOfNamedEntries));
						if (IRD.NumberOfIdEntries > 0)
							mr.declField(nullptr, arrayOf(type(pcEntry), IRD.NumberOfIdEntries));
#else
						const char* pcEntry(i.depth() == 1 ? _PFX("IMAGE_RESOURCE_DIRECTORY_ROOT") : _PFX("IMAGE_RESOURCE_DIRECTORY_ENTRY"));
						for (int i(0); i < IRD.NumberOfNamedEntries + IRD.NumberOfIdEntries; i++)
						{
							if (!mr.declField(nullptr, type(pcEntry)))
								throw (-2);//cannot create the rest of entries
						}
#endif
					}
					EXIT_PORTAL(mr);
				}

				if (i.isNamedEntry())
				{
					ENTER_PORTAL(m, i.nameRVA())
					{
						//CHECK(z == 257)
						//STOP
						//fprintf(stdout, "%s = %d\n", i.namez().c_str(), z++);
						mr.declField(nullptr, toNUnicode(mr), ATTR_NUNICODE);
					}
					EXIT_PORTAL(mr);
				}

				if (!i.isDirectory())
				{
					ENTER_PORTAL(mr, i.dataEntryRVA())
					{
						DECLDATAEX(IMAGE_RESOURCE_DATA_ENTRY, irdata);
						mr.declField(nullptr, type("IMAGE_RESOURCE_DATA_ENTRY"));
						ENTER_PORTAL(mr, irdata.OffsetToData)
						{
							if (!createResourceItem(i.resourceType()))
								mr.declField(nullptr, arrayOf(type(TYPEID_BYTE), irdata.Size), ATTR_COLLAPSED);
						}
						EXIT_PORTAL(mr);
					}
					EXIT_PORTAL(mr);
				}
			}
			if (i.hasBackRefs())
				fprintf(stderr, "Warning: Possible resource loop(s) ignored\n");
		}
	}

	////////////////////////////////////////////Load Config
	void processLoadConfigTable()
	{
		const IMAGE_DATA_DIRECTORY* pidd(ImageDataDirectoryPtr(IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG));
		if (pidd && pidd->Size > 0)
		{
			ENTER_PORTAL(mr, pidd->VirtualAddress)
			{
				if (!m64bit)
					mr.declField(nullptr, type("IMAGE_LOAD_CONFIG_DIRECTORY32"));
				else
					mr.declField(nullptr, type("IMAGE_LOAD_CONFIG_DIRECTORY64"));
			}
			EXIT_PORTAL(mr);
		}
	}


	///////////////////////////////////////////////////COFF_SYMBOL_ENTRY
	void processCOFFSymbolTable()
	{
		DWORD rva;
		if (ImageFileHeader().PointerToSymbolTable != 0 && ImageFileHeader().NumberOfSymbols > 0 && FP2RVA(ImageFileHeader().PointerToSymbolTable, rva))
		{
			ENTER_SCOPE(mr, RVA2VA(rva))
			{
#if(0)
				//DECLDATA(IMAGE_SYMBOL_ENTRY, ise);
				mr.declField("COFF_SymbolTable", arrayOf(type("IMAGE_SYMBOL_ENTRY"), ImageFileHeader().NumberOfSymbols));
#else
				if (mr.NewScope(mr.declField()))
				{
					SAFE_SCOPE_HERE(mr);
					for (DWORD i(0); i < ImageFileHeader().NumberOfSymbols; i++)
					{
						DECLDATAEX(IMAGE_SYMBOL_ENTRY, u);
						mr.declField(nullptr, type(_PFX("COFF_SymbolEntry")));
						for (int j(0); j < u.NumberOfAuxSymbols; j++, i++)
						{
							DECLDATAEX(IMAGE_SYMBOL_ENTRY, v);
							if (u.TypeLo == IMAGE_SYMBOL_TYPE_FUNCTION)
							{
								mr.declField(nullptr, type("COFF_SymbolEntryAux1"));
								continue;
							}
							if (u.StorageClass == IMAGE_SYM_CLASS_FILE)
								mr.declField(nullptr, type("COFF_SymbolEntryAux4"));
							else if (u.StorageClass == IMAGE_SYM_CLASS_STATIC)// && u.TypeLo != IMAGE_SYMBOL_TYPE_FUNCTION)
								mr.declField(nullptr, type("COFF_SymbolEntryAux5"));
							//else if (u.StorageClass == IMAGE_SYM_CLASS_EXTERNAL && u.TypeLo == IMAGE_SYMBOL_TYPE_FUNCTION && u.SectionNumber > 0)
							///mr.declField(nullptr, type("COFF_SymbolEntryAux1"));
							else
								mr.declField(nullptr, type(_PFX("COFF_SymbolEntry")));
						}
					}
				}
				else
					return;//?
#endif
				//string table comes right after a symbol table.
				if (mr.NewScope(mr.declField("COFF_StringTable")))
				{
					SAFE_SCOPE_HERE(mr);
					DECLDATA(unsigned, uSize);
					POSITION from(mr.cp());
					mr.declField(nullptr, type(TYPEID_DWORD));
					POSITION cur(0);
					do {
						CHECK(cur == 0x4d92c)
							STOP
							mr.declField(nullptr, toAscii(mr));
						cur = mr.cp();
					} while (cur - from < uSize);
				}
			}
			EXIT_SCOPE(mr);
		}
	}


	/////////////////////////////////////// Sections
	void createSections(DWORD entryPointRVA, AuxData_t<PE__META_TOTAL> &meta)
	{
		if (ImageFileHeader().NumberOfSections > 0)
		{
			POSITION atP(0);//for tracking attachement point in parent for traceless segs

			for (int i(0); i < ImageFileHeader().NumberOfSections; i++)
			{
				DECLDATAEX(IMAGE_SECTION_HEADER, ish);

				char buf1[32];
				sprintf(buf1, "ImageSectionHeader_%d", i);
				mr.declField(buf1, type("IMAGE_SECTION_HEADER"));

				ENTER_SCOPE(mr, ish.PointerToRawData != 0 ? RVA2VA(ish.PointerToRawData) : atP)
				{
					//SAFE_SCOPE_HERE(mr);
					std::string buf(this->GetSectionName(ish));

					unsigned flags(m64bit ? I_SuperModule::ISEG_64BIT : I_SuperModule::ISEG_32BIT);//SEG_DATA;
					if (ish.Characteristics & IMAGE_SCN_CNT_CODE)
						flags |= I_SuperModule::ISEG_CODE;
					if (!(ish.Characteristics & IMAGE_SCN_MEM_WRITE))
						flags |= I_SuperModule::ISEG_CONST;
					//					mr.setFlags(flags);

					if (mr.AddSubRange(mhRangeSet, RVA2VA(ish.VirtualAddress), ish.Misc.VirtualSize, mr.NewSegment(ish.SizeOfRawData, buf.c_str(), flags)))
					{
						SAFE_SCOPE_HERE(mr);

						//set the entry point in context of current segment
						if (entryPointRVA != 0)
						{
							DWORD lower(ish.VirtualAddress);
							DWORD upper(lower + (ish.Misc.VirtualSize ? ish.Misc.VirtualSize : ish.SizeOfRawData));
							if (lower <= entryPointRVA && entryPointRVA < upper)
							{
								//mr.enqueEntryPoint(RVA2VA(entryPointRVA));
								if (mr.EnterScope(RVA2VA(entryPointRVA)))
								{
									mr.declCField(nullptr);
									SAFE_SCOPE_HERE(mr);
								}
								entryPointRVA = 0;
							}
						}
						//						if (ish.Characteristics & IMAGE_SCN_CNT_CODE)
						//							mr.DeclareCodeType(_PFX("IA32CODE"));//several sections may have CODE attribute

						if (buf == ".debug_line")
						{
							mr.declField(nullptr, type("DW2_LINES_HEADER"));
						}
						else if (buf == ".debug_info")
						{
							if (mr.EnterAttic())
							{
								DWARF_CreateStructures(mr);
								mr.Leave();
							}
							int cus(0);
							MyDebugInfo di(*this);
							for (typename MyDebugInfo::CU_Iterator i(di); i; ++i, cus++)
							{
//CHECK(i.lowerBound()==0xbd06)
//STOP
								assert(i.dieIterator().tag() == DW_TAG_compile_unit);
								mr.setcp((POSITION)i.lowerBound());
								mr.declField(nullptr, mr.type(_PFX("DWARF_CompilationUnit")), ATTR_COLLAPSED);
								//if (cus == 75)break;
							}
							create_dwarf_meta(ish, meta);
						}
						else if (buf == ".debug_abbrev")
						{
							const IMAGE_SECTION_HEADER* psh_inf(FindSection(".debug_info"));
							MyDebugInfo di(*this);
							for (typename MyDebugInfo::CU_Iterator i(di); i; ++i)
							{
								//WARNING: THIS IS NOT AN ABBREV ITERATOR!
								for (typename MyDebugInfo::CU_Iterator::DIE_Iterator& j(i.dieIterator()); j; ++j)
								{
									POSITION oPos((POSITION)j.abbrevLower());
									OFF_t oDie(j.dieLower());
CHECK(oDie==0x2037)
STOP
									mr.setcp(oPos);
									//IT MAY HAPPEN THE FIELD AT THIS LOCATION ALREADY EXISTS
									mr.declField(nullptr, mr.type(_PFX("DWARF_Abbreviation")), ATTR_COLLAPSED);
									STOP
								}
							}
							STOP
						}
					}

					atP = mr.cp();//NewSegment must have advanced CP in parent by size of its trace
				}
				EXIT_SCOPE(mr);
			}
		}
	}

	void create_dwarf_meta(const IMAGE_SECTION_HEADER &shdr, AuxData_t<PE__META_TOTAL> &meta)
	{
		const IMAGE_SECTION_HEADER* psh_abbr(FindSection(".debug_abbrev"));
		std::vector<dwarf::die2abbrev_t::elt_t>	m;
		m.reserve(1024);
		//int cus(0), dies(0);
		MyDebugInfo di(*this);
		for (typename MyDebugInfo::CU_Iterator i(di); i; ++i)//, cus++)
		{
			assert(i.dieIterator().tag() == DW_TAG_compile_unit);
			for (typename MyDebugInfo::CU_Iterator::DIE_Iterator& j(i.dieIterator()); j; ++j)//, ++dies)
			{
				POSITION a(POSITION(i.dieIterator().dieLower()));
				POSITION a2(POSITION(shdr.PointerToRawData + a));
				POSITION b(POSITION(psh_abbr->PointerToRawData + i.dieIterator().abbrevLower()));
				assert(m.empty() || a2 > m.back().die);
				m.push_back(dwarf::die2abbrev_t::elt_t(a2, b));
			}
		}
		unsigned aux_sz(unsigned(offsetof(dwarf::die2abbrev_t, a) + m.size() * sizeof(dwarf::die2abbrev_t::elt_t)));
		meta.reserve("DIE2ABBREV", aux_sz, PE_META_DWARF);
		dwarf::die2abbrev_t* pmap(meta.data<dwarf::die2abbrev_t>(PE_META_DWARF));
		pmap->n = unsigned(m.size());
		std::copy(m.begin(), m.begin() + m.size(), &pmap->a[0]);
		//memcpy(pmap, &(m)[0], m.size());
	}

	/////////////////////////////////////////////////////DOS
	void createDOSStub()
	{
		unsigned uDosRange(RVA2VA(ImageDOSHeader().e_lfanew) - mr.cp());
		if (uDosRange > 0)
		{
			HTYPE hRangeSet1(nullptr);//mr.NewRangeSet(0));
			if (mr.AddSubRange(hRangeSet1, 0, uDosRange, mr.NewSegment(uDosRange, ".DOSTUB", I_SuperModule::ISEG_16BIT | I_SuperModule::ISEG_CODE)))
			{
				SAFE_SCOPE_HERE(mr);
				mr.installNamespace();
				mr.installTypesMgr();
				mr.setDefaultCodeType(type(_PFX("UDIS86_16")));
				//mr.setFlags(I_Module::ISEG_16BIT | I_Module::ISEG_CODE);
				mr.declField("start", mr.code());
#if(0)
				mr.skip(0xE);
				mr.declUField("test3", mr.type(TYPEID_DWORD));
				mr.declUField("test2", mr.type(TYPEID_WORD));
				mr.declUField("test1", mr.type(TYPEID_BYTE));
#endif
			}
		}
	}


	const char *name() const { return m64bit ? "PE64" : "PE32"; }
	///////////////////////////////////////////////////////////////// Create
	void preformat(I_SuperModule &, unsigned long dataSize)
	{
		__SafeAuxData<PE__META_TOTAL> meta(mr);//meta data will be associated (if any)

		int flavor;
		if (!(flavor = this->CheckWindowsPortableExecutable()))
			return;
		m64bit = (flavor == 2);

		mhRangeSet = mr.NewRangeSet(IBASE(), "primary");

		unsigned flags(m64bit ? I_SuperModule::ISEG_64BIT : I_SuperModule::ISEG_32BIT);
		flags |= I_SuperModule::ISEG_NCASE;

		if (mpe = mr.AddSubRange(mhRangeSet, RVA2VA(0), -1, mr.NewSegment(dataSize, name(), flags)))
		{
			SAFE_SCOPE_HERE(mr);

			mr.installNamespace();

			if (!m64bit)
			{
				mr.installFrontend(_PFX("IA32Front"));
				//mr.setFlags(I_Module::ISEG_32BIT);//this will establish segment's affinity
			}
			else
			{
				mr.installFrontend(_PFX("IA64Front"));
				//mr.setFlags(I_Module::ISEG_64BIT);
			}

			mr.installTypesMgr();
#if (0)
			if (mr.NewScope("TEST"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField("c", type(TYPEID_WORD));
				mr.declField("d", type(TYPEID_WORD));
			}

			if (mr.NewScope("TEST0"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField("a", type(TYPEID_WORD));
				mr.declField("b", arrayOf(type("TEST"), 2));
			}
			mr.declField("test1", type("TEST"));
			//mr.skip(-1);
			mr.declField("test2", type("TEST0"));
			return;
#endif
			if (mr.EnterAttic())
			{
				createStructures();
				//createStructures_dotNET();
				//DWARF_CreateStructures(mr);
				PE_declareDynamicTypes(mr);
				declareDynamicTypes();
				mr.Leave();
			}
			//declareDynamicTypes_dotNET();

			//DECLDATAEX(IMAGE_DOS_HEADER, idh);
			mr.declField("ImageDosHeader", type("IMAGE_DOS_HEADER"));//, ATTR_COLLAPSED);

#if(1)
			createDOSStub();
#endif

			//mr.declField("ImageDosHeader", type("IMAGE_DOS_HEADER"));//, ATTR_COLLAPSED);
			//return;

			//mr.declField("My1stBitset", type("BITSET"));
			//mr.declField("MyTestUnion", type("TestUnion"));
			//mr.declField("UnwindCodes", mr.type("UNWIND_CODE"));
#if(0)
			mr.skip(4);
			mr.declField("Test", type(_PFX("UNWIND_INFO")));
#endif

			//return mpe;
			POSITION oCur(mr.setcp(RVA2VA(ImageDOSHeader().e_lfanew)));
			mr.declField("PESignature", enumOf(type("IMAGE_SIGNATURE"), OPTYP_DWORD));

			//DECLDATAEX(IMAGE_FILE_HEADER, ifh);
			//IMAGE_FILE_HEADER& ifh = this->GetFileHeader();
			oCur = mr.setcp(oCur + sizeof(DWORD));
			DECLDATAEX(IMAGE_FILE_HEADER, ifh);
			mr.declField("ImageFileHeader", type("IMAGE_FILE_HEADER"));

			//DECLDATAEX(IMAGE_OPTIONAL_HEADER, ioh);
			oCur = mr.setcp(oCur + sizeof(IMAGE_FILE_HEADER));
			mr.declField("ImageOptionalHeader", type("IMAGE_OPTIONAL_HEADER"));

			oCur = mr.setcp(oCur + sizeof(IMAGE_OPTIONAL_HEADER));
			mr.declField("DataDirectory", arrayOfIndex(
				arrayOf(type("IMAGE_DATA_DIRECTORY"), this->NumberOfDirectoryEntries()),
				enumOf(type("IMAGE_DIRECTORY_ENTRY"), TYPEID_BYTE)
				));

			//if (!m64bit)
				//mr.setDefaultCodeType(_PFX("IA32CODE"));//available for all sections
		
			switch (ifh.Machine)
			{
			case I386:
				mr.setDefaultCodeType(type(TYPECODE_X86_32));
				break;
			case IA64:
			case AMD64:
				mr.setDefaultCodeType(type(TYPECODE_X86_64));
				break;
			case ARM:
				mr.setDefaultCodeType(type(_PFX("CAPSTONE_ARM")));
			default:
				break;
			}

			createSections(this->GetEntryPointRVA(), meta);
#if(1)
			processCOFFSymbolTable();//and string table

			//directories
			QxTime _timer;
			_timer.start();

			processExportTable(meta);//return;
			_timers[0] += _timer.restart();
			processImportTable();//return;
			_timers[1] += _timer.restart();
			processExceptionsTable();
			processSecurityTable();
			processTLSTable();
			processRelocationsTable();
			processBoundImportTable();
			processDebugTable();
			_timers[2] += _timer.restart();
			processResourceTable();
			_timers[3] += _timer.restart();
			processLoadConfigTable();
			processLoadCOMTable();
			_timers[4] += _timer.elapsed();

#if(0)
			for (size_t i(0); i < _timers.size(); i++)
				fprintf(stdout, "[%d]=%g ", (int)i, _timers[i] / 1000);
			fprintf(stdout, "\n");
#endif
#endif
			//		DWORD entryPointVA(RVA2VA(this->GetEntryPointRVA()));
			//			if (!m64bit)
			//				mr.addEntryPointV(mpe, entryPointVA, true);
			//			if (idd2.Size)
			//			mr.addFileView("rsrc/?.rc");

		}
	}
};




///////////////////////////////////////////////////////// PE_FormatterType

void PE_FormatterType::createz(I_SuperModule &r, unsigned long nSize)
{
	I_SuperModule &mr(r);
	if (mb64bit)
	{
		PE_Formatter_t<PE2_t<ADCPE::IA64_types_t> > PE(mr);
		if (!PE.CheckWindowsPortableExecutable())
			return;
		PE.preformat(mr, nSize);
	}
	else
	{
		PE_Formatter_t<PE2_t<ADCPE::IA32_types_t> > PE(mr);
		if (!PE.CheckWindowsPortableExecutable())
			return;
		PE.preformat(mr, nSize);
	}
	//?return PE.top();

#if(0)//check imported functions
	for (ImportEntryIterator i(PE); i; i++)
	{
		PBYTE pDll = i.moduleName();
		PBYTE pFunc(i.functionName());
		if (!pFunc)
		{
			fprintf(stdout, "by ordinal: %08X\n", i.functionVA());
			continue;
		}
		DWORD va(i.functionVA());
		fprintf(stdout, "%s\t%s\t%08X\n", pDll, pFunc, va);
	}
#endif
}


DECLARE_FORMATTER1(PE_FormatterType, PE32, false);
DECLARE_FORMATTER1(PE_FormatterType, PE64, true);
DECLARE_FORMATTER(COFF_FormatterType, COFF32);
//DECLARE_FORMATTER(CodeX8632bitType, X86_IA32);


void PE_RegisterFormatters(I_ModuleEx &rMain)
{
	rMain.RegisterFormatterType(_PFX("PE32"));
	rMain.RegisterFormatterType(_PFX("PE64"));
	rMain.RegisterFormatterType(_PFX("COFF32"));
	//rMain.RegisterFormatterType(_PFX("X86_IA32"));
}




