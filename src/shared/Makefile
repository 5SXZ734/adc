
include ../../Parameters

		
LIB_SRC_FILES = \
	action.cpp \
	dump_util.cpp \
	misc.cpp \
	mmap.cpp \
	$(addprefix qx/, \
		ConvertUTF.cpp \
		DynLib.cpp \
		MyApp.cpp \
		MyArgs.cpp \
		MyCapture.cpp \
		MyDir.cpp \
		MyFileMgr.cpp \
		MyGuiThread.cpp \
		MyPath.cpp \
		MyRedirect.cpp \
		MyRedirect2.cpp \
		MyStream.cpp \
		MyString.cpp \
		MyStringList.cpp \
		QxTime.cpp \
	) \
	$(addprefix tiny_process/, \
		process.cpp \
		process_unix.cpp \
	)

LIB_OBJ0 = $(LIB_SRC_FILES:.cpp=.o)
LIB_OBJ := $(addprefix $(OBJDIR)/shared/, $(LIB_OBJ0))
LIB_CXXFLAGS = $(CXXFLAGS) -I.. -I../shared
LIB_LFLAGS = $(LINKER_FLAGS)

LIB_DEP_FILES = $(addprefix $(OBJDIR)/shared/, $(LIB_SRC_FILES:.cpp=.d))

#echo:=$(shell echo "LIB_OBJ: $(LIB_OBJ)" >&2)

$(OBJDIR)/libshared.a : $(LIB_OBJ)
	@echo Archiving $@ ...
	@$(AR) rcs $@ $(LIB_OBJ)	
	@echo "Made $@"	

$(LIB_OBJ) : $(OBJDIR)/shared/%.o: %.cpp
	@echo Compiling $< ...
	@mkdir -p $(dir $@)
	@$(CXX) -c $(DEP_FLAGS) $(LIB_CXXFLAGS) -o $@ $<



$(LIB_DEP_FILES):

include $(wildcard $(LIB_DEP_FILES))

#$(OUTDIR): ; @mkdir -p $(dir $@)


