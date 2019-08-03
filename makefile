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

OBJ = obj

DOSBOX = c:\bin\dosbox-x\dosbox-x.exe

run: app.exe
	$(DOSBOX) -conf build-support\dosbox.conf

app.exe: app.lib
	$(LD) N $@ F $<
	upx -9 app.exe

app.$(OBJ): app.cpp app_kefrens_bars.lib app_player_adapter.lib kb_tinymod.lib ost.lib pc_kbd.lib sb16.lib sb_detect.lib vga_mode.lib vga_pageflip.lib vga_reg.lib vga_irq.lib
	$(CPP) $[@
app.lib: app.$(OBJ) app_kefrens_bars.lib app_player_adapter.lib kb_tinymod.lib ost.lib pc_kbd.lib sb16.lib sb_detect.lib vga_mode.lib vga_pageflip.lib vga_reg.lib vga_irq.lib
	$(LIB) $@ $<

app_player_adapter.$(OBJ): app_player_adapter.cpp    app_player_adapter.hpp kb_tinymod.lib alg_ringindex.lib
	$(CPP) $[@
app_player_adapter.lib:    app_player_adapter.$(OBJ)                        kb_tinymod.lib alg_ringindex.lib
	$(LIB) $@ $<

app_kefrens_bars.$(OBJ): app_kefrens_bars.cpp    app_kefrens_bars.hpp vga_mode.lib vga_reg.lib
	$(CPP) $[@
app_kefrens_bars.lib:    app_kefrens_bars.$(OBJ)                      vga_mode.lib vga_reg.lib
	$(LIB) $@ $<

pc_kbd.$(OBJ): pc_kbd.cpp    pc_kbd.hpp pc_pic.lib
	$(CPP) $[@
pc_kbd.lib:    pc_kbd.$(OBJ)            pc_pic.lib
	$(LIB) $@ $<

vga_irq.$(OBJ): vga_irq.cpp    vga_irq.hpp vga_reg.lib pc_pit.lib pc_cpu.lib
	$(CPP) $[@
vga_irq.lib:    vga_irq.$(OBJ)             vga_reg.lib pc_pit.lib pc_cpu.lib
	$(LIB) $@ $<

vga_mode.$(OBJ): vga_mode.cpp    vga_mode.hpp vga_reg.lib vga_bios.lib pc_bus.lib pc_cpu.lib
	$(CPP) $[@
vga_mode.lib:    vga_mode.$(OBJ)              vga_reg.lib vga_bios.lib pc_bus.lib pc_cpu.lib
	$(LIB) $@ $<

vga_reg.$(OBJ): vga_reg.cpp    vga_reg.hpp pc_bus.lib pc_cpu.lib
	$(CPP) $[@
vga_reg.lib:    vga_reg.$(OBJ)             pc_bus.lib pc_cpu.lib
	$(LIB) $@ $<

vga_bios.$(OBJ): vga_bios.cpp    vga_bios.hpp pc_bus.lib
	$(CPP) $[@
vga_bios.lib:    vga_bios.$(OBJ)              pc_bus.lib
	$(LIB) $@ $<

sb16.$(OBJ): sb16.cpp    sb16.hpp pc_dma.lib pc_pic.lib pc_cpu.lib pc_bus.lib 
	$(CPP) $[@
sb16.lib:    sb16.$(OBJ)          pc_dma.lib pc_pic.lib pc_cpu.lib pc_bus.lib
	$(LIB) $@ $<

sb_detect.$(OBJ): sb_detect.cpp    sb_detect.hpp
	$(CPP) $[@
sb_detect.lib:    sb_detect.$(OBJ)
	$(LIB) $@ $<

kb_tinymod.$(OBJ): kb_tinymod.cpp    kb_tinymod.hpp
	$(CPP) $[@
kb_tinymod.lib:    kb_tinymod.$(OBJ)
	$(LIB) $@ $<

pc_pit.$(OBJ): pc_pit.cpp    pc_pit.hpp pc_pic.lib pc_cpu.lib
	$(CPP) $[@
pc_pit.lib:    pc_pit.$(OBJ)            pc_pic.lib pc_cpu.lib
	$(LIB) $@ $<

pc_cpu.$(OBJ): pc_cpu.cpp    pc_cpu.hpp
	$(CPP) $[@
pc_cpu.lib:    pc_cpu.$(OBJ)
	$(LIB) $@ $<

pc_bus.$(OBJ): pc_bus.cpp    pc_bus.hpp
	$(CPP) $[@
pc_bus.lib:    pc_bus.$(OBJ)
	$(LIB) $@ $<

pc_dma.$(OBJ): pc_dma.cpp    pc_dma.hpp os_realmem.lib pc_bus.lib
	$(CPP) $[@
pc_dma.lib:    pc_dma.$(OBJ)            os_realmem.lib pc_bus.lib
	$(LIB) $@ $<

vga_pageflip.$(OBJ): vga_pageflip.cpp    vga_pageflip.hpp vga_mode.lib
	$(CPP) $[@
vga_pageflip.lib:    vga_pageflip.$(OBJ)                  vga_mode.lib
	$(LIB) $@ $<

pc_pic.$(OBJ): pc_pic.cpp    pc_pic.hpp pc_bus.lib
	$(CPP) $[@
pc_pic.lib:    pc_pic.$(OBJ)            pc_bus.lib
	$(LIB) $@ $<

os_realmem.$(OBJ): os_realmem.cpp    os_realmem.hpp
	$(CPP) $[@
os_realmem.lib:    os_realmem.$(OBJ)
	$(LIB) $@ $<

alg_ringindex.$(OBJ): alg_ringindex.cpp    alg_ringindex.hpp
	$(CPP) $[@
alg_ringindex.lib:    alg_ringindex.$(OBJ)
	$(LIB) $@ $<

ost.$(OBJ): ost.cpp    ost.hpp
	$(CPP) $[@
ost.lib:    ost.$(OBJ)
	$(LIB) $@ $<

ost.cpp ost.hpp: urea.mod bin2cpp.exe
	bin2cpp $[@ ost rqdq ostData

bin2cpp.exe: bin2cpp.cpp
	$(HOST_CPP) $[@

clean:
	del *.$(OBJ)
	del *.lib
	del app.exe
	del bin2cpp.exe
	del ost.cpp
	del ost.hpp
