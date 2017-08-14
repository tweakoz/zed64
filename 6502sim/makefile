#todo : switch to scons

CFLAGS = -g --std=c++11 -I $(Z64ROOT)/stage/include -DOSX
LFLAGS = -L$(Z64ROOT)/stage/lib -lork.core.OSX.release -lglfw3 -ldrawtext -framework OpenGL

all:
	clang++ $(CFLAGS) main.cpp mos6502.cpp ui.cpp -o 6502sim.exe $(LFLAGS) 
