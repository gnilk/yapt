##########################################################################
##########################################################################
#
# Main make file for YAPT, plugins and player
#
# - depends:
#		libexpat, glfw
#
# Compiler settings
CC     = clang++

OBJDIR 	 = obj

ROOT		 = ../..
EXT_ROOT	 = $(ROOT)/ext

NOICE_IO_SRC = $(EXT_ROOT)/noice.io/dev/src
NOICE_IO_INC = $(EXT_ROOT)/noice.io/dev/inc
EXPAT_ROOT   = $(EXT_ROOT)/expat/lib
YAPT_SRC_DIR = $(ROOT)/src
YAPT_INC_DIR = $(ROOT)/inc

MONGOOSE_ROOT = $(EXT_ROOT)/mongoose

PLUGIN_SRC_DIR = $(ROOT)/plugins

STDCURVE_PLUGIN = $(PLUGIN_SRC_DIR)/StdCurve

PLUGIN_DEST_DIR = plugins
PLUGINS = StdCurve utils glFuncs Geometry Enigma
PLUGINS_DIRS = \
	$(PLUGIN_SRC_DIR)/StdCurve  \
	$(PLUGIN_SRC_DIR)/utils 	\
	$(PLUGIN_SRC_DIR)/glFuncs	\
	$(PLUGIN_SRC_DIR)/Geometry	\
	$(PLUGIN_SRC_DIR)/Enigma	\

INCLUDE_FILES = -I/opt/local/include/freetype2 -I/opt/local/include -I/usr/local/include -I. -I$(YAPT_INC_DIR) -I$(NOICE_IO_INC) -I$(EXPAT_ROOT) -I$(MONGOOSE_ROOT)
LIB_FILES = -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -L/opt/local/lib -lGLEW -lglfw -lexpat -lfreetype


CPPFLAGS = -g -arch x86_64 -stdlib=libc++
CPPFLAGS += -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk
CPPFLAGS += $(INCLUDE_FILES)

CFLAGS = -g -arch x86_64 -std=c99
CFLAGS += -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk
CFLAGS += $(INCLUDE_FILES)

# Linker settings
LD = clang++
LDFLAGS =  -dynamiclib -arch x86_64 -stdlib=libc++
LDFLAGS += $(LIB_FILES)

LINK_LIBS += -framework Cocoa -framework OpenGL -L/opt/local/lib -lglfw3 -lexpat
PLAYER_LINK_LIBS += $(LIB_FILES) -L. -lyapt


PLAYER_SRC_FILES = \
	glfwplayer.cpp \
	WebService.cpp \
	ConsoleWindow.cpp \
	BaseWindow.cpp \
	PlayerWindow.cpp \

MONGOOSE_SRC_FILES = \
	$(MONGOOSE_ROOT)/mongcpp.cpp \
	$(MONGOOSE_ROOT)/mongoose.c

MONGOOSE_OBJ_FILES = \
	$(MONGOOSE_ROOT)/mongcpp.o \
	$(MONGOOSE_ROOT)/mongoose.o


NOICE_IO_SRC_FILES = \
	$(NOICE_IO_SRC)/BasicFileDevice.cpp \
	$(NOICE_IO_SRC)/BasicFileStream.cpp \
	$(NOICE_IO_SRC)/DeviceFactoryHolder.cpp \
	$(NOICE_IO_SRC)/DeviceManager.cpp \
	$(NOICE_IO_SRC)/TextReader.cpp \
	$(NOICE_IO_SRC)/URLParser.cpp \

YAPT_SRC_FILES = \
	$(YAPT_SRC_DIR)/Attribute.cpp \
	$(YAPT_SRC_DIR)/BaseInstance.cpp \
	$(YAPT_SRC_DIR)/ConfigObject.cpp \
	$(YAPT_SRC_DIR)/Context.cpp \
	$(YAPT_SRC_DIR)/DocNode.cpp \
	$(YAPT_SRC_DIR)/Document.cpp \
	$(YAPT_SRC_DIR)/DocumentController.cpp \
	$(YAPT_SRC_DIR)/ExpatXMLImporter.cpp \
	$(YAPT_SRC_DIR)/ExportXML.cpp \
	$(YAPT_SRC_DIR)/FPSController.cpp \
	$(YAPT_SRC_DIR)/logger.cpp \
	$(YAPT_SRC_DIR)/Lookup.cpp \
	$(YAPT_SRC_DIR)/MetaInstance.cpp \
	$(YAPT_SRC_DIR)/ObjectDefinition.cpp \
	$(YAPT_SRC_DIR)/ObjectInstance.cpp \
	$(YAPT_SRC_DIR)/PlatformUtils.cpp \
	$(YAPT_SRC_DIR)/PluginContainer.cpp \
	$(YAPT_SRC_DIR)/PluginScanner_nix.cpp \
	$(YAPT_SRC_DIR)/Property.cpp \
	$(YAPT_SRC_DIR)/RenderVars.cpp \
	$(YAPT_SRC_DIR)/ResourceContainer.cpp \
	$(YAPT_SRC_DIR)/System.cpp \
	$(YAPT_SRC_DIR)/SystemHookHandler.cpp \
	$(YAPT_SRC_DIR)/Timeline.cpp \
	$(YAPT_SRC_DIR)/Timer.cpp \
	$(YAPT_SRC_DIR)/TopLevel.cpp \


YAPT_OBJ_FILES := $(patsubst %.cpp,%.o,$(YAPT_SRC_FILES))
NOICE_IO_OBJ_FILES := $(patsubst %.cpp,%.o,$(NOICE_IO_SRC_FILES))
#MONGOOSE_OBJ_FILES := $(patsubst %.cpp,%.o,$(MONGOOSE_SRC_FILES))
PLAYER_OBJ_FILES := $(patsubst %.cpp,%.o,$(PLAYER_SRC_FILES))

# Default: Build all tests
all: yapt player plugins

.PHONY: plugins $(PLUGINS_DIRS)

 %.o : %.cpp
	$(CC) -c $(CPPFLAGS) $< -o $@

%.o : %.c
	$(CC) -c $(CFLAGS)  $< -o $@

player: $(PLAYER_OBJ_FILES)
	$(CC) $(CFLAGS) $(PLAYER_OBJ_FILES) $(PLAYER_LINK_LIBS) -o glfwplayer

yapt: $(YAPT_OBJ_FILES) $(NOICE_IO_OBJ_FILES) $(MONGOOSE_OBJ_FILES)
	$(LD) $(LDFLAGS) $(YAPT_OBJ_FILES) $(NOICE_IO_OBJ_FILES) $(MONGOOSE_OBJ_FILES) -o libyapt.a

## - compile all plugins by calling recursive make's
plugins: $(PLUGINS_DIRS)
$(PLUGINS_DIRS):
	$(MAKE) -C $@ PLUGIN_NAME=$(@F)
	cp $@/lib$(@F).dylib $(PLUGIN_DEST_DIR)

clean: cleanyapt cleanplugins cleanplayer
	echo cleaned

cleanyapt:
	rm $(YAPT_OBJ_FILES) $(NOICE_IO_OBJ_FILES)	libyapt.a 
cleanplayer:
	rm $(PLAYER_OBJ_FILES) glfwplayer

cleanplugins:
	rm $(PLUGIN_DEST_DIR)/*.dylib
	for dir in $(PLUGINS_DIRS); do\
		$(MAKE) -C $$dir clean; \
	done
