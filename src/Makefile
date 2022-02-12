.DEFAULT_GOAL := m

FLTK_LIBS=`fltk-config --ldflags`
FLTK_FLAGS=`fltk-config --cxxflags`
SDL2_LIBS=`sdl2-config --libs`
SDL2_FLAGS=`sdl2-config --cflags`

######################################################
# MacOSX
# To compile on MacOSX I used 10.6 with XCode dev tools and then
# used MacPorts to install fltk 1.1.7 and libsdl 1.2.13 
CC=g++
CFLGAGS=
# This may need to be changed for you. These are the X11 libraries used by
# ftlk or sdl
DYLIB=/opt/X11/lib/libX11.6.dylib /opt/X11/lib/libXext.6.dylib /opt/X11/lib/libXrandr.2.dylib /opt/X11/lib/libXrender.1.dylib /opt/X11/lib/libxcb.1.dylib /opt/X11/lib/libXau.6.dylib /opt/X11/lib/libXdmcp.6.dylib

Morse.app: Bargraph.o Codebox.o Cw.o Knob.o Help.h m.o Info.plist
	mkdir -p Morse.app/Contents/MacOS
	mkdir    Morse.app/Contents/plugins
	mkdir    Morse.app/Contents/Resources
	cp $(DYLIB) Morse.app/Contents/plugins
	$(CC) $(CFLAGS) -o Morse m.o Bargraph.o Codebox.o Cw.o Knob.o \
	  $(FLTK_LIBS) \
	  `pkg-config --cflags --libs sdl` \
	  -framework Carbon
	mv Morse Morse.app/Contents/MacOS/
	cp Info.plist Morse.app/Contents/info.plist
	for i in $(DYLIB); do \
           cp $$i Morse.app/Contents/plugins/`basename $$i`; \
           install_name_tool -id @executable_path/../plugins/`basename $$i` Morse.app/Contents/plugins/`basename $$i`; \
           install_name_tool -change $$i @executable_path/../plugins/`basename $$i` Morse.app/Contents/MacOS/Morse; \
           for j in $(DYLIB); do \
                install_name_tool -change $$j @executable_path/../plugins/`basename $$j` Morse.app/Contents/plugins/`basename $$i`; \
           done; \
        done
######################################################

######################################################
# Windows
# Use "xmake" to generate m.exe
m.exe: m.fl Bargraph.o Codebox.o Cw.cxx Cw.h Knob.cxx Knob.h Help.h m.o
	g++ -c -Os  -I/usr/local/include/SDL Cw.cxx
	g++ -om.exe m.o Bargraph.o Codebox.o Cw.o Knob.o \
	  $(FLTK_LIBS) \
	  $(SDL2_LIBS) \
	  -lmingw32 -lmingwex -lwinmm
	strip m.exe
	rm *.o m.cxx m.h
######################################################

# Regular "make" is what you want for the Linux version
m: m.fl Bargraph.o Codebox.o Cw.o Knob.o Help.h m.o
	g++ -om m.o Bargraph.o Codebox.o Cw.o Knob.o \
	  $(SDL2_LIBS) \
	  $(FLTK_LIBS) -ldl
	strip m
	rm *.o m.cxx m.h

m.cxx: m.fl
	fluid -c m.fl
m.o: m.cxx m.h
	g++ -c -Os $(FLTK_FLAGS) m.cxx
Bargraph.o: Bargraph.cxx Bargraph.h
	g++ -c -Os $(FLTK_FLAGS) Bargraph.cxx
Codebox.o: Codebox.cxx Codebox.h
	g++ -c -Os $(FLTK_FLAGS) Codebox.cxx
Knob.o: Knob.cxx Knob.h
	g++ -c -Os $(FLTK_FLAGS) Knob.cxx
Cw.o: Cw.cxx Cw.h
	g++ -c -Os $(SDL2_FLAGS) Cw.cxx

# Besemer's Python extension version
cw.so: cw.pyx Cw.cxx Cw.h
	pyrexc cw.pyx
	gcc -c -fPIC -I/usr/include/python2.5 cw.c
	gcc -c -fPIC $(SDL2_FLAGS) Cw.cxx
	g++ -shared cw.o Cw.o -static $(SDL2_LIBS) -ocw.so
	rm cw.c *.o

Help.h: m.htm
	./help.py <m.htm >Help.h

clean:
	rm -rf ./Morse.app m.cxx m.h *.o

.PHONY: clean
