set(SRC_FILES_QX
	qx/ConvertUTF.cpp
	qx/DynLib.cpp
	qx/MyApp.cpp
	qx/MyArgs.cpp
	qx/MyCapture.cpp
	qx/MyDir.cpp
	qx/MyFileMgr.cpp
	qx/MyGuiThread.cpp
	qx/MyPath.cpp
	qx/MyRedirect.cpp
	qx/MyRedirect2.cpp
	qx/MyStream.cpp
	qx/MyString.cpp
	qx/MyStringList.cpp
	qx/QxTime.cpp
	qx/ConvertUTF.h
	qx/DynLib.h
	qx/IGui.h
	qx/IString.h
	qx/IUnk.h
	qx/MyApp.h
	qx/MyArgs.h
	qx/MyCapture.h
	qx/MyDir.h
	qx/MyFileMgr.h
	qx/MyGuiThread.h
	qx/MyMemoryPool.h
	qx/MyPath.h
	qx/MyRedirect.h
	qx/MyRedirect2.h
	qx/MyStream.h
	qx/MyStreamBase.h
	qx/MyString.h
	qx/MyStringList.h
	qx/QxReadWriteMutex.h
	qx/QxSemaphore.h
	qx/QxSingleton.h
	qx/QxTime.h
)

set(SRC_FILES_TP
	tiny_process/process.cpp
	tiny_process/process_win.cpp
	tiny_process/process.hpp
)

set(SRC_FILES
	action.cpp
	dump_util.cpp
	misc.cpp
	mmap.cpp
	action.h
	anlz.h
	avl_tree.h
	avl_tree_node.h
	avl_tree2.h
	avl-tree.h
	bostree.h
	circvec.h
	data_source.h
	defs.h
	dump_util.h
	front.h
	INIReader.h
	link.h
	misc.h
	mmap.h
	obj_id.h
	sbtree.h
	sbtree2.h
	table.h
	tree.h
)

add_library(shared
	${SRC_FILES}
	${SRC_FILES_QX}
	${SRC_FILES_TP}
)

source_group("" FILES ${SRC_FILES})
source_group("qx" FILES ${SRC_FILES_QX})
source_group("tiny_process" FILES ${SRC_FILES_TP})

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

list(APPEND INCLUDES
	"${PROJECT_SOURCE_DIR}/src"
	"qx/win32"
	)

target_include_directories(shared PUBLIC ${INCLUDES})

target_include_directories(shared INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})