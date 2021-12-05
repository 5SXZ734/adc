set(SRC_FILES
	libudis86/decode.c
	libudis86/itab.c
	libudis86/syn.c
	libudis86/syn-att.c
	libudis86/syn-intel.c
	libudis86/udis86.c
	libudis86/decode.h
	libudis86/extern.h
	libudis86/itab.h
	libudis86/syn.h
	libudis86/types.h
	libudis86/udint.h
)

#list(TRANSFORM ${SRC_FILES} PREPEND "libudis86/")

add_definitions(-D_USRDLL)

add_library(udis86 SHARED ${SRC_FILES})


#target_include_directories(udis86 PUBLIC ${INCLUDES})

target_include_directories(udis86 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(udis86 PUBLIC "LIBUDIS86_EXPORTS")

install(TARGETS udis86 RUNTIME DESTINATION ${INSTALL_SUBDIR})
