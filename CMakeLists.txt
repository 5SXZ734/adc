cmake_minimum_required(VERSION 3.21)

set(SS_MAKE_DIR "${CMAKE_SOURCE_DIR}/../MyMake")

if (WIN32)
	set(CMAKE_GENERATOR_PLATFORM x64)
else()
	set(SSMAKE_GCC_V 8.5)
	include(${SS_MAKE_DIR}/Compiler.cmake)
endif()

project(adc)
set(VERSION 2.0.2.R)

include(${SS_MAKE_DIR}/Parameters.cmake)

#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED True)


set(VS_STARTUP_PROJECT adc)

#option(NEW_OP_PTR "Save/Restore support for OP objects" 1)
#option(NEW_PATH_PTR "Save/Restore support for PATH objects" 1)


set(PROG_DEFINE NOMINMAX _CRT_SECURE_NO_WARNINGS)

if (MSVC)
    # Multi-processor Compilation (/MP)
 #   add_compile_options(/MP)
else()
    # lots of warnings and all warnings as errors
 #   add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()


#list(APPEND LIBS db dc shared)


include(src/files.cmake)

############################################## shared

set(LIB1_DEFINE _CRT_SECURE_NO_WARNINGS)
set(LIB1_INCLUDE src)
set(LIB1_LIB -Uqx_1_0_0_R)
set(LIB1_LOCAL 1)

set(LIB1_SRC ${SHARED_SRC})

DeclareLibrary(LIB1 shared)

############################################## db

set(LIB2_DEFINE _CRT_SECURE_NO_WARNINGS)
set(LIB2_INCLUDE src src/shared)
set(LIB2_LIB -Uqx_1_0_0_R -Utiny_process_1_0_0_R)
set(LIB2_LOCAL 1)

set(LIB2_SRC ${DB_SRC})


DeclareLibrary(LIB2 db)

############################################## dc

set(LIB3_DEFINE _CRT_SECURE_NO_WARNINGS)
set(LIB3_INCLUDE src src/shared)
set(LIB3_LIB -Uqx_1_0_0_R -Utiny_process_1_0_0_R)
set(LIB3_LOCAL 1)

set(LIB3_SRC ${DC_SRC})


DeclareLibrary(LIB3 dc)


############################################## adcui5

set(QT_DIR $ENV{QT_DIR})
set(QT_VERSION "5.15.1")
#set(QT_SHARED 1)

FindQT5()

set(LIB4_SRC ${UIQ5_SRC})
set(LIB4_H ${UIQ5_H})
set(LIB4_UIC ${UIQ5_UIC})
set(LIB4_MOC ${UIQ5_SRC_MOC})
set(LIB4_RCC ${UIQ5_RCC})

set(LIB4_DEFINE "EXPORT_IMPORT_API=")

set(LIB4_INCLUDE
	src
	src/shared
	${SSMAKE_GEN_DIR}/src/uiqt5
	)

set(LIB4_LIB
	-Iqx_1_0_0_R
	-IVersion_1_0_0_R
	-Pqt:${QT_VERSION}
)

if(QT_STATIC)
	list(APPEND LIB4_LIB dwmapi wtsapi32 imm32 ws2_32 winmm netapi32 userenv version)
endif()

set(LIB4_SHARED_MODULE 1)

DeclareLibrary(LIB4 uiqt5)


############################################## dc

set(LIB5_DEFINE _CRT_SECURE_NO_WARNINGS)
set(LIB5_INCLUDE src src/shared)
set(LIB5_LIB 
	shared
	-Iqx_1_0_0_R 
	-Ddemanglers_4_0_2_R:demanglers
	-Dcapstone_4_0_2_R:capstone
	-Dudis86_7_0_2_R:udis86
	)
set(LIB5_LOCAL 1)
set(LIB5_SHARED 1)

set(LIB5_SRC ${FRONT_SRC})


DeclareLibrary(LIB5 front)

############################################## adc


set(PROG_INCLUDE src)

set(PROG_SRC ${ADC_SRC} ${ADC_H})
set(PROG_EXTRA src/files.cmake)


set(PROG_LIB 
	db 
	dc 
	shared
	-Iqx_1_0_0_R 
	-Itiny_process_1_0_0_R
	-IVersion_1_0_0_R
#	-Dcapstone_4_0_2_R:capstone
#	-Ddemanglers_4_0_2_R:demanglers
#	-Dudis86_7_0_2_R:udis86
	)

#add_dependencies(adc front uiqt5)


DeclareProgram(PROG adc)

set(PROTO_DIR $<TARGET_FILE_DIR:front>/proto)

set(INI_FILE "adc.ini")
add_custom_command(TARGET adc POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${INI_FILE} "$<TARGET_FILE_DIR:adc>" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)