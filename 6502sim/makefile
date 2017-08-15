#todo : switch to scons

CFLAGS = -g --std=c++11 -I $(Z64ROOT)/stage/include -D$(Z64BUILDPLATFORM)

LFLAGS = -L$(Z64ROOT)/stage/lib -lork.core.$(Z64BUILDPLATFORM).release -lglfw3 -ldrawtext

ifeq ($(Z64BUILDPLATFORM),OSX)
LFLAGS += -framework OpenGL
else
LFLAGS += -lGL -lpthread -lXxf86vm -lX11 -lXrandr -lXinerama -lXcursor -lrt -ldl
endif

all:
	clang++ $(CFLAGS) main.cpp mos6502.cpp ui.cpp -o 6502sim.exe $(LFLAGS) 
