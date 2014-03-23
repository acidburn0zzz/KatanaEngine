WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = 
CFLAGS = -Wall -g -fPIC
RESINC = 
LIBDIR = 
LIB = 
LDFLAGS = 

INC_X64 = $(INC) -I. -I.. -I../shared -I../common -I../game/shared -I../Include -I../platform/include -I../external -I../external/SDL2/include
CFLAGS_X64 = $(CFLAGS) -DKATANA
RESINC_X64 = $(RESINC)
RCFLAGS_X64 = $(RCFLAGS)
LIBDIR_X64 = $(LIBDIR)
LIB_X64 = $(LIB)../platform/lib/liblibplatformx86.a -lSDL2
LDFLAGS_X64 = $(LDFLAGS) -lGL -lGLU -lopenal -lX11
OBJDIR_X64 = obj/Release
DEP_X64 = 
OUT_X64 = ../../../engine/engine.so
OBJ_X64 = $(OBJDIR_X64)/__/menu.o $(OBJDIR_X64)/__/net_dgrm.o $(OBJDIR_X64)/__/net_loop.o $(OBJDIR_X64)/__/net_main.o $(OBJDIR_X64)/__/pr_edict.o $(OBJDIR_X64)/__/r_brush.o $(OBJDIR_X64)/__/r_world.o $(OBJDIR_X64)/__/keys.o $(OBJDIR_X64)/__/image.o $(OBJDIR_X64)/__/host_cmd.o $(OBJDIR_X64)/__/host.o $(OBJDIR_X64)/__/gl_warp.o $(OBJDIR_X64)/__/gl_vidnt.o $(OBJDIR_X64)/__/gl_texmgr.o $(OBJDIR_X64)/__/gl_sky.o $(OBJDIR_X64)/__/gl_rmisc.o $(OBJDIR_X64)/__/gl_rmain.o $(OBJDIR_X64)/__/gl_refrag.o $(OBJDIR_X64)/__/sys_win.o $(OBJDIR_X64)/__/snd_dma.o $(OBJDIR_X64)/__/snd_mem.o $(OBJDIR_X64)/__/snd_mix.o $(OBJDIR_X64)/__/sv_main.o $(OBJDIR_X64)/__/sv_user.o $(OBJDIR_X64)/__/world.o $(OBJDIR_X64)/__/zone.o $(OBJDIR_X64)/__/gl_model.o $(OBJDIR_X64)/__/shared/shared_png.o $(OBJDIR_X64)/__/shared/shared_math.o $(OBJDIR_X64)/__/sbar.o $(OBJDIR_X64)/__/common.o $(OBJDIR_X64)/__/console.o $(OBJDIR_X64)/__/crc.o $(OBJDIR_X64)/__/cmdlib.o $(OBJDIR_X64)/__/cvar.o $(OBJDIR_X64)/engine_audio.o $(OBJDIR_X64)/engine_client.o $(OBJDIR_X64)/__/cmd.o $(OBJDIR_X64)/__/cl_tent.o $(OBJDIR_X64)/__/cl_parse.o $(OBJDIR_X64)/__/cl_main.o $(OBJDIR_X64)/__/cl_input.o $(OBJDIR_X64)/__/cl_demo.o $(OBJDIR_X64)/__/chase.o $(OBJDIR_X64)/__/KatGL.o $(OBJDIR_X64)/__/gl_fog.o $(OBJDIR_X64)/__/gl_draw.o $(OBJDIR_X64)/__/gl_alias.o $(OBJDIR_X64)/engine_view.o $(OBJDIR_X64)/engine_video.o $(OBJDIR_X64)/engine_sprite.o $(OBJDIR_X64)/engine_server.o $(OBJDIR_X64)/engine_script.o $(OBJDIR_X64)/engine_screen.o $(OBJDIR_X64)/engine_physics.o $(OBJDIR_X64)/engine_network.o $(OBJDIR_X64)/__/Include/glee/GLee.o $(OBJDIR_X64)/engine_menu.o $(OBJDIR_X64)/engine_masterserver.o $(OBJDIR_X64)/engine_main.o $(OBJDIR_X64)/engine_loader.o $(OBJDIR_X64)/engine_light.o $(OBJDIR_X64)/engine_input.o $(OBJDIR_X64)/engine_game.o $(OBJDIR_X64)/engine_editor.o $(OBJDIR_X64)/engine_console.o

all: x64

clean: clean_x64

before_x64: 
	test -d ../../../engine || mkdir -p ../../../engine
	test -d $(OBJDIR_X64)/__ || mkdir -p $(OBJDIR_X64)/__
	test -d $(OBJDIR_X64)/__/shared || mkdir -p $(OBJDIR_X64)/__/shared
	test -d $(OBJDIR_X64) || mkdir -p $(OBJDIR_X64)
	test -d $(OBJDIR_X64)/__/Include/glee || mkdir -p $(OBJDIR_X64)/__/Include/glee

after_x64: 

x64: before_x64 out_x64 after_x64

out_x64: before_x64 $(OBJ_X64) $(DEP_X64)
	$(LD) -shared $(LIBDIR_X64) $(OBJ_X64)  -o $(OUT_X64) $(LDFLAGS_X64) $(LIB_X64)

$(OBJDIR_X64)/__/menu.o: ../menu.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../menu.c -o $(OBJDIR_X64)/__/menu.o

$(OBJDIR_X64)/__/net_dgrm.o: ../net_dgrm.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../net_dgrm.c -o $(OBJDIR_X64)/__/net_dgrm.o

$(OBJDIR_X64)/__/net_loop.o: ../net_loop.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../net_loop.c -o $(OBJDIR_X64)/__/net_loop.o

$(OBJDIR_X64)/__/net_main.o: ../net_main.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../net_main.c -o $(OBJDIR_X64)/__/net_main.o

$(OBJDIR_X64)/__/pr_edict.o: ../pr_edict.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../pr_edict.c -o $(OBJDIR_X64)/__/pr_edict.o

$(OBJDIR_X64)/__/r_brush.o: ../r_brush.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../r_brush.c -o $(OBJDIR_X64)/__/r_brush.o

$(OBJDIR_X64)/__/r_world.o: ../r_world.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../r_world.c -o $(OBJDIR_X64)/__/r_world.o

$(OBJDIR_X64)/__/keys.o: ../keys.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../keys.c -o $(OBJDIR_X64)/__/keys.o

$(OBJDIR_X64)/__/image.o: ../image.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../image.c -o $(OBJDIR_X64)/__/image.o

$(OBJDIR_X64)/__/host_cmd.o: ../host_cmd.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../host_cmd.c -o $(OBJDIR_X64)/__/host_cmd.o

$(OBJDIR_X64)/__/host.o: ../host.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../host.c -o $(OBJDIR_X64)/__/host.o

$(OBJDIR_X64)/__/gl_warp.o: ../gl_warp.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../gl_warp.c -o $(OBJDIR_X64)/__/gl_warp.o

$(OBJDIR_X64)/__/gl_vidnt.o: ../gl_vidnt.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../gl_vidnt.c -o $(OBJDIR_X64)/__/gl_vidnt.o

$(OBJDIR_X64)/__/gl_texmgr.o: ../gl_texmgr.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../gl_texmgr.c -o $(OBJDIR_X64)/__/gl_texmgr.o

$(OBJDIR_X64)/__/gl_sky.o: ../gl_sky.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../gl_sky.c -o $(OBJDIR_X64)/__/gl_sky.o

$(OBJDIR_X64)/__/gl_rmisc.o: ../gl_rmisc.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../gl_rmisc.c -o $(OBJDIR_X64)/__/gl_rmisc.o

$(OBJDIR_X64)/__/gl_rmain.o: ../gl_rmain.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../gl_rmain.c -o $(OBJDIR_X64)/__/gl_rmain.o

$(OBJDIR_X64)/__/gl_refrag.o: ../gl_refrag.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../gl_refrag.c -o $(OBJDIR_X64)/__/gl_refrag.o

$(OBJDIR_X64)/__/sys_win.o: ../sys_win.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../sys_win.c -o $(OBJDIR_X64)/__/sys_win.o

$(OBJDIR_X64)/__/snd_dma.o: ../snd_dma.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../snd_dma.c -o $(OBJDIR_X64)/__/snd_dma.o

$(OBJDIR_X64)/__/snd_mem.o: ../snd_mem.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../snd_mem.c -o $(OBJDIR_X64)/__/snd_mem.o

$(OBJDIR_X64)/__/snd_mix.o: ../snd_mix.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../snd_mix.c -o $(OBJDIR_X64)/__/snd_mix.o

$(OBJDIR_X64)/__/sv_main.o: ../sv_main.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../sv_main.c -o $(OBJDIR_X64)/__/sv_main.o

$(OBJDIR_X64)/__/sv_user.o: ../sv_user.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../sv_user.c -o $(OBJDIR_X64)/__/sv_user.o

$(OBJDIR_X64)/__/world.o: ../world.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../world.c -o $(OBJDIR_X64)/__/world.o

$(OBJDIR_X64)/__/zone.o: ../zone.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../zone.c -o $(OBJDIR_X64)/__/zone.o

$(OBJDIR_X64)/__/gl_model.o: ../gl_model.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../gl_model.c -o $(OBJDIR_X64)/__/gl_model.o

$(OBJDIR_X64)/__/shared/shared_png.o: ../shared/shared_png.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../shared/shared_png.c -o $(OBJDIR_X64)/__/shared/shared_png.o

$(OBJDIR_X64)/__/shared/shared_math.o: ../shared/shared_math.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../shared/shared_math.c -o $(OBJDIR_X64)/__/shared/shared_math.o

$(OBJDIR_X64)/__/sbar.o: ../sbar.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../sbar.c -o $(OBJDIR_X64)/__/sbar.o

$(OBJDIR_X64)/__/common.o: ../common.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../common.c -o $(OBJDIR_X64)/__/common.o

$(OBJDIR_X64)/__/console.o: ../console.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../console.c -o $(OBJDIR_X64)/__/console.o

$(OBJDIR_X64)/__/crc.o: ../crc.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../crc.c -o $(OBJDIR_X64)/__/crc.o

$(OBJDIR_X64)/__/cmdlib.o: ../cmdlib.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../cmdlib.c -o $(OBJDIR_X64)/__/cmdlib.o

$(OBJDIR_X64)/__/cvar.o: ../cvar.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../cvar.c -o $(OBJDIR_X64)/__/cvar.o

$(OBJDIR_X64)/engine_audio.o: engine_audio.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_audio.c -o $(OBJDIR_X64)/engine_audio.o

$(OBJDIR_X64)/engine_client.o: engine_client.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_client.c -o $(OBJDIR_X64)/engine_client.o

$(OBJDIR_X64)/__/cmd.o: ../cmd.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../cmd.c -o $(OBJDIR_X64)/__/cmd.o

$(OBJDIR_X64)/__/cl_tent.o: ../cl_tent.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../cl_tent.c -o $(OBJDIR_X64)/__/cl_tent.o

$(OBJDIR_X64)/__/cl_parse.o: ../cl_parse.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../cl_parse.c -o $(OBJDIR_X64)/__/cl_parse.o

$(OBJDIR_X64)/__/cl_main.o: ../cl_main.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../cl_main.c -o $(OBJDIR_X64)/__/cl_main.o

$(OBJDIR_X64)/__/cl_input.o: ../cl_input.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../cl_input.c -o $(OBJDIR_X64)/__/cl_input.o

$(OBJDIR_X64)/__/cl_demo.o: ../cl_demo.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../cl_demo.c -o $(OBJDIR_X64)/__/cl_demo.o

$(OBJDIR_X64)/__/chase.o: ../chase.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../chase.c -o $(OBJDIR_X64)/__/chase.o

$(OBJDIR_X64)/__/KatGL.o: ../KatGL.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../KatGL.c -o $(OBJDIR_X64)/__/KatGL.o

$(OBJDIR_X64)/__/gl_fog.o: ../gl_fog.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../gl_fog.c -o $(OBJDIR_X64)/__/gl_fog.o

$(OBJDIR_X64)/__/gl_draw.o: ../gl_draw.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../gl_draw.c -o $(OBJDIR_X64)/__/gl_draw.o

$(OBJDIR_X64)/__/gl_alias.o: ../gl_alias.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../gl_alias.c -o $(OBJDIR_X64)/__/gl_alias.o

$(OBJDIR_X64)/engine_view.o: engine_view.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_view.c -o $(OBJDIR_X64)/engine_view.o

$(OBJDIR_X64)/engine_video.o: engine_video.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_video.c -o $(OBJDIR_X64)/engine_video.o

$(OBJDIR_X64)/engine_sprite.o: engine_sprite.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_sprite.c -o $(OBJDIR_X64)/engine_sprite.o

$(OBJDIR_X64)/engine_server.o: engine_server.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_server.c -o $(OBJDIR_X64)/engine_server.o

$(OBJDIR_X64)/engine_script.o: engine_script.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_script.c -o $(OBJDIR_X64)/engine_script.o

$(OBJDIR_X64)/engine_screen.o: engine_screen.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_screen.c -o $(OBJDIR_X64)/engine_screen.o

$(OBJDIR_X64)/engine_physics.o: engine_physics.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_physics.c -o $(OBJDIR_X64)/engine_physics.o

$(OBJDIR_X64)/engine_network.o: engine_network.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_network.c -o $(OBJDIR_X64)/engine_network.o

$(OBJDIR_X64)/__/Include/glee/GLee.o: ../Include/glee/GLee.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c ../Include/glee/GLee.c -o $(OBJDIR_X64)/__/Include/glee/GLee.o

$(OBJDIR_X64)/engine_menu.o: engine_menu.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_menu.c -o $(OBJDIR_X64)/engine_menu.o

$(OBJDIR_X64)/engine_masterserver.o: engine_masterserver.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_masterserver.c -o $(OBJDIR_X64)/engine_masterserver.o

$(OBJDIR_X64)/engine_main.o: engine_main.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_main.c -o $(OBJDIR_X64)/engine_main.o

$(OBJDIR_X64)/engine_loader.o: engine_loader.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_loader.c -o $(OBJDIR_X64)/engine_loader.o

$(OBJDIR_X64)/engine_light.o: engine_light.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_light.c -o $(OBJDIR_X64)/engine_light.o

$(OBJDIR_X64)/engine_input.o: engine_input.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_input.c -o $(OBJDIR_X64)/engine_input.o

$(OBJDIR_X64)/engine_game.o: engine_game.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_game.c -o $(OBJDIR_X64)/engine_game.o

$(OBJDIR_X64)/engine_editor.o: engine_editor.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_editor.c -o $(OBJDIR_X64)/engine_editor.o

$(OBJDIR_X64)/engine_console.o: engine_console.c
	$(CC) $(CFLAGS_X64) $(INC_X64) -c engine_console.c -o $(OBJDIR_X64)/engine_console.o

clean_x64: 
	rm -f $(OBJ_X64) $(OUT_X64)
	rm -rf ../../../engine
	rm -rf $(OBJDIR_X64)/__
	rm -rf $(OBJDIR_X64)/__/shared
	rm -rf $(OBJDIR_X64)
	rm -rf $(OBJDIR_X64)/__/Include/glee

.PHONY: before_x64 after_x64 clean_x64

