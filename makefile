
CPPFLAGS = -bt=dos -mf -3r -s -oilrt -fp5
#-fo main.obj

CPP = wpp386.exe $(CPPFLAGS)

LFLAGS = SYSTEM dos4g

LD = wlink.exe $(LFLAGS)

OBJ = obj

DOSBOX = C:\Users\benjamin\Desktop\bin\x64\Release\dosbox-x.exe

run: main.exe
	$(DOSBOX) -conf dosbox.conf

main.$(OBJ): main.cpp
	$(CPP) main.cpp

main.exe: main.lnk
	$(LD) @main.lnk

main.lnk: main.$(OBJ)
	echo N main.exe > main.lnk
	echo F main.$(OBJ) >> main.lnk

clean:
	del *.obj
	del *.lnk
	del main.exe
