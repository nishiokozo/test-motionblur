#ŠÈ—ªŽ®makfefile for msys64 

TAR = main.exe

OBJS = \
	obj/main.o \
	obj/ahdr.o \
	obj/afilter.o \

LIBS = \
	-lgdi32 \
	-ld3d9 \
	-ld3dx9 \

FLGS = \
	-c \
	-m64 \
	-std=c++14 \
	-Wall \
	-Werror \
	-Wno-unknown-pragmas \
	-Wno-unused-function \
	-Wno-unused-variable \
	-O3 \

CC	= clang++
#CC	= g++

$(TAR)	:	obj $(OBJS) $(SHDR)
	$(CC) -o $(TAR) $(OBJS) $(LIBS)

obj/%.o:%.cpp
	$(CC)  $(FLGS) $< -o $@

obj:
	mkdir obj

clean:
	rm -f *.exe
	rm -rf obj





DEFS	= -DAPMAIN -DBOOK_ID=0 -D_M_IX86 -D__GNUC_AAA__ -DSTRICT -D_WIN32 -D__AGP__ -Di386 \
	  -DDLLEXPORT="__declspec(dllexport)" -DDLLIMPORT="__declspec(dllimport)" \
	  -DCOMMON_TILEDMAP -DDEBUG_WITH_JOYSTICK \
	  #-DCARMODEL_FF
OPT	= -O2 -ffast-math -Werror -Wall -mno-cygwin -fvtable-thunks -fnative-struct -fno-rtti \
	  -g -Wno-unknown-pragmas

