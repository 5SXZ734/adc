
include ../../Parameters

LIB_SRC_FILES  = \
		cs.c \
		MCInst.c \
		MCInstrDesc.c \
		MCRegisterInfo.c \
		SStream.c \
		utils.c \
		$(addprefix arch/, \
			$(addprefix AArch64/, \
				AArch64BaseInfo.c \
				AArch64Disassembler.c \
				AArch64InstPrinter.c \
				AArch64Mapping.c \
				AArch64Module.c \
			) \
			$(addprefix ARM/, \
				ARMDisassembler.c \
				ARMInstPrinter.c \
				ARMMapping.c \
				ARMModule.c \
			)\
			$(addprefix EVM/, \
				EVMDisassembler.c \
				EVMInstPrinter.c \
				EVMMapping.c \
				EVMModule.c \
			) \
			$(addprefix M680X/, \
				M680XDisassembler.c \
				M680XInstPrinter.c \
				M680XModule.c \
			) \
			$(addprefix M68K/, \
				M68KDisassembler.c \
				M68KInstPrinter.c \
				M68KModule.c \
			) \
			$(addprefix Mips/, \
				MipsDisassembler.c \
				MipsInstPrinter.c \
				MipsMapping.c \
				MipsModule.c \
			) \
			$(addprefix PowerPC/, \
				PPCDisassembler.c \
				PPCInstPrinter.c \
				PPCMapping.c \
				PPCModule.c \
			) \
			$(addprefix Sparc/, \
				SparcDisassembler.c \
				SparcInstPrinter.c \
				SparcMapping.c \
				SparcModule.c \
			) \
			$(addprefix SystemZ/, \
				SystemZDisassembler.c \
				SystemZInstPrinter.c \
				SystemZMapping.c \
				SystemZMCTargetDesc.c \
				SystemZModule.c \
			) \
			$(addprefix X86/, \
				X86ATTInstPrinter.c \
				X86Disassembler.c \
				X86DisassemblerDecoder.c \
				X86IntelInstPrinter.c \
				X86Mapping.c \
				X86Module.c \
			) \
			$(addprefix XCore/, \
				XCoreDisassembler.c \
				XCoreInstPrinter.c \
				XCoreMapping.c \
				XCoreModule.c \
			) \
	)

LIB_CAPSTONE_OBJ := $(addprefix $(OBJDIR)/capstone/, $(LIB_SRC_FILES:.c=.o))
LIB_DEP_FILES := $(addprefix $(OBJDIR)/capstone/, $(LIB_SRC_FILES:.c=.d))

LIB_CAPSTONE_CFLAGS = $(CFLAGS) -I. -I./include -I./src/arch \
	-DCAPSTONE_X86_ATT_DISABLE_NO \
	-DCAPSTONE_DIET_NO \
	-DCAPSTONE_X86_REDUCE_NO \
	-DCAPSTONE_HAS_ARM \
	-DCAPSTONE_HAS_EVM \
	-DCAPSTONE_HAS_ARM64 \
	-DCAPSTONE_HAS_M68K \
	-DCAPSTONE_HAS_MIPS \
	-DCAPSTONE_HAS_POWERPC \
	-DCAPSTONE_HAS_SPARC \
	-DCAPSTONE_HAS_SYSZ \
	-DCAPSTONE_HAS_X86 \
	-DCAPSTONE_HAS_XCORE \
	-DCAPSTONE_USE_SYS_DYN_MEM \
	-DCAPSTONE_SHARED


LIB_CAPSTONE_LFLAGS = $(LINKER_FLAGS) -Wl,--no-undefined -shared

#echo:=$(shell echo "LIB_DEP_FILES: $(LIB_DEP_FILES)" >&2)


$(OBJDIR)/libcapstone.so : $(LIB_CAPSTONE_OBJ)
	@echo Linking $@ ...
	@$(CC) -o $@ $(LIB_CAPSTONE_OBJ) $(LIB_CAPSTONE_LFLAGS)
	@echo "Made $@"


$(LIB_CAPSTONE_OBJ) : $(OBJDIR)/capstone/%.o: src/%.c
	@echo Compiling $< ...
	@mkdir -p $(dir $@)
	@$(CC) -c $(DEP_FLAGS) $(LIB_CAPSTONE_CFLAGS) -o $@ $<



$(LIB_DEP_FILES):

include $(wildcard $(LIB_DEP_FILES))

