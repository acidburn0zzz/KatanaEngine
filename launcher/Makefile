WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = -I../shared -I../platform/include
CFLAGS = -Wall -g
RESINC = 
LIBDIR = 
LIB = -lX11
LDFLAGS = 

INC_X64 = $(INC) -I../external -I../shared -I../platform/include
CFLAGS_X64 = $(CFLAGS) -Wall -g
RESINC_X64 = $(RESINC)
RCFLAGS_X64 = $(RCFLAGS)
LIBDIR_X64 = $(LIBDIR)
LIB_X64 = $(LIB) ../platform/lib/liblibplatformx86.a -lX11
LDFLAGS_X64 = $(LDFLAGS)
OBJDIR_X64 = obj/Release
DEP_X64 = 
OUT_X64 = ../../../OpenKatanax64

INC_X86 = $(INC) -I../external -I../shared
CFLAGS_X86 = $(CFLAGS) -m32
RESINC_X86 = $(RESINC)
RCFLAGS_X86 = $(RCFLAGS)
LIBDIR_X86 = $(LIBDIR)
LIB_X86 = $(LIB) ../platform/lib/liblibplatformx86.a -lX11
LDFLAGS_X86 = $(LDFLAGS)
OBJDIR_X86 = obj/Release
DEP_X86 = 
OUT_X86 = ../../../OpenKatanax86

OBJ_X64 = $(OBJDIR_X64)/launcher_main.o

OBJ_X86 = $(OBJDIR_X86)/launcher_main.o

all: x64 x86

clean: clean_x64 clean_x86

before_x64: 
	test -d ../../.. || mkdir -p ../../..
	test -d $(OBJDIR_X64) || mkdir -p $(OBJDIR_X64)

after_x64: 

x64: before_x64 out_x64 after_x64

out_x64: before_x64 $(OBJ_X64) $(DEP_X64)
	$(LD) $(LIBDIR_X64) -o $(OUT_X64) $(OBJ_X64)  $(LDFLAGS_X64) $(LIB_X64)

$(OBJDIR_X64)/launcher_main.o: launcher_main.cpp
	$(CXX) $(CFLAGS_X64) $(INC_X64) -c launcher_main.cpp -o $(OBJDIR_X64)/launcher_main.o

clean_x64: 
	rm -f $(OBJ_X64) $(OUT_X64)
	rm -rf ../../..
	rm -rf $(OBJDIR_X64)

before_x86: 
	test -d ../../.. || mkdir -p ../../..
	test -d $(OBJDIR_X86) || mkdir -p $(OBJDIR_X86)

after_x86: 

x86: before_x86 out_x86 after_x86

out_x86: before_x86 $(OBJ_X86) $(DEP_X86)
	$(LD) $(LIBDIR_X86) -o $(OUT_X86) $(OBJ_X86)  $(LDFLAGS_X86) $(LIB_X86)

$(OBJDIR_X86)/launcher_main.o: launcher_main.cpp
	$(CXX) $(CFLAGS_X86) $(INC_X86) -c launcher_main.cpp -o $(OBJDIR_X86)/launcher_main.o

clean_x86: 
	rm -f $(OBJ_X86) $(OUT_X86)
	rm -rf ../../..
	rm -rf $(OBJDIR_X86)

.PHONY: before_x64 after_x64 clean_x64 before_x86 after_x86 clean_x86

