#include "symbol_map.h"
#include "mem.h"
#include "front_impl.h"
#include "main.h"


I_Front::SymbolKind demangleSymbol(I_Front *pIFront, const MyString &symbol, MyString& sDemangled)
{
	MyStream ss;
	I_Front::SymbolKind eKind;
	int n(symbol.find('.'));
	if (n > 0)
		eKind = pIFront->demangleName(symbol.left(n), ss);
	else
		eKind = pIFront->demangleName(symbol, ss);
	ss.ReadString(sDemangled);
	return eKind;
}


//////////////////////////////////////////////////////////////////SymbolMap

SymbolMap::SymbolMap(I_DataSource &r, I_Front *pIFront)
: mRaw(r),
mpIFront(pIFront)
{
}

void SymbolMap::dumpImported()
{
	try
	{
		SymbolFetch_t symDump(*this);// I_Front::DUMP_IE_NAMED_ONLY);
		mpIFront->dumpImports(&symDump, I_Front::DUMP_IE_ALL);
	}
	catch (...)
	{
		MAIN.printError() << "unknown" << std::endl;
	}
}

void SymbolMap::dumpExported()
{
	try
	{
		SymbolFetch_t symDump(*this);// I_Front::DUMP_IE_NAMED_ONLY);
		mpIFront->dumpExports(&symDump, I_Front::DUMP_IE_ALL);
	}
	catch (...)
	{
		MAIN.printError() << "unknown" << std::endl;
	}
}


/////////////////////////////////////////////////////////////DumpDebugInfoCallback

DumpDebugInfoCallback::DumpDebugInfoCallback(Project_t &rProj, TypePtr iModule, SymbolMap &_ss)
	: FrontImplBase_t(rProj, iModule),
	symIt(_ss)
{
}

void DumpDebugInfoCallback::dump(ADDR va, OFF_t oSymbolName, unsigned uNameMax, unsigned uFlags)
{
	if (oSymbolName == OFF_NULL)
		return;//pSymbolName = "<noname>";
	//fprintf(stdout, "SYMBOL @%08X %s\n", va, pSymbolName);
	SymbolInfo a;
	//a.oModuleName = OFF_NULL;
	a.moSymbolName = oSymbolName;
	//a.sSymbolName = sName;
	a.muNameMax = uNameMax;
	a.mva = va;
	a.meKind = I_Front::SYMK_NULL;
	symIt.push_back(a);
	//CHECK(symIt.size() == 0x213)
	//STOP
}

void DumpDebugInfoCallback::dump(ADDR va, const char *pSymbolName, unsigned uFlags)
{
	//fprintf(stdout, "SYMBOL @%08X %s\n", va, pSymbolName);
	SymbolInfo a;
	//a.oModuleName = OFF_NULL;
	a.moSymbolName = OFF_NULL;
	a.msSymbolName = MyString(pSymbolName);
	a.muNameMax = 0;//ignored
	a.mva = va;
	a.meKind = I_Front::SYMK_NULL;
	symIt.push_back(a);
}

void DumpDebugInfoCallback::dumpSrc(const char *path)
{
	symIt.addSrc(path);
	//fprintf(stdout, "%s\n", path);
}

void DumpDebugInfoCallback::resetProgress(const char *desc, unsigned)
{
}


#define NO_TRY2	0
void SymbolMap::dumpDebugInfo1(const ModuleInfo_t &MI)
{
#if(NO_TRY2)
	try
#endif
	{
		DumpDebugInfoCallback user(MI.Project(), MI.ModulePtr(), *this);
		FrontIfaceDbg_t iface(user);
		mpIFront->dumpDebugInfo(I_ModuleCB::DUMP_D_FUNCTIONS_ONLY, iface);
	}
#if(NO_TRY2)
	catch (int code)
	{
		PrintError() << "[dumpDebugInfo]: code: " << code << std::endl;
	}
	catch (...)
	{
		PrintError() << "[dumpDebugInfo]: unknown" << std::endl;
	}
#endif
}
#undef NO_TRY2




