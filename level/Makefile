WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = -I../shared
CFLAGS = -Wall -g
RESINC = 
LIBDIR = 
LIB = 
LDFLAGS = 

INC_X64 = $(INC)
CFLAGS_X64 = $(CFLAGS)
RESINC_X64 = $(RESINC)
RCFLAGS_X64 = $(RCFLAGS)
LIBDIR_X64 = $(LIBDIR)
LIB_X64 = $(LIB)
LDFLAGS_X64 = $(LDFLAGS) -lpthread
OBJDIR_X64 = obj/Release
DEP_X64 = 
OUT_X64 = /usr/local/bin/kbsp

OBJ_X64 = $(OBJDIR_X64)/tjunc.o $(OBJDIR_X64)/mem.o $(OBJDIR_X64)/outside.o $(OBJDIR_X64)/portals.o $(OBJDIR_X64)/scriptlib.o $(OBJDIR_X64)/solidbsp.o $(OBJDIR_X64)/threads.o $(OBJDIR_X64)/tree.o $(OBJDIR_X64)/vis.o $(OBJDIR_X64)/vis_flow.o $(OBJDIR_X64)/vis_sound.o $(OBJDIR_X64)/winding.o $(OBJDIR_X64)/writebsp.o $(OBJDIR_X64)/faces.o $(OBJDIR_X64)/bsp2prt.o $(OBJDIR_X64)/bsp_wad.o $(OBJDIR_X64)/bspfile.o $(OBJDIR_X64)/bspinfo.o $(OBJDIR_X64)/cmdlib.o $(OBJDIR_X64)/csg4.o $(OBJDIR_X64)/brush.o $(OBJDIR_X64)/level_main.o $(OBJDIR_X64)/light.o $(OBJDIR_X64)/light_face.o $(OBJDIR_X64)/light_trace.o $(OBJDIR_X64)/map.o $(OBJDIR_X64)/mathlib.o

all: x64

clean: clean_x64

before_x64: 
	test -d ../bin || mkdir -p ../bin
	test -d $(OBJDIR_X64) || mkdir -p $(OBJDIR_X64)

after_x64: 

x64: before_x64 out_x64 after_x64

out_x64: before_x64 $(OBJ_X64) $(DEP_X64)
	$(LD) $(LIBDIR_X64) -o $(OUT_X64) $(OBJ_X64)  $(LDFLAGS_X64) $(LIB_X64)

$(OBJDIR_X64)/tjunc.o: tjunc.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c tjunc.c -o $(OBJDIR_X64)/tjunc.o

$(OBJDIR_X64)/mem.o: mem.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c mem.c -o $(OBJDIR_X64)/mem.o

$(OBJDIR_X64)/outside.o: outside.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c outside.c -o $(OBJDIR_X64)/outside.o

$(OBJDIR_X64)/portals.o: portals.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c portals.c -o $(OBJDIR_X64)/portals.o

$(OBJDIR_X64)/scriptlib.o: scriptlib.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c scriptlib.c -o $(OBJDIR_X64)/scriptlib.o

$(OBJDIR_X64)/solidbsp.o: solidbsp.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c solidbsp.c -o $(OBJDIR_X64)/solidbsp.o

$(OBJDIR_X64)/threads.o: threads.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c threads.c -o $(OBJDIR_X64)/threads.o

$(OBJDIR_X64)/tree.o: tree.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c tree.c -o $(OBJDIR_X64)/tree.o

$(OBJDIR_X64)/vis.o: vis.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c vis.c -o $(OBJDIR_X64)/vis.o

$(OBJDIR_X64)/vis_flow.o: vis_flow.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c vis_flow.c -o $(OBJDIR_X64)/vis_flow.o

$(OBJDIR_X64)/vis_sound.o: vis_sound.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c vis_sound.c -o $(OBJDIR_X64)/vis_sound.o

$(OBJDIR_X64)/winding.o: winding.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c winding.c -o $(OBJDIR_X64)/winding.o

$(OBJDIR_X64)/writebsp.o: writebsp.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c writebsp.c -o $(OBJDIR_X64)/writebsp.o

$(OBJDIR_X64)/faces.o: faces.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c faces.c -o $(OBJDIR_X64)/faces.o

$(OBJDIR_X64)/bsp2prt.o: bsp2prt.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c bsp2prt.c -o $(OBJDIR_X64)/bsp2prt.o

$(OBJDIR_X64)/bsp_wad.o: bsp_wad.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c bsp_wad.c -o $(OBJDIR_X64)/bsp_wad.o

$(OBJDIR_X64)/bspfile.o: bspfile.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c bspfile.c -o $(OBJDIR_X64)/bspfile.o

$(OBJDIR_X64)/bspinfo.o: bspinfo.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c bspinfo.c -o $(OBJDIR_X64)/bspinfo.o

$(OBJDIR_X64)/cmdlib.o: cmdlib.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c cmdlib.c -o $(OBJDIR_X64)/cmdlib.o

$(OBJDIR_X64)/csg4.o: csg4.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c csg4.c -o $(OBJDIR_X64)/csg4.o

$(OBJDIR_X64)/brush.o: brush.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c brush.c -o $(OBJDIR_X64)/brush.o

$(OBJDIR_X64)/level_main.o: level_main.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c level_main.c -o $(OBJDIR_X64)/level_main.o

$(OBJDIR_X64)/light.o: light.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c light.c -o $(OBJDIR_X64)/light.o

$(OBJDIR_X64)/light_face.o: light_face.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c light_face.c -o $(OBJDIR_X64)/light_face.o

$(OBJDIR_X64)/light_trace.o: light_trace.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c light_trace.c -o $(OBJDIR_X64)/light_trace.o

$(OBJDIR_X64)/map.o: map.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c map.c -o $(OBJDIR_X64)/map.o

$(OBJDIR_X64)/mathlib.o: mathlib.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c mathlib.c -o $(OBJDIR_X64)/mathlib.o

clean_x64: 
	rm -f $(OBJ_X64) $(OUT_X64)
	rm ../bin/Levelx64
	rm -rf $(OBJDIR_X64)

.PHONY: before_x64 after_x64 clean_x64

