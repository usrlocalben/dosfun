HOST_CPP = wcl386 -q -xs

OBJ = obj
LIB = lib

COMMON_FLAGS = -q -bt=dos -mf -3r -fp5 -dSHOW_TIMING
RELEASE_FLAGS = -onatx -d0 -dNDEBUG
DEBUG_FLAGS = -od -d3

CPPFLAGS = $(COMMON_FLAGS) $(RELEASE_FLAGS)
CPP = wpp386.exe -q $(COMMON_FLAGS) $(RELEASE_FLAGS)

AR = wlib -n -q

#LFLAGS = SYSTEM dos4g OPTION quiet OPTION map
LFLAGS = SYSTEM pmodewi OPTION quiet OPTION map OPTION eliminate
LD = wlink.exe $(LFLAGS)

DOSBOX = c:\bin\dosbox-x\dosbox-x.exe

run: app.exe
	$(DOSBOX) -conf build-support\dosbox.conf

app.exe: app.$(LIB)
	$(LD) N $@ F $<
	upx -9 app.exe

app.$(OBJ): app.cpp    app_kefrens_bars.$(LIB) app_player_adapter.$(LIB) kb_tinymod.$(LIB) ost.$(LIB) pc_kbd.$(LIB) sb16.$(LIB) sb_detect.$(LIB) vga_mode.$(LIB) vga_pageflip.$(LIB) vga_reg.$(LIB) vga_irq.$(LIB)
	$(CPP) $[@
app.$(LIB): app.$(OBJ) app_kefrens_bars.$(LIB) app_player_adapter.$(LIB) kb_tinymod.$(LIB) ost.$(LIB) pc_kbd.$(LIB) sb16.$(LIB) sb_detect.$(LIB) vga_mode.$(LIB) vga_pageflip.$(LIB) vga_reg.$(LIB) vga_irq.$(LIB)
	$(AR) $@ $<

app_player_adapter.$(OBJ): app_player_adapter.cpp    app_player_adapter.hpp kb_tinymod.$(LIB) alg_ringindex.$(LIB)
	$(CPP) $[@
app_player_adapter.$(LIB): app_player_adapter.$(OBJ)                        kb_tinymod.$(LIB) alg_ringindex.$(LIB)
	$(AR) $@ $<

app_kefrens_bars.$(OBJ): app_kefrens_bars.cpp    app_kefrens_bars.hpp vga_mode.$(LIB) vga_reg.$(LIB)
	$(CPP) $[@
app_kefrens_bars.$(LIB): app_kefrens_bars.$(OBJ)                      vga_mode.$(LIB) vga_reg.$(LIB)
	$(AR) $@ $<

pc_kbd.$(OBJ): pc_kbd.cpp    pc_kbd.hpp pc_pic.$(LIB)
	$(CPP) $[@
pc_kbd.$(LIB): pc_kbd.$(OBJ)            pc_pic.$(LIB)
	$(AR) $@ $<

vga_irq.$(OBJ): vga_irq.cpp    vga_irq.hpp vga_reg.$(LIB) pc_pit.$(LIB) pc_cpu.$(LIB)
	$(CPP) $[@
vga_irq.$(LIB): vga_irq.$(OBJ)             vga_reg.$(LIB) pc_pit.$(LIB) pc_cpu.$(LIB)
	$(AR) $@ $<

vga_mode.$(OBJ): vga_mode.cpp    vga_mode.hpp vga_reg.$(LIB) vga_bios.$(LIB) pc_bus.$(LIB) pc_cpu.$(LIB)
	$(CPP) $[@
vga_mode.$(LIB): vga_mode.$(OBJ)              vga_reg.$(LIB) vga_bios.$(LIB) pc_bus.$(LIB) pc_cpu.$(LIB)
	$(AR) $@ $<

vga_reg.$(OBJ): vga_reg.cpp    vga_reg.hpp pc_bus.$(LIB) pc_cpu.$(LIB)
	$(CPP) $[@
vga_reg.$(LIB): vga_reg.$(OBJ)             pc_bus.$(LIB) pc_cpu.$(LIB)
	$(AR) $@ $<

vga_bios.$(OBJ): vga_bios.cpp    vga_bios.hpp pc_bus.$(LIB)
	$(CPP) $[@
vga_bios.$(LIB): vga_bios.$(OBJ)              pc_bus.$(LIB)
	$(AR) $@ $<

sb16.$(OBJ): sb16.cpp    sb16.hpp pc_dma.$(LIB) pc_pic.$(LIB) pc_cpu.$(LIB) pc_bus.$(LIB) 
	$(CPP) $[@
sb16.$(LIB): sb16.$(OBJ)          pc_dma.$(LIB) pc_pic.$(LIB) pc_cpu.$(LIB) pc_bus.$(LIB)
	$(AR) $@ $<

sb_detect.$(OBJ): sb_detect.cpp    sb_detect.hpp
	$(CPP) $[@
sb_detect.$(LIB): sb_detect.$(OBJ)
	$(AR) $@ $<

kb_tinymod.$(OBJ): kb_tinymod.cpp    kb_tinymod.hpp
	$(CPP) $[@
kb_tinymod.$(LIB): kb_tinymod.$(OBJ)
	$(AR) $@ $<

pc_pit.$(OBJ): pc_pit.cpp    pc_pit.hpp pc_pic.$(LIB) pc_cpu.$(LIB)
	$(CPP) $[@
pc_pit.$(LIB): pc_pit.$(OBJ)            pc_pic.$(LIB) pc_cpu.$(LIB)
	$(AR) $@ $<

pc_cpu.$(OBJ): pc_cpu.cpp    pc_cpu.hpp
	$(CPP) $[@
pc_cpu.$(LIB): pc_cpu.$(OBJ)
	$(AR) $@ $<

pc_bus.$(OBJ): pc_bus.cpp    pc_bus.hpp
	$(CPP) $[@
pc_bus.$(LIB): pc_bus.$(OBJ)
	$(AR) $@ $<

pc_dma.$(OBJ): pc_dma.cpp    pc_dma.hpp os_realmem.$(LIB) pc_bus.$(LIB)
	$(CPP) $[@
pc_dma.$(LIB): pc_dma.$(OBJ)            os_realmem.$(LIB) pc_bus.$(LIB)
	$(AR) $@ $<

vga_pageflip.$(OBJ): vga_pageflip.cpp    vga_pageflip.hpp vga_mode.$(LIB)
	$(CPP) $[@
vga_pageflip.$(LIB): vga_pageflip.$(OBJ)                  vga_mode.$(LIB)
	$(AR) $@ $<

pc_pic.$(OBJ): pc_pic.cpp    pc_pic.hpp pc_bus.$(LIB)
	$(CPP) $[@
pc_pic.$(LIB): pc_pic.$(OBJ)            pc_bus.$(LIB)
	$(AR) $@ $<

os_realmem.$(OBJ): os_realmem.cpp    os_realmem.hpp
	$(CPP) $[@
os_realmem.$(LIB): os_realmem.$(OBJ)
	$(AR) $@ $<

alg_ringindex.$(OBJ): alg_ringindex.cpp    alg_ringindex.hpp
	$(CPP) $[@
alg_ringindex.$(LIB): alg_ringindex.$(OBJ)
	$(AR) $@ $<

ost.$(OBJ): ost.cpp    ost.hpp
	$(CPP) $[@
ost.$(LIB): ost.$(OBJ)
	$(AR) $@ $<

ost.cpp ost.hpp: urea.mod bin2cpp.exe
	bin2cpp $[@ ost rqdq ostData

bin2cpp.exe: bin2cpp.cpp
	$(HOST_CPP) $[@

clean:
	del *.$(OBJ)
	del *.$(LIB)
	del app.exe
	del bin2cpp.exe
	del ost.cpp
	del ost.hpp
