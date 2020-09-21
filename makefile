HOST_CPP = g++

OBJ = o
LIB = a

COMMON_FLAGS = -std=gnu++17 -DSHOW_TIMING #-DTTYCON #-DEARLY_EOI
RELEASE_FLAGS = -O3 -ffast-math -DNDEBUG
DEBUG_FLAGS = -g

#CPPFLAGS = $(COMMON_FLAGS) $(DEBUG_FLAGS)
CPPFLAGS = $(COMMON_FLAGS) $(RELEASE_FLAGS)
CPP = /usr/local/djgpp/bin/i586-pc-msdosdjgpp-g++ -c $(CPPFLAGS)

AR = /usr/local/djgpp/bin/i586-pc-msdosdjgpp-ar rcsT

LFLAGS =
LD = /usr/local/djgpp/bin/i586-pc-msdosdjgpp-g++ $(LFLAGS)

DOSBOX = c:\bin\dosbox-x\dosbox-x.exe

run: app.exe
	$(DOSBOX) -conf build-support\dosbox.conf

app_cwsstub.exe: app.$(LIB)
	$(LD) -o $@ -s $<

app_cwsstub.coff: app_cwsstub.exe exe2coff.exe
	./exe2coff $<

app.exe: app_cwsstub.coff
	cat build-support/PMODSTUB.EXE app_cwsstub.coff > app.exe
	upx -9 app.exe

app.$(OBJ): app.cpp    ryg.$(LIB) app_kefrens_bars.$(LIB) app_player_adapter.$(LIB) kb_tinymod.$(LIB) data_ost.$(LIB) pc_kbd.$(LIB) sb16.$(LIB) sb_detect.$(LIB) vga_mode.$(LIB) vga_pageflip.$(LIB) vga_irq.$(LIB) vga_reg.$(LIB) pc_com.$(LIB) log.$(LIB) text.$(LIB) vga_bios.$(LIB)
	$(CPP) $<
app.$(LIB): app.$(OBJ) ryg.$(LIB) app_kefrens_bars.$(LIB) app_player_adapter.$(LIB) kb_tinymod.$(LIB) data_ost.$(LIB) pc_kbd.$(LIB) sb16.$(LIB) sb_detect.$(LIB) vga_mode.$(LIB) vga_pageflip.$(LIB) vga_irq.$(LIB) vga_reg.$(LIB) pc_com.$(LIB) log.$(LIB) text.$(LIB) vga_bios.$(LIB)
	@rm -f $@
	$(AR) $@ $^

app_player_adapter.$(OBJ): app_player_adapter.cpp    app_player_adapter.hpp kb_tinymod.$(LIB) alg_ringindex.$(LIB)
	$(CPP) $<
app_player_adapter.$(LIB): app_player_adapter.$(OBJ)                        kb_tinymod.$(LIB) alg_ringindex.$(LIB)
	@rm -f $@
	$(AR) $@ $^

app_kefrens_bars.$(OBJ): app_kefrens_bars.cpp    app_kefrens_bars.hpp vga_mode.$(LIB) vga_reg.$(LIB) data_amy.$(LIB) canvas.$(LIB)
	$(CPP) $<
app_kefrens_bars.$(LIB): app_kefrens_bars.$(OBJ)                      vga_mode.$(LIB) vga_reg.$(LIB) data_amy.$(LIB) canvas.$(LIB)
	@rm -f $@
	$(AR) $@ $^

pc_kbd.$(OBJ): pc_kbd.cpp    pc_kbd.hpp pc_pic.$(LIB) alg_ringindex.$(LIB)
	$(CPP) $<
pc_kbd.$(LIB): pc_kbd.$(OBJ)            pc_pic.$(LIB) alg_ringindex.$(LIB)
	@rm -f $@
	$(AR) $@ $^

pc_com.$(OBJ): pc_com.cpp    pc_com.hpp pc_pic.$(LIB) alg_ringindex.$(LIB)
	$(CPP) $<
pc_com.$(LIB): pc_com.$(OBJ)            pc_pic.$(LIB) alg_ringindex.$(LIB)
	@rm -f $@
	$(AR) $@ $^

vga_irq.$(OBJ): vga_irq.cpp    vga_irq.hpp vga_reg.$(LIB) pc_pit.$(LIB) pc_cpu.$(LIB)
	$(CPP) $<
vga_irq.$(LIB): vga_irq.$(OBJ)             vga_reg.$(LIB) pc_pit.$(LIB) pc_cpu.$(LIB)
	@rm -f $@
	$(AR) $@ $^

vga_mode.$(OBJ): vga_mode.cpp    vga_mode.hpp vga_reg.$(LIB) vga_bios.$(LIB) pc_bus.$(LIB) pc_cpu.$(LIB)
	$(CPP) $<
vga_mode.$(LIB): vga_mode.$(OBJ)              vga_reg.$(LIB) vga_bios.$(LIB) pc_bus.$(LIB) pc_cpu.$(LIB)
	@rm -f $@
	$(AR) $@ $^

vga_reg.$(OBJ): vga_reg.cpp    vga_reg.hpp pc_bus.$(LIB) pc_cpu.$(LIB) pixel.$(LIB)
	$(CPP) $<
vga_reg.$(LIB): vga_reg.$(OBJ)             pc_bus.$(LIB) pc_cpu.$(LIB) pixel.$(LIB)
	@rm -f $@
	$(AR) $@ $^

vga_bios.$(OBJ): vga_bios.cpp    vga_bios.hpp pc_bus.$(LIB)
	$(CPP) $<
vga_bios.$(LIB): vga_bios.$(OBJ)              pc_bus.$(LIB)
	@rm -f $@
	$(AR) $@ $^

sb16.$(OBJ): sb16.cpp    sb16.hpp pc_dma.$(LIB) pc_pic.$(LIB) pc_cpu.$(LIB) pc_bus.$(LIB)
	$(CPP) $<
sb16.$(LIB): sb16.$(OBJ)          pc_dma.$(LIB) pc_pic.$(LIB) pc_cpu.$(LIB) pc_bus.$(LIB)
	@rm -f $@
	$(AR) $@ $^

sb_detect.$(OBJ): sb_detect.cpp    sb_detect.hpp
	$(CPP) $<
sb_detect.$(LIB): sb_detect.$(OBJ)
	@rm -f $@
	$(AR) $@ $^

kb_tinymod.$(OBJ): kb_tinymod.cpp    kb_tinymod.hpp
	$(CPP) -Wno-multichar $<
kb_tinymod.$(LIB): kb_tinymod.$(OBJ)
	@rm -f $@
	$(AR) $@ $^

pc_pit.$(OBJ): pc_pit.cpp    pc_pit.hpp pc_pic.$(LIB) pc_cpu.$(LIB)
	$(CPP) $<
pc_pit.$(LIB): pc_pit.$(OBJ)            pc_pic.$(LIB) pc_cpu.$(LIB)
	@rm -f $@
	$(AR) $@ $^

pc_cpu.$(OBJ): pc_cpu.cpp    pc_cpu.hpp
	$(CPP) $<
pc_cpu.$(LIB): pc_cpu.$(OBJ)
	@rm -f $@
	$(AR) $@ $^

pc_bus.$(OBJ): pc_bus.cpp    pc_bus.hpp
	$(CPP) $<
pc_bus.$(LIB): pc_bus.$(OBJ)
	@rm -f $@
	$(AR) $@ $^

pc_dma.$(OBJ): pc_dma.cpp    pc_dma.hpp os_realmem.$(LIB) pc_bus.$(LIB)
	$(CPP) $<
pc_dma.$(LIB): pc_dma.$(OBJ)            os_realmem.$(LIB) pc_bus.$(LIB)
	@rm -f $@
	$(AR) $@ $^

vga_pageflip.$(OBJ): vga_pageflip.cpp    vga_pageflip.hpp vga_mode.$(LIB)
	$(CPP) $<
vga_pageflip.$(LIB): vga_pageflip.$(OBJ)                  vga_mode.$(LIB)
	@rm -f $@
	$(AR) $@ $^

pc_pic.$(OBJ): pc_pic.cpp    pc_pic.hpp pc_bus.$(LIB)
	$(CPP) $<
pc_pic.$(LIB): pc_pic.$(OBJ)            pc_bus.$(LIB)
	@rm -f $@
	$(AR) $@ $^

os_realmem.$(OBJ): os_realmem.cpp    os_realmem.hpp
	$(CPP) $<
os_realmem.$(LIB): os_realmem.$(OBJ)
	@rm -f $@
	$(AR) $@ $^

log.$(OBJ): log.cpp    log.hpp alg_ringindex.$(LIB)
	$(CPP) $<
log.$(LIB): log.$(OBJ)         alg_ringindex.$(LIB)
	@rm -f $@
	$(AR) $@ $^

alg_ringindex.$(OBJ): alg_ringindex.cpp    alg_ringindex.hpp
	$(CPP) $<
alg_ringindex.$(LIB): alg_ringindex.$(OBJ)
	@rm -f $@
	$(AR) $@ $^

data_ost.$(OBJ): data_ost.cpp    data_ost.hpp
	$(CPP) $<
data_ost.$(LIB): data_ost.$(OBJ)
	@rm -f $@
	$(AR) $@ $^

text.$(OBJ): text.cpp    text.hpp
	$(CPP) $<
text.$(LIB): text.$(OBJ)
	@rm -f $@
	$(AR) $@ $^

vec.$(OBJ): vec.cpp    vec.hpp
	$(CPP) $<
vec.$(LIB): vec.$(OBJ)
	@rm -f $@
	$(AR) $@ $^

canvas.$(OBJ): canvas.cpp    canvas.hpp vec.$(LIB) picopng.$(LIB) pixel.$(LIB)
	$(CPP) $<
canvas.$(LIB): canvas.$(OBJ)            vec.$(LIB) picopng.$(LIB) pixel.$(LIB)
	@rm -f $@
	$(AR) $@ $^

ryg.$(OBJ): ryg.cpp    ryg.hpp
	$(CPP) $<
ryg.$(LIB): ryg.$(OBJ)
	@rm -f $@
	$(AR) $@ $^

pixel.$(OBJ): pixel.cpp    pixel.hpp ryg.$(LIB) vec.$(LIB)
	$(CPP) $<
pixel.$(LIB): pixel.$(OBJ)           ryg.$(LIB) vec.$(LIB)
	@rm -f $@
	$(AR) $@ $^

picopng.$(OBJ): picopng.cpp    picopng.hpp
	$(CPP) $<
picopng.$(LIB): picopng.$(OBJ)
	@rm -f $@
	$(AR) $@ $^

data_amy.$(OBJ): data_amy.cpp    data_amy.hpp
	$(CPP) $<
data_amy.$(LIB): data_amy.$(OBJ)
	@rm -f $@
	$(AR) $@ $^

data_ost.cpp data_ost.hpp: urea.mod bin2cpp.exe
	./bin2cpp $< data_ost data ost

data_amy.cpp data_amy.hpp: amy.png bin2cpp.exe
	./bin2cpp $< data_amy data amy

bin2cpp.exe: bin2cpp.cpp
	$(HOST_CPP) -o $@ $<

exe2coff.exe: exe2coff.cpp
	$(HOST_CPP) -o $@ $<

clean:
	@rm -f *.$(OBJ)
	@rm -f *.$(LIB)
	@rm -f app*.exe
	@rm -f app*.coff
	@rm -f app*.map
	@rm -f bin2cpp.exe
	@rm -f exe2coff.exe
	@rm -f data_*.{cpp,hpp}
