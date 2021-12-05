SHELL = /bin/sh

PROJ = ADC

ifndef CONFIG
	CONFIG = Release
endif
ifndef PLATFORM
	PLATFORM = x64
endif

ROOT_DIR=../../..

#TOOLSET=v141

COPY = $(ROOT_DIR)/utils/cp.exe
MKDIR = $(ROOT_DIR)/utils/mkdir.exe
SRC_DIR = $(ROOT_DIR)/src
OBJ_DIR = $(ROOT_DIR)/.build/$(TOOLSET)/$(PLATFORM)/$(CONFIG)

PROTO_DIR = $(SRC_DIR)/front/proto
PROTO_FILES = $(wildcard $(PROTO_DIR)/*.hh) $(wildcard $(PROTO_DIR)/*.h) $(wildcard $(PROTO_DIR)/*.export)
PROTO_DIR_OUT = $(OBJ_DIR)/proto
PROTO_FILES_OUT = $(addprefix $(PROTO_DIR_OUT)/, $(notdir $(PROTO_FILES)))

CAPSTONE_DLL = $(OBJ_DIR)/capstone.dll
CAPSTONE_DLL_SOURCE = $(THIRDPARTY)/.build/$(TOOLSET)/$(PLATFORM)/$(CONFIG)/capstone.dll

UDIS86_DLL = $(OBJ_DIR)/udis86.dll
UDIS86_DLL_SOURCE = $(THIRDPARTY)/.build/$(TOOLSET)/$(PLATFORM)/$(CONFIG)/udis86.dll

DEMANGLERS_DLL = $(OBJ_DIR)/demanglers.dll
DEMANGLERS_DLL_SOURCE = $(THIRDPARTY)/.build/$(TOOLSET)/$(PLATFORM)/$(CONFIG)/demanglers.dll

#echo:=$(shell echo "OBJ_DIR: $(notdir $(OBJ_DIR))" >&2)
#echo:=$(shell echo "PROTO_FILES_OUT: $(PROTO_FILES_OUT)" >&2)

# R U L E S : : : : : : : 

$(PROJ): \
	$(CAPSTONE_DLL) \
	$(UDIS86_DLL) \
	$(DEMANGLERS_DLL) \
	$(PROTO_FILES_OUT)
	@echo DONE

$(CAPSTONE_DLL) : $(CAPSTONE_DLL_SOURCE)
	-@$(MKDIR) $(subst /,\\,$(dir $@))
	$(COPY) -f $(CAPSTONE_DLL_SOURCE) $(CAPSTONE_DLL)

$(UDIS86_DLL) : $(UDIS86_DLL_SOURCE)
	-@$(MKDIR) $(subst /,\\,$(dir $@))
	$(COPY) -f $(UDIS86_DLL_SOURCE) $(UDIS86_DLL)

$(DEMANGLERS_DLL) : $(DEMANGLERS_DLL_SOURCE)
	-@$(MKDIR) $(subst /,\\,$(dir $@))
	$(COPY) -f $(DEMANGLERS_DLL_SOURCE) $(DEMANGLERS_DLL)

$(PROTO_FILES_OUT) : $(PROTO_DIR_OUT)/% : $(PROTO_DIR)/%
	-@$(MKDIR) $(subst /,\\,$(dir $@))
	$(COPY) -f $< $@


