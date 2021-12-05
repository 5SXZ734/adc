set(SRC_FILES
	demanglers.cpp
	demanglers.h
	src/undname.c
	src/undname.h
	src/libiberty/cp-demangle.c
	src/libiberty/cplus-dem.c
	src/libiberty/d-demangle.c
	src/libiberty/rust-demangle.c
	src/libiberty/safe-ctype.c
	src/libiberty/xexit.c
	src/libiberty/xmalloc.c
	src/libiberty/xmemdup.c
	src/libiberty/xstrdup.c
	src/libiberty/ansidecl.h
	src/libiberty/cp-demangle.h
	src/libiberty/demangle.h
	src/libiberty/environ.h
	src/libiberty/libiberty.h
	src/libiberty/safe-ctype.h
	src/llvm/Demangle/Demangle.cpp
	src/llvm/Demangle/ItaniumDemangle.cpp
	src/llvm/Demangle/MicrosoftDemangle.cpp
	src/llvm/Demangle/MicrosoftDemangleNodes.cpp
	src/llvm/Demangle/Demangle.h
	src/llvm/Demangle/DemangleConfig.h
	src/llvm/Demangle/ItaniumDemangle.h
	src/llvm/Demangle/MicrosoftDemangle.h
	src/llvm/Demangle/MicrosoftDemangleNodes.h
	src/llvm/Demangle/StringView.h
	src/llvm/Demangle/Utility.h
)

add_library(demanglers SHARED ${SRC_FILES})


target_include_directories(demanglers PUBLIC src)
target_include_directories(demanglers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(demanglers PUBLIC "DEMANGLERS_SHARED")
target_compile_definitions(demanglers PUBLIC "_CRT_SECURE_NO_WARNINGS")

install(TARGETS demanglers RUNTIME DESTINATION ${INSTALL_SUBDIR})
