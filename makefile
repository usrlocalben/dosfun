COMMON_FLAGS = -q -bt=dos -mf -3r -fp5 -dSHOW_TIMING
RELEASE_FLAGS = -onatx -d0 -dNDEBUG
DEBUG_FLAGS = -od -d3

CPP = wpp386.exe -q $(COMMON_FLAGS) $(RELEASE_FLAGS)
# CPPSM = wpp386.exe -q $(COMMON_FLAGS) -os -d0 -dNDEBUG

HOST_CPP = wcl386 -q -xs

#LFLAGS = SYSTEM dos4g OPTION quiet OPTION map
LFLAGS = SYSTEM pmodewi OPTION quiet OPTION map OPTION eliminate

LD = wlink.exe $(LFLAGS)

LIB = wlib -n -q

DOSBOX = c:\bin\dosbox-x\dosbox-x.exe

run: app.exe
	$(DOSBOX) -conf build-support\dosbox.conf

app.exe: app.lib
	$(LD) N $@ F $<
	upx -9 app.exe

app.obj: app.cpp app_kefrens_bars.lib app_player_adapter.lib kb_tinymod.lib ost.lib pc_kbd.lib sb16.lib sb_detect.lib vga_mode.lib vga_pageflip.lib vga_reg.lib vga_irq.lib
	$(CPP) $[@
app.lib: app.obj app_kefrens_bars.lib app_player_adapter.lib kb_tinymod.lib ost.lib pc_kbd.lib sb16.lib sb_detect.lib vga_mode.lib vga_pageflip.lib vga_reg.lib vga_irq.lib
	$(LIB) $@ $<

app_player_adapter.obj: app_player_adapter.cpp app_player_adapter.hpp kb_tinymod.lib
	$(CPP) $[@
app_player_adapter.lib: app_player_adapter.obj                        kb_tinymod.lib
	$(LIB) $@ $<

app_kefrens_bars.obj: app_kefrens_bars.cpp app_kefrens_bars.hpp vga_mode.lib vga_reg.lib
	$(CPP) $[@
app_kefrens_bars.lib: app_kefrens_bars.obj                      vga_mode.lib vga_reg.lib
	$(LIB) $@ $<

pc_kbd.obj: pc_kbd.cpp pc_kbd.hpp pc_pic.lib
	$(CPP) $[@
pc_kbd.lib: pc_kbd.obj            pc_pic.lib
	$(LIB) $@ $<

vga_irq.obj: vga_irq.cpp vga_irq.hpp vga_reg.lib pc_pit.lib pc_cpu.lib
	$(CPP) $[@
vga_irq.lib: vga_irq.obj             vga_reg.lib pc_pit.lib pc_cpu.lib
	$(LIB) $@ $<

vga_mode.obj: vga_mode.cpp vga_mode.hpp vga_reg.lib vga_bios.lib pc_bus.lib pc_cpu.lib
	$(CPP) $[@
vga_mode.lib: vga_mode.obj              vga_reg.lib vga_bios.lib pc_bus.lib pc_cpu.lib
	$(LIB) $@ $<

vga_reg.obj: vga_reg.cpp vga_reg.hpp pc_bus.lib pc_cpu.lib
	$(CPP) $[@
vga_reg.lib: vga_reg.obj             pc_bus.lib pc_cpu.lib
	$(LIB) $@ $<

vga_bios.obj: vga_bios.cpp vga_bios.hpp pc_bus.lib
	$(CPP) $[@
vga_bios.lib: vga_bios.obj              pc_bus.lib
	$(LIB) $@ $<

sb16.obj: sb16.cpp sb16.hpp pc_dma.lib pc_pic.lib pc_cpu.lib pc_bus.lib 
	$(CPP) $[@
sb16.lib: sb16.obj          pc_dma.lib pc_pic.lib pc_cpu.lib pc_bus.lib
	$(LIB) $@ $<

sb_detect.obj: sb_detect.cpp sb_detect.hpp
	$(CPP) $[@
sb_detect.lib: sb_detect.obj
	$(LIB) $@ $<

kb_tinymod.obj: kb_tinymod.cpp kb_tinymod.hpp
	$(CPP) $[@
kb_tinymod.lib: kb_tinymod.obj
	$(LIB) $@ $<

pc_pit.obj: pc_pit.cpp pc_pit.hpp pc_pic.lib pc_cpu.lib
	$(CPP) $[@
pc_pit.lib: pc_pit.obj            pc_pic.lib pc_cpu.lib
	$(LIB) $@ $<

pc_cpu.obj: pc_cpu.cpp pc_cpu.hpp
	$(CPP) $[@
pc_cpu.lib: pc_cpu.obj
	$(LIB) $@ $<

pc_bus.obj: pc_bus.cpp pc_bus.hpp
	$(CPP) $[@
pc_bus.lib: pc_bus.obj
	$(LIB) $@ $<

pc_dma.obj: pc_dma.cpp pc_dma.hpp os_realmem.lib pc_bus.lib
	$(CPP) $[@
pc_dma.lib: pc_dma.obj            os_realmem.lib pc_bus.lib
	$(LIB) $@ $<

vga_pageflip.obj: vga_pageflip.cpp vga_pageflip.hpp vga_mode.lib
	$(CPP) $[@
vga_pageflip.lib: vga_pageflip.obj                  vga_mode.lib
	$(LIB) $@ $<

pc_pic.obj: pc_pic.cpp pc_pic.hpp pc_bus.lib
	$(CPP) $[@
pc_pic.lib: pc_pic.obj            pc_bus.lib
	$(LIB) $@ $<

os_realmem.obj: os_realmem.cpp os_realmem.hpp
	$(CPP) $[@
os_realmem.lib: os_realmem.obj
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
