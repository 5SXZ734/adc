set(SRC_FILES_ALL
	front_main.cpp
	shared.cpp
	front_main.h
	shared.h
	wintypes.h
)
list(APPEND SRC_FILES ${SRC_FILES_ALL})
source_group("" FILES ${SRC_FILES_ALL})

set(SRC_FILES_MISC
	format.adc.cpp
	format.ico.cpp
	format.tr0.cpp
	format.misc.h
)
list(APPEND SRC_FILES ${SRC_FILES_MISC})
source_group("misc" FILES ${SRC_FILES_MISC})

set(SRC_FILES_X86
	cpu16.cpp
	cpu32.cpp
	cpu64.cpp
	front_IA.cpp
	x86_dump.cpp
	x86_IR.cpp
	front_IA.h
	x86.h
	x86_dump.h
	x86_IR.h
	fpu32.inl
	fpu32.1.inl
	jumps16.inl
	jumps32.inl
	sets32.inl
)
list(APPEND SRC_FILES ${SRC_FILES_X86})
source_group("X86" FILES ${SRC_FILES_X86})

set(SRC_FILES_JAVA
	format.java.cpp
	format.java.h
)
list(APPEND SRC_FILES ${SRC_FILES_JAVA})
source_group("JAVA" FILES ${SRC_FILES_JAVA})

set(SRC_FILES_PDB
	decode.PDB.cpp
	format.pdb_D.cpp
	format.pdb_S.cpp
	format.pdb20.cpp
	format.pdb70.cpp
	decode.PDB.h
	format.pdb.h
	PDB.h
)
list(APPEND SRC_FILES ${SRC_FILES_PDB})
source_group("PDB" FILES ${SRC_FILES_PDB})

set(SRC_FILES_PE
	decode.COFF.h
	decode.pe.cpp
	format.exe.cpp
	format.pe.cpp
	format.pe.NET.cpp
	format.pe_D.cpp
	format.pe_S.cpp
	decode.pe.h
	decode.RTTI.h
	format.exe.h
	format.pe.h
	PE.h
)
list(APPEND SRC_FILES ${SRC_FILES_PE})
source_group("PE" FILES ${SRC_FILES_PE})

set(SRC_FILES_ELF
	format.elf.cpp
	format.elf_S.cpp
	ELF.h
	format.elf.h
)
list(APPEND SRC_FILES ${SRC_FILES_ELF})
source_group("ELF" FILES ${SRC_FILES_ELF})


set(SRC_FILES_DWARF
	decode.DWARF.cpp
	format.dwarf.cpp
	decode.DWARF.h
	DWARF.h
	format.dwarf.h
)
list(APPEND SRC_FILES ${SRC_FILES_DWARF})
source_group("DWARF" FILES ${SRC_FILES_DWARF})


set(SRC_FILES_CODE
	code.capstone.cpp
	code.udis86.cpp
)
list(APPEND SRC_FILES ${SRC_FILES_CODE})
source_group("CODE" FILES ${SRC_FILES_CODE})

macro(SrcList _id _src)
	foreach (i ${_src})
		#list(APPEND SRC_FILES_${_id} ${i})
		#list(APPEND SRC_FILES ${i})
	endforeach()
#	message("SRC_FILES=" "${_src}")
	list(APPEND SRC_FILES_${_id} ${_src})
	list(APPEND SRC_FILES ${_src})
	source_group("CV" FILES ${_src})
endmacro()

#[[set(SRC_FILES_CV
	cv/cvconst.h
	cv/cvinfo.h
)
list(APPEND SRC_FILES ${SRC_FILES_CV})
source_group("CV" FILES ${SRC_FILES_CV})
]]

SrcList("CV"
	"cv/cvconst.h;cv/cvinfo.h"
	)

########################################

add_library(front MODULE)
target_sources(front PRIVATE ${SRC_FILES})

target_include_directories(front PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(front PRIVATE ${PROJECT_SOURCE_DIR}/src/shared)
#target_include_directories(front PRIVATE ${PROJECT_SOURCE_DIR}/src/shared)

target_link_libraries(front PUBLIC shared)
target_link_libraries(front PUBLIC udis86)
target_link_libraries(front PUBLIC demanglers)
target_link_libraries(front PUBLIC capstone)


set(PROTO_DIR $<TARGET_FILE_DIR:front>/proto)

set(PROTO_FILES
	proto/MFC42.DLL.export
	proto/ADVAPI32.DLL.h
	proto/COMDLG32.DLL.h
	proto/GDI32.DLL.h
	proto/KERNEL32.DLL.h
	proto/MSVCRT.DLL.h
	proto/SHELL32.DLL.h
	proto/USER32.DLL.h
	proto/WINSPOOL.DRV.h
	proto/front.stubs.hh
	proto/front.types.hh
)

add_custom_command(TARGET front POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory "${PROTO_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy ${PROTO_FILES} "${PROTO_DIR}" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# define the symbol stating we are using the declspec(dllexport) when
# building on windows
#target_compile_definitions(front PRIVATE "ADCFRONT_EXPORT")

# state that SqrtLibrary need PIC when the default is shared libraries
#set_target_properties(front PROPERTIES POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})

# install rules
#set(installable_libs MathFunctions)

install(TARGETS front DESTINATION ${INSTALL_SUBDIR})

install(FILES ${PROTO_FILES} DESTINATION ${INSTALL_SUBDIR}/proto)





