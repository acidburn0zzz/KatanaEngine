WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = 
CFLAGS = -Wall -g
RESINC = 
LIBDIR = 
LIB = 
LDFLAGS = 

INC_X64 = $(INC) -I../ -I../shared -I../game/shared -I../external
CFLAGS_X64 = $(CFLAGS) -fPIC
RESINC_X64 = $(RESINC)
RCFLAGS_X64 = $(RCFLAGS)
LIBDIR_X64 = $(LIBDIR)
LIB_X64 = $(LIB)../platform/lib/liblibplatformx86.a
LDFLAGS_X64 = $(LDFLAGS)
OBJDIR_X64 = obj/Release
DEP_X64 = 
OUT_X64 = ../../../openkatana/menu.so

OBJ_X64 = $(OBJDIR_X64)/menu_button.o $(OBJDIR_X64)/menu_main.o

all: x64

clean: clean_x64

before_x64: 
	test -d ../../../openkatana || mkdir -p ../../../openkatana
	test -d $(OBJDIR_X64) || mkdir -p $(OBJDIR_X64)

after_x64: 

x64: before_x64 out_x64 after_x64

out_x64: before_x64 $(OBJ_X64) $(DEP_X64)
	$(LD) -shared $(LIBDIR_X64) $(OBJ_X64)  -o $(OUT_X64) $(LDFLAGS_X64) $(LIB_X64)

$(OBJDIR_X64)/menu_button.o: menu_button.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c menu_button.c -o $(OBJDIR_X64)/menu_button.o

$(OBJDIR_X64)/menu_main.o: menu_main.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c menu_main.c -o $(OBJDIR_X64)/menu_main.o

clean_x64: 
	rm -f $(OBJ_X64) $(OUT_X64)
	rm -rf ../../../openkatana
	rm -rf $(OBJDIR_X64)

.PHONY: before_x64 after_x64 clean_x64

