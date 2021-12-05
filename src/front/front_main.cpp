
#include "shared.h"
#include "format.exe.h"
#include "format.pe.h"
#include "format.misc.h"
#include "format.elf.h"
#include "format.pdb.h"
#include "format.java.h"
#include "PDB.h"
#include "front_main.h"
#include "interface/IADCGui.h"
#include <sstream>

///////////////////////////////////////////////////////////////////FrontMain_t

enum FILETYP_t
{
	FILETYP_NULL,
	FILETYP_ADC,
	FILETYP_LUASCRIPT,
	FILETYP_STUBS,
	//FILETYP_FEID,	//filetype makes known the frontend
	FILETYP_EXE,// = FILETYP_FEID + FE_X86_32,
	//FILETYP_EXE,// = FILETYP_FEID + FE_X86_16,	//DOS exe
	FILETYP_MPL,// = FILETYP_FEID + FE_MPL,
	FILETYP_JAVA,// = FILETYP_FEID + FE_JAVA,
	FILETYP_ICO// = FILETYP_FEID + FE_ICO
};

static int CheckHSpiceOutfile(const std::string &sExt)
{
	int ret(0);
	if (sExt.compare(0, 2, "tr") == 0)
		ret = 1;
	else if (sExt.compare(0, 2, "sw") == 0 || sExt.compare(0, 2, "cx") == 0)
		ret = 2;
	else if (sExt.compare(0, 2, "ac") == 0)
		ret = 3;
	else if (sExt.compare(0, 2, "ft") == 0)
		ret = 4;
	else
		return 0;
	if (sExt.substr(2).find_first_not_of("0123456789") != std::string::npos)//is_number?
		return 0;
	return ret;
}

static int CheckSilosSimSignature(const I_DataSourceBase& aRaw)
{
	DataStream_t aPtr(aRaw, 0);
	return (aPtr.read<unsigned char>() == 0xA7);//CR_CNTL_BLOCK
}

static const char * EstimateFrontend(const I_DataSourceBase &aRaw, unsigned size, const char *ext)
{
	//#ifdef WIN32
	std::string sExt(ext ? ext : "");
	std::transform(sExt.begin(), sExt.end(), sExt.begin(), [](unsigned char c){ return (char)std::tolower(c); });
	//#endif

	if (sExt == PRIMARY_EXT)//adc binary workspace
		return _PFX("FORMAT_ADC");
	if (sExt == "class")
		return _PFX("FORMAT_JAVA_CLASS");
	if (sExt == "mpl")
		return _PFX("PEUS.MPL");
	if (sExt == "ico")
		return _PFX("RC_ICO");
	if (sExt == "pdb")
	{
		switch (CheckPdbSignature(aRaw, size))
		{
		case 2: return _PFX("MS_PDB20");
		case 7: return _PFX("MS_PDB70");
		default: break;
		}
	}
	if (sExt == "bmp")
		return _PFX("RC_BMP");
	if (sExt == "trz")
		return _PFX("HSPICE_TRZ");
	int ihs(CheckHSpiceOutfile(sExt));
	if (ihs == 1)
		return _PFX("HSPICE_TR0");
	if (ihs == 2)
		return _PFX("HSPICE_SW0");
	if (ihs == 3)
		return _PFX("HSPICE_AC0");
	if (ihs == 4)
		return _PFX("HSPICE_FT0");
	if (sExt == "sim" && CheckSilosSimSignature(aRaw))
		return _PFX("SILOS_SIM");

	switch (CheckElfSignature(aRaw, size))
	{
	case 1: return _PFX("ELF32");
	case 2: return _PFX("ELF64");
	default: break;
	}

	switch (CheckPortableExecutable(aRaw, size))
	{
	case -1: return _PFX("MSDOS_EXE");
	case 1: return _PFX("PE32");
	case 2: return _PFX("PE64");
	default: break;
	}

	return nullptr;
}


const char * FrontMain_t::RecognizeFormat(const I_DataSourceBase &mr, const char *ext)
{
	return EstimateFrontend(mr, (unsigned)mr.size(), ext);
}

I_Front *FrontMain_t::CreateFrontend(const char *frontName, const I_DataSourceBase *aRaw, const I_Main* pIMain)
{
	if (!frontName)
		return nullptr;
	if (strcmp(frontName, _PFX("IA16Front")) == 0)
		return CreateIA16Front(aRaw, pIMain);
	if (strcmp(frontName, _PFX("IA32Front")) == 0)
		return CreateIA32Front(aRaw, pIMain);
	if (strcmp(frontName, _PFX("IA64Front")) == 0)
		return CreateIA64Front(aRaw, pIMain);
	if (strcmp(frontName, _PFX("ELF32Front")) == 0)
		return CreateELF32Front(aRaw);
	if (strcmp(frontName, _PFX("ELF64Front")) == 0)
		return CreateELF64Front(aRaw);
	if (strcmp(frontName, _PFX("FE_PDB")) == 0)
		return CreateFE_PDB(aRaw);
	if (strcmp(frontName, _PFX("FE_SILOS")) == 0)
		return CreateFE_SILOS(aRaw);
	if (strcmp(frontName, _PFX("FE_JAVA")) == 0)
		return CreateFE_JAVA(aRaw);
	return nullptr;
}


const char *FrontMain_t::name() const { return FRONT_NAME_PFX; }

void ADC_RegisterFormatters(I_ModuleEx &);

void FrontMain_t::RegisterTypes(I_ModuleEx &rMain)
{
	EXE_RegisterFormatters(rMain);
	PE_RegisterFormatters(rMain);
	ICO_RegisterFormatters(rMain);
	HSPICE_RegisterFormatters(rMain);
	ELF_RegisterFormatters(rMain);
	PDB_RegisterFormatters(rMain);
	PDB20_RegisterFormatters(rMain);
	ADC_RegisterFormatters(rMain);
	JAVA_RegisterFormatters(rMain);
}


DECLARE_DYNAMIC_TYPE(FrontMain_t, FrontMain);

