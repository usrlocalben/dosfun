#CPPFLAGS = -q -bt=dos -mf -3r -fp5 -od -d3 -xs
CPPFLAGS = -q -bt=dos -mf -3r -fp5 -s -oilrt -dNDEBUG
# -fo main.obj

CPP = wpp386.exe $(CPPFLAGS)

LFLAGS = SYSTEM dos4g

LD = wlink.exe $(LFLAGS)

DOSBOX = c:\bin\dosbox-x\dosbox-x.exe

run: app.exe
	$(DOSBOX) -conf dosbox.conf

app.exe: app.lnk
	$(LD) @app.lnk

app.lnk: app.obj
	echo N app.exe > app.lnk
	echo F app.obj >> app.lnk
	echo F kbd.obj >> app.lnk
	echo F vga.obj >> app.lnk
	echo F snd.obj >> app.lnk
	echo F mod.obj >> app.lnk
	echo F pit.obj >> app.lnk
	echo F dma.obj >> app.lnk
	echo F pic.obj >> app.lnk
	echo F mem.obj >> app.lnk

app.obj: app.cpp kbd.obj vga.obj snd.obj mod.obj
	$(CPP) app.cpp

kbd.obj: kbd.cpp kbd.hpp 
	$(CPP) kbd.cpp

vga.obj: vga.cpp vga.hpp pit.obj
	$(CPP) vga.cpp

snd.obj: snd.cpp snd.hpp dma.obj pic.obj
	$(CPP) snd.cpp

mod.obj: mod.cpp mod.hpp
	$(CPP) mod.cpp

pit.obj: pit.cpp pit.hpp
	$(CPP) pit.cpp

dma.obj: dma.cpp dma.hpp mem.obj
	$(CPP) dma.cpp

pic.obj: pic.cpp pic.hpp
	$(CPP) pic.cpp

mem.obj: mem.cpp mem.hpp
	$(CPP) mem.cpp

clean:
	del *.obj
	del *.lnk
	del app.exe
