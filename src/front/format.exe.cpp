
#include "format.exe.h"

#include "shared/front.h"
#include "shared/data_source.h"
#include "shared.h"
#include "wintypes.h"
#include "front_IA.h"


using namespace adcwin;
///////////////////////////////////////////////////////// MSDOSExecutableType


void MSDOSExecutableType::createz(I_SuperModule &mr, unsigned long dataSize)
{
	if (CheckPortableExecutable(mr, dataSize) != -1)
		return;

	HTYPE hRangeSet0(mr.NewRangeSet(0));

	if (mpe = mr.AddSubRange(hRangeSet0, 0, 0, mr.NewSegment(dataSize, name(), I_SuperModule::ISEG_16BIT)))
	{
		SAFE_SCOPE_HERE(mr);

		mr.installNamespace();

		//IMAGE_DOS_HEADER &idh(pe.GetImageDosHeader());
		mr.installFrontend(_PFX("IA16Front"));
		//mr.setFlags(I_Module::ISEG_16BIT);//this will establish segment's affinity

		mr.installTypesMgr();
		createStructures(mr);
		declareDynamicTypes(mr);

		mr.setDefaultCodeType(mr.type(_PFX("UDIS86_16")));


		DECLDATAEX(IMAGE_DOS_HEADER, idh);
		mr.declField("ImageDosHeader", mr.type("EXEHEADER"));//, ATTR_COLLAPSED);

		POSITION aStart(idh.e_cparhdr * 16);
		mr.setcp(aStart);

		mr.declField("start", mr.type(_PFX("UDIS86_16")));

		STOP
	}
}

void MSDOSExecutableType::declareDynamicTypes(I_ModuleEx &mr)
{
	mr.DeclareCodeType(_PFX("UDIS86_16"));
}

enum IA16_AttrIdEnum
{
	ATTR16_START = ATTR___MISC_BEGIN,
	ATTR16_END
};

void MSDOSExecutableType::createStructures(I_Module &mr)
{
	if (mr.NewScope("EXEHEADER"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("Signature", mr.type(TYPEID_WORD));
		mr.declField("ExtraBytes", mr.type(TYPEID_WORD));
		mr.declField("Pages", mr.type(TYPEID_WORD));
		mr.declField("RelocItems", mr.type(TYPEID_WORD));
		mr.declField("HeaderSize", mr.type(TYPEID_WORD), (AttrIdEnum)ATTR16_START);
		mr.declField("MinAlloc", mr.type(TYPEID_WORD));
		mr.declField("MaxAlloc", mr.type(TYPEID_WORD));
		mr.declField("InitSS", mr.type(TYPEID_WORD));
		mr.declField("InitSP", mr.type(TYPEID_WORD));
		mr.declField("CheckSum", mr.type(TYPEID_WORD));
		mr.declField("InitIP", mr.type(TYPEID_WORD));
		mr.declField("InitCS", mr.type(TYPEID_WORD));
		mr.declField("RelocTable", mr.type(TYPEID_WORD));
		mr.declField("Overlay", mr.type(TYPEID_WORD));
		if (mr.NewScope(mr.declField()))//nullptr, SCOPE_ UNION))
		{
			SAFE_SCOPE_HERE(mr);
			if (mr.NewScope(mr.declUField("NewExecutableInfo")))
			{
				SAFE_SCOPE_HERE(mr);
				//mr.declField(nullptr, mr.arrayOf(mr.type(TYPEID_BYTE), 4));
				//mr.declField("BehaviourBits", mr.type(TYPEID_WORD));
				//mr.declField("Reserved", mr.arrayOf(mr.type(TYPEID_BYTE), 26));
				//mr.declField("NewOffset", mr.type(TYPEID_DWORD), ATTR_OFFS);//LONG
				mr.declField("e_res", mr.arrayOf(mr.type(TYPEID_WORD), 4));
				mr.declField("e_oemid", mr.type(TYPEID_WORD));
				mr.declField("e_oeminfo", mr.type(TYPEID_WORD));
				mr.declField("e_res2", mr.arrayOf(mr.type(TYPEID_WORD), 10));
				mr.declField("e_lfanew", mr.type(TYPEID_DWORD), ATTR_OFFS);//LONG
			}
			if (mr.NewScope(mr.declUField("BorlandTLinkInfo")))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField("Sig", mr.type(TYPEID_WORD));
				mr.declField("Id", mr.type(TYPEID_BYTE));
				mr.declField("Version", mr.type(TYPEID_BYTE));
				mr.declField("Reserved", mr.type(TYPEID_WORD));
			}
		}
	}
}


class IA16Front_t : public I_Front
{
	const I_DataSourceBase *mpRaw;
	const I_Main* mpIMain;
public:
	IA16Front_t(const I_DataSourceBase *aRaw, const I_Main* pIMain)
		: mpRaw(aRaw),
		mpIMain(pIMain)
	{
	}
	virtual ~IA16Front_t()
	{
	}
	virtual AKindEnum translateAddress(const I_DataStreamBase &r, int moduleId, ADDR &addr, AttrIdEnum attr){
		if ((IA16_AttrIdEnum)attr == ATTR16_START)
		{
			addr = addr * 16;
			return AKIND_RAW;
		}
		return AKIND_NULL;
	}
};


I_Front *CreateIA16Front(const I_DataSourceBase *aRaw, const I_Main* pIMain)
{
	return new IA16Front_t(aRaw, pIMain);
}

class CodeX8616bitType : public I_FormatterType
{
public:
	CodeX8616bitType(){}
	virtual const char *name() const { return _PFX("X86_IA16"); }
	virtual void createz(I_SuperModule &, unsigned long nSize)
	{
		assert(0);//? mr.CreateCode(new IA32Code_t(pSelf, false));
	}
};

DECLARE_FORMATTER(MSDOSExecutableType, MSDOS_EXE);
DECLARE_FORMATTER(CodeX8616bitType, X86_IA16);

void EXE_RegisterFormatters(I_ModuleEx &rMain)
{
	rMain.RegisterFormatterType(_PFX("MSDOS_EXE"));
	rMain.RegisterFormatterType(_PFX("X86_IA16"));
}

