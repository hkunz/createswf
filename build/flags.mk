ifeq ($(STATIC),1)
PKG_CONFIG_LIBS_FLAGS := --static
CONFIG_LIBS_FLAGS = --static-libs
else
CONFIG_LIBS_FLAGS = --libs
endif

CFLAGS += -DHAVE_CONFIG_H
CFLAGS += -ggdb
CFLAGS += -pipe
CFLAGS += -Winline
CFLAGS += -Wcast-qual
CFLAGS += -Wcast-align
CFLAGS += -Wpointer-arith
CFLAGS += -Wno-long-long
CFLAGS += -pedantic
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wno-sign-compare
CFLAGS += -Wno-unused-parameter
CFLAGS += -Wreturn-type
CFLAGS += -Wwrite-strings
CFLAGS += -I$(SRCDIR)
CFLAGS += -I$(SRCDIR)/libs

ifeq ($(PROFILING),1)
  CFLAGS  += -pg
  CCFLAGS += -pg
  LDFLAGS += -pg
endif

ifeq ($(DEBUG),1)
  CFLAGS  += -DDEBUG
  CCFLAGS  += -DDEBUG
else
  CFLAGS  += -DNDEBUG
  CCFLAGS  += -DNDEBUG
endif

CCFLAGS += $(CFLAGS)
CCFLAGS += -std=c99

CXXFLAGS += $(CFLAGS)
CXXFLAGS += -Wnon-virtual-dtor
