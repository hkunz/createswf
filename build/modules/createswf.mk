TARGET             := createswf

$(TARGET)_STATIC   ?= $(STATIC)
ifeq ($($(TARGET)_STATIC),1)
$(TARGET)_LDFLAGS  += -static
endif

$(TARGET)_LINKER   := $(CXX)
$(TARGET)_FILE     := $(TARGET)$(EXE_EXT)
$(TARGET)_LDFLAGS  += $(OPENGL_LIBS)
$(TARGET)_CFLAGS   += $(OPENGLES_CFLAGS)
$(TARGET)_SRCS      = \
	main.cpp \
	CoreApplication.cpp \
	\
	common/Logger.cpp \
	gui/MainWindow.cpp \
	parsers/CommandLineParser.cpp \
	parsers/DefinitionParser.cpp \
	parsers/DirectoryParser.cpp \
	\

ifeq ($(TARGET_OS),mingw32)
	$(TARGET)_SRCS +=\
		\
	$(TARGET)_LDFLAGS +=
else
	$(TARGET)_SRCS +=\
		\
	$(TARGET)_LDFLAGS +=
endif

$(TARGET)_OBJS     := $(call ASSEMBLE_OBJECTS,$(TARGET))
$(TARGET)_CXXFLAGS := $($(TARGET)_CFLAGS) -fno-rtti -fno-exceptions -fcheck-new
$(TARGET)_CCFLAGS  := $($(TARGET)_CFLAGS)
