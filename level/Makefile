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

INC_X86 = $(INC)
CFLAGS_X86 = $(CFLAGS)
RESINC_X86 = $(RESINC)
RCFLAGS_X86 = $(RCFLAGS)
LIBDIR_X86 = $(LIBDIR)
LIB_X86 = $(LIB)
LDFLAGS_X86 = $(LDFLAGS) -lpthread
OBJDIR_X86 = obj/Release
DEP_X86 = 
OUT_X86 = ../bin/Levelx86

OBJ_X86 = $(OBJDIR_X86)/tjunc.o $(OBJDIR_X86)/mem.o $(OBJDIR_X86)/outside.o $(OBJDIR_X86)/portals.o $(OBJDIR_X86)/scriptlib.o $(OBJDIR_X86)/solidbsp.o $(OBJDIR_X86)/threads.o $(OBJDIR_X86)/tree.o $(OBJDIR_X86)/vis.o $(OBJDIR_X86)/vis_flow.o $(OBJDIR_X86)/vis_sound.o $(OBJDIR_X86)/winding.o $(OBJDIR_X86)/writebsp.o $(OBJDIR_X86)/faces.o $(OBJDIR_X86)/bsp2prt.o $(OBJDIR_X86)/bsp_wad.o $(OBJDIR_X86)/bspfile.o $(OBJDIR_X86)/bspinfo.o $(OBJDIR_X86)/cmdlib.o $(OBJDIR_X86)/csg4.o $(OBJDIR_X86)/brush.o $(OBJDIR_X86)/level_main.o $(OBJDIR_X86)/light.o $(OBJDIR_X86)/light_face.o $(OBJDIR_X86)/light_trace.o $(OBJDIR_X86)/map.o $(OBJDIR_X86)/mathlib.o

all: x86

clean: clean_x86

before_x86: 
	test -d ../bin || mkdir -p ../bin
	test -d $(OBJDIR_X86) || mkdir -p $(OBJDIR_X86)

after_x86: 

x86: before_x86 out_x86 after_x86

out_x86: before_x86 $(OBJ_X86) $(DEP_X86)
	$(LD) $(LIBDIR_X86) -o $(OUT_X86) $(OBJ_X86)  $(LDFLAGS_X86) $(LIB_X86)

$(OBJDIR_X86)/tjunc.o: tjunc.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c tjunc.c -o $(OBJDIR_X86)/tjunc.o

$(OBJDIR_X86)/mem.o: mem.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c mem.c -o $(OBJDIR_X86)/mem.o

$(OBJDIR_X86)/outside.o: outside.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c outside.c -o $(OBJDIR_X86)/outside.o

$(OBJDIR_X86)/portals.o: portals.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c portals.c -o $(OBJDIR_X86)/portals.o

$(OBJDIR_X86)/scriptlib.o: scriptlib.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c scriptlib.c -o $(OBJDIR_X86)/scriptlib.o

$(OBJDIR_X86)/solidbsp.o: solidbsp.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c solidbsp.c -o $(OBJDIR_X86)/solidbsp.o

$(OBJDIR_X86)/threads.o: threads.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c threads.c -o $(OBJDIR_X86)/threads.o

$(OBJDIR_X86)/tree.o: tree.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c tree.c -o $(OBJDIR_X86)/tree.o

$(OBJDIR_X86)/vis.o: vis.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c vis.c -o $(OBJDIR_X86)/vis.o

$(OBJDIR_X86)/vis_flow.o: vis_flow.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c vis_flow.c -o $(OBJDIR_X86)/vis_flow.o

$(OBJDIR_X86)/vis_sound.o: vis_sound.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c vis_sound.c -o $(OBJDIR_X86)/vis_sound.o

$(OBJDIR_X86)/winding.o: winding.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c winding.c -o $(OBJDIR_X86)/winding.o

$(OBJDIR_X86)/writebsp.o: writebsp.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c writebsp.c -o $(OBJDIR_X86)/writebsp.o

$(OBJDIR_X86)/faces.o: faces.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c faces.c -o $(OBJDIR_X86)/faces.o

$(OBJDIR_X86)/bsp2prt.o: bsp2prt.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c bsp2prt.c -o $(OBJDIR_X86)/bsp2prt.o

$(OBJDIR_X86)/bsp_wad.o: bsp_wad.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c bsp_wad.c -o $(OBJDIR_X86)/bsp_wad.o

$(OBJDIR_X86)/bspfile.o: bspfile.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c bspfile.c -o $(OBJDIR_X86)/bspfile.o

$(OBJDIR_X86)/bspinfo.o: bspinfo.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c bspinfo.c -o $(OBJDIR_X86)/bspinfo.o

$(OBJDIR_X86)/cmdlib.o: cmdlib.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c cmdlib.c -o $(OBJDIR_X86)/cmdlib.o

$(OBJDIR_X86)/csg4.o: csg4.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c csg4.c -o $(OBJDIR_X86)/csg4.o

$(OBJDIR_X86)/brush.o: brush.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c brush.c -o $(OBJDIR_X86)/brush.o

$(OBJDIR_X86)/level_main.o: level_main.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c level_main.c -o $(OBJDIR_X86)/level_main.o

$(OBJDIR_X86)/light.o: light.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c light.c -o $(OBJDIR_X86)/light.o

$(OBJDIR_X86)/light_face.o: light_face.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c light_face.c -o $(OBJDIR_X86)/light_face.o

$(OBJDIR_X86)/light_trace.o: light_trace.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c light_trace.c -o $(OBJDIR_X86)/light_trace.o

$(OBJDIR_X86)/map.o: map.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c map.c -o $(OBJDIR_X86)/map.o

$(OBJDIR_X86)/mathlib.o: mathlib.c
	$(CC) $(CFLAGS_X86) $(INC_X86) -c mathlib.c -o $(OBJDIR_X86)/mathlib.o

clean_x86: 
	rm -f $(OBJ_X86) $(OUT_X86)
	rm ../bin/Levelx86
	rm -rf $(OBJDIR_X86)

.PHONY: before_x86 after_x86 clean_x86

