# ---------------------------------------------------------------------------
# Helper: collect files into a list and assign a source_group in one shot
#   OUT_VAR  – name of the variable to append to (e.g. DB_SRC)
#   GROUP    – source_group name (e.g. "src/dump")
#   BASEDIR  – directory prefix on disk (e.g. "src/db")
#   ARGN     – list of *filenames* (no prefix)
# ---------------------------------------------------------------------------
function(add_src_block OUT_VAR GROUP BASEDIR)
    set(tmp "${${OUT_VAR}}")
    set(_files)
    foreach(f IN LISTS ARGN)
        list(APPEND _files "${BASEDIR}/${f}")
    endforeach()
    list(APPEND tmp ${_files})
    set(${OUT_VAR} "${tmp}" PARENT_SCOPE)
    # IMPORTANT: quote GROUP so "" is preserved as an actual empty name
    source_group("${GROUP}" FILES ${_files})
endfunction()




function(add_uiq5_block OUT_VAR GROUP BASEDIR)
    # OUT_VAR  – variable to append to (e.g. UIQ5_SRC)
    # GROUP    – source_group name (may be "")
    # BASEDIR  – base directory on disk (e.g. "src/uiqt5")

    # Start from existing list in parent
    set(tmp "${${OUT_VAR}}")

    # Build full paths
    set(_files)
    foreach(f IN LISTS ARGN)
        list(APPEND _files "${BASEDIR}/${f}")
    endforeach()

    # Append to OUT_VAR
    list(APPEND tmp ${_files})
    set(${OUT_VAR} "${tmp}" PARENT_SCOPE)

    # Maintain UIQ5_MOC
    set(_moc "${${OUT_VAR}_MOC}")
    foreach(f IN LISTS _files)
        if("${f}" MATCHES "\\.h$")
            list(APPEND _moc "${f}")
        endif()
    endforeach()
    set(${OUT_VAR}_MOC "${_moc}" PARENT_SCOPE)

    # Do source_group exactly like add_src_block (including GROUP = "")
    source_group("${GROUP}" FILES ${_files})
endfunction()






# ========================= SHARED ==========================================
set(SHARED_SRC)
add_src_block(SHARED_SRC "" "src/shared"
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

# ========================= DB ==============================================
set(DB_SRC)

add_src_block(DB_SRC "" "src/db"
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

add_src_block(DB_SRC "info" "src/db"
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

add_src_block(DB_SRC "type" "src/db"
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

add_src_block(DB_SRC "dump" "src/db"
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

add_src_block(DB_SRC "ui" "src/db"
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

# ========================= DC ==============================================
set(DC_SRC)

add_src_block(DC_SRC "" "src/dc"
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

add_src_block(DC_SRC "anal" "src/dc"
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

add_src_block(DC_SRC "dump" "src/dc"
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

add_src_block(DC_SRC "info" "src/dc"
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

add_src_block(DC_SRC "expr" "src/dc"
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

add_src_block(DC_SRC "save" "src/dc"
    save_ex.cpp
    save_ex.h
    savex_impl.h
    savex_move.h
)

add_src_block(DC_SRC "ui" "src/dc"
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

# ========================= UIQT5 ===========================================
set(UIQ5_UIC
    src/uiqt5/ADCProtoDlgBase.ui
)

# aggregate lists
set(UIQ5_SRC)
set(UIQ5_SRC_MOC)

# main uiqt5 (src/uiqt5)
add_uiq5_block(UIQ5_SRC "" "src/uiqt5"
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

# QSil (src/uiqt5/qsil)
add_uiq5_block(UIQ5_SRC "qsil" "src/uiqt5/qsil"
    toolpalette.cpp
    xmru.cpp
    xresmgr.cpp
    xresource.cpp
    xshared.cpp
    xwindowsettings.cpp
    toolpalette.h
    xmru.h
    xresmgr.h
    xresource.h
    xshared.h
    xwindowsettings.h
)

# Sx (src/uiqt5/sx)
add_uiq5_block(UIQ5_SRC "sx" "src/uiqt5/sx"
    SxChildWindow.cpp
    SxCommandLine.cpp
    SxCommandWin.cpp
    SxDockBar.cpp
    SxDockWindow.cpp
    SxDocument.cpp
    SxFindTextDlg.cpp
    SxGUI.cpp
    SxMainWindow.cpp
    SxOutputWin.cpp
    SxSignalMultiplexer.cpp
    SxTabWidget.cpp
    SxViewMgrMDI.cpp
    SxViewMgrTabs.cpp
    MAINWINBASE.h
    SxChildWindow.h
    SxCommandLine.h
    SxCommandWin.h
    SxDockBar.h
    SxDockWindow.h
    SxDocument.h
    SxFindTextDlg.h
    SxGUI.h
    SxMainWindow.h
    SxOutputWin.h
    SxSignalMultiplexer.h
    SxTabWidget.h
    SxViewMgrMDI.h
    SxViewMgrTabs.h
)

set(UIQ5_RCC
    src/uiqt5/images/images.qrc
)



# ========================= FRONT ===========================================
set(FRONT_SRC)

add_src_block(FRONT_SRC "" "src/front"   # root group
    front_main.cpp
    shared.cpp
    front_main.h
    shared.h
    wintypes.h
)

add_src_block(FRONT_SRC "misc" "src/front"
    format.adc.cpp
    format.ico.cpp
    format.tr0.cpp
    format.misc.h
)

add_src_block(FRONT_SRC "X86" "src/front"
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

add_src_block(FRONT_SRC "JAVA" "src/front"
    format.java.cpp
    format.java.h
)

add_src_block(FRONT_SRC "PDB" "src/front"
    decode.PDB.cpp
    format.pdb_D.cpp
    format.pdb_S.cpp
    format.pdb20.cpp
    format.pdb70.cpp
    decode.PDB.h
    format.pdb.h
    PDB.h
)

add_src_block(FRONT_SRC "PE" "src/front"
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

add_src_block(FRONT_SRC "ELF" "src/front"
    format.elf.cpp
    format.elf_S.cpp
    ELF.h
    format.elf.h
)

add_src_block(FRONT_SRC "DWARF" "src/front"
    decode.DWARF.cpp
    format.dwarf.cpp
    decode.DWARF.h
    DWARF.h
    format.dwarf.h
)

add_src_block(FRONT_SRC "CODE" "src/front"
    code.capstone.cpp
    code.udis86.cpp
)

# ========================= ADC =============================================
set(ADC_SRC)
add_src_block(ADC_SRC "" "src"
    adc.rc
    adc.cpp
    loadui.cpp
    startup.cpp
    WinMain.cpp
    adb.h
    adc.h
    config.h
    loadui.h
    prefix.h
    resource.h
    startup.h
    targetver.h
)

set(ADC_H)
add_src_block(ADC_H "interface" "src/interface"
    IADCFront.h
    IADCGui.h
    IADCMain.h
)

