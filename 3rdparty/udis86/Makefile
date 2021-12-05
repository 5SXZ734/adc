
include ../../Parameters

LIB_SRC  = \
		decode.c \
		itab.c \
		syn.c \
		syn-att.c \
		syn-intel.c \
		udis86.c


LIB_OBJ := $(addprefix $(OBJDIR)/udis86/, $(LIB_SRC:.c=.o))
LIB_DEP_FILES = $(addprefix $(OBJDIR)/udis86/, $(LIB_SRC:.c=.d))

LIB_CFLAGS = $(CFLAGS) -DLIBUDIS86_EXPORTS
LIB_LFLAGS = $(LINKER_FLAGS) -Wl,--no-undefined -shared

#echo:=$(shell echo "CURDIR: $(CURDIR)" >&2)


$(OBJDIR)/libudis86.so : $(LIB_OBJ)
	@echo Linking $@ ...
	@$(CC) -o $@ $(LIB_OBJ) $(LIB_LFLAGS)
	@echo "Made $@"


$(LIB_OBJ) : $(OBJDIR)/udis86/%.o: libudis86/%.c
	@echo Compiling $< ...
	@mkdir -p $(dir $@)
	@$(CC) -c $(DEP_FLAGS) $(LIB_CFLAGS) -o $@ $<



$(LIB_DEP_FILES):

include $(wildcard $(LIB_DEP_FILES))

