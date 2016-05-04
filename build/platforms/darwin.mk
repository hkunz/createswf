SO_EXT                    = dylib
SO_CFLAGS                 = -fPIC -fno-common
SO_LDFLAGS                = -dynamiclib
SO_LIBS                  := -ldl

CFLAGS                   += -D_BSD_SOURCE -D_XOPEN_SOURCE
LDFLAGS                  += -framework IOKit -framework Foundation -framework Cocoa -lQtOpenGL -lQtGui -lQtCore -framework OpenGL -framework AGL
LDFLAGS                  += -rdynamic

CFLAGS                   += -I/opt/local/include -F/opt/local/Library/Frameworks -I/opt/local/share/qt4/mkspecs/macx-g++ -I. -I.moc -I../libtiled -I/opt/local/include/QtOpenGL -I/opt/local/include/QtGui -I/opt/local/include/QtCore -I/opt/local/include -I/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers -I/System/Library/Frameworks/AGL.framework/Headers -I.uic
LDFLAGS                  += -L/opt/local/lib -F/opt/local/Library/Frameworks

ifdef UNIVERSAL
	CFLAGS += -arch i386 -arch ppc
	LDFLAGS += -arch i386 -arch ppc
endif

FRAMEWORK_DIR            ?= /opt/local/Library/Frameworks
SDL_IMAGE_CFLAGS         ?= -I$(FRAMEWORK_DIR)/SDL_image.framework/Headers
SDL_IMAGE_LIBS           ?= -framework SDL_image
OPENGL_CFLAGS            ?=
OPENGL_LIBS              ?= -framework OpenGL
OPENAL_CFLAGS            ?=
OPENAL_LIBS              ?= -framework OpenAL
