#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#pragma warning(disable:4201)
#pragma warning(disable:4100)
#ifdef DEMANGLERS_SHARED
#define DEMANGLERS_EXPORT __declspec(dllexport)
#else    // defined(DEMANGLERS_STATIC)
#define DEMANGLERS_EXPORT
#endif
#else
#if defined(__GNUC__) && !defined(DEMANGLERS_STATIC)
#define DEMANGLERS_EXPORT __attribute__((visibility("default")))
#else    // defined(DEMANGLERS_STATIC)
#define DEMANGLERS_EXPORT
#endif
#endif


DEMANGLERS_EXPORT
char* undname(const char* mangled);

DEMANGLERS_EXPORT
char * iberty_demangle(const char *mangled);

DEMANGLERS_EXPORT
char *llvm_demangle_msvc(const char *mangled_name);

DEMANGLERS_EXPORT
void free_demangled(char *);


#ifdef __cplusplus
}
#endif


