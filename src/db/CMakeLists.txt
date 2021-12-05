set(SRC_FILES
	anlzbin.cpp
	clean.cpp
	command.cpp
	debug.cpp
	field.cpp
	files.cpp
	front_impl.cpp
	locus.cpp
	main.cpp
	mem.cpp
	names.cpp
	obj.cpp
	print.cpp
	proj.cpp
	proj_cmd.cpp
	save.cpp
	script.cpp
	symbol_map.cpp
	anlz.h
	anlzbin.h
	clean.h
	command.h
	data.h
	debug.h
	field.h
	files.h
	front_impl.h
	locus.h
	main.h
	mem.h
	names.h
	obj.h
	print.h
	probe.h
	proj.h
	proj_cmd.h
	save.h
	save_impl.h
	script.h
	symbol_map.h
)

set(SRC_FILES_INFO
	info_module.cpp
	info_proj.cpp
	info_proj_s.cpp
	info_strucvar.cpp
	info_types.cpp
	info_module.h
	info_proj.h
	info_strucvar.h
	info_types.h
)

set(SRC_FILES_TYPE
	type_code.cpp
	type_module.cpp
	type_obj.cpp
	type_proc.cpp
	type_seg.cpp
	type_struc.cpp
	type_strucvar.cpp
	types.cpp
	types_mgr.cpp
	type_alias.h
	type_code.h
	type_module.h
	type_obj.h
	type_proc.h
	type_proxy.h
	type_seg.h
	type_struc.h
	type_strucvar.h
	types.h
	types_mgr.h
)

set(SRC_FILES_DUMP
	dump_bin.cpp
	dump_cache.cpp
	dump_iter.cpp
	dump_strucvar.cpp
	dump_target.cpp
	dump_visit.cpp
	dump_bin.h
	dump_cache.h
	dump_iter.h
	dump_strucvar.h
	dump_target.h
	dump_visit.h
)

set(SRC_FILES_UI
	ui_bin_view.cpp
	ui_main.cpp
	ui_bin_view.h
	ui_exports.h
	ui_files.h
	ui_main.h
	ui_names.h
	ui_res_view.h
	ui_types.h
)

add_library(db)
target_sources(db PRIVATE ${SRC_FILES_DUMP} ${SRC_FILES_INFO} ${SRC_FILES_TYPE} ${SRC_FILES_UI} ${SRC_FILES})

source_group("src" FILES ${SRC_FILES})
source_group("src/dump" FILES ${SRC_FILES_DUMP})
source_group("src/info" FILES ${SRC_FILES_INFO})
source_group("src/type" FILES ${SRC_FILES_TYPE})
source_group("src/ui" FILES ${SRC_FILES_UI})

#add_definitions(-DNOMINMAX)

target_include_directories(db PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(db PRIVATE ${PROJECT_SOURCE_DIR}/src/shared)