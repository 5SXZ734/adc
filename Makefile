
include ./Parameters


PROG_SRC_FILES  = $(addprefix src/, \
	loadui.cpp \
	startup.cpp \
	adc.cpp \
	$(addprefix db/, \
		anlzbin.cpp \
		clean.cpp \
		command.cpp \
		debug.cpp \
		dump_bin.cpp \
		dump_cache.cpp \
		dump_iter.cpp \
		dump_strucvar.cpp \
		dump_target.cpp \
		dump_visit.cpp \
		field.cpp \
		files.cpp \
		front_impl.cpp \
		globals.cpp \
		info_module.cpp \
		info_proj.cpp \
		info_proj_s.cpp \
		info_strucvar.cpp \
		info_types.cpp \
		locus.cpp \
		main.cpp \
		names.cpp \
		obj.cpp \
		print.cpp \
		proj.cpp \
		proj_cmd.cpp \
		save.cpp \
		script.cpp \
		symbol_map.cpp \
		type_code.cpp \
		type_module.cpp \
		type_obj.cpp \
		type_proc.cpp \
		type_seg.cpp \
		type_struc.cpp \
		type_strucvar.cpp \
		types.cpp \
		types_mgr.cpp \
		ui_bin_view.cpp \
		ui_main.cpp \
	) \
	$(addprefix dc/, \
		anal_branch.cpp \
		anal_data.cpp \
		anal_expr.cpp \
		anal_init.cpp \
		anal_local.cpp \
		anal_main.cpp \
		anal_pcode.cpp \
		anal_post.cpp \
		anal_ptr.cpp \
		anal_switch.cpp \
		cc.cpp \
		c_ppp.cpp \
		cleanx.cpp \
		compile.cpp \
		context.cpp \
		dc.cpp \
		dump.cpp \
		dump_base.cpp \
		dump_file.cpp \
		dump_func.cpp \
		dump_scan.cpp \
		expr.cpp \
		expr_cache.cpp \
		expr_dump.cpp \
		expr_ptr.cpp \
		expr_simpl.cpp \
		expr_term.cpp \
		expr_type.cpp \
		file_cmd.cpp \
		filesx.cpp \
		flow.cpp \
		front_dc.cpp \
		info_class.cpp \
		info_dc.cpp \
		info_dc_s.cpp \
		info_file.cpp \
		info_func.cpp \
		info_func_s.cpp \
		info_proto.cpp \
		mainx.cpp \
		make_type.cpp \
		mem.cpp \
		op.cpp \
		path.cpp \
		projx.cpp \
		reg.cpp \
		savex.cpp \
		stubs.cpp \
		sym_parse.cpp \
		type_funcdef.cpp \
		ui_main_ex.cpp \
		ui_src_view.cpp \
		) \
	)
	
PROTO_DIR = $(SRC)/front/proto
PROTO_FILES = $(wildcard $(PROTO_DIR)/*.hh) $(wildcard $(PROTO_DIR)/*.h) $(wildcard $(PROTO_DIR)/*.export)
PROTO_DIR_OUT = $(OBJDIR)/proto
PROTO_FILES_OUT = $(addprefix $(PROTO_DIR_OUT)/, $(notdir $(PROTO_FILES)))


PROG_OBJ_0 = $(PROG_SRC_FILES:.cpp=.o)
PROG_OBJ := $(addprefix $(OBJDIR)/, $(PROG_OBJ_0))
PROG_DEPS = $(addprefix $(OBJDIR)/, libshared.a)
PROG_LFLAGS := $(LINKER_FLAGS) \
	-Xlinker -Bstatic -L$(OBJDIR) -lshared \
	-Xlinker -Bdynamic -ldl -lpthread


PROG_FLAGS = $(CXXFLAGS) -Isrc -Isrc/shared

PROG_DEP_FILES := $(addprefix $(OBJDIR)/, $(PROG_SRC_FILES:.cpp=.d))


#echo:=$(shell echo "PROG_LFLAGS: $(PROG_LFLAGS)" >&2)


############################### RULES

.PHONY: all
all: $(addprefix $(OBJDIR)/, adc libfront.so libuiqt5.so)


$(OBJDIR)/adc : $(PROG_OBJ) $(PROG_DEPS) $(PROTO_FILES_OUT)
	@echo Linking $@ ...
	@$(CXX) -o $@ $(PROG_OBJ) $(PROG_LFLAGS)
	@echo "Made $@"



$(OBJDIR)/libfront.so : FORCE
	@$(MAKE) -s -C $(SRC)/front

$(OBJDIR)/libuiqt5.so : FORCE
	@$(MAKE) -s -C $(SRC)/uiqt5

$(OBJDIR)/libshared.a : FORCE
	@$(MAKE) -s -C $(SRC)/shared

FORCE:



$(PROG_OBJ) : $(OBJDIR)/%.o: %.cpp
	@echo Compiling $< ...
	@mkdir -p $(dir $@)
	@$(CXX) -c $(DEP_FLAGS) $(PROG_FLAGS) -o $@ $<

$(PROTO_FILES_OUT) : $(PROTO_DIR_OUT)/% : $(PROTO_DIR)/%
	@echo Copying $< ...
	@mkdir -p $(dir $@)
	@cp -f $< $@


.PHONY: clean
clean:
	@echo Cleaning up ...
	@rm -rf $(OBJDIR)



$(PROG_DEP_FILES):

include $(wildcard $(PROG_DEP_FILES))

