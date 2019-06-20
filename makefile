COMMON_FLAGS = -q -bt=dos -mf -3r -fp5 #-dSHOW_TIMING
RELEASE_FLAGS = -ox -d0 -dNDEBUG
DEBUG_FLAGS = -od -d3 -xs

CPP = wpp386.exe -q $(COMMON_FLAGS) $(RELEASE_FLAGS)

HOST_CPP = wcl386 -q -xs

LFLAGS = SYSTEM pmodewi OPTION quiet

LD = wlink.exe $(LFLAGS)

LIB = wlib -n -q

DOSBOX = c:\bin\dosbox-x\dosbox-x.exe

run: app.exe
	$(DOSBOX) -conf dosbox.conf

app.exe: app.lib
	$(LD) N $@ F $<

app.obj: app.cpp kbd.lib vga.lib snd.lib mod.lib ost.lib efx.lib
	$(CPP) $[@
app.lib: app.obj kbd.lib vga.lib snd.lib mod.lib ost.lib efx.lib
	$(LIB) $@ $<

efx.obj: efx.cpp efx.hpp vga.lib
	$(CPP) $[@
efx.lib: efx.obj         vga.lib
	$(LIB) $@ $<

kbd.obj: kbd.cpp kbd.hpp 
	$(CPP) $[@
kbd.lib: kbd.obj
	$(LIB) $@ $<

vga.obj: vga.cpp vga.hpp pit.lib
	$(CPP) $[@
vga.lib: vga.obj         pit.lib
	$(LIB) $@ $<

snd.obj: snd.cpp snd.hpp dma.lib pic.lib
	$(CPP) $[@
snd.lib: snd.obj         dma.lib pic.lib
	$(LIB) $@ $<

mod.obj: mod.cpp mod.hpp
	$(CPP) $[@
mod.lib: mod.obj
	$(LIB) $@ $<

pit.obj: pit.cpp pit.hpp
	$(CPP) $[@
pit.lib: pit.obj
	$(LIB) $@ $<

dma.obj: dma.cpp dma.hpp mem.lib
	$(CPP) $[@
dma.lib: dma.obj         mem.lib
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
