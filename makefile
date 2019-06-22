COMMON_FLAGS = -q -bt=dos -mf -3r -fp5 -dSHOW_TIMING
RELEASE_FLAGS = -oabhii+klmnrt -s -d0 -dNDEBUG
DEBUG_FLAGS = -od -d3

CPP = wpp386.exe -q $(COMMON_FLAGS) $(RELEASE_FLAGS)

HOST_CPP = wcl386 -q -xs

#LFLAGS = SYSTEM dos4g OPTION quiet OPTION map
LFLAGS = SYSTEM pmodewi OPTION quiet OPTION map

LD = wlink.exe $(LFLAGS)

LIB = wlib -n -q

DOSBOX = c:\bin\dosbox-x\dosbox-x.exe

run: app.exe
	$(DOSBOX) -conf dosbox.conf

app.exe: app.lib
	$(LD) N $@ F $<

app.obj: app.cpp app_player_adapter.lib kbd.lib vga_mode.lib vga_softvbi.lib vga_reg.lib snd.lib kb_tinymod.lib ost.lib efx.lib vga_pageflip.lib
	$(CPP) $[@
app.lib: app.obj app_player_adapter.lib kbd.lib vga_mode.lib vga_softvbi.lib vga_reg.lib snd.lib kb_tinymod.lib ost.lib efx.lib vga_pageflip.lib
	$(LIB) $@ $<

app_player_adapter.obj: app_player_adapter.cpp app_player_adapter.hpp kb_tinymod.lib
	$(CPP) $[@
app_player_adapter.lib: app_player_adapter.obj                        kb_tinymod.lib
	$(LIB) $@ $<

efx.obj: efx.cpp efx.hpp vga_mode.lib vga_reg.lib
	$(CPP) $[@
efx.lib: efx.obj         vga_mode.lib vga_reg.lib
	$(LIB) $@ $<

kbd.obj: kbd.cpp kbd.hpp 
	$(CPP) $[@
kbd.lib: kbd.obj
	$(LIB) $@ $<

vga_softvbi.obj: vga_softvbi.cpp vga_softvbi.hpp vga_reg.lib pit.lib
	$(CPP) $[@
vga_softvbi.lib: vga_softvbi.obj                 vga_reg.lib pit.lib
	$(LIB) $@ $<

vga_mode.obj: vga_mode.cpp vga_mode.hpp vga_reg.lib pit.lib
	$(CPP) $[@
vga_mode.lib: vga_mode.obj              vga_reg.lib pit.lib
	$(LIB) $@ $<

vga_reg.obj: vga_reg.cpp vga_reg.hpp
	$(CPP) $[@
vga_reg.lib: vga_reg.obj
	$(LIB) $@ $<

snd.obj: snd.cpp snd.hpp dma.lib pic.lib
	$(CPP) $[@
snd.lib: snd.obj         dma.lib pic.lib
	$(LIB) $@ $<

kb_tinymod.obj: kb_tinymod.cpp kb_tinymod.hpp
	$(CPP) $[@
kb_tinymod.lib: kb_tinymod.obj
	$(LIB) $@ $<

pit.obj: pit.cpp pit.hpp pic.lib
	$(CPP) $[@
pit.lib: pit.obj         pic.lib
	$(LIB) $@ $<

dma.obj: dma.cpp dma.hpp mem.lib
	$(CPP) $[@
dma.lib: dma.obj         mem.lib
	$(LIB) $@ $<

vga_pageflip.obj: vga_pageflip.cpp vga_pageflip.hpp vga_mode.lib
	$(CPP) $[@
vga_pageflip.lib: vga_pageflip.obj                  vga_mode.lib
	$(LIB) $@ $<

pic.obj: pic.cpp pic.hpp
	$(CPP) $[@
pic.lib: pic.obj
	$(LIB) $@ $<

mem.obj: mem.cpp mem.hpp
	$(CPP) $[@
mem.lib: mem.obj
	$(LIB) $@ $<

ost.obj: ost.cpp ost.hpp
	$(CPP) $[@
ost.lib: ost.obj
	$(LIB) $@ $<

ost.cpp ost.hpp: urea.mod bin2cpp.exe
	bin2cpp $[@ ost rqdq ostData

bin2cpp.exe: bin2cpp.cpp
	$(HOST_CPP) $[@

clean:
	del *.obj
	del *.lib
	del app.exe
	del bin2cpp.exe
	del ost.cpp
	del ost.hpp
