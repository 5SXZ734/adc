
set(SHARED_SRC
	src/shared/action.cpp
	src/shared/dump_util.cpp
	src/shared/misc.cpp
	src/shared/mmap.cpp
	src/shared/action.h
	src/shared/anlz.h
	src/shared/avl_tree.h
	src/shared/avl_tree_node.h
	src/shared/avl_tree2.h
	src/shared/avl-tree.h
	src/shared/bostree.h
	src/shared/circvec.h
	src/shared/data_source.h
	src/shared/defs.h
	src/shared/dump_util.h
	src/shared/front.h
	src/shared/INIReader.h
	src/shared/link.h
	src/shared/misc.h
	src/shared/mmap.h
	src/shared/obj_id.h
	src/shared/sbtree.h
	src/shared/sbtree2.h
	src/shared/table.h
	src/shared/tree.h
)

source_group("shared" FILES ${SHARED_SRC})

#################################################


set(DB_0_SRC
	src/db/anlzbin.cpp
	src/db/clean.cpp
	src/db/command.cpp
	src/db/debug.cpp
	src/db/field.cpp
	src/db/files.cpp
	src/db/front_impl.cpp
	src/db/locus.cpp
	src/db/main.cpp
	src/db/mem.cpp
	src/db/names.cpp
	src/db/obj.cpp
	src/db/print.cpp
	src/db/proj.cpp
	src/db/proj_cmd.cpp
	src/db/save.cpp
	src/db/script.cpp
	src/db/symbol_map.cpp
	src/db/anlz.h
	src/db/anlzbin.h
	src/db/clean.h
	src/db/command.h
	src/db/data.h
	src/db/debug.h
	src/db/field.h
	src/db/files.h
	src/db/front_impl.h
	src/db/locus.h
	src/db/main.h
	src/db/mem.h
	src/db/names.h
	src/db/obj.h
	src/db/print.h
	src/db/probe.h
	src/db/proj.h
	src/db/proj_cmd.h
	src/db/save.h
	src/db/save_impl.h
	src/db/script.h
	src/db/symbol_map.h
)

set(DB_INFO_SRC
	src/db/info_module.cpp
	src/db/info_proj.cpp
	src/db/info_proj_s.cpp
	src/db/info_strucvar.cpp
	src/db/info_types.cpp
	src/db/info_module.h
	src/db/info_proj.h
	src/db/info_strucvar.h
	src/db/info_types.h
)

set(DB_TYPE_SRC
	src/db/type_code.cpp
	src/db/type_module.cpp
	src/db/type_obj.cpp
	src/db/type_proc.cpp
	src/db/type_seg.cpp
	src/db/type_struc.cpp
	src/db/type_strucvar.cpp
	src/db/types.cpp
	src/db/types_mgr.cpp
	src/db/type_alias.h
	src/db/type_code.h
	src/db/type_module.h
	src/db/type_obj.h
	src/db/type_proc.h
	src/db/type_proxy.h
	src/db/type_seg.h
	src/db/type_struc.h
	src/db/type_strucvar.h
	src/db/types.h
	src/db/types_mgr.h
)

set(DB_DUMP_SRC
	src/db/dump_bin.cpp
	src/db/dump_cache.cpp
	src/db/dump_iter.cpp
	src/db/dump_strucvar.cpp
	src/db/dump_target.cpp
	src/db/dump_visit.cpp
	src/db/dump_bin.h
	src/db/dump_cache.h
	src/db/dump_iter.h
	src/db/dump_strucvar.h
	src/db/dump_target.h
	src/db/dump_visit.h
)

set(DB_UI_SRC
	src/db/ui_bin_view.cpp
	src/db/ui_main.cpp
	src/db/ui_bin_view.h
	src/db/ui_exports.h
	src/db/ui_files.h
	src/db/ui_main.h
	src/db/ui_names.h
	src/db/ui_res_view.h
	src/db/ui_types.h
)

set(DB_SRC ${DB_DUMP_SRC} ${DB_INFO_SRC} ${DB_TYPE_SRC} ${DB_UI_SRC} ${DB_0_SRC})

source_group("src" FILES ${DB_0_SRC})
source_group("src/dump" FILES ${DB_DUMP_SRC})
source_group("src/info" FILES ${DB_INFO_SRC})
source_group("src/type" FILES ${DB_TYPE_SRC})
source_group("src/ui" FILES ${DB_UI_SRC})

#add_definitions(-DNOMINMAX)

####################################################################


set(DC_0_SRC
	src/dc/c_pp.cpp
	src/dc/cc.cpp
	src/dc/clean_ex.cpp
	src/dc/compile.cpp
	src/dc/dc.cpp
	src/dc/file_cmd.cpp
	src/dc/files_ex.cpp
	src/dc/flow.cpp
	src/dc/front_dc.cpp
	src/dc/front_impl_ex.cpp
	src/dc/main_ex.cpp
	src/dc/make_type.cpp
	src/dc/mem_ex.cpp
	src/dc/op.cpp
	src/dc/path.cpp
	src/dc/probe_ex.cpp
	src/dc/proj_ex.cpp
	src/dc/reg.cpp
	src/dc/stubs.cpp
	src/dc/sym_parse.cpp
	src/dc/type_funcdef.cpp
	src/dc/arglist.h
	src/dc/c_pp.h
	src/dc/cc.h
	src/dc/clean_ex.h
	src/dc/compile.h
	src/dc/dc.h
	src/dc/file_cmd.h
	src/dc/files_ex.h
	src/dc/flow.h
	src/dc/front_dc.h
	src/dc/front_impl_ex.h
	src/dc/globs.h
	src/dc/main_ex.h
	src/dc/make_type.h
	src/dc/mem_ex.h
	src/dc/op.h
	src/dc/path.h
	src/dc/probe_ex.h
	src/dc/proj_ex.h
	src/dc/reg.h
	src/dc/stubs.h
	src/dc/sym_parse.h
	src/dc/type_funcdef.h
	src/dc/xref.h
)

set(DC_ANAL_SRC
	src/dc/anal_branch.cpp
	src/dc/anal_data.cpp
	src/dc/anal_expr.cpp
	src/dc/anal_init.cpp
	src/dc/anal_local.cpp
	src/dc/anal_main.cpp
	src/dc/anal_pcode.cpp
	src/dc/anal_post.cpp
	src/dc/anal_ptr.cpp
	src/dc/anal_switch.cpp
	src/dc/anal_branch.h
	src/dc/anal_data.h
	src/dc/anal_expr.h
	src/dc/anal_init.h
	src/dc/anal_local.h
	src/dc/anal_main.h
	src/dc/anal_pcode.h
	src/dc/anal_post.h
	src/dc/anal_ptr.h
	src/dc/anal_switch.h
)

set(DC_DUMP_SRC
	src/dc/dump.cpp
	src/dc/dump_base.cpp
	src/dc/dump_file.cpp
	src/dc/dump_func.cpp
	src/dc/dump_scan.cpp
	src/dc/dump.h
	src/dc/dump_base.h
	src/dc/dump_file.h
	src/dc/dump_func.h
	src/dc/dump_proto.h
	src/dc/dump_scan.h
)

set(DC_INFO_SRC
	src/dc/info_class.cpp
	src/dc/info_dc.cpp
	src/dc/info_dc_s.cpp
	src/dc/info_file.cpp
	src/dc/info_func.cpp
	src/dc/info_func_s.cpp
	src/dc/info_proto.cpp
	src/dc/info_class.h
	src/dc/info_dc.h
	src/dc/info_file.h
	src/dc/info_func.h
	src/dc/info_proto.h
)

set(DC_EXPR_SRC
	src/dc/expr.cpp
	src/dc/expr_cache.cpp
	src/dc/expr_dump.cpp
	src/dc/expr_ptr.cpp
	src/dc/expr_simpl.cpp
	src/dc/expr_term.cpp
	src/dc/expr_type.cpp
	src/dc/expr.h
	src/dc/expr_cache.h
	src/dc/expr_dump.h
	src/dc/expr_ptr.h
	src/dc/expr_simpl.h
	src/dc/expr_term.h
	src/dc/expr_type.h
)

set(DC_SAVE_SRC
	src/dc/save_ex.cpp
	src/dc/save_ex.h
	src/dc/savex_impl.h
	src/dc/savex_move.h
)

set(DC_UI_SRC
	src/dc/ui_main_ex.cpp
	src/dc/ui_src_view.cpp
	src/dc/ui_expr_view.h
	src/dc/ui_files_ex.h
	src/dc/ui_main_ex.h
	src/dc/ui_names_ex.h
	src/dc/ui_src_view.h
	src/dc/ui_stubs.h
	src/dc/ui_templ.h
)


set(DC_SRC ${DC_0_SRC} ${DC_ANAL_SRC} ${DC_DUMP_SRC} ${DC_INFO_SRC} ${DC_EXPR_SRC} ${DC_SAVE_SRC} ${DC_UI_SRC})

source_group("src/" FILES ${DC_0_SRC})
source_group("src/anal" FILES ${DC_ANAL_SRC})
source_group("src/dump" FILES ${DC_DUMP_SRC})
source_group("src/info" FILES ${DC_INFO_SRC})
source_group("src/expr" FILES ${DC_EXPR_SRC})
source_group("src/save" FILES ${DC_SAVE_SRC})
source_group("src/ui" FILES ${DC_UI_SRC})


###########################################################

set(UIQ5_UIC
	src/uiqt5/ADCProtoDlgBase.ui
	)


set(UIQ5_0_SRC
	src/uiqt5/ADBDataWin.cpp
	src/uiqt5/ADBFilesWin.cpp
	src/uiqt5/ADBInterWin.cpp
	src/uiqt5/ADBMainWin.cpp
	src/uiqt5/ADCApp.cpp
	src/uiqt5/ADCBinWin.cpp
	src/uiqt5/ADCBlocksWin.cpp
	src/uiqt5/ADCCodeWin.cpp
	src/uiqt5/ADCConfigDlg.cpp
	src/uiqt5/ADCCutListWin.cpp
	src/uiqt5/ADCExprWin.cpp
	src/uiqt5/ADCLineEdit.cpp
	src/uiqt5/ADCMainWin.cpp
	src/uiqt5/ADCNamesWin.cpp
	src/uiqt5/ADCOutputWin.cpp
	src/uiqt5/ADCProtoDlg.cpp
	src/uiqt5/ADCResourceWin.cpp
	src/uiqt5/ADCSourceWin.cpp
	src/uiqt5/ADCSplitWin.cpp
	src/uiqt5/ADCStubsWin.cpp
	src/uiqt5/ADCTableView.cpp
	src/uiqt5/ADCTabsWin.cpp
	src/uiqt5/ADCTasksWin.cpp
	src/uiqt5/ADCTemplWin.cpp
	src/uiqt5/ADCTextView.cpp
	src/uiqt5/ADCTypesWin.cpp
	src/uiqt5/ADCUserPrefs.cpp
	src/uiqt5/ADCUtils.cpp
	src/uiqt5/colors.cpp
	src/uiqt5/ADBDataWin.h
	src/uiqt5/ADBFilesWin.h
	src/uiqt5/ADBInterWin.h
	src/uiqt5/ADBMainWin.h
	src/uiqt5/ADCApp.h
	src/uiqt5/ADCBinWin.h
	src/uiqt5/ADCBlocksWin.h
	src/uiqt5/ADCCell.h
	src/uiqt5/ADCCodeWin.h
	src/uiqt5/ADCConfigDlg.h
	src/uiqt5/ADCCutListWin.h
	src/uiqt5/ADCExprWin.h
	src/uiqt5/ADCLineEdit.h
	src/uiqt5/ADCMainWin.h
	src/uiqt5/ADCNamesWin.h
	src/uiqt5/ADCOutputWin.h
	src/uiqt5/ADCProtoDlg.h
	src/uiqt5/ADCResourceWin.h
	src/uiqt5/ADCSourceWin.h
	src/uiqt5/ADCSplitWin.h
	src/uiqt5/ADCStream.h
	src/uiqt5/ADCStubsWin.h
	src/uiqt5/ADCTableView.h
	src/uiqt5/ADCTabsWin.h
	src/uiqt5/ADCTasksWin.h
	src/uiqt5/ADCTemplWin.h
	src/uiqt5/ADCTextView.h
	src/uiqt5/ADCTypesWin.h
	src/uiqt5/ADCUserPrefs.h
	src/uiqt5/ADCUtils.h
	src/uiqt5/colors.h
)

set(UIQ5_QSIL_SRC
	src/uiqt5/qsil/toolpalette.cpp
	src/uiqt5/qsil/xmru.cpp
	src/uiqt5/qsil/xresmgr.cpp
	src/uiqt5/qsil/xresource.cpp
	src/uiqt5/qsil/xshared.cpp
	src/uiqt5/qsil/xwindowsettings.cpp
	src/uiqt5/qsil/toolpalette.h
	src/uiqt5/qsil/xmru.h
	src/uiqt5/qsil/xresmgr.h
	src/uiqt5/qsil/xresource.h
	src/uiqt5/qsil/xshared.h
	src/uiqt5/qsil/xwindowsettings.h
)

set(UIQ5_SX_SRC
	src/uiqt5/sx/SxChildWindow.cpp
	src/uiqt5/sx/SxCommandLine.cpp
	src/uiqt5/sx/SxCommandWin.cpp
	src/uiqt5/sx/SxDockBar.cpp
	src/uiqt5/sx/SxDockWindow.cpp
	src/uiqt5/sx/SxDocument.cpp
	src/uiqt5/sx/SxFindTextDlg.cpp
	src/uiqt5/sx/SxGUI.cpp
	src/uiqt5/sx/SxMainWindow.cpp
	src/uiqt5/sx/SxOutputWin.cpp
	src/uiqt5/sx/SxSignalMultiplexer.cpp
	src/uiqt5/sx/SxTabWidget.cpp
	src/uiqt5/sx/SxViewMgrMDI.cpp
	src/uiqt5/sx/SxViewMgrTabs.cpp
	src/uiqt5/sx/MAINWINBASE.h
	src/uiqt5/sx/SxChildWindow.h
	src/uiqt5/sx/SxCommandLine.h
	src/uiqt5/sx/SxCommandWin.h
	src/uiqt5/sx/SxDockBar.h
	src/uiqt5/sx/SxDockWindow.h
	src/uiqt5/sx/SxDocument.h
	src/uiqt5/sx/SxFindTextDlg.h
	src/uiqt5/sx/SxGUI.h
	src/uiqt5/sx/SxMainWindow.h
	src/uiqt5/sx/SxOutputWin.h
	src/uiqt5/sx/SxSignalMultiplexer.h
	src/uiqt5/sx/SxTabWidget.h
	src/uiqt5/sx/SxViewMgrMDI.h
	src/uiqt5/sx/SxViewMgrTabs.h
)


set(UIQ5_SRC ${UIQ5_0_SRC} ${UIQ5_QSIL_SRC} ${UIQ5_SX_SRC})

foreach(i IN LISTS UIQ5_SRC)
	get_filename_component(_d "${i}" DIRECTORY)
	source_group(${_d} FILES ${i})
	if(${i} MATCHES ".h$")
		list(APPEND UIQ5_MOC ${i})
	endif()
endforeach()

set(UIQ5_RCC
	src/uiqt5/images/images.qrc
)

#source_group("" FILES ${UIQ5_QRC})
source_group("src" FILES ${UIQ5_0_SRC})
source_group("src/QSil" FILES ${UIQ5_QSIL_SRC})
source_group("src/Sx" FILES ${UIQ5_SX_SRC})

###########################################################


set(FRONT_0_SRC
	src/front/front_main.cpp
	src/front/shared.cpp
	src/front/front_main.h
	src/front/shared.h
	src/front/wintypes.h
)

set(FRONT_MISC_SRC
	src/front/format.adc.cpp
	src/front/format.ico.cpp
	src/front/format.tr0.cpp
	src/front/format.misc.h
)

set(FRONT_X86_SRC
	src/front/cpu16.cpp
	src/front/cpu32.cpp
	src/front/cpu64.cpp
	src/front/front_IA.cpp
	src/front/x86_dump.cpp
	src/front/x86_IR.cpp
	src/front/front_IA.h
	src/front/x86.h
	src/front/x86_dump.h
	src/front/x86_IR.h
	src/front/fpu32.inl
	src/front/fpu32.1.inl
	src/front/jumps16.inl
	src/front/jumps32.inl
	src/front/sets32.inl
)

set(FRONT_JAVA_SRC
	src/front/format.java.cpp
	src/front/format.java.h
)

set(FRONT_PDB_SRC
	src/front/decode.PDB.cpp
	src/front/format.pdb_D.cpp
	src/front/format.pdb_S.cpp
	src/front/format.pdb20.cpp
	src/front/format.pdb70.cpp
	src/front/decode.PDB.h
	src/front/format.pdb.h
	src/front/PDB.h
)

set(FRONT_PE_SRC
	src/front/decode.COFF.h
	src/front/decode.pe.cpp
	src/front/format.exe.cpp
	src/front/format.pe.cpp
	src/front/format.pe.NET.cpp
	src/front/format.pe_D.cpp
	src/front/format.pe_S.cpp
	src/front/decode.pe.h
	src/front/decode.RTTI.h
	src/front/format.exe.h
	src/front/format.pe.h
	src/front/PE.h
)

set(FRONT_ELF_SRC
	src/front/format.elf.cpp
	src/front/format.elf_S.cpp
	src/front/ELF.h
	src/front/format.elf.h
)

set(FRONT_DWARF_SRC
	src/front/decode.DWARF.cpp
	src/front/format.dwarf.cpp
	src/front/decode.DWARF.h
	src/front/DWARF.h
	src/front/format.dwarf.h
)

set(FRONT_CODE_SRC
	src/front/code.capstone.cpp
	src/front/code.udis86.cpp
)



list(APPEND FRONT_SRC 
	${FRONT_0_SRC} ${FRONT_MISC_SRC} 
	${FRONT_X86_SRC} ${FRONT_JAVA_SRC} 
	${FRONT_PDB_SRC} ${FRONT_CODE_SRC}
	${FRONT_DWARF_SRC} ${FRONT_ELF_SRC}
	${FRONT_PE_SRC}
	)

#list(TRANSFORM FRONT_SRC PREPEND "src/front/")

source_group("" FILES ${FRONT_0_SRC})
source_group("misc" FILES ${FRONT_MISC_SRC})
source_group("X86" FILES ${FRONT_X86_SRC})
source_group("JAVA" FILES ${FRONT_JAVA_SRC})
source_group("PDB" FILES ${FRONT_PDB_SRC})
source_group("CODE" FILES ${FRONT_CODE_SRC})
source_group("DWARF" FILES ${FRONT_DWARF_SRC})
source_group("ELF" FILES ${FRONT_ELF_SRC})
source_group("PE" FILES ${FRONT_PE_SRC})


###########################################################

set(ADC_SRC
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

set(ADC_H
	src/interface/IADCFront.h
	src/interface/IADCGui.h
	src/interface/IADCMain.h
)

source_group("src" FILES ${ADC_SRC})
source_group("interface" FILES ${ADC_H})
