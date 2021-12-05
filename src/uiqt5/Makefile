
include ../../Parameters

QTDIR = $(HOME)/git/qt5/qt5-build/qtbase

TMP_UI_DIR := $(ROOT_DIR).tmp/uiqt5ub

UIC = $(QTDIR)/bin/uic
MOC = $(QTDIR)/bin/moc
RCC = $(QTDIR)/bin/rcc

LIB_SRC_FILES = \
		$(addprefix sx/, \
			SxChildWindow.cpp \
			SxCommandLine.cpp \
			SxCommandWin.cpp \
			SxDockBar.cpp \
			SxDockWindow.cpp \
			SxDocument.cpp \
			SxFindTextDlg.cpp \
			SxGUI.cpp \
			SxMainWindow.cpp \
			SxOutputWin.cpp \
			SxSignalMultiplexer.cpp \
			SxTabWidget.cpp \
			SxViewMgrMDI.cpp \
			SxViewMgrTabs.cpp \
		) \
		$(addprefix qsil/, \
			toolpalette.cpp \
			xmru.cpp \
			xresmgr.cpp \
			xresource.cpp \
			xshared.cpp \
			xwindowsettings.cpp \
		) \
		ADBDataWin.cpp \
		ADBFilesWin.cpp \
		ADBInterWin.cpp \
		ADBMainWin.cpp \
		ADCApp.cpp \
		ADCBinWin.cpp \
		ADCBlocksWin.cpp \
		ADCCodeWin.cpp \
		ADCConfigDlg.cpp \
		ADCCutListWin.cpp \
		ADCExprWin.cpp \
		ADCLineEdit.cpp \
		ADCMainWin.cpp \
		ADCNamesWin.cpp \
		ADCOutputWin.cpp \
		ADCProtoDlg.cpp \
		ADCResourceWin.cpp \
		ADCSourceWin.cpp \
		ADCSplitWin.cpp \
		ADCStubsWin.cpp \
		ADCTableView.cpp \
		ADCTabsWin.cpp \
		ADCTasksWin.cpp \
		ADCTemplWin.cpp \
		ADCTextView.cpp \
		ADCTypesWin.cpp \
		ADCUserPrefs.cpp \
		ADCUtils.cpp \
		colors.cpp

LIB_UI_FILES = \
	ADCProtoDlgBase.ui

LIB_H2MOC_FILES = \
	ADBDataWin.h \
	ADBFilesWin.h \
	ADBInterWin.h \
	ADBMainWin.h \
	ADCApp.h \
	ADCBinWin.h \
	ADCBlocksWin.h \
	ADCCodeWin.h \
	ADCConfigDlg.h \
	ADCCutListWin.h \
	ADCExprWin.h \
	ADCLineEdit.h \
	ADCMainWin.h \
	ADCNamesWin.h \
	ADCOutputWin.h \
	ADCProtoDlg.h \
	ADCResourceWin.h \
	ADCSourceWin.h \
	ADCSplitWin.h \
	ADCStubsWin.h \
	ADCTableView.h \
	ADCTemplWin.h \
	ADCTextView.h \
	ADCTasksWin.h \
	ADCTabsWin.h \
	ADCTypesWin.h \
	ADCUtils.h \
	$(addprefix qsil/, \
		xmru.h \
		xresmgr.h \
		xresource.h \
		xshared.h \
		toolpalette.h \
		xwindowsettings.h \
		) \
	$(addprefix sx/, \
		SxChildWindow.h \
		SxCommandLine.h \
		SxCommandWin.h \
		SxDockBar.h \
		SxDockWindow.h \
		SxFindTextDlg.h \
		SxMainWindow.h \
		SxOutputWin.h \
		SxSignalMultiplexer.h \
		SxTabWidget.h \
		SxViewMgrMDI.h \
		SxViewMgrTabs.h \
		)


IMG_FILES = $(wildcard images/*.png)	
IMG_COL_FILE_0 = ADCImageCollection.cpp
IMG_COL_FILE = $(TMP_UI_DIR)/$(IMG_COL_FILE_0)
#IMG_COL_OBJ_0 = $(IMG_COL_FILE_0:.cpp=.o)
		
LIB_UI2H_FILES_0 = $(LIB_UI_FILES:.ui=.ui.h)
LIB_UI2H_FILES = $(addprefix $(TMP_UI_DIR)/, $(LIB_UI2H_FILES_0))

LIB_MOC_FILES_0 = $(LIB_H2MOC_FILES:.h=.h.cpp)
LIB_MOC_FILES = $(addprefix $(TMP_UI_DIR)/, $(LIB_MOC_FILES_0))
#LIB_MOC_OBJ_0 = $(LIB_MOC_FILES_0:.cpp=.o)

LIB_TMP_FILES = $(LIB_MOC_FILES_0) $(IMG_COL_FILE_0)

LIB_TMP_OBJ := $(addprefix $(OBJDIR)/uiqt5/, $(LIB_TMP_FILES:.cpp=.o))

LIB_OBJ_ = $(LIB_SRC_FILES:.cpp=.o)
LIB_OBJ := $(addprefix $(OBJDIR)/uiqt5/, $(LIB_OBJ_))

LIB_DEP_FILES := $(addprefix $(OBJDIR)/uiqt5/, $(LIB_SRC_FILES:.cpp=.d))
LIB_DEP_TMP_FILES := $(addprefix $(OBJDIR)/uiqt5/, $(LIB_TMP_FILES:.cpp=.d))

LIB_CXXFLAGS = $(CXXFLAGS) -DQT_THREAD_SUPPORT
ifdef DEBUG
	LIB_CXXFLAGS += -DQT_DEBUG
else
	LIB_CXXFLAGS += -DQT_NODEBUG
endif
LIB_CXXFLAGS += -I.. -I../shared -I$(TMP_UI_DIR) -I$(QTDIR)/include -I$(QTDIR)/include/QtCore -I$(QTDIR)/include/QtGui -I$(QTDIR)/include/QtWidgets \
	-fvisibility-inlines-hidden -fvisibility=hidden -DADCUI_EXPORT='__attribute__((visibility("default")))'


LIB_LFLAGS := $(LINKER_FLAGS) \
	-shared -Wl,--no-undefined -Wl,-rpath=\$$ORIGIN \
	-Xlinker -Bdynamic \
	-L$(QTDIR)/lib -lQt5Widgets -lQt5Gui -lQt5Core \
	-L$(OBJDIR) -lshared \
	-Xlinker -Bdynamic -lrt -lm -ldl -lpthread \
	-L/usr/lib64 -lXext -lX11 -lfreetype -lfontconfig



#echo:=$(shell echo "LIB_TMP_OBJ: $(LIB_TMP_OBJ)" >&2)


all: \
	$(LIB_UI2H_FILES) \
	$(LIB_MOC_FILES) \
	$(IMG_COL_FILE) \
	$(OBJDIR)/libuiqt5.so
#	@echo DONE


$(OBJDIR)/libuiqt5.so : $(LIB_OBJ) $(LIB_TMP_OBJ)
	@echo Linking $@ ...
	@$(CXX) -o $@ $(LIB_OBJ) $(LIB_TMP_OBJ) $(LIB_LFLAGS)
	@echo "Made $@"

$(LIB_OBJ) : $(OBJDIR)/uiqt5/%.o: %.cpp
	@echo Compiling $< ...
	@mkdir -p $(dir $@)
	@$(CXX) -c $(DEP_FLAGS) $(LIB_CXXFLAGS) -o $@ $<

$(LIB_TMP_OBJ) : $(OBJDIR)/uiqt5/%.o: $(TMP_UI_DIR)/%.cpp
	@echo Compiling $< ...
	@mkdir -p $(dir $@)
	@$(CXX) -c $(DEP_FLAGS) $(LIB_CXXFLAGS) -o $@ $<



$(LIB_MOC_FILES) : $(TMP_UI_DIR)/%.h.cpp: %.h
	@echo MOC''ing $< ...
	@mkdir -p $(dir $@)
	$(MOC) $< -o $@

$(LIB_UI2H_FILES) : $(TMP_UI_DIR)/%.ui.h: %.ui
	@echo UIC''ing $< ...
	@mkdir -p $(dir $@)
	$(UIC) -o $@ $<

$(IMG_COL_FILE) : $(IMG_FILES)
	@echo Generating image collection $@ ...
	$(RCC) -o $(IMG_COL_FILE) -name adc images/images.qrc



$(LIB_DEP_FILES):

$(LIB_DEP_TMP_FILES):

include $(wildcard $(LIB_DEP_FILES) $(LIB_DEP_TMP_FILES))




