#---------------------------------------------------------------------------------
# ONScripter-jh for Nintendo Switch
# Updated Makefile with modern compiler flags and C++17 support
#---------------------------------------------------------------------------------

# GLES renderer objects (from OnscripterYuri)
GLES_OBJS = gles_renderer.o

GUI_OBJS = ONScripter.o \
	ONScripter_animation.o \
	ONScripter_command.o \
	ONScripter_effect.o \
	ONScripter_effect_breakup.o \
	ONScripter_event.o \
	ONScripter_file.o \
	ONScripter_file2.o \
	ONScripter_image.o \
	ONScripter_lut.o \
	ONScripter_rmenu.o \
	ONScripter_sound.o \
	ONScripter_video.o \
	ONScripter_text.o \
	AnimationInfo.o \
	FontInfo.o \
	DirtyRect.o \
	resize_image.o \
	LUAHandler.o \
	ONScripter_directdraw.o \
	Parallel.o \
	layer_snow.o \
	ONScripter_effect_cascade.o \
	ONScripter_effect_trig.o \
	layer_oldmovie.o

DECODER_OBJS = DirectReader.o \
	SarReader.o \
	NsaReader.o \
	sjis2utf16.o \
	gbk2utf16.o \
	coding2utf16.o


ONSCRIPTER_OBJS = \
	main.o \
	Common.o \
	GameBrowser.o \
	onscripter_main.o \
	$(DECODER_OBJS) \
	ScriptHandler.o \
	ScriptParser.o \
	ScriptParser_command.o \
	$(GUI_OBJS) \
	$(GLES_OBJS) \

#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
# EXEFS_SRC is the optional input directory containing data copied into exefs, if anything this normally should only contain "main.npdm".
#
# NO_ICON: if set to anything, do not use icon.
# NO_NACP: if set to anything, no .nacp file is generated.
# APP_TITLE is the name of the app stored in the .nacp file (Optional)
# APP_AUTHOR is the author of the app stored in the .nacp file (Optional)
# APP_VERSION is the version of the app stored in the .nacp file (Optional)
# APP_TITLEID is the titleID of the app stored in the .nacp file (Optional)
# ICON is the filename of the icon (.jpg), relative to the project folder.
#   If not set, it attempts to use one of the following (in this order):
#     - <Project name>.jpg
#     - icon.jpg
#     - <libnx folder>/default_icon.jpg
#---------------------------------------------------------------------------------
VERSION_MAJOR := 2
VERSION_MINOR := 2
VERSION_MICRO := 0

APP_TITLE	:=	ONScripter
APP_TITLEID :=	010FF000AE9A2C1B
APP_AUTHOR	:=	ONScripter-jh Contributors
APP_VERSION	:=	${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}

TARGET		:=	ONScripter
BUILD		:=	build
SOURCES		:=	source source/builtin_dll source/player source/reader source/onscripter source/renderer
DATA		:=	data
INCLUDES	:=	include SDL_kitchensink/Output/include
EXEFS_SRC	:=	exefs_src
CONFIG_JSON :=	ONScripter.json
ROMFS		:=	romfs

ICON		:= Icon.jpg

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
# Updated architecture flags for better optimization
ARCH	:=	-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE

# Optimization and warning flags
CFLAGS	:=	-O2 -ffunction-sections -fdata-sections \
			-Wall -Wextra -Wno-unused-parameter -Wno-sign-compare \
			$(ARCH) $(DEFINES)

CFLAGS	+=	$(INCLUDE) -DSWITCH -D__SWITCH__ \
			-I$(DEVKITPRO)/portlibs/switch/include/SDL2 \
			-I$(DEVKITPRO)/portlibs/switch/include/ \
			-I$(DEVKITPRO)/portlibs/switch/include/lua5.1

# Feature flags
CFLAGS	+= -DUSE_SDL_RENDERER -DNDEBUG -DUSE_OGG_VORBIS -DUSE_LUA
CFLAGS	+= -DUSE_SIMD_ARM_NEON -DUSE_SIMD
CFLAGS	+= -DUSE_BUILTIN_EFFECTSX -DUSE_BUILTIN_LAYER_EFFECTSX
CFLAGS	+= -DUSE_PARALLEL -DENABLE_1BYTE_CHAR
# GLES renderer for CAS sharpening (from OnscripterYuri)
CFLAGS	+= -DUSE_GLES

# C++17 standard with modern features
CXXFLAGS	:= $(CFLAGS) -std=gnu++17 \
			-fno-rtti -fno-exceptions \
			-Wno-deprecated-declarations

ASFLAGS	:=	$(ARCH)
LDFLAGS	=	-specs=$(DEVKITPRO)/libnx/switch.specs $(ARCH) \
			-Wl,-Map,$(notdir $*.map) \
			-Wl,--gc-sections

# Libraries
# Note: Order matters for static linking - dependencies must come after dependents
LIBS	:= -lSDL_kitchensink \
	-lswscale -lswresample -lavformat -lavfilter -lavcodec -lavutil \
	-ldav1d \
	-lpthread \
	-lSDL2_ttf -lSDL2_gfx -lSDL2_image -lSDL2_mixer \
	-lopusfile -lopus -lSDL2main -lSDL2 \
	-lbz2 -lass -lfribidi -ltheora -lvorbis \
	-lfreetype -lharfbuzz -lpng -lz -ljpeg -lwebp \
	-lEGL -lGLESv2 -lglapi -ldrm_nouveau \
	-llua5.1 \
	-lvorbisidec -logg -lvpx -lmpg123 -lmodplug -lFLAC \
	-lstdc++ \
	-lnx -lm

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(PORTLIBS) $(LIBNX) $(CURDIR)/SDL_kitchensink/Output


#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(ONSCRIPTER_OBJS:.o=.cpp)
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES 	:=	$(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))


export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

export BUILD_EXEFS_SRC := $(TOPDIR)/$(EXEFS_SRC)

ifeq ($(strip $(CONFIG_JSON)),)
	jsons := $(wildcard *.json)
	ifneq (,$(findstring $(TARGET).json,$(jsons)))
		export APP_JSON := $(TOPDIR)/$(TARGET).json
	else
		ifneq (,$(findstring config.json,$(jsons)))
			export APP_JSON := $(TOPDIR)/config.json
		endif
	endif
else
	export APP_JSON := $(TOPDIR)/$(CONFIG_JSON)
endif

ifeq ($(strip $(ICON)),)
	icons := $(wildcard *.jpg)
	ifneq (,$(findstring $(TARGET).jpg,$(icons)))
		export APP_ICON := $(TOPDIR)/$(TARGET).jpg
	else
		ifneq (,$(findstring icon.jpg,$(icons)))
			export APP_ICON := $(TOPDIR)/icon.jpg
		endif
	endif
else
	export APP_ICON := $(TOPDIR)/$(ICON)
endif

ifeq ($(strip $(NO_ICON)),)
	export NROFLAGS += --icon=$(APP_ICON)
endif

ifeq ($(strip $(NO_NACP)),)
	export NROFLAGS += --nacp=$(CURDIR)/$(TARGET).nacp
endif

ifneq ($(APP_TITLEID),)
	export NACPFLAGS += --titleid=$(APP_TITLEID)
endif

ifneq ($(ROMFS),)
	export NROFLAGS += --romfsdir=$(CURDIR)/$(ROMFS)
endif

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@cd SDL_kitchensink && mkdir -p build && cd build && cmake .. && make -j$(shell nproc 2>/dev/null || echo 4) install && cd ../..
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@cd SDL_kitchensink && rm -fr build && rm -fr Output && cd ..
	@rm -fr $(BUILD) $(TARGET).pfs0 $(TARGET).nso $(TARGET).nro $(TARGET).nacp $(TARGET).elf


#---------------------------------------------------------------------------------
else
.PHONY:	all

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------


all	:	$(OUTPUT).nro

#$(OUTPUT).nsp	:	$(OUTPUT).nso $(OUTPUT).npdm

#$(OUTPUT).nso	:	$(OUTPUT).elf

ifeq ($(strip $(NO_NACP)),)
$(OUTPUT).nro	:	$(OUTPUT).elf $(OUTPUT).nacp
else
$(OUTPUT).nro	:	$(OUTPUT).elf
endif


$(OUTPUT).elf	:	$(OFILES)

$(OFILES_SRC)	: $(HFILES_BIN)
#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	%_bin.h :	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)
#---------------------------------------------------------------------------------

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
