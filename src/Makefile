# For some reason, xmake is required to make m.tgz.

m.tgz: m m.exe
	rm -f m.tgz
	tar czvf m.tgz *

# Use "xmake" to generate m.exe

m.exe: m.fl Bargraph.cxx Bargraph.h Codebox.cxx Codebox.h Cw.cxx Cw.h \
    Knob.cxx Knob.h Help.h m.fl
	fluid -c m.fl
	g++ -c -Os `fltk-config --cxxflags` m.cxx
	g++ -c -Os `fltk-config --cxxflags` Bargraph.cxx
	g++ -c -Os `fltk-config --cxxflags` Codebox.cxx
	g++ -c -Os `fltk-config --cxxflags` Knob.cxx
	g++ -c -Os  -I/usr/local/include/SDL Cw.cxx
	g++ -om.exe m.o Bargraph.o Codebox.o Cw.o Knob.o \
	  -static `fltk-config --ldstaticflags` \
	  -lSDL -lmingw32 -lmingwex -lwinmm -lSDLmain	  
	strip m.exe
	rm *.o m.cxx m.h
	
# Regular "make" is what you want for the Linux version

m: m.fl Bargraph.cxx Bargraph.h Codebox.cxx Codebox.h Cw.cxx Cw.h \
    Knob.cxx Knob.h Help.h m.fl
	fluid -c m.fl
	g++ -c -Os `fltk-config --cxxflags` m.cxx
	g++ -c -Os `fltk-config --cxxflags` Bargraph.cxx	
	g++ -c -Os `fltk-config --cxxflags` Codebox.cxx
	g++ -c -Os `fltk-config --cxxflags` Knob.cxx
	g++ -c -Os `sdl-config --cflags` Cw.cxx
	g++ -o m m.o Bargraph.o Codebox.o Cw.o Knob.o \
	  `sdl-config --libs` \
	  `fltk-config --ldflags` -ldl
	strip m
	rm *.o m.cxx m.h

# Besemer's Python extension version

cw.so: cw.pyx Cw.cxx Cw.h
	pyrexc cw.pyx
	gcc -c -fPIC -I/usr/include/python2.3 cw.c
	gcc -c -fPIC `sdl-config --cflags` Cw.cxx
	g++ -shared cw.o Cw.o -static `sdl-config --static-libs` -ocw.so
	rm cw.c *.o

Help.h: m.htm
	./help.py <m.htm >Help.h
