CUR_DIR  = .
OBJDIR   = $(CUR_DIR)
BUILDDIR = $(CUR_DIR)/build

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
	ONScripter_text.o \
	AnimationInfo.o \
	FontInfo.o \
	DirtyRect.o \
	resize_image.o \
	LUAHandler.o \
	ONScripter_directdraw.o \
	Parallel.o \
	builtin_dll/layer_snow.o \
	builtin_dll/ONScripter_effect_cascade.o \
	builtin_dll/ONScripter_effect_trig.o \
	builtin_dll/layer_oldmovie.o 

DECODER_OBJS = DirectReader.o \
	SarReader.o \
	NsaReader.o \
	sjis2utf16.o \
	gbk2utf16.o \
	coding2utf16.o 


ONSCRIPTER_OBJS = \
	onscripter_main.o \
	$(DECODER_OBJS) \
	ScriptHandler.o \
	ScriptParser.o \
	ScriptParser_command.o \
	$(GUI_OBJS)





TARGET   = onscripter
TITLE_ID = ONSJHVITB


PREFIX   = arm-vita-eabi
LDFLAGS = \
#DMOVIE
DEFS = -DPSV -DUSE_SDL_RENDERER -DNDEBUG -DUSE_OGG_VORBIS -DUSE_LUA
DEFS += -DUSE_SIMD_ARM_NEON -DUSE_SIMD
DEFS += -DUSE_BUILTIN_EFFECTS -DUSE_BUILTIN_LAYER_EFFECTS
DEFS += -DUSE_PARALLEL

CCFLAGS  = -Wl,-q -O3 -g $(DEFS) -I. -I.. -I$(VITASDK)/$(PREFIX)/include/SDL2
CXXFLAGS = $(CCFLAGS) -fno-rtti -fno-exceptions -fno-optimize-sibling-calls -std=c++11

# -lsmpeg
LDFLAGS  += -liniparser -lSceLibKernel_stub \
	-lSDL2_mixer -lSDL2_ttf -lSDL2_image -lSDL2 -lmikmod \
	-lvita2d -lvita2d_ext -lScePgf_stub -lSceHid_stub \
	-lbz2 -lm -lFLAC -lFLAC++ -lmpg123\
	-lvorbisfile -lvorbis -logg -ljpeg -lluajit\
	-lfreetype -lpng -lz -lSceNetCtl_stub -lSceNet_stub\
	-lSceAudio_stub  -lSceCommonDialog_stub \
	-lSceCtrl_stub -lSceDisplay_stub -lSceGxm_stub -lScePower_stub \
	-lSceTouch_stub -ldl -lSceSysmodule_stub -ltaihen_stub \
	-lSceShellSvc_stub -lSceAppMgr_stub -lSceAppUtil_stub \
	-lScePromoterUtil_stub


#include Makefile.onscripter
all: $(TARGET).vpk

$(TARGET).vpk: eboot.bin
	vita-mksfoex -s TITLE_ID=$(TITLE_ID) "$(TARGET)" $(BUILDDIR)/param.sfo
	vita-pack-vpk -s $(BUILDDIR)/param.sfo -b $(BUILDDIR)/eboot.bin \
	-a $(BUILDDIR)/ons.bin=ons.bin \
	-a res/default.ttf=default.ttf \
	-a res/icon0.png=sce_sys/icon0.png \
	-a res/bg.png=sce_sys/livearea/contents/bg.png \
	-a res/startup.png=sce_sys/livearea/contents/startup.png \
	-a res/template.xml=sce_sys/livearea/contents/template.xml $(BUILDDIR)/$@

eboot.bin: $(TARGET).velf
	vita-make-fself -c -s $(BUILDDIR)/gui.velf $(BUILDDIR)/eboot.bin
	vita-make-fself -c -s $(BUILDDIR)/$< $(BUILDDIR)/ons.bin

%.velf: %.elf
	vita-elf-create $(BUILDDIR)/$< $(BUILDDIR)/$@
	vita-elf-create $(BUILDDIR)/gui.elf $(BUILDDIR)/gui.velf

$(TARGET).elf: $(ONSCRIPTER_OBJS) $(GUI_W_OBJS)
	@echo $@
	@$(PREFIX)-g++ $(CXXFLAGS)  -MMD -MT -MF $(ONSCRIPTER_OBJS) -o $(BUILDDIR)/$@ $(LDFLAGS)
	@$(PREFIX)-g++ $(CXXFLAGS)  -MMD -MT -MF $(GUI_W_OBJS) -o $(BUILDDIR)/gui.elf $(LDFLAGS)

	

%.o : %.cpp
	@echo [CC] $<
	@$(PREFIX)-g++ $(CXXFLAGS) -c $(OBJDIR)/$< -o $(OBJDIR)/$@
	

vpksend: $(TARGET).vpk
	curl -T $(BUILDDIR)/$(TARGET).vpk ftp://$(VITAIP):1337/ux0:/
	@echo "Sent."

send: eboot.bin
	curl -T $(BUILDDIR)/eboot.bin ftp://$(VITAIP):1337/ux0:/app/$(TITLE_ID)/
	@echo "Sent."

clean:
	@rm -rf $(BUILDDIR)/$(TARGET).velf $(BUILDDIR)/$(TARGET).elf $(BUILDDIR)/$(TARGET).vpk $(BUILDDIR)/eboot.bin $(BUILDDIR)/param.sfo
	@rm -rf $(BUILDDIR)/$(TARGET)
	@rm -rf $(OBJDIR)/*.o
	@rm -rf $(OBJDIR)/builtin_dll/*.o
	@rm -rf *.o
	@rm -rf builtin_dll/*.o
	@rm -rf vpkinstall/*.o
#mymv:
#	mv *.o $(OBJDIR)/
#	mv builtin_dll/*.o $(OBJDIR)/builtin_dll/

