#ŠÈ—ªŽ®makfefile for msys64 

TAR = main.exe

OBJS = \
	obj/main.o \
	obj/ahdr.o \
	obj/afilter.o \
	obj/shapeobj.o \
	obj/model.o \
	obj/light.o \
	obj/renderer.o \


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

