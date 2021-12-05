
include ../../Parameters

LIB_SRC_FILES = \
	code.capstone.cpp \
	code.udis86.cpp \
	cpu16.cpp \
	cpu32.cpp \
	decode.DWARF.cpp \
	decode.PDB.cpp \
	decode.pe.cpp \
	format.adc.cpp \
	format.dwarf.cpp \
	format.elf.cpp \
	format.elf_S.cpp \
	format.exe.cpp \
	format.ico.cpp \
	format.java.cpp \
	format.pdb_D.cpp \
	format.pdb_S.cpp \
	format.pdb20.cpp \
	format.pdb70.cpp \
	format.pe.cpp \
	format.pe.NET.cpp \
	format.pe_D.cpp \
	format.pe_S.cpp \
	front_IA.cpp \
	front_main.cpp \
	shared.cpp \
	x86_dump.cpp \
	x86_IR.cpp

LIB_OBJ0 = $(LIB_SRC_FILES:.cpp=.o)
LIB_OBJ := $(addprefix $(OBJDIR)/front/, $(LIB_OBJ0))

LIB_CXXFLAGS := $(CXXFLAGS) -DFRONTEND_EXPORTS -I.. -I../shared -I$(THIRDPARTY)/capstone/include -I$(THIRDPARTY)/udis86 -I$(THIRDPARTY)/demanglers
LIB_LFLAGS := $(LINKER_FLAGS) \
	-shared -Wl,--no-undefined -Wl,-rpath=\$$ORIGIN \
	-Xlinker -Bstatic -L$(OBJDIR) -lshared \
	-Xlinker -Bdynamic -L$(OBJDIR) -lcapstone -ludis86 -ldemanglers
	
LIB_DEP_FILES := $(addprefix $(OBJDIR)/front/, $(LIB_SRC_FILES:.cpp=.d))

#echo:=$(shell echo "LIB_OBJ: $(LIB_OBJ)" >&2)


$(OBJDIR)/libfront.so : $(LIB_OBJ) $(addprefix $(OBJDIR)/, libdemanglers.so libcapstone.so libudis86.so libshared.a)
	@echo Linking $@ ...
	@$(CXX) -o $@ $(LIB_OBJ) $(LIB_LFLAGS)
	@echo "Made $@"

$(LIB_OBJ) : $(OBJDIR)/front/%.o: %.cpp
	@echo Compiling $< ...
	@mkdir -p $(dir $@)
	@$(CXX) -c $(DEP_FLAGS) $(LIB_CXXFLAGS) -o $@ $<



$(OBJDIR)/libdemanglers.so : FORCE
	@$(MAKE) -s -C $(THIRDPARTY)/demanglers

$(OBJDIR)/libcapstone.so : FORCE
	@$(MAKE) -s -C $(THIRDPARTY)/capstone

$(OBJDIR)/libudis86.so : FORCE
	@$(MAKE) -s -C $(THIRDPARTY)/udis86



$(OBJDIR)/libshared.a : FORCE
	@$(MAKE) -s -C $(SRC)/shared

FORCE:



$(LIB_DEP_FILES):

include $(wildcard $(LIB_DEP_FILES))
