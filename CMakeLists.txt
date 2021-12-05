cmake_minimum_required(VERSION 3.21)

set(CMAKE_GENERATOR_PLATFORM Win32) #before project
set(CMAKE_CONFIGURATION_TYPES Debug Release)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)	#hide predefined targets

project(adc VERSION 2.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

string(TIMESTAMP BUILD_DATE "%y%m%d" UTC)
message("TIMESTAMP=" ${BUILD_DATE})


# Set the install directory
set(CMAKE_INSTALL_PREFIX "$ENV{USERPROFILE}")
string(REPLACE "\\" "/" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
set(INSTALL_SUBDIR "adc/${BUILD_DATE}$<$<CONFIG:Debug>:/_debug>")


set(VS_STARTUP_PROJECT adc)

#option(NEW_OP_PTR "Save/Restore support for OP objects" 1)
#option(NEW_PATH_PTR "Save/Restore support for PATH objects" 1)

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

add_definitions(-DNOMINMAX)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if (MSVC)
    # Multi-processor Compilation (/MP)
    add_compile_options(/MP)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

configure_file(config.h.in config.h)

add_subdirectory(3rdparty/udis86)
add_subdirectory(3rdparty/demanglers)
add_subdirectory(3rdparty/capstone)

add_subdirectory(src/db)
add_subdirectory(src/dc)
add_subdirectory(src/shared)
add_subdirectory(src/front)
add_subdirectory(src/uiqt5)

list(APPEND LIBS db dc shared)

list(APPEND INCLUDES "${PROJECT_SOURCE_DIR}/src")
#list(APPEND INCLUDES "${PROJECT_SOURCE_DIR}/src/shared")

set(SRC_FILES
    src/adc.rc
    src/adc.cpp
    src/loadui.cpp
    src/startup.cpp
    src/WinMain.cpp
    src/adb.h
    src/adc.h
    src/config.h
    src/loadui.h
    src/prefix.h
    src/resource.h
    src/startup.h
    src/targetver.h
)

set(H_FILES
	src/interface/IADCFront.h
	src/interface/IADCGui.h
	src/interface/IADCMain.h
)

#source_group("" FILES ${SRC_FILES})

add_executable(adc WIN32 "")
target_sources(adc PRIVATE ${SRC_FILES} ${H_FILES})

source_group("src" FILES ${SRC_FILES})
source_group("interface" FILES ${H_FILES})

target_link_libraries(adc PUBLIC ${LIBS})

target_include_directories(adc PUBLIC ${INCLUDES})

add_dependencies(adc front uiqt5)

set(PROTO_DIR $<TARGET_FILE_DIR:front>/proto)

set(INI_FILE "adc.ini")
add_custom_command(TARGET adc POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${INI_FILE} "$<TARGET_FILE_DIR:adc>" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)


install(TARGETS adc RUNTIME DESTINATION ${INSTALL_SUBDIR})


install(FILES $<TARGET_FILE_DIR:adc>/${INI_FILE} DESTINATION ${INSTALL_SUBDIR})


#example:

#cd ..\build
#cmake -DCMAKE_INSTALL_PREFIX="C:\test" -DCMAKE_GENERATOR_PLATFORM=Win32|x64 ..\adc
#cmake --build . --config Release --target INSTALL
#bin\helloWorld.exe