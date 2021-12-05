
SHELL = /bin/sh

PROJ = IWorkbench
ifndef PROJ_NAME
	PROJ_NAME=$(PROJ)
endif

ifndef SRC_DIR
	SRC_DIR=../Src
endif

ISYSNAME=x86-nt
SYSNAME=i686-CYGWIN_NT-5.0

include $(SRC_DIR)/Versions

MY_DIR=$(SRC_DIR)/../Utils
BIN_DIR=$(SRC_DIR)/../.bin/Release
MSVC_DIR=X:/packages/msvc71
MANUALS_DIR=$(SRC_DIR)/../docs
EXAMPLES_DIR=$(SRC_DIR)/../examples
LIBC_DIR=$(SRC_DIR)/include/libc

ifndef ROOT_DIR
	INSTALL_DIR = $(PROJ)/$(IWORKBENCH_VERSION)
else
	INSTALL_DIR = $(ROOT_DIR)/$(PROJ)/$(IWORKBENCH_VERSION)
endif

INSTALL_DOCS = $(INSTALL_DIR)/docs
INSTALL_EXAMPLES = $(INSTALL_DIR)/examples
INSTALL_TRANS = $(INSTALL_DIR)/translations
INSTALL_BIN = $(INSTALL_DIR)/x86-nt

SRC_FILES =  \
	$(BIN_DIR)/IWorkbench.exe \
	$(BIN_DIR)/sci.exe \
	$(BIN_DIR)/jsi.exe \		$(MSVC_DIR)/msvcp71.dll \
		$(MSVC_DIR)/msvcr71.dll \
			$(LIBC_DIR)/assert.h \
			$(LIBC_DIR)/ctype.h \
			$(LIBC_DIR)/malloc.h \
			$(LIBC_DIR)/math.h \
			$(LIBC_DIR)/stdio.h \
			$(LIBC_DIR)/stdlib.h \
			$(LIBC_DIR)/string.h \
			$(LIBC_DIR)/time.h \
				$(EXAMPLES_DIR)/tests/array.c \
				$(EXAMPLES_DIR)/tests/enum.c \
				$(EXAMPLES_DIR)/tests/loop.c \
				$(EXAMPLES_DIR)/JavaScript/perfect.js \
		$(MANUALS_DIR)/iworkbench_users1.pdf
				
#$(BIN_DIR)/qt-mt335.dll
#$(BIN_DIR)/translations/smartspice_cn.qm

DST_FILES = \
	$(INSTALL_BIN)/IWorkbench.exe \
	$(INSTALL_BIN)/sci.exe \
	$(INSTALL_BIN)/jsi.exe \		$(INSTALL_BIN)/msvcp71.dll \
		$(INSTALL_BIN)/msvcr71.dll \
			$(INSTALL_BIN)/SCI/assert.h \
			$(INSTALL_BIN)/SCI/ctype.h \
			$(INSTALL_BIN)/SCI/malloc.h \
			$(INSTALL_BIN)/SCI/math.h \
			$(INSTALL_BIN)/SCI/stdio.h \
			$(INSTALL_BIN)/SCI/stdlib.h \
			$(INSTALL_BIN)/SCI/string.h \
			$(INSTALL_BIN)/SCI/time.h \
				$(INSTALL_EXAMPLES)/c/array.c \
				$(INSTALL_EXAMPLES)/c/enum.c \
				$(INSTALL_EXAMPLES)/c/loop.c \
				$(INSTALL_EXAMPLES)/js/perfect.js \
		$(INSTALL_DOCS)/iworkbench_users1.pdf

#$(INSTALL_BIN)/qt-mt335.dll \
#$(INSTALL_TRANS)/smartspice_cn.qm \
		

$(PROJ) : $(DST_FILES) 
	@echo DONE 
	
$(DST_FILES) : update
	@$(MAKE) -f $(MY_DIR)/copyf.mk --no-print-directory DST_FILE=$@ SRC_FILE=$(filter %$(@F), $(SRC_FILES))
	
update:






