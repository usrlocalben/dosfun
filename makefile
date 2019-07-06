#OBJ = obj
##COMMON_FLAGS = -q -bt=dos -mf -3r -fp5 #-dSHOW_TIMING
#RELEASE_FLAGS = -onatx -d0 -dNDEBUG
#DEBUG_FLAGS = -od -d3
#CPP = wpp386.exe -q $(COMMON_FLAGS) $(RELEASE_FLAGS)
#LFLAGS = SYSTEM dos4g OPTION quiet OPTION map
#LFLAGS = SYSTEM pmodewi OPTION quiet OPTION map OPTION eliminate
#LD = wlink.exe $(LFLAGS)
#LIB = wlib -n -q

OBJ = o
CPP = /usr/local/djgpp/bin/i586-pc-msdosdjgpp-g++ -c -O3 -ffast-math -DSHOW_TIMING
#CPP = /usr/local/djgpp/bin/i586-pc-msdosdjgpp-g++ -c -O2 -DNDEBUG

LIB = /usr/local/djgpp/bin/i586-pc-msdosdjgpp-ar rcs
LD = /usr/local/djgpp/bin/i586-pc-msdosdjgpp-g++

HOST_CPP = g++


OBJ = obj

DOSBOX = c:\bin\dosbox-x\dosbox-x.exe

run: app.exe
	$(DOSBOX) -conf build-support\dosbox.conf

app_cwsstub.exe: app.lib
	$(LD) -o $@ -s *.o
	# $(LD) -o $@ -s $<

app_cwsstub.coff: app_cwsstub.exe exe2coff.exe
	./exe2coff $<

app.exe: app_cwsstub.coff
	cat build-support/PMODSTUB.EXE app_cwsstub.coff > app.exe
	upx -9 app.exe

app.$(OBJ): app.cpp app_kefrens_bars.lib app_player_adapter.lib kb_tinymod.lib ost.lib pc_kbd.lib sb16.lib sb_detect.lib vga_mode.lib vga_pageflip.lib vga_reg.lib vga_irq.lib
	$(CPP) $<
app.lib: app.$(OBJ) app_kefrens_bars.lib app_player_adapter.lib kb_tinymod.lib ost.lib pc_kbd.lib sb16.lib sb_detect.lib vga_mode.lib vga_pageflip.lib vga_reg.lib vga_irq.lib
	rm -f $@
	$(LIB) $@ $^

app_player_adapter.$(OBJ): app_player_adapter.cpp    app_player_adapter.hpp kb_tinymod.lib
	$(CPP) $<
app_player_adapter.lib:    app_player_adapter.$(OBJ)                        kb_tinymod.lib
	rm -f $@
	$(LIB) $@ $^

app_kefrens_bars.$(OBJ): app_kefrens_bars.cpp    app_kefrens_bars.hpp vga_mode.lib vga_reg.lib
	$(CPP) $<
app_kefrens_bars.lib:    app_kefrens_bars.$(OBJ)                      vga_mode.lib vga_reg.lib
	rm -f $@
	$(LIB) $@ $^

pc_kbd.$(OBJ): pc_kbd.cpp    pc_kbd.hpp pc_pic.lib
	$(CPP) $<
pc_kbd.lib:    pc_kbd.$(OBJ)            pc_pic.lib
	rm -f $@
	$(LIB) $@ $^

vga_irq.$(OBJ): vga_irq.cpp    vga_irq.hpp vga_reg.lib pc_pit.lib pc_cpu.lib
	$(CPP) $<
vga_irq.lib:    vga_irq.$(OBJ)             vga_reg.lib pc_pit.lib pc_cpu.lib
	rm -f $@
	$(LIB) $@ $^

vga_mode.$(OBJ): vga_mode.cpp    vga_mode.hpp vga_reg.lib vga_bios.lib pc_bus.lib pc_cpu.lib
	$(CPP) $<
vga_mode.lib:    vga_mode.$(OBJ)              vga_reg.lib vga_bios.lib pc_bus.lib pc_cpu.lib
	rm -f $@
	$(LIB) $@ $^

vga_reg.$(OBJ): vga_reg.cpp    vga_reg.hpp pc_bus.lib pc_cpu.lib
	$(CPP) $<
vga_reg.lib:    vga_reg.$(OBJ)             pc_bus.lib pc_cpu.lib
	rm -f $@
	$(LIB) $@ $^

vga_bios.$(OBJ): vga_bios.cpp    vga_bios.hpp pc_bus.lib
	$(CPP) $<
vga_bios.lib:    vga_bios.$(OBJ)              pc_bus.lib
	rm -f $@
	$(LIB) $@ $^

sb16.$(OBJ): sb16.cpp    sb16.hpp pc_dma.lib pc_pic.lib pc_cpu.lib pc_bus.lib 
	$(CPP) $<
sb16.lib:    sb16.$(OBJ)          pc_dma.lib pc_pic.lib pc_cpu.lib pc_bus.lib
	rm -f $@
	$(LIB) $@ $^

sb_detect.$(OBJ): sb_detect.cpp    sb_detect.hpp
	$(CPP) $<
sb_detect.lib:    sb_detect.$(OBJ)
	rm -f $@
	$(LIB) $@ $^

kb_tinymod.$(OBJ): kb_tinymod.cpp    kb_tinymod.hpp
	$(CPP) $<
kb_tinymod.lib:    kb_tinymod.$(OBJ)
	rm -f $@
	$(LIB) $@ $^

pc_pit.$(OBJ): pc_pit.cpp    pc_pit.hpp pc_pic.lib pc_cpu.lib
	$(CPP) $<
pc_pit.lib:    pc_pit.$(OBJ)            pc_pic.lib pc_cpu.lib
	rm -f $@
	$(LIB) $@ $^

pc_cpu.$(OBJ): pc_cpu.cpp    pc_cpu.hpp
	$(CPP) $<
pc_cpu.lib:    pc_cpu.$(OBJ)
	rm -f $@
	$(LIB) $@ $^

pc_bus.$(OBJ): pc_bus.cpp    pc_bus.hpp
	$(CPP) $<
pc_bus.lib:    pc_bus.$(OBJ)
	rm -f $@
	$(LIB) $@ $^

pc_dma.$(OBJ): pc_dma.cpp    pc_dma.hpp os_realmem.lib pc_bus.lib
	$(CPP) $<
pc_dma.lib:    pc_dma.$(OBJ)            os_realmem.lib pc_bus.lib
	rm -f $@
	$(LIB) $@ $^

vga_pageflip.$(OBJ): vga_pageflip.cpp    vga_pageflip.hpp vga_mode.lib
	$(CPP) $<
vga_pageflip.lib:    vga_pageflip.$(OBJ)                  vga_mode.lib
	rm -f $@
	$(LIB) $@ $^

pc_pic.$(OBJ): pc_pic.cpp    pc_pic.hpp pc_bus.lib
	$(CPP) $<
pc_pic.lib:    pc_pic.$(OBJ)            pc_bus.lib
	rm -f $@
	$(LIB) $@ $^

os_realmem.$(OBJ): os_realmem.cpp    os_realmem.hpp
	$(CPP) $<
os_realmem.lib:    os_realmem.$(OBJ)
	rm -f $@
	$(LIB) $@ $^

ost.$(OBJ): ost.cpp    ost.hpp
	$(CPP) $<
ost.lib:    ost.$(OBJ)
	rm -f $@
	$(LIB) $@ $^

ost.cpp ost.hpp: urea.mod bin2cpp.exe
	./bin2cpp $< ost rqdq ostData

bin2cpp.exe: bin2cpp.cpp
	$(HOST_CPP) -o $@ $<

exe2coff.exe: exe2coff.cpp
	$(HOST_CPP) -o $@ $<

clean:
	rm *.$(OBJ)
	rm *.lib
	rm app*.exe
	rm app*.coff
	rm app*.map
	rm bin2cpp.exe
	rm exe2coff.exe
	rm ost.cpp
	rm ost.hpp
