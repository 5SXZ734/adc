
include ../../Parameters

LIB_SRC  = \
		demanglers.cpp \
		$(addprefix src/, \
			undname.c \
			$(addprefix libiberty/, \
				cp-demangle.c \
				cplus-dem.c \
				d-demangle.c \
				rust-demangle.c \
				safe-ctype.c \
				xexit.c \
				xmalloc.c \
				xmemdup.c \
				xstrdup.c \
			) \
			$(addprefix llvm/Demangle/, \
				Demangle.cpp \
				ItaniumDemangle.cpp \
				MicrosoftDemangle.cpp \
				MicrosoftDemangleNodes.cpp \
			) \
		)


LIB_SRC_C = $(filter %.c, $(LIB_SRC))
LIB_SRC_CPP = $(filter %.cpp, $(LIB_SRC))

LIB_OBJDIR := $(OBJDIR)/demanglers

LIB_OBJ_C_ = $(LIB_SRC_C:.c=.o)
LIB_OBJ_CPP_ = $(LIB_SRC_CPP:.cpp=.o)
LIB_OBJ_C := $(addprefix $(LIB_OBJDIR)/, $(LIB_OBJ_C_))
LIB_OBJ_CPP := $(addprefix $(LIB_OBJDIR)/, $(LIB_OBJ_CPP_))
LIB_DEP_C := $(addprefix $(LIB_OBJDIR)/, $(LIB_SRC_C:.c=.d))
LIB_DEP_CPP := $(addprefix $(LIB_OBJDIR)/, $(LIB_SRC_CPP:.cpp=.d))

LIB_OBJ = $(LIB_OBJ_C) $(LIB_OBJ_CPP)


LIB_CFLAGS = $(CFLAGS) -DHAVE_STDLIB_H -DHAVE_STRING_H -DDEMANGLERS_SHARED
LIB_CXXFLAGS = $(LIB_CFLAGS) -I./src
LIB_LFLAGS = $(LINKER_FLAGS) -Wl,--no-undefined -shared


#echo:=$(shell echo "LIB_DEP_C: $(LIB_DEP_C)" >&2)
#echo:=$(shell echo "LIB_DEP_CPP: $(LIB_DEP_CPP)" >&2)


$(OBJDIR)/libdemanglers.so : $(LIB_OBJ)
	@echo Linking $@ ...
	@$(CXX) -o $@ $(LIB_OBJ) $(LIB_LFLAGS)
	@echo "Made $@"


$(LIB_OBJ_C) : $(LIB_OBJDIR)/%.o: %.c
	@echo Compiling $< ...
	@mkdir -p $(dir $@)
	@$(CC) -c $(DEP_FLAGS) $(LIB_CFLAGS) -o $@ $<

$(LIB_OBJ_CPP) : $(LIB_OBJDIR)/%.o: %.cpp
	@echo Compiling $< ...
	@mkdir -p $(dir $@)
	@$(CXX) -c $(DEP_FLAGS) $(LIB_CXXFLAGS) -o $@ $<



$(LIB_DEP_C):

$(LIB_DEP_CPP):

include $(wildcard $(LIB_DEP_C) $(LIB_DEP_CPP))

