set(SRC_FILES
	ADCProtoDlgBase.ui

	ADBDataWin.cpp
	ADBFilesWin.cpp
	ADBInterWin.cpp
	ADBMainWin.cpp
	ADCApp.cpp
	ADCBinWin.cpp
	ADCBlocksWin.cpp
	ADCCodeWin.cpp
	ADCConfigDlg.cpp
	ADCCutListWin.cpp
	ADCExprWin.cpp
	ADCLineEdit.cpp
	ADCMainWin.cpp
	ADCNamesWin.cpp
	ADCOutputWin.cpp
	ADCProtoDlg.cpp
	ADCResourceWin.cpp
	ADCSourceWin.cpp
	ADCSplitWin.cpp
	ADCStubsWin.cpp
	ADCTableView.cpp
	ADCTabsWin.cpp
	ADCTasksWin.cpp
	ADCTemplWin.cpp
	ADCTextView.cpp
	ADCTypesWin.cpp
	ADCUserPrefs.cpp
	ADCUtils.cpp
	colors.cpp
	ADBDataWin.h
	ADBFilesWin.h
	ADBInterWin.h
	ADBMainWin.h
	ADCApp.h
	ADCBinWin.h
	ADCBlocksWin.h
	ADCCell.h
	ADCCodeWin.h
	ADCConfigDlg.h
	ADCCutListWin.h
	ADCExprWin.h
	ADCLineEdit.h
	ADCMainWin.h
	ADCNamesWin.h
	ADCOutputWin.h
	ADCProtoDlg.h
	ADCResourceWin.h
	ADCSourceWin.h
	ADCSplitWin.h
	ADCStream.h
	ADCStubsWin.h
	ADCTableView.h
	ADCTabsWin.h
	ADCTasksWin.h
	ADCTemplWin.h
	ADCTextView.h
	ADCTypesWin.h
	ADCUserPrefs.h
	ADCUtils.h
	colors.h
)

set(SRC_FILES_QSIL
	qsil/toolpalette.cpp
	qsil/xmru.cpp
	qsil/xresmgr.cpp
	qsil/xresource.cpp
	qsil/xshared.cpp
	qsil/xwindowsettings.cpp
	qsil/toolpalette.h
	qsil/xmru.h
	qsil/xresmgr.h
	qsil/xresource.h
	qsil/xshared.h
	qsil/xwindowsettings.h
)

set(SRC_FILES_SX
	sx/SxChildWindow.cpp
	sx/SxCommandLine.cpp
	sx/SxCommandWin.cpp
	sx/SxDockBar.cpp
	sx/SxDockWindow.cpp
	sx/SxDocument.cpp
	sx/SxFindTextDlg.cpp
	sx/SxGUI.cpp
	sx/SxMainWindow.cpp
	sx/SxOutputWin.cpp
	sx/SxSignalMultiplexer.cpp
	sx/SxTabWidget.cpp
	sx/SxViewMgrMDI.cpp
	sx/SxViewMgrTabs.cpp
	sx/MAINWINBASE.h
	sx/SxChildWindow.h
	sx/SxCommandLine.h
	sx/SxCommandWin.h
	sx/SxDockBar.h
	sx/SxDockWindow.h
	sx/SxDocument.h
	sx/SxFindTextDlg.h
	sx/SxGUI.h
	sx/SxMainWindow.h
	sx/SxOutputWin.h
	sx/SxSignalMultiplexer.h
	sx/SxTabWidget.h
	sx/SxViewMgrMDI.h
	sx/SxViewMgrTabs.h
)

set(SRC_FILES_QRC
	images/images.qrc
)


#message("test:" ${CMAKE_PREFIX_PATH})

set(Qt5_DIR "$ENV{QT5DIR}/v142/win32/lib/cmake/Qt5")
#set(Qt5_DIR $ENV{QT_DIR}lib/cmake/Qt5)

#find_package(Qt5Widgets REQUIRED)
#find_package(Qt5Gui REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)


#get_target_property(QtCore_location Qt5::Core LOCATION)
#message(" ***" ${QtCore_location})

#foreach(plugin ${Qt5Network_PLUGINS})
#  get_target_property(_loc ${plugin} LOCATION)
#  message(" *** Plugin ${plugin} is at location ${_loc}")
#endforeach()

#qt5_add_resources(SOURCES images/images.qrc)

add_library(uiqt5 MODULE)
target_sources(uiqt5 PRIVATE ${SRC_FILES_QRC} ${SRC_FILES} ${SRC_FILES_QSIL} ${SRC_FILES_SX})

source_group("" FILES ${SRC_FILES_QRC})
source_group("src" FILES ${SRC_FILES})
source_group("src/QSil" FILES ${SRC_FILES_QSIL})
source_group("src/Sx" FILES ${SRC_FILES_SX})

target_include_directories(uiqt5 PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(uiqt5 PRIVATE ${PROJECT_SOURCE_DIR}/src/shared)

target_link_libraries(uiqt5 Qt5::Widgets shared)

set(QT_PLUGINS_DIR $<TARGET_FILE_DIR:uiqt5>/platforms)

set(QT_DLLS
	$<TARGET_FILE:Qt5::Widgets>
	$<TARGET_FILE:Qt5::Gui>
	$<TARGET_FILE:Qt5::Core>
)

set(QT_PLUGINS
	$<TARGET_FILE:Qt5::QWindowsIntegrationPlugin>
)

add_custom_command (TARGET uiqt5 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${QT_DLLS} $<TARGET_FILE_DIR:uiqt5>
)

add_custom_command (TARGET uiqt5 POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${QT_PLUGINS_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_PLUGINS} ${QT_PLUGINS_DIR}
)

install(TARGETS uiqt5 DESTINATION ${INSTALL_SUBDIR})

install(FILES $<TARGET_FILE:Qt5::Widgets> DESTINATION ${INSTALL_SUBDIR})
install(FILES $<TARGET_FILE:Qt5::Gui> DESTINATION ${INSTALL_SUBDIR})
install(FILES $<TARGET_FILE:Qt5::Core> DESTINATION ${INSTALL_SUBDIR})

install(FILES ${QT_PLUGINS} DESTINATION ${INSTALL_SUBDIR}/platforms)



