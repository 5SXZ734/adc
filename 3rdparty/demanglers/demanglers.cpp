#include "demanglers.h"
#include <cstddef>
#include "undname.h"
#include "libiberty/demangle.h"
#include "llvm/Demangle/Demangle.h"

DEMANGLERS_EXPORT
char* undname(const char* mangled)
{
	return __unDName(NULL, mangled, 0, malloc, free, UNDNAME_NO_ACCESS_SPECIFIERS);
}

DEMANGLERS_EXPORT
char * iberty_demangle(const char *mangled)
{
	return cplus_demangle(mangled, DMGL_PARAMS | DMGL_ANSI);
	//return cplus_demangle_v3(mangled, DMGL_PARAMS | DMGL_ANSI);///*|DMGL_TYPES//|DMGL_RET_POSTFIX//|DMGL_TYPES//|DMGL_VERBOSE);
}

DEMANGLERS_EXPORT
char *llvm_demangle_msvc(const char *mangled)
{
#if _MSC_VER >= 1900		//Visual Studio 2015 (14.0)
	return llvm::microsoftDemangle(mangled, nullptr, nullptr, nullptr, nullptr);
#else
	return NULL;
#endif
}
DEMANGLERS_EXPORT
void free_demangled(char *demangled)
{
	free(demangled);
}




