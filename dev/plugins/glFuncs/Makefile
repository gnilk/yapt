##########################################################################
##########################################################################

# Compiler settings
CC     = g++

OBJDIR 	 = obj

ROOT		 = ../..
EXT_ROOT	 = $(ROOT)/ext
SDK = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk


NOICE_IO_SRC = $(EXT_ROOT)/noice.io/dev/src
NOICE_IO_INC = $(EXT_ROOT)/noice.io/dev/inc
EXPAT_ROOT   = $(EXT_ROOT)/expat/lib
YAPT_SRC_DIR = $(ROOT)/src
YAPT_INC_DIR = $(ROOT)/inc

CFLAGS = -g -arch x86_64 -stdlib=libc++
#CFLAGS += -macosx_version_min 10.8
CFLAGS += -isysroot $(SDK)
CFLAGS += -I/opt/local/include/freetype2 -I/opt/local/include -I/usr/local/include -I. -I$(YAPT_INC_DIR) -I$(NOICE_IO_INC) -I$(EXPAT_ROOT)


# Linker settings
LD = clang++
LDFLAGS =  -dynamiclib -arch x86_64 -stdlib=libc++
LDFLAGS += -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -L/opt/local/lib -lGLEW -lfreetype

PLUGIN_NAME_LIB = lib$(PLUGIN_NAME).dylib
PLUGIN_SRC_FILES = $(wildcard *.cpp)
PLUGIN_OBJ_FILES := $(patsubst %.cpp,%.o,$(PLUGIN_SRC_FILES))

# Default: Build all tests
all: plugin

 %.o : %.cpp
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

plugin: $(PLUGIN_OBJ_FILES)
	$(LD) $(LDFLAGS) $(PLUGIN_OBJ_FILES) -o $(PLUGIN_NAME_LIB)

clean:
	rm $(PLUGIN_OBJ_FILES) *.dylib
