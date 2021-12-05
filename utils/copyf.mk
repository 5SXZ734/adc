
SHELL = /bin/sh

PROJ = copyf

ifndef DST_FILE
DST_FILE=dst
endif

ifndef SRC_FILE
SRC_FILE=src
endif

$(PROJ) : $(DST_FILE)
	@DONE=1

$(DST_FILE) : $(SRC_FILE)
	@echo $@
	@-mkdir -p $(@D)
	@cp $< $@