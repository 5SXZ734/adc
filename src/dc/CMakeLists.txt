set(SRC_FILES
	c_pp.cpp
	cc.cpp
	clean_ex.cpp
	compile.cpp
	dc.cpp
	file_cmd.cpp
	files_ex.cpp
	flow.cpp
	front_dc.cpp
	front_impl_ex.cpp
	main_ex.cpp
	make_type.cpp
	mem_ex.cpp
	op.cpp
	path.cpp
	probe_ex.cpp
	proj_ex.cpp
	reg.cpp
	stubs.cpp
	sym_parse.cpp
	type_funcdef.cpp
	arglist.h
	c_pp.h
	cc.h
	clean_ex.h
	compile.h
	dc.h
	file_cmd.h
	files_ex.h
	flow.h
	front_dc.h
	front_impl_ex.h
	globs.h
	main_ex.h
	make_type.h
	mem_ex.h
	op.h
	path.h
	probe_ex.h
	proj_ex.h
	reg.h
	stubs.h
	sym_parse.h
	type_funcdef.h
	xref.h
)

set(SRC_FILES_ANAL
	anal_branch.cpp
	anal_data.cpp
	anal_expr.cpp
	anal_init.cpp
	anal_local.cpp
	anal_main.cpp
	anal_pcode.cpp
	anal_post.cpp
	anal_ptr.cpp
	anal_switch.cpp
	anal_branch.h
	anal_data.h
	anal_expr.h
	anal_init.h
	anal_local.h
	anal_main.h
	anal_pcode.h
	anal_post.h
	anal_ptr.h
	anal_switch.h
)

set(SRC_FILES_DUMP
	dump.cpp
	dump_base.cpp
	dump_file.cpp
	dump_func.cpp
	dump_scan.cpp
	dump.h
	dump_base.h
	dump_file.h
	dump_func.h
	dump_proto.h
	dump_scan.h
)

set(SRC_FILES_INFO
	info_class.cpp
	info_dc.cpp
	info_dc_s.cpp
	info_file.cpp
	info_func.cpp
	info_func_s.cpp
	info_proto.cpp
	info_class.h
	info_dc.h
	info_file.h
	info_func.h
	info_proto.h
)

set(SRC_FILES_EXPR
	expr.cpp
	expr_cache.cpp
	expr_dump.cpp
	expr_ptr.cpp
	expr_simpl.cpp
	expr_term.cpp
	expr_type.cpp
	expr.h
	expr_cache.h
	expr_dump.h
	expr_ptr.h
	expr_simpl.h
	expr_term.h
	expr_type.h
)

set(SRC_FILES_SAVE
	save_ex.cpp
	save_ex.h
	savex_impl.h
	savex_move.h
)

set(SRC_FILES_UI
	ui_main_ex.cpp
	ui_src_view.cpp
	ui_expr_view.h
	ui_files_ex.h
	ui_main_ex.h
	ui_names_ex.h
	ui_src_view.h
	ui_stubs.h
	ui_templ.h
)


add_library(dc)
target_sources(dc PRIVATE ${SRC_FILES} ${SRC_FILES_ANAL} ${SRC_FILES_DUMP} ${SRC_FILES_INFO} ${SRC_FILES_EXPR} ${SRC_FILES_SAVE} ${SRC_FILES_UI})

source_group("src/" FILES ${SRC_FILES})
source_group("src/anal" FILES ${SRC_FILES_ANAL})
source_group("src/dump" FILES ${SRC_FILES_DUMP})
source_group("src/info" FILES ${SRC_FILES_INFO})
source_group("src/expr" FILES ${SRC_FILES_EXPR})
source_group("src/save" FILES ${SRC_FILES_SAVE})
source_group("src/ui" FILES ${SRC_FILES_UI})


target_include_directories(dc PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(dc PUBLIC ${PROJECT_SOURCE_DIR}/src/shared)

